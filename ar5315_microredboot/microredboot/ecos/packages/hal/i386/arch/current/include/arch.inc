#ifndef CYGONCE_HAL_ARCH_INC
#define CYGONCE_HAL_ARCH_INC
##=============================================================================
##
##	arch.inc
##
##	i386 assembler header file
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
## Author(s): 	 nickg
## Contributors: nickg, pjo
## Date:	1999-10-15
## Purpose:	Architecture definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the i386 CPU family.
## Usage:
##		#include <cyg/hal/arch.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <cyg/hal/i386.inc>

#include <cyg/hal/variant.inc>

##-----------------------------------------------------------------------------
## CPU specific macros. These provide a common assembler interface to
## operations that may have CPU specific implementations on different
## variants of the architecture.		
	
#ifndef CYGPKG_HAL_I386_CPU_INIT_DEFINED
	# Initialize CPU
	.macro	hal_cpu_init
	.endm
#endif /* !CYGPKG_HAL_I386_CPU_INIT_DEFINED */

##-----------------------------------------------------------------------------

#ifndef CYGPKG_HAL_I386_INTC_DEFINED

#ifndef CYGPKG_HAL_I386_INTC_INIT_DEFINED
	# initialize all interrupts to disabled
	.macro	hal_intc_init
	.endm
#endif

	.macro	hal_intc_decode vnum
	.endm

#endif

#------------------------------------------------------------------------------
# SMP support
	
#ifdef CYGPKG_HAL_SMP_SUPPORT

	.macro	hal_smp_init
#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
	movl	$0,cyg_hal_smp_vsr_sync_flag
#endif
	.endm
	
	// Put CPU number in register
	.macro	hal_smp_cpu reg
	movl	cyg_hal_smp_local_apic,\reg
	movl	0x20(\reg),\reg
	shrl	$24,\reg	
	.endm

#else

	.macro	hal_smp_init
	.endm

	.macro hal_smp_cpu reg
	movl	$0,\reg
	.endm

#endif				

#------------------------------------------------------------------------------
# Stack switching macros

#ifndef CYG_HAL_I386_INTSTACK_MACROS_DEFINED

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

#ifdef CYGPKG_HAL_SMP_SUPPORT

	.macro	hal_init_istack reg
	hal_smp_cpu %ebx
	movl	$__interrupt_stack_vector,%ecx
	movl	$CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE,%eax
	imull	%ebx,%eax
	addl    $__interrupt_stack_first,%eax
	movl	%eax,0(%ecx,%ebx,4)
	movl	$CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE,\reg
	addl	%eax,\reg
	.endm

	.macro	hal_load_istack reg
	hal_load_istack_base \reg
	addl	$CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE,\reg
	.endm

	.macro	hal_load_istack_base reg
	hal_smp_cpu \reg
	movl	$__interrupt_stack_vector,%eax
	movl	0(%eax,\reg,4),\reg
	.endm

#else // CYGPKG_HAL_SMP_SUPPORT
	.macro	hal_init_istack reg,tr
	movl	$__interrupt_stack,\reg		// Load interrupt stack
	.endm

	.macro	hal_load_istack reg
	movl	$__interrupt_stack,\reg		// Load interrupt stack
	.endm

	.macro	hal_load_istack_base reg
	movl	$__interrupt_stack_base,\reg	// Load interrupt stack base
	.endm

#endif // CYGPKG_HAL_SMP_SUPPORT
		
	.macro	hal_to_intstack
	hal_load_istack_base %ebx		// EBX = stack base
	movl	%ebx,%eax
	addl	$CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE,%eax
	cmpl	%ebx,%ebp			# compare SP with istack base
	jb	1f				# if sp < istack base, switch
	cmpl	%eax,%ebp			# compare SP with istack top
	jbe	2f				# if sp < istack top, dont switch
1:	
	movl	%eax,%esp			# move on to new stack
2:		
	pushl	%ebp				# Save old SP on new stack
	.endm

	.macro	hal_from_intstack
	popl	%esp				# pop old SP from stack
	.endm

#define CYG_HAL_I386_INTSTACK_MACROS_DEFINED


#else // CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

	.macro	hal_init_istack reg
	.endm

	.macro	hal_load_istack_base reg
	.endm

	.macro	hal_load_istack reg
	.endm

	.macro	hal_to_intstack
	.endm

	.macro	hal_from_intstack
	.endm

#define CYG_HAL_I386_INTSTACK_MACROS_DEFINED

#endif // CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

#endif // CYG_HAL_I386_INTSTACK_MACROS_DEFINED
	
#------------------------------------------------------------------------------
# FPU macros.
	
