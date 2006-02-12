/*
 * signal32.c: Support 32bit signal syscalls.
 *
 * Copyright (C) 2001 IBM
 * Copyright (C) 1997,1998 Jakub Jelinek (jj@sunsite.mff.cuni.cz)
 * Copyright (C) 1997 David S. Miller (davem@caip.rutgers.edu)
 *
 * These routines maintain argument size conversion between 32bit and 64bit
 * environment.
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */

#include <asm/ptrace.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h> 
#include <linux/mm.h> 
#include <linux/file.h> 
#include <linux/signal.h>
#include <linux/utime.h>
#include <linux/resource.h>
#include <linux/times.h>
#include <linux/utsname.h>
#include <linux/timex.h>
#include <linux/smp.h>
#include <linux/smp_lock.h>
#include <linux/sem.h>
#include <linux/msg.h>
#include <linux/shm.h>
#include <linux/slab.h>
#include <linux/uio.h>
#include <linux/nfs_fs.h>
#include <linux/smb_fs.h>
#include <linux/smb_mount.h>
#include <linux/ncp_fs.h>
#include <linux/quota.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/personality.h>
#include <linux/stat.h>
#include <linux/filter.h>
#include <asm/types.h>
#include <asm/ipc.h>
#include <asm/uaccess.h>
#include <linux/elf.h>
#include <asm/ppc32.h>
#include <asm/unistd.h>
#include <asm/ucontext.h>

#define _BLOCKABLE (~(sigmask(SIGKILL) | sigmask(SIGSTOP)))
/* 
 * These are the flags in the MSR that the user is allowed to change
 * by modifying the saved value of the MSR on the stack.  SE and BE
 * should not be in this list since gdb may want to change these.  I.e,
 * you should be able to step out of a signal handler to see what
 * instruction executes next after the signal handler completes.
 * Alternately, if you stepped into a signal handler, you should be
 * able to continue 'til the next breakpoint from within the signal
 * handler, even if the handler returns.
 */
#define MSR_USERCHANGE	(MSR_FE0 | MSR_FE1)

struct timespec32 {
	s32    tv_sec;
	s32    tv_nsec;
};

struct sigregs32 {
	/*
	 * the gp_regs array is 32 bit representation of the pt_regs
	 * structure that was stored on the kernle stack during the
	 * system call that was interrupted for the signal.
	 *
	 * Note that the entire pt_regs regs structure will fit in
	 * the gp_regs structure because the ELF_NREG value is 48 for
	 * PPC and the pt_regs structure contains 44 registers
	 */
	elf_gregset_t32	gp_regs;
	double		fp_regs[ELF_NFPREG];
	unsigned int	tramp[2];
	/*
	 * Programs using the rs6000/xcoff abi can save up to 19 gp
	 * regs and 18 fp regs below sp before decrementing it.
	 */
	int		abigap[56];
};


struct rt_sigframe_32 {
	/*
	 * Unused space at start of frame to allow for storing of
	 * stack pointers
	 */
	unsigned long _unused;
	/*
	 * This is a 32 bit pointer in user address space 
	 *     it is a pointer to the siginfo stucture in the rt stack frame 
	 */
	u32 pinfo;
	/*
	 * This is a 32 bit pointer in user address space
	 * it is a pointer to the user context in the rt stack frame
	 */
	u32 puc;
	struct siginfo32  info;
	struct ucontext32 uc;
};





extern asmlinkage long sys_wait4(pid_t pid,unsigned int * stat_addr, int options, struct rusage * ru);


/****************************************************************************/
/*  Start of nonRT signal support                                           */
/*                                                                          */
/*     sigset_t is 32 bits for non-rt signals                               */
/*                                                                          */
/*  System Calls                                                            */
/*       sigaction                sys32_sigaction                           */
/*       sigpending               sys32_sigpending                          */
/*       sigprocmask              sys32_sigprocmask                         */
/*       sigreturn                sys32_sigreturn                           */
/*                                                                          */
/*  Note sigsuspend has no special 32 bit routine - uses the 64 bit routine */ 
/*                                                                          */
/*  Other routines                                                          */
/*        setup_frame32                                                     */
/*                                                                          */
/****************************************************************************/


asmlinkage long sys32_sigaction(int sig, struct old_sigaction32 *act, struct old_sigaction32 *oact)
{
	struct k_sigaction new_ka, old_ka;
	int ret;
	u32 handler, restorer;
	
	if (sig < 0)
		sig = -sig;

	if (act) {
		old_sigset_t32 mask;

		ret = get_user(handler, &act->sa_handler);
		ret |= __get_user(restorer, &act->sa_restorer);
		ret |= __get_user(new_ka.sa.sa_flags, &act->sa_flags);
		ret |= __get_user(mask, &act->sa_mask);
		if (ret)
			return ret;
		new_ka.sa.sa_handler = (__sighandler_t)(long) handler;
		new_ka.sa.sa_restorer = (void (*)(void))(long) restorer;
		siginitset(&new_ka.sa.sa_mask, mask);
	}

	ret = do_sigaction(sig, act ? &new_ka : NULL, oact ? &old_ka : NULL);

	if (!ret && oact)
	{
		ret = put_user((long)old_ka.sa.sa_handler, &oact->sa_handler);
		ret |= __put_user((long)old_ka.sa.sa_restorer, &oact->sa_restorer);
		ret |= __put_user(old_ka.sa.sa_flags, &oact->sa_flags);
		ret |= __put_user(old_ka.sa.sa_mask.sig[0], &oact->sa_mask);
	}

	return ret;
}




extern asmlinkage long sys_sigpending(old_sigset_t *set);

