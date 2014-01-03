/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2009-2013 John Crispin <blogic@openwrt.org>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/of_net.h>
#include <linux/of_mdio.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/switch.h>

#include <asm/mach-ralink-openwrt/ralink_regs.h>

#include "ralink_soc_eth.h"

#include <linux/ioport.h>
#include <linux/switch.h>
#include <linux/mii.h>

#include <ralink_regs.h>
#include <asm/mach-ralink-openwrt/mt7620.h>

#include "ralink_soc_eth.h"
#include "gsw_mt7620a.h"
#include "mdio.h"

#define GSW_REG_PHY_TIMEOUT	(5 * HZ)

#define MT7620A_GSW_REG_PIAC	0x7004

#define GSW_NUM_VLANS		16
#define GSW_NUM_VIDS		4096
#define GSW_NUM_PORTS		7
#define GSW_PORT6		6

#define GSW_MDIO_ACCESS		BIT(31)
#define GSW_MDIO_READ		BIT(19)
#define GSW_MDIO_WRITE		BIT(18)
#define GSW_MDIO_START		BIT(16)
#define GSW_MDIO_ADDR_SHIFT	20
#define GSW_MDIO_REG_SHIFT	25

#define GSW_REG_PORT_PMCR(x)	(0x3000 + (x * 0x100))
#define GSW_REG_PORT_STATUS(x)	(0x3008 + (x * 0x100))
#define GSW_REG_SMACCR0		0x3fE4
#define GSW_REG_SMACCR1		0x3fE8
#define GSW_REG_CKGCR		0x3ff0

#define GSW_REG_IMR		0x7008
#define GSW_REG_ISR		0x700c

#define SYSC_REG_CFG1		0x14

#define PORT_IRQ_ST_CHG		0x7f

#define GSW_VLAN_VTCR		0x90
#define GSW_VLAN_VTCR_VID_M	0xfff
#define GSW_VLAN_ID(_x)		(0x100 + (4 * (_x)))
#define GSW_VLAN_ID_VID_S	12
#define GSW_VLAN_ID_VID_M	0xfff

#define GSW_VAWD1		0x94
#define GSW_VAWD1_VTAG_EN	BIT(28)
#define GSW_VAWD1_PORTM_S	16
#define GSW_VAWD1_PORTM_M	0xff

#define GSW_VAWD2		0x98
#define GSW_VAWD2_PORTT_S	16
#define GSW_VAWD2_PORTT_M	0xff

#define GSW_VTIM(_x)		(0x100 + (4 * (_x)))
#define GSW_VTIM_M		0xfff
#define GSW_VTIM_S		12

#define GSW_REG_PCR(x)		(0x2004 + (x * 0x100))
#define GSW_REG_PCR_EG_TAG_S	28
#define GSW_REG_PCR_EG_TAG_M	0x3

#define SYSCFG1			0x14

#define ESW_PHY_POLLING		0x7000

#define	PMCR_IPG		BIT(18)
#define	PMCR_MAC_MODE		BIT(16)
#define	PMCR_FORCE		BIT(15)
#define	PMCR_TX_EN		BIT(14)
#define	PMCR_RX_EN		BIT(13)
#define	PMCR_BACKOFF		BIT(9)
#define	PMCR_BACKPRES		BIT(8)
#define	PMCR_RX_FC		BIT(5)
#define	PMCR_TX_FC		BIT(4)
#define	PMCR_SPEED(_x)		(_x << 2)
#define	PMCR_DUPLEX		BIT(1)
#define	PMCR_LINK		BIT(0)

#define PHY_AN_EN		BIT(31)
#define PHY_PRE_EN		BIT(30)
#define PMY_MDC_CONF(_x)	((_x & 0x3f) << 24)

enum {
	/* Global attributes. */
	GSW_ATTR_ENABLE_VLAN,
	/* Port attributes. */
	GSW_ATTR_PORT_UNTAG,
};

enum {
	PORT4_EPHY = 0,
	PORT4_EXT,
};

struct gsw_port {
	bool	disable;
	bool	untag;
	u16	pvid;
};

struct gsw_vlan {
	u8	ports;
	u16	vid;
};

struct mt7620_gsw {
	struct device		*dev;
	void __iomem		*base;
	int			irq;

