/*
 * Copyright (C) 2006-2018, Marvell International Ltd.
 *
 * This software file (the "File") is distributed by Marvell International
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991
 * (the "License").  You may use, redistribute and/or modify this File in
 * accordance with the terms and conditions of the License, a copy of which
 * is available by writing to the Free Software Foundation, Inc.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 * this warranty disclaimer.
 */

/* Description:  This file implements receive related functions for new data
 * path.
 */

#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include "sysadpt.h"
#include "core.h"
#include "utils.h"
#include "hif/pcie/dev.h"
#include "hif/pcie/8964/rx_ndp.h"

#define MAX_NUM_RX_RING_BYTES   (MAX_NUM_RX_DESC * \
				sizeof(struct pcie_rx_desc_ndp))
#define MAX_NUM_RX_RING_DONE_BYTES (MAX_NUM_RX_DESC * \
				sizeof(struct rx_ring_done))

static int pcie_rx_ring_alloc_ndp(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_desc_data_ndp *desc = &pcie_priv->desc_data_ndp;

	desc->prx_ring =
		dma_alloc_coherent(priv->dev,
				   MAX_NUM_RX_RING_BYTES,
				   &desc->pphys_rx_ring,
				   GFP_KERNEL);
	if (!desc->prx_ring)
		goto err_no_mem;
	memset(desc->prx_ring, 0x00, MAX_NUM_RX_RING_BYTES);

	desc->prx_ring_done =
		dma_alloc_coherent(priv->dev,
				   MAX_NUM_RX_RING_DONE_BYTES,
				   &desc->pphys_rx_ring_done,
				   GFP_KERNEL);
	if (!desc->prx_ring_done)
		goto err_no_mem;
	memset(desc->prx_ring_done, 0x00, MAX_NUM_RX_RING_DONE_BYTES);

	pcie_priv->rx_page_pool =
		pcie_rx_create_page_pool(pcie_priv, MAX_NUM_RX_DESC,
					 RX_HEADROOM_NDP, MAX_AGGR_SIZE);
	if (IS_ERR(pcie_priv->rx_page_pool)) {
		int rc = PTR_ERR(pcie_priv->rx_page_pool);

		pcie_priv->rx_page_pool = NULL;
		wiphy_err(priv->hw->wiphy, "cannot create page pool\n");
		return rc;
	}
	return 0;

err_no_mem:

	wiphy_err(priv->hw->wiphy, "cannot alloc mem\n");
	return -ENOMEM;
}

static int pcie_rx_ring_init_ndp(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_desc_data_ndp *desc = &pcie_priv->desc_data_ndp;
	struct page_pool *pool = pcie_priv->rx_page_pool;
	int i;
	struct page *page;

	if (desc->prx_ring) {
		desc->rx_buf_size = MAX_AGGR_SIZE;

		for (i = 0; i < MAX_NUM_RX_DESC; i++) {
			page = page_pool_dev_alloc_pages(pool);
			if (!page) {
				wiphy_err(priv->hw->wiphy,
					  "rxdesc %i: no page available\n",
					  i);
				return -ENOMEM;
			}

			desc->rx_vbuflist[i] = page;
			desc->prx_ring[i].user = cpu_to_le32(i);
			desc->prx_ring[i].data =
				cpu_to_le32(pcie_rx_page_dma(pool, page));
		}

		writel(1023, pcie_priv->iobase1 + MACREG_REG_RXDESCHEAD);
		return 0;
	}

	wiphy_err(priv->hw->wiphy, "no valid RX mem\n");
	return -ENOMEM;
}

static void pcie_rx_ring_cleanup_ndp(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_desc_data_ndp *desc = &pcie_priv->desc_data_ndp;
	int i;

	if (desc->prx_ring) {
		for (i = 0; i < MAX_NUM_RX_DESC; i++) {
			if (desc->rx_vbuflist[i]) {
				page_pool_put_full_page(pcie_priv->rx_page_pool,
							desc->rx_vbuflist[i],
							false);
				desc->rx_vbuflist[i] = NULL;
			}
		}
	}
}

