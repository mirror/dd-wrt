// #========================================================================
// #
// #    vectors.S
// #
// #    NEC/V85x exception/interrupt vectors
// #
// #========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
// #========================================================================
// ######DESCRIPTIONBEGIN####
// #
// # Author(s):     gthomas, jlarmour
// # Contributors:  gthomas, jlarmour
// # Date:          2001-03-21
// # Purpose:       NEC/V85x exception vectors
// # Description:   This file defines the code placed into the exception
// #                vectors. It also contains the first level default VSRs
// #		    that save and restore state for both exceptions and
// #		    interrupts.
// #
// #####DESCRIPTIONEND####
// #
// #========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/hal_v85x.h>
#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#else
# undef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
# undef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
#endif
#include <cyg/hal/hal_platform_setup.h>
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/hal/nec_offsets.inc>

// Two different macros in case we ever decide to differentiate
// between exceptions and interrupts.                
        .macro  EXCEPTION
        .org    reset_vector+(0x0010*VECTOR)
        addi    -CYGARC_EXCEPTION_FRAME_SIZE,sp,sp
        st.w    r8,CYGARC_REG_R8[sp]
        movea   VECTOR,r0,r8
        jr      exception
        .set    VECTOR, VECTOR+1
        .endm

        .macro  INTERRUPT
        .org    reset_vector+(0x0010*VECTOR)
        addi    -CYGARC_EXCEPTION_FRAME_SIZE,sp,sp
        st.w    r8,CYGARC_REG_R8[sp]
        movea   VECTOR,r0,r8
        jr      exception
        .set    VECTOR, VECTOR+1
        .endm

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
        .section ".vectors","ax"
        .globl  reset_vector
reset_vector:
        
//
// These are the hardware exception vectors.
//
__ROM_vsr:
        // start of ROM
        mov     r0,sp  // needed for platforms that have NMI 2 insns after reset
        // This jump works even for ROMRAM because it is PC relative
        jr      start

        .set    VECTOR, 1
        .rept   CYGNUM_HAL_EXCEPTION_COUNT-1
        EXCEPTION
        .endr
        .set    VECTOR, CYGNUM_HAL_ISR_MIN
        .rept   CYGNUM_HAL_ISR_COUNT
        INTERRUPT
        .endr
#endif
        
        .text
        .globl  start
start:
#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
        // Before anything else, *carefully* check if this was actually an NMI
        // The assumption is that the stack pointer can only be 0 on reset
        // as set above at __ROM_vsr
        cmp     r0,sp
        be      1f
        addi    -CYGARC_EXCEPTION_FRAME_SIZE,sp,sp
        movea   1,r0,r8
        jr      exception
1:      
#endif
        
        // Perform hardware initialization
        PLATFORM_SETUP1

        // Copy everything to the proper VMA base and jump to it.
#if defined(CYG_HAL_STARTUP_ROMRAM)
        lea     CYGMEM_REGION_rom,r6
        lea     CYGMEM_REGION_ram,r7
        lea     ___rom_data_end,r8 // Assume end of .data == end of ROM
1:      ld.w    0[r6],r1
        st.w    r1,0[r7]
        addi    4,r6,r6
        addi    4,r7,r7
        cmp     r6,r8
        bne     1b
        
        // now jump to RAM
        lea     ___romram_jump_vector,r1
        jmp     [r1]

___romram_jump_vector:
        
#elif defined( CYG_HAL_STARTUP_ROM )
	// Relocate [copy] data from ROM to RAM
        lea     ___rom_data_start,r6
        lea     ___ram_data_start,r7
        lea     ___ram_data_end,r8
1:      ld.w    0[r6],r1
        st.w    r1,0[r7]
        addi    4,r6,r6
        addi    4,r7,r7
        cmp     r7,r8
        bne     1b

        // Initialize RAM vectors
        lea     ___ram_vectors_loadaddr,r6
        lea     ___ram_vectors_start,r7
        lea     ___ram_vectors_end,r8
1:      ld.w    0[r6],r1
        st.w    r1,0[r7]
        addi    4,r6,r6
        addi    4,r7,r7
        cmp     r7,r8
        bne     1b
#endif
        
        // Initialize stack
        lea     __startup_stack-16,r1
        mov     r1,sp

        // Clear BSS
        lea     ___bss_start,r6
        lea     ___bss_end,r7
