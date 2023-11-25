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

/* Description:  This file implements receive related functions. */

#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include "sysadpt.h"
#include "core.h"
#include "utils.h"
#include "hif/pcie/dev.h"
#include "hif/pcie/8997/rx.h"

#define MAX_NUM_RX_RING_BYTES  (PCIE_MAX_NUM_RX_DESC * \
				sizeof(struct pcie_rx_desc))

#define MAX_NUM_RX_HNDL_BYTES  (PCIE_MAX_NUM_RX_DESC * \
				sizeof(struct pcie_rx_hndl))

#define DECRYPT_ERR_MASK        0x80
#define GENERAL_DECRYPT_ERR     0xFF
#define TKIP_DECRYPT_MIC_ERR    0x02
#define WEP_DECRYPT_ICV_ERR     0x04
#define TKIP_DECRYPT_ICV_ERR    0x08

#define W836X_RSSI_OFFSET       8

static int pcie_rx_ring_alloc(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_desc_data *desc;

	desc = &pcie_priv->desc_data[0];

	desc->prx_ring = (struct pcie_rx_desc *)
		dma_alloc_coherent(priv->dev,
				   MAX_NUM_RX_RING_BYTES,
				   &desc->pphys_rx_ring,
				   GFP_KERNEL);

	if (!desc->prx_ring) {
		wiphy_err(priv->hw->wiphy, "cannot alloc mem\n");
		return -ENOMEM;
	}

	memset(desc->prx_ring, 0x00, MAX_NUM_RX_RING_BYTES);

	desc->rx_hndl = kzalloc(MAX_NUM_RX_HNDL_BYTES, GFP_KERNEL);

	if (!desc->rx_hndl) {
		dma_free_coherent(priv->dev,
				  MAX_NUM_RX_RING_BYTES,
				  desc->prx_ring,
				  desc->pphys_rx_ring);
		return -ENOMEM;
	}

	return 0;
}

static int pcie_rx_ring_init(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_desc_data *desc;
	int i;
	struct pcie_rx_hndl *rx_hndl;
	dma_addr_t dma;
	u32 val;

	desc = &pcie_priv->desc_data[0];

	if (desc->prx_ring) {
		desc->rx_buf_size = SYSADPT_MAX_AGGR_SIZE;

		for (i = 0; i < PCIE_MAX_NUM_RX_DESC; i++) {
			rx_hndl = &desc->rx_hndl[i];
			rx_hndl->psk_buff =
				dev_alloc_skb(desc->rx_buf_size);

			if (!rx_hndl->psk_buff) {
				wiphy_err(priv->hw->wiphy,
					  "rxdesc %i: no skbuff available\n",
					  i);
				return -ENOMEM;
			}

			skb_reserve(rx_hndl->psk_buff,
				    PCIE_MIN_BYTES_HEADROOM);
			desc->prx_ring[i].rx_control =
				EAGLE_RXD_CTRL_DRIVER_OWN;
			desc->prx_ring[i].status = EAGLE_RXD_STATUS_OK;
			desc->prx_ring[i].qos_ctrl = 0x0000;
			desc->prx_ring[i].channel = 0x00;
			desc->prx_ring[i].rssi = 0x00;
			desc->prx_ring[i].pkt_len =
				cpu_to_le16(SYSADPT_MAX_AGGR_SIZE);
			dma = dma_map_single(&(pcie_priv->pdev)->dev,
					     rx_hndl->psk_buff->data,
					     desc->rx_buf_size,
					     DMA_FROM_DEVICE);
			if (dma_mapping_error(&(pcie_priv->pdev)->dev, dma)) {
				wiphy_err(priv->hw->wiphy,
					  "failed to map pci memory!\n");
				return -ENOMEM;
			}
			desc->prx_ring[i].pphys_buff_data = cpu_to_le32(dma);
			val = (u32)desc->pphys_rx_ring +
			      ((i + 1) * sizeof(struct pcie_rx_desc));
			desc->prx_ring[i].pphys_next = cpu_to_le32(val);
			rx_hndl->pdesc = &desc->prx_ring[i];
			if (i < (PCIE_MAX_NUM_RX_DESC - 1))
				rx_hndl->pnext = &desc->rx_hndl[i + 1];
		}
		desc->prx_ring[PCIE_MAX_NUM_RX_DESC - 1].pphys_next =
			cpu_to_le32((u32)desc->pphys_rx_ring);
		desc->rx_hndl[PCIE_MAX_NUM_RX_DESC - 1].pnext =
			&desc->rx_hndl[0];
		desc->pnext_rx_hndl = &desc->rx_hndl[0];

		return 0;
	}

	wiphy_err(priv->hw->wiphy, "no valid RX mem\n");

	return -ENOMEM;
}

