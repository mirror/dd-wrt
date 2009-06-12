#ifndef _H8300_SYSTEM_H
#define _H8300_SYSTEM_H

#include <linux/config.h> /* get configuration macros */
#include <linux/linkage.h>

#define prepare_to_switch()	do { } while(0)

/*
 * switch_to(n) should switch tasks to task ptr, first checking that
 * ptr isn't the current task, in which case it does nothing.  This
 * also clears the TS-flag if the task we switched to has used the
 * math co-processor latest.
 */
/*
 * switch_to() saves the extra registers, that are not saved
 * automatically by SAVE_SWITCH_STACK in resume(), ie. d0-d5 and
 * a0-a1. Some of these are used by schedule() and its predecessors
 * and so we might get see unexpected behaviors when a task returns
 * with unexpected register values.
 *
 * syscall stores these registers itself and none of them are used
 * by syscall after the function in the syscall has been called.
 *
 * Beware that resume now expects *next to be in d1 and the offset of
 * tss to be in a1. This saves a few instructions as we no longer have
 * to push them onto the stack and read them back right after.
 *
 * 02/17/96 - Jes Sorensen (jds@kom.auc.dk)
 *
 * Changed 96/09/19 by Andreas Schwab
 * pass prev in a0, next in a1, offset of tss in d1, and whether
 * the mm structures are shared in d2 (to avoid atc flushing).
 *
 * H8/300 Porting 2002/09/04 Yoshinori Sato
 */
asmlinkage void resume(void);
#define switch_to(prev,next,last) { \
  void *_last;								        \
  __asm__ __volatile__(								\
  			"mov.l	%1, er0\n\t"					\
			"mov.l	%2, er1\n\t"					\
			"jsr @_" SYMBOL_NAME_STR(resume) "\n\t"                 \
			"mov.l	er2, %0\n\t"					\
		       : "=r" (_last)						\
		       : "r" (prev),						\
			 "r" (next)						\
		       : "cc", "er0", "er1", "er2", "er3");	                \
  (last) = _last; 								\
}

#define __sti() asm volatile ("andc #0x7f,ccr")
#define __cli() asm volatile ("orc  #0x80,ccr")

#define __save_flags(x) \
       asm volatile ("stc ccr,r1l\n\tmov.l er1,%0":"=r" (x) : : "er1")

#define __restore_flags(x) \
       asm volatile ("mov.l %0,er1\n\tldc r1l,ccr": :"r" (x) : "er1")

#define	__save_and_cli(flags) do { save_flags(flags); cli(); } while(0) 
#define	__save_and_sti(flags) do { save_flags(flags); sti(); } while(0) 

#define iret() __asm__ __volatile__ ("rte": : :"memory", "sp", "cc")

/* For spinlocks etc */
#define local_irq_save(x)	({ __save_flags(x); __cli(); })
#define local_irq_set(x)	({ __save_flags(x); __sti(); })
#define local_irq_restore(x)	__restore_flags(x)
#define local_irq_disable()	__cli()
#define local_irq_enable()	__sti()

#define cli()			__cli()
#define sti()			__sti()
#define save_flags(x)		__save_flags(x)
#define restore_flags(x)	__restore_flags(x)
#define save_and_cli(x)	__save_and_cli(x)
#define save_and_set(x)	__save_and_sti(x)

/*
 * Force strict CPU ordering.
 * Not really required on h8...
 */
#define nop()  asm volatile ("nop"::)
#define mb()   asm volatile (""   : : :"memory")
#define rmb()  asm volatile (""   : : :"memory")
#define wmb()  asm volatile (""   : : :"memory")
#define set_mb(var, value)     do { var = value; mb(); } while (0)
#define set_wmb(var, value)    do { var = value; wmb(); } while (0)

#ifdef CONFIG_SMP
#define smp_mb()	mb()
#define smp_rmb()	rmb()
#define smp_wmb()	wmb()
#else
#define smp_mb()	barrier()
#define smp_rmb()	barrier()
#define smp_wmb()	barrier()
#endif

#define xchg(ptr,x) ((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))
#define tas(ptr) (xchg((ptr),1))

struct __xchg_dummy { unsigned long a[100]; };
#define __xg(x) ((volatile struct __xchg_dummy *)(x))

static inline unsigned long __xchg(unsigned long x, volatile void * ptr, int size)
{
  unsigned long tmp, flags;

  save_flags(flags);
  cli();

  switch (size) {
  case 1:
    __asm__ __volatile__
    ("mov.b %2,%0\n\t"
     "mov.b %1,%2"
    : "=&r" (tmp) : "r" (x), "m" (*__xg(ptr)) : "er0","memory");
    break;
  case 2:
    __asm__ __volatile__
    ("mov.w %2,%0\n\t"
     "mov.w %1,%2"
    : "=&r" (tmp) : "r" (x), "m" (*__xg(ptr)) : "memory");
    break;
  case 4:
    __asm__ __volatile__
    ("mov.l %2,%0\n\t"
     "mov.l %1,%2"
    : "=&r" (tmp) : "r" (x), "m" (*__xg(ptr)) : "memory");
    break;
  default:
    tmp = 0;	  
  }
  restore_flags(flags);
  return tmp;
}

#define HARD_RESET_NOW() ({		\
        cli();				\
        asm("jmp @@0");			\
})

#endif /* _H8300_SYSTEM_H */
