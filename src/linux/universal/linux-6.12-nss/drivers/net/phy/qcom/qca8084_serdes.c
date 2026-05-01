// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/dev_printk.h>
#include <linux/mdio.h>
#include <linux/of.h>
#include <linux/phy.h>
#include <linux/property.h>
#include <linux/reset.h>

#include "qca8084_serdes.h"

/* XPCS includes 4 channels, each channel has the different MMD ID for
 * configuring auto-negotiation complete interrupt, mii-4bit, auto-
 * negotiation capabilities and TX configuration for the connected PHY.
 *
 * MMD31 is for channel 0;
 * MMD26 is for channel 1;
 * MMD27 is for channel 2;
 * MMD28 is for channel 3;
 */
#define QCA8084_CHANNEL_MAX			4

/* MII registers */
#define PLL_POWER_ON_AND_RESET			0x0
#define PCS_ANA_SW_RESET			BIT(6)

#define PLL_CONTROL				6
#define PLL_CONTROL_CMLDIV2_IBSEL_MASK		GENMASK(5, 4)

/* MMD_PMAPMD registers */
#define CDR_CONTRL				0x20
#define SSC_FIX_MODE				BIT(3)

#define CALIBRATION4				0x78
#define CALIBRATION_DONE			BIT(7)

#define MODE_CONTROL				0x11b
#define MODE_CONTROL_SEL_MASK			GENMASK(12, 8)
#define MODE_CONTROL_XPCS			0x10
#define MODE_CONTROL_SGMII_PLUS			0x8
#define MODE_CONTROL_SGMII			0x4
#define MODE_CONTROL_SGMII_SEL_MASK		GENMASK(6, 4)
#define MODE_CONTROL_SGMII_PHY			1
#define MODE_CONTROL_SGMII_MAC			2

#define QP_USXG_OPTION1				0x180
#define QP_USXG_OPTION1_DATAPASS		BIT(0)
#define QP_USXG_OPTION1_DATAPASS_SGMII		0
#define QP_USXG_OPTION1_DATAPASS_USXGMII	1

#define BYPASS_TUNNING_IPG			0x189
#define BYPASS_TUNNING_IPG_MASK			GENMASK(11, 0)

#define QP_USXG_RESET				0x18c
#define QP_USXG_SGMII_FUNC_RESET		BIT(4)
#define QP_USXG_P3_FUNC_RESET			BIT(3)
#define QP_USXG_P2_FUNC_RESET			BIT(2)
#define QP_USXG_P1_FUNC_RESET			BIT(1)
#define QP_USXG_P0_FUNC_RESET			BIT(0)

/* MDIO_MMD_PCS register */
#define PCS_CONTROL2				0x7
#define PCS_TYPE_MASK				GENMASK(3, 0)
#define PCS_TYPE_BASER				0

#define PCS_EEE_CONTROL				0x14
#define EEE_CAPABILITY				BIT(6)

#define PCS_STATUS1				0x20
#define PCS_BASER_UP				BIT(12)

#define DIG_CTRL1				0x8000
#define DIG_CTRL1_USXGMII_EN			BIT(9)
#define DIG_CTRL1_XPCS_RESET			BIT(15)
#define FIFO_RESET_CH0				BIT(10)
#define FIFO_RESET_CH1_CH2_CH3			BIT(5)

#define EEE_MODE_CONTROL			0x8006
#define EEE_LCT_RES				GENMASK(11, 8)
#define EEE_SIGN				BIT(6)
#define EEE_LRX_EN				BIT(1)
#define EEE_LTX_EN				BIT(0)

#define PCS_TPC					0x8007
#define PCS_QXGMII_MODE_MASK			GENMASK(12, 10)
#define PCS_QXGMII_EN				0x5

#define EEE_RX_TIMER				0x8009
#define EEE_RX_TIMER_100US_RES			GENMASK(7, 0)
#define EEE_RX_TIMER_RWR_RES			GENMASK(12, 8)

