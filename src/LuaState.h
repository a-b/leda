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
    
    bool load( const char* init = NULL ) const;
    void call( const std::string& callbackName, int registryIndex = -1, bool exception = false) const; 
    bool reload( ) const;
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
    
    
private:
     void loadlibs() const;   
     void addPaths( const char* name ) const;
     
    
private:
    lua_State* m_lua;
    bool m_close;    
    sys::Lock m_lock;
    std::string m_filename;
};

#endif	/* LUASTATE_H */

