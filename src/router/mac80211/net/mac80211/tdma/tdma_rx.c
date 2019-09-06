#include <linux/module.h>
#include <linux/random.h>
#include <linux/time.h>
#include <linux/export.h>


#include "tdma_i.h"
#include "../driver-ops.h"
#include "../rate.h"
#include "../key.h"
#include "../aes_ccm.h"

#include "pdwext.h"
#undef EXPORT_SYMBOL
#define EXPORT_SYMBOL(a)

#define COMPR_TYPE_LZO	1
#define COMPR_TYPE_LZMA	2
#define COMPR_TYPE_LZ4	3
#define COMPR_TYPE_ZSTD	4

/* See IEEE 802.1H for LLC/SNAP encapsulation/decapsulation */
/* Ethernet-II snap header (RFC1042 for most EtherTypes) */
const unsigned char s_rfc1042_header[] __aligned(2) = {
0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};

/* Bridge-Tunnel header (for EtherTypes ETH_P_AARP and ETH_P_IPX) */
const unsigned char s_bridge_tunnel_header[] __aligned(2) = {
0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8};

void tdma_amsdu_to_8023s(struct ieee80211_sub_if_data *sdata, struct sk_buff *skb, struct sk_buff_head *list, const unsigned int extra_headroom, __be16 len, u16 to_skip)
{
	struct sk_buff *frame = NULL;
	u16 ethertype;
	u8 *payload, *in;
	int remaining;
	u8 dst[ETH_ALEN], src[ETH_ALEN];
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	bool first = true;
	u8 compression = 0;

	memcpy(dst, ieee80211_get_DA(hdr), ETH_ALEN);
	memcpy(src, ieee80211_get_SA(hdr), ETH_ALEN);
	in = (u8 *)skb_pull(skb, to_skip);
	if (!in)
		goto out;

	while (skb != frame) {
		bool compressed = false;
		u16 clen = 0;
		size_t outlen = 0, inlen, extralen = 0;

		if (!first) {
			len = get_unaligned_le16((void *)in);
			skb_pull(skb, sizeof(__le16));
			clen = sizeof(__le16);
		}
		if (len & TDMA_HDR_COMPRESSED) {
			len &= ~(TDMA_HDR_COMPRESSED | TDMA_HDR_ADD_INFO);
			compressed = true;
		}
		clen += len;
		first = false;
		remaining = skb->len;
		if (len > remaining)
			goto purge;

		if (compressed) {
#if IS_ENABLED(CPTCFG_MAC80211_COMPRESS)
			in = (u8 *)skb->data;
			inlen = (size_t)get_unaligned_le16((void *)in);
			in += sizeof(__le16);
			compression = (u8)get_unaligned((u8 *)in);
			in += sizeof(u8);
			extralen = len - inlen - sizeof(__le16) - sizeof(u8);
//                  spin_lock(&sdata->rx_cmpr_lock);

			switch (compression) {
			case COMPR_TYPE_LZO:
			case COMPR_TYPE_LZMA:
			case COMPR_TYPE_LZ4:
#ifdef ZSTD
			case COMPR_TYPE_ZSTD:
#endif
				if (decompress_wrapper)
					outlen = decompress_wrapper(sdata, in, inlen, compression);
			default:
				goto purge;
			}

			if (extralen) {
				memcpy((u8 *)((u8 *)sdata->rx_outbuf + outlen), (u8 *)(in + inlen), extralen);
				outlen += extralen;
			}
#else
#ifdef TDMA_DEBUG
			printk("TDMA: Compressed AMSDU subframe, but compression is disabled!\n");
#endif
			goto purge;
#endif
		}
		/* reuse skb for the last subframe */
		if (remaining <= len) {
			frame = skb;
			if (compressed) {
#if IS_ENABLED(CPTCFG_MAC80211_COMPRESS)
				if (skb_tailroom(frame) < (outlen - remaining)) {
					if (pskb_expand_head(frame, 0, (outlen - remaining) - skb_tailroom(frame), GFP_ATOMIC)) {
#ifdef TDMA_DEBUG
						printk("TDMA: Could not expand original skb\n");
#endif
//                                  spin_unlock(&sdata->rx_cmpr_lock);
						goto purge;
					}
				}
				memcpy(frame->data, sdata->rx_outbuf, outlen);
//                          spin_unlock(&sdata->rx_cmpr_lock);
				frame->len = outlen;
#endif
			}
		} else {
			unsigned int hlen = ALIGN(extra_headroom, 4);
			/*
			 * Allocate and reserve two bytes more for payload
			 * alignment since sizeof(struct ethhdr) is 14.
			 */
			frame = dev_alloc_skb(hlen + ((compressed) ? outlen : len) + sizeof(struct ethhdr) + 2);
			if (!frame)
				goto purge;

			skb_reserve(frame, hlen + sizeof(struct ethhdr) + 2);
			if (compressed) {
#if IS_ENABLED(CPTCFG_MAC80211_COMPRESS)
				memcpy(skb_put(frame, outlen), sdata->rx_outbuf, outlen);
//                          spin_unlock(&sdata->rx_cmpr_lock);
#else
				printk("TDMA: Compressed AMSDU subframe, but compression is disabled!\n");
				goto purge;
#endif
			} else
				memcpy(skb_put(frame, len), skb->data, len);

			in = (u8 *)skb_pull(skb, len);
			if (!in) {
				dev_kfree_skb(frame);
				goto purge;
			}
		}

		skb_reset_network_header(frame);
		frame->dev = skb->dev;
		frame->priority = skb->priority;

		payload = frame->data;
		ethertype = (payload[6] << 8) | payload[7];

		if (likely((ether_addr_equal(payload, s_rfc1042_header) && ethertype != ETH_P_AARP && ethertype != ETH_P_IPX) || ether_addr_equal(payload, s_bridge_tunnel_header))) {
			/* remove RFC1042 or Bridge-Tunnel
			 * encapsulation and replace EtherType */
			skb_pull(frame, 6);
			memcpy(skb_push(frame, ETH_ALEN), src, ETH_ALEN);
			memcpy(skb_push(frame, ETH_ALEN), dst, ETH_ALEN);
		} else {
			if (compressed)
				len = outlen;
			len = htons(len);
			memcpy(skb_push(frame, sizeof(__be16)), &len, sizeof(__be16));
			memcpy(skb_push(frame, ETH_ALEN), src, ETH_ALEN);
			memcpy(skb_push(frame, ETH_ALEN), dst, ETH_ALEN);
		}
		__skb_queue_tail(list, frame);
	}

	return;

purge:
	__skb_queue_purge(list);
out:
	dev_kfree_skb(skb);
}

