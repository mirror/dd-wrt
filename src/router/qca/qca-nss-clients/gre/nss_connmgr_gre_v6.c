/*
 **************************************************************************
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * nss_connnmgr_gre_v6.c
 *
 *  This file implements client for GRE implementation.
 */

#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <net/route.h>
#include <net/ip6_route.h>
#include <net/ip6_tunnel.h>
#include <net/ip_tunnels.h>
#include <net/addrconf.h>
#include <net/gre.h>

#include <nss_api_if.h>
#include "nss_connmgr_gre_public.h"
#include "nss_connmgr_gre.h"

/*
 * nss_connmgr_gre_v6_get_tx_dev()
 *	Find tx interface for the IP address.
 */
static struct net_device *nss_connmgr_gre_v6_get_tx_dev(uint8_t *dest_ip)
{
	struct rt6_info *rt;
	struct in6_addr ipv6_addr;
	struct net_device *dev;

	memcpy(ipv6_addr.s6_addr, dest_ip, 16);
	rt = rt6_lookup(&init_net, &ipv6_addr, NULL, 0, NULL, 0);
	if (!rt) {
		return NULL;
	}

	dev = rt->dst.dev;
	if (!dev) {
		ip6_rt_put(rt);
		return NULL;
	}

	dev_hold(dev);
	ip6_rt_put(rt);
	return dev;
}

/*
 * nss_connmgr_gre_v6_get_mac_address()
 *	Find source and destination MAC address for source and destination IP
 *	address.
 */
static int nss_connmgr_gre_v6_get_mac_address(uint8_t *src_ip, uint8_t *dest_ip,
					     uint8_t *src_mac, uint8_t *dest_mac)
{
	struct neighbour *neigh;
	struct rt6_info *rt;
	struct dst_entry *dst;
	struct in6_addr ipv6_addr;
	struct net_device *local_dev;

	/*
	 * Find src MAC address
	 */
	memcpy(ipv6_addr.s6_addr, src_ip, 16);
	local_dev = (struct net_device *)ipv6_dev_find(&init_net, &ipv6_addr, 1);
	if (!local_dev) {
		nss_connmgr_gre_warning("Unable to find local dev for %pI6", src_ip);
		return GRE_ERR_NO_LOCAL_NETDEV;
	}
	ether_addr_copy(src_mac, local_dev->dev_addr);
	dev_put(local_dev);

	/*
	 * Find dest MAC address
	 */
	memcpy(ipv6_addr.s6_addr, dest_ip, 16);
	rt = rt6_lookup(&init_net, &ipv6_addr, NULL, 0, NULL, 0);
	if (!rt) {
		return GRE_ERR_NEIGH_LOOKUP;
	}
	dst = (struct dst_entry *)rt;
	neigh = dst_neigh_lookup(dst, &ipv6_addr);

	if (!neigh) {
		neigh = neigh_lookup(&nd_tbl, (const void *)&ipv6_addr,  rt->dst.dev);
	}

	if (neigh && !is_valid_ether_addr(neigh->ha)) {
		neigh_release(neigh);
		neigh = NULL;
	}

	if (!neigh) {
		ip6_rt_put(rt);
		nss_connmgr_gre_warning("Err in MAC address, neighbour look up failed\n");
		return GRE_ERR_NEIGH_LOOKUP;
	}

	ether_addr_copy(dest_mac, neigh->ha);

	ip6_rt_put(rt);
	neigh_release(neigh);
	return GRE_SUCCESS;
}

/*
 * nss_connmgr_gre_tap_v6_outer_exception()
 * 	Handle IPv6 exception for GRETAP outer device
 */
void nss_connmgr_gre_tap_v6_outer_exception(struct net_device *dev, struct sk_buff *skb)
{
	struct ethhdr *eth = (struct ethhdr *)skb->data;

	/*
	 * GRE encapsulated packet exceptioned, remove the encapsulation
	 * and transmit on GRE interface.
	 */
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct ethhdr) + sizeof(struct ipv6hdr)
				+ sizeof(struct gre_base_hdr))))) {
		nss_connmgr_gre_warning("%p: pskb_may_pull failed for skb:%p\n", dev, skb);
		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * TODO: Support parsing GRE options
	 */
	skb_pull(skb, (sizeof(struct ethhdr) + sizeof(struct ipv6hdr)
				+ sizeof(struct gre_base_hdr)));

	if (unlikely(!pskb_may_pull(skb, sizeof(struct ethhdr)))) {
		nss_connmgr_gre_warning("%p: pskb_may_pull failed for skb:%p\n", dev, skb);
		dev_kfree_skb_any(skb);
		return;
	}
	skb->dev = dev;
	if (likely(ntohs(eth->h_proto) >= ETH_P_802_3_MIN)) {
		skb->protocol = eth->h_proto;
	} else {
		skb->protocol = htons(ETH_P_802_2);
	}
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);
	skb_reset_mac_len(skb);
	dev_queue_xmit(skb);
}

/*
 * nss_connmgr_gre_tun_v6_outer_exception()
 * 	Handle IPv6 exception for GRETUN outer device
 */
