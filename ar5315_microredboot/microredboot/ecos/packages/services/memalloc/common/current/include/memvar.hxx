#ifndef CYGONCE_MEMALLOC_MEMVAR_HXX
#define CYGONCE_MEMALLOC_MEMVAR_HXX

//==========================================================================
//
//      memvar.hxx
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
// Author(s):    dsm, jlarmour
// Contributors: 
// Date:         2000-06-12
// Purpose:      Define Memvar class interface
// Description:  Inline class for constructing a variable block allocator
// Usage:        #include <cyg/memalloc/memvar.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// CONFIGURATION

#include <pkgconf/memalloc.h>
#ifdef CYGSEM_MEMALLOC_ALLOCATOR_VARIABLE_THREADAWARE
# include <pkgconf/system.h>
# ifdef CYGPKG_KERNEL
#  include <pkgconf/kernel.h>
# endif
#endif

// when used as an implementation for malloc, we need the following
// to let the system know the name of the class
#define CYGCLS_MEMALLOC_MALLOC_IMPL Cyg_Mempool_Variable

// if the implementation is all that's required, don't output anything else
#ifndef __MALLOC_IMPL_WANTED
// INCLUDES

#include <cyg/infra/cyg_type.h>        // types
#include <cyg/infra/cyg_ass.h>         // assertion macros

#ifdef CYGFUN_KERNEL_THREADS_TIMER
# include <cyg/kernel/ktypes.h>        // cyg_tick_count
#endif

#ifdef CYGSEM_MEMALLOC_ALLOCATOR_VARIABLE_THREADAWARE
# include <cyg/memalloc/mempolt2.hxx>  // kernel safe mempool template
#endif

#include <cyg/memalloc/mvarimpl.hxx>   // implementation of a variable mem pool
#include <cyg/memalloc/common.hxx>     // Common memory allocator infra


// TYPE DEFINITIONS

class Cyg_Mempool_Variable
{
protected:
#ifdef CYGSEM_MEMALLOC_ALLOCATOR_VARIABLE_THREADAWARE
    Cyg_Mempolt2<Cyg_Mempool_Variable_Implementation> mypool;
#else
    Cyg_Mempool_Variable_Implementation mypool;
#endif

public:
    // This API makes concrete a class which implements a thread-safe
    // kernel-savvy memory pool which manages variable size blocks.

    // Constructor: gives the base and size of the arena in which memory is
    // to be carved out, note that management structures are taken from the
    // same arena.
    Cyg_Mempool_Variable( cyg_uint8 * /* base */, cyg_int32  /* size */,
                          cyg_int32 /* alignment */=8);

    // Destructor
    ~Cyg_Mempool_Variable();

    // get some memory; wait if none available
    // if we aren't configured to be thread-aware this is irrelevant
#ifdef CYGSEM_MEMALLOC_ALLOCATOR_VARIABLE_THREADAWARE
    cyg_uint8 *
    alloc( cyg_int32 /* size */ );
    
# ifdef CYGFUN_KERNEL_THREADS_TIMER
    // get some memory with a timeout
    cyg_uint8 *
    alloc( cyg_int32 /* size */, cyg_tick_count /* delay_timeout */ );
# endif
#endif

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
                  cyg_int32 * /* oldsize */ =NULL );

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

    CYGDBG_DEFINE_CHECK_THIS
};

#endif // ifndef __MALLOC_IMPL_WANTED

#endif // ifndef CYGONCE_MEMALLOC_MEMVAR_HXX
// EOF memvar.hxx
