// SPDX-License-Identifier: GPL-2.0
// Copyright (c) 2018-2019 MediaTek Inc.
/* A library and platform driver for the MediaTek LynxI SGMII circuit
 *
 * Author: Sean Wang <sean.wang@mediatek.com>
 * Author: Alexander Couzens <lynxis@fe80.eu>
 * Author: Daniel Golle <daniel@makrotopia.org>
 *
 */

#include <linux/clk.h>
#include <linux/mdio.h>
#include <linux/mfd/syscon.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/pcs/pcs-mtk-lynxi.h>
#include <linux/phylink.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/reset.h>

/* SGMII subsystem config registers */
/* BMCR (low 16) BMSR (high 16) */
#define SGMSYS_PCS_CONTROL_1		0x0
#define SGMII_BMCR			GENMASK(15, 0)
#define SGMII_BMSR			GENMASK(31, 16)

#define SGMSYS_PCS_DEVICE_ID		0x4
#define SGMII_LYNXI_DEV_ID		0x4d544950

#define SGMSYS_PCS_ADVERTISE		0x8
#define SGMII_ADVERTISE			GENMASK(15, 0)
#define SGMII_LPA			GENMASK(31, 16)

#define SGMSYS_PCS_SCRATCH		0x14
#define SGMII_DEV_VERSION		GENMASK(31, 16)

/* Register to programmable link timer, the unit in 2 * 8ns */
#define SGMSYS_PCS_LINK_TIMER		0x18
#define SGMII_LINK_TIMER_MASK		GENMASK(19, 0)
#define SGMII_LINK_TIMER_VAL(ns)	FIELD_PREP(SGMII_LINK_TIMER_MASK, \
						   ((ns) / 2 / 8))

/* Register to control remote fault */
#define SGMSYS_SGMII_MODE		0x20
#define SGMII_IF_MODE_SGMII		BIT(0)
#define SGMII_SPEED_DUPLEX_AN		BIT(1)
#define SGMII_SPEED_MASK		GENMASK(3, 2)
#define SGMII_SPEED_10			FIELD_PREP(SGMII_SPEED_MASK, 0)
#define SGMII_SPEED_100			FIELD_PREP(SGMII_SPEED_MASK, 1)
#define SGMII_SPEED_1000		FIELD_PREP(SGMII_SPEED_MASK, 2)
#define SGMII_DUPLEX_HALF		BIT(4)
#define SGMII_REMOTE_FAULT_DIS		BIT(8)

/* Register to reset SGMII design */
#define SGMSYS_RESERVED_0		0x34
#define SGMII_SW_RESET			BIT(0)

/* Register to set SGMII speed, ANA RG_ Control Signals III */
#define SGMII_PHY_SPEED_MASK		GENMASK(3, 2)
#define SGMII_PHY_SPEED_1_25G		FIELD_PREP(SGMII_PHY_SPEED_MASK, 0)
#define SGMII_PHY_SPEED_3_125G		FIELD_PREP(SGMII_PHY_SPEED_MASK, 1)

/* Register to power up QPHY */
#define SGMSYS_QPHY_PWR_STATE_CTRL	0xe8
#define	SGMII_PHYA_PWD			BIT(4)

/* Register to QPHY wrapper control */
#define SGMSYS_QPHY_WRAP_CTRL		0xec
#define SGMII_PN_SWAP_MASK		GENMASK(1, 0)
#define SGMII_PN_SWAP_TX_RX		(BIT(0) | BIT(1))

#define MTK_NETSYS_V3_AMA_RGC3		0x128

/* struct mtk_pcs_lynxi -  This structure holds each sgmii regmap andassociated
 *                         data
 * @regmap:                The register map pointing at the range used to setup
 *                         SGMII modes
 * @dev:                   Pointer to device owning the PCS
 * @ana_rgc3:              The offset of register ANA_RGC3 relative to regmap
 * @interface:             Currently configured interface mode
 * @pcs:                   Phylink PCS structure
 * @flags:                 Flags indicating hardware properties
 * @rstc:                  Reset controller
 * @sgmii_sel:             SGMII Register Clock
 * @sgmii_rx:              SGMII RX Clock
 * @sgmii_tx:              SGMII TX Clock
 * @node:                  List node
 */
