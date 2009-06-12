##==========================================================================
##
##      vectors.S
##
##      SH exception vectors
##
##==========================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2003 Nick Garnett 
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
## Author(s):    jskov
## Contributors: jskov, gthomas, nickg
## Date:         1999-05-01
## Purpose:      SH exception vectors
## Description:  This file defines the code placed into the exception
##               vectors. It also contains the first level default VSRs
##               that save and restore state for both exceptions and
##               interrupts.
##
######DESCRIPTIONEND####
##
##==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_sh.h>

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>     // CYGPKG_KERNEL_INSTRUMENT
#endif
#include CYGHWR_MEMORY_LAYOUT_H
        
#include <cyg/hal/arch.inc>
#include <cyg/hal/variant.inc>
#include <cyg/hal/sh_regs.h>
#include <cyg/hal/sh_offsets.inc>

#===========================================================================

//        .file   "vectors.S"

#define n__DEBUG
        
#===========================================================================
# Start by defining the exceptions vectors.

        .section ".vectors","ax"
# Include exception entry code since it exists in two variants,
# depending on the CPU model. This file also defines macros used
# for exception return.

FUNC_START(_vector_code_vma)

#include CYGBLD_HAL_VAR_EXCEPTION_MODEL_INC

#---------------------------------------------------------------------------
# This code handles the common part of all exception handlers.
# It saves the machine state onto the stack  and then calls
# a "C" routine to do the rest of the work. This work may result
# in thread switches, and changes to the saved state. When we return
# here the saved state is restored and execution is continued.

FUNC_START(cyg_hal_default_exception_vsr)
	hal_cpu_save_regs
	hal_exception_entry_extras
#ifdef __DEBUG
        mov.l   1f,r0
        mov.l   r0,@-r15
	bra	2f
	 nop
	.align  2
1:      .long   0x77777770
2:
        # It is safe to use breakpoints below this point.
        .globl  _cyg_hal_default_exception_vsr_bp_safe
_cyg_hal_default_exception_vsr_bp_safe:
#endif

	# Make sure the saved registers structure contain
	# the decoded exception number
	hal_exception_translate

        mov      r15,r4                 ! R4 = register dump
        
#if (defined(CYGSEM_HAL_ROM_MONITOR) || defined(CYGPKG_REDBOOT)) && \
    defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK)

        mov.l   $cyg_interrupt_stack_base,r0
        mov.l   $cyg_interrupt_stack,r1
        cmp/hi  r15,r0                  ! if r0 > r15 or
        bt      2f
        cmp/hi  r1,r15                  ! if r15 > r1
        bf      1f
2:      mov     r1,r15                  ! change to supervisor stack
1:      mov.l   r4,@-r15                ! save old stack pointer

#endif
        
        # The entire CPU state is now stashed on the stack,
        # call into C to do something with it.

        mov.l   $cyg_hal_exception_handler,r0
        jsr     @r0                     ! call C code, r4 = registers
         nop

#if (defined(CYGSEM_HAL_ROM_MONITOR) || defined(CYGPKG_REDBOOT)) && \
    defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK)

        # If we are returning from the last nested exception, move back
        # to the thread stack. 
        # Since we have arranged for the top of stack location to
        # contain the sp we need to go back to here, just pop it off
        # and put it in SP.

        mov.l   @r15,r15
#endif
        
        bra     restore_state
         nop
        
        .align  2
        SYM_PTR_REF(cyg_hal_exception_handler)

#---------------------------------------------------------------------------
# Common interrupt handling code.

FUNC_START(cyg_hal_default_interrupt_vsr)
	hal_cpu_save_regs
	hal_interrupt_entry_extras
	
#ifdef __DEBUG
        mov.l   1f,r0
        mov.l   r0,@-r15
 	bra	2f
	 nop
	.align  2
1:	.long   0x77777771
2:
        # It is safe to use breakpoints below this point.
        .globl  _cyg_hal_default_interrupt_vsr_bp_safe
_cyg_hal_default_interrupt_vsr_bp_safe:
#endif

        # The entire CPU state is now stashed on the stack,
        # increment the scheduler lock and call the ISR
        # for this vector.

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT                 
        mov.l   $cyg_scheduler_sched_lock,r0
        mov.l   @r0,r1
        add     #1,r1
        mov.l   r1,@r0
#endif

        mov     r15,r8                  ! R8 = register dump

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK 

        mov.l   $cyg_interrupt_stack_base,r0
        mov.l   $cyg_interrupt_stack,r1
        cmp/hi  r15,r0                  ! if r0 > r15 or
        bt      2f
        cmp/hi  r1,r15                  ! if r15 > r1
        bf      1f
