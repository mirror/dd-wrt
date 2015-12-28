/*
 * Ethernet driver for the Kendin/Micrel KS8695.
 *
 * Copyright (C) 2006 Andrew Victor
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/mii.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/mach/irq.h>
#include <asm/uaccess.h>
#include <asm/arch/regs-wan.h>
#include <asm/arch/regs-lan.h>
#include <asm/arch/regs-hpna.h>
#include <asm/arch/regs-switch.h>
#include <asm/arch/regs-misc.h>

#include <asm/arch/regs-irq.h>

#include "ks8695_ether.h"


#define DRV_NAME	"ks8695_ether"
#define DRV_VERSION	"0.01"

static size_t _format_mac_addr(char *buf, int buflen,
				const unsigned char *addr, int len)
{
	int i;
	char *cp = buf;

	for (i = 0; i < len; i++) {
		cp += scnprintf(cp, buflen - (cp - buf), "%02x", addr[i]);
		if (i == len - 1)
			break;
		cp += strlcpy(cp, ":", buflen - (cp - buf));
	}
	return cp - buf;
}

char *print_mac(char *buf, const unsigned char *addr)
{
	_format_mac_addr(buf, MAC_BUF_SIZE, addr, ETH_ALEN);
	return buf;
}
EXPORT_SYMBOL(print_mac);

/* ..................................................................... */

static inline unsigned long ks8695_read(struct net_device *dev, unsigned int reg)
{
	return __raw_readl(dev->base_addr + reg);
}

static inline void ks8695_write(struct net_device *dev, unsigned int reg, unsigned long value)
{
	__raw_writel(value, dev->base_addr + reg);
}


/* ......................... ADDRESS MANAGEMENT ........................ */

#define KS8695_NR_ADDRESSES	16

/*
 * Add the specified multicast addresses to the Additional Station
 * Address registers.
 */
static void ks8695_set_mcast_address(struct net_device *dev, struct dev_mc_list *addr, int nr_addr)
{
	unsigned long low, high;
	int i;

	/* Set multicast addresses in Additional Station Address registers */
	for (i = 0; i < nr_addr; i++, addr = addr->next) {
		if (!addr) break;				/* unexpected end of list */
		else if (i == KS8695_NR_ADDRESSES) break;	/* too many addresses */

		low = (addr->dmi_addr[2] << 24) | (addr->dmi_addr[3] << 16) | (addr->dmi_addr[4] << 8) | (addr->dmi_addr[5]);
		high = (addr->dmi_addr[0] << 8) | (addr->dmi_addr[1]);

		ks8695_write(dev, KS8695_WMAAL_(i), low);
		ks8695_write(dev, KS8695_WMAAH_(i), WMAAH_E | high);
	}

	/* Clear the remaining Additional Station Addresses */
	for (; i < KS8695_NR_ADDRESSES; i++) {
		ks8695_write(dev, KS8695_WMAAL_(i), 0);
		ks8695_write(dev, KS8695_WMAAH_(i), 0);
	}
}

/*
 * Enable/Disable promiscuous and multicast modes.
 */
static void ks8695eth_set_multi(struct net_device *dev)
{
	unsigned long ctrl;

	ctrl = ks8695_read(dev, KS8695_WMDRXC);

	if (dev->flags & IFF_PROMISC)			/* enable promiscuous mode */
		ctrl |= WMDRXC_WMRA;
	else if (dev->flags & ~IFF_PROMISC)		/* disable promiscuous mode */
		ctrl &= ~WMDRXC_WMRA;

	if (dev->flags & IFF_ALLMULTI)			/* enable all multicast mode */
		ctrl |= WMDRXC_WMRM;
	else if (dev->mc_count > KS8695_NR_ADDRESSES)	/* more specific multicast addresses than can be handled in hardware */
		ctrl |= WMDRXC_WMRM;
	else if (dev->mc_count > 0) {			/* enable specific multicasts */
		ctrl &= ~WMDRXC_WMRM;
		ks8695_set_mcast_address(dev, dev->mc_list, dev->mc_count);
	}
	else if (dev->flags & ~IFF_ALLMULTI) {		/* disable multicast mode */
		ctrl &= ~WMDRXC_WMRM;
		ks8695_set_mcast_address(dev, NULL, 0);
	}

	ks8695_write(dev, KS8695_WMDRXC, ctrl);
}

/*
 * Program the hardware MAC address from dev->dev_addr.
 */
static void update_mac_address(struct net_device *dev)
{
	unsigned long low, high;

	low = (dev->dev_addr[2] << 24) | (dev->dev_addr[3] << 16) | (dev->dev_addr[4] << 8) | (dev->dev_addr[5]);
	high = (dev->dev_addr[0] << 8) | (dev->dev_addr[1]);

	ks8695_write(dev, KS8695_WMAL, low);
	ks8695_write(dev, KS8695_WMAH, high);
}

