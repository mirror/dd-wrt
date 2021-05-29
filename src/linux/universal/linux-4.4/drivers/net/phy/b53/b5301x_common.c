/*
 * B53 switch driver main logic
 *
 * Copyright (C) 2011-2013 Jonas Gorski <jogo@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/delay.h>
#include <linux/export.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/switch.h>
#include <linux/platform_data/b53.h>

#include "b53_regs.h"
#include "b53_priv.h"

/* buffer size needed for displaying all MIBs with max'd values */
#define B53_BUF_SIZE	1188

struct b53_mib_desc {
	u8 size;
	u8 offset;
	const char *name;
};

/* BCM5365 MIB counters */
static const struct b53_mib_desc b53_mibs_65[] = {
	{ 8, 0x00, "TxOctets" },
	{ 4, 0x08, "TxDropPkts" },
	{ 4, 0x10, "TxBroadcastPkts" },
	{ 4, 0x14, "TxMulticastPkts" },
	{ 4, 0x18, "TxUnicastPkts" },
	{ 4, 0x1c, "TxCollisions" },
	{ 4, 0x20, "TxSingleCollision" },
	{ 4, 0x24, "TxMultipleCollision" },
	{ 4, 0x28, "TxDeferredTransmit" },
	{ 4, 0x2c, "TxLateCollision" },
	{ 4, 0x30, "TxExcessiveCollision" },
	{ 4, 0x38, "TxPausePkts" },
	{ 8, 0x44, "RxOctets" },
	{ 4, 0x4c, "RxUndersizePkts" },
	{ 4, 0x50, "RxPausePkts" },
	{ 4, 0x54, "Pkts64Octets" },
	{ 4, 0x58, "Pkts65to127Octets" },
	{ 4, 0x5c, "Pkts128to255Octets" },
	{ 4, 0x60, "Pkts256to511Octets" },
	{ 4, 0x64, "Pkts512to1023Octets" },
	{ 4, 0x68, "Pkts1024to1522Octets" },
	{ 4, 0x6c, "RxOversizePkts" },
	{ 4, 0x70, "RxJabbers" },
	{ 4, 0x74, "RxAlignmentErrors" },
	{ 4, 0x78, "RxFCSErrors" },
	{ 8, 0x7c, "RxGoodOctets" },
	{ 4, 0x84, "RxDropPkts" },
	{ 4, 0x88, "RxUnicastPkts" },
	{ 4, 0x8c, "RxMulticastPkts" },
	{ 4, 0x90, "RxBroadcastPkts" },
	{ 4, 0x94, "RxSAChanges" },
	{ 4, 0x98, "RxFragments" },
	{ },
};

#define B63XX_MIB_TXB_ID	0	/* TxOctets */
#define B63XX_MIB_RXB_ID	14	/* RxOctets */

/* BCM63xx MIB counters */
static const struct b53_mib_desc b53_mibs_63xx[] = {
	{ 8, 0x00, "TxOctets" },
	{ 4, 0x08, "TxDropPkts" },
	{ 4, 0x0c, "TxQoSPkts" },
	{ 4, 0x10, "TxBroadcastPkts" },
	{ 4, 0x14, "TxMulticastPkts" },
	{ 4, 0x18, "TxUnicastPkts" },
	{ 4, 0x1c, "TxCollisions" },
	{ 4, 0x20, "TxSingleCollision" },
	{ 4, 0x24, "TxMultipleCollision" },
	{ 4, 0x28, "TxDeferredTransmit" },
	{ 4, 0x2c, "TxLateCollision" },
	{ 4, 0x30, "TxExcessiveCollision" },
	{ 4, 0x38, "TxPausePkts" },
	{ 8, 0x3c, "TxQoSOctets" },
	{ 8, 0x44, "RxOctets" },
	{ 4, 0x4c, "RxUndersizePkts" },
	{ 4, 0x50, "RxPausePkts" },
	{ 4, 0x54, "Pkts64Octets" },
	{ 4, 0x58, "Pkts65to127Octets" },
	{ 4, 0x5c, "Pkts128to255Octets" },
	{ 4, 0x60, "Pkts256to511Octets" },
	{ 4, 0x64, "Pkts512to1023Octets" },
	{ 4, 0x68, "Pkts1024to1522Octets" },
	{ 4, 0x6c, "RxOversizePkts" },
	{ 4, 0x70, "RxJabbers" },
	{ 4, 0x74, "RxAlignmentErrors" },
	{ 4, 0x78, "RxFCSErrors" },
	{ 8, 0x7c, "RxGoodOctets" },
	{ 4, 0x84, "RxDropPkts" },
	{ 4, 0x88, "RxUnicastPkts" },
	{ 4, 0x8c, "RxMulticastPkts" },
	{ 4, 0x90, "RxBroadcastPkts" },
	{ 4, 0x94, "RxSAChanges" },
	{ 4, 0x98, "RxFragments" },
	{ 4, 0xa0, "RxSymbolErrors" },
	{ 4, 0xa4, "RxQoSPkts" },
	{ 8, 0xa8, "RxQoSOctets" },
	{ 4, 0xb0, "Pkts1523to2047Octets" },
	{ 4, 0xb4, "Pkts2048to4095Octets" },
	{ 4, 0xb8, "Pkts4096to8191Octets" },
	{ 4, 0xbc, "Pkts8192to9728Octets" },
	{ 4, 0xc0, "RxDiscarded" },
	{ }
};

#define B53XX_MIB_TXB_ID	0	/* TxOctets */
#define B53XX_MIB_RXB_ID	12	/* RxOctets */

