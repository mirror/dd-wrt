// SPDX-License-Identifier: GPL-2.0-only
/* Realtek RTL838X Ethernet MDIO interface driver
 *
 * Copyright (C) 2020 B. Koblitz
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/firmware.h>
#include <linux/crc32.h>
#include <linux/sfp.h>
#include <linux/mii.h>
#include <linux/mdio.h>

#include <asm/mach-rtl838x/mach-rtl83xx.h>
#include "rtl83xx-phy.h"

extern struct rtl83xx_soc_info soc_info;
extern struct mutex smi_lock;
extern int phy_package_port_write_paged(struct phy_device *phydev, int port, int page, u32 regnum, u16 val);
extern int phy_package_write_paged(struct phy_device *phydev, int page, u32 regnum, u16 val);
extern int phy_port_write_paged(struct phy_device *phydev, int port, int page, u32 regnum, u16 val);
extern int phy_package_port_read_paged(struct phy_device *phydev, int port, int page, u32 regnum);
extern int phy_package_read_paged(struct phy_device *phydev, int page, u32 regnum);
extern int phy_port_read_paged(struct phy_device *phydev, int port, int page, u32 regnum);

#define PHY_PAGE_2	2
#define PHY_PAGE_4	4

/* all Clause-22 RealTek MDIO PHYs use register 0x1f for page select */
#define RTL8XXX_PAGE_SELECT		0x1f

#define RTL8XXX_PAGE_MAIN		0x0000
#define RTL821X_PAGE_PORT		0x0266
#define RTL821X_PAGE_POWER		0x0a40
#define RTL821X_PAGE_GPHY		0x0a42
#define RTL821X_PAGE_MAC		0x0a43
#define RTL821X_PAGE_STATE		0x0b80
#define RTL821X_PAGE_PATCH		0x0b82

/* Using the special page 0xfff with the MDIO controller found in
 * RealTek SoCs allows to access the PHY in RAW mode, ie. bypassing
 * the cache and paging engine of the MDIO controller.
 */
#define RTL838X_PAGE_RAW		0x0fff
#define RTL839X_PAGE_RAW		0x1fff

/* internal RTL821X PHY uses register 0x1d to select media page */
#define RTL821XINT_MEDIA_PAGE_SELECT	0x1d
/* external RTL821X PHY uses register 0x1e to select media page */
#define RTL821XEXT_MEDIA_PAGE_SELECT	0x1e
#define RTL821X_PHYCR2			0x19
#define RTL821X_PHYCR2_PHY_EEE_ENABLE	BIT(5)

#define RTL821X_CHIP_ID			0x6276

#define RTL821X_MEDIA_PAGE_AUTO		0
#define RTL821X_MEDIA_PAGE_COPPER	1
#define RTL821X_MEDIA_PAGE_FIBRE	3
#define RTL821X_MEDIA_PAGE_INTERNAL	8

#define RTL9300_PHY_ID_MASK 0xf0ffffff

/* RTL930X SerDes supports the following modes:
 * 0x02: SGMII		0x04: 1000BX_FIBER	0x05: FIBER100
 * 0x06: QSGMII		0x09: RSGMII		0x0d: USXGMII
 * 0x10: XSGMII		0x12: HISGMII		0x16: 2500Base_X
 * 0x17: RXAUI_LITE	0x19: RXAUI_PLUS	0x1a: 10G Base-R
 * 0x1b: 10GR1000BX_AUTO			0x1f: OFF
 */
#define RTL930X_SDS_MODE_SGMII		0x02
#define RTL930X_SDS_MODE_1000BASEX	0x04
#define RTL930X_SDS_MODE_USXGMII	0x0d
#define RTL930X_SDS_MODE_XGMII		0x10
#define RTL930X_SDS_MODE_HSGMII		0x12
#define RTL930X_SDS_MODE_2500BASEX	0x16
#define RTL930X_SDS_MODE_10GBASER	0x1a
#define RTL930X_SDS_OFF			0x1f
#define RTL930X_SDS_MASK		0x1f

#define RTSDS_930X_PLL_1000		0x1
#define RTSDS_930X_PLL_10000		0x5
#define RTSDS_930X_PLL_2500		0x3
#define RTSDS_930X_PLL_LC		0x3
#define RTSDS_930X_PLL_RING		0x1

/* This lock protects the state of the SoC automatically polling the PHYs over the SMI
 * bus to detect e.g. link and media changes. For operations on the PHYs such as
 * patching or other configuration changes such as EEE, polling needs to be disabled
 * since otherwise these operations may fails or lead to unpredictable results.
 */
DEFINE_MUTEX(poll_lock);

static const struct firmware rtl838x_8380_fw;
static const struct firmware rtl838x_8214fc_fw;
static const struct firmware rtl838x_8218b_fw;

static u64 disable_polling(int port)
{
	u64 saved_state;

	mutex_lock(&poll_lock);

	switch (soc_info.family) {
	case RTL8380_FAMILY_ID:
		saved_state = sw_r32(RTL838X_SMI_POLL_CTRL);
		sw_w32_mask(BIT(port), 0, RTL838X_SMI_POLL_CTRL);
		break;
	case RTL8390_FAMILY_ID:
		saved_state = sw_r32(RTL839X_SMI_PORT_POLLING_CTRL + 4);
		saved_state <<= 32;
		saved_state |= sw_r32(RTL839X_SMI_PORT_POLLING_CTRL);
		sw_w32_mask(BIT(port % 32), 0,
		            RTL839X_SMI_PORT_POLLING_CTRL + ((port >> 5) << 2));
		break;
	case RTL9300_FAMILY_ID:
		saved_state = sw_r32(RTL930X_SMI_POLL_CTRL);
		sw_w32_mask(BIT(port), 0, RTL930X_SMI_POLL_CTRL);
		break;
	case RTL9310_FAMILY_ID:
		pr_warn("%s not implemented for RTL931X\n", __func__);
		break;
	}

	mutex_unlock(&poll_lock);

	return saved_state;
}

static int resume_polling(u64 saved_state)
{
	mutex_lock(&poll_lock);

	switch (soc_info.family) {
	case RTL8380_FAMILY_ID:
		sw_w32(saved_state, RTL838X_SMI_POLL_CTRL);
		break;
	case RTL8390_FAMILY_ID:
		sw_w32(saved_state >> 32, RTL839X_SMI_PORT_POLLING_CTRL + 4);
		sw_w32(saved_state, RTL839X_SMI_PORT_POLLING_CTRL);
		break;
	case RTL9300_FAMILY_ID:
		sw_w32(saved_state, RTL930X_SMI_POLL_CTRL);
		break;
	case RTL9310_FAMILY_ID:
		pr_warn("%s not implemented for RTL931X\n", __func__);
		break;
	}

	mutex_unlock(&poll_lock);

	return 0;
}

static int rtl821x_match_phy_device(struct phy_device *phydev)
{
	u64 poll_state;
	int rawpage, port = phydev->mdio.addr & ~3;
	int oldpage, chip_mode, chip_cfg_mode;

	if (phydev->phy_id == PHY_ID_RTL8218B_E)
		return PHY_IS_RTL8218B_E;

	if (phydev->phy_id != PHY_ID_RTL8214_OR_8218)
		return PHY_IS_NOT_RTL821X;

	if (soc_info.family == RTL8380_FAMILY_ID)
		rawpage = RTL838X_PAGE_RAW;
	else if (soc_info.family == RTL8390_FAMILY_ID)
		rawpage = RTL839X_PAGE_RAW;
	else
		return PHY_IS_NOT_RTL821X;

	poll_state = disable_polling(port);
	/*
	 * At this stage the write_page()/read_page() PHY functions are not yet
	 * registered and normal paged access is not possible. The following
	 * detection routine works because our MDIO bus has all the Realtek
	 * PHY page handling (register 31) integrated into the port functions.
	 */
	oldpage = phy_port_read_paged(phydev, port, rawpage, 31);
	phy_port_write_paged(phydev, port, rawpage, 31, 0xa42);
	phy_port_write_paged(phydev, port, rawpage, 29, 0x008);
	phy_port_write_paged(phydev, port, rawpage, 31, 0x278);
	phy_port_write_paged(phydev, port, rawpage, 18, 0x455);
	phy_port_write_paged(phydev, port, rawpage, 31, 0x260);
	chip_mode = phy_port_read_paged(phydev, port, rawpage, 18);
	phy_port_write_paged(phydev, port, rawpage, 31, 0xa42);
	phy_port_write_paged(phydev, port, rawpage, 29, 0x000);
	phy_port_write_paged(phydev, port, rawpage, 31, oldpage);

	resume_polling(poll_state);

	pr_debug("%s(%d): got chip mode %x\n", __func__, phydev->mdio.addr, chip_mode);

	/* we checked the 4th port of a RTL8218B and got no config values */
	if (!chip_mode)
		return PHY_IS_RTL8218B_E;

	chip_cfg_mode = (chip_mode >> 4) & 0xf;
	chip_mode &= 0xf;

	if (chip_mode == 0xd || chip_mode == 0xf)
		return PHY_IS_RTL8218B_E;

	if (chip_mode == 0x4 || chip_mode == 0x6)
		return PHY_IS_RTL8214FC;

	if (chip_mode != 0xc && chip_mode != 0xe)
		return PHY_IS_NOT_RTL821X;

	if (chip_cfg_mode == 0x4 || chip_cfg_mode == 0x6)
		return PHY_IS_RTL8214FC;

	return PHY_IS_RTL8214FB;
}