/*
 * Store the new hardware address in dev->dev_addr, and update the MAC.
 */
static int ks8695eth_set_mac(struct net_device *dev, void* addr)
{
	struct sockaddr *address = addr;
	DECLARE_MAC_BUF(mac);

	if (!is_valid_ether_addr(address->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(dev->dev_addr, address->sa_data, dev->addr_len);
	update_mac_address(dev);

	printk("%s: Setting MAC address to %s\n", dev->name, print_mac(mac, dev->dev_addr));

	return 0;
}

/*
 * Retrieve the MAC address set by the bootloader.
 */
static void __init get_mac_address(struct net_device *dev)
{
	unsigned char addr[6];
	unsigned long low, high;

	low = ks8695_read(dev, KS8695_WMAL);
	high = ks8695_read(dev, KS8695_WMAH);

	addr[0] = (high & 0xff00) >> 8;
	addr[1] = (high & 0xff);
	addr[2] = (low & 0xff000000) >> 24;
	addr[3] = (low & 0xff0000) >> 16;
	addr[4] = (low & 0xff00) >> 8;
	addr[5] = (low & 0xff);

	if (is_valid_ether_addr(addr))
		memcpy(dev->dev_addr, &addr, 6);
}


/* ......................... ETHTOOL SUPPORT ........................... */

/*
 * Get device-specific settings.
 */
static int ks8695eth_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	unsigned long ctrl;

	/* the defaults for all ports */
	cmd->supported = SUPPORTED_10baseT_Half | SUPPORTED_10baseT_Full
			| SUPPORTED_100baseT_Half | SUPPORTED_100baseT_Full
			| SUPPORTED_TP | SUPPORTED_MII;
	cmd->advertising = ADVERTISED_TP | ADVERTISED_MII;
	cmd->port = PORT_MII;
	cmd->transceiver = XCVR_INTERNAL;

	if (dev->base_addr == KS8695_HPNA_VA) {
		cmd->phy_address = 0;
		cmd->autoneg = AUTONEG_DISABLE;		/* not supported for HPNA */

		ctrl = __raw_readl(KS8695_MISC_VA + KS8695_HMC);
		cmd->speed = (ctrl & HMC_HSS) ? SPEED_100 : SPEED_10;
		cmd->duplex = (ctrl & HMC_HDS) ? DUPLEX_FULL : DUPLEX_HALF;
	}
	else if (dev->base_addr == KS8695_WAN_VA) {
		cmd->supported |= (SUPPORTED_Autoneg | SUPPORTED_Pause);
		cmd->phy_address = 0;

		ctrl = __raw_readl(KS8695_MISC_VA + KS8695_WMC);
		if ((ctrl & WMC_WAND) == 0) {		/* auto-negotiation is enabled */
			cmd->advertising |= ADVERTISED_Autoneg;
			if (ctrl & WMC_WANA100F)
				cmd->advertising |= ADVERTISED_100baseT_Full;
			if (ctrl & WMC_WANA100H)
				cmd->advertising |= ADVERTISED_100baseT_Half;
			if (ctrl & WMC_WANA10F)
				cmd->advertising |= ADVERTISED_10baseT_Full;
			if (ctrl & WMC_WANA10H)
				cmd->advertising |= ADVERTISED_10baseT_Half;
			if (ctrl & WMC_WANAP)
				cmd->advertising |= ADVERTISED_Pause;
			cmd->autoneg = AUTONEG_ENABLE;

			cmd->speed = (ctrl & WMC_WSS) ? SPEED_100 : SPEED_10;
			cmd->duplex = (ctrl & WMC_WDS) ? DUPLEX_FULL : DUPLEX_HALF;
		}
		else {					/* auto-negotiation is disabled */
			cmd->autoneg = AUTONEG_DISABLE;

			cmd->speed = (ctrl & WMC_WANF100) ? SPEED_100 : SPEED_10;
			cmd->duplex = (ctrl & WMC_WANFF) ? DUPLEX_FULL : DUPLEX_HALF;
		}
	}
	else if (dev->base_addr == KS8695_LAN_VA) {
		// TODO: Implement for Switch ports
	}

	return 0;
}

/*
 * Set device-specific settings.
 */
static int ks8695eth_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	unsigned long ctrl;

	if ((cmd->speed != SPEED_10) && (cmd->speed != SPEED_100))
		return -EINVAL;
	if ((cmd->duplex != DUPLEX_HALF) && (cmd->duplex != DUPLEX_FULL))
		return -EINVAL;
	if (cmd->port != PORT_MII)
		return -EINVAL;
	if (cmd->transceiver != XCVR_INTERNAL)
		return -EINVAL;
	if ((cmd->autoneg != AUTONEG_DISABLE) && (cmd->autoneg != AUTONEG_ENABLE))
		return -EINVAL;

	if (cmd->autoneg == AUTONEG_ENABLE) {
		if ((cmd->advertising & (ADVERTISED_10baseT_Half |
				ADVERTISED_10baseT_Full |
				ADVERTISED_100baseT_Half |
				ADVERTISED_100baseT_Full)) == 0)
			return -EINVAL;

		if (dev->base_addr == KS8695_HPNA_VA)
			return -EINVAL;		/* HPNA does not support auto-negotiation. */
		else if (dev->base_addr == KS8695_WAN_VA) {
			ctrl = __raw_readl(KS8695_MISC_VA + KS8695_WMC);

			ctrl &= ~(WMC_WAND | WMC_WANA100F | WMC_WANA100H | WMC_WANA10F | WMC_WANA10H);
			if (cmd->advertising & ADVERTISED_100baseT_Full)
				ctrl |= WMC_WANA100F;
			if (cmd->advertising & ADVERTISED_100baseT_Half)
				ctrl |= WMC_WANA100H;
			if (cmd->advertising & ADVERTISED_10baseT_Full)
				ctrl |= WMC_WANA10F;
			if (cmd->advertising & ADVERTISED_10baseT_Half)
				ctrl |= WMC_WANA10H;

			ctrl |= WMC_WANR;	/* force a re-negotiation */
			__raw_writel(ctrl, KS8695_MISC_VA + KS8695_WMC);
		}
		else if (dev->base_addr == KS8695_LAN_VA) {
			// TODO: Implement for Switch ports
		}

	}
	else {
		if (dev->base_addr == KS8695_HPNA_VA) {
			ctrl = __raw_readl(KS8695_MISC_VA + KS8695_HMC);

			ctrl &= ~(HMC_HSS | HMC_HDS);
			if (cmd->speed == SPEED_100)
				ctrl |= HMC_HSS;
			if (cmd->duplex == DUPLEX_FULL)
				ctrl |= HMC_HDS;

			__raw_writel(ctrl, KS8695_MISC_VA + KS8695_HMC);
		}
		else if (dev->base_addr == KS8695_WAN_VA) {
			ctrl = __raw_readl(KS8695_MISC_VA + KS8695_WMC);

			ctrl |= WMC_WAND;	/* disable auto-negotiation */
			ctrl &= ~(WMC_WANF100 | WMC_WANFF);
			if (cmd->speed == SPEED_100)
				ctrl |= WMC_WANF100;
			if (cmd->duplex == DUPLEX_FULL)
				ctrl |= WMC_WANFF;

			__raw_writel(ctrl, KS8695_MISC_VA + KS8695_WMC);
		}
		else if (dev->base_addr == KS8695_LAN_VA) {
			// TODO: Implement for Switch ports
		}
	}

	return 0;
}

