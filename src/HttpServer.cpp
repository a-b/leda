/* 
 * File:   HttpServer.cpp
 * Author: sergey
 * 
 * Created on January 4, 2014, 5:08 AM
 */

#include "HttpServer.h"
#include "Breeze.h"

HttpServer::HttpServer()
: m_threadId( 0 ), m_stoppedThreads( 0 )
{
    BREEZE_TRACE_ENTERLEAVE();
    
}

void HttpServer::start(  )
{
    BREEZE_TRACE_ENTERLEAVE();
    
    
    propeller::http::Server::start();
}

void HttpServer::stop(  )
{
    BREEZE_TRACE_ENTERLEAVE();
    propeller::http::Server::stop();
    
    m_stop.wait();
}

void HttpServer::onThreadStop( const sys::ThreadPool::Worker& thread )
{
    BREEZE_TRACE_ENTERLEAVE();
    
    {
        LuaState& lua = *( ( LuaState* ) thread.data() );
        sys::LockEnterLeave lock( lua.lock() );
        Breeze::instance()->callTerminate( lua );
    }
    
    unsigned int old = sys::General::interlockedIncrement( &m_stoppedThreads );
    if ( old == getPoolThreadCount() - 1  )
    {
        BREEZE_TRACE("all pool threads stopped", "");
        m_stop.post();
    }
}

void HttpServer::onThreadStart( sys::ThreadPool::Worker& thread )
{
    BREEZE_TRACE_ENTERLEAVE();
    
    char initString[64];
    sprintf( initString, "breezeApi.threadId=%d", m_threadId );
    LuaState* lua = new LuaState();
    lua->load( Breeze::instance()->script(), initString );
    thread.setData( lua );
    m_threadId++;

}

void HttpServer::onRequest( const propeller::http::Request& request, propeller::http::Response& response,  sys::ThreadPool::Worker& thread )
{
    BREEZE_TRACE_ENTERLEAVE();
    
    LuaState& lua = *( ( LuaState* ) thread.data() );
    sys::LockEnterLeave lock( thread.lock() );
    
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
        if ( Breeze::instance()->debug() )
        {
            //
            //  reload script if needed
            //
            if ( !lua.reload( Breeze::instance()->script() ) )
            {
                throw std::runtime_error("error parsing script");
            }
        }
        
        lua.call( "__onHttpRequest", -1, true );
    }
    
    catch ( std::runtime_error& e )
    {
        response.setStatus( 500 );
        
        if ( Breeze::instance()->debug() )
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
    lua_getglobal( lua, "__httpResponse" );

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
    
    lua_pop( lua, 1 );
}

void HttpServer::onTimer( void* data )
{
    BREEZE_TRACE_ENTERLEAVE();
        
    for ( sys::ThreadPool::WorkerList::iterator i =  threads().begin(); i != threads().end(); i++ )
    {

        sys::ThreadPool::Worker* worker = *i;
        BREEZE_TRACE("need to lock thread %d", worker->workerId());

        sys::LockEnterLeave lock(worker->lock());
        sys::LockEnterLeave lock1(Breeze::instance()->lua().lock());
        LuaState& lua = *((LuaState*) worker->data());
        lua.call("__onCollectMetrics");


        lua_getglobal(lua, "__metrics");
        lua_pushnil(lua);
        lua_newtable(Breeze::instance()->lua());

        while ( lua_next( lua, -2 ) ) 
        {
            if ( lua_isnumber( lua, -1 ) ) 
            {
                lua_pushnumber( Breeze::instance()->lua(), lua_tonumber( lua, -1 ) );
                lua_setfield( Breeze::instance()->lua(), -2, lua_tostring( lua, -2 ) );

            }
           lua_pop(lua, 1);
        }
        lua_pop( lua, 1 );
        
        lua_setglobal( Breeze::instance()->lua(), "__threadMetrics" );
        
        Breeze::instance()->lua().execute("if not __totalMetrics then __totalMetrics = table.clone(__threadMetrics) else "
                "for k, v in pairs(__threadMetrics) do  __totalMetrics[k] = __totalMetrics[k] + v  end end;");
    }
    
    
    
    sys::LockEnterLeave lock( Breeze::instance()->lua().lock() );
    Breeze::instance()->lua().call( "__onCollectMetricsTotals" );
    
}

void HttpServer::onStarted( )
{
    BREEZE_TRACE_ENTERLEAVE();
    
    
    
    addTimer( 1, false, NULL );
    
    
    
}