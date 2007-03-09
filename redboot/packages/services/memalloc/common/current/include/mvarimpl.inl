#ifndef CYGONCE_MEMALLOC_MVARIMPL_INL
#define CYGONCE_MEMALLOC_MVARIMPL_INL

//==========================================================================
//
//      mvarimpl.inl
//
//      Memory pool with variable block class declarations
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
// Author(s):    hmt
// Contributors: jlarmour
// Date:         2000-06-12
// Purpose:      Define Mvarimpl class interface
// Description:  Inline class for constructing a variable block allocator
// Usage:        #include <cyg/memalloc/mvarimpl.hxx>
//
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/memalloc.h>
#include <cyg/memalloc/mvarimpl.hxx>

#include <cyg/infra/cyg_ass.h>           // assertion support
#include <cyg/infra/cyg_trac.h>          // tracing support

// Simple allocator

// The free list is stored on a doubly linked list, each member of
// which is stored in the body of the free memory.  The head of the
// list has the same structure but its size field is zero.  This
// resides in the memory pool structure.  Always having at least one
// item on the list simplifies the alloc and free code.

// 
inline cyg_int32
Cyg_Mempool_Variable_Implementation::roundup( cyg_int32 size )
{

    size += sizeof(struct memdq);
    size = (size + alignment - 1) & -alignment;
    return size;
}

inline struct Cyg_Mempool_Variable_Implementation::memdq *
Cyg_Mempool_Variable_Implementation::addr2memdq( cyg_uint8 *addr )
{
    struct memdq *dq;
    dq = (struct memdq *)(roundup((cyg_int32)addr) - sizeof(struct memdq));
    return dq;
}

inline struct Cyg_Mempool_Variable_Implementation::memdq *
Cyg_Mempool_Variable_Implementation::alloc2memdq( cyg_uint8 *addr )
{
    return (struct memdq *)(addr - sizeof(struct memdq));
}

inline cyg_uint8 *
Cyg_Mempool_Variable_Implementation::memdq2alloc( struct memdq *dq )
{
    return ((cyg_uint8 *)dq + sizeof(struct memdq));
}

// -------------------------------------------------------------------------

inline void
Cyg_Mempool_Variable_Implementation::insert_free_block( struct memdq *dq )
{
    struct memdq *hdq=&head;

    freemem += dq->size;
#ifdef CYGSEM_MEMALLOC_ALLOCATOR_VARIABLE_COALESCE
// For simple coalescing have the free list be sorted by memory base address
    struct memdq *idq;
    
    for (idq = hdq->next; idq != hdq; idq = idq->next) {
        if (idq > dq)
            break;
    }
    // we want to insert immediately before idq
    dq->next = idq;
    dq->prev = idq->prev;
    idq->prev = dq;
    dq->prev->next = dq;

    // Now do coalescing, but leave the head of the list alone.
    if (dq->next != hdq && (char *)dq + dq->size == (char *)dq->next) {
        dq->size += dq->next->size;
        dq->next = dq->next->next;
        dq->next->prev = dq;
    }
    if (dq->prev != hdq && (char *)dq->prev + dq->prev->size == (char *)dq) {
        dq->prev->size += dq->size;
        dq->prev->next = dq->next;
        dq->next->prev = dq->prev;
        dq = dq->prev;
    }
#else
    dq->prev = hdq;
    dq->next = hdq->next;
    hdq->next = dq;
    dq->next->prev=dq;
#endif
}

// -------------------------------------------------------------------------

