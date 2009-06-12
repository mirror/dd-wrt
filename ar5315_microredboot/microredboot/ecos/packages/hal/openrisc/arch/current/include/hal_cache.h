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
// Author(s):   Scott Furman
// Contributors:
// Date:        2003-02-08
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

//-----------------------------------------------------------------------------
// Cache dimensions.
// These really should be defined in var_cache.h. If they are not, then provide
// a set of numbers that are typical of many variants.

#ifndef HAL_DCACHE_SIZE

// Data cache
#define HAL_DCACHE_SIZE                 4096    // Size of data cache in bytes
#define HAL_DCACHE_LINE_SIZE            16      // Bytes in a data cache line
#define HAL_DCACHE_WAYS                 1       // Associativity of the cache

// Instruction cache
#define HAL_ICACHE_SIZE                 4096    // Size of cache in bytes
#define HAL_ICACHE_LINE_SIZE            16      // Bytes in a cache line
#define HAL_ICACHE_WAYS                 1       // Associativity of the cache

#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))
#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))

#endif

#ifndef __ASSEMBLER__

#include <cyg/hal/hal_arch.h>

//-----------------------------------------------------------------------------
// Global control of data cache

// Enable the data cache
#define HAL_DCACHE_ENABLE() MTSPR(SPR_SR, MFSPR(SPR_SR) | SPR_SR_DCE)

// Disable the data cache
#define HAL_DCACHE_DISABLE() MTSPR(SPR_SR, MFSPR(SPR_SR) & ~SPR_SR_DCE)

// Enable or disable the data cache, depending on argument, which is required
// to be 0 or 1.
#define HAL_SET_DCACHE_ENABLED(enable)                          \
    MTSPR(SPR_SR, MFSPR(SPR_SR) | (SPR_SR_DCE & -(enable)))

// Invalidate the entire data cache
#define HAL_DCACHE_INVALIDATE_ALL()                             \
    CYG_MACRO_START                                             \
    int cache_enabled, addr;                                    \
                                                                \
    /* Save current cache mode (disabled/enabled) */            \
    HAL_DCACHE_IS_ENABLED(cache_enabled);                       \
                                                                \
    /* Disable cache, so that invalidation ignores cache tags */\
    HAL_DCACHE_DISABLE();                                       \
    addr = HAL_DCACHE_SIZE;                                     \
    do {                                                        \
        MTSPR(SPR_DCBIR, addr);                                 \
        addr -= HAL_DCACHE_LINE_SIZE;                           \
    } while (addr > 0);                                         \
                                                                \
    /* Re-enable cache if it was enabled on entry */            \
    HAL_SET_DCACHE_ENABLED(cache_enabled);                      \
    CYG_MACRO_END

// Synchronize the contents of the cache with memory.
// (Unnecessary on OR12K, since cache is write-through.)
#define HAL_DCACHE_SYNC()                       \
    CYG_MACRO_START                             \
    CYG_MACRO_END

// Query the state (enabled/disabled) of the data cache
#define HAL_DCACHE_IS_ENABLED(_state_)                          \
    CYG_MACRO_START                                             \
    (_state_) = (1 - !(MFSPR(SPR_SR) & SPR_SR_DCE));            \
    CYG_MACRO_END

// Load the contents of the given address range into the data cache 
// and then lock the cache so that it stays there.

// The OpenRISC architecture defines these operations, but no
// implementation supports them yet.

//#define HAL_DCACHE_LOCK(_base_, _size_)
        
// Undo a previous lock operation
//#define HAL_DCACHE_UNLOCK(_base_, _size_)

// Unlock entire cache
//#define HAL_DCACHE_UNLOCK_ALL()


//-----------------------------------------------------------------------------
// Data cache line control

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
// OR12k has write-through cache, so no flushing of writes to memory
// are necessary.
#define HAL_DCACHE_FLUSH( _base_ , _size_ )                          \
    HAL_DCACHE_INVALIDATE(_base_, _size_)
   
// Invalidate cache lines in the given range without writing to memory.
#define HAL_DCACHE_INVALIDATE( _base_ , _size_ )                     \
    CYG_MACRO_START                                                  \
    int addr;                                                        \
    int end = _base_ + _size_;                                       \
    for (addr = end; addr >= _base_; addr -= HAL_DCACHE_LINE_SIZE) { \
        MTSPR(SPR_DCBIR, addr);                                      \
    }                                                                \
    CYG_MACRO_END

// Write dirty cache lines to memory for the given address range.
// OR12k has write-through cache, so this is a NOP
#define HAL_DCACHE_STORE( _base_ , _size_ )

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
#define HAL_ICACHE_ENABLE() MTSPR(SPR_SR, MFSPR(SPR_SR) | SPR_SR_ICE)

// Disable the instruction cache
#define HAL_ICACHE_DISABLE() MTSPR(SPR_SR, MFSPR(SPR_SR) & ~SPR_SR_ICE)

// Enable or disable the data cache, depending on argument, which must
// be 0 or 1.
#define HAL_SET_ICACHE_ENABLED(enable)                          \
    MTSPR(SPR_SR, MFSPR(SPR_SR) | (SPR_SR_ICE & -(enable)))

// Invalidate the entire instruction cache
#define HAL_ICACHE_INVALIDATE_ALL()                             \
    CYG_MACRO_START                                             \
    int cache_enabled, addr;                                    \
                                                                \
    /* Save current cache mode (disabled/enabled) */            \
    HAL_ICACHE_IS_ENABLED(cache_enabled);                       \
                                                                \
    /* Disable cache, so that invalidation ignores cache tags */\
    HAL_ICACHE_DISABLE();                                       \
    addr = HAL_ICACHE_SIZE;                                     \
    do {                                                        \
        MTSPR(SPR_ICBIR, addr);                                 \
        addr -= HAL_ICACHE_LINE_SIZE;                           \
    } while (addr > 0);                                         \
                                                                \
    /* Re-enable cache if it was enabled on entry */            \
    HAL_SET_ICACHE_ENABLED(cache_enabled);                      \
    CYG_MACRO_END

// Synchronize the contents of the cache with memory.
#define HAL_ICACHE_SYNC() HAL_ICACHE_INVALIDATE_ALL()

// Query the state of the instruction cache
#define HAL_ICACHE_IS_ENABLED(_state_)                          \
    CYG_MACRO_START                                             \
    (_state_) = (1 - !(MFSPR(SPR_SR) & SPR_SR_ICE));            \
    CYG_MACRO_END


// Load the contents of the given address range into the instruction cache
// and then lock the cache so that it stays there.

// The OpenRISC architecture defines these operations, but no
// implementation supports them yet.

//#define HAL_ICACHE_LOCK(_base_, _size_)

// Undo a previous lock operation
//#define HAL_ICACHE_UNLOCK(_base_, _size_)

// Unlock entire cache
//#define HAL_ICACHE_UNLOCK_ALL()

#endif /* __ASSEMBLER__ */

#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