struct mtk_pcs_lynxi {
	struct regmap		*regmap;
	struct device		*dev;
	u32			ana_rgc3;
	phy_interface_t		interface;
	struct			phylink_pcs pcs;
	u32			flags;
	struct reset_control	*rstc;
	struct clk		*sgmii_sel;
	struct clk		*sgmii_rx;
	struct clk		*sgmii_tx;
	struct list_head	node;
};

static LIST_HEAD(mtk_pcs_lynxi_instances);
static DEFINE_MUTEX(instance_mutex);

static struct mtk_pcs_lynxi *pcs_to_mtk_pcs_lynxi(struct phylink_pcs *pcs)
{
	return container_of(pcs, struct mtk_pcs_lynxi, pcs);
}

static void mtk_pcs_lynxi_get_state(struct phylink_pcs *pcs,
				    struct phylink_link_state *state)
{
	struct mtk_pcs_lynxi *mpcs = pcs_to_mtk_pcs_lynxi(pcs);
	unsigned int bm, bmsr, adv;

	/* Read the BMSR and LPA */
	regmap_read(mpcs->regmap, SGMSYS_PCS_CONTROL_1, &bm);
	bmsr = FIELD_GET(SGMII_BMSR, bm);

	if (state->interface == PHY_INTERFACE_MODE_2500BASEX) {
		state->link = !!(bmsr & BMSR_LSTATUS);
		state->an_complete = !!(bmsr & BMSR_ANEGCOMPLETE);
		state->speed = SPEED_2500;
		state->duplex = DUPLEX_FULL;

		return;
	}

	regmap_read(mpcs->regmap, SGMSYS_PCS_ADVERTISE, &adv);
	phylink_mii_c22_pcs_decode_state(state, bmsr, FIELD_GET(SGMII_LPA, adv));
}

static void mtk_sgmii_reset(struct mtk_pcs_lynxi *mpcs)
{
	if (!mpcs->rstc)
		return;

	reset_control_assert(mpcs->rstc);
	udelay(100);
	reset_control_deassert(mpcs->rstc);
	mdelay(1);
}

