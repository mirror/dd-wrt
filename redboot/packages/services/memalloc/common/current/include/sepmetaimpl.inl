#ifndef CYGONCE_MEMALLOC_SEPMETAIMPL_INL
#define CYGONCE_MEMALLOC_SEPMETAIMPL_INL

//==========================================================================
//
//      sepmetaimpl.inl
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
// Contributors: hmt
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

#include <pkgconf/system.h>
#ifdef CYGPKG_ISOINFRA
# include <pkgconf/isoinfra.h>
#endif
#include <pkgconf/memalloc.h>
#include <cyg/memalloc/sepmetaimpl.hxx>

#include <cyg/infra/cyg_ass.h>           // assertion support
#include <cyg/infra/cyg_trac.h>          // tracing support

// Simple allocator

// The memory block lists are doubly linked lists. One for all alloced
// blocks, one for all free blocks. There's also a list of unused
// metadata from the metadata pool.  The head of the
// list has the same structure but its memnext/memprev fields are zero.
// Always having at least one item on the list simplifies the alloc and
// free code.
#ifdef CYGINT_ISO_STRING_MEMFUNCS
# include <string.h>
#endif

inline void
Cyg_Mempool_Sepmeta_Implementation::copy_data( cyg_uint8 *dst, 
                                               cyg_uint8 *src, 
                                               cyg_int32 nbytes )
{
#ifdef CYGINT_ISO_STRING_MEMFUNCS
    memmove( dst, src, nbytes );
#else
    if ((src < dst) && (dst < (src + nbytes))) {
        // Have to copy backwards
        src += nbytes;
        dst += nbytes;
        while (nbytes--) {
            *--dst = *--src;
        }
    } else {
        while (nbytes--) {
            *dst++ = *src++;
        }
    }
#endif
}

inline cyg_uint8 *
Cyg_Mempool_Sepmeta_Implementation::alignup( cyg_uint8 *addr )
{
    return (cyg_uint8 *)((cyg_int32)(addr + alignment-1) & -alignment);
}

inline cyg_uint8 *
Cyg_Mempool_Sepmeta_Implementation::aligndown( cyg_uint8 *addr )
{
    return (cyg_uint8 *)((cyg_int32)addr & -alignment);
}

inline cyg_uint8 *
Cyg_Mempool_Sepmeta_Implementation::alignmetaup( cyg_uint8 *addr )
{
    const size_t memdqalign = __alignof__ (struct memdq);
    return (cyg_uint8 *)((cyg_int32)(addr + memdqalign-1) & -memdqalign);
}

inline cyg_uint8 *
Cyg_Mempool_Sepmeta_Implementation::alignmetadown( cyg_uint8 *addr )
{
    const size_t memdqalign = __alignof__ (struct memdq);
    return (cyg_uint8 *)((cyg_int32)addr & -memdqalign);
}

// return the alloced dq at mem 
inline struct Cyg_Mempool_Sepmeta_Implementation::memdq *
Cyg_Mempool_Sepmeta_Implementation::find_alloced_dq( cyg_uint8 *mem )
{
    struct memdq *dq=allocedhead.next;

    while (dq->mem != mem ) {
        CYG_ASSERT( dq->next->prev==dq, "Bad link in dq");
        CYG_ASSERT( dq->memnext->memprev==dq, "Bad link in mem dq");
        if (dq->next == &memend) // address not found!
            return NULL;
        dq = dq->next;
    }
    return dq;
}

// returns a free dq of at least size, or NULL if none
inline struct Cyg_Mempool_Sepmeta_Implementation::memdq *
Cyg_Mempool_Sepmeta_Implementation::find_free_dq( cyg_int32 size )
{
    struct memdq *dq = freehead.next;

    while ( (dq->memnext->mem - dq->mem) < size ) {
        CYG_ASSERT( dq->next->prev==dq, "Bad link in dq");
        CYG_ASSERT( dq->memnext->memprev==dq, "Bad link in mem dq");
        if (dq->next == &freehead) { // reached end of list
            return NULL;
        }
        dq = dq->next; // next on free list
    }
    return dq;
}

