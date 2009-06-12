//vic - temp define task_size
/*
 * include/asm-niosnommu/processor.h
 *
 * Copyright (C) 2001  Ken Hill (khill@microtronix.com)    
 *                     Vic Phillips (vic@microtronix.com)
 *
 * hacked from:
 *
 * include/asm-sparc/processor.h
 *
 * Copyright (C) 1994 David S. Miller (davem@caip.rutgers.edu)
 */

#ifndef __ASM_NIOS_PROCESSOR_H
#define __ASM_NIOS_PROCESSOR_H

/*
 * Default implementation of macro that returns current
 * instruction pointer ("program counter").
 */
#define current_text_addr() ({ __label__ _l; _l: &&_l;})

#include <linux/a.out.h>

#include <asm/psr.h>
#include <asm/ptrace.h>
#include <asm/signal.h>
#include <asm/segment.h>
#include <asm/current.h>
#include <asm/system.h> /* for get_hi_limit */
/*
 * Bus types
 */
#define EISA_bus 0
#define EISA_bus__is_a_macro /* for versions in ksyms.c */
#define MCA_bus 0
#define MCA_bus__is_a_macro /* for versions in ksyms.c */

/*
 * The nios has no problems with write protection
 */
#define wp_works_ok 1
#define wp_works_ok__is_a_macro /* for versions in ksyms.c */

/* Whee, this is STACK_TOP and the lowest kernel address too... */
#if 0
#define KERNBASE        0x00000000  /* First address the kernel will eventually be */
#define TASK_SIZE	(KERNBASE)
#define MAX_USER_ADDR	TASK_SIZE
#define MMAP_SEARCH_START (TASK_SIZE/3)
#endif

#define TASK_SIZE	0x2000000	//vic for lack of something better ...

/* The Nios processor specific thread struct. */
struct thread_struct {
	unsigned long spare;
	struct pt_regs *kregs;

	/* For signal handling */
	unsigned long sig_address;
	unsigned long sig_desc;

	/* Context switch saved kernel state. */
	unsigned long ksp;
	unsigned long kpc;
	unsigned long kpsr;
	unsigned long kwvalid;

	/* Special child fork kpsr/kwim values. */
	unsigned long fork_kpsr;
	unsigned long fork_kwvalid;

	/* A place to store user windows and stack pointers
	 * when the stack needs inspection.
	 *
	 * KH - I don't think the nios port requires this
	 */
#define NSWINS 1 //vic8
	struct reg_window spare_reg_window[NSWINS];
	unsigned long spare_w_saved;

	/* struct sigstack sstk_info; */

	/* Flags are defined below */

	unsigned long flags;
	int current_ds;
	struct exec core_exec;     /* just what it says. */
};

#define NIOS_FLAG_KTHREAD	0x00000001	/* task is a kernel thread */
#define NIOS_FLAG_COPROC	0x00000002	/* Thread used coprocess */
#define NIOS_FLAG_DEBUG		0x00000004	/* task is being debugged */

#define INIT_MMAP { &init_mm, (0), (0), \
		    __pgprot(0x0) , VM_READ | VM_WRITE | VM_EXEC }

#define INIT_THREAD  { \
/* uwinmask, kregs, sig_address, sig_desc, ksp, kpc, kpsr, kwvalid */ \
   0,        0,     0,           0,        0,   0,   0,    0, \
/* fork_kpsr, fork_kwvalid */ \
   0,         0, \
/* spare_reg_window */  \
{ { { 0, }, { 0, } }, }, \
/* spare_w_saved */ \
   0, \
/* sstk_info */ \
/* flags,              current_ds, */ \
/*   NIOS_FLAG_KTHREAD, USER_DS, */ \
   NIOS_FLAG_KTHREAD, __KERNEL_DS, \
/* core_exec */ \
{ 0, }, \
}

/* Return saved PC of a blocked thread. */
extern inline unsigned long thread_saved_pc(struct thread_struct *t)
{
	return t->kregs->pc;
}

/*
 * Do necessary setup to start up a newly executed thread.
 */
extern inline void start_thread(struct pt_regs * regs, unsigned long pc, unsigned long sp)
{
	unsigned long saved_psr = (get_hi_limit() << 4) | PSR_IPRI | PSR_IE;
	int i;

	for(i = 0; i < 16; i++) regs->u_regs[i] = 0;
	regs->pc = pc >> 1;
	regs->psr = saved_psr;
	regs->u_regs[UREG_FP] = (sp - REGWIN_SZ);
}

extern int kernel_thread(int (*fn)(void *), void * arg, unsigned long flags);
/* Copy and release all segment info associated with a VM */
#define copy_segments(tsk, mm)		do { } while (0)
#define release_segments(mm)		do { } while (0)

unsigned long get_wchan(struct task_struct *p);

#define KSTK_EIP(tsk)  ((tsk)->thread.kregs->pc)
#define KSTK_ESP(tsk)  ((tsk)->thread.kregs->u_regs[UREG_FP])

#ifdef __KERNEL__
/* Allocation and freeing of basic task resources. */
#define alloc_task_struct() \
	((struct task_struct *) __get_free_pages(GFP_KERNEL,1))
#define free_task_struct(p)	free_pages((unsigned long)(p),1)
#define get_task_struct(tsk)      atomic_inc(&mem_map[MAP_NR(tsk)].count)

#define init_task	(init_task_union.task)
#define init_stack	(init_task_union.stack)
#endif

#define cpu_relax()    do { } while (0)
#endif /* __ASM_NIOS_PROCESSOR_H */
