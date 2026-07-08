// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * CMN PLL block expects the reference clock from on-board Wi-Fi block,
 * and supplies fixed rate clocks as output to the networking hardware
 * blocks and to GCC. The networking related blocks include PPE (packet
 * process engine), the externally connected PHY or switch devices, and
 * the PCS.
 *
 * On the IPQ9574 SoC, there are three clocks with 50 MHZ and one clock
 * with 25 MHZ which are output from the CMN PLL to Ethernet PHY (or switch),
 * and one clock with 353 MHZ to PPE. The other fixed rate output clocks
 * are supplied to GCC (24 MHZ as XO and 32 KHZ as sleep clock), and to PCS
 * with 31.25 MHZ.
 *
 * On the IPQ5424 SoC, there is an output clock from CMN PLL to PPE at 375 MHZ,
 * and an output clock to NSS (network subsystem) at 300 MHZ. The other output
 * clocks from CMN PLL on IPQ5424 are the same as IPQ9574.
 *
 * On the IPQ5332 SoC, the CMN PLL provides a single 50 MHZ clock output to
 * the Ethernet PHY (or switch) via the UNIPHY (PCS). It also supplies a 200
 * MHZ clock to the PPE. The remaining fixed-rate clocks to the GCC and PCS
 * are the same as those in the IPQ9574 SoC.
 *
 *               +---------+
 *               |   GCC   |
 *               +--+---+--+
 *           AHB CLK|   |SYS CLK
 *                  V   V
 *          +-------+---+------+
 *          |                  +-------------> eth0-50mhz
 * REF CLK  |     IPQ9574      |
 * -------->+                  +-------------> eth1-50mhz
 *          |  CMN PLL block   |
 *          |                  +-------------> eth2-50mhz
 *          |                  |
 *          +----+----+----+---+-------------> eth-25mhz
 *               |    |    |
 *               V    V    V
 *              GCC  PCS  NSS/PPE
 */

#include <linux/bitfield.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm_clock.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/string.h>

#include <dt-bindings/clock/qcom,ipq-cmn-pll.h>
#include <dt-bindings/clock/qcom,ipq5018-cmn-pll.h>
#include <dt-bindings/clock/qcom,ipq5210-cmn-pll.h>
#include <dt-bindings/clock/qcom,ipq5332-cmn-pll.h>
#include <dt-bindings/clock/qcom,ipq5424-cmn-pll.h>
#include <dt-bindings/clock/qcom,ipq6018-cmn-pll.h>
#include <dt-bindings/clock/qcom,ipq8074-cmn-pll.h>
#include <dt-bindings/clock/qcom,ipq9650-cmn-pll.h>

#define CMN_PLL_REFCLK_SRC_SELECTION		0x28
#define CMN_PLL_REFCLK_SRC_DIV			GENMASK(9, 8)

#define CMN_PLL_LOCKED				0x64
#define CMN_PLL_CLKS_LOCKED			BIT(8)

#define CMN_PLL_NSS_PPE_FREQ_CTRL		0x98
#define CMN_PLL_NSS_CLK_SEL			GENMASK(13, 8)
#define CMN_PLL_PPE_CLK_SEL			GENMASK(5, 0)
/* CMNPLL divider for NSS/PPE: 6-bit field, valid range 8-63. */
#define CMN_PLL_NSS_PPE_DIV_MIN			8
#define CMN_PLL_NSS_PPE_DIV_MAX			63

#define CMN_PLL_PCS_CLK_CTRL			0x41c
#define CMN_PLL_PCS2_CLK_DIVSEL			GENMASK(8, 7)
#define CMN_PLL_PCS1_CLK_DIVSEL			GENMASK(6, 5)
#define CMN_PLL_PCS0_CLK_DIVSEL			GENMASK(4, 3)
#define CMN_PLL_PCS2_CLK_EN			BIT(2)
#define CMN_PLL_PCS1_CLK_EN			BIT(1)
#define CMN_PLL_PCS0_CLK_EN			BIT(0)

#define CMN_PLL_PON_CONFIG			0x42c
#define CMN_PLL_GEPHY_312P5M_125M_SEL		BIT(10)
#define CMN_PLL_PON_MODE_SEL			BIT(9)
#define CMN_PLL_PON_EN				BIT(8)
#define CMN_PLL_PON_DIV_CTRL			GENMASK(7, 0)

#define CMN_PLL_POWER_ON_AND_RESET		0x780
#define CMN_ANA_EN_SW_RSTN			BIT(6)

#define CMN_PLL_REFCLK_CONFIG			0x784
#define CMN_PLL_REFCLK_EXTERNAL			BIT(9)
#define CMN_PLL_REFCLK_DIV			GENMASK(8, 4)
#define CMN_PLL_REFCLK_INDEX			GENMASK(3, 0)

#define CMN_PLL_CTRL				0x78c
#define CMN_PLL_CTRL_LOCK_DETECT_EN		BIT(15)

#define CMN_PLL_DIVIDER_CTRL			0x794
#define CMN_PLL_DIVIDER_CTRL_FACTOR		GENMASK(9, 0)

#define CMN_PLL_OUTPUT_RELATED_1		0x79c
#define CLK25M_EN_BIT				15
#define CLK50M_EN_BIT3_BIT			14
#define CLK250M_EN_BIT				13
#define CLK31P25M_EN_BIT			12
#define CLK50M_EN_BIT				11
#define CLK50M_EN_BIT2_BIT			10

#define CMN_PLL_OUTPUT_RELATED_2		0x7a0
#define CMN_PLL_OUTPUT_MUX_SEL			BIT(4)

/**
 * struct cmn_pll_fixed_output_clk - CMN PLL output clocks information
 * @id:	Clock specifier to be supplied
 * @name: Clock name to be registered
 * @rate: Clock rate
 * @enable_bit: Enable bit in register for gate clock, -1 if not a gate clock
 */
struct cmn_pll_fixed_output_clk {
	unsigned int id;
	const char *name;
	unsigned long rate;
	int enable_bit;
};

/**
 * struct clk_cmn_pll - CMN PLL hardware specific data
 * @regmap: hardware regmap.
 * @base: register base address for gate clock registration
 * @hw: handle between common and hardware-specific interfaces
 *
 * This structure is used for all CMN PLL-derived clocks including
 * the main PLL, PON reference clock, NSS clock, PPE clock, and PCS clocks.
 */
struct clk_cmn_pll {
	struct regmap *regmap;
	void __iomem *base;
	struct clk_hw hw;
};

#define CLK_PLL_OUTPUT_RAW(_id, _name, _rate, _bit) {		\
	.id =		_id,				\
	.name =		_name,				\
	.rate =		_rate,				\
	.enable_bit =	_bit,				\
}

#define CLK_PLL_OUTPUT(_id, _name, _rate)	\
	CLK_PLL_OUTPUT_RAW(_id, _name, _rate, -1)

#define CLK_PLL_GATE(_id, _name, _rate, _bit)	\
	CLK_PLL_OUTPUT_RAW(_id, _name, _rate, _bit)

#define to_clk_cmn_pll(_hw) container_of(_hw, struct clk_cmn_pll, hw)

static const struct regmap_config ipq_cmn_pll_regmap_config = {
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.max_register = 0x7fc,
};

static const struct cmn_pll_fixed_output_clk ipq5018_output_clks[] = {
	CLK_PLL_OUTPUT(IPQ5018_XO_24MHZ_CLK, "xo-24mhz", 24000000UL),
	CLK_PLL_OUTPUT(IPQ5018_SLEEP_32KHZ_CLK, "sleep-32khz", 32000UL),
	CLK_PLL_OUTPUT(IPQ5018_ETH_50MHZ_CLK, "eth-50mhz", 50000000UL),
	{ /* Sentinel */ }
};

static const struct cmn_pll_fixed_output_clk ipq5332_output_clks[] = {
	CLK_PLL_OUTPUT(IPQ5332_XO_24MHZ_CLK, "xo-24mhz", 24000000UL),
	CLK_PLL_OUTPUT(IPQ5332_SLEEP_32KHZ_CLK, "sleep-32khz", 32000UL),
	CLK_PLL_OUTPUT(IPQ5332_PCS_31P25MHZ_CLK, "pcs-31p25mhz", 31250000UL),
	CLK_PLL_OUTPUT(IPQ5332_NSS_300MHZ_CLK, "nss-300mhz", 300000000UL),
	CLK_PLL_OUTPUT(IPQ5332_PPE_200MHZ_CLK, "ppe-200mhz", 200000000UL),
	CLK_PLL_OUTPUT(IPQ5332_ETH_50MHZ_CLK, "eth-50mhz", 50000000UL),
	{ /* Sentinel */ }
};

