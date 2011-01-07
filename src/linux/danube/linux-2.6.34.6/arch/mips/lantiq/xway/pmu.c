/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#include <xway.h>

#define LQ_PMU_PWDCR        ((u32 *)(LQ_PMU_BASE_ADDR + 0x001C))
#define LQ_PMU_PWDSR        ((u32 *)(LQ_PMU_BASE_ADDR + 0x0020))

void
lq_pmu_enable(unsigned int module)
{
	int err = 1000000;

	lq_w32(lq_r32(LQ_PMU_PWDCR) & ~module, LQ_PMU_PWDCR);
	while (--err && (lq_r32(LQ_PMU_PWDSR) & module));

	if (!err)
		panic("activating PMU module failed!");
}
EXPORT_SYMBOL(lq_pmu_enable);

void
lq_pmu_disable(unsigned int module)
{
	lq_w32(lq_r32(LQ_PMU_PWDCR) | module, LQ_PMU_PWDCR);
}
EXPORT_SYMBOL(lq_pmu_disable);
