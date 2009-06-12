#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	Malta board assembler header file
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
## Author(s): 	dmoseley
## Contributors:dmoseley, jskov
## Date:	2001-03-20
## Purpose:	Malta board definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the Malta board.
## Usage:
##		#include <cyg/hal/platform.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <cyg/hal/mips.inc>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/plf_io.h>

#------------------------------------------------------------------------------
# Additional bits for status register.
# We set the IM[0] bit to accept all interrupts.

#define INITIAL_SR_PLF 0x00000400

#------------------------------------------------------------------------------

#define CYGPKG_HAL_RESET_VECTOR_FIRST_CODE
	.macro hal_reset_vector_first_code
        # Branch forward past the board ID register.
        b 1f
	nop
	nop
	nop
	nop
	nop
1:
	.endm

#define CYGPKG_HAL_EARLY_INIT
	.macro	hal_early_init
	# Do these initializations early (rather than in hal_memc_init) so
	# we have access to the LEDs on the board for debugging purposes.

        #
        # WARNING: THESE WRITES NEED TO HANDLE BYTE-SWAPPING PROPERLY WHEN DOING BIG-ENDIAN
        #

	#
        # Setup Galileo CPU Interface Register
        #
        # Set the WriteRate bit - ie Accept 'DDD' back-to-back transfers (see CoreLV "Users Manual")
        # All other bits stay the same
	#
 	li	k0, CYGARC_UNCACHED_ADDRESS(HAL_GALILEO_REGISTER_BASE)
	lw	k1, HAL_GALILEO_CPU_INTERFACE_CONFIG_OFFSET(k0)
	li	k0, HAL_GALILEO_CPU_WRITERATE_MASK
	or	k1, k1, k0
	li	k0, CYGARC_UNCACHED_ADDRESS(HAL_GALILEO_REGISTER_BASE)
	sw	k1, HAL_GALILEO_CPU_INTERFACE_CONFIG_OFFSET(k0)

	#
	# Use BootCS chip-select for the entire device bus region
	#
	li	k1, 0
	sw	k1, HAL_GALILEO_CS3_HIGH_DECODE_OFFSET(k0)
	li	k1, 0xf0
	sw	k1, HAL_GALILEO_CSBOOT_LOW_DECODE_OFFSET(k0)
	li	k1, 0xff
	sw	k1, HAL_GALILEO_CSBOOT_HIGH_DECODE_OFFSET(k0)
	.endm

#------------------------------------------------------------------------------
# Interrupt decode macros

# Only INTN[0] is connected on the Malta board. We need to access the
# interrupt controller to get the actual vector number.

#ifndef CYGPKG_HAL_MIPS_INTC_INIT_DEFINED

	.macro	hal_intc_init
	mfc0	v0,status
	nop
	lui	v1,0xFFFF
	ori	v1,v1,0x04FF
	and	v0,v0,v1	# Clear the IntMask bits except IM[0]
	mtc0	v0,status
	nop
	nop
	nop
	.endm

#define CYGPKG_HAL_MIPS_INTC_INIT_DEFINED

#endif

#ifndef CYGPKG_HAL_MIPS_INTC_TRANSLATE_DEFINED
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
	# When chaining, go through trampoline for external
	# interrupts (we need to query the intc for the source)
	# but let other internal vectors go directly to the
	# chaining ISR.
	.macro	hal_intc_translate inum,vnum
	beqz    \inum,1f
         move    \vnum,\inum
	ori	\vnum,zero,CYGNUM_HAL_INTERRUPT_CHAINING
1:	
	.endm
#else			
	.macro	hal_intc_translate inum,vnum
	move	\vnum,\inum			# Vector == interrupt number
	.endm
#endif

#define CYGPKG_HAL_MIPS_INTC_TRANSLATE_DEFINED
#endif

// We also define our own interrupt tables in platform.S...
#define CYG_HAL_MIPS_ISR_TABLES_DEFINED

#------------------------------------------------------------------------------
# Diagnostic macros

#ifndef CYGPKG_HAL_MIPS_DIAG_DEFINED

	.macro	hal_diag_init
	.endm

	.macro	hal_diag_excpt_start
	.endm

	.macro	hal_diag_intr_start
	.endm

	.macro	hal_diag_restore
	.endm

#define CYGPKG_HAL_MIPS_DIAG_DEFINED

#endif // ifndef CYGPKG_HAL_MIPS_DIAG_DEFINED
	
#------------------------------------------------------------------------------
# MEMC macros.
# 
	
#if defined(CYG_HAL_STARTUP_ROM)

#------------------------------------------------------------------------------
# Make sure the jump to _start in vectors.S is done uncached
#

#define CYGARC_START_FUNC_UNCACHED

	.macro	hal_memc_init
	.extern hal_malta_init_sdram

	lar	k0,hal_malta_init_sdram
	CYGARC_ADDRESS_REG_UNCACHED(k0)
	jalr	k0
	nop

	beqz	v0, 2f
	nop

	# Error in sizing memory
 	DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS0, 'M')
 	DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS1, 'E')
 	DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS2, 'M')
 	DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS3, 'E')
 	DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS4, 'R')
 	DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS5, 'R')
 	DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS6, 'O')
 	DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS7, 'R')
1:	b	1b
	nop

2:	# No error in sizing memory

	# Store the memory size at the base of RAM for later
	la	k0, 0x80000000
        move	k1, k0
        CYGARC_ADDRESS_REG_UNCACHED(k1)
	sw	v1, 0(k1)
	nop
	nop
	nop
	.endm

#define CYGPKG_HAL_MIPS_MEMC_DEFINED

#endif	

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLATFORM_INC
# end of platform.inc
