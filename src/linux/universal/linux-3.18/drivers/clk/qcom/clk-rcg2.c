/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
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
#include <linux/bug.h>
#include <linux/export.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/regmap.h>
#include <linux/math64.h>

#include <asm/div64.h>

#include "clk-rcg.h"
#include "common.h"

#define CMD_REG			0x0
#define CMD_UPDATE		BIT(0)
#define CMD_ROOT_EN		BIT(1)
#define CMD_DIRTY_CFG		BIT(4)
#define CMD_DIRTY_N		BIT(5)
#define CMD_DIRTY_M		BIT(6)
#define CMD_DIRTY_D		BIT(7)
#define CMD_ROOT_OFF		BIT(31)

#define CFG_REG			0x4
#define CFG_SRC_DIV_SHIFT	0
#define CFG_SRC_SEL_SHIFT	8
#define CFG_SRC_SEL_MASK	(0x7 << CFG_SRC_SEL_SHIFT)
#define CFG_MODE_SHIFT		12
#define CFG_MODE_MASK		(0x3 << CFG_MODE_SHIFT)
#define CFG_MODE_DUAL_EDGE	(0x2 << CFG_MODE_SHIFT)

#define M_REG			0x8
#define N_REG			0xc
#define D_REG			0x10

static int clk_rcg2_is_enabled(struct clk_hw *hw)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	u32 cmd;
	int ret;

	ret = regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + CMD_REG, &cmd);
	if (ret)
		return ret;

	return (cmd & CMD_ROOT_OFF) == 0;
}

static u8 clk_rcg2_get_parent(struct clk_hw *hw)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	int num_parents = __clk_get_num_parents(hw->clk);
	u32 cfg;
	int i, ret;

	ret = regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG, &cfg);
	if (ret)
		return ret;

	cfg &= CFG_SRC_SEL_MASK;
	cfg >>= CFG_SRC_SEL_SHIFT;

	for (i = 0; i < num_parents; i++)
		if (cfg == rcg->parent_map[i])
			return i;

	return -EINVAL;
}

static int update_config(struct clk_rcg2 *rcg)
{
	int count, ret;
	u32 cmd;
	struct clk_hw *hw = &rcg->clkr.hw;
	const char *name = __clk_get_name(hw->clk);
	u32 flags;

	flags = __clk_get_flags(hw->clk);

	ret = regmap_update_bits(rcg->clkr.regmap, rcg->cmd_rcgr + CMD_REG,
				 CMD_UPDATE, CMD_UPDATE);
	if (ret)
		return ret;

	if (flags && CLK_RCG2_NO_WAIT) {
		return 0;
	} else {
		/* Wait for update to take effect */
		for (count = 500; count > 0; count--) {
			ret = regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr +
								CMD_REG, &cmd);
			if (ret)
				return ret;
			if (!(cmd & CMD_UPDATE))
				return 0;
			udelay(1);
		}

		WARN(1, "%s: rcg didn't update its configuration.", name);
	}
	return 0;
}

static int clk_rcg2_set_parent(struct clk_hw *hw, u8 index)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	int ret;

	ret = regmap_update_bits(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG,
				 CFG_SRC_SEL_MASK,
				 rcg->parent_map[index] << CFG_SRC_SEL_SHIFT);
	if (ret)
		return ret;

	return update_config(rcg);
}

/*
 * Calculate m/n:d rate
 *
 *          parent_rate     m
 *   rate = ----------- x  ---
 *            hid_div       n
 */
static unsigned long
calc_rate(unsigned long rate, u32 m, u32 n, u32 mode, u32 hid_div)
{
	if (hid_div) {
		rate *= 2;
		rate /= hid_div + 1;
	}

	if (mode) {
		u64 tmp = rate;
		tmp *= m;
		do_div(tmp, n);
		rate = tmp;
	}

	return rate;
}