1:      st.w    r0,0[r6]
        addi    4,r6,r6
        cmp     r6,r7
        bne     1b

#if defined(CYG_HAL_STARTUP_RAM)
// Set up VSR pointers into this application/image
        lea     _hal_vsr_table+(CYGNUM_HAL_ISR_MIN*4),r6
        movea   CYGNUM_HAL_ISR_COUNT,r0,r7
        lea     do_interrupt,r8
10:     st.w    r8,0[r6]
        addi    4,r6,r6
        addi    -1,r7,r7
        bne     10b        
#endif

        // Initialize hardware
        jarl    _cyg_hal_hardware_init,r31

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
# ifdef CYG_HAL_STARTUP_RAM
// Need to handle exceptions as well
        lea     _hal_vsr_table,r6
        movea   CYGNUM_HAL_EXCEPTION_COUNT,r0,r7
        lea     do_exception,r8
10:     st.w    r8,0[r6]
        addi    4,r6,r6
        addi    -1,r7,r7
        bne     10b
# endif
        jarl    _initialize_stub,r31
#else // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
// All programs need to provide for NMI handlers        
        lea     _hal_vsr_table+4,r6
        lea     do_exception,r8
        st.w    r8,0[r6]    // NMI pin
#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
        // we can process NMIs now
        lea     __allow_nmi,r1
        addi    1,r0,r2
        st.w    r2,0[r1]
#endif        
                
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
        .extern hal_ctrlc_isr_init
        jarl    _hal_ctrlc_isr_init,r31
#endif
        
        // Run through static constructors
        jarl    _cyg_hal_invoke_constructors,r31

        // Ready to start the eCos kernel
        jarl    _cyg_start,r31

        // If we get here, something is _very_ wrong
_start_hang:    
        br      _start_hang
        
#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
        // Handle exception
exception:
        // save exception context
        st.w    ep,CYGARC_REG_EP[sp]
        mov     sp,ep
        sst.w   r8,CYGARC_REG_VECTOR[ep]
        sst.w   r1,CYGARC_REG_R1[ep]
        addi    CYGARC_EXCEPTION_FRAME_SIZE,ep,r1
        sst.w   r2,CYGARC_REG_R2[ep]
        sst.w   r1,CYGARC_REG_SP[ep]
        sst.w   r4,CYGARC_REG_R4[ep]        
        sst.w   r5,CYGARC_REG_R5[ep]        
        sst.w   r6,CYGARC_REG_R6[ep]        
        sst.w   r7,CYGARC_REG_R7[ep]        
//        sst.w   r8,CYGARC_REG_R8[ep]        
        sst.w   r9,CYGARC_REG_R9[ep]        
        sst.w   r10,CYGARC_REG_R10[ep]        
        sst.w   r11,CYGARC_REG_R11[ep]        
        sst.w   r12,CYGARC_REG_R12[ep]        
        sst.w   r13,CYGARC_REG_R13[ep]        
        sst.w   r14,CYGARC_REG_R14[ep]        
        sst.w   r15,CYGARC_REG_R15[ep]        
        sst.w   r16,CYGARC_REG_R16[ep]        
        sst.w   r17,CYGARC_REG_R17[ep]        
        sst.w   r18,CYGARC_REG_R18[ep]        
        sst.w   r19,CYGARC_REG_R19[ep]        
#ifndef CYGDBG_HAL_COMMON_INTERRUPTS_SAVE_MINIMUM_CONTEXT
        sst.w   r20,CYGARC_REG_R20[ep]        
        sst.w   r21,CYGARC_REG_R21[ep]        
        sst.w   r22,CYGARC_REG_R22[ep]        
        sst.w   r23,CYGARC_REG_R23[ep]        
        sst.w   r24,CYGARC_REG_R24[ep]        
        sst.w   r25,CYGARC_REG_R25[ep]        
        sst.w   r26,CYGARC_REG_R26[ep]        
        sst.w   r27,CYGARC_REG_R27[ep]        
        sst.w   r28,CYGARC_REG_R28[ep]        