#define AM_LINK_TIMER				0x800a
#define AM_LINK_TIMER_VAL			0x6018

#define EEE_MODE_CONTROL1			0x800b
#define TRANS_LPI_MODE				BIT(0)
#define TRANS_RX_LPI_MODE			BIT(8)

/* QXGMII channel MMD register */
#define MII_CONTROL				0x0
#define AUTO_NEGOTIATION_EN			BIT(12)
#define AUTO_NEGOTIATION_RESTART		BIT(9)
#define PCS_SPEED_2500				BIT(5)
#define PCS_SPEED_1000				BIT(6)
#define PCS_SPEED_100				BIT(13)
#define PCS_SPEED_10				0

#define DIG_CONTROL2				0x8001
#define MII_BIT_CONTROL				BIT(8)
#define TX_CONFIG				BIT(3)
#define AUTO_NEGOTIATION_CMPLT_INTR		BIT(0)

#define PCS_ERR_SEL				0x8002
#define PCS_AN_COMPLETE				BIT(0)

#define XAUI_CONTROL				0x8004
#define TX_IPG_CHECK_DISABLE			BIT(0)

enum pcs_clk_id {
	PCS_CLK,
	PCS_RX_ROOT_CLK,
	PCS_TX_ROOT_CLK,
	PCS_CLK_MAX
};

enum xpcs_clk_id {
	XPCS_XGMII_RX_CLK,
	XPCS_XGMII_TX_CLK,
	XPCS_RX_CLK,
	XPCS_TX_CLK,
	XPCS_PORT_RX_CLK,
	XPCS_PORT_TX_CLK,
	XPCS_RX_SRC_CLK,
	XPCS_TX_SRC_CLK,
	XPCS_CLK_MAX
};

struct qca8084_xpcs_channel_priv {
	int ch_id;
	struct reset_control *rstcs;
	struct clk *clks[XPCS_CLK_MAX];
};

struct qca8084_pcs_data {
	struct reset_control *rstc;
	struct clk *clks[PCS_CLK_MAX];
};

struct qca8084_xpcs_data {
	struct reset_control *rstc;
	struct qca8084_xpcs_channel_priv xpcs_ch[QCA8084_CHANNEL_MAX];
};

static const char *const xpcs_clock_names[XPCS_CLK_MAX] = {
	[XPCS_XGMII_RX_CLK] =	"xgmii_rx",
	[XPCS_XGMII_TX_CLK] =	"xgmii_tx",
	[XPCS_RX_CLK] =		"xpcs_rx",
	[XPCS_TX_CLK] =		"xpcs_tx",
	[XPCS_PORT_RX_CLK] =	"port_rx",
	[XPCS_PORT_TX_CLK] =	"port_tx",
	[XPCS_RX_SRC_CLK] =	"rx_src",
	[XPCS_TX_SRC_CLK] =	"tx_src",
};

static const char *const pcs_clock_names[PCS_CLK_MAX] = {
	[PCS_CLK] =		"pcs",
	[PCS_RX_ROOT_CLK] =	"pcs_rx_root",
	[PCS_TX_ROOT_CLK] =	"pcs_tx_root",
};

static const int qca8084_xpcs_ch_mmd[QCA8084_CHANNEL_MAX] = { 31, 26, 27, 28 };

struct mdio_device *qca8084_package_pcs_probe(struct device_node *pcs_np)
{
	struct qca8084_pcs_data *pcs_data;
	struct mdio_device *mdiodev;
	struct reset_control *rstc;
	struct device *dev;
	struct clk *clk;
	int i;

	mdiodev = fwnode_mdio_find_device(of_fwnode_handle(pcs_np));
	if (!mdiodev)
		return ERR_PTR(-EPROBE_DEFER);

	dev = &mdiodev->dev;
	pcs_data = devm_kzalloc(dev, sizeof(*pcs_data), GFP_KERNEL);
	if (!pcs_data) {
		dev_err(dev, "Allocate PCS data failed\n");
		return ERR_PTR(-ENOMEM);
	}

