// SPDX-License-Identifier: GPL-2.0+

#include <dt-bindings/net/qcom,qca808x.h>
#include <linux/phy.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/reset.h>
#include <linux/clk.h>

#include "../phylib.h"
#include "qca8084_serdes.h"
#include "qcom.h"

/* ADC threshold */
#define QCA808X_PHY_DEBUG_ADC_THRESHOLD		0x2c80
#define QCA808X_ADC_THRESHOLD_MASK		GENMASK(7, 0)
#define QCA808X_ADC_THRESHOLD_80MV		0
#define QCA808X_ADC_THRESHOLD_100MV		0xf0
#define QCA808X_ADC_THRESHOLD_200MV		0x0f
#define QCA808X_ADC_THRESHOLD_300MV		0xff

/* CLD control */
#define QCA808X_PHY_MMD3_ADDR_CLD_CTRL7		0x8007
#define QCA808X_8023AZ_AFE_CTRL_MASK		GENMASK(8, 4)
#define QCA808X_8023AZ_AFE_EN			0x90

/* AZ control */
#define QCA808X_PHY_MMD3_AZ_TRAINING_CTRL	0x8008
#define QCA808X_MMD3_AZ_TRAINING_VAL		0x1c32

#define QCA808X_PHY_MMD1_MSE_THRESHOLD_20DB	0x8014
#define QCA808X_MSE_THRESHOLD_20DB_VALUE	0x529

#define QCA808X_PHY_MMD1_MSE_THRESHOLD_17DB	0x800E
#define QCA808X_MSE_THRESHOLD_17DB_VALUE	0x341

#define QCA808X_PHY_MMD1_MSE_THRESHOLD_27DB	0x801E
#define QCA808X_MSE_THRESHOLD_27DB_VALUE	0x419

#define QCA808X_PHY_MMD1_MSE_THRESHOLD_28DB	0x8020
#define QCA808X_MSE_THRESHOLD_28DB_VALUE	0x341

#define QCA808X_PHY_MMD7_TOP_OPTION1		0x901c
#define QCA808X_TOP_OPTION1_DATA		0x0

#define QCA808X_PHY_MMD3_DEBUG_1		0xa100
#define QCA808X_MMD3_DEBUG_1_VALUE		0x9203
#define QCA808X_PHY_MMD3_DEBUG_2		0xa101
#define QCA808X_MMD3_DEBUG_2_VALUE		0x48ad
#define QCA808X_PHY_MMD3_DEBUG_3		0xa103
#define QCA808X_MMD3_DEBUG_3_VALUE		0x1698
#define QCA808X_PHY_MMD3_DEBUG_4		0xa105
#define QCA808X_MMD3_DEBUG_4_VALUE		0x8001
#define QCA808X_PHY_MMD3_DEBUG_5		0xa106
#define QCA808X_MMD3_DEBUG_5_VALUE		0x1111
#define QCA808X_PHY_MMD3_DEBUG_6		0xa011
#define QCA808X_MMD3_DEBUG_6_VALUE		0x5f85

/* master/slave seed config */
#define QCA808X_PHY_DEBUG_LOCAL_SEED		9
#define QCA808X_MASTER_SLAVE_SEED_ENABLE	BIT(1)
#define QCA808X_MASTER_SLAVE_SEED_CFG		GENMASK(12, 2)
#define QCA808X_MASTER_SLAVE_SEED_RANGE		0x32

/* Hibernation yields lower power consumpiton in contrast with normal operation mode.
 * when the copper cable is unplugged, the PHY enters into hibernation mode in about 10s.
 */
#define QCA808X_DBG_AN_TEST			0xb
#define QCA808X_HIBERNATION_EN			BIT(15)

#define QCA808X_MMD7_LED2_CTRL			0x8074
#define QCA808X_MMD7_LED2_FORCE_CTRL		0x8075
#define QCA808X_MMD7_LED1_CTRL			0x8076
#define QCA808X_MMD7_LED1_FORCE_CTRL		0x8077
#define QCA808X_MMD7_LED0_CTRL			0x8078
#define QCA808X_MMD7_LED_CTRL(x)		(0x8078 - ((x) * 2))

#define QCA808X_MMD7_LED0_FORCE_CTRL		0x8079
#define QCA808X_MMD7_LED_FORCE_CTRL(x)		(0x8079 - ((x) * 2))

#define QCA808X_MMD7_LED_POLARITY_CTRL		0x901a
/* QSDK sets by default 0x46 to this reg that sets BIT 6 for
 * LED to active high. It's not clear what BIT 3 and BIT 4 does.
 */
#define QCA808X_LED_ACTIVE_HIGH			BIT(6)

/* QCA808X 1G chip type */
#define QCA808X_PHY_MMD7_CHIP_TYPE		0x901d
#define QCA808X_PHY_CHIP_TYPE_1G		BIT(0)

#define QCA8081_PHY_SERDES_MMD1_FIFO_CTRL	0x9072
#define QCA8081_PHY_FIFO_RSTN			BIT(11)

#define QCA8081_PHY_ID				0x004dd101
#define QCA8084_PHY_ID				0x004dd180

#define QCA8084_MMD3_CDT_PULSE_CTRL		0x8075
#define QCA8084_CDT_PULSE_THRESH_VAL		0xa060

#define QCA8084_MMD3_CDT_NEAR_CTRL		0x807f
#define QCA8084_CDT_NEAR_BYPASS			BIT(15)

/* QCA8084 ADC clock edge */
#define QCA8084_ADC_CLK_SEL			0x8b80
#define QCA8084_ADC_CLK_SEL_ACLK		GENMASK(7, 4)
#define QCA8084_ADC_CLK_SEL_ACLK_FALL		0xf
#define QCA8084_ADC_CLK_SEL_ACLK_RISE		0x0

#define QCA8084_MSE_THRESHOLD			0x800a
#define QCA8084_MSE_THRESHOLD_2P5G_VAL		0x51c6

/* QCA8084 FIFO reset control */
#define QCA8084_FIFO_CONTROL			0x19
#define QCA8084_FIFO_MAC_2_PHY			BIT(1)
#define QCA8084_FIFO_PHY_2_MAC			BIT(0)

#define QCA8084_MMD7_IPG_OP			0x901d
#define QCA8084_IPG_10_TO_11_EN			BIT(0)

