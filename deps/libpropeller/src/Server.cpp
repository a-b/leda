/*
Copyright 2012-2014 Sergey Zavadski

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

#include "Server.h"
#include "Client.h"

//
//	Trace function
//
#include "trace.h"

namespace propeller
{

    Server::Connection::Connection( sys::Socket* socket, Thread& thread )
    : MessageConnection( socket, ( EventThread& ) thread ), m_accepted( false )
    {
        TRACE_ENTERLEAVE();
    }
            
    
    //
    //  on read callback  
    //
    void Server::Connection::onWrite()
    {
        TRACE_ENTERLEAVE();
        
        //
        //  invoke the accepted callback if needed
        //
        if ( !m_accepted )
        {
            thread().addConnection( this );
            m_accepted = true;
        }
    }
    
    void Server::Connection::onRead()
    {
        TRACE_ENTERLEAVE();
        //
        //  invoke the accepted callback if needed
        //
        if ( !m_accepted )
        {
            thread().addConnection( this );
            m_accepted = true;
        }
        
        if ( server().type() == Server::Simple )
        {
            MessageConnection::onRead();
        } 
        else
        {
            unsigned int length = inputLength();
            char* data = new char[ length ] ; 
            
            read( data, length );
            server().onDataReceived( *this, data, length );
            
            delete[] data;
        }
        
        
    }
    
    void Server::Connection::onMessage( const Message& message )
    {
        TRACE_ENTERLEAVE();
        server().onMessageReceived( *this, message );
    }
    
    
    Server::Connection::~Connection()
    {
        TRACE_ENTERLEAVE();
        thread().removeConnection( this );
    }
    
    
    Server::Server( Type type )
    : libevent::Listener( type == Udp ? sys::Socket::Udp : sys::Socket::Tcp ), m_threadCount( 1 ), m_connectionReadTimeout( 0 ), 
            m_connectionWriteTimeout( 0 ), m_stop( false ), m_type( type ), m_threadId( 0 ), libevent::Timer( m_base )
    {
        TRACE_ENTERLEAVE( );

        libevent::General::initThreads( );
    }

     Server::~Server( )
    {
        TRACE_ENTERLEAVE( );

        if ( !m_stop )
        {
            stop( );
        }

        libevent::General::shutdown( );
        sys::Socket::cleanup();
        
    }
     
    void Server::stop( )
    {
        TRACE_ENTERLEAVE( );
        
        m_base.stop();
        
        for ( ThreadMap::iterator i = m_threads.begin(); i != m_threads.end(); i++ )
        {
            i->second->base().stop();
        }
        
        m_stop = true;

    }
    
     void Server::start( )
     {
        TRACE_ENTERLEAVE( );
        
        sys::Socket::startup();

        if ( !m_threadCount )
        {
            m_threadCount = 1;
        }
        
        //
        //  start threads
        //
        while ( m_threads.size( ) < m_threadCount )
        {
            TRACE( "threads %d, need %d", m_threads.size( ), m_threadCount );
            createThread( );
        }
       
     
        //
        //  bind to listening port  
        //
        listen( m_base );
        
        
        //
        //  invoke callback
        //
        onStarted();
 
        //
        //  start event base (blocking)
        //
        m_base.start( );
    }
     
     Server::Thread* Server::createThread( ) 
     {
         
         Thread* thread = new Thread( *this, m_threadId );
         m_threadId ++;
         thread->start();
         m_threads[ thread->id() ] = thread;
         m_threadQueue.push_back( thread );
         TRACE("created thread id %ld", thread->id() );
         
         return thread;
     }
     
     

     void Server::onAccept()
     {
        TRACE_ENTERLEAVE();
        
        //
        //  get next connection thread
        //
        Thread* thread = m_threadQueue.front();
        m_threadQueue.pop_front(); 
        
        
        
        if ( m_type == Udp )
        {
            TRACE("need to accept udp datagram", "");
            
            sys::Socket::Datagram* datagram;
            
            if ( m_socket.receive( &datagram ) == sys::Socket::StatusSuccess )
            {
                thread->addDatagram( datagram );
            }
        }
        else
        {
            //
            //  accept new connection and add it to connection thread
            //
            sys::Socket* socket = m_socket.accept( );

            if ( !socket )
            {
                TRACE_ERROR( "accept failed, error %d", sys::Socket::getLastError( ) );
                return;
            }
            
            //
            //  create new connection and assign it to connection thread event base (it will delete itself when socket is closed)
            //  
            propeller::Connection* connection = createConnection( *thread, socket );
            //
            //  set connection read and write timeouts
            //
            connection->setTimeouts( m_connectionReadTimeout, m_connectionWriteTimeout );
            connection->enable( );
        }
        
        m_threadQueue.push_back( thread );
    }
     
    void Server::Thread::removeConnection( propeller::Connection* connection )
    {
        TRACE_ENTERLEAVE();
        
        m_connections.erase( connection );
        
        server().onConnectionClosed( ( Connection& ) *connection );
    }
    
    void Server::Thread::addConnection( propeller::Connection* connection )
    {
        TRACE_ENTERLEAVE();

        m_connections[ connection ] = connection;
        
        server().onConnectionAccepted( ( Connection& ) *connection );
    }
     
    void Server::onThreadStarted( Thread& thread )
    {
        TRACE_ENTERLEAVE();
    }
    
    void Server::onThreadStopped( const Thread& thread )
    {
        TRACE_ENTERLEAVE();
    }
    
    void Server::onStarted()
    {
        TRACE_ENTERLEAVE();
    }


    propeller::Connection* Server::createConnection( Thread& thread, sys::Socket* socket )
    {
        return ( propeller::Connection* ) new Connection( socket, thread );
    }
     

    void Server::onConnectionAccepted( const Connection& connection )
    {
        TRACE_ENTERLEAVE();
    }

    void Server::onMessageReceived( const Connection& connection, const Message& message )
    {
        TRACE_ENTERLEAVE();
    }
    
    
    void Server::onDataReceived( const Connection& connection, const char* data, unsigned int length )
    {
        TRACE_ENTERLEAVE();
    }

    void Server::onDataReceived( const Thread& thread, const std::string& address, const char* data, unsigned int length )
    {
        TRACE_ENTERLEAVE();
    }
    
    void Server::onConnectionClosed( const Connection& connection )
    {
        TRACE_ENTERLEAVE();
    }
    
     
    void Server::onTimer( const Thread& thread, void* data )
    {
        TRACE_ENTERLEAVE();
    }
    
    void Server::addTimer( unsigned int seconds, intptr_t threadId, bool once, void* data )
    {
        TRACE_ENTERLEAVE( );
        
        TRACE( "adding timer to thread %ld", threadId );
        
    
        Thread* thread;
        //  add timer to the first connection thread
        //
        if ( threadId )
        {
            ThreadMap::const_iterator found = threads().find( threadId );
            if ( found != threads().end() )
            {
                thread = found->second;
            }
            else
            {
                TRACE( "thread with id %ld not found", threadId );
            }
        }
        else
        {
            if ( !m_threads.size() )
            {
                //
                //  has to be called from the main thread
                //
                thread = createThread();
            }
            else
            {
                thread = threads().begin()->second;
            }
        }
        
        thread->addCustomTimer( seconds, once, data );
    }
    
    void Server::Thread::addCustomTimer( unsigned int seconds, bool once, void* data )
    {
        addTimerHandler( ( EventThread::TimerHandler ) &Thread::onCustomTimer, seconds, once, data );
    }
    
    void Server::Thread::addDatagram( sys::Socket::Datagram* datagram )
    {
        addTimerHandler( ( EventThread::TimerHandler ) &Thread::onData, 0, true, datagram );
    }
    
    void Server::Thread::onCustomTimer( void* data )
    {
        TRACE( "thread 0x%x onTimer() ", this );
        server().onTimer( *this, data );
    }
    
    void Server::Thread::onData( void* data )
    {
        TRACE_ENTERLEAVE();
        sys::Socket::Datagram* datagram = ( sys::Socket::Datagram* ) data;
        server().onDataReceived( *this, datagram->from.address(), datagram->data, datagram->length );
        
        delete datagram;
    }
    
    void Server::Thread::onStart()
    {
        TRACE_ENTERLEAVE();
        m_server.onThreadStarted( *this );
    }
    
    void Server::Thread::onStop()
    {
        TRACE_ENTERLEAVE();
        m_server.onThreadStopped( *this );
    }
    
    Server::Thread::~Thread()
    {
        TRACE_ENTERLEAVE();
    }
    
    void Server::sendTo( const std::string& ip, unsigned int port, const char* data, unsigned int length )
    {
        SendBuffer* buffer = new SendBuffer( data, length, ip, port );
        libevent::Timer::addTimer( 0, ( void* ) buffer, true );  
    }
    
    void Server::onTimer( unsigned int seconds, void* data )
    {
        TRACE_ENTERLEAVE();
        
        SendBuffer* buffer = ( SendBuffer* ) data;
        
        
        
        
        m_socket.sendto( buffer->ip, buffer->port, buffer->data, buffer->length );
        
        delete buffer; 
    }
    
    
}   