#ifndef CYGPKG_HAL_I386_FPU_DEFINED

#ifdef CYGHWR_HAL_I386_FPU

#define CYGPKG_HAL_I386_FPU_DEFINED

	.macro	hal_fpu_init
	# Tell the CPU to use the math hardware.
	movl	%cr0, %eax
	orl	$0x32, %eax	# Set MP, ET, NE bits
	andl	$~0x8, %eax	# And clear TS bit
	movl	%eax, %cr0

	finit			# and initialize...

        ## Enable floating point exceptions. Bit mask:
        ##  1 - invalid operation
        ##  2 - denormalized operand
        ##  4 - zero divide
        ##  8 - overflow
        ## 16 - underflow
        ## 32 - precision
	pushl	$0		# space for CW
        fstcw   0(%esp)		# store FPCW to stack
        movl	0(%esp),%eax	# get into EAX
        andb	$(~0x04),%al	# allow only zero divide exceptions
        movl	%eax,0(%esp)	# put back into memory
        fldcw	0(%esp)		# reload
	addl	$4,%esp		# pop value

#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
        # set CR4.OSFXSR to safely use stmxcsr/ldmxcsr
        movl	%cr4, %eax
        orl	$0x200, %eax
        movl	%eax, %cr4

        ## Enable SIMD exceptions. Bit mask:
        ## 0x0080 - invalid operation
        ## 0x0100 - denormalized operand
        ## 0x0200 - zero divide
        ## 0x0400 - overflow
        ## 0x0800 - underflow
        ## 0x1000 - precision
        pushl   $0              # space for MXCSR
        stmxcsr 0(%esp)         # store MXCSR to stack	
        movl    0(%esp),%eax    # get into EAX
        andw    $(~0x0200),%ax  # allow only zero divide exceptions
        movl    %eax,0(%esp)    # put back into memory
        ldmxcsr 0(%esp)         # reload
        addl    $4,%esp         # pop value
#endif

#ifdef CYGHWR_HAL_I386_FPU_SWITCH_LAZY
	# Tell the CPU to generate an FPU unavailable exception
	# when the FPU is first used.
	movl	%cr0, %eax
	orl	$0x8, %eax
	movl	%eax, %cr0
	# Plant a pointer to the FPU switch VSR into slot 7
	# of the VSR table.
	movl	$__fpu_switch_vsr,%eax
	movl	%eax,(hal_vsr_table+7*4)
	# Now create an FPU context on the stack so that we can take
	# FPU-using interrupts and exceptions before the machine starts
	# up.
	subl	$i386reg_fpucontext_size,%esp
	movl	$0,i386reg_fpucontext_valid(%esp)
	hal_smp_cpu %ebx			# get CPU id
	movl	$cyg_hal_fpustate_current,%ecx	# current state table
	movl	%esp,0(%ecx,%ebx,4)		# save in table[cpu] entry
#endif
	.endm

	.macro	hal_fpu_cpu_init
	# Tell the CPU to use the math hardware.
	movl	%cr0, %eax
	orl	$0x32, %eax	# Set MP, ET, NE bits
	andl	$~0x8, %eax	# And clear TS bit
	movl	%eax, %cr0

	finit			# and initialize...

        ## Enable floating point exceptions. Bit mask:
        ##  1 - invalid operation
        ##  2 - denormalized operand
        ##  4 - zero divide
        ##  8 - overflow
        ## 16 - underflow
        ## 32 - precision
	pushl	$0		# space for CW
        fstcw   0(%esp)		# store FPCW to stack
        movl	0(%esp),%eax	# get into EAX
        andb	$(~0x04),%al	# allow only zero divide exceptions
        movl	%eax,0(%esp)	# put back into memory
        fldcw	0(%esp)		# reload
	addl	$4,%esp		# pop value

#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
        # set CR4.OSFXSR to safely use stmxcsr/ldmxcsr
        movl	%cr4, %eax
        orl	$0x200, %eax
        movl	%eax, %cr4

        ## Enable SIMD exceptions. Bit mask:
        ## 0x0080 - invalid operation
        ## 0x0100 - denormalized operand
        ## 0x0200 - zero divide
        ## 0x0400 - overflow
        ## 0x0800 - underflow
        ## 0x1000 - precision
        pushl   $0              # space for MXCSR
        stmxcsr 0(%esp)         # store MXCSR to stack	
        movl    0(%esp),%eax    # get into EAX
        andw    $(~0x0200),%ax  # allow only zero divide exceptions
        movl    %eax,0(%esp)    # put back into memory
        ldmxcsr 0(%esp)         # reload
        addl    $4,%esp         # pop value
