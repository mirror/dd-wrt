// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 * Copyright (C) 2015 Nikolay Martynov <mar.kolya@gmail.com>
 * Copyright (C) 2015 John Crispin <john@phrozen.org>
 */

#include <linux/init.h>

#include <linux/of.h>
#include <linux/of_clk.h>
#include <linux/clocksource.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>

#include <asm/time.h>

#include "common.h"

void __init plat_time_init(void)
{
	ralink_of_remap();

	of_clk_init(NULL);
	timer_probe();
}
#ifndef CONFIG_MIPS_GIC
int getCPUClock(void)
{
    struct clk *clk;
    clk = clk_get_sys("cpu", NULL);
    return clk_get_rate(clk) / 1000000;
}

u32 get_surfboard_sysclk(void) 
{
    struct clk *clk;
    clk = clk_get_sys("sys", NULL);
    return clk_get_rate(clk);
}
EXPORT_SYMBOL(get_surfboard_sysclk);
#endif
