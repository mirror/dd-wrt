#ifndef CYGONCE_VAR_CACHE_H
#define CYGONCE_VAR_CACHE_H

//=============================================================================
//
//      var_cache.h
//
//      HAL variant cache control API
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
// Author(s):   jskov
// Contributors:jskov
// Date:        1999-04-23
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

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_io.h>             // HAL_READ/WRITE macros

#include <cyg/hal/plf_cache.h>          // Platform cache definitions

#include CYGBLD_HAL_CPU_MODULES_H       // cache module specs

#include <cyg/hal/sh_regs.h>            // CYGARC_REG_ definitions

//-----------------------------------------------------------------------------
// Cache dimensions - one unified cache

#define HAL_CACHE_UNIFIED

#define HAL_UCACHE_SIZE                 CYGARC_SH_MOD_CAC_SIZE
#define HAL_UCACHE_LINE_SIZE            CYGARC_SH_MOD_CAC_LINE_SIZE
#define HAL_UCACHE_WAYS                 CYGARC_SH_MOD_CAC_WAYS

// Cache addressing information
#define CYGARC_REG_CACHE_ADDRESS_BASE   CYGARC_SH_MOD_CAC_ADDRESS_BASE
#define CYGARC_REG_CACHE_ADDRESS_TOP    CYGARC_SH_MOD_CAC_ADDRESS_TOP
#define CYGARC_REG_CACHE_ADDRESS_STEP   CYGARC_SH_MOD_CAC_ADDRESS_STEP

// Writing this to a cache address entry forces a flush of the line if
// it is dirty.
#define CYGARC_REG_CACHE_ADDRESS_FLUSH  CYGARC_SH_MOD_CAC_ADDRESS_FLUSH

#define HAL_UCACHE_SETS (HAL_UCACHE_SIZE/(HAL_UCACHE_LINE_SIZE*HAL_UCACHE_WAYS))

//-----------------------------------------------------------------------------
// Global control of cache

// This is all handled in assembly (see variant.S) due to a requirement about
// not fiddling the cache from cachable memory.

externC void cyg_hal_cache_enable(void);
externC void cyg_hal_cache_disable(void);
externC void cyg_hal_cache_invalidate_all(void);
externC void cyg_hal_cache_sync(void);
externC void cyg_hal_cache_sync_region(cyg_haladdress base, 
                                       cyg_haladdrword len);
externC void cyg_hal_cache_write_mode(int mode);

// Enable the cache
#define HAL_UCACHE_ENABLE() cyg_hal_cache_enable()

// Disable the cache
#define HAL_UCACHE_DISABLE() cyg_hal_cache_disable()

// Invalidate the entire cache
#define HAL_UCACHE_INVALIDATE_ALL() cyg_hal_cache_invalidate_all()

// Synchronize the contents of the cache with memory.
#define HAL_UCACHE_SYNC() cyg_hal_cache_sync()

// Query the state of the cache (does not affect the caching)
#define HAL_UCACHE_IS_ENABLED(_state_)                  \
    CYG_MACRO_START                                     \
    cyg_uint32 _tmp;                                    \
    HAL_READ_UINT32(CYGARC_REG_CCR, _tmp);              \
    (_state_) = (_tmp & CYGARC_REG_CCR_CE) ? 1 : 0;     \
    CYG_MACRO_END

// Set the cache refill burst size
//#define HAL_UCACHE_BURST_SIZE(_size_)

// Set the cache write mode
#define HAL_UCACHE_WRITE_MODE( _mode_ )         \
    CYG_MACRO_START                             \
    cyg_uint32 _m_;                             \
    if (HAL_UCACHE_WRITETHRU_MODE == _mode_)    \
      _m_ = CYGARC_REG_CCR_WT;                  \
    else                                        \
      _m_ = CYGARC_REG_CCR_CB;                  \
    cyg_hal_cache_write_mode(_m_);              \
    CYG_MACRO_END

#define HAL_UCACHE_WRITETHRU_MODE       0
#define HAL_UCACHE_WRITEBACK_MODE       1

// This macro allows the client to specify separate modes for the two
// regions.
#define HAL_UCACHE_WRITE_MODE_SH( _mode_ ) cyg_hal_cache_write_mode(_mode_)

// Load the contents of the given address range into the cache
// and then lock the cache so that it stays there.
//#define HAL_UCACHE_LOCK(_base_, _size_)
        
// Undo a previous lock operation
//#define HAL_UCACHE_UNLOCK(_base_, _size_)

// Unlock entire cache
//#define HAL_UCACHE_UNLOCK_ALL()

//-----------------------------------------------------------------------------
// Cache line control

// Allocate cache lines for the given address range without reading its
// contents from memory.
//#define HAL_UCACHE_ALLOCATE( _base_ , _size_ )

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
#define HAL_UCACHE_FLUSH( _base_ , _size_ ) \
 cyg_hal_cache_sync_region((cyg_haladdress) _base_, (cyg_haladdrword)_size_)

// Invalidate cache lines in the given range without writing to memory.
//#define HAL_UCACHE_INVALIDATE( _base_ , _size_ )

// Write dirty cache lines to memory for the given address range.
//#define HAL_UCACHE_STORE( _base_ , _size_ )


// Preread the given range into the cache with the intention of reading
// from it later.
//#define HAL_UCACHE_READ_HINT( _base_ , _size_ )

// Preread the given range into the cache with the intention of writing
// to it later.
//#define HAL_UCACHE_WRITE_HINT( _base_ , _size_ )

// Allocate and zero the cache lines associated with the given range.
//#define HAL_UCACHE_ZERO( _base_ , _size_ )


//-----------------------------------------------------------------------------
// Data and instruction cache macros map onto the both-cache macros

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
#define HAL_DCACHE_FLUSH( _base_ , _size_ ) HAL_UCACHE_FLUSH( _base_, _size_ )

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

#define HAL_ICACHE_SIZE                 HAL_UCACHE_SIZE
#define HAL_ICACHE_LINE_SIZE            HAL_UCACHE_LINE_SIZE
#define HAL_ICACHE_WAYS                 HAL_UCACHE_WAYS
#define HAL_ICACHE_SETS                 HAL_UCACHE_SETS

// Enable the instruction cache
#define HAL_ICACHE_ENABLE()             HAL_UCACHE_ENABLE()

// Disable the instruction cache
#define HAL_ICACHE_DISABLE()            HAL_UCACHE_DISABLE()

// Invalidate the entire cache
#define HAL_ICACHE_INVALIDATE_ALL()     HAL_UCACHE_INVALIDATE_ALL()


// Synchronize the contents of the cache with memory.
#define HAL_ICACHE_SYNC()               HAL_UCACHE_SYNC()

// Query the state of the instruction cache
#define HAL_ICACHE_IS_ENABLED(_state_)  HAL_UCACHE_IS_ENABLED(_state_)

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
#endif // ifndef CYGONCE_VAR_CACHE_H
// End of var_cache.h