2:      mov     r1,r15                  ! change to supervisor stack
1:      mov.l   r8,@-r15                ! save old stack pointer

#endif

#if defined(CYGPKG_KERNEL_INSTRUMENT) && defined(CYGDBG_KERNEL_INSTRUMENT_INTR)
        mov.l   $n0301,r4               ! arg0 = type = INTR,RAISE
        mov     r7,r5                   ! arg1 = vector number
        mov.l   $cyg_instrument,r0
        mov     #0,r6                   ! arg2 = 0
        jsr      @r0                    ! call instrument function
         nop
        bra     1f
         nop

        .align  2
$n0301: 
        .long   0x0301
        SYM_PTR_REF(cyg_instrument)
1:
#endif

        # Decode the interrupt vector, and find ISR index
        mov     #CYGARC_SHREG_EVENT,r0
        mov.l   @(r0,r8),r4             ! load existing vector number
        hal_intc_decode r1,r4
        mov.l   r4,@(r0,r8)             ! store decoded vector number back
                                        ! to saved state.
        hal_intc_translate r4,r9

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
        # If we are supporting Ctrl-C interrupts from GDB, we must squirrel
        # away a pointer to the save interrupt state here so that we can
        # plant a breakpoint at some later time.
        
        mov.l   $hal_saved_interrupt_state,r1
        mov.l   r8,@r1
#endif

#ifdef CYGSEM_HAL_COMMON_INTERRUPTS_ALLOW_NESTING

#if defined(CYGARC_SH_SOFTWARE_IP_UPDATE)
        # The interrupt mask bits in the SR are not updated by the
        # CPU. Proper nested operation requires the level to be
        # found and put in the SR.

        # R4 contains the vector number:
        # CYGNUM_HAL_INTERRUPT_NMI:     
        #  Ix = 15
        # CYGNUM_HAL_INTERRUPT_LVL0-CYGNUM_HAL_INTERRUPT_LVL14:
        #  Ix = 15-(CYGNUM_HAL_INTERRUPT_LVL0-R4)
        # IRA sources:
        #  Get level from IRA
        # IRB sources:
        #  Get level from IRB

        # However, doing mutiple checks and branches here is not smart.
        # Instead rely on alternative implementation where all programmed
        # priorities are also kept in a table.

        mov.l   $cyg_hal_ILVL_table,r0
        mov.b   @(r0,r4),r0
        shll2   r0
        shll2   r0
        mov.l   $unmasked_SR,r1
        or      r0,r1
        ldc     r1,sr
#endif

#endif

        mov     r9,r0
        mov.l   $hal_interrupt_handlers,r1 ! get interrupt handler
        mov.l   @(r0,r1),r1

        mov.l   $hal_interrupt_data,r5 ! get interrupt data
        mov.l   @(r0,r5),r5

        jsr     @r1                     ! r4=vector, r5=data
         nop

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

        # If we are returning from the last nested interrupt, move back
        # to the thread stack. interrupt_end() must be called on the
        # thread stack since it potentially causes a context switch.
        # Since we have arranged for the top of stack location to
        # contain the sp we need to go back to here, just pop it off
        # and put it in SP.

        mov.l   @r15,r15
#endif

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT

        # We only need to call _interrupt_end() when there is a kernel
        # present to do any tidying up.
        
        # on return r0 bit 1 will indicate whether a DSR is
        # to be posted. Pass this together with a pointer to
        # the interrupt object we have just used to the
        # interrupt tidy up routine.
        mov     r0,r4                   ! arg1 = isr_ret

        # Note that r8 and r9 are defined to be preserved across
        # calls by the calling convention, so they still contain
        # the register dump and the vector table index respectively.

        mov.l   $hal_interrupt_objects,r0  ! get interrupt object table
        mov.l   @(r0,r9),r5             ! arg2 = interrupt object
        mov.l   $interrupt_end,r0
        mov     r8,r6                   ! arg3 = saved register dump
        jsr     @r0                     ! call into C to finish off
         nop
#endif

restore_state:  
        # All done, restore CPU state and continue
#ifdef __DEBUG
	# skip past debug marker
	add     #4,sp
