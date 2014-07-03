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

#ifndef _PROPELLER_SERVER_H_
#define _PROPELLER_SERVER_H_

#include "Client.h"
#include "Connection.h" 


namespace propeller
{
    
    /**
     *  multithreaded TCP server class 
     */
    class Server : public libevent::Listener, public libevent::Timer
    {
    public:
        /**
         *  server type
         */
        enum Type
        {
            /**
             * TCP server
             */
            Tcp
            ,
            /**
             * UDP server 
             */
            Udp,
            /**
             * Simple TCP server 
             */
            Simple
        };
        
        friend class Connection;
        
        /**
         * Connection thread
         */
        class Thread: public EventThread
        {
            friend class Server;
            friend class propeller::Connection;
            
        public:
            virtual ~Thread();
            
            /**
             * add custom timer event
             * @param seconds timer interval in seconds
             * @param once false if event should be repeated
             * @param data custom data that is passed to callback
             */
            void addCustomTimer( unsigned int seconds, bool once, void* data );
            
            
            
            
            /**
             * get server reference
             * @return reference to server
             */
            Server& server()
            {
                return m_server;
            }
            
            typedef std::map< propeller::Connection*, propeller::Connection* > ConnectionMap;
            
            /**
            * get all connections that are connected to server (connections are tracked only for server with a single connection thread)
            * @return map with established connections
             */
            const ConnectionMap& connections() const
            {
                return m_connections;
            }
            
            
        protected:
            Thread( Server& server, unsigned int id )
            : m_server( server ), EventThread( id )
            {
            }
            
            void addConnection( propeller::Connection* connection );
            void removeConnection( propeller::Connection* connection );
            
            void addDatagram( sys::Socket::Datagram* datagram );
            
        private: 
            
            virtual void onStart();
            virtual void onStop();
            void onCustomTimer( void* data );
            void onData( void* data );
            
            
        private: 
            Server& m_server;
            ConnectionMap m_connections;
        };
        
        class Connection: public propeller::MessageConnection
        {
            friend class Server;
            
            public:
               
                virtual ~Connection();
                const Thread& thread() const
                {
                    return ( Thread& ) m_thread;
                }
                
                
            protected:
                virtual void onWrite();
                virtual void onRead();
                
                virtual void onMessage( const Message& message );
                
                

            private: 
                Connection( sys::Socket* socket, Thread& thread );
                 
                Thread& thread()
                {
                    return ( Thread& ) m_thread;
                }
                
                Server& server()
                {
                    return thread().server();
                }
                    
            private:
                bool m_accepted;    
        };
        

        Server( Type type );
        virtual ~Server( );

        /**
         * Start server (blocking)
         */
        virtual void start(  );
        /**
         * Stop server
         */
        virtual void stop( );
        /**
         * Add a timer
         * @param seconds timer interval (in seconds)
         * @param threadId id of the thread where to add the tjmer
         * @param once true if onTimer callback should  be called once, false to set the repeating timer
         * @param data arbitrary data to pass to timer
         */
        void addTimer( unsigned int seconds, intptr_t threadId = 0, bool once = true, void* data = NULL );
        
         /**
         * Set thread count
         * @param threadCount number of connection threads to serve all connections (at least one connection thread is started)
         */
        void setThreadCount( unsigned int threadCount )
        {
            m_threadCount = threadCount;
        }
        /**
         * Get number of connection threads
         * @return number of connection threads
         */
        unsigned int getThreadCount( ) const
        {
            return m_threadCount;
        }
        /**
         * Set connection read timeout (if connection is not available for reading after timeout elapses, it is closed)
         * @param timeout read timeout in seconds
         */
        void setConnectionReadTimeout( unsigned int timeout )
        {
            m_connectionReadTimeout = timeout;
        }
        /**
         * Set connection write timeout (if connection is not available for writing after timeout elapses, it is closed)
         * @param timeout write timeout in seconds
         */        
        void setConnectionWriteTimeout( unsigned int timeout )
        {
            m_connectionWriteTimeout = timeout;
        }
        /**
         * Get connection read timeout
         * @return read timeout in seconds
         */
        unsigned int getConnectionReadTimeout( ) const
        {
            return m_connectionReadTimeout;
        }
        /**
         * Get connection write timeout
         * @return write timeout in seconds
         */
        unsigned int getConnectionWriteTimeout( ) const
        {
            return m_connectionWriteTimeout;
        }
        
