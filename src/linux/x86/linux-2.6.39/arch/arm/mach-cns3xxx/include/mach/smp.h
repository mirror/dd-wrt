#ifndef __MACH_SMP_H
#define __MACH_SMP_H

#include <asm/hardware/gic.h>

/*
 * We use IRQ1 as the IPI
 */
static inline void smp_cross_call(const struct cpumask *mask, int call)
{
	gic_raise_softirq(mask, call);
}

static inline void smp_cross_call_cache(const struct cpumask *mask)
{
	gic_raise_softirq(mask, 1);
}

#endif
