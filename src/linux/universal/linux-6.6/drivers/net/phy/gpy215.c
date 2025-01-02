#include <linux/phy.h>
#include <linux/delay.h>
#include <linux/reboot.h>

MODULE_DESCRIPTION("GPY 215 driver");
MODULE_AUTHOR("Hancheng Yang <hancheng.yang@acksys.fr>");
MODULE_AUTHOR("Sebastian Gottschall <s.gottschall@dd-wrt.com>");
MODULE_LICENSE("GPL");

#define DEBUG 0

#if DEBUG != 0
#define debug(fmt, ...) printk(KERN_INFO "%s: " fmt, __func__, ##__VA_ARGS__)
#else
#define debug(fmt, ...) \
	do {            \
	} while (0)
#endif

/* mdio */
#define MII_DEVADDR_C45_SHIFT 16

#define MDIO_CTRL1_SPEED2_5G (MDIO_CTRL1_SPEEDSELEXT | 0x18)
#define MDIO_CTRL1_SPEED5G (MDIO_CTRL1_SPEEDSELEXT | 0x1c)

#define MDIO_PMA_CTRL2_2_5GBT 0x0030 /* 2.5GBaseT type */
#define MDIO_PMA_CTRL2_5GBT 0x0031 /* 5GBaseT type */

#define MDIO_AN_10GBT_STAT 33 /* 10GBASE-T auto-negotiation status */

/* AN 10GBASE-T control register. */
#define MDIO_AN_10GBT_CTRL_ADV2_5G 0x0080 /* Advertise 2.5GBASE-T */
#define MDIO_AN_10GBT_CTRL_ADV5G 0x0100 /* Advertise 5GBASE-T */
#define MDIO_AN_10GBT_CTRL_ADV10G 0x1000 /* Advertise 10GBASE-T */

#define PHY_CTL1 0x13
#define PHY_CTL1_MDICD BIT(3)
#define PHY_CTL1_MDIAB BIT(2)
#define PHY_CTL1_AMDIX BIT(0)

#define PHY_PMA_MGBT_POLARITY 0x82
#define PHY_MDI_MDI_X_MASK GENMASK(1, 0)
#define PHY_MDI_MDI_X_NORMAL 0x3
#define PHY_MDI_MDI_X_AB 0x2
#define PHY_MDI_MDI_X_CD 0x1
#define PHY_MDI_MDI_X_CROSS 0x0

static int gpy_read_mmd(struct phy_device *phydev, int devad, u32 regnum)
{
	int ret;

	ret = phy_read_mmd(phydev, devad, regnum);

	return ret;
}

static int gpy_write_mmd(struct phy_device *phydev, int devad, u32 regnum,
			 u16 val)
{
	int ret;

	ret = phy_write_mmd(phydev, devad, regnum, val);

	return ret;
}

static int gpy_phy_modify_mmd_changed(struct phy_device *phydev, int devad,
				      u32 regnum, u16 mask, u16 set)
{
	int new, ret;

	ret = gpy_read_mmd(phydev, devad, regnum);
	if (ret < 0)
		return ret;

	new = (ret & ~mask) | set;
	debug("%d.%d current value = 0x%04x, change to 0x%04x\n", devad, regnum,
	      ret, new);
	if (new == ret)
		return 0;

	ret = gpy_write_mmd(phydev, devad, regnum, new);

	return ret < 0 ? ret : 1;
}

static int gpy_phy_modify_mmd(struct phy_device *phydev, int devad, u32 regnum,
			      u16 mask, u16 set)
{
	int ret;
	ret = gpy_phy_modify_mmd_changed(phydev, devad, regnum, mask, set);
	return ret < 0 ? ret : 0;
}

/**
 * genphy_config_eee_advert - disable unwanted eee mode advertisement
 * @phydev: target phy_device struct
 *
 * Description: Writes MDIO_AN_EEE_ADV after disabling unsupported energy
 *   efficent ethernet modes. Returns 0 if the PHY's advertisement hasn't
 *   changed, and 1 if it has changed.
 */
int gpy_genphy_config_eee_advert(struct phy_device *phydev)
{
	int err;

	/* Nothing to disable */
	if (!phydev->eee_broken_modes)
		return 0;

	err = gpy_phy_modify_mmd_changed(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV,
					 phydev->eee_broken_modes, 0);
	/* If the call failed, we assume that EEE is not supported */
	return err < 0 ? 0 : err;
}

