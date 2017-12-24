/*
 * Copyright (c) 2013, 2015-2016 The Linux Foundation. All rights reserved.
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
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/export.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>

#include <asm/div64.h>

#include "clk-qcapll.h"

#define PLL_CONFIG1_SRESET_L		BIT(0)
#define PLL_MODULATION_START		BIT(0)
#define PLL_CONFIG_PLLPWD		BIT(5)

#define PLL_POSTDIV_MASK	0x380
#define PLL_POSTDIV_SHFT	7
#define PLL_PLLPWD_MASK         0x20
#define PLL_PLLPWD_SHFT         5
#define PLL_REFDIV_MASK		0x7
#define PLL_REFDIV_SHFT		0
#define PLL_TGT_INT_SHFT	1
#define PLL_TGT_INT_MASK	0x3FE
#define PLL_TGT_FRAC_MASK	0x1FFFF800
#define PLL_TGT_FRAC_SHFT	11


static int clk_qcapll_enable(struct clk_hw *hw)
{
	struct clk_qcapll *pll = to_clk_qcapll(hw);
	int ret;

	/* Enable PLL bypass mode. */
	ret = regmap_update_bits(pll->clkr.regmap, pll->config_reg,
				 PLL_CONFIG_PLLPWD, 0);
	if (ret)
		return ret;

	return 0;
}

static void clk_qcapll_disable(struct clk_hw *hw)
{
	struct clk_qcapll *pll = to_clk_qcapll(hw);

	/* Disable PLL bypass mode. */
	regmap_update_bits(pll->clkr.regmap, pll->config_reg, PLL_CONFIG_PLLPWD,
			   0x1);
}

static int clk_qcapll_is_enabled(struct clk_hw *hw)
{
	u32 config;
	struct clk_qcapll *pll = to_clk_qcapll(hw);

	regmap_read(pll->clkr.regmap, pll->config_reg, &config);
	return config & PLL_PLLPWD_MASK;
}

static unsigned long
clk_qcapll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_qcapll *pll = to_clk_qcapll(hw);
	u32 ref_div, post_plldiv, tgt_div_frac, tgt_div_int;
	u32 config, mod_reg;

	regmap_read(pll->clkr.regmap, pll->config_reg, &config);
	regmap_read(pll->clkr.regmap, pll->mod_reg, &mod_reg);

	ref_div = (config & PLL_REFDIV_MASK) >> PLL_REFDIV_SHFT;
	post_plldiv = (config & PLL_POSTDIV_SHFT) >> PLL_POSTDIV_SHFT;
	tgt_div_frac = (mod_reg & PLL_TGT_FRAC_MASK) >>  PLL_TGT_FRAC_SHFT;
	tgt_div_int = (mod_reg & PLL_TGT_INT_MASK) >> PLL_TGT_INT_SHFT;

	/*
	 * FICO = (Fref / (refdiv+1)) * (Ninv + Nfrac[17:5]/2^13
	 *	   + Nfrac[4:0]/(25*2^13)).
	 *
	 * we use this Lookups to get the precise frequencies as we need
	 * the calculation would need use of some math functions to get
	 * precise values which will add to the complexity. Hence, a simple
	 * lookup table based on the Fract values
	 */

	if (tgt_div_frac == 0x3D708)
		return 163840000;
	else if (tgt_div_frac == 0xA234)
		return 180633600;
	else if (tgt_div_frac == 0x51E9)
		return 184320000;
	else if (tgt_div_frac == 0x9bA6)
		return 196608000;
	else if (tgt_div_frac == 0x19168)
		return 197568000;

	return 0;
}

static const
struct pll_freq_tbl *find_freq(const struct pll_freq_tbl *f, unsigned long rate)
{
	if (!f)
		return NULL;

	for (; f->freq; f++)
		if (rate <= f->freq)
			return f;

	return NULL;
}

static int
clk_qcapll_determine_rate(struct clk_hw *hw, struct clk_rate_request *req)
{
	struct clk_qcapll *pll = to_clk_qcapll(hw);
	const struct pll_freq_tbl *f;
	unsigned long rate;

	f = find_freq(pll->freq_tbl, req->rate);
	if (!f)
		rate = clk_qcapll_recalc_rate(hw, req->best_parent_rate);
	else
		rate = f->freq;

	req->best_parent_rate = req->rate = rate;

	return 0;
}

static int
clk_qcapll_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long p_rate)
{
	struct clk_qcapll *pll = to_clk_qcapll(hw);
	const struct pll_freq_tbl *f;
	u32 val, mask;

	f = find_freq(pll->freq_tbl, rate);
	if (!f)
		return -EINVAL;

	if (clk_qcapll_is_enabled(hw))
		clk_qcapll_disable(hw);

	regmap_write(pll->clkr.regmap, pll->config1_reg, 0xc);
	udelay(2);
	regmap_write(pll->clkr.regmap, pll->config1_reg, 0xd);

	val = f->postplldiv << PLL_POSTDIV_SHFT;
	val |= f->refdiv << PLL_REFDIV_SHFT;

	mask = PLL_POSTDIV_MASK | PLL_REFDIV_MASK;
	regmap_update_bits(pll->clkr.regmap, pll->config_reg, mask, val);

	clk_qcapll_enable(hw);

	val = f->tgt_div_int << PLL_TGT_INT_SHFT;
	val |= f->tgt_div_frac << PLL_TGT_FRAC_SHFT;

	mask = PLL_TGT_FRAC_MASK | PLL_TGT_INT_MASK;
	regmap_update_bits(pll->clkr.regmap, pll->mod_reg, mask, val);

	/* Start the PLL to initiate the Modulation. */
	regmap_update_bits(pll->clkr.regmap, pll->mod_reg,
						PLL_MODULATION_START, 0);

	/*
	 * Wait until PLL is locked. Refer DPLL document for IPQ4019.
	 * This is recommended settling time for the PLL.
	 */
	udelay(50);

	return 0;
}

const struct clk_ops clk_qcapll_ops = {
	.enable = clk_qcapll_enable,
	.disable = clk_qcapll_disable,
	.is_enabled = clk_qcapll_is_enabled,
	.recalc_rate = clk_qcapll_recalc_rate,
	.determine_rate = clk_qcapll_determine_rate,
	.set_rate = clk_qcapll_set_rate,
};
EXPORT_SYMBOL_GPL(clk_qcapll_ops);
