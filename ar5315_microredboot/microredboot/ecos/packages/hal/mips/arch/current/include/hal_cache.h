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

// Use this macro to allow the assembler to accept "cache" instructions,
// which are MIPS ISA 3. This is useful if someone is compiling
// with -mips2, but the architecture is really MIPS ISA 3.

#define _hal_asm_mips_cpp_stringize( _x_ ) #_x_
#define _HAL_ASM_SET_MIPS_ISA( _isal_ ) asm volatile ( \
      ".set mips" _hal_asm_mips_cpp_stringize(_isal_) )


//=============================================================================
// Default Implementation. This uses the standard MIPS CP0 registers and
// cache instructions. Note that not all variants will have all of the
// functionality defined here. 

//-----------------------------------------------------------------------------
// Cache dimensions.
// These really should be defined in var_cache.h. If they are not, then provide
// a set of numbers that are typical of many variants.

#ifndef HAL_DCACHE_SIZE

// Data cache
#define HAL_DCACHE_SIZE                 4096    // Size of data cache in bytes
#define HAL_DCACHE_LINE_SIZE            16      // Size of a data cache line
#define HAL_DCACHE_WAYS                 2       // Associativity of the cache

// Instruction cache
#define HAL_ICACHE_SIZE                 4096    // Size of cache in bytes
#define HAL_ICACHE_LINE_SIZE            16      // Size of a cache line
#define HAL_ICACHE_WAYS                 2       // Associativity of the cache

#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))
#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))

#endif

// Defines for various R4000+ MIPS cache operations
// this is a 5 bit field with bits 4-2 defining the operation
// and bits 1-0 defining which cache is being operated on
#define HAL_MIPS_CACHE_INDEX_INVALIDATE_I    0x00 /* 0 0 */
#define HAL_MIPS_CACHE_INDEX_WRITEBACK_INV_D 0x01 /* 0 1 */
#define HAL_MIPS_CACHE_INDEX_WRITEBACK_INV_S 0x03 /* 0 3 */
#define HAL_MIPS_CACHE_INDEX_LOAD_TAG_I      0x04 /* 1 0 */
#define HAL_MIPS_CACHE_INDEX_LOAD_TAG_D      0x05 /* 1 1 */
#define HAL_MIPS_CACHE_INDEX_LOAD_TAG_S      0x07 /* 1 3 */
#define HAL_MIPS_CACHE_INDEX_STORE_TAG_I     0x08 /* 2 0 */
#define HAL_MIPS_CACHE_INDEX_STORE_TAG_D     0x09 /* 2 1 */
#define HAL_MIPS_CACHE_INDEX_STORE_TAG_S     0x0b /* 2 3 */
#define HAL_MIPS_CACHE_HIT_INVALIDATE_I      0x10 /* 4 0 */
#define HAL_MIPS_CACHE_HIT_INVALIDATE_D      0x11 /* 4 1 */
#define HAL_MIPS_CACHE_HIT_INVALIDATE_S      0x13 /* 4 3 */
#define HAL_MIPS_CACHE_FILL_I                0x14 /* 5 0 */
#define HAL_MIPS_CACHE_HIT_WRITEBACK_INV_D   0x15 /* 5 1 */
#define HAL_MIPS_CACHE_HIT_WRITEBACK_INV_S   0x17 /* 5 3 */

// there seems to be different uses for operation
// code 6 depending on CPU
#define HAL_MIPS_CACHE_HIT_WRITEBACK_D       0x19 /* 6 1 */

#define HAL_MIPS_CACHE_INDEX_LOAD_DATA_I     0x18 /* 6 0 */
#define HAL_MIPS_CACHE_INDEX_LOAD_DATA_D     0x19 /* 6 1 */
#define HAL_MIPS_CACHE_INDEX_LOAD_DATA_S     0x1b /* 6 3 */

// there seems to be different uses for operation
// code 7 depending on CPU
#define HAL_MIPS_CACHE_FETCH_AND_LOCK_I      0x1c /* 7 0 */
#define HAL_MIPS_CACHE_FETCH_AND_LOCK_D      0x1d /* 7 1 */

