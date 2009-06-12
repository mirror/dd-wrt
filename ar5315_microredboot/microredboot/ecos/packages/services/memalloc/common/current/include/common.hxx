#ifndef CYGONCE_MEMALLOC_COMMON_HXX
#define CYGONCE_MEMALLOC_COMMON_HXX

/*==========================================================================
//
//      common.hxx
//
//      Shared definitions used by memory allocators
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
// Purpose:      Shared definitions used by memory allocators
// Description:  
// Usage:        #include <cyg/memalloc/common.hxx>
//              
//
//####DESCRIPTIONEND####
//
//========================================================================*/

/* CONFIGURATION */

#include <pkgconf/memalloc.h>

/* TYPE DEFINITIONS */

// struct Cyg_Mempool_Status is returned by the get_status() method of
// standard eCos memory allocators. After return from get_status(), any
// field of type T may be set to ((T)-1) to indicate that the information
// is not available or not applicable to this allocator.


class Cyg_Mempool_Status {
public:
    const cyg_uint8 *arenabase; // base address of entire pool
    cyg_int32   arenasize;      // total size of entire pool
    cyg_int32   freeblocks;     // number of chunks free for use
    cyg_int32   totalallocated; // total allocated space in bytes
    cyg_int32   totalfree;      // total space in bytes not in use
    cyg_int32   blocksize;      // block size if fixed block
    cyg_int32   maxfree;        // size of largest unused block
    cyg_int8    waiting;        // are there any threads waiting for memory?
    const cyg_uint8 *origbase;  // address of original region used when pool
                                // created
    cyg_int32   origsize;       // size of original region used when pool
                                // created

    // maxoverhead is the *maximum* per-allocation overhead imposed by
    // the allocator implementation. Note: this is rarely the typical
    // overhead which often depends on the size of the allocation requested.
    // It includes overhead due to alignment constraints. For example, if
    // maxfree and maxoverhead are available for this allocator, then an
    // allocation request of (maxfree-maxoverhead) bytes must always succeed
    // Unless maxoverhead is set to -1 of course, in which case the allocator
    // does not support reporting this information.

    cyg_int8    maxoverhead;    

    void
    init() {
        arenabase       = (const cyg_uint8 *)-1;
        arenasize       = -1;
        freeblocks      = -1;
        totalallocated  = -1;
        totalfree       = -1;
        blocksize       = -1;
        maxfree         = -1;
        waiting         = -1;
        origbase        = (const cyg_uint8 *)-1;
        origsize        = -1;
        maxoverhead     = -1;
    }

    // constructor
    Cyg_Mempool_Status() { init(); }
};

// Flags to pass to get_status() methods to tell it which stat(s) is/are
// being requested

#define CYG_MEMPOOL_STAT_ARENABASE       (1<<0)
#define CYG_MEMPOOL_STAT_ARENASIZE       (1<<1)
#define CYG_MEMPOOL_STAT_FREEBLOCKS      (1<<2)
#define CYG_MEMPOOL_STAT_TOTALALLOCATED  (1<<3)
#define CYG_MEMPOOL_STAT_TOTALFREE       (1<<4)
#define CYG_MEMPOOL_STAT_BLOCKSIZE       (1<<5)
#define CYG_MEMPOOL_STAT_MAXFREE         (1<<6)
#define CYG_MEMPOOL_STAT_WAITING         (1<<7)
#define CYG_MEMPOOL_STAT_ORIGBASE        (1<<9)
#define CYG_MEMPOOL_STAT_ORIGSIZE        (1<<10)
#define CYG_MEMPOOL_STAT_MAXOVERHEAD     (1<<11)

// And an opaque type for any arguments with these flags
typedef cyg_uint16 cyg_mempool_status_flag_t;

// breakpoint site for out of memory conditions
#ifdef CYGSEM_MEMALLOC_INVOKE_OUT_OF_MEMORY
#include <cyg/memalloc/kapi.h> // protoype for cyg_memalloc_alloc_fail
#define CYG_MEMALLOC_FAIL_TEST( test, size )                \
   CYG_MACRO_START                                          \
   if ( test) {                                             \
        cyg_memalloc_alloc_fail(__FILE__, __LINE__, size ); \
   }                                                        \
   CYG_MACRO_END
#define CYG_MEMALLOC_FAIL( size)                            \
   CYG_MACRO_START                                          \
   cyg_memalloc_alloc_fail(__FILE__, __LINE__, size );      \
   CYG_MACRO_END
#else
#define CYG_MEMALLOC_FAIL_TEST( test, size )  CYG_EMPTY_STATEMENT
#define CYG_MEMALLOC_FAIL( size )             CYG_EMPTY_STATEMENT
#endif        

#endif /* ifndef CYGONCE_MEMALLOC_COMMON_HXX */
/* EOF common.hxx */
