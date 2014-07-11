
#include "Leda.h"
#include <stdexcept>  

#include <stdio.h>

#ifndef WIN32
#include <signal.h>
#include <execinfo.h>

void sigSegvHandler( int sig ) 
{
  void *array[10];
  size_t size;

  size = backtrace(array, 10);

  backtrace_symbols_fd(array, size, STDERR_FILENO);
  
  exit( 1 );
}

void sigKillHandler( int sig ) 
{
    Leda::instance()->onTerminate();  
    exit( 1 );
}

#else

BOOL ctrlHandler( DWORD ctrlType )
{
	switch ( ctrlType )
	{
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
		Leda::instance( )->onTerminate( );
		exit( 1 );
		return TRUE;

	default:
		return FALSE;
	}
}

#endif

class CmdOption
{
public:
    CmdOption( const std::string& optShort, const std::string& optLong, const std::string& description, std::string name = "", bool needValue = false )
    : m_long( optLong ), m_short( optShort ), m_description( description ), m_needValue( needValue ), m_name( name )
    {
        m_value[0] = 0;
    }

    CmdOption( const CmdOption& option )
    {
        m_value[0] = 0;
        m_long = option.m_long;
        m_short = option.m_short;
        m_needValue = option.m_needValue;
        m_description = option.m_description;
        m_name = option.m_name;
    }

    std::string usage()
    {
        std::string usage;

        if ( m_short.size() > 0)
        {
            usage += m_short + ", ";
        }

        if ( m_long.size() > 0 )
        {
            usage += m_long;
            if ( m_needValue )
            {
                usage += "=val";
            }
        }

        usage +=  m_description;

        return usage;
    }

    const char* value() const
    {
        return m_value;
    }

    bool parse( const char* arg ) const
    {
        if ( strcmp( arg, m_short.c_str() ) == 0 || strstr( arg, m_long.c_str() ) )
        {
            if ( m_needValue )
            {
                std::string format = m_long + "=%s";
                if ( sscanf( arg, format.c_str(), m_value ) != 1 )
                {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    static bool isOption( const char* arg )
    {
        return arg[0] == '-';
    }

    const std::string& name() const
    {
        return m_name;
    }

private:
    std::string m_long;
    std::string m_short;
    bool m_needValue;

    std::string m_description;
    char m_value[128];
    std::string m_name;
};

std::vector< CmdOption > options;

void usage( bool help = false )
{
    printf( "Usage: %s [options] [script] [script arguments]\n\n", LEDA_NAME );
    printf("Options: \n");

    for ( int  i = 0; i < options.size(); i++ )
    {
        printf( "%s\n", options[i].usage().c_str() );
    }

    exit( 1 );
}

const CmdOption* findOption( const char* arg )
{
    for ( unsigned int i = 0; i < options.size( ); i++ )
    {
        const CmdOption* option = &options[i];
        
        if ( option->parse( arg ) )
        {
            return option;
        }
    }
    
    return NULL;
}

int main(int argc, char* argv[])
{
#ifndef WIN32
    //
    // install signal handlers
    //
    signal( SIGSEGV, sigSegvHandler );
    signal( SIGTERM, sigKillHandler );
    signal( SIGINT, sigKillHandler );
    signal( SIGABRT, sigKillHandler );
    signal( SIGURG, sigKillHandler );
    
    signal( SIGPIPE, SIG_IGN );
    
#else
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) ctrlHandler, TRUE );
#endif

    options.push_back( CmdOption( "-h", "--help", "\t\tprints help", "help" ) );
    options.push_back( CmdOption( "", "--version", "\t\tprints version", "version" ) );
    
    
    //
    //  parse command line
        //
    std::string script;
    LuaState::ScriptArguments arguments;
    
    //
    //  create instance
    //
    Leda* leda = Leda::instance();
    
    try
    {
        for ( int i = 1; i < argc; i++ )
        {
            if ( CmdOption::isOption( argv[i] ) && script.empty() )
            {

                const CmdOption* option = findOption( argv[i] );
                if ( !option )
                {
                    printf( "Unrecognized option %s \n", argv[i] );
                    throw argv[i];
                }
                if ( option->name( ) == "help" )
                {
                    usage( true );
                }
                if ( option->name( ) == "version" )
                {
                    printf( "%s  version %s \n",  LEDA_NAME, leda->version().c_str() );
                    exit(1);
                }
            }
            else
            {
                if ( script.empty() )
                {
                    script = argv[i];
                }
                else
                {
                    arguments.push_back( argv[i] );
                }
            }
        }
    }
    catch ( ... )
    {
        usage();
    }

    if ( !script.size() )
    {
        usage( true );
    }

    TRACE("command line arguments length %d", arguments.size());
    
    
    char* frameworkPath = getenv( "LEDA_PATH" );
    
    if ( !frameworkPath )
    {
#ifdef WIN32
        char path[ MAX_PATH ];
        GetModuleFileNameA( NULL, path, MAX_PATH );
        TRACE( "module filename %s", path );
        
        *strrchr( path, '\\' ) = 0;
        std::string dllPath = path;
        dllPath.append("\\lib");
        leda->addPath( dllPath );
#else
        leda->addPath( LEDA_PATH );
#endif
    }
    else
    {
        leda->addPath( frameworkPath );
    }       
            
    leda->setScriptArguments( arguments );
    //      
    //  add paths to locate lua files
    //
    std::string path;
    
    char separator = '/';
#ifdef WIN32
    separator = '\\';
#endif
    size_t lastSlash = script.rfind( separator );
    
    if ( lastSlash != std::string::npos )
    {
        path = script.substr( 0, lastSlash );
        script = script.substr( lastSlash + 1 );
    }
    
    TRACE("script %s, path %s", script.c_str(), path.c_str());
    
    //
    //  get script path
    //
    if ( path[0] != '/' )
    {
        //
        //  form the absolute path in case of relative script path
        //  
        char buffer[256] = "/0";
        
        sys::General::getCurrentDirectory( buffer, sizeof( buffer ) );
        
        leda->addPath( buffer );
        
        if ( path != "." )
        {
            std::string temp = path;
            path = buffer;
            
            if ( temp.size() )
            {
                path = path + "/" + temp;
            }
        }
    }
    
    //
    //  set working directory
    //
    sys::General::setCurrentDirectory( path.c_str() );
    
    leda->addPath( path );
    leda->setScript( script );
    
    try
    {
        leda->execScript();
        leda->onTerminate();
    }
    catch ( const std::runtime_error& e )
    {
        return 1;
    }

//#ifndef WIN32
//	catch( ... )
//    {          
//       // 
//       //   crash
//       // 
//       raise( SIGSEGV ); 
//       return 1;
//    }
//#endif       

    
    return 0;
}
