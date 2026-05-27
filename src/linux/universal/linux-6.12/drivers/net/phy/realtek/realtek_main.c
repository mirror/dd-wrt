// SPDX-License-Identifier: GPL-2.0+
/* drivers/net/phy/realtek.c
 *
 * Driver for Realtek PHYs
 *
 * Author: Johnson Leung <r58129@freescale.com>
 *
 * Copyright (c) 2004 Freescale Semiconductor, Inc.
 */
#include <linux/bitops.h>
#include <linux/ethtool_netlink.h>
#include <linux/of.h>
#include <linux/phy.h>
#include <linux/phy/phy-common-props.h>
#include <linux/pm_wakeirq.h>
#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/crc32.h>
#include <linux/string_choices.h>
#include <net/phy/realtek_phy.h>

#include "realtek.h"

#define RTL8201F_IER				0x13

#define RTL8201F_ISR				0x1e
#define RTL8201F_ISR_ANERR			BIT(15)
#define RTL8201F_ISR_DUPLEX			BIT(13)
#define RTL8201F_ISR_LINK			BIT(11)
#define RTL8201F_ISR_MASK			(RTL8201F_ISR_ANERR | \
						 RTL8201F_ISR_DUPLEX | \
						 RTL8201F_ISR_LINK)

#define RTL821x_INER				0x12
#define RTL8211B_INER_INIT			0x6400
#define RTL8211E_INER_LINK_STATUS		BIT(10)
#define RTL8211F_INER_PME			BIT(7)
#define RTL8211F_INER_LINK_STATUS		BIT(4)

#define RTL821x_INSR				0x13

#define RTL821x_EXT_PAGE_SELECT			0x1e

#define RTL821x_PAGE_SELECT			0x1f
#define RTL821x_SET_EXT_PAGE			0x07

/* RTL8211E extension page 44/0x2c */
#define RTL8211E_LEDCR_EXT_PAGE			0x2c
#define RTL8211E_LEDCR1				0x1a
#define RTL8211E_LEDCR1_ACT_TXRX		BIT(4)
#define RTL8211E_LEDCR1_MASK			BIT(4)
#define RTL8211E_LEDCR1_SHIFT			1

#define RTL8211E_LEDCR2				0x1c
#define RTL8211E_LEDCR2_LINK_1000		BIT(2)
#define RTL8211E_LEDCR2_LINK_100		BIT(1)
#define RTL8211E_LEDCR2_LINK_10			BIT(0)
#define RTL8211E_LEDCR2_MASK			GENMASK(2, 0)
#define RTL8211E_LEDCR2_SHIFT			4

/* RTL8211E extension page 164/0xa4 */
#define RTL8211E_RGMII_EXT_PAGE			0xa4
#define RTL8211E_RGMII_DELAY			0x1c
#define RTL8211E_CTRL_DELAY			BIT(13)
#define RTL8211E_TX_DELAY			BIT(12)
#define RTL8211E_RX_DELAY			BIT(11)
#define RTL8211E_DELAY_MASK			GENMASK(13, 11)

/* RTL8211F PHY configuration */
#define RTL8211F_PHYCR1				0x18
#define RTL8211F_ALDPS_PLL_OFF			BIT(1)
#define RTL8211F_ALDPS_ENABLE			BIT(2)
#define RTL8211F_ALDPS_XTAL_OFF			BIT(12)

#define RTL8211F_PHYCR2				0x19
#define RTL8211F_CLKOUT_EN			BIT(0)
#define RTL8211F_PHYCR2_PHY_EEE_ENABLE		BIT(5)

#define RTL8211F_INSR				0x1d

/* RTL8211F LED configuration */
#define RTL8211F_LEDCR_PAGE			0xd04
#define RTL8211F_LEDCR				0x10
#define RTL8211F_LEDCR_MODE			BIT(15)
#define RTL8211F_LEDCR_ACT_TXRX			BIT(4)
#define RTL8211F_LEDCR_LINK_1000		BIT(3)
#define RTL8211F_LEDCR_LINK_100			BIT(1)
#define RTL8211F_LEDCR_LINK_10			BIT(0)
#define RTL8211F_LEDCR_MASK			GENMASK(4, 0)
#define RTL8211F_LEDCR_SHIFT			5

/* RTL8211F(D)(I)-VD-CG CLKOUT configuration is specified via magic values
 * to undocumented register pages. The names here do not reflect the datasheet.
 * Unlike other PHY models, CLKOUT configuration does not go through PHYCR2.
 */
#define RTL8211FVD_CLKOUT_PAGE			0xd05
#define RTL8211FVD_CLKOUT_REG			0x11
#define RTL8211FVD_CLKOUT_EN			BIT(8)

/* RTL8211F RGMII configuration */
#define RTL8211F_RGMII_PAGE			0xd08

#define RTL8211F_TXCR				0x11
#define RTL8211F_TX_DELAY			BIT(8)

#define RTL8211F_RXCR				0x15
#define RTL8211F_RX_DELAY			BIT(3)

/* RTL8211F WOL settings */
#define RTL8211F_WOL_PAGE		0xd8a
#define RTL8211F_WOL_SETTINGS_EVENTS		16
#define RTL8211F_WOL_EVENT_MAGIC		BIT(12)
#define RTL8211F_WOL_RST_RMSQ		17
#define RTL8211F_WOL_RG_RSTB			BIT(15)
#define RTL8211F_WOL_RMSQ			0x1fff

/* RTL8211F Unique phyiscal and multicast address (WOL) */
#define RTL8211F_PHYSICAL_ADDR_PAGE		0xd8c
#define RTL8211F_PHYSICAL_ADDR_WORD0		16
#define RTL8211F_PHYSICAL_ADDR_WORD1		17
#define RTL8211F_PHYSICAL_ADDR_WORD2		18

#define RTL822X_VND1_SERDES_OPTION			0x697a
#define RTL822X_VND1_SERDES_OPTION_MODE_MASK		GENMASK(5, 0)
#define RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX_SGMII		0
#define RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX_SGMII_HSGMII	1
#define RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX		2
#define RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX_HSGMII	3

#define RTL822X_VND1_SERDES_CTRL3			0x7580
#define RTL822X_VND1_SERDES_CTRL3_MODE_MASK		GENMASK(5, 0)
#define RTL822X_VND1_SERDES_CTRL3_MODE_SGMII			0x02
#define RTL822X_VND1_SERDES_CTRL3_MODE_2500BASEX		0x16

#define RTL822X_VND1_SERDES_CMD			0x7587
#define  RTL822X_VND1_SERDES_CMD_WRITE		BIT(1)
#define  RTL822X_VND1_SERDES_CMD_BUSY		BIT(0)
#define RTL822X_VND1_SERDES_ADDR		0x7588
#define  RTL822X_VND1_SERDES_ADDR_AUTONEG	0x2
#define   RTL822X_VND1_SERDES_INBAND_DISABLE	0x71d0
#define   RTL822X_VND1_SERDES_INBAND_ENABLE	0x70d0
#define RTL822X_VND1_SERDES_DATA		0x7589

#define RTL822X_VND2_TO_PAGE(reg)		((reg) >> 4)
#define RTL822X_VND2_TO_PAGE_REG(reg)		(16 + (((reg) & GENMASK(3, 0)) >> 1))
#define RTL822X_VND2_TO_C22_REG(reg)		(((reg) - 0xa400) / 2)
#define RTL822X_VND2_C22_REG(reg)		(0xa400 + 2 * (reg))

#define RTL8221B_VND2_INER			0xa4d2
#define RTL8221B_VND2_INER_LINK_STATUS		BIT(4)

#define RTL8221B_VND2_INSR			0xa4d4

#define RTL8224_MII_RTCT			0x11
#define RTL8224_MII_RTCT_ENABLE			BIT(0)
#define RTL8224_MII_RTCT_PAIR_A			BIT(4)
#define RTL8224_MII_RTCT_PAIR_B			BIT(5)
#define RTL8224_MII_RTCT_PAIR_C			BIT(6)
#define RTL8224_MII_RTCT_PAIR_D			BIT(7)
#define RTL8224_MII_RTCT_DONE			BIT(15)

#define RTL8224_MII_SRAM_ADDR			0x1b
#define RTL8224_MII_SRAM_DATA			0x1c

#define RTL8224_SRAM_RTCT_FAULT(pair)		(0x8026 + (pair) * 4)
#define RTL8224_SRAM_RTCT_FAULT_BUSY		BIT(0)
#define RTL8224_SRAM_RTCT_FAULT_OPEN		BIT(3)
#define RTL8224_SRAM_RTCT_FAULT_SAME_SHORT	BIT(4)
#define RTL8224_SRAM_RTCT_FAULT_OK		BIT(5)
#define RTL8224_SRAM_RTCT_FAULT_DONE		BIT(6)
#define RTL8224_SRAM_RTCT_FAULT_CROSS_SHORT	BIT(7)

#define RTL8224_SRAM_RTCT_LEN(pair)		(0x8028 + (pair) * 4)

#define RTL8224_VND1_SERDES_CMD			0x3f8
#define RTL8224_VND1_SERDES_READ		0x3fc
#define RTL8224_VND1_SERDES_WRITE		0x400
#define RTL8224_SDS_CMD_READ			BIT(15)
#define RTL8224_SDS_CMD_WRITE			(BIT(15) | BIT(14))

#define RTL8224_VND1_SERDES_MODE		0x7b20
#define RTL8224_VND1_SERDES_MODE_MASK		GENMASK(4, 0)
#define RTL8224_VND1_SERDES_MODE_OFF		0x1f
#define RTL8224_VND1_SERDES_MODE_USXGMII	0x0d
#define RTL8224_VND1_SERDES_SUBMODE_MASK	GENMASK(14, 10)
#define RTL8224_VND1_SERDES_SUBMODE_10G_QXGMII	(0x2 << 10)

#define RTL8224_SDS_AM_PAGE			0x6
#define RTL8224_SDS_AM_PERIOD_REG		0x12

#define RTL8224_SDS_AM_CFG0_REG			0x13
#define RTL8224_SDS_AM_CFG1_REG			0x14
#define RTL8224_SDS_AM_CFG2_REG			0x15
#define RTL8224_SDS_AM_CFG3_REG			0x16
#define RTL8224_SDS_AM_CFG4_REG			0x17
#define RTL8224_SDS_AM_CFG5_REG			0x18

#define RTL8224_SDS_NWAY_PAGE			0x7
#define RTL8224_SDS_NWAY_OPCODE_REG		0x10
#define RTL8224_SDS_NWAY_OPCODE_MASK		GENMASK(7, 0)
#define RTL8224_SDS_NWAY_AN_REG			0x11
#define RTL8224_SDS_NWAY_AN_EN_MASK		GENMASK(3, 0)

#define RTL8224_SDS_REG0_TX_PAGE		0x2e
#define RTL8224_SDS_REG0_TX_POST1_REG		0x6
#define RTL8224_SDS_REG0_TX_REG			0x7
#define RTL8224_SDS_REG0_TX_Z0_REG		0xb


#define RTL8221B_PHYCR1				0xa430
#define RTL8221B_PHYCR1_ALDPS_EN		BIT(2)
#define RTL8221B_PHYCR1_ALDPS_XTAL_OFF_EN	BIT(12)
#define RTL8221B_PHYCR1_PHYAD_0_EN		BIT(13)

#define RTL8224_VND1_MDI_PAIR_SWAP		0xa90
#define RTL8224_VND1_MDI_POLARITY_SWAP		0xa94

#define RTL8226_VND1_UNKNOWN_6A21		0x6a21
#define  RTL8226_VND1_UNKNOWN_6A21_MDI_SWAP_EN	BIT(5)

#define RTL8226_VND2_UNKNOWN_D068		0xd068
#define  RTL8226_VND2_UNKNOWN_D068_MDI_SWAP_FLAG	BIT(1)
#define  RTL8226_VND2_UNKNOWN_D068_PAIR_SEL	GENMASK(4, 3)
#define RTL8226_VND2_ADCCAL_OFFSET		0xd06a

#define RTL8226_VND2_RG_LPF_CAP_XG_P0_P1	0xbd5a
#define RTL8226_VND2_RG_LPF_CAP_XG_P2_P3	0xbd5c
#define RTL8226_VND2_RG_LPF_CAP_P0_P1		0xbc18
#define RTL8226_VND2_RG_LPF_CAP_P2_P3		0xbc1a
#define  RTL8226_RG_LPF_CAP_PAIR_A_MASK		GENMASK(4, 0)
#define  RTL8226_RG_LPF_CAP_PAIR_B_MASK		GENMASK(12, 8)

#define RTL8366RB_POWER_SAVE			0x15
#define RTL8366RB_POWER_SAVE_ON			BIT(12)

#define RTL9000A_GINMR				0x14
#define RTL9000A_GINMR_LINK_STATUS		BIT(4)

#define RTL_PHYSR				MII_RESV2
#define RTL_PHYSR_DUPLEX			BIT(3)
#define RTL_PHYSR_SPEEDL			GENMASK(5, 4)
#define RTL_PHYSR_SPEEDH			GENMASK(10, 9)
#define RTL_PHYSR_MASTER			BIT(11)
#define RTL_PHYSR_SPEED_MASK			(RTL_PHYSR_SPEEDL | RTL_PHYSR_SPEEDH)

#define	RTL_MDIO_PCS_EEE_ABLE			0xa5c4
#define	RTL_MDIO_AN_EEE_ADV			0xa5d0
#define	RTL_MDIO_AN_EEE_LPABLE			0xa5d2
#define	RTL_MDIO_AN_10GBT_CTRL			0xa5d4
#define	RTL_MDIO_AN_10GBT_STAT			0xa5d6
#define	RTL_MDIO_PMA_SPEED			0xa616
#define	RTL_MDIO_AN_EEE_LPABLE2			0xa6d0
#define	RTL_MDIO_AN_EEE_ADV2			0xa6d4
#define	RTL_MDIO_PCS_EEE_ABLE2			0xa6ec

#define RTL_GENERIC_PHYID			0x001cc800
#define RTL_8211FVD_PHYID			0x001cc878
#define RTL_8221B				0x001cc840
#define RTL_8221B_VB_CG				0x001cc849
#define RTL_8221B_VM_CG				0x001cc84a
#define RTL_8251B				0x001cc862
#define RTL_8261C				0x001cc890
#define RTL_8261N				0x001ccaf3
#define RTL_8264				0x001ccaf2
#define RTL_8264B				0x001cc813

/* RTL8211E and RTL8211F support up to three LEDs */
#define RTL8211x_LED_COUNT			3

#define RTL826X_VEND1_SERDES_GLOBAL_CFG		0xc1
#define   RTL826X_VEND1_SERDES_GLOBAL_CFG_HSI_INV	BIT(6)
#define   RTL826X_VEND1_SERDES_GLOBAL_CFG_HSO_INV	BIT(7)

#define RTL826X_VEND1_PKG_MODEL			0x103
#define RTL826X_VEND1_VERSION_ID		0x104

#define RTL826X_VND2_INER			0xA424
#define   RTL826X_VND2_INER_LINK_STATUS		BIT(4)
#define   RTL826X_VND2_INER_PME			BIT(7)

#define RTL826X_VND2_INSR			0xA43A

MODULE_DESCRIPTION("Realtek PHY driver");
MODULE_AUTHOR("Johnson Leung");
MODULE_LICENSE("GPL");

struct rtl821x_priv {
	bool enable_aldps;
	bool disable_clk_out;
	struct clk *clk;
	/* rtl8211f */
	u16 iner;
};

struct rtl822x_priv {
	bool enable_aldps;
};

static int rtl821x_read_page(struct phy_device *phydev)
{
	return __phy_read(phydev, RTL821x_PAGE_SELECT);
}

static int rtl821x_write_page(struct phy_device *phydev, int page)
{
	return __phy_write(phydev, RTL821x_PAGE_SELECT, page);
}