	rstc = devm_reset_control_get_exclusive(dev, NULL);
	if (IS_ERR(rstc)) {
		dev_err(dev, "Get PCS reset failed\n");
		return ERR_CAST(rstc);
	}

	pcs_data->rstc = rstc;

	for (i = 0; i < ARRAY_SIZE(pcs_clock_names); i++) {
		clk = devm_clk_get(dev, pcs_clock_names[i]);
		if (IS_ERR(clk)) {
			dev_err(dev, "Failed to get the PCS clock ID %s\n",
				pcs_clock_names[i]);
			return ERR_CAST(clk);
		}
		pcs_data->clks[i] = clk;
	}

	mdiodev_set_drvdata(mdiodev, pcs_data);

	return mdiodev;
}

struct mdio_device *qca8084_package_xpcs_probe(struct device_node *xpcs_np)
{
	struct qca8084_xpcs_data *xpcs_data;
	struct mdio_device *mdiodev;
	struct reset_control *rstc;
	struct device_node *child;
	struct device *dev;
	struct clk *clk;
	int i, j, node;

	mdiodev = fwnode_mdio_find_device(of_fwnode_handle(xpcs_np));
	if (!mdiodev)
		return ERR_PTR(-EPROBE_DEFER);

	dev = &mdiodev->dev;

	xpcs_data = devm_kzalloc(dev, sizeof(*xpcs_data), GFP_KERNEL);
	if (!xpcs_data) {
		dev_err(dev, "Allocate XPCS data failed\n");
		return ERR_PTR(-ENOMEM);
	}

	rstc = devm_reset_control_get_exclusive(dev, NULL);
	if (IS_ERR(rstc)) {
		dev_err(dev, "Get XPCS reset failed\n");
		return ERR_CAST(rstc);
	}

	xpcs_data->rstc = rstc;

	/* Sanity check the number of channel sub nodes */
	node = of_get_available_child_count(xpcs_np);
	if (node != QCA8084_CHANNEL_MAX)
		return ERR_PTR(-EINVAL);

	node = 0;
	for_each_available_child_of_node(xpcs_np, child) {
		struct qca8084_xpcs_channel_priv *ch_data;
		u32 channel;

		/* The subnode name must be 'channel'. */
		if (!(of_node_name_eq(child, "channel")))
			continue;

		if (of_property_read_u32(child, "reg", &channel)) {
			dev_err(dev, "%s: Failed to get reg\n",
				child->full_name);

			mdiodev = ERR_PTR(-EINVAL);
			goto put_ch_clk_rst;
		}

		if (channel >= QCA8084_CHANNEL_MAX) {
			dev_err(dev, "%s: Invalid reg %d\n",
				child->full_name, channel);

			mdiodev = ERR_PTR(-EINVAL);
			goto put_ch_clk_rst;
		}

		ch_data = &xpcs_data->xpcs_ch[node];
		ch_data->ch_id = channel;

		ch_data->rstcs = of_reset_control_array_get_exclusive(child);
		if (IS_ERR(ch_data->rstcs)) {
			dev_err(dev, "%s: Failed to get reset\n",
				child->full_name);

			mdiodev = ERR_CAST(ch_data->rstcs);
			goto put_ch_clk_rst;
		}

		for (j = 0; j < ARRAY_SIZE(xpcs_clock_names); j++) {
			clk = of_clk_get_by_name(child, xpcs_clock_names[j]);
			if (IS_ERR(clk)) {
				dev_err(dev, "Failed to get the clock ID %s\n",
					xpcs_clock_names[j]);
				mdiodev = ERR_CAST(clk);
				goto put_ch_child;
			}
			ch_data->clks[j] = clk;
		}

		node++;
	}

	mdiodev_set_drvdata(mdiodev, xpcs_data);

