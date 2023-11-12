/*
 * sfe_ipv6_gre.c
 *	Shortcut forwarding engine file for IPv6 GRE
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
#include <net/gre.h>
#include <net/protocol.h>
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <net/ip6_checksum.h>

#include "sfe_debug.h"
#include "sfe_api.h"
#include "sfe.h"
#include "sfe_flow_cookie.h"
#include "sfe_ipv6.h"
#include "sfe_pppoe.h"
#include "sfe_vlan.h"
#include "sfe_trustsec.h"

/*
 * sfe_ipv6_recv_gre()
 *	Handle GRE packet receives and forwarding.
 */
int sfe_ipv6_recv_gre(struct sfe_ipv6 *si, struct sk_buff *skb, struct net_device *dev,
		      unsigned int len, struct ipv6hdr *iph, unsigned int ihl, bool sync_on_find,
		      struct sfe_l2_info *l2_info, bool tun_outer)
{
	struct sfe_ipv6_connection_match *cm;
	struct sfe_ipv6_addr *dest_ip;
	struct sfe_ipv6_addr *src_ip;
	struct net_device *xmit_dev;
	bool bridge_flow;
	bool passthrough;
	bool ret;

	/*
	 * Is our packet too short to contain a valid UDP header?
	 */
	if (!pskb_may_pull(skb, (sizeof(struct gre_base_hdr) + ihl))) {

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_GRE_HEADER_INCOMPLETE);
		DEBUG_TRACE("packet too short for GRE header\n");
		return 0;
	}

	/*
	 * Read the IP address and port information.  Read the IP header data first
	 * because we've almost certainly got that in the cache.  We may not yet have
	 * the UDP header cached though so allow more time for any prefetching.
	 */
	src_ip = (struct sfe_ipv6_addr *)iph->saddr.s6_addr32;
	dest_ip = (struct sfe_ipv6_addr *)iph->daddr.s6_addr32;

	rcu_read_lock();

	/*
	 * Look for a connection match.
	 */
#ifdef CONFIG_NF_FLOW_COOKIE
	cm = si->sfe_flow_cookie_table[skb->flow_cookie & SFE_FLOW_COOKIE_MASK].match;
	if (unlikely(!cm)) {
		cm = sfe_ipv6_find_connection_match_rcu(si, dev, IPPROTO_GRE, src_ip, 0, dest_ip, 0);
	}
#else
	cm = sfe_ipv6_find_connection_match_rcu(si, dev, IPPROTO_GRE, src_ip, 0, dest_ip, 0);
