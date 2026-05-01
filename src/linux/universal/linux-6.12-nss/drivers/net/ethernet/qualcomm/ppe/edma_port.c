// SPDX-License-Identifier: GPL-2.0-only
 /* Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
  */

/* EDMA port initialization, configuration and netdevice ops handling */

#include <linux/etherdevice.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/of_net.h>
#include <linux/phylink.h>
#include <linux/printk.h>

#include "edma.h"
#include "edma_cfg_rx.h"
#include "edma_cfg_tx.h"
#include "edma_port.h"
#include "ppe_regs.h"

/* Number of netdev queues. */
#define EDMA_NETDEV_QUEUE_NUM	4

static int edma_port_stats_alloc(struct net_device *netdev)
{
	struct edma_port_priv *port_priv = (struct edma_port_priv *)netdev_priv(netdev);

	if (!port_priv)
		return -EINVAL;

	/* Allocate per-cpu stats memory. */
	port_priv->pcpu_stats.rx_stats =
		netdev_alloc_pcpu_stats(struct edma_port_rx_stats);
	if (!port_priv->pcpu_stats.rx_stats) {
		netdev_err(netdev, "Per-cpu EDMA Rx stats alloc failed for %s\n",
			   netdev->name);
		return -ENOMEM;
	}

	port_priv->pcpu_stats.tx_stats =
		netdev_alloc_pcpu_stats(struct edma_port_tx_stats);
	if (!port_priv->pcpu_stats.tx_stats) {
		netdev_err(netdev, "Per-cpu EDMA Tx stats alloc failed for %s\n",
			   netdev->name);
		free_percpu(port_priv->pcpu_stats.rx_stats);
		return -ENOMEM;
	}

	return 0;
}

static void edma_port_stats_free(struct net_device *netdev)
{
	struct edma_port_priv *port_priv = (struct edma_port_priv *)netdev_priv(netdev);

	free_percpu(port_priv->pcpu_stats.rx_stats);
	free_percpu(port_priv->pcpu_stats.tx_stats);
}

static void edma_port_configure(struct net_device *netdev)
{
	struct edma_port_priv *port_priv = (struct edma_port_priv *)netdev_priv(netdev);
	struct ppe_port *port =  port_priv->ppe_port;
	int port_id = port->port_id;

	edma_cfg_tx_fill_per_port_tx_map(netdev, port_id);
	edma_cfg_tx_rings_enable(port_id);
	edma_cfg_tx_napi_add(netdev, port_id);
}

static void edma_port_deconfigure(struct net_device *netdev)
{
	struct edma_port_priv *port_priv = (struct edma_port_priv *)netdev_priv(netdev);
	struct ppe_port *port =  port_priv->ppe_port;
	int port_id = port->port_id;

	edma_cfg_tx_napi_delete(port_id);
	edma_cfg_tx_rings_disable(port_id);
}

static u16 __maybe_unused edma_port_select_queue(__maybe_unused struct net_device *netdev,
						 __maybe_unused struct sk_buff *skb,
				__maybe_unused struct net_device *sb_dev)
{
	int cpu = get_cpu();

	put_cpu();

	return cpu;
}

static int edma_port_open(struct net_device *netdev)
{
	struct edma_port_priv *port_priv = (struct edma_port_priv *)netdev_priv(netdev);
	struct ppe_port *ppe_port;
	int port_id;

	if (!port_priv)
		return -EINVAL;

	/* Inform the Linux Networking stack about the hardware capability of
	 * checksum offloading and other features. Each port is
	 * responsible to maintain the feature set it supports.
	 */
	netdev->features |= EDMA_NETDEV_FEATURES;
	netdev->hw_features |= EDMA_NETDEV_FEATURES;
	netdev->vlan_features |= EDMA_NETDEV_FEATURES;
	netdev->wanted_features |= EDMA_NETDEV_FEATURES;

	ppe_port  = port_priv->ppe_port;
	port_id = ppe_port->port_id;

	if (ppe_port->phylink)
		phylink_start(ppe_port->phylink);

	edma_cfg_tx_napi_enable(port_id);
	edma_cfg_tx_enable_interrupts(port_id);

	netif_start_queue(netdev);

	return 0;
}

