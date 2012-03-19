/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/pm.h>
#include <linux/export.h>
#include <asm/reboot.h>

#include <lantiq_soc.h>

#include "../devices.h"

#define ltq_rcu_w32(x, y)	ltq_w32((x), ltq_rcu_membase + (y))
#define ltq_rcu_r32(x)		ltq_r32(ltq_rcu_membase + (x))

/* register definitions */
#define LTQ_RCU_RST		0x0010
#define LTQ_RCU_RST_ALL		0x40000000

#define LTQ_RCU_RST_STAT	0x0014
#define LTQ_RCU_STAT_SHIFT	26

static struct resource ltq_rcu_resource =
	MEM_RES("rcu", LTQ_RCU_BASE_ADDR, LTQ_RCU_SIZE);

/* remapped base addr of the reset control unit */
static void __iomem *ltq_rcu_membase;

/* This function is used by the watchdog driver */
int ltq_reset_cause(void)
{
	u32 val = ltq_rcu_r32(LTQ_RCU_RST_STAT);
	return val >> LTQ_RCU_STAT_SHIFT;
}
EXPORT_SYMBOL_GPL(ltq_reset_cause);

static void ltq_machine_restart(char *command)
{
	pr_notice("System restart\n");
	local_irq_disable();
	ltq_rcu_w32(ltq_rcu_r32(LTQ_RCU_RST) | LTQ_RCU_RST_ALL, LTQ_RCU_RST);
	unreachable();
}

static void ltq_machine_halt(void)
{
	pr_notice("System halted.\n");
	local_irq_disable();
	unreachable();
}

static void ltq_machine_power_off(void)
{
	pr_notice("Please turn off the power now.\n");
	local_irq_disable();
	unreachable();
}

static int __init mips_reboot_setup(void)
{
	/* remap rcu register range */
	ltq_rcu_membase = ltq_remap_resource(&ltq_rcu_resource);
	if (!ltq_rcu_membase)
		panic("Failed to remap rcu memory\n");

	_machine_restart = ltq_machine_restart;
	_machine_halt = ltq_machine_halt;
	pm_power_off = ltq_machine_power_off;

	return 0;
}

arch_initcall(mips_reboot_setup);
