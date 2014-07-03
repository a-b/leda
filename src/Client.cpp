/* 
 * File:   Client.cpp
     * Author: sergey
 * 
 * Created on January 3, 2014, 8:27 AM
 */

#include "Client.h"
#include "Leda.h"

Client::Client()
: m_lua( NULL )
{
    
}

void Client::onStart()
{
    TRACE_ENTERLEAVE();
    m_lua = new LuaState( Leda::instance()->script() );
    m_lua->setGlobal( "client" );
    m_lua->load();
    
    //
    //  add repeating timer  to prevent weird libevent bug 
    //
    addTimer( 1, false, ( void*) 1 );
}

Client::~Client()
{
    //
    //  set close lua to false (this is  to handle termination)
    //
    //m_lua->setClose( false );
}


void Client::onTimer( void* data )
{
    TRACE_ENTERLEAVE();
    
    if ( data )
    {
        if ( ( intptr_t ) data == 1 )
        {
            //
            //  not a real timer, it is here to prevent libevent from locking
            //
        }
        else
        {
            Leda::instance()->callTimer( *m_lua, ( Leda::TimerData* ) data );
        }
    } 
}

void Client::onStop(  )
{
    propeller::Client::onStop();
    
    TRACE_ENTERLEAVE();
    //Breeze::instance()->callTerminate( m_lua );
    
}
        
void Client::onConnectionOpened( const propeller::Client::Connection& connection )
{
    TRACE_ENTERLEAVE();
    
    registerConnection( connection );
    m_lua->call( "__onClientConnectionNew" );
}

void Client::onMessageReceived( const propeller::Client::Connection& connection, const propeller::Message& message )
{
    TRACE_ENTERLEAVE();
    
//     lua_pushlstring( m_lua, message.data, message.length );
//     lua_setglobal( m_lua, "__message" );
//     
//     registerConnection( connection );
//     m_lua.call( "__onClientMessageReceived" );
}

void Client::onConnectionClosed( const propeller::Client::Connection& connection )
{
    TRACE_ENTERLEAVE();
    
    registerConnection( connection );
//    m_lua.call( "__onClientConnectionClosed" );
}

void Client::registerConnection( const propeller::Client::Connection& connection )
{
    TRACE_ENTERLEAVE();
    TRACE("%p", ( void* ) &connection );
    
//    m_lua->setGlobal(, "")
//    
//    lua_pushlightuserdata( m_lua, ( void* ) &connection );
//    lua_setglobal( m_lua, "__clientConnection" );
//
//    lua_pushnumber( m_lua, connection.fd() );
//    lua_setglobal( m_lua, "__clientConnectionId" );
//
//    lua_pushnumber( m_lua, connection.port( ) );
//    lua_setglobal( m_lua, "__clientConnectionPort" );
//
//    lua_pushstring( m_lua, connection.host( ).c_str( ) );
//    lua_setglobal( m_lua, "__clientConnectionHost" );
}


