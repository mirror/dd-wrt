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
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Date:         2002-12-08
// Purpose:      Intel XScale Generic Residential Platform specific mmu table
// Description: 
// Usage:        #include <cyg/hal/hal_platform_extras.h>
//     Only used by "vectors.S"         
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
#if defined(CYG_HAL_STARTUP_ROMRAM)
        .section .text
	.ltorg
	.p2align 13
#else
        .section .mmu_tables, "a"
#endif

    mmu_table:
        //  This page table sets up the preferred mapping:
        //
        //  Virtual Address   Physical Address  XCB  Size (MB)  Description
        //  ---------------   ----------------  ---  ---------  -----------
        //     0x00000000       0x00000000      010      32     SDRAM (cached)
        //     0x10000000       0x10000000      000      32     SDRAM (alias)
        //     0x20000000       0x00000000      000      32     SDRAM (uncached)
        //     0x48000000       0x48000000      000      64     PCI Data
        //     0x50000000       0x50000000      010      16     Flash (CS0)
        //     0x51000000       0x51000000      000     112     CS1 - CS7
	//     0x60000000       0x60000000      000      64     Queue Manager
	//     0xC0000000       0xC0000000      000       1     PCI Controller
	//     0xC4000000       0xC4000000      000       1     Exp. Bus Config
	//     0xC8000000       0xC8000000      000       1     Misc IXP425 IO
	//     0xCC000000       0xCC000000      000       1     SDRAM Config

	// 32MB SDRAM
	.set	__base,0x000
	.rept	0x020 - 0x000
	FL_SECTION_ENTRY __base,0,3,0,0,1,0
	.set	__base,__base+1
	.endr

	// 224MB Unused
	.rept	0x100 - 0x020
	.word 0
	.set	__base,__base+1
	.endr

	// 32MB SDRAM Alias
	.rept	0x120 - 0x100
	FL_SECTION_ENTRY __base,0,3,0,0,1,0
	.set	__base,__base+1
	.endr

	// 224MB Unused
	.rept	0x200 - 0x120
	.word 0
	.set	__base,__base+1
	.endr

	// 32MB SDRAM (uncached)
	.set	__base,0x000
	.rept	0x220 - 0x200
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 224MB Unused
	.set	__base,0x220
	.rept	0x300 - 0x220
	.word 0
	.set	__base,__base+1
	.endr

	// 384MB Unused
	.rept	0x480 - 0x300
	.word 0
	.set	__base,__base+1
	.endr

	// 64MB PCI Data
	.rept	0x4C0 - 0x480
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 64MB Unused
	.rept	0x500 - 0x4C0
	.word 0
	.set	__base,__base+1
	.endr

	// 16MB Flash  (Expansion bus CS0)
	.rept	0x510 - 0x500
	FL_SECTION_ENTRY __base,0,3,0,0,1,0
	.set	__base,__base+1
	.endr

	// Rest of Expansion bus (CS1-CS7)
	.rept	0x600 - 0x510
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 64MB Queue Manager
	.rept	0x640 - 0x600
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 1472MB Unused
	.rept	0xC00 - 0x640
	.word 0
	.set	__base,__base+1
	.endr
  
	// 1MB PCI Controller
	.rept	0xC01 - 0xC00
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 63MB Unused
	.rept	0xC40 - 0xC01
	.word 0
	.set	__base,__base+1
	.endr

	// 1MB Expansion bus config
	.rept	0xC41 - 0xC40
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 63MB Unused
	.rept	0xC80 - 0xC41
	.word 0
	.set	__base,__base+1
	.endr

	// 1MB Misc IO
	.rept	0xC81 - 0xC80
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 63MB Unused
	.rept	0xCC0 - 0xC81
	.word 0
	.set	__base,__base+1
	.endr

	// 1MB SDRAM Config
	.rept	0xCC1 - 0xCC0
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 63MB Unused
	.rept	0xD00 - 0xCC1
	.word 0
	.set	__base,__base+1
	.endr
                  
	// Rest is Unused
	.rept	0x1000 - 0xD00
        .word 0
	.set	__base,__base+1
	.endr

#endif /* defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM) */

/*---------------------------------------------------------------------------*/
/* end of hal_platform_extras.h                                              */
#endif /* CYGONCE_HAL_PLATFORM_EXTRAS_H */