/* MIB counters */
static const struct b53_mib_desc b53_mibs[] = {
	{ 8, 0x00, "TxOctets" },
	{ 4, 0x08, "TxDropPkts" },
	{ 4, 0x10, "TxBroadcastPkts" },
	{ 4, 0x14, "TxMulticastPkts" },
	{ 4, 0x18, "TxUnicastPkts" },
	{ 4, 0x1c, "TxCollisions" },
	{ 4, 0x20, "TxSingleCollision" },
	{ 4, 0x24, "TxMultipleCollision" },
	{ 4, 0x28, "TxDeferredTransmit" },
	{ 4, 0x2c, "TxLateCollision" },
	{ 4, 0x30, "TxExcessiveCollision" },
	{ 4, 0x38, "TxPausePkts" },
	{ 8, 0x50, "RxOctets" },
	{ 4, 0x58, "RxUndersizePkts" },
	{ 4, 0x5c, "RxPausePkts" },
	{ 4, 0x60, "Pkts64Octets" },
	{ 4, 0x64, "Pkts65to127Octets" },
	{ 4, 0x68, "Pkts128to255Octets" },
	{ 4, 0x6c, "Pkts256to511Octets" },
	{ 4, 0x70, "Pkts512to1023Octets" },
	{ 4, 0x74, "Pkts1024to1522Octets" },
	{ 4, 0x78, "RxOversizePkts" },
	{ 4, 0x7c, "RxJabbers" },
	{ 4, 0x80, "RxAlignmentErrors" },
	{ 4, 0x84, "RxFCSErrors" },
	{ 8, 0x88, "RxGoodOctets" },
	{ 4, 0x90, "RxDropPkts" },
	{ 4, 0x94, "RxUnicastPkts" },
	{ 4, 0x98, "RxMulticastPkts" },
	{ 4, 0x9c, "RxBroadcastPkts" },
	{ 4, 0xa0, "RxSAChanges" },
	{ 4, 0xa4, "RxFragments" },
	{ 4, 0xa8, "RxJumboPkts" },
	{ 4, 0xac, "RxSymbolErrors" },
	{ 4, 0xc0, "RxDiscarded" },
	{ }
};



/*
 * Swconfig glue functions
 */

static int b53_global_get_vlan_enable(struct switch_dev *dev,
				      const struct switch_attr *attr,
				      struct switch_val *val)
{
	struct b53_device *priv = sw_to_b53(dev);

	val->value.i = priv->enable_vlan;

	return 0;
}

static int b53_global_set_vlan_enable(struct switch_dev *dev,
				      const struct switch_attr *attr,
				      struct switch_val *val)
{
	struct b53_device *priv = sw_to_b53(dev);

	priv->enable_vlan = val->value.i;

	return 0;
}

static int b53_global_get_jumbo_enable(struct switch_dev *dev,
				       const struct switch_attr *attr,
				       struct switch_val *val)
{
	struct b53_device *priv = sw_to_b53(dev);

	val->value.i = priv->enable_jumbo;

	return 0;
}

static int b53_global_set_jumbo_enable(struct switch_dev *dev,
				       const struct switch_attr *attr,
				       struct switch_val *val)
{
	struct b53_device *priv = sw_to_b53(dev);

	priv->enable_jumbo = val->value.i;

	return 0;
}

static int b53_global_get_4095_enable(struct switch_dev *dev,
				      const struct switch_attr *attr,
				      struct switch_val *val)
{
	struct b53_device *priv = sw_to_b53(dev);

	val->value.i = priv->allow_vid_4095;

	return 0;
}

static int b53_global_set_4095_enable(struct switch_dev *dev,
				      const struct switch_attr *attr,
				      struct switch_val *val)
{
	struct b53_device *priv = sw_to_b53(dev);

	priv->allow_vid_4095 = val->value.i;

	return 0;
}

static int b53_global_get_ports(struct switch_dev *dev,
				const struct switch_attr *attr,
				struct switch_val *val)
{
	struct b53_device *priv = sw_to_b53(dev);

	val->len = snprintf(priv->buf, B53_BUF_SIZE, "0x%04x",
			    priv->enabled_ports);
	val->value.s = priv->buf;

	return 0;
}

static int b53_port_get_pvid(struct switch_dev *dev, int port, int *val)
{
	struct b53_device *priv = sw_to_b53(dev);

	*val = priv->ports[port].pvid;

	return 0;
}

static int b53_port_set_pvid(struct switch_dev *dev, int port, int val)
{
	struct b53_device *priv = sw_to_b53(dev);

	if (val > 15 && is5325(priv))
		return -EINVAL;
	if (val == 4095 && !priv->allow_vid_4095)
		return -EINVAL;

	priv->ports[port].pvid = val;

//	printk("%s port=%d, pvid=%d \n", __FUNCTION__, port, val);

	return 0;
}

static int b53_vlan_get_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct b53_device *priv = sw_to_b53(dev);
	struct switch_port *port = &val->value.ports[0];
	struct b53_vlan *vlan = &priv->vlans[val->port_vlan];
	int i;

	val->len = 0;

	if (!vlan->members)
		return 0;

	for (i = 0; i < dev->ports; i++) {
		if (!(vlan->members & BIT(i)))
			continue;


		if (!(vlan->untag & BIT(i)))
			port->flags = BIT(SWITCH_PORT_FLAG_TAGGED);
		else
			port->flags = 0;

		port->id = i;

		val->len++;
		port++;

	}

	return 0;
}