static int rtl821x_read_ext_page(struct phy_device *phydev, u16 ext_page,
				 u32 regnum)
{
	int oldpage, ret = 0;

	oldpage = phy_select_page(phydev, RTL821x_SET_EXT_PAGE);
	if (oldpage >= 0) {
		ret = __phy_write(phydev, RTL821x_EXT_PAGE_SELECT, ext_page);
		if (ret == 0)
			ret = __phy_read(phydev, regnum);
	}

	return phy_restore_page(phydev, oldpage, ret);
}

static int rtl821x_modify_ext_page(struct phy_device *phydev, u16 ext_page,
				   u32 regnum, u16 mask, u16 set)
{
	int oldpage, ret = 0;

	oldpage = phy_select_page(phydev, RTL821x_SET_EXT_PAGE);
	if (oldpage >= 0) {
		ret = __phy_write(phydev, RTL821x_EXT_PAGE_SELECT, ext_page);
		if (ret == 0)
			ret = __phy_modify(phydev, regnum, mask, set);
	}

	return phy_restore_page(phydev, oldpage, ret);
}

static int rtl821x_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	struct rtl821x_priv *priv;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->clk = devm_clk_get_optional_enabled(dev, NULL);
	if (IS_ERR(priv->clk))
		return dev_err_probe(dev, PTR_ERR(priv->clk),
				     "failed to get phy clock\n");

	priv->enable_aldps = of_property_read_bool(dev->of_node,
						   "realtek,aldps-enable");
	priv->disable_clk_out = of_property_read_bool(dev->of_node,
						      "realtek,clkout-disable");

	phydev->priv = priv;

	return 0;
}

static int rtl8211f_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int ret;

	ret = rtl821x_probe(phydev);
	if (ret < 0)
		return ret;

	/* Disable all PME events */
	ret = phy_write_paged(phydev, RTL8211F_WOL_PAGE,
			      RTL8211F_WOL_SETTINGS_EVENTS, 0);
	if (ret < 0)
		return ret;

	/* Mark this PHY as wakeup capable and register the interrupt as a
	 * wakeup IRQ if the PHY is marked as a wakeup source in firmware,
	 * and the interrupt is valid.
	 */
	if (device_property_read_bool(dev, "wakeup-source") &&
	    phy_interrupt_is_valid(phydev)) {
		device_set_wakeup_capable(dev, true);
		devm_pm_set_wake_irq(dev, phydev->irq);
	}

	return ret;
}

static int rtl8201_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL8201F_ISR);

	return (err < 0) ? err : 0;
}

static int rtl821x_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL821x_INSR);

	return (err < 0) ? err : 0;
}

static int rtl8211f_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL8211F_INSR);

	return (err < 0) ? err : 0;
}

static int rtl8201_config_intr(struct phy_device *phydev)
{
	u16 val;
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED) {
		err = rtl8201_ack_interrupt(phydev);
		if (err)
			return err;

		val = BIT(13) | BIT(12) | BIT(11);
		err = phy_write_paged(phydev, 0x7, RTL8201F_IER, val);
	} else {
		val = 0;
		err = phy_write_paged(phydev, 0x7, RTL8201F_IER, val);
		if (err)
			return err;

		err = rtl8201_ack_interrupt(phydev);
	}

	return err;
}

static int rtl8211b_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED) {
		err = rtl821x_ack_interrupt(phydev);
		if (err)
			return err;

		err = phy_write(phydev, RTL821x_INER,
				RTL8211B_INER_INIT);
	} else {
		err = phy_write(phydev, RTL821x_INER, 0);
		if (err)
			return err;

		err = rtl821x_ack_interrupt(phydev);
	}

	return err;
}

static int rtl8211e_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED) {
		err = rtl821x_ack_interrupt(phydev);
		if (err)
			return err;

		err = phy_write(phydev, RTL821x_INER,
				RTL8211E_INER_LINK_STATUS);
	} else {
		err = phy_write(phydev, RTL821x_INER, 0);
		if (err)
			return err;

		err = rtl821x_ack_interrupt(phydev);
	}

	return err;
}

static int rtl8211f_config_intr(struct phy_device *phydev)
{
	struct rtl821x_priv *priv = phydev->priv;
	u16 val;
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED) {
		err = rtl8211f_ack_interrupt(phydev);
		if (err)
			return err;

		val = RTL8211F_INER_LINK_STATUS;
		err = phy_write_paged(phydev, 0xa42, RTL821x_INER, val);
		if (err == 0)
			priv->iner = val;
	} else {
		priv->iner = val = 0;
		err = phy_write_paged(phydev, 0xa42, RTL821x_INER, val);
		if (err)
			return err;

		err = rtl8211f_ack_interrupt(phydev);
	}

	return err;
}