/*
 * Restart the auto-negotiation.
 */
static int ks8695eth_nwayreset(struct net_device *dev)
{
	unsigned long ctrl;

	if (dev->base_addr == KS8695_HPNA_VA)	/* HPNA has no auto-negotiation */
		return -EINVAL;
	else if (dev->base_addr == KS8695_WAN_VA) {
		ctrl = __raw_readl(KS8695_MISC_VA + KS8695_WMC);

		if ((ctrl & WMC_WAND) == 0)
			__raw_writel(ctrl | WMC_WANR, KS8695_MISC_VA + KS8695_WMC);
		else
			return -EINVAL;		/* auto-negitiation not enabled */
	}
	else if (dev->base_addr == KS8695_LAN_VA) {
		// TODO: Implement for Switch ports
	}

	return 0;
}

static void ks8695eth_get_pause(struct net_device *dev, struct ethtool_pauseparam *param)
{
	unsigned long ctrl;

	if (dev->base_addr == KS8695_HPNA_VA)
		return;
	else if (dev->base_addr == KS8695_WAN_VA) {
		ctrl = __raw_readl(KS8695_MISC_VA + KS8695_WMC);	/* advertise Pause */
		param->autoneg = (ctrl & WMC_WANAP);

		ctrl = ks8695_read(dev, KS8695_WMDRXC);			/* current Tx Flow-control */
		param->rx_pause = (ctrl & WMDRXC_WMRFCE);

		ctrl = ks8695_read(dev, KS8695_WMDRXC);			/* current Rx Flow-control */
		param->tx_pause = (ctrl & WMDTXC_WMTFCE);
	}
	else if (dev->base_addr == KS8695_LAN_VA) {
		// TODO: Implement for Switch ports
	}
}