static const struct cmn_pll_fixed_output_clk ipq5424_output_clks[] = {
	CLK_PLL_OUTPUT(IPQ5424_XO_24MHZ_CLK, "xo-24mhz", 24000000UL),
	CLK_PLL_OUTPUT(IPQ5424_SLEEP_32KHZ_CLK, "sleep-32khz", 32000UL),
	CLK_PLL_OUTPUT(IPQ5424_PCS_31P25MHZ_CLK, "pcs-31p25mhz", 31250000UL),
	CLK_PLL_OUTPUT(IPQ5424_NSS_300MHZ_CLK, "nss-300mhz", 300000000UL),
	CLK_PLL_OUTPUT(IPQ5424_PPE_375MHZ_CLK, "ppe-375mhz", 375000000UL),
	CLK_PLL_OUTPUT(IPQ5424_ETH0_50MHZ_CLK, "eth0-50mhz", 50000000UL),
	CLK_PLL_OUTPUT(IPQ5424_ETH1_50MHZ_CLK, "eth1-50mhz", 50000000UL),
	CLK_PLL_OUTPUT(IPQ5424_ETH2_50MHZ_CLK, "eth2-50mhz", 50000000UL),
	CLK_PLL_OUTPUT(IPQ5424_ETH_25MHZ_CLK, "eth-25mhz", 25000000UL),
	{ /* Sentinel */ }
};

static const struct cmn_pll_fixed_output_clk ipq6018_output_clks[] = {
	CLK_PLL_OUTPUT(IPQ6018_BIAS_PLL_CC_CLK, "bias_pll_cc_clk", 300000000UL),
	CLK_PLL_OUTPUT(IPQ6018_BIAS_PLL_NSS_NOC_CLK, "bias_pll_nss_noc_clk", 416500000UL),
};

static const struct cmn_pll_fixed_output_clk ipq8074_output_clks[] = {
	CLK_PLL_OUTPUT(IPQ8074_BIAS_PLL_CC_CLK, "bias_pll_cc_clk", 300000000UL),
	CLK_PLL_OUTPUT(IPQ8074_BIAS_PLL_NSS_NOC_CLK, "bias_pll_nss_noc_clk", 416500000UL),
	{ /* Sentinel */ }
};

static const struct cmn_pll_fixed_output_clk ipq9574_output_clks[] = {
	CLK_PLL_OUTPUT(XO_24MHZ_CLK, "xo-24mhz", 24000000UL),
	CLK_PLL_OUTPUT(SLEEP_32KHZ_CLK, "sleep-32khz", 32000UL),
	CLK_PLL_OUTPUT(PCS_31P25MHZ_CLK, "pcs-31p25mhz", 31250000UL),
	CLK_PLL_OUTPUT(NSS_1200MHZ_CLK, "nss-1200mhz", 1200000000UL),
	CLK_PLL_OUTPUT(PPE_353MHZ_CLK, "ppe-353mhz", 353000000UL),
	CLK_PLL_OUTPUT(ETH0_50MHZ_CLK, "eth0-50mhz", 50000000UL),
	CLK_PLL_OUTPUT(ETH1_50MHZ_CLK, "eth1-50mhz", 50000000UL),
	CLK_PLL_OUTPUT(ETH2_50MHZ_CLK, "eth2-50mhz", 50000000UL),
	CLK_PLL_OUTPUT(ETH_25MHZ_CLK, "eth-25mhz", 25000000UL),
	{ /* Sentinel */ }
};

static const struct cmn_pll_fixed_output_clk ipq5210_output_clks[] = {
	CLK_PLL_OUTPUT(IPQ5210_XO_24MHZ_CLK, "xo-24mhz", 24000000UL),
	CLK_PLL_OUTPUT(IPQ5210_SLEEP_32KHZ_CLK, "sleep-32khz", 32000UL),
	CLK_PLL_GATE(IPQ5210_PCS_31P25MHZ_CLK, "pcs-31p25mhz", 31250000UL, CLK31P25M_EN_BIT),
	CLK_PLL_GATE(IPQ5210_ETH0_50MHZ_CLK, "eth0-50mhz", 50000000UL, CLK50M_EN_BIT),
	CLK_PLL_GATE(IPQ5210_ETH1_50MHZ_CLK, "eth1-50mhz", 50000000UL, CLK50M_EN_BIT2_BIT),
	CLK_PLL_GATE(IPQ5210_ETH2_50MHZ_CLK, "eth2-50mhz", 50000000UL, CLK50M_EN_BIT3_BIT),
	CLK_PLL_GATE(IPQ5210_EPHY_50MHZ_CLK, "ephy-50mhz", 50000000UL, CLK250M_EN_BIT),
	CLK_PLL_GATE(IPQ5210_ETH_25MHZ_CLK, "eth-25mhz", 25000000UL, CLK25M_EN_BIT),
	CLK_PLL_OUTPUT(IPQ5210_NSS_CLK, "nss", 0),
	CLK_PLL_OUTPUT(IPQ5210_PPE_CLK, "ppe", 0),
	CLK_PLL_OUTPUT(IPQ5210_PON_REFCLK, "pon", 0),
	CLK_PLL_OUTPUT(IPQ5210_EPHY_RAW_CLK, "ephy-raw", 0),
	{ /* Sentinel */ }
};

static const struct cmn_pll_fixed_output_clk ipq9650_output_clks[] = {
	CLK_PLL_OUTPUT(IPQ9650_XO_24MHZ_CLK, "xo-24mhz", 24000000UL),
	CLK_PLL_OUTPUT(IPQ9650_SLEEP_32KHZ_CLK, "sleep-32khz", 32000UL),
	CLK_PLL_OUTPUT(IPQ9650_NSS_CLK, "nss", 0),
	CLK_PLL_OUTPUT(IPQ9650_PPE_CLK, "ppe", 0),
	CLK_PLL_OUTPUT(IPQ9650_PCS0_CLK, "pcs0", 0),
	CLK_PLL_OUTPUT(IPQ9650_PCS1_CLK, "pcs1", 0),
	CLK_PLL_OUTPUT(IPQ9650_PCS2_CLK, "pcs2", 0),
	CLK_PLL_OUTPUT(IPQ9650_ETH_PON_CLK, "eth-pon", 0),
	CLK_PLL_GATE(IPQ9650_ETH0_50MHZ_CLK, "eth0-50mhz", 50000000, CLK50M_EN_BIT),
	CLK_PLL_GATE(IPQ9650_ETH1_50MHZ_CLK, "eth1-50mhz", 50000000, CLK50M_EN_BIT2_BIT),
	CLK_PLL_GATE(IPQ9650_ETH2_50MHZ_CLK, "eth2-50mhz", 50000000, CLK50M_EN_BIT3_BIT),
	CLK_PLL_GATE(IPQ9650_ETH_25MHZ_CLK, "eth-25mhz", 25000000, CLK25M_EN_BIT),
	{ /* Sentinel */ }
};

/*
 * CMN PLL has the single parent clock, which supports the several
 * possible parent clock rates, each parent clock rate is reflected
 * by the specific reference index value in the hardware.
 */
static int ipq_cmn_pll_find_freq_index(unsigned long parent_rate)
{
	int index = -EINVAL;

	switch (parent_rate) {
	case 25000000:
		index = 3;
		break;
	case 31250000:
		index = 4;
		break;
	case 40000000:
		index = 6;
		break;
	case 48000000:
	case 96000000:
		/*
		 * Parent clock rate 48 MHZ and 96 MHZ take the same value
		 * of reference clock index. 96 MHZ needs the source clock
		 * divider to be programmed as 2.
		 */
		index = 7;
		break;
	case 50000000:
		index = 8;
		break;
	default:
		break;
	}

	return index;
}

/**
 * clk_cmn_pll_calc_rate - Compute the 64-bit CMN PLL output rate
 * @regmap: CMN PLL regmap
 * @ref_rate: Reference clock rate (parent of CMN PLL, fits in 32 bits)
 *
 * The CMN PLL rate = ref_rate * 2 * factor / ref_div.
 * On 32-bit platforms this result (~12 GHz) exceeds unsigned long, so we
 * always compute it as u64 to avoid truncation in callers.
 *
 * Returns the actual CMN PLL rate as u64.
 */
