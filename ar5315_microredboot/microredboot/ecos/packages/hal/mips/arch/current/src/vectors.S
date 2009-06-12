##=============================================================================
##
##	vectors.S
##
##	MIPS exception vectors
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
## Contributors:	nickg, dmoseley
## Date:	1998-02-04
## Purpose:	MIPS exception vectors
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
	
#ifdef at
#undef at
#endif
	.extern cyg_instrument

##-----------------------------------------------------------------------------
## Hardware supplied vectors
	
	.set	noreorder

	.section ".reset_vector","ax"

	# Reset vector at 0xBFC00000

FUNC_START(reset_vector)
#ifndef CYG_HAL_STARTUP_RAM
#  if defined(CYGPKG_HAL_RESET_VECTOR_FIRST_CODE)
	hal_reset_vector_first_code
#  endif
#  if defined(CYGPKG_HAL_EARLY_INIT)
	hal_early_init
#  endif
	# Decide whether this is an NMI, cold or warm boot.

	mfc0	k0,status		# get status reg
	lui	k1,0x0008		# isolate NMI bit
	and	k1,k1,k0		
	beqz	k1,1f			# skip if zero
	nop

	lar	k1,__nmi_entry		# jump to ROM nmi code
	jalr	k1
	nop
1:
	lui	k1,0x0010		# isolate soft reset bit
	and	k1,k1,k0		
	beqz	k1,2f			# skip if zero
	nop

	lar	k1,__warm_start		# jump to ROM warm_start code
	jr	k1
	nop
2:
	la	k0,INITIAL_CONFIG0	# Set up config0 register
	mtc0	k0,config0		# to disable cache
#endif	
	lar	v0,_start		# jump to start
#ifdef CYGARC_START_FUNC_UNCACHED
	CYGARC_ADDRESS_REG_UNCACHED(v0)
#endif

	jr	v0
	nop				# (delay slot)

FUNC_END(reset_vector)
	
	.section ".debug_vector","ax"
		
	# Debug vector at 0xBFC00200
	
FUNC_START(debug_vector)
	la	k0,32*4
	la	k1,hal_vsr_table	# Get VSR table
	lw	k1,32*4(k1)		# load debug vector
	jr	k1			# go there
	nop				# (delay slot)
FUNC_END(debug_vector)

	.section ".other_vector","ax"

	# Common vector at 0x80000080 or 0xBFC00180
	
FUNC_START(other_vector)
	mfc0	k0,cause		# K0 = exception cause
	nop
	andi	k0,k0,0x7F		# isolate exception code
	la	k1,hal_vsr_table	# address of VSR table
	add	k1,k1,k0		# offset of VSR entry
	lw	k1,0(k1)		# k1 = pointer to VSR
	jr	k1			# go there
	nop				# (delay slot)
FUNC_END(other_vector)
	
	.section ".utlb_vector","ax"

FUNC_START(utlb_vector)
	mfc0	k0,cause		# K0 = exception cause
	nop
	andi	k0,k0,0x7F		# isolate exception code
	la	k1,hal_vsr_table	# address of VSR table
	add	k1,k1,k0		# offset of VSR entry
	lw	k1,0(k1)		# k1 = pointer to VSR
	jr	k1			# go there
	nop				# (delay slot)
FUNC_END(utlb_vector)

##-----------------------------------------------------------------------------
## Startup code
	.text
	
FUNC_START(_start)

	# Initialize hardware
	hal_cpu_init
	hal_diag_init
	hal_mmu_init
	hal_fpu_init
        hal_memc_init
	hal_intc_init
        hal_cache_init
        hal_timer_init
	
#if 0
## Spin to allow ejtag gdb to attach
1:
	li t0, 1
	bnez t0, 1b
	nop
#endif
#ifdef	CYG_HAL_STARTUP_RAM
	hal_serial_init
#endif
			
#ifdef CYGARC_START_FUNC_UNCACHED
	# switch to cached execution address if necessary
	# assumption is that hal_cache_init makes this safe
	lar	v0,1f
	jr	v0
	nop
   1:
#endif
	
	# Load Global Pointer register.
	la	gp,_gp

	# load initial stack pointer
	la	a0,__interrupt_stack
	move	sp,a0

	hal_mon_init
		
#ifdef CYG_HAL_STARTUP_ROM
	# Copy data from ROM to RAM

	.extern	hal_copy_data
	jal	hal_copy_data
	nop

#endif

	# Zero BSS

	.extern hal_zero_bss
	jal	hal_zero_bss
	nop

	# Call variant and platform HAL
	# initialization routines.

	.extern	hal_variant_init
	jal	hal_variant_init
	nop

	.extern	hal_platform_init
	jal	hal_platform_init
	nop

	# Call constructors
	.extern cyg_hal_invoke_constructors
	jal     cyg_hal_invoke_constructors
	nop

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
        .extern initialize_stub
        jal     initialize_stub
        nop
#endif
#if defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT)
        .extern hal_ctrlc_isr_init
        jal     hal_ctrlc_isr_init
        nop
#endif

	# Call cyg_start	
	
	.extern	cyg_start
	j	cyg_start
        lui     ra,0
	
FUNC_END(_start	)

##-----------------------------------------------------------------------------
		
FUNC_START(__warm_start)
	## The following is debug code left in here for now in case it
	## proves useful in the near future.	
#if 0
	move	s0,t0
	move	s1,a1
	
#	hal_diag_init

	hal_diag_writec '$'
	mvafc0	a0,$30			# get ErrorEPC
	lar	k0,hal_diag_ai_write_hex8
	jalr	k0
	nop
	hal_diag_writec '-'	
	move	a0,s0
	jalr	k0
	nop
	hal_diag_writec '-'	
	move	a0,s1
	jalr	k0
	nop
1:	
	b	1b
	nop
#endif
	
#if 0
	# Treat a warm-start either as a cold-start or an NMI
#if defined(CYGHWR_HAL_MIPS_WARMSTART_COLDSTART)
	lar	v0,_start		# jump to start
	jr	v0
	nop				# (delay slot)
#else
        # Defaults to NMI
	b	__nmi_entry
	nop
#endif
	
	lar	v0,_start		# jump to start
	jr	v0
	nop				
#endif	

FUNC_END(__warm_start)		

##-----------------------------------------------------------------------------
	
FUNC_START(__nmi_entry)		

	# Clear exception state
	hal_cpu_except_enable
	
	# Move the ErrorEPC register to the EPC register so that the
	# default exception handler saves the right PC value.
	mvafc0	k0,$30
	nop; nop; nop;
	mvatc0	k0,epc
	nop; nop; nop;	

#if (INITIAL_SR & 0x00400000) == 0
	# Taking this exception will have set the BEV bit to 1.
	# If we normally run with it zero, we must clear it here.
	mfc0	k0,status	
	la	k1,0xFFBFFFFF
	and	k0,k0,k1
	mtc0	k0,status
#endif	

	la	k0,34*4
	la	k1,hal_vsr_table	# Get VSR table
	lw	k1,34*4(k1)		# load NMI vector
	jr	k1			# go there
	nop				# (delay slot)
	
FUNC_END(__nmi_entry)

##-----------------------------------------------------------------------------
## Default exception VSR.
## Saves machine state and calls external handling code.
	
FUNC_START(__default_exception_vsr)

	# We enter here with all of the CPU state still
	# in its registers except:
	# K0 = vector index
	# K1 = address of this function

	move	k1,sp			# K1 = original SP
	
	addi	sp,sp,-mips_exception_decrement
				# space for registers + safety margin

	sw	k0,mipsreg_vector(sp)	# store vector

	# store GPRs
	.set	noat
	sgpr	0,sp
	sgpr	1,sp
	sgpr	2,sp
	sgpr	3,sp
	sgpr	4,sp
	sgpr	5,sp
	sgpr	6,sp
	sgpr	7,sp
	sgpr	8,sp
	sgpr	9,sp
	sgpr	10,sp
	sgpr	11,sp
	sgpr	12,sp
	sgpr	13,sp
	sgpr	14,sp
	sgpr	15,sp
	sgpr	16,sp
	sgpr	17,sp
	sgpr	18,sp
	sgpr	19,sp
	sgpr	20,sp
	sgpr	21,sp
	sgpr	22,sp
	sgpr	23,sp
	sgpr	24,sp
	sgpr	25,sp
