#include "Server.h"
#include "Leda.h"

Server::Server( propeller::Server::Type type )
: propeller::Server( type )
{
    TRACE_ENTERLEAVE();
    
    TRACE( "created server type %d", type );
    
}

void Server::stop( )
{
    TRACE_ENTERLEAVE();
    propeller::Server::stop();
 
    
    //
    //  wait on the stop semaphore
    //
    m_stop.wait();
}

//
//  thread has been started
//
void Server::onThreadStarted( propeller::Server::Thread& thread )
{
    TRACE_ENTERLEAVE();
    sys::LockEnterLeave lock( m_lock );
    
    LuaState* lua = LuaState::luaForThread( thread, thread.id() );    
    lua->call( "onThreadStarted" );
}


void Server::onThreadStopped( const propeller::Server::Thread& thread )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = LuaState::luaFromThread( thread, thread.id() );
    
    lua.call( "onThreadStopped" );
    
    m_stop.post();
}

void Server::onConnectionAccepted( const propeller::Server::Connection& connection )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = LuaState::luaFromThread( connection.thread(), connection.thread().id() );
    
    lua.setGlobal( "connection", ( void* ) &connection );
    lua.call( "onConnectionAccepted" );

}

void Server::onDataReceived( const propeller::Server::Connection& connection, const char* data, unsigned int length )
{
    TRACE_ENTERLEAVE();
    LuaState& lua = LuaState::luaFromThread( connection.thread(), connection.thread().id() );
    
    lua.setGlobal( "connection", ( void* ) &connection );
    lua.setGlobal( "data", data, length );
    
    lua.call( "onConnectionDataReceived" ); 
 }

void Server::onDataReceived( const propeller::Server::Thread& thread, const std::string& from, const char* data, unsigned int length )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = LuaState::luaFromThread( thread, thread.id() );

    
    lua.setGlobal( "data", data, length );
    lua.setGlobal(  "from", from );

    lua.call( "onUdpDataReceived" );        
}


void Server::onConnectionClosed( const propeller::Server::Connection& connection )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = LuaState::luaFromThread( connection.thread(), connection.thread().id() );
    lua.setGlobal( "connection", ( void* ) &connection );

    lua.call( "onConnectionClosed" );
}

void Server::onTimer( const propeller::Server::Thread& thread, void* data )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = LuaState::luaFromThread( thread, thread.id() );
    
    if ( data )
    {
        Leda::instance()->callTimer( lua, ( Leda::TimerData* ) data );
    }
}

void Server::addTimer( lua_State* lua, unsigned int timeout, bool once, void* data )
{
    TRACE_ENTERLEAVE();
    
    unsigned int threadId = LuaState::getThreadId( lua );
    
    TRACE( "adding timer to thread id %d", threadId );
    propeller::Server::addTimer( timeout, threadId, once, data );
}

Server::~Server( )
{
}