// returns the free dq following mem
inline struct Cyg_Mempool_Sepmeta_Implementation::memdq *
Cyg_Mempool_Sepmeta_Implementation::find_free_dq_slot( cyg_uint8 *mem )
{
    struct memdq *dq;
    for (dq = freehead.next; dq->mem < mem; dq = dq->next) {
        if ( dq == &freehead ) // wrapped round
            break;
    }
    return dq;
}

inline void
Cyg_Mempool_Sepmeta_Implementation::check_free_memdq( struct memdq *dq )
{
    if (dq == &freehead)
        return;
    CYG_ASSERT(dq->memnext->memprev == dq, "corrupted free dq #1");
    CYG_ASSERT(dq->next->prev == dq, "corrupted free dq #2");
    CYG_ASSERT(dq->memprev->memnext == dq, "corrupted free dq #3");
    CYG_ASSERT(dq->prev->next == dq, "corrupted free dq #4");
    CYG_ASSERT(dq->memnext->mem > dq->mem, "free dq mem not sorted #1");
    if (dq->memprev != &memend)
        CYG_ASSERT(dq->memprev->mem < dq->mem, "free dq mem not sorted #2");
}

inline void
Cyg_Mempool_Sepmeta_Implementation::check_alloced_memdq( struct memdq *dq )
{
    CYG_ASSERT(dq->memnext->memprev == dq, "corrupted alloced dq #1");
    CYG_ASSERT(dq->next->prev == dq, "corrupted alloced dq #2");
    CYG_ASSERT(dq->memprev->memnext == dq, "corrupted alloced dq #3");
    CYG_ASSERT(dq->prev->next == dq, "corrupted alloced dq #4");
    if (dq != &memend)
        CYG_ASSERT(dq->memnext->mem > dq->mem, "alloced dq mem not sorted #1");
    if (dq->memprev != &memhead)
        CYG_ASSERT(dq->memprev->mem < dq->mem, "alloced dq mem not sorted #2");
}

// -------------------------------------------------------------------------

inline void
Cyg_Mempool_Sepmeta_Implementation::insert_free_block( struct memdq *dq )
{
    // scan for correct slot in the sorted free list
    struct memdq *fdq = find_free_dq_slot( dq->mem );

    CYG_ASSERT(fdq != &freehead ? fdq->mem > dq->mem : 1,
               "Block address is already in freelist");

    check_free_memdq(fdq);

    if (dq->memnext == fdq) {
        // we can coalesce these two together
        // adjust fdq's mem address backwards to include dq
        fdq->mem = dq->mem;
        // and remove dq
        fdq->memprev = dq->memprev;
        fdq->memprev->memnext = fdq;
        // Don't need to adjust fdq's next/prev links as it stays in the
        // same place in the free list

        // dq is now redundant so return to metadata free list
        dq->next = freemetahead;
        freemetahead = dq;

        // reset dq
        dq = fdq;
    } else {
        // insert behind fdq
        dq->next = fdq;
        dq->prev = fdq->prev;
        fdq->prev = dq;
        dq->prev->next = dq;
    }

    check_free_memdq(dq);

    // maybe also coalesce backwards
    if (dq->memprev == dq->prev) {
        // adjust dq's mem address backwards to include dq->prev
        dq->mem = dq->prev->mem;

        // return dq->prev to metadata free list
        dq->prev->next = freemetahead;
        freemetahead = dq->prev;

        // and remove dq->prev from mem list
        dq->memprev = dq->prev->memprev;
        dq->memprev->memnext = dq;
        // and free list
        dq->prev = dq->prev->prev;
        dq->prev->next = dq;

        check_free_memdq(dq);
    }
}

