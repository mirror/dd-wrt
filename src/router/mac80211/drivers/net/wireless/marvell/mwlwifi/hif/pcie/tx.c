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

/* Description:  This file implements transmit related functions. */

#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include "sysadpt.h"
#include "core.h"
#include "utils.h"
#include "hif/fwcmd.h"
#include "hif/pcie/dev.h"
#include "hif/pcie/tx.h"

#define MAX_NUM_TX_RING_BYTES  (PCIE_MAX_NUM_TX_DESC * \
				sizeof(struct pcie_tx_desc))

#define MAX_NUM_TX_HNDL_BYTES  (PCIE_MAX_NUM_TX_DESC * \
				sizeof(struct pcie_tx_hndl))

#define TOTAL_HW_QUEUES        (SYSADPT_TX_WMM_QUEUES + \
				PCIE_AMPDU_QUEUES)

#define EAGLE_TXD_XMITCTRL_USE_MC_RATE     0x8     /* Use multicast data rate */

#define MWL_QOS_ACK_POLICY_MASK	           0x0060
#define MWL_QOS_ACK_POLICY_NORMAL          0x0000
#define MWL_QOS_ACK_POLICY_BLOCKACK        0x0060

#define EXT_IV                             0x20
#define INCREASE_IV(iv16, iv32) \
{ \
	(iv16)++; \
	if ((iv16) == 0) \
		(iv32)++; \
}

/* Transmission information to transmit a socket buffer. */
struct pcie_tx_ctrl {
	void *vif;
	void *sta;
	void *k_conf;
	void *amsdu_pkts;
	u8 tx_priority;
	u8 type;
	u16 qos_ctrl;
	u8 xmit_control;
};

struct ccmp_hdr {
	__le16 iv16;
	u8 rsvd;
	u8 key_id;
	__le32 iv32;
} __packed;

static int pcie_tx_ring_alloc(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_desc_data *desc;
	int num;
	u8 *mem;

	desc = &pcie_priv->desc_data[0];

	mem = dma_alloc_coherent(priv->dev,
				 MAX_NUM_TX_RING_BYTES *
				 PCIE_NUM_OF_DESC_DATA,
				 &desc->pphys_tx_ring,
				 GFP_KERNEL);

	if (!mem) {
		wiphy_err(priv->hw->wiphy, "cannot alloc mem\n");
		return -ENOMEM;
	}

	for (num = 0; num < PCIE_NUM_OF_DESC_DATA; num++) {
		desc = &pcie_priv->desc_data[num];

		desc->ptx_ring = (struct pcie_tx_desc *)
			(mem + num * MAX_NUM_TX_RING_BYTES);

		desc->pphys_tx_ring = (dma_addr_t)
			((u32)pcie_priv->desc_data[0].pphys_tx_ring +
			num * MAX_NUM_TX_RING_BYTES);

		memset(desc->ptx_ring, 0x00,
		       MAX_NUM_TX_RING_BYTES);
	}

	mem = kzalloc(MAX_NUM_TX_HNDL_BYTES * PCIE_NUM_OF_DESC_DATA,
		      GFP_KERNEL);

	if (!mem) {
		wiphy_err(priv->hw->wiphy, "cannot alloc mem\n");
		dma_free_coherent(priv->dev,
				  MAX_NUM_TX_RING_BYTES *
				  PCIE_NUM_OF_DESC_DATA,
				  pcie_priv->desc_data[0].ptx_ring,
				  pcie_priv->desc_data[0].pphys_tx_ring);
		return -ENOMEM;
	}

	for (num = 0; num < PCIE_NUM_OF_DESC_DATA; num++) {
		desc = &pcie_priv->desc_data[num];

		desc->tx_hndl = (struct pcie_tx_hndl *)
			(mem + num * MAX_NUM_TX_HNDL_BYTES);
	}

	return 0;
}

static int pcie_txbd_ring_create(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	int num;
	u8 *mem;

	/* driver maintaines the write pointer and firmware maintaines the read
	 * pointer.
	 */
	pcie_priv->txbd_wrptr = 0;
	pcie_priv->txbd_rdptr = 0;

	/* allocate shared memory for the BD ring and divide the same in to
	 * several descriptors
	 */
	pcie_priv->txbd_ring_size =
		sizeof(struct pcie_data_buf) * PCIE_MAX_TXRX_BD;
	wiphy_info(priv->hw->wiphy, "TX ring: allocating %d bytes\n",
		   pcie_priv->txbd_ring_size);

	mem = dma_alloc_coherent(priv->dev,
				 pcie_priv->txbd_ring_size,
				 &pcie_priv->txbd_ring_pbase,
				 GFP_KERNEL);

	if (!mem) {
		wiphy_err(priv->hw->wiphy, "cannot alloc mem\n");
		return -ENOMEM;
	}
	pcie_priv->txbd_ring_vbase = mem;
	wiphy_info(priv->hw->wiphy,
		   "TX ring: - base: %p, pbase: 0x%x, len: %d\n",
		   pcie_priv->txbd_ring_vbase,
		   pcie_priv->txbd_ring_pbase,
		   pcie_priv->txbd_ring_size);

	for (num = 0; num < PCIE_MAX_TXRX_BD; num++) {
		pcie_priv->txbd_ring[num] =
			(struct pcie_data_buf *)(pcie_priv->txbd_ring_vbase +
			(sizeof(struct pcie_data_buf) * num));
		pcie_priv->txbd_ring[num]->flags = 0;
		pcie_priv->txbd_ring[num]->offset = 0;
		pcie_priv->txbd_ring[num]->frag_len = 0;
		pcie_priv->txbd_ring[num]->len = 0;
		pcie_priv->txbd_ring[num]->paddr = 0;
		pcie_priv->tx_buf_list[num] = NULL;
	}

	return 0;
}

static int pcie_tx_ring_init(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	int num, i;
	struct pcie_desc_data *desc;

	for (num = 0; num < PCIE_NUM_OF_DESC_DATA; num++) {
		skb_queue_head_init(&pcie_priv->txq[num]);
		pcie_priv->fw_desc_cnt[num] = 0;

		if (priv->chip_type == MWL8997)
			continue;

		desc = &pcie_priv->desc_data[num];

		if (desc->ptx_ring) {
			for (i = 0; i < PCIE_MAX_NUM_TX_DESC; i++) {
				desc->ptx_ring[i].status =
					cpu_to_le32(EAGLE_TXD_STATUS_IDLE);
				desc->ptx_ring[i].pphys_next =
					cpu_to_le32((u32)desc->pphys_tx_ring +
					((i + 1) *
					sizeof(struct pcie_tx_desc)));
				desc->tx_hndl[i].pdesc =
					&desc->ptx_ring[i];
				if (i < PCIE_MAX_NUM_TX_DESC - 1)
					desc->tx_hndl[i].pnext =
						&desc->tx_hndl[i + 1];
			}
			desc->ptx_ring[PCIE_MAX_NUM_TX_DESC - 1].pphys_next =
				cpu_to_le32((u32)desc->pphys_tx_ring);
			desc->tx_hndl[PCIE_MAX_NUM_TX_DESC - 1].pnext =
				&desc->tx_hndl[0];

			desc->pstale_tx_hndl = &desc->tx_hndl[0];
			desc->pnext_tx_hndl  = &desc->tx_hndl[0];
		} else {
			wiphy_err(priv->hw->wiphy, "no valid TX mem\n");
			return -ENOMEM;
		}
	}

	return 0;
}

