/*
 *  PowerPC version 
 *    Copyright (C) 1995-1996 Gary Thomas (gdt@linuxppc.org)
 *  Rewritten by Cort Dougan (cort@cs.nmt.edu) for PReP
 *    Copyright (C) 1996 Cort Dougan <cort@cs.nmt.edu>
 *  Adapted for Power Macintosh by Paul Mackerras.
 *  Low-level exception handlers and MMU support
 *  rewritten by Paul Mackerras.
 *    Copyright (C) 1996 Paul Mackerras.
 *  MPC8xx modifications Copyright (C) 1997 Dan Malek (dmalek@jlc.net).
 *
 *  This file contains the system call entry code, context switch
 *  code, and exception/interrupt return code for PowerPC.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#include <linux/errno.h>
#include <linux/err.h>
#include <asm/unistd.h>
#include <asm/processor.h>
#include <asm/page.h>
#include <asm/mmu.h>
#include <asm/thread_info.h>
#include <asm/code-patching-asm.h>
#include <asm/ppc_asm.h>
#include <asm/asm-offsets.h>
#include <asm/cputable.h>
#include <asm/firmware.h>
#include <asm/bug.h>
#include <asm/ptrace.h>
#include <asm/irqflags.h>
#include <asm/hw_irq.h>
#include <asm/context_tracking.h>
#include <asm/tm.h>
#include <asm/ppc-opcode.h>
#include <asm/barrier.h>
#include <asm/export.h>
#ifdef CONFIG_PPC_BOOK3S
#include <asm/exception-64s.h>
#else
#include <asm/exception-64e.h>
#endif

/*
 * System calls.
 */
	.section	".toc","aw"
SYS_CALL_TABLE:
	.tc sys_call_table[TC],sys_call_table

/* This value is used to mark exception frames on the stack. */
exception_marker:
	.tc	ID_EXC_MARKER[TC],STACK_FRAME_REGS_MARKER

	.section	".text"
	.align 7

	.globl system_call_common
system_call_common:
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
BEGIN_FTR_SECTION
	extrdi.	r10, r12, 1, (63-MSR_TS_T_LG) /* transaction active? */
	bne	.Ltabort_syscall
END_FTR_SECTION_IFSET(CPU_FTR_TM)
#endif
	andi.	r10,r12,MSR_PR
	mr	r10,r1
	addi	r1,r1,-INT_FRAME_SIZE
	beq-	1f
	ld	r1,PACAKSAVE(r13)
1:	std	r10,0(r1)
	std	r11,_NIP(r1)
	std	r12,_MSR(r1)
	std	r0,GPR0(r1)
	std	r10,GPR1(r1)
	beq	2f			/* if from kernel mode */
#ifdef CONFIG_PPC_FSL_BOOK3E
START_BTB_FLUSH_SECTION
	BTB_FLUSH(r10)
END_BTB_FLUSH_SECTION
#endif
	ACCOUNT_CPU_USER_ENTRY(r13, r10, r11)
2:	std	r2,GPR2(r1)
	std	r3,GPR3(r1)
	mfcr	r2
	std	r4,GPR4(r1)
	std	r5,GPR5(r1)
	std	r6,GPR6(r1)
	std	r7,GPR7(r1)
	std	r8,GPR8(r1)
	li	r11,0
	std	r11,GPR9(r1)
	std	r11,GPR10(r1)
	std	r11,GPR11(r1)
	std	r11,GPR12(r1)
	std	r11,_XER(r1)
	std	r11,_CTR(r1)
	std	r9,GPR13(r1)
	mflr	r10
	/*
	 * This clears CR0.SO (bit 28), which is the error indication on
	 * return from this system call.
	 */
	rldimi	r2,r11,28,(63-28)
	li	r11,0xc01
	std	r10,_LINK(r1)
	std	r11,_TRAP(r1)
	std	r3,ORIG_GPR3(r1)
	std	r2,_CCR(r1)
	ld	r2,PACATOC(r13)
	addi	r9,r1,STACK_FRAME_OVERHEAD
	ld	r11,exception_marker@toc(r2)
	std	r11,-16(r9)		/* "regshere" marker */
#if defined(CONFIG_VIRT_CPU_ACCOUNTING_NATIVE) && defined(CONFIG_PPC_SPLPAR)
BEGIN_FW_FTR_SECTION
	beq	33f
	/* if from user, see if there are any DTL entries to process */
	ld	r10,PACALPPACAPTR(r13)	/* get ptr to VPA */
	ld	r11,PACA_DTL_RIDX(r13)	/* get log read index */
	addi	r10,r10,LPPACA_DTLIDX
	LDX_BE	r10,0,r10		/* get log write index */
	cmpd	cr1,r11,r10
	beq+	cr1,33f
	bl	accumulate_stolen_time
	REST_GPR(0,r1)
	REST_4GPRS(3,r1)
	REST_2GPRS(7,r1)
	addi	r9,r1,STACK_FRAME_OVERHEAD
33:
END_FW_FTR_SECTION_IFSET(FW_FEATURE_SPLPAR)
#endif /* CONFIG_VIRT_CPU_ACCOUNTING_NATIVE && CONFIG_PPC_SPLPAR */

	/*
	 * A syscall should always be called with interrupts enabled
	 * so we just unconditionally hard-enable here. When some kind
	 * of irq tracing is used, we additionally check that condition
	 * is correct
	 */
#if defined(CONFIG_TRACE_IRQFLAGS) && defined(CONFIG_BUG)
	lbz	r10,PACASOFTIRQEN(r13)
	xori	r10,r10,1
1:	tdnei	r10,0
	EMIT_BUG_ENTRY 1b,__FILE__,__LINE__,BUGFLAG_WARNING
#endif

#ifdef CONFIG_PPC_BOOK3E
	wrteei	1
#else
	li	r11,MSR_RI
	ori	r11,r11,MSR_EE
	mtmsrd	r11,1
#endif /* CONFIG_PPC_BOOK3E */

