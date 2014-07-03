/* 
 * File:   TestServer.cpp
 * Author: sergey
 *
 * Created on February 18, 2013, 1:09 PM
 */

#include <propeller/Client.h>

#include <signal.h>

#include <execinfo.h>


using namespace std;
using namespace propeller;

//Client* client;

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

static Client::Connection* storedConnection;

class EventHandler: public propeller::Client::EventHandler  
        {
        public:
            virtual void onConnectionNew( propeller::Client::Connection& connection )
            {
                storedConnection = &connection;
            }
            
            virtual void onConnectionClosed( const propeller::Client::Connection& connection )
            {
                printf("connection closed \n");
            }
            
            virtual void onMessageReceived( const propeller::Client::Connection& connection, const  propeller::Client::Message& message )
            {
                printf("received message length: %d \n", message.length );
            }
            
            virtual void onTimer( unsigned int seconds, void* data )
            {
                printf("timer event with %d seconds \n", seconds );
                storedConnection->sendMessage( "test", 4 );
            }
        
        };

/*
 * 
 */
int main( int argc, char** argv )
{
    signal(SIGSEGV, crashHandler);
    
    EventHandler eventHandler;
    propeller::Client client(eventHandler);
    
    client.connect(9002, "localhost" );
    
    client.addTimer(1, NULL);
    
    client.start();
    client.worker().join();
    
    //server = new Server( 10000, ( Server::EventHandler& ) handler );
    
//    TcpThread* thread = new TcpThread();
//    thread->start();
    
//    server.setPoolThreadCount( 1 );
//    server.setConnectionThreadCount( 1 );
    
    
//    server->setConnectionReadTimeout( 2 );
    ///server.addTimer( 5 );
  //  server->addTimer( 1 );
    
    
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

