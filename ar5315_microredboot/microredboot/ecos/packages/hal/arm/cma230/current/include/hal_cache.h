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
// Author(s):   nickg, gthomas
// Contributors:        nickg, gthomas, msalter
// Date:        1998-09-28
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

#include <cyg/infra/cyg_type.h>

#ifdef __CMA222
//-----------------------------------------------------------------------------
// Cache dimensions

#define HAL_UCACHE_SIZE                 0x2000   // Size of data cache in bytes
#define HAL_UCACHE_LINE_SIZE            16       // Size of a data cache line
#define HAL_UCACHE_WAYS                 4        // Associativity of the cache
#define HAL_UCACHE_SETS (HAL_UCACHE_SIZE/(HAL_UCACHE_LINE_SIZE*HAL_UCACHE_WAYS))

#define HAL_CACHE_UNIFIED   // Let programs know the caches are combined

//-----------------------------------------------------------------------------
// Global control of caches

// Note: the 'mrc' doesn't seem to work.
#if 0
// Enable the data cache
//    mrc  MMU_CP,0,r1,MMU_Control,c0
//    orr  r1,r1,#MMU_Control_C|MMU_Control_B
//    mcr  MMU_CP,0,r1,MMU_Control,c0    

#define HAL_UCACHE_ENABLE()                     \
{                                               \
    asm volatile ("mrc  p15,0,r1,c1,c0;"        \
                  "orr  r1,r1,#0x000C;"         \
                  "mcr  p15,0,r1,c1,c0;"        \
                  :                             \
                  :                             \
                  : "r1" /* Clobber list */     \
        );                                      \
                                                \
}

// Disable the data cache
#define HAL_UCACHE_DISABLE()                     \
{                                               \
    asm volatile ("mrc  p15,0,r1,c1,c0;"        \
                  "bic  r1,r1,#0x000C;"         \
                  "mcr  p15,0,r1,c1,c0;"        \
                  :                             \
                  :                             \
                  : "r1" /* Clobber list */     \
        );                                      \
                                                \
}
#else
#define HAL_UCACHE_ENABLE()                     \
{                                               \
    asm volatile ("mov  r1,#0x7D;"              \
                  "mcr  p15,0,r1,c1,c0;"        \
                  :                             \
                  :                             \
                  : "r1" /* Clobber list */     \
        );                                      \
                                                \
}

// Disable the data cache
#define HAL_UCACHE_DISABLE()                    \
{                                               \
    asm volatile ("mov  r1,#0x71;"              \
                  "mcr  p15,0,r1,c1,c0;"        \
                  :                             \
                  :                             \
                  : "r1" /* Clobber list */     \
        );                                      \
                                                \
}
#endif

// Is the cache turned on?
#define HAL_UCACHE_IS_ENABLED(_state_) _state_ = 1;

// Invalidate the entire cache
//    mcr  MMU_CP,0,r1,MMU_InvalidateCache,c0
#define HAL_UCACHE_INVALIDATE_ALL()             \
{                                               \
    asm volatile (                              \
        "mov    r1,#0;"                         \
	"mcr p15,0,r1,c7,c0,0;"                 \
        :                                       \
        :                                       \
        : "r1","memory" /* Clobber list */      \
    );                                          \
}

// Synchronize the contents of the cache with memory.
#define HAL_UCACHE_SYNC()                                               \
{                                                                       \
    cyg_uint32 *ROM = (cyg_uint32 *)0xE0000000;                         \
    int i;                                                              \
    volatile cyg_uint32 val;                                            \
    for (i = 0;  i < HAL_UCACHE_SETS;  i++) {                           \
        val = *ROM;                                                     \
        ROM += HAL_UCACHE_LINE_SIZE;                                    \
    }                                                                   \
}

// Purge contents of data cache
#define HAL_UCACHE_PURGE_ALL()  HAL_UCACHE_INVALIDATE_ALL()

//-----------------------------------------------------------------------------
// Data cache line control

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
#define HAL_UCACHE_FLUSH( _base_ , _size_ )  HAL_UCACHE_SYNC()

// Write dirty cache lines to memory for the given address range.
#define HAL_UCACHE_STORE( _base_ , _size_ )  HAL_UCACHE_SYNC()

#endif // ifdef __CMA222

#ifdef HAL_CACHE_UNIFIED
//-----------------------------------------------------------------------------
// Global control of data cache

#define HAL_DCACHE_SIZE                 HAL_UCACHE_SIZE
#define HAL_DCACHE_LINE_SIZE            HAL_UCACHE_LINE_SIZE
#define HAL_DCACHE_WAYS                 HAL_UCACHE_WAYS
#define HAL_DCACHE_SETS                 HAL_UCACHE_SETS

// Enable the data cache
#define HAL_DCACHE_ENABLE()             HAL_UCACHE_ENABLE()

// Disable the data cache
#define HAL_DCACHE_DISABLE()            HAL_UCACHE_DISABLE()

// Invalidate the entire cache
#define HAL_DCACHE_INVALIDATE_ALL()     HAL_UCACHE_INVALIDATE_ALL()