#	sgpr	26,sp	# == K0
#	sgpr	27,sp	# == K1
	sgpr	28,sp	# == GP
#	sgpr	29,sp	# == SP
	sgpr	30,sp	# == FP
	sgpr	31,sp	# == RA
	.set	at
	
	mfhi	a0
	mflo	a1
	shi	a0,sp
	slo	a1,sp

	# K1 contains original SP
	ssp	k1,sp			# store in reg dump	
	
	# save remaining machine state registers	
	mfc0	t0,cause
	mfc0	t1,status
	mfc0	t2,cachectrl
	mvafc0	t3,badvr
	mfc0	t4,config
	mfc0	t5,prid
	mvafc0	t6,epc
	
	sw	t0,mipsreg_cause(sp)
	sw	t1,mipsreg_sr(sp)
	sw	t2,mipsreg_cachectrl(sp)
	sva	t3,mipsreg_badvr(sp)
	sw	t4,mipsreg_config(sp)
	sw	t5,mipsreg_prid(sp)
	sva	t6,mipsreg_pc(sp)

	hal_fpu_save sp
	
	# The machine state is now all saved on the stack.

	hal_diag_excpt_start
	
	# Load Global Pointer register.
	la	gp,_gp
	
	move	s0,sp				# save pointer to saved state

#if defined(CYGSEM_HAL_ROM_MONITOR) && \
    defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK)

	la	a0,__interrupt_stack		# a0 = stack top
	la	a1,__interrupt_stack_base	# a1 = stack base
	sub	a3,sp,a1			# a3 = sp - base
	bltz	a3,1f				# not on istack if < 0 
	nop					# delay slot
	sub	t0,a0,sp			# t0 = top - sp
	bgtz	t0,8f				# already on istack if > 0 
	nop					# delay slot
1:	
	move	sp,a0				# switch to istack
8:
	addi	sp,sp,-8			# space for old SP 
						# (8 to keep dword alignment!)
	sw	s0,0(sp)			# save old SP on stack
	
#endif		
	addi	sp,sp,-mips_stack_frame_size	# make a null frame	

	# Need to set up back pointers etc. ???

	hal_cpu_except_enable			# reenable exceptions

	.extern	cyg_hal_exception_handler
	jal	cyg_hal_exception_handler	# call C code
	move	a0,s0				# arg0 = register dump (delay slot)
		
#if defined(CYGSEM_HAL_ROM_MONITOR) && \
    defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK)

	# If we are returning from the last nested exception, move back
	# to the thread stack.
	# Since we have arranged for the top of stack location to
	# contain the sp we need to go back to here, just pop it off
	# and put it in SP.

	lw	sp,mips_stack_frame_size(sp)	# sp = *sp
	subu	sp,sp,mips_stack_frame_size	# make a null frame		
#endif
	
	j	restore_state
	nop
				
FUNC_END(__default_exception_vsr)

##------------------------------------------------------------------------------
## Default interrupt VSR.
## Saves machine state and calls appropriate ISR. When done, calls
## interrupt_end() to finish up and possibly reschedule.	

FUNC_START(__default_interrupt_vsr)

	
	# We enter here with all of the CPU state still
	# in its registers except:
	# K0 = vector index
	# K1 = address of this function

	move	k1,sp			# K1 = original SP
		
	addi	sp,sp,-mips_exception_decrement
				# space for registers + safety margin

	sw	k0,mipsreg_vector(sp)	# store vector

	# store GPRs
	.set	noat
	sgpr	0,sp
	sgpr	1,sp
	sgpr	2,sp
	sgpr	3,sp
	sgpr	4,sp
	sgpr	5,sp
	sgpr	6,sp
	sgpr	7,sp
	sgpr	8,sp
	sgpr	9,sp
	sgpr	10,sp
	sgpr	11,sp
	sgpr	12,sp
	sgpr	13,sp
	sgpr	14,sp
	sgpr	15,sp
	sgpr	16,sp
	sgpr	17,sp
	sgpr	18,sp
	sgpr	19,sp
	sgpr	20,sp
	sgpr	21,sp
	sgpr	22,sp
	sgpr	23,sp
	sgpr	24,sp
	sgpr	25,sp
