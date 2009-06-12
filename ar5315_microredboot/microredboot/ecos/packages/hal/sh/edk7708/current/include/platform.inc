#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	SH/EDK7708 board assembler header file
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 2003 Bart Veer
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
## Contributors:jskov
## Date:	2000-02-02
## Purpose:	SH/EDK7708 board definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the SH/EDK7708 board.
## Usage:
##		#include <cyg/hal/platform.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/sh_regs.h>
#include <cyg/hal/sh_offsets.inc>

#------------------------------------------------------------------------------
# Hardware initialization.

        .macro  hal_hardware_init
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
        # These are the settings set by the Hitachi ROM Monitor.

	.word   (CYGARC_REG_FRQCR & 0x0000FFFF)
	.word   CYGARC_REG_FRQCR_INIT

        # BCR2: Bus size of areas 1-6 to 32 bits
        .word   (CYGARC_REG_BCR2 & 0x0000FFFF)
        .word   0x3ffc
        # BCR1: Areas 2 and 3 are SDRAM
        .word   (CYGARC_REG_BCR1 & 0x0000FFFF)
        .word   0x080c
        # BCR2: Bus size of areas 1-6 to 32 bits [note: second write!]
        .word   (CYGARC_REG_BCR2 & 0x0000FFFF)
        .word   0x3ffc
        # WCR1: 3 wait-state cycles inserted for all areas
        .word   (CYGARC_REG_WCR1 & 0x0000FFFF)
        .word   0x3fff
        # WCR2: extra wait states and full pitch for burst
        .word   (CYGARC_REG_WCR2 & 0x0000FFFF)
        .word   0xffd7
        # MCR: RAS/CAS & burst timing area 2/3
        .word   (CYGARC_REG_MCR & 0x0000FFFF)
        .word   0x963c
        # RTCNT: refresh counter (needs a5 in top byte to accept write)
        .word   (CYGARC_REG_RTCNT & 0x0000FFFF)
        .word   (0xa500 | 0x0000)
        # RTCOR: refresh time constant (needs a5 in top byte to accept write)
        .word   (CYGARC_REG_RTCOR & 0x0000FFFF)
        .word   (0xa500 | 0x003b)
        # RFCR:  refresh count register (needs a4 in top byte to accept write)
        .word   (CYGARC_REG_RFCR & 0x0000FFFF)
        .word   (0xa400 | 0x0000)
        # RTCSR: refresh timer control (needs a5 in top byte to accept write)
        .word   (CYGARC_REG_RTCSR & 0x0000FFFF)
        .word   (0xa500 | 0x0008)

        # Set SDMR to 0x220
        .word    0xd880
        .word    0       
        
        # Table end
        .word   0

        .align  2
2:      
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
