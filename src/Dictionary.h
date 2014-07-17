/* 
 * File:   Dictionary.h
 * Author: sergey
 *
 * Created on July 17, 2014, 6:13 AM
 */

//
// thread safe dictionary class
//
#include <propeller/system.h>
#include "common.h"

extern "C" 
{
#include <tchdb.h>
}




#ifndef DICTIONARY_H
#define	DICTIONARY_H

class Dictionary 
{
public:
    Dictionary( );
    
    virtual ~Dictionary();
    
    void addThread();
    
    void set( const char* key, const char* value );
    char* get( const char* key );
    TCLIST* getkeys( const char* prefix );
    
private:
    
    
    typedef std::map< intptr_t, sys::Lock* > LockMap;
    
private:
    
    TCHDB* m_hdb;
    LockMap m_readLocks;
    sys::Lock m_lock;
    std::string m_filename;

};

#endif	/* DICTIONARY_H */

