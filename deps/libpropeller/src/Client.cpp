/*
Copyright 2013-2014 Sergey Zavadski

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "Client.h"

#include "trace.h"

namespace propeller
{    
    void Client::WorkerThread::onStart()
    {
        TRACE_ENTERLEAVE();
        //
        //  invoke client start callback
        //
        m_client.onStart();
    }
    
    void Client::WorkerThread::onStop()
    {
        TRACE_ENTERLEAVE();
        //
        //  invoke client stop callback
        //
        m_client.onStop();
        m_stop.post();
    }
    
    void Client::WorkerThread::addCustomTimer( unsigned int seconds, bool once, void* data )
    {
        TRACE_ENTERLEAVE();
        
        addTimerHandler( ( EventThread::TimerHandler ) &WorkerThread::onCustomTimer, seconds, once, data );
    }
    
    void Client::WorkerThread::onCustomTimer( void * data )
    {
        TRACE_ENTERLEAVE();
        //
        //  invoke client timer callback
        //
        m_client.onTimer( data );
    }

    Client::Connection::Connection( const std::string& host, unsigned int port, WorkerThread& thread )
    : MessageConnection( host, port, ( EventThread& ) thread )
    {
        
    }
    //
    //  connection connect callback
    //
    void Client::Connection::onConnect()
    {
        client().addConnection( this );
    }
    
    //
    //  connection on message callback
    //
    void  Client::Connection::onMessage( const Message& message )
    {
        //
        //  invoke client callback
        //
        client().onMessageReceived( *this, message );
    }
    
    Client::Connection::~Connection()
    {
        TRACE_ENTERLEAVE();
        client().removeConnection( this );
    }
  
    Client::Connection* Client::connect( const std::string& host, unsigned  int port )
    {
        TRACE_ENTERLEAVE();
        
        //
        //  create new connection to provided host and port
        //
        return new Connection( host, port, m_workerThread );
    }
    
    void Client::start()
    {
        TRACE_ENTERLEAVE();
        
        //
        //  start worker thread
        //  
        m_workerThread.start();
        m_workerThread.join();

    }
    
    Client::Client()
    : m_workerThread( *this, 0 ), m_data( NULL ), m_stop( false )
    {
        TRACE_ENTERLEAVE();
        
    }
    
    Client::~Client()
    {
        
        TRACE_ENTERLEAVE();
        stop();
        
    }
    
    void Client::onStart()
    {
        TRACE_ENTERLEAVE();
    }
    
    void Client::addTimer( unsigned int seconds, bool once, void* data )
    {
        TRACE_ENTERLEAVE();
        
        m_workerThread.addCustomTimer( seconds, once, data );
    }
    
    void Client::onMessageReceived( const Connection& connection, const Message& message )
    {
        TRACE_ENTERLEAVE();
        
        std::string messageString;
        messageString.append( message.data, message.length );
        TRACE("%s", messageString.c_str());
    }
    
    void Client::onConnectionOpened( const Connection& connection )
    {
        TRACE_ENTERLEAVE();
    }
    
    void Client::onConnectionClosed( const Connection& connection )
    {
        TRACE_ENTERLEAVE();
    }
    
    void Client::onTimer( void* data )
    {
        TRACE_ENTERLEAVE();
    }
    
    void Client::stop()
    {
        if ( !m_stop )
        {
            m_stop = true;
            m_workerThread.exit();
        }
    }
    
    void Client::addConnection( Connection* connection )
    {
        onConnectionOpened( *connection );
        TRACE_ENTERLEAVE( );
        m_connections[ connection ] = connection;
    }
    
    void Client::removeConnection( Connection* connection )
    {
        TRACE_ENTERLEAVE();

        if ( !m_stop )
        {
            onConnectionClosed( *connection );
        }
        
        m_connections.erase( connection );
    }
    
    void Client::onStop(  )
    {
        TRACE_ENTERLEAVE();
        
        while ( !m_connections.empty() )
        {
            Connection* connection = m_connections.begin()->first;
            delete connection;
        }

    }

}
    