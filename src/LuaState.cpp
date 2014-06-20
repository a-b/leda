/* 
 * File:   LuaState.cpp
 * Author: sergey
 * 
 * Created on January 3, 2014, 8:36 AM
 */

#include "LuaState.h"
#include "Leda.h"


LuaState::LuaState( const std::string& filename )
: m_lua( NULL ), m_close( true ), m_filename( filename )
{
    TRACE_ENTERLEAVE();
    

    TRACE( "creating new lua state", "" );

    //
    //  create new lua state
    //
    m_lua = luaL_newstate( );

    TRACE( "%0x", m_lua );

    // //
    // //  register api functons
    // //
    static const luaL_Reg functions[] = {
        {"serverAddTimer", serverAddTimer },
        {"serverCreate", serverCreate },
//        {"generateUniqueString", generateRandomString },
        {"serverConnectionSendData", serverConnectionSendData },
        {"serverConnectionSendMessage", serverConnectionSendMessage },
        {"threadGetId", threadGetId },
        {"serverConnectionGetAddress", serverConnectionGetAddress},
        {"serverConnectionGetId", serverConnectionGetId},
        
        //             {"clientCreate", clientCreate },
        //             {"clientConnect", clientConnect },
        //             {"clientAddTimer", clientAddTimer },
        //             {"clientConnectionSendMessage", clientConnectionSendMessage },
        //             {"clientConnectionClose", clientConnectionClose },
        {"getpid", getpid },

        {NULL, NULL }
    };

    luaL_register( m_lua, "__api", functions );
    
    loadlibs();
}

LuaState::LuaState( const LuaState& lua)
: m_lua( lua.m_lua ), m_close( true )
{
    m_lua = lua.m_lua;
    
}

void LuaState::addPaths( const char* name ) const
{
    TRACE_ENTERLEAVE();
    
    //
    // modify package search path
    //
    lua_getglobal( m_lua, "package" );
    lua_getfield( m_lua, -1, name );
    
    std::string path = lua_tostring( m_lua, -1 );
    
    std::string initName =  path.substr( path.find_last_of( "/") ).c_str();
    const char* ext = strstr( initName.c_str(), "." );
    
    

    for ( std::list< std::string >::const_iterator i = Leda::instance()->paths().begin( ); i != Leda::instance()->paths().end( ); i++ )
    {
        path.append( ";" );
        path.append( i->c_str( ) );
        path.append( "/?" );
        path.append( ext );
        
        
        path.append( ";" );
        path.append( i->c_str( ) );
        path.append( "/?" );
        path.append( initName );
    }

    TRACE( "%s: %s", name, path.c_str( ) );
    lua_pop( m_lua, 1 );
    lua_pushstring( m_lua, path.c_str( ) );
    lua_setfield( m_lua, -2, name );
    lua_pop( m_lua, 1 );
}


void LuaState::loadlibs() const
{
    TRACE_ENTERLEAVE();
            
    luaL_openlibs( m_lua );
    
    
    
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
    
    if ( Leda::instance()->debug() )
    {
        reload();
    }
            
    //
    //  load debug.traceback
    //  
    lua_getglobal( m_lua, "debug" );
    lua_getfield( m_lua, -1, "traceback" );
    
    
    int debugIndex = -2;
    //
    //  load callback function to the top of the stack
    //
    if ( registryIndex < 0  )
    {
        //
        //  push function identified by global name to the stack
        //  
        lua_getglobal( m_lua, "__leda" );
        lua_getfield( m_lua, -1, callbackName.c_str() );
        debugIndex = -3;
    }
    else
    {
        //
        //  push function identified by registry index to the stack
        //  
        lua_rawgeti( m_lua, LUA_REGISTRYINDEX, registryIndex );
    }
    
    if ( !lua_isfunction( m_lua, -1 ) )
    {
        TRACE( "no lua function found on top of stack", "" );
        
        lua_pop( m_lua, 3 );
        return;
    }
    //
    //  call lua callback function
    //
    int result = lua_pcall( m_lua, 0, 0, debugIndex );
    
    TRACE( "lua call result: %d", result );

    if ( result > 0 )
    {
        //
        //  error executing lua
        //
        const char* error = lua_tostring( m_lua, -1 );
        
        
        TRACE_ERROR( "%s", error );
        
        lua_pop( m_lua, -debugIndex + 1 );
        
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
    //  pop all from stack
    //
    lua_pop( m_lua, -debugIndex );
}


bool LuaState::reload( ) const
{
    TRACE_ENTERLEAVE();
    
    if ( !m_lua )
    {
        throw std::runtime_error("cannot reload without a loaded script first");
    }
    
    luaL_dostring( m_lua, "for key, value in pairs(package.loaded) do package.loaded[key] = nil end" );
    loadlibs( );

    //
    //  reload script
    //
    return load( );
}

bool LuaState::load( const char* init ) const
{
    TRACE_ENTERLEAVE();
    
    //
    //  perform custom initilization if needed
    //    
    if ( init )
    {
        TRACE( "%s", init );
        luaL_dostring( m_lua, init );
    }
    
    addPaths( "path" );

    luaL_dostring( m_lua, "require 'leda'");
    luaL_dostring( m_lua, "require 'moonscript';");
    
    addPaths( "moonpath" );
    
    //
    //  load moonscript environment
    //
    char script[ 256 ];
    sprintf( script, "local moonscript = require('moonscript'); local init = moonscript.loadstring(\"require 'leda.init'\"); init(); "
            "local code = assert(moonscript.loadfile('%s')); if code then code() return true; end; return false;", m_filename.c_str() ); 
    

    TRACE( "executing %s", script );
    
    //
    //  debug.traceback function
    //
    lua_getglobal( m_lua, "debug");
    lua_getfield( m_lua, -1, "traceback");

    luaL_loadstring( m_lua, script ); 
    bool success = false;
    
    int res = lua_pcall( m_lua, 0, LUA_MULTRET, -2 );

    if ( res )
    {
        TRACE_ERROR( "%s", lua_tostring( m_lua, -1 ) );
        success = false;
    }
    else
    {
        //
        //  remove debug.traceback from stack
        //  
        success = lua_toboolean( m_lua, -1 );
    }
    //
    //  pop result or error from the stack
    //
    lua_pop( m_lua, 3 );
    
   return success;
}