static int ks8695eth_set_pause(struct net_device *dev, struct ethtool_pauseparam *param)
{
	// TODO.

	return 0;
}

static u32 ks8695eth_get_link(struct net_device *dev)
{
	unsigned long ctrl;

	if (dev->base_addr == KS8695_HPNA_VA)
		return 1;		/* HPNA always has link */
	else if (dev->base_addr == KS8695_WAN_VA) {
		ctrl = __raw_readl(KS8695_MISC_VA + KS8695_WMC);
		return (ctrl & WMC_WLS);
	}
	else if (dev->base_addr == KS8695_LAN_VA) {
		// TODO: Implement for Switch ports
	}

	return 0;
}

/*
 * Report driver information.
 */
static void ks8695eth_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, DRV_NAME, sizeof(info->driver));
	strlcpy(info->version, DRV_VERSION, sizeof(info->version));
	strlcpy(info->bus_info, dev->dev.parent->bus_id, sizeof(info->bus_info));
}

static struct ethtool_ops ks8695eth_ethtool_ops = {
	.get_settings	= ks8695eth_get_settings,
	.set_settings	= ks8695eth_set_settings,
	.get_drvinfo	= ks8695eth_get_drvinfo,
	.nway_reset	= ks8695eth_nwayreset,
	.get_pauseparam = ks8695eth_get_pause,
	.set_pauseparam = ks8695eth_set_pause,
	.get_link	= ks8695eth_get_link,
};


/* ................................ MAC ................................ */

/*
 * Setup the RX DMA descriptors, and enable and start the DMA receiver.
 */
static void ks8695eth_start_rx(struct net_device *dev)
{
	struct ks8695eth_priv *lp = (struct ks8695eth_priv *) dev->priv;
	unsigned long ctrl;
	int i;

	/* Setup the DMA descriptors */
	for (i = 0; i < MAX_RX_DESC; i++) {
		lp->rxdma[i].length = MAX_RXBUF_SIZE;
		lp->rxdma[i].addr = (unsigned long) lp->rxSkb[i].dma;
		lp->rxdma[i].next = (unsigned long) lp->rxdma_phys + (sizeof(struct rx_descriptor) * (i+1));
		lp->rxdma[i].status = RDES_OWN;
	}

	/* Create ring of DMA descriptors */
	lp->rxdma[MAX_RX_DESC-1].next = (unsigned long) lp->rxdma_phys;		/* phys address of 1st descriptor */

	/* Reset receive index (since hardware was reset) */
	lp->rx_idx = 0;

	/* Program address of 1st descriptor in KS8695 */
	ks8695_write(dev, KS8695_WRDLB, (unsigned long) lp->rxdma_phys);

	/* Enable and start the DMA Receiver */
	ctrl = ks8695_read(dev, KS8695_WMDRXC);
	ks8695_write(dev, KS8695_WMDRXC, ctrl | WMDRXC_WMRE);
	ks8695_write(dev, KS8695_WMDRSC, 0);
}

/*
 * Stop the DMA receiver.
 */
static void ks8695eth_stop_rx(struct net_device *dev)
{
	unsigned long ctrl;

	/* Disable receive DMA */
	ctrl = ks8695_read(dev, KS8695_WMDRXC);
	ks8695_write(dev, KS8695_WMDRXC, ctrl & ~WMDRXC_WMRE);
}

/*
 * Setup the TX DMA descriptors, and enable DMA transmitter.
 */
static void ks8695eth_start_tx(struct net_device *dev)
{
	struct ks8695eth_priv *lp = (struct ks8695eth_priv *) dev->priv;
	unsigned long ctrl;
	int i;

	/* Setup the DMA descriptors */
	for (i = 0; i < MAX_TX_DESC; i++) {
		lp->txdma[i].ownership = 0;
		lp->txdma[i].status = 0;
		lp->txdma[i].addr = 0;
		lp->txdma[i].next = (unsigned long) lp->txdma_phys + (sizeof(struct tx_descriptor) * (i+1));
	}

	/* Create ring of DMA descriptors */
	lp->txdma[MAX_TX_DESC-1].next = (unsigned long) lp->txdma_phys;		/* phys address of 1st desc */

	/* Reset transmit indexes (since hardware was reset) */
	lp->tx_head = 0;
	lp->tx_tail = 0;

	/* Program address of 1st descriptor in KS8695 */
	ks8695_write(dev, KS8695_WTDLB, (unsigned long) lp->txdma_phys);

	/* Enable the DMA transmitter (will be started on first packet) */
	ctrl = ks8695_read(dev, KS8695_WMDTXC);
	ks8695_write(dev, KS8695_WMDTXC, ctrl | WMDTXC_WMTE);
}