// -------------------------------------------------------------------------
#include <cyg/infra/diag.h>
inline
Cyg_Mempool_Sepmeta_Implementation::Cyg_Mempool_Sepmeta_Implementation(
        cyg_uint8 *base,
        cyg_int32 size,
        CYG_ADDRWORD consargs)
{
    CYG_REPORT_FUNCTION();
    struct constructorargs *args = (struct constructorargs *)consargs;
    CYG_CHECK_DATA_PTRC( args );

    alignment = args->alignment;

    CYG_ASSERT( alignment > 0, "Bad alignment" );
    CYG_ASSERT( 0!=alignment, "alignment is zero" );
    CYG_ASSERT( 0==(alignment & alignment-1), "alignment not a power of 2" );

    obase=base;
    osize=size;
    metabase = args->metabase;
    metasize = args->metasize;

    // bottom is set to the lowest available address given the alignment. 
    bottom = alignup( base );
    cyg_uint8 *metabottom = alignmetaup( metabase );

    // because we split free blocks by allocating memory from the end, not
    // the beginning, then to preserve alignment, the *top* must also be
    // aligned
    top = aligndown( base+size );
    cyg_uint8 *metatop = metabottom + 
        sizeof(struct memdq)*(metasize/sizeof(struct memdq));
    
    CYG_ASSERT( top > bottom , "heap too small" );
    CYG_ASSERT( top <= (base+size), "top too large" );
    CYG_ASSERT( (((cyg_int32)(top)) & alignment-1)==0,
                "top badly aligned" );
    CYG_ASSERT( (((cyg_int32)(bottom)) & alignment-1)==0,
                "bottom badly aligned" );

    CYG_ASSERT( metatop > metabottom , "meta space too small" );
    CYG_ASSERT( metatop <= (metabase+metasize), "metatop too large" );

    // Initialize list of unused metadata blocks. Only need to do next
    // pointers - can ignore prev and size
    struct memdq *fq = freemetahead = (struct memdq *)metabottom;
    
    while ((cyg_uint8 *)fq < metatop) {
        fq->next = fq+1;
        fq++;
    }

    CYG_ASSERT((cyg_uint8 *)fq == metatop, "traversed metadata not aligned");

    // set final pointer to NULL;
    --fq; fq->next = NULL;

    // initialize the free list. memhead is the initial free block occupying
    // all of free memory.
    memhead.next = memhead.prev = &freehead;
    // The mem list is circular for consistency.
    memhead.memprev = memhead.memnext = &memend;
    memhead.mem = bottom;

    // initialize block that indicates end of memory. This pretends to
    // be an allocated block
    memend.next = memend.prev = &allocedhead;
    memend.memnext = memend.memprev = &memhead;
    memend.mem = top;

    // initialize alloced list memdq. memend pretends to be allocated memory
    // at the end
    allocedhead.next = allocedhead.prev = &memend;
    freehead.next = freehead.prev = &memhead;
    // Since allocedhead and freehead are placeholders, not real blocks,
    // assign addresses which can't match list searches
    allocedhead.memnext = allocedhead.memprev = NULL;
    freehead.memnext = freehead.memprev = NULL;
    freehead.mem = allocedhead.mem = NULL;

    freemem = top - bottom;
}

// -------------------------------------------------------------------------

inline
Cyg_Mempool_Sepmeta_Implementation::~Cyg_Mempool_Sepmeta_Implementation()
{
}

// -------------------------------------------------------------------------
// allocation is mostly simple
// First we look down the free list for a large enough block
// If we find a block the right size, we unlink the block from
//    the free list and return a pointer to it.
// If we find a larger block, we chop a piece off the end
//    and return that
// Otherwise we reach the end of the list and return NULL

