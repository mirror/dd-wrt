#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	Hitachi SE77x9 board assembler header file
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
## Author(s): 	jskov
## Contributors:jskov, gthomas
## Date:	2001-06-12
## Purpose:	Hitachi SE77x9 platform startup
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the Hitachi SE77x9
##              board.
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

        .macro  hal_hardware_init
#ifndef CYG_HAL_STARTUP_RAM
        // Set up the Bus State Controller
        mova     BSC_settings_table,r0
        mov      r0,r3
1:      mov.w    @r3+,r0                // Address (or zero)
        cmp/eq   #0,r0
        bt       2f
        mov.w    @r3+,r1                // data
        bra      1b
         mov.w    r1,@r0                // delay slot

        .align  2
BSC_settings_table:
        .word   CYGARC_REG_WTCSR, 0xA502
        .word   CYGARC_REG_WTCNT, 0x5A00
        .word   CYGARC_REG_FRQCR, CYGARC_REG_FRQCR_INIT
	# Settings from Hitachi docs for SE7709A
#ifdef CYGNUM_HAL_SH_SE77X9_SDRAM_SETUP
	.word   CYGARC_REG_BCR1,  0x0008
#else
	.word   CYGARC_REG_BCR1,  0x0810
#endif
        .word   CYGARC_REG_BCR2,  0x2ef0
        .word   CYGARC_REG_BCR3,  0x0000
        .word   CYGARC_REG_WCR1,  0x0c30
#ifdef CYGNUM_HAL_SH_SE77X9_SDRAM_SETUP
        .word   CYGARC_REG_WCR2,  0x7ddb
        .word   CYGARC_REG_MCR,   0x002c
#else
        .word   CYGARC_REG_WCR2,  0x5d5a
        .word   CYGARC_REG_MCR,   0x0055
#endif
	.word	CYGARC_REG_DCR,   0x0000
	.word	CYGARC_REG_PCR,   0x0000
#ifdef CYGNUM_HAL_SH_SE77X9_SDRAM_SETUP
	.word	CYGARC_REG_RTCOR, 0xa580
#else
	.word	CYGARC_REG_RTCOR, 0xa54e
#endif
	.word	CYGARC_REG_RFCR,  0xa400
	.word	CYGARC_REG_RTCNT, 0xa500
	.word	CYGARC_REG_RTCSR, 0xa508
        .word   0

2:
#ifdef CYGNUM_HAL_SH_SE77X9_SDRAM_SETUP
	mov.l	 $SDMR,r1		// Turns on SDRAM controller
	mov.l	 $SDMR_val,r2
	mov.b	 r2,@r1
#endif
        mova     Post_settings_table,r0
        mov      r0,r3
1:      mov.l    @r3+,r0                // Address (or zero)
        cmp/eq   #0,r0
        bt       4f
        mov.l    @r3+,r1                // data
        bra      1b
         mov.w    r1,@r0                // delay slot

	.align	4
#ifdef CYGNUM_HAL_SH_SE77X9_SDRAM_SETUP
$SDMR:	.long	0xffffe088
$SDMR_val:
	.long	0
#endif

Post_settings_table:
	.long	CYGNUM_HAL_SH_SE77X9_LEDS_BASE,0x5555	// leds
	.long	0
4:
#endif // CYG_HAL_STARTUP_RAM
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