	struct switch_dev	swdev;
	bool			global_vlan_enable;
	struct gsw_vlan		vlans[GSW_NUM_VLANS];
	struct gsw_port		ports[GSW_NUM_PORTS];
	long unsigned int	autopoll;
	int			port4;
};

static inline void gsw_w32(struct mt7620_gsw *gsw, u32 val, unsigned reg)
{
	iowrite32(val, gsw->base + reg);
}

static inline u32 gsw_r32(struct mt7620_gsw *gsw, unsigned reg)
{
	return ioread32(gsw->base + reg);
}

static int mt7620_mii_busy_wait(struct mt7620_gsw *gsw)
{
	unsigned long t_start = jiffies;

	while (1) {
		if (!(gsw_r32(gsw, MT7620A_GSW_REG_PIAC) & GSW_MDIO_ACCESS))
			return 0;
		if (time_after(jiffies, t_start + GSW_REG_PHY_TIMEOUT)) {
			break;
		}
	}

	printk(KERN_ERR "mdio: MDIO timeout\n");
	return -1;
}

static u32 _mt7620_mii_write(struct mt7620_gsw *gsw, u32 phy_addr, u32 phy_register,
				u32 write_data)
{
	if (mt7620_mii_busy_wait(gsw))
		return -1;

	write_data &= 0xffff;

	gsw_w32(gsw, GSW_MDIO_ACCESS | GSW_MDIO_START | GSW_MDIO_WRITE |
		(phy_register << GSW_MDIO_REG_SHIFT) |
		(phy_addr << GSW_MDIO_ADDR_SHIFT) | write_data,
		MT7620A_GSW_REG_PIAC);

	if (mt7620_mii_busy_wait(gsw))
		return -1;

	return 0;
}

int mt7620_mdio_write(struct mii_bus *bus, int phy_addr, int phy_reg, u16 val)
{
	struct fe_priv *priv = bus->priv;
	struct mt7620_gsw *gsw = (struct mt7620_gsw *) priv->soc->swpriv;

	return _mt7620_mii_write(gsw, phy_addr, phy_reg, val);
}

int mt7620_mdio_read(struct mii_bus *bus, int phy_addr, int phy_reg)
{
	struct fe_priv *priv = bus->priv;
	struct mt7620_gsw *gsw = (struct mt7620_gsw *) priv->soc->swpriv;
	u32 d;

	if (mt7620_mii_busy_wait(gsw))
		return 0xffff;

	gsw_w32(gsw, GSW_MDIO_ACCESS | GSW_MDIO_START | GSW_MDIO_READ |
		(phy_reg << GSW_MDIO_REG_SHIFT) |
		(phy_addr << GSW_MDIO_ADDR_SHIFT),
		MT7620A_GSW_REG_PIAC);

	if (mt7620_mii_busy_wait(gsw))
		return 0xffff;

	d = gsw_r32(gsw, MT7620A_GSW_REG_PIAC) & 0xffff;

	return d;
}

static unsigned char *fe_speed_str(int speed)
{
	switch (speed) {
	case 2:
	case SPEED_1000:
		return "1000";
	case 1:
	case SPEED_100:
		return "100";
	case 0:
	case SPEED_10:
		return "10";
	}

	return "? ";
}

int mt7620a_has_carrier(struct fe_priv *priv)
{
        struct mt7620_gsw *gsw = (struct mt7620_gsw *) priv->soc->swpriv;
	int i;

	for (i = 0; i < GSW_PORT6; i++)
		if (gsw_r32(gsw, GSW_REG_PORT_STATUS(i)) & 0x1)
			return 1;
	return 0;
}

static void mt7620a_handle_carrier(struct fe_priv *priv)
{
	if (!priv->phy)
		return;

	if (mt7620a_has_carrier(priv))
		netif_carrier_on(priv->netdev);
	else
		netif_carrier_off(priv->netdev);
}

void mt7620_mdio_link_adjust(struct fe_priv *priv, int port)
{
	if (priv->link[port])
		netdev_info(priv->netdev, "port %d link up (%sMbps/%s duplex)\n",
			port, fe_speed_str(priv->phy->speed[port]),
			(DUPLEX_FULL == priv->phy->duplex[port]) ? "Full" : "Half");
	else
		netdev_info(priv->netdev, "port %d link down\n", port);
	mt7620a_handle_carrier(priv);
}

