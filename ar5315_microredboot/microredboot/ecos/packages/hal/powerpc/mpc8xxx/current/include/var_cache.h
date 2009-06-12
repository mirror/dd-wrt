#ifndef CYGONCE_VAR_CACHE_H
#define CYGONCE_VAR_CACHE_H
//=============================================================================
//
//      var_cache.h
//
//      Variant HAL cache control API
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
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
// Author(s):   pfine
// Contributors:nickg, jskov
// Date:        2001-12-12
// Purpose:     Variant cache control API
// Description: The macros defined here provide the HAL APIs for handling
//              cache control operations on the MPC8260 variant CPU.
// Usage:       Is included via the architecture cache header:
//              #include <cyg/hal/hal_cache.h>
//              ...
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/var_regs.h>

#include <cyg/hal/plf_cache.h>

//-----------------------------------------------------------------------------
// Cache dimensions

// Data cache
#define HAL_DCACHE_SIZE                 16384    // Size of data cache in bytes
#define HAL_DCACHE_LINE_SIZE            32       // Size of a data cache line
#define HAL_DCACHE_WAYS                 4        // Associativity of the cache

// Instruction cache
#define HAL_ICACHE_SIZE                 16384    // Size of cache in bytes
#define HAL_ICACHE_LINE_SIZE            32       // Size of a cache line
#define HAL_ICACHE_WAYS                 4        // Associativity of the cache

#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))
#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))

//-----------------------------------------------------------------------------
// Global control of data cache

// Enable the data cache
#define HAL_DCACHE_ENABLE()                 \
    CYG_MACRO_START                         \
    cyg_uint32 tmp1, tmp2;                  \
    asm volatile (                          \
        "mfspr %1, %2;"                     \
        "li %0, 0x4000;"                     \
        "rlwimi %1,%0,0,17,17;"             \
        "sync;"                             \
        "mtspr %2,%1;"                      \
        "isync;"                            \
        "sync;"                             \
        : "=r" (tmp1), "=r" (tmp2)          \
        : "I" (CYGARC_REG_HID0) /* %2 ==> HID0 */); \
    CYG_MACRO_END

// Disable the data cache
#define HAL_DCACHE_DISABLE()                \
    CYG_MACRO_START                         \
    register cyg_uint32 tmp1;               \
    register cyg_uint32 tmp2;               \
    for (tmp1 = 0; tmp1 < HAL_DCACHE_SIZE; tmp1 += HAL_DCACHE_LINE_SIZE) \
        tmp2 = *((cyg_uint32 *) tmp1);      \
    asm volatile (                          \
        "mfspr %1, %2;"                     \
        "li %0, 0x0;"                       \
        "rlwimi %1,%0,0,17,17;"             \
        "sync;"                             \
        "mtspr %2,%1;"                      \
        "isync;"                            \
        "sync;"                             \
        : "=r" (tmp1), "=r" (tmp2)          \
        : "I" (CYGARC_REG_HID0) /* %2 ==> HID0 */); \
    CYG_MACRO_END

// Invalidate the entire cache
#define HAL_DCACHE_INVALIDATE_ALL()                   \
    CYG_MACRO_START                                   \
    cyg_uint32 tmp1, tmp2;                            \
    asm volatile ("sync;"                             \
                  "mfspr %0, %2;"                     \
                  "ori   %0, %0, 0x0400;"             \
                  "mtspr %2, %0;"                     \
                  "li    %1, 0;"                      \
                  "rlwimi %0,%1,0,21,21;"             \
                  "mtspr %2, %0;"                     \
                  "sync;"                             \
                  : "=r" (tmp1), "=r" (tmp2)          \
                  : "I" (CYGARC_REG_HID0) /* %3 ==> HID0 */);\
    CYG_MACRO_END


// Synchronize the contents of the cache with memory.
// Modifications to this macro should mirror modifications to the
// identically named one in the ppc60x variant.
// We step through twice the number of lines in the cache in order
// to ensure that all dirty lines are flushed to main memory.
// (Consider the case where one of the dirty lines is in the
// first 16Kbytes of RAM -- it won't get flushed by loading
// in words from the first 16Kbytes of RAM).
#define HAL_DCACHE_SYNC()                                       \
    CYG_MACRO_START                                             \
    cyg_int32 i;                                                \
    cyg_uint32 *__base = (cyg_uint32 *) (0);                    \
    for(i=0;i< (2 * HAL_DCACHE_SIZE/HAL_DCACHE_LINE_SIZE);i++,__base += HAL_DCACHE_LINE_SIZE/4){                                                 \
        asm volatile ("lwz %%r0,0(%0);"::"r"(__base):"r0");     \
    }                                                           \
    CYG_MACRO_END