        /**
         * on message received callback. called when a new message has been received(in case of simple server)
         * @param connection
         * @param thread
         * @param message
         */
        virtual void onMessageReceived( const Connection& connection, const Message& message );
        /**
         * callback that is called when data was received (in case of generic server)
         * @param connection
         * @param data
         * @param length
         */
        virtual void onDataReceived( const Connection& connection, const char* data, unsigned int length );
        /**
         * callback that is called when data was received (in case of generic server)
         * @param thread
         * @param data
         * @param length
         */
        virtual void onDataReceived( const Thread& thread, const std::string& address, const char* data, unsigned int length );
        /**
         * callback that is called when a new connection has been accepted by server
         * @param connection
         */
        virtual void onConnectionAccepted( const Connection& connection );
        /**
         * callback that is called when a connection has been closed
         * @param connection
         */
        virtual void onConnectionClosed( const Connection& connection );
        /**
         * callback that  is called when a timer event occurs
         * @param thread 
         * @param data custom data passed when creating a timer
         */
        virtual void onTimer( const Thread& thread, void* data );
        /**
         * callback that is called when a new thread has been started (default setting for server is one thread)
         * @param thread
         */
        virtual void onThreadStarted( Thread& thread )
;        /**
         * callback that is called when a thread has been stopped 
         * @param thread
         */
        virtual void onThreadStopped( const Thread& thread );
        /**
         * callback that is called after startup has been completed and before server enters blocking operation
         */
        virtual void onStarted( );
        
        typedef std::map< propeller::Connection*, propeller::Connection* > ConnectionMap;
        /**
         * get all connections that are connected to server (connections are tracked only for server with a single connection thread)
         * @return map with established connections
         */
        const ConnectionMap& connections() const
        {
            return m_connections;
        }
        
        Type type() const
        {
            return m_type;
        }
        
        void setType( Type type )
        {
            m_type = type;
        }
        
        virtual void onTimer( unsigned int seconds, void* data );
        void sendTo( const std::string& ip, unsigned int port, const char* data, unsigned int length );
        
     protected:
        virtual propeller::Connection* createConnection( Thread& thread, sys::Socket* socket ); 
        virtual Thread* createThread( ); 
        
        typedef std::map< intptr_t, Thread* > ThreadMap;
        
        const ThreadMap& threads() const
        {
            return m_threads;
        }
        
        
        
    private:
        //
        //  libevent::Listener method implementation
        //
        virtual void onAccept( );
        
        
        Thread* startThread();
        
        struct SendBuffer
        {
            SendBuffer( const char *_data, unsigned int _length, const std::string& _ip, unsigned int _port )
            : length( _length ), ip( _ip ), port( _port )
            {
                data = new char[ length ];
                memcpy( data, _data, length );
            }
            
            ~SendBuffer()
            {
                delete[] data;
            }
                    
            char* data;
            unsigned int length;
            std::string ip;
            unsigned int port;
            
        };
        
        typedef std::list< Thread* > ThreadList;
        
    protected:
        
        ThreadMap m_threads;
        ThreadList m_threadQueue
;        
        
        

    private:
        libevent::Base m_base;
        
        unsigned int m_threadCount;
        unsigned int m_connectionReadTimeout;
        unsigned int m_connectionWriteTimeout;
        ConnectionMap m_connections;
        bool m_stop;
        Type m_type;
        unsigned int m_threadId;
    };
}
#endif //_PROPELLER_SERVER_H_