static unsigned long
clk_rcg2_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	u32 cfg, hid_div, m = 0, n = 0, mode = 0, mask;

	regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG, &cfg);

	if (rcg->mnd_width) {
		mask = BIT(rcg->mnd_width) - 1;
		regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + M_REG, &m);
		m &= mask;
		regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + N_REG, &n);
		n =  ~n;
		n &= mask;
		n += m;
		mode = cfg & CFG_MODE_MASK;
		mode >>= CFG_MODE_SHIFT;
	}

	mask = BIT(rcg->hid_width) - 1;
	hid_div = cfg >> CFG_SRC_DIV_SHIFT;
	hid_div &= mask;

	return calc_rate(parent_rate, m, n, mode, hid_div);
}

static long _freq_tbl_determine_rate(struct clk_hw *hw,
		const struct freq_tbl *f, unsigned long rate,
		unsigned long *p_rate, struct clk **p)
{
	unsigned long clk_flags;

	f = qcom_find_freq(f, rate);
	if (!f)
		return -EINVAL;

	clk_flags = __clk_get_flags(hw->clk);
	*p = clk_get_parent_by_index(hw->clk, f->src);
	if (clk_flags & CLK_SET_RATE_PARENT) {
		if (f->pre_div) {
			rate /= 2;
			rate *= f->pre_div + 1;
		}

		if (f->n) {
			u64 tmp = rate;
			tmp = tmp * f->n;
			do_div(tmp, f->m);
			rate = tmp;
		}
	} else {
		rate =  __clk_get_rate(*p);
	}
	*p_rate = rate;

	return f->freq;
}

static long clk_rcg2_determine_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long *p_rate, struct clk **p)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);

	return _freq_tbl_determine_rate(hw, rcg->freq_tbl, rate, p_rate, p);
}

static int clk_rcg2_configure(struct clk_rcg2 *rcg, const struct freq_tbl *f)
{
	u32 cfg, mask;
	int ret;

	if (rcg->mnd_width && f->n) {
		mask = BIT(rcg->mnd_width) - 1;
		ret = regmap_update_bits(rcg->clkr.regmap,
				rcg->cmd_rcgr + M_REG, mask, f->m);
		if (ret)
			return ret;

		ret = regmap_update_bits(rcg->clkr.regmap,
				rcg->cmd_rcgr + N_REG, mask, ~(f->n - f->m));
		if (ret)
			return ret;

		ret = regmap_update_bits(rcg->clkr.regmap,
				rcg->cmd_rcgr + D_REG, mask, ~f->n);
		if (ret)
			return ret;
	}

	mask = BIT(rcg->hid_width) - 1;
	mask |= CFG_SRC_SEL_MASK | CFG_MODE_MASK;
	cfg = f->pre_div << CFG_SRC_DIV_SHIFT;
	cfg |= rcg->parent_map[f->src] << CFG_SRC_SEL_SHIFT;
	if (rcg->mnd_width && f->n && (f->m != f->n))
		cfg |= CFG_MODE_DUAL_EDGE;
	ret = regmap_update_bits(rcg->clkr.regmap,
			rcg->cmd_rcgr + CFG_REG, mask, cfg);
	if (ret)
		return ret;

	return update_config(rcg);
}

static int __clk_rcg2_set_rate(struct clk_hw *hw, unsigned long rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	const struct freq_tbl *f;

	f = qcom_find_freq(rcg->freq_tbl, rate);
	if (!f)
		return -EINVAL;

	return clk_rcg2_configure(rcg, f);
}

static int clk_rcg2_set_rate(struct clk_hw *hw, unsigned long rate,
			    unsigned long parent_rate)
{
	return __clk_rcg2_set_rate(hw, rate);
}

static int clk_rcg2_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	return __clk_rcg2_set_rate(hw, rate);
}

const struct clk_ops clk_rcg2_ops = {
	.is_enabled = clk_rcg2_is_enabled,
	.get_parent = clk_rcg2_get_parent,
	.set_parent = clk_rcg2_set_parent,
	.recalc_rate = clk_rcg2_recalc_rate,
	.determine_rate = clk_rcg2_determine_rate,
	.set_rate = clk_rcg2_set_rate,
	.set_rate_and_parent = clk_rcg2_set_rate_and_parent,
};
EXPORT_SYMBOL_GPL(clk_rcg2_ops);