static int rtl8218b_ext_match_phy_device(struct phy_device *phydev)
{
	return rtl821x_match_phy_device(phydev) == PHY_IS_RTL8218B_E;
}

static int rtl8214fc_match_phy_device(struct phy_device *phydev)
{
	return rtl821x_match_phy_device(phydev) == PHY_IS_RTL8214FC;
}

static void rtl8380_int_phy_on_off(struct phy_device *phydev, bool on)
{
	phy_modify(phydev, 0, BMCR_PDOWN, on ? 0 : BMCR_PDOWN);
}

static void rtl8380_phy_reset(struct phy_device *phydev)
{
	phy_modify(phydev, 0, BMCR_RESET, BMCR_RESET);
}

/* Read the link and speed status of the 2 internal SGMII/1000Base-X
 * ports of the RTL838x SoCs
 */
static int rtl8380_read_status(struct phy_device *phydev)
{
	int err;

	err = genphy_read_status(phydev);

	if (phydev->link) {
		phydev->speed = SPEED_1000;
		phydev->duplex = DUPLEX_FULL;
	}

	return err;
}

/* Read the link and speed status of the 2 internal SGMII/1000Base-X
 * ports of the RTL8393 SoC
 */
static int rtl8393_read_status(struct phy_device *phydev)
{
	int offset = 0;
	int err;
	int phy_addr = phydev->mdio.addr;
	u32 v;

	err = genphy_read_status(phydev);
	if (phy_addr == 49)
		offset = 0x100;

	if (phydev->link) {
		phydev->speed = SPEED_100;
		/* Read SPD_RD_00 (bit 13) and SPD_RD_01 (bit 6) out of the internal
		 * PHY registers
		 */
		v = sw_r32(RTL839X_SDS12_13_XSG0 + offset + 0x80);
		if (!(v & (1 << 13)) && (v & (1 << 6)))
			phydev->speed = SPEED_1000;
		phydev->duplex = DUPLEX_FULL;
	}

	return err;
}

static int rtl821x_read_page(struct phy_device *phydev)
{
	return __phy_read(phydev, RTL8XXX_PAGE_SELECT);
}

static int rtl821x_write_page(struct phy_device *phydev, int page)
{
	return __phy_write(phydev, RTL8XXX_PAGE_SELECT, page);
}

static int rtl8226_read_status(struct phy_device *phydev)
{
	int ret = 0;
	u32 val;

/* TODO: ret = genphy_read_status(phydev);
 * 	if (ret < 0) {
 * 		pr_info("%s: genphy_read_status failed\n", __func__);
 * 		return ret;
 * 	}
 */

	/* Link status must be read twice */
	for (int i = 0; i < 2; i++)
		val = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA402);

	phydev->link = val & BIT(2) ? 1 : 0;
	if (!phydev->link)
		goto out;

	/* Read duplex status */
	val = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA434);
	if (val < 0)
		goto out;
	phydev->duplex = !!(val & BIT(3));

	/* Read speed */
	val = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA434);
	switch (val & 0x0630) {
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

out:
	return ret;
}

static int rtl8226_advertise_aneg(struct phy_device *phydev)
{
	int ret = 0;
	u32 v;

	pr_info("In %s\n", __func__);

	v = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE);
	if (v < 0)
		goto out;

	v |= ADVERTISE_10HALF;
	v |= ADVERTISE_10FULL;
	v |= ADVERTISE_100HALF;
	v |= ADVERTISE_100FULL;

	ret = phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE, v);

	/* Allow 1GBit */
	v = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA412);
	if (v < 0)
		goto out;
	v |= ADVERTISE_1000FULL;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, 0xA412, v);
	if (ret < 0)
		goto out;

	/* Allow 2.5G */
	v = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_10GBT_CTRL);
	if (v < 0)
		goto out;

	v |= MDIO_AN_10GBT_CTRL_ADV2_5G;
	ret = phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_10GBT_CTRL, v);

out:
	return ret;
}

static int rtl8226_config_aneg(struct phy_device *phydev)
{
	int ret = 0;
	u32 v;

	pr_debug("In %s\n", __func__);
	if (phydev->autoneg == AUTONEG_ENABLE) {
		ret = rtl8226_advertise_aneg(phydev);
		if (ret)
			goto out;
		/* AutoNegotiationEnable */
		v = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_CTRL1);
		if (v < 0)
			goto out;

		v |= MDIO_AN_CTRL1_ENABLE; /* Enable AN */
		ret = phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_CTRL1, v);
		if (ret < 0)
			goto out;

		/* RestartAutoNegotiation */
		v = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA400);
		if (v < 0)
			goto out;
		v |= MDIO_AN_CTRL1_RESTART;

		ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, 0xA400, v);
	}

/*	TODO: ret = __genphy_config_aneg(phydev, ret); */

out:
	return ret;
}

static int rtl8218b_set_eee(struct phy_device *phydev, struct ethtool_keee *e)
{
	int port = phydev->mdio.addr;
	u64 poll_state;
	u32 val;
	bool an_enabled;

	pr_debug("In %s, port %d, enabled %d\n", __func__, port, e->eee_enabled);

	poll_state = disable_polling(port);

	/* Set GPHY page to copper */
	phy_write(phydev, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);
	val = phy_read(phydev, MII_BMCR);
	an_enabled = val & BMCR_ANENABLE;

	if (e->eee_enabled) {
		/* 100/1000M EEE Capability */
		phy_write(phydev, 13, 0x0007);
		phy_write(phydev, 14, 0x003C);
		phy_write(phydev, 13, 0x4007);
		phy_write(phydev, 14, 0x0006);

		val = phy_read_paged(phydev, RTL821X_PAGE_MAC, 25);
		val |= BIT(4);
		phy_write_paged(phydev, RTL821X_PAGE_MAC, 25, val);
	} else {
		/* 100/1000M EEE Capability */
		phy_write(phydev, 13, 0x0007);
		phy_write(phydev, 14, 0x003C);
		phy_write(phydev, 13, 0x0007);
		phy_write(phydev, 14, 0x0000);

		val = phy_read_paged(phydev, RTL821X_PAGE_MAC, 25);
		val &= ~BIT(4);
		phy_write_paged(phydev, RTL821X_PAGE_MAC, 25, val);
	}

	/* Restart AN if enabled */
	if (an_enabled) {
		val = phy_read(phydev, MII_BMCR);
		val |= BMCR_ANRESTART;
		phy_write(phydev, MII_BMCR, val);
	}

	/* GPHY page back to auto */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	pr_debug("%s done\n", __func__);
	resume_polling(poll_state);

	return 0;
}

static int rtl8218b_get_eee(struct phy_device *phydev,
			    struct ethtool_keee *e)
{
	u32 val;
	int addr = phydev->mdio.addr;

	pr_debug("In %s, port %d, was enabled: %d\n", __func__, addr, e->eee_enabled);

	/* Set GPHY page to copper */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);

	val = phy_read_paged(phydev, 7, MDIO_AN_EEE_ADV);
	if (e->eee_enabled) {
		/* Verify vs MAC-based EEE */
		e->eee_enabled = !!(val & BIT(7));
		if (!e->eee_enabled) {
			val = phy_read_paged(phydev, RTL821X_PAGE_MAC, 25);
			e->eee_enabled = !!(val & BIT(4));
		}
	}
	pr_debug("%s: enabled: %d\n", __func__, e->eee_enabled);

	/* GPHY page to auto */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	return 0;
}

static bool __rtl8214fc_media_is_fibre(struct phy_device *phydev)
{
	struct mii_bus *bus = phydev->mdio.bus;
	static int regs[] = {16, 19, 20, 21};
	int addr = phydev->mdio.addr & ~3;
	int reg = regs[phydev->mdio.addr & 3];
	int oldpage, val;

	/*
	 * The fiber status cannot be read directly from the phy. It is a package "global"
	 * attribute and therefore located in the first phy. To avoid state handling assume
	 * an aligment to addresses divisible by 4.
	 */

	oldpage = __mdiobus_read(bus, addr, RTL8XXX_PAGE_SELECT);
	__mdiobus_write(bus, addr, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_INTERNAL);
	__mdiobus_write(bus, addr, RTL8XXX_PAGE_SELECT, RTL821X_PAGE_PORT);
	val = __mdiobus_read(bus, addr, reg);
	__mdiobus_write(bus, addr, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);
	__mdiobus_write(bus, addr, RTL8XXX_PAGE_SELECT, oldpage);

	return !(val & BMCR_PDOWN);
}

static bool rtl8214fc_media_is_fibre(struct phy_device *phydev)
{
	struct mii_bus *bus = phydev->mdio.bus;
	int ret;

	mutex_lock(&bus->mdio_lock);
	ret = __rtl8214fc_media_is_fibre(phydev);
	mutex_unlock(&bus->mdio_lock);

	return ret;
}

