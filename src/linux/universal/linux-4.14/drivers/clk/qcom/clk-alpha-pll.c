/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
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
#include <linux/export.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>
#include <linux/delay.h>

#include "clk-alpha-pll.h"
#include "common.h"

#define PLL_MODE		0x00
# define PLL_OUTCTRL		BIT(0)
# define PLL_BYPASSNL		BIT(1)
# define PLL_RESET_N		BIT(2)
# define PLL_OFFLINE_REQ	BIT(7)
# define PLL_LOCK_COUNT_SHIFT	8
# define PLL_LOCK_COUNT_MASK	0x3f
# define PLL_BIAS_COUNT_SHIFT	14
# define PLL_BIAS_COUNT_MASK	0x3f
# define PLL_VOTE_FSM_ENA	BIT(20)
# define PLL_FSM_ENA		BIT(20)
# define PLL_VOTE_FSM_RESET	BIT(21)
# define PLL_UPDATE		BIT(22)
# define PLL_UPDATE_BYPASS	BIT(23)
# define PLL_OFFLINE_ACK	BIT(28)
# define ALPHA_PLL_ACK_LATCH	BIT(29)
# define PLL_ACTIVE_FLAG	BIT(30)
# define PLL_LOCK_DET		BIT(31)

#define PLL_L_VAL		0x04
#define PLL_ALPHA_VAL		0x08
#define PLL_ALPHA_VAL_U		0x0c

#define PLL_USER_CTL		0x10
# define PLL_POST_DIV_SHIFT	8
# define PLL_ALPHA_EN		BIT(24)
# define PLL_ALPHA_MODE		BIT(25)
# define PLL_VCO_SHIFT		20
# define PLL_VCO_MASK		0x3

#define PLL_USER_CTL_U		0x14

#define PLL_CONFIG_CTL		0x18
#define PLL_CONFIG_CTL_U	0x20
#define PLL_TEST_CTL		0x1c
#define PLL_TEST_CTL_U		0x20
#define PLL_STATUS		0x24

#define PLL_HUAYRA_M_WIDTH		8
#define PLL_HUAYRA_M_SHIFT		8
#define PLL_HUAYRA_M_MASK		0xff
#define PLL_HUAYRA_N_SHIFT		0
#define PLL_HUAYRA_N_MASK		0xff
#define PLL_HUAYRA_ALPHA_WIDTH		16

/*
 * Even though 40 bits are present, use only 32 for ease of calculation.
 */
#define ALPHA_REG_BITWIDTH	40
#define ALPHA_REG_16BIT_WIDTH	16
#define ALPHA_BITWIDTH		32

#define PLL_MODE_REG(pll)	(pll->offset + PLL_MODE)
#define PLL_L_REG(pll)		(pll->offset + PLL_L_VAL + pll->l_offset)
#define PLL_ALPHA_REG(pll)	(pll->offset + PLL_ALPHA_VAL + \
				 pll->alpha_offset)
#define PLL_ALPHA_U_REG(pll)	(pll->offset + PLL_ALPHA_VAL_U + \
				 pll->alpha_offset)
#define PLL_USER_CTL_REG(pll)	(pll->offset + PLL_USER_CTL + \
				 pll->alpha_offset)
#define PLL_USER_CTL_U_REG(pll)	(pll->offset + PLL_USER_CTL_U + \
				 pll->alpha_offset)
#define PLL_CONFIG_CTL_REG(pll)	(pll->offset + PLL_CONFIG_CTL + \
				 pll->alpha_offset)
#define PLL_TEST_CTL_REG(pll)	(pll->offset + PLL_TEST_CTL + \
				 pll->alpha_offset)
#define PLL_TEST_CTL_U_REG(pll)	(pll->offset + PLL_TEST_U_CTL + \
				 pll->alpha_offset)
#define PLL_STATUS_REG(pll)	(pll->offset + PLL_STATUS + pll->alpha_offset)
#define PLL_CONFIG_CTL_U_REG(pll)	(pll->offset + PLL_CONFIG_CTL_U + \
					 pll->alpha_offset)

