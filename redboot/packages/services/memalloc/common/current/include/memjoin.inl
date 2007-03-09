#ifndef CYGONCE_MEMALLOC_MEMJOIN_INL
#define CYGONCE_MEMALLOC_MEMJOIN_INL

//==========================================================================
//
//      memjoin.inl
//
//      Pseudo memory pool used to join together other memory pools
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-06-12
// Purpose:      Implement joined up memory pool class interface
// Description:  Inline class for constructing a pseudo allocator that contains
//               multiple other allocators. It caters solely to the requirements
//               of the malloc implementation.
// Usage:        #include <cyg/memalloc/memjoin.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// CONFIGURATION

#include <pkgconf/memalloc.h>

// INCLUDES

#include <cyg/infra/cyg_type.h>        // types
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/memalloc/memjoin.hxx>    // header for this file just in case


// FUNCTIONS


// -------------------------------------------------------------------------
// find_pool_for_ptr returns the pool that ptr came from

template <class T>
inline T *
Cyg_Mempool_Joined<T>::find_pool_for_ptr( const cyg_uint8 *ptr )
{
    cyg_uint8 i;

    for ( i=0; i < poolcount; i++ ) {
        if ( ptr >= pools[i].startaddr &&
             ptr < pools[i].endaddr ) {
            return pools[i].pool;
        } // if
    } // for
    return NULL;
} // Cyg_Mempool_Joined<T>::find_pool_for_ptr()


// -------------------------------------------------------------------------
// Constructor
template <class T>
inline
Cyg_Mempool_Joined<T>::Cyg_Mempool_Joined( cyg_uint8 num_heaps, T *heaps[] )
{
    Cyg_Mempool_Status stat;
    cyg_uint8 i;

    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2( "num_heaps=%u, heaps=%08x", (int)num_heaps, heaps );

    CYG_CHECK_DATA_PTRC( heaps );

    poolcount = num_heaps;

    // allocate internal structures - this should work because we should be
    // the first allocation for this pool; and if there isn't enough space
    // for these teeny bits, what hope is there!
    for (i=0; i<num_heaps; i++) {
        pools = (struct pooldesc *)
            heaps[i]->try_alloc( num_heaps * sizeof(struct pooldesc) );
        if ( NULL != pools )
            break;
    } // for

    CYG_ASSERT( pools != NULL,
                "Couldn't allocate internal structures from any pools!");

    // now set up internal structures
    for (i=0; i<num_heaps; i++) {
        pools[i].pool = heaps[i];
        heaps[i]->get_status( CYG_MEMPOOL_STAT_ARENABASE|
                              CYG_MEMPOOL_STAT_ARENASIZE,
                              stat );

        CYG_ASSERT( stat.arenabase != (const cyg_uint8 *)-1,
                    "pool returns valid pool base" );
        CYG_CHECK_DATA_PTR( stat.arenabase, "Bad arena location" );
        CYG_ASSERT( stat.arenasize > 0, "pool returns valid pool size" );
        
        pools[i].startaddr = stat.arenabase;
        pools[i].endaddr = stat.arenabase + stat.arenasize;
    } // for

    CYG_REPORT_RETURN();
} // Cyg_Mempool_Joined<T>::Cyg_Mempool_Joined()



// -------------------------------------------------------------------------
// Destructor
template <class T>
inline
Cyg_Mempool_Joined<T>::~Cyg_Mempool_Joined()
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARGVOID();

    cyg_bool freestat;
    
    freestat = free( (cyg_uint8 *)pools, poolcount * sizeof(struct pooldesc) );
    CYG_ASSERT( freestat, "free failed!");
    CYG_REPORT_RETURN();
} // Cyg_Mempool_Joined<T>::~Cyg_Mempool_Joined()



// -------------------------------------------------------------------------
// get some memory, return NULL if none available
template <class T>
inline cyg_uint8 *
Cyg_Mempool_Joined<T>::try_alloc( cyg_int32 size )
{
    cyg_uint8 i;
    cyg_uint8 *ptr=NULL;

    CYG_REPORT_FUNCTYPE( "returning memory at addr %08x" );
    CYG_REPORT_FUNCARG1DV( size );

    for (i=0; i<poolcount; i++) {
        ptr = pools[i].pool->try_alloc( size );
        if ( NULL != ptr )
            break;
    }

    CYG_REPORT_RETVAL( ptr );

    CYG_MEMALLOC_FAIL_TEST(ptr==NULL, size);

    return ptr;
} // Cyg_Mempool_Joined<T>::try_alloc()


// -------------------------------------------------------------------------
// resize existing allocation, if oldsize is non-NULL, previous
// allocation size is placed into it. If previous size not available,
// it is set to 0. NB previous allocation size may have been rounded up.
// Occasionally the allocation can be adjusted *backwards* as well as,
// or instead of forwards, therefore the address of the resized
// allocation is returned, or NULL if no resizing was possible.
// Note that this differs from ::realloc() in that no attempt is
// made to call malloc() if resizing is not possible - that is left
// to higher layers. The data is copied from old to new though.
// The effects of alloc_ptr==NULL or newsize==0 are undefined
template <class T>
inline cyg_uint8 *
Cyg_Mempool_Joined<T>::resize_alloc( cyg_uint8 *alloc_ptr, cyg_int32 newsize,
                                     cyg_int32 *oldsize )
{
    T *pool;
    cyg_uint8 * ret;
    
    CYG_REPORT_FUNCTYPE( "success=" );
    CYG_REPORT_FUNCARG3( "alloc_ptr=%08x, newsize=%d, &oldsize=%08x",
                        alloc_ptr, newsize, oldsize );
    CYG_CHECK_DATA_PTRC( alloc_ptr );
    if (NULL != oldsize )
        CYG_CHECK_DATA_PTRC( oldsize );

    pool = find_pool_for_ptr( alloc_ptr );
    CYG_ASSERT( NULL != pool, "Couldn't find pool for pointer!" );

    ret = pool->resize_alloc( alloc_ptr, newsize, oldsize );

    CYG_REPORT_RETVAL( ret );

    return ret;    
} // Cyg_Mempool_Joined<T>::resize_alloc()


