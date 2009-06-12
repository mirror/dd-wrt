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
//              Travis C. Furrer <furrer@mit.edu>
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
#include <cyg/hal/hal_sa11x0.h>
#include <cyg/hal/hal_mmu.h>

//-----------------------------------------------------------------------------
// FIXME: This definition forces the IO flash driver to use a
// known-good procedure for fiddling flash before calling flash device
// driver functions. The procedure breaks on other platform/driver
// combinations though so is depricated. Hence this definition.
//
// If you work on this target, please try to remove this definition
// and verify that the flash driver still works (both from RAM and
// flash). If it does, remove the definition and this comment for good
// [and the old macro definition if this happens to be the last client
// of that code].
#if defined(CYGPKG_HAL_ARM_SA11X0_ASSABET) \
    || defined(CYGPKG_HAL_ARM_SA11X0_SA1100MM)
# define HAL_FLASH_CACHES_OLD_MACROS
#endif

//-----------------------------------------------------------------------------
// Cache dimensions

#define HAL_ICACHE_SIZE                 SA11X0_ICACHE_SIZE
#define HAL_ICACHE_LINE_SIZE            SA11X0_ICACHE_LINESIZE_BYTES
#define HAL_ICACHE_WAYS                 SA11X0_ICACHE_WAYS
#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))

#define HAL_DCACHE_SIZE                 SA11X0_DCACHE_SIZE
#define HAL_DCACHE_LINE_SIZE            SA11X0_DCACHE_LINESIZE_BYTES
#define HAL_DCACHE_WAYS                 SA11X0_DCACHE_WAYS
#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))

// FIXME: much of the code below should make better use of
// the definitions from hal_mmu.h

//-----------------------------------------------------------------------------
// Global control of Instruction cache

// Enable the instruction cache
#define HAL_ICACHE_ENABLE()                                             \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc  p15,0,r1,c1,c0,0;"                                        \
        "orr  r1,r1,#0x1000;"                                           \
        "orr  r1,r1,#0x0003;" /* enable ICache (also ensures   */       \
                              /* that MMU and alignment faults */       \
                              /* are enabled)                  */       \
        "mcr  p15,0,r1,c1,c0,0"                                         \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Disable the instruction cache (and invalidate it, required semanitcs)
#define HAL_ICACHE_DISABLE()                                            \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc    p15,0,r1,c1,c0,0;"                                      \
        "bic    r1,r1,#0x1000;" /* disable ICache (but not MMU, etc) */ \
        "mcr    p15,0,r1,c1,c0,0;"                                      \
        "mov    r1,#0;"                                                 \
        "mcr    p15,0,r1,c7,c5,0;"  /* flush ICache */                  \
        "nop;" /* next few instructions may be via cache    */          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop"                                                           \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Query the state of the instruction cache
#define HAL_ICACHE_IS_ENABLED(_state_)                                   \
CYG_MACRO_START                                                          \
    register cyg_uint32 reg;                                             \
    asm volatile ("mrc  p15,0,%0,c1,c0,0"                                \
                  : "=r"(reg)                                            \
                  :                                                      \
        );                                                               \
    (_state_) = (0 != (0x1000 & reg)); /* Bit 12 is ICache enable */     \
CYG_MACRO_END

