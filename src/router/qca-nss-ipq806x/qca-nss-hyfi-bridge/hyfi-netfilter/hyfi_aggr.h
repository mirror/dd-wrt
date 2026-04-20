/*
 *  QCA HyFi Packet Aggregation
 *
 * Copyright (c) 2013-2014, 2016 The Linux Foundation. All rights reserved.
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

#ifndef HYFI_AGGR_H_
#define HYFI_AGGR_H_

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <net/ip.h>
#include "hyfi_api.h"
#include "hyfi_bridge.h"
#include "hyfi_hatbl.h"
#include "queue.h"
#include "hyfi_aggr_types.h"

int hyfi_aggr_init_entry(struct net_hatbl_entry *ha, u_int16_t seq);
int hyfi_aggr_new_flow(struct hyfi_net_bridge *br, struct __hatbl_entry *hae,
		struct net_hatbl_entry *ha);
int hyfi_aggr_update_flow(struct hyfi_net_bridge *br, struct __hatbl_entry *hae,
		struct net_hatbl_entry *ha);
int hyfi_aggr_flush(struct net_hatbl_entry *ha);
struct net_bridge_port *hyfi_aggr_process_pkt(struct net_hatbl_entry *ha,
		struct sk_buff **skb, u_int16_t seq);
struct net_bridge_port *hyfi_aggr_handle_tx_path(struct net_hatbl_entry *ha,
		struct sk_buff **skb);
int hyfi_aggr_end(struct net_hatbl_entry *ha);

static inline void hyfi_aggr_untag_packet(struct sk_buff *skb)
{
	struct iphdr *iph;
	u_int32_t ihl;

	if (unlikely(skb->protocol != htons(ETH_P_IP)))
		return;

	if (unlikely(!pskb_may_pull(skb, sizeof(struct iphdr))))
		return;

	iph = ip_hdr(skb);
	ihl = iph->ihl * 4;

	if (unlikely((ihl < 24) || !pskb_may_pull(skb, ihl)))
		return;

	ihl -= HYFI_AGGR_HEADER_LEN;

	/* Strip the tag from the IP header */
	iph->ihl--;
	iph->tot_len -= HYFI_AGGR_HEADER_LEN;
	ip_send_check(iph);

	/* Move Ethernet header and IP header 4 bytes forward */
	memmove(skb_mac_header(skb) + HYFI_AGGR_HEADER_LEN, skb_mac_header(skb),
			skb->mac_len + ihl);

	/* Update pointers */
	skb_pull(skb, HYFI_AGGR_HEADER_LEN);
	skb_set_mac_header(skb, -skb->mac_len);
	skb_reset_network_header(skb);
}

static inline void hyfi_aggr_tag_packet(struct net_hatbl_entry *ha,
		struct sk_buff *skb)
{
	struct ip_opts *ip_opts;
	struct iphdr *iph;
	u_int32_t ihl;

	if (skb->protocol != htons(ETH_P_IP))
		return;

	if (skb_headroom(skb) < HYFI_AGGR_HEADER_LEN) {
		pskb_expand_head(skb, 0, HYFI_AGGR_HEADER_LEN, GFP_ATOMIC );
	}

	iph = ip_hdr(skb);
	ihl = iph->ihl * 4;

	/* Update pointers */
	skb_push(skb, HYFI_AGGR_HEADER_LEN);

	/* Move Ethernet header and IP header 4 bytes backward */
	memmove(skb_mac_header(skb) - HYFI_AGGR_HEADER_LEN, skb_mac_header(skb),
			skb->mac_len + ihl);

	skb_set_mac_header(skb, -(skb->mac_len));
	skb_reset_network_header(skb);

	/* Insert tag */
	iph = ip_hdr(skb);
	ip_opts = (struct ip_opts*) ((char *) iph + ihl);
	iph->ihl++;
	iph->tot_len += HYFI_AGGR_HEADER_LEN;
	ip_opts->type = 0x88;
	ip_opts->length = 4;
	ip_opts->data = ha->aggr_seq_data.aggr_cur_seq
			| (ha->aggr_seq_data.num_ifs << 14);
	ha->aggr_seq_data.aggr_cur_seq++;
	ip_send_check(iph);
}

static inline u_int16_t hyfi_aggr_find_next_seq(struct net_hatbl_entry *ha)
{
	u_int32_t i;
	u_int16_t min_seq_delta = ~0;
	u_int16_t next_seq = ~0;

	DEBUG_TRACE("%s: Current sequence = %d, valid = %d\n",
			__func__, ha->aggr_rx_entry->aggr_next_seq, ha->aggr_rx_entry->next_seq_valid);

	/* Handle the gap. Find which queue has the next closest sequence number */
	for (i = 0; i < ha->aggr_rx_entry->num_ifs; i++) {
		if (ha->aggr_rx_entry->hyfi_iface_info[i].seq_valid) {
			u_int32_t q_seq = ha->aggr_rx_entry->hyfi_iface_info[i].seq;

			DEBUG_TRACE("%s: Queue %d sequence = %d\n",
					__func__, i, ha->aggr_rx_entry->hyfi_iface_info[i].seq);

			if (ha->aggr_rx_entry->aggr_next_seq
					> ha->aggr_rx_entry->hyfi_iface_info[i].seq) {
				q_seq += 1 << 14;
			}

			if (q_seq - ha->aggr_rx_entry->aggr_next_seq <= min_seq_delta) {
				min_seq_delta = q_seq - ha->aggr_rx_entry->aggr_next_seq;
				next_seq = ha->aggr_rx_entry->hyfi_iface_info[i].seq;
			}
		}
	}

	return next_seq;
}

static inline int hyfi_aggr_timeout(struct net_hatbl_entry *ha)
{
	u_int32_t i, valids = 0;

	if (time_after( jiffies, ha->aggr_rx_entry->time_stamp + msecs_to_jiffies(HYFI_AGGR_TIMEOUT) )) {
		/* Timeout */
		return 2;
	} else {
		/* Fast detection: If all queues for this flow have pending skbs we conclude
		 * that we missed at least one packet.
		 */
		for (i = 0; i < ha->aggr_rx_entry->num_ifs; i++) {
			if (ha->aggr_rx_entry->hyfi_iface_info[i].seq_valid)
				valids++;
		}
	}

	return valids >= ha->aggr_rx_entry->num_ifs;
}

#endif /* HYFI_AGGR_H_ */
