/*
 * sfe_ipv6_tunipip6.c
 *	Shortcut forwarding engine file for IPv6 TUNIPIP6
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
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <net/ip6_checksum.h>
#include <net/protocol.h>

#include "sfe_debug.h"
#include "sfe_api.h"
#include "sfe.h"
#include "sfe_flow_cookie.h"
#include "sfe_ipv6.h"
#include "sfe_vlan.h"
#include "sfe_trustsec.h"
#include "sfe_pppoe.h"

/*
 * sfe_ipv6_recv_tunipip6()
 *	Handle TUNIPIP6 packet receives and forwarding.
 */
int sfe_ipv6_recv_tunipip6(struct sfe_ipv6 *si, struct sk_buff *skb, struct net_device *dev,
			     unsigned int len, struct ipv6hdr *iph, unsigned int ihl,
				 bool sync_on_find, struct sfe_l2_info *l2_info, bool tun_outer)
{
	struct sfe_ipv6_addr *src_ip;
	struct sfe_ipv6_addr *dest_ip;
	__be16 src_port = 0;
	__be16 dest_port = 0;
	unsigned int ihl_tmp = sizeof(struct ipv6hdr);
	struct sfe_ipv6_connection_match *cm;
	bool non_dst = false;
	bool passthrough;
	u8 next_hdr;

	DEBUG_TRACE("%px: sfe: sfe_ipv6_recv_tunipip6 called.\n", skb);

	/*
	 * Read the IP address information.  Read the IP header data first
	 * because we've almost certainly got that in the cache.
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
		cm = sfe_ipv6_find_connection_match_rcu(si, dev, IPPROTO_IPIP, src_ip, src_port, dest_ip, dest_port);
	}
#else
	cm = sfe_ipv6_find_connection_match_rcu(si, dev, IPPROTO_IPIP, src_ip, src_port, dest_ip, dest_port);
#endif
	if (unlikely(!cm)) {
		rcu_read_unlock();
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_TUNIPIP6_NO_CONNECTION);
		DEBUG_TRACE("%px: no connection found\n", skb);
		return 0;
	}

	next_hdr = iph->nexthdr;

	/*
	 * Try to find an extension header(if any) that is not NEXTHDR_DEST.
	 */
	while (unlikely(sfe_ipv6_is_ext_hdr(next_hdr))) {
		struct sfe_ipv6_ext_hdr *ext_hdr;
		unsigned int ext_hdr_len;

		if(next_hdr != NEXTHDR_DEST) {
			non_dst = true;
			break;
		}

		ext_hdr = (struct sfe_ipv6_ext_hdr *)(skb->data + ihl_tmp);

		ext_hdr_len = ext_hdr->hdr_len;
		ext_hdr_len <<= 3;
		ext_hdr_len += sizeof(struct sfe_ipv6_ext_hdr);
		ihl_tmp += ext_hdr_len;

		next_hdr = ext_hdr->next_hdr;
	}

	/*
	 * If our packet has been marked as "sync on find" we will sync the status
	 * and forward it to slowpath, except that encap_limit is set for dslite tunnel
	 * which is embedded in exthdr type NEXTHDR_DEST.
	 */
	if (unlikely(sync_on_find && non_dst)) {
		sfe_ipv6_sync_status(si, cm->connection, SFE_SYNC_REASON_STATS);
		rcu_read_unlock();
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_TUNIPIP6_SYNC_ON_FIND);
		DEBUG_TRACE("%px: Sync on find\n", skb);

		return 0;
	}

	/*
	 * In the passthrough case, the packet needs to decrement hop_limit.
	 */
	passthrough = cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PASSTHROUGH;
	if (passthrough) {
		if (iph->hop_limit < 2) {
			sfe_ipv6_sync_status(si, cm->connection, SFE_SYNC_REASON_STATS);
			rcu_read_unlock();

			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_TUNIPIP6_SMALL_TTL);
			DEBUG_TRACE("%px: hop_limit too low\n", skb);
			return 0;
		}
		iph->hop_limit--;
	}

	/*
	 * If cm->proto is set, it means the decap path.
	 * Otherwise we forward the packet in encap path.
	 */
	if(cm->proto) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
		const struct inet6_protocol *ipprot = cm->proto;
#else
		struct inet6_protocol *ipprot = cm->proto;
