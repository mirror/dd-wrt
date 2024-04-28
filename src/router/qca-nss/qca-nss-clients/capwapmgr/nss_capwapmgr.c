/*
 **************************************************************************
 * Copyright (c) 2014-2019, The Linux Foundation. All rights reserved.
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
 **************************************************************************
 */

/*
 * nss_capwapmgr.c
 *	NSS to HLOS CAPWAP manager
 */
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/of.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/ipv6.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3,9,0))
#include <net/ipip.h>
#else
#include <net/ip_tunnels.h>
#endif

#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <nss_api_if.h>
#include <linux/in.h>
#include <nss_cmn.h>
#include <nss_capwap.h>
#include <nss_capwapmgr.h>
#include <nss_capwap_user.h>
#include <fal/fal_qos.h>
#include <fal/fal_acl.h>

#define NSS_CAPWAPMGR_NETDEV_NAME	"nsscapwap"

/*
 * This file is responsible for interacting with qca-nss-drv's
 * CAPWAP API to manage CAPWAP tunnels.
 *
 * This driver also exposes few APIs which can be used by
 * another module to perform operations on CAPWAP tunnels. However, we create
 * one netdevice for all the CAPWAP tunnels which is done at the module's
 * init time if NSS_CAPWAPMGR_ONE_NETDEV is set in the Makefile.
 *
 * If your requirement is to create one netdevice per-CAPWAP tunnel, then
 * netdevice needs to be created before CAPWAP tunnel create. Netdevice are
 * created using nss_capwapmgr_netdev_create() API.
 *
 */

/*
 * NSS capwap mgr macros
 */
#define NSS_CAPWAPMGR_NORMAL_FRAME_MTU 1500

/*
 * Ethernet types.
 */
#define NSS_CAPWAPMGR_ETH_TYPE_MASK 0xFFFF
#define NSS_CAPWAPMGR_ETH_TYPE_TRUSTSEC 0x8909
#define NSS_CAPWAPMGR_ETH_TYPE_IPV4 ETH_P_IP
#define NSS_CAPWAPMGR_ETH_TYPE_IPV6 ETH_P_IPV6
#define NSS_CAPWAPMGR_DSCP_MAX 64

/*
 * ACL specific parameters.
 */
#define NSS_CAPWAPMGR_ETH_HDR_OFFSET 6
#define NSS_CAPWAPMGR_IPV4_OFFSET 8
#define NSS_CAPWAPMGR_DSCP_MASK_IPV4_SHIFT 2
#define NSS_CAPWAPMGR_DSCP_MASK_IPV6_SHIFT 6
#define NSS_CAPWAPMGR_DEV_ID 0
#define NSS_CAPWAPMGR_GROUP_ID 0
#define NSS_CAPWAPMGR_RULE_NR 1

/*
 * ACL rule bind bitmap for all physical ports (1 through 6)
 */
#define NSS_CAPWAPMGR_BIND_BITMAP 0x7E

/*
 * The number of rules supported by a list is 4. Since we need 2 rules for every
 * dscp classification (v4 and v6). We set this value to 2.
 */
#define NSS_CAPWAPMGR_ACL_RULES_PER_LIST 2

/*
 * We currently have list-id 60 and 61 reserved for this purpose.
 * TODO: Find a better approach to reserve list-id.
 */
#define NSS_CAPWAPMGR_ACL_LIST_START 60
#define NSS_CAPWAPMGR_ACL_LIST_CNT 2

#define NSS_CAPWAPMGR_NORMAL_FRAME_MTU 1500

#if (NSS_CAPWAPMGR_DEBUG_LEVEL < 1)
#define nss_capwapmgr_assert(fmt, args...)
#else
#define nss_capwapmgr_assert(c) if (!(c)) { BUG_ON(!(c)); }
#endif /* NSS_CAPWAPMGR_DEBUG_LEVEL */

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_capwapmgr_warn(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_capwapmgr_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_capwapmgr_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_CAPWAPMGR_DEBUG_LEVEL < 2)
#define nss_capwapmgr_warn(s, ...)
#else
#define nss_capwapmgr_warn(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_CAPWAPMGR_DEBUG_LEVEL < 3)
#define nss_capwapmgr_info(s, ...)
#else
#define nss_capwapmgr_info(s, ...)   pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_CAPWAPMGR_DEBUG_LEVEL < 4)
#define nss_capwapmgr_trace(s, ...)
#else
#define nss_capwapmgr_trace(s, ...)  pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

/*
 * nss_capwapmgr_ip_response
 *	Response structure for IPv4 and IPv6 messages
 */
static struct nss_capwapmgr_ip_response {
	struct semaphore sem;
	wait_queue_head_t wq;
	enum nss_cmn_response response;
	bool cond;
} ip_response;

/*
 * nss_capwapmgr_acl
 *	Object containing rule related info.
 */
struct nss_capwapmgr_acl {
	bool in_use;			/* Set when rule is in use. */
	uint8_t uid;			/* Unique ID for this rule object. */
	uint8_t list_id;		/* List on which this rule resides. */
	uint8_t rule_id;		/* Rule-id of this rule. */
	uint8_t dscp_value;		/* DSCP value */
	uint8_t dscp_mask;		/* DSCP mask */
};

/*
 * nss_capwapmgr_acl_list
 */
struct nss_capwapmgr_acl_list {
	struct nss_capwapmgr_acl rule[NSS_CAPWAPMGR_ACL_RULES_PER_LIST];
					/* Rules on this ACL list. */
};

/*
 * nss_capwapmgr_global
 *	Global structure for capwapmgr.
 */
static struct nss_capwapmgr_global {
	uint32_t count;				/* Counter for driver queue selection. */
	struct nss_capwap_tunnel_stats tunneld;	/* What tunnels that don't exist any more. */
	struct nss_capwapmgr_acl_list acl_list[NSS_CAPWAPMGR_ACL_LIST_CNT];
						/* Set when ACL rule is in use. */
} global;

static void nss_capwapmgr_receive_pkt(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi);

#if defined(NSS_CAPWAPMGR_ONE_NETDEV)
/*
 * If you want only one netdev for all the tunnels. If you don't want
 * to create one netdev for all the tunnels, then netdev must be
 * created using nss_capwapmgr_netdev_create() before every tunnel create
 * operation.
 */
static struct net_device *nss_capwapmgr_ndev = NULL;
#endif

/*
 * nss_capwapmgr_open()
 *	Netdev's open call.
 */
static int nss_capwapmgr_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

/*
 * nss_capwapmgr_close()
 *	Netdev's close call.
 */
static int nss_capwapmgr_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

/*
 * nss_capwapmgr_decongestion_callback()
 *	Wakeup netif queue if we were stopped by start_xmit
 */
static void nss_capwapmgr_decongestion_callback(void *arg)
{
	struct net_device *dev = arg;

	if (netif_queue_stopped(dev)) {
		netif_wake_queue(dev);
	}
}

/*
 * nss_capwapmgr_start_xmit()
 *	Transmit's skb to NSS FW over CAPWAP if_num.
 *
 * Please make sure to leave headroom of NSS_CAPWAP_HEADROOM with every
 * packet so that NSS can encap eth,vlan,ip,udp,capwap headers.
 * Also, skb->len must include size of metaheader. Essentially skb->len is
 * size of CAPWAP Payload (including wireless info sections) and metaheader.
 */
static netdev_tx_t nss_capwapmgr_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats = &dev->stats;
	struct nss_capwapmgr_priv *priv;
	struct nss_capwap_metaheader *pre;
	uint32_t if_num;
	nss_tx_status_t status;

	priv = netdev_priv(dev);
	pre = (struct nss_capwap_metaheader *)skb->data;

	if (unlikely(pre->tunnel_id > NSS_CAPWAPMGR_MAX_TUNNELS)) {
		nss_capwapmgr_warn("%p: (CAPWAP packet) tunnel-id invalid: %d\n", dev, pre->tunnel_id);
		kfree_skb(skb);
		stats->tx_dropped++;
		return NETDEV_TX_OK;
	}

	if_num = priv->tunnel[pre->tunnel_id].if_num;
	if (unlikely(if_num == 0)) {
		nss_capwapmgr_warn("%p: (CAPWAP packet) if_num in the tunnel not set pre->tunnel_id %d\n", dev,
				pre->tunnel_id);
		kfree_skb(skb);
		stats->tx_dropped++;
		return NETDEV_TX_OK;
	}

	/*
	 * We use the lowest bit in the inner flow_id to determine which Tx ring
	 * to use (drv uses queue mapping to select Tx ring).
	 *
	 * This ring distribution will in turn get used in NSS firmware
	 * for better thread distribution of encap operation.
	 */
	skb_set_queue_mapping(skb, pre->flow_id & 0x1);

	status = nss_capwap_tx_buf(priv->nss_ctx, skb, if_num);
	if (unlikely(status != NSS_TX_SUCCESS)) {
		if (status == NSS_TX_FAILURE_QUEUE) {
			nss_capwapmgr_warn("%p: netdev :%p queue is full", dev, dev);
			if (!netif_queue_stopped(dev)) {
				netif_stop_queue(dev);
			}
		}

		return NETDEV_TX_BUSY;
	}

	return NETDEV_TX_OK;
}

/*
 * nss_capwapmgr_fill_up_stats()
 *	Fills up stats in netdev's stats.
 */
static void nss_capwapmgr_fill_up_stats(struct rtnl_link_stats64 *stats, struct nss_capwap_tunnel_stats *tstats)
{
	stats->rx_packets += tstats->pnode_stats.rx_packets;
	stats->rx_dropped += tstats->pnode_stats.rx_dropped;

	/* rx_fifo_errors will appear as rx overruns in ifconfig */
	stats->rx_fifo_errors += (tstats->rx_queue_full_drops + tstats->rx_n2h_queue_full_drops);
	stats->rx_errors += (tstats->rx_mem_failure_drops + tstats->rx_oversize_drops + tstats->rx_frag_timeout_drops);
	stats->rx_bytes += tstats->pnode_stats.rx_bytes;

	/* tx_fifo_errors  will appear as tx overruns in ifconfig */
	stats->tx_fifo_errors += tstats->tx_queue_full_drops;
	stats->tx_errors += tstats->tx_mem_failure_drops;
	stats->tx_bytes += tstats->pnode_stats.tx_bytes;

	stats->tx_dropped += (tstats->tx_dropped_sg_ref + tstats->tx_dropped_ver_mis + tstats->tx_dropped_unalign
			+ tstats->tx_dropped_hroom + tstats->tx_dropped_dtls + tstats->tx_dropped_nwireless);
	stats->tx_packets += tstats->pnode_stats.tx_packets;
}

/*
 * nss_capwapmgr_get_tunnel_stats()
 *	Netdev get stats function to get tunnel stats
 */
static struct rtnl_link_stats64 *nss_capwapmgr_get_tunnel_stats(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	struct nss_capwap_tunnel_stats tstats;
	int i;

	if (!stats) {
		nss_capwapmgr_warn("%p: invalid rtnl structure\n", dev);
		return stats;
	}

	/*
	 * Netdev seems to be incrementing rx_dropped because we don't give IP header.
	 * So reset it as it's of no use for us.
	 */
	atomic_long_set(&dev->rx_dropped, 0);

	memset(stats, 0, sizeof (struct rtnl_link_stats64));
	nss_capwapmgr_fill_up_stats(stats, &global.tunneld);

	for (i = NSS_DYNAMIC_IF_START; i <= (NSS_DYNAMIC_IF_START + NSS_MAX_DYNAMIC_INTERFACES); i++) {
		if (nss_capwap_get_stats(i, &tstats) == false) {
			continue;
		}

		nss_capwapmgr_fill_up_stats(stats, &tstats);
	}

	return stats;
}

/*
 * nss_capwapmgr_netdev_ops
 *	Netdev operations.
 */
static const struct net_device_ops nss_capwapmgr_netdev_ops = {
	.ndo_open		= nss_capwapmgr_open,
	.ndo_stop		= nss_capwapmgr_close,
	.ndo_start_xmit		= nss_capwapmgr_start_xmit,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_get_stats64	= nss_capwapmgr_get_tunnel_stats,
};

/*
 * nss_capwapmgr_dummpy_netdev_setup()
 *	Netdev setup function.
 */
static void nss_capwapmgr_dummpy_netdev_setup(struct net_device *dev)
{
	dev->addr_len = ETH_ALEN;
	dev->mtu = ETH_DATA_LEN;
	dev->needed_headroom = NSS_CAPWAP_HEADROOM;
	dev->needed_tailroom = 4;
	dev->type = ARPHRD_VOID;
	dev->ethtool_ops = NULL;
	dev->header_ops = NULL;
	dev->netdev_ops = &nss_capwapmgr_netdev_ops;
	dev->destructor = NULL;
	memcpy(dev->dev_addr, "\x00\x00\x00\x00\x00\x00", dev->addr_len);
	memset(dev->broadcast, 0xff, dev->addr_len);
	memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);
}

/*
 * nss_capwapmgr_msg_event_receive()
 *	CAPWAP message callback for responses to commands sent to NSS FW
 *
 * This is command hanlder for all the messages since all we do is wake-up
 * the caller who is sending message to NSS FW.
 */
static void nss_capwapmgr_msg_event_receive(void *app_data, struct nss_capwap_msg *nim)
{
	struct net_device *dev = app_data;
	struct nss_cmn_msg *ncm = (struct nss_cmn_msg *)nim;
	struct nss_capwapmgr_response *r;
	struct nss_capwapmgr_priv *priv;
	uint32_t if_num;

	if (ncm->response == NSS_CMN_RESPONSE_NOTIFY) {
		return;
	}

	/*
	 * Since all CAPWAP messages are sync in nature we need to wake-up caller.
	 */
	if_num = ncm->interface - NSS_DYNAMIC_IF_START;
	dev_hold(dev);
	priv = netdev_priv(dev);
	r = &priv->resp[if_num];

	/*
	 * If somebody is waiting...
	 */
	if (atomic_read(&r->seq) != 0) {
		if (ncm->response != NSS_CMN_RESPONSE_ACK) {
			r->error = ncm->error;
		}

		r->response = ncm->response;
		atomic_dec(&r->seq);
		wake_up(&r->wq);
	}

	dev_put(dev);
}
/*
 * nss_capwapmgr_ip_common_handler()
 *	Common Callback handler for IPv4 and IPv6 messages
 */
static void nss_capwapmgr_ip_common_handler(struct nss_cmn_msg *ncm)
{
	if (ncm->response == NSS_CMN_RESPONSE_NOTIFY) {
		return;
	}

	ip_response.response = ncm->response;
	ip_response.cond = 0;
	wake_up(&ip_response.wq);
}

/*
 * nss_capwapmgr_ipv4_handler()
 *	Callback handler for IPv4 messages
 */
static void nss_capwapmgr_ipv4_handler(void *app_data, struct nss_ipv4_msg *nim)
{
	nss_capwapmgr_ip_common_handler(&nim->cm);
}