static irqreturn_t gsw_interrupt(int irq, void *_priv)
{
	struct fe_priv *priv = (struct fe_priv *) _priv;
	struct mt7620_gsw *gsw = (struct mt7620_gsw *) priv->soc->swpriv;
	u32 status;
	int i, max = (gsw->port4 == PORT4_EPHY) ? (4) : (3);

	status = gsw_r32(gsw, GSW_REG_ISR);
	if (status & PORT_IRQ_ST_CHG)
		for (i = 0; i <= max; i++) {
			u32 status = gsw_r32(gsw, GSW_REG_PORT_STATUS(i));
			int link = status & 0x1;

			if (link != priv->link[i]) {
				if (link)
					netdev_info(priv->netdev, "port %d link up (%sMbps/%s duplex)\n",
							i, fe_speed_str((status >> 2) & 3),
							(status & 0x2) ? "Full" : "Half");
				else
					netdev_info(priv->netdev, "port %d link down\n", i);
			}

			priv->link[i] = link;
		}
	mt7620a_handle_carrier(priv);

	gsw_w32(gsw, status, GSW_REG_ISR);

	return IRQ_HANDLED;
}

static int mt7620_is_bga(void)
{
	u32 bga = rt_sysc_r32(0x0c);

	return (bga >> 16) & 1;
}

static void gsw_auto_poll(struct mt7620_gsw *gsw)
{
	int phy;
	int lsb = -1, msb = 0;

	for_each_set_bit(phy, &gsw->autopoll, 32) {
		if (lsb < 0)
			lsb = phy;
		msb = phy;
	}

	gsw_w32(gsw, PHY_AN_EN | PHY_PRE_EN | PMY_MDC_CONF(5) | (msb << 8) | lsb, ESW_PHY_POLLING);
}

void mt7620_port_init(struct fe_priv *priv, struct device_node *np)
{
	struct mt7620_gsw *gsw = (struct mt7620_gsw *) priv->soc->swpriv;
	const __be32 *_id = of_get_property(np, "reg", NULL);
	int phy_mode, size, id;
	int shift = 12;
	u32 val, mask = 0;
	int min = (gsw->port4 == PORT4_EPHY) ? (5) : (4);

	if (!_id || (be32_to_cpu(*_id) < min) || (be32_to_cpu(*_id) > 5)) {
		if (_id)
			pr_err("%s: invalid port id %d\n", np->name, be32_to_cpu(*_id));
		else
			pr_err("%s: invalid port id\n", np->name);
		return;
	}

	id = be32_to_cpu(*_id);

	if (id == 4)
		shift = 14;

	priv->phy->phy_fixed[id] = of_get_property(np, "ralink,fixed-link", &size);
	if (priv->phy->phy_fixed[id] && (size != (4 * sizeof(*priv->phy->phy_fixed[id])))) {
		pr_err("%s: invalid fixed link property\n", np->name);
		priv->phy->phy_fixed[id] = NULL;
		return;
	}

	phy_mode = of_get_phy_mode(np);
	switch (phy_mode) {
	case PHY_INTERFACE_MODE_RGMII:
		mask = 0;
		break;
	case PHY_INTERFACE_MODE_MII:
		mask = 1;
		break;
	case PHY_INTERFACE_MODE_RMII:
		mask = 2;
		break;
	default:
		dev_err(priv->device, "port %d - invalid phy mode\n", priv->phy->speed[id]);
		return;
	}

	priv->phy->phy_node[id] = of_parse_phandle(np, "phy-handle", 0);
	if (!priv->phy->phy_node[id] && !priv->phy->phy_fixed[id])
		return;

	val = rt_sysc_r32(SYSCFG1);
	val &= ~(3 << shift);
	val |= mask << shift;
	rt_sysc_w32(val, SYSCFG1);

	if (priv->phy->phy_fixed[id]) {
		const __be32 *link = priv->phy->phy_fixed[id];
		int tx_fc = be32_to_cpup(link++);
		int rx_fc = be32_to_cpup(link++);
		u32 val = 0;

		priv->phy->speed[id] = be32_to_cpup(link++);
		priv->phy->duplex[id] = be32_to_cpup(link++);
		priv->link[id] = 1;

		switch (priv->phy->speed[id]) {
		case SPEED_10:
			val = 0;
			break;
		case SPEED_100:
			val = 1;
			break;
		case SPEED_1000:
			val = 2;
			break;
		default:
			dev_err(priv->device, "invalid link speed: %d\n", priv->phy->speed[id]);
			priv->phy->phy_fixed[id] = 0;
			return;
		}
		val = PMCR_SPEED(val);
		val |= PMCR_LINK | PMCR_BACKPRES | PMCR_BACKOFF | PMCR_RX_EN |
			PMCR_TX_EN | PMCR_FORCE | PMCR_MAC_MODE | PMCR_IPG;
		if (tx_fc)
			val |= PMCR_TX_FC;
		if (rx_fc)
			val |= PMCR_RX_FC;
		if (priv->phy->duplex[id])
			val |= PMCR_DUPLEX;
		gsw_w32(gsw, val, GSW_REG_PORT_PMCR(id));
		dev_info(priv->device, "using fixed link parameters\n");
		return;
	}

	if (priv->phy->phy_node[id] && priv->mii_bus->phy_map[id]) {
		u32 val = PMCR_BACKPRES | PMCR_BACKOFF | PMCR_RX_EN |
			PMCR_TX_EN |  PMCR_MAC_MODE | PMCR_IPG;

		gsw_w32(gsw, val, GSW_REG_PORT_PMCR(id));
		fe_connect_phy_node(priv, priv->phy->phy_node[id]);
		gsw->autopoll |= BIT(id);
		gsw_auto_poll(gsw);
		return;
	}
}

