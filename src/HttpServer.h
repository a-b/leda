    /* 
 * File:   HttpServer.h
 * Author: sergey
 *
 * Created on January 4, 2014, 5:08 AM
 */

#ifndef HTTPSERVER_H
#define	HTTPSERVER_H

#include "LuaState.h"
#include <propeller/HttpServer.h>


class HttpServer: public propeller::http::Server
{
public:
    HttpServer();
    
    virtual void onRequest( const propeller::http::Request& request, propeller::http::Response& response, sys::ThreadPool::Worker& thread );
    virtual void onThreadStart( sys::ThreadPool::Worker& thread );
    virtual void onThreadStop( const sys::ThreadPool::Worker& thread );
    virtual void stop();
    virtual void start();
    virtual void onTimer( void* data );
    virtual void onStarted( );
    
    
private:
    LuaState m_lua;
    unsigned int m_threadId;
    sys::Semaphore m_stop;
    unsigned int m_stoppedThreads;
};

#endif	/* HTTPSERVER_H */

