#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	SuperH SH4-202 MicroDev CPU board assembler header file
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2003 Nick Garnett 
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
## Author(s): 	jskov, nickg
## Contributors:jskov, nickg
## Date:	2003-08-20
## Purpose:	SuperH SH4-202 MicroDev CPU platform startup
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the SuperH SH4-202
##              MicroDev CPU board.
## Usage:
##		#include <cyg/hal/platform.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/sh_offsets.inc>
#include <cyg/hal/sh_regs.h>


#------------------------------------------------------------------------------
# Hardware initialization.

#define END   0
#define INT8  1
#define INT16 2
#define INT32 3
#define DELAY 4
#define SPIN  5

        .macro  hal_hardware_init
        mova     init_table,r0
        mov      r0,r3
1:      mov.l    @r3+,r0                // Data type
        cmp/eq   #END,r0	        // End of table
        bf       0f
	bra      9f
	 nop
0:	cmp/eq   #INT8,r0
	bf       2f
	// 8 bit data
        mov.l    @r3+,r0                // addr
        mov.l    @r3+,r1                // data
        bra      1b
         mov.b   r1,@r0
2:	cmp/eq   #INT16,r0
	bf       3f
	// 16 bit data
        mov.l    @r3+,r0                // addr
        mov.l    @r3+,r1                // data
        bra      1b
         mov.w   r1,@r0
3:	cmp/eq   #INT32,r0
	bf       4f
	// 32 bit data
        mov.l    @r3+,r0                // addr
        mov.l    @r3+,r1                // data
        bra      1b
         mov.l   r1,@r0
4:	cmp/eq   #DELAY,r0
	bf       6f
	// delay
	mov.l    @r3+,r0                // count
5:	cmp/eq   #0,r0
	bt       1b
        bra      5b
         add     #-1,r0
6:	cmp/eq   #SPIN,r0
	bf       8f
	// delay
	mov.l    @r3+,r0                // ID
        mov      #0,r0
7:	cmp/eq   #0,r0
	bt       1b
        bra      7b
         nop
8:	bra      1b
	 nop

        .align  2
init_table:

        # Clock setup
        .long   INT16, CYGARC_REG_WTCSR, 0xA567
        .long   INT16, CYGARC_REG_WTCNT, 0x5A00
        .long   INT16, CYGARC_REG_FRQCR, CYGARC_REG_FRQCR_INIT
        .long   INT32, CYGARC_REG_FRQCR3, 0x006a

        # FEMI setup
#if 0
        # We rely on the FPGA to set up FEMI area 0
        .long   INT32, CYGARC_REG_FEMI_A0MCR, 0x0b777138
#endif
        .long   INT32, CYGARC_REG_FEMI_A1MCR, 0x0b777108
        .long   INT32, CYGARC_REG_FEMI_A2MCR, 0x03777510
        .long   INT32, CYGARC_REG_FEMI_A3MCR, 0x0b777110
        .long   INT32, CYGARC_REG_FEMI_A4MCR, 0x0b777108

