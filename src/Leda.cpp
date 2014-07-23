#include "Leda.h"
#include "trace.h"

#include <stdexcept>

 #include "Client.h"
 #include "Server.h"
 #include "HttpServer.h"


Leda* Leda::m_instance = NULL;

Leda::Leda( )
:  m_client( NULL ), m_server( NULL ), m_debug( false ), m_fwatcher( NULL ), m_changes( 0 ), m_dictionary( NULL )
{
    TRACE_ENTERLEAVE();
    
    if ( getenv( "LEDA_DEBUG" ) )
    {
        m_debug = true;
    }
}

Leda::~Leda( )
{
    TRACE_ENTERLEAVE();
    
    m_instance = NULL;
    if ( m_dictionary )
    {
        delete m_dictionary;
    }
    
    ::remove( m_dictionaryFilename.c_str() );
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
        m_server->setThreadCount( 1 );
    }
    
     lua_pop( lua, 1 );
     
     TRACE( "created %s server on %s:%d" , type.c_str(), m_server->host(), m_server->port() );
 }

void Leda::execScript( )
{
    TRACE_ENTERLEAVE( );

    if ( debug() )
    {
        //
        //  watch for file changes
        //
        m_fwatcher = new FWatcher( "./" );
    }
    
    //
    //  create dictionary
    //
    char filename[ 128 ];
    sprintf( filename, "/tmp/leda-dictionary-%d", sys::General::getProcessId() );
    
    leveldb::Options options;
    options.create_if_missing = true;
    
    leveldb::Status status = leveldb::DB::Open( options, filename, &m_dictionary );
    
    if ( !status.ok() )
    {
        TRACE_ERROR( "failed to create dictionary: %s", status.ToString().c_str() );
        throw std::runtime_error(" ");
    }
    
    m_dictionaryFilename  = filename;

    //
    //  create new lua environment
    //
    m_lua = newLua();
    
    m_lua->setGlobal( "init" );
    m_lua->load( -1, true );

    
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
     
     delete this;
 }
 
LuaState* Leda::newLua()
{
    
    return new LuaState( m_script, &m_scriptArguments );
}
 
propeller::Client* Leda::clientCreate( unsigned int threadCount )
{
    TRACE_ENTERLEAVE( );
    TRACE("creating client with %d threads", threadCount );
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

std::string Leda::version() const
{
    char buffer[ 256 ];
    sprintf( buffer, "%d.%d.%d", LEDA_VERSION_MAJOR,  LEDA_VERSION_MINOR, LEDA_VERSION_REVISION );
    return buffer;
}
