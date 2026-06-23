// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/pcs/pcs-qcom-ipq9574.h>
#include <linux/phy.h>
#include <linux/phylink.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/reset.h>

#include <dt-bindings/net/qcom,ipq9574-pcs.h>

/* Maximum number of MIIs per PCS instance. There are 5 MIIs for PSGMII. */
#define PCS_MAX_MII_NRS			5

#define PCS_CALIBRATION			0x1e0
#define PCS_CALIBRATION_DONE		BIT(7)

#define PCS_MISC2			0x218
#define PCS_MISC2_MODE_MASK		GENMASK(6, 5)
#define PCS_MISC2_MODE_SGMII		FIELD_PREP(PCS_MISC2_MODE_MASK, 0x1)
#define PCS_MISC2_MODE_SGMII_PLUS	FIELD_PREP(PCS_MISC2_MODE_MASK, 0x2)

#define PCS_MODE_CTRL			0x46c
#define PCS_MODE_SEL_MASK		GENMASK(12, 8)
#define PCS_MODE_SGMII			FIELD_PREP(PCS_MODE_SEL_MASK, 0x4)
#define PCS_MODE_QSGMII			FIELD_PREP(PCS_MODE_SEL_MASK, 0x1)
#define PCS_MODE_PSGMII			FIELD_PREP(PCS_MODE_SEL_MASK, 0x2)
#define PCS_MODE_2500BASEX		FIELD_PREP(PCS_MODE_SEL_MASK, 0x8)
#define PCS_MODE_XPCS			FIELD_PREP(PCS_MODE_SEL_MASK, 0x10)
#define PCS_MODE_SGMII_MODE_MASK	GENMASK(6, 4)
#define PCS_MODE_SGMII_MODE_MAC		FIELD_PREP(PCS_MODE_SGMII_MODE_MASK, \
						   0x2)
#define PCS_MODE_SGMII_MODE_1000BASEX	FIELD_PREP(PCS_MODE_SGMII_MODE_MASK, \
						   0x0)

#define PCS_MII_CTRL(x)			(0x480 + 0x18 * (x))
#define PCS_MII_ADPT_RESET		BIT(11)
#define PCS_MII_FORCE_MODE		BIT(3)
#define PCS_MII_SPEED_MASK		GENMASK(2, 1)
#define PCS_MII_SPEED_1000		FIELD_PREP(PCS_MII_SPEED_MASK, 0x2)
#define PCS_MII_SPEED_100		FIELD_PREP(PCS_MII_SPEED_MASK, 0x1)
#define PCS_MII_SPEED_10		FIELD_PREP(PCS_MII_SPEED_MASK, 0x0)

#define PCS_MII_STS(x)			(0x488 + 0x18 * (x))
#define PCS_MII_LINK_STS		BIT(7)
#define PCS_MII_STS_DUPLEX_FULL		BIT(6)
#define PCS_MII_STS_SPEED_MASK		GENMASK(5, 4)
#define PCS_MII_STS_SPEED_10		0
#define PCS_MII_STS_SPEED_100		1
#define PCS_MII_STS_SPEED_1000		2
#define PCS_MII_STS_PAUSE_TX_EN		BIT(1)
#define PCS_MII_STS_PAUSE_RX_EN		BIT(0)

#define PCS_QP_USXG_OPTION		0x584
#define PCS_QP_USXG_GMII_SRC_XPCS	BIT(0)

#define PCS_PLL_RESET			0x780
#define PCS_ANA_SW_RESET		BIT(6)

#define XPCS_INDIRECT_ADDR		0x8000
#define XPCS_INDIRECT_AHB_ADDR		0x83fc
#define XPCS_INDIRECT_ADDR_H		GENMASK(20, 8)
#define XPCS_INDIRECT_ADDR_L		GENMASK(7, 0)
#define XPCS_INDIRECT_DATA_ADDR(reg)	(FIELD_PREP(GENMASK(15, 10), 0x20) | \
					 FIELD_PREP(GENMASK(9, 2), \
					 FIELD_GET(XPCS_INDIRECT_ADDR_L, reg)))

#define XPCS_KR_STS			0x30020
#define XPCS_KR_LINK_STS		BIT(12)

#define XPCS_DIG_CTRL			0x38000
#define XPCS_SOFT_RESET			BIT(15)
#define XPCS_USXG_ADPT_RESET		BIT(10)
#define XPCS_USXG_EN			BIT(9)

#define XPCS_KR_CTRL			0x38007
#define XPCS_USXG_MODE_MASK		GENMASK(12, 10)
#define XPCS_10G_QXGMII_MODE		FIELD_PREP(XPCS_USXG_MODE_MASK, 0x5)

#define XPCS_DIG_STS			0x3800a
#define XPCS_DIG_STS_AM_COUNT		GENMASK(14, 0)

/* DIG control for MII1 - MII3 */
#define XPCS_MII1_DIG_CTRL(x)		(0x1a8000 + 0x10000 * ((x) - 1))
#define XPCS_MII1_USXG_ADPT_RESET	BIT(5)