/*
 * nss_capwapmgr_ipv6_handler()
 *	Callback handler for IPv4 messages
 */
static void nss_capwapmgr_ipv6_handler(void *app_data, struct nss_ipv6_msg *nim)
{
	nss_capwapmgr_ip_common_handler(&nim->cm);
}

/*
 * nss_capwap_remap_error()
 *	Remaps NSS FW response error to nss_capwapmgr_status_t
 */
static nss_capwapmgr_status_t nss_capwap_remap_error(nss_capwap_msg_response_t error)
{
	nss_capwapmgr_status_t status;

	switch (error) {
	case NSS_CAPWAP_ERROR_MSG_INVALID_REASSEMBLY_TIMEOUT:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_REASSEMBLY_TIMEOUT;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_PATH_MTU:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_PATH_MTU;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_MAX_FRAGMENT:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_MAX_FRAGMENT;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_BUFFER_SIZE:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_BUFFER_SIZE;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_L3_PROTO:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_L3_PROTO;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_UDP_PROTO:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_UDP_PROTO;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_VERSION:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_VERSION;
		break;
	case NSS_CAPWAP_ERROR_MSG_TUNNEL_DISABLED:
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_DISABLED;
		break;
	case NSS_CAPWAP_ERROR_MSG_TUNNEL_ENABLED:
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_ENABLED;
		break;
	case NSS_CAPWAP_ERROR_MSG_TUNNEL_NOT_CFG:
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_NOT_CFG;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_IP_NODE:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_IP_NODE;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_TYPE_FLAG:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_TYPE_FLAG;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_DTLS_CFG:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_DTLS_CFG;
		break;
	default:
		status = NSS_CAPWAPMGR_FAILURE;
	}

	return status;
}

/*
 * nss_capwapmgr_verify_tunnel_param()
 *	Common function to verify tunnel_id and returns pointer to tunnel.
 */
static struct nss_capwapmgr_tunnel *nss_capwapmgr_verify_tunnel_param(struct net_device *dev, uint8_t tunnel_id)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_tunnel *t;

	if (!dev) {
		nss_capwapmgr_warn("Invalid net_device\n");
		return NULL;
	}

	if (tunnel_id > NSS_CAPWAPMGR_MAX_TUNNELS) {
		nss_capwapmgr_warn("%p: tunnel_id: %d out of range (%d)\n", dev, tunnel_id, NSS_CAPWAPMGR_MAX_TUNNELS);
		return NULL;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);
	t = &priv->tunnel[tunnel_id];
	if (t->if_num == 0) {
		dev_put(dev);
		return NULL;
	}

	dev_put(dev);
	return t;
}

/*
 * nss_capwapmgr_netdev_create()
 *	API to create a CAPWAP netdev
 */
struct net_device *nss_capwapmgr_netdev_create()
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_response *r;
	struct net_device *ndev;
	int i;
	int err;

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 16, 0))
	ndev = alloc_netdev(sizeof(struct nss_capwapmgr_priv),
					"nsscapwap%d", nss_capwapmgr_dummpy_netdev_setup);
#else
	ndev = alloc_netdev(sizeof(struct nss_capwapmgr_priv),
					"nsscapwap%d", NET_NAME_ENUM, nss_capwapmgr_dummpy_netdev_setup);
#endif
	if (!ndev) {
		nss_capwapmgr_warn("Error allocating netdev\n");
		return NULL;
	}

	err = register_netdev(ndev);
	if (err) {
		nss_capwapmgr_warn("register_netdev() fail with error :%d\n", err);
		free_netdev(ndev);
		return NULL;
	}

	priv = netdev_priv(ndev);
	priv->nss_ctx = nss_capwap_get_ctx();
	priv->tunnel = kmalloc(sizeof(struct nss_capwapmgr_tunnel) * NSS_CAPWAPMGR_MAX_TUNNELS, GFP_ATOMIC);
	if (!priv->tunnel) {
		nss_capwapmgr_warn("%p: failed to allocate tunnel memory\n", ndev);
		goto fail1;
	}
	memset(priv->tunnel, 0, sizeof(struct nss_capwapmgr_tunnel) * NSS_CAPWAPMGR_MAX_TUNNELS);

	priv->resp = kmalloc(sizeof(struct nss_capwapmgr_response) * NSS_MAX_DYNAMIC_INTERFACES, GFP_ATOMIC);
	if (!priv->resp) {
		nss_capwapmgr_warn("%p: failed to allocate tunnel response memory\n", ndev);
		goto fail2;
	}
	for (i = 0; i < NSS_MAX_DYNAMIC_INTERFACES; i++) {
		r = &priv->resp[i];
		init_waitqueue_head(&r->wq);

		/*
		 * CAPWAP interface is limited to one command per-tunnel.
		 */
		sema_init(&r->sem, 1);
	}

	priv->if_num_to_tunnel_id = kmalloc(sizeof(uint8_t) * NSS_MAX_NET_INTERFACES, GFP_ATOMIC);
	if (!priv->if_num_to_tunnel_id) {
		nss_capwapmgr_warn("%p: failed to allocate if_num to tunnel_id memory\n", ndev);
		goto fail3;
	}
	memset(priv->if_num_to_tunnel_id, 0, sizeof(uint8_t) * NSS_MAX_NET_INTERFACES);

	if (nss_cmn_register_queue_decongestion(priv->nss_ctx, nss_capwapmgr_decongestion_callback, ndev) != NSS_CB_REGISTER_SUCCESS) {
		nss_capwapmgr_warn("%p: failed to register decongestion callback\n", ndev);
		goto fail4;
	}

	return ndev;
fail4:
	kfree(priv->if_num_to_tunnel_id);
fail3:
	kfree(priv->resp);
fail2:
	kfree(priv->tunnel);
fail1:
	unregister_netdev(ndev);
	free_netdev(ndev);
	return NULL;
}
EXPORT_SYMBOL(nss_capwapmgr_netdev_create);

/*
 * nss_capwapmgr_netdev_destroy()
 *	API for destroying a netdevice.
 *
 * All the CAPWAP tunnels must be destroyed first before netdevice.
 */
nss_capwapmgr_status_t nss_capwapmgr_netdev_destroy(struct net_device *dev)
{
	rtnl_is_locked() ? unregister_netdevice(dev) : unregister_netdev(dev);
	return NSS_CAPWAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_capwapmgr_netdev_destroy);

/*
 * nss_capwapmgr_register_with_nss()
 *	Internal function to register with NSS FW.
 */
static nss_capwapmgr_status_t nss_capwapmgr_register_with_nss(uint32_t interface_num, struct net_device *dev)
{
	struct nss_ctx_instance *ctx;

	/* features denote the skb_types supported */
	uint32_t features = 0;

	ctx = nss_capwap_data_register(interface_num, nss_capwapmgr_receive_pkt, dev, features);
	if (!ctx) {
		nss_capwapmgr_warn("%p: %d: nss_capwapmgr_data_register failed\n", dev, interface_num);
		return NSS_CAPWAPMGR_FAILURE;
	}

	return NSS_CAPWAPMGR_SUCCESS;
}

/*
 * nss_capwapmgr_unregister_with_nss()
 *	Internal function to unregister with NSS FW
 */
static void nss_capwapmgr_unregister_with_nss(uint32_t if_num)
{
	nss_capwapmgr_trace("%d: unregister with NSS FW\n", if_num);
	nss_capwap_data_unregister(if_num);
}

/*
 * nss_capwapmgr_destroy_ipv4_rule()
 *	Destroy a given connection in the NSS
 */
static nss_tx_status_t nss_capwapmgr_destroy_ipv4_rule(void *ctx, struct nss_ipv4_destroy *unid)
{
	struct nss_ctx_instance *nss_ctx = (struct nss_ctx_instance *) ctx;
	struct nss_ipv4_msg nim;
	struct nss_ipv4_rule_destroy_msg *nirdm;
	nss_tx_status_t status;

	nss_capwapmgr_info("%p: ctx: Destroy IPv4: %pI4h:%d, %pI4h:%d, p: %d\n", nss_ctx,
		&unid->src_ip, ntohs(unid->src_port), &unid->dest_ip, ntohs(unid->dest_port), unid->protocol);

	nss_ipv4_msg_init(&nim, NSS_IPV4_RX_INTERFACE, NSS_IPV4_TX_DESTROY_RULE_MSG,
			sizeof(struct nss_ipv4_rule_destroy_msg), nss_capwapmgr_ipv4_handler, NULL);

	nirdm = &nim.msg.rule_destroy;

	nirdm->tuple.protocol = (uint8_t)unid->protocol;
	nirdm->tuple.flow_ip = unid->src_ip;
	nirdm->tuple.flow_ident = (uint32_t)unid->src_port;
	nirdm->tuple.return_ip = unid->dest_ip;
	nirdm->tuple.return_ident = (uint32_t)unid->dest_port;

	down(&ip_response.sem);
	status = nss_ipv4_tx(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		up(&ip_response.sem);
		nss_capwapmgr_warn("%p: Destroy IPv4 message failed %d\n", ctx, status);
		return status;
	}

	ip_response.cond = 1;
	if (!wait_event_timeout(ip_response.wq, ip_response.cond == 0, 5 * HZ)) {
		nss_capwapmgr_warn("%p: Destroy IPv4 command msg response timeout\n", ctx);
		status = NSS_TX_FAILURE;
	} else if (ip_response.response != NSS_CMN_RESPONSE_ACK) {
		nss_capwapmgr_warn("%p: Destroy IPv4 command msg failed with response : %d\n", ctx, ip_response.response);
		status = NSS_TX_FAILURE;
	}

	up(&ip_response.sem);
	return status;
}

/*
 * nss_capwapmgr_unconfigure_ipv4_rule()
 *	Internal function to unconfigure IPv4 rule.
 */
static nss_tx_status_t nss_capwapmgr_unconfigure_ipv4_rule(struct nss_ipv4_destroy *destroy)
{
	void *ctx;

	ctx = nss_ipv4_get_mgr();
	if (!ctx) {
		nss_capwapmgr_warn("%s: couldn't get IPv4 ctx\n", "CAPWAP");
		return NSS_TX_FAILURE_NOT_READY;
	}

	return nss_capwapmgr_destroy_ipv4_rule(ctx, destroy);
}

/*
 * nss_capwapmgr_unconfigure_ipv6_rule()
 *	Internal function to unconfigure IPv6 rule.
 */
static nss_tx_status_t nss_capwapmgr_unconfigure_ipv6_rule(struct nss_ipv6_destroy *unid)
{
	struct nss_ctx_instance *nss_ctx;
	struct nss_ipv6_msg nim;
	struct nss_ipv6_rule_destroy_msg *nirdm;
	nss_tx_status_t status;

	nss_ctx = nss_ipv6_get_mgr();
	if (!nss_ctx) {
		nss_capwapmgr_warn("%s: couldn't get IPv6 ctx\n", "CAPWAP");
		return NSS_TX_FAILURE_NOT_READY;
	}

	nss_capwapmgr_info("%p: ctx: Destroy IPv4: %x:%d, %x:%d, p: %d\n", nss_ctx,
		unid->src_ip[0], ntohs(unid->src_port), unid->dest_ip[0], ntohs(unid->dest_port), unid->protocol);

	nss_ipv6_msg_init(&nim, NSS_IPV6_RX_INTERFACE, NSS_IPV6_TX_DESTROY_RULE_MSG,
			sizeof(struct nss_ipv6_rule_destroy_msg), nss_capwapmgr_ipv6_handler, NULL);

	nirdm = &nim.msg.rule_destroy;

	nirdm->tuple.protocol = (uint8_t)unid->protocol;
	nirdm->tuple.flow_ident = (uint32_t)unid->src_port;
	nirdm->tuple.flow_ip[0] = unid->src_ip[0];
	nirdm->tuple.flow_ip[1] = unid->src_ip[1];
	nirdm->tuple.flow_ip[2] = unid->src_ip[2];
	nirdm->tuple.flow_ip[3] = unid->src_ip[3];

	nirdm->tuple.return_ident = (uint32_t)unid->dest_port;
	nirdm->tuple.return_ip[0] = unid->dest_ip[0];
	nirdm->tuple.return_ip[1] = unid->dest_ip[1];
	nirdm->tuple.return_ip[2] = unid->dest_ip[2];
	nirdm->tuple.return_ip[3] = unid->dest_ip[3];

	down(&ip_response.sem);
	status = nss_ipv6_tx(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		up(&ip_response.sem);
		nss_capwapmgr_warn("%p: Destroy IPv6 message failed %d\n", nss_ctx, status);
		return status;
	}

	ip_response.cond = 1;
	if (!wait_event_timeout(ip_response.wq, ip_response.cond == 0, 5 * HZ)) {
		nss_capwapmgr_warn("%p: Destroy IPv6 command msg response timeout\n", nss_ctx);
		status = NSS_TX_FAILURE;
	} else if (ip_response.response != NSS_CMN_RESPONSE_ACK) {
		nss_capwapmgr_warn("%p: Destroy IPv6 command msg failed with response : %d\n", nss_ctx, ip_response.response);
		status = NSS_TX_FAILURE;
	}

	up(&ip_response.sem);
	return status;
}

/*
 * nss_capwapmgr_create_ipv4_rule()
 *	Create a nss entry to accelerate the given connection
 */