static int edma_port_close(struct net_device *netdev)
{
	struct edma_port_priv *port_priv = (struct edma_port_priv *)netdev_priv(netdev);
	struct ppe_port *ppe_port;
	int port_id;

	if (!port_priv)
		return -EINVAL;

	netif_stop_queue(netdev);

	/* 20ms delay would provide a plenty of margin to take care of in-flight packets. */
	msleep(20);

	ppe_port  = port_priv->ppe_port;
	port_id = ppe_port->port_id;

	edma_cfg_tx_disable_interrupts(port_id);
	edma_cfg_tx_napi_disable(port_id);

	/* Phylink close. */
	if (ppe_port->phylink)
		phylink_stop(ppe_port->phylink);

	return 0;
}

static int edma_port_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
	struct edma_port_priv *port_priv = (struct edma_port_priv *)netdev_priv(netdev);
	struct ppe_port *ppe_port;
	int ret = -EINVAL;

	if (!port_priv)
		return -EINVAL;

	ppe_port = port_priv->ppe_port;
	if (ppe_port->phylink)
		return phylink_mii_ioctl(ppe_port->phylink, ifr, cmd);

	return ret;
}

static int edma_port_change_mtu(struct net_device *netdev, int mtu)
{
	struct edma_port_priv *port_priv = (struct edma_port_priv *)netdev_priv(netdev);

	if (!port_priv)
		return -EINVAL;

	netdev->mtu = mtu;

	return ppe_port_set_maxframe(port_priv->ppe_port, mtu);
}

static netdev_features_t edma_port_feature_check(__maybe_unused struct sk_buff *skb,
						 __maybe_unused struct net_device *netdev,
						 netdev_features_t features)
{
	return features;
}

static netdev_tx_t edma_port_xmit(struct sk_buff *skb,
				  struct net_device *dev)
{
	struct edma_port_priv *port_priv = NULL;
	struct edma_port_pcpu_stats *pcpu_stats;
	struct edma_txdesc_ring *txdesc_ring;
	struct edma_port_tx_stats *stats;
	enum edma_tx_gso_status result;
	struct sk_buff *segs = NULL;
	u8 cpu_id;
	u32 skbq;
	int ret;

	if (!skb || !dev)
		return NETDEV_TX_OK;

	port_priv = netdev_priv(dev);

	/* Select a TX ring. */
	skbq = (skb_get_queue_mapping(skb) & (num_possible_cpus() - 1));

	txdesc_ring = (struct edma_txdesc_ring *)port_priv->txr_map[skbq];

	pcpu_stats = &port_priv->pcpu_stats;
	stats = this_cpu_ptr(pcpu_stats->tx_stats);

	/* HW does not support TSO for packets with more than or equal to
	 * 32 segments. Perform SW GSO for such packets.
	 */
	result = edma_tx_gso_segment(skb, dev, &segs);
	if (likely(result == EDMA_TX_GSO_NOT_NEEDED)) {
		/* Transmit the packet. */
		ret = edma_tx_ring_xmit(dev, skb, txdesc_ring, stats);

		if (unlikely(ret == EDMA_TX_FAIL_NO_DESC)) {
			if (likely(!edma_ctx->tx_requeue_stop)) {
				cpu_id = smp_processor_id();
				netdev_dbg(dev, "Stopping tx queue due to lack oftx descriptors\n");
				u64_stats_update_begin(&stats->syncp);
				++stats->tx_queue_stopped[cpu_id];
				u64_stats_update_end(&stats->syncp);
				netif_tx_stop_queue(netdev_get_tx_queue(dev, skbq));
				return NETDEV_TX_BUSY;
			}
		}

		if (unlikely(ret != EDMA_TX_OK)) {
			dev_kfree_skb_any(skb);
			u64_stats_update_begin(&stats->syncp);
			++stats->tx_drops;
			u64_stats_update_end(&stats->syncp);
		}

		return NETDEV_TX_OK;
	} else if (unlikely(result == EDMA_TX_GSO_FAIL)) {
		netdev_dbg(dev, "%p: SW GSO failed for segment size: %d\n",
			   skb, skb_shinfo(skb)->gso_segs);
		dev_kfree_skb_any(skb);
		u64_stats_update_begin(&stats->syncp);
		++stats->tx_gso_drop_pkts;
		u64_stats_update_end(&stats->syncp);
		return NETDEV_TX_OK;
	}

