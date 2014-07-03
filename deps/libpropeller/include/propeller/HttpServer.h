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

#ifndef _PROPELLER_HTTPSERVER_H_
#define	_PROPELLER_HTTPSERVER_H_

#include "Server.h"
#include "HttpProtocol.h"


namespace propeller
{
    namespace http
    {
        
         /**
         * HTTP request class         
         */
        class Request  
        {
            friend class Connection;
            
        public:
            /**
             * struct representing http header
             */
            struct Header
            {
                char* name;
                char* value;
                char* _line;
                
                char* lowerCase( char* s )
                {
                   int length =  strlen( s );
                    
                    while ( *s )
                    { 
                        *s = tolower( *s );
                        s ++;
                    }
                    
                    s = s - length;
                    
                    return s;
                }
                
                Header()
                {
                    
                }
                
                Header( const Header& other )
                {
                    name = other.name;
                    value = other.value;
                    _line = other._line;
                }
            };
                        
            /**
             * Get HTTP method
             * @return HTTP method string
             */
            const char* method( ) const
            {
                return m_method;
            }

            /**
             * Get request URI
             * @return string containing the request URI
             */
            const char* uri( ) const
            {
                return m_uri;
            }
    
            /**
             * Get request protocol 
             * @return string containing the request protocol
             */
            const char* protocol( ) const
            {
                return m_protocol;
            }


            /**
             * Get body
             * @return request body
             */
            const char* body() const
            {
                return m_body;
            }
            
            /**
             * get body length
             * @return body length
             */
            unsigned int bodyLength() const
            {
                return m_bodyLength;
            }
            
            

            //
            //  headers 
            //  
            typedef std::list< Header > HeaderList;
            
            /**
             * Get all request headers
             * @return HeaderMap with headers
             */
            const HeaderList& headers( ) const
            {
                return m_headers;
            }
            
        public:
            Request( Connection* connection );
            ~Request();
            
            
            const Connection* connection() const
            {
                return m_connection;
            }
            
            
            void parse();
            bool closeConnection() const
            {
                return m_closeConnection;
            }
            
            void reset();
            
            bool complete() const
            {
                return m_complete;
            }

            
            
            
            
        private:
            Header* parseHeader( char* line );
            
            
            
          
            
            
            
            

        private:
            char m_method[64];
            char m_uri[128];
            char m_protocol[64];
            char m_header[2048];
            char* m_body;
            bool m_complete;
            HeaderList m_headers;
            unsigned int m_bodyLength;
            bool m_parsedHeader;
            unsigned int m_read;
            Connection* m_connection;
            unsigned int m_timestamp;
            const char* m_contentLength;
            bool m_closeConnection;

        };

        /**
         * HTTP response class 
         */
        class Response 
        {
            friend class Connection;
            
        public:
            
            /**
             * Get response status code
             * @return status code number
             */
            unsigned int status( ) const
            {
                return m_status;
            }

            /**
             * Set status code
             * @param status HTTP status code number
             */
            void setStatus( unsigned int status )
            {
                m_status = status;
            }

            /**
             * Add header`
             * @param name header name
             * @param value header value
             */
            void addHeader( const char* name, const char* value );
            /**
             * Set body
             * @param body pointer to buffer 
             * @param length if length is 0 then body is assumed to be zero terminated string otherwise length of buffer passed as first parameter
             */
            void setBody( const char* body = NULL, unsigned int length = 0 );
            
            bool closeConnection() const
            {
                return m_closeConnection;
            }
            
            

        public:
            Response( Connection* connection, unsigned int status = HttpProtocol::Ok );
            ~Response();
            
            void reset();
            bool complete() const
            {
                return m_init;
            }

            
        private:
            void init( );
                        
            libevent::Buffer& buffer();
            
        private:
            unsigned int m_status;
            bool m_init;
            Connection* m_connection;
            bool m_closeConnection;
            bool m_body;
            
        };

        
         /**
         * multithreaded HTTP server class that executes requests asynchronously in the thread pool
         */
        class  Server : public propeller::Server, public sys::ThreadPool
        {
        public:
            Server();
            virtual ~Server()
            {
                
            }
            
        
            
            
            //
            //  connection class
            //
            class Connection: public propeller::Connection
            {
                friend class Server;
                friend class Response;
            public:
                
                virtual ~Connection();
                
                Request* request()
                {
                    return &m_request;
                }
                
                Response* response()
                {
                    return &m_response;
                }
                
            protected:
                Connection( sys::Socket* socket, Thread& thread );
                
                Server& server()
                {
                    Thread& connectionThread = ( Thread& ) thread();
                    return ( Server& ) connectionThread.server();
                }
                
                
                
                virtual void processRequest();
                virtual void close();
                
            private:
                virtual void onRead();
                virtual void onWrite();
                
                void parseRequest();
                
                
                
                
                libevent::Buffer& buffer()
                {
                    return m_buffer;
                }
                
            private:
                Request m_request;
                Response m_response;
                libevent::Buffer m_buffer;
                bool m_close;
            };

            /**
             * set number of threads in the thread pool
             * @param poolThreadCount nunmber of threads
             */
            void setPoolThreadCount( unsigned int poolThreadCount )
            {
                m_poolThreadCount = poolThreadCount;
            }
            /**
             * get number of threads in thread pool
             * @return number of threads in thread pool
             */
            unsigned int getPoolThreadCount() const
            {
                return m_poolThreadCount;
            }
         
            //
            //  sys::ThreadPool overloads
            //
            virtual void onThreadStart( sys::ThreadPool::Worker& thread );
            virtual void onThreadStop( const sys::ThreadPool::Worker& thread );
            
            //
            //  propeller::Server overloads
            //
            virtual void start();
            virtual void stop();
            virtual propeller::Connection* createConnection( Thread& thread, sys::Socket* socket );
            virtual void onRequest( const Request& request, Response& response, const propeller::Server::Thread& thread );
            
        friend class Connection;
        
        private:
            struct Task : public sys::ThreadPool::Task
            {
            
                Task( Connection& _connection )
                : connection( _connection )
                {
                    
                }
                
                Connection& connection;
            };

            virtual void onTaskProcess( sys::ThreadPool::Task* task, sys::ThreadPool::Worker& thread );
            
        private:
            unsigned int m_poolThreadCount;
            
        };
        
        
          

        class Exception
        {
        public:
            Exception( HttpProtocol::Status status )
            : m_status( status )
            {
                
            }

            const HttpProtocol::Status status( ) const
            {
                return m_status;
            }

        private:
            HttpProtocol::Status m_status;
        };

    };
};

#endif	/* _PROPELLER_HTTPSERVER_H_*/
