/*
Copyright 2012-2013 Sergey Zavadski

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */

#ifndef _LEDA_H
#define	_LEDA_H


#include "common.h"
#include <propeller/HttpServer.h>
#include <propeller/Client.h>

#include "LuaState.h"


//
//  C API to lua
//
extern int generateRandomString( lua_State* lua );
extern int clientCreate( lua_State* lua );
extern int clientConnect( lua_State* lua );
extern int clientAddTimer( lua_State* lua );
extern int clientConnectionSendMessage( lua_State* lua );
extern int clientConnectionClose( lua_State* lua );
extern int serverConnectionSendMessage( lua_State* lua );
extern int serverConnectionSendData( lua_State* lua );
extern int serverConnectionGetAddress( lua_State* lua );
extern int serverConnectionGetId( lua_State* lua );
extern int serverCreate( lua_State* lua );
extern int serverAddTimer( lua_State* lua );
extern int getpid( lua_State* lua );
extern int threadGetId( lua_State* lua );


    
class Leda 
{
public:
    
    virtual ~Leda();
    static Leda* instance();

            
    void addPath( const std::string& path )
    {   
        m_paths.push_back( path );
    }
    
    void setScript( const std::string& script )
    {
        m_script = script;
    }
    
    const std::string& script()
    {
        return m_script;
    }
    
    void clientCreate( );
    
    void onTerminate();
    void execScript( );
    void serverCreate( lua_State* lua );
    
    propeller::Client* client() const
    {
        return m_client;
    }
    
    propeller::Server* server() const
    {
        return m_server;
    }
    
    enum ServerType
    {
        ServerHttp,
        ServerMessage
    };
    
    struct TimerData
    {
       int index;
       bool once;
       
       TimerData( int _index, bool _once )
       : index( _index ), once( _once )
       {
           
       }
    };
    
    typedef std::list< std::string > PathList;
    
    const PathList& paths() const
    {
        return m_paths;
    }
    
    bool debug() const
    {
        return m_debug;
    }

    void callTimer( const LuaState& lua, TimerData* data );
    void callTerminate( const LuaState& lua );
    
    const LuaState& lua() const
    {
        return *m_lua;
    }
    

private:
    Leda();

private:
    std::string m_script;
    static Leda* m_instance;
    PathList m_paths;
    LuaState* m_lua;
    propeller::Client* m_client;
    propeller::Server* m_server;
    ServerType m_serverType;
    bool m_debug;
};

#endif	/* _LEDA_H */