asmlinkage long sys32_sigpending(old_sigset_t32 *set)
{
	old_sigset_t s;
	int ret;
	mm_segment_t old_fs = get_fs();
	
	set_fs (KERNEL_DS);
	ret = sys_sigpending(&s);
	set_fs (old_fs);
	if (put_user (s, set)) return -EFAULT;
	
	return ret;
}




extern asmlinkage long sys_sigprocmask(int how, old_sigset_t *set, old_sigset_t *oset);

/* Note: it is necessary to treat how as an unsigned int, 
 * with the corresponding cast to a signed int to insure that the 
 * proper conversion (sign extension) between the register representation of a signed int (msr in 32-bit mode)
 * and the register representation of a signed int (msr in 64-bit mode) is performed.
 */
asmlinkage long sys32_sigprocmask(u32 how, old_sigset_t32 *set, old_sigset_t32 *oset)
{
	old_sigset_t s;
	int ret;
	mm_segment_t old_fs = get_fs();
	
	if (set && get_user(s, set))
		return -EFAULT;
	set_fs (KERNEL_DS);
	ret = sys_sigprocmask((int)how, set ? &s : NULL, oset ? &s : NULL);
	set_fs (old_fs);
	
	if (ret)
		return ret;
	if (oset && put_user (s, oset))
		return -EFAULT;
	return 0;
}



/*
 * When we have signals to deliver, we set up on the
 * user stack, going down from the original stack pointer:
 *	a sigregs struct
 *	one or more sigcontext structs
 *	a gap of __SIGNAL_FRAMESIZE32 bytes
 *
 * Each of these things must be a multiple of 16 bytes in size.
 *
*/


/*
 * Do a signal return; undo the signal stack.
 */
long sys32_sigreturn(unsigned long r3, unsigned long r4, unsigned long r5,
		     unsigned long r6, unsigned long r7, unsigned long r8,
		     struct pt_regs *regs)
{
	struct sigcontext32 *sc, sigctx;
	struct sigregs32 *sr;
	elf_gregset_t32 saved_regs;  /* an array of ELF_NGREG unsigned ints (32 bits) */
	sigset_t set;
	int i;

	sc = (struct sigcontext32 *)(regs->gpr[1] + __SIGNAL_FRAMESIZE32);
	if (copy_from_user(&sigctx, sc, sizeof(sigctx)))
		goto badframe;

	/*
	 * Note that PPC32 puts the upper 32 bits of the sigmask in the
	 * unused part of the signal stackframe
	 */
	set.sig[0] = sigctx.oldmask + ((long)(sigctx._unused[3])<< 32);
	sigdelsetmask(&set, ~_BLOCKABLE);
	spin_lock_irq(&current->sigmask_lock);
	current->blocked = set;
	recalc_sigpending(current);
	spin_unlock_irq(&current->sigmask_lock);

	sr = (struct sigregs32*)(u64)sigctx.regs;
	/*
	 * Copy the 32 bit register values off the user stack
	 * into the 32 bit register area
	 */
	if (copy_from_user(saved_regs, &sr->gp_regs,sizeof(sr->gp_regs)))
		goto badframe;

	/*
	 * The saved reg structure in the frame is an elf_grepset_t32,
	 * it is a 32 bit register save of the registers in the
	 * pt_regs structure that was stored on the kernel stack
	 * during the system call when the system call was interrupted
	 * for the signal. Only 32 bits are saved because the
	 * sigcontext contains a pointer to the regs and the sig
	 * context address is passed as a pointer to the signal
	 * handler.  
	 *
	 * The entries in the elf_grepset have the same index as the
	 * elements in the pt_regs structure.
	 */
	for (i = 0; i < 32; i++)
		regs->gpr[i] = (u64)(saved_regs[i]) & 0xFFFFFFFF;

	/*
	 *  restore the non gpr registers 
	 */
	regs->nip = (u64)(saved_regs[PT_NIP]) & 0xFFFFFFFF;
	regs->orig_gpr3 = (u64)(saved_regs[PT_ORIG_R3]) & 0xFFFFFFFF; 
	regs->ctr = (u64)(saved_regs[PT_CTR]) & 0xFFFFFFFF; 
	regs->link = (u64)(saved_regs[PT_LNK]) & 0xFFFFFFFF; 
	regs->xer = (u64)(saved_regs[PT_XER]) & 0xFFFFFFFF; 
	regs->ccr = (u64)(saved_regs[PT_CCR]) & 0xFFFFFFFF;
	/* regs->softe is left unchanged (like the MSR.EE bit) */
	regs->result = (u64)(saved_regs[PT_RESULT]) & 0xFFFFFFFF;

	/* force the process to reload the FP registers from
	   current->thread when it next does FP instructions */
	regs->msr &= ~(MSR_FP | MSR_FE0 | MSR_FE1);
#ifndef CONFIG_SMP
	if (last_task_used_math == current)
		last_task_used_math = NULL;
#endif
	if (copy_from_user(current->thread.fpr, &sr->fp_regs,
			   sizeof(sr->fp_regs)))
		goto badframe;

	return regs->result;

badframe:
	force_sig(SIGSEGV, current);
	return 0;
}	

/*
 * Set up a signal frame.
 */
static void
setup_frame32(struct pt_regs *regs, int sig, struct k_sigaction *ka,
	      struct sigregs32 *frame, unsigned int newsp)
{
	int i;

	if (verify_area(VERIFY_WRITE, frame, sizeof(*frame)))
		goto badframe;
	if (regs->msr & MSR_FP)
		giveup_fpu(current);