static void pcie_rx_ring_cleanup(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_desc_data *desc;
	int i;
	struct pcie_rx_hndl *rx_hndl;

	desc = &pcie_priv->desc_data[0];

	if (desc->prx_ring) {
		for (i = 0; i < PCIE_MAX_NUM_RX_DESC; i++) {
			rx_hndl = &desc->rx_hndl[i];
			if (!rx_hndl->psk_buff)
				continue;

			dma_unmap_single(&(pcie_priv->pdev)->dev,
					 le32_to_cpu
					 (rx_hndl->pdesc->pphys_buff_data),
					 desc->rx_buf_size,
					 DMA_FROM_DEVICE);

			dev_kfree_skb_any(rx_hndl->psk_buff);

			wiphy_dbg(priv->hw->wiphy,
				    "unmapped+free'd %i 0x%p 0x%x %i\n",
				    i, rx_hndl->psk_buff->data,
				    le32_to_cpu(
				    rx_hndl->pdesc->pphys_buff_data),
				    desc->rx_buf_size);

			rx_hndl->psk_buff = NULL;
		}
	}
}

static void pcie_rx_ring_free(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_desc_data *desc;

	desc = &pcie_priv->desc_data[0];

	if (desc->prx_ring) {
		pcie_rx_ring_cleanup(priv);

		dma_free_coherent(priv->dev,
				  MAX_NUM_RX_RING_BYTES,
				  desc->prx_ring,
				  desc->pphys_rx_ring);

		desc->prx_ring = NULL;
	}

	kfree(desc->rx_hndl);

	desc->pnext_rx_hndl = NULL;
}

static inline void pcie_rx_status(struct mwl_priv *priv,
				  struct pcie_rx_desc *pdesc,
				  struct ieee80211_rx_status *status)
{
	u16 rx_rate;

	memset(status, 0, sizeof(*status));

	status->signal = (s8)pdesc->rssi;

	rx_rate = le16_to_cpu(pdesc->rate);
	pcie_rx_prepare_status(priv,
			       rx_rate & MWL_RX_RATE_FORMAT_MASK,
			       (rx_rate & MWL_RX_RATE_NSS_MASK) >>
			       MWL_RX_RATE_NSS_SHIFT,
			       (rx_rate & MWL_RX_RATE_BW_MASK) >>
			       MWL_RX_RATE_BW_SHIFT,
			       (rx_rate & MWL_RX_RATE_GI_MASK) >>
			       MWL_RX_RATE_GI_SHIFT,
			       (rx_rate & MWL_RX_RATE_RT_MASK) >>
			       MWL_RX_RATE_RT_SHIFT,
			       status);

	status->freq = ieee80211_channel_to_frequency(pdesc->channel,
						      status->band);

	/* check if status has a specific error bit (bit 7) set or indicates
	 * a general decrypt error
	 */
	if ((pdesc->status == GENERAL_DECRYPT_ERR) ||
	    (pdesc->status & DECRYPT_ERR_MASK)) {
		/* check if status is not equal to 0xFF
		 * the 0xFF check is for backward compatibility
		 */
		if (pdesc->status != GENERAL_DECRYPT_ERR) {
			if (((pdesc->status & (~DECRYPT_ERR_MASK)) &
			    TKIP_DECRYPT_MIC_ERR) && !((pdesc->status &
			    (WEP_DECRYPT_ICV_ERR | TKIP_DECRYPT_ICV_ERR)))) {
				status->flag |= RX_FLAG_MMIC_ERROR;
			}
		}
	}
}

static inline bool pcie_rx_process_mesh_amsdu(struct mwl_priv *priv,
					     struct sk_buff *skb,
					     struct ieee80211_rx_status *status)
{
	struct ieee80211_hdr *wh;
	struct mwl_sta *sta_info;
	struct ieee80211_sta *sta;
	u8 *qc;
	int wh_len;
	int len;
	u8 pad;
	u8 *data;
	u16 frame_len;
	struct sk_buff *newskb;

	wh = (struct ieee80211_hdr *)skb->data;

