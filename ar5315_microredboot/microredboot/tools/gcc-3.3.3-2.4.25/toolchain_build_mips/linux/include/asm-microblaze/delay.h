/*
 * include/asm-microblaze/delay.h -- Delay routines, using a pre-computed
 * 	"loops_per_second" value
 *
 *  Copyright (C) 2003  John Williams <jwilliams@itee.uq.edu.au>
 *                      based upon v850 version
 *  Copyright (C) 2001  NEC Corporation
 *  Copyright (C) 2001  Miles Bader <miles@gnu.org>
 *  Copyright (C) 1994 Hamish Macdonald
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 */

#ifndef __MICROBLAZE_DELAY_H__
#define __MICROBLAZE_DELAY_H__

#include <asm/param.h>

extern __inline__ void __delay(unsigned long loops)
{
	__asm__ __volatile__ ("1:	addi	%0, %0, -1;		\
					bneid	%0, 1b;			\
					nop;"
			      : "=r" (loops) : "0" (loops));
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

#endif /* __MICROBLAZE_DELAY_H__ */