#	sgpr	26,sp	# == K0
#	sgpr	27,sp	# == K1
	sgpr	28,sp	# == GP
#	sgpr	29,sp	# == SP
	sgpr	30,sp	# == FP
	sgpr	31,sp	# == RA
	.set	at
	
	mfhi	a0
	mflo	a1
	shi	a0,sp
	slo	a1,sp

	# K1 contains original SP
	ssp	k1,sp			# store in reg dump	
		
	mfc0	t1,status
	mfc0	t2,cachectrl
	mvafc0	t3,epc
	
	sw	t1,mipsreg_sr(sp)
	sw	t2,mipsreg_cachectrl(sp)
	sva	t3,mipsreg_pc(sp)

	hal_fpu_save sp
		
	# The machine state is now all saved on the stack.

	# Load Global Pointer register.
	la	gp,_gp
	
#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT			
	.extern	cyg_scheduler_sched_lock
	la	v0,cyg_scheduler_sched_lock
	lw	a0,0(v0)
	addi	a0,a0,1
	sw	a0,0(v0)
#endif

	move	s0,sp				# save pointer to saved state

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK	

	la	a0,__interrupt_stack		# a0 = stack top
	la	a1,__interrupt_stack_base	# a1 = stack base
	sub	a3,sp,a1			# a3 = sp - base
	bltz	a3,1f				# not on istack if < 0 
	nop					# delay slot
	sub	t0,a0,sp			# t0 = top - sp
	bgtz	t0,8f				# already on istack if > 0 
	nop					# delay slot
1:	
	move	sp,a0				# switch to istack
8:
	addi	sp,sp,-8			# space for old SP 
						# (8 to keep dword alignment!)
	sw	s0,0(sp)			# save old SP on stack
	
#endif
					
	subu	sp,sp,mips_stack_frame_size	# make a null frame	

	# Need to set up back pointers etc. ???

	# Decode external interrupt via interrupt controller

	hal_intc_decode	s2

	# Here, s2 contains the number of the interrupt being serviced,
	# we need to derive from that the vector number to call in the ISR
	# table.
	
	hal_intc_translate s2,s1
	
	# Here s1 is the number of the vector to be called and s2 is
	# the number of the interrupt being serviced. 

	hal_diag_intr_start
		
#if defined(CYGPKG_KERNEL_INSTRUMENT) && defined(CYGDBG_KERNEL_INSTRUMENT_INTR)

	# Call cyg_instrument to record that this interrupt is being raised.
		
	li	a0,0x0301			# a0 = type = INTR,RAISE
	move	a1,s1				# a1 = vector number
	jal	cyg_instrument			# call instrument function
	 move	a2,s2				# a2 = interrupt number
#endif

#if defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT)
	# If we are supporting Ctrl-C interrupts from GDB, we must squirrel
	# away a pointer to the save interrupt state here so that we can
	# plant a breakpoint at some later time.
	
	.extern	hal_saved_interrupt_state
	la	v0,hal_saved_interrupt_state
	sw	s0,0(v0)
	
#endif
	
	sll	s1,s1,2				# s1 = byte offset of vector

	hal_cpu_except_enable			# reenable exceptions
			
	la	t2,hal_interrupt_handlers	# handler table
	add	t2,t2,s1			# address of ISR ptr
	lw	t2,0(t2)			# ISR pointer

	la	a1,hal_interrupt_data		# data table
	add	a1,a1,s1			# address of data ptr
	lw	a1,0(a1)			# Data pointer

	move	a0,s2				# pass interrupt number
	move	a2,s0				# pass saved interrupt state

	jalr	t2				# call ISR via t2
	nop					# (delay slot)

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

	# If we are returning from the last nested interrupt, move back
	# to the thread stack. interrupt_end() must be called on the
	# thread stack since it potentially causes a context switch.
	# Since we have arranged for the top of stack location to
	# contain the sp we need to go back to here, just pop it off
	# and put it in SP.

	
	lw	sp,mips_stack_frame_size(sp)	# sp = *sp
	subu	sp,sp,mips_stack_frame_size	# make a null frame		