system_call:			/* label this so stack traces look sane */
	/* We do need to set SOFTE in the stack frame or the return
	 * from interrupt will be painful
	 */
	li	r10,1
	std	r10,SOFTE(r1)

	CURRENT_THREAD_INFO(r11, r1)
	ld	r10,TI_FLAGS(r11)
	andi.	r11,r10,_TIF_SYSCALL_DOTRACE
	bne	.Lsyscall_dotrace		/* does not return */
	cmpldi	0,r0,NR_syscalls
	bge-	.Lsyscall_enosys

.Lsyscall:
/*
 * Need to vector to 32 Bit or default sys_call_table here,
 * based on caller's run-mode / personality.
 */
	ld	r11,SYS_CALL_TABLE@toc(2)
	andi.	r10,r10,_TIF_32BIT
	beq	15f
	addi	r11,r11,8	/* use 32-bit syscall entries */
	clrldi	r3,r3,32
	clrldi	r4,r4,32
	clrldi	r5,r5,32
	clrldi	r6,r6,32
	clrldi	r7,r7,32
	clrldi	r8,r8,32
15:
	slwi	r0,r0,4

	barrier_nospec_asm
	/*
	 * Prevent the load of the handler below (based on the user-passed
	 * system call number) being speculatively executed until the test
	 * against NR_syscalls and branch to .Lsyscall_enosys above has
	 * committed.
	 */

	ldx	r12,r11,r0	/* Fetch system call handler [ptr] */
	mtctr   r12
	bctrl			/* Call handler */

.Lsyscall_exit:
	std	r3,RESULT(r1)
	CURRENT_THREAD_INFO(r12, r1)

	ld	r8,_MSR(r1)
#ifdef CONFIG_PPC_BOOK3S
	/* No MSR:RI on BookE */
	andi.	r10,r8,MSR_RI
	beq-	.Lunrecov_restore
#endif

/*
 * This is a few instructions into the actual syscall exit path (which actually
 * starts at .Lsyscall_exit) to cater to kprobe blacklisting and to reduce the
 * number of visible symbols for profiling purposes.
 *
 * We can probe from system_call until this point as MSR_RI is set. But once it
 * is cleared below, we won't be able to take a trap.
 *
 * This is blacklisted from kprobes further below with _ASM_NOKPROBE_SYMBOL().
 */
system_call_exit:
	/*
	 * Disable interrupts so current_thread_info()->flags can't change,
	 * and so that we don't get interrupted after loading SRR0/1.
	 */
#ifdef CONFIG_PPC_BOOK3E
	wrteei	0
#else
	/*
	 * For performance reasons we clear RI the same time that we
	 * clear EE. We only need to clear RI just before we restore r13
	 * below, but batching it with EE saves us one expensive mtmsrd call.
	 * We have to be careful to restore RI if we branch anywhere from
	 * here (eg syscall_exit_work).
	 */
	li	r11,0
	mtmsrd	r11,1
#endif /* CONFIG_PPC_BOOK3E */

	ld	r9,TI_FLAGS(r12)
	li	r11,-MAX_ERRNO
	andi.	r0,r9,(_TIF_SYSCALL_DOTRACE|_TIF_SINGLESTEP|_TIF_USER_WORK_MASK|_TIF_PERSYSCALL_MASK)
	bne-	.Lsyscall_exit_work

	andi.	r0,r8,MSR_FP
	beq 2f
#ifdef CONFIG_ALTIVEC
	andis.	r0,r8,MSR_VEC@h
	bne	3f
#endif
2:	addi    r3,r1,STACK_FRAME_OVERHEAD
#ifdef CONFIG_PPC_BOOK3S
	li	r10,MSR_RI
	mtmsrd	r10,1		/* Restore RI */
#endif
	bl	restore_math
#ifdef CONFIG_PPC_BOOK3S
	li	r11,0
	mtmsrd	r11,1
#endif
	ld	r8,_MSR(r1)
	ld	r3,RESULT(r1)
	li	r11,-MAX_ERRNO

3:	cmpld	r3,r11
	ld	r5,_CCR(r1)
	bge-	.Lsyscall_error
.Lsyscall_error_cont:
	ld	r7,_NIP(r1)
BEGIN_FTR_SECTION
	stdcx.	r0,0,r1			/* to clear the reservation */
END_FTR_SECTION_IFCLR(CPU_FTR_STCX_CHECKS_ADDRESS)
	andi.	r6,r8,MSR_PR
	ld	r4,_LINK(r1)

	beq-	1f
	ACCOUNT_CPU_USER_EXIT(r13, r11, r12)

BEGIN_FTR_SECTION
	HMT_MEDIUM_LOW
END_FTR_SECTION_IFSET(CPU_FTR_HAS_PPR)

	ld	r13,GPR13(r1)	/* only restore r13 if returning to usermode */
	ld	r2,GPR2(r1)
	ld	r1,GPR1(r1)
	mtlr	r4
	mtcr	r5
	mtspr	SPRN_SRR0,r7
	mtspr	SPRN_SRR1,r8
	RFI_TO_USER
	b	.	/* prevent speculative execution */

	/* exit to kernel */
1:	ld	r2,GPR2(r1)
	ld	r1,GPR1(r1)
	mtlr	r4
	mtcr	r5
	mtspr	SPRN_SRR0,r7
	mtspr	SPRN_SRR1,r8
	RFI_TO_KERNEL
	b	.	/* prevent speculative execution */

.Lsyscall_error:
	oris	r5,r5,0x1000	/* Set SO bit in CR */
	neg	r3,r3
	std	r5,_CCR(r1)
	b	.Lsyscall_error_cont

