#include <linux/ip.h>
#include "pnwext.h"
#include "../ieee80211_i.h"

#undef EXPORT_SYMBOL
#define EXPORT_SYMBOL(a)

static const unsigned ht20_mcs_rates[] = { 65, 130, 195, 260, 390, 520, 585, 650,
	130, 260, 390, 520, 780, 1040, 1170, 1300,
	195, 390, 585, 780, 1170, 1560, 1755, 1950,
	260, 520, 780, 1040, 1560, 2080, 2340, 2600
};

static const unsigned ht20sgi_mcs_rates[] = { 72, 144, 217, 289, 433, 578, 650, 722,
	144, 289, 433, 578, 867, 1156, 1303, 1444,
	217, 433, 650, 867, 1300, 1733, 19500, 2167,
	289, 578, 867, 1156, 1733, 2311, 2600, 2889
};

static const unsigned ht40_mcs_rates[] = { 135, 270, 405, 540, 810, 1080, 1215, 1350,
	270, 540, 810, 1080, 1620, 2160, 2430, 2700,
	405, 810, 1215, 1620, 2430, 3240, 3645, 4050,
	540, 1080, 1620, 2160, 3240, 4320, 4860, 5400
};

static const unsigned ht40sgi_mcs_rates[] = { 150, 300, 450, 600, 900, 1200, 1350, 1500,
	300, 600, 900, 1200, 1800, 2400, 2700, 3000,
	450, 900, 1350, 1800, 270, 3600, 4050, 4500,
	600, 1200, 1800, 240, 360, 4800, 5400, 6000
};

static const unsigned vht80_rates[3][10] = {
	{293, 585, 878, 1170, 1755, 2340, 2633, 2925, 3510, 3900},
	{585, 1170, 1755, 2340, 3510, 4680, 5265, 5850, 7020, 7800},
	{878, 1755, 2633, 3510, 5265, 7020, 10, 8775, 10530, 11700},
};

static const unsigned vht80sgi_rates[3][10] = {
	{325, 650, 975, 1300, 1950, 2600, 2925, 3250, 3900, 4333},
	{650, 1300, 1950, 2600, 3900, 5200, 5850, 6500, 7800, 8667},
	{975, 1920, 2925, 3900, 5850, 7800, 10, 9750, 11700, 13000},
};

static const unsigned vht160_rates[3][10] = {
	{585, 1170, 1755, 2340, 3510, 4680, 5265, 5850, 7020, 7800},
	{1170, 2340, 3510, 4680, 7020, 9360, 10530, 11700, 14040, 15600},
	{1755, 3510, 5265, 7020, 10530, 14040, 15795, 17550, 21060, 10},
};

static const unsigned vht160sgi_rates[3][10] = {
	{650, 1300, 1950, 2600, 3900, 5200, 5850, 6500, 7800, 8667},
	{1300, 2600, 3900, 5200, 7800, 10400, 11700, 13000, 15600, 17333},
	{1950, 3900, 5850, 7800, 11700, 15600, 17550, 19500, 23400, 10},
};

unsigned long ptdma_tx_slot_duration(struct ieee80211_if_tdma *tdma)
{
	return (tdma->tx_slot_duration - tdma->max_clock_drift);
}

EXPORT_SYMBOL(ptdma_tx_slot_duration);

unsigned long ptdma_calc_ideal_interval(struct ieee80211_if_tdma *tdma, int node)
{

	if ((node > 0) && (tdma->node_id > 0)) {
		int i;
		u8 ver = TDMA_CFG_VERSION(tdma);

		if ((ver == 2) || ((ver == 3) && (tdma->node_num == 2)))
			return tdma->second_tx_slot_duration;
		else {
			unsigned long res = tdma->second_tx_slot_duration;

			if (tdma->node_id < node) {
				i = tdma->node_num - node + tdma->node_id;
				if ((ver == 3) && (i > 0))
					i--;
			} else
				i = tdma->node_id - node;
			if (i >= 0) {
				while (i) {
					res += (tdma->tx_slot_duration + tdma->second_tx_slot_duration);
					i--;
				}
				return res;
			}
		}
	}
	return 0;
}

EXPORT_SYMBOL(ptdma_calc_ideal_interval);

int pamsdu_limit(struct ieee80211_supported_band *sband, struct ieee80211_sta *sta)
{
	if ((sband != NULL) && sband->ht_cap.ht_supported && (sta != NULL) && sta->ht_cap.ht_supported) {
		if ((sband->ht_cap.cap & IEEE80211_HT_CAP_MAX_AMSDU) && (sta->ht_cap.cap & IEEE80211_HT_CAP_MAX_AMSDU))
			return (IEEE80211_MAX_DATA_LEN_DMG + 48);
		return 3849;
	}
	return IEEE80211_MAX_FRAME_LEN_NONSTD;
}

EXPORT_SYMBOL(pamsdu_limit);