	/*
	 * Copy the register contents for the pt_regs structure on the
	 *   kernel stack to the elf_gregset_t32 structure on the user
	 *   stack. This is a copy of 64 bit register values to 32 bit
	 *   register values. The high order 32 bits of the 64 bit
	 *   registers are not needed since a 32 bit application is
	 *   running and the saved registers are the contents of the
	 *   user registers at the time of a system call.
	 * 
	 * The values saved on the user stack will be restored into
	 *  the registers during the signal return processing
	 *
	 * Note the +1 is needed in order to get the lower 32 bits
	 * of 64 bit register
	 */
	for (i = 0; i < sizeof(struct pt_regs32)/sizeof(u32); i++) {
		if (__copy_to_user(&frame->gp_regs[i], (u32*)(&regs->gpr[i])+1, sizeof(u32)))
			goto badframe;
	}

	/*
	 * Now copy the floating point registers onto the user stack 
	 *
	 * Also set up so on the completion of the signal handler, the
	 * sys_sigreturn will get control to reset the stack
	 */
	if (__copy_to_user(&frame->fp_regs, current->thread.fpr,
			   ELF_NFPREG * sizeof(double))
	    /* li r0, __NR_sigreturn */
	    || __put_user(0x38000000U + __NR_sigreturn, &frame->tramp[0])
	    /* sc */
	    || __put_user(0x44000002U, &frame->tramp[1]))
		goto badframe;

	flush_icache_range((unsigned long) &frame->tramp[0],
			   (unsigned long) &frame->tramp[2]);
	current->thread.fpscr = 0;      /* turn off all fp exceptions */

	/*
	 * first parameter to the signal handler is the signal number
	 *  - the value is in gpr3
	 * second parameter to the signal handler is the sigcontext
	 *   - set the value into gpr4
	 */
	regs->gpr[3] = sig;
	regs->gpr[4] = newsp;
	regs->nip = (u64)ka->sa.sa_handler & 0xFFFFFFFF;
	regs->link = (unsigned long) frame->tramp;

	newsp -= __SIGNAL_FRAMESIZE32;
	if (put_user(regs->gpr[1], (u32*)(u64)newsp))
		goto badframe;
	regs->gpr[1] = newsp;

	return;

 badframe:
#if DEBUG_SIG
	printk("badframe in setup_frame32, regs=%p frame=%p newsp=%lx\n",
	       regs, frame, newsp);
#endif
	if (sig == SIGSEGV)
		ka->sa.sa_handler = SIG_DFL;
	force_sig(SIGSEGV, current);
}


/*
 *  Start of RT signal support
 *
 *     sigset_t is 64 bits for rt signals
 *
 *  System Calls
 *       sigaction                sys32_rt_sigaction
 *       sigpending               sys32_rt_sigpending
 *       sigprocmask              sys32_rt_sigprocmask
 *       sigreturn                sys32_rt_sigreturn
 *       sigtimedwait             sys32_rt_sigtimedwait
 *       sigqueueinfo             sys32_rt_sigqueueinfo
 *       sigsuspend               sys32_rt_sigsuspend
 *
 *  Other routines
 *        setup_rt_frame32
 *        copy_siginfo_to_user32
 *        siginfo32to64
 */


/*
 * This code executes after the rt signal handler in 32 bit mode has
 * completed and returned  
 */
long sys32_rt_sigreturn(unsigned long r3, unsigned long r4, unsigned long r5,
			unsigned long r6, unsigned long r7, unsigned long r8,
			struct pt_regs * regs)
{
	struct rt_sigframe_32 *rt_stack_frame;
	struct sigcontext32 sigctx;
	struct sigregs32 *signalregs;
 
	int i;
	elf_gregset_t32 saved_regs;   /* an array of 32 bit register values */
	sigset_t signal_set; 
	stack_t stack;

	/* Adjust the inputted reg1 to point to the rt signal frame */
	rt_stack_frame = (struct rt_sigframe_32 *)(regs->gpr[1] + __SIGNAL_FRAMESIZE32);
	/* Copy the information from the user stack  */
	if (copy_from_user(&sigctx, &rt_stack_frame->uc.uc_mcontext,sizeof(sigctx))
	    || copy_from_user(&signal_set, &rt_stack_frame->uc.uc_sigmask,sizeof(signal_set))
	    || copy_from_user(&stack,&rt_stack_frame->uc.uc_stack,sizeof(stack)))
		/* unable to copy from user storage */
		goto badframe;

	/*
	 * Unblock the signal that was processed 
	 *   After a signal handler runs - 
	 *     if the signal is blockable - the signal will be unblocked  
	 *       ( sigkill and sigstop are not blockable)
	 */
	sigdelsetmask(&signal_set, ~_BLOCKABLE); 
	/* update the current based on the sigmask found in the rt_stackframe */
	spin_lock_irq(&current->sigmask_lock);
	current->blocked = signal_set;
	recalc_sigpending(current);
	spin_unlock_irq(&current->sigmask_lock);

	signalregs = (struct sigregs32 *) (u64)sigctx.regs;

	if (copy_from_user(saved_regs, &signalregs->gp_regs,
			   sizeof(signalregs->gp_regs))) 
		goto badframe;