static void pcie_tx_ring_cleanup(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	int cleaned_tx_desc = 0;
	int num, i;
	struct pcie_desc_data *desc;

	for (num = 0; num < PCIE_NUM_OF_DESC_DATA; num++) {
		skb_queue_purge(&pcie_priv->txq[num]);
		pcie_priv->fw_desc_cnt[num] = 0;

		if (priv->chip_type == MWL8997)
			continue;

		desc = &pcie_priv->desc_data[num];

		if (desc->ptx_ring) {
			for (i = 0; i < PCIE_MAX_NUM_TX_DESC; i++) {
				if (!desc->tx_hndl[i].psk_buff)
					continue;

				wiphy_debug(priv->hw->wiphy,
					    "unmapped and free'd %i %p %x\n",
					    i,
					    desc->tx_hndl[i].psk_buff->data,
					    le32_to_cpu(
					    desc->ptx_ring[i].pkt_ptr));
				pci_unmap_single(pcie_priv->pdev,
						 le32_to_cpu(
						 desc->ptx_ring[i].pkt_ptr),
						 desc->tx_hndl[i].psk_buff->len,
						 PCI_DMA_TODEVICE);
				dev_kfree_skb_any(desc->tx_hndl[i].psk_buff);
				desc->ptx_ring[i].status =
					cpu_to_le32(EAGLE_TXD_STATUS_IDLE);
				desc->ptx_ring[i].pkt_ptr = 0;
				desc->ptx_ring[i].pkt_len = 0;
				desc->tx_hndl[i].psk_buff = NULL;
				cleaned_tx_desc++;
			}
		}
	}

	wiphy_info(priv->hw->wiphy, "cleaned %i TX descr\n", cleaned_tx_desc);
}

static void pcie_tx_ring_free(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	int num;

	if (pcie_priv->desc_data[0].ptx_ring) {
		dma_free_coherent(priv->dev,
				  MAX_NUM_TX_RING_BYTES *
				  PCIE_NUM_OF_DESC_DATA,
				  pcie_priv->desc_data[0].ptx_ring,
				  pcie_priv->desc_data[0].pphys_tx_ring);
	}

	for (num = 0; num < PCIE_NUM_OF_DESC_DATA; num++) {
		if (pcie_priv->desc_data[num].ptx_ring)
			pcie_priv->desc_data[num].ptx_ring = NULL;
		pcie_priv->desc_data[num].pstale_tx_hndl = NULL;
		pcie_priv->desc_data[num].pnext_tx_hndl = NULL;
	}

	kfree(pcie_priv->desc_data[0].tx_hndl);
}

static void pcie_txbd_ring_delete(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct sk_buff *skb;
	struct pcie_tx_desc *tx_desc;
	int num;

	if (pcie_priv->txbd_ring_vbase) {
		dma_free_coherent(priv->dev,
				  pcie_priv->txbd_ring_size,
				  pcie_priv->txbd_ring_vbase,
				  pcie_priv->txbd_ring_pbase);
	}

	for (num = 0; num < PCIE_MAX_TXRX_BD; num++) {
		pcie_priv->txbd_ring[num] = NULL;
		if (pcie_priv->tx_buf_list[num]) {
			skb = pcie_priv->tx_buf_list[num];
			tx_desc = (struct pcie_tx_desc *)skb->data;

			pci_unmap_single(pcie_priv->pdev,
					 le32_to_cpu(tx_desc->pkt_ptr),
					 skb->len,
					 PCI_DMA_TODEVICE);
			dev_kfree_skb_any(skb);
		}
		pcie_priv->tx_buf_list[num] = NULL;
	}

	pcie_priv->txbd_wrptr = 0;
	pcie_priv->txbd_rdptr = 0;
	pcie_priv->txbd_ring_size = 0;
	pcie_priv->txbd_ring_vbase = NULL;
	pcie_priv->txbd_ring_pbase = 0;
}

static inline void pcie_tx_add_ccmp_hdr(u8 *pccmp_hdr,
					u8 key_id, u16 iv16, u32 iv32)
{
	struct ccmp_hdr *ccmp_h = (struct ccmp_hdr *)pccmp_hdr;

	ccmp_h->iv16 = cpu_to_le16(iv16);
	ccmp_h->rsvd = 0;
	ccmp_h->key_id = EXT_IV | (key_id << 6);
	ccmp_h->iv32 = cpu_to_le32(iv32);
}

static inline bool pcie_tx_available(struct mwl_priv *priv, int desc_num)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_tx_hndl *tx_hndl;

	if (priv->chip_type == MWL8997)
		return PCIE_TXBD_NOT_FULL(pcie_priv->txbd_wrptr,
					  pcie_priv->txbd_rdptr);

	tx_hndl = pcie_priv->desc_data[desc_num].pnext_tx_hndl;

	if (!tx_hndl->pdesc)
		return false;

	if (tx_hndl->pdesc->status != EAGLE_TXD_STATUS_IDLE) {
		/* Interrupt F/W anyway */
		if (tx_hndl->pdesc->status &
		    cpu_to_le32(EAGLE_TXD_STATUS_FW_OWNED))
			writel(MACREG_H2ARIC_BIT_PPA_READY,
			       pcie_priv->iobase1 +
			       MACREG_REG_H2A_INTERRUPT_EVENTS);
		return false;
	}

	return true;
}

