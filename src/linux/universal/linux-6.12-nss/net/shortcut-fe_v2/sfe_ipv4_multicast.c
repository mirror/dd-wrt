/*
 * sfe_ipv4_multicast.c
 *	Shortcut forwarding engine file for IPv4 multicast
 *
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/skbuff.h>
#include <net/udp.h>
#include <net/protocol.h>
#include <linux/etherdevice.h>
#include <linux/lockdep.h>
#include <linux/ip.h>

#include "sfe_debug.h"
#include "sfe_api.h"
#include "sfe.h"
#include "sfe_flow_cookie.h"
#include "sfe_ipv4.h"
#include "sfe_pppoe.h"
#include "sfe_vlan.h"


/*
 * sfe_ipv4_forward_multicast()
 *	Send the packet to an interface.
 */
int sfe_ipv4_forward_multicast(struct sfe_ipv4 *si, struct sk_buff *skb, unsigned int len,struct iphdr *iph,
		struct udphdr *udph, struct sfe_ipv4_mc_dest *mc_xmit_dev, struct sfe_l2_info *l2_info, bool tun_outer)

{
	struct net_device *xmit_dev;
	bool hw_csum;
	bool bridge_flow;
	bool fast_xmit;
	netdev_features_t features;

	bridge_flow = !!(mc_xmit_dev->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_BRIDGE_FLOW);

	/*
	 * For PPPoE flows, add PPPoE header before L2 header is added.
	 */
	if (unlikely(mc_xmit_dev->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PPPOE_ENCAP)) {
		DEBUG_TRACE("%px: PPPoE added session id:%u \n", skb, mc_xmit_dev->pppoe_session_id);
		sfe_pppoe_add_header(skb, mc_xmit_dev->pppoe_session_id, PPP_IP);
		this_cpu_inc(si->stats_pcpu->pppoe_encap_packets_forwarded64);
	}

	/*
	 * Enable HW csum if rx checksum is verified and xmit interface is CSUM offload capable.
	 * Note: If L4 csum at Rx was found to be incorrect, we (router) should use incremental L4 checksum here
	 * so that HW does not re-calculate/replace the L4 csum
	 */
	hw_csum = !!(mc_xmit_dev->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_CSUM_OFFLOAD) && (skb->ip_summed == CHECKSUM_UNNECESSARY);

	/*
	 * Do we have to perform translations of the source address/port?
	 */
	if (unlikely(mc_xmit_dev->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC)) {
		u16 udp_csum;

		iph->saddr = mc_xmit_dev->xlate_src_ip;
		udph->source = mc_xmit_dev->xlate_src_ident;
		DEBUG_TRACE("%px: SRC XLAT :%pI4 ident:%u\n", skb, &mc_xmit_dev->xlate_src_ip,	mc_xmit_dev->xlate_src_ident);
		/*
		 * Do we have a non-zero UDP checksum?  If we do then we need
		 * to update it.
		 */
		if (unlikely(!hw_csum)) {
			udp_csum = udph->check;
			if (likely(udp_csum)) {
				u32 sum;

				if (unlikely(skb->ip_summed == CHECKSUM_PARTIAL)) {
					sum = udp_csum + mc_xmit_dev->xlate_src_partial_csum_adjustment;
				} else {
					sum = udp_csum + mc_xmit_dev->xlate_src_csum_adjustment;
				}

				sum = (sum & 0xffff) + (sum >> 16);
				udph->check = (u16)sum;
			}
		}
	}

	/*
	 * Decrement our TTL
	 * Except when called from hook function in post-decap.
	 */
	if (likely(!bridge_flow)) {
		iph->ttl -= (u8)(!tun_outer);
	}

	/*
	 * If HW checksum offload is not possible, full L3 checksum and incremental L4 checksum
	 * are used to update the packet. Setting ip_summed to CHECKSUM_UNNECESSARY ensures checksum is
	 * not recalculated further in packet path.
	 */
	if (likely(hw_csum)) {
		skb->ip_summed = CHECKSUM_PARTIAL;
	} else {
		iph->check = sfe_ipv4_gen_ip_csum(iph);
	}

	xmit_dev = mc_xmit_dev->xmit_dev;
	skb->dev = xmit_dev;

	/*
	 * Check to see if we need to add VLAN tags
	 */
	if (unlikely(mc_xmit_dev->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_INSERT_EGRESS_VLAN_TAG)) {
		DEBUG_TRACE("%px: Vlan tag added:\n", skb);
		sfe_vlan_add_tag(skb, mc_xmit_dev->egress_vlan_hdr_cnt,	mc_xmit_dev->egress_vlan_hdr);
	}

	/*
	 * Check to see if we need to write an Ethernet header.
	 */
	if (likely(mc_xmit_dev->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_L2_HDR)) {
		if (unlikely(!(mc_xmit_dev->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR))) {
			DEBUG_TRACE("%px: fast write l2 hdr:dest %pM: src %pM\n", skb, mc_xmit_dev->xmit_dest_mac, mc_xmit_dev->xmit_src_mac);
			dev_hard_header(skb, xmit_dev, ntohs(skb->protocol),
					mc_xmit_dev->xmit_dest_mac, mc_xmit_dev->xmit_src_mac, len);
		} else {
			/*
			 * For the simple case we write this really fast.
			 */
			struct ethhdr *eth = (struct ethhdr *)__skb_push(skb, ETH_HLEN);
			DEBUG_TRACE("%px: direct write l2 hdr:dest %pM: src %pM\n", skb, mc_xmit_dev->xmit_dest_mac, mc_xmit_dev->xmit_src_mac);
			eth->h_proto = skb->protocol;
			ether_addr_copy((u8 *)eth->h_dest, (u8 *)mc_xmit_dev->xmit_dest_mac);
			ether_addr_copy((u8 *)eth->h_source, (u8 *)mc_xmit_dev->xmit_src_mac);
		}
	}

