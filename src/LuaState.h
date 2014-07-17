#ifndef LUASTATE_H
#define	LUASTATE_H

//
//  include lua
//
#define LUA_COMPAT_MODULE
#include <lua.hpp>

//
//  common library
//  
#include "common.h"
//
//  system library
//  
#include "propeller/system.h"

class LuaState
{
public:
    typedef std::list< std::string > ScriptArguments;
    
    LuaState( const std::string& filename, const ScriptArguments* arguments = NULL );
    LuaState( const LuaState& lua );
    virtual ~LuaState( );
    
    void load(  unsigned int threadId = 0, bool exception = false, const char* init = NULL );
    
    void call( const std::string& callbackName, int registryIndex = -1, bool exception = false ); 
    
    void execute( const std::string& script ) const;
    
    
    
    operator lua_State*() const
    {
        return m_lua;
    }
    
    void setClose( bool close )
    {
        m_close = close;
    }
    
    sys::Lock& lock()
    {
        return m_lock;
    }
    
    void setGlobal( const std::string& name, unsigned int value );
    void setGlobal( const std::string& name, void* value );
    void setGlobal( const std::string& name, const char* value, unsigned int length );
    void setGlobal( const std::string& name, const std::string& value );
    void setGlobal( const std::string& name, bool value = true );
    void setArguments( );
    
    unsigned int getGlobal( const std::string& name );
    
    static LuaState& luaFromThread( const sys::Thread& thread, unsigned int threadId );
    static LuaState* luaForThread( sys::Thread& thread, unsigned int id, const char* init = NULL );
    static unsigned int getThreadId( lua_State* lua );
     
    
private:
    void destroy();
    void create();
    
    
    
    
private:
    void reload( unsigned int threadId );
     void loadlibs() const;   
     void addPaths( const char* name ) const; 
     void getGlobalTable();
    
private:
    lua_State* m_lua;
    bool m_close;    
    sys::Lock m_lock;
    std::string m_filename;
    ScriptArguments m_arguments;
    static sys::Lock s_lock;
};

#endif	/* LUASTATE_H */