static int gpy_genphy_c45_an_config_aneg(struct phy_device *phydev)
{
	int changed = 0, ret;
	u32 adv;

	linkmode_and(phydev->advertising, phydev->advertising,
		     phydev->supported);

	changed = gpy_genphy_config_eee_advert(phydev);

	adv = linkmode_adv_to_mii_adv_t(phydev->advertising);

	ret = gpy_phy_modify_mmd_changed(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE,
					 ADVERTISE_ALL | ADVERTISE_100BASE4 |
						 ADVERTISE_PAUSE_CAP |
						 ADVERTISE_PAUSE_ASYM,
					 adv);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = 1;

	//	adv = linkmode_adv_to_mii_10gbt_adv_t(phydev->advertising);
	adv = MDIO_AN_10GBT_CTRL_ADV2_5G; // We know that gpy215 only supports 2G5

	ret = gpy_phy_modify_mmd_changed(
		phydev, MDIO_MMD_AN, MDIO_AN_10GBT_CTRL,
		MDIO_AN_10GBT_CTRL_ADV10G | MDIO_AN_10GBT_CTRL_ADV5G |
			MDIO_AN_10GBT_CTRL_ADV2_5G,
		adv);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = 1;

	return changed;
}

/* driver utilities */

#define PHY_ID_GPY215 0x67c9dc00
#define PHY_MASK_GPY215 0xfffffc00

#define GPY_FW 0x1E /* Firmware version */
#define GPY_IMASK 0x19 /* interrupt mask */
#define GPY_ISTAT 0x1A /* interrupt status */
#define GPY_LED 0x1B /* LED Control */
#define GPY_INTR_WOL BIT(15) /* Wake-on-LAN */
#define GPY_INTR_ANC BIT(10) /* Auto-Neg complete */
#define GPY_INTR_DXMC BIT(2) /* Duplex mode change */
#define GPY_INTR_LSPC BIT(1) /* Link speed change */
#define GPY_INTR_LSTC BIT(0) /* Link state change */
#define GPY_INTR_MASK \
	(GPY_INTR_LSTC | GPY_INTR_LSPC | GPY_INTR_DXMC | GPY_INTR_ANC)

/* GPY VENDOR SPECIFIC 1 */
#define GPY_VSPEC1_LED0 0x1
#define GPY_VSPEC1_LED1 0x2
#define GPY_VSPEC1_SGMII_CTRL 0x8
#define GPY_VSPEC1_SGMII_STS 0x9
#define GPY_SGMII_ANEN BIT(12) /* Aneg enable */
#define GPY_SGMII_SGMII_FIXED2G5 BIT(5) /* Force SGMII to 2.5G */
#define GPY_SGMII_ANRS BIT(9) /* Restart Aneg */
#define GPY_SGMII_ANOK BIT(5) /* Aneg complete */
#define GPY_SGMII_LS BIT(2) /* Link status */
#define GPY_SGMII_DR_MASK GENMASK(1, 0) /* Data rate */
#define GPY_SGMII_DR_2500 0x3

/* GPY VENDOR SPECIFIC 2 */
#define GPY_VSPEC2_WOL_CTL 0xe06
#define GPY_WOL_EN BIT(0)

#define GPY_VSPEV2_WOL_AD01 0xe08 /* WOL addr Byte5:Byte6 */
#define GPY_VSPEV2_WOL_AD23 0xe09 /* WOL addr Byte3:Byte4 */
#define GPY_VSPEV2_WOL_AD45 0xe0a /* WOL addr Byte1:Byte2 */

static int phy_poll_reset(struct phy_device *phydev)
{
	/* Poll until the reset bit clears (50ms per retry == 0.6 sec) */
	unsigned int retries = 12;
	int ret;

	do {
		msleep(50);
		ret = phy_read(phydev, MII_BMCR);
		if (ret < 0)
			return ret;
	} while (ret & BMCR_RESET && --retries);
	if (ret & BMCR_RESET)
		return -ETIMEDOUT;

	/* Some chips (smsc911x) may still need up to another 1ms after the
	 * BMCR_RESET bit is cleared before they are usable.
	 */
	msleep(1);
	return 0;
}

/* driver */

