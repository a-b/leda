#include "trace.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdarg.h>


#ifdef _DEBUG
static unsigned long TRACE_LEVEL = TRACE_LEVEL_INFO;
#else
static unsigned long TRACE_LEVEL = TRACE_LEVEL_ERROR;
#endif

void moonTraceText( const char* scope, unsigned long level, const char *format, ... )
{

    //
    // Check do we need to process request
    //
    if ( level <= TRACE_LEVEL )
    {
        char temp[ 0x10000 ];
        unsigned int bytes = 0;

        va_list next;

        va_start( next, format );

        //
        // Format the string
        //
        memset( &temp, 0, sizeof( temp ) );

        if ( level == TRACE_LEVEL_ERROR )
        {
            strcat( temp, "*Error*: " );
        }
        else if ( level > TRACE_LEVEL_ERROR )
        {
            if ( scope )
            {
                sprintf( temp + strlen( temp ), "%s: ", scope );
            }
        }
        

        vsnprintf( temp + strlen( temp ), sizeof( temp ) - strlen( temp ) - 1, format, next );

        //
        //	append new line
        //
        strcat( temp, "\n" );

        //
        // Dump just generated string to the debug output
        //
        //#ifdef WIN32
        //      OutputDebugStringA( temp );
        //#else
        fprintf( stderr, "%s", temp );
        //#endif

        va_end( next );
    }
}