static void gsw_hw_init(struct mt7620_gsw *gsw)
{
	u32 is_BGA = mt7620_is_bga();

	rt_sysc_w32(rt_sysc_r32(SYSC_REG_CFG1) | BIT(8), SYSC_REG_CFG1);
	gsw_w32(gsw, gsw_r32(gsw, GSW_REG_CKGCR) & ~(0x3 << 4), GSW_REG_CKGCR);

	/*correct  PHY  setting L3.0 BGA*/
	_mt7620_mii_write(gsw, 1, 31, 0x4000); //global, page 4

	_mt7620_mii_write(gsw, 1, 17, 0x7444);
	if (is_BGA)
		_mt7620_mii_write(gsw, 1, 19, 0x0114);
	else
		_mt7620_mii_write(gsw, 1, 19, 0x0117);

	_mt7620_mii_write(gsw, 1, 22, 0x10cf);
	_mt7620_mii_write(gsw, 1, 25, 0x6212);
	_mt7620_mii_write(gsw, 1, 26, 0x0777);
	_mt7620_mii_write(gsw, 1, 29, 0x4000);
	_mt7620_mii_write(gsw, 1, 28, 0xc077);
	_mt7620_mii_write(gsw, 1, 24, 0x0000);

	_mt7620_mii_write(gsw, 1, 31, 0x3000); //global, page 3
	_mt7620_mii_write(gsw, 1, 17, 0x4838);

	_mt7620_mii_write(gsw, 1, 31, 0x2000); //global, page 2
	if (is_BGA) {
		_mt7620_mii_write(gsw, 1, 21, 0x0515);
		_mt7620_mii_write(gsw, 1, 22, 0x0053);
		_mt7620_mii_write(gsw, 1, 23, 0x00bf);
		_mt7620_mii_write(gsw, 1, 24, 0x0aaf);
		_mt7620_mii_write(gsw, 1, 25, 0x0fad);
		_mt7620_mii_write(gsw, 1, 26, 0x0fc1);
	} else {
		_mt7620_mii_write(gsw, 1, 21, 0x0517);
		_mt7620_mii_write(gsw, 1, 22, 0x0fd2);
		_mt7620_mii_write(gsw, 1, 23, 0x00bf);
		_mt7620_mii_write(gsw, 1, 24, 0x0aab);
		_mt7620_mii_write(gsw, 1, 25, 0x00ae);
		_mt7620_mii_write(gsw, 1, 26, 0x0fff);
	}
	_mt7620_mii_write(gsw, 1, 31, 0x1000); //global, page 1
	_mt7620_mii_write(gsw, 1, 17, 0xe7f8);

	_mt7620_mii_write(gsw, 1, 31, 0x8000); //local, page 0
	_mt7620_mii_write(gsw, 0, 30, 0xa000);
	_mt7620_mii_write(gsw, 1, 30, 0xa000);
	_mt7620_mii_write(gsw, 2, 30, 0xa000);
	_mt7620_mii_write(gsw, 3, 30, 0xa000);

	_mt7620_mii_write(gsw, 0, 4, 0x05e1);
	_mt7620_mii_write(gsw, 1, 4, 0x05e1);
	_mt7620_mii_write(gsw, 2, 4, 0x05e1);
	_mt7620_mii_write(gsw, 3, 4, 0x05e1);
	_mt7620_mii_write(gsw, 1, 31, 0xa000); //local, page 2
	_mt7620_mii_write(gsw, 0, 16, 0x1111);
	_mt7620_mii_write(gsw, 1, 16, 0x1010);
	_mt7620_mii_write(gsw, 2, 16, 0x1515);
	_mt7620_mii_write(gsw, 3, 16, 0x0f0f);

	/* CPU Port6 Force Link 1G, FC ON */
	gsw_w32(gsw, 0x5e33b, GSW_REG_PORT_PMCR(6));
	/* Set Port6 CPU Port */
	gsw_w32(gsw, 0x7f7f7fe0, 0x0010);

//	GSW_VAWD2

	/* setup port 4 */
	if (gsw->port4 == PORT4_EPHY) {
		u32 val = rt_sysc_r32(SYSCFG1);
		val |= 3 << 14;
		rt_sysc_w32(val, SYSCFG1);
		_mt7620_mii_write(gsw, 4, 30, 0xa000);
		_mt7620_mii_write(gsw, 4, 4, 0x05e1);
		_mt7620_mii_write(gsw, 4, 16, 0x1313);
		pr_info("gsw: setting port4 to ephy mode\n");
	}
}