	return mdiodev;

put_ch_child:
	node++;

put_ch_clk_rst:
	for (i = 0; i < node; i++) {
		j--;
		while (j >= 0) {
			clk_put(xpcs_data->xpcs_ch[i].clks[j]);
			j--;
		}

		j = ARRAY_SIZE(xpcs_clock_names);
	}

	for (i = 0; i < node; i++)
		reset_control_put(xpcs_data->xpcs_ch[i].rstcs);

	of_node_put(child);

	return mdiodev;
}

void qca8084_package_xpcs_and_pcs_remove(struct mdio_device *xpcs_mdiodev,
					 struct mdio_device *pcs_mdiodev)
{
	struct qca8084_xpcs_data *xpcs_data = mdiodev_get_drvdata(xpcs_mdiodev);
	int i, j;

	for (i = 0; i < ARRAY_SIZE(xpcs_data->xpcs_ch); i++) {
		reset_control_put(xpcs_data->xpcs_ch[i].rstcs);

		for (j = 0; j < ARRAY_SIZE(xpcs_data->xpcs_ch[i].clks); j++)
			clk_put(xpcs_data->xpcs_ch[i].clks[j]);
	}

	mdio_device_put(xpcs_mdiodev);
	mdio_device_put(pcs_mdiodev);
}

static int qca8084_pcs_set_interface_mode(struct mdio_device *mdio_dev,
					  phy_interface_t ifmode)
{
	int ret, hw_ifmode, data;

	switch (ifmode) {
	case PHY_INTERFACE_MODE_SGMII:
		hw_ifmode = MODE_CONTROL_SGMII;
		data = QP_USXG_OPTION1_DATAPASS_SGMII;
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		hw_ifmode = MODE_CONTROL_SGMII_PLUS;
		data = QP_USXG_OPTION1_DATAPASS_SGMII;
		break;
	case PHY_INTERFACE_MODE_10G_QXGMII:
		hw_ifmode = MODE_CONTROL_XPCS;
		data = QP_USXG_OPTION1_DATAPASS_USXGMII;
		break;
	default:
		return -EOPNOTSUPP;
	}

	/* For PLL stable under high temperature */
	ret = mdiodev_modify(mdio_dev, PLL_CONTROL,
			     PLL_CONTROL_CMLDIV2_IBSEL_MASK,
			     FIELD_PREP(PLL_CONTROL_CMLDIV2_IBSEL_MASK, 3));
	if (ret)
		return ret;

	/* Configure the interface mode of PCS */
	ret = mdiodev_c45_modify(mdio_dev, MDIO_MMD_PMAPMD, MODE_CONTROL,
				 MODE_CONTROL_SEL_MASK,
				 FIELD_PREP(MODE_CONTROL_SEL_MASK, hw_ifmode));
	if (ret)
		return ret;

	/* Data pass selects SGMII or USXGMII */
	return mdiodev_c45_modify(mdio_dev, MDIO_MMD_PMAPMD, QP_USXG_OPTION1,
				  QP_USXG_OPTION1_DATAPASS,
				  FIELD_PREP(QP_USXG_OPTION1_DATAPASS, data));
}

static int qca8084_do_calibration(struct mdio_device *mdio_dev)
{
	int ret;

	ret = mdiodev_modify(mdio_dev, PLL_POWER_ON_AND_RESET,
			     PCS_ANA_SW_RESET, 0);
	if (ret)
		return ret;

	usleep_range(10000, 11000);
	ret = mdiodev_modify(mdio_dev, PLL_POWER_ON_AND_RESET,
			     PCS_ANA_SW_RESET, PCS_ANA_SW_RESET);
	if (ret)
		return ret;

	/* Wait calibration done */
	return read_poll_timeout(mdiodev_c45_read, ret,
				 (ret & CALIBRATION_DONE),
				 100, 100000, true, mdio_dev,
				 MDIO_MMD_PMAPMD, CALIBRATION4);
}