// -------------------------------------------------------------------------
// free the memory back to the pool
// returns true on success
template <class T>
inline cyg_bool
Cyg_Mempool_Joined<T>::free( cyg_uint8 *ptr, cyg_int32 size )
{
    T *pool;
    cyg_bool ret;

    CYG_REPORT_FUNCTYPE("success=");
    CYG_REPORT_FUNCARG2( "ptr=%08x, size=%d", ptr, size );
    CYG_CHECK_DATA_PTRC( ptr );

    pool = find_pool_for_ptr( ptr );
    CYG_ASSERT( NULL != pool, "Couldn't find pool for pointer!" );

    ret = pool->free( ptr, size );

    CYG_REPORT_RETVAL( ret );
    return ret;    
} // Cyg_Mempool_Joined<T>::free()


// -------------------------------------------------------------------------
// Get memory pool status
// flags is a bitmask of requested fields to fill in. The flags are
// defined in common.hxx
template <class T>
inline void
Cyg_Mempool_Joined<T>::get_status( cyg_mempool_status_flag_t flags,
                                Cyg_Mempool_Status &status )
{
    cyg_uint8 i;
    Cyg_Mempool_Status tmpstat;

    status.arenasize      = status.freeblocks = 0;
    status.totalallocated = status.totalfree  = 0;
    status.maxfree        = status.origsize   = 0;

    for ( i=0; i<poolcount; i++ ) {
        if ( status.arenasize >= 0 ) {
            if ( 0 != (flags & CYG_MEMPOOL_STAT_ARENASIZE) ) {
                pools[i].pool->get_status( CYG_MEMPOOL_STAT_ARENASIZE,
                                           tmpstat );
                if ( tmpstat.arenasize > 0)
                    status.arenasize += tmpstat.arenasize;
                else
                    status.arenasize = -1;
            } // if
        } // if

        if ( status.freeblocks >= 0 ) {
            if ( 0 != (flags & CYG_MEMPOOL_STAT_FREEBLOCKS) ) {
                pools[i].pool->get_status( CYG_MEMPOOL_STAT_FREEBLOCKS,
                                           tmpstat );
                if ( tmpstat.freeblocks > 0 )
                    status.freeblocks += tmpstat.freeblocks;
                else
                    status.freeblocks = -1;
            } // if
        } // if

        if ( status.totalallocated >= 0 ) {
            if ( 0 != (flags & CYG_MEMPOOL_STAT_TOTALALLOCATED) ) {
                pools[i].pool->get_status( CYG_MEMPOOL_STAT_TOTALALLOCATED,
                                           tmpstat );
                if ( tmpstat.totalallocated > 0 )
                    status.totalallocated += tmpstat.totalallocated;
                else
                    status.totalallocated = -1;
            } // if
        } // if

        if ( status.totalfree >= 0 ) {
            if ( 0 != (flags & CYG_MEMPOOL_STAT_TOTALFREE) ) {
                pools[i].pool->get_status( CYG_MEMPOOL_STAT_TOTALFREE,
                                           tmpstat );
                if ( tmpstat.totalfree > 0 )
                    status.totalfree += tmpstat.totalfree;
                else
                    status.totalfree = -1;
            } // if
        } // if

        if ( status.maxfree >= 0 ) {
            if ( 0 != (flags & CYG_MEMPOOL_STAT_MAXFREE) ) {
                pools[i].pool->get_status( CYG_MEMPOOL_STAT_MAXFREE, tmpstat );
                if ( tmpstat.maxfree < 0 )
                    status.maxfree = -1;
                else if ( tmpstat.maxfree > status.maxfree )
                    status.maxfree = tmpstat.maxfree;
            } // if
        } // if

        if ( status.origsize >= 0 ) {
            if ( 0 != (flags & CYG_MEMPOOL_STAT_ORIGSIZE) ) {
                pools[i].pool->get_status( CYG_MEMPOOL_STAT_ORIGSIZE, tmpstat );
                if ( tmpstat.origsize > 0 )
                    status.origsize += tmpstat.origsize;
                else
                    status.origsize = -1;
            } // if
        } // if

        if ( status.maxoverhead >= 0 ) {
            if ( 0 != (flags & CYG_MEMPOOL_STAT_MAXOVERHEAD) ) {
                pools[i].pool->get_status( CYG_MEMPOOL_STAT_MAXOVERHEAD,
                                           tmpstat );
                if ( tmpstat.maxoverhead < 0 )
                    status.maxoverhead = -1;
                else if ( tmpstat.maxoverhead > status.maxoverhead )
                    status.maxoverhead = tmpstat.maxoverhead;
            } // if
        } // if
    } // for
} // Cyg_Mempool_Joined<T>::get_status()


// -------------------------------------------------------------------------

#endif // ifndef CYGONCE_MEMALLOC_MEMJOIN_INL
// EOF memjoin.inl
