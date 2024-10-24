/*
 * ar8216.c: AR8216 switch driver
 *
 * Copyright (C) 2009 Felix Fietkau <nbd@openwrt.org>
 * Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
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
 */

#include <linux/if.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <linux/bitops.h>
#include <net/genetlink.h>
#include <linux/switch.h>
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/lockdep.h>
#include <linux/ar8216_platform.h>
#include "ar8216.h"

/* size of the vlan table */
#define AR8X16_MAX_VLANS	128
#define AR8X16_PROBE_RETRIES	10
#define AR8X16_MAX_PORTS	8

struct ar8216_priv;

#define AR8XXX_CAP_GIGE		BIT(0)

struct ar8xxx_chip {
	unsigned long caps;

	int (*hw_init)(struct ar8216_priv *priv);
	void (*init_globals)(struct ar8216_priv *priv);
	void (*init_port)(struct ar8216_priv *priv, int port);
	void (*setup_port)(struct ar8216_priv *priv, int port, u32 egress,
			   u32 ingress, u32 members, u32 pvid);
	u32 (*read_port_status)(struct ar8216_priv *priv, int port);
	int (*atu_flush)(struct ar8216_priv *priv);
	void (*vtu_flush)(struct ar8216_priv *priv);
	void (*vtu_load_vlan)(struct ar8216_priv *priv, u32 vid, u32 port_mask);
};

struct ar8216_priv {
	struct switch_dev dev;
	struct phy_device *phy;
	u32 (*read)(struct ar8216_priv *priv, int reg);
	void (*write)(struct ar8216_priv *priv, int reg, u32 val);
	const struct net_device_ops *ndo_old;
	struct net_device_ops ndo;
	struct mutex reg_mutex;
	int chip_type;
	const struct ar8xxx_chip *chip;
	bool initialized;
	bool port4_phy;
	char buf[80];

	bool init;
	bool mii_lo_first;

	/* all fields below are cleared on reset */
	bool vlan;
	u16 vlan_id[AR8X16_MAX_VLANS];
	u8 vlan_table[AR8X16_MAX_VLANS];
	u8 vlan_tagged;
	u16 pvid[AR8X16_MAX_PORTS];
};

#define to_ar8216(_dev) container_of(_dev, struct ar8216_priv, dev)

static inline bool ar8xxx_has_gige(struct ar8216_priv *priv)
{
	return priv->chip->caps & AR8XXX_CAP_GIGE;
}

static inline void
split_addr(u32 regaddr, u16 *r1, u16 *r2, u16 *page)
{
	regaddr >>= 1;
	*r1 = regaddr & 0x1e;

	regaddr >>= 5;
	*r2 = regaddr & 0x7;

	regaddr >>= 3;
	*page = regaddr & 0x1ff;
}

static u32
ar8216_mii_read(struct ar8216_priv *priv, int reg)
{
	struct phy_device *phy = priv->phy;
	struct mii_bus *bus = phy->bus;
	u16 r1, r2, page;
	u16 lo, hi;

	split_addr((u32) reg, &r1, &r2, &page);

	mutex_lock(&bus->mdio_lock);

	bus->write(bus, 0x18, 0, page);
	usleep_range(1000, 2000); /* wait for the page switch to propagate */
	lo = bus->read(bus, 0x10 | r2, r1);
	hi = bus->read(bus, 0x10 | r2, r1 + 1);

	mutex_unlock(&bus->mdio_lock);

	return (hi << 16) | lo;
}

static void
ar8216_mii_write(struct ar8216_priv *priv, int reg, u32 val)
{
	struct phy_device *phy = priv->phy;
	struct mii_bus *bus = phy->bus;
	u16 r1, r2, r3;
	u16 lo, hi;

	split_addr((u32) reg, &r1, &r2, &r3);
	lo = val & 0xffff;
	hi = (u16) (val >> 16);

	mutex_lock(&bus->mdio_lock);

	bus->write(bus, 0x18, 0, r3);
	usleep_range(1000, 2000); /* wait for the page switch to propagate */
	if (priv->mii_lo_first) {
		bus->write(bus, 0x10 | r2, r1, lo);
		bus->write(bus, 0x10 | r2, r1 + 1, hi);
	} else {
		bus->write(bus, 0x10 | r2, r1 + 1, hi);
		bus->write(bus, 0x10 | r2, r1, lo);
	}

	mutex_unlock(&bus->mdio_lock);
}

static void
ar8216_phy_dbg_write(struct ar8216_priv *priv, int phy_addr,
		     u16 dbg_addr, u16 dbg_data)
{
	struct mii_bus *bus = priv->phy->bus;

	mutex_lock(&bus->mdio_lock);
	bus->write(bus, phy_addr, MII_ATH_DBG_ADDR, dbg_addr);
	bus->write(bus, phy_addr, MII_ATH_DBG_DATA, dbg_data);
	mutex_unlock(&bus->mdio_lock);
}

static u32
ar8216_rmw(struct ar8216_priv *priv, int reg, u32 mask, u32 val)
{
	u32 v;

	lockdep_assert_held(&priv->reg_mutex);

	v = priv->read(priv, reg);
	v &= ~mask;
	v |= val;
	priv->write(priv, reg, v);

	return v;
}

static void
ar8216_read_port_link(struct ar8216_priv *priv, int port,
		      struct switch_port_link *link)
{
	u32 status;
	u32 speed;

	memset(link, '\0', sizeof(*link));

	status = priv->chip->read_port_status(priv, port);

	link->aneg = !!(status & AR8216_PORT_STATUS_LINK_AUTO);
	if (link->aneg) {
		link->link = !!(status & AR8216_PORT_STATUS_LINK_UP);
		if (!link->link)
			return;
	} else {
		link->link = true;
	}

	link->duplex = !!(status & AR8216_PORT_STATUS_DUPLEX);
	link->tx_flow = !!(status & AR8216_PORT_STATUS_TXFLOW);
	link->rx_flow = !!(status & AR8216_PORT_STATUS_RXFLOW);

	speed = (status & AR8216_PORT_STATUS_SPEED) >>
		 AR8216_PORT_STATUS_SPEED_S;