/* Traced system call support */
.Lsyscall_dotrace:
	bl	save_nvgprs
	addi	r3,r1,STACK_FRAME_OVERHEAD
	bl	do_syscall_trace_enter

	/*
	 * We use the return value of do_syscall_trace_enter() as the syscall
	 * number. If the syscall was rejected for any reason do_syscall_trace_enter()
	 * returns an invalid syscall number and the test below against
	 * NR_syscalls will fail.
	 */
	mr	r0,r3

	/* Restore argument registers just clobbered and/or possibly changed. */
	ld	r3,GPR3(r1)
	ld	r4,GPR4(r1)
	ld	r5,GPR5(r1)
	ld	r6,GPR6(r1)
	ld	r7,GPR7(r1)
	ld	r8,GPR8(r1)

	/* Repopulate r9 and r10 for the syscall path */
	addi	r9,r1,STACK_FRAME_OVERHEAD
	CURRENT_THREAD_INFO(r10, r1)
	ld	r10,TI_FLAGS(r10)

	cmpldi	r0,NR_syscalls
	blt+	.Lsyscall

	/* Return code is already in r3 thanks to do_syscall_trace_enter() */
	b	.Lsyscall_exit


.Lsyscall_enosys:
	li	r3,-ENOSYS
	b	.Lsyscall_exit
	
.Lsyscall_exit_work:
#ifdef CONFIG_PPC_BOOK3S
	li	r10,MSR_RI
	mtmsrd	r10,1		/* Restore RI */
#endif
	/* If TIF_RESTOREALL is set, don't scribble on either r3 or ccr.
	 If TIF_NOERROR is set, just save r3 as it is. */

	andi.	r0,r9,_TIF_RESTOREALL
	beq+	0f
	REST_NVGPRS(r1)
	b	2f
0:	cmpld	r3,r11		/* r11 is -MAX_ERRNO */
	blt+	1f
	andi.	r0,r9,_TIF_NOERROR
	bne-	1f
	ld	r5,_CCR(r1)
	neg	r3,r3
	oris	r5,r5,0x1000	/* Set SO bit in CR */
	std	r5,_CCR(r1)
1:	std	r3,GPR3(r1)
2:	andi.	r0,r9,(_TIF_PERSYSCALL_MASK)
	beq	4f

	/* Clear per-syscall TIF flags if any are set.  */

	li	r11,_TIF_PERSYSCALL_MASK
	addi	r12,r12,TI_FLAGS
3:	ldarx	r10,0,r12
	andc	r10,r10,r11
	stdcx.	r10,0,r12
	bne-	3b
	subi	r12,r12,TI_FLAGS

4:	/* Anything else left to do? */
BEGIN_FTR_SECTION
	lis	r3,INIT_PPR@highest	/* Set thread.ppr = 3 */
	ld	r10,PACACURRENT(r13)
	sldi	r3,r3,32	/* bits 11-13 are used for ppr */
	std	r3,TASKTHREADPPR(r10)
END_FTR_SECTION_IFSET(CPU_FTR_HAS_PPR)

	andi.	r0,r9,(_TIF_SYSCALL_DOTRACE|_TIF_SINGLESTEP)
	beq	ret_from_except_lite

	/* Re-enable interrupts */
#ifdef CONFIG_PPC_BOOK3E
	wrteei	1
#else
	li	r10,MSR_RI
	ori	r10,r10,MSR_EE
	mtmsrd	r10,1
#endif /* CONFIG_PPC_BOOK3E */

	bl	save_nvgprs
	addi	r3,r1,STACK_FRAME_OVERHEAD
	bl	do_syscall_trace_leave
	b	ret_from_except

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
.Ltabort_syscall:
	/* Firstly we need to enable TM in the kernel */
	mfmsr	r10
	li	r9, 1
	rldimi	r10, r9, MSR_TM_LG, 63-MSR_TM_LG
	mtmsrd	r10, 0

	/* tabort, this dooms the transaction, nothing else */
	li	r9, (TM_CAUSE_SYSCALL|TM_CAUSE_PERSISTENT)
	TABORT(R9)

	/*
	 * Return directly to userspace. We have corrupted user register state,
	 * but userspace will never see that register state. Execution will
	 * resume after the tbegin of the aborted transaction with the
	 * checkpointed register state.
	 */
	li	r9, MSR_RI
	andc	r10, r10, r9
	mtmsrd	r10, 1
	mtspr	SPRN_SRR0, r11
	mtspr	SPRN_SRR1, r12
	RFI_TO_USER
	b	.	/* prevent speculative execution */
#endif
_ASM_NOKPROBE_SYMBOL(system_call_common);
_ASM_NOKPROBE_SYMBOL(system_call_exit);

/* Save non-volatile GPRs, if not already saved. */
_GLOBAL(save_nvgprs)
	ld	r11,_TRAP(r1)
	andi.	r0,r11,1
	beqlr-
	SAVE_NVGPRS(r1)
	clrrdi	r0,r11,1
	std	r0,_TRAP(r1)
	blr
_ASM_NOKPROBE_SYMBOL(save_nvgprs);

	
/*
 * The sigsuspend and rt_sigsuspend system calls can call do_signal
 * and thus put the process into the stopped state where we might
 * want to examine its user state with ptrace.  Therefore we need
 * to save all the nonvolatile registers (r14 - r31) before calling
 * the C code.  Similarly, fork, vfork and clone need the full
 * register state on the stack so that it can be copied to the child.
 */

_GLOBAL(ppc_fork)
	bl	save_nvgprs
	bl	sys_fork
	b	.Lsyscall_exit

_GLOBAL(ppc_vfork)
	bl	save_nvgprs
	bl	sys_vfork
	b	.Lsyscall_exit

_GLOBAL(ppc_clone)
	bl	save_nvgprs
	bl	sys_clone
	b	.Lsyscall_exit

_GLOBAL(ppc32_swapcontext)
	bl	save_nvgprs
	bl	compat_sys_swapcontext
	b	.Lsyscall_exit

_GLOBAL(ppc64_swapcontext)
	bl	save_nvgprs
	bl	sys_swapcontext
	b	.Lsyscall_exit

_GLOBAL(ppc_switch_endian)
	bl	save_nvgprs
	bl	sys_switch_endian
	b	.Lsyscall_exit