static int mtk_pcs_lynxi_config(struct phylink_pcs *pcs, unsigned int neg_mode,
				phy_interface_t interface,
				const unsigned long *advertising,
				bool permit_pause_to_mac)
{
	struct mtk_pcs_lynxi *mpcs = pcs_to_mtk_pcs_lynxi(pcs);
	bool mode_changed = false, changed;
	unsigned int rgc3, sgm_mode, bmcr = 0;
	int advertise, link_timer;

	advertise = phylink_mii_c22_pcs_encode_advertisement(interface,
							     advertising);
	if (advertise < 0)
		return advertise;

	/* Clearing IF_MODE_BIT0 switches the PCS to BASE-X mode, and
	 * we assume that fixes it's speed at bitrate = line rate (in
	 * other words, 1000Mbps or 2500Mbps).
	 */
	if (interface == PHY_INTERFACE_MODE_SGMII)
		sgm_mode = SGMII_IF_MODE_SGMII;
	else
		sgm_mode = 0;

	if (neg_mode & PHYLINK_PCS_NEG_INBAND)
		sgm_mode |= SGMII_REMOTE_FAULT_DIS;

	if (neg_mode == PHYLINK_PCS_NEG_INBAND_ENABLED) {
		if (interface == PHY_INTERFACE_MODE_SGMII)
			sgm_mode |= SGMII_SPEED_DUPLEX_AN;
		if (interface != PHY_INTERFACE_MODE_2500BASEX)
			bmcr = BMCR_ANENABLE;
	}

	if (mpcs->interface != interface) {
		link_timer = phylink_get_link_timer_ns(interface);
		if (link_timer < 0)
			return link_timer;

		/* PHYA power down */
		regmap_set_bits(mpcs->regmap, SGMSYS_QPHY_PWR_STATE_CTRL,
				SGMII_PHYA_PWD);

		/* Reset SGMII PCS state */
		mtk_sgmii_reset(mpcs);
		regmap_set_bits(mpcs->regmap, SGMSYS_RESERVED_0,
				SGMII_SW_RESET);

		if (mpcs->flags & MTK_SGMII_FLAG_PN_SWAP)
			regmap_update_bits(mpcs->regmap, SGMSYS_QPHY_WRAP_CTRL,
					   SGMII_PN_SWAP_MASK,
					   SGMII_PN_SWAP_TX_RX);

		if (interface == PHY_INTERFACE_MODE_2500BASEX)
			rgc3 = SGMII_PHY_SPEED_3_125G;
		else
			rgc3 = SGMII_PHY_SPEED_1_25G;

		/* Configure the underlying interface speed */
		regmap_update_bits(mpcs->regmap, mpcs->ana_rgc3,
				   SGMII_PHY_SPEED_MASK, rgc3);

		/* Setup the link timer */
		regmap_write(mpcs->regmap, SGMSYS_PCS_LINK_TIMER,
			     SGMII_LINK_TIMER_VAL(link_timer));

		mpcs->interface = interface;
		mode_changed = true;
	}

	/* Update the advertisement, noting whether it has changed */
	regmap_update_bits_check(mpcs->regmap, SGMSYS_PCS_ADVERTISE,
				 SGMII_ADVERTISE, advertise, &changed);

	/* Update the sgmsys mode register */
	regmap_update_bits(mpcs->regmap, SGMSYS_SGMII_MODE,
			   SGMII_REMOTE_FAULT_DIS | SGMII_SPEED_DUPLEX_AN |
			   SGMII_IF_MODE_SGMII, sgm_mode);

	/* Update the BMCR */
	regmap_update_bits(mpcs->regmap, SGMSYS_PCS_CONTROL_1,
			   BMCR_ANENABLE, bmcr);

	/* Release PHYA power down state
	 * Only removing bit SGMII_PHYA_PWD isn't enough.
	 * There are cases when the SGMII_PHYA_PWD register contains 0x9 which
	 * prevents SGMII from working. The SGMII still shows link but no traffic
	 * can flow. Writing 0x0 to the PHYA_PWD register fix the issue. 0x0 was
	 * taken from a good working state of the SGMII interface.
	 * Unknown how much the QPHY needs but it is racy without a sleep.
	 * Tested on mt7622 & mt7986.
	 */
	usleep_range(50, 100);
	regmap_write(mpcs->regmap, SGMSYS_QPHY_PWR_STATE_CTRL, 0);

	return changed || mode_changed;
}

static void mtk_pcs_lynxi_restart_an(struct phylink_pcs *pcs)
{
	struct mtk_pcs_lynxi *mpcs = pcs_to_mtk_pcs_lynxi(pcs);

	regmap_set_bits(mpcs->regmap, SGMSYS_PCS_CONTROL_1, BMCR_ANRESTART);
}

static void mtk_pcs_lynxi_link_up(struct phylink_pcs *pcs,
				  unsigned int neg_mode,
				  phy_interface_t interface, int speed,
				  int duplex)
{
	struct mtk_pcs_lynxi *mpcs = pcs_to_mtk_pcs_lynxi(pcs);
	unsigned int sgm_mode;

	if (neg_mode != PHYLINK_PCS_NEG_INBAND_ENABLED) {
		/* Force the speed and duplex setting */
		if (speed == SPEED_10)
			sgm_mode = SGMII_SPEED_10;
		else if (speed == SPEED_100)
			sgm_mode = SGMII_SPEED_100;
		else
			sgm_mode = SGMII_SPEED_1000;

		if (duplex != DUPLEX_FULL)
			sgm_mode |= SGMII_DUPLEX_HALF;

		regmap_update_bits(mpcs->regmap, SGMSYS_SGMII_MODE,
				   SGMII_DUPLEX_HALF | SGMII_SPEED_MASK,
				   sgm_mode);
	}
}

static int mtk_pcs_lynxi_enable(struct phylink_pcs *pcs)
{
	struct mtk_pcs_lynxi *mpcs = pcs_to_mtk_pcs_lynxi(pcs);

	if (mpcs->sgmii_tx && mpcs->sgmii_rx) {
		clk_prepare_enable(mpcs->sgmii_rx);
		clk_prepare_enable(mpcs->sgmii_tx);
	}

	return 0;
}

