/*
 ***************************************************************************
 * Copyright (c) 2015-2016, 2018-2021, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ***************************************************************************
 */

#include <linux/version.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <net/genetlink.h>
#include <nss_api_if.h>
#include <nss_nl_if.h>
#include "nss_nlcmn_if.h"
#include "nss_nl.h"
#include "nss_nlgre_redir_if.h"
#include "nss_nlgre_redir_cmn.h"

static struct nss_nlgre_redir_cmn_tun_data tun_data[NSS_NLGRE_REDIR_CMN_MAX_TUNNELS];
static const struct net_device_ops gre_redir_netdev_ops;
static DEFINE_SPINLOCK(lock);

/*
 * nss_nlgre_redir_cmn_get_tun_data()
 * 	Returns the tun_data after checking for lock
 */
static struct nss_nlgre_redir_cmn_tun_data nss_nlgre_redir_cmn_get_tun_data(struct net_device *dev)
{
	struct nss_nlgre_redir_cmn_tun_data dummy_tun_data = {0};
	int index;

	spin_lock(&lock);
	for (index = 0; index < NSS_NLGRE_REDIR_CMN_MAX_TUNNELS; index++) {
		if (dev != tun_data[index].dev) {
			continue;
		}

		spin_unlock(&lock);
		return tun_data[index];
	}

	spin_unlock(&lock);
	return dummy_tun_data;
}

/*
 * nss_nlgre_redir_cmn_get_next_free_tun()
 * 	Returns the next free tunnel available
 */
static int nss_nlgre_redir_cmn_get_next_free_tun(void)
{
	int index;

	spin_lock(&lock);
	for (index = 0; index < NSS_NLGRE_REDIR_CMN_MAX_TUNNELS ; index++) {
		if (!tun_data[index].enable) {
			spin_unlock(&lock);
			return index;
		}
	}

	spin_unlock(&lock);
	nss_nl_error("Max tunnel count exceeded: %d\n", index);
	return -1;
}

/*
 * nss_nlgre_redir_cmn_set_tun_data()
 * 	Set the tun_data value to value passed
 */
static bool nss_nlgre_redir_cmn_set_tun_data(struct nss_nlgre_redir_cmn_tun_data *data, int index)
{
	if (!data) {
		nss_nl_error("data is NULL\n");
		return false;
	}

	spin_lock(&lock);
	tun_data[index] = *data;
	spin_unlock(&lock);
	return true;
}

/*
 * nss_nlgre_redir_cmn_init_tun_data()
 * 	Initializes the tun_data
 */
static void nss_nlgre_redir_cmn_init_tun_data(struct nss_nlgre_redir_cmn_tun_data *tun_data)
{
	tun_data->dev = NULL;
	tun_data->enable = false;
	tun_data->host_inner_ifnum = -1;
	tun_data->wifi_offl_inner_ifnum = -1;
	tun_data->sjack_inner_ifnum = -1;
	tun_data->outer_ifnum = -1;
}

/*
 * nss_nlgre_redir_cmn_deinit_tun_data()
 *	Deinitialize private data for the given index.
 */
static bool nss_nlgre_redir_cmn_deinit_tun_data(struct nss_nlgre_redir_cmn_tun_data *tun_data, int index)
{
	struct nss_ctx_instance *nss_ctx;

	nss_ctx = nss_gre_redir_get_context();
	tun_data->dev = NULL;
	tun_data->enable = false;
	tun_data->host_inner_ifnum = -1;
	tun_data->wifi_offl_inner_ifnum = -1;
	tun_data->sjack_inner_ifnum = -1;
	tun_data->outer_ifnum = -1;

	if (!nss_nlgre_redir_cmn_set_tun_data(tun_data, index)) {
		nss_nl_error("%px: Unable to set tun_data\n", nss_ctx);
		return false;
	}

	return true;
}

/*
 * nss_nlgre_redir_cmn_host_data_cb()
 *	Data callback for host offload inner node.
 */
static void nss_nlgre_redir_cmn_host_data_cb(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();

	if (!skb) {
		nss_nl_trace("%px: SKB is NULL\n", nss_ctx);
		return;
	}

	nss_nl_trace("%px: Exception packet on host inner:\n", skb);
	nss_nlgre_redir_cmn_print_hex_dump(skb);
	skb->protocol = eth_type_trans(skb, netdev);
	netif_receive_skb(skb);
}

/*
 * nss_nlgre_redir_cmn_wifi_offl_data_cb()
 *	Data callback for wifi offload inner node.
 */
