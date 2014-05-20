/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/io.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/clk.h>

#include <asm/time.h>
#include <asm/irq.h>
#include <asm/div64.h>

#include <lantiq_soc.h>

#define CLOCK_62_5M                         62500000
#define CLOCK_83_5M                         83500000
#define CLOCK_125M                          125000000
#define CLOCK_200M                          200000000
#define CLOCK_250M                          250000000
#define CLOCK_300M                          300000000
#define CLOCK_98_304M                       98304000
#define CLOCK_150M                          150000000
#define CLOCK_196_608M                      196608000
#define CLOCK_600M                          600000000
#define CLOCK_500M                          500000000
#define CLOCK_393M                          393215332
#define CLOCK_166M                          166666666

#define LTQ_CGU_SYS	0x0c
#define LTQ_CGU_IF_CLK	0x24

unsigned int ltq_get_cpu_hz(void)
{
	int clks[] = {
		CLOCK_600M, CLOCK_500M,	CLOCK_393M, CLOCK_333M, CLOCK_125M,
		CLOCK_125M, CLOCK_196_608M, CLOCK_166M, CLOCK_125M, CLOCK_125M };
	int val = (ltq_cgu_r32(LTQ_CGU_SYS) >> 4) & 0xf;

	if (val > 9)
		panic("bad cpu speed\n");
	if (val == 2)
		panic("missing workaround\n");
		//cgu_get_pll1_fosc(); //CLOCK_393M;
	return clks[val];
}
EXPORT_SYMBOL(ltq_get_cpu_hz);

unsigned int ltq_get_fpi_hz(void)
{
	int clks[] = {
		CLOCK_62_5M, CLOCK_62_5M, CLOCK_83_5M, CLOCK_125M, CLOCK_125M,
		CLOCK_125M, CLOCK_167M, CLOCK_200M, CLOCK_250M, CLOCK_300M,
		CLOCK_62_5M, CLOCK_98_304M, CLOCK_150M, CLOCK_196_608M };
	int val = ((ltq_cgu_r32(LTQ_CGU_IF_CLK) >> 25) & 0xf);

	if (val > 13)
		panic("bad fpi speed\n");

	return clks[val];
}
EXPORT_SYMBOL(ltq_get_fpi_hz);

unsigned int ltq_get_io_region_clock(void)
{
	return ltq_get_fpi_hz() / 2;
}
EXPORT_SYMBOL(ltq_get_io_region_clock);

unsigned int getCPUClock(void)
{
    return ltq_get_cpu_hz()/1000000;
}

unsigned int ltq_get_fpi_bus_clock(int fpi)
{
	return ltq_get_fpi_hz();
}
EXPORT_SYMBOL(ltq_get_fpi_bus_clock);