	/*
	 * The saved reg structure in the frame is an elf_grepset_t32,
	 * it is a 32 bit register save of the registers in the
	 * pt_regs structure that was stored on the kernel stack
	 * during the system call when the system call was interrupted
	 * for the signal. Only 32 bits are saved because the
	 * sigcontext contains a pointer to the regs and the sig
	 * context address is passed as a pointer to the signal handler
	 *
	 * The entries in the elf_grepset have the same index as
	 * the elements in the pt_regs structure.
	 */
	for (i = 0; i < 32; i++)
		regs->gpr[i] = (u64)(saved_regs[i]) & 0xFFFFFFFF;
	/*
	 * restore the non gpr registers
	 */
	regs->nip = (u64)(saved_regs[PT_NIP]) & 0xFFFFFFFF;
	regs->orig_gpr3 = (u64)(saved_regs[PT_ORIG_R3]) & 0xFFFFFFFF; 
	regs->ctr = (u64)(saved_regs[PT_CTR]) & 0xFFFFFFFF; 
	regs->link = (u64)(saved_regs[PT_LNK]) & 0xFFFFFFFF; 
	regs->xer = (u64)(saved_regs[PT_XER]) & 0xFFFFFFFF; 
	regs->ccr = (u64)(saved_regs[PT_CCR]) & 0xFFFFFFFF;
	/* regs->softe is left unchanged (like MSR.EE) */
	regs->result = (u64)(saved_regs[PT_RESULT]) & 0xFFFFFFFF;

	/* force the process to reload the FP registers from
	   current->thread when it next does FP instructions */
	regs->msr &= ~(MSR_FP | MSR_FE0 | MSR_FE1);
#ifndef CONFIG_SMP
	if (last_task_used_math == current)
		last_task_used_math = NULL;
#endif
	if (copy_from_user(current->thread.fpr, &signalregs->fp_regs,
			   sizeof(signalregs->fp_regs))) 
		goto badframe;

	return regs->result;

 badframe:
	force_sig(SIGSEGV, current);
	return 0;
}



asmlinkage long sys32_rt_sigaction(int sig, const struct sigaction32 *act, struct sigaction32 *oact, size_t sigsetsize)
{
	struct k_sigaction new_ka, old_ka;
	int ret;
	sigset32_t set32;
	u32 handler;

	/* XXX: Don't preclude handling different sized sigset_t's.  */
	if (sigsetsize != sizeof(sigset32_t))
		return -EINVAL;

	if (act) {
		ret = get_user(handler, &act->sa_handler);
		ret |= __copy_from_user(&set32, &act->sa_mask,
					sizeof(sigset32_t));
		switch (_NSIG_WORDS) {
		case 4: new_ka.sa.sa_mask.sig[3] = set32.sig[6]
				| (((long)set32.sig[7]) << 32);
		case 3: new_ka.sa.sa_mask.sig[2] = set32.sig[4]
				| (((long)set32.sig[5]) << 32);
		case 2: new_ka.sa.sa_mask.sig[1] = set32.sig[2]
				| (((long)set32.sig[3]) << 32);
		case 1: new_ka.sa.sa_mask.sig[0] = set32.sig[0]
				| (((long)set32.sig[1]) << 32);
		}

		ret |= __get_user(new_ka.sa.sa_flags, &act->sa_flags);
		
		if (ret)
			return -EFAULT;
		new_ka.sa.sa_handler = (__sighandler_t)(long) handler;
	}

	ret = do_sigaction(sig, act ? &new_ka : NULL, oact ? &old_ka : NULL);

	if (!ret && oact) {
		switch (_NSIG_WORDS) {
		case 4:
			set32.sig[7] = (old_ka.sa.sa_mask.sig[3] >> 32);
			set32.sig[6] = old_ka.sa.sa_mask.sig[3];
		case 3:
			set32.sig[5] = (old_ka.sa.sa_mask.sig[2] >> 32);
			set32.sig[4] = old_ka.sa.sa_mask.sig[2];
		case 2:
			set32.sig[3] = (old_ka.sa.sa_mask.sig[1] >> 32);
			set32.sig[2] = old_ka.sa.sa_mask.sig[1];
		case 1:
			set32.sig[1] = (old_ka.sa.sa_mask.sig[0] >> 32);
			set32.sig[0] = old_ka.sa.sa_mask.sig[0];
		}
		ret = put_user((long)old_ka.sa.sa_handler, &oact->sa_handler);
		ret |= __copy_to_user(&oact->sa_mask, &set32,
				      sizeof(sigset32_t));
		ret |= __put_user(old_ka.sa.sa_flags, &oact->sa_flags);
	}

  	return ret;
}


extern asmlinkage long sys_rt_sigprocmask(int how, sigset_t *set, sigset_t *oset,
					  size_t sigsetsize);

/*
 * Note: it is necessary to treat how as an unsigned int, with the
 * corresponding cast to a signed int to insure that the proper
 * conversion (sign extension) between the register representation
 * of a signed int (msr in 32-bit mode) and the register representation
 * of a signed int (msr in 64-bit mode) is performed.
 */
asmlinkage long sys32_rt_sigprocmask(u32 how, sigset32_t *set, sigset32_t *oset, size_t sigsetsize)
{
	sigset_t s;
	sigset32_t s32;
	int ret;
	mm_segment_t old_fs = get_fs();

	if (set) {
		if (copy_from_user (&s32, set, sizeof(sigset32_t)))
			return -EFAULT;
    
		switch (_NSIG_WORDS) {
		case 4: s.sig[3] = s32.sig[6] | (((long)s32.sig[7]) << 32);
		case 3: s.sig[2] = s32.sig[4] | (((long)s32.sig[5]) << 32);
		case 2: s.sig[1] = s32.sig[2] | (((long)s32.sig[3]) << 32);
		case 1: s.sig[0] = s32.sig[0] | (((long)s32.sig[1]) << 32);
		}
	}
	
	set_fs (KERNEL_DS);
	ret = sys_rt_sigprocmask((int)how, set ? &s : NULL, oset ? &s : NULL,
				 sigsetsize); 
	set_fs (old_fs);
	if (ret)
		return ret;
	if (oset) {
		switch (_NSIG_WORDS) {
		case 4: s32.sig[7] = (s.sig[3] >> 32); s32.sig[6] = s.sig[3];
		case 3: s32.sig[5] = (s.sig[2] >> 32); s32.sig[4] = s.sig[2];
		case 2: s32.sig[3] = (s.sig[1] >> 32); s32.sig[2] = s.sig[1];
		case 1: s32.sig[1] = (s.sig[0] >> 32); s32.sig[0] = s.sig[0];
		}
		if (copy_to_user (oset, &s32, sizeof(sigset32_t)))
			return -EFAULT;
	}
	return 0;
}


