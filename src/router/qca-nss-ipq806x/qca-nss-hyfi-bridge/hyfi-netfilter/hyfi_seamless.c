/*
 * Copyright (c) 2012-2014, 2016, The Linux Foundation. All rights reserved.
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

#define DEBUG_LEVEL HYFI_NF_DEBUG_LEVEL

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#include <net/ip.h>
#include <asm/uaccess.h>
#include "hyfi_bridge.h"
#include "hyfi_api.h"
#include "hyfi_hatbl.h"
#include "hyfi_seamless.h"
#include "hyfi_hash.h"

static const u_int8_t field1[15] = { 0x88, 0xb7, 0x00, 0x03, 0x7f, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x20 };

#define TAG_80211Q_START_BYTE_POS 	12

struct nbuf_cb {
	u_int8_t vlan[8]; /* reserve for vlan tag info */
	void *ni;
	u_int32_t flags;
#define	N_LINK0		0x01			/* frame needs WEP encryption */
#define	N_FF		0x02			/* fast frame */
#define	N_PWR_SAV	0x04			/* bypass power save handling */
#define N_UAPSD		0x08			/* frame flagged for u-apsd handling */
#define N_EAPOL		0x10			/* frame flagged for EAPOL handling */
#define N_AMSDU		0x20			/* frame flagged for AMSDU handling */
#define	N_NULL_PWR_SAV	0x40			/* null data with power save bit on */
#define N_PROBING	0x80			/* frame flagged as a probing one */
#define N_ERROR         0x100                   /* frame flagged as a error one */
#define N_MOREDATA      0x200                   /* more data flag */
#define N_SMPSACTM      0x400                   /* This frame is SM power save Action Mgmt frame */
#define N_QOS           0x800                   /* This is a QOS frame*/
#define N_ENCAP_DONE    0x1000              /* This frame is marked as fast-path pkts, some encapsulation work has been done by h/w */
#define N_CLONED		0x2000		/* frame is cloned in rx path */
#define N_ANT_TRAIN		0x8000		/* frame is smart antenna training packet */
};

#ifdef HYFI_SUPPORT_PSW
static inline void hyfi_psw_read_idx(struct sk_buff *skb,
		struct ha_psw_stm_entry *pha_psw_stm_entry)
{
	struct ethhdr *ethhdr = eth_hdr(skb);
	struct psw_pkt *pkt = NULL;

	if (unlikely(ethhdr->h_proto == htons(0x88b7))) {
		pkt = (struct psw_pkt *) ethhdr;
	} else if (ethhdr->h_proto == htons(ETH_P_8021Q)) {
		if (unlikely(
				vlan_eth_hdr(skb)->h_vlan_encapsulated_proto
						== htons(0x88b7))) {
			pkt = (struct psw_pkt *) ((char *) (ethhdr) + 4);
		}
	}
}
#endif

void hyfi_psw_init(struct hyfi_net_bridge *br)
{

	br->path_switch_param.enable_switch_markers = 0;

	br->path_switch_param.mse_timeout_val = msecs_to_jiffies(
			HYFI_PSW_REORD_TIMEOUT);
	br->path_switch_param.drop_markers = 1;
	br->path_switch_param.old_if_quiet_timeout = msecs_to_jiffies(
			HYFI_PSW_OLD_IF_QUIET_TIME);
	br->path_switch_param.dup_buf_flush_quota = HYFI_PSW_DUP_BUF_FLUSH_QUOTA;
}

void hyfi_psw_param_update( struct hyfi_net_bridge *br,
	struct __path_switch_param *p )
{
	br->path_switch_param.enable_switch_markers = p->enable_switch_markers;

	DEBUG_INFO("Load-balancing seamless path switching: %s\n",
			br->path_switch_param.enable_switch_markers ? "Enabled" : "Disabled");
}