static irqreturn_t rtl8201_handle_interrupt(struct phy_device *phydev)
{
	int irq_status;

	irq_status = phy_read(phydev, RTL8201F_ISR);
	if (irq_status < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	if (!(irq_status & RTL8201F_ISR_MASK))
		return IRQ_NONE;

	phy_trigger_machine(phydev);

	return IRQ_HANDLED;
}

static irqreturn_t rtl821x_handle_interrupt(struct phy_device *phydev)
{
	int irq_status, irq_enabled;

	irq_status = phy_read(phydev, RTL821x_INSR);
	if (irq_status < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	irq_enabled = phy_read(phydev, RTL821x_INER);
	if (irq_enabled < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	if (!(irq_status & irq_enabled))
		return IRQ_NONE;

	phy_trigger_machine(phydev);

	return IRQ_HANDLED;
}

static irqreturn_t rtl8211f_handle_interrupt(struct phy_device *phydev)
{
	int irq_status;

	irq_status = phy_read(phydev, RTL8211F_INSR);
	if (irq_status < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	if (irq_status & RTL8211F_INER_LINK_STATUS) {
		phy_trigger_machine(phydev);
		return IRQ_HANDLED;
	}

	if (irq_status & RTL8211F_INER_PME) {
		pm_wakeup_event(&phydev->mdio.dev, 0);
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static void rtl8211f_get_wol(struct phy_device *dev, struct ethtool_wolinfo *wol)
{
	int wol_events;

	/* If the PHY is not capable of waking the system, then WoL can not
	 * be supported.
	 */
	if (!device_can_wakeup(&dev->mdio.dev)) {
		wol->supported = 0;
		return;
	}

	wol->supported = WAKE_MAGIC;

	wol_events = phy_read_paged(dev, RTL8211F_WOL_PAGE, RTL8211F_WOL_SETTINGS_EVENTS);
	if (wol_events < 0)
		return;

	if (wol_events & RTL8211F_WOL_EVENT_MAGIC)
		wol->wolopts = WAKE_MAGIC;
}

static int rtl8211f_set_wol(struct phy_device *dev, struct ethtool_wolinfo *wol)
{
	const u8 *mac_addr = dev->attached_dev->dev_addr;
	int oldpage;

	if (!device_can_wakeup(&dev->mdio.dev))
		return -EOPNOTSUPP;

	oldpage = phy_save_page(dev);
	if (oldpage < 0)
		goto err;

	if (wol->wolopts & WAKE_MAGIC) {
		/* Store the device address for the magic packet */
		rtl821x_write_page(dev, RTL8211F_PHYSICAL_ADDR_PAGE);
		__phy_write(dev, RTL8211F_PHYSICAL_ADDR_WORD0, mac_addr[1] << 8 | (mac_addr[0]));
		__phy_write(dev, RTL8211F_PHYSICAL_ADDR_WORD1, mac_addr[3] << 8 | (mac_addr[2]));
		__phy_write(dev, RTL8211F_PHYSICAL_ADDR_WORD2, mac_addr[5] << 8 | (mac_addr[4]));

		/* Enable magic packet matching */
		rtl821x_write_page(dev, RTL8211F_WOL_PAGE);
		__phy_write(dev, RTL8211F_WOL_SETTINGS_EVENTS, RTL8211F_WOL_EVENT_MAGIC);
		/* Set the maximum packet size, and assert WoL reset */
		__phy_write(dev, RTL8211F_WOL_RST_RMSQ, RTL8211F_WOL_RMSQ);
	} else {
		/* Disable magic packet matching */
		rtl821x_write_page(dev, RTL8211F_WOL_PAGE);
		__phy_write(dev, RTL8211F_WOL_SETTINGS_EVENTS, 0);

		/* Place WoL in reset */
		__phy_clear_bits(dev, RTL8211F_WOL_RST_RMSQ,
				 RTL8211F_WOL_RG_RSTB);
	}

	device_set_wakeup_enable(&dev->mdio.dev, !!(wol->wolopts & WAKE_MAGIC));

err:
	return phy_restore_page(dev, oldpage, 0);
}

static int rtl8211_config_aneg(struct phy_device *phydev)
{
	int ret;

	ret = genphy_config_aneg(phydev);
	if (ret < 0)
		return ret;

	/* Quirk was copied from vendor driver. Unfortunately it includes no
	 * description of the magic numbers.
	 */
	if (phydev->speed == SPEED_100 && phydev->autoneg == AUTONEG_DISABLE) {
		phy_write(phydev, 0x17, 0x2138);
		phy_write(phydev, 0x0e, 0x0260);
	} else {
		phy_write(phydev, 0x17, 0x2108);
		phy_write(phydev, 0x0e, 0x0000);
	}

	return 0;
}

static int rtl8211c_config_init(struct phy_device *phydev)
{
	/* RTL8211C has an issue when operating in Gigabit slave mode */
	return phy_set_bits(phydev, MII_CTRL1000,
			    CTL1000_ENABLE_MASTER | CTL1000_AS_MASTER);
}

static int rtl8211f_config_rgmii_delay(struct phy_device *phydev)
{
	u16 val_txdly, val_rxdly;
	int ret;

	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_RGMII:
		val_txdly = 0;
		val_rxdly = 0;
		break;

	case PHY_INTERFACE_MODE_RGMII_RXID:
		val_txdly = 0;
		val_rxdly = RTL8211F_RX_DELAY;
		break;

	case PHY_INTERFACE_MODE_RGMII_TXID:
		val_txdly = RTL8211F_TX_DELAY;
		val_rxdly = 0;
		break;

	case PHY_INTERFACE_MODE_RGMII_ID:
		val_txdly = RTL8211F_TX_DELAY;
		val_rxdly = RTL8211F_RX_DELAY;
		break;

	default: /* the rest of the modes imply leaving delay as is. */
		return 0;
	}

	ret = phy_modify_paged_changed(phydev, RTL8211F_RGMII_PAGE,
				       RTL8211F_TXCR, RTL8211F_TX_DELAY,
				       val_txdly);
	if (ret < 0) {
		phydev_err(phydev, "Failed to update the TX delay register: %pe\n",
			   ERR_PTR(ret));
		return ret;
	} else if (ret) {
		phydev_dbg(phydev,
			   "%s 2ns TX delay (and changing the value from pin-strapping RXD1 or the bootloader)\n",
			   str_enable_disable(val_txdly));
	} else {
		phydev_dbg(phydev,
			   "2ns TX delay was already %s (by pin-strapping RXD1 or bootloader configuration)\n",
			   str_enabled_disabled(val_txdly));
	}

	ret = phy_modify_paged_changed(phydev, RTL8211F_RGMII_PAGE,
				       RTL8211F_RXCR, RTL8211F_RX_DELAY,
				       val_rxdly);
	if (ret < 0) {
		phydev_err(phydev, "Failed to update the RX delay register: %pe\n",
			   ERR_PTR(ret));
		return ret;
	} else if (ret) {
		phydev_dbg(phydev,
			   "%s 2ns RX delay (and changing the value from pin-strapping RXD0 or the bootloader)\n",
			   str_enable_disable(val_rxdly));
	} else {
		phydev_dbg(phydev,
			   "2ns RX delay was already %s (by pin-strapping RXD0 or bootloader configuration)\n",
			   str_enabled_disabled(val_rxdly));
	}

	return 0;
}

static int rtl8211f_config_clk_out(struct phy_device *phydev)
{
	struct rtl821x_priv *priv = phydev->priv;
	int ret;

	/* The value is preserved if the device tree property is absent */
	if (!priv->disable_clk_out)
		return 0;

	if (phydev->drv->phy_id == RTL_8211FVD_PHYID)
		ret = phy_modify_paged(phydev, RTL8211FVD_CLKOUT_PAGE,
				       RTL8211FVD_CLKOUT_REG,
				       RTL8211FVD_CLKOUT_EN, 0);
	else
		ret = phy_modify(phydev, RTL8211F_PHYCR2, RTL8211F_CLKOUT_EN,
				 0);
	if (ret)
		return ret;

	return genphy_soft_reset(phydev);
}

/* Advance Link Down Power Saving (ALDPS) mode changes crystal/clock behaviour,
 * which causes the RXC clock signal to stop for tens to hundreds of
 * milliseconds.
 *
 * Some MACs need the RXC clock to support their internal RX logic, so ALDPS is
 * only enabled based on an opt-in device tree property.
 */
static int rtl8211f_config_aldps(struct phy_device *phydev)
{
	struct rtl821x_priv *priv = phydev->priv;
	u16 mask = RTL8211F_ALDPS_PLL_OFF |
		   RTL8211F_ALDPS_ENABLE |
		   RTL8211F_ALDPS_XTAL_OFF;

	/* The value is preserved if the device tree property is absent */
	if (!priv->enable_aldps)
		return 0;

	return phy_modify(phydev, RTL8211F_PHYCR1, mask, mask);
}

static int rtl8211f_config_phy_eee(struct phy_device *phydev)
{
	/* Disable PHY-mode EEE so LPI is passed to the MAC */
	return phy_modify(phydev, RTL8211F_PHYCR2,
			  RTL8211F_PHYCR2_PHY_EEE_ENABLE, 0);
}

static int rtl8211f_config_init(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int ret;

	ret = rtl8211f_config_aldps(phydev);
	if (ret) {
		dev_err(dev, "aldps mode configuration failed: %pe\n",
			ERR_PTR(ret));
		return ret;
	}

	ret = rtl8211f_config_rgmii_delay(phydev);
	if (ret)
		return ret;

	ret = rtl8211f_config_clk_out(phydev);
	if (ret) {
		dev_err(dev, "clkout configuration failed: %pe\n",
			ERR_PTR(ret));
		return ret;
	}

	return rtl8211f_config_phy_eee(phydev);
}

static int rtl821x_suspend(struct phy_device *phydev)
{
	struct rtl821x_priv *priv = phydev->priv;
	int ret = 0;

	if (!phydev->wol_enabled) {
		ret = genphy_suspend(phydev);

		if (ret)
			return ret;

		clk_disable_unprepare(priv->clk);
	}

	return ret;
}

static int rtl8211f_suspend(struct phy_device *phydev)
{
	u16 wol_rst;
	int ret;

	ret = rtl821x_suspend(phydev);
	if (ret < 0)
		return ret;

	/* If a PME event is enabled, then configure the interrupt for
	 * PME events only, disabling link interrupt. We avoid switching
	 * to PMEB mode as we don't have a status bit for that.
	 */
	if (device_may_wakeup(&phydev->mdio.dev)) {
		ret = phy_write_paged(phydev, 0xa42, RTL821x_INER,
				      RTL8211F_INER_PME);
		if (ret < 0)
			goto err;

		/* Read the INSR to clear any pending interrupt */
		phy_read(phydev, RTL8211F_INSR);

		/* Reset the WoL to ensure that an event is picked up.
		 * Unless we do this, even if we receive another packet,
		 * we may not have a PME interrupt raised.
		 */
		ret = phy_read_paged(phydev, RTL8211F_WOL_PAGE,
				     RTL8211F_WOL_RST_RMSQ);
		if (ret < 0)
			goto err;

		wol_rst = ret & ~RTL8211F_WOL_RG_RSTB;
		ret = phy_write_paged(phydev, RTL8211F_WOL_PAGE,
				      RTL8211F_WOL_RST_RMSQ, wol_rst);
		if (ret < 0)
			goto err;

		wol_rst |= RTL8211F_WOL_RG_RSTB;
		ret = phy_write_paged(phydev, RTL8211F_WOL_PAGE,
				      RTL8211F_WOL_RST_RMSQ, wol_rst);
	}

err:
	return ret;
}

static int rtl821x_resume(struct phy_device *phydev)
{
	struct rtl821x_priv *priv = phydev->priv;
	int ret;

	if (!phydev->wol_enabled)
		clk_prepare_enable(priv->clk);

	ret = genphy_resume(phydev);
	if (ret < 0)
		return ret;

	msleep(20);

	return 0;
}

static int rtl8211f_resume(struct phy_device *phydev)
{
	struct rtl821x_priv *priv = phydev->priv;
	int ret;

	ret = rtl821x_resume(phydev);
	if (ret < 0)
		return ret;

	/* If the device was programmed for a PME event, restore the interrupt
	 * enable so phylib can receive link state interrupts.
	 */
	if (device_may_wakeup(&phydev->mdio.dev))
		ret = phy_write_paged(phydev, 0xa42, RTL821x_INER, priv->iner);

	return ret;
}

static int rtl8211x_led_hw_is_supported(struct phy_device *phydev, u8 index,
					unsigned long rules)
{
	const unsigned long mask = BIT(TRIGGER_NETDEV_LINK) |
				   BIT(TRIGGER_NETDEV_LINK_10) |
				   BIT(TRIGGER_NETDEV_LINK_100) |
				   BIT(TRIGGER_NETDEV_LINK_1000) |
				   BIT(TRIGGER_NETDEV_RX) |
				   BIT(TRIGGER_NETDEV_TX);

	/* The RTL8211F PHY supports these LED settings on up to three LEDs:
	 * - Link: Configurable subset of 10/100/1000 link rates
	 * - Active: Blink on activity, RX or TX is not differentiated
	 * The Active option has two modes, A and B:
	 * - A: Link and Active indication at configurable, but matching,
	 *      subset of 10/100/1000 link rates
	 * - B: Link indication at configurable subset of 10/100/1000 link
	 *      rates and Active indication always at all three 10+100+1000
	 *      link rates.
	 * This code currently uses mode B only.
	 *
	 * RTL8211E PHY LED has one mode, which works like RTL8211F mode B.
	 */

	if (index >= RTL8211x_LED_COUNT)
		return -EINVAL;

	/* Filter out any other unsupported triggers. */
	if (rules & ~mask)
		return -EOPNOTSUPP;

	/* RX and TX are not differentiated, either both are set or not set. */
	if (!(rules & BIT(TRIGGER_NETDEV_RX)) ^ !(rules & BIT(TRIGGER_NETDEV_TX)))
		return -EOPNOTSUPP;

	return 0;
}

static int rtl8211f_led_hw_control_get(struct phy_device *phydev, u8 index,
				       unsigned long *rules)
{
	int val;

	if (index >= RTL8211x_LED_COUNT)
		return -EINVAL;

	val = phy_read_paged(phydev, 0xd04, RTL8211F_LEDCR);
	if (val < 0)
		return val;

	val >>= RTL8211F_LEDCR_SHIFT * index;
	val &= RTL8211F_LEDCR_MASK;

	if (val & RTL8211F_LEDCR_LINK_10)
		__set_bit(TRIGGER_NETDEV_LINK_10, rules);

	if (val & RTL8211F_LEDCR_LINK_100)
		__set_bit(TRIGGER_NETDEV_LINK_100, rules);

	if (val & RTL8211F_LEDCR_LINK_1000)
		__set_bit(TRIGGER_NETDEV_LINK_1000, rules);

	if ((val & RTL8211F_LEDCR_LINK_10) &&
	    (val & RTL8211F_LEDCR_LINK_100) &&
	    (val & RTL8211F_LEDCR_LINK_1000)) {
		__set_bit(TRIGGER_NETDEV_LINK, rules);
	}

	if (val & RTL8211F_LEDCR_ACT_TXRX) {
		__set_bit(TRIGGER_NETDEV_RX, rules);
		__set_bit(TRIGGER_NETDEV_TX, rules);
	}

	return 0;
}

static int rtl8211f_led_hw_control_set(struct phy_device *phydev, u8 index,
				       unsigned long rules)
{
	const u16 mask = RTL8211F_LEDCR_MASK << (RTL8211F_LEDCR_SHIFT * index);
	u16 reg = 0;

	if (index >= RTL8211x_LED_COUNT)
		return -EINVAL;

	if (test_bit(TRIGGER_NETDEV_LINK, &rules) ||
	    test_bit(TRIGGER_NETDEV_LINK_10, &rules)) {
		reg |= RTL8211F_LEDCR_LINK_10;
	}

	if (test_bit(TRIGGER_NETDEV_LINK, &rules) ||
	    test_bit(TRIGGER_NETDEV_LINK_100, &rules)) {
		reg |= RTL8211F_LEDCR_LINK_100;
	}

	if (test_bit(TRIGGER_NETDEV_LINK, &rules) ||
	    test_bit(TRIGGER_NETDEV_LINK_1000, &rules)) {
		reg |= RTL8211F_LEDCR_LINK_1000;
	}

	if (test_bit(TRIGGER_NETDEV_RX, &rules) ||
	    test_bit(TRIGGER_NETDEV_TX, &rules)) {
		reg |= RTL8211F_LEDCR_ACT_TXRX;
	}

	reg <<= RTL8211F_LEDCR_SHIFT * index;
	reg |= RTL8211F_LEDCR_MODE;	 /* Mode B */

	return phy_modify_paged(phydev, 0xd04, RTL8211F_LEDCR, mask, reg);
}

static int rtl8211e_led_hw_control_get(struct phy_device *phydev, u8 index,
				       unsigned long *rules)
{
	int ret;
	u16 cr1, cr2;

	if (index >= RTL8211x_LED_COUNT)
		return -EINVAL;

	ret = rtl821x_read_ext_page(phydev, RTL8211E_LEDCR_EXT_PAGE,
				    RTL8211E_LEDCR1);
	if (ret < 0)
		return ret;

	cr1 = ret >> RTL8211E_LEDCR1_SHIFT * index;
	if (cr1 & RTL8211E_LEDCR1_ACT_TXRX) {
		__set_bit(TRIGGER_NETDEV_RX, rules);
		__set_bit(TRIGGER_NETDEV_TX, rules);
	}

	ret = rtl821x_read_ext_page(phydev, RTL8211E_LEDCR_EXT_PAGE,
				    RTL8211E_LEDCR2);
	if (ret < 0)
		return ret;

	cr2 = ret >> RTL8211E_LEDCR2_SHIFT * index;
	if (cr2 & RTL8211E_LEDCR2_LINK_10)
		__set_bit(TRIGGER_NETDEV_LINK_10, rules);

	if (cr2 & RTL8211E_LEDCR2_LINK_100)
		__set_bit(TRIGGER_NETDEV_LINK_100, rules);

	if (cr2 & RTL8211E_LEDCR2_LINK_1000)
		__set_bit(TRIGGER_NETDEV_LINK_1000, rules);

	if ((cr2 & RTL8211E_LEDCR2_LINK_10) &&
	    (cr2 & RTL8211E_LEDCR2_LINK_100) &&
	    (cr2 & RTL8211E_LEDCR2_LINK_1000)) {
		__set_bit(TRIGGER_NETDEV_LINK, rules);
	}

	return ret;
}

static int rtl8211e_led_hw_control_set(struct phy_device *phydev, u8 index,
				       unsigned long rules)
{
	const u16 cr1mask =
		RTL8211E_LEDCR1_MASK << (RTL8211E_LEDCR1_SHIFT * index);
	const u16 cr2mask =
		RTL8211E_LEDCR2_MASK << (RTL8211E_LEDCR2_SHIFT * index);
	u16 cr1 = 0, cr2 = 0;
	int ret;

	if (index >= RTL8211x_LED_COUNT)
		return -EINVAL;

	if (test_bit(TRIGGER_NETDEV_RX, &rules) ||
	    test_bit(TRIGGER_NETDEV_TX, &rules)) {
		cr1 |= RTL8211E_LEDCR1_ACT_TXRX;
	}

	cr1 <<= RTL8211E_LEDCR1_SHIFT * index;
	ret = rtl821x_modify_ext_page(phydev, RTL8211E_LEDCR_EXT_PAGE,
				      RTL8211E_LEDCR1, cr1mask, cr1);
	if (ret < 0)
		return ret;

	if (test_bit(TRIGGER_NETDEV_LINK, &rules) ||
	    test_bit(TRIGGER_NETDEV_LINK_10, &rules)) {
		cr2 |= RTL8211E_LEDCR2_LINK_10;
	}

	if (test_bit(TRIGGER_NETDEV_LINK, &rules) ||
	    test_bit(TRIGGER_NETDEV_LINK_100, &rules)) {
		cr2 |= RTL8211E_LEDCR2_LINK_100;
	}

	if (test_bit(TRIGGER_NETDEV_LINK, &rules) ||
	    test_bit(TRIGGER_NETDEV_LINK_1000, &rules)) {
		cr2 |= RTL8211E_LEDCR2_LINK_1000;
	}

	cr2 <<= RTL8211E_LEDCR2_SHIFT * index;
	ret = rtl821x_modify_ext_page(phydev, RTL8211E_LEDCR_EXT_PAGE,
				      RTL8211E_LEDCR2, cr2mask, cr2);

	return ret;
}

static int rtl8211e_config_init(struct phy_device *phydev)
{
	u16 val;

	/* enable TX/RX delay for rgmii-* modes, and disable them for rgmii. */
	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_RGMII:
		val = RTL8211E_CTRL_DELAY | 0;
		break;
	case PHY_INTERFACE_MODE_RGMII_ID:
		val = RTL8211E_CTRL_DELAY | RTL8211E_TX_DELAY | RTL8211E_RX_DELAY;
		break;
	case PHY_INTERFACE_MODE_RGMII_RXID:
		val = RTL8211E_CTRL_DELAY | RTL8211E_RX_DELAY;
		break;
	case PHY_INTERFACE_MODE_RGMII_TXID:
		val = RTL8211E_CTRL_DELAY | RTL8211E_TX_DELAY;
		break;
	default: /* the rest of the modes imply leaving delays as is. */
		return 0;
	}

	/* According to a sample driver there is a 0x1c config register on the
	 * 0xa4 extension page (0x7) layout. It can be used to disable/enable
	 * the RX/TX delays otherwise controlled by RXDLY/TXDLY pins.
	 * The configuration register definition:
	 * 14 = reserved
	 * 13 = Force Tx RX Delay controlled by bit12 bit11,
	 * 12 = RX Delay, 11 = TX Delay
	 * 10:0 = Test && debug settings reserved by realtek
	 */
	return rtl821x_modify_ext_page(phydev, RTL8211E_RGMII_EXT_PAGE,
				       RTL8211E_RGMII_DELAY,
				       RTL8211E_DELAY_MASK, val);
}

static int rtl8211b_suspend(struct phy_device *phydev)
{
	phy_write(phydev, MII_MMD_DATA, BIT(9));

	return genphy_suspend(phydev);
}

static int rtl8211b_resume(struct phy_device *phydev)
{
	phy_write(phydev, MII_MMD_DATA, 0);

	return genphy_resume(phydev);
}

static int rtl8366rb_config_init(struct phy_device *phydev)
{
	int ret;

	ret = phy_set_bits(phydev, RTL8366RB_POWER_SAVE,
			   RTL8366RB_POWER_SAVE_ON);
	if (ret) {
		dev_err(&phydev->mdio.dev,
			"error enabling power management\n");
	}

	return ret;
}

/* get actual speed to cover the downshift case */
static void rtlgen_decode_physr(struct phy_device *phydev, int val)
{
	/* bit 3
	 * 0: Half Duplex
	 * 1: Full Duplex
	 */
	if (val & RTL_PHYSR_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	switch (val & RTL_PHYSR_SPEED_MASK) {
	case 0x0000:
		phydev->speed = SPEED_10;
		break;
	case 0x0010:
		phydev->speed = SPEED_100;
		break;
	case 0x0020:
		phydev->speed = SPEED_1000;
		break;
	case 0x0200:
		phydev->speed = SPEED_10000;
		break;
	case 0x0210:
		phydev->speed = SPEED_2500;
		break;
	case 0x0220:
		phydev->speed = SPEED_5000;
		break;
	default:
		break;
	}

	/* bit 11
	 * 0: Slave Mode
	 * 1: Master Mode
	 */
	if (phydev->speed >= 1000) {
		if (val & RTL_PHYSR_MASTER)
			phydev->master_slave_state = MASTER_SLAVE_STATE_MASTER;
		else
			phydev->master_slave_state = MASTER_SLAVE_STATE_SLAVE;
	} else {
		phydev->master_slave_state = MASTER_SLAVE_STATE_UNSUPPORTED;
	}
}

static int rtlgen_read_status(struct phy_device *phydev)
{
	int ret, val;

	ret = genphy_read_status(phydev);
	if (ret < 0)
		return ret;

	if (!phydev->link)
		return 0;

	val = phy_read(phydev, RTL_PHYSR);
	if (val < 0)
		return val;

	rtlgen_decode_physr(phydev, val);

	return 0;
}

static int rtlgen_read_vend2(struct phy_device *phydev, int regnum)
{
	return __mdiobus_c45_read(phydev->mdio.bus, 0, MDIO_MMD_VEND2, regnum);
}

static int rtlgen_write_vend2(struct phy_device *phydev, int regnum, u16 val)
{
	return __mdiobus_c45_write(phydev->mdio.bus, 0, MDIO_MMD_VEND2, regnum,
				   val);
}

static int rtlgen_read_mmd(struct phy_device *phydev, int devnum, u16 regnum)
{
	int ret;

	if (devnum == MDIO_MMD_VEND2)
		ret = rtlgen_read_vend2(phydev, regnum);
	else if (devnum == MDIO_MMD_PCS && regnum == MDIO_PCS_EEE_ABLE)
		ret = rtlgen_read_vend2(phydev, RTL_MDIO_PCS_EEE_ABLE);
	else if (devnum == MDIO_MMD_AN && regnum == MDIO_AN_EEE_ADV)
		ret = rtlgen_read_vend2(phydev, RTL_MDIO_AN_EEE_ADV);
	else if (devnum == MDIO_MMD_AN && regnum == MDIO_AN_EEE_LPABLE)
		ret = rtlgen_read_vend2(phydev, RTL_MDIO_AN_EEE_LPABLE);
	else
		ret = -EOPNOTSUPP;

	return ret;
}

static int rtlgen_write_mmd(struct phy_device *phydev, int devnum, u16 regnum,
			    u16 val)
{
	int ret;

	if (devnum == MDIO_MMD_VEND2)
		ret = rtlgen_write_vend2(phydev, regnum, val);
	else if (devnum == MDIO_MMD_AN && regnum == MDIO_AN_EEE_ADV)
		ret = rtlgen_write_vend2(phydev, regnum, RTL_MDIO_AN_EEE_ADV);
	else
		ret = -EOPNOTSUPP;

	return ret;
}

static int rtl822x_read_mmd(struct phy_device *phydev, int devnum, u16 regnum)
{
	int ret = rtlgen_read_mmd(phydev, devnum, regnum);

	if (ret != -EOPNOTSUPP)
		return ret;

	if (devnum == MDIO_MMD_PCS && regnum == MDIO_PCS_EEE_ABLE2)
		ret = rtlgen_read_vend2(phydev, RTL_MDIO_PCS_EEE_ABLE2);
	else if (devnum == MDIO_MMD_AN && regnum == MDIO_AN_EEE_ADV2)
		ret = rtlgen_read_vend2(phydev, RTL_MDIO_AN_EEE_ADV2);
	else if (devnum == MDIO_MMD_AN && regnum == MDIO_AN_EEE_LPABLE2)
		ret = rtlgen_read_vend2(phydev, RTL_MDIO_AN_EEE_LPABLE2);

	return ret;
}

static int rtl822x_write_mmd(struct phy_device *phydev, int devnum, u16 regnum,
			     u16 val)
{
	int ret = rtlgen_write_mmd(phydev, devnum, regnum, val);

	if (ret != -EOPNOTSUPP)
		return ret;

	if (devnum == MDIO_MMD_AN && regnum == MDIO_AN_EEE_ADV2)
		ret = rtlgen_write_vend2(phydev, RTL_MDIO_AN_EEE_ADV2, val);

	return ret;
}

static int rtl822x_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	struct rtl822x_priv *priv;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->enable_aldps = of_property_read_bool(dev->of_node,
						   "realtek,aldps-enable");

	phydev->priv = priv;

	if (IS_ENABLED(CONFIG_REALTEK_PHY_HWMON) &&
	    phydev->phy_id != RTL_GENERIC_PHYID)
		return rtl822x_hwmon_init(phydev);

	return 0;
}

/* RTL822x cannot access MDIO_MMD_VEND2 via MII_MMD_CTRL/MII_MMD_DATA.
 * A mapping to use paged access needs to be used instead.
 * All other MMD devices can be accessed as usual.
 */
static int rtl822xb_read_mmd(struct phy_device *phydev, int devnum, u16 reg)
{
	int oldpage, ret, read_ret;
	u16 page;

	/* Use default method for all MMDs except MDIO_MMD_VEND2 or in case
	 * Clause-45 access is available
	 */
	if (devnum != MDIO_MMD_VEND2 || phydev->is_c45)
		return mmd_phy_read(phydev->mdio.bus, phydev->mdio.addr,
				    phydev->is_c45, devnum, reg);

	/* Simplify access to C22-registers addressed inside MDIO_MMD_VEND2 */
	if (reg >= RTL822X_VND2_C22_REG(0) &&
	    reg <= RTL822X_VND2_C22_REG(30))
		return __phy_read(phydev, RTL822X_VND2_TO_C22_REG(reg));

	/* Use paged access for MDIO_MMD_VEND2 over Clause-22 */
	page = RTL822X_VND2_TO_PAGE(reg);
	oldpage = __phy_read(phydev, RTL821x_PAGE_SELECT);
	if (oldpage < 0)
		return oldpage;

	if (oldpage != page) {
		ret = __phy_write(phydev, RTL821x_PAGE_SELECT, page);
		if (ret < 0)
			return ret;
	}

	read_ret = __phy_read(phydev, RTL822X_VND2_TO_PAGE_REG(reg));
	if (oldpage != page) {
		ret = __phy_write(phydev, RTL821x_PAGE_SELECT, oldpage);
		if (ret < 0)
			return ret;
	}

	return read_ret;
}

static int rtl822xb_write_mmd(struct phy_device *phydev, int devnum, u16 reg,
			      u16 val)
{
	int oldpage, ret, write_ret;
	u16 page;

	/* Use default method for all MMDs except MDIO_MMD_VEND2 or in case
	 * Clause-45 access is available
	 */
	if (devnum != MDIO_MMD_VEND2 || phydev->is_c45)
		return mmd_phy_write(phydev->mdio.bus, phydev->mdio.addr,
				     phydev->is_c45, devnum, reg, val);

	/* Simplify access to C22-registers addressed inside MDIO_MMD_VEND2 */
	if (reg >= RTL822X_VND2_C22_REG(0) &&
	    reg <= RTL822X_VND2_C22_REG(30))
		return __phy_write(phydev, RTL822X_VND2_TO_C22_REG(reg), val);

	/* Use paged access for MDIO_MMD_VEND2 over Clause-22 */
	page = RTL822X_VND2_TO_PAGE(reg);
	oldpage = __phy_read(phydev, RTL821x_PAGE_SELECT);
	if (oldpage < 0)
		return oldpage;

	if (oldpage != page) {
		ret = __phy_write(phydev, RTL821x_PAGE_SELECT, page);
		if (ret < 0)
			return ret;
	}

	write_ret = __phy_write(phydev,  RTL822X_VND2_TO_PAGE_REG(reg), val);
	if (oldpage != page) {
		ret = __phy_write(phydev, RTL821x_PAGE_SELECT, oldpage);
		if (ret < 0)
			return ret;
	}

	return write_ret;
}

static int rtl822x_init_phycr1(struct phy_device *phydev, bool no_aldps)
{
	struct rtl822x_priv *priv = phydev->priv;
	u16 mask = RTL8221B_PHYCR1_ALDPS_EN | RTL8221B_PHYCR1_ALDPS_XTAL_OFF_EN;
	u16 val = 0;

	/* The controller in some SFP modules uses MDIO address 0 to access the
	 * PHY. Leave the MDIO broadcast configuration bit alone for SFP
	 * modules, as it won't cause any issues there anyways.
	 */
	if (!phydev->is_on_sfp_module)
		mask |= RTL8221B_PHYCR1_PHYAD_0_EN;

	if (priv->enable_aldps && !no_aldps)
		val = RTL8221B_PHYCR1_ALDPS_EN | RTL8221B_PHYCR1_ALDPS_XTAL_OFF_EN;

	return phy_modify_mmd_changed(phydev, MDIO_MMD_VEND2, RTL8221B_PHYCR1,
				      mask, val);
}

static int rtl8226_set_mdi_swap(struct phy_device *phydev, bool swap_enable)
{
	u16 val = swap_enable ? RTL8226_VND1_UNKNOWN_6A21_MDI_SWAP_EN : 0;

	return phy_modify_mmd(phydev, MDIO_MMD_VEND1, RTL8226_VND1_UNKNOWN_6A21,
			      RTL8226_VND1_UNKNOWN_6A21_MDI_SWAP_EN, val);
}

static int rtl8226_swap_rg_lpf_cap(struct phy_device *phydev, u32 reg_p0_p1, u32 reg_p2_p3)
{
	u16 val_p0, val_p1, val_p2, val_p3;
	int ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND2, reg_p0_p1);
	if (ret < 0)
		return ret;

	val_p0 = FIELD_GET(RTL8226_RG_LPF_CAP_PAIR_A_MASK, ret);
	val_p1 = FIELD_GET(RTL8226_RG_LPF_CAP_PAIR_B_MASK, ret);

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND2, reg_p2_p3);
	if (ret < 0)
		return ret;

	val_p2 = FIELD_GET(RTL8226_RG_LPF_CAP_PAIR_A_MASK, ret);
	val_p3 = FIELD_GET(RTL8226_RG_LPF_CAP_PAIR_B_MASK, ret);

	ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, reg_p0_p1,
			     RTL8226_RG_LPF_CAP_PAIR_A_MASK | RTL8226_RG_LPF_CAP_PAIR_B_MASK,
			     FIELD_PREP(RTL8226_RG_LPF_CAP_PAIR_A_MASK, val_p3) |
			     FIELD_PREP(RTL8226_RG_LPF_CAP_PAIR_B_MASK, val_p2));
	if (ret < 0)
		return ret;

	return phy_modify_mmd(phydev, MDIO_MMD_VEND2, reg_p2_p3,
			     RTL8226_RG_LPF_CAP_PAIR_A_MASK | RTL8226_RG_LPF_CAP_PAIR_B_MASK,
			     FIELD_PREP(RTL8226_RG_LPF_CAP_PAIR_A_MASK, val_p1) |
			     FIELD_PREP(RTL8226_RG_LPF_CAP_PAIR_B_MASK, val_p0));
}

static int rtl8226_patch_mdi_swap(struct phy_device *phydev, bool swap_enable)
{
	u16 adccal_offset[4];
	bool is_patched;
	int ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL8226_VND2_UNKNOWN_D068);
	if (ret < 0)
		return ret;

	is_patched = !(ret & RTL8226_VND2_UNKNOWN_D068_MDI_SWAP_FLAG);

	if (is_patched == swap_enable) {
		/* Nothing to do */
		return 0;
	}

	if (!swap_enable) {
		/* Patching is only implemented one-way, see next comment. */
		phydev_err(phydev, "MDI swapping disabled, but PHY is already patched.\n");
		return -EINVAL;
	}

	/* The exact meaning of these bits is unknown. We only know that bit 1
	 * is used as a flag that swapping is already done.
	 */
	ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, RTL8226_VND2_UNKNOWN_D068, 0x7, 0x1);
	if (ret < 0)
		return ret;

	for (int i = 0; i < 4; i++) {
		ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, RTL8226_VND2_UNKNOWN_D068,
				     RTL8226_VND2_UNKNOWN_D068_PAIR_SEL,
				     FIELD_PREP(RTL8226_VND2_UNKNOWN_D068_PAIR_SEL, i));
		if (ret < 0)
			return ret;

		ret = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL8226_VND2_ADCCAL_OFFSET);
		if (ret < 0)
			return ret;

		adccal_offset[i] = ret;
	}

	for (int i = 0; i < 4; i++) {
		ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, RTL8226_VND2_UNKNOWN_D068,
				     RTL8226_VND2_UNKNOWN_D068_PAIR_SEL,
				     FIELD_PREP(RTL8226_VND2_UNKNOWN_D068_PAIR_SEL, i));
		if (ret < 0)
			return ret;

		ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, RTL8226_VND2_ADCCAL_OFFSET,
				    adccal_offset[3 - i]);
		if (ret < 0)
			return ret;
	}

	ret = rtl8226_swap_rg_lpf_cap(phydev, RTL8226_VND2_RG_LPF_CAP_XG_P0_P1,
				      RTL8226_VND2_RG_LPF_CAP_XG_P2_P3);
	if (ret < 0)
		return ret;

	return rtl8226_swap_rg_lpf_cap(phydev, RTL8226_VND2_RG_LPF_CAP_P0_P1,
				       RTL8226_VND2_RG_LPF_CAP_P2_P3);
}