/* QCA8084 includes secure control module, which supports customizing the
 * MDIO address of PHY device and PCS device and configuring package mode
 * for the interface mode of PCS. The register of secure control is accessed
 * by MDIO bus with the special MDIO sequences, where the 32 bits register
 * address is split into 3 MDIO operations with 16 bits address.
 */
#define QCA8084_HIGH_ADDR_PREFIX		0x18
#define QCA8084_LOW_ADDR_PREFIX			0x10

/* Bottom two bits of REG must be zero */
#define QCA8084_MII_REG_MASK			GENMASK(4, 0)
#define QCA8084_MII_PHY_ADDR_MASK		GENMASK(7, 5)
#define QCA8084_MII_PAGE_MASK			GENMASK(23, 8)
#define QCA8084_MII_SW_ADDR_MASK		GENMASK(31, 24)
#define QCA8084_MII_REG_DATA_UPPER_16_BITS	BIT(1)

/* QCA8084 integrates 4 PHYs, PCS0 and PCS1(includes PCS and XPCS). */
#define QCA8084_MDIO_DEVICE_NUM			7

#define QCA8084_PCS_CFG				0xc90f014
#define QCA8084_PCS_ADDR0_MASK			GENMASK(4, 0)
#define QCA8084_PCS_ADDR1_MASK			GENMASK(9, 5)
#define QCA8084_PCS_ADDR2_MASK			GENMASK(14, 10)

#define QCA8084_EPHY_CFG			0xc90f018
#define QCA8084_EPHY_ADDR0_MASK			GENMASK(4, 0)
#define QCA8084_EPHY_ADDR1_MASK			GENMASK(9, 5)
#define QCA8084_EPHY_ADDR2_MASK			GENMASK(14, 10)
#define QCA8084_EPHY_ADDR3_MASK			GENMASK(19, 15)
#define QCA8084_EPHY_LDO_EN			GENMASK(21, 20)

#define QCA8084_WORK_MODE_CFG			0xc90f030
#define QCA8084_WORK_MODE_MASK			GENMASK(5, 0)
#define QCA8084_WORK_MODE_QXGMII		(BIT(5) | GENMASK(3, 0))
#define QCA8084_WORK_MODE_SWITCH		BIT(4)
#define QCA8084_WORK_MODE_SWITCH_PORT4_SGMII	BIT(5)

MODULE_DESCRIPTION("Qualcomm Atheros QCA808X PHY driver");
MODULE_AUTHOR("Matus Ujhelyi, Luo Jie");
MODULE_LICENSE("GPL");

enum {
	APB_BRIDGE_CLK,
	AHB_CLK,
	SEC_CTRL_AHB_CLK,
	TLMM_CLK,
	TLMM_AHB_CLK,
	CNOC_AHB_CLK,
	MDIO_AHB_CLK,
	MDIO_MASTER_AHB_CLK,
	SWITCH_CORE_CLK,
	PACKAGE_CLK_MAX
};

struct qca808x_priv {
	int led_polarity_mode;
	int channel_id;
};

struct qca808x_shared_priv {
	int package_mode;
	struct clk *clk[PACKAGE_CLK_MAX];
	struct mdio_device *mdiodev[2];	/* PCS and XPCS mdio device */
};

static const char *const qca8084_package_clk_name[PACKAGE_CLK_MAX] = {
	[APB_BRIDGE_CLK] =	"apb_bridge",
	[AHB_CLK] =		"ahb",
	[SEC_CTRL_AHB_CLK] =	"sec_ctrl_ahb",
	[TLMM_CLK] =		"tlmm",
	[TLMM_AHB_CLK] =	"tlmm_ahb",
	[CNOC_AHB_CLK] =	"cnoc_ahb",
	[MDIO_AHB_CLK] =	"mdio_ahb",
	[MDIO_MASTER_AHB_CLK] =	"mdio_master_ahb",
	[SWITCH_CORE_CLK] =	"switch_core",
};

static int __qca8084_set_page(struct mii_bus *bus, u16 sw_addr, u16 page)
{
	return __mdiobus_write(bus, QCA8084_HIGH_ADDR_PREFIX | (sw_addr >> 5),
			       sw_addr & 0x1f, page);
}

static int __qca8084_mii_read(struct mii_bus *bus, u16 addr, u16 reg, u32 *val)
{
	int ret, data;

	ret = __mdiobus_read(bus, addr, reg);
	if (ret < 0)
		return ret;

	data = ret;
	ret = __mdiobus_read(bus, addr,
			     reg | QCA8084_MII_REG_DATA_UPPER_16_BITS);
	if (ret < 0)
		return ret;

	*val =  data | ret << 16;

	return 0;
}

static int __qca8084_mii_write(struct mii_bus *bus, u16 addr, u16 reg, u32 val)
{
	int ret;

	ret = __mdiobus_write(bus, addr, reg, lower_16_bits(val));
	if (!ret)
		ret = __mdiobus_write(bus, addr,
				      reg | QCA8084_MII_REG_DATA_UPPER_16_BITS,
				      upper_16_bits(val));

	return ret;
}

static int qca8084_mii_modify(struct phy_device *phydev, u32 regaddr,
			      u32 clear, u32 set)
{
	u16 reg, addr, page, sw_addr;
	struct mii_bus *bus;
	u32 val;
	int ret;

	bus = phydev->mdio.bus;
	mutex_lock(&bus->mdio_lock);

	reg = FIELD_GET(QCA8084_MII_REG_MASK, regaddr);
	addr = FIELD_GET(QCA8084_MII_PHY_ADDR_MASK, regaddr);
	page = FIELD_GET(QCA8084_MII_PAGE_MASK, regaddr);
	sw_addr = FIELD_GET(QCA8084_MII_SW_ADDR_MASK, regaddr);

	ret = __qca8084_set_page(bus, sw_addr, page);
	if (ret < 0)
		goto qca8084_mii_modify_exit;

	ret = __qca8084_mii_read(bus, QCA8084_LOW_ADDR_PREFIX | addr,
				 reg, &val);
	if (ret < 0)
		goto qca8084_mii_modify_exit;

	val &= ~clear;
	val |= set;
	ret = __qca8084_mii_write(bus, QCA8084_LOW_ADDR_PREFIX | addr,
				  reg, val);
qca8084_mii_modify_exit:
	mutex_unlock(&bus->mdio_lock);
	return ret;
};