struct frac_entry {
	int num;
	int den;
};

static const struct frac_entry frac_table_675m[] = {	/* link rate of 270M */
	{ 52, 295 },	/* 119 M */
	{ 11, 57 },	/* 130.25 M */
	{ 63, 307 },	/* 138.50 M */
	{ 11, 50 },	/* 148.50 M */
	{ 47, 206 },	/* 154 M */
	{ 31, 100 },	/* 205.25 M */
	{ 107, 269 },	/* 268.50 M */
	{ },
};

static struct frac_entry frac_table_810m[] = { /* Link rate of 162M */
	{ 31, 211 },	/* 119 M */
	{ 32, 199 },	/* 130.25 M */
	{ 63, 307 },	/* 138.50 M */
	{ 11, 60 },	/* 148.50 M */
	{ 50, 263 },	/* 154 M */
	{ 31, 120 },	/* 205.25 M */
	{ 119, 359 },	/* 268.50 M */
	{ },
};

static int clk_edp_pixel_set_rate(struct clk_hw *hw, unsigned long rate,
			      unsigned long parent_rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	struct freq_tbl f = *rcg->freq_tbl;
	const struct frac_entry *frac;
	int delta = 100000;
	s64 src_rate = parent_rate;
	s64 request;
	u32 mask = BIT(rcg->hid_width) - 1;
	u32 hid_div;

	if (src_rate == 810000000)
		frac = frac_table_810m;
	else
		frac = frac_table_675m;

	for (; frac->num; frac++) {
		request = rate;
		request *= frac->den;
		request = div_s64(request, frac->num);
		if ((src_rate < (request - delta)) ||
		    (src_rate > (request + delta)))
			continue;

		regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG,
				&hid_div);
		f.pre_div = hid_div;
		f.pre_div >>= CFG_SRC_DIV_SHIFT;
		f.pre_div &= mask;
		f.m = frac->num;
		f.n = frac->den;

		return clk_rcg2_configure(rcg, &f);
	}

	return -EINVAL;
}

static int clk_edp_pixel_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	/* Parent index is set statically in frequency table */
	return clk_edp_pixel_set_rate(hw, rate, parent_rate);
}

static long clk_edp_pixel_determine_rate(struct clk_hw *hw, unsigned long rate,
				 unsigned long *p_rate, struct clk **p)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	const struct freq_tbl *f = rcg->freq_tbl;
	const struct frac_entry *frac;
	int delta = 100000;
	s64 src_rate = *p_rate;
	s64 request;
	u32 mask = BIT(rcg->hid_width) - 1;
	u32 hid_div;

	/* Force the correct parent */
	*p = clk_get_parent_by_index(hw->clk, f->src);

	if (src_rate == 810000000)
		frac = frac_table_810m;
	else
		frac = frac_table_675m;

	for (; frac->num; frac++) {
		request = rate;
		request *= frac->den;
		request = div_s64(request, frac->num);
		if ((src_rate < (request - delta)) ||
		    (src_rate > (request + delta)))
			continue;

		regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG,
				&hid_div);
		hid_div >>= CFG_SRC_DIV_SHIFT;
		hid_div &= mask;

		return calc_rate(src_rate, frac->num, frac->den, !!frac->den,
				 hid_div);
	}

	return -EINVAL;
}

const struct clk_ops clk_edp_pixel_ops = {
	.is_enabled = clk_rcg2_is_enabled,
	.get_parent = clk_rcg2_get_parent,
	.set_parent = clk_rcg2_set_parent,
	.recalc_rate = clk_rcg2_recalc_rate,
	.set_rate = clk_edp_pixel_set_rate,
	.set_rate_and_parent = clk_edp_pixel_set_rate_and_parent,
	.determine_rate = clk_edp_pixel_determine_rate,
};
EXPORT_SYMBOL_GPL(clk_edp_pixel_ops);