extern asmlinkage long sys_rt_sigpending(sigset_t *set, size_t sigsetsize);



asmlinkage long sys32_rt_sigpending(sigset32_t *set,   __kernel_size_t32 sigsetsize)
{

	sigset_t s;
	sigset32_t s32;
	int ret;
	mm_segment_t old_fs = get_fs();
		
	set_fs (KERNEL_DS);
	ret = sys_rt_sigpending(&s, sigsetsize);
	set_fs (old_fs);
	if (!ret) {
		switch (_NSIG_WORDS) {
		case 4: s32.sig[7] = (s.sig[3] >> 32); s32.sig[6] = s.sig[3];
		case 3: s32.sig[5] = (s.sig[2] >> 32); s32.sig[4] = s.sig[2];
		case 2: s32.sig[3] = (s.sig[1] >> 32); s32.sig[2] = s.sig[1];
		case 1: s32.sig[1] = (s.sig[0] >> 32); s32.sig[0] = s.sig[0];
		}
		if (copy_to_user (set, &s32, sizeof(sigset32_t)))
			return -EFAULT;
	}
	return ret;
}



siginfo_t32 *
siginfo64to32(siginfo_t32 *d, siginfo_t *s)
{
	memset (d, 0, sizeof(siginfo_t32));
	d->si_signo = s->si_signo;
	d->si_errno = s->si_errno;
	d->si_code = (short)s->si_code;
	if (s->si_signo >= SIGRTMIN) {
		d->si_pid = s->si_pid;
		d->si_uid = s->si_uid;
		
		d->si_int = s->si_int;
	} else switch (s->si_signo) {
	/* XXX: What about POSIX1.b timers */
	case SIGCHLD:
		d->si_pid = s->si_pid;
		d->si_status = s->si_status;
		d->si_utime = s->si_utime;
		d->si_stime = s->si_stime;
		break;
	case SIGSEGV:
	case SIGBUS:
	case SIGFPE:
	case SIGILL:
		d->si_addr = (unsigned int)(u64)(s->si_addr);
        break;
	case SIGPOLL:
		d->si_band = s->si_band;
		d->si_fd = s->si_fd;
		break;
	default:
		d->si_pid = s->si_pid;
		d->si_uid = s->si_uid;
		break;
	}
	return d;
}

extern asmlinkage long
sys_rt_sigtimedwait(const sigset_t *uthese, siginfo_t *uinfo,
		    const struct timespec *uts, size_t sigsetsize);

asmlinkage long
sys32_rt_sigtimedwait(sigset32_t *uthese, siginfo_t32 *uinfo,
		      struct timespec32 *uts, __kernel_size_t32 sigsetsize)
{
	sigset_t s;
	sigset32_t s32;
	struct timespec t;
	int ret;
	mm_segment_t old_fs = get_fs();
	siginfo_t info;
	siginfo_t32 info32;
		
	if (copy_from_user (&s32, uthese, sizeof(sigset32_t)))
		return -EFAULT;
	switch (_NSIG_WORDS) {
	case 4: s.sig[3] = s32.sig[6] | (((long)s32.sig[7]) << 32);
	case 3: s.sig[2] = s32.sig[4] | (((long)s32.sig[5]) << 32);
	case 2: s.sig[1] = s32.sig[2] | (((long)s32.sig[3]) << 32);
	case 1: s.sig[0] = s32.sig[0] | (((long)s32.sig[1]) << 32);
	}
	if (uts) {
		ret = get_user (t.tv_sec, &uts->tv_sec);
		ret |= __get_user (t.tv_nsec, &uts->tv_nsec);
		if (ret)
			return -EFAULT;
	}
	set_fs (KERNEL_DS);
	if (uts) 
		ret = sys_rt_sigtimedwait(&s, &info, &t, sigsetsize);
	else
		ret = sys_rt_sigtimedwait(&s, &info, (struct timespec *)uts,
				sigsetsize);
	set_fs (old_fs);
	if (ret >= 0 && uinfo) {
		if (copy_to_user (uinfo, siginfo64to32(&info32, &info),
				  sizeof(siginfo_t32)))
			return -EFAULT;
	}
	return ret;
}



siginfo_t *
siginfo32to64(siginfo_t *d, siginfo_t32 *s)
{
	d->si_signo = s->si_signo;
	d->si_errno = s->si_errno;
	d->si_code = s->si_code;
	if (s->si_signo >= SIGRTMIN) {
		d->si_pid = s->si_pid;
		d->si_uid = s->si_uid;
		d->si_int = s->si_int;
        
	} else switch (s->si_signo) {
	/* XXX: What about POSIX1.b timers */
	case SIGCHLD:
		d->si_pid = s->si_pid;
		d->si_status = s->si_status;
		d->si_utime = s->si_utime;
		d->si_stime = s->si_stime;
		break;
	case SIGSEGV:
	case SIGBUS:
	case SIGFPE:
	case SIGILL:
		d->si_addr = (void *)A(s->si_addr);
  		break;
	case SIGPOLL:
		d->si_band = s->si_band;
		d->si_fd = s->si_fd;
		break;
	default:
		d->si_pid = s->si_pid;
		d->si_uid = s->si_uid;
		break;
	}
	return d;
}


