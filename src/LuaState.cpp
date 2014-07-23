/* 
 * File:   LuaState.cpp
 * Author: sergey
 * 
 * Created on January 3, 2014, 8:36 AM
 */

#include "LuaState.h"
#include "Leda.h"
#include "api.h"


sys::Lock LuaState::s_lock;

LuaState::LuaState( const std::string& filename, const ScriptArguments* arguments  )
: m_lua( NULL ), m_close( true ), m_filename( filename )
{
    TRACE_ENTERLEAVE();
    
    if ( arguments )
    {
        m_arguments = *arguments;
    }
    
    create();
    
}

LuaState::LuaState( const LuaState& lua)
: m_lua( lua.m_lua ), m_close( true )
{
    m_lua = lua.m_lua;
    
}

void LuaState::destroy()
{
    TRACE_ENTERLEAVE();
    
    if ( m_lua && m_close )
    {
        //
        //  close lua state if it was created by this instance
        //
        lua_close( m_lua );
        
        m_lua = NULL;
    }
}


void LuaState::create()
{
    if ( m_lua )
    {
        return;
    }
    
    TRACE_ENTERLEAVE();
    
    //
    //  create new lua state
    //
    m_lua = luaL_newstate( );

    // //
    // //  register api functons
    // //
    static const luaL_Reg functions[] = {
        {"serverAddTimer", serverAddTimer },
        {"serverCreate", serverCreate },
        {"serverConnectionSendData", serverConnectionSendData },
        {"serverConnectionGetAddress", serverConnectionGetAddress},
        {"serverConnectionGetId", serverConnectionGetId},
        {"serverSendTo", serverSendTo},
        {"clientCreate", clientCreate },
        {"clientConnect", clientConnect },
        {"clientAddTimer", clientAddTimer },
        {"clientConnectionSendData", clientConnectionSendData },
        {"clientConnectionClose", clientConnectionClose },
        {"getpid", getpid },
        {"processorCount", processorCount},
        {"httpRequestGetUrl", httpRequestGetUrl},
        {"httpRequestGetHeaders", httpRequestGetHeaders},
        {"httpRequestGetBody", httpRequestGetBody},
        {"httpRequestGetMethod", httpRequestGetMethod},
        {"httpRequestGetAddress", httpRequestGetAddress},
        {"httpResponseSetBody", httpResponseSetBody},
        {"httpResponseSetStatus", httpResponseSetStatus},
        {"httpResponseSetHeaders", httpResponseSetHeaders},
        {"httpResponseAddHeader", httpResponseAddHeader},
        {"getVersion", getVersion},
        {"dictionaryGet", dictionaryGet},
        {"dictionarySet", dictionarySet},
        {"dictionaryRemove", dictionaryRemove},
        {"dictionaryGetKeys", dictionaryGetKeys},
        {"processStart", processStart},
        {"processWrite", processWrite},
        
        {NULL, NULL }
    };

    luaL_register( m_lua, "__api", functions );
    
    loadlibs();
        
}

