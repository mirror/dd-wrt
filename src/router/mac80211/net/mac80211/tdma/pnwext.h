/*
 * Some TDMA support code for Linux wireless stack.
 *
 * Copyright 2011-2015	Stanislav V. Korsakov <sta@stasoft.net>
 */
#ifndef _PNWEXT_H
#define _PNWEXT_H

#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <net/cfg80211.h>
#include <net/mac80211.h>

#define TDMA_TX_TASKLET	1
#define	TDMA_DEBUG	1
#define	TDMA_MAX_NODE		48
#define	TDMA_MAX_NODE_PER_CELL		16
#define	TDMA_MAX_TX_QUEUE_LEN		123
#define	TDMA_MAX_TX_PER_ROUND		123

#define TDMA_MAX_REACH_VAL	0xFFFF

struct p_originator {
	struct p_originator *next;
	u8 mac[ETH_ALEN];
	u8 relay[ETH_ALEN];
	s8 energy;
	s8 enqueued;
	u8 hops;
	unsigned long last_seen;
};

struct sta_info;
struct ieee80211_sub_if_data;

#ifdef CPTCFG_MAC80211_TDMA_MESH

struct p_bcast {
	struct p_bcast *next;
	u8 sa[ETH_ALEN];
	u8 da[ETH_ALEN];
	u16 len;
	u8 ttl;
	u8 sequence;
	unsigned long jiffies;
};

#ifdef CPTCFG_MAC80211_DEBUGFS

#define	TMD_MAX_RECORDS	12

struct tmd_record {
	u8 da[ETH_ALEN];
	u8 ra[ETH_ALEN];
	u16 reach_val;
};

#endif
#endif

struct ieee80211_if_tdma {
	unsigned long counter;
	unsigned long tx_round_duration;
	unsigned long tx_slot_duration;
	unsigned long second_tx_slot_duration;
	unsigned long last_beacon;
	unsigned long last_own_beacon;
	unsigned long last_verified;
	unsigned long last_tx_start;
	unsigned long last_tx_call;

	int poll_timer_interval;

	bool privacy;
	bool control_port;
	bool optimize_tx;

	u32 rate;
	u16 node_bitmap;
	u16 cell_bitmap;
	u16 unknow_state_counter;
	u16 num_seqs;
	u16 ack_duration;
	u16 max_clock_drift;
	u16 poll_interval;

	struct hrtimer hk;

#ifdef TDMA_TX_TASKLET
	struct tasklet_struct tx_tasklet;
#else
	struct work_struct tx_work;
#endif
	struct work_struct reset_work;
	struct work_struct join_work;
	struct work_struct create_work;
	struct timer_list poll_timer;

	struct cfg80211_bitrate_mask mask;
	struct sta_info __rcu *sta;
	struct cfg80211_chan_def chandef;
	struct sk_buff_head tx_skb_list;
	struct p_originator *ohead;
#ifdef CPTCFG_MAC80211_TDMA_MESH
	struct p_bcast *bhead;
	unsigned long mesh_fwd;
#ifdef CPTCFG_MAC80211_DEBUGFS
	u8 tmd_pointer;
	struct tmd_record mesh_tx[TMD_MAX_RECORDS];
#endif
#endif

	u8 cfg;
	u8 state;
	u8 node_id;
	u8 node_num;
	u8 cur_rate;
	u8 slot_size;
	u8 tx_ratio;
	u8 rx_ratio;
	u8 last_beacon_node;
	u8 b_counter;
	u8 sc;
	u8 last_tx;
	u8 bssid[ETH_ALEN];

	unsigned int (*tdma_hdrlen) (__le16);
	unsigned int (*tdma_get_hdrlen_from_skb) (const struct sk_buff *);
	int (*tdma_chandef_get_shift) (struct cfg80211_chan_def *);
	int (*tdma_frame_duration) (enum nl80211_band, size_t, int, int, int, int);
	struct sta_info *(*tdma_sta_info_get) (struct ieee80211_sub_if_data *, const u8 *);
};

