##=============================================================================
##
##	vectors.S
##
##	CalmRISC16 exception vectors
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
## Purpose:	CalmRISC16 exception vectors
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

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>	
#endif
			
#include <cyg/hal/arch.inc>	
#include <cyg/hal/hal_arch.h>	
	
##-----------------------------------------------------------------------------
## Hardware supplied vectors
	
//==========================================================================
// Hardware exception vectors.
//   This entire section will be copied to location 0x0000 at startup time.
//
        .section ".vectors","ax"
	.global SYM_NAME(reset_vector)
SYM_NAME(reset_vector):
	.short  0x100	; reset
        .short  0x110	; fiq
        .short  0x120	; irq
        .short  0x130	; trq
	.rept 62
	brad	0f	; swi
	 nop
	.endr

	;; 0x100 - reset
	ld	a8,SYM_NAME(_start)
	jmp	a8

	.p2align 4
	;; 0x110 - fiq
	push	a8
	ld	a8,0
	ldw	a8,@[a8+0]
	jmp	a8

	.p2align 4
	;; 0x120 - irq
	push	a8
	ld	a8,0
	ldw	a8,@[a8+4]
	jmp	a8

	.p2align 4
	;; 0x130 - trq
	push	a8
	ld	a8,0
	ldw	a8,@[a8+8]
	jmp	a8

	.p2align 4
	;; 0x140 - swi
  0:
	push	a8
	ld	a8,0
	ldw	a8,@[a8+12]
	jmp	a8

##-----------------------------------------------------------------------------
## Startup code

	.text
	
FUNC_START _start
	ld	r0,0
	cld	#28,r0
	cld	#29,r0
	cld	#30,r0
	cld	#31,r0
  2:
	# Initialize hardware
	hal_cpu_init
	hal_diag_init
	hal_memc_init
	hal_cache_init
	hal_timer_init
		
	# load initial stack pointer
	ld	sp,__startup_stack

	hal_mon_init

#ifdef CYG_HAL_STARTUP_ROM
	# Copy data from ROM to RAM

	.extern	SYM_NAME(hal_copy_data)
	jsr	SYM_NAME(hal_copy_data)
#endif
	# Zero BSS

	.extern SYM_NAME(hal_zero_bss)
	jsr	SYM_NAME(hal_zero_bss)

	# Call variant and platform HAL
	# initialization routines.

	.extern	SYM_NAME(hal_variant_init)
	jsr	SYM_NAME(hal_variant_init)

	.extern	SYM_NAME(hal_platform_init)
	jsr	SYM_NAME(hal_platform_init)

	# Call constructors
	.extern SYM_NAME(cyg_hal_invoke_constructors)
	jsr     SYM_NAME(cyg_hal_invoke_constructors)

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
        .extern SYM_NAME(initialize_stub)
        jsr     SYM_NAME(initialize_stub)
#endif
#if defined(CYGDBG_HAL_CALM16_DEBUG_GDB_CTRLC_SUPPORT)
        .extern SYM_NAME(hal_ctrlc_isr_init)
        jsr     SYM_NAME(hal_ctrlc_isr_init)
#endif

	# Call cyg_start	
	setsr	fe
	setsr	ie
	
	.extern	SYM_NAME(cyg_start)
        ld	lr,SYM_NAME(_start)
	jmp	SYM_NAME(cyg_start)
	
FUNC_END _start

#if 0
FUNC_START __txchar
	ldw	r0,@[sp+2]
    99:
	cld	r6,#29
	cmp	eq,r6,#0
	brfd	99b
	 nop
	cld	#30,r0
	ld	r6,1
	cld	#29,r6
	jmp	lr
FUNC_END __txchar
#endif

	.macro switch_stack_and_save_regs
	// a8 already pushed on the current stack
	push	r0
	
	ld	r0,e15
	and	r0,#0x3f
	cmp	ugt,r0,#%hi(__GDB_stack)
	brt	80f
	// sp <= __GDB_stack
	cmp	eq,r0,#%hi(__GDB_stack)
	brf	80f
	// %hi(sp) == %hi(__GDB_stack)
	ld	a8,#__GDB_stack
	cmp	ugt,r15,r8
	brt	80f
	// don't need switch
	pop	r0
	pop	a8
	sub	sp,#4
	push	a14
	ld	a14,sp
	add	a14,#8
	ldw	@[sp+6],a14
	bra	85f
   80:
	// switch to GDB stack
	pop	r0
	ld	a8,sp
	ld	sp,#__GDB_stack
	add	a8,#4
	push	a8
	push	a14
	sub	a8,#2
	ldw	a8,@[a8+0]
   85:
	// at this point, we're using the correct stack
	// and a15/a14 have been pushed
	push	a13,a12
	push	a11,a10
	push	a9,a8
	push	r7,r6
	push	r5,r4
	push	r3,r2
	push	r1,r0

	ld	r0,ssr_swi
	push	r0
	ld	r0,ssr_irq
	push	r0
	ld	r0,ssr_fiq
	push	r0
	ld	r0,spcl_irq
	push	r0
	ld	r0,spch_irq
	push	r0
	ld	r0,spcl_fiq
	push	r0
	ld	r0,spch_fiq
	push	r0
	.endm //  switch_stack_and_save_regs


	.macro restore_regs
	pop	r0
	ld	spch_fiq,r0
	pop	r0
	ld	spcl_fiq,r0
	pop	r0
	ld	spch_irq,r0
	pop	r0
	ld	spcl_irq,r0
	pop	r0
	ld	ssr_fiq,r0
	pop	r0
	ld	ssr_irq,r0
	pop	r0
	ld	ssr_swi,r0
	
	pop	r0,r1
	pop	r2,r3
	pop	r4,r5
	pop	r6,r7
	pop	a8,a9
	pop	a10,a11
	pop	a12,a13
	pop	a14
	ldw	sp,@[sp+2]
	.endm

