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

#include "HttpServer.h"
#include "trace.h"

namespace propeller
{
    
    namespace http
    {
        
        Server::Connection::Connection( sys::Socket* socket, Thread& thread )
        : propeller::Connection( socket, ( EventThread& ) thread ), m_response( this ), m_request( this ), m_close( false )
        {
            TRACE_ENTERLEAVE();
        }
        
        Server::Connection::~Connection()
        {
            TRACE_ENTERLEAVE();
        }
        
        void Server::Connection::onWrite()
        {
            TRACE_ENTERLEAVE();
            
            
            if ( m_buffer.written() )
            {
                m_buffer.reset();
                
                if ( m_close )
                {
                    this->close();
                }
            }
        }
        
        //
        //  libevent on read callback
        //  
        void Server::Connection::onRead()
        {
            TRACE_ENTERLEAVE();
            
            //
            //  see if request needs to be parsed
            //
            if ( !m_response.complete() && !m_request.complete() )
            {
                //
                //  parse the request
                //
                parseRequest( );
            } 
            
            if ( m_request.complete() )
            {
                
                processRequest();
            }
        }
        
        void Server::Connection::close()
        {
            deref( );
        }
        
        void Server::Connection::parseRequest()
        {
            try
            {
                m_request.parse( );
            }
            
            catch ( const Exception& e )
            {
                if ( e.status() >= 400 )
                {                    
                    Response* response = new Response( this, e.status() );
                    response->setBody();
                    m_close = true;
                }
            }
        }
        
        void Server::Connection::processRequest()
        {
            
            //
            //  increase reference count
            //
            //ref( );
            //
            //  handle request in the thread pool  
            //
            //server( ).queue( new Task( *this ) );
            
            server().onRequest( m_request, m_response, ( const Server::Thread& ) thread() );
            if ( m_request.closeConnection() )
            {
                m_close = true;
            }
            
            m_response.reset();
            m_request.reset();
        }

        Server::Server( )
        : propeller::Server( propeller::Server::Tcp )
        {
            TRACE_ENTERLEAVE( );

            HttpProtocol::initialize( );
            
            setPoolThreadCount( 20 );
            setThreadCount( 10 );
        }
        
        void Server::start( )
        {
            if ( !m_poolThreadCount )
            {
                m_poolThreadCount = 1;
            }
            
            TRACE_ENTERLEAVE();
            //sys::ThreadPool::start( m_poolThreadCount );
            
            propeller::Server::start();
        }
        
        void Server::stop( )
        {
            TRACE_ENTERLEAVE();
            sys::ThreadPool::stop();
            propeller::Server::stop();
        }
        
        void Server::onTaskProcess(  sys::ThreadPool::Task* task, sys::ThreadPool::Worker& thread )
        {
            TRACE_ENTERLEAVE();
            
            Task* serverTask = ( Task* ) task; 
//            
            Connection& connection = serverTask->connection;
//            
//            if ( connection.response() && connection.request() )
//            {
//                //
//                //  invoke the callback
//                //
//                onRequest( *connection.request(), *connection.response(), tho );
//            }
            
            //
            //  decrease connection reference count
            //
            connection.deref();

            delete serverTask;
        }
        
        void Server::onThreadStart( sys::ThreadPool::Worker& thread )
        {
            TRACE_ENTERLEAVE();
        }
        
        void Server::onThreadStop( const sys::ThreadPool::Worker& thread )
        {
            TRACE_ENTERLEAVE();
        }
        
        propeller::Connection* Server::createConnection( Thread& thread, sys::Socket* socket )
        {
            return ( propeller::Connection* ) new Server::Connection( socket, thread );
        }
        
        void Server::onRequest( const Request& request, Response& response, const propeller::Server::Thread& thread )
        {
            TRACE_ENTERLEAVE();
        }
        
        Request::Request( Connection* connection  )
        : m_bodyLength( 0 ),  m_parsedHeader( false ), m_read( 0 ), m_connection( connection ), m_body( NULL ), m_contentLength( NULL ), m_closeConnection( false ), m_complete( false )
        {
            TRACE_ENTERLEAVE( );

            reset();
        }
        
        void Request::reset()
        {
            TRACE_ENTERLEAVE( );
            
            m_method[0] = 0;
            m_uri[0] = 0;
            m_protocol[0] = 0;
            m_header[0] = 0;
            m_bodyLength = 0;
            m_read = 0;
            m_contentLength = NULL;
            m_parsedHeader = false;
            
            if ( m_body )
            {
                delete[] m_body;
                m_body = NULL;
            }
            
            for ( HeaderList::iterator i = m_headers.begin(); i != m_headers.end(); i++ )
            {
                Header header = *i;
                
                free( header._line );
                //delete header;
            }
            
            m_headers.clear();
            
            m_complete = false;
        }