static int gsw_reset_switch(struct switch_dev *dev)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);

	gsw->global_vlan_enable = 0;
	memset(gsw->ports, 0, sizeof(gsw->ports));
	memset(gsw->vlans, 0, sizeof(gsw->vlans));
	gsw_hw_init(gsw);

	return 0;
}

static int gsw_get_vlan_enable(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);

	val->value.i = gsw->global_vlan_enable;

	return 0;
}

static int gsw_set_vlan_enable(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);

	gsw->global_vlan_enable = val->value.i != 0;

	return 0;
}

static unsigned gsw_get_pvid(struct mt7620_gsw *gsw, unsigned port)
{
	unsigned s, val;

	s = GSW_VTIM_S * (port % 2);
	val = gsw_r32(gsw, GSW_VTIM(port / 2));

	return (val >> s) & GSW_VTIM_M;
}

static void gsw_set_pvid(struct mt7620_gsw *gsw, unsigned port, unsigned pvid)
{
	unsigned s, val;

	s = GSW_VTIM_S * (port % 2);
	val = gsw_r32(gsw, GSW_VTIM(port / 2));
	val &= ~(GSW_VTIM_M << s);
	val |= (pvid && GSW_VTIM_M) << s;
	gsw_w32(gsw, val, GSW_VTIM(port / 2));
}

static int gsw_get_port_bool(struct switch_dev *dev,
			 const struct switch_attr *attr,
			 struct switch_val *val)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);
	int idx = val->port_vlan;

	if (idx < 0 || idx >= GSW_NUM_PORTS)
		return -EINVAL;

	switch (attr->id) {
	case GSW_ATTR_PORT_UNTAG:
		return gsw->ports[idx].untag;
	}

	return -EINVAL;
}

static int gsw_get_port_pvid(struct switch_dev *dev, int port, int *val)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);

	if (port >= GSW_NUM_PORTS)
		return -EINVAL;

	*val = gsw_get_pvid(gsw, port);

	return 0;
}

static int gsw_set_port_pvid(struct switch_dev *dev, int port, int val)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);

	if (port >= GSW_NUM_PORTS)
		return -EINVAL;

	gsw->ports[port].pvid = val;

	return 0;
}

static void gsw_set_vtcr(struct switch_dev *dev, u32 vid)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);
	int retry = 1000;

	gsw_w32(gsw, 0x80000000 | (BIT(vid) & GSW_VLAN_VTCR_VID_M), GSW_VLAN_VTCR);
	while (retry-- && (gsw_r32(gsw, GSW_VLAN_VTCR) & 0x80000000))
		;
}

