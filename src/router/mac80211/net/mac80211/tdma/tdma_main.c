/*
 * Some TDMA support code for mac80211.
 *
 * Copyright 2011-2013	Stanislav V. Korsakov <sta@stasoft.net>
*/

#include <linux/export.h>
#include <linux/delay.h>
#include <linux/cpumask.h>
#include "tdma_i.h"
#include "../driver-ops.h"
#include "../wme.h"

static ieee80211_tx_result debug_noinline ieee80211_tx_h_rate_ctrl(struct ieee80211_tx_data *);
static ieee80211_tx_result debug_noinline ieee80211_tx_h_select_key(struct ieee80211_tx_data *);
static ieee80211_tx_result debug_noinline ieee80211_tx_h_encrypt(struct ieee80211_tx_data *);
static ieee80211_tx_result ieee80211_tx_h_michael_mic_add(struct ieee80211_tx_data *);

static unsigned long tdma_get_frame_duration(struct ieee80211_if_tdma *tdma, struct sk_buff *skb, struct sta_info **sta, bool do_all)
{
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);
	unsigned long duration = 0;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	bool ack;
	int erp = 0, bitrate;
	struct ieee80211_supported_band *sband = sdata->local->hw.wiphy->bands[info->band];
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;

	if (do_all) {
		bool brdcast, qos = ieee80211_is_data_qos(hdr->frame_control);

		bitrate = tdma->rate;
#ifdef CPTCFG_MAC80211_TDMA_MESH
		if (TDMA_IS_MESHED(tdma) && ieee80211_is_data_present(hdr->frame_control)) {
			brdcast = tdma_originator_get(tdma, hdr->addr3, hdr->addr1);
		} else
#endif
			brdcast = is_multicast_ether_addr(hdr->addr1);
		if (!brdcast) {
			if (*sta == NULL) {
				rcu_read_lock();
				if ((*sta = tdma->tdma_sta_info_get(sdata, hdr->addr1)) == NULL) {
					eth_broadcast_addr(hdr->addr1);
					brdcast = true;
				}
				rcu_read_unlock();
			} else if (hdr->duration_id != 0) {
				duration = (unsigned long)le16_to_cpu(hdr->duration_id);
				if (!qos)
					duration += (duration << 1);
				return duration;
			}
		}
		info->flags &= ~(IEEE80211_TX_CTL_NO_ACK | IEEE80211_TX_UNICAST | IEEE80211_TX_CTL_REQ_TX_STATUS);
		if (brdcast || (TDMA_CFG_VERSION(tdma) == 0)) {
			info->control.rates[0].count = 1;
			info->control.rates[0].idx = tdma->cur_rate;
			info->flags |= IEEE80211_TX_CTL_NO_ACK;
		} else {
			struct ieee80211_tx_data tx;

			info->flags |= IEEE80211_TX_UNICAST;
			memset(&tx, 0, sizeof(tx));
			tx.sdata = sdata;
			tx.local = sdata->local;
			tx.skb = skb;
			tx.sta = *sta;
			__skb_queue_head_init(&tx.skbs);
			if (ieee80211_tx_h_rate_ctrl(&tx) != TX_CONTINUE)
				return duration;
			if (qos)
				info->flags |= IEEE80211_TX_CTL_NO_ACK;
			else
				info->flags |= IEEE80211_TX_CTL_REQ_TX_STATUS;
			memcpy(&info->control.rates[0], &tx.sta->tx_stats.last_rate, sizeof(info->control.rates[0]));
			bitrate = tdma_adjust_rates(sband, &info->control.rates[0], &erp, !qos);
			if (bitrate < tdma->rate) {
				bitrate = tdma->rate;
				info->control.rates[0].idx = tdma->cur_rate;
			}
		}
		info->control.rates[1].idx = -1;
		ack = !(brdcast || qos);
	} else {
		ack = !!(info->control.rates[0].count > 1);
		bitrate = tdma_adjust_rates(sband, &info->control.rates[0], &erp, ack);
	}
	duration = tdma->tdma_frame_duration(sband->band, skb->len, bitrate, erp, sdata->vif.bss_conf.use_short_preamble, tdma->tdma_chandef_get_shift(&tdma->chandef));
	duration += TDMA_TX_TAIL_SPACE;
	if (ack)
		duration += tdma->ack_duration;
	hdr->duration_id = cpu_to_le16((u16)duration);
	if (ack)
		duration += (duration << 1);
	return duration;
}

static struct sk_buff *tdma_get_skb_from_queue(struct ieee80211_if_tdma *tdma, struct sk_buff_head *txlist, unsigned long window, unsigned long txs, u8 ver, unsigned long *duration)
{
	struct sk_buff *skb = NULL, *tmp, *from = NULL;
	bool removed = false, no_reorder = TDMA_CFG_NO_REORDER(tdma);
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);
	struct sta_info *sta = rcu_dereference(tdma->sta);

tgsfq_in:
	*duration = 0;
	skb_queue_walk_safe(txlist, skb, tmp) {
		*duration = tdma_get_frame_duration(tdma, skb, &sta, true);
		if (txs <= (window + *duration)) {
#ifdef TDMA_DEBUG_TX_PROC
			printk("TDMA: Not enough time to do TX - %lu < %lu. Skb duration is %lu, size is - %u. In Q %u\n", window, txs, *duration, skb->len, skb_queue_len(txlist));
#endif
			if (no_reorder || (txs <= window + TDMA_TX_TAIL_SPACE))
				break;
			continue;
		}
		from = tmp;
		spin_lock_bh(&txlist->lock);
		__skb_unlink(skb, txlist);
		spin_unlock_bh(&txlist->lock);
		removed = true;
		break;
	}
	if (removed) {
		struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

		if (*duration == 0) {
			I802_DEBUG_INC(sdata->local->tx_handlers_drop);
			ieee80211_free_txskb(&sdata->local->hw, skb);
			goto tgsfq_in;
		}
#ifdef CPTCFG_MAC80211_TDMA_MESH
		if (TDMA_IS_MESHED(tdma)) {
			bool no_ack = !!(info->flags & IEEE80211_TX_CTL_NO_ACK);

			if (no_ack) {
				info->control.rts_cts_rate_idx = -1;
				if (unlikely(info->ack_frame_id)) {
					tdma_remove_ack_frame(sdata, info->ack_frame_id);
					info->ack_frame_id = 0;
				}
			}
			if (!no_ack) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0))
				if (skb->sk && (skb_shinfo(skb)->tx_flags & SKBTX_WIFI_STATUS) && (info->ack_frame_id == 0)) {
					int id;
					unsigned long flags;

					spin_lock_irqsave(&sdata->local->ack_status_lock, flags);
					id = idr_alloc(&sdata->local->ack_status_frames, skb, 1, 0x10000, GFP_ATOMIC);
					spin_unlock_irqrestore(&sdata->local->ack_status_lock, flags);

					if (id > 0) {
						info->ack_frame_id = id;
						info->flags |= IEEE80211_TX_CTL_REQ_TX_STATUS;
					}
				}
#endif
			}
		}