void nss_connmgr_gre_tun_v6_outer_exception(struct net_device *dev, struct sk_buff *skb)
{
	struct ipv6hdr *ip6h;

	/*
	 * GRE encapsulated packet exceptioned, remove the encapsulation
	 * and transmit on GRE interface.
	 */
	if (unlikely(!pskb_may_pull(skb, sizeof(struct ipv6hdr) + sizeof(struct gre_base_hdr)))) {
		nss_connmgr_gre_warning("%p: pskb_may_pull failed for skb:%p\n", dev, skb);
		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * TODO: Support parsing GRE options
	 */
	skb_pull(skb, sizeof(struct ipv6hdr) + sizeof(struct gre_base_hdr));

	ip6h = (struct ipv6hdr *)skb->data;
	skb->dev = dev;

	switch (ip6h->version) {
	case 4:
		skb->protocol = htons(ETH_P_IP);
		break;
	case 6:
		skb->protocol = htons(ETH_P_IPV6);
		break;
	default:
		nss_connmgr_gre_info("%p: wrong IP version in GRE encapped packet. skb: %p\n", dev, skb);
		dev_kfree_skb_any(skb);
		return;
	}

	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);
	skb_reset_mac_len(skb);
	dev_queue_xmit(skb);
}

/*
 * nss_connmgr_gre_v6_set_config()
 *	Set user config to dev inerface.
 */
int nss_connmgr_gre_v6_set_config(struct net_device *dev, struct nss_connmgr_gre_cfg *cfg)
{
	nss_connmgr_gre_priv_t *priv = netdev_priv(dev);
	struct ip6_tnl *t = (struct ip6_tnl *)priv;

	/*
	 * IP address validate
	 */
	if (!ipv6_addr_any(((const struct in6_addr *)&cfg->src_ip)) ||
	    !ipv6_addr_any(((const struct in6_addr *)&cfg->dest_ip))) {
		nss_connmgr_gre_warning("Source ip/Destination IP is invalid");
		return  GRE_ERR_INVALID_IP;
	}

	/*
	 * MAC address validate
	 */
	if (cfg->use_mac_hdr) {
		if (!is_valid_ether_addr((const u8 *)cfg->src_mac) ||
		    !is_valid_ether_addr((const u8 *)cfg->dest_mac)) {
			nss_connmgr_gre_warning("User should provide valid MAC address if flag add_mac hdr is set\n");
			return GRE_ERR_INVALID_MAC;
		}
	}

	memset(t, 0, sizeof(struct ip6_tnl));

	priv->pad_len = (cfg->add_padding) ? GRE_HDR_PAD_LEN : 0;
	priv->gre_hlen = nss_connmgr_gre_get_hlen(cfg);

	memcpy(t->parms.laddr.s6_addr, &cfg->src_ip, 16);
	memcpy(t->parms.raddr.s6_addr, &cfg->dest_ip, 16);

	t->parms.flowinfo = 0;

	t->parms.hop_limit = cfg->ttl;
	if (cfg->ttl_inherit) {
		t->parms.hop_limit = 0;
	}

	if (cfg->ikey_valid) {
		t->parms.i_key = cfg->ikey;
	}

	if (cfg->okey_valid) {
		t->parms.o_key = cfg->okey;
	}

	nss_connmgr_gre_set_gre_flags(cfg, &t->parms.o_flags, &t->parms.i_flags);

	strlcpy(t->parms.name, dev->name, IFNAMSIZ);
	t->dev = dev;
	return GRE_SUCCESS;
}

/*
 * nss_connmgr_gre_v6_get_config()
 *	Fill info in config message to send to NSS.
 */
int nss_connmgr_gre_v6_get_config(struct net_device *dev, struct nss_gre_msg *req,
				  struct net_device **next_dev, bool hold)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct net_device *out_dev;
	struct nss_gre_config_msg *cmsg = &req->msg.cmsg;
	int ret;

	memcpy(cmsg->src_ip, t->parms.laddr.s6_addr, 16);
	memcpy(cmsg->dest_ip, t->parms.raddr.s6_addr, 16);

	/*
	 * IPv6 outer tos field is always inherited from inner IP header.
	 */
	cmsg->flags |= nss_connmgr_gre_get_nss_config_flags(t->parms.o_flags,
								     t->parms.i_flags,
								     0x1,
								     t->parms.hop_limit, 0);

	cmsg->ikey = t->parms.i_key;
	cmsg->okey = t->parms.o_key;
	cmsg->ttl = t->parms.hop_limit;
	cmsg->tos = t->parms.flowinfo;

	/*
	 * fill in MAC addresses
	 */
	ret = nss_connmgr_gre_v6_get_mac_address(t->parms.laddr.s6_addr, t->parms.raddr.s6_addr,
						 (uint8_t *)cmsg->src_mac,
						 (uint8_t *)cmsg->dest_mac);
	if (!ret) {
		cmsg->flags |= NSS_GRE_CONFIG_SET_MAC;
	}

	/*
	 * fill in NSS interface number
	 */
	out_dev = nss_connmgr_gre_v6_get_tx_dev(t->parms.raddr.s6_addr);
	if (out_dev) {
		cmsg->next_node_if_num = nss_cmn_get_interface_number_by_dev(out_dev);
		cmsg->flags |= NSS_GRE_CONFIG_NEXT_NODE_AVAILABLE;
		*next_dev = out_dev;
		if (!hold) {
			dev_put(out_dev);
		}
	}

	return GRE_SUCCESS;
}