static inline void pcie_tx_skb(struct mwl_priv *priv, int desc_num,
			       struct sk_buff *tx_skb)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct ieee80211_tx_info *tx_info;
	struct pcie_tx_ctrl *tx_ctrl;
	struct pcie_tx_hndl *tx_hndl = NULL;
	struct pcie_tx_desc *tx_desc;
	struct ieee80211_sta *sta;
	struct ieee80211_vif *vif;
	struct mwl_vif *mwl_vif;
	struct ieee80211_key_conf *k_conf;
	bool ccmp = false;
	struct pcie_pfu_dma_data *pfu_dma_data;
	struct pcie_dma_data *dma_data;
	struct ieee80211_hdr *wh;
	dma_addr_t dma;

	if (WARN_ON(!tx_skb))
		return;

	tx_info = IEEE80211_SKB_CB(tx_skb);
	tx_ctrl = (struct pcie_tx_ctrl *)&tx_info->status;
	sta = (struct ieee80211_sta *)tx_ctrl->sta;
	vif = (struct ieee80211_vif *)tx_ctrl->vif;
	mwl_vif = mwl_dev_get_vif(vif);
	k_conf = (struct ieee80211_key_conf *)tx_ctrl->k_conf;

	pcie_tx_encapsulate_frame(priv, tx_skb, k_conf, &ccmp);

	if (priv->chip_type == MWL8997) {
		pfu_dma_data = (struct pcie_pfu_dma_data *)tx_skb->data;
		tx_desc = &pfu_dma_data->tx_desc;
		dma_data = &pfu_dma_data->dma_data;
	} else {
		tx_hndl = pcie_priv->desc_data[desc_num].pnext_tx_hndl;
		tx_hndl->psk_buff = tx_skb;
		tx_desc = tx_hndl->pdesc;
		dma_data = (struct pcie_dma_data *)tx_skb->data;
	}
	wh = &dma_data->wh;

	if (ieee80211_is_probe_resp(wh->frame_control) &&
	    priv->dump_probe)
		wiphy_info(priv->hw->wiphy,
			  "Probe Resp: %pM\n", wh->addr1);

	if (ieee80211_is_data(wh->frame_control) ||
	    (ieee80211_is_mgmt(wh->frame_control) &&
	    ieee80211_has_protected(wh->frame_control) &&
	    !is_multicast_ether_addr(wh->addr1))) {
		if (is_multicast_ether_addr(wh->addr1)) {
			if (ccmp) {
				pcie_tx_add_ccmp_hdr(dma_data->data,
						     mwl_vif->keyidx,
						     mwl_vif->iv16,
						     mwl_vif->iv32);
				INCREASE_IV(mwl_vif->iv16, mwl_vif->iv32);
			}
		} else {
			if (ccmp) {
				if (vif->type == NL80211_IFTYPE_STATION) {
					pcie_tx_add_ccmp_hdr(dma_data->data,
							     mwl_vif->keyidx,
							     mwl_vif->iv16,
							     mwl_vif->iv32);
					INCREASE_IV(mwl_vif->iv16,
						    mwl_vif->iv32);
				} else {
					struct mwl_sta *sta_info;

					sta_info = mwl_dev_get_sta(sta);

					pcie_tx_add_ccmp_hdr(dma_data->data,
							     0,
							     sta_info->iv16,
							     sta_info->iv32);
					INCREASE_IV(sta_info->iv16,
						    sta_info->iv32);
				}
			}
		}
	}

	if (tx_info->flags & IEEE80211_TX_INTFL_DONT_ENCRYPT)
		tx_desc->flags |= PCIE_TX_WCB_FLAGS_DONT_ENCRYPT;
	if (tx_info->flags & IEEE80211_TX_CTL_NO_CCK_RATE)
		tx_desc->flags |= PCIE_TX_WCB_FLAGS_NO_CCK_RATE;
	tx_desc->tx_priority = tx_ctrl->tx_priority;
	tx_desc->qos_ctrl = cpu_to_le16(tx_ctrl->qos_ctrl);
	tx_desc->pkt_len = cpu_to_le16(tx_skb->len);
	tx_desc->packet_info = 0;
	tx_desc->data_rate = 0;
	tx_desc->type = tx_ctrl->type;
	tx_desc->xmit_control = tx_ctrl->xmit_control;
	tx_desc->sap_pkt_info = 0;
	dma = pci_map_single(pcie_priv->pdev, tx_skb->data,
			     tx_skb->len, PCI_DMA_TODEVICE);
	if (pci_dma_mapping_error(pcie_priv->pdev, dma)) {
		dev_kfree_skb_any(tx_skb);
		wiphy_err(priv->hw->wiphy,
			  "failed to map pci memory!\n");
		return;
	}
	if (priv->chip_type == MWL8997)
		tx_desc->pkt_ptr = cpu_to_le32(sizeof(struct pcie_tx_desc));
	else
		tx_desc->pkt_ptr = cpu_to_le32(dma);
	tx_desc->status = cpu_to_le32(EAGLE_TXD_STATUS_FW_OWNED);
	/* make sure all the memory transactions done by cpu were completed */
	wmb();	/*Data Memory Barrier*/

	if (priv->chip_type == MWL8997) {
		u32 wrindx;
		struct pcie_data_buf *data_buf;
		const u32 num_tx_buffs = PCIE_MAX_TXRX_BD << PCIE_TX_START_PTR;

		wrindx = (pcie_priv->txbd_wrptr & PCIE_TXBD_MASK) >>
			PCIE_TX_START_PTR;
		pcie_priv->tx_buf_list[wrindx] = tx_skb;
		data_buf = pcie_priv->txbd_ring[wrindx];
		data_buf->paddr = cpu_to_le64(dma);
		data_buf->len = cpu_to_le16(tx_skb->len);
		data_buf->flags = cpu_to_le16(PCIE_BD_FLAG_FIRST_DESC |
					      PCIE_BD_FLAG_LAST_DESC);
		data_buf->frag_len = cpu_to_le16(tx_skb->len);
		data_buf->offset = 0;
		pcie_priv->txbd_wrptr += PCIE_BD_FLAG_TX_START_PTR;

		if ((pcie_priv->txbd_wrptr & PCIE_TXBD_MASK) == num_tx_buffs)
			pcie_priv->txbd_wrptr = ((pcie_priv->txbd_wrptr &
			PCIE_BD_FLAG_TX_ROLLOVER_IND) ^
			PCIE_BD_FLAG_TX_ROLLOVER_IND);

		/* Write the TX ring write pointer in to REG_TXBD_WRPTR */
		writel(pcie_priv->txbd_wrptr,
		       pcie_priv->iobase1 + REG_TXBD_WRPTR);
	} else {
		writel(MACREG_H2ARIC_BIT_PPA_READY,
		       pcie_priv->iobase1 + MACREG_REG_H2A_INTERRUPT_EVENTS);
		pcie_priv->desc_data[desc_num].pnext_tx_hndl = tx_hndl->pnext;
		pcie_priv->fw_desc_cnt[desc_num]++;
	}
}