#define to_clk_alpha_pll(_hw) container_of(to_clk_regmap(_hw), \
					   struct clk_alpha_pll, clkr)

#define to_clk_alpha_pll_postdiv(_hw) container_of(to_clk_regmap(_hw), \
					   struct clk_alpha_pll_postdiv, clkr)

static int wait_for_pll(struct clk_alpha_pll *pll, u32 mask, bool inverse,
			const char *action)
{
	u32 val;
	int count;
	int ret;
	const char *name = clk_hw_get_name(&pll->clkr.hw);

	ret = regmap_read(pll->clkr.regmap, PLL_MODE_REG(pll), &val);
	if (ret)
		return ret;

	for (count = 100; count > 0; count--) {
		ret = regmap_read(pll->clkr.regmap, PLL_MODE_REG(pll), &val);
		if (ret)
			return ret;
		if (inverse && !(val & mask))
			return 0;
		else if ((val & mask) == mask)
			return 0;

		udelay(1);
	}

	WARN(1, "%s failed to %s!\n", name, action);
	return -ETIMEDOUT;
}

#define wait_for_pll_enable_active(pll) \
	wait_for_pll(pll, PLL_ACTIVE_FLAG, 0, "enable")

#define wait_for_pll_enable_lock(pll) \
	wait_for_pll(pll, PLL_LOCK_DET, 0, "enable")

#define wait_for_pll_disable(pll) \
	wait_for_pll(pll, PLL_ACTIVE_FLAG, 1, "disable")

#define wait_for_pll_offline(pll) \
	wait_for_pll(pll, PLL_OFFLINE_ACK, 0, "offline")

#define wait_for_pll_update(pll) \
	wait_for_pll(pll, PLL_UPDATE, 0, "update")

#define wait_for_pll_update_ack_set(pll) \
	wait_for_pll(pll, ALPHA_PLL_ACK_LATCH, 1, "update_ack_set")

#define wait_for_pll_update_ack_clear(pll) \
	wait_for_pll(pll, ALPHA_PLL_ACK_LATCH, 0, "update_ack_clear")

void clk_alpha_pll_configure(struct clk_alpha_pll *pll, struct regmap *regmap,
			     const struct alpha_pll_config *config)
{
	u32 val, mask;

	regmap_write(regmap, PLL_L_REG(pll), config->l);
	regmap_write(regmap, PLL_ALPHA_REG(pll), config->alpha);
	regmap_write(regmap, PLL_CONFIG_CTL_REG(pll), config->config_ctl_val);
	regmap_write(regmap, PLL_CONFIG_CTL_U_REG(pll),
		     config->config_ctl_hi_val);

	val = config->main_output_mask;
	val |= config->aux_output_mask;
	val |= config->aux2_output_mask;
	val |= config->early_output_mask;
	val |= config->pre_div_val;
	val |= config->post_div_val;
	val |= config->vco_val;

	mask = config->main_output_mask;
	mask |= config->aux_output_mask;
	mask |= config->aux2_output_mask;
	mask |= config->early_output_mask;
	mask |= config->pre_div_mask;
	mask |= config->post_div_mask;
	mask |= config->vco_mask;

	regmap_update_bits(regmap, PLL_USER_CTL_REG(pll), mask, val);

	if (pll->flags & SUPPORTS_FSM_MODE)
		qcom_pll_set_fsm_mode(regmap, PLL_MODE_REG(pll), 6, 0);
}

static int clk_alpha_pll_hwfsm_enable(struct clk_hw *hw)
{
	int ret;
	u32 val;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);

	ret = regmap_read(pll->clkr.regmap, PLL_MODE_REG(pll), &val);
	if (ret)
		return ret;

	val |= PLL_FSM_ENA;

	if (pll->flags & SUPPORTS_OFFLINE_REQ)
		val &= ~PLL_OFFLINE_REQ;

	ret = regmap_write(pll->clkr.regmap, PLL_MODE_REG(pll), val);
	if (ret)
		return ret;

	/* Make sure enable request goes through before waiting for update */
	mb();

	return wait_for_pll_enable_active(pll);
}