EXPORT_SYMBOL(tdma_amsdu_to_8023s);

#ifdef CPTCFG_MAC80211_TDMA_MESH
int tdma_retr_expire(struct ieee80211_if_tdma *tdma, long timeout)
{
	int expired = 0;
	struct p_bcast *o = tdma->bhead, *prev = NULL;

	while (o != NULL) {
		struct p_bcast *tmp = o->next;

		if ((timeout == -1L) || ((o->jiffies + timeout) < jiffies)) {
#ifdef TDMA_DEBUG_MESH_BCAST_EXPIRATION
			printk("TDMA_MESH: expire bcast to %pM from %pM with size %d\n", o->da, o->sa, (int)o->len);
#endif
			expired++;
			if (prev == NULL)
				tdma->bhead = tmp;
			else
				prev->next = tmp;
			kfree(o);
		} else
			prev = o;
		o = tmp;
	}
	return expired;
}

EXPORT_SYMBOL(tdma_retr_expire);

static bool tdma_check_retr(struct ieee80211_if_tdma *tdma, u16 len, struct ieee80211_hdr *hdr)
{
	bool res = true;
	struct tdma_hdr *thdr;
	struct p_bcast *tmp = tdma->bhead;

	if (!is_broadcast_ether_addr(hdr->addr1))
		return true;
	thdr = (struct tdma_hdr *)(hdr + tdma->tdma_hdrlen(hdr->frame_control));
	while (res && (tmp != NULL)) {
		if ((tmp->sequence == thdr->sequence) && (tmp->len == len) && (tmp->ttl > TDMA_HDR_TTL(thdr)) && ether_addr_equal(tmp->da, hdr->addr3) && ether_addr_equal(tmp->sa, hdr->addr4)) {
			tmp->jiffies = jiffies;
			res = false;
#ifdef TDMA_DEBUG_MESH_BCAST_GUARD
			printk("TDMA_MESH: drop bcast to %pM from %pM with size %d\n", tmp->da, tmp->sa, (int)tmp->len);
#endif
		}
		tmp = tmp->next;
	}
	if (res) {
		if ((tmp = kzalloc(sizeof(struct p_bcast), in_interrupt()? GFP_ATOMIC : GFP_KERNEL)) != NULL) {
			tmp->jiffies = jiffies;
			tmp->len = len;
			tmp->ttl = TDMA_HDR_TTL(thdr);
			tmp->sequence = thdr->sequence;
			memcpy(tmp->da, hdr->addr3, ETH_ALEN);
			memcpy(tmp->sa, hdr->addr4, ETH_ALEN);
			tmp->next = tdma->bhead;
			tdma->bhead = tmp;
		}
	}
	return res;
}
#endif