static void pcie_rx_ring_free_ndp(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_desc_data_ndp *desc = &pcie_priv->desc_data_ndp;

	if (desc->prx_ring) {
		pcie_rx_ring_cleanup_ndp(priv);
		dma_free_coherent(priv->dev,
				  MAX_NUM_RX_RING_BYTES,
				  desc->prx_ring,
				  desc->pphys_rx_ring);
		desc->prx_ring = NULL;
	}

	if (desc->prx_ring_done) {
		dma_free_coherent(priv->dev,
				  MAX_NUM_RX_RING_DONE_BYTES,
				  desc->prx_ring_done,
				  desc->pphys_rx_ring_done);
		desc->prx_ring_done = NULL;
	}

	page_pool_destroy(pcie_priv->rx_page_pool);
	pcie_priv->rx_page_pool = NULL;
}

static inline void pcie_rx_update_ndp_cnts(struct mwl_priv *priv, u32 ctrl)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;

	switch (ctrl) {
	case RXRING_CASE_DROP:
		pcie_priv->rx_cnts.drop_cnt++;
		break;
	case RXRING_CASE_FAST_BAD_AMSDU:
		pcie_priv->rx_cnts.fast_bad_amsdu_cnt++;
		break;
	case RXRING_CASE_FAST_DATA:
		pcie_priv->rx_cnts.fast_data_cnt++;
		break;
	case RXRING_CASE_SLOW_BAD_MIC:
		pcie_priv->rx_cnts.slow_bad_mic_cnt++;
		break;
	case RXRING_CASE_SLOW_BAD_PN:
		pcie_priv->rx_cnts.slow_bad_pn_cnt++;
		break;
	case RXRING_CASE_SLOW_BAD_STA:
		pcie_priv->rx_cnts.slow_bad_sta_cnt++;
		break;
	case RXRING_CASE_SLOW_MCAST:
		pcie_priv->rx_cnts.slow_mcast_cnt++;
		break;
	case RXRING_CASE_SLOW_MGMT:
		pcie_priv->rx_cnts.slow_mgmt_cnt++;
		break;
	case RXRING_CASE_SLOW_NOQUEUE:
		pcie_priv->rx_cnts.slow_noqueue_cnt++;
		break;
	case RXRING_CASE_SLOW_NORUN:
		pcie_priv->rx_cnts.slow_norun_cnt++;
		break;
	case RXRING_CASE_SLOW_PROMISC:
		pcie_priv->rx_cnts.slow_promisc_cnt++;
		break;
	}
}

static void pcie_rx_status_ndp(struct mwl_priv *priv,
			       struct mwl_sta *sta_info,
			       struct ieee80211_rx_status *status)
{
	memset(status, 0, sizeof(*status));
	pcie_rx_prepare_status(priv,
			       sta_info->rx_format,
			       sta_info->rx_nss,
			       sta_info->rx_bw,
			       sta_info->rx_gi,
			       sta_info->rx_rate_mcs,
			       status);
	status->signal = -sta_info->rx_signal;
	status->band = priv->hw->conf.chandef.chan->band;
	status->freq = ieee80211_channel_to_frequency(
		priv->hw->conf.chandef.chan->hw_value, status->band);
}

