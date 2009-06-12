##=============================================================================
##
##	vectors.S
##
##	MN10300 exception vectors
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
## Author(s): 	nickg
## Contributors:	nickg, dmoseley, dhowells
## Date:	1997-10-16
## Purpose:	MN10300 exception vectors
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
		
	
##-----------------------------------------------------------------------------
	
//	.file	"vectors.S"

#ifdef CYGPKG_CYGMON
##-----------------------------------------------------------------------------
## Macros for Stack handling when running Cygmon
        		
.macro hal_cygmon_switch_app_stack
	#
	# Switch to interrupt stack to handle exception
	#

	# First, save some scratch registers
	mov	a2(a2_save)
	mov	a3,(a3_save)

	# Copy the exception frame
	mov	__cygmon_interrupt_stack-8,a2
	mov	(sp),a3				// Copy over the saved EPSW
	mov	a3,(a2)
	mov	(4,sp),a3			// Copy over the saved PC
	mov	a3,(4,a2)

	# Save the pre-exception sp in the register image	 
	add	-4,a2				# space for SP
	mov	sp,a3
	add	8,a3				# adjust for exception frame
	mov	a3,(a2)

	# Actually switch the stack
	mov	a2,sp

	# Now, restore the scratch registers
	mov	(a2_save),a2
	mov	(a3_save),a3
.endm

.macro hal_cygmon_restore_app_stack
	// For cygmon we are switching stacks immediately on exception.
	// We must wait until the very end before restoring the original stack.

	# Save some scratch registers
	mov	a2,(a2_save)
	mov	a3,(a3_save)

	# We need to restore the application stack pointer, but we also
	# need to restore the exception frame.
	mov	(sp),a2
	add	4,sp				// remove saved sp
	add	-8,a2				// adjust for exception frame
	
	mov	(sp),a3				// Copy the saved EPSW
	mov	a3,(a2)
	mov	(4,sp),a3			// Copy the saved PC
	mov	a3,(4,a2)
	mov	a2,sp				// Restore the frame-adjusted SP

	# Restore the scratch registers
	mov	(a2_save),a2
	mov	(a3_save),a3
.endm

#endif // CYGPKG_CYGMON

##-----------------------------------------------------------------------------
#define EXCEPTION_HANDLER(VECTOR,NAME,HANDLER) .org VECTOR; \
NAME##_vector: \
jmp HANDLER
		
	.section ".vectors","ax"
	
	.globl	reset_vector

	.org 0x000	
reset_vector:
	# Reset vector
	jmp	_start

EXCEPTION_HANDLER(0x008, nmi					,__nmi)
EXCEPTION_HANDLER(0x010, trap					,__trap)
EXCEPTION_HANDLER(0x100, ins_tlb_miss_exception			,__unknown)
EXCEPTION_HANDLER(0x108, data_tlb_miss_exception		,__unknown)
EXCEPTION_HANDLER(0x110, ins_access_exception			,__unknown)
EXCEPTION_HANDLER(0x118, data_access_exception			,__unknown)
EXCEPTION_HANDLER(0x160, priv_ins_exception			,__unknown)
EXCEPTION_HANDLER(0x168, unimpl_ins_exception			,__unknown)
EXCEPTION_HANDLER(0x170, unimpl_ext_ins_exception		,__unknown)
EXCEPTION_HANDLER(0x180, misalignment_exception			,__unknown)
EXCEPTION_HANDLER(0x188, bus_error				,__unknown)
EXCEPTION_HANDLER(0x190, illegal_ins_access_exception		,__unknown)
EXCEPTION_HANDLER(0x198, illegal_data_access_exception		,__unknown)
EXCEPTION_HANDLER(0x1a0, iospace_ins_access_exception		,__unknown)
EXCEPTION_HANDLER(0x1a8, priv_space_ins_access_exception	,__unknown)
EXCEPTION_HANDLER(0x1b0, priv_space_data_access_exception	,__unknown)
EXCEPTION_HANDLER(0x1b8, data_space_ins_access_exception	,__unknown)
EXCEPTION_HANDLER(0x1c0, fpu_disabled_exception			,__unknown)
EXCEPTION_HANDLER(0x1c8, fpu_unimplemented_exception		,__unknown)
EXCEPTION_HANDLER(0x1d0, fpu_operation_exception		,__unknown)
EXCEPTION_HANDLER(0x200, double_fault				,__unknown)
EXCEPTION_HANDLER(0x240, wdt_overflow				,__unknown)
EXCEPTION_HANDLER(0x248, nmi_pin				,__nmi)
EXCEPTION_HANDLER(0x280, maskable_int0				,__unknown)
EXCEPTION_HANDLER(0x288, maskable_int1				,__unknown)
EXCEPTION_HANDLER(0x290, maskable_int2				,__unknown)
EXCEPTION_HANDLER(0x298, maskable_int3				,__unknown)
EXCEPTION_HANDLER(0x2a0, maskable_int4				,__unknown)
EXCEPTION_HANDLER(0x2a8, maskable_int5				,__unknown)
EXCEPTION_HANDLER(0x2b0, maskable_int6				,__unknown)
		
	// We need to put the syscall handler at 0x300
	// For cygmon, we will use the am33 "syscall 0" as the syscall trap.
