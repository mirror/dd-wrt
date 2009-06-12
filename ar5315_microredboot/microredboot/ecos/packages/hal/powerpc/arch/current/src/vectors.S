##==========================================================================
##
##      vectors.S
##
##      PowerPC exception vectors
##
##==========================================================================
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
##==========================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):    nickg, jskov
## Contributors: nickg, jskov
## Date:         1999-02-20
## Purpose:      PowerPC exception vectors
## Description:  This file defines the code placed into the exception
##               vectors. It also contains the first level default VSRs
##               that save and restore state for both exceptions and
##               interrupts.
##
######DESCRIPTIONEND####
##
##==========================================================================

#===========================================================================
#
#       The PowerPC exception handling has changed as of version 1.3.1.
#       The primary motivation for rewriting the code was to bring it more
#       in line with the other HALs, in particular to allow a RAM application
#       to cleanly take over only a subset of vectors from a running ROM
#	monitor.
#
#       GDB stubs (and CygMon, should it be ported to PPC) copies
#       exception vector entry handler code to address 0. These vector entry
#       handlers (defined by the exception_vector macro below) compute
#       a vector index into the hal_vsr_table, fetch the pointer, and
#       jump to the HAL vector service routine (VSR).
#
#       The hal_vsr_table is located immediately after the vector
#       handlers (at address 0x3000), allowing RAM applications to
#       change VSRs as necessary, while still keeping desired ROM
#       monitor functionality available for debugging.
#
#       ROM applications can still be configured to leave the vector entry
#       handlers at 0xff000000, but there is at the moment no
#       provision for reclaiming the reserved vector space in RAM to
#       application usage.
#
#       RAM applications can also be configured to provide exception
#       handlers which are copied to address 0 on startup, thus taking
#       full control of the target.
#
#
#       Default configuration is for RAM applications to rely on an
#       existing ROM monitor to provide debugging functionality, and
#       for ROM applications to copy vectors to address 0.
#
#
#       Unfortunately the new exception scheme is not compatible with the
#       old scheme. Stubs and applications must be compiled using the same
#       scheme (i.e., old binaries will not run with new stubs, and new
#       binaries will not run with old stubs).
#
#===========================================================================

#include <pkgconf/hal.h>

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>     // CYGPKG_KERNEL_INSTRUMENT
#endif

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/ppc_regs.h>

#===========================================================================
                
//        .file   "vectors.S"
        
        .extern hal_interrupt_data
        .extern hal_interrupt_handlers
        .extern hal_interrupt_objects
        .extern hal_vsr_table

        .extern cyg_hal_invoke_constructors
        .extern cyg_instrument
        .extern cyg_start
        .extern hal_IRQ_init
        .extern hal_MMU_init
        .extern hal_enable_caches
        .extern hal_hardware_init
        .extern initialize_stub

        .extern __bss_start
        .extern __bss_end
        .extern __sbss_start
        .extern __sbss_end

#===========================================================================
# MSR initialization value
# zero all bits except:
# FP = floating point available
# ME = machine check enabled
# IP = vectors at 0xFFFxxxxx (ROM startup only)
# IR = instruction address translation
# DR = data address translation
# RI = recoverable interrupt

#define CYG_MSR_COMMON (MSR_FP | MSR_ME | MSR_RI)

#if (CYGHWR_HAL_POWERPC_VECTOR_BASE == 0xfff00000)
# define IP_BIT MSR_IP
#else
# define IP_BIT 0
#endif

#ifdef CYGHWR_HAL_POWERPC_ENABLE_MMU
# define IR_DR_BITS (MSR_IR | MSR_DR)
#else
# define IR_DR_BITS 0
#endif

#define CYG_MSR (CYG_MSR_COMMON | IP_BIT | IR_DR_BITS)

# Include variant macros after MSR definition.        
#include <cyg/hal/arch.inc>
#include <cyg/hal/ppc_offsets.inc>