	switch (speed) {
	case AR8216_PORT_SPEED_10M:
		link->speed = SWITCH_PORT_SPEED_10;
		break;
	case AR8216_PORT_SPEED_100M:
		link->speed = SWITCH_PORT_SPEED_100;
		break;
	case AR8216_PORT_SPEED_1000M:
		link->speed = SWITCH_PORT_SPEED_1000;
		break;
	default:
		link->speed = SWITCH_PORT_SPEED_UNKNOWN;
		break;
	}
}

static int
ar8216_mangle_tx(struct sk_buff *skb, struct net_device *dev)
{
	struct ar8216_priv *priv = dev->phy_ptr;
	unsigned char *buf;

	if (unlikely(!priv))
		goto error;

	if (!priv->vlan)
		goto send;

	if (unlikely(skb_headroom(skb) < 2)) {
		if (pskb_expand_head(skb, 2, 0, GFP_ATOMIC) < 0)
			goto error;
	}

	buf = skb_push(skb, 2);
	buf[0] = 0x10;
	buf[1] = 0x80;

send:
	return priv->ndo_old->ndo_start_xmit(skb, dev);

error:
	dev_kfree_skb_any(skb);
	return 0;
}

static int
ar8216_mangle_rx(struct sk_buff *skb, int napi)
{
	struct ar8216_priv *priv;
	struct net_device *dev;
	unsigned char *buf;
	int port, vlan;

	dev = skb->dev;
	if (!dev)
		goto error;

	priv = dev->phy_ptr;
	if (!priv)
		goto error;

	/* don't strip the header if vlan mode is disabled */
	if (!priv->vlan)
		goto recv;

	/* strip header, get vlan id */
	buf = skb->data;
	skb_pull(skb, 2);

	/* check for vlan header presence */
	if ((buf[12 + 2] != 0x81) || (buf[13 + 2] != 0x00))
		goto recv;

	port = buf[0] & 0xf;

	/* no need to fix up packets coming from a tagged source */
	if (priv->vlan_tagged & (1 << port))
		goto recv;

	/* lookup port vid from local table, the switch passes an invalid vlan id */
	vlan = priv->vlan_id[priv->pvid[port]];

	buf[14 + 2] &= 0xf0;
	buf[14 + 2] |= vlan >> 8;
	buf[15 + 2] = vlan & 0xff;

recv:
	skb->protocol = eth_type_trans(skb, skb->dev);

	if (napi)
		return netif_receive_skb(skb);
	else
		return netif_rx(skb);

error:
	/* no vlan? eat the packet! */
	dev_kfree_skb_any(skb);
	return NET_RX_DROP;
}

static int
ar8216_netif_rx(struct sk_buff *skb)
{
	return ar8216_mangle_rx(skb, 0);
}

static int
ar8216_netif_receive_skb(struct sk_buff *skb)
{
	return ar8216_mangle_rx(skb, 1);
}

static int
ar8216_wait_bit(struct ar8216_priv *priv, int reg, u32 mask, u32 val)
{
	int timeout = 20;
	u32 t = 0;

	while (1) {
		t = priv->read(priv, reg);
		if ((t & mask) == val)
			return 0;

		if (timeout-- <= 0)
			break;

		udelay(10);
	}

	pr_err("ar8216: timeout on reg %08x: %08x & %08x != %08x\n",
	       (unsigned int) reg, t, mask, val);
	return -ETIMEDOUT;
}

static void
ar8216_vtu_op(struct ar8216_priv *priv, u32 op, u32 val)
{
	if (ar8216_wait_bit(priv, AR8216_REG_VTU, AR8216_VTU_ACTIVE, 0))
		return;
	if ((op & AR8216_VTU_OP) == AR8216_VTU_OP_LOAD) {
		val &= AR8216_VTUDATA_MEMBER;
		val |= AR8216_VTUDATA_VALID;
		priv->write(priv, AR8216_REG_VTU_DATA, val);
	}
	op |= AR8216_VTU_ACTIVE;
	priv->write(priv, AR8216_REG_VTU, op);
}

static void
ar8216_vtu_flush(struct ar8216_priv *priv)
{
	ar8216_vtu_op(priv, AR8216_VTU_OP_FLUSH, 0);
}

static void
ar8216_vtu_load_vlan(struct ar8216_priv *priv, u32 vid, u32 port_mask)
{
	u32 op;

	op = AR8216_VTU_OP_LOAD | (vid << AR8216_VTU_VID_S);
	ar8216_vtu_op(priv, op, port_mask);
}

static int
ar8216_atu_flush(struct ar8216_priv *priv)
{
	int ret;

	ret = ar8216_wait_bit(priv, AR8216_REG_ATU, AR8216_ATU_ACTIVE, 0);
	if (!ret)
		priv->write(priv, AR8216_REG_ATU, AR8216_ATU_OP_FLUSH);

	return ret;
}

static u32
ar8216_read_port_status(struct ar8216_priv *priv, int port)
{
	return priv->read(priv, AR8216_REG_PORT_STATUS(port));
}

static void
ar8216_setup_port(struct ar8216_priv *priv, int port, u32 egress, u32 ingress,
		  u32 members, u32 pvid)
{
	u32 header;

	if (priv->vlan && port == AR8216_PORT_CPU && priv->chip_type == AR8216)
		header = AR8216_PORT_CTRL_HEADER;
	else
		header = 0;

	ar8216_rmw(priv, AR8216_REG_PORT_CTRL(port),
		   AR8216_PORT_CTRL_LEARN | AR8216_PORT_CTRL_VLAN_MODE |
		   AR8216_PORT_CTRL_SINGLE_VLAN | AR8216_PORT_CTRL_STATE |
		   AR8216_PORT_CTRL_HEADER | AR8216_PORT_CTRL_LEARN_LOCK,
		   AR8216_PORT_CTRL_LEARN | header |
		   (egress << AR8216_PORT_CTRL_VLAN_MODE_S) |
		   (AR8216_PORT_STATE_FORWARD << AR8216_PORT_CTRL_STATE_S));

	ar8216_rmw(priv, AR8216_REG_PORT_VLAN(port),
		   AR8216_PORT_VLAN_DEST_PORTS | AR8216_PORT_VLAN_MODE |
		   AR8216_PORT_VLAN_DEFAULT_ID,
		   (members << AR8216_PORT_VLAN_DEST_PORTS_S) |
		   (ingress << AR8216_PORT_VLAN_MODE_S) |
		   (pvid << AR8216_PORT_VLAN_DEFAULT_ID_S));
}