static void clk_alpha_pll_hwfsm_disable(struct clk_hw *hw)
{
	int ret;
	u32 val, off;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);

	off = pll->offset;
	ret = regmap_read(pll->clkr.regmap, PLL_MODE_REG(pll), &val);
	if (ret)
		return;

	if (pll->flags & SUPPORTS_OFFLINE_REQ) {
		ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE_REG(pll),
					 PLL_OFFLINE_REQ, PLL_OFFLINE_REQ);
		if (ret)
			return;

		ret = wait_for_pll_offline(pll);
		if (ret)
			return;
	}

	/* Disable hwfsm */
	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE_REG(pll),
				 PLL_FSM_ENA, 0);
	if (ret)
		return;

	wait_for_pll_disable(pll);
}

static int pll_is_enabled(struct clk_hw *hw, u32 mask)
{
	int ret;
	u32 val;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);

	ret = regmap_read(pll->clkr.regmap, PLL_MODE_REG(pll), &val);
	if (ret)
		return ret;

	return !!(val & mask);
}

static int clk_alpha_pll_hwfsm_is_enabled(struct clk_hw *hw)
{
	return pll_is_enabled(hw, PLL_ACTIVE_FLAG);
}

static int clk_alpha_pll_is_enabled(struct clk_hw *hw)
{
	return pll_is_enabled(hw, PLL_LOCK_DET);
}

static int clk_alpha_pll_enable(struct clk_hw *hw)
{
	int ret;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val, mask;

	mask = PLL_OUTCTRL | PLL_RESET_N | PLL_BYPASSNL;
	ret = regmap_read(pll->clkr.regmap, PLL_MODE_REG(pll), &val);
	if (ret)
		return ret;

	/* If in FSM mode, just vote for it */
	if (val & PLL_VOTE_FSM_ENA) {
		ret = clk_enable_regmap(hw);
		if (ret)
			return ret;
		return wait_for_pll_enable_active(pll);
	}

	/* Skip if already enabled */
	if ((val & mask) == mask)
		return 0;

	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE_REG(pll),
				 PLL_BYPASSNL, PLL_BYPASSNL);
	if (ret)
		return ret;

	/*
	 * H/W requires a 5us delay between disabling the bypass and
	 * de-asserting the reset.
	 */
	mb();
	udelay(5);

	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE_REG(pll),
				 PLL_RESET_N, PLL_RESET_N);
	if (ret)
		return ret;

	ret = wait_for_pll_enable_lock(pll);
	if (ret)
		return ret;

	ret = regmap_update_bits(pll->clkr.regmap, PLL_MODE_REG(pll),
				 PLL_OUTCTRL, PLL_OUTCTRL);

	/* Ensure that the write above goes through before returning. */
	mb();
	return ret;
}

static void clk_alpha_pll_disable(struct clk_hw *hw)
{
	int ret;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 val, mask;

	ret = regmap_read(pll->clkr.regmap, PLL_MODE_REG(pll), &val);
	if (ret)
		return;

	/* If in FSM mode, just unvote it */
	if (val & PLL_VOTE_FSM_ENA) {
		clk_disable_regmap(hw);
		return;
	}

	mask = PLL_OUTCTRL;
	regmap_update_bits(pll->clkr.regmap, PLL_MODE_REG(pll), mask, 0);

	/* Delay of 2 output clock ticks required until output is disabled */
	mb();
	udelay(1);

	mask = PLL_RESET_N | PLL_BYPASSNL;
	regmap_update_bits(pll->clkr.regmap, PLL_MODE_REG(pll), mask, 0);
}

static unsigned long
alpha_pll_calc_rate(u64 prate, u32 l, u32 a, u32 alpha_width)
{
	return (prate * l) + ((prate * a) >>
		(alpha_width > ALPHA_BITWIDTH ? ALPHA_BITWIDTH : alpha_width));
}