static int qca808x_phy_fast_retrain_config(struct phy_device *phydev)
{
	int ret;

	/* Enable fast retrain */
	ret = genphy_c45_fast_retrain(phydev, true);
	if (ret)
		return ret;

	phy_write_mmd(phydev, MDIO_MMD_AN, QCA808X_PHY_MMD7_TOP_OPTION1,
		      QCA808X_TOP_OPTION1_DATA);
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, QCA808X_PHY_MMD1_MSE_THRESHOLD_20DB,
		      QCA808X_MSE_THRESHOLD_20DB_VALUE);
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, QCA808X_PHY_MMD1_MSE_THRESHOLD_17DB,
		      QCA808X_MSE_THRESHOLD_17DB_VALUE);
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, QCA808X_PHY_MMD1_MSE_THRESHOLD_27DB,
		      QCA808X_MSE_THRESHOLD_27DB_VALUE);
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, QCA808X_PHY_MMD1_MSE_THRESHOLD_28DB,
		      QCA808X_MSE_THRESHOLD_28DB_VALUE);
	phy_write_mmd(phydev, MDIO_MMD_PCS, QCA808X_PHY_MMD3_DEBUG_1,
		      QCA808X_MMD3_DEBUG_1_VALUE);
	phy_write_mmd(phydev, MDIO_MMD_PCS, QCA808X_PHY_MMD3_DEBUG_4,
		      QCA808X_MMD3_DEBUG_4_VALUE);
	phy_write_mmd(phydev, MDIO_MMD_PCS, QCA808X_PHY_MMD3_DEBUG_5,
		      QCA808X_MMD3_DEBUG_5_VALUE);
	phy_write_mmd(phydev, MDIO_MMD_PCS, QCA808X_PHY_MMD3_DEBUG_3,
		      QCA808X_MMD3_DEBUG_3_VALUE);
	phy_write_mmd(phydev, MDIO_MMD_PCS, QCA808X_PHY_MMD3_DEBUG_6,
		      QCA808X_MMD3_DEBUG_6_VALUE);
	phy_write_mmd(phydev, MDIO_MMD_PCS, QCA808X_PHY_MMD3_DEBUG_2,
		      QCA808X_MMD3_DEBUG_2_VALUE);

	return 0;
}

static int qca808x_phy_ms_seed_enable(struct phy_device *phydev, bool enable)
{
	u16 seed_value;

	if (!enable)
		return at803x_debug_reg_mask(phydev, QCA808X_PHY_DEBUG_LOCAL_SEED,
				QCA808X_MASTER_SLAVE_SEED_ENABLE, 0);

	seed_value = get_random_u32_below(QCA808X_MASTER_SLAVE_SEED_RANGE);
	return at803x_debug_reg_mask(phydev, QCA808X_PHY_DEBUG_LOCAL_SEED,
			QCA808X_MASTER_SLAVE_SEED_CFG | QCA808X_MASTER_SLAVE_SEED_ENABLE,
			FIELD_PREP(QCA808X_MASTER_SLAVE_SEED_CFG, seed_value) |
			QCA808X_MASTER_SLAVE_SEED_ENABLE);
}

static bool qca808x_is_prefer_master(struct phy_device *phydev)
{
	return (phydev->master_slave_get == MASTER_SLAVE_CFG_MASTER_FORCE) ||
		(phydev->master_slave_get == MASTER_SLAVE_CFG_MASTER_PREFERRED);
}

static bool qca808x_has_fast_retrain_or_slave_seed(struct phy_device *phydev)
{
	return phydev_id_compare(phydev, QCA8081_PHY_ID) &&
		linkmode_test_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
				  phydev->supported);
}

static bool qca808x_is_1g_only(struct phy_device *phydev)
{
	int ret;

	if (!phydev_id_compare(phydev, QCA8081_PHY_ID))
		return false;

	ret = phy_read_mmd(phydev, MDIO_MMD_AN, QCA808X_PHY_MMD7_CHIP_TYPE);
	if (ret < 0)
		return true;

	return !!(QCA808X_PHY_CHIP_TYPE_1G & ret);
}

static void qca808x_fill_possible_interfaces(struct phy_device *phydev)
{
	unsigned long *possible = phydev->possible_interfaces;

	__set_bit(PHY_INTERFACE_MODE_SGMII, possible);

	if (!qca808x_is_1g_only(phydev))
		__set_bit(PHY_INTERFACE_MODE_2500BASEX, possible);
}

static int qca808x_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	struct qca808x_priv *priv;
	u32 ch_id = 0;
	int ret;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	/* Init LED polarity mode to -1 */
	priv->led_polarity_mode = -1;

	/* DT property qcom,xpcs-channel" is optional and only available for
	 * 10G-QXGMII mode.
	 */
	ret = of_property_read_u32(dev->of_node, "qcom,xpcs-channel", &ch_id);
	if (ret && ret != -EINVAL)
		return ret;

	priv->channel_id = ch_id;
	phydev->priv = priv;

	return 0;
}

static int qca808x_config_init(struct phy_device *phydev)
{
	struct qca808x_priv *priv = phydev->priv;
	int ret;

	/* Default to LED Active High if active-low not in DT */
	if (priv->led_polarity_mode == -1) {
		ret = phy_set_bits_mmd(phydev, MDIO_MMD_AN,
				       QCA808X_MMD7_LED_POLARITY_CTRL,
				       QCA808X_LED_ACTIVE_HIGH);
		if (ret)
			return ret;
	}

	/* Active adc&vga on 802.3az for the link 1000M and 100M */
	ret = phy_modify_mmd(phydev, MDIO_MMD_PCS, QCA808X_PHY_MMD3_ADDR_CLD_CTRL7,
			     QCA808X_8023AZ_AFE_CTRL_MASK, QCA808X_8023AZ_AFE_EN);
	if (ret)
		return ret;

	/* Adjust the threshold on 802.3az for the link 1000M */
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
			    QCA808X_PHY_MMD3_AZ_TRAINING_CTRL,
			    QCA808X_MMD3_AZ_TRAINING_VAL);
	if (ret)
		return ret;

	if (qca808x_has_fast_retrain_or_slave_seed(phydev)) {
		/* Config the fast retrain for the link 2500M */
		ret = qca808x_phy_fast_retrain_config(phydev);
		if (ret)
			return ret;

		ret = genphy_read_master_slave(phydev);
		if (ret < 0)
			return ret;

		if (!qca808x_is_prefer_master(phydev)) {
			/* Enable seed and configure lower ramdom seed to make phy
			 * linked as slave mode.
			 */
			ret = qca808x_phy_ms_seed_enable(phydev, true);
			if (ret)
				return ret;
		}
	}

	qca808x_fill_possible_interfaces(phydev);

	/* Configure adc threshold as 100mv for the link 10M */
	return at803x_debug_reg_mask(phydev, QCA808X_PHY_DEBUG_ADC_THRESHOLD,
				     QCA808X_ADC_THRESHOLD_MASK,
				     QCA808X_ADC_THRESHOLD_100MV);
}