typedef enum {
	TDMA_CFG_VERSION = (BIT(0) | BIT(1)),
	TDMA_CFG_GACK = BIT(2),
	TDMA_CFG_NO_REORDER = BIT(3),
	TDMA_CFG_NO_MSDU = BIT(4),
	TDMA_CFG_POLLING = BIT(5),
} tdma_cfg_fields;

#define TDMA_CFG_BSSID(a)	((u8)(a->cfg & (TDMA_CFG_VERSION|TDMA_CFG_GACK)))

#define TDMA_CFG_VERSION(a)	((u8)(a->cfg & TDMA_CFG_VERSION))
#define TDMA_CFG_VERSION_P(a)	((u8)(a & TDMA_CFG_VERSION))
#define TDMA_CFG_NO_MSDU(a)	((u8)(a->cfg & TDMA_CFG_NO_MSDU))
#define TDMA_CFG_NO_MSDU_P(a)	((u8)(a & TDMA_CFG_NO_MSDU))
#define TDMA_CFG_NO_REORDER(a)	(!!(a->cfg & TDMA_CFG_NO_REORDER))
#define TDMA_CFG_NO_REORDER_P(a)	(!!(a & TDMA_CFG_NO_REORDER))
#define TDMA_CFG_GACK(a)	(!!(a->cfg & TDMA_CFG_GACK))
#define TDMA_CFG_GACK_P(a)	(!!(a & TDMA_CFG_GACK))
#define TDMA_CFG_POLLING(a)	(!!(a->cfg & TDMA_CFG_POLLING))
#define TDMA_CFG_POLLING_P(a)	(!!(a & TDMA_CFG_POLLING))

#define TDMA_CFG_SET_VERSION(a, v)	do { \
a->cfg &= ~(TDMA_CFG_VERSION); \
a->cfg |= (u8)(((u8)(v)) & TDMA_CFG_VERSION); \
} while (0);

#define TDMA_CFG_SET_NO_MSDU(a, v)	do { \
    if (!!(v)) \
	a->cfg |= TDMA_CFG_NO_MSDU; \
    else \
	a->cfg &= ~(TDMA_CFG_NO_MSDU); \
} while(0);

#define TDMA_CFG_SET_NO_REORDER(a, v)	do { \
    if (!!(v)) \
	a->cfg |= TDMA_CFG_NO_REORDER; \
    else \
	a->cfg &= ~(TDMA_CFG_NO_REORDER); \
} while(0);

#define TDMA_CFG_SET_GACK(a, v)	do { \
    if (!!(v)) \
	a->cfg |= TDMA_CFG_GACK; \
    else \
	a->cfg &= ~(TDMA_CFG_GACK); \
} while(0);

#define TDMA_CFG_SET_POLLING(a, v)	do { \
    if (!!(v)) \
	a->cfg |= TDMA_CFG_POLLING; \
    else \
	a->cfg &= ~(TDMA_CFG_POLLING); \
} while(0);

typedef enum {
	TDMA_STATUS_UNKNOW = 0,
	TDMA_STATUS_ASSOCIATING = BIT(0),
	TDMA_STATUS_ASSOCIATED = BIT(1),
	TDMA_STATUS_MAX = (BIT(0) | BIT(1)),
	TDMA_STATUS_JOINED = BIT(2),
	TDMA_STATUS_INITIALIZED = BIT(3),
} tdma_state;

#define TDMA_STATE(a)	(a->state & TDMA_STATUS_MAX)
#define TDMA_STATE_P(a)	(a & TDMA_STATUS_MAX)

#define TDMA_SET_STATE(a, v)	do { \
a->state &= ~(TDMA_STATUS_MAX); \
a->state |= (u8)(((u8)(v)) & TDMA_STATUS_MAX); \
} while(0);

#define TDMA_JOINED(a)	(!!(a->state & TDMA_STATUS_JOINED))
#define TDMA_JOINED_P(a)	(!!(a & TDMA_STATUS_JOINED))

#define TDMA_SET_JOINED(a, v)	do { \
    if (!!(v)) \
	a->state |= TDMA_STATUS_JOINED; \
    else \
	a->state &= ~(TDMA_STATUS_JOINED); \
} while(0);

