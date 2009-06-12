/*
 * include/asm-h8300/processor.h
 *
 * Copyright (C) 2002 Yoshinori Sato
 *
 * Based on: linux/asm-m68nommu/processor.h
 *
 * Copyright (C) 1995 Hamish Macdonald
 */

#ifndef __ASM_H8300_PROCESSOR_H
#define __ASM_H8300_PROCESSOR_H

/*
 * Default implementation of macro that returns current
 * instruction pointer ("program counter").
 */
#define current_text_addr() ({ __label__ _l; _l: &&_l;})

#include <linux/config.h>
#include <asm/segment.h>
#include <asm/fpu.h>
#include <asm/ptrace.h>
#include <asm/current.h>

extern inline unsigned long rdusp(void) {
	extern unsigned int	sw_usp;
	return(sw_usp);
}

extern inline void wrusp(unsigned long usp) {
	extern unsigned int	sw_usp;
	sw_usp = usp;
}

/*
 * User space process size: 3.75GB. This is hardcoded into a few places,
 * so don't change it unless you know what you are doing.
 */
#define TASK_SIZE	(0xFFFFFFFFUL)

/*
 * Bus types
 */
#define EISA_bus 0
#define MCA_bus 0

struct thread_struct {
	unsigned long  ksp;		/* kernel stack pointer */
	unsigned long  usp;		/* user stack pointer */
	unsigned long  ccr;		/* saved status register */
	unsigned long  esp0;            /* points to SR of stack frame */
	unsigned long  debugreg[8];
};

#define INIT_THREAD  { \
	sizeof(init_stack) + (unsigned long) init_stack, 0, \
	PS_S,  0, {0, 0}, \
}

/*
 * Do necessary setup to start up a newly executed thread.
 *
 * pass the data segment into user programs if it exists,
 * it can't hurt anything as far as I can tell
 */
#if defined(__H8300H__)
#define start_thread(_regs, _pc, _usp)			        \
do {							        \
	(_regs)->pc = (_pc);				        \
	(_regs)->ccr = 0;				        \
	wrusp((unsigned long)(_usp) - sizeof(unsigned long)*3);	\
} while(0)
#endif
#if defined(__H8300S__)
#define start_thread(_regs, _pc, _usp)			  \
do {                                              \
	set_fs(USER_DS); /* reads from user space */  \
	(_regs)->pc = (_pc);                          \
	(_regs)->ccr = 0;                             \
	(_regs)->exr = 0x00;                          \
	/* 14 = space for retaddr(4), vector(4), er0(4) and ext(2) on stack */ \
	wrusp(((unsigned long)(_usp)) - 14);          \
} while(0)
#endif

/* Forward declaration, a strange C thing */
struct task_struct;

/* Free all resources held by a thread. */
static inline void release_thread(struct task_struct *dead_task)
{
}

extern int arch_kernel_thread(int (*fn)(void *), void * arg, unsigned long flags);

#define copy_segments(tsk, mm)		do { } while (0)
#define release_segments(mm)		do { } while (0)
#define forget_segments()		do { } while (0)

/*
 * Free current thread data structures etc..
 */
static inline void exit_thread(void)
{
}

/*
 * Return saved PC of a blocked thread.
 */
extern inline unsigned long thread_saved_pc(struct thread_struct *t)
{
	extern void scheduling_functions_start_here(void);
	extern void scheduling_functions_end_here(void);
	struct switch_stack *sw = (struct switch_stack *)t->ksp;
	/* Check whether the thread is blocked in resume() */
	if (sw->retpc > (unsigned long)scheduling_functions_start_here &&
	    sw->retpc < (unsigned long)scheduling_functions_end_here)
		return ((unsigned long *)sw->er6)[1];
	else
		return sw->retpc;
}

unsigned long get_wchan(struct task_struct *p);

#define	KSTK_EIP(tsk)	\
    ({			\
	unsigned long eip = 0;	 \
	if ((tsk)->thread.esp0 > PAGE_SIZE && \
	    MAP_NR((tsk)->thread.esp0) < max_mapnr) \
	      eip = ((struct pt_regs *) (tsk)->thread.esp0)->pc; \
	eip; })
#define	KSTK_ESP(tsk)	((tsk) == current ? rdusp() : (tsk)->thread.usp)

#ifdef CONFIG_SMALL_TASKS
#define THREAD_SIZE PAGE_SIZE

/* Allocation and freeing of basic task resources. */
#define alloc_task_struct() \
	((struct task_struct *) __get_free_pages(GFP_KERNEL,0))
#define free_task_struct(p)	free_pages((unsigned long)(p),0)
#else
#define THREAD_SIZE (2*PAGE_SIZE)

/* Allocation and freeing of basic task resources. */
#define alloc_task_struct() \
	((struct task_struct *) __get_free_pages(GFP_KERNEL,1))
#define free_task_struct(p)	free_pages((unsigned long)(p),1)
#endif
#define get_task_struct(tsk)      atomic_inc(&mem_map[MAP_NR(tsk)].count)

#define init_task	(init_task_union.task)
#define init_stack	(init_task_union.stack)

#define cpu_relax()    barrier()

#endif