extern asmlinkage long sys_rt_sigqueueinfo(int pid, int sig, siginfo_t *uinfo);

/*
 * Note: it is necessary to treat pid and sig as unsigned ints, with the
 * corresponding cast to a signed int to insure that the proper conversion
 * (sign extension) between the register representation of a signed int
 * (msr in 32-bit mode) and the register representation of a signed int
 * (msr in 64-bit mode) is performed.
 */
asmlinkage long sys32_rt_sigqueueinfo(u32 pid, u32 sig, siginfo_t32 *uinfo)
{
	siginfo_t info;
	siginfo_t32 info32;
	int ret;
	mm_segment_t old_fs = get_fs();
	
	if (copy_from_user (&info32, uinfo, sizeof(siginfo_t32)))
		return -EFAULT;
    	/* XXX: Is this correct? */
	siginfo32to64(&info, &info32);

	set_fs (KERNEL_DS);
	ret = sys_rt_sigqueueinfo((int)pid, (int)sig, &info);
	set_fs (old_fs);
	return ret;
}


int do_signal(sigset_t *oldset, struct pt_regs *regs);
int sys32_rt_sigsuspend(sigset32_t* unewset, size_t sigsetsize, int p3, int p4, int p6, int p7, struct pt_regs *regs)
{
	sigset_t saveset, newset;
	
	sigset32_t s32;

	/* XXX: Don't preclude handling different sized sigset_t's.  */
	if (sigsetsize != sizeof(sigset_t))
		return -EINVAL;

	if (copy_from_user(&s32, unewset, sizeof(s32)))
		return -EFAULT;

	/*
	 * Swap the 2 words of the 64-bit sigset_t (they are stored
	 * in the "wrong" endian in 32-bit user storage).
	 */
	switch (_NSIG_WORDS) {
		case 4: newset.sig[3] = s32.sig[6] | (((long)s32.sig[7]) << 32);
		case 3: newset.sig[2] = s32.sig[4] | (((long)s32.sig[5]) << 32);
		case 2: newset.sig[1] = s32.sig[2] | (((long)s32.sig[3]) << 32);
		case 1: newset.sig[0] = s32.sig[0] | (((long)s32.sig[1]) << 32);
	}

	sigdelsetmask(&newset, ~_BLOCKABLE);

	spin_lock_irq(&current->sigmask_lock);
	saveset = current->blocked;
	current->blocked = newset;
	recalc_sigpending(current);
	spin_unlock_irq(&current->sigmask_lock);

	regs->result = -EINTR;
	regs->gpr[3] = EINTR;
	regs->ccr |= 0x10000000;
	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		if (do_signal(&saveset, regs))
			/*
			 * If a signal handler needs to be called,
			 * do_signal() has set R3 to the signal number (the
			 * first argument of the signal handler), so don't
			 * overwrite that with EINTR !
			 * In the other cases, do_signal() doesn't touch 
			 * R3, so it's still set to -EINTR (see above).
			 */
			return regs->gpr[3];
	}
}


/*
 * Set up a rt signal frame.
 */
static void
setup_rt_frame32(struct pt_regs *regs, int sig, struct k_sigaction *ka,
		 struct sigregs32 *frame, unsigned int newsp)
{
	struct rt_sigframe_32 * rt_sf = (struct rt_sigframe_32 *) (u64)newsp;
	int i;
  
	if (verify_area(VERIFY_WRITE, frame, sizeof(*frame)))
		goto badframe;
	if (regs->msr & MSR_FP)
		giveup_fpu(current);

	/*
	 * Copy the register contents for the pt_regs structure on the
	 *   kernel stack to the elf_gregset_t32 structure on the user
	 *   stack. This is a copy of 64 bit register values to 32 bit
	 *   register values. The high order 32 bits of the 64 bit
	 *   registers are not needed since a 32 bit application is
	 *   running and the saved registers are the contents of the
	 *   user registers at the time of a system call.
	 *
	 * The values saved on the user stack will be restored into
	 *  the registers during the signal return processing.
	 *
	 * Note the +1 is needed in order to get the lower 32 bits
	 * of 64 bit register
	 */
	for (i = 0; i < sizeof(struct pt_regs32)/sizeof(u32); i++) {
		if (__copy_to_user(&frame->gp_regs[i], (u32*)(&regs->gpr[i])+1, sizeof(u32)))
			goto badframe;
	}


	/*
	 * Now copy the floating point registers onto the user stack
	 *
	 * Also set up so on the completion of the signal handler, the
	 * sys_sigreturn will get control to reset the stack
	 */
	if (__copy_to_user(&frame->fp_regs, current->thread.fpr,
			   ELF_NFPREG * sizeof(double))
	    || __put_user(0x38000000U + __NR_rt_sigreturn, &frame->tramp[0])    /* li r0, __NR_rt_sigreturn */
	    || __put_user(0x44000002U, &frame->tramp[1]))   /* sc */
		goto badframe;

	flush_icache_range((unsigned long) &frame->tramp[0],
			   (unsigned long) &frame->tramp[2]);
	current->thread.fpscr = 0;	/* turn off all fp exceptions */
  
	/*
	 * Set up registers for signal handler
	 */
	newsp -= __SIGNAL_FRAMESIZE32;

	regs->gpr[1] = newsp;
	regs->gpr[3] = sig;
	regs->gpr[4] = (unsigned long) &rt_sf->info;
	regs->gpr[5] = (unsigned long) &rt_sf->uc;
	regs->gpr[6] = (unsigned long) rt_sf;
	regs->nip    = (unsigned long) ka->sa.sa_handler;
	regs->link   = (unsigned long) frame->tramp;

