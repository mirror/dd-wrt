##=============================================================================
##
##      csb281.S
##
##      CSB281 board hardware setup
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2002, 2003 Gary Thomas
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):   gthomas
## Contributors:hmt
## Date:        2002-07-22
## Purpose:     CSB281 board hardware setup
## Description: This file contains any code needed to initialize the
##              hardware on a Cogent CSB281 (PowerPC 8245) board.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/hal_powerpc.h>
#include <pkgconf/hal_powerpc_csb281.h>
        
#include <cyg/hal/arch.inc>		/* register symbols et al */
#include <cyg/hal/ppc_regs.h>		/* on-chip resource layout, special */

#------------------------------------------------------------------------------

// LED macro uses r23, r25: r4 left alone
#if 0        
#define LED( x ) \
	lwi	r25,0xFA100018;         \
	lwi	r23,(x);                \
	stb	r23,0(r25)
#else
#define LED(x)                          
#endif

FUNC_START(_led)        
        lwi     r25,0xFF000000
        lwz     r26,0(r25)
        ori     r26,r26,0x18
        stw     r26,0(r25)
        blr
FUNC_END(_led)        
	
#------------------------------------------------------------------------------
                
FUNC_START( hal_hardware_init )
                
        // Basic hardware initialization
        mflr    r31
        bl      10f     // Gets position independent address of table
HW_init:        
#ifndef CYG_HAL_STARTUP_RAM
        .long   0x80000080, 0x00000000  // MSAR1 - SDRAM Bank 0 start
        .long   0x80000090, 0x0000003F  // MEAR1 - SDRAM Bank 0 end
        .long   0x800000A0, 0x00000001  // MBEN1 - SDRAM Bank 1 enable
        .long   0x800000F0, 0x88000000  // MCCR1 - SDRAM control (no GO)
        .long   0x800000F4, 0x1E00023C  // MCCR2 - Timing
        .long   0x800000F8, 0xB6000000  // MCCR3
        .long   0x800000FC, 0x35B03334  // MCCR4
        .long   0x800000F0, 0x88080000  // MCCR1 - SDRAM control (GO)
        .long   0x800000D0, 0xB4000000  // ERCR1
        .long   0x800000D4, 0xBCF7B1E3  // ERCR2
        .long   0x800000D8, 0x7000000D  // RCS2  - 0x70000000..0x71FFFFFF
        .long   0x800000DC, 0x7800000D  // RCS3  - 0x78000000..0x79FFFFFF
#endif        
        .long   0x80000078, _CSB281_EUMBBAR  // EUMBBAR - machine registers
        .long   0x800000A8, 0xFF141110  // PICR1 - RCS0 local, Big Endian, DEC/TB
        .long   0x800000AC, 0x08000000  // PICR2
        .long   0x80000070, 0x0000001A  // ODCR  - SDRAM driver control
        .long   0
10:     nop
        mflr    r3              // Pointer to initialization table
        subi    r3,r3,4
        lwi     r4,_CSB281_PCI_CONFIG_ADDR
        lwi     r5,_CSB281_PCI_CONFIG_DATA
20:     lwzu    r6,4(r3)        // Register address
        lwzu    r7,4(r3)        // Data
        cmpi    0,r6,0
        beq     30f             // end of table?
        stwbrx  r6,0,r4         // Set address
        stwbrx  r7,0,r5         // value
        b       20b
30:     mtlr    r31        
		
	# set the decrementer to maxint
	lwi	r2,0
	not	r2,r2
	mtdec	r2

#ifdef CYG_HAL_STARTUP_ROMRAM
        // Copy image from ROM to RAM
        mflr    r3              
        lwi     r4,0xFF000000
        lwi     r5,0x00FFFFFF   // ROM/FLASH base
        and     r3,r3,r5        // segment relative
        lwi     r6,_hal_hardware_init_done
        mtlr    r6
        sub     r6,r3,r6        // Absolute address
        add     r6,r6,r4        // FLASH address
        lwi     r7,0            // where to copy to
        lwi     r8,__ram_data_end
10:     lwz     r5,0(r6)
        stw     r5,0(r7)
        addi    r6,r6,4
        addi    r7,r7,4
        cmplw   r7,r8
        bne     10b
#endif

	blr
FUNC_END( hal_hardware_init )

#------------------------------------------------------------------------------
# end of csb281.S
