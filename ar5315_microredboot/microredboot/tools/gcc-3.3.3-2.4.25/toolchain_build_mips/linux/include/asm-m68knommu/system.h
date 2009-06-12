#ifndef _M68KNOMMU_SYSTEM_H
#define _M68KNOMMU_SYSTEM_H

#include <linux/config.h> /* get configuration macros */
#include <linux/linkage.h>
#include <asm/segment.h>
#include <asm/entry.h>

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
 */
asmlinkage void resume(void);
#define switch_to(prev,next,last) { \
  void *_last;										\
  __asm__ __volatile__(								\
  			"movel	%1, %%a0\n\t"					\
			"movel	%2, %%a1\n\t"					\
			"jbsr " SYMBOL_NAME_STR(resume) "\n\t" 	\
			"movel	%%d1, %0\n\t"					\
		       : "=d" (_last)						\
		       : "d" (prev),						\
			     "d" (next)							\
		       : "cc", "d0", "d1", "d2", "d3", "d4", "d5", "a0", "a1");	\
  (last) = _last; 									\
}

#ifdef CONFIG_COLDFIRE
#define __sti() __asm__ __volatile__ ( \
	"move %/sr,%%d0\n\t" \
	"andi.l #0xf8ff,%%d0\n\t" \
	"move %%d0,%/sr\n" \
	: /* no outputs */ \
	: \
        : "cc", "%d0", "memory")
#define __cli() __asm__ __volatile__ ( \
	"move %/sr,%%d0\n\t" \
	"ori.l  #0x0700,%%d0\n\t" \
	"move %%d0,%/sr\n" \
	: /* no inputs */ \
	: \
	: "cc", "%d0", "memory")
#else

/* portable version */ /* FIXME - see entry.h*/
#define ALLOWINT 0xf8ff

#define __sti() asm volatile ("andiw %0,%%sr": : "i" (ALLOWINT) : "memory")
#define __cli() asm volatile ("oriw  #0x0700,%%sr": : : "memory")
#endif

#define __save_flags(x) asm volatile ("movew %%sr,%0":"=d" (x) : : "memory")
#define __restore_flags(x) asm volatile ("movew %0,%%sr": :"d" (x) : "memory")
#define	__save_and_cli(flags) do { save_flags(flags); cli(); } while(0) 
#define	__save_and_sti(flags) do { save_flags(flags); sti(); } while(0) 

#define iret() __asm__ __volatile__ ("rte": : :"memory", "sp", "cc")

/* For spinlocks etc */
#define local_irq_save(x)	__save_and_cli(x)
#define local_irq_set(x)	__save_and_sti(x)
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
 * Not really required on m68k...
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

#ifndef CONFIG_RMW_INSNS
static inline unsigned long __xchg(unsigned long x, volatile void * ptr, int size)
{
  unsigned long tmp, flags;

  save_flags(flags);
  cli();

  switch (size) {
  case 1:
    __asm__ __volatile__
    ("moveb %2,%0\n\t"
     "moveb %1,%2"
    : "=&d" (tmp) : "d" (x), "m" (*__xg(ptr)) : "memory");
    break;
  case 2:
    __asm__ __volatile__
    ("movew %2,%0\n\t"
     "movew %1,%2"
    : "=&d" (tmp) : "d" (x), "m" (*__xg(ptr)) : "memory");
    break;
  case 4:
    __asm__ __volatile__
    ("movel %2,%0\n\t"
     "movel %1,%2"
    : "=&d" (tmp) : "d" (x), "m" (*__xg(ptr)) : "memory");
    break;
  }
  restore_flags(flags);
  return tmp;
}
#else
static inline unsigned long __xchg(unsigned long x, volatile void * ptr, int size)
{
	switch (size) {
	    case 1:
		__asm__ __volatile__
			("moveb %2,%0\n\t"
			 "1:\n\t"
			 "casb %0,%1,%2\n\t"
			 "jne 1b"
			 : "=&d" (x) : "d" (x), "m" (*__xg(ptr)) : "memory");
		break;
	    case 2:
		__asm__ __volatile__
			("movew %2,%0\n\t"
			 "1:\n\t"
			 "casw %0,%1,%2\n\t"
			 "jne 1b"
			 : "=&d" (x) : "d" (x), "m" (*__xg(ptr)) : "memory");
		break;
	    case 4:
		__asm__ __volatile__
			("movel %2,%0\n\t"
			 "1:\n\t"
			 "casl %0,%1,%2\n\t"
			 "jne 1b"
			 : "=&d" (x) : "d" (x), "m" (*__xg(ptr)) : "memory");
		break;
	}
	return x;
}
#endif