static int rtl8226_config_mdi_order(struct phy_device *phydev)
{
	u32 order;
	bool swap_enable;
	int ret;

	ret = of_property_read_u32(phydev->mdio.dev.of_node, "enet-phy-pair-order", &order);

	/* Property not present, nothing to do */
	if (ret == -EINVAL || ret == -ENOSYS)
		return 0;

	if (ret)
		return ret;

	if (order & ~1)
		return -EINVAL;

	swap_enable = !!(order & 1);

	ret = rtl8226_set_mdi_swap(phydev, swap_enable);
	if (ret)
		return ret;

	return rtl8226_patch_mdi_swap(phydev, swap_enable);
}

static int rtl8226_probe(struct phy_device *phydev)
{
	return rtl8226_config_mdi_order(phydev);
}

static int rtl822x_set_serdes_option_mode(struct phy_device *phydev, bool gen1)
{
	bool has_2500, has_sgmii, has_hsgmii;
	u16 mode;
	int ret;

	has_2500 = test_bit(PHY_INTERFACE_MODE_2500BASEX,
			    phydev->host_interfaces) ||
		   phydev->interface == PHY_INTERFACE_MODE_2500BASEX;

	has_sgmii = test_bit(PHY_INTERFACE_MODE_SGMII,
			     phydev->host_interfaces) ||
		    phydev->interface == PHY_INTERFACE_MODE_SGMII;

	has_hsgmii = test_bit(PHY_INTERFACE_MODE_HSGMII,
			     phydev->host_interfaces) ||
		    phydev->interface == PHY_INTERFACE_MODE_HSGMII;

	/* fill in possible interfaces */
	__assign_bit(PHY_INTERFACE_MODE_2500BASEX, phydev->possible_interfaces,
		     has_2500);
	__assign_bit(PHY_INTERFACE_MODE_SGMII, phydev->possible_interfaces,
		     has_sgmii);

	__assign_bit(PHY_INTERFACE_MODE_HSGMII, phydev->possible_interfaces,
		     has_hsgmii);

	if (!has_2500 && !has_sgmii && !has_hsgmii)
		return 0;

	/* determine SerDes option mode */
	if (has_2500 && !has_sgmii && !has_hsgmii) {
		mode = RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX;
		phydev->rate_matching = RATE_MATCH_PAUSE;
	} else if (has_2500 && has_sgmii && !has_hsgmii) {
		mode = RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX_SGMII_HSGMII;
		phydev->rate_matching = RATE_MATCH_PAUSE;
	} else if (has_sgmii && has_hsgmii) {
		mode = RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX_SGMII_HSGMII;
		phydev->rate_matching = RATE_MATCH_PAUSE;
	} else if (has_hsgmii) {
		mode = RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX_HSGMII;
		phydev->rate_matching = RATE_MATCH_PAUSE;
	} else if (has_2500) {
		mode = RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX;
		phydev->rate_matching = RATE_MATCH_PAUSE;
	} else if (has_sgmii) {
		mode = RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX_SGMII_HSGMII;
		phydev->rate_matching = RATE_MATCH_PAUSE;
	} else {
		mode = RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX_SGMII_HSGMII;
		phydev->rate_matching = RATE_MATCH_NONE;
	}
	/* the following sequence with magic numbers sets up the SerDes
	 * option mode
	 */

	if (!gen1) {
		ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x75f3, 0);
		if (ret < 0)
			return ret;
	}

	ret = phy_modify_mmd_changed(phydev, MDIO_MMD_VEND1,
				     RTL822X_VND1_SERDES_OPTION,
				     RTL822X_VND1_SERDES_OPTION_MODE_MASK,
				     mode);
	if (gen1 || ret < 0)
		return ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x6a04, 0x0503);
	if (ret < 0)
		return ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x6f10, 0xd455);
	if (ret < 0)
		return ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x6f11, 0x8020);
	if (ret < 0)
		return ret;

	ret = rtl822x_init_phycr1(phydev, false);
	if (ret < 0)
		return ret;

	return 0;
}

static int rtl822x_config_init(struct phy_device *phydev)
{
	return rtl822x_set_serdes_option_mode(phydev, true);
}

static int rtl822xb_config_init(struct phy_device *phydev)
{
	return rtl822x_set_serdes_option_mode(phydev, false);
}

static int rtl822x_serdes_write(struct phy_device *phydev, u16 reg, u16 val)
{
	int ret, poll;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, RTL822X_VND1_SERDES_ADDR, reg);
	if (ret < 0)
		return ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, RTL822X_VND1_SERDES_DATA, val);
	if (ret < 0)
		return ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, RTL822X_VND1_SERDES_CMD,
			    RTL822X_VND1_SERDES_CMD_WRITE |
			    RTL822X_VND1_SERDES_CMD_BUSY);
	if (ret < 0)
		return ret;

	return phy_read_mmd_poll_timeout(phydev, MDIO_MMD_VEND1,
					 RTL822X_VND1_SERDES_CMD, poll,
					 !(poll & RTL822X_VND1_SERDES_CMD_BUSY),
					 500, 100000, false);
}

static int rtl822x_config_inband(struct phy_device *phydev, unsigned int modes)
{
	return rtl822x_serdes_write(phydev, RTL822X_VND1_SERDES_ADDR_AUTONEG,
				    (modes != LINK_INBAND_DISABLE) ?
				    RTL822X_VND1_SERDES_INBAND_ENABLE :
				    RTL822X_VND1_SERDES_INBAND_DISABLE);
}

static unsigned int rtl822x_inband_caps(struct phy_device *phydev,
					phy_interface_t interface)
{
	switch (interface) {
	case PHY_INTERFACE_MODE_2500BASEX:
		return LINK_INBAND_DISABLE;
	case PHY_INTERFACE_MODE_SGMII:
		return LINK_INBAND_DISABLE | LINK_INBAND_ENABLE;
	default:
		return 0;
	}
}