	u64_stats_update_begin(&stats->syncp);
	++stats->tx_gso_pkts;
	u64_stats_update_end(&stats->syncp);

	dev_kfree_skb_any(skb);
	while (segs) {
		skb = segs;
		segs = segs->next;

		/* Transmit the packet. */
		ret = edma_tx_ring_xmit(dev, skb, txdesc_ring, stats);
		if (unlikely(ret != EDMA_TX_OK)) {
			dev_kfree_skb_any(skb);
			u64_stats_update_begin(&stats->syncp);
			++stats->tx_drops;
			u64_stats_update_end(&stats->syncp);
		}
	}

	return NETDEV_TX_OK;
}

static void edma_port_get_stats64(struct net_device *netdev,
				  struct rtnl_link_stats64 *stats)
{
	struct edma_port_priv *port_priv = (struct edma_port_priv *)netdev_priv(netdev);

	if (!port_priv)
		return;

	ppe_port_get_stats64(port_priv->ppe_port, stats);
}

static int edma_port_set_mac_address(struct net_device *netdev, void *macaddr)
{
	struct edma_port_priv *port_priv = (struct edma_port_priv *)netdev_priv(netdev);
	struct sockaddr *addr = (struct sockaddr *)macaddr;
	int ret;

	if (!port_priv)
		return -EINVAL;

	netdev_dbg(netdev, "AddrFamily: %d, %0x:%0x:%0x:%0x:%0x:%0x\n",
		   addr->sa_family, addr->sa_data[0], addr->sa_data[1],
		   addr->sa_data[2], addr->sa_data[3], addr->sa_data[4],
		   addr->sa_data[5]);

	ret = eth_prepare_mac_addr_change(netdev, addr);
	if (ret)
		return ret;

	if (ppe_port_set_mac_address(port_priv->ppe_port, (u8 *)addr)) {
		netdev_err(netdev, "set mac address failed for dev: %s\n", netdev->name);
		return -EINVAL;
	}

	eth_commit_mac_addr_change(netdev, addr);

	return 0;
}

static const struct net_device_ops edma_port_netdev_ops = {
	.ndo_open = edma_port_open,
	.ndo_stop = edma_port_close,
	.ndo_start_xmit = edma_port_xmit,
	.ndo_get_stats64 = edma_port_get_stats64,
	.ndo_set_mac_address = edma_port_set_mac_address,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_change_mtu = edma_port_change_mtu,
	.ndo_eth_ioctl = edma_port_ioctl,
	.ndo_features_check = edma_port_feature_check,
	.ndo_select_queue = edma_port_select_queue,
};

/**
 * edma_port_destroy - EDMA port destroy.
 * @port: PPE port
 *
 * Unregister and free the netdevice.
 */
void edma_port_destroy(struct ppe_port *port)
{
	int port_id = port->port_id;
	struct net_device *netdev = edma_ctx->netdev_arr[port_id - 1];

	edma_port_deconfigure(netdev);
	edma_port_stats_free(netdev);
	unregister_netdev(netdev);
	free_netdev(netdev);
	ppe_port_phylink_destroy(port);
	edma_ctx->netdev_arr[port_id - 1] = NULL;
}