#endif
	if (unlikely(!cm)) {
		rcu_read_unlock();
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_GRE_NO_CONNECTION);
		DEBUG_TRACE("no connection match found dev %s src ip %pI6 dest ip %pI6\n", dev->name, src_ip, dest_ip);
		return 0;
	}

	/*
	 * Do we expect an ingress VLAN tag for this flow?
	 */
	if (unlikely(!sfe_vlan_validate_ingress_tag(skb, cm->ingress_vlan_hdr_cnt, cm->ingress_vlan_hdr, l2_info, 0))) {
		rcu_read_unlock();
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_INGRESS_VLAN_TAG_MISMATCH);
		DEBUG_TRACE("VLAN tag mismatch. skb=%px\n", skb);
		return 0;
	}

	/*
	 * Do we expect a trustsec header for this flow ?
	 */
	if (unlikely(!sfe_trustsec_validate_ingress_sgt(skb, cm->ingress_trustsec_valid, &cm->ingress_trustsec_hdr, l2_info))) {
		rcu_read_unlock();
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_INGRESS_TRUSTSEC_SGT_MISMATCH);
		DEBUG_TRACE("Trustsec SGT mismatch. skb=%px\n", skb);
		return 0;
	}

	/*
	 * Source interface validate.
	 */
	if (unlikely((cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_SRC_INTERFACE_CHECK) && (cm->match_dev != dev))) {
		if (!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_SRC_INTERFACE_CHECK_NO_FLUSH)) {
			struct sfe_ipv6_connection *c = cm->connection;
			int ret;
			DEBUG_TRACE("flush on source interface check failure\n");
			spin_lock_bh(&si->lock);
			ret = sfe_ipv6_remove_connection(si, c);
			spin_unlock_bh(&si->lock);

			if (ret) {
				sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
			}
		}
		rcu_read_unlock();
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_INVALID_SRC_IFACE);
		DEBUG_TRACE("exception the packet on source interface check failure\n");
		return 0;
	}

	passthrough = cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PASSTHROUGH;

	/*
	 * If our packet has beern marked as "sync on find" we can't actually
	 * forward it in the fast path, but now that we've found an associated
	 * connection we need sync its status before exception it to slow path. unless
	 * it is passthrough packet.
	 * TODO: revisit to ensure that pass through traffic is not bypassing firewall for fragmented cases
	 */
	if (unlikely(sync_on_find) && !passthrough) {
		sfe_ipv6_sync_status(si, cm->connection, SFE_SYNC_REASON_STATS);
		rcu_read_unlock();

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_GRE_IP_OPTIONS_OR_INITIAL_FRAGMENT);
		DEBUG_TRACE("Sync on find\n");
		return 0;
	}

	bridge_flow = !!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_BRIDGE_FLOW);

	/*
	 * Does our hop_limit allow forwarding?
	 */
	if (!bridge_flow && (iph->hop_limit < 2) && passthrough) {
		sfe_ipv6_sync_status(si, cm->connection, SFE_SYNC_REASON_STATS);
		rcu_read_unlock();

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_GRE_SMALL_TTL);
		DEBUG_TRACE("hop_limit too low\n");
		return 0;
	}

	/*
	 * Check if skb was cloned. If it was, unclone it. Because
	 * the data area is going to be written in this path and we don't want to
	 * change the cloned skb's data section.
	 */
	if (unlikely(skb_cloned(skb))) {
		DEBUG_TRACE("%px: skb is a cloned skb\n", skb);

		if (unlikely(skb_shared(skb)) || unlikely(skb_unclone(skb, GFP_ATOMIC))) {
			rcu_read_unlock();
			DEBUG_WARN("Failed to unclone the cloned skb\n");
			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_UNCLONE_FAILED);
			return 0;
		}

		/*
		 * Update the iph and udph pointers with the uncloned skb's data area.
		 */
		iph = (struct ipv6hdr *)skb->data;
	}

	/*
	 * For PPPoE packets, match server MAC and session id
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PPPOE_DECAP)) {
		struct ethhdr *eth;
		bool pppoe_match;

		if (unlikely(!sfe_l2_parse_flag_check(l2_info, SFE_L2_PARSE_FLAGS_PPPOE_INGRESS))) {
			rcu_read_unlock();
			DEBUG_TRACE("%px: PPPoE header not present in packet for PPPoE rule\n", skb);
			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_INCORRECT_PPPOE_PARSING);
			return 0;
		}

		eth = eth_hdr(skb);

		pppoe_match = (cm->pppoe_session_id == sfe_l2_pppoe_session_id_get(l2_info)) &&
				ether_addr_equal((u8*)cm->pppoe_remote_mac, (u8 *)eth->h_source);

		if (unlikely(!pppoe_match)) {
			DEBUG_TRACE("%px: PPPoE sessions ID %d and %d or MAC %pM and %pM did not match\n",
					skb, cm->pppoe_session_id, sfe_l2_pppoe_session_id_get(l2_info),
					cm->pppoe_remote_mac, eth->h_source);
			rcu_read_unlock();
			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_INVALID_PPPOE_SESSION);
			return 0;
		}

		skb->protocol = htons(l2_info->protocol);
		this_cpu_inc(si->stats_pcpu->pppoe_decap_packets_forwarded64);
	} else if (unlikely(sfe_l2_parse_flag_check(l2_info, SFE_L2_PARSE_FLAGS_PPPOE_INGRESS))) {

		/*
		 * If packet contains PPPoE header but CME doesn't contain PPPoE flag yet we are exceptioning the packet to linux
		 */
		if (unlikely(!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_BRIDGE_FLOW))) {
			rcu_read_unlock();
			DEBUG_TRACE("%px: CME doesn't contain PPPoE flag but packet has PPPoE header\n", skb);
			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_PPPOE_NOT_SET_IN_CME);
			return 0;

		}

		/*
		 * For bridged flows when packet contains PPPoE header, restore the header back and forward to xmit interface
		 */
		__skb_push(skb, (sizeof(struct pppoe_hdr) + sizeof(struct sfe_ppp_hdr)));

		this_cpu_inc(si->stats_pcpu->pppoe_bridge_packets_forwarded64);
	}

	/*
	 * protocol handler will be valid only in decap-path.
	 */
	if (cm->proto) {
		struct inet6_protocol *ipprot = cm->proto;
		skb_pull(skb, ihl);
		skb_reset_transport_header(skb);
		skb->fast_forwarded = 1;

		ret = ipprot->handler(skb);
		if (ret) {
			this_cpu_inc(si->stats_pcpu->packets_not_forwarded64);
			rcu_read_unlock();
			return 1;
		}

		/*
		 * Update traffic stats.
		 */
		atomic_inc(&cm->rx_packet_count);
		atomic_add(len, &cm->rx_byte_count);

		this_cpu_inc(si->stats_pcpu->packets_forwarded64);
		rcu_read_unlock();
		DEBUG_TRACE("%p: %s decap done\n",skb, __func__);
		return 1;
	}

	/*
	 * Check if skb has enough headroom to write L2 headers
	 */
	if (unlikely(skb_headroom(skb) < cm->l2_hdr_size)) {
		rcu_read_unlock();
		DEBUG_WARN("%px: Not enough headroom: %u\n", skb, skb_headroom(skb));
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_NO_HEADROOM);
		return 0;
	}

	/*
	 * Set SKB packet type to PACKET_HOST
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PACKET_HOST)) {
		skb->pkt_type = PACKET_HOST;
	}

	/*
	 * If our packet is larger than the MTU of the transmit interface then
	 * we can't forward it easily.
	 */
	if (unlikely(len > cm->xmit_dev_mtu)) {
		sfe_ipv6_sync_status(si, cm->connection, SFE_SYNC_REASON_STATS);
		rcu_read_unlock();

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_GRE_NEEDS_FRAGMENTATION);
		DEBUG_TRACE("Larger than MTU\n");
		return 0;
	}

	/*
	 * Update DSCP
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_DSCP_REMARK)) {
		sfe_ipv6_change_dsfield(iph, cm->dscp);
	}

	iph->hop_limit -= (u8)(!bridge_flow & !tun_outer);

	/*
	 * Update traffic stats.
	 */
	atomic_inc(&cm->rx_packet_count);
	atomic_add(len, &cm->rx_byte_count);

	xmit_dev = cm->xmit_dev;
	skb->dev = xmit_dev;

	/*
	 * For PPPoE flows, add PPPoE header before L2 header is added.
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PPPOE_ENCAP)) {
		sfe_pppoe_add_header(skb, cm->pppoe_session_id, PPP_IPV6);
		this_cpu_inc(si->stats_pcpu->pppoe_encap_packets_forwarded64);
	}

	/*
	 * For trustsec flows, add trustsec header before L2 header is added.
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_INSERT_EGRESS_TRUSTSEC_SGT)) {
		sfe_trustsec_add_sgt(skb, &cm->egress_trustsec_hdr);
	}

	/*
	 * Check to see if we need to add VLAN tags
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_INSERT_EGRESS_VLAN_TAG)) {
		sfe_vlan_add_tag(skb, cm->egress_vlan_hdr_cnt, cm->egress_vlan_hdr);
	}

	if (cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR) {
		/*
		 * For the simple case we write this really fast.
		 */
		struct ethhdr *eth = (struct ethhdr *)__skb_push(skb, ETH_HLEN);
		eth->h_proto = skb->protocol;
		ether_addr_copy((u8 *)eth->h_dest, (u8 *)cm->xmit_dest_mac);
		ether_addr_copy((u8 *)eth->h_source, (u8 *)cm->xmit_src_mac);
	} else if (cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_L2_HDR) {
		dev_hard_header(skb, xmit_dev, ntohs(skb->protocol),
				cm->xmit_dest_mac, cm->xmit_src_mac, len);
	}

	/*
	 * Update priority of skb.
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
		skb->priority = cm->priority;
	}

	/*
	 * Mark outgoing packet.
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_MARK)) {
		skb->mark = cm->mark;
	}

	rcu_read_unlock();

	this_cpu_inc(si->stats_pcpu->packets_forwarded64);

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
