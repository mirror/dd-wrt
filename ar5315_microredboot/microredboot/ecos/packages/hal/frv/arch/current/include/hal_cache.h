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
// Author(s):   gthomas
// Contributors:hmt
// Date:        2000-05-08
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
#include CYGBLD_HAL_PLF_DEFS_H        
#include <cyg/hal/plf_cache.h>      // Platform (model) details

//-----------------------------------------------------------------------------
// Global control of Instruction cache

// Enable the instruction cache
#define HAL_ICACHE_ENABLE()                     \
CYG_MACRO_START                                 \
    cyg_uint32 mask = _HSR0_ICE;                \
    asm volatile (                              \
        "movsg hsr0,gr4\n"                      \
        "\tor    gr4,%0,gr4\n"                  \
        "\tmovgs gr4,hsr0\n"                    \
        :                                       \
        : "r"(mask)                             \
        : "gr4" /* Clobber list */              \
        );                                      \
CYG_MACRO_END

// Disable the instruction cache (and invalidate it, required semanitcs)
#define HAL_ICACHE_DISABLE()                    \
CYG_MACRO_START                                 \
    cyg_uint32 mask = ~_HSR0_ICE;               \
    asm volatile (                              \
        "movsg hsr0,gr4\n"                      \
        "\tand   gr4,%0,gr4\n"                  \
        "\tmovgs gr4,hsr0\n"                    \
        :                                       \
        : "r"(mask)                             \
        : "gr4" /* Clobber list */              \
        );                                      \
CYG_MACRO_END

// Query the state of the instruction cache
#define HAL_ICACHE_IS_ENABLED(_state_)          \
CYG_MACRO_START                                 \
    register cyg_uint32 reg;                    \
    asm volatile ("movsg hsr0,%0"               \
                  : "=r"(reg)                   \
                  :                             \
        );                                      \
    (_state_) = (0 != (_HSR0_ICE & reg));       \
CYG_MACRO_END

// Invalidate the entire cache
#define HAL_ICACHE_INVALIDATE_ALL()             \
CYG_MACRO_START                                 \
    asm volatile ("icei @(gr4,gr0),1"           \
                  :                             \
                  :                             \
        );                                      \
CYG_MACRO_END

// Synchronize the contents of the cache with memory.
// (which includes flushing out pending writes)
#define HAL_ICACHE_SYNC()                                       \
CYG_MACRO_START                                                 \
    HAL_DCACHE_SYNC(); /* ensure data gets to RAM */            \
    HAL_ICACHE_INVALIDATE_ALL(); /* forget all we know */       \
CYG_MACRO_END

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
#define HAL_ICACHE_INVALIDATE( _base_ , _size_ )        \
CYG_MACRO_START                                         \
    cyg_uint32 _b = _base_;                             \
    cyg_uint32 _s = _size_;                             \
    while (_s > HAL_DCACHE_LINE_SIZE) {                 \
        asm volatile ("ici @(%0,gr0)"                   \
                      :                                 \
                      : "r"(_b)                         \
            );                                          \
        _s -= HAL_DCACHE_LINE_SIZE;                     \
        _b += HAL_DCACHE_LINE_SIZE;                     \
    }                                                   \
CYG_MACRO_END

//-----------------------------------------------------------------------------
// Global control of data cache

// Enable the data cache
#define HAL_DCACHE_ENABLE()                     \
CYG_MACRO_START                                 \
    cyg_uint32 mask = _HSR0_DCE;                \
    asm volatile (                              \
        "movsg hsr0,gr4\n"                      \
        "\tor    gr4,%0,gr4\n"                  \
        "\tmovgs gr4,hsr0\n"                    \
        :                                       \
        : "r"(mask)                             \
        : "gr4" /* Clobber list */              \
        );                                      \
CYG_MACRO_END

