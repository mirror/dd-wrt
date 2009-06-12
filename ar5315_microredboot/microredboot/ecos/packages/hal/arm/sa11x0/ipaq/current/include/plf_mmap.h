#ifndef CYGONCE_HAL_SA11X0_PLATFORM_PLF_MMAP_H
#define CYGONCE_HAL_SA11X0_PLATFORM_PLF_MMAP_H
/*=============================================================================
//
//      plf_mmap.h
//
//      Platform specific memory map support
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
// Author(s):    hmt
// Contributors: hmt
// Date:         2001-01-04
// Purpose:      Intel SA11x0 series platform-specific memory mapping macros
// Description:  Macros to convert a cached, virtual address to
//               1) an uncached adddress for the same memory (if available)
//               2) the equivalent physical address for giving to external
//               hardware eg. DMA engines.
//               
//               NB: this mapping is expressed here statically, independent
//               of the actual mapping installed in the MMAP table.  So if
//               someone changes that, or its initialisation is changed,
//               then this module must change.  This is intended to be
//               efficient at a cost of generality.  It is also intended to
//               be called with constants (such as &my_struct) as input
//               args, so that all the work can be done at compile time,
//               with optimization on.
//
// Usage:        #include <cyg/hal/hal_cache.h>
//		 (which includes this file itself)
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/hal/hal_misc.h>

// Get the pagesize for a particular virtual address:

// This does not depend on the vaddr.
#define HAL_MM_PAGESIZE( vaddr, pagesize ) CYG_MACRO_START      \
    (pagesize) = SZ_1M;                                         \
CYG_MACRO_END

// Get the physical address from a virtual address:

// Only RAM and ROM are mapped; we just pass through all other values,
// rather than detecting nonexistent areas here.

#define HAL_VIRT_TO_PHYS_ADDRESS( vaddr, paddr ) CYG_MACRO_START           \
    cyg_uint32 _v_ = (cyg_uint32)(vaddr);                                  \
    if ( 32 * SZ_1M > _v_ )         /* 32Mb of SDRAM Bank 0 from 0-32Mb */ \
        _v_ += 0xc00u * SZ_1M;                                             \
    else if ( 0x500u * SZ_1M > _v_ ) /* Space between RAM and mapped ROM */\
        /* no change */ ;                                                  \
    else if ( 0x520u * SZ_1M > _v_ ) /* Mapped boot ROM size 32Mb */       \
        _v_ -= 0x500u * SZ_1M;                                             \
    else                            /* Rest of it */                       \
        /* no change */ ;                                                  \
    (paddr) = _v_;                                                         \
CYG_MACRO_END

// Get the virtual address for a physical address:

// Only RAM and ROM are mapped; we just pass through all other values,
// rather than detecting nonexistent areas here.

#define HAL_PHYS_TO_VIRT_ADDRESS( paddr, vaddr ) CYG_MACRO_START           \
    cyg_uint32 _p_ = (cyg_uint32)(paddr);                                  \
    if ( 32 * SZ_1M > _p_ )         /* 32Mb Boot ROM mapped to 0x500Mb */  \
        _p_ += 0x500u * SZ_1M;                                             \
    else if ( 0xc00u * SZ_1M > _p_ ) /* Space between ROM and SDRAM */     \
        /* no change */ ;                                                  \
    else if ( 0xc20u * SZ_1M > _p_ ) /* Raw RAM size 32Mb */               \
        _p_ -= 0xc00u * SZ_1M;                                             \
    else                            /* Rest of it */                       \
        /* no change */ ;                                                  \
    (vaddr) = _p_ ;                                                        \
CYG_MACRO_END

// Get a non-cacheable address for accessing the same store as a virtual
// (assumed cachable) address:

// Only RAM is mapped: ROM is only available cachable, everything else is
// not cachable anyway.

#define HAL_VIRT_TO_UNCACHED_ADDRESS( vaddr, uaddr ) CYG_MACRO_START       \
    cyg_uint32 _v_ = (cyg_uint32)(vaddr);                                  \
    if ( 32 * SZ_1M > _v_ )         /* 32Mb of SDRAM Bank 0 from 0-32Mb */ \
        _v_ += 0xc00u * SZ_1M;                                              \
    else            /* Everything else is already uncacheable or is ROM */ \
        /* no change */ ;                                                  \
    (uaddr) = _v_ ;                                                        \
CYG_MACRO_END

//---------------------------------------------------------------------------
#endif // CYGONCE_HAL_SA11X0_PLATFORM_PLF_MMAP_H
// EOF plf_mmap.h