static unsigned long
alpha_pll_round_rate(unsigned long rate, unsigned long prate, u32 *l, u64 *a,
			u32 alpha_width)
{
	u64 remainder;
	u64 quotient;

	quotient = rate;
	remainder = do_div(quotient, prate);
	*l = quotient;

	if (!remainder) {
		*a = 0;
		return rate;
	}

	/* Upper ALPHA_BITWIDTH bits of Alpha */
	quotient = remainder <<
			(alpha_width > ALPHA_BITWIDTH ?
			 ALPHA_BITWIDTH : alpha_width);

	remainder = do_div(quotient, prate);

	if (remainder)
		quotient++;

	*a = quotient;
	return alpha_pll_calc_rate(prate, *l, *a, alpha_width);
}

static const struct pll_vco *
alpha_pll_find_vco(const struct clk_alpha_pll *pll, unsigned long rate)
{
	const struct pll_vco *v = pll->vco_table;
	const struct pll_vco *end = v + pll->num_vco;

	for (; v < end; v++)
		if (rate >= v->min_freq && rate <= v->max_freq)
			return v;

	return NULL;
}

static unsigned long
clk_alpha_pll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	u32 l, low, high, ctl, alpha_width;
	u64 a = 0, prate = parent_rate;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);

	regmap_read(pll->clkr.regmap, PLL_L_REG(pll), &l);
	alpha_width = pll->flags & SUPPORTS_16BIT_ALPHA ?
				ALPHA_REG_16BIT_WIDTH : ALPHA_REG_BITWIDTH;

	regmap_read(pll->clkr.regmap, PLL_USER_CTL_REG(pll), &ctl);
	if (ctl & PLL_ALPHA_EN) {
		regmap_read(pll->clkr.regmap, PLL_ALPHA_REG(pll), &low);
		if (alpha_width > ALPHA_BITWIDTH) {
			regmap_read(pll->clkr.regmap, PLL_ALPHA_U_REG(pll),
				    &high);
			a = (u64)high << ALPHA_BITWIDTH | low;
			a >>= alpha_width - ALPHA_BITWIDTH;
		} else {
			a = low;
		}
	}

	return alpha_pll_calc_rate(prate, l, a, alpha_width);
}

static int clk_alpha_pll_set_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long prate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	const struct pll_vco *vco;
	u32 l, alpha_width;
	u64 a;

	alpha_width = pll->flags & SUPPORTS_16BIT_ALPHA ?
				ALPHA_REG_16BIT_WIDTH : ALPHA_REG_BITWIDTH;
	rate = alpha_pll_round_rate(rate, prate, &l, &a, alpha_width);
	vco = alpha_pll_find_vco(pll, rate);
	if (!vco) {
		pr_err("alpha pll not in a valid vco range\n");
		return -EINVAL;
	}

	regmap_write(pll->clkr.regmap, PLL_L_REG(pll), l);

	if (alpha_width > ALPHA_BITWIDTH) {
		a <<= (alpha_width - ALPHA_BITWIDTH);
		regmap_update_bits(pll->clkr.regmap, PLL_ALPHA_U_REG(pll),
				   GENMASK(0, alpha_width - ALPHA_BITWIDTH - 1),
				   a >> ALPHA_BITWIDTH);
	}

	regmap_update_bits(pll->clkr.regmap, PLL_ALPHA_REG(pll),
			   GENMASK(0, alpha_width - 1), a);

	regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL_REG(pll),
			   PLL_VCO_MASK << PLL_VCO_SHIFT,
			   vco->val << PLL_VCO_SHIFT);

	regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL_REG(pll),
			   PLL_ALPHA_EN, PLL_ALPHA_EN);

	return 0;
}

static long clk_alpha_pll_round_rate(struct clk_hw *hw, unsigned long rate,
				     unsigned long *prate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 l, alpha_width;
	u64 a;
	unsigned long min_freq, max_freq;

	alpha_width = pll->flags & SUPPORTS_16BIT_ALPHA ?
				ALPHA_REG_16BIT_WIDTH : ALPHA_REG_BITWIDTH;

	rate = alpha_pll_round_rate(rate, *prate, &l, &a, alpha_width);
	if (alpha_pll_find_vco(pll, rate))
		return rate;

	min_freq = pll->vco_table[0].min_freq;
	max_freq = pll->vco_table[pll->num_vco - 1].max_freq;

	return clamp(rate, min_freq, max_freq);
}