static int
ar8216_hw_init(struct ar8216_priv *priv)
{
	return 0;
}

static void
ar8216_init_globals(struct ar8216_priv *priv)
{
	/* standard atheros magic */
	priv->write(priv, 0x38, 0xc000050e);

	ar8216_rmw(priv, AR8216_REG_GLOBAL_CTRL,
		   AR8216_GCTRL_MTU, 1518 + 8 + 2);
}

static void
ar8216_init_port(struct ar8216_priv *priv, int port)
{
	/* Enable port learning and tx */
	priv->write(priv, AR8216_REG_PORT_CTRL(port),
		AR8216_PORT_CTRL_LEARN |
		(4 << AR8216_PORT_CTRL_STATE_S));

	priv->write(priv, AR8216_REG_PORT_VLAN(port), 0);

	if (port == AR8216_PORT_CPU) {
		priv->write(priv, AR8216_REG_PORT_STATUS(port),
			AR8216_PORT_STATUS_LINK_UP |
			(ar8xxx_has_gige(priv) ? AR8216_PORT_SPEED_1000M :
						AR8216_PORT_SPEED_100M) |
			AR8216_PORT_STATUS_TXMAC |
			AR8216_PORT_STATUS_RXMAC |
			((priv->chip_type == AR8316) ? AR8216_PORT_STATUS_RXFLOW : 0) |
			((priv->chip_type == AR8316) ? AR8216_PORT_STATUS_TXFLOW : 0) |
			AR8216_PORT_STATUS_DUPLEX);
	} else {
		priv->write(priv, AR8216_REG_PORT_STATUS(port),
			AR8216_PORT_STATUS_LINK_AUTO);
	}
}

static const struct ar8xxx_chip ar8216_chip = {
	.hw_init = ar8216_hw_init,
	.init_globals = ar8216_init_globals,
	.init_port = ar8216_init_port,
	.setup_port = ar8216_setup_port,
	.read_port_status = ar8216_read_port_status,
	.atu_flush = ar8216_atu_flush,
	.vtu_flush = ar8216_vtu_flush,
	.vtu_load_vlan = ar8216_vtu_load_vlan,
};

static void
ar8236_setup_port(struct ar8216_priv *priv, int port, u32 egress, u32 ingress,
		  u32 members, u32 pvid)
{
	ar8216_rmw(priv, AR8216_REG_PORT_CTRL(port),
		   AR8216_PORT_CTRL_LEARN | AR8216_PORT_CTRL_VLAN_MODE |
		   AR8216_PORT_CTRL_SINGLE_VLAN | AR8216_PORT_CTRL_STATE |
		   AR8216_PORT_CTRL_HEADER | AR8216_PORT_CTRL_LEARN_LOCK,
		   AR8216_PORT_CTRL_LEARN |
		   (egress << AR8216_PORT_CTRL_VLAN_MODE_S) |
		   (AR8216_PORT_STATE_FORWARD << AR8216_PORT_CTRL_STATE_S));

	ar8216_rmw(priv, AR8236_REG_PORT_VLAN(port),
		   AR8236_PORT_VLAN_DEFAULT_ID,
		   (pvid << AR8236_PORT_VLAN_DEFAULT_ID_S));

	ar8216_rmw(priv, AR8236_REG_PORT_VLAN2(port),
		   AR8236_PORT_VLAN2_VLAN_MODE |
		   AR8236_PORT_VLAN2_MEMBER,
		   (ingress << AR8236_PORT_VLAN2_VLAN_MODE_S) |
		   (members << AR8236_PORT_VLAN2_MEMBER_S));
}

static int
ar8236_hw_init(struct ar8216_priv *priv)
{
	int i;
	struct mii_bus *bus;

	if (priv->initialized)
		return 0;

	/* Initialize the PHYs */
	bus = priv->phy->bus;
	for (i = 0; i < 5; i++) {
		mdiobus_write(bus, i, MII_ADVERTISE,
			      ADVERTISE_ALL | ADVERTISE_PAUSE_CAP |
			      ADVERTISE_PAUSE_ASYM);
		mdiobus_write(bus, i, MII_BMCR, BMCR_RESET | BMCR_ANENABLE);
	}
	msleep(1000);

	priv->initialized = true;
	return 0;
}

static void
ar8236_init_globals(struct ar8216_priv *priv)
{
	/* enable jumbo frames */
	ar8216_rmw(priv, AR8216_REG_GLOBAL_CTRL,
		   AR8316_GCTRL_MTU, 9018 + 8 + 2);
}

static const struct ar8xxx_chip ar8236_chip = {
	.hw_init = ar8236_hw_init,
	.init_globals = ar8236_init_globals,
	.init_port = ar8216_init_port,
	.setup_port = ar8236_setup_port,
	.read_port_status = ar8216_read_port_status,
	.atu_flush = ar8216_atu_flush,
	.vtu_flush = ar8216_vtu_flush,
	.vtu_load_vlan = ar8216_vtu_load_vlan,
};

