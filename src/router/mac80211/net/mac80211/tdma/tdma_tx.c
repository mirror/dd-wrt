#include <linux/module.h>
#include <linux/random.h>
#include <linux/time.h>
#include <linux/export.h>

#include "pdwext.h"

unsigned long tdma_tx_slot_duration(struct ieee80211_if_tdma *tdma)
{
	return ptdma_tx_slot_duration(tdma);
}


bool tdma_is_tx_slot(struct ieee80211_if_tdma * tdma)
{
	unsigned long now = 0;

	if ((tdma->last_tx_start == 0) || (tdma->last_tx_call == 0) || (tdma->last_beacon >= tdma->last_tx_start))
		return false;
	now = (unsigned long)(ktime_to_ns(ktime_get()) >> 10);
	now++;
	if (now < tdma->last_tx_call)
		now = tdma->last_tx_call;
	return ! !(((now - tdma->last_tx_start) < ptdma_tx_slot_duration(tdma)));
}


static bool tdma_can_tx(struct ieee80211_if_tdma * tdma, u16 duration)
{
	unsigned long now = 0, txs = ptdma_tx_slot_duration(tdma);

	if ((tdma->last_tx_start == 0) || (tdma->last_tx_call == 0))
		return false;
	now = (unsigned long)(ktime_to_ns(ktime_get()) >> 10);
	if (now < tdma->last_tx_call)
		now = tdma->last_tx_call;
	return ! !((now - tdma->last_tx_start + duration) < txs);
}


void ieee80211_tdma_tx_notify(struct ieee80211_vif *vif, bool force)
{

	if (vif->type == NL80211_IFTYPE_TDMA) {
		struct ieee80211_sub_if_data *sdata = vif_to_sdata(vif);

		if ((sdata->u.tdma.last_tx_start != 0) && (force || (TDMA_CFG_VERSION_P(sdata->u.tdma.cfg) && !TDMA_CFG_GACK_P(sdata->u.tdma.cfg))))
#ifdef TDMA_TX_TASKLET
			tasklet_hi_schedule(&sdata->u.tdma.tx_tasklet);
#else
			schedule_work(&sdata->u.tdma.tx_work);
#endif
	}
}


bool tdma_do_msdu(struct ieee80211_if_tdma *tdma)
{
	return ptdma_do_msdu(tdma);
}


void tdma_plan_next_tx(struct ieee80211_if_tdma *tdma, unsigned long msecs)
{
	hrtimer_cancel(&tdma->hk);
	hrtimer_start(&tdma->hk, ns_to_ktime(msecs * 1000), HRTIMER_MODE_REL);
}


bool tdma_store_pending_frame(struct tdma_neighbour *n, struct sk_buff *skb)
{

	if (spin_trylock_bh(&n->pending.lock)) {
		skb_orphan(skb);
		__skb_queue_tail(&n->pending, skb);
		spin_unlock_bh(&n->pending.lock);
		return true;
	}
	return false;
}


void tdma_remove_ack_frame(struct ieee80211_sub_if_data *sdata, u16 ack_frame_id)
{
	if (unlikely(ack_frame_id)) {
		struct sk_buff *ack_skb;
		unsigned long flags;

		spin_lock_irqsave(&sdata->local->ack_status_lock, flags);
		ack_skb = idr_find(&sdata->local->ack_status_frames, ack_frame_id);
		if (ack_skb)
			idr_remove(&sdata->local->ack_status_frames, ack_frame_id);
		spin_unlock_irqrestore(&sdata->local->ack_status_lock, flags);

		if (ack_skb)
			dev_kfree_skb_any(ack_skb);
	}
}


unsigned long tdma_get_tx_window(struct ieee80211_if_tdma *tdma)
{
	unsigned long now = (unsigned long)(ktime_to_ns(ktime_get()) >> 10);

	now++;
	if (now > tdma->last_tx_call)
		tdma->last_tx_call = now;
	return (unsigned long)(tdma->last_tx_call - tdma->last_tx_start);
}


