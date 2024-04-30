/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 95, 96, 97, 98, 99, 2003, 06 by Ralf Baechle
 * Copyright (C) 1996 by Paul M. Antoine
 * Copyright (C) 1999 Silicon Graphics
 * Kevin D. Kissell, kevink@mips.org and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000 MIPS Technologies, Inc.
 */
#ifndef _ASM_SYSTEM_H
#define _ASM_SYSTEM_H

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/irqflags.h>

#include <asm/addrspace.h>
#include <asm/barrier.h>
#include <asm/cmpxchg.h>
#include <asm/cpu-features.h>
#include <asm/dsp.h>
#include <asm/watch.h>
#include <asm/war.h>
#include <asm/compiler.h>


/*
 * switch_to(n) should switch tasks to task nr n, first
 * checking that n isn't the current task, in which case it does nothing.
 */
extern asmlinkage void *resume(void *last, void *next, void *next_ti);

struct task_struct;

extern unsigned int ll_bit;
extern struct task_struct *ll_task;

#ifdef CONFIG_MIPS_MT_FPAFF

/*
 * Handle the scheduler resume end of FPU affinity management.  We do this
 * inline to try to keep the overhead down. If we have been forced to run on
 * a "CPU" with an FPU because of a previous high level of FP computation,
 * but did not actually use the FPU during the most recent time-slice (CU1
 * isn't set), we undo the restriction on cpus_allowed.
 *
 * We're not calling set_cpus_allowed() here, because we have no need to
 * force prompt migration - we're already switching the current CPU to a
 * different thread.
 */

#define __mips_mt_fpaff_switch_to(prev)					\
do {									\
	struct thread_info *__prev_ti = task_thread_info(prev);		\
									\
	if (cpu_has_fpu &&						\
	    test_ti_thread_flag(__prev_ti, TIF_FPUBOUND) &&		\
	    (!(KSTK_STATUS(prev) & ST0_CU1))) {				\
		clear_ti_thread_flag(__prev_ti, TIF_FPUBOUND);		\
		prev->cpus_allowed = prev->thread.user_cpus_allowed;	\
	}								\
	next->thread.emulated_fp = 0;					\
} while(0)

#else
#define __mips_mt_fpaff_switch_to(prev) do { (void) (prev); } while (0)
#endif

#define __clear_software_ll_bit()					\
do {									\
	if (!__builtin_constant_p(cpu_has_llsc) || !cpu_has_llsc)	\
		ll_bit = 0;						\
} while (0)

#define switch_to(prev, next, last)					\
do {									\
	__mips_mt_fpaff_switch_to(prev);				\
	if (cpu_has_dsp)						\
		__save_dsp(prev);					\
	__clear_software_ll_bit();					\
	(last) = resume(prev, next, task_thread_info(next));		\
} while (0)

#define finish_arch_switch(prev)					\
do {									\
	if (cpu_has_dsp)						\
		__restore_dsp(current);					\
	if (cpu_has_userlocal)						\
		write_c0_userlocal(current_thread_info()->tp_value);	\
	__restore_watch();						\
} while (0)

#if R10000_LLSC_WAR
# define __scbeqz "beqzl"
#else
# define __scbeqz "beqz"
#endif

extern unsigned long __xchg_called_with_bad_pointer(void)
	__compiletime_error("Bad argument size for xchg");


#define __xchg_asm(ld, st, m, val)					\
({									\
	__typeof(*(m)) __ret;						\
									\
	if (kernel_uses_llsc) {						\
		__asm__ __volatile__(					\
		"	.set	push				\n"	\
		"	.set	noat				\n"	\
		"	.set	" MIPS_ISA_ARCH_LEVEL "		\n"	\
		"1:	" ld "	%0, %2		# __xchg_asm	\n"	\
		"	.set	mips0				\n"	\
		"	move	$1, %z3				\n"	\
		"	.set	" MIPS_ISA_ARCH_LEVEL "		\n"	\
		"	" st "	$1, %1				\n"	\
		"\t" __scbeqz "	$1, 1b				\n"	\
		"	.set	pop				\n"	\
		: "=&r" (__ret), "=" GCC_OFF12_ASM() (*m)		\
		: GCC_OFF12_ASM() (*m), "Jr" (val)			\
		: "memory");						\
	} else {							\
		unsigned long __flags;					\
									\
		raw_local_irq_save(__flags);				\
		__ret = *m;						\
		*m = val;						\
		raw_local_irq_restore(__flags);				\
	}								\
									\
	__ret;								\
})

extern unsigned long __xchg_small(volatile void *ptr, unsigned long val,
				  unsigned int size);

static __always_inline
unsigned long __xchg(volatile void *ptr, unsigned long x, int size)
{
	switch (size) {
	case 1:
	case 2:
		return __xchg_small(ptr, x, size);

	case 4:
		return __xchg_asm("ll", "sc", (volatile u32 *)ptr, x);

	case 8:
		if (!IS_ENABLED(CONFIG_64BIT))
			return __xchg_called_with_bad_pointer();

		return __xchg_asm("lld", "scd", (volatile u64 *)ptr, x);

	default:
		return __xchg_called_with_bad_pointer();
	}
}

#define xchg(ptr, x)							\
({									\
	__typeof__(*(ptr)) __res;					\
									\
	smp_mb__before_llsc();						\
									\
	__res = (__typeof__(*(ptr)))					\
		__xchg((ptr), (unsigned long)(x), sizeof(*(ptr)));	\
									\
	smp_llsc_mb();							\
									\
	__res;								\
})

extern void set_handler(unsigned long offset, void *addr, unsigned long len);
extern void set_uncached_handler(unsigned long offset, void *addr, unsigned long len);

typedef void (*vi_handler_t)(void);
extern void *set_vi_handler(int n, vi_handler_t addr);

extern void *set_except_vector(int n, void *addr);
extern unsigned long ebase;
extern void per_cpu_trap_init(void);

/*
 * See include/asm-ia64/system.h; prevents deadlock on SMP
 * systems.
 */
#define __ARCH_WANT_UNLOCKED_CTXSW

extern unsigned long arch_align_stack(unsigned long sp);

#endif /* _ASM_SYSTEM_H */