	/*
	 * For the first packets, check if it could go fast xmit.
	 */
	if (unlikely(!(mc_xmit_dev->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT_FLOW_CHECKED)
				&& (mc_xmit_dev->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT_DEV_ADMISSION))){
		mc_xmit_dev->features = netif_skb_features(skb);
		if (likely(sfe_fast_xmit_check(skb, mc_xmit_dev->features))) {
			mc_xmit_dev->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT;
		}
		mc_xmit_dev->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT_FLOW_CHECKED;
	}
	features = mc_xmit_dev->features;

	fast_xmit = !!(mc_xmit_dev->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT);

	this_cpu_inc(si->stats_pcpu->packets_forwarded64);

	/*
	 * We do per packet condition check before we could fast xmit the
	 * packet.
	 */
	if (likely(fast_xmit && dev_fast_xmit(skb, xmit_dev, features))) {
		this_cpu_inc(si->stats_pcpu->packets_fast_xmited64);
		return 1;
	}

	/*
	 * We're going to check for GSO flags when we transmit the packet so
	 * start fetching the necessary cache line now.
	 */
	prefetch(skb_shinfo(skb));

	/*
	 * Mark that this packet has been fast forwarded.
	 */
	skb->fast_forwarded = 1;

	/*
	 * Send the packet on its way.
	 */
	dev_queue_xmit(skb);

	return 1;

}

/*
 * sfe_ipv4_recv_multicast()
 *	Multicast packet receive and forwarding.
 * RCU lock held when calling this function.
 */
int sfe_ipv4_recv_multicast(struct sfe_ipv4 *si, struct sk_buff *skb,
		unsigned int ihl, unsigned int len, struct sfe_ipv4_connection_match *cm,
		struct sfe_l2_info *l2_info, bool tun_outer)

{
	u32 service_class_id;
	struct iphdr *iph;
	struct udphdr *udph;
	struct sfe_ipv4_mc_dest *mc_xmit_dev;

	/*
	 * Update traffic stats.
	 */
	atomic_inc(&cm->rx_packet_count);
	atomic_add(len, &cm->rx_byte_count);

	/*
	 * Temporary WAR to avoid SKB leak where mc_list is empty here.
	 */
	if (list_empty(&cm->mc_list)) {
		return 0;
	}

	/*
	 * Update DSCP
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK)) {
		iph = (struct iphdr *)skb->data;
		iph->tos = (iph->tos & SFE_IPV4_DSCP_MASK) | cm->dscp;
	}

	/*
	 * Update priority of skb.
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
		skb->priority = cm->priority;
	}

	/*
	 * Set SKB packet type to PACKET_HOST
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PACKET_HOST)) {
		skb->pkt_type = PACKET_HOST;
	}

	/*
	 * Mark outgoing packet.
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_MARK)) {
		skb->mark = cm->mark;
		/*
		 * Update service class stats if SAWF is valid.
		 */
		if (likely(cm->sawf_valid)) {
			service_class_id = SFE_GET_SAWF_SERVICE_CLASS(cm->mark);
			sfe_ipv4_service_class_stats_inc(si, service_class_id, len);
		}
	}

	/*
	 * Walk through the mc_list and send the skb to each one.
	 */
	list_for_each_entry_rcu(mc_xmit_dev, &cm->mc_list, list) {
		struct sk_buff *nskb;
		bool bridge_flow;

		bridge_flow = !!(mc_xmit_dev->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_BRIDGE_FLOW);

		/*
		 * If we are going to change the packet content, we need copy
		 * instead of clone.
		 * The last interface could reuse the original skb.When we
		 * create the mc_xmit_dev list, we make the interface to be modified on in the
		 * head of the list.
		 */
		if (mc_xmit_dev->list.next == &cm->mc_list) {
			nskb = skb;
		} else if ((!bridge_flow && !tun_outer) || (mc_xmit_dev->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_MULTICAST_CHANGED)) {
			nskb = skb_copy(skb, GFP_ATOMIC);
		} else {
			nskb = skb_clone(skb, GFP_ATOMIC);
		}

		if (!nskb) {
			/*
			 * When allocate failed, we need ignore this error and
			 * continue, otherwise, the packets could be forwarded
			 * twice for on-the-going packet.
			 */
			DEBUG_TRACE("%px: Can't allocate the new skb for mc interface:%s\n",
					skb, mc_xmit_dev->xmit_dev->name);
			continue;

		}

		/*
		 * Reset the iph and udp header
		 */
		iph = (struct iphdr *)nskb->data;
		udph = (struct udphdr *)(nskb->data + ihl);
		/*
		 * Always return 1
		 */
		sfe_ipv4_forward_multicast(si, nskb, len, iph, udph, mc_xmit_dev, l2_info,tun_outer);
	}

	return 1;
}