_GLOBAL(ret_from_fork)
	bl	schedule_tail
	REST_NVGPRS(r1)
	li	r3,0
	b	.Lsyscall_exit

_GLOBAL(ret_from_kernel_thread)
	bl	schedule_tail
	REST_NVGPRS(r1)
	mtlr	r14
	mr	r3,r15
#ifdef PPC64_ELF_ABI_v2
	mr	r12,r14
#endif
	blrl
	li	r3,0
	b	.Lsyscall_exit

#ifdef CONFIG_PPC_BOOK3S_64

#define FLUSH_COUNT_CACHE	\
1:	nop;			\
	patch_site 1b, patch__call_flush_count_cache


#define BCCTR_FLUSH	.long 0x4c400420

.macro nops number
	.rept \number
	nop
	.endr
.endm

.balign 32
.global flush_count_cache
flush_count_cache:
	/* Save LR into r9 */
	mflr	r9

	// Flush the link stack
	.rept 64
	bl	.+4
	.endr
	b	1f
	nops	6

	.balign 32
	/* Restore LR */
1:	mtlr	r9

	// If we're just flushing the link stack, return here
3:	nop
	patch_site 3b patch__flush_link_stack_return

	li	r9,0x7fff
	mtctr	r9

	BCCTR_FLUSH

2:	nop
	patch_site 2b patch__flush_count_cache_return

	nops	3

	.rept 278
	.balign 32
	BCCTR_FLUSH
	nops	7
	.endr

	blr
#else
#define FLUSH_COUNT_CACHE
#endif /* CONFIG_PPC_BOOK3S_64 */

/*
 * This routine switches between two different tasks.  The process
 * state of one is saved on its kernel stack.  Then the state
 * of the other is restored from its kernel stack.  The memory
 * management hardware is updated to the second process's state.
 * Finally, we can return to the second process, via ret_from_except.
 * On entry, r3 points to the THREAD for the current task, r4
 * points to the THREAD for the new task.
 *
 * Note: there are two ways to get to the "going out" portion
 * of this code; either by coming in via the entry (_switch)
 * or via "fork" which must set up an environment equivalent
 * to the "_switch" path.  If you change this you'll have to change
 * the fork code also.
 *
 * The code which creates the new task context is in 'copy_thread'
 * in arch/powerpc/kernel/process.c 
 */
	.align	7
_GLOBAL(_switch)
	mflr	r0
	std	r0,16(r1)
	stdu	r1,-SWITCH_FRAME_SIZE(r1)
	/* r3-r13 are caller saved -- Cort */
	SAVE_8GPRS(14, r1)
	SAVE_10GPRS(22, r1)
	std	r0,_NIP(r1)	/* Return to switch caller */
	mfcr	r23
	std	r23,_CCR(r1)
	std	r1,KSP(r3)	/* Set old stack pointer */

	FLUSH_COUNT_CACHE

	/*
	 * On SMP kernels, care must be taken because a task may be
	 * scheduled off CPUx and on to CPUy. Memory ordering must be
	 * considered.
	 *
	 * Cacheable stores on CPUx will be visible when the task is
	 * scheduled on CPUy by virtue of the core scheduler barriers
	 * (see "Notes on Program-Order guarantees on SMP systems." in
	 * kernel/sched/core.c).
	 *
	 * Uncacheable stores in the case of involuntary preemption must
	 * be taken care of. The smp_mb__before_spin_lock() in __schedule()
	 * is implemented as hwsync on powerpc, which orders MMIO too. So
	 * long as there is an hwsync in the context switch path, it will
	 * be executed on the source CPU after the task has performed
	 * all MMIO ops on that CPU, and on the destination CPU before the
	 * task performs any MMIO ops there.
	 */

	/*
	 * The kernel context switch path must contain a spin_lock,
	 * which contains larx/stcx, which will clear any reservation
	 * of the task being switched.
	 */
#ifdef CONFIG_PPC_BOOK3S
/* Cancel all explict user streams as they will have no use after context
 * switch and will stop the HW from creating streams itself
 */
	DCBT_STOP_ALL_STREAM_IDS(r6)
#endif

	addi	r6,r4,-THREAD	/* Convert THREAD to 'current' */
	std	r6,PACACURRENT(r13)	/* Set new 'current' */

	ld	r8,KSP(r4)	/* new stack pointer */
#ifdef CONFIG_PPC_STD_MMU_64
BEGIN_MMU_FTR_SECTION
	b	2f
END_MMU_FTR_SECTION_IFSET(MMU_FTR_TYPE_RADIX)
BEGIN_FTR_SECTION
	clrrdi	r6,r8,28	/* get its ESID */
	clrrdi	r9,r1,28	/* get current sp ESID */
FTR_SECTION_ELSE
	clrrdi	r6,r8,40	/* get its 1T ESID */
	clrrdi	r9,r1,40	/* get current sp 1T ESID */
ALT_MMU_FTR_SECTION_END_IFCLR(MMU_FTR_1T_SEGMENT)
	clrldi.	r0,r6,2		/* is new ESID c00000000? */
	cmpd	cr1,r6,r9	/* or is new ESID the same as current ESID? */
	cror	eq,4*cr1+eq,eq
	beq	2f		/* if yes, don't slbie it */

	/* Bolt in the new stack SLB entry */
	ld	r7,KSP_VSID(r4)	/* Get new stack's VSID */
	oris	r0,r6,(SLB_ESID_V)@h
	ori	r0,r0,(SLB_NUM_BOLTED-1)@l
BEGIN_FTR_SECTION
	li	r9,MMU_SEGSIZE_1T	/* insert B field */
	oris	r6,r6,(MMU_SEGSIZE_1T << SLBIE_SSIZE_SHIFT)@h
	rldimi	r7,r9,SLB_VSID_SSIZE_SHIFT,0