static int
ar8316_hw_init(struct ar8216_priv *priv)
{
	int i;
	u32 val, newval;
	struct mii_bus *bus;

	val = priv->read(priv, 0x8);

	if (priv->phy->interface == PHY_INTERFACE_MODE_RGMII) {
		if (priv->port4_phy) {
			/* value taken from Ubiquiti RouterStation Pro */
			newval = 0x81461bea;
			printk(KERN_INFO "ar8316: Using port 4 as PHY\n");
		} else {
			newval = 0x01261be2;
			printk(KERN_INFO "ar8316: Using port 4 as switch port\n");
		}
	} else if (priv->phy->interface == PHY_INTERFACE_MODE_GMII) {
		/* value taken from AVM Fritz!Box 7390 sources */
		newval = 0x010e5b71;
	} else {
		/* no known value for phy interface */
		printk(KERN_ERR "ar8316: unsupported mii mode: %d.\n",
			priv->phy->interface);
		return -EINVAL;
	}

	if (val == newval)
		goto out;

	priv->write(priv, 0x8, newval);

	/* Initialize the ports */
	bus = priv->phy->bus;
	for (i = 0; i < 5; i++) {
		if ((i == 4) && priv->port4_phy &&
		    priv->phy->interface == PHY_INTERFACE_MODE_RGMII) {
			/* work around for phy4 rgmii mode */
			ar8216_phy_dbg_write(priv, i, 0x12, 0x480c);
			/* rx delay */
			ar8216_phy_dbg_write(priv, i, 0x0, 0x824e);
			/* tx delay */
			ar8216_phy_dbg_write(priv, i, 0x5, 0x3d47);
			msleep(1000);
		}

		/* initialize the port itself */
		mdiobus_write(bus, i, MII_ADVERTISE,
			ADVERTISE_ALL | ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
		mdiobus_write(bus, i, MII_CTRL1000, ADVERTISE_1000FULL);
		mdiobus_write(bus, i, MII_BMCR, BMCR_RESET | BMCR_ANENABLE);
		msleep(1000);
	}

out:
	priv->initialized = true;
	return 0;
}


static void
ar8316_init_globals(struct ar8216_priv *priv)
{
	/* standard atheros magic */
	priv->write(priv, 0x38, 0xc000050e);

	/* enable cpu port to receive multicast and broadcast frames */
	priv->write(priv, AR8216_REG_FLOOD_MASK, 0x003f003f);

	/* enable jumbo frames */
	ar8216_rmw(priv, AR8216_REG_GLOBAL_CTRL,
		   AR8316_GCTRL_MTU, 9018 + 8 + 2);
}

static const struct ar8xxx_chip ar8316_chip = {
	.caps = AR8XXX_CAP_GIGE,
	.hw_init = ar8316_hw_init,
	.init_globals = ar8316_init_globals,
	.init_port = ar8216_init_port,
	.setup_port = ar8216_setup_port,
	.read_port_status = ar8216_read_port_status,
	.atu_flush = ar8216_atu_flush,
	.vtu_flush = ar8216_vtu_flush,
	.vtu_load_vlan = ar8216_vtu_load_vlan,
};

static u32
ar8327_get_pad_cfg(struct ar8327_pad_cfg *cfg)
{
	u32 t;

	if (!cfg)
		return 0;

	t = 0;
	switch (cfg->mode) {
	case AR8327_PAD_NC:
		break;

	case AR8327_PAD_MAC2MAC_MII:
		t = AR8327_PAD_MAC_MII_EN;
		if (cfg->rxclk_sel)
			t |= AR8327_PAD_MAC_MII_RXCLK_SEL;
		if (cfg->txclk_sel)
			t |= AR8327_PAD_MAC_MII_TXCLK_SEL;
		break;

	case AR8327_PAD_MAC2MAC_GMII:
		t = AR8327_PAD_MAC_GMII_EN;
		if (cfg->rxclk_sel)
			t |= AR8327_PAD_MAC_GMII_RXCLK_SEL;
		if (cfg->txclk_sel)
			t |= AR8327_PAD_MAC_GMII_TXCLK_SEL;
		break;

	case AR8327_PAD_MAC_SGMII:
		t = AR8327_PAD_SGMII_EN;
		break;

	case AR8327_PAD_MAC2PHY_MII:
		t = AR8327_PAD_PHY_MII_EN;
		if (cfg->rxclk_sel)
			t |= AR8327_PAD_PHY_MII_RXCLK_SEL;
		if (cfg->txclk_sel)
			t |= AR8327_PAD_PHY_MII_TXCLK_SEL;
		break;

	case AR8327_PAD_MAC2PHY_GMII:
		t = AR8327_PAD_PHY_GMII_EN;
		if (cfg->pipe_rxclk_sel)
			t |= AR8327_PAD_PHY_GMII_PIPE_RXCLK_SEL;
		if (cfg->rxclk_sel)
			t |= AR8327_PAD_PHY_GMII_RXCLK_SEL;
		if (cfg->txclk_sel)
			t |= AR8327_PAD_PHY_GMII_TXCLK_SEL;
		break;

	case AR8327_PAD_MAC_RGMII:
		t = AR8327_PAD_RGMII_EN;
		t |= cfg->txclk_delay_sel << AR8327_PAD_RGMII_TXCLK_DELAY_SEL_S;
		t |= cfg->rxclk_delay_sel << AR8327_PAD_RGMII_RXCLK_DELAY_SEL_S;
		if (cfg->rxclk_delay_en)
			t |= AR8327_PAD_RGMII_RXCLK_DELAY_EN;
		if (cfg->txclk_delay_en)
			t |= AR8327_PAD_RGMII_TXCLK_DELAY_EN;
		break;

	case AR8327_PAD_PHY_GMII:
		t = AR8327_PAD_PHYX_GMII_EN;
		break;

	case AR8327_PAD_PHY_RGMII:
		t = AR8327_PAD_PHYX_RGMII_EN;
		break;

	case AR8327_PAD_PHY_MII:
		t = AR8327_PAD_PHYX_MII_EN;
		break;
	}

	return t;
}

static int
ar8327_hw_init(struct ar8216_priv *priv)
{
	struct ar8327_platform_data *pdata;
	u32 t;
	int i;

	pdata = priv->phy->dev.platform_data;
	if (!pdata)
		return -EINVAL;

	t = ar8327_get_pad_cfg(pdata->pad0_cfg);
	priv->write(priv, AR8327_REG_PAD0_MODE, t);
	t = ar8327_get_pad_cfg(pdata->pad5_cfg);
	priv->write(priv, AR8327_REG_PAD5_MODE, t);
	t = ar8327_get_pad_cfg(pdata->pad6_cfg);
	priv->write(priv, AR8327_REG_PAD6_MODE, t);

	priv->write(priv, AR8327_REG_POWER_ON_STRIP, 0x40000000);

	/* fixup PHYs */
	for (i = 0; i < AR8327_NUM_PHYS; i++) {
		/* For 100M waveform */
		ar8216_phy_dbg_write(priv, i, 0, 0x02ea);

		/* Turn on Gigabit clock */
		ar8216_phy_dbg_write(priv, i, 0x3d, 0x68a0);
	}

	return 0;
}

static void
ar8327_init_globals(struct ar8216_priv *priv)
{
	u32 t;

	/* enable CPU port and disable mirror port */
	t = AR8327_FWD_CTRL0_CPU_PORT_EN |
	    AR8327_FWD_CTRL0_MIRROR_PORT;
	priv->write(priv, AR8327_REG_FWD_CTRL0, t);

	/* forward multicast and broadcast frames to CPU */
	t = (AR8327_PORTS_ALL << AR8327_FWD_CTRL1_UC_FLOOD_S) |
	    (AR8327_PORTS_ALL << AR8327_FWD_CTRL1_MC_FLOOD_S) |
	    (AR8327_PORTS_ALL << AR8327_FWD_CTRL1_BC_FLOOD_S);
	priv->write(priv, AR8327_REG_FWD_CTRL1, t);

	/* setup MTU */
	ar8216_rmw(priv, AR8327_REG_MAX_FRAME_SIZE,
		   AR8327_MAX_FRAME_SIZE_MTU, 1518 + 8 + 2);

	/* Disable EEE on all ports due to stability issues */
	t = priv->read(priv, AR8327_REG_EEE_CTRL);
	t |= AR8327_EEE_CTRL_DISABLE_PHY(0) |
	     AR8327_EEE_CTRL_DISABLE_PHY(1) |
	     AR8327_EEE_CTRL_DISABLE_PHY(2) |
	     AR8327_EEE_CTRL_DISABLE_PHY(3) |
	     AR8327_EEE_CTRL_DISABLE_PHY(4);
	priv->write(priv, AR8327_REG_EEE_CTRL, t);
}

static void
ar8327_init_cpuport(struct ar8216_priv *priv)
{
	struct ar8327_platform_data *pdata;
	struct ar8327_port_cfg *cfg;
	u32 t;

	pdata = priv->phy->dev.platform_data;
	if (!pdata)
		return;

	cfg = &pdata->cpuport_cfg;
	if (!cfg->force_link) {
		priv->write(priv, AR8327_REG_PORT_STATUS(AR8216_PORT_CPU),
			    AR8216_PORT_STATUS_LINK_AUTO);
		return;
	}

	t = AR8216_PORT_STATUS_TXMAC | AR8216_PORT_STATUS_RXMAC;
	t |= cfg->duplex ? AR8216_PORT_STATUS_DUPLEX : 0;
	t |= cfg->rxpause ? AR8216_PORT_STATUS_RXFLOW : 0;
	t |= cfg->txpause ? AR8216_PORT_STATUS_TXFLOW : 0;
	switch (cfg->speed) {
	case AR8327_PORT_SPEED_10:
		t |= AR8216_PORT_SPEED_10M;
		break;
	case AR8327_PORT_SPEED_100:
		t |= AR8216_PORT_SPEED_100M;
		break;
	case AR8327_PORT_SPEED_1000:
		t |= AR8216_PORT_SPEED_1000M;
		break;
	}

	priv->write(priv, AR8327_REG_PORT_STATUS(AR8216_PORT_CPU), t);
}

static void
ar8327_init_port(struct ar8216_priv *priv, int port)
{
	u32 t;

	if (port == AR8216_PORT_CPU) {
		ar8327_init_cpuport(priv);
	} else {
		t = AR8216_PORT_STATUS_LINK_AUTO;
		priv->write(priv, AR8327_REG_PORT_STATUS(port), t);
	}

	priv->write(priv, AR8327_REG_PORT_HEADER(port), 0);

	priv->write(priv, AR8327_REG_PORT_VLAN0(port), 0);

	t = AR8327_PORT_VLAN1_OUT_MODE_UNTOUCH << AR8327_PORT_VLAN1_OUT_MODE_S;
	priv->write(priv, AR8327_REG_PORT_VLAN1(port), t);

	t = AR8327_PORT_LOOKUP_LEARN;
	t |= AR8216_PORT_STATE_FORWARD << AR8327_PORT_LOOKUP_STATE_S;
	priv->write(priv, AR8327_REG_PORT_LOOKUP(port), t);
}

static u32
ar8327_read_port_status(struct ar8216_priv *priv, int port)
{
	return priv->read(priv, AR8327_REG_PORT_STATUS(port));
}

static int
ar8327_atu_flush(struct ar8216_priv *priv)
{
	int ret;

	ret = ar8216_wait_bit(priv, AR8327_REG_ATU_FUNC,
			      AR8327_ATU_FUNC_BUSY, 0);
	if (!ret)
		priv->write(priv, AR8327_REG_ATU_FUNC,
			    AR8327_ATU_FUNC_OP_FLUSH);

	return ret;
}

static void
ar8327_vtu_op(struct ar8216_priv *priv, u32 op, u32 val)
{
	if (ar8216_wait_bit(priv, AR8327_REG_VTU_FUNC1,
			    AR8327_VTU_FUNC1_BUSY, 0))
		return;

	if ((op & AR8327_VTU_FUNC1_OP) == AR8327_VTU_FUNC1_OP_LOAD)
		priv->write(priv, AR8327_REG_VTU_FUNC0, val);

	op |= AR8327_VTU_FUNC1_BUSY;
	priv->write(priv, AR8327_REG_VTU_FUNC1, op);
}

static void
ar8327_vtu_flush(struct ar8216_priv *priv)
{
	ar8327_vtu_op(priv, AR8327_VTU_FUNC1_OP_FLUSH, 0);
}

static void
ar8327_vtu_load_vlan(struct ar8216_priv *priv, u32 vid, u32 port_mask)
{
	u32 op;
	u32 val;
	int i;

	op = AR8327_VTU_FUNC1_OP_LOAD | (vid << AR8327_VTU_FUNC1_VID_S);
	val = AR8327_VTU_FUNC0_VALID | AR8327_VTU_FUNC0_IVL;
	for (i = 0; i < AR8327_NUM_PORTS; i++) {
		u32 mode;

		if ((port_mask & BIT(i)) == 0)
			mode = AR8327_VTU_FUNC0_EG_MODE_NOT;
		else if (priv->vlan == 0)
			mode = AR8327_VTU_FUNC0_EG_MODE_KEEP;
		else if (priv->vlan_tagged & BIT(i))
			mode = AR8327_VTU_FUNC0_EG_MODE_TAG;
		else
			mode = AR8327_VTU_FUNC0_EG_MODE_UNTAG;

		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
	}
	ar8327_vtu_op(priv, op, val);
}

static void
ar8327_setup_port(struct ar8216_priv *priv, int port, u32 egress, u32 ingress,
		  u32 members, u32 pvid)
{
	u32 t;
	u32 mode;

	t = pvid << AR8327_PORT_VLAN0_DEF_SVID_S;
	t |= pvid << AR8327_PORT_VLAN0_DEF_CVID_S;
	priv->write(priv, AR8327_REG_PORT_VLAN0(port), t);

	mode = AR8327_PORT_VLAN1_OUT_MODE_UNMOD;
	switch (egress) {
	case AR8216_OUT_KEEP:
		mode = AR8327_PORT_VLAN1_OUT_MODE_UNTOUCH;
		break;
	case AR8216_OUT_STRIP_VLAN:
		mode = AR8327_PORT_VLAN1_OUT_MODE_UNTAG;
		break;
	case AR8216_OUT_ADD_VLAN:
		mode = AR8327_PORT_VLAN1_OUT_MODE_TAG;
		break;
	}

	t = AR8327_PORT_VLAN1_PORT_VLAN_PROP;
	t |= mode << AR8327_PORT_VLAN1_OUT_MODE_S;
	priv->write(priv, AR8327_REG_PORT_VLAN1(port), t);

	t = members;
	t |= AR8327_PORT_LOOKUP_LEARN;
	t |= ingress << AR8327_PORT_LOOKUP_IN_MODE_S;
	t |= AR8216_PORT_STATE_FORWARD << AR8327_PORT_LOOKUP_STATE_S;
	priv->write(priv, AR8327_REG_PORT_LOOKUP(port), t);
}

static const struct ar8xxx_chip ar8327_chip = {
	.caps = AR8XXX_CAP_GIGE,
	.hw_init = ar8327_hw_init,
	.init_globals = ar8327_init_globals,
	.init_port = ar8327_init_port,
	.setup_port = ar8327_setup_port,
	.read_port_status = ar8327_read_port_status,
	.atu_flush = ar8327_atu_flush,
	.vtu_flush = ar8327_vtu_flush,
	.vtu_load_vlan = ar8327_vtu_load_vlan,
};

static int
ar8216_sw_set_vlan(struct switch_dev *dev, const struct switch_attr *attr,
		   struct switch_val *val)
{
	struct ar8216_priv *priv = to_ar8216(dev);
	priv->vlan = !!val->value.i;
	return 0;
}

static int
ar8216_sw_get_vlan(struct switch_dev *dev, const struct switch_attr *attr,
		   struct switch_val *val)
{
	struct ar8216_priv *priv = to_ar8216(dev);
	val->value.i = priv->vlan;
	return 0;
}


static int
ar8216_sw_set_pvid(struct switch_dev *dev, int port, int vlan)
{
	struct ar8216_priv *priv = to_ar8216(dev);

	/* make sure no invalid PVIDs get set */

	if (vlan >= dev->vlans)
		return -EINVAL;

	priv->pvid[port] = vlan;
	return 0;
}

static int
ar8216_sw_get_pvid(struct switch_dev *dev, int port, int *vlan)
{
	struct ar8216_priv *priv = to_ar8216(dev);
	*vlan = priv->pvid[port];
	return 0;
}

static int
ar8216_sw_set_vid(struct switch_dev *dev, const struct switch_attr *attr,
		  struct switch_val *val)
{
	struct ar8216_priv *priv = to_ar8216(dev);
	priv->vlan_id[val->port_vlan] = val->value.i;
	return 0;
}

static int
ar8216_sw_get_vid(struct switch_dev *dev, const struct switch_attr *attr,
		  struct switch_val *val)
{
	struct ar8216_priv *priv = to_ar8216(dev);
	val->value.i = priv->vlan_id[val->port_vlan];
	return 0;
}

static int
ar8216_sw_get_port_link(struct switch_dev *dev, int port,
			struct switch_port_link *link)
{
	struct ar8216_priv *priv = to_ar8216(dev);

	ar8216_read_port_link(priv, port, link);
	return 0;
}

static int
ar8216_sw_get_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct ar8216_priv *priv = to_ar8216(dev);
	u8 ports = priv->vlan_table[val->port_vlan];
	int i;

	val->len = 0;
	for (i = 0; i < dev->ports; i++) {
		struct switch_port *p;

		if (!(ports & (1 << i)))
			continue;

		p = &val->value.ports[val->len++];
		p->id = i;
		if (priv->vlan_tagged & (1 << i))
			p->flags = (1 << SWITCH_PORT_FLAG_TAGGED);
		else
			p->flags = 0;
	}
	return 0;
}