static int rtl822xb_get_rate_matching(struct phy_device *phydev,
				      phy_interface_t iface)
{
	int val;

	/* Only rate matching at 2500base-x */
	if (iface != PHY_INTERFACE_MODE_2500BASEX)
		return RATE_MATCH_NONE;

	val = phy_read_mmd(phydev, MDIO_MMD_VEND1, RTL822X_VND1_SERDES_OPTION);
	if (val < 0)
		return val;

	if ((val & RTL822X_VND1_SERDES_OPTION_MODE_MASK) ==
	    RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX)
		return RATE_MATCH_PAUSE;

	/* RTL822X_VND1_SERDES_OPTION_MODE_2500BASEX_SGMII */
	return RATE_MATCH_NONE;
}

static int rtl822x_get_features(struct phy_device *phydev)
{
	int val;

	val = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL_MDIO_PMA_SPEED);
	if (val < 0)
		return val;

	linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
			 phydev->supported, val & MDIO_PMA_SPEED_2_5G);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
			 phydev->supported, val & MDIO_PMA_SPEED_5G);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_10000baseT_Full_BIT,
			 phydev->supported, val & MDIO_SPEED_10G);

	return genphy_read_abilities(phydev);
}

static int rtl822x_config_aneg(struct phy_device *phydev)
{
	int ret = 0;

	if (phydev->autoneg == AUTONEG_ENABLE) {
		u16 adv = linkmode_adv_to_mii_10gbt_adv_t(phydev->advertising);

		ret = phy_modify_mmd_changed(phydev, MDIO_MMD_VEND2,
					     RTL_MDIO_AN_10GBT_CTRL,
					     MDIO_AN_10GBT_CTRL_ADV2_5G |
					     MDIO_AN_10GBT_CTRL_ADV5G, adv);
		if (ret < 0)
			return ret;
	}

	return __genphy_config_aneg(phydev, ret);
}

static void rtl822xb_update_interface(struct phy_device *phydev)
{
	int val;

	if (!phydev->link)
		return;

	/* Change interface according to serdes mode */
	val = phy_read_mmd(phydev, MDIO_MMD_VEND1, RTL822X_VND1_SERDES_CTRL3);
	if (val < 0)
		return;

	switch (val & RTL822X_VND1_SERDES_CTRL3_MODE_MASK) {
	case RTL822X_VND1_SERDES_CTRL3_MODE_2500BASEX:
		phydev->interface = PHY_INTERFACE_MODE_2500BASEX;
		break;
	case RTL822X_VND1_SERDES_CTRL3_MODE_SGMII:
		phydev->interface = PHY_INTERFACE_MODE_SGMII;
		break;
	}
}

static int rtl822x_read_status(struct phy_device *phydev)
{
	int lpadv, ret;

	mii_10gbt_stat_mod_linkmode_lpa_t(phydev->lp_advertising, 0);

	ret = rtlgen_read_status(phydev);
	if (ret < 0)
		return ret;

	if (phydev->autoneg == AUTONEG_DISABLE ||
	    !phydev->autoneg_complete)
		return 0;

	lpadv = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL_MDIO_AN_10GBT_STAT);
	if (lpadv < 0)
		return lpadv;

	mii_10gbt_stat_mod_linkmode_lpa_t(phydev->lp_advertising, lpadv);

	return 0;
}

static int rtl822xb_read_status(struct phy_device *phydev)
{
	int ret;

	ret = rtl822x_read_status(phydev);
	if (ret < 0)
		return ret;

	rtl822xb_update_interface(phydev);

	return 0;
}

static int rtl822x_c45_get_features(struct phy_device *phydev)
{
	linkmode_set_bit(ETHTOOL_LINK_MODE_TP_BIT,
			 phydev->supported);

	phydev->c45_ids.mmds_present |= MDIO_DEVS_PMAPMD | MDIO_DEVS_PCS |
				        MDIO_DEVS_AN;

	return genphy_c45_pma_read_abilities(phydev);
}

static int rtl822x_c45_config_aneg(struct phy_device *phydev)
{
	bool changed = false;
	int ret, val;

	if (phydev->autoneg == AUTONEG_DISABLE)
		return genphy_c45_pma_setup_forced(phydev);

	ret = genphy_c45_an_config_aneg(phydev);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	val = linkmode_adv_to_mii_ctrl1000_t(phydev->advertising);

	/* Vendor register as C45 has no standardized support for 1000BaseT */
	ret = phy_modify_mmd_changed(phydev, MDIO_MMD_VEND2,
				     RTL822X_VND2_C22_REG(MII_CTRL1000),
				     ADVERTISE_1000FULL, val);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	return genphy_c45_check_and_restart_aneg(phydev, changed);
}

static int rtl822x_c45_read_status(struct phy_device *phydev)
{
	int ret, val;

	/* Vendor register as C45 has no standardized support for 1000BaseT */
	if (phydev->autoneg == AUTONEG_ENABLE && genphy_c45_aneg_done(phydev)) {
		val = phy_read_mmd(phydev, MDIO_MMD_VEND2,
				   RTL822X_VND2_C22_REG(MII_STAT1000));
		if (val < 0)
			return val;
	} else {
		val = 0;
	}
	mii_stat1000_mod_linkmode_lpa_t(phydev->lp_advertising, val);

	ret = genphy_c45_read_status(phydev);
	if (ret < 0)
		return ret;

	if (!phydev->link) {
		phydev->master_slave_state = MASTER_SLAVE_STATE_UNKNOWN;
		return 0;
	}

	/* Read actual speed from vendor register. */
	val = phy_read_mmd(phydev, MDIO_MMD_VEND2,
			   RTL822X_VND2_C22_REG(RTL_PHYSR));
	if (val < 0)
		return val;

	rtlgen_decode_physr(phydev, val);

	return 0;
}

static int rtl822x_c45_soft_reset(struct phy_device *phydev)
{
	int ret, val;

	ret = phy_modify_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_CTRL1,
			     MDIO_CTRL1_RESET, MDIO_CTRL1_RESET);
	if (ret < 0)
		return ret;

	return phy_read_mmd_poll_timeout(phydev, MDIO_MMD_PMAPMD,
					 MDIO_CTRL1, val,
					 !(val & MDIO_CTRL1_RESET),
					 5000, 100000, true);
}

static int rtl822xb_c45_read_status(struct phy_device *phydev)
{
	int ret;

	ret = rtl822x_c45_read_status(phydev);
	if (ret < 0)
		return ret;

	rtl822xb_update_interface(phydev);

	return 0;
}

static int rtl8224_cable_test_start(struct phy_device *phydev)
{
	u32 val;
	int ret;

	/* disable auto-negotiation and force 1000/Full */
	ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2,
			     RTL822X_VND2_C22_REG(MII_BMCR),
			     BMCR_ANENABLE | BMCR_SPEED100 | BMCR_SPEED10,
			     BMCR_SPEED1000 | BMCR_FULLDPLX);
	if (ret)
		return ret;

	mdelay(500);

	/* trigger cable test */
	val = RTL8224_MII_RTCT_ENABLE;
	val |= RTL8224_MII_RTCT_PAIR_A;
	val |= RTL8224_MII_RTCT_PAIR_B;
	val |= RTL8224_MII_RTCT_PAIR_C;
	val |= RTL8224_MII_RTCT_PAIR_D;

	return phy_modify_mmd(phydev, MDIO_MMD_VEND2,
			      RTL822X_VND2_C22_REG(RTL8224_MII_RTCT),
			      RTL8224_MII_RTCT_DONE, val);
}

static int rtl8224_sram_read(struct phy_device *phydev, u32 reg)
{
	int ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND2,
			    RTL822X_VND2_C22_REG(RTL8224_MII_SRAM_ADDR),
			    reg);
	if (ret)
		return ret;

	return phy_read_mmd(phydev, MDIO_MMD_VEND2,
			    RTL822X_VND2_C22_REG(RTL8224_MII_SRAM_DATA));
}

static int rtl8224_pair_len_get(struct phy_device *phydev, u32 pair)
{
	int cable_len;
	u32 reg_len;
	int ret;
	u32 cm;

	reg_len = RTL8224_SRAM_RTCT_LEN(pair);

	ret = rtl8224_sram_read(phydev, reg_len);
	if (ret < 0)
		return ret;

	cable_len = ret & 0xff00;

	ret = rtl8224_sram_read(phydev, reg_len + 1);
	if (ret < 0)
		return ret;

	cable_len |= (ret & 0xff00) >> 8;

	cable_len -= 620;
	cable_len = max(cable_len, 0);

	cm = cable_len * 100 / 78;

	return cm;
}

static int rtl8224_cable_test_result_trans(u32 result)
{
	if (!(result & RTL8224_SRAM_RTCT_FAULT_DONE))
		return -EBUSY;

	if (result & RTL8224_SRAM_RTCT_FAULT_OK)
		return ETHTOOL_A_CABLE_RESULT_CODE_OK;

	if (result & RTL8224_SRAM_RTCT_FAULT_OPEN)
		return ETHTOOL_A_CABLE_RESULT_CODE_OPEN;

	if (result & RTL8224_SRAM_RTCT_FAULT_SAME_SHORT)
		return ETHTOOL_A_CABLE_RESULT_CODE_SAME_SHORT;

	if (result & RTL8224_SRAM_RTCT_FAULT_BUSY)
		return ETHTOOL_A_CABLE_RESULT_CODE_UNSPEC;

	if (result & RTL8224_SRAM_RTCT_FAULT_CROSS_SHORT)
		return ETHTOOL_A_CABLE_RESULT_CODE_CROSS_SHORT;

	return ETHTOOL_A_CABLE_RESULT_CODE_UNSPEC;
}

static int rtl8224_cable_test_report_pair(struct phy_device *phydev, unsigned int pair)
{
	int fault_rslt;
	int ret;

	ret = rtl8224_sram_read(phydev, RTL8224_SRAM_RTCT_FAULT(pair));
	if (ret < 0)
		return ret;

	fault_rslt = rtl8224_cable_test_result_trans(ret);
	if (fault_rslt < 0)
		return 0;

	ret = ethnl_cable_test_result(phydev, pair, fault_rslt);
	if (ret < 0)
		return ret;

	switch (fault_rslt) {
	case ETHTOOL_A_CABLE_RESULT_CODE_OPEN:
	case ETHTOOL_A_CABLE_RESULT_CODE_SAME_SHORT:
	case ETHTOOL_A_CABLE_RESULT_CODE_CROSS_SHORT:
		ret = rtl8224_pair_len_get(phydev, pair);
		if (ret < 0)
			return ret;

		return ethnl_cable_test_fault_length(phydev, pair, ret);
	default:
		return  0;
	}
}

static int rtl8224_cable_test_report(struct phy_device *phydev, bool *finished)
{
	unsigned int pair;
	int ret;

	for (pair = ETHTOOL_A_CABLE_PAIR_A; pair <= ETHTOOL_A_CABLE_PAIR_D; pair++) {
		ret = rtl8224_cable_test_report_pair(phydev, pair);
		if (ret == -EBUSY) {
			*finished = false;
			return 0;
		}

		if (ret < 0)
			return ret;
	}

	return 0;
}

static int rtl8224_cable_test_get_status(struct phy_device *phydev, bool *finished)
{
	int ret;

	*finished = false;

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND2,
			   RTL822X_VND2_C22_REG(RTL8224_MII_RTCT));
	if (ret < 0)
		return ret;

	if (!(ret & RTL8224_MII_RTCT_DONE))
		return 0;

	*finished = true;

	return rtl8224_cable_test_report(phydev, finished);
}

static int rtl8224_package_modify_mmd(struct phy_device *phydev, int devad,
				      u32 regnum, u16 mask, u16 set)
{
	int val, ret;

	phy_lock_mdio_bus(phydev);

	val = __phy_package_read_mmd(phydev, 0, devad, regnum);
	if (val < 0) {
		ret = val;
		goto exit;
	}

	val &= ~mask;
	val |= set;

	ret = __phy_package_write_mmd(phydev, 0, devad, regnum, val);

exit:
	phy_unlock_mdio_bus(phydev);
	return ret;
}

static int rtl8224_sds_read(struct phy_device *phydev, int page, u32 reg)
{
	int ret;
	u32 val;
	u32 cmd = RTL8224_SDS_CMD_READ | (reg << 7) | (page << 1);

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, RTL8224_VND1_SERDES_CMD, cmd);
	if (ret < 0)
		return ret;

	ret = phy_read_mmd_poll_timeout(phydev, MDIO_MMD_VEND1,
					RTL8224_VND1_SERDES_CMD, val,
					!(val & BIT(15)), 500, 500000000,
					false);
	if (ret < 0)
		return ret;

	return phy_read_mmd(phydev, MDIO_MMD_VEND1, RTL8224_VND1_SERDES_READ);
}

static int rtl8224_sds_write(struct phy_device *phydev, int page, int reg, u32 data)
{
	int ret;
	u32 tmp;
	u32 cmd = RTL8224_SDS_CMD_WRITE | (reg << 7) | (page << 1);

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, RTL8224_VND1_SERDES_WRITE,
			    data);
	if (ret < 0)
		return ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, RTL8224_VND1_SERDES_CMD, cmd);
	if (ret < 0)
		return ret;

	return phy_read_mmd_poll_timeout(phydev, MDIO_MMD_VEND1,
					 RTL8224_VND1_SERDES_CMD, tmp,
					 !(tmp & BIT(15)), 500, 500000000,
					 false);
}

static int rtl8224_sds_modify(struct phy_device *phydev, int page, int reg, u32 mask, u32 data)
{
	int ret;
	u32 val;

	ret = rtl8224_sds_read(phydev, page, reg);
	if (ret < 0)
		return ret;

	val = (u32)ret;
	val = (val & ~mask) | (data & mask);

	ret = rtl8224_sds_write(phydev, page, reg, val);
	if (ret)
		return ret;

	return 0;
}

