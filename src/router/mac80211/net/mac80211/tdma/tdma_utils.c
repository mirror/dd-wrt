#include <linux/random.h>
#include <linux/time.h>
#include <linux/export.h>
#include <linux/random.h>
#include <linux/time.h>
#include "tdma_i.h"
#include "../driver-ops.h"
#include "../rate.h"
#include "pdwext.h"

#undef EXPORT_SYMBOL
#define EXPORT_SYMBOL(a)

int tdma_adjust_rates(struct ieee80211_supported_band *sband, struct ieee80211_tx_rate *irate, int *erp, bool ack)
{
	return ptdma_adjust_rates(sband, irate, erp, ack);
}

EXPORT_SYMBOL(tdma_adjust_rates);

struct ieee80211_tx_info *tdma_skb_fill_info(struct ieee80211_sub_if_data *sdata, struct sk_buff *skb)
{
	return ptdma_skb_fill_info(sdata, skb);
}

EXPORT_SYMBOL(tdma_skb_fill_info);

unsigned long tdma_tu_adjust(unsigned long val, int slotsize, int shift, bool noack)
{
	return ptdma_tu_adjust(val, slotsize, shift, noack);
}

EXPORT_SYMBOL(tdma_tu_adjust);

unsigned long tdma_calc_ideal_interval(struct ieee80211_if_tdma *tdma, int node)
{
	return ptdma_calc_ideal_interval(tdma, node);
}

EXPORT_SYMBOL(tdma_calc_ideal_interval);

u8 tdma_get_avail_slot(struct ieee80211_if_tdma *tdma)
{
	size_t i;
	u8 slot = 0, rnd;
	u16 test;
	u16 slots[TDMA_MAX_NODE_PER_CELL];

	if (TDMA_CFG_VERSION(tdma) == 2)
		return 1;
	for (i = 0; i < tdma->node_num; i++) {
		test = (1 << i);
		if ((tdma->node_bitmap & test) == 0)
			slots[slot++] = i + 1;
	}
	if (slot > 0) {
		do
			get_random_bytes(&rnd, 1);
		while (rnd >= slot);
		slot = slots[rnd];
	}

	return slot;
}

EXPORT_SYMBOL(tdma_get_avail_slot);

bool tdma_sta_init(struct sta_info * sta)
{
	skb_queue_head_init(&sta->n.pending);
	skb_queue_head_init(&sta->n.tx);
	return false;
}

EXPORT_SYMBOL(tdma_sta_init);

void tdma_update_skb_hdr(struct ieee80211_if_tdma *tdma, struct sk_buff *skb, struct sta_info *sta, int meshhdrlen)
{
	u8 *in = skb_push(skb, meshhdrlen);

	if (in != NULL) {
		struct tdma_hdr *thdr = (struct tdma_hdr *)in;

		memset(in, 0, meshhdrlen);
		TDMA_HDR_SET_MAX_TTL(thdr);
		if (sta != NULL) {
			sta->n.sc++;
			if (sta->n.sc == 0)
				sta->n.sc++;
			thdr->sequence = sta->n.sc;
		} else {
			tdma->sc++;
			if (tdma->sc == 0)
				tdma->sc++;
			thdr->sequence = tdma->sc;
		}
	}
}

EXPORT_SYMBOL(tdma_update_skb_hdr);

unsigned long tdma_max_clock_drift(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_supported_band *sband;
	struct ieee80211_if_tdma *tdma = &sdata->u.tdma;
	int erp = 0, shift = tdma->tdma_chandef_get_shift(&tdma->chandef), bmtu = TDMA_MAX_BEACON_LEN;
	struct ieee80211_rate *rate;
	bool gack = TDMA_CFG_GACK(tdma);
	u8 ver = TDMA_CFG_VERSION(tdma);

	sband = sdata->local->hw.wiphy->bands[tdma->chandef.chan->band];
	if (ver == 0) {
		rate = &sband->bitrates[sdata->u.tdma.cur_rate];

		if (sdata->flags & IEEE80211_SDATA_OPERATING_GMODE)
			erp = rate->flags & IEEE80211_RATE_ERP_G;
	}
	if (gack)
		bmtu += (tdma->node_num - 1) * (3 + (TDMA_MAX_SEQS_IN_BEACON_SZ >> shift));
	else
		bmtu += (tdma->node_num - 1) * 3;
	return (unsigned long)tdma->tdma_frame_duration(sband->band, bmtu, tdma->rate, erp, sdata->vif.bss_conf.use_short_preamble, shift) + TDMA_TX_TAIL_SPACE;
}

