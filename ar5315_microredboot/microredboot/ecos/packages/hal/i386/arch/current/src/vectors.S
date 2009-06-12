##=============================================================================
##
##      vectors.S
##
##      x86 exception vectors
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
## Author(s):   jskov
## Contributors:jskov
## Date:        1999-01-07
## Purpose:     x86 exception vectors
## Description: This file defines the code placed into the exception
##              vectors. It also contains the first level default VSRs
##              that save and restore state for both exceptions and
##              interrupts.
##
######DESCRIPTIONEND####
##
##=============================================================================


#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include CYGBLD_HAL_PLATFORM_H

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#endif /* CYGPKG_KERNEL */

#include <cyg/hal/arch.inc>

#==============================================================================

//    .file   "vectors.S"

#==============================================================================
# Real startup code. We jump here from the various reset vectors to set up the
# world.

	.text
	.globl	_start

_start: 
	hal_cpu_init
	hal_smp_init
	hal_diag_init
	hal_mmu_init
	hal_memc_init
	hal_intc_init
	hal_cache_init
	hal_timer_init

	# Loading the stack pointer seems appropriate now.
	# In SMP systems, this may not be the interrupt stack we 
	# actually need to use for this CPU. We fix that up later.
	
	movl	$__interrupt_stack, %esp

#if defined(CYG_HAL_STARTUP_FLOPPY) \
    || defined(CYG_HAL_STARTUP_ROM) \
    || defined(CYG_HAL_STARTUP_GRUB)
	# If we are here first, initialize the IDT. RAM startup
	# configurations can assume that Redboot has already set
	# the IDT up.
	hal_idt_init
#endif	
						
	hal_mon_init

	# Init FPU
	# Do this after the monitor init so that we can plant our
	# own FPU unavailable VSR.
	
	hal_fpu_init		# WARNING: may adjust stack pointer
	
	# Zero the BSS. If the BSS is not a whole number of words
	# long we will write up to 3 extra bytes at the end.
	# (This should not be a problem usually).
	movl	$__bss_end,%ecx		# ECX = end of BSS
	movl	$__bss_start,%edi	# EDI = base of BSS
	subl	%edi,%ecx		# ECX = size of BSS
	addl	$3,%ecx			# ECX += sizeof(long)-1
	shrl	$2,%ecx			# ECX >>= 2 = number of words to fill
	xorl	%eax,%eax		# EAX = 0 = fill value
	rep     stosl			# Fill it in

#ifdef CYG_HAL_STARTUP_ROM

	# In a ROM booted system, we also need to copy the data section
	# out to the RAM.
	movl	$__rom_data_start,%esi	# ESI = base of ROM data area
	movl	$__ram_data_start,%edi	# EDI = base of RAM data area
	movl	$__ram_data_end,%ecx	# ECX = end of data
	subl	%edi,%ecx		# ECX = size of data in bytes
	shrl	$2,%ecx			# ECX >>= 2 = number of words to copy
	rep     movsl			# Copy it over

#endif	
	
	.extern hal_variant_init
	call	hal_variant_init

	.extern hal_platform_init
	call	hal_platform_init

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
	// This is here so we can debug the constructors.
	.extern initialize_stub
	call	initialize_stub
#endif

	.extern cyg_hal_invoke_constructors
	call	cyg_hal_invoke_constructors

#ifdef CYGPKG_HAL_SMP_SUPPORT 
	
	# Now move SP to actual interrupt stack we will use for this
	# processor.
	hal_init_istack	%esp

#ifndef CYG_HAL_STARTUP_RAM
	# Only start other CPUs when we are the original boot executable.
	# RAM executables are loaded via RedBoot, so only FLOPPY, GRUB 
        # and ROM startups count here.
	
	.extern cyg_hal_smp_cpu_start_all
	call cyg_hal_smp_cpu_start_all
#endif
		
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_INITIAL_BREAK
        .extern breakpoint
        call breakpoint
#endif        
	.extern cyg_start
	call	cyg_start

	# Hmm.  Not expecting to return from cyg_start.
1:	hlt
	jmp	1b

##-----------------------------------------------------------------------------
## SMP entry point