static void gsw_apply_vtcr(struct switch_dev *dev, u32 vid)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);
	int retry = 1000;

	gsw_w32(gsw, 0x80001000 | (BIT(vid) & GSW_VLAN_VTCR_VID_M), GSW_VLAN_VTCR);
	while (retry-- && (gsw_r32(gsw, GSW_VLAN_VTCR) & 0x80000000))
		;
}

static unsigned gsw_get_vlan_id(struct mt7620_gsw *gsw, unsigned vlan)
{
	unsigned s;
	unsigned val;

	s = GSW_VLAN_ID_VID_S * (vlan % 2);
	val = gsw_r32(gsw, GSW_VLAN_ID(vlan / 2));
	val = (val >> s) & GSW_VLAN_ID_VID_M;

	return val;
}

static void gsw_set_vlan_id(struct mt7620_gsw *gsw, unsigned vlan, unsigned vid)
{
	unsigned s;
	unsigned val;

	s = GSW_VLAN_ID_VID_S * (vlan % 2);
	val = gsw_r32(gsw, GSW_VLAN_ID(vlan / 2));
	val &= ~(GSW_VLAN_ID_VID_M << s);
	val |= (vid << s);
	gsw_w32(gsw, val, GSW_VLAN_ID(vlan / 2));
}

static void gsw_vlan_tagging_enable(struct mt7620_gsw *gsw, unsigned vlan, unsigned enable)
{
	unsigned val;

	val = gsw_r32(gsw, GSW_VAWD1);
	if (enable)
		val |= GSW_VAWD1_VTAG_EN;
	else
		val &= ~GSW_VAWD1_VTAG_EN;
	gsw_w32(gsw, val, GSW_VAWD1);
}

static unsigned gsw_get_port_member(struct mt7620_gsw *gsw, unsigned vlan)
{
	unsigned val;

	gsw_set_vtcr(&gsw->swdev, vlan);

	val = gsw_r32(gsw, GSW_VAWD1);
	val = (val >> GSW_VAWD1_PORTM_S) & GSW_VAWD1_PORTM_M;

	return val;
}

static void gsw_set_port_member(struct mt7620_gsw *gsw, unsigned vlan, unsigned member)
{
	unsigned val;

	val = gsw_r32(gsw, GSW_VAWD1);
	val = ~(GSW_VAWD1_PORTM_M << GSW_VAWD1_PORTM_S);
	val |= (member & GSW_VAWD1_PORTM_M) << GSW_VAWD1_PORTM_S;
        gsw_w32(gsw, val, GSW_VAWD1);
}

static unsigned gsw_get_port_tag(struct mt7620_gsw *gsw, unsigned port)
{
	unsigned val;

	val = gsw_r32(gsw, GSW_REG_PCR(port));
	val >>= GSW_REG_PCR_EG_TAG_S;
	val &= GSW_REG_PCR_EG_TAG_M;

	return !!val;
}

static void gsw_set_port_untag(struct mt7620_gsw *gsw, unsigned port, unsigned untag)
{
	unsigned val;

	val = gsw_r32(gsw, GSW_REG_PCR(port));
	if (!untag)
		untag = 0x2;
	else
		untag = 0;
	val &= ~(GSW_REG_PCR_EG_TAG_M << GSW_REG_PCR_EG_TAG_S);
	val |= (untag & GSW_REG_PCR_EG_TAG_M) << GSW_REG_PCR_EG_TAG_S;
	gsw_w32(gsw, val, GSW_REG_PCR(port));
}

static int gsw_get_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);
	int vlan_idx = -1;
	u32 member;
	int i;

	val->len = 0;

	if (val->port_vlan < 0 || val->port_vlan >= GSW_NUM_VIDS)
		return -EINVAL;

	/* valid vlan? */
	for (i = 0; i < GSW_NUM_VLANS; i++) {
		if (gsw_get_vlan_id(gsw, i) != val->port_vlan)
			continue;
		member = gsw_get_port_member(gsw, i);
		vlan_idx = i;
		break;
	}

	if (vlan_idx == -1)
		return -EINVAL;

	for (i = 0; i < GSW_NUM_PORTS; i++) {
		struct switch_port *p;
		int port_mask = 1 << i;

		if (!(member & port_mask))
			continue;

		p = &val->value.ports[val->len++];
		p->id = i;
		if (gsw_get_port_tag(gsw, i))
			p->flags = 1 << SWITCH_PORT_FLAG_TAGGED;
		else
			p->flags = 0;
	}

	return 0;
}