#===========================================================================
# If the following option is enabled, we only save registers up to R12.
# The PowerPC ABI defines registers 13..31 as callee saved and thus we do
# not need to save them when calling C functions.

#ifdef CYGDBG_HAL_COMMON_INTERRUPTS_SAVE_MINIMUM_CONTEXT
# define MAX_SAVE_REG    12
#else
# define MAX_SAVE_REG    31
#endif                  
        

#if defined(CYGHWR_HAL_POWERPC_NEED_VECTORS)

#===========================================================================
# Start by defining the exceptions vectors that must be placed at
# locations 0xFFF00000 or 0x00000000. The following code will normally
# be located at 0xFFF00000 in the ROM. It may optionally be copied out
# to 0x00000000 if we want to use the RAM vectors. For this reason this code
# MUST BE POSITION INDEPENDENT.
        
        .section ".vectors","ax"

#---------------------------------------------------------------------------
# Macros for generating an exception vector service routine

# Reset vector macro

        .macro  reset_vector name
        .p2align 8
        .globl  __exception_\name
__exception_\name:
#ifdef CYGSEM_HAL_POWERPC_RESET_USES_JUMP        
        bl      _start               
#else
        lwi     r3,_start
        mtlr    r3
        blr
#endif                

        .endm
        
# Generic vector macro
                
        .macro  exception_vector name
        .p2align 8
        .globl  __exception_\name
__exception_\name:
        mtspr   SPRG1,r3                     # stash some work registers away
        mtspr   SPRG2,r4                    
        mtspr   SPRG3,r5                    
        mfcr    r4                           # stash CR
        li      r5,__exception_\name-rom_vectors       # load low half of vector addr
        srwi    r5,r5,6                      # shift right by 6      
        lwi     r3,hal_vsr_table             # table base
        lwzx    r3,r3,r5                     # address of vsr
        mflr    r5                           # save link register
        mtlr    r3                           # put vsr address into it
        li      r3,__exception_\name-rom_vectors       # reload low half of vector addr
        blr                                  # go to common code
        .endm
        
#---------------------------------------------------------------------------
# Define the exception vectors.

// Some platforms won't let us put the vector code just where we want
// This macro introduces some lattitude in vector placement
                        
#ifdef CYG_HAL_FUDGE_VECTOR_ALIGNMENT        
        hal_fudge_vector_alignment
#endif
                
rom_vectors:
        # These are the architecture defined vectors that
        # are always present.
#ifdef CYG_HAL_RESERVED_VECTOR_00000
        hal_reserved_vector_00000                
#else                
        exception_vector        reserved_00000
#endif        
        reset_vector            reset
        exception_vector        machine_check
        exception_vector        data_storage
        exception_vector        instruction_storage
        exception_vector        external
        exception_vector        alignment
        exception_vector        program
        exception_vector        floatingpoint_unavailable
        exception_vector        decrementer
        exception_vector        reserved_00a00
        exception_vector        reserved_00b00
        exception_vector        system_call
        exception_vector        trace
        exception_vector        floatingpoint_assist
        exception_vector        reserved_00f00

        # Variants may define extra vectors.
        hal_extra_vectors

rom_vectors_end:   

#else //  CYGHWR_HAL_POWERPC_NEED_VECTORS

        # When vectors are not included this is the primary entry point.
        .globl  __exception_reset
__exception_reset:
        lwi     r3,_start
        mtlr    r3
        blr
        
#endif //  CYGHWR_HAL_POWERPC_NEED_VECTORS



#===========================================================================
# Real startup code. We jump here from the various reset vectors to set up
# the world.
        
        .text   
        .globl  _start
_start:
        # Initialize CPU to a post-reset state, ensuring the ground doesn''t
        # shift under us while we try to set things up.
        hal_cpu_init

        # Set up global offset table
        lwi     r2,_GLOBAL_OFFSET_TABLE_

        # set up time base register to zero
        xor     r3,r3,r3
        mtspr   TBL_W,r3
        xor     r4,r4,r4
        mtspr   TBU_W,r4

        # Call platform specific hardware initialization
        # This may include memory controller initialization. It is not
        # safe to access RAM until after this point.
        bl      hal_hardware_init	# this is platform dependent
        .globl  _hal_hardware_init_done