EXCEPTION_HANDLER(0x300, syscall0				,__nmi)
EXCEPTION_HANDLER(0x308, syscall1				,__unknown)
EXCEPTION_HANDLER(0x310, syscall2				,__unknown)
EXCEPTION_HANDLER(0x318, syscall3				,__unknown)
EXCEPTION_HANDLER(0x320, syscall4				,__unknown)
EXCEPTION_HANDLER(0x328, syscall5				,__unknown)
EXCEPTION_HANDLER(0x330, syscall6				,__unknown)
EXCEPTION_HANDLER(0x338, syscall7				,__unknown)
EXCEPTION_HANDLER(0x340, syscall8				,__unknown)
EXCEPTION_HANDLER(0x348, syscall9				,__unknown)
EXCEPTION_HANDLER(0x350, syscalla				,__unknown)
EXCEPTION_HANDLER(0x358, syscallb				,__unknown)
EXCEPTION_HANDLER(0x360, syscallc				,__unknown)
EXCEPTION_HANDLER(0x368, syscalld				,__unknown)
EXCEPTION_HANDLER(0x370, syscalle				,__unknown)
EXCEPTION_HANDLER(0x378, syscallf				,__unknown)

##-----------------------------------------------------------------------------
## unknown interrupt or exception vector
__unknown:
#ifdef CYG_HAL_HANDLE_UNKNOWN_INT_DEFINED
	hal_handle_unknown_int
#endif

##-----------------------------------------------------------------------------
## NMI vector

	.globl __nmi
__nmi:
#ifdef CYGPKG_CYGMON
	hal_cygmon_switch_app_stack
#else
	add	-4,sp				# space for SP
#endif
	hal_cpu_save_all			# push all registers
	mov	(_hal_vsr_table+28),a0 
	jmp	(a0)

##-----------------------------------------------------------------------------
## Trap vector

	.globl __trap
__trap:
#ifdef CYGPKG_CYGMON
	add	-4,sp				# space for PSW
	hal_cygmon_switch_app_stack
#else
	add	-8,sp				# space for SP and PSW
#endif
	hal_cpu_save_all			# push all registers
	mov	(_hal_vsr_table+32),a0
	jmp	(a0)
		
##-----------------------------------------------------------------------------
## Macro to define a hardware VSR	

	.macro	hardware_vector name
	.globl	__hardware_vector_\name
__hardware_vector_\name:
#ifdef CYGPKG_CYGMON
	hal_cygmon_switch_app_stack
#else
	add	-4,sp				# space for SP
