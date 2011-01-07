/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2007 Xu Liang, infineon
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/io.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/clk.h>

#include <asm/time.h>
#include <asm/irq.h>
#include <asm/div64.h>

#include <xway.h>

static unsigned int lq_ram_clocks[] = {CLOCK_167M, CLOCK_133M, CLOCK_111M, CLOCK_83M };
#define DDR_HZ lq_ram_clocks[lq_r32(LQ_CGU_SYS) & 0x3]

#define BASIC_FREQUENCY_1	35328000
#define BASIC_FREQUENCY_2	36000000
#define BASIS_REQUENCY_USB	12000000

#define GET_BITS(x, msb, lsb)           (((x) & ((1 << ((msb) + 1)) - 1)) >> (lsb))

#define CGU_PLL0_PHASE_DIVIDER_ENABLE   (lq_r32(LQ_CGU_PLL0_CFG) & (1 << 31))
#define CGU_PLL0_BYPASS                 (lq_r32(LQ_CGU_PLL0_CFG) & (1 << 30))
#define CGU_PLL0_CFG_DSMSEL             (lq_r32(LQ_CGU_PLL0_CFG) & (1 << 28))
#define CGU_PLL0_CFG_FRAC_EN            (lq_r32(LQ_CGU_PLL0_CFG) & (1 << 27))
#define CGU_PLL1_SRC                    (lq_r32(LQ_CGU_PLL1_CFG) & (1 << 31))
#define CGU_PLL2_PHASE_DIVIDER_ENABLE   (lq_r32(LQ_CGU_PLL2_CFG) & (1 << 20))
#define CGU_SYS_FPI_SEL                 (1 << 6)
#define CGU_SYS_DDR_SEL                 0x3
#define CGU_PLL0_SRC                    (1 << 29)

#define CGU_PLL0_CFG_PLLK               GET_BITS(*LQ_CGU_PLL0_CFG, 26, 17)
#define CGU_PLL0_CFG_PLLN               GET_BITS(*LQ_CGU_PLL0_CFG, 12, 6)
#define CGU_PLL0_CFG_PLLM               GET_BITS(*LQ_CGU_PLL0_CFG, 5, 2)
#define CGU_PLL2_SRC                    GET_BITS(*LQ_CGU_PLL2_CFG, 18, 17)
#define CGU_PLL2_CFG_INPUT_DIV          GET_BITS(*LQ_CGU_PLL2_CFG, 16, 13)

#define LQ_GPTU_GPT_CLC		((u32 *)(LQ_GPTU_BASE_ADDR + 0x0000))
#define LQ_CGU_PLL0_CFG		((u32 *)(LQ_CGU_BASE_ADDR + 0x0004))
#define LQ_CGU_PLL1_CFG		((u32 *)(LQ_CGU_BASE_ADDR + 0x0008))
#define LQ_CGU_PLL2_CFG		((u32 *)(LQ_CGU_BASE_ADDR + 0x000C))
#define LQ_CGU_SYS			((u32 *)(LQ_CGU_BASE_ADDR + 0x0010))
#define LQ_CGU_UPDATE		((u32 *)(LQ_CGU_BASE_ADDR + 0x0014))
#define LQ_CGU_IF_CLK		((u32 *)(LQ_CGU_BASE_ADDR + 0x0018))
#define LQ_CGU_OSC_CON		((u32 *)(LQ_CGU_BASE_ADDR + 0x001C))
#define LQ_CGU_SMD			((u32 *)(LQ_CGU_BASE_ADDR + 0x0020))
#define LQ_CGU_CT1SR		((u32 *)(LQ_CGU_BASE_ADDR + 0x0028))
#define LQ_CGU_CT2SR		((u32 *)(LQ_CGU_BASE_ADDR + 0x002C))
#define LQ_CGU_PCMCR		((u32 *)(LQ_CGU_BASE_ADDR + 0x0030))
#define LQ_CGU_PCI_CR		((u32 *)(LQ_CGU_BASE_ADDR + 0x0034))
#define LQ_CGU_PD_PC		((u32 *)(LQ_CGU_BASE_ADDR + 0x0038))
#define LQ_CGU_FMR			((u32 *)(LQ_CGU_BASE_ADDR + 0x003C))

static unsigned int lq_get_pll0_fdiv(void);

