/*
 * Broadcom BCM5325E/536x switch configuration module
 *
 * Copyright (C) 2005 Felix Fietkau <nbd@nbd.name>
 * Copyright (C) 2008 Michael Buesch <mb@bu3sch.de>
 * Copyright (C) 2013 Hauke Mehrtens <hauke@hauke-m.de>
 * Based on 'robocfg' by Oleg I. Vdovikin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>

#include "switch-core.h"
#include "etc53xx.h"

#ifdef CONFIG_BCM47XX
#include <bcmnvram.h>
#endif

#define DRIVER_NAME		"bcm53xx"
#define DRIVER_VERSION		"0.03"
#define PFX			"roboswitch: "

#define ROBO_PHY_ADDR		0x1E	/* robo switch phy address */
#define ROBO_PHY_ADDR_TG3	0x01	/* Tigon3 PHY address */
#define ROBO_PHY_ADDR_BCM63XX	0x00	/* BCM63XX PHY address */

/* MII registers */
#define REG_MII_PAGE	0x10	/* MII Page register */
#define REG_MII_ADDR	0x11	/* MII Address register */
#define REG_MII_DATA0	0x18	/* MII Data register 0 */

#define REG_MII_PAGE_ENABLE	1
#define REG_MII_ADDR_WRITE	1
#define REG_MII_ADDR_READ	2

/* Robo device ID register (in ROBO_MGMT_PAGE) */
#define ROBO_DEVICE_ID		0x30
#define  ROBO_DEVICE_ID_5325	0x25	/* Faked */
#define  ROBO_DEVICE_ID_5395	0x95
#define  ROBO_DEVICE_ID_5397	0x97
#define  ROBO_DEVICE_ID_5398	0x98
#define  ROBO_DEVICE_ID_53115	0x3115
#define  ROBO_DEVICE_ID_53125	0x3125

#define ROBO_DEVICE_ID_53010	0x53010	/* 53010 */
#define ROBO_DEVICE_ID_53011	0x53011	/* 53011 */
#define ROBO_DEVICE_ID_53012	/* 53012 */
#define ROBO_DEVICE_ID_53018	/* 53012 */
#define ROBO_DEVICE_ID_53019	/* 53019 */
#define ROBO_DEVICE_ID_53030	/* 53019 */

#define DEVID53010	0x53010	/* 53010 */
#define DEVID53011	0x53011	/* 53011 */
#define DEVID53012	0x53012	/* 53012 */
#define DEVID53018	0x53018	/* 53018 */
#define DEVID53019	0x53019	/* 53019 */
#define DEVID53030	0x53030	/* 53030 */
#define ROBO_IS_BCM5301X(id) ((id) == DEVID53010 || (id) == DEVID53011 || (id) == DEVID53012 || \
(id) == DEVID53018 || (id) == DEVID53019 || (id) == DEVID53030)

/* Private et.o ioctls */
#define SIOCGETCPHYRD           (SIOCDEVPRIVATE + 9)
#define SIOCSETCPHYWR           (SIOCDEVPRIVATE + 10)

#define ETCROBORD	14
#define ETCROBOWR	15
#define ETCROBORD4	16
#define ETCROBOWR4	17
#define ETCROBOWR1	18
#define SIOCGETCROBORD		(SIOCDEVPRIVATE + ETCROBORD)
#define SIOCSETCROBOWR		(SIOCDEVPRIVATE + ETCROBOWR)
#define SIOCGETCROBORD4		(SIOCDEVPRIVATE + ETCROBORD4)
#define SIOCSETCROBOWR4		(SIOCDEVPRIVATE + ETCROBOWR4)
#define SIOCSETCROBOWR1		(SIOCDEVPRIVATE + ETCROBOWR1)

#ifdef CONFIG_MACH_BRCM_NS
#define ROBO_SRAB
#endif

/* Data structure for a Roboswitch device. */
struct robo_switch {
	char *device;		/* The device name string (ethX) */
	u32 devid;		/* ROBO_DEVICE_ID_53xx */
	bool is_5365;
	bool gmii;		/* gigabit mii */
	u8 corerev;
	int gpio_robo_reset;
	int gpio_lanports_enable;
	struct ifreq ifr;
	struct net_device *dev;
	unsigned char port[9];
};

/* Currently we can only have one device in the system. */
static struct robo_switch robo;

static int do_ioctl(int cmd)
{
	mm_segment_t old_fs = get_fs();
	int ret;

	set_fs(KERNEL_DS);
	ret = robo.dev->netdev_ops->ndo_do_ioctl(robo.dev, &robo.ifr, cmd);
	set_fs(old_fs);

	return ret;
}

static u16 mdio_read(__u16 phy_id, __u8 reg)
{
#ifdef ROBO_SRAB
	__u32 vecarg[5];
	int err;
	robo.ifr.ifr_data = (caddr_t) vecarg;
	vecarg[0] = (phy_id + 0x10) << 16;;
	vecarg[0] |= reg & 0xffff;
	err = do_ioctl(SIOCGETCROBORD);
	if (err < 0) {
		printk(KERN_ERR PFX "failed to read page %i with reg %i (%d)\n", phy_id + 0x10, reg, err);
	}
	return vecarg[1] & 0xffff;
#else

	struct mii_ioctl_data *mii = if_mii(&robo.ifr);
	int err;

	mii->phy_id = phy_id;
	mii->reg_num = reg;

	err = do_ioctl(SIOCGMIIREG);
	if (err < 0) {
		printk(KERN_ERR PFX "failed to read mdio reg %i with err %i.\n", reg, err);

		return 0xffff;
	}

	return mii->val_out;
#endif
}

static void mdio_write(__u16 phy_id, __u8 reg, __u16 val)
{
#ifdef ROBO_SRAB
	__u32 vecarg[5];
	int err;
	robo.ifr.ifr_data = (caddr_t) vecarg;
	vecarg[0] = (phy_id + 0x10) << 16;;
	vecarg[0] |= reg & 0xffff;
	vecarg[1] = val;
	err = do_ioctl(SIOCSETCROBOWR);
	if (err < 0) {
		printk(KERN_ERR PFX "failed to write page %i with reg %i and val %i (%d)\n", phy_id + 0x10, reg, val, err);
	}
#else
	struct mii_ioctl_data *mii = if_mii(&robo.ifr);
	int err;

	mii->phy_id = phy_id;
	mii->reg_num = reg;
	mii->val_in = val;

	err = do_ioctl(SIOCSMIIREG);
	if (err < 0) {
		printk(KERN_ERR PFX "failed to write mdio reg: %i with err %i.\n", reg, err);
		return;
	}
#endif
}