static void nss_nlgre_redir_cmn_wifi_offl_data_cb(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();

	if (!skb) {
		nss_nl_warn("%px: SKB is NULL\n", nss_ctx);
		return;
	}

	nss_nl_trace("%px: Exception packet on wifi offld inner:\n", skb);
	nss_nlgre_redir_cmn_print_hex_dump(skb);
	skb->protocol = eth_type_trans(skb, netdev);
	netif_receive_skb(skb);
}

/*
 * nss_nlgre_redir_cmn_sjack_data_cb
 *	Data callback for sjack inner node.
 */
static void nss_nlgre_redir_cmn_sjack_data_cb(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi)
{
	nss_nl_trace("%px: Exception packet on sjack inner node:\n", skb);
	nss_nlgre_redir_cmn_print_hex_dump(skb);
	dev_kfree_skb(skb);
}

/*
 * nss_nlgre_redir_cmn_outer_data_cb()
 *	Data callback for outer node.
 */
static void nss_nlgre_redir_cmn_outer_data_cb(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi)
{
	nss_nl_trace("%px: Exception packet on outer node:\n", skb);
	nss_nlgre_redir_cmn_print_hex_dump(skb);
	dev_kfree_skb(skb);
}

/*
 * nss_nlgre_redir_cmn_map_unmap_msg_cb()
 *	HLOS->NSS message completion callback.
 */
static void nss_nlgre_redir_cmn_map_unmap_msg_cb(void *app_data, struct nss_cmn_msg *cmnmsg)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();
	nss_nl_info("%px: callback gre_redir tunnel msg from NSS\n", nss_ctx);
}

/*
 * nss_nlgre_redir_cmn_interface_alloc_and_register()
 * 	Allocates nodes and registers callbacks
 */
static int nss_nlgre_redir_cmn_interface_alloc_and_register(struct nss_nlgre_redir_cmn_tun_data *tun_data, struct net_device *dev)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();

	tun_data->host_inner_ifnum = nss_gre_redir_alloc_and_register_node(dev,
			nss_nlgre_redir_cmn_host_data_cb,
			NULL, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_HOST_INNER, dev);
	nss_nl_info("%px: host_inner = %d\n", nss_ctx, tun_data->host_inner_ifnum);
	if (tun_data->host_inner_ifnum == -1) {
		nss_nl_error("%px: Unable to allocate and register wifi host inner interface\n", nss_ctx);
		return -1;
	}

	tun_data->wifi_offl_inner_ifnum = nss_gre_redir_alloc_and_register_node(dev,
			nss_nlgre_redir_cmn_wifi_offl_data_cb,
			NULL, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_OFFL_INNER, dev);
	nss_nl_info("%px: wifi_inner = %d\n", nss_ctx, tun_data->wifi_offl_inner_ifnum);
	if (tun_data->wifi_offl_inner_ifnum == -1) {
		nss_nl_error("%px: Unable to allocate and register wifi offload inner interface\n", nss_ctx);
		return -1;
	}

	tun_data->sjack_inner_ifnum = nss_gre_redir_alloc_and_register_node(dev,
			nss_nlgre_redir_cmn_sjack_data_cb,
			NULL, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_SJACK_INNER, dev);
	nss_nl_info("%px: sjack_inner = %d\n", nss_ctx, tun_data->sjack_inner_ifnum);
	if (tun_data->sjack_inner_ifnum == -1) {
		nss_nl_error("%px: Unable to allocate and register sjack inner interface\n", nss_ctx);
		return -1;
	}

	tun_data->outer_ifnum = nss_gre_redir_alloc_and_register_node(dev,
			nss_nlgre_redir_cmn_outer_data_cb,
			NULL, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_OUTER, dev);
	nss_nl_info("%px: outer = %d\n", nss_ctx, tun_data->outer_ifnum);
	if (tun_data->outer_ifnum == -1) {
		nss_nl_error("%px: Unable to allocate and register outer interface\n", nss_ctx);
		return -1;
	}

	return 0;

}

/*
 * nss_nlgre_redir_cmn_open_interface()
 *	Used when the interface is opened for use.
 */
static int nss_nlgre_redir_cmn_open_interface(struct net_device *dev)
{
	struct nss_gre_redir_cmn_ndev_priv *priv;
	priv = netdev_priv(dev);
	priv->gre_seq = 0;
	netif_start_queue(dev);
	netif_carrier_on(dev);
	return 0;
}

/*
 * nss_nlgre_redir_cmn_close_interace()
 *	Used when the interface is closed.
 */