#endif

	hal_cpu_restore_regs_return

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
##-----------------------------------------------------------------------------
## Execute pending DSRs on the interrupt stack with interrupts enabled.
## Note: this can only be called from code running on a thread stack
FUNC_START(hal_interrupt_stack_call_pending_DSRs)
        # Change to interrupt stack
        mov     r15,r2
        mov.l   $cyg_interrupt_stack,r15

        mov.l   r2,@-r15                ! save old stack pointer
        sts.l   pr,@-r15                ! save pr on stack
        stc     sr,r3
        mov.l   r3,@-r15                ! save sr on stack

        # enable interrupts
        hal_cpu_int_enable r0,r1

        # Call into kernel which will execute DSRs
        mov.l   $cyg_interrupt_call_pending_DSRs,r0
        jsr     @r0
         nop

        # Get old sr, pr, and stack values
        mov.l   @r15+,r3                ! get old sr
        lds.l   @r15+,pr                ! get old pr
        mov.l   @r15+,r15               ! get old stack pointer

        # Restore SR interrupt state
        hal_cpu_int_merge r3,r0,r1
        rts
         nop

#endif // CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
#endif // CYGFUN_HAL_COMMON_KERNEL_SUPPORT

        .align  2        

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
        SYM_PTR_REF(hal_saved_interrupt_state)
#endif

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK 
        SYM_PTR_REF(cyg_interrupt_stack_base)
        SYM_PTR_REF(cyg_interrupt_stack)
#endif
        SYM_PTR_REF(hal_interrupt_handlers)
        SYM_PTR_REF(hal_interrupt_data)
        SYM_PTR_REF(hal_interrupt_objects)

#ifdef CYGSEM_HAL_COMMON_INTERRUPTS_ALLOW_NESTING
        SYM_PTR_REF(cyg_hal_ILVL_table)
$unmasked_SR:
        .long   (CYG_SR & ~CYGARC_REG_SR_IMASK)
#endif // CYGSEM_HAL_COMMON_INTERRUPTS_ALLOW_NESTING
        
        
#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
        SYM_PTR_REF(cyg_interrupt_call_pending_DSRs)
        SYM_PTR_REF(interrupt_end)
        SYM_PTR_REF(cyg_scheduler_sched_lock)
#endif

#---------------------------------------------------------------------------
# Platform initialization (reset)
FUNC_START(_reset_platform)
        hal_post_reset_init     

#ifdef __DEBUG
        mov     #0,r0
        mov     #1,r1
        mov     #2,r2
        mov     #3,r3
        mov     #4,r4
        mov     #5,r5
        mov     #6,r6
        mov     #7,r7
        mov     #8,r8
        mov     #9,r9
        mov     #10,r10
        mov     #11,r11
        mov     #12,r12
        mov     #13,r13
        mov     #14,r14
#endif
       
        # Call platform specific hardware initialization
        # This may include memory controller initialization. It is not
        # safe to access RAM until after this point.
        # Note that caches must not be enabled until after this point,
        # since we may be fiddling the FRQCR which cannot be safely done
        # by code in burst/cachable memory (errata SH7-184e).
        hal_hardware_init

        # Now copy necessary bits to RAM and jump to the VMA base

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_SUBSETROMRAM)

        # Copy data from ROM to ram
        mov.l   $_rom_data_start,r3    ! r3 = rom start
        mov.l   $_ram_data_start,r4    ! r4 = ram start
        mov.l   $_ram_data_end,r5      ! r5 = ram end
        
        cmp/eq  r4,r5                   ! skip if no data
        bt      2f
1:      mov.l   @r3+,r0                 ! get word from ROM
        mov.l   r0,@r4                  ! store in RAM
        add     #4,r4
        cmp/eq  r4,r5                   ! compare
        bf      1b                      ! loop if not yet done
2:


        # Jump to the proper VMA base of the code.
        mov.l   $complete_setup,r0
        jmp     @r0
         nop
        .align 2

        SYM_PTR_REF(_rom_data_start)
        SYM_PTR_REF(_ram_data_start)
        SYM_PTR_REF(_ram_data_end)
        SYM_PTR_REF(complete_setup)


#elif defined(CYG_HAL_STARTUP_ROMRAM)

        # Copy everything to the proper VMA base and jump to it.
        mov.l   $_vector_code_lma,r0
        mov.l   $_vector_code_vma,r1
        mov.l   $end,r2
1:      mov.l   @r0+,r3                 ! get word from ROM
        mov.l   r3,@r1                  ! store in RAM
        add     #4,r1
        cmp/eq  r1,r2                   ! compare
        bf      1b                      ! loop if not yet done
        mov.l   $complete_setup,r0
        jmp     @r0
         nop
        .align  2

        SYM_PTR_REF(_vector_code_lma)
        SYM_PTR_REF(_vector_code_vma)
        SYM_PTR_REF(end)
        SYM_PTR_REF(complete_setup)