bool tdma_process_hdr(struct ieee80211_rx_data * rx, struct ieee80211_hdr * hdr, bool * amsdu, u16 *len, u16 *to_skip)
{
	struct ieee80211_if_tdma *tdma = &rx->sdata->u.tdma;
	bool res = true;
	int i;
	struct tdma_hdr *thdr;
	size_t sz = tdma->tdma_hdrlen(hdr->frame_control), sz1 = tdma_hdr_len();
	bool gack = TDMA_CFG_GACK(tdma);
	u8 ver = TDMA_CFG_VERSION(tdma);
#ifdef CPTCFG_MAC80211_TDMA_MESH
	bool retransmit = false, local = true;
#endif

	if (!TDMA_JOINED(tdma) || (TDMA_STATE(tdma) != TDMA_STATUS_ASSOCIATED))
		return false;
	if (ieee80211_has_a4(hdr->frame_control)) {
		if (ether_addr_equal(hdr->addr4, rx->sdata->vif.addr))
			return false;
		if ((tdma->node_num > 2) && (ver != 3) && rx->sta)
			tdma_originator_update_rx(tdma, hdr->addr4, hdr->addr2, rx->skb->len, rx->sta->rx_stats.last_signal);
	} else
		return false;
	if (!pskb_may_pull(rx->skb, sz + sz1))
		return false;
	hdr = (struct ieee80211_hdr *)rx->skb->data;
	sz = tdma->tdma_hdrlen(hdr->frame_control);
	thdr = (struct tdma_hdr *)(rx->skb->data + sz);
	if (TDMA_CFG_POLLING(tdma)) {
		if (TDMA_HDR_INFO(thdr)) {
			struct sk_buff *tmp;

			if ((tmp = tdma_close_frame(rx->sdata, NULL)) != NULL) {
				skb_queue_tail(&rx->sdata->skb_queue, tmp);
				queue_work(rx->sdata->local->workqueue, &rx->sdata->work);
			}
		}
	}
#ifdef CPTCFG_MAC80211_TDMA_MESH
	if (TDMA_IS_MESHED(tdma)) {
		tdma_originator_get_processing(tdma, hdr->addr3, hdr->addr1, &retransmit, &local);
		if (retransmit) {
			struct sk_buff *tmp;

			if (ptdma_update_hdr_ttl(tdma, rx->skb, hdr))
				return false;
			if (!tdma_check_retr(tdma, rx->skb->len, hdr))
				return false;
			if ((tmp = skb_copy(rx->skb, GFP_ATOMIC)) != NULL) {
				struct ieee80211_hdr *tmp_hdr;
				struct ieee80211_tx_info *info = ptdma_skb_fill_info(rx->sdata, tmp);
				bool to_head = false;

				skb_orphan(tmp);
				tmp->sk = NULL;
				tmp_hdr = (struct ieee80211_hdr *)tmp->data;
				tmp_hdr->frame_control &= ~cpu_to_le16(IEEE80211_FCTL_RETRY);
				tmp_hdr->seq_ctrl = cpu_to_le16(rx->sdata->sequence_number);
				rx->sdata->sequence_number += 0x10;
				eth_broadcast_addr(tmp_hdr->addr1);
				memcpy(tmp_hdr->addr2, rx->sdata->vif.addr, ETH_ALEN);
				info->control.skip_table = (ver == 0) ? 1 : 0;
				if (!TDMA_CFG_NO_REORDER(tdma))
					to_head = ptdma_skb_priority(tmp);
				spin_lock_bh(&tdma->tx_skb_list.lock);
				if (to_head)
					__skb_queue_head(&tdma->tx_skb_list, tmp);
				else
					__skb_queue_tail(&tdma->tx_skb_list, tmp);
				spin_unlock_bh(&tdma->tx_skb_list.lock);
				tdma->mesh_fwd++;
			}
			if (!local)
				return false;
		}
	}
#endif
	if (gack) {
		if (rx->sta) {
			for (i = 0; (i < rx->sta->n.num_rcvd && res); i++) {
				if (rx->sta->n.rcvd[i] == thdr->sequence)
					res = false;
			}
			for (i = 0; (i < rx->sta->n.num_seqs && res); i++) {
				if (rx->sta->n.seqs[i] == thdr->sequence)
					res = false;
			}
			if (rx->sta->n.num_seqs < TDMA_MAX_TX_QUEUE_LEN) {
				rx->sta->n.seqs[rx->sta->n.num_seqs] = thdr->sequence;
				rx->sta->n.num_seqs++;
				tdma->num_seqs++;
			}
		}
	}
	if (res) {
		*amsdu = TDMA_HDR_AMSDU(thdr);
		if (*amsdu) {
			if (skb_linearize(rx->skb))
				res = false;
			if (res) {
				*to_skip = sz + sz1;
				*len = le16_to_cpu(thdr->len);
			}
		}
	} else {
#ifdef CPTCFG_MAC80211_DEBUG_COUNTERS
		rx->local->dot11FrameDuplicateCount++;
#endif
		rx->sta->rx_stats.num_duplicates++;
	}
	return res;
}

EXPORT_SYMBOL(tdma_process_hdr);

static void tdma_close_timeslot(struct ieee80211_sub_if_data *sdata, u8 data)
{
	struct ieee80211_if_tdma *tdma = &sdata->u.tdma;

	if (!TDMA_JOINED(tdma) || (TDMA_STATE(tdma) == TDMA_STATUS_UNKNOW) || !TDMA_CFG_POLLING(tdma))
		return;
	if (data == tdma->node_id)
		tdma_plan_next_tx(tdma, 50);
}