#endif
        sst.w   r29,CYGARC_REG_R29[ep] // NB frame pointer
        sst.w   lp,CYGARC_REG_LP[ep]
        cmp     CYGNUM_HAL_VECTOR_TRAP00,r8  // NMI?
        bge     05f
        stsr    FEPC,r6                 // saved state differs
        stsr    FEPSW,r7
        lea     __allow_nmi,r1          // is the system ready for NMIs?
        tst1    0, 0[r1]
        bne     10f
        // if not processing NMIs, allow restore
        sst.w   r6,CYGARC_REG_PC[ep]
        sst.w   r7,CYGARC_REG_PSW[ep]
        // Reenable exception processing
        stsr    PSW,r6
        andi    ~(CYGARC_PSW_EP|CYGARC_PSW_NP),r6,r6
        ldsr    r6,PSW
        lea     return_from_exception_or_interrupt,r1
        jmp     [r1]

05:     stsr    EIPC,r6
        stsr    EIPSW,r7        
        cmp     CYGNUM_HAL_VECTOR_ILGOP,r8 // Illegal opcode?
        bne     10f
        addi    -4,r6,r6                // yes, adjust PC
10:     sst.w   r6,CYGARC_REG_PC[ep]
        sst.w   r7,CYGARC_REG_PSW[ep]
        stsr    ECR,r6
        sst.w   r6,CYGARC_REG_CAUSE[ep]
        // Reenable exception processing
        stsr    PSW,r6
        andi    ~(CYGARC_PSW_EP|CYGARC_PSW_NP),r6,r6
        ldsr    r6,PSW
        shl     2,r8
        lea     _hal_vsr_table,r1
        add     r1,r8
        ld.w    0[r8],r1
        jmp     [r1]
#endif
        
do_exception:
        // pass in saved registers
        mov     sp,r6
        
        // leave space for the parameter flush back area (as per calling
        // conventions)
        addi    -16,sp,sp
        
        jarl    _exception_handler,r31

return_from_exception_or_interrupt: 
        // save sp, but remove space for the parameter flush back area
        // (as per calling conventions) at the same time
        addi    16,sp,ep
                
        sld.w   CYGARC_REG_PSW[ep],r7
        sld.w   CYGARC_REG_R1[ep],r1
        sld.w   CYGARC_REG_PC[ep],r6
        sld.w   CYGARC_REG_R2[ep],r2       
        // disable interrupts while restoring context
        ld.w    CYGARC_REG_VECTOR[ep],r4
        stsr    PSW,r8
        cmp     CYGNUM_HAL_VECTOR_TRAP00,r4
        bge     10f
        ori     CYGARC_PSW_NP,r8,r8     // returning from NMI
        ori     CYGARC_PSW_ID,r8,r8     // disable interrupt
        ldsr    r8,PSW
        ldsr    r7,FEPSW
        ldsr    r6,FEPC
        br      30f
10:     cmp     CYGNUM_HAL_ISR_MIN,r4   // exception or interrupt
        bge     20f
        ori     CYGARC_PSW_EP,r8,r8     // returning from exception
20:     ori     CYGARC_PSW_ID,r8,r8     // disable interrupt
        ldsr    r8,PSW
        ldsr    r7,EIPSW                // Avoid pipeline bubbles
        ldsr    r6,EIPC
30:     sld.w   CYGARC_REG_R4[ep],r4
        sld.w   CYGARC_REG_R5[ep],r5      
        sld.w   CYGARC_REG_R6[ep],r6      
        sld.w   CYGARC_REG_R7[ep],r7      
        sld.w   CYGARC_REG_R8[ep],r8      
        sld.w   CYGARC_REG_R9[ep],r9      
        sld.w   CYGARC_REG_R10[ep],r10       
        sld.w   CYGARC_REG_R11[ep],r11     
        sld.w   CYGARC_REG_R12[ep],r12
        sld.w   CYGARC_REG_R13[ep],r13              
        sld.w   CYGARC_REG_R14[ep],r14       
        sld.w   CYGARC_REG_R15[ep],r15       
        sld.w   CYGARC_REG_R16[ep],r16       
        sld.w   CYGARC_REG_R17[ep],r17       
        sld.w   CYGARC_REG_R18[ep],r18               
        sld.w   CYGARC_REG_R19[ep],r19       
#ifndef CYGDBG_HAL_COMMON_INTERRUPTS_SAVE_MINIMUM_CONTEXT
        sld.w   CYGARC_REG_R20[ep],r20       
        sld.w   CYGARC_REG_R21[ep],r21       
        sld.w   CYGARC_REG_R22[ep],r22       
        sld.w   CYGARC_REG_R23[ep],r23       
        sld.w   CYGARC_REG_R24[ep],r24       
        sld.w   CYGARC_REG_R25[ep],r25       
        sld.w   CYGARC_REG_R26[ep],r26       
        sld.w   CYGARC_REG_R27[ep],r27       
        sld.w   CYGARC_REG_R28[ep],r28       