static long clk_byte_determine_rate(struct clk_hw *hw, unsigned long rate,
			 unsigned long *p_rate, struct clk **p)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	const struct freq_tbl *f = rcg->freq_tbl;
	unsigned long parent_rate, div;
	u32 mask = BIT(rcg->hid_width) - 1;

	if (rate == 0)
		return -EINVAL;

	*p = clk_get_parent_by_index(hw->clk, f->src);
	*p_rate = parent_rate = __clk_round_rate(*p, rate);

	div = DIV_ROUND_UP((2 * parent_rate), rate) - 1;
	div = min_t(u32, div, mask);

	return calc_rate(parent_rate, 0, 0, 0, div);
}

static int clk_byte_set_rate(struct clk_hw *hw, unsigned long rate,
			 unsigned long parent_rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	struct freq_tbl f = *rcg->freq_tbl;
	unsigned long div;
	u32 mask = BIT(rcg->hid_width) - 1;

	div = DIV_ROUND_UP((2 * parent_rate), rate) - 1;
	div = min_t(u32, div, mask);

	f.pre_div = div;

	return clk_rcg2_configure(rcg, &f);
}

static int clk_byte_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	/* Parent index is set statically in frequency table */
	return clk_byte_set_rate(hw, rate, parent_rate);
}

const struct clk_ops clk_byte_ops = {
	.is_enabled = clk_rcg2_is_enabled,
	.get_parent = clk_rcg2_get_parent,
	.set_parent = clk_rcg2_set_parent,
	.recalc_rate = clk_rcg2_recalc_rate,
	.set_rate = clk_byte_set_rate,
	.set_rate_and_parent = clk_byte_set_rate_and_parent,
	.determine_rate = clk_byte_determine_rate,
};
EXPORT_SYMBOL_GPL(clk_byte_ops);

static const struct frac_entry frac_table_pixel[] = {
	{ 3, 8 },
	{ 2, 9 },
	{ 4, 9 },
	{ 1, 1 },
	{ }
};

static long clk_pixel_determine_rate(struct clk_hw *hw, unsigned long rate,
				 unsigned long *p_rate, struct clk **p)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	unsigned long request, src_rate;
	int delta = 100000;
	const struct freq_tbl *f = rcg->freq_tbl;
	const struct frac_entry *frac = frac_table_pixel;
	struct clk *parent = *p = clk_get_parent_by_index(hw->clk, f->src);

	for (; frac->num; frac++) {
		request = (rate * frac->den) / frac->num;

		src_rate = __clk_round_rate(parent, request);
		if ((src_rate < (request - delta)) ||
			(src_rate > (request + delta)))
			continue;

		*p_rate = src_rate;
		return (src_rate * frac->num) / frac->den;
	}

	return -EINVAL;
}

static int clk_pixel_set_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long parent_rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	struct freq_tbl f = *rcg->freq_tbl;
	const struct frac_entry *frac = frac_table_pixel;
	unsigned long request, src_rate;
	int delta = 100000;
	u32 mask = BIT(rcg->hid_width) - 1;
	u32 hid_div;
	struct clk *parent = clk_get_parent_by_index(hw->clk, f.src);

	for (; frac->num; frac++) {
		request = (rate * frac->den) / frac->num;

		src_rate = __clk_round_rate(parent, request);
		if ((src_rate < (request - delta)) ||
			(src_rate > (request + delta)))
			continue;

		regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG,
				&hid_div);
		f.pre_div = hid_div;
		f.pre_div >>= CFG_SRC_DIV_SHIFT;
		f.pre_div &= mask;
		f.m = frac->num;
		f.n = frac->den;

		return clk_rcg2_configure(rcg, &f);
	}
	return -EINVAL;
}

static int clk_pixel_set_rate_and_parent(struct clk_hw *hw, unsigned long rate,
		unsigned long parent_rate, u8 index)
{
	/* Parent index is set statically in frequency table */
	return clk_pixel_set_rate(hw, rate, parent_rate);
}

