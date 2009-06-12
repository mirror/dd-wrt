#ifndef CYGONCE_HAL_ARM_XSCALE_UE250_UE250_H
#define CYGONCE_HAL_ARM_XSCALE_UE250_UE250_H

//=============================================================================
//
//      uE250.h
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_H
#include <pkgconf/hal_arm_xscale_uE250.h>
#include <cyg/hal/hal_pxa2x0.h>           // Applications Processor defines

#define SDRAM_PHYS_BASE     0xa0000000
#define SDRAM_BASE          0x00000000
#define SDRAM_SIZE          0x04000000    // 64 MB
#define SDRAM_MAX           0x10000000 

#define UE250_FLASH_ADDR    0x50000000
// These must match setup in the page table in hal_platform_extras.h
#define SDRAM_UNCACHED_BASE 0xc0000000
#undef  DCACHE_FLUSH_AREA
#define DCACHE_FLUSH_AREA   0xe0000000

#define FPGA_BASE           0x50040000
#define VGA_BASE            0x50080000

// ------------------------------------------------------------------------

#endif // CYGONCE_HAL_ARM_XSCALE_UE250_UE250_H