static int qca808x_read_status(struct phy_device *phydev)
{
	struct at803x_ss_mask ss_mask = { 0 };
	int ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_10GBT_STAT);
	if (ret < 0)
		return ret;

	linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT, phydev->lp_advertising,
			 ret & MDIO_AN_10GBT_STAT_LP2_5G);

	ret = genphy_read_status(phydev);
	if (ret)
		return ret;

	/* qca8081 takes the different bits for speed value from at803x */
	ss_mask.speed_mask = QCA808X_SS_SPEED_MASK;
	ss_mask.speed_shift = __bf_shf(QCA808X_SS_SPEED_MASK);
	ret = at803x_read_specific_status(phydev, ss_mask);
	if (ret < 0)
		return ret;

	if (phydev->link) {
		/* There are two PCSes available for QCA8084, which support
		 * the following interface modes.
		 *
		 * 1. PHY_INTERFACE_MODE_10G_QXGMII utilizes PCS1 for all
		 * available 4 ports, which is for all link speeds.
		 *
		 * 2. PHY_INTERFACE_MODE_2500BASEX utilizes PCS0 for the
		 * fourth port, which is only for the link speed 2500M same
		 * as QCA8081.
		 *
		 * 3. PHY_INTERFACE_MODE_SGMII utilizes PCS0 for the fourth
		 * port, which is for the link speed 10M, 100M and 1000M same
		 * as QCA8081.
		 */
		if (phydev->interface == PHY_INTERFACE_MODE_10G_QXGMII)
			return 0;

		if (phydev->speed == SPEED_2500)
			phydev->interface = PHY_INTERFACE_MODE_2500BASEX;
		else
			phydev->interface = PHY_INTERFACE_MODE_SGMII;
	} else {
		/* generate seed as a lower random value to make PHY linked as SLAVE easily,
		 * except for master/slave configuration fault detected or the master mode
		 * preferred.
		 *
		 * the reason for not putting this code into the function link_change_notify is
		 * the corner case where the link partner is also the qca8081 PHY and the seed
		 * value is configured as the same value, the link can't be up and no link change
		 * occurs.
		 */
		if (qca808x_has_fast_retrain_or_slave_seed(phydev)) {
			if (phydev->master_slave_state == MASTER_SLAVE_STATE_ERR ||
			    qca808x_is_prefer_master(phydev)) {
				qca808x_phy_ms_seed_enable(phydev, false);
			} else {
				qca808x_phy_ms_seed_enable(phydev, true);
			}
		}
	}

	return 0;
}

static int qca808x_soft_reset(struct phy_device *phydev)
{
	int ret;

	ret = genphy_soft_reset(phydev);
	if (ret < 0)
		return ret;

	if (qca808x_has_fast_retrain_or_slave_seed(phydev))
		ret = qca808x_phy_ms_seed_enable(phydev, true);

	return ret;
}

static int qca808x_cable_test_start(struct phy_device *phydev)
{
	int ret;

	/* perform CDT with the following configs:
	 * 1. disable hibernation.
	 * 2. force PHY working in MDI mode.
	 * 3. for PHY working in 1000BaseT.
	 * 4. configure the threshold.
	 */

	ret = at803x_debug_reg_mask(phydev, QCA808X_DBG_AN_TEST, QCA808X_HIBERNATION_EN, 0);
	if (ret < 0)
		return ret;

	ret = at803x_config_mdix(phydev, ETH_TP_MDI);
	if (ret < 0)
		return ret;

	/* Force 1000base-T needs to configure PMA/PMD and MII_BMCR */
	phydev->duplex = DUPLEX_FULL;
	phydev->speed = SPEED_1000;
	ret = genphy_c45_pma_setup_forced(phydev);
	if (ret < 0)
		return ret;

	ret = genphy_setup_forced(phydev);
	if (ret < 0)
		return ret;

	/* configure the thresholds for open, short, pair ok test */
	phy_write_mmd(phydev, MDIO_MMD_PCS, 0x8074, 0xc040);
	phy_write_mmd(phydev, MDIO_MMD_PCS, 0x8076, 0xc040);
	phy_write_mmd(phydev, MDIO_MMD_PCS, 0x8077, 0xa060);
	phy_write_mmd(phydev, MDIO_MMD_PCS, 0x8078, 0xc050);
	phy_write_mmd(phydev, MDIO_MMD_PCS, 0x807a, 0xc060);
	phy_write_mmd(phydev, MDIO_MMD_PCS, 0x807e, 0xb060);

	if (phydev_id_compare(phydev, QCA8084_PHY_ID)) {
		/* Adjust the positive and negative pulse thereshold of CDT. */
		phy_write_mmd(phydev, MDIO_MMD_PCS,
			      QCA8084_MMD3_CDT_PULSE_CTRL,
			      QCA8084_CDT_PULSE_THRESH_VAL);

		/* Disable the near bypass of CDT. */
		phy_modify_mmd(phydev, MDIO_MMD_PCS,
			       QCA8084_MMD3_CDT_NEAR_CTRL,
			       QCA8084_CDT_NEAR_BYPASS, 0);
	}

	return 0;
}

static int qca808x_get_features(struct phy_device *phydev)
{
	int ret;

	ret = genphy_c45_pma_read_abilities(phydev);
	if (ret)
		return ret;

	/* The autoneg ability is not existed in bit3 of MMD7.1,
	 * but it is supported by qca808x PHY, so we add it here
	 * manually.
	 */
	linkmode_set_bit(ETHTOOL_LINK_MODE_Autoneg_BIT, phydev->supported);

	/* As for the qca8081 1G version chip, the 2500baseT ability is also
	 * existed in the bit0 of MMD1.21, we need to remove it manually if
	 * it is the qca8081 1G chip according to the bit0 of MMD7.0x901d.
	 */
	if (qca808x_is_1g_only(phydev))
		linkmode_clear_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT, phydev->supported);

	return 0;
}

