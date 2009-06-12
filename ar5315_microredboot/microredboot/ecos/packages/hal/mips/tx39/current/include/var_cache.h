#ifndef CYGONCE_IMP_CACHE_H
#define CYGONCE_IMP_CACHE_H

//=============================================================================
//
//      imp_cache.h
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
//              #include <cyg/hal/imp_cache.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/plf_cache.h>

//=============================================================================
// Toshiba TX3904

#ifdef CYGPKG_HAL_MIPS_TX3904

//-----------------------------------------------------------------------------
// Cache dimensions

// Data cache
#define HAL_DCACHE_SIZE                 1024    // Size of data cache in bytes
#define HAL_DCACHE_LINE_SIZE            4       // Size of a data cache line
#define HAL_DCACHE_WAYS                 2       // Associativity of the cache

// Instruction cache
#define HAL_ICACHE_SIZE                 4096    // Size of cache in bytes
#define HAL_ICACHE_LINE_SIZE            16      // Size of a cache line
#define HAL_ICACHE_WAYS                 1       // Associativity of the cache

#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))
#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))

//-----------------------------------------------------------------------------
// Global control of data cache

// Enable the data cache
// This uses a bit in the config register, which is TX39 specific.
#define HAL_DCACHE_ENABLE_DEFINED
#define HAL_DCACHE_ENABLE()                     \
{                                               \
    asm volatile ("mfc0 $2,$3;"                 \
                  "ori  $2,$2,0x0010;"          \
                  "mtc0 $2,$3;"                 \
                  :                             \
                  :                             \
                  : "$2"                        \
                 );                             \
                                                \
}

// Disable the data cache
#define HAL_DCACHE_DISABLE_DEFINED
#define HAL_DCACHE_DISABLE()                    \
{                                               \
    asm volatile ("mfc0 $2,$3;"                 \
                  "la   $3,0xFFFFFFEF;"         \
                  "and  $2,$2,$3;"              \
                  "mtc0 $2,$3;"                 \
                  :                             \
                  :                             \
                  : "$2", "$3"                  \
                 );                             \
                                                \
}


// Invalidate the entire cache
// The TX39 only has hit-invalidate on the DCACHE, not
// index-invalidate, so we cannot just empty the cache out without
// knowing what is in it. This is annoying. So, the best we can do is
// fill the cache with data that is unlikely to be there
// otherwise. Hence we read bytes from the ROM space since this is
// most likely to be code, and will not get out of sync even if it is not.
#define HAL_DCACHE_INVALIDATE_ALL_DEFINED
#define HAL_DCACHE_INVALIDATE_ALL()                                     \
{                                                                       \
    volatile CYG_BYTE *addr = (CYG_BYTE *)(0x9fc00000);                 \
    volatile CYG_BYTE tmp = 0;                                          \
    int i;                                                              \
    for( i = 0; i < (HAL_DCACHE_SIZE*2); i += HAL_DCACHE_LINE_SIZE )    \
    {                                                                   \
        tmp = addr[i];                                                  \
    }                                                                   \
}

// Synchronize the contents of the cache with memory.
#define HAL_DCACHE_SYNC_DEFINED
#define HAL_DCACHE_SYNC() HAL_DCACHE_INVALIDATE_ALL()

// Set the data cache refill burst size
//#define HAL_DCACHE_BURST_SIZE(_size_)

// Set the data cache write mode
//#define HAL_DCACHE_WRITE_MODE( _mode_ )

//#define HAL_DCACHE_WRITETHRU_MODE       0
//#define HAL_DCACHE_WRITEBACK_MODE       1

// Load the contents of the given address range into the data cache
// and then lock the cache so that it stays there.
#define HAL_DCACHE_LOCK_DEFINED
#define HAL_DCACHE_LOCK(_base_, _size_)         \
{                                               \
    asm volatile ("mfc0 $2,$7;"                 \
                  "ori  $2,$2,0x0100;"          \
                  "mtc0 $2,$7;"                 \
                  :                             \
                  :                             \
                  : "$2"                        \
                 );                             \
}

// Undo a previous lock operation
#define HAL_DCACHE_UNLOCK_DEFINED
#define HAL_DCACHE_UNLOCK(_base_, _size_)       \
{                                               \
    asm volatile ("mfc0 $2,$7;"                 \
                  "la   $3,0xFFFFFEFF;"         \
                  "and  $2,$2,$3;"              \
                  "mtc0 $2,$7;"                 \
                  :                             \
                  :                             \
                  : "$2", "$3"                  \
                 );                             \
}

// Unlock entire cache
#define HAL_DCACHE_UNLOCK_ALL_DEFINED
#define HAL_DCACHE_UNLOCK_ALL() HAL_DCACHE_UNLOCK(0,HAL_DCACHE_SIZE)


//-----------------------------------------------------------------------------
// Data cache line control