#endif
	hal_cpu_save_all
	mov	\name,d0			# d0 = vector id
	mov	d0,(0,sp)			# save in dummy stack loc
	mov	_hal_vsr_table,a0		# a0 = vsr table
	add	\name<<2,a0			# a0 = &required vsr
	mov	(a0),a0				# a0 = required vsr
	jmp	(a0)				# go there
	.endm	

##-----------------------------------------------------------------------------
## Hardware Vectors. These are pointed to by the registers in
## mn10300_interrupt_vectors and must be in the range 0x4000XXXX.

	hardware_vector	0
	hardware_vector	1
	hardware_vector	2
	hardware_vector	3
	hardware_vector	4
	hardware_vector	5
	hardware_vector	6
		
##-----------------------------------------------------------------------------
## Startup code
			
	.text

	.globl	_start
_start:

#ifdef HAL_EARLY_INIT
	HAL_EARLY_INIT
#endif
	# set up stack

	mov	__interrupt_stack,a0
	sub	8,a0
	mov	a0,sp

	# Initialize hardware
	hal_cpu_init
	hal_mmu_init
	hal_memc_init
	hal_diag_init
	hal_intc_init
	hal_cache_init
	hal_timer_init
	hal_mon_init

#ifdef CYG_HAL_STARTUP_ROM
	
	# Copy data from ROM to RAM

	mov	__rom_data_start,a0
	mov	__ram_data_end,a1
	mov	__ram_data_start,a2
	
	cmp	a2,a1
	beq	8f
1:	
	movbu	(a0),d0
	movbu	d0,(a2)
	inc	a0
	inc	a2
	cmp	a2,a1
	bne	1b	
8:	
	
#endif
	
	# Clear BSS
	mov	__bss_start,a0
	mov	__bss_end,a1

	cmp	a0,a1
	beq	8f
	clr	d0
1:	
	movbu	d0,(a0)
	inc	a0
	cmp	a0,a1
	bne	1b	
8:	

	# Call variant and platform HAL
	# initialization routines.

	.extern _hal_variant_init
	call    _hal_variant_init,[],0

	.extern _hal_platform_init
	call	_hal_platform_init,[],0
	
	# Call constructors

	.extern _cyg_hal_invoke_constructors
	call    _cyg_hal_invoke_constructors,[],0

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        .extern _initialize_stub
        call    _initialize_stub,[],0
#endif

#ifdef CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT
	.extern hal_ctrlc_isr_init
	call    hal_ctrlc_isr_init,[],0
#endif

       	# Call cyg_start
	clr	d0
	mov	d0,(4,sp)
	
	.extern _cyg_start
	call    _cyg_start,[],0

9:
	bra	9b		# Loop if we return

##-----------------------------------------------------------------------------
## The following macros are defined depending on whether the HAL is configured
## to support the kernel or not. 

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
	
	.extern	_cyg_scheduler_sched_lock
	
	# Increment the scheduler lock
			
	.macro increment_sched_lock reg=d0
	mov	(_cyg_scheduler_sched_lock),\reg
	inc	\reg
	mov	\reg,(_cyg_scheduler_sched_lock)
	.endm

#else
	
	.macro increment_sched_lock reg=d0
	.endm

#endif		

##-----------------------------------------------------------------------------
## Default interrupt VSR

	.text
	.globl	__default_interrupt_vsr
__default_interrupt_vsr:

	# We come here with all the registers pushed
	# onto the stack.

	hal_diag_intr_start
	
	increment_sched_lock
	
#if defined(CYGPKG_CYGMON)
        // For Cygmon, we saved this back when we originally switched stacks.
	mov	(sp),a2				# A2 = saved thread state
#elif defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK)
	# Increment interrupt nesting counter
	mov	__interrupt_stack,a0		# A0 = interrupt stack top
	mov	sp,a2				# A2 = SP
	cmp	__interrupt_stack_base,a2	# compare with base of stack
	blt	1f				# if lt switch to int stack
	cmp	a0,a2				# compare sp with stack top
	ble	8f				# if le already on istack