int ptdma_adjust_rates(struct ieee80211_supported_band *sband, struct ieee80211_tx_rate *irate, int *erp, bool ack)
{
	int bitrate = 10;

	if (irate != NULL) {
		irate->count = (ack ? 3 : 1);
		if (irate->idx < 0)
			irate->idx = 0;
		if (irate->flags & IEEE80211_TX_RC_VHT_MCS) {
			bool sgi = irate->flags & IEEE80211_TX_RC_SHORT_GI;
			u8 nss = ((irate->idx & 0xF0) >> 4), idx = (irate->idx & 0xF);

			if (nss > 3)
				nss = 3;
			if (idx > 9)
				idx = 9;
			if (irate->flags & IEEE80211_TX_RC_160_MHZ_WIDTH) {
				bitrate = (sgi ? vht160sgi_rates[nss][idx] : vht160_rates[nss][idx]);
			} else if (irate->flags & IEEE80211_TX_RC_80_MHZ_WIDTH) {
				bitrate = (sgi ? vht80sgi_rates[nss][idx] : vht80_rates[nss][idx]);
			}
		} else if (irate->flags & IEEE80211_TX_RC_MCS) {
			bool sgi = irate->flags & IEEE80211_TX_RC_SHORT_GI;

			if (irate->idx >= 32)
				irate->idx = 7;
			if (irate->flags & IEEE80211_TX_RC_40_MHZ_WIDTH) {
				bitrate = (sgi ? ht40sgi_mcs_rates[irate->idx] : ht40_mcs_rates[irate->idx]);
			} else {
				bitrate = (sgi ? ht20sgi_mcs_rates[irate->idx] : ht20_mcs_rates[irate->idx]);
			}
		} else {
			if (irate->idx >= sband->n_bitrates)
				irate->idx = 0;
			bitrate = sband->bitrates[irate->idx].bitrate;
			*erp = sband->bitrates[irate->idx].flags & IEEE80211_RATE_ERP_G;
		}
		irate->flags &= ~(IEEE80211_TX_RC_USE_CTS_PROTECT | IEEE80211_TX_RC_USE_RTS_CTS);
	}
	return bitrate;
}

EXPORT_SYMBOL(ptdma_adjust_rates);

static unsigned long ss_ack_table[4][5] = {
	{45, 45, 45, 45, 130},
	{26, 30, 38, 26, 130},
	{12, 22, 34, 12, 130},
	{10, 18, 31, 10, 130},
};

static unsigned long ss_noack_table[4][5] = {
	{8, 9, 14, 6, 50},
	{8, 9, 14, 6, 50},
	{8, 9, 14, 6, 50},
	{6, 8, 12, 6, 50},
};

unsigned long ptdma_tu_adjust(unsigned long val, int slotsize, int shift, bool noack)
{
	unsigned long tmp = val >> 10, minval;

	if ((tmp << 10) < val)
		tmp++;
	if (noack)
		minval = ss_noack_table[slotsize][shift];
	else
		minval = ss_ack_table[slotsize][shift];
	if (tmp < minval)
		tmp = minval;
	return tmp << 10;
}

EXPORT_SYMBOL(ptdma_tu_adjust);

void ptdma_update_hdr_counter(struct ieee80211_if_tdma *tdma, struct sk_buff *skb, struct ieee80211_hdr *hdr)
{
	struct tdma_hdr *thdr;
	u8 count;

	thdr = (struct tdma_hdr *)(skb->data + tdma->tdma_hdrlen(hdr->frame_control));
	count = TDMA_HDR_COUNTER(thdr);
	count++;
	TDMA_HDR_SET_COUNTER(thdr, count);
}

EXPORT_SYMBOL(ptdma_update_hdr_counter);

bool ptdma_update_hdr_ttl(struct ieee80211_if_tdma *tdma, struct sk_buff *skb, struct ieee80211_hdr *hdr)
{
	struct tdma_hdr *thdr;
	u8 count;

	thdr = (struct tdma_hdr *)(skb->data + tdma->tdma_hdrlen(hdr->frame_control));
	count = TDMA_HDR_TTL(thdr);
	if (count > 0)
		count--;
	TDMA_HDR_SET_TTL(thdr, count);
	return ! !(count == 0);
}

EXPORT_SYMBOL(ptdma_update_hdr_ttl);

void ptdma_update_hdr_info(struct ieee80211_if_tdma *tdma, struct sk_buff *skb, struct ieee80211_hdr *hdr, bool info)
{
	struct tdma_hdr *thdr;

	thdr = (struct tdma_hdr *)(skb->data + tdma->tdma_hdrlen(hdr->frame_control));
	TDMA_HDR_SET_INFO(thdr, info);
}

EXPORT_SYMBOL(ptdma_update_hdr_info);

struct ieee80211_tx_info *ptdma_skb_fill_info(struct ieee80211_sub_if_data *sdata, struct sk_buff *skb)
{
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

	memset(info, 0, sizeof(*info));
	info->flags |= (IEEE80211_TX_INTFL_DONT_ENCRYPT | IEEE80211_TX_CTL_NO_ACK | IEEE80211_TX_CTL_CLEAR_PS_FILT | IEEE80211_TX_CTL_ASSIGN_SEQ | IEEE80211_TX_CTL_FIRST_FRAGMENT | IEEE80211_TX_CTL_INJECTED);
	info->control.vif = &sdata->vif;
	info->control.rates[0].idx = sdata->u.tdma.cur_rate;
	info->control.rates[0].count = 1;
	info->control.rates[1].idx = -1;
	info->hw_queue = IEEE80211_AC_BE;
	info->band = sdata->u.tdma.chandef.chan->band;
	skb->queue_mapping = info->hw_queue;
	skb->priority = 0;
	skb->dev = sdata->dev;
	return info;
}

EXPORT_SYMBOL(ptdma_skb_fill_info);

bool ptdma_skb_priority(struct sk_buff * skb)
{
	bool priority = false;

	if (skb->protocol == htons(ETH_P_ARP))
		priority = true;
	else if (skb->protocol == htons(ETH_P_IP)) {
		struct iphdr *iph = (struct iphdr *)skb_network_header(skb);
		if (iph->protocol == IPPROTO_ICMP)
			priority = true;
	}
	return priority;
}

EXPORT_SYMBOL(ptdma_skb_priority);

bool ptdma_do_msdu(struct ieee80211_if_tdma * tdma)
{
	return ! !(TDMA_CFG_NO_MSDU(tdma) == 0);
}

EXPORT_SYMBOL(ptdma_do_msdu);