_hal_hardware_init_done:

#if !defined(CYG_HAL_STARTUP_ROM) && defined(CYGSEM_HAL_POWERPC_COPY_VECTORS)
        lwi     r3,rom_vectors-4
        lwi     r4,((CYGHWR_HAL_POWERPC_VECTOR_BASE)-4)
        lwi     r5,rom_vectors_end-4
0:      lwzu    r0,4(r3)
        stwu    r0,4(r4)
        cmplw   r3,r5
        bne     0b
#endif        

        # set up stack
        lwi     sp,__interrupt_stack
        mtspr   SPRG0,sp        # save in sprg0 for later use

        # Set up exception handlers and VSR table, taking care not to
        # step on any ROM monitor''s toes.
        hal_mon_init        

#if defined(CYG_HAL_STARTUP_ROM)

        # Copy data from ROM to ram
        lwi     r3,__rom_data_start     # r3 = rom start
        lwi     r4,__ram_data_start     # r4 = ram start
        lwi     r5,__ram_data_end       # r5 = ram end

        cmplw   r4,r5                   # skip if no data
        beq     2f
        
1:
        lwz     r0,0(r3)                # get word from ROM
        stw     r0,0(r4)                # store in RAM
        addi    r3,r3,4                 # increment by 1 word
        addi    r4,r4,4                 # increment by 1 word
        cmplw   r4,r5                   # compare
        blt     1b                      # loop if not yet done
2:
#endif

        # clear BSS
        lwi     r3,__bss_start  # r3 = start
        lwi     r4,__bss_end    # r4 = end
        li      r0,0            # r0 = 0
        cmplw   r3,r4           # skip if no bss
        beq     2f
        
1:      stw     r0,0(r3)        # store zero
        addi    r3,r3,4         # increment by 1 word
        cmplw   r3,r4           # compare
        blt     1b              # loop if not yet done
2:

        # clear SBSS
        lwi     r3,__sbss_start # r3 = start
        lwi     r4,__sbss_end   # r4 = end
        cmplw   r3,r4           # skip if no sbss
        beq     2f
        
1:      stw     r0,0(r3)        # store zero
        addi    r3,r3,4         # increment by 1 word
        cmplw   r3,r4           # compare
        blt     1b              # loop if not yet done
2:

        # It is now safe to call C functions which may rely on initialized
        # data.
        
        # Set up stack for calls to C code.
        subi    sp,sp,12                        # make space on stack
        li      r0,0
        stw     r0,0(sp)                        # clear back chain
        stw     r0,8(sp)                        # zero return pc
        stwu    sp,-CYGARC_PPC_STACK_FRAME_SIZE(sp) # create new stack frame

        # Variant HALs may need to do something special before we continue
        bl      hal_variant_init

        # Platform initialization
        bl      hal_platform_init
        
        # MMU and cache are controlled by the same option since caching
        # on the PPC [typically] does not make sense without the MMU to mark 
        # regions which should not be cached.
#ifdef CYGHWR_HAL_POWERPC_ENABLE_MMU
        # Initialize MMU.
        bl      hal_MMU_init

        # Enable MMU (if desired) so we can safely enable caches.
        lwi     r3,CYG_MSR              # interrupts enabled later
        sync
        mtmsr   r3
        sync
	
        # Enable caches
        bl      hal_enable_caches
#endif // CYGHWR_HAL_POWERPC_ENABLE_MMU

        # set up platform specific interrupt environment
        bl      hal_IRQ_init

        # call c++ constructors
        bl      cyg_hal_invoke_constructors

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        bl      initialize_stub
#endif
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
        .extern hal_ctrlc_isr_init
        bl     hal_ctrlc_isr_init
#endif
        
        bl      cyg_start                       # call cyg_start