#endif
		if (tdma_do_msdu(tdma) && (from != NULL)) {
			unsigned long dur;
			struct sta_info *sta1;

			skb_queue_walk_from_safe(txlist, from, tmp) {
				dur = tdma_get_frame_duration_prognose(tdma, from->len + skb->len, &info->control.rates[0], from, &sta1);
				if ((sta == sta1) && ((dur + window) < txs) && tdma_tx_can_aggregate(sdata, sta, skb, from)) {
					if (tdma_tx_aggr_skb(sdata, skb, from, txlist))
						*duration = tdma_get_frame_duration(tdma, skb, &sta, false);
					else if (no_reorder)
						break;
				}
			}
		}
#ifdef TDMA_DEBUG_TX_PROC
		printk("TDMA: TX - %d, Used slot time - %lu. Slot size - %lu. Skb duration is %lu, size is - %u. In Q %u\n", tdma->last_tx, window, txs, *duration, skb->len, skb_queue_len(txlist));
#endif
	}
	return (removed ? skb : NULL);
}

static bool tdma_do_tx(struct ieee80211_if_tdma *tdma, struct sk_buff *skb, unsigned long txs, unsigned long duration, unsigned long *window, bool gack, u8 ver, struct sta_info *sta, unsigned long idx)
{
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	bool do_break = false, optimize_tx = !!(tdma->optimize_tx && TDMA_CFG_POLLING(tdma));
	struct ieee80211_sta *ista = NULL;
	struct ieee80211_tx_control control;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	ieee80211_tx_result res = TX_CONTINUE;
	struct ieee80211_tx_data tx;

	if (sta == NULL) {
		if ((ver == 2) && gack)
			optimize_tx = false;
		rcu_read_lock();
		sta = tdma->tdma_sta_info_get(sdata, hdr->addr1);
		rcu_read_unlock();
	}
	if (sta) {
#ifdef CPTCFG_MAC80211_TDMA_MESH
		if (TDMA_IS_MESHED(tdma)) {
			if (ether_addr_equal(hdr->addr1, hdr->addr4)) {
				I802_DEBUG_INC(sdata->local->tx_handlers_drop);
				ieee80211_free_txskb(&sdata->local->hw, skb);
				return false;
			}
			tdma_originator_update_tx(tdma, hdr->addr1, duration);
		}
#endif
		sta->tx_stats.bytes[IEEE80211_AC_BE] += skb->len;
		sta->tx_stats.packets[IEEE80211_AC_BE]++;
		ista = &sta->sta;
		do_break = !(gack || (ver == 0));
		if (!do_break) {
			u16 *seq = &sta->tid_seq[0];

			hdr->seq_ctrl = cpu_to_le16(*seq);
			*seq = (*seq + 0x10) & IEEE80211_SCTL_SEQ;
		} else {
			info->flags |= IEEE80211_TX_CTL_ASSIGN_SEQ;
			hdr->seq_ctrl = cpu_to_le16(sdata->sequence_number);
			sdata->sequence_number += 0x10;
		}
		if (gack)
			ptdma_update_hdr_counter(tdma, skb, hdr);
		txs -= duration;
		if (optimize_tx && ((idx == 1) || (tdma->last_tx >= 24)) && (*window >= tdma->poll_interval) && ((txs - *window) > 1024)) {
			tdma->last_tx_start = 0;
			del_timer_sync(&tdma->poll_timer);
			ptdma_update_hdr_info(tdma, skb, hdr, true);
			do_break = true;
		}
	} else {
		info->flags |= IEEE80211_TX_CTL_ASSIGN_SEQ;
		hdr->seq_ctrl = cpu_to_le16(sdata->sequence_number);
		sdata->sequence_number += 0x10;
	}

	memset(&tx, 0, sizeof(tx));
	tx.sta = sta;
	tx.skb = skb;
	tx.local = sdata->local;
	tx.sdata = sdata;
	__skb_queue_head_init(&tx.skbs);

#define CALL_TXH(txh) \
	do {				\
		res = txh(&tx);		\
		if (res != TX_CONTINUE)	\
			goto txh_done;	\
	} while (0)

	CALL_TXH(ieee80211_tx_h_select_key);
	CALL_TXH(ieee80211_tx_h_michael_mic_add);
	CALL_TXH(ieee80211_tx_h_encrypt);
#undef CALL_TXH

txh_done:
	if (unlikely(res == TX_DROP)) {
		I802_DEBUG_INC(tx.local->tx_handlers_drop);
		if (tx.skb)
			ieee80211_free_txskb(&tx.local->hw, tx.skb);
		else
			ieee80211_purge_tx_queue(&tx.local->hw, &tx.skbs);
		return false;
	}
	if (!do_break) {
		tdma->last_tx_call += duration;
		*window += duration;
	}
	control.sta = ista;
	netif_trans_update(sdata->dev);
	sdata->local->ops->tx(&sdata->local->hw, &control, skb);
	tdma->last_tx++;
	return do_break;
}

static void tdma_tx_first_step(struct ieee80211_if_tdma *tdma)
{
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);

	if ((tdma->last_tx == 0) && (tdma->last_tx_start > 0)) {
		struct sk_buff *skb;

#ifdef TDMA_DEBUG_TX
		printk("TDMA: (%d) Started TX at %lu\n", sdata->vif.bss_conf.aid, tdma->last_tx_start);
#endif
		del_timer_sync(&tdma->poll_timer);
		if ((skb = tdma_create_beacon(sdata)) != NULL) {
			struct ieee80211_tx_control control;

			tdma->last_tx++;
			control.sta = NULL;
			netif_trans_update(sdata->dev);
			drv_tx(sdata->local, &control, skb);
			if (tdma->optimize_tx && TDMA_CFG_POLLING(tdma))
				mod_timer(&tdma->poll_timer, jiffies + tdma->poll_timer_interval);
		}
	}
}

#ifdef TDMA_TX_TASKLET
static void tdma_tx(unsigned long data)
{
	struct ieee80211_if_tdma *tdma = (struct ieee80211_if_tdma *)data;
#else
static void tdma_tx(struct work_struct *work)
{
	struct ieee80211_if_tdma *tdma = container_of(work, struct ieee80211_if_tdma, tx_work);
#endif
	struct sk_buff *skb;
	int idx;
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);
	u8 ver = TDMA_CFG_VERSION(tdma);
	unsigned long window = 0, duration = 0, txs = tdma_tx_slot_duration(tdma);
	bool do_break = false, gack = TDMA_CFG_GACK(tdma);
	struct sta_info *sta = rcu_dereference(tdma->sta);

	tdma_tx_first_step(tdma);
	while (!do_break) {
		if (!TDMA_JOINED(tdma) || (TDMA_STATE(tdma) != TDMA_STATUS_ASSOCIATED) || (tdma->last_beacon >= tdma->last_tx_start) || (tdma->last_tx_start == 0)) {
			tdma->last_tx_start = 0;
			return;
		}
		if (((ver == 0) || gack) && (tdma->last_tx >= TDMA_MAX_TX_PER_ROUND))
			return;
		if ((window = tdma_get_tx_window(tdma)) >= txs)
			return;
		if ((idx = tdma_check_break_tx(tdma, &tdma->tx_skb_list, ver, txs, window)) == 0)
			return;
		if (ieee80211_queue_stopped(&sdata->local->hw, IEEE80211_AC_BE) && ((window + 512) < txs)) {
			ieee80211_tdma_tx_notify(&sdata->vif, true);
			return;
		}
		if ((skb = tdma_get_skb_from_queue(tdma, &tdma->tx_skb_list, window, txs, ver, &duration)) != NULL)
			do_break = tdma_do_tx(tdma, skb, txs, duration, &window, gack, ver, sta, idx);
		else {
			tdma_check_break_tx(tdma, NULL, ver, txs, window);
			break;
		}
	}
}

