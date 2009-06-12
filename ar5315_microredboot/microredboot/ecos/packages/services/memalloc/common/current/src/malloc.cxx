//========================================================================
//
//      malloc.cxx
//
//      Implementation of ISO C memory allocation routines
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-30
// Purpose:       Provides ISO C calloc(), malloc(), realloc() and free()
//                functions
// Description:   Implementation of ISO standard allocation routines as per
//                ISO C section 7.10.3
// Usage:       
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/memalloc.h>   // Configuration header

// Do we want these functions?
#ifdef CYGPKG_MEMALLOC_MALLOC_ALLOCATORS

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Common tracing support
#include <cyg/infra/cyg_ass.h>     // Common assertion support
#include <string.h>                // For memset() and memmove()
#include <stdlib.h>                // header for this file
#ifdef CYGBLD_MEMALLOC_MALLOC_EXTERNAL_HEAP_H
# include CYGBLD_MEMALLOC_MALLOC_EXTERNAL_HEAP_H
#else
# include <pkgconf/heaps.hxx>       // heap pools information
#endif
#include CYGBLD_MEMALLOC_MALLOC_IMPLEMENTATION_HEADER

// STATIC VARIABLES

// First deal with the worst case, that the memory layout didn't define a
// heap
#if CYGMEM_HEAP_COUNT == 0

// the data space for the memory pool
cyg_uint8 cyg_memalloc_mallocpool_memory[ 
    CYGNUM_MEMALLOC_FALLBACK_MALLOC_POOL_SIZE ] CYGBLD_ATTRIB_WEAK;

// the memory pool object itself
CYGCLS_MEMALLOC_MALLOC_IMPL cyg_memalloc_mallocpool
   CYGBLD_ATTRIB_INIT_BEFORE( CYG_INIT_LIBC ) =
   CYGCLS_MEMALLOC_MALLOC_IMPL( cyg_memalloc_mallocpool_memory,
                                sizeof( cyg_memalloc_mallocpool_memory ) );

# define POOL cyg_memalloc_mallocpool

#elif CYGMEM_HEAP_COUNT == 1
// one heap, so it's straightforward

# define POOL (*cygmem_memalloc_heaps[0])

#else 
// multiple heaps

# include <cyg/memalloc/memjoin.hxx>

Cyg_Mempool_Joined<CYGCLS_MEMALLOC_MALLOC_IMPL> cyg_memalloc_mallocpool
   CYGBLD_ATTRIB_INIT_BEFORE( CYG_INIT_LIBC ) =
     Cyg_Mempool_Joined<CYGCLS_MEMALLOC_MALLOC_IMPL>(
       CYGMEM_HEAP_COUNT, cygmem_memalloc_heaps
     );

# define POOL cyg_memalloc_mallocpool

#endif

// FUNCTIONS

void *
malloc( size_t size )
{
    void *data_ptr;

    CYG_REPORT_FUNCNAMETYPE( "malloc", "returning pointer %08x" );
    
    CYG_REPORT_FUNCARG1DV( size );

#ifdef CYGSEM_MEMALLOC_MALLOC_ZERO_RETURNS_NULL
    // first check if size wanted is 0
    if ( 0 == size ) {
        CYG_REPORT_RETVAL( NULL );
        return NULL;
    } // if
#endif

    // ask the pool for the data
    data_ptr = POOL.try_alloc( size );

    // if it isn't NULL is the pointer valid?
    if ( NULL != data_ptr ) {
        CYG_CHECK_DATA_PTR( data_ptr,
                            "allocator returned invalid pointer!" );

        // And just check its alignment
        CYG_ASSERT( !((CYG_ADDRWORD)data_ptr & (sizeof(CYG_ADDRWORD) - 1)),
                    "Allocator has returned badly aligned data!");
    } // if

    CYG_REPORT_RETVAL( data_ptr );

    return data_ptr;
} // malloc()