#define XPCS_MII_CTRL			0x1f0000
#define XPCS_MII1_CTRL(x)		(0x1a0000 + 0x10000 * ((x) - 1))
#define XPCS_MII_AN_EN			BIT(12)
#define XPCS_DUPLEX_FULL		BIT(8)
#define XPCS_SPEED_MASK			(BIT(13) | BIT(6) | BIT(5))
#define XPCS_SPEED_10000		(BIT(13) | BIT(6))
#define XPCS_SPEED_5000			(BIT(13) | BIT(5))
#define XPCS_SPEED_2500			BIT(5)
#define XPCS_SPEED_1000			BIT(6)
#define XPCS_SPEED_100			BIT(13)
#define XPCS_SPEED_10			0

#define XPCS_MII_AN_CTRL		0x1f8001
#define XPCS_MII1_AN_CTRL(x)		(0x1a8001 + 0x10000 * ((x) - 1))
#define XPCS_MII_AN_8BIT		BIT(8)

#define XPCS_MII_AN_INTR_STS		0x1f8002
#define XPCS_MII1_AN_INTR_STS(x)	(0x1a8002 + 0x10000 * ((x) - 1))
#define XPCS_USXG_AN_LINK_STS		BIT(14)
#define XPCS_USXG_AN_SPEED_MASK		GENMASK(12, 10)
#define XPCS_USXG_AN_SPEED_10		0
#define XPCS_USXG_AN_SPEED_100		1
#define XPCS_USXG_AN_SPEED_1000		2
#define XPCS_USXG_AN_SPEED_2500		4
#define XPCS_USXG_AN_SPEED_5000		5
#define XPCS_USXG_AN_SPEED_10000	3

#define XPCS_XAUI_MODE_CTRL		0x1f8004
#define XPCS_MII1_XAUI_MODE_CTRL(x)	(0x1a8004 + 0x10000 * ((x) - 1))
#define XPCS_TX_IPG_CHECK_DIS		BIT(0)

/* Per PCS MII private data */
struct ipq_pcs_mii {
	struct ipq_pcs *qpcs;
	struct phylink_pcs pcs;
	int index;

	/* RX clock from NSSCC to PCS MII */
	struct clk *rx_clk;
	/* TX clock from NSSCC to PCS MII */
	struct clk *tx_clk;
};

/* PCS private data */
struct ipq_pcs {
	struct device *dev;
	void __iomem *base;
	struct regmap *regmap;
	phy_interface_t interface;

	/* RX clock supplied to NSSCC */
	struct clk_hw rx_hw;
	/* TX clock supplied to NSSCC */
	struct clk_hw tx_hw;

	struct ipq_pcs_mii *qpcs_mii[PCS_MAX_MII_NRS];
	struct reset_control *xpcs_rstc;
};

#define phylink_pcs_to_qpcs_mii(_pcs)	\
	container_of(_pcs, struct ipq_pcs_mii, pcs)

static void ipq_pcs_get_state_sgmii(struct ipq_pcs *qpcs,
				    int index,
				    struct phylink_link_state *state)
{
	unsigned int val;
	int ret;

	ret = regmap_read(qpcs->regmap, PCS_MII_STS(index), &val);
	if (ret) {
		state->link = 0;
		return;
	}

	state->link = !!(val & PCS_MII_LINK_STS);

	if (!state->link)
		return;

	switch (FIELD_GET(PCS_MII_STS_SPEED_MASK, val)) {
	case PCS_MII_STS_SPEED_1000:
		state->speed = SPEED_1000;
		break;
	case PCS_MII_STS_SPEED_100:
		state->speed = SPEED_100;
		break;
	case PCS_MII_STS_SPEED_10:
		state->speed = SPEED_10;
		break;
	default:
		state->link = false;
		return;
	}

	if (val & PCS_MII_STS_DUPLEX_FULL)
		state->duplex = DUPLEX_FULL;
	else
		state->duplex = DUPLEX_HALF;

	if (val & PCS_MII_STS_PAUSE_TX_EN)
		state->pause |= MLO_PAUSE_TX;
	if (val & PCS_MII_STS_PAUSE_RX_EN)
		state->pause |= MLO_PAUSE_RX;
}

static void ipq_pcs_get_state_2500basex(struct ipq_pcs *qpcs,
					struct phylink_link_state *state)
{
	unsigned int val;
	int ret;

	ret = regmap_read(qpcs->regmap, PCS_MII_STS(0), &val);
	if (ret) {
		state->link = 0;
		return;
	}

	state->link = !!(val & PCS_MII_LINK_STS);

	if (!state->link)
		return;

	state->speed = SPEED_2500;
	state->duplex = DUPLEX_FULL;
	state->pause |= MLO_PAUSE_TXRX_MASK;
}

static void ipq_pcs_get_state_usxgmii(struct ipq_pcs *qpcs, int index,
				      struct phylink_link_state *state)
{
	unsigned int reg, val;
	int ret;

	reg = (index == 0) ? XPCS_MII_AN_INTR_STS : XPCS_MII1_AN_INTR_STS(index);
	ret = regmap_read(qpcs->regmap, reg, &val);
	if (ret) {
		state->link = 0;
		return;
	}

	state->link = !!(val & XPCS_USXG_AN_LINK_STS);

	if (!state->link)
		return;