void hyfi_psw_adv_param_update(struct hyfi_net_bridge *br, u_int32_t param,
		void *val)
{
	u_int32_t *new_val = (u_int32_t *) val;

	if (!val)
		return;

	switch (param) {
	case HYFI_SET_PSW_MSE_TIMEOUT:
		br->path_switch_param.mse_timeout_val = msecs_to_jiffies(*new_val);
		DEBUG_INFO("Switch end timeout value: %dms\n", jiffies_to_msecs(br->path_switch_param.mse_timeout_val));
		break;

	case HYFI_SET_PSW_DROP_MARKERS:
		br->path_switch_param.drop_markers = *new_val;
		DEBUG_INFO("Drop markers: %s\n", br->path_switch_param.drop_markers ? "Yes" : "No");
		break;

	case HYFI_SET_PSW_OLD_IF_QUIET_TIME:
		br->path_switch_param.old_if_quiet_timeout = msecs_to_jiffies(*new_val);
		DEBUG_INFO("Old interface quiet time to declare timeout = %dms\n", jiffies_to_msecs(br->path_switch_param.old_if_quiet_timeout));
		break;

	case HYFI_SET_PSW_DUP_PKT_FLUSH_QUOTA:
		br->path_switch_param.dup_buf_flush_quota = *new_val;
		DEBUG_INFO("Duplicate buffer flush quota = %d packets\n", br->path_switch_param.dup_buf_flush_quota);
		break;

	default:
		break;
	}
}

void hyfi_psw_stm_init(struct hyfi_net_bridge *br, struct net_hatbl_entry *ha)
{
	struct ha_psw_stm_entry *pha_psw_stm_entry;

	pha_psw_stm_entry = &ha->psw_stm_entry;

	memset(pha_psw_stm_entry, 0, sizeof(struct ha_psw_stm_entry));

	TAILQ_INIT(&pha_psw_stm_entry->skb_track_q);
	spin_lock_init(&pha_psw_stm_entry->track_q_lock);

	pha_psw_stm_entry->buffered_port_type = hyfi_portgrp_num(hyfi_bridge_get_port(ha->dst));
}

void hyfi_psw_flush_track_q(struct ha_psw_stm_entry *pha_psw_stm_entry)
{
	struct hyfi_skb_track_q *skb_track_q;
	struct hyfi_skb_track *hyfi_skb_track;
	int flush_pkt_cnt = 0;

	spin_lock_bh(&pha_psw_stm_entry->track_q_lock);

	skb_track_q = &pha_psw_stm_entry->skb_track_q;
	hyfi_skb_track = TAILQ_FIRST( skb_track_q );

	while (hyfi_skb_track) {
		TAILQ_REMOVE( skb_track_q, hyfi_skb_track, skb_track_qelem);
		kfree_skb(hyfi_skb_track->skb);
		flush_pkt_cnt++;
		hyfi_skb_track = TAILQ_FIRST( skb_track_q );
	}

	spin_unlock_bh(&pha_psw_stm_entry->track_q_lock);
	DEBUG_WARN("%s[%d] flushed packet=%d\n",
			__func__, __LINE__, flush_pkt_cnt);
}

/* Name:        hyfi_psw_send_pkt
 * Purpose:     Send a hybrid packet.
 * Notes:
 * Inputs:      br - pointer to hybrid bridge context
 *              ha - pointer to HA table entry
 *              pkt_value - PSW packet type
 *              dpkt - pkt count
 * Outputs:     None.
 * Returns:     None.
 */
