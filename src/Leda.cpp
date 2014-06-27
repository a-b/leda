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
 
 
// int generateRandomString( lua_State* lua )
// {
//     TRACE_ENTERLEAVE();
//     
//     uuid_t out;
//     uuid_generate( out ); 
//   
//     char hex[32];
//     char* current = hex;
//     
//     for ( unsigned int i = 0; i < sizeof(uuid_t); i++ )
//     {
//         sprintf( current, "%x2", out[ i ] );
//         current +=2;
//     }
//     
//     lua_pushlstring( lua, hex, 32 );
//     
//     return 1;
// }
// 
 
 int serverConnectionSendMessage( lua_State* lua )
 {
     propeller::Server::Connection* connection = ( propeller::Server::Connection* ) lua_touserdata( lua, -2 );   
     size_t size = 0;
     
     const char* data = lua_tolstring( lua, -1, &size );
     
     connection->sendMessage( data, size );
     lua_pop( lua, 2 );
     
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
         Leda::instance()->clientCreate();
     }
             
     return 0;
 }
// 
// int clientConnect( lua_State* lua )
// {
//     TRACE_ENTERLEAVE();
//     
//     unsigned int port = lua_tonumber( lua, -1 );
//     const char* host = lua_tostring( lua, -2 );
//     
//     lua_pop( lua, 2 );
//     
//     Leda::instance()->client()->connect( host, port );
//     
//     return 0;
// }
// 
 int clientAddTimer (lua_State* lua )
 {
     TRACE_ENTERLEAVE();
     
     
     int callback = luaL_ref( lua, LUA_REGISTRYINDEX );
     
     int interval = lua_tointeger( lua, -2 );
     bool once = lua_toboolean( lua, -1 );
     lua_pop( lua, 2 );
     
     
     Leda::instance()->client()->addTimer( interval, once, new Leda::TimerData( callback, once ) );
     
     return 0;
 }
// 
// int clientConnectionSendMessage( lua_State* lua )
// {
//     TRACE_ENTERLEAVE();
//     
//     propeller::Client::Connection* connection = ( propeller::Client::Connection* ) lua_touserdata( lua, -2 );
//     
//     size_t size = 0;
//     const char* data = lua_tolstring( lua, -1, &size ); 
//     
//     
//     connection->sendMessage( data, size );
//     
//     lua_pop( lua, 2 );
//     return 0;
// }
// 
// int clientConnectionClose( lua_State* lua )
// {
//     TRACE_ENTERLEAVE();
//     
//     propeller::Client::Connection* connection = ( propeller::Client::Connection* ) lua_touserdata( lua, -1 );
//     
//     lua_pop( lua, 1 );
//     
//     
//     return 0;
// }
// 
 
 int getpid( lua_State* lua )
 {
     lua_pushnumber( lua, sys::General::getProcessId() );
     
     return 1;
 }


Leda* Leda::m_instance = NULL;

Leda::Leda( )
:  m_client( NULL ), m_server( NULL ), m_serverType( ServerHttp ), m_debug( false )
{
    TRACE_ENTERLEAVE();
    
    if (getenv( "LEDA_DEBUG" ) )
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
         else if (type == "simple")
         {
             serverType = propeller::Server::Simple;
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
 
         if ( !strcmp( name, "pool" ) )
         {
             if ( m_serverType == ServerHttp )
             {
                 if ( !m_debug )
                 {
                     ( ( propeller::http::Server* ) m_server )->setPoolThreadCount( lua_tonumber( lua, -1 ) );
                 }
             }
         }
 
        if ( !strcmp( name, "threads" ) )
        {
            if ( !m_debug )
            {
                m_server->setThreadCount( lua_tonumber( lua, -1 ) );
            }
            
         }
 
         if ( !strcmp( name, "debug" ) )
         {
             m_debug = lua_toboolean( lua, -1 );
             
             if ( m_serverType == ServerHttp )
             {
                 ( ( propeller::http::Server* ) m_server )->setPoolThreadCount( 1 );
             }
             
             m_server->setThreadCount( 1 );            
             
         }
 
         lua_pop( lua, 1 );
     }
     
     lua_pop( lua, 1 );
     
     TRACE( "created %s server on %s:%d" , type.c_str(), m_server->host(), m_server->port() );
 }

void Leda::ClientWorkerThread::routine()
{
    TRACE_ENTERLEAVE();
    
    if ( Leda::instance()->client() )
    {
        Leda::instance()->client()->start();
    }
}

void Leda::execScript( )
{
    TRACE_ENTERLEAVE( );

    //
    //  create new lua environment
    //
    m_lua = new LuaState( m_script );
    if ( !m_lua->load( "__init=true" ) )
    {
        throw std::runtime_error( "" );
    }

    if ( m_server )
    {
        if ( m_client )
        {
            //
            //    start client in a new thread
            //
            ( new ClientWorkerThread( ) )->start( );
        }

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
            m_client->start( );
        }
    }
}

 void Leda::onTerminate()
 {
     TRACE_ENTERLEAVE();

     if ( m_server )
     {
         m_server->stop();
         m_server = NULL;

     }

     if ( m_client )
     {
         m_client->stop();
         m_client = NULL;
     }
 }

void Leda::clientCreate( )
{
    TRACE_ENTERLEAVE( );
    m_client = new Client( );
}

  