void
free( void *ptr )
{
    cyg_bool freeret;

    CYG_REPORT_FUNCNAME( "free");
    
    CYG_REPORT_FUNCARG1XV( ptr );

    // if null pointer, do nothing as per spec
    if ( NULL==ptr )
        return;

    CYG_CHECK_DATA_PTR( ptr, "Pointer to free isn't even valid!" );

    // get pool to free it
    freeret = POOL.free( (cyg_uint8 *) ptr );

    CYG_ASSERT( freeret , "Couldn't free!" );

    CYG_REPORT_RETURN();

} // free()


void *
calloc( size_t nmemb, size_t size )
{
    void *data_ptr;
    cyg_ucount32 realsize;

    CYG_REPORT_FUNCNAMETYPE( "calloc", "returning pointer %08x" );
    
    CYG_REPORT_FUNCARG2DV( nmemb, size );

    realsize = nmemb * size;

    data_ptr = malloc( realsize );

    // Fill with 0's if non-NULL
    if ( data_ptr != NULL )
        memset( data_ptr, 0, realsize );

    CYG_REPORT_RETVAL( data_ptr );
    return data_ptr;
} // calloc()


externC void *
realloc( void *ptr, size_t size )
{
    cyg_int32 oldsize;

    CYG_REPORT_FUNCNAMETYPE( "realloc", "returning pointer %08x" );

    CYG_REPORT_FUNCARG2( "ptr=%08x, size=%d", ptr, size );

    // if pointer is NULL, we must malloc it
    if ( ptr == NULL ) {
        ptr = malloc( size );
        CYG_REPORT_RETVAL( ptr );
        return ptr;
    } // if

    CYG_CHECK_DATA_PTR( ptr, "realloc() passed a bogus pointer!" );

    // if size is 0, we must free it
    if (size == 0) {
        free(ptr);
        CYG_REPORT_RETVAL( NULL );
        return NULL;
    } // if
        
    void *newptr;

    // otherwise try to resize allocation
    newptr = POOL.resize_alloc( (cyg_uint8 *)ptr, size, &oldsize );

    if ( NULL == newptr ) {
        // if resize_alloc doesn't return a pointer, it failed, so we
        // just have to allocate new space instead, and later copy it
        
        CYG_ASSERT( oldsize != 0,
                    "resize_alloc() couldn't determine allocation size!" );

        newptr = malloc( size );
        
        if ( NULL != newptr ) {
            memcpy( newptr, ptr, size < (size_t) oldsize ? size
                    : (size_t) oldsize );
            free( ptr );
        }
    }
    
    CYG_REPORT_RETVAL( newptr );
    return newptr;
} // realloc()


externC struct mallinfo
mallinfo( void )
{
    struct mallinfo ret = { 0 }; // initialize to all zeros
    Cyg_Mempool_Status stat;

    CYG_REPORT_FUNCTION();

    POOL.get_status( CYG_MEMPOOL_STAT_ARENASIZE|
                     CYG_MEMPOOL_STAT_FREEBLOCKS|
                     CYG_MEMPOOL_STAT_TOTALALLOCATED|
                     CYG_MEMPOOL_STAT_TOTALFREE|
                     CYG_MEMPOOL_STAT_MAXFREE, stat );

    if ( stat.arenasize > 0 )
        ret.arena = stat.arenasize;
    
    if ( stat.freeblocks > 0 )
        ret.ordblks = stat.freeblocks;

    if ( stat.totalallocated > 0 )
        ret.uordblks = stat.totalallocated;
    
    if ( stat.totalfree > 0 )
        ret.fordblks = stat.totalfree;

    if ( stat.maxfree > 0 )
        ret.maxfree = stat.maxfree;

    CYG_REPORT_RETURN();
    return ret;
} // mallinfo()

#endif // ifdef CYGPKG_MEMALLOC_MALLOC_ALLOCATORS

// EOF malloc.cxx