/*
 * Stop the DMA transmitter.
 */
static void ks8695eth_stop_tx(struct net_device *dev)
{
	struct ks8695eth_priv *lp = (struct ks8695eth_priv *) dev->priv;
	unsigned long ctrl;
	int i;

	/* Disable transmit DMA */
	ctrl = ks8695_read(dev, KS8695_WMDTXC);
	ks8695_write(dev, KS8695_WMDTXC, ctrl & ~WMDTXC_WMTE);

	/* Clear any pending skb's still on transmit queue */
	for (i = 0; i < MAX_TX_DESC; i++) {
		lp->txdma[i].ownership = 0;
		lp->txdma[i].status = 0;
		lp->txdma[i].addr = 0;

		if (lp->txSkb[i].skb) {
			dma_unmap_single(lp->dev, lp->txSkb[i].dma, lp->txSkb[i].length, DMA_TO_DEVICE);
			dev_kfree_skb_irq(lp->txSkb[i].skb);
			lp->txSkb[i].skb = NULL;
		}
	}
}

/*
 * Reset the MAC hardware.
 */
static void ks8695eth_hw_reset(struct net_device *dev)
{
	/* Perform hardware reset */
	ks8695_write(dev, KS8695_WMDTXC, WMDTXC_WMTRST);
	while (ks8695_read(dev, KS8695_WMDTXC) & WMDTXC_WMTRST) { barrier(); }

	/* Initialize the hardware */
	ks8695_write(dev, KS8695_WMDRXC, WMDRXC_WMRU | WMDRXC_WMRB);		/* RX: receive Unicast & Broadcast */
	ks8695_write(dev, KS8695_WMDTXC, WMDTXC_WMTEP | WMDTXC_WMTAC);		/* TX: add Padding & CRC */

	// TODO: Can set Rx/Tx PBL: (Micrel using 8)
	// TODO: Enable hardware checksumming.
	// TODO: Enable Rx/Tx flow-control
}

/*
 * Enable or Disable the IRQs associated with a network interface.
 */
static void ks8695eth_set_irq(struct net_device *dev, short enable)
{
	struct ks8695eth_priv *lp = (struct ks8695eth_priv *) dev->priv;
	int i;

	for (i = 0; i < NR_IRQS; i++) {
		if (lp->irqs & (1 << i)) {
			if (enable)
				enable_irq(i);
			else
				disable_irq(i);
		}
	}
}

/*
 * Open the ethernet interface.
 */
static int ks8695eth_open(struct net_device *dev)
{
	if (!is_valid_ether_addr(dev->dev_addr))
		return -EADDRNOTAVAIL;

	/* MUST reset hardware in _open() */
	ks8695eth_hw_reset(dev);

	/*  Update the MAC address (incase user has changed it) */
	update_mac_address(dev);

	/* Start DMA */
	ks8695eth_start_tx(dev);
	ks8695eth_start_rx(dev);

	/* Enable interrupts */
	ks8695eth_set_irq(dev, 1);

	netif_start_queue(dev);
	return 0;
}

/*
 * Close the ethernet interface.
 */
static int ks8695eth_close(struct net_device *dev)
{
	/* Stop DMA */
	ks8695eth_stop_rx(dev);
	ks8695eth_stop_tx(dev);

	/* Disable interrupts */
	ks8695eth_set_irq(dev, 0);

	netif_stop_queue(dev);
	return 0;
}

/*
 * Return the current statistics.
 */
static struct net_device_stats *ks8695eth_stats(struct net_device *dev)
{
	struct ks8695eth_priv *adap = (struct ks8695eth_priv *) dev->priv;

	return &adap->stats;
}

/*
 * Queue a packet for transmission in next TX DMA descriptor.
 */
static int ks8695eth_xmit_frame(struct sk_buff *skb, struct net_device *dev)
{
	struct ks8695eth_priv *lp = (struct ks8695eth_priv *) dev->priv;
	int i;

	/* Packets are added to head of array */
	i = lp->tx_head;

	/* Store packet information */
	lp->txSkb[i].skb = skb;
	lp->txSkb[i].length = skb->len;
	lp->txSkb[i].dma = dma_map_single(lp->dev, skb->data, skb->len, DMA_TO_DEVICE);

	spin_lock_irq(&lp->tx_lock);

	/* Set Tx descriptor information */
	lp->txdma[i].addr = lp->txSkb[i].dma;
	lp->txdma[i].status = TDES_IC | TDES_FS | TDES_LS | (lp->txSkb[i].length & TDES_TBS);
	lp->txdma[i].ownership = TDES_OWN;

	/* Start the DMA transmitter (if necessary) */
	ks8695_write(dev, KS8695_WMDTSC, 0);

	lp->tx_head = (lp->tx_head + 1) % MAX_TX_DESC;
	if (lp->tx_head == lp->tx_tail)		/* no more descriptors */
		netif_stop_queue(dev);

	spin_unlock_irq(&lp->tx_lock);

	dev->trans_start = jiffies;
	return 0;
}