static int qca8084_xpcs_set_mode(struct mdio_device *xpcs_mdiodev)
{
	int ret, val, i;

	/* Configure BaseR mode */
	ret = mdiodev_c45_modify(xpcs_mdiodev, MDIO_MMD_PCS, PCS_CONTROL2,
				 PCS_TYPE_MASK,
				 FIELD_PREP(PCS_TYPE_MASK, PCS_TYPE_BASER));
	if (ret)
		return ret;

	/* Wait BaseR link up */
	ret = read_poll_timeout(mdiodev_c45_read, val,
				(val & PCS_BASER_UP), 1000, 100000, true,
				xpcs_mdiodev,
				MDIO_MMD_PCS, PCS_STATUS1);
	if (ret) {
		dev_err(&xpcs_mdiodev->dev, "BaseR link failed!\n");
		return ret;
	}

	/* Enable USXGMII mode */
	ret = mdiodev_c45_modify(xpcs_mdiodev, MDIO_MMD_PCS, DIG_CTRL1,
				 DIG_CTRL1_USXGMII_EN,
				 DIG_CTRL1_USXGMII_EN);
	if (ret)
		return ret;

	/* Configure QXGMII mode */
	ret = mdiodev_c45_modify(xpcs_mdiodev, MDIO_MMD_PCS, PCS_TPC,
				 PCS_QXGMII_MODE_MASK,
				 FIELD_PREP(PCS_QXGMII_MODE_MASK,
					    PCS_QXGMII_EN));
	if (ret)
		return ret;

	/* Configure AM interval */
	ret = mdiodev_c45_write(xpcs_mdiodev, MDIO_MMD_PCS, AM_LINK_TIMER,
				AM_LINK_TIMER_VAL);
	if (ret)
		return ret;

	/* Reset XPCS */
	ret = mdiodev_c45_modify(xpcs_mdiodev, MDIO_MMD_PCS, DIG_CTRL1,
				 DIG_CTRL1_XPCS_RESET,
				 DIG_CTRL1_XPCS_RESET);
	if (ret)
		return ret;

	/* Wait XPCS reset done */
	ret = read_poll_timeout(mdiodev_c45_read, val,
				!(val & DIG_CTRL1_XPCS_RESET),
				1000, 100000, true, xpcs_mdiodev,
				MDIO_MMD_PCS, DIG_CTRL1);
	if (ret) {
		dev_err(&xpcs_mdiodev->dev, "XPCS reset failed!\n");
		return ret;
	}

	/* Enable auto-negotiation complete interrupt, using mii-4bit
	 * and TX configureation of PHY side on all XPCS channels.
	 */
	for (i = 0; i < QCA8084_CHANNEL_MAX; i++) {
		ret = mdiodev_c45_modify(xpcs_mdiodev, qca8084_xpcs_ch_mmd[i],
					 DIG_CONTROL2,
					 (MII_BIT_CONTROL | TX_CONFIG |
					 AUTO_NEGOTIATION_CMPLT_INTR),
					 (TX_CONFIG | AUTO_NEGOTIATION_CMPLT_INTR));
		if (ret)
			return ret;

		/* Enable auto-negotiation capability */
		ret = mdiodev_c45_modify(xpcs_mdiodev, qca8084_xpcs_ch_mmd[i],
					 MII_CONTROL,
					 AUTO_NEGOTIATION_EN,
					 AUTO_NEGOTIATION_EN);
		if (ret)
			return ret;

		/* Disable TX IPG check */
		ret = mdiodev_c45_modify(xpcs_mdiodev, qca8084_xpcs_ch_mmd[i],
					 XAUI_CONTROL,
					 TX_IPG_CHECK_DISABLE,
					 TX_IPG_CHECK_DISABLE);
		if (ret)
			return ret;
	}

	/* Check EEE capability supported or not */
	ret = mdiodev_c45_read(xpcs_mdiodev, MDIO_MMD_PCS, PCS_EEE_CONTROL);
	if (ret < 0)
		return ret;

	if (ret & EEE_CAPABILITY) {
		ret = mdiodev_c45_modify(xpcs_mdiodev, MDIO_MMD_PCS,
					 EEE_MODE_CONTROL,
					 EEE_LCT_RES | EEE_SIGN,
					 FIELD_PREP(EEE_LCT_RES, 1) | EEE_SIGN);
		if (ret)
			return ret;

		ret = mdiodev_c45_modify(xpcs_mdiodev, MDIO_MMD_PCS,
					 EEE_RX_TIMER,
					 EEE_RX_TIMER_100US_RES | EEE_RX_TIMER_RWR_RES,
					 FIELD_PREP(EEE_RX_TIMER_100US_RES, 0xc8) |
					 FIELD_PREP(EEE_RX_TIMER_RWR_RES, 0x1c));
		if (ret)
			return ret;

		/* Enable EEE LPI */
		ret = mdiodev_c45_modify(xpcs_mdiodev, MDIO_MMD_PCS,
					 EEE_MODE_CONTROL1,
					 TRANS_LPI_MODE | TRANS_RX_LPI_MODE,
					 TRANS_LPI_MODE | TRANS_RX_LPI_MODE);
		if (ret)
			return ret;

		/* Enable TX/RX LPI pattern */
		ret = mdiodev_c45_modify(xpcs_mdiodev, MDIO_MMD_PCS,
					 EEE_MODE_CONTROL,
					 EEE_LRX_EN | EEE_LTX_EN,
					 EEE_LRX_EN | EEE_LTX_EN);
	}

	return ret;
}