#endif

#ifdef CYGHWR_HAL_I386_FPU_SWITCH_LAZY
	# Tell the CPU to generate an FPU unavailable exception
	# when the FPU is first used.
	movl	%cr0, %eax
	orl	$0x8, %eax
	movl	%eax, %cr0
	# Now create an FPU context on the stack so that we can take
	# FPU-using interrupts and exceptions before the kernel starts
	# up.
	subl	$i386reg_fpucontext_size,%esp
	movl	$0,i386reg_fpucontext_valid(%esp)
	hal_smp_cpu %ebx			# get CPU id
	movl	$cyg_hal_fpustate_current,%ecx	# current state table
	movl	%esp,0(%ecx,%ebx,4)		# save in table[cpu] entry
#endif

	.endm

#ifndef CYGHWR_HAL_I386_FPU_SWITCH_LAZY

	# Non-lazy CPU state switching. We simply switch the entire
	# FPU state on every context switch, interrupt or exception.

	# ------------------------------------------------------------
	# Context switch handling

	.macro	hal_fpu_push_ctx
	subl	$i386reg_fpstate_size,%esp	# make space
	fnsave	i386reg_fpstate(%esp)		# save FPU state
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
	# Save SIMD state.

	# FIXME. This is awfully inefficient. Need to use FXSAVE to
	# save FPU and SIMD at same time. FXSAVE requires a 16 byte
	# alignment and does not have an implicit finit as does FSAVE.

	stmxcsr	i386reg_simd_mxcsr(%esp)
	movups	%xmm0,i386reg_simd_xmm0(%esp)
	movups	%xmm1,i386reg_simd_xmm1(%esp)
	movups	%xmm2,i386reg_simd_xmm2(%esp)
	movups	%xmm3,i386reg_simd_xmm3(%esp)
	movups	%xmm4,i386reg_simd_xmm4(%esp)
	movups	%xmm5,i386reg_simd_xmm5(%esp)
	movups	%xmm6,i386reg_simd_xmm6(%esp)
	movups	%xmm7,i386reg_simd_xmm7(%esp)
#endif
	movl	$1,i386reg_fpstate_valid(%esp)	# indicate it is valid
	.endm

	.macro	hal_fpu_pop_ctx
	btl	$0,i386reg_fpstate_valid(%esp)	# check ls bit of valid flag
	jc	1f				# if set, restore state
	finit					# otherwise init FPU
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
	# FIXME. Anything needed here?
#endif
	jmp	2f				# and skip restore
1:
	frstor  i386reg_fpstate(%esp)		# restore FPU state
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
	# Restore SIMD state.

	# FIXME. This is awfully inefficient. Need to use FXRSTOR to
	# restore FPU and SIMD at same time. FXRSTOR requires a 16 byte
	# alignment.

	movups	i386reg_simd_xmm0(%esp),%xmm0
	movups	i386reg_simd_xmm1(%esp),%xmm1
	movups	i386reg_simd_xmm2(%esp),%xmm2
	movups	i386reg_simd_xmm3(%esp),%xmm3
	movups	i386reg_simd_xmm4(%esp),%xmm4
	movups	i386reg_simd_xmm5(%esp),%xmm5
	movups	i386reg_simd_xmm6(%esp),%xmm6
	movups	i386reg_simd_xmm7(%esp),%xmm7
	ldmxcsr	i386reg_simd_mxcsr(%esp)
#endif
2:
	addl	$i386reg_fpstate_size,%esp	# pop space used
	.endm

	# ------------------------------------------------------------
	# Interrupt and exception handling

	# In this configuration, the interrupt and exception code behaves in
	# exactly the same way as the context switch code.

	.macro	hal_fpu_push_int
	hal_fpu_push_ctx
	.endm

	.macro	hal_fpu_push_int_annex
	.endm

	.macro	hal_fpu_pop_int_annex
	.endm

	.macro	hal_fpu_pop_int
	hal_fpu_pop_ctx
	.endm

	.macro	hal_fpu_push_exc
	hal_fpu_push_ctx
	.endm

	.macro	hal_fpu_push_exc_annex
	.endm

	.macro	hal_fpu_pop_exc_annex
	.endm

	.macro	hal_fpu_pop_exc
	hal_fpu_pop_ctx
	.endm