static int gpy215_soft_reset(struct phy_device *phydev)
{
	int ret;

	debug("====Called====\n");

	ret = phy_write(phydev, MII_BMCR, BMCR_RESET);
	if (ret < 0)
		return ret;

	// add delay before polling reset, 0.5 sec
	msleep(500);

	return phy_poll_reset(phydev);
}

int gpy_genphy_c45_restart_aneg(struct phy_device *phydev)
{
	u16 reg = MDIO_CTRL1;

	return phy_set_bits_mmd(phydev, MDIO_MMD_AN, reg,
				MDIO_AN_CTRL1_ENABLE | MDIO_AN_CTRL1_RESTART);
}

static int gpy215_config_init(struct phy_device *phydev)
{
	int ret, fw_ver = 0;

	debug("====Called====\n");

	fw_ver = phy_read(phydev, GPY_FW);
	debug("Firmware Version: 0x%04X (%s)\n", fw_ver,
	      (fw_ver & 8000) ? "release" : "test");

	/* In GPY PHY FW, by default EEE mode is enabled. So, disable EEE mode
	 * during power up. Ethtool must be used to enable or disable it.
	 */
	debug("Configuring EEE\n");
	ret = gpy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV);
	if (ret < 0) {
		debug("Read MDIO_MMD_AN/MDIO_AN_EEE_ADV error!\n");
		return ret;
	}

	ret = gpy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV, 0);
	if (ret < 0) {
		debug("Write MDIO_MMD_AN/MDIO_AN_EEE_ADV error!\n");
		return ret;
	}

	debug("Configuring LED\n");
	/*******
	LED0 => yellow
	LED1 => green
	*******/
	/* Enable LED0 LED1 */
	ret = phy_write(phydev, GPY_LED, BIT(8) | BIT(9));
	if (ret < 0) {
		debug("Write GPY_LED error!\n");
		return ret;
	}

	/* LED0 configuration */
	ret = gpy_write_mmd(phydev, MDIO_MMD_VEND1, GPY_VSPEC1_LED0,
			    BIT(7) | // constant on when link is up at 2500
				    BIT(8) | BIT(9)); // pulse when tx/rx
	if (ret < 0) {
		debug("Write MDIO_MMD_VEND1/GPY_VSPEC1_LED0 error!\n");
		return ret;
	}

	/* LED1 configuration */
	ret = gpy_write_mmd(
		phydev, MDIO_MMD_VEND1, GPY_VSPEC1_LED1,
		BIT(4) | BIT(5) |
			BIT(6) | // constant on when link is up at 10/100/1000
			BIT(8) | BIT(9)); // pulse when tx/rx
	if (ret < 0) {
		debug("Write MDIO_MMD_VEND1/GPY_VSPEC1_LED1 error!\n");
		return ret;
	}

	return gpy_genphy_c45_restart_aneg(phydev);
}

static int gpy215_probe(struct phy_device *phydev)
{
	debug("====Called====\n");
	//TODO: Should do it properly
	phydev->is_c45 = true;
	return 0;
}

static int gpy215_suspend(struct phy_device *phydev)
{
	debug("====Called====\n");
	return genphy_suspend(phydev);
}

static int gpy215_resume(struct phy_device *phydev)
{
	debug("====Called====\n");
	return genphy_resume(phydev);
}

int gpy_genphy_c45_check_and_restart_aneg(struct phy_device *phydev,
					  bool restart)
{
	u16 reg = MDIO_CTRL1;
	int ret;

	if (!restart) {
		/* Configure and restart aneg if it wasn't set before */
		ret = phy_read_mmd(phydev, MDIO_MMD_AN, reg);
		if (ret < 0)
			return ret;

		if (!(ret & MDIO_AN_CTRL1_ENABLE))
			restart = true;
	}

	if (restart)
		return gpy_genphy_c45_restart_aneg(phydev);

	return 0;
}

static int gpy_config_mdix(struct phy_device *phydev, u8 ctrl)
{
	int ret;
	u16 val;

	switch (ctrl) {
	case ETH_TP_MDI_AUTO:
		val = PHY_CTL1_AMDIX;
		break;
	case ETH_TP_MDI_X:
		val = (PHY_CTL1_MDIAB | PHY_CTL1_MDICD);
		break;
	case ETH_TP_MDI:
		val = 0;
		break;
	default:
		return 0;
	}

	ret = phy_modify(phydev, PHY_CTL1,
			 PHY_CTL1_AMDIX | PHY_CTL1_MDIAB | PHY_CTL1_MDICD, val);
	if (ret < 0)
		return ret;

	return genphy_c45_restart_aneg(phydev);
}