static nss_tx_status_t nss_capwapmgr_create_ipv4_rule(void *ctx, struct nss_ipv4_create *unic, uint16_t rule_flags, uint16_t valid_flags)
{
	struct nss_ctx_instance *nss_ctx = (struct nss_ctx_instance *) ctx;
	struct nss_ipv4_msg nim;
	struct nss_ipv4_rule_create_msg *nircm;
	nss_tx_status_t status;

	nss_capwapmgr_info("%p: ctx: Create IPv4: %pI4h:%d (%pI4h:%d), %pI4h:%d (%pI4h:%d), p: %d\n", nss_ctx,
		&unic->src_ip, unic->src_port, &unic->src_ip_xlate, unic->src_port_xlate,
		&unic->dest_ip, unic->dest_port, &unic->dest_ip_xlate, unic->dest_port_xlate,
		unic->protocol);

	memset(&nim, 0, sizeof (struct nss_ipv4_msg));
	nss_ipv4_msg_init(&nim, NSS_IPV4_RX_INTERFACE, NSS_IPV4_TX_CREATE_RULE_MSG,
			sizeof(struct nss_ipv4_rule_create_msg), nss_capwapmgr_ipv4_handler, NULL);

	nircm = &nim.msg.rule_create;
	nircm->valid_flags = 0;
	nircm->rule_flags = 0;

	/*
	 * Copy over the 5 tuple details.
	 */
	nircm->tuple.protocol = (uint8_t)unic->protocol;
	nircm->tuple.flow_ip = unic->src_ip;
	nircm->tuple.flow_ident = (uint32_t)unic->src_port;
	nircm->tuple.return_ip = unic->dest_ip;
	nircm->tuple.return_ident = (uint32_t)unic->dest_port;

	/*
	 * Copy over the connection rules and set the CONN_VALID flag
	 */
	nircm->conn_rule.flow_interface_num = unic->src_interface_num;
	nircm->conn_rule.flow_mtu = unic->from_mtu;
	nircm->conn_rule.flow_ip_xlate = unic->src_ip_xlate;
	nircm->conn_rule.flow_ident_xlate = (uint32_t)unic->src_port_xlate;
	memcpy(nircm->conn_rule.flow_mac, unic->src_mac, 6);
	nircm->conn_rule.return_interface_num = unic->dest_interface_num;
	nircm->conn_rule.return_mtu = unic->to_mtu;
	nircm->conn_rule.return_ip_xlate = unic->dest_ip_xlate;
	nircm->conn_rule.return_ident_xlate = (uint32_t)unic->dest_port_xlate;
	if (nircm->tuple.return_ip != nircm->conn_rule.return_ip_xlate ||
		nircm->tuple.return_ident != nircm->conn_rule.return_ident_xlate) {
		memcpy(nircm->conn_rule.return_mac, unic->dest_mac_xlate, 6);
	} else {
		memcpy(nircm->conn_rule.return_mac, unic->dest_mac, 6);
	}

	/*
	 * Copy over the DSCP rule parameters
	 */
	if (unic->flags & NSS_IPV4_CREATE_FLAG_DSCP_MARKING) {
		nircm->dscp_rule.flow_dscp = unic->flow_dscp;
		nircm->dscp_rule.return_dscp = unic->return_dscp;
		nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_DSCP_MARKING;
		nircm->valid_flags |= NSS_IPV4_RULE_CREATE_DSCP_MARKING_VALID;
	}

	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_CONN_VALID;

	/*
	 * Copy over the pppoe rules and set the PPPOE_VALID flag.
	 */
	nircm->pppoe_rule.flow_if_exist = unic->flow_pppoe_if_exist;
	nircm->pppoe_rule.flow_if_num = unic->flow_pppoe_if_num;
	nircm->pppoe_rule.return_if_exist = unic->return_pppoe_if_exist;
	nircm->pppoe_rule.return_if_num = unic->return_pppoe_if_num;
	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_PPPOE_VALID;

	/*
	 * Copy over the vlan rules and set the VLAN_VALID flag
	 */
	nircm->vlan_primary_rule.ingress_vlan_tag = unic->in_vlan_tag[0];
	nircm->vlan_primary_rule.egress_vlan_tag = unic->out_vlan_tag[0];
	nircm->vlan_secondary_rule.ingress_vlan_tag = unic->in_vlan_tag[1];
	nircm->vlan_secondary_rule.egress_vlan_tag = unic->out_vlan_tag[1];
	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_VLAN_VALID;

	/*
	 * Copy over the qos rules and set the QOS_VALID flag
	 */
	if (unic->flags & NSS_IPV6_CREATE_FLAG_QOS_VALID) {
		nircm->qos_rule.flow_qos_tag = unic->flow_qos_tag;
		nircm->qos_rule.return_qos_tag = unic->return_qos_tag;
		nircm->valid_flags |= NSS_IPV4_RULE_CREATE_QOS_VALID;
	}

	if (unic->flags & NSS_IPV4_CREATE_FLAG_NO_SEQ_CHECK) {
		nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_NO_SEQ_CHECK;
	}

	if (unic->flags & NSS_IPV4_CREATE_FLAG_BRIDGE_FLOW) {
		nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_BRIDGE_FLOW;
	}

	if (unic->flags & NSS_IPV4_CREATE_FLAG_ROUTED) {
		nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_ROUTED;
	}

	/*
	 * Set the flag NSS_IPV4_RULE_CREATE_FLAG_ICMP_NO_CME_FLUSH so that
	 * rule is not flushed when NSS FW receives ICMP errors/packets.
	 */
	nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_ICMP_NO_CME_FLUSH;

	/*
	 * Add any other additional flags which caller has requested.
	 * For example: update MTU, update destination MAC address.
	 */
	nircm->rule_flags |= rule_flags;
	nircm->valid_flags |= valid_flags;

	down(&ip_response.sem);
	status = nss_ipv4_tx(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		up(&ip_response.sem);
		nss_capwapmgr_warn("%p: Create IPv4 message failed %d\n", ctx, status);
		return status;
	}

	ip_response.cond = 1;
	if (!wait_event_timeout(ip_response.wq, ip_response.cond == 0, 5 * HZ)) {
		nss_capwapmgr_warn("%p: Create IPv4 command msg response timeout\n", ctx);
		status = NSS_TX_FAILURE;
	} else if (ip_response.response != NSS_CMN_RESPONSE_ACK) {
		nss_capwapmgr_warn("%p: Create IPv4 command msg failed with response: %d\n", ctx, ip_response.response);
		status = NSS_TX_FAILURE;
	}

	up(&ip_response.sem);
	return status;
}

/*
 * nss_capwapmgr_create_ipv6_rule()
 *	Create a nss entry to accelerate the given connection
 */
static nss_tx_status_t nss_capwapmgr_create_ipv6_rule(void *ctx, struct nss_ipv6_create *unic, uint16_t rule_flags, uint16_t valid_flags)
{
	struct nss_ctx_instance *nss_ctx = (struct nss_ctx_instance *) ctx;
	struct nss_ipv6_msg nim;
	struct nss_ipv6_rule_create_msg *nircm;
	nss_tx_status_t status;

	nss_capwapmgr_info("%p: Create IPv6: %pI6:%d, %pI6:%d, p: %d\n", nss_ctx,
		unic->src_ip, unic->src_port, unic->dest_ip, unic->dest_port, unic->protocol);

	memset(&nim, 0, sizeof (struct nss_ipv6_msg));
	nss_ipv6_msg_init(&nim, NSS_IPV6_RX_INTERFACE, NSS_IPV6_TX_CREATE_RULE_MSG,
			sizeof(struct nss_ipv6_rule_create_msg), nss_capwapmgr_ipv6_handler, NULL);

	nircm = &nim.msg.rule_create;

	nircm->rule_flags = 0;
	nircm->valid_flags = 0;

	/*
	 * Copy over the 5 tuple information.
	 */
	nircm->tuple.protocol = (uint8_t)unic->protocol;
	nircm->tuple.flow_ip[0] = unic->src_ip[0];
	nircm->tuple.flow_ip[1] = unic->src_ip[1];
	nircm->tuple.flow_ip[2] = unic->src_ip[2];
	nircm->tuple.flow_ip[3] = unic->src_ip[3];
	nircm->tuple.flow_ident = (uint32_t)unic->src_port;

	nircm->tuple.return_ip[0] = unic->dest_ip[0];
	nircm->tuple.return_ip[1] = unic->dest_ip[1];
	nircm->tuple.return_ip[2] = unic->dest_ip[2];
	nircm->tuple.return_ip[3] = unic->dest_ip[3];
	nircm->tuple.return_ident = (uint32_t)unic->dest_port;

	/*
	 * Copy over the connection rules and set CONN_VALID flag
	 */
	nircm->conn_rule.flow_interface_num = unic->src_interface_num;
	nircm->conn_rule.flow_mtu = unic->from_mtu;
	nircm->conn_rule.return_interface_num = unic->dest_interface_num;
	nircm->conn_rule.return_mtu = unic->to_mtu;
	memcpy(nircm->conn_rule.flow_mac, unic->src_mac, 6);
	memcpy(nircm->conn_rule.return_mac, unic->dest_mac, 6);
	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_CONN_VALID;

	/*
	 * Copy over the DSCP rule parameters
	 */
	if (unic->flags & NSS_IPV6_CREATE_FLAG_DSCP_MARKING) {
		nircm->dscp_rule.flow_dscp = unic->flow_dscp;
		nircm->dscp_rule.return_dscp = unic->return_dscp;
		nircm->rule_flags |= NSS_IPV6_RULE_CREATE_FLAG_DSCP_MARKING;
		nircm->valid_flags |= NSS_IPV6_RULE_CREATE_DSCP_MARKING_VALID;
	}

	/*
	 * Copy over the pppoe rules and set PPPOE_VALID flag
	 */
	nircm->pppoe_rule.flow_if_exist = unic->flow_pppoe_if_exist;
	nircm->pppoe_rule.flow_if_num = unic->flow_pppoe_if_num;
	nircm->pppoe_rule.return_if_exist = unic->return_pppoe_if_exist;
	nircm->pppoe_rule.return_if_num = unic->return_pppoe_if_num;
	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_PPPOE_VALID;

	/*
	 * Copy over the tcp rules and set TCP_VALID flag
	 */
	nircm->tcp_rule.flow_window_scale = unic->flow_window_scale;
	nircm->tcp_rule.flow_max_window = unic->flow_max_window;
	nircm->tcp_rule.flow_end = unic->flow_end;
	nircm->tcp_rule.flow_max_end = unic->flow_max_end;
	nircm->tcp_rule.return_window_scale = unic->return_window_scale;
	nircm->tcp_rule.return_max_window = unic->return_max_window;
	nircm->tcp_rule.return_end = unic->return_end;
	nircm->tcp_rule.return_max_end = unic->return_max_end;
	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_TCP_VALID;

	/*
	 * Copy over the vlan rules and set the VLAN_VALID flag
	 */
	nircm->vlan_primary_rule.egress_vlan_tag = unic->out_vlan_tag[0];
	nircm->vlan_primary_rule.ingress_vlan_tag = unic->in_vlan_tag[0];
	nircm->vlan_secondary_rule.egress_vlan_tag = unic->out_vlan_tag[1];
	nircm->vlan_secondary_rule.ingress_vlan_tag = unic->in_vlan_tag[1];
	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_VLAN_VALID;

	/*
	 * Copy over the qos rules and set the QOS_VALID flag
	 */
	if (unic->flags & NSS_IPV6_CREATE_FLAG_QOS_VALID) {
		nircm->qos_rule.flow_qos_tag = unic->flow_qos_tag;
		nircm->qos_rule.return_qos_tag = unic->return_qos_tag;
		nircm->valid_flags |= NSS_IPV6_RULE_CREATE_QOS_VALID;
	}

	if (unic->flags & NSS_IPV6_CREATE_FLAG_NO_SEQ_CHECK) {
		nircm->rule_flags |= NSS_IPV6_RULE_CREATE_FLAG_NO_SEQ_CHECK;
	}

	if (unic->flags & NSS_IPV6_CREATE_FLAG_BRIDGE_FLOW) {
		nircm->rule_flags |= NSS_IPV6_RULE_CREATE_FLAG_BRIDGE_FLOW;
	}

	if (unic->flags & NSS_IPV6_CREATE_FLAG_ROUTED) {
		nircm->rule_flags |= NSS_IPV6_RULE_CREATE_FLAG_ROUTED;
	}

	/*
	 * Set the flag NSS_IPV6_RULE_CREATE_FLAG_ICMP_NO_CME_FLUSH so that
	 * rule is not flushed when NSS FW receives ICMP errors/packets.
	 */
	nircm->rule_flags |= NSS_IPV6_RULE_CREATE_FLAG_ICMP_NO_CME_FLUSH;

	/*
	 * Add any other additional flags which caller has requested.
	 * For example: update MTU, Update destination MAC address.
	 */
	nircm->rule_flags |= rule_flags;
	nircm->valid_flags |= valid_flags;

	status = nss_ipv6_tx(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		up(&ip_response.sem);
		nss_capwapmgr_warn("%p: Create IPv6 message failed %d\n", ctx, status);
		return status;
	}

	ip_response.cond = 1;
	if (!wait_event_timeout(ip_response.wq, ip_response.cond == 0, 5 * HZ)) {
		nss_capwapmgr_warn("%p: Create IPv6 command msg response timeout\n", ctx);
		status = NSS_TX_FAILURE;
	} else if (ip_response.response != NSS_CMN_RESPONSE_ACK) {
		nss_capwapmgr_warn("%p: Create IPv6 command msg failed with response: %d\n", ctx, ip_response.response);
		status = NSS_TX_FAILURE;
	}

	up(&ip_response.sem);
	return status;
}

/*
 * nss_capwapmgr_configure_ipv4()
 *	Internal function for configuring IPv4 connection
 */
static nss_tx_status_t nss_capwapmgr_configure_ipv4(struct nss_ipv4_create *pcreate, uint16_t rule_flags, uint16_t valid_flags)
{
	nss_tx_status_t status;
	void *ctx;

	ctx = nss_ipv4_get_mgr();
	if (!ctx) {
		nss_capwapmgr_warn("%s couldn't get IPv4 ctx\n", "CAPWAP");
		return NSS_TX_FAILURE_NOT_READY;
	}

	status = nss_capwapmgr_create_ipv4_rule(ctx, pcreate, rule_flags, valid_flags);
	if (status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%p: ctx: nss_ipv4_tx() failed with %d\n", ctx, status);
		return status;
	}

	return NSS_TX_SUCCESS;
}

/*
 * nss_capwapmgr_configure_ipv6()
 *	Internal function for configuring IPv4 connection
 */
static nss_tx_status_t nss_capwapmgr_configure_ipv6(struct nss_ipv6_create *pcreate, uint16_t rule_flags, uint16_t valid_flags)
{
	nss_tx_status_t status;
	void *ctx;

	ctx = nss_ipv6_get_mgr();
	if (!ctx) {
		nss_capwapmgr_warn("%s couldn't get IPv6 ctx\n", "CAPWAP");
		return NSS_TX_FAILURE_NOT_READY;
	}

	status = nss_capwapmgr_create_ipv6_rule(ctx, pcreate, rule_flags, valid_flags);
	if (status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%p: ctx: nss_ipv6_tx() failed with %d\n", ctx, status);
		return status;
	}

	return NSS_TX_SUCCESS;
}

/*
 * nss_capwapmgr_tx_msg_sync()
 *	Waits for message to return.
 */
static nss_capwapmgr_status_t nss_capwapmgr_tx_msg_sync(struct nss_ctx_instance *ctx, struct net_device *dev, struct nss_capwap_msg *msg)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_response *r;
	uint32_t if_num;
	nss_capwapmgr_status_t status;

	if_num = msg->cm.interface - NSS_DYNAMIC_IF_START;
	dev_hold(dev);
	priv = netdev_priv(dev);
	r = &priv->resp[if_num];
	down(&r->sem);
	r->response = NSS_CMN_RESPONSE_ACK;
	atomic_set(&r->seq, 1);		/* Indicate that we are waiting */

	/*
	 * Call NSS driver
	 */
	status = nss_capwap_tx_msg(ctx, msg);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		up(&r->sem);
		dev_put(dev);
		return status;
	}

	if (!wait_event_timeout(r->wq, atomic_read(&r->seq) == 0, 5 * HZ)) {
		atomic_set(&r->seq, 0);		/* Indicate that we are no longer waiting */
		up(&r->sem);
		nss_capwapmgr_warn("%p: CAPWAP command msg response timeout\n", ctx);
		dev_put(dev);
		return NSS_CAPWAPMGR_FAILURE_CMD_TIMEOUT;
	}

	/*
	 * If NSS FW responded back with an error.
	 */
	if (r->response != NSS_CMN_RESPONSE_ACK) {
		up(&r->sem);
		nss_capwapmgr_warn("%p: CAPWAP command msg response : %d, error:%d\n", ctx,
				r->response, r->error);
		dev_put(dev);
		return nss_capwap_remap_error(r->error);
	}

	up(&r->sem);
	dev_put(dev);
	return status;
}

