#include <propeller.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <execinfo.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>

void* server = NULL;

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}


void onRequest( const void* request, void* response, void* data, void* threadData )
{
    propeller_responseSetBody( response, "test", 0 );
}

void onThreadStarted( void** threadData, void* data, void* lock )
{
    *threadData = ( void* ) 5;
}

void onTimer( void* data )
{
    
}

void handlerSigint( int sig )
{
    propeller_serverDestroy( server );
}

int main ( int argc, char** argv )
{

        //rlimit limit;
        
     signal(SIGSEGV, handler); 
     signal(SIGINT, handlerSigint); 
    
    
    //
    //  create server
    //
    server = propeller_serverCreate( 8080 );
    //
    // set hadler callback
    //
    propeller_serverSetOnRequestCallback( server, onRequest, NULL );

    propeller_serverSetOnThreadStartedCallback( server, onThreadStarted, NULL );
    
    propeller_serverSetTimer( server, 10, onTimer, NULL );
    
    
    //
    //  start server
    //
    propeller_serverStart( server );

    return 0;
}

