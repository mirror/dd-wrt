/*
 * Broadcom BCM5325E/536x switch configuration module
 *
 * Copyright (C) 2005 Felix Fietkau <nbd@nbd.name>
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
 *
 */


#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <asm/uaccess.h>

#include "switch-core.h"
#include "etc53xx.h"

#include <bcmdevs.h>

#define DRIVER_NAME		"bcm53xx"
#define DRIVER_VERSION	"0.01"

#define ROBO_PHY_ADDR	0x1E	/* robo switch phy address */

/* MII registers */
#define REG_MII_PAGE	0x10	/* MII Page register */
#define REG_MII_ADDR	0x11	/* MII Address register */
#define REG_MII_DATA0	0x18	/* MII Data register 0 */

#define REG_MII_PAGE_ENABLE	1
#define REG_MII_ADDR_WRITE	1
#define REG_MII_ADDR_READ	2

/* Private et.o ioctls */
#define SIOCGETCPHYRD           (SIOCDEVPRIVATE + 9)
#define SIOCSETCPHYWR           (SIOCDEVPRIVATE + 10)

//#define ROBO_DBG(fmt, args...) printk(fmt, ##args);
#define ROBO_DBG(fmt, args...)

static int use_et = 0;
static int is_5350 = 0;
static struct ifreq ifr;
static struct net_device *dev;
static unsigned char port[6] = { 0, 1, 2, 3, 4, 8 };

static int do_ioctl(int cmd, void *buf)
{
	mm_segment_t old_fs = get_fs();
	int ret;

	if (buf != NULL)
		ifr.ifr_data = (caddr_t) buf;

	set_fs(KERNEL_DS);
	ret = dev->do_ioctl(dev, &ifr, cmd);
	set_fs(old_fs);

	return ret;
}

static u16 mdio_read(__u16 phy_id, __u8 reg)
{
	if (use_et) {
		int args[2] = { reg };
		
		if (phy_id != ROBO_PHY_ADDR) {
			ROBO_DBG(
				"Access to real 'phy' registers unavaliable.\n"
				"Upgrade kernel driver.\n");

			return 0xffff;
		}


		if (do_ioctl(SIOCGETCPHYRD, &args) < 0) {
			ROBO_DBG("[%s:%d] SIOCGETCPHYRD failed!\n", __FILE__, __LINE__);
			return 0xffff;
		}
	
		return args[1];
	} else {
		struct mii_ioctl_data *mii = (struct mii_ioctl_data *) &ifr.ifr_data;
		mii->phy_id = phy_id;
		mii->reg_num = reg;

		if (do_ioctl(SIOCGMIIREG, NULL) < 0) {
			ROBO_DBG("[%s:%d] SIOCGMIIREG failed!\n", __FILE__, __LINE__);

			return 0xffff;
		}

		return mii->val_out;
	}
}

static void mdio_write(__u16 phy_id, __u8 reg, __u16 val)
{
	if (use_et) {
		int args[2] = { reg, val };

		if (phy_id != ROBO_PHY_ADDR) {
			ROBO_DBG(
				"Access to real 'phy' registers unavaliable.\n"
				"Upgrade kernel driver.\n");

			return;
		}
		
		if (do_ioctl(SIOCSETCPHYWR, args) < 0) {
			ROBO_DBG("[%s:%d] SIOCGETCPHYWR failed!\n", __FILE__, __LINE__);
			return;
		}
	} else {
		struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifr.ifr_data;

		mii->phy_id = phy_id;
		mii->reg_num = reg;
		mii->val_in = val;

		if (do_ioctl(SIOCSMIIREG, NULL) < 0) {
			ROBO_DBG("[%s:%d] SIOCSMIIREG failed!\n", __FILE__, __LINE__);
			return;
		}
	}
}

static int robo_reg(__u8 page, __u8 reg, __u8 op)
{
	int i = 3;
	
	/* set page number */
	mdio_write(ROBO_PHY_ADDR, REG_MII_PAGE, 
		(page << 8) | REG_MII_PAGE_ENABLE);
	
	/* set register address */
	mdio_write(ROBO_PHY_ADDR, REG_MII_ADDR, 
		(reg << 8) | op);

	/* check if operation completed */
	while (i--) {
		if ((mdio_read(ROBO_PHY_ADDR, REG_MII_ADDR) & 3) == 0)
			return 0;
	}

	ROBO_DBG("[%s:%d] timeout in robo_reg!\n", __FILE__, __LINE__);
	
	return 0;
}

static void robo_read(__u8 page, __u8 reg, __u16 *val, int count)
{
	int i;
	
	robo_reg(page, reg, REG_MII_ADDR_READ);
	
	for (i = 0; i < count; i++)
		val[i] = mdio_read(ROBO_PHY_ADDR, REG_MII_DATA0 + i);
}

