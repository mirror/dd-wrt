/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/module.h>
#include <linux/clk.h>
#include <asm/bootinfo.h>
#include <asm/time.h>

#include <lantiq_soc.h>

#include "devices.h"
#include "../prom.h"

#define SOC_VRX288	"VRX288"

#define PART_SHIFT	12
#define PART_MASK	0x0FFFFFFF
#define REV_SHIFT	28
#define REV_MASK	0xF0000000

void __init ltq_soc_detect(struct ltq_soc_info *i)
{
	i->partnum = (ltq_r32(LTQ_MPS_CHIPID) & PART_MASK) >> PART_SHIFT;
	i->rev = (ltq_r32(LTQ_MPS_CHIPID) & REV_MASK) >> REV_SHIFT;
	sprintf(i->rev_type, "1.%d", i->rev);
	switch (i->partnum) {
	case SOC_ID_VRX288:
		i->name = SOC_VRX288;
		i->type = SOC_TYPE_VR9;
		break;

	default:
		unreachable();
		break;
	}
	printk("%08X\n", i->partnum);
}

void __init ltq_soc_setup(void)
{
	/*
		reg = IFX_REG_R32(IFX_XBAR_ALWAYS_LAST);
		reg &= ~ IFX_XBAR_FPI_BURST_EN;
		IFX_REG_W32(reg, IFX_XBAR_ALWAYS_LAST);
	*/

	ltq_register_asc(1);
	ltq_register_gpio();
	ltq_register_wdt();
}