void hyfi_psw_send_pkt(struct hyfi_net_bridge *br, struct net_hatbl_entry *ha,
		psw_pkt_type pkt_value, u_int16_t dpkt)
{
	struct psw_ip_pkt *ip_pkt;
	struct psw_pkt *pkt;
	struct sk_buff *skb;

	skb = alloc_skb(ETH_FRAME_LEN + 50, GFP_ATOMIC );
	if (skb == NULL ) {
		DEBUG_ERROR("%s[%d] alloc skb fail\n", __func__, __LINE__);
		return;
	}

	skb_reserve(skb, 50);
	ip_pkt = (struct psw_ip_pkt *) skb_put(skb, sizeof(struct psw_ip_pkt));

	memcpy(ip_pkt->eh.h_dest, ha->da.addr, ETH_ALEN);
	memcpy(ip_pkt->eh.h_source, br->dev->dev_addr, ETH_ALEN);
	ip_pkt->eh.h_proto = htons(ETH_P_IP);

	skb_set_network_header(skb, offsetof(struct psw_ip_pkt, ip));
	ip_pkt->ip.version = 4;
	ip_pkt->ip.ihl = 5;
	ip_pkt->ip.frag_off = 0;
	ip_pkt->ip.ttl = 64;
	ip_pkt->ip.daddr = htonl(0x1010101);
	ip_pkt->ip.saddr = 0;
	ip_pkt->ip.tos = (__u8 ) ((ha->priority
			& ~HYFI_HACTIVE_TBL_PRIORITY_DSCP_VALID)>> 1);
	ip_pkt->ip.protocol = IPPROTO_ETHERIP;
	ip_pkt->ip.tot_len = htons(
			sizeof(struct psw_ip_pkt) - sizeof(struct ethhdr));

	ip_pkt->field0 = htons(0x3000);

	pkt = &ip_pkt->pkt;
	memcpy(pkt->h_dest, ha->da.addr, ETH_ALEN);
	memcpy(pkt->h_source, br->dev->dev_addr, ETH_ALEN);
	memcpy(pkt->field1, field1, sizeof(field1));
	memcpy(pkt->field2, br->dev->dev_addr, ETH_ALEN);
	pkt->field3 = 0;
	pkt->field4[0] = 0x32;
	pkt->field4[1] = 0x03;
	pkt->field5 = pkt_value;
	pkt->field6[0] = 0x33;
	pkt->field6[1] = 0xe;
	pkt->field7 = ha->hash;
	memcpy(pkt->field8, (unsigned char *) ha->da.addr, ETH_ALEN);
	pkt->field9 = ha->sub_class;
	pkt->field10 = ha->priority;

	switch (pkt_value) {
	case HYFI_PSW_PKT_3:
	case HYFI_PSW_PKT_4:
		pkt->field11[0] = 0x36;
		pkt->field11[1] = 0x04;
		pkt->field12 = htons(ha->psw_stm_entry.mrk_id);
		pkt->field13[0] = 0xff;
		pkt->field13[1] = 0x02;
		break;

	default:
		pkt->field11[0] = 0xff;
		pkt->field11[1] = 0x02;
		break;
	}

	skb_reset_mac_header(skb);
	skb_pull(skb, ETH_HLEN);

	skb->dev = br->dev;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	br_forward(ha->dst, skb, false, true);
#else
	br_deliver(ha->dst, skb);
#endif
}

static int hyfi_psw_recv_pkt(psw_pkt_type type, struct psw_pkt *pkt,
		u_int32_t *hash, u_int16_t *idx, u_int32_t *priority,
		const unsigned char **da, u_int32_t *sub_class, u_int16_t *freed_pkt)
{
	*idx = pkt->field12;
	*hash = pkt->field7;
	*priority = pkt->field10;
	*da = pkt->field8;
	*sub_class = pkt->field9;

	return 0;
}

static psw_pkt_type hyfi_psw_detect_pkt(struct psw_pkt *psw_pkt)
{
	switch (psw_pkt->field5) {
	case HYFI_PSW_PKT_3:
	case HYFI_PSW_PKT_4:
		if (psw_pkt->field11[0] == 0x36 && psw_pkt->field13[0] == 0xff)
			return psw_pkt->field5;

		return HYFI_PSW_PKT_UNKNOWN;
	default:
		return HYFI_PSW_PKT_UNKNOWN;
	}
}

static int hyfi_psw_forward_and_queue(struct net_hatbl_entry *ha,
		struct sk_buff **skb)
{
	struct hyfi_skb_buf_q *skb_buf_q;
	struct hyfi_skb_buffer *hyfi_skb_buffer;
	struct sk_buff *pskb;
	u_int32_t quota = HYFI_PSW_REORD_FLUSH_QUOTA;

	skb_buf_q = &ha->psw_info.skb_buf_q;
	hyfi_skb_buffer = TAILQ_FIRST( skb_buf_q );

	while (hyfi_skb_buffer && quota--) {
		TAILQ_REMOVE( skb_buf_q, hyfi_skb_buffer, skb_buf_qelem);
		hyfi_br_forward(ha->dst, hyfi_skb_buffer->skb);
		ha->psw_info.pkt_cnt++;
		ha->psw_info.buf_pkt--;
		hyfi_skb_buffer = TAILQ_FIRST( skb_buf_q );
	}

