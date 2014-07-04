#ifndef _LEDA_H
#define	_LEDA_H


#include "common.h"
#include <propeller/HttpServer.h>
#include <propeller/Client.h>

#include "LuaState.h"
#include "FWatcher.h"


//
//  C API to lua
//
extern int generateRandomString( lua_State* lua );
extern int clientCreate( lua_State* lua );
extern int clientConnect( lua_State* lua );
extern int clientAddTimer( lua_State* lua );
extern int clientConnectionSendMessage( lua_State* lua );
extern int clientConnectionClose( lua_State* lua );
extern int clientConnectionSendData( lua_State* lua );
extern int serverConnectionSendData( lua_State* lua );
extern int serverConnectionGetAddress( lua_State* lua );
extern int serverConnectionGetId( lua_State* lua );
extern int serverSendTo( lua_State* lua );
extern int serverCreate( lua_State* lua );
extern int serverAddTimer( lua_State* lua );
extern int getpid( lua_State* lua );
extern int processorCount( lua_State* lua );
extern int httpRequestGetUrl( lua_State* lua );
extern int httpRequestGetHeaders( lua_State* lua );
extern int httpRequestGetBody( lua_State* lua );
extern int httpRequestGetAddress( lua_State* lua );
extern int httpResponseSetBody( lua_State* lua );
extern int httpResponseSetStatus( lua_State* lua );
extern int httpResponseSetHeaders( lua_State* lua );
extern int httpResponseAddHeader( lua_State* lua );





    
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
    
    propeller::Client* clientCreate( unsigned int threadCount = 1 );
    
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

    void callTimer( LuaState& lua, TimerData* data );
    void callTerminate( const LuaState& lua );
    
    const LuaState& lua() const
    {
        return *m_lua;
    }
    
    void addFileChange();
    unsigned int changes() const
    {
        return m_changes;
    }
    
    void resetChanges();
    

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
    FWatcher* m_fwatcher;
    unsigned int m_changes;
    
};

#endif	/* _LEDA_H */

