#ifndef CYGONCE_HAL_VARIANT_INC
#define CYGONCE_HAL_VARIANT_INC
##=============================================================================
##
##	variant.inc
##
##	PPC60x family assembler header file
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2002 Gary Thomas
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
## Date:	2000-02-04
## Purpose:	PPC60x family definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the PPC60x CPU family.
## Usage:
##		#include <cyg/hal/variant.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
	
#include <cyg/hal/arch.inc>
#include <cyg/hal/platform.inc>

##-----------------------------------------------------------------------------
## PPC60x defined vectors

        .macro hal_extra_vectors
        exception_vector        instruction_TLB
        exception_vector        data_load_TLB
        exception_vector        data_store_TLB
        exception_vector        instruction_address_breakpoint
        exception_vector        system_management_interrupt
        .endm

##-----------------------------------------------------------------------------
## PPC60x CPU initialization
##
## Initialize CPU to a post-reset state, ensuring the ground doesn''t
## shift under us while we try to set things up.
        
        .macro hal_cpu_init
        # Set up MSR (disable MMU for now)
        lwi     r3,(CYG_MSR & ~(MSR_IR | MSR_DR))
        sync
        mtmsr   r3
        sync
        .endm

##-----------------------------------------------------------------------------
## PPC60x monitor initialization

#ifndef CYGPKG_HAL_PPC_MON_DEFINED

#if     defined(CYG_HAL_STARTUP_ROM) ||                 \
        defined(CYG_HAL_STARTUP_ROMRAM) ||              \
        (       defined(CYG_HAL_STARTUP_RAM) &&         \
                !defined(CYGSEM_HAL_USE_ROM_MONITOR))

        .macro  hal_mon_init
#ifdef CYGSEM_HAL_POWERPC_COPY_VECTORS
        # If we are starting up from ROM and want vectors in RAM
        # or we are starting in RAM and NOT using a ROM monitor,
        # copy exception handler code to 0.
        lwi     r3,rom_vectors          # r3 = rom start
        lwi     r4,0                    # r4 = ram start
        lwi     r5,rom_vectors_end      # r5 = rom end
        cmplw   r3,r5                   # skip if no vectors
        beq     2f

        subi    r3,r3,4
        subi    r4,r4,4
        subi    r5,r5,4
1:
        lwzu    r0,4(r3)                # get word from ROM
        stwu    r0,4(r4)                # store in RAM
        cmplw   r3,r5                   # compare
        blt     1b                      # loop if not yet done
2:
#endif

        # Next initialize the VSR table. This happens whether the
        # vectors were copied to RAM or not.

        # First fill with exception handlers
        lwi     r3,cyg_hal_default_exception_vsr
        lwi     r4,hal_vsr_table
        subi    r4,r4,4
        li      r5,CYGNUM_HAL_VSR_COUNT
1:      stwu    r3,4(r4)
        subi    r5,r5,1
        cmpwi   r5,0
        bne     1b

        # Then fill in the special vectors
        lwi     r3,cyg_hal_default_interrupt_vsr
        lwi     r4,hal_vsr_table
        stw     r3,CYGNUM_HAL_VECTOR_INTERRUPT*4(r4)
        stw     r3,CYGNUM_HAL_VECTOR_DECREMENTER*4(r4)
        .endm

#elif defined(CYG_HAL_STARTUP_RAM) && defined(CYGSEM_HAL_USE_ROM_MONITOR)

        # Initialize the VSR table entries
        # We only take control of the interrupt vectors,
        # the rest are left to the ROM for now...

        .macro  hal_mon_init
        lwi     r3,cyg_hal_default_interrupt_vsr
        lwi     r4,hal_vsr_table
        stw     r3,CYGNUM_HAL_VECTOR_INTERRUPT*4(r4)
        stw     r3,CYGNUM_HAL_VECTOR_DECREMENTER*4(r4)
        .endm


#else

        .macro  hal_mon_init

        .endm

#endif


#define CYGPKG_HAL_PPC_MON_DEFINED

#endif // CYGPKG_HAL_PPC_MON_DEFINED

##-----------------------------------------------------------------------------
## PPC60x exception state handling
        .macro	hal_variant_save regs
        .endm

        .macro	hal_variant_load regs
        .endm

##-----------------------------------------------------------------------------
## Indicate that the ISR tables are defined in variant.S

#define CYG_HAL_PPC_ISR_TABLES_DEFINED

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VARIANT_INC
# end of variant.inc
