libpropeller is multithreaded class library in C++ for building high performance network applications powered by libevent. All network operations are asynchronous and executed concurrently multiple libevent event loops. It offers easy to integrate classes that encapsulate functionality of a http server,  tcp server, asynchronous client. 


**Building:**

    git clone https://github.com/sergeyzavadski/libpropeller.git
    cd libpropeller
    configure && make
    sudo make install

**Example of asynchronous multithreaded HTTP server listening on localhost port 10000**

(compile with `-L/usr/local/lib -I/usr/local/include -lpropeller`)

     #include <propeller/HttpServer.h>
        
     using namespace propeller;


     class ExampleHttpServer : public http::Server
     {
     public:
         virtual void onRequest( const http::Request& request, http::Response& response, const sys::ThreadPool::Worker& thread )
         {
             //
             //  return this text for every response
             //
             response.setBody( "hello world!" );
         }
     };

     int main( int argv, char** argc )
     {
         //
         //  create http server instance
         //
         ExampleHttpServer* server = new ExampleHttpServer( );
         //
         // set port
         //
         server->setPort( 10000 );
         //
         //  set host
         //
         server->setHost( "127.0.0.1" );
         //
         //  start the server
         //
         server->start( );
     }


**Example of asynchronous multithreaded echo TCP server listening on localhost port 10000**

(compile with `-L/usr/local/lib -I/usr/local/include -lpropeller`)

    #include <propeller/Server.h>

    using namespace propeller;

    //
    //  connection class
    //
    class ExampleConnection : public Connection
    {
    public:
        ExampleConnection( sys::Socket* socket, Server::Thread& thread )
        : Connection( socket, thread )
        {
        }

        virtual void onRead( )
        {
            //
            // read bytes count
            //
            unsigned int length = inputLength( );
            //
            // read all from the input
            //
            char* buffer = new char[ length ];
            read( buffer, length );
            //
            //  write to output
            //
            write( buffer, length );
        }
    };

    class ExampleServer : public Server
    {
    public:
        virtual propeller::Connection* createConnection( Thread& thread, sys::Socket* socket )
        {
            //
            //  create instance of custom connection class
            //
            return new ExampleConnection( socket, thread );
        }
    };

    int main( int argv, char** argc )
    {
        //
        //  create server instance
        //
        ExampleServer* server = new ExampleServer( );
        // 
        //  set the port
        //
        server->setPort( 10000 );
        //
        //  ser threads count
        //
        server->setThreadCount( 10 );
        //
        //  start the server
        //
        server->start( );
    }

**Example of asynchronous TCP client that connects to localhost port 6379**

(compile with `-L/usr/local/lib -I/usr/local/include -lpropeller`)

    
        #include <propeller/Client.h>
        
        using namespace propeller;

        class ExampleClient : public Client
        {
        public:

            virtual void onConnectionOpened( const Client::Connection& connection )
            {
                printf( "connected to %s:%d", connection.host( ).c_str( ), connection.port( ) );
            }

            virtual void onTimer( void* data )
            {
                //
                //  timer callback
                //
                printf( "timer \n" );
            }

            virtual void onStart( )
            {
                //
                //  add timer that repeats every second
                //
                addTimer( 1, false );

                //
                //  connect to redis server at localhost
                //        
                connect( "localhost", 6379 );
            }
    
    
        };

        int main( int argc, char** argv )
        {
            //
            //  start the client
            //        
            ( new ExampleClient() )->start();
        }


For api documentation see [documentation](http://sergeyzavadski.github.io/libpropeller/)





    
    
    