// Allocate cache lines for the given address range without reading its
// contents from memory.
//#define HAL_DCACHE_ALLOCATE( _base_ , _size_ )

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
//#define HAL_DCACHE_FLUSH( _base_ , _size_ )
#define HAL_DCACHE_FLUSH_DEFINED        // Ensure no default definition

// Write dirty cache lines to memory for the given address range.
//#define HAL_DCACHE_STORE( _base_ , _size_ )
#define HAL_DCACHE_STORE_DEFINED        // Disable default definition

//-----------------------------------------------------------------------------
// Global control of Instruction cache

// Enable the instruction cache
// This uses a bit in the config register, which is TX39 specific.
#define HAL_ICACHE_ENABLE_DEFINED
#define HAL_ICACHE_ENABLE()                     \
{                                               \
    asm volatile ("mfc0 $2,$3;"                 \
                  "ori  $2,$2,0x0020;"          \
                  "mtc0 $2,$3;"                 \
                  :                             \
                  :                             \
                  : "$2"                        \
                 );                             \
                                                \
}

// Disable the instruction cache
#define HAL_ICACHE_DISABLE_DEFINED
#define HAL_ICACHE_DISABLE()                    \
{                                               \
    asm volatile ("mfc0 $2,$3;"                 \
                  "la   $3,0xFFFFFFDF;"         \
                  "and  $2,$2,$3;"              \
                  "mtc0 $2,$3;"                 \
                  "j    1f;"                    \
                  "nop;"                        \
                  ".balign 16,0;"               \
                  "1:;"                         \
                  :                             \
                  :                             \
                  : "$2", "$3"                  \
                 );                             \
                                                \
}

// Load the contents of the given address range into the instruction cache
// and then lock the cache so that it stays there.
#define HAL_ICACHE_LOCK_DEFINED
#define HAL_ICACHE_LOCK(_base_, _size_)         \
{                                               \
    asm volatile ("mfc0 $2,$7;"                 \
                  "ori  $2,$2,0x0200;"          \
                  "mtc0 $2,$7;"                 \
                  :                             \
                  :                             \
                  : "$2"                        \
                 );                             \
}

// Undo a previous lock operation
#define HAL_ICACHE_UNLOCK_DEFINED
#define HAL_ICACHE_UNLOCK(_base_, _size_)       \
{                                               \
    asm volatile ("mfc0 $2,$7;"                 \
                  "la   $3,0xFFFFFDFF;"         \
                  "and  $2,$2,$3;"              \
                  "mtc0 $2,$7;"                 \
                  :                             \
                  :                             \
                  : "$2", "$3"                  \
                 );                             \
}

// Unlock entire cache
#define HAL_ICACHE_UNLOCK_ALL_DEFINED
#define HAL_ICACHE_UNLOCK_ALL() HAL_ICACHE_UNLOCK(0, HAL_ICACHE_SIZE)

//-----------------------------------------------------------------------------
// Instruction cache line control
// On the TX39, the instruction cache must be disabled to use the index-invalidate
// cache operation.

// Invalidate the entire cache
// This uses the index-invalidate cache operation.
#define HAL_ICACHE_INVALIDATE_ALL_DEFINED
#define HAL_ICACHE_INVALIDATE_ALL()                                             \
{                                                                               \
    register CYG_ADDRESS _baddr_ = 0x80000000;                                  \
    register CYG_ADDRESS _addr_ = 0x80000000;                                   \
    register CYG_WORD _state_;                                                  \
    HAL_ICACHE_IS_ENABLED(_state_);                                             \
    HAL_ICACHE_DISABLE();                                                       \
    for( ; _addr_ < _baddr_+HAL_ICACHE_SIZE; _addr_ += HAL_ICACHE_LINE_SIZE )   \
    {                                                                           \
        asm volatile ("cache 0x00,0(%0)" : : "r"(_addr_) );                     \
    }                                                                           \
    if( _state_ ) HAL_ICACHE_ENABLE();                                          \
}

// Invalidate cache lines in the given range without writing to memory.
// This uses the index-invalidate cache operation since the TX39 does not
// have hit-invalidate on the instruction cache.
#define HAL_ICACHE_INVALIDATE_DEFINED
#define HAL_ICACHE_INVALIDATE( _base_ , _asize_ )                       \
{                                                                       \
    register CYG_ADDRESS _addr_ = (CYG_ADDRESS)(_base_);                \
    register CYG_WORD _size_ = (_asize_);                               \
    register CYG_WORD _state_;                                          \
    HAL_ICACHE_IS_ENABLED(_state_);                                     \
    HAL_ICACHE_DISABLE();                                               \
    for( ; _addr_ <= _addr_+_size_; _addr_ += HAL_ICACHE_LINE_SIZE )    \
    {                                                                   \
        asm volatile ("cache 0,0(%0)" : : "r"(_addr_) );                \
    }                                                                   \
    if( _state_ ) HAL_ICACHE_ENABLE();                                  \
}

#endif // CYGPKG_HAL_MIPS_TX3904

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_IMP_CACHE_H
// End of imp_cache.h
