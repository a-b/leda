/* 
 * File:   Dictionary.h
 * Author: sergey
 *
 * Created on July 17, 2014, 6:13 AM
 */

//
// thread safe dictionary class
//

#ifndef DICTIONARY_H
#define	DICTIONARY_H

#include <propeller/system.h>
#include "common.h"
#include "leveldb/db.h"

class Dictionary 
{
public:
    Dictionary( );
    void  create( );
    
    virtual ~Dictionary();
    
    void addThread();
    
    void set( const std::string& key, const std::string& value );
    void remove( const std::string& key );
    void get( const std::string& key, std::string* value );
    
    
    
private:
    void setOrRemove( const std::string& key, const std::string* value );
    
    typedef std::map< intptr_t, sys::Lock* > LockMap;
    
private:
    leveldb::DB* m_db;
    LockMap m_readLocks;
    sys::Lock m_lock;
    std::string m_filename;

};

#endif	/* DICTIONARY_H */