const struct clk_ops clk_pixel_ops = {
	.is_enabled = clk_rcg2_is_enabled,
	.get_parent = clk_rcg2_get_parent,
	.set_parent = clk_rcg2_set_parent,
	.recalc_rate = clk_rcg2_recalc_rate,
	.set_rate = clk_pixel_set_rate,
	.set_rate_and_parent = clk_pixel_set_rate_and_parent,
	.determine_rate = clk_pixel_determine_rate,
};
EXPORT_SYMBOL_GPL(clk_pixel_ops);


static int clk_cdiv_rcg2_is_enabled(struct clk_hw *hw)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	u32 cmd;
	int ret;

	ret = regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + CMD_REG, &cmd);
	if (ret)
		return ret;

	return (cmd & CMD_ROOT_OFF) == 0;
}

static u8 clk_cdiv_rcg2_get_parent(struct clk_hw *hw)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	int num_parents = __clk_get_num_parents(hw->clk);
	u32 cfg;
	int i, ret;

	ret = regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG, &cfg);
	if (ret)
		return ret;

	cfg &= CFG_SRC_SEL_MASK;
	cfg >>= CFG_SRC_SEL_SHIFT;

	for (i = 0; i < num_parents; i++)
		if (cfg == rcg->parent_map[i])
			return i;

	return -EINVAL;
}

static int cdiv_update_config(struct clk_cdiv_rcg2 *rcg)
{
	int count, ret;
	u32 cmd;
	struct clk_hw *hw = &rcg->clkr.hw;
	const char *name = __clk_get_name(hw->clk);
	u32 flags;

	flags = __clk_get_flags(hw->clk);

	ret = regmap_update_bits(rcg->clkr.regmap, rcg->cmd_rcgr + CMD_REG,
				 CMD_UPDATE, CMD_UPDATE);
	if (ret)
		return ret;

	if (flags && CLK_RCG2_NO_WAIT) {
		return 0;
	} else {
		/* Wait for update to take effect */
		for (count = 500; count > 0; count--) {
			ret = regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr +
								CMD_REG, &cmd);
			if (ret)
				return ret;
			if (!(cmd & CMD_UPDATE))
				return 0;
			udelay(1);
		}

		WARN(1, "%s: rcg didn't update its configuration.", name);
	}
	return 0;
}


static int clk_cdiv_rcg2_set_parent(struct clk_hw *hw, u8 index)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	int ret;

	ret = regmap_update_bits(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG,
				 CFG_SRC_SEL_MASK,
				 rcg->parent_map[index] << CFG_SRC_SEL_SHIFT);
	if (ret)
		return ret;

	return cdiv_update_config(rcg);
}




static unsigned long
clk_cdiv_rcg2_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	u32 cfg, hid_div , m = 0 , n = 0 , mode = 0 , mask , rate , cdiv;

	regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG, &cfg);

	if (rcg->mnd_width) {
		mask = BIT(rcg->mnd_width) - 1;
		regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + M_REG, &m);
		m &= mask;
		regmap_read(rcg->clkr.regmap, rcg->cmd_rcgr + N_REG, &n);
		n =  ~n;
		n &= mask;
		n += m;
		mode = cfg & CFG_MODE_MASK;
		mode >>= CFG_MODE_SHIFT;
	}

	mask = BIT(rcg->hid_width) - 1;
	hid_div = cfg >> CFG_SRC_DIV_SHIFT;
	hid_div &= mask;
	rate = calc_rate(parent_rate, m, n, mode, hid_div);

	regmap_read(rcg->clkr.regmap, rcg->cdiv.offset, &cdiv);
	cdiv &= (rcg->cdiv.mask << rcg->cdiv.shift);
	cdiv =  (cdiv >> rcg->cdiv.shift);
	if (cdiv)
		rate *= cdiv + 1;
	return rate;
}

static long _cdiv_rcg2_freq_tbl_determine_rate(struct clk_hw *hw,
		const struct freq_tbl *f, unsigned long rate,
		unsigned long *p_rate, struct clk **p)
{
	unsigned long clk_flags;

	f = qcom_find_freq(f, rate);
	if (!f)
		return -EINVAL;

	clk_flags = __clk_get_flags(hw->clk);
	*p = clk_get_parent_by_index(hw->clk, f->src);
	if (clk_flags & CLK_SET_RATE_PARENT) {
		if (f->pre_div)
			rate *= f->pre_div;
		if (f->n) {
			u64 tmp = rate;
			tmp = tmp * f->n;
			do_div(tmp, f->m);
			rate = tmp;
		}
	} else {
		rate =	__clk_get_rate(*p);
	}
	*p_rate = rate;

	return f->freq;
}