END_MMU_FTR_SECTION_IFSET(MMU_FTR_1T_SEGMENT)

	/* Update the last bolted SLB.  No write barriers are needed
	 * here, provided we only update the current CPU's SLB shadow
	 * buffer.
	 */
	ld	r9,PACA_SLBSHADOWPTR(r13)
	li	r12,0
	std	r12,SLBSHADOW_STACKESID(r9)	/* Clear ESID */
	li	r12,SLBSHADOW_STACKVSID
	STDX_BE	r7,r12,r9			/* Save VSID */
	li	r12,SLBSHADOW_STACKESID
	STDX_BE	r0,r12,r9			/* Save ESID */

	/* No need to check for MMU_FTR_NO_SLBIE_B here, since when
	 * we have 1TB segments, the only CPUs known to have the errata
	 * only support less than 1TB of system memory and we'll never
	 * actually hit this code path.
	 */

	isync
	slbie	r6
	slbie	r6		/* Workaround POWER5 < DD2.1 issue */
	slbmte	r7,r0
	isync
2:
#endif /* CONFIG_PPC_STD_MMU_64 */

	CURRENT_THREAD_INFO(r7, r8)  /* base of new stack */
	/* Note: this uses SWITCH_FRAME_SIZE rather than INT_FRAME_SIZE
	   because we don't need to leave the 288-byte ABI gap at the
	   top of the kernel stack. */
	addi	r7,r7,THREAD_SIZE-SWITCH_FRAME_SIZE

	/*
	 * PMU interrupts in radix may come in here. They will use r1, not
	 * PACAKSAVE, so this stack switch will not cause a problem. They
	 * will store to the process stack, which may then be migrated to
	 * another CPU. However the rq lock release on this CPU paired with
	 * the rq lock acquire on the new CPU before the stack becomes
	 * active on the new CPU, will order those stores.
	 */
	mr	r1,r8		/* start using new stack pointer */
	std	r7,PACAKSAVE(r13)

	ld	r6,_CCR(r1)
	mtcrf	0xFF,r6

	/* r3-r13 are destroyed -- Cort */
	REST_8GPRS(14, r1)
	REST_10GPRS(22, r1)

	/* convert old thread to its task_struct for return value */
	addi	r3,r3,-THREAD
	ld	r7,_NIP(r1)	/* Return to _switch caller in new task */
	mtlr	r7
	addi	r1,r1,SWITCH_FRAME_SIZE
	blr

	.align	7
_GLOBAL(ret_from_except)
	ld	r11,_TRAP(r1)
	andi.	r0,r11,1
	bne	ret_from_except_lite
	REST_NVGPRS(r1)

_GLOBAL(ret_from_except_lite)
	/*
	 * Disable interrupts so that current_thread_info()->flags
	 * can't change between when we test it and when we return
	 * from the interrupt.
	 */
#ifdef CONFIG_PPC_BOOK3E
	wrteei	0
#else
	li	r10,MSR_RI
	mtmsrd	r10,1		  /* Update machine state */
#endif /* CONFIG_PPC_BOOK3E */

	CURRENT_THREAD_INFO(r9, r1)
	ld	r3,_MSR(r1)
#ifdef CONFIG_PPC_BOOK3E
	ld	r10,PACACURRENT(r13)
#endif /* CONFIG_PPC_BOOK3E */
	ld	r4,TI_FLAGS(r9)
	andi.	r3,r3,MSR_PR
	beq	resume_kernel
#ifdef CONFIG_PPC_BOOK3E
	lwz	r3,(THREAD+THREAD_DBCR0)(r10)
#endif /* CONFIG_PPC_BOOK3E */

	/* Check current_thread_info()->flags */
	andi.	r0,r4,_TIF_USER_WORK_MASK
	bne	1f
#ifdef CONFIG_PPC_BOOK3E
	/*
	 * Check to see if the dbcr0 register is set up to debug.
	 * Use the internal debug mode bit to do this.
	 */
	andis.	r0,r3,DBCR0_IDM@h
	beq	restore
	mfmsr	r0
	rlwinm	r0,r0,0,~MSR_DE	/* Clear MSR.DE */
	mtmsr	r0
	mtspr	SPRN_DBCR0,r3
	li	r10, -1
	mtspr	SPRN_DBSR,r10
	b	restore
#else
	addi	r3,r1,STACK_FRAME_OVERHEAD
	bl	restore_math
	b	restore
#endif
1:	andi.	r0,r4,_TIF_NEED_RESCHED_MASK
	beq	2f
	bl	restore_interrupts
	SCHEDULE_USER
	b	ret_from_except_lite
2:
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	andi.	r0,r4,_TIF_USER_WORK_MASK & ~_TIF_RESTORE_TM
	bne	3f		/* only restore TM if nothing else to do */
	addi	r3,r1,STACK_FRAME_OVERHEAD
	bl	restore_tm_state
	b	restore
3:
#endif
	bl	save_nvgprs
	/*
	 * Use a non volatile GPR to save and restore our thread_info flags
	 * across the call to restore_interrupts.
	 */
	mr	r30,r4
	bl	restore_interrupts
	mr	r4,r30
	addi	r3,r1,STACK_FRAME_OVERHEAD
	bl	do_notify_resume
	b	ret_from_except

resume_kernel:
	/* check current_thread_info, _TIF_EMULATE_STACK_STORE */
	andis.	r8,r4,_TIF_EMULATE_STACK_STORE@h
	beq+	1f

	addi	r8,r1,INT_FRAME_SIZE	/* Get the kprobed function entry */

	ld	r3,GPR1(r1)
	subi	r3,r3,INT_FRAME_SIZE	/* dst: Allocate a trampoline exception frame */
	mr	r4,r1			/* src:  current exception frame */
	mr	r1,r3			/* Reroute the trampoline frame to r1 */

	/* Copy from the original to the trampoline. */
	li	r5,INT_FRAME_SIZE/8	/* size: INT_FRAME_SIZE */
	li	r6,0			/* start offset: 0 */
	mtctr	r5
2:	ldx	r0,r6,r4
	stdx	r0,r6,r3
	addi	r6,r6,8
	bdnz	2b

	/* Do real store operation to complete stdu */
	ld	r5,GPR1(r1)
	std	r8,0(r5)

	/* Clear _TIF_EMULATE_STACK_STORE flag */
	lis	r11,_TIF_EMULATE_STACK_STORE@h
	addi	r5,r9,TI_FLAGS