static int b53_vlan_set_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct b53_device *priv = sw_to_b53(dev);
	struct switch_port *port;
	struct b53_vlan *vlan = &priv->vlans[val->port_vlan];
	int i;

	/* only BCM5325 and BCM5365 supports VID 0 */
	if (val->port_vlan == 0 && !is5325(priv) && !is5365(priv))
		return -EINVAL;

	/* VLAN 4095 needs special handling */
	if (val->port_vlan == 4095 && !priv->allow_vid_4095)
		return -EINVAL;

	port = &val->value.ports[0];
	vlan->members = 0;
	vlan->untag = 0;
	for (i = 0; i < val->len; i++, port++) {
		vlan->members |= BIT(port->id);

		if (!(port->flags & BIT(SWITCH_PORT_FLAG_TAGGED))) {
			vlan->untag |= BIT(port->id);
			priv->ports[port->id].pvid = val->port_vlan;
		};
	}

//	printk("%s vlan->members=0x%x, vlan->untag=0x%x \n", __FUNCTION__, vlan->members, vlan->untag);

	/* ignore disabled ports */
	vlan->members &= priv->enabled_ports;
	vlan->untag &= priv->enabled_ports;

	return 0;
}

static int b53_port_get_link(struct switch_dev *dev, int port,
			     struct switch_port_link *link)
{
	struct b53_device *priv = sw_to_b53(dev);

	if (is_cpu_port(priv, port)) {
		link->link = 1;
		link->duplex = 1;
		link->speed = is5325(priv) || is5365(priv) ?
				SWITCH_PORT_SPEED_100 : SWITCH_PORT_SPEED_1000;
		link->aneg = 0;
	} else if (priv->enabled_ports & BIT(port)) {
		u32 speed;
		u16 lnk, duplex;

		b53_read16(priv, B53_STAT_PAGE, B53_LINK_STAT, &lnk);
		b53_read16(priv, B53_STAT_PAGE, priv->duplex_reg, &duplex);

		lnk = (lnk >> port) & 1;
		duplex = (duplex >> port) & 1;

		if (is5325(priv) || is5365(priv)) {
			u16 tmp;

			b53_read16(priv, B53_STAT_PAGE, B53_SPEED_STAT, &tmp);
			speed = SPEED_PORT_FE(tmp, port);
		} else {
			b53_read32(priv, B53_STAT_PAGE, B53_SPEED_STAT, &speed);
			speed = SPEED_PORT_GE(speed, port);
		}

		link->link = lnk;
		if (lnk) {
			link->duplex = duplex;
			switch (speed) {
			case SPEED_STAT_10M:
				link->speed = SWITCH_PORT_SPEED_10;
				break;
			case SPEED_STAT_100M:
				link->speed = SWITCH_PORT_SPEED_100;
				break;
			case SPEED_STAT_1000M:
				link->speed = SWITCH_PORT_SPEED_1000;
				break;
			}
		}

		link->aneg = 1;
	} else {
		link->link = 0;
	}

	return 0;

}

static int b53_port_set_link(struct switch_dev *sw_dev, int port,
			     struct switch_port_link *link)
{
	struct b53_device *dev = sw_to_b53(sw_dev);

	if (port == sw_dev->cpu_port)
		return -EINVAL;

	if (!(BIT(port) & dev->enabled_ports))
		return -EINVAL;

	if (link->speed == SWITCH_PORT_SPEED_1000 &&
	    (is5325(dev) || is5365(dev)))
		return -EINVAL;

	if (link->speed == SWITCH_PORT_SPEED_1000 && !link->duplex)
		return -EINVAL;

	return switch_generic_set_link(sw_dev, port, link);
}

extern void bcm_robo_reset_switch(void *robo);
static int b53_global_reset_switch(struct switch_dev *dev)
{
	struct b53_device *priv = sw_to_b53(dev);

	/* reset vlans */
	priv->enable_vlan = 0;
	priv->enable_jumbo = 0;
	priv->allow_vid_4095 = 0;

	memset(priv->vlans, 0, sizeof(*priv->vlans) * dev->vlans);
	memset(priv->ports, 0, sizeof(*priv->ports) * dev->ports);

	bcm_robo_reset_switch(priv->robo);

	return 0;
}

static void b53_enable_vlan(struct switch_dev *dev, int enable)
{
	struct b53_device *priv = sw_to_b53(dev);
	__u16 val16;
	int disable = enable ? 0 : 1;

	b53_read16(priv, B53_VLAN_PAGE, B53_VLAN_CTRL0, &val16);
	b53_write16(priv, B53_VLAN_PAGE, B53_VLAN_CTRL0, disable ? 0 : val16 | (1 << 7) /* 802.1Q VLAN */ |(3 << 5) /* mac check and hash */ );

	b53_read16(priv, B53_VLAN_PAGE, B53_VLAN_CTRL1, &val16);
	b53_write16(priv, B53_VLAN_PAGE, B53_VLAN_CTRL1, disable ? 0 : val16 | (is5325(priv)  ? (1 << 1) : 0) | (1 << 2) | (1 << 3));	/* RSV multicast */

	if (!is5325(priv))
		return;

//      b53_write16(priv, B53_VLAN_PAGE, B53_VLAN_CTRL4, disable ? 0 :
//              (1 << 6) /* drop invalid VID frames */);
	b53_write16(priv, B53_VLAN_PAGE, B53_VLAN_CTRL4, disable ? 0 : (2 << 6) /* do not check */ );
	b53_write16(priv, B53_VLAN_PAGE, B53_VLAN_CTRL5, disable ? 0 : (1 << 3) /* drop miss V table frames */ );

}

static int b53_set_jumbo(struct b53_device *dev, int enable, int allow_10_100)
{
	u32 port_mask = 0;
	u16 max_size = JMS_MIN_SIZE;

	if (is5325(dev) || is5365(dev))
		return -EINVAL;

	if (enable) {
		port_mask = dev->enabled_ports;
		max_size = JMS_MAX_SIZE;
		if (allow_10_100)
			port_mask |= JPM_10_100_JUMBO_EN;
	}

	b53_write32(dev, B53_JUMBO_PAGE, dev->jumbo_pm_reg, port_mask);
	return b53_write16(dev, B53_JUMBO_PAGE, dev->jumbo_size_reg, max_size);
}