/*
 * nss_capwapmgr_create_capwap_rule()
 *	Internal function to create a CAPWAP rule
 */
static nss_capwapmgr_status_t nss_capwapmgr_create_capwap_rule(struct net_device *dev,
	uint32_t if_num, struct nss_capwap_rule_msg *msg, uint16_t type_flags)
{
	struct nss_ctx_instance *ctx = nss_capwap_get_ctx();
	struct nss_capwap_msg capwapmsg;
	struct nss_capwap_rule_msg *capwapcfg;
	nss_tx_status_t status;

	nss_capwapmgr_info("%p: ctx: CAPWAP Rule src_port: 0x%d dest_port:0x%d\n", ctx,
	    ntohl(msg->encap.src_port), ntohl(msg->encap.dest_port));

	/*
	 * Verify CAPWAP rule parameters.
	 */
	if (ntohl(msg->decap.reassembly_timeout) > NSS_CAPWAP_MAX_REASSEMBLY_TIMEOUT) {
		nss_capwapmgr_warn("%p: invalid reassem timeout: %d, max: %d\n",
			ctx, ntohl(msg->decap.reassembly_timeout), NSS_CAPWAP_MAX_REASSEMBLY_TIMEOUT);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (msg->decap.reassembly_timeout == 0) {
		msg->decap.reassembly_timeout = htonl(10);	/* 10 milli-seconds */
	}

	if (ntohl(msg->decap.max_fragments) > NSS_CAPWAP_MAX_FRAGMENTS) {
		nss_capwapmgr_warn("%p: invalid fragment setting: %d, max: %d\n",
			ctx, ntohl(msg->decap.max_fragments), NSS_CAPWAP_MAX_FRAGMENTS);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (msg->decap.max_fragments == 0) {
		msg->decap.max_fragments = htonl(NSS_CAPWAP_MAX_FRAGMENTS);
	}

	if (ntohl(msg->decap.max_buffer_size) > NSS_CAPWAP_MAX_BUFFER_SIZE) {
		nss_capwapmgr_warn("%p: invalid buffer size: %d, max: %d\n",
			ctx, ntohl(msg->decap.max_buffer_size), NSS_CAPWAP_MAX_BUFFER_SIZE);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (msg->decap.max_buffer_size == 0) {
		msg->decap.max_buffer_size = htonl(nss_capwap_get_max_buf_size(ctx));
	}

	if (ntohl(msg->encap.path_mtu) > NSS_CAPWAP_MAX_MTU) {
		nss_capwapmgr_warn("%p: invalid path_mtu: %d, max: %d\n",
			ctx, ntohl(msg->encap.path_mtu), NSS_CAPWAP_MAX_MTU);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (msg->encap.path_mtu == 0) {
		msg->encap.path_mtu = htonl(NSS_CAPWAPMGR_NORMAL_FRAME_MTU);
	}

	msg->type_flags = type_flags;

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));
	capwapcfg = &capwapmsg.msg.rule;
	memcpy(capwapcfg, msg, sizeof(struct nss_capwap_rule_msg));

	/*
	 * Send CAPWAP tunnel create command to NSS
	 */
	nss_capwap_msg_init(&capwapmsg, if_num, NSS_CAPWAP_MSG_TYPE_CFG_RULE,
			sizeof(struct nss_capwap_rule_msg),
			nss_capwapmgr_msg_event_receive, dev);

	status = nss_capwapmgr_tx_msg_sync(ctx, dev, &capwapmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%p: ctx: create encap data tunnel error %d \n", ctx, status);
		return status;
	}

	return NSS_CAPWAPMGR_SUCCESS;
}

/*
 * nss_capwapmgr_get_dtls_netdev()
 *	API for getting the dtls netdev associated to the capwap tunnel
 */
struct net_device *nss_capwapmgr_get_dtls_netdev(struct net_device *capwap_dev, uint8_t tunnel_id)
{
	struct nss_capwapmgr_tunnel *t;
	struct net_device *dtls_dev;

	t = nss_capwapmgr_verify_tunnel_param(capwap_dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_warn("%p: can't find tunnel: %d\n", capwap_dev, tunnel_id);
		return NULL;
	}

	dtls_dev = t->dtls_dev;

	dev_hold(dtls_dev);
	return dtls_dev;
}
EXPORT_SYMBOL(nss_capwapmgr_get_dtls_netdev);

/*
 * nss_capwapmgr_update_path_mtu()
 *	API for updating Path MTU
 */
nss_capwapmgr_status_t nss_capwapmgr_update_path_mtu(struct net_device *dev, uint8_t tunnel_id, uint32_t mtu)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwap_msg capwapmsg;
	struct nss_capwapmgr_tunnel *t;
	nss_capwapmgr_status_t status;
	nss_tx_status_t nss_status;

	if (mtu > NSS_CAPWAP_MAX_MTU) {
		nss_capwapmgr_warn("%p: invalid path_mtu: %d, max: %d\n", dev, mtu, NSS_CAPWAP_MAX_MTU);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_warn("%p: can't find tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);
	nss_capwapmgr_info("%p: %d: tunnel update MTU is being called\n", dev, t->if_num);

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));

	/*
	 * Send CAPWAP data tunnel command to NSS
	 */
	nss_capwap_msg_init(&capwapmsg, t->if_num, NSS_CAPWAP_MSG_TYPE_UPDATE_PATH_MTU,
		sizeof(struct nss_capwap_path_mtu_msg), nss_capwapmgr_msg_event_receive, dev);
	capwapmsg.msg.mtu.path_mtu = htonl(mtu);
	status = nss_capwapmgr_tx_msg_sync(priv->nss_ctx, dev, &capwapmsg);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%p: Update Path MTU CAPWAP tunnel error : %d \n", dev, status);
		dev_put(dev);
		return NSS_CAPWAPMGR_FAILURE_CAPWAP_RULE;
	}

	/*
	 * Update the IPv4/IPv6 rule with the new MTU for flow and return
	 * TODO: Change rule flag to valid flag
	 */
	if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
		struct nss_ipv4_create *v4;

		v4 = &t->ip_rule.v4;
		v4->from_mtu = v4->to_mtu = mtu;
		nss_status = nss_capwapmgr_configure_ipv4(v4, NSS_IPV4_RULE_UPDATE_FLAG_CHANGE_MTU, 0);
		if (nss_status != NSS_TX_SUCCESS) {
			v4->from_mtu = v4->to_mtu = ntohl(t->capwap_rule.encap.path_mtu);
		}
	} else {
		struct nss_ipv6_create *v6;

		v6 = &t->ip_rule.v6;
		v6->from_mtu = v6->to_mtu = mtu;
		nss_status = nss_capwapmgr_configure_ipv6(v6, NSS_IPV6_RULE_UPDATE_FLAG_CHANGE_MTU, 0);
		if (nss_status != NSS_TX_SUCCESS) {
			v6->from_mtu = v6->to_mtu = ntohl(t->capwap_rule.encap.path_mtu);
		}
	}

	if (nss_status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%p: Update Path MTU IP RULE tunnel error : %d \n", dev, nss_status);
		capwapmsg.msg.mtu.path_mtu = t->capwap_rule.encap.path_mtu;
		status = nss_capwapmgr_tx_msg_sync(priv->nss_ctx, dev, &capwapmsg);
		if (status != NSS_CAPWAPMGR_SUCCESS) {
			nss_capwapmgr_warn("%p: Restore Path MTU CAPWAP tunnel error : %d \n", dev, status);
		}

		dev_put(dev);
		return NSS_CAPWAPMGR_FAILURE_IP_RULE;
	}

	t->capwap_rule.encap.path_mtu = htonl(mtu);
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_update_path_mtu);

/*
 * nss_capwapmgr_update_dest_mac_addr()
 *	API for updating Destination Mac Addr
 */
nss_capwapmgr_status_t nss_capwapmgr_update_dest_mac_addr(struct net_device *dev, uint8_t tunnel_id, uint8_t *mac_addr)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_tunnel *t;
	nss_capwapmgr_status_t status;
	nss_tx_status_t nss_status;
	struct nss_ipv6_create *v6;
	uint8_t mac_addr_old[ETH_ALEN];

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_warn("%p: can't find tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);
	nss_capwapmgr_info("%p: %d: tunnel update mac Addr is being called\n", dev, t->if_num);

	/*
	 * Update the IPv4/IPv6 rule with the new destination mac address for flow and return.
	 * Since the encap direction is handled by the return rule, we are updating the src_mac.
	 */
	if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
		struct nss_ipv4_create *v4;

		v4 = &t->ip_rule.v4;
		memcpy(mac_addr_old, v4->src_mac, ETH_ALEN);
		memcpy(v4->src_mac, mac_addr, ETH_ALEN);
		nss_status = nss_capwapmgr_configure_ipv4(v4, 0, NSS_IPV4_RULE_CREATE_DEST_MAC_VALID);

		if (nss_status != NSS_TX_SUCCESS) {
			nss_capwapmgr_warn("%p: Update Destination Mac for tunnel error : %d \n", dev, nss_status);
			memcpy(t->ip_rule.v4.src_mac, mac_addr_old, ETH_ALEN);
		}

		dev_put(dev);
		return status;

	}

	v6 = &t->ip_rule.v6;
	memcpy(mac_addr_old, v6->src_mac, ETH_ALEN);
	memcpy(v6->src_mac, mac_addr, ETH_ALEN);
	nss_status = nss_capwapmgr_configure_ipv6(v6, 0, NSS_IPV6_RULE_CREATE_DEST_MAC_VALID);

	if (nss_status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%p: Update Destination Mac for tunnel error : %d \n", dev, nss_status);
		memcpy(t->ip_rule.v6.src_mac, mac_addr_old, ETH_ALEN);
		dev_put(dev);
		return NSS_CAPWAPMGR_FAILURE_IP_RULE;
	}

	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_update_dest_mac_addr);

/*
 * nss_capwapmgr_dscp_rule_destroy()
 *	API to destroy previously created DSCP rule.
 */
nss_capwapmgr_status_t nss_capwapmgr_dscp_rule_destroy(uint8_t id)
{
	sw_error_t rv;
	fal_qos_cosmap_t cosmap;
	struct nss_capwapmgr_acl *acl_rule;
	uint8_t dev_id = NSS_CAPWAPMGR_DEV_ID;
	uint8_t rule_nr = NSS_CAPWAPMGR_RULE_NR;
	uint8_t group_id = NSS_CAPWAPMGR_GROUP_ID;
	uint8_t i, j, list_id, v4_rule_id, v6_rule_id, dscp_value, dscp_mask;

	for (i = 0; i < NSS_CAPWAPMGR_ACL_LIST_CNT; i++) {
		for (j = 0; j < NSS_CAPWAPMGR_ACL_RULES_PER_LIST; j++) {
			if (global.acl_list[i].rule[j].uid == id) {
				acl_rule = &global.acl_list[i].rule[j];
				goto found;
			}
		}
	}

	nss_capwapmgr_warn("Invalid id: %u\n", id);
	return NSS_CAPWAPMGR_FAILURE_DSCP_RULE_ID_INVALID;

found:
	if (!acl_rule->in_use) {
		nss_capwapmgr_warn("Rule matching id: %d not in use\n", id);
		return NSS_CAPWAPMGR_FAILURE_DSCP_RULE_ID_NOT_IN_USE;
	}

	dscp_value = acl_rule->dscp_value;
	dscp_mask = acl_rule->dscp_mask;

	/*
	 * Reset all classification fields on cosmap table.
	 */
	cosmap.internal_pcp = 0;
	cosmap.internal_dei = 0;
	cosmap.internal_pri = 0;
	cosmap.internal_dscp = 0;
	cosmap.internal_dp = 0;

	for (i = 0; i < NSS_CAPWAPMGR_DSCP_MAX; i++) {
		if ((i & dscp_mask) != dscp_value) {
			continue;
		}

		nss_capwapmgr_trace("dscpmap: resetting for dscp %u\n", i);
		rv = fal_qos_cosmap_dscp_set(dev_id, group_id, i, &cosmap);
		if (rv != SW_OK) {
			nss_capwapmgr_warn("Failed to reset cosmap for dscp %d - code: %d\n", i, rv);
			return NSS_CAPWAPMGR_FAILURE_DSCP_RULE_DELETE_FAILED;
		}
	}

	/*
	 * Since we use 2 ACL entries per rule (i.e. v4/v6) we multiply by
	 * two to get rule_ids.
	 */
	v4_rule_id = acl_rule->rule_id * 2;
	v6_rule_id = v4_rule_id + 1;
	list_id = NSS_CAPWAPMGR_ACL_LIST_START + acl_rule->list_id;

	rv = fal_acl_rule_delete(dev_id, list_id, v6_rule_id, rule_nr);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to del ACL v6_rule %d from list %d - code: %d\n", v6_rule_id, list_id, rv);
		return NSS_CAPWAPMGR_FAILURE_DSCP_RULE_DELETE_FAILED;
	}

	rv = fal_acl_rule_delete(dev_id, list_id, v4_rule_id, rule_nr);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to del ACL v4_rule %d from list %d - code: %d\n", v4_rule_id, list_id, rv);
		return NSS_CAPWAPMGR_FAILURE_DSCP_RULE_DELETE_FAILED;
	}

	acl_rule->in_use = false;
	return NSS_CAPWAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_capwapmgr_dscp_rule_destroy);

/*
 * nss_capwapmgr_dscp_rule_create()
 *	API to prioritize packets based on DSCP.
 */