0:	ldarx	r4,0,r5
	andc	r4,r4,r11
	stdcx.	r4,0,r5
	bne-	0b
1:

#ifdef CONFIG_PREEMPT
	/* Check if we need to preempt */
	lwz	r8,TI_PREEMPT(r9)
	cmpwi	0,r8,0		/* if non-zero, just restore regs and return */
	bne	restore
	andi.	r0,r4,_TIF_NEED_RESCHED
	bne+	check_count

	andi.	r0,r4,_TIF_NEED_RESCHED_LAZY
	beq+	restore
	lwz	r8,TI_PREEMPT_LAZY(r9)

	/* Check that preempt_count() == 0 and interrupts are enabled */
check_count:
	cmpwi	cr1,r8,0
	ld	r0,SOFTE(r1)
	cmpdi	r0,0
	crandc	eq,cr1*4+eq,eq
	bne	restore

	/*
	 * Here we are preempting the current task. We want to make
	 * sure we are soft-disabled first and reconcile irq state.
	 */
	RECONCILE_IRQ_STATE(r3,r4)
1:	bl	preempt_schedule_irq

	/* Re-test flags and eventually loop */
	CURRENT_THREAD_INFO(r9, r1)
	ld	r4,TI_FLAGS(r9)
	andi.	r0,r4,_TIF_NEED_RESCHED_MASK
	bne	1b

	/*
	 * arch_local_irq_restore() from preempt_schedule_irq above may
	 * enable hard interrupt but we really should disable interrupts
	 * when we return from the interrupt, and so that we don't get
	 * interrupted after loading SRR0/1.
	 */
#ifdef CONFIG_PPC_BOOK3E
	wrteei	0
#else
	li	r10,MSR_RI
	mtmsrd	r10,1		  /* Update machine state */
#endif /* CONFIG_PPC_BOOK3E */
#endif /* CONFIG_PREEMPT */

	.globl	fast_exc_return_irq
fast_exc_return_irq:
restore:
	/*
	 * This is the main kernel exit path. First we check if we
	 * are about to re-enable interrupts
	 */
	ld	r5,SOFTE(r1)
	lbz	r6,PACASOFTIRQEN(r13)
	cmpwi	cr0,r5,0
	beq	.Lrestore_irq_off

	/* We are enabling, were we already enabled ? Yes, just return */
	cmpwi	cr0,r6,1
	beq	cr0,.Ldo_restore

	/*
	 * We are about to soft-enable interrupts (we are hard disabled
	 * at this point). We check if there's anything that needs to
	 * be replayed first.
	 */
	lbz	r0,PACAIRQHAPPENED(r13)
	cmpwi	cr0,r0,0
	bne-	.Lrestore_check_irq_replay

	/*
	 * Get here when nothing happened while soft-disabled, just
	 * soft-enable and move-on. We will hard-enable as a side
	 * effect of rfi
	 */
.Lrestore_no_replay:
	TRACE_ENABLE_INTS
	li	r0,1
	stb	r0,PACASOFTIRQEN(r13);

	/*
	 * Final return path. BookE is handled in a different file
	 */
.Ldo_restore:
#ifdef CONFIG_PPC_BOOK3E
	b	exception_return_book3e
#else
	/*
	 * Clear the reservation. If we know the CPU tracks the address of
	 * the reservation then we can potentially save some cycles and use
	 * a larx. On POWER6 and POWER7 this is significantly faster.
	 */
BEGIN_FTR_SECTION
	stdcx.	r0,0,r1		/* to clear the reservation */
FTR_SECTION_ELSE
	ldarx	r4,0,r1
ALT_FTR_SECTION_END_IFCLR(CPU_FTR_STCX_CHECKS_ADDRESS)

	/*
	 * Some code path such as load_up_fpu or altivec return directly
	 * here. They run entirely hard disabled and do not alter the
	 * interrupt state. They also don't use lwarx/stwcx. and thus
	 * are known not to leave dangling reservations.
	 */
	.globl	fast_exception_return
fast_exception_return:
	ld	r3,_MSR(r1)
	ld	r4,_CTR(r1)
	ld	r0,_LINK(r1)
	mtctr	r4
	mtlr	r0
	ld	r4,_XER(r1)
	mtspr	SPRN_XER,r4

	REST_8GPRS(5, r1)

	andi.	r0,r3,MSR_RI
	beq-	.Lunrecov_restore

	/* Load PPR from thread struct before we clear MSR:RI */
BEGIN_FTR_SECTION
	ld	r2,PACACURRENT(r13)
	ld	r2,TASKTHREADPPR(r2)
END_FTR_SECTION_IFSET(CPU_FTR_HAS_PPR)

	/*
	 * Clear RI before restoring r13.  If we are returning to
	 * userspace and we take an exception after restoring r13,
	 * we end up corrupting the userspace r13 value.
	 */
	li	r4,0
	mtmsrd	r4,1

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	/* TM debug */
	std	r3, PACATMSCRATCH(r13) /* Stash returned-to MSR */
#endif
	/*
	 * r13 is our per cpu area, only restore it if we are returning to
	 * userspace the value stored in the stack frame may belong to
	 * another CPU.
	 */
	andi.	r0,r3,MSR_PR
	beq	1f
BEGIN_FTR_SECTION
	mtspr	SPRN_PPR,r2	/* Restore PPR */
END_FTR_SECTION_IFSET(CPU_FTR_HAS_PPR)
	ACCOUNT_CPU_USER_EXIT(r13, r2, r4)
	REST_GPR(13, r1)

	mtspr	SPRN_SRR1,r3

	ld	r2,_CCR(r1)
	mtcrf	0xFF,r2
	ld	r2,_NIP(r1)
	mtspr	SPRN_SRR0,r2

	ld	r0,GPR0(r1)
	ld	r2,GPR2(r1)
	ld	r3,GPR3(r1)
	ld	r4,GPR4(r1)
	ld	r1,GPR1(r1)
	RFI_TO_USER
	b	.	/* prevent speculative execution */

