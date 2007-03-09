//==========================================================================
//
//      kapi.cxx
//
//      Implementation of kernel C API functions for memory pools
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
// Author(s):    nickg, dsm, jlarmour
// Contributors: 
// Date:         2000-06-12
// Description:  Implementation of kernel C API functions for memory pools
// Usage:        
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

#ifdef CYGFUN_MEMALLOC_KAPI

// INCLUDES

#include <cyg/infra/cyg_type.h>        // types
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/kernel/ktypes.h>         // base kernel types

#include <cyg/memalloc/memvar.hxx>
#include <cyg/memalloc/memfixed.hxx>
#include <cyg/memalloc/common.hxx>     // status flags

#include <cyg/kernel/kapi.h>           // C API

// MACROS

#ifdef CYGDBG_USE_ASSERTS

#define CYG_ASSERT_SIZES(cstruct, cxxstruct)                      \
CYG_MACRO_START                                                   \
    char *msg = "Size of C struct " #cstruct                      \
                       " != size of C++ struct " #cxxstruct ;     \
    CYG_ASSERT( sizeof(cstruct) == sizeof(cxxstruct) , msg );     \
CYG_MACRO_END

#else

#define CYG_ASSERT_SIZES(cstruct, cxxstruct)

#endif

// FUNCTIONS

// -------------------------------------------------------------------------
// Magic new function

inline void *operator new(size_t size, void *ptr)
{
    CYG_CHECK_DATA_PTR( ptr, "Bad pointer" );
    return ptr;
}

/*-----------------------------------------------------------------------*/
/* Memory pools                                                          */

/* Create a variable size memory pool */
externC void cyg_mempool_var_create(
    void            *base,              /* base of memory to use for pool */
    cyg_int32       size,               /* size of memory in bytes        */
    cyg_handle_t    *handle,            /* returned handle of memory pool */
    cyg_mempool_var *var                /* space to put pool structure in */
    ) __THROW
{
    CYG_ASSERT_SIZES( cyg_mempool_var, Cyg_Mempool_Variable );

    Cyg_Mempool_Variable *t = new((void *)var) Cyg_Mempool_Variable (
        (cyg_uint8 *)base,
        size
    );
    t=t;

    CYG_CHECK_DATA_PTR( handle, "Bad handle pointer" );
    *handle = (cyg_handle_t)var;
}

/* Delete variable size memory pool */
externC void cyg_mempool_var_delete(cyg_handle_t varpool) __THROW
{
    ((Cyg_Mempool_Variable *)varpool)->~Cyg_Mempool_Variable();
}

/* Allocates a block of length size.  This waits if the memory is not
   currently available.  */
#ifdef CYGSEM_MEMALLOC_ALLOCATOR_VARIABLE_THREADAWARE
externC void *cyg_mempool_var_alloc(cyg_handle_t varpool, cyg_int32 size) __THROW
{
    return ((Cyg_Mempool_Variable *)varpool)->alloc(size);
}

# ifdef CYGFUN_KERNEL_THREADS_TIMER

/* Allocates a block of length size.  This waits for up to delay
   ticks, if the memory is not already available.  NULL is returned if
   no memory is available. */
externC void *cyg_mempool_var_timed_alloc(
    cyg_handle_t     varpool,
    cyg_int32        size,
    cyg_tick_count_t abstime) __THROW
{
    return ((Cyg_Mempool_Variable *)varpool)->alloc(size, abstime);
}

# endif
#endif

/* Allocates a block of length size.  NULL is returned if no memory is
   available. */
externC void *cyg_mempool_var_try_alloc(
    cyg_handle_t varpool,
    cyg_int32    size) __THROW
{
    return ((Cyg_Mempool_Variable *)varpool)->try_alloc(size);
}

/* Frees memory back into variable size pool. */
externC void cyg_mempool_var_free(cyg_handle_t varpool, void *p) __THROW
{
    cyg_bool b;
    b = ((Cyg_Mempool_Variable *)varpool)->free((cyg_uint8 *)p, 0);
    CYG_ASSERT( b, "Bad free");
}


/* Returns true if there are any threads waiting for memory in the
   given memory pool. */
externC cyg_bool_t cyg_mempool_var_waiting(cyg_handle_t varpool) __THROW
{
    Cyg_Mempool_Variable *v = (Cyg_Mempool_Variable *)varpool;
    Cyg_Mempool_Status stat;

    v->get_status( CYG_MEMPOOL_STAT_WAITING, stat );
    return (stat.waiting != 0);
}

/* Puts information about a variable memory pool into the structure
   provided. */
externC void cyg_mempool_var_get_info(
    cyg_handle_t varpool,
    cyg_mempool_info *info) __THROW
{
    Cyg_Mempool_Variable *v = (Cyg_Mempool_Variable *)varpool;
    Cyg_Mempool_Status stat;

    v->get_status( CYG_MEMPOOL_STAT_ARENASIZE|
                   CYG_MEMPOOL_STAT_TOTALFREE|
                   CYG_MEMPOOL_STAT_ARENABASE|
                   CYG_MEMPOOL_STAT_ORIGSIZE|
                   CYG_MEMPOOL_STAT_MAXFREE, stat );

    info->totalmem = stat.arenasize;
    info->freemem  = stat.totalfree;
    info->size = stat.origsize;
    info->base = const_cast<cyg_uint8 *>(stat.arenabase);
    info->blocksize  = -1;
    info->maxfree = stat.maxfree;
}