static u64 clk_cmn_pll_calc_rate(struct regmap *regmap, unsigned long ref_rate)
{
	u32 val, factor, ref_div;

	/*
	 * The value of CMN_PLL_DIVIDER_CTRL_FACTOR is automatically adjusted
	 * by HW according to the parent clock rate.
	 */
	regmap_read(regmap, CMN_PLL_DIVIDER_CTRL, &val);
	factor = FIELD_GET(CMN_PLL_DIVIDER_CTRL_FACTOR, val);
	if (WARN_ON(factor == 0))
		factor = 1;

	regmap_read(regmap, CMN_PLL_REFCLK_CONFIG, &val);
	ref_div = FIELD_GET(CMN_PLL_REFCLK_DIV, val);
	if (WARN_ON(ref_div == 0))
		ref_div = 1;

	return div_u64((u64)ref_rate * 2 * factor, ref_div);
}

/**
 * clk_cmn_pll_get_parent_rate64 - Get the 64-bit CMN PLL rate from a child clock
 * @hw: Child clock hw (NSS, PPE, or PON clock whose parent is the CMN PLL)
 *
 * On 32-bit platforms, the CMN PLL rate (~12 GHz) overflows unsigned long.
 * When the clock framework passes parent_rate to a child's recalc_rate /
 * set_rate / round_rate callback, the value is already truncated to 32 bits,
 * giving a wrong result.
 *
 * This helper traverses up to the grandparent (reference clock, ~48 MHz,
 * which fits in 32 bits) and recomputes the actual 64-bit CMN PLL rate
 * directly from the hardware registers.
 *
 * Returns the actual CMN PLL rate as u64, or 0 on error.
 */
static u64 clk_cmn_pll_get_parent_rate64(struct clk_hw *hw)
{
	struct clk_cmn_pll *clk = to_clk_cmn_pll(hw);
	struct clk_hw *cmn_pll_hw;
	struct clk_hw *ref_hw;
	unsigned long ref_rate;

	/* hw is NSS/PPE/PON clock; its parent is CMN PLL */
	cmn_pll_hw = clk_hw_get_parent(hw);
	if (!cmn_pll_hw)
		return 0;

	/* CMN PLL's parent is the reference clock (~48 MHz, fits in 32 bits) */
	ref_hw = clk_hw_get_parent(cmn_pll_hw);
	if (!ref_hw)
		return 0;

	ref_rate = clk_hw_get_rate(ref_hw);
	if (!ref_rate)
		return 0;

	return clk_cmn_pll_calc_rate(clk->regmap, ref_rate);
}

static unsigned long clk_cmn_pll_recalc_rate(struct clk_hw *hw,
					     unsigned long parent_rate)
{
	struct clk_cmn_pll *cmn_pll = to_clk_cmn_pll(hw);

	/*
	 * NOTE: On 32-bit platforms, the CMN PLL rate (~12 GHz) exceeds
	 * unsigned long (max ~4.29 GHz) and will be truncated here.
	 * Child clocks (NSS/PPE/PON) use clk_cmn_pll_get_parent_rate64()
	 * to obtain the correct 64-bit rate and avoid this truncation.
	 */
	return (unsigned long)clk_cmn_pll_calc_rate(cmn_pll->regmap, parent_rate);
}

static int clk_cmn_pll_determine_rate(struct clk_hw *hw,
				      struct clk_rate_request *req)
{
	int ret;

	/* Validate the rate of the single parent clock. */
	ret = ipq_cmn_pll_find_freq_index(req->best_parent_rate);

	return ret < 0 ? ret : 0;
}

static int clk_cmn_pll_ana_soft_reset(struct regmap *regmap)
{
	int ret;
	u32 val;

	ret = regmap_clear_bits(regmap, CMN_PLL_POWER_ON_AND_RESET,
				CMN_ANA_EN_SW_RSTN);
	if (ret)
		return ret;

	usleep_range(1000, 1200);
	ret = regmap_set_bits(regmap, CMN_PLL_POWER_ON_AND_RESET,
			      CMN_ANA_EN_SW_RSTN);
	if (ret)
		return ret;

	/* Stability check of CMN PLL output clocks. */
	return regmap_read_poll_timeout(regmap, CMN_PLL_LOCKED, val,
					(val & CMN_PLL_CLKS_LOCKED),
					100, 100 * USEC_PER_MSEC);
}

/*
 * This function is used to initialize the CMN PLL to enable the fixed
 * rate output clocks. It is expected to be configured once.
 */
static int clk_cmn_pll_set_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long parent_rate)
{
	struct clk_cmn_pll *cmn_pll = to_clk_cmn_pll(hw);
	int ret, index;

	/*
	 * Configure the reference input clock selection as per the given
	 * parent clock. The output clock rates are always of fixed value.
	 */
	index = ipq_cmn_pll_find_freq_index(parent_rate);
	if (index < 0)
		return index;

	ret = regmap_update_bits(cmn_pll->regmap, CMN_PLL_REFCLK_CONFIG,
				 CMN_PLL_REFCLK_INDEX,
				 FIELD_PREP(CMN_PLL_REFCLK_INDEX, index));
	if (ret)
		return ret;

	/*
	 * Update the source clock rate selection and source clock
	 * divider as 2 when the parent clock rate is 96 MHZ.
	 */
	if (parent_rate == 96000000) {
		ret = regmap_update_bits(cmn_pll->regmap, CMN_PLL_REFCLK_CONFIG,
					 CMN_PLL_REFCLK_DIV,
					 FIELD_PREP(CMN_PLL_REFCLK_DIV, 2));
		if (ret)
			return ret;

		ret = regmap_update_bits(cmn_pll->regmap, CMN_PLL_REFCLK_SRC_SELECTION,
					 CMN_PLL_REFCLK_SRC_DIV,
					 FIELD_PREP(CMN_PLL_REFCLK_SRC_DIV, 0));
		if (ret)
			return ret;
	}

	/* Enable PLL locked detect. */
	ret = regmap_set_bits(cmn_pll->regmap, CMN_PLL_CTRL,
			      CMN_PLL_CTRL_LOCK_DETECT_EN);
	if (ret)
		return ret;

	/*
	 * Reset the CMN PLL block to ensure the updated configurations
	 * take effect.
	 */
	return clk_cmn_pll_ana_soft_reset(cmn_pll->regmap);
}

static const struct clk_ops clk_cmn_pll_ops = {
	.recalc_rate = clk_cmn_pll_recalc_rate,
	.determine_rate = clk_cmn_pll_determine_rate,
	.set_rate = clk_cmn_pll_set_rate,
};

/*
 * PON (Passive Optical Network) reference clock operations.
 * The PON refclk is derived from CMN PLL rate / 2, then divided by
 * a configurable 8-bit divider (1-255).
 */
static int clk_pon_refclk_enable(struct clk_hw *hw)
{
	struct clk_cmn_pll *pon_clk = to_clk_cmn_pll(hw);

	return regmap_set_bits(pon_clk->regmap, CMN_PLL_PON_CONFIG,
			       CMN_PLL_PON_EN);
}

static void clk_pon_refclk_disable(struct clk_hw *hw)
{
	struct clk_cmn_pll *pon_clk = to_clk_cmn_pll(hw);

	regmap_clear_bits(pon_clk->regmap, CMN_PLL_PON_CONFIG,
			  CMN_PLL_PON_EN);
}

static int clk_pon_refclk_is_enabled(struct clk_hw *hw)
{
	struct clk_cmn_pll *pon_clk = to_clk_cmn_pll(hw);

	return regmap_test_bits(pon_clk->regmap, CMN_PLL_PON_CONFIG,
				CMN_PLL_PON_EN);
}

static unsigned long clk_pon_refclk_recalc_rate(struct clk_hw *hw,
						unsigned long parent_rate)
{
	struct clk_cmn_pll *pon_clk = to_clk_cmn_pll(hw);
	u64 pll_rate;
	u32 val, div;

	regmap_read(pon_clk->regmap, CMN_PLL_PON_CONFIG, &val);

	/* Check if in UNIPHY mode (bit 9 = 0) - fixed 31.25 MHz */
	if (!(val & CMN_PLL_PON_MODE_SEL))
		return 31250000UL;

	/* PON mode: calculate from divider */
	div = FIELD_GET(CMN_PLL_PON_DIV_CTRL, val);
	if (WARN_ON_ONCE(!div))
		return 0;

	/*
	 * Use the actual 64-bit CMN PLL rate to avoid 32-bit overflow
	 * on 32-bit platforms where parent_rate (~12 GHz) is truncated.
	 */
	pll_rate = clk_cmn_pll_get_parent_rate64(hw);
	if (!pll_rate) {
		WARN_ONCE(1, "Failed to get 64-bit CMN PLL rate\n");
		return 0;
	}

	return DIV_ROUND_CLOSEST_ULL(pll_rate, 2ULL * div);
}