static int rtl8214fc_set_eee(struct phy_device *phydev,
			     struct ethtool_keee *e)
{
	u32 poll_state;
	int port = phydev->mdio.addr;
	bool an_enabled;
	u32 val;

	pr_debug("In %s port %d, enabled %d\n", __func__, port, e->eee_enabled);

	if (rtl8214fc_media_is_fibre(phydev)) {
		netdev_err(phydev->attached_dev, "Port %d configured for FIBRE", port);
		return -ENOTSUPP;
	}

	poll_state = disable_polling(port);

	/* Set GPHY page to copper */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);

	/* Get auto-negotiation status */
	val = phy_read(phydev, MII_BMCR);
	an_enabled = val & BMCR_ANENABLE;

	pr_debug("%s: aneg: %d\n", __func__, an_enabled);
	val = phy_read_paged(phydev, RTL821X_PAGE_MAC, 25);
	val &= ~BIT(5);  /* Use MAC-based EEE */
	phy_write_paged(phydev, RTL821X_PAGE_MAC, 25, val);

	/* Enable 100M (bit 1) / 1000M (bit 2) EEE */
	phy_write_paged(phydev, 7, MDIO_AN_EEE_ADV, e->eee_enabled ? (MDIO_EEE_100TX | MDIO_EEE_1000T) : 0);

	/* 500M EEE ability */
	val = phy_read_paged(phydev, RTL821X_PAGE_GPHY, 20);
	if (e->eee_enabled)
		val |= BIT(7);
	else
		val &= ~BIT(7);

	phy_write_paged(phydev, RTL821X_PAGE_GPHY, 20, val);

	/* Restart AN if enabled */
	if (an_enabled) {
		pr_debug("%s: doing aneg\n", __func__);
		val = phy_read(phydev, MII_BMCR);
		val |= BMCR_ANRESTART;
		phy_write(phydev, MII_BMCR, val);
	}

	/* GPHY page back to auto */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	resume_polling(poll_state);

	return 0;
}

static int rtl8214fc_get_eee(struct phy_device *phydev,
			     struct ethtool_keee *e)
{
	int addr = phydev->mdio.addr;

	pr_debug("In %s port %d, enabled %d\n", __func__, addr, e->eee_enabled);
	if (rtl8214fc_media_is_fibre(phydev)) {
		netdev_err(phydev->attached_dev, "Port %d configured for FIBRE", addr);
		return -ENOTSUPP;
	}

	return rtl8218b_get_eee(phydev, e);
}

/* Enable EEE on the RTL8218B PHYs
 * The method used is not the preferred way (which would be based on the MAC-EEE state,
 * but the only way that works since the kernel first enables EEE in the MAC
 * and then sets up the PHY. The MAC-based approach would require the oppsite.
 */
static void rtl8218d_eee_set(struct phy_device *phydev, bool enable)
{
	u32 val;
	bool an_enabled;

	pr_debug("In %s %d, enable %d\n", __func__, phydev->mdio.addr, enable);
	/* Set GPHY page to copper */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);

	val = phy_read(phydev, MII_BMCR);
	an_enabled = val & BMCR_ANENABLE;

	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV);
	val |= MDIO_EEE_1000T | MDIO_EEE_100TX;
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV, enable ? (MDIO_EEE_100TX | MDIO_EEE_1000T) : 0);

	/* 500M EEE ability */
	val = phy_read_paged(phydev, RTL821X_PAGE_GPHY, 20);
	if (enable)
		val |= BIT(7);
	else
		val &= ~BIT(7);
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, 20, val);

	/* Restart AN if enabled */
	if (an_enabled) {
		val = phy_read(phydev, MII_BMCR);
		val |= BMCR_ANRESTART;
		phy_write(phydev, MII_BMCR, val);
	}

	/* GPHY page back to auto */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);
}

static int rtl8218d_set_eee(struct phy_device *phydev, struct ethtool_keee *e)
{
	int addr = phydev->mdio.addr;
	u64 poll_state;

	pr_debug("In %s, port %d, enabled %d\n", __func__, addr, e->eee_enabled);

	poll_state = disable_polling(addr);

	rtl8218d_eee_set(phydev, (bool) e->eee_enabled);

	resume_polling(poll_state);

	return 0;
}



static int rtl8218d_get_eee(struct phy_device *phydev,
			    struct ethtool_keee *e)
{
	u32 val;
	int addr = phydev->mdio.addr;

	pr_debug("In %s, port %d, was enabled: %d\n", __func__, addr, e->eee_enabled);

	/* Set GPHY page to copper */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);

	val = phy_read_paged(phydev, 7, MDIO_AN_EEE_ADV);
	if (e->eee_enabled)
		e->eee_enabled = !!(val & BIT(7));
	pr_debug("%s: enabled: %d\n", __func__, e->eee_enabled);

	/* GPHY page to auto */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	return 0;
}

static int rtl8226_get_eee(struct phy_device *phydev, struct ethtool_keee *e)
{
	u32 val;
	int addr = phydev->mdio.addr;

	pr_debug("In %s, port %d, was enabled: %d\n", __func__, addr, e->eee_enabled);

	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV);
	if (e->eee_enabled) {
		e->eee_enabled = !!(val & MDIO_EEE_100TX);
		if (!e->eee_enabled) {
			val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV2);
			e->eee_enabled = !!(val & MDIO_EEE_2_5GT);
		}
	}
	pr_debug("%s: enabled: %d\n", __func__, e->eee_enabled);

	return 0;
}

static int rtl8226_set_eee(struct phy_device *phydev, struct ethtool_keee *e)
{
	int port = phydev->mdio.addr;
	u64 poll_state;
	bool an_enabled;
	u32 val;

	pr_info("In %s, port %d, enabled %d\n", __func__, port, e->eee_enabled);

	poll_state = disable_polling(port);

	/* Remember aneg state */
	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_CTRL1);
	an_enabled = !!(val & MDIO_AN_CTRL1_ENABLE);

	/* Setup 100/1000MBit */
	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV);
	if (e->eee_enabled)
		val |= (MDIO_EEE_100TX | MDIO_EEE_1000T);
	else
		val &= (MDIO_EEE_100TX | MDIO_EEE_1000T);
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV, val);

	/* Setup 2.5GBit */
	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV2);
	if (e->eee_enabled)
		val |= MDIO_EEE_2_5GT;
	else
		val &= MDIO_EEE_2_5GT;
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV2, val);

	/* RestartAutoNegotiation */
	val = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA400);
	val |= MDIO_AN_CTRL1_RESTART;
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0xA400, val);

	resume_polling(poll_state);

	return 0;
}

static struct fw_header *rtl838x_request_fw(struct phy_device *phydev,
					    const struct firmware *fw,
					    const char *name)
{
	struct device *dev = &phydev->mdio.dev;
	int err;
	struct fw_header *h;
	uint32_t checksum, my_checksum;

	err = request_firmware(&fw, name, dev);
	if (err < 0)
		goto out;

	if (fw->size < sizeof(struct fw_header)) {
		pr_err("Firmware size too small.\n");
		err = -EINVAL;
		goto out;
	}

	h = (struct fw_header *) fw->data;
	pr_info("Firmware loaded. Size %d, magic: %08x\n", fw->size, h->magic);

	if (h->magic != 0x83808380) {
		pr_err("Wrong firmware file: MAGIC mismatch.\n");
		goto out;
	}

	checksum = h->checksum;
	h->checksum = 0;
	my_checksum = ~crc32(0xFFFFFFFFU, fw->data, fw->size);
	if (checksum != my_checksum) {
		pr_err("Firmware checksum mismatch.\n");
		err = -EINVAL;
		goto out;
	}
	h->checksum = checksum;

	return h;
out:
	dev_err(dev, "Unable to load firmware %s (%d)\n", name, err);
	return NULL;
}

static void rtl821x_phy_setup_package_broadcast(struct phy_device *phydev, bool enable)
{
	int mac = phydev->mdio.addr;

	/* select main page 0 */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
	/* write to 0x8 to register 0x1d on main page 0 */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_INTERNAL);
	/* select page 0x266 */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL821X_PAGE_PORT);
	/* set phy id and target broadcast bitmap in register 0x16 on page 0x266 */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, 0x16, (enable?0xff00:0x00) | mac);
	/* return to main page 0 */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
	/* write to 0x0 to register 0x1d on main page 0 */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);
	mdelay(1);
}

static int rtl8390_configure_generic(struct phy_device *phydev)
{
	int mac = phydev->mdio.addr;
	u32 val, phy_id;

	val = phy_read(phydev, 2);
	phy_id = val << 16;
	val = phy_read(phydev, 3);
	phy_id |= val;
	pr_debug("Phy on MAC %d: %x\n", mac, phy_id);

	/* Read internal PHY ID */
	phy_write_paged(phydev, 31, 27, 0x0002);
	val = phy_read_paged(phydev, 31, 28);

	/* Internal RTL8218B, version 2 */
	phydev_info(phydev, "Detected unknown %x\n", val);

	return 0;
}

