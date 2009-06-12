#ifndef CYGONCE_MEMALLOC_SEPMETAIMPL_HXX
#define CYGONCE_MEMALLOC_SEPMETAIMPL_HXX

//==========================================================================
//
//      sepmetaimpl.hxx
//
//      Variable block memory pool with separate metadata class declarations
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
// Date:         2001-06-28
// Purpose:      Define Sepmetaimpl class interface
// Description:  Inline class for constructing a variable block allocator
//               with separate metadata.
// Usage:        #include <cyg/memalloc/sepmetaimpl.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


#include <cyg/infra/cyg_type.h>
#include <pkgconf/memalloc.h>
#include <cyg/memalloc/common.hxx>     // Common memory allocator infra

class Cyg_Mempool_Sepmeta_Implementation {
protected:
    // these constructors are explicitly disallowed
    Cyg_Mempool_Sepmeta_Implementation() {};
//    Cyg_Mempool_Sepmeta_Implementation( Cyg_Mempool_Sepmeta_Implementation &ref )
//    {};
    Cyg_Mempool_Sepmeta_Implementation &
    operator=( Cyg_Mempool_Sepmeta_Implementation &ref )
    { return ref; };

    struct memdq {
        struct memdq *prev, *next;       // prev/next alloced/free block
        struct memdq *memprev, *memnext; // prev/next block in memory
        cyg_uint8 *mem;           // memory address associated with this block
    };

    struct memdq allocedhead;   // list of alloced memory
    struct memdq freehead;      // list of free memory
    struct memdq memhead;       // initial block on free list
    struct memdq memend;        // dummy memdq indicating the end
                                // of memory, as if it were alloced
    struct memdq *freemetahead; // unused memdq's
    cyg_uint8  *obase;
    cyg_int32  osize;
    cyg_uint8  *metabase;
    cyg_int32  metasize;
    cyg_uint8  *bottom;
    cyg_uint8  *top;
    cyg_int32  alignment;
    cyg_int32  freemem;

    // round up addresses according to required alignment of pool
    cyg_uint8 *
    alignup( cyg_uint8 *addr );
            
    cyg_uint8 *
    aligndown( cyg_uint8 *addr );

    // round up addresses according to required alignment of metadata
    cyg_uint8 *
    alignmetaup( cyg_uint8 *addr );

    cyg_uint8 *
    alignmetadown( cyg_uint8 *addr );

    // return the alloced dq at mem
    struct memdq *
    find_alloced_dq( cyg_uint8 *mem );

    // returns a free dq of at least size, or NULL if none
    struct memdq *
    find_free_dq( cyg_int32 size );

    // returns the free dq following mem
    struct memdq *
    find_free_dq_slot( cyg_uint8 *mem );

    void
    insert_free_block( struct memdq *freedq );

    static void
    copy_data( cyg_uint8 *dst, cyg_uint8 *src, cyg_int32 nbytes );

    void
    check_free_memdq( struct memdq *dq );

    void
    check_alloced_memdq( struct memdq *dq );

public:
    // THIS is the public API of memory pools generally that can have the
    // kernel oriented thread-safe package layer atop.

    struct constructorargs {
        cyg_int32 alignment;
        cyg_uint8 *metabase;
        cyg_uint32 metasize;
        constructorargs(cyg_int32 align, cyg_uint8 *mbase, cyg_uint32 msize)
        {
            alignment = align; metabase = mbase; metasize = msize;
        }
    };

    // Constructor: gives the base and size of the arena in which memory is
    // to be carved out.
    Cyg_Mempool_Sepmeta_Implementation(
        cyg_uint8 *  /* base */,
        cyg_int32    /* size */,
        CYG_ADDRWORD /* constructorargs */ );

    // Destructor
    ~Cyg_Mempool_Sepmeta_Implementation();

    // get size bytes of memory
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

    // free size bytes of memory back to the pool
    // returns true on success
    cyg_bool
    free( cyg_uint8 * /* ptr */,
          cyg_int32   /* size */ );

    // Get memory pool status
    // flags is a bitmask of requested fields to fill in. The flags are
    // defined in common.hxx
    void
    get_status( cyg_mempool_status_flag_t /* flags */,
                Cyg_Mempool_Status & /* status */ );
    
};

#include <cyg/memalloc/sepmetaimpl.inl>

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_MEMALLOC_SEPMETAIMPL_HXX
// EOF sepmetaimpl.hxx
