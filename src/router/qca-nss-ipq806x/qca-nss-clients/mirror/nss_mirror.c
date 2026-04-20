/*
 ***************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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
 ***************************************************************************
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/netdevice.h>

#include <nss_api_if.h>
#include <nss_cmn.h>

#include "nss_mirror.h"

#define NSS_MIRROR_NETDEV_NAME "mirror."

/*
 * Mirror netdevice count for naming convension.
 */
static u8 nss_mirror_netdev_count = 0;

/*
 * Mirror device statistics lock.
 */
static DEFINE_SPINLOCK(nss_mirror_stats_lock);

/*
 * Mirror interface array lock.
 */
static DEFINE_SPINLOCK(nss_mirror_arr_lock);

/*
 * Array of mirror interfaces.
 */
static struct net_device *nss_mirror_arr[NSS_MAX_MIRROR_DYNAMIC_INTERFACES];

/*
 * nss_mirror_arr_del()
 *	API to delete mirror netdev from mirror array.
 */
static void nss_mirror_arr_del(struct net_device *mirror_dev)
{
	struct nss_mirror_instance_priv *mirror_priv = netdev_priv(mirror_dev);

	spin_lock(&nss_mirror_arr_lock);
	nss_mirror_arr[mirror_priv->mirror_arr_index] = NULL;
	spin_unlock(&nss_mirror_arr_lock);
}

/*
 * nss_mirror_arr_add()
 *	API to add mirror netdev in mirror array.
 */
static int nss_mirror_arr_add(struct net_device *dev)
{
	uint8_t i;

	spin_lock(&nss_mirror_arr_lock);
	for (i = 0; i < NSS_MAX_MIRROR_DYNAMIC_INTERFACES; i++) {
		struct nss_mirror_instance_priv *mirror_priv;

		if (nss_mirror_arr[i] != NULL) {
			continue;
		}

		/*
		 * Add interface in mirror array.
		 */
		nss_mirror_arr[i] = dev;
		mirror_priv = netdev_priv(dev);
		mirror_priv->mirror_arr_index = i;
		spin_unlock(&nss_mirror_arr_lock);
		return 0;
	}
	spin_unlock(&nss_mirror_arr_lock);
	return -1;
}

/*
 * nss_mirror_display_info()
 *	API to display configure information for the given mirror device.
 */
void nss_mirror_display_info(struct net_device *mirror_dev)
{
	struct nss_mirror_instance_priv *mirror_priv = netdev_priv(mirror_dev);

	pr_info("Mirror interface:%s info:\n", mirror_dev->name);

	pr_info("mirror interface number: %d\n"
			"mirror configure: mirror size: %d, mirror point: %d, mirror offset: %d\n"
			"rule config mode: %d\n",
			 mirror_priv->mirror_instance_if_num, mirror_priv->mirror_size,
			 mirror_priv->mirror_point, mirror_priv->mirror_offset,
			 mirror_priv->rule_config_mode);
}

/*
 * nss_mirror_display_all_info()
 *	API to display configure information for all mirror devices.
 */
void nss_mirror_display_all_info(void)
{
	int i;
	struct net_device *mirror_dev;

	spin_lock(&nss_mirror_arr_lock);
	for (i = 0; i < NSS_MAX_MIRROR_DYNAMIC_INTERFACES; i++) {

		if (nss_mirror_arr[i] == NULL) {
			continue;
		}

		mirror_dev = nss_mirror_arr[i];
		nss_mirror_display_info(mirror_dev);
	}
	spin_unlock(&nss_mirror_arr_lock);
}

/*
 * nss_mirror_netdev_up()
 *	API to make the net device up.
 */