static __u16 robo_read16(__u8 page, __u8 reg)
{
	robo_reg(page, reg, REG_MII_ADDR_READ);
	
	return mdio_read(ROBO_PHY_ADDR, REG_MII_DATA0);
}

static __u32 robo_read32(__u8 page, __u8 reg)
{
	robo_reg(page, reg, REG_MII_ADDR_READ);
	
	return mdio_read(ROBO_PHY_ADDR, REG_MII_DATA0) +
		(mdio_read(ROBO_PHY_ADDR, REG_MII_DATA0 + 1) << 16);
}

static void robo_write16(__u8 page, __u8 reg, __u16 val16)
{
	/* write data */
	mdio_write(ROBO_PHY_ADDR, REG_MII_DATA0, val16);

	robo_reg(page, reg, REG_MII_ADDR_WRITE);
}

static void robo_write32(__u8 page, __u8 reg, __u32 val32)
{
	/* write data */
	mdio_write(ROBO_PHY_ADDR, REG_MII_DATA0, val32 & 65535);
	mdio_write(ROBO_PHY_ADDR, REG_MII_DATA0 + 1, val32 >> 16);
	
	robo_reg(page, reg, REG_MII_ADDR_WRITE);
}

/* checks that attached switch is 5325E/5350 */
static int robo_vlan5350(void)
{
	/* set vlan access id to 15 and read it back */
	__u16 val16 = 15;
	robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_TABLE_ACCESS_5350, val16);
	
	/* 5365 will refuse this as it does not have this reg */
	return (robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_TABLE_ACCESS_5350) == val16);
}



static int robo_probe(char *devname)
{
	struct ethtool_drvinfo info;
	__u32 phyid;

	ROBO_DBG("Probing device %s: ", devname);
	strcpy(ifr.ifr_name, devname);

	if ((dev = dev_get_by_name(devname)) == NULL) {
		ROBO_DBG("Device %s not present\n", devname);
		return 1;
	}

	info.cmd = ETHTOOL_GDRVINFO;
	if (do_ioctl(SIOCETHTOOL, (void *) &info) < 0) {
		ROBO_DBG("SIOCETHTOOL: not supported\n");
		return 1;
	}
	
	/* try access using MII ioctls - get phy address */
	if (do_ioctl(SIOCGMIIPHY, NULL) < 0) {
		use_et = 1;
	} else {
		/* got phy address check for robo address */
		struct mii_ioctl_data *mii = (struct mii_ioctl_data *) &ifr.ifr_data;
		if (mii->phy_id != ROBO_PHY_ADDR) {
			ROBO_DBG("Invalid phy address (%d)\n", mii->phy_id);
			return 1;
		}
	}

	phyid = mdio_read(ROBO_PHY_ADDR, 0x2) | 
		(mdio_read(ROBO_PHY_ADDR, 0x3) << 16);
		
/* set phy id number, used for id-ing hardware. Eko */
	char buff[32];
	sprintf (buff, "%#010x", phyid);
	nvram_set ("phyid_num", buff);
/* end */

	if (phyid == 0xffffffff || phyid == 0x55210022) {
		ROBO_DBG("No Robo switch in managed mode found %X\n",phyid);
		return 1;
	}
	
	is_5350 = robo_vlan5350();
	
	ROBO_DBG("found!\n");
	return 0;
}

static int handle_port_enable_read(void *driver, char *buf, int nr)
{
	__u16 val16;

	if(nr == 5) nr = 8;

	val16 = robo_read16(ROBO_CTRL_PAGE, nr);

	return sprintf(buf, "%d\n", (val16 & 3) ? 0 : 1);
}

static int handle_port_enable_write(void *driver, char *buf, int nr)
{
	__u16 val16=0;

	if(nr == 5) nr = 8;

	val16 = robo_read16(ROBO_CTRL_PAGE, nr);

	if (buf[0] == '0')
		val16 = val16 | 3;
	else if (buf[0] == '1')
		val16 = val16 & ~3;
	else return -1;

	robo_write16(ROBO_CTRL_PAGE, nr, val16);

	return 0;
}

static int handle_port_media_read(void *driver, char *buf, int nr)
{
	__u16 val16=0;
	int len, media=0;
	ROBO_MII_CTRL_STRUC* mii;

	if(nr == 5) nr = 8;

	mii = (ROBO_MII_CTRL_STRUC*)&val16;

	val16 = robo_read16(ROBO_PORT0_MII_PAGE+nr, ROBO_MII_AUX_CTRL);

//	ROBO_DBG(KERN_EMERG "handle_port_media_read: port %d read 0x%X\n", nr, val16);

/*	if (mii->ANenable)
		media |= SWITCH_MEDIA_AUTO;
	if (mii->speed)
		media |= SWITCH_MEDIA_100;
	if (mii->duplex)
		media |= SWITCH_MEDIA_FD;
*/

	if (val16 & ROBO_MII_AUX_CTRL_SP100)
		media |= SWITCH_MEDIA_100;
	if (val16 & ROBO_MII_AUX_CTRL_FD)
		media |= SWITCH_MEDIA_FD;

	len = switch_print_media(buf, media);
	return len + sprintf(buf + len, "\n");
}

