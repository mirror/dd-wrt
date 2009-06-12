/*
 * include/asm-v850/processor.h
 *
 *  Copyright (C) 2001,02,03  NEC Electronics Corporation
 *  Copyright (C) 2001,02,03  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#ifndef __V850_PROCESSOR_H__
#define __V850_PROCESSOR_H__

#include <linux/config.h>

#include <asm/ptrace.h>
#include <asm/current.h>
#include <asm/entry.h>

/* Some code expects `segment' stuff to be defined here.  */
#include <asm/segment.h>


/*
 * The only places this is used seem to be horrible bletcherous kludges,
 * so we just define it to be as large as possible.
 */
#define TASK_SIZE	(0xFFFFFFFF)


#define THREAD_SIZE	(2*PAGE_SIZE)


#ifndef __ASSEMBLY__


/*
 * Default implementation of macro that returns current
 * instruction pointer ("program counter").
 */
#define current_text_addr()	({ __label__ _l; _l: &&_l;})


/*
 * Bus types
 */
#define EISA_bus 0
#define EISA_bus__is_a_macro /* for versions in ksyms.c */
#define MCA_bus 0
#define MCA_bus__is_a_macro /* for versions in ksyms.c */

/* If you change this, you must change the associated assembly-languages
   constants defined below, THREAD_*.  */
struct thread_struct {
	/* kernel stack pointer (must be first field in structure) */
	unsigned long  ksp;
};

#define INIT_THREAD { sizeof init_stack + (unsigned long)init_stack }


/* Do necessary setup to start up a newly executed thread.  */
extern inline void start_thread (struct pt_regs *regs,
				 unsigned long pc, unsigned long usp)
{
	regs->pc = pc;
	regs->gpr[GPR_SP] = usp;
	regs->kernel_mode = 0;
}

/* Free all resources held by a thread. */
extern inline void release_thread (struct task_struct *dead_task)
{
}

extern int arch_kernel_thread (int (*fn)(void *), void * arg, unsigned long flags);

/* Free current thread data structures etc.  */
static inline void exit_thread (void)
{
}


/* Return the registers saved during context-switch by the currently
   not-running thread T.  Note that this only includes some registers!
   See entry.S for details.  */
#define thread_saved_regs(t) ((struct pt_regs*)((t)->ksp + STATE_SAVE_PT_OFFSET))
/* Return saved (kernel) PC of a blocked thread.  Actually, we return the
   LP register, because the thread is actually blocked in switch_thread,
   and we're interested in the PC it will _return_ to.  */
#define thread_saved_pc(t)   (thread_saved_regs(t)->gpr[GPR_LP])


unsigned long get_wchan (struct task_struct *p);


/* Return some info about the user process TASK.  */
#define task_tos(task)	((unsigned long)(task) + KERNEL_STACK_SIZE)
#define task_regs(task) ((struct pt_regs *)task_tos (task) - 1)
#define task_sp(task)	(task_regs (task)->gpr[GPR_SP])
#define task_pc(task)	(task_regs (task)->pc)
/* Grotty old names for some.  */
#define KSTK_EIP(task)	task_pc (task)
#define KSTK_ESP(task)	task_sp (task)


#define cpu_relax()    ((void)0)


/* Allocation and freeing of basic task resources. */
#define alloc_task_struct() \
   ((struct task_struct *)__get_free_pages(GFP_KERNEL,1))
#define free_task_struct(p)	free_pages ((unsigned long)(p),1)
#define get_task_struct(tsk)	atomic_inc (&mem_map[MAP_NR(tsk)].count)

#define init_task	(init_task_union.task)
#define init_stack	(init_task_union.stack)


#else /* __ASSEMBLY__ */

#define THREAD_KSP	0

#endif /* !__ASSEMBLY__ */


#endif /* __V850_PROCESSOR_H__ */
