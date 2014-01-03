/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * Parts of this file are based on Ralink's 2.6.21 BSP
 *
 * Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <asm/mipsregs.h>
#include <asm/mach-ralink-openwrt/ralink_regs.h>
#include <asm/mach-ralink-openwrt/mt7620.h>
#include <asm/mach-ralink-openwrt/pinmux.h>

#include "common.h"

/* clock scaling */
#define CLKCFG_FDIV_MASK	0x1f00
#define CLKCFG_FDIV_USB_VAL	0x0300
#define CLKCFG_FFRAC_MASK	0x001f
#define CLKCFG_FFRAC_USB_VAL	0x0003

/* analog */
#define PMU0_CFG		0x88
#define PMU_SW_SET		BIT(28)
#define A_DCDC_EN		BIT(24)
#define A_SSC_PERI		BIT(19)
#define A_SSC_GEN		BIT(18)
#define A_SSC_M			0x3
#define A_SSC_S			16
#define A_DLY_M			0x7
#define A_DLY_S			8
#define A_VTUNE_M		0xff

/* digital */
#define PMU1_CFG		0x8C
#define DIG_SW_SEL		BIT(25)

/* does the board have sdram or ddram */
static int dram_type;

/* the pll dividers */
static u32 mt7620_clk_divider[] = { 2, 3, 4, 8 };

static struct rt2880_pmx_func i2c_grp[] =  { FUNC("i2c", 1, 1, 2) };
static struct rt2880_pmx_func spi_grp[] = { FUNC("spi", 1, 3, 4) };
static struct rt2880_pmx_func uartf_grp[] = {
	FUNC("uartf", MT7620_GPIO_MODE_UARTF, 7, 8),
	FUNC("pcm uartf", MT7620_GPIO_MODE_PCM_UARTF, 7, 8),
	FUNC("pcm i2s", MT7620_GPIO_MODE_PCM_I2S, 7, 8),
	FUNC("i2s uartf", MT7620_GPIO_MODE_I2S_UARTF, 7, 8),
	FUNC("pcm gpio", MT7620_GPIO_MODE_PCM_GPIO, 11, 4),
	FUNC("gpio uartf", MT7620_GPIO_MODE_GPIO_UARTF, 7, 4),
	FUNC("gpio i2s", MT7620_GPIO_MODE_GPIO_I2S, 7, 4),
};
static struct rt2880_pmx_func uartlite_grp[] = { FUNC("uartlite", 1, 15, 2) };
static struct rt2880_pmx_func wdt_grp[] = { FUNC("wdt", 1, 17, 1) };
static struct rt2880_pmx_func mdio_grp[] = { FUNC("mdio", 1, 22, 2) };
static struct rt2880_pmx_func rgmii1_grp[] = { FUNC("rgmii1", 1, 24, 12) };
static struct rt2880_pmx_func refclk_grp[] = { FUNC("spi refclk", 1, 37, 3) };
static struct rt2880_pmx_func ephy_grp[] = { FUNC("ephy", 1, 40, 5) };
static struct rt2880_pmx_func rgmii2_grp[] = { FUNC("rgmii2", 1, 60, 12) };
static struct rt2880_pmx_func wled_grp[] = { FUNC("wled", 1, 72, 1) };

static struct rt2880_pmx_group mt7620a_pinmux_data[] = {
	GRP("i2c", i2c_grp, 1, MT7620_GPIO_MODE_I2C),
	GRP("spi", spi_grp, 1, MT7620_GPIO_MODE_SPI),
	GRP("uartlite", uartlite_grp, 1, MT7620_GPIO_MODE_UART1),
	GRP("wdt", wdt_grp, 1, MT7620_GPIO_MODE_WDT),
	GRP("mdio", mdio_grp, 1, MT7620_GPIO_MODE_MDIO),
	GRP("rgmii1", rgmii1_grp, 1, MT7620_GPIO_MODE_RGMII1),
	GRP("spi refclk", refclk_grp, 1, MT7620_GPIO_MODE_SPI_REF_CLK),
	GRP("rgmii2", rgmii2_grp, 1, MT7620_GPIO_MODE_RGMII2),
	GRP("ephy", ephy_grp, 1, MT7620_GPIO_MODE_EPHY),
	GRP("wled", wled_grp, 1, MT7620_GPIO_MODE_WLED),
	GRP("uartf", uartf_grp, MT7620_GPIO_MODE_UART0_MASK,
		MT7620_GPIO_MODE_UART0_SHIFT),
	{ 0 }
};