// Disable the data cache (and invalidate it, required semanitcs)
#define HAL_DCACHE_DISABLE()                    \
CYG_MACRO_START                                 \
    cyg_uint32 mask = ~_HSR0_DCE;               \
    asm volatile (                              \
        "movsg hsr0,gr4\n"                      \
        "\tand   gr4,%0,gr4\n"                  \
        "\tmovgs gr4,hsr0\n"                    \
        :                                       \
        : "r"(mask)                             \
        : "gr4" /* Clobber list */              \
        );                                      \
CYG_MACRO_END

// Query the state of the data cache
#define HAL_DCACHE_IS_ENABLED(_state_)          \
CYG_MACRO_START                                 \
    register cyg_uint32 reg;                    \
    asm volatile ("movsg hsr0,%0"               \
                  : "=r"(reg)                   \
                  :                             \
        );                                      \
    (_state_) = (0 != (_HSR0_DCE & reg));       \
CYG_MACRO_END

// Flush (invalidate) the entire dcache 
#define HAL_DCACHE_INVALIDATE_ALL()             \
CYG_MACRO_START                                 \
    asm volatile ("dcei @(gr4,gr0),1"           \
                  :                             \
                  :                             \
        );                                      \
CYG_MACRO_END

// Synchronize the contents of the cache with memory.
#define HAL_DCACHE_SYNC()                       \
CYG_MACRO_START                                 \
    asm volatile ("dcef @(gr4,gr0),1"           \
                  :                             \
                  :                             \
        );                                      \
CYG_MACRO_END

// Set the data cache refill burst size
//#define HAL_DCACHE_BURST_SIZE(_size_)

// Set the data cache write mode
//#define HAL_DCACHE_WRITE_MODE( _mode_ )

#define HAL_DCACHE_WRITETHRU_MODE       0
#define HAL_DCACHE_WRITEBACK_MODE       1

// Get the current writeback mode - or only writeback mode if fixed
#define HAL_DCACHE_QUERY_WRITE_MODE( _mode_ ) CYG_MACRO_START           \
    _mode_ = HAL_DCACHE_WRITETHRU_MODE;                                 \
CYG_MACRO_END

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
#define HAL_DCACHE_FLUSH( _base_ , _size_ )     \
CYG_MACRO_START                                 \
    HAL_DCACHE_STORE( _base_ , _size_ );        \
    HAL_DCACHE_INVALIDATE( _base_ , _size_ );   \
CYG_MACRO_END

// Invalidate cache lines in the given range without writing to memory.
#define HAL_DCACHE_INVALIDATE( _base_ , _size_ )        \
CYG_MACRO_START                                         \
    cyg_uint32 _b = _base_;                             \
    cyg_uint32 _s = _size_;                             \
    while (_s > HAL_DCACHE_LINE_SIZE) {                 \
        asm volatile ("dci @(%0,gr0)"                   \
                      :                                 \
                      : "r"(_b)                         \
            );                                          \
        _s -= HAL_DCACHE_LINE_SIZE;                     \
        _b += HAL_DCACHE_LINE_SIZE;                     \
    }                                                   \
CYG_MACRO_END
                          
// Write dirty cache lines to memory for the given address range.
#define HAL_DCACHE_STORE( _base_ , _size_ )     \
CYG_MACRO_START                                 \
    cyg_uint32 _b = _base_;                     \
    cyg_uint32 _s = _size_;                     \
    while (_s > HAL_DCACHE_LINE_SIZE) {         \
        asm volatile ("dcf @(%0,gr0)"           \
                      :                         \
                      : "r"(_b)                 \
            );                                  \
        _s -= HAL_DCACHE_LINE_SIZE;             \
        _b += HAL_DCACHE_LINE_SIZE;             \
    }                                           \
CYG_MACRO_END

// Preread the given range into the cache with the intention of reading
// from it later.
//#define HAL_DCACHE_READ_HINT( _base_ , _size_ )

// Preread the given range into the cache with the intention of writing
// to it later.
//#define HAL_DCACHE_WRITE_HINT( _base_ , _size_ )

// Allocate and zero the cache lines associated with the given range.
//#define HAL_DCACHE_ZERO( _base_ , _size_ )

//-----------------------------------------------------------------------------

#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