static int rtl8380_configure_int_rtl8218b(struct phy_device *phydev)
{
	u32 val, phy_id;
	int mac = phydev->mdio.addr;
	struct fw_header *h;
	u32 *rtl838x_6275B_intPhy_perport;
	u32 *rtl8218b_6276B_hwEsd_perport;

	val = phy_read(phydev, 2);
	phy_id = val << 16;
	val = phy_read(phydev, 3);
	phy_id |= val;
	pr_debug("Phy on MAC %d: %x\n", mac, phy_id);

	/* Read internal PHY ID */
	phy_write_paged(phydev, 31, 27, 0x0002);
	val = phy_read_paged(phydev, 31, 28);
	if (val != 0x6275) {
		phydev_err(phydev, "Expected internal RTL8218B, found PHY-ID %x\n", val);
		return -1;
	}

	/* Internal RTL8218B, version 2 */
	phydev_info(phydev, "Detected internal RTL8218B\n");

	h = rtl838x_request_fw(phydev, &rtl838x_8380_fw, FIRMWARE_838X_8380_1);
	if (!h)
		return -1;

	if (h->phy != 0x83800000) {
		phydev_err(phydev, "Wrong firmware file: PHY mismatch.\n");
		return -1;
	}

	rtl838x_6275B_intPhy_perport = (void *)h + sizeof(struct fw_header) + h->parts[8].start;
	rtl8218b_6276B_hwEsd_perport = (void *)h + sizeof(struct fw_header) + h->parts[9].start;

	// Currently not used
	// if (sw_r32(RTL838X_DMY_REG31) == 0x1) {
	// 	int ipd_flag = 1;
	// }

	val = phy_read(phydev, MII_BMCR);
	if (val & BMCR_PDOWN)
		rtl8380_int_phy_on_off(phydev, true);
	else
		rtl8380_phy_reset(phydev);
	msleep(100);

	/* Ready PHY for patch */
	for (int p = 0; p < 8; p++) {
		phy_package_port_write_paged(phydev, p, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL821X_PAGE_PATCH);
		phy_package_port_write_paged(phydev, p, RTL838X_PAGE_RAW, 0x10, 0x0010);
	}
	msleep(500);
	for (int p = 0; p < 8; p++) {
		int i;

		for (i = 0; i < 100 ; i++) {
			val = phy_package_port_read_paged(phydev, p, RTL821X_PAGE_STATE, 0x10);
			if (val & 0x40)
				break;
		}
		if (i >= 100) {
			phydev_err(phydev,
			           "ERROR: Port %d not ready for patch.\n",
			           mac + p);
			return -1;
		}
	}
	for (int p = 0; p < 8; p++) {
		int i;

		i = 0;
		while (rtl838x_6275B_intPhy_perport[i * 2]) {
			phy_package_port_write_paged(phydev, p, RTL838X_PAGE_RAW,
			                             rtl838x_6275B_intPhy_perport[i * 2],
			                             rtl838x_6275B_intPhy_perport[i * 2 + 1]);
			i++;
		}
		i = 0;
		while (rtl8218b_6276B_hwEsd_perport[i * 2]) {
			phy_package_port_write_paged(phydev, p, RTL838X_PAGE_RAW,
			                             rtl8218b_6276B_hwEsd_perport[i * 2],
			                             rtl8218b_6276B_hwEsd_perport[i * 2 + 1]);
			i++;
		}
	}

	return 0;
}

static int rtl8380_configure_ext_rtl8218b(struct phy_device *phydev)
{
	u32 val, ipd, phy_id;
	int mac = phydev->mdio.addr;
	struct fw_header *h;
	u32 *rtl8380_rtl8218b_perchip;
	u32 *rtl8218B_6276B_rtl8380_perport;
	u32 *rtl8380_rtl8218b_perport;

	if (soc_info.family == RTL8380_FAMILY_ID && mac != 0 && mac != 16) {
		phydev_err(phydev, "External RTL8218B must have PHY-IDs 0 or 16!\n");
		return -1;
	}
	val = phy_read(phydev, 2);
	phy_id = val << 16;
	val = phy_read(phydev, 3);
	phy_id |= val;
	pr_info("Phy on MAC %d: %x\n", mac, phy_id);

	/* Read internal PHY ID */
	phy_write_paged(phydev, 31, 27, 0x0002);
	val = phy_read_paged(phydev, 31, 28);
	if (val != RTL821X_CHIP_ID) {
		phydev_err(phydev, "Expected external RTL8218B, found PHY-ID %x\n", val);
		return -1;
	}
	phydev_info(phydev, "Detected external RTL8218B\n");

	h = rtl838x_request_fw(phydev, &rtl838x_8218b_fw, FIRMWARE_838X_8218b_1);
	if (!h)
		return -1;

	if (h->phy != 0x8218b000) {
		phydev_err(phydev, "Wrong firmware file: PHY mismatch.\n");
		return -1;
	}

	rtl8380_rtl8218b_perchip = (void *)h + sizeof(struct fw_header) + h->parts[0].start;
	rtl8218B_6276B_rtl8380_perport = (void *)h + sizeof(struct fw_header) + h->parts[1].start;
	rtl8380_rtl8218b_perport = (void *)h + sizeof(struct fw_header) + h->parts[2].start;

	val = phy_read(phydev, MII_BMCR);
	if (val & BMCR_PDOWN)
		rtl8380_int_phy_on_off(phydev, true);
	else
		rtl8380_phy_reset(phydev);

	msleep(100);

	/* Get Chip revision */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
	phy_write_paged(phydev, RTL838X_PAGE_RAW, 0x1b, 0x4);
	val = phy_read_paged(phydev, RTL838X_PAGE_RAW, 0x1c);

	phydev_info(phydev, "Detected chip revision %04x\n", val);

	for (int i = 0; rtl8380_rtl8218b_perchip[i * 3] &&
	                rtl8380_rtl8218b_perchip[i * 3 + 1]; i++) {
		phy_package_port_write_paged(phydev, rtl8380_rtl8218b_perchip[i * 3],
					     RTL838X_PAGE_RAW, rtl8380_rtl8218b_perchip[i * 3 + 1],
					     rtl8380_rtl8218b_perchip[i * 3 + 2]);
	}

	/* Enable PHY */
	for (int i = 0; i < 8; i++) {
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, 0x00, 0x1140);
	}
	mdelay(100);

	/* Request patch */
	for (int i = 0; i < 8; i++) {
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL821X_PAGE_PATCH);
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, 0x10, 0x0010);
	}

	mdelay(300);

	/* Verify patch readiness */
	for (int i = 0; i < 8; i++) {
		int l;

		for (l = 0; l < 100; l++) {
			val = phy_package_port_read_paged(phydev, i, RTL821X_PAGE_STATE, 0x10);
			if (val & 0x40)
				break;
		}
		if (l >= 100) {
			phydev_err(phydev, "Could not patch PHY\n");
			return -1;
		}
	}

	/* Use Broadcast ID method for patching */
	rtl821x_phy_setup_package_broadcast(phydev, true);

	phy_write_paged(phydev, RTL838X_PAGE_RAW, 30, 8);
	phy_write_paged(phydev, 0x26e, 17, 0xb);
	phy_write_paged(phydev, 0x26e, 16, 0x2);
	mdelay(1);
	ipd = phy_read_paged(phydev, 0x26e, 19);
	phy_write_paged(phydev, 0, 30, 0);
	ipd = (ipd >> 4) & 0xf; /* unused ? */

	for (int i = 0; rtl8218B_6276B_rtl8380_perport[i * 2]; i++) {
		phy_write_paged(phydev, RTL838X_PAGE_RAW, rtl8218B_6276B_rtl8380_perport[i * 2],
		                rtl8218B_6276B_rtl8380_perport[i * 2 + 1]);
	}

	/* Disable broadcast ID */
	rtl821x_phy_setup_package_broadcast(phydev, false);

	return 0;
}

static void rtl8214fc_power_set(struct phy_device *phydev, int port, bool on)
{
	int page = port == PORT_FIBRE ? RTL821X_MEDIA_PAGE_FIBRE : RTL821X_MEDIA_PAGE_COPPER;
	int pdown = on ? 0 : BMCR_PDOWN;

	pr_info("%s: Powering %s %s (port %d)\n", __func__,
		on ? "on" : "off",
		port == PORT_FIBRE ? "FIBRE" : "COPPER",
		phydev->mdio.addr);

	phy_write(phydev, RTL821XINT_MEDIA_PAGE_SELECT, page);
	phy_modify_paged(phydev, RTL821X_PAGE_POWER, 0x10, BMCR_PDOWN, pdown);
	phy_write(phydev, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);
}

static int rtl8214fc_suspend(struct phy_device *phydev)
{
	rtl8214fc_power_set(phydev, PORT_MII, false);
	rtl8214fc_power_set(phydev, PORT_FIBRE, false);

	return 0;
}

static int rtl8214fc_resume(struct phy_device *phydev)
{
	if (rtl8214fc_media_is_fibre(phydev)) {
		rtl8214fc_power_set(phydev, PORT_MII, false);
		rtl8214fc_power_set(phydev, PORT_FIBRE, true);
	} else {
		rtl8214fc_power_set(phydev, PORT_FIBRE, false);
		rtl8214fc_power_set(phydev, PORT_MII, true);
	}

	return 0;
}

