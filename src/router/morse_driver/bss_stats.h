/*
 * Copyright 2025 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef _MORSE_BSS_STATS_H_
#define _MORSE_BSS_STATS_H_

#include "morse.h"
#include "linux/list.h"

/** Default BSS Statistics event period in msec */
#define DEFAULT_BSS_STATS_MONITOR_WINDOW_MS 1000
/** Minimum BSS Statistics event period in msec */
#define MIN_BSS_STATS_MONITOR_WINDOW_MS 500
/** Maximum BSS Statistics event period in msec */
#define MAX_BSS_STATS_MONITOR_WINDOW_MS 20000
/** Inactive STA, without traffic in monitoring window */
#define MORSE_STA_TYPE_INACTIVE 0
/** Inactive STA, with traffic in monitoring window */
#define MORSE_STA_TYPE_ACTIVE   1

/** Station statistics info */
struct morse_sta_stats {
	u32 num_tx_bytes[IEEE80211_NUM_ACS];
	u32 num_rx_bytes[IEEE80211_NUM_ACS];
	u32 num_tx_pkts[IEEE80211_NUM_ACS];
	u32 num_rx_pkts[IEEE80211_NUM_ACS];
	u32 avg_tx_pkt_size;
	u32 avg_rx_pkt_size;
	u32 avg_tx_iat_us;
	u32 avg_rx_iat_us;
	u32 avg_tx_jitter_us;
	u32 avg_rx_jitter_us;
	u32 last_tx_iat_us;
	u32 last_rx_iat_us;
	u64 last_tx_timestamp_us;
	u64 last_rx_timestamp_us;
	u32 num_tx_retries;
	u32 num_rx_retries;
	u64 last_reset_time;
};

/** BSS statistics context */
struct morse_bss_stats_context {
    /** Flag to keep track of enable/disable  */
	bool enabled;
    /** Flag to keep track of pause/resume  */
	bool paused;
    /** Serialize stats update and timer functions */
	spinlock_t lock;
    /** Stations List */
	struct list_head stas;
    /** Station statistics reporting timer */
	struct timer_list timer;
    /** Statistics event interval */
	u32 monitor_window_ms;
    /** back pointer to morse */
	struct morse *mors;
    /** back pointer to morse_vif */
	struct morse_vif *mors_vif;
};

/** BSS statistics STA specific context */
struct morse_bss_stats_sta {
	struct morse_sta_stats *stats;
	struct list_head list;
	unsigned long last_update;
};

/** Morse event data for active STA statistics */
struct morse_active_sta_stats {
	u8 mac_addr[ETH_ALEN];
	u8 raw_priority;
	u16 aid;
	s16 avg_rssi;
	u32 num_tx_bytes[IEEE80211_NUM_ACS];
	u32 num_rx_bytes[IEEE80211_NUM_ACS];
	u32 num_tx_pkts[IEEE80211_NUM_ACS];
	u32 num_rx_pkts[IEEE80211_NUM_ACS];
	u32 avg_tx_pkt_size;
	u32 avg_rx_pkt_size;
	u32 avg_tx_iat_us;
	u32 avg_rx_iat_us;
	u32 avg_tx_jitter_us;
	u32 avg_rx_jitter_us;
	u32 num_tx_retries;
	u32 num_rx_retries;
	u8 last_tx_rate_mcs;
	u8 last_rx_rate_mcs;
	u32 last_tx_rate_kbps;
	u32 last_rx_rate_kbps;
	u32 monitor_window_us;
} __packed;

/** Morse event data for inactive STA statistics */
struct morse_inactive_sta_info {
	u8 mac_addr[ETH_ALEN];
	u8 raw_priority;
	u16 aid;
	s16 avg_rssi;
} __packed;

/** Morse station entry data for BSS statistics */
struct morse_sta_entry {
	/* 0 = inactive (basic info), 1 = active (full stats) */
	u8 is_active;
	union {
		struct morse_inactive_sta_info sta_info;
		struct morse_active_sta_stats sta_stats;
	};
} __packed;

/** Morse event data for BSS statistics for all STAs */
struct morse_evt_bss_stats {
	/* Num of connected STAs */
	u16 num_stas;
	/* Num of active STAs */
	u16 num_active_stas;
	/* variable STA entry data */
	u8 data[];
} __packed;

/**
 * morse_bss_stats_init() - Initialize BSS stats
 *
 * @mors_vif: Morse VIF structure
 *
 * Return: 0 - OK
 */
int morse_bss_stats_init(struct morse_vif *mors_vif);

/**
 * morse_bss_stats_deinit() - Clean up BSS stats
 *
 * @mors_vif: Morse VIF structure
 */
void morse_bss_stats_deinit(struct morse_vif *mors_vif);

/**
 * morse_bss_stats_is_enabled() - Is BSS stats module enabled to pass up stats
 *
 * @mors_vif: Morse VIF
 *
 * Return: true if enabled, otherwise false
 */
bool morse_bss_stats_is_enabled(struct morse_vif *mors_vif);

/**
 * morse_bss_stats_update_tx() - Update STA statistics for Tx
 *
 * @vif: pointer to ieee80211_vif
 * @skb: Tx skbuff
 * @sta: pointer to ieee80211_sta
 * @tx_status: Tx status
 * @tx_attempts: Number of Tx attempts
 */
void morse_bss_stats_update_tx(struct ieee80211_vif *vif, struct sk_buff *skb,
				struct ieee80211_sta *sta, struct morse_skb_tx_status *tx_status,
				int tx_attempts);

/**
 * morse_bss_stats_update_rx() - Update STA statistics for Rx
 *
 * @vif: pointer to ieee80211_vif
 * @skb: Rx skbuff
 * @sta: pointer to ieee80211_sta
 * @rx_status: Rx status
 */
void morse_bss_stats_update_rx(struct ieee80211_vif *vif, struct sk_buff *skb,
				struct ieee80211_sta *sta, struct morse_skb_rx_status *rx_status);

/**
 * morse_bss_stats_update_sta_state() - Add/remove STA from the BSS stats STA list
 *
 * @mors: Global morse struct
 * @mors_vif: Morse VIF
 * @sta: pointer to ieee80211_sta
 * @old_state: Old state of STA
 * @new_state: New state of STA
 */
void morse_bss_stats_update_sta_state(struct morse *mors,
			      struct ieee80211_vif *vif, struct ieee80211_sta *sta,
			      enum ieee80211_sta_state old_state,
			      enum ieee80211_sta_state new_state);

/**
 * morse_cmd_process_bss_stats_conf() - Stats config to enable and configure interval
 *
 * @mors_vif: Morse VIF
 * @config: pointer to BSS stats config
 *
 * Return: 0 if command was processed successfully, otherwise error code
 */
int morse_cmd_process_bss_stats_conf(struct morse_vif *mors_vif,
				  struct morse_cmd_req_config_bss_stats *config);

/**
 * morse_bss_stats_pause() - Pause BSS Stats module, if it's enabled
 *
 * @mors_vif: Morse VIF
 *
 * Return: 0 if processed successfully, otherwise error code
 */
int morse_bss_stats_pause(struct morse_vif *mors_vif);

/**
 * morse_bss_stats_resume() - Resume BSS Stats module, if it's enabled
 *
 * @mors_vif: Morse VIF
 *
 * Return: 0 if processed successfully, otherwise error code
 */
int morse_bss_stats_resume(struct morse_vif *mors_vif);

#endif /* !_MORSE_BSS_STATS_H_ */
