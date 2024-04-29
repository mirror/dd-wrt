/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
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

#include <linux/if_ether.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/version.h>
#include <net/addrconf.h>
#include <net/dst.h>
#include <net/flow.h>
#include <net/ipv6.h>
#include <net/route.h>
#include <net/vxlan.h>
#include <nss_api_if.h>
#include "nss_vxlanmgr.h"
#include "nss_vxlanmgr_tun_stats.h"

/*
 * VxLAN context
 */
extern struct nss_vxlanmgr_ctx vxlan_ctx;

/*
 * nss_vxlanmgr_tunnel_ctx_dev_get()
 *	Find VxLAN tunnel context using netdev.
 *	Context lock must be held before calling this API.
 */
struct nss_vxlanmgr_tun_ctx *nss_vxlanmgr_tunnel_ctx_dev_get(struct net_device *dev)
{
	struct nss_vxlanmgr_tun_ctx *tun_ctx;

	list_for_each_entry(tun_ctx, &vxlan_ctx.list, head) {
		if (tun_ctx->dev == dev) {
			return tun_ctx;
		}
	}

	return NULL;
}

/*
 * nss_vxlanmgr_tunnel_tx_msg()
 *	Transmit VxLAN tunnel operation messages asynchronously.
 */
static nss_tx_status_t nss_vxlanmgr_tunnel_tx_msg(struct nss_ctx_instance *ctx,
						  struct nss_vxlan_msg *msg,
						  uint32_t if_num,
						  enum nss_vxlan_msg_type type,
						  uint32_t len)
{
	nss_vxlan_msg_init(msg, if_num, type, len, NULL, NULL);
	return nss_vxlan_tx_msg(ctx, msg);
}

/*
 * nss_vxlanmgr_tunnel_tx_msg_sync()
 *	Transmit VxLAN tunnel operation messages.
 */
static nss_tx_status_t nss_vxlanmgr_tunnel_tx_msg_sync(struct nss_ctx_instance *ctx,
							struct nss_vxlan_msg *msg,
							uint32_t if_num,
							enum nss_vxlan_msg_type type,
							uint32_t len)
{
	nss_vxlan_msg_init(msg, if_num, type, len, NULL, NULL);
	return nss_vxlan_tx_msg_sync(ctx, msg);
}

/*
 * nss_vxlanmgr_tunnel_flags_parse()
 *	Function to parse vxlan flags.
 */
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 5, 7))
static uint16_t nss_vxlanmgr_tunnel_flags_parse(struct vxlan_dev *priv)
{
	uint16_t flags = 0;
	uint32_t priv_flags = priv->flags;

	if (priv_flags & VXLAN_F_GBP)
		flags |= NSS_VXLAN_RULE_FLAG_GBP_ENABLED;
	if (priv_flags & VXLAN_F_IPV6)
		flags |= NSS_VXLAN_RULE_FLAG_IPV6;
	else if (!(priv_flags & VXLAN_F_IPV6))
		flags |= NSS_VXLAN_RULE_FLAG_IPV4;
	if (priv->cfg.tos == 1)
		flags |= NSS_VXLAN_RULE_FLAG_INHERIT_TOS;
	if (priv_flags & VXLAN_F_UDP_CSUM)
		flags |= NSS_VXLAN_RULE_FLAG_ENCAP_L4_CSUM_REQUIRED;
	else if (!(priv_flags & VXLAN_F_UDP_ZERO_CSUM6_TX))
		flags |= NSS_VXLAN_RULE_FLAG_ENCAP_L4_CSUM_REQUIRED;

	return (flags | NSS_VXLAN_RULE_FLAG_UDP);
}
#else
static uint16_t nss_vxlanmgr_tunnel_flags_parse(struct vxlan_dev *priv)
{
	uint16_t flags = 0;
	struct vxlan_config *cfg = &priv->cfg;
	uint32_t priv_flags = cfg->flags;

	if (priv_flags & VXLAN_F_GBP)
		flags |= NSS_VXLAN_RULE_FLAG_GBP_ENABLED;
	if (priv_flags & VXLAN_F_IPV6)
		flags |= NSS_VXLAN_RULE_FLAG_IPV6;
	else if (!(priv_flags & VXLAN_F_IPV6))
		flags |= NSS_VXLAN_RULE_FLAG_IPV4;
	if (cfg->tos == 1)
		flags |= NSS_VXLAN_RULE_FLAG_INHERIT_TOS;
	if (priv_flags & VXLAN_F_UDP_ZERO_CSUM_TX)
		flags |= NSS_VXLAN_RULE_FLAG_ENCAP_L4_CSUM_REQUIRED;
	else if (!(priv_flags & VXLAN_F_UDP_ZERO_CSUM6_TX))
		flags |= NSS_VXLAN_RULE_FLAG_ENCAP_L4_CSUM_REQUIRED;

	return (flags | NSS_VXLAN_RULE_FLAG_UDP);
}
#endif

/*
 * nss_vxlanmgr_tunnel_fill_src_ip()
 *	Return src_ip using route lookup.
 */
static bool nss_vxlanmgr_tunnel_fill_src_ip(struct vxlan_dev *vxlan,
						union vxlan_addr *src_ip,
						union vxlan_addr *rem_ip,
						sa_family_t sa_family,
						uint32_t *new_src_ip)
{
	struct flowi4 fl4;
	struct flowi6 fl6;
	struct rtable *rt = NULL;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 5, 7))
	struct dst_entry *dst = NULL;
	int err;