static int nss_nlgre_redir_cmn_close_interface(struct net_device *dev)
{
	netif_stop_queue (dev);
	netif_carrier_off(dev);
	return 0;
}

/*
 * nss_nlgre_redir_cmn_xmit_data()
 *	Used when the interface is used for transmit data.
 */
static netdev_tx_t nss_nlgre_redir_cmn_xmit_data(struct sk_buff *skb, struct net_device *dev)
{
	struct nss_gre_redir_encap_per_pkt_metadata *meta_data_encap = NULL;
	struct nss_gre_redir_cmn_ndev_priv *priv;
	uint32_t ifnum, ret = 0;
	struct nss_ctx_instance *nss_ctx;

	nss_ctx = nss_gre_redir_get_context();
	priv = netdev_priv(dev);
	ifnum = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_HOST_INNER);

	/*
	 * Initializing the start of skb with offset of metadata
	 */
	*(skb->head) = NSS_GRE_REDIR_PER_PACKET_METADATA_OFFSET;

	/*
	 * Configuring gre_redir meta data.
	 */
	meta_data_encap = (struct nss_gre_redir_encap_per_pkt_metadata *)(skb->head + NSS_GRE_REDIR_PER_PACKET_METADATA_OFFSET);
	memset(meta_data_encap, 0, sizeof(struct nss_gre_redir_encap_per_pkt_metadata));
	meta_data_encap->gre_flags = 0;
	meta_data_encap->gre_prio = 0;
	meta_data_encap->gre_seq = ++priv->gre_seq;
	meta_data_encap->gre_tunnel_id = 10;
	meta_data_encap->ip_dscp = 0;
	meta_data_encap->ip_df_override = 0;
	meta_data_encap->ipsec_pattern = 0;

	nss_ctx = nss_gre_redir_get_context();
	ret = nss_gre_redir_tx_buf(nss_ctx, skb, ifnum);
	if (ret != NSS_TX_SUCCESS) {
		nss_nl_error("%px: Transmit failed and returned with %d\n", nss_ctx, ret);
		dev_kfree_skb_any(skb);
	}

	return ret;
}

/*
 * nss_nlgre_redir_cmn_stats64_get()
 *	Used to get link statistics.
 */
static struct rtnl_link_stats64 *nss_nlgre_redir_cmn_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	struct nss_gre_redir_tunnel_stats get_stats;
	bool found = false;
	int i;

	for (i = 0; i < NSS_GRE_REDIR_MAX_INTERFACES; i++) {
		if (!nss_gre_redir_stats_get(i, &get_stats)) {
			continue;
		}

		if (get_stats.dev != dev) {
			continue;
		}

		found = true;
		break;
	}

	if (found == false)
		return NULL;

	stats->tx_bytes   = get_stats.tstats.tx_bytes;
	stats->tx_packets = get_stats.tstats.tx_packets;
	stats->rx_bytes   = get_stats.tstats.rx_bytes;
	stats->rx_packets = get_stats.tstats.rx_packets;
	for (i = 0;i < ARRAY_SIZE(get_stats.tstats.rx_dropped); i++) {
		stats->rx_dropped += get_stats.tstats.rx_dropped[i];
	}

	stats->tx_dropped = get_stats.tstats.tx_dropped;

	return stats;
}

/*
 * nss_nlgre_redir_cmn_dev_stats64
 *	Report packet statistics to linux
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
static struct rtnl_link_stats64 *nss_nlgre_redir_cmn_dev_stats64(struct net_device *dev,
		struct rtnl_link_stats64 *stats)
{
	return nss_nlgre_redir_cmn_get_stats64(dev, stats);
}
#else
static void nss_nlgre_redir_cmn_dev_stats64(struct net_device *dev,
		struct rtnl_link_stats64 *stats)
{
	nss_nlgre_redir_cmn_get_stats64(dev, stats);
}
#endif

/*
 * nss_nlgre_redir_cmn_set_mac_address()
 * 	Sets the mac address of netdev
 */
static int nss_nlgre_redir_cmn_set_mac_address(struct net_device *dev, void *p)
{
	struct sockaddr *addr = (struct sockaddr *)p;

	if (!is_valid_ether_addr(addr->sa_data)) {
		nss_nl_error("%pM: MAC address validation failed\n", addr->sa_data);
		return -EINVAL;
	}

	memcpy((void *) dev->dev_addr, addr->sa_data, ETH_ALEN);
	return 0;
}