9:      
        b       9b              # if we return, loop

#---------------------------------------------------------------------------
# This code handles the common part of all exception handlers.
# It saves the machine state onto the stack  and then calls
# a "C" routine to do the rest of the work. This work may result
# in thread switches, and changes to the saved state. When we return
# here the saved state is restored and execution is continued.
        
        .text

        .globl cyg_hal_default_exception_vsr
cyg_hal_default_exception_vsr:
        
        # We come here with all register containing their
        # pre-exception values except:
        # R3    = ls 16 bits of vector address
        # R4    = saved CR
        # R5    = saved LR
        # LR    = VSR address
        # SPRG1 = old R3
        # SPRG2 = old R4
        # SPRG3 = old R5
        # SRR0  = old PC
        # SRR1  = old MSR and the exception cause (the POW state is lost!)

        subi    sp,sp,CYGARC_PPC_EXCEPTION_DECREMENT
                                        # leave space for registers and
                                        # a safety margin
#ifdef CYGPKG_HAL_POWERPC_PPC40x
// The caches on this processor are always enabled when the MMU is on
// (and disabled when off).  Thus we need to be careful about cache
// polution and staleness when changing the MMU state.
// At this point, the MMU is off due to the exception.  We need to
// flush the part of the cache which may be touched before the MMU
// is reenabled so that memory will be consistent when that happens.
// Of course, this is complicated by the fact that there are no "free"
// registers at this point in the code.
        dcbf    0,sp                    // Flushes first line
        stw     r3,0(sp)                // This is now safe
        li      r3,CYGARC_PPCREG_VECTOR // Flush lines which will be changed
        dcbf    r3,sp                   
        li      r3,CYGARC_PPCREG_CR
        dcbf    r3,sp                   
        li      r3,CYGARC_PPCREG_LR
        dcbf    r3,sp              
        lwz     r3,0(sp)                // Restore register
#endif        

        # First, save away some registers
        stw     r3,CYGARC_PPCREG_VECTOR(sp)    # stash vector
        stw     r4,CYGARC_PPCREG_CR(sp)        # stash CR
        stw     r5,CYGARC_PPCREG_LR(sp)        # stash LR

#ifdef CYGDBG_HAL_POWERPC_FRAME_WALLS
        # Mark this fram as an Exception frame
        lwi     r3,0xDDDDDDE0
        stw     r3,CYGARC_PPCREG_WALL_HEAD(sp)
        lwi     r3,0xDDDDDDE1
        stw     r3,CYGARC_PPCREG_WALL_TAIL(sp)
#endif

        # Enable MMU & interrupt/FPU environment (as configured)
        lwi     r3,CYG_MSR
        sync
        mtmsr   r3
        sync
                
        mfspr   r3,SPRG1                # save original R3
        stw     r3,CYGARC_PPCREG_REGS+3*4(sp)
        mfspr   r4,SPRG2                # save original R4
        stw     r4,CYGARC_PPCREG_REGS+4*4(sp)
        mfspr   r5,SPRG3                # save original R5
        stw     r5,CYGARC_PPCREG_REGS+5*4(sp)

        stw     r0,CYGARC_PPCREG_REGS+0*4(sp)  # save R0
        stw     r2,CYGARC_PPCREG_REGS+2*4(sp)  # save R2
        
        mr      r3,sp                   # recreate original SP
        addi    r3,r3,CYGARC_PPC_EXCEPTION_DECREMENT
        stw     r3,CYGARC_PPCREG_REGS+1*4(sp)  # and save it in state
        
        # Save registers r6..r12/r31
        .set    _reg,6
        .rept   MAX_SAVE_REG+1-6
        stw     _reg,(CYGARC_PPCREG_REGS+_reg*4)(sp)
        .set    _reg,_reg+1
        .endr

        # Save registers used in vsr (r14+r15)
        stw     r14,(CYGARC_PPCREG_REGS+14*4)(sp)
        stw     r15,(CYGARC_PPCREG_REGS+15*4)(sp)
        
        # get remaining family CPU registers
        mfxer   r3
        mfctr   r4
        mfsrr0  r5
        mfsrr1  r6
        # and store them
        stw     r3,CYGARC_PPCREG_XER(sp)
        stw     r4,CYGARC_PPCREG_CTR(sp)
        stw     r5,CYGARC_PPCREG_PC(sp)
        stw     r6,CYGARC_PPCREG_MSR(sp)

        # Save variant registers
        hal_variant_save sp

        # Save FPU registers
        hal_fpu_save sp

        # The entire CPU state is now stashed on the stack,
        # call into C to do something with it.

        mr      r3,sp                           # R3 = register dump
        
        subi    sp,sp,CYGARC_PPC_STACK_FRAME_SIZE # make a null frame

        li      r0,0                            # R0 = 0
        stw     r0,0(sp)                        # backchain = 0
        stw     r0,8(sp)                        # return pc = 0
        
        stwu    sp,-CYGARC_PPC_STACK_FRAME_SIZE(sp) # create new stack frame
                                                # where C code can save LR

        lwi     r5,restore_state                # get return link
        mtlr    r5                              # to link register

        .extern cyg_hal_exception_handler
        b       cyg_hal_exception_handler       # call C code, r3 = registers

        # When the call returns it will go to restore_state below.


