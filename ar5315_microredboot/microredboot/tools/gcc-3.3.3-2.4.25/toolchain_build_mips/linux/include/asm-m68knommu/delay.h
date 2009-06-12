#ifndef _M68KNOMMU_DELAY_H
#define _M68KNOMMU_DELAY_H

#include <asm/param.h>

/*
 * Copyright (C) 1994 Hamish Macdonald
 *
 * Delay routines, using a pre-computed "loops_per_second" value.
 */

extern __inline__ void __delay(unsigned long loops)
{
#if defined(CONFIG_COLDFIRE)
	/* The coldfire runs this loop at significantly different speeds
	 * depending upon long word alignment or not.  We'll pad it to
	 * long word alignment which is the faster version.
	 * The 0x4a8e is of course a 'tstl %fp' instruction.  This is better
	 * than using a NOP (0x4e71) instruction because it executes in one
	 * cycle not three and doesn't allow for an arbitary delay waiting
	 * for bus cycles to finish.  Also fp/a6 isn't likely to cause a
	 * stall waiting for the register to become valid if such is added
	 * to the coldfire at some stage.
	 */
	__asm__ __volatile__ (	".balignw 4, 0x4a8e\n\t"
				"1: subql #1, %0\n\t"
				"jcc 1b"
		: "=d" (loops) : "0" (loops));
#else
	__asm__ __volatile__ (	"1: subql #1, %0\n\t"
				"jcc 1b"
		: "=d" (loops) : "0" (loops));
#endif
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
#ifdef CONFIG_M68332
	usecs *= 0x000010c6;       
	__asm__ __volatile__ (	"mulul %1,%0:%2"
		: "=d" (usecs) : "d" (usecs), "d" (loops_per_jiffy*HZ));
	__delay(usecs);

#elif defined(CONFIG_M68328) || defined(CONFIG_M68EZ328) || \
		defined(CONFIG_COLDFIRE) || defined(CONFIG_M68360) || \
		defined(CONFIG_M68VZ328)
	register unsigned long full_loops, part_loops;

	full_loops = ((usecs * HZ) / 1000000) * loops_per_jiffy;
	usecs %= (1000000 / HZ);
	part_loops = (usecs * HZ * loops_per_jiffy) / 1000000;

	__delay(full_loops + part_loops);
#else
	unsigned long tmp;

	usecs *= 4295;		/* 2**32 / 1000000 */
	__asm__ ("mulul %2,%0:%1"
		: "=d" (usecs), "=d" (tmp)
		: "d" (usecs), "1" (loops_per_jiffy*HZ));
	__delay(usecs);
#endif
}

#define ndelay(n)	udelay((n) * 5)

#if 0 /* DAVIDM - elsewhere under m68knommu */
extern __inline__ unsigned long muldiv(unsigned long a, unsigned long b, unsigned long c)
{
	unsigned long tmp;

	__asm__ ("mulul %2,%0:%1; divul %3,%0:%1"
		: "=d" (tmp), "=d" (a)
		: "d" (b), "d" (c), "1" (a));
	return a;
}
#endif

#endif /* defined(_M68KNOMMU_DELAY_H) */
