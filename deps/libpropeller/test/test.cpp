#include "Client.h"
#include "Server.h"
#include "HttpServer.h"

#include "trace.h"

using namespace propeller;

class TestClient: public Client
{
public:
    virtual void onConnect( Client::Connection& connection )
    {
        TRACE("connected to %s:%d", connection.host().c_str(), connection.port() );
    }
    
    virtual void onStart()
    {
        TRACE_ENTERLEAVE();
        
        connect( "localhost", 11857 );
    }
    
};

class TestServer: public Server
{
public:
    virtual void onThreadStarted( const Server::Thread& thread ) 
    {
        TRACE_ENTERLEAVE();
        this->addTimer( 1, false, 0 );
        
        
    }
    
    virtual void onTimer( void* data )
    {
        for ( Server::ConnectionMap::const_iterator i = connections().begin(); i != connections().end(); i++ )
        {
            Server::Connection* connection = ( Server::Connection* ) i->first;
            Message message;
            char data[] = "asfwef";
            message.data = data;
            message.length = 4;     
            connection->sendMessage( message );
        }
    }
};


class TestHttpServer: public http::Server
{
public:
    virtual void onRequest( const http::Request& request, http::Response& response, const sys::ThreadPool::Worker& thread )
    {
        TRACE_ENTERLEAVE();
        
        response.setBody( "test", 4 );
    }
    
};


TestClient* client;
TestServer* server;
TestHttpServer* httpServer;

void sigKillHandler( int signal )
{
   
//    delete server;
//    client->stop();
//    delete client;
}

int main(int argx, char** argv)
{
//    signal( SIGTERM, sigKillHandler );
//    signal( SIGINT, sigKillHandler );
//    signal( SIGABRT, sigKillHandler );
    
//    httpServer = new TestHttpServer();
//    httpServer->setHost( "127.0.0.1" );
//    httpServer->setPort( 13000 );
//    httpServer->start();
    
    client = new TestClient();
    client->addTimer(1, true, NULL);
    client->start();
    
    
    
    return 0;
}