##--------------------------------------------------------------------------
## The following macros are defined depending on whether the Interrupt
## system is using isr tables or chaining, and depending on the interrupt
## controller in the system.

#ifndef CYGPKG_HAL_POWERPC_INTC_DEFINED

## This is the simple version. No interrupt controller, CYGARC_PPCREG_VECTOR 
## is updated with the decoded interrupt vector. Isr tables/chaining
## use same interrupt decoder.
## Bit 21 biffers between decrementer (0) and external (1).

        # decode the interrupt
	.macro  hal_intc_decode dreg,state
        lwz     \dreg,CYGARC_PPCREG_VECTOR(\state) # retrieve vector number,
        rlwinm  \dreg,\dreg,22,31,31               # isolate bit 21 and update
        stw     \dreg,CYGARC_PPCREG_VECTOR(\state) # vector in state frame.
        slwi    \dreg,\dreg,2                      # convert to word offset.
        .endm                              

#endif // CYGPKG_HAL_POWERPC_INTC_DEFINED

#---------------------------------------------------------------------------
# Common interrupt handling code.

        .globl cyg_hal_default_interrupt_vsr
cyg_hal_default_interrupt_vsr:

        # We come here with all register containing their
        # pre-exception values except:
        # R3    = ls 16 bits of vector address
        # R4    = saved CR
        # R5    = saved LR
        # LR    = VSR address
        # SPRG1 = old R3
        # SPRG2 = old R4
        # SPRG3 = old R5
        # SRR0  = old PC
        # SRR1  = old MSR


        subi    sp,sp,CYGARC_PPC_EXCEPTION_DECREMENT
                                        # leave space for registers and
                                        # a safety margin
#ifdef CYGPKG_HAL_POWERPC_PPC40x
// The caches on this processor are always enabled when the MMU is on
// (and disabled when off).  Thus we need to be careful about cache
// polution and staleness when changing the MMU state.
// At this point, the MMU is off due to the exception.  We need to
// flush the part of the cache which may be touched before the MMU
// is reenabled so that memory will be consistent when that happens.
// Of course, this is complicated by the fact that there are no "free"
// registers at this point in the code.
        dcbf    0,sp                    // Flushes first line
        stw     r3,0(sp)                // This is now safe
        li      r3,CYGARC_PPCREG_VECTOR // Flush lines which will be changed
        dcbf    r3,sp                   
        li      r3,CYGARC_PPCREG_CR
        dcbf    r3,sp                   
        li      r3,CYGARC_PPCREG_LR
        dcbf    r3,sp              
        lwz     r3,0(sp)                // Restore register
