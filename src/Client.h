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
        Client();
            
    protected:
        
        virtual ~Client();
        virtual void onTimer( void* data );
        
        virtual void onConnectionOpened( const propeller::Client::Connection& connection );
        virtual void onConnectionClosed( const propeller::Client::Connection& connection );
        
        virtual void onMessageReceived( const propeller::Client::Connection& connection, const propeller::Message& message );
        
        virtual void onStart();
        virtual void onStop();
        
//        const LuaState& lua() const
//        {
//            return m_lua;
//        }
        
    private:
        void registerConnection( const propeller::Client::Connection& connection );
        
    private:
        LuaState* m_lua;
        sys::Event m_stop;
        
    };

#endif	/* CLIENT_H */