static void rtl8214fc_media_set(struct phy_device *phydev, bool set_fibre)
{
	int mac = phydev->mdio.addr;

	static int reg[] = {16, 19, 20, 21};
	int val;

	pr_info("%s: port %d, set_fibre: %d\n", __func__, mac, set_fibre);
	phy_package_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_INTERNAL);
	val = phy_package_read_paged(phydev, RTL821X_PAGE_PORT, reg[mac % 4]);

	val |= BIT(10);
	if (set_fibre) {
		val &= ~BMCR_PDOWN;
	} else {
		val |= BMCR_PDOWN;
	}

	phy_package_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_INTERNAL);
	phy_package_write_paged(phydev, RTL821X_PAGE_PORT, reg[mac % 4], val);
	phy_package_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	if (!phydev->suspended) {
		if (set_fibre) {
			rtl8214fc_power_set(phydev, PORT_MII, false);
			rtl8214fc_power_set(phydev, PORT_FIBRE, true);
		} else {
			rtl8214fc_power_set(phydev, PORT_FIBRE, false);
			rtl8214fc_power_set(phydev, PORT_MII, true);
		}
	}
}

static int rtl8214fc_set_tunable(struct phy_device *phydev,
				 struct ethtool_tunable *tuna, const void *data)
{
	/*
	 * The RTL8214FC driver usually detects insertion of SFP modules and automatically toggles
	 * between copper and fiber. There may be cases where the user wants to switch the port on
	 * demand. Usually ethtool offers to change the port of a multiport network card with
	 * "ethtool -s lan25 port fibre/tp" if the driver supports it. This does not work for
	 * attached phys. For more details see phy_ethtool_ksettings_set(). To avoid patching the
	 * kernel misuse the phy downshift tunable to offer that feature. For this use
	 * "ethtool --set-phy-tunable lan25 downshift on/off".
	 */
	switch (tuna->id) {
	case ETHTOOL_PHY_DOWNSHIFT:
		rtl8214fc_media_set(phydev, !rtl8214fc_media_is_fibre(phydev));
		return 0;
	default:
		return -EOPNOTSUPP;
	}
}

static int rtl8214fc_get_tunable(struct phy_device *phydev,
				 struct ethtool_tunable *tuna, void *data)
{
	/* Needed to make rtl8214fc_set_tunable() work */
	return 0;
}

static int rtl8214fc_get_features(struct phy_device *phydev)
{
	int ret = 0;

	ret = genphy_read_abilities(phydev);
	if (ret)
		return ret;
	/*
	 * The RTL8214FC only advertises TP capabilities in the standard registers. This is
	 * independent from what fibre/copper combination is currently activated. For now just
	 * announce the superset of all possible features.
	 */
	linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseX_Full_BIT, phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_FIBRE_BIT, phydev->supported);

	return 0;
}

static int rtl8214fc_read_status(struct phy_device *phydev)
{
	bool changed;
	int ret;

	if (rtl8214fc_media_is_fibre(phydev)) {
		phydev->port = PORT_FIBRE;
		ret = genphy_c37_read_status(phydev, &changed);
	} else {
		phydev->port = PORT_MII; /* for now aligend with rest of code */
		ret = genphy_read_status(phydev);
	}

	return ret;
}

static int rtl8214fc_config_aneg(struct phy_device *phydev)
{
	int ret;

	if (rtl8214fc_media_is_fibre(phydev))
		ret = genphy_c37_config_aneg(phydev);
	else
		ret = genphy_config_aneg(phydev);

	return ret;
}

static int rtl821x_read_mmd(struct phy_device *phydev, int devnum, u16 regnum)
{
	struct mii_bus *bus = phydev->mdio.bus;
	int addr = phydev->mdio.addr;
	int ret;

	/*
	 * The RTL821x PHYs are usually only C22 capable and are defined accordingly in DTS.
	 * Nevertheless GPL source drops clearly indicate that EEE features can be accessed
	 * directly via C45. Testing shows that C45 over C22 (as used in kernel EEE framework)
	 * works as well but only as long as PHY polling is disabled in the SOC. To avoid ugly
	 * hacks pass through C45 accesses for important EEE registers. Maybe some day the mdio
	 * bus can intercept these patterns and switch off/on polling on demand. That way this
	 * phy device driver can avoid handling special cases on its own.
	 */

	if ((devnum == MDIO_MMD_PCS && regnum == MDIO_PCS_EEE_ABLE) ||
	    (devnum == MDIO_MMD_AN && regnum == MDIO_AN_EEE_ADV) ||
	    (devnum == MDIO_MMD_AN && regnum == MDIO_AN_EEE_LPABLE))
		ret = __mdiobus_c45_read(bus, addr, devnum, regnum);
	else
		ret = -EOPNOTSUPP;

	return ret;
}

static int rtl821x_write_mmd(struct phy_device *phydev, int devnum, u16 regnum, u16 val)
{
	struct mii_bus *bus = phydev->mdio.bus;
	int addr = phydev->mdio.addr;
	int ret;

	/* see rtl821x_read_mmd() */
	if (devnum == MDIO_MMD_AN && regnum == MDIO_AN_EEE_ADV)
		ret = __mdiobus_c45_write(bus, addr, devnum, regnum, val);
	else
		ret = -EOPNOTSUPP;

	return ret;
}

static int rtl8214fc_read_mmd(struct phy_device *phydev, int devnum, u16 regnum)
{
	if (__rtl8214fc_media_is_fibre(phydev))
		return -EOPNOTSUPP;

	return rtl821x_read_mmd(phydev, devnum, regnum);
}

static int rtl8214fc_write_mmd(struct phy_device *phydev, int devnum, u16 regnum, u16 val)
{
	if (__rtl8214fc_media_is_fibre(phydev))
		return -EOPNOTSUPP;

	return rtl821x_write_mmd(phydev, devnum, regnum, val);
}

static int rtl8380_configure_rtl8214c(struct phy_device *phydev)
{
	u32 phy_id, val;
	int mac = phydev->mdio.addr;

	val = phy_read(phydev, 2);
	phy_id = val << 16;
	val = phy_read(phydev, 3);
	phy_id |= val;
	pr_debug("Phy on MAC %d: %x\n", mac, phy_id);

	phydev_info(phydev, "Detected external RTL8214C\n");

	/* GPHY auto conf */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	return 0;
}

static int rtl8380_configure_rtl8214fc(struct phy_device *phydev)
{
	int mac = phydev->mdio.addr;
	struct fw_header *h;
	u32 *rtl8380_rtl8214fc_perchip;
	u32 *rtl8380_rtl8214fc_perport;
	u32 phy_id;
	u32 val;

	val = phy_read(phydev, 2);
	phy_id = val << 16;
	val = phy_read(phydev, 3);
	phy_id |= val;
	pr_debug("Phy on MAC %d: %x\n", mac, phy_id);

	/* Read internal PHY id */
	phy_write_paged(phydev, 0, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);
	phy_write_paged(phydev, 0x1f, 0x1b, 0x0002);
	val = phy_read_paged(phydev, 0x1f, 0x1c);
	if (val != RTL821X_CHIP_ID) {
		phydev_err(phydev, "Expected external RTL8214FC, found PHY-ID %x\n", val);
		return -1;
	}
	phydev_info(phydev, "Detected external RTL8214FC\n");

	h = rtl838x_request_fw(phydev, &rtl838x_8214fc_fw, FIRMWARE_838X_8214FC_1);
	if (!h)
		return -1;

	if (h->phy != 0x8214fc00) {
		phydev_err(phydev, "Wrong firmware file: PHY mismatch.\n");
		return -1;
	}

	rtl8380_rtl8214fc_perchip = (void *)h + sizeof(struct fw_header) + h->parts[0].start;

	rtl8380_rtl8214fc_perport = (void *)h + sizeof(struct fw_header) + h->parts[1].start;

	/* detect phy version */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, 27, 0x0004);
	val = phy_read_paged(phydev, RTL838X_PAGE_RAW, 28);

	val = phy_read(phydev, 16);
	if (val & BMCR_PDOWN) {
		rtl8214fc_power_set(phydev, PORT_MII, true);
		rtl8214fc_power_set(phydev, PORT_FIBRE, true);
	} else
		rtl8380_phy_reset(phydev);

	msleep(100);
	phy_write_paged(phydev, 0, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);

	for (int i = 0; rtl8380_rtl8214fc_perchip[i * 3] &&
	                rtl8380_rtl8214fc_perchip[i * 3 + 1]; i++) {
		u32 page = 0;

		if (rtl8380_rtl8214fc_perchip[i * 3 + 1] == 0x1f)
			page = rtl8380_rtl8214fc_perchip[i * 3 + 2];
		if (rtl8380_rtl8214fc_perchip[i * 3 + 1] == 0x13 && page == 0x260) {
			val = phy_read_paged(phydev, 0x260, 13);
			val = (val & 0x1f00) | (rtl8380_rtl8214fc_perchip[i * 3 + 2] & 0xe0ff);
			phy_write_paged(phydev, RTL838X_PAGE_RAW,
					rtl8380_rtl8214fc_perchip[i * 3 + 1], val);
		} else {
			phy_write_paged(phydev, RTL838X_PAGE_RAW,
					rtl8380_rtl8214fc_perchip[i * 3 + 1],
					rtl8380_rtl8214fc_perchip[i * 3 + 2]);
		}
	}

	/* Force copper medium */
	for (int i = 0; i < 4; i++) {
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);
	}

	/* Enable PHY */
	for (int i = 0; i < 4; i++) {
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, 0x00, 0x1140);
	}
	mdelay(100);

	/* Disable Autosensing */
	for (int i = 0; i < 4; i++) {
		int l;

		for (l = 0; l < 100; l++) {
			val = phy_package_port_read_paged(phydev, i, RTL821X_PAGE_GPHY, 0x10);
			if ((val & 0x7) >= 3)
				break;
		}
		if (l >= 100) {
			phydev_err(phydev, "Could not disable autosensing\n");
			return -1;
		}
	}

	/* Request patch */
	for (int i = 0; i < 4; i++) {
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL821X_PAGE_PATCH);
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, 0x10, 0x0010);
	}
	mdelay(300);

	/* Verify patch readiness */
	for (int i = 0; i < 4; i++) {
		int l;

		for (l = 0; l < 100; l++) {
			val = phy_package_port_read_paged(phydev, i, RTL821X_PAGE_STATE, 0x10);
			if (val & 0x40)
				break;
		}
		if (l >= 100) {
			phydev_err(phydev, "Could not patch PHY\n");
			return -1;
		}
	}
	/* Use Broadcast ID method for patching */
	rtl821x_phy_setup_package_broadcast(phydev, true);

	for (int i = 0; rtl8380_rtl8214fc_perport[i * 2]; i++) {
		phy_write_paged(phydev, RTL838X_PAGE_RAW, rtl8380_rtl8214fc_perport[i * 2],
		                rtl8380_rtl8214fc_perport[i * 2 + 1]);
	}

	/* Disable broadcast ID */
	rtl821x_phy_setup_package_broadcast(phydev, false);

	/* Auto medium selection */
	for (int i = 0; i < 4; i++) {
		phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
		phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);
	}

	return 0;
}

