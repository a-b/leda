/*
Copyright 2012-2014  Sergey Zavadski

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

#ifndef _PROPELLER_EVENT_H_  
#define	_PROPELLER_EVENT_H_

//
//  libevent headers
//
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>


#include "system.h"
#include "common.h"

//
//  libevent wrappers
//
namespace libevent
{
    class General
    {
    public:
        static void setSocketNonBlocking( sys::Socket& socket );
        static void initThreads();
        static void shutdown();
    };
    

    enum Error
    {
        BindError,
        ConnectionError,
        AcceptError,
        GeneralError,
        ListenError

    };
    
    /**
     * class that encapsulates libevent event base
     */
    class Base
    {

    public:

        Base();
        ~Base();
                
        /**
         * start the event base
         */
        void start( );
        /**
         * stop event base
         */
        void stop() const;
        
        operator event_base*() const
        {
            return m_base;
        }

    private:
         event_base* m_base;
    };

    /**
     * class that represents a listening socket
     */
    class Listener
    {
    public:
        
        
        
        Listener( sys::Socket::Type type );
        void listen( const Base& base );
        
        virtual void onAccept() = 0;
        virtual ~Listener();
        
        /**
         * set port to bind to
         * @param port
         */
        void setPort( unsigned int port )
        {
            m_port = port;
        }
        /**
         * set host to bind to
         * @param host
         */
        void setHost( const std::string& host )
        {
            m_host = host;
        }
        
        unsigned int port() const
        {
            return m_port;
        }
        
        const char* host() const
        {
            return m_host.c_str();
        }
        

    protected:
        sys::Socket m_socket;
        
    private:
        static void onAcceptStatic( evutil_socket_t listener, short event, void *arg );
        
    private:
        struct event* m_listenerEvent;
        unsigned int m_port;
        std::string m_host;
        
    };
    
    
    /**
     * class that encapsulates libevent timer functionality
     */
    class Timer
    {
    public:
        Timer( const Base& base )
        : m_base( base )
        {
            
        }
        
        void addTimer( unsigned int seconds, void* data,  bool once = false );
        
        static void onTimerStatic( evutil_socket_t fd, short what, void *arg );
        virtual void onTimer( unsigned int seconds, void* data = NULL ) = 0;
        
    private:
        struct Data
        {
            Data( Timer* _instance )
            : instance( _instance), arg( NULL ), once( false ), seconds( 0 )
            {
            }
            
            Timer* instance;
            void* arg;
            unsigned int seconds;
            bool once;
            event* timer;
        };
      
    protected: 
        const Base& m_base;
    };
    
    class Buffer
    {
    public:
        Buffer();
        Buffer( evbuffer* buffer );
        virtual ~Buffer();
        
        void write( const char* data, unsigned int length );
        operator evbuffer*() const
        {
            return m_buffer;
        }
        
        bool written() const
        {
            return m_write;
        }
        
        void reset()
        {
            m_write = false;
        }
        
        
        
    private:
        evbuffer* m_buffer;
        bool m_free;
        bool m_write;
        
    };
    
    /**
     * class that encapsulates libevent buffered event
     */
    class BufferedEvent
    {
    public:
        /**
         * create buffered event from a connected socket
         * @param socket connected socket
         * @param base libevent base
         */
        BufferedEvent( sys::Socket* socket, const Base& base );
        /**
         * create buffered event after establishing connection to given host and port
         * @param host host to establish connection to
         * @param port port to establish connection to
         * @param base libevent  base
         */
        BufferedEvent( const std::string& host, unsigned int port, const Base& base );
        virtual ~BufferedEvent();

        /**
         * enable event processing
         */
        void enable( );
        /**
         * disavle event processing
         */
        void disable();
         /**
         * on read callback. called when there is new data available for reading
         */
        virtual void onRead() 
        {
            
        }
        /**
         * on write callback. called when output buffer is empty
         */
        virtual void onWrite()
        {
            
        }
        /**
         * error callback. called when unrecoverable error has occured
         */
        virtual void onError( )
        {
            
        }
        /**
         * connect callback. called when new connection has been established
         */
        virtual void onConnect( )
        {
            
        }
        /**
         * Read data from buffer
         * @param data pointer to preallocated buffer to read data to
         * @param length size of the buffer
         * @return number of bytes read
         */
        unsigned int read( char* data, unsigned int length ) const;
       /**
         * write data to buffer
         * @param data pointer to buffer containing data to write
         * @param length size of the buffer
         */
        void write( const char* data, unsigned int length );
        /**
         * write data to buffer
         * @param buffer pointer to existing buffer containing data to write
         */
        void write( evbuffer* buffer );
        /**
         * Read line from buffer
         * @return  pointer to the buffer that contains characters read. Buffer must be freed by the caller
         */
        char* readLine() const;
        /**
         * get length of the input buffer
         * @return input buffer length
         */
        unsigned int inputLength() const;
        /**
         * get length of the output buffer
         * @return output buffer length
         */
        unsigned int outputLength() const;
        /**
         * search input buffer for first occurence of a given buffer
         * @param what buffer to search for
         * @param length buffer length
         * @return -1 if buffer not found or a position within the input buffer
         */
        int searchInput( const char* what, unsigned int length ) const;
        /**
         * Sets read and write timeouts
         * @param read timeout in seconds (0 for no timeout)
         * @param write timeout in seconds (0 for no timeout)
         * 
         */
        void setTimeouts( unsigned int read, unsigned int write ) const;
        /**
         * check whether event processing is enabled
         * @return enabled flasg
         */
        bool enabled();
        /**
         * get the underlying file descriptor
         * @return file descriptior associated with the buffered event
         */
        int fd() const;
        /**
         * get input buffer
         * @return pointer to input buffer struct
         */
        evbuffer* input() const
        {
            return m_input;
        }
        /**
         * get output buffer
         * @return pointer to output buffer struct
         */
        evbuffer* output() const
        {
            return m_output;
        }
        
        
    protected:
        sys::Socket* socket() const
        {
            return m_socket;
        }
        
    private:
        static void onReadStatic( bufferevent* bev, void* ctx );
        static void onWriteStatic( bufferevent* bev, void* ctx );
        static void onEventStatic( bufferevent* bev, short event, void* ctx );
        void onDelete();
        void onEvent( short event );
        
    private:
        sys::Socket* m_socket;
        evbuffer* m_input;
        evbuffer* m_output;
        bufferevent* m_event;
        bool m_error;
        
    };
    
    
}
#endif	/* _PROPELLER_EVENT_H_ */
