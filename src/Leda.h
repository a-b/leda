#ifndef _LEDA_H
#define	_LEDA_H


#include "common.h"
#include <propeller/HttpServer.h>
#include <propeller/Client.h>

#include "LuaState.h"
#include "FWatcher.h"
#include "Dictionary.h"

    
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
    
    void setScriptArguments( const LuaState::ScriptArguments& arguments ) 
    {
        m_scriptArguments = arguments;
    }
    
    LuaState* newLua();
    
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
    
    void addFileChange();
    unsigned int changes() const
    {
        return m_changes;
    }
    
    void resetChanges();
    
    std::string version() const;
    
    Dictionary& dictionary() 
    {
        return m_dictionary;
    }
    
    void addThread( const sys::Thread& thread );

private:
    Leda();
        

private:
    std::string m_script;
    LuaState::ScriptArguments m_scriptArguments;
    
    static Leda* m_instance;
    PathList m_paths;
    LuaState* m_lua;
    propeller::Client* m_client;
    propeller::Server* m_server;
    bool m_debug;
    FWatcher* m_fwatcher;
    unsigned int m_changes;
    Dictionary m_dictionary;

    
};

#endif	/* _LEDA_H */