#if defined(CYGPKG_HAL_SMP_SUPPORT)
	.extern cyg_hal_smp_startup
	.global cyg_hal_smp_start
cyg_hal_smp_start:

	# Finalize CPU init?
	# Interrupts?
	
#	hal_cpu_init
#	hal_intc_init
	
	hal_init_istack %esp

	# Init flags register
	pushl	$0
	popfl
	
	hal_fpu_cpu_init

	call	cyg_hal_smp_startup
1:	
	jmp	1b	

#ifdef CYG_HAL_STARTUP_RAM	
	.align	4, 0xFF	
gdtStart:
	/* Selector 0x00 == invalid. */
	.word	0x0000
	.word	0x0000
	.byte	0x00
	.byte	0x00
	.byte	0x00
	.byte	0x00

	/* Selector 0x08 == code. */
	.word	0xFFFF
	.word	0x0000
	.byte	0x00
	.byte	0x9B
	.byte	0xCF
	.byte	0x00

	/* Selector 0x10 == data. */
	.word	0xFFFF
	.word	0x0000
	.byte	0x00
	.byte	0x93
	.byte	0xCF
	.byte	0x00

	/* Selector 0x18 == shorter code: faults any code 
         * access 0xF0000000-0xFFFFFFFF.
         */
	.word	0xFFFF
	.word	0x0000
	.byte	0x00
	.byte	0x9B
	.byte	0xC7
	.byte	0x00

	/* Selector 0x20 == data; faults any access 0xF0000000-0xFFFFFFFF. */
	.word	0xFFFF
	.word	0x0000
	.byte	0x00
	.byte	0x93
	.byte	0xC7
	.byte	0x00

	.align	4, 0xFF
gdtEnd:
#endif	// CYG_HAL_STARTUP_RAM	
	
##-----------------------------------------------------------------------------
## Slave processor startup code
## This code is copied into low RAM, at 0x2000 and is the destination of the
## startup interrupt that is sent to get the slaves running.

	.data
	.code16
	
	.global cyg_hal_slave_trampoline
	.global cyg_hal_slave_trampoline_end
cyg_hal_slave_trampoline:
slave_base = .
	cld				/* always count up. */	
	cli				/* disable interrupts */

	# Load up selector registers
	# Set DS == CS
	movw	%cs,%ax
	movw	%ax,%ds

	# load GDTR
	lgdt	slave_gdt - slave_base
	lidt	slave_idt - slave_base

	/* Switch to protected mode. */
	movl	%cr0,%eax
	orb	$1, %al
	movl	%eax,%cr0
	ljmp	$8, $3f-slave_base+0x2000

	hlt

	.align	4, 0xFF
slave_gdt:
	.word	gdtEnd - gdtStart
#	.word	39
	.long	gdtStart

	.align	4, 0xFF
slave_idt:
	.extern idtStart
	.word	0x07FF		# space for 256 entries
	.long	idtStart
	
	.code32

3:

	# Load up selector registers
	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %ss
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	# Go to real HAL entry point
	movl	$cyg_hal_smp_start,%eax
	jmp	*%eax
	
cyg_hal_slave_trampoline_end:	

	.text

#endif // defined(CYGPKG_HAL_SMP_SUPPORT)
		
#==============================================================================
# Default exception VSR

	.align	4, 0xCC
	.globl	__default_exception_vsr
__default_exception_vsr:

	## We enter here with the CPU state still in the registers and:
	## 12(%esp)	EFLAGS pushed by hardware
	##  8(%esp)	CS pushed by hardware
	##  4(%esp)	PC pushed by hardware
	##  0(%esp)	vector number pushed by trampoline

	pusha			# save all registers

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
	mov	%esp,%ebp                 # save SP
	cmpl	$__stub_stack_base,%esp   # compare SP with stub stack base
	jb	1f                        # if sp < istack base, switch
	cmpl	$__stub_stack,%esp        # compare SP with stub stack top
	jbe	2f                        # if sp < stack top, dont switch
