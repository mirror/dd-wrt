##=============================================================================
##
##	vectors.S
##
##	CalmRISC32 exception vectors
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
## Author(s):	msalter
## Contributors: msalter	
## Date:	2001-02-12
## Purpose:	CalmRISC32 exception vectors
## Description:	This file defines the code placed into the exception
##              vectors. It also contains the first level default VSRs
##		that save and restore state for both exceptions and
##		interrupts.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/system.h>	
#include <pkgconf/hal.h>
#include <pkgconf/hal_calm32.h>

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>	
#else
# undef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
# undef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
#endif
			
#include <cyg/hal/arch.inc>	
#include <cyg/hal/hal_arch.h>	

	.macro  txreg reg
    99:
	cld	r0,fr29
	cmp	eq r0,#0
	brfd	99b
	 nop
	cld	fr30,\reg
	ld	r0,#1
	cld	fr29,r0
	.endm

	.extern cyg_instrument

##-----------------------------------------------------------------------------
## Hardware supplied vectors

	.macro	cpu_vector  vsr_nr
	setsr	bs
	clrsr	rs0
	clrsr	rs1
	nop
	push	r8
	brad	__exc_trampoline
	 ld	r8,#\vsr_nr
	.endm
	
##==========================================================================
## Hardware exception vectors.
##
        .section ".vectors","ax"
	.global reset_vector
reset_vector:
	ldw	r0,#_start
	jmpd	r0
	 nop

	# 0x20 - FIQ  
	.p2align 5
	cpu_vector 0
	
	# 0x40 - IRQ
	.p2align 5
	cpu_vector 1
	
	# 0x50 - COP
	.p2align 4
	cpu_vector 2
	
	# 0x60 - DABRT
	.p2align 4
	cpu_vector 3
	
	# 0x70 - IABRT
	.p2align 4
	cpu_vector 4
	
	# 0x80 - PRIV
	.p2align 4
	cpu_vector 5
	
	# 0xA0 - UNIMPL
	.p2align 5
	cpu_vector 6
	
	# 0xC0 - TRACE
	.p2align 5
	cpu_vector 7
	
	# 0x100 - SWI 0
	.p2align 6
	cpu_vector 8
	
	# SWI 1
	.p2align 4
	cpu_vector 8
	
	# SWI 2
	.p2align 4
	cpu_vector 8
	
	# SWI 3
	.p2align 4
	cpu_vector 8
	
	# SWI 4
	.p2align 4
	cpu_vector 8
	
	# SWI 5
	.p2align 4
	cpu_vector 8
	
	# SWI 6
	.p2align 4
	cpu_vector 8
	
	# SWI 7
	.p2align 4
	cpu_vector 8
	
	# SWI 8
	.p2align 4
	cpu_vector 8
	
	# SWI 9
	.p2align 4
	cpu_vector 8
	
	# SWI 10
	.p2align 4
	cpu_vector 8
	
	# SWI 11
	.p2align 4
	cpu_vector 8
	
	# SWI 12
	.p2align 4
	cpu_vector 8
	
	# SWI 13
	.p2align 4
	cpu_vector 8
	
	# SWI 14
	.p2align 4
	cpu_vector 8
	
	# SWI 15
	.p2align 4
	cpu_vector 8

__exc_trampoline:
	push	r8	// vsr number
	sl2	r8	// offset into vsr table
	ldw	r8,@[r8 + 0]
	jmpd	r8
	 pop	r8
	
##-----------------------------------------------------------------------------
## Startup code

	.text
	
FUNC_START _start
	# default vector table
	ld	r0,#0
	ld	vbr,r0

	# ensure privileged mode with all bank1 register view
	setsr	pm
	setsr	bs
	clrsr	rs0
	clrsr	rs1
	
	# Initialize hardware
	hal_cpu_init
	hal_diag_init
	hal_memc_init
	hal_cache_init
	hal_timer_init
		
	# load initial stack pointer
	ldw	sp,#__startup_stack

	clrsr	bs
	nop
	ldw	sp,#__user_stack
	setsr	bs
	nop

	hal_mon_init
		
#ifdef CYG_HAL_STARTUP_ROM
	# Copy data from ROM to RAM

	.extern	hal_copy_data
	ldw	r0,#hal_copy_data
	jsrd	r0
	 nop
