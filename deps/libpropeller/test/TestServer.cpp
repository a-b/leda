/* 
 * File:   TestServer.cpp
 * Author: sergey
 *
 * Created on February 18, 2013, 1:09 PM
 */

#include <propeller/Server.h>
#include <propeller/HttpServer.h>

#include <signal.h>

#include <execinfo.h>


using namespace std;
using namespace propeller;

Server*  server;

void crashHandler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}


//class HttpHandler : public http::Server::EventHandler
//{
//public:
//        
//    virtual void onRequest( const http::Request& request, http::Response& response, sys::ThreadPool::Worker&  thread, const Server& server )
//    {
//       response.setBody("test");
//        
//        //response.write( "test" );
//        
//    }
//    
//    virtual void onThreadStarted( sys::ThreadPool::Worker& thread, const Server& server )
//    {
//        fprintf(stderr, "thread started with pool id %p\n", thread.poolId() );
//    }
//    
//    virtual void onTimer( unsigned int interval, const Server& server, void* data )
//    {
//        fprintf(stderr, "timer 2 %d \n", interval );
//    }
//};
//
//class Handler : public Server::EventHandler
//{
//public:
//     
//    virtual void onConnectionAccepted( const Connection& connection, const Server& server )
//    {
//        fprintf( stderr, "connection accepted, %s %s \n", connection.ip().c_str(), connection.hostname().c_str());
//    }
//    
//    virtual void onConnectionClosed( const Connection& connection, const Server& server )
//    {
//        fprintf( stderr, "connection closed \n");
//    }
//    
//    virtual void onThreadStarted( sys::ThreadPool::Worker& thread, const Server& server )
//    {
//        fprintf(stderr, "thread started with pool id %p\n", thread.poolId() );
//        
//        
//        fprintf(stderr, "data: %s\n", ( char* ) server.data() );
//    }
//    
//    virtual void onDataReceived( Connection& connection, sys::ThreadPool::Worker& thread, Server& server )
//    {
//        fprintf( stderr, "received %d bytes\n", connection.inputLength() );
//        
//        fprintf( stderr, "%s \n", connection.readLine() );
//        
//      //  connection.write( "data", 4 );
//        
//    }
//    
//    virtual void onMessageReceived( Connection& connection, const Client::Message& message, sys::ThreadPool::Worker& thread, const Server& server )
//    {
////        char* str = new char[ message.length + 1];
////        memcpy( str, message.data, message.length );
////        str[ message.length ] = 0;
////        
////        printf( "received message: %s \n", str );
//    }
//    
//    virtual void onTimer( unsigned int interval, Server& server,  void* data )
//    {
//        fprintf(stderr, "timer %d \n", interval );
//        
//        server.addTimer( 1, NULL, true );
//    }
//    
//};

class TestServer : public Server
{
public:
    TestServer( const std::string& host, unsigned int port )
    
    {
        setPort( port );
        setHost( host );
        setThreadCount( 2 );
    }
    
    virtual void onConnectionAccepted( const Server::Connection& connection )
    {
        fprintf(stderr, "thread %ld, accepted connection from %s", connection.thread().id(), connection.ip().c_str() );
        
        
    }
    
    virtual void onThreadStarted( Server::Thread& thread )
    {
        addTimer( 1, thread.id(), false );
    }
    
    virtual void onTimer( const Thread& thread, void* data )
    {
        fprintf(stderr, "timer: thread %ld, data 0x%x \n", thread.id(), data );
        
        for ( Thread::ConnectionMap::const_iterator i = thread.connections().begin(); i != thread.connections().end(); i++ )
        {
            i->second->write( "timer", 5 );
        }
    }
    
    
    
   
};

/*
 * 
 */
int main( int argc, char** argv )
{
   
    
    signal(SIGSEGV, crashHandler);
    
//    propeller::http::Server server( 9000,  handler, "127.0.0.1" );
//    //server = new Server( 10000, ( Server::EventHandler& ) handler );
    
//    TcpThread* thread = new TcpThread();
//    thread->start();
//    thread->join();
    
    
    TestServer server( "127.0.0.1", 11000 );
    server.start(); 
    
    
//    server.setPoolThreadCount( 1 );
//    server.setConnectionThreadCount( 1 );
    
    
//    server->setConnectionReadTimeout( 2 );
    ///server.addTimer( 5 );
  //  server->addTimer( 1 );
    
//    server.start();
//    
//    try
//    {
//        server->start();
//    }
//    catch ( ... )
//    {
//        
//    }
//    thread->join();
    
    return 0;
}

