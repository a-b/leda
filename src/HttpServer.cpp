/* 
 * File:   HttpServer.cpp
 * Author: sergey
 * 
 * Created on January 4, 2014, 5:08 AM
 */

#include "HttpServer.h"
#include "Leda.h"

HttpServer::HttpServer()
: m_stoppedThreads( 0 ), m_threadId( 0 ), m_poolThreadId( 0 )
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

void HttpServer::onThreadStop( const sys::ThreadPool::Worker& thread )
{
    TRACE_ENTERLEAVE();
    
    onThreadStopCommon( thread );
}

void HttpServer::onThreadStopCommon( const sys::Thread& thread )
{
    LuaState* lua = ( LuaState* ) thread.data();
    lua->call( "onThreadStopped" );

    unsigned int value = sys::General::interlockedIncrement( &m_stoppedThreads );
    if ( value == getPoolThreadCount() +  getThreadCount() )
    {
        lua->call( "onServerStopped" );
        
        m_stop.post();
    }
}
void HttpServer::onThreadStopped( const propeller::Server::Thread& thread )
{
    TRACE_ENTERLEAVE();
    
    onThreadStopCommon( thread );
}

LuaState* HttpServer::onThreadStartCommon( sys::Thread& thread, unsigned int id )
{
    LuaState* lua = new LuaState( Leda::instance()->script() );
    lua->load();
    lua->setGlobal( "__threadId", id );
    thread.setData( lua );
    lua->call( "onThreadStarted" );
    
    return lua;
}

void HttpServer::onThreadStart( sys::ThreadPool::Worker& thread )
{
    TRACE_ENTERLEAVE();
    onThreadStartCommon( thread, m_poolThreadId );
    
    
    m_poolThreadId ++;
    
}

void HttpServer::onThreadStarted( propeller::Server::Thread& thread )
{
    TRACE_ENTERLEAVE();
    
    LuaState* lua = onThreadStartCommon( thread, m_threadId );
    
    if ( !m_threadId )
    {
        lua->call( "onServerStarted" );
    }

    
    m_threadId ++;
    
    
}

void HttpServer::onRequest( const propeller::http::Request& request, propeller::http::Response& response,  sys::ThreadPool::Worker& thread )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = *( ( LuaState* ) thread.data() );
    
    //
    //  export request to lua
    //
    lua_newtable( lua );
        
    if ( request.body() && request.bodyLength() > 0 )
    {
        lua_pushlstring( lua, request.body(), request.bodyLength() );
    }
    else
    {
        lua_pushnil( lua );
    }
   
    lua_setfield( lua, -2, "body" );
    
    lua_pushstring( lua, request.uri() );
    lua_setfield( lua, -2, "url" );
    
    lua_pushstring( lua, request.method() );
    lua_setfield( lua, -2, "method" );
    
    lua_newtable( lua );
    
    for ( propeller::http::Request::HeaderList::const_iterator i = request.headers().begin(); i != request.headers().end(); i++ )
    {
        propeller::http::Request::Header* header = *i;
        
        lua_pushstring( lua, header->value );
        lua_setfield( lua, -2, header->name );
    }
    
    lua_setfield( lua, -2, "headers" );
    lua_setglobal( lua, "__httpRequest" );
    
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

    //
    //  load response from lua
    //
    lua_getglobal( lua, "__leda" );
    lua_getfield( lua, -1, "httpResponse" );

    lua_getfield( lua, -1, "status" );
    response.setStatus( lua_tonumber( lua, -1 ) );
    lua_pop( lua, 1 );
    
    lua_getfield( lua, -1, "headers" );

    lua_pushnil( lua );
    while ( lua_next( lua, -2 ) != 0 )
    {
        const char* name = lua_tostring( lua, -2 );
        const char* value = lua_tostring( lua, -1 );

        response.addHeader( name, value );

        lua_pop( lua, 1 );
    }
    
    lua_pop( lua, 1 );
    
    lua_getfield( lua, -1, "body" );
    size_t length = 0;
    const char* body = lua_tolstring( lua, -1, &length );

    response.setBody( body, length );
    lua_pop( lua, 1 );
    
    lua_pop( lua, 2 );
}

void HttpServer::addTimer( lua_State* lua, unsigned int timeout, bool once, void* data )
{
    TRACE_ENTERLEAVE();
    
    lua_getglobal( lua, "__threadId" );
    
    unsigned int threadId = lua_tonumber( lua, -1 );
    
    TRACE( "adding timer to thread id %d", threadId );
    propeller::Server::addTimer( timeout, threadId, once, data );
    lua_pop( lua, 1 );
}

void HttpServer::onTimer( const propeller::Server::Thread& thread, void* data )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = *( ( LuaState* ) thread.data() );
    
    if ( data )
    {
        Leda::instance()->callTimer( lua, ( Leda::TimerData* ) data );
    }
}