#define TDMA_INITIALIZED(a)	(!!(a->state & TDMA_STATUS_INITIALIZED))
#define TDMA_INITIALIZED_P(a)	(!!(a & TDMA_STATUS_INITIALIZED))

#define TDMA_SET_INITIALIZED(a, v)	do { \
    if (!!(v)) \
	a->state |= TDMA_STATUS_INITIALIZED; \
    else \
	a->state &= ~(TDMA_STATUS_INITIALIZED); \
} while(0);

typedef enum {
	TDMA_HDR_COUNTER = 0xf,
	TDMA_HDR_TTL = 0x70,
	TDMA_HDR_AMSDU = 0x80,
	TDMA_HDR_ADD_INFO = 0x4000,
	TDMA_HDR_COMPRESSED = 0x8000,
} tdma_hdr_fields;

struct tdma_hdr {
	u8 flags;
	u8 sequence;
	__le16 len;
} __attribute__((__packed__));

#define TDMA_HDR_COUNTER(a)	(a->flags & TDMA_HDR_COUNTER)
#define TDMA_HDR_SET_COUNTER(a, v)	do { \
a->flags &= ~(TDMA_HDR_COUNTER); \
a->flags |= (((u8)(v)) & TDMA_HDR_COUNTER); \
} while (0);

#define TDMA_HDR_TTL(a)	((a->flags & TDMA_HDR_TTL) >> 4)
#define TDMA_HDR_SET_TTL(a, v)	do { \
a->flags &= ~(TDMA_HDR_TTL); \
a->flags |= (((u8)(v << 4)) & TDMA_HDR_TTL); \
} while (0);
#define TDMA_HDR_SET_MAX_TTL(a)	do { \
a->flags |= TDMA_HDR_TTL; \
} while (0);

#define TDMA_HDR_AMSDU(a)	(a->flags & TDMA_HDR_AMSDU)
#define TDMA_HDR_SET_AMSDU(a, v)	do { \
    if (!!(v)) \
	a->flags |= TDMA_HDR_AMSDU; \
    else \
	a->flags &= ~(TDMA_HDR_AMSDU); \
} while(0);

#define TDMA_HDR_GET_LEN(a)	(a->len & ~(cpu_to_le16(TDMA_HDR_ADD_INFO|TDMA_HDR_COMPRESSED)))

#define TDMA_HDR_INFO(a)	(a->len & cpu_to_le16(TDMA_HDR_ADD_INFO))
#define TDMA_HDR_SET_INFO(a, v)	do { \
    if (!!(v)) \
	a->len |= cpu_to_le16(TDMA_HDR_ADD_INFO); \
    else \
	a->len &= ~(cpu_to_le16(TDMA_HDR_ADD_INFO)); \
} while(0);

#define TDMA_HDR_COMPRESSED(a)	(a->len & cpu_to_le16(TDMA_HDR_COMPRESSED))
#define TDMA_HDR_SET_COMPRESSED(a, v)	do { \
    if (!!(v)) \
	a->len |= cpu_to_le16(TDMA_HDR_COMPRESSED); \
    else \
	a->len &= ~(cpu_to_le16(TDMA_HDR_COMPRESSED)); \
} while(0);

#define tdma_hdr_len()	(sizeof(struct tdma_hdr))
#define TDMA_MAX_HDR_LEN()	(sizeof(struct tdma_hdr))

#define TDMA_IS_MESHED(a)	((a->node_num > 2) && (TDMA_CFG_VERSION(a) < 2))

/* Should never be used because causes unaligned access on some archs */

struct tdma_ie {
	u8 tie_cfg;
	u8 tie_node_id;
	u8 tie_state;
	u8 tie_node_num;
	u8 tie_cur_rate;
	u8 tie_tx_rx_ratio;
	u8 tie_bcn_for;
	__le16 tie_node_bitmap;
	__le32 tie_rate;
	__le16 tie_info_sz;
} __attribute__((__packed__));