static inline unsigned int
get_input_clock(int pll)
{
	switch (pll) {
	case 0:
		if (lq_r32(LQ_CGU_PLL0_CFG) & CGU_PLL0_SRC)
			return BASIS_REQUENCY_USB;
		else if (CGU_PLL0_PHASE_DIVIDER_ENABLE)
			return BASIC_FREQUENCY_1;
		else
			return BASIC_FREQUENCY_2;
	case 1:
		if (CGU_PLL1_SRC)
			return BASIS_REQUENCY_USB;
		else if (CGU_PLL0_PHASE_DIVIDER_ENABLE)
			return BASIC_FREQUENCY_1;
		else
			return BASIC_FREQUENCY_2;
	case 2:
		switch (CGU_PLL2_SRC) {
		case 0:
			return lq_get_pll0_fdiv();
		case 1:
			return CGU_PLL2_PHASE_DIVIDER_ENABLE ?
				BASIC_FREQUENCY_1 :
				BASIC_FREQUENCY_2;
		case 2:
			return BASIS_REQUENCY_USB;
		}
	default:
		return 0;
	}
}

static inline unsigned int
cal_dsm(int pll, unsigned int num, unsigned int den)
{
	u64 res, clock = get_input_clock(pll);
	res = num * clock;
	do_div(res, den);
	return res;
}

static inline unsigned int
mash_dsm(int pll, unsigned int M, unsigned int N, unsigned int K)
{
	unsigned int num = ((N + 1) << 10) + K;
	unsigned int den = (M + 1) << 10;
	return cal_dsm(pll, num, den);
}

static inline unsigned int
ssff_dsm_1(int pll, unsigned int M, unsigned int N,	unsigned int K)
{
	unsigned int num = ((N + 1) << 11) + K + 512;
	unsigned int den = (M + 1) << 11;
	return cal_dsm(pll, num, den);
}

static inline unsigned int
ssff_dsm_2(int pll, unsigned int M, unsigned int N, unsigned int K)
{
	unsigned int num = K >= 512 ?
		((N + 1) << 12) + K - 512 : ((N + 1) << 12) + K + 3584;
	unsigned int den = (M + 1) << 12;
	return cal_dsm(pll, num, den);
}

static inline unsigned int
dsm(int pll, unsigned int M, unsigned int N, unsigned int K,
	unsigned int dsmsel, unsigned int phase_div_en)
{
	if (!dsmsel)
		return mash_dsm(pll, M, N, K);
	else if (!phase_div_en)
		return mash_dsm(pll, M, N, K);
	else
		return ssff_dsm_2(pll, M, N, K);
}

static inline unsigned int
lq_get_pll0_fosc(void)
{
	if (CGU_PLL0_BYPASS)
		return get_input_clock(0);
	else
		return !CGU_PLL0_CFG_FRAC_EN
			? dsm(0, CGU_PLL0_CFG_PLLM, CGU_PLL0_CFG_PLLN, 0, CGU_PLL0_CFG_DSMSEL,
				CGU_PLL0_PHASE_DIVIDER_ENABLE)
			: dsm(0, CGU_PLL0_CFG_PLLM, CGU_PLL0_CFG_PLLN, CGU_PLL0_CFG_PLLK,
				CGU_PLL0_CFG_DSMSEL, CGU_PLL0_PHASE_DIVIDER_ENABLE);
}

static unsigned int
lq_get_pll0_fdiv(void)
{
	unsigned int div = CGU_PLL2_CFG_INPUT_DIV + 1;
	return (lq_get_pll0_fosc() + (div >> 1)) / div;
}

unsigned int
lq_get_io_region_clock(void)
{
	unsigned int ret = lq_get_pll0_fosc();
	switch (lq_r32(LQ_CGU_PLL2_CFG) & CGU_SYS_DDR_SEL) {
	default:
	case 0:
		return (ret + 1) / 2;
	case 1:
		return (ret * 2 + 2) / 5;
	case 2:
		return (ret + 1) / 3;
	case 3:
		return (ret + 2) / 4;
	}
}
EXPORT_SYMBOL(lq_get_io_region_clock);

unsigned int
lq_get_fpi_bus_clock(int fpi)
{
	unsigned int ret = lq_get_io_region_clock();
	if ((fpi == 2) && (lq_r32(LQ_CGU_SYS) & CGU_SYS_FPI_SEL))
		ret >>= 1;
	return ret;
}
EXPORT_SYMBOL(lq_get_fpi_bus_clock);

unsigned int
lq_get_cpu_hz(void)
{
	switch (lq_r32(LQ_CGU_SYS) & 0xc)
	{
	case 0:
		return CLOCK_333M;
	case 4:
		return DDR_HZ;
	case 8:
		return DDR_HZ << 1;
	default:
		return DDR_HZ >> 1;
	}
}
EXPORT_SYMBOL(lq_get_cpu_hz);

unsigned int getCPUClock(void)
{
    return lq_get_cpu_hz()/1000000;
}

unsigned int
lq_get_fpi_hz(void)
{
	unsigned int ddr_clock = DDR_HZ;
	if (lq_r32(LQ_CGU_SYS) & 0x40)
		return ddr_clock >> 1;
	return ddr_clock;
}
EXPORT_SYMBOL(lq_get_fpi_hz);