#define HAL_MIPS_CACHE_INDEX_STORE_DATA_I    0x1c /* 7 0 */
#define HAL_MIPS_CACHE_INDEX_STORE_DATA_D    0x1d /* 7 1 */
#define HAL_MIPS_CACHE_INDEX_STORE_DATA_S    0x1f /* 7 3 */


//-----------------------------------------------------------------------------
// Cache instruction uses LSBs or MSBs (depending on the
// implementation) of the virtual address to specify which WAY to
// affect. The _ALL_WAYS macro defines the necessary cache instructions
// to affect all ways.

#ifdef HAL_MIPS_CACHE_INSN_USES_LSB
# define _IWAY(_n_) (_n_)
# define _DWAY(_n_) (_n_)
#else
# define _IWAY(_n_) ((_n_)*HAL_ICACHE_SIZE/HAL_ICACHE_WAYS)
# define _DWAY(_n_) ((_n_)*HAL_DCACHE_SIZE/HAL_DCACHE_WAYS)
#endif

#if (HAL_DCACHE_WAYS == 1)
#define _HAL_ASM_DCACHE_ALL_WAYS( _cmd_ , _addr_ )       \
    asm volatile ("cache %0,0(%1);"                      \
                    : : "I" ((_cmd_) | 1), "r"(_addr_) )
#elif (HAL_DCACHE_WAYS == 2)
#define _HAL_ASM_DCACHE_ALL_WAYS( _cmd_ , _addr_ )      \
    asm volatile ("cache %0,0(%1);"                     \
                  "cache %0,%2(%1);"                    \
                    : : "I" ((_cmd_) | 1), "r"(_addr_), \
                        "I" (_DWAY(1)))
#elif (HAL_DCACHE_WAYS == 4)
#define _HAL_ASM_DCACHE_ALL_WAYS( _cmd_ , _addr_ )      \
    asm volatile ("cache %0,0(%1);"                     \
                  "cache %0,%2(%1);"                    \
                  "cache %0,%3(%1);"                    \
                  "cache %0,%4(%1);"                    \
                    : : "I" ((_cmd_) | 1), "r"(_addr_), \
                        "I" (_DWAY(1)),                  \
                        "I" (_DWAY(2)),                  \
                        "I" (_DWAY(3)))
#else
# error "Unsupported number of ways"
#endif

#if (HAL_ICACHE_WAYS == 1)
#define _HAL_ASM_ICACHE_ALL_WAYS( _cmd_ , _addr_ )       \
    asm volatile ("cache %0,0(%1);"                      \
                    : : "I" (_cmd_), "r"(_addr_) )
#elif (HAL_ICACHE_WAYS == 2)
#define _HAL_ASM_ICACHE_ALL_WAYS( _cmd_ , _addr_ )      \
    asm volatile ("cache %0,0(%1);"                     \
                  "cache %0,%2(%1);"                    \
                    : : "I" (_cmd_), "r"(_addr_),       \
                        "I" (_IWAY(1)))
#elif (HAL_ICACHE_WAYS == 4)
#define _HAL_ASM_ICACHE_ALL_WAYS( _cmd_ , _addr_ )      \
    asm volatile ("cache %0,0(%1);"                     \
                  "cache %0,%2(%1);"                    \
                  "cache %0,%3(%1);"                    \
                  "cache %0,%4(%1);"                    \
                    : : "I" (_cmd_), "r"(_addr_),       \
                        "I" (_IWAY(1)),                  \
                        "I" (_IWAY(2)),                  \
                        "I" (_IWAY(3)))
#else
# error "Unsupported number of ways"
#endif

//-----------------------------------------------------------------------------
// Address adjustment.
// Given an address and a size, these macros return the first 
// cacheline containing the requested range and a terminating address.

#define HAL_DCACHE_START_ADDRESS(_addr_) \
(((CYG_ADDRESS)(_addr_)) & ~(HAL_DCACHE_LINE_SIZE-1))

