/*
 * Some TDMA support code for mac80211.
 *
 * Copyright 2011-2013	Stanislav V. Korsakov <sta@stasoft.net>
*/

#ifndef	_PDWEXT_H
#define	_PDWEXT_H

#include "pnwext.h"
#include "../ieee80211_i.h"

#ifdef CPTCFG_MAC80211_TDMA

#define	TDMA_DEBUG	1

#ifdef TDMA_DEBUG

#define TDMA_DEBUG_SYNC	1
#undef TDMA_DEBUG_SYNC

#define TDMA_DEBUG_MESH	1
//#undef TDMA_DEBUG_MESH

#ifdef TDMA_DEBUG_MESH
//#define TDMA_DEBUG_MESH_TIME  1
//#define TDMA_DEBUG_MESH_TX    1
//#define TDMA_DEBUG_MESH_RX    1
//#define TDMA_DEBUG_MESH_PROCESSING    1
//#define TDMA_DEBUG_MESH_PROPAGATE_TX  1
//#define TDMA_DEBUG_MESH_PROPAGATE_RX  1
//#define TDMA_DEBUG_MESH_EXPIRATION    1
//#define TDMA_DEBUG_MESH_INTERNAL      1
//#define TDMA_DEBUG_MESH_ROUTE 1
#define TDMA_DEBUG_MESH_BCAST_EXPIRATION	1
#define TDMA_DEBUG_MESH_BCAST_GUARD	1
#endif

#define TDMA_DEBUG_TX	1
#undef TDMA_DEBUG_TX

#ifdef TDMA_DEBUG_TX

#define TDMA_DEBUG_TX_PROC	1
#define TDMA_DEBUG_TX_QUEUE_START_STOP	1

#endif

#define TDMA_DEBUG_BEACON	1

#ifdef TDMA_DEBUG_BEACON

#define TDMA_DEBUG_BEACON_TX	1
#undef TDMA_DEBUG_BEACON_TX

#define TDMA_DEBUG_BEACON_RX	1

#define TDMA_DEBUG_BEACON_RX_ALL	1
#undef TDMA_DEBUG_BEACON_RX_ALL

#endif

#define TDMA_DEBUG_HK	1
#undef TDMA_DEBUG_HK

#endif

static struct ieee80211_tx_info *tdma_skb_fill_info(struct ieee80211_sub_if_data *, struct sk_buff *);
static unsigned long tdma_tx_slot_calc(struct ieee80211_sub_if_data *, unsigned int);
static unsigned long tdma_max_clock_drift(struct ieee80211_sub_if_data *);
static void tdma_update_skb_hdr(struct ieee80211_if_tdma *, struct sk_buff *, struct sta_info *, int);
static unsigned long tdma_tx_slot_duration(struct ieee80211_if_tdma *);
static int tdma_adjust_rates(struct ieee80211_supported_band *, struct ieee80211_tx_rate *, int *, bool);
static u8 tdma_get_avail_slot(struct ieee80211_if_tdma *);
static bool tdma_is_tx_slot(struct ieee80211_if_tdma *);
static void ieee80211_tdma_tx_notify(struct ieee80211_vif *, bool);
static bool tdma_do_msdu(struct ieee80211_if_tdma *tdma);
static unsigned long tdma_tu_adjust(unsigned long, int, int, bool);
static void tdma_plan_next_tx(struct ieee80211_if_tdma *, unsigned long);
static unsigned long tdma_calc_ideal_interval(struct ieee80211_if_tdma *, int);
static bool tdma_store_pending_frame(struct tdma_neighbour *, struct sk_buff *);
static unsigned long tdma_get_tx_window(struct ieee80211_if_tdma *);
static bool tdma_sta_init(struct sta_info *);
static void tdma_process_pending_frames(struct sta_info *, u8, struct sk_buff_head *, struct sk_buff_head *);
static bool tdma_tx_can_aggregate(struct ieee80211_sub_if_data *, struct sta_info *, struct sk_buff *, struct sk_buff *);
static bool tdma_tx_aggr_skb(struct ieee80211_sub_if_data *, struct sk_buff *, struct sk_buff *, struct sk_buff_head *);
static unsigned long tdma_get_frame_duration_prognose(struct ieee80211_if_tdma *, int, struct ieee80211_tx_rate *, struct sk_buff *, struct sta_info **);

static void tdma_sched_tx(struct ieee80211_sub_if_data *, struct sk_buff *, enum nl80211_band);
static int tdma_check_break_tx(struct ieee80211_if_tdma *, struct sk_buff_head *, u8, unsigned long, unsigned long);
static void tdma_poll_timer(struct timer_list *t);
static void tdma_remove_ack_frame(struct ieee80211_sub_if_data *, u16);

static void tdma_amsdu_to_8023s(struct ieee80211_sub_if_data *, struct sk_buff *, struct sk_buff_head *, const unsigned int, __be16, u16);
static bool tdma_process_hdr(struct ieee80211_rx_data *, struct ieee80211_hdr *, bool *, u16 *, u16 *);
static struct sk_buff *tdma_close_frame(struct ieee80211_sub_if_data *, struct ieee80211_tx_control *);
static u32 tdma_adjust_basic_rates(struct ieee80211_sub_if_data *, int, u32);
static void tdma_setup_polling(struct ieee80211_if_tdma *);

/* TDMA originator tables handling code */
static bool tdma_originator_update_rx(struct ieee80211_if_tdma *, const u8 *, const u8 *, size_t, int);
static bool tdma_originator_get(struct ieee80211_if_tdma *, const u8 *, u8 *);
static int tdma_originator_expire(struct ieee80211_if_tdma *, long);
static void tdma_originator_get_processing(struct ieee80211_if_tdma *, const u8 *, const u8 *, bool *, bool *);
static u8 tdma_originator_put(struct ieee80211_if_tdma *, u8 *, const u8 *, size_t);
static void tdma_originator_update_tx(struct ieee80211_if_tdma *, const u8 *, u16);
static void tdma_originator_install_record(struct ieee80211_if_tdma *, const u8 *, const u8 *, u16, int);
static void tdma_originator_update_ack(struct ieee80211_if_tdma *, const u8 *, bool, size_t, int);

#ifdef CPTCFG_MAC80211_TDMA_MESH
static int tdma_retr_expire(struct ieee80211_if_tdma *, long);

#ifdef CPTCFG_MAC80211_DEBUGFS
static u16 tdma_mesh_calc_reachability(struct p_originator *, unsigned);
static void tdma_mesh_store_path(struct ieee80211_if_tdma *, struct p_originator *, u16);
static ssize_t tdma_mesh_fmt_path(const struct ieee80211_if_tdma *, char *, int);
#endif
#endif

#endif

#if IS_ENABLED(CPTCFG_MAC80211_COMPRESS)
static void mac80211_compress_uninit(struct ieee80211_sub_if_data *);
static void mac80211_compress_init(struct ieee80211_sub_if_data *);
static bool mac80211_tx_compress(struct ieee80211_sub_if_data *, struct sk_buff *);
static size_t decompress_wrapper(struct ieee80211_sub_if_data *sdata, char *in, size_t inlen, u8 compression);

#endif

#endif
