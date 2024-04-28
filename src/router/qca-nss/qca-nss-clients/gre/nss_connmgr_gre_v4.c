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
 * nss_connnmgr_gre_v4.c
 *
 *  This file implements client for GRE implementation.
 */

#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/ip_tunnels.h>
#include <net/ip6_tunnel.h>
#include <net/arp.h>
#include <net/gre.h>

#include <nss_api_if.h>
#include "nss_connmgr_gre_public.h"
#include "nss_connmgr_gre.h"

/*
 * nss_connmgr_gre_v4_get_tx_dev()
 *	Find tx interface for IP address. Holds ref to next_dev.
 */
static struct net_device *nss_connmgr_gre_v4_get_tx_dev(uint32_t dest_ip)
{
	struct rtable *rt;
	struct net_device *dev;
	uint32_t ip_addr __attribute__ ((unused)) = ntohl(dest_ip);

	rt = ip_route_output(&init_net, htonl(dest_ip), 0, 0, 0);
	if (IS_ERR(rt)) {
		nss_connmgr_gre_warning("Unable to lookup route for %pI4\n", &ip_addr);
		return NULL;
	}

	dev = rt->dst.dev;
	if (!dev) {
		ip_rt_put(rt);
		nss_connmgr_gre_warning("Unable to find route dev for %pI4\n", &ip_addr);
		return NULL;
	}

	dev_hold(dev);
	ip_rt_put(rt);
	return dev;
}

/*
 * nss_connmgr_gre_v4_get_mac_address()
 *	Find source and destination MAC for source and destination IP address.
 */
static int nss_connmgr_gre_v4_get_mac_address(uint32_t src_ip, uint32_t dest_ip,
					     uint8_t *src_mac, uint8_t *dest_mac)
{
	struct neighbour *neigh;
	struct rtable *rt;
	__be32 raddr = htonl(dest_ip);
	__be32 laddr = htonl(src_ip);

	/*
	 * find local MAC address
	 */
	struct net_device *local_dev = ip_dev_find(&init_net, laddr);
	if (!local_dev) {
		nss_connmgr_gre_warning("Unable to find local dev for %pI4", &laddr);
		return GRE_ERR_NO_LOCAL_NETDEV;
	}
	ether_addr_copy(src_mac, local_dev->dev_addr);
	dev_put(local_dev);
	nss_connmgr_gre_info("Src MAC address for %pI4 is %pM\n", &laddr, src_mac);

	rt = ip_route_output(&init_net, raddr, 0, 0, 0);
	if (IS_ERR(rt)) {
		nss_connmgr_gre_warning("route look up failed for %pI4\n", &raddr);
		return GRE_ERR_RADDR_ROUTE_LOOKUP;
	}

	rcu_read_lock();

	neigh = dst_neigh_lookup(&rt->dst, (const void *)&raddr);
	if (!neigh) {
		neigh = neigh_lookup(&arp_tbl, (const void *)&raddr,  rt->dst.dev);
	}

	if (neigh && !is_valid_ether_addr(neigh->ha)) {
		neigh_release(neigh);
		neigh = NULL;
	}

	/*
	 * Send arp request
	 */
	if (!neigh) {
		neigh = neigh_create(&arp_tbl, &raddr, rt->dst.dev);
		if (IS_ERR_OR_NULL(neigh)) {
			nss_connmgr_gre_warning("Unable to create ARP request neigh for %pI4\n", &raddr);
			rcu_read_unlock();
			ip_rt_put(rt);
			return GRE_ERR_NEIGH_CREATE;
		}
		nss_connmgr_gre_info("Send ARP request neigh for %pI4\n", &raddr);
		neigh_event_send(neigh, NULL);

		msleep(2000);
	}

	if (neigh->dev->type == ARPHRD_LOOPBACK) {
		rcu_read_unlock();
		ip_rt_put(rt);
		neigh_release(neigh);
		nss_connmgr_gre_warning("Err in destination MAC address, neighbour dev is loop back for %pI4\n", &raddr);
		return GRE_ERR_NEIGH_DEV_LOOPBACK;

	}

	if (neigh->dev->flags & IFF_NOARP) {
		rcu_read_unlock();
		ip_rt_put(rt);
		neigh_release(neigh);
		nss_connmgr_gre_warning("Err in destination MAC address, neighbour dev is of type NO_ARP for %pI4\n", &raddr);
		return GRE_ERR_NEIGH_DEV_NOARP;
	}

	ether_addr_copy(dest_mac, neigh->ha);
	rcu_read_unlock();
	ip_rt_put(rt);
	neigh_release(neigh);
	nss_connmgr_gre_info("Destination MAC address for %pI4 is %pM\n", &raddr, dest_mac);
	return GRE_SUCCESS;
}

/*
 * nss_connmgr_gre_v4_set_config()
 *	Set User configuration to netdevice
 */