static unsigned long
alpha_pll_huayra_calc_rate(u64 prate, u32 l, u32 a)
{
	/*
	 * a contains 16 bit alpha_val in two’s compliment number in the range
	 * of [-0.5, 0.5).
	 */
	if (a >= BIT(PLL_HUAYRA_ALPHA_WIDTH - 1))
		return (prate * l) -
		       (prate * (BIT(PLL_HUAYRA_ALPHA_WIDTH) - a) >>
			PLL_HUAYRA_ALPHA_WIDTH);
	else
		return (prate * l) + (prate * a >> PLL_HUAYRA_ALPHA_WIDTH);
}

static unsigned long
alpha_pll_huayra_round_rate(unsigned long rate, unsigned long prate,
			    u32 *l, u32 *a)
{
	u64 remainder;
	u64 quotient;

	quotient = rate;
	remainder = do_div(quotient, prate);
	*l = quotient;

	if (!remainder) {
		*a = 0;
		return rate;
	}

	quotient = remainder << PLL_HUAYRA_ALPHA_WIDTH;
	remainder = do_div(quotient, prate);

	if (remainder)
		quotient++;

	/*
	 * alpha_val should be in two’s compliment number in the range
	 * of [-0.5, 0.5) so if quotient >= 0.5 then increment the l value
	 * specify the alpha value to be subtracted.
	 */
	if (quotient >= BIT(PLL_HUAYRA_ALPHA_WIDTH - 1)) {
		*l += 1;
		quotient = BIT(PLL_HUAYRA_ALPHA_WIDTH) - quotient;
	}

	*a = quotient;
	return alpha_pll_huayra_calc_rate(prate, *l, *a);
}

static unsigned long
clk_alpha_pll_huayra_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	u32 l, alpha, ctl, alpha_m, alpha_n;
	u64 rate = parent_rate, tmp;
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);

	regmap_read(pll->clkr.regmap, PLL_L_REG(pll), &l);
	regmap_read(pll->clkr.regmap, PLL_USER_CTL_REG(pll), &ctl);

	if (ctl & PLL_ALPHA_EN) {
		regmap_read(pll->clkr.regmap, PLL_ALPHA_REG(pll), &alpha);
		/*
		 * Depending upon alpha_mode, it can be treated as M/N value or
		 * as a two’s compliment number. When
		 * alpha_mode=1 pll_alpha_val<15:8>=M & pll_apla_val<7:0>=N
		 *		Fout=FIN*(L+(M/N))
		 * M is a signed number (-128 to 127) and N is unsigned
		 * (0 to 255). M/N has to be within +/-0.5.
		 *
		 * alpha_mode=0, it is a two’s compliment number in the range
		 * of [-0.5, 0.5).
		 *		Fout=FIN*(L+(alpha_val)/2^16),where alpha_val is
		 * two’s compliment number.
		 */
		if (ctl & PLL_ALPHA_MODE) {
			alpha_m = alpha >> PLL_HUAYRA_M_SHIFT &
				  PLL_HUAYRA_M_MASK;
			alpha_n = alpha >> PLL_HUAYRA_N_SHIFT &
				  PLL_HUAYRA_N_MASK;

			rate *= l;
			tmp = parent_rate;
			if (alpha_m >= BIT(PLL_HUAYRA_M_WIDTH - 1)) {
				alpha_m = BIT(PLL_HUAYRA_M_WIDTH) - alpha_m;
				tmp *= alpha_m;
				do_div(tmp, alpha_n);
				rate -= tmp;
			} else {
				tmp *= alpha_m;
				do_div(tmp, alpha_n);
				rate += tmp;
			}

			return rate;
		} else {
			return alpha_pll_huayra_calc_rate(rate, l, alpha);
		}
	}

	return alpha_pll_huayra_calc_rate(rate, l, alpha);
}

