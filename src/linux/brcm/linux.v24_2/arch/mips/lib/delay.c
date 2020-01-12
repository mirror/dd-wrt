/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994 by Waldorf Electronics
 * Copyright (C) 1995 - 2000, 01, 03 by Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 * Copyright (C) 2007  Maciej W. Rozycki
 */
#include <linux/delay.h>
#include <linux/module.h>

void __delay(unsigned long loops)
{
	if (sizeof(long) == 4)
		__asm__ __volatile__ (
		"	.set	noreorder				\n"
		"	.align	3					\n"
		"1:	bnez	%0, 1b					\n"
		"	subu	%0, 1					\n"
		"	.set	reorder					\n"
		: "=r" (loops)
		: "0" (loops));
	else if (sizeof(long) == 8)
		__asm__ __volatile__ (
		"	.set	noreorder				\n"
		"	.align	3					\n"
		"1:	bnez	%0, 1b					\n"
		"	dsubu	%0, 1					\n"
		"	.set	reorder					\n"
		: "=r" (loops)
		: "0" (loops));
}
EXPORT_SYMBOL(__delay);

void __udelay(unsigned long us, unsigned int lpj)
{
	__delay((us * 0x000010c7ull * HZ * lpj) >> 32);
}
EXPORT_SYMBOL(__udelay);
/*
 * Division by multiplication: you don't have to worry about
 * loss of precision.
 *
 * Use only for very small delays ( < 1 msec).  Should probably use a
 * lookup table, really, as the multiplications take much too long with
 * short delays.  This is a "reasonable" implementation, though (and the
 * first constant multiplications gets optimized away if the delay is
 * a constant)
 */

void __ndelay(unsigned long ns, unsigned int lpj)
{
	__delay((ns * 0x00000005ull * HZ * lpj) >> 32);
}
EXPORT_SYMBOL(__ndelay);