static void mtk_pcs_lynxi_disable(struct phylink_pcs *pcs)
{
	struct mtk_pcs_lynxi *mpcs = pcs_to_mtk_pcs_lynxi(pcs);

	regmap_set_bits(mpcs->regmap, SGMSYS_QPHY_PWR_STATE_CTRL, SGMII_PHYA_PWD);

	if (mpcs->sgmii_tx && mpcs->sgmii_rx) {
		clk_disable_unprepare(mpcs->sgmii_tx);
		clk_disable_unprepare(mpcs->sgmii_rx);
	}

	mpcs->interface = PHY_INTERFACE_MODE_NA;
}

static const struct phylink_pcs_ops mtk_pcs_lynxi_ops = {
	.pcs_get_state = mtk_pcs_lynxi_get_state,
	.pcs_config = mtk_pcs_lynxi_config,
	.pcs_an_restart = mtk_pcs_lynxi_restart_an,
	.pcs_link_up = mtk_pcs_lynxi_link_up,
	.pcs_disable = mtk_pcs_lynxi_disable,
	.pcs_enable = mtk_pcs_lynxi_enable,
};

static struct phylink_pcs *mtk_pcs_lynxi_init(struct device *dev, struct regmap *regmap,
					      u32 ana_rgc3, u32 flags,
					      struct mtk_pcs_lynxi *prealloc)
{
	struct mtk_pcs_lynxi *mpcs;
	u32 id, ver;
	int ret;

	ret = regmap_read(regmap, SGMSYS_PCS_DEVICE_ID, &id);
	if (ret < 0)
		return ERR_PTR(ret);

	if (id != SGMII_LYNXI_DEV_ID) {
		dev_err(dev, "unknown PCS device id %08x\n", id);
		return ERR_PTR(-ENODEV);
	}

	ret = regmap_read(regmap, SGMSYS_PCS_SCRATCH, &ver);
	if (ret < 0)
		return ERR_PTR(ret);

	ver = FIELD_GET(SGMII_DEV_VERSION, ver);
	if (ver != 0x1) {
		dev_err(dev, "unknown PCS device version %04x\n", ver);
		return ERR_PTR(-ENODEV);
	}

	dev_dbg(dev, "MediaTek LynxI SGMII PCS (id 0x%08x, ver 0x%04x)\n", id,
		ver);

	if (prealloc) {
		mpcs = prealloc;
	} else {
		mpcs = kzalloc(sizeof(*mpcs), GFP_KERNEL);
		if (!mpcs)
			return ERR_PTR(-ENOMEM);
	};

	mpcs->ana_rgc3 = ana_rgc3;
	mpcs->regmap = regmap;
	mpcs->flags = flags;
	mpcs->pcs.ops = &mtk_pcs_lynxi_ops;
	mpcs->pcs.neg_mode = true;
	mpcs->pcs.poll = true;
	mpcs->interface = PHY_INTERFACE_MODE_NA;

	return &mpcs->pcs;
};

struct phylink_pcs *mtk_pcs_lynxi_create(struct device *dev,
					 struct regmap *regmap, u32 ana_rgc3,
					 u32 flags)
{
	return mtk_pcs_lynxi_init(dev, regmap, ana_rgc3, flags, NULL);
}
EXPORT_SYMBOL(mtk_pcs_lynxi_create);

void mtk_pcs_lynxi_destroy(struct phylink_pcs *pcs)
{
	if (!pcs)
		return;

	kfree(pcs_to_mtk_pcs_lynxi(pcs));
}
EXPORT_SYMBOL(mtk_pcs_lynxi_destroy);