inline
Cyg_Mempool_Variable_Implementation::Cyg_Mempool_Variable_Implementation(
        cyg_uint8 *base,
        cyg_int32 size,
        CYG_ADDRWORD align )
{
    CYG_REPORT_FUNCTION();

    CYG_ASSERT( align > 0, "Bad alignment" );
    CYG_ASSERT(0!=align ,"align is zero");
    CYG_ASSERT(0==(align & align-1),"align not a power of 2");

    if ((unsigned)size < sizeof(struct memdq)) {
        bottom = NULL;
        return;
    }

    obase=base;
    osize=size;

    alignment = align;
    while (alignment < (cyg_int32)sizeof(struct memdq))
        alignment += alignment;
    CYG_ASSERT(0==(alignment & alignment-1),"alignment not a power of 2");

    // the memdq for each allocation is always positioned immediately before
    // an aligned address, so that the allocation (i.e. what eventually gets
    // returned from alloc()) is at the correctly aligned address
    // Therefore bottom is set to the lowest available address given the size of
    // struct memdq and the alignment. 
    bottom = (cyg_uint8 *)addr2memdq(base);

    // because we split free blocks by allocating memory from the end, not
    // the beginning, then to preserve alignment, the *top* must also be
    // aligned such that (top-bottom) is a multiple of the alignment
    top = (cyg_uint8 *)((cyg_int32)(base+size+sizeof(struct memdq)) & -alignment) -
        sizeof(struct memdq);
    
    CYG_ASSERT( top > bottom , "heap too small" );
    CYG_ASSERT( top <= (base+size), "top too large" );
    CYG_ASSERT( ((cyg_int32)(top+sizeof(struct memdq)) & alignment-1)==0,
                "top badly aligned" );

    struct memdq *hdq = &head, *dq = (struct memdq *)bottom;
    
    CYG_ASSERT( ((cyg_int32)memdq2alloc(dq) & alignment-1)==0,
                 "bottom badly aligned" );

    hdq->prev = hdq->next = dq;
    hdq->size = 0;
    dq->prev = dq->next = hdq;

    freemem = dq->size = top - bottom;
}

// -------------------------------------------------------------------------

inline
Cyg_Mempool_Variable_Implementation::~Cyg_Mempool_Variable_Implementation()
{
}

