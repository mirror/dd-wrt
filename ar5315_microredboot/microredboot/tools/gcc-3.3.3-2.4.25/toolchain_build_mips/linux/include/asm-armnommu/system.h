#ifndef __ASM_ARM_SYSTEM_H
#define __ASM_ARM_SYSTEM_H

#ifdef __KERNEL__

#include <linux/config.h>
#include <linux/kernel.h>

/* information about the system we're running on */
extern unsigned int system_rev;
extern unsigned int system_serial_low;
extern unsigned int system_serial_high;
extern unsigned int mem_fclk_21285;

/*
 * This tells us if we have an ISA bridge
 * present in a PCI system.
 */
#ifdef CONFIG_PCI
extern int have_isa_bridge;
#else
#define have_isa_bridge		(0)
#endif

#include <asm/proc-fns.h>

#define xchg(ptr,x) \
	((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))

#define tas(ptr) (xchg((ptr),1))

extern asmlinkage void __backtrace(void);

/*
 * Include processor dependent parts
 */
#include <asm/proc/system.h>

#define mb() __asm__ __volatile__ ("" : : : "memory")
#define rmb() mb()
#define wmb() mb()
#define set_mb(var, value)  do { var = value; mb(); } while (0)
#define set_wmb(var, value) do { var = value; wmb(); } while (0)
#define nop() __asm__ __volatile__("mov\tr0,r0\t@ nop\n\t");

#define prepare_to_switch()    do { } while(0)

/*
 * switch_to(prev, next) should switch from task `prev' to `next'
 * `prev' will never be the same as `next'.
 * The `mb' is to tell GCC not to cache `current' across this call.
 */
extern struct task_struct *__switch_to(struct task_struct *prev, struct task_struct *next);

#define switch_to(prev,next,last)		\
	do {			 		\
		last = __switch_to(prev,next);	\
		mb();				\
	} while (0)

/* For spinlocks etc */
#define local_irq_save(x)	__save_flags_cli(x)
#define local_irq_set(x)	__save_flags_sti(x)
#define local_irq_restore(x)	__restore_flags(x)
#define local_irq_disable()	__cli()
#define local_irq_enable()	__sti()

#ifdef CONFIG_SMP
#error SMP not supported

#define smp_mb()		mb()
#define smp_rmb()		rmb()
#define smp_wmb()		wmb()

#else

#define smp_mb()		barrier()
#define smp_rmb()		barrier()
#define smp_wmb()		barrier()

#define cli()			__cli()
#define sti()			__sti()
#define clf()			__clf()
#define stf()			__stf()
#define save_flags(x)		__save_flags(x)
#define restore_flags(x)	__restore_flags(x)
#define save_flags_cli(x)	__save_flags_cli(x)
#define save_and_cli(x)		__save_flags_cli(x)
#define save_and_set(x)		__save_flags_sti(x)

#endif /* CONFIG_SMP */

#endif /* __KERNEL__ */

#endif