static int qca8084_pcs_set_mode(struct mdio_device *xpcs_mdiodev,
				       struct mdio_device *pcs_mdiodev)
{
	struct qca8084_xpcs_data *xpcs_data = mdiodev_get_drvdata(xpcs_mdiodev);
	struct qca8084_pcs_data *pcs_data = mdiodev_get_drvdata(pcs_mdiodev);
	struct qca8084_xpcs_channel_priv xpcs_ch;
	int ret, channel;

	/* Enable clock and de-assert for PCS. */
	ret = clk_prepare_enable(pcs_data->clks[PCS_CLK]);
	if (ret)
		return ret;

	ret = reset_control_deassert(pcs_data->rstc);
	if (ret)
		return ret;

	/* IPG tunning selection for RX, TX and XGMII of all channels. */
	ret = mdiodev_c45_modify(pcs_mdiodev, MDIO_MMD_PMAPMD,
				 BYPASS_TUNNING_IPG,
				 BYPASS_TUNNING_IPG_MASK, 0);
	if (ret)
		return ret;

	reset_control_assert(xpcs_data->rstc);

	ret = qca8084_pcs_set_interface_mode(pcs_mdiodev,
					     PHY_INTERFACE_MODE_10G_QXGMII);
	if (ret)
		return ret;

	/* Reset of 4 channels */
	for (channel = 0; channel < QCA8084_CHANNEL_MAX; channel++) {
		xpcs_ch = xpcs_data->xpcs_ch[channel];
		ret = reset_control_reset(xpcs_ch.rstcs);
		if (ret)
			return ret;
	}

	ret = qca8084_do_calibration(pcs_mdiodev);
	if (ret) {
		dev_err(&pcs_mdiodev->dev, "PCS calibration timeout!\n");
		return ret;
	}

	/* Enable PCS SSC to fix mode */
	ret = mdiodev_c45_modify(pcs_mdiodev, MDIO_MMD_PMAPMD,
				 CDR_CONTRL, SSC_FIX_MODE, SSC_FIX_MODE);
	if (ret)
		return ret;

	return reset_control_deassert(xpcs_data->rstc);
}