static long clk_pon_refclk_round_rate(struct clk_hw *hw, unsigned long rate,
				      unsigned long *parent_rate)
{
	u64 pll_rate;
	unsigned long div;

	if (!rate)
		return 0;

	/*
	 * Use the actual 64-bit CMN PLL rate to avoid 32-bit overflow
	 * on 32-bit platforms where *parent_rate (~12 GHz) is truncated.
	 */
	pll_rate = clk_cmn_pll_get_parent_rate64(hw);
	if (!pll_rate) {
		WARN_ONCE(1, "Failed to get 64-bit CMN PLL rate\n");
		return 0;
	}

	div = DIV_ROUND_CLOSEST_ULL(pll_rate, 2ULL * rate);

	/* Clamp to valid range (1-255) */
	div = clamp_t(unsigned long, div, 1, 255);

	return DIV_ROUND_CLOSEST_ULL(pll_rate, 2ULL * div);
}

static int clk_pon_refclk_set_rate(struct clk_hw *hw, unsigned long rate,
				   unsigned long parent_rate)
{
	struct clk_cmn_pll *pon_clk = to_clk_cmn_pll(hw);
	u64 pll_rate;
	unsigned long div;
	int ret;

	/* Check if requesting UNIPHY fixed rate (31.25 MHz) */
	if (rate == 31250000UL) {
		return regmap_clear_bits(pon_clk->regmap,
					 CMN_PLL_PON_CONFIG,
					 CMN_PLL_PON_MODE_SEL);
	}

	/*
	 * Use the actual 64-bit CMN PLL rate to avoid 32-bit overflow
	 * on 32-bit platforms where parent_rate (~12 GHz) is truncated.
	 */
	pll_rate = clk_cmn_pll_get_parent_rate64(hw);
	if (!pll_rate) {
		WARN_ONCE(1, "Failed to get 64-bit CMN PLL rate\n");
		return -EINVAL;
	}

	div = DIV_ROUND_CLOSEST_ULL(pll_rate, 2ULL * rate);

	/* Constrain divider to 8-bit register width: [1, 255]. */
	if (div == 0 || div > 255)
		return -EINVAL;

	/* Switch to PON mode (bit 9 = 1) */
	ret = regmap_set_bits(pon_clk->regmap, CMN_PLL_PON_CONFIG,
			      CMN_PLL_PON_MODE_SEL);
	if (ret)
		return ret;

	/* Update divider field */
	ret = regmap_update_bits(pon_clk->regmap, CMN_PLL_PON_CONFIG,
				 CMN_PLL_PON_DIV_CTRL,
				 FIELD_PREP(CMN_PLL_PON_DIV_CTRL, div));
	if (ret)
		return ret;

	return clk_cmn_pll_ana_soft_reset(pon_clk->regmap);
}

static const struct clk_ops clk_pon_refclk_ops = {
	.enable = clk_pon_refclk_enable,
	.disable = clk_pon_refclk_disable,
	.is_enabled = clk_pon_refclk_is_enabled,
	.recalc_rate = clk_pon_refclk_recalc_rate,
	.round_rate = clk_pon_refclk_round_rate,
	.set_rate = clk_pon_refclk_set_rate,
};

/*
 * NSS (Network Subsystem) clock operations.
 * The NSS clock is derived from CMN PLL rate / 2, then divided by
 * a configurable 6-bit divider (8-63).
 */
static unsigned long clk_nss_recalc_rate(struct clk_hw *hw,
					 unsigned long parent_rate)
{
	struct clk_cmn_pll *nss_clk = to_clk_cmn_pll(hw);
	u64 pll_rate;
	u32 val, div;

	regmap_read(nss_clk->regmap, CMN_PLL_NSS_PPE_FREQ_CTRL, &val);
	div = FIELD_GET(CMN_PLL_NSS_CLK_SEL, val);
	if (WARN_ON_ONCE(!div))
		return 0;

	/*
	 * Use the actual 64-bit CMN PLL rate to avoid 32-bit overflow
	 * on 32-bit platforms where parent_rate (~12 GHz) is truncated.
	 */
	pll_rate = clk_cmn_pll_get_parent_rate64(hw);
	if (!pll_rate) {
		WARN_ONCE(1, "Failed to get 64-bit CMN PLL rate\n");
		return 0;
	}

	return DIV_ROUND_CLOSEST_ULL(pll_rate, 2ULL * div);
}

static long clk_nss_ppe_round_rate(struct clk_hw *hw, unsigned long rate,
				   unsigned long *parent_rate)
{
	u64 pll_rate;
	unsigned long div;

	/*
	 * Use the actual 64-bit CMN PLL rate to avoid 32-bit overflow
	 * on 32-bit platforms where *parent_rate (~12 GHz) is truncated.
	 */
	pll_rate = clk_cmn_pll_get_parent_rate64(hw);
	if (!pll_rate) {
		WARN_ONCE(1, "Failed to get 64-bit CMN PLL rate\n");
		return 0;
	}

	/* Calculate divider */
	div = DIV_ROUND_CLOSEST_ULL(pll_rate, 2ULL * rate);

	/* Clamp to valid range (8-63) */
	div = clamp_t(unsigned long, div, CMN_PLL_NSS_PPE_DIV_MIN,
		      CMN_PLL_NSS_PPE_DIV_MAX);

	return DIV_ROUND_CLOSEST_ULL(pll_rate, 2ULL * div);
}

static int clk_nss_set_rate(struct clk_hw *hw, unsigned long rate,
			    unsigned long parent_rate)
{
	struct clk_cmn_pll *nss_clk = to_clk_cmn_pll(hw);
	u64 pll_rate;
	unsigned long div;
	int ret;

	/*
	 * Use the actual 64-bit CMN PLL rate to avoid 32-bit overflow
	 * on 32-bit platforms where parent_rate (~12 GHz) is truncated.
	 */
	pll_rate = clk_cmn_pll_get_parent_rate64(hw);
	if (!pll_rate) {
		WARN_ONCE(1, "Failed to get 64-bit CMN PLL rate\n");
		return -EINVAL;
	}

	/* Calculate divider */
	div = DIV_ROUND_CLOSEST_ULL(pll_rate, 2ULL * rate);

	/* Validate range */
	if (div < CMN_PLL_NSS_PPE_DIV_MIN || div > CMN_PLL_NSS_PPE_DIV_MAX)
		return -EINVAL;

	/* Update divider field */
	ret = regmap_update_bits(nss_clk->regmap, CMN_PLL_NSS_PPE_FREQ_CTRL,
				 CMN_PLL_NSS_CLK_SEL,
				 FIELD_PREP(CMN_PLL_NSS_CLK_SEL, div));
	if (ret)
		return ret;

	return clk_cmn_pll_ana_soft_reset(nss_clk->regmap);
}

static const struct clk_ops clk_nss_ops = {
	.recalc_rate = clk_nss_recalc_rate,
	.round_rate = clk_nss_ppe_round_rate,
	.set_rate = clk_nss_set_rate,
};

/*
 * PPE (Packet Process Engine) clock operations.
 * The PPE clock is derived from CMN PLL rate / 2, then divided by
 * a configurable 6-bit divider (8-63).
 */
static unsigned long clk_ppe_recalc_rate(struct clk_hw *hw,
					 unsigned long parent_rate)
{
	struct clk_cmn_pll *ppe_clk = to_clk_cmn_pll(hw);
	u64 pll_rate;
	u32 val, div;

	regmap_read(ppe_clk->regmap, CMN_PLL_NSS_PPE_FREQ_CTRL, &val);
	div = FIELD_GET(CMN_PLL_PPE_CLK_SEL, val);
	if (WARN_ON_ONCE(!div))
		return 0;

	/*
	 * Use the actual 64-bit CMN PLL rate to avoid 32-bit overflow
	 * on 32-bit platforms where parent_rate (~12 GHz) is truncated.
	 */
	pll_rate = clk_cmn_pll_get_parent_rate64(hw);
	if (!pll_rate) {
		WARN_ONCE(1, "Failed to get 64-bit CMN PLL rate\n");
		return 0;
	}

	return DIV_ROUND_CLOSEST_ULL(pll_rate, 2ULL * div);
}

