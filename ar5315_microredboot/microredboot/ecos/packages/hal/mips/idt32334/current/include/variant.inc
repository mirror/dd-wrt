#ifndef CYGONCE_HAL_VARIANT_INC
#define CYGONCE_HAL_VARIANT_INC
##=============================================================================
##
##	variant.inc
##
##	IDT32334 family assembler header file
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
## Author(s): 	 tmichals
## Contributors: nickg
## Date:	 2003-02-13
## Purpose:	 MIPS IDT32334 family definitions.
## Description:	 This file contains various definitions and macros that are
##               useful for writing assembly code for the MIPS IDT32334 CPU family.
## Usage:
##		 #include <cyg/hal/variant.inc>
##		 ...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/mips.inc>
#include <cyg/hal/platform.inc>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#ifndef __ASSEMBLER__
#define __ASSEMBLER__
#endif
//#include <cyg/hal/mips-regs.h>

#define CYGARC_ADDRESS_REG_UNCACHED(reg)               \
        and     reg, reg, ~CYGARC_KSEG_MASK;           \
        or      reg, reg, CYGARC_KSEG_UNCACHED

#define CYGARC_KSEG_MASK                               (0xE0000000)
#define CYGARC_KSEG_CACHED                             (0x80000000)
#define CYGARC_KSEG_UNCACHED                           (0xA0000000)
#define CYGARC_KSEG_CACHED_BASE                        (0x80000000)
#define CYGARC_KSEG_UNCACHED_BASE                      (0xA0000000)



//-----------------------------------------------------------------------------
// Load Address and Relocate. This macro is used in code that may be
// linked to execute out of RAM but is actually executed from ROM. The
// code that initializes the memory controller and copies the ROM
// contents to RAM must work in this way, for example. This macro is used
// in place of an "la" macro instruction when loading code and data
// addresses.  There are two versions of the macro here. The first
// assumes that we are executing in the ROM space at 0xbfc00000 and are
// linked to run in the RAM space at 0x80000000.  It simply adds the
// difference between the two to the loaded address.  The second is more
// code, but will execute correctly at either location since it
// calculates the difference at runtime.  The second variant is enabled
// by default.


#ifdef CYG_HAL_STARTUP_ROMRAM

	.macro	lar	reg,addr
.set	noat
	move	$at,ra			# save ra
	la	\reg,\addr		# get address into register
	la	ra,x\@			# get linked address of label
	sub	\reg,\reg,ra		# subtract it from value
	bal	x\@			# branch and link to label
	nop				#  to get current actual address
x\@:
	add	\reg,\reg,ra		# add actual address
	move	ra,$at			# restore ra
.set	at
	.endm
	
#define	CYGPKG_HAL_MIPS_LAR_DEFINED

#endif


#------------------------------------------------------------------------------
# Cache macros.
        
#ifndef CYGPKG_HAL_MIPS_CACHE_DEFINED

        .macro  hal_cache_init

	# Setup a temporary stack pointer for running C code.
	la	a0,__interrupt_stack
	move	sp,a0
        CYGARC_ADDRESS_REG_UNCACHED(sp)
        
        # Read the CONFIG1 register into a0
#       mfc0	a0, C0_CONFIG
	nop
	nop
	nop

	# Jump to C-code to initialize caches (uncached)
	lar	k0, hal_c_cache_init
	CYGARC_ADDRESS_REG_UNCACHED(k0)
	jalr	k0
	nop
        .endm

#define CYGPKG_HAL_MIPS_CACHE_DEFINED

#endif  


##-----------------------------------------------------------------------------
## Define CPU variant for architecture HAL.
			

##-----------------------------------------------------------------------------
## Indicate that the ISR tables are defined in variant.S

#define CYG_HAL_MIPS_ISR_TABLES_DEFINED

##-----------------------------------------------------------------------------
## 

#ifndef CYGPKG_HAL_MIPS_MEMC_DEFINED

## ROM timing characteristics are dependent on the clock speed.
		

	.macro	hal_memc_init
	.endm



#define CYGPKG_HAL_MIPS_MEMC_DEFINED
		
#endif	
	
##-----------------------------------------------------------------------------
## IDT32334 interrupt handling.

#ifndef CYGPKG_HAL_MIPS_INTC_DEFINED



	# Set all ILRX registers to 0, masking all external interrupts.
	.macro	hal_intc_init
	.endm

	.macro	hal_intc_decode vnum
	mfc0	v1,cause 
	nop
	mfc0	v0,status
	nop
	and	v1,v0,v1
			
	srl	v1,v1,11		        # shift IP bits to ls bits
	andi	v1,v1,0x7F			# isolate IP bits
		
	la	v0,hal_intc_translation_table	# address of translation table
	add	v0,v0,v1			# offset of index byte
	lb	\vnum,0(v0)			# load it
	.endm

#ifndef CYGPKG_HAL_MIPS_INTC_TRANSLATE_DEFINED
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
	.macro	hal_intc_translate inum,vnum
	move	\vnum,zero			# Just vector zero is supported
	.endm
#else			
	.macro	hal_intc_translate inum,vnum
	move	\vnum,\inum			# Vector == interrupt number
	.endm
#endif
#endif

# This table translates from the 6 bit value supplied in the IP bits
# of the cause register into a 0..16 offset into the ISR tables.
	.macro	hal_intc_decode_data
hal_intc_translation_table:	

	.byte	0,1,0,0,3,3,0,0
	.byte   0,0,0,0,0,0,0,0
	.byte   5,5,0,0,3,5,0,0
	.byte   0,0,0,0,0,0,0,0
	.byte	0,0,0,0,0,0,0,0
	.byte   0,0,0,0,0,0,0,0
	.byte   0,0,0,0,0,0,0,0
	.byte   0,0,0,0,0,0,0,0
	
	.endm

#define CYGPKG_HAL_MIPS_INTC_DEFINED

#endif

#------------------------------------------------------------------------------
# Diagnostics macros.
	
#if 0	
#ifndef CYGPKG_HAL_MIPS_DIAG_DEFINED

	# Set up PIO0 for debugging output
	.macro	hal_diag_init
	.endm

#define CYGPKG_HAL_MIPS_DIAG_DEFINED
		
#endif	
#endif

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VARIANT_INC
# end of variant.inc
