##=============================================================================
##
##	vectors.S
##
##	H8/300 exception vectors
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
## Author(s): 	Yoshinori Sato
## Contributors: Yoshinori Sato
## Date:	2002-02-14
## Purpose:	H8/300 exception vectors
## Description:	This file defines the code placed into the exception
##              vectors. It also contains the first level default VSRs
##		that save and restore state for both exceptions and
##		interrupts.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#endif
	
#ifdef CYGPKG_KERNEL

#include <pkgconf/kernel.h>
	
#endif		

#include <cyg/hal/arch.inc>	
#include <cyg/hal/basetype.h>	
#include <cyg/hal/variant.inc>
			
	
##-----------------------------------------------------------------------------
	
#ifdef CYGPKG_HAL_H8300_H8300H
	.h8300h
	.macro	shll2 reg
	shll.l	\reg
	shll.l	\reg
	.endm
	.macro	shlr2 reg
	shlr.l	\reg
	shlr.l	\reg
	.endm
#endif
#ifdef CYGPKG_HAL_H8300_H8S
	.h8300s
	.macro	shll2 reg
	shll.l	#2,\reg
	.endm
	.macro	shlr2 reg
	shlr.l	#2,\reg
	.endm
#endif
	
#ifdef CYGPKG_CYGMON
##-----------------------------------------------------------------------------
## Macros for Stack handling when running Cygmon
        		
.macro hal_cygmon_switch_app_stack
#ifdef CYGPKG_HAL_H8300_H8300H
	; Switch to interrupt stack to handle exception

	; First, save some scratch registers
	mov.l	er0,@-sp
	mov.l	er1,@-sp

	; Copy the exception frame
	mov.l	#__cygmon_interrupt_stack,er0
	mov.l	@(8:16,sp),er1
	mov.l	er1,@-er0

	; Save the pre-exception sp in the register image	 
	mov.l	sp,@-er0

	; Actually switch the stack
	mov.l	er0,sp
	
	mov.l	@sp,er0
	adds	#4,er0
	adds	#4,er0
	mov.l	er0,@sp

	; Now, restore the scratch registers
	mov.l	@(-4:16,er0),er1
	mov.l	@(-8:16,er0),er0
#endif
#ifdef CYGPKG_HAL_H8300_H8S
	; Switch to interrupt stack to handle exception

	; First, save some scratch registers
	stm.l	er0-er1,@-sp
	
	; Copy the exception frame
	mov.l	#__cygmon_interrupt_stack,er0
	mov.l	@(10,sp:16),er1
	mov.l	er1,@-er0
	mov.w	@(10,sp:16),r1
	mov.w	r1,@-er0

	; Save the pre-exception sp in the register image	 
	mov.l	sp,@-er0

	; Actually switch the stack
	mov.l	er0,sp
	
	mov.l	@sp,er0
	adds	#4,er0
	adds	#4,er0
	mov.l	er0,@sp

	; Now, restore the scratch registers
	mov.l	@(-4:16,er0),er1
	mov.l	@(-8:16,er0),er0
#endif
.endm

.macro hal_cygmon_restore_app_stack
	// For cygmon we are switching stacks immediately on exception.
	// We must wait until the very end before restoring the original stack.

#ifdef CYGPKG_HAL_H8300_H8300H
	; Save some scratch registers
	subs	#4,sp
	mov.l	er0,@-sp
	mov.l	er1,@-sp

	; We need to restore the application stack pointer, but we also
	; need to restore the exception frame.
	mov.l	@(12:16,sp),er0
	mov.l	@(16:16,sp),er1
	mov.l	er1,@er0
	mov.l	er0,@(8:16,sp)

	; Restore the scratch registers
	mov.l	@sp+,er1
	mov.l	@sp+,er0
	mov.l	@sp+,sp			// Restore the frame-adjusted SP