##------------------------------------------------------------------------------
## Default interrupt VSR.
## Saves machine state and calls appropriate ISR.

FUNC_START __default_irq_vsr
	
	// IRQ is overloaded for use by the break insn.
	// For RedBoot (or any stub) on the core eval boards, we treat
	// IRQ as an exception.

	switch_stack_and_save_regs
	ld	r0,2
	push	r0

#if 0
	// push interrupt_data arg

	ld	a8,#SYM_NAME(hal_interrupt_data)
	ldw	a8,@[a8 + 4]
	push	a8

	// push vector arg
	push	r0
	ld	r0,0
	push	r0
	
	// call handler
	ld	a8,#SYM_NAME(hal_interrupt_handlers)
	ldw	a8,@[a8 + 4]
	jsr	a8
	add	sp,#10
#else
	// call exception handler
	ld	a8,sp
	add	a8,#2
	push	a8
	jsr     SYM_NAME(cyg_hal_exception_handler)
	add	sp,#6
#endif

	restore_regs
	ret_irq
FUNC_END   __default_irq_vsr

##------------------------------------------------------------------------------
## Default fast interrupt VSR.
## Saves machine state and calls appropriate ISR.

FUNC_START __default_fiq_vsr
	
	switch_stack_and_save_regs
	ld	r0,1
	push	r0

	// push interrupt_data arg
	ld	a8,#SYM_NAME(hal_interrupt_data)
	ldw	a8,@[a8 + 0]
	push	a8

	// push vector arg
	push	r0
	ld	r0,0
	push	r0
	
	// call handler
	ld	a8,#SYM_NAME(hal_interrupt_handlers)
	ldw	a8,@[a8 + 0]
	jsr	a8
	add	sp,#10

	restore_regs
	ret_fiq
FUNC_END   __default_fiq_vsr

##------------------------------------------------------------------------------
## Default trace VSR.
## Saves machine state and calls appropriate handler.

FUNC_START __default_trq_vsr
	switch_stack_and_save_regs
	ld	r0,3
	push	r0

	// call exception handler
	ld	a8,sp
	add	a8,#2
	push	a8
	jsr     SYM_NAME(cyg_hal_exception_handler)
	add	sp,#6

	restore_regs
	ret_irq
FUNC_END   __default_trq_vsr

##------------------------------------------------------------------------------
## Default swi VSR.
## Saves machine state and calls appropriate handler.

FUNC_START __default_swi_vsr
	switch_stack_and_save_regs
	ld	r0,4
	push	r0

	// call exception handler
	ld	a8,sp
	add	a8,#2
	push	a8
	jsr     SYM_NAME(cyg_hal_exception_handler)
	add	sp,#6

	restore_regs
	ret_swi
FUNC_END   __default_swi_vsr

##-----------------------------------------------------------------------------
## Short circuit in case any code tries to use "__gccmain()"

FUNC_START __gccmain
	jmp	lr
FUNC_END __gccmain

##-----------------------------------------------------------------------------
## Stacks.
## Used during intialization and for executing ISRs.
	
        .section ".bss"

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
	.p2align 2
__GDB_stack_base:
        .rept 0x1000
        .byte 0
        .endr
__GDB_stack:
	.long	0
#endif
	.p2align 2
__startup_stack_base:
	.rept 0x1000
	.byte 0
	.endr
	.p2align 2
__startup_stack:
	.long	0

##-----------------------------------------------------------------------------
## VSR table.
## The main interrupt code indirects through here to find the VSR
## to execute for each architecture defined interrupt.
## This is only used for simulated targets, on real targets a fixed location VSR
## table is now allocated at 0x000000.

#ifndef CYG_HAL_CALM16_VSR_TABLE_DEFINED
	
##	.section ".vsr_table","a"
	
	.data
			
	.globl	SYM_NAME(hal_vsr_table)

SYM_NAME(hal_vsr_table):
	.long	SYM_NAME(__default_fiq_vsr)	// FIQ
	.long	SYM_NAME(__default_irq_vsr)	// IRQ
	.long	SYM_NAME(__default_trq_vsr)	// TRQ
	.long	SYM_NAME(__default_swi_vsr)	// SWI
#endif	
	
	.data
        .balign 16
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
	// Vectors used to communicate between eCos and ROM environments
        .globl  SYM_NAME(hal_virtual_vector_table)
SYM_NAME(hal_virtual_vector_table):
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

#ifndef CYG_HAL_CALM16_ISR_TABLES_DEFINED

	.extern SYM_NAME(hal_default_isr)
	
	.data

	.globl	SYM_NAME(hal_interrupt_handlers)
SYM_NAME(hal_interrupt_handlers):
	.long	SYM_NAME(hal_default_isr)
	.long	SYM_NAME(hal_default_isr)


	.globl	SYM_NAME(hal_interrupt_data)
SYM_NAME(hal_interrupt_data):
	.long	0
	.long	0

	.globl	SYM_NAME(hal_interrupt_objects)
SYM_NAME(hal_interrupt_objects):
	.long	0
	.long	0

#endif

##-----------------------------------------------------------------------------
## end of vectors.S


