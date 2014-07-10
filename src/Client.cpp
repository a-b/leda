#include "Client.h"
#include "Leda.h"

Client::Client( unsigned int threadCount )
: propeller::Client( threadCount )
{
    TRACE_ENTERLEAVE();
}

//void Client::onStart()
//{
//    TRACE_ENTERLEAVE();
////    m_lua = new LuaState( Leda::instance()->script() );
////    m_lua->setGlobal( "client" );
////    m_lua->load();
////    
////    //
////    //  add repeating timer  to prevent weird libevent bug 
////    //
////    addTimer( 1, false, ( void*) 1 );
//}

Client::~Client()
{
    TRACE_ENTERLEAVE();
    
    //
    //  set close lua to false (this is  to handle termination)
    //
    //m_lua->setClose( false );
}

void Client::onThreadStarted( propeller::Client::Thread& thread )
{
    TRACE_ENTERLEAVE();
   
    LuaState* lua = LuaState::luaForThread( thread, thread.id(), "__leda.client = true" );
    
    lua->call( "onClientThreadStarted" );
}

void Client::onThreadStopped( const propeller::Client::Thread& thread )
{
    TRACE_ENTERLEAVE();
    LuaState& lua = LuaState::luaFromThread( thread, thread.id() );
    
    lua.call( "onClientThreadStopped" );
}

void Client::onTimer( const propeller::Client::Thread& thread, void* data )
{
    TRACE_ENTERLEAVE();
    
    if ( data )
    {
        LuaState& lua = LuaState::luaFromThread( thread, thread.id() );
        Leda::instance()->callTimer( lua, ( Leda::TimerData* ) data );
    } 
}

void Client::onConnectionOpened( const propeller::Client::Connection& connection )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = LuaState::luaFromThread( connection.thread(), connection.thread().id() );
    lua.setGlobal( "clientConnection", ( void* ) &connection );
    
    lua.call( "onClientConnectionOpened" );
}

void Client::onData( const propeller::Client::Connection& connection, const char* data, unsigned int length )
{
    TRACE_ENTERLEAVE();
    LuaState& lua = LuaState::luaFromThread( connection.thread(), connection.thread().id() );
    
    std::string text;
    text.append( data, length );
    
    lua.setGlobal( "clientConnection", ( void* ) &connection );
    lua.setGlobal( "clientData", data, length );
    
    lua.call( "onClientData" );
}

void Client::onConnectionClosed( const propeller::Client::Connection& connection )
{
    TRACE_ENTERLEAVE();
    
    LuaState& lua = LuaState::luaFromThread( connection.thread(), connection.thread().id() );
    
    lua.setGlobal( "clientConnection", ( void* ) &connection );
    
    lua.call( "onClientConnectionClosed" );
}


void Client::addTimer( lua_State* lua, unsigned int timeout, bool once, void* data )
{
    TRACE_ENTERLEAVE();
    
    unsigned int threadId = LuaState::getThreadId( lua );
    
    TRACE( "adding timer to thread id %d", threadId );
    propeller::Client::addTimer( threadId, timeout, once, data );
}