#endif
#ifdef CYGPKG_HAL_H8300_H8S
	; Save some scratch registers
	subs	#4,sp
	stm.l	er0-er1,@-sp

	; We need to restore the application stack pointer, but we also
	; need to restore the exception frame.
	mov.l	@(12:16,sp),er0
	mov.w	@(16:16,sp),r1
	mov.w	r1,@er0
	mov.l	@(18:16,sp),er1
	mov.l	er1,@er0
	mov.l	er0,@(8:16,sp)

	; Restore the scratch registers
	ldm.l	@sp+,er0-er1
	mov.l	@sp+,sp			// Restore the frame-adjusted SP
#endif
.endm

#endif // CYGPKG_CYGMON

##-----------------------------------------------------------------------------
		
#if (defined(CYG_HAL_STARTUP_ROM) || \
     defined(CYGPKG_HAL_H8300_H8300H_SIM) || \
     defined(CYGPKG_HAL_H8300_H8S_SIM))
    
	.section .vectors,"a"
	
	.globl	reset_vector

reset_vector:
	.long	CYG_LABEL_DEFN(_start)
	.long	CYG_LABEL_DEFN(_start)
vector	=	2
	.rept	CYG_ISR_TABLE_SIZE-2
	.long	interrupt_redirect_table+vector*4
vector	=	vector + 1
	.endr
#endif

#if !defined(CYGSEM_HAL_H8300_VECTOR_HOOK)
	.section .int_hook_table,"ax"
interrupt_redirect_table:	
vector	=	0
	.rept	CYG_ISR_TABLE_SIZE
	jsr	@interrupt_entry
vector	=	vector + 1
	.endr
#else
	.section .int_hook_table,"x"
interrupt_redirect_table:	
	.space	4*CYG_ISR_TABLE_SIZE
#endif

##-----------------------------------------------------------------------------
## Startup code
			
	.text

	.globl	CYG_LABEL_DEFN(_start)
CYG_LABEL_DEFN(_start):
	; set up stack

	mov.l	#__interrupt_stack,sp
	
	; Initialize hardware
	hal_cpu_init
	hal_mmu_init
	hal_memc_init
	hal_diag_init
	hal_intc_init
	hal_cache_init
	hal_timer_init
	hal_mon_init

#ifdef CYG_HAL_STARTUP_ROM
	
	; Copy data from ROM to RAM

	mov.l	#CYG_LABEL_DEFN(__rom_data_start),er5
	mov.l	#CYG_LABEL_DEFN(__ram_data_start),er6
	mov.l	#CYG_LABEL_DEFN(__ram_data_end),er4
	sub.l	er6,er4
	shlr2	er4
1:	
	mov.l	@er5+,er0
	mov.l	er0,@er6
	adds	#4,er6
	dec.l	#1,er4
	bne	1b

#endif

#if defined(CYGSEM_HAL_H8300_VECTOR_HOOK)
	; Setup Interrupt Vector (virtual)

	;; check rom table
	mov.l	#8*4,er0
	mov.l	@er0+,er1
	adds	#4,er1
	mov.b	#3,r3l
1:
	mov.l	@er0+,er2
	cmp.l	er1,er2
	bne	3f
	adds	#4,er1
	dec.b	r3l
	bne	1b
	;; calculate vector address
	mov.l	@8*4,er0
	and.w	#0xff,e0
	sub.l	#8*4,er0
	;; build ram vector
	mov.l	er0,@__interrupt_table
	mov.l	#0x5e000000+interrupt_entry,er1
	mov.l	#save_vector_table,er3
	mov.b	#0,r2l
2:
	;; check saved vector
	mov.b	@er3+,r2h
	cmp.b	r2l,r2h
	beq	5f
	subs	#1,er3
	mov.l	er1,@er0
5:	
	adds	#4,er0
	inc.b	r2l
	cmp.b	#CYG_ISR_TABLE_SIZE,r2l
	bcs	2b
	bra	4f
3:
	bra	3b
4:	
		
#else
	mov.l	#interrupt_redirect_table,er0
	mov.l	er0,@__interrupt_table
#endif
	
	; Clear BSS
	mov.l	#CYG_LABEL_DEFN(__bss_start),er5
	mov.l	#CYG_LABEL_DEFN(__bss_end),er4
	sub.l	#CYG_LABEL_DEFN(__bss_start),er4
	shlr2	er4
	sub.l	er0,er0