#else // CYGHWR_HAL_I386_FPU_SWITCH_LAZY

	# Lazy CPU state switching. We defer CPU state switching until the new
	# thread actually uses the FPU. This state switch is handled by
	# __fpu_switch_vsr in vectors.S.

	.extern cyg_hal_fpustate_owner
	.extern cyg_hal_fpustate_current

	# ------------------------------------------------------------
	# Context switch handling

	# On context switch we simply stack a pointer to this
	# threads FPU context save area.

	.macro	hal_fpu_push_ctx
	hal_smp_cpu %ebx			# Get CPU id
	movl	$cyg_hal_fpustate_current,%ecx	# current state table
	pushl	0(%ecx,%ebx,4)			# push table[cpu] entry
	.endm

	# We do nothing here but set the CR0:TS bit to force
	# an exception when the FPU is next used and pop the
	# FPU save area pointer into the static variable.

	.macro	hal_fpu_pop_ctx
	movl	%cr0, %ecx			# get CR0
	orl	$0x8, %ecx			# set TS bit
	movl	%ecx, %cr0			# restore CR0
	hal_smp_cpu %ebx			# get CPU id
	movl	$cyg_hal_fpustate_current,%ecx	# current state table
	popl	0(%ecx,%ebx,4)			# pop table[cpu] entry
	.endm

	# ------------------------------------------------------------
	# Interrupt handling

	# On entry to an interrupt we save the current threads FPU context
	# pointer and set the CR0:TS bit to trap any FP operations in the
	# interrupt.

	.macro	hal_fpu_push_int
	hal_smp_cpu %ebx			# get CPU id
	movl	$cyg_hal_fpustate_current,%ecx	# current state table
	pushl	0(%ecx,%ebx,4)			# push table[cpu] entry
	# ensure that CR0:TS bit is set
	movl	%cr0, %ecx			# get CR0
	orl	$0x8, %ecx			# set TS bit
	movl	%ecx, %cr0			# restore CR0
	.endm

	# The following is called after we transfer to the interrupt
	# stack. We make space here for the FPU state of the interrupt
	# handler to be saved in case we get nested interrupts that use FP.

	.macro	hal_fpu_push_int_annex
	subl	$i386reg_fpucontext_size,%esp
	movl	$0,i386reg_fpucontext_valid(%esp)
	hal_smp_cpu %ebx			# get CPU id
	movl	$cyg_hal_fpustate_current,%ecx	# current state table
	movl	%esp,0(%ecx,%ebx,4)		# save in table[cpu] entry
	.endm

	# This is invoked just before any transfer back to the thread stack.
	# We check whether we are the FPU state owner, and if so, abdicate.
	# There is no need to save the state, the next thread will load its
	# own state over the top of it.

	.macro	hal_fpu_pop_int_annex
	hal_smp_cpu %ebx			# get CPU id
	movl	$cyg_hal_fpustate_owner,%ecx	# current state table
	cmpl	0(%ecx,%ebx,4),%esp		# are we FPU owner?
	jne	1f				# if not, then just continue
	movl	$0,0(%ecx,%ebx,4)		# no one owns FPU now
	# ensure that CR0:TS bit is set to force a reload of
	# the previous FPU state
	movl	%cr0, %ecx			# get CR0
	orl	$0x8, %ecx			# set TS bit
	movl	%ecx, %cr0			# restore CR0
1:
	addl	$i386reg_fpucontext_size,%esp	# pop FPU save area
	.endm

	# Final return from interrupt handling. Just pull the current
	# FPU context off the stack.
	.macro	hal_fpu_pop_int
	hal_smp_cpu %ebx			# get CPU id
	movl	$cyg_hal_fpustate_current,%ecx	# current state table
	popl	0(%ecx,%ebx,4)			# pop table[cpu] entry
	.endm

	# ------------------------------------------------------------
	# Exception handling

	# Whenever we take an exception, we save the current FPU state away
	# into its save area. This way, if we are going to end up in GDB, the
	# whole machine state is saved in memory.

	.macro	hal_fpu_push_exc
	hal_smp_cpu %ebx			# get CPU id
	movl	$cyg_hal_fpustate_current,%ecx	# current state table
	pushl	0(%ecx,%ebx,4)			# push table[cpu] entry
	movl	$cyg_hal_fpustate_owner,%ecx	# current owner table
	movl	0(%ecx,%ebx,4),%eax		# EAX = FPU state owner
	cmpl	$0,%eax				# test it
	je	1f				# skip if zero
	fnsave	i386reg_fpucontext_state(%eax)	# save state
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
	# Save SIMD state.

	# FIXME. This is awfully inefficient. Need to use FXSAVE to
	# save FPU and SIMD at same time. FXSAVE requires a 16 byte
	# alignment and does not have an implicit finit as does FSAVE.

	stmxcsr	i386reg_simd_mxcsr(%eax)
	movups	%xmm0,i386reg_simd_xmm0(%eax)
	movups	%xmm1,i386reg_simd_xmm1(%eax)
	movups	%xmm2,i386reg_simd_xmm2(%eax)
	movups	%xmm3,i386reg_simd_xmm3(%eax)
	movups	%xmm4,i386reg_simd_xmm4(%eax)
	movups	%xmm5,i386reg_simd_xmm5(%eax)
	movups	%xmm6,i386reg_simd_xmm6(%eax)
	movups	%xmm7,i386reg_simd_xmm7(%eax)