1:
	movl	$__stub_stack,%esp        # move to stub stack
	
	## We switched stacks with previous ESP in EBP
	## 44(%ebp)	EFLAGS pushed by hardware
	## 40(%ebp)	CS pushed by hardware
	## 36(%ebp)	PC pushed by hardware
	## 32(%ebp)	vector number pushed by trampoline
	## 28(%ebp)	EAX
	## 24(%ebp)	ECX
	## 20(%ebp)	EDX
	## 16(%ebp)	EBX
	## 12(%ebp)	(ESP - 16)
	##  8(%ebp)	EBP
	##  4(%ebp)	ESI
	##  0(%ebp)	EDI
	
	pushl	44(%ebp)		# copy EFLAGS from original stack
	pushl	40(%ebp)		# copy CS
	pushl	36(%ebp)		# copy PC
	pushl	32(%ebp)		# copy vector number
	pusha
	movl	8(%ebp),%eax		# copy EBP
	movl	%eax,8(%esp)
	movl	12(%ebp),%eax		# copy ESP
	movl	%eax,12(%esp)
2:
#endif
	hal_fpu_push_exc	# save FPU state

	mov	%esp,%edi	# save state pointer in EDI

        # adjust ESP by 16 for the state stored before the pusha
        add     $16,i386reg_esp(%edi)
		
#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS) && defined(CYGPKG_HAL_SMP_SUPPORT)

	.extern cyg_hal_smp_cpu_sync
	.extern cyg_hal_smp_cpu_sync_flag
	.extern cyg_hal_smp_vsr_sync_flag
			
	# An SMP ROM monitor needs to suspend all other CPUs when
	# taking an exception.

1:		
	lock btsl $0,cyg_hal_smp_vsr_sync_flag	# test serialization bit
	jnc	9f				# if it was zero we are first here

	# Some other CPU is already handling an exception. We need to spin until
	# released.
	
	hal_smp_cpu %eax		# get CPU index
	movl	$cyg_hal_smp_cpu_sync,%ebx
	movl	$cyg_hal_smp_cpu_sync_flag,%ecx
	lock incl 0(%ecx,%eax,4)	# inc cpu sync flag
2:
	cmpl    $0,0(%ebx,%eax,4)	# test sync location
	je	2b			# loop while value is zero

	lock decl 0(%ecx,%eax,4)	# dec cpu sync flag

	# Jump to return from this VSR. If the exception was genuine,
	# we will re-execute the cause and come back here. If it was
	# just a duplicate, or a halt NMI, we will continue as if
	# nothing had happened.
	
	 jmp	__default_exception_vsr_return

9:	
	# Stop all other CPUs	
	.extern cyg_hal_smp_halt_other_cpus
	call	cyg_hal_smp_halt_other_cpus

#endif			

	hal_fpu_push_exc_annex
	
	# Call exception handler
	.extern	cyg_hal_exception_handler
	pushl	%edi			# arg1 = saved state
	call	cyg_hal_exception_handler
	addl	$4,%esp			# pop arg
	
	hal_fpu_pop_exc_annex
		
#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS) && defined(CYGPKG_HAL_SMP_SUPPORT)

	.extern cyg_hal_smp_release_other_cpus
	call	cyg_hal_smp_release_other_cpus

	lock btrl $0,cyg_hal_smp_vsr_sync_flag	# clear serialization bit
		
__default_exception_vsr_return:

#endif		

	hal_fpu_pop_exc		# restore FPU state

	## At this point, the stack contains:
	## 44(%esp)	EFLAGS pushed by hardware
	## 40(%esp)	CS pushed by hardware
	## 36(%esp)	PC pushed by hardware
	## 32(%esp)	vector number pushed by trampoline
	## 28(%esp)	EAX
	## 24(%esp)	ECX
	## 20(%esp)	EDX
	## 16(%esp)	EBX
	## 12(%esp)	ESP
	##  8(%esp)	EBP
	##  4(%esp)	ESI
	##  0(%esp)	EDI
	
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
	movl	12(%esp),%ebp	# pre-exception ESP
	sub	$48,%ebp	# adjust for current stack frame
	cmpl	%esp,%ebp
	je	1f

	## need to switch stacks
	xchg	%esp,%ebp
	add	$48,%esp
	pushl	44(%ebp)	# EFLAGS
	pushl	40(%ebp)	# CS
	pushl	36(%ebp)	# PC
	mov	%ebp,%esp
	popa
	movl	-20(%esp),%esp	# popa does not restore %esp
	sub	$12,%esp	# adjust for EFLAGS, CS, and PC
	iret	