/* ..................................................................... */

/*
 * The link state of the WAN port has changed.
 * (Called from interrupt context)
 */
static void ks8695eth_wan_link(struct net_device *dev)
{
	unsigned long ctrl;

	ctrl = __raw_readl(KS8695_MISC_VA + KS8695_WMC);
	if (ctrl & WMC_WLS) {
		netif_carrier_on(dev);
		printk(KERN_INFO "%s: Link is now %s-%s\n", dev->name,
				(ctrl & WMC_WSS) ? "100" : "10",
				(ctrl & WMC_WDS) ? "FullDuplex" : "HalfDuplex");
	}
	else {
		netif_carrier_off(dev);
		printk(KERN_INFO "%s: Link down.\n", dev->name);
	}
}

/* ..................................................................... */

/*
 * A frame has been received.  Exteract from buffer descriptor and deliver to
 * upper layers.
 * (Called from interrupt context)
 */
static void ks8695eth_rx_interrupt(struct net_device *dev)
{
	struct ks8695eth_priv *lp = (struct ks8695eth_priv *) dev->priv;
	struct sk_buff *skb;
	unsigned long flags;
	unsigned int pktlen;

	while (!(lp->rxdma[lp->rx_idx].status & RDES_OWN)) {
		flags = lp->rxdma[lp->rx_idx].status;

		if ((flags & (RDES_FS | RDES_LS)) != (RDES_FS | RDES_LS)) {
			printk(KERN_ERR "%s: Spanning packet detected\n", dev->name);
			goto rx_complete;
		}

		/* handle errors */
		if (flags & (RDES_ES | RDES_RE)) {
			lp->stats.rx_errors++;

			if (flags & RDES_TL)		/* Frame too long */
				lp->stats.rx_length_errors++;
			else if (flags & RDES_RF)	/* Runt frame */
				lp->stats.rx_length_errors++;
			else if (flags & RDES_CE)	/* CRC error */
				lp->stats.rx_crc_errors++;
			else if (flags & RDES_RE)	/* MII error */
				lp->stats.rx_missed_errors++;
			// TODO: If hardware checksumming, then check IP/TCP/UDP errors.

			goto rx_complete;
		}

		pktlen = flags & RDES_FLEN;
		pktlen = pktlen - 4;			/* remove CRC */

		// OLD CALL: consistent_sync(lp->rxSkb[lp->rx_idx].skb->data, MAX_RXBUF_SIZE, DMA_FROM_DEVICE);
		dma_sync_single_for_cpu(lp->dev, lp->rxSkb[lp->rx_idx].dma, MAX_RXBUF_SIZE, DMA_FROM_DEVICE);

		skb = dev_alloc_skb(pktlen+2);		/* +2 to align IP header */
		if (!skb) {
			lp->stats.rx_dropped++;
			printk(KERN_NOTICE "%s: Memory squeeze, dropping packet.\n", dev->name);
			goto rx_complete;
		}

		skb_reserve(skb, 2);			/* align IP header */
		memcpy(skb_put(skb, pktlen), lp->rxSkb[lp->rx_idx].skb->data, pktlen);

		skb->protocol = eth_type_trans(skb, dev);
		netif_rx(skb);

		/* update statistics */
		lp->stats.rx_packets++;
		lp->stats.rx_bytes += pktlen;
		if (flags & RDES_MF)
			lp->stats.multicast++;
		dev->last_rx = jiffies;

rx_complete:
		lp->rxdma[lp->rx_idx].status = RDES_OWN;	/* reset ownership bit */

		lp->rx_idx = (lp->rx_idx + 1) % MAX_RX_DESC;	/* next descriptor */
	}

	/* restart DMA receiver incase it was suspended */
	ks8695_write(dev, KS8695_WMDRSC, 0);
}

/*
 * A packet has been transmitted.
 * (Called from interrupt context)
 */