	switch (FIELD_GET(XPCS_USXG_AN_SPEED_MASK, val)) {
	case XPCS_USXG_AN_SPEED_10000:
		state->speed = SPEED_10000;
		break;
	case XPCS_USXG_AN_SPEED_5000:
		state->speed = SPEED_5000;
		break;
	case XPCS_USXG_AN_SPEED_2500:
		state->speed = SPEED_2500;
		break;
	case XPCS_USXG_AN_SPEED_1000:
		state->speed = SPEED_1000;
		break;
	case XPCS_USXG_AN_SPEED_100:
		state->speed = SPEED_100;
		break;
	case XPCS_USXG_AN_SPEED_10:
		state->speed = SPEED_10;
		break;
	default:
		state->link = false;
		return;
	}

	state->duplex = DUPLEX_FULL;
}

static void ipq_pcs_get_state_10gbaser(struct ipq_pcs *qpcs,
				       struct phylink_link_state *state)
{
	unsigned int val;
	int ret;

	ret = regmap_read(qpcs->regmap, XPCS_KR_STS, &val);
	if (ret) {
		state->link = 0;
		return;
	}

	state->link = !!(val & XPCS_KR_LINK_STS);

	if (!state->link)
		return;

	state->speed = SPEED_10000;
	state->duplex = DUPLEX_FULL;
	state->pause |= MLO_PAUSE_TXRX_MASK;
}

static int ipq_pcs_config_mode(struct ipq_pcs *qpcs,
			       phy_interface_t interface)
{
	unsigned long rate = 125000000;
	unsigned int val, mask, misc2 = 0;
	bool xpcs_mode = false;
	int ret;

	/* Assert XPCS reset */
	reset_control_assert(qpcs->xpcs_rstc);

	/* Configure PCS interface mode */
	mask = PCS_MODE_SEL_MASK;
	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
		mask |= PCS_MODE_SGMII_MODE_MASK;
		val = PCS_MODE_SGMII | PCS_MODE_SGMII_MODE_MAC;
		misc2 = PCS_MISC2_MODE_SGMII;
		break;
	case PHY_INTERFACE_MODE_QSGMII:
		mask |= PCS_MODE_SGMII_MODE_MASK;
		val = PCS_MODE_QSGMII | PCS_MODE_SGMII_MODE_MAC;
		break;
	case PHY_INTERFACE_MODE_PSGMII:
		mask |= PCS_MODE_SGMII_MODE_MASK;
		val = PCS_MODE_PSGMII | PCS_MODE_SGMII_MODE_MAC;
		break;
	case PHY_INTERFACE_MODE_1000BASEX:
		mask |= PCS_MODE_SGMII_MODE_MASK;
		val = PCS_MODE_SGMII | PCS_MODE_SGMII_MODE_1000BASEX;
		misc2 = PCS_MISC2_MODE_SGMII;
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		val = PCS_MODE_2500BASEX;
		misc2 = PCS_MISC2_MODE_SGMII_PLUS;
		rate = 312500000;
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10G_QXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
		val = PCS_MODE_XPCS;
		rate = 312500000;
		xpcs_mode = true;
		break;
	default:
		return -EOPNOTSUPP;
	}

	ret = regmap_update_bits(qpcs->regmap, PCS_MODE_CTRL, mask, val);
	if (ret)
		return ret;

	if (interface == PHY_INTERFACE_MODE_10G_QXGMII) {
		ret = regmap_set_bits(qpcs->regmap, PCS_QP_USXG_OPTION,
				      PCS_QP_USXG_GMII_SRC_XPCS);
		if (ret)
			return ret;
	}

	if (misc2) {
		ret = regmap_update_bits(qpcs->regmap, PCS_MISC2,
					 PCS_MISC2_MODE_MASK, misc2);
		if (ret)
			return ret;
	}

	/* PCS PLL reset */
	ret = regmap_clear_bits(qpcs->regmap, PCS_PLL_RESET, PCS_ANA_SW_RESET);
	if (ret)
		return ret;

	fsleep(20000);
	ret = regmap_set_bits(qpcs->regmap, PCS_PLL_RESET, PCS_ANA_SW_RESET);
	if (ret)
		return ret;

	/* Wait for calibration completion */
	ret = regmap_read_poll_timeout(qpcs->regmap, PCS_CALIBRATION,
				       val, val & PCS_CALIBRATION_DONE,
				       1000, 100000);
	if (ret) {
		dev_err(qpcs->dev, "PCS calibration timed-out\n");
		return ret;
	}

	qpcs->interface = interface;

	/* Configure the RX and TX clock to NSSCC as 125M or 312.5M based
	 * on current interface mode.
	 */
	ret = clk_set_rate(qpcs->rx_hw.clk, rate);
	if (ret) {
		dev_err(qpcs->dev, "Failed to set RX clock rate\n");
		return ret;
	}

	ret = clk_set_rate(qpcs->tx_hw.clk, rate);
	if (ret) {
		dev_err(qpcs->dev, "Failed to set TX clock rate\n");
		return ret;
	}

	/* Deassert XPCS */
	if (xpcs_mode)
		reset_control_deassert(qpcs->xpcs_rstc);

	dev_err(qpcs->dev, "All your rate (%ld) are belong to us(%s %s), phy-mode=%s\n",
		rate,
		__clk_get_name(qpcs->rx_hw.clk),
		__clk_get_name(qpcs->tx_hw.clk),
		phy_modes(interface));
	return 0;
}