#ifndef ROBO_SRAB
static int robo_reg(__u8 page, __u8 reg, __u8 op)
{
	int i = 3;

	/* set page number */
	mdio_write(ROBO_PHY_ADDR, REG_MII_PAGE, (page << 8) | REG_MII_PAGE_ENABLE);

	/* set register address */
	mdio_write(ROBO_PHY_ADDR, REG_MII_ADDR, (reg << 8) | op);

	/* check if operation completed */
	while (i--) {
		if ((mdio_read(ROBO_PHY_ADDR, REG_MII_ADDR) & 3) == 0)
			return 0;
	}

	printk(KERN_ERR PFX "timeout in robo_reg on page %i and reg %i with op %i.\n", page, reg, op);

	return 1;
}
#endif

/*
static void robo_read(__u8 page, __u8 reg, __u16 *val, int count)
{
	int i;

	robo_reg(page, reg, REG_MII_ADDR_READ);

	for (i = 0; i < count; i++)
		val[i] = mdio_read(ROBO_PHY_ADDR, REG_MII_DATA0 + i);
}
*/
static __u16 robo_read16(__u8 page, __u8 reg)
{
#ifdef ROBO_SRAB
	__u32 vecarg[5];
	int err;
	robo.ifr.ifr_data = (caddr_t) vecarg;
	vecarg[0] = (page) << 16;;
	vecarg[0] |= reg & 0xffff;
	err = do_ioctl(SIOCGETCROBORD);
	if (err < 0) {
		printk(KERN_ERR PFX "failed to read page %i with reg %i (%d)\n", page, reg, err);
	}
	return vecarg[1] & 0xffff;
#else

	__u16 ret;
	robo_reg(page, reg, REG_MII_ADDR_READ);
	ret = mdio_read(ROBO_PHY_ADDR, REG_MII_DATA0);
	return ret;
#endif
}

static __u32 robo_read32(__u8 page, __u8 reg)
{
#ifdef ROBO_SRAB
	__u32 vecarg[5];
	int err;
	robo.ifr.ifr_data = (caddr_t) vecarg;
	vecarg[0] = (page) << 16;;
	vecarg[0] |= reg & 0xffff;
	err = do_ioctl(SIOCGETCROBORD4);
	if (err < 0) {
		printk(KERN_ERR PFX "failed to read page %i with reg %i (%d)\n", page, reg, err);
	}
	return vecarg[1];
#else
	__u32 ret;
	robo_reg(page, reg, REG_MII_ADDR_READ);
	ret = mdio_read(ROBO_PHY_ADDR, REG_MII_DATA0) | (mdio_read(ROBO_PHY_ADDR, REG_MII_DATA0 + 1) << 16);
	return ret;
#endif
}

static void robo_write16(__u8 page, __u8 reg, __u16 val16)
{
#ifdef ROBO_SRAB
	__u32 vecarg[5];
	int err;
	robo.ifr.ifr_data = (caddr_t) vecarg;
	vecarg[0] = (page) << 16;;
	vecarg[0] |= reg & 0xffff;
	vecarg[1] = val16;
	err = do_ioctl(SIOCSETCROBOWR);
	if (err < 0) {
		printk(KERN_ERR PFX "failed to write page %i with reg %i and val %i (%d)\n", page, reg, val16, err);
	}
#else
	/* write data */
	mdio_write(ROBO_PHY_ADDR, REG_MII_DATA0, val16);

	robo_reg(page, reg, REG_MII_ADDR_WRITE);
#endif
}

static void robo_write8(__u8 page, __u8 reg, __u8 val8)
{
#ifdef ROBO_SRAB
	__u32 vecarg[5];
	int err;
	robo.ifr.ifr_data = (caddr_t) vecarg;
	vecarg[0] = (page) << 16;;
	vecarg[0] |= reg & 0xffff;
	vecarg[1] = val8;
	err = do_ioctl(SIOCSETCROBOWR1);
	if (err < 0) {
		printk(KERN_ERR PFX "failed to write page %i with reg %i and val %i (%d)\n", page, reg, val8, err);
	}
#else
	/* write data */
	mdio_write(ROBO_PHY_ADDR, REG_MII_DATA0, val8);

	robo_reg(page, reg, REG_MII_ADDR_WRITE);
#endif
}

static void robo_write32(__u8 page, __u8 reg, __u32 val32)
{
#ifdef ROBO_SRAB
	__u32 vecarg[5];
	int err;
	robo.ifr.ifr_data = (caddr_t) vecarg;
	vecarg[0] = (page) << 16;;
	vecarg[0] |= reg & 0xffff;
	vecarg[1] = val32;
	err = do_ioctl(SIOCSETCROBOWR4);
	if (err < 0) {
		printk(KERN_ERR PFX "failed to write page %i with reg %i and val %i (%d)\n", page, reg, val32, err);
	}
#else
	/* write data */
	mdio_write(ROBO_PHY_ADDR, REG_MII_DATA0, val32 & 0xFFFF);
	mdio_write(ROBO_PHY_ADDR, REG_MII_DATA0 + 1, val32 >> 16);

	robo_reg(page, reg, REG_MII_ADDR_WRITE);
#endif
}

/* checks that attached switch is 5365 */
static bool robo_bcm5365(void)
{
	/* set vlan access id to 15 and read it back */
	__u16 val16 = 15;
	robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_TABLE_ACCESS, val16);

	/* 5365 will refuse this as it does not have this reg */
	return robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_TABLE_ACCESS) != val16;
}

static bool robo_gmii(void)
{
	if (mdio_read(0, ROBO_MII_STAT) & 0x0100)
		return ((mdio_read(0, 0x0f) & 0xf000) != 0);
	return false;
}

#ifdef ROBO_SRAB
#include <bcmutils.h>

#define FLAG_TAGGED	't'	/* output tagged (external ports only) */
#define FLAG_UNTAG	'u'	/* input & output untagged (CPU port only, for OS (linux, ...) */
#define FLAG_LAN	'*'	/* input & output untagged (CPU port only, for CFE */

static int get_cpuport(void)
{
	char port[] = "XXXX", *next;
	const char *ports, *cur;
	int pid, len;
	/* get vlan member ports from nvram */
	ports = nvram_get("vlan1ports");
	if (!ports)
		return 8;

	/* search last port include '*' or 'u' */
	for (cur = ports; cur; cur = next) {
		/* tokenize the port list */
		while (*cur == ' ')
			cur++;
		next = bcmstrstr(cur, " ");
		len = next ? next - cur : strlen(cur);
		if (!len)
			break;
		if (len > sizeof(port) - 1)
			len = sizeof(port) - 1;
		strncpy(port, cur, len);
		port[len] = 0;

		/* make sure port # is within the range */
		pid = bcm_atoi(port);
		if (pid >= 9) {
			printk(KERN_ERR "get_cpuport: port %d in vlan1ports is out " "of range[0-8]\n", pid);
			return 8;
		}

		if (strchr(port, FLAG_LAN) || strchr(port, FLAG_UNTAG)) {
			/* Change it and return */
			printk(KERN_INFO "detected CPU Port is %d\n",pid);
			return pid;
		}
	}
	return 8;
}
#endif

