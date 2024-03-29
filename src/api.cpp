#include "api.h"
#include "Leda.h"
#include "HttpServer.h"


void getTimeval( lua_State* lua, struct timeval* timeout )
{

    TRACE_ENTERLEAVE( );

    memset( timeout, 0, sizeof( struct timeval ) );

    if ( lua_isnumber( lua, -1 ) )
    {
        timeout->tv_sec = lua_tointeger( lua, -1 );
    }
    else if ( lua_istable( lua, -1 ) )
    {
        lua_pushnil( lua );

        while ( lua_next( lua, -2 ) != 0 )
        {
            std::string name = lua_tostring( lua, -2 );

            unsigned int value = lua_tointeger( lua, -1 );

            TRACE( "%s %d", name.c_str( ), value );
            if ( name == "sec" )
            {
                timeout->tv_sec = value;
            }

            if ( name == "usec" )
            {
                timeout->tv_usec = value;
            }

            if ( name == "msec" )
            {
                timeout->tv_usec = value * 1000;
            }

            lua_pop( lua, 1 );
        }
    }

    lua_pop( lua, 1 );
}

//
// exported c functions
//
 int serverAddTimer( lua_State* lua )
 {
     TRACE_ENTERLEAVE();
     
     int callback = luaL_ref( lua, LUA_REGISTRYINDEX );
     bool once = lua_toboolean( lua, -1 );
     lua_pop( lua, 1 );
     
     struct timeval timeout;
     getTimeval( lua,  &timeout );

     Leda::instance()->server()->addTimer( &timeout, once, new Leda::TimerData( callback, once ) );
     
     return 0;
 }
 
 int serverCreate( lua_State* lua )
 {
     TRACE_ENTERLEAVE();
     
     if ( !Leda::instance()->server() )
     {
         Leda::instance()->serverCreate( lua );
     }
     
     return 0;
 }
 
 
 int serverConnectionSendData( lua_State* lua )
 {
     propeller::Server::Connection* connection = ( propeller::Server::Connection* ) lua_touserdata( lua, -2 );   
     size_t size = 0;
     
     if ( lua_isnil( lua, -1 ) )
     {
         return 0;
     }
     
     const char* data = lua_tolstring( lua, -1, &size );
     
     connection->write( data, size );
     lua_pop( lua, 2 );
     
     return 0;
 }
 
 int serverConnectionGetAddress( lua_State* lua )
 {
    propeller::Server::Connection* connection = ( propeller::Server::Connection* ) lua_touserdata( lua, -1 );   
    lua_pop( lua, 1 );
    
    lua_pushstring( lua, connection->address().c_str() );
    
    return 1;
 }
 
 int serverConnectionGetId( lua_State* lua )
 {
    propeller::Server::Connection* connection = ( propeller::Server::Connection* ) lua_touserdata( lua, -1 );   
    lua_pop( lua, 1 );
    
    lua_pushnumber( lua, ( unsigned int ) connection->fd() );
    return 1;
 }
 
 
 
 int threadGetId( lua_State* lua )
 {
     TRACE_ENTERLEAVE();
     
     propeller::Server::Thread* thread = ( propeller::Server::Thread* ) lua_touserdata( lua, -1 );   
     
     
     lua_pop( lua, 1 );
     
     lua_pushinteger( lua, thread->id() );
     
     return 1;
 }
 
 int serverSendTo( lua_State* lua )
 {
     TRACE_ENTERLEAVE();
     
     size_t size = 0;
     
     const char* data = lua_tolstring( lua, -1, &size );
     unsigned int port = lua_tointeger( lua, -2 );
     const char* ip = lua_tostring( lua, -3 );
     
     TRACE( "%s:%d", ip, port );
     
     Leda::instance()->server()->sendTo( ip, port, data, size );
     
     lua_pop( lua, 3 );
     
     return 0;
     
 }
 
 
// 
 int clientCreate( lua_State* lua )
 {
     TRACE_ENTERLEAVE();
     
     if ( !Leda::instance()->client() )
     {
         propeller::Client* client = Leda::instance()->clientCreate( lua_tointeger( lua, -1 ) );
         lua_pop( lua, 1 );
         
         lua_pushlightuserdata( lua, client );
         
         return 1;
     }
             
     return 0;
 }