#endif

	# Zero BSS

	.extern hal_zero_bss
	ldw	r0,#hal_zero_bss
	jsrd	r0
	 nop

	# Call variant and platform HAL
	# initialization routines.

	.extern	hal_variant_init
	ldw	r0,#hal_variant_init
	jsrd	r0
	 nop

	.extern	hal_platform_init
	ldw	r0,#hal_platform_init
	jsrd	r0
	 nop

	# Call constructors
	.extern cyg_hal_invoke_constructors
	ldw	r0,#cyg_hal_invoke_constructors
	jsrd     r0
	 nop

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
	.extern initialize_stub
	ldw	r0,#initialize_stub
	jsrd     r0
         nop
#endif
	
#if defined(CYGDBG_HAL_CALM32_DEBUG_GDB_CTRLC_SUPPORT)
	.extern hal_ctrlc_isr_init
	ldw	r0,#hal_ctrlc_isr_init
	jsrd     r0
	 nop
#endif

	# Call cyg_start	
	
	.extern	cyg_start
#if 0
	clrsr	pm
	nop
	clrsr	bs
	nop
#endif
	ld	fp,sp

        ldw	lr,#_start
	ldw	r0,#cyg_start
	jmpd	r0
	 nop
FUNC_END _start


	.macro switch_stack_and_save_regs tmpvar
	// save vector number temporarily
	push	r9
	ldw	r9,#\tmpvar
	ldw	@[r9 + 0],r8
	pop	r9

	// Make sure we use the gdb stack
	ldw	r8,#__GDB_stack
	cmpu	gt sp,r8
	brtd	0f
	 nop
	ldw	r8,#__GDB_stack_base
	cmpu	gt sp,r8
	brtd	1f
	 nop
    0:
	// need to switch stack
	ld	r8,sp
	add	r8,#4
	ldw	sp,#__GDB_stack
	push	r8
	push	r14
	push	r13
	push	r12
	sub	r8,#4
	brad	2f
	 ldw	r8,@[r8+0]
    1:
	// already using gdb stack
	pop	r8
	// save bank 1 regs
	push	q3
    2:
	// save rest of bank1 regs
	push	q2
	push	q1
	push	q0
	
	// save bank 0 regs
	setsr	rs1
	nop
	push	q1
	push	q0
	clrsr	rs1
	setsr	rs0
	nop
	push	q1
	push	q0
	clrsr	rs0
	nop
	// save special regs
	ld	r0,ssr_expt
	push	r0
	ld	r0,ssr_swi
	push	r0
	ld	r0,ssr_fiq
	push	r0
	ld	r0,ssr_irq
	push	r0
	ld	r0,spc_expt
	push	r0
	ld	r0,spc_swi
	push	r0
	ld	r0,spc_fiq
	push	r0
	ld	r0,spc_irq
	push	r0
	ld	r0,vbr
	push	r0

	ldw	r8,#\tmpvar
	ldw	r8,@[r8+0]
	push	r8
	.endm

	.macro restore_regs
	// discard vector
	pop	r0
	// restore special regs
	pop	r0
	ld	vbr,r0
	pop	r0
	ld	spc_irq,r0
	pop	r0
	ld	spc_fiq,r0
	pop	r0
	ld	spc_swi,r0
	pop	r0
	ld	spc_expt,r0
	pop	r0
	ld	ssr_irq,r0
	pop	r0
	ld	ssr_fiq,r0
	pop	r0
	ld	ssr_swi,r0
	pop	r0
	ld	ssr_expt,r0
	// restore bank 0 regs
	setsr	rs1
	nop
	pop	q0
	pop	q1
	clrsr	rs1
	setsr	rs0
	nop
	pop	q0
	pop	q1
	clrsr	rs0
	nop
	// restore bank 1 regs
	pop	q0
	pop	q1
	pop	q2
	pop	q3
	.endm


##-----------------------------------------------------------------------------
## Default exception VSR.
## Saves machine state and calls external handling code.
## On entry, the original R8 has been pushed on the stack and R8 now
## contains the vector number.
	
FUNC_START __default_exception_vsr

	switch_stack_and_save_regs __expt_temp

	// call exception handler
	ldw	r8,#cyg_hal_exception_handler
	jsrd	r8
	 ld	r0,sp

	restore_regs
	
	ret_expt
	 nop

FUNC_END   __default_exception_vsr

##-----------------------------------------------------------------------------
## Default swi VSR.
## Saves machine state and calls external handling code.
## On entry, the original R8 has been pushed on the stack and R8 now
## contains the vector number.
	
FUNC_START __default_swi_vsr

	switch_stack_and_save_regs __swi_temp

	// call exception handler
	ldw	r8,#cyg_hal_exception_handler
	jsrd	r8
	 ld	r0,sp

	restore_regs

	ret_swi
	 nop

FUNC_END  __default_swi_vsr