static int gsw_set_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);
	int ports;
	int vlan_idx = -1;
	int i;

	if (val->port_vlan < 0 || val->port_vlan >= GSW_NUM_VIDS ||
			val->len > GSW_NUM_PORTS)
		return -EINVAL;

	/* one of the already defined vlans? */
	for (i = 0; i < GSW_NUM_VLANS; i++) {
		if (gsw->vlans[i].vid == val->port_vlan &&
		    gsw->vlans[i].ports) {
			vlan_idx = i;
			break;
		}
	}

	/* select a free slot */
	for (i = 0; vlan_idx == -1 && i < GSW_NUM_VLANS; i++) {
		if (!gsw->vlans[i].ports)
			vlan_idx = i;
	}

	/* bail if all slots are in use */
	if (vlan_idx == -1)
		return -EINVAL;

	ports = 0;
	for (i = 0; i < val->len; i++) {
		struct switch_port *p = &val->value.ports[i];
		int port_mask = 1 << p->id;
		bool untagged = !(p->flags & (1 << SWITCH_PORT_FLAG_TAGGED));

		if (p->id >= GSW_NUM_PORTS)
			return -EINVAL;

		ports |= port_mask;
		gsw->ports[p->id].untag = untagged;
	}
	gsw->vlans[vlan_idx].ports = ports;
	if (!ports)
		gsw->vlans[vlan_idx].vid = 0xfff;
	else
		gsw->vlans[vlan_idx].vid = val->port_vlan;

	return 0;
}

static int gsw_apply_config(struct switch_dev *dev)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);
	int i;

	for (i = 0; i < GSW_NUM_VLANS; i++) {
		gsw_set_vtcr(&gsw->swdev, i);
		if (gsw->global_vlan_enable) {
			gsw_set_vlan_id(gsw, i, gsw->vlans[i].vid);
			gsw_set_port_member(gsw, i, gsw->vlans[i].ports);
			gsw_vlan_tagging_enable(gsw, i, 1);
		} else {
			gsw_set_vlan_id(gsw, i, 0xfff);
			gsw_set_port_member(gsw, i, 0);
			gsw_vlan_tagging_enable(gsw, i, 0);
		}
		gsw_apply_vtcr(&gsw->swdev, i);
	}

	for (i = 0; i < GSW_NUM_PORTS; i++) {
		if (gsw->global_vlan_enable) {
			gsw_set_port_untag(gsw, i, !gsw->ports[i].untag);
			gsw_set_pvid(gsw, i, gsw->ports[i].pvid);
		} else {
			gsw_set_port_untag(gsw, i, 0);
			gsw_set_pvid(gsw, i, 0);
		}
	}

	if (!gsw->global_vlan_enable)
		gsw_set_vlan_id(gsw, 0, 0);

	return 0;
}

static int gsw_get_port_link(struct switch_dev *dev,
			 int port,
			 struct switch_port_link *link)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);
	u32 status;

	if (port < 0 || port >= GSW_NUM_PORTS)
		return -EINVAL;

	status = gsw_r32(gsw, GSW_REG_PORT_STATUS(port));
	link->link = status & 0x1;
	link->duplex = (status >> 1) & 1;

	switch ((status >> 2) & 0x3) {
	case 0:
		link->speed = SWITCH_PORT_SPEED_10;
		break;
	case 1:
		link->speed = SWITCH_PORT_SPEED_100;
		break;
	case 2:
	case 3: // forced gige speed can be 2 or 3
		link->speed = SWITCH_PORT_SPEED_1000;
		break;
	}

	return 0;
}

static int gsw_set_port_bool(struct switch_dev *dev,
			 const struct switch_attr *attr,
			 struct switch_val *val)
{
	struct mt7620_gsw *gsw = container_of(dev, struct mt7620_gsw, swdev);
	int idx = val->port_vlan;

	if (idx < 0 || idx >= GSW_NUM_PORTS ||
	    val->value.i < 0 || val->value.i > 1)
		return -EINVAL;