#define MAX_NO_PHYS		5

static int robo_switch_enable(void)
{
	unsigned int i, last_port;
	u16 val;
	u8 cpuport;
#ifdef CONFIG_BCM47XX
	char *buf;
#endif
	uint16 val16 = 0;
	char *boothwmodel = nvram_get("boot_hw_model");
	char *boothwver = nvram_get("boot_hw_ver");
	char *boardnum = nvram_get("boardnum");
	char *boardtype = nvram_get("boardtype");
	char *cardbus = nvram_get("cardbus");

	val = robo_read16(ROBO_CTRL_PAGE, ROBO_SWITCH_MODE);
	if (!(val & (1 << 1))) {
		/* Unmanaged mode */
		val &= ~(1 << 0);
		/* With forwarding */
		val |= (1 << 1);
		robo_write16(ROBO_CTRL_PAGE, ROBO_SWITCH_MODE, val);
		val = robo_read16(ROBO_CTRL_PAGE, ROBO_SWITCH_MODE);
		if (!(val & (1 << 1))) {
			printk(KERN_ERR PFX "Failed to enable switch\n");
			return -EBUSY;
		}

		/* No spanning tree for unmanaged mode */
		last_port = ((robo.devid == ROBO_DEVICE_ID_5398) || ROBO_IS_BCM5301X(robo.devid)) ? ROBO_PORT7_CTRL : ROBO_PORT4_CTRL;
		for (i = ROBO_PORT0_CTRL; i <= last_port; i++) {
			if (ROBO_IS_BCM5301X(robo.devid) && i == ROBO_PORT6_CTRL)
				continue;
			robo_write16(ROBO_CTRL_PAGE, i, 0);
		}

		/* No spanning tree on IMP port too */
		robo_write16(ROBO_CTRL_PAGE, ROBO_IM_PORT_CTRL, 0);
	}

	if (robo.devid == ROBO_DEVICE_ID_53125 || (robo.devid == ROBO_DEVICE_ID_53115) || ROBO_IS_BCM5301X(robo.devid)) {
		/* Make IM port status link by default */
		val = robo_read16(ROBO_CTRL_PAGE, ROBO_PORT_OVERRIDE_CTRL) | 0xb1;
		robo_write16(ROBO_CTRL_PAGE, ROBO_PORT_OVERRIDE_CTRL, val);
		// TODO: init EEE feature
	}

	if (boothwmodel != NULL && !strcmp(boothwmodel, "E4200") && boothwver != NULL && !strcmp(boothwver, "1.0")) {
		printk(KERN_EMERG "%s:E4200 switch LEDs fix\n", __FILE__);
		/* Taken from cfe */
		val16 = 0x8008;
		robo_write16(ROBO_CTRL_PAGE, 0x12, val16);

		uint phy;
		for (phy = 0; phy < MAX_NO_PHYS; phy++) {
			mdio_write(phy, 0x1c, 0xb8aa);
			mdio_write(phy, 0x17, 0x0f04);
			mdio_write(phy, 0x15, 0x0088);
		}
	}

	if (boardnum != NULL && boardtype != NULL && cardbus != NULL)
		if (!strcmp(boardnum, "42") && !strcmp(boardtype, "0x478") && !strcmp(cardbus, "1") && (!boothwmodel || (strcmp(boothwmodel, "WRT300N") && strcmp(boothwmodel, "WRT610N")))) {
			printk(KERN_EMERG "%s:Enable WRT350 LED fix\n", __FILE__);
			/* WAN port LED */
			robo_write16(ROBO_CTRL_PAGE, 0x16, 0x1f);
		}
#ifdef ROBO_SRAB
	if (ROBO_IS_BCM5301X(robo.devid)) {
		cpuport = get_cpuport();
		val = robo_read16(ROBO_CTRL_PAGE, ROBO_REG_CTRL_PORT0_GMIIPO + cpuport);
		/* (GMII_SPEED_UP_2G|SW_OVERRIDE|TXFLOW_CNTL|RXFLOW_CNTL|LINK_STS) */
		val |= 0xf1;
		robo_write16(ROBO_CTRL_PAGE, ROBO_REG_CTRL_PORT0_GMIIPO + cpuport, val);
		char *asus = nvram_get("model");
		if (asus && (!strcmp(asus,"RT-AC87U") || !strcmp(asus,"RT-AC88U"))) {
		printk(KERN_INFO "workaround for RT-AC87U/RT-AC88U\n");
		//val = robo_read16(ROBO_CTRL_PAGE, ROBO_REG_CTRL_PORT5_GMIIPO);
		/* (GMII_SPEED_UP_2G|SW_OVERRIDE|TXFLOW_CNTL|RXFLOW_CNTL|LINK_STS) */
		val = 0xfb;
		robo_write16(ROBO_CTRL_PAGE, ROBO_REG_CTRL_PORT5_GMIIPO, val);
		}
	}
#endif

#ifdef CONFIG_BCM47XX
	/* WAN port LED, except for Netgear WGT634U */
	buf = nvram_get("nvram_type");
	if (buf) {
		if (strcmp(buf, "cfe") != 0)
			robo_write16(ROBO_CTRL_PAGE, 0x16, 0x1F);
	}
#endif
	return 0;
}

static void robo_switch_reset(void)
{
	if ((robo.devid == ROBO_DEVICE_ID_5395) || (robo.devid == ROBO_DEVICE_ID_5397) || (robo.devid == ROBO_DEVICE_ID_5398)) {
		/* Trigger a software reset. */
		robo_write16(ROBO_CTRL_PAGE, 0x79, 0x83);
		mdelay(500);
		robo_write16(ROBO_CTRL_PAGE, 0x79, 0);
	}
}

#ifdef CONFIG_BCM47XX
extern int gpio_kernel_api(unsigned int cmd, unsigned int mask, unsigned int val);

static int get_gpio_pin(const char *name)
{
	int i, err;
	char nvram_var[10];
	char *buf;

	for (i = 0; i < 16; i++) {
		err = snprintf(nvram_var, sizeof(nvram_var), "gpio%i", i);
		if (err <= 0)
			continue;
		buf = nvram_get(nvram_var);
		if (!buf)
			continue;
		if (!strcmp(name, buf))
			return i;
	}
	return -1;
}
#endif
static int iswrt610nv1 = 0;

