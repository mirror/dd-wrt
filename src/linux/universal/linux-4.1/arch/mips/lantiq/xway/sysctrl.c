/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2011 John Crispin <blogic@openwrt.org>
 */

#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <lantiq_soc.h>

#include "../devices.h"

/* clock control register */
#define LTQ_CGU_IFCCR	0x0018

/* the enable / disable registers */
#define LTQ_PMU_PWDCR	0x1C
#define LTQ_PMU_PWDSR	0x20

#define ltq_pmu_w32(x, y)	ltq_w32((x), ltq_pmu_membase + (y))
#define ltq_pmu_r32(x)		ltq_r32(ltq_pmu_membase + (x))

static struct resource ltq_cgu_resource =
	MEM_RES("cgu", LTQ_CGU_BASE_ADDR, LTQ_CGU_SIZE);

static struct resource ltq_pmu_resource =
	MEM_RES("pmu", LTQ_PMU_BASE_ADDR, LTQ_PMU_SIZE);

static struct resource ltq_ebu_resource =
	MEM_RES("ebu", LTQ_EBU_BASE_ADDR, LTQ_EBU_SIZE);

void __iomem *ltq_cgu_membase;
void __iomem *ltq_ebu_membase;
static void __iomem *ltq_pmu_membase;

void ltq_cgu_enable(unsigned int clk)
{
	ltq_cgu_w32(ltq_cgu_r32(LTQ_CGU_IFCCR) | clk, LTQ_CGU_IFCCR);
}

EXPORT_SYMBOL(ltq_cgu_enable);

void ltq_pmu_enable(unsigned int module)
{
	int err = 1000000;

	ltq_pmu_w32(ltq_pmu_r32(LTQ_PMU_PWDCR) & ~module, LTQ_PMU_PWDCR);
	do {} while (--err && (ltq_pmu_r32(LTQ_PMU_PWDSR) & module));

	if (!err)
		panic("activating PMU module failed!\n");
}
EXPORT_SYMBOL(ltq_pmu_enable);

void ltq_pmu_disable(unsigned int module)
{
	ltq_pmu_w32(ltq_pmu_r32(LTQ_PMU_PWDCR) | module, LTQ_PMU_PWDCR);
}
EXPORT_SYMBOL(ltq_pmu_disable);

void __init ltq_soc_init(void)
{
	ltq_pmu_membase = ltq_remap_resource(&ltq_pmu_resource);
	if (!ltq_pmu_membase)
		panic("Failed to remap pmu memory\n");

	ltq_cgu_membase = ltq_remap_resource(&ltq_cgu_resource);
	if (!ltq_cgu_membase)
		panic("Failed to remap cgu memory\n");

	ltq_ebu_membase = ltq_remap_resource(&ltq_ebu_resource);
	if (!ltq_ebu_membase)
		panic("Failed to remap ebu memory\n");

	/* make sure to unprotect the memory region where flash is located */
	ltq_ebu_w32(ltq_ebu_r32(LTQ_EBU_BUSCON0) & ~EBU_WRDIS, LTQ_EBU_BUSCON0);
}