static int ipq_pcs_config_sgmii(struct ipq_pcs *qpcs,
				int index,
				unsigned int neg_mode,
				phy_interface_t interface)
{
	int ret;

	/* Configure the PCS mode if required */
	if (qpcs->interface != interface) {
		ret = ipq_pcs_config_mode(qpcs, interface);
		if (ret)
			return ret;
	}

	/* Set AN mode or force mode */
	if (neg_mode == PHYLINK_PCS_NEG_INBAND_ENABLED)
		return regmap_clear_bits(qpcs->regmap,
					 PCS_MII_CTRL(index), PCS_MII_FORCE_MODE);
	else
		return regmap_set_bits(qpcs->regmap,
				       PCS_MII_CTRL(index), PCS_MII_FORCE_MODE);
}

static int ipq_pcs_config_2500basex(struct ipq_pcs *qpcs)
{
	/* Configure PCS for 2500BASEX mode if required */
	if (qpcs->interface == PHY_INTERFACE_MODE_2500BASEX)
		return 0;

	return ipq_pcs_config_mode(qpcs, PHY_INTERFACE_MODE_2500BASEX);
}

static int ipq_pcs_config_usxgmii(struct ipq_pcs *qpcs,
				  int index,
				  phy_interface_t interface)
{
	unsigned int reg;
	int ret;

	/* Configure the XPCS for USXGMII mode if required */
	if (qpcs->interface != interface) {
		ret = ipq_pcs_config_mode(qpcs, interface);
		if (ret)
			return ret;

		ret = regmap_set_bits(qpcs->regmap, XPCS_DIG_CTRL, XPCS_USXG_EN);
		if (ret)
			return ret;

		if (interface == PHY_INTERFACE_MODE_10G_QXGMII) {
			ret = regmap_update_bits(qpcs->regmap, XPCS_KR_CTRL,
						 XPCS_USXG_MODE_MASK, XPCS_10G_QXGMII_MODE);
			if (ret)
				return ret;

			/* Set Alignment Marker Interval value as 0x6018 */
			ret = regmap_update_bits(qpcs->regmap, XPCS_DIG_STS,
						 XPCS_DIG_STS_AM_COUNT, 0x6018);
			if (ret)
				return ret;

			ret = regmap_set_bits(qpcs->regmap, XPCS_DIG_CTRL, XPCS_SOFT_RESET);
			if (ret)
				return ret;
		}
	}

	/* Disable Tx IPG check for 10G_QXGMII */
	if (interface == PHY_INTERFACE_MODE_10G_QXGMII) {
		reg = (index == 0) ? XPCS_XAUI_MODE_CTRL : XPCS_MII1_XAUI_MODE_CTRL(index);
		ret = regmap_set_bits(qpcs->regmap, reg, XPCS_TX_IPG_CHECK_DIS);
		if (ret)
			return ret;
	}

	reg = (index == 0) ? XPCS_MII_AN_CTRL : XPCS_MII1_AN_CTRL(index);
	ret = regmap_set_bits(qpcs->regmap, reg, XPCS_MII_AN_8BIT);
	if (ret)
		return ret;

	reg = (index == 0) ? XPCS_MII_CTRL : XPCS_MII1_CTRL(index);
	return regmap_set_bits(qpcs->regmap, reg, XPCS_MII_AN_EN);
}

static int ipq_pcs_config_10gbaser(struct ipq_pcs *qpcs)
{
	/* Configure 10GBASER mode if required */
	if (qpcs->interface == PHY_INTERFACE_MODE_10GBASER)
		return 0;

	return ipq_pcs_config_mode(qpcs, PHY_INTERFACE_MODE_10GBASER);
}

static int ipq_pcs_link_up_config_sgmii(struct ipq_pcs *qpcs,
					int index,
					unsigned int neg_mode,
					int speed)
{
	unsigned int val;
	int ret;

	/* PCS speed need not be configured if in-band autoneg is enabled */
	if (neg_mode != PHYLINK_PCS_NEG_INBAND_ENABLED) {
		/* PCS speed set for force mode */
		switch (speed) {
		case SPEED_1000:
			val = PCS_MII_SPEED_1000;
			break;
		case SPEED_100:
			val = PCS_MII_SPEED_100;
			break;
		case SPEED_10:
			val = PCS_MII_SPEED_10;
			break;
		default:
			dev_err(qpcs->dev, "Invalid SGMII speed %d\n", speed);
			return -EINVAL;
		}

		ret = regmap_update_bits(qpcs->regmap, PCS_MII_CTRL(index),
					 PCS_MII_SPEED_MASK, val);
		if (ret)
			return ret;
	}

	/* PCS adapter reset */
	ret = regmap_clear_bits(qpcs->regmap,
				PCS_MII_CTRL(index), PCS_MII_ADPT_RESET);
	if (ret)
		return ret;

	return regmap_set_bits(qpcs->regmap,
			       PCS_MII_CTRL(index), PCS_MII_ADPT_RESET);
}

