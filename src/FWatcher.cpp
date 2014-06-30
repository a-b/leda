/* 
 * File:   FSWatcher.cpp
 * Author: sergey
 * 
 * Created on June 29, 2014, 8:55 AM
 */

#include "FWatcher.h"
#include "Leda.h"

FWatcher::FWatcher( const std::string& path ) 
: m_path( path ) 
{
    TRACE_ENTERLEAVE();
    
    TRACE("watching path: '%s'", m_path.c_str() );
    
    ( new WorkerThread( this ) )->start();
}

FWatcher::~FWatcher( ) 
{
}

void FWatcher::start()
{
    TRACE_ENTERLEAVE();
    
#if __MACH__
    startOSX();
    
    CFRunLoopRun();
    
#endif
}

#ifdef __MACH__

void callback(
        ConstFSEventStreamRef streamRef,
        void *clientCallBackInfo,
        size_t numEvents,
        void *eventPaths,
        const FSEventStreamEventFlags eventFlags[],
        const FSEventStreamEventId eventIds[] )
{
    TRACE_ENTERLEAVE();
    
    int i;
    char **paths = ( char** ) eventPaths;

    // printf("Callback called\n");
    for ( i = 0; i < numEvents; i++ )
    {
        /* flags are unsigned long, IDs are uint64_t */
        TRACE( "Change %llu in %s, flags %lu\n", eventIds[i], paths[i], eventFlags[i] );
        
        Leda::instance()->addFileChange();
    }
}

#endif  

void FWatcher::startOSX()
{
    TRACE_ENTERLEAVE();
    
    CFStringRef path = CFStringCreateWithCString( NULL, m_path.c_str(), kCFStringEncodingUTF8 );
    
    FSEventStreamContext context;
    memset( &context, 0, sizeof( context ) );
    context.info = this;
    
            
    
    m_stream = FSEventStreamCreate( 
            NULL,
            &callback,
            &context,
            CFArrayCreate( NULL, ( const void ** )&path, 1, NULL ),
            kFSEventStreamEventIdSinceNow,
            1.0,
            kFSEventStreamCreateFlagNoDefer
            
   );
 
  
  FSEventStreamScheduleWithRunLoop( m_stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );

  FSEventStreamStart( m_stream ); 
    
  TRACE("created fs event stream: 0x%x", m_stream );  
}