static void ks8695eth_tx_interrupt(struct net_device *dev)
{
	struct ks8695eth_priv *lp = (struct ks8695eth_priv *) dev->priv;
	int i;

	/* Packets are removed from tail of array */
	i = lp->tx_tail;

	// TODO: Loop through multiple times?

	if (lp->txSkb[i].skb) {
		/* update statistics */
		lp->stats.tx_packets++;
		lp->stats.tx_bytes += lp->txSkb[i].length;

		/* free packet */
		dma_unmap_single(lp->dev, lp->txSkb[i].dma, lp->txSkb[i].length, DMA_TO_DEVICE);
		dev_kfree_skb_irq(lp->txSkb[i].skb);
		lp->txSkb[i].skb = NULL;

		/* Not necessary to clear descriptor since we still own it */
	}

	lp->tx_tail = (lp->tx_tail + 1) % MAX_TX_DESC;

	netif_wake_queue(dev);
}

/*
 * MAC interrupt handler
 */
static irqreturn_t ks8695eth_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *) dev_id;

	switch (irq) {
		case KS8695_IRQ_LAN_RX_STATUS:
		case KS8695_IRQ_HPNA_RX_STATUS:
		case KS8695_IRQ_WAN_RX_STATUS:
			ks8695eth_rx_interrupt(dev);
			return IRQ_HANDLED;

		case KS8695_IRQ_LAN_TX_STATUS:
		case KS8695_IRQ_HPNA_TX_STATUS:
		case KS8695_IRQ_WAN_TX_STATUS:
			ks8695eth_tx_interrupt(dev);
			return IRQ_HANDLED;

		case KS8695_IRQ_WAN_LINK:
			ks8695eth_wan_link(dev);
			return IRQ_HANDLED;

		default:
			return IRQ_NONE;
	}
}


/* ..................................................................... */

/*
 * Initialize the WAN hardware to known defaults.
 */
static void __init ks8695eth_init_wan(void)
{
	unsigned long ctrl;

	/* Support auto-negotiation */
	ctrl = WMC_WANAP | WMC_WANA100F | WMC_WANA100H | WMC_WANA10F | WMC_WANA10H;

	/* LED0 = Speed , LED1 = Link/Activity */
	ctrl |= (WLED0S_SPEED | WLED1S_LINK_ACTIVITY);

	/* Restart Auto-negotiation */
	ctrl |= WMC_WANR;

	__raw_writel(ctrl, KS8695_MISC_VA + KS8695_WMC);

	__raw_writel(0, KS8695_MISC_VA + KS8695_WPPM);
	__raw_writel(0, KS8695_MISC_VA + KS8695_PPS);
}

/*
 * Initialize the LAN Switch hardware to known defaults.
 */
static void __init ks8695eth_init_switch(void)
{
	unsigned long ctrl;

	ctrl = 0x40819e00;		/* default */

	/* LED0 = Speed  LED1 = Link/Activity */
	ctrl &= ~(SEC0_LLED1S | SEC0_LLED0S);
	ctrl |= (LLED0S_SPEED | LLED1S_LINK_ACTIVITY);

	/* Enable Switch */
	ctrl |= SEC0_ENABLE;

	__raw_writel(ctrl, KS8695_SWITCH_VA + KS8695_SEC0);

	__raw_writel(0x9400100, KS8695_SWITCH_VA + KS8695_SEC1);	/* reset defaults */
}

static int ks8695eth_hook_irqs(struct platform_device *pdev, struct net_device *dev, unsigned long *irqset)
{
	struct resource *res;
	int i = 0, ret;

	while ((res = platform_get_resource(pdev, IORESOURCE_IRQ, i))) {
		set_irq_flags(res->start, IRQF_VALID | IRQF_NOAUTOEN);

		ret = request_irq(res->start, ks8695eth_interrupt, IRQF_DISABLED | IRQF_SHARED, res->name, dev);
		if (ret) {
			printk(KERN_ERR "%s: return_irq %u failed\n", dev->name, res->start);
			return -EBUSY;
		}

		*irqset |= (1 << res->start);

		// TODO: Can set different priorities for interrupts [0x  BB AA FF].

		i++;
	}

	return 0;
}

