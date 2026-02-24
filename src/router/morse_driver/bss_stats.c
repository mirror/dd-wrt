/*
 * Copyright 2025 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "debug.h"
#include "bss_stats.h"
#include "vendor.h"

/**
 * Structure passed to station iterate func
 */
struct sta_stats_iter_data {
	const struct ieee80211_vif *vif;
	u16 num_stas_alloc;
	struct morse_evt_bss_stats *evt_data;
	u8 *next_sta_entry;
	size_t evt_data_len;
};

/* Add an inactive STA entry to the buffer */
#define ADD_INACTIVE_STA(ptr, mac_addr)                  \
	do {                                                 \
		*(ptr)++ = MORSE_STA_TYPE_INACTIVE;              \
		memcpy(ptr, mac_addr, ETH_ALEN);                        \
		(ptr) += ETH_ALEN;                                      \
	} while (0)

/* Get next STA entry in the buffer based on active STA or not */
#define NEXT_STA_ENTRY_PTR(ptr, is_active) \
	((is_active) \
	 ? ((u8 *)(ptr) + 1 + sizeof(struct morse_active_sta_stats)) \
	 : ((u8 *)(ptr) + 1 + sizeof(struct morse_inactive_sta_info)))

/**
 * prepare_sta_stats() - Prepare STA statistics. Called per active STA.
 *
 * @data: iterative func data
 * @sta: ieee80211 STA context
 */