void LuaState::addPaths( const char* name ) const
{
    TRACE_ENTERLEAVE();
    
    //
    // modify package search path
    //
    lua_getglobal( m_lua, "package" );
    lua_getfield( m_lua, -1, name );
    
    if ( lua_isnil( m_lua, -1 ) )
    {
        TRACE( "package.%s is not set in lua", name );
        lua_pop( m_lua, 1 );
        return;
    }
    
    std::string path = lua_tostring( m_lua, -1 );
    
    std::string separator = "/";
#ifdef WIN32
    separator = "\\";
#endif
    
    std::string initName =  path.substr( path.find_last_of( separator ) );
    const char* ext = strstr( initName.c_str(), "." );
    
    for ( std::list< std::string >::const_iterator i = Leda::instance()->paths().begin( ); i != Leda::instance()->paths().end( ); i++ )
    {
        path.append( ";" );
        path.append( *i );
        path.append( separator );
        path.append( "?" );
        path.append( ext );
        
        
        path.append( ";" );
        path.append( *i );
        path.append( separator );
        path.append( "?" );
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
    
    destroy();    
}

void LuaState::execute( const std::string& script ) const
{
    TRACE_ENTERLEAVE();
    TRACE( "%s", script.c_str() );
    
     //
    //  debug.traceback function
    //
    lua_getglobal( m_lua, "debug");
    lua_getfield( m_lua, -1, "traceback");

    luaL_loadstring( m_lua, script.c_str() ); 
    
    int res = lua_pcall( m_lua, 0, LUA_MULTRET, -2 );

    if ( res )
    {
        std::string error = lua_tostring( m_lua, -1 ); 
        lua_pop( m_lua, 3 );
        
        TRACE_ERROR( "%s", error.c_str() );
        
        throw std::runtime_error( error );
    }
    
    //
    //  pop result or error from the stack
    //
    lua_pop( m_lua, 2 );
}


void LuaState::call( const std::string& callbackName, int registryIndex, bool exception ) 
{
    
    TRACE_ENTERLEAVE();
    
    TRACE( "callback name: %s, callback index %d", callbackName.c_str(), registryIndex );
    
            
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
        
        lua_pop( m_lua, -debugIndex + 1 );
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


void LuaState::reload( unsigned int threadId )
{
    TRACE_ENTERLEAVE();
    
    if ( Leda::instance()->debug() && Leda::instance()->changes() > 0 )
    {
        if ( !m_lua )
        {
            throw std::runtime_error( "cannot reload without a loaded script first" );
        }
        
        TRACE("have %d filesystem changes, need to reload lua", Leda::instance()->changes() );
        destroy();
        create();
        loadlibs();
        load();
        setGlobal( "threadId", threadId );
        
        Leda::instance()->resetChanges();
    }
}


 void LuaState::load( unsigned int threadId, bool exception, const char* init ) 
{
    TRACE_ENTERLEAVE();
    
    addPaths( "path" );
    addPaths( "cpath" );
    
    
    char script[ 1024 ];
    
    TRACE( "loading %s", m_filename.c_str() );
    setGlobal( "threadId", threadId );
    
    try
    {
        execute( "require 'leda'" );
        if ( init )
        {
            execute( init );
        }    
    }
    catch ( const  std::runtime_error& e )
    {
        if ( exception )
        {
            throw e;
        }
    }
    setArguments( );
    
    if ( m_filename.find( ".moon") != std::string::npos )
    {
        //
        //  moonscript
        //
        try
        {
            execute( "require 'moonscript'");
        }
        catch ( const std::runtime_error& e )
        {
            if ( exception )
            {
                throw e;
            }
            
            return;
        }
        
        sprintf( script, "local moonscript = require('moonscript'); "
        "moonscript.dofile('%s');", m_filename.c_str() ); 
    }
    else
    {
        //
        //  lua
        //
        sprintf( script,"dofile('%s');", m_filename.c_str() ); 
    }

    try
    {
        execute( script );
    }
    catch( const std::runtime_error& e )
    {
        if ( exception )
        {
            throw e;
        }
    }
    
   
   
}

void LuaState::setGlobal( const std::string& name, unsigned int value )
{
    TRACE_ENTERLEAVE();
    getGlobalTable();
    lua_pushinteger( m_lua, value );
    lua_setfield( m_lua, -2, name.c_str() );
    lua_pop( m_lua, 1 );
}


void LuaState::setGlobal( const std::string& name, void* value )
{
    getGlobalTable();
    lua_pushlightuserdata( m_lua, value );
    lua_setfield( m_lua, -2, name.c_str()  );
    lua_pop( m_lua, 1 );
}

void LuaState::setGlobal( const std::string& name, const char* value, unsigned int length )
{
    getGlobalTable();
    lua_pushlstring( m_lua, value, length );
    lua_setfield( m_lua, -2, name.c_str()  );
    lua_pop( m_lua, 1 );
}

void LuaState::setGlobal( const std::string& name, const std::string& value )
{
    getGlobalTable();
    lua_pushstring( m_lua, value.c_str() );
    lua_setfield( m_lua, -2, name.c_str()  );
    lua_pop( m_lua, 1 );
}


void LuaState::setGlobal( const std::string& name, bool value )
{
    getGlobalTable();
    lua_pushboolean( m_lua, ( int ) value );
    lua_setfield( m_lua, -2, name.c_str()  );
    lua_pop( m_lua, 1 );
}

void LuaState::setArguments(  )
{
    unsigned int index = 1;
    lua_newtable( m_lua );
    for ( ScriptArguments::const_iterator i = m_arguments.begin(); i != m_arguments.end(); i++ )
    {
        lua_pushnumber( m_lua, index );
        lua_pushstring( m_lua, i->c_str() );
        lua_settable( m_lua, -3 );
        index ++;
    }
    
    lua_setglobal( m_lua, "arg" );
}


LuaState& LuaState::luaFromThread( const sys::Thread& thread, unsigned int threadId )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = *( ( LuaState* ) thread.data() );
    lua.reload( threadId );
    return lua;
}

LuaState* LuaState::luaForThread( sys::Thread& thread, unsigned int id, const char* init )
{
    sys::LockEnterLeave lock( s_lock );
    
    //
    //  create new lua state for new thread
    //  
    LuaState* lua = Leda::instance()->newLua();
    lua->load( id, false, init );
    thread.setData( lua ); 
    
    return lua;
}


void LuaState::getGlobalTable()
{
    lua_getglobal( m_lua, "__leda" );
    if ( lua_isnil( m_lua, -1 ) )
    {
        lua_pop( m_lua, 1 );
        //
        //  create new table if not set
        //
        lua_newtable( m_lua );
        lua_setglobal( m_lua, "__leda" );
        getGlobalTable();
    }
}

unsigned int LuaState::getGlobal( const std::string& name )
{
    unsigned int result = 0;
    
    getGlobalTable();
    
    if ( lua_istable( m_lua, -1 ) )
    {
        lua_getfield( m_lua, -1, name.c_str() );
        result = lua_tointeger( m_lua, -1 );
        lua_pop( m_lua, 1 ); 
    }
    lua_pop( m_lua, 1 ); 

    return result;
}
    