1:
#endif
	popa			# restore all our registers.
	addl	$4, %esp	# skip the vector number
                                # Note: we do not need to re-adjust ESP
                                # back by 16 as popa does not pop ESP.
	iret			# and return to the program.
			
#==============================================================================
# Default interrupt VSR
#
#
	.extern	__interrupt_stack

	.align	4, 0xCC
	.globl	__default_interrupt_vsr
__default_interrupt_vsr:

	## We enter here with the CPU state still in the registers and:
	##  0(%esp)	vector number pushed by trampoline
	##  4(%esp)	PC pushed by hardware
	##  8(%esp)	CS pushed by hardware
	## 12(%esp)	EFLAGS pushed by hardware
	
	pusha			# save registers
	hal_fpu_push_int	# save FPU state

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) || \
    defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
	# Save the context just to be able to set a breakpoint
	# when we have a CTRL-C
	.extern hal_saved_interrupt_state
	movl %esp,hal_saved_interrupt_state
#endif

#if defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) && \
    !defined(CYGPKG_HAL_SMP_SUPPORT)
	# Increment scheduler lock
	.extern cyg_scheduler_sched_lock
	incl	cyg_scheduler_sched_lock
#endif

	movl	%esp,%ebp			# EBP = copy of ESP
	
        # adjust ESP by 16 for the state stored before the pusha
        add     $16,i386reg_esp(%esp)
		
	hal_to_intstack
		
	hal_fpu_push_int_annex			# save extra FPU state
	
#ifdef CYGSEM_HAL_COMMON_INTERRUPTS_ALLOW_NESTING

	# If we are allowing nested interrupts, restore the flags pushed
	# by the hardware when this interrupt was taken.
	
	movl	i386reg_eflags(%ebp), %eax
	btrl	$8,%eax			# Clear TF bit
	pushl	%eax
	popfl
#endif

#if defined(CYGPKG_KERNEL_INSTRUMENT) && defined(CYGDBG_KERNEL_INSTRUMENT_INTR)

	# Call cyg_instrument to record that this interrupt is being raised.

	movl	i386reg_vector(%ebp), %ecx # vector number from saved state
	movl	%ecx,%edi		# EDI = copy of vector
	subl	$0x20,%edi		# EDI = interrupt table offset
			
	pushl	%edi			# arg3 = interrupt number
	pushl	%ecx			# arg2 = vector number
	pushl	$0x0301			# arg1 = type = INTR.RAISE

	call	cyg_instrument		# call instrument function
	add	$12,%esp		# skip arguments
#endif
	
	# Call hal_interrupt_handlers[vector](vector, cyg_hal_interrupt_data[vector])
	
	movl	i386reg_vector(%ebp), %ecx # vector number from saved state
	movl	%ecx,%edi		# EDI = copy of vector
	subl	$0x20,%edi		# EDI = interrupt table offset
	movl	$hal_interrupt_handlers, %ebx
	movl	(%ebx, %edi, 4), %edx	# EDX = interrupt routine
	movl	$hal_interrupt_data, %ebx
	movl	(%ebx, %edi, 4), %eax	# EAX = interrupt data
	pushl	%eax			# arg2 = data
	pushl	%ecx			# arg1 = vector
	call	*%edx			# EAX = return value, needed for interrupt_end()
	addl	$8,%esp			# pop args

	# At this point:
	# EAX = ISR return code (returned by call)
	# EDI = ISR table offset (saved across call)
	# EBP = State pointer (saved across call)

	hal_fpu_pop_int_annex		# Pop any saved interrupt state
		
	# If we are returning from the last nested interrupt, move back
	# to the thread stack. interrupt_end() must be called on the
	# thread stack since it potentially causes a context switch.
	
	hal_from_intstack	
			
