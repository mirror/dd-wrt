#ifndef _H8300_DELAY_H
#define _H8300_DELAY_H

#include <asm/param.h>

/*
 * Copyright (C) 2002 Yoshinori Sato <ysato@sourceforge.jp>
 *
 * Delay routines, using a pre-computed "loops_per_second" value.
 */

extern __inline__ void __delay(unsigned long loops)
{
	__asm__ __volatile__ ("mov.l %0,er0\n\t"
			      "1:\n\t"
			      "dec.l #1,er0\n\t"
			      "bne 1b"
			      ::"r" (loops):"er0");
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
	usecs *= 4295;		/* 2**32 / 1000000 */
	usecs /= (loops_per_jiffy*HZ);
	if (usecs)
		__delay(usecs);
}

#define ndelay(nsecs) udelay((nsecs) * 5)

#endif /* _H8300_DELAY_H */
