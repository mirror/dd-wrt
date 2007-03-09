#ifndef CYGONCE_MEMALLOC_MFIXIMPL_INL
#define CYGONCE_MEMALLOC_MFIXIMPL_INL

//==========================================================================
//
//      mfiximpl.inl
//
//      Memory pool with fixed block class declarations
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
// Purpose:      Define Mfiximpl class interface
// Description:  Inline class for constructing a fixed block allocator
// Usage:        #include <cyg/kernel/mfiximpl.hxx>
//
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/memalloc.h>
#include <cyg/hal/hal_arch.h>          // HAL_LSBIT_INDEX magic asm code
#include <cyg/memalloc/mfiximpl.hxx>


// -------------------------------------------------------------------------

inline
Cyg_Mempool_Fixed_Implementation::Cyg_Mempool_Fixed_Implementation(
        cyg_uint8 *base,
        cyg_int32 size,
        CYG_ADDRWORD alloc_unit )
{
    cyg_int32 i;
    bitmap = (cyg_uint32 *)base;
    blocksize = alloc_unit;

    CYG_ASSERT( blocksize > 0, "Bad blocksize" );
    CYG_ASSERT( size > 2, "Bad blocksize" );
    CYG_ASSERT( blocksize < size, "blocksize, size bad" );

    numblocks = size / blocksize;
    top = base + size;

    CYG_ASSERT( numblocks >= 2, "numblocks bad" );

    i = (numblocks + 31)/32;        // number of words to map blocks
    while ( (i * 4 + numblocks * blocksize) > size ) {
        numblocks --;               // steal one block for admin
        i = (numblocks + 31)/32;    // number of words to map blocks
    }

    CYG_ASSERT( 0 < i, "Bad word count for bitmap after fitment" );
    CYG_ASSERT( 0 < numblocks, "Bad block count after fitment" );

    maptop = i;
    // this should leave space for the bitmap and maintain alignment
    mempool = top - (numblocks * blocksize);
    CYG_ASSERT( base < mempool && mempool < top, "mempool escaped" );
    CYG_ASSERT( (cyg_uint8 *)(&bitmap[ maptop ]) <= mempool,
                "mempool overwrites bitmap" );
    CYG_ASSERT( &mempool[ numblocks * blocksize ] <= top,
                "mempool overflows top" );
    freeblocks = numblocks;
    firstfree = 0;

    // clear out the bitmap; no blocks allocated yet
    for ( i = 0; i < maptop; i++ )
        bitmap[ i ] = 0;
    // apart from the non-existent ones at the top
    for ( i = ((numblocks-1)&31) + 1; i < 32; i++ )
        bitmap[ maptop - 1 ] |= ( 1 << i );
}

// -------------------------------------------------------------------------

inline
Cyg_Mempool_Fixed_Implementation::~Cyg_Mempool_Fixed_Implementation()
{
}

// -------------------------------------------------------------------------

inline cyg_uint8 *
Cyg_Mempool_Fixed_Implementation::try_alloc( cyg_int32 size )
{
    // size parameter is not used
    CYG_UNUSED_PARAM( cyg_int32, size );
    if ( 0 >= freeblocks ) {
	CYG_MEMALLOC_FAIL(size);
        return NULL;
    }
    cyg_int32 i = firstfree;
    cyg_uint8 *p = NULL;
    do {
        if ( 0xffffffff != bitmap[ i ] ) {
            // then there is a free block in this bucket
            register cyg_uint32 j, k;
            k = ~bitmap[ i ];       // look for a 1 in complement
            HAL_LSBIT_INDEX( j, k );
            CYG_ASSERT( 0 <= j && j <= 31, "Bad bit index" );
            CYG_ASSERT( 0 == (bitmap[ i ] & (1 << j)), "Found bit not clear" );
            bitmap[ i ] |= (1 << j); // set it allocated
            firstfree = i;
            freeblocks--;
            CYG_ASSERT( freeblocks >= 0, "allocated too many" );
            p = &mempool[ ((32 * i) + j) * blocksize ];
            break;
        }
        if ( ++i >= maptop )
            i = 0;                  // wrap if at top
    } while ( i != firstfree );     // prevent hang if internal error
    CYG_ASSERT( NULL != p, "Should have a block here" );
    CYG_ASSERT( mempool <= p  && p <= top, "alloc mem escaped" );
    return p;
}
    
