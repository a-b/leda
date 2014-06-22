
#ifndef _SERVER_H
#define	_SERVER_H

#include "LuaState.h"
#include <propeller/Server.h>



class Server: public propeller::Server
{
public:
    Server( propeller::Server::Type type );
    virtual ~Server( );
    
    
    
    //
    //  propeller::Server overloads
    //  
    virtual void onConnectionAccepted( const propeller::Server::Connection& connection );
    virtual void onMessageReceived( const propeller::Server::Connection& connection, const propeller::Message& message );
    virtual void onDataReceived( const propeller::Server::Connection& connection, const char* data, unsigned int length );
    virtual void onDataReceived( const propeller::Server::Thread& thread, const std::string& from, const char* data, unsigned int length );
    
    virtual void onConnectionClosed( const propeller::Server::Connection& connection );
    virtual void onTimer( const propeller::Server::Thread& thread, void* data );
    virtual void onThreadStarted( propeller::Server::Thread& thread );
    virtual void onThreadStopped( const propeller::Server::Thread& thread );
    virtual void stop();
    
    void addTimer( lua_State* lua, unsigned int timeout, bool once, void* data );

private:
    typedef std::map< lua_State*, propeller::Server::Thread* > LuaThreadMap; 
        

private:
    sys::Semaphore m_stop;
};

#endif	/* _SERVER_H */