#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT

	# Call interrupt_end(r, cyg_hal_interrupt_objects[vector], regs)
	# - r is the return value from the ISR
	# - regs points to saved CPU context

	movl	$hal_interrupt_objects, %ebx
	movl	(%ebx, %edi, 4), %edx	# EDX = interrupt object ptr
	pushl	%ebp			# arg3 = ptr to saved registers
	pushl	%edx			# arg2 = object
	pushl	%eax			# arg1 = ISR return code
	call	interrupt_end		# Call it
	addl	$12, %esp		# pop args
	
#endif

	# Now pull saved state from stack and return to 
	# what thread was originally doing.
	
	hal_fpu_pop_int			# restore FPU state
	popa				# restore all our registers.
	addl	$4, %esp		# skip the vector number.
	iret				# and return to the thread.

#==============================================================================
## Execute pending DSRs on the interrupt stack with interrupts enabled.
## Note: this can only be called from code running on a thread stack so we 
## can always just jump to the interrupt stack without looking.

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

	.extern cyg_interrupt_call_pending_DSRs
	.global hal_interrupt_stack_call_pending_DSRs
	
hal_interrupt_stack_call_pending_DSRs:	

	pushl	%ebp			# save EBP
	pushfl				# save flags
	movl	%esp,%ebp		# EBP = saved SP

	hal_load_istack	%edx		# load new SP into EDX
	movl	%edx,%esp		# move it to ESP
	
	sti				# Enable interrupts

	# Call back to kernel to run DSRs
	call	cyg_interrupt_call_pending_DSRs

	# On return EBP will still contain the old ESP since
	# it is a callee saved register.
	
	movl	%ebp,%esp		# restore saved SP

	# Now merge the original IF bit into the current
	# EFLAGS. 
	
	popl	%eax			# EAX = saved flags
	pushfl				# 0(%esp) = current flags
	btrl	$9,0(%esp)		# clear IF bit in current flags
	andl	$0x00000200,%eax	# isolate saved IF bit
	orl	%eax,0(%esp)		# OR it in to the saved flags
	popfl				# restore flags
	
	popl	%ebp			# restore EBP
	ret				# and return
	
#endif				

#==============================================================================
# FPU lazy state switch VSR
# This is invoked via hardware exception 7 (FPU unavailable) as a result of
# setting the TS bit in CR0 whenever we context switch. If we discover here
# that a different context from the current owner of the FPU has attempted
# a floating point operation, we save the old context and load the new before 
# allowing it to proceed.
	
#ifdef CYGHWR_HAL_I386_FPU_SWITCH_LAZY		

#ifndef CYGPKG_HAL_SMP_CPU_MAX
#define CYGPKG_HAL_SMP_CPU_MAX 1
#endif
				
	.data
	.global cyg_hal_fpustate_owner
cyg_hal_fpustate_owner:
	.rept	CYGPKG_HAL_SMP_CPU_MAX
	.long	0				# pointer to FPU owning context
	.endr
	.global cyg_hal_fpustate_current	
cyg_hal_fpustate_current:	
	.rept	CYGPKG_HAL_SMP_CPU_MAX
	.long	0				# pointer to current threads FPU context
	.endr
			
	.text	
__fpu_switch_vsr:

	## We enter here with the CPU state still in the registers and:
	##  0(%esp)	vector number pushed by trampoline
	##  4(%esp)	PC pushed by hardware
	##  8(%esp)	CS pushed by hardware
	## 12(%esp)	EFLAGS pushed by hardware
	
	clts					# clear CR0:TS bit
	pusha					# save all regs

	hal_smp_cpu %ecx			# ECX = CPU id
	movl	$cyg_hal_fpustate_owner,%eax	# EAX = FPU context owner table
	leal	0(%eax,%ecx,4),%esi		# ESI = address of owner pointer
	movl	$cyg_hal_fpustate_current,%ebx	# EBX = current threads context table
	leal	0(%ebx,%ecx,4),%edi		# EDI = address of context pointer
	movl	0(%esi),%eax			# EAX = Current FPU context owner
	movl	0(%edi),%ebx			# EBX = Current threads FPU context
	cmpl	%ebx,%eax			# current == owner ?
	je	9f				# yes, nothing else to do
	cmpl	$0,%eax				# is FPU even in use?
	je	1f				# if not, skip save
	fnsave	i386reg_fpucontext_state(%eax)	# save FPU state
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
	movl	$1,i386reg_fpucontext_valid(%eax) # mark valid