#endif        
              
        stw     r3,CYGARC_PPCREG_VECTOR(sp)    # stash vector
        stw     r4,CYGARC_PPCREG_CR(sp)        # stash CR
        stw     r5,CYGARC_PPCREG_LR(sp)        # stash LR

#ifdef CYGDBG_HAL_POWERPC_FRAME_WALLS
        # Mark this fram as an 1nterrupt frame
        lwi     r3,0xDDDDDD10
        stw     r3,CYGARC_PPCREG_WALL_HEAD(sp)
        lwi     r3,0xDDDDDD11
        stw     r3,CYGARC_PPCREG_WALL_TAIL(sp)
#endif

        # Enable MMU & interrupt/FPU environment as configured
        lwi     r3,CYG_MSR
        sync
        mtmsr   r3
        sync

        mfspr   r3,SPRG1                # save original R3
        stw     r3,CYGARC_PPCREG_REGS+3*4(sp)
        mfspr   r4,SPRG2                # save original R4
        stw     r4,CYGARC_PPCREG_REGS+4*4(sp)
        mfspr   r5,SPRG3                # save original R5
        stw     r5,CYGARC_PPCREG_REGS+5*4(sp)

        stw     r0,CYGARC_PPCREG_REGS+0*4(sp)  # save R0
        stw     r2,CYGARC_PPCREG_REGS+2*4(sp)  # save R2

        mr      r3,sp                   # recreate original SP
        addi    r3,r3,CYGARC_PPC_EXCEPTION_DECREMENT
        stw     r3,CYGARC_PPCREG_REGS+1*4(sp)  # and save it in state
        
        # Save registers r6..r12/r31
        .set    _reg,6
        .rept   MAX_SAVE_REG+1-6
        stw     _reg,(CYGARC_PPCREG_REGS+_reg*4)(sp)
        .set    _reg,_reg+1
        .endr

        # Save registers used in vsr (r14+r15)
        stw     r14,CYGARC_PPCREG_REGS+14*4(sp)
        stw     r15,CYGARC_PPCREG_REGS+15*4(sp)

        # get remaining family CPU registers
        mfxer   r3
        mfctr   r4
        mfsrr0  r5
        mfsrr1  r6

        # and store them
        stw     r3,CYGARC_PPCREG_XER(sp)
        stw     r4,CYGARC_PPCREG_CTR(sp)
        stw     r5,CYGARC_PPCREG_PC(sp)
        stw     r6,CYGARC_PPCREG_MSR(sp)
                
        # Save variant registers
        hal_variant_save sp

        # Save FPU registers
        hal_fpu_save sp

        # The entire CPU state is now stashed on the stack,
        # increment the scheduler lock and call the ISR
        # for this vector.

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT                 
        .extern cyg_scheduler_sched_lock
        lwi     r3,cyg_scheduler_sched_lock
        lwz     r4,0(r3)
        addi    r4,r4,1
        stw     r4,0(r3)
#endif
        
        mr      r14,sp                          # r14 = register dump
        
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK 
        lwi     r3,__interrupt_stack            # stack top
        lwi     r4,__interrupt_stack_base       # stack base
        sub.    r5,sp,r4                        # sp - base
        blt     1f                              # if < 0 - not on istack
        sub.    r5,r3,sp                        # top - sp
        bgt     2f                              # if > 0 - already on istack

1:      mr      sp,r3                           # switch to istack

2:      stwu    r14,-4(sp)                      # save old SP on stack

#endif
        
        subi    sp,sp,CYGARC_PPC_STACK_FRAME_SIZE # make a null frame

        li      r0,0                            # R0 = 0
        stw     r0,0(sp)                        # backchain = 0
        stw     r0,8(sp)                        # return pc = 0
        
        stwu    sp,-CYGARC_PPC_STACK_FRAME_SIZE(sp) # create new stack frame
                                                # where C code can save LR
        