inline cyg_uint8 *
Cyg_Mempool_Sepmeta_Implementation::try_alloc( cyg_int32 size )
{
    struct memdq *alloced;

    CYG_REPORT_FUNCTION();

    //  Allow uninitialised (zero sized) heaps because they could exist as a
    //  quirk of the MLT setup where a dynamically sized heap is at the top of
    //  memory.
    if (NULL == bottom || NULL==metabase)
        return NULL;

    size = (size + alignment - 1) & -alignment;

    struct memdq *dq = find_free_dq( size );
   

    if (NULL == dq) {	
	CYG_MEMALLOC_FAIL(size);
        return NULL;
    }

    cyg_int32 dqsize = dq->memnext->mem - dq->mem;

    if( size == dqsize ) {
        // exact fit -- unlink from free list
        dq->prev->next = dq->next;
        dq->next->prev = dq->prev;

        // set up this block for insertion into alloced list
        dq->next = dq->memnext; // since dq was free, dq->memnext must
                                // be allocated otherwise it would have
                                // been coalesced
        dq->prev = dq->next->prev;

        alloced = dq;
    } else {

        CYG_ASSERT( dqsize > size, "block found is too small");

        // Split into two memdq's, returning the second one

        // first get a memdq

        if ( NULL == freemetahead ) {
 	    // out of metadata. 
	    CYG_MEMALLOC_FAIL(size);
            return NULL;
	}

        // FIXME: since we don't search all the way for an exact fit
        // first we may be able to find an exact fit later and therefore
        // not need more metadata. We don't do this yet though.

        alloced = freemetahead;
        freemetahead = alloced->next;

        // now set its values
        alloced->memnext = dq->memnext;
        alloced->next = dq->memnext; // since dq was free, dq->memnext must
                                     // be allocated otherwise it would have
                                     // been coalesced
        alloced->memprev = dq;
        alloced->prev = alloced->next->prev;

        alloced->mem = alloced->next->mem - size;

        // now set up dq (the portion that remains a free block)
        // dq->next and dq->prev are unchanged as we still end up pointing
        // at the same adjacent free blocks
        // dq->memprev obviously doesn't change
        
        dq->memnext = alloced;
        
        // finish inserting into memory block list
        alloced->memnext->memprev = alloced;
    alloced->next->prev = alloced->prev->next = alloced;

        check_free_memdq(dq);
    }

    CYG_ASSERT( bottom <= alloced->mem && alloced->mem <= top,
                "alloced outside pool" );

    // Insert block into alloced list. 
    alloced->next->prev = alloced->prev->next = alloced;

    check_alloced_memdq(alloced);

    freemem -=size;

    CYG_ASSERT( ((CYG_ADDRESS)alloced->mem & (alignment-1)) == 0,
                "returned memory not aligned" );
    return alloced->mem;
}

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

inline cyg_uint8 *
Cyg_Mempool_Sepmeta_Implementation::resize_alloc( cyg_uint8 *alloc_ptr,
                                                   cyg_int32 newsize,
                                                   cyg_int32 *oldsize )
{
    cyg_int32 currsize, origsize;

    CYG_REPORT_FUNCTION();
    
    CYG_CHECK_DATA_PTRC( alloc_ptr );
    if ( NULL != oldsize )
        CYG_CHECK_DATA_PTRC( oldsize );

    CYG_ASSERT( (bottom <= alloc_ptr) && (alloc_ptr <= top),
                "alloc_ptr outside pool" );
    
    struct memdq *dq=find_alloced_dq( alloc_ptr );
    CYG_ASSERT( dq != NULL, "passed address not previously alloced");
    
    currsize = origsize = dq->memnext->mem - dq->mem;
    if ( NULL != oldsize )
        *oldsize = currsize;

    if ( newsize > currsize ) {
        cyg_int32 nextmemsize=0, prevmemsize=0;

        // see if we can increase the allocation size. Don't change anything
        // so we don't have to undo it later if it wouldn't fit
        if ( dq->next != dq->memnext ) { // if not equal, memnext must
                                         // be on free list
            nextmemsize = dq->memnext->memnext->mem - dq->memnext->mem;
        }
        if ( dq->prev != dq->memprev) { // ditto
            prevmemsize = dq->mem - dq->memprev->mem;
        }
        if (nextmemsize + prevmemsize + currsize < newsize)
	{
    	    CYG_MEMALLOC_FAIL_TEST(true, newsize);
            return NULL; // can't fit it
	}

        // expand forwards
        if ( nextmemsize != 0 ) {
            if (nextmemsize <= (newsize - currsize)) { // taking all of it
                struct memdq *fblk = dq->memnext;
                
                // fix up mem list ptrs
                dq->memnext = fblk->memnext;
                dq->memnext->memprev=dq;
                // fix up free list ptrs
                fblk->next->prev = fblk->prev;
                fblk->prev->next = fblk->next;

                // return to meta list
                fblk->next = freemetahead;
                freemetahead = fblk->next;
                currsize += nextmemsize;
            } else { // only needs some
                dq->memnext->mem += (newsize - currsize);
                currsize = newsize;
            }
        }

        // expand backwards
        if ( currsize < newsize && prevmemsize != 0 ) {
            cyg_uint8 *oldmem = dq->mem;

            CYG_ASSERT( prevmemsize >= newsize - currsize,
                        "miscalculated expansion" );
            if (prevmemsize == (newsize - currsize)) { // taking all of it
                struct memdq *fblk = dq->memprev;
                
                // fix up mem list ptrs
                dq->memprev = fblk->memprev;
                dq->memprev->memnext=dq;
                dq->mem = fblk->mem;
                // fix up free list ptrs
                fblk->next->prev = fblk->prev;
                fblk->prev->next = fblk->next;

                // return to meta list
                fblk->next = freemetahead;
                freemetahead = fblk->next;
            } else { // only needs some
                dq->mem -= (newsize - currsize);
            }

            // move data into place
            copy_data( dq->mem, oldmem, origsize );
        }
    }

    if (newsize < currsize) {
        // shrink allocation

        // easy if the next block is already a free block
        if ( dq->memnext != dq->next ) {
            dq->memnext->mem -= currsize - newsize;
            CYG_ASSERT( dq->memnext->mem > dq->mem,
                        "moving next block back corruption" );
        } else {
            // if its already allocated we need to create a new free list
            // entry
            if (NULL == freemetahead) {
		CYG_MEMALLOC_FAIL(newsize);
                return NULL;  // can't do it
	    }

            struct memdq *fdq = freemetahead;
            freemetahead = fdq->next;

            fdq->memprev = dq;
            fdq->memnext = dq->memnext;
            fdq->mem = dq->mem + newsize;
            
            insert_free_block( fdq );
        }
    }

    freemem += origsize - newsize;

    return dq->mem;
} // resize_alloc()


