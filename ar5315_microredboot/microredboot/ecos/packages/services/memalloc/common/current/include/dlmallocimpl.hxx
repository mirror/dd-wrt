#ifndef CYGONCE_MEMALLOC_DLMALLOCIMPL_HXX
#define CYGONCE_MEMALLOC_DLMALLOCIMPL_HXX

//==========================================================================
//
//      dlmallocimpl.hxx
//
//      Interface to the port of Doug Lea's malloc implementation
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
// Date:         2000-06-18
// Purpose:      Define standard interface to Doug Lea's malloc implementation
// Description:  Doug Lea's malloc has been ported to eCos. This file provides
//               the interface between the implementation and the standard
//               memory allocator interface required by eCos
// Usage:        #include <cyg/memalloc/dlmalloc.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// CONFIGURATION

#include <pkgconf/memalloc.h>

// INCLUDES

#include <stddef.h>                    // size_t, ptrdiff_t
#include <cyg/infra/cyg_type.h>        // types

#include <cyg/memalloc/common.hxx>     // Common memory allocator infra

// As a special case, override CYGIMP_MEMALLOC_ALLOCATOR_DLMALLOC_SAFE_MULTIPLE
// if the malloc config says so
#ifdef CYGIMP_MEMALLOC_MALLOC_DLMALLOC
// forward declaration to prevent header dependency problems
class Cyg_Mempool_dlmalloc;
# include <pkgconf/heaps.hxx>
# if (CYGMEM_HEAP_COUNT > 1) && \
     !defined(CYGIMP_MEMALLOC_ALLOCATOR_DLMALLOC_SAFE_MULTIPLE)
#  define CYGIMP_MEMALLOC_ALLOCATOR_DLMALLOC_SAFE_MULTIPLE 1
# endif
#endif

// CONSTANTS

// number of bins - but changing this alone will not change the number of
// bins!
#define CYGPRI_MEMALLOC_ALLOCATOR_DLMALLOC_NAV 128

// TYPE DEFINITIONS


class Cyg_Mempool_dlmalloc_Implementation
{
public:
    /* cyg_dlmalloc_size_t is the word-size used for internal bookkeeping
       of chunk sizes. On a 64-bit machine, you can reduce malloc
       overhead, especially for very small chunks, by defining
       cyg_dlmalloc_size_t to be a 32-bit type at the expense of not
       being able to handle requests greater than 2^31. This limitation is
       hardly ever a concern; you are encouraged to set this. However, the
       default version is the same as size_t. */

    typedef size_t Cyg_dlmalloc_size_t;
    
    typedef struct malloc_chunk
    {
        Cyg_dlmalloc_size_t prev_size; /* Size of previous chunk (if free). */
        Cyg_dlmalloc_size_t size;      /* Size in bytes, including overhead. */
        struct malloc_chunk* fd;   /* double links -- used only if free. */
        struct malloc_chunk* bk;
    };
    
protected:
    /* The first value returned from sbrk */
    cyg_uint8 *arenabase;

    /* The total memory in the pool */
    cyg_int32 arenasize;

#ifdef CYGIMP_MEMALLOC_ALLOCATOR_DLMALLOC_SAFE_MULTIPLE
    struct Cyg_Mempool_dlmalloc_Implementation::malloc_chunk *
    av_[ CYGPRI_MEMALLOC_ALLOCATOR_DLMALLOC_NAV * 2 + 2 ];
#endif

#ifdef CYGDBG_MEMALLOC_ALLOCATOR_DLMALLOC_DEBUG

    void
    do_check_chunk( struct malloc_chunk * );

    void
    do_check_free_chunk( struct malloc_chunk * );
    
    void
    do_check_inuse_chunk( struct malloc_chunk * );

    void
    do_check_malloced_chunk( struct malloc_chunk *, Cyg_dlmalloc_size_t );
#endif
    
public:
    // Constructor: gives the base and size of the arena in which memory is
    // to be carved out, note that management structures are taken from the
    // same arena.
    Cyg_Mempool_dlmalloc_Implementation( cyg_uint8 *  /* base */,
                                         cyg_int32    /* size */,
                                         CYG_ADDRWORD /* argthru */ );

    // Destructor
    ~Cyg_Mempool_dlmalloc_Implementation() {}

    // get some memory, return NULL if none available
    cyg_uint8 *
    try_alloc( cyg_int32 /* size */ );
    
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
    cyg_uint8 *
    resize_alloc( cyg_uint8 * /* alloc_ptr */, cyg_int32 /* newsize */,
                  cyg_int32 * /* oldsize */ );

    // free the memory back to the pool
    // returns true on success
    cyg_bool
    free( cyg_uint8 * /* ptr */, cyg_int32 /* size */ =0 );

    // Get memory pool status
    // flags is a bitmask of requested fields to fill in. The flags are
    // defined in common.hxx
    void
    get_status( cyg_mempool_status_flag_t /* flags */,
                Cyg_Mempool_Status & /* status */ );

};

#endif // ifndef CYGONCE_MEMALLOC_DLMALLOCIMPL_HXX
// EOF dlmallocimpl.hxx