/*
 * nss_nlgre_redir_cmn_netdev_destructor()
 * 	Unregisters and free the net device
 */
static void nss_nlgre_redir_cmn_netdev_destructor(struct net_device *dev)
{
	nss_nl_info("Gre_redir tunnel device freed %s\n", dev->name);
	free_netdev(dev);
}

/*
 * nss_nlgre_redir_cmn_dev_setup()
 * 	To setup the netdevice
 */
static void nss_nlgre_redir_cmn_dev_setup(struct net_device *dev)
{
	ether_setup(dev);
	dev->needed_headroom = NSS_NLGRE_REDIR_CMN_NEEDED_HEADROOM;
	dev->netdev_ops = &gre_redir_netdev_ops;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
	dev->destructor = nss_nlgre_redir_cmn_netdev_destructor;
#else
	dev->priv_destructor = nss_nlgre_redir_cmn_netdev_destructor;
#endif
	eth_hw_addr_random(dev);
}

/*
 * net_device_ops
 *	Netdevice operations.
 */
static const struct net_device_ops gre_redir_netdev_ops = {
	.ndo_open = nss_nlgre_redir_cmn_open_interface,
	.ndo_stop = nss_nlgre_redir_cmn_close_interface,
	.ndo_start_xmit = nss_nlgre_redir_cmn_xmit_data,
	.ndo_get_stats64 = nss_nlgre_redir_cmn_dev_stats64,
	.ndo_set_mac_address = nss_nlgre_redir_cmn_set_mac_address,
};

/*
 * nss_nlgre_redir_cmn_print_hex_dump()
 *	To print hex dump of packet received
 */
void nss_nlgre_redir_cmn_print_hex_dump(struct sk_buff *skb)
{
	int16_t dump_sz = (skb->len < NSS_NLGRE_REDIR_PKT_DUMP_SZ) ? skb->len : NSS_NLGRE_REDIR_PKT_DUMP_SZ;

	dump_sz -= NSS_NLGRE_REDIR_PKT_DUMP_OFFSET;
	if (dump_sz > 0) {
		/*
		 * Enable dynamic debug to print
		 */
		print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, skb->data, dump_sz);
		return;
	}

	nss_nl_warn("Could not print packet skb->len=%d, DUMP_SZ=%d, DUMP_OFFSET=%d\n",
			skb->len, NSS_NLGRE_REDIR_PKT_DUMP_SZ, NSS_NLGRE_REDIR_PKT_DUMP_OFFSET);
}

/*
 * nss_nlgre_redir_cmn_mode_str_to_enum()
 * 	Returns the type of mode
 */
enum nss_nlgre_redir_cmn_mode_type nss_nlgre_redir_cmn_mode_str_to_enum(char *mode)
{
	if (!mode)
		return NSS_NLGRE_REDIR_CMN_MODE_TYPE_UNKNOWN;
	if (!strncmp(mode, "wifi", NSS_NLGRE_REDIR_MODE_MAX_SZ))
		return NSS_NLGRE_REDIR_CMN_MODE_TYPE_WIFI;
	if (!strncmp(mode, "split", NSS_NLGRE_REDIR_MODE_MAX_SZ))
		return NSS_NLGRE_REDIR_CMN_MODE_TYPE_SPLIT;

	return NSS_NLGRE_REDIR_CMN_MODE_TYPE_UNKNOWN;
}

/*
 * nss_nlgre_redir_cmn_get_tun_ifnum()
 * 	Returns the interface number of the net device
 */
int32_t nss_nlgre_redir_cmn_get_tun_ifnum(enum nss_nlgre_redir_cmn_mode_type type, struct net_device *dev)
{
	struct nss_nlgre_redir_cmn_tun_data tun_data;

	if (!dev) {
		nss_nl_error("net_dev is NULL\n");
		return -1;
	}

	tun_data = nss_nlgre_redir_cmn_get_tun_data(dev);
	if (!tun_data.dev) {
		nss_nl_error("Invalid tun_data: %px\n", tun_data.dev);
		return -1;
	}

	switch(type) {
	case NSS_NLGRE_REDIR_CMN_MODE_TYPE_WIFI:
		return tun_data.wifi_offl_inner_ifnum;
	case NSS_NLGRE_REDIR_CMN_MODE_TYPE_SPLIT:
		return NSS_ETH_RX_INTERFACE;
	default:
		nss_nl_error("Wrong mode type: %d\n", type);
		return -1;
	}

	return -1;
}

/*
 * nss_nlgre_redir_cmn_get_tun_type()
 * 	Returns the type of tunnel we'll operate in
 */