1:
	movl	%ebx,0(%esi)            	# Set new owner
	btl	$0,i386reg_fpucontext_valid(%ebx) # Valid state?
	jc	2f				# If yes, go to restore it
	finit					# Otherwise init FPU
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
	# FIXME. Anything needed here?
#endif
	jmp	9f				# skip restore
2:
	frstor	i386reg_fpucontext_state(%ebx)	# restore FPU state
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
	# Restore SIMD state.

	# FIXME. This is awfully inefficient. Need to use FXRSTOR to
	# restore FPU and SIMD at same time. FXRSTOR requires a 16 byte
	# alignment.

	movups	i386reg_simd_xmm0(%ebx),%xmm0
	movups	i386reg_simd_xmm1(%ebx),%xmm1
	movups	i386reg_simd_xmm2(%ebx),%xmm2
	movups	i386reg_simd_xmm3(%ebx),%xmm3
	movups	i386reg_simd_xmm4(%ebx),%xmm4
	movups	i386reg_simd_xmm5(%ebx),%xmm5
	movups	i386reg_simd_xmm6(%ebx),%xmm6
	movups	i386reg_simd_xmm7(%ebx),%xmm7
	ldmxcsr	i386reg_simd_mxcsr(%ebx)
#endif
9:
	popa					# restore all our registers.
	addl	$4, %esp			# skip the vector number.
	iret					# and return to the thread.
		
#endif

#if 0			
#==============================================================================
# Assembler swap routines
	
		
# x = CPU_swap_u16(x)
	.align	4
	.global CPU_swap_u16
CPU_swap_u16:
	xorl	%eax, %eax
	movb	4(%esp), %ah
	movb	5(%esp), %al
	ret

# x = CPU_swap_u32(x)
	.align	4
	.global CPU_swap_u32
CPU_swap_u32:
	xorl	%eax, %eax
	movb	4(%esp), %ah
	movb	5(%esp), %al
	sall	$16, %eax
	movb	6(%esp), %ah
	movb	7(%esp), %al
	ret
#endif
		
#==============================================================================
# Exception trampolines
# IDT exception gates point to short code sequences that push the vector
# number on to the stack and then indirect via the VSR table to a handler.
# 

	# Just for yuks we keep a count of the number of times each
	# vector is called.
	.bss
	.globl hal_vsr_stats
hal_vsr_stats:	
	.rept 64	// Default VSR table is 64 entries long
	.long 0
	.endr

	# When an exception which supplies an error code is generated,
	# we move the code here. 
hal_trap_error_code:
	.long 0

	.text
	
	# macro to create exception handler (no error code)
	.macro  hal_pc_exception_noerr idx
hal_pc_exception_\idx:
	movl	$0,hal_trap_error_code
	pushl	$\idx
	incl	(hal_vsr_stats+\idx*4)
	jmp	*(hal_vsr_table+\idx*4)
	.endm

	# macro to create exception handler (with error code)
	.macro  hal_pc_exception_err idx
hal_pc_exception_\idx:
	popl	hal_trap_error_code
	pushl	$\idx
	incl	(hal_vsr_stats+\idx*4)
	jmp	*(hal_vsr_table+\idx*4)
	.endm

	# Now generate all the default exception VSR trampolines.
	
	hal_pc_exception_noerr  0
	hal_pc_exception_noerr  1
	hal_pc_exception_noerr  2
	hal_pc_exception_noerr  3	
	hal_pc_exception_noerr  4
	hal_pc_exception_noerr  5	
	hal_pc_exception_noerr  6
	hal_pc_exception_noerr  7	
	hal_pc_exception_err    8
	hal_pc_exception_noerr  9	
	hal_pc_exception_err   10
	hal_pc_exception_err   11	
	hal_pc_exception_err   12
	hal_pc_exception_err   13	
	hal_pc_exception_err   14
	hal_pc_exception_noerr 15	
	hal_pc_exception_noerr 16
	hal_pc_exception_err   17	
	hal_pc_exception_noerr 18
	hal_pc_exception_noerr 19	
	hal_pc_exception_noerr 20
	hal_pc_exception_noerr 21	
	hal_pc_exception_noerr 22
	hal_pc_exception_noerr 23	
	hal_pc_exception_noerr 24
	hal_pc_exception_noerr 25	
	hal_pc_exception_noerr 26
	hal_pc_exception_noerr 27	
	hal_pc_exception_noerr 28
	hal_pc_exception_noerr 29	
	hal_pc_exception_noerr 30
	hal_pc_exception_noerr 31	