#endif

		/*
		 * Do we expect an ingress VLAN tag for this flow?
		 * Note: We will only have ingress tag check in decap direction.
		 */
		if (unlikely(!sfe_vlan_validate_ingress_tag(skb, cm->ingress_vlan_hdr_cnt, cm->ingress_vlan_hdr, l2_info, 0))) {
			rcu_read_unlock();
			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_INGRESS_VLAN_TAG_MISMATCH);
			DEBUG_TRACE("VLAN tag mismatch. skb=%px\n"
				"cm: %u [0]=%x/%x [1]=%x/%x\n"
				"l2_info+: %u [0]=%x/%x [1]=%x/%x\n", skb,
				cm->ingress_vlan_hdr_cnt,
				htons(cm->ingress_vlan_hdr[0].tpid), cm->ingress_vlan_hdr[0].tci,
				htons(cm->ingress_vlan_hdr[1].tpid), cm->ingress_vlan_hdr[1].tci,
				l2_info->vlan_hdr_cnt,
				htons(l2_info->vlan_hdr[0].tpid), l2_info->vlan_hdr[0].tci,
				htons(l2_info->vlan_hdr[1].tpid), l2_info->vlan_hdr[1].tci);
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
				DEBUG_TRACE("%px: PPPoE session ID %d and %d or MAC %pM and %pM did not match\n",
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
			 * If packet contains PPPoE header but CME doesn't contain PPPoE flag yet we are exceptioning
			 * the packet to linux
			 */
			if (unlikely(!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_BRIDGE_FLOW))) {
				rcu_read_unlock();
				DEBUG_TRACE("%px: CME doesn't contain PPPoE flag but packet has PPPoE header\n", skb);
				sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_PPPOE_NOT_SET_IN_CME);
				return 0;
			}

			/*
			 * For bridged flows when packet contains PPPoE header, restore the header back and forward
			 * to xmit interface
			 */
			__skb_push(skb, (sizeof(struct pppoe_hdr) + sizeof(struct sfe_ppp_hdr)));

			this_cpu_inc(si->stats_pcpu->pppoe_bridge_packets_forwarded64);
		}

		skb_reset_network_header(skb);
		skb_pull(skb, ihl);
		skb_reset_transport_header(skb);

		/*
		 * ipprot->handler(skb) will always return 0;
		 * There is no way to tell whether the packet is dropped later in linux or not.
		 * Hence here inc the byte/packet count always.
		 */
		atomic_inc(&cm->rx_packet_count);
		atomic_add(len, &cm->rx_byte_count);
		this_cpu_inc(si->stats_pcpu->packets_forwarded64);
		rcu_read_unlock();
		DEBUG_TRACE("%px: %s decap done \n",skb, __func__);

		/*
		 * Update top interface for tunnel searching.
		 */
		skb->dev = cm->top_interface_dev;
		ipprot->handler(skb);
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
	 * If our packet is larger than the MTU of the transmit interface then
	 * we can't forward it easily.
	 */
	if (unlikely(len > cm->xmit_dev_mtu)) {
		sfe_ipv6_sync_status(si, cm->connection, SFE_SYNC_REASON_STATS);
		rcu_read_unlock();

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_TUNIPIP6_NEEDS_FRAGMENTATION);
		DEBUG_TRACE("%px: Larger than mtu\n", skb);
		return 0;
	}

	/*
	 * Update DSCP
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_DSCP_REMARK)) {
		sfe_ipv6_change_dsfield(iph, cm->dscp);
	}

	/*
	 * Set SKB packet type to PACKET_HOST
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PACKET_HOST)) {
		skb->pkt_type = PACKET_HOST;
	}

	/*
	 * Update traffic stats.
	 */
	atomic_inc(&cm->rx_packet_count);
	atomic_add(len, &cm->rx_byte_count);

	skb->dev = cm->xmit_dev;

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

	/*
	 * Check to see if we need to write a header.
	 */
	if (likely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_L2_HDR)) {
		if (unlikely(!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR))) {
			dev_hard_header(skb, cm->xmit_dev, ntohs(skb->protocol),
					cm->xmit_dest_mac, cm->xmit_src_mac, len);
		} else {
			struct ethhdr *eth = (struct ethhdr *)__skb_push(skb, ETH_HLEN);
			eth->h_proto = skb->protocol;
			ether_addr_copy((u8 *)eth->h_dest, (u8 *)cm->xmit_dest_mac);
			ether_addr_copy((u8 *)eth->h_source, (u8 *)cm->xmit_src_mac);
		}
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
	 * Mark that this packet has been fast forwarded and send it on its way.
	 */
	skb->fast_forwarded = 1;
	dev_queue_xmit(skb);

	return 1;
}