extern int bcm_robo_global_config(void *robo, struct b53_vlan *vlans, int vlans_num, struct b53_port *ports);
static int b53_global_apply_config(struct switch_dev *dev)
{
	struct b53_device *priv = sw_to_b53(dev);

	bcm_robo_global_config(priv->robo, priv->vlans, dev->vlans, priv->ports);
	b53_enable_vlan(dev, priv->enable_vlan);
	if (!is5325(priv) && !is5365(priv))
		b53_set_jumbo(priv, priv->enable_jumbo, 1);

	return 0;
}

static int b53_global_reset_mib(struct switch_dev *dev,
				const struct switch_attr *attr,
				struct switch_val *val)
{
	struct b53_device *priv = sw_to_b53(dev);
	u8 gc;

	b53_read8(priv, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, &gc);

	b53_write8(priv, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, gc | GC_RESET_MIB);
	mdelay(1);
	b53_write8(priv, B53_MGMT_PAGE, B53_GLOBAL_CONFIG, gc & ~GC_RESET_MIB);
	mdelay(1);

	return 0;
}

static int b53_port_set_disable(struct switch_dev *sw_dev,
			    const struct switch_attr *attr,
			    struct switch_val *val)
{
	struct b53_device *dev = sw_to_b53(sw_dev);
	u16 val16;
	int port = val->port_vlan;
	if (!(BIT(port) & dev->enabled_ports))
		return -1;

	b53_read16(dev, B53_CTRL_PAGE, B53_PORT_CTRL(port), &val16);
	val16 &= ~3; /* enable state */
	if (val->value.i)
		val16 |= 3; /* disable */
	
	b53_write16(dev, B53_CTRL_PAGE, B53_PORT_CTRL(port), val16);

	return 0;
}


static int b53_port_get_mib(struct switch_dev *sw_dev,
			    const struct switch_attr *attr,
			    struct switch_val *val)
{
	struct b53_device *dev = sw_to_b53(sw_dev);
	const struct b53_mib_desc *mibs;
	int port = val->port_vlan;
	int len = 0;

	if (!(BIT(port) & dev->enabled_ports))
		return -1;

	if (is5365(dev)) {
		if (port == 5)
			port = 8;

		mibs = b53_mibs_65;
	} else if (is63xx(dev)) {
		mibs = b53_mibs_63xx;
	} else {
		mibs = b53_mibs;
	}

	dev->buf[0] = 0;

	for (; mibs->size > 0; mibs++) {
		u64 val;

		if (mibs->size == 8) {
			b53_read64(dev, B53_MIB_PAGE(port), mibs->offset, &val);
		} else {
			u32 val32;

			b53_read32(dev, B53_MIB_PAGE(port), mibs->offset,
				   &val32);
			val = val32;
		}

		len += snprintf(dev->buf + len, B53_BUF_SIZE - len,
				"%-20s: %llu\n", mibs->name, val);
	}

	val->len = len;
	val->value.s = dev->buf;

	return 0;
}
#define B63XX_MIB_TXB_ID	0	/* TxOctets */
#define B63XX_MIB_RXB_ID	14	/* RxOctets */
#define B53XX_MIB_TXB_ID	0	/* TxOctets */
#define B53XX_MIB_RXB_ID	12	/* RxOctets */

static int b53_port_get_stats(struct switch_dev *sw_dev, int port,
				struct switch_port_stats *stats)
{
	struct b53_device *dev = sw_to_b53(sw_dev);
	const struct b53_mib_desc *mibs;
	int txb_id, rxb_id;
	u64 rxb, txb;

	if (!(BIT(port) & dev->enabled_ports))
		return -EINVAL;

	txb_id = B53XX_MIB_TXB_ID;
	rxb_id = B53XX_MIB_RXB_ID;

	if (is5365(dev)) {
		if (port == 5)
			port = 8;

		mibs = b53_mibs_65;
	} else if (is63xx(dev)) {
		mibs = b53_mibs_63xx;
		txb_id = B63XX_MIB_TXB_ID;
		rxb_id = B63XX_MIB_RXB_ID;
	} else {
		mibs = b53_mibs;
	}

	dev->buf[0] = 0;

	if (mibs->size == 8) {
		b53_read64(dev, B53_MIB_PAGE(port), mibs[txb_id].offset, &txb);
		b53_read64(dev, B53_MIB_PAGE(port), mibs[rxb_id].offset, &rxb);
	} else {
		u32 val32;

		b53_read32(dev, B53_MIB_PAGE(port), mibs[txb_id].offset, &val32);
		txb = val32;

		b53_read32(dev, B53_MIB_PAGE(port), mibs[rxb_id].offset, &val32);
		rxb = val32;
	}

	stats->tx_bytes = txb;
	stats->rx_bytes = rxb;

	return 0;
}



static int b53_phy_read16(struct switch_dev *dev, int addr, u8 reg, u16 *value)
{
	struct b53_device *priv = sw_to_b53(dev);

	if (priv->ops->phy_read16)
		return priv->ops->phy_read16(priv, addr, reg, value);

	return b53_read16(priv, B53_PORT_MII_PAGE(addr), reg, value);
}

static int b53_phy_write16(struct switch_dev *dev, int addr, u8 reg, u16 value)
{
	struct b53_device *priv = sw_to_b53(dev);

	if (priv->ops->phy_write16)
		return priv->ops->phy_write16(priv, addr, reg, value);

	return b53_write16(priv, B53_PORT_MII_PAGE(addr), reg, value);
}