static inline
struct sk_buff *pcie_tx_do_amsdu(struct mwl_priv *priv,
				 int desc_num,
				 struct sk_buff *tx_skb,
				 struct ieee80211_tx_info *tx_info)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct ieee80211_sta *sta;
	struct mwl_sta *sta_info;
	struct pcie_tx_ctrl *tx_ctrl = (struct pcie_tx_ctrl *)&tx_info->status;
	struct ieee80211_tx_info *amsdu_info;
	struct sk_buff_head *amsdu_pkts;
	struct mwl_amsdu_frag *amsdu;
	int amsdu_allow_size;
	struct ieee80211_hdr *wh;
	int wh_len;
	u16 len;
	u8 *data;

	sta = (struct ieee80211_sta *)tx_ctrl->sta;
	sta_info = mwl_dev_get_sta(sta);

	if (!sta_info->is_amsdu_allowed)
		return tx_skb;

	wh = (struct ieee80211_hdr *)tx_skb->data;
	if (sta_info->is_mesh_node && is_multicast_ether_addr(wh->addr3))
		return tx_skb;

	if (ieee80211_is_qos_nullfunc(wh->frame_control))
		return tx_skb;

	if (sta_info->amsdu_ctrl.cap == MWL_AMSDU_SIZE_4K)
		amsdu_allow_size = SYSADPT_AMSDU_4K_MAX_SIZE;
	else if (sta_info->amsdu_ctrl.cap == MWL_AMSDU_SIZE_8K)
		amsdu_allow_size = SYSADPT_AMSDU_8K_MAX_SIZE;
	else
		return tx_skb;

	spin_lock_bh(&sta_info->amsdu_lock);
	amsdu = &sta_info->amsdu_ctrl.frag[desc_num];

	if ((tx_skb->len > SYSADPT_AMSDU_ALLOW_SIZE) ||
	    utils_is_non_amsdu_packet(tx_skb->data, true)) {
		if (amsdu->num) {
			pcie_tx_skb(priv, desc_num, amsdu->skb);
			amsdu->num = 0;
			amsdu->cur_pos = NULL;
		}
		spin_unlock_bh(&sta_info->amsdu_lock);
		return tx_skb;
	}

	/* potential amsdu size, should add amsdu header 14 bytes +
	 * maximum padding 3.
	 */
	wh_len = ieee80211_hdrlen(wh->frame_control);
	len = tx_skb->len - wh_len + 17;

	if (amsdu->num) {
		if ((amsdu->skb->len + len) > amsdu_allow_size) {
			pcie_tx_skb(priv, desc_num, amsdu->skb);
			amsdu->num = 0;
			amsdu->cur_pos = NULL;
		}
	}

	amsdu->jiffies = jiffies;
	len = tx_skb->len - wh_len;

	if (amsdu->num == 0) {
		struct sk_buff *newskb;
		int headroom;

		amsdu_pkts = (struct sk_buff_head *)
			kmalloc(sizeof(*amsdu_pkts), GFP_ATOMIC);
		if (!amsdu_pkts) {
			spin_unlock_bh(&sta_info->amsdu_lock);
			return tx_skb;
		}
		newskb = dev_alloc_skb(amsdu_allow_size +
				       pcie_priv->tx_head_room);
		if (!newskb) {
			spin_unlock_bh(&sta_info->amsdu_lock);
			kfree(amsdu_pkts);
			return tx_skb;
		}

		headroom = skb_headroom(newskb);
		if (headroom < pcie_priv->tx_head_room)
			skb_reserve(newskb,
				    (pcie_priv->tx_head_room - headroom));

		data = newskb->data;
		memcpy(data, tx_skb->data, wh_len);
		if (sta_info->is_mesh_node) {
			ether_addr_copy(data + wh_len, wh->addr3);
			ether_addr_copy(data + wh_len + ETH_ALEN, wh->addr4);
		} else {
			ether_addr_copy(data + wh_len,
					ieee80211_get_DA(wh));
			ether_addr_copy(data + wh_len + ETH_ALEN,
					ieee80211_get_SA(wh));
		}
		*(u8 *)(data + wh_len + ETH_HLEN - 1) = len & 0xff;
		*(u8 *)(data + wh_len + ETH_HLEN - 2) = (len >> 8) & 0xff;
		memcpy(data + wh_len + ETH_HLEN, tx_skb->data + wh_len, len);

		skb_put(newskb, tx_skb->len + ETH_HLEN);
		tx_ctrl->qos_ctrl |= IEEE80211_QOS_CTL_A_MSDU_PRESENT;
		amsdu_info = IEEE80211_SKB_CB(newskb);
		memcpy(amsdu_info, tx_info, sizeof(*tx_info));
		skb_queue_head_init(amsdu_pkts);
		((struct pcie_tx_ctrl *)&amsdu_info->status)->amsdu_pkts =
			(void *)amsdu_pkts;
		amsdu->skb = newskb;
	} else {
		amsdu->cur_pos += amsdu->pad;
		data = amsdu->cur_pos;

		if (sta_info->is_mesh_node) {
			ether_addr_copy(data, wh->addr3);
			ether_addr_copy(data + ETH_ALEN, wh->addr4);
		} else {
			ether_addr_copy(data, ieee80211_get_DA(wh));
			ether_addr_copy(data + ETH_ALEN, ieee80211_get_SA(wh));
		}
		*(u8 *)(data + ETH_HLEN - 1) = len & 0xff;
		*(u8 *)(data + ETH_HLEN - 2) = (len >> 8) & 0xff;
		memcpy(data + ETH_HLEN, tx_skb->data + wh_len, len);

		skb_put(amsdu->skb, len + ETH_HLEN + amsdu->pad);
		amsdu_info = IEEE80211_SKB_CB(amsdu->skb);
		amsdu_pkts = (struct sk_buff_head *)
			((struct pcie_tx_ctrl *)
			&amsdu_info->status)->amsdu_pkts;
	}

	amsdu->num++;
	amsdu->pad = ((len + ETH_HLEN) % 4) ? (4 - (len + ETH_HLEN) % 4) : 0;
	amsdu->cur_pos = amsdu->skb->data + amsdu->skb->len;
	skb_queue_tail(amsdu_pkts, tx_skb);

	if (amsdu->num > SYSADPT_AMSDU_PACKET_THRESHOLD) {
		amsdu->num = 0;
		amsdu->cur_pos = NULL;
		spin_unlock_bh(&sta_info->amsdu_lock);
		return amsdu->skb;
	}

	spin_unlock_bh(&sta_info->amsdu_lock);
	return NULL;
}

static inline void pcie_tx_ack_amsdu_pkts(struct ieee80211_hw *hw, u32 rate,
					  struct sk_buff_head *amsdu_pkts)
{
	struct sk_buff *amsdu_pkt;
	struct ieee80211_tx_info *info;

	while (skb_queue_len(amsdu_pkts) > 0) {
		amsdu_pkt = skb_dequeue(amsdu_pkts);
		info = IEEE80211_SKB_CB(amsdu_pkt);
		pcie_tx_prepare_info(hw->priv, rate, info);
		ieee80211_tx_status(hw, amsdu_pkt);
	}

	kfree(amsdu_pkts);
}