static int qca8084_xpcs_clock_parent_set(struct mdio_device *xpcs_mdiodev,
					 struct mdio_device *pcs_mdiodev)
{
	struct qca8084_xpcs_data *xpcs_data = mdiodev_get_drvdata(xpcs_mdiodev);
	struct qca8084_pcs_data *pcs_data = mdiodev_get_drvdata(pcs_mdiodev);
	struct qca8084_xpcs_channel_priv xpcs_ch;
	int ret, channel;

	for (channel = 0; channel < QCA8084_CHANNEL_MAX; channel++) {
		xpcs_ch = xpcs_data->xpcs_ch[channel];
		ret = clk_set_parent(xpcs_ch.clks[XPCS_RX_SRC_CLK],
				     pcs_data->clks[PCS_RX_ROOT_CLK]);
		if (ret)
			return ret;

		ret = clk_set_parent(xpcs_ch.clks[XPCS_TX_SRC_CLK],
				     pcs_data->clks[PCS_TX_ROOT_CLK]);
		if (ret)
			return ret;
	}

	return 0;
}

int qca8084_qxgmii_set_mode(struct mdio_device *xpcs_mdiodev,
			    struct mdio_device *pcs_mdiodev)
{
	int ret;

	ret = qca8084_xpcs_clock_parent_set(xpcs_mdiodev, pcs_mdiodev);
	if (ret)
		return ret;

	ret = qca8084_pcs_set_mode(xpcs_mdiodev, pcs_mdiodev);
	if (ret)
		return ret;

	return qca8084_xpcs_set_mode(xpcs_mdiodev);
}

static int qca8084_pcs_ipg_tune_reset(struct mdio_device *mdio_dev,
				      int reset_function)
{
	int ret;

	ret = mdiodev_c45_modify(mdio_dev, MDIO_MMD_PMAPMD, QP_USXG_RESET,
				 reset_function, 0);
	if (ret)
		return ret;

	usleep_range(1000, 1100);

	return mdiodev_c45_modify(mdio_dev, MDIO_MMD_PMAPMD, QP_USXG_RESET,
				  reset_function, reset_function);
}

static int qca8084_xpcs_an_restart(struct mdio_device *xpcs_mdiodev,
				   int channel)
{
	int ret, mmd;

	mmd = qca8084_xpcs_ch_mmd[channel];

	/* Restart auto-negotiation */
	ret = mdiodev_c45_modify(xpcs_mdiodev, mmd, MII_CONTROL,
				 AUTO_NEGOTIATION_RESTART,
				 AUTO_NEGOTIATION_RESTART);
	if (ret)
		return ret;

	usleep_range(1000, 1100);

	/* Clear pcs auto-negotiation complete interrupt */
	return mdiodev_c45_modify(xpcs_mdiodev, mmd, PCS_ERR_SEL,
				  PCS_AN_COMPLETE, 0);
}