// Query the state of the data cache
#define HAL_DCACHE_IS_ENABLED(_state_)                          \
    asm volatile ("mfspr  %0, %1;"                              \
                  "rlwinm %0,%0,18,31,31;"                      \
                  : "=r" (_state_) : "I" (CYGARC_REG_HID0))     

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
#define HAL_DCACHE_UNLOCK_ALL()                       \
    asm volatile ("isync;"                            \
                  "mfspr %0, %2;"                     \
                  "oris  %1, 0,0xFFFF;"               \
                  "ori   %1,%1,0xEFFF;"               \
                  "and   %0,%0,%1;"                   \
                  "mtspr %2,%0;"                      \
                  "isync;"                            \
                  "sync;"                             \
                  : /* No output */                   \
                  : "I" (5) /* %0 ==> r5 */,          \
                    "I" (6) /* %1 ==> r6 */,          \
                    "I" (CYGARC_REG_HID0) /* %2 ==> HID0 */);

//-----------------------------------------------------------------------------
// Data cache line control

// Allocate cache lines for the given address range without reading its
// contents from memory.
//#define HAL_DCACHE_ALLOCATE( _base_ , _size_ )

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
#define HAL_DCACHE_FLUSH( _base_ , _size_ )                     \
    CYG_MACRO_START                                             \
    cyg_uint32 __base = (cyg_uint32) (_base_);                  \
    cyg_int32 __size = (cyg_int32) (_size_);                    \
    while (__size > 0) {                                        \
        asm volatile ("dcbf 0,%0;sync;" : : "r" (__base));      \
        __base += HAL_DCACHE_LINE_SIZE;                         \
        __size -= HAL_DCACHE_LINE_SIZE;                         \
    }                                                           \
    CYG_MACRO_END


// Invalidate cache lines in the given range without writing to memory.
// NOTE: The errata for the 603e processor indicates use of the dcbf
// command as the dcbi command will only invalidate modified blocks.
#define HAL_DCACHE_INVALIDATE( _base_ , _size_ )                \
    CYG_MACRO_START                                             \
    cyg_uint32 __base = (cyg_uint32) (_base_);                  \
    cyg_int32 __size = (cyg_int32) (_size_);                    \
    while (__size > 0) {                                        \
        asm volatile ("dcbf 0,%0;sync;" : : "r" (__base));      \
        __base += HAL_DCACHE_LINE_SIZE;                         \
        __size -= HAL_DCACHE_LINE_SIZE;                         \
    }                                                           \
    CYG_MACRO_END

// Write dirty cache lines to memory for the given address range.
#define HAL_DCACHE_STORE( _base_ , _size_ )                     \
    CYG_MACRO_START                                             \
    cyg_uint32 __base = (cyg_uint32) (_base_);                  \
    cyg_int32 __size = (cyg_int32) (_size_);                    \
    while (__size > 0) {                                        \
        asm volatile ("dcbst 0,%0;sync;" : : "r" (__base));     \
        __base += HAL_DCACHE_LINE_SIZE;                         \
        __size -= HAL_DCACHE_LINE_SIZE;                         \
    }                                                           \
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
// Global control of Instruction cache

// Enable the instruction cache
#define HAL_ICACHE_ENABLE()                 \
    CYG_MACRO_START                         \
    cyg_uint32 tmp1, tmp2;                  \
    asm volatile (                          \
        "mfspr %1, %2;"                     \
        "li %0, 0x4000;"                    \
        "rlwimi %1,%0,1,16,16;"             \
        "sync;"                             \
        "isync;"                            \
        "mtspr %2,%1;"                      \
        "isync;"                            \
        "sync;"                             \
        : "=r" (tmp1), "=r" (tmp2)          \
        : "I" (CYGARC_REG_HID0) /* %2 ==> HID0 */); \
    CYG_MACRO_END