#endif
	
#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT

	# We only need to call _interrupt_end() when there is a kernel
	# present to do any tidying up.
	
	# On return v0 bit 1 will indicate whether a DSR is
	# to be posted. Pass this together with a pointer to
	# the interrupt object we have just used to the
	# interrupt tidy up routine.

	# Note that s0, s1 and s2 are defined to be preserved across
	# calls by the calling convention, so they still contain
	# the register dump, the vector offset and the interrupt number
	# respectively.

	move	s2,v0
		
	la	a1,hal_interrupt_objects	# interrupt object table
	add	a1,a1,s1			# address of object ptr
	lw	a1,0(a1)			# a1 = object ptr

	move	a2,s0				# arg3 = saved register dump
	
	.extern	interrupt_end
	jal	interrupt_end			# call into C to finish off 
	 move	a0,v0				# put ISR result in arg0
	
	move	v0,s2				# return value from isr
#endif
				
restore_state:
#if defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon)
	move	k0,v0
#endif

	# All done, restore CPU state and continue

	addi	sp,sp,mips_stack_frame_size	# retrieve CPU state ptr

	# Disable interrupts again while we restore state. 
	hal_cpu_int_disable

	hal_diag_restore

	hal_fpu_load sp

	lw	t0,mipsreg_cachectrl(sp)
	lhi	t1,sp
	llo	t2,sp

	mtc0	t0,cachectrl
	mthi	t1
	mtlo	t2

	# load GPRs
	.set	noat
#	lgpr	0,sp
	lgpr	1,sp
	lgpr	2,sp
	lgpr	3,sp
	lgpr	4,sp
	lgpr	5,sp
	lgpr	6,sp
	lgpr	7,sp
	lgpr	8,sp
	lgpr	9,sp
	lgpr	10,sp
	lgpr	11,sp
	lgpr	12,sp
	lgpr	13,sp
	lgpr	14,sp
	lgpr	15,sp
	lgpr	16,sp
	lgpr	17,sp
	lgpr	18,sp
	lgpr	19,sp
	lgpr	20,sp
	lgpr	21,sp
	lgpr	22,sp
	lgpr	23,sp
	lgpr	24,sp
	lgpr	25,sp
#	lgpr	26,sp	# == K0
#	lgpr	27,sp	# == K1
	lgpr	28,sp	# == GP
#	lgpr	29,sp	# == SP
	lgpr	30,sp	# == FP
	lgpr	31,sp	# == RA
	.set	at

#if defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon)

        # If we have a Cygmon that wants to listen to network interrupts, then
        # the return code from the earlier call to hal_default_isr() will
        # have been negative to indicate this. So we jump into Cygmon here
        # because Cygmon requires the processor state to be the same as when
        # the interrupt was taken, but with k0 as the exception number.
        
	bgez	k0,1f
	nop
	# Check for new cygmon
	sw	k0,(mipsreg_regs+26*4)(sp)	# save k0
	la	k1,0x80000100 + 41*4		# New cygmon "magic" id
	lw	k1,0(k1)
	lui	k0,0x55aa
	ori	k0,0x4321
	bne	k0,k1,1f
			
	# Need to let cygmon handle this
	la	k1,0x80000100 + 39*4		# stub entry vector
	lw	k0,(mipsreg_regs+26*4)(sp)	# restore k0
	lw	k1,0(k1)
	lw	sp,(mipsreg_regs+29*4)(sp)	# restore SP
	sll	k0,1				# clear bit 31.
	jr	k1
        srl	k0,1
    1:
#endif

	lw	k1,mipsreg_sr(sp)		# K1 = saved SR

