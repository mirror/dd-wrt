#ifndef CYGONCE_HAL_VARIANT_INC
#define CYGONCE_HAL_VARIANT_INC
##=============================================================================
##
##	variant.inc
##
##	MPC5xx family assembler header file
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
## Author(s): 	Bob Koninckx
## Contributors:Bob Koninckx
## Date:	2001-12-15
## Purpose:	MPC5xx family definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the MPC5xx CPU family.
## Usage:
##		#include <cyg/hal/variant.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

##------------------------------------------------------------------------------
## Handle denormalized numbers etc in hardware, or use a software envelope to
## generate 100% IEEE correct results
#define FULL_IEEE_FLOATING_POINT_SUPPORT 29

#include <pkgconf/hal.h>
	
#include <cyg/hal/arch.inc>

##-----------------------------------------------------------------------------
## MPC5xx defined vectors

        .macro hal_extra_vectors
        # MPC5xx vectors
        exception_vector        software_emu
        exception_vector        reserved_01100
        exception_vector        reserved_01200
        exception_vector        instruction_tlb_error
        exception_vector        data_tlb_error
        exception_vector        reserved_01500
        exception_vector        reserved_01600
        exception_vector        reserved_01700
        exception_vector        reserved_01800
        exception_vector        reserved_01900
        exception_vector        reserved_01A00
        exception_vector        reserved_01B00
        exception_vector        data_breakpoint
        exception_vector        instruction_breakpoint
        exception_vector        peripheral_breakpoint
        exception_vector        NMI_port
        .endm
        
##-----------------------------------------------------------------------------
## MPC5xx CPU initialization
##
## Initialize CPU to a post-reset state, ensuring the ground doesn''t
## shift under us while we try to set things up.

        .macro hal_cpu_init

        # Set up MSR
        lwi     r3,CYG_MSR
        sync
        mtmsr   r3
        sync

#ifndef CYGSEM_HAL_USE_ROM_MONITOR
#ifndef CYGSEM_HAL_POWERPC_MPC5XX_OCD_ENABLE
#ifdef CYGSEM_HAL_ROM_MONITOR
        ## Do not touch the DER register if OCD support is wanted. 
        ## DER will be set with the OCD debugger in this case.
        ## Leave it untouched if co-existence with a ROM monitor
        ## is wanted as well.
        ## If we want to make a ROM monitor, we have to specify events
        ## that assert the FREEZE signal
        ## Single steps are implemented using hardware interrupts, 
        ## breakpoints use program exceptions
        lwi     r3, 0x2082000f         

#else
        ## Disable special MPC5xx "development support" for non
        ## debug loads.
        lwi     r3, 0x00000000
#endif
        mtder r3                      
#endif
#endif

#ifdef CYGSEM_HAL_POWERPC_IEEE_FLOATING_POINT
        mtfsb0 FULL_IEEE_FLOATING_POINT_SUPPORT
#else
        mtfsb1 FULL_IEEE_FLOATING_POINT_SUPPORT
#endif

        # Set up IMMR
        lwi     r3, 0x00000000
#ifdef CYGHWR_HAL_POWERPC_MPC5XX_IFLASH_ENABLE
        lwi     r4, CYGARC_REG_IMM_IMMR_FLEN
#else
        lwi     r4, 0x00000000
#endif    
        ## We load the internal memory space to address zero
        ## Board specific initialization can then relocate this
        ## afterwards
        or      r3, r3, r4
        mtspr   CYGARC_REG_IMMR, r3

        ## Disable bursts. Again, board initialization can enable this
        ## afterwards.
        li      r0, 0
        mtspr   560, r0

        .endm
        
##-----------------------------------------------------------------------------
## MPC5xx exception state handling
        .macro	hal_variant_save regs
#ifdef CYGSEM_HAL_ROM_MONITOR
        ## This is not really a save of this registers. It just clears any pending
        ## flags so that vwe can be sure that FREEZE will be negated after returning
        ## from the interrupt. Failing to do can block a number of devices. 
		mfspr r3, 148;
#endif
        .endm

        .macro	hal_variant_load regs
        .endm
        
##-----------------------------------------------------------------------------
## MPC5xx monitor initialization

#ifndef CYGPKG_HAL_PPC_MON_DEFINED

#if     defined(CYG_HAL_STARTUP_ROM) ||                 \
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
## Indicate that the ISR tables are defined in variant.S
#define CYG_HAL_PPC_ISR_TABLES_DEFINED

##-----------------------------------------------------------------------------
## MPC5xx interrupt handling.

#ifndef CYGPKG_HAL_POWERPC_INTC_DEFINED

## First level decoding of MPC5xx SIU interrupt controller.

        # decode the interrupt
	.macro  hal_intc_decode dreg,state
        lwz     \dreg,CYGARC_PPCREG_VECTOR(\state) # retrieve vector number,
        rlwinm. \dreg,\dreg,22,31,31               # isolate bit 21
        beq     0f                                 # done if decrementer (vec 0)
        lwi     \dreg,CYGARC_REG_IMM_SIVEC         # if external, get SIU
        lbz     \dreg,0(\dreg)                     # vector.
        srwi    \dreg,\dreg,2
        addi    \dreg,\dreg,1                      # Skip decrementer vector
0:      stw     \dreg,CYGARC_PPCREG_VECTOR(\state) # update vector in state frame.
        slwi    \dreg,\dreg,2                      # convert to byte offset.
        .endm                              


#define CYGPKG_HAL_POWERPC_INTC_DEFINED
#endif // CYGPKG_HAL_POWERPC_INTC_DEFINED

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VARIANT_INC
# end of variant.inc
