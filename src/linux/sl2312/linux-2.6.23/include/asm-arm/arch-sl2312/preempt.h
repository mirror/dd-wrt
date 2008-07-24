/*
 * include/asm-arm/arch-sl2312/preempt.h
 *
 * Timing support for preempt-stats, kfi, ilatency patches
 *
 * Author: dsingleton <dsingleton@mvista.com>
 *
 * 2001-2004 (c) MontaVista Software, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */

#ifndef _ASM_ARCH_PREEMT_H
#define _ASM_ARCH_PREEMT_H

#include <asm/arch/hardware.h>
#include <asm/arch/sl2312.h>

static inline unsigned long clock_diff(unsigned long start, unsigned long stop)
{
        return (start - stop);
}

static inline unsigned int readclock(void)
{
	unsigned int	x;

	x = readl(IO_ADDRESS(SL2312_TIMER2_BASE));
	return x;
}

static inline unsigned __ticks_per_usec(void)
{
#ifdef CONFIG_SL3516_ASIC
	unsigned int ahb_clock_rate_base=130;  /* unit = MHz*/
	unsigned int reg_v=0;
	unsigned int ticks_usec;

	reg_v = readl(IO_ADDRESS((SL2312_GLOBAL_BASE+4)));
	reg_v >>=15;
	ticks_usec = (ahb_clock_rate_base + (reg_v & 0x07)*10)>>2;

#else
	unsigned int ticks_usec=20;
#endif

    return ticks_usec;
}

/*
 * timer 1 runs @ 6Mhz  6 ticks = 1 microsecond
 * and is configed as a count down timer.
 */
#define TICKS_PER_USEC		    __ticks_per_usec()
#define ARCH_PREDEFINES_TICKS_PER_USEC

#define clock_to_usecs(x)	    ((x) / TICKS_PER_USEC)

#define INTERRUPTS_ENABLED(x)   (!(x & PSR_I_BIT))

#endif

