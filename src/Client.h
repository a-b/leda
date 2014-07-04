/* 
 * File:   Client.h
 * Author: sergey
 *
 * Created on January 3, 2014, 8:27 AM
 */

#ifndef CLIENT_H
#define	CLIENT_H

#include <propeller/Client.h>
#include "LuaState.h"

class Client: public propeller::Client
    {
    public:
        Client( unsigned int threadCount );
        void addTimer( lua_State* lua, unsigned int timeout, bool once, void* data );
            
    protected:
        
        virtual ~Client();
        virtual void onTimer( const propeller::Client::Thread& thread, void* data );
        virtual void onThreadStarted( propeller::Client::Thread& thread );
        virtual void onThreadStopped( const propeller::Client::Thread& thread );
        
        virtual void onConnectionOpened( const propeller::Client::Connection& connection );
        virtual void onConnectionClosed( const propeller::Client::Connection& connection );
        
        virtual void onData( const propeller::Client::Connection& connection, const char* data, unsigned int length );
        
        
    private:
        
    };

#endif	/* CLIENT_H */

