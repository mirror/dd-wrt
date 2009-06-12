#ifndef _NIOS_DELAY_H
#define _NIOS_DELAY_H

#include <asm/param.h>

extern __inline__ void __delay(unsigned long loops)
{
	unsigned long dummy;
	__asm__ __volatile__("cmpi %0, 0\n\t"
			     "1: skps cc_eq\n\t"
			     "br 1b\n\t"
			     "subi %0, 1\n" 
			     :
			     "=&r" (dummy) :
			     "0" (loops) :
			     "cc");
}

/*
 * Use only for very small delays ( < 1 msec).  Should probably use a
 * lookup table, really, as the multiplications take much too long with
 * short delays.  This is a "reasonable" implementation, though (and the
 * first constant multiplications gets optimized away if the delay is
 * a constant)  
 */

extern unsigned long loops_per_jiffy;

extern __inline__ void udelay(unsigned long usecs)
{
	register unsigned long full_loops, part_loops;

	full_loops = ((usecs * HZ) / 1000000) * loops_per_jiffy;
	usecs %= (1000000 / HZ);
	part_loops = (usecs * HZ * loops_per_jiffy) / 1000000;

	__delay(full_loops + part_loops);
}

#define ndelay(nsecs) udelay((nsecs) * 5)

#define muldiv(a, b, c)    (((a)*(b))/(c))

#endif /* defined(_NIOS_DELAY_H) */