/* Create a fixed size memory pool */
externC void cyg_mempool_fix_create(
    void            *base,              // base of memory to use for pool
    cyg_int32       size,               // size of memory in byte
    cyg_int32       blocksize,          // size of allocation in bytes
    cyg_handle_t    *handle,            // handle of memory pool
    cyg_mempool_fix *fix                // space to put pool structure in
    ) __THROW
{
    CYG_ASSERT_SIZES( cyg_mempool_fix, Cyg_Mempool_Fixed );

    Cyg_Mempool_Fixed *t = new((void *)fix) Cyg_Mempool_Fixed (
        (cyg_uint8 *)base,
        size,
        blocksize
    );
    t=t;

    CYG_CHECK_DATA_PTR( handle, "Bad handle pointer" );
    *handle = (cyg_handle_t)fix;
}

/* Delete fixed size memory pool */
externC void cyg_mempool_fix_delete(cyg_handle_t fixpool) __THROW
{
    ((Cyg_Mempool_Fixed *)fixpool)->~Cyg_Mempool_Fixed();
}

#ifdef CYGSEM_MEMALLOC_ALLOCATOR_FIXED_THREADAWARE
/* Allocates a block.  This waits if the memory is not
   currently available.  */
externC void *cyg_mempool_fix_alloc(cyg_handle_t fixpool) __THROW
{
    return ((Cyg_Mempool_Fixed *)fixpool)->alloc();
}

# ifdef CYGFUN_KERNEL_THREADS_TIMER

/* Allocates a block.  This waits for up to delay ticks, if the memory
   is not already available.  NULL is returned if no memory is
   available. */
externC void *cyg_mempool_fix_timed_alloc(
    cyg_handle_t     fixpool,
    cyg_tick_count_t abstime) __THROW
{
    return ((Cyg_Mempool_Fixed *)fixpool)->alloc(abstime);
}

# endif
#endif

/* Allocates a block.  NULL is returned if no memory is available. */
externC void *cyg_mempool_fix_try_alloc(cyg_handle_t fixpool) __THROW
{
    return ((Cyg_Mempool_Fixed *)fixpool)->try_alloc();
}

/* Frees memory back into fixed size pool. */
externC void cyg_mempool_fix_free(cyg_handle_t fixpool, void *p) __THROW
{
    cyg_bool b;
    b = ((Cyg_Mempool_Fixed *)fixpool)->free((cyg_uint8 *)p);
    CYG_ASSERT( b, "Bad free");
}

/* Returns true if there are any threads waiting for memory in the
   given memory pool. */
externC cyg_bool_t cyg_mempool_fix_waiting(cyg_handle_t fixpool) __THROW
{
    Cyg_Mempool_Fixed *f = (Cyg_Mempool_Fixed *)fixpool;
    Cyg_Mempool_Status stat;

    f->get_status( CYG_MEMPOOL_STAT_WAITING, stat );
    return (stat.waiting != 0);
}

/* Puts information about a fixed block memory pool into the structure
   provided. */
externC void cyg_mempool_fix_get_info(
    cyg_handle_t fixpool,
    cyg_mempool_info *info) __THROW
{
    Cyg_Mempool_Fixed *f = (Cyg_Mempool_Fixed *)fixpool;
    Cyg_Mempool_Status stat;

    f->get_status( CYG_MEMPOOL_STAT_ARENASIZE|
                   CYG_MEMPOOL_STAT_TOTALFREE|
                   CYG_MEMPOOL_STAT_ARENABASE|
                   CYG_MEMPOOL_STAT_ORIGSIZE|
                   CYG_MEMPOOL_STAT_BLOCKSIZE|
                   CYG_MEMPOOL_STAT_MAXFREE, stat );

    info->totalmem = stat.arenasize;
    info->freemem  = stat.totalfree;
    info->size = stat.origsize;
    info->base = const_cast<cyg_uint8 *>(stat.arenabase);
    info->blocksize  = stat.blocksize;
    info->maxfree = stat.maxfree;
}

// -------------------------------------------------------------------------
// Check structure sizes.
// This class and constructor get run automatically in debug versions
// of the kernel and check that the structures configured in the C
// code are the same size as the C++ classes they should match.

#ifdef CYGPKG_INFRA_DEBUG

class Cyg_Check_Mem_Structure_Sizes
{
    int dummy;
public:    
    Cyg_Check_Mem_Structure_Sizes( int x ) __THROW;

};

#define CYG_CHECK_SIZES(cstruct, cxxstruct)                               \
if( sizeof(cstruct) != sizeof(cxxstruct) )                                \
{                                                                         \
    char *fmt = "Size of C struct " #cstruct                              \
                " != size of C++ struct " #cxxstruct ;                    \
    CYG_TRACE2(1, fmt, sizeof(cstruct) , sizeof(cxxstruct) );             \
    fail = true;                                                          \
    fmt = fmt;                                                            \
}

Cyg_Check_Mem_Structure_Sizes::Cyg_Check_Mem_Structure_Sizes(int x) __THROW
{
    cyg_bool fail = false;

    dummy = x+1;
    
    CYG_CHECK_SIZES( cyg_mempool_var, Cyg_Mempool_Variable );
    CYG_CHECK_SIZES( cyg_mempool_fix, Cyg_Mempool_Fixed );
    
    CYG_ASSERT( !fail, "Size checks failed");
}

static Cyg_Check_Mem_Structure_Sizes cyg_memalloc_check_structure_sizes(1);

#endif

// -------------------------------------------------------------------------


#endif // ifdef CYGFUN_MEMALLOC_KAPI

// End of kapi.cxx