	if (put_user((u32)(regs->gpr[1]), (unsigned int *)(u64)newsp))
		goto badframe;

	return;

 badframe:
#if DEBUG_SIG
	printk("badframe in setup_rt_frame32, regs=%p frame=%p newsp=%lx\n",
	       regs, frame, newsp);
#endif
	if (sig == SIGSEGV)
		ka->sa.sa_handler = SIG_DFL;
	force_sig(SIGSEGV, current);
}


/*
 * OK, we're invoking a handler
 */
static void
handle_signal32(unsigned long sig, struct k_sigaction *ka,
	      siginfo_t *info, sigset_t *oldset, struct pt_regs * regs,
	      unsigned int *newspp, unsigned int frame)
{
	struct sigcontext32 *sc;
	struct rt_sigframe_32 *rt_stack_frame;
	siginfo_t32 siginfo32bit;

	if (regs->trap == 0x0C00 /* System Call! */
	    && ((int)regs->result == -ERESTARTNOHAND ||
		((int)regs->result == -ERESTARTSYS &&
		 !(ka->sa.sa_flags & SA_RESTART))))
		regs->result = -EINTR;

	/* Set up the signal frame             */
	/*   Determine if an real time frame - siginfo required   */
	if (ka->sa.sa_flags & SA_SIGINFO)
	{
		siginfo64to32(&siginfo32bit,info);
		/* The ABI requires quadword alignment for the stack. */
		*newspp = (*newspp - sizeof(*rt_stack_frame)) & -16ul;
		rt_stack_frame = (struct rt_sigframe_32 *) (u64)(*newspp) ;
    
		if (verify_area(VERIFY_WRITE, rt_stack_frame, sizeof(*rt_stack_frame)))
		{
			goto badframe;
		}
		if (__put_user((u32)(u64)ka->sa.sa_handler, &rt_stack_frame->uc.uc_mcontext.handler)
		    || __put_user((u32)(u64)&rt_stack_frame->info, &rt_stack_frame->pinfo)
		    || __put_user((u32)(u64)&rt_stack_frame->uc, &rt_stack_frame->puc)
		    /*  put the siginfo on the user stack                    */
		    || __copy_to_user(&rt_stack_frame->info,&siginfo32bit,sizeof(siginfo32bit))
		    /*  set the ucontext on the user stack                   */ 
		    || __put_user(0,&rt_stack_frame->uc.uc_flags)
		    || __put_user(0,&rt_stack_frame->uc.uc_link)
		    || __put_user(current->sas_ss_sp, &rt_stack_frame->uc.uc_stack.ss_sp)
		    || __put_user(sas_ss_flags(regs->gpr[1]),
				  &rt_stack_frame->uc.uc_stack.ss_flags)
		    || __put_user(current->sas_ss_size, &rt_stack_frame->uc.uc_stack.ss_size)
		    || __copy_to_user(&rt_stack_frame->uc.uc_sigmask, oldset,sizeof(*oldset))
		    /* point the mcontext.regs to the pramble register frame  */
		    || __put_user(frame, &rt_stack_frame->uc.uc_mcontext.regs)
		    || __put_user(sig,&rt_stack_frame->uc.uc_mcontext.signal))
		{
			goto badframe; 
		}
	} else {
		/* Put a sigcontext on the stack */
		*newspp = (*newspp - sizeof(*sc)) & -16ul;
		sc = (struct sigcontext32 *)(u64)*newspp;
		if (verify_area(VERIFY_WRITE, sc, sizeof(*sc)))
			goto badframe;
		/*
		 * Note the upper 32 bits of the signal mask are stored
		 * in the unused part of the signal stack frame
		 */
		if (__put_user((u32)(u64)ka->sa.sa_handler, &sc->handler)
		    || __put_user(oldset->sig[0], &sc->oldmask)
		    || __put_user((oldset->sig[0] >> 32), &sc->_unused[3])
		    || __put_user((unsigned int)frame, &sc->regs)
		    || __put_user(sig, &sc->signal))
			goto badframe;
	}

	if (ka->sa.sa_flags & SA_ONESHOT)
		ka->sa.sa_handler = SIG_DFL;

	if (!(ka->sa.sa_flags & SA_NODEFER)) {
		spin_lock_irq(&current->sigmask_lock);
		sigorsets(&current->blocked,&current->blocked,&ka->sa.sa_mask);
		sigaddset(&current->blocked,sig);
		recalc_sigpending(current);
		spin_unlock_irq(&current->sigmask_lock);
	}
	
	return;

badframe:
#if DEBUG_SIG
	printk("badframe in handle_signal32, regs=%p frame=%lx newsp=%lx\n",
	       regs, frame, *newspp);
	printk("sc=%p sig=%d ka=%p info=%p oldset=%p\n", sc, sig, ka, info, oldset);
#endif
	if (sig == SIGSEGV)
		ka->sa.sa_handler = SIG_DFL;
	force_sig(SIGSEGV, current);
}


/*
 *  Start Alternate signal stack support
 *
 *  System Calls
 *       sigaltatck               sys32_sigaltstack
 */


