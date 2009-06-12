#ifndef CYGONCE_HAL_PLATFORM_EXTRAS_H
#define CYGONCE_HAL_PLATFORM_EXTRAS_H

/*=============================================================================
//
//      hal_platform_extras.h
//
//      Platform specific MMU table.
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2001-12-03
// Purpose:      Intel XScale IQ80321 platform specific mmu table
// Description: 
// Usage:        #include <cyg/hal/hal_platform_extras.h>
//     Only used by "vectors.S"         
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#if defined(CYG_HAL_STARTUP_ROM)
        .section .mmu_tables, "a"

#ifdef CYG_HAL_MEMORY_MAP_NORMAL
    mmu_table:
        //  This page table sets up the preferred mapping:
        //
        //  Virtual Address   Physical Address  Size (MB)  Description
        //  ---------------   ----------------  ---------  -----------
        //     0x00000000       0xA0000000         512     DRAM
        //     0x20000000       0x00000000        2048     ATU Outbound Direct
        //     0xA0000000       0x80000000         257     ATU Outbound Xlate
        //     0xB0100000       0x90100000         255     Unused
        //     0xC0000000       0xA0000000         512     Uncached DRAM alias
        //     0xE0000000       0xE0000000           1     Cache Flush
        //     0xE0100000       0xE0100000         255     Unused
        //     0xF0000000       0xF0000000           8     FLASH (PBIU CS0)
        //     0xF0800000       0xF0800000         224     Unused
        //     0xFE800000       0xFE800000           1     PBIU CS1-CS5
        //     0xFE900000       0xFE900000          22     Unused
        //     0xFFF00000       0xFFF00000           1     Verde PMMR

	// 512MB ECC DRAM
	// In flash based table: X=1, C=1, B=1, ECC
	.set	__base,0xA00
	.rept	0x200 - 0x000
	FL_SECTION_ENTRY __base,1,3,1,0,1,1
	.set	__base,__base+1
	.endr

	// 2048MB ATU Outbound direct
	// X=0, Non-cacheable, Non-bufferable
	.set	__base,0x000
	.rept	0xA00 - 0x200
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 257MB ATU Outbound translation
	// X=0, Non-cacheable, Non-bufferable
	.rept	0xB01 - 0xA00
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// Nothing here
	.rept	0xC00 - 0xB01
	FL_SECTION_ENTRY __base,0,0,0,0,0,0
	.set	__base,__base+1
	.endr

	// Uncached/unbuffered alias for 512MB ECC DRAM
	.set	__base,0xA00
	.rept	0xE00 - 0xC00
	FL_SECTION_ENTRY __base,0,3,1,0,0,0
	.set	__base,__base+1
	.endr

	// Cache flush region.
	// Don't need physical memory, just a cached area.
	.set	__base,0xE00
	.rept	0xE01 - 0xE00
	FL_SECTION_ENTRY __base,1,3,0,0,1,1
	.set	__base,__base+1
	.endr
	
	// Nothing here
	.rept	0xF00 - 0xE01
	FL_SECTION_ENTRY __base,0,0,0,0,0,0
	.set	__base,__base+1
	.endr

	// 8MB of FLASH
	// X=0, Cacheable, Non-bufferable
	.rept	0xF08 - 0xF00
	FL_SECTION_ENTRY __base,0,3,0,0,1,0
	.set	__base,__base+1
	.endr	

	// Nothing here
	.rept	0xFE8 - 0xF08
	FL_SECTION_ENTRY __base,0,0,0,0,0,0
	.set	__base,__base+1
	.endr

	// PBIU CS1 - CS5
	// X=0, Non-cacheable, Non-bufferable
	.rept	0xFE9 - 0xFE8
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// Nothing here
	.rept	0xFFF - 0xFE9
	FL_SECTION_ENTRY __base,0,0,0,0,0,0
	.set	__base,__base+1
	.endr

	// Verde PMMR
	// X=0, Non-cacheable, Non-bufferable
	.rept	0x1000 - 0xFFF
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr
#else  /* CYG_HAL_MEMORY_MAP_NORMAL */
    mmu_table:
	//  This page table sets up an alternate mapping:
	//
	//  Virtual Address   Physical Address  Size (MB)  Description
	//  ---------------   ----------------  ---------  -----------
	//     0x00000000       0xA0000000           1     DRAM
	//     0x00100000       0x00100000        2047     ATU Outbound Direct
        //     0x80000000       0x80000000         257     ATU Outbound Xlate
        //     0x90100000       0x90100000         255     Invalid
        //     0xA0000000       0xA0000000         512     DRAM
        //     0xC0000000       0xA0000000         512     Uncached DRAM alias
	//     0xE0000000       0xE0000000           1     Cache Flush
	//     0xE0100000       0xE0100000         255     Invalid
        //     0xF0000000       0xF0000000           8     FLASH (PBIU CS0)
        //     0xF0800000       0xF0800000         224     Invalid
        //     0xFE800000       0xFE800000           1     PBIU CS1-CS5
        //     0xFE900000       0xFE900000          22     Invalid
        //     0xFFF00000       0xFFF00000           1     Verde PMMR

	// 1MB ECC DRAM
	// This is a duplicate mapping of the first MB of RAM. It is mapped
        // here for CPU exception vectors.
	.set	__base,0xA00
	.rept	0x001 - 0x000
	FL_SECTION_ENTRY __base,1,3,1,0,1,1
	.set	__base,__base+1
	.endr
	.set	__base,0x001

	// 2047MB ATU Outbound direct
	// X=0, Non-cacheable, Non-bufferable
	.rept	0x800 - 0x001
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 257MB ATU Outbound translation
	// X=0, Non-cacheable, Non-bufferable
	.rept	0x901 - 0x800
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// Nothing here. Invalid.
	.rept	0xA00 - 0x901
	FL_SECTION_ENTRY __base,0,0,0,0,0,0
	.set	__base,__base+1
	.endr

	// 512MB ECC DRAM
	// X=1, C=1, B=1, ECC
	.rept	0xC00 - 0xA00
	FL_SECTION_ENTRY __base,1,3,1,0,1,1
	.set	__base,__base+1
	.endr

	// Uncached/unbuffered alias for 512MB ECC DRAM
	.set	__base,0xA00
	.rept	0xE00 - 0xC00
	FL_SECTION_ENTRY __base,0,3,1,0,0,0
	.set	__base,__base+1
	.endr

	// Cache flush region.
	// Don't need physical memory, just a cached area.
	.set	__base,0xE00
	.rept	0xE01 - 0xE00
	FL_SECTION_ENTRY __base,1,3,0,0,1,1
	.set	__base,__base+1
	.endr
	
	// Nothing here
	.rept	0xF00 - 0xE01
	FL_SECTION_ENTRY __base,0,0,0,0,0,0
	.set	__base,__base+1
	.endr

	// 8MB of FLASH
	// X=0, Cacheable, Non-bufferable
	.rept	0xF08 - 0xF00
	FL_SECTION_ENTRY __base,0,3,0,0,1,0
	.set	__base,__base+1
	.endr	

	// Nothing here
	.rept	0xFE8 - 0xF08
	FL_SECTION_ENTRY __base,0,0,0,0,0,0
	.set	__base,__base+1
	.endr

	// PBIU CS1 - CS5
	// X=0, Non-cacheable, Non-bufferable
	.rept	0xFE9 - 0xFE8
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// Nothing here
	.rept	0xFFF - 0xFE9
	FL_SECTION_ENTRY __base,0,0,0,0,0,0
	.set	__base,__base+1
	.endr

	// Verde PMMR
	// X=0, Non-cacheable, Non-bufferable
	.rept	0x1000 - 0xFFF
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr
#endif /* CYG_HAL_MEMORY_MAP_NORMAL */


#endif /* CYG_HAL_STARTUP_ROM */

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