#define HAL_DCACHE_END_ADDRESS(_addr_, _asize_) \
((CYG_ADDRESS)((_addr_) + (_asize_)))

#define HAL_ICACHE_START_ADDRESS(_addr_) \
(((CYG_ADDRESS)(_addr_)) & ~(HAL_ICACHE_LINE_SIZE-1))

#define HAL_ICACHE_END_ADDRESS(_addr_, _asize_) \
((CYG_ADDRESS)((_addr_) + (_asize_)))

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
    register CYG_ADDRESS _baddr_ = 0x80000000;                          \
    register CYG_ADDRESS _addr_ = 0x80000000;                           \
    register CYG_WORD _size_ = HAL_DCACHE_SIZE;                         \
    _HAL_ASM_SET_MIPS_ISA(3);                                           \
    for( ; _addr_ <= _baddr_+_size_; _addr_ += HAL_DCACHE_LINE_SIZE )   \
    { _HAL_ASM_DCACHE_ALL_WAYS(HAL_MIPS_CACHE_INDEX_WRITEBACK_INV_D, _addr_); } \
    _HAL_ASM_SET_MIPS_ISA(0);                                           \
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
#define HAL_DCACHE_LOCK(_base_, _asize_)                                    \
    CYG_MACRO_START                                                         \
    register CYG_ADDRESS _addr_  = HAL_DCACHE_START_ADDRESS(_base_);        \
    register CYG_ADDRESS _eaddr_ = HAL_DCACHE_END_ADDRESS(_base_, _asize_); \
    register CYG_WORD _state_;                                              \
    HAL_DCACHE_IS_ENABLED( _state_ );                                       \
    if( _state_ ) {                                                         \
        _HAL_ASM_SET_MIPS_ISA(3);                                           \
        for( ; _addr_ < _eaddr_; _addr_ += HAL_DCACHE_LINE_SIZE )           \
        { _HAL_ASM_DCACHE_ALL_WAYS(HAL_MIPS_CACHE_FETCH_AND_LOCK_D, _addr_); } \
        _HAL_ASM_SET_MIPS_ISA(0);                                           \
    }                                                                       \
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
#define HAL_DCACHE_FLUSH( _base_ , _asize_ )                                \
    CYG_MACRO_START                                                         \
    register CYG_ADDRESS _addr_  = HAL_DCACHE_START_ADDRESS(_base_);        \
    register CYG_ADDRESS _eaddr_ = HAL_DCACHE_END_ADDRESS(_base_, _asize_); \
    register CYG_WORD _state_;                                              \
    HAL_DCACHE_IS_ENABLED( _state_ );                                       \
    if( _state_ ) {                                                         \
        _HAL_ASM_SET_MIPS_ISA(3);                                           \
        for( ; _addr_ < _eaddr_; _addr_ += HAL_DCACHE_LINE_SIZE )           \
        { _HAL_ASM_DCACHE_ALL_WAYS(HAL_MIPS_CACHE_HIT_WRITEBACK_INV_D, _addr_); } \
        _HAL_ASM_SET_MIPS_ISA(0);                                           \
    }                                                                       \
    CYG_MACRO_END
#endif

// Invalidate cache lines in the given range without writing to memory.
// This uses the hit-invalidate cache operation.
#ifndef HAL_DCACHE_INVALIDATE_DEFINED
#define HAL_DCACHE_INVALIDATE( _base_ , _asize_ )                           \
    CYG_MACRO_START                                                         \
    register CYG_ADDRESS _addr_  = HAL_DCACHE_START_ADDRESS(_base_);        \
    register CYG_ADDRESS _eaddr_ = HAL_DCACHE_END_ADDRESS(_base_, _asize_); \
    _HAL_ASM_SET_MIPS_ISA(3);                                               \
    for( ; _addr_ < _eaddr_; _addr_ += HAL_DCACHE_LINE_SIZE )               \
    { _HAL_ASM_DCACHE_ALL_WAYS(HAL_MIPS_CACHE_HIT_INVALIDATE_D, _addr_); }  \
    _HAL_ASM_SET_MIPS_ISA(0);                                               \
    CYG_MACRO_END