// Synchronize the contents of the cache with memory.
#define HAL_DCACHE_SYNC()               HAL_UCACHE_SYNC()

// Query the state of the data cache
#define HAL_DCACHE_IS_ENABLED(_state_)  HAL_UCACHE_IS_ENABLED(_state_)

//-----------------------------------------------------------------------------
// Data cache line control

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
#define HAL_DCACHE_FLUSH( _base_ , _size_ )  HAL_UCACHE_FLUSH( _base_ , _size_ )

// Write dirty cache lines to memory for the given address range.
#define HAL_DCACHE_STORE( _base_ , _size_ )  HAL_UCACHE_STORE( _base_ , _size_ )

//-----------------------------------------------------------------------------
// Global control of Instruction cache - use Data cache controls since they
// are not separatable.

#define HAL_ICACHE_SIZE                 HAL_UCACHE_SIZE
#define HAL_ICACHE_LINE_SIZE            HAL_UCACHE_LINE_SIZE
#define HAL_ICACHE_WAYS                 HAL_UCACHE_WAYS
#define HAL_ICACHE_SETS                 HAL_UCACHE_SETS

// Enable the instruction cache
#define HAL_ICACHE_ENABLE()      HAL_UCACHE_ENABLE()

// Disable the instruction cache
#define HAL_ICACHE_DISABLE()     HAL_UCACHE_DISABLE()

// Invalidate the entire cache
#define HAL_ICACHE_INVALIDATE_ALL()  HAL_UCACHE_INVALIDATE_ALL()

// Synchronize the contents of the cache with memory.
#define HAL_ICACHE_SYNC()        HAL_UCACHE_SYNC()

#else
//-----------------------------------------------------------------------------
// Cache dimensions

// Data cache
//#define HAL_DCACHE_SIZE                 0    // Size of data cache in bytes
//#define HAL_DCACHE_LINE_SIZE            0    // Size of a data cache line
//#define HAL_DCACHE_WAYS                 0    // Associativity of the cache

// Instruction cache
//#define HAL_ICACHE_SIZE                 0    // Size of cache in bytes
//#define HAL_ICACHE_LINE_SIZE            0    // Size of a cache line
//#define HAL_ICACHE_WAYS                 0    // Associativity of the cache

//#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))
//#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))


//-----------------------------------------------------------------------------
// Global control of data cache

// Enable the data cache
#define HAL_DCACHE_ENABLE()

// Disable the data cache
#define HAL_DCACHE_DISABLE()

// Invalidate the entire cache
#define HAL_DCACHE_INVALIDATE_ALL()

// Synchronize the contents of the cache with memory.
#define HAL_DCACHE_SYNC()

// Purge contents of data cache
#define HAL_DCACHE_PURGE_ALL()

// Set the data cache refill burst size
//#define HAL_DCACHE_BURST_SIZE(_size_)

// Set the data cache write mode
//#define HAL_DCACHE_WRITE_MODE( _mode_ )

//#define HAL_DCACHE_WRITETHRU_MODE       0
//#define HAL_DCACHE_WRITEBACK_MODE       1

// Load the contents of the given address range into the data cache
// and then lock the cache so that it stays there.
//#define HAL_DCACHE_LOCK(_base_, _size_)

// Undo a previous lock operation
//#define HAL_DCACHE_UNLOCK(_base_, _size_)

// Unlock entire cache
//#define HAL_DCACHE_UNLOCK_ALL()

//-----------------------------------------------------------------------------
// Data cache line control

// Allocate cache lines for the given address range without reading its
// contents from memory.
//#define HAL_DCACHE_ALLOCATE( _base_ , _size_ )

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
//#define HAL_DCACHE_FLUSH( _base_ , _size_ )

// Invalidate cache lines in the given range without writing to memory.
//#define HAL_DCACHE_INVALIDATE( _base_ , _size_ )

// Write dirty cache lines to memory for the given address range.
//#define HAL_DCACHE_STORE( _base_ , _size_ )

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
#define HAL_ICACHE_ENABLE()

// Disable the instruction cache
#define HAL_ICACHE_DISABLE()

// Invalidate the entire cache
#define HAL_ICACHE_INVALIDATE_ALL()

// Synchronize the contents of the cache with memory.
#define HAL_ICACHE_SYNC()

// Set the instruction cache refill burst size
//#define HAL_ICACHE_BURST_SIZE(_size_)

// Load the contents of the given address range into the instruction cache
// and then lock the cache so that it stays there.
//#define HAL_ICACHE_LOCK(_base_, _size_)

// Undo a previous lock operation
//#define HAL_ICACHE_UNLOCK(_base_, _size_)

// Unlock entire cache
//#define HAL_ICACHE_UNLOCK_ALL()

//-----------------------------------------------------------------------------
// Instruction cache line control

// Invalidate cache lines in the given range without writing to memory.
//#define HAL_ICACHE_INVALIDATE( _base_ , _size_ )

//-----------------------------------------------------------------------------
#endif // ifndef HAL_CACHE_UNIFIED
#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
