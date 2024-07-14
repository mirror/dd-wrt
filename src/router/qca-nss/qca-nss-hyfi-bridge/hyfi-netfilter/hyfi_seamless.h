/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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

#ifndef _HYFI_PATH_SWITCH_H
#define _HYFI_PATH_SWITCH_H

#include "hyfi_api.h"
#include "queue.h"
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/if_ether.h>

typedef enum {
	HYFI_FORWARD_PKT, HYFI_DELIVER_PKT

} hyfi_ath_pkt_path_type;

struct hyfi_skb_track {
	TAILQ_ENTRY(hyfi_skb_track)
	skb_track_qelem; // Link list for packets
	struct sk_buff *skb;
	hyfi_ath_pkt_path_type hyfi_pkt_path;

	unsigned int network_header;
	unsigned int l3_pkt_len;
	unsigned long jiffies;    // Jiffies value when packet is received
	__be16 protocol;
	unsigned short data_pack;
	void *pheader_8023;
};

struct hyfi_skb_buffer {
	TAILQ_ENTRY(hyfi_skb_buffer)
	skb_buf_qelem; // Link list for packets
	struct sk_buff *skb;
};

struct path_switch_param {
	unsigned int enable_switch_markers;
	unsigned int old_if_quiet_timeout;
	unsigned int drop_markers;
	unsigned int mse_timeout_val;
	unsigned int dup_buf_flush_quota;
};

#define HYFI_PSW_PKT_CNT	      1024
#define HYFI_PSW_MAX_REORD_BUF        3072
#define HYFI_PSW_REORD_TIMEOUT        1500
#define HYFI_PSW_REORD_FLUSH_QUOTA    2
#define HYFI_PSW_MSE_CNT              4
#define HYFI_PSW_DUP_BUF_FLUSH_QUOTA  3
#define HYFI_PSW_OLD_IF_QUIET_TIME    64

struct ha_psw_stm_entry {
	enum hyInterfaceType buffered_port_type;
	spinlock_t track_q_lock;
	TAILQ_HEAD(hyfi_skb_track_q, hyfi_skb_track)
	skb_track_q;

	u_int32_t rmv_pkts;
	u_int16_t last_idx;
	u_int16_t mrk_id;
};
#define ETHER_ADDR_LEN 6

#ifndef _NET_ETHERNET_H_
struct ether_header {
	u_int8_t ether_dhost[ETHER_ADDR_LEN];
	u_int8_t ether_shost[ETHER_ADDR_LEN];
	u_int16_t ether_type;
} __packed;
#endif

struct net_hatbl_entry;
struct net_bridge;

typedef enum psw_pkt_type {
	HYFI_PSW_PKT_3 = 2,
	HYFI_PSW_PKT_4,

	HYFI_PSW_PKT_UNKNOWN = ~0

} psw_pkt_type;

struct psw_pkt {
	u_int8_t h_dest[ETH_ALEN]; /* destination eth addr */
	u_int8_t h_source[ETH_ALEN]; /* source ether addr    */
	u_int8_t field1[15];
	u_int8_t field2[ETH_ALEN];
	u_int32_t field3;
	u_int8_t field4[2];
	u_int8_t field5;
	u_int8_t field6[2];
	u_int8_t field7;
	u_int8_t field8[ETH_ALEN];
	u_int8_t field9;
	u_int32_t field10;
	u_int8_t field11[2];
	u_int16_t field12;
	u_int8_t field13[2];
	u_int16_t field14;
	u_int8_t field15[2];

}__attribute__((packed));

struct psw_ip_pkt {
	struct ethhdr eh;
	struct iphdr ip;
	u_int16_t field0;
	struct psw_pkt pkt;

}__attribute__((packed));

struct psw_flow_info {
	u_int16_t pkt_cnt;
	u_int16_t last_idx;
	u_int16_t dup_pkt;
	int32_t wait_idx;
	u_int16_t msb_rcv;
	u_int16_t mse_rcv;
	u_int32_t buf_pkt;
	u_int32_t buf_dev_idx;
	u_int16_t last_mrk_id;
	unsigned long last_jiffies;
	u_int32_t mbb_dev_idx;
	u_int32_t mse_timeout;
	unsigned long old_if_jiffies;

	spinlock_t buf_q_lock;
	TAILQ_HEAD(hyfi_skb_buf_q, hyfi_skb_buffer)
	skb_buf_q;
};

struct hyfi_net_bridge;

void hyfi_psw_init(struct hyfi_net_bridge *br);
void hyfi_psw_param_update(struct hyfi_net_bridge *br,
		struct __path_switch_param *p);
void hyfi_psw_adv_param_update(struct hyfi_net_bridge *br, u_int32_t param,
		void *p);
void hyfi_psw_stm_init(struct hyfi_net_bridge *br, struct net_hatbl_entry *ha);
void hyfi_psw_pkt_track(struct sk_buff *skb, struct net_hatbl_entry *ha,
		hyfi_ath_pkt_path_type pkt_path);
void hyfi_psw_flush_track_q(struct ha_psw_stm_entry *pha_psw_stm_entry);
void hyfi_psw_send_pkt(struct hyfi_net_bridge *br, struct net_hatbl_entry *ha,
		psw_pkt_type pkt_value, u_int16_t dpkt);
int hyfi_psw_process_hyfi_pkt(struct hyfi_net_bridge *br, struct sk_buff *skb,
		u_int32_t flag);
int hyfi_psw_process_pkt(struct net_hatbl_entry *ha, struct sk_buff **skb,
		const struct hyfi_net_bridge *br);
int hyfi_psw_flush_buf_q(struct net_hatbl_entry *ha);
int hyfi_psw_init_entry(struct net_hatbl_entry *ha);
#endif