static int mtk_pcs_lynxi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct mtk_pcs_lynxi *mpcs;
	struct phylink_pcs *pcs;
	struct regmap *regmap;
	u32 flags = 0;

	mpcs = devm_kzalloc(dev, sizeof(*mpcs), GFP_KERNEL);
	if (!mpcs)
		return -ENOMEM;

	mpcs->dev = dev;
	regmap = syscon_node_to_regmap(np->parent);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	if (of_property_read_bool(np->parent, "mediatek,pnswap"))
		flags |= MTK_SGMII_FLAG_PN_SWAP;

	mpcs->rstc = of_reset_control_get_shared(np->parent, NULL);
	if (IS_ERR(mpcs->rstc))
		return PTR_ERR(mpcs->rstc);

	reset_control_deassert(mpcs->rstc);
	mpcs->sgmii_sel = devm_clk_get_enabled(dev, "sgmii_sel");
	if (IS_ERR(mpcs->sgmii_sel))
		return PTR_ERR(mpcs->sgmii_sel);

	mpcs->sgmii_rx = devm_clk_get(dev, "sgmii_rx");
	if (IS_ERR(mpcs->sgmii_rx))
		return PTR_ERR(mpcs->sgmii_rx);

	mpcs->sgmii_tx = devm_clk_get(dev, "sgmii_tx");
	if (IS_ERR(mpcs->sgmii_tx))
		return PTR_ERR(mpcs->sgmii_tx);

	pcs = mtk_pcs_lynxi_init(dev, regmap, (uintptr_t)of_device_get_match_data(dev),
				 flags, mpcs);
	if (IS_ERR(pcs))
		return PTR_ERR(pcs);

	regmap_set_bits(mpcs->regmap, SGMSYS_QPHY_PWR_STATE_CTRL, SGMII_PHYA_PWD);

	platform_set_drvdata(pdev, mpcs);

	mutex_lock(&instance_mutex);
	list_add_tail(&mpcs->node, &mtk_pcs_lynxi_instances);
	mutex_unlock(&instance_mutex);

	return 0;
}

static int mtk_pcs_lynxi_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mtk_pcs_lynxi *cur, *tmp;

	mutex_lock(&instance_mutex);
	list_for_each_entry_safe(cur, tmp, &mtk_pcs_lynxi_instances, node)
		if (cur->dev == dev) {
			list_del(&cur->node);
			kfree(cur);
			break;
		}
	mutex_unlock(&instance_mutex);

	return 0;
}

static const struct of_device_id mtk_pcs_lynxi_of_match[] = {
	{ .compatible = "mediatek,mt7988-sgmii", .data = (void *)MTK_NETSYS_V3_AMA_RGC3 },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, mtk_pcs_lynxi_of_match);

struct phylink_pcs *mtk_pcs_lynxi_get(struct device *dev, struct device_node *np)
{
	struct platform_device *pdev;
	struct mtk_pcs_lynxi *mpcs;

	if (!np)
		return NULL;

	if (!of_device_is_available(np))
		return ERR_PTR(-ENODEV);

	if (!of_match_node(mtk_pcs_lynxi_of_match, np))
		return ERR_PTR(-EINVAL);

	pdev = of_find_device_by_node(np);
	if (!pdev || !platform_get_drvdata(pdev)) {
		if (pdev)
			put_device(&pdev->dev);
		return ERR_PTR(-EPROBE_DEFER);
	}

	mpcs = platform_get_drvdata(pdev);
	device_link_add(dev, mpcs->dev, DL_FLAG_AUTOREMOVE_CONSUMER);

	return &mpcs->pcs;
}
EXPORT_SYMBOL(mtk_pcs_lynxi_get);

void mtk_pcs_lynxi_put(struct phylink_pcs *pcs)
{
	struct mtk_pcs_lynxi *cur, *mpcs = NULL;

	if (!pcs)
		return;

	mutex_lock(&instance_mutex);
	list_for_each_entry(cur, &mtk_pcs_lynxi_instances, node)
		if (pcs == &cur->pcs) {
			mpcs = cur;
			break;
		}
	mutex_unlock(&instance_mutex);

	if (WARN_ON(!mpcs))
		return;

	put_device(mpcs->dev);
}
EXPORT_SYMBOL(mtk_pcs_lynxi_put);

static struct platform_driver mtk_pcs_lynxi_driver = {
	.driver = {
		.name			= "mtk-pcs-lynxi",
		.suppress_bind_attrs	= true,
		.of_match_table		= mtk_pcs_lynxi_of_match,
	},
	.probe = mtk_pcs_lynxi_probe,
	.remove = mtk_pcs_lynxi_remove,
};
module_platform_driver(mtk_pcs_lynxi_driver);

MODULE_AUTHOR("Daniel Golle <daniel@makrotopia.org>");
MODULE_DESCRIPTION("MediaTek LynxI HSGMII PCS");
MODULE_LICENSE("GPL");