static inline void pcie_rx_process_fast_data(struct mwl_priv *priv,
					     struct sk_buff *skb,
					     u16 stnid)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct ieee80211_sta *sta;
	struct mwl_sta *sta_info;
	struct mwl_vif *mwl_vif;
	struct ieee80211_hdr hdr;
	u16 hdrlen, ethertype;
	__le16 fc;
	struct ieee80211_rx_status *status;

	if (stnid == RXRING_CTRL_STA_FROMDS)
		stnid = 0;

	if (stnid > SYSADPT_MAX_STA_SC4)
		goto drop_packet;

	sta = pcie_priv->sta_link[stnid];
	if (!sta)
		goto drop_packet;

	sta_info = mwl_dev_get_sta(sta);
	mwl_vif = sta_info->mwl_vif;
	if (!mwl_vif)
		goto drop_packet;

	ethertype = (skb->data[20] << 8) | skb->data[21];
	fc = cpu_to_le16(IEEE80211_FTYPE_DATA | IEEE80211_STYPE_DATA);

	memset(&hdr, 0, sizeof(hdr));
	switch (mwl_vif->type) {
	case NL80211_IFTYPE_AP:
		if (sta_info->wds) {
			fc |= (cpu_to_le16(IEEE80211_FCTL_TODS) |
				cpu_to_le16(IEEE80211_FCTL_FROMDS));
			/* RA TA DA SA */
			ether_addr_copy(hdr.addr1, mwl_vif->bssid);
			ether_addr_copy(hdr.addr2, sta->addr);
			ether_addr_copy(hdr.addr3, skb->data);
			ether_addr_copy(hdr.addr4, skb->data + ETH_ALEN);
			hdrlen = 30;
		} else {
			fc |= cpu_to_le16(IEEE80211_FCTL_TODS);
			/* BSSID SA DA */
			ether_addr_copy(hdr.addr1, mwl_vif->bssid);
			ether_addr_copy(hdr.addr2, skb->data + ETH_ALEN);
			ether_addr_copy(hdr.addr3, skb->data);
			hdrlen = 24;
		}
		break;
	case NL80211_IFTYPE_STATION:
		if (sta_info->wds) {
			fc |= (cpu_to_le16(IEEE80211_FCTL_TODS) |
				cpu_to_le16(IEEE80211_FCTL_FROMDS));
			/* RA TA DA SA */
			ether_addr_copy(hdr.addr1, mwl_vif->sta_mac);
			ether_addr_copy(hdr.addr2, mwl_vif->bssid);
			ether_addr_copy(hdr.addr3, skb->data);
			ether_addr_copy(hdr.addr4, skb->data + ETH_ALEN);
			hdrlen = 30;
		} else {
			fc |= cpu_to_le16(IEEE80211_FCTL_FROMDS);
			/* DA BSSID SA */
			ether_addr_copy(hdr.addr1, skb->data);
			ether_addr_copy(hdr.addr2, mwl_vif->bssid);
			ether_addr_copy(hdr.addr3, skb->data + ETH_ALEN);
			hdrlen = 24;
		}
		break;
	default:
		goto drop_packet;
	}

	if (sta->wme) {
		fc |= cpu_to_le16(IEEE80211_STYPE_QOS_DATA);
		hdrlen += 2;
	}

	status = IEEE80211_SKB_RXCB(skb);
	pcie_rx_status_ndp(priv, sta_info, status);
	if (mwl_vif->is_hw_crypto_enabled) {
		fc |= cpu_to_le16(IEEE80211_FCTL_PROTECTED);
		status->flag |= RX_FLAG_IV_STRIPPED |
				RX_FLAG_DECRYPTED |
				RX_FLAG_MMIC_STRIPPED;
	}

	hdr.frame_control = fc;
	hdr.duration_id = 0;

	skb_pull(skb, ETH_HLEN);

	if (ieee80211_is_data_qos(fc)) {
		__le16 *qos_control;

		qos_control = (__le16 *)skb_push(skb, IEEE80211_QOS_CTL_LEN);
		memcpy(skb_push(skb, hdrlen - IEEE80211_QOS_CTL_LEN), &hdr, hdrlen - IEEE80211_QOS_CTL_LEN);
		if (ethertype == ETH_P_PAE)
			*qos_control = cpu_to_le16(
				IEEE80211_QOS_CTL_ACK_POLICY_NOACK | 7);
		else
			*qos_control = cpu_to_le16(
				IEEE80211_QOS_CTL_ACK_POLICY_NOACK);
	} else
		memcpy(skb_push(skb, hdrlen), &hdr, hdrlen);

	status->flag |= RX_FLAG_DUP_VALIDATED;
	ieee80211_rx_napi(priv->hw, NULL, skb, &pcie_priv->napi);

	return;