#ifdef TDMA_TX_TASKLET
static void tdma_tx2(unsigned long data)
{
	struct ieee80211_if_tdma *tdma = (struct ieee80211_if_tdma *)data;
#else
static void tdma_tx2(struct work_struct *work)
{
	struct ieee80211_if_tdma *tdma = container_of(work, struct ieee80211_if_tdma, tx_work);
#endif
	struct sk_buff *skb;
	unsigned long window = 0, txs, idx;
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);
	unsigned long duration = 0;
	struct sta_info *sta;
	u8 aid;
	bool found = false;

	txs = tdma_tx_slot_duration(tdma);
	tdma_tx_first_step(tdma);
	aid = tdma->b_counter;
	if (aid == 0)
		aid = tdma->node_num;
	else
		aid++;
	while ((idx = skb_queue_len(&tdma->tx_skb_list)) > 0) {
		if (!TDMA_JOINED(tdma) || (TDMA_STATE(tdma) != TDMA_STATUS_ASSOCIATED) || (tdma->last_beacon >= tdma->last_tx_start) || (tdma->last_tx_start == 0))
			return;
		if (tdma->last_tx >= TDMA_MAX_TX_PER_ROUND)
			return;
		if ((window = tdma_get_tx_window(tdma)) >= txs)
			return;
		if (ieee80211_queue_stopped(&sdata->local->hw, IEEE80211_AC_BE) && ((window + TDMA_TX_TAIL_SPACE) < txs)) {
			ieee80211_tdma_tx_notify(&sdata->vif, true);
			return;
		}
		if ((skb = tdma_get_skb_from_queue(tdma, &tdma->tx_skb_list, window, txs, 2, &duration)) != NULL)
			tdma_do_tx(tdma, skb, txs, duration, &window, false, 2, NULL, idx);
		else
			break;
	}
	rcu_read_lock();
	list_for_each_entry_rcu(sta, &sdata->local->sta_list, list) {
		if (sta->n.aid == aid) {
			found = true;
			break;
		}
	}
	rcu_read_unlock();
	while (found) {
		if (!TDMA_JOINED(tdma) || (TDMA_STATE(tdma) != TDMA_STATUS_ASSOCIATED) || (tdma->last_beacon >= tdma->last_tx_start) || (tdma->last_tx_start == 0))
			return;
		if (tdma->last_tx >= TDMA_MAX_TX_PER_ROUND)
			return;
		if ((window = tdma_get_tx_window(tdma)) >= txs)
			return;
		if ((idx = skb_queue_len(&sta->n.tx)) == 0)
			return;
		if (ieee80211_queue_stopped(&sdata->local->hw, IEEE80211_AC_BE) && ((window + TDMA_TX_TAIL_SPACE) < txs)) {
			ieee80211_tdma_tx_notify(&sdata->vif, true);
			return;
		}
		if ((skb = tdma_get_skb_from_queue(tdma, &sta->n.tx, window, txs, 2, &duration)) != NULL)
			tdma_do_tx(tdma, skb, txs, duration, &window, true, 2, sta, idx);
		else
			break;
	}
}

static struct sk_buff *tdma_prepare_beacon(struct ieee80211_sub_if_data *sdata)
{
	struct sk_buff *skb = NULL;
	struct ieee80211_mgmt *mgmt;
	struct ieee80211_if_tdma *tdma;
	int hdr_len = offsetof(struct ieee80211_mgmt, u.beacon) + sizeof(mgmt->u.beacon), rates, i;
	u8 *pos;
	struct ieee80211_supported_band *sband;
	u8 supp_rates[IEEE80211_MAX_SUPP_RATES];
	bool ht_support;
	size_t sz;

	tdma = &sdata->u.tdma;
	sband = sdata->local->hw.wiphy->bands[tdma->chandef.chan->band];
	ht_support = (bool) ((tdma->chandef.width != NL80211_CHAN_WIDTH_20_NOHT) && sband->ht_cap.ht_supported);
	sz = hdr_len + 12 + sdata->vif.bss_conf.ssid_len;
	if (sband->n_bitrates > 8)
		sz += sband->n_bitrates - 6;
	if (ht_support)
		sz += (4 + sizeof(struct ieee80211_ht_cap) + sizeof(struct ieee80211_ht_operation));
	if ((skb = dev_alloc_skb(sdata->local->tx_headroom + sz + 2 + TDMA_MIN_IE_SIZE + ((TDMA_CFG_VERSION(tdma) < 2) ? 256 : 0) +
				 ((TDMA_CFG_VERSION(tdma) < 3) ? (tdma->node_num - 1) : 1) * (3 + (TDMA_CFG_GACK(tdma) ? (TDMA_MAX_SEQS_IN_BEACON_SZ) : 0)))) == NULL) {
#ifdef TDMA_DEBUG
		printk("TDMA: Could not allocate memory for beacon!\n");
#endif
		return NULL;
	}
	skb_reserve(skb, sdata->local->tx_headroom);
	if ((mgmt = (struct ieee80211_mgmt *)skb_put(skb, hdr_len)) == NULL) {
#ifdef TDMA_DEBUG
		printk("TDMA: Could not put beacon header into skb!\n");
#endif
		goto tpb_err_out;
	}
	memset(mgmt, 0, hdr_len);
	mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_BEACON);
	eth_broadcast_addr(mgmt->da);
	memcpy(mgmt->sa, sdata->vif.addr, ETH_ALEN);
	memcpy(mgmt->bssid, sdata->vif.bss_conf.bssid, ETH_ALEN);
	mgmt->u.beacon.beacon_int = cpu_to_le16(sdata->vif.bss_conf.beacon_int);
	mgmt->u.beacon.capab_info = cpu_to_le16((tdma->privacy ? WLAN_CAPABILITY_PRIVACY : 0) | WLAN_CAPABILITY_IBSS);