static struct switch_attr b53_global_ops_25[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "Enable VLAN mode",
		.set = b53_global_set_vlan_enable,
		.get = b53_global_get_vlan_enable,
		.max = 1,
	},
	{
		.type = SWITCH_TYPE_STRING,
		.name = "ports",
		.description = "Available ports (as bitmask)",
		.get = b53_global_get_ports,
	},
};

static struct switch_attr b53_global_ops_65[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "Enable VLAN mode",
		.set = b53_global_set_vlan_enable,
		.get = b53_global_get_vlan_enable,
		.max = 1,
	},
	{
		.type = SWITCH_TYPE_STRING,
		.name = "ports",
		.description = "Available ports (as bitmask)",
		.get = b53_global_get_ports,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "reset_mib",
		.description = "Reset MIB counters",
		.set = b53_global_reset_mib,
	},
};

static struct switch_attr b53_global_ops[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "Enable VLAN mode",
		.set = b53_global_set_vlan_enable,
		.get = b53_global_get_vlan_enable,
		.max = 1,
	},
	{
		.type = SWITCH_TYPE_STRING,
		.name = "ports",
		.description = "Available Ports (as bitmask)",
		.get = b53_global_get_ports,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "reset_mib",
		.description = "Reset MIB counters",
		.set = b53_global_reset_mib,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_jumbo",
		.description = "Enable Jumbo Frames",
		.set = b53_global_set_jumbo_enable,
		.get = b53_global_get_jumbo_enable,
		.max = 1,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "allow_vid_4095",
		.description = "Allow VID 4095",
		.set = b53_global_set_4095_enable,
		.get = b53_global_get_4095_enable,
		.max = 1,
	},
};

static struct switch_attr b53_port_ops[] = {
	{
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get port's MIB counters",
		.get = b53_port_get_mib,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "disable",
		.description = "Disable Port",
//		.get = b53_port_get_disable,
		.set = b53_port_set_disable,
	},
};

static struct switch_attr b53_no_ops[] = {
};

static const struct switch_dev_ops b53_switch_ops_25 = {
	.attr_global = {
		.attr = b53_global_ops_25,
		.n_attr = ARRAY_SIZE(b53_global_ops_25),
	},
	.attr_port = {
		.attr = b53_no_ops,
		.n_attr = ARRAY_SIZE(b53_no_ops),
	},
	.attr_vlan = {
		.attr = b53_no_ops,
		.n_attr = ARRAY_SIZE(b53_no_ops),
	},

	.get_vlan_ports = b53_vlan_get_ports,
	.set_vlan_ports = b53_vlan_set_ports,
	.get_port_pvid = b53_port_get_pvid,
	.set_port_pvid = b53_port_set_pvid,
	.apply_config = b53_global_apply_config,
	.reset_switch = b53_global_reset_switch,
	.set_port_link = b53_port_set_link,
	.get_port_link = b53_port_get_link,
	.get_port_stats = b53_port_get_stats,
	.phy_read16 = b53_phy_read16,
	.phy_write16 = b53_phy_write16,
};

static const struct switch_dev_ops b53_switch_ops_65 = {
	.attr_global = {
		.attr = b53_global_ops_65,
		.n_attr = ARRAY_SIZE(b53_global_ops_65),
	},
	.attr_port = {
		.attr = b53_port_ops,
		.n_attr = ARRAY_SIZE(b53_port_ops),
	},
	.attr_vlan = {
		.attr = b53_no_ops,
		.n_attr = ARRAY_SIZE(b53_no_ops),
	},

	.get_vlan_ports = b53_vlan_get_ports,
	.set_vlan_ports = b53_vlan_set_ports,
	.get_port_pvid = b53_port_get_pvid,
	.set_port_pvid = b53_port_set_pvid,
	.apply_config = b53_global_apply_config,
	.reset_switch = b53_global_reset_switch,
	.set_port_link = b53_port_set_link,
	.get_port_link = b53_port_get_link,
	.get_port_stats = b53_port_get_stats,
	.phy_read16 = b53_phy_read16,
	.phy_write16 = b53_phy_write16,
};

static const struct switch_dev_ops b53_switch_ops = {
	.attr_global = {
		.attr = b53_global_ops,
		.n_attr = ARRAY_SIZE(b53_global_ops),
	},
	.attr_port = {
		.attr = b53_port_ops,
		.n_attr = ARRAY_SIZE(b53_port_ops),
	},
	.attr_vlan = {
		.attr = b53_no_ops,
		.n_attr = ARRAY_SIZE(b53_no_ops),
	},

	.get_vlan_ports = b53_vlan_get_ports,
	.set_vlan_ports = b53_vlan_set_ports,
	.get_port_pvid = b53_port_get_pvid,
	.set_port_pvid = b53_port_set_pvid,
	.apply_config = b53_global_apply_config,
	.reset_switch = b53_global_reset_switch,
	.set_port_link = b53_port_set_link,
	.get_port_link = b53_port_get_link,
	.get_port_stats = b53_port_get_stats,
	.phy_read16 = b53_phy_read16,
	.phy_write16 = b53_phy_write16,
};

struct b53_chip_data {
	u32 chip_id;
	const char *dev_name;
	const char *alias;
	u16 vlans;
	u16 enabled_ports;
	u8 cpu_port;
	u8 vta_regs[3];
	u8 duplex_reg;
	u8 jumbo_pm_reg;
	u8 jumbo_size_reg;
	const struct switch_dev_ops *sw_ops;
};

#define B53_VTA_REGS	\
	{ B53_VT_ACCESS, B53_VT_INDEX, B53_VT_ENTRY }