static int qca808x_config_aneg(struct phy_device *phydev)
{
	int phy_ctrl = 0;
	int ret;

	ret = at803x_prepare_config_aneg(phydev);
	if (ret)
		return ret;

	/* The reg MII_BMCR also needs to be configured for force mode, the
	 * genphy_config_aneg is also needed.
	 */
	if (phydev->autoneg == AUTONEG_DISABLE)
		genphy_c45_pma_setup_forced(phydev);

	if (linkmode_test_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT, phydev->advertising))
		phy_ctrl = MDIO_AN_10GBT_CTRL_ADV2_5G;

	ret = phy_modify_mmd_changed(phydev, MDIO_MMD_AN, MDIO_AN_10GBT_CTRL,
				     MDIO_AN_10GBT_CTRL_ADV2_5G, phy_ctrl);
	if (ret < 0)
		return ret;

	return __genphy_config_aneg(phydev, ret);
}

static void qca808x_link_change_notify(struct phy_device *phydev)
{
	/* Assert interface sgmii fifo on link down, deassert it on link up,
	 * the interface device address is always phy address added by 1.
	 */
	mdiobus_c45_modify_changed(phydev->mdio.bus, phydev->mdio.addr + 1,
				   MDIO_MMD_PMAPMD, QCA8081_PHY_SERDES_MMD1_FIFO_CTRL,
				   QCA8081_PHY_FIFO_RSTN,
				   phydev->link ? QCA8081_PHY_FIFO_RSTN : 0);
}

static int qca808x_led_parse_netdev(struct phy_device *phydev, unsigned long rules,
				    u16 *offload_trigger)
{
	/* Parsing specific to netdev trigger */
	if (test_bit(TRIGGER_NETDEV_TX, &rules))
		*offload_trigger |= QCA808X_LED_TX_BLINK;
	if (test_bit(TRIGGER_NETDEV_RX, &rules))
		*offload_trigger |= QCA808X_LED_RX_BLINK;
	if (test_bit(TRIGGER_NETDEV_LINK_10, &rules))
		*offload_trigger |= QCA808X_LED_SPEED10_ON;
	if (test_bit(TRIGGER_NETDEV_LINK_100, &rules))
		*offload_trigger |= QCA808X_LED_SPEED100_ON;
	if (test_bit(TRIGGER_NETDEV_LINK_1000, &rules))
		*offload_trigger |= QCA808X_LED_SPEED1000_ON;
	if (test_bit(TRIGGER_NETDEV_LINK_2500, &rules))
		*offload_trigger |= QCA808X_LED_SPEED2500_ON;
	if (test_bit(TRIGGER_NETDEV_HALF_DUPLEX, &rules))
		*offload_trigger |= QCA808X_LED_HALF_DUPLEX_ON;
	if (test_bit(TRIGGER_NETDEV_FULL_DUPLEX, &rules))
		*offload_trigger |= QCA808X_LED_FULL_DUPLEX_ON;

	if (rules && !*offload_trigger)
		return -EOPNOTSUPP;

	/* Enable BLINK_CHECK_BYPASS by default to make the LED
	 * blink even with duplex or speed mode not enabled.
	 */
	*offload_trigger |= QCA808X_LED_BLINK_CHECK_BYPASS;

	return 0;
}

static int qca808x_led_hw_control_enable(struct phy_device *phydev, u8 index)
{
	u16 reg;

	if (index > 2)
		return -EINVAL;

	reg = QCA808X_MMD7_LED_FORCE_CTRL(index);
	return qca808x_led_reg_hw_control_enable(phydev, reg);
}

static int qca808x_led_hw_is_supported(struct phy_device *phydev, u8 index,
				       unsigned long rules)
{
	u16 offload_trigger = 0;

	if (index > 2)
		return -EINVAL;

	return qca808x_led_parse_netdev(phydev, rules, &offload_trigger);
}

static int qca808x_led_hw_control_set(struct phy_device *phydev, u8 index,
				      unsigned long rules)
{
	u16 reg, offload_trigger = 0;
	int ret;

	if (index > 2)
		return -EINVAL;

	reg = QCA808X_MMD7_LED_CTRL(index);

	ret = qca808x_led_parse_netdev(phydev, rules, &offload_trigger);
	if (ret)
		return ret;

	ret = qca808x_led_hw_control_enable(phydev, index);
	if (ret)
		return ret;

	return phy_modify_mmd(phydev, MDIO_MMD_AN, reg,
			      QCA808X_LED_PATTERN_MASK,
			      offload_trigger);
}

static bool qca808x_led_hw_control_status(struct phy_device *phydev, u8 index)
{
	u16 reg;

	if (index > 2)
		return false;

	reg = QCA808X_MMD7_LED_FORCE_CTRL(index);
	return qca808x_led_reg_hw_control_status(phydev, reg);
}

static int qca808x_led_hw_control_get(struct phy_device *phydev, u8 index,
				      unsigned long *rules)
{
	u16 reg;
	int val;

	if (index > 2)
		return -EINVAL;

	/* Check if we have hw control enabled */
	if (qca808x_led_hw_control_status(phydev, index))
		return -EINVAL;

	reg = QCA808X_MMD7_LED_CTRL(index);

	val = phy_read_mmd(phydev, MDIO_MMD_AN, reg);
	if (val & QCA808X_LED_TX_BLINK)
		set_bit(TRIGGER_NETDEV_TX, rules);
	if (val & QCA808X_LED_RX_BLINK)
		set_bit(TRIGGER_NETDEV_RX, rules);
	if (val & QCA808X_LED_SPEED10_ON)
		set_bit(TRIGGER_NETDEV_LINK_10, rules);
	if (val & QCA808X_LED_SPEED100_ON)
		set_bit(TRIGGER_NETDEV_LINK_100, rules);
	if (val & QCA808X_LED_SPEED1000_ON)
		set_bit(TRIGGER_NETDEV_LINK_1000, rules);
	if (val & QCA808X_LED_SPEED2500_ON)
		set_bit(TRIGGER_NETDEV_LINK_2500, rules);
	if (val & QCA808X_LED_HALF_DUPLEX_ON)
		set_bit(TRIGGER_NETDEV_HALF_DUPLEX, rules);
	if (val & QCA808X_LED_FULL_DUPLEX_ON)
		set_bit(TRIGGER_NETDEV_FULL_DUPLEX, rules);

	return 0;
}