#else
	const struct in6_addr *final_dst = NULL;
	struct dst_entry *dentry;
#endif

	/*
	 * IPv4
	 */
	if (sa_family == AF_INET) {
		if (src_ip->sin.sin_addr.s_addr == htonl(INADDR_ANY)) {
			/*
			 * Lookup
			 */
			memset(&fl4, 0, sizeof(fl4));
			fl4.flowi4_proto = IPPROTO_UDP;
			fl4.daddr = rem_ip->sin.sin_addr.s_addr;
			fl4.saddr = src_ip->sin.sin_addr.s_addr;

			rt = ip_route_output_key(vxlan->net, &fl4);
			if (IS_ERR(rt)) {
				nss_vxlanmgr_warn("No route available.\n");
				return false;
			}
			new_src_ip[0] = fl4.saddr;
			return true;
		}
		new_src_ip[0] = src_ip->sin.sin_addr.s_addr;
		return true;
	}

	/*
	 * IPv6
	 */
	if (ipv6_addr_any(&src_ip->sin6.sin6_addr)) {
		/*
		 * Lookup
		 */
		memset(&fl6, 0, sizeof(fl6));
		fl6.flowi6_proto = IPPROTO_UDP;
		fl6.daddr = rem_ip->sin6.sin6_addr;
		fl6.saddr = src_ip->sin6.sin6_addr;

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 5, 7))
		err = ipv6_stub->ipv6_dst_lookup(vxlan->net,
				vxlan->vn6_sock->sock->sk, &dst, &fl6);
		if (err < 0) {
#else
		dentry = ipv6_stub->ipv6_dst_lookup_flow(vxlan->net,
				vxlan->vn6_sock->sock->sk, &fl6, final_dst);
		if (!dentry) {
#endif
			nss_vxlanmgr_warn("No route, drop packet.\n");
			return false;
		}
		memcpy(new_src_ip, &fl6.saddr, sizeof(struct in6_addr));
		return true;
	}
	memcpy(new_src_ip, &src_ip->sin6.sin6_addr, sizeof(struct in6_addr));
	return true;
}

/*
 * nss_vxlanmgr_tunnel_mac_del()
 *	VxLAN tunnel mac delete messages.
 */
static nss_tx_status_t nss_vxlanmgr_tunnel_mac_del(struct nss_vxlanmgr_tun_ctx *tun_ctx,
						   struct vxlan_fdb_event *vfe)
{
	struct net_device *dev;
	struct nss_vxlan_mac_msg *mac_del_msg;
	struct nss_vxlan_msg vxlanmsg;
	struct vxlan_config *cfg;
	struct vxlan_dev *priv;
	union vxlan_addr *remote_ip, *src_ip;
	uint32_t i, inner_ifnum;
	nss_tx_status_t status = NSS_TX_FAILURE;

	dev = vfe->dev;
	dev_hold(dev);

	spin_lock_bh(&vxlan_ctx.tun_lock);
	inner_ifnum = tun_ctx->inner_ifnum;
	spin_unlock_bh(&vxlan_ctx.tun_lock);

	/*
	 * Only non-zero mac entries should be sent to NSS.
	 */
	if (is_zero_ether_addr(vfe->eth_addr)) {
		nss_vxlanmgr_trace("Only non-zero mac entries should be sent to NSS.\n");
		goto done;
	}

	memset(&vxlanmsg, 0, sizeof(struct nss_vxlan_msg));
	priv = netdev_priv(dev);

	/*
	 * Set MAC rule message
	 */
	mac_del_msg = &vxlanmsg.msg.mac_del;
	mac_del_msg->vni = vxlan_get_vni(priv);
	ether_addr_copy((uint8_t *)mac_del_msg->mac_addr, (uint8_t *)vfe->eth_addr);

	cfg = &priv->cfg;
	src_ip = &cfg->saddr;
	remote_ip = &vfe->rdst->remote_ip;

	if (remote_ip->sa.sa_family == AF_INET){
		if (remote_ip->sin.sin_addr.s_addr == htonl(INADDR_ANY)) {
			nss_vxlanmgr_warn("%px: MAC deletion failed for unknown remote\n", dev);
			goto done;
		}
		memcpy(&mac_del_msg->encap.dest_ip, &remote_ip->sin.sin_addr, sizeof(struct in_addr));
		memcpy(&mac_del_msg->encap.src_ip, &src_ip->sin.sin_addr, sizeof(struct in_addr));
	} else {
		if (ipv6_addr_any(&remote_ip->sin6.sin6_addr)) {
			nss_vxlanmgr_warn("%px: MAC deletion failed for unknown remote\n", dev);
			goto done;
		}
		memcpy(&mac_del_msg->encap.dest_ip, &remote_ip->sin6.sin6_addr, sizeof(struct in6_addr));
		memcpy(&mac_del_msg->encap.src_ip, &src_ip->sin6.sin6_addr, sizeof(struct in6_addr));
	}

	/*
	 * Send MAC del message asynchronously as it is called by chain
	 * notifier in atomic context from the vxlan driver.
	 */
	status = nss_vxlanmgr_tunnel_tx_msg(vxlan_ctx.nss_ctx,
						&vxlanmsg,
						inner_ifnum,
						NSS_VXLAN_MSG_TYPE_MAC_DEL,
						sizeof(struct nss_vxlan_mac_msg));
	if (status != NSS_TX_SUCCESS) {
		nss_vxlanmgr_warn("%px: MAC deletion failed %d\n", dev, status);
	}