#endif
	movl	$1,i386reg_fpucontext_valid(%eax) # set valid
	movl	$0,0(%ecx,%ebx,4)		# zero owner pointer
1:
	.endm

	# The rest of the exception macros behave exactly like the
	# interrupt ones.

	.macro	hal_fpu_push_exc_annex
	hal_fpu_push_int_annex
	.endm

	.macro	hal_fpu_pop_exc_annex
	hal_fpu_pop_int_annex	
	.endm

	.macro	hal_fpu_pop_exc
	hal_fpu_pop_int
	.endm

#endif // CYGHWR_HAL_I386_FPU_SWITCH_LAZY

#else /* !CYGHWR_HAL_I386_FPU */

	# Non-FP macros.

	.macro	hal_fpu_init
	.endm

	.macro	hal_fpu_cpu_init
	.endm

	.macro	hal_fpu_push_ctx
	.endm

	.macro	hal_fpu_pop_ctx
	.endm

	.macro	hal_fpu_push_int
	.endm

	.macro	hal_fpu_push_int_annex
	.endm

	.macro	hal_fpu_pop_int_annex
	.endm

	.macro	hal_fpu_pop_int
	.endm

	.macro	hal_fpu_push_exc
	.endm

	.macro	hal_fpu_push_exc_annex
	.endm

	.macro	hal_fpu_pop_exc_annex
	.endm

	.macro	hal_fpu_pop_exc
	.endm

#endif

#endif	

#------------------------------------------------------------------------------
# MMU macros.

#ifndef CYGPKG_HAL_I386_MMU_DEFINED

#define CYGPKG_HAL_I386_MMU_DEFINED

	.macro 	hal_mmu_init
	.endm

#endif	

#------------------------------------------------------------------------------
# A20 gate enable

#define K_RDWR			0x60
#define	K_STATUS		0x64
#define	K_CMD			0x64
#define K_OBUF_FUL		0x01
#define K_IBUF_FUL		0x02
#define KC_CMD_WIN		0xD0
#define	KC_CMD_WOUT		0xD1
#define KB_A20			0xDF

	.macro	hal_a20_enable
	// Enable A20 so that addresses at 1MB don't wrap around back to 0.
1:	inb 	$K_STATUS, %al
	testb 	$K_IBUF_FUL, %al
	jnz 	1b

2:	inb 	$K_STATUS, %al
	testb 	$K_OBUF_FUL, %al
	jz 	3f
	inb 	$K_RDWR, %al
	jmp 	2b

3:	movb 	$KC_CMD_WOUT, %al
	outb 	%al, $K_CMD
1:	inb 	$K_STATUS, %al
	testb 	$K_IBUF_FUL, %al
	jnz 	1b

	movb 	$KB_A20, %al
	outb 	%al, $K_RDWR
1:	inb 	$K_STATUS, %al
	testb 	$K_IBUF_FUL, %al
	jnz 	1b
	.endm

#------------------------------------------------------------------------------
# MEMC macros.
# This version simply enables the A20 gate.
	
#ifndef CYGPKG_HAL_I386_MEMC_DEFINED

	.macro	hal_memc_init
	hal_a20_enable
	.endm

#endif	
	
#------------------------------------------------------------------------------
# Cache macros.
	
#ifndef CYGPKG_HAL_I386_CACHE_DEFINED

	.macro	hal_cache_init
	.endm

#endif	

#------------------------------------------------------------------------------
# Diagnostics macros.
	
#ifndef CYGPKG_HAL_I386_DIAG_DEFINED

	.macro	hal_diag_init
	.endm

	.macro	hal_diag_excpt_start
	.endm

	.macro	hal_diag_intr_start
	.endm

	.macro	hal_diag_restore
	.endm
#endif	

#------------------------------------------------------------------------------
# Timer initialization.
	
#ifndef CYGPKG_HAL_I386_TIMER_DEFINED

	.macro	hal_timer_init
	.endm

#endif	

#------------------------------------------------------------------------------
# Monitor initialization.
	
#ifndef CYGPKG_HAL_I386_MON_DEFINED

	.macro	hal_mon_init
	.endm

#endif	

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_ARCH_INC
# end of arch.inc
