/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 * Copyright (C) 2010 Thomas Langer, Lantiq Deutschland
 * Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/io.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/list.h>

#include <asm/time.h>
#include <asm/irq.h>
#include <asm/div64.h>

#include <lantiq.h>
#ifdef CONFIG_SOC_LANTIQ_XWAY
#include <xway.h>
#endif

extern unsigned long lq_get_cpu_hz(void);
extern unsigned long lq_get_fpi_hz(void);
extern unsigned long lq_get_io_region_clock(void);

struct clk {
	const char *name;
	unsigned long rate;
	unsigned long (*get_rate) (void);
};

static struct clk *cpu_clk = 0;
static int cpu_clk_cnt = 0;

static unsigned int r4k_offset;
static unsigned int r4k_cur;

static struct clk cpu_clk_generic[] = {
	{
		.name = "cpu",
		.get_rate = lq_get_cpu_hz,
	}, {
		.name = "fpi",
		.get_rate = lq_get_fpi_hz,
	}, {
		.name = "io",
		.get_rate = lq_get_io_region_clock,
	},
};

void
clk_init(void)
{
	int i;
	cpu_clk = cpu_clk_generic;
	cpu_clk_cnt = ARRAY_SIZE(cpu_clk_generic);
	for(i = 0; i < cpu_clk_cnt; i++)
		printk("%s: %ld\n", cpu_clk[i].name, clk_get_rate(&cpu_clk[i]));
}

static inline int
clk_good(struct clk *clk)
{
	return clk && !IS_ERR(clk);
}

unsigned long
clk_get_rate(struct clk *clk)
{
	if (unlikely(!clk_good(clk)))
		return 0;

	if (clk->rate != 0)
		return clk->rate;

	if (clk->get_rate != NULL)
		return clk->get_rate();

	return 0;
}
EXPORT_SYMBOL(clk_get_rate);

struct clk*
clk_get(struct device *dev, const char *id)
{
	int i;
	for(i = 0; i < cpu_clk_cnt; i++)
		if (!strcmp(id, cpu_clk[i].name))
			return &cpu_clk[i];
	BUG();
	return ERR_PTR(-ENOENT);
}
EXPORT_SYMBOL(clk_get);

void
clk_put(struct clk *clk)
{
	/* not used */
}
EXPORT_SYMBOL(clk_put);

static inline u32
lq_get_counter_resolution(void)
{
	u32 res;
	__asm__ __volatile__(
		".set   push\n"
		".set   mips32r2\n"
		".set   noreorder\n"
		"rdhwr  %0, $3\n"
		"ehb\n"
		".set pop\n"
		: "=&r" (res)
		: /* no input */
		: "memory");
	instruction_hazard();
	return res;
}

void __init
plat_time_init(void)
{
	struct clk *clk = clk_get(0, "cpu");
	mips_hpt_frequency = clk_get_rate(clk) / lq_get_counter_resolution();
	r4k_cur = (read_c0_count() + r4k_offset);
	write_c0_compare(r4k_cur);

#ifdef CONFIG_SOC_LANTIQ_XWAY
#define LQ_GPTU_GPT_CLC			((u32 *)(LQ_GPTU_BASE_ADDR + 0x0000))
	lq_pmu_enable(PMU_GPT);
	lq_pmu_enable(PMU_FPI);

	lq_w32(0x100, LQ_GPTU_GPT_CLC);
#endif
}
