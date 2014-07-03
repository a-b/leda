/*
Copyright 2012 Sergey Zavadski

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

#include "system.h"
#include "trace.h"

#include "event.h"

#ifdef __MACH__
    #include <uuid/uuid.h>
#endif

#define DATAGRAM_BUFFER_SIZE 1024

namespace sys
{
    //
    //	General
    //

    unsigned int General::getLastError ( )
    {
#ifdef WIN32
        return( unsigned int ) GetLastError( );
#else
        return errno;
#endif
    }

    unsigned int General::getTime ( )
    {
        time_t currentTime = 0;
        time( &currentTime );
        return( unsigned int ) currentTime;
    }
    
    unsigned int General::getMillisecondTimestamp()
    {
        unsigned int timestamp;

#ifdef WIN32
        timestamp = GetTickCount();
#else
        timeval time;
        gettimeofday( &time, NULL );
        
        timestamp = ( unsigned int ) ( ( time.tv_sec ) * 1000 + time.tv_usec / 1000.0 ) + 0.5;
#endif        
        return timestamp;
    }
    
    

    void* General::interlockedExchangePointer ( void** target, void* value )
    {
#ifdef WIN32
        return InterlockedExchangePointer( target, value );
#else
        void* oldValue = __sync_fetch_and_add( target, 0 );
        return __sync_val_compare_and_swap( target, oldValue, value );
#endif

    }

    unsigned int General::interlockedIncrement( unsigned int* target )
    {
#ifdef WIN32
    return InterlockedIncrement( target );
#else
        unsigned int result = __sync_fetch_and_add( target, 1 );
        return result + 1;
#endif
    }

    unsigned int General::interlockedDecrement( unsigned int* target )
    {
#ifdef WIN32
        return InterlockedDecrement( target );
#else
        unsigned int result = __sync_fetch_and_sub( target, 1 );
        return result - 1;
#endif
    }



    void General::sleep ( unsigned int seconds )
    {
#ifdef WIN32
        Sleep( seconds * 1000 );
#else
        ::sleep( seconds );
#endif
    }
    
    void General::getCurrentDirectory( char* buffer, unsigned int length )
    {
#ifdef WIN32
        GetCurrentDirectory( length, buffer );
#else
        getcwd( buffer, length );
#endif
    }

    void General::setCurrentDirectory( const char* path )
    {
#ifdef WIN32
        SetCurrentDirectory( path );
#else
        chdir( path );
#endif
    }
    
    unsigned int General::getProcessId()
    {
#ifdef WIN32
        return GetProcessId( GetCurrentProcess() );
#else
        return getpid();
#endif
        
    }
    

    //
    //	Lock
    //

    Lock::Lock ( )
    {
#ifdef WIN32
        InitializeCriticalSection( &m_lock );
#else
        pthread_mutexattr_t mutexAttributes;
        pthread_mutexattr_init( &mutexAttributes );
        pthread_mutexattr_settype( &mutexAttributes, PTHREAD_MUTEX_ERRORCHECK );
        pthread_mutex_init( &m_lock, &mutexAttributes );
#endif
    }

    Lock::~Lock ( )
    {
#ifdef WIN32
        DeleteCriticalSection( &m_lock );
#else
        pthread_mutex_destroy( &m_lock );
#endif
    }

    LOCK_HANDLE* Lock::handle ( )
    {
        return &m_lock;
    }
    
    void Lock::lock()
    {
 #ifdef WIN32
        EnterCriticalSection( &m_lock );
#else
        pthread_mutex_lock( &m_lock );
#endif
    }
    
    void Lock::unlock()
    {
 #ifdef WIN32
        LeaveCriticalSection( &m_lock );
#else
        pthread_mutex_unlock( &m_lock );
#endif
    }
    
    
    

    //
    //	LockEnterLeave
    //
    LockEnterLeave::LockEnterLeave ( Lock& lock )
    : m_lock ( lock )
    {
        m_lock.lock();
    }

    LockEnterLeave::~LockEnterLeave ( )
    {
        m_lock.unlock();    
    }

    //
    //	Event
    //
    Event::Event ( )
    : m_disabled ( false )
    {
#ifdef WIN32
        m_handle = CreateEvent( 0, FALSE, 0, NULL );
#else
        pthread_mutex_init( &m_lock, NULL );

        pthread_cond_init( &m_condition, NULL );
#endif
    }

    Event::~Event ( )
    {
#ifdef WIN32
        CloseHandle( m_handle );
#else
        pthread_cond_destroy( &m_condition );

        pthread_mutex_destroy( &m_lock );
#endif
    }

    void Event::wait ( )
    {
        if ( m_disabled )
        {
            return;
        }

#ifdef WIN32
        WaitForSingleObject( m_handle, INFINITE );
#else		
        pthread_mutex_lock( &m_lock );

        pthread_cond_wait( &m_condition, &m_lock );

        pthread_mutex_unlock( &m_lock );
#endif
    }

    void Event::set ( )
    {
#ifdef WIN32
        SetEvent( m_handle );
#else
        pthread_cond_signal( &m_condition );
#endif
    }
    
    
    
    
    Semaphore::Semaphore()
    {
#ifdef WIN32
        m_handle = CreateSemaphore( NULL, 0, MAX_SEMAPHORE_COUNT, NULL );
#else                    
#ifdef __MACH__
        uuid_t out;
        uuid_generate( out );
        
        char name[16];
        char* current = name;
        for ( unsigned int i = 0; i < sizeof(name); i++ )
        {
            sprintf( current, "%0x", out[ i ] );
            current ++;
        }
        
        m_handle = sem_open( name,  O_CREAT, S_IRUSR | S_IWUSR, 0 );
        
        
#else
        sem_init( &m_handle, 0, 0 );
#endif
#endif
    }

    Semaphore::~Semaphore()
    {
#ifdef WIN32
        CloseHandle( m_handle );
#else                
#ifdef __MACH__
        sem_destroy( m_handle );
#else
        sem_destroy( &m_handle );
#endif
#endif
    }

    void Semaphore::post()
    {
#ifdef WIN32
        ReleaseSemaphore( m_handle, 1, NULL );
#else        
#ifdef __MACH__
        sem_post( m_handle );
#else
        sem_post( &m_handle );
#endif
#endif        
    }
    void Semaphore::wait()
    {
#ifdef WIN32
        WaitForSingleObject( m_handle, INFINITE );
#else        
#ifdef __MACH__
        int res = sem_wait( m_handle );
#else
        sem_wait( &m_handle );
#endif
#endif        
    }

    void Semaphore::setValue( unsigned int value )
    {
        for ( int i = 0; i < value; i++ )
        {
            post();
        }
    }

    //
    //	Thread
    //

    Thread::Thread ( )
    : m_started ( false ), m_stackSize( 0 )
    {
        
    }

    Thread::~Thread ( )
    {
      //  stop();
    }

    void Thread::start ( )
    {
        TRACE_ENTERLEAVE();
        
        m_started = true;
        
#ifdef WIN32
        m_handle = CreateThread( 0, m_stackSize, routineStatic, this, 0, NULL );
#else
        pthread_attr_t attributes;
        
        pthread_attr_init( &attributes );
        
        if ( m_stackSize )
        {
            
            int result = pthread_attr_setstacksize( &attributes, m_stackSize );
        }
        
        pthread_create( &m_handle, &attributes, ( void *( * )( void* ) ) routineStatic, this );

#endif

    }

    intptr_t Thread::currentId()
    {
#ifdef WIN32
        return ( intptr_t ) GetCurrentThreadId();
#else
        return ( intptr_t ) pthread_self();
#endif
    }
    
    void Thread::exit()
    {
        
#ifdef WIN32
        ExitThread( 0 );
#else
        int ret = 0;
        pthread_exit( &ret );
#endif
   
        
    }
    
    void Thread::stop( )
    {
        
        if ( !m_started )
        {
            return;
        }
        
        join();
        cleanup();

        m_started = false;
    }


#ifdef WIN32
    DWORD WINAPI Thread::routineStatic ( void* data )
#else

    void Thread::routineStatic ( void* data )
#endif
    {
        Thread* instance = reinterpret_cast < Thread* > ( data );

        if ( instance )
        {
            instance->routine( );
            instance->stop();
        }

#ifdef WIN32
        return 0;
#endif
    }

    void Thread::cleanup( )
    {

#ifdef WIN32
        CloseHandle( m_handle );
#else
        pthread_detach( m_handle );
#endif
    }

    void Thread::join ( ) const
    {

#ifdef WIN32

        WaitForSingleObject( m_handle, INFINITE );
#else
        void* ret = NULL;
        
        pthread_join( m_handle, &ret );
#endif
    }
    
    bool Socket::s_initialized = false;

    void Socket::startup ( )
    {
#ifdef	WIN32
        WORD versionRequested;
        WSADATA wsaData;
        versionRequested = MAKEWORD( 2, 2 );
        WSAStartup( versionRequested, &wsaData );
#endif
    }

    void Socket::cleanup ( )
    {
#ifdef	WIN32
        WSACleanup( );
#endif
    }

    Socket::Socket ( Type type )
    : m_type( type ), m_socket( 0 )
    {
        #ifdef WIN32
        if ( !s_initialized )
        {
            startup( );
            s_initialized = true;
        }
#endif

    }

    Socket::Socket ( const Socket& socket )
    {
        m_socket = socket.m_socket;
    }

    Socket::Socket ( const SOCKET& socket, const Ip& peer )
    : m_socket ( socket ), m_peer( peer )
    {
    }

    Socket::~Socket ( )
    {
        if ( !m_socket )
        {
            return;
        }
        TRACE_ENTERLEAVE();
        
#ifdef  WIN32
        ::shutdown( m_socket, SD_BOTH );

        ::closesocket( m_socket );
#else
        ::close( m_socket );
#endif
    }

    void Socket::create( int af )
    {




        if ( m_type == Tcp )
        {
            TRACE( "creating tcp socket with af %d", af );
        
#ifdef WIN32
            m_socket = ::socket( af, SOCK_STREAM, IPPROTO_TCP );
#else
            m_socket = ::socket( af, SOCK_STREAM, 0 );
#endif
        }
        else
        {   TRACE( "creating udp socket", "" );
#ifdef WIN32
            m_socket = ::socket( af, SOCK_DGRAM, IPPROTO_UDP );
#else
            m_socket = ::socket( af, SOCK_DGRAM, 0 );
#endif
        }

#ifndef WIN32
         //
         // set socket options on unix
         //        
        int set = 1;
        
//        if ( af == AF_INET6 )
//        {
//            setsockopt( m_socket, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&set, sizeof( set ) ); 
//        }
        
#ifdef __MACH__
        setsockopt(m_socket, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int) );
#else
        setsockopt(m_socket, SOL_SOCKET, MSG_NOSIGNAL, (void *)&set, sizeof(int) );
        setsockopt( m_socket, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &set, sizeof( set ) );
#endif
#endif	

        TRACE("setting socket non blocking mode, socket %d", m_socket);

        //
        //  set socket non blocking
        //
        libevent::General::setSocketNonBlocking( *this );
    }
    
    
    Socket::Status Socket::receive( Datagram** datagram )
    {
        TRACE_ENTERLEAVE();
        
        Status status = StatusFailed;
        struct sockaddr_storage address;
        unsigned int length = sizeof( address );
        
        *datagram = new Datagram( DATAGRAM_BUFFER_SIZE );
        unsigned int received = recvfrom(  m_socket, ( *datagram )->data, ( *datagram )->bufferLength, 0, ( struct sockaddr* )&address, ( socklen_t* ) &length );
        
        if ( received && received != SOCKET_ERROR )
        {
            status = StatusSuccess;
            
            struct sockaddr* addr = ( struct sockaddr* ) &address;
            
            Ip parsedAddress( addr, length ); 
            ( *datagram )->from = parsedAddress;
            
            ( *datagram )->length = received;
            
            TRACE( "received %d bytes from %s", received, ( *datagram )->from.address().c_str() );

        }
        else
        {
            delete *datagram;
        }
                
        return status;
    }
    
    Socket::Status Socket::receive ( char* buffer, unsigned int bufferSize, unsigned int& bytesReceived )
    {
        Status status = StatusFailed;

        int received = ::recv( m_socket, buffer, bufferSize, 0 );

        if ( received && received != SOCKET_ERROR )
        {
            bytesReceived = received;
            status = StatusSuccess;
        }

        return status;
    }

    Socket::Status Socket::shutdown ( )
    {
#ifdef WIN32
        ::shutdown( m_socket, SD_BOTH );
#else
        ::shutdown (m_socket, SHUT_WR );
#endif
        return StatusSuccess;
    }

        Socket::Status Socket::send ( const char* buffer, unsigned int length, unsigned int& bytesSent )
    {
        Status status = StatusFailed;

#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif
        int sent = ::send( m_socket, buffer, length, 0 );
        
        
        if ( sent == -1 && getLastError() == EINPROGRESS )
        {
            //
            //  not an error
            //
            sent = 0;
        }

        if ( sent != SOCKET_ERROR )
        {
            bytesSent = sent;
            status = StatusSuccess;
        }
        else
        {
            TRACE_ERROR("socket %d send error %d: ", m_socket, getLastError() );
        }

        return status;
    }

    Socket::Status Socket::listen( )
    {
        int result = ::listen( m_socket, SOMAXCONN );

        return( result == 0 ) ? StatusSuccess : StatusFailed;
    }

    Socket* Socket::accept( )
    {
        struct sockaddr_storage addr;
        socklen_t length = sizeof( addr );
        
        Socket* socket = NULL;

        SOCKET connection = ::accept( m_socket, ( struct sockaddr* ) &addr, &length );

        if ( connection != INVALID_SOCKET )
        {
            Ip address( ( struct sockaddr* ) &addr, length );
            
            TRACE( "received connection from %s:%d", address.ip.c_str(), address.port );
            socket = new Socket( connection, address );
        }

        return socket;
    }

    
    Socket::Status Socket::sendto( const std::string& ip, unsigned int port, const char* data, unsigned int length )
    {
        Address address( ip.c_str(), m_type );
              
        address.setPort( port );    
        int result = ::sendto( m_socket, data, length, 0, address.addr(), address.length );
       
        
        
        return( result == 0 ) ? StatusSuccess : StatusFailed;
    }
    
    
    Socket::Ip::Ip( struct sockaddr* address, unsigned int addressLength )
    : port( 0 )
    {
#ifdef WIN32
        char ipstr[ MAX_PATH ];
        DWORD length = MAX_PATH;
        WSAAddressToString( ( LPSOCKADDR ) address, addressLength, NULL, ipstr, &length );
        
        // if ( address->sa_family == AF_INET )
        // {
        //     port = ( ( struct sockaddr_in * ) address )->sin_port;
        // }
        // else
        // { 
        //     port = ( ( struct sockaddr_in6 * ) &address )->sin6_port;
        // }
        
        
#else
       char ipstr[256];

       if ( address->sa_family == AF_INET )
        {
            struct sockaddr_in *s = ( struct sockaddr_in * ) address;
            inet_ntop( AF_INET, &s->sin_addr, ipstr, sizeof( ipstr ) );
            port = s->sin_port;
            
        }
        else
        { 
            struct sockaddr_in6 *s = ( struct sockaddr_in6 * ) address;
            inet_ntop( AF_INET6, &s->sin6_addr, ipstr, sizeof( ipstr ) );
            port = s->sin6_port;
        }
       
       TRACE("%s", ipstr);
       
    
#endif        
       
       ip = ipstr;
    }
    
    Socket::Address::Address( const char* name, Type type )
    : family( 0 ), parsed( false )
    {
        memset( &a, 0, sizeof( a ) );
        
        struct addrinfo hints, *info;
        memset( &hints, 0, sizeof hints );

        hints.ai_socktype = type == Tcp ? SOCK_STREAM : SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;

        int res = getaddrinfo( name, NULL, &hints, &info );

        if ( !res )
        {
            family = info->ai_family;
            parsed = true; 
            
            if ( info->ai_family == AF_INET6 )
            {
                //
                //  ipv6
                //
                
                length = sizeof( sockaddr_in6 );
                
                struct sockaddr_in6* addr = ipv6();
                
                addr->sin6_addr = ( ( struct sockaddr_in6* ) info->ai_addr )->sin6_addr;
                addr->sin6_family = info->ai_family;
            }
            else
            {
                
                //
                // ipv4
                //
                length = sizeof( sockaddr_in );
                
                struct sockaddr_in* addr = ipv4();
                
                addr->sin_addr = ( ( struct sockaddr_in* ) info->ai_addr )->sin_addr;
                addr->sin_family = info->ai_family;
            }
            
            freeaddrinfo( info );
        }
        
    }
    
   
    Socket::Status Socket::bind ( unsigned int port, const char* host )
    {
        TRACE_ENTERLEAVE();
        
        Address address ( host, m_type );
       
        if ( !address.parsed ) 
        {
            //
            //  default address
            //  
            address.family = AF_INET;
            address.ipv4()->sin_addr.s_addr = htonl( INADDR_ANY );
            address.length = sizeof( sockaddr_in );
        }
        
        address.setPort( port );   
        create( address.family );
        
        TRACE( "binding to address %s:%d", host, port );
        
        int result = ::bind( m_socket, address.addr(), address.length );
        
        return( result == 0 ) ? StatusSuccess : StatusFailed;
    }
    
    

    unsigned int Socket::getLastError ( )
    {
#ifdef WIN32
        return( unsigned int ) WSAGetLastError( );
#else
        return errno;
#endif
    }

    SOCKET Socket::s( ) const
    {
        return m_socket;
    }
        
    ThreadPool::ThreadPool( )
    : m_stop( false )
    {
        TRACE_ENTERLEAVE( );
    }

    ThreadPool::~ThreadPool( )
    {
        TRACE_ENTERLEAVE( );

        stop();
        
        while ( !m_threads.empty( ) )
        {
            Thread* thread = m_threads.front( );

            delete thread;
            m_threads.pop_front( );
        }
    }

    void ThreadPool::queue( Task* task )
    {
        TRACE_ENTERLEAVE( );

        if ( m_stop )
        {
            return;
        }

        {
            LockEnterLeave lock( m_lock );
            m_queue.push_back( task );
        }
        
        m_semaphore.post( );
    }

    void ThreadPool::start( unsigned int threads )
    {
        TRACE_ENTERLEAVE( );
        int i = 0;

        while ( i < threads )
        {
            
            Worker* thread = new Worker( *this, i );

            //
            //  invoke callback
            //
            
            onThreadStart( *thread );
            m_threads.push_back( thread );
            i++;
            thread->start( );
        }
    }

    void ThreadPool::stop( )
    {
        TRACE_ENTERLEAVE( );

        if ( m_stop )
        {
            return;
        }

        m_stop = true;
        
        {
            sys::LockEnterLeave lock( m_lock );

            while ( !m_queue.empty( ) )
            {
                Task* task = m_queue.front( );
                delete task;
                m_queue.pop_front( );
            }
        }

        m_semaphore.setValue( m_threads.size( ) );


    }

    ThreadPool::Task* ThreadPool::get( )
    {
        TRACE_ENTERLEAVE( );

        //
        //  wait for semaphore
        //
        m_semaphore.wait( );

        if ( m_stop )
        {
            return NULL;
        }

        //
        //  return next task to process or wait if the queue is empty
        //
        LockEnterLeave lock( m_lock );

        if ( m_queue.empty( ) )
        {
            return get();
        }

        Task* task = m_queue.front( );
        m_queue.pop_front( );
        
        return task;
    }
    

    ThreadPool::Worker::Worker( ThreadPool& pool, unsigned int workerId )  
    : m_pool( pool ), m_workerId( workerId )
    {
        TRACE_ENTERLEAVE( );

        m_stackSize = 1024 * 1024 * 4;
    }

    ThreadPool::Worker::~Worker( )
    {
        TRACE_ENTERLEAVE( );
    }
    
    void ThreadPool::Worker::routine( )
    {
        TRACE_ENTERLEAVE( );

        for ( ;; )
        {
            Task* task = m_pool.get( );

            if ( task )
            {
                //
                //  process data
                //
                m_pool.onTaskProcess( task, *this );
            }
            else
            {
                m_pool.onThreadStop( *this );
                return;
            }
        }
    }
}