// 
 int clientConnect( lua_State* lua )
 {
     TRACE_ENTERLEAVE();
     
     if ( !Leda::instance()->client() )
     {
         Leda::instance()->clientCreate();
     }
     
     bool secure = lua_toboolean( lua, -1 );
     unsigned int port = lua_tonumber( lua, -2 );
     const char* host = lua_tostring( lua, -3 );
     
     TRACE( "secure: %d, host: %s, port: %d", secure, host, port );
     
     if ( lua_isnil( lua, -1 ) || lua_isnil( lua, -2 ) || lua_isnil( lua, -3 ) )
     {
         TRACE( "some of the passed arguments were invalid", "" );
         lua_pop( lua, 3 );
         return 0;
     }
             
     propeller::Client::Connection* connection = Leda::instance()->client()->connect( host, port, secure );
     
     lua_pop( lua, 3 );
     
     if ( connection )
     {
         lua_pushlightuserdata( lua, connection );
     }
     else
     {
         
         lua_pushnil( lua );
     }
     
     return 1;
 }
// 
 int clientAddTimer (lua_State* lua )
 {
     TRACE_ENTERLEAVE();
     if ( !Leda::instance()->client() )
     {
         Leda::instance()->clientCreate();
     }
     
     int callback = luaL_ref( lua, LUA_REGISTRYINDEX );
     
     bool once = lua_toboolean( lua, -1 );
     lua_pop( lua, 1 );
     
     struct timeval timeout;
     getTimeval( lua, &timeout );
     
     Leda::instance()->client()->addTimer( &timeout, once, new Leda::TimerData( callback, once ) );

     return 0;
 }
 
 int clientConnectionSendData( lua_State* lua )
 {
     TRACE_ENTERLEAVE();
     
     if ( lua_isnil( lua, -2 ) )
     {
         return 0;
     }
     
     propeller::Client::Connection* connection = ( propeller::Client::Connection* ) lua_touserdata( lua, -2 );
     
//     if ( !Leda::instance()->client()->connectionExists( connection ))
//     {
//         TRACE_VERBOSE("attempt to send on non existing connection", "");   
//         return 0;
//     }
     
     
     if ( lua_isnil( lua, -1 ) )
     {
         return 0;
     }
     
 
     
     size_t size = 0;
     const char* data = lua_tolstring( lua, -1, &size ); 
     
     connection->write( data, size );
     
     lua_pop( lua, 2 );
     return 0;
 }
// 
 int clientConnectionClose( lua_State* lua )
 {
     TRACE_ENTERLEAVE();
     
     if ( lua_isuserdata( lua, -1 ) )
     {
        propeller::Client::Connection* connection = ( propeller::Client::Connection* ) lua_touserdata( lua, -1 );
     
        connection->deref();
        lua_pop( lua, 1 );  
     }
                
     return 0;
 }
// 
 
 int getpid( lua_State* lua )
 {
     lua_pushnumber( lua, sys::General::getProcessId() );
     
     return 1;
 }
 
 int processorCount( lua_State* lua )
 {
     unsigned int processors = 0;
#ifdef WIN32
     SYSTEM_INFO sysinfo;
    GetSystemInfo( &sysinfo );

    processors = sysinfo.dwNumberOfProcessors;
#else
    processors = sysconf( _SC_NPROCESSORS_ONLN  );
#endif
            
     lua_pushnumber( lua, processors );

     return 1;
 }
 

int httpRequestGetUrl( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    
    propeller::http::Request* request = ( propeller::http::Request* ) lua_touserdata( lua, -1 );
    lua_pop( lua, 1 );
        
    lua_pushstring( lua, request->uri( ) );
    return 1;
}

int httpRequestGetMethod( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    
    propeller::http::Request* request = ( propeller::http::Request* ) lua_touserdata( lua, -1 );
    lua_pop( lua, 1 );

    lua_pushstring( lua, request->method( ) );
    return 1;
}

int httpRequestGetHeaders( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    
    propeller::http::Request* request = ( propeller::http::Request* ) lua_touserdata( lua, -1 );
    lua_pop( lua, 1 );

    lua_newtable( lua );

    for ( propeller::http::Request::HeaderList::const_iterator i = request->headers( ).begin( ); i != request->headers( ).end( ); i++ )
    {
        propeller::http::Request::Header header = *i;

        lua_pushstring( lua, header.value );
        lua_setfield( lua, -2, header.name );
    }

    return 1;
}

int httpRequestGetBody( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    
    propeller::http::Request* request = ( propeller::http::Request* ) lua_touserdata( lua, -1 );
    lua_pop( lua, 1 );

    if ( request->body() )
    {
        lua_pushlstring( lua, request->body(), request->bodyLength() );
    }
    else
    {
        lua_pushnil( lua );
    }
        
    return 1;
}