#define WLAN_EID_TDMA_PEER_INFO	200
#define WLAN_EID_TDMA_PEER_INFO_RETRY	223
#define WLAN_EID_TDMA_MESH_INFO	253

#define TDMA_MIN_IE_SIZE	sizeof(struct tdma_ie)
#define TDMA_IE_GET_TX_RATIO(a)	(a & 0xF)
#define TDMA_IE_GET_RX_RATIO(a)	((a & 0xF0) >> 4)
#define TDMA_IE_PACK_TX_RX_RATIO(a,b)	((a & 0xF) | ((b & 0xF) << 4))

#define TDMA_MAX_SEQS_IN_BEACON	TDMA_MAX_TX_QUEUE_LEN
#define TDMA_MAX_SEQS_IN_BEACON_SZ	(TDMA_MAX_SEQS_IN_BEACON << 1)
#ifdef CPTCFG_MAC80211_TDMA_MESH
#define	TDMA_MAX_BEACON_LEN	(320+IEEE80211_MAX_SSID_LEN+sizeof(struct ieee80211_ht_cap)+sizeof(struct ieee80211_ht_operation)+TDMA_MIN_IE_SIZE)
#else
#define	TDMA_MAX_BEACON_LEN	(64+IEEE80211_MAX_SSID_LEN+sizeof(struct ieee80211_ht_cap)+sizeof(struct ieee80211_ht_operation)+TDMA_MIN_IE_SIZE)
#endif

#define	TDMA_MAGIC_BYTE	0xfa
#define	TDMA_MAX_UNVERIFIED_COUNT	25
#define	TDMA_MAX_LOST_BEACON_COUNT	15

#define	TDMA_MIN_UNKNOW_STATE_COUNT	(TDMA_MAX_NODE_PER_CELL + 2)
#define	TDMA_MAX_UNKNOW_STATE_COUNT	(TDMA_MAX_NODE_PER_CELL + 10)

#define	TDMA_FUDGE_ASSOCIATING_STATE_COUNT	4
#define	TDMA_MAX_ASSOCIATING_STATE_COUNT	10
#define	TDMA_BAD_NEIGHBOUR_RATIO	60

#define	TDMA_MIN_READY_TX_4_COMPRESS		2

#define	tdma_get_slot_num(tdma)	(tdma->node_num)

#define TDMA_TX_TAIL_SPACE	128

struct tdma_neighbour {
	unsigned long created;
	unsigned long last_seen;
	unsigned long miss_counter;
	unsigned long counter;
	bool voted;
	bool miss_own_beacon;
	u16 node_bitmap;
	u16 sc;
	struct sk_buff_head pending;
	struct sk_buff_head tx;
	u8 aid;
	u8 num_seqs;
	u8 seqs[TDMA_MAX_TX_QUEUE_LEN];
	u8 num_rcvd;
	u8 rcvd[TDMA_MAX_TX_QUEUE_LEN];
};

struct ieee80211_sub_if_data;

extern unsigned long ptdma_tx_slot_duration(struct ieee80211_if_tdma *);
extern unsigned long ptdma_calc_ideal_interval(struct ieee80211_if_tdma *, int);
extern unsigned long ptdma_tu_adjust(unsigned long, int, int, bool);
extern int pamsdu_limit(struct ieee80211_supported_band *, struct ieee80211_sta *);
extern int ptdma_adjust_rates(struct ieee80211_supported_band *, struct ieee80211_tx_rate *, int *, bool);
extern void ptdma_update_hdr_counter(struct ieee80211_if_tdma *, struct sk_buff *, struct ieee80211_hdr *);
extern void ptdma_update_hdr_info(struct ieee80211_if_tdma *, struct sk_buff *, struct ieee80211_hdr *, bool);
extern bool ptdma_update_hdr_ttl(struct ieee80211_if_tdma *, struct sk_buff *, struct ieee80211_hdr *);
extern struct ieee80211_tx_info *ptdma_skb_fill_info(struct ieee80211_sub_if_data *, struct sk_buff *);
extern bool ptdma_skb_priority(struct sk_buff *);
extern bool ptdma_do_msdu(struct ieee80211_if_tdma *);

#endif