static int qca808x_led_hw_control_reset(struct phy_device *phydev, u8 index)
{
	u16 reg;

	if (index > 2)
		return -EINVAL;

	reg = QCA808X_MMD7_LED_CTRL(index);

	return phy_clear_bits_mmd(phydev, MDIO_MMD_AN, reg,
				  QCA808X_LED_PATTERN_MASK);
}

static int qca808x_led_brightness_set(struct phy_device *phydev,
				      u8 index, enum led_brightness value)
{
	u16 reg;
	int ret;

	if (index > 2)
		return -EINVAL;

	if (!value) {
		ret = qca808x_led_hw_control_reset(phydev, index);
		if (ret)
			return ret;
	}

	reg = QCA808X_MMD7_LED_FORCE_CTRL(index);
	return qca808x_led_reg_brightness_set(phydev, reg, value);
}

static int qca808x_led_blink_set(struct phy_device *phydev, u8 index,
				 unsigned long *delay_on,
				 unsigned long *delay_off)
{
	u16 reg;

	if (index > 2)
		return -EINVAL;

	reg = QCA808X_MMD7_LED_FORCE_CTRL(index);
	return qca808x_led_reg_blink_set(phydev, reg, delay_on, delay_off);
}

static int qca808x_led_polarity_set(struct phy_device *phydev, int index,
				    unsigned long modes)
{
	struct qca808x_priv *priv = phydev->priv;
	bool active_low = false;
	u32 mode;

	for_each_set_bit(mode, &modes, __PHY_LED_MODES_NUM) {
		switch (mode) {
		case PHY_LED_ACTIVE_LOW:
			active_low = true;
			break;
		default:
			return -EINVAL;
		}
	}

	/* PHY polarity is global and can't be set per LED.
	 * To detect this, check if last requested polarity mode
	 * match the new one.
	 */
	if (priv->led_polarity_mode >= 0 &&
	    priv->led_polarity_mode != active_low) {
		phydev_err(phydev, "PHY polarity is global. Mismatched polarity on different LED\n");
		return -EINVAL;
	}

	/* Save the last PHY polarity mode */
	priv->led_polarity_mode = active_low;

	return phy_modify_mmd(phydev, MDIO_MMD_AN,
			      QCA808X_MMD7_LED_POLARITY_CTRL,
			      QCA808X_LED_ACTIVE_HIGH,
			      active_low ? 0 : QCA808X_LED_ACTIVE_HIGH);
}

static int qca8084_package_clock_init(struct qca808x_shared_priv *shared_priv)
{
	int ret;

	/* Configure clock rate 312.5MHZ for the PHY package
	 * APB bridge clock tree.
	 */
	ret = clk_set_rate(shared_priv->clk[APB_BRIDGE_CLK], 312500000);
	if (ret)
		return ret;

	ret = clk_prepare_enable(shared_priv->clk[SWITCH_CORE_CLK]);
	if (ret)
		return ret;

	ret = clk_prepare_enable(shared_priv->clk[APB_BRIDGE_CLK]);
	if (ret)
		return ret;

	/* Configure clock rate 104.17MHZ for the PHY package
	 * AHB clock tree.
	 */
	ret = clk_set_rate(shared_priv->clk[AHB_CLK], 104170000);
	if (ret)
		return ret;

	ret = clk_prepare_enable(shared_priv->clk[AHB_CLK]);
	if (ret)
		return ret;

	ret = clk_prepare_enable(shared_priv->clk[SEC_CTRL_AHB_CLK]);
	if (ret)
		return ret;

	ret = clk_prepare_enable(shared_priv->clk[TLMM_CLK]);
	if (ret)
		return ret;

	ret = clk_prepare_enable(shared_priv->clk[TLMM_AHB_CLK]);
	if (ret)
		return ret;

	ret = clk_prepare_enable(shared_priv->clk[CNOC_AHB_CLK]);
	if (ret)
		return ret;

	ret = clk_prepare_enable(shared_priv->clk[MDIO_MASTER_AHB_CLK]);
	if (ret)
		return ret;

	return clk_prepare_enable(shared_priv->clk[MDIO_AHB_CLK]);
}

static int qca8084_phy_package_config_init_once(struct phy_device *phydev)
{
	struct qca808x_shared_priv *shared_priv;
	int ret, mode;

	shared_priv = phy_package_get_priv(phydev);
	switch (shared_priv->package_mode) {
	case QCA808X_PCS1_10G_QXGMII_PCS0_UNUNSED:
		mode = QCA8084_WORK_MODE_QXGMII;
		break;
	case QCA808X_PCS1_SGMII_MAC_PCS0_SGMII_MAC:
		mode = QCA8084_WORK_MODE_SWITCH;
		break;
	case QCA808X_PCS1_SGMII_MAC_PCS0_SGMII_PHY:
		mode = QCA8084_WORK_MODE_SWITCH_PORT4_SGMII;
		break;
	default:
		phydev_err(phydev, "Invalid qcom,package-mode %d\n",
			   shared_priv->package_mode);
		return -EINVAL;
	}

	ret = qca8084_mii_modify(phydev, QCA8084_WORK_MODE_CFG,
				 QCA8084_WORK_MODE_MASK,
				 FIELD_PREP(QCA8084_WORK_MODE_MASK, mode));
	if (ret)
		return ret;

	/* Enable efuse loading into analog circuit */
	ret = qca8084_mii_modify(phydev, QCA8084_EPHY_CFG,
				 QCA8084_EPHY_LDO_EN, 0);
	if (ret)
		return ret;

	usleep_range(10000, 11000);

	/* Configure PCS working on 10G-QXGMII mode */
	if (phydev->interface == PHY_INTERFACE_MODE_10G_QXGMII) {
		ret = qca8084_qxgmii_set_mode(shared_priv->mdiodev[1],
					      shared_priv->mdiodev[0]);
		if (ret)
			return ret;
	}

	/* Initialize the PHY package clock and reset, which is the
	 * necessary config sequence after GPIO reset on the PHY package.
	 */
	return qca8084_package_clock_init(shared_priv);
}

