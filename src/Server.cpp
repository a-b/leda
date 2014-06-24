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
    
    //
    //  create new lua state for new thread
    //  
    LuaState* lua = new LuaState( Leda::instance()->script() );
    lua->load( );

    //
    //  store thread pointer in lua state
    //
    lua->setGlobal( "__threadId", thread.id() );
    thread.setData( lua ); 
    
    lua->call( "onThreadStarted" );
}


void Server::onThreadStopped( const propeller::Server::Thread& thread )
{
    TRACE_ENTERLEAVE();
    
    LuaState* lua =  ( LuaState* ) thread.data();
    
    lua->setGlobal( "__threadId", thread.id() );
    lua->call( "onThreadStopped" );
    
    m_stop.post();
}

void Server::onConnectionAccepted( const propeller::Server::Connection& connection )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = *( ( LuaState* ) connection.thread().data() );
    lua.setGlobal( "__connection", ( void* ) &connection );
    lua.call( "onConnectionAccepted" );

}

void Server::onMessageReceived( const propeller::Server::Connection& connection, const propeller::Message& message )
{
    TRACE_ENTERLEAVE();
    
//    lua_pushnumber( m_lua, ( unsigned int ) connection.fd() );
//    lua_setglobal( m_lua, "__connectionId" );
//    
//    lua_pushlstring( m_lua, message.data, message.length );
//    lua_setglobal( m_lua, "__message" );
//
//    m_lua.call( "__onConnectionMessageReceived" ); 
}

void Server::onDataReceived( const propeller::Server::Connection& connection, const char* data, unsigned int length )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = *( ( LuaState* ) connection.thread().data() );
    
    lua.setGlobal( "__connection", ( void* ) &connection );
    lua.setGlobal( "__data", data, length );
    

    lua.call( "onConnectionDataReceived" ); 
 }

void Server::onDataReceived( const propeller::Server::Thread& thread, const std::string& from, const char* data, unsigned int length )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = *( ( LuaState* ) thread.data() );
    
    lua.setGlobal( "__data", data, length );
    lua.setGlobal(  "__from", from.c_str() );

    lua.call( "onUdpDataReceived" );        
}


void Server::onConnectionClosed( const propeller::Server::Connection& connection )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = *( ( LuaState* ) connection.thread().data() );
    lua.setGlobal( "__connection", ( void* ) &connection );

    lua.call( "onConnectionClosed" );
}

void Server::onTimer( const propeller::Server::Thread& thread, void* data )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = *( ( LuaState* ) thread.data() );
    
    if ( data )
    {
        Leda::instance()->callTimer( lua, ( Leda::TimerData* ) data );
    }
}

void Server::addTimer( lua_State* lua, unsigned int timeout, bool once, void* data )
{
    TRACE_ENTERLEAVE();
    
    lua_getglobal( lua, "__threadId" );
    
    
    unsigned int threadId = lua_tonumber( lua, -1 );
    
    TRACE( "adding timer to thread id %d", threadId );
    propeller::Server::addTimer( timeout, threadId, once, data );
    
    lua_pop( lua, 1 );
}

Server::~Server( )
{
}