#if defined(CYGPKG_KERNEL_INSTRUMENT) && defined(CYGDBG_KERNEL_INSTRUMENT_INTR)

        lwi     r3,0x0301                       # r3 = type = INTR,RAISE
        lwz     r4,CYGARC_PPCREG_VECTOR(r14)    # arg1 = vector address
        srwi    r4,r4,8                         # arg1 = vector number
        xor     r5,r5,r5                        # arg2 = 0
        bl      cyg_instrument                  # call instrument function
        
#endif

        hal_intc_decode r15,r14                # get table index

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
	# If we are supporting Ctrl-C interrupts from GDB, we must squirrel
	# away a pointer to the save interrupt state here so that we can
	# plant a breakpoint at some later time.
	
	.extern	hal_saved_interrupt_state
	lwi	r3,hal_saved_interrupt_state
	stw	r14,0(r3)
	
#endif

#ifdef CYGSEM_HAL_COMMON_INTERRUPTS_ALLOW_NESTING

#ifdef CYGPKG_HAL_POWERPC_MPC8xx
        # The CPM controller allows nested interrupts. However,
        # it sits on the back of the SIU controller which has no
        # HW support for this. In effect, SW masking of lower
        # priority IRQs in the SIU would be required for this to work.
#endif

#endif

        lwz     r3,CYGARC_PPCREG_VECTOR(r14)    # retrieve decoded vector #

        lwi     r6,hal_interrupt_handlers       # get interrupt handler table
        lwzx    r6,r6,r15                       # load routine pointer

        lwi     r4,hal_interrupt_data           # get interrupt data table
        lwzx    r4,r4,r15                       # load data pointer
                                                # R4 = data argument
        mr      r5,r14                          # R5 = saved registers        
        
        mtctr   r6                              # put isr address in ctr

        bctrl                                   # branch to ctr reg and link

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

        # If we are returning from the last nested interrupt, move back
        # to the thread stack. interrupt_end() must be called on the
        # thread stack since it potentially causes a context switch.
        # Since we have arranged for the top of stack location to
        # contain the sp we need to go back to here, just pop it off
        # and put it in SP.

        
        lwz     sp,CYGARC_PPC_STACK_FRAME_SIZE*2(sp) # sp = *sp

        subi    sp,sp,CYGARC_PPC_STACK_FRAME_SIZE # make a null frame

        li      r0,0                            # R0 = 0
        stw     r0,0(sp)                        # backchain = 0
        stw     r0,8(sp)                        # return pc = 0
        
        stwu    sp,-CYGARC_PPC_STACK_FRAME_SIZE(sp) # create new stack frame
                                                # where C code can save LR
#endif  

        # On return r3 bit 1 will indicate whether a DSR is
        # to be posted. Pass this together with a pointer to
        # the interrupt object we have just used to the
        # interrupt tidy up routine.

        # Note that r14 and r15 are defined to be preserved across
        # calls by the calling convention, so they still contain
        # the register dump and the vector number respectively.

        lwi     r4,hal_interrupt_objects        # get interrupt object table
        lwzx    r4,r4,r15                       # load object pointer
        mr      r5,r14                          # arg3 = saved register dump

        .extern interrupt_end
        bl      interrupt_end                   # call into C to finish off 