#define B53_VTA_REGS_9798 \
	{ B53_VT_ACCESS_9798, B53_VT_INDEX_9798, B53_VT_ENTRY_9798 }
#define B53_VTA_REGS_63XX \
	{ B53_VT_ACCESS_63XX, B53_VT_INDEX_63XX, B53_VT_ENTRY_63XX }

static const struct b53_chip_data b53_switch_chips[] = {
	{
		.chip_id = BCM5325_DEVICE_ID,
		.dev_name = "BCM5325",
		.alias = "bcm5325",
		.vlans = 16,
		.enabled_ports = 0x1f,
		.cpu_port = B53_CPU_PORT_25,
		.duplex_reg = B53_DUPLEX_STAT_FE,
		.sw_ops = &b53_switch_ops_25,
	},
	{
		.chip_id = BCM5365_DEVICE_ID,
		.dev_name = "BCM5365",
		.alias = "bcm5365",
		.vlans = 256,
		.enabled_ports = 0x1f,
		.cpu_port = B53_CPU_PORT_25,
		.duplex_reg = B53_DUPLEX_STAT_FE,
		.sw_ops = &b53_switch_ops_65,
	},
	{
		.chip_id = BCM5395_DEVICE_ID,
		.dev_name = "BCM5395",
		.alias = "bcm5395",
		.vlans = 4096,
		.enabled_ports = 0x1f,
		.cpu_port = B53_CPU_PORT,
		.vta_regs = B53_VTA_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		.sw_ops = &b53_switch_ops,
	},
	{
		.chip_id = BCM5397_DEVICE_ID,
		.dev_name = "BCM5397",
		.alias = "bcm5397",
		.vlans = 4096,
		.enabled_ports = 0x1f,
		.cpu_port = B53_CPU_PORT,
		.vta_regs = B53_VTA_REGS_9798,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		.sw_ops = &b53_switch_ops,
	},
	{
		.chip_id = BCM5398_DEVICE_ID,
		.dev_name = "BCM5398",
		.alias = "bcm5398",
		.vlans = 4096,
		.enabled_ports = 0x7f,
		.cpu_port = B53_CPU_PORT,
		.vta_regs = B53_VTA_REGS_9798,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		.sw_ops = &b53_switch_ops,
	},
	{
		.chip_id = BCM53115_DEVICE_ID,
		.dev_name = "BCM53115",
		.alias = "bcm53115",
		.vlans = 4096,
		.enabled_ports = 0x1f,
		.vta_regs = B53_VTA_REGS,
		.cpu_port = B53_CPU_PORT,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		.sw_ops = &b53_switch_ops,
	},
	{
		.chip_id = BCM53125_DEVICE_ID,
		.dev_name = "BCM53125",
		.alias = "bcm53125",
		.vlans = 4096,
		.enabled_ports = 0x1f,
		.cpu_port = B53_CPU_PORT,
		.vta_regs = B53_VTA_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		.sw_ops = &b53_switch_ops,
	},
	{
		.chip_id = BCM53128_DEVICE_ID,
		.dev_name = "BCM53128",
		.alias = "bcm53128",
		.vlans = 4096,
		.enabled_ports = 0x1ff,
		.cpu_port = B53_CPU_PORT,
		.vta_regs = B53_VTA_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		.sw_ops = &b53_switch_ops,
	},
	{
		.chip_id = BCM63XX_DEVICE_ID,
		.dev_name = "BCM63xx",
		.alias = "bcm63xx",
		.vlans = 4096,
		.enabled_ports = 0, /* pdata must provide them */
		.cpu_port = B53_CPU_PORT,
		.vta_regs = B53_VTA_REGS_63XX,
		.duplex_reg = B53_DUPLEX_STAT_63XX,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK_63XX,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE_63XX,
		.sw_ops = &b53_switch_ops,
	},
	{
		.chip_id = BCM53010_DEVICE_ID,
		.dev_name = "BCM53010",
		.alias = "bcm53011",
		.vlans = 4096,
		.enabled_ports = 0x1f,
		.cpu_port = B53_CPU_PORT_25, /* TODO: auto detect */
		.vta_regs = B53_VTA_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		.sw_ops = &b53_switch_ops,
	},
	{
		.chip_id = BCM53011_DEVICE_ID,
		.dev_name = "BCM53011",
		.alias = "bcm53011",
		.vlans = 4096,
		.enabled_ports = 0x1bf,
		.cpu_port = B53_CPU_PORT_25, /* TODO: auto detect */
		.vta_regs = B53_VTA_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		.sw_ops = &b53_switch_ops,
	},
	{
		.chip_id = BCM53012_DEVICE_ID,
		.dev_name = "BCM53012",
		.alias = "bcm53011",
		.vlans = 4096,
		.enabled_ports = 0x1bf,
		.cpu_port = B53_CPU_PORT_25, /* TODO: auto detect */
		.vta_regs = B53_VTA_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		.sw_ops = &b53_switch_ops,
	},
	{
		.chip_id = BCM53018_DEVICE_ID,
		.dev_name = "BCM53018",
		.alias = "bcm53018",
		.vlans = 4096,
		.enabled_ports = 0x1f,
		.cpu_port = B53_CPU_PORT_25, /* TODO: auto detect */
		.vta_regs = B53_VTA_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		.sw_ops = &b53_switch_ops,
	},
	{
		.chip_id = BCM53019_DEVICE_ID,
		.dev_name = "BCM53019",
		.alias = "bcm53019",
		.vlans = 4096,
		.enabled_ports = 0x1f,
		.cpu_port = B53_CPU_PORT_25, /* TODO: auto detect */
		.vta_regs = B53_VTA_REGS,
		.duplex_reg = B53_DUPLEX_STAT_GE,
		.jumbo_pm_reg = B53_JUMBO_PORT_MASK,
		.jumbo_size_reg = B53_JUMBO_MAX_SIZE,
		.sw_ops = &b53_switch_ops,
	},
};