static int qca8084_config_init(struct phy_device *phydev)
{
	int ret;

	if (phy_package_init_once(phydev)) {
		ret = qca8084_phy_package_config_init_once(phydev);
		if (ret)
			return ret;
	}

	if (phydev->interface == PHY_INTERFACE_MODE_10G_QXGMII)
		__set_bit(PHY_INTERFACE_MODE_10G_QXGMII,
			  phydev->possible_interfaces);
	else
		qca808x_fill_possible_interfaces(phydev);

	/* Configure the ADC to convert the signal using falling edge
	 * instead of the default rising edge.
	 */
	ret = at803x_debug_reg_mask(phydev, QCA8084_ADC_CLK_SEL,
				    QCA8084_ADC_CLK_SEL_ACLK,
				    FIELD_PREP(QCA8084_ADC_CLK_SEL_ACLK,
					       QCA8084_ADC_CLK_SEL_ACLK_FALL));
	if (ret < 0)
		return ret;

	/* Adjust MSE threshold value to avoid link issue with
	 * some link partner.
	 */
	return phy_write_mmd(phydev, MDIO_MMD_PMAPMD,
			     QCA8084_MSE_THRESHOLD,
			     QCA8084_MSE_THRESHOLD_2P5G_VAL);
}

static void qca8084_link_change_notify(struct phy_device *phydev)
{
	struct qca808x_shared_priv *shared_priv;
	int ret;

	/* Assert the FIFO between PHY and MAC. */
	ret = phy_modify(phydev, QCA8084_FIFO_CONTROL,
			 QCA8084_FIFO_MAC_2_PHY | QCA8084_FIFO_PHY_2_MAC,
			 0);
	if (ret) {
		phydev_err(phydev, "Asserting PHY FIFO failed\n");
		return;
	}

	/* If the PHY is in 10G_QXGMII mode, the FIFO needs to be kept in
	 * reset state when link is down, otherwise the FIFO needs to be
	 * de-asserted after waiting 50 ms to make the assert completed.
	 */
	if (phydev->interface != PHY_INTERFACE_MODE_10G_QXGMII ||
	    phydev->link) {
		msleep(50);

		/* Deassert the FIFO between PHY and MAC. */
		ret = phy_modify(phydev, QCA8084_FIFO_CONTROL,
				 QCA8084_FIFO_MAC_2_PHY |
				 QCA8084_FIFO_PHY_2_MAC,
				 QCA8084_FIFO_MAC_2_PHY |
				 QCA8084_FIFO_PHY_2_MAC);
		if (ret) {
			phydev_err(phydev, "De-asserting PHY FIFO failed\n");
			return;
		}
	}

	/* Enable IPG level 10 to 11 tuning for link speed 1000M and
	 * configure the related XPCS channel with the phydev in the
	 * 10G_QXGMII mode.
	 */
	if (phydev->interface == PHY_INTERFACE_MODE_10G_QXGMII) {
		shared_priv = phy_package_get_priv(phydev);
		struct qca808x_priv *priv = phydev->priv;

		phy_modify_mmd(phydev, MDIO_MMD_AN, QCA8084_MMD7_IPG_OP,
			       QCA8084_IPG_10_TO_11_EN,
			       phydev->speed == SPEED_1000 ?
			       QCA8084_IPG_10_TO_11_EN : 0);

		qca8084_qxgmii_set_speed(shared_priv->mdiodev[1],
					 shared_priv->mdiodev[0],
					 priv->channel_id,
					 phydev->speed);
	}
}

/* QCA8084 is a four-port PHY, which integrates the clock controller,
 * 4 PHY devices and 2 PCS interfaces (PCS0 and PCS1). PCS1 includes
 * XPCS and PCS to support 10G-QXGMII and SGMII. PCS0 includes one PCS
 * to support SGMII.
 *
 * The clocks and resets are sourced from the integrated clock controller
 * of the PHY package. This integrated clock controller is driven by a
 * QCA8K clock provider that supplies the clocks and resets to the four
 * PHYs, PCS and PHY package.
 */
static int qca8084_phy_package_probe_once(struct phy_device *phydev)
{
	int addr[QCA8084_MDIO_DEVICE_NUM] = {0, 1, 2, 3, 4, 5, 6};
	struct device_node *np = phy_package_get_node(phydev);
	struct qca808x_shared_priv *shared_priv;
	struct reset_control *rstc;
	struct device_node *child;
	int i, ret, clear, set;
	struct clk *clk;

	/* Program the MDIO address of PHY and PCS optionally, the MDIO
	 * address 0-6 is used for PHY and PCS MDIO devices by default.
	 */
	ret = of_property_read_u32_array(np, "qcom,phy-addr-fixup",
					 addr, ARRAY_SIZE(addr));
	if (ret && ret != -EINVAL)
		return ret;

	/* Configure the MDIO addresses for the four PHY devices. */
	clear = QCA8084_EPHY_ADDR0_MASK | QCA8084_EPHY_ADDR1_MASK |
		QCA8084_EPHY_ADDR2_MASK | QCA8084_EPHY_ADDR3_MASK;
	set = FIELD_PREP(QCA8084_EPHY_ADDR0_MASK, addr[0]);
	set |= FIELD_PREP(QCA8084_EPHY_ADDR1_MASK, addr[1]);
	set |= FIELD_PREP(QCA8084_EPHY_ADDR2_MASK, addr[2]);
	set |= FIELD_PREP(QCA8084_EPHY_ADDR3_MASK, addr[3]);

	ret = qca8084_mii_modify(phydev, QCA8084_EPHY_CFG, clear, set);
	if (ret)
		return ret;

	/* Configure the MDIO addresses for PCS0 and PCS1 including
	 * PCS and XPCS.
	 */
	clear = QCA8084_PCS_ADDR0_MASK | QCA8084_PCS_ADDR1_MASK |
		QCA8084_PCS_ADDR2_MASK;
	set = FIELD_PREP(QCA8084_PCS_ADDR0_MASK, addr[4]);
	set |= FIELD_PREP(QCA8084_PCS_ADDR1_MASK, addr[5]);
	set |= FIELD_PREP(QCA8084_PCS_ADDR2_MASK, addr[6]);

	ret =  qca8084_mii_modify(phydev, QCA8084_PCS_CFG, clear, set);
	if (ret)
		return ret;

	shared_priv = phy_package_get_priv(phydev);
	for (i = 0; i < ARRAY_SIZE(qca8084_package_clk_name); i++) {
		clk = of_clk_get_by_name(np, qca8084_package_clk_name[i]);
		if (IS_ERR(clk)) {
			if (PTR_ERR(clk) == -EINVAL)
				clk = NULL;
			else
				return dev_err_probe(&phydev->mdio.dev, PTR_ERR(clk),
						     "package clock %s not ready\n",
						     qca8084_package_clk_name[i]);
		}

		shared_priv->clk[i] = clk;
	}

	/* The package mode 10G-QXGMII of PCS1 is used for Quad PHY and
	 * PCS0 is unused by default.
	 */
	shared_priv->package_mode = QCA808X_PCS1_10G_QXGMII_PCS0_UNUNSED;
	ret = of_property_read_u32(np, "qcom,package-mode",
				   &shared_priv->package_mode);
	if (ret && ret != -EINVAL)
		return ret;

	for_each_available_child_of_node(np, child) {
		struct mdio_device *mdiodev;

		if (of_node_name_eq(child, "pcs-phy")) {
			mdiodev = qca8084_package_pcs_probe(child);
			if (IS_ERR(mdiodev))
				return PTR_ERR(mdiodev);

			shared_priv->mdiodev[0] = mdiodev;
		}

		if (of_node_name_eq(child, "xpcs-phy")) {
			mdiodev = qca8084_package_xpcs_probe(child);
			if (IS_ERR(mdiodev))
				return PTR_ERR(mdiodev);

			shared_priv->mdiodev[1] = mdiodev;
		}
	}

	rstc = of_reset_control_get_exclusive(np, NULL);
	if (IS_ERR(rstc))
		return dev_err_probe(&phydev->mdio.dev, PTR_ERR(rstc),
				     "package reset not ready\n");

	/* Deassert PHY package. */
	return reset_control_deassert(rstc);
}