1:	mtspr	SPRN_SRR1,r3

	ld	r2,_CCR(r1)
	mtcrf	0xFF,r2
	ld	r2,_NIP(r1)
	mtspr	SPRN_SRR0,r2

	ld	r0,GPR0(r1)
	ld	r2,GPR2(r1)
	ld	r3,GPR3(r1)
	ld	r4,GPR4(r1)
	ld	r1,GPR1(r1)
	RFI_TO_KERNEL
	b	.	/* prevent speculative execution */

#endif /* CONFIG_PPC_BOOK3E */

	/*
	 * We are returning to a context with interrupts soft disabled.
	 *
	 * However, we may also about to hard enable, so we need to
	 * make sure that in this case, we also clear PACA_IRQ_HARD_DIS
	 * or that bit can get out of sync and bad things will happen
	 */
.Lrestore_irq_off:
	ld	r3,_MSR(r1)
	lbz	r7,PACAIRQHAPPENED(r13)
	andi.	r0,r3,MSR_EE
	beq	1f
	rlwinm	r7,r7,0,~PACA_IRQ_HARD_DIS
	stb	r7,PACAIRQHAPPENED(r13)
1:
#if defined(CONFIG_TRACE_IRQFLAGS) && defined(CONFIG_BUG)
	/* The interrupt should not have soft enabled. */
	lbz	r7,PACASOFTIRQEN(r13)
1:	tdnei	r7,0
	EMIT_BUG_ENTRY 1b,__FILE__,__LINE__,BUGFLAG_WARNING
#endif
	b	.Ldo_restore

	/*
	 * Something did happen, check if a re-emit is needed
	 * (this also clears paca->irq_happened)
	 */
.Lrestore_check_irq_replay:
	/* XXX: We could implement a fast path here where we check
	 * for irq_happened being just 0x01, in which case we can
	 * clear it and return. That means that we would potentially
	 * miss a decrementer having wrapped all the way around.
	 *
	 * Still, this might be useful for things like hash_page
	 */
	bl	__check_irq_replay
	cmpwi	cr0,r3,0
	beq	.Lrestore_no_replay
 
	/*
	 * We need to re-emit an interrupt. We do so by re-using our
	 * existing exception frame. We first change the trap value,
	 * but we need to ensure we preserve the low nibble of it
	 */
	ld	r4,_TRAP(r1)
	clrldi	r4,r4,60
	or	r4,r4,r3
	std	r4,_TRAP(r1)

	/*
	 * Then find the right handler and call it. Interrupts are
	 * still soft-disabled and we keep them that way.
	*/
	cmpwi	cr0,r3,0x500
	bne	1f
	addi	r3,r1,STACK_FRAME_OVERHEAD;
 	bl	do_IRQ
	b	ret_from_except
1:	cmpwi	cr0,r3,0xe60
	bne	1f
	addi	r3,r1,STACK_FRAME_OVERHEAD;
	bl	handle_hmi_exception
	b	ret_from_except
1:	cmpwi	cr0,r3,0x900
	bne	1f
	addi	r3,r1,STACK_FRAME_OVERHEAD;
	bl	timer_interrupt
	b	ret_from_except
#ifdef CONFIG_PPC_DOORBELL
1:
#ifdef CONFIG_PPC_BOOK3E
	cmpwi	cr0,r3,0x280
#else
	cmpwi	cr0,r3,0xa00
#endif /* CONFIG_PPC_BOOK3E */
	bne	1f
	addi	r3,r1,STACK_FRAME_OVERHEAD;
	bl	doorbell_exception
#endif /* CONFIG_PPC_DOORBELL */
1:	b	ret_from_except /* What else to do here ? */
 
.Lunrecov_restore:
	addi	r3,r1,STACK_FRAME_OVERHEAD
	bl	unrecoverable_exception
	b	.Lunrecov_restore

_ASM_NOKPROBE_SYMBOL(ret_from_except);
_ASM_NOKPROBE_SYMBOL(ret_from_except_lite);
_ASM_NOKPROBE_SYMBOL(resume_kernel);
_ASM_NOKPROBE_SYMBOL(fast_exc_return_irq);
_ASM_NOKPROBE_SYMBOL(restore);
_ASM_NOKPROBE_SYMBOL(fast_exception_return);


#ifdef CONFIG_PPC_RTAS
/*
 * On CHRP, the Run-Time Abstraction Services (RTAS) have to be
 * called with the MMU off.
 *
 * In addition, we need to be in 32b mode, at least for now.
 * 
 * Note: r3 is an input parameter to rtas, so don't trash it...
 */
_GLOBAL(enter_rtas)
	mflr	r0
	std	r0,16(r1)
        stdu	r1,-RTAS_FRAME_SIZE(r1)	/* Save SP and create stack space. */

	/* Because RTAS is running in 32b mode, it clobbers the high order half
	 * of all registers that it saves.  We therefore save those registers
	 * RTAS might touch to the stack.  (r0, r3-r13 are caller saved)
   	 */
	SAVE_GPR(2, r1)			/* Save the TOC */
	SAVE_GPR(13, r1)		/* Save paca */
	SAVE_8GPRS(14, r1)		/* Save the non-volatiles */
	SAVE_10GPRS(22, r1)		/* ditto */

	mfcr	r4
	std	r4,_CCR(r1)
	mfctr	r5
	std	r5,_CTR(r1)
	mfspr	r6,SPRN_XER
	std	r6,_XER(r1)
	mfdar	r7
	std	r7,_DAR(r1)
	mfdsisr	r8
	std	r8,_DSISR(r1)

	/* Temporary workaround to clear CR until RTAS can be modified to
	 * ignore all bits.
	 */
	li	r0,0
	mtcr	r0

