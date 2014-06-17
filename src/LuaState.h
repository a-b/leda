#ifndef LUASTATE_H
#define	LUASTATE_H

//
//  include lua
//
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
    LuaState();
    LuaState( const LuaState& lua );
    virtual ~LuaState( );
    
    bool load( const std::string& script, const char* init = NULL );
    void call( const std::string& callbackName, int registryIndex = -1, bool exception = false) const; 
    bool reload( const std::string& script );
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
     void create();   
     
    
private:
    lua_State* m_lua;
    bool m_close;    
    sys::Lock m_lock;
};

#endif	/* LUASTATE_H */