static void pcie_pfu_tx_done(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	u32 wrdoneidx, rdptr;
	const u32 num_tx_buffs = PCIE_MAX_TXRX_BD << PCIE_TX_START_PTR;
	struct pcie_data_buf *data_buf;
	struct sk_buff *done_skb;
	struct pcie_pfu_dma_data *pfu_dma;
	struct pcie_tx_desc *tx_desc;
	struct pcie_dma_data *dma_data;
	struct ieee80211_hdr *wh;
	struct ieee80211_tx_info *info;
	struct pcie_tx_ctrl *tx_ctrl;
	struct ieee80211_sta *sta;
	struct mwl_sta *sta_info;
	u32 rate = 0;
	struct sk_buff_head *amsdu_pkts;
	int hdrlen;

	spin_lock_bh(&pcie_priv->tx_desc_lock);
	/* Read the TX ring read pointer set by firmware */
	rdptr = readl(pcie_priv->iobase1 + REG_TXBD_RDPTR);
	/* free from previous txbd_rdptr to current txbd_rdptr */
	while (((pcie_priv->txbd_rdptr & PCIE_TXBD_MASK) !=
	       (rdptr & PCIE_TXBD_MASK)) ||
	       ((pcie_priv->txbd_rdptr & PCIE_BD_FLAG_TX_ROLLOVER_IND) !=
	       (rdptr & PCIE_BD_FLAG_TX_ROLLOVER_IND))) {
		wrdoneidx = pcie_priv->txbd_rdptr & PCIE_TXBD_MASK;
		wrdoneidx >>= PCIE_TX_START_PTR;

		data_buf = pcie_priv->txbd_ring[wrdoneidx];
		done_skb = pcie_priv->tx_buf_list[wrdoneidx];
		if (done_skb) {
			pfu_dma = (struct pcie_pfu_dma_data *)done_skb->data;
			tx_desc = &pfu_dma->tx_desc;
			dma_data = &pfu_dma->dma_data;
			pci_unmap_single(pcie_priv->pdev,
					 le32_to_cpu(data_buf->paddr),
					 le16_to_cpu(data_buf->len),
					 PCI_DMA_TODEVICE);
			tx_desc->pkt_ptr = 0;
			tx_desc->pkt_len = 0;
			tx_desc->status = cpu_to_le32(EAGLE_TXD_STATUS_IDLE);
			wmb(); /* memory barrier */

			wh = &dma_data->wh;
			if (ieee80211_is_nullfunc(wh->frame_control) ||
			    ieee80211_is_qos_nullfunc(wh->frame_control)) {
				dev_kfree_skb_any(done_skb);
				done_skb = NULL;
				goto next;
			}

			info = IEEE80211_SKB_CB(done_skb);
			tx_ctrl = (struct pcie_tx_ctrl *)&info->status;
			sta = (struct ieee80211_sta *)tx_ctrl->sta;
			if (sta) {
				sta_info = mwl_dev_get_sta(sta);
				rate = sta_info->tx_rate_info;
			}

			if (ieee80211_is_data(wh->frame_control) ||
			    ieee80211_is_data_qos(wh->frame_control)) {
				amsdu_pkts = (struct sk_buff_head *)
					tx_ctrl->amsdu_pkts;
				if (amsdu_pkts) {
					pcie_tx_ack_amsdu_pkts(priv->hw, rate,
							       amsdu_pkts);
					dev_kfree_skb_any(done_skb);
					done_skb = NULL;
				} else {
					pcie_tx_prepare_info(priv, rate, info);
				}
			} else {
				pcie_tx_prepare_info(priv, 0, info);
			}

			if (done_skb) {
				/* Remove H/W dma header */
				hdrlen = ieee80211_hdrlen(
					dma_data->wh.frame_control);
				memmove(dma_data->data - hdrlen,
					&dma_data->wh, hdrlen);
				skb_pull(done_skb, sizeof(*pfu_dma) - hdrlen);
				ieee80211_tx_status(priv->hw, done_skb);
			}
		}
next:
		memset(data_buf, 0, sizeof(*data_buf));
		pcie_priv->tx_buf_list[wrdoneidx] = NULL;

		pcie_priv->txbd_rdptr += PCIE_BD_FLAG_TX_START_PTR;
		if ((pcie_priv->txbd_rdptr & PCIE_TXBD_MASK) == num_tx_buffs)
			pcie_priv->txbd_rdptr = ((pcie_priv->txbd_rdptr &
				PCIE_BD_FLAG_TX_ROLLOVER_IND) ^
				PCIE_BD_FLAG_TX_ROLLOVER_IND);
	}
	spin_unlock_bh(&pcie_priv->tx_desc_lock);

	if (pcie_priv->is_tx_done_schedule) {
		pcie_mask_int(pcie_priv, MACREG_A2HRIC_BIT_TX_DONE, true);
		tasklet_schedule(&pcie_priv->tx_task);
		pcie_priv->is_tx_done_schedule = false;
	}
}

static void pcie_non_pfu_tx_done(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	int num;
	struct pcie_desc_data *desc;
	struct pcie_tx_hndl *tx_hndl;
	struct pcie_tx_desc *tx_desc;
	struct sk_buff *done_skb;
	int idx;
	u32 rate;
	struct pcie_dma_data *dma_data;
	struct ieee80211_hdr *wh;
	struct ieee80211_tx_info *info;
	struct pcie_tx_ctrl *tx_ctrl;
	struct sk_buff_head *amsdu_pkts;
	int hdrlen;

	spin_lock_bh(&pcie_priv->tx_desc_lock);
	for (num = 0; num < SYSADPT_TX_WMM_QUEUES; num++) {
		desc = &pcie_priv->desc_data[num];
		tx_hndl = desc->pstale_tx_hndl;
		tx_desc = tx_hndl->pdesc;

		if ((tx_desc->status &
		    cpu_to_le32(EAGLE_TXD_STATUS_FW_OWNED)) &&
		    (tx_hndl->pnext->pdesc->status &
		    cpu_to_le32(EAGLE_TXD_STATUS_OK)))
			tx_desc->status = cpu_to_le32(EAGLE_TXD_STATUS_OK);

		while (tx_hndl &&
		       (tx_desc->status & cpu_to_le32(EAGLE_TXD_STATUS_OK)) &&
		       (!(tx_desc->status &
		       cpu_to_le32(EAGLE_TXD_STATUS_FW_OWNED)))) {
			pci_unmap_single(pcie_priv->pdev,
					 le32_to_cpu(tx_desc->pkt_ptr),
					 le16_to_cpu(tx_desc->pkt_len),
					 PCI_DMA_TODEVICE);
			done_skb = tx_hndl->psk_buff;
			rate = le32_to_cpu(tx_desc->rate_info);
			tx_desc->pkt_ptr = 0;
			tx_desc->pkt_len = 0;
			tx_desc->status =
				cpu_to_le32(EAGLE_TXD_STATUS_IDLE);
			tx_hndl->psk_buff = NULL;
			wmb(); /*Data Memory Barrier*/

			skb_get(done_skb);
			idx = pcie_priv->delay_q_idx;
			if (pcie_priv->delay_q[idx])
				dev_kfree_skb_any(pcie_priv->delay_q[idx]);
			pcie_priv->delay_q[idx] = done_skb;
			idx++;
			if (idx >= PCIE_DELAY_FREE_Q_LIMIT)
				idx = 0;
			pcie_priv->delay_q_idx = idx;

			dma_data = (struct pcie_dma_data *)done_skb->data;
			wh = &dma_data->wh;
			if (ieee80211_is_nullfunc(wh->frame_control) ||
			    ieee80211_is_qos_nullfunc(wh->frame_control)) {
				dev_kfree_skb_any(done_skb);
				done_skb = NULL;
				goto next;
			}

			info = IEEE80211_SKB_CB(done_skb);
			if (ieee80211_is_data(wh->frame_control) ||
			    ieee80211_is_data_qos(wh->frame_control)) {
				tx_ctrl = (struct pcie_tx_ctrl *)&info->status;
				amsdu_pkts = (struct sk_buff_head *)
					tx_ctrl->amsdu_pkts;
				if (amsdu_pkts) {
					pcie_tx_ack_amsdu_pkts(priv->hw, rate,
							       amsdu_pkts);
					dev_kfree_skb_any(done_skb);
					done_skb = NULL;
				} else {
					pcie_tx_prepare_info(priv, rate, info);
				}
			} else {
				pcie_tx_prepare_info(priv, 0, info);
			}

			if (done_skb) {
				/* Remove H/W dma header */
				hdrlen = ieee80211_hdrlen(
					dma_data->wh.frame_control);
				memmove(dma_data->data - hdrlen,
					&dma_data->wh, hdrlen);
				skb_pull(done_skb, sizeof(*dma_data) - hdrlen);
				ieee80211_tx_status(priv->hw, done_skb);
			}
next:
			tx_hndl = tx_hndl->pnext;
			tx_desc = tx_hndl->pdesc;
			pcie_priv->fw_desc_cnt[num]--;
		}

		desc->pstale_tx_hndl = tx_hndl;
	}
	spin_unlock_bh(&pcie_priv->tx_desc_lock);

	if (pcie_priv->is_tx_done_schedule) {
		pcie_mask_int(pcie_priv, MACREG_A2HRIC_BIT_TX_DONE, true);
		tasklet_schedule(&pcie_priv->tx_task);
		pcie_priv->is_tx_done_schedule = false;
	}
}