static int handle_port_media_write(void *driver, char *buf, int nr)
{
	int media = switch_parse_media(buf);
	__u16 val16;
	ROBO_MII_CTRL_STRUC* mii;

return 0;

	if(nr == 5) nr = 8;

	mii = (ROBO_MII_CTRL_STRUC*)&val16;

	val16 = robo_read16(ROBO_PORT0_MII_PAGE+nr, ROBO_MII_AUX_CTRL);

//	ROBO_DBG(KERN_EMERG "handle_port_media_write: port %d read 0x%X\n", nr, val16);

	if (media < 0)
		return -1;

/*	mii->ANenable = (media & SWITCH_MEDIA_AUTO) ? 1 : 0;
	mii->speed = (media & SWITCH_MEDIA_100)  ? 1 : 0;
	mii->duplex = (media & SWITCH_MEDIA_FD)  ? 1 : 0;
*/

	if(media & SWITCH_MEDIA_100)
		val16 |= ROBO_MII_AUX_CTRL_SP100;
	else
		val16 &= ~ROBO_MII_AUX_CTRL_SP100;

	if(media & SWITCH_MEDIA_FD)
		val16 |= ROBO_MII_AUX_CTRL_FD;
	else
		val16 &= ~ROBO_MII_AUX_CTRL_FD;

//	ROBO_DBG(KERN_EMERG "handle_port_media_write: writing 0x%X\n", val16);

	robo_write16(ROBO_PORT0_MII_PAGE+nr, ROBO_MII_AUX_CTRL, val16);
	
	return 0;
}

static int handle_vlan_port_read(void *driver, char *buf, int nr)
{
	__u16 val16;
	int len = 0;
	int j;

	val16 = (nr) /* vlan */ | (0 << 12) /* read */ | (1 << 13) /* enable */;
	
	if (is_5350) {
		u32 val32;
		robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_TABLE_ACCESS_5350, val16);
		/* actual read */
		val32 = robo_read32(ROBO_VLAN_PAGE, ROBO_VLAN_READ);
		if ((val32 & (1 << 20)) /* valid */) {
			for (j = 0; j < 6; j++) {
				if (val32 & (1 << j)) {
					len += sprintf(buf + len, "%d", j);
					if (val32 & (1 << (j + 6))) {
						if (j == 5) buf[len++] = 'u';
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
		robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_TABLE_ACCESS, val16);
		/* actual read */
		val16 = robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_READ);
		if ((val16 & (1 << 14)) /* valid */) {
			for (j = 0; j < 6; j++) {
				if (val16 & (1 << j)) {
					len += sprintf(buf + len, "%d", j);
					if (val16 & (1 << (j + 7))) {
						if (j == 5) buf[len++] = 'u';
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

	return len;
}

static int handle_vlan_port_write(void *driver, char *buf, int nr)
{
	switch_driver *d = (switch_driver *) driver;
	switch_vlan_config *c = switch_parse_vlan(d, buf);
	int j;
	__u16 val16;
	
	if (c == NULL)
		return -EINVAL;

	for (j = 0; j < d->ports; j++) {
		if ((c->untag | c->pvid) & (1 << j))
			/* change default vlan tag */
			robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_PORT0_DEF_TAG + (j << 1), nr);
	}

	/* write config now */
	val16 = (nr) /* vlan */ | (1 << 12) /* write */ | (1 << 13) /* enable */;
	if (is_5350) {
		robo_write32(ROBO_VLAN_PAGE, ROBO_VLAN_WRITE_5350,
			(1 << 20) /* valid */ | (c->untag << 6) | c->port);
		robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_TABLE_ACCESS_5350, val16);
	} else {
		robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_WRITE,
			(1 << 14)  /* valid */ | (c->untag << 7) | c->port);
		robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_TABLE_ACCESS, val16);
	}

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

static int handle_enable_vlan_read(void *driver, char *buf, int nr)
{
	return sprintf(buf, "%d\n", (((robo_read16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL0) & (1 << 7)) == (1 << 7)) ? 1 : 0));
}

static int handle_enable_vlan_write(void *driver, char *buf, int nr)
{
	int disable = ((buf[0] != '1') ? 1 : 0);
	
	robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL0, disable ? 0 :
		(1 << 7) /* 802.1Q VLAN */ | (3 << 5) /* mac check and hash */);
	robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL1, disable ? 0 :
		(1 << 1) | (1 << 2) | (1 << 3) /* RSV multicast */);
	robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL4, disable ? 0 :
		(1 << 6) /* drop invalid VID frames */);
	robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_CTRL5, disable ? 0 :
		(1 << 3) /* drop miss V table frames */);

	return 0;
}