enum nss_nlgre_redir_cmn_tun_type nss_nlgre_redir_cmn_get_tun_type(char *tun_type)
{
	if (!tun_type)
		return NSS_NLGRE_REDIR_CMN_TUN_TYPE_UNKNOWN;
	if (!strncmp(tun_type, "tun", NSS_NLGRE_REDIR_CMN_TUN_TYPE_MAX_SZ))
		return NSS_NLGRE_REDIR_CMN_TUN_TYPE_TUN;
	if (!strncmp(tun_type, "dtun", NSS_NLGRE_REDIR_CMN_TUN_TYPE_MAX_SZ))
		return NSS_NLGRE_REDIR_CMN_TUN_TYPE_DTUN;
	if (!strncmp(tun_type, "split", NSS_NLGRE_REDIR_CMN_TUN_TYPE_MAX_SZ))
		return NSS_NLGRE_REDIR_CMN_TUN_TYPE_SPLIT;

	return NSS_NLGRE_REDIR_CMN_TUN_TYPE_UNKNOWN;
}

/*
 * nss_nlgre_redir_cmn_get_tun_data_index()
 *	Returns index in array of private data.
 */
int nss_nlgre_redir_cmn_get_tun_data_index(struct net_device *dev)
{
	struct nss_ctx_instance *nss_ctx;
	uint32_t iter;

	nss_ctx = nss_gre_redir_get_context();
	if (!dev) {
		nss_nl_error("%px: Dev is NULL\n", nss_ctx);
		return -1;
	}

	spin_lock(&lock);
	for (iter = 0; iter < NSS_NLGRE_REDIR_CMN_MAX_TUNNELS; iter++) {
		if (tun_data[iter].dev != dev) {
			continue;
		}

		spin_unlock(&lock);
		return iter;
	}

	spin_unlock(&lock);
	return -1;
}

/*
 * nss_gre_redir_unregister_and_deallocate()
 *	Unregisters and deallocates corresponding dev and node.
 */
bool nss_nlgre_redir_cmn_unregister_and_deallocate(struct net_device *dev, uint32_t type)
{
	nss_tx_status_t status;
	int ifnum;
	bool ret;

	ifnum = nss_cmn_get_interface_number_by_dev_and_type(dev, type);
	if (ifnum == -1) {
		nss_nl_error("%px: unable to get NSS interface for net device %s of type %d\n", dev, dev->name, type);
		return false;
	}

	ret = nss_gre_redir_unregister_if(ifnum);
	if (!ret) {
		nss_nl_error("%px: Unable to unregister interface %d\n", dev, ret);
		return false;
	}

	status = nss_dynamic_interface_dealloc_node(ifnum, type);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_error("%px: Unable to deallocate node %d\n", dev, status);
		return false;
	}

	nss_nl_trace("%s: Sucessfully unregistered and deallocated %d\n", dev->name, ifnum);
	return true;
}

/*
 * nss_nlgre_redir_cmn_interfaces_unregister_and_dealloc
 * 	Find out the interfaces to be deallocated
 */
void nss_nlgre_redir_cmn_interfaces_unregister_and_dealloc(struct nss_nlgre_redir_cmn_tun_data *tun_data)
{
	struct nss_ctx_instance *nss_ctx;

	nss_ctx = nss_gre_redir_get_context();
	if (tun_data->sjack_inner_ifnum != -1) {
		if(!nss_nlgre_redir_cmn_unregister_and_deallocate(tun_data->dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_HOST_INNER)) {
			nss_nl_error("%px: Unable to unregister and deallocate node of type %d\n", nss_ctx,
					NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_HOST_INNER);
		}
	}

	if (tun_data->wifi_offl_inner_ifnum != -1) {
		if (!nss_nlgre_redir_cmn_unregister_and_deallocate(tun_data->dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_OFFL_INNER)) {
			nss_nl_error("%px: Unable to unregister and deallocate node of type %d\n", nss_ctx,
					NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_OFFL_INNER);
		}
	}

	if (tun_data->host_inner_ifnum != -1) {
		if (!nss_nlgre_redir_cmn_unregister_and_deallocate(tun_data->dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_SJACK_INNER)) {
			nss_nl_error("%px: Unable to unregister and deallocate node of type %d\n", nss_ctx,
					NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_SJACK_INNER);
		}
	}

	if (tun_data->outer_ifnum != -1) {
		if (!nss_nlgre_redir_cmn_unregister_and_deallocate(tun_data->dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_OUTER)) {
			nss_nl_error("%px: Unable to unregister and deallocate node of type %d\n", nss_ctx,
					NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_OUTER);
		}
	}

	nss_nl_trace("%s: Sucessfully unregistered and deallocated\n", tun_data->dev->name);
}