#==============================================================================
# IRQ handler trampolines


	# macro to create exception handler (no error code)
	.macro  hal_pc_irq_handler idx
hal_pc_irq_\idx:
	pushl	$\idx
	incl	(hal_vsr_stats+\idx*4)
	jmp	*(hal_vsr_table+\idx*4)
	.endm

	hal_pc_irq_handler 32
	hal_pc_irq_handler 33
	hal_pc_irq_handler 34
	hal_pc_irq_handler 35
	hal_pc_irq_handler 36
	hal_pc_irq_handler 37
	hal_pc_irq_handler 38
	hal_pc_irq_handler 39
	hal_pc_irq_handler 40
	hal_pc_irq_handler 41
	hal_pc_irq_handler 42
	hal_pc_irq_handler 43
	hal_pc_irq_handler 44
	hal_pc_irq_handler 45
	hal_pc_irq_handler 46
	hal_pc_irq_handler 47
#ifdef CYGPKG_HAL_SMP_SUPPORT

	# Extra interrupt vectors for IOAPIc routed PCI and
	# other interrupt sources
	hal_pc_irq_handler 48
	hal_pc_irq_handler 49
	hal_pc_irq_handler 50
	hal_pc_irq_handler 51
	hal_pc_irq_handler 52
	hal_pc_irq_handler 53
	hal_pc_irq_handler 54
	hal_pc_irq_handler 55
	hal_pc_irq_handler 56
	hal_pc_irq_handler 57
	hal_pc_irq_handler 58
	hal_pc_irq_handler 59
	hal_pc_irq_handler 60
	hal_pc_irq_handler 61
	hal_pc_irq_handler 62
	hal_pc_irq_handler 63

	# Inter-CPU interrupts start at 64
	hal_pc_irq_handler 64
	hal_pc_irq_handler 65
	hal_pc_irq_handler 66
	hal_pc_irq_handler 67
#endif		


	# default vsr entries: pop the vector code from the stack and return.
default_vsr_iret:
	add $4,%esp
	iret

	# GNUPro apps use "int $0x80" for syscalls.
        # There is no vsr table entry for this.
	.global __syscall_tramp
__syscall_tramp:
	pushl $0x80
	jmp __default_exception_vsr

#==============================================================================
# Initial and interrupt stack

#ifndef CYG_HAL_I386_INTERRUPT_STACK_DEFINED

	.bss

#ifndef CYGPKG_HAL_SMP_SUPPORT	

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

#else // CYGPKG_HAL_SMP_SUPPORT		

__interrupt_stack_vector:
	.rept CYGPKG_HAL_SMP_CPU_MAX
	.long 0
	.endr
		
	.balign 16
	.global cyg_interrupt_stack_base
cyg_interrupt_stack_base:
__interrupt_stack_base:
__interrupt_stack_first:
	.rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
	.byte 0
	.endr
	.global cyg_interrupt_stack
cyg_interrupt_stack:
__interrupt_stack:
	.rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE*(CYGPKG_HAL_SMP_CPU_MAX-1)
	.byte 0
	.endr
	
#endif // CYGPKG_HAL_SMP_SUPPORT	

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
	.global __stub_stack_base
__stub_stack_base:
	.rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
	.byte 0
	.endr
	.balign 16
	.global __stub_stack
__stub_stack:
        
        .long   0,0,0,0,0,0,0,0
#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

	
#endif // CYG_HAL_I386_INTERRUPT_STACK_DEFINED
		
#------------------------------------------------------------------------------
# end of vectors.S
