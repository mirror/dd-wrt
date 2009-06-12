#ifndef _HYPERSTONE_NOMMU_SYSTEM_H
#define _HYPERSTONE_NOMMU_SYSTEM_H

#include <linux/config.h> /* get configuration macros */
#include <asm/segment.h>

#define prepare_to_switch()          do { } while(0)
struct task_struct;
extern void __switch_to(struct task_struct * prev, struct task_struct * next);
#define switch_to(prev,next,last)    __switch_to( prev, next )

/* interrupt control */
#define __cli()			asm volatile ("ori   SR, 0x00008000 \n":::"cc"); 
#define __sti()			asm volatile ("andni SR, 0x00008000 \n":::"cc");

#define __save_flags(x)		\
	  asm volatile ("mask %0, SR, 0x0000800F\n\t" \
				    :"=l"(x) \
		  :/* no input*/ \
		  :"memory"); 
							
#define __restore_flags(x)						\
{ static unsigned long temp; 					\
		asm volatile (							\
			"movi %0, 0x0\n\t"					\
			"mask %0, SR, 0xffff7FF0\n\t" 		\
			"or   %0, %1\n\t" 					\
			"mov  SR, %0\n\t" 					\
		  :/* no output*/  \
		  :"l" (temp), "l"(x) \
		  :"memory"); }

#define	__save_and_cli(flags) \
		  do { save_flags(flags); cli(); }\
		  while(0) 

#define cli()   		__cli()
#define sti()   		__sti()
#define save_flags(flags)       __save_flags(flags)
#define restore_flags(flags)    __restore_flags(flags)
#define save_and_cli(flags)     __save_and_cli(flags)

/* For spinlocks etc */
#define local_irq_save(x)       ({ __save_flags(x); __cli(); })
#define local_irq_restore(x)    ({__restore_flags(x); }) 
#define local_irq_disable()     __cli()
#define local_irq_enable()      __sti()

#define nop()          	__asm__ __volatile__ ("nop"::)
#define mb()			__asm__ __volatile__ ("" ::: "memory")
#define rmb()			mb ()
#define wmb()			mb ()
#define read_barrier_depends()	((void)0)
#define set_mb(var, value)	do { var = value; mb (); } while (0)
#define set_wmb(var, value)	do { var = value; wmb (); } while (0)

#define smp_mb()	mb ()
#define smp_rmb()	rmb ()
#define smp_wmb()	wmb ()
#define smp_read_barrier_depends()	read_barrier_depends()
	
#define xchg(ptr,x) ((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))
#define tas(ptr) (xchg((ptr),1))

/* This function doesn't exist, so you'll get a linker error
   if something tries to do an invalid xchg().  */

extern unsigned long __xchg_called_with_bad_pointer(void);

static inline unsigned long __xchg(unsigned long x, volatile void * ptr, int size)
{

  unsigned long tmp; //, flags; /* FIXME ?? */
//  save_flags(flags);
//  cli();

  switch (size) {
  case 1:
	__asm__ __volatile__
    ("
	  ldbu.d %2, %0, 0 
	  stbu.d %2, %1, 0"
    : "=l" (tmp) : "l" (x), "l" (ptr) : "memory");
    break;
  case 2:
    __asm__ __volatile__
    ("
	  ldhu.d %2, %0, 0 
	  sthu.d %2, %1, 0"
    : "=l" (tmp) : "l" (x), "l" (ptr) : "memory");
    break;
  case 4:
    __asm__ __volatile__
    ("
	  ldw.d %2, %0, 0 
	  stw.d %2, %1, 0"
    : "=l" (tmp) : "l" (x), "l" (ptr) : "memory");
    break;
	
  default:
	__xchg_called_with_bad_pointer();
  }
//  restore_flags(flags);
	return tmp;
}
/* Yannis: FIXME */
#define HARD_RESET_NOW()	do{}while(0)

#endif 