void tdma_process_pending_frames(struct sta_info *sta, u8 ver, struct sk_buff_head *skbs, struct sk_buff_head *txskbs)
{
	struct sk_buff *skb, *tmp;

	if (!spin_trylock(&sta->n.pending.lock))
		return;
	skb_queue_walk_safe(&sta->n.pending, skb, tmp) {
		struct tdma_hdr *thdr = (struct tdma_hdr *)(skb->data + sta->sdata->u.tdma.tdma_get_hdrlen_from_skb(skb));
		u8 count = TDMA_HDR_COUNTER(thdr);

		count++;
		TDMA_HDR_SET_COUNTER(thdr, count);
		if (count > 8) {
			__skb_unlink(skb, &sta->n.pending);
			if (ver != 0) {
				struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

				info->flags &= ~(IEEE80211_TX_CTL_NO_ACK | IEEE80211_TX_STAT_ACK);
				info->flags |= (IEEE80211_TX_CTL_INJECTED);
				info->status.rates[0].count = 3;
				info->status.rates[1].idx = -1;
			}
			__skb_queue_tail(skbs, skb);
		} else if ((count == 3) || (count == 6)) {
			struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
			struct ieee80211_key *key;

			info->flags &= ~(IEEE80211_TX_STAT_ACK);
			info->flags |= IEEE80211_TX_CTL_NO_ACK;
			info->control.rates[0] = info->status.rates[0];
			info->control.rates[0].count = 1;
			info->control.rates[1].idx = -1;
			info->control.vif = &sta->sdata->vif;
			info->control.hw_key = NULL;
			if ((key = rcu_dereference(sta->ptk[sta->ptk_idx])) != NULL) {
				if (key->flags & KEY_FLAG_UPLOADED_TO_HARDWARE)
					info->control.hw_key = &key->conf;
			}
			info->control.rts_cts_rate_idx = -1;
			info->control.use_rts = 0;
			info->control.use_cts_prot = 0;
			info->control.short_preamble = sta->sdata->vif.bss_conf.use_short_preamble;
			info->control.skip_table = (ver == 0) ? 1 : 0;
			__skb_unlink(skb, &sta->n.pending);
			__skb_queue_tail(txskbs, skb);
		}
	}
	spin_unlock(&sta->n.pending.lock);
}


bool tdma_tx_can_aggregate(struct ieee80211_sub_if_data *sdata, struct sta_info *sta, struct sk_buff *orig, struct sk_buff *skb)
{
	struct ieee80211_supported_band *sband = sdata->local->hw.wiphy->bands[sdata->u.tdma.chandef.chan->band];

	if (skb_is_nonlinear(orig) && skb_linearize(orig))
		return false;
	if (skb_is_nonlinear(skb) && skb_linearize(skb))
		return false;
	if (skb->len + orig->len > pamsdu_limit(sband, (sta) ? &sta->sta : NULL))
		return false;
	return true;
}


bool tdma_tx_aggr_skb(struct ieee80211_sub_if_data * sdata, struct sk_buff * orig, struct sk_buff * skb, struct sk_buff_head * txlist)
{
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	u8 *in;
	u16 subframe_len, hdrlen;
	struct tdma_hdr *thdr;
	__le16 flag;
	struct ieee80211_hdr *hdr1 = (struct ieee80211_hdr *)orig->data, *hdr = (struct ieee80211_hdr *)skb->data;
	bool qos = ieee80211_is_data_qos(hdr->frame_control);

	hdrlen = sdata->u.tdma.tdma_hdrlen(hdr1->frame_control);
	thdr = (struct tdma_hdr *)(orig->data + hdrlen);
	hdrlen += tdma_hdr_len();
	subframe_len = orig->len - hdrlen;
	if (!TDMA_HDR_AMSDU(thdr)) {
		TDMA_HDR_SET_AMSDU(thdr, true);
		flag = cpu_to_le16((qos ? IEEE80211_STYPE_QOS_DATA_CFACK : IEEE80211_STYPE_DATA_CFACK));
		if ((hdr1->frame_control & flag) == flag) {
			thdr->len = cpu_to_le16(subframe_len);
			TDMA_HDR_SET_COMPRESSED(thdr, true);
			hdr1->frame_control &= ~flag;
			if (qos)
				hdr1->frame_control |= cpu_to_le16(IEEE80211_STYPE_QOS_DATA);
		} else
			thdr->len = cpu_to_le16(subframe_len);
		orig->ip_summed = CHECKSUM_UNNECESSARY;
	}
	hdrlen = sdata->u.tdma.tdma_hdrlen(hdr->frame_control);
	hdrlen += tdma_hdr_len();
	subframe_len = skb->len - hdrlen;

	if (skb_tailroom(orig) < (subframe_len + sizeof(__le16))) {
		if (pskb_expand_head(orig, 0, (subframe_len + sizeof(__le16)) - skb_tailroom(orig), GFP_ATOMIC)) {
#ifdef TDMA_DEBUG
			printk("TDMA: Could not expand original skb\n");
#endif
			return false;
		}
	}
	if ((in = skb_put(orig, sizeof(__le16))) == NULL) {
#ifdef TDMA_DEBUG
		printk("TDMA: Could not obtain room\n");
#endif
		return false;
	}
	flag = cpu_to_le16((qos ? IEEE80211_STYPE_QOS_DATA_CFACK : IEEE80211_STYPE_DATA_CFACK));
	if ((hdr->frame_control & flag) == flag)
		put_unaligned_le16(TDMA_HDR_COMPRESSED | subframe_len, (void *)in);
	else
		put_unaligned_le16(subframe_len, (void *)in);
	if ((in = skb_put(orig, subframe_len)) == NULL) {
#ifdef TDMA_DEBUG
		printk("TDMA: Could not obtain room\n");
#endif
		return false;
	}
	memcpy((void *)in, (const void *)(skb->data + hdrlen), subframe_len);

	tdma_remove_ack_frame(sdata, info->ack_frame_id);
	spin_lock_bh(&txlist->lock);
	__skb_unlink(skb, txlist);
	spin_unlock_bh(&txlist->lock);
	skb_orphan(skb);
	dev_kfree_skb_any(skb);
	return true;
}