static int robo_probe(char *devname)
{
	__u32 phyid;
	unsigned int i;
	int err = -1;
	int rreset=8;
	struct mii_ioctl_data *mii;

	char *boothwmodel = nvram_get("boot_hw_model");
	char *boothwver = nvram_get("boot_hw_ver");

	printk(KERN_INFO PFX "Probing device '%s'\n", devname);
	strcpy(robo.ifr.ifr_name, devname);

	if ((robo.dev = dev_get_by_name(&init_net, devname)) == NULL) {
		printk(KERN_ERR PFX "No such device\n");
		err = -ENODEV;
		goto err_done;
	}
	if (!robo.dev->netdev_ops || !robo.dev->netdev_ops->ndo_do_ioctl) {
		printk(KERN_ERR PFX "ndo_do_ioctl not implemented in ethernet driver\n");
		err = -ENXIO;
		goto err_put;
	}

	robo.device = devname;

	/* try access using MII ioctls - get phy address */
	err = do_ioctl(SIOCGMIIPHY);
	if (err < 0) {
		printk(KERN_ERR PFX "error (%i) while accessing MII phy registers with ioctls\n", err);
		goto err_put;
	}

	/* got phy address check for robo address */
	mii = if_mii(&robo.ifr);
	if ((mii->phy_id != ROBO_PHY_ADDR) && (mii->phy_id != ROBO_PHY_ADDR_BCM63XX) && (mii->phy_id != ROBO_PHY_ADDR_TG3)) {
		printk(KERN_ERR PFX "Invalid phy address (%d)\n", mii->phy_id);
		err = -ENODEV;
		goto err_put;
	}
#ifdef CONFIG_BCM47XX
	robo.gpio_lanports_enable = get_gpio_pin("lanports_enable");
	if (robo.gpio_lanports_enable >= 0) {
		gpio_kernel_api(1, 1 << robo.gpio_lanports_enable, 1 << robo.gpio_lanports_enable);
		gpio_kernel_api(0, 1 << robo.gpio_lanports_enable, 1 << robo.gpio_lanports_enable);
		mdelay(5);
	}
	robo.gpio_robo_reset = get_gpio_pin("robo_reset");
	if (robo.gpio_robo_reset == -1)
	    	robo.gpio_robo_reset = 8;
	    	
	if (boothwmodel == NULL || strcmp(boothwmodel, "WRT300N"))
		robo.gpio_robo_reset = -1;

	if (robo.gpio_robo_reset >= 0) {
		gpio_kernel_api(0, 1 << robo.gpio_robo_reset, 0);
		gpio_kernel_api(1, 1 << robo.gpio_robo_reset, 1 << robo.gpio_robo_reset);
		gpio_kernel_api(0, 1 << robo.gpio_robo_reset, 1 << robo.gpio_robo_reset);
		gpio_kernel_api(0, 1 << robo.gpio_robo_reset, 0);
		mdelay(50);

		gpio_kernel_api(0, 1 << robo.gpio_robo_reset, 1 << robo.gpio_robo_reset);
		mdelay(20);
	} else {
		// TODO: reset the internal robo switch
	}
#endif

	mii->phy_id = ROBO_PHY_ADDR;
	mii->reg_num = 0x2;
	err = do_ioctl(SIOCGMIIREG);
	phyid = mii->val_out;

	if (boothwmodel != NULL && !strcmp(boothwmodel, "WRT610N")
	    && boothwver != NULL && !strcmp(boothwver, "1.0")) {
		if (phyid == 0xffff) {

			mii->phy_id = ROBO_PHY_ADDR;
			mii->reg_num = 16;
			mii->val_in = 1;
			err = do_ioctl(SIOCSMIIREG);

			mii->phy_id = ROBO_PHY_ADDR;
			mii->reg_num = 0x2;
			err = do_ioctl(SIOCGMIIREG);
			phyid = mii->val_out;
		}
	}

	mii->phy_id = ROBO_PHY_ADDR;
	mii->reg_num = 0x3;
	err = do_ioctl(SIOCGMIIREG);
	phyid |= mii->val_out;

	if (phyid == 0xffffffff || phyid == 0x55210022) {
		printk(KERN_ERR PFX "No Robo switch in managed mode found, phy_id = 0x%08x\n", phyid);
		err = -ENODEV;
		goto err_put;
	}

	if (boothwmodel != NULL && !strcmp(boothwmodel, "WRT610N")
	    && boothwver != NULL && !strcmp(boothwver, "1.0"))
		iswrt610nv1 = 1;

#ifdef ROBO_SRAB
	robo.devid = robo_read32(ROBO_MGMT_PAGE, ROBO_DEVICE_ID);
#else
	/* Get the device ID */
	for (i = 0; i < 10; i++) {
		robo.devid = robo_read16(ROBO_MGMT_PAGE, ROBO_DEVICE_ID);
		if (robo.devid)
			break;
		udelay(10);
	}
#endif
	if (!robo.devid)
		robo.devid = ROBO_DEVICE_ID_5325;	/* Fake it */
	if (robo.devid == ROBO_DEVICE_ID_5325)
		robo.is_5365 = robo_bcm5365();
	else
		robo.is_5365 = false;

	robo.gmii = robo_gmii();
	if (robo.devid == ROBO_DEVICE_ID_5325) {
		for (i = 0; i < 5; i++)
			robo.port[i] = i;
	} else {
		for (i = 0; i < 8; i++)
			robo.port[i] = i;
	}
	robo.port[i] = ROBO_IM_PORT_CTRL;

	printk(KERN_INFO PFX "trying a %s%s%x!%s at %s\n", robo.devid & 0xf0000 ? "" : "5", robo.devid & 0xff00 ? "" : "3", robo.devid, robo.is_5365 ? " It's a BCM5365." : "", devname);

	if (!iswrt610nv1)
		robo_switch_reset();

	err = robo_switch_enable();
	if (err)
		goto err_put;

	printk(KERN_INFO PFX "found a %s%s%x!%s at %s\n", robo.devid & 0xf0000 ? "" : "5", robo.devid & 0xff00 ? "" : "3", robo.devid, robo.is_5365 ? " It's a BCM5365." : "", devname);

	return 0;

err_put:
	dev_put(robo.dev);
	robo.dev = NULL;
err_done:
	return err;
}

