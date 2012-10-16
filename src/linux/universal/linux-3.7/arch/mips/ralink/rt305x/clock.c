/*
 *  Ralink RT305X clock API
 *
 *  Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/clk.h>

#include <asm/mach-ralink/common.h>
#include <asm/mach-ralink/rt305x.h>
#include <asm/mach-ralink/rt305x_regs.h>
#include "common.h"

struct clk {
	unsigned long rate;
};

static struct clk rt305x_cpu_clk;
static struct clk rt305x_sys_clk;
static struct clk rt305x_wdt_clk;
static struct clk rt305x_uart_clk;

void __init rt305x_clocks_init(void)
{
	u32	t;

	t = rt305x_sysc_rr(SYSC_REG_SYSTEM_CONFIG);
#if defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT3350)
        t = (t>>8) & 0x01;
#else
	t = ((t >> SYSTEM_CONFIG_CPUCLK_SHIFT) & SYSTEM_CONFIG_CPUCLK_MASK);
#endif

	switch (t) {
	case SYSTEM_CONFIG_CPUCLK_320:
#if defined (CONFIG_RALINK_RT3352)
		rt305x_cpu_clk.rate = 384000000;
#else
		rt305x_cpu_clk.rate = 320000000;
#endif
		break;
	case SYSTEM_CONFIG_CPUCLK_384:
#if defined (CONFIG_RALINK_RT3352)
		rt305x_cpu_clk.rate = 400000000;
#elif defined (CONFIG_RALINK_RT3350)
		rt305x_cpu_clk.rate = 320000000;
#else
		rt305x_cpu_clk.rate = 384000000;
#endif	
	break;
	}

	rt305x_sys_clk.rate = rt305x_cpu_clk.rate / 3;
#if defined (CONFIG_RALINK_RT3352)
	rt305x_uart_clk.rate = 40000000;
#else
	rt305x_uart_clk.rate = rt305x_sys_clk.rate;
#endif
	rt305x_wdt_clk.rate = rt305x_sys_clk.rate;
}

int getCPUClock(void)
{
    return rt305x_cpu_clk.rate / 1000 / 1000;
}

u32 get_surfboard_sysclk(void) 
{
	return rt305x_sys_clk.rate;
}
EXPORT_SYMBOL(get_surfboard_sysclk);

/*
 * Linux clock API
 */
struct clk *clk_get(struct device *dev, const char *id)
{
	if (!strcmp(id, "sys"))
		return &rt305x_sys_clk;

	if (!strcmp(id, "cpu"))
		return &rt305x_cpu_clk;

	if (!strcmp(id, "wdt"))
		return &rt305x_wdt_clk;

	if (!strcmp(id, "uart"))
		return &rt305x_uart_clk;

	return ERR_PTR(-ENOENT);
}
EXPORT_SYMBOL(clk_get);

int clk_enable(struct clk *clk)
{
	return 0;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	return clk->rate;
}
EXPORT_SYMBOL(clk_get_rate);

void clk_put(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_put);