static int
ar8216_sw_set_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct ar8216_priv *priv = to_ar8216(dev);
	u8 *vt = &priv->vlan_table[val->port_vlan];
	int i, j;

	*vt = 0;
	for (i = 0; i < val->len; i++) {
		struct switch_port *p = &val->value.ports[i];

		if (p->flags & (1 << SWITCH_PORT_FLAG_TAGGED)) {
			priv->vlan_tagged |= (1 << p->id);
		} else {
			priv->vlan_tagged &= ~(1 << p->id);
			priv->pvid[p->id] = val->port_vlan;

			/* make sure that an untagged port does not
			 * appear in other vlans */
			for (j = 0; j < AR8X16_MAX_VLANS; j++) {
				if (j == val->port_vlan)
					continue;
				priv->vlan_table[j] &= ~(1 << p->id);
			}
		}

		*vt |= 1 << p->id;
	}
	return 0;
}

static int
ar8216_sw_hw_apply(struct switch_dev *dev)
{
	struct ar8216_priv *priv = to_ar8216(dev);
	u8 portmask[AR8X16_MAX_PORTS];
	int i, j;

	mutex_lock(&priv->reg_mutex);
	/* flush all vlan translation unit entries */
	priv->chip->vtu_flush(priv);

	memset(portmask, 0, sizeof(portmask));
	if (!priv->init) {
		/* calculate the port destination masks and load vlans
		 * into the vlan translation unit */
		for (j = 0; j < AR8X16_MAX_VLANS; j++) {
			u8 vp = priv->vlan_table[j];

			if (!vp)
				continue;

			for (i = 0; i < dev->ports; i++) {
				u8 mask = (1 << i);
				if (vp & mask)
					portmask[i] |= vp & ~mask;
			}

			priv->chip->vtu_load_vlan(priv, priv->vlan_id[j],
						 priv->vlan_table[j]);
		}
	} else {
		/* vlan disabled:
		 * isolate all ports, but connect them to the cpu port */
		for (i = 0; i < dev->ports; i++) {
			if (i == AR8216_PORT_CPU)
				continue;

			portmask[i] = 1 << AR8216_PORT_CPU;
			portmask[AR8216_PORT_CPU] |= (1 << i);
		}
	}

	/* update the port destination mask registers and tag settings */
	for (i = 0; i < dev->ports; i++) {
		int egress, ingress;
		int pvid;

		if (priv->vlan) {
			pvid = priv->vlan_id[priv->pvid[i]];
			if (priv->vlan_tagged & (1 << i))
				egress = AR8216_OUT_ADD_VLAN;
			else
				egress = AR8216_OUT_STRIP_VLAN;
			ingress = AR8216_IN_SECURE;
		} else {
			pvid = i;
			egress = AR8216_OUT_KEEP;
			ingress = AR8216_IN_PORT_ONLY;
		}

		priv->chip->setup_port(priv, i, egress, ingress, portmask[i],
				       pvid);
	}
	mutex_unlock(&priv->reg_mutex);
	return 0;
}