1:
	mov.l	er0,@er5
	adds	#4,er5
	dec.l	#1,er4
	bne	1b
	
	; Call variant and platform HAL
	; initialization routines.

	.extern CYG_LABEL_DEFN(hal_variant_init)
	jsr	@CYG_LABEL_DEFN(hal_variant_init)

	.extern CYG_LABEL_DEFN(hal_platform_init)
	jsr	@CYG_LABEL_DEFN(hal_platform_init)
	
	; Call constructors

	.extern CYG_LABEL_DEFN(cyg_hal_invoke_constructors)
	jsr	@CYG_LABEL_DEFN(cyg_hal_invoke_constructors)

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        .extern CYG_LABEL_DEFN(initialize_stub)
        jsr	@CYG_LABEL_DEFN(initialize_stub)
#endif

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
	.extern CYG_LABEL_DEFN(hal_ctrlc_isr_init)
	jsr	@CYG_LABEL_DEFN(hal_ctrlc_isr_init)
#endif


       	; Call cyg_start
	sub.l	er0,er0
		
	.extern CYG_LABEL_DEFN(cyg_start)
	jsr	@CYG_LABEL_DEFN(cyg_start)

9:
	bra	9b		; Loop if we return

save_vector_table:	
#if defined(CYGSEM_HAL_H8300_SAVE_STUB_VECTOR)
	.byte	5,11
#if defined(CYGPKG_HAL_H8300_SAVED_VECTORS)
	h8300_save_vectors
#endif
#endif
	.byte	0		; end maker
	.align	2
	
interrupt_entry:
	mov.l	er0,@-sp
	mov.l	@(4:16,sp),er0	; interrupt vector offset
	mov.l	er1,@-sp
	mov.l	@__interrupt_table,er1
	sub.l	er1,er0
	shlr2	er0
	dec.l	#1,er0
	mov.l	er0,@CYG_LABEL_DEFN(_intvector)
	mov.l	@sp+,er1
	mov.l	@sp+,er0
	adds	#4,sp
#ifdef CYGPKG_CYGMON
	hal_cygmon_switch_app_stack
#endif
	hal_cpu_save_all
	h8300_var_interrupt_entry		; load isr address
	jmp	@er0

##-----------------------------------------------------------------------------
## The following macros are defined depending on whether the HAL is configured
## to support the kernel or not. 

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
	
	.extern	CYG_LABEL_DEFN(cyg_scheduler_sched_lock)
	
	; Increment the scheduler lock
			
	.macro increment_sched_lock reg=er0
	mov.l	@CYG_LABEL_DEFN(cyg_scheduler_sched_lock),\reg
	inc.l	#1,\reg
	mov.l	\reg,@CYG_LABEL_DEFN(cyg_scheduler_sched_lock)
	.endm

#else
	
	.macro increment_sched_lock reg=er0
	.endm

#endif		

##-----------------------------------------------------------------------------
## Default interrupt VSR

	.text
	.globl	CYG_LABEL_DEFN(__default_interrupt_vsr)
CYG_LABEL_DEFN(__default_interrupt_vsr):

	; We come here with all the registers pushed
	; onto the stack.

	hal_diag_intr_start
	
	increment_sched_lock

#if defined(CYGPKG_CYGMON)
        // For Cygmon, we saved this back when we originally switched stacks.
	mov.l	sp,er1				; A2 = saved thread state
#elif defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK)
	; Increment interrupt nesting counter
	mov.l	sp,er1
	mov.l	#__interrupt_stack,er0		; A0 = interrupt stack top
	cmp.l	#__interrupt_stack_base,sp	; compare with base of stack
	blt	1f				; if lt switch to int stack
	cmp.l	er0,sp				; compare sp with stack top
	ble	8f				; if le already on istack
1:
	mov.l	er0,sp				; switch to new SP
8:
	mov.l	er1,@-sp			; save old SP
	
#else
	mov	sp,er1				; A2 = saved thread state
#endif
	mov.l	er1,er4

	; Here D3 contains the table byte offset of the vector to
	; call.