static int __init ks8695eth_probe(struct platform_device *pdev)
{
	struct net_device *dev;
	struct ks8695eth_priv *lp;
	struct resource *res;
	int i = 0, ret, size;
	DECLARE_MAC_BUF(mac);

	/* Create ethernet device */
	dev = alloc_etherdev(sizeof(struct ks8695eth_priv));
	if (!dev)
		return -ENOMEM;

	/* Get I/O base address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		free_netdev(dev);
		return -ENODEV;
	}
	dev->base_addr = res->start;

	lp = (struct ks8695eth_priv *) dev->priv;

	/* Retreive MAC address before the MAC registers are reset */
	get_mac_address(dev);

	/* Reset the hardware */
	ks8695_write(dev, KS8695_WMDTXC, WMDTXC_WMTRST);
	while (ks8695_read(dev, KS8695_WMDTXC) & WMDTXC_WMTRST) { barrier(); }

	/* Get IRQ's */
	dev->irq = platform_get_irq(pdev, 0);
	ret = ks8695eth_hook_irqs(pdev, dev, &lp->irqs);
	if (ret) {
		// Cleanup.
	}

	/* Allocate DMA-able memory for Tx descriptor */
	size = sizeof(struct tx_descriptor) * MAX_TX_DESC;
	lp->txdma = dma_alloc_coherent(&pdev->dev, size, &lp->txdma_phys, GFP_KERNEL);
	if (lp->txdma == NULL) {
		// free IRQs
		free_netdev(dev);
		return -ENOMEM;
	}
	memset(lp->txdma, 0, size);
	lp->tx_head = 0;
	lp->tx_tail = 0;

	/* Allocate DMA-able memory for Rx descriptor */
	size = sizeof(struct rx_descriptor) * MAX_RX_DESC;
	lp->rxdma = dma_alloc_coherent(&pdev->dev, size, &lp->rxdma_phys, GFP_KERNEL);
	if (lp->rxdma == NULL) {
		// free IRQs
		// Free TX descriptor memory.
		free_netdev(dev);
		return -ENOMEM;
	}
	memset(lp->rxdma, 0, size);
	lp->rx_idx = 0;

	/* Allocate DMA-able memory for Rx Data */
	for (i = 0; i < MAX_RX_DESC; i++) {
		lp->rxSkb[i].skb = alloc_skb(MAX_RXBUF_SIZE, GFP_KERNEL);
		if (lp->rxSkb[i].skb == NULL) {
			// Cleanup
			return -ENOMEM;
		}
		lp->rxSkb[i].length = MAX_RXBUF_SIZE;
		lp->rxSkb[i].dma = dma_map_single(&pdev->dev, lp->rxSkb[i].skb->data, MAX_RXBUF_SIZE, DMA_FROM_DEVICE);
	}

	spin_lock_init(&lp->tx_lock);

	platform_set_drvdata(pdev, dev);

	ether_setup(dev);
	dev->open		= ks8695eth_open;
	dev->stop		= ks8695eth_close;
	dev->hard_start_xmit	= ks8695eth_xmit_frame;
	dev->get_stats		= ks8695eth_stats;
	dev->set_multicast_list	= ks8695eth_set_multi;
	dev->set_mac_address	= ks8695eth_set_mac;
	dev->ethtool_ops	= &ks8695eth_ethtool_ops;

	SET_NETDEV_DEV(dev, &pdev->dev);
	lp->dev = &pdev->dev;

	if (dev->base_addr == KS8695_WAN_VA)
		ks8695eth_init_wan();
	else if (dev->base_addr == KS8695_LAN_VA)
		ks8695eth_init_switch();

	/* Register the network interface */
	ret = register_netdev(dev);
	if (ret) {
		// free IRQs
		free_netdev(dev);
//		dma_free_coherent(&pdev->dev, sizeof(struct ks8695_tx_dma), lp->txdma, lp->txdma_phys);
//		dma_free_coherent(&pdev->dev, sizeof(struct ks8695_rx_dma), lp->rxdma, lp->rxdma_phys);
		return ret;
	}

	printk(KERN_INFO "%s: KS8695 ethernet (%s)\n", dev->name, print_mac(mac, dev->dev_addr));

	return 0;
}

static int __devexit ks8695eth_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
//	struct ks8695eth_priv *lp = (struct ks8695eth_priv *) dev->priv;

	unregister_netdev(dev);

	// Free IRQ
//	dma_free_coherent(&pdev->dev, sizeof(struct ks8695_tx_dma), lp->txdma, lp->txdma_phys);
//	dma_free_coherent(&pdev->dev, sizeof(struct ks8695_rx_dma), lp->rxdma, lp->rxdma_phys);

	platform_set_drvdata(pdev, NULL);
	free_netdev(dev);
	return 0;
}

static struct platform_driver ks8695ether_driver = {
	.probe		= ks8695eth_probe,
	.remove		= __devexit_p(ks8695eth_remove),
//	.suspend	=
//	.resume		=
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};


static int __init ks8695eth_init(void)
{
	return platform_driver_register(&ks8695ether_driver);
}

static void __exit ks8695eth_exit(void)
{
	platform_driver_unregister(&ks8695ether_driver);
}

module_init(ks8695eth_init);
module_exit(ks8695eth_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KS8695 Ethernet driver");
MODULE_AUTHOR("Andrew Victor");