static int rtl8380_configure_serdes(struct phy_device *phydev)
{
	u32 v;
	u32 sds_conf_value;
	int i;
	struct fw_header *h;
	u32 *rtl8380_sds_take_reset;
	u32 *rtl8380_sds_common;
	u32 *rtl8380_sds01_qsgmii_6275b;
	u32 *rtl8380_sds23_qsgmii_6275b;
	u32 *rtl8380_sds4_fiber_6275b;
	u32 *rtl8380_sds5_fiber_6275b;
	u32 *rtl8380_sds_reset;
	u32 *rtl8380_sds_release_reset;

	phydev_info(phydev, "Detected internal RTL8380 SERDES\n");

	h = rtl838x_request_fw(phydev, &rtl838x_8218b_fw, FIRMWARE_838X_8380_1);
	if (!h)
		return -1;

	if (h->magic != 0x83808380) {
		phydev_err(phydev, "Wrong firmware file: magic number mismatch.\n");
		return -1;
	}

	rtl8380_sds_take_reset = (void *)h + sizeof(struct fw_header) + h->parts[0].start;

	rtl8380_sds_common = (void *)h + sizeof(struct fw_header) + h->parts[1].start;

	rtl8380_sds01_qsgmii_6275b = (void *)h + sizeof(struct fw_header) + h->parts[2].start;

	rtl8380_sds23_qsgmii_6275b = (void *)h + sizeof(struct fw_header) + h->parts[3].start;

	rtl8380_sds4_fiber_6275b = (void *)h + sizeof(struct fw_header) + h->parts[4].start;

	rtl8380_sds5_fiber_6275b = (void *)h + sizeof(struct fw_header) + h->parts[5].start;

	rtl8380_sds_reset = (void *)h + sizeof(struct fw_header) + h->parts[6].start;

	rtl8380_sds_release_reset = (void *)h + sizeof(struct fw_header) + h->parts[7].start;

	/* Back up serdes power off value */
	sds_conf_value = sw_r32(RTL838X_SDS_CFG_REG);
	pr_info("SDS power down value: %x\n", sds_conf_value);

	/* take serdes into reset */
	i = 0;
	while (rtl8380_sds_take_reset[2 * i]) {
		sw_w32(rtl8380_sds_take_reset[2 * i + 1], rtl8380_sds_take_reset[2 * i]);
		i++;
		udelay(1000);
	}

	/* apply common serdes patch */
	i = 0;
	while (rtl8380_sds_common[2 * i]) {
		sw_w32(rtl8380_sds_common[2 * i + 1], rtl8380_sds_common[2 * i]);
		i++;
		udelay(1000);
	}

	/* internal R/W enable */
	sw_w32(3, RTL838X_INT_RW_CTRL);

	/* SerDes ports 4 and 5 are FIBRE ports */
	sw_w32_mask(0x7 | 0x38, 1 | (1 << 3), RTL838X_INT_MODE_CTRL);

	/* SerDes module settings, SerDes 0-3 are QSGMII */
	v = 0x6 << 25 | 0x6 << 20 | 0x6 << 15 | 0x6 << 10;
	/* SerDes 4 and 5 are 1000BX FIBRE */
	v |= 0x4 << 5 | 0x4;
	sw_w32(v, RTL838X_SDS_MODE_SEL);

	pr_info("PLL control register: %x\n", sw_r32(RTL838X_PLL_CML_CTRL));
	sw_w32_mask(0xfffffff0, 0xaaaaaaaf & 0xf, RTL838X_PLL_CML_CTRL);
	i = 0;
	while (rtl8380_sds01_qsgmii_6275b[2 * i]) {
		sw_w32(rtl8380_sds01_qsgmii_6275b[2 * i + 1],
		       rtl8380_sds01_qsgmii_6275b[2 * i]);
		i++;
	}

	i = 0;
	while (rtl8380_sds23_qsgmii_6275b[2 * i]) {
		sw_w32(rtl8380_sds23_qsgmii_6275b[2 * i + 1], rtl8380_sds23_qsgmii_6275b[2 * i]);
		i++;
	}

	i = 0;
	while (rtl8380_sds4_fiber_6275b[2 * i]) {
		sw_w32(rtl8380_sds4_fiber_6275b[2 * i + 1], rtl8380_sds4_fiber_6275b[2 * i]);
		i++;
	}

	i = 0;
	while (rtl8380_sds5_fiber_6275b[2 * i]) {
		sw_w32(rtl8380_sds5_fiber_6275b[2 * i + 1], rtl8380_sds5_fiber_6275b[2 * i]);
		i++;
	}

	i = 0;
	while (rtl8380_sds_reset[2 * i]) {
		sw_w32(rtl8380_sds_reset[2 * i + 1], rtl8380_sds_reset[2 * i]);
		i++;
	}

	i = 0;
	while (rtl8380_sds_release_reset[2 * i]) {
		sw_w32(rtl8380_sds_release_reset[2 * i + 1], rtl8380_sds_release_reset[2 * i]);
		i++;
	}

	pr_info("SDS power down value now: %x\n", sw_r32(RTL838X_SDS_CFG_REG));
	sw_w32(sds_conf_value, RTL838X_SDS_CFG_REG);

	pr_info("Configuration of SERDES done\n");

	return 0;
}

static int rtl8390_configure_serdes(struct phy_device *phydev)
{
	phydev_info(phydev, "Detected internal RTL8390 SERDES\n");

	/* In autoneg state, force link, set SR4_CFG_EN_LINK_FIB1G */
	sw_w32_mask(0, 1 << 18, RTL839X_SDS12_13_XSG0 + 0x0a);

	/* Disable EEE: Clear FRE16_EEE_RSG_FIB1G, FRE16_EEE_STD_FIB1G,
	 * FRE16_C1_PWRSAV_EN_FIB1G, FRE16_C2_PWRSAV_EN_FIB1G
	 * and FRE16_EEE_QUIET_FIB1G
	 */
	sw_w32_mask(0x1f << 10, 0, RTL839X_SDS12_13_XSG0 + 0xe0);

	return 0;
}

/* On the RTL838x SoCs, the internal SerDes is accessed through direct access to
 * standard PHY registers, where a 32 bit register holds a 16 bit word as found
 * in a standard page 0 of a PHY
 */
int rtl838x_read_sds_phy(int phy_addr, int phy_reg)
{
	int offset = 0;
	u32 val;

	if (phy_addr == 26)
		offset = 0x100;
	val = sw_r32(RTL838X_SDS4_FIB_REG0 + offset + (phy_reg << 2)) & 0xffff;

	return val;
}

/* On the RTL839x family of SoCs with inbuilt SerDes, these SerDes are accessed through
 * a 2048 bit register that holds the contents of the PHY being simulated by the SoC.
 */