/**
 * edma_port_setup - EDMA port Setup.
 * @port: PPE port
 *
 * Initialize and register the netdevice.
 *
 * Return 0 on success, negative error code on failure.
 */
int edma_port_setup(struct ppe_port *port)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct device_node *np = port->np;
	struct edma_port_priv *port_priv;
	int port_id = port->port_id;
	struct net_device *netdev;
	u8 mac_addr[ETH_ALEN];
	const char *name;
	int assign_type;
	int ret = 0;
	u8 *maddr;

	name = of_get_property(np, "label", NULL);
	if (name) {
		assign_type = NET_NAME_PREDICTABLE;
	} else {
		name = "eth%d";
		assign_type = NET_NAME_ENUM;
	}

	netdev = alloc_netdev_mqs(sizeof(struct edma_port_priv),
				  name, assign_type,
				  ether_setup,
				  EDMA_NETDEV_QUEUE_NUM, EDMA_NETDEV_QUEUE_NUM);
	if (!netdev) {
		dev_err(ppe_dev->dev, "alloc_netdev_mqs() failed\n");
		return -ENOMEM;
	}

	SET_NETDEV_DEV(netdev, ppe_dev->dev);
	netdev->dev.of_node = np;

	/* max_mtu is set to 1500 in ether_setup(). */
	netdev->max_mtu = ETH_MAX_MTU;

	port_priv = netdev_priv(netdev);
	memset((void *)port_priv, 0, sizeof(struct edma_port_priv));

	port_priv->ppe_port = port;
	port_priv->netdev = netdev;
	netdev->watchdog_timeo = 5 * HZ;
	netdev->priv_flags |= IFF_LIVE_ADDR_CHANGE;
	netdev->netdev_ops = &edma_port_netdev_ops;
	netdev->gso_max_segs = GSO_MAX_SEGS;
	edma_set_ethtool_ops(netdev);

	maddr = mac_addr;
	if (of_get_mac_address(np, maddr))
		maddr = NULL;

	if (maddr && is_valid_ether_addr(maddr)) {
		eth_hw_addr_set(netdev, maddr);
	} else {
		eth_hw_addr_random(netdev);
		netdev_info(netdev, "GMAC%d Using random MAC address - %pM\n",
			    port_id, netdev->dev_addr);
	}

	/* Allocate memory for EDMA port statistics. */
	ret = edma_port_stats_alloc(netdev);
	if (ret) {
		netdev_dbg(netdev, "EDMA port stats alloc failed\n");
		goto stats_alloc_fail;
	}

	netdev_dbg(netdev, "Configuring the port %s(qcom-id:%d)\n",
		   netdev->name, port_id);

	/* We expect 'port_id' to correspond to ports numbers on SoC.
	 * These begin from '1' and hence we subtract
	 * one when using it as an array index.
	 */
	edma_ctx->netdev_arr[port_id - 1] = netdev;

	edma_port_configure(netdev);

	/* Setup phylink. */
	ret = ppe_port_phylink_setup(port, netdev);
	if (ret) {
		netdev_dbg(netdev, "EDMA port phylink setup for netdevice %s\n",
			   netdev->name);
		goto port_phylink_setup_fail;
	}

	/* Register the network interface. */
	ret = register_netdev(netdev);
	if (ret) {
		netdev_dbg(netdev, "Error registering netdevice %s\n",
			   netdev->name);
		goto register_netdev_fail;
	}

	netdev_dbg(netdev, "Setup EDMA port GMAC%d done\n", port_id);
	return ret;

register_netdev_fail:
	ppe_port_phylink_destroy(port);
port_phylink_setup_fail:
	edma_port_deconfigure(netdev);
	edma_ctx->netdev_arr[port_id - 1] = NULL;
	edma_port_stats_free(netdev);
stats_alloc_fail:
	free_netdev(netdev);

	return ret;
}