/* May provide 'scheduling while atomic' bug on some SMP systems
    mgmt->u.beacon.timestamp = cpu_to_le64(drv_get_tsf(sdata->local, sdata));
*/
	/* build supported rates array */
	pos = supp_rates;
	rates = 0;
	for (i = 0; i < sband->n_bitrates; i++) {
		if (sdata->vif.bss_conf.basic_rates & BIT(i)) {
			*pos++ = 0x80 | (u8)(sband->bitrates[i].bitrate / 5);
			rates++;
		}
	}
	i = rates;
	if (sdata->vif.bss_conf.ssid_len > 0) {
		if ((pos = skb_put(skb, 2 + sdata->vif.bss_conf.ssid_len)) == NULL) {
#ifdef TDMA_DEBUG
			printk("TDMA: Could not put ESSID into skb!\n");
#endif
			goto tpb_err_out;
		}
		*pos++ = (u8)WLAN_EID_SSID;
		*pos++ = (u8)sdata->vif.bss_conf.ssid_len;
		memcpy(pos, sdata->vif.bss_conf.ssid, sdata->vif.bss_conf.ssid_len);
		pos += sdata->vif.bss_conf.ssid_len;
	}
	if (rates > 8)
		rates = 8;
	if ((pos = skb_put(skb, 2 + rates)) == NULL) {
#ifdef TDMA_DEBUG
		printk("TDMA: Could not put basic rates into skb!\n");
#endif
		goto tpb_err_out;
	}
	*pos++ = (u8)WLAN_EID_SUPP_RATES;
	*pos++ = (u8)rates;
	memcpy(pos, supp_rates, rates);

	if (i > rates) {
		rates = i - rates;
		if ((pos = skb_put(skb, 2 + rates)) == NULL) {
#ifdef TDMA_DEBUG
			printk("TDMA: Could not put ext rates into skb!\n");
#endif
			goto tpb_err_out;
		}
		*pos++ = (u8)WLAN_EID_EXT_SUPP_RATES;
		*pos++ = (u8)rates;
		memcpy(pos, &supp_rates[8], rates);
	}
	/* add HT capability and information IEs */
	if (ht_support) {
		if ((pos = skb_put(skb, 4 + sizeof(struct ieee80211_ht_cap) + sizeof(struct ieee80211_ht_operation))) == NULL) {
#ifdef TDMA_DEBUG
			printk("TDMA: Could not put HT capabilities into skb!\n");
#endif
			goto tpb_err_out;
		}
		pos = ieee80211_ie_build_ht_cap(pos, &sband->ht_cap, sband->ht_cap.cap);
		/*
		 * Note: According to 802.11n-2009 9.13.3.1, HT Protection
		 * field and RIFS Mode are reserved in IBSS mode, therefore
		 * keep them at 0
		 */
		pos = ieee80211_ie_build_ht_oper(pos, &sband->ht_cap, &tdma->chandef, 0, false);
	}
	return skb;
tpb_err_out:
	skb_orphan(skb);
	dev_kfree_skb_any(skb);
	return NULL;
}

struct sk_buff *tdma_create_beacon(struct ieee80211_sub_if_data *sdata)
{
	struct sk_buff *skb = NULL;
	struct ieee80211_if_tdma *tdma = &sdata->u.tdma;
	u8 *pos, *saved_pos;
	u16 len = 0, sz;
	u8 ver;
	u8 tie_bcn_for = 0, tie_bcn_to = 0;
	tdma_state state;
	struct sk_buff_head skbs, txskbs;
	bool gack;
	struct sta_info *sta;
	struct sk_buff_head *txlist = &tdma->tx_skb_list;

	if (!TDMA_JOINED(tdma))
		return NULL;
	state = TDMA_STATE(tdma);
	if ((state == TDMA_STATUS_UNKNOW) || (tdma->node_id == 0))
		return NULL;

	ver = TDMA_CFG_VERSION(tdma);
	gack = TDMA_CFG_GACK(tdma);
	if (gack) {
		__skb_queue_head_init(&skbs);
		__skb_queue_head_init(&txskbs);
	}
	tdma->last_own_beacon = (unsigned long)(ktime_to_ns(ktime_get()) >> 10);
#ifdef TDMA_DEBUG_BEACON_TX
	printk("TDMA: (%d) Created own beacon - %lu.\n", sdata->vif.bss_conf.aid, tdma->last_own_beacon);
#endif
	if ((skb = tdma_prepare_beacon(sdata)) == NULL) {
#ifdef TDMA_DEBUG
		printk("TDMA: Could not allocate beacon\n");
#endif
		goto cb_out;
	}

	sz = skb->len;
	if (ver == 2) {
		tdma->b_counter++;
		tie_bcn_to = tie_bcn_for = tdma->b_counter;
		if (tdma->b_counter == 1)
			tie_bcn_to = tdma->node_num;
		tie_bcn_for++;
		if (tie_bcn_for >= tdma->node_num)
			tdma->b_counter = 0;
	}
	if ((pos = skb_put(skb, TDMA_MIN_IE_SIZE + 2)) == NULL) {
		skb_orphan(skb);
		dev_kfree_skb_any(skb);
		skb = NULL;
#ifdef TDMA_DEBUG
		printk("TDMA: Could not put TDMA info into skb!\n");
#endif
		goto cb_out;
	}
	sz += (TDMA_MIN_IE_SIZE + 2);
	*pos++ = (u8)WLAN_EID_PEER_MGMT;
	*pos++ = (u8)TDMA_MIN_IE_SIZE;
	*pos++ = tdma->cfg;
	*pos++ = tdma->node_id;
	*pos++ = (u8)state;
	*pos++ = tdma->node_num;
	*pos++ = tdma->slot_size;
	*pos++ = TDMA_IE_PACK_TX_RX_RATIO(tdma->tx_ratio, tdma->rx_ratio);
	*pos++ = tie_bcn_for;
	put_unaligned_le16(tdma->node_bitmap, (void *)pos);
	pos += sizeof(__le16);
	put_unaligned_le32(tdma->rate, (void *)pos);
	pos += sizeof(__le32);
	saved_pos = pos;
	pos += sizeof(__le16);
	if ((pos = skb_put(skb, ((ver < 2) ? 256 : 0) + ((ver < 3) ? (tdma->node_num - 1) : 1) * (3 + (gack ? (TDMA_MAX_SEQS_IN_BEACON_SZ) : 0)))) == NULL) {
		skb_orphan(skb);
		dev_kfree_skb_any(skb);
		skb = NULL;
#ifdef TDMA_DEBUG
		printk("TDMA: Could not put peer info into skb!\n");
#endif
		goto cb_out;
	}
	tdma->optimize_tx = false;
	rcu_read_lock();
	list_for_each_entry_rcu(sta, &sdata->local->sta_list, list) {
		u8 *lsaved_pos;
		bool missed = !!((sta->n.last_seen + ((ver == 2) ? tdma->node_num - 1 : 1) * tdma->tx_round_duration) < tdma->last_own_beacon);	/* Previous beacon from this node is missed */

		*pos++ = WLAN_EID_TDMA_PEER_INFO + (u8)(sta->n.aid);
		lsaved_pos = pos;
		*pos++ = 1;
		*pos++ = (u8)missed;
		len += 3;
		if ((tdma->node_num == 2) || (ver != 2)) {
			if ((ver == 3) || (tdma->node_num == 2))
				tdma->optimize_tx = true;
			else if (!missed && (((tdma->node_id == tdma->node_num) && (sta->n.aid == 1)) || ((tdma->node_id + 1) == sta->n.aid)))
				tdma->optimize_tx = true;
			if (gack) {
				*lsaved_pos += sta->n.num_seqs;
				memcpy(pos, &sta->n.seqs[0], sta->n.num_seqs);
				pos += sta->n.num_seqs;
				len += sta->n.num_seqs;
				tdma->num_seqs -= sta->n.num_seqs;
				if (sta->n.miss_own_beacon && (sta->n.num_rcvd > 0)) {
					*lsaved_pos += sta->n.num_rcvd;
					memcpy(pos, &sta->n.rcvd[0], sta->n.num_rcvd);
					pos += sta->n.num_rcvd;
					len += sta->n.num_rcvd;
				}
				if (sta->n.num_seqs > 0)
					memcpy(&sta->n.rcvd, &sta->n.seqs, TDMA_MAX_TX_QUEUE_LEN);
				sta->n.num_rcvd = sta->n.num_seqs;
				sta->n.num_seqs = 0;
				rcu_read_unlock();
				tdma_process_pending_frames(sta, ver, &skbs, &txskbs);
				rcu_read_lock();
			}
		} else {
			if (tie_bcn_to == sta->n.aid) {
				if (gack) {
					*lsaved_pos += sta->n.num_seqs;
					memcpy(pos, &sta->n.seqs[0], sta->n.num_seqs);
					pos += sta->n.num_seqs;
					len += sta->n.num_seqs;
					tdma->num_seqs -= sta->n.num_seqs;
					if (sta->n.miss_own_beacon && (sta->n.num_rcvd > 0)) {
						*lsaved_pos += sta->n.num_rcvd;
						memcpy(pos, &sta->n.rcvd[0], sta->n.num_rcvd);
						pos += sta->n.num_rcvd;
						len += sta->n.num_rcvd;
					}
					if (sta->n.num_seqs > 0)
						memcpy(&sta->n.rcvd, &sta->n.seqs, TDMA_MAX_TX_QUEUE_LEN);
					sta->n.num_rcvd = sta->n.num_seqs;
					sta->n.num_seqs = 0;
				}
			}
			if (tie_bcn_for == sta->n.aid) {
				tdma->optimize_tx = !missed;
				if (gack) {
					rcu_read_unlock();
					tdma_process_pending_frames(sta, ver, &skbs, &txskbs);
					rcu_read_lock();
					txlist = &sta->n.tx;
				}
			}
		}
	}
	rcu_read_unlock();
	if (gack) {
		if (!skb_queue_empty(&skbs)) {
			if (ver == 0)
				__skb_queue_purge(&skbs);
			else {
				struct sk_buff *tmp;

				while ((tmp = __skb_dequeue(&skbs)))
					ieee80211_tx_status(&sdata->local->hw, tmp);
			}
		}
		if (!skb_queue_empty(&txskbs))
			skb_queue_splice(&txskbs, txlist);
	}
#ifdef CPTCFG_MAC80211_TDMA_MESH
	if ((state == TDMA_STATUS_ASSOCIATED) && TDMA_IS_MESHED(tdma))
		len += tdma_originator_put(tdma, pos, sdata->vif.addr, skb_queue_len(txlist));
#endif
	put_unaligned_le16(len, (void *)saved_pos);
	pskb_trim(skb, sz + len);
	skb->len = sz + len;
	tdma_skb_fill_info(sdata, skb);
cb_out:
	return skb;
}