EXPORT_SYMBOL(tdma_max_clock_drift);

unsigned long tdma_tx_slot_calc(struct ieee80211_sub_if_data *sdata, unsigned int mtu)
{
	int erp = 0, shift;
	unsigned long msecs, bsecs, rounded_msecs;
	struct ieee80211_supported_band *sband;
	struct ieee80211_if_tdma *tdma = &sdata->u.tdma;
	struct ieee80211_rate *rate;
	bool gack = TDMA_CFG_GACK(tdma);
	u8 ver = TDMA_CFG_VERSION(tdma);

	sband = sdata->local->hw.wiphy->bands[tdma->chandef.chan->band];
	if (TDMA_CFG_VERSION(tdma) == 0) {
		rate = &sband->bitrates[sdata->u.tdma.cur_rate];

		if (sdata->flags & IEEE80211_SDATA_OPERATING_GMODE)
			erp = rate->flags & IEEE80211_RATE_ERP_G;
	}
	shift = tdma->tdma_chandef_get_shift(&tdma->chandef);
	mtu += TDMA_MAX_HDR_LEN();
	msecs = tdma_max_clock_drift(sdata);
	bsecs = tdma->tdma_frame_duration(sband->band, mtu, tdma->rate, erp, sdata->vif.bss_conf.use_short_preamble, shift);
	if (gack || (ver == 0))
		msecs += bsecs;
	else
		msecs += (3 * (TDMA_TX_TAIL_SPACE + bsecs + tdma->ack_duration));
	rounded_msecs = ptdma_tu_adjust(msecs, tdma->slot_size, shift, ((ver == 0) || gack));
#ifdef TDMA_DEBUG
	printk("TDMA: TX Slot duration in uSec - %lu and rounded in uSec - %lu\n", msecs, rounded_msecs);
#endif
	return rounded_msecs;
}

EXPORT_SYMBOL(tdma_tx_slot_calc);

u32 tdma_adjust_basic_rates(struct ieee80211_sub_if_data *sdata, int shift, u32 supp_rates)
{
	struct ieee80211_if_tdma *tdma = &sdata->u.tdma;
	int i;
	struct ieee80211_supported_band *sband = sdata->local->hw.wiphy->bands[tdma->chandef.chan->band];

	for (i = 0; i < sband->n_bitrates; i++) {
		if ((sband->bitrates[i].bitrate < tdma->rate) || ((shift > 0) && (sband->bitrates[i].bitrate == 110)))
			supp_rates &= ~BIT(i);
		if (sband->bitrates[i].bitrate == tdma->rate)
			tdma->cur_rate = i;
	}
	return supp_rates;
}

EXPORT_SYMBOL(tdma_adjust_basic_rates);

void tdma_setup_polling(struct ieee80211_if_tdma *tdma)
{

	if (TDMA_CFG_POLLING(tdma)) {
		unsigned usecs = jiffies_to_usecs(1);

		if (usecs > 4000) {
			TDMA_CFG_SET_POLLING(tdma, false);
#ifdef TDMA_DEBUG
			printk("TDMA: Jiffies accuracy is too poor for polling. 1 jiffie = %u usecs. Polling is disabled\n", usecs);
#endif
		} else {
			int divider;

			tdma->poll_interval = 2048;
			if (usecs < tdma->poll_interval)
				usecs = tdma->poll_interval;
			divider = (int)(tdma->tx_slot_duration / usecs);
			if (divider > 4)
				divider = 4;
			if (divider < 2)
				divider = 2;
			tdma->poll_timer_interval = (int)(tdma->tx_slot_duration / divider);
			if (tdma->poll_timer_interval > 4000)
				tdma->poll_timer_interval = 4000;
			tdma->poll_timer_interval = usecs_to_jiffies(tdma->poll_timer_interval);

#ifdef TDMA_DEBUG
			printk("TDMA: Polling interval in jiffies - %d, in microseconds - %u\n", (int)tdma->poll_timer_interval, jiffies_to_usecs(tdma->poll_timer_interval));
#endif
		}
	}
}