#ifdef CONFIG_BUG	
	/* There is no way it is acceptable to get here with interrupts enabled,
	 * check it with the asm equivalent of WARN_ON
	 */
	lbz	r0,PACASOFTIRQEN(r13)
1:	tdnei	r0,0
	EMIT_BUG_ENTRY 1b,__FILE__,__LINE__,BUGFLAG_WARNING
#endif
	
	/* Hard-disable interrupts */
	mfmsr	r6
	rldicl	r7,r6,48,1
	rotldi	r7,r7,16
	mtmsrd	r7,1

	/* Unfortunately, the stack pointer and the MSR are also clobbered,
	 * so they are saved in the PACA which allows us to restore
	 * our original state after RTAS returns.
         */
	std	r1,PACAR1(r13)
        std	r6,PACASAVEDMSR(r13)

	/* Setup our real return addr */	
	LOAD_REG_ADDR(r4,rtas_return_loc)
	clrldi	r4,r4,2			/* convert to realmode address */
       	mtlr	r4

	li	r0,0
	ori	r0,r0,MSR_EE|MSR_SE|MSR_BE|MSR_RI
	andc	r0,r6,r0
	
        li      r9,1
        rldicr  r9,r9,MSR_SF_LG,(63-MSR_SF_LG)
	ori	r9,r9,MSR_IR|MSR_DR|MSR_FE0|MSR_FE1|MSR_FP|MSR_RI|MSR_LE
	andc	r6,r0,r9

__enter_rtas:
	sync				/* disable interrupts so SRR0/1 */
	mtmsrd	r0			/* don't get trashed */

	LOAD_REG_ADDR(r4, rtas)
	ld	r5,RTASENTRY(r4)	/* get the rtas->entry value */
	ld	r4,RTASBASE(r4)		/* get the rtas->base value */
	
	mtspr	SPRN_SRR0,r5
	mtspr	SPRN_SRR1,r6
	RFI_TO_KERNEL
	b	.	/* prevent speculative execution */

rtas_return_loc:
	FIXUP_ENDIAN

	/* relocation is off at this point */
	GET_PACA(r4)
	clrldi	r4,r4,2			/* convert to realmode address */

	bcl	20,31,$+4
0:	mflr	r3
	ld	r3,(1f-0b)(r3)		/* get &rtas_restore_regs */

	mfmsr   r6
	li	r0,MSR_RI
	andc	r6,r6,r0
	sync	
	mtmsrd  r6
        
        ld	r1,PACAR1(r4)           /* Restore our SP */
        ld	r4,PACASAVEDMSR(r4)     /* Restore our MSR */

	mtspr	SPRN_SRR0,r3
	mtspr	SPRN_SRR1,r4
	RFI_TO_KERNEL
	b	.	/* prevent speculative execution */
_ASM_NOKPROBE_SYMBOL(__enter_rtas)
_ASM_NOKPROBE_SYMBOL(rtas_return_loc)

	.align	3
1:	.8byte	rtas_restore_regs

rtas_restore_regs:
	/* relocation is on at this point */
	REST_GPR(2, r1)			/* Restore the TOC */
	REST_GPR(13, r1)		/* Restore paca */
	REST_8GPRS(14, r1)		/* Restore the non-volatiles */
	REST_10GPRS(22, r1)		/* ditto */

	GET_PACA(r13)

	ld	r4,_CCR(r1)
	mtcr	r4
	ld	r5,_CTR(r1)
	mtctr	r5
	ld	r6,_XER(r1)
	mtspr	SPRN_XER,r6
	ld	r7,_DAR(r1)
	mtdar	r7
	ld	r8,_DSISR(r1)
	mtdsisr	r8

        addi	r1,r1,RTAS_FRAME_SIZE	/* Unstack our frame */
	ld	r0,16(r1)		/* get return address */

	mtlr    r0
        blr				/* return to caller */

#endif /* CONFIG_PPC_RTAS */

_GLOBAL(enter_prom)
	mflr	r0
	std	r0,16(r1)
        stdu	r1,-PROM_FRAME_SIZE(r1)	/* Save SP and create stack space */

	/* Because PROM is running in 32b mode, it clobbers the high order half
	 * of all registers that it saves.  We therefore save those registers
	 * PROM might touch to the stack.  (r0, r3-r13 are caller saved)
   	 */
	SAVE_GPR(2, r1)
	SAVE_GPR(13, r1)
	SAVE_8GPRS(14, r1)
	SAVE_10GPRS(22, r1)
	mfcr	r10
	mfmsr	r11
	std	r10,_CCR(r1)
	std	r11,_MSR(r1)

	/* Put PROM address in SRR0 */
	mtsrr0	r4

	/* Setup our trampoline return addr in LR */
	bcl	20,31,$+4
0:	mflr	r4
	addi	r4,r4,(1f - 0b)
       	mtlr	r4

	/* Prepare a 32-bit mode big endian MSR
	 */
#ifdef CONFIG_PPC_BOOK3E
	rlwinm	r11,r11,0,1,31
	mtsrr1	r11
	rfi
#else /* CONFIG_PPC_BOOK3E */
	LOAD_REG_IMMEDIATE(r12, MSR_SF | MSR_ISF | MSR_LE)
	andc	r11,r11,r12
	mtsrr1	r11
	RFI_TO_KERNEL
#endif /* CONFIG_PPC_BOOK3E */

1:	/* Return from OF */
	FIXUP_ENDIAN

	/* Just make sure that r1 top 32 bits didn't get
	 * corrupt by OF
	 */
	rldicl	r1,r1,0,32

	/* Restore the MSR (back to 64 bits) */
	ld	r0,_MSR(r1)
	MTMSRD(r0)
        isync

	/* Restore other registers */
	REST_GPR(2, r1)
	REST_GPR(13, r1)
	REST_8GPRS(14, r1)
	REST_10GPRS(22, r1)
	ld	r4,_CCR(r1)
	mtcr	r4
	
        addi	r1,r1,PROM_FRAME_SIZE
	ld	r0,16(r1)
	mtlr    r0
        blr