	switch (attr->id) {
	case GSW_ATTR_PORT_UNTAG:
		gsw->ports[idx].untag = val->value.i;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct switch_attr gsw_global[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "VLAN mode (1:enabled)",
		.max = 1,
		.id = GSW_ATTR_ENABLE_VLAN,
		.get = gsw_get_vlan_enable,
		.set = gsw_set_vlan_enable,
	},
};

static const struct switch_attr gsw_port[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "untag",
		.description = "Untag (1:strip outgoing vlan tag)",
		.max = 1,
		.id = GSW_ATTR_PORT_UNTAG,
		.get = gsw_get_port_bool,
		.set = gsw_set_port_bool,
	},
};

static const struct switch_attr gsw_vlan[] = {
};

static const struct switch_dev_ops gsw_ops = {
	.attr_global = {
		.attr = gsw_global,
		.n_attr = ARRAY_SIZE(gsw_global),
	},
	.attr_port = {
		.attr = gsw_port,
		.n_attr = ARRAY_SIZE(gsw_port),
	},
	.attr_vlan = {
		.attr = gsw_vlan,
		.n_attr = ARRAY_SIZE(gsw_vlan),
	},
	.get_vlan_ports = gsw_get_vlan_ports,
	.set_vlan_ports = gsw_set_vlan_ports,
	.get_port_pvid = gsw_get_port_pvid,
	.set_port_pvid = gsw_set_port_pvid,
	.get_port_link = gsw_get_port_link,
	.apply_config = gsw_apply_config,
	.reset_switch = gsw_reset_switch,
};

void mt7620_set_mac(struct fe_priv *priv, unsigned char *mac)
{
	struct mt7620_gsw *gsw = (struct mt7620_gsw *) priv->soc->swpriv;
	unsigned long flags;

	spin_lock_irqsave(&priv->page_lock, flags);
	gsw_w32(gsw, (mac[0] << 8) | mac[1], GSW_REG_SMACCR1);
	gsw_w32(gsw, (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5],
		GSW_REG_SMACCR0);
	spin_unlock_irqrestore(&priv->page_lock, flags);
}

static struct of_device_id gsw_match[] = {
	{ .compatible = "ralink,mt7620a-gsw" },
	{}
};

int mt7620_gsw_probe(struct fe_priv *priv)
{
	struct mt7620_gsw *gsw;
	struct device_node *np;
	struct switch_dev *swdev;
	const char *port4 = NULL;

	np = of_find_matching_node(NULL, gsw_match);
	if (!np) {
		dev_err(priv->device, "no gsw node found\n");
		return -EINVAL;
	}
	np = of_node_get(np);

	gsw = devm_kzalloc(priv->device, sizeof(struct mt7620_gsw), GFP_KERNEL);
	if (!gsw) {
		dev_err(priv->device, "no gsw memory for private data\n");
		return -ENOMEM;
	}

	gsw->irq = irq_of_parse_and_map(np, 0);
	if (!gsw->irq) {
		dev_err(priv->device, "no gsw irq resource found\n");
		return -ENOMEM;
	}

	gsw->base = of_iomap(np, 0);
	if (!gsw->base) {
		dev_err(priv->device, "gsw ioremap failed\n");
	}

	gsw->dev = priv->device;
	priv->soc->swpriv = gsw;

	swdev = &gsw->swdev;
	swdev->of_node = np;
	swdev->name = "mt7620a-gsw";
	swdev->alias = "mt7620x";
	swdev->cpu_port = GSW_PORT6;
	swdev->ports = GSW_NUM_PORTS;
	swdev->vlans = GSW_NUM_VLANS;
	swdev->ops = &gsw_ops;

	if (register_switch(swdev, NULL))
		dev_err(priv->device, "register_switch failed\n");

	of_property_read_string(np, "ralink,port4", &port4);
	if (port4 && !strcmp(port4, "ephy"))
		gsw->port4 = PORT4_EPHY;
	else if (port4 && !strcmp(port4, "gmac"))
		gsw->port4 = PORT4_EXT;
	else
		WARN_ON(port4);

	gsw_hw_init(gsw);

	gsw_w32(gsw, ~PORT_IRQ_ST_CHG, GSW_REG_IMR);
	request_irq(gsw->irq, gsw_interrupt, 0, "gsw", priv);

	return 0;
}