1:
	mov	a0,sp				# switch to new SP
8:
	movm	[a2],(sp)			# save old SP

#else
	mov	sp,a2				# A2 = saved thread state
#endif

	# Here A2 -> saved thread state on the threads own
	# stack. We will be executing either on the same stack
	# or on the interrupt stack, depending on config options.
		
	hal_intc_decode d3,a3,d0

	# Here D3 contains the table byte offset of the vector to
	# call.

#if defined(CYGPKG_KERNEL_INSTRUMENT) && defined(CYGDBG_KERNEL_INSTRUMENT_INTR)

	# Call cyg_instrument to record that this interrupt is being raised.

	.extern _cyg_instrument		
	add	-16,sp				# make space for return link + args	
	mov	0x0301,d0			# d0 = type = INTR,RAISE
	mov	(0,a2),d1			# d1 = arg1 = vector
	mov	d3,(12,sp)			# (12,sp) = arg2 = table offset
	calls	_cyg_instrument			# call instrumentation
	add	16,sp				# pop space
	
#endif		

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
        .extern _cyg_hal_gdb_isr
        add     -16,sp                          # make space for return link + args
        mov     (0,a2),d0                       # d0 = vector
        mov     (SAVED_CONTEXT_PC_OFFSET,a2),d1 # d1 = pc
        calls   _cyg_hal_gdb_isr                # call GDB isr function
        cmp     0x00,d0                         # Call ISR proper?
        beq     2f                              # (d0 is 0 when skipping
                                                #  to avoid DSR call)
        add     16,sp                           # pop space
#endif
	
#ifdef CYGSEM_HAL_COMMON_INTERRUPTS_ALLOW_NESTING

	# To allow nested interrupts, we set the IE bit. We do
	# not touch the IPL bits, so only higher priority interrupts
	# will be nested on top of us. Also, new interrupts will not
	# be delivered until the ISR calls 
	# Cyg_Interrupt::acknowledge_interrupt(). At some future point
	# we may want to do the ack stuff here to allow immediate nesting.
	
	or	0x0800,psw
	
#endif
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)

	# If we have Ctrl-C support enabled, save a pointer to the
	# saved CPU state here so we can plant a breakpoint there if
	# this is a ^C.
	
	.extern _hal_saved_interrupt_state
	mov	a2,(_hal_saved_interrupt_state)
	
#endif
		
	mov	_hal_interrupt_handlers,a0	# a0 = isr table
	mov	(d3,a0),a0			# a0 = isr
	
	mov	_hal_interrupt_data,a1		# a1 = data table
	mov	(d3,a1),d1			# d1 = isr data

	mov	(0,a2),d0			# d0 = vector. (d3 is...
						# ...adjusted for table reads)

	add	-16,sp				# make space for return link

	calls	(a0)				# call isr

	# on return d0 bit 1 will indicate whether a DSR is
	# to be posted. Pass this together with a pointer to
	# the interrupt object we have just used to the
	# interrupt tidy up routine.
	
	# D3 is defined to be saved across procedure calls, and
	# should still contain the vector byte index. Similarly,
	# A2 should still point to the saved machine state.

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
        # If interrupt was caused by GDB, the ISR call above
        # is skipped by jumping here.
2:
#endif

#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK) && !defined(CYGPKG_CYGMON)

	# If we are returning from the last nested interrupt, move back
	# to the thread stack. interrupt_end() must be called on the
	# thread stack since it potentially causes a context switch.
	
	add	16,sp				# pop call frame from int stack
	movm	(sp),[a3]			# pop old sp
	mov	a3,sp				# put in SP

	add	-16,sp				# make call frame for call

