/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/init.h>
#include <linux/io.h>
#include <linux/pm.h>
#include <asm/reboot.h>

#include <xway.h>

#define LQ_RCU_RST			((u32 *)(LQ_RCU_BASE_ADDR + 0x0010))
#define LQ_RCU_RST_ALL		0x40000000

static void
lq_machine_restart(char *command)
{
	printk(KERN_NOTICE "System restart\n");
	local_irq_disable();
#if defined(CONFIG_AR9)
	lq_w32(lq_r32(LQ_RCU_PPE_CONF) & ~(3 << 30),	LQ_RCU_PPE_CONF);
#endif
	lq_w32(lq_r32(LQ_RCU_RST) | LQ_RCU_RST_ALL,	LQ_RCU_RST);
	for(;;);
}

static void
lq_machine_halt(void)
{
	printk(KERN_NOTICE "System halted.\n");
	local_irq_disable();
	for(;;);
}

static void
lq_machine_power_off(void)
{
	printk(KERN_NOTICE "Please turn off the power now.\n");
	local_irq_disable();
	for(;;);
}

static int __init
mips_reboot_setup(void)
{
	_machine_restart = lq_machine_restart;
	_machine_halt = lq_machine_halt;
	pm_power_off = lq_machine_power_off;
	return 0;
}

arch_initcall(mips_reboot_setup);