drop_packet:

	dev_kfree_skb_any(skb);
}

static inline void pcie_rx_process_slow_data(struct mwl_priv *priv,
					     struct sk_buff *skb,
					     bool bad_mic, u8 signal)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct ieee80211_rx_status *status;
	struct ieee80211_hdr *wh;
	struct mwl_vif *mwl_vif = NULL;
	struct sk_buff* monitor_skb;

	pcie_rx_remove_dma_header(skb, 0);
	status = IEEE80211_SKB_RXCB(skb);
	memset(status, 0, sizeof(*status));
	status->signal = -signal;
	status->band = priv->hw->conf.chandef.chan->band;
	status->freq = ieee80211_channel_to_frequency(
		priv->hw->conf.chandef.chan->hw_value, status->band);

	if (bad_mic)
		status->flag |= RX_FLAG_MMIC_ERROR;
	else {
		wh = (struct ieee80211_hdr *)skb->data;

		if (ieee80211_has_protected(wh->frame_control)) {
			if (ieee80211_has_tods(wh->frame_control)) {
				mwl_vif = utils_find_vif_bss(priv, wh->addr1);
				if (!mwl_vif &&
				    ieee80211_has_a4(wh->frame_control))
					mwl_vif =
						utils_find_vif_bss(priv,
								   wh->addr2);
			} else {
				mwl_vif = utils_find_vif_bss(priv, wh->addr2);
			}

			if ((mwl_vif && mwl_vif->is_hw_crypto_enabled) ||
			    is_multicast_ether_addr(wh->addr1) ||
			    (ieee80211_is_mgmt(wh->frame_control) &&
			    !is_multicast_ether_addr(wh->addr1))) {
				if (!ieee80211_is_mgmt(wh->frame_control))
					status->flag |= RX_FLAG_IV_STRIPPED |
							RX_FLAG_DECRYPTED |
							RX_FLAG_MMIC_STRIPPED;
				else if (!ieee80211_is_auth(wh->frame_control))
					/* The firmware decrypts protected
					 * management frames in place, but
					 * leaves the CCMP header and MIC for
					 * mac80211 to remove.
					 */
					status->flag |= RX_FLAG_DECRYPTED;
			}
		}

		if (ieee80211_has_a4(wh->frame_control) && !priv->wds_check) {
			ether_addr_copy(priv->wds_check_sta, wh->addr2);
			ieee80211_queue_work(priv->hw, &priv->wds_check_handle);
			priv->wds_check = true;
		}

		if (status->flag & RX_FLAG_DECRYPTED) {
			monitor_skb = skb_copy(skb, GFP_ATOMIC);
			if (monitor_skb) {
				IEEE80211_SKB_RXCB(monitor_skb)->flag |= RX_FLAG_ONLY_MONITOR;
				((struct ieee80211_hdr *)monitor_skb->data)->frame_control &= ~__cpu_to_le16(IEEE80211_FCTL_PROTECTED);
				ieee80211_rx_napi(priv->hw, NULL, monitor_skb,
						  &pcie_priv->napi);
			}
			status->flag |= RX_FLAG_SKIP_MONITOR;
		}
	}

	status->flag |= RX_FLAG_DUP_VALIDATED;
	ieee80211_rx_napi(priv->hw, NULL, skb, &pcie_priv->napi);
}