#if 0 < CYGINT_HAL_MIPS_INTERRUPT_RETURN_KEEP_SR_IM
	# Keep the current settings of the IM[7:0] bits within the status
	# register.  These may be used as interrupt masks, so if an ISR or
	# DSR masks interrupts they must be preserved.
	# If they are not used, then this does no harm.
	ori	k0,zero,0xff00
	nor	k0,k0,k0			# 0xffff00ff
	and	k1,k1,k0			# all interrupts disabled

	mfc0	k0,status			# V0 = current SR
	nop
	nop
	andi	k0,k0,0xff00			# preserve interrupt set
	or	k1,k1,k0			# insert into "saved SR"
#endif // 0 < CYGINT_HAL_MIPS_INTERRUPT_RETURN_KEEP_SR_IM
	lva	k0,mipsreg_pc(sp)		# K0 = return PC
	lsp	sp,sp				# load SP

	# Invoke CPU specific mechanism for returning from this
	# exception
	
	hal_cpu_eret k0,k1
		
FUNC_END(__default_interrupt_vsr)

	hal_intc_decode_data

##-----------------------------------------------------------------------------
## Execute pending DSRs on the interrupt stack with interrupts enabled.
## Note: this can only be called from code running on a thread stack
	
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
	.extern cyg_interrupt_call_pending_DSRs

FUNC_START(hal_interrupt_stack_call_pending_DSRs)
	mfc0	t0,status			# get status register value
	la	v0,__interrupt_stack		# v0 = interrupt stack
	move	v1,sp				# v1 = original stack ptr
	move	sp,v0				# sp = interrupt stack
	addi	sp,sp,-32			# make a null frame
	sw	v1,16(sp)			# save old sp
	sw	ra,20(sp)			# save old ra
	sw	t0,24(sp)			# save old sr

	hal_cpu_int_enable

	jal	cyg_interrupt_call_pending_DSRs	# call back to kernel
	nop

	lw	a0,24(sp)			# get status reg

	hal_cpu_int_merge a0			# merge with current SR
	
	lw	ra,20(sp)			# restore ra
	lw	sp,16(sp)			# restore sp

	jr	ra				# go back
	nop					# delay slot
	
FUNC_END(hal_interrupt_stack_call_pending_DSRs)	
#endif		

##-----------------------------------------------------------------------------
## Short circuit in case any code tries to use "__gccmain()"

FUNC_START(__gccmain)
	jr	ra
	nop
FUNC_END(__gccmain)

##-----------------------------------------------------------------------------
## Switch to a new stack.
## This is used in RedBoot to allow code to execute in a different
## stack context.

FUNC_START(hal_program_new_stack)
	# Arguments are:
	# a0 = function to call
	# a1 = stack pointer to use

	move	v1,sp				# v1 = original stack ptr
	move	sp,a1				# sp = new stack
	addi	sp,sp,-32			# make a null frame
	sva	v1,8(sp)			# save old sp
	sva	ra,16(sp)			# save old ra

	jalr	a0				# call function
	 nop

	lva	ra,16(sp)			# restore ra
	lva	sp,8(sp)			# restore sp

	jr	ra				# go back
	 nop					# delay slot
	
FUNC_END(hal_program_new_stack)
		
##-----------------------------------------------------------------------------
## hal_zero_bss
## Zero bss. Done in assembler to be optimal rather than using memset,
## which would risk zeroing bss while using it.
        
FUNC_START(hal_zero_bss)
#ifdef CYGHWR_HAL_MIPS_64BIT
#define STORE_OP        sd
#define BLOCK_SHIFT     6
#else
#define STORE_OP        sw
#define BLOCK_SHIFT     5
#endif
        la      a0,__bss_start          # start of bss
        la      a1,__bss_end            # end of bss
        andi    a2,a0,mips_regsize-1    # is bss aligned?
        bne     a2,zero,1f              # skip word copy
        nop
        
        # loop with 8 stores per loop
        subu            a3,a1,a0                # get length
        srl             a3,a3,BLOCK_SHIFT       # get number of blocks
        sll             a3,a3,BLOCK_SHIFT       # get length of blocks
        addu            a3,a0,a3                # get end addr of blocks