static int clk_ppe_set_rate(struct clk_hw *hw, unsigned long rate,
			    unsigned long parent_rate)
{
	struct clk_cmn_pll *ppe_clk = to_clk_cmn_pll(hw);
	u64 pll_rate;
	unsigned long div;
	int ret;

	/*
	 * Use the actual 64-bit CMN PLL rate to avoid 32-bit overflow
	 * on 32-bit platforms where parent_rate (~12 GHz) is truncated.
	 */
	pll_rate = clk_cmn_pll_get_parent_rate64(hw);
	if (!pll_rate) {
		WARN_ONCE(1, "Failed to get 64-bit CMN PLL rate\n");
		return -EINVAL;
	}

	/* Calculate divider */
	div = DIV_ROUND_CLOSEST_ULL(pll_rate, 2ULL * rate);

	/* Validate range */
	if (div < CMN_PLL_NSS_PPE_DIV_MIN || div > CMN_PLL_NSS_PPE_DIV_MAX)
		return -EINVAL;

	/* Update divider field */
	ret = regmap_update_bits(ppe_clk->regmap, CMN_PLL_NSS_PPE_FREQ_CTRL,
				 CMN_PLL_PPE_CLK_SEL,
				 FIELD_PREP(CMN_PLL_PPE_CLK_SEL, div));
	if (ret)
		return ret;

	return clk_cmn_pll_ana_soft_reset(ppe_clk->regmap);
}

static const struct clk_ops clk_ppe_ops = {
	.recalc_rate = clk_ppe_recalc_rate,
	.round_rate = clk_nss_ppe_round_rate,
	.set_rate = clk_ppe_set_rate,
};

/*
 * ETH/PON clock operations.
 * The output clock rate is determined by the bit 4 of CMN_PLL_OUTPUT_RELATED_2.
 * 0: 25 MHz
 * 1: 31.25 MHz
 */
static int clk_eth_pon_enable(struct clk_hw *hw)
{
	struct clk_cmn_pll *cmn_pll = to_clk_cmn_pll(hw);
	unsigned long rate = clk_hw_get_rate(hw);
	u32 enable_bit;

	if (rate == 25000000) {
		enable_bit = CLK25M_EN_BIT;
		regmap_clear_bits(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_2,
				  CMN_PLL_OUTPUT_MUX_SEL);
	} else {
		enable_bit = CLK31P25M_EN_BIT;
		regmap_set_bits(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_2,
				CMN_PLL_OUTPUT_MUX_SEL);
	}

	return regmap_set_bits(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_1,
			       BIT(enable_bit));
}

static void clk_eth_pon_disable(struct clk_hw *hw)
{
	struct clk_cmn_pll *cmn_pll = to_clk_cmn_pll(hw);

	regmap_clear_bits(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_1,
			  BIT(CLK25M_EN_BIT) | BIT(CLK31P25M_EN_BIT));
}

static int clk_eth_pon_is_enabled(struct clk_hw *hw)
{
	struct clk_cmn_pll *cmn_pll = to_clk_cmn_pll(hw);
	u32 val;

	regmap_read(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_2, &val);
	if (val & CMN_PLL_OUTPUT_MUX_SEL)
		return regmap_test_bits(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_1,
					BIT(CLK31P25M_EN_BIT));

	return regmap_test_bits(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_1,
				BIT(CLK25M_EN_BIT));
}

static unsigned long clk_eth_pon_recalc_rate(struct clk_hw *hw,
					     unsigned long parent_rate)
{
	struct clk_cmn_pll *cmn_pll = to_clk_cmn_pll(hw);
	u32 val;

	regmap_read(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_2, &val);

	if (val & CMN_PLL_OUTPUT_MUX_SEL)
		return 31250000UL;

	return 25000000UL;
}

static int clk_eth_pon_determine_rate(struct clk_hw *hw,
				       struct clk_rate_request *req)
{
	if (req->rate <= 25000000UL)
		req->rate = 25000000UL;
	else
		req->rate = 31250000UL;

	return 0;
}

static int clk_eth_pon_set_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long parent_rate)
{
	struct clk_cmn_pll *cmn_pll = to_clk_cmn_pll(hw);

	/* Disable clock output if enabled. */
	if (clk_eth_pon_is_enabled(hw)) {
		regmap_clear_bits(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_1,
				  BIT(CLK31P25M_EN_BIT));
		regmap_clear_bits(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_1,
				  BIT(CLK25M_EN_BIT));
	}

	/* Set the clock rate. */
	if (rate == 25000000)
		regmap_clear_bits(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_2,
				  CMN_PLL_OUTPUT_MUX_SEL);
	else
		regmap_set_bits(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_2,
				CMN_PLL_OUTPUT_MUX_SEL);

	/* Enable clock output if enabled. */
	if (clk_eth_pon_is_enabled(hw)) {
		if (rate == 25000000)
			regmap_set_bits(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_1,
					BIT(CLK25M_EN_BIT));
		else
			regmap_set_bits(cmn_pll->regmap, CMN_PLL_OUTPUT_RELATED_1,
					BIT(CLK31P25M_EN_BIT));
	}

	return clk_cmn_pll_ana_soft_reset(cmn_pll->regmap);
}

static const struct clk_ops clk_eth_pon_ops = {
	.enable = clk_eth_pon_enable,
	.disable = clk_eth_pon_disable,
	.is_enabled = clk_eth_pon_is_enabled,
	.recalc_rate = clk_eth_pon_recalc_rate,
	.determine_rate = clk_eth_pon_determine_rate,
	.set_rate = clk_eth_pon_set_rate,
};

/*
 * EPHY raw clock operations for IPQ5210.
 * The output clock rate is determined by bit 10 of CMN_PLL_PON_CONFIG.
 * 0: 125 MHz (for link speeds other than 2.5G)
 * 1: 312.5 MHz (for 2.5G link speed)
 */
static unsigned long clk_ephy_raw_recalc_rate(struct clk_hw *hw,
					      unsigned long parent_rate)
{
	struct clk_cmn_pll *ephy_raw_clk = to_clk_cmn_pll(hw);
	u32 val;

	regmap_read(ephy_raw_clk->regmap, CMN_PLL_PON_CONFIG, &val);

	if (val & CMN_PLL_GEPHY_312P5M_125M_SEL)
		return 312500000UL;

	return 125000000UL;
}

static int clk_ephy_raw_determine_rate(struct clk_hw *hw,
					struct clk_rate_request *req)
{
	if (req->rate <= 125000000UL)
		req->rate = 125000000UL;
	else
		req->rate = 312500000UL;

	return 0;
}

static int clk_ephy_raw_set_rate(struct clk_hw *hw, unsigned long rate,
				 unsigned long parent_rate)
{
	struct clk_cmn_pll *ephy_raw_clk = to_clk_cmn_pll(hw);
	int ret;

	/* Set the clock rate based on bit 10 */
	if (rate == 125000000UL)
		ret = regmap_clear_bits(ephy_raw_clk->regmap,
					CMN_PLL_PON_CONFIG,
					CMN_PLL_GEPHY_312P5M_125M_SEL);
	else
		ret = regmap_set_bits(ephy_raw_clk->regmap,
				      CMN_PLL_PON_CONFIG,
				      CMN_PLL_GEPHY_312P5M_125M_SEL);

	return ret;
}

static const struct clk_ops clk_ephy_raw_ops = {
	.recalc_rate = clk_ephy_raw_recalc_rate,
	.determine_rate = clk_ephy_raw_determine_rate,
	.set_rate = clk_ephy_raw_set_rate,
};

/*
 * PCS clock operations for IPQ9650.
 * The PCS clocks are controlled via the UPHY_REFCLK_CTRL register (0x41C).
 * Each of the three PCS clocks (PCS0/1/2) can be independently enabled and
 * configured to output one of four frequencies: 46.875, 93.75, 31.25, or 62.5 MHz.
 *
 * Hardware divsel encoding:
 *   0b00 (0) -> 46.875 MHz
 *   0b01 (1) -> 93.75 MHz
 *   0b10 (2) -> 31.25 MHz
 *   0b11 (3) -> 62.5 MHz
 */

/**
 * struct pcs_freq_map - PCS frequency to divsel mapping
 * @freq: Output frequency in Hz
 * @divsel: Hardware divider select value (0-3)
 */
struct pcs_freq_map {
	unsigned long freq;
	u32 divsel;
};

/* Supported PCS frequencies sorted by frequency for efficient lookup */
static const struct pcs_freq_map pcs_freq_table[] = {
	{ 31250000UL, 2 },  /* 31.25 MHz */
	{ 46875000UL, 0 },  /* 46.875 MHz */
	{ 62500000UL, 3 },  /* 62.5 MHz */
	{ 93750000UL, 1 },  /* 93.75 MHz */
};

