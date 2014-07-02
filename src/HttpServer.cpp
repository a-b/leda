/* 
* File:   HttpServer.cpp
 * Author: sergey
 * 
 * Created on January 4, 2014, 5:08 AM
 */

#include "HttpServer.h"
#include "Leda.h"

HttpServer::HttpServer()
: m_stoppedThreads( 0 ), m_threadId( 0 )
{
    TRACE_ENTERLEAVE();
    
}

void HttpServer::start(  )
{
    TRACE_ENTERLEAVE();
    
    
    propeller::http::Server::start();
}

void HttpServer::stop(  )
{
    TRACE_ENTERLEAVE();
    propeller::http::Server::stop();
    
    m_stop.wait();
}

void HttpServer::onThreadStopped( const propeller::Server::Thread& thread )
{
    TRACE_ENTERLEAVE();
    
    LuaState* lua = ( LuaState* ) thread.data();
    lua->call( "onThreadStopped" );

    unsigned int value = sys::General::interlockedIncrement( &m_stoppedThreads );
    if ( value == getThreadCount() )
    {
        lua->call( "onServerStopped" );
        
        m_stop.post();
    }
}

void HttpServer::onThreadStarted( propeller::Server::Thread& thread )
{
    TRACE_ENTERLEAVE();
    
        
    
    LuaState* lua = new LuaState( Leda::instance()->script() );
    lua->load();
    lua->setGlobal( "__threadId", thread.id() );
    thread.setData( lua );
    lua->call( "onThreadStarted" );
    
    if ( !m_threadId )
    {
        lua->call( "onServerStarted" );
    }
}



void HttpServer::onRequest( const propeller::http::Request& request, propeller::http::Response& response,  const propeller::Server::Thread& thread )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = LuaState::luaFromThread( thread, thread.id() );
    
//    //
//    //  export request to lua
//    //
//    lua_newtable( lua );
//        
//    if ( request.body() && request.bodyLength() > 0 )
//    {
//        lua_pushlstring( lua, request.body(), request.bodyLength() );
//    }
//    else
//    {
//        lua_pushnil( lua );
//    }
//   
//    lua_setfield( lua, -2, "body" );
//    
//    lua_pushstring( lua, request.uri() );
//    lua_setfield( lua, -2, "url" );
//    
//    lua_pushstring( lua, request.method() );
//    lua_setfield( lua, -2, "method" );
//    
//    l
//    
//    lua_setfield( lua, -2, "headers" );
//
//    lua_setglobal( lua, "__httpRequest" );
    
    //
    //  export request and response pointers to lua
    //
    lua_getglobal( lua, "__leda" );
    lua_pushlightuserdata( lua, ( void* ) &request );
    lua_setfield( lua, -2, "httpRequest" );
    
    lua_pushlightuserdata( lua, ( void* ) &response );
    lua_setfield( lua, -2, "httpResponse" );
    
    lua_pop( lua, 1 );
    
    
    //
    //  call lua 
    //
    try
    {
        lua.call( "onHttpRequest", -1, true );
    }
    
    catch ( std::runtime_error& e )
    {
        response.setStatus( 500 );
        
        if ( Leda::instance()->debug() )
        {
            response.setBody( e.what() );
        }
        else
        {
            response.setBody( "" );
        }
        
        return;
    }

}