// -------------------------------------------------------------------------
// allocation is simple
// First we look down the free list for a large enough block
// If we find a block the right size, we unlink the block from
//    the free list and return a pointer to it.
// If we find a larger block, we chop a piece off the end
//    and return that
// Otherwise we will eventually get back to the head of the list
//    and return NULL
inline cyg_uint8 *
Cyg_Mempool_Variable_Implementation::try_alloc( cyg_int32 size )
{
    struct memdq *dq = &head;
    cyg_uint8 *alloced;

    CYG_REPORT_FUNCTION();

    //  Allow uninitialised (zero sized) heaps because they could exist as a
    //  quirk of the MLT setup where a dynamically sized heap is at the top of
    //  memory.
    if (NULL == bottom)
        return NULL;

    size = roundup(size);

    do {
        CYG_ASSERT( dq->next->prev==dq, "Bad link in dq");
        dq = dq->next;
        if(0 == dq->size) {
            CYG_ASSERT(dq == &head, "bad free block");
            return NULL;
        }
    } while(dq->size < size);

    if( size == dq->size ) {
        // exact fit -- unlink from free list
        dq->prev->next = dq->next;
        dq->next->prev = dq->prev;
        alloced = (cyg_uint8 *)dq;
    } else {

        CYG_ASSERT( dq->size > size, "block found is too small");

        // allocate portion of memory from end of block
        
        dq->size -=size;

        // The portion left over has to be large enough to store a
        // struct memdq.  This is guaranteed because the alignment is
        // larger than the size of this structure.

        CYG_ASSERT( (cyg_int32)sizeof(struct memdq)<=dq->size ,
                "not enough space for list item" );

        alloced = (cyg_uint8 *)dq + dq->size;
    }

    CYG_ASSERT( bottom<=alloced && alloced<=top, "alloced outside pool" );

    // Set size on allocated block

    dq = (struct memdq *)alloced;
    dq->size = size;
    dq->next = dq->prev = (struct memdq *)0xd530d53; // magic number

    freemem -=size;

    cyg_uint8 *ptr = memdq2alloc( dq );
    CYG_ASSERT( ((CYG_ADDRESS)ptr & (alignment-1)) == 0,
                "returned memory not aligned" );
    CYG_MEMALLOC_FAIL_TEST(ptr==NULL, size);

    return ptr;
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
Cyg_Mempool_Variable_Implementation::resize_alloc( cyg_uint8 *alloc_ptr,
                                                   cyg_int32 newsize,
                                                   cyg_int32 *oldsize )
{
    cyg_uint8 *ret = NULL;

    CYG_REPORT_FUNCTION();
    
    CYG_CHECK_DATA_PTRC( alloc_ptr );
    if ( NULL != oldsize )
        CYG_CHECK_DATA_PTRC( oldsize );

    CYG_ASSERT( (bottom <= alloc_ptr) && (alloc_ptr <= top),
                "alloc_ptr outside pool" );
    
    struct memdq *dq=alloc2memdq( alloc_ptr );
    
    // check magic number in block for validity
    CYG_ASSERT( (dq->next == dq->prev) &&
                (dq->next == (struct memdq *)0xd530d53), "bad alloc_ptr" );

    newsize = roundup(newsize);

    if ( NULL != oldsize )
        *oldsize = dq->size;

    if ( newsize > dq->size ) {
        // see if we can increase the allocation size
        if ( (cyg_uint8 *)dq + newsize <= top ) { // obviously can't exceed pool
            struct memdq *nextdq = (struct memdq *)((cyg_uint8 *)dq + dq->size);

            if ( (nextdq->next != nextdq->prev) &&
                 (nextdq->size >= (newsize - dq->size)) ) {
                // it's free and it's big enough
                // we therefore temporarily join this block and *all* of
                // the next block, so that the code below can then split it
                nextdq->next->prev = nextdq->prev;
                nextdq->prev->next = nextdq->next;
                dq->size += nextdq->size;
                freemem -= nextdq->size;
            }
        } // if
    } // if

    // this is also used if the allocation size was increased and we need
    // to split it
    if ( newsize < dq->size ) {
        // We can shrink the allocation by splitting into smaller allocation and
        // new free block
        struct memdq *newdq = (struct memdq *)((cyg_uint8 *)dq + newsize);
        
        newdq->size = dq->size - newsize;
        dq->size = newsize;
        
        CYG_ASSERT( (cyg_int32)sizeof(struct memdq)<=newdq->size ,
                    "not enough space for list item" );

        // now return the new space back to the freelist
        insert_free_block( newdq );
        
        ret = alloc_ptr;
        
    } // if
    else if ( newsize == dq->size ) {
        ret = alloc_ptr;
    }
        
    CYG_MEMALLOC_FAIL_TEST(ret==NULL, newsize);

    return ret;

} // resize_alloc()


// -------------------------------------------------------------------------
// When no coalescing is done, free is simply a matter of using the
// freed memory as an element of the free list linking it in at the
// start. When coalescing, the free list is sorted
    
inline cyg_bool
Cyg_Mempool_Variable_Implementation::free( cyg_uint8 *p, cyg_int32 size )
{
    CYG_REPORT_FUNCTION();

    CYG_CHECK_DATA_PTRC( p );

    if (!((bottom <= p) && (p <= top)))
        return false;
    
    struct memdq *dq=alloc2memdq( p );

    // check magic number in block for validity
    if ( (dq->next != dq->prev) ||
         (dq->next != (struct memdq *)0xd530d53) )
        return false;

    if ( 0==size ) {
        size = dq->size;
    } else {
        size = roundup(size);
    }

    if( dq->size != size )
        return false;

    CYG_ASSERT( (cyg_int32)sizeof(struct memdq)<=size ,
                "not enough space for list item" );

    insert_free_block( dq );

    return true;
}    

// -------------------------------------------------------------------------

inline void
Cyg_Mempool_Variable_Implementation::get_status(
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
        struct memdq *dq = &head;
        cyg_int32 mf = 0;
        
        do {
            CYG_ASSERT( dq->next->prev==dq, "Bad link in dq");
            dq = dq->next;
            if(0 == dq->size) {
                CYG_ASSERT(dq == &head, "bad free block");
                break;
            }
            if(dq->size > mf)
                mf = dq->size;
        } while(1);
        status.maxfree = mf - sizeof(struct memdq);
    }
// as quick or quicker to just set it, rather than test flag first
    status.origbase = obase;
// as quick or quicker to just set it, rather than test flag first
    status.origsize = osize;
        
    CYG_REPORT_RETURN();

} // get_status()


// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_MEMALLOC_MVARIMPL_INL
// EOF mvarimpl.inl