	spin_lock_bh(&priv->sta_lock);
	list_for_each_entry(sta_info, &priv->sta_list, list) {
		sta = container_of((void *)sta_info, struct ieee80211_sta,
				   drv_priv[0]);
		if (ether_addr_equal(sta->addr, wh->addr2)) {
			if (!sta_info->is_mesh_node) {
				spin_unlock_bh(&priv->sta_lock);
				return false;
			}
		}
	}
	spin_unlock_bh(&priv->sta_lock);

	qc = ieee80211_get_qos_ctl(wh);
	*qc &= ~IEEE80211_QOS_CTL_A_MSDU_PRESENT;

	wh_len = ieee80211_hdrlen(wh->frame_control);
	len = wh_len;
	data = skb->data;

	while (len < skb->len) {
		frame_len = *(u8 *)(data + len + ETH_HLEN - 1) |
			(*(u8 *)(data + len + ETH_HLEN - 2) << 8);

		if ((len + ETH_HLEN + frame_len) > skb->len)
			break;

		newskb = dev_alloc_skb(wh_len + frame_len);
		if (!newskb)
			break;

		ether_addr_copy(wh->addr3, data + len);
		ether_addr_copy(wh->addr4, data + len + ETH_ALEN);
		memcpy(newskb->data, wh, wh_len);
		memcpy(newskb->data + wh_len, data + len + ETH_HLEN, frame_len);
		skb_put(newskb, wh_len + frame_len);

		pad = ((ETH_HLEN + frame_len) % 4) ?
			(4 - (ETH_HLEN + frame_len) % 4) : 0;
		len += (ETH_HLEN + frame_len + pad);
		if (len < skb->len)
			status->flag |= RX_FLAG_AMSDU_MORE;
		else
			status->flag &= ~RX_FLAG_AMSDU_MORE;
		memcpy(IEEE80211_SKB_RXCB(newskb), status, sizeof(*status));
		ieee80211_rx(priv->hw, newskb);
	}

	dev_kfree_skb_any(skb);

	return true;
}

static inline int pcie_rx_refill(struct mwl_priv *priv,
				 struct pcie_rx_hndl *rx_hndl)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_desc_data *desc;
	dma_addr_t dma;

	desc = &pcie_priv->desc_data[0];

	rx_hndl->psk_buff = dev_alloc_skb(desc->rx_buf_size);

	if (!rx_hndl->psk_buff)
		return -ENOMEM;

	skb_reserve(rx_hndl->psk_buff, PCIE_MIN_BYTES_HEADROOM);

	rx_hndl->pdesc->status = EAGLE_RXD_STATUS_OK;
	rx_hndl->pdesc->qos_ctrl = 0x0000;
	rx_hndl->pdesc->channel = 0x00;
	rx_hndl->pdesc->rssi = 0x00;
	rx_hndl->pdesc->pkt_len = cpu_to_le16(desc->rx_buf_size);

	dma = dma_map_single(&(pcie_priv->pdev)->dev,
			     rx_hndl->psk_buff->data,
			     desc->rx_buf_size,
			     DMA_FROM_DEVICE);
	if (dma_mapping_error(&(pcie_priv->pdev)->dev, dma)) {
		dev_kfree_skb_any(rx_hndl->psk_buff);
		wiphy_err(priv->hw->wiphy,
			  "failed to map pci memory!\n");
		return -ENOMEM;
	}

	rx_hndl->pdesc->pphys_buff_data = cpu_to_le32(dma);

	return 0;
}

int pcie_8997_rx_init(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	int rc;

	rc = pcie_rx_ring_alloc(priv);
	if (rc) {
		wiphy_err(hw->wiphy, "allocating RX ring failed\n");
		return rc;
	}

	rc = pcie_rx_ring_init(priv);
	if (rc) {
		pcie_rx_ring_free(priv);
		wiphy_err(hw->wiphy,
			  "initializing RX ring failed\n");
		return rc;
	}

	return 0;
}

void pcie_8997_rx_deinit(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	pcie_rx_ring_cleanup(priv);
	pcie_rx_ring_free(priv);
}