EXPORT_SYMBOL(tdma_setup_polling);


void tdma_reset_state(struct ieee80211_if_tdma *tdma)
{
	struct ieee80211_sub_if_data *sdata = container_of(tdma, struct ieee80211_sub_if_data, u.tdma);
	u8 rnd;
	struct sk_buff *skb;
	int ac;

	TDMA_SET_STATE(tdma, TDMA_STATUS_UNKNOW);
	TDMA_SET_INITIALIZED(tdma, false);
	mb();
	netif_carrier_off(sdata->dev);
	del_timer_sync(&tdma->poll_timer);
	if (TDMA_JOINED(tdma)) {
		sdata->vif.bss_conf.ibss_creator = false;
		sdata->vif.bss_conf.ibss_joined = false;
		ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_IBSS);
	}
	if (TDMA_CFG_VERSION(tdma) == 2) {
		rnd = TDMA_MIN_UNKNOW_STATE_COUNT;
	} else {
		do
			get_random_bytes(&rnd, 1);
		while ((rnd < TDMA_MIN_UNKNOW_STATE_COUNT) || (rnd > TDMA_MAX_UNKNOW_STATE_COUNT));
	}
	tdma->unknow_state_counter = rnd;
	tdma->node_id = 0;
	tdma->last_tx = 0;
	tdma->last_tx_start = 0;
	tdma->counter = 0;
	tdma->node_bitmap = tdma->cell_bitmap = 0;
	tdma->last_verified = 0;
	tdma->last_own_beacon = 0;
	tdma->last_beacon_node = 0;
	tdma->last_beacon = 0;
	tdma->b_counter = 0;
	sdata->vif.bss_conf.aid = 0;
	sdata->vif.bss_conf.ibss_creator = false;
	sdata->vif.bss_conf.ibss_joined = false;
	sdata->vif.bss_conf.assoc = false;
#ifdef TDMA_DEBUG
	printk("TDMA: USC - %u\n", (u32)tdma->unknow_state_counter);
#endif
	sta_info_flush(sdata);
	while ((skb = skb_dequeue(&tdma->tx_skb_list))) {
		ieee80211_free_txskb(&sdata->local->hw, skb);
	}
	RCU_INIT_POINTER(tdma->sta, NULL);
	ieee80211_clear_tx_pending(sdata->local);
	ieee80211_flush_queues(sdata->local, sdata, false);
	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++) {
		if (__netif_subqueue_stopped(sdata->dev, ac)) {
			netif_wake_subqueue(sdata->dev, ac);
#ifdef TDMA_DEBUG
			printk("TDMA: Reset starts Intf Queue %d\n", ac);
#endif
		}
		if (ieee80211_queue_stopped(&sdata->local->hw, ac)) {
			ieee80211_wake_queue(&sdata->local->hw, ac);
#ifdef TDMA_DEBUG
			printk("TDMA: Reset starts IEEE80211 Queue %d\n", ac);
#endif
		}
	}
	synchronize_net();
	tdma_originator_expire(tdma, -1L);
#ifdef CPTCFG_MAC80211_TDMA_MESH
	tdma_retr_expire(tdma, -1L);
#endif
}

void tdma_sta_cleanup(struct sta_info *sta)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(&sta->n.pending))) {
		ieee80211_free_txskb(&sta->sdata->local->hw, skb);
	}
	while ((skb = skb_dequeue(&sta->n.tx))) {
		ieee80211_free_txskb(&sta->sdata->local->hw, skb);
	}
}