nss_capwapmgr_status_t nss_capwapmgr_dscp_rule_create(uint8_t dscp_value, uint8_t dscp_mask, uint8_t pri, uint8_t *id)
{
	sw_error_t rv;
	fal_qos_cosmap_t cosmap;
	fal_qos_cosmap_t *orig_cosmap;
	fal_acl_rule_t *acl_rule;
	uint8_t dev_id = NSS_CAPWAPMGR_DEV_ID;
	uint8_t group_id = NSS_CAPWAPMGR_GROUP_ID;
	uint8_t rule_nr = NSS_CAPWAPMGR_RULE_NR;
	uint8_t list_id, v4_rule_id, v6_rule_id;
	uint8_t lid, rid, i, j;
	int8_t err, fail_dscp;
	int8_t uid = -1;

	nss_capwapmgr_info("Setting priority %u for dscp %u mask %u\n", pri, dscp_value, dscp_mask);

	orig_cosmap = kzalloc(NSS_CAPWAPMGR_DSCP_MAX * sizeof(*orig_cosmap), GFP_KERNEL);
	if (!orig_cosmap) {
		nss_capwapmgr_warn("Failed to alloc memory for orig_cosmap\n");
		return NSS_CAPWAPMGR_FAILURE_MEM_UNAVAILABLE;
	}

	acl_rule = kzalloc(sizeof(*acl_rule), GFP_KERNEL);
	if (!acl_rule) {
		nss_capwapmgr_warn("Failed to alloc memory for acl_rule\n");
		kfree(orig_cosmap);
		return NSS_CAPWAPMGR_FAILURE_MEM_UNAVAILABLE;
	}

	/*
	 * Get an empty acl rule.
	 */
	for (i = 0; i < NSS_CAPWAPMGR_ACL_LIST_CNT; i++) {
		for (j = 0; j < NSS_CAPWAPMGR_ACL_RULES_PER_LIST; j++) {
			if (global.acl_list[i].rule[j].in_use) {
				continue;
			}

			uid = global.acl_list[i].rule[j].uid;
			rid = global.acl_list[i].rule[j].rule_id;
			lid = global.acl_list[i].rule[j].list_id;
			goto found;
		}
	}

found:
	if (uid < 0) {
		nss_capwapmgr_warn("No free ACL rules available\n");
		err = NSS_CAPWAPMGR_FAILURE_ACL_UNAVAILABLE;
		goto fail1;
	};

	/*
	 * Since we use 2 ACL entries per rule (i.e. v4/v6) we multiply rid by
	 * two to get rule_id.
	 */
	v4_rule_id = rid * 2;
	v6_rule_id = v4_rule_id + 1;
	list_id = NSS_CAPWAPMGR_ACL_LIST_START + lid;

	nss_capwapmgr_info("Using ACL rules: %d & %d from list: %d\n", v4_rule_id, v6_rule_id, list_id);

	/*
	 * Prioritize packets with the dscp value. For trustsec packets, we need to specify
	 * the location of the dscp value with ACL configuration.
	 * ACL rule always start from the L2 header. It will be trustsec header for our case.
	 * We need two user defined profile to set beginning of the
	 * Profile 0 is for start of the ethernet type.
	 * Profile 1 is for the start of the ip header.
	 */
	rv = fal_acl_udf_profile_set(dev_id, FAL_ACL_UDF_NON_IP, 0, FAL_ACL_UDF_TYPE_L3, NSS_CAPWAPMGR_ETH_HDR_OFFSET);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to create UDF 0 Map - code: %d\n", rv);
		err = NSS_CAPWAPMGR_FAILURE_CREATE_UDF_PROFILE;
		goto fail1;
	}

	rv = fal_acl_udf_profile_set(dev_id, FAL_ACL_UDF_NON_IP, 1, FAL_ACL_UDF_TYPE_L3, NSS_CAPWAPMGR_IPV4_OFFSET);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to create UDF 1 Map - code: %d\n", rv);
		err = NSS_CAPWAPMGR_FAILURE_CREATE_UDF_PROFILE;
		goto fail1;
	}

	acl_rule->rule_type = FAL_ACL_RULE_MAC;

	/*
	 * Sets valid flags for the acl rule.
	 * Following rules are valid:
	 * - Ethernet type
	 * - User defined field 0. Correspond to ethernet type (ipv4/ipv6)
	 * - User defined field 1. Correspond to DSCP value (dscp_value)
	 */
	FAL_FIELD_FLG_SET(acl_rule->field_flg, FAL_ACL_FIELD_MAC_ETHTYPE);
	FAL_FIELD_FLG_SET(acl_rule->field_flg, FAL_ACL_FIELD_UDF0);
	FAL_FIELD_FLG_SET(acl_rule->field_flg, FAL_ACL_FIELD_UDF1);
	FAL_ACTION_FLG_SET(acl_rule->action_flg, FAL_ACL_ACTION_PERMIT);
	FAL_ACTION_FLG_SET(acl_rule->action_flg, FAL_ACL_ACTION_ENQUEUE_PRI);

	/*
	 * Set common parameters for ipv4/ipv6
	 */
	acl_rule->ethtype_val = NSS_CAPWAPMGR_ETH_TYPE_TRUSTSEC;
	acl_rule->ethtype_mask = NSS_CAPWAPMGR_ETH_TYPE_MASK;
	acl_rule->udf0_op = FAL_ACL_FIELD_MASK;
	acl_rule->udf1_op = FAL_ACL_FIELD_MASK;
	acl_rule->enqueue_pri = pri;

	/*
	 * Create ACL rule for IPv4
	 */
	acl_rule->udf0_val = NSS_CAPWAPMGR_ETH_TYPE_IPV4;
	acl_rule->udf0_mask = NSS_CAPWAPMGR_ETH_TYPE_MASK;
	acl_rule->udf1_val = dscp_value << NSS_CAPWAPMGR_DSCP_MASK_IPV4_SHIFT;
	acl_rule->udf1_mask = dscp_mask << NSS_CAPWAPMGR_DSCP_MASK_IPV4_SHIFT;

	rv = fal_acl_rule_query(dev_id, list_id, v4_rule_id, acl_rule);
	if (rv != SW_NOT_FOUND) {
		nss_capwapmgr_warn("ACL rule already exist for list_id: %u, rule_id: %u - code: %d\n", list_id, v4_rule_id, rv);
		err = NSS_CAPWAPMGR_FAILURE_ACL_RULE_ALREADY_EXIST;
		goto fail1;
	}

	rv = fal_acl_list_unbind(dev_id, list_id, FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORTBITMAP, NSS_CAPWAPMGR_BIND_BITMAP);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to unbind list: %d - code: %d\n", list_id, rv);
		err = NSS_CAPWAPMGR_FAILURE_ADD_ACL_RULE;
		goto fail1;
	}

	rv = fal_acl_rule_add(dev_id, list_id, v4_rule_id, rule_nr, acl_rule);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to add ACL v4_rule: %d - code: %d\n", rv, v4_rule_id);
		err = NSS_CAPWAPMGR_FAILURE_ADD_ACL_RULE;
		goto fail1;
	}

	/*
	 * Create ACL rule for IPv6
	 */
	acl_rule->udf0_val = NSS_CAPWAPMGR_ETH_TYPE_IPV6;
	acl_rule->udf0_mask = NSS_CAPWAPMGR_ETH_TYPE_MASK;
	acl_rule->udf1_val = dscp_value << NSS_CAPWAPMGR_DSCP_MASK_IPV6_SHIFT;
	acl_rule->udf1_mask = dscp_mask << NSS_CAPWAPMGR_DSCP_MASK_IPV6_SHIFT;

	rv = fal_acl_rule_add(dev_id, list_id, v6_rule_id, rule_nr, acl_rule);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to add ACL v6_rule: %d - code: %d\n", rv, v6_rule_id);
		err = NSS_CAPWAPMGR_FAILURE_ADD_ACL_RULE;
		goto fail2;
	}

	/*
	 * Bind list to all ethernet ports
	 */
	rv = fal_acl_list_bind(dev_id, list_id, FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORTBITMAP, NSS_CAPWAPMGR_BIND_BITMAP);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to bind ACL list: %d - code: %d\n", list_id, rv);
		err = NSS_CAPWAPMGR_FAILURE_BIND_ACL_LIST;
		goto fail3;
	}

	/*
	 * Set ACL as in_use and save dscp value and mask.
	 */
	global.acl_list[lid].rule[rid].in_use = true;
	global.acl_list[lid].rule[rid].dscp_value = dscp_value;
	global.acl_list[lid].rule[rid].dscp_mask = dscp_mask;

	/*
	 * Prioritize packets with the dscp value is dscp_value for non trustsec packets.
	 * These packets do not require any ACL Rule.
	 */
	cosmap.internal_pcp = 0;
	cosmap.internal_dei = 0;
	cosmap.internal_pri = pri;
	cosmap.internal_dscp = 0;
	cosmap.internal_dp = 0;
	for (i = 0; i < NSS_CAPWAPMGR_DSCP_MAX; i++) {
		if ((i & dscp_mask) != dscp_value) {
			continue;
		}

		rv = fal_qos_cosmap_dscp_get(dev_id, group_id, i, &orig_cosmap[i]);
		if (rv != SW_OK) {
			nss_capwapmgr_warn("dscpmap: failed to get cosmap for dscp %d\n", i);
			err = NSS_CAPWAPMGR_FAILURE_CONFIGURE_DSCP_MAP;
			goto fail4;
		}

		nss_capwapmgr_trace("dscpmap: setting priority %u for dscp %u\n", pri, i);
		rv = fal_qos_cosmap_dscp_set(dev_id, group_id, i, &cosmap);
		if (rv != SW_OK) {
			nss_capwapmgr_warn("Failed to configure cosmap for dscp %d - code: %d\n", i, rv);
			err = NSS_CAPWAPMGR_FAILURE_CONFIGURE_DSCP_MAP;
			goto fail4;
		}
	}

	kfree(acl_rule);
	kfree(orig_cosmap);

	*id = uid;

	return NSS_CAPWAPMGR_SUCCESS;

fail4:
	fail_dscp = i;
	for (i = 0; i < fail_dscp; i++) {
		if ((i & dscp_mask) != dscp_value) {
			continue;
		}

		nss_capwapmgr_trace("dscpmap: resetting to priority %u for dscp %u\n", orig_cosmap[i].internal_pri, i);
		rv = fal_qos_cosmap_dscp_set(dev_id, group_id, i, &orig_cosmap[i]);
		if (rv != SW_OK) {
			nss_capwapmgr_warn("Failed to reset cosmap for dscp %d - code: %d\n", i, rv);
		}
	}

fail3:
	rv = fal_acl_rule_delete(dev_id, list_id, v6_rule_id, rule_nr);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to del ACL v6_rule %d from list %d - code: %d\n", v6_rule_id, list_id, rv);
	}

fail2:
	rv = fal_acl_rule_delete(dev_id, list_id, v4_rule_id, rule_nr);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to del ACL v4_rule %d from list %d - code: %d\n", v4_rule_id, list_id, rv);
	}

fail1:
	kfree(orig_cosmap);
	kfree(acl_rule);

	return err;
}
EXPORT_SYMBOL(nss_capwapmgr_dscp_rule_create);

/*
 * nss_capwapmgr_configure_dtls
 *	Enable or disable DTLS of a capwap tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_configure_dtls(struct net_device *dev, uint8_t tunnel_id, uint8_t enable_dtls, struct nss_dtlsmgr_config *in_data)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwap_msg capwapmsg;
	struct nss_capwapmgr_tunnel *t;
	struct nss_ipv4_destroy v4;
	struct nss_ipv6_destroy v6;
	nss_tx_status_t nss_status = NSS_TX_SUCCESS;
	nss_capwapmgr_status_t status;
	uint32_t ip_if_num, dtls_enabled, outer_trustsec_enabled;

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_warn("%p: can't find tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	outer_trustsec_enabled = t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_OUTER_TRUSTSEC_ENABLED;
	dtls_enabled = t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED;
	if ((enable_dtls && dtls_enabled) || (!enable_dtls && !dtls_enabled)) {
		nss_capwapmgr_warn("%p: nothing changed for tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	/*
	 * We don't allow configuring dtls on tunnel if it's still
	 * enabled.
	 */
	if (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED) {
		nss_capwapmgr_warn("%p: tunnel %d is already enabled\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_TUNNEL_ENABLED;
	}

	/*
	 * Prepare DTLS configure message
	 */
	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));
	nss_capwap_msg_init(&capwapmsg, t->if_num, NSS_CAPWAP_MSG_TYPE_DTLS,
		sizeof(struct nss_capwap_dtls_msg), nss_capwapmgr_msg_event_receive, dev);

	if (!enable_dtls) {
		nss_capwapmgr_info("%p disabling DTLS for tunnel: %d\n", dev, tunnel_id);

		ip_if_num = nss_capwap_ifnum_with_core_id(t->if_num);
		capwapmsg.msg.dtls.enable = 0;
		capwapmsg.msg.dtls.dtls_inner_if_num = t->capwap_rule.dtls_inner_if_num;
		capwapmsg.msg.dtls.mtu_adjust = 0;

		/*
		 * Unconfigure trustsec tx first
		 */
		if (outer_trustsec_enabled) {
			nss_status = nss_trustsec_tx_unconfigure_sgt(t->capwap_rule.dtls_inner_if_num, t->capwap_rule.outer_sgt_value);
			if (nss_status != NSS_TX_SUCCESS) {
				nss_capwapmgr_warn("%p: unconfigure trustsec_tx failed\n", dev);
				return NSS_CAPWAPMGR_FAILURE_UNCONFIGURE_TRUSTSEC_TX;
			}
		}
	} else {
		nss_capwapmgr_info("%p enabling DTLS for tunnel: %d\n", dev, tunnel_id);

		if (!t->capwap_rule.dtls_inner_if_num) {
			/*
			 * Create a DTLS node, we only validate caller is providing a DTLS
			 * configuration structure, the correctness of these settings are
			 * validated by dtlsmgr
			 */
			if (!in_data) {
				nss_capwapmgr_info("%p: dtls in_data required to create dtls tunnel\n", dev);
				return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
			}
			in_data->decap.nexthop_ifnum = nss_capwap_ifnum_with_core_id(t->if_num);
			t->dtls_dev = nss_dtlsmgr_session_create(in_data);
			if (!t->dtls_dev) {
				nss_capwapmgr_warn("%p: cannot create DTLS session\n", dev);
				return NSS_CAPWAPMGR_FAILURE_DI_ALLOC_FAILED;
			}

			/* Store the DTLS encap and decap interface numbers */
			t->capwap_rule.dtls_inner_if_num = nss_dtlsmgr_get_interface(t->dtls_dev,
										     NSS_DTLSMGR_INTERFACE_TYPE_INNER);
			t->capwap_rule.mtu_adjust = t->dtls_dev->needed_headroom + t->dtls_dev->needed_tailroom;
			nss_capwapmgr_info("%p: created dtls node for tunnel: %d if_num: %d mtu_adjust: %d\n",
					   dev, tunnel_id, t->capwap_rule.dtls_inner_if_num, t->capwap_rule.mtu_adjust);
		}

		ip_if_num = nss_dtlsmgr_get_interface(t->dtls_dev, NSS_DTLSMGR_INTERFACE_TYPE_OUTER);

		capwapmsg.msg.dtls.enable = 1;
		capwapmsg.msg.dtls.dtls_inner_if_num = t->capwap_rule.dtls_inner_if_num;
		capwapmsg.msg.dtls.mtu_adjust = t->capwap_rule.mtu_adjust;

		/*
		 * Unconfigure trustsec tx first
		 */
		if (outer_trustsec_enabled) {
			nss_status = nss_trustsec_tx_unconfigure_sgt(t->if_num, t->capwap_rule.outer_sgt_value);
			if (nss_status != NSS_TX_SUCCESS) {
				nss_capwapmgr_warn("%p: unconfigure trustsec_tx failed\n", dev);
				return NSS_CAPWAPMGR_FAILURE_UNCONFIGURE_TRUSTSEC_TX;
			}
		}
	}

	/*
	 * Re-configure trustsec_tx
	 */
	if (outer_trustsec_enabled) {
		nss_status = nss_trustsec_tx_configure_sgt(ip_if_num, t->capwap_rule.gmac_ifnum, t->capwap_rule.outer_sgt_value);
		if (nss_status != NSS_TX_SUCCESS) {
			nss_capwapmgr_warn("%p: configure trustsec_tx failed\n", dev);
			return NSS_CAPWAPMGR_FAILURE_CONFIGURE_TRUSTSEC_TX;
		}
	}

	dev_hold(dev);
	priv = netdev_priv(dev);

	/*
	 * Recreate ipv4/v6 rules with the new interface number
	 */
	if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
		v4.protocol = IPPROTO_UDP;
		v4.src_ip = t->ip_rule.v4.src_ip;
		v4.dest_ip = t->ip_rule.v4.dest_ip;
		v4.src_port = t->ip_rule.v4.src_port;
		v4.dest_port = t->ip_rule.v4.dest_port;
		nss_status = nss_capwapmgr_unconfigure_ipv4_rule(&v4);
		if (nss_status != NSS_TX_SUCCESS) {
			nss_capwapmgr_warn("%p: unconfigure ipv4 rule failed : %d\n", dev, nss_status);
			dev_put(dev);
			return NSS_CAPWAPMGR_FAILURE_IP_DESTROY_RULE;
		}
		t->ip_rule.v4.dest_interface_num = ip_if_num;
		nss_status = nss_capwapmgr_configure_ipv4(&t->ip_rule.v4, 0, 0);
	} else {
		if (t->capwap_rule.which_udp == NSS_CAPWAP_TUNNEL_UDP) {
			v6.protocol = IPPROTO_UDP;
		} else {
			v6.protocol = IPPROTO_UDPLITE;
		}

		v6.src_ip[0] = t->ip_rule.v6.src_ip[0];
		v6.src_ip[1] = t->ip_rule.v6.src_ip[1];
		v6.src_ip[2] = t->ip_rule.v6.src_ip[2];
		v6.src_ip[3] = t->ip_rule.v6.src_ip[3];

		v6.dest_ip[0] = t->ip_rule.v6.dest_ip[0];
		v6.dest_ip[1] = t->ip_rule.v6.dest_ip[1];
		v6.dest_ip[2] = t->ip_rule.v6.dest_ip[2];
		v6.dest_ip[3] = t->ip_rule.v6.dest_ip[3];

		v6.src_port = t->ip_rule.v6.src_port;
		v6.dest_port = t->ip_rule.v6.dest_port;
		nss_status = nss_capwapmgr_unconfigure_ipv6_rule(&v6);
		if (nss_status != NSS_TX_SUCCESS) {
			nss_capwapmgr_warn("%p: unconfigure ipv6 rule failed : %d\n", dev, nss_status);
			dev_put(dev);
			return NSS_CAPWAPMGR_FAILURE_IP_DESTROY_RULE;
		}

		t->ip_rule.v6.dest_interface_num = ip_if_num;
		nss_status = nss_capwapmgr_configure_ipv6(&t->ip_rule.v6, 0, 0);
	}

	if (nss_status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%p: configure ip rule failed : %d\n", dev, nss_status);
		dev_put(dev);
		return NSS_CAPWAPMGR_FAILURE_IP_RULE;
	}

	/*
	 * Now configure capwap dtls
	 */
	status = nss_capwapmgr_tx_msg_sync(priv->nss_ctx, dev, &capwapmsg);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%p: configure DTLS failed : %d\n", dev, status);
		dev_put(dev);
		return nss_status;
	}

	if (enable_dtls) {
		t->capwap_rule.enabled_features |= NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED;
	} else {
		t->capwap_rule.enabled_features &= ~NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED;
	}
	dev_put(dev);
	return NSS_CAPWAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_capwapmgr_configure_dtls);