static long clk_cdiv_rcg2_determine_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long *p_rate, struct clk **p)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);

	return _cdiv_rcg2_freq_tbl_determine_rate(hw, rcg->freq_tbl,
							rate, p_rate, p);
}

static int clk_cdiv_rcg2_configure(struct clk_cdiv_rcg2 *rcg,
						const struct freq_tbl *f)
{
	u32 cfg, mask;
	u32 i;
	int ret;

	if (rcg->mnd_width && f->n) {
		mask = BIT(rcg->mnd_width) - 1;
		ret = regmap_update_bits(rcg->clkr.regmap,
				rcg->cmd_rcgr + M_REG, mask, f->m);
		if (ret)
			return ret;

		ret = regmap_update_bits(rcg->clkr.regmap,
				rcg->cmd_rcgr + N_REG, mask, ~(f->n - f->m));
		if (ret)
			return ret;

		ret = regmap_update_bits(rcg->clkr.regmap,
				rcg->cmd_rcgr + D_REG, mask, ~f->n);
		if (ret)
			return ret;
	}


	if ((rcg->cdiv.mask) && (f->pre_div > 16)) {
		for (i = 2; i <= 32; i++) {
			if (f->pre_div % i == 0)
				cfg = i;
		}

		if (f->pre_div/cfg > 16)
			return -EINVAL;
		mask = (rcg->cdiv.mask)<<rcg->cdiv.shift;
		ret = regmap_update_bits(rcg->clkr.regmap,
					rcg->cdiv.offset, mask,
				((cfg - 1) << rcg->cdiv.shift) & mask);
		if (ret)
			return ret;
		cfg = (2 * (f->pre_div / cfg)) - 1;
	} else {
		ret = regmap_write(rcg->clkr.regmap, rcg->cdiv.offset, 0x0);
		if (ret)
			return ret;
		cfg = ((2 * f->pre_div) - 1) << CFG_SRC_DIV_SHIFT;
	}

	mask = BIT(rcg->hid_width) - 1;
	mask |= CFG_SRC_SEL_MASK | CFG_MODE_MASK;
	cfg |= rcg->parent_map[f->src] << CFG_SRC_SEL_SHIFT;
	if (rcg->mnd_width && f->n)
		cfg |= CFG_MODE_DUAL_EDGE;
	ret = regmap_update_bits(rcg->clkr.regmap,
			rcg->cmd_rcgr + CFG_REG, mask, cfg);
	if (ret)
		return ret;

	return cdiv_update_config(rcg);
}

static int __clk_cdiv_rcg2_set_rate(struct clk_hw *hw, unsigned long rate)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	const struct freq_tbl *f;

	f = qcom_find_freq(rcg->freq_tbl, rate);
	if (!f)
		return -EINVAL;

	return clk_cdiv_rcg2_configure(rcg, f);
}

static int clk_cdiv_rcg2_set_rate(struct clk_hw *hw, unsigned long rate,
			    unsigned long parent_rate)
{
	return __clk_cdiv_rcg2_set_rate(hw, rate);
}

static int clk_cdiv_rcg2_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	return __clk_cdiv_rcg2_set_rate(hw, rate);
}

const struct clk_ops clk_cdiv_rcg2_ops = {
	.is_enabled			= clk_cdiv_rcg2_is_enabled,
	.get_parent			= clk_cdiv_rcg2_get_parent,
	.set_parent			= clk_cdiv_rcg2_set_parent,
	.recalc_rate			= clk_cdiv_rcg2_recalc_rate,
	.determine_rate			= clk_cdiv_rcg2_determine_rate,
	.set_rate			= clk_cdiv_rcg2_set_rate,
	.set_rate_and_parent		= clk_cdiv_rcg2_set_rate_and_parent,
};
EXPORT_SYMBOL_GPL(clk_cdiv_rcg2_ops);
static int clk_muxr_is_enabled(struct clk_hw *hw)
{
	return 0;
}