#ifndef CYG_HAL_STARTUP_RAM
        # EMI setup
        .long   SPIN, 0xA5A50000
        .long   INT32, CYGARC_REG_EMI_COC, 0x00000010
        .long   DELAY, 10000
        .long   SPIN, 0xA5A50001
        .long   INT32, CYGARC_REG_EMI_MIM, 0x00000021
        .long   SPIN, 0xA5A50002
        .long   INT32, CYGARC_REG_EMI_STR, 0x0000116b
        .long   SPIN, 0xA5A50003
        .long   INT32, CYGARC_REG_EMI_SDRA0, 0x0a001900
        .long   SPIN, 0xA5A50004
        .long   INT32, CYGARC_REG_EMI_SDRA1, 0x0c001900
        .long   SPIN, 0xA5A50005
        .long   INT32, CYGARC_REG_EMI_SCR, 0x00000003
        .long   SPIN, 0xA5A50006
        .long   DELAY, 10000
        .long   SPIN, 0xA5A50007
        .long   INT32, CYGARC_REG_EMI_SCR, 0x00000002
        .long   INT32, CYGARC_REG_EMI_SCR, 0x00000004
        .long   INT32, CYGARC_REG_EMI_SCR, 0x00000004
        .long   INT32, CYGARC_REG_EMI_SCR, 0x00000004
        .long   INT32, CYGARC_REG_EMI_SCR, 0x00000004
        .long   INT32, CYGARC_REG_EMI_SCR, 0x00000004
        .long   INT32, CYGARC_REG_EMI_SCR, 0x00000004
        .long   INT32, CYGARC_REG_EMI_SCR, 0x00000004
        .long   INT32, CYGARC_REG_EMI_SCR, 0x00000004
        .long   SPIN, 0xA5A50008
        .long   INT32, CYGARC_REG_EMI_SDMR0+0x110, 0xffffffff
        .long   SPIN, 0xA5A50009
        .long   INT32, CYGARC_REG_EMI_SDMR1+0x110, 0xffffffff
        .long   SPIN, 0xA5A5000a
        .long   INT32, CYGARC_REG_EMI_MIM, 0x00900281
        .long   SPIN, 0xA5A5000b
        .long   INT32, CYGARC_REG_EMI_SCR, 0x00000000
        .long   SPIN, 0xA5A5000c
#endif

        .long   END
9:	
        .endm

#------------------------------------------------------------------------------
# Monitor initialization.

#ifndef CYGPKG_HAL_SH_MON_DEFINED

#if	!defined(CYG_HAL_STARTUP_RAM) ||		\
	(	defined(CYG_HAL_STARTUP_RAM) &&		\
		!defined(CYGSEM_HAL_USE_ROM_MONITOR))

	# If we are not starting up from RAM, or we are starting in
	# RAM and NOT using a ROM monitor, initialize the VSR table.

	.macro	hal_mon_init
        mov.l   $hal_vsr_table,r3
	# Write exception vectors
	mov.l   $cyg_hal_default_exception_vsr,r4
	mov     #CYGNUM_HAL_VSR_EXCEPTION_COUNT,r5
1:	mov.l   r4,@r3
        add     #4,r3
	dt	r5
	bf	1b
	# Write interrupt vector
	mov.l   $cyg_hal_default_interrupt_vsr,r4
	mov.l   $hal_vsr_table,r3
	add	#CYGNUM_HAL_VECTOR_INTERRUPT*4,r3
	mov.l   r4,@r3
	bra	2f
	 nop
        .align  2

	SYM_PTR_REF(cyg_hal_default_exception_vsr)
	SYM_PTR_REF(cyg_hal_default_interrupt_vsr)
	SYM_PTR_REF(hal_vsr_table)
2:
	.endm

#elif defined(CYG_HAL_STARTUP_RAM) && defined(CYGSEM_HAL_USE_ROM_MONITOR)

	# Initialize the VSR table entries
	# We only take control of the interrupt vector,
	# the rest are left to the ROM for now...

	.macro	hal_mon_init
	# Write interrupt vector
	mov.l   $hal_vsr_table,r3
	mov.l   $cyg_hal_default_interrupt_vsr,r4
	add	#CYGNUM_HAL_VECTOR_INTERRUPT*4,r3
	mov.l   r4,@r3
	bra	2f
	 nop
        .align  2

	SYM_PTR_REF(cyg_hal_default_interrupt_vsr)
	SYM_PTR_REF(hal_vsr_table)
2:
	.endm

#else

	.macro	hal_mon_init
	.endm

#endif


#define CYGPKG_HAL_SH_MON_DEFINED

#endif // CYGPKG_HAL_SH_MON_DEFINED

#endif // CYGONCE_HAL_PLATFORM_INC