/*
 * nss_capwapmgr_verify_dtls_rekey_param()
 *	Validate the rekey param for a DTLS tunnel and return the DTLS netdevice
 */
static inline struct net_device *nss_capwapmgr_verify_dtls_rekey_param(struct net_device *dev, uint8_t tunnel_id,
								 struct nss_dtlsmgr_config_update *udata)
{
	struct nss_capwapmgr_tunnel *t;

	if (!udata) {
		nss_capwapmgr_info("%p: dtls session update data required\n", dev);
		return NULL;
	}

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_warn("%p: can't find tunnel: %d\n", dev, tunnel_id);
		return NULL;
	}

	if (!(t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED)) {
		nss_capwapmgr_warn("%p: tunnel does not enable DTLS: %d\n", dev, tunnel_id);
		return NULL;
	}

	return t->dtls_dev;
}

/*
 * nss_capwapmgr_dtls_rekey_rx_cipher_update()
 *	Update the rx cipher key for an DTLS tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_rx_cipher_update(struct net_device *dev, uint8_t tunnel_id,
								 struct nss_dtlsmgr_config_update *udata)
{
	struct net_device *dtls_ndev = nss_capwapmgr_verify_dtls_rekey_param(dev, tunnel_id, udata);

	/*
	 * Calling dtlsmgr for rekey
	 */
	if (nss_dtlsmgr_session_update_decap(dtls_ndev, udata) != NSS_DTLSMGR_OK) {
		nss_capwapmgr_warn("%p: tunnel: %d rekey rx cipher update failed\n", dtls_ndev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_INVALID_DTLS_CFG;
	}
	return NSS_CAPWAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_capwapmgr_dtls_rekey_rx_cipher_update);

/*
 * nss_capwapmgr_dtls_rekey_tx_cipher_update()
 *	Update the tx cipher key for an DTLS tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_tx_cipher_update(struct net_device *dev, uint8_t tunnel_id,
								 struct nss_dtlsmgr_config_update *udata)
{
	struct net_device *dtls_ndev = nss_capwapmgr_verify_dtls_rekey_param(dev, tunnel_id, udata);

	/*
	 * Calling dtlsmgr for rekey
	 */
	if (nss_dtlsmgr_session_update_encap(dtls_ndev, udata) != NSS_DTLSMGR_OK) {
		nss_capwapmgr_warn("%p: tunnel: %d rekey tx cipher update failed\n", dtls_ndev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_INVALID_DTLS_CFG;
	}
	return NSS_CAPWAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_capwapmgr_dtls_rekey_tx_cipher_update);

/*
 * nss_capwapmgr_dtls_rekey_rx_cipher_switch()
 *	Switch the rx cipher key for an DTLS tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_rx_cipher_switch(struct net_device *dev, uint8_t tunnel_id)
{
	struct nss_capwapmgr_tunnel *t;

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_warn("%p: can't find tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (!(t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED)) {
		nss_capwapmgr_warn("%p: tunnel does not enable DTLS: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	/*
	 * Calling dtlsmgr for rekey switch
	 */
	if (!nss_dtlsmgr_session_switch_decap(t->dtls_dev)) {
		nss_capwapmgr_warn("%p: tunnel: %d rekey rx cipher switch failed\n", t->dtls_dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_INVALID_DTLS_CFG;
	}

	return NSS_CAPWAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_capwapmgr_dtls_rekey_rx_cipher_switch);

/*
 * nss_capwapmgr_dtls_rekey_tx_cipher_switch()
 *	Switch the tx cipher key for an DTLS tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_tx_cipher_switch(struct net_device *dev, uint8_t tunnel_id)
{
	struct nss_capwapmgr_tunnel *t;

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_warn("%p: can't find tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (!(t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED)) {
		nss_capwapmgr_warn("%p: tunnel does not enable DTLS: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	/*
	 * Calling dtlsmgr for rekey switch
	 */
	if (!nss_dtlsmgr_session_switch_encap(t->dtls_dev)) {
		nss_capwapmgr_warn("%p: tunnel: %d rekey tx cipher switch failed\n", t->dtls_dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_INVALID_DTLS_CFG;
	}

	return NSS_CAPWAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_capwapmgr_dtls_rekey_tx_cipher_switch);

/*
 * nss_capwapmgr_change_version()
 *	Change CAPWAP version
 */

nss_capwapmgr_status_t nss_capwapmgr_change_version(struct net_device *dev, uint8_t tunnel_id, uint8_t ver)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwap_msg capwapmsg;
	struct nss_capwapmgr_tunnel *t;
	nss_capwapmgr_status_t status;

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_warn("%p: can't find tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (ver > NSS_CAPWAP_VERSION_V2) {
		nss_capwapmgr_warn("%p: un-supported Version: %d\n", dev, ver);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));

	/*
	 * Send CAPWAP data tunnel command to NSS
	 */
	nss_capwap_msg_init(&capwapmsg, t->if_num, NSS_CAPWAP_MSG_TYPE_VERSION,
		sizeof(struct nss_capwap_version_msg), nss_capwapmgr_msg_event_receive, dev);
	capwapmsg.msg.version.version = ver;
	status = nss_capwapmgr_tx_msg_sync(priv->nss_ctx, dev, &capwapmsg);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%p: Update Path MTU Tunnel error : %d \n", dev, status);
		dev_put(dev);
		return status;
	}

	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_change_version);

/*
 * nss_capwapmgr_tunnel_action()
 *	Common function for CAPWAP tunnel operation messages without
 *	any message data structures.
 */
static nss_tx_status_t nss_capwapmgr_tunnel_action(struct nss_ctx_instance *ctx, struct net_device *dev, uint32_t if_num, nss_capwap_msg_type_t cmd)
{
	struct nss_capwap_msg capwapmsg;
	nss_tx_status_t status;

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));

	/*
	 * Send CAPWAP data tunnel command to NSS
	 */
	nss_capwap_msg_init(&capwapmsg, if_num, cmd, 0, nss_capwapmgr_msg_event_receive, dev);
	status = nss_capwapmgr_tx_msg_sync(ctx, dev, &capwapmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%p: ctx: CMD: %d Tunnel error : %d \n", ctx, cmd, status);
		return status;
	}

	return status;
}

/*
 * nss_capwapmgr_enable_tunnel()
 *	API for enabling a data tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_enable_tunnel(struct net_device *dev, uint8_t tunnel_id)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_tunnel *t;
	nss_tx_status_t ret;

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_warn("%p: can't find tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED) {
		nss_capwapmgr_warn("%p: tunnel %d is already enabled\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_TUNNEL_ENABLED;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);
	nss_capwapmgr_info("%p: %d: tunnel enable is being called\n", dev, t->if_num);
	ret = nss_capwapmgr_tunnel_action(priv->nss_ctx, dev, t->if_num, NSS_CAPWAP_MSG_TYPE_ENABLE_TUNNEL);
	if (ret == NSS_TX_SUCCESS) {
		t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED;
	}
	dev_put(dev);
	return ret;
}
EXPORT_SYMBOL(nss_capwapmgr_enable_tunnel);

/*
 * nss_capwapmgr_disable_tunnel()
 *	API for disabling a data tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_disable_tunnel(struct net_device *dev, uint8_t tunnel_id)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_tunnel *t;
	nss_tx_status_t ret;

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_warn("%p: can't find tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (!(t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED)) {
		nss_capwapmgr_warn("%p: tunnel %d is already enabled\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_TUNNEL_DISABLED;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);
	nss_capwapmgr_info("%p: %d: tunnel disable is being called\n", dev, t->if_num);
	ret = nss_capwapmgr_tunnel_action(priv->nss_ctx, dev, t->if_num, NSS_CAPWAP_MSG_TYPE_DISABLE_TUNNEL);
	if (ret == NSS_TX_SUCCESS) {
		t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED;
	}
	dev_put(dev);
	return ret;
}
EXPORT_SYMBOL(nss_capwapmgr_disable_tunnel);

/*
 * nss_capwapmgr_tunnel_create_common()
 *	Common handling for creating IPv4 or IPv6 tunnel
 */