// -------------------------------------------------------------------------
// When no coalescing is done, free is simply a matter of using the
// freed memory as an element of the free list linking it in at the
// start. When coalescing, the free list is sorted
    
inline cyg_bool
Cyg_Mempool_Sepmeta_Implementation::free( cyg_uint8 *p, cyg_int32 size )
{
    CYG_REPORT_FUNCTION();

    CYG_CHECK_DATA_PTRC( p );

    if (!((bottom <= p) && (p <= top)))
        return false;
    
    struct memdq *dq = find_alloced_dq( p );
    if (NULL == dq)
        return false;
    
    if (0 == size)
        size = dq->memnext->mem - dq->mem;
    else {
        size = (size + alignment - 1) & -alignment;
        if( (dq->memnext->mem - dq->mem) != size )
            return false;
    }
    
    check_alloced_memdq( dq );

    // Remove dq from alloced list
    dq->prev->next = dq->next;
    dq->next->prev = dq->prev;

    insert_free_block( dq );

    freemem += size;

    return true;
}    

// -------------------------------------------------------------------------

inline void
Cyg_Mempool_Sepmeta_Implementation::get_status(
    cyg_mempool_status_flag_t flags,
    Cyg_Mempool_Status &status )
{
    CYG_REPORT_FUNCTION();

// as quick or quicker to just set it, rather than test flag first
    status.arenabase = obase;
    if ( 0 != (flags & CYG_MEMPOOL_STAT_ARENASIZE) )
        status.arenasize = top - bottom;
    if ( 0 != (flags & CYG_MEMPOOL_STAT_TOTALALLOCATED) )
        status.totalallocated = (top-bottom) - freemem;
// as quick or quicker to just set it, rather than test flag first
    status.totalfree = freemem;
    if ( 0 != (flags & CYG_MEMPOOL_STAT_MAXFREE) ) {
        struct memdq *dq = &freehead;
        cyg_int32 mf = 0;
        
        do {
            CYG_ASSERT( dq->next->prev==dq, "Bad link in dq");
            dq = dq->next;
            if (dq == &freehead) // wrapped round
                break;            
            if(dq->memnext->mem - dq->mem > mf)
                mf = dq->memnext->mem - dq->mem;
        } while(1);
        status.maxfree = mf;
    }
// as quick or quicker to just set it, rather than test flag first
    status.origbase = obase;
// as quick or quicker to just set it, rather than test flag first
    status.origsize = osize;
        
    CYG_REPORT_RETURN();

} // get_status()


// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_MEMALLOC_SEPMETAIMPL_INL
// EOF sepmetaimpl.inl
