#include "Client.h"
#include "Leda.h"

Client::Client( unsigned int threadCount )
: propeller::Client( threadCount )
{
    TRACE_ENTERLEAVE();
}


Client::~Client()
{
    TRACE_ENTERLEAVE();    
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
    delete &lua;
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


 void Client::onProcessExit( const propeller::Client::ChildProcess& process, const propeller::Client::Thread& thread, unsigned int code )
 {
     TRACE_ENTERLEAVE();
     
    LuaState& lua = LuaState::luaFromThread( thread, thread.id() );
    
    lua.setGlobal( "process", ( void* ) &process );
    lua.setGlobal( "processExitCode", code );
    
    lua.call( "onProcessExit" );
     
 }

 void Client::onProcessData( const propeller::Client::ChildProcess& process, const propeller::Client::Thread& thread, unsigned int type, const char* data, unsigned int length )
 {
     TRACE_ENTERLEAVE();
     
     LuaState& lua = LuaState::luaFromThread( thread, thread.id() );
     lua.setGlobal( "process", ( void* ) &process );
     lua.setGlobal( "processData", data, length );
     lua.setGlobal( "processDataType", type );
     
     lua.call( "onProcessData" );
     
     
     
 }