static int ipq_pcs_link_up_config_2500basex(struct ipq_pcs *qpcs, int speed)
{
	int ret;

	/* 2500BASEX does not support autoneg and does not need to
	 * configure PCS speed. Only reset PCS adapter here.
	 */
	ret = regmap_clear_bits(qpcs->regmap,
				PCS_MII_CTRL(0), PCS_MII_ADPT_RESET);
	if (ret)
		return ret;

	return regmap_set_bits(qpcs->regmap,
			       PCS_MII_CTRL(0), PCS_MII_ADPT_RESET);
}

static int ipq_pcs_link_up_config_usxgmii(struct ipq_pcs *qpcs,
					  int index, int speed)
{
	unsigned int reg, val;
	int ret;

	switch (speed) {
	case SPEED_10000:
		val = XPCS_SPEED_10000;
		break;
	case SPEED_5000:
		val = XPCS_SPEED_5000;
		break;
	case SPEED_2500:
		val = XPCS_SPEED_2500;
		break;
	case SPEED_1000:
		val = XPCS_SPEED_1000;
		break;
	case SPEED_100:
		val = XPCS_SPEED_100;
		break;
	case SPEED_10:
		val = XPCS_SPEED_10;
		break;
	default:
		dev_err(qpcs->dev, "Invalid USXGMII speed %d\n", speed);
		return -EINVAL;
	}

	/* Configure XPCS speed */
	reg = (index == 0) ? XPCS_MII_CTRL : XPCS_MII1_CTRL(index);
	ret = regmap_update_bits(qpcs->regmap, reg,
				 XPCS_SPEED_MASK, val | XPCS_DUPLEX_FULL);
	if (ret)
		return ret;

	/* XPCS adapter reset */
	reg = (index == 0) ? XPCS_DIG_CTRL : XPCS_MII1_DIG_CTRL(index);
	val = (index == 0) ? XPCS_USXG_ADPT_RESET : XPCS_MII1_USXG_ADPT_RESET;
	return regmap_set_bits(qpcs->regmap, reg, val);
}

static int ipq_pcs_validate(struct phylink_pcs *pcs, unsigned long *supported,
			    const struct phylink_link_state *state)
{
	struct ipq_pcs_mii *qpcs_mii = phylink_pcs_to_qpcs_mii(pcs);
	struct ipq_pcs *qpcs = qpcs_mii->qpcs;

	switch (state->interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_QSGMII:
	case PHY_INTERFACE_MODE_PSGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
	case PHY_INTERFACE_MODE_10GBASER:
		return 0;
	case PHY_INTERFACE_MODE_2500BASEX:
		/* In-band autoneg is not supported for 2500BASEX */
		phylink_clear(supported, Autoneg);
		return 0;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10G_QXGMII:
		/* USXGMII only supports full duplex mode */
		phylink_clear(supported, 100baseT_Half);
		phylink_clear(supported, 10baseT_Half);
		return 0;
	default:
		WARN_ON(1);
		dev_err(qpcs->dev, "interface %s not supported\n",
			phy_modes(state->interface));
		return -EINVAL;
	}
}

static unsigned int ipq_pcs_inband_caps(struct phylink_pcs *pcs,
					phy_interface_t interface)
{
	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_QSGMII:
	case PHY_INTERFACE_MODE_PSGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10G_QXGMII:
		return LINK_INBAND_DISABLE | LINK_INBAND_ENABLE;
	case PHY_INTERFACE_MODE_2500BASEX:
	case PHY_INTERFACE_MODE_10GBASER:
		return LINK_INBAND_DISABLE;
	default:
		return 0;
	}
}

static int ipq_pcs_enable(struct phylink_pcs *pcs)
{
	struct ipq_pcs_mii *qpcs_mii = phylink_pcs_to_qpcs_mii(pcs);
	struct ipq_pcs *qpcs = qpcs_mii->qpcs;
	int index = qpcs_mii->index;
	int ret;

	ret = clk_prepare_enable(qpcs_mii->rx_clk);
	if (ret) {
		dev_err(qpcs->dev, "Failed to enable MII %d RX clock\n", index);
		return ret;
	}

	ret = clk_prepare_enable(qpcs_mii->tx_clk);
	if (ret) {
		/* This is a fatal event since phylink does not support unwinding
		 * the state back for this error. So, we only report the error
		 * and do not disable the clocks.
		 */
		dev_err(qpcs->dev, "Failed to enable MII %d TX clock\n", index);
		return ret;
	}

	return 0;
}

static void ipq_pcs_disable(struct phylink_pcs *pcs)
{
	struct ipq_pcs_mii *qpcs_mii = phylink_pcs_to_qpcs_mii(pcs);

	clk_disable_unprepare(qpcs_mii->rx_clk);
	clk_disable_unprepare(qpcs_mii->tx_clk);
}

static void ipq_pcs_get_state(struct phylink_pcs *pcs,
			      struct phylink_link_state *state)
{
	struct ipq_pcs_mii *qpcs_mii = phylink_pcs_to_qpcs_mii(pcs);
	struct ipq_pcs *qpcs = qpcs_mii->qpcs;
	int index = qpcs_mii->index;