asmlinkage int sys32_sigaltstack(u32 newstack, u32 oldstack, int p3, int p4, int p6,
				 int p7, struct pt_regs *regs)
{
	stack_t uss, uoss;
	int ret;
	mm_segment_t old_fs;
	unsigned long sp;
	u32 sssp;
	stack_32_t *ustack;

	/*
	 * set sp to the user stack on entry to the system call
	 * the system call router sets R9 to the saved registers
	 */
	sp = regs->gpr[1];

	/*  Put new stack info in local 64 bit stack struct */ 
	ustack = (stack_32_t *)(long) newstack;
	if (newstack && (get_user(sssp, &ustack->ss_sp) ||
			 __get_user(uss.ss_flags, &ustack->ss_flags) ||
			 __get_user(uss.ss_size, &ustack->ss_size)))
		return -EFAULT; 
	uss.ss_sp = (void *)(long) sssp;
   
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = do_sigaltstack(newstack ? &uss : NULL, oldstack ? &uoss : NULL, sp);
	set_fs(old_fs);
	if (ret)
		return ret;

	/* Copy the stack information to the user output buffer */
	ustack = (stack_32_t *)(long) oldstack;
	if (oldstack && (put_user((long)uoss.ss_sp, &ustack->ss_sp) ||
			 __put_user(uoss.ss_flags, &ustack->ss_flags) ||
			 __put_user(uoss.ss_size, &ustack->ss_size)))
		return -EFAULT;
	return ret;
}



/*
 *  Start of do_signal32 routine
 *
 *   This routine gets control when a pending signal needs to be processed
 *     in the 32 bit target thread -
 *
 *   It handles both rt and non-rt signals
 */

/*
 * Note that 'init' is a special process: it doesn't get signals it doesn't
 * want to handle. Thus you cannot kill init even with a SIGKILL even by
 * mistake.
 */

int do_signal32(sigset_t *oldset, struct pt_regs *regs)
{
	siginfo_t info;
	struct k_sigaction *ka;
	unsigned int frame, newsp;
	unsigned long signr = 0;

	if (!oldset)
		oldset = &current->blocked;

	newsp = frame = 0;

	for (;;) {
		spin_lock_irq(&current->sigmask_lock);
		signr = dequeue_signal(&current->blocked, &info);
		spin_unlock_irq(&current->sigmask_lock);

		if (!signr)
			break;

		if ((current->ptrace & PT_PTRACED) && signr != SIGKILL) {
			/* Let the debugger run.  */
			current->exit_code = signr;
			current->state = TASK_STOPPED;
			notify_parent(current, SIGCHLD);
			schedule();

			/* We're back.  Did the debugger cancel the sig?  */
			if (!(signr = current->exit_code))
				continue;
			current->exit_code = 0;

			/* The debugger continued.  Ignore SIGSTOP.  */
			if (signr == SIGSTOP)
				continue;

			/* Update the siginfo structure.  Is this good?  */
			if (signr != info.si_signo) {
				info.si_signo = signr;
				info.si_errno = 0;
				info.si_code = SI_USER;
				info.si_pid = current->p_pptr->pid;
				info.si_uid = current->p_pptr->uid;
			}

			/* If the (new) signal is now blocked, requeue it.  */
			if (sigismember(&current->blocked, signr)) {
				send_sig_info(signr, &info, current);
				continue;
			}
		}

		ka = &current->sig->action[signr-1];

		if (ka->sa.sa_handler == SIG_IGN) {
			if (signr != SIGCHLD)
				continue;
			/* Check for SIGCHLD: it's special.  */
			while (sys_wait4(-1, NULL, WNOHANG, NULL) > 0)
				/* nothing */;
			continue;
		}

		if (ka->sa.sa_handler == SIG_DFL) {
			int exit_code = signr;

			/* Init gets no signals it doesn't want.  */
			if (current->pid == 1)
				continue;

			switch (signr) {
			case SIGCONT: case SIGCHLD: case SIGWINCH: case SIGURG:
				continue;

			case SIGTSTP: case SIGTTIN: case SIGTTOU:
				if (is_orphaned_pgrp(current->pgrp))
					continue;
				/* FALLTHRU */

			case SIGSTOP:
				current->state = TASK_STOPPED;
				current->exit_code = signr;
				if (!(current->p_pptr->sig->action[SIGCHLD-1].sa.sa_flags & SA_NOCLDSTOP))
					notify_parent(current, SIGCHLD);
				schedule();
				continue;

			case SIGQUIT: case SIGILL: case SIGTRAP:
			case SIGABRT: case SIGFPE: case SIGSEGV:
			case SIGBUS: case SIGSYS: case SIGXCPU: case SIGXFSZ:
				if (do_coredump(signr, regs))
					exit_code |= 0x80;
				/* FALLTHRU */

			default:
				sig_exit(signr, exit_code, &info);
				/* NOTREACHED */
			}
		}

		if ( (ka->sa.sa_flags & SA_ONSTACK)
		     && (! on_sig_stack(regs->gpr[1])))
			newsp = (current->sas_ss_sp + current->sas_ss_size);
		else
			newsp = regs->gpr[1];
		/* The ABI requires quadword alignment for the stack. */
		newsp = frame = (newsp - sizeof(struct sigregs32)) & -16ul;

		/* Whee!  Actually deliver the signal.  */
		handle_signal32(signr, ka, &info, oldset, regs, &newsp, frame);
		break;
	}

	if (regs->trap == 0x0C00 /* System Call! */ &&
	    ((int)regs->result == -ERESTARTNOHAND ||
	     (int)regs->result == -ERESTARTSYS ||
	     (int)regs->result == -ERESTARTNOINTR)) {
		regs->gpr[3] = regs->orig_gpr3;
		regs->nip -= 4;		/* Back up & retry system call */
		regs->result = 0;
	}

	if (newsp == frame)
		return 0;		/* no signals delivered */

	/* Invoke correct stack setup routine */
	if (ka->sa.sa_flags & SA_SIGINFO) 
		setup_rt_frame32(regs, signr, ka,
				 (struct sigregs32*)(u64)frame, newsp);
	else
		setup_frame32(regs, signr, ka,
			      (struct sigregs32*)(u64)frame, newsp);
	return 1;
}