int rtl839x_read_sds_phy(int phy_addr, int phy_reg)
{
	int offset = 0;
	int reg;
	u32 val;

	if (phy_addr == 49)
		offset = 0x100;

	/* For the RTL8393 internal SerDes, we simulate a PHY ID in registers 2/3
	 * which would otherwise read as 0.
	 */
	if (soc_info.id == 0x8393) {
		if (phy_reg == MII_PHYSID1)
			return 0x1c;
		if (phy_reg == MII_PHYSID2)
			return 0x8393;
	}

	/* Register RTL839X_SDS12_13_XSG0 is 2048 bit broad, the MSB (bit 15) of the
	 * 0th PHY register is bit 1023 (in byte 0x80). Because PHY-registers are 16
	 * bit broad, we offset by reg << 1. In the SoC 2 registers are stored in
	 * one 32 bit register.
	 */
	reg = (phy_reg << 1) & 0xfc;
	val = sw_r32(RTL839X_SDS12_13_XSG0 + offset + 0x80 + reg);

	if (phy_reg & 1)
		val = (val >> 16) & 0xffff;
	else
		val &= 0xffff;

	return val;
}


int rtl839x_write_sds_phy(int phy_addr, int phy_reg, u16 v)
{
	int offset = 0;
	int reg;
	u32 val;

	if (phy_addr == 49)
		offset = 0x100;

	reg = (phy_reg << 1) & 0xfc;
	val = v;
	if (phy_reg & 1) {
		val = val << 16;
		sw_w32_mask(0xffff0000, val,
			    RTL839X_SDS12_13_XSG0 + offset + 0x80 + reg);
	} else {
		sw_w32_mask(0xffff, val,
			    RTL839X_SDS12_13_XSG0 + offset + 0x80 + reg);
	}

	return 0;
}

/* Read the link and speed status of the internal SerDes of the RTL9300
 */
static int rtl9300_read_status(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int phy_addr = phydev->mdio.addr;
	struct device_node *dn;
	u32 sds_num = 0, status, latch_status, mode;

	if (dev->of_node) {
		dn = dev->of_node;

		if (of_property_read_u32(dn, "sds", &sds_num))
			sds_num = -1;
		pr_debug("%s: Port %d, SerDes is %d\n", __func__, phy_addr, sds_num);
	} else {
		dev_err(dev, "No DT node.\n");
		return -EINVAL;
	}

	if (sds_num < 0)
		return 0;

	mode = rtl930x_sds_mode_get(sds_num);
	pr_debug("%s got SDS mode %02x\n", __func__, mode);
	if (mode == RTL930X_SDS_OFF)
		mode = rtl930x_sds_field_r(sds_num, 0x1f, 9, 11, 7);
	if (mode == RTL930X_SDS_MODE_10GBASER) { /* 10GR mode */
		status = rtl930x_sds_field_r(sds_num, 0x5, 0, 12, 12);
		latch_status = rtl930x_sds_field_r(sds_num, 0x4, 1, 2, 2);
		status |= rtl930x_sds_field_r(sds_num, 0x5, 0, 12, 12);
		latch_status |= rtl930x_sds_field_r(sds_num, 0x4, 1, 2, 2);
	} else {
		status = rtl930x_sds_field_r(sds_num, 0x1, 29, 8, 0);
		latch_status = rtl930x_sds_field_r(sds_num, 0x1, 30, 8, 0);
		status |= rtl930x_sds_field_r(sds_num, 0x1, 29, 8, 0);
		latch_status |= rtl930x_sds_field_r(sds_num, 0x1, 30, 8, 0);
	}

	pr_debug("%s link status: status: %d, latch %d\n", __func__, status, latch_status);

	if (latch_status) {
		phydev->link = true;
		if (mode == RTL930X_SDS_MODE_10GBASER) {
			phydev->speed = SPEED_10000;
			phydev->interface = PHY_INTERFACE_MODE_10GBASER;
		} else {
			phydev->speed = SPEED_1000;
			phydev->interface = PHY_INTERFACE_MODE_1000BASEX;
		}

		phydev->duplex = DUPLEX_FULL;
	}

	return 0;
}

static int rtl931x_link_sts_get(u32 sds)
{
	u32 sts, sts1, latch_sts, latch_sts1;
	if (0){
		u32 xsg_sdsid_0, xsg_sdsid_1;

		xsg_sdsid_0 = sds < 2 ? sds : (sds - 1) * 2;
		xsg_sdsid_1 = xsg_sdsid_0 + 1;

		sts = rtl931x_sds_field_r(xsg_sdsid_0, 0x1, 29, 8, 0);
		sts1 = rtl931x_sds_field_r(xsg_sdsid_1, 0x1, 29, 8, 0);
		latch_sts = rtl931x_sds_field_r(xsg_sdsid_0, 0x1, 30, 8, 0);
		latch_sts1 = rtl931x_sds_field_r(xsg_sdsid_1, 0x1, 30, 8, 0);
	} else {
		u32  asds, dsds;

		asds = rtl931x_get_analog_sds(sds);
		sts = rtl931x_sds_field_r(asds, 0x5, 0, 12, 12);
		latch_sts = rtl931x_sds_field_r(asds, 0x4, 1, 2, 2);

		dsds = sds < 2 ? sds : (sds - 1) * 2;
		latch_sts1 = rtl931x_sds_field_r(dsds, 0x2, 1, 2, 2);
		sts1 = rtl931x_sds_field_r(dsds, 0x2, 1, 2, 2);
	}

	pr_debug("%s: serdes %d sts %d, sts1 %d, latch_sts %d, latch_sts1 %d\n", __func__,
		sds, sts, sts1, latch_sts, latch_sts1);

	return sts1;
}

static int rtl9310_read_status(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int phy_addr = phydev->mdio.addr;
	struct device_node *dn;
	u32 sds_num = 0, latch_status;

	if (dev->of_node) {
		dn = dev->of_node;

		if (of_property_read_u32(dn, "sds", &sds_num))
			sds_num = -1;
		pr_debug("%s: Port %d, SerDes is %d\n", __func__, phy_addr, sds_num);
	} else {
		dev_err(dev, "No DT node.\n");
		return -EINVAL;
	}

	if (sds_num < 0)
		return 0;

	latch_status = rtl931x_link_sts_get(sds_num);


	if (latch_status) {
		phydev->link = true;
		phydev->speed = SPEED_10000;
		phydev->interface = PHY_INTERFACE_MODE_10GBASER;
		phydev->duplex = DUPLEX_FULL;
	}

	return 0;
}

static int rtl93xx_read_status(struct phy_device *phydev){
	if (soc_info.family == RTL9300_FAMILY_ID)
		return rtl9300_read_status(phydev);
	else
		return rtl9310_read_status(phydev);
}

static int rtl8214fc_sfp_insert(void *upstream, const struct sfp_eeprom_id *id)
{
	__ETHTOOL_DECLARE_LINK_MODE_MASK(support) = { 0, };
	DECLARE_PHY_INTERFACE_MASK(interfaces);
	struct phy_device *phydev = upstream;
	phy_interface_t iface;

	sfp_parse_support(phydev->sfp_bus, id, support, interfaces);
	iface = sfp_select_interface(phydev->sfp_bus, support);

	dev_info(&phydev->mdio.dev, "%s SFP module inserted\n", phy_modes(iface));

	rtl8214fc_media_set(phydev, true);

	return 0;
}

static void rtl8214fc_sfp_remove(void *upstream)
{
	struct phy_device *phydev = upstream;

	rtl8214fc_media_set(phydev, false);
}

static const struct sfp_upstream_ops rtl8214fc_sfp_ops = {
	.attach = phy_sfp_attach,
	.detach = phy_sfp_detach,
	.module_insert = rtl8214fc_sfp_insert,
	.module_remove = rtl8214fc_sfp_remove,
};

static int rtl8214fc_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int addr = phydev->mdio.addr;
	int ret = 0;

	/* All base addresses of the PHYs start at multiples of 8 */
	devm_phy_package_join(dev, phydev, addr & (~7),
				sizeof(struct rtl83xx_shared_private));

	if (!(addr % 8)) {
		struct rtl83xx_shared_private *shared = phydev->shared->priv;
		shared->name = "RTL8214FC";
		/* Configuration must be done while patching still possible */
		if (soc_info.family == RTL8380_FAMILY_ID)
			ret = rtl8380_configure_rtl8214fc(phydev);
		if (ret)
			return ret;
	}

	return phy_sfp_probe(phydev, &rtl8214fc_sfp_ops);
}

static int rtl8214c_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int addr = phydev->mdio.addr;

	/* All base addresses of the PHYs start at multiples of 8 */
	devm_phy_package_join(dev, phydev, addr & (~7),
				sizeof(struct rtl83xx_shared_private));

	if (!(addr % 8)) {
		struct rtl83xx_shared_private *shared = phydev->shared->priv;
		shared->name = "RTL8214C";
		/* Configuration must be done whil patching still possible */
		return rtl8380_configure_rtl8214c(phydev);
	}

	return 0;
}

static int rtl8218b_ext_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int addr = phydev->mdio.addr;

	/* All base addresses of the PHYs start at multiples of 8 */
	devm_phy_package_join(dev, phydev, addr & (~7),
				sizeof(struct rtl83xx_shared_private));

	if (!(addr % 8)) {
		struct rtl83xx_shared_private *shared = phydev->shared->priv;
		shared->name = "RTL8218B (external)";
		if (soc_info.family == RTL8380_FAMILY_ID) {
			/* Configuration must be done while patching still possible */
			return rtl8380_configure_ext_rtl8218b(phydev);
		}
	}

	return 0;
}