int pcie_tx_init(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	int rc;
	int i;

	if (priv->chip_type == MWL8997)
		rc = pcie_txbd_ring_create(priv);
	else
		rc = pcie_tx_ring_alloc(priv);

	if (rc) {
		wiphy_err(hw->wiphy, "allocating TX ring failed\n");
		return rc;
	}

	rc = pcie_tx_ring_init(priv);
	if (rc) {
		pcie_tx_ring_free(priv);
		wiphy_err(hw->wiphy, "initializing TX ring failed\n");
		return rc;
	}

	pcie_priv->delay_q_idx = 0;
	for (i = 0; i < PCIE_DELAY_FREE_Q_LIMIT; i++)
		pcie_priv->delay_q[i] = NULL;

	return 0;
}

void pcie_tx_deinit(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	int i;

	for (i = 0; i < PCIE_DELAY_FREE_Q_LIMIT; i++)
		if (pcie_priv->delay_q[i])
			dev_kfree_skb_any(pcie_priv->delay_q[i]);

	pcie_tx_ring_cleanup(priv);

	if (priv->chip_type == MWL8997)
		pcie_txbd_ring_delete(priv);
	else
		pcie_tx_ring_free(priv);
}

void pcie_tx_skbs(unsigned long data)
{
	struct ieee80211_hw *hw = (struct ieee80211_hw *)data;
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	int num = SYSADPT_TX_WMM_QUEUES;
	struct sk_buff *tx_skb;

	spin_lock_bh(&pcie_priv->tx_desc_lock);
	while (num--) {
		while (skb_queue_len(&pcie_priv->txq[num]) > 0) {
			struct ieee80211_tx_info *tx_info;
			struct pcie_tx_ctrl *tx_ctrl;

			if (!pcie_tx_available(priv, num))
				break;

			tx_skb = skb_dequeue(&pcie_priv->txq[num]);
			if (!tx_skb)
				continue;
			tx_info = IEEE80211_SKB_CB(tx_skb);
			tx_ctrl = (struct pcie_tx_ctrl *)&tx_info->status;

			if (tx_ctrl->tx_priority >= SYSADPT_TX_WMM_QUEUES)
				tx_skb = pcie_tx_do_amsdu(priv, num,
							  tx_skb, tx_info);

			if (tx_skb) {
				if (pcie_tx_available(priv, num))
					pcie_tx_skb(priv, num, tx_skb);
				else
					skb_queue_head(&pcie_priv->txq[num],
						       tx_skb);
			}
		}

		if (skb_queue_len(&pcie_priv->txq[num]) <
		    pcie_priv->txq_wake_threshold) {
			int queue;

			queue = SYSADPT_TX_WMM_QUEUES - num - 1;
			if (ieee80211_queue_stopped(hw, queue))
				ieee80211_wake_queue(hw, queue);
		}
	}
	spin_unlock_bh(&pcie_priv->tx_desc_lock);
}

void pcie_tx_flush_amsdu(unsigned long data)
{
	struct ieee80211_hw *hw = (struct ieee80211_hw *)data;
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct mwl_sta *sta_info;
	int i;
	struct mwl_amsdu_frag *amsdu_frag;

	spin_lock(&priv->sta_lock);
	list_for_each_entry(sta_info, &priv->sta_list, list) {
		spin_lock(&pcie_priv->tx_desc_lock);
		spin_lock(&sta_info->amsdu_lock);
		for (i = 0; i < SYSADPT_TX_WMM_QUEUES; i++) {
			amsdu_frag = &sta_info->amsdu_ctrl.frag[i];
			if (amsdu_frag->num) {
				if (time_after(jiffies,
					       (amsdu_frag->jiffies + 1))) {
					if (pcie_tx_available(priv, i)) {
						pcie_tx_skb(priv, i,
							    amsdu_frag->skb);
						amsdu_frag->num = 0;
						amsdu_frag->cur_pos = NULL;
					}
				}
			}
		}
		spin_unlock(&sta_info->amsdu_lock);
		spin_unlock(&pcie_priv->tx_desc_lock);
	}
	spin_unlock(&priv->sta_lock);

	pcie_mask_int(pcie_priv, MACREG_A2HRIC_BIT_QUE_EMPTY, true);
	pcie_priv->is_qe_schedule = false;
}

void pcie_tx_done(unsigned long data)
{
	struct ieee80211_hw *hw = (struct ieee80211_hw *)data;
	struct mwl_priv *priv = hw->priv;

	if (priv->chip_type == MWL8997)
		pcie_pfu_tx_done(priv);
	else
		pcie_non_pfu_tx_done(priv);
}