#else

        # Jump to remaining setup code. Relative branch is OK since VMA=LMA.
        bra   CYG_LABEL_DEFN(complete_setup)
         nop
                
#endif

#-----------------------------------------------------------------------------
# Complete target initialization and setup.
# After this point we can use absolute addressing modes and access all the
# memory in the system.

	.align  2
FUNC_START(complete_setup)
                
        # Set up monitor related stuff (vectors primarily)
        hal_mon_init

        # set up stack
        mov.l    $startup_stack,r15

        # clear BSS
        mov.l   $_bss_start,r3         ! r3 = start
        mov.l   $_bss_end,r4           ! r4 = end
        mov     #0,r0                   ! r0 = 0
1:      cmp/eq  r3,r4                   ! skip if no bss
        bt      2f
        mov.l   r0,@r3                  ! store zero
        add     #4,r3
        bra     1b                      ! loop
         nop
2:

        # It is now safe to call C functions which may rely on initialized
        # data.
        
#        # Initialize MMU.
#        .extern hal_MMU_init
#        jsr      hal_MMU_init
#         nop
        
        # Enable caches
        mov.l    $cyg_var_enable_caches,r1
        jsr      @r1
         nop

        # Variant HALs may need to do something special before we continue
        mov.l    $hal_variant_init,r1
        jsr      @r1
         nop

        # Platform initialization
        mov.l    $hal_platform_init,r1
        jsr      @r1
         nop
        
        # call c++ constructors
        mov.l    $cyg_hal_invoke_constructors,r1
        jsr      @r1
         nop

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        mov.l    $initialize_stub,r1
        jsr      @r1
         nop
#endif
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
        mov.l    $hal_ctrlc_isr_init,r1
        jsr      @r1
         nop
#endif

        mov.l    $cyg_start,r1
        jsr      @r1
         nop
9:      
        bra      9b                     ! if we return, loop
         nop

        .align  2
        SYM_PTR_REF(_bss_start)
        SYM_PTR_REF(_bss_end)
        SYM_PTR_REF(startup_stack)
        SYM_PTR_REF(cyg_hal_invoke_constructors)
        SYM_PTR_REF(cyg_var_enable_caches)
        SYM_PTR_REF(hal_variant_init)
        SYM_PTR_REF(hal_platform_init)
        SYM_PTR_REF(cyg_start)

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        SYM_PTR_REF(initialize_stub)
#endif
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
        SYM_PTR_REF(hal_ctrlc_isr_init)
#endif

#---------------------------------------------------------------------------
# Interrupt vector tables.
# These tables contain the isr, data and object pointers used to deliver
# interrupts to user code.

        .data

        .extern CYG_LABEL_DEFN(hal_default_isr)
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
#define CYG_ISR_TABLE_SIZE    1
#else
#define CYG_ISR_TABLE_SIZE    CYGNUM_HAL_ISR_COUNT
#endif

SYM_DEF(hal_interrupt_handlers)
        .rept   CYG_ISR_TABLE_SIZE
        .long   CYG_LABEL_DEFN(hal_default_isr)
        .endr

SYM_DEF(hal_interrupt_data)
        .rept   CYG_ISR_TABLE_SIZE
        .long   0
        .endr

SYM_DEF(hal_interrupt_objects)
        .rept   CYG_ISR_TABLE_SIZE
        .long   0
        .endr

#---------------------------------------------------------------------------
## Temporary interrupt stack
        
        .section ".bss"

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK 
        .balign 16
SYM_DEF(cyg_interrupt_stack_base)
        .rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
        .byte 0
        .endr
        .balign 16
        .global _cyg_interrupt_stack
SYM_DEF(cyg_interrupt_stack)
        
        .long   0,0,0,0
#endif // CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK


#if defined(CYGSEM_HAL_ROM_MONITOR) || defined(CYGPKG_REDBOOT)
  // Enough space for stub to handle downloads. If using thread capabilities
  // it will be using the RAM application's stack.
# define STARTUP_STACK_SIZE CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
#else
# ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
   // Enough space to run constructors.
   // FIXME: 512 is enough for all tests. doc/examples/twothreads
   // calls printf on this stack though, and so requires more space.
#  define STARTUP_STACK_SIZE 1024
# else
#  define STARTUP_STACK_SIZE CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
# endif
#endif

        .balign 16
SYM_DEF(startup_stack_base)
        .rept   STARTUP_STACK_SIZE
        .byte 0
        .endr
        .balign 16
SYM_DEF(startup_stack)
        .long   0,0,0,0

#---------------------------------------------------------------------------
# end of vectors.S