static int handle_vlan_port_read_old(switch_driver * d, char *buf, int nr)
{
	__u16 val16;
	int len = 0;
	int j;

	val16 = (nr) /* vlan */ |(0 << 12) /* read */ |(1 << 13) /* enable */ ;

	if (robo.is_5365) {
		robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_TABLE_ACCESS_5365, val16);
		/* actual read */
		val16 = robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_READ);
		if ((val16 & (1 << 14)) /* valid */ ) {
			for (j = 0; j < d->ports; j++) {
				if (val16 & (1 << j)) {
					len += sprintf(buf + len, "%d", j);
					if (val16 & (1 << (j + 7))) {
						if (j == d->cpuport)
							buf[len++] = 'u';
					} else {
						buf[len++] = 't';
						if (robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_PORT0_DEF_TAG + (j << 1)) == nr)
							buf[len++] = '*';
					}
					buf[len++] = '\t';
				}
			}
			len += sprintf(buf + len, "\n");
		}
	} else {
		u32 val32;
		robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_TABLE_ACCESS, val16);
		/* actual read */
		val32 = robo_read32(ROBO_VLAN_PAGE, ROBO_VLAN_READ);
		if ((val32 & (1 << 20)) /* valid */ ) {
			for (j = 0; j < d->ports; j++) {
				if (val32 & (1 << j)) {
					len += sprintf(buf + len, "%d", j);
					if (val32 & (1 << (j + d->ports))) {
						if (j == d->cpuport)
							buf[len++] = 'u';
					} else {
						buf[len++] = 't';
						if (robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_PORT0_DEF_TAG + (j << 1)) == nr)
							buf[len++] = '*';
					}
					buf[len++] = '\t';
				}
			}
			len += sprintf(buf + len, "\n");
		}
	}

	buf[len] = '\0';

	return len;
}

static int handle_vlan_port_read_new(switch_driver * d, char *buf, int nr)
{
	__u8 vtbl_entry, vtbl_index, vtbl_access;
	__u32 val32;
	int len = 0;
	int j;

	if ((robo.devid == ROBO_DEVICE_ID_5395) || (robo.devid == ROBO_DEVICE_ID_53115) || (robo.devid == ROBO_DEVICE_ID_53125) || (ROBO_IS_BCM5301X(robo.devid))) {
		vtbl_access = ROBO_VTBL_ACCESS_5395;
		vtbl_index = ROBO_VTBL_INDX_5395;
		vtbl_entry = ROBO_VTBL_ENTRY_5395;
	} else {
		vtbl_access = ROBO_VTBL_ACCESS;
		vtbl_index = ROBO_VTBL_INDX;
		vtbl_entry = ROBO_VTBL_ENTRY;
	}

	robo_write16(ROBO_ARLIO_PAGE, vtbl_index, nr);
	robo_write8(ROBO_ARLIO_PAGE, vtbl_access, (1 << 7) | (1 << 0));
	val32 = robo_read32(ROBO_ARLIO_PAGE, vtbl_entry);
	for (j = 0; j < d->ports; j++) {
		if (val32 & (1 << j)) {
			len += sprintf(buf + len, "%d", j);
			if (val32 & (1 << (j + d->ports))) {
				if (j == d->cpuport)
					buf[len++] = 'u';
			} else {
				buf[len++] = 't';
				if (robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_PORT0_DEF_TAG + (j << 1)) == nr)
					buf[len++] = '*';
			}
			buf[len++] = '\t';
		}
	}
	len += sprintf(buf + len, "\n");
	buf[len] = '\0';
	return len;
}

static int handle_vlan_port_read(void *driver, char *buf, int nr)
{
	switch_driver *d = (switch_driver *) driver;

	if (robo.devid != ROBO_DEVICE_ID_5325)
		return handle_vlan_port_read_new(d, buf, nr);
	else
		return handle_vlan_port_read_old(d, buf, nr);
}

static void handle_vlan_port_write_old(switch_driver * d, switch_vlan_config * c, int nr)
{
	__u16 val16;
	__u32 val32;
	__u32 untag = ((c->untag & ~(1 << d->cpuport)) << d->ports);

	/* write config now */
	val16 = (nr) /* vlan */ |(1 << 12) /* write */ |(1 << 13) /* enable */ ;
	if (robo.is_5365) {
		robo_write32(ROBO_VLAN_PAGE, ROBO_VLAN_WRITE_5365, (1 << 14) /* valid */ |(untag << 1) | c->port);
		robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_TABLE_ACCESS_5365, val16);
	} else {
		if (robo.corerev < 3)
			val32 = (1 << 20) | ((nr >> 4) << 12) | untag | c->port;
		else
			val32 = (1 << 24) | (nr << 12) | untag | c->port;
		robo_write32(ROBO_VLAN_PAGE, ROBO_VLAN_WRITE, val32);
		robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_TABLE_ACCESS, val16);
	}
}

static void handle_vlan_port_write_new(switch_driver * d, switch_vlan_config * c, int nr)
{
	__u8 vtbl_entry, vtbl_index, vtbl_access;
	__u32 untag = ((c->untag & ~(1 << d->cpuport)) << d->ports);

	/* write config now */
	if ((robo.devid == ROBO_DEVICE_ID_5395) || (robo.devid == ROBO_DEVICE_ID_53115) || (robo.devid == ROBO_DEVICE_ID_53125) || (ROBO_IS_BCM5301X(robo.devid))) {
		vtbl_access = ROBO_VTBL_ACCESS_5395;
		vtbl_index = ROBO_VTBL_INDX_5395;
		vtbl_entry = ROBO_VTBL_ENTRY_5395;
	} else {
		vtbl_access = ROBO_VTBL_ACCESS;
		vtbl_index = ROBO_VTBL_INDX;
		vtbl_entry = ROBO_VTBL_ENTRY;
	}

	robo_write32(ROBO_ARLIO_PAGE, vtbl_entry, untag | c->port);
	robo_write16(ROBO_ARLIO_PAGE, vtbl_index, nr);
	robo_write16(ROBO_ARLIO_PAGE, vtbl_access, 1 << 7);
}

static int handle_vlan_port_write(void *driver, char *buf, int nr)
{
	switch_driver *d = (switch_driver *) driver;
	switch_vlan_config *c = switch_parse_vlan(d, buf);
	int j;

	if (c == NULL)
		return -EINVAL;

	for (j = 0; j < d->ports; j++) {
		if ((c->untag | c->pvid) & (1 << j)) {
			/* change default vlan tag */
			robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_PORT0_DEF_TAG + (j << 1), nr);
		}
	}

	if (robo.devid != ROBO_DEVICE_ID_5325)
		handle_vlan_port_write_new(d, c, nr);
	else
		handle_vlan_port_write_old(d, c, nr);

	kfree(c);
	return 0;
}

#define set_switch(state) \
	robo_write16(ROBO_CTRL_PAGE, ROBO_SWITCH_MODE, (robo_read16(ROBO_CTRL_PAGE, ROBO_SWITCH_MODE) & ~2) | (state ? 2 : 0));

static int handle_enable_read(void *driver, char *buf, int nr)
{
	return sprintf(buf, "%d\n", (((robo_read16(ROBO_CTRL_PAGE, ROBO_SWITCH_MODE) & 2) == 2) ? 1 : 0));
}