static nss_capwapmgr_status_t nss_capwapmgr_tunnel_create_common(struct net_device *dev, uint8_t tunnel_id,
	struct nss_ipv4_create *v4, struct nss_ipv6_create *v6, struct nss_capwap_rule_msg *capwap_rule, struct nss_dtlsmgr_config *in_data)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_tunnel *t;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	int32_t capwap_if_num, forward_if_num;
	uint16_t type_flags = 0;
	nss_tx_status_t nss_status = NSS_TX_SUCCESS;
	uint32_t dtls_enabled = capwap_rule->enabled_features & NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED;
	uint32_t outer_trustsec_enabled = capwap_rule->enabled_features & NSS_CAPWAPMGR_FEATURE_OUTER_TRUSTSEC_ENABLED;

	if (!v4 && !v6) {
		nss_capwapmgr_warn("%p: invalid ip create rule for tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (tunnel_id > NSS_CAPWAPMGR_MAX_TUNNELS) {
		nss_capwapmgr_warn("%p: invalid tunnel_id: %d max: NSS_CAPWAPMGR_MAX_TUNNELS\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (!(capwap_rule->l3_proto == NSS_CAPWAP_TUNNEL_IPV4 ||
		capwap_rule->l3_proto == NSS_CAPWAP_TUNNEL_IPV6)) {
		nss_capwapmgr_warn("%p: tunnel %d: wrong argument for l3_proto\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (!(capwap_rule->which_udp == NSS_CAPWAP_TUNNEL_UDP ||
		capwap_rule->which_udp == NSS_CAPWAP_TUNNEL_UDPLite)) {
		nss_capwapmgr_warn("%p: tunnel %d: wrong argument for which_udp\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (dtls_enabled && !in_data) {
		nss_capwapmgr_warn("%p: need to supply in_data if DTLS is enabled\n", dev);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (t) {
		nss_capwapmgr_warn("%p: tunnel: %d already created\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_TUNNEL_EXISTS;
	}

	capwap_if_num = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP);
	if (capwap_if_num < 0) {
		nss_capwapmgr_warn("%p: di returned error : %d\n", dev, capwap_if_num);
		return NSS_CAPWAPMGR_FAILURE_DI_ALLOC_FAILED;
	}

	if (nss_capwapmgr_register_with_nss(capwap_if_num, dev) != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%d: NSS CAPWAP register with NSS failed", capwap_if_num);
		(void)nss_dynamic_interface_dealloc_node(capwap_if_num, NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP);
		return NSS_CAPWAPMGR_FAILURE_REGISTER_NSS;
	}

	if (!dtls_enabled) {
		capwap_rule->mtu_adjust = 0;
		capwap_rule->dtls_inner_if_num = 0;
		forward_if_num = nss_capwap_ifnum_with_core_id(capwap_if_num);
	} else {
		in_data->decap.nexthop_ifnum = nss_capwap_ifnum_with_core_id(capwap_if_num);
		t->dtls_dev = nss_dtlsmgr_session_create(in_data);
		if (!t->dtls_dev) {
			nss_capwapmgr_warn("%p: NSS DTLS node alloc failed\n", dev);
			nss_capwapmgr_unregister_with_nss(capwap_if_num);
			(void)nss_dynamic_interface_dealloc_node(capwap_if_num, NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP);
			return NSS_CAPWAPMGR_FAILURE_DI_ALLOC_FAILED;
		}
		capwap_rule->dtls_inner_if_num = nss_dtlsmgr_get_interface(t->dtls_dev, NSS_DTLSMGR_INTERFACE_TYPE_INNER);
		forward_if_num = nss_dtlsmgr_get_interface(t->dtls_dev, NSS_DTLSMGR_INTERFACE_TYPE_OUTER);
		capwap_rule->mtu_adjust = t->dtls_dev->needed_headroom + t->dtls_dev->needed_tailroom;
	}

	if (outer_trustsec_enabled) {
		nss_capwapmgr_info("%p: configure TrustsecTx with sgt value: %x\n", dev, capwap_rule->outer_sgt_value);
		if (v4) {
			capwap_rule->gmac_ifnum = v4->src_interface_num;
			nss_status = nss_trustsec_tx_configure_sgt(forward_if_num, v4->src_interface_num, capwap_rule->outer_sgt_value);
			v4->src_interface_num = NSS_TRUSTSEC_TX_INTERFACE;
		} else {
			capwap_rule->gmac_ifnum = v6->src_interface_num;
			nss_status = nss_trustsec_tx_configure_sgt(forward_if_num, v6->src_interface_num, capwap_rule->outer_sgt_value);
			v6->src_interface_num = NSS_TRUSTSEC_TX_INTERFACE;
		}
		if (nss_status != NSS_TX_SUCCESS) {
			nss_capwapmgr_warn("%p: configure trustsectx node failed\n", dev);
			status = NSS_CAPWAPMGR_FAILURE_CONFIGURE_TRUSTSEC_TX;
			goto fail;
		}
	}

	/*
	 * We use type_flags to determine the correct header sizes
	 * for a frame when encaping. CAPWAP processing node in the
	 * NSS FW does not know anything about IP rule information.
	 */
	if (v4) {
		if ((v4->out_vlan_tag[0] & 0xFFF) != 0xFFF) {
			type_flags |= NSS_CAPWAP_RULE_CREATE_VLAN_CONFIGURED;
		}

		if (v4->flow_pppoe_if_exist) {
			type_flags |= NSS_CAPWAP_RULE_CREATE_PPPOE_CONFIGURED;
		}
	} else {
		if ((v6->out_vlan_tag[0] & 0xFFF) != 0xFFF) {
			type_flags |= NSS_CAPWAP_RULE_CREATE_VLAN_CONFIGURED;
		}

		if (v6->flow_pppoe_if_exist) {
			type_flags |= NSS_CAPWAP_RULE_CREATE_PPPOE_CONFIGURED;
		}
	}

	if (capwap_rule->l3_proto == NSS_CAPWAP_TUNNEL_IPV6 && capwap_rule->which_udp == NSS_CAPWAP_TUNNEL_UDPLite) {
		type_flags |= NSS_CAPWAP_ENCAP_UDPLITE_HDR_CSUM;
	}

	/*
	 * Copy over the IP rule information to capwap rule for encap side.
	 * This will avoid any confusions because IP rule direction is AC->AP and
	 * capwap encap rule direction is AP->AC.
	 */
	if (v4) {
		capwap_rule->encap.src_port = htonl(v4->dest_port);
		capwap_rule->encap.src_ip.ip.ipv4 = htonl(v4->dest_ip);

		capwap_rule->encap.dest_port = htonl(v4->src_port);
		capwap_rule->encap.dest_ip.ip.ipv4 = htonl(v4->src_ip);
	} else {
		capwap_rule->encap.src_port = htonl(v6->dest_port);
		capwap_rule->encap.src_ip.ip.ipv6[0] = htonl(v6->dest_ip[0]);
		capwap_rule->encap.src_ip.ip.ipv6[1] = htonl(v6->dest_ip[1]);
		capwap_rule->encap.src_ip.ip.ipv6[2] = htonl(v6->dest_ip[2]);
		capwap_rule->encap.src_ip.ip.ipv6[3] = htonl(v6->dest_ip[3]);

		capwap_rule->encap.dest_port = htonl(v6->src_port);
		capwap_rule->encap.dest_ip.ip.ipv6[0] = htonl(v6->src_ip[0]);
		capwap_rule->encap.dest_ip.ip.ipv6[1] = htonl(v6->src_ip[1]);
		capwap_rule->encap.dest_ip.ip.ipv6[2] = htonl(v6->src_ip[2]);
		capwap_rule->encap.dest_ip.ip.ipv6[3] = htonl(v6->src_ip[3]);
	}

	status = nss_capwapmgr_create_capwap_rule(dev, capwap_if_num, capwap_rule, type_flags);
	nss_capwapmgr_info("%p: dynamic interface if_num is :%d and capwap tunnel status:%d\n", dev, capwap_if_num, status);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%p: %d: CAPWAP rule create failed with status: %d", dev, capwap_if_num, status);
		status = NSS_CAPWAPMGR_FAILURE_CAPWAP_RULE;
		goto fail;
	}

	if (v4) {
		v4->dest_interface_num = forward_if_num;
		nss_status = nss_capwapmgr_configure_ipv4(v4, 0, 0);
	} else {
		v6->dest_interface_num = forward_if_num;
		nss_status = nss_capwapmgr_configure_ipv6(v6, 0, 0);
	}

	if (nss_status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%p: %d: IPv4/IPv6 rule create failed with status: %d", dev, forward_if_num, nss_status);
		status = NSS_CAPWAPMGR_FAILURE_IP_RULE;
		goto fail;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);
	t = &priv->tunnel[tunnel_id];
	nss_capwapmgr_info("%p: %d: CAPWAP TUNNEL CREATE DONE tunnel_id:%d (%p)\n", dev, capwap_if_num, tunnel_id, t);

	/*
	 * Keep a copy of rule information.
	 */
	if (v4) {
		memcpy(&t->ip_rule.v4, v4, sizeof (struct nss_ipv4_create));
	} else {
		memcpy(&t->ip_rule.v6, v6, sizeof (struct nss_ipv6_create));
	}

	memcpy(&t->capwap_rule, capwap_rule, sizeof (struct nss_capwap_rule_msg));

	/*
	 * Make it globally visible inside the netdev.
	 */
	t->if_num = capwap_if_num;
	priv->if_num_to_tunnel_id[capwap_if_num] = tunnel_id;
	t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_CONFIGURED;

	dev_put(dev);
	return status;

fail:
	nss_capwapmgr_unregister_with_nss(capwap_if_num);
	(void)nss_dynamic_interface_dealloc_node(capwap_if_num, NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP);
	if (dtls_enabled) {
		if (nss_dtlsmgr_session_destroy(t->dtls_dev) != NSS_DTLSMGR_OK) {
			nss_capwapmgr_warn("%p: failed to destroy DTLS session", t->dtls_dev);
		}
	}
	return status;
}

/*
 * nss_capwapmgr_ipv4_tunnel_create()
 *	API for creating IPv4 and CAPWAP rule.
 */
nss_capwapmgr_status_t nss_capwapmgr_ipv4_tunnel_create(struct net_device *dev, uint8_t tunnel_id,
			struct nss_ipv4_create *ip_rule, struct nss_capwap_rule_msg *capwap_rule, struct nss_dtlsmgr_config *in_data)
{
	return nss_capwapmgr_tunnel_create_common(dev, tunnel_id, ip_rule, NULL, capwap_rule, in_data);
}
EXPORT_SYMBOL(nss_capwapmgr_ipv4_tunnel_create);

/*
 * nss_capwapmgr_ipv6_tunnel_create()
 *	API for creating IPv6 and CAPWAP rule.
 */
nss_capwapmgr_status_t nss_capwapmgr_ipv6_tunnel_create(struct net_device *dev, uint8_t tunnel_id,
			struct nss_ipv6_create *ip_rule, struct nss_capwap_rule_msg *capwap_rule, struct nss_dtlsmgr_config *in_data)
{
	return nss_capwapmgr_tunnel_create_common(dev, tunnel_id, NULL, ip_rule, capwap_rule, in_data);
}
EXPORT_SYMBOL(nss_capwapmgr_ipv6_tunnel_create);

/*
 * nss_capwapmgr_tunnel_save_stats()
 *	Internal function to save tunnel stats when a tunnel is being
 *	destroyed.
 */
static void nss_capwapmgr_tunnel_save_stats(struct nss_capwap_tunnel_stats *save, struct nss_capwap_tunnel_stats *fstats)
{
	save->dtls_pkts += fstats->dtls_pkts;

	save->rx_segments += fstats->rx_segments;
	save->rx_dup_frag += fstats->rx_dup_frag;
	save->rx_oversize_drops += fstats->rx_oversize_drops;
	save->rx_frag_timeout_drops += fstats->rx_frag_timeout_drops;
	save->rx_queue_full_drops += fstats->rx_queue_full_drops;
	save->rx_n2h_queue_full_drops += fstats->rx_n2h_queue_full_drops;
	save->rx_mem_failure_drops += fstats->rx_mem_failure_drops;
	save->rx_csum_drops += fstats->rx_csum_drops;
	save->rx_malformed += fstats->rx_malformed;
	save->rx_frag_gap_drops += fstats->rx_frag_gap_drops;

	save->tx_segments += fstats->tx_segments;
	save->tx_queue_full_drops += fstats->tx_queue_full_drops;
	save->tx_mem_failure_drops += fstats->tx_mem_failure_drops;
	save->tx_dropped_sg_ref += fstats->tx_dropped_sg_ref;
	save->tx_dropped_ver_mis += fstats->tx_dropped_ver_mis;
	save->tx_dropped_unalign += fstats->tx_dropped_unalign;
	save->tx_dropped_hroom += fstats->tx_dropped_hroom;
	save->tx_dropped_dtls += fstats->tx_dropped_dtls;
	save->tx_dropped_nwireless += fstats->tx_dropped_nwireless;

	/*
	 * add pnode stats now.
	 */
	save->pnode_stats.rx_packets += fstats->pnode_stats.rx_packets;
	save->pnode_stats.rx_bytes += fstats->pnode_stats.rx_bytes;
	save->pnode_stats.rx_dropped += fstats->pnode_stats.rx_dropped;
	save->pnode_stats.tx_packets += fstats->pnode_stats.tx_packets;
	save->pnode_stats.tx_bytes += fstats->pnode_stats.tx_bytes;
}

/*
 * nss_capwapmgr_tunnel_destroy()
 *	API for destroying a tunnel. CAPWAP tunnel must be first disabled.
 */
nss_capwapmgr_status_t nss_capwapmgr_tunnel_destroy(struct net_device *dev, uint8_t tunnel_id)
{
	struct nss_capwap_tunnel_stats stats;
	struct nss_ipv4_destroy v4;
	struct nss_ipv6_destroy v6;
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_tunnel *t;
	nss_tx_status_t nss_status = NSS_TX_SUCCESS;
	uint32_t if_num;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_warn("%p: tunnel %d: wrong argument for tunnel destroy\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (!(t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_CONFIGURED)) {
		nss_capwapmgr_warn("%p: tunnel %d is not configured yet\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_TUNNEL_NOT_CFG;
	}

	/*
	 * We don't allow destroy operation on tunnel if it's still enabled.
	 */
	if (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED) {
		nss_capwapmgr_warn("%p: no destroy alloed for an eanbled tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_TUNNEL_ENABLED;
	}

	if (!(t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4 ||
		t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV6)) {
		nss_capwapmgr_warn("%p: tunnel %d: wrong argument for l3_proto\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (!(t->capwap_rule.which_udp == NSS_CAPWAP_TUNNEL_UDP ||
		t->capwap_rule.which_udp == NSS_CAPWAP_TUNNEL_UDPLite)) {
		nss_capwapmgr_warn("%p: tunnel %d: wrong argument for which_udp(%d)\n", dev, tunnel_id, t->capwap_rule.which_udp);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);
	nss_capwapmgr_info("%p: %d: tunnel destroy is being called\n", dev, t->if_num);
	if_num = t->if_num;

	if (priv->if_num_to_tunnel_id[if_num] != tunnel_id) {
		nss_capwapmgr_warn("%p: %d: tunnel_id %d didn't match with tunnel_id :%d\n",
			dev, if_num, tunnel_id, priv->if_num_to_tunnel_id[if_num]);
		dev_put(dev);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (nss_capwap_get_stats(if_num, &stats) == true) {
		nss_capwapmgr_tunnel_save_stats(&global.tunneld, &stats);
	}

	/*
	 * Destroy IP rule first.
	 */
	if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
		memset(&v4, 0, sizeof (struct nss_ipv4_destroy));
		v4.protocol = IPPROTO_UDP;
		v4.src_ip = t->ip_rule.v4.src_ip;
		v4.dest_ip = t->ip_rule.v4.dest_ip;
		v4.src_port = t->ip_rule.v4.src_port;
		v4.dest_port = t->ip_rule.v4.dest_port;
		nss_status = nss_capwapmgr_unconfigure_ipv4_rule(&v4);
	} else {
		memset(&v6, 0, sizeof (struct nss_ipv6_destroy));
		if (t->capwap_rule.which_udp == NSS_CAPWAP_TUNNEL_UDP) {
			v6.protocol = IPPROTO_UDP;
		} else {
			v6.protocol = IPPROTO_UDPLITE;
		}

		v6.src_ip[0] = t->ip_rule.v6.src_ip[0];

		v6.src_ip[1] = t->ip_rule.v6.src_ip[1];
		v6.src_ip[2] = t->ip_rule.v6.src_ip[2];
		v6.src_ip[3] = t->ip_rule.v6.src_ip[3];

		v6.dest_ip[0] = t->ip_rule.v6.dest_ip[0];
		v6.dest_ip[1] = t->ip_rule.v6.dest_ip[1];
		v6.dest_ip[2] = t->ip_rule.v6.dest_ip[2];
		v6.dest_ip[3] = t->ip_rule.v6.dest_ip[3];

		v6.src_port = t->ip_rule.v6.src_port;
		v6.dest_port = t->ip_rule.v6.dest_port;
		nss_status = nss_capwapmgr_unconfigure_ipv6_rule(&v6);
	}

	if (nss_status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%p: %d: Unconfigure IP rule failed for tunnel : %d\n",
			dev, if_num, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_IP_DESTROY_RULE;
	}

	/*
	 * Destroy CAPWAP rule now.
	 */
	status = nss_capwapmgr_tunnel_action(priv->nss_ctx, dev, if_num, NSS_CAPWAP_MSG_TYPE_UNCFG_RULE);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%p: %d: Unconfigure CAPWAP rule failed for tunnel : %d\n",
			dev, if_num, tunnel_id);

		if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
			nss_capwapmgr_configure_ipv4(&t->ip_rule.v4, 0, 0);
		} else {
			nss_capwapmgr_configure_ipv6(&t->ip_rule.v6, 0, 0);
		}

		return NSS_CAPWAPMGR_FAILURE_CAPWAP_DESTROY_RULE;
	}

	nss_capwapmgr_unregister_with_nss(if_num);

	/*
	 * Deallocate dynamic interface
	 */
	nss_status = nss_dynamic_interface_dealloc_node(if_num, NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%p: %d: Dealloc of dynamic interface failed for tunnel : %d\n",
			dev, if_num, tunnel_id);
	}

	/*
	 * Unconfigure Trustsec Tx
	 */
	 if (t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_OUTER_TRUSTSEC_ENABLED) {
		if (t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED) {
			nss_status = nss_trustsec_tx_unconfigure_sgt(t->capwap_rule.dtls_inner_if_num, t->capwap_rule.outer_sgt_value);
		} else {
			nss_status = nss_trustsec_tx_unconfigure_sgt(t->if_num, t->capwap_rule.outer_sgt_value);
		}
		if (nss_status != NSS_TX_SUCCESS) {
			nss_capwapmgr_warn("%p: unconfigure trustsec_tx failed\n", dev);
		}
	 }

	/*
	 * Destroy DTLS node if there is one associated to this tunnel
	 */
	if (t->capwap_rule.dtls_inner_if_num) {
		if (nss_dtlsmgr_session_destroy(t->dtls_dev) != NSS_DTLSMGR_OK) {
			nss_capwapmgr_warn("%p: failed to destroy DTLS session", t->dtls_dev);
		}
	}

	t->if_num = 0;
	t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_CONFIGURED;
	priv->if_num_to_tunnel_id[if_num] = 0;
	memset(t, 0, sizeof(struct nss_capwapmgr_tunnel));

	nss_capwapmgr_info("%p: %d: Tunnel %d is completely destroyed\n", dev, if_num, tunnel_id);
	dev_put(dev);
	return NSS_CAPWAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_capwapmgr_tunnel_destroy);

