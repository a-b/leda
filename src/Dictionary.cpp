/* 
 * File:   Dictionary.cpp
 * Author: sergey
 * 
 * Created on July 17, 2014, 6:13 AM
 */

#include "Dictionary.h"

Dictionary::Dictionary( ) 
: m_db( NULL )
{
    TRACE_ENTERLEAVE();
    
    //
    //  get temp filename
    //
    char filename[ 256 ] = "/tmp/leda-dictionary-XXXXXXXX";
    int file = mkstemp( filename );
    if ( !file )
    {
        TRACE_ERROR( "unable to create temporary file", "" );
        throw std::runtime_error( "" );
    }
    
    close( file );
    ::remove( filename );
    
    m_filename = filename;
    
}

void Dictionary::create()
{
    TRACE_ENTERLEAVE();
    
    TRACE( "opening %s", m_filename.c_str() );
    
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open( options, m_filename, &m_db );
    
    TRACE( "db: 0x%x", m_db );
  
    if ( !status.ok() )
    {
        TRACE_ERROR( "failed to create dictionary: %s", status.ToString().c_str() );
        throw std::runtime_error(" ");
    }
}

Dictionary::~Dictionary( ) 
{
    TRACE_ENTERLEAVE();
    
    if ( m_db )
    {
        delete m_db;
    }
}

void Dictionary::addThread()
{
    TRACE_ENTERLEAVE();
    sys::LockEnterLeave lock( m_lock );
    intptr_t id = sys::Thread::currentId();
    TRACE( "adding read lock for thread %ld", id );
    m_readLocks[ id ] = new sys::Lock();
    
}

void Dictionary::set( const std::string& key, const std::string& value )
{
    setOrRemove( key.c_str(), &value );
}

void Dictionary::remove( const std::string& key )
{
    setOrRemove( key.c_str(), NULL );
}

void Dictionary::setOrRemove( const std::string& key, const std::string* value )
{
    TRACE_ENTERLEAVE();
    intptr_t id = sys::Thread::currentId();

    //
    //  lock read locks
    //
    for ( LockMap::const_iterator i = m_readLocks.begin(); i != m_readLocks.end(); i++ )
    {
        i->second->lock();
    }
    
    sys::LockEnterLeave lock( m_lock );
    
    
    
    if ( value )
    {
        TRACE( "writing %s: %s thread id %ld", key.c_str(), value->c_str(), sys::Thread::currentId() );
        m_db->Put( leveldb::WriteOptions(), key, *value );
    } 
    else
    {
        TRACE( "deleting %s", key.c_str() );
        m_db->Delete( leveldb::WriteOptions(), key );
    }
    
    //
    //  unlock read locks
    //
    for ( LockMap::const_iterator i = m_readLocks.begin(); i != m_readLocks.end(); i++ )
    {
        i->second->unlock();
    }    
}

void Dictionary::get( const std::string& key, std::string* value )
{
    TRACE_ENTERLEAVE();
    
    intptr_t id = sys::Thread::currentId();
    LockMap::const_iterator found = m_readLocks.find( id );
    if ( found == m_readLocks.end() )
    {
        TRACE( "no read lock found for thread %ld", id );
        return;
    }
    
    TRACE( "read lock for thread id %ld 0x%x", sys::Thread::currentId(), found->second );
    
    sys::LockEnterLeave lock( *( found->second ) );
    m_db->Get(leveldb::ReadOptions(), key, value );
    TRACE( "read %s: %s", key.c_str(), value->c_str() );
}