int httpRequestGetAddress( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    
    propeller::http::Request* request = ( propeller::http::Request* ) lua_touserdata( lua, -1 );
    lua_pop( lua, 1 );

    lua_pushstring( lua, request->connection()->address().c_str() );
        
    return 1;
}

int httpResponseSetBody( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    
    propeller::http::Response* response = ( propeller::http::Response* ) lua_touserdata( lua, -2 );
    
    
    size_t length = 0;
    
    const char* body = lua_tolstring( lua, -1, &length );
    
    TRACE("length: %d", length );
    
    
    response->setBody( body, length );
    lua_pop( lua, 2 );
    
    return 0;
}

int httpResponseSetStatus( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    propeller::http::Response* response = ( propeller::http::Response* ) lua_touserdata( lua, -2 );
    
    response->setStatus( lua_tonumber( lua, -1 ) );
    lua_pop( lua, 2 );
    
    return 0;
}

int httpResponseSetHeaders( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    
    propeller::http::Response* response = ( propeller::http::Response* ) lua_touserdata( lua, -2 );

    //
    //  iterate through headers table
    //
    if ( lua_istable( lua, -1 ) )
    {
        lua_pushnil( lua );
    
        while ( lua_next( lua, -2 ) != 0 )
        {
            const char* name = lua_tostring( lua, -2 );

            if ( lua_isstring( lua, -1 ) )
            {
                const char* value = lua_tostring( lua, -1 );
                response->addHeader( name, value );
            }

            lua_pop( lua, 1 );
        }
    }
    
    lua_pop( lua, 2 );
    
    return 0;
}

int httpResponseAddHeader( lua_State* lua )
{
    propeller::http::Response* response = ( propeller::http::Response* ) lua_touserdata( lua, -3 );

    const char* name = lua_tostring( lua, -2 );
    const char* value = lua_tostring( lua, -1 );
    
    response->addHeader( name, value );
    lua_pop( lua, 3 );
    
    
    return 0;
}

int getVersion( lua_State* lua )
{
    lua_pushstring( lua, Leda::instance()->version().c_str() );
    return 1;
}

int dictionarySet( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    
    std::string key = lua_tostring( lua, -2 );
    std::string value = lua_tostring( lua, -1 );
    
    TRACE( "%s: %s", key.c_str(), value.c_str() );
    
    Leda::instance()->dictionary()->Put( leveldb::WriteOptions(), key, value );

    lua_pop( lua, 2 );

    return 0;    
}

int dictionaryGet( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    
    std::string key = lua_tostring( lua, -1 );
    std::string value;
    
    leveldb::Status status = Leda::instance()->dictionary()->Get( leveldb::ReadOptions(), key, &value );
    lua_pop( lua, 1 );
    
    TRACE( "%s", value.c_str() );
    
    if ( !value.empty() )
    {
        lua_pushstring( lua, value.c_str() );
    }
    else
    {
        lua_pushnil( lua );
    }

    return 1;    
}

int dictionaryRemove( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    
    std::string key = lua_tostring( lua, -1 );
    TRACE( "%s", key.c_str() );
    
    Leda::instance()->dictionary()->Delete( leveldb::WriteOptions(), key ); 
    lua_pop( lua, 1 );
    
    return 0;    
}

int dictionaryGetKeys( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    
    lua_newtable( lua );
    leveldb::Iterator* it = Leda::instance()->dictionary()->NewIterator( leveldb::ReadOptions() );
    unsigned int index = 1;
    for ( it->SeekToFirst(); it->Valid(); it->Next() ) 
    {
        std::string key = it->key().ToString();
        lua_pushinteger( lua, index );
        lua_pushstring( lua, key.c_str() );
        lua_settable( lua, -3 );
        index ++;
    }
    
    return 1;    
}

int processStart( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    std::string command = lua_tostring( lua, -1 );
    
    TRACE( "%s", command.c_str() );
    
    propeller::Client::ChildProcess* process = Leda::instance()->client()->processStart( command );
    lua_pop( lua, 1 );
    
    lua_pushlightuserdata( lua, process );
    return 1;
}

int processWrite( lua_State* lua )
{
    TRACE_ENTERLEAVE();
    
    if ( lua_isnil( lua, -2 ) )
    {
        TRACE( "no process instance passed", "" );
        lua_pop( lua, 2 );
        return 0;
    }
            
    propeller::Client::ChildProcess* process = ( propeller::Client::ChildProcess* ) lua_touserdata( lua, -2 );
    
    const char* data = NULL;
    size_t length = 0;
    
    if ( lua_isstring( lua, -1 ) )
    {
        data = lua_tolstring( lua, -1, &length );
    }

    process->write( data, length );
    
    return 0;
}