void pcie_tx_xmit(struct ieee80211_hw *hw,
		  struct ieee80211_tx_control *control,
		  struct sk_buff *skb)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	int index;
	struct ieee80211_sta *sta;
	struct ieee80211_tx_info *tx_info;
	struct mwl_vif *mwl_vif;
	struct ieee80211_hdr *wh;
	u8 xmitcontrol;
	u16 qos;
	int txpriority;
	u8 tid = 0;
	struct mwl_ampdu_stream *stream = NULL;
	bool start_ba_session = false;
	bool mgmtframe = false;
	struct ieee80211_mgmt *mgmt;
	bool eapol_frame = false;
	struct pcie_tx_ctrl *tx_ctrl;
	struct ieee80211_key_conf *k_conf = NULL;
	int rc;

	index = skb_get_queue_mapping(skb);
	sta = control->sta;

	wh = (struct ieee80211_hdr *)skb->data;
	tx_info = IEEE80211_SKB_CB(skb);
	mwl_vif = mwl_dev_get_vif(tx_info->control.vif);

	if (ieee80211_is_data_qos(wh->frame_control))
		qos = le16_to_cpu(*((__le16 *)ieee80211_get_qos_ctl(wh)));
	else
		qos = 0;

	if (ieee80211_is_mgmt(wh->frame_control)) {
		mgmtframe = true;
		mgmt = (struct ieee80211_mgmt *)skb->data;
	} else {
		u16 pkt_type;
		struct mwl_sta *sta_info;

		pkt_type = be16_to_cpu(*((__be16 *)
			&skb->data[ieee80211_hdrlen(wh->frame_control) + 6]));
		if (pkt_type == ETH_P_PAE) {
			index = IEEE80211_AC_VO;
			eapol_frame = true;
		}
		if (sta) {
			if (mwl_vif->is_hw_crypto_enabled) {
				sta_info = mwl_dev_get_sta(sta);
				if (!sta_info->is_key_set && !eapol_frame) {
					dev_kfree_skb_any(skb);
					return;
				}
			}
		}
	}

	if (tx_info->flags & IEEE80211_TX_CTL_ASSIGN_SEQ) {
		wh->seq_ctrl &= cpu_to_le16(IEEE80211_SCTL_FRAG);
		wh->seq_ctrl |= cpu_to_le16(mwl_vif->seqno);
		mwl_vif->seqno += 0x10;
	}

	/* Setup firmware control bit fields for each frame type. */
	xmitcontrol = 0;

	if (mgmtframe || ieee80211_is_ctl(wh->frame_control)) {
		qos = 0;
	} else if (ieee80211_is_data(wh->frame_control)) {
		qos &= ~MWL_QOS_ACK_POLICY_MASK;

		if (tx_info->flags & IEEE80211_TX_CTL_AMPDU) {
			xmitcontrol &= 0xfb;
			qos |= MWL_QOS_ACK_POLICY_BLOCKACK;
		} else {
			xmitcontrol |= 0x4;
			qos |= MWL_QOS_ACK_POLICY_NORMAL;
		}

		if (is_multicast_ether_addr(wh->addr1) || eapol_frame)
			xmitcontrol |= EAGLE_TXD_XMITCTRL_USE_MC_RATE;
	}

	k_conf = tx_info->control.hw_key;

	/* Queue ADDBA request in the respective data queue.  While setting up
	 * the ampdu stream, mac80211 queues further packets for that
	 * particular ra/tid pair.  However, packets piled up in the hardware
	 * for that ra/tid pair will still go out. ADDBA request and the
	 * related data packets going out from different queues asynchronously
	 * will cause a shift in the receiver window which might result in
	 * ampdu packets getting dropped at the receiver after the stream has
	 * been setup.
	 */
	if (mgmtframe) {
		u16 capab;

		if (unlikely(ieee80211_is_action(wh->frame_control) &&
			     mgmt->u.action.category == WLAN_CATEGORY_BACK &&
			     mgmt->u.action.u.addba_req.action_code ==
			     WLAN_ACTION_ADDBA_REQ)) {
			capab = le16_to_cpu(mgmt->u.action.u.addba_req.capab);
			tid = (capab & IEEE80211_ADDBA_PARAM_TID_MASK) >> 2;
			index = utils_tid_to_ac(tid);
		}

		if (unlikely(ieee80211_is_assoc_req(wh->frame_control)))
			utils_add_basic_rates(hw->conf.chandef.chan->band, skb);
	}

	index = SYSADPT_TX_WMM_QUEUES - index - 1;
	txpriority = index;

	if (sta && sta->ht_cap.ht_supported && !eapol_frame &&
	    ieee80211_is_data_qos(wh->frame_control)) {
		tid = qos & 0xf;
		pcie_tx_count_packet(sta, tid);

		spin_lock_bh(&priv->stream_lock);
		stream = mwl_fwcmd_lookup_stream(hw, sta, tid);

		if (stream) {
			if (stream->state == AMPDU_STREAM_ACTIVE) {
				if (WARN_ON(!(qos &
					    MWL_QOS_ACK_POLICY_BLOCKACK))) {
					spin_unlock_bh(&priv->stream_lock);
					dev_kfree_skb_any(skb);
					return;
				}

				txpriority =
					(SYSADPT_TX_WMM_QUEUES + stream->idx) %
					TOTAL_HW_QUEUES;
			} else if (stream->state == AMPDU_STREAM_NEW) {
				/* We get here if the driver sends us packets
				 * after we've initiated a stream, but before
				 * our ampdu_action routine has been called
				 * with IEEE80211_AMPDU_TX_START to get the SSN
				 * for the ADDBA request.  So this packet can
				 * go out with no risk of sequence number
				 * mismatch.  No special handling is required.
				 */
			} else {
				/* Drop packets that would go out after the
				 * ADDBA request was sent but before the ADDBA
				 * response is received.  If we don't do this,
				 * the recipient would probably receive it
				 * after the ADDBA request with SSN 0.  This
				 * will cause the recipient's BA receive window
				 * to shift, which would cause the subsequent
				 * packets in the BA stream to be discarded.
				 * mac80211 queues our packets for us in this
				 * case, so this is really just a safety check.
				 */
				wiphy_warn(hw->wiphy,
					   "can't send packet during ADDBA\n");
				spin_unlock_bh(&priv->stream_lock);
				dev_kfree_skb_any(skb);
				return;
			}
		} else {
			if (mwl_fwcmd_ampdu_allowed(sta, tid)) {
				stream = mwl_fwcmd_add_stream(hw, sta, tid);

				if (stream)
					start_ba_session = true;
			}
		}

		spin_unlock_bh(&priv->stream_lock);
	} else {
		qos &= ~MWL_QOS_ACK_POLICY_MASK;
		qos |= MWL_QOS_ACK_POLICY_NORMAL;
	}

	tx_ctrl = (struct pcie_tx_ctrl *)&tx_info->status;
	tx_ctrl->vif = (void *)tx_info->control.vif;
	tx_ctrl->sta = (void *)sta;
	tx_ctrl->k_conf = (void *)k_conf;
	tx_ctrl->amsdu_pkts = NULL;
	tx_ctrl->tx_priority = txpriority;
	tx_ctrl->type = (mgmtframe ? IEEE_TYPE_MANAGEMENT : IEEE_TYPE_DATA);
	tx_ctrl->qos_ctrl = qos;
	tx_ctrl->xmit_control = xmitcontrol;

	if (skb_queue_len(&pcie_priv->txq[index]) > pcie_priv->txq_limit)
		ieee80211_stop_queue(hw, SYSADPT_TX_WMM_QUEUES - index - 1);

	skb_queue_tail(&pcie_priv->txq[index], skb);

	tasklet_schedule(&pcie_priv->tx_task);

	/* Initiate the ampdu session here */
	if (start_ba_session) {
		spin_lock_bh(&priv->stream_lock);
		rc = mwl_fwcmd_start_stream(hw, stream);
		if (rc)
			mwl_fwcmd_remove_stream(hw, stream);
		else
			wiphy_debug(hw->wiphy, "Mac80211 start BA %pM\n",
				    stream->sta->addr);
		spin_unlock_bh(&priv->stream_lock);
	}
}

