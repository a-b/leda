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

#include "Connection.h"
#include "trace.h"

namespace propeller
{
    EventThread::EventThread( unsigned int id )
    : libevent::Timer( m_base ), m_id( id )
    {
        TRACE_ENTERLEAVE();
    }
    
    
    EventThread::~EventThread()
    {
        TRACE_ENTERLEAVE();
    }
     
    void EventThread::addTimerHandler( EventThread::TimerHandler handler, unsigned int seconds, bool once, void* data )
    {
        TRACE_ENTERLEAVE();
        
        TimerData* timerData = new TimerData();
        timerData->handler = handler;
        timerData->data = data;
        timerData->once = once;
        addTimer( seconds, timerData, once );
    }
    
    void EventThread::routine( )
    {
        TRACE_ENTERLEAVE( );        
        //
        //  call callback
        //
        onStart();
        //
        //  start thread (blocking)
        //
        m_base.start( );
        //
        //  call callback
        //
        onStop();
    }
    
    void EventThread::onTimer( unsigned int seconds, void* data )
    {
        TRACE_ENTERLEAVE();
        
        TimerData* timerData = ( TimerData* ) data;
        EventThread::TimerHandler handler = timerData->handler;
        
        
        //
        //  call the provided handler for this timer
        //
        ( this->*handler )( timerData->data );
        
        if ( timerData->once )
        {
            delete timerData;
        }   
    }
    
    void EventThread::deleteConnection( Connection* connection )
    {   
        TRACE_ENTERLEAVE();
        
        addTimerHandler( &EventThread::onDeleteConnection, 0, true, connection );
    }
    
    
    
    void EventThread::onDeleteConnection( void* data )  
    {
        TRACE_ENTERLEAVE();
        
        Connection* connection = ( Connection* ) data;
        
        TRACE("need to delete connection %p", connection);
        
        delete connection;
    }
    
    Connection::Connection( sys::Socket* socket, EventThread& thread )
    : m_data( NULL ), m_thread( thread ), m_ref( 1 ), BufferedEvent( socket, thread.base() ), m_port( 0 ), m_address( socket->peer().address() )
    {
        TRACE_ENTERLEAVE();
     
    }
    
    Connection::Connection( const std::string& host, unsigned int port, EventThread& thread )
    : m_data( NULL ), m_thread( thread ), m_ref( 1 ), BufferedEvent( host, port, thread.base() ), m_host( host ), m_port( port )
    {
        TRACE_ENTERLEAVE();
    }
     
    Connection::~Connection()
    {
        TRACE_ENTERLEAVE();
        
        if ( socket() )
        {
            delete socket();
        }
    }
    
    //
    //  increase reference count
    //
    void Connection::ref()
    {
        TRACE_ENTERLEAVE();
        unsigned int value = sys::General::interlockedIncrement( &m_ref );
        
        TRACE("%p: value: %d", this, value);
        
    }
    
    //
    //  decrease reference count
    //
    void Connection::deref()
    {
        TRACE_ENTERLEAVE();
        unsigned int value = sys::General::interlockedDecrement( &m_ref );
        
        if ( value == 0 )
        {
            onDelete();
        }
    }
    
    void Connection::onDelete()
    {
        TRACE_ENTERLEAVE();
        
        TRACE("onDelete: thread id: %d, connection thread id %d", sys::Thread::currentId(), m_thread.id() );

        //
        //  disable further event processingΩ
        //  
        disable();
        //
        //  if calling thread is not the connection thread schedule delete
        //
        if ( sys::Thread::currentId() != m_thread.id() )
        {   
            m_thread.deleteConnection( this );
        }
        else
        {
            //
            //  free the connection
            //
            delete this;
        }
    }
    
    void Connection::onRead()
    {
        TRACE_ENTERLEAVE();
    }
    
    void Connection::onWrite()
    {
        TRACE_ENTERLEAVE();
    }
    
    void Connection::onError(  )
    {
        TRACE_ENTERLEAVE();
        deref();
            
    }
    
    void Connection::onConnect(  )
    {
        TRACE_ENTERLEAVE();
            
    }
    
    MessageHeader::MessageHeader( unsigned int _messageLength )
    : messageLength( _messageLength )
    {
        int i = 0;
        while ( i < sizeof( marker ) )
        {
            marker[i] = 0xaa;
            i++;
        }
    }

    MessageHeader MessageConnection::m_exampleHeader( 0 );

    Message* MessageConnection::getMessage()
    {
        if ( !m_header )
        {
            if ( inputLength( ) < sizeof ( MessageHeader ) )
            {
                return NULL;
            }
            
            //
            //  see if there is a message header 
            //
            int found = searchInput( m_exampleHeader.marker, sizeof ( m_exampleHeader.marker ) );
            if ( found > -1 )
            {
                m_header = new MessageHeader( 0 );
                read( ( char* ) m_header, sizeof ( MessageHeader ) );
            }
        }
        
        if ( m_header )
        {   
            if ( inputLength( ) >= m_header->messageLength )
            {
                Message* message = new Message();
                message->length = m_header->messageLength;
                message->data = new char[ message->length ];

                read( message->data, message->length );

                delete m_header;
                m_header = NULL;
         
                return message;
            }
        }
        
        return NULL;
    }
    
    void MessageConnection::onRead()
    {
        TRACE_ENTERLEAVE();
        
        Message* message = getMessage();
        
        while ( message )
        {
            onMessage( *message );
            delete[] message->data;
            delete message;
            
            message = getMessage();
        }
    }
    
    void MessageConnection::onMessage( const Message& message )
    {
        
    }
    
    void MessageConnection::sendMessage( const Message& message )
    {
        TRACE_ENTERLEAVE();
        MessageHeader header( message.length );
        write( ( const char* ) &header, sizeof( MessageHeader ) );
        write( message.data, message.length );
    }
    
    void MessageConnection::sendMessage( const char* data, unsigned int length )
    {
        TRACE_ENTERLEAVE();
        TRACE( "sending message of %d bytes", length );
        MessageHeader header( length );
        write( ( const char* ) &header, sizeof( MessageHeader ) );
        write( data, length );
    }
}    