static void tdma_rx_mgmt_action(struct ieee80211_sub_if_data *sdata, struct ieee80211_mgmt *mgmt, size_t len, struct ieee80211_rx_status *rx_status)
{
	switch (mgmt->u.action.category) {
	case WLAN_CATEGORY_SELF_PROTECTED:
		switch (mgmt->u.action.u.self_prot.action_code) {
		case WLAN_SP_MESH_PEERING_CLOSE:
			tdma_close_timeslot(sdata, mgmt->u.action.u.self_prot.variable[0]);
			break;
		case WLAN_SP_MESH_PEERING_OPEN:
		case WLAN_SP_MESH_PEERING_CONFIRM:
			break;
		}
		break;
	case WLAN_CATEGORY_MESH_ACTION:
		break;
	case WLAN_CATEGORY_SPECTRUM_MGMT:
		break;
	}
}

static void tdma_frame_acked(struct sta_info *sta, int rssi, int nseqs, u8 *start)
{
	struct ieee80211_if_tdma *tdma = &sta->sdata->u.tdma;
	struct ieee80211_sub_if_data *sdata = sta->sdata;
	struct sk_buff *skb, *tmp;
	int i;
	__le16 seqnum;
	u8 ver = TDMA_CFG_VERSION(tdma), *start_pos, *pos;
	struct sk_buff_head skbs;

	if (!TDMA_CFG_GACK(tdma))
		return;

	__skb_queue_head_init(&skbs);
	if (nseqs > TDMA_MAX_TX_QUEUE_LEN)
		nseqs = TDMA_MAX_TX_QUEUE_LEN;
	start_pos = start;
	if (!spin_trylock(&sta->n.pending.lock))
		return;
	skb_queue_walk_safe(&sta->n.pending, skb, tmp) {
		struct tdma_hdr *hdr = (struct tdma_hdr *)(skb->data + tdma->tdma_get_hdrlen_from_skb(skb));
		u8 count = TDMA_HDR_COUNTER(hdr);
		bool process = true;

		i = 0;
		pos = start_pos;
		while ((i < nseqs) && process) {
			seqnum = *pos;
			pos++;
			if (hdr->sequence == seqnum) {
				process = false;
				__skb_unlink(skb, &sta->n.pending);
				if (ver != 0) {
					struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

					info->status.rates[0].count = ((count < 4) ? 1 : ((count >= 9) ? 3 : 2));
					info->status.rates[1].idx = -1;
					info->flags &= ~(IEEE80211_TX_STAT_AMPDU_NO_BACK | IEEE80211_TX_CTL_NO_ACK);
					info->flags |= (IEEE80211_TX_STAT_ACK | IEEE80211_TX_CTL_INJECTED);
					info->status.ack_signal = rssi;
				}
			}
			i++;
		}
		if (process && (count > 8)) {
			process = false;
			__skb_unlink(skb, &sta->n.pending);
			if (ver != 0) {
				struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

				info->flags &= ~(IEEE80211_TX_CTL_NO_ACK | IEEE80211_TX_STAT_ACK);
				info->flags |= IEEE80211_TX_CTL_INJECTED;
				info->status.rates[0].count = 3;
				info->status.rates[1].idx = -1;
			}
		}
		if (!process)
			__skb_queue_tail(&skbs, skb);
	}
	spin_unlock(&sta->n.pending.lock);
	if (!skb_queue_empty(&skbs)) {
		if (ver == 0)
			__skb_queue_purge(&skbs);
		else {
			while ((skb = __skb_dequeue(&skbs))) {
				ieee80211_tx_status(&sdata->local->hw, skb);
			}
		}
	}
}

extern void ieee80211_send_layer2_update(struct sta_info *);