static int
ar8216_sw_reset_switch(struct switch_dev *dev)
{
	struct ar8216_priv *priv = to_ar8216(dev);
	int i;

	mutex_lock(&priv->reg_mutex);
	memset(&priv->vlan, 0, sizeof(struct ar8216_priv) -
		offsetof(struct ar8216_priv, vlan));

	for (i = 0; i < AR8X16_MAX_VLANS; i++)
		priv->vlan_id[i] = i;

	/* Configure all ports */
	for (i = 0; i < dev->ports; i++)
		priv->chip->init_port(priv, i);

	priv->chip->init_globals(priv);
	mutex_unlock(&priv->reg_mutex);

	return ar8216_sw_hw_apply(dev);
}

static struct switch_attr ar8216_globals[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "Enable VLAN mode",
		.set = ar8216_sw_set_vlan,
		.get = ar8216_sw_get_vlan,
		.max = 1
	},
};

static struct switch_attr ar8216_port[] = {
};

static struct switch_attr ar8216_vlan[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "vid",
		.description = "VLAN ID (0-4094)",
		.set = ar8216_sw_set_vid,
		.get = ar8216_sw_get_vid,
		.max = 4094,
	},
};

static const struct switch_dev_ops ar8216_sw_ops = {
	.attr_global = {
		.attr = ar8216_globals,
		.n_attr = ARRAY_SIZE(ar8216_globals),
	},
	.attr_port = {
		.attr = ar8216_port,
		.n_attr = ARRAY_SIZE(ar8216_port),
	},
	.attr_vlan = {
		.attr = ar8216_vlan,
		.n_attr = ARRAY_SIZE(ar8216_vlan),
	},
	.get_port_pvid = ar8216_sw_get_pvid,
	.set_port_pvid = ar8216_sw_set_pvid,
	.get_vlan_ports = ar8216_sw_get_ports,
	.set_vlan_ports = ar8216_sw_set_ports,
	.apply_config = ar8216_sw_hw_apply,
	.reset_switch = ar8216_sw_reset_switch,
	.get_port_link = ar8216_sw_get_port_link,
};

