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

#ifndef _PROPELLER_CLIENT_H_
#define	_PROPELLER_CLIENT_H_

#include "Connection.h"


namespace propeller
{
    /**
     * Class that implements multithreaded asynchroonus TCP client functionality and event loop.
     */
    class Client
    {
    public:
        class WorkerThread;
        
        /**
        * Client connection class
        */
        class Connection: public MessageConnection
        {
            friend class Client;
        public:
            
            virtual ~Connection();
            /**
             * callback that runs when the connection has been established
             */
            virtual void onConnect();
            /**
             * callback that runs when a new message has been received
             * @param message message received
             */
            virtual void onMessage( const Message& message );
            
        private:
            Connection( const std::string& host, unsigned int port, WorkerThread& thread );
            
            WorkerThread& thread()
            {
                return ( WorkerThread& ) m_thread;
            }
            
            Client& client()
            {
                return thread().client();
            }
        };
   
        
        friend class Connection;
        
        
        /**
         * worker thread class
         */
        class WorkerThread: public EventThread
        {
           
            
        public:
            WorkerThread( Client& client, unsigned int id )
            : m_client( client ), EventThread( id )
            {

            }
            
            virtual void onStart();
            virtual void onStop();
            
            void exit()
            {
                m_base.stop();
                m_stop.wait();
            }
            
            /**
             * add custom timer
             * @param seconds timer interval in seconds
             * @param once true if timer should not be repeated
             * @param data custom data that will be passed in a callback
             */
            void addCustomTimer( unsigned int seconds, bool once, void* data );
            
            /**
             * get client reference
             * @return client reference
             */
            Client& client()
            {
                return m_client;
            }
            
        private:
            void onCustomTimer( void* data );
            
        private: 
            Client& m_client;
            sys::Semaphore m_stop;
            
            
        };
        
        
        /**
         * Client constructor
         */
        Client( );
        virtual ~Client();
        /**
         * start the client ( blocking )
         */
        void start();
        /*
         * stop client
         */
        virtual void stop();
        
        /**
         * callback that runs when client has been started
         */
        virtual void onStart();
        
        
        /**
         * Connect asynchronously via TCP
         * @param port remore port
         * @param host remote host
         * @return 
         */
        Connection* connect( const std::string& host, unsigned  int port );
        
        /**
         * add timer
         * @param seconds timer timeout in seconds
         * @param data timer data that will be passed  to event handler
         * @param once if true creates one time timer event 
         */
        void addTimer( unsigned int seconds, bool once = true, void* data = NULL );
        /**
         * timer callback
         * @param data custom data passed when creating a timer
         */
        virtual void onTimer( void* data );
        /**
         * callback that runs when the connection has been established
         * @param connection connection established
         */
        virtual void onConnectionOpened( const Connection& connection );
        /**
         * callback that runs when the connection is closed
         * @param connection
         */
        virtual void onConnectionClosed( const Connection& connection );
        /**
         * 
         * @param connection
         * @param message
         */
        virtual void onMessageReceived( const Connection& connection, const Message& message );
        
        virtual void onStop( );
        
        
        
        /**
         * map of established connections
         */
        typedef std::map< Connection*, Connection* > ConnectionMap;
        
        /**
         * all established connections
         * @return map that contains pointers to establised connections
         */
        const ConnectionMap& connections() const
        {
            return m_connections;
        }
        
    private:
        void addConnection( Connection* connection );
        void removeConnection( Connection* connection );
        
        
    private:
        WorkerThread m_workerThread;
        static MessageHeader s_header;
        void* m_data;
        ConnectionMap m_connections;
        bool m_stop;
    };
}


#endif	/* _PROPELLER_CLIENT_H_ */