static int nss_mirror_netdev_up(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

/*
 * nss_mirror_netdev_down()
 *	API to make the net device down.
 */
static int nss_mirror_netdev_down(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

/*
 * nss_mirror_get_stats()
 *	API to get statistics for mirror interface.
 */
static struct rtnl_link_stats64 *nss_mirror_get_stats(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	struct nss_mirror_instance_priv *mirror_priv;

	if (!stats) {
		nss_mirror_warn("%px: Invalid stats parameter.\n", dev);
		return stats;
	}

	memset(stats, 0, sizeof(struct rtnl_link_stats64));

	spin_lock_bh(&nss_mirror_stats_lock);
	if (!dev) {
		spin_unlock_bh(&nss_mirror_stats_lock);
		nss_mirror_warn("Invalid netdev.\n");
		return stats;
	}
	dev_hold(dev);
	mirror_priv = netdev_priv(dev);
	memcpy(stats, &mirror_priv->stats, sizeof(struct rtnl_link_stats64));
	dev_put(dev);
	spin_unlock_bh(&nss_mirror_stats_lock);

	return stats;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0))
/*
 * nss_mirror_netdev_stats64()
 *	Netdev ops function to retrieve stats for kernel version < 4.6
 */
static struct rtnl_link_stats64 *nss_mirror_netdev_stats64(struct net_device *dev,
						struct rtnl_link_stats64 *tot)
{
	return nss_mirror_get_stats(dev, tot);
}
#else
/*
 * nss_mirror_netdev_stats64()
 *	Netdev ops function to retrieve stats
 */
static void nss_mirror_netdev_stats64(struct net_device *dev,
						struct rtnl_link_stats64 *tot)
{
	nss_mirror_get_stats(dev, tot);
}
#endif

/*
 * nss_mirror_netdev_ops
 *	Mirror net device operations.
 */
static const struct net_device_ops nss_mirror_netdev_ops = {
	.ndo_open		= nss_mirror_netdev_up,
	.ndo_stop		= nss_mirror_netdev_down,
	.ndo_get_stats64	= nss_mirror_netdev_stats64,
};

/*
 * nss_mirror_netdev_setup()
 *	Setup the mirror net device.
 */
static void nss_mirror_netdev_setup(struct net_device *dev)
{
	dev->addr_len = 0;
	dev->flags = IFF_NOARP;
	dev->features = NETIF_F_FRAGLIST;
	dev->netdev_ops = &nss_mirror_netdev_ops;
}

/*
 * nss_mirror_stats_update()
 *	API to update mirror interface statistics.
 */
static void nss_mirror_stats_update(struct net_device *mirror_dev, struct nss_mirror_stats_sync_msg *stats)
{
	struct rtnl_link_stats64 *netdev_stats;
	struct nss_mirror_instance_priv *mirror_priv;
	uint64_t dropped = 0;

	spin_lock_bh(&nss_mirror_stats_lock);
	if (!mirror_dev) {
		spin_unlock_bh(&nss_mirror_stats_lock);
		nss_mirror_warn("Invalid mirror interface\n");
		return;
	}

	dev_hold(mirror_dev);
	mirror_priv = netdev_priv(mirror_dev);
	netdev_stats = &mirror_priv->stats;

	dropped += nss_cmn_rx_dropped_sum(&stats->node_stats);

	/*
	 * TODO: Currently mirror stat tx_send fail is used by NSS firmware
	 * even after sending the packet to the host.
	 * So, at present, this stat cannot be used as drop stats by this API.
	 * In future, if we need to add this stat too in mirror interface
	 * drop stats, then NSS firmware should not use this stat after sending
	 * the packets to the host and should use some new stat for post mirrored
	 * errors (errors which may occur after sending the packet to the host).
	 */
	dropped += stats->mirror_stats.mem_alloc_fail;
	dropped += stats->mirror_stats.copy_fail;

	/*
	 * There will not be any tx stats for mirror interface, as mirror interface
	 * is intented to receive mirrored packets only.
	 */
	netdev_stats->rx_packets += stats->mirror_stats.mirror_pkts;
	netdev_stats->rx_bytes += stats->mirror_stats.mirror_bytes;
	netdev_stats->rx_dropped += dropped;
	dev_put(mirror_dev);
	spin_unlock_bh(&nss_mirror_stats_lock);
}

/*
 * nss_mirror_event_cb()
 *	Event callback.
 */
static void nss_mirror_event_cb(void *if_ctx, struct nss_cmn_msg *ncm)
{
	struct net_device *netdev = if_ctx;
	struct nss_mirror_msg *nim = (struct nss_mirror_msg *)ncm;

	switch (ncm->type) {
	case NSS_MIRROR_MSG_SYNC_STATS:
		nss_mirror_stats_update(netdev, &nim->msg.stats);
		break;

       default:
		nss_mirror_warn("%px: Unknown Event from NSS\n", netdev);
		break;
       }
}

/*
 * nss_mirror_data_cb()
 *	Data callback.
 */
static void nss_mirror_data_cb(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi)
{
	if (!skb || !skb->data) {
		return;
	}

	dev_hold(netdev);

	/*
	 * Free the packet if mirror device is not up.
	 */
	if (!(netdev->flags & IFF_UP)) {
		kfree_skb(skb);
		dev_put(netdev);
		return;
	}

	skb->dev = netdev;
	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = netdev->ifindex;
	skb_reset_mac_header(skb);
	netif_receive_skb(skb);

	dev_put(netdev);
}

/*
 * nss_mirror_disable()
 *	API to send disable message to mirror interface in NSS firmware.
 */
int nss_mirror_disable(struct net_device *mirror_dev)
{
	struct nss_mirror_msg nmm = {0};
	struct nss_ctx_instance *nss_ctx;
	const struct net_device_ops *ops;
	nss_tx_status_t status;
	int32_t if_num, ret;

	if (!mirror_dev) {
		nss_mirror_warn("Invalid input mirror interface\n");
		return -1;
	}

	/*
	 * Fetch the mirror netdevice.
	 */
	if ((if_num = nss_cmn_get_interface_number_by_dev_and_type(mirror_dev, NSS_DYNAMIC_INTERFACE_TYPE_MIRROR)) < 0) {
		nss_mirror_warn("No valid NSS FW interface for %s device\n", mirror_dev->name);
		return -1;
	}

	nss_ctx = nss_mirror_get_context();
	if (!nss_ctx) {
		nss_mirror_warn("Invalid NSS context\n");
		return -1;
	}

	nss_cmn_msg_init(&nmm.cm, if_num, NSS_MIRROR_MSG_DISABLE, 0, NULL, NULL);

	status = nss_mirror_tx_msg_sync(nss_ctx, &nmm);
	if (status != NSS_TX_SUCCESS) {
		nss_mirror_warn("Sending disable config to %d mirror interface failed\n", if_num);
		return -1;
	}

	/*
	 * Bring down the mirror netdev.
	 */
	ops = mirror_dev->netdev_ops;
	if (ops->ndo_stop) {
		ret = ops->ndo_stop(mirror_dev);
		if (ret) {
			nss_mirror_warn("%s device stop function failed: %d\n", mirror_dev->name, ret);
			return -1;
		}
		mirror_dev->flags &= ~IFF_UP;
	}

	return 0;
}
EXPORT_SYMBOL(nss_mirror_disable);

/*
 * nss_mirror_enable()
 *	API to send enable message to mirror interface in NSS firmware.
 */
int nss_mirror_enable(struct net_device *mirror_dev)
{
	struct nss_mirror_msg nmm = {0};
	struct nss_ctx_instance *nss_ctx;
	const struct net_device_ops *ops;
	nss_tx_status_t status;
	int32_t if_num, ret;

	if (!mirror_dev) {
		nss_mirror_warn("Invalid input mirror interface\n");
		return -1;
	}

	/*
	 * Verify the mirror netdevice.
	 */
	if ((if_num = nss_cmn_get_interface_number_by_dev_and_type(mirror_dev, NSS_DYNAMIC_INTERFACE_TYPE_MIRROR)) < 0) {
		nss_mirror_warn("No valid NSS FW interface for %s device\n", mirror_dev->name);
		return -1;
	}

	nss_ctx = nss_mirror_get_context();
	if (!nss_ctx) {
		nss_mirror_warn("Invalid NSS context\n");
		return -1;
	}

	nss_cmn_msg_init(&nmm.cm, if_num, NSS_MIRROR_MSG_ENABLE, 0, NULL, NULL);

	status = nss_mirror_tx_msg_sync(nss_ctx, &nmm);
	if (status != NSS_TX_SUCCESS) {
		nss_mirror_warn("Sending enable config to %d mirror interface failed\n", if_num);
		return -1;
	}

	/*
	 * Bring up the mirror netdev.
	 */
	ops = mirror_dev->netdev_ops;
	if (ops->ndo_open) {
		ret = ops->ndo_open(mirror_dev);
		if (ret) {
			nss_mirror_warn("%s device open function failed: %d\n", mirror_dev->name, ret);
			return -1;
		}
		mirror_dev->flags |= IFF_UP;
	}

	return 0;
}
EXPORT_SYMBOL(nss_mirror_enable);

/*
 * nss_mirror_set_nexthop()
 *	API to send set nexthop message to mirror interface in NSS firmware.
 */
int nss_mirror_set_nexthop(struct net_device *mirror_dev, int32_t mirror_next_hop)
{
	struct nss_mirror_msg nmm = {0};
	struct nss_ctx_instance *nss_ctx;
	struct nss_mirror_instance_priv *mirror_priv;
	int32_t mirror_if_num;
	nss_tx_status_t status;

	if (!mirror_dev) {
		nss_mirror_warn("Invalid input mirror interface\n");
		return -1;
	}

	/*
	 * Validate the nexthop argument.
	 */
	if (mirror_next_hop >= NSS_MAX_NET_INTERFACES) {
		nss_mirror_warn("Invalid mirror next hop interface:%d\n", mirror_next_hop);
		return -1;
	}

	/*
	 * Fetch the mirror netdevice.
	 */
	if ((mirror_if_num = nss_cmn_get_interface_number_by_dev_and_type(mirror_dev, NSS_DYNAMIC_INTERFACE_TYPE_MIRROR)) < 0) {
		nss_mirror_warn("No valid NSS FW interface for %s device\n", mirror_dev->name);
		return -1;
	}

	/*
	 * Return if the mirror interface is in open state.
	 */
	if ((mirror_dev->flags & IFF_UP)) {
		nss_mirror_warn("%s mirror interface is in open state\n", mirror_dev->name);
		return -1;
	}

	mirror_priv = netdev_priv(mirror_dev);
	if (mirror_priv->rule_config_mode == NSS_MIRROR_MODE_INGRESS_PMC) {
		nss_mirror_warn("Nexthop already configured for %s mirror device\n", mirror_dev->name);
		return -1;
	}

	nss_ctx = nss_mirror_get_context();
	if (!nss_ctx) {
		nss_mirror_warn("Invalid NSS context\n");
		return -1;
	}

	nss_cmn_msg_init(&nmm.cm, mirror_if_num, NSS_MIRROR_MSG_SET_NEXTHOP, 0, NULL, NULL);
	nmm.msg.nexthop.if_num = mirror_next_hop;

	status = nss_mirror_tx_msg_sync(nss_ctx, &nmm);
	if (status != NSS_TX_SUCCESS) {
		nss_mirror_warn("Sending set nexthop config to %d mirror interface failed"
				" Nexthop: %d\n", mirror_if_num, mirror_next_hop);
		return -1;
	}

	/*
	 * Update the configure mode in mirror netdev.
	 */
	mirror_priv->rule_config_mode = NSS_MIRROR_MODE_INGRESS_PMC;

	return 0;
}
EXPORT_SYMBOL(nss_mirror_set_nexthop);

/*
 * nss_mirror_reset_nexthop()
 *	API to send reset nexthop message to mirror interface in NSS firmware.
 */
int nss_mirror_reset_nexthop(struct net_device *mirror_dev)
{
	struct nss_mirror_msg nmm = {0};
	struct nss_ctx_instance *nss_ctx;
	struct nss_mirror_instance_priv *mirror_priv;
	nss_tx_status_t status;
	int32_t if_num;

	if (!mirror_dev) {
		nss_mirror_warn("Invalid input mirror interface\n");
		return -1;
	}

	/*
	 * Verify the mirror netdevice.
	 */
	if ((if_num = nss_cmn_get_interface_number_by_dev_and_type(mirror_dev, NSS_DYNAMIC_INTERFACE_TYPE_MIRROR)) < 0) {
		nss_mirror_warn("No valid NSS FW interface for %s device\n", mirror_dev->name);
		return -1;
	}

	mirror_priv = netdev_priv(mirror_dev);
	if (mirror_priv->rule_config_mode != NSS_MIRROR_MODE_INGRESS_PMC) {
		nss_mirror_warn("Invalid request for %s mirror device\n", mirror_dev->name);
		return -1;
	}

	nss_ctx = nss_mirror_get_context();
	if (!nss_ctx) {
		nss_mirror_warn("Invalid NSS context\n");
		return -1;
	}

	nss_cmn_msg_init(&nmm.cm, if_num, NSS_MIRROR_MSG_RESET_NEXTHOP, 0, NULL, NULL);

	status = nss_mirror_tx_msg_sync(nss_ctx, &nmm);
	if (status != NSS_TX_SUCCESS) {
		nss_mirror_warn("Sending reset nexthop config to %d mirror interface failed\n", if_num);
		return -1;
	}

	/*
	 * Reset the mirror configuration.
	 */
	mirror_priv->rule_config_mode = NSS_MIRROR_MODE_NONE;

	return 0;
}
EXPORT_SYMBOL(nss_mirror_reset_nexthop);

/*
 * nss_mirror_configure()
 *	API to send configure message to mirror interface in NSS firmware.
 */
int nss_mirror_configure(struct net_device *mirror_dev, enum nss_mirror_pkt_clone_point pkt_clone_point,
		 uint16_t pkt_clone_size, uint16_t pkt_clone_offset)
{
	struct nss_mirror_msg nmm = {0};
	struct nss_ctx_instance *nss_ctx;
	struct nss_mirror_instance_priv *mirror_priv;
	nss_tx_status_t status;
	int32_t if_num;

	if (!mirror_dev) {
		nss_mirror_warn("Invalid input mirror interface\n");
		return -1;
	}

	/*
	 * Validate clone point parameter.
	 */
	if (!pkt_clone_point || (pkt_clone_point >= NSS_MIRROR_PKT_CLONE_POINT_MAX)) {
		nss_mirror_warn("Invalid clone point value: %d for device %s\n", pkt_clone_point, mirror_dev->name);
		return -1;
	}

	/*
	 * Fetch the mirror netdevice.
	 */
	if ((if_num = nss_cmn_get_interface_number_by_dev_and_type(mirror_dev, NSS_DYNAMIC_INTERFACE_TYPE_MIRROR)) < 0) {
		nss_mirror_warn("No valid NSS FW interface for %s device\n", mirror_dev->name);
		return -1;
	}

	nss_ctx = nss_mirror_get_context();
	if (!nss_ctx) {
		nss_mirror_warn("Invalid NSS context\n");
		return -1;
	}

	nss_cmn_msg_init(&nmm.cm, if_num, NSS_MIRROR_MSG_CONFIGURE,
			 sizeof(struct nss_mirror_configure_msg), NULL, NULL);
	nmm.msg.config.pkt_clone_size = pkt_clone_size;
	nmm.msg.config.pkt_clone_point = pkt_clone_point;
	nmm.msg.config.pkt_clone_offset = pkt_clone_offset;

	nss_mirror_trace("pkt_clone_size : %d pkt_clone_point %d pkt_clone_offset %d\n", nmm.msg.config.pkt_clone_size, nmm.msg.config.pkt_clone_point, nmm.msg.config.pkt_clone_offset);

	status = nss_mirror_tx_msg_sync(nss_ctx, &nmm);
	if (status != NSS_TX_SUCCESS) {
		nss_mirror_warn("Sending create config to %d mirror interface failed\n", if_num);
		return -1;
	}

	/*
	 * Save the mirror configurations in mirror netdev.
	 */
	mirror_priv = netdev_priv(mirror_dev);
	mirror_priv->mirror_point = pkt_clone_point;
	mirror_priv->mirror_offset = pkt_clone_offset;
	mirror_priv->mirror_size = pkt_clone_size;

	return 0;
}
EXPORT_SYMBOL(nss_mirror_configure);

/*
 * nss_mirror_reset_if_nexthop()
 *	API to send reset nexthop command to NSS firmware.
 * TODO: Remove this API and its usage and use NSS driver API, once
 * similar API is added into NSS driver.
 */
nss_tx_status_t nss_mirror_reset_if_nexthop(uint32_t if_num)
{
	struct nss_if_msg nim;
	struct nss_ctx_instance *nss_ctx;
	nss_tx_status_t status;

	/*
	 * TODO: Use context of the input interface instead of mirror context.
	 */
	nss_ctx = nss_mirror_get_context();
	if (!nss_ctx) {
		nss_mirror_warn("Invalid NSS context\n");
		return -1;
	}

	nss_cmn_msg_init(&nim.cm, if_num, NSS_IF_RESET_NEXTHOP, 0, NULL, NULL);

	status = nss_if_tx_msg(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_mirror_warn("%px: Failed to send reset nexthop message to %d interface\n", nss_ctx, if_num);
		return status;
	}

	nss_mirror_info("%px: Reset nexthop message is sent successfully\n", nss_ctx);
	return status;
}

/*
 * nss_mirror_destroy_if()
 *	API to un-register and destroy the mirror instance in NSS firmware.
 */
static int nss_mirror_destroy_if(struct net_device *mirror_dev)
{
	int32_t if_num;

	/*
	 * Fetch the mirror netdevice.
	 */
	if ((if_num = nss_cmn_get_interface_number_by_dev_and_type(mirror_dev, NSS_DYNAMIC_INTERFACE_TYPE_MIRROR)) < 0) {
		nss_mirror_warn("No valid NSS FW interface for %s device\n", mirror_dev->name);
		return -1;
	}

	nss_mirror_unregister_if(if_num);
	if (nss_dynamic_interface_dealloc_node(if_num, NSS_DYNAMIC_INTERFACE_TYPE_MIRROR)
			!= NSS_TX_SUCCESS) {
		nss_mirror_warn("Failed to dealloc mirror interface instance\n");
		return -1;
	}

	return 0;
}

/*
 * nss_mirror_netdev_destroy()
 *	API to un-register and free mirror net device.
 */
static void nss_mirror_netdev_destroy(struct net_device *mirror_dev)
{
	if (!mirror_dev) {
		nss_mirror_warn("Invalid mirror device pointer\n");
		return;
	}

	unregister_netdev(mirror_dev);

	spin_lock_bh(&nss_mirror_stats_lock);
	free_netdev(mirror_dev);
	spin_unlock_bh(&nss_mirror_stats_lock);
}

/*
 * nss_mirror_destroy()
 *	API to de-register and delete mirror interface.
 */
int nss_mirror_destroy(struct net_device *mirror_dev)
{
	if (!mirror_dev) {
		nss_mirror_warn("Invalid input mirror interface\n");
		return -1;
	}

	/*
	 * Destroy mirror instance in NSS firmware.
	 */
	if (nss_mirror_destroy_if(mirror_dev)) {
		nss_mirror_warn("Error in destroying %s mirror interface from NSS"
				" firmware\n", mirror_dev->name);
		return -1;
	}

	/*
	 * Remove mirror netdev from mirror array.
	 */
	nss_mirror_arr_del(mirror_dev);

	/*
	 * Destroy the mirror device.
	 */
	nss_mirror_netdev_destroy(mirror_dev);
	return 0;
}
EXPORT_SYMBOL(nss_mirror_destroy);

/*
 * nss_mirror_create_if()
 *	Create and register mirror instance in NSS.
 */
static int32_t nss_mirror_create_if(struct net_device *dev, nss_mirror_data_callback_t data_callback)
{
	struct nss_ctx_instance *nss_ctx;
	struct nss_mirror_instance_priv *mirror_priv;
	int32_t if_num, ret;
	uint32_t features = 0;

	if_num = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_MIRROR);
	if (if_num < 0) {
		nss_mirror_warn("Mirror interface creation failed - alloc node failed\n");
		return -1;
	}

	nss_ctx = nss_mirror_register_if(if_num,
			data_callback,
			nss_mirror_event_cb,
			dev,
			features);
	if (!nss_ctx) {
		nss_mirror_warn("%d interface registration failed\n", if_num);
		ret = nss_dynamic_interface_dealloc_node(if_num, NSS_DYNAMIC_INTERFACE_TYPE_MIRROR);
		if (ret != NSS_TX_SUCCESS) {
			nss_mirror_warn("%d interface dealloc failed\n", if_num);
		}
		return -1;
	}

	nss_mirror_trace("Alloc status on %d interface\n", if_num);

	/*
	 * Save NSS mirror interface number in mirror netdev.
	 */
	mirror_priv = netdev_priv(dev);
	mirror_priv->mirror_instance_if_num = if_num;

	return if_num;
}