static void prepare_sta_stats(void *data, struct morse_sta *msta)
{
	struct sta_stats_iter_data *iter_data = data;
	struct morse_vif *mors_vif =
		ieee80211_vif_to_morse_vif((struct ieee80211_vif *)iter_data->vif);
	const struct morse *mors = morse_vif_to_morse(mors_vif);
	struct ieee80211_sta *sta = morse_sta_to_ieee80211_sta(msta);
	struct morse_active_sta_stats *sta_stats;
	struct morse_bss_stats_sta *bs_sta;
	struct morse_sta_entry *sta_entry;
	u64 now, elapsed_ns;
	u32 total_tx_pkts = 0, total_rx_pkts = 0;
	int ac;
	u16 sta_index;

	if (!msta) {
		MORSE_WARN_ON(FEATURE_ID_RAW, 1);
		return;
	}

	if (msta->vif != iter_data->vif)
		return;
	bs_sta = &msta->bss_stats_sta;
	sta_index = iter_data->evt_data->num_stas;
	if (sta_index > iter_data->num_stas_alloc) {
		MORSE_DBG(mors, "no space for new STA %pM in stats\n",
		       msta->addr);
		return;
	}
	sta_entry = (struct morse_sta_entry *)iter_data->next_sta_entry;
	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++) {
		total_tx_pkts += bs_sta->stats->num_tx_pkts[ac];
		total_rx_pkts += bs_sta->stats->num_rx_pkts[ac];
	}
	iter_data->evt_data->num_stas++;
	/* Not an active STA, fill basic info */
	if (!total_tx_pkts && !total_rx_pkts) {
		sta_entry->is_active = false;
		memcpy(sta_entry->sta_info.mac_addr, msta->addr, ETH_ALEN);
		sta_entry->sta_info.raw_priority = msta->raw_priority;
		sta_entry->sta_info.avg_rssi = msta->avg_rssi;
		sta_entry->sta_info.aid = sta->aid;
		iter_data->next_sta_entry = NEXT_STA_ENTRY_PTR(sta_entry, false);
		iter_data->evt_data_len += 1 + sizeof(struct morse_inactive_sta_info);
		return;
	}

	iter_data->evt_data->num_active_stas++;
	sta_entry->is_active = true;
	sta_stats = &sta_entry->sta_stats;

	memcpy(sta_stats->num_tx_bytes, &bs_sta->stats->num_tx_bytes,
		sizeof(bs_sta->stats->num_tx_bytes));
	memcpy(sta_stats->num_rx_bytes, &bs_sta->stats->num_rx_bytes,
		sizeof(bs_sta->stats->num_rx_bytes));
	memcpy(sta_stats->mac_addr, msta->addr, ETH_ALEN);
	sta_stats->aid = sta->aid;
	sta_stats->avg_rssi = msta->avg_rssi;
	memcpy(sta_stats->num_tx_pkts, &bs_sta->stats->num_tx_pkts,
		sizeof(bs_sta->stats->num_tx_pkts));
	memcpy(sta_stats->num_rx_pkts, &bs_sta->stats->num_rx_pkts,
		sizeof(bs_sta->stats->num_rx_pkts));
	sta_stats->avg_tx_pkt_size = bs_sta->stats->avg_tx_pkt_size;
	sta_stats->avg_rx_pkt_size = bs_sta->stats->avg_rx_pkt_size;
	sta_stats->avg_tx_iat_us = bs_sta->stats->avg_tx_iat_us;
	sta_stats->avg_rx_iat_us = bs_sta->stats->avg_rx_iat_us;
	sta_stats->avg_tx_jitter_us = bs_sta->stats->avg_tx_jitter_us;
	sta_stats->avg_rx_jitter_us = bs_sta->stats->avg_rx_jitter_us;

	sta_stats->last_tx_rate_mcs = msta->last_sta_tx_rate.rate;
	sta_stats->last_tx_rate_kbps =
	    BPS_TO_KBPS(mmrc_calculate_theoretical_throughput(msta->last_sta_tx_rate));
	if (msta->last_rx.is_data_set) {
		struct morse_skb_rx_status *status = &msta->last_rx.data_status;

		msta->last_sta_rx_rate.guard = morse_ratecode_sgi_get(status->morse_ratecode);
		msta->last_sta_rx_rate.flags = morse_ratecode_rts_get(status->morse_ratecode);
		msta->last_sta_rx_rate.rate =
			morse_ratecode_mcs_index_get(status->morse_ratecode);
		msta->last_sta_rx_rate.ss =
			morse_ratecode_nss_index_get(status->morse_ratecode);
		msta->last_sta_rx_rate.bw =
			morse_ratecode_bw_index_get(status->morse_ratecode);

		sta_stats->last_rx_rate_mcs = msta->last_sta_rx_rate.rate;
		sta_stats->last_rx_rate_kbps =
			BPS_TO_KBPS(mmrc_calculate_theoretical_throughput(msta->last_sta_rx_rate));
	}
	sta_stats->num_tx_retries = bs_sta->stats->num_tx_retries;
	sta_stats->num_rx_retries = bs_sta->stats->num_rx_retries;
	now = ktime_get_ns();
	elapsed_ns = now - bs_sta->stats->last_reset_time;
	do_div(elapsed_ns, 1000);
	sta_stats->monitor_window_us = elapsed_ns;
	memset(bs_sta->stats, 0, sizeof(*bs_sta->stats));
	bs_sta->stats->last_reset_time = now;
	sta_stats->raw_priority = msta->raw_priority;
	iter_data->next_sta_entry = NEXT_STA_ENTRY_PTR(sta_entry, true);
	iter_data->evt_data_len += sizeof(struct morse_sta_entry);
}

/**
 * morse_bss_stats_timer_cb() - BSS stats timer callback
 *
 * @t: BSS Stats context
 */