static struct sta_info *tdma_upd_sta_rcu(struct ieee80211_sub_if_data *sdata,
					 const u8 *addr, struct ieee802_11_elems *elems, tdma_state state, u8 tie_tx_ratio, u8 tie_rx_ratio,
					 u8 tie_node_num, u8 tie_cfg, u8 tie_rate, u8 tie_slot_size, u16 tie_node_bitmap, u8 tie_node_id, int signal)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_tdma *tdma = &sdata->u.tdma;
	struct sta_info *sta = NULL;
	struct cfg80211_chan_def sta_chan_def;
	bool rates_updated = false;
	u32 supp_rates;
	struct ieee80211_key *key = NULL, *newkey;
	unsigned long i, now = (unsigned long)(ktime_to_ns(ktime_get()) >> 10);
	u8 ver = TDMA_CFG_VERSION(tdma);

	rcu_read_lock();
	sta = sta_info_get(sdata, addr);
	if (!sta) {
		rcu_read_unlock();
		/*
		 * XXX: Consider removing the least recently used entry and
		 *  allow new one to be added.
		 */
		if (local->num_sta >= TDMA_MAX_NODE_PER_CELL) {
			net_info_ratelimited("%s: No room for a new TDMA STA entry %pM\n", sdata->name, addr);
			return NULL;
		}
		if ((sta = sta_info_alloc(sdata, addr, GFP_KERNEL)) == NULL) {
			return NULL;
		}
		key = key_mtx_dereference(sdata->local, sdata->default_unicast_key);
		if (key && (key->conf.cipher == WLAN_CIPHER_SUITE_CCMP)) {
			if ((newkey = kzalloc(sizeof(struct ieee80211_key) + key->conf.keylen, GFP_KERNEL))) {
				memcpy(newkey, key, sizeof(struct ieee80211_key));
				newkey->u.ccmp.tfm = ieee80211_aes_key_setup_encrypt(key->conf.key, 128, IEEE80211_CCMP_MIC_LEN);
				memcpy(newkey->conf.key, key->conf.key, key->conf.keylen);
				newkey->conf.flags |= IEEE80211_KEY_FLAG_PAIRWISE;
				if (ieee80211_key_link(newkey, sdata, sta)) {
					ieee80211_key_free(newkey, false);
#ifdef TDMA_DEBUG
					printk("TDMA: Could not link the key\n");
#endif
					sta_info_free(sdata->local, sta);
					return NULL;
				}
			} else {
#ifdef TDMA_DEBUG
				printk("TDMA: Could not allocate key\n");
#endif
				sta_info_free(sdata->local, sta);
				return NULL;
			}
		}
		sta->n.created = now;
		sta->listen_interval = 3;
		sta_info_pre_move_state(sta, IEEE80211_STA_AUTH);
		sta_info_pre_move_state(sta, IEEE80211_STA_ASSOC);
		sta_info_insert_rcu(sta);
		if ((ver == 3) || (tdma->node_num == 2)) {
			rcu_dereference(tdma->sta);
			rcu_assign_pointer(tdma->sta, sta);
		}
		rates_updated = true;
		ieee80211_set_wmm_default(sdata, true, sdata->vif.type != NL80211_IFTYPE_STATION);
		ieee80211_send_layer2_update(sta);
	}
	if (state == TDMA_STATUS_ASSOCIATED)
		sta_info_move_state(sta, IEEE80211_STA_AUTHORIZED);
	else
		sta_info_move_state(sta, IEEE80211_STA_ASSOC);
	sta->rx_stats.last_rx = jiffies;
	tdma->last_beacon_node = sta->n.aid = tie_node_id;
	sta->n.node_bitmap = (ver == 2) ? BIT(tie_node_id - 1) : tie_node_bitmap;
	sta->n.voted = false;
	sta->n.miss_own_beacon = true;
	tdma->last_beacon = sta->n.last_seen = now;
	if (elems->supp_rates) {
		u32 prev_rates;

		supp_rates = ieee80211_sta_get_rates(sdata, elems, tdma->chandef.chan->band, NULL);
		prev_rates = sta->sta.supp_rates[tdma->chandef.chan->band];
		if (ver) {
			/* make sure mandatory rates are always added */
			supp_rates |= ieee80211_mandatory_rates(sdata->local->hw.wiphy->bands[tdma->chandef.chan->band], cfg80211_chandef_to_scan_width(&tdma->chandef));
			for (i = 0; i < sdata->local->hw.wiphy->bands[tdma->chandef.chan->band]->n_bitrates; i++) {
				switch (tdma->slot_size) {
				case 1:
					if (sdata->local->hw.wiphy->bands[tdma->chandef.chan->band]->bitrates[i].bitrate < 20)
						supp_rates &= ~BIT(i);
					break;
				case 2:
					if (sdata->local->hw.wiphy->bands[tdma->chandef.chan->band]->bitrates[i].bitrate < 55)
						supp_rates &= ~BIT(i);
					break;
				case 3:
					if (sdata->local->hw.wiphy->bands[tdma->chandef.chan->band]->bitrates[i].bitrate < 60)
						supp_rates &= ~BIT(i);
					break;
				default:
					break;
				}
			}
		}
		sta->sta.supp_rates[tdma->chandef.chan->band] = supp_rates;

		if (sta->sta.supp_rates[tdma->chandef.chan->band] != prev_rates)
			rates_updated = true;

		if (elems->ht_operation && elems->ht_cap_elem && tdma->chandef.width != NL80211_CHAN_WIDTH_20_NOHT) {
			/* we both use HT */
			struct ieee80211_ht_cap htcap_ie;
			struct cfg80211_chan_def chandef;

			cfg80211_chandef_create(&sta_chan_def, sdata->vif.bss_conf.chandef.chan, NL80211_CHAN_NO_HT);
			ieee80211_chandef_ht_oper(elems->ht_operation, &sta_chan_def);

			memcpy(&htcap_ie, elems->ht_cap_elem, sizeof(htcap_ie));

			/*
			 * fall back to HT20 if we don't use or use
			 * the other extension channel
			 */
			if (chandef.center_freq1 != sdata->u.tdma.chandef.center_freq1)
				htcap_ie.cap_info &= cpu_to_le16(~IEEE80211_HT_CAP_SUP_WIDTH_20_40);

			rates_updated |= ieee80211_ht_cap_ie_to_sta_ht_cap(sdata, sdata->local->hw.wiphy->bands[tdma->chandef.chan->band], &htcap_ie, sta);
		}
	}
	if ((ver == 3) && !TDMA_INITIALIZED(tdma)) {

		TDMA_CFG_SET_GACK(tdma, TDMA_CFG_GACK_P(tie_cfg));
		TDMA_CFG_SET_NO_REORDER(tdma, TDMA_CFG_NO_REORDER_P(tie_cfg));
		TDMA_CFG_SET_NO_MSDU(tdma, TDMA_CFG_NO_MSDU_P(tie_cfg));
		TDMA_CFG_SET_POLLING(tdma, TDMA_CFG_POLLING_P(tie_cfg));
		if (tie_tx_ratio == 0)
			tie_tx_ratio = 1;
		if (tie_rx_ratio == 0)
			tie_rx_ratio = 1;
		tdma->tx_ratio = tie_tx_ratio;
		tdma->rx_ratio = tie_rx_ratio;
		tdma->slot_size = tie_slot_size;
		tdma->rate = tie_rate;
		tdma->cur_rate = 0;
		tdma->node_num = tie_node_num;
		i = ieee80211_chandef_get_shift(&tdma->chandef);
		sdata->vif.bss_conf.basic_rates =
		    tdma_adjust_basic_rates(sdata, i, sdata->vif.bss_conf.basic_rates | ieee80211_mandatory_rates(sdata->local->hw.wiphy->bands[tdma->chandef.chan->band], cfg80211_chandef_to_scan_width(&tdma->chandef)));
		if (tdma->chandef.chan->band == NL80211_BAND_2GHZ) {
			if ((tdma->slot_size == 3) || (i > 0)) {
				sdata->vif.bss_conf.use_short_preamble = true;
				sdata->flags |= IEEE80211_SDATA_OPERATING_GMODE;
			} else {
				sdata->flags &= ~IEEE80211_SDATA_OPERATING_GMODE;
				sdata->vif.bss_conf.use_short_preamble = false;
			}
		} else
			sdata->vif.bss_conf.use_short_preamble = true;
		tdma->ack_duration = TDMA_TX_TAIL_SPACE + ieee80211_frame_duration(tdma->chandef.chan->band, 10, tdma->rate, 0, sdata->vif.bss_conf.use_short_preamble, i);
		tdma->max_clock_drift = tdma_max_clock_drift(sdata);
		tdma->tx_slot_duration = tdma_tx_slot_calc(sdata, sdata->dev->mtu);
		if (tie_tx_ratio == tie_rx_ratio) {
			tdma->second_tx_slot_duration = tdma->tx_slot_duration;
		} else if (tie_tx_ratio < tie_rx_ratio) {
			tdma->second_tx_slot_duration = tdma->tx_slot_duration;
			tdma->tx_slot_duration = tdma_tu_adjust((tdma->tx_slot_duration / tie_tx_ratio) * tie_rx_ratio, tdma->slot_size, i, TDMA_CFG_GACK(tdma));
		} else
			tdma->second_tx_slot_duration = tdma_tu_adjust((tdma->tx_slot_duration / tie_rx_ratio) * tie_tx_ratio, tdma->slot_size, i, TDMA_CFG_GACK(tdma));
		tdma->tx_round_duration = (tdma->second_tx_slot_duration + tdma->tx_slot_duration) * (tie_node_num - 1);
		tdma_setup_polling(tdma);
		sdata->vif.bss_conf.beacon_int = (u16)(tdma->tx_round_duration >> 10);
		TDMA_SET_INITIALIZED(tdma, true);
		mb();
		ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_HT | BSS_CHANGED_BASIC_RATES | BSS_CHANGED_BSSID | BSS_CHANGED_SSID | BSS_CHANGED_ERP_PREAMBLE | BSS_CHANGED_ERP_SLOT);