static int clk_alpha_pll_huayra_set_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long prate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 l, a, cur_alpha;

	rate = alpha_pll_huayra_round_rate(rate, prate, &l, &a);

	regmap_read(pll->clkr.regmap, PLL_ALPHA_REG(pll), &cur_alpha);

	/*
	 * Huayra PLL supports PLL dynamic programming. User can change L_VAL,
	 * without having to go through the power on sequence.
	 */
	if (cur_alpha == a) {
		regmap_write(pll->clkr.regmap, PLL_L_REG(pll), l);
		/* Ensure that the write above goes to detect L val change. */
		mb();
		return wait_for_pll_enable_lock(pll);
	}

	if (clk_hw_is_enabled(hw)) {
		pr_err("clock needs to be gated %s\n", clk_hw_get_name(hw));
		return -EBUSY;
	}

	regmap_write(pll->clkr.regmap, PLL_ALPHA_REG(pll), a);

	if (a == 0)
		regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL_REG(pll),
				   PLL_ALPHA_EN, 0x0);
	else
		regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL_REG(pll),
				   PLL_ALPHA_EN | PLL_ALPHA_MODE, PLL_ALPHA_EN);

	return 0;
}

static long clk_alpha_pll_huayra_round_rate(struct clk_hw *hw,
					    unsigned long rate,
					    unsigned long *prate)
{
	u32 l, a;

	return alpha_pll_huayra_round_rate(rate, *prate, &l, &a);
}

static int clk_alpha_pll_brammo_set_rate(struct clk_hw *hw, unsigned long rate,
					 unsigned long prate)
{
	struct clk_alpha_pll *pll = to_clk_alpha_pll(hw);
	u32 l, mode;
	int ret;
	u64 a;

	rate = alpha_pll_round_rate(rate, prate, &l, &a, ALPHA_REG_BITWIDTH);

	regmap_read(pll->clkr.regmap, PLL_MODE_REG(pll), &mode);
	regmap_write(pll->clkr.regmap, PLL_L_REG(pll), l);

	a <<= ALPHA_REG_BITWIDTH - ALPHA_BITWIDTH;

	regmap_write(pll->clkr.regmap, PLL_ALPHA_REG(pll), a);
	regmap_write(pll->clkr.regmap, PLL_ALPHA_U_REG(pll), a >> 32);
	regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL_REG(pll),
			   PLL_ALPHA_EN, PLL_ALPHA_EN);
	regmap_update_bits(pll->clkr.regmap, PLL_MODE_REG(pll), PLL_UPDATE,
			   PLL_UPDATE);

	/* Make sure PLL_UPDATE request goes through*/
	mb();

	/*
	 * PLL will latch the new L, Alpha and freq control word.
	 * PLL will respond by raising PLL_ACK_LATCH output when new programming
	 * has been latched in and PLL is being updated. When
	 * UPDATE_LOGIC_BYPASS bit is not set, PLL_UPDATE will be cleared
	 * automatically by hardware when PLL_ACK_LATCH is asserted by PLL.
	 */
	if (!(mode & PLL_UPDATE_BYPASS))
		return wait_for_pll_update(pll);

	ret = wait_for_pll_update_ack_set(pll);
	if (ret)
		return ret;

	regmap_update_bits(pll->clkr.regmap, PLL_MODE_REG(pll), PLL_UPDATE, 0);

	/* Make sure PLL_UPDATE request goes through*/
	mb();

	ret = wait_for_pll_update_ack_clear(pll);
	if (ret)
		return ret;

	return 0;
}

const struct clk_ops clk_alpha_pll_ops = {
	.enable = clk_alpha_pll_enable,
	.disable = clk_alpha_pll_disable,
	.is_enabled = clk_alpha_pll_is_enabled,
	.recalc_rate = clk_alpha_pll_recalc_rate,
	.round_rate = clk_alpha_pll_round_rate,
	.set_rate = clk_alpha_pll_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_ops);