static int handle_reset(void *driver, char *buf, int nr)
{
	switch_driver *d = (switch_driver *) driver;
	switch_vlan_config *c = switch_parse_vlan(d, buf);
	int j;
	__u16 val16;
	
	if (c == NULL)
		return -EINVAL;

	/* disable switching */
	set_switch(0);

	/* reset vlans */
	for (j = 0; j <= (is_5350 ? VLAN_ID_MAX5350 : VLAN_ID_MAX); j++) {
		/* write config now */
		val16 = (j) /* vlan */ | (1 << 12) /* write */ | (1 << 13) /* enable */;
		if (is_5350)
			robo_write32(ROBO_VLAN_PAGE, ROBO_VLAN_WRITE_5350, 0);
		else
			robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_WRITE, 0);
		robo_write16(ROBO_VLAN_PAGE, (is_5350 ? ROBO_VLAN_TABLE_ACCESS_5350 : ROBO_VLAN_TABLE_ACCESS), val16);
	}

	/* reset ports to a known good state */
	for (j = 0; j < d->ports; j++) {
		robo_write16(ROBO_CTRL_PAGE, port[j], 0x0000);
		robo_write16(ROBO_VLAN_PAGE, ROBO_VLAN_PORT0_DEF_TAG + (j << 1), 0);
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
	return sprintf(buf, "%d\n", ((robo_read16(ROBO_QOS_PAGE, ROBO_QOS_CTRL) & BIT(port[nr])) ? 1 : 0));
}

static int handle_port_prio_enable_write(void *driver, char *buf, int nr)
{
	__u16 val16;
	
	val16 = robo_read16(ROBO_QOS_PAGE, ROBO_QOS_CTRL);

	if (buf[0] == '0')
		val16 &= ~(1 << port[nr]);
	else
		val16 |= (1 << port[nr]);

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
	return sprintf(buf, "%d\n", ((robo_read16(ROBO_QOS_PAGE, ROBO_QOS_PAUSE_ENA) & BIT(port[nr])) ? 1 : 0));
}

static int handle_port_flow_enable_write(void *driver, char *buf, int nr)
{
	__u16 val16;
	
	val16 = robo_read16(ROBO_QOS_PAGE, ROBO_QOS_PAUSE_ENA);

	if (buf[0] == '0')
		val16 &= ~(1 << port[nr]);
	else
		val16 |= (1 << port[nr]);

	robo_write16(ROBO_QOS_PAGE, ROBO_QOS_PAUSE_ENA, val16);

	return 0;
}

static int __init robo_init(void)
{
	char *device = "ethX";
	int notfound = 1;

	for (device[3] = '0'; (device[3] <= '3') && notfound; device[3]++) {
		notfound = robo_probe(device);
	}
	device[3]--;
	
	if (notfound)
		return -ENODEV;
	else {
		switch_config cfg[] = {
			{"enable", handle_enable_read, handle_enable_write},
			{"enable_vlan", handle_enable_vlan_read, handle_enable_vlan_write},
			{"reset", NULL, handle_reset},
			{NULL, NULL, NULL}
		};

		switch_config port[] = {
			{"enable", handle_port_enable_read, handle_port_enable_write},
			{"media", handle_port_media_read, handle_port_media_write},
			{"bandwidth", handle_port_bandwidth_read, handle_port_bandwidth_write},
			{"prio-enable", handle_port_prio_enable_read, handle_port_prio_enable_write},
			{"prio", handle_port_prio_read, handle_port_prio_write},
			{"flow", handle_port_flow_enable_read, handle_port_flow_enable_write},
			{NULL, NULL, NULL}
		};

		switch_config vlan[] = {
			{"ports", handle_vlan_port_read, handle_vlan_port_write},
			{NULL, NULL, NULL}
		};
		switch_driver driver = {
			name: DRIVER_NAME,
			version: DRIVER_VERSION,
			interface: device,
			cpuport: 5,
			ports: 6,
			vlans: 16,
			driver_handlers: cfg,
			port_handlers: port,
			vlan_handlers: vlan,
		};

		return switch_register_driver(&driver);
	}
}

static void __exit robo_exit(void)
{
	switch_unregister_driver(DRIVER_NAME);
}


MODULE_AUTHOR("Felix Fietkau <openwrt@nbd.name>");
MODULE_LICENSE("GPL");

module_init(robo_init);
module_exit(robo_exit);