	spin_lock_bh(&vxlan_ctx.tun_lock);
	for (i = 0; i < NSS_VXLAN_MACDB_ENTRIES_MAX; i++) {
		if (ether_addr_equal((uint8_t *)&tun_ctx->stats->mac_stats[i][0], (uint8_t *)vfe->eth_addr)) {
			tun_ctx->stats->mac_stats[i][0] = 0;
			tun_ctx->stats->mac_stats[i][1] = 0;
			break;
		}
	}
	spin_unlock_bh(&vxlan_ctx.tun_lock);

done:
	dev_put(dev);
	return status;
}

/*
 * nss_vxlanmgr_tunnel_mac_add()
 *	VxLAN tunnel mac add messages.
 */
static nss_tx_status_t nss_vxlanmgr_tunnel_mac_add(struct nss_vxlanmgr_tun_ctx *tun_ctx,
						   struct vxlan_fdb_event *vfe)
{
	struct net_device *dev;
	struct nss_vxlan_mac_msg *mac_add_msg;
	struct nss_vxlan_msg vxlanmsg;
	struct vxlan_config *cfg;
	struct vxlan_dev *priv;
	union vxlan_addr *remote_ip, *src_ip;
	uint32_t i, inner_ifnum;
	uint32_t new_src_ip[4] = {0};
	nss_tx_status_t status = NSS_TX_FAILURE;

	dev = vfe->dev;
	dev_hold(dev);

	spin_lock_bh(&vxlan_ctx.tun_lock);
	inner_ifnum = tun_ctx->inner_ifnum;
	spin_unlock_bh(&vxlan_ctx.tun_lock);

	/*
	 * Only non-zero mac entries should be sent to NSS.
	 */
	if (is_zero_ether_addr(vfe->eth_addr)) {
		nss_vxlanmgr_trace("Only non-zero mac entries should be sent to NSS.\n");
		goto done;
	}

	memset(&vxlanmsg, 0, sizeof(struct nss_vxlan_msg));
	priv = netdev_priv(dev);

	/*
	 * Set MAC rule message
	 */
	mac_add_msg = &vxlanmsg.msg.mac_add;
	mac_add_msg->vni = vxlan_get_vni(priv);
	ether_addr_copy((uint8_t *)mac_add_msg->mac_addr, (uint8_t *)vfe->eth_addr);

	cfg = &priv->cfg;
	src_ip = &cfg->saddr;
	remote_ip = &vfe->rdst->remote_ip;

	if (remote_ip->sa.sa_family == AF_INET){
		if (remote_ip->sin.sin_addr.s_addr == htonl(INADDR_ANY)) {
			nss_vxlanmgr_warn("%px: MAC addition failed for unknown remote\n", dev);
			goto done;
		}
		memcpy(&mac_add_msg->encap.dest_ip[0], &remote_ip->sin.sin_addr, sizeof(struct in_addr));
		if (!nss_vxlanmgr_tunnel_fill_src_ip(priv, src_ip, remote_ip, AF_INET, new_src_ip)) {
			nss_vxlanmgr_warn("%px: MAC addition failed for unknown source\n", dev);
			goto done;
		}
		mac_add_msg->encap.src_ip[0] = new_src_ip[0];
	} else {
		if (ipv6_addr_any(&remote_ip->sin6.sin6_addr)) {
			nss_vxlanmgr_warn("%px: MAC addition failed for unknown remote\n", dev);
			goto done;
		}
		memcpy(mac_add_msg->encap.dest_ip, &remote_ip->sin6.sin6_addr, sizeof(struct in6_addr));
		if (!nss_vxlanmgr_tunnel_fill_src_ip(priv, src_ip, remote_ip, AF_INET6, new_src_ip)) {
			nss_vxlanmgr_warn("%px: MAC addition failed for unknown source\n", dev);
			goto done;
		}
		memcpy(mac_add_msg->encap.src_ip, new_src_ip, sizeof(struct in6_addr));
	}

	/*
	 * Send MAC add message asynchronously as it is called by chain
	 * notifier in atomic context from the vxlan driver.
	 */
	status = nss_vxlanmgr_tunnel_tx_msg(vxlan_ctx.nss_ctx,
						&vxlanmsg,
						inner_ifnum,
						NSS_VXLAN_MSG_TYPE_MAC_ADD,
						sizeof(struct nss_vxlan_mac_msg));
	if (status != NSS_TX_SUCCESS) {
		nss_vxlanmgr_warn("%px: MAC addition failed %d\n", dev, status);
		goto done;
	}

	spin_lock_bh(&vxlan_ctx.tun_lock);
	for (i = 0; i < NSS_VXLAN_MACDB_ENTRIES_MAX; i++) {
		if (!tun_ctx->stats->mac_stats[i][0]) {
			ether_addr_copy((uint8_t *)&tun_ctx->stats->mac_stats[i][0],
					(uint8_t *)vfe->eth_addr);
			break;
		}
	}
	spin_unlock_bh(&vxlan_ctx.tun_lock);

done:
	dev_put(dev);
	return status;
}

/*
 * nss_vxlanmgr_tunnel_fdb_event()
 *	Event handler for VxLAN fdb updates
 */