const struct clk_ops clk_alpha_pll_hwfsm_ops = {
	.enable = clk_alpha_pll_hwfsm_enable,
	.disable = clk_alpha_pll_hwfsm_disable,
	.is_enabled = clk_alpha_pll_hwfsm_is_enabled,
	.recalc_rate = clk_alpha_pll_recalc_rate,
	.round_rate = clk_alpha_pll_round_rate,
	.set_rate = clk_alpha_pll_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_hwfsm_ops);

const struct clk_ops clk_alpha_pll_huayra_ops = {
	.enable = clk_alpha_pll_enable,
	.disable = clk_alpha_pll_disable,
	.is_enabled = clk_alpha_pll_is_enabled,
	.recalc_rate = clk_alpha_pll_huayra_recalc_rate,
	.round_rate = clk_alpha_pll_huayra_round_rate,
	.set_rate = clk_alpha_pll_huayra_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_huayra_ops);

const struct clk_ops clk_alpha_pll_brammo_ops = {
	.enable = clk_alpha_pll_enable,
	.disable = clk_alpha_pll_disable,
	.is_enabled = clk_alpha_pll_is_enabled,
	.recalc_rate = clk_alpha_pll_recalc_rate,
	.round_rate = clk_alpha_pll_round_rate,
	.set_rate = clk_alpha_pll_brammo_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_brammo_ops);

static unsigned long
clk_alpha_pll_postdiv_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_alpha_pll_postdiv *pll = to_clk_alpha_pll_postdiv(hw);
	u32 ctl;

	regmap_read(pll->clkr.regmap, PLL_USER_CTL_REG(pll), &ctl);

	ctl >>= PLL_POST_DIV_SHIFT;
	ctl &= BIT(pll->width) - 1;

	return parent_rate >> fls(ctl);
}

static const struct clk_div_table clk_alpha_div_table[] = {
	{ 0x0, 1 },
	{ 0x1, 2 },
	{ 0x3, 4 },
	{ 0x7, 8 },
	{ 0xf, 16 },
	{ }
};

static const struct clk_div_table clk_alpha_div_table_2bit[] = {
	{ 0x0, 1 },
	{ 0x1, 2 },
	{ 0x3, 4 },
	{ }
};

static long
clk_alpha_pll_postdiv_round_rate(struct clk_hw *hw, unsigned long rate,
				 unsigned long *prate)
{
	struct clk_alpha_pll_postdiv *pll = to_clk_alpha_pll_postdiv(hw);

	if (pll->width == 2)
		return divider_round_rate(hw, rate, prate,
				clk_alpha_div_table_2bit, pll->width,
				CLK_DIVIDER_POWER_OF_TWO);

	return divider_round_rate(hw, rate, prate, clk_alpha_div_table,
				  pll->width, CLK_DIVIDER_POWER_OF_TWO);
}

static int clk_alpha_pll_postdiv_set_rate(struct clk_hw *hw, unsigned long rate,
					  unsigned long parent_rate)
{
	struct clk_alpha_pll_postdiv *pll = to_clk_alpha_pll_postdiv(hw);
	int div;

	/* 16 -> 0xf, 8 -> 0x7, 4 -> 0x3, 2 -> 0x1, 1 -> 0x0 */
	div = DIV_ROUND_UP_ULL((u64)parent_rate, rate) - 1;

	return regmap_update_bits(pll->clkr.regmap, PLL_USER_CTL_REG(pll),
				  (BIT(pll->width) - 1) << PLL_POST_DIV_SHIFT,
				  div << PLL_POST_DIV_SHIFT);
}

const struct clk_ops clk_alpha_pll_postdiv_ops = {
	.recalc_rate = clk_alpha_pll_postdiv_recalc_rate,
	.round_rate = clk_alpha_pll_postdiv_round_rate,
	.set_rate = clk_alpha_pll_postdiv_set_rate,
};
EXPORT_SYMBOL_GPL(clk_alpha_pll_postdiv_ops);
