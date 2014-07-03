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

#ifndef _PROPELLER_CONNECTION_H_
#define	_PROPELLER_CONNECTION_H_

#include "event.h"
#include "system.h"

namespace propeller
{

    class Connection;

    /**
     * event thread class that encapsulates libevent loop in a separate thread
     */
    class EventThread : public sys::Thread, public libevent::Timer
    {    
    public:
        friend class Connection;
        
        EventThread( unsigned int id );
        virtual ~EventThread();

        /**
         * return  event base associated with the instance
         */
        const libevent::Base& base( ) const
        {
            return m_base;
        }

        /**
         * definition of timer callback cmethod 
         */
        typedef void ( EventThread::*TimerHandler )( void* );
        /**
         * add timer callback
         * @param handler
         * @param seconds
         * @param once
         * @param data
         */
        void addTimerHandler( EventThread::TimerHandler handler, unsigned int seconds, bool once = true, void* data = NULL );
        /**
         * get thread id
         * @return thread id
         */
        unsigned int id() const
        {
            return m_id;
        }
        
    protected:
        //
        //  sys::Thread routine
        //
        virtual void routine( );
        
        struct TimerData
        {
            std::string handlerName;
            void* data;
            unsigned int seconds;
            bool once;
            EventThread::TimerHandler handler;
        };

        virtual void onTimer( unsigned int seconds, void* data );
        
        
        /**
         * callback that is invoked when a thread is started
         */
        virtual void onStart()
        {
            
        }
        
         /**
         * callback that is invoked when a thread is stopped
         */
        virtual void onStop()
        {
            
        }
        
    private:
        void onDeleteConnection( void* data );
        void deleteConnection( Connection* connection );
        
        
    protected:
        libevent::Base m_base;

    private:
        
        unsigned int m_id;
        
    };

    /**
     * Base connection class 
     */
    class Connection: public libevent::BufferedEvent
    {
    public:
        /**
         * connection constructor. connection wraps estabished TCP connection over socket
         * @param socket established socket instance
         * @param thread event thread 
         */
        Connection( sys::Socket* socket, EventThread& thread );
        /**
         * connection constructor. new TCP connection is estabished asynchronously to provided address
         * @param host host 
         * @param port port
         * @param thread event thread
         */
        Connection( const std::string& host, unsigned int port, EventThread& thread );        
        virtual ~Connection();
        /**
         * get ip
         * @return ip address of the established connection peer
         */
        const std::string& address() const
        {
            
            return m_address;
        }
        
       
        /**
         *  host
         * @return return host that connection has been made to 
         */
        const std::string& host() const
        {
            return m_host;
        }
        /**
         * port
         * @return port that connection is made to
         */
        unsigned int port() const
        {
            return m_port;
        }
        /**
         * get connection address string
         * @return connection address string
         */
//        const std::string& address() const
//        {
//            return m_address;
//        }

        /**
         * increase reference count of connection
         */
        void ref();
        /**
         * decrease reference count of connection. the memory is freed and connection is closed when reference count reaches 0. (initial ref count is 1)
         */
        void deref();
        /**
         * virtual method that is run when ref count reaches 0. default behavior is to free the memory and close the connection
         */
        virtual void onDelete();
        //
        // libevent::BufferedEvent overoads
        //
      protected:   
        /**
         * callback that is run when new data is available to read from the input buffer
         */
        virtual void onRead( );
        /**
         * callback  that is run when connection output buffer length reaches 0
         */
        virtual void onWrite();
        /**
         * callback that is run when a connection error has occured (default behavior is to close the connection)
         */
        virtual void onError( );
        /**
         * callback that is run when the connection has been established
         */
        virtual void onConnect( );
        
        /**
         * get the event base that this connecton is assigned to
         * @return event base
         */
        const libevent::Base& base() const
        {
            return m_thread.base();
        }
        /**
         * get event thread that this connecton is assigned to
         * @return event thread
         */
        EventThread& thread()
        {
            return m_thread;
        }

    private:
        static void onEventStatic( evutil_socket_t socket, short event, void *arg );
        
        
    protected:
        EventThread& m_thread;
    
    private:
        void* m_data;
        unsigned int m_ref;
        std::string m_host;
        unsigned int m_port;
        std::string m_address;    
    };

    /**
     * header for simple message exchange protocol over secure persistent connections
     */
    struct MessageHeader
    {
        char marker[ 4 ];
        unsigned int messageLength;
        MessageHeader( unsigned int messageLength = 0 );
    };

    /**
     * communication message
     */
    struct Message
    {
        char* data;
        unsigned int length;
        
    };
    
    /**
     * connection class that has logic for sending and receiving messages
     */
    class MessageConnection: public Connection
    {
    public:
        
        MessageConnection( sys::Socket* socket, EventThread& thread )
        : Connection( socket, thread ), m_header( NULL )
        {
        }
        
        MessageConnection( const std::string& host, unsigned int port, EventThread& thread )
        : Connection( host, port, thread ), m_header( NULL )
        {
        }        
        /**
         * send message
         * @param message
         */
        void sendMessage( const Message& message );
        /**
         * send message
         * @param data
         * @param length
         */
        void sendMessage( const char* data, unsigned int length );
        
     protected:
        /**
         * on messsage received callback
         * @param message
         */
        virtual void onMessage( const Message& message );
        
        virtual void onRead();
        
    private:
        Message* getMessage();
        
    private:
        static MessageHeader m_exampleHeader;
        MessageHeader* m_header;
    };
}
#endif	/* _PROPELLER_CONNECTION_H_ */