	if (skb) {
		pskb = *skb;

		if (pskb && hyfi_skb_buffer) {
			hyfi_skb_buffer = (struct hyfi_skb_buffer *) pskb->head;
			hyfi_skb_buffer->skb = pskb;
			TAILQ_INSERT_TAIL( skb_buf_q, hyfi_skb_buffer, skb_buf_qelem);
			ha->psw_info.buf_pkt++;
			if (*skb) {
				*skb = NULL;
			}

			return 1;
		}
	}

	if (!hyfi_skb_buffer) {
		ha->psw_info.mse_rcv = 0;
		ha->psw_info.msb_rcv = 0;
		ha->psw_info.mse_timeout = 0;
		ha->psw_info.buf_dev_idx = 0;
		ha->psw_info.old_if_jiffies = 0;
	}

	return 0;
}

static int hyfi_psw_handle_pkt3(struct psw_pkt *psw_pkt,
		struct hyfi_net_bridge *br)
{
	int32_t ret = -1;
#ifndef DISABLE_APS_HOOKS
	struct net_hatbl_entry *ha = NULL;
#endif
	u_int32_t hash, priority, sub_class;
	u_int16_t mrk_id;
	const unsigned char *da;

	ret = hyfi_psw_recv_pkt(HYFI_PSW_PKT_3, psw_pkt, &hash, &mrk_id, &priority,
			&da, &sub_class, NULL );

	if (ret < 0)
		return 0;
#ifndef DISABLE_APS_HOOKS
	spin_lock(&br->hash_ha_lock);

	ha = hyfi_hatbl_find_tracked_entry(br, hash, da, sub_class, priority);

	if (!ha) {
		ha = hyfi_hatbl_create_tracked_entry(br, hash, psw_pkt->h_source, da,
				sub_class, priority);

		if (!ha) {
			spin_unlock(&br->hash_ha_lock);
			return 0;
		}
	}

	spin_unlock(&br->hash_ha_lock);
	spin_lock(&ha->psw_info.buf_q_lock);

	ha->psw_info.last_jiffies = jiffies;
	ha->psw_info.mbb_dev_idx = 0;
	if (ha->psw_info.msb_rcv && ha->psw_info.last_mrk_id == mrk_id) {
		if (!ha->psw_info.mse_timeout) {
			DEBUG_TRACE("Switch end of stream marker %d detected AFTER switch begin (Delta=%ums), forwarding %d buffered packets, buffered ifindex %d\n",
					mrk_id, jiffies_to_msecs(jiffies - ha->psw_info.old_if_jiffies), ha->psw_info.buf_pkt, ha->psw_info.buf_dev_idx);

			ha->psw_info.msb_rcv = 0;
			ha->psw_info.mse_rcv = 0;
			ha->psw_info.old_if_jiffies = 0;
		} else {
			DEBUG_TRACE("Ignoring Switch end of stream marker %d after timeout\n",
					mrk_id);
		}
	} else {
		if (mrk_id != ha->psw_info.last_mrk_id) {
			ha->psw_info.last_mrk_id = mrk_id;

			if (!(ha->psw_info.dup_pkt || ha->psw_info.wait_idx >= 0)) {
				DEBUG_TRACE("Switch end of stream marker %d detected\n", mrk_id);
				ha->psw_info.mse_rcv = 1;
				ha->psw_info.mse_timeout = 0;
			} else {
				DEBUG_TRACE("Switch end of stream marker during buffer - ignore\n");
			}
		} else {
			DEBUG_TRACE("Ignoring Switch end of stream marker %d detected\n",
					mrk_id);
		}
	}

	spin_unlock(&ha->psw_info.buf_q_lock);
#endif
	return 0;
}

static int hyfi_psw_handle_pkt4(struct psw_pkt *psw_pkt,
		struct hyfi_net_bridge *br, struct sk_buff *pskb)
{
	int32_t ret = -1;
#ifndef DISABLE_APS_HOOKS
	struct net_hatbl_entry *ha = NULL;
#endif
	u_int32_t hash, priority, sub_class;
	u_int16_t mrk_id;
	const unsigned char *da;