static int
ar8216_id_chip(struct ar8216_priv *priv)
{
	u32 val;
	u16 id;
	int i;

	priv->chip_type = UNKNOWN;

	val = ar8216_mii_read(priv, AR8216_REG_CTRL);
	if (val == ~0)
		return -ENODEV;

	id = val & (AR8216_CTRL_REVISION | AR8216_CTRL_VERSION);
	for (i = 0; i < AR8X16_PROBE_RETRIES; i++) {
		u16 t;

		val = ar8216_mii_read(priv, AR8216_REG_CTRL);
		if (val == ~0)
			return -ENODEV;

		t = val & (AR8216_CTRL_REVISION | AR8216_CTRL_VERSION);
		if (t != id)
			return -ENODEV;
	}

	switch (id) {
	case 0x0101:
		priv->chip_type = AR8216;
		priv->chip = &ar8216_chip;
		break;
	case 0x0301:
		priv->chip_type = AR8236;
		priv->chip = &ar8236_chip;
		break;
	case 0x1000:
	case 0x1001:
		priv->chip_type = AR8316;
		priv->chip = &ar8316_chip;
		break;
	case 0x1202:
		priv->chip_type = AR8327;
		priv->mii_lo_first = true;
		priv->chip = &ar8327_chip;
		break;
	default:
		printk(KERN_DEBUG
			"ar8216: Unknown Atheros device [ver=%d, rev=%d, phy_id=%04x%04x]\n",
			(int)(id >> AR8216_CTRL_VERSION_S),
			(int)(id & AR8216_CTRL_REVISION),
			mdiobus_read(priv->phy->bus, priv->phy->addr, 2),
			mdiobus_read(priv->phy->bus, priv->phy->addr, 3));

		return -ENODEV;
	}

	return 0;
}