static enum hrtimer_restart tdma_housekeeper(struct hrtimer *timer)
{
	struct ieee80211_if_tdma *tdma = container_of(timer, struct ieee80211_if_tdma, hk);
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);
	u32 counter = 0;
	unsigned long now;
	tdma_state state;
	u8 ver = TDMA_CFG_VERSION(tdma);
	bool voted = true;
	struct sta_info *sta;

	if (!TDMA_JOINED(tdma))
		return HRTIMER_NORESTART;
	/* Schedule next call of keeper */
	hrtimer_forward_now(timer, ns_to_ktime(tdma->tx_round_duration * 1000));
	state = TDMA_STATE(tdma);
	now = (unsigned long)(ktime_to_ns(ktime_get()) >> 10);
	if ((ver == 3) && !TDMA_INITIALIZED(tdma)) {
#ifdef TDMA_DEBUG_HK
		printk("TDMA: CPE is not initialized\n");
#endif
		goto out;
	}
	if (tdma->node_id > 0)
		tdma->node_bitmap = BIT(tdma->node_id - 1);
	else
		tdma->node_bitmap = 0;
	rcu_read_lock();
	list_for_each_entry_rcu(sta, &sdata->local->sta_list, list) {
		if (now > sta->n.last_seen) {
			sta->n.counter++;
			tdma->node_bitmap |= sta->n.node_bitmap;
			if (ver != 2) {
				if ((now - sta->n.last_seen) > (tdma->tx_round_duration + tdma_tx_slot_duration(tdma))) {
					sta->n.miss_counter++;
					sta->n.voted = false;
				}
				if (test_sta_flag(sta, WLAN_STA_AUTHORIZED)) {
					if (((sta->n.miss_counter * 100) / sta->n.counter) < TDMA_BAD_NEIGHBOUR_RATIO) {
						voted = (voted && sta->n.voted);
					} else {
#ifdef TDMA_DEBUG_HK
						printk("TDMA: NB (%d). Miss counter - %lu, BAD neighbour ratio is %lu\n", sta->n.aid, sta->n.miss_counter, (sta->n.miss_counter * 100) / sta->n.counter);
#endif
						if (ver == 3)
							voted = false;
					}
				}
			}
		}
		counter++;
	}
	rcu_read_unlock();
	if (counter == 0) {
		tdma->last_beacon = 0;
		tdma->last_beacon_node = 0;
	}
	tdma->counter++;
#ifdef TDMA_DEBUG_HK
	printk("TDMA: HOUSEKEEPER (%d) is called (%lu) time after our state has changed. Right now the node has (%lu) neighbours. Now is (%lu)\n", state, tdma->counter, (unsigned long)counter, now);