	ret = hyfi_psw_recv_pkt(HYFI_PSW_PKT_4, psw_pkt, &hash, &mrk_id, &priority,
			&da, &sub_class, NULL );

	if (ret < 0)
		return 0;
#ifndef DISABLE_APS_HOOKS
	spin_lock(&br->hash_ha_lock);

	ha = hyfi_hatbl_find_tracked_entry(br, hash, da, sub_class, priority);

	if (!ha) {
		ha = hyfi_hatbl_create_tracked_entry(br, hash, psw_pkt->h_source, da,
				sub_class, priority);

		if (!ha) {
			spin_unlock(&br->hash_ha_lock);
			return 0;
		}
	}

	spin_unlock(&br->hash_ha_lock);
	spin_lock(&ha->psw_info.buf_q_lock);

	ha->psw_info.last_jiffies = jiffies;
	if (ha->psw_info.mse_rcv && ha->psw_info.last_mrk_id == mrk_id) {
		DEBUG_TRACE("Switch beginning of stream marker %d detected after switch end - No buffering\n",
				mrk_id);
		ha->psw_info.mse_rcv = 0;
		ha->psw_info.msb_rcv = 0;
		ha->psw_info.buf_dev_idx = 0;
		ha->psw_info.mse_timeout = 0;
		ha->psw_info.old_if_jiffies = 0;
	} else {
		ha->psw_info.last_mrk_id = mrk_id;
		if (!(ha->psw_info.dup_pkt || ha->psw_info.wait_idx >= 0)) {
			DEBUG_TRACE("Switch beginning of stream marker %d detected BEFORE switch end - Starting to buffer from ifindex %d\n",
					mrk_id, pskb->dev->ifindex);
			ha->psw_info.msb_rcv = 1;
			ha->psw_info.buf_dev_idx = pskb->dev->ifindex;
			ha->psw_info.dup_pkt = 0;
			ha->psw_info.mse_timeout = 0;
			ha->psw_info.old_if_jiffies = jiffies;
		} else {
			DEBUG_TRACE("Switch begin of stream marker during buffer - ignore\n");
			ha->psw_info.msb_rcv = 0;
		}
	}

	spin_unlock(&ha->psw_info.buf_q_lock);
#endif
	return 0;
}

int hyfi_psw_process_pkt(struct net_hatbl_entry *ha, struct sk_buff **skb,
		const struct hyfi_net_bridge *br)
{
	struct sk_buff *pskb = *skb;

	spin_lock(&ha->psw_info.buf_q_lock);

	if (unlikely(ha->psw_info.dup_pkt || ha->psw_info.wait_idx >= 0)) {
		if (*skb) {
			kfree_skb(*skb);
			*skb = NULL;
		}

		if (ha->psw_info.wait_idx < 0) {
			ha->psw_info.dup_pkt--;
			ha->psw_info.pkt_cnt++;
		}

		spin_unlock(&ha->psw_info.buf_q_lock);
		return 1;
	}

	if (unlikely(
			ha->psw_info.mse_timeout && pskb
					&& ha->psw_info.buf_dev_idx != pskb->dev->ifindex)) {
		kfree_skb(pskb);
		*skb = NULL;

		DEBUG_TRACE("Dropping packet from old interface %d\n",
			pskb->dev->ifindex);

		spin_unlock(&ha->psw_info.buf_q_lock);
		return 1;
	}

	if (unlikely(
			ha->psw_info.msb_rcv && !ha->psw_info.mse_rcv
					&& !ha->psw_info.mse_timeout)) {
		if (pskb && ha->psw_info.buf_dev_idx == pskb->dev->ifindex) {
			struct hyfi_skb_buf_q *skb_buf_q;
			struct hyfi_skb_buffer *hyfi_skb_buffer =
					(struct hyfi_skb_buffer *) pskb->head;

			if (likely(
					time_before(jiffies, ha->psw_info.last_jiffies + br->path_switch_param.mse_timeout_val)
							&& ha->psw_info.buf_pkt < HYFI_PSW_MAX_REORD_BUF)) {
				hyfi_skb_buffer->skb = pskb;
				skb_buf_q = &ha->psw_info.skb_buf_q;

				TAILQ_INSERT_TAIL( skb_buf_q, hyfi_skb_buffer, skb_buf_qelem);
				ha->psw_info.buf_pkt++;

				if (*skb) {
					*skb = NULL;
				}

				if (time_after( jiffies, ha->psw_info.old_if_jiffies + br->path_switch_param.old_if_quiet_timeout )) {
					DEBUG_TRACE("Timeout waiting for switch end, old medium appears to be dead (jiffies=%lu, old_if_jiffies=%lu, timeout=%ums), forwarding %d buffered packets\n",
							jiffies, ha->psw_info.old_if_jiffies, jiffies_to_msecs(br->path_switch_param.old_if_quiet_timeout), ha->psw_info.buf_pkt);
					ha->psw_info.mse_timeout = 1;
					ha->psw_info.old_if_jiffies = 0;
				}

				spin_unlock(&ha->psw_info.buf_q_lock);
				return 1;
			} else {
				if (!ha->psw_info.mse_timeout) {
					DEBUG_TRACE("Timeout waiting for switch end (jiffies=%lu, last_jiffies=%lu, timeout=%ums), forwarding %d buffered packets\n",
							jiffies, ha->psw_info.last_jiffies, jiffies_to_msecs(br->path_switch_param.mse_timeout_val), ha->psw_info.buf_pkt);
					ha->psw_info.mse_timeout = 1;

					if (hyfi_psw_forward_and_queue(ha, skb)) {
						spin_unlock(&ha->psw_info.buf_q_lock);
						return 1;
					}
				}
			}
		} else {
			ha->psw_info.old_if_jiffies = jiffies;
		}
	} else if (unlikely(!ha->psw_info.mse_rcv && ha->psw_info.buf_pkt)) {
		if (hyfi_psw_forward_and_queue(ha, skb)) {
			spin_unlock(&ha->psw_info.buf_q_lock);
			return 1;
		}
	}

	ha->psw_info.pkt_cnt++;

	spin_unlock(&ha->psw_info.buf_q_lock);
	return 0;
}