static int nss_vxlanmgr_tunnel_fdb_event(struct notifier_block *nb, unsigned long event, void *data)
{
	struct vxlan_fdb_event *vfe;
	struct nss_vxlanmgr_tun_ctx *tun_ctx;

	vfe = (struct vxlan_fdb_event *)data;
	spin_lock_bh(&vxlan_ctx.tun_lock);
	tun_ctx = nss_vxlanmgr_tunnel_ctx_dev_get(vfe->dev);
	if (!tun_ctx) {
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_warn("%px: Invalid tunnel context\n", vfe->dev);
		return NOTIFY_DONE;
	}
	spin_unlock_bh(&vxlan_ctx.tun_lock);

	switch(event) {
	case RTM_DELNEIGH:
		nss_vxlanmgr_tunnel_mac_del(tun_ctx, vfe);
		break;
	case RTM_NEWNEIGH:
		nss_vxlanmgr_tunnel_mac_add(tun_ctx, vfe);
		break;
	default:
		nss_vxlanmgr_warn("%lu: Unknown FDB event received.\n", event);
	}
	return NOTIFY_DONE;
}

/*
 * Notifier to receive fdb events from VxLAN
 */
static struct notifier_block nss_vxlanmgr_tunnel_fdb_notifier = {
	.notifier_call = nss_vxlanmgr_tunnel_fdb_event,
};

/*
 * nss_vxlanmgr_tunnel_inner_stats()
 *	Update vxlan netdev stats with inner node stats
 */
static void nss_vxlanmgr_tunnel_inner_stats(struct nss_vxlanmgr_tun_ctx *tun_ctx, struct nss_vxlan_msg *nvm)
{
	struct nss_vxlan_stats_msg *stats;
	struct pcpu_sw_netstats *tstats;
	struct net_device *dev;
	struct net_device_stats *netdev_stats;
	uint32_t i;
	uint64_t dropped = 0;

	stats = &nvm->msg.stats;
	dev = tun_ctx->dev;

	dev_hold(dev);
	netdev_stats = (struct net_device_stats *)&dev->stats;

	/*
	 * Only look at the tx_packets/tx_bytes for both host_inner/outer interfaces.
	 * rx_bytes/rx_packets are increased when the packet is received by the node.
	 * Therefore, it includes both transmitted/dropped packets. tx_bytes/tx_packets
	 * reflect successfully transmitted packets.
	 */
	for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
		dropped += stats->node_stats.rx_dropped[i];
	}

	tstats = this_cpu_ptr(dev->tstats);
	u64_stats_update_begin(&tstats->syncp);
	u64_stats_add(&tstats->tx_packets, stats->node_stats.tx_packets);
	u64_stats_add(&tstats->tx_bytes, stats->node_stats.tx_bytes);
	u64_stats_update_end(&tstats->syncp);
	netdev_stats->tx_dropped += dropped;
	dev_put(dev);
}

/*
 * nss_vxlanmgr_tunnel_outer_stats()
 *	Update vxlan netdev stats with outer node stats
 */
static void nss_vxlanmgr_tunnel_outer_stats(struct nss_vxlanmgr_tun_ctx *tun_ctx, struct nss_vxlan_msg *nvm)
{
	struct nss_vxlan_stats_msg *stats;
	struct pcpu_sw_netstats *tstats;
	struct net_device *dev;
	struct net_device_stats *netdev_stats;
	uint32_t i;
	uint64_t dropped = 0;

	stats = &nvm->msg.stats;
	dev = tun_ctx->dev;

	dev_hold(dev);
	netdev_stats = (struct net_device_stats *)&dev->stats;

	/*
	 * Only look at the tx_packets/tx_bytes for both host_inner/outer interfaces.
	 * rx_bytes/rx_packets are increased when the packet is received by the node.
	 * Therefore, it includes both transmitted/dropped packets. tx_bytes/tx_packets
	 * reflect successfully transmitted packets.
	 */
	for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
		dropped += stats->node_stats.rx_dropped[i];
	}

	tstats = this_cpu_ptr(dev->tstats);
	u64_stats_update_begin(&tstats->syncp);
	u64_stats_add(&tstats->rx_packets, stats->node_stats.rx_packets);
	u64_stats_add(&tstats->rx_bytes, stats->node_stats.rx_bytes);
	u64_stats_update_end(&tstats->syncp);
	netdev_stats->rx_dropped += dropped;
	dev_put(dev);
}

/*
 * nss_vxlanmgr_tunnel_fdb_update()
 *	Update vxlan fdb entries
 */
static void nss_vxlanmgr_tunnel_fdb_update(struct nss_vxlanmgr_tun_ctx *tun_ctx, struct nss_vxlan_msg *nvm)
{
	uint8_t *mac;
	uint16_t i, nentries;
	struct vxlan_dev *priv;
	struct nss_vxlan_macdb_stats_msg *db_stats;

	db_stats = &nvm->msg.db_stats;
	nentries = db_stats->cnt;
	priv = netdev_priv(tun_ctx->dev);

	dev_hold(tun_ctx->dev);

	if (nentries > NSS_VXLAN_MACDB_ENTRIES_PER_MSG) {
		nss_vxlanmgr_warn("%px: No more than 20 entries allowed per message.\n", tun_ctx->dev);
		dev_put(tun_ctx->dev);
		return;
	}

	for (i = 0; i < nentries; i++) {
		if (likely(db_stats->entry[i].hits)) {
			mac = (uint8_t *)db_stats->entry[i].mac;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 5, 7))
			vxlan_fdb_update_mac(priv, mac);
#else
			vxlan_fdb_update_mac(priv, mac, tun_ctx->vni);
#endif
		}
	}
	dev_put(tun_ctx->dev);
}

