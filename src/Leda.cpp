#include "Leda.h"
#include "trace.h"

#include <stdexcept>

 #include "Client.h"
 #include "Server.h"
 #include "HttpServer.h"

//
// exported c functions
//
 int serverAddTimer( lua_State* lua )
 {
     TRACE_ENTERLEAVE();
     
     int callback = luaL_ref( lua, LUA_REGISTRYINDEX );
     bool once = lua_toboolean( lua, -1 );
     int interval = lua_tointeger( lua, -2 );
     
    
     Server* server = ( Server* ) Leda::instance()->server();
     server->addTimer( lua, interval, once, new Leda::TimerData( callback, once ) );
     
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
             
     Client::Connection* connection = Leda::instance()->client()->connect( host, port, LuaState::getThreadId( lua ), secure );
     
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
         TRACE( "no client instance found", "" );
         return 0;
     }
     
     int callback = luaL_ref( lua, LUA_REGISTRYINDEX );
     
     int interval = lua_tointeger( lua, -2 );
     bool once = lua_toboolean( lua, -1 );
     lua_pop( lua, 2 );
     
     Client* client = ( Client* ) Leda::instance()->client();
     
     client->addTimer( lua, interval, once, new Leda::TimerData( callback, once ) );

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
    propeller::http::Request* request = ( propeller::http::Request* ) lua_touserdata( lua, -1 );
    lua_pop( lua, 1 );

    lua_pushstring( lua, request->uri( ) );
    return 1;
}

int httpRequestGetMethod( lua_State* lua )
{
    propeller::http::Request* request = ( propeller::http::Request* ) lua_touserdata( lua, -1 );
    lua_pop( lua, 1 );

    lua_pushstring( lua, request->method( ) );
    return 1;
}

int httpRequestGetHeaders( lua_State* lua )
{
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
    TRACE("response status %d", lua_tonumber( lua, -1 ));
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


Leda* Leda::m_instance = NULL;

Leda::Leda( )
:  m_client( NULL ), m_server( NULL ), m_serverType( ServerHttp ), m_debug( false ), m_fwatcher( NULL ), m_changes( 0 )
{
    TRACE_ENTERLEAVE();
    
    if ( getenv( "LEDA_DEBUG" ) )
    {
        m_debug = true;
    }
}

Leda::~Leda( )
{
    m_instance = NULL;
}

Leda* Leda::instance()
{
    if ( !m_instance )
    {
        m_instance = new Leda( );
    }

    return m_instance;
}

 void Leda::callTimer( LuaState& lua, TimerData* data )
 {
     TRACE_ENTERLEAVE();
 
     lua.call( "", data->index );
 
     if ( data->once )
     {
         //
         // remove callback function from registry
         //
         luaL_unref( lua, LUA_REGISTRYINDEX, data->index );
         delete data;
     }
 }
 
 void Leda::serverCreate( lua_State* lua )
 {
     TRACE_ENTERLEAVE( );
     
     lua_getfield( lua, -1, "type" );
     std::string type = lua_tostring( lua, -1 );
     lua_pop( lua, 1 );
     
     propeller::Server* server;
     
     if ( type == "http" )
     {
 
         m_serverType = ServerHttp;
         m_server = new HttpServer();
         m_server->setConnectionReadTimeout( 30 );
         m_server->setConnectionWriteTimeout( 30 );
     }
     else
     {
         propeller::Server::Type serverType = propeller::Server::Tcp;
         if ( type == "udp" )
         {
             serverType = propeller::Server::Udp;
         }
         
                 
         m_server = new Server( serverType );
         m_serverType = ServerMessage;
     }
 
     
     //
     //  get server options
     //
     lua_pushnil( lua );
     while ( lua_next( lua, -2 ) != 0 )
     {
         const char* name = lua_tostring( lua, -2 );
         
         if ( !strcmp( name, "port" ) )
         {            
             m_server->setPort( lua_tonumber( lua, -1 ) );
         }
         
         if ( !strcmp( name, "host" ) )
         {
             m_server->setHost( lua_tostring( lua, -1 ) );
         }
         
         if ( !strcmp( name, "connectionReadTimeout" ) )
         {
             m_server->setConnectionReadTimeout( lua_tonumber( lua, -1 ) );
         }
 
         if ( !strcmp( name, "connectionWriteTimeout" ) )
         {
             m_server->setConnectionWriteTimeout( lua_tonumber( lua, -1 ) );
         }
 
 
 
        if ( !strcmp( name, "threads" ) )
        {
            if ( !m_debug )
            {
                m_server->setThreadCount( lua_tonumber( lua, -1 ) );
            }
            
         }
 
         lua_pop( lua, 1 );
     }

    if ( m_debug )
    {
        TRACE( "enabling debug mode", "" );

        m_server->setThreadCount( 1 );
    }
    
             

     
     lua_pop( lua, 1 );
     
     TRACE( "created %s server on %s:%d" , type.c_str(), m_server->host(), m_server->port() );
 }

void Leda::execScript( )
{
    TRACE_ENTERLEAVE( );

    if ( getenv( "LEDA_WATCH_TREE" ) )
    {
        //
        //  watch for file changes
        //
        m_fwatcher = new FWatcher( "./" );
    }

    //
    //  create new lua environment
    //
    m_lua = newLua();
    
    m_lua->setGlobal( "init" );
    m_lua->load( 0, true );

    
    if ( m_client )
    {
        m_client->start( false );
    }

    if ( m_server )
    {
        try
        {
            m_server->start( );
        }
        catch( ... )
        {
            throw std::runtime_error( "" );
        }
    }
    else
    {
        if ( m_client )
        {
            m_client->join( );
        }
    }
 }

 void Leda::onTerminate()
 {
     TRACE_ENTERLEAVE();

     if ( m_server )
     {
         m_server->stop();
         delete m_server;
         m_server = NULL;
     }
  
     if ( m_client )
     {
         m_client->stop();
         delete m_client;
         m_client = NULL;
         
     }
 
 }
 
LuaState* Leda::newLua()
{
    return new LuaState( m_script, &m_scriptArguments );
}
 
propeller::Client* Leda::clientCreate( unsigned int threadCount )
{
    TRACE_ENTERLEAVE( );
    m_client = new Client( threadCount );
    
    return m_client;
}

void Leda::addFileChange()  
{
    TRACE_ENTERLEAVE();
    sys::General::interlockedIncrement( &m_changes );
    
}

void Leda::resetChanges()  
{
    TRACE_ENTERLEAVE();
 
    for ( unsigned int i = 0; i < m_changes; i++ )
    {
        sys::General::interlockedDecrement( &m_changes );
    }
    
}