#ifdef TDMA_DEBUG
		printk("TDMA: Round duration - %lu, TX slot duration - %lu, Second TX slot duration - %lu\n", tdma->tx_round_duration, tdma->tx_slot_duration, tdma->second_tx_slot_duration);
#endif
	}
	if (sta && ((ver == 0) || TDMA_CFG_GACK(tdma))) {
		sta->sta.wme = true;
		sdata->noack_map = 0xffff;
	}
	if (sta && rates_updated) {
		drv_sta_rc_update(local, sdata, &sta->sta, IEEE80211_RC_SUPP_RATES_CHANGED);
		rate_control_rate_init(sta);
	}
	if (sta && (ver != 3) && (tdma->node_num > 2))
		tdma_originator_update_rx(tdma, addr, addr, 0, signal);
	return sta;
}

static void tdma_rx_mgmt_beacon(struct ieee80211_sub_if_data *sdata, struct ieee80211_mgmt *mgmt, size_t len, struct ieee80211_rx_status *rx_status)
{
	struct ieee80211_if_tdma *tdma;
	u8 *start_pos, ie_len;
	int baselen;
	bool off_carrier = false;
	tdma_state state = TDMA_STATUS_UNKNOW;
	struct ieee802_11_elems elems;
	u8 tie_cfg;
	u8 tie_node_id;
	u8 tie_state;
	u8 tie_node_num;
	u8 tie_slot_size;
	u8 tie_tx_ratio;
	u8 tie_rx_ratio;
	u8 tie_bcn_for;
	u8 num_seqs;
	u8 over, ever;
	u16 tie_node_bitmap, num_bytes;
	u32 tie_rate;
	struct sta_info *sta;

	tdma = &sdata->u.tdma;
	if (!TDMA_JOINED(tdma))
		return;
	/* Process beacon */
	baselen = (u8 *)mgmt->u.beacon.variable - (u8 *)mgmt;
	if (baselen > len) {
#ifdef TDMA_DEBUG_BEACON_RX
		printk("TDMA: Wrong beacon len - %d\n", baselen - len);
#endif
		return;
	}

	ieee802_11_parse_elems(mgmt->u.beacon.variable, len - baselen, false, &elems);
	if (!elems.peering) {
		return;
	}
	/* check SSID */
	if ((elems.ssid_len != sdata->vif.bss_conf.ssid_len) || ((elems.ssid_len > 0) && memcmp(elems.ssid, sdata->vif.bss_conf.ssid, sdata->vif.bss_conf.ssid_len))) {
#ifdef TDMA_DEBUG_BEACON_RX
		printk("TDMA: Different SSID\n");
#endif
		return;
	}
	start_pos = (u8 *)elems.peering;
	ie_len = elems.peering_len;
	if (ie_len < TDMA_MIN_IE_SIZE) {
#ifdef TDMA_DEBUG_BEACON_RX
		printk("TDMA: Wrong ie len - %d\n", ie_len);
#endif
		return;
	}
	tie_cfg = *start_pos;
	start_pos++;
	tie_node_id = *start_pos;
	start_pos++;
	if ((tie_node_id == 0) || (tie_node_id > TDMA_MAX_NODE_PER_CELL)) {
#ifdef TDMA_DEBUG_BEACON_RX
		printk("TDMA: Wrong IE - %d\n", tie_node_id);
#endif
		return;
	}
	tie_state = *start_pos;
	start_pos++;
	tie_node_num = *start_pos;
	start_pos++;
	tie_slot_size = *start_pos;
	start_pos++;
	tie_tx_ratio = TDMA_IE_GET_TX_RATIO(*start_pos);
	tie_rx_ratio = TDMA_IE_GET_RX_RATIO(*start_pos);
	start_pos++;
	tie_bcn_for = *start_pos;
	start_pos++;
	over = TDMA_CFG_VERSION(tdma);
	ever = TDMA_CFG_VERSION_P(tie_cfg);
	if ((over == 3) && (ever != 2)) {
#ifdef TDMA_DEBUG_BEACON_RX_ALL
		printk("TDMA: Ignore beacons from other CPEs \n");
#endif
		return;
	}
	if ((over == 2) && (ever == 2)) {
#ifdef TDMA_DEBUG_BEACON_RX_ALL
		printk("TDMA: Ignore beacons from other BS \n");
#endif
		return;
	}
	if (((over == 2) && (ever != 3)) || ((over == 3) && (ever != 2)) || ((over < 2) && (ever != over))) {
#ifdef TDMA_DEBUG_BEACON_RX
		printk("TDMA: Wrong version - %d %d\n", over, ever);
#endif
		return;
	}
	if (tie_node_id > tdma_get_slot_num(tdma)) {
#ifdef TDMA_DEBUG_BEACON_RX
		printk("TDMA: Wrong node_id \n");
#endif
		return;
	}
	if ((over == 3) && (ever == 2)) {
		if ((elems.ssid_len == 0) && !ether_addr_equal(mgmt->sa, tdma->bssid)) {
#ifdef TDMA_DEBUG_BEACON_RX_ALL
			printk("TDMA: Ignore beacons from not our BS\n");
#endif
			return;
		}
	}
	state = TDMA_STATE(tdma);
	tie_node_bitmap = get_unaligned_le16((void *)start_pos);
	start_pos += sizeof(__le16);
	tie_rate = get_unaligned_le32((void *)start_pos);
	start_pos += sizeof(__le32);
	num_bytes = get_unaligned_le16((void *)start_pos);
	start_pos += sizeof(__le16);
	if (over == 0) {
		if (tie_rate != tdma->rate) {
#ifdef TDMA_DEBUG_BEACON_RX
			printk("TDMA: Rate mismatch\n");
#endif
			return;
		}
	}
	if (tie_node_id == tdma->node_id) {
#ifdef TDMA_DEBUG_BEACON_RX
		printk("TDMA: Uh-Oh! Beacon with the same ID received.\n");
#endif
		if ((tie_state == TDMA_STATUS_ASSOCIATED) && (state != TDMA_STATUS_UNKNOW)) {
#ifdef TDMA_DEBUG_BEACON_RX
			printk("TDMA: Desynchronize this node\n");
#endif
			off_carrier = true;
		} else {
#ifdef TDMA_DEBUG_BEACON_RX
			printk("TDMA: Just ignore it.\n");
#endif
		}
		goto out;
	}
	if ((over == 3) && (TDMA_INITIALIZED(tdma) && ((tdma->node_num != tie_node_num) || (tie_slot_size != tdma->slot_size) || (TDMA_CFG_GACK(tdma) != TDMA_CFG_GACK_P(tie_cfg))))) {
#ifdef TDMA_DEBUG_BEACON_RX
		printk("TDMA: Desynchronize this node\n");
#endif
		off_carrier = true;
		goto out;
	}
	sta = tdma_upd_sta_rcu(sdata, mgmt->sa, &elems, tie_state, tie_tx_ratio, tie_rx_ratio, tie_node_num, tie_cfg, tie_rate, tie_slot_size, tie_node_bitmap, tie_node_id, rx_status->signal);
	if (sta != NULL) {
#ifdef TDMA_DEBUG_BEACON_RX_ALL
		printk("TDMA: CBR (%d) time (%lu). IE_Len - %d, num_bytes - %d. Interval is - (%lu). (S %d/R %d)\n", sta->n.aid, sta->n.last_seen, (int)ie_len, num_bytes, tdma->tx_round_duration, state, tie_state);
#endif
		if (state != TDMA_STATUS_UNKNOW) {
			u8 *pos;

			while (num_bytes > 0) {
				pos = start_pos;
				start_pos++;
				num_bytes--;
				num_seqs = *start_pos;
				start_pos++;
				num_bytes--;
#ifdef CPTCFG_MAC80211_TDMA_MESH
				if (*pos == WLAN_EID_TDMA_MESH_INFO) {
					while (num_seqs > 0) {
						if (!ether_addr_equal(start_pos, sdata->vif.addr)) {
							u16 reachval = ntohs(get_unaligned((u16 *)(start_pos + ETH_ALEN)));
							tdma_originator_install_record(tdma, start_pos, mgmt->sa, reachval, rx_status->signal);
						}
						start_pos += (ETH_ALEN + sizeof(u16));
						num_seqs -= (ETH_ALEN + sizeof(u16));
						num_bytes -= (ETH_ALEN + sizeof(u16));
					}
				} else {
#endif
					num_seqs--;
					sta->n.miss_own_beacon = !!(*start_pos);
					start_pos++;
					num_bytes--;
					if ((WLAN_EID_TDMA_PEER_INFO + tdma->node_id) == *pos) {
						tdma->last_verified = sta->n.last_seen;
						if (state == TDMA_STATUS_ASSOCIATING) {
#ifdef TDMA_DEBUG_BEACON_RX
							printk("TDMA: Neighbour with node_id (%d) is voted for this node.\n", (int)tie_node_id);
#endif
							sta->n.voted = true;
						} else {
							rcu_read_unlock();
							tdma_frame_acked(sta, rx_status->signal, (int)num_seqs, start_pos);
							rcu_read_lock();
						}
#ifdef CPTCFG_MAC80211_TDMA_MESH
						if (over > 1)
#endif
							break;
					}
					start_pos += num_seqs;
					num_bytes -= num_seqs;
#ifdef CPTCFG_MAC80211_TDMA_MESH
				}
#endif
			}
		}
		rcu_read_unlock();
	}
out:
	if (tie_state == TDMA_STATUS_ASSOCIATED) {
		unsigned long delta;

		if ((over == 3) && (tdma->node_num > 2))
			tie_node_id = tie_bcn_for;
		if ((delta = tdma_calc_ideal_interval(tdma, tie_node_id)) > 0) {
			tdma_plan_next_tx(tdma, delta);
		}
	}
	if (off_carrier) {
#ifdef TDMA_DEBUG_BEACON_RX
		printk("TDMA: Disable frame transmission after beacon precessing\n");
#endif
		tdma_reset_state(tdma);
	}
}

void ieee80211_tdma_rx_queued_mgmt(struct ieee80211_sub_if_data *sdata, struct sk_buff *skb)
{
	struct ieee80211_rx_status *rx_status;
	struct ieee80211_mgmt *mgmt;
	u16 fc;

	rx_status = IEEE80211_SKB_RXCB(skb);
	mgmt = (struct ieee80211_mgmt *)skb->data;
	fc = le16_to_cpu(mgmt->frame_control);

	switch (fc & IEEE80211_FCTL_STYPE) {
	case IEEE80211_STYPE_BEACON:
		tdma_rx_mgmt_beacon(sdata, mgmt, skb->len, rx_status);
		break;
	case IEEE80211_STYPE_ACTION:
		tdma_rx_mgmt_action(sdata, mgmt, skb->len, rx_status);
		break;
	default:
		break;
	}
}