#ifdef CONFIG_M68332
#define HARD_RESET_NOW() ({		\
        cli();				\
        asm("				\
	movew   #0x0000, 0xfffa6a;	\
        reset;				\
        /*movew #0x1557, 0xfffa44;*/	\
        /*movew #0x0155, 0xfffa46;*/	\
        moveal #0, %a0;			\
        movec %a0, %vbr;		\
        moveal 0, %sp;			\
        moveal 4, %a0;			\
        jmp (%a0);			\
        ");				\
})
#endif

#if defined( CONFIG_M68328 ) || defined( CONFIG_M68EZ328 ) || \
	defined (CONFIG_M68360) || defined( CONFIG_M68VZ328 )
#define HARD_RESET_NOW() ({		\
        cli();				\
        asm("				\
        moveal #0x10c00000, %a0;	\
        moveb #0, 0xFFFFF300;		\
        moveal 0(%a0), %sp;		\
        moveal 4(%a0), %a0;		\
        jmp (%a0);			\
        ");				\
})
#endif

#ifdef CONFIG_COLDFIRE
#if defined(CONFIG_M5272) && (defined(CONFIG_NETtel) || \
	defined(CONFIG_GILBARCONAP) || defined(CONFIG_SE1100) || defined(CONFIG_HW_FEITH))
/*
 *	Need to account for broken early mask of 5272 silicon. So don't
 *	jump through the original start address. Jump strait into the
 *	known start of the FLASH code.
 */
#define HARD_RESET_NOW() ({		\
        asm("				\
	movew #0x2700, %sr;		\
        jmp 0xf0000400;			\
        ");				\
})
/*
 * HARD_RESET_NOW() for senTec COBRA5272:
 *
 * ATTENTION!
 * This seems to be a dirty workaround for the reboot!
 * We currently just jump to the startcode of colilo.
 * 
 * If you are using dBUG, you will have to ajust the jmp
 * destination. Look for the address of _asm_startmeup, and
 * put it in the jump statement!
 * 
 */
#elif defined(CONFIG_COBRA5272)
#define HARD_RESET_NOW() ({      \
   asm("           \
         jmp 0xffe00400;             \
       ");          \
})

#elif defined(CONFIG_NETtel) || defined(CONFIG_eLIA) || defined(CONFIG_DISKtel) || defined(CONFIG_SECUREEDGEMP3) || defined(CONFIG_CLEOPATRA)
#define HARD_RESET_NOW() ({		\
        asm("				\
	movew #0x2700, %sr;		\
	moveal #0x10000044, %a0;	\
	movel #0xffffffff, (%a0);	\
	moveal #0x10000001, %a0;	\
	moveb #0x00, (%a0);		\
        moveal #0xf0000004, %a0;	\
        moveal (%a0), %a0;		\
        jmp (%a0);			\
        ");				\
})
#else
#define HARD_RESET_NOW() ({		\
        asm("				\
	movew #0x2700, %sr;		\
        moveal #0x4, %a0;		\
        moveal (%a0), %a0;		\
        jmp (%a0);			\
        ");				\
})
#endif
#endif

#endif /* _M68KNOMMU_SYSTEM_H */
