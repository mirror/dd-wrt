#ifndef CYGONCE_MEMALLOC_KAPI_H
#define CYGONCE_MEMALLOC_KAPI_H

/*==========================================================================
//
//      kapi.h
//
//      Memory allocator portion of kernel C API
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
// Purpose:      Memory allocator portion of kernel C API
// Description:  This is intentionally only to be included from
//               <cyg/kernel/kapi.h>
// Usage:        This file should not be used directly - instead it should
//               be used via <cyg/kernel/kapi.h>
//              
//
//####DESCRIPTIONEND####
//
//========================================================================*/

/* CONFIGURATION */
#include <pkgconf/system.h>
#include <pkgconf/memalloc.h>

/* TYPE DEFINITIONS */
#ifdef CYGPKG_KERNEL
#include <cyg/kernel/kapi.h>
#else
typedef cyg_uint32 cyg_handle_t;
#endif

/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
struct cyg_mempool_var;
typedef struct cyg_mempool_var cyg_mempool_var;

struct cyg_mempool_fix;
typedef struct cyg_mempool_fix cyg_mempool_fix;

/*-----------------------------------------------------------------------*/
/* Memory pools                                                          */

/* There are two sorts of memory pools.  A variable size memory pool
   is for allocating blocks of any size.  A fixed size memory pool, has
   the block size specified when the pool is created, and only provides
   blocks of that size.  */

/* Create a variable size memory pool */
void cyg_mempool_var_create(
    void            *base,              /* base of memory to use for pool */
    cyg_int32       size,               /* size of memory in bytes        */
    cyg_handle_t    *handle,            /* returned handle of memory pool */
    cyg_mempool_var *var                /* space to put pool structure in */
    ) __THROW;

/* Delete variable size memory pool */
void cyg_mempool_var_delete(cyg_handle_t varpool) __THROW;

#ifdef CYGSEM_MEMALLOC_ALLOCATOR_VARIABLE_THREADAWARE

/* Allocates a block of length size.  This waits if the memory is not
   currently available.  */
void *cyg_mempool_var_alloc(cyg_handle_t varpool, cyg_int32 size) __THROW;

# ifdef CYGFUN_KERNEL_THREADS_TIMER

/* Allocates a block of length size.  This waits until abstime,
   if the memory is not already available.  NULL is returned if
   no memory is available. */
void *cyg_mempool_var_timed_alloc(
    cyg_handle_t     varpool,
    cyg_int32        size,
    cyg_tick_count_t abstime) __THROW;

# endif
#endif

/* Allocates a block of length size.  NULL is returned if no memory is
   available. */
void *cyg_mempool_var_try_alloc(
    cyg_handle_t varpool,
    cyg_int32    size) __THROW;

/* Frees memory back into variable size pool. */
void cyg_mempool_var_free(cyg_handle_t varpool, void *p) __THROW;

/* Returns true if there are any threads waiting for memory in the
   given memory pool. */
cyg_bool_t cyg_mempool_var_waiting(cyg_handle_t varpool) __THROW;

typedef struct {
    cyg_int32 totalmem;
    cyg_int32 freemem;
    void      *base;
    cyg_int32 size;
    cyg_int32 blocksize;
    cyg_int32 maxfree;                  // The largest free block
} cyg_mempool_info;

/* Puts information about a variable memory pool into the structure
   provided. */
void cyg_mempool_var_get_info(cyg_handle_t varpool, cyg_mempool_info *info) __THROW;

/* Create a fixed size memory pool */
void cyg_mempool_fix_create(
    void            *base,              // base of memory to use for pool
    cyg_int32       size,               // size of memory in byte
    cyg_int32       blocksize,          // size of allocation in bytes
    cyg_handle_t    *handle,            // handle of memory pool
    cyg_mempool_fix *fix                // space to put pool structure in
    ) __THROW;

/* Delete fixed size memory pool */
void cyg_mempool_fix_delete(cyg_handle_t fixpool) __THROW;

#ifdef CYGSEM_MEMALLOC_ALLOCATOR_FIXED_THREADAWARE
/* Allocates a block.  This waits if the memory is not
   currently available.  */
void *cyg_mempool_fix_alloc(cyg_handle_t fixpool) __THROW;

# ifdef CYGFUN_KERNEL_THREADS_TIMER

/* Allocates a block.  This waits until abstime, if the memory
   is not already available.  NULL is returned if no memory is
   available. */
void *cyg_mempool_fix_timed_alloc(
    cyg_handle_t     fixpool,
    cyg_tick_count_t abstime) __THROW;

# endif
#endif

/* Allocates a block.  NULL is returned if no memory is available. */
void *cyg_mempool_fix_try_alloc(cyg_handle_t fixpool) __THROW;

/* Frees memory back into fixed size pool. */
void cyg_mempool_fix_free(cyg_handle_t fixpool, void *p) __THROW;

/* Returns true if there are any threads waiting for memory in the
   given memory pool. */
cyg_bool_t cyg_mempool_fix_waiting(cyg_handle_t fixpool) __THROW;

/* Puts information about a variable memory pool into the structure
   provided. */
void cyg_mempool_fix_get_info(cyg_handle_t fixpool, cyg_mempool_info *info) __THROW;

/* user overrideable function invoked before running out of memory. */
__externC void cyg_memalloc_alloc_fail(char * file, int line, cyg_int32 size) 
     __THROW;

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/


#endif /* ifndef CYGONCE_MEMALLOC_KAPI_H */
/* EOF kapi.h */