	switch (state->interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_QSGMII:
	case PHY_INTERFACE_MODE_PSGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
		/* SGMII and 1000BASEX in-band autoneg word format are decoded
		 * by PCS hardware and both placed to the same status register.
		 */
		ipq_pcs_get_state_sgmii(qpcs, index, state);
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		ipq_pcs_get_state_2500basex(qpcs, state);
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10G_QXGMII:
		ipq_pcs_get_state_usxgmii(qpcs, index, state);
		break;
	case PHY_INTERFACE_MODE_10GBASER:
		ipq_pcs_get_state_10gbaser(qpcs, state);
		break;
	default:
		break;
	}

	dev_dbg_ratelimited(qpcs->dev,
			    "mode=%s/%s/%s link=%u\n",
			    phy_modes(state->interface),
			    phy_speed_to_str(state->speed),
			    phy_duplex_to_str(state->duplex),
			    state->link);
}

static int ipq_pcs_config(struct phylink_pcs *pcs,
			  unsigned int neg_mode,
			  phy_interface_t interface,
			  const unsigned long *advertising,
			  bool permit)
{
	struct ipq_pcs_mii *qpcs_mii = phylink_pcs_to_qpcs_mii(pcs);
	struct ipq_pcs *qpcs = qpcs_mii->qpcs;
	int index = qpcs_mii->index;

	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_QSGMII:
	case PHY_INTERFACE_MODE_PSGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
		return ipq_pcs_config_sgmii(qpcs, index, neg_mode, interface);
	case PHY_INTERFACE_MODE_2500BASEX:
		return ipq_pcs_config_2500basex(qpcs);
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10G_QXGMII:
		return ipq_pcs_config_usxgmii(qpcs, index, interface);
	case PHY_INTERFACE_MODE_10GBASER:
		return ipq_pcs_config_10gbaser(qpcs);
	default:
		return -EOPNOTSUPP;
	};
}

static void ipq_pcs_an_restart(struct phylink_pcs *pcs)
{
	/* Currently not used */
}

static void ipq_pcs_link_up(struct phylink_pcs *pcs,
			    unsigned int neg_mode,
			    phy_interface_t interface,
			    int speed, int duplex)
{
	struct ipq_pcs_mii *qpcs_mii = phylink_pcs_to_qpcs_mii(pcs);
	struct ipq_pcs *qpcs = qpcs_mii->qpcs;
	int index = qpcs_mii->index;
	int ret;

	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_QSGMII:
	case PHY_INTERFACE_MODE_PSGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
		ret = ipq_pcs_link_up_config_sgmii(qpcs, index,
						   neg_mode, speed);
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		ret = ipq_pcs_link_up_config_2500basex(qpcs, speed);
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10G_QXGMII:
		ret = ipq_pcs_link_up_config_usxgmii(qpcs, index, speed);
		break;
	case PHY_INTERFACE_MODE_10GBASER:
		/* Nothing to do here */
		return;
	default:
		dev_err(qpcs->dev,
			"interface %s not supported\n", phy_modes(interface));
		return;
	}

	if (ret)
		dev_err(qpcs->dev, "PCS link up fail for interface %s\n",
			phy_modes(interface));
}

static const struct phylink_pcs_ops ipq_pcs_phylink_ops = {
	.pcs_validate = ipq_pcs_validate,
	.pcs_inband_caps = ipq_pcs_inband_caps,
	.pcs_enable = ipq_pcs_enable,
	.pcs_disable = ipq_pcs_disable,
	.pcs_get_state = ipq_pcs_get_state,
	.pcs_config = ipq_pcs_config,
	.pcs_an_restart = ipq_pcs_an_restart,
	.pcs_link_up = ipq_pcs_link_up,
};

/* Parse the PCS MII DT nodes which are child nodes of the PCS node,
 * and instantiate each MII PCS instance.
 */
static int ipq_pcs_create_miis(struct ipq_pcs *qpcs)
{
	struct device *dev = qpcs->dev;
	struct ipq_pcs_mii *qpcs_mii;
	struct device_node *mii_np;
	u32 index;
	int ret;

	for_each_available_child_of_node(dev->of_node, mii_np) {
		ret = of_property_read_u32(mii_np, "reg", &index);
		if (ret) {
			dev_err(dev, "Failed to read MII index\n");
			of_node_put(mii_np);
			return ret;
		}

		if (index >= PCS_MAX_MII_NRS) {
			dev_err(dev, "Invalid MII index\n");
			of_node_put(mii_np);
			return -EINVAL;
		}

		qpcs_mii = devm_kzalloc(dev, sizeof(*qpcs_mii), GFP_KERNEL);
		if (!qpcs_mii) {
			of_node_put(mii_np);
			return -ENOMEM;
		}

		qpcs_mii->qpcs = qpcs;
		qpcs_mii->index = index;
		qpcs_mii->pcs.ops = &ipq_pcs_phylink_ops;
		qpcs_mii->pcs.poll = true;

		qpcs->qpcs_mii[index] = qpcs_mii;
	}

	return 0;
}