static int gpy215_config_aneg(struct phy_device *phydev)
{
	bool changed = false;
	u32 adv;
	int ret;

	debug("====Called====\n");

	if (phydev->autoneg == AUTONEG_DISABLE)
		return genphy_c45_pma_setup_forced(phydev);

	ret = gpy_config_mdix(phydev, phydev->mdix_ctrl);
	if (ret < 0)
		return ret;

	ret = gpy_genphy_c45_an_config_aneg(phydev);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	adv = linkmode_adv_to_mii_ctrl1000_t(phydev->advertising);

	ret = phy_modify_changed(phydev, MII_CTRL1000,
				 ADVERTISE_1000FULL | ADVERTISE_1000HALF, adv);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	/* Cisco SGMII specification 1.8 specify that SGMI auto negotiation
	 * supports speed of 10/100/1000Mbps. So, for 2.5Gbps, SGMI auto
	 * negotiation should be disabled.
	 */
	ret = gpy_phy_modify_mmd_changed(phydev, MDIO_MMD_VEND1,
					 GPY_VSPEC1_SGMII_CTRL, GPY_SGMII_ANEN,
					 0);
	if (ret < 0)
		return ret;

	ret = gpy_genphy_c45_check_and_restart_aneg(phydev, changed);
	if (ret < 0)
		return ret;

	/* There is a design constraint in GPY2xx device where SGMII AN is
	 * only triggered when there is change of speed. If, PHY link
	 * partner`s speed is still same even after PHY TPI is down and up
	 * again, SGMII AN is not triggered and hence no new in-band message
	 * from GPY to MAC side SGMII.
	 * This could cause an issue during power up, when PHY is up prior to
	 * MAC. At this condition, once MAC side SGMII is up, MAC side SGMII
	 * wouldn`t receive new in-band message from GPY with correct link
	 * status, speed and duplex info.
	 *
	 * 1) If PHY is already up and TPI link status is still down (such as
	 *    hard reboot), TPI link status is polled for 4 seconds before
	 *    retriggerring SGMII AN.
	 * 2) If PHY is already up and TPI link status is also up (such as soft
	 *    reboot), polling of TPI link status is not needed and SGMII AN is
	 *    immediately retriggered.
	 * 3) Other conditions such as PHY is down, speed change etc, skip
	 *    retriggering SGMII AN. Note: in case of speed change, GPY FW will
	 *    initiate SGMII AN.
	 */

	// Acksys: trigger SGMII AN everytime.

	/*
	debug("phydev->state=%d\n", phydev->state);
	if (phydev->state != PHY_UP)
		return 0;

	debug("Check SGMII re-neg condition\n");
	ret = phy_read_poll_timeout(phydev, MII_BMSR, ret, ret & BMSR_LSTATUS,
				    20000, 4000000, false);
	if (ret == -ETIMEDOUT) {
		debug("Poll MII_BMSR.BMSR_LSTATUS timeout!\n");
		return 0;
	} else if (ret < 0) {
		debug("Poll MII_BMSR.BMSR_LSTATUS error(%d)!\n", ret);
		return ret;
	}*/

	debug("Trigger SGMII AN\n");

	/* Trigger SGMII AN. */
	return gpy_phy_modify_mmd(phydev, MDIO_MMD_VEND1, GPY_VSPEC1_SGMII_CTRL,
				  GPY_SGMII_ANRS, GPY_SGMII_ANRS);
}

static void gpy215_link_change_notify(struct phy_device *phydev)
{
	if (genphy_c45_aneg_done(phydev)) {
		debug("Auto negotiation ok\n");
		phy_modify_mmd_changed(phydev, MDIO_MMD_VEND1,
				       GPY_VSPEC1_SGMII_CTRL,
				       GPY_SGMII_SGMII_FIXED2G5,
				       GPY_SGMII_SGMII_FIXED2G5);
	} else {
		debug("Auto negotiation failed\n");
	}
}