        void Request::parse( )
        {
            TRACE_ENTERLEAVE();
            
            //
            //	try to parse out buffer
            //

            if ( !strlen( m_method ) )
            {
                //
                //	check request line
                //
                if ( m_connection->searchInput( "\r\n",  2 ) == -1 )
                {
                    throw Exception( HttpProtocol::Ok );
                }
                
                char* requestLine;
                
                requestLine = m_connection->readLine( );

                if ( !requestLine )
                {
                   
                    throw Exception( HttpProtocol::BadRequest );
                }

                int read = sscanf( requestLine, "%s %s %s", m_method, m_uri, m_protocol );

                if ( read != 3 )
                {
                    free( requestLine );
                    throw Exception( HttpProtocol::BadRequest );
                }

                TRACE( "%s", requestLine );

                free( requestLine );
            }

            if ( !m_parsedHeader )
            {
                //
                //  see if we have all headers
                //
                if ( m_connection->searchInput( "\r\n\r\n", 4 ) == -1 )
                {
                    throw Exception( HttpProtocol::Ok );
                }

                //
                //	parse headers
                //
                for (;; )
                {
                    char* line = m_connection->readLine( );

                    Header* header = parseHeader( line );
                    if ( !header )
                    {
                        free( line );
                        break;
                    }
                        
                    
                    if ( !strcmp( header->name, "connection" ) && !strcmp( header->value, "close" ) )
                    {
                        TRACE( "need to close connection", "" );
                        m_closeConnection = true;
                    }   
                    
                    if ( !strcmp( header->name, "content-length" ) ) 
                    {
                        m_contentLength = header->value;
                    }
                }
                
                m_parsedHeader = true;
            }
            

            if ( !strcmp( m_method, "PUT" ) && !m_contentLength )
            {
                //
                //  allow put requests with no body
                //
                m_complete = true;
                return;
            }
            
            //	body is required for PUT and POST method requests
            //
            bool needBody = m_method[0] == 'P';

            //
            //	check request headers
            //
            if ( !needBody )
            {
                m_complete = true;
                return;
            }
            
            if ( m_contentLength )
            {
                //
                //	Get body length
                //
                sscanf( m_contentLength, "%d", &m_bodyLength );
            }
            else
            {
                //
                //	Always require content length
                //
                throw Exception( HttpProtocol::LengthRequired );
            }

            if ( !m_body )
            {
                m_body = new char[ m_bodyLength ];
                TRACE( "allocated body buffer of length %d", m_bodyLength );
            }

            m_read += m_connection->read( m_body + m_read, m_bodyLength - m_read );

            if ( m_read != m_bodyLength )
            {
                //
                //  wait for more data
                //
                throw Exception( HttpProtocol::Ok );
            }
            
            m_complete = true;
        }
        
        Request::Header* Request::parseHeader( char* line )
        {
            TRACE_ENTERLEAVE();
            
            char* separator = strstr( line, ": " );

            if ( !separator )
            {
                return NULL;
            }

            Header header;
            header._line = line;

            *( strchr( line, ':' ) ) = 0;

            header.name = header.lowerCase( line );
            separator += 2;

            header.value = separator;


            TRACE( "name: '%s'", header.name );
            TRACE( "value: '%s'", header.value );
            
            m_headers.push_back( header );
            
            return &( m_headers.back() );
        }
    
        Response::Response( Connection* connection, unsigned int status )
        : m_status( status ), m_connection( connection ), m_init( false ), m_closeConnection( false ), m_body( false )
        {
            TRACE_ENTERLEAVE( );
        }

        Request::~Request( )
        {
            TRACE_ENTERLEAVE();
            
            reset();
        }
        
        Response::~Response()
        {
            TRACE_ENTERLEAVE();
        }
        
        libevent::Buffer& Response::buffer()
        {
            return ( ( Server::Connection* ) m_connection )->buffer();
        }

            
        void Response::reset( )
        {
            TRACE_ENTERLEAVE();
            
            m_status = 200;
            m_init = false;
            m_body = false;
        }
        
        void Response::init( )
        {
            if ( m_init )
            {
                return;
            }

            m_init = true;

            TRACE_ENTERLEAVE( );

            char responseLine[64] = "0";

            sprintf( responseLine, "HTTP/1.1 %u %s\r\n", m_status, HttpProtocol::reason( m_status ).c_str( ) );
            TRACE( "%s", responseLine );

            buffer().write( responseLine, strlen( responseLine ) );

            char serverVersion[32] = "\0";
            sprintf( serverVersion, "%s/%s", PROPELLER_NAME, PROPELLER_VERSION );
            addHeader( "Server", serverVersion );
        }

        void Response::addHeader( const char* name, const char* value )
        {
            TRACE_ENTERLEAVE();
            init( );
            
            TRACE( "adding header %s:%s", name, value );
            
            TRACE( "%d", m_body );
            
            if ( m_body )
            {
                TRACE( "body was already written, ignoring header ", "" );
                return;
            }
            
            buffer().write( name, strlen( name ) );
            buffer().write( ": ", 2 );
            buffer().write( value, strlen( value ) );
            buffer().write( "\r\n", 2 );
        }

        void Response::setBody( const char* body, unsigned int length )
        {
            TRACE_ENTERLEAVE();
            init( );
            
            if ( m_status >= 400 )
            {
                m_closeConnection = true;
            }
            
            if ( !body )
            {
                addHeader( "Content-Length", "0" );
                m_body = true;
                buffer().write( "\r\n", 2 );
            }
            else
            {
                char value[16];
                sprintf( value, "%u", length ? length : ( unsigned int ) strlen( body ) );
                addHeader( "Content-Length", value );
                
                m_body = true;
            
                buffer().write( "\r\n", 2 );
                buffer().write( body, length ? length : strlen( body ) );
            }

            
            m_connection->write( buffer() );
        }
    }
}
    