/*
 * nss_vxlanmgr_tunnel_inner_notifier()
 *	Notifier for vxlan tunnel encap node
 */
static void nss_vxlanmgr_tunnel_inner_notifier(void *app_data, struct nss_cmn_msg *ncm)
{
	struct net_device *dev = (struct net_device *)app_data;
	struct nss_vxlanmgr_tun_ctx *tun_ctx;
	struct nss_vxlan_msg *nvm;

	if (!ncm) {
	    nss_vxlanmgr_info("%px: NULL msg received.\n", dev);
	    return;
	}

	spin_lock_bh(&vxlan_ctx.tun_lock);
	tun_ctx = nss_vxlanmgr_tunnel_ctx_dev_get(dev);
	if (!tun_ctx) {
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_warn("%px: Invalid tunnel context\n", dev);
		return;
	}

	nvm = (struct nss_vxlan_msg *)ncm;
	switch (nvm->cm.type) {
	case NSS_VXLAN_MSG_TYPE_STATS_SYNC:
		nss_vxlanmgr_tunnel_inner_stats(tun_ctx, nvm);
		nss_vxlanmgr_tun_stats_sync(tun_ctx, nvm);
		break;
	case NSS_VXLAN_MSG_TYPE_MACDB_STATS:
		nss_vxlanmgr_tunnel_fdb_update(tun_ctx, nvm);
		nss_vxlanmgr_tun_macdb_stats_sync(tun_ctx, nvm);
		break;
	default:
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_info("%px: Unknown Event from NSS", dev);
		return;
	}
	spin_unlock_bh(&vxlan_ctx.tun_lock);
}

/*
 * nss_vxlanmgr_tunnel_outer_notifier()
 *	Notifier for vxlan tunnel decap node
 */
static void nss_vxlanmgr_tunnel_outer_notifier(void *app_data, struct nss_cmn_msg *ncm)
{
	struct net_device *dev = (struct net_device *)app_data;
	struct nss_vxlanmgr_tun_ctx *tun_ctx;
	struct nss_vxlan_msg *nvm;

	if (!ncm) {
	    nss_vxlanmgr_info("%px: NULL msg received.\n", dev);
	    return;
	}

	spin_lock_bh(&vxlan_ctx.tun_lock);
	tun_ctx = nss_vxlanmgr_tunnel_ctx_dev_get(dev);
	if (!tun_ctx) {
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_warn("%px: Invalid tunnel context\n", dev);
		return;
	}

	nvm = (struct nss_vxlan_msg *)ncm;
	switch (nvm->cm.type) {
	case NSS_VXLAN_MSG_TYPE_STATS_SYNC:
		nss_vxlanmgr_tunnel_outer_stats(tun_ctx, nvm);
		nss_vxlanmgr_tun_stats_sync(tun_ctx, nvm);
		break;
	default:
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_info("%px: Unknown Event from NSS", dev);
		return;
	}
	spin_unlock_bh(&vxlan_ctx.tun_lock);
}

/*
 * nss_vxlanmgr_tunnel_inner_recv()
 *	Receives a pkt from NSS
 */
static void nss_vxlanmgr_tunnel_inner_recv(struct net_device *dev, struct sk_buff *skb,
		__attribute__((unused)) struct napi_struct *napi)
{
	dev_hold(dev);
	nss_vxlanmgr_info("%px: (vxlan packet) Exception packet received.\n", dev);

	/*
	 * These are decapped and exceptioned packets.
	 */
	skb->protocol = eth_type_trans(skb, dev);
	netif_receive_skb(skb);
	dev_put(dev);
	return;
}

/*
 * nss_vxlanmgr_tunnel_outer_recv()
 *	Receives a pkt from NSS
 */
static void nss_vxlanmgr_tunnel_outer_recv(struct net_device *dev, struct sk_buff *skb,
		__attribute__((unused)) struct napi_struct *napi)
{
	struct iphdr *iph;
	size_t l3_hdr_size;
	struct nss_vxlanmgr_tun_ctx *tun_ctx;

	nss_vxlanmgr_info("%px: (vxlan packet) Exception packet received.\n", dev);

	spin_lock_bh(&vxlan_ctx.tun_lock);
	tun_ctx = nss_vxlanmgr_tunnel_ctx_dev_get(dev);
	if (!tun_ctx) {
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_warn("%px: Invalid tunnel context\n", dev);
		dev_kfree_skb_any(skb);
		return;
	}

	iph = (struct iphdr *)skb->data;
	switch (iph->version) {
	case 4:
		l3_hdr_size = sizeof(struct iphdr);
		skb->protocol = htons(ETH_P_IP);
		break;
	case 6:
		l3_hdr_size = sizeof(struct ipv6hdr);
		skb->protocol = htons(ETH_P_IPV6);
		break;
	default:
		tun_ctx->stats->host_packet_drop++;
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_trace("%px: Skb received with unknown IP version: %d.\n", dev, iph->version);
		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * VxLAN encapsulated packet exceptioned, remove the encapsulation
	 * and transmit on VxLAN interface.
	 */
	if (unlikely(!pskb_may_pull(skb, (l3_hdr_size + sizeof(struct udphdr)
							+ sizeof(struct vxlanhdr))))) {
		tun_ctx->stats->host_packet_drop++;
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_trace("%px: pskb_may_pull failed for skb:%px\n", dev, skb);
		dev_kfree_skb_any(skb);
		return;
	}

	skb_pull(skb, (l3_hdr_size + sizeof(struct udphdr) + sizeof(struct vxlanhdr)));

	/*
	 * Inner ethernet payload.
	 */
	if (unlikely(!pskb_may_pull(skb, sizeof(struct ethhdr)))) {
		tun_ctx->stats->host_packet_drop++;
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_trace("%px: pskb_may_pull failed for skb:%px\n", dev, skb);
		dev_kfree_skb_any(skb);
		return;
	}

	spin_unlock_bh(&vxlan_ctx.tun_lock);
	skb->dev = dev;
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);
	skb_reset_mac_len(skb);
	dev_queue_xmit(skb);
}

