#ifndef CYGONCE_HAL_ARM_EDB7XXX_PLF_IO_H
#define CYGONCE_HAL_ARM_EDB7XXX_PLF_IO_H

/*=============================================================================
//
//      plf_io.h
//
//      Platform specific support (register layout, etc)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas <gary@mind.be>
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
// Author(s):    jskov
// Contributors: jskov, gthomas
// Date:         2002-01-28
// Purpose:      Platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_io.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include <cyg/hal/hal_edb7xxx.h>

extern unsigned long _edb7xxx_physical_address(unsigned long addr);
#define CYGARC_PHYSICAL_ADDRESS(x) _edb7xxx_physical_address((unsigned long) x)

#if defined(CYGPKG_REDBOOT_ARM_LINUX_EXEC) && defined(CYGHWR_HAL_ARM_EDB7XXX_VARIANT_EP7312)
#define _CYGHWR_LAYOUT_ONLY
#include <cyg/hal/hal_platform_setup.h>
// Describe memory layout for Linux
#define CYGHWR_REDBOOT_LINUX_ATAG_MEM(_p_)                                                      \
    CYG_MACRO_START                                                                             \
    /* Next ATAG_MEM. */                                                                        \
    _p_->hdr.size = (sizeof(struct tag_mem32) + sizeof(struct tag_header))/sizeof(long);        \
    _p_->hdr.tag = ATAG_MEM;                                                                    \
    /* Round up so there's only one bit set in the memory size.                                 \
     * Don't double it if it's already a power of two, though.                                  \
     */                                                                                         \
    _p_->u.mem.size  = 1<<hal_msbindex(CYGMEM_REGION_ram_SIZE);                                 \
    if (_p_->u.mem.size < CYGMEM_REGION_ram_SIZE)                                               \
	    _p_->u.mem.size <<= 1;                                                              \
    _p_->u.mem.start = DRAM_PA_START;                                                           \
    CYG_MACRO_END
#endif

#endif // CYGONCE_HAL_ARM_EDB7XXX_PLF_IO_H
// EOF plf_io.h