static int gpy_update_mdix(struct phy_device *phydev)
{
	int ret;

	ret = phy_read(phydev, PHY_CTL1);
	if (ret < 0)
		return ret;

	if (ret & PHY_CTL1_AMDIX)
		phydev->mdix_ctrl = ETH_TP_MDI_AUTO;
	else if (ret & PHY_CTL1_MDICD || ret & PHY_CTL1_MDIAB)
		phydev->mdix_ctrl = ETH_TP_MDI_X;
	else
		phydev->mdix_ctrl = ETH_TP_MDI;

	ret = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, PHY_PMA_MGBT_POLARITY);
	if (ret < 0)
		return ret;

	if ((ret & PHY_MDI_MDI_X_MASK) < PHY_MDI_MDI_X_NORMAL)
		phydev->mdix = ETH_TP_MDI_X;
	else
		phydev->mdix = ETH_TP_MDI;

	return 0;
}

static int gpy215_read_status(struct phy_device *phydev)
{
	int ret, status;
	int adv;
	int lpa;
	int lpagb = 0;
	int common_adv;
	int common_adv_gb = 0;
	int aneg_mgbt = 0;
	uint64_t *pcsx_int_reg;
	static int pcsx_int_reg_err_count = 0;

#define CVMX_IO_SEG 2LL
#define CVMX_ADD_SEG(segment, add) ((((uint64_t)segment) << 62) | (add))
#define CVMX_ADD_IO_SEG(add) CVMX_ADD_SEG(CVMX_IO_SEG, (add))
#define CVMX_PCSX_INTX_REG(offset, block_id)     \
	CVMX_ADD_IO_SEG(0x00011800B0001080ull) + \
		(((offset) & 3) + ((block_id) & 1) * 0x20000ull) * 1024

	pcsx_int_reg = (uint64_t *)(CVMX_PCSX_INTX_REG(
		0, phydev->mdio.addr == 0x01 ? 0 : 1));
	//debug("PCS0_INT%d_REG=0x%04x\n", phydev->mdio.addr == 0x01 ? 0 : 1, (uint16_t)(*reg & 0xFFFF));
	ret = genphy_update_link(phydev);
	if (ret)
		return ret;
	phydev->speed = SPEED_UNKNOWN;
	phydev->duplex = DUPLEX_UNKNOWN;
	phydev->pause = 0;
	phydev->asym_pause = 0;

	//	phydev->lp_advertising = 0;
	/* Read link and autonegotiation status */
	status = phy_read(phydev, MII_BMSR);
	if (status < 0) {
		return status;
	}
	//	debug("autoneg %d autoneg complete %d %d\n", phydev->autoneg, status, status & BMSR_ANEGCOMPLETE);

	//	phydev->lp_advertising = 0;

	if (phydev->autoneg == AUTONEG_ENABLE && status & BMSR_ANEGCOMPLETE) {
		/* Read the link partner's 1G advertisement */
		lpagb = phy_read(phydev, MII_STAT1000);
		if (lpagb < 0)
			return lpagb;

		adv = phy_read(phydev, MII_CTRL1000);
		if (adv < 0)
			return adv;

		//		phydev->lp_advertising =
		//			mii_stat1000_to_ethtool_lpa_t(lpagb);
		mii_stat1000_mod_linkmode_lpa_t(phydev->lp_advertising, lpagb);

		common_adv_gb = lpagb & adv << 2;

		/* Read the link partner's base page advertisement */
		lpa = phy_read(phydev, MII_LPA);
		if (lpa < 0)
			return lpa;
		mii_lpa_mod_linkmode_lpa_t(phydev->lp_advertising, lpa);
		//		phydev->lp_advertising |= mii_lpa_to_ethtool_lpa_t(lpa);

		/* Read the link partner's 10G advertisement */
		aneg_mgbt =
			gpy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_10GBT_STAT);
		if (aneg_mgbt < 0)
			return aneg_mgbt;
		/* Cannot set flag in lp_advertising because kernel is too old.
		   lp_advertising is 32 bit in kernel 4.9.111 */
		mii_10gbt_stat_mod_linkmode_lpa_t(phydev->lp_advertising, lpa);
		adv = phy_read(phydev, MII_ADVERTISE);
		if (adv < 0)
			return adv;

		common_adv = lpa & adv;

		phydev->speed = SPEED_10;
		phydev->duplex = DUPLEX_HALF;
		phydev->pause = 0;
		phydev->asym_pause = 0;

		if (aneg_mgbt) {
			phydev->speed = SPEED_2500;
			phydev->duplex = DUPLEX_FULL;
		} else if (common_adv_gb & (LPA_1000FULL | LPA_1000HALF)) {
			phydev->speed = SPEED_1000;

			if (common_adv_gb & LPA_1000FULL)
				phydev->duplex = DUPLEX_FULL;
		} else if (common_adv & (LPA_100FULL | LPA_100HALF)) {
			phydev->speed = SPEED_100;

			if (common_adv & LPA_100FULL)
				phydev->duplex = DUPLEX_FULL;
		} else if (common_adv & LPA_10FULL)
			phydev->duplex = DUPLEX_FULL;

		if (phydev->duplex == DUPLEX_FULL) {
			phydev->pause = lpa & LPA_PAUSE_CAP ? 1 : 0;
			phydev->asym_pause = lpa & LPA_PAUSE_ASYM ? 1 : 0;
		}

		if (phydev->drv->aneg_done) {
			if (*pcsx_int_reg != 0x0)
				pcsx_int_reg_err_count++;
			else
				pcsx_int_reg_err_count = 0;
		}

		if (pcsx_int_reg_err_count > 10) {
			printk(KERN_EMERG
			       "SGMII SYNC UNRECOVERABLE ERROR, EMERGENCY RESTART\n");
			//			emergency_restart();
		}

		return 0;
	} else if (phydev->autoneg == AUTONEG_DISABLE) {
		ret = genphy_c45_read_pma(phydev);
	}

	gpy_update_mdix(phydev);

	return ret;
}