static int rtl8224_serdes_config(struct phy_device *phydev)
{
	if ((phydev->mdio.addr & 3) != 0)
		return 0;

	/* Turn SerDes OFF */
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, RTL8224_VND1_SERDES_MODE,
		       RTL8224_VND1_SERDES_MODE_MASK,
		       RTL8224_VND1_SERDES_MODE_OFF);

	/* Use generic/standard-compliant AN mode */
	rtl8224_sds_modify(phydev, RTL8224_SDS_NWAY_PAGE,
			   RTL8224_SDS_NWAY_OPCODE_REG,
			   RTL8224_SDS_NWAY_OPCODE_MASK, 0x0003);

	rtl8224_sds_write(phydev, RTL8224_SDS_AM_PAGE,
			  RTL8224_SDS_AM_PERIOD_REG, 0x00a4);

	/* Set all AM markers to 0 */
	rtl8224_sds_write(phydev, RTL8224_SDS_AM_PAGE,
			  RTL8224_SDS_AM_CFG0_REG, 0);
	rtl8224_sds_write(phydev, RTL8224_SDS_AM_PAGE,
			  RTL8224_SDS_AM_CFG1_REG, 0);
	rtl8224_sds_write(phydev, RTL8224_SDS_AM_PAGE,
			  RTL8224_SDS_AM_CFG2_REG, 0);
	rtl8224_sds_write(phydev, RTL8224_SDS_AM_PAGE,
			  RTL8224_SDS_AM_CFG3_REG, 0);
	rtl8224_sds_write(phydev, RTL8224_SDS_AM_PAGE,
			  RTL8224_SDS_AM_CFG4_REG, 0);
	rtl8224_sds_write(phydev, RTL8224_SDS_AM_PAGE,
			  RTL8224_SDS_AM_CFG5_REG, 0);

	/* Enable USXGMII autoneg on all 4 channels */
	rtl8224_sds_modify(phydev, RTL8224_SDS_NWAY_PAGE,
			   RTL8224_SDS_NWAY_AN_REG,
			   RTL8224_SDS_NWAY_AN_EN_MASK, 0xf);

	rtl8224_sds_modify(phydev, 0x06, 0x03, BIT(15), BIT(15));
	rtl8224_sds_modify(phydev, 0x06, 0x1d, BIT(9), BIT(9));
	rtl8224_sds_modify(phydev, 0x06, 0x1f, BIT(13), BIT(13));
	rtl8224_sds_write(phydev, 0x21, 0x10, 0x4480);
	rtl8224_sds_write(phydev, 0x21, 0x13, 0x0400);
	rtl8224_sds_write(phydev, 0x21, 0x18, 0x6d02);
	rtl8224_sds_write(phydev, 0x21, 0x1b, 0x424e);
	rtl8224_sds_write(phydev, 0x21, 0x1d, 0x0002);
	rtl8224_sds_write(phydev, 0x36, 0x1c, 0x1390);
	rtl8224_sds_write(phydev, 0x2e, 0x04, 0x0080);

	rtl8224_sds_write(phydev, RTL8224_SDS_REG0_TX_PAGE,
			  RTL8224_SDS_REG0_TX_POST1_REG, 0x0408);

	rtl8224_sds_write(phydev, RTL8224_SDS_REG0_TX_PAGE,
			  RTL8224_SDS_REG0_TX_REG, 0x020d);

	rtl8224_sds_write(phydev, 0x2e, 0x09, 0x0601);

	rtl8224_sds_write(phydev, RTL8224_SDS_REG0_TX_PAGE,
			  RTL8224_SDS_REG0_TX_Z0_REG, 0x222c);

	rtl8224_sds_write(phydev, 0x2e, 0x0c, 0xa217);
	rtl8224_sds_write(phydev, 0x2e, 0x0d, 0xfe40);
	rtl8224_sds_write(phydev, 0x2e, 0x15, 0xf5f1);
	rtl8224_sds_write(phydev, 0x2e, 0x16, 0x0443);
	rtl8224_sds_write(phydev, 0x2e, 0x1d, 0xabb0);

	/* Turn SerDes ON in 10G_QXGMII mode */
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, RTL8224_VND1_SERDES_MODE,
		       RTL8224_VND1_SERDES_SUBMODE_MASK,
		       RTL8224_VND1_SERDES_SUBMODE_10G_QXGMII);
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, RTL8224_VND1_SERDES_MODE,
		       RTL8224_VND1_SERDES_MODE_MASK,
		       RTL8224_VND1_SERDES_MODE_USXGMII);

	rtl8224_sds_modify(phydev, 0x20, 0x00, GENMASK(5, 4), 0x30);
	rtl8224_sds_modify(phydev, 0x20, 0x00, GENMASK(5, 4), 0x10);
	rtl8224_sds_modify(phydev, 0x20, 0x00, GENMASK(5, 4), 0x30);
	rtl8224_sds_modify(phydev, 0x20, 0x00, GENMASK(5, 4), 0x00);

	return 0;
}

static int rtl8224_mdi_config_order(struct phy_device *phydev)
{
	struct device_node *np = phydev->mdio.dev.of_node;
	u8 port_offset = phydev->mdio.addr & 3;
	u32 order = 0;
	int ret;

	ret = of_property_read_u32(np, "enet-phy-pair-order", &order);

	/* Do nothing in case the property is not present */
	if (ret == -EINVAL || ret == -ENOSYS)
		return 0;

	if (ret)
		return ret;

	if (order & ~1)
		return -EINVAL;

	return rtl8224_package_modify_mmd(phydev, MDIO_MMD_VEND1,
					  RTL8224_VND1_MDI_PAIR_SWAP,
					  BIT(port_offset),
					  order ? BIT(port_offset) : 0);
}

static int rtl8224_mdi_config_polarity(struct phy_device *phydev)
{
	struct device_node *np = phydev->mdio.dev.of_node;
	u8 offset = (phydev->mdio.addr & 3) * 4;
	u32 polarity = 0;
	int ret;

	ret = of_property_read_u32(np, "enet-phy-pair-polarity", &polarity);

	/* Do nothing if the property is not present */
	if (ret == -EINVAL || ret == -ENOSYS)
		return 0;

	if (ret)
		return ret;

	if (polarity & ~0xf)
		return -EINVAL;

	return rtl8224_package_modify_mmd(phydev, MDIO_MMD_VEND1,
					  RTL8224_VND1_MDI_POLARITY_SWAP,
					  0xf << offset,
					  polarity << offset);
}

static int rtl8224_config_init(struct phy_device *phydev)
{
	int ret;

	ret = rtl8224_serdes_config(phydev);
	if (ret)
		return ret;

	ret = rtl8224_mdi_config_order(phydev);
	if (ret)
		return ret;

	return rtl8224_mdi_config_polarity(phydev);
}

static int rtl8224_probe(struct phy_device *phydev)
{
	/* Chip exposes 4 ports, join all of them in the same package */
	return devm_phy_package_join(&phydev->mdio.dev, phydev,
				     phydev->mdio.addr & ~3, 0);
}

static bool rtlgen_supports_2_5gbps(struct phy_device *phydev)
{
	int val;

	mutex_lock(&phydev->mdio.bus->mdio_lock);
	rtl821x_write_page(phydev, 0xa61);
	val = __phy_read(phydev, 0x13);
	rtl821x_write_page(phydev, 0);
	mutex_unlock(&phydev->mdio.bus->mdio_lock);

	return val >= 0 && val & MDIO_PMA_SPEED_2_5G;
}

/* On internal PHY's MMD reads over C22 always return 0.
 * Check a MMD register which is known to be non-zero.
 */
static bool rtlgen_supports_mmd(struct phy_device *phydev)
{
	int val;

	phy_lock_mdio_bus(phydev);
	__phy_write(phydev, MII_MMD_CTRL, MDIO_MMD_PCS);
	__phy_write(phydev, MII_MMD_DATA, MDIO_PCS_EEE_ABLE);
	__phy_write(phydev, MII_MMD_CTRL, MDIO_MMD_PCS | MII_MMD_CTRL_NOINCR);
	val = __phy_read(phydev, MII_MMD_DATA);
	phy_unlock_mdio_bus(phydev);

	return val > 0;
}

static int rtlgen_match_phy_device(struct phy_device *phydev,
				   const struct phy_driver *phydrv)
{
	return phydev->phy_id == RTL_GENERIC_PHYID &&
	       !rtlgen_supports_2_5gbps(phydev);
}

static int rtl8226_match_phy_device(struct phy_device *phydev,
				    const struct phy_driver *phydrv)
{
	return phydev->phy_id == RTL_GENERIC_PHYID &&
	       rtlgen_supports_2_5gbps(phydev) &&
	       rtlgen_supports_mmd(phydev);
}

static int rtlgen_is_c45_match(struct phy_device *phydev, unsigned int id,
			       bool is_c45)
{
	if (phydev->is_c45) {
		u32 rid;

		if (!is_c45)
			return 0;

		rid = phydev->c45_ids.device_ids[1];
		if ((rid == 0xffffffff) && phydev->mdio.bus->read_c45) {
			int val;

			val = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PKGID1);
			if (val < 0)
				return 0;

			rid = val << 16;
			val = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PKGID2);
			if (val < 0)
				return 0;

			rid |= val;
		}

		return (id == rid);
	} else {
		return !is_c45 && (id == phydev->phy_id);
	}
}

static int rtl8221b_match_phy_device(struct phy_device *phydev,
				     const struct phy_driver *phydrv)
{
	return phydev->phy_id == RTL_8221B && rtlgen_supports_mmd(phydev);
}

static int rtl8221b_vb_cg_match_phy_device(struct phy_device *phydev,
					   const struct phy_driver *phydrv)
{
	return rtlgen_is_c45_match(phydev, RTL_8221B_VB_CG, true) ||
	       rtlgen_is_c45_match(phydev, RTL_8221B_VB_CG, false);
}

static int rtl8221b_vm_cg_match_phy_device(struct phy_device *phydev,
					   const struct phy_driver *phydrv)
{
	return rtlgen_is_c45_match(phydev, RTL_8221B_VM_CG, true) ||
	       rtlgen_is_c45_match(phydev, RTL_8221B_VM_CG, false);
}

static int rtl_internal_nbaset_match_phy_device(struct phy_device *phydev,
						const struct phy_driver *phydrv)
{
	if (phydev->is_c45)
		return false;

	switch (phydev->phy_id) {
	case RTL_GENERIC_PHYID:
	case RTL_8221B:
	case RTL_8251B:
	case RTL_8261C:
	case 0x001cc841:
		break;
	default:
		return false;
	}

	return rtlgen_supports_2_5gbps(phydev) && !rtlgen_supports_mmd(phydev);
}

static int rtl8251b_c45_match_phy_device(struct phy_device *phydev,
					 const struct phy_driver *phydrv)
{
	return rtlgen_is_c45_match(phydev, RTL_8251B, true);
}

static int rtl8251l_match_phy_device(struct phy_device *phydev,
				     const struct phy_driver *phydrv)
{
	int data;

	if (!rtlgen_is_c45_match(phydev, RTL_8261N, true))
		return 0;

	if (!phydev->mdio.bus->read_c45)
		return 0;

	data = phy_read_mmd(phydev, MDIO_MMD_VEND1, RTL826X_VEND1_PKG_MODEL);
	if (data < 0)
		return 0;

	if (data != 0x8251)
		return 0;

	return 1;
}

static int rtl8254b_match_phy_device(struct phy_device *phydev,
				     const struct phy_driver *phydrv)
{
	int data;

	if (!rtlgen_is_c45_match(phydev, RTL_8264B, true))
		return 0;

	if (!phydev->mdio.bus->read_c45)
		return 0;

	data = phy_read_mmd(phydev, MDIO_MMD_VEND1, RTL826X_VEND1_PKG_MODEL);
	if (data < 0)
		return 0;

	if (data != 0x8254)
		return 0;

	return 1;
}

static int rtl8261be_match_phy_device(struct phy_device *phydev,
				      const struct phy_driver *phydrv)
{
	int data;

	if (!rtlgen_is_c45_match(phydev, RTL_8261N, true))
		return 0;

	if (!phydev->mdio.bus->read_c45)
		return 0;

	data = phy_read_mmd(phydev, MDIO_MMD_VEND1, RTL826X_VEND1_PKG_MODEL);
	if (data < 0)
		return 0;

	if (data == 0x8251)
		return 0;

	data = phy_read_mmd(phydev, MDIO_MMD_VEND1, RTL826X_VEND1_VERSION_ID);
	if (data < 0)
		return 0;

	if ((data & 0xFFC0) != 0x1140)
		return 0;

	return 1;
}

static int rtl8261n_match_phy_device(struct phy_device *phydev,
				     const struct phy_driver *phydrv)
{
	int data;

	if (!rtlgen_is_c45_match(phydev, RTL_8261N, true))
		return 0;

	if (!phydev->mdio.bus->read_c45)
		return 0;

	data = phy_read_mmd(phydev, MDIO_MMD_VEND1, RTL826X_VEND1_PKG_MODEL);
	if (data < 0)
		return 0;

	if (data == 0x8251)
		return 0;

	data = phy_read_mmd(phydev, MDIO_MMD_VEND1, RTL826X_VEND1_VERSION_ID);
	if (data < 0)
		return 0;

	if ((data & 0xFFC0) == 0x1140)
		return 0;

	return 1;
}

static int rtl8264b_match_phy_device(struct phy_device *phydev,
				     const struct phy_driver *phydrv)
{
	int data;

	if (!rtlgen_is_c45_match(phydev, RTL_8264B, true))
		return 0;

	if (!phydev->mdio.bus->read_c45)
		return 0;

	data = phy_read_mmd(phydev, MDIO_MMD_VEND1, RTL826X_VEND1_PKG_MODEL);
	if (data < 0)
		return 0;

	if (data == 0x8254)
		return 0;

	return 1;
}

static int rtlgen_resume(struct phy_device *phydev)
{
	int ret = genphy_resume(phydev);

	/* Internal PHY's from RTL8168h up may not be instantly ready */
	msleep(20);

	return ret;
}

static int rtlgen_c45_resume(struct phy_device *phydev)
{
	int ret = genphy_c45_pma_resume(phydev);

	msleep(20);

	return ret;
}

static int rtl9000a_config_init(struct phy_device *phydev)
{
	phydev->autoneg = AUTONEG_DISABLE;
	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;

	return 0;
}

static int rtl9000a_config_aneg(struct phy_device *phydev)
{
	int ret;
	u16 ctl = 0;

	switch (phydev->master_slave_set) {
	case MASTER_SLAVE_CFG_MASTER_FORCE:
		ctl |= CTL1000_AS_MASTER;
		break;
	case MASTER_SLAVE_CFG_SLAVE_FORCE:
		break;
	case MASTER_SLAVE_CFG_UNKNOWN:
	case MASTER_SLAVE_CFG_UNSUPPORTED:
		return 0;
	default:
		phydev_warn(phydev, "Unsupported Master/Slave mode\n");
		return -EOPNOTSUPP;
	}

	ret = phy_modify_changed(phydev, MII_CTRL1000, CTL1000_AS_MASTER, ctl);
	if (ret == 1)
		ret = genphy_soft_reset(phydev);

	return ret;
}

static int rtl9000a_read_status(struct phy_device *phydev)
{
	int ret;

	phydev->master_slave_get = MASTER_SLAVE_CFG_UNKNOWN;
	phydev->master_slave_state = MASTER_SLAVE_STATE_UNKNOWN;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	ret = phy_read(phydev, MII_CTRL1000);
	if (ret < 0)
		return ret;
	if (ret & CTL1000_AS_MASTER)
		phydev->master_slave_get = MASTER_SLAVE_CFG_MASTER_FORCE;
	else
		phydev->master_slave_get = MASTER_SLAVE_CFG_SLAVE_FORCE;

	ret = phy_read(phydev, MII_STAT1000);
	if (ret < 0)
		return ret;
	if (ret & LPA_1000MSRES)
		phydev->master_slave_state = MASTER_SLAVE_STATE_MASTER;
	else
		phydev->master_slave_state = MASTER_SLAVE_STATE_SLAVE;

	return 0;
}

static int rtl9000a_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL8211F_INSR);

	return (err < 0) ? err : 0;
}

static int rtl9000a_config_intr(struct phy_device *phydev)
{
	u16 val;
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED) {
		err = rtl9000a_ack_interrupt(phydev);
		if (err)
			return err;

		val = (u16)~RTL9000A_GINMR_LINK_STATUS;
		err = phy_write_paged(phydev, 0xa42, RTL9000A_GINMR, val);
	} else {
		val = ~0;
		err = phy_write_paged(phydev, 0xa42, RTL9000A_GINMR, val);
		if (err)
			return err;

		err = rtl9000a_ack_interrupt(phydev);
	}

	return phy_write_paged(phydev, 0xa42, RTL9000A_GINMR, val);
}

static irqreturn_t rtl9000a_handle_interrupt(struct phy_device *phydev)
{
	int irq_status;

	irq_status = phy_read(phydev, RTL8211F_INSR);
	if (irq_status < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	if (!(irq_status & RTL8211F_INER_LINK_STATUS))
		return IRQ_NONE;

	phy_trigger_machine(phydev);

	return IRQ_HANDLED;
}

static int rtl8221b_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL8221B_VND2_INSR);

	return (err < 0) ? err : 0;
}

static int rtl8221b_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED) {
		err = rtl8221b_ack_interrupt(phydev);
		if (err)
			return err;

		err = phy_write_mmd(phydev, MDIO_MMD_VEND2, RTL8221B_VND2_INER,
				    RTL8221B_VND2_INER_LINK_STATUS);
	} else {
		err = phy_write_mmd(phydev, MDIO_MMD_VEND2,
				    RTL8221B_VND2_INER, 0);
		if (err)
			return err;

		err = rtl8221b_ack_interrupt(phydev);
	}

	return err;
}

