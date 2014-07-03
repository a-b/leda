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
#include "event.h"

#include "trace.h"

    
namespace libevent
{
    static bool s_initThreads = false;

    void General::setSocketNonBlocking( sys::Socket& socket )
    {
        evutil_make_socket_nonblocking( socket.s() );
    }

    void General::initThreads()
    {
        if ( s_initThreads )
        {
            return;
        }

        s_initThreads = true;
            
#ifdef WIN32
     evthread_use_windows_threads();
#else
     evthread_use_pthreads();
#endif
    }
    
    void General::shutdown()
    {
        libevent_global_shutdown();
    }

    Base::Base( )
    : m_base( NULL )
    {
        m_base = event_base_new( );

        evthread_make_base_notifiable( m_base );
    }
    void Base::start(  )
    {
        TRACE_ENTERLEAVE();
            
        event_base_loop( m_base, EVLOOP_NO_EXIT_ON_EMPTY );
    }
    
    void Base::stop( ) const
    {
        TRACE_ENTERLEAVE( );

        if ( m_base )
        {
            event_base_loopbreak( m_base );
        }

    }
    
    Base::~Base( )
    {
        TRACE_ENTERLEAVE();
        stop( );
        
// this segfaults on os x
//        if ( m_base )
//        {
//            event_base_free( m_base );
//        }
    }

    Listener::Listener( sys::Socket::Type type )
    : m_listenerEvent( NULL ), m_port( 0 ), m_host( "" ), m_socket( type )
    {
    }

    void Listener::listen( const Base& base)
    {
        TRACE_ENTERLEAVE();

        if ( m_socket.bind( m_port, m_host.empty() ? NULL : m_host.c_str() ) == sys::Socket::StatusFailed )
        {
            TRACE_ERROR( "cannot bind to socket (%s:%d), error %d", m_host.c_str(), m_port, sys::General::getLastError() );
            throw BindError;
        }

        if ( m_socket.type() == sys::Socket::Tcp )
        {
            if ( m_socket.listen() == sys::Socket::StatusFailed )
            {
                TRACE_ERROR("socket listen() failed with error %d", sys::Socket::getLastError() );
                throw ListenError;
            }
        }
        

        m_listenerEvent = event_new( base, m_socket.s(), EV_READ | EV_PERSIST, onAcceptStatic,  this );
        
        

        if ( !m_listenerEvent )
        {
            throw GeneralError;
        }

        event_add( m_listenerEvent, NULL );
    }

    Listener::~Listener()
    {
        if ( m_listenerEvent )
        {
            event_free( m_listenerEvent );
        }
    }

    void Listener::onAcceptStatic( int listener, short event, void* arg )
    {
        (( Listener* ) arg)->onAccept();
    }

    BufferedEvent::BufferedEvent( const std::string& host, unsigned int port, const Base& base )
    : m_socket( NULL ), m_event( NULL ), m_error( false )
    {
        TRACE( "connecting to %s:%d", host.c_str(), port );
        
        m_event = bufferevent_socket_new( base, -1, BEV_OPT_CLOSE_ON_FREE );
        
        if ( !m_event )
        {
            TRACE_ERROR( "failed to create buffered event", "" );
            throw GeneralError;
        }
        
        enable();

        struct sockaddr_in service;
        memset( &service, 0, sizeof(service) );

        service.sin_family = AF_INET;
        inet_pton( AF_INET, host.c_str(), &service.sin_addr );
        service.sin_port = htons( port );
        
        bufferevent_setcb( m_event, onReadStatic, onWriteStatic, onEventStatic, this );
        
        
        int result = bufferevent_socket_connect( m_event, ( struct sockaddr* ) &service, sizeof( service ) );
        m_input = bufferevent_get_input( m_event );
        m_output = bufferevent_get_output( m_event );        
    }
    
    BufferedEvent::BufferedEvent( sys::Socket* socket, const Base& base )
    : m_socket( socket ), m_event( NULL ), m_error( false )
    {
        TRACE_ENTERLEAVE();
        General::setSocketNonBlocking( *socket );
        
        m_event = bufferevent_socket_new( base, *m_socket, 0  );

        if ( !m_event )
        {
            TRACE_ERROR( "failed to create buffered event", "" );
            throw GeneralError;
        }
        
        bufferevent_setcb( m_event, onReadStatic, onWriteStatic, onEventStatic, this );
        
        m_input = bufferevent_get_input( m_event );
        m_output = bufferevent_get_output( m_event );
        
     }

    BufferedEvent::~BufferedEvent()
    {
        TRACE_ENTERLEAVE();
        
        if ( m_event )
        {
            bufferevent_free( m_event );
        }
    }
    
    void BufferedEvent::enable()
    {
        bufferevent_enable( m_event, EV_READ | EV_WRITE  ); 
    }
   

    unsigned int BufferedEvent::read( char* data, unsigned int length ) const
    {
        return bufferevent_read( m_event, data, length );
    }
    