static int
ar8216_config_init(struct phy_device *pdev)
{
	struct ar8216_priv *priv = pdev->priv;
	struct net_device *dev = pdev->attached_dev;
	struct switch_dev *swdev;
	int ret;

	if (!priv) {
		priv = kzalloc(sizeof(struct ar8216_priv), GFP_KERNEL);
		if (priv == NULL)
			return -ENOMEM;
	}

	priv->phy = pdev;

	ret = ar8216_id_chip(priv);
	if (ret)
		goto err_free_priv;

	if (pdev->addr != 0) {
		if (ar8xxx_has_gige(priv)) {
			pdev->supported |= SUPPORTED_1000baseT_Full;
			pdev->advertising |= ADVERTISED_1000baseT_Full;
		}

		if (priv->chip_type == AR8316) {
			/* check if we're attaching to the switch twice */
			pdev = pdev->bus->phy_map[0];
			if (!pdev) {
				kfree(priv);
				return 0;
			}

			/* switch device has not been initialized, reuse priv */
			if (!pdev->priv) {
				priv->port4_phy = true;
				pdev->priv = priv;
				return 0;
			}

			kfree(priv);

			/* switch device has been initialized, reinit */
			priv = pdev->priv;
			priv->dev.ports = (AR8216_NUM_PORTS - 1);
			priv->initialized = false;
			priv->port4_phy = true;
			ar8316_hw_init(priv);
			return 0;
		}

		kfree(priv);
		return 0;
	}

	printk(KERN_INFO "%s: AR%d switch driver attached.\n",
		pdev->attached_dev->name, priv->chip_type);

	if (ar8xxx_has_gige(priv))
		pdev->supported = SUPPORTED_1000baseT_Full;
	else
		pdev->supported = SUPPORTED_100baseT_Full;
	pdev->advertising = pdev->supported;

	mutex_init(&priv->reg_mutex);
	priv->read = ar8216_mii_read;
	priv->write = ar8216_mii_write;

	pdev->priv = priv;

	swdev = &priv->dev;
	swdev->cpu_port = AR8216_PORT_CPU;
	swdev->ops = &ar8216_sw_ops;
	swdev->ports = AR8216_NUM_PORTS;

	if (priv->chip_type == AR8316) {
		swdev->name = "Atheros AR8316";
		swdev->vlans = AR8X16_MAX_VLANS;

		if (priv->port4_phy) {
			/* port 5 connected to the other mac, therefore unusable */
			swdev->ports = (AR8216_NUM_PORTS - 1);
		}
	} else if (priv->chip_type == AR8236) {
		swdev->name = "Atheros AR8236";
		swdev->vlans = AR8216_NUM_VLANS;
		swdev->ports = AR8216_NUM_PORTS;
	} else if (priv->chip_type == AR8327) {
		swdev->name = "Atheros AR8327";
		swdev->vlans = AR8X16_MAX_VLANS;
		swdev->ports = AR8327_NUM_PORTS;
	} else {
		swdev->name = "Atheros AR8216";
		swdev->vlans = AR8216_NUM_VLANS;
	}

	ret = register_switch(&priv->dev, pdev->attached_dev);
	if (ret)
		goto err_free_priv;

	priv->init = true;

	ret = priv->chip->hw_init(priv);
	if (ret)
		goto err_free_priv;

	ret = ar8216_sw_reset_switch(&priv->dev);
	if (ret)
		goto err_free_priv;

	dev->phy_ptr = priv;

	/* VID fixup only needed on ar8216 */
	if (pdev->addr == 0 && priv->chip_type == AR8216) {
		pdev->pkt_align = 2;
		pdev->netif_receive_skb = ar8216_netif_receive_skb;
		pdev->netif_rx = ar8216_netif_rx;
		priv->ndo_old = dev->netdev_ops;
		memcpy(&priv->ndo, priv->ndo_old, sizeof(struct net_device_ops));
		priv->ndo.ndo_start_xmit = ar8216_mangle_tx;
		dev->netdev_ops = &priv->ndo;
	}

	priv->init = false;

	return 0;

err_free_priv:
	kfree(priv);
	return ret;
}

static int
ar8216_read_status(struct phy_device *phydev)
{
	struct ar8216_priv *priv = phydev->priv;
	struct switch_port_link link;
	int ret;

	if (phydev->addr != 0)
		return genphy_read_status(phydev);

	ar8216_read_port_link(priv, phydev->addr, &link);
	phydev->link = !!link.link;
	if (!phydev->link)
		return 0;

	switch (link.speed) {
	case SWITCH_PORT_SPEED_10:
		phydev->speed = SPEED_10;
		break;
	case SWITCH_PORT_SPEED_100:
		phydev->speed = SPEED_100;
		break;
	case SWITCH_PORT_SPEED_1000:
		phydev->speed = SPEED_1000;
		break;
	default:
		phydev->speed = 0;
	}
	phydev->duplex = link.duplex ? DUPLEX_FULL : DUPLEX_HALF;

	/* flush the address translation unit */
	mutex_lock(&priv->reg_mutex);
	ret = priv->chip->atu_flush(priv);
	mutex_unlock(&priv->reg_mutex);

	phydev->state = PHY_RUNNING;
	netif_carrier_on(phydev->attached_dev);
	phydev->adjust_link(phydev->attached_dev);

	return ret;
}

static int
ar8216_config_aneg(struct phy_device *phydev)
{
	if (phydev->addr == 0)
		return 0;

	return genphy_config_aneg(phydev);
}

static int
ar8216_probe(struct phy_device *pdev)
{
	struct ar8216_priv priv;

	priv.phy = pdev;
	return ar8216_id_chip(&priv);
}

static void
ar8216_remove(struct phy_device *pdev)
{
	struct ar8216_priv *priv = pdev->priv;
	struct net_device *dev = pdev->attached_dev;

	if (!priv)
		return;

	if (priv->ndo_old && dev)
		dev->netdev_ops = priv->ndo_old;
	if (pdev->addr == 0)
		unregister_switch(&priv->dev);
	kfree(priv);
}

static struct phy_driver ar8216_driver = {
	.phy_id		= 0x004d0000,
	.name		= "Atheros AR8216/AR8236/AR8316",
	.phy_id_mask	= 0xffff0000,
	.features	= PHY_BASIC_FEATURES,
	.probe		= ar8216_probe,
	.remove		= ar8216_remove,
	.config_init	= &ar8216_config_init,
	.config_aneg	= &ar8216_config_aneg,
	.read_status	= &ar8216_read_status,
	.driver		= { .owner = THIS_MODULE },
};

int __init
ar8216_init(void)
{
	return phy_driver_register(&ar8216_driver);
}

void __exit
ar8216_exit(void)
{
	phy_driver_unregister(&ar8216_driver);
}

module_init(ar8216_init);
module_exit(ar8216_exit);
MODULE_LICENSE("GPL");