#endif		

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
	
	# We only need to call _interrupt_end() when there is a kernel
	# present to do any tidying up.
		
	# Using the vector offset in D3, get the interrupt object pointer
	# into D1.
	mov	_hal_interrupt_objects,a0	# a0 = object table
	mov	(d3,a0),d1			# d1 = object

	# Even when this is not the last nested interrupt, we must call
	# _interrupt_end() to post the DSR and decrement the scheduler
	# lock.
		
	mov	a2,(12,sp)			# arg3 = saved state.
	
	calls	_interrupt_end			# call interrupt end fn		
#endif

	add	16,sp				# pop return link

#	show_interrupts

	hal_diag_restore
	
	hal_cpu_load_all
#ifdef CYGPKG_CYGMON
	hal_cygmon_restore_app_stack
#else
	add	4,sp
#endif
	rti					# and return

##-----------------------------------------------------------------------------
## Execute pending DSRs on the interrupt stack with interrupts enabled.
## Note: this can only be called from code running on a thread stack
	
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
	.extern _cyg_interrupt_call_pending_DSRs
	.global _hal_interrupt_stack_call_pending_DSRs

_hal_interrupt_stack_call_pending_DSRs:		
	movm	[d2,a2],(sp)			# save some work regs
	mov	__interrupt_stack,a0		# interrupt stack pointer
	mov	sp,a2				# save current SP
	mov	a0,sp				# set SP to interrupt stack
	hal_cpu_get_psw d2			# save PSW
	hal_cpu_int_enable			# enable interrupts

	# Call kernel code to invoke DSRs.
	calls	_cyg_interrupt_call_pending_DSRs	

	# On return the old SP in a2 and the old PSW in d2 will 
	# have been preserved by the calling conventions.
	
	hal_cpu_int_merge d2			# Restore previous PSW
	mov	a2,sp				# restore old SP
	movm	(sp),[d2,a2]			# Retrieve old work regs
	ret	[],0				# and return
		
#endif		
		
##-----------------------------------------------------------------------------
## Default NMI VSR

	.globl	_cyg_hal_exception_handler
	
	.text
	.globl	__default_nmi_vsr
__default_nmi_vsr:

	# We come here with all the registers saved
	# on the stack.

#ifdef CYG_HAL_DIAG_EXCPT_END
	hal_diag_excpt_start
#endif
	
	# Decode the cause of the NMI and cancel all the bits in all
	# the registers. We need to clear any bits set in the ISR and
	# then clear any bits set in the NMICR. Note that we can only 
	# access the ISR if the DCR:DE bit is set. 
	
	movhu	(NMICR),d0			# D0 = NMI Control register

	movhu	(DCR),d1			# D1 = Debug Control Register
	mov	d1,d2				# D2 = copy of DCR
	movhu	(ISR),d3			# D3 = Interrupt Status Register

	or	DCR_DE,d1			# Set DE bit	
	movhu	d1,(DCR)			

	movhu	d3,(ISR)			# clear ISR bits

	movhu	d2,(DCR)			# restore DCR to original value

	movhu	d0,(NMICR)			# clear NMI bits
	and	0x7,d0				# LS 3 bits only

	mov	hal_lsbit_table,a0		
	movbu	(d0,a0),d1			# D1 = NMI code
						## 0 = NMI
						## 1 = Watchdog
						## 2 = System Error

	add	9,d1				# offset into exception number space
	
#if defined(CYGPKG_CYGMON) || defined(CYGSEM_REDBOOT_BSP_SYSCALLS)
	# Check to see if this was a syscall.
	# If so, set d1 appropriately
#define TBR 0xC0000024
	mov	(TBR), d0
	and	0x00FFFFFF, d0
	cmp	0x300, d0
	bne	0f

	# This was apparently a syscall.
	mov     SIGSYSCALL, d1

0:	
#endif
	hal_cpu_clear_nmid

	mov	d1,(sp)				# save initial vector in spare save slot
        
	mov	sp,a1				# a1 = saved state

	mov	a1,d0				# d0 = arg1 = saved state

	add	-16,sp				# return link + args
	mov	d3,d1				# d1 = arg2 = ISR value

	calls	_cyg_hal_exception_handler	# call C code

	add	16,sp				# pop args

