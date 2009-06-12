#ifndef CYGONCE_VAR_CACHE_H
#define CYGONCE_VAR_CACHE_H

//=============================================================================
//
//      var_cache.h
//
//      HAL cache control API
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   msalter
// Contributors: msalter
// Date:        2001-02-12
// Purpose:     Cache control API
// Description: The macros defined here provide the HAL APIs for handling
//              cache control operations.
// Usage:
//              #include <cyg/hal/var_cache.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/plf_cache.h>
#include <cyg/hal/var_arch.h>


//-----------------------------------------------------------------------------
// Cache dimensions

// Data cache
//#define HAL_DCACHE_SIZE                 0   // Size of data cache in bytes
//#define HAL_DCACHE_LINE_SIZE            0   // Size of a data cache line
//#define HAL_DCACHE_WAYS                 0   // Associativity of the cache

// Instruction cache
//#define HAL_ICACHE_SIZE                 0   // Size of cache in bytes
//#define HAL_ICACHE_LINE_SIZE            0   // Size of a cache line
//#define HAL_ICACHE_WAYS                 0   // Associativity of the cache

//#define HAL_DCACHE_SETS 0
//#define HAL_ICACHE_SETS 0

//#define HAL_DCACHE_WRITETHRU_MODE       1
//#define HAL_DCACHE_WRITEBACK_MODE       0

//-----------------------------------------------------------------------------
// Global control of data cache

// Invalidate the entire cache
#define HAL_DCACHE_INVALIDATE_ALL_DEFINED
#define HAL_DCACHE_INVALIDATE_ALL()

// Synchronize the contents of the cache with memory.
#define HAL_DCACHE_SYNC_DEFINED
#define HAL_DCACHE_SYNC()

// Set the data cache refill burst size
//#define HAL_DCACHE_BURST_SIZE(_asize_)

// Set the data cache write mode
//#define HAL_DCACHE_WRITE_MODE( _mode_ )

// Load the contents of the given address range into the data cache
// and then lock the cache so that it stays there.
#define HAL_DCACHE_LOCK_DEFINED
#define HAL_DCACHE_LOCK(_base_, _asize_)

// Undo a previous lock operation
#define HAL_DCACHE_UNLOCK_DEFINED
#define HAL_DCACHE_UNLOCK(_base_, _asize_)

// Unlock entire cache
#define HAL_DCACHE_UNLOCK_ALL_DEFINED
#define HAL_DCACHE_UNLOCK_ALL()


//-----------------------------------------------------------------------------
// Data cache line control

// Allocate cache lines for the given address range without reading its
// contents from memory.
//#define HAL_DCACHE_ALLOCATE( _base_ , _asize_ )

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
#define HAL_DCACHE_FLUSH_DEFINED
#define HAL_DCACHE_FLUSH( _base_ , _asize_ )


// Write dirty cache lines to memory for the given address range.
#define HAL_DCACHE_STORE_DEFINED
#define HAL_DCACHE_STORE( _base_ , _asize_ )

// Invalidate cache lines in the given range without writing to memory.
#define HAL_DCACHE_INVALIDATE_DEFINED
#define HAL_DCACHE_INVALIDATE( _base_ , _asize_ )


//-----------------------------------------------------------------------------
// Global control of Instruction cache

// Invalidate the entire cache
#define HAL_ICACHE_INVALIDATE_ALL_DEFINED
#define HAL_ICACHE_INVALIDATE_ALL()

// Synchronize the contents of the cache with memory.
#define HAL_ICACHE_SYNC_DEFINED
#define HAL_ICACHE_SYNC()

// Set the instruction cache refill burst size
//#define HAL_ICACHE_BURST_SIZE(_asize_)

// Load the contents of the given address range into the data cache
// and then lock the cache so that it stays there.
#define HAL_ICACHE_LOCK_DEFINED
#define HAL_ICACHE_LOCK(_base_, _asize_)

// Undo a previous lock operation
#define HAL_ICACHE_UNLOCK_DEFINED
#define HAL_ICACHE_UNLOCK(_base_, _asize_)

// Unlock entire cache
#define HAL_ICACHE_UNLOCK_ALL_DEFINED
#define HAL_ICACHE_UNLOCK_ALL()

//-----------------------------------------------------------------------------
// Instruction cache line control

// Invalidate cache lines in the given range without writing to memory.
#define HAL_ICACHE_INVALIDATE_DEFINED
#define HAL_ICACHE_INVALIDATE( _base_ , _asize_ )

//-----------------------------------------------------------------------------
// For the flash driver.

#define HAL_FLASH_CACHES_WANT_OPTIMAL

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_VAR_CACHE_H
// End of var_cache.h