#endif

// Write dirty cache lines to memory for the given address range.
// This uses the hit-writeback cache operation.
#ifndef HAL_DCACHE_STORE_DEFINED
#define HAL_DCACHE_STORE( _base_ , _asize_ )                                \
    CYG_MACRO_START                                                         \
    register CYG_ADDRESS _addr_  = HAL_DCACHE_START_ADDRESS(_base_);        \
    register CYG_ADDRESS _eaddr_ = HAL_DCACHE_END_ADDRESS(_base_, _asize_); \
    register CYG_WORD _state_;                                              \
    HAL_DCACHE_IS_ENABLED( _state_ );                                       \
    if( _state_ ) {                                                         \
        _HAL_ASM_SET_MIPS_ISA(3);                                           \
        for( ; _addr_ < _eaddr_; _addr_ += HAL_DCACHE_LINE_SIZE )           \
        { _HAL_ASM_DCACHE_ALL_WAYS(HAL_MIPS_CACHE_HIT_WRITEBACK_D, _addr_); } \
        _HAL_ASM_SET_MIPS_ISA(0);                                           \
    }                                                                       \
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
    register CYG_ADDRESS _baddr_ = 0x80000000;                                \
    register CYG_ADDRESS _addr_ = 0x80000000;                                 \
    _HAL_ASM_SET_MIPS_ISA(3);                                                 \
    for( ; _addr_ < _baddr_+HAL_ICACHE_SIZE; _addr_ += HAL_ICACHE_LINE_SIZE ) \
    { _HAL_ASM_ICACHE_ALL_WAYS(HAL_MIPS_CACHE_INDEX_INVALIDATE_I, _addr_); }  \
    _HAL_ASM_SET_MIPS_ISA(0);                                                 \
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
#define HAL_ICACHE_LOCK(_base_, _asize_)                                    \
    CYG_MACRO_START                                                         \
    register CYG_ADDRESS _addr_  = HAL_ICACHE_START_ADDRESS(_base_);        \
    register CYG_ADDRESS _eaddr_ = HAL_ICACHE_END_ADDRESS(_base_, _asize_); \
    register CYG_WORD _state_;                                              \
    HAL_ICACHE_IS_ENABLED( _state_ );                                       \
    if( _state_ ) {                                                         \
        _HAL_ASM_SET_MIPS_ISA(3);                                           \
        for( ; _addr_ < _eaddr_; _addr_ += HAL_ICACHE_LINE_SIZE )           \
        { _HAL_ASM_ICACHE_ALL_WAYS(HAL_MIPS_CACHE_FETCH_AND_LOCK_I, _addr_); } \
        _HAL_ASM_SET_MIPS_ISA(0);                                           \
    }                                                                       \
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
#define HAL_ICACHE_INVALIDATE( _base_ , _asize_ )                           \
    CYG_MACRO_START                                                         \
    register CYG_ADDRESS _addr_  = HAL_ICACHE_START_ADDRESS(_base_);        \
    register CYG_ADDRESS _eaddr_ = HAL_ICACHE_END_ADDRESS(_base_, _asize_); \
    _HAL_ASM_SET_MIPS_ISA(3);                                               \
    for( ; _addr_ < _eaddr_; _addr_ += HAL_ICACHE_LINE_SIZE )               \
    { _HAL_ASM_ICACHE_ALL_WAYS(HAL_MIPS_CACHE_HIT_INVALIDATE_I, _addr_); }  \
    _HAL_ASM_SET_MIPS_ISA(0);                                               \
    CYG_MACRO_END
#endif

//-----------------------------------------------------------------------------
// Check that a supported configuration has actually defined some macros.

#ifndef HAL_DCACHE_ENABLE

#error Unsupported MIPS configuration

#endif

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