static int b53_switch_init(struct b53_device *dev)
{
	struct switch_dev *sw_dev = &dev->sw_dev;
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(b53_switch_chips); i++) {
		const struct b53_chip_data *chip = &b53_switch_chips[i];

		if (chip->chip_id == dev->chip_id) {
			sw_dev->name = chip->dev_name;
//			printk("%s, dev->chip_id=%d, sw_dev->name=%s \n", __FUNCTION__, dev->chip_id, sw_dev->name);

			if (!sw_dev->alias)
				sw_dev->alias = chip->alias;
			if (!dev->enabled_ports)
				dev->enabled_ports = chip->enabled_ports;
			dev->duplex_reg = chip->duplex_reg;
			dev->vta_regs[0] = chip->vta_regs[0];
			dev->vta_regs[1] = chip->vta_regs[1];
			dev->vta_regs[2] = chip->vta_regs[2];
			dev->jumbo_pm_reg = chip->jumbo_pm_reg;
			sw_dev->ops = chip->sw_ops;
			sw_dev->cpu_port = chip->cpu_port;
			sw_dev->vlans = chip->vlans;
			break;
		}
	}

	if (!sw_dev->name)
		return -EINVAL;

	/* check which BCM5325x version we have */
	if (is5325(dev)) {
		u8 vc4;

		b53_read8(dev, B53_VLAN_PAGE, B53_VLAN_CTRL4_25, &vc4);

		/* check reserved bits */
		switch (vc4 & 3) {
		case 1:
			/* BCM5325E */
			break;
		case 3:
			/* BCM5325F - do not use port 4 */
			dev->enabled_ports &= ~BIT(4);
			break;
		default:
/* On the BCM47XX SoCs this is the supported internal switch.*/
#ifndef CONFIG_BCM47XX
			/* BCM5325M */
			return -EINVAL;
#else
			break;
#endif
		}
	} else if (dev->chip_id == BCM53115_DEVICE_ID) {
		u64 strap_value;

		b53_read48(dev, B53_STAT_PAGE, B53_STRAP_VALUE, &strap_value);
		/* use second IMP port if GMII is enabled */
		if (strap_value & SV_GMII_CTRL_115)
			sw_dev->cpu_port = 5;
	}

	/* BCM5301x switch has port0, port1, ... port8 */
	sw_dev->ports = 8 + 1;

	dev->enabled_ports |= BIT(sw_dev->cpu_port);

	dev->ports = devm_kzalloc(dev->dev,
				  sizeof(struct b53_port) * sw_dev->ports,
				  GFP_KERNEL);
	if (!dev->ports)
		return -ENOMEM;

	dev->vlans = devm_kzalloc(dev->dev,
				  sizeof(struct b53_vlan) * sw_dev->vlans,
				  GFP_KERNEL);
	if (!dev->vlans)
		return -ENOMEM;

	dev->buf = devm_kzalloc(dev->dev, B53_BUF_SIZE, GFP_KERNEL);
	if (!dev->buf)
		return -ENOMEM;

	return 0;

}