static int rtl8218b_int_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int addr = phydev->mdio.addr;

	if (soc_info.family != RTL8380_FAMILY_ID)
		return -ENODEV;
	if (addr >= 24)
		return -ENODEV;

	pr_debug("%s: id: %d\n", __func__, addr);
	/* All base addresses of the PHYs start at multiples of 8 */
	devm_phy_package_join(dev, phydev, addr & (~7),
			      sizeof(struct rtl83xx_shared_private));

	if (!(addr % 8)) {
		struct rtl83xx_shared_private *shared = phydev->shared->priv;
		shared->name = "RTL8218B (internal)";
		/* Configuration must be done while patching still possible */
		return rtl8380_configure_int_rtl8218b(phydev);
	}

	return 0;
}

static int rtl8218d_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int addr = phydev->mdio.addr;

	pr_debug("%s: id: %d\n", __func__, addr);
	/* All base addresses of the PHYs start at multiples of 8 */
	devm_phy_package_join(dev, phydev, addr & (~7),
			      sizeof(struct rtl83xx_shared_private));

	/* All base addresses of the PHYs start at multiples of 8 */
	if (!(addr % 8)) {
		struct rtl83xx_shared_private *shared = phydev->shared->priv;
		shared->name = "RTL8218D";
		/* Configuration must be done while patching still possible */
/* TODO:		return configure_rtl8218d(phydev); */
	}

	return 0;
}

static int rtl8218e_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int addr = phydev->mdio.addr;

	pr_debug("%s: id: %d\n", __func__, addr);
	/* All base addresses of the PHYs start at multiples of 8 */
	devm_phy_package_join(dev, phydev, addr & (~7),
			      sizeof(struct rtl83xx_shared_private));

	/* All base addresses of the PHYs start at multiples of 8 */
	if (!(addr % 8)) {
		struct rtl83xx_shared_private *shared = phydev->shared->priv;
		shared->name = "RTL8218E";
		/* Configuration must be done while patching still possible */
/* TODO:		return configure_rtl8218d(phydev); */
	}

	return 0;
}

static int rtl821x_config_init(struct phy_device *phydev)
{
	/* Disable PHY-mode EEE so LPI is passed to the MAC */
	phy_modify_paged(phydev, RTL821X_PAGE_MAC, RTL821X_PHYCR2,
			 RTL821X_PHYCR2_PHY_EEE_ENABLE, 0);

	return 0;
}

static int rtl838x_serdes_probe(struct phy_device *phydev)
{
	int addr = phydev->mdio.addr;

	if (soc_info.family != RTL8380_FAMILY_ID)
		return -ENODEV;
	if (addr < 24)
		return -ENODEV;

	/* On the RTL8380M, PHYs 24-27 connect to the internal SerDes */
	if (soc_info.id == 0x8380) {
		if (addr == 24)
			return rtl8380_configure_serdes(phydev);
		return 0;
	}

	return -ENODEV;
}

static int rtl8393_serdes_probe(struct phy_device *phydev)
{
	int addr = phydev->mdio.addr;

	pr_info("%s: id: %d\n", __func__, addr);
	if (soc_info.family != RTL8390_FAMILY_ID)
		return -ENODEV;

	if (addr < 24)
		return -ENODEV;

	return rtl8390_configure_serdes(phydev);
}

static int rtl8390_serdes_probe(struct phy_device *phydev)
{
	int addr = phydev->mdio.addr;

	if (soc_info.family != RTL8390_FAMILY_ID)
		return -ENODEV;

	if (addr < 24)
		return -ENODEV;

	return rtl8390_configure_generic(phydev);
}

static int rtl9300_serdes_probe(struct phy_device *phydev)
{
	if (soc_info.family != RTL9300_FAMILY_ID && soc_info.family != RTL9310_FAMILY_ID)
		return -ENODEV;

	phydev_info(phydev, "Detected internal RTL9300 Serdes\n");

	return 0;
}

static struct phy_driver rtl83xx_phy_driver[] = {
	{
		PHY_ID_MATCH_EXACT(PHY_ID_RTL8214C),
		.name		= "Realtek RTL8214C",
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8214c_phy_probe,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	},
	{
		.match_phy_device = rtl8214fc_match_phy_device,
		.name		= "Realtek RTL8214FC",
		.config_aneg	= rtl8214fc_config_aneg,
		.config_init	= rtl821x_config_init,
		.get_features	= rtl8214fc_get_features,
		.get_tunable    = rtl8214fc_get_tunable,
		.probe		= rtl8214fc_phy_probe,
		.read_mmd	= rtl8214fc_read_mmd,
		.read_page	= rtl821x_read_page,
		.read_status    = rtl8214fc_read_status,
		.resume		= rtl8214fc_resume,
		.set_tunable	= rtl8214fc_set_tunable,
		.suspend	= rtl8214fc_suspend,
		.write_mmd	= rtl8214fc_write_mmd,
		.write_page	= rtl821x_write_page,
		.get_eee	= rtl8214fc_get_eee,
		.set_eee	= rtl8214fc_set_eee,
	},
	{
		.match_phy_device = rtl8218b_ext_match_phy_device,
		.name		= "Realtek RTL8218B (external)",
		.config_init	= rtl821x_config_init,
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8218b_ext_phy_probe,
		.read_mmd	= rtl821x_read_mmd,
		.read_page	= rtl821x_read_page,
		.resume		= genphy_resume,
		.suspend	= genphy_suspend,
		.write_mmd	= rtl821x_write_mmd,
		.write_page	= rtl821x_write_page,
		.set_eee	= rtl8218b_set_eee,
		.get_eee	= rtl8218b_get_eee,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8218B_I),
		.name		= "Realtek RTL8218B (internal)",
		.config_init	= rtl821x_config_init,
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8218b_int_phy_probe,
		.read_mmd	= rtl821x_read_mmd,
		.read_page	= rtl821x_read_page,
		.resume		= genphy_resume,
		.suspend	= genphy_suspend,
		.write_mmd	= rtl821x_write_mmd,
		.write_page	= rtl821x_write_page,
	},
	{
		PHY_ID_MATCH_EXACT(PHY_ID_RTL8218D),
		.name		= "REALTEK RTL8218D",
		.config_init	= rtl821x_config_init,
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8218d_phy_probe,
		.read_mmd	= rtl821x_read_mmd,
		.read_page	= rtl821x_read_page,
		.resume		= genphy_resume,
		.suspend	= genphy_suspend,
		.write_mmd	= rtl821x_write_mmd,
		.write_page	= rtl821x_write_page,
		.set_eee	= rtl8218d_set_eee,
		.get_eee	= rtl8218d_get_eee,
	},
	{
		PHY_ID_MATCH_EXACT(PHY_ID_RTL8218E),
		.name		= "REALTEK RTL8218E",
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8218e_phy_probe,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.write_mmd	= rtl821x_write_mmd,
		.write_page	= rtl821x_write_page,
		.set_eee	= rtl8218d_set_eee,
		.get_eee	= rtl8218d_get_eee,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8221B),
		.name           = "REALTEK RTL8221B",
		.features       = PHY_GBIT_FEATURES,
		.suspend        = genphy_suspend,
		.resume         = genphy_resume,
		.read_page      = rtl821x_read_page,
		.write_page     = rtl821x_write_page,
		.read_status    = rtl8226_read_status,
		.config_aneg    = rtl8226_config_aneg,
		.set_eee        = rtl8226_set_eee,
		.get_eee        = rtl8226_get_eee,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8226),
		.name		= "REALTEK RTL8226",
		.features	= PHY_GBIT_FEATURES,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.read_status	= rtl8226_read_status,
		.config_aneg	= rtl8226_config_aneg,
		.set_eee	= rtl8226_set_eee,
		.get_eee	= rtl8226_get_eee,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8218B_I),
		.name		= "Realtek RTL8380 SERDES",
		.features	= PHY_GBIT_FIBRE_FEATURES,
		.probe		= rtl838x_serdes_probe,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_status	= rtl8380_read_status,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8393_I),
		.name		= "Realtek RTL8393 SERDES",
		.features	= PHY_GBIT_FIBRE_FEATURES,
		.probe		= rtl8393_serdes_probe,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_status	= rtl8393_read_status,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8390_GENERIC),
		.name		= "Realtek RTL8390 Generic",
		.features	= PHY_GBIT_FIBRE_FEATURES,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.probe		= rtl8390_serdes_probe,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL9300_I),
		.name		= "REALTEK RTL9300 SERDES",
		.features	= PHY_GBIT_FIBRE_FEATURES,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.probe		= rtl9300_serdes_probe,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_status	= rtl93xx_read_status,
#ifdef CONFIG_RTL931X		
//		.config_aneg	= rtl9300_config_aneg,
#endif
	},
};

module_phy_driver(rtl83xx_phy_driver);

static struct mdio_device_id __maybe_unused rtl83xx_tbl[] = {
	{ PHY_ID_MATCH_MODEL(PHY_ID_RTL8214_OR_8218) },
	{ }
};

MODULE_DEVICE_TABLE(mdio, rtl83xx_tbl);

MODULE_AUTHOR("B. Koblitz");
MODULE_DESCRIPTION("RTL83xx PHY driver");
MODULE_LICENSE("GPL");