/*
 * nss_vxlanmgr_tunnel_deconfig()
 *	Function to send dynamic interface disable message
 */
int nss_vxlanmgr_tunnel_deconfig(struct net_device *dev)
{
	struct nss_vxlanmgr_tun_ctx *tun_ctx;
	uint32_t inner_ifnum, outer_ifnum;
	struct nss_vxlan_msg vxlanmsg;
	nss_tx_status_t ret;

	dev_hold(dev);

	spin_lock_bh(&vxlan_ctx.tun_lock);
	tun_ctx = nss_vxlanmgr_tunnel_ctx_dev_get(dev);
	if (!tun_ctx) {
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_warn("%px: Invalid tunnel context\n", dev);
		goto done;
	}

	inner_ifnum = tun_ctx->inner_ifnum;
	outer_ifnum = tun_ctx->outer_ifnum;
	spin_unlock_bh(&vxlan_ctx.tun_lock);

	memset(&vxlanmsg, 0, sizeof(struct nss_vxlan_msg));

	ret = nss_vxlanmgr_tunnel_tx_msg_sync(vxlan_ctx.nss_ctx,
						&vxlanmsg,
						inner_ifnum,
						NSS_VXLAN_MSG_TYPE_TUN_DISABLE, 0);
	if (ret != NSS_TX_SUCCESS) {
		nss_vxlanmgr_warn("%px: Sending configuration to inner interface failed: %d\n", dev, ret);
		goto done;
	}

	ret = nss_vxlanmgr_tunnel_tx_msg_sync(vxlan_ctx.nss_ctx,
						&vxlanmsg,
						outer_ifnum,
						NSS_VXLAN_MSG_TYPE_TUN_DISABLE, 0);
	if (ret != NSS_TX_SUCCESS) {
		nss_vxlanmgr_warn("%px: Sending configuration to outer interface failed: %d\n", dev, ret);
	}

done:
	dev_put(dev);
	return NOTIFY_DONE;
}

/*
 * nss_vxlanmgr_tunnel_config()
 *	Function to send dynamic interface enable message
 */
int nss_vxlanmgr_tunnel_config(struct net_device *dev)
{
	uint32_t inner_ifnum, outer_ifnum;
	struct nss_vxlanmgr_tun_ctx *tun_ctx;
	struct nss_vxlan_msg vxlanmsg;
	nss_tx_status_t ret;

	dev_hold(dev);

	spin_lock_bh(&vxlan_ctx.tun_lock);
	tun_ctx = nss_vxlanmgr_tunnel_ctx_dev_get(dev);
	if (!tun_ctx) {
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_warn("%px: Invalid tunnel context\n", dev);
		goto done;
	}

	inner_ifnum = tun_ctx->inner_ifnum;
	outer_ifnum = tun_ctx->outer_ifnum;
	spin_unlock_bh(&vxlan_ctx.tun_lock);

	memset(&vxlanmsg, 0, sizeof(struct nss_vxlan_msg));

	ret = nss_vxlanmgr_tunnel_tx_msg_sync(vxlan_ctx.nss_ctx,
						&vxlanmsg,
						inner_ifnum,
						NSS_VXLAN_MSG_TYPE_TUN_ENABLE, 0);
	if (ret != NSS_TX_SUCCESS) {
		nss_vxlanmgr_warn("%px: Sending configuration to inner interface failed: %d\n", dev, ret);
		goto done;
	}

	ret = nss_vxlanmgr_tunnel_tx_msg_sync(vxlan_ctx.nss_ctx,
						&vxlanmsg,
						outer_ifnum,
						NSS_VXLAN_MSG_TYPE_TUN_ENABLE, 0);
	if (ret != NSS_TX_SUCCESS) {
		nss_vxlanmgr_warn("%px: Sending configuration to outer interface failed: %d\n", dev, ret);
		/*
		 * Disable inner node.
		 */
		nss_vxlanmgr_tunnel_tx_msg_sync(vxlan_ctx.nss_ctx,
						&vxlanmsg,
						inner_ifnum,
						NSS_VXLAN_MSG_TYPE_TUN_DISABLE, 0);
	}

done:
	dev_put(dev);
	return NOTIFY_DONE;
}

/*
 * nss_vxlanmgr_tunnel_destroy()
 *	Function to unregister and destroy dynamic interfaces.
 */