static void qca8084_phy_package_remove_once(struct phy_device *phydev)
{
	struct qca808x_shared_priv *shared_priv = phy_package_get_priv(phydev);;

	qca8084_package_xpcs_and_pcs_remove(shared_priv->mdiodev[1],
					    shared_priv->mdiodev[0]);
}

static int qca8084_probe(struct phy_device *phydev)
{
	struct qca808x_shared_priv *shared_priv;
	struct device *dev = &phydev->mdio.dev;
	struct reset_control *rstc;
	struct clk *clk;
	int ret;

	ret = devm_of_phy_package_join(dev, phydev, sizeof(*shared_priv));
	if (ret)
		return ret;

	if (phy_package_probe_once(phydev)) {
		ret = qca8084_phy_package_probe_once(phydev);
		if (ret)
			return ret;
	}

	ret = qca808x_probe(phydev);
	if (ret)
		return ret;

	/* Enable clock of PHY device, so that the PHY register
	 * can be accessed to get PHY features.
	 */
	clk = devm_clk_get_enabled(dev, NULL);
	if (IS_ERR(clk))
		return dev_err_probe(dev, PTR_ERR(clk),
				     "Enable PHY clock failed\n");

	/* De-assert PHY reset after the clock of PHY enabled. */
	rstc = devm_reset_control_get_exclusive(dev, NULL);
	if (IS_ERR(rstc))
		return dev_err_probe(dev, PTR_ERR(rstc),
				     "Get PHY reset failed\n");

	return reset_control_deassert(rstc);
}

static void qca8084_remove(struct phy_device *phydev)
{
	if (phydev->interface != PHY_INTERFACE_MODE_10G_QXGMII)
		return;

	if (phy_package_remove_once(phydev))
		qca8084_phy_package_remove_once(phydev);
}

static struct phy_driver qca808x_driver[] = {
{
	/* Qualcomm QCA8081 */
	PHY_ID_MATCH_EXACT(QCA8081_PHY_ID),
	.name			= "Qualcomm QCA8081",
	.flags			= PHY_POLL_CABLE_TEST,
	.probe			= qca808x_probe,
	.config_intr		= at803x_config_intr,
	.handle_interrupt	= at803x_handle_interrupt,
	.get_tunable		= at803x_get_tunable,
	.set_tunable		= at803x_set_tunable,
	.set_wol		= at8031_set_wol,
	.get_wol		= at803x_get_wol,
	.get_features		= qca808x_get_features,
	.config_aneg		= qca808x_config_aneg,
	.suspend		= genphy_suspend,
	.resume			= genphy_resume,
	.read_status		= qca808x_read_status,
	.config_init		= qca808x_config_init,
	.soft_reset		= qca808x_soft_reset,
	.cable_test_start	= qca808x_cable_test_start,
	.cable_test_get_status	= qca808x_cable_test_get_status,
	.link_change_notify	= qca808x_link_change_notify,
	.led_brightness_set	= qca808x_led_brightness_set,
	.led_blink_set		= qca808x_led_blink_set,
	.led_hw_is_supported	= qca808x_led_hw_is_supported,
	.led_hw_control_set	= qca808x_led_hw_control_set,
	.led_hw_control_get	= qca808x_led_hw_control_get,
	.led_polarity_set	= qca808x_led_polarity_set,
}, {
	/* Qualcomm QCA8084 */
	PHY_ID_MATCH_MODEL(QCA8084_PHY_ID),
	.name			= "Qualcomm QCA8084",
	.flags			= PHY_POLL_CABLE_TEST,
	.config_intr		= at803x_config_intr,
	.handle_interrupt	= at803x_handle_interrupt,
	.get_tunable		= at803x_get_tunable,
	.set_tunable		= at803x_set_tunable,
	.set_wol		= at803x_set_wol,
	.get_wol		= at803x_get_wol,
	.get_features		= qca808x_get_features,
	.config_aneg		= qca808x_config_aneg,
	.suspend		= genphy_suspend,
	.resume			= genphy_resume,
	.read_status		= qca808x_read_status,
	.soft_reset		= qca808x_soft_reset,
	.cable_test_start	= qca808x_cable_test_start,
	.cable_test_get_status	= qca808x_cable_test_get_status,
	.config_init		= qca8084_config_init,
	.link_change_notify	= qca8084_link_change_notify,
	.probe			= qca8084_probe,
	.remove			= qca8084_remove,
}, };

module_phy_driver(qca808x_driver);

static const struct mdio_device_id __maybe_unused qca808x_tbl[] = {
	{ PHY_ID_MATCH_EXACT(QCA8081_PHY_ID) },
	{ PHY_ID_MATCH_MODEL(QCA8084_PHY_ID) },
	{ }
};

MODULE_DEVICE_TABLE(mdio, qca808x_tbl);