##------------------------------------------------------------------------------
## Default interrupt VSR.
## Saves machine state and calls appropriate ISR. When done, calls
## interrupt_end() to finish up and possibly reschedule.	

FUNC_START __default_fiq_vsr
	switch_stack_and_save_regs __fiq_temp

	ldw	r8,#hal_interrupt_data
	ldw	r1,@[r8 + 0]
	ldw	r8,#hal_interrupt_handlers
	ldw	r8,@[r8 + 0]

	// call fiq handler
	jsrd	r8
	 ld	r0,#0

	restore_regs

	ret_fiq
	 nop
FUNC_END   __default_fiq_vsr
	
FUNC_START __default_irq_vsr
	switch_stack_and_save_regs __irq_temp

	ldw	r8,#hal_interrupt_data
	ldw	r1,@[r8 + 4]
	ldw	r8,#hal_interrupt_handlers
	ldw	r8,@[r8 + 4]

	// call fiq handler
	jsrd	r8
	 ld	r0,#1
	
	restore_regs

	ret_irq
	 nop
FUNC_END   __default_irq_vsr

##-----------------------------------------------------------------------------
## Execute pending DSRs on the interrupt stack with interrupts enabled.
## Note: this can only be called from code running on a thread stack
	
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
	.extern cyg_interrupt_call_pending_DSRs

FUNC_START hal_interrupt_stack_call_pending_DSRs
FUNC_END hal_interrupt_stack_call_pending_DSRs
#endif		

##-----------------------------------------------------------------------------
## Short circuit in case any code tries to use "__gccmain()"

FUNC_START __gccmain
	jmpd	lr
	 nop
FUNC_END __gccmain

##-----------------------------------------------------------------------------
## Interrupt Stack.
## Used during intialization and for executing ISRs.
	
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

	.long	0
	
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        .balign 16
__GDB_stack_base:
        .rept 0x400
        .byte 0
        .endr
__GDB_stack:
	.long	0
#endif
        .balign 16
__startup_stack_base:
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        .rept 512
#else
        .rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
#endif
        .byte 0
        .endr
        .balign 16
__startup_stack:
	.long	0

        .balign 16
__user_stack_base:
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        .rept 512
#else
        .rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
#endif
        .byte 0
        .endr
        .balign 16
__user_stack:
	.long	0

__expt_temp:
	.long	0
__swi_temp:
	.long	0
__irq_temp:
	.long	0
__fiq_temp:
	.long	0


##-----------------------------------------------------------------------------
## VSR table.
## The main interrupt code indirects through here to find the VSR
## to execute for each architecture defined interrupt.
## This is only used for simulated targets, on real targets a fixed location VSR
## table is now allocated at 0x80000100.

#ifndef CYG_HAL_CALM32_VSR_TABLE_DEFINED
	
##	.section ".vsr_table","a"
	
	.data
			
	.globl	hal_vsr_table
hal_vsr_table:
	.long	__default_fiq_vsr	// FIQ
	.long	__default_irq_vsr	// IRQ
	.long	__default_exception_vsr	// COP
	.long	__default_exception_vsr	// DABRT
	.long	__default_exception_vsr	// IABRT
	.long	__default_exception_vsr	// PRIV
	.long	__default_exception_vsr	// UNIMPL
	.long	__default_exception_vsr	// TRACE
	.long	__default_swi_vsr	// SWI
#endif	
	
        .balign 16
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
	## Vectors used to communicate between eCos and ROM environments
        .globl  hal_virtual_vector_table
hal_virtual_vector_table:
        .rept   64
        .long   0
        .endr
#endif
        
#------------------------------------------------------------------------------
# Interrupt vector tables.
# These tables contain the isr, data and object pointers used to deliver
# interrupts to user code.
# hal_interrupt_level contains the interrupt level set by 
# HAL_INTERRUPT_CONFIGURE().
# This is a default set that provide support only for the single external
# interrupt. Platforms or boards are expected to define their own versions
# of these if they have their own interrupt mappings.

#ifndef CYG_HAL_CALM32_ISR_TABLES_DEFINED

	.extern hal_default_isr
	
	.data

	.globl	hal_interrupt_handlers
hal_interrupt_handlers:
	.long	hal_default_isr
	.long	hal_default_isr


	.globl	hal_interrupt_data
hal_interrupt_data:
	.long	0
	.long	0

	.globl	hal_interrupt_objects
hal_interrupt_objects:
	.long	0
	.long	0
#endif

	.data
	.globl __break_inst_in_data
__break_inst_in_data:
	.short 0x80e0


##-----------------------------------------------------------------------------
## end of vectors.S


