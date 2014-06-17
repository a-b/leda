/* 
 * File:   LuaState.cpp
 * Author: sergey
 * 
 * Created on January 3, 2014, 8:36 AM
 */

#include "LuaState.h"
#include "Leda.h"


LuaState::LuaState( )
: m_lua( NULL ), m_close( true )
{
    TRACE_ENTERLEAVE();
    
}

LuaState::LuaState( const LuaState& lua)
: m_lua( lua.m_lua ), m_close( true )
{
    m_lua = lua.m_lua;
    
}

void LuaState::create()
{
    TRACE_ENTERLEAVE();
            
    if ( !m_lua )
    {
        TRACE( "creating new lua state", "" );
        
        //
        //  create new lua state
        //
        m_lua = luaL_newstate( );
        
        TRACE( "%0x", m_lua );

        // //
        // //  register api functons
        // //
        // static const luaL_Reg functions[] = {
        //     {"serverAddTimer", serverAddTimer },
        //     {"serverCreate", serverCreate },
        //     {"generateUniqueString", generateRandomString },
        //     {"serverConnectionSendData", serverConnectionSendData },
        //     {"serverConnectionSendMessage", serverConnectionSendMessage },
        //     {"clientCreate", clientCreate },
        //     {"clientConnect", clientConnect },
        //     {"clientAddTimer", clientAddTimer },
        //     {"clientConnectionSendMessage", clientConnectionSendMessage },
        //     {"clientConnectionClose", clientConnectionClose },
        //     {"getpid", getpid },
        // 
        //     {NULL, NULL }
        // };
        // 
        // luaL_register( m_lua, "breezeApi", functions );
        
    }
    

    luaL_openlibs( m_lua );
    
    
    //
    // modify package search path
    //
    lua_getglobal( m_lua, "package" );
    lua_getfield( m_lua, -1, "path" );

    std::string path = lua_tostring( m_lua, -1 );

    for ( std::list< std::string >::const_iterator i = Leda::instance()->paths().begin( ); i != Leda::instance()->paths().end( ); i++ )
    {
        path.append( ";" );
        path.append( i->c_str( ) );
        path.append( "/?.lua" );

        path.append( ";" );
        path.append( i->c_str( ) );
        path.append( "/?/init.lua" );
    }

    TRACE( "lua library path: %s", path.c_str( ) );

    lua_pop( m_lua, 1 );
    lua_pushstring( m_lua, path.c_str( ) );
    lua_setfield( m_lua, -2, "path" );
    lua_pop( m_lua, 1 );
 }

LuaState::~LuaState( )
{
    TRACE_ENTERLEAVE();

    if ( m_lua && m_close )
    {
        //
        //  close lua state if it was created by this instance
        //
        lua_close( m_lua );
    }
}

void LuaState::execute( const std::string& script ) const
{
    luaL_dostring( m_lua, script.c_str() );
}


void LuaState::call( const std::string& callbackName, int registryIndex, bool exception ) const
{
    TRACE_ENTERLEAVE();
    
    if ( !m_lua )
    {
        throw std::runtime_error("cannot call without a loaded script first");
    }
            
    //
    //  load debug.traceback
    //  
    lua_getglobal( m_lua, "debug" );
    lua_getfield( m_lua, -1, "traceback" );
    
    //
    //  load callback function to the top of the stack
    //
    if ( registryIndex < 0 )
    {
        //
        //  by global name
        //  
        lua_getglobal( m_lua, callbackName.c_str() );
    }
    else
    {
        //
        //  by registry index
        //  
        lua_rawgeti( m_lua, LUA_REGISTRYINDEX, registryIndex );
    }
    
    if ( lua_isnil( m_lua, -1 ) )
    {
        TRACE( "%s is not defined in lua", callbackName.c_str() );
        lua_pop( m_lua, 1 );
        return;
    }
    //
    //  call lua callback function
    //
    int result = lua_pcall( m_lua, 0, 0, -2 );

    if ( result > 0 )
    {
        //
        //  error executing lua
        //
        const char* error = lua_tostring( m_lua, -1 );
        
        
        
        TRACE_ERROR( "%s", error );
        
        
        lua_pop( m_lua, 3 );
        
        std::string errorString;
        if ( error )
        {
            errorString = error;
        }
        
        if ( exception )
        {
            throw std::runtime_error( errorString );
        }
        
        return;
    }
    
    //
    //  remove debug.traceback from stack
    //
    lua_pop( m_lua, 2 );
}


bool LuaState::reload( const std::string& script ) 
{
    TRACE_ENTERLEAVE();
    
    if ( !m_lua )
    {
        throw std::runtime_error("cannot reload without a loaded script first");
    }
    
    luaL_dostring( m_lua, "for key, value in pairs(package.loaded) do package.loaded[key] = nil end" );
    create( );
    
    //
    //  reload script
    //
    return load( script );
}

bool LuaState::load(const std::string& filename, const char* init )
{
    TRACE_ENTERLEAVE();
    
    if ( !m_lua )
    {
        create();
    }
    
    //
    //  perform custom initilization if needed
    //    
    if ( init )
    {
        luaL_dostring( m_lua, init );
    }
    
    //
    //  load moonscript environment
    //
    char script[ 256 ];
    sprintf( script, "local loader = require 'moonscript.loader'; return loader.load('%s')", filename.c_str() );
    
    TRACE( "executing %s", script );
    
//    //
//    //  debug.traceback function
//    //
//    lua_getglobal( m_lua, "debug");
//    lua_getfield( m_lua, -1, "traceback");
//    
//    int res = luaL_loadfile( m_lua, filename.c_str() );
//
//    if ( res == 0 )
//    {
    int res = luaL_dostring( m_lua, script );
    bool success = false;

    if ( res )
    {
        TRACE_ERROR( "%s", lua_tostring( m_lua, -1 ) );
    }
    else
    {
        //
        //  remove debug.traceback from stack
        //  
        success = lua_toboolean( m_lua, -1 );
        TRACE( "call result %d", success );
    }
    //
    //  pop result or error from the stack
    //
    lua_pop( m_lua, 1 );
    
//    }
//    else
//    {
//        const char* error = lua_tostring( m_lua, -1 );
//        TRACE_ERROR( "%s", error );
//        lua_pop( m_lua, 3 );
//        
//        return false;
//    }
    
    return success;
}