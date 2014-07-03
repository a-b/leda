#include "FWatcher.h"
#include "Leda.h"

#ifdef __linux
 #include <sys/inotify.h>
#endif

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
void FWatcher::start()
{
    TRACE_ENTERLEAVE();

#ifdef __MACH__    
    //
    //  OSX
    //

    CFStringRef path = CFStringCreateWithCString( NULL, m_path.c_str(), kCFStringEncodingUTF8 );
    
    FSEventStreamContext context;
    memset( &context, 0, sizeof( context ) );
    context.info = this;
    
            
    
    FSEventStreamRef stream = FSEventStreamCreate( 
            NULL,
            &callback,
            &context,
            CFArrayCreate( NULL, ( const void ** )&path, 1, NULL ),
            kFSEventStreamEventIdSinceNow,
            1.0,
            kFSEventStreamCreateFlagNoDefer
            
   );
 
  
  FSEventStreamScheduleWithRunLoop( stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );

  FSEventStreamStart( stream ); 

  CFRunLoopRun();
    
  TRACE("created fs event stream: 0x%x", stream );  

#endif

#ifdef __linux

    while ( true )
    {

        char buffer[ 1024 * 4 ];

        int notify = inotify_init();
        int watch = inotify_add_watch( notify, m_path.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE );
         

        int length = read( notify, buffer, sizeof( buffer ) );

        int current = 0;

        while ( current < length )
        {
             struct inotify_event *event = ( struct inotify_event * ) &buffer[ current ];
             TRACE( "file change  with mask %d", event-> mask );
             Leda::instance()->addFileChange();

             current += event->len + sizeof( struct inotify_event );
        }  
    }

#endif 

}