// Invalidate the entire cache
#define HAL_ICACHE_INVALIDATE_ALL()                                     \
CYG_MACRO_START                                                         \
    /* this macro can discard dirty cache lines (N/A for ICache) */     \
    asm volatile (                                                      \
        "mov    r1,#0;"                                                 \
        "mcr    p15,0,r1,c7,c5,0;"  /* flush ICache */                  \
        "mcr    p15,0,r1,c8,c5,0;"  /* flush ITLB only */               \
        "nop;" /* next few instructions may be via cache    */          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
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
// This feature is not available on the SA11X0.

// Load the contents of the given address range into the instruction cache
// and then lock the cache so that it stays there.
//#define HAL_ICACHE_LOCK(_base_, _size_)
// This feature is not available on the SA11X0.

// Undo a previous lock operation
//#define HAL_ICACHE_UNLOCK(_base_, _size_)
// This feature is not available on the SA11X0.

// Unlock entire cache
//#define HAL_ICACHE_UNLOCK_ALL()
// This feature is not available on the SA11X0.

//-----------------------------------------------------------------------------
// Instruction cache line control

// Invalidate cache lines in the given range without writing to memory.
//#define HAL_ICACHE_INVALIDATE( _base_ , _size_ )
// This feature is not available on the SA11X0.

//-----------------------------------------------------------------------------
// Global control of data cache

// Enable the data cache
#define HAL_DCACHE_ENABLE()                                             \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc  p15,0,r1,c1,c0,0;"                                        \
        "orr  r1,r1,#0x000F;" /* enable DCache (also ensures    */      \
                              /* the MMU, alignment faults, and */      \
                              /* write buffer are enabled)      */      \
        "mcr  p15,0,r1,c1,c0,0"                                         \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Disable the data cache (and invalidate it, required semanitcs)
#define HAL_DCACHE_DISABLE()                                            \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc  p15,0,r1,c1,c0,0;"                                        \
        "bic  r1,r1,#0x000C;" /* disable DCache AND write buffer  */    \
                              /* but not MMU and alignment faults */    \
        "mcr  p15,0,r1,c1,c0,0;"                                        \
        "mov    r1,#0;"                                                 \
        "mcr  p15,0,r1,c7,c6,0" /* clear data cache */                  \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Query the state of the data cache
#define HAL_DCACHE_IS_ENABLED(_state_)                                   \
CYG_MACRO_START                                                          \
    register int reg;                                                    \
    asm volatile ("mrc  p15,0,%0,c1,c0,0;"                               \
                  : "=r"(reg)                                            \
                  :                                                      \
        );                                                               \
    (_state_) = (0 != (4 & reg)); /* Bit 2 is DCache enable */           \
CYG_MACRO_END

// Flush the entire dcache (and then both TLBs, just in case)
#define HAL_DCACHE_INVALIDATE_ALL()                                     \
CYG_MACRO_START    /* this macro can discard dirty cache lines. */      \
    asm volatile (                                                      \
        "mov    r1,#0;"                                                 \
	"mcr    p15,0,r1,c7,c6,0;"                                      \
        "mcr    p15,0,r1,c8,c7,0;"                                      \
        :                                                               \
	:                                                               \
	: "r1","memory" );                                              \
CYG_MACRO_END


// Synchronize the contents of the cache with memory.
#define HAL_DCACHE_SYNC()                                               \
CYG_MACRO_START                                                         \
    /* This is slightly naff in that the only way to force a dirty */   \
    /* line out is by loading other data into its slot, so         */   \
    /* invalidating that slot.                                     */   \
    asm volatile (                                                      \
        "mov    r0, #0xE0000000;" /* SA11X0 zeros bank (128Mb) */       \
        "add    r1, r0, #0x2000;" /* We read 8kB of it */               \
 "667: "                                                                \
        "ldr    r2, [r0], #32;"                                         \
        "teq    r1, r0;"                                                \
        "bne    667b;"                                                  \
        "mov    r0,#0;"                                                 \
        "mcr    p15,0,r0,c7,c6,0;"  /* flush DCache */                  \
        "mcr    p15,0,r0,c7,c10,4;" /* and drain the write buffer */    \
        :                                                               \
        :                                                               \
        : "r0","r1","r2" /* Clobber list */                             \
        );                                                              \
CYG_MACRO_END

// Set the data cache refill burst size
//#define HAL_DCACHE_BURST_SIZE(_size_)
// This feature is not available on the SA11X0.

// Set the data cache write mode
//#define HAL_DCACHE_WRITE_MODE( _mode_ )
// This feature is not available on the SA11X0.

#define HAL_DCACHE_WRITETHRU_MODE       0
#define HAL_DCACHE_WRITEBACK_MODE       1

// Get the current writeback mode - or only writeback mode if fixed
#define HAL_DCACHE_QUERY_WRITE_MODE( _mode_ ) CYG_MACRO_START           \
    _mode_ = HAL_DCACHE_WRITEBACK_MODE;                                 \
CYG_MACRO_END

// Load the contents of the given address range into the data cache
// and then lock the cache so that it stays there.
//#define HAL_DCACHE_LOCK(_base_, _size_)
// This feature is not available on the SA11X0.

// Undo a previous lock operation
//#define HAL_DCACHE_UNLOCK(_base_, _size_)
// This feature is not available on the SA11X0.

// Unlock entire cache
//#define HAL_DCACHE_UNLOCK_ALL()
// This feature is not available on the SA11X0.

//-----------------------------------------------------------------------------
// Data cache line control

// Allocate cache lines for the given address range without reading its
// contents from memory.
//#define HAL_DCACHE_ALLOCATE( _base_ , _size_ )
// This feature is not available on the SA11X0.

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
#define HAL_DCACHE_FLUSH( _base_ , _size_ )     \
CYG_MACRO_START                                 \
    HAL_DCACHE_STORE( _base_ , _size_ );        \
    HAL_DCACHE_INVALIDATE( _base_ , _size_ );   \
CYG_MACRO_END

// Invalidate cache lines in the given range without writing to memory.
#define HAL_DCACHE_INVALIDATE( _base_ , _size_ )                        \
CYG_MACRO_START                                                         \
    register int addr, enda;                                            \
    for ( addr = (~(HAL_DCACHE_LINE_SIZE - 1)) & (int)(_base_),         \
              enda = (int)(_base_) + (_size_);                          \
          addr < enda ;                                                 \
          addr += HAL_DCACHE_LINE_SIZE )                                \
    {                                                                   \
        asm volatile (                                                  \
                      "mcr  p15,0,%0,c7,c6,1;" /* flush entry away */   \
                      :                                                 \
                      : "r"(addr)                                       \
                      : "memory"                                        \
            );                                                          \
    }                                                                   \
CYG_MACRO_END
                          
// Write dirty cache lines to memory for the given address range.
#define HAL_DCACHE_STORE( _base_ , _size_ )                             \
CYG_MACRO_START                                                         \
    register int addr, enda;                                            \
    for ( addr = (~(HAL_DCACHE_LINE_SIZE - 1)) & (int)(_base_),         \
              enda = (int)(_base_) + (_size_);                          \
          addr < enda ;                                                 \
          addr += HAL_DCACHE_LINE_SIZE )                                \
    {                                                                   \
        asm volatile ("mcr  p15,0,%0,c7,c10,1;" /* push entry to RAM */ \
                      :                                                 \
                      : "r"(addr)                                       \
                      : "memory"                                        \
            );                                                          \
    }                                                                   \
    /* and also drain the write buffer */                               \
    asm volatile (                                                      \
        "mov    r1,#0;"                                                 \
	"mcr    p15,0,r1,c7,c10,4;"                                     \
        :                                                               \
        :                                                               \
        : "r1", "memory" /* Clobber list */                             \
    );                                                                  \
CYG_MACRO_END

// Preread the given range into the cache with the intention of reading
// from it later.
//#define HAL_DCACHE_READ_HINT( _base_ , _size_ )
// This feature is available on the SA11X0, but due to tricky
// coherency issues with the read buffer (see SA11X0 developer's
// manual page 6-7) we don't bother to implement it here.

// Preread the given range into the cache with the intention of writing
// to it later.
//#define HAL_DCACHE_WRITE_HINT( _base_ , _size_ )
// This feature is not available on the SA11X0.

// Allocate and zero the cache lines associated with the given range.
//#define HAL_DCACHE_ZERO( _base_ , _size_ )
// This feature is not available on the SA11X0.

//-----------------------------------------------------------------------------
// Now include the details of the platform's Memory Map setup:

#include <cyg/hal/plf_mmap.h>

// and define the (considerably less efficient) routines that are available
// for testing the actual memory map in force.

externC cyg_uint32 hal_virt_to_phys_address( cyg_uint32 vaddr );
externC cyg_uint32 hal_phys_to_virt_address( cyg_uint32 paddr );
externC cyg_uint32 hal_virt_to_uncached_address( cyg_uint32 vaddr );

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
