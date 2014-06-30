/* 
 * File:   FSWatcher.h
 * Author: sergey
 *
 * Created on June 29, 2014, 8:55 AM
 */

#ifndef FSWATCHER_H
#define	FSWATCHER_H

#include "common.h"
#include <propeller/system.h>

class FWatcher
{
public:
    FWatcher( const std::string& path );
    virtual ~FWatcher();
    
    void start();
    
    
private:
    
#ifdef __MACH__
    void startOSX();
#endif
    
    class WorkerThread: public sys::Thread
    {
    public:
        WorkerThread( FWatcher* watcher )
        : m_watcher( watcher )
        {
            
        }
        
    private:
        virtual void routine()
        {
            TRACE_ENTERLEAVE();
            
            m_watcher->start();
        }

        
    private:
        FWatcher* m_watcher;
    };
    
    
    
private:
    std::string m_path;
    
#ifdef __MACH__
    FSEventStreamRef m_stream;
#endif
};

#endif	/* FSWATCHER_H */
