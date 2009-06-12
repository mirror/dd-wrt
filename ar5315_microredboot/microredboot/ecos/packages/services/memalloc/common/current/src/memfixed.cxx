//==========================================================================
//
//      memfixed.cxx
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
// Date:         2000-06-16
// Purpose:      Define Memfixed class interface
// Description:  Inline class for constructing a fixed block allocator
// Usage:        #include <cyg/memalloc/memfixed.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// CONFIGURATION

#include <pkgconf/memalloc.h>
#include <pkgconf/system.h>
#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
#endif


// INCLUDES

#include <cyg/infra/cyg_type.h>        // types
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/infra/cyg_trac.h>        // tracing macros

#ifdef CYGFUN_KERNEL_THREADS_TIMER
# include <cyg/kernel/ktypes.h>        // cyg_tick_count
#endif

#ifdef CYGSEM_MEMALLOC_ALLOCATOR_FIXED_THREADAWARE
// tell it to optimize for the fixed block one-to-one case
# define CYGIMP_MEM_T_ONEFREE_TO_ONEALLOC
# include <cyg/memalloc/mempolt2.hxx>  // kernel safe mempool template
#endif

#include <cyg/memalloc/memfixed.hxx>
#include <cyg/memalloc/mfiximpl.hxx>   // implementation of a fixed mem pool
#include <cyg/memalloc/common.hxx>     // Common memory allocator infra

// -------------------------------------------------------------------------
// debugging/assert function

#ifdef CYGDBG_USE_ASSERTS
cyg_bool
Cyg_Mempool_Fixed::check_this(cyg_assert_class_zeal zeal) const
{
    CYG_REPORT_FUNCTION();
    // check that we have a non-NULL pointer first
    if( this == NULL ) return false;
#ifdef CYGSEM_MEMALLOC_ALLOCATOR_FIXED_THREADAWARE
    return mypool.check_this( zeal );
#else
    return true;
#endif
}
#endif

// -------------------------------------------------------------------------
// Constructor: gives the base and size of the arena in which memory is
// to be carved out, note that management structures are taken from the
// same arena.  Alloc_unit is the blocksize allocated.
Cyg_Mempool_Fixed::Cyg_Mempool_Fixed(
    cyg_uint8 *base,
    cyg_int32 size,
    CYG_ADDRWORD alloc_unit )
    : mypool( base, size, alloc_unit )
{
}

// Destructor
Cyg_Mempool_Fixed::~Cyg_Mempool_Fixed()
{
}

// -------------------------------------------------------------------------
// get some memory; wait if none available
#ifdef CYGSEM_MEMALLOC_ALLOCATOR_FIXED_THREADAWARE
cyg_uint8 *
Cyg_Mempool_Fixed::alloc()
{
    return mypool.alloc( 0 );
}
    
# ifdef CYGFUN_KERNEL_THREADS_TIMER
// get some memory with a timeout
cyg_uint8 *
Cyg_Mempool_Fixed::alloc(cyg_tick_count delay_timeout )
{
    return mypool.alloc( 0, delay_timeout );
}
# endif
#endif

// get some memory, return NULL if none available
cyg_uint8 *
Cyg_Mempool_Fixed::try_alloc()
{
    return mypool.try_alloc( 0 );
}
    
// free the memory back to the pool
cyg_bool 
Cyg_Mempool_Fixed::free( cyg_uint8 *p )
{
    return mypool.free( p, 0 );
}

// supposedly resize existing allocation. This is defined in the
// fixed block allocator purely for API consistency. It will return
// an error (false) for all values, except for the blocksize
// returns true on success
cyg_uint8 *
Cyg_Mempool_Fixed::resize_alloc( cyg_uint8 *alloc_ptr, cyg_int32 newsize,
                                 cyg_int32 *oldsize )
{
    return mypool.resize_alloc( alloc_ptr, newsize, oldsize );
}

// Get memory pool status
void
Cyg_Mempool_Fixed::get_status( cyg_mempool_status_flag_t flags,
                               Cyg_Mempool_Status &status )
{
    // set to 0 - if there's anything really waiting, it will be set to
    // 1 later
    status.waiting = 0;

    return mypool.get_status( flags, status );
}

// -------------------------------------------------------------------------

// End of memfixed.cxx