void __init ralink_clk_init(void)
{
	unsigned long cpu_rate, sys_rate;
	u32 c0 = rt_sysc_r32(SYSC_REG_CPLL_CONFIG0);
	u32 c1 = rt_sysc_r32(SYSC_REG_CPLL_CONFIG1);
	u32 swconfig = (c0 >> CPLL_SW_CONFIG_SHIFT) & CPLL_SW_CONFIG_MASK;
	u32 cpu_clk = (c1 >> CPLL_CPU_CLK_SHIFT) & CPLL_CPU_CLK_MASK;

	if (cpu_clk) {
		cpu_rate = 480000000;
	} else if (!swconfig) {
		cpu_rate = 600000000;
	} else {
		u32 m = (c0 >> CPLL_MULT_RATIO_SHIFT) & CPLL_MULT_RATIO;
		u32 d = (c0 >> CPLL_DIV_RATIO_SHIFT) & CPLL_DIV_RATIO;

		cpu_rate = ((40 * (m + 24)) / mt7620_clk_divider[d]) * 1000000;
	}

	if (dram_type == SYSCFG0_DRAM_TYPE_SDRAM)
		sys_rate = cpu_rate / 4;
	else
		sys_rate = cpu_rate / 3;

	ralink_clk_add("cpu", cpu_rate);
	ralink_clk_add("10000100.timer", 40000000);
	ralink_clk_add("10000120.watchdog", 40000000);
	ralink_clk_add("10000500.uart", 40000000);
	ralink_clk_add("10000b00.spi", 40000000);
	ralink_clk_add("10000c00.uartlite", 40000000);

	if (IS_ENABLED(CONFIG_USB)) {
		/*
		 * When the CPU goes into sleep mode, the BUS clock will be too low for
		 * USB to function properly
		 */
		u32 val = rt_sysc_r32(SYSC_REG_CPU_SYS_CLKCFG);

		val &= ~(CLKCFG_FDIV_MASK | CLKCFG_FFRAC_MASK);
		val |= CLKCFG_FDIV_USB_VAL | CLKCFG_FFRAC_USB_VAL;

		rt_sysc_w32(val, SYSC_REG_CPU_SYS_CLKCFG);
	}
}

void __init ralink_of_remap(void)
{
	rt_sysc_membase = plat_of_remap_node("ralink,mt7620a-sysc");
	rt_memc_membase = plat_of_remap_node("ralink,mt7620a-memc");

	if (!rt_sysc_membase || !rt_memc_membase)
		panic("Failed to remap core resources");
}

void prom_soc_init(struct ralink_soc_info *soc_info)
{
	void __iomem *sysc = (void __iomem *) KSEG1ADDR(MT7620_SYSC_BASE);
	unsigned char *name = NULL;
	u32 n0;
	u32 n1;
	u32 rev;
	u32 cfg0;
	u32 pmu0;
	u32 pmu1;

	n0 = __raw_readl(sysc + SYSC_REG_CHIP_NAME0);
	n1 = __raw_readl(sysc + SYSC_REG_CHIP_NAME1);

	if (n0 == MT7620N_CHIP_NAME0 && n1 == MT7620N_CHIP_NAME1) {
		name = "MT7620N";
		soc_info->compatible = "ralink,mt7620n-soc";
	} else if (n0 == MT7620A_CHIP_NAME0 && n1 == MT7620A_CHIP_NAME1) {
		name = "MT7620A";
		soc_info->compatible = "ralink,mt7620a-soc";
	} else {
		panic("mt7620: unknown SoC, n0:%08x n1:%08x\n", n0, n1);
	}

	rev = __raw_readl(sysc + SYSC_REG_CHIP_REV);

	snprintf(soc_info->sys_type, RAMIPS_SYS_TYPE_LEN,
		"Ralink %s ver:%u eco:%u",
		name,
		(rev >> CHIP_REV_VER_SHIFT) & CHIP_REV_VER_MASK,
		(rev & CHIP_REV_ECO_MASK));

	cfg0 = __raw_readl(sysc + SYSC_REG_SYSTEM_CONFIG0);
	dram_type = (cfg0 >> SYSCFG0_DRAM_TYPE_SHIFT) & SYSCFG0_DRAM_TYPE_MASK;

	switch (dram_type) {
	case SYSCFG0_DRAM_TYPE_SDRAM:
		pr_info("Board has SDRAM\n");
		soc_info->mem_size_min = MT7620_SDRAM_SIZE_MIN;
		soc_info->mem_size_max = MT7620_SDRAM_SIZE_MAX;
		break;

	case SYSCFG0_DRAM_TYPE_DDR1:
		pr_info("Board has DDR1\n");
		soc_info->mem_size_min = MT7620_DDR1_SIZE_MIN;
		soc_info->mem_size_max = MT7620_DDR1_SIZE_MAX;
		break;

	case SYSCFG0_DRAM_TYPE_DDR2:
		pr_info("Board has DDR2\n");
		soc_info->mem_size_min = MT7620_DDR2_SIZE_MIN;
		soc_info->mem_size_max = MT7620_DDR2_SIZE_MAX;
		break;
	default:
		BUG();
	}
	soc_info->mem_base = MT7620_DRAM_BASE;

	pmu0 = __raw_readl(sysc + PMU0_CFG);
	pmu1 = __raw_readl(sysc + PMU1_CFG);

	pr_info("Analog PMU set to %s control\n",
		(pmu0 & PMU_SW_SET) ? ("sw") : ("hw"));
	pr_info("Digital PMU set to %s control\n",
		(pmu1 & DIG_SW_SEL) ? ("sw") : ("hw"));

	rt2880_pinmux_data = mt7620a_pinmux_data;
}
