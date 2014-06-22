 
#include "Server.h"
#include "Leda.h"

Server::Server( propeller::Server::Type type )
: propeller::Server( type )
{
    TRACE_ENTERLEAVE();
    
    
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
    thread.setData( lua ); 
    
    {
        //
        //  thread locking code here
        //
        sys::LockEnterLeave lock( m_lock );
        m_threads[ *lua ] = &thread;
    }
    
    
    lua_pushlightuserdata( *lua, ( void* ) &thread );
    lua_setglobal( *lua, "__thread" );
    
    lua->call( "onThreadStarted" );
}


void Server::onThreadStopped( const propeller::Server::Thread& thread )
{
    TRACE_ENTERLEAVE();
    
    //Leda::instance()->callTerminate( m_lua );
    
    const LuaState& lua = *( ( LuaState* ) thread.data() );
    
    lua_pushlightuserdata( lua, ( void* ) &thread );
    lua_setglobal( lua, "__thread" );
    
    lua.call( "onThreadStopped" );
    
    m_stop.post();
}

void Server::onConnectionAccepted( const propeller::Server::Connection& connection )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = *( ( LuaState* ) connection.thread().data() );
    
    lua_pushlightuserdata( lua, ( void* ) &connection );
    lua_setglobal( lua, "__connection" );
    
    
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
    
    lua_pushnumber( lua, ( unsigned int ) connection.fd() );
    lua_setglobal( lua, "__connectionId" );
    
    lua_pushlstring( lua, data, length );
    lua_setglobal( lua, "__data" );

    lua.call( "onConnectionDataReceived" ); 
 }

void Server::onDataReceived( const propeller::Server::Thread& thread, const std::string& from, const char* data, unsigned int length )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = *( ( LuaState* ) thread.data() );
    
    lua_pushlstring( lua, data, length );
    lua_setglobal( lua, "__data" );
    
    lua_pushstring( lua, from.c_str() );
    lua_setglobal( lua, "__from" );

    lua.call( "onUdpDataReceived" );        
}


void Server::onConnectionClosed( const propeller::Server::Connection& connection )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = *( ( LuaState* ) connection.thread().data() );
    
    lua_pushlightuserdata( lua, ( void* ) &connection );
    lua_setglobal( lua, "__connection" );
    
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
    
    LuaThreadMap::const_iterator found = threads().find( lua );
    if ( found != threads().end() )
    {
        propeller::Server::Thread* thread = found->second;
        TRACE( "found thread 0x%x for lua state 0x%x", thread, lua );
        propeller::Server::addTimer( timeout, thread->id(), once, data );
    }
    else
    {
        TRACE("not found thread for lua state 0x%x", lua );
    }
}


    
Server::~Server( )
{
}

