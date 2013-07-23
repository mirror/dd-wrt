/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * Copyright (C) 2011 Thomas Langer <thomas.langer@lantiq.com>
 * Copyright (C) 2011 John Crispin <blogic@openwrt.org>
 */

#include <linux/ioport.h>
#include <linux/module.h>

#include <lantiq_soc.h>

#include "devices.h"

/* CPU0 Clock Control Register */
#define LTQ_SYS1_CPU0CC		0x0040
/* clock divider bit */
#define LTQ_CPU0CC_CPUDIV	0x0001

unsigned int
ltq_get_io_region_clock(void)
{
	return CLOCK_200M;
}
EXPORT_SYMBOL(ltq_get_io_region_clock);

unsigned int
ltq_get_cpu_hz(void)
{
	if (ltq_sys1_r32(LTQ_SYS1_CPU0CC) & LTQ_CPU0CC_CPUDIV)
		return CLOCK_200M;
	else
		return CLOCK_400M;
}
EXPORT_SYMBOL(ltq_get_cpu_hz);

unsigned int
ltq_get_fpi_hz(void)
{
	return CLOCK_100M;
}
EXPORT_SYMBOL(ltq_get_fpi_hz);