int nss_vxlanmgr_tunnel_destroy(struct net_device *dev)
{
	uint32_t inner_ifnum, outer_ifnum;
	struct nss_vxlanmgr_tun_ctx *tun_ctx;
	struct nss_vxlan_msg vxlanmsg;
	nss_tx_status_t ret;

	dev_hold(dev);

	spin_lock_bh(&vxlan_ctx.tun_lock);
	if (!vxlan_ctx.tun_count) {
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_warn("%px: No more tunnels to destroy.\n", dev);
		goto done;
	}

	tun_ctx = nss_vxlanmgr_tunnel_ctx_dev_get(dev);
	if (!tun_ctx) {
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_warn("%px: Invalid tunnel context\n", dev);
		goto done;
	}

	inner_ifnum = tun_ctx->inner_ifnum;
	outer_ifnum = tun_ctx->outer_ifnum;

	/*
	 * Remove tunnel from global list.
	 */
	list_del(&tun_ctx->head);

	/*
	 * Decrement interface count.
	 */
	vxlan_ctx.tun_count--;
	spin_unlock_bh(&vxlan_ctx.tun_lock);

	nss_vxlanmgr_tun_stats_deinit(tun_ctx);
	nss_vxlanmgr_tun_stats_dentry_remove(tun_ctx);
	kfree(tun_ctx);

	if (!vxlan_ctx.tun_count) {
		/*
		 * Unregister fdb notifier chain if
		 * all vxlan tunnels are destroyed.
		 */
		vxlan_fdb_unregister_notify(&nss_vxlanmgr_tunnel_fdb_notifier);
	}
	nss_vxlanmgr_info("%px: VxLAN interface count is #%d\n", dev, vxlan_ctx.tun_count);

	memset(&vxlanmsg, 0, sizeof(struct nss_vxlan_msg));
	ret = nss_vxlanmgr_tunnel_tx_msg_sync(vxlan_ctx.nss_ctx,
						&vxlanmsg,
						inner_ifnum,
						NSS_VXLAN_MSG_TYPE_TUN_UNCONFIGURE, 0);
	if (ret != NSS_TX_SUCCESS) {
		nss_vxlanmgr_warn("%px: Sending configuration to inner interface failed: %d\n", dev, ret);
	}

	if (!nss_vxlan_unregister_if(inner_ifnum)) {
		nss_vxlanmgr_warn("%px: Inner interface not found\n", dev);
	}
	ret = nss_dynamic_interface_dealloc_node(inner_ifnum,
						NSS_DYNAMIC_INTERFACE_TYPE_VXLAN_INNER);
	if (ret != NSS_TX_SUCCESS) {
		nss_vxlanmgr_warn("%px: Failed to dealloc inner: %d\n", dev, ret);
	}

	ret = nss_vxlanmgr_tunnel_tx_msg_sync(vxlan_ctx.nss_ctx,
						&vxlanmsg,
						outer_ifnum,
						NSS_VXLAN_MSG_TYPE_TUN_UNCONFIGURE, 0);
	if (ret != NSS_TX_SUCCESS) {
		nss_vxlanmgr_warn("%px: Sending configuration to outer interface failed: %d\n", dev, ret);
	}

	if (!nss_vxlan_unregister_if(outer_ifnum)) {
		nss_vxlanmgr_warn("%px: Outer interface not found\n", dev);
	}
	ret = nss_dynamic_interface_dealloc_node(outer_ifnum,
						NSS_DYNAMIC_INTERFACE_TYPE_VXLAN_OUTER);
	if (ret != NSS_TX_SUCCESS) {
		nss_vxlanmgr_warn("%px: Failed to dealloc outer: %d\n", dev, ret);
	}

done:
	dev_put(dev);
	return NOTIFY_DONE;
}

/*
 * nss_vxlanmgr_tunnel_create()
 *	Function to create and register dynamic interfaces.
 */
