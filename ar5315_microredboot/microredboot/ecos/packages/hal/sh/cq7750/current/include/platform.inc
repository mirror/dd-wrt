#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	SH/CQ7750 board assembler header file
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
## Contributors:Ryozaburo Suzuki
## Date:	2000-04-18
## Purpose:	SH/CQ7750 board definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the SH/CQ7750 board.
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
        // Set up the Bus State Controller
        mova     BSC_settings_table,r0
        mov      r0,r3
1:      mov.l    @r3+,r0                // Address (or zero)
        mov      r0,r2
        cmp/eq   #0,r0
        bt       4f
        mov.l    @r3+,r1                // data
        mov.l    @r3+,r0                // byte or word or long?

        cmp/eq   #0,r0   // byte
        bt       2f

        cmp/eq   #1,r0   // word
        bt       3f

        bra      1b      // long
        mov.l    r1,@r2  // delay slot

2:      bra      1b
        mov.b    r1,@r2  // delay slot

3:      bra      1b
        mov.w    r1,@r2  // delay slot

        .align  2

BSC_settings_table:
        # FRQCR:
	.long   CYGARC_REG_FRQCR
	.long   CYGARC_REG_FRQCR_INIT
	.long	2
	
        # BCR1:
        .long   CYGARC_REG_BCR1
        .long   0x0020000c
        .long   2
        # BCR2:
        .long   CYGARC_REG_BCR2
        .long   0x35F8
        .long   1
        # WCR1:
        .long   CYGARC_REG_WCR1
        .long   0x77117121
        .long   2
        # WCR2:
        .long   CYGARC_REG_WCR2
        .long   0xFC8AE520
        .long   2
        # WCR3:
        .long   CYGARC_REG_WCR3
        .long   0x07117101
        .long   2
        # PCR:
        .long   CYGARC_REG_PCR
        .long   0x0000
        .long   1
        # RTCNT:
        .long   CYGARC_REG_RTCNT
        .long   0xa500
        .long   1
        # RTCOR:
        .long   CYGARC_REG_RTCOR
        .long   0xa53b
        .long   1
        # RFCR:
        .long   CYGARC_REG_RFCR
        .long   0xa400
        .long   1
        # RTCSR:
        .long   CYGARC_REG_RTCSR
        .long   0xa508
        .long   1
        # MCR:
        .long   CYGARC_REG_MCR
        .long   0x10192194
        .long   2
        # SDMR2:
        .long   0xFF90008c
        .long   0xaa
        .long   0
        # MCR:
        .long   CYGARC_REG_MCR
        .long   0x50192194
        .long   2
        # SDMR2:
        .long   0xFF90008c
        .long   0xaa
        .long   0
        # Table end
        .long   0
        
        .align  2
4:
        .endm

#------------------------------------------------------------------------------
# Monitor initialization.

#ifndef CYGPKG_HAL_SH_MON_DEFINED

#if	defined(CYG_HAL_STARTUP_ROM) ||			\
	(	defined(CYG_HAL_STARTUP_RAM) &&		\
		!defined(CYGSEM_HAL_USE_ROM_MONITOR))

	# If we are starting up from ROM, or we are starting in
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