#endif
        sld.w   CYGARC_REG_R29[ep],r29 // NB frame pointer
        sld.w   CYGARC_REG_LP[ep],lp
        sld.w   CYGARC_REG_SP[ep],sp
        sld.w   CYGARC_REG_EP[ep],ep
        reti
        
        // Handle interrupt - these are typically vectored into user code
do_interrupt:
        mov     sp,ep                   // save pointer to regs frame
        // leave space for the parameter flush back area (as per calling
        // conventions)
        addi    -16,sp,sp

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
	// Switch to interrupt stack
        lea     irq_level,r6            // current number of nested interrupts
        ld.w    0[r6],r7
        addi    1,r7,r8
        st.w    r8,0[r6]
        cmp     0,r7                    // if was zero, switch stacks
        bne     10f
        mov     sp,r1                   // save old stack pointer
        lea     __interrupt_stack-20,r2
        mov     r2,sp
        st.w    r1,16[sp]                
10:
#endif // CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

        // The entire CPU state is now stashed on the stack,
        // increment the scheduler lock and handle the interrupt

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT			
        .extern	_cyg_scheduler_sched_lock
        lea     _cyg_scheduler_sched_lock,r7
        ld.w    0[r7],r8
        addi    1,r8,r8
        st.w    r8,0[r7]
#endif

#if defined(CYGPKG_KERNEL_INSTRUMENT) && \
    defined(CYGDBG_KERNEL_INSTRUMENT_INTR)
        lea     RAISE_INTR,r6                   // arg0 = type = INTR,RAISE
        ld.w    CYGARC_REG_VECTOR[ep],r7        // args = vector
        mov     r0,r8                           // arg2 = 0
        jarl    _cyg_instrument,r31
#endif

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
	// If we are supporting Ctrl-C interrupts from GDB, we must squirrel
	// away a pointer to the save interrupt state here so that we can
	// plant a breakpoint at some later time.
	
	.extern	_hal_saved_interrupt_state
	lea     _hal_saved_interrupt_state,r8
        st.w    ep,0[r8]
#endif

#ifdef CYGIMP_KERNEL_INTERRUPTS_CHAIN
        mov     r0,r8                           // vector # (overloaded by kernel)
#else	
        ld.w    CYGARC_REG_VECTOR[ep],r6        // vector #
        addi    -CYGNUM_HAL_ISR_MIN,r6,r8
        shl     2,r8
#endif        
        lea     _hal_interrupt_objects,r1
        add     r8,r1
        ld.w    0[r1],r29                       // save object handle
        lea     _hal_interrupt_data,r1
        add     r8,r1
        ld.w    0[r1],r7                        // handler data
        lea     _hal_interrupt_handlers,r1
        add     r8,r1
        mov     ep,r8                           // pointer to saved registers
        ld.w    0[r1],r1                        // handler routine
#ifdef CYGIMP_KERNEL_INTERRUPTS_CHAIN
        ld.w    CYGARC_REG_VECTOR[ep],r6        // vector #
#endif        
        jarl    call_via_r1,r31
        
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
	// If we are returning from the last nested interrupt, move back
	// to the thread stack. interrupt_end() must be called on the
	// thread stack since it potentially causes a context switch.
        lea     irq_level,r6
        ld.w    0[r6],r7
        addi    -1,r7,r8
        st.w    r8,0[r6]
        cmp     0,r8
        bne     10f
        ld.w    16[sp],sp       // Restore non-interrupt stack at last interrupt
                                // this is offset 16 because of parameter
                                // flush back area
10:
#endif // CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
	// The return value from the handler (in r10) will indicate whether a 
	// DSR is to be posted. Pass this together with a pointer to the
	// interrupt object we have just used to the interrupt tidy up routine.

        mov     r10,r6                          // interrupt handler return code
        mov     r29,r7                          // object handle
        mov     ep,r8                           // register frame
        jarl    _interrupt_end,r31              // post any bottom layer handle
        