void pcie_tx_del_pkts_via_vif(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	int num;
	struct sk_buff *skb, *tmp;
	struct ieee80211_tx_info *tx_info;
	struct pcie_tx_ctrl *tx_ctrl;
	struct sk_buff_head *amsdu_pkts;
	unsigned long flags;

	for (num = 1; num < PCIE_NUM_OF_DESC_DATA; num++) {
		spin_lock_irqsave(&pcie_priv->txq[num].lock, flags);
		skb_queue_walk_safe(&pcie_priv->txq[num], skb, tmp) {
			tx_info = IEEE80211_SKB_CB(skb);
			tx_ctrl = (struct pcie_tx_ctrl *)&tx_info->status;
			if (tx_ctrl->vif == vif) {
				amsdu_pkts = (struct sk_buff_head *)
					tx_ctrl->amsdu_pkts;
				if (amsdu_pkts) {
					skb_queue_purge(amsdu_pkts);
					kfree(amsdu_pkts);
				}
				__skb_unlink(skb, &pcie_priv->txq[num]);
				dev_kfree_skb_any(skb);
			}
		}
		spin_unlock_irqrestore(&pcie_priv->txq[num].lock, flags);
	}
}

void pcie_tx_del_pkts_via_sta(struct ieee80211_hw *hw,
			      struct ieee80211_sta *sta)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	int num;
	struct sk_buff *skb, *tmp;
	struct ieee80211_tx_info *tx_info;
	struct pcie_tx_ctrl *tx_ctrl;
	struct sk_buff_head *amsdu_pkts;
	unsigned long flags;

	for (num = 1; num < PCIE_NUM_OF_DESC_DATA; num++) {
		spin_lock_irqsave(&pcie_priv->txq[num].lock, flags);
		skb_queue_walk_safe(&pcie_priv->txq[num], skb, tmp) {
			tx_info = IEEE80211_SKB_CB(skb);
			tx_ctrl = (struct pcie_tx_ctrl *)&tx_info->status;
			if (tx_ctrl->sta == sta) {
				amsdu_pkts = (struct sk_buff_head *)
					tx_ctrl->amsdu_pkts;
				if (amsdu_pkts) {
					skb_queue_purge(amsdu_pkts);
					kfree(amsdu_pkts);
				}
				__skb_unlink(skb, &pcie_priv->txq[num]);
				dev_kfree_skb_any(skb);
			}
		}
		spin_unlock_irqrestore(&pcie_priv->txq[num].lock, flags);
	}
}

void pcie_tx_del_ampdu_pkts(struct ieee80211_hw *hw,
			    struct ieee80211_sta *sta, u8 tid)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct mwl_sta *sta_info = mwl_dev_get_sta(sta);
	int ac, desc_num;
	struct mwl_amsdu_frag *amsdu_frag;
	struct sk_buff *skb, *tmp;
	struct ieee80211_tx_info *tx_info;
	struct pcie_tx_ctrl *tx_ctrl;
	struct sk_buff_head *amsdu_pkts;
	unsigned long flags;

	ac = utils_tid_to_ac(tid);
	desc_num = SYSADPT_TX_WMM_QUEUES - ac - 1;
	spin_lock_irqsave(&pcie_priv->txq[desc_num].lock, flags);
	skb_queue_walk_safe(&pcie_priv->txq[desc_num], skb, tmp) {
		tx_info = IEEE80211_SKB_CB(skb);
		tx_ctrl = (struct pcie_tx_ctrl *)&tx_info->status;
		if (tx_ctrl->sta == sta) {
			amsdu_pkts = (struct sk_buff_head *)
				tx_ctrl->amsdu_pkts;
			if (amsdu_pkts) {
				skb_queue_purge(amsdu_pkts);
				kfree(amsdu_pkts);
			}
			__skb_unlink(skb, &pcie_priv->txq[desc_num]);
			dev_kfree_skb_any(skb);
		}
	}
	spin_unlock_irqrestore(&pcie_priv->txq[desc_num].lock, flags);

	spin_lock_bh(&sta_info->amsdu_lock);
	amsdu_frag = &sta_info->amsdu_ctrl.frag[desc_num];
	if (amsdu_frag->num) {
		amsdu_frag->num = 0;
		amsdu_frag->cur_pos = NULL;
		if (amsdu_frag->skb) {
			tx_info = IEEE80211_SKB_CB(amsdu_frag->skb);
			tx_ctrl = (struct pcie_tx_ctrl *)&tx_info->status;
			amsdu_pkts = (struct sk_buff_head *)
				tx_ctrl->amsdu_pkts;
			if (amsdu_pkts) {
				skb_queue_purge(amsdu_pkts);
				kfree(amsdu_pkts);
			}
			dev_kfree_skb_any(amsdu_frag->skb);
		}
	}
	spin_unlock_bh(&sta_info->amsdu_lock);
}

void pcie_tx_del_sta_amsdu_pkts(struct ieee80211_hw *hw,
				struct ieee80211_sta *sta)
{
	struct mwl_sta *sta_info = mwl_dev_get_sta(sta);
	int num;
	struct mwl_amsdu_frag *amsdu_frag;
	struct ieee80211_tx_info *tx_info;
	struct pcie_tx_ctrl *tx_ctrl;
	struct sk_buff_head *amsdu_pkts;

	spin_lock_bh(&sta_info->amsdu_lock);
	for (num = 0; num < SYSADPT_TX_WMM_QUEUES; num++) {
		amsdu_frag = &sta_info->amsdu_ctrl.frag[num];
		if (amsdu_frag->num) {
			amsdu_frag->num = 0;
			amsdu_frag->cur_pos = NULL;
			if (amsdu_frag->skb) {
				tx_info = IEEE80211_SKB_CB(amsdu_frag->skb);
				tx_ctrl = (struct pcie_tx_ctrl *)
					&tx_info->status;
				amsdu_pkts = (struct sk_buff_head *)
					tx_ctrl->amsdu_pkts;
				if (amsdu_pkts) {
					skb_queue_purge(amsdu_pkts);
					kfree(amsdu_pkts);
				}
				dev_kfree_skb_any(amsdu_frag->skb);
			}
		}
	}
	spin_unlock_bh(&sta_info->amsdu_lock);
}