#endif
	switch (state) {
	case TDMA_STATUS_UNKNOW:
		tdma->node_id = tdma_get_avail_slot(tdma);
		if (tdma->node_id == 0) {
#ifdef TDMA_DEBUG_HK
			printk("TDMA: Uh-oh! No available timeslot for this node\n");
#endif
			goto out;
		}
		if ((ver == 2) || ((counter == 0) && (ver != 3) && (tdma->counter > tdma->unknow_state_counter))) {
			/* No neighbours. So let us be first */
#ifdef TDMA_DEBUG
			printk("TDMA: It is the first node. Start with node_id (%d)\n", (int)tdma->node_id);
#endif
			TDMA_SET_STATE(tdma, TDMA_STATUS_ASSOCIATED);
			tdma->counter = 0;
			mb();
			schedule_work(&tdma->create_work);
		} else if ((counter > 0) && (tdma->counter > tdma->unknow_state_counter)) {
			/* We got some beacons and have some neighbours. Try to associate */
#ifdef TDMA_DEBUG
			printk("TDMA: Try to associate with node_id (%d) and node_bitmap (%04X)\n", (int)tdma->node_id, (unsigned int)tdma->node_bitmap);
#endif
			TDMA_SET_STATE(tdma, TDMA_STATUS_ASSOCIATING);
			tdma->counter = 0;
			mb();
			schedule_work(&tdma->join_work);
		}
		break;
	case TDMA_STATUS_ASSOCIATING:
		if (tdma->counter > TDMA_MAX_ASSOCIATING_STATE_COUNT) {
			tdma->counter = 0;
			if ((ver != 3) && (counter == 0)) {
				/* We losted worked neighbours or all beacons from neighbours? Switch to ASSOCIATED state */
				sdata->vif.bss_conf.ibss_joined = true;
				state = TDMA_STATUS_ASSOCIATED;
				TDMA_SET_STATE(tdma, state);
			} else {
#ifdef TDMA_DEBUG
				printk("TDMA: No neighbours or beacons in ASSOCIATING state. Reset state to UNKNOW\n");
#endif
				state = TDMA_STATUS_UNKNOW;
				TDMA_SET_STATE(tdma, state);
				mb();
				schedule_work(&tdma->reset_work);
			}
		} else if (tdma->counter > TDMA_FUDGE_ASSOCIATING_STATE_COUNT) {
			/* Change status to ASSOCIATED if all neightbours are marked as voted */
			if (voted) {
				tdma->counter = 0;
				sdata->vif.bss_conf.ibss_joined = true;
				state = TDMA_STATUS_ASSOCIATED;
				TDMA_SET_STATE(tdma, state);
#ifdef TDMA_DEBUG
				printk("TDMA: All neighbours accept this node settings. Change state to ASSOCIATED\n");
#endif
			} else {
#ifdef TDMA_DEBUG
				printk("TDMA: Not all neighbours voted for this node.\n");
#endif
			}
		}
		break;
	case TDMA_STATUS_ASSOCIATED:
		if (((ver != 2) && counter && (tdma->counter > 1) && (now > tdma->last_verified) && (now - tdma->last_verified > tdma->tx_round_duration * TDMA_MAX_UNVERIFIED_COUNT)) ||
		    ((tdma->counter > ((ver == 2) ? 10 : 3)) && (now > tdma->last_own_beacon) && ((now - tdma->last_own_beacon) > (tdma->tx_round_duration * TDMA_MAX_LOST_BEACON_COUNT))) ||
		    ((ver == 3) && (counter == 0) && (tdma->counter > 3))) {
#ifdef TDMA_DEBUG
			printk("TDMA: (%lu) All neighbours do'nt see node / or beaconing troubles. Reset node. (LV %lu / LB %lu / TC %lu / NB %d)\n", now, tdma->last_verified, tdma->last_own_beacon, tdma->counter,
			       counter);
#endif
			tdma->counter = 0;
			state = TDMA_STATUS_UNKNOW;
			TDMA_SET_STATE(tdma, state);
			mb();
			schedule_work(&tdma->reset_work);
		}
		break;
	default:
		break;
	}
out:
	mb();
	if (state != TDMA_STATUS_UNKNOW) {
		tdma->last_tx_start = now;
		tdma->last_tx_call = tdma->last_tx_start;
		tdma->last_tx = 0;
		ieee80211_tdma_tx_notify(&sdata->vif, true);
	}
	ieee80211_queue_work(&sdata->local->hw, &sdata->work);
	return HRTIMER_RESTART;
}

static int ieee80211_join_tdma(struct wiphy *wiphy, struct net_device *dev, struct cfg80211_tdma_params *params)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	int ret = -EBUSY, i;
	struct ieee80211_if_tdma *tdma = &sdata->u.tdma;
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_rate *rate;
	struct ieee80211_supported_band *sband;

	if (!test_bit(SDATA_STATE_RUNNING, &sdata->state))
		return ret;
	if (TDMA_JOINED(tdma))
		return ret;
	if (cfg80211_chandef_dfs_required(sdata->local->hw.wiphy, &params->chandef, NL80211_IFTYPE_TDMA) != 0) {
		sdata_info(sdata, "Failed to join TDMA network, DFS for specified channel is required\n");
		return ret;
	}
	tdma->privacy = params->privacy;
	tdma->control_port = params->control_port;
	tdma->slot_size = params->cur_rate;
	TDMA_CFG_SET_VERSION(tdma, params->version);
	TDMA_CFG_SET_NO_MSDU(tdma, params->no_msdu);
	TDMA_CFG_SET_NO_REORDER(tdma, params->no_reorder);
	if (params->version != 3) {
		TDMA_CFG_SET_GACK(tdma, params->gack);
		TDMA_CFG_SET_POLLING(tdma, params->enable_polling);
	}
	if (params->tx_ratio == 0)
		tdma->tx_ratio = 1;
	else
		tdma->tx_ratio = params->tx_ratio;
	if (params->rx_ratio == 0)
		tdma->rx_ratio = 1;
	else
		tdma->rx_ratio = params->rx_ratio;
	if ((params->node_num == 0) || (params->node_num > TDMA_MAX_NODE_PER_CELL))
		tdma->node_num = TDMA_MAX_NODE_PER_CELL;
	else
		tdma->node_num = params->node_num;
	if ((params->version != 3) && params->gack && (tdma->node_num > 8))
		tdma->node_num = 8;
	tdma->chandef = params->chandef;
	tdma->rate = params->rate;
	sdata->vif.bss_conf.ssid_len = params->ssid.ssid_len;
	memcpy(sdata->vif.bss_conf.ssid, params->ssid.ssid, params->ssid.ssid_len);

	if (!cfg80211_reg_can_beacon(local->hw.wiphy, &tdma->chandef, NL80211_IFTYPE_TDMA)) {
#ifdef TDMA_DEBUG
		printk("TDMA: Something wrong\n");
#endif
		if ((params->chandef.width == NL80211_CHAN_WIDTH_40))
			tdma->chandef.width = NL80211_CHAN_WIDTH_20;
		tdma->chandef.center_freq1 = tdma->chandef.chan->center_freq;
	}

	sband = sdata->local->hw.wiphy->bands[tdma->chandef.chan->band];
	if (tdma->chandef.chan->band == NL80211_BAND_2GHZ)
		sdata->vif.bss_conf.use_short_preamble = false;
	else
		sdata->vif.bss_conf.use_short_preamble = true;
	if ((tdma->node_num < 2) || (TDMA_CFG_VERSION(tdma) == 3))
		tdma->node_num = 2;
	if (tdma->slot_size > 3)
		tdma->slot_size = 3;
	sdata->vif.bss_conf.ibss_creator = false;
	sdata->vif.bss_conf.use_cts_prot = false;
	sdata->flags &= ~IEEE80211_SDATA_OPERATING_GMODE;
	memcpy(sdata->vif.bss_conf.mcast_rate, params->mcast_rate, sizeof(params->mcast_rate));
	i = ieee80211_chandef_get_shift(&tdma->chandef);
	if (TDMA_CFG_VERSION(tdma) == 0) {
		params->supp_rates = BIT(tdma->cur_rate);
		rate = &sband->bitrates[tdma->cur_rate];
		if (tdma->chandef.chan->band == NL80211_BAND_2GHZ) {
			if (!(rate->bitrate == 110) && !(rate->bitrate < 60)) {
				sdata->flags |= IEEE80211_SDATA_OPERATING_GMODE;
				sdata->vif.bss_conf.use_short_preamble = true;
			}
		}
		sdata->vif.bss_conf.basic_rates = params->supp_rates;
		params->mask.control[tdma->chandef.chan->band].legacy = params->supp_rates;
	} else {
		if ((tdma->chandef.chan->band == NL80211_BAND_2GHZ) && !TDMA_CFG_GACK(tdma) && (i == 0)) {
			switch (tdma->slot_size) {
			case 1:
				tdma->rate = 20;
				break;
			case 2:
				tdma->rate = 55;
				break;
			case 3:
				sdata->flags |= IEEE80211_SDATA_OPERATING_GMODE;
				tdma->rate = 60;
				sdata->vif.bss_conf.use_short_preamble = true;
				break;
			default:
				tdma->rate = 10;
				break;
			}
		} else {
			if (tdma->chandef.chan->band == NL80211_BAND_2GHZ)
				sdata->flags |= IEEE80211_SDATA_OPERATING_GMODE;
			if (i > 2)
				tdma->rate = 90;
			else
				tdma->rate = 60;
			sdata->vif.bss_conf.use_short_preamble = true;
			if (TDMA_CFG_GACK(tdma) && (tdma->slot_size < 2))
				tdma->slot_size = 2;
		}
		sdata->vif.bss_conf.basic_rates = tdma_adjust_basic_rates(sdata, i, 0xFFFF);
		params->mask.control[tdma->chandef.chan->band].legacy = sdata->vif.bss_conf.basic_rates;
	}
	if (i > 0)
		tdma->rate >>= i;
	sdata->local->hw.wiphy->rts_threshold = (u32)-1;
	drv_set_rts_threshold(sdata->local, sdata->local->hw.wiphy->rts_threshold);
	sdata->local->hw.wiphy->frag_threshold = (u32)-1;
	drv_set_frag_threshold(sdata->local, sdata->local->hw.wiphy->frag_threshold);