static u8 clk_muxr_get_parent(struct clk_hw *hw)
{
	struct clk_muxr_misc *rcg = to_clk_muxr_misc(hw);
	int num_parents = __clk_get_num_parents(hw->clk);
	u32 cfg;
	int i, ret;

	ret = regmap_read(rcg->clkr.regmap, rcg->muxr.offset, &cfg);
	if (ret)
		return ret;

	cfg >>= rcg->muxr.shift;
	cfg &= rcg->muxr.mask;

	for (i = 0; i < num_parents; i++)
		if (cfg == rcg->parent_map[i])
			return i;

	return -EINVAL;
}

static int clk_muxr_set_parent(struct clk_hw *hw, u8 index)
{
	struct clk_muxr_misc *rcg = to_clk_muxr_misc(hw);
	int ret;

	ret = regmap_update_bits(rcg->clkr.regmap, rcg->muxr.offset,
				 (rcg->muxr.mask<<rcg->muxr.shift),
				 rcg->parent_map[index] << rcg->muxr.shift);
	if (ret)
		return ret;

	return 0;
}

static unsigned long
clk_muxr_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_muxr_misc *rcg = to_clk_muxr_misc(hw);
	u32 misc;

	regmap_read(rcg->clkr.regmap, rcg->misc.offset, &misc);
	misc = (misc >> rcg->misc.shift) & (rcg->misc.mask >> rcg->misc.shift);

	return parent_rate * (misc+1);
}

static long clk_muxr_determine_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long *p_rate, struct clk **p)
{
	struct clk_muxr_misc *rcg = to_clk_muxr_misc(hw);
	const struct freq_tbl *f;
	unsigned long clk_flags;

	f = qcom_find_freq(rcg->freq_tbl, rate);
	clk_flags = __clk_get_flags(hw->clk);
	*p = clk_get_parent_by_index(hw->clk, f->src);
	if (clk_flags & CLK_SET_RATE_PARENT) {
		if (f->pre_div)
			rate *= f->pre_div;
	} else {
		rate =	__clk_get_rate(*p);
	}
	*p_rate = rate;

	return f->freq;
}

static int __clk_muxr_set_rate(struct clk_hw *hw, unsigned long rate)
{
	struct clk_muxr_misc *rcg = to_clk_muxr_misc(hw);
	const struct freq_tbl *f;
	int ret;

	f = qcom_find_freq(rcg->freq_tbl, rate);
	if (!f)
		return -EINVAL;

	ret = regmap_update_bits(rcg->clkr.regmap, rcg->muxr.offset,
				rcg->muxr.mask << rcg->muxr.shift,
				rcg->parent_map[f->src] << rcg->muxr.shift);
	if (ret)
		return ret;

	ret = regmap_update_bits(rcg->clkr.regmap, rcg->misc.offset,
				rcg->misc.mask << rcg->misc.shift,
				(f->pre_div - 1) << rcg->misc.shift);
	if (ret)
		return ret;

	return 0;
}

static int clk_muxr_set_rate(struct clk_hw *hw, unsigned long rate,
			    unsigned long parent_rate)
{
	return __clk_muxr_set_rate(hw, rate);
}

static int clk_muxr_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	return __clk_muxr_set_rate(hw, rate);
}


const struct clk_ops clk_muxr_misc_ops = {
	.is_enabled	=	clk_muxr_is_enabled,
	.get_parent	=	clk_muxr_get_parent,
	.set_parent	=	clk_muxr_set_parent,
	.recalc_rate	=	clk_muxr_recalc_rate,
	.determine_rate	=	clk_muxr_determine_rate,
	.set_rate	=	clk_muxr_set_rate,
	.set_rate_and_parent	=	clk_muxr_set_rate_and_parent,
};
EXPORT_SYMBOL_GPL(clk_muxr_misc_ops);