/*
 * nss_capwapmgr_flow_rule_action()
 */
static inline nss_capwapmgr_status_t nss_capwapmgr_flow_rule_action(struct net_device *dev, uint8_t tunnel_id,
						nss_capwap_msg_type_t cmd, uint16_t ip_version,
						uint16_t protocol, uint32_t *src_ip, uint32_t *dst_ip,
						uint16_t src_port, uint16_t dst_port, uint32_t flow_id)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwap_msg capwapmsg;
	struct nss_capwap_flow_rule_msg *ncfrm;
	struct nss_capwapmgr_tunnel *t;
	nss_capwapmgr_status_t status;

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_warn("%p: can't find tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);

	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));
	nss_capwap_msg_init(&capwapmsg, t->if_num, cmd,
		sizeof(struct nss_capwap_flow_rule_msg), nss_capwapmgr_msg_event_receive, dev);

	/*
	 * Set flow rule message
	 */
	if (cmd == NSS_CAPWAP_MSG_TYPE_FLOW_RULE_ADD) {
		ncfrm = &capwapmsg.msg.flow_rule_add;
	} else {
		ncfrm = &capwapmsg.msg.flow_rule_del;
	}
	ncfrm->protocol = protocol;
	ncfrm->src_port = src_port;
	ncfrm->dst_port = dst_port;
	ncfrm->ip_version = ip_version;
	memcpy(ncfrm->src_ip, src_ip, sizeof(struct in6_addr));
	memcpy(ncfrm->dst_ip, dst_ip, sizeof(struct in6_addr));
	ncfrm->flow_id = flow_id;

	/*
	 * Send flow rule message to NSS core
	 */
	status = nss_capwapmgr_tx_msg_sync(priv->nss_ctx, dev, &capwapmsg);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%p: send flow rule message failed with error: %d\n", dev, status);
	}

	dev_put(dev);
	return status;
}

/*
 * nss_capwapmgr_add_flow_rule()
 *	Send a capwap flow rule add message to NSS core.
 */
nss_capwapmgr_status_t nss_capwapmgr_add_flow_rule(struct net_device *dev, uint8_t tunnel_id, uint16_t ip_version,
						uint16_t protocol, uint32_t *src_ip, uint32_t *dst_ip,
						uint16_t src_port, uint16_t dst_port, uint32_t flow_id)
{
	return nss_capwapmgr_flow_rule_action(dev, tunnel_id, NSS_CAPWAP_MSG_TYPE_FLOW_RULE_ADD, ip_version,
											protocol, src_ip, dst_ip, src_port, dst_port, flow_id);
}
EXPORT_SYMBOL(nss_capwapmgr_add_flow_rule);

/*
 * nss_capwapmgr_del_flow_rule()
 *	Send a capwap flow rule del message to NSS core.
 */
nss_capwapmgr_status_t nss_capwapmgr_del_flow_rule(struct net_device *dev, uint8_t tunnel_id, uint16_t ip_version,
						uint16_t protocol, uint32_t *src_ip, uint32_t *dst_ip,
						uint16_t src_port, uint16_t dst_port)
{
	return nss_capwapmgr_flow_rule_action(dev, tunnel_id, NSS_CAPWAP_MSG_TYPE_FLOW_RULE_DEL, ip_version,
											protocol, src_ip, dst_ip, src_port, dst_port, 0);
}
EXPORT_SYMBOL(nss_capwapmgr_del_flow_rule);

/*
 * nss_capwapmgr_tunnel_stats()
 *	Gets tunnel stats from netdev
 */
nss_capwapmgr_status_t nss_capwapmgr_tunnel_stats(struct net_device *dev,
		uint8_t tunnel_id, struct nss_capwap_tunnel_stats *stats)
{
	struct nss_capwapmgr_tunnel *t;

	if (!stats) {
		nss_capwapmgr_warn("%p: invalid rtnl structure\n", dev);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	t = nss_capwapmgr_verify_tunnel_param(dev, tunnel_id);
	if (!t) {
		nss_capwapmgr_trace("%p: can't find tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	if (!(t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_CONFIGURED)) {
		nss_capwapmgr_trace("%p: tunnel: %d not configured yet\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_TUNNEL_NOT_CFG;
	}

	if (nss_capwap_get_stats(t->if_num, stats) == false) {
		nss_capwapmgr_warn("%p: tunnel %d not ready yet\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_NOT_READY;
	}

	return NSS_CAPWAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_capwapmgr_tunnel_stats);

/*
 * nss_capwapmgr_receive_pkt()
 *	Receives a pkt from NSS
 */
static void nss_capwapmgr_receive_pkt(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwap_metaheader *pre = (struct nss_capwap_metaheader *)skb->data;
	int32_t if_num;

	if (unlikely(skb->len < sizeof(struct nss_capwap_metaheader))) {
		nss_capwapmgr_warn("%p: skb len is short :%d", dev, skb->len);
		dev_kfree_skb_any(skb);
		return;
	}

	/* SKB NETIF START */
	dev_hold(dev);
	priv = netdev_priv(dev);
	if_num = pre->tunnel_id;	/* NSS FW sends interface number */
	if (unlikely(if_num > NSS_MAX_NET_INTERFACES)) {
		nss_capwapmgr_warn("%p: if_num %d is wrong for skb\n", dev, if_num);
		pre->tunnel_id = 0xFF;
	} else {
		/*
		 * Remap interface number to tunnel_id.
		 */
		pre->tunnel_id = priv->if_num_to_tunnel_id[if_num];
	}

	skb->dev = dev;
	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = dev->ifindex;
	skb_reset_mac_header(skb);
	skb_reset_transport_header(skb);
	(void)netif_receive_skb(skb);
	/* SKB NETIF END */
	dev_put(dev);
}

/*
 * nss_capwapmgr_acl_init()
 *	Initializes ACL related tables and objects.
 */
bool nss_capwapmgr_acl_init(void)
{
	sw_error_t rv;
	int i, j, uid = 0;

	/*
	 * Create and bind the ACL list we will be using for dscp prioritization.
	 */
	for (i = 0; i < NSS_CAPWAPMGR_ACL_LIST_CNT; i++) {
		int list_id = NSS_CAPWAPMGR_ACL_LIST_START + i;
		rv = fal_acl_list_creat(0, list_id, 0);
		if (rv != SW_OK) {
			nss_capwapmgr_warn("Failed to create ACL list err:%d\n", rv);
			return false;
		}

		rv = fal_acl_list_bind(0, list_id, FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORTBITMAP, NSS_CAPWAPMGR_BIND_BITMAP);
		if (rv != SW_OK) {
			nss_capwapmgr_warn("Failed to bind ACL list err:%d\n", rv);
			return false;
		}
	}

	/*
	 * Initialize the globacl ACL table.
	 */
	for (i = 0; i < NSS_CAPWAPMGR_ACL_LIST_CNT; i++) {
		for (j = 0; j < NSS_CAPWAPMGR_ACL_RULES_PER_LIST; j++) {
			global.acl_list[i].rule[j].uid = uid++;
			global.acl_list[i].rule[j].rule_id = j;
			global.acl_list[i].rule[j].list_id = i;
		}
	}

	return true;
}

#if defined(NSS_CAPWAPMGR_ONE_NETDEV)
/*
 * nss_capwapmgr_get_netdev()
 *	Returns net device used.
 */
struct net_device *nss_capwapmgr_get_netdev(void)
{
	return nss_capwapmgr_ndev;
}
EXPORT_SYMBOL(nss_capwapmgr_get_netdev);
#endif

/*
 * nss_capwapmgr_netdev_up()
 *	NSS CAPWAP Tunnel device i/f up handler
 */
static int nss_capwapmgr_netdev_up(struct net_device *netdev)
{
	struct nss_capwapmgr_priv *priv;
	uint8_t i;

	priv = netdev_priv(netdev);
	for (i = 0; i < NSS_CAPWAPMGR_MAX_TUNNELS; i++) {
		(void)nss_capwapmgr_enable_tunnel(nss_capwapmgr_ndev, i);
	}

	return NOTIFY_DONE;
}

/*
 * nss_capwapmgr_netdev_down()
 *	NSS CAPWAP Tunnel device i/f up handler
 */
static int nss_capwapmgr_netdev_down(struct net_device *netdev)
{
	struct nss_capwapmgr_priv *priv;
	uint8_t i;

	priv = netdev_priv(netdev);
	for (i = 0; i < NSS_CAPWAPMGR_MAX_TUNNELS; i++) {
		(void)nss_capwapmgr_disable_tunnel(nss_capwapmgr_ndev, i);
	}

	return NOTIFY_DONE;
}

/*
 * nss_capwapmgr_netdev_event()
 *	Net device notifier for NSS CAPWAP manager module
 */
static int nss_capwapmgr_netdev_event(struct notifier_block  *nb, unsigned long event, void  *dev)
{
	struct net_device *netdev = (struct net_device *)dev;

	if (strstr(netdev->name, NSS_CAPWAPMGR_NETDEV_NAME) == NULL) {
		return NOTIFY_DONE;
	}

	switch (event) {
	case NETDEV_UP:
		nss_capwapmgr_trace("%p: NETDEV_UP: event %lu name %s\n", netdev, event, netdev->name);
		return nss_capwapmgr_netdev_up(netdev);

	case NETDEV_DOWN:
		nss_capwapmgr_trace("%p: NETDEV_DOWN: event %lu name %s\n", netdev, event, netdev->name);
		return nss_capwapmgr_netdev_down(netdev);

	default:
		nss_capwapmgr_trace("%p: Unhandled notifier event %lu name %s\n", netdev, event, netdev->name);
		break;
	}

	return NOTIFY_DONE;
}

/*
 * Linux netdev event function.
 */
static int nss_capwapmgr_netdev_event(struct notifier_block  *nb, unsigned long event, void  *dev);

/*
 * Linux Net device Notifier
 */
struct notifier_block nss_capwapmgr_netdev_notifier = {
	.notifier_call = nss_capwapmgr_netdev_event,
};

/*
 * nss_capwapmgr_init_module()
 *	Tunnel CAPWAP module init function
 */
int __init nss_capwapmgr_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	nss_capwapmgr_info("module (platform - IPQ806x, %s) loaded\n",
			   NSS_CLIENT_BUILD_ID);

	register_netdevice_notifier(&nss_capwapmgr_netdev_notifier);

#if defined(NSS_CAPWAPMGR_ONE_NETDEV)
	/*
	 * In this code, we create a single netdev for all the CAPWAP
	 * tunnels.
	 */
	nss_capwapmgr_ndev = nss_capwapmgr_netdev_create();
	if (!nss_capwapmgr_ndev) {
		nss_capwapmgr_warn("Couldn't create capwap interface\n");
		return -1;
	}
#endif

	memset(&global.tunneld, 0, sizeof(struct nss_capwap_tunnel_stats));

	/*
	 * Initialize ACL related objects and tables.
	 */
	if (!nss_capwapmgr_acl_init()) {
		nss_capwapmgr_warn("Couldn't initialize ACL objects/tables\n");
		return -1;
	}

	sema_init(&ip_response.sem, 1);
	init_waitqueue_head(&ip_response.wq);

	return 0;
}

/*
 * nss_capwapmgr_exit_module()
 *	Tunnel CAPWAP module exit function
 */
void __exit nss_capwapmgr_exit_module(void)
{
#if defined(NSS_CAPWAPMGR_ONE_NETDEV)
	struct nss_capwapmgr_priv *priv;
	uint8_t i;
#endif

#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif

#if defined(NSS_CAPWAPMGR_ONE_NETDEV)
	priv = netdev_priv(nss_capwapmgr_ndev);
	for (i = 0; i < NSS_CAPWAPMGR_MAX_TUNNELS; i++) {
		(void) nss_capwapmgr_disable_tunnel(nss_capwapmgr_ndev, i);
		(void) nss_capwapmgr_tunnel_destroy(nss_capwapmgr_ndev, i);
	}
	kfree(priv->if_num_to_tunnel_id);
	kfree(priv->resp);
	kfree(priv->tunnel);
	unregister_netdev(nss_capwapmgr_ndev);
	nss_capwapmgr_ndev = NULL;
#endif
	unregister_netdevice_notifier(&nss_capwapmgr_netdev_notifier);

	nss_capwapmgr_info("module unloaded\n");
}

module_init(nss_capwapmgr_init_module);
module_exit(nss_capwapmgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS CAPWAP manager");