static irqreturn_t rtl8221b_handle_interrupt(struct phy_device *phydev)
{
	int err;

	err = rtl8221b_ack_interrupt(phydev);
	if (err) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	phy_trigger_machine(phydev);

	return IRQ_HANDLED;
}

static int rtlgen_sfp_get_features(struct phy_device *phydev)
{
	linkmode_set_bit(ETHTOOL_LINK_MODE_10000baseT_Full_BIT,
			 phydev->supported);

	/* set default mode */
	phydev->speed = SPEED_10000;
	phydev->duplex = DUPLEX_FULL;

	phydev->port = PORT_FIBRE;

	return 0;
}

static int rtlgen_sfp_read_status(struct phy_device *phydev)
{
	int val, err;

	err = genphy_update_link(phydev);
	if (err)
		return err;

	if (!phydev->link)
		return 0;

	val = phy_read(phydev, RTL_PHYSR);
	if (val < 0)
		return val;

	rtlgen_decode_physr(phydev, val);

	return 0;
}

static int rtlgen_sfp_config_aneg(struct phy_device *phydev)
{
	return 0;
}

static int rtl826x_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	struct rtl826x_priv *priv;

	priv = devm_kzalloc(dev, sizeof(struct rtl826x_priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->enable_pma_low_power = device_property_read_bool(dev, "realtek,enable-pma-low-power");
	phydev->priv = priv;

	if (!priv->enable_pma_low_power)
		phydev_warn(phydev, "PMA low-power suspend disabled\n");

	/* Disable EEE due to link stability issues */
	phy_set_eee_broken(phydev, MDIO_EEE_100TX);
	phy_set_eee_broken(phydev, MDIO_EEE_1000T);

	if (IS_ENABLED(CONFIG_REALTEK_PHY_HWMON))
		return rtl822x_hwmon_init(phydev);

	return 0;
}

static int rtl8261n_probe(struct phy_device *phydev)
{
	int ret;

	ret = rtl826x_probe(phydev);
	if (ret < 0)
		return ret;

	ret = rtl8261n_phy_patch_db_init(phydev);
	if (ret < 0)
		return ret;

	return 0;
}

static int rtl8264b_probe(struct phy_device *phydev)
{
	int ret;

	ret = rtl826x_probe(phydev);
	if (ret < 0)
		return ret;

	ret = rtl8264b_phy_patch_db_init(phydev);
	if (ret < 0)
		return ret;

	return 0;
}

static int rtl826x_config_serdes_polarity(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	unsigned int pol;
	u16 mask = 0;
	u16 set = 0;
	int ret;

	ret = phy_get_rx_polarity(dev_fwnode(dev), phy_modes(phydev->interface),
				  BIT(PHY_POL_NORMAL) | BIT(PHY_POL_INVERT),
				  PHY_POL_NORMAL, &pol);
	if (ret)
		return ret;

	mask |= RTL826X_VEND1_SERDES_GLOBAL_CFG_HSI_INV;
	if (pol == PHY_POL_INVERT)
		set |= RTL826X_VEND1_SERDES_GLOBAL_CFG_HSI_INV;

	ret = phy_get_tx_polarity(dev_fwnode(dev), phy_modes(phydev->interface),
				  BIT(PHY_POL_NORMAL) | BIT(PHY_POL_INVERT),
				  PHY_POL_NORMAL, &pol);
	if (ret)
		return ret;

	mask |= RTL826X_VEND1_SERDES_GLOBAL_CFG_HSO_INV;
	if (pol == PHY_POL_INVERT)
		set |= RTL826X_VEND1_SERDES_GLOBAL_CFG_HSO_INV;

	return phy_modify_mmd(phydev, MDIO_MMD_VEND1, RTL826X_VEND1_SERDES_GLOBAL_CFG,
			      mask, set);
}

static int rtl826x_config_init(struct phy_device *phydev)
{
	const unsigned int max_dereset_tries = 5;
	unsigned int tries = 0;
	int ret;

	/* Suppress PHY interrupt output before the hardware reset below.
	 * The RTL8261N asserts its interrupt pin during power-on reset, which
	 * races with phylib registering phy_interrupt. If the IRQ fires first
	 * the kernel disables it permanently. rtl826x_config_intr() will
	 * re-enable the interrupt correctly once phylib is ready.
	 */
	ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, RTL826X_VND2_INER, 0);
	if (ret < 0)
		return ret;

	ret = phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, 0xE1, BIT(0));
	if (ret < 0)
		return ret;

	/* toggle reset */
	ret = phy_set_bits_mmd(phydev, MDIO_MMD_VEND1, 0x145, BIT(0));
	if (ret < 0)
		return ret;

	do {
		msleep(30);
		tries++;
		ret = phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, 0x145, BIT(0));
	} while (ret < 0 && tries <= max_dereset_tries);

	if (ret < 0) {
		phydev_err(phydev, "deassert reset failed %d\n", ret);
		return ret;
	}

	msleep(30);

	ret = rtlgen_phy_patch(phydev);
	if (ret) {
		phydev_err(phydev, "patch failed!! %d\n", ret);
		return ret;
	}

	return rtl826x_config_serdes_polarity(phydev);
}

static int rtl826x_suspend(struct phy_device *phydev)
{
	struct rtl826x_priv *priv = phydev->priv;

	if (!priv->enable_pma_low_power)
		return -EOPNOTSUPP;

	return genphy_c45_pma_suspend(phydev);
}

static int rtl826x_get_features(struct phy_device *phydev)
{
	int ret;

	ret = genphy_c45_pma_read_abilities(phydev);
	if (ret)
		return ret;

	/* not support 10M modes */
	linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT,
			   phydev->supported);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT,
			   phydev->supported);

	return 0;
}

static int rtl825xb_get_features(struct phy_device *phydev)
{
	int ret;

	ret = genphy_c45_pma_read_abilities(phydev);
	if (ret)
		return ret;

	/* not support 10M modes */
	linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT,
			   phydev->supported);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT,
			   phydev->supported);

	/* faulty rtl826x silicon having issues with 10G, sold as only 5G phy */
	linkmode_clear_bit(ETHTOOL_LINK_MODE_10000baseT_Full_BIT,
			   phydev->supported);

	return 0;
}

static int rtl826x_config_aneg(struct phy_device *phydev)
{
	bool changed = false;
	u16 reg;
	int ret;

	phydev->mdix_ctrl = ETH_TP_MDI_AUTO;
	if (phydev->autoneg == AUTONEG_DISABLE)
		return genphy_c45_pma_setup_forced(phydev);

	ret = genphy_c45_an_config_aneg(phydev);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	reg = 0;
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			      phydev->advertising))
		reg |= ADVERTISE_1000FULL;

	if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Half_BIT,
			      phydev->advertising))
		reg |= ADVERTISE_1000HALF;

	ret = phy_modify_mmd_changed(phydev, MDIO_MMD_VEND2,
				     RTL822X_VND2_C22_REG(MII_CTRL1000),
				     ADVERTISE_1000FULL | ADVERTISE_1000HALF, reg);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	return genphy_c45_check_and_restart_aneg(phydev, changed);
}

static int rtl826x_read_status(struct phy_device *phydev)
{
	int status;
	int ret;

	phydev->speed = SPEED_UNKNOWN;
	phydev->duplex = DUPLEX_UNKNOWN;
	phydev->pause = 0;
	phydev->asym_pause = 0;

	ret = genphy_c45_read_link(phydev);
	if (ret)
		return ret;

	if (phydev->autoneg == AUTONEG_ENABLE) {
		ret = genphy_c45_read_lpa(phydev);
		if (ret)
			return ret;

		status = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL822X_VND2_C22_REG(MII_STAT1000));
		if (status < 0)
			return status;

		mii_stat1000_mod_linkmode_lpa_t(phydev->lp_advertising, status);

		phy_resolve_aneg_linkmode(phydev);
	} else {
		ret = genphy_c45_read_pma(phydev);
		if (ret < 0)
			return ret;
	}

	return genphy_c45_read_mdix(phydev);
}

static int rtl826x_config_intr(struct phy_device *phydev)
{
	u16 interrupts;
	int ret;

	/* Disable all IMR */
	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xE1, 0);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xE3, 0);
	if (ret < 0)
		return ret;

	/* source */
	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xE4, 0x1);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, 0xE0, 0x2F);
	if (ret < 0)
		return ret;

	/* init common link change & WOL */
	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		interrupts = RTL826X_VND2_INER_LINK_STATUS | RTL826X_VND2_INER_PME;
	else
		interrupts = 0;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, RTL826X_VND2_INER, interrupts);
	if (ret < 0)
		return ret;

	/* clear status */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x2DC, 0xFF);

	return phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0xE1, BIT(0),
			      phydev->interrupts == PHY_INTERRUPT_ENABLED);
}

static irqreturn_t rtl826x_handle_interrupt(struct phy_device *phydev)
{
	int irq_enabled;
	int irq_status;

	irq_status = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL826X_VND2_INSR);
	if (irq_status < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	if (phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x2DC, 0xFF) < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	irq_enabled = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL826X_VND2_INER);
	if (irq_enabled < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	if (!(irq_status & irq_enabled))
		return IRQ_NONE;

	if (irq_status & RTL826X_VND2_INER_LINK_STATUS) {
		phydev_dbg(phydev, "RTK_PHY_INTR_LINK_CHANGE\n");
		phy_mac_interrupt(phydev);
	}

	if (irq_status & RTL826X_VND2_INER_PME) {
		phydev_dbg(phydev, "RTK_PHY_INTR_WOL\n");

		if (phy_clear_bits_mmd(phydev, MDIO_MMD_VEND2, 0xD8A2, BIT(15)) < 0)
			return IRQ_NONE;

		if (phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, 0xD8A2, BIT(15)) < 0)
			return IRQ_NONE;
	}

	return IRQ_HANDLED;
}

static int rtl826x_set_wol(struct phy_device *phydev, struct ethtool_wolinfo *wol)
{
	static const u32 cfg_reg[4] = {0xD8C6, 0xD8C8, 0xD8CA, 0xD8CC};
	struct net_device *ndev = phydev->attached_dev;
	struct netdev_hw_addr *ha;
	u16 rtk_wolopts;
	size_t idx;
	u32 offset;
	int ret;

	if (!ndev)
		return -EINVAL;

	if (wol->wolopts & ~(WAKE_PHY | WAKE_MAGIC | WAKE_UCAST | WAKE_BCAST | WAKE_MCAST))
		return -EOPNOTSUPP;

	if (wol->wolopts & (WAKE_MAGIC | WAKE_UCAST)) {
		const u8 *mac_addr = ndev->dev_addr;

		ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, 0xD8C0,
				    mac_addr[1] << 8 | mac_addr[0]);
		if (ret < 0)
			return ret;

		ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, 0xD8C2,
				    mac_addr[3] << 8 | mac_addr[2]);
		if (ret < 0)
			return ret;

		ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, 0xD8C4,
				    mac_addr[5] << 8 | mac_addr[4]);
		if (ret < 0)
			return ret;
	}

	if (wol->wolopts & WAKE_MCAST) {
		for (idx = 0; idx < ARRAY_SIZE(cfg_reg); idx++) {
			ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, cfg_reg[idx], 0);
			if (ret < 0)
				return ret;
		}

		if (!netdev_mc_empty(ndev)) {
			netdev_for_each_mc_addr(ha, ndev) {
				phydev_dbg(phydev, "mac: %pM\n", ha->addr);

				offset = crc32_be(~0, ha->addr, 6) >> 26;
				idx = offset >> 4;

				ret = phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, cfg_reg[idx],
						       BIT(offset & 0xF));
				if (ret < 0)
					return ret;
			}
		}
	}

	rtk_wolopts = 0;
	if (wol->wolopts & WAKE_PHY)
		rtk_wolopts |= BIT(13);
	if (wol->wolopts & WAKE_MAGIC)
		rtk_wolopts |= BIT(12);
	if (wol->wolopts & WAKE_UCAST)
		rtk_wolopts |= BIT(10);
	if (wol->wolopts & WAKE_MCAST)
		rtk_wolopts |= BIT(9);
	if (wol->wolopts & WAKE_BCAST)
		rtk_wolopts |= BIT(8);

	ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0,
			     GENMASK(13, 12) | GENMASK(10, 8), rtk_wolopts);
	if (ret < 0)
		return ret;

	return 0;
}

static void rtl826x_get_wol(struct phy_device *phydev, struct ethtool_wolinfo *wol)
{
	int rtk_wolopts;

	wol->supported = WAKE_PHY | WAKE_MAGIC | WAKE_UCAST | WAKE_BCAST | WAKE_MCAST;
	wol->wolopts = 0;

	rtk_wolopts = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0);
	if (rtk_wolopts < 0)
		return;

	if (rtk_wolopts & BIT(13))
		wol->wolopts |= WAKE_PHY;
	if (rtk_wolopts & BIT(12))
		wol->wolopts |= WAKE_MAGIC;
	if (rtk_wolopts & BIT(10))
		wol->wolopts |= WAKE_UCAST;
	if (rtk_wolopts & BIT(9))
		wol->wolopts |= WAKE_MCAST;
	if (rtk_wolopts & BIT(8))
		wol->wolopts |= WAKE_BCAST;
}