#if defined(CYGPKG_KERNEL_INSTRUMENT) && defined(CYGDBG_KERNEL_INSTRUMENT_INTR)

	; Call cyg_instrument to record that this interrupt is being raised.

	.extern CYG_LABEL_DEFN(cyg_instrument)
	mov.l	#0x0301,er0			; type = INTR,RAISE
	mov.l	@CYG_LABEL_DEFN(_intvector),er2		; arg2 = table offset
	jsr	CYG_LABEL_DEFN(cyg_instrument)	; call instrumentation
	
#endif		

#ifdef CYGSEM_HAL_COMMON_INTERRUPTS_ALLOW_NESTING

	; To allow nested interrupts, we set the IE bit. We do
	; not touch the IPL bits, so only higher priority interrupts
	; will be nested on top of us. Also, new interrupts will not
	; be delivered until the ISR calls 
	; Cyg_Interrupt::acknowledge_interrupt(). At some future point
	; we may want to do the ack stuff here to allow immediate nesting.
	
	mov.l	@CYG_LABEL_DEFN(_intvector),er0
	mov.b	@(CYG_LABEL_DEFN(cyg_hal_level_table),er0),r0l
	hal_cpu_set_int_level
#endif
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) || \
    defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)

	; If we have Ctrl-C support enabled, save a pointer to the
	; saved CPU state here so we can plant a breakpoint there if
	; this is a ^C.
	
	.extern CYG_LABEL_DEFN(hal_saved_interrupt_state)
	mov.l	er4,@CYG_LABEL_DEFN(hal_saved_interrupt_state)
	
#endif
	mov.l	@CYG_LABEL_DEFN(_intvector),er0
	shll2	er0
	mov.l	@(CYG_LABEL_DEFN(hal_interrupt_handlers),er0),er2
	
	mov.l	@(CYG_LABEL_DEFN(hal_interrupt_data),er0),er1

	shlr2	er0

	jsr	@er2

	; on return d0 bit 1 will indicate whether a DSR is
	; to be posted. Pass this together with a pointer to
	; the interrupt object we have just used to the
	; interrupt tidy up routine.
	
	; D3 is defined to be saved across procedure calls, and
	; should still contain the vector byte index. Similarly,
	; A2 should still point to the saved machine state.

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
        ; If interrupt was caused by GDB, the ISR call above
        ; is skipped by jumping here.
2:
#endif

#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK) && !defined(CYGPKG_CYGMON)

	; If we are returning from the last nested interrupt, move back
	; to the thread stack. interrupt_end() must be called on the
	; thread stack since it potentially causes a context switch.
	
	mov.l	@sp+,sp				; pop old sp

#endif		

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
	
	; We only need to call _interrupt_end() when there is a kernel
	; present to do any tidying up.
		
	; Using the vector offset in D3, get the interrupt object pointer
	; into D1.
	mov.l	@CYG_LABEL_DEFN(_intvector),er1
	shll2	er1
	mov.l	@(CYG_LABEL_DEFN(hal_interrupt_objects),er1),er1

	; Even when this is not the last nested interrupt, we must call
	; _interrupt_end() to post the DSR and decrement the scheduler
	; lock.
		
	mov.l	er3,er2				; arg3 = saved state.
	
	jsr	@CYG_LABEL_DEFN(interrupt_end)	; call interrupt end fn		
#endif


#	show_interrupts

	hal_diag_restore

	hal_cpu_load_all
#ifdef CYGPKG_CYGMON
	hal_cygmon_restore_app_stack
#endif
        mov.l   er0,@-sp
        mov.l   @(4:16,sp),er0
        mov.l   er0,@__lastpc
        mov.l   @sp+,er0
        
	rte					; and return

	.section	.bss
	.global	CYG_LABEL_DEFN(_intvector)
CYG_LABEL_DEFN(_intvector):
	.long	0
        .global __lastpc        
__lastpc:
        .long   0
__interrupt_table:	
	.long	0

	.text	
##-----------------------------------------------------------------------------
## Execute pending DSRs on the interrupt stack with interrupts enabled.
## Note: this can only be called from code running on a thread stack
	
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
	.extern CYG_LABEL_DEFN(cyg_interrupt_call_pending_DSRs)
	.global CYG_LABEL_DEFN(hal_interrupt_stack_call_pending_DSRs)

