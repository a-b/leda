/* 
* File:   HttpServer.cpp
 * Author: sergey
 * 
 * Created on January 4, 2014, 5:08 AM
 */

#include "HttpServer.h"
#include "Leda.h"

HttpServer::HttpServer()
: m_threadId( 0 )
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
}

void HttpServer::onThreadStopped( const propeller::Server::Thread& thread )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = LuaState::luaFromThread( thread, thread.id() );
    
    lua.call( "onThreadStopped" );
    
    delete &lua;
}

void HttpServer::onThreadStarted( propeller::Server::Thread& thread )
{
    TRACE_ENTERLEAVE();
    
    sys::LockEnterLeave lock( m_lock );
    
    LuaState* lua = LuaState::luaForThread( thread, thread.id() );    
    lua->call( "onThreadStarted" );
}

void HttpServer::onRequest( const propeller::http::Request& request, propeller::http::Response& response,  const propeller::Server::Thread& thread )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = LuaState::luaFromThread( thread, thread.id() );
    
    //
    //  export request and response pointers to lua
    //
    lua.setGlobal( "httpRequest", ( void* ) &request );
    lua.setGlobal( "httpResponse", ( void* ) &response );
    
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
