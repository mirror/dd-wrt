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
// Author(s):   nickg
// Contributors:        nickg, dmoseley
// Date:        1998-02-17
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

//#include <cyg/hal/plf_cache.h>

//=============================================================================
// AM33 implementation

//-----------------------------------------------------------------------------
// Cache dimensions


// Data cache
#define HAL_DCACHE_SIZE                 4096    // Size of data cache in bytes
#define HAL_DCACHE_LINE_SIZE            16      // Size of a data cache line
#define HAL_DCACHE_WAYS                 4       // Associativity of the cache

// Instruction cache
#define HAL_ICACHE_SIZE                 8192    // Size of cache in bytes
#define HAL_ICACHE_LINE_SIZE            16      // Size of a cache line
#define HAL_ICACHE_WAYS                 4       // Associativity of the cache

#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))
#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))

//-----------------------------------------------------------------------------
// Control registers

#define HAL_CHCTR               ((volatile CYG_ADDRWORD *)0xC0000070)

#define HAL_CHCTR_ICEN          0x0001
#define HAL_CHCTR_DCEN          0x0002
#define HAL_CHCTR_ICBUSY        0x0004
#define HAL_CHCTR_DCBUSY        0x0008
#define HAL_CHCTR_ICINV         0x0010
#define HAL_CHCTR_DCINV         0x0020
#define HAL_CHCTR_DCWTMD        0x0040
#define HAL_CHCTR_ICWMD         0x0300
#define HAL_CHCTR_DCWMD         0x3000

#define HAL_DCACHE_PURGE_WAY0   ((volatile CYG_BYTE *)0xC8400000)
#define HAL_DCACHE_PURGE_WAY1   ((volatile CYG_BYTE *)0xC8401000)
#define HAL_DCACHE_PURGE_WAY2   ((volatile CYG_BYTE *)0xC8402000)
#define HAL_DCACHE_PURGE_WAY3   ((volatile CYG_BYTE *)0xC8403000)

//-----------------------------------------------------------------------------
// Global control of data cache

// Enable the data cache
#define HAL_DCACHE_ENABLE()                     \
{                                               \
    register CYG_ADDRWORD chctr = *HAL_CHCTR;   \
    chctr |= HAL_CHCTR_DCEN;                    \
    *HAL_CHCTR = chctr;                         \
}

// Disable the data cache
#define HAL_DCACHE_DISABLE()                    \
{                                               \
    register CYG_ADDRWORD chctr = *HAL_CHCTR;   \
    chctr &= ~HAL_CHCTR_DCEN;                   \
    *HAL_CHCTR = chctr;                         \
    while( HAL_CHCTR_DCBUSY & *HAL_CHCTR );     \
}

// Query the state of the data cache
#define HAL_DCACHE_IS_ENABLED(_state_)          \
{                                               \
    register CYG_ADDRWORD chctr = *HAL_CHCTR;   \
    _state_ = (0 != (chctr & HAL_CHCTR_DCEN));  \
}

// Invalidate the entire cache
#define HAL_DCACHE_INVALIDATE_ALL()             \
{                                               \
    register CYG_ADDRWORD chctr;                \
    register CYG_ADDRWORD state;                \
    HAL_DCACHE_IS_ENABLED(state);               \
    if (state)                                  \
        HAL_DCACHE_DISABLE();                   \
    chctr = *HAL_CHCTR;                         \
    chctr |= HAL_CHCTR_DCINV;                   \
    *HAL_CHCTR = chctr;                         \
    while( HAL_CHCTR_DCBUSY & *HAL_CHCTR );     \
    if (state)                                  \
        HAL_DCACHE_ENABLE();                    \
}

// Synchronize the contents of the cache with memory.
#define HAL_DCACHE_SYNC() HAL_DCACHE_STORE( 0, HAL_DCACHE_SIZE )

// Set the data cache refill burst size
//#define HAL_DCACHE_BURST_SIZE(_size_)

// Set the data cache write mode
#define HAL_DCACHE_WRITE_MODE( _mode_ )         \
{                                               \
    register CYG_ADDRWORD chctr;                \
    register CYG_ADDRWORD state;                \
    HAL_DCACHE_IS_ENABLED(state);               \
    if (state)                                  \
        HAL_DCACHE_DISABLE();                   \
    chctr = *HAL_CHCTR;                         \
    chctr &= ~HAL_CHCTR_DCWTMD;                 \
    chctr |= HAL_CHCTR_DCWTMD*(_mode_);         \
    *HAL_CHCTR = chctr;                         \
    while( HAL_CHCTR_DCBUSY & *HAL_CHCTR );     \
    if (state)                                  \
        HAL_DCACHE_ENABLE();                    \
}

#define HAL_DCACHE_WRITEBACK_MODE       0
#define HAL_DCACHE_WRITETHRU_MODE       1

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

// This functionality requires 4 register variables. To prevent register
// spilling, put the code in a separate function.
externC void cyg_hal_dcache_store(CYG_ADDRWORD base, int size);

#define HAL_DCACHE_STORE( _base_ , _size_ ) \
    cyg_hal_dcache_store((CYG_ADDRWORD)(_base_), (_size_))

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
#define HAL_ICACHE_ENABLE()                     \
{                                               \
    register CYG_ADDRWORD chctr = *HAL_CHCTR;   \
    chctr |= HAL_CHCTR_ICEN;                    \
    *HAL_CHCTR = chctr;                         \
}

// Disable the instruction cache
#define HAL_ICACHE_DISABLE()                    \
{                                               \
    register CYG_ADDRWORD chctr = *HAL_CHCTR;   \
    chctr &= ~HAL_CHCTR_ICEN;                   \
    *HAL_CHCTR = chctr;                         \
    while( HAL_CHCTR_ICBUSY & *HAL_CHCTR );     \
}

// Query the state of the instruction cache
#define HAL_ICACHE_IS_ENABLED(_state_)          \
{                                               \
    register CYG_ADDRWORD chctr = *HAL_CHCTR;   \
    _state_ = (0 != (chctr & HAL_CHCTR_ICEN));  \
}

// Invalidate the entire cache
#define HAL_ICACHE_INVALIDATE_ALL()             \
{                                               \
    register CYG_ADDRWORD chctr;                \
    register CYG_ADDRWORD state;                \
    HAL_ICACHE_IS_ENABLED(state);               \
    if (state)                                  \
        HAL_ICACHE_DISABLE();                   \
    chctr = *HAL_CHCTR;                         \
    chctr |= HAL_CHCTR_ICINV;                   \
    *HAL_CHCTR = chctr;                         \
    while( HAL_CHCTR_ICBUSY & *HAL_CHCTR );     \
    if (state)                                  \
        HAL_ICACHE_ENABLE();                    \
}

// Synchronize the contents of the cache with memory.
#define HAL_ICACHE_SYNC() HAL_ICACHE_INVALIDATE_ALL()

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
// flash caching control
#ifdef CYGSEM_HAL_UNCACHED_FLASH_ACCESS
#define HAL_FLASH_CACHES_OFF(_d_, _i_)		\
    CYG_MACRO_START                             \
    _d_ = 0; /* avoids warning */               \
    _i_ = 0; /* avoids warning */               \
    CYG_MACRO_END

#define HAL_FLASH_CACHES_ON(_d_, _i_)           \
    CYG_MACRO_START                             \
    CYG_MACRO_END
#endif

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_VAR_CACHE_H
// End of var_cache.h
