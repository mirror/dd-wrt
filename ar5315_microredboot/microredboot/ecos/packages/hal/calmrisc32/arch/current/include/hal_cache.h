#ifndef CYGONCE_HAL_CACHE_H
#define CYGONCE_HAL_CACHE_H

//=============================================================================
//
//      hal_cache.h
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1998-02-17
// Purpose:     Cache control API
// Description: The macros defined here provide the HAL APIs for handling
//              cache control operations.
// Usage:
//              #include <cyg/hal/hal_cache.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/var_cache.h>


//-----------------------------------------------------------------------------
// Cache dimensions.
// These really should be defined in var_cache.h. If they are not, then provide
// a set of numbers that are typical of many variants.

#ifndef HAL_DCACHE_SIZE

// Data cache
//#define HAL_DCACHE_SIZE                 0     // Size of data cache in bytes
//#define HAL_DCACHE_LINE_SIZE            0     // Size of a data cache line
//#define HAL_DCACHE_WAYS                 0     // Associativity of the cache

// Instruction cache
//#define HAL_ICACHE_SIZE                 0     // Size of cache in bytes
//#define HAL_ICACHE_LINE_SIZE            0     // Size of a cache line
//#define HAL_ICACHE_WAYS                 0     // Associativity of the cache

//#define HAL_DCACHE_SETS 0
//#define HAL_ICACHE_SETS 0

#endif

//-----------------------------------------------------------------------------
// Global control of data cache

// Enable the data cache
// There is no default mechanism for enabling or disabling the caches.
#ifndef HAL_DCACHE_ENABLE_DEFINED
#define HAL_DCACHE_ENABLE()
#endif

// Disable the data cache
#ifndef HAL_DCACHE_DISABLE_DEFINED
#define HAL_DCACHE_DISABLE()
#endif

#ifndef HAL_DCACHE_IS_ENABLED_DEFINED
#define HAL_DCACHE_IS_ENABLED(_state_) (_state_) = 1;
#endif

// Invalidate the entire cache
// We simply use HAL_DCACHE_SYNC() to do this. For writeback caches this
// is not quite what we want, but there is no index-invalidate operation
// available.
#ifndef HAL_DCACHE_INVALIDATE_ALL_DEFINED
#define HAL_DCACHE_INVALIDATE_ALL() HAL_DCACHE_SYNC()
#endif

// Synchronize the contents of the cache with memory.
// This uses the index-writeback-invalidate operation.
#ifndef HAL_DCACHE_SYNC_DEFINED
#define HAL_DCACHE_SYNC()                                               \
    CYG_MACRO_START                                                     \
    CYG_MACRO_END
#endif

// Set the data cache refill burst size
//#define HAL_DCACHE_BURST_SIZE(_size_)

// Set the data cache write mode
//#define HAL_DCACHE_WRITE_MODE( _mode_ )

//#define HAL_DCACHE_WRITETHRU_MODE       0
//#define HAL_DCACHE_WRITEBACK_MODE       1

// Load the contents of the given address range into the data cache
// and then lock the cache so that it stays there.
// This uses the fetch-and-lock cache operation.
#ifndef HAL_DCACHE_LOCK_DEFINED
#define HAL_DCACHE_LOCK(_base_, _asize_)                                  \
    CYG_MACRO_START                                                       \
    CYG_MACRO_END
#endif

// Undo a previous lock operation.
// Do this by flushing the cache, which is defined to clear the lock bit.
#ifndef HAL_DCACHE_UNLOCK_DEFINED
#define HAL_DCACHE_UNLOCK(_base_, _size_) \
        HAL_DCACHE_FLUSH( _base_, _size_ )
#endif

// Unlock entire cache
#ifndef HAL_DCACHE_UNLOCK_ALL_DEFINED
#define HAL_DCACHE_UNLOCK_ALL() \
        HAL_DCACHE_INVALIDATE_ALL()
#endif

//-----------------------------------------------------------------------------
// Data cache line control