void tdma_sched_tx(struct ieee80211_sub_if_data *sdata, struct sk_buff *skb, enum nl80211_band band)
{
	struct ieee80211_if_tdma *tdma = &sdata->u.tdma;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	bool qos = false, unicast = false, to_head = false;
	struct sk_buff_head *txlist = &tdma->tx_skb_list;

	if (!TDMA_JOINED(tdma) || (TDMA_STATE(tdma) != TDMA_STATUS_ASSOCIATED))
		return;
#if IS_ENABLED(CPTCFG_MAC80211_COMPRESS)
	mac80211_tx_compress(sdata, skb);
#endif
	info->hw_queue = IEEE80211_AC_BE;
	info->band = band;
	if (!TDMA_CFG_NO_REORDER(tdma))
		to_head = ptdma_skb_priority(skb);
	hdr->duration_id = 0;
	if (!is_multicast_ether_addr(hdr->addr1)) {
		unicast = true;
		qos = ieee80211_is_data_qos(hdr->frame_control);
	}
	if ((tdma->node_num > 2) && unicast && qos && (TDMA_CFG_VERSION(tdma) == 2)) {
		struct sta_info *sta = tdma->tdma_sta_info_get(sdata, hdr->addr1);

		if (sta)
			txlist = &sta->n.tx;
		else
			txlist = &tdma->tx_skb_list;
	} else
		txlist = &tdma->tx_skb_list;
	spin_lock_bh(&txlist->lock);
	if (to_head)
		__skb_queue_head(txlist, skb);
	else
		__skb_queue_tail(txlist, skb);
	spin_unlock_bh(&txlist->lock);
	if (skb_queue_len(txlist) >= (TDMA_MAX_TX_PER_ROUND >> 1)) {
		if (!__netif_subqueue_stopped(sdata->dev, IEEE80211_AC_BE))
			netif_stop_subqueue(sdata->dev, IEEE80211_AC_BE);
#ifdef TDMA_DEBUG_TX_QUEUE_START_STOP
		printk("TDMA: Stop Queue\n");
#endif
	}
	if (tdma_is_tx_slot(tdma))
		ieee80211_tdma_tx_notify(&sdata->vif, true);
}


struct sk_buff *tdma_close_frame(struct ieee80211_sub_if_data *sdata, struct ieee80211_tx_control *control)
{
	struct ieee80211_if_tdma *tdma = &sdata->u.tdma;
	struct sk_buff *skb;
	struct ieee80211_mgmt *mgmt;
	int hdr_len = offsetof(struct ieee80211_mgmt, u.action.u.self_prot) + sizeof(mgmt->u.action.u.self_prot) + sizeof(u8);
	u8 next_node = tdma->node_id;

