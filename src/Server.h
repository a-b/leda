
#ifndef _SERVER_H
#define	_SERVER_H

#include "LuaState.h"
#include <propeller/Server.h>

class Server: public propeller::Server
{
public:
    Server( const std::string& type );
    virtual ~Server( );
    
    //
    //  propeller::Server overloads
    //  
    virtual void onConnectionAccepted( const propeller::Server::Connection& connection );
    virtual void onMessageReceived( const propeller::Server::Connection& connection, const propeller::Message& message );
    virtual void onDataReceived( const propeller::Server::Connection& connection, const char* data, unsigned int length );
    
    virtual void onConnectionClosed( const propeller::Server::Connection& connection );
    virtual void onTimer( const propeller::Server::Thread& thread, void* data );
    virtual void onThreadStarted( propeller::Server::Thread& thread );
    virtual void onThreadStopped( const propeller::Server::Thread& thread );
    virtual void stop();
    
    void setTimer( lua_State* lua, unsigned int timeout, void* data );

private:
    typedef std::map< lua_State*, propeller::Server::Thread* > LuaThreadMap; 
    
    const LuaThreadMap threads() const
    {
        return m_threads;
    }
    

private:
    sys::Semaphore m_stop;
    LuaThreadMap m_threads;
    sys::Lock m_lock;
    

};

#endif	/* _SERVER_H */