// Allocate cache lines for the given address range without reading its
// contents from memory.
//#define HAL_DCACHE_ALLOCATE( _base_ , _size_ )

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
// This uses the hit-writeback-invalidate cache operation.
#ifndef HAL_DCACHE_FLUSH_DEFINED
#define HAL_DCACHE_FLUSH( _base_ , _asize_ )                              \
    CYG_MACRO_START                                                       \
    CYG_MACRO_END
#endif

// Invalidate cache lines in the given range without writing to memory.
// This uses the hit-invalidate cache operation.
#ifndef HAL_DCACHE_INVALIDATE_DEFINED
#define HAL_DCACHE_INVALIDATE( _base_ , _asize_ )                       \
    CYG_MACRO_START                                                     \
    CYG_MACRO_END
#endif

// Write dirty cache lines to memory for the given address range.
// This uses the hit-writeback cache operation.
#ifndef HAL_DCACHE_STORE_DEFINED
#define HAL_DCACHE_STORE( _base_ , _asize_ )                              \
    CYG_MACRO_START                                                       \
    CYG_MACRO_END
#endif

// Preread the given range into the cache with the intention of reading
// from it later.
//#define HAL_DCACHE_READ_HINT( _base_ , _size_ )

// Preread the given range into the cache with the intention of writing
// to it later.
//#define HAL_DCACHE_WRITE_HINT( _base_ , _size_ )

// Allocate and zero the cache lines associated with the given range.
//#define HAL_DCACHE_ZERO( _base_ , _size_ )

//-----------------------------------------------------------------------------
// Global control of Instruction cache

// Enable the instruction cache
// There is no default mechanism for enabling or disabling the caches.
#ifndef HAL_ICACHE_ENABLE_DEFINED
#define HAL_ICACHE_ENABLE()
#endif

// Disable the instruction cache
#ifndef HAL_ICACHE_DISABLE_DEFINED
#define HAL_ICACHE_DISABLE()
#endif

#ifndef HAL_ICACHE_IS_ENABLED_DEFINED
#define HAL_ICACHE_IS_ENABLED(_state_) (_state_) = 1;
#endif

// Invalidate the entire cache
// This uses the index-invalidate cache operation.
#ifndef HAL_ICACHE_INVALIDATE_ALL_DEFINED
#define HAL_ICACHE_INVALIDATE_ALL()                                           \
    CYG_MACRO_START                                                           \
    CYG_MACRO_END
#endif

// Synchronize the contents of the cache with memory.
// Simply force the cache to reload.
#ifndef HAL_ICACHE_SYNC_DEFINED
#define HAL_ICACHE_SYNC() HAL_ICACHE_INVALIDATE_ALL()
#endif

// Set the instruction cache refill burst size
//#define HAL_ICACHE_BURST_SIZE(_size_)

// Load the contents of the given address range into the instruction cache
// and then lock the cache so that it stays there.
// This uses the fetch-and-lock cache operation.
#ifndef HAL_ICACHE_LOCK_DEFINED
#define HAL_ICACHE_LOCK(_base_, _asize_)                                  \
    CYG_MACRO_START                                                       \
    CYG_MACRO_END
#endif

// Undo a previous lock operation.
// Do this by invalidating the cache, which is defined to clear the lock bit.
#ifndef HAL_ICACHE_UNLOCK_DEFINED
#define HAL_ICACHE_UNLOCK(_base_, _size_) \
        HAL_ICACHE_INVALIDATE( _base_, _size_ )
#endif

// Unlock entire cache
//#define HAL_ICACHE_UNLOCK_ALL()

//-----------------------------------------------------------------------------
// Instruction cache line control

// Invalidate cache lines in the given range without writing to memory.
// This uses the hit-invalidate cache operation.
#ifndef HAL_ICACHE_INVALIDATE_DEFINED
#define HAL_ICACHE_INVALIDATE( _base_ , _asize_ )                       \
    CYG_MACRO_START                                                     \
    CYG_MACRO_END
#endif


//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