struct b53_device *b53_switch_alloc(struct device *base, struct b53_io_ops *ops,
				    void *priv)
{
	struct b53_device *dev;

	dev = devm_kzalloc(base, sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return NULL;

	dev->dev = base;
	dev->ops = ops;
	dev->priv = priv;
	mutex_init(&dev->reg_mutex);

	return dev;
}
EXPORT_SYMBOL(b53_switch_alloc);

int b53_switch_detect(struct b53_device *dev)
{
	u32 id32;
	u16 tmp;
	u8 id8;
	int ret;

	ret = b53_read8(dev, B53_MGMT_PAGE, B53_DEVICE_ID, &id8);
	if (ret)
		return ret;

	switch (id8) {
	case 0:
		/*
		 * BCM5325 and BCM5365 do not have this register so reads
		 * return 0. But the read operation did succeed, so assume
		 * this is one of them.
		 *
		 * Next check if we can write to the 5325's VTA register; for
		 * 5365 it is read only.
		 */

		b53_write16(dev, B53_VLAN_PAGE, B53_VLAN_TABLE_ACCESS_25, 0xf);
		b53_read16(dev, B53_VLAN_PAGE, B53_VLAN_TABLE_ACCESS_25, &tmp);

		if (tmp == 0xf)
			dev->chip_id = BCM5325_DEVICE_ID;
		else
			dev->chip_id = BCM5365_DEVICE_ID;
		break;
	case BCM5395_DEVICE_ID:
	case BCM5397_DEVICE_ID:
	case BCM5398_DEVICE_ID:
		dev->chip_id = id8;
		break;
	default:
		ret = b53_read32(dev, B53_MGMT_PAGE, B53_DEVICE_ID, &id32);
		if (ret)
			return ret;

		switch (id32) {
		case BCM53115_DEVICE_ID:
		case BCM53125_DEVICE_ID:
		case BCM53128_DEVICE_ID:
		case BCM53010_DEVICE_ID:
		case BCM53011_DEVICE_ID:
		case BCM53012_DEVICE_ID:
		case BCM53018_DEVICE_ID:
		case BCM53019_DEVICE_ID:
			dev->chip_id = id32;
			break;
		default:
			pr_err("unsupported switch detected (BCM53%02x/BCM%x)\n",
			       id8, id32);
			return -ENODEV;
		}
	}

	if (dev->chip_id == BCM5325_DEVICE_ID)
		return b53_read8(dev, B53_STAT_PAGE, B53_REV_ID_25,
				 &dev->core_rev);
	else
		return b53_read8(dev, B53_MGMT_PAGE, B53_REV_ID,
				 &dev->core_rev);
}
EXPORT_SYMBOL(b53_switch_detect);

struct b53_device *thisdev=NULL;
static inline int get_port(struct net_device *dev)
{
	if (!strcmp(dev->name,"WAN"))  return(4);
	if (!strcmp(dev->name,"LAN1")) return(3);
	if (!strcmp(dev->name,"LAN2")) return(2);
	if (!strcmp(dev->name,"LAN3")) return(1);
	if (!strcmp(dev->name,"LAN4")) return(0);
	return(-1);
}

/* MIB counters */
static const struct b53_mib_desc b53_dev_mibs[] = {
	{ 8, 0x00, "TxOctets" },
	{ 4, 0x08, "TxDropPkts" },
	{ 4, 0x10, "TxBroadcastPkts" },
	{ 4, 0x14, "TxMulticastPkts" },
	{ 4, 0x18, "TxUnicastPkts" },
	{ 4, 0x84, "RxFCSErrors" },
	{ 8, 0x88, "RxGoodOctets" },
	{ 4, 0x90, "RxDropPkts" },
	{ 4, 0x94, "RxUnicastPkts" },
	{ 4, 0x98, "RxMulticastPkts" },
	{ 4, 0x9c, "RxBroadcastPkts" },
	{ 4, 0xc0, "RxDiscarded" },
	{ }
};

int swconfig_dev_ndo_get_stats64(struct net_device *srgdev,int port,struct rtnl_link_stats64 *storage)
{
	const struct b53_mib_desc *mibs;
	struct b53_device *dev = thisdev;
	u64  tx_multicast=0;
	u64  rx_multicast=0;
	u64  tx_broadcast=0;
	u64  rx_broadcast=0;
	u64  rx_discard=0;

	if (dev==NULL) return(-1);

	if (port < 0) return(-1);
		
	mibs = b53_dev_mibs;

	for (; mibs->size > 0; mibs++) {
		u64 val;

		if (mibs->size == 8) {
			b53_read64(dev, B53_MIB_PAGE(port), mibs->offset, &val);
		} else {
			u32 val32;

			b53_read32(dev, B53_MIB_PAGE(port), mibs->offset,
				   &val32);
			val = val32;
		}
		switch(mibs->offset) {
			case 0x00:
			storage->tx_bytes = val;
			break;
			case 0x08:
			storage->tx_dropped = val;
			break;
			case 0x10:
			tx_broadcast = val;
			break;
			case 0x14:
			tx_multicast = val;
			break;
			case 0x18:
			storage->tx_packets = val;
			break;
			case 0x84:
			storage->rx_crc_errors = val;
			break;
			case 0x88:
			storage->rx_bytes = val;
			break;
			case 0x90:
			storage->rx_dropped = val;
			break;
			case 0x94:
			storage->rx_packets = val;
			case 0x98:
			rx_multicast = val;
			break;
			case 0x9c:
			rx_broadcast = val;
			break;
			case 0xc0:
			rx_discard = val;
			break;
		}
	}
	storage->rx_packets += rx_multicast;
	storage->rx_packets += rx_broadcast;
	storage->tx_packets += tx_multicast;
	storage->tx_packets += tx_broadcast;
	storage->rx_dropped += rx_discard;
	storage->multicast = rx_multicast;
	
	return(0);
}
EXPORT_SYMBOL(swconfig_dev_ndo_get_stats64);


int swconfig_dev_get_link(struct net_device *srgdev,int port,int *carrier,int *speed,int *duplex)
{
	int ret;
	u32 spd;
	u16 lnk;
	struct b53_device *dev = thisdev;

	if (dev==NULL) return(-1);

	if (port < 0) return(-1);

	ret=b53_read16(dev, B53_STAT_PAGE, B53_LINK_STAT, &lnk);

	if (ret !=0) return -1;

	lnk = (lnk >> port) & 1;

	b53_read32(dev, B53_STAT_PAGE, B53_SPEED_STAT, &spd);

	spd = SPEED_PORT_GE(spd, port);

	if (lnk) {
		switch (spd) {
		case SPEED_STAT_10M:
			*speed = 10;
			break;
		case SPEED_STAT_100M:
			*speed = 100;
			break;
		case SPEED_STAT_1000M:
			*speed = 1000;
			break;
		}
		*duplex=1;
		*carrier=1;
	} else {
		*carrier=0;
	}

	return(0);
}
EXPORT_SYMBOL(swconfig_dev_get_link);


int b53_switch_register(struct b53_device *dev)
{
	int ret;
	if (dev->pdata) {
		dev->chip_id = dev->pdata->chip_id;
		dev->enabled_ports = dev->pdata->enabled_ports;
		dev->sw_dev.alias = dev->pdata->alias;

		dev->robo = dev->pdata->robo;
	}

	if (!dev->chip_id && b53_switch_detect(dev))
		return -EINVAL;

	ret = b53_switch_init(dev);
	if (ret)
		return ret;

	thisdev=dev;
//	printk("%s:%d TMH counter fix plus mc pdb port swconfig switch: %s, rev %i\n",__FUNCTION__,__LINE__,thisdev->sw_dev.name, thisdev->core_rev);
	pr_info("found switch: %s, rev %i\n", dev->sw_dev.name, dev->core_rev);

	return register_switch(&dev->sw_dev, NULL);
}
EXPORT_SYMBOL(b53_switch_register);

MODULE_AUTHOR("Jonas Gorski <jogo@openwrt.org>");
MODULE_DESCRIPTION("B53 switch library");
MODULE_LICENSE("Dual BSD/GPL");
