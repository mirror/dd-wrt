#ifndef CYGONCE_HAL_ARCH_INC
#define CYGONCE_HAL_ARCH_INC
##=============================================================================
##
##	arch.inc
##
##	H8/300 assembler header file
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
## Author(s): 	yoshinori sato
## Contributors:	yoshinori sato
## Date:	2002-02-14
## Purpose:	Architecture definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the H8300 CPU family.
## Usage:
##		#include <cyg/hal/arch.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
	
#include <cyg/hal/variant.inc>

##-----------------------------------------------------------------------------
## CPU specific macros. These provide a common assembler interface to
## operations that may have CPU specific implementations on different
## variants of the architecture.		

#ifndef CYGPKG_HAL_H8300_CPU_INIT_DEFINED
	# Initialize CPU
	.macro	hal_cpu_init
	# Set up the PSW
	ldc	#0xc0,ccr
	.endm
#endif

	# Enable further exception processing, and disable
	# interrupt processing.
	.macro hal_cpu_except_enable
	.endm		
	
	# Return from exception.
	.macro	hal_cpu_eret pc,sr
	.endm
	
##-----------------------------------------------------------------------------
# Default interrupt decoding macros.

#ifndef CYGPKG_HAL_H8300_INTC_DEFINED

#ifndef CYGPKG_HAL_H8300_INTC_INIT_DEFINED
	# initialize all interrupts to disabled
	.macro	hal_intc_init
	.endm
#endif

	.macro	hal_intc_decode vnum
	.endm

#endif

#------------------------------------------------------------------------------
# MMU macros.
	
#ifndef CYGPKG_HAL_H8300_MMU_DEFINED

	.macro	hal_mmu_init
	.endm

#endif	

#------------------------------------------------------------------------------
# MEMC macros.
	
#ifndef CYGPKG_HAL_H8300_MEMC_DEFINED

	.macro	hal_memc_init
	.endm

#endif	
	
#------------------------------------------------------------------------------
# Cache macros.
	
#ifndef CYGPKG_HAL_H8300_CACHE_DEFINED

	.macro	hal_cache_init
	.endm

#endif	

#------------------------------------------------------------------------------
# Diagnostics macros.
	
#ifndef CYGPKG_HAL_H8300_DIAG_DEFINED

	.macro	hal_diag_init
	.endm

	.macro	hal_diag_excpt_start
	.endm

	.macro	hal_diag_intr_start
	.endm

	.macro	hal_diag_restore
	.endm

	.macro	hal_diag_data
	.endm
#endif	

#------------------------------------------------------------------------------
# Timer initialization.
	
#ifndef CYGPKG_HAL_H8300_TIMER_DEFINED

	.macro	hal_timer_init
	.endm

#endif	

#------------------------------------------------------------------------------
# Monitor initialization.
	
#ifndef CYGPKG_HAL_H8300_MON_DEFINED

	.macro	hal_mon_init
	.endm

#endif	

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_ARCH_INC
# end of arch.inc