/*
 * nss_nlgre_redir_cmn_destroy_tun()
 *	Unregisters and deallocs dynamic interfaces.
 */
int nss_nlgre_redir_cmn_destroy_tun(struct net_device *dev)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();
	struct nss_nlgre_redir_cmn_tun_data tun_data;
	int index;

	dev_hold(dev);
	tun_data = nss_nlgre_redir_cmn_get_tun_data(dev);
	if (!tun_data.dev) {
		nss_nl_error("%px: Invalid tun data\n", nss_ctx);
		return -1;
	}

	index = nss_nlgre_redir_cmn_get_tun_data_index(tun_data.dev);
	if (index < NSS_NLGRE_REDIR_CMN_MIN_TUNNELS || index >= NSS_NLGRE_REDIR_CMN_MAX_TUNNELS) {
		nss_nl_error("%px: index out of bound %d\n", nss_ctx, index);
		return -1;
	}

	nss_nlgre_redir_cmn_interfaces_unregister_and_dealloc(&tun_data);
	nss_nlgre_redir_cmn_deinit_tun_data(&tun_data, index);
	dev_put(dev);
	unregister_netdev(dev);
	nss_nl_info("%px: Successfully destroyed gretun = gretun%d tunnel\n", dev, index);
	return index;
}

/*
 * nss_nlgre_redir_cmn_create_tun()
 *	Allocates netdevice and configures tunnel.
 */
struct net_device *nss_nlgre_redir_cmn_create_tun(uint32_t sip[4], uint32_t dip[4], uint8_t iptype)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();
	struct nss_gre_redir_outer_configure_msg ngrocm = {0};
	struct nss_gre_redir_inner_configure_msg ngrm = {0};
	struct nss_nlgre_redir_cmn_tun_data tun_data;
	struct nss_gre_redir_cmn_ndev_priv *priv;
	struct net_device *dev;
	nss_tx_status_t status;
	int tun_idx = -1, ret;

	tun_idx = nss_nlgre_redir_cmn_get_next_free_tun();
	if (tun_idx == -1) {
		nss_nl_error("Unable to allocate any tunnel\n");
		return NULL;
	}

	/*
	 * Initializes the tun_data
	 */
	nss_nlgre_redir_cmn_init_tun_data(&tun_data);
	dev = alloc_netdev(sizeof(*priv), "gretun%d", NET_NAME_UNKNOWN, nss_nlgre_redir_cmn_dev_setup);
	if (!dev) {
		nss_nl_error("Unable to allocate netdev\n");
		return NULL;
	}

	if (register_netdev(dev)) {
		nss_nl_warn("Unable to register netdev %s\n", dev->name);
		free_netdev(dev);
		goto fail;
	}

	/*
	 * Dynamic interface allocation.
	 */
	ret = nss_nlgre_redir_cmn_interface_alloc_and_register(&tun_data, dev);
	if (ret == -1) {
		nss_nl_error("%px: Unable to allocate and register gre_redir nodes\n", nss_ctx);
		unregister_netdev(dev);
		goto fail;
	}

	memcpy(ngrm.ip_src_addr, sip, sizeof(ngrm.ip_src_addr));
	memcpy(ngrm.ip_dest_addr, dip, sizeof(ngrm.ip_dest_addr));

	/*
	 * TODO: Dynamic assignment of values from userspace
	 * ip_df_policy value currently hard coded. This needs to be supplied from userspace.
	 */
	ngrm.ip_hdr_type = iptype;
	ngrm.ip_df_policy = 0;
	ngrm.gre_version = 0;
	ngrm.ip_ttl = NSS_NLGRE_REDIR_CMN_IP_TTL;
	ngrm.except_outerif = tun_data.outer_ifnum;

	/*
	 * TODO: Dynamic assignment of values from userspace
	 * rps_hint value currently hard coded. This needs to be supplied from userspace.
	 */
	ngrocm.ip_hdr_type = iptype;
	ngrocm.rps_hint = 0;
	ngrocm.except_hostif = tun_data.host_inner_ifnum;
	ngrocm.except_offlif = tun_data.wifi_offl_inner_ifnum;
	ngrocm.except_sjackif = tun_data.sjack_inner_ifnum;

	status = nss_gre_redir_configure_inner_node(tun_data.host_inner_ifnum, &ngrm);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_warn("%px: unable to configure host inner node %d\n", nss_ctx, tun_data.host_inner_ifnum);
		unregister_netdev(dev);
		goto fail;
	}

	status = nss_gre_redir_configure_inner_node(tun_data.wifi_offl_inner_ifnum, &ngrm);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_warn("%px: unable to configure wifi offload inner node %d\n", nss_ctx, tun_data.host_inner_ifnum);
		unregister_netdev(dev);
		goto fail;
	}

	status = nss_gre_redir_configure_inner_node(tun_data.sjack_inner_ifnum, &ngrm);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_warn("%px: unable to configure sjack inner node %d\n", nss_ctx, tun_data.sjack_inner_ifnum);
		unregister_netdev(dev);
		goto fail;
	}

	status = nss_gre_redir_configure_outer_node(tun_data.outer_ifnum, &ngrocm);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_warn("%px: unable to configure outer node %d\n", nss_ctx, tun_data.host_inner_ifnum);
		unregister_netdev(dev);
		goto fail;
	}

	tun_data.enable = true;
	tun_data.dev = dev;
	if (!nss_nlgre_redir_cmn_set_tun_data(&tun_data, tun_idx)) {
		nss_nl_error("%px: Unable to set tun data\n", nss_ctx);
		unregister_netdev(dev);
		goto fail;
	}

	return dev;