/*
 * nss_mirror_create()
 *	API to create and register mirror interface.
 */
struct net_device *nss_mirror_create(void)
{
	struct net_device *dev;
	char *devname;
	int32_t mirror_if_num;
	int ret;

	devname = vzalloc(strlen(NSS_MIRROR_NETDEV_NAME)+3+1);
	if (!devname) {
		nss_mirror_warn("memory allocation failed\n");
		return NULL;
	}

	snprintf(devname, strlen(NSS_MIRROR_NETDEV_NAME)+3, "%s%d", NSS_MIRROR_NETDEV_NAME, nss_mirror_netdev_count);

	dev = alloc_netdev(sizeof(struct nss_mirror_instance_priv), devname, NET_NAME_UNKNOWN, nss_mirror_netdev_setup);
	if (!dev) {
		nss_mirror_warn("netdev allocation failed\n");
		vfree(devname);
		return NULL;
	}

	mirror_if_num = nss_mirror_create_if(dev, nss_mirror_data_cb);
	if (mirror_if_num < 0) {
		nss_mirror_warn("mirror interface instance creation failed\n");
		free_netdev(dev);
		vfree(devname);
		return NULL;
	}

	ret = register_netdev(dev);
	if (ret) {
		nss_mirror_warn("netdev registration failed\n");
		if (nss_mirror_destroy_if(dev)) {
			nss_mirror_warn("Error in destroying %s mirror device from"
					" NSS firmware\n", dev->name);
		}
		free_netdev(dev);
		vfree(devname);
		return NULL;
	}

