/*
 * Copyright (c) 2014, 2015 The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>
#include <linux/reset-controller.h>

#include <dt-bindings/reset/qcom,gcc-ipq40xx.h>
#include <dt-bindings/clock/qcom,gcc-ipq40xx.h>

#include "common.h"
#include "clk-regmap.h"
#include "clk-rcg.h"
#include "clk-branch.h"
#include "reset.h"

#define SPARE_CLOCK_BRANCH_ENA_VOTE		0x0004
#define GCC_SPARE0_REG				0x0008
#define GCC_SPARE1_REG				0x000C
#define BLSP1_BCR				0x1000
#define BLSP1_AHB_CBCR				0x1008
#define BLSP1_QUP1_BCR				0x2000
#define BLSP1_QUP1_SPI_APPS_CBCR		0x2004
#define BLSP1_QUP1_I2C_APPS_CBCR		0x2008
#define BLSP1_QUP1_I2C_APPS_CMD_RCGR		0x200C
#define BLSP1_QUP1_I2C_APPS_CFG_RCGR		0x2010
#define BLSP1_QUP1_SPI_APPS_CMD_RCGR		0x2024
#define BLSP1_QUP1_SPI_APPS_CFG_RCGR		0x2028
#define BLSP1_QUP1_SPI_APPS_M			0x202C
#define BLSP1_QUP1_SPI_APPS_N			0x2030
#define BLSP1_QUP1_SPI_APPS_D			0x2034
#define BLSP1_UART1_BCR				0x2038
#define BLSP1_UART1_APPS_CBCR			0x203C
#define BLSP1_UART1_APPS_CMD_RCGR		0x2044
#define BLSP1_UART1_APPS_CFG_RCGR		0x2048
#define BLSP1_UART1_APPS_M			0x204C
#define BLSP1_UART1_APPS_N			0x2050
#define BLSP1_UART1_APPS_D			0x2054
#define BLSP1_QUP2_I2C_APPS_CMD_RCGR		0x3000
#define BLSP1_QUP2_I2C_APPS_CFG_RCGR		0x3004
#define BLSP1_QUP2_BCR				0x3008
#define BLSP1_QUP2_SPI_APPS_CBCR		0x300C
#define BLSP1_QUP2_I2C_APPS_CBCR		0x3010
#define BLSP1_QUP2_SPI_APPS_CMD_RCGR		0x3014
#define BLSP1_QUP2_SPI_APPS_CFG_RCGR		0x3018
#define BLSP1_QUP2_SPI_APPS_M			0x301C
#define BLSP1_QUP2_SPI_APPS_N			0x3020
#define BLSP1_QUP2_SPI_APPS_D			0x3024
#define BLSP1_UART2_BCR				0x3028
#define BLSP1_UART2_APPS_CBCR			0x302C
#define BLSP1_UART2_APPS_CMD_RCGR		0x3034
#define BLSP1_UART2_APPS_CFG_RCGR		0x3038
#define BLSP1_UART2_APPS_M			0x303C
#define BLSP1_UART2_APPS_N			0x3040
#define BLSP1_UART2_APPS_D			0x3044
#define BIMC_BCR				0x4000
#define TLMM_BCR				0x5000
#define TLMM_AHB_CBCR				0x5004
#define APCS_CLOCK_BRANCH_ENA_VOTE		0x6000
#define APCS_CLOCK_SLEEP_ENA_VOTE		0x6004
#define RPM_CLOCK_BRANCH_ENA_VOTE		0x7000
#define RPM_CLOCK_SLEEP_ENA_VOTE		0x7004
#define GP1_CBCR				0x8000
#define GP1_CMD_RCGR				0x8004
#define GP1_CFG_RCGR				0x8008
#define GP1_M					0x800C
#define GP1_N					0x8010
#define GP1_D					0x8014
#define GP2_CBCR				0x9000
#define GP2_CMD_RCGR				0x9004
#define GP2_CFG_RCGR				0x9008
#define GP2_M					0x900C
#define GP2_N					0x9010
#define GP2_D					0x9014
#define GP3_CBCR				0xA000
#define GP3_CMD_RCGR				0xA004
#define GP3_CFG_RCGR				0xA008
#define GP3_M					0xA00C
#define GP3_N					0xA010
#define GP3_D					0xA014
#define USB_BOOT_CLOCK_CTL			0xB000
#define TIC_MODE_APSS_BOOT			0xC000
#define RAW_SLEEP_CLK_CTRL			0xD000
#define IMEM_BCR				0xE000
#define IMEM_AXI_CBCR				0xE004
#define IMEM_CFG_AHB_CBCR			0xE008
#define APCS_HYP_CLOCK_BRANCH_ENA_VOTE		0xF000
#define APCS_HYP_CLOCK_SLEEP_ENA_VOTE		0xF004
#define WCSS_CLOCK_BRANCH_ENA_VOTE		0x10000
#define WCSS_CLOCK_SLEEP_ENA_VOTE		0x10004
#define GCC_SPARE2_REG				0x11000
#define GCC_SPARE3_REG				0x11004
#define FEPHY_125M_DLY_CMD_RCGR			0x12000
#define FEPHY_125M_DLY_CFG_RCGR			0x12004
#define ESS_BCR					0x12008
#define ESS_RST_CTRL				0x1200C
#define ESS_CBCR				0x12010
#define PRNG_BCR				0x13000
#define PRNG_AHB_CBCR				0x13004
#define BOOT_ROM_BCR				0x13008
#define BOOT_ROM_AHB_CBCR			0x1300C
#define APCS_TZ_CLOCK_BRANCH_ENA_VOTE		0x13014
#define APCS_TZ_CLOCK_SLEEP_ENA_VOTE		0x13018
#define PROC_HALT				0x1301C
#define RESET_DEBUG				0x14000
#define FLUSH_ETR_DEBUG_TIMER			0x15000
#define STOP_CAPTURE_DEBUG_TIMER		0x15004
#define RESET_STATUS				0x15008
#define SW_SRST					0x1500C
#define CRYPTO_BCR				0x16000
#define CRYPTO_CBCR				0x1601C
#define CRYPTO_AXI_CBCR				0x16020
#define CRYPTO_AHB_CBCR				0x16024
#define SDCC1_BCR				0x18000
#define SDCC1_APPS_CMD_RCGR			0x18004
#define SDCC1_APPS_CFG_RCGR			0x18008
#define SDCC1_APPS_CBCR				0x1800C
#define SDCC1_AHB_CBCR				0x18010
#define SDCC1_MISC				0x18014
#define APSS_AHB_MISC				0x19000
#define APSS_AHB_CBCR				0x19004
#define APSS_CMD_RCGR				0x1900C
#define APSS_CFG_RCGR				0x19010
#define APSS_AHB_CMD_RCGR			0x19014
#define APSS_AHB_CFG_RCGR			0x19018
#define SEC_CTRL_BCR				0x1A000
#define ACC_CMD_RCGR				0x1A004
#define ACC_CFG_RCGR				0x1A008
#define ACC_MISC				0x1A01C
#define SEC_CTRL_ACC_CBCR			0x1A020
#define SEC_CTRL_AHB_CBCR			0x1A024
#define SEC_CTRL_CBCR				0x1A028
#define SEC_CTRL_SENSE_CBCR			0x1A02C
#define SEC_CTRL_BOOT_ROM_PATCH_CBCR		0x1A030
#define SEC_CTRL_CMD_RCGR			0x1A034
#define SEC_CTRL_CFG_RCGR			0x1A038
#define SEC_CTRL_PHI90_CBCR			0x1A03C
#define AUDIO_CMD_RCGR				0x1B000
#define AUDIO_CFG_RCGR				0x1B004
#define AUDIO_BCR				0x1B008
#define AUDIO_PWM_CBCR				0x1B00C
#define AUDIO_AHB_CBCR				0x1B010
#define QPIC_BCR				0x1C000
#define QPIC_CBCR				0x1C004
#define QPIC_AHB_CBCR				0x1C008
#define QPIC_SLEEP_CBCR				0x1C00C
#define PCIE_BCR				0x1D000
#define PCIE_AXI_M_CBCR				0x1D004
#define PCIE_AXI_S_CBCR				0x1D008
#define PCIE_AHB_CBCR				0x1D00C
#define PCIE_RST_CTRL				0x1D010
#define PCIE_SLEEP_CBCR				0x1D014
#define USB_MOCK_UTMI_CMD_RCGR			0x1E000
#define USB_MOCK_UTMI_CFG_RCGR			0x1E004
#define USB2_BCR				0x1E008
#define USB2_MASTER_CBCR			0x1E00C
#define USB2_SLEEP_CBCR				0x1E010
#define USB2_MOCK_UTMI_CBCR			0x1E014
#define USB2_PHY_BCR				0x1E018
#define USB2_RST_CTRL				0x1E01C
#define USB2_MISC				0x1E020
#define USB3_BCR				0x1E024
#define USB3_MASTER_CBCR			0x1E028
#define USB3_SLEEP_CBCR				0x1E02C
#define USB3_MOCK_UTMI_CBCR			0x1E030
#define USB3_PHY_BCR				0x1E034
#define USB3_RST_CTRL				0x1E038
#define USB3_MISC				0x1E03C
#define WCSS2G_CMD_RCGR				0x1F000
#define WCSS2G_CFG_RCGR				0x1F004
#define WCSS2G_RST_CTRL				0x1F008
#define WCSS2G_REF_CBCR				0x1F00C
#define WCSS2G_RTC_CBCR				0x1F010
#define WCSS2G_MISC				0x1F014
#define WCSS5G_CMD_RCGR				0x20000
#define WCSS5G_CFG_RCGR				0x20004
#define WCSS5G_RST_CTRL				0x20008
#define WCSS5G_REF_CBCR				0x2000C
#define WCSS5G_RTC_CBCR				0x20010
#define WCSS5G_MISC				0x20014
#define SYSTEM_NOC_BCR				0x21000
#define SYSTEM_NOC_BFDCD_CMD_RCGR		0x21004
#define SYSTEM_NOC_BFDCD_CFG_RCGR		0x21008
#define SNOC_QOSGEN_EXTREF_CTL			0x2100C
#define SYS_NOC_AXI_CBCR			0x21010
#define SNOC_PCNOC_AHB_CBCR			0x2101C
#define SYS_NOC_AT_CBCR				0x21020
#define PCNOC_BFDCD_CMD_RCGR			0x21024
#define PCNOC_BFDCD_CFG_RCGR			0x21028
#define PCNOC_BCR				0x2102C
#define PCNOC_AHB_CBCR				0x21030
#define PCNOC_AT_CBCR				0x21034
#define DCD_BCR					0x21038
#define DCD_XO_CBCR				0x2103C
#define SNOC_DCD_CONFIG				0x21040
#define SNOC_DCD_HYSTERESIS_CNT			0x21044
#define PCNOC_DCD_CONFIG			0x21048
#define PCNOC_DCD_HYSTERESIS_CNT		0x2104C
#define GCC_AHB_CBCR				0x21050
#define GCC_XO_CMD_RCGR				0x21054
#define GCC_XO_CBCR				0x21058
#define GCC_XO_DIV4_CBCR			0x2105C
#define GCC_IM_SLEEP_CBCR			0x21060
#define SNOC_BUS_TIMEOUT0_BCR			0x21064
#define SNOC_BUS_TIMEOUT1_BCR			0x2106C
#define SNOC_BUS_TIMEOUT2_BCR			0x21074
#define SNOC_BUS_TIMEOUT3_BCR			0x2107C
#define PCNOC_BUS_TIMEOUT0_BCR			0x21084
#define PCNOC_BUS_TIMEOUT1_BCR			0x2108C
#define PCNOC_BUS_TIMEOUT2_BCR			0x21094
#define PCNOC_BUS_TIMEOUT3_BCR			0x2109C
#define PCNOC_BUS_TIMEOUT4_BCR			0x210A4
#define PCNOC_BUS_TIMEOUT5_BCR			0x210AC
#define PCNOC_BUS_TIMEOUT6_BCR			0x210B4
#define PCNOC_BUS_TIMEOUT7_BCR			0x210BC
#define PCNOC_BUS_TIMEOUT8_BCR			0x210C4
#define PCNOC_BUS_TIMEOUT9_BCR			0x210CC
#define GCC_DEBUG_CLK_CTL			0x210D4
#define CLOCK_FRQ_MEASURE_CTL			0x210D8
#define PLLTEST_PAD_CFG				0x210E0
#define SYSTEM_NOC_125M_BFDCD_CMD_RCGR		0x210E4
#define SYSTEM_NOC_125M_BFDCD_CFG_RCGR		0x210E8
#define SYS_NOC_125M_CBCR			0x210EC
#define GCC_SLEEP_CMD_RCGR			0x210F0
#define TCSR_BCR				0x22000
#define TCSR_AHB_CBCR				0x22004
#define MPM_BCR					0x24000
#define MPM_MISC				0x24004
#define MPM_AHB_CBCR				0x24008
#define MPM_SLEEP_CBCR				0x2400C
#define SPDM_BCR				0x25000
#define MDIO_AHB_CBCR				0x26000


static const u8 gcc_xo_200_500_map[] = {
	[0]		= 0,
	[1]		= 1,
	[2]		= 2,
};

static const char *gcc_xo_200_500[] = {
	"xo",
	"fepll200",
	"fepll500",
};

static const u8 gcc_xo_200_map[] = {
	[0]		= 0,
	[1]		= 1,
};

static const char *gcc_xo_200[] = {
	"xo",
	"fepll200",
};

static const u8 gcc_xo_200_spi_map[] = {
	[0]		= 0,
	[1]		= 2,
};

static const char *gcc_xo_200_spi[] = {
	"xo",
	"fepll200",
};

static const u8 gcc_xo_sdcc1_500_map[] = {
	[0]		= 0,
	[1]		= 1,
	[2]		= 2,
};

static const char *gcc_xo_sdcc1_500[] = {
	"xo",
	"ddrpll",
	"fepll500",
};


static const u8 gcc_xo_wcss2g_map[] = {
	[0]		= 0,
	[1]		= 1,
};

static const char *gcc_xo_wcss2g[] = {
	"xo",
	"fepllwcss2g",
};

static const u8 gcc_xo_wcss5g_map[] = {
	[0]		= 0,
	[1]		= 1,
};

static const char *gcc_xo_wcss5g[] = {
	"xo",
	"fepllwcss5g",
};

static const u8 gcc_xo_125_dly_map[] = {
	[0]		= 0,
	[1]		= 1,
};

static const char *gcc_xo_125_dly[] = {
	"xo",
	"fepll125dly",
};


static const u8 gcc_xo_ddr_500_200_map[] = {
	[0]		= 0,
	[1]		= 3,
	[2]		= 2,
	[3]		= 1,
};

static const char *gcc_xo_ddr_500_200[] = {
	"xo",
	"fepll200",
	"fepll500",
	"ddrpllapss",
};


static int clk_dummy_is_enabled(struct clk_hw *hw)
{
	return 1;
};

static int clk_dummy_enable(struct clk_hw *hw)
{
	return 0;
};

static void clk_dummy_disable(struct clk_hw *hw)
{
	return;
};

static u8 clk_dummy_get_parent(struct clk_hw *hw)
{
	return 0;
};

static int clk_dummy_set_parent(struct clk_hw *hw, u8 index)
{
	return 0;
};

static int clk_dummy_set_rate(struct clk_hw *hw, unsigned long rate,
			      unsigned long parent_rate)
{
	return 0;
};

static long clk_dummy_determine_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long *p_rate, struct clk **p)
{
	return rate;
};

static unsigned long clk_dummy_recalc_rate(struct clk_hw *hw,
					   unsigned long parent_rate)
{
	return parent_rate;
};

const struct clk_ops clk_ipq40xx_dummy_ops = {
	.is_enabled = clk_dummy_is_enabled,
	.enable = clk_dummy_enable,
	.disable = clk_dummy_disable,
	.get_parent = clk_dummy_get_parent,
	.set_parent = clk_dummy_set_parent,
	.set_rate = clk_dummy_set_rate,
	.recalc_rate = clk_dummy_recalc_rate,
	.determine_rate = clk_dummy_determine_rate,
};

static struct clk_regmap dummy = {
	.hw.init = &(struct clk_init_data){
		.name = "dummy_clk_src",
		.parent_names = (const char *[]){ "xo"},
		.num_parents = 1,
		.ops = &clk_ipq40xx_dummy_ops,
	},
};

#define F(f, s, h, m, n) { (f), (s), (2 * (h) - 1), (m), (n) }
#define P_XO 0
#define FE_PLL_200 1
#define FE_PLL_500 2
#define DDRC_PLL_666  3

#define DDRC_PLL_666_SDCC  1
#define FE_PLL_125_DLY 1

#define FE_PLL_WCSS2G 1
#define FE_PLL_WCSS5G 1

static const struct freq_tbl ftbl_gcc_audio_pwm_clk[] = {
	F(48000000, P_XO, 1, 0, 0),
	F(200000000, FE_PLL_200, 1, 0, 0),
	{ }
};

static struct clk_rcg2 audio_clk_src = {
	.cmd_rcgr = AUDIO_CMD_RCGR,
	.hid_width = 5,
	.parent_map = gcc_xo_200_map,
	.freq_tbl = ftbl_gcc_audio_pwm_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "audio_clk_src",
		.parent_names = gcc_xo_200,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,

},
};

static struct clk_branch gcc_audio_ahb_clk = {
	.halt_reg = AUDIO_AHB_CBCR,
	.clkr = {
		.enable_reg = AUDIO_AHB_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_audio_ahb_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.flags = CLK_SET_RATE_PARENT,
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_audio_pwm_clk = {
	.halt_reg = AUDIO_PWM_CBCR,
	.clkr = {
		.enable_reg = AUDIO_PWM_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_audio_pwm_clk",
			.parent_names = (const char *[]){
				"audio_clk_src",
			},
			.flags = CLK_SET_RATE_PARENT,
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};


static const struct freq_tbl ftbl_gcc_blsp1_qup1_2_i2c_apps_clk[] = {
	F(19200000, P_XO, 1, 2, 5),
	F(24000000, P_XO, 1, 1, 2),
	{ }
};

static struct clk_rcg2 blsp1_qup1_i2c_apps_clk_src = {
	.cmd_rcgr = BLSP1_QUP1_I2C_APPS_CMD_RCGR,
	.hid_width = 5,
	.parent_map = gcc_xo_200_map,
	.freq_tbl = ftbl_gcc_blsp1_qup1_2_i2c_apps_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_qup1_i2c_apps_clk_src",
		.parent_names = gcc_xo_200,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
	},
};


static struct clk_branch gcc_blsp1_qup1_i2c_apps_clk = {
	.halt_reg = BLSP1_QUP1_I2C_APPS_CBCR,
	.clkr = {
		.enable_reg = BLSP1_QUP1_I2C_APPS_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_blsp1_qup1_i2c_apps_clk",
			.parent_names = (const char *[]){
				"blsp1_qup1_i2c_apps_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};

static struct clk_rcg2 blsp1_qup2_i2c_apps_clk_src = {
	.cmd_rcgr = BLSP1_QUP2_I2C_APPS_CMD_RCGR,
	.hid_width = 5,
	.parent_map = gcc_xo_200_map,
	.freq_tbl = ftbl_gcc_blsp1_qup1_2_i2c_apps_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_qup2_i2c_apps_clk_src",
		.parent_names = gcc_xo_200,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_branch gcc_blsp1_qup2_i2c_apps_clk = {
	.halt_reg = BLSP1_QUP2_I2C_APPS_CBCR,
	.clkr = {
		.enable_reg = BLSP1_QUP2_I2C_APPS_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_blsp1_qup2_i2c_apps_clk",
			.parent_names = (const char *[]){
				"blsp1_qup2_i2c_apps_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};

static const struct freq_tbl ftbl_gcc_blsp1_qup1_2_spi_apps_clk[] = {
	F(960000, P_XO, 12, 1, 4),
	F(4800000, P_XO, 1, 1, 10),
	F(9600000, P_XO, 1, 1, 5),
	F(15000000, P_XO, 1, 1, 3),
	F(19200000, P_XO, 1, 2, 5),
	F(24000000, P_XO, 1, 1, 2),
	F(48000000, P_XO, 1, 0, 0),
	{ }
};

static struct clk_rcg2 blsp1_qup1_spi_apps_clk_src = {
	.cmd_rcgr = BLSP1_QUP1_SPI_APPS_CMD_RCGR,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = gcc_xo_200_spi_map,
	.freq_tbl = ftbl_gcc_blsp1_qup1_2_spi_apps_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_qup1_spi_apps_clk_src",
		.parent_names = gcc_xo_200_spi,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
	},
};
static struct clk_branch gcc_blsp1_qup1_spi_apps_clk = {
	.halt_reg = BLSP1_QUP1_SPI_APPS_CBCR,
	.clkr = {
		.enable_reg = BLSP1_QUP1_SPI_APPS_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_blsp1_qup1_spi_apps_clk",
			.parent_names = (const char *[]){
				"blsp1_qup1_spi_apps_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};


static struct clk_rcg2 blsp1_qup2_spi_apps_clk_src = {
	.cmd_rcgr = BLSP1_QUP2_SPI_APPS_CMD_RCGR,
	.mnd_width = 8,
	.hid_width = 5,
	.freq_tbl = ftbl_gcc_blsp1_qup1_2_spi_apps_clk,
	.parent_map = gcc_xo_200_spi_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_qup2_spi_apps_clk_src",
		.parent_names = gcc_xo_200_spi,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_branch gcc_blsp1_qup2_spi_apps_clk = {
	.halt_reg = BLSP1_QUP2_SPI_APPS_CBCR,
	.clkr = {
		.enable_reg = BLSP1_QUP2_SPI_APPS_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_blsp1_qup2_spi_apps_clk",
			.parent_names = (const char *[]){
				"blsp1_qup2_spi_apps_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};

static const struct freq_tbl ftbl_gcc_blsp1_uart1_2_apps_clk[] = {
	F(1843200, FE_PLL_200, 1, 144, 15625),
	F(3686400, FE_PLL_200, 1, 288, 15625),
	F(7372800, FE_PLL_200, 1, 576, 15625),
	F(14745600, FE_PLL_200, 1, 1152, 15625),
	F(16000000, FE_PLL_200, 1, 2, 25),
	F(24000000, P_XO, 1, 1, 2),
	F(32000000, FE_PLL_200, 1, 4, 25),
	F(40000000, FE_PLL_200, 1, 1, 5),
	F(46400000, FE_PLL_200, 1, 29, 125),
	F(48000000, P_XO, 1, 0, 0),
	{ }
};

static struct clk_rcg2 blsp1_uart1_apps_clk_src = {
	.cmd_rcgr = BLSP1_UART1_APPS_CMD_RCGR,
	.mnd_width = 16,
	.hid_width = 5,
	.freq_tbl = ftbl_gcc_blsp1_uart1_2_apps_clk,
	.parent_map = gcc_xo_200_spi_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_uart1_apps_clk_src",
		.parent_names = gcc_xo_200_spi,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_branch gcc_blsp1_uart1_apps_clk = {
	.halt_reg = BLSP1_UART1_APPS_CBCR,
	.clkr = {
		.enable_reg = BLSP1_UART1_APPS_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_blsp1_uart1_apps_clk",
			.parent_names = (const char *[]){
				"blsp1_uart1_apps_clk_src",
			},
			.flags = CLK_SET_RATE_PARENT,
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};


static struct clk_rcg2 blsp1_uart2_apps_clk_src = {
	.cmd_rcgr = BLSP1_UART2_APPS_CMD_RCGR,
	.mnd_width = 16,
	.hid_width = 5,
	.freq_tbl = ftbl_gcc_blsp1_uart1_2_apps_clk,
	.parent_map = gcc_xo_200_spi_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "blsp1_uart2_apps_clk_src",
		.parent_names = gcc_xo_200_spi,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_branch gcc_blsp1_uart2_apps_clk = {
	.halt_reg = BLSP1_UART2_APPS_CBCR,
	.clkr = {
		.enable_reg = BLSP1_UART2_APPS_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_blsp1_uart2_apps_clk",
			.parent_names = (const char *[]){
				"blsp1_uart2_apps_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};


static const struct freq_tbl ftbl_gcc_gp_clk[] = {
	F(1250000,  FE_PLL_200, 1, 16, 0),
	F(2500000,  FE_PLL_200, 1,  8, 0),
	F(5000000,  FE_PLL_200, 1,  4, 0),
	{ }
};

static struct clk_rcg2 gp1_clk_src = {
	.cmd_rcgr = GP1_CMD_RCGR,
	.mnd_width = 8,
	.hid_width = 5,
	.freq_tbl = ftbl_gcc_gp_clk,
	.parent_map = gcc_xo_200_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gp1_clk_src",
		.parent_names = gcc_xo_200,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_branch gcc_gp1_clk = {
	.halt_reg = GP1_CBCR,
	.clkr = {
		.enable_reg = GP1_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_gp1_clk",
			.parent_names = (const char *[]){
				"gp1_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};


static struct clk_rcg2 gp2_clk_src = {
	.cmd_rcgr = GP2_CMD_RCGR,
	.mnd_width = 8,
	.hid_width = 5,
	.freq_tbl = ftbl_gcc_gp_clk,
	.parent_map = gcc_xo_200_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gp2_clk_src",
		.parent_names = gcc_xo_200,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_branch gcc_gp2_clk = {
	.halt_reg = GP2_CBCR,
	.clkr = {
		.enable_reg = GP2_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_gp2_clk",
			.parent_names = (const char *[]){
				"gp2_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};


static struct clk_rcg2 gp3_clk_src = {
	.cmd_rcgr = GP3_CMD_RCGR,
	.mnd_width = 8,
	.hid_width = 5,
	.freq_tbl = ftbl_gcc_gp_clk,
	.parent_map = gcc_xo_200_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gp3_clk_src",
		.parent_names = gcc_xo_200,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_branch gcc_gp3_clk = {
	.halt_reg = GP3_CBCR,
	.clkr = {
		.enable_reg = GP3_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_gp3_clk",
			.parent_names = (const char *[]){
				"gp3_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};



static const struct freq_tbl ftbl_gcc_sdcc1_apps_clk[] = {
	F(193000000, DDRC_PLL_666_SDCC,		1,  0, 0),
	{ }
};


static struct clk_cdiv_rcg2  sdcc1_apps_clk_src = {
	.cmd_rcgr = SDCC1_APPS_CMD_RCGR,
	.hid_width = 5,
	.freq_tbl = ftbl_gcc_sdcc1_apps_clk,
	.parent_map = gcc_xo_sdcc1_500_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "sdcc1_apps_clk_src",
		.parent_names = gcc_xo_sdcc1_500,
		.num_parents = 3,
		.ops = &clk_rcg2_ops,
		.flags = CLK_SET_RATE_PARENT,
	},
};




/*APPS CLOCKS*/
static const struct freq_tbl ftbl_gcc_apps_clk[] = {
	F(48000000 , P_XO,	   1, 0, 0),
	F(200000000, FE_PLL_200,   1, 0, 0),
	F(500000000, FE_PLL_500,   1, 0, 0),
	F(626000000, DDRC_PLL_666, 1, 0, 0),
	{ }
};