static int handle_enable_write(void *driver, char *buf, int nr)
{
	set_switch(buf[0] == '1');

	return 0;
}

static int handle_port_enable_read(void *driver, char *buf, int nr)
{
	return sprintf(buf, "%d\n", ((robo_read16(ROBO_CTRL_PAGE, robo.port[nr]) & 3) == 3 ? 0 : 1));
}

static int handle_port_enable_write(void *driver, char *buf, int nr)
{
	u16 val16;

	if (buf[0] == '0')
		val16 = 3;	/* disabled */
	else if (buf[0] == '1')
		val16 = 0;	/* enabled */
	else
		return -EINVAL;

	robo_write16(ROBO_CTRL_PAGE, robo.port[nr], (robo_read16(ROBO_CTRL_PAGE, robo.port[nr]) & ~3) | val16);

	return 0;
}

static int handle_port_status_read(void *driver, char *buf, int nr)
{
	u16 speed_status = robo_read16(ROBO_STAT_PAGE, ROBO_SPEED_STAT_SUMMARY);
	u16 duplex_status = robo_read16(ROBO_STAT_PAGE, ROBO_DUPLEX_STAT_SUMMARY);
	int media, len;
	u16 mask;

	if ((robo.devid == ROBO_DEVICE_ID_53115) || (robo.devid == ROBO_DEVICE_ID_53125) || (ROBO_IS_BCM5301X(robo.devid))) {
		mask = 0x3 << (robo.port[nr] * 2);
		speed_status &= mask;
		speed_status >>= (robo.port[nr] * 2);
	} else {
		mask = 0x1 << (robo.port[nr]);
		speed_status &= mask;
		speed_status >>= robo.port[nr];
	}

	mask = 0x1 << (robo.port[nr]);
	duplex_status &= mask;
	duplex_status >>= robo.port[nr];

	switch (speed_status) {
	case 0:
		media = 0;
		break;
	case 1:
		media = SWITCH_MEDIA_100;
		break;
	case 2:
		media = SWITCH_MEDIA_1000;
		break;
	}
	if (duplex_status)
		media |= SWITCH_MEDIA_FD;

	if (ROBO_IS_BCM5301X(robo.devid) || robo.devid == ROBO_DEVICE_ID_53125 || robo.devid == ROBO_DEVICE_ID_53115) {	//special case for srab based devices like BCM4708
		speed_status = robo_read16(ROBO_STAT_PAGE, ROBO_LINK_STAT_SUMMARY);	// port status
	} else {
		speed_status = mdio_read(robo.port[nr], MII_BMSR);	// port status       
		if ((speed_status & 0x22) == 0x20)	// negotiation in progress, read again
			speed_status = mdio_read(robo.port[nr], MII_BMSR);
		if (speed_status & (1 << 2))
			speed_status = 0x1 << robo.port[nr];
	}
	mask = 0x1 << robo.port[nr];
	speed_status &= mask;
	speed_status >>= robo.port[nr];
	switch (speed_status) {
	case 0:
		len = sprintf(buf, "disconnected\n");
		return len;
		break;
	default:
		len = switch_print_media(buf, media);
		return len + sprintf(buf + len, "\n");
		break;
	}
}

static int handle_port_status_write(void *driver, char *buf, int nr)
{
	return 0;
}

static int handle_port_media_read(void *driver, char *buf, int nr)
{
	u16 bmcr = mdio_read(robo.port[nr], MII_BMCR);
	int media, len;
	if (bmcr & BMCR_ANENABLE)
		media = SWITCH_MEDIA_AUTO;
	else {
		if (bmcr & BMCR_SPEED1000)
			media = SWITCH_MEDIA_1000;
		else if (bmcr & BMCR_SPEED100)
			media = SWITCH_MEDIA_100;
		else
			media = 0;

		if (bmcr & BMCR_FULLDPLX)
			media |= SWITCH_MEDIA_FD;
	}
	len = switch_print_media(buf, media);
	return len + sprintf(buf + len, "\n");
}

static int handle_port_media_write(void *driver, char *buf, int nr)
{
	int media = switch_parse_media(buf);
	u16 bmcr, bmcr_mask;

	if (media & SWITCH_MEDIA_AUTO)
		bmcr = BMCR_ANENABLE | BMCR_ANRESTART;
	else {
		if (media & SWITCH_MEDIA_1000) {
			if (!robo.gmii)
				return -EINVAL;
			bmcr = BMCR_SPEED1000;
		} else if (media & SWITCH_MEDIA_100)
			bmcr = BMCR_SPEED100;
		else
			bmcr = 0;

		if (media & SWITCH_MEDIA_FD)
			bmcr |= BMCR_FULLDPLX;
	}

	bmcr_mask = ~(BMCR_SPEED1000 | BMCR_SPEED100 | BMCR_FULLDPLX | BMCR_ANENABLE | BMCR_ANRESTART);

	mdio_write(robo.port[nr], MII_BMCR, (mdio_read(robo.port[nr], MII_BMCR) & bmcr_mask) | bmcr);

	return 0;
}

static int handle_enable_vlan_read(void *driver, char *buf, int nr)
{
	return sprintf(buf, "%d\n", (((robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL0) & (1 << 7)) == (1 << 7)) ? 1 : 0));
}

static int handle_enable_vlan_write(void *driver, char *buf, int nr)
{
	__u16 val16;
	int disable = ((buf[0] != '1') ? 1 : 0);

	val16 = robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL0);
	robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL0, disable ? 0 : val16 | (1 << 7) /* 802.1Q VLAN */ |(3 << 5) /* mac check and hash */ );

	val16 = robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL1);
	robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL1, disable ? 0 : val16 | (robo.devid == ROBO_DEVICE_ID_5325 ? (1 << 1) : 0) | (1 << 2) | (1 << 3));	/* RSV multicast */

	if (robo.devid != ROBO_DEVICE_ID_5325)
		return 0;

//      robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL4, disable ? 0 :
//              (1 << 6) /* drop invalid VID frames */);
	robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL4, disable ? 0 : (2 << 6) /* do not check */ );
	robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL5, disable ? 0 : (1 << 3) /* drop miss V table frames */ );

	return 0;
}

static int handle_enable_4095_write(void *driver, char *buf, int nr)
{
	__u16 val16;
	int disable = ((buf[0] != '1') ? 1 : 0);

	if (robo.devid == ROBO_DEVICE_ID_5325 || robo.is_5365)
		return 0;
	val16 = robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL5);
	if (disable)
		val16 &= 1 << 2;
	else
		val16 |= 1 << 2;
	robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL5, val16);

	return 0;
}