/**
 * pcs_parse_index - Extract PCS index from clock name
 * @name: Clock name (e.g., "pcs0", "pcs1", "pcs2")
 *
 * Return: PCS index (0-2) on success, -EINVAL on error
 */
static int pcs_parse_index(const char *name)
{
	size_t len;

	if (!name)
		return -EINVAL;

	len = strlen(name);

	/* Expected format: "pcs" followed by single digit 0-2 */
	if (len == 4 && !strncmp(name, "pcs", 3)) {
		int idx = name[3] - '0';
		if (idx >= 0 && idx <= 2)
			return idx;
	}

	return -EINVAL;
}

/**
 * pcs_get_enable_bit - Get PCS clock enable bit
 * @name: Clock name
 * @enable_bit: Output parameter for enable bit
 *
 * Return: 0 on success, -EINVAL on error
 */
static int pcs_get_enable_bit(const char *name, u32 *enable_bit)
{
	static const u32 enable_bits[] = {
		CMN_PLL_PCS0_CLK_EN,
		CMN_PLL_PCS1_CLK_EN,
		CMN_PLL_PCS2_CLK_EN,
	};
	int idx;

	idx = pcs_parse_index(name);
	if (idx < 0)
		return idx;

	*enable_bit = enable_bits[idx];
	return 0;
}

/**
 * pcs_get_divsel_mask - Get PCS clock divsel mask
 * @name: Clock name
 * @divsel_mask: Output parameter for divsel mask
 *
 * Return: 0 on success, -EINVAL on error
 */
static int pcs_get_divsel_mask(const char *name, u32 *divsel_mask)
{
	static const u32 divsel_masks[] = {
		CMN_PLL_PCS0_CLK_DIVSEL,
		CMN_PLL_PCS1_CLK_DIVSEL,
		CMN_PLL_PCS2_CLK_DIVSEL,
	};
	int idx;

	idx = pcs_parse_index(name);
	if (idx < 0)
		return idx;

	*divsel_mask = divsel_masks[idx];
	return 0;
}

/**
 * pcs_divsel_to_freq - Convert divsel value to frequency
 * @divsel: Hardware divider select value (0-3)
 *
 * Return: Frequency in Hz, or 0 if divsel is invalid
 */
static unsigned long pcs_divsel_to_freq(u32 divsel)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(pcs_freq_table); i++) {
		if (pcs_freq_table[i].divsel == divsel)
			return pcs_freq_table[i].freq;
	}

	return 0;
}

/**
 * pcs_freq_to_divsel - Convert frequency to divsel value
 * @rate: Requested frequency in Hz
 * @divsel: Output parameter for divsel value
 *
 * Return: 0 on success, -EINVAL if frequency is not supported
 */
static int pcs_freq_to_divsel(unsigned long rate, u32 *divsel)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(pcs_freq_table); i++) {
		if (pcs_freq_table[i].freq == rate) {
			*divsel = pcs_freq_table[i].divsel;
			return 0;
		}
	}

	return -EINVAL;
}

static int clk_pcs_clk_enable(struct clk_hw *hw)
{
	struct clk_cmn_pll *pcs_clk = to_clk_cmn_pll(hw);
	const char *name = clk_hw_get_name(hw);
	u32 enable_bit;
	int ret;

	ret = pcs_get_enable_bit(name, &enable_bit);
	if (ret)
		return ret;

	return regmap_set_bits(pcs_clk->regmap, CMN_PLL_PCS_CLK_CTRL, enable_bit);
}

static void clk_pcs_clk_disable(struct clk_hw *hw)
{
	struct clk_cmn_pll *pcs_clk = to_clk_cmn_pll(hw);
	const char *name = clk_hw_get_name(hw);
	u32 enable_bit;

	if (pcs_get_enable_bit(name, &enable_bit))
		return;

	regmap_clear_bits(pcs_clk->regmap, CMN_PLL_PCS_CLK_CTRL, enable_bit);
}

static int clk_pcs_clk_is_enabled(struct clk_hw *hw)
{
	struct clk_cmn_pll *pcs_clk = to_clk_cmn_pll(hw);
	const char *name = clk_hw_get_name(hw);
	u32 enable_bit;
	int ret;

	ret = pcs_get_enable_bit(name, &enable_bit);
	if (ret)
		return 0;

	return regmap_test_bits(pcs_clk->regmap, CMN_PLL_PCS_CLK_CTRL,
				enable_bit);
}

static unsigned long clk_pcs_clk_recalc_rate(struct clk_hw *hw,
					     unsigned long parent_rate)
{
	struct clk_cmn_pll *pcs_clk = to_clk_cmn_pll(hw);
	const char *name = clk_hw_get_name(hw);
	u32 val, divsel, divsel_mask;
	unsigned long freq;
	int ret;

	ret = pcs_get_divsel_mask(name, &divsel_mask);
	if (ret)
		return 0;

	regmap_read(pcs_clk->regmap, CMN_PLL_PCS_CLK_CTRL, &val);
	divsel = (val & divsel_mask) >> __ffs(divsel_mask);

	/* Convert divsel to frequency using lookup table */
	freq = pcs_divsel_to_freq(divsel);
	if (WARN_ON_ONCE(!freq))
		return 0;

	return freq;
}

static long clk_pcs_clk_round_rate(struct clk_hw *hw, unsigned long rate,
				   unsigned long *parent_rate)
{
	int i;
	unsigned long best_freq = pcs_freq_table[0].freq;
	unsigned long min_diff = ULONG_MAX;

	/* Find the closest supported frequency */
	for (i = 0; i < ARRAY_SIZE(pcs_freq_table); i++) {
		unsigned long diff = abs((long)(rate - pcs_freq_table[i].freq));

		if (diff < min_diff) {
			min_diff = diff;
			best_freq = pcs_freq_table[i].freq;
		}
	}

	return best_freq;
}

static int clk_pcs_clk_set_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long parent_rate)
{
	struct clk_cmn_pll *pcs_clk = to_clk_cmn_pll(hw);
	const char *name = clk_hw_get_name(hw);
	u32 divsel, divsel_mask;
	int ret;

	/* Convert frequency to divsel value using lookup table */
	ret = pcs_freq_to_divsel(rate, &divsel);
	if (ret)
		return ret;

	ret = pcs_get_divsel_mask(name, &divsel_mask);
	if (ret)
		return ret;

	ret = regmap_update_bits(pcs_clk->regmap, CMN_PLL_PCS_CLK_CTRL,
				 divsel_mask, (divsel << __ffs(divsel_mask)));
	if (ret)
		return ret;

	return clk_cmn_pll_ana_soft_reset(pcs_clk->regmap);
}

static const struct clk_ops clk_pcs_clk_ops = {
	.enable = clk_pcs_clk_enable,
	.disable = clk_pcs_clk_disable,
	.is_enabled = clk_pcs_clk_is_enabled,
	.recalc_rate = clk_pcs_clk_recalc_rate,
	.round_rate = clk_pcs_clk_round_rate,
	.set_rate = clk_pcs_clk_set_rate,
};

static struct clk_hw *ipq_cmn_pll_clk_hw_register(struct platform_device *pdev)
{
	struct clk_parent_data pdata = { .index = 0 };
	struct device *dev = &pdev->dev;
	struct clk_init_data init = {};
	struct clk_cmn_pll *cmn_pll;
	struct regmap *regmap;
	void __iomem *base;
	int ret;

	base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(base))
		return ERR_CAST(base);

	regmap = devm_regmap_init_mmio(dev, base, &ipq_cmn_pll_regmap_config);
	if (IS_ERR(regmap))
		return ERR_CAST(regmap);

	cmn_pll = devm_kzalloc(dev, sizeof(*cmn_pll), GFP_KERNEL);
	if (!cmn_pll)
		return ERR_PTR(-ENOMEM);

	init.name = "cmn_pll";
	init.parent_data = &pdata;
	init.num_parents = 1;
	init.ops = &clk_cmn_pll_ops;

	cmn_pll->hw.init = &init;
	cmn_pll->regmap = regmap;
	cmn_pll->base = base;

	ret = devm_clk_hw_register(dev, &cmn_pll->hw);
	if (ret)
		return ERR_PTR(ret);

	return &cmn_pll->hw;
}

/*
 * Register PON reference clock with CMN PLL as parent.
 * The PON refclk is derived from CMN PLL / 2, then divided by
 * a configurable divider (default 129 for 46.512 MHz).
 */