#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
static void morse_bss_stats_timer_cb(unsigned long addr)
#else
static void morse_bss_stats_timer_cb(struct timer_list *t)
#endif
{
#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
	struct morse_bss_stats_context *bss_stats = (struct morse_bss_stats_context *)addr;
#else
	struct morse_bss_stats_context *bss_stats = from_timer(bss_stats, t, timer);
#endif
	struct morse_ap *ap = container_of(bss_stats, struct morse_ap, bss_stats);
	struct morse_vif *mors_vif = ap->mors_vif;
	struct sta_stats_iter_data iter_data;
	struct morse *mors = morse_vif_to_morse(mors_vif);
	size_t evt_data_len;
	struct list_head *pos;
	int ret;

	/* No STAs associated yet */
	if (!ap->num_stas)
		goto exit;

	iter_data.num_stas_alloc = ap->num_stas;
	iter_data.vif = morse_vif_to_ieee80211_vif(mors_vif);
	evt_data_len = sizeof(struct morse_evt_bss_stats) +
		(iter_data.num_stas_alloc * sizeof(struct morse_sta_entry));

	iter_data.evt_data = kzalloc(evt_data_len, GFP_ATOMIC);
	if (!iter_data.evt_data) {
		MORSE_ERR(mors, "Memory allocation for station stats failed\n");
		goto exit;
	}

	iter_data.next_sta_entry = iter_data.evt_data->data;
	iter_data.evt_data_len = sizeof(struct morse_evt_bss_stats);

	spin_lock_bh(&bss_stats->lock);
	list_for_each(pos, &bss_stats->stas) {
		struct morse_bss_stats_sta *sta_entry =
			container_of(pos, struct morse_bss_stats_sta, list);
		struct morse_sta *msta = container_of(sta_entry, struct morse_sta, bss_stats_sta);

		sta_entry->last_update = jiffies;

		prepare_sta_stats(&iter_data, msta);
	}
	spin_unlock_bh(&bss_stats->lock);

	ret = morse_vendor_send_bss_stats_event(morse_vif_to_ieee80211_vif(mors_vif),
						iter_data.evt_data, iter_data.evt_data_len);
	if (ret)
		MORSE_ERR(mors, "Failed to send station stats event :%d\n", ret);
	kfree(iter_data.evt_data);

exit:
	mod_timer(&bss_stats->timer, jiffies + msecs_to_jiffies(bss_stats->monitor_window_ms));
}

void morse_bss_stats_update_tx(struct ieee80211_vif *vif, struct sk_buff *skb,
				struct ieee80211_sta *sta, struct morse_skb_tx_status *tx_status,
				int tx_attempts)
{
	struct morse_sta *msta;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif((struct ieee80211_vif *)vif);
	struct morse *mors;
	struct morse_sta_stats *entry;
	size_t len;
	u16 tid;
	int ac;
	__le16 fc;
	u64 now_us, iat_us, jitter_us;

	if (vif->type != NL80211_IFTYPE_AP || !skb ||
		!morse_bss_stats_is_enabled(mors_vif))
		return;

	fc = ((struct ieee80211_hdr *)skb->data)->frame_control;
	/* Check for data packets */
	if (!sta || !ieee80211_is_data_qos(fc))
		return;

	msta = (struct morse_sta *)sta->drv_priv;
	mors = morse_vif_to_morse(mors_vif);
	tid = ieee80211_get_tid((struct ieee80211_hdr *)skb->data);
	ac = dot11_tid_to_ac(tid);
	if (ac >= IEEE80211_NUM_ACS) {
		MORSE_ERR(mors, "%s: Invalid AC:%d TID:%d\n", __func__, ac, tid);
		MORSE_WARN_ON(FEATURE_ID_DEFAULT, 1);
		return;
	}
	len = skb->len;
	entry = msta->bss_stats_sta.stats;
	entry->num_tx_bytes[ac] += len;
	entry->num_tx_pkts[ac]++;
	entry->avg_tx_pkt_size = ema_update_u32(entry->avg_tx_pkt_size, len);
	entry->num_tx_retries += tx_attempts - 1;

	now_us = div_u64(ktime_get_ns(), 1000);
	if (entry->num_tx_pkts[ac] <= 1) {
		entry->last_tx_timestamp_us = now_us;
		return;
	}
	/* Calculate Inter-Packet Arrival Time (IAT) in microseconds */
	iat_us = now_us - entry->last_tx_timestamp_us;

	/* Calculate Jitter as absolute difference from last IAT */
	jitter_us = (iat_us > entry->last_tx_iat_us) ?
					(iat_us - entry->last_tx_iat_us) :
					(entry->last_tx_iat_us - iat_us);

	/* Update average IAT & jitter */
	entry->avg_tx_jitter_us =
		ema_update_u32(entry->avg_tx_jitter_us, jitter_us);

	entry->last_tx_iat_us = iat_us;
	entry->avg_tx_iat_us = ema_update_u32(entry->avg_tx_iat_us, iat_us);
	entry->last_tx_timestamp_us = now_us;
}