fail:
	nss_nlgre_redir_cmn_interfaces_unregister_and_dealloc(&tun_data);
	nss_nlgre_redir_cmn_deinit_tun_data(&tun_data, tun_idx);
	return NULL;
}

/*
 * nss_nlgre_redir_cmn_get_dev_ifnum()
 * 	Returns the interface number by dev and type
 */
int32_t nss_nlgre_redir_cmn_get_dev_ifnum(char *dev_name)
{
	struct net_device *dev;
	uint32_t ifnum;

	if (!dev_name) {
		nss_nl_error("dev_name is NULL\n");
		return -1;
	}

	/*
	 * Get the dev reference
	 */
	dev = dev_get_by_name(&init_net, dev_name);
	if (!dev) {
		nss_nl_error("Invalid parameter: %s\n", dev_name);
		return -ENODEV;
	}

	ifnum = nss_cmn_get_interface_number_by_dev(dev);
	dev_put(dev);
	return ifnum;
}

/*
 * nss_nlgre_redir_cmn_map_interface()
 *	Map nss interface to tunnel ID.
 */
int nss_nlgre_redir_cmn_map_interface(uint32_t nexthop_nssif, uint16_t lag_en, struct nss_nlgre_redir_map *map_params)
{
	struct nss_gre_redir_msg ngrm;
	struct nss_ctx_instance *nss_ctx;
	nss_tx_status_t ret;
	uint32_t vap_nss_if;
	uint8_t tun_type;
	uint32_t len;

	len = sizeof(struct nss_gre_redir_msg) - sizeof(struct nss_cmn_msg);
	nss_ctx = nss_gre_redir_get_context();
	tun_type = nss_nlgre_redir_cmn_get_tun_type(map_params->tun_type);
	vap_nss_if = nss_nlgre_redir_cmn_get_dev_ifnum(map_params->vap_nss_if);
	if ((vap_nss_if >= NSS_DYNAMIC_IF_START+NSS_MAX_DYNAMIC_INTERFACES) || (vap_nss_if < NSS_DYNAMIC_IF_START)) {
		nss_nl_error("%px: vap_nss_if is out of valid range for vap: %d\n", nss_ctx, vap_nss_if);
		return -1;
	}

	if (map_params->rid >= NSS_NLGRE_REDIR_CMN_RADIO_ID_MAX) {
		nss_nl_error("%px: radio_id is out of valid range for vap: %d\n", nss_ctx, vap_nss_if);
		return -1;
	}

	if (map_params->vid >= NSS_NLGRE_REDIR_CMN_VAP_ID_MAX) {
		nss_nl_error("%px: vap_id is out of valid range for vap: %d\n", nss_ctx, vap_nss_if);
		return -1;
	}

	nss_cmn_msg_init(&ngrm.cm, NSS_GRE_REDIR_INTERFACE, NSS_GRE_REDIR_TX_INTERFACE_MAP_MSG,
			len, nss_nlgre_redir_cmn_map_unmap_msg_cb, NULL);

	ngrm.msg.interface_map.vap_nssif = vap_nss_if;
	ngrm.msg.interface_map.radio_id = map_params->rid;
	ngrm.msg.interface_map.vap_id = map_params->vid;
	ngrm.msg.interface_map.tunnel_type = tun_type;
	ngrm.msg.interface_map.ipsec_pattern = map_params->ipsec_sa_pattern;
	ngrm.msg.interface_map.lag_en = lag_en;
	ngrm.msg.interface_map.nexthop_nssif = nexthop_nssif;

	ret = nss_gre_redir_tx_msg_sync(nss_ctx, &ngrm);
	if (ret != NSS_TX_SUCCESS) {
		nss_nl_error("%px: Tx to firmware failed\n", nss_ctx);
		return -1;
	}

	nss_nl_info("%px: Successfully transmitted msg to firmware\n", nss_ctx);
	return 0;
}

