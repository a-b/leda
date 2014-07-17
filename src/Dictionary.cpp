/* 
 * File:   Dictionary.cpp
 * Author: sergey
 * 
 * Created on July 17, 2014, 6:13 AM
 */

#include "Dictionary.h"

Dictionary::Dictionary( ) 
{
    TRACE_ENTERLEAVE();
    m_hdb = tchdbnew();
    
    tchdbsetcache( m_hdb, 10000 );
    
    TRACE( "created hdb 0x%x", m_hdb );
    
    char filename[ 256 ] = "/tmp/leda-dictionary-XXXXXXXX";
    int file = mkstemp( filename );
    if ( !file )
    {
        TRACE_ERROR( "unable to create temporary file", "" );
        throw std::runtime_error( "" );
    }
    
    
    
    close( file );
    
    if ( !tchdbopen( m_hdb, filename, HDBOWRITER | HDBOCREAT ) )
    {
        TRACE_ERROR( "tchdbopen open failed with error %d", tchdbecode( m_hdb ) );
    }
    
    TRACE( "opened %s", filename );
    m_filename = filename;
           
}



Dictionary::~Dictionary( ) 
{
    TRACE_ENTERLEAVE();
    
    tchdbclose( m_hdb );
    ::remove( m_filename.c_str() );
    
}

void Dictionary::addThread()
{
    TRACE_ENTERLEAVE();
    sys::LockEnterLeave lock( m_lock );
    intptr_t id = sys::Thread::currentId();
    TRACE( "adding read lock for thread %ld", id );
    m_readLocks[ id ] = new sys::Lock();
    
}

void Dictionary::set( const char* key, const char* value )
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
    
    TRACE( "writing %s: %s thread id %ld", key, value, sys::Thread::currentId() );
    
    if ( value )
    {
        tchdbput2( m_hdb, key, value );
    } 
    else
    {
        tchdbout2( m_hdb, key );
    }
    
    //
    //  unlock read locks
    //
    for ( LockMap::const_iterator i = m_readLocks.begin(); i != m_readLocks.end(); i++ )
    {
        i->second->unlock();
    }
    
}

char* Dictionary::get( const char* key )
{
    TRACE_ENTERLEAVE();
    
    intptr_t id = sys::Thread::currentId();
    LockMap::const_iterator found = m_readLocks.find( id );
    if ( found == m_readLocks.end() )
    {
        TRACE( "no read lock found for thread %ld", id );
        return NULL;
    }
    
    
    TRACE( "read lock for thread id %ld 0x%x", sys::Thread::currentId(), found->second );
    
    
    sys::LockEnterLeave lock( *( found->second ) );
    
    char* value =  tchdbget2( m_hdb, key );
    TRACE( "read %s: %s", key, value );
    return value;
}



TCLIST* Dictionary::getkeys( const char* prefix )
{
    intptr_t id = sys::Thread::currentId();
    LockMap::const_iterator found = m_readLocks.find( id );
    if ( found == m_readLocks.end() )
    {
        TRACE( "no read lock found for thread %ld", id );
        return NULL;
    }
     
   TRACE( "read lock for thread id %ld 0x%x", sys::Thread::currentId(), found->second );
    
    sys::LockEnterLeave lock( *( found->second ) );
    
    return tchdbfwmkeys2( m_hdb, prefix, 100 );
}