// -------------------------------------------------------------------------
// supposedly resize existing allocation. This is defined in the
// fixed block allocator purely for API consistency. It will return
// an error (false) for all values, except for the blocksize
// returns true on success

inline cyg_uint8 *
Cyg_Mempool_Fixed_Implementation::resize_alloc( cyg_uint8 *alloc_ptr,
                                                cyg_int32 newsize,
                                                cyg_int32 *oldsize )
{
    CYG_CHECK_DATA_PTRC( alloc_ptr );
    if ( NULL != oldsize )
        CYG_CHECK_DATA_PTRC( oldsize );

    CYG_ASSERT( alloc_ptr >= mempool && alloc_ptr < top,
                "alloc_ptr outside pool" );
    
    if ( NULL != oldsize )
        *oldsize = blocksize;

    if (newsize == blocksize)
        return alloc_ptr;
    else {
	CYG_MEMALLOC_FAIL(newsize);
        return NULL;
    }
} // resize_alloc()


// -------------------------------------------------------------------------

inline cyg_bool
Cyg_Mempool_Fixed_Implementation::free( cyg_uint8 *p, cyg_int32 size )
{
    // size parameter is not used
    CYG_UNUSED_PARAM( cyg_int32, size );
    if ( p < mempool || p >= top )
        return false;               // address way out of bounds
    cyg_int32 i = p - mempool;
    i = i / blocksize;
    if ( &mempool[ i * blocksize ] != p )
        return false;               // address not aligned
    cyg_int32 j = i / 32;
    CYG_ASSERT( 0 <= j && j < maptop, "map index escaped" );
    i = i - 32 * j;
    CYG_ASSERT( 0 <= i && i < 32, "map bit index escaped" );
    if ( ! ((1 << i) & bitmap[ j ] ) )
        return false;               // block was not allocated
    bitmap[ j ] &=~(1 << i);        // clear the bit
    freeblocks++;                   // count the block
    CYG_ASSERT( freeblocks <= numblocks, "freeblocks overflow" );
    return true;
}

// -------------------------------------------------------------------------

inline void
Cyg_Mempool_Fixed_Implementation::get_status(
    cyg_mempool_status_flag_t flags,
    Cyg_Mempool_Status &status )
{
// as quick or quicker to just set it, rather than test flag first
    status.arenabase = (const cyg_uint8 *)bitmap;
    if ( 0 != (flags & CYG_MEMPOOL_STAT_ARENASIZE) )
        status.arenasize = top - (cyg_uint8 *)bitmap;
    if ( 0 != (flags & CYG_MEMPOOL_STAT_FREEBLOCKS) )
        status.freeblocks = freeblocks;
    if ( 0 != (flags & CYG_MEMPOOL_STAT_TOTALALLOCATED) )
        status.totalallocated = blocksize * numblocks;
    if ( 0 != (flags & CYG_MEMPOOL_STAT_TOTALFREE) )
        status.totalfree = blocksize * freeblocks;
    if ( 0 != (flags & CYG_MEMPOOL_STAT_BLOCKSIZE) )
        status.blocksize = blocksize;
    if ( 0 != (flags & CYG_MEMPOOL_STAT_MAXFREE) ) {
        status.maxfree = freeblocks > 0 ? blocksize : 0;
    }
// as quick or quicker to just set it, rather than test flag first
    status.origbase = (const cyg_uint8 *)bitmap;
    if ( 0 != (flags & CYG_MEMPOOL_STAT_ORIGSIZE) )
        status.origsize = top - (cyg_uint8 *)bitmap;
// quicker to just set it, rather than test flag first
    status.maxoverhead = 0;
        
} // get_status()

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_MEMALLOC_MFIXIMPL_INL
// EOF mfiximpl.inl