void qca8084_qxgmii_set_speed(struct mdio_device *xpcs_mdiodev,
			      struct mdio_device *pcs_mdiodev,
			      int channel, int speed)
{
	struct qca8084_xpcs_data *xpcs_data = mdiodev_get_drvdata(xpcs_mdiodev);
	struct qca8084_xpcs_channel_priv *xpcs_ch;
	int mmd, i, ret, xpcs_rate;
	unsigned long rate;

	for (i = 0; i < QCA8084_CHANNEL_MAX; i++) {
		xpcs_ch = &(xpcs_data->xpcs_ch[channel]);
		if (channel == xpcs_ch->ch_id)
			break;
	}

	if (i == QCA8084_CHANNEL_MAX) {
		dev_err(&xpcs_mdiodev->dev, "Invalid channel %d\n", channel);
		return;
	}

	mmd = qca8084_xpcs_ch_mmd[channel];

	ret = qca8084_xpcs_an_restart(xpcs_mdiodev, channel);
	if (ret)
		return;

	switch (speed) {
	case SPEED_2500:
		rate = 312500000;
		xpcs_rate = PCS_SPEED_2500;
		break;
	case SPEED_1000:
		rate = 125000000;
		xpcs_rate = PCS_SPEED_1000;
		break;
	case SPEED_100:
		rate = 25000000;
		xpcs_rate = PCS_SPEED_100;
		break;
	case SPEED_10:
	default:
		rate = 2500000;
		xpcs_rate = PCS_SPEED_10;
		break;
	}

	clk_set_rate(xpcs_ch->clks[XPCS_RX_CLK], rate);
	clk_set_rate(xpcs_ch->clks[XPCS_TX_CLK], rate);

	/* XGMII takes the different clock rate 78.125Mhz from XPCS clock
	 * when linked at 2500M.
	 */
	if (speed == SPEED_2500)
		rate = 78125000;

	clk_set_rate(xpcs_ch->clks[XPCS_XGMII_RX_CLK], rate);
	clk_set_rate(xpcs_ch->clks[XPCS_XGMII_TX_CLK], rate);

	ret = mdiodev_c45_modify(xpcs_mdiodev, mmd, MII_CONTROL,
				 PCS_SPEED_2500 | PCS_SPEED_1000 |
				 PCS_SPEED_100 | PCS_SPEED_10,
				 xpcs_rate);
	if (ret)
		return;

	/* Disable clocks if link down with unknown speed. The channel clocks
	 * are disabled by default, __clk_is_enabled() is used to avoid
	 * disabling the clocks that is already in the disabled status.
	 */
	if (speed == SPEED_UNKNOWN) {
		if (__clk_is_enabled(xpcs_ch->clks[XPCS_RX_CLK]))
			clk_disable_unprepare(xpcs_ch->clks[XPCS_RX_CLK]);
		if (__clk_is_enabled(xpcs_ch->clks[XPCS_TX_CLK]))
			clk_disable_unprepare(xpcs_ch->clks[XPCS_TX_CLK]);
		if (__clk_is_enabled(xpcs_ch->clks[XPCS_PORT_RX_CLK]))
			clk_disable_unprepare(xpcs_ch->clks[XPCS_PORT_RX_CLK]);
		if (__clk_is_enabled(xpcs_ch->clks[XPCS_PORT_TX_CLK]))
			clk_disable_unprepare(xpcs_ch->clks[XPCS_PORT_TX_CLK]);
		if (__clk_is_enabled(xpcs_ch->clks[XPCS_XGMII_RX_CLK]))
			clk_disable_unprepare(xpcs_ch->clks[XPCS_XGMII_RX_CLK]);
		if (__clk_is_enabled(xpcs_ch->clks[XPCS_XGMII_TX_CLK]))
			clk_disable_unprepare(xpcs_ch->clks[XPCS_XGMII_TX_CLK]);
	} else {
		clk_prepare_enable(xpcs_ch->clks[XPCS_RX_CLK]);
		clk_prepare_enable(xpcs_ch->clks[XPCS_TX_CLK]);
		clk_prepare_enable(xpcs_ch->clks[XPCS_PORT_RX_CLK]);
		clk_prepare_enable(xpcs_ch->clks[XPCS_PORT_TX_CLK]);
		clk_prepare_enable(xpcs_ch->clks[XPCS_XGMII_RX_CLK]);
		clk_prepare_enable(xpcs_ch->clks[XPCS_XGMII_TX_CLK]);
	}

	msleep(100);

	ret = reset_control_reset(xpcs_ch->rstcs);
	if (ret)
		return;

	/* Reset IPG tune of PCS device. */
	ret = qca8084_pcs_ipg_tune_reset(pcs_mdiodev, BIT(channel));
	if (ret)
		return;

	if (channel == 0)
		mdiodev_c45_modify(xpcs_mdiodev, MDIO_MMD_PCS, DIG_CTRL1,
				   FIFO_RESET_CH0, FIFO_RESET_CH0);
	else
		mdiodev_c45_modify(xpcs_mdiodev, mmd, DIG_CTRL1,
				   FIFO_RESET_CH1_CH2_CH3,
				   FIFO_RESET_CH1_CH2_CH3);
}