/*
 * nss_nlgre_redir_cmn_unmap_interface()
 *	Interface unmap message.
 */
int nss_nlgre_redir_cmn_unmap_interface(struct nss_nlgre_redir_unmap *unmap_params)
{
	struct nss_ctx_instance *nss_ctx;
	struct nss_gre_redir_msg ngrm;
	uint32_t vap_nss_if, len;
	nss_tx_status_t ret;

	len = sizeof(struct nss_gre_redir_msg) - sizeof(struct nss_cmn_msg);
	vap_nss_if = nss_nlgre_redir_cmn_get_dev_ifnum(unmap_params->vap_nss_if);
	nss_ctx = nss_gre_redir_get_context();

	if ((vap_nss_if >= NSS_DYNAMIC_IF_START+NSS_MAX_DYNAMIC_INTERFACES) || (vap_nss_if < NSS_DYNAMIC_IF_START)) {
		nss_nl_error("%px: vap_nss_if is out of valid range for vap: %d\n", nss_ctx, vap_nss_if);
		return -1;
	}

	if (unmap_params->rid >= NSS_NLGRE_REDIR_CMN_RADIO_ID_MAX) {
		nss_nl_error("%px: radio_id is out of valid range for vap: %d\n", nss_ctx, vap_nss_if);
		return -1;
	}

	if (unmap_params->vid >= NSS_NLGRE_REDIR_CMN_VAP_ID_MAX) {
		nss_nl_error("%px: vap_id is out of valid range for vap: %d\n", nss_ctx, vap_nss_if);
		return -1;
	}

	nss_cmn_msg_init(&ngrm.cm, NSS_GRE_REDIR_INTERFACE, NSS_GRE_REDIR_TX_INTERFACE_UNMAP_MSG,
			len, nss_nlgre_redir_cmn_map_unmap_msg_cb, NULL);
	ngrm.msg.interface_unmap.vap_nssif = vap_nss_if;
	ngrm.msg.interface_unmap.radio_id = unmap_params->rid;
	ngrm.msg.interface_unmap.vap_id = unmap_params->vid;

	ret = nss_gre_redir_tx_msg_sync(nss_ctx, &ngrm);
	if (ret != NSS_TX_SUCCESS) {
		nss_nl_error("%px: Tx to firmware failed\n", nss_ctx);
		return -1;
	}

	nss_nl_info("%px: Successfully transmitted msg to firmware\n", nss_ctx);
	return 0;
}

/*
 * nss_nlgre_redir_cmn_set_next_hop()
 *	Sets next hop as gre-redir for wifi.
 */
int nss_nlgre_redir_cmn_set_next_hop(uint32_t next_dev_ifnum, struct nss_nlgre_redir_set_next *setnext_params)
{
	struct nss_ctx_instance *nss_ctx;
	nss_tx_status_t ret;
	int ifnumber;
	void *ctx;

	nss_ctx = nss_gre_redir_get_context();
	ifnumber = nss_nlgre_redir_cmn_get_dev_ifnum(setnext_params->dev_name);
	if (ifnumber == -1) {
		nss_nl_error("%px: Unable to find NSS interface for net device %s\n", nss_ctx, setnext_params->dev_name);
		return -1;
	}

	nss_nl_info("%px: next hop interface number is %d\n", nss_ctx, next_dev_ifnum);
	ctx = nss_wifili_get_context();

	ret = nss_wifi_vdev_set_next_hop(ctx, ifnumber, next_dev_ifnum);
	if (ret != NSS_TX_SUCCESS) {
		nss_nl_error("%px: wifi drv api failed to set next hop\n", nss_ctx);
		return -1;
	}

	nss_nl_info("%px: Successfully set the next hop\n", nss_ctx);
	return 0;
}