	if ((skb = dev_alloc_skb(sdata->local->tx_headroom + hdr_len)) == NULL)
		return NULL;
	skb_reserve(skb, sdata->local->tx_headroom);
	mgmt = (struct ieee80211_mgmt *)skb_put(skb, hdr_len);
	memset(mgmt, 0, hdr_len);
	mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
	memcpy(mgmt->sa, sdata->vif.addr, ETH_ALEN);
	memcpy(mgmt->bssid, sdata->vif.bss_conf.bssid, ETH_ALEN);
	mgmt->u.action.category = WLAN_CATEGORY_SELF_PROTECTED;
	mgmt->u.action.u.self_prot.action_code = WLAN_SP_MESH_PEERING_CLOSE;
	eth_broadcast_addr(mgmt->da);
	if (control) {
		struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
		struct sta_info *sta;

		control->sta = NULL;
		switch (TDMA_CFG_VERSION(tdma)) {
		case 2:
			if (tdma->node_num > 2) {
				next_node = tdma->b_counter;
				if (next_node == 0)
					next_node = tdma->node_num;
				else
					next_node++;
			} else
				next_node = 2;
			break;
		case 3:
			next_node = 1;
			break;
		default:
			next_node++;
			if (next_node > tdma->node_num)
				next_node = 1;
			break;
		}
		info = ptdma_skb_fill_info(sdata, skb);
		rcu_read_lock();
		list_for_each_entry_rcu(sta, &sdata->local->sta_list, list) {
			if (next_node == sta->n.aid) {
				info->flags |= IEEE80211_TX_UNICAST;
				info->control.rates[0].idx = sta->tx_stats.last_rate.idx;
				info->control.rates[0].count = 3;
				info->control.rates[0].flags = sta->tx_stats.last_rate.flags;
				info->control.rates[0].flags &= ~(IEEE80211_TX_RC_USE_CTS_PROTECT | IEEE80211_TX_RC_USE_RTS_CTS);
				memcpy(mgmt->da, sta->sta.addr, ETH_ALEN);
				control->sta = &sta->sta;
				break;
			}
		}
		rcu_read_unlock();
	}
	mgmt->u.action.u.self_prot.variable[0] = next_node;
	return skb;
}


int tdma_check_break_tx(struct ieee80211_if_tdma *tdma, struct sk_buff_head *txlist, u8 ver, unsigned long tx, unsigned long usedtime)
{
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);
	int idx = 0;

	if (txlist != NULL)
		idx = (int)skb_queue_len(txlist);

	if (tdma->optimize_tx && TDMA_CFG_POLLING(tdma) && ((idx == 0) || (tdma->last_tx >= 24)) && (tdma->last_tx_start > 0) && (usedtime >= tdma->poll_interval) && ((tx - usedtime) > 1024)) {
		struct sk_buff *skb;
		struct ieee80211_tx_control control;

		if ((skb = tdma_close_frame(sdata, &control)) != NULL) {
			tdma->last_tx_start = 0;
			netif_trans_update(sdata->dev);
			sdata->local->ops->tx(&sdata->local->hw, &control, skb);
			del_timer_sync(&tdma->poll_timer);
			idx = 0;
		}
	}
	return idx;
}


void tdma_poll_timer(struct timer_list *t)
{
	struct ieee80211_if_tdma *tdma= from_timer(tdma, t, poll_timer);
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);

	if (TDMA_JOINED(tdma) && (TDMA_STATE(tdma) != TDMA_STATUS_UNKNOW) && tdma_is_tx_slot(tdma)) {
		ieee80211_tdma_tx_notify(&sdata->vif, true);
		mod_timer(&tdma->poll_timer, jiffies + tdma->poll_timer_interval);
	}
}


unsigned long tdma_get_frame_duration_prognose(struct ieee80211_if_tdma *tdma, int len, struct ieee80211_tx_rate *txrate, struct sk_buff *skb, struct sta_info **sta)
{
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);
	unsigned long duration;
	bool ack = ! !(txrate->count > 1), brdcast;
	int erp = 0, bitrate;
	struct ieee80211_supported_band *sband = sdata->local->hw.wiphy->bands[tdma->chandef.chan->band];
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;

	*sta = NULL;
#ifdef CPTCFG_MAC80211_TDMA_MESH
	if (TDMA_IS_MESHED(tdma) && ieee80211_is_data_present(hdr->frame_control))
		brdcast = tdma_originator_get(tdma, hdr->addr3, hdr->addr1);
	else
#endif
		brdcast = is_multicast_ether_addr(hdr->addr1);
	if (!brdcast) {
		if ((*sta = (struct sta_info *)rcu_dereference(tdma->sta)) == NULL) {
			rcu_read_lock();
			*sta = tdma->tdma_sta_info_get(sdata, hdr->addr1);
			rcu_read_unlock();
		}
	}
	bitrate = ptdma_adjust_rates(sband, txrate, &erp, ack);
	duration = tdma->tdma_frame_duration(sband->band, skb->len + len, bitrate, erp, sdata->vif.bss_conf.use_short_preamble, tdma->tdma_chandef_get_shift(&tdma->chandef));
	duration += TDMA_TX_TAIL_SPACE;
	if (ack) {
		duration += tdma->ack_duration;
		duration += (duration << 1);
	}
	return duration;
}

