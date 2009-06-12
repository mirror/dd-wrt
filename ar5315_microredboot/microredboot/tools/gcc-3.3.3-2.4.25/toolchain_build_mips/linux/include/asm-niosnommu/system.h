/*
 * Copyright (C) 2001		Ken Hill
 *				Adapted from Keith Adams i960 file and Sparcnommu
 *
 * Copyright (C) 1999		Keith Adams	<kma@cse.ogi.edu>
 * 				Oregon Graduate Institute
 */
#ifndef _NIOS_SYSTEM_H
#define _NIOS_SYSTEM_H

#include <linux/linkage.h>

asmlinkage void resume(void);
struct task_struct;

/* When a context switch happens we must flush all user windows so that
 * the windows of the current process are flushed onto its stack. This
 * way the windows are all clean for the next process and the stack
 * frames are up to date.
 */
extern void flush_user_windows(void);
extern void synchronize_user_stack(void);
extern void nios_switch_to(void *old_task, void *new_task);
#define prepare_to_switch()	do { } while(0)

#define switch_to(prev,next,last) do {\
			nios_switch_to(prev, next); \
			} while(0)

#define HARD_RESET_NOW() hard_reset_now();

#define xchg(ptr,x) ((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))
#define tas(ptr) (xchg((ptr),1))

/* Changing the IRQ level on the NIOS. */
extern inline void setipl(int __new_ipl)
{
	__asm__ __volatile__("rdctl  %%g1\n\t"
			     "pfx %%hi(%1)\n\t"
			     "and %%g1, %%lo(%1)\n\t"
			     "pfx %%hi(%2)\n\t"
			     "and %0, %%lo(%2)\n\t"
			     "or %%g1, %0\n\t"
			     "wrctl %%g1\n\t"
			     "nop\n\t"
			     "nop\n\t"
			     : /* No output */
			     : "r" (__new_ipl<<9), "i" (~0x7E00), "i" (0x7E00)
			     : "g1");
}

extern inline int getipl(void)
{
	int retval;

	__asm__ __volatile__("rdctl %0\n\t"
			     "pfx %%hi(%1)\n\t"
			     "and %0, %%lo(%1)\n\t"
			     "lsri %0, 9\n\t"
			     : "=r" (retval)
			     : "i" (0x7E00));
	return retval;
}

extern inline int swpipl(int __new_ipl)
{
	int retval;

	__asm__ __volatile__("rdctl  %0\n\t"
			     "mov %%g1,%0\n\t"
			     "pfx %%hi(%2)\n\t"
			     "and %%g1, %%lo(%2)\n\t"
			     "pfx %%hi(%2)\n\t"
			     "and %1, %%lo(%2)\n\t"
			     "or %%g1, %1\n\t"
			     "wrctl %%g1\n\t"
			     "nop\n\t"
			     "nop\n\t"
			     "pfx %%hi(%3)\n\t"
			     "and %0, %%lo(%3)\n\t"
			     "lsri %0, 9\n\t"
			     "nop\n\t"
			     : "=&r" (retval)
			     : "r" (__new_ipl<<9), "i" (~0x7E00), "i" (0x7E00)
			     : "g1");
	return retval;
}

#define __cli()			setipl(3)    /* 3 = no int's except reg window over/underflow */
#define __sti()			setipl(63)   /* This will allow lower priority interrupts
					        to occur that should not if IE bit could have
					        been used. */
#define __save_flags(flags)	do { flags = getipl(); } while (0)
#define __restore_flags(flags)	setipl(flags)

#define nop() __asm__ __volatile__ ("nop")

#define DEBUG_TRAP5      0x7905  /* Nios Trap 5 instruction */
#define NOP              0x3000  /* Nios nop instruction    */

/* For spinlocks etc */
#define local_irq_save(x)	do { __save_flags(x); __cli(); } while (0)
#define local_irq_set(x)	do { __save_flags(x); __sti(); } while (0)
#define local_irq_restore(x)	do { __restore_flags(x); } while (0)
#define local_irq_disable()	__cli()
#define local_irq_enable()	__sti()

#define cli()			__cli()
#define sti()			__sti()
#define save_flags(x)		__save_flags(x)
#define restore_flags(x)	__restore_flags(x)
#define save_and_cli(x)	__save_and_cli(x)
#define save_and_set(x)	__save_and_sti(x)

/* These definitions for en/disabling interrupts can only be used when there
 * is no chance that the body of code between disable and enable does a call
 * or save/restore instruction.  Primarily for use in the definition of
 * atomic operations like xchg, atomic_add, etc.
 */
extern inline void _disable_interrupts(void)
{
	asm("pfx 8\n\t"
		 "wrctl %g0\n\t"
		 "nop\n\t"
		 "nop");
}

extern inline void _enable_interrupts(void)
{
	asm("pfx 9\n\t"
		 "wrctl %g0");
}


static inline unsigned long __xchg(unsigned long x, 
				   volatile void * ptr, int size)
{
	unsigned long retval;

	_disable_interrupts();

	switch (size) {
		case 1:
			__asm__ __volatile__ ("ld	%0,[%1]\n\t"
					      "ext8d	%0,%1\n\t"
					      "fill8	%%r0,%2\n\t"
					      "st8d	[%1],%%r0\n\t"
					      : "=&r" (retval)
					      : "r" (ptr), "r" (x)
					      : "r0"); /* Same as %r0 */
			break;

		case 2: /* Alignment for data types is strict therefore
			   do not need to account for unaligned access */
			__asm__ __volatile__ ("ld	%0,[%1]\n\t"
					      "ext16d	%0,%1\n\t"
					      "fill16	%%r0,%2\n\t"
					      "st16d	[%1],%%r0\n\t"
					      : "=&r" (retval)
					      : "r" (ptr), "r" (x)
					      : "r0"); /* Same as %r0 */
			break;

		case 4: /* Alignment for data types is strict therefore
			   do not need to account for unaligned access */
			__asm__ __volatile__ ("ld	%0,[%1]\n\t"
					      "st	[%1],%2\n\t"
					      : "=&r" (retval)
					      : "r" (ptr), "r" (x));
			break;
	}

	_enable_interrupts();
	return retval;
}

/* Get the value of HI_LIMIT from wvalid control register */
extern inline int get_hi_limit(void)
{
	int retval;
	__asm__ __volatile__(
			"pfx 2\n\t"
			"rdctl %0\n\t"
			"lsri %0, 5\n\t"
			"pfx %%hi(%1)\n\t"
			"and %0, %%lo(%1)\n\t"
			: "=r" (retval)
			: "i" (0x001f));
	return retval;
}

/***********************************************/
/* hjz: Memory-Barrier definition. If the Nios */
/* ever supports MP, this function would have  */
/* to flush the write cache and invalidate the */
/* read cache so all processors see the same   */
/* values in memory. But not today...          */
/***********************************************/
#define mb()	__asm__ __volatile__ ("" : : : "memory")
#define rmb()	__asm__ __volatile__ ("" : : : "memory")
#define wmb()	__asm__ __volatile__ ("" : : : "memory")

#ifdef CONFIG_SMP
#define smp_mb()	mb()
#define smp_rmb()	rmb()
#define smp_wmb()	wmb()
#else
#define smp_mb()	barrier()
#define smp_rmb()	barrier()
#define smp_wmb()	barrier()
#endif

#endif /* _NIOS_SYSTEM_H */