restore_state:  
        # All done, restore CPU state and continue

        # retrieve CPU state pointer
        addi    sp,sp,CYGARC_PPC_STACK_FRAME_SIZE*2

        # Restore FPU registers
        hal_fpu_load sp

        # Restore variant registers
        hal_variant_load sp

        # get sprs we want to restore
        # stuff some of them into the CPU
        lwz     r3,CYGARC_PPCREG_XER(sp)
        lwz     r4,CYGARC_PPCREG_LR(sp)
        lwz     r5,CYGARC_PPCREG_CTR(sp)
        mtxer   r3
        mtlr    r4
        mtctr   r5

        # Restore registers used in vsr (r14+r15)
        lwz     r14,CYGARC_PPCREG_REGS+14*4(r1)
        lwz     r15,CYGARC_PPCREG_REGS+15*4(r1)

        # restore registers r6..r12/r31
        .set    _reg,6
        .rept   MAX_SAVE_REG+1-6
        lwz     _reg,(CYGARC_PPCREG_REGS+_reg*4)(sp)
        .set    _reg,_reg+1
        .endr

        hal_cpu_int_disable
        
        # restore R0 and R2
        lwz     r0,CYGARC_PPCREG_REGS+0*4(sp)
        lwz     r2,CYGARC_PPCREG_REGS+2*4(sp)

        # Here all the registers are loaded except
        # sp = HAL_SavedRegisters
        # r3 = ccr
        # r4 = srr0 = pc
        # r5 = srr1 = msr
        #
        # We have to disable interrupts while srr0 and
        # srr1 are loaded, since another interrupt will
        # destroy them.

        lwz     r3,CYGARC_PPCREG_CR(sp)
        lwz     r4,CYGARC_PPCREG_PC(sp)
        lwz     r5,CYGARC_PPCREG_MSR(sp)
        mtcr    r3                      # set ccr
        mtsrr0  r4                      # load old pc
        mtsrr1  r5                      # load old msr
        
#ifdef CYGDBG_HAL_POWERPC_FRAME_WALLS
        # Mark this frame as (almost) dead.
        lwi     r3,0xDDDDDDD0
        stw     r3,CYGARC_PPCREG_WALL_HEAD(sp)
        lwi     r3,0xDDDDDDD1
        stw     r3,CYGARC_PPCREG_WALL_TAIL(sp)
#endif

        lwz     r3,CYGARC_PPCREG_REGS+3*4(sp)  # load r3 value
        lwz     r4,CYGARC_PPCREG_REGS+4*4(sp)  # load r4 value
        lwz     r5,CYGARC_PPCREG_REGS+5*4(sp)  # load r5 value
        lwz     sp,CYGARC_PPCREG_REGS+1*4(sp)  # restore sp
                
        sync                            # settle things down
        isync   
        rfi                             # and return



##-----------------------------------------------------------------------------
## Execute pending DSRs on the interrupt stack with interrupts enabled.
## Note: this can only be called from code running on a thread stack

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
	.extern cyg_interrupt_call_pending_DSRs

FUNC_START(hal_interrupt_stack_call_pending_DSRs)
        # Change to interrupt stack, save state and set up stack for
        # calls to C code.
        mr      r3,sp
        lwi     r4,__interrupt_stack
        subi    r4,r4,24                        # make space on stack
        mr      sp,r4
        stw     r3,12(sp)                       # save old sp
        mfmsr   r3
        stw     r3,16(sp)                       # save old MSR
        mflr    r3
        stw     r3,20(sp)                       # save old LR

        li      r0,0
        stw     r0,0(sp)                        # clear back chain
        stw     r0,8(sp)                        # zero return pc

        hal_cpu_int_enable

        # Call into kernel which will execute DSRs
        stwu    sp,-CYGARC_PPC_STACK_FRAME_SIZE(sp)
        bl      cyg_interrupt_call_pending_DSRs
        addi    sp,sp,CYGARC_PPC_STACK_FRAME_SIZE

        lwz     r3,20(sp)                       # restore LR
        mtlr    r3
        lwz     r5,12(sp)                       # get SP from saved state
        lwz     r3,16(sp)                       # restore interrupt setting
        hal_cpu_int_merge r3

        mr      sp,r5                           # restore stack pointer
        blr                                     # and return to caller
#endif		

#---------------------------------------------------------------------------
## Temporary interrupt stack
        
        .section ".bss"

	.balign 16
	.global cyg_interrupt_stack_base
cyg_interrupt_stack_base:
__interrupt_stack_base:
	.rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
	.byte 0
	.endr
	.balign 16
	.global cyg_interrupt_stack
cyg_interrupt_stack:
__interrupt_stack:
        
        .long   0,0,0,0,0,0,0,0 

#---------------------------------------------------------------------------
# end of vectors.S