static struct clk_hw *ipq_cmn_pll_pon_refclk_register(struct platform_device *pdev,
						      struct regmap *regmap,
						      struct clk_hw *cmn_pll_hw)
{
	struct clk_parent_data pdata = { .hw = cmn_pll_hw };
	struct clk_cmn_pll *pon_clk;
	struct device *dev = &pdev->dev;
	struct clk_init_data init = {};
	int ret;

	pon_clk = devm_kzalloc(dev, sizeof(*pon_clk), GFP_KERNEL);
	if (!pon_clk)
		return ERR_PTR(-ENOMEM);

	init.name = "pon-clk";
	init.parent_data = &pdata;
	init.num_parents = 1;
	init.ops = &clk_pon_refclk_ops;
	init.flags = 0;

	pon_clk->hw.init = &init;
	pon_clk->regmap = regmap;

	ret = devm_clk_hw_register(dev, &pon_clk->hw);
	if (ret)
		return ERR_PTR(ret);

	return &pon_clk->hw;
}

/*
 * Register NSS clock with CMN PLL as parent.
 * The NSS clock is derived from CMN PLL / 2, then divided by
 * a configurable divider (default 14 for 428.57 MHz).
 */
static struct clk_hw *ipq_cmn_pll_nss_register(struct platform_device *pdev,
					       struct regmap *regmap,
					       struct clk_hw *cmn_pll_hw)
{
	struct clk_parent_data pdata = { .hw = cmn_pll_hw };
	struct clk_cmn_pll *nss_clk;
	struct device *dev = &pdev->dev;
	struct clk_init_data init = {};
	int ret;

	nss_clk = devm_kzalloc(dev, sizeof(*nss_clk), GFP_KERNEL);
	if (!nss_clk)
		return ERR_PTR(-ENOMEM);

	init.name = "nss-clk";
	init.parent_data = &pdata;
	init.num_parents = 1;
	init.ops = &clk_nss_ops;
	init.flags = 0;

	nss_clk->hw.init = &init;
	nss_clk->regmap = regmap;

	ret = devm_clk_hw_register(dev, &nss_clk->hw);
	if (ret)
		return ERR_PTR(ret);

	return &nss_clk->hw;
}

/*
 * Register PPE clock with CMN PLL as parent.
 * The PPE clock is derived from CMN PLL / 2, then divided by
 * a configurable divider (default 16 for 375 MHz).
 */
static struct clk_hw *ipq_cmn_pll_ppe_register(struct platform_device *pdev,
					       struct regmap *regmap,
					       struct clk_hw *cmn_pll_hw)
{
	struct clk_parent_data pdata = { .hw = cmn_pll_hw };
	struct clk_cmn_pll *ppe_clk;
	struct device *dev = &pdev->dev;
	struct clk_init_data init = {};
	int ret;

	ppe_clk = devm_kzalloc(dev, sizeof(*ppe_clk), GFP_KERNEL);
	if (!ppe_clk)
		return ERR_PTR(-ENOMEM);

	init.name = "ppe-clk";
	init.parent_data = &pdata;
	init.num_parents = 1;
	init.ops = &clk_ppe_ops;
	init.flags = 0;

	ppe_clk->hw.init = &init;
	ppe_clk->regmap = regmap;

	ret = devm_clk_hw_register(dev, &ppe_clk->hw);
	if (ret)
		return ERR_PTR(ret);

	return &ppe_clk->hw;
}

/*
 * Register PCS clock with CMN PLL as parent.
 * The PCS clocks are controlled via the UPHY_REFCLK_CTRL register (0x41C).
 */
static struct clk_hw *ipq_cmn_pll_pcs_register(struct platform_device *pdev,
					       struct regmap *regmap,
					       struct clk_hw *cmn_pll_hw,
					       const char *name)
{
	struct clk_parent_data pdata = { .hw = cmn_pll_hw };
	struct clk_cmn_pll *pcs_clk;
	struct device *dev = &pdev->dev;
	struct clk_init_data init = {};
	int ret;

	pcs_clk = devm_kzalloc(dev, sizeof(*pcs_clk), GFP_KERNEL);
	if (!pcs_clk)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.parent_data = &pdata;
	init.num_parents = 1;
	init.ops = &clk_pcs_clk_ops;
	init.flags = 0;

	pcs_clk->hw.init = &init;
	pcs_clk->regmap = regmap;

	ret = devm_clk_hw_register(dev, &pcs_clk->hw);
	if (ret)
		return ERR_PTR(ret);

	return &pcs_clk->hw;
}

/*
 * Register EPHY raw clock with CMN PLL as parent.
 * The EPHY raw clock can switch between 125 MHz and 312.5 MHz.
 */
static struct clk_hw *ipq_cmn_pll_ephy_raw_register(struct platform_device *pdev,
						    struct regmap *regmap,
						    struct clk_hw *cmn_pll_hw)
{
	struct clk_parent_data pdata = { .hw = cmn_pll_hw };
	struct clk_cmn_pll *ephy_raw_clk;
	struct device *dev = &pdev->dev;
	struct clk_init_data init = {};
	int ret;

	ephy_raw_clk = devm_kzalloc(dev, sizeof(*ephy_raw_clk), GFP_KERNEL);
	if (!ephy_raw_clk)
		return ERR_PTR(-ENOMEM);

	init.name = "ephy-raw";
	init.parent_data = &pdata;
	init.num_parents = 1;
	init.ops = &clk_ephy_raw_ops;
	init.flags = 0;

	ephy_raw_clk->hw.init = &init;
	ephy_raw_clk->regmap = regmap;

	ret = devm_clk_hw_register(dev, &ephy_raw_clk->hw);
	if (ret)
		return ERR_PTR(ret);

	return &ephy_raw_clk->hw;
}

/*
 * Register ETH/PON clock with CMN PLL as parent.
 * The ETH/PON clock can switch between 25 MHz and 31.25 MHz.
 */
static struct clk_hw *ipq_cmn_pll_eth_pon_register(struct platform_device *pdev,
						   struct regmap *regmap,
						   void __iomem *base,
						   struct clk_hw *cmn_pll_hw,
						   const struct cmn_pll_fixed_output_clk *fixed_clk)
{
	struct clk_parent_data pdata = { .hw = cmn_pll_hw };
	struct clk_cmn_pll *eth_pon_clk;
	struct device *dev = &pdev->dev;
	struct clk_init_data init = {};
	int ret;

	eth_pon_clk = devm_kzalloc(dev, sizeof(*eth_pon_clk), GFP_KERNEL);
	if (!eth_pon_clk)
		return ERR_PTR(-ENOMEM);

	init.name = fixed_clk->name;
	init.parent_data = &pdata;
	init.num_parents = 1;
	init.ops = &clk_eth_pon_ops;
	init.flags = 0;

	eth_pon_clk->hw.init = &init;
	eth_pon_clk->regmap = regmap;
	eth_pon_clk->base = base;

	ret = devm_clk_hw_register(dev, &eth_pon_clk->hw);
	if (ret)
		return ERR_PTR(ret);

	return &eth_pon_clk->hw;
}

/*
 * Register a composite clock that combines a fixed-rate clock with a gate.
 * This is used for clocks that need both a specific fixed rate and gate control.
 */
static struct clk_hw *ipq_cmn_pll_register_fixed_gate(struct device *dev,
						      const char *name,
						      struct clk_hw *parent_hw,
						      unsigned long rate,
						      void __iomem *base,
						      u8 enable_bit)
{
	struct clk_parent_data pdata = { .hw = parent_hw };
	struct clk_fixed_rate *fixed;
	struct clk_gate *gate;
	spinlock_t *lock;

	/* Allocate and initialize spinlock for gate protection */
	lock = devm_kzalloc(dev, sizeof(*lock), GFP_KERNEL);
	if (!lock)
		return ERR_PTR(-ENOMEM);

	spin_lock_init(lock);

	/* Allocate gate structure */
	gate = devm_kzalloc(dev, sizeof(*gate), GFP_KERNEL);
	if (!gate)
		return ERR_PTR(-ENOMEM);

	/* Allocate fixed-rate structure */
	fixed = devm_kzalloc(dev, sizeof(*fixed), GFP_KERNEL);
	if (!fixed)
		return ERR_PTR(-ENOMEM);

	/* Setup gate */
	gate->reg = base + CMN_PLL_OUTPUT_RELATED_1;
	gate->bit_idx = enable_bit;
	gate->lock = lock;

	/* Setup fixed-rate */
	fixed->fixed_rate = rate;

	/*
	 * Use devm_clk_hw_register_composite_pdata() so devres handles
	 * cleanup automatically, avoiding the need for an explicit
	 * devm_add_action_or_reset() and preventing double-unregister
	 * or memory leaks when the device is removed.
	 */
	return devm_clk_hw_register_composite_pdata(dev, name, &pdata, 1,
						    NULL, NULL,
						    &fixed->hw, &clk_fixed_rate_ops,
						    &gate->hw, &clk_gate_ops,
						    0);
}