static int handle_enable_4095_read(void *driver, char *buf, int nr)
{
	__u16 val16;
	if (robo.devid == ROBO_DEVICE_ID_5325 || robo.is_5365)
		return sprintf(buf, "%d\n", 0);

	return sprintf(buf, "%d\n", (((robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL5) & (1 << 2)) == (1 << 2)) ? 1 : 0));
}

static int handle_enable_jumbo_write(void *driver, char *buf, int nr)
{
	__u16 val16;
	u32 port_mask = 0;
	u16 max_size = JMS_MIN_SIZE;
	int disable = ((buf[0] != '1') ? 1 : 0);

	if (robo.devid == ROBO_DEVICE_ID_5325 || robo.is_5365)
		return 0;

	if (!disable) {
		port_mask = 0x1f;
		if (robo.devid == ROBO_DEVICE_ID_5398)
			port_mask = 0x7f;
		max_size = JMS_MAX_SIZE;
		port_mask |= JPM_10_100_JUMBO_EN;
	}

	robo_write32(B53_JUMBO_PAGE, B53_JUMBO_PORT_MASK, port_mask);
	robo_write16(B53_JUMBO_PAGE, B53_JUMBO_MAX_SIZE, max_size);
	return 0;
}

static int handle_enable_jumbo_read(void *driver, char *buf, int nr)
{
	__u16 val16;
	if (robo.devid == ROBO_DEVICE_ID_5325 || robo.is_5365)
		return sprintf(buf, "%d\n", 0);

	return sprintf(buf, "%d\n", robo_read16(B53_JUMBO_PAGE, B53_JUMBO_MAX_SIZE));
}

static void handle_reset_old(switch_driver * d, char *buf, int nr)
{
	int j;
	__u16 val16;

	/* reset vlans */
	for (j = 0; j <= ((robo.is_5365) ? VLAN_ID_MAX_5365 : VLAN_ID_MAX); j++) {
		/* write config now */
		val16 = (j) /* vlan */ |(1 << 12) /* write */ |(1 << 13) /* enable */ ;
		if (robo.is_5365)
			robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_WRITE_5365, 0);
		else
			robo_write32(ROBO_VLAN_PAGE, ROBO_VLAN_WRITE, 0);
		robo_write16(ROBO_VLAN_PAGE, robo.is_5365 ? ROBO_VLAN_TABLE_ACCESS_5365 : ROBO_VLAN_TABLE_ACCESS, val16);
	}
}

static void handle_reset_new(switch_driver * d, char *buf, int nr)
{
	int j;
	__u8 vtbl_entry, vtbl_index, vtbl_access;

	if ((robo.devid == ROBO_DEVICE_ID_5395) || (robo.devid == ROBO_DEVICE_ID_53115) || (robo.devid == ROBO_DEVICE_ID_53125) || (ROBO_IS_BCM5301X(robo.devid))) {
		vtbl_access = ROBO_VTBL_ACCESS_5395;
		vtbl_index = ROBO_VTBL_INDX_5395;
		vtbl_entry = ROBO_VTBL_ENTRY_5395;
	} else {
		vtbl_access = ROBO_VTBL_ACCESS;
		vtbl_index = ROBO_VTBL_INDX;
		vtbl_entry = ROBO_VTBL_ENTRY;
	}

	for (j = 0; j <= VLAN_ID_MAX; j++) {
		/* write config now */
		robo_write32(ROBO_ARLIO_PAGE, vtbl_entry, 0);
		robo_write16(ROBO_ARLIO_PAGE, vtbl_index, j);
		robo_write16(ROBO_ARLIO_PAGE, vtbl_access, 1 << 7);
	}
}

static int
s_nvram_match(const char *name, const char *match)
{
	const char *value = nvram_get(name);
	return (value && !strcmp(value, match));
}
static int handle_reset(void *driver, char *buf, int nr)
{
	int j;
	switch_driver *d = (switch_driver *) driver;
	
	char *boardnum = nvram_get("boardnum");
	char *boardrev = nvram_get("boardrev");
	char *boardtype = nvram_get("boardtype");

	/* disable switching */
	set_switch(0);


	if (robo.devid != ROBO_DEVICE_ID_5325)
		handle_reset_new(d, buf, nr);
	else
		handle_reset_old(d, buf, nr);


	/* reset ports to a known good state */
	if (boardnum && boardtype && boardrev && !strcmp(boardnum, "32") && ( !strcmp(boardtype, "0x0665") || !strcmp(boardtype,"0x072F") ) && !strcmp(boardrev, "0x1101")) {
		//do nothing
		printk(KERN_INFO "Netgear R8000 workaround\n");
	} else if (boardnum && boardtype && boardrev && !strcmp(boardnum,"1234") && !strcmp(boardtype,"0x072F") && !strcmp(boardrev, "0x1202")) {
		printk(KERN_INFO "Handle TEW828 workaround\n");
	} else if (s_nvram_match("model", "RT-AC1200G+")) {
		printk(KERN_INFO "Handle Asus RT-AC1200G+\n");
	} else if (s_nvram_match("model", "RT-AC3100")) {
		printk(KERN_INFO "Handle Asus RT-AC3100\n");
	} else if ((s_nvram_match("boardnum", "24") || s_nvram_match("boardnum", "N/A")) && s_nvram_match("boardtype", "0x072F") && s_nvram_match("1:devid", "0x43c5")
	    && s_nvram_match("boardrev", "0x1101")
	    && s_nvram_match("gpio7", "wps_button")) {
		printk(KERN_INFO "Handle DIR885 workaround\n");
	} else {
		for (j = 0; j < d->ports; j++) {
			robo_write16(ROBO_CTRL_PAGE, robo.port[j], 0x0000);
			robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_PORT0_DEF_TAG + (j << 1), 0);
		}
	}


	/* enable switching */
	set_switch(1);

	/* enable vlans */
	handle_enable_vlan_write(driver, "1", 0);

	return 0;
}

static int handle_port_bandwidth_read(void *driver, char *buf, int nr)
{
	return sprintf(buf, "FULL\n");
}

static int handle_port_bandwidth_write(void *driver, char *buf, int nr)
{
	return 0;
}

static int handle_port_prio_enable_read(void *driver, char *buf, int nr)
{
	return sprintf(buf, "%d\n", ((robo_read16(ROBO_QOS_PAGE, ROBO_QOS_CTRL) & BIT(robo.port[nr])) ? 1 : 0));
}

static int handle_port_prio_enable_write(void *driver, char *buf, int nr)
{
	__u16 val16;

	val16 = robo_read16(ROBO_QOS_PAGE, ROBO_QOS_CTRL);

	if (buf[0] == '0')
		val16 &= ~(1 << robo.port[nr]);
	else
		val16 |= (1 << robo.port[nr]);

	robo_write16(ROBO_QOS_PAGE, ROBO_QOS_CTRL, val16);

	return 0;
}