void morse_bss_stats_update_rx(struct ieee80211_vif *vif, struct sk_buff *skb,
				 struct ieee80211_sta *sta, struct morse_skb_rx_status *rx_status)
{
	struct morse_sta *msta = (struct morse_sta *)sta->drv_priv;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	struct morse *mors = morse_vif_to_morse(mors_vif);
	struct morse_sta_stats *entry = msta->bss_stats_sta.stats;
	size_t len;
	u16 tid;
	int ac;
	__le16 fc;
	u64 iat_us, jitter_us;

	if (vif->type != NL80211_IFTYPE_AP ||
		!morse_bss_stats_is_enabled(mors_vif))
		return;

	fc = ((struct ieee80211_hdr *)skb->data)->frame_control;
	if (!ieee80211_is_data_qos(fc))
		return;

	tid = ieee80211_get_tid((struct ieee80211_hdr *)skb->data);
	ac = dot11_tid_to_ac(tid);
	if (ac >= IEEE80211_NUM_ACS) {
		MORSE_ERR(mors, "%s: Invalid AC:%d TID:%d\n", __func__, ac, tid);
		MORSE_WARN_ON(FEATURE_ID_DEFAULT, 1);
		return;
	}

	len = skb->len;
	entry->num_rx_bytes[ac] += len;
	entry->num_rx_pkts[ac]++;
	entry->avg_rx_pkt_size = ema_update_u32(entry->avg_rx_pkt_size, len);

	if (entry->num_rx_pkts[ac] <= 1) {
		entry->last_rx_timestamp_us = le64_to_cpu(rx_status->rx_timestamp_us);
		return;
	}

	/* Calculate Inter-Packet Arrival Time (IAT) and Jitter */
	iat_us = le64_to_cpu(rx_status->rx_timestamp_us) -
					entry->last_rx_timestamp_us;

	jitter_us = (iat_us > entry->last_rx_iat_us) ?
			(iat_us - entry->last_rx_iat_us) :
			(entry->last_rx_iat_us - iat_us);
	entry->last_rx_iat_us = iat_us;
	if (le16_to_cpu(fc) & IEEE80211_FCTL_RETRY)
		entry->num_rx_retries++;
	/* Update average IAT & jitter */
	entry->avg_rx_iat_us = ema_update_u32(entry->avg_rx_iat_us, iat_us);
	entry->avg_rx_jitter_us =
		ema_update_u32(entry->avg_rx_jitter_us, jitter_us);
	entry->last_rx_timestamp_us = le64_to_cpu(rx_status->rx_timestamp_us);
}

/**
 * morse_bss_stats_remove_sta() - Remove STA from BSS stats list
 *
 * @vif: pointer to ieee80211_vif
 * @sta: pointer to ieee80211_sta
 */
static void morse_bss_stats_remove_sta(struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct morse_sta *msta = (struct morse_sta *)sta->drv_priv;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	struct morse_bss_stats_context *bss_stats;

	if (!mors_vif || !mors_vif->ap)
		return;

	bss_stats = &mors_vif->ap->bss_stats;
	spin_lock_bh(&bss_stats->lock);
	if (msta->bss_stats_sta.stats) {
		list_del_init(&msta->bss_stats_sta.list);
		kfree(msta->bss_stats_sta.stats);
		msta->bss_stats_sta.stats = NULL;
	}
	spin_unlock_bh(&bss_stats->lock);
}

/**
 * morse_bss_stats_add_sta() - Add STA to BSS stats list
 *
 * @vif: pointer to ieee80211_vif
 * @sta: pointer to ieee80211_sta
 *
 * Return: 0 if command was processed successfully, otherwise error code
 */