static int ipq_cmn_pll_register_clks(struct platform_device *pdev)
{
	const struct cmn_pll_fixed_output_clk *p, *fixed_clk;
	struct clk_hw_onecell_data *hw_data;
	struct device *dev = &pdev->dev;
	struct clk_hw *cmn_pll_hw;
	struct clk_cmn_pll *cmn_pll;
	unsigned int num_clks;
	struct clk_hw *hw;
	int ret, i;

	fixed_clk = device_get_match_data(dev);
	if (!fixed_clk)
		return -EINVAL;

	/* Count fixed clocks from the array */
	num_clks = 0;
	for (p = fixed_clk; p->name; p++)
		num_clks++;

	/*
	 * Register the CMN PLL clock, which is the parent clock of
	 * the fixed rate output clocks.
	 */
	cmn_pll_hw = ipq_cmn_pll_clk_hw_register(pdev);
	if (IS_ERR(cmn_pll_hw))
		return PTR_ERR(cmn_pll_hw);

	cmn_pll = to_clk_cmn_pll(cmn_pll_hw);

	/* Allocate hw_data for all clocks */
	hw_data = devm_kzalloc(dev, struct_size(hw_data, hws, num_clks + 1),
			       GFP_KERNEL);
	if (!hw_data)
		return -ENOMEM;

	/* Register the fixed rate output clocks (common for all platforms) */
	for (i = 0; fixed_clk[i].name; i++) {
		if (fixed_clk[i].enable_bit != -1) {
			hw = ipq_cmn_pll_register_fixed_gate(dev,
							     fixed_clk[i].name,
							     cmn_pll_hw,
							     fixed_clk[i].rate,
							     cmn_pll->base,
							     fixed_clk[i].enable_bit);
		} else if (fixed_clk[i].rate) {
			hw = clk_hw_register_fixed_rate_parent_hw(dev,
							fixed_clk[i].name,
							cmn_pll_hw, 0,
							fixed_clk[i].rate);
		} else if (!strcmp(fixed_clk[i].name, "eth-pon")) {
			hw = ipq_cmn_pll_eth_pon_register(pdev, cmn_pll->regmap,
							  cmn_pll->base,
							  cmn_pll_hw,
							  &fixed_clk[i]);
		} else if (!strncmp(fixed_clk[i].name, "pcs", 3)) {
			hw = ipq_cmn_pll_pcs_register(pdev, cmn_pll->regmap,
						      cmn_pll_hw,
						      fixed_clk[i].name);
		} else if (!strcmp(fixed_clk[i].name, "nss")) {
			hw = ipq_cmn_pll_nss_register(pdev, cmn_pll->regmap,
						      cmn_pll_hw);
		} else if (!strcmp(fixed_clk[i].name, "ppe")) {
			hw = ipq_cmn_pll_ppe_register(pdev, cmn_pll->regmap,
						      cmn_pll_hw);
		} else if (!strcmp(fixed_clk[i].name, "pon")) {
			hw = ipq_cmn_pll_pon_refclk_register(pdev,
							     cmn_pll->regmap,
							     cmn_pll_hw);
		} else if (!strcmp(fixed_clk[i].name, "ephy-raw")) {
			hw = ipq_cmn_pll_ephy_raw_register(pdev,
							   cmn_pll->regmap,
							   cmn_pll_hw);
		} else {
			continue;
		}

		if (IS_ERR(hw)) {
			ret = PTR_ERR(hw);
			goto unregister_fixed_clk;
		}

		hw_data->hws[fixed_clk[i].id] = hw;
	}

	/*
	 * Provide the CMN PLL clock. The clock rate of CMN PLL
	 * is configured to 12 GHZ by DT property assigned-clock-rates-u64.
	 */
	hw_data->hws[CMN_PLL_CLK] = cmn_pll_hw;
	hw_data->num = num_clks + 1;

	ret = devm_of_clk_add_hw_provider(dev, of_clk_hw_onecell_get, hw_data);
	if (ret)
		goto unregister_fixed_clk;

	platform_set_drvdata(pdev, hw_data);

	return 0;

unregister_fixed_clk:
	while (i > 0)
		clk_hw_unregister(hw_data->hws[fixed_clk[--i].id]);

	return ret;
}

static int ipq_cmn_pll_clk_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;

	if (IS_ENABLED(CONFIG_PM)) {
		ret = devm_pm_runtime_enable(dev);
		if (ret)
			return ret;

		ret = devm_pm_clk_create(dev);
		if (ret)
			return ret;

		/*
		 * To access the CMN PLL registers, the GCC AHB & SYS clocks
		 * of CMN PLL block need to be enabled.
		 */
		ret = pm_clk_add(dev, "ahb");
		if (ret)
			return dev_err_probe(dev, ret, "Failed to add AHB clock\n");

		ret = pm_clk_add(dev, "sys");
		if (ret)
			return dev_err_probe(dev, ret, "Failed to add SYS clock\n");

		ret = pm_runtime_resume_and_get(dev);
		if (ret)
			return ret;

	} else {
		struct clk *clk = NULL;

		clk = devm_clk_get_enabled(dev, "ahb");
		if (IS_ERR(clk))
			return dev_err_probe(dev, PTR_ERR(clk),
					"Failed to enable AHB clock\n");

		clk = devm_clk_get_enabled(dev, "sys");
		if (IS_ERR(clk))
			return dev_err_probe(dev, PTR_ERR(clk),
					"Failed to enable SYS clock\n");
	}

	/* Register CMN PLL clock and fixed rate output clocks. */
	ret = ipq_cmn_pll_register_clks(pdev);

	if (IS_ENABLED(CONFIG_PM))
		pm_runtime_put(dev);

	if (ret)
		return dev_err_probe(dev, ret,
				     "Failed to register CMN PLL clocks\n");

	return 0;
}

static void ipq_cmn_pll_clk_remove(struct platform_device *pdev)
{
	const struct cmn_pll_fixed_output_clk *fixed_clk;
	struct clk_hw_onecell_data *hw_data = platform_get_drvdata(pdev);
	int i;

	fixed_clk = device_get_match_data(&pdev->dev);
	if (!fixed_clk)
		return;

	for (i = 0; fixed_clk[i].name; i++) {
		if (fixed_clk[i].enable_bit == -1 && fixed_clk[i].rate != 0)
			clk_hw_unregister(hw_data->hws[fixed_clk[i].id]);
	}
}

static const struct dev_pm_ops ipq_cmn_pll_pm_ops = {
	SET_RUNTIME_PM_OPS(pm_clk_suspend, pm_clk_resume, NULL)
};

static const struct of_device_id ipq_cmn_pll_clk_ids[] = {
	{ .compatible = "qcom,ipq5018-cmn-pll", .data = &ipq5018_output_clks },
	{ .compatible = "qcom,ipq5210-cmn-pll", .data = &ipq5210_output_clks },
	{ .compatible = "qcom,ipq5332-cmn-pll", .data = &ipq5332_output_clks },
	{ .compatible = "qcom,ipq5424-cmn-pll", .data = &ipq5424_output_clks },
	{ .compatible = "qcom,ipq6018-cmn-pll", .data = &ipq6018_output_clks },
	{ .compatible = "qcom,ipq8074-cmn-pll", .data = &ipq8074_output_clks },
	{ .compatible = "qcom,ipq9574-cmn-pll", .data = &ipq9574_output_clks },
	{ .compatible = "qcom,ipq9650-cmn-pll", .data = &ipq9650_output_clks },
	{ }
};
MODULE_DEVICE_TABLE(of, ipq_cmn_pll_clk_ids);

static struct platform_driver ipq_cmn_pll_clk_driver = {
	.probe = ipq_cmn_pll_clk_probe,
	.remove_new = ipq_cmn_pll_clk_remove,
	.driver = {
		.name = "ipq_cmn_pll",
		.of_match_table = ipq_cmn_pll_clk_ids,
		.pm = &ipq_cmn_pll_pm_ops,
	},
};
module_platform_driver(ipq_cmn_pll_clk_driver);

MODULE_DESCRIPTION("Qualcomm Technologies, Inc. IPQ CMN PLL Driver");
MODULE_LICENSE("GPL");