static int handle_port_prio_read(void *driver, char *buf, int nr)
{
	return handle_port_prio_enable_read(driver, buf, nr);
}

static int handle_port_prio_write(void *driver, char *buf, int nr)
{
	return handle_port_prio_enable_write(driver, buf, nr);
}

static int handle_port_flow_enable_read(void *driver, char *buf, int nr)
{
	return sprintf(buf, "%d\n", ((robo_read16(ROBO_QOS_PAGE, ROBO_QOS_PAUSE_ENA) & BIT(robo.port[nr])) ? 1 : 0));
}

static int handle_port_flow_enable_write(void *driver, char *buf, int nr)
{
	__u16 val16;

	val16 = robo_read16(ROBO_QOS_PAGE, ROBO_QOS_PAUSE_ENA);

	if (buf[0] == '0')
		val16 &= ~(1 << robo.port[nr]);
	else
		val16 |= (1 << robo.port[nr]);

	robo_write16(ROBO_QOS_PAGE, ROBO_QOS_PAUSE_ENA, val16);

	return 0;
}

// Added by quarkysg 3 Dec 17
static int handle_port_vlanpcp_read(void *driver, char *buf, int nr)
{
	if (nr > 4) {
		return sprintf(buf, "Port[%d] VLAN priority confiuration unsupported!\n", nr);
	}

	int vlanTag = robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_PORT0_DEF_TAG + (nr << 1));
	int vlanId = vlanTag & 4095;	// VLAN ID is the from bit 11 - 0
	int vlanPcp = (vlanTag & 57344) >> 13;	// VLAN PCP is from bit 15 - 13

	return sprintf(buf, "Port [%d]: VLAN ID[%d], VLAN PCP[%d], VLAN Tag[%d]\n", nr, vlanId, vlanPcp, vlanTag);
}

static int handle_port_vlanpcp_write(void *driver, char *buf, int nr)
{
	if (nr > 4) {
		printk(KERN_WARNING "Port[%d] VLAN configuration unsupported!\n", nr);
		return 0;
	}

	int vlanTag = robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_PORT0_DEF_TAG + (nr << 1));
	long longVlanPcp;
	int vlanPcp;

	if (kstrtol(buf, 10, &longVlanPcp) == 0) {
		//printk(KERN_INFO "Port [%d] VLAN PCP[%d] write requested\n", nr, longVlanPcp);
		if (longVlanPcp > -1 && longVlanPcp < 8) {
			vlanPcp = longVlanPcp << 13;
			vlanTag = (vlanTag & 8191) | vlanPcp;

			robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_PORT0_DEF_TAG + (nr << 1), (__u16) vlanTag);
		}
		else {
			printk(KERN_WARNING "[%d] is not a valid VLAN PCP\n", (int) longVlanPcp);
		}
	}
	else {
		printk(KERN_WARNING "[%s] is not a valid VLAN PCP number\n", buf);
	}

	return 0;
}
// End add by quarkysg, 3 Dec 2017

static int __init robo_init(void)
{
	int notfound = 1;
	char *device;

	device = strdup("ethX");
	for (device[3] = '0'; (device[3] <= '3') && notfound; device[3]++) {
		if (!switch_device_registered(device))
			notfound = robo_probe(device);
	}
	device[3]--;

	if (notfound) {
		kfree(device);
		return -ENODEV;
	} else {
		static const switch_config cfg[] = {
			{
			 .name = "enable",
			 .read = handle_enable_read,
			 .write = handle_enable_write},	// 
			{
			 .name = "enable_vlan",
			 .read = handle_enable_vlan_read,
			 .write = handle_enable_vlan_write},	//
			{
			 .name = "enable_4095",
			 .read = handle_enable_4095_read,
			 .write = handle_enable_4095_write},	// 
			{
			 .name = "enable_jumbo",
			 .read = handle_enable_jumbo_read,
			 .write = handle_enable_jumbo_write},	//
			{
			 .name = "reset",
			 .read = NULL,
			 .write = handle_reset},	//
			{NULL,},
		};
		static const switch_config port[] = {
			{
			 .name = "enable",
			 .read = handle_port_enable_read,
			 .write = handle_port_enable_write},	//
			{
			 .name = "media",
			 .read = handle_port_media_read,
			 .write = handle_port_media_write},	//
			{
			 .name = "status",
			 .read = handle_port_status_read,
			 .write = handle_port_status_write},	//
			{
			 .name = "bandwidth",
			 .read = handle_port_bandwidth_read,
			 .write = handle_port_bandwidth_write},	//
			{
			 .name = "prio-enable",
			 .read = handle_port_prio_enable_read,
			 .write = handle_port_prio_enable_write},	//
			{.name = "prio",
			 .read = handle_port_prio_read,
			 .write = handle_port_prio_write},	//
			{.name = "flow",
			 .read = handle_port_flow_enable_read,
			 .write = handle_port_flow_enable_write},	// 
			// Added by quarkysg, 3 Dec 17
			{.name = "vlanpcp",
			 .read = handle_port_vlanpcp_read,
			 .write = handle_port_vlanpcp_write},
			// End add by quarkysg, 3 Dec 17
			{NULL,},
		};
		static const switch_config vlan[] = {
			{
			 .name = "ports",
			 .read = handle_vlan_port_read,
			 .write = handle_vlan_port_write}, {NULL,},
		};
		switch_driver driver = {
			.name = DRIVER_NAME,
			.version = DRIVER_VERSION,
			.interface = device,
			.cpuport = 5,
			.ports = 6,
			//.vlans = 16,
			.vlans = 4096,	// Modified by quarkysg, 13 Sep 2017
			.driver_handlers = cfg,
			.port_handlers = port,
			.vlan_handlers = vlan,
		};
		if (robo.devid != ROBO_DEVICE_ID_5325) {
			driver.ports = 9;
			if (!ROBO_IS_BCM5301X(robo.devid))
				driver.cpuport = 8;
#ifdef ROBO_SRAB
			else
				driver.cpuport = get_cpuport();
#endif
		}
		if (robo.is_5365)
			snprintf(driver.dev_name, SWITCH_NAME_BUFSZ, "BCM5365");
		else
			snprintf(driver.dev_name, SWITCH_NAME_BUFSZ, "BCM%s%s%x", robo.devid & 0xf0000 ? "" : "5", robo.devid & 0xff00 ? "" : "3", robo.devid);

		return switch_register_driver(&driver);
	}
}

static void __exit robo_exit(void)
{
	switch_unregister_driver(DRIVER_NAME);
	if (robo.dev)
		dev_put(robo.dev);
	kfree(robo.device);
}

MODULE_AUTHOR("Felix Fietkau <openwrt@nbd.name>");
MODULE_LICENSE("GPL");

module_init(robo_init);
module_exit(robo_exit);