static int morse_bss_stats_add_sta(struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct morse_sta *msta = (struct morse_sta *)sta->drv_priv;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	struct morse_bss_stats_context *bss_stats;
	struct morse_sta_stats *stats;

	if (!mors_vif || !mors_vif->ap)
		return -EINVAL;
	bss_stats = &mors_vif->ap->bss_stats;

	stats = kzalloc(sizeof(*stats), GFP_KERNEL);

	spin_lock_bh(&bss_stats->lock);

	kfree(msta->bss_stats_sta.stats);
	msta->bss_stats_sta.stats = stats;
	list_add(&msta->bss_stats_sta.list, &bss_stats->stas);
	msta->bss_stats_sta.last_update = jiffies;

	spin_unlock_bh(&bss_stats->lock);

	return 0;
}

void morse_bss_stats_update_sta_state(struct morse *mors,
			      struct ieee80211_vif *vif, struct ieee80211_sta *sta,
			      enum ieee80211_sta_state old_state,
			      enum ieee80211_sta_state new_state)
{
	struct morse_sta *msta = (struct morse_sta *)sta->drv_priv;

	if (vif->type != NL80211_IFTYPE_AP)
		return;

	/* Update Morse BSS Stats STA list */
	if (old_state < new_state && new_state == IEEE80211_STA_ASSOC) {
		/* Newly associated, add to BSS stats list */
		morse_bss_stats_add_sta(vif, sta);
	} else if (old_state > new_state &&
		   (old_state == IEEE80211_STA_ASSOC || old_state == IEEE80211_STA_AUTH)) {
		/* Lost or failed association; remove from list */
		morse_bss_stats_remove_sta(vif, sta);
	} else if (old_state < new_state &&
		   old_state == IEEE80211_STA_NONE &&
		   msta->bss_stats_sta.list.prev) {
		/* Special case for driver warning issue causing a sta to be left on the list */
		MORSE_INFO(mors, "Remove stale sta from BSS stats list\n");
		morse_bss_stats_remove_sta(vif, sta);
	}
}

/**
 * @brief Checks if bss stats can be enbaled based on the satisfying conditions.
 *
 * @mors_vif:	Morse VIF.
 * @bss_stats:	BSS stats struct.
 *
 * @return 0 on success, else error code
 */
static bool morse_bss_stats_can_be_enabled(struct morse_vif *mors_vif)
{
	struct morse_raw *raw = &mors_vif->ap->raw;

	/* List of conditions at which bss stats can be enabled. */
	return (raw && !morse_raw_has_static_config(raw));
}

/**
 * @brief Removes all the stations from the BSS stats list.
 *
 * @mors_vif:	Morse VIF.
 * @bss_stats:	BSS stats struct.
 *
 * @return 0 on success, else error code
 */
static int morse_bss_stats_remove_all(struct morse_vif *mors_vif,
				struct morse_bss_stats_context *bss_stats)
{
	struct morse_bss_stats_sta *bs_sta, *temp;
	struct morse_sta *msta;

	list_for_each_entry_safe(bs_sta, temp, &bss_stats->stas, list) {
		msta = container_of(bs_sta, struct morse_sta, bss_stats_sta);
		morse_bss_stats_remove_sta(morse_vif_to_ieee80211_vif(mors_vif),
				morse_sta_to_ieee80211_sta(msta));
	}

	return 0;
}

int morse_bss_stats_pause(struct morse_vif *mors_vif)
{
	struct morse_bss_stats_context *bss_stats;
	struct morse *mors;

	if (!mors_vif || !mors_vif->ap)
		return -ENOTSUPP;

	mors = morse_vif_to_morse(mors_vif);
	bss_stats = &mors_vif->ap->bss_stats;

	if (!bss_stats->enabled || bss_stats->paused) {
		MORSE_DBG(mors, "%s: BSS stats not enabled or already paused %d, %d\n",
				__func__, bss_stats->enabled, bss_stats->paused);
		return -EINVAL;
	}

	/* disable and stop the stats timer */
	bss_stats->paused = true;
	del_timer_sync(&bss_stats->timer);

	return 0;
}