static irqreturn_t gpy215_handle_interrupt(struct phy_device *phydev)
{
	int reg;
	debug("====Called====\n");

	reg = phy_read(phydev, GPY_ISTAT);
	if (reg < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	if (!(reg & GPY_INTR_MASK))
		return IRQ_NONE;

	phy_trigger_machine(phydev);

	return IRQ_HANDLED;
}

static int gpy215_ack_interrupt(struct phy_device *phydev)
{
	int reg;
	debug("====Called====\n");

	reg = phy_read(phydev, GPY_ISTAT);
	return (reg < 0) ? reg : 0;
}

static int gpy215_config_intr(struct phy_device *phydev)
{
	int err;

	debug("====Called====\n");
	if (phydev->interrupts == PHY_INTERRUPT_ENABLED) {
		err = gpy215_ack_interrupt(phydev);
		if (err)
			return err;

		err = phy_write(phydev, GPY_IMASK, GPY_INTR_MASK);
	} else {
		err = phy_write(phydev, GPY_IMASK, 0);
		if (err)
			return err;
		err = gpy215_ack_interrupt(phydev);
	}

	return err;
}

static void gpy215_remove(struct phy_device *phydev)
{
	debug("====Called====\n");
	//TODO: Should do it properly
	phydev->is_c45 = false;
}

static int gpy215_get_features(struct phy_device *phydev)
{
	__ETHTOOL_DECLARE_LINK_MODE_MASK(supported) = {
		0,
	};

	linkmode_set_bit(ETHTOOL_LINK_MODE_Autoneg_BIT, supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_TP_BIT, supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_MII_BIT, phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_Pause_BIT, supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT, supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT, supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT, supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT, supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT, supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseT_Half_BIT, supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT, supported);

	linkmode_copy(phydev->supported, supported);

	return 0;
}

static struct phy_driver gpy215_driver[] = { {
	.phy_id = PHY_ID_GPY215,
	.name = "GPY 215",
	.phy_id_mask = PHY_MASK_GPY215,
	.get_features = gpy215_get_features,
	.soft_reset = gpy215_soft_reset,
	.config_init = gpy215_config_init,
	.probe = gpy215_probe,
	.suspend = gpy215_suspend,
	.resume = gpy215_resume,
	.link_change_notify = gpy215_link_change_notify,
	.config_aneg = gpy215_config_aneg,
	.read_status = gpy215_read_status,
	.config_intr = gpy215_config_intr,
	.handle_interrupt = gpy215_handle_interrupt,
	.remove = gpy215_remove,
} };

module_phy_driver(gpy215_driver);

static struct mdio_device_id __maybe_unused
	gpy215_tbl[] = { { PHY_ID_GPY215, PHY_MASK_GPY215 }, {} };

MODULE_DEVICE_TABLE(mdio, gpy215_tbl);