	/*
	 * Add the mirror netdev to mirror array.
	 */
	if (nss_mirror_arr_add(dev)) {
		nss_mirror_warn("Error in adding %s mirror device in mirror array\n", dev->name);
		if (nss_mirror_destroy_if(dev)) {
			nss_mirror_warn("Error in destroying %s mirror device from"
					" NSS firmware\n", dev->name);
		}
		nss_mirror_netdev_destroy(dev);
		vfree(devname);
		return NULL;
	}

	nss_mirror_netdev_count++;
	return dev;
}
EXPORT_SYMBOL(nss_mirror_create);

/*
 * nss_mirror_deconfigure_mirror()
 *	API to deconfigure mirror device.
 */
int nss_mirror_deconfigure_mirror(struct net_device *mirror_dev)
{
	struct nss_mirror_instance_priv *mirror_priv = netdev_priv(mirror_dev);

	if (mirror_priv->rule_config_mode == NSS_MIRROR_MODE_NONE) {
		return 0;
	}

	if (mirror_priv->rule_config_mode == NSS_MIRROR_MODE_INGRESS_PMC) {
		if (nss_mirror_reset_nexthop(mirror_dev)) {
			nss_mirror_warn("Error in sending reset nexthop config to mirror interface: %d\n",
					mirror_priv->mirror_instance_if_num);
			return -1;
		}
	}

	/*
	 * Reset the mirror mode.
	 */
	mirror_priv->rule_config_mode = NSS_MIRROR_MODE_NONE;
	return 0;
}

/*
 * nss_mirror_destroy_all()
 *	API to destroy all the configured mirror devices.
 */
int nss_mirror_destroy_all(void)
{
	int i;

	for (i = 0; i < NSS_MAX_MIRROR_DYNAMIC_INTERFACES; i++) {
		struct net_device *mirror_dev;

		spin_lock(&nss_mirror_arr_lock);
		if (nss_mirror_arr[i] == NULL) {
			spin_unlock(&nss_mirror_arr_lock);
			continue;
		}

		mirror_dev = nss_mirror_arr[i];
		spin_unlock(&nss_mirror_arr_lock);

		/*
		 * Deconfigure mirror interface.
		 */
		if (nss_mirror_deconfigure_mirror(mirror_dev) < 0) {
			nss_mirror_warn("Error in deconfiguring mirror interface: %s\n", mirror_dev->name);
			return -1;
		}

		if (nss_mirror_destroy(mirror_dev)) {
			nss_mirror_warn("Error in sending delete config to mirror interface: %s\n", mirror_dev->name);
			return -1;
		}
	}
	return 0;
}