int nss_connmgr_gre_v4_set_config(struct net_device *dev, struct nss_connmgr_gre_cfg *cfg)
{
	nss_connmgr_gre_priv_t *priv = netdev_priv(dev);
	struct ip_tunnel *t = (struct ip_tunnel *)priv;
	struct iphdr *iphdr;

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

	/*
	 * IP address validate
	 */
	if ((cfg->src_ip == 0) || (cfg->dest_ip == 0)) {
		nss_connmgr_gre_warning("Source ip/Destination IP is invalid");
		return GRE_ERR_INVALID_IP;
	}

	memset(t, 0, sizeof(struct ip_tunnel));

	priv->pad_len =  (cfg->add_padding) ? GRE_HDR_PAD_LEN : 0;
	priv->gre_hlen = nss_connmgr_gre_get_hlen(cfg);

	iphdr = (struct iphdr *)&(t->parms.iph);
	iphdr->protocol = IPPROTO_GRE;

	memcpy(&iphdr->saddr, (uint8_t *)cfg->src_ip, 4);
	memcpy(&iphdr->daddr, (uint8_t *)cfg->dest_ip, 4);

	iphdr->saddr = htonl(iphdr->saddr);
	iphdr->daddr = htonl(iphdr->daddr);

	iphdr->tos = cfg->tos << 2;
	if (cfg->tos_inherit) {
		iphdr->tos |= 0x1;
	}

	iphdr->ttl = cfg->ttl;
	if (cfg->ttl_inherit) {
		iphdr->ttl = 0;
	}

	if (cfg->set_df) {
		iphdr->frag_off = htons(IP_DF);
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
 * nss_connmgr_gre_tap_v4_outer_exception()
 * 	Handle IPv4 exception for GRETAP outer device
 */
void nss_connmgr_gre_tap_v4_outer_exception(struct net_device *dev, struct sk_buff *skb)
{
	struct ethhdr *eth_hdr = (struct ethhdr *)skb->data;

	/*
	 * GRE encapsulated packet exceptioned, remove the encapsulation
	 * and transmit on GRE interface.
	 */
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct ethhdr) + sizeof(struct iphdr)
				+ sizeof(struct gre_base_hdr))))) {
		nss_connmgr_gre_warning("%p: pskb_may_pull failed for skb:%p\n", dev, skb);
		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * TODO: Support parsing GRE options
	 */
	skb_pull(skb, (sizeof(struct ethhdr) + sizeof(struct iphdr)
				+ sizeof(struct gre_base_hdr)));

	if (unlikely(!pskb_may_pull(skb, sizeof(struct ethhdr)))) {
		nss_connmgr_gre_warning("%p: pskb_may_pull failed for skb:%p\n", dev, skb);
		dev_kfree_skb_any(skb);
		return;
	}
	skb->dev = dev;
	if (likely(ntohs(eth_hdr->h_proto) >= ETH_P_802_3_MIN)) {
		skb->protocol = eth_hdr->h_proto;
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
 * nss_connmgr_gre_tun_v4_outer_exception()
 * 	Handle IPv4 exception for GRETUN outer device
 */
void nss_connmgr_gre_tun_v4_outer_exception(struct net_device *dev, struct sk_buff *skb)
{
	struct iphdr *iph;

	/*
	 * GRE encapsulated packet exceptioned, remove the encapsulation
	 * and transmit on GRE interface.
	 */
	if (unlikely(!pskb_may_pull(skb, sizeof(struct iphdr) + sizeof(struct gre_base_hdr)))) {
		nss_connmgr_gre_warning("%p: pskb_may_pull failed for skb:%p\n", dev, skb);
		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * TODO: Support parsing GRE options
	 */
	skb_pull(skb, sizeof(struct iphdr) + sizeof(struct gre_base_hdr));
	iph = (struct iphdr *)skb->data;
	skb->dev = dev;

	switch (iph->version) {
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
 * nss_connmgr_gre_v4_get_config()
 *	Fill in config message to send to NSS.
 */
int nss_connmgr_gre_v4_get_config(struct net_device *dev, struct nss_gre_msg *req,
				  struct net_device **next_dev, bool hold)
{
	uint32_t src_ip, dest_ip;
	struct ip_tunnel *t = netdev_priv(dev);
	struct iphdr *iphdr = (struct iphdr *)&(t->parms.iph);
	struct net_device *out_dev;
	struct nss_gre_config_msg *cmsg = &req->msg.cmsg;
	int ret;

	src_ip = ntohl(iphdr->saddr);
	dest_ip = ntohl(iphdr->daddr);
	memcpy(cmsg->src_ip, &src_ip, 4);
	memcpy(cmsg->dest_ip, &dest_ip, 4);

	cmsg->flags |= nss_connmgr_gre_get_nss_config_flags(t->parms.o_flags,
								     t->parms.i_flags,
								     iphdr->tos, iphdr->ttl,
								     iphdr->frag_off);

	cmsg->ikey = t->parms.i_key;
	cmsg->okey = t->parms.o_key;
	cmsg->ttl = iphdr->ttl;
	cmsg->tos = iphdr->tos >> 2;

	/*
	 * fill in MAC addresses
	 */
	ret = nss_connmgr_gre_v4_get_mac_address(src_ip, dest_ip,
						 (uint8_t *)cmsg->src_mac,
						 (uint8_t *)cmsg->dest_mac);
	if (!ret) {
		cmsg->flags |= NSS_GRE_CONFIG_SET_MAC;
	}

	/*
	 * fill in NSS interface number
	 */
	out_dev = nss_connmgr_gre_v4_get_tx_dev(dest_ip);
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