2:      STORE_OP        zero,(mips_regsize*0)(a0)
        STORE_OP        zero,(mips_regsize*1)(a0)
        STORE_OP        zero,(mips_regsize*2)(a0)
        STORE_OP        zero,(mips_regsize*3)(a0)
        STORE_OP        zero,(mips_regsize*4)(a0)
        STORE_OP        zero,(mips_regsize*5)(a0)
        STORE_OP        zero,(mips_regsize*6)(a0)
        STORE_OP        zero,(mips_regsize*7)(a0)
        addu            a0,a0,mips_regsize*8    # next addr
        bne             a3,a0,2b                # to next store
        nop

        # If length is a multiple of block size then we
        # are done and need to skip the byte loop
        beq		a1,a0,3f
        nop
	
        # finish 1 byte at a time
1:      sb      zero,0(a0)              # zero memory
        addiu   a0,a0,1                 # next addr
        bne     a0,a1,1b                # to next store
        nop
3:      jr      ra
        nop
FUNC_END(hal_zero_bss)

        
##-----------------------------------------------------------------------------
## VSR springboard for break instruction exceptions
## Both GCC and GDB use break instructions. GCC for division-by-zero
## notification and GDB for program-flow breakpoints. This springboard
## looks for the d-b-z kind and directs them to another vector so libc
## can handle these without affecting the debugger.
	
FUNC_START(__break_vsr_springboard)
	mvafc0	k0,epc
	mfc0	k1,cause
	bltzl   k1,1f
	addi    k0,k0,4                 # delay slot (only executed if BD set)
1:	lw      k1,0(k0)                # get break instruction
	la      k0,0x0007000d           # break 0x7 used by GCC for d-b-z
	bne     k0,k1,2f
	nop
	ori     k0,$0,14*4              # CYGNUM_HAL_VECTOR_DIV_BY_ZERO
	la	k1,hal_vsr_table	# address of VSR table
	add	k1,k1,k0		# offset of VSR entry
	lw	k1,0(k1)		# k1 = pointer to VSR
	jr	k1			# go there
	nop				# (delay slot)
2:	ori     k0,$0,9*4               # CYGNUM_HAL_VECTOR_BREAKPOINT
	j       __default_exception_vsr
	nop				# (delay slot)
FUNC_END(__break_vsr_springboard)


##-----------------------------------------------------------------------------
## Interrupt Stack.
## Used during intialization and for executing ISRs.
	
	.bss

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

	.long	0,0,0,0,0,0,0,0	

##-----------------------------------------------------------------------------
## VSR table.
## The main interrupt code indirects through here to find the VSR
## to execute for each architecture defined interrupt.
## This is only used for simulated targets, on real targets a fixed location VSR
## table is now allocated at 0x80000100.

#ifndef CYG_HAL_MIPS_VSR_TABLE_DEFINED
	
##	.section ".vsr_table","a"
	
	.data
			
	.globl	hal_vsr_table

hal_vsr_table:	
	.long	__default_interrupt_vsr	
	.rept	63
	.long	__default_exception_vsr	
	.endr

#endif	
	
#------------------------------------------------------------------------------
# Interrupt vector tables.
# These tables contain the isr, data and object pointers used to deliver
# interrupts to user code.
# hal_interrupt_level contains the interrupt level set by 
# HAL_INTERRUPT_CONFIGURE().
# This is a default set that provide support only for the 6 external
# interrupts in the status/cause registers. Platforms or boards are expected
# to define their own versions of these if they have their own interrupt mappings.

#ifndef CYG_HAL_MIPS_ISR_TABLES_DEFINED
	
	.extern hal_default_isr
	
	.data

	.globl	hal_interrupt_handlers
hal_interrupt_handlers:
	.long	hal_default_isr
	.long	hal_default_isr
	.long	hal_default_isr
	.long	hal_default_isr
	.long	hal_default_isr
	.long	hal_default_isr


	.globl	hal_interrupt_data
hal_interrupt_data:
	.rept	6
	.long	0
	.endr

	.globl	hal_interrupt_objects
hal_interrupt_objects:
	.rept	6
	.long	0
	.endr

#endif	

##-----------------------------------------------------------------------------
## end of vectors.S