int nss_vxlanmgr_tunnel_create(struct net_device *dev)
{
	struct vxlan_dev *priv;
	struct nss_vxlan_msg vxlanmsg;
	struct nss_vxlanmgr_tun_ctx *tun_ctx;
	struct nss_vxlan_rule_msg *vxlan_cfg;
	struct nss_ctx_instance *nss_ctx;
	uint32_t inner_ifnum, outer_ifnum;
	nss_tx_status_t ret;

	spin_lock_bh(&vxlan_ctx.tun_lock);
	if (vxlan_ctx.tun_count == NSS_VXLAN_MAX_TUNNELS) {
		spin_unlock_bh(&vxlan_ctx.tun_lock);
		nss_vxlanmgr_warn("%px: Max number of vxlan interfaces supported is %d\n", dev, NSS_VXLAN_MAX_TUNNELS);
		return NOTIFY_DONE;
	}
	spin_unlock_bh(&vxlan_ctx.tun_lock);

	dev_hold(dev);

	tun_ctx = kzalloc(sizeof(struct nss_vxlanmgr_tun_ctx), GFP_ATOMIC);
	if (!tun_ctx) {
		nss_vxlanmgr_warn("Failed to allocate memory for tun_ctx\n");
		goto ctx_alloc_fail;
	}
	tun_ctx->dev = dev;
	tun_ctx->vxlan_ctx = &vxlan_ctx;
	INIT_LIST_HEAD(&tun_ctx->head);

	inner_ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_VXLAN_INNER);
	if (inner_ifnum < 0) {
		nss_vxlanmgr_warn("%px: Inner interface allocation failed.\n", dev);
		goto inner_alloc_fail;
	}
	tun_ctx->inner_ifnum = inner_ifnum;

	outer_ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_VXLAN_OUTER);
	if (outer_ifnum < 0) {
		nss_vxlanmgr_warn("%px: Outer interface allocation failed.\n", dev);
		goto outer_alloc_fail;
	}
	tun_ctx->outer_ifnum = outer_ifnum;

	/*
	 * Register vxlan tunnel with NSS
	 */
	nss_ctx = nss_vxlan_register_if(inner_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_VXLAN_INNER,
					nss_vxlanmgr_tunnel_inner_recv,
					nss_vxlanmgr_tunnel_inner_notifier, dev, 0);
	if (!nss_ctx) {
		nss_vxlanmgr_warn("%px: Failed to register inner iface\n", dev);
		goto inner_reg_fail;
	}

	nss_ctx = nss_vxlan_register_if(outer_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_VXLAN_OUTER,
					nss_vxlanmgr_tunnel_outer_recv,
					nss_vxlanmgr_tunnel_outer_notifier, dev, 0);
	if (!nss_ctx) {
		nss_vxlanmgr_warn("%px: Failed to register outer iface\n", dev);
		goto outer_reg_fail;
	}

	nss_vxlanmgr_trace("%px: Successfully registered inner and outer iface for VxLAN\n", dev);

	memset(&vxlanmsg, 0, sizeof(struct nss_vxlan_msg));
	vxlan_cfg = &vxlanmsg.msg.vxlan_create;

	priv = netdev_priv(dev);
	vxlan_cfg->vni = vxlan_get_vni(priv);
	vxlan_cfg->tunnel_flags = nss_vxlanmgr_tunnel_flags_parse(priv);
	vxlan_cfg->src_port_min = priv->cfg.port_min;
	vxlan_cfg->src_port_max = priv->cfg.port_max;
	vxlan_cfg->dest_port = priv->cfg.dst_port;
	vxlan_cfg->tos = priv->cfg.tos;
	vxlan_cfg->ttl = (priv->cfg.ttl ? priv->cfg.ttl : IPDEFTTL);

	vxlan_cfg->sibling_if_num = outer_ifnum;
	ret = nss_vxlanmgr_tunnel_tx_msg_sync(vxlan_ctx.nss_ctx,
						&vxlanmsg,
						inner_ifnum,
						NSS_VXLAN_MSG_TYPE_TUN_CONFIGURE,
						sizeof(struct nss_vxlan_rule_msg));
	if (ret != NSS_TX_SUCCESS) {
		nss_vxlanmgr_warn("%px: Sending configuration to inner interface failed: %d\n", dev, ret);
		goto config_fail;
	}

	vxlan_cfg->sibling_if_num = inner_ifnum;
	ret = nss_vxlanmgr_tunnel_tx_msg_sync(vxlan_ctx.nss_ctx,
						&vxlanmsg,
						outer_ifnum,
						NSS_VXLAN_MSG_TYPE_TUN_CONFIGURE,
						sizeof(struct nss_vxlan_rule_msg));
	if (ret != NSS_TX_SUCCESS) {
		nss_vxlanmgr_warn("%px: Sending configuration to outer interface failed: %d\n", dev, ret);
		goto config_fail;
	}

	if (!nss_vxlanmgr_tun_stats_dentry_create(tun_ctx)) {
		nss_vxlanmgr_warn("%px: Tun stats dentry init failed\n", vxlan_ctx.nss_ctx);
		goto config_fail;
	}

	if (!nss_vxlanmgr_tun_stats_init(tun_ctx)) {
		nss_vxlanmgr_warn("%px: Tun stats init failed\n", vxlan_ctx.nss_ctx);
		goto config_fail;
	}

	tun_ctx->vni = vxlan_cfg->vni;
	tun_ctx->tunnel_flags = vxlan_cfg->tunnel_flags;
	tun_ctx->flow_label = vxlan_cfg->flow_label;
	tun_ctx->src_port_min = vxlan_cfg->src_port_min;
	tun_ctx->src_port_max = vxlan_cfg->src_port_max;
	tun_ctx->dest_port = vxlan_cfg->dest_port;
	tun_ctx->tos = vxlan_cfg->tos;
	tun_ctx->ttl = vxlan_cfg->ttl;

	spin_lock_bh(&vxlan_ctx.tun_lock);
	/*
	 * Add tunnel to global list.
	 */
	list_add(&tun_ctx->head, &vxlan_ctx.list);

	if (!vxlan_ctx.tun_count) {
		/*
		 * Register with fdb notifier chain
		 * when first tunnel is created.
		 */
		vxlan_fdb_register_notify(&nss_vxlanmgr_tunnel_fdb_notifier);
	}

	/*
	 * Increment vxlan tunnel interface count
	 */
	vxlan_ctx.tun_count++;
	spin_unlock_bh(&vxlan_ctx.tun_lock);
	nss_vxlanmgr_info("%px: VxLAN interface count is #%d\n", dev, vxlan_ctx.tun_count);

	dev_put(dev);
	return NOTIFY_DONE;

config_fail:
	nss_vxlan_unregister_if(outer_ifnum);
outer_reg_fail:
	nss_vxlan_unregister_if(inner_ifnum);
inner_reg_fail:
	ret = nss_dynamic_interface_dealloc_node(outer_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_VXLAN_OUTER);
	if (ret != NSS_TX_SUCCESS)
		nss_vxlanmgr_warn("%px: Outer interface dealloc failed: %d\n", dev, ret);
outer_alloc_fail:
	ret = nss_dynamic_interface_dealloc_node(inner_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_VXLAN_INNER);
	if (ret != NSS_TX_SUCCESS)
		nss_vxlanmgr_warn("%px: Inner interface dealloc failed: %d\n", dev, ret);
inner_alloc_fail:
	kfree(tun_ctx);
ctx_alloc_fail:
	dev_put(dev);
	return NOTIFY_DONE;
}
