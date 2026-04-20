/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
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

#ifndef HYFI_AGGR_TYPES_H_
#define HYFI_AGGR_TYPES_H_

#define HYFI_AGGR_HEADER_LEN            ( 4 )
#define HYFI_AGGR_TIMEOUT               ( 100 )  /* ms */
#define HYFI_AGGR_MAX_PACKETS           ( 255 ) /* Max packets per interface */
#define HYFI_AGGR_MAX_QUEUE_LEN         ( 3000 )
#define HYFI_AGGR_HEAD_COUNT            ( HYFI_AGGR_MAX_PACKETS )  /* packets */
#define HYFI_AGGR_TAIL_COUNT            ( HYFI_AGGR_MAX_PACKETS ) /* packets */
#define HYFI_AGGR_MAX_IFACES            ( 3 )

struct hyfi_iface_info {
	struct net_bridge_port *dst;
	u_int8_t packet_quota;
	u_int8_t packet_count;
};

struct hyfi_aggr_seq_data {
	u_int16_t aggr_cur_seq :14;
	u_int16_t aggr_cur_iface :2;
	u_int16_t num_ifs;
};

struct hyfi_aggr_skb_buffer {
	TAILQ_ENTRY(hyfi_aggr_skb_buffer) skb_aggr_qelem; // Link list for packets
	struct sk_buff *skb;

	u_int16_t pkt_seq :14;
};

struct hyfi_aggr_rx_info {
	u_int16_t ifindex;
	u_int16_t seq_valid;
	u_int16_t seq :14;
	TAILQ_HEAD(hyfi_skb_aggr_q, hyfi_aggr_skb_buffer)
	skb_aggr_q;
	u_int32_t pkt_cnt;
};

struct hyfi_aggr_rx_entry {
	u_int16_t next_seq_valid;
	u_int16_t aggr_next_seq :14;
	u_int16_t num_ifs :2;
	unsigned long time_stamp;
	struct hyfi_aggr_rx_info hyfi_iface_info[HYFI_AGGR_MAX_IFACES];
	u_int32_t aggr_new_flow;
};

struct ip_opts {
	u_int8_t type;
	u_int8_t length;
	u_int16_t data;
}__attribute__ ((packed));

#endif /* HYFI_AGGR_TYPES_H_ */