    void BufferedEvent::write( const char* data, unsigned int length ) 
    {
        bufferevent_write( m_event, data, length );
    }
    
    void BufferedEvent::write( evbuffer* buffer )
    {
        TRACE_ENTERLEAVE();
        evbuffer_add_buffer( m_output, buffer );
    }
    
    void BufferedEvent::onReadStatic( bufferevent* bev, void* ctx )
    {
        TRACE_ENTERLEAVE();
        (( BufferedEvent* ) ctx)->onRead();
    }

    void BufferedEvent::onWriteStatic( bufferevent* bev, void* ctx )
    {
        TRACE_ENTERLEAVE();
        (( BufferedEvent* ) ctx)->onWrite();
    }

    void BufferedEvent::onEventStatic( bufferevent* bev, short event, void* ctx )
    {
        TRACE_ENTERLEAVE();
        
        (( BufferedEvent* ) ctx)->onEvent( event );
    }
    
    void BufferedEvent::onEvent( short event )
    {
        TRACE_ENTERLEAVE();
        //
        //  invoke callbacks
        //
        if ( event & BEV_EVENT_CONNECTED )
        {
            onConnect();
        }
        
        if ( event & BEV_EVENT_ERROR || event & BEV_EVENT_TIMEOUT || event & BEV_EVENT_EOF )
        {
            if ( !m_error )
            {
                onError();
                m_error = true;
            }
        }
    }

    char* BufferedEvent::readLine() const  
    {
        return evbuffer_readln( m_input, NULL, EVBUFFER_EOL_CRLF );
    }

    unsigned int BufferedEvent::inputLength() const
    {
        return evbuffer_get_length( m_input );
    }
    
    unsigned int BufferedEvent::outputLength() const
    {
        return evbuffer_get_length( m_output );
    }

    int BufferedEvent::searchInput( const char* what, unsigned int length ) const
    {
        evbuffer_ptr found = evbuffer_search( m_input, what, length, NULL );
        return found.pos;
    }

    void BufferedEvent::setTimeouts( unsigned int read, unsigned int write ) const
    {
        TRACE_ENTERLEAVE();

        timeval* readTimeout = NULL;
                
        if ( read )
        {
            readTimeout = new timeval();
            readTimeout->tv_sec = ( long ) read;
            readTimeout->tv_usec = 0;            
        }
        timeval* writeTimeout = NULL;
        
        if ( write )
        {
            writeTimeout = new timeval();
            writeTimeout->tv_sec = ( long ) write;
            writeTimeout->tv_usec = 0;
        }
                
        bufferevent_set_timeouts( m_event, readTimeout, writeTimeout );
        
        if ( readTimeout )
        {
            delete readTimeout;
        }
        
        if ( writeTimeout )
        {
            delete writeTimeout;
        }
    }

    bool BufferedEvent::enabled()
    {
        return bufferevent_get_enabled( m_event ) == 1;
    }
    
    void BufferedEvent::disable()
    {
        if ( m_socket )
        {
            m_socket->shutdown();
        }
        
        bufferevent_disable( m_event, EV_READ | EV_WRITE  );
    }
    
    int BufferedEvent::fd() const
    {
        return ( int ) bufferevent_getfd( m_event );
    }
    
    void Timer::addTimer( unsigned int seconds, void* data, bool once )
    {
        TRACE_ENTERLEAVE();
        
        TRACE("adding timer: seconds %d, data %p, once %d", seconds, data, once );
        
        struct timeval timeout = { seconds, 0 };
        
        Data* timerData = new Data( this );
        
        timerData->arg = data;
        timerData->once = once;
        timerData->seconds = seconds;
        
        
        
        if ( !once )
        {
            timerData->timer = event_new( m_base, -1, EV_PERSIST, onTimerStatic, timerData );
            event_add( timerData->timer, &timeout );
        }
        else
        {
            timerData->timer = evtimer_new( m_base, onTimerStatic, timerData );
            evtimer_add( timerData->timer, &timeout );
        }
    }       
    
    void Timer::onTimerStatic( evutil_socket_t fd, short what, void *arg )
    {
        Data* data = ( Data* )arg;
        data->instance->onTimer( data->seconds, data->arg );
        
        //
        //  delete timer if it was one time
        //
        if ( data->once )
        {
            TRACE("freeing timer", "");
            event_free( data->timer );
            delete data;
        }
    }
    
    Buffer::Buffer()
     : m_free( true ), m_write( false )
    
    {
        TRACE_ENTERLEAVE();
        
        m_buffer = evbuffer_new();
    }
    
    Buffer::Buffer( evbuffer* buffer )
     : m_free( false ), m_buffer( buffer ), m_write( false )
    {
        
    }
    
    Buffer::~Buffer()
    {
        if ( m_free )
        {
            evbuffer_free( m_buffer );
        }
    }
    
    void Buffer::write( const char* data, unsigned int length )
    {
        evbuffer_add( m_buffer, data, length );         
        m_write = true; 
    }
    
    
        
}