CYG_LABEL_DEFN(hal_interrupt_stack_call_pending_DSRs):
	h8300_var_call_pending_DSR
#endif		
		
##-----------------------------------------------------------------------------
## Default TRAP VSR

	.text
	.globl	CYG_LABEL_DEFN(__default_trap_vsr)
CYG_LABEL_DEFN(__default_trap_vsr):

#ifdef CYG_HAL_DIAG_EXCPT_END
	hal_diag_excpt_start
#endif
	mov.l	@CYG_LABEL_DEFN(_intvector),er1
	cmp.l	#11,er1		; Adjust trap entry address
	bne	1f
	h8300_trap_adjust_pc
1:	
	mov.l	sp,er0
	jsr	@CYG_LABEL_DEFN(cyg_hal_exception_handler)

#ifdef CYG_HAL_DIAG_EXCPT_END
	hal_diag_excpt_end
#endif
	hal_cpu_load_all
#ifdef CYGPKG_CYGMON
	hal_cygmon_restore_app_stack
#endif
	rte

##-----------------------------------------------------------------------------
## VSR table. The VSRs pointed to by this table are called from the stubs
## connected to the hardware.

#ifndef CYG_HAL_H8300_VSR_TABLE_DEFINED
	
	.data

	.globl	CYG_LABEL_DEFN(hal_vsr_table)
CYG_LABEL_DEFN(hal_vsr_table):	
	.long	CYG_LABEL_DEFN(__default_trap_vsr)
	.long	CYG_LABEL_DEFN(__default_trap_vsr)
	.long	0
	.long	0
	.long	0
	.long	CYG_LABEL_DEFN(__default_trap_vsr)
	.long	CYG_LABEL_DEFN(__default_trap_vsr)
	.long	CYG_LABEL_DEFN(__default_trap_vsr)
	.long	CYG_LABEL_DEFN(__default_trap_vsr)
	.long	CYG_LABEL_DEFN(__default_trap_vsr)
	.long	CYG_LABEL_DEFN(__default_trap_vsr)
	.long	CYG_LABEL_DEFN(__default_trap_vsr)

	.rept	CYG_ISR_TABLE_SIZE-12
	.long	CYG_LABEL_DEFN(__default_interrupt_vsr)
	.endr
#endif
	
##-----------------------------------------------------------------------------
## Interrupt tables
	
	.data

	.extern CYG_LABEL_DEFN(hal_default_isr)
	
	.globl	CYG_LABEL_DEFN(hal_interrupt_handlers)
CYG_LABEL_DEFN(hal_interrupt_handlers):
	.rept	CYG_ISR_TABLE_SIZE
	.long	CYG_LABEL_DEFN(hal_default_isr)
	.endr
	
	.globl	CYG_LABEL_DEFN(hal_interrupt_data)
CYG_LABEL_DEFN(hal_interrupt_data):
	.rept	CYG_ISR_TABLE_SIZE
	.long	0
	.endr
	
	.globl	CYG_LABEL_DEFN(hal_interrupt_objects)
CYG_LABEL_DEFN(hal_interrupt_objects):
	.rept	CYG_ISR_TABLE_SIZE
	.long	0
	.endr
	
			
##-----------------------------------------------------------------------------
## Temporary interrupt stack
	
	.section	.bss

	.balign 2
	.global _cyg_interrupt_stack_base
_cyg_interrupt_stack_base:
__interrupt_stack_base:
	.rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
	.byte 0
	.endr
	.balign 2
	.global _cyg_interrupt_stack
_cyg_interrupt_stack:
__interrupt_stack:
	
#ifdef CYGPKG_CYGMON
	.balign 2
	.global __cygmon_interrupt_stack_base
__cygmon_interrupt_stack_base:
	.rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
	.byte 0
	.endr
	.balign 2
	.global __cygmon_interrupt_stack
__cygmon_interrupt_stack:
#endif

	.long	0,0,0,0,0,0,0,0	

##-----------------------------------------------------------------------------

	.data
	hal_diag_data


##-----------------------------------------------------------------------------
## end of vectors.S