static unsigned long ipq_pcs_clk_rate_get(struct ipq_pcs *qpcs)
{
	switch (qpcs->interface) {
	case PHY_INTERFACE_MODE_2500BASEX:
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10G_QXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
		return 312500000;
	default:
		return 125000000;
	}
}

/* Return clock rate for the RX clock supplied to NSSCC
 * as per the interface mode.
 */
static unsigned long ipq_pcs_rx_clk_recalc_rate(struct clk_hw *hw,
						unsigned long parent_rate)
{
	struct ipq_pcs *qpcs = container_of(hw, struct ipq_pcs, rx_hw);

	return ipq_pcs_clk_rate_get(qpcs);
}

/* Return clock rate for the TX clock supplied to NSSCC
 * as per the interface mode.
 */
static unsigned long ipq_pcs_tx_clk_recalc_rate(struct clk_hw *hw,
						unsigned long parent_rate)
{
	struct ipq_pcs *qpcs = container_of(hw, struct ipq_pcs, tx_hw);

	return ipq_pcs_clk_rate_get(qpcs);
}

static int ipq_pcs_clk_determine_rate(struct clk_hw *hw,
				      struct clk_rate_request *req)
{
	switch (req->rate) {
	case 125000000:
	case 312500000:
		return 0;
	default:
		return -EINVAL;
	}
}

/* Clock ops for the RX clock supplied to NSSCC */
static const struct clk_ops ipq_pcs_rx_clk_ops = {
	.determine_rate = ipq_pcs_clk_determine_rate,
	.recalc_rate = ipq_pcs_rx_clk_recalc_rate,
};

/* Clock ops for the TX clock supplied to NSSCC */
static const struct clk_ops ipq_pcs_tx_clk_ops = {
	.determine_rate = ipq_pcs_clk_determine_rate,
	.recalc_rate = ipq_pcs_tx_clk_recalc_rate,
};

static struct clk_hw *ipq_pcs_clk_hw_get(struct of_phandle_args *clkspec,
					 void *data)
{
	struct ipq_pcs *qpcs = data;

	switch (clkspec->args[0]) {
	case PCS_RX_CLK:
		return &qpcs->rx_hw;
	case PCS_TX_CLK:
		return &qpcs->tx_hw;
	}

	return ERR_PTR(-EINVAL);
}

/* Register the RX and TX clock which are output from SerDes to
 * the NSSCC. The NSSCC driver assigns the RX and TX clock as
 * parent, divides them to generate the MII RX and TX clock to
 * each MII interface of the PCS as per the link speeds and
 * interface modes.
 */
static int ipq_pcs_clk_register(struct ipq_pcs *qpcs)
{
	struct clk_init_data init = { };
	int ret;

	init.ops = &ipq_pcs_rx_clk_ops;
	init.name = devm_kasprintf(qpcs->dev, GFP_KERNEL, "%s::rx_clk",
				   dev_name(qpcs->dev));
	if (!init.name)
		return -ENOMEM;

	qpcs->rx_hw.init = &init;
	ret = devm_clk_hw_register(qpcs->dev, &qpcs->rx_hw);
	if (ret)
		return ret;

	init.ops = &ipq_pcs_tx_clk_ops;
	init.name = devm_kasprintf(qpcs->dev, GFP_KERNEL, "%s::tx_clk",
				   dev_name(qpcs->dev));
	if (!init.name)
		return -ENOMEM;

	qpcs->tx_hw.init = &init;
	ret = devm_clk_hw_register(qpcs->dev, &qpcs->tx_hw);
	if (ret)
		return ret;

	return devm_of_clk_add_hw_provider(qpcs->dev, ipq_pcs_clk_hw_get, qpcs);
}

static int ipq_pcs_regmap_read(void *context, unsigned int reg,
			       unsigned int *val)
{
	struct ipq_pcs *qpcs = context;

	/* PCS uses direct AHB access while XPCS uses indirect AHB access */
	if (reg >= XPCS_INDIRECT_ADDR) {
		writel(FIELD_GET(XPCS_INDIRECT_ADDR_H, reg),
		       qpcs->base + XPCS_INDIRECT_AHB_ADDR);
		*val = readl(qpcs->base + XPCS_INDIRECT_DATA_ADDR(reg));
	} else {
		*val = readl(qpcs->base + reg);
	}

	return 0;
}

static int ipq_pcs_regmap_write(void *context, unsigned int reg,
				unsigned int val)
{
	struct ipq_pcs *qpcs = context;

	/* PCS uses direct AHB access while XPCS uses indirect AHB access */
	if (reg >= XPCS_INDIRECT_ADDR) {
		writel(FIELD_GET(XPCS_INDIRECT_ADDR_H, reg),
		       qpcs->base + XPCS_INDIRECT_AHB_ADDR);
		writel(val, qpcs->base + XPCS_INDIRECT_DATA_ADDR(reg));
	} else {
		writel(val, qpcs->base + reg);
	}

	return 0;
}

static const struct regmap_config ipq_pcs_regmap_cfg = {
	.reg_bits = 32,
	.val_bits = 32,
	.reg_read = ipq_pcs_regmap_read,
	.reg_write = ipq_pcs_regmap_write,
	.fast_io = true,
};

