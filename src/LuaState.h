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
    LuaState( const std::string& filename );
    LuaState( const LuaState& lua );
    virtual ~LuaState( );
    
    bool load( const char* init = NULL, bool reload = false );
    void call( const std::string& callbackName, int registryIndex = -1, bool exception = false ); 
    bool reload( );
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
    
    
protected:
    
    
    
private:
     void loadlibs() const;   
     void addPaths( const char* name ) const;
     void destroy();
     void create();
     
     
     
    
private:
    lua_State* m_lua;
    bool m_close;    
    sys::Lock m_lock;
    std::string m_filename;
};

#endif	/* LUASTATE_H */

