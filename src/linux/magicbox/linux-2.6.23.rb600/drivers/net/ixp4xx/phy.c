/*
 * phy.c - MDIO functions and mii initialisation
 *
 * Copyright (C) 2006 Christian Hohnstaedt <chohnstaedt@innominate.com>
 *
 * This file is released under the GPLv2
 */


#include <linux/mutex.h>
#include "mac.h"

#define MAX_PHYS (1<<5)

/*
 * We must always use the same MAC for acessing the MDIO
 * We may not use each MAC for its PHY :-(
 */

static struct net_device *phy_dev = NULL;
static struct mutex mtx;

/* here we remember if the PHY is alive, to avoid log dumping */
static int phy_works[MAX_PHYS];



static int mdio_read_register_dir(struct net_device *dev, int phy_addr, int phy_reg)
{
	struct mac_info *mac;
	u32 cmd, reg;
	int cnt = 0;

	if (!phy_dev)
		return 0;

	mac = netdev_priv(phy_dev);
	cmd = mdio_cmd(phy_addr, phy_reg);
	mutex_lock_interruptible(&mtx);
	mac_mdio_cmd_write(mac, cmd);
	while((cmd = mac_mdio_cmd_read(mac)) & MII_GO) {
		if (++cnt >= 100) {
			printk("%s: PHY[%d] access failed\n",
				dev->name, phy_addr);
			break;
		}
		schedule();
	}
	reg = mac_mdio_status_read(mac);
	mutex_unlock(&mtx);
	if (reg & MII_READ_FAIL) {
		if (phy_works[phy_addr]) {
			printk("%s: PHY[%d] unresponsive\n",
					dev->name, phy_addr);
		}
		reg = 0;
		phy_works[phy_addr] = 0;
	} else {
		if ( !phy_works[phy_addr]) {
			printk("%s: PHY[%d] responsive again\n",
				dev->name, phy_addr);
		}
		phy_works[phy_addr] = 1;
	}
	return reg & 0xffff;
}
int mdio_read_register(struct net_device *dev, int phy_addr, int phy_reg)
{
	if (!phy_dev)
		return 0;
	struct mac_info *mac;
	mac = netdev_priv(phy_dev);
	if (phy_reg==MII_BMSR && mac->iskendin)
	{
	    return BMSR_LSTATUS|BMSR_100FULL|BMSR_ANEGCAPABLE;	 
	}
	else 
	    return mdio_read_register_dir(dev,phy_addr,phy_reg);
}

void
mdio_write_register(struct net_device *dev, int phy_addr, int phy_reg, int val)
{
	struct mac_info *mac;
	u32 cmd;
	int cnt=0;

	if (!phy_dev)
		return;

	mac = netdev_priv(phy_dev);
	cmd = mdio_cmd(phy_addr, phy_reg) | MII_WRITE | val;

	mutex_lock_interruptible(&mtx);
	mac_mdio_cmd_write(mac, cmd);
	while((cmd = mac_mdio_cmd_read(mac)) & MII_GO) {
		if (++cnt >= 100) {
			printk("%s: PHY[%d] access failed\n",
					dev->name, phy_addr);
			break;
		}
		schedule();
	}
	mutex_unlock(&mtx);
}

void init_mdio(struct net_device *dev, int phy_id,int kendin)
{
	struct mac_info *mac = netdev_priv(dev);
	int i;

	/* All phy operations should use the same MAC
	 * (my experience)
	 */
	if (mac->plat->eth_id == 0) {
		mutex_init(&mtx);
		phy_dev = dev;
		for (i=0; i<MAX_PHYS; i++)
			phy_works[i] = 1;
	}
	mac->mii.dev = dev;
	mac->mii.phy_id = phy_id;
	mac->mii.phy_id_mask = MAX_PHYS - 1;
	mac->mii.reg_num_mask = 0x1f;
	mac->mii.mdio_read = mdio_read_register;
	mac->mii.mdio_write = mdio_write_register;
	mac->iskendin = kendin;
}

