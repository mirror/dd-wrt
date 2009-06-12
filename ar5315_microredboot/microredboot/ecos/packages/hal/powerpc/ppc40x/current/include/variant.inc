#ifndef CYGONCE_HAL_VARIANT_INC
#define CYGONCE_HAL_VARIANT_INC
##=============================================================================
##
##    variant.inc
##
##    PPC40x family assembler header file
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
## Author(s):     jskov
## Contributors:  jskov, gthomas
## Date:          2000-08-27
## Purpose:       PPC40x family support
## Description:   This file contains various definitions and macros that are
##                useful for writing assembly code for the PPC40x CPU family.
## Usage:
##        #include <cyg/hal/variant.inc>
##        ...
##        
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
    
#undef CYG_MSR_COMMON
#define CYG_MSR_COMMON (MSR_ME | MSR_RI)

#include <cyg/hal/arch.inc>
#include <cyg/hal/plf.inc>

##-----------------------------------------------------------------------------
## PPC40x defined vectors
                
## Special code to handle the various timers which generate non-standard
## interrupts.  There are really three VSRs here, each 0x10 apart.
        .macro  timer_vector name
        .p2align 8
        .globl  __exception_\name
__exception_\name:
    # Variable timer interrupt
        mtspr   SPRG1,r3                     # stash some work registers away
        mtspr   SPRG2,r4                    
        li      r4,0x1000                    # vector offset
        b       0f
    # Fixed timer interrupt
        mtspr   SPRG1,r3                     # stash some work registers away
        mtspr   SPRG2,r4                    
        li      r4,0x1010                    # vector offset
        b       0f
    # Watchdog timer interrupt
        mtspr   SPRG1,r3                     # stash some work registers away
        mtspr   SPRG2,r4                    
        li      r4,0x1020                    # vector offset
0:      mtspr   SPRG3,r5
        lwi     r3,hal_vsr_table+(0x1000>>6) # table base
        lwz     r3,0(r3)                     # address of vsr
        mflr    r5                           # save link register
        mtlr    r3                           # put vsr address into it
        mr      r3,r4                        # original vector
        mfcr    r4                           # stash CR
        blr                                  # go to common code
        .endm

        .macro hal_extra_vectors
         timer_vector        ppc40x_timers   // 0x1000
         exception_vector    data_TLB_miss   // 0x1100
         exception_vector    instr_TLB_miss  // 0x1200
         exception_vector    reserved_01300
         exception_vector    reserved_01400
         exception_vector    reserved_01500
         exception_vector    reserved_01600
         exception_vector    reserved_01700
         exception_vector    reserved_01800
         exception_vector    reserved_01900
         exception_vector    reserved_01A00
         exception_vector    reserved_01B00
         exception_vector    reserved_01C00
         exception_vector    reserved_01D00
         exception_vector    reserved_01E00
         exception_vector    reserved_01F00
         exception_vector    ppc40x_debug
        .endm

##-----------------------------------------------------------------------------
## PPC40x CPU initialization
##
## Initialize CPU to a post-reset state, ensuring the ground doesn''t
## shift under us while we try to set things up.
        
         .macro hal_cpu_init
        # Set up MSR (disable MMU for now)
        lwi     r3,(CYG_MSR & ~(MSR_IR | MSR_DR))
        sync
        mfmsr   r4
        mtmsr   r3
        sync
        .endm

##-----------------------------------------------------------------------------
## PPC40x monitor initialization

#ifndef CYGPKG_HAL_PPC_MON_DEFINED

#if     defined(CYG_HAL_STARTUP_ROM) ||                 \
        defined(CYG_HAL_STARTUP_ROMRAM) || \
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

        # Ensure that the vectors will come from 0x0000
        lwi     r4,0
        mtspr   SPR_EVPR,r4

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
        stw     r3,CYGNUM_HAL_VECTOR_TIMERS*4(r4)
        .endm

#elif defined(CYG_HAL_STARTUP_RAM) && defined(CYGSEM_HAL_USE_ROM_MONITOR)

        # Initialize the VSR table entries
        # We only take control of the interrupt vectors,
        # the rest are left to the ROM for now...

        .macro  hal_mon_init

        # Ensure that the vectors will come from 0x0000
        lwi     r4,0
        mtspr   SPR_EVPR,r4

        lwi     r3,cyg_hal_default_interrupt_vsr
        lwi     r4,hal_vsr_table
        stw     r3,CYGNUM_HAL_VECTOR_INTERRUPT*4(r4)
        stw     r3,CYGNUM_HAL_VECTOR_DECREMENTER*4(r4)
        stw     r3,CYGNUM_HAL_VECTOR_TIMERS*4(r4)
        .endm


#else

        .macro  hal_mon_init

        .endm

#endif


#define CYGPKG_HAL_PPC_MON_DEFINED

#endif // CYGPKG_HAL_PPC_MON_DEFINED

##-----------------------------------------------------------------------------
## PPC40x exception state handling
        .macro    hal_variant_save regs
        .endm

        .macro    hal_variant_load regs
        .endm

##-----------------------------------------------------------------------------
## Indicate that the ISR tables are defined in variant.S

#define CYG_HAL_PPC_ISR_TABLES_DEFINED

##-----------------------------------------------------------------------------
## MPC4xx interrupt handling.

## Decode the EXISR appropriately

#ifndef CYGPKG_HAL_POWERPC_INTC_DEFINED
#define CYGPKG_HAL_POWERPC_INTC_DEFINED
        # decode the interrupt
        # the interrupt vector will be 0x500 or 0x10x0
        # 0x0500 = external, must be decoded via hardware (e.g. EXISR)
        # 0x10X0 = timer, X is the timer type
        # r3 used as scratch
        .macro  hal_intc_decode dreg,state
        lwz     \dreg,CYGARC_PPCREG_VECTOR(\state) # retrieve vector number,
        extrwi. \dreg,\dreg,1,19        # isolate bit 19
        beq     0f                      # timer interrupt = 1
        lwz     \dreg,CYGARC_PPCREG_VECTOR(\state) # retrieve vector number,
        extrwi  \dreg,\dreg,2,26        # isolate bits 26-27
        addi    \dreg,\dreg,CYGNUM_HAL_INTERRUPT_VAR_TIMER # FIXME
        b       1f
#if defined(CYGHWR_HAL_POWERPC_PPC4XX_403)
0:      mfdcr   r3,DCR_EXISR            # Interrupt status register
        lwi     \dreg,0x8FF0001F        # significant bits
        and     \dreg,\dreg,r3
        cntlzw  r3,\dreg                # find highest "1" bit
        slwi    r3,r3,2
        lwi     \dreg,EXISR_TAB         # convert bit # to signal #
        lwzx    \dreg,\dreg,r3
#elif defined(CYGHWR_HAL_POWERPC_PPC4XX_405) || defined(CYGHWR_HAL_POWERPC_PPC4XX_405GP)
0:      mfdcr   r3,DCR_UIC0_MSR          # Status register
        cntlzw  \dreg,r3                # find highest "1" bit
        addi    \dreg,\dreg,2
#else
#error "No interrupt handling for variant"                
#endif
1:      stw     \dreg,CYGARC_PPCREG_VECTOR(\state) # update vector in state frame.
        slwi    \dreg,\dreg,2                      # convert to byte offset.
        .endm                              
#endif

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VARIANT_INC
# end of variant.inc