#ifdef TDMA_DEBUG
	printk("TDMA: Start network join with %d nodes and %d version. GACK(%d)\n", tdma->node_num, TDMA_CFG_VERSION(tdma), TDMA_CFG_GACK(tdma));
	printk("TDMA: Setuped bitrate - %d. Calced - %d. Slot size - %d. Cur_rate - %d. Supported Rates - %x\n", params->rate, tdma->rate, tdma->slot_size, tdma->cur_rate, sdata->vif.bss_conf.basic_rates);
#endif
	tdma->ack_duration = TDMA_TX_TAIL_SPACE + ieee80211_frame_duration(tdma->chandef.chan->band, 10, tdma->rate, 0, sdata->vif.bss_conf.use_short_preamble, i);
	tdma->tx_slot_duration = tdma_tx_slot_calc(sdata, dev->mtu);
	switch (TDMA_CFG_VERSION(tdma)) {
	case 2:
		if (tdma->tx_ratio == tdma->rx_ratio) {
			tdma->second_tx_slot_duration = tdma->tx_slot_duration;
		} else if (tdma->tx_ratio < tdma->rx_ratio) {
			tdma->second_tx_slot_duration = tdma_tu_adjust((tdma->tx_slot_duration / tdma->tx_ratio) * tdma->rx_ratio, tdma->slot_size, i, TDMA_CFG_GACK(tdma));
		} else {
			tdma->second_tx_slot_duration = tdma->tx_slot_duration;
			tdma->tx_slot_duration = tdma_tu_adjust((tdma->tx_slot_duration / tdma->rx_ratio) * tdma->tx_ratio, tdma->slot_size, i, TDMA_CFG_GACK(tdma));
		}
		tdma->tx_round_duration = tdma->tx_slot_duration + tdma->second_tx_slot_duration;
		break;
	default:
		tdma->tx_round_duration = tdma->tx_slot_duration * tdma_get_slot_num(tdma);
		break;
	}
#ifdef TDMA_DEBUG
	printk("TDMA: Round duration - %lu, TX slot duration - %lu, Second TX slot duration - %lu\n", tdma->tx_round_duration, tdma->tx_slot_duration, tdma->second_tx_slot_duration);
#endif
	tdma_setup_polling(tdma);
	if ((TDMA_CFG_VERSION(tdma) == 0) || TDMA_CFG_GACK(tdma))
		sdata->noack_map = 0xffff;
	memcpy(&tdma->mask, &params->mask, sizeof(params->mask));
	for (i = 0; i < NUM_NL80211_BANDS; i++) {
		struct ieee80211_supported_band *sband = sdata->local->hw.wiphy->bands[i];
		int j;

		sdata->rc_rateidx_mask[i] = params->mask.control[i].legacy;
		memcpy(sdata->rc_rateidx_mcs_mask[i], params->mask.control[i].ht_mcs, sizeof(params->mask.control[i].ht_mcs));

		sdata->rc_has_mcs_mask[i] = false;
		if (!sband)
			continue;

		for (j = 0; j < IEEE80211_HT_MCS_MASK_LEN; j++) {
			sband->ht_cap.mcs.rx_mask[j] = tdma->mask.control[i].ht_mcs[j];	/* turn off all masked bits */
			if (~sdata->rc_rateidx_mcs_mask[i][j]) {
				sdata->rc_has_mcs_mask[i] = true;
			}
		}
	}

	if (ieee80211_hw_check(&local->hw, HAS_RATE_CONTROL))
		drv_set_bitrate_mask(local, sdata, &params->mask);
	tdma->max_clock_drift = tdma_max_clock_drift(sdata);
	sdata->vif.bss_conf.beacon_int = (u16)(tdma->tx_round_duration >> 10);
	tdma_reset_state(tdma);

	ieee80211_vif_release_channel(sdata);
	sdata->smps_mode = IEEE80211_SMPS_OFF;
	sdata->needed_rx_chains = sdata->local->rx_chains;
	if (ieee80211_vif_use_channel(sdata, &tdma->chandef, IEEE80211_CHANCTX_SHARED)) {
		sdata_info(sdata, "Failed to join network, no channel context\n");
		goto start_out;
	}
	local->fif_control++;
	ieee80211_configure_filter(local);
#ifdef TDMA_DEBUG
	printk("TDMA: Beacon interval in TU - %d.\n", sdata->vif.bss_conf.beacon_int);
#endif
	if ((TDMA_CFG_VERSION(tdma) == 2) && (sdata->vif.bss_conf.ssid_len == 0)) {
		memcpy(tdma->bssid, sdata->vif.addr, ETH_ALEN);
	} else if ((TDMA_CFG_VERSION(tdma) == 3) && (sdata->vif.bss_conf.ssid_len == 0)) {
		memcpy(tdma->bssid, params->bs_mac, ETH_ALEN);
	} else {
		tdma->bssid[0] = 0x4f;
		if (TDMA_CFG_VERSION(tdma) < 2) {
			tdma->bssid[1] = TDMA_CFG_BSSID(tdma);
			tdma->bssid[2] = 0;
			tdma->bssid[3] = tdma->node_num;
		} else {
			tdma->bssid[1] = TDMA_MAGIC_BYTE;
			tdma->bssid[2] = TDMA_MAGIC_BYTE;
			tdma->bssid[3] = TDMA_MAGIC_BYTE;
		}
		tdma->bssid[4] = (u8)((tdma->chandef.chan->center_freq & 0xff00) >> 8);
		tdma->bssid[5] = (u8)(tdma->chandef.chan->center_freq & 0xff);
	}
	sdata->vif.bss_conf.bssid = tdma->bssid;

	if ((TDMA_CFG_VERSION(tdma) == 2) && TDMA_CFG_GACK(tdma) && (tdma->node_num > 2))
#ifdef TDMA_TX_TASKLET
		tasklet_init(&tdma->tx_tasklet, tdma_tx2, (unsigned long)tdma);
#else
		INIT_WORK(&tdma->tx_work, tdma_tx2);
#endif
	else
