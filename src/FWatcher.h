#ifndef _FSWATCHER_H
#define	_FSWATCHER_H

#include "common.h"
#include <propeller/system.h>

class FWatcher
{
public:
    FWatcher( const std::string& path );
    virtual ~FWatcher();
    
    void start();
    
    
private:
    
    void startInternal();

    
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
    
};

#endif	/* _FSWATCHER_H */