int pcie_rx_init_ndp(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	int rc;

	rc = pcie_rx_ring_alloc_ndp(priv);
	if (rc) {
		pcie_rx_ring_free_ndp(priv);
		wiphy_err(hw->wiphy, "allocating RX ring failed\n");
		return rc;
	}

	rc = pcie_rx_ring_init_ndp(priv);
	if (rc) {
		pcie_rx_ring_free_ndp(priv);
		wiphy_err(hw->wiphy,
			  "initializing RX ring failed\n");
		return rc;
	}

	return 0;
}

void pcie_rx_deinit_ndp(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	pcie_rx_ring_cleanup_ndp(priv);
	pcie_rx_ring_free_ndp(priv);
}

int pcie_rx_poll_ndp(struct napi_struct *napi, int budget)
{
	struct pcie_priv *pcie_priv = container_of(napi, struct pcie_priv, napi);
	struct mwl_priv *priv = pcie_priv->mwl_priv;
	struct ieee80211_hw *hw = priv->hw;
	struct page_pool *pool = pcie_priv->rx_page_pool;
	struct pcie_desc_data_ndp *desc = &pcie_priv->desc_data_ndp;
	struct rx_ring_done *prx_ring_done;
	struct pcie_rx_desc_ndp *prx_desc;
	u32 rx_done_head;
	u32 rx_done_tail;
	u32 rx_desc_head;
	struct sk_buff *psk_buff;
	struct page *page, *new_page;
	u32 buf_idx;
	u32 rx_cnt;
	u32 ctrl, ctrl_case;
	bool bad_mic;
	u16 stnid;
	u16 pktlen;
	struct rx_info *rx_info;
	struct pcie_dma_data *dma_data;
	u8 signal;

	if (pcie_priv->data_tasks_killed) {
		napi_complete(napi);
		return 0;
	}

	rx_done_head = readl(pcie_priv->iobase1 + MACREG_REG_RXDONEHEAD);
	rx_done_tail = readl(pcie_priv->iobase1 + MACREG_REG_RXDONETAIL);
	rx_desc_head = readl(pcie_priv->iobase1 + MACREG_REG_RXDESCHEAD);
	rx_cnt = 0;

	while ((rx_done_tail != rx_done_head) &&
	       (rx_cnt < budget)) {
recheck:
		prx_ring_done = &desc->prx_ring_done[rx_done_tail];
		wmb(); /*Data Memory Barrier*/
		if (le32_to_cpu(prx_ring_done->user) == 0xdeadbeef) {
			pcie_priv->recheck_rxringdone++;
			udelay(1);
			goto recheck;
		}
		buf_idx = le32_to_cpu(prx_ring_done->user) & 0x3fff;
		prx_ring_done->user = cpu_to_le32(0xdeadbeef);
		rx_done_tail++;
		prx_desc = &desc->prx_ring[buf_idx];
		page = desc->rx_vbuflist[buf_idx];

		/* Replace the buffer before handing this one up. If there
		 * is no replacement, drop the frame and reuse its buffer.
		 */
		new_page = page_pool_dev_alloc_pages(pool);
		if (unlikely(!new_page))
			goto out;

		desc->rx_vbuflist[buf_idx] = new_page;
		prx_desc->data = cpu_to_le32(pcie_rx_page_dma(pool, new_page));

		page_pool_dma_sync_for_cpu(pool, page, 0, desc->rx_buf_size);

		psk_buff = pcie_rx_build_skb(pool, page);
		if (unlikely(!psk_buff)) {
			page_pool_recycle_direct(pool, page);
			goto out;
		}

		bad_mic = false;
		ctrl = le32_to_cpu(prx_ring_done->ctrl);
		ctrl_case = ctrl & RXRING_CTRL_CASE_MASK;
		stnid = (ctrl >> RXRING_CTRL_STA_SHIFT) & RXRING_CTRL_STA_MASK;
		pcie_rx_update_ndp_cnts(priv, ctrl_case);

		switch (ctrl_case) {
		case RXRING_CASE_FAST_DATA:
			if (stnid == RXRING_CTRL_STA_UNKNOWN) {
				dev_kfree_skb_any(psk_buff);
				break;
			}
			pktlen = psk_buff->data[12] << 8 | psk_buff->data[13];
			pktlen += ETH_HLEN;

			if (pktlen <= desc->rx_buf_size) {
				skb_put(psk_buff, pktlen);
				pcie_rx_process_fast_data(priv, psk_buff,
							  stnid);
			} else {
				wiphy_err(hw->wiphy,
					  "fast: space %d(%d) is not enough\n",
					  desc->rx_buf_size, pktlen);
				dev_kfree_skb_any(psk_buff);
			}
			break;
		case RXRING_CASE_FAST_BAD_AMSDU:
		case RXRING_CASE_SLOW_BAD_STA:
		case RXRING_CASE_SLOW_DEL_DONE:
		case RXRING_CASE_DROP:
		case RXRING_CASE_SLOW_BAD_PN:
			if (ctrl_case == RXRING_CASE_SLOW_DEL_DONE) {
				wiphy_dbg(hw->wiphy,
					    "staid %d deleted\n",
					    stnid);
				utils_free_stnid(priv, stnid);
			}
			dev_kfree_skb_any(psk_buff);
			break;
		case RXRING_CASE_SLOW_BAD_MIC:
		case RXRING_CASE_SLOW_NOQUEUE:
		case RXRING_CASE_SLOW_NORUN:
		case RXRING_CASE_SLOW_MGMT:
		case RXRING_CASE_SLOW_MCAST:
		case RXRING_CASE_SLOW_PROMISC:
			if (RXRING_CASE_SLOW_BAD_MIC == ctrl_case)
				bad_mic = true;

			rx_info = (struct rx_info *)psk_buff->data;
			dma_data = (struct pcie_dma_data *)&rx_info->hdr[0];
			pktlen = le16_to_cpu(dma_data->fwlen);
			pktlen += sizeof(*rx_info);
			pktlen += sizeof(struct pcie_dma_data);
			if (bad_mic) {
				memset(&dma_data->data, 0, 4);
				pktlen += 4;
			}
			if (pktlen <= desc->rx_buf_size) {
				skb_put(psk_buff, pktlen);
				skb_pull(psk_buff, sizeof(*rx_info));
				signal = ((le32_to_cpu(rx_info->rssi_x) >>
					RXINFO_RSSI_X_SHIFT) &
					RXINFO_RSSI_X_MASK);
				pcie_rx_process_slow_data(priv, psk_buff,
							  bad_mic, signal);
			} else {
				wiphy_err(hw->wiphy,
					  "slow: space %d(%d) is not enough\n",
					  desc->rx_buf_size, pktlen);
				dev_kfree_skb_any(psk_buff);
			}
			break;
		default:
			wiphy_err(hw->wiphy, "unknown control case: %d\n",
				  ctrl_case);
			dev_kfree_skb_any(psk_buff);
			break;
		}
out:
		if (rx_done_tail >= MAX_RX_RING_DONE_SIZE)
			rx_done_tail = 0;

		rx_done_head =
			readl(pcie_priv->iobase1 + MACREG_REG_RXDONEHEAD);
		rx_cnt++;
	}

	rx_desc_head += rx_cnt;
	if (rx_desc_head >= MAX_RX_RING_SEND_SIZE)
		rx_desc_head = rx_desc_head - MAX_RX_RING_SEND_SIZE;
	writel(rx_done_tail, pcie_priv->iobase1 + MACREG_REG_RXDONETAIL);
	writel(rx_desc_head, pcie_priv->iobase1 + MACREG_REG_RXDESCHEAD);

	if (rx_cnt < budget && napi_complete_done(napi, rx_cnt))
		pcie_mask_int(pcie_priv, MACREG_A2HRIC_RX_DONE_HEAD_RDY, true);

	return rx_cnt;
}