#endif
        br      return_from_exception_or_interrupt

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        .globl  _hal_interrupt_stack_call_pending_DSRs
_hal_interrupt_stack_call_pending_DSRs:
        stsr    PSW,r9
        di                              // disable interrupts while changing stack
	// Switch to interrupt stack
        lea     irq_level,r6            // current number of nested interrupts
        mov     1,r7                    // note: this can never be non-zero at this point
        st.w    r7,0[r6]
        mov     sp,r1                   // save old stack pointer
        lea     __interrupt_stack-32,r8 // 32 for 16-byte parameter flush back
                                        // plus 12 bytes for following reg saves
                                        // plus 4 bytes to preserve alignment
        mov     r8,sp        
        st.w    r1,16[sp]               
        st.w    r9,20[sp]
        st.w    lp,24[sp]
        ei
       	jarl    _cyg_interrupt_call_pending_DSRs,lp
        di
        ld.w    24[sp],lp                // restore state
        ld.w    20[sp],r6
        ld.w    16[sp],sp
        lea     irq_level,r7
        st.w    r0,0[r7]
        ldsr    r6,PSW
        jmp     [lp]
#endif // CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
                
//
// Indirect subroutine call, via R1

call_via_r1:    
        jmp     [r1]
                                        
//
// Reset the board
//
        .globl  _hal_reset_board
_hal_reset_board:   
        di                      // Turn off interrupts
        // Don't process NMIs
        lea     __allow_nmi,r1
        st.w    r0,0[r1]
        jmp     [r0]            // Restart machine        

#if !defined(CYGSEM_HAL_NEC_INLINE_INTERRUPT_FUNCTIONS)
        .globl  _hal_disable_interrupts
_hal_disable_interrupts:
        stsr    PSW,r10
        di
        jmp     [lp]

        .globl  _hal_enable_interrupts
_hal_enable_interrupts:
        stsr    PSW,r6
        ei
        jmp     [lp]

        .globl  _hal_restore_interrupts
_hal_restore_interrupts:
        ldsr    r6,PSW
        jmp     [lp]
#endif        
                
// -------------------------------------------------------------------------
        .section ".ram_vectors","awx",@progbits

#if defined(CYG_HAL_STARTUP_ROM)
// Trap jump table - used by builtin ROM
__vsr_table:            
        .rept   CYGNUM_HAL_VSR_COUNT
        .word   0x00000794,0,0,0   // jr 0x0100xx0, nop, nop, nop
        .endr
__vsr_table_end:
#endif

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)        
_hal_vsr_table:
        .rept   CYGNUM_HAL_EXCEPTION_COUNT
        .word   do_exception
        .endr
        .rept   CYGNUM_HAL_ISR_COUNT
        .word   do_interrupt
        .endr

// Flag to indicate whether NMIs are allowed
// Without this, if we do a soft reset (using hal_plf_reset_board above)
// we may process NMIs like the watchdog timeout before the system can
// accept them
__allow_nmi:
        .word 0
        
        .balign 64
_hal_virtual_vector_table:      
        .rept   CYGNUM_CALL_IF_TABLE_SIZE
        .word   0
        .endr

#if CYGINT_HAL_V85X_ICE_DEBUG
        .globl  _hal_v85x_ice_syscall_info
_hal_v85x_ice_syscall_info:
         .rept  48
         .long  0
         .endr
#endif // if CYGINT_HAL_V85X_ICE_DEBUG
#endif // if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
        
        .data
// Initial stack        
        .rept   1024
        .byte   0
        .endr
__startup_stack:        

#ifndef CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
#define CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE 4096
#endif
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
	.balign 16
	.global _cyg_interrupt_stack_base
_cyg_interrupt_stack_base:
__interrupt_stack_base:
        .rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
        .byte 0
        .endr
	.balign 16
	.global _cyg_interrupt_stack
_cyg_interrupt_stack:
__interrupt_stack:
irq_level:
	.long	0
#endif

        
// Interrupt vector tables.
// These tables contain the isr, data and object pointers used to deliver
// interrupts to user code.

	.extern _hal_default_isr

	.globl	_hal_interrupt_handlers
_hal_interrupt_handlers:
	.rept	CYGNUM_HAL_ISR_COUNT
	.long	_hal_default_isr
	.endr

	.globl	_hal_interrupt_data
_hal_interrupt_data:
	.rept	CYGNUM_HAL_ISR_COUNT
	.long	0
	.endr

	.globl	_hal_interrupt_objects
_hal_interrupt_objects:
	.rept	CYGNUM_HAL_ISR_COUNT
	.long	0
	.endr

// -------------------------------------------------------------------------
// EOF vectors.S