static int ipq9574_pcs_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ipq_pcs *qpcs;
	struct clk *clk;
	int ret;

	qpcs = devm_kzalloc(dev, sizeof(*qpcs), GFP_KERNEL);
	if (!qpcs)
		return -ENOMEM;

	qpcs->dev = dev;

	qpcs->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(qpcs->base))
		return dev_err_probe(dev, PTR_ERR(qpcs->base),
				     "Failed to ioremap resource\n");

	qpcs->regmap = devm_regmap_init(dev, NULL, qpcs, &ipq_pcs_regmap_cfg);
	if (IS_ERR(qpcs->regmap))
		return dev_err_probe(dev, PTR_ERR(qpcs->regmap),
				     "Failed to allocate register map\n");

	clk = devm_clk_get_enabled(dev, "sys");
	if (IS_ERR(clk))
		return dev_err_probe(dev, PTR_ERR(clk),
				     "Failed to enable SYS clock\n");

	clk = devm_clk_get_enabled(dev, "ahb");
	if (IS_ERR(clk))
		return dev_err_probe(dev, PTR_ERR(clk),
				     "Failed to enable AHB clock\n");

	qpcs->xpcs_rstc = devm_reset_control_get_optional(dev, NULL);
	if (IS_ERR_OR_NULL(qpcs->xpcs_rstc))
		return dev_err_probe(dev, PTR_ERR(qpcs->xpcs_rstc),
				     "Failed to get XPCS reset\n");

	ret = ipq_pcs_clk_register(qpcs);
	if (ret)
		return ret;

	ret = ipq_pcs_create_miis(qpcs);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, qpcs);

	return 0;
}

static const struct of_device_id ipq9574_pcs_of_mtable[] = {
	{ .compatible = "qcom,ipq9574-pcs" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, ipq9574_pcs_of_mtable);

/**
 * ipq_pcs_get() - Get the IPQ PCS MII instance
 * @np: Device tree node to the PCS MII
 *
 * Description: Get the phylink PCS instance for the given PCS MII node @np.
 * This instance is associated with the specific MII of the PCS and the
 * corresponding Ethernet netdevice.
 *
 * Return: A pointer to the phylink PCS instance or an error-pointer value.
 */
struct phylink_pcs *ipq_pcs_get(struct device_node *np)
{
	struct platform_device *pdev;
	struct ipq_pcs_mii *qpcs_mii;
	struct ipq_pcs *qpcs;
	u32 index;

	if (of_property_read_u32(np, "reg", &index))
		return ERR_PTR(-EINVAL);

	if (index >= PCS_MAX_MII_NRS)
		return ERR_PTR(-EINVAL);

	if (!of_match_node(ipq9574_pcs_of_mtable, np->parent))
		return ERR_PTR(-EINVAL);

	/* Get the parent device */
	pdev = of_find_device_by_node(np->parent);
	if (!pdev)
		return ERR_PTR(-ENODEV);

	qpcs = platform_get_drvdata(pdev);
	if (!qpcs) {
		put_device(&pdev->dev);

		/* If probe is not yet completed, return DEFER to
		 * the dependent driver.
		 */
		return ERR_PTR(-EPROBE_DEFER);
	}

	qpcs_mii = qpcs->qpcs_mii[index];
	if (!qpcs_mii) {
		put_device(&pdev->dev);
		return ERR_PTR(-ENOENT);
	}

	qpcs_mii->rx_clk = devm_get_clk_from_child(&pdev->dev, np, "rx");
	if (IS_ERR(qpcs_mii->rx_clk)) {
		put_device(&pdev->dev);
		return dev_err_ptr_probe(&pdev->dev, PTR_ERR(qpcs_mii->rx_clk),
					 "Failed to get MII %d RX clock\n",
					 index);
	}

	qpcs_mii->tx_clk = devm_get_clk_from_child(&pdev->dev, np, "tx");
	if (IS_ERR(qpcs_mii->tx_clk)) {
		put_device(&pdev->dev);
		return dev_err_ptr_probe(&pdev->dev, PTR_ERR(qpcs_mii->tx_clk),
					 "Failed to get MII %d TX clock\n",
					 index);
	}

	return &qpcs_mii->pcs;
}
EXPORT_SYMBOL(ipq_pcs_get);

/**
 * ipq_pcs_put() - Release the IPQ PCS MII instance
 * @pcs: PCS instance
 *
 * Description: Release a phylink PCS instance.
 */
void ipq_pcs_put(struct phylink_pcs *pcs)
{
	struct ipq_pcs_mii *qpcs_mii = phylink_pcs_to_qpcs_mii(pcs);

	/* Put reference taken by of_find_device_by_node() in
	 * ipq_pcs_get().
	 */
	put_device(qpcs_mii->qpcs->dev);
}
EXPORT_SYMBOL(ipq_pcs_put);

static struct platform_driver ipq9574_pcs_driver = {
	.driver = {
		.name = "ipq9574_pcs",
		.suppress_bind_attrs = true,
		.of_match_table = ipq9574_pcs_of_mtable,
	},
	.probe = ipq9574_pcs_probe,
};
module_platform_driver(ipq9574_pcs_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Qualcomm IPQ9574 PCS driver");
MODULE_AUTHOR("Lei Wei <quic_leiwei@quicinc.com>");