int morse_bss_stats_resume(struct morse_vif *mors_vif)
{
	struct morse_bss_stats_context *bss_stats;
	struct morse *mors;

	if (!mors_vif || !mors_vif->ap)
		return -ENOTSUPP;

	mors = morse_vif_to_morse(mors_vif);
	bss_stats = &mors_vif->ap->bss_stats;

	if (!bss_stats->enabled)
		return -EINVAL;

	if (!bss_stats->paused) {
		MORSE_DBG(mors, "%s: BSS Stats already resumed\n", __func__);
		return -EINVAL;
	}

	if (!bss_stats->monitor_window_ms) {
		MORSE_ERR(mors, "%s: Resume failed. Monitor window is not configured\n", __func__);
		return -EINVAL;
	}
	/* enable and restart the stats timer */
	bss_stats->paused = false;
	mod_timer(&bss_stats->timer,
			jiffies + msecs_to_jiffies(bss_stats->monitor_window_ms));

	return 0;
}

int morse_cmd_process_bss_stats_conf(struct morse_vif *mors_vif,
			struct morse_cmd_req_config_bss_stats *config)
{
	struct morse_bss_stats_context *bss_stats = &mors_vif->ap->bss_stats;

	if (le32_to_cpu(config->monitor_window_ms) < MIN_BSS_STATS_MONITOR_WINDOW_MS ||
		le32_to_cpu(config->monitor_window_ms) > MAX_BSS_STATS_MONITOR_WINDOW_MS)
		return -EINVAL;

	bss_stats->monitor_window_ms = config->enable ?
			le32_to_cpu(config->monitor_window_ms) : 0;

	if (!morse_bss_stats_can_be_enabled(mors_vif))
		return 0;

	bss_stats->enabled = config->enable;
	bss_stats->paused = !config->enable;

	if (config->enable)
		mod_timer(&bss_stats->timer,
			jiffies + msecs_to_jiffies(bss_stats->monitor_window_ms));
	else
		del_timer_sync(&bss_stats->timer);

	return 0;
}

bool morse_bss_stats_is_enabled(struct morse_vif *mors_vif)
{
	return mors_vif && mors_vif->ap && mors_vif->ap->bss_stats.enabled;
}

int morse_bss_stats_init(struct morse_vif *mors_vif)
{
	struct morse_bss_stats_context *bss_stats;
	struct morse *mors;

	if (!mors_vif || !mors_vif->ap)
		return -ENOTSUPP;

	mors = morse_vif_to_morse(mors_vif);
	bss_stats = &mors_vif->ap->bss_stats;

	memset(bss_stats, 0, sizeof(*bss_stats));

	spin_lock_init(&bss_stats->lock);
	INIT_LIST_HEAD(&bss_stats->stas);
	bss_stats->mors = mors;
	bss_stats->mors_vif = mors_vif;
	bss_stats->monitor_window_ms = DEFAULT_BSS_STATS_MONITOR_WINDOW_MS;

#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
	init_timer(&bss_stats->timer);
	bss_stats->timer.data = (unsigned long)bss_stats;
	bss_stats->timer.function = morse_bss_stats_timer_cb;
	add_timer(&bss_stats->timer);
#else
	timer_setup(&bss_stats->timer, morse_bss_stats_timer_cb, 0);
#endif

	return 0;
}

void morse_bss_stats_deinit(struct morse_vif *mors_vif)
{
	struct morse_bss_stats_context *bss_stats;

	if (!mors_vif || !mors_vif->ap)
		return;

	bss_stats = &mors_vif->ap->bss_stats;
	morse_bss_stats_remove_all(mors_vif, bss_stats);
	del_timer_sync(&bss_stats->timer);
}
