#ifndef _TRACE_H_
#define _TRACE_H_

#include <string.h>
#include <string>
#include <typeinfo>


#define	TRACE_LEVEL_INFO			3
#define TRACE_LEVEL_WARNING			2	
#define TRACE_LEVEL_ERROR			1	
#define TRACE_LEVEL_VERBOSE			0	


#ifdef WIN32
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

void moonTraceText ( const char* scope, unsigned long level, const char *format, ... );

class TraceEnterLeave
{
public:

    TraceEnterLeave ( const char* function )
    : m_function ( function )
    {
        size_t brace = m_function.find ( '(' );

        if ( brace != std::string::npos )
        {
            m_function[brace] = 0 ;
            m_function.append ( "()" );
        }

        std::string format = "Entering %s ";
        moonTraceText( NULL, TRACE_LEVEL_INFO, format.c_str(), m_function.c_str ( ) );
    }

    ~TraceEnterLeave ( )
    {
        moonTraceText( NULL, TRACE_LEVEL_INFO, "Leaving %s ", m_function.c_str ( ) );
    }

private:
    std::string m_function;
};

#define TRACE_VERBOSE( format, ... ) moonTraceText( __FUNCTION__, TRACE_LEVEL_VERBOSE, format, __VA_ARGS__ )

#ifdef _DEBUG
#define TRACE_ENTERLEAVE()  TraceEnterLeave trace( __PRETTY_FUNCTION__ )
#define TRACE( format, ... ) moonTraceText( __FUNCTION__, TRACE_LEVEL_INFO, format, __VA_ARGS__ )
#define TRACE_WARNING( format, ... ) moonTraceText( __FUNCTION__, TRACE_LEVEL_WARNING, format, __VA_ARGS__ )
#define TRACE_ERROR( format, ... ) moonTraceText( __FUNCTION__, TRACE_LEVEL_ERROR, format, __VA_ARGS__ )
#else
#define TRACE_ENTERLEAVE() 
#define TRACE( format, ... )
#define TRACE_WARNING( format, ... )
#define TRACE_ERROR( format, ... ) moonTraceText( __FUNCTION__, TRACE_LEVEL_ERROR, format, __VA_ARGS__ )

#endif

#endif //_TRACE_H_