#ifdef CYG_HAL_DIAG_EXCPT_END
	hal_diag_excpt_end
#endif

	hal_cpu_load_all

#ifdef CYGPKG_CYGMON
	hal_cygmon_restore_app_stack
#else
	add	4,sp
#endif

	rti

	.globl hal_lsbit_table
hal_lsbit_table:
	.byte	0, 0, 1, 0
	.byte	2, 0, 1, 0
	.byte	3, 0, 1, 0
	.byte	2, 0, 1, 0
	
#ifdef CYGPKG_CYGMON
        .section ".bss"
d0_save:
	.long	0
d1_save:
	.long	0
d2_save:
	.long	0
d3_save:
	.long	0
a0_save:
	.long	0
a1_save:
	.long	0
a2_save:
	.long	0
a3_save:
	.long	0
sp_save:
	.long	0
#endif

##-----------------------------------------------------------------------------
## Default TRAP VSR

	.text
	.globl	__default_trap_vsr
__default_trap_vsr:

#ifdef CYG_HAL_DIAG_EXCPT_END
	hal_diag_excpt_start
#endif

	# We come here with all the registers saved
	# on the stack.
	mov	8,d1
	mov	d1,(sp)				# set the VSR vector number
	mov	sp,a1				# a1 = saved state
	mov	a1,d0				# d0 = arg1 = saved state

	add	-16,sp				# return link + arg
	mov	3,d1				# d1 = arg2 = ISR value (3 == TRAP trap)
	calls	_cyg_hal_exception_handler	# call C code
	add	16,sp				# pop args

#ifdef CYG_HAL_DIAG_EXCPT_END
	hal_diag_excpt_end
#endif
	hal_cpu_load_all
#ifdef CYGPKG_CYGMON
	hal_cygmon_restore_app_stack
	add	4,sp
#else
	add	8,sp
#endif
	rets

##-----------------------------------------------------------------------------
## VSR table. The VSRs pointed to by this table are called from the stubs
## connected to the hardware.

#ifndef CYG_HAL_MN10300_VSR_TABLE_DEFINED
	
	.data

	.globl	_hal_vsr_table
_hal_vsr_table:	
	.long	__default_interrupt_vsr
	.long	__default_interrupt_vsr
	.long	__default_interrupt_vsr
	.long	__default_interrupt_vsr
	.long	__default_interrupt_vsr
	.long	__default_interrupt_vsr
	.long	__default_interrupt_vsr
	.long	__default_nmi_vsr
	.long	__default_trap_vsr

	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
	.long	__default_nmi_vsr
#endif
	
##-----------------------------------------------------------------------------
## Interrupt tables
	
	.data

	.extern _hal_default_isr
	
	.globl	_hal_interrupt_handlers
_hal_interrupt_handlers:
	.rept	CYG_ISR_TABLE_SIZE
	.long	_hal_default_isr
	.endr
	
	.globl	_hal_interrupt_data
_hal_interrupt_data:
	.rept	CYG_ISR_TABLE_SIZE
	.long	0
	.endr
	
	.globl	_hal_interrupt_objects
_hal_interrupt_objects:
	.rept	CYG_ISR_TABLE_SIZE
	.long	0
	.endr
	
			
##-----------------------------------------------------------------------------
## Temporary interrupt stack
	
	.section ".bss"

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
	
#ifdef CYGPKG_CYGMON
	.balign 16
	.global __cygmon_interrupt_stack_base
__cygmon_interrupt_stack_base:
	.rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
	.byte 0
	.endr
	.balign 16
	.global __cygmon_interrupt_stack
__cygmon_interrupt_stack:
#endif

	.long	0,0,0,0,0,0,0,0	

##-----------------------------------------------------------------------------

	.data
	hal_diag_data

##-----------------------------------------------------------------------------
## end of vectors.S