#ifdef TDMA_TX_TASKLET
		tasklet_init(&tdma->tx_tasklet, tdma_tx, (unsigned long)tdma);
#else
		INIT_WORK(&tdma->tx_work, tdma_tx);
#endif
	TDMA_SET_JOINED(tdma, true);
	mb();
	ieee80211_set_wmm_default(sdata, true, sdata->vif.type != NL80211_IFTYPE_STATION);
	hrtimer_start(&tdma->hk, ns_to_ktime(tdma->tx_round_duration << 10), HRTIMER_MODE_REL);

	ret = 0;
start_out:
	if (!ret) {

		mutex_lock(&sdata->local->mtx);
		ieee80211_recalc_idle(sdata->local);
		mutex_unlock(&sdata->local->mtx);

		/*
		 * 802.11n-2009 9.13.3.1: In an IBSS, the HT Protection field is
		 * reserved, but an HT STA shall protect HT transmissions as though
		 * the HT Protection field were set to non-HT mixed mode.
		 *
		 * In an IBSS, the RIFS Mode field of the HT Operation element is
		 * also reserved, but an HT STA shall operate as though this field
		 * were set to 1.
		 */

		sdata->vif.bss_conf.ht_operation_mode |= IEEE80211_HT_OP_MODE_PROTECTION_NONHT_MIXED | IEEE80211_HT_PARAM_RIFS_MODE;

		ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_HT | BSS_CHANGED_BASIC_RATES | BSS_CHANGED_BSSID | BSS_CHANGED_SSID | BSS_CHANGED_ERP_PREAMBLE | BSS_CHANGED_ERP_SLOT);

		ieee80211_queue_work(&sdata->local->hw, &sdata->work);
	}
	return ret;
}

void ieee80211_tdma_stop(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_tdma *tdma = &sdata->u.tdma;

	if (!TDMA_JOINED(tdma))
		return;
	TDMA_SET_JOINED(tdma, false);
#ifdef TDMA_DEBUG
	printk("TDMA: stop is called\n");
#endif
	sdata->vif.bss_conf.ssid_len = 0;
	hrtimer_cancel(&tdma->hk);
#ifdef TDMA_TX_TASKLET
	tasklet_kill(&tdma->tx_tasklet);
#else
	cancel_work_sync(&tdma->tx_work);
#endif
	cancel_work_sync(&tdma->join_work);
	cancel_work_sync(&tdma->create_work);
	cancel_work_sync(&tdma->reset_work);
	tdma_reset_state(tdma);
	mutex_lock(&sdata->local->mtx);
	ieee80211_recalc_idle(sdata->local);
	mutex_unlock(&sdata->local->mtx);
	cancel_work_sync(&sdata->work);
	sdata->local->fif_control--;
	skb_queue_purge(&sdata->skb_queue);
	ieee80211_configure_filter(sdata->local);
	synchronize_rcu();
	RCU_INIT_POINTER(sdata->vif.chanctx_conf, NULL);
}

static int ieee80211_leave_tdma(struct wiphy *wiphy, struct net_device *dev)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);

#ifdef TDMA_DEBUG
	printk("TDMA: leave is called\n");
#endif
	if (sdata->vif.type != NL80211_IFTYPE_TDMA)
		return -EBUSY;
	ieee80211_tdma_stop(sdata);
	return 0;
}

static void tdma_reset_work(struct work_struct *work)
{
	struct ieee80211_if_tdma *tdma = container_of(work, struct ieee80211_if_tdma, reset_work);

	tdma_reset_state(tdma);
}

static void tdma_join_work(struct work_struct *work)
{
	struct ieee80211_if_tdma *tdma = container_of(work, struct ieee80211_if_tdma, join_work);
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);

	drv_flush(sdata->local, sdata, 0, true);
	tdma->last_beacon = 0;
	sdata->vif.bss_conf.aid = tdma->node_id;
	sdata->vif.bss_conf.enable_beacon = false;
	ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_IBSS);
}

static void tdma_create_work(struct work_struct *work)
{
	struct ieee80211_if_tdma *tdma = container_of(work, struct ieee80211_if_tdma, create_work);
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);

	drv_flush(sdata->local, sdata, 0, true);
	tdma->last_own_beacon = 0;
	sdata->vif.bss_conf.aid = tdma->node_id;
	sdata->vif.bss_conf.ibss_creator = true;
	sdata->vif.bss_conf.ibss_joined = true;
	sdata->vif.bss_conf.enable_beacon = false;
	ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_IBSS);
}

void ieee80211_tdma_setup_sdata(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_tdma *tdma = &sdata->u.tdma;

	memset(tdma, 0, sizeof(*tdma));
	hrtimer_init(&tdma->hk, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	tdma->hk.function = &tdma_housekeeper;
	sdata->flags |= IEEE80211_SDATA_ALLMULTI;
	RCU_INIT_POINTER(tdma->sta, NULL);
	INIT_WORK(&tdma->reset_work, tdma_reset_work);
	INIT_WORK(&tdma->join_work, tdma_join_work);
	INIT_WORK(&tdma->create_work, tdma_create_work);
	timer_setup(&tdma->poll_timer, tdma_poll_timer, 0);
	add_timer(&tdma->poll_timer);
	skb_queue_head_init(&tdma->tx_skb_list);
	sdata->local->hw.extra_tx_headroom += tdma_hdr_len();
	tdma->tdma_hdrlen = ieee80211_hdrlen;
	tdma->tdma_get_hdrlen_from_skb = ieee80211_get_hdrlen_from_skb;
	tdma->tdma_chandef_get_shift = ieee80211_chandef_get_shift;
	tdma->tdma_frame_duration = ieee80211_frame_duration;
	tdma->tdma_sta_info_get = sta_info_get;
}

void ieee80211_tdma_work(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_tdma *tdma = &sdata->u.tdma;
	u8 ver;
	unsigned long timeout;

	if (!TDMA_JOINED(tdma))
		return;
	ver = TDMA_CFG_VERSION(tdma);
	timeout = usecs_to_jiffies(((ver == 2) ? (tdma->node_num - 1) : 1) * tdma->tx_round_duration * TDMA_MAX_LOST_BEACON_COUNT);
	ieee80211_sta_expire(sdata, timeout);
#ifdef CPTCFG_MAC80211_TDMA_MESH
	tdma_retr_expire(tdma, usecs_to_jiffies(tdma->tx_round_duration << 3));
#endif
	if (TDMA_STATE(tdma) == TDMA_STATUS_ASSOCIATED) {
		if (!netif_carrier_ok(sdata->dev)) {
#ifdef TDMA_DEBUG
			printk("TDMA: Enable frame transmission\n");
#endif
			netif_carrier_on(sdata->dev);
		}
		if (skb_queue_len(&tdma->tx_skb_list) < (TDMA_MAX_TX_PER_ROUND >> 2)) {
			if (!ieee80211_queue_stopped(&sdata->local->hw, IEEE80211_AC_BE) && __netif_subqueue_stopped(sdata->dev, IEEE80211_AC_BE)) {
				netif_wake_subqueue(sdata->dev, IEEE80211_AC_BE);
#ifdef TDMA_DEBUG_TX_QUEUE_START_STOP
				printk("TDMA: Worker starts Intf Queue %d\n", IEEE80211_AC_BE);
#endif
			}
		}
	}
}