void pcie_8997_rx_recv(unsigned long data)
{
	struct ieee80211_hw *hw = (struct ieee80211_hw *)data;
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_desc_data *desc;
	struct pcie_rx_hndl *curr_hndl;
	int work_done = 0;
	struct sk_buff *prx_skb = NULL;
	struct sk_buff *monitor_skb;
	int pkt_len;
	struct ieee80211_rx_status *status;
	struct ieee80211_hdr *wh;
	u8 *_data;
	u8 *qc;
	const u8 eapol[] = {0x88, 0x8e};

	desc = &pcie_priv->desc_data[0];
	curr_hndl = desc->pnext_rx_hndl;

	if (!curr_hndl) {
		pcie_mask_int(pcie_priv, MACREG_A2HRIC_BIT_RX_RDY, true);
		pcie_priv->is_rx_schedule = false;
		wiphy_warn(hw->wiphy, "busy or no receiving packets\n");
		return;
	}

	while ((curr_hndl->pdesc->rx_control == EAGLE_RXD_CTRL_DMA_OWN) &&
	       (work_done < pcie_priv->recv_limit)) {
		prx_skb = curr_hndl->psk_buff;
		if (!prx_skb)
			goto out;
		dma_unmap_single(&(pcie_priv->pdev)->dev,
				 le32_to_cpu(curr_hndl->pdesc->pphys_buff_data),
				 desc->rx_buf_size,
				 DMA_FROM_DEVICE);
		pkt_len = le16_to_cpu(curr_hndl->pdesc->pkt_len);

		if (skb_tailroom(prx_skb) < pkt_len) {
			dev_kfree_skb_any(prx_skb);
			goto out;
		}

		if (curr_hndl->pdesc->channel !=
		    hw->conf.chandef.chan->hw_value) {
			dev_kfree_skb_any(prx_skb);
			goto out;
		}

		status = IEEE80211_SKB_RXCB(prx_skb);
		pcie_rx_status(priv, curr_hndl->pdesc, status);

		priv->noise = (s8)curr_hndl->pdesc->noise_floor;
		if (priv->noise > 0)
			priv->noise = -priv->noise;

		wh = &((struct pcie_dma_data *)prx_skb->data)->wh;

		if (utils_is_crypted(wh)) {
			/* When MMIC ERROR is encountered
			* by the firmware, payload is
			* dropped and only 32 bytes of
			* mwlwifi Firmware header is sent
			* to the host.
			*
			* We need to add four bytes of
			* key information.  In it
			* MAC80211 expects keyidx set to
			* 0 for triggering Counter
			* Measure of MMIC failure.
			*/
			if (status->flag & RX_FLAG_MMIC_ERROR) {
				struct pcie_dma_data *dma_data;

				dma_data = (struct pcie_dma_data *)
				     prx_skb->data;
				memset((void *)&dma_data->data, 0, 4);
				pkt_len += 4;
			}

			status->flag |=
				RX_FLAG_DECRYPTED |
				RX_FLAG_MMIC_STRIPPED;
		}

		skb_put(prx_skb, pkt_len);
		pcie_rx_remove_dma_header(prx_skb, curr_hndl->pdesc->qos_ctrl);

		wh = (struct ieee80211_hdr *)prx_skb->data;

		if (ieee80211_is_data_qos(wh->frame_control)) {
			qc = ieee80211_get_qos_ctl(wh);
			_data = prx_skb->data +
				ieee80211_hdrlen(wh->frame_control) + 6;

			if (!memcmp(_data, eapol, sizeof(eapol)))
				*qc |= 7;

			if ((*qc & IEEE80211_QOS_CTL_A_MSDU_PRESENT) && (ieee80211_has_a4(wh->frame_control))) {
				if (pcie_rx_process_mesh_amsdu(priv, prx_skb, status))
					goto out;
			}
		}

		if (status->flag & RX_FLAG_DECRYPTED) {
			monitor_skb = skb_copy(prx_skb, GFP_ATOMIC);
			if (monitor_skb) {
				IEEE80211_SKB_RXCB(monitor_skb)->flag |= RX_FLAG_ONLY_MONITOR;
				((struct ieee80211_hdr *)monitor_skb->data)->frame_control &= ~__cpu_to_le16(IEEE80211_FCTL_PROTECTED);

				ieee80211_rx(hw, monitor_skb);
			}
			status->flag |= RX_FLAG_SKIP_MONITOR;
		}

		ieee80211_rx(hw, prx_skb);
out:
		pcie_rx_refill(priv, curr_hndl);
		curr_hndl->pdesc->rx_control = EAGLE_RXD_CTRL_DRIVER_OWN;
		curr_hndl->pdesc->qos_ctrl = 0;
		curr_hndl = curr_hndl->pnext;
		work_done++;
	}

	desc->pnext_rx_hndl = curr_hndl;
	pcie_mask_int(pcie_priv, MACREG_A2HRIC_BIT_RX_RDY, true);
	pcie_priv->is_rx_schedule = false;
}