// Disable the instruction cache
#define HAL_ICACHE_DISABLE()                          \
    CYG_MACRO_START                         \
    cyg_uint32 tmp1, tmp2;                  \
    asm volatile (                          \
        "mfspr %1, %2;"                     \
        "li %0, 0x0;"                       \
        "rlwimi %1,%0,0,16,16;"             \
        "sync;"                             \
        "isync;"                            \
        "mtspr %2,%1;"                      \
        "isync;"                            \
        "sync;"                             \
        : "=r" (tmp1), "=r" (tmp2)          \
        : "I" (CYGARC_REG_HID0) /* %2 ==> HID0 */); \
    CYG_MACRO_END

// Invalidate the entire cache
#if 1
#define HAL_ICACHE_INVALIDATE_ALL()                   \
    CYG_MACRO_START                                   \
    cyg_uint32 tmp1, tmp2;                            \
    asm volatile ("sync;"                             \
                  "mfspr %0, %2;"                     \
                  "ori   %1, %0, 0x8000;"             \
                  "mtspr %2, %1;"                     \
                  "isync;"                            \
                  "sync;"                             \
                  "ori   %1, %1, 0x0800;"             \
                  "mtspr %2, %1;"                     \
                  "isync;"                            \
                  "sync;"                             \
                  "mtspr %2, %0;"                     \
                  "isync;"                            \
                  "sync;"                             \
                  : "=r" (tmp1), "=r" (tmp2)          \
                  : "I" (CYGARC_REG_HID0) /* %3 ==> HID0 */);\
    CYG_MACRO_END
#else
#define HAL_ICACHE_INVALIDATE_ALL()                   \
    CYG_MACRO_START                                   \
    cyg_uint32 tmp1, tmp2;                            \
    asm volatile ("sync;"                             \
                  "mfspr %0, %2;"                     \
                  "ori   %0, %0, 0x0800;"             \
                  "isync;"                            \
                  "mtspr %2, %0;"                     \
                  "li    %1, 0;"                      \
                  "rlwimi %0,%1,0,20,20;"             \
                  "isync;"                            \
                  "mtspr %2, %0;"                     \
                  "isync;"                            \
                  "sync;"                             \
                  : "=r" (tmp1), "=r" (tmp2)          \
                  : "I" (CYGARC_REG_HID0) /* %3 ==> HID0 */);\
    CYG_MACRO_END
#endif
// Synchronize the contents of the cache with memory.
#define HAL_ICACHE_SYNC()                             \
    HAL_ICACHE_INVALIDATE_ALL()


// Query the state of the instruction cache
#define HAL_ICACHE_IS_ENABLED(_state_)                          \
    asm volatile ("mfspr  %0, %1;"                              \
                  "rlwinm %0,%0,17,31,31;"                      \
                  : "=r" (_state_) : "I" (CYGARC_REG_HID0))


// Set the instruction cache refill burst size
//#define HAL_ICACHE_BURST_SIZE(_size_)

// Load the contents of the given address range into the instruction cache
// and then lock the cache so that it stays there.
//#define HAL_ICACHE_LOCK(_base_, _size_)

// Undo a previous lock operation
//#define HAL_ICACHE_UNLOCK(_base_, _size_)

// Unlock entire cache
#define HAL_ICACHE_UNLOCK_ALL()                       \
    asm volatile ("isync;"                            \
                  "mfspr %0, %2;"                     \
                  "oris  %1, 0,0xFFFF;"               \
                  "ori   %1,%1,0xDFFF;"               \
                  "and   %0,%0,%1;"                   \
                  "isync;"                            \
                  "mtspr %2,%0;"                      \
                  "isync;"                            \
                  "sync;"                             \
                  : /* No output */                   \
                  : "I" (5) /* %0 ==> r5 */,          \
                    "I" (6) /* %1 ==> r6 */,          \
                    "I" (CYGARC_REG_HID0) /* %2 ==> HID0 */);

//-----------------------------------------------------------------------------
// Instruction cache line control

// Invalidate cache lines in the given range without writing to memory.
//#define HAL_ICACHE_INVALIDATE( _base_ , _size_ )

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_VAR_CACHE_H
// End of var_cache.h