static int rtl826x_get_tunable(struct phy_device *phydev, struct ethtool_tunable *tuna, void *data)
{
	int val;

	switch (tuna->id) {
	case ETHTOOL_PHY_EDPD:
		val = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL8221B_PHYCR1);
		if (val < 0)
			return val;

		*(u16 *)data = (!(val & RTL8221B_PHYCR1_ALDPS_EN)) ? ETHTOOL_PHY_EDPD_DISABLE :
			       ETHTOOL_PHY_EDPD_DFLT_TX_MSECS;
		break;

	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

static int rtl826x_set_tunable(struct phy_device *phydev,
			       struct ethtool_tunable *tuna, const void *data)
{
	int ret;
	u16 val;

	switch (tuna->id) {
	case ETHTOOL_PHY_EDPD:
		switch (*(const u16 *)data) {
		case ETHTOOL_PHY_EDPD_DFLT_TX_MSECS:
			val = RTL8221B_PHYCR1_ALDPS_EN;
			break;
		case ETHTOOL_PHY_EDPD_DISABLE:
			val = 0;
			break;
		default:
			return -EINVAL;
		}

		ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, RTL8221B_PHYCR1,
				     RTL8221B_PHYCR1_ALDPS_EN, val);
		if (ret < 0)
			return ret;
		break;

	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

static struct phy_driver realtek_drvs[] = {
	{
		PHY_ID_MATCH_EXACT(0x00008201),
		.name		= "RTL8201CP Ethernet",
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc816),
		.name		= "RTL8201F Fast Ethernet",
		.config_intr	= &rtl8201_config_intr,
		.handle_interrupt = rtl8201_handle_interrupt,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
	}, {
		PHY_ID_MATCH_MODEL(0x001cc880),
		.name		= "RTL8208 Fast Ethernet",
		.read_mmd	= genphy_read_mmd_unsupported,
		.write_mmd	= genphy_write_mmd_unsupported,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc910),
		.name		= "RTL8211 Gigabit Ethernet",
		.config_aneg	= rtl8211_config_aneg,
		.read_mmd	= &genphy_read_mmd_unsupported,
		.write_mmd	= &genphy_write_mmd_unsupported,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc912),
		.name		= "RTL8211B Gigabit Ethernet",
		.config_intr	= &rtl8211b_config_intr,
		.handle_interrupt = rtl821x_handle_interrupt,
		.read_mmd	= &genphy_read_mmd_unsupported,
		.write_mmd	= &genphy_write_mmd_unsupported,
		.suspend	= rtl8211b_suspend,
		.resume		= rtl8211b_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc913),
		.name		= "RTL8211C Gigabit Ethernet",
		.config_init	= rtl8211c_config_init,
		.read_mmd	= &genphy_read_mmd_unsupported,
		.write_mmd	= &genphy_write_mmd_unsupported,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc914),
		.name		= "RTL8211DN Gigabit Ethernet",
		.config_intr	= rtl8211e_config_intr,
		.handle_interrupt = rtl821x_handle_interrupt,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc915),
		.name		= "RTL8211E Gigabit Ethernet",
		.config_init	= &rtl8211e_config_init,
		.config_intr	= &rtl8211e_config_intr,
		.handle_interrupt = rtl821x_handle_interrupt,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.led_hw_is_supported = rtl8211x_led_hw_is_supported,
		.led_hw_control_get = rtl8211e_led_hw_control_get,
		.led_hw_control_set = rtl8211e_led_hw_control_set,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc916),
		.name		= "RTL8211F Gigabit Ethernet",
		.probe		= rtl8211f_probe,
		.config_init	= &rtl8211f_config_init,
		.read_status	= rtlgen_read_status,
		.config_intr	= &rtl8211f_config_intr,
		.handle_interrupt = rtl8211f_handle_interrupt,
		.set_wol	= rtl8211f_set_wol,
		.get_wol	= rtl8211f_get_wol,
		.suspend	= rtl8211f_suspend,
		.resume		= rtl8211f_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.flags		= PHY_ALWAYS_CALL_SUSPEND,
		.led_hw_is_supported = rtl8211x_led_hw_is_supported,
		.led_hw_control_get = rtl8211f_led_hw_control_get,
		.led_hw_control_set = rtl8211f_led_hw_control_set,
	}, {
		PHY_ID_MATCH_EXACT(RTL_8211FVD_PHYID),
		.name		= "RTL8211F-VD Gigabit Ethernet",
		.probe		= rtl821x_probe,
		.config_init	= &rtl8211f_config_init,
		.read_status	= rtlgen_read_status,
		.config_intr	= &rtl8211f_config_intr,
		.handle_interrupt = rtl8211f_handle_interrupt,
		.suspend	= rtl821x_suspend,
		.resume		= rtl821x_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.flags		= PHY_ALWAYS_CALL_SUSPEND,
	}, {
		.name		= "Generic FE-GE Realtek PHY",
		.match_phy_device = rtlgen_match_phy_device,
		.read_status	= rtlgen_read_status,
		.suspend	= genphy_suspend,
		.resume		= rtlgen_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.read_mmd	= rtlgen_read_mmd,
		.write_mmd	= rtlgen_write_mmd,
	}, {
		.name		= "RTL8226 2.5Gbps PHY",
		.match_phy_device = rtl8226_match_phy_device,
		.soft_reset	= genphy_soft_reset,
		.get_features	= rtl822x_get_features,
		.config_aneg	= rtl822x_config_aneg,
		.read_status	= rtl822x_read_status,
		.suspend	= genphy_suspend,
		.resume		= rtlgen_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.read_mmd	= rtl822xb_read_mmd,
		.write_mmd	= rtl822xb_write_mmd,
	}, {
		.match_phy_device = rtl8221b_match_phy_device,
		.name		= "RTL8226B_RTL8221B 2.5Gbps PHY",
		.soft_reset	= genphy_soft_reset,
		.get_features	= rtl822x_get_features,
		.config_aneg	= rtl822x_config_aneg,
		.config_init	= rtl822xb_config_init,
		.inband_caps	= rtl822x_inband_caps,
		.config_inband	= rtl822x_config_inband,
		.get_rate_matching = rtl822xb_get_rate_matching,
		.read_status	= rtl822xb_read_status,
		.suspend	= genphy_suspend,
		.resume		= rtlgen_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.read_mmd	= rtl822xb_read_mmd,
		.write_mmd	= rtl822xb_write_mmd,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc838),
		.name		= "RTL8226-CG 2.5Gbps PHY",
		.soft_reset	= rtl822x_c45_soft_reset,
		.get_features	= rtl822x_c45_get_features,
		.config_aneg	= rtl822x_c45_config_aneg,
		.probe		= rtl8226_probe,
		.config_init	= rtl822x_config_init,
		.inband_caps	= rtl822x_inband_caps,
		.config_inband	= rtl822x_config_inband,
		.read_status	= rtl822xb_c45_read_status,
		.suspend	= genphy_c45_pma_suspend,
		.resume		= rtlgen_c45_resume,
		.read_mmd	= rtl822xb_read_mmd,
		.write_mmd	= rtl822xb_write_mmd,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc848),
		.name		= "RTL8226B-CG_RTL8221B-CG 2.5Gbps PHY",
		.soft_reset	= genphy_soft_reset,
		.get_features	= rtl822x_get_features,
		.config_aneg	= rtl822x_config_aneg,
		.config_init	= rtl822xb_config_init,
		.inband_caps	= rtl822x_inband_caps,
		.config_inband	= rtl822x_config_inband,
		.get_rate_matching = rtl822xb_get_rate_matching,
		.read_status	= rtl822xb_read_status,
		.suspend	= genphy_suspend,
		.resume		= rtlgen_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.read_mmd	= rtl822xb_read_mmd,
		.write_mmd	= rtl822xb_write_mmd,
	}, {
		.match_phy_device = rtl8221b_vb_cg_match_phy_device,
		.name		= "RTL8221B-VB-CG 2.5Gbps PHY",
		.config_intr	= rtl8221b_config_intr,
		.handle_interrupt = rtl8221b_handle_interrupt,
		.soft_reset	= rtl822x_c45_soft_reset,
		.probe		= rtl822x_probe,
		.config_init	= rtl822xb_config_init,
		.inband_caps	= rtl822x_inband_caps,
		.config_inband	= rtl822x_config_inband,
		.get_rate_matching = rtl822xb_get_rate_matching,
		.get_features	= rtl822x_c45_get_features,
		.config_aneg	= rtl822x_c45_config_aneg,
		.read_status	= rtl822xb_c45_read_status,
		.suspend	= genphy_c45_pma_suspend,
		.resume		= rtlgen_c45_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.read_mmd	= rtl822xb_read_mmd,
		.write_mmd	= rtl822xb_write_mmd,
	}, {
		.match_phy_device = rtl8221b_vm_cg_match_phy_device,
		.name		= "RTL8221B-VM-CG 2.5Gbps PHY",
		.config_intr	= rtl8221b_config_intr,
		.handle_interrupt = rtl8221b_handle_interrupt,
		.soft_reset	= rtl822x_c45_soft_reset,
		.probe		= rtl822x_probe,
		.config_init	= rtl822xb_config_init,
		.inband_caps	= rtl822x_inband_caps,
		.config_inband	= rtl822x_config_inband,
		.get_rate_matching = rtl822xb_get_rate_matching,
		.get_features	= rtl822x_c45_get_features,
		.config_aneg	= rtl822x_c45_config_aneg,
		.read_status	= rtl822xb_c45_read_status,
		.suspend	= genphy_c45_pma_suspend,
		.resume		= rtlgen_c45_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.read_mmd	= rtl822xb_read_mmd,
		.write_mmd	= rtl822xb_write_mmd,
	}, {
		.match_phy_device = rtl8251b_c45_match_phy_device,
		.name		= "RTL8251B 5Gbps PHY",
		.probe		= rtl822x_probe,
		.get_features	= rtl822x_get_features,
		.config_aneg	= rtl822x_config_aneg,
		.read_status	= rtl822x_read_status,
		.suspend	= genphy_suspend,
		.resume		= rtlgen_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
	}, {
		.match_phy_device = rtl_internal_nbaset_match_phy_device,
		.name		= "Realtek Internal NBASE-T PHY",
		.flags		= PHY_IS_INTERNAL,
		.probe		= rtl822x_probe,
		.get_features	= rtl822x_get_features,
		.config_aneg	= rtl822x_config_aneg,
		.read_status	= rtl822x_read_status,
		.suspend	= genphy_suspend,
		.resume		= rtlgen_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.read_mmd	= rtl822x_read_mmd,
		.write_mmd	= rtl822x_write_mmd,
	}, {
		PHY_ID_MATCH_EXACT(PHY_ID_RTL_DUMMY_SFP),
		.name		= "Realtek SFP PHY Mode",
		.flags		= PHY_IS_INTERNAL,
		.probe		= rtl822x_probe,
		.get_features	= rtlgen_sfp_get_features,
		.config_aneg	= rtlgen_sfp_config_aneg,
		.read_status	= rtlgen_sfp_read_status,
		.suspend	= genphy_suspend,
		.resume		= rtlgen_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.read_mmd	= rtl822x_read_mmd,
		.write_mmd	= rtl822x_write_mmd,
	}, {
		PHY_ID_MATCH_EXACT(0x001ccad0),
		.name		= "RTL8224 2.5Gbps PHY",
		.flags		= PHY_POLL_CABLE_TEST,
		.probe		= rtl8224_probe,
		.config_init	= rtl8224_config_init,
		.get_features	= rtl822x_c45_get_features,
		.config_aneg	= rtl822x_c45_config_aneg,
		.read_status	= rtl822x_c45_read_status,
		.suspend	= genphy_c45_pma_suspend,
		.resume		= rtlgen_c45_resume,
		.cable_test_start = rtl8224_cable_test_start,
		.cable_test_get_status = rtl8224_cable_test_get_status,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc961),
		.name		= "RTL8366RB Gigabit Ethernet",
		.config_init	= &rtl8366rb_config_init,
		/* These interrupts are handled by the irq controller
		 * embedded inside the RTL8366RB, they get unmasked when the
		 * irq is requested and ACKed by reading the status register,
		 * which is done by the irqchip code.
		 */
		.config_intr	= genphy_no_config_intr,
		.handle_interrupt = genphy_handle_interrupt_no_ack,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	}, {
		PHY_ID_MATCH_EXACT(0x001ccb00),
		.name		= "RTL9000AA_RTL9000AN Ethernet",
		.features	= PHY_BASIC_T1_FEATURES,
		.config_init	= rtl9000a_config_init,
		.config_aneg	= rtl9000a_config_aneg,
		.read_status	= rtl9000a_read_status,
		.config_intr	= rtl9000a_config_intr,
		.handle_interrupt = rtl9000a_handle_interrupt,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc942),
		.name		= "RTL8365MB-VC Gigabit Ethernet",
		/* Interrupt handling analogous to RTL8366RB */
		.config_intr	= genphy_no_config_intr,
		.handle_interrupt = genphy_handle_interrupt_no_ack,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc960),
		.name		= "RTL8366S Gigabit Ethernet",
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_mmd	= genphy_read_mmd_unsupported,
		.write_mmd	= genphy_write_mmd_unsupported,
	}, {
		.name           = "RTL8251L 5Gbps PHY",
		.config_init    = rtl826x_config_init,
		.probe          = rtl8261n_probe,
		.get_features   = rtl825xb_get_features,
		.suspend        = rtl826x_suspend,
		.resume         = rtlgen_c45_resume,
		.config_aneg    = rtl826x_config_aneg,
		.aneg_done      = genphy_c45_aneg_done,
		.read_status    = rtl826x_read_status,
		.config_intr    = rtl826x_config_intr,
		.handle_interrupt = rtl826x_handle_interrupt,
		.match_phy_device = rtl8251l_match_phy_device,
		.set_wol        = rtl826x_set_wol,
		.get_wol        = rtl826x_get_wol,
		.get_tunable    = rtl826x_get_tunable,
		.set_tunable    = rtl826x_set_tunable,
	}, {
		.name           = "RTL8254B 5Gbps PHY",
		.config_init    = rtl826x_config_init,
		.probe          = rtl8264b_probe,
		.get_features   = rtl825xb_get_features,
		.suspend        = rtl826x_suspend,
		.resume         = rtlgen_c45_resume,
		.config_aneg    = rtl826x_config_aneg,
		.aneg_done      = genphy_c45_aneg_done,
		.read_status    = rtl826x_read_status,
		.config_intr    = rtl826x_config_intr,
		.handle_interrupt = rtl826x_handle_interrupt,
		.match_phy_device = rtl8254b_match_phy_device,
		.set_wol        = rtl826x_set_wol,
		.get_wol        = rtl826x_get_wol,
		.get_tunable    = rtl826x_get_tunable,
		.set_tunable    = rtl826x_set_tunable,
	}

	, {
		.name           = "RTL8261BE 10Gbps PHY",
		.config_init    = rtl826x_config_init,
		.probe          = rtl8261n_probe,
		.get_features   = rtl826x_get_features,
		.suspend        = rtl826x_suspend,
		.resume         = rtlgen_c45_resume,
		.config_aneg    = rtl826x_config_aneg,
		.aneg_done      = genphy_c45_aneg_done,
		.read_status    = rtl826x_read_status,
		.config_intr    = rtl826x_config_intr,
		.handle_interrupt = rtl826x_handle_interrupt,
		.match_phy_device = rtl8261be_match_phy_device,
		.set_wol        = rtl826x_set_wol,
		.get_wol        = rtl826x_get_wol,
		.get_tunable    = rtl826x_get_tunable,
		.set_tunable    = rtl826x_set_tunable,
	}, {
		.name           = "RTL8261N 10Gbps PHY",
		.config_init    = rtl826x_config_init,
		.probe          = rtl8261n_probe,
		.get_features   = rtl826x_get_features,
		.suspend        = rtl826x_suspend,
		.resume         = rtlgen_c45_resume,
		.config_aneg    = rtl826x_config_aneg,
		.aneg_done      = genphy_c45_aneg_done,
		.read_status    = rtl826x_read_status,
		.config_intr    = rtl826x_config_intr,
		.handle_interrupt = rtl826x_handle_interrupt,
		.match_phy_device = rtl8261n_match_phy_device,
		.set_wol        = rtl826x_set_wol,
		.get_wol        = rtl826x_get_wol,
		.get_tunable    = rtl826x_get_tunable,
		.set_tunable    = rtl826x_set_tunable,
	}, {
		PHY_ID_MATCH_EXACT(RTL_8264),
		.name           = "RTL8264 10Gbps PHY",
		.config_init    = rtl826x_config_init,
		.probe          = rtl8264b_probe,
		.get_features   = rtl826x_get_features,
		.suspend        = rtl826x_suspend,
		.resume         = rtlgen_c45_resume,
		.config_aneg    = rtl826x_config_aneg,
		.aneg_done      = genphy_c45_aneg_done,
		.read_status    = rtl826x_read_status,
		.config_intr    = rtl826x_config_intr,
		.handle_interrupt = rtl826x_handle_interrupt,
		.set_wol        = rtl826x_set_wol,
		.get_wol        = rtl826x_get_wol,
		.get_tunable    = rtl826x_get_tunable,
		.set_tunable    = rtl826x_set_tunable,
	}, {
		.name           = "RTL8264B 10Gbps PHY",
		.config_init    = rtl826x_config_init,
		.probe          = rtl8264b_probe,
		.get_features   = rtl826x_get_features,
		.suspend        = rtl826x_suspend,
		.resume         = rtlgen_c45_resume,
		.config_aneg    = rtl826x_config_aneg,
		.aneg_done      = genphy_c45_aneg_done,
		.read_status    = rtl826x_read_status,
		.config_intr    = rtl826x_config_intr,
		.handle_interrupt = rtl826x_handle_interrupt,
		.match_phy_device = rtl8264b_match_phy_device,
		.set_wol        = rtl826x_set_wol,
		.get_wol        = rtl826x_get_wol,
		.get_tunable    = rtl826x_get_tunable,
		.set_tunable    = rtl826x_set_tunable,
	},
};

module_phy_driver(realtek_drvs);

static const struct mdio_device_id __maybe_unused realtek_tbl[] = {
	{ PHY_ID_MATCH_VENDOR(0x001cc800) },
	{ }
};

MODULE_DEVICE_TABLE(mdio, realtek_tbl);