int hyfi_psw_process_hyfi_pkt(struct hyfi_net_bridge *br, struct sk_buff *skb,
		u_int32_t flag)
{
	struct psw_pkt *psw_pkt;
	psw_pkt_type pkt_type;

	if (flag & IS_HYFI_PKT) {
		psw_pkt = (struct psw_pkt *) eth_hdr(skb);
	} else {
		psw_pkt = (struct psw_pkt *) (skb_network_header(skb) + ip_hdrlen(skb)
				+ sizeof(u_int16_t));
	}

	pkt_type = hyfi_psw_detect_pkt(psw_pkt);

	switch (pkt_type) {

	case HYFI_PSW_PKT_3:
		hyfi_psw_handle_pkt3(psw_pkt, br);
		break;

	case HYFI_PSW_PKT_4:
		hyfi_psw_handle_pkt4(psw_pkt, br, skb);
		break;

	default:
		return -1;

	}

	return 0;
}

int hyfi_psw_flush_buf_q(struct net_hatbl_entry *ha)
{
	struct hyfi_skb_buf_q *skb_buf_q;
	struct hyfi_skb_buffer *hyfi_skb_buffer;

	spin_lock_bh(&ha->psw_info.buf_q_lock);
	skb_buf_q = &ha->psw_info.skb_buf_q;
	hyfi_skb_buffer = TAILQ_FIRST( skb_buf_q );

	while (hyfi_skb_buffer) {
		TAILQ_REMOVE( skb_buf_q, hyfi_skb_buffer, skb_buf_qelem);
		kfree_skb(hyfi_skb_buffer->skb);
		hyfi_skb_buffer = TAILQ_FIRST( skb_buf_q );
	}

	ha->psw_info.buf_pkt = 0;
	spin_unlock_bh(&ha->psw_info.buf_q_lock);

	return 0;
}

int hyfi_psw_init_entry(struct net_hatbl_entry *ha)
{
	memset(&ha->psw_info, 0, sizeof(struct psw_flow_info));
	ha->psw_info.wait_idx = -1;
	ha->psw_info.last_mrk_id = ~0;

	TAILQ_INIT(&ha->psw_info.skb_buf_q);
	spin_lock_init(&ha->psw_info.buf_q_lock);

	return 0;
}

