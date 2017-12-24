/*
 * Copyright (c) 2014 - 2016 The Linux Foundation. All rights reserved.
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
#include <linux/delay.h>
#include <linux/regmap.h>
#include <linux/reset-controller.h>

#include <dt-bindings/clock/qca,adcc-ipq4019.h>

#include "common.h"
#include "clk-regmap.h"
#include "clk-rcg.h"
#include "clk-qcapll.h"
#include "clk-branch.h"
#include "reset.h"

#define AUDIO_PLL_CONFIG_REG				0x00000
#define AUDIO_PLL_MODULATION_REG			0x00004
#define AUDIO_PLL_MOD_STEP_REG				0x00008
#define CURRENT_AUDIO_PLL_MODULATION_REG		0x0000c
#define AUDIO_PLL_CONFIG1_REG				0x00010
#define AUDIO_ATB_SETTING_REG				0x00014
#define AUDIO_RXB_CFG_MUXR_REG				0x000cc
#define AUDIO_RXB_MISC_REG				0x000d0
#define AUDIO_RXB_CBCR_REG				0x000D4
#define AUDIO_RXM_CMD_RCGR_REG				0x000e8
#define AUDIO_RXM_CFG_RCGR_REG				0x000ec
#define AUDIO_RXM_MISC_REG				0x000f0
#define AUDIO_RXM_CBCR_REG				0x000F4
#define AUDIO_TXB_CFG_MUXR_REG				0x0010c
#define AUDIO_TXB_MISC_REG				0x00110
#define AUDIO_TXB_CBCR_REG				0x00114
#define AUDIO_SPDIF_MISC_REG				0x00118
#define AUDIO_SPDIF_CBCR_REG				0x0011c
#define AUDIO_SPDIFDIV2_MISC_REG			0x00120
#define AUDIO_SPDIFDIV2_CBCR_REG			0x00124
#define AUDIO_TXM_CMD_RCGR_REG				0x00128
#define AUDIO_TXM_CFG_RCGR_REG				0x0012c
#define AUDIO_TXM_MISC_REG				0x00130
#define AUDIO_TXM_CBCR_REG				0x00134
#define AUDIO_SAMPLE_CBCR_REG				0x00154
#define AUDIO_PCM_CMD_RCGR_REG				0x00168
#define AUDIO_PCM_CFG_RCGR_REG				0x0016C
#define AUDIO_PCM_MISC_REG				0x00170
#define AUDIO_PCM_CBCR_REG				0x00174
#define AUDIO_XO_CBCR_REG				0x00194
#define AUDIO_SPDIFINFAST_CMD_RCGR_REG			0x001A8
#define AUDIO_SPDIFINFAST_CFG_RCGR_REG			0x001AC
#define AUDIO_SPDIFINFAST_CBCR_REG			0x001B4
#define AUDIO_AHB_CBCR_REG				0x001c8
#define AUDIO_AHB_I2S0_CBCR_REG				0x001cc
#define AUDIO_AHB_I2S3_CBCR_REG				0x001d0
#define AUDIO_AHB_MBOX0_CBCR_REG			0x001D4
#define AUDIO_AHB_MBOX3_CBCR_REG			0x001d8

#define F(f, s, h, m, n) { (f), (s), (2 * (h) - 1), (m), (n) }
#define P_XO 0
#define ADSS_PLL 1
#define MCLK_MCLK_IN 2
#define BCLK_BCLK_IN 2
#define BCLK_MCLK_IN 3

static struct parent_map adcc_xo_adpll_padmclk_map[] = {
	{  P_XO, 0 },
	{  ADSS_PLL, 1 },
	{  MCLK_MCLK_IN, 2 },
};

static const char * const adcc_xo_adpll_padmclk[] = {
	"xo",
	"adss_pll",
	"padmclk",
};

static struct parent_map adcc_xo_adpll_padbclk_padmclk_map[] = {
	{  P_XO, 0 },
	{  ADSS_PLL, 1 },
	{  MCLK_MCLK_IN, 2 },
	{  BCLK_MCLK_IN, 3 },
};

static const char * const adcc_xo_adpll_padbclk_padmclk[] = {
	"xo",
	"adss_pll",
	"padbclk",
	"padmclk",
};

static struct parent_map adcc_xo_adpll_map[] = {
	{  P_XO, 0 },
	{  ADSS_PLL, 1 },
};

static const char * const adcc_xo_adpll[] = {
	"xo",
	"adss_pll",
};

static const struct pll_freq_tbl adss_freq_tbl[] = {
	{163840000, 1, 5, 40, 0x3D708},
	{180633600, 1, 5, 45, 0xA234},
	{184320000, 1, 5, 46, 0x51E9},
	{196608000, 1, 5, 49, 0x9bA6},
	{197568000, 1, 5, 49, 0x19168},
	{}
};

static struct clk_qcapll adss_pll_src = {
	.config_reg		= AUDIO_PLL_CONFIG_REG,
	.mod_reg		= AUDIO_PLL_MODULATION_REG,
	.modstep_reg		= AUDIO_PLL_MOD_STEP_REG,
	.current_mod_pll_reg	= CURRENT_AUDIO_PLL_MODULATION_REG,
	.config1_reg		= AUDIO_PLL_CONFIG1_REG,
	.freq_tbl = adss_freq_tbl,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "adss_pll",
		.parent_names = (const char *[]){ "xo" },
		.num_parents = 1,
		.ops = &clk_qcapll_ops,
		.flags = CLK_IGNORE_UNUSED,
	},
};

static const struct freq_tbl ftbl_m_clk[] = {
	{255, MCLK_MCLK_IN, 1, 0, 0},
	{2048000, ADSS_PLL, 96, 0, 0},
	{2822400, ADSS_PLL, 64, 0, 0},
	{4096000, ADSS_PLL, 48, 0, 0},
	{5644800, ADSS_PLL, 32, 0, 0},
	{6144000, ADSS_PLL, 32, 0, 0},
	{8192000, ADSS_PLL, 24, 0, 0},
	{11289600, ADSS_PLL, 16, 0, 0},
	{12288000, ADSS_PLL, 16, 0, 0},
	{14112000, ADSS_PLL, 14, 0, 0},
	{15360000, ADSS_PLL, 12, 0, 0},
	{16384000, ADSS_PLL, 12, 0, 0},
	{20480000, ADSS_PLL, 8, 0, 0},
	{22579200, ADSS_PLL, 8, 0, 0},
	{24576000, ADSS_PLL, 8, 0, 0},
	{30720000, ADSS_PLL, 6, 0, 0},
	{ }
};

static struct clk_cdiv_rcg2 rxm_clk_src = {
	.cdiv.offset = AUDIO_RXM_MISC_REG,
	.cdiv.shift = 4,
	.cdiv.mask = 0xf,
	.cmd_rcgr = AUDIO_RXM_CMD_RCGR_REG,
	.hid_width = 5,
	.parent_map = adcc_xo_adpll_padmclk_map,
	.freq_tbl = ftbl_m_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "rxm_clk_src",
		.parent_names = adcc_xo_adpll_padmclk,
		.num_parents = 3,
		.ops = &clk_cdiv_rcg2_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_branch adcc_rxm_clk_src = {
	.halt_reg = AUDIO_RXM_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_RXM_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_rxm_clk_src",
			.parent_names = (const char *[]){"rxm_clk_src"},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};

static struct clk_cdiv_rcg2 txm_clk_src = {
	.cdiv.offset = AUDIO_TXM_MISC_REG,
	.cdiv.shift = 4,
	.cdiv.mask = 0xf,
	.cmd_rcgr = AUDIO_TXM_CMD_RCGR_REG,
	.hid_width = 5,
	.parent_map = adcc_xo_adpll_padmclk_map,
	.freq_tbl = ftbl_m_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "txm_clk_src",
		.parent_names = adcc_xo_adpll_padmclk,
		.num_parents = 3,
		.ops = &clk_cdiv_rcg2_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_branch adcc_txm_clk_src = {
	.halt_reg = AUDIO_TXM_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_TXM_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_txm_clk_src",
			.parent_names = (const char *[]){
				"txm_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};

static const struct freq_tbl ftbl_bclk_clk[] = {
	{254, BCLK_BCLK_IN, 1, 0, 0},
	{255, BCLK_MCLK_IN, 1, 0, 0},
	{512000, ADSS_PLL, 384, 0, 0},
	{705600, ADSS_PLL, 256, 0, 0},
	{1024000, ADSS_PLL, 192, 0, 0},
	{1411200, ADSS_PLL, 128, 0, 0},
	{1536000, ADSS_PLL, 128, 0, 0},
	{2048000, ADSS_PLL, 96, 0, 0},
	{2822400, ADSS_PLL, 64, 0, 0},
	{3072000, ADSS_PLL, 64, 0, 0},
	{4096000, ADSS_PLL, 48, 0, 0},
	{5120000, ADSS_PLL, 32, 0, 0},
	{5644800, ADSS_PLL, 32, 0, 0},
	{6144000, ADSS_PLL, 32, 0, 0},
	{7056000, ADSS_PLL, 24, 0, 0},
	{7680000, ADSS_PLL, 24, 0, 0},
	{8192000, ADSS_PLL, 24, 0, 0},
	{10240000, ADSS_PLL, 16, 0, 0},
	{11289600, ADSS_PLL, 16, 0, 0},
	{12288000, ADSS_PLL, 16, 0, 0},
	{14112000, ADSS_PLL, 16, 0, 0},
	{15360000, ADSS_PLL, 12, 0, 0},
	{16384000, ADSS_PLL, 12, 0, 0},
	{22579200, ADSS_PLL, 8, 0, 0},
	{24576000, ADSS_PLL,  8, 0, 0},
	{30720000, ADSS_PLL,  6, 0, 0},
	{ }
};

static struct clk_muxr_misc txb_clk_src = {
	.misc.offset = AUDIO_TXB_MISC_REG,
	.misc.shift = 1,
	.misc.mask = 0x1FF,
	.muxr.offset = AUDIO_TXB_CFG_MUXR_REG,
	.muxr.shift = 8,
	.muxr.mask = 0x7,
	.parent_map = adcc_xo_adpll_padbclk_padmclk_map,
	.freq_tbl = ftbl_bclk_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "txb_clk_src",
		.parent_names = adcc_xo_adpll_padbclk_padmclk,
		.num_parents = 4,
		.ops = &clk_muxr_misc_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_branch adcc_txb_clk_src = {
	.halt_reg = AUDIO_TXB_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_TXB_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_txb_clk_src",
			.parent_names = (const char *[]){
				"txb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};

static struct clk_muxr_misc rxb_clk_src = {
	.misc.offset = AUDIO_RXB_MISC_REG,
	.misc.shift = 1,
	.misc.mask = 0x1FF,
	.muxr.offset = AUDIO_RXB_CFG_MUXR_REG,
	.muxr.shift = 8,
	.muxr.mask = 0x7,
	.parent_map = adcc_xo_adpll_padbclk_padmclk_map,
	.freq_tbl = ftbl_bclk_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "rxb_clk_src",
		.parent_names = adcc_xo_adpll_padbclk_padmclk,
		.num_parents = 4,
		.ops = &clk_muxr_misc_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};


static struct clk_branch adcc_rxb_clk_src = {
	.halt_reg = AUDIO_RXB_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_RXB_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_rxb_clk_src",
			.parent_names = (const char *[]){
				"rxb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT,
		},
	},
};



static const struct freq_tbl ftbl_adcc_pcm_clk[] = {
	{8192000, ADSS_PLL, 24, 0, 0},
	{16384000, ADSS_PLL, 12, 0, 0},
	{ }
};

static struct clk_cdiv_rcg2 pcm_clk_src = {
	.cdiv.offset = AUDIO_PCM_MISC_REG,
	.cdiv.shift = 4,
	.cdiv.mask = 0xf,
	.cmd_rcgr = AUDIO_PCM_CMD_RCGR_REG,
	.hid_width = 5,
	.parent_map = adcc_xo_adpll_map,
	.freq_tbl = ftbl_adcc_pcm_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "pcm_clk_src",
		.parent_names = adcc_xo_adpll,
		.num_parents = 2,
		.ops = &clk_cdiv_rcg2_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};



static struct clk_branch adcc_pcm_clk_src = {
	.halt_reg = AUDIO_PCM_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_PCM_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_pcm_clk_src",
			.parent_names = (const char *[]){
				"pcm_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
		},
	},
};



static const struct freq_tbl ftbl_adcc_spdifinfast_clk[] = {
	F(49152000, ADSS_PLL, 0x04, 0, 0),
	{ }
};

static struct clk_rcg2 spdifinfast_src = {
	.cmd_rcgr = AUDIO_SPDIFINFAST_CMD_RCGR_REG,
	.hid_width = 5,
	.parent_map = adcc_xo_adpll_map,
	.freq_tbl = ftbl_adcc_spdifinfast_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "spdifinfast_src",
		.parent_names = adcc_xo_adpll,
		.num_parents = 2,
		.ops = &clk_rcg2_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_branch adcc_spdifinfast_src = {
	.halt_reg = AUDIO_SPDIFINFAST_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_SPDIFINFAST_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_spdifinfast_src",
			.parent_names = (const char *[]){
				"spdifinfast_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_muxr_misc spdif_src = {
	.misc.offset = AUDIO_SPDIF_MISC_REG,
	.misc.shift = 1,
	.misc.mask = 0x1FF,
	.muxr.offset = AUDIO_TXB_CFG_MUXR_REG,
	.muxr.shift = 8,
	.muxr.mask = 0x7,
	.parent_map = adcc_xo_adpll_padbclk_padmclk_map,
	.freq_tbl = ftbl_m_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "spdif_src",
		.parent_names = adcc_xo_adpll_padbclk_padmclk,
		.num_parents = 4,
		.ops = &clk_muxr_misc_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_branch adcc_spdif_src = {
	.halt_reg = AUDIO_SPDIF_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_SPDIF_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_spdif_src",
			.parent_names = (const char *[]){
				"spdif_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_muxr_misc spdifdiv2_src = {
	.misc.offset = AUDIO_SPDIFDIV2_MISC_REG,
	.misc.shift = 1,
	.misc.mask = 0x1FF,
	.parent_map = adcc_xo_adpll_padbclk_padmclk_map,
	.muxr.offset = AUDIO_TXB_CFG_MUXR_REG,
	.muxr.shift = 8,
	.muxr.mask = 0x7,
	.freq_tbl = ftbl_bclk_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "spdifdiv2_src",
		.parent_names = adcc_xo_adpll_padbclk_padmclk,
		.num_parents = 4,
		.ops = &clk_muxr_misc_ops,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_branch adcc_spdifdiv2_src = {
	.halt_reg = AUDIO_SPDIFDIV2_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_SPDIFDIV2_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_spdifdiv2_src",
			.parent_names = (const char *[]){
				"spdifdiv2_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch adcc_sample_src = {
	.halt_reg = AUDIO_SAMPLE_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_SAMPLE_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_sample_src",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch adcc_xo_src = {
	.halt_reg = AUDIO_XO_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_XO_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_xo_src",
			.parent_names = (const char *[]){
				"XO",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
		},
	},
};


static struct clk_branch adcc_ahb_src = {
	.halt_reg = AUDIO_AHB_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_AHB_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_audio_ahb_src",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch adcc_ahb_i2s0_src = {
	.halt_reg = AUDIO_AHB_I2S0_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_AHB_I2S0_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_audio_ahb_i2s0",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch adcc_ahb_i2s3_src = {
	.halt_reg = AUDIO_AHB_I2S3_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_AHB_I2S3_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_audio_ahb_i2s3",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch adcc_ahb_mbox0_src = {
	.halt_reg = AUDIO_AHB_MBOX0_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_AHB_MBOX0_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_audio_mbox0_src",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch adcc_ahb_mbox3_src = {
	.halt_reg = AUDIO_AHB_MBOX3_CBCR_REG,
	.clkr = {
		.enable_reg = AUDIO_AHB_MBOX3_CBCR_REG,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "adcc_ahb_mbox3_src",
			.parent_names = (const char *[]){
				"pcnoc_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_regmap *adcc_ipq4019_clocks[] = {
	[ADSS_PLL_SRC]				= &adss_pll_src.clkr,
	[RXM_CLK_SRC]				= &rxm_clk_src.clkr,
	[ADCC_RXM_CLK_SRC]			= &adcc_rxm_clk_src.clkr,
	[TXM_CLK_SRC]				= &txm_clk_src.clkr,
	[ADCC_TXM_CLK_SRC]			= &adcc_txm_clk_src.clkr,
	[TXB_CLK_SRC]				= &txb_clk_src.clkr,
	[ADCC_TXB_CLK_SRC]			= &adcc_txb_clk_src.clkr,
	[RXB_CLK_SRC]				= &rxb_clk_src.clkr,
	[ADCC_RXB_CLK_SRC]			= &adcc_rxb_clk_src.clkr,
	[PCM_CLK_SRC]				= &pcm_clk_src.clkr,
	[ADCC_PCM_CLK_SRC]			= &adcc_pcm_clk_src.clkr,
	[AUDIO_SPDIFINFAST_SRC]			= &spdifinfast_src.clkr,
	[ADCC_AUDIO_SPDIFINFAST_SRC]		= &adcc_spdifinfast_src.clkr,
	[ADCC_AUDIO_AHB_SRC]			= &adcc_ahb_src.clkr,
	[ADCC_AHB_I2S0]				= &adcc_ahb_i2s0_src.clkr,
	[ADCC_AHB_I2S3]				= &adcc_ahb_i2s3_src.clkr,
	[ADCC_AHB_MBOX0_SRC]			= &adcc_ahb_mbox0_src.clkr,
	[ADCC_AHB_MBOX3_SRC]			= &adcc_ahb_mbox3_src.clkr,
	[SPDIF_SRC]				= &spdif_src.clkr,
	[ADCC_SPDIF_SRC]			= &adcc_spdif_src.clkr,
	[SPDIFDIV2_SRC]				= &spdifdiv2_src.clkr,
	[ADCC_SPDIFDIV2_SRC]			= &adcc_spdifdiv2_src.clkr,
	[ADCC_SAMPLE_SRC]			= &adcc_sample_src.clkr,
	[ADCC_XO_SRC]				= &adcc_xo_src.clkr,
};

static const struct qcom_reset_map adcc_ipq4019_resets[] = {
};

struct cdiv_fixed {
	char	*name;
	char	*pname;
	u32	flags;
	u32	mult;
	u32	div;
};

static const struct regmap_config adcc_ipq4019_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= 0x2ff,
	.fast_io	= true,
};

static const struct qcom_cc_desc adcc_ipq4019_desc = {
	.config = &adcc_ipq4019_regmap_config,
	.clks = adcc_ipq4019_clocks,
	.num_clks = ARRAY_SIZE(adcc_ipq4019_clocks),
	.resets = adcc_ipq4019_resets,
	.num_resets = ARRAY_SIZE(adcc_ipq4019_resets),
};

static const struct of_device_id adcc_ipq4019_match_table[] = {
	{ .compatible = "qcom,adcc-ipq4019" },
	{ }
};
MODULE_DEVICE_TABLE(of, gcc_ipq4019_match_table);

static int adcc_ipq4019_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct regmap *regmap;

	/* High speed external clock */
	clk_register_fixed_rate(dev, "xo", NULL, CLK_IS_ROOT, 48000000);

	/* External padbclk & padmclk clock.These [254 & 255] frequencies are
	 * taken as tokens only to support the INPUTS from PADS.
	 * Reference: ADSS_HPG/HDD document for IPQ4019
	 */
	clk_register_fixed_rate(dev, "padbclk", NULL, CLK_IS_ROOT, 254);
	clk_register_fixed_rate(dev, "padmclk", NULL, CLK_IS_ROOT, 255);

	regmap = qcom_cc_map(pdev, &adcc_ipq4019_desc);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	return qcom_cc_really_probe(pdev, &adcc_ipq4019_desc, regmap);
}

static int adcc_ipq4019_remove(struct platform_device *pdev)
{
	qcom_cc_remove(pdev);
	return 0;
}

static struct platform_driver adcc_ipq4019_driver = {
	.probe		= adcc_ipq4019_probe,
	.remove		= adcc_ipq4019_remove,
	.driver		= {
		.name	= "adcc-ipq4019",
		.of_match_table = adcc_ipq4019_match_table,
	},
};
module_platform_driver(adcc_ipq4019_driver);

MODULE_DESCRIPTION("Audio Digital Clock controller Driver for IPQ4019");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:adcc-ipq4019");