static struct clk_rcg2 apps_clk_src = {
	.cmd_rcgr = APSS_CMD_RCGR,
	.hid_width = 5,
	.freq_tbl = ftbl_gcc_apps_clk,
	.parent_map = gcc_xo_ddr_500_200_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "apps_clk_src",
		.parent_names = gcc_xo_ddr_500_200,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static const struct freq_tbl ftbl_gcc_apps_ahb_clk[] = {
	F(48000000 , P_XO,	   1, 0, 0),
	F(100000000, FE_PLL_200,   2, 0, 0),
	{ }
};

static struct clk_rcg2 apps_ahb_clk_src = {
	.cmd_rcgr = APSS_AHB_CMD_RCGR,
	.hid_width = 5,
	.parent_map = gcc_xo_200_500_map,
	.freq_tbl = ftbl_gcc_apps_ahb_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "apps_ahb_clk_src",
		.parent_names = gcc_xo_200_500,
		.num_parents = 3,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_branch gcc_apss_ahb_clk = {
	.halt_reg = APSS_AHB_CBCR,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
		.enable_mask = BIT(14),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_apss_ahb_clk",
			.parent_names = (const char *[]){
				"apps_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};


static struct clk_branch gcc_blsp1_ahb_clk = {
	.halt_reg = BLSP1_AHB_CBCR,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
		.enable_mask = BIT(10),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_blsp1_ahb_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};


static struct clk_branch gcc_boot_rom_ahb_clk = {
	.halt_reg = BOOT_ROM_AHB_CBCR,
	.clkr = {
		.enable_reg = BOOT_ROM_AHB_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_boot_rom_ahb_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};

static struct clk_branch gcc_crypto_ahb_clk = {
	.halt_reg = CRYPTO_AHB_CBCR,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
		.enable_reg = CRYPTO_AHB_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_crypto_ahb_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_crypto_axi_clk = {
	.halt_reg = CRYPTO_AXI_CBCR,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
		.enable_mask = BIT(1),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_crypto_axi_clk",
			.parent_names = (const char *[]){
				"fepll125",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};



static struct clk_branch gcc_crypto_clk = {
	.halt_reg = CRYPTO_CBCR,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
		.enable_mask = BIT(2),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_crypto_clk",
			.parent_names = (const char *[]){
				"fepll125",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};



static struct clk_branch gcc_ess_clk = {
	.halt_reg = ESS_CBCR,
	.clkr = {
		.enable_reg = ESS_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_ess_clk",
			.parent_names = (const char *[]){
				"fephy_125m_dly_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};


static struct clk_branch gcc_imem_axi_clk = {
	.halt_reg = IMEM_AXI_CBCR,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
		.enable_mask = BIT(17),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_imem_axi_clk",
			.parent_names = (const char *[]){
				"fepll200",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_imem_cfg_ahb_clk = {
	.halt_reg = IMEM_CFG_AHB_CBCR,
	.clkr = {
		.enable_reg = IMEM_CFG_AHB_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_imem_cfg_ahb_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};


static struct clk_branch gcc_pcie_ahb_clk = {
	.halt_reg = PCIE_AHB_CBCR,
	.clkr = {
		.enable_reg = PCIE_AHB_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_pcie_ahb_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pcie_axi_m_clk = {
	.halt_reg = PCIE_AXI_M_CBCR,
	.clkr = {
		.enable_reg = PCIE_AXI_M_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_pcie_axi_m_clk",
			.parent_names = (const char *[]){
				"fepll200",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pcie_axi_s_clk = {
	.halt_reg = PCIE_AXI_S_CBCR,
	.clkr = {
		.enable_reg = PCIE_AXI_S_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_pcie_axi_s_clk",
			.parent_names = (const char *[]){
				"fepll200",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_pcnoc_ahb_clk = {
	.halt_reg = PCNOC_AHB_CBCR,
	.clkr = {
		.enable_reg = PCNOC_AHB_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_pcnoc_ahb_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};



static struct clk_branch gcc_prng_ahb_clk = {
	.halt_reg = PRNG_AHB_CBCR,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
		.enable_mask = BIT(8),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_prng_ahb_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};


static struct clk_branch gcc_qpic_ahb_clk = {
	.halt_reg = QPIC_AHB_CBCR,
	.clkr = {
		.enable_reg = QPIC_AHB_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_qpic_ahb_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_qpic_clk = {
	.halt_reg = QPIC_CBCR,
	.clkr = {
		.enable_reg = QPIC_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_qpic_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc1_ahb_clk = {
	.halt_reg = SDCC1_AHB_CBCR,
	.clkr = {
		.enable_reg = SDCC1_AHB_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_sdcc1_ahb_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_sdcc1_apps_clk = {
	.halt_reg = SDCC1_APPS_CBCR,
	.clkr = {
		.enable_reg = SDCC1_APPS_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_sdcc1_apps_clk",
			.parent_names = (const char *[]){
				"sdcc1_apps_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};



static struct clk_branch gcc_tcsr_ahb_clk = {
	.halt_reg = TCSR_AHB_CBCR,
	.clkr = {
		.enable_reg = TCSR_AHB_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_tcsr_ahb_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_tlmm_ahb_clk = {
	.halt_reg = TLMM_AHB_CBCR,
	.halt_check = BRANCH_HALT_VOTED,
	.clkr = {
		.enable_reg = APCS_CLOCK_BRANCH_ENA_VOTE,
		.enable_mask = BIT(5),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_tlmm_ahb_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb2_master_clk = {
	.halt_reg = USB2_MASTER_CBCR,
	.clkr = {
		.enable_reg = USB2_MASTER_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_usb2_master_clk",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb2_sleep_clk = {
	.halt_reg = USB2_SLEEP_CBCR,
	.clkr = {
		.enable_reg = USB2_SLEEP_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_usb2_sleep_clk",
			.parent_names = (const char *[]){
				"gcc_sleep_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb2_mock_utmi_clk = {
	.halt_reg = USB2_MOCK_UTMI_CBCR,
	.clkr = {
		.enable_reg = USB2_MOCK_UTMI_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_usb2_mock_utmi_clk",
			.parent_names = (const char *[]){
				"usb30_mock_utmi_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};


static const struct freq_tbl ftbl_gcc_usb30_mock_utmi_clk[] = {
	F(2000000, FE_PLL_200, 10, 0, 0),
	{ }
};

static struct clk_rcg2 usb30_mock_utmi_clk_src = {
	.cmd_rcgr = USB_MOCK_UTMI_CMD_RCGR,
	.hid_width = 5,
	.parent_map = gcc_xo_200_map,
	.freq_tbl = ftbl_gcc_usb30_mock_utmi_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "usb30_mock_utmi_clk_src",
		.parent_names = gcc_xo_200,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
	},
};


static struct clk_branch gcc_usb3_master_clk = {
	.halt_reg = USB3_MASTER_CBCR,
	.clkr = {
		.enable_reg = USB3_MASTER_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_usb3_master_clk",
			.parent_names = (const char *[]){
				"fepll125",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb3_sleep_clk = {
	.halt_reg = USB3_SLEEP_CBCR,
	.clkr = {
		.enable_reg = USB3_SLEEP_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_usb3_sleep_clk",
			.parent_names = (const char *[]){
				"gcc_sleep_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gcc_usb3_mock_utmi_clk = {
	.halt_reg = USB3_MOCK_UTMI_CBCR,
	.clkr = {
		.enable_reg = USB3_MOCK_UTMI_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_usb3_mock_utmi_clk",
			.parent_names = (const char *[]){
				"usb30_mock_utmi_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};

static const struct freq_tbl ftbl_gcc_fephy_dly_clk[] = {
	F(125000000, FE_PLL_125_DLY, 1, 0, 0),
	{ }
};

static struct clk_rcg2 fephy_125m_dly_clk_src = {
	.cmd_rcgr = FEPHY_125M_DLY_CMD_RCGR,
	.hid_width = 5,
	.parent_map = gcc_xo_125_dly_map,
	.freq_tbl = ftbl_gcc_fephy_dly_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "fephy_125m_dly_clk_src",
		.parent_names = gcc_xo_125_dly,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
	},
};


static const struct freq_tbl ftbl_gcc_wcss2g_clk[] = {
	F(48000000, P_XO, 1, 0, 0),
	F(250000000, FE_PLL_WCSS2G, 1, 0, 0),
	{ }
};

static struct clk_rcg2 wcss2g_clk_src = {
	.cmd_rcgr = WCSS2G_CMD_RCGR,
	.hid_width = 5,
	.freq_tbl = ftbl_gcc_wcss2g_clk,
	.parent_map = gcc_xo_wcss2g_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "wcss2g_clk_src",
		.parent_names = gcc_xo_wcss2g,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_branch gcc_wcss2g_clk = {
	.halt_reg = WCSS2G_REF_CBCR,
	.clkr = {
		.enable_reg = WCSS2G_REF_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_wcss2g_clk",
			.parent_names = (const char *[]){
				"wcss2g_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};


static struct clk_branch gcc_wcss2g_ref_clk = {
	.halt_reg = WCSS2G_REF_CBCR,
	.clkr = {
		.enable_reg = WCSS2G_REF_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_wcss2g_ref_clk",
			.parent_names = (const char *[]){
				"xo",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};


static struct clk_branch gcc_wcss2g_rtc_clk = {
	.halt_reg = WCSS2G_RTC_CBCR,
	.clkr = {
		.enable_reg = WCSS2G_RTC_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_wcss2g_rtc_clk",
			.parent_names = (const char *[]){
				"wifi_rtc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};




static const struct freq_tbl ftbl_gcc_wcss5g_clk[] = {
	F(48000000, P_XO, 1, 0, 0),
	F(250000000, FE_PLL_WCSS5G, 1, 0, 0),
	{ }
};

static struct clk_rcg2 wcss5g_clk_src = {
	.cmd_rcgr = WCSS5G_CMD_RCGR,
	.hid_width = 5,
	.parent_map = gcc_xo_wcss5g_map,
	.freq_tbl = ftbl_gcc_wcss5g_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "wcss5g_clk_src",
		.parent_names = gcc_xo_wcss5g,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_branch gcc_wcss5g_clk = {
	.halt_reg = WCSS5G_REF_CBCR,
	.clkr = {
		.enable_reg = WCSS5G_REF_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_wcss5g_clk",
			.parent_names = (const char *[]){
				"wcss5g_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};

static struct clk_branch gcc_wcss5g_ref_clk = {
	.halt_reg = WCSS5G_REF_CBCR,
	.clkr = {
		.enable_reg = WCSS5G_REF_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_wcss5g_ref_clk",
			.parent_names = (const char *[]){
				"xo",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};

static struct clk_branch gcc_wcss5g_rtc_clk = {
	.halt_reg = WCSS5G_RTC_CBCR,
	.clkr = {
		.enable_reg = WCSS5G_RTC_CBCR,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gcc_wcss5g_rtc_clk",
			.parent_names = (const char *[]){
				"wifi_rtc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};




static struct clk_regmap *gcc_ipq40xx_clocks[] = {
	[GCC_DUMMY_CLK] = &dummy,
	[AUDIO_CLK_SRC] = &audio_clk_src.clkr,
	[BLSP1_QUP1_I2C_APPS_CLK_SRC] = &blsp1_qup1_i2c_apps_clk_src.clkr,
	[BLSP1_QUP1_SPI_APPS_CLK_SRC] = &blsp1_qup1_spi_apps_clk_src.clkr,
	[BLSP1_QUP2_I2C_APPS_CLK_SRC] = &blsp1_qup2_i2c_apps_clk_src.clkr,
	[BLSP1_QUP2_SPI_APPS_CLK_SRC] = &blsp1_qup2_spi_apps_clk_src.clkr,
	[BLSP1_UART1_APPS_CLK_SRC] = &blsp1_uart1_apps_clk_src.clkr,
	[BLSP1_UART2_APPS_CLK_SRC] = &blsp1_uart2_apps_clk_src.clkr,
	[GCC_USB3_MOCK_UTMI_CLK_SRC] = &usb30_mock_utmi_clk_src.clkr,
	[GCC_APPS_CLK_SRC] = &apps_clk_src.clkr,
	[GCC_APPS_AHB_CLK_SRC] = &apps_ahb_clk_src.clkr,
	[GP1_CLK_SRC] = &gp1_clk_src.clkr,
	[GP2_CLK_SRC] = &gp2_clk_src.clkr,
	[GP3_CLK_SRC] = &gp3_clk_src.clkr,
	[SDCC1_APPS_CLK_SRC] = &sdcc1_apps_clk_src.clkr,
	[FEPHY_125M_DLY_CLK_SRC] = &fephy_125m_dly_clk_src.clkr,
	[WCSS2G_CLK_SRC] = &wcss2g_clk_src.clkr,
	[WCSS5G_CLK_SRC] = &wcss5g_clk_src.clkr,
	[GCC_APSS_AHB_CLK] = &gcc_apss_ahb_clk.clkr,
	[GCC_AUDIO_AHB_CLK] = &gcc_audio_ahb_clk.clkr,
	[GCC_AUDIO_PWM_CLK] = &gcc_audio_pwm_clk.clkr,
	[GCC_BLSP1_AHB_CLK] = &gcc_blsp1_ahb_clk.clkr,
	[GCC_BLSP1_QUP1_I2C_APPS_CLK] = &gcc_blsp1_qup1_i2c_apps_clk.clkr,
	[GCC_BLSP1_QUP1_SPI_APPS_CLK] = &gcc_blsp1_qup1_spi_apps_clk.clkr,
	[GCC_BLSP1_QUP2_I2C_APPS_CLK] = &gcc_blsp1_qup2_i2c_apps_clk.clkr,
	[GCC_BLSP1_QUP2_SPI_APPS_CLK] = &gcc_blsp1_qup2_spi_apps_clk.clkr,
	[GCC_BLSP1_UART1_APPS_CLK] = &gcc_blsp1_uart1_apps_clk.clkr,
	[GCC_BLSP1_UART2_APPS_CLK] = &gcc_blsp1_uart2_apps_clk.clkr,
	[GCC_GP1_CLK] = &gcc_gp1_clk.clkr,
	[GCC_GP2_CLK] = &gcc_gp2_clk.clkr,
	[GCC_GP3_CLK] = &gcc_gp3_clk.clkr,
	[GCC_BOOT_ROM_AHB_CLK] = &gcc_boot_rom_ahb_clk.clkr,
	[GCC_CRYPTO_AHB_CLK] = &gcc_crypto_ahb_clk.clkr,
	[GCC_CRYPTO_AXI_CLK] = &gcc_crypto_axi_clk.clkr,
	[GCC_CRYPTO_CLK] = &gcc_crypto_clk.clkr,
	[GCC_ESS_CLK] = &gcc_ess_clk.clkr,
	[GCC_IMEM_AXI_CLK] = &gcc_imem_axi_clk.clkr,
	[GCC_IMEM_CFG_AHB_CLK] = &gcc_imem_cfg_ahb_clk.clkr,
	[GCC_PCIE_AHB_CLK] = &gcc_pcie_ahb_clk.clkr,
	[GCC_PCIE_AXI_M_CLK] = &gcc_pcie_axi_m_clk.clkr,
	[GCC_PCIE_AXI_S_CLK] = &gcc_pcie_axi_s_clk.clkr,
	[GCC_PCNOC_AHB_CLK] = &gcc_pcnoc_ahb_clk.clkr,
	[GCC_PRNG_AHB_CLK] = &gcc_prng_ahb_clk.clkr,
	[GCC_QPIC_AHB_CLK] = &gcc_qpic_ahb_clk.clkr,
	[GCC_QPIC_CLK] = &gcc_qpic_clk.clkr,
	[GCC_SDCC1_AHB_CLK] = &gcc_sdcc1_ahb_clk.clkr,
	[GCC_SDCC1_APPS_CLK] = &gcc_sdcc1_apps_clk.clkr,
	[GCC_TCSR_AHB_CLK] = &gcc_tcsr_ahb_clk.clkr,
	[GCC_TLMM_AHB_CLK] = &gcc_tlmm_ahb_clk.clkr,
	[GCC_USB2_MASTER_CLK] = &gcc_usb2_master_clk.clkr,
	[GCC_USB2_SLEEP_CLK] = &gcc_usb2_sleep_clk.clkr,
	[GCC_USB2_MOCK_UTMI_CLK] = &gcc_usb2_mock_utmi_clk.clkr,
	[GCC_USB3_MASTER_CLK] = &gcc_usb3_master_clk.clkr,
	[GCC_USB3_SLEEP_CLK] = &gcc_usb3_sleep_clk.clkr,
	[GCC_USB3_MOCK_UTMI_CLK] = &gcc_usb3_mock_utmi_clk.clkr,
	[GCC_WCSS2G_CLK] = &gcc_wcss2g_clk.clkr,
	[GCC_WCSS2G_REF_CLK] = &gcc_wcss2g_ref_clk.clkr,
	[GCC_WCSS2G_RTC_CLK] = &gcc_wcss2g_rtc_clk.clkr,
	[GCC_WCSS5G_CLK] = &gcc_wcss5g_clk.clkr,
	[GCC_WCSS5G_REF_CLK] = &gcc_wcss5g_ref_clk.clkr,
	[GCC_WCSS5G_RTC_CLK] = &gcc_wcss5g_rtc_clk.clkr,
};

static const struct qcom_reset_map gcc_ipq40xx_resets[] = {
	[WIFI0_CPU_INIT_RESET] = { 0x1f008, 5 },
	[WIFI0_RADIO_SRIF_RESET] = { 0x1f008, 4 },
	[WIFI0_RADIO_WARM_RESET] = { 0x1f008, 3 },
	[WIFI0_RADIO_COLD_RESET] = { 0x1f008, 2 },
	[WIFI0_CORE_WARM_RESET] = { 0x1f008, 1 },
	[WIFI0_CORE_COLD_RESET] = { 0x1f008, 0 },
	[WIFI1_CPU_INIT_RESET] = { 0x20008, 5 },
	[WIFI1_RADIO_SRIF_RESET] = { 0x20008, 4 },
	[WIFI1_RADIO_WARM_RESET] = { 0x20008, 3 },
	[WIFI1_RADIO_COLD_RESET] = { 0x20008, 2 },
	[WIFI1_CORE_WARM_RESET] = { 0x20008, 1 },
	[WIFI1_CORE_COLD_RESET] = { 0x20008, 0 },
	[USB3_UNIPHY_PHY_ARES] = { 0x1e038, 5 },
	[USB3_HSPHY_POR_ARES] = { 0x1e038, 4 },
	[USB3_HSPHY_S_ARES] = { 0x1e038, 2 },
	[USB2_HSPHY_POR_ARES] = { 0x1e01c, 4 },
	[USB2_HSPHY_S_ARES] = { 0x1e01c, 2 },
	[PCIE_PHY_AHB_ARES] = { 0x1d010, 11 },
	[PCIE_AHB_ARES] = { 0x1d010, 10 },
	[PCIE_PWR_ARES] = { 0x1d010, 9 },
	[PCIE_PIPE_STICKY_ARES] = { 0x1d010, 8 },
	[PCIE_AXI_M_STICKY_ARES] = { 0x1d010, 7 },
	[PCIE_PHY_ARES] = { 0x1d010, 6 },
	[PCIE_PARF_XPU_ARES] = { 0x1d010, 5 },
	[PCIE_AXI_S_XPU_ARES] = { 0x1d010, 4 },
	[PCIE_AXI_M_VMIDMT_ARES] = { 0x1d010, 3 },
	[PCIE_PIPE_ARES] = { 0x1d010, 2 },
	[PCIE_AXI_S_ARES] = { 0x1d010, 1 },
	[PCIE_AXI_M_ARES] = { 0x1d010, 0 },
	[ESS_RESET] = { 0x12008, 0},
	[GCC_BLSP1_BCR] = {0x01000, 0},
	[GCC_BLSP1_QUP1_BCR] = {0x02000, 0},
	[GCC_BLSP1_UART1_BCR] = {0x02038, 0},
	[GCC_BLSP1_QUP2_BCR] = {0x03008, 0},
	[GCC_BLSP1_UART2_BCR] = {0x03028, 0},
	[GCC_BIMC_BCR] = {0x04000, 0},
	[GCC_TLMM_BCR] = {0x05000, 0},
	[GCC_IMEM_BCR] = {0x0E000, 0},
	[GCC_ESS_BCR] = {0x12008, 0},
	[GCC_PRNG_BCR] = {0x13000, 0},
	[GCC_BOOT_ROM_BCR] = {0x13008, 0},
	[GCC_CRYPTO_BCR] = {0x16000, 0},
	[GCC_SDCC1_BCR] = {0x18000, 0},
	[GCC_SEC_CTRL_BCR] = {0x1A000, 0},
	[GCC_AUDIO_BCR] = {0x1B008, 0},
	[GCC_QPIC_BCR] = {0x1C000, 0},
	[GCC_PCIE_BCR] = {0x1D000, 0},
	[GCC_USB2_BCR] = {0x1E008, 0},
	[GCC_USB2_PHY_BCR] = {0x1E018, 0},
	[GCC_USB3_BCR] = {0x1E024, 0},
	[GCC_USB3_PHY_BCR] = {0x1E034, 0},
	[GCC_SYSTEM_NOC_BCR] = {0x21000, 0},
	[GCC_PCNOC_BCR] = {0x2102C, 0},
	[GCC_DCD_BCR] = {0x21038, 0},
	[GCC_SNOC_BUS_TIMEOUT0_BCR] = {0x21064, 0},
	[GCC_SNOC_BUS_TIMEOUT1_BCR] = {0x2106C, 0},
	[GCC_SNOC_BUS_TIMEOUT2_BCR] = {0x21074, 0},
	[GCC_SNOC_BUS_TIMEOUT3_BCR] = {0x2107C, 0},
	[GCC_PCNOC_BUS_TIMEOUT0_BCR] = {0x21084, 0},
	[GCC_PCNOC_BUS_TIMEOUT1_BCR] = {0x2108C, 0},
	[GCC_PCNOC_BUS_TIMEOUT2_BCR] = {0x21094, 0},
	[GCC_PCNOC_BUS_TIMEOUT3_BCR] = {0x2109C, 0},
	[GCC_PCNOC_BUS_TIMEOUT4_BCR] = {0x210A4, 0},
	[GCC_PCNOC_BUS_TIMEOUT5_BCR] = {0x210AC, 0},
	[GCC_PCNOC_BUS_TIMEOUT6_BCR] = {0x210B4, 0},
	[GCC_PCNOC_BUS_TIMEOUT7_BCR] = {0x210BC, 0},
	[GCC_PCNOC_BUS_TIMEOUT8_BCR] = {0x210C4, 0},
	[GCC_PCNOC_BUS_TIMEOUT9_BCR] = {0x210CC, 0},
	[GCC_TCSR_BCR] = {0x22000, 0},
	[GCC_MPM_BCR] = {0x24000, 0},
	[GCC_SPDM_BCR] = {0x25000, 0},
	[ESS_MAC1_ARES] = {0x1200C, 0},
	[ESS_MAC2_ARES] = {0x1200C, 1},
	[ESS_MAC3_ARES] = {0x1200C, 2},
	[ESS_MAC4_ARES] = {0x1200C, 3},
	[ESS_MAC5_ARES] = {0x1200C, 4},
	[ESS_PSGMII_ARES] = {0x1200C, 5},
	[ESS_MAC1_CLK_DIS] = {0x1200C, 8},
	[ESS_MAC2_CLK_DIS] = {0x1200C, 9},
	[ESS_MAC3_CLK_DIS] = {0x1200C, 10},
	[ESS_MAC4_CLK_DIS] = {0x1200C, 11},
	[ESS_MAC5_CLK_DIS] = {0x1200C, 12},
};


static const struct regmap_config gcc_ipq40xx_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= 0x2DFFF,
	.fast_io	= true,
};

static const struct qcom_cc_desc gcc_ipq40xx_desc = {
	.config = &gcc_ipq40xx_regmap_config,
	.clks = gcc_ipq40xx_clocks,
	.num_clks = ARRAY_SIZE(gcc_ipq40xx_clocks),
	.resets = gcc_ipq40xx_resets,
	.num_resets = ARRAY_SIZE(gcc_ipq40xx_resets),
};

static const struct of_device_id gcc_ipq40xx_match_table[] = {
	{ .compatible = "qcom,gcc-ipq40xx" },
	{ }
};
MODULE_DEVICE_TABLE(of, gcc_ipq40xx_match_table);

static int gcc_ipq40xx_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	const struct of_device_id *id;

	id = of_match_device(gcc_ipq40xx_match_table, dev);
	if (!id)
		return -ENODEV;

	/*RTC sleep clock */
	clk_register_fixed_rate(dev, "wifi_rtc_clk_src", NULL,
					 CLK_IS_ROOT, 32768);

	/* FE PLL post dividers */
	clk_register_fixed_rate(dev, "fepll500", NULL, CLK_IS_ROOT,
				      500000000);
	clk_register_fixed_rate(dev, "fepll200", NULL, CLK_IS_ROOT,
				      200000000);
	clk_register_fixed_rate(dev, "fepll125", NULL, CLK_IS_ROOT,
				      125000000);
	clk_register_fixed_rate(dev, "fepll125dly", NULL, CLK_IS_ROOT,
				      125000000);
	clk_register_fixed_rate(dev, "fepllwcss2g", NULL, CLK_IS_ROOT,
				      250000000);
	clk_register_fixed_rate(dev, "fepllwcss5g", NULL, CLK_IS_ROOT,
				      250000000);

	/* DDR PLL post dividers */
	clk_register_fixed_rate(dev, "ddrpllsdcc1", NULL, CLK_IS_ROOT,
				      409800000);
	clk_register_fixed_rate(dev, "ddrpllapss", NULL, CLK_IS_ROOT,
				      626000000);
	clk_register_fixed_rate(dev, "pcnoc_clk_src", NULL, CLK_IS_ROOT,
				      100000000);

	return qcom_cc_probe(pdev, &gcc_ipq40xx_desc);
}
static int gcc_ipq40xx_remove(struct platform_device *pdev)
{
	qcom_cc_remove(pdev);
	return 0;
}

static struct platform_driver gcc_ipq40xx_driver = {
	.probe		= gcc_ipq40xx_probe,
	.remove		= gcc_ipq40xx_remove,
	.driver		= {
		.name	= "qcom,gcc-ipq40xx",
		.owner	= THIS_MODULE,
		.of_match_table = gcc_ipq40xx_match_table,
	},
};

static int __init gcc_ipq40xx_init(void)
{
	return platform_driver_register(&gcc_ipq40xx_driver);
}
core_initcall(gcc_ipq40xx_init);

static void __exit gcc_ipq40xx_exit(void)
{
	platform_driver_unregister(&gcc_ipq40xx_driver);
}
module_exit(gcc_ipq40xx_exit);

MODULE_ALIAS("platform:gcc-ipq40xx.c");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("GCC Driver for ipq40xx driver");
