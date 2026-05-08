/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/debugfs.h>
#include <asm/cacheflush.h>

#include "nss_dp_dev.h"
#include "edma_regs.h"
#include "edma_debug.h"
#include "edma.h"
#include <ppe_drv_sc.h>

/*
 * edma_tx_complete()
 *	Reap Tx descriptors
 */
uint32_t edma_tx_complete(uint32_t work_to_do, struct edma_txcmpl_ring *txcmpl_ring)
{
	struct edma_txcmpl_desc *txcmpl;
	struct edma_tx_cmpl_stats *txcmpl_stats = &txcmpl_ring->tx_cmpl_stats;
	uint32_t prod_idx = 0;
	uint32_t cons_idx;
	uint32_t data;
	struct sk_buff *skb;
	struct sk_buff_head h;
	uint32_t txcmpl_errors;
	uint32_t avail, count;
	uint8_t cpu_id;
	uint32_t end_idx;
	uint32_t more_bit = 0;
	struct netdev_queue *nq;

	cons_idx = txcmpl_ring->cons_idx;

	if (likely(txcmpl_ring->avail_pkt >= work_to_do)) {
		avail = work_to_do;
	} else {

		/*
		 * Get TXCMPL ring producer index
		 */
		data = edma_reg_read(EDMA_REG_TXCMPL_PROD_IDX(txcmpl_ring->id));
		prod_idx = data & EDMA_TXCMPL_PROD_IDX_MASK;

		avail = EDMA_DESC_AVAIL_COUNT(prod_idx, cons_idx, EDMA_TX_RING_SIZE);
		txcmpl_ring->avail_pkt = avail;

		if (unlikely(!avail)) {
			edma_debug("No available descriptors are pending for %d txcmpl ring\n",
					txcmpl_ring->id);
			u64_stats_update_begin(&txcmpl_stats->syncp);
			++txcmpl_stats->no_pending_desc;
			u64_stats_update_end(&txcmpl_stats->syncp);
			return 0;
		}

		avail = min(avail, work_to_do);
	}

	count = avail;

	end_idx = (cons_idx + avail) & EDMA_TX_RING_SIZE_MASK;
	txcmpl = EDMA_TXCMPL_DESC(txcmpl_ring, cons_idx);

	if (end_idx > cons_idx) {
		dmac_inv_range_no_dsb((void *)txcmpl, txcmpl + avail);
	} else {
		dmac_inv_range_no_dsb(txcmpl_ring->desc, txcmpl_ring->desc + end_idx);
		dmac_inv_range_no_dsb((void *)txcmpl, txcmpl_ring->desc + EDMA_TX_RING_SIZE);
	}

	dsb(st);

	skb_queue_head_init(&h);

	/*
	 * TODO:
	 * Instead of freeing the skb, it might be better to save and use
	 * for Rxfill.
	 */
	while (likely(avail--)) {

		/*
		 * The last descriptor holds the SKB pointer for scattered frames.
		 * So skip the descriptors with more bit set.
		 */
		more_bit = EDMA_TXCMPL_MORE_BIT_GET(txcmpl);
		if (unlikely(more_bit)) {
			u64_stats_update_begin(&txcmpl_stats->syncp);
			++txcmpl_stats->desc_with_more_bit;
			u64_stats_update_end(&txcmpl_stats->syncp);
			cons_idx = ((cons_idx + 1) & EDMA_TX_RING_SIZE_MASK);
			txcmpl = EDMA_TXCMPL_DESC(txcmpl_ring, cons_idx);
			continue;
		}

		/*
		 * Find and free the skb for Tx completion
		 */
		skb = (struct sk_buff *)EDMA_TXCMPL_OPAQUE_GET(txcmpl);
		if (unlikely(!skb)) {
			if (net_ratelimit()) {
				edma_warn("Invalid skb: cons_idx:%u prod_idx:%u word2:%x word3:%x\n",
						cons_idx, prod_idx, txcmpl->word2, txcmpl->word3);
			}

			u64_stats_update_begin(&txcmpl_stats->syncp);
			++txcmpl_stats->invalid_buffer;
			u64_stats_update_end(&txcmpl_stats->syncp);
		} else {
			edma_debug("skb:%px, skb->len %d, skb->data_len %d, cons_idx:%d prod_idx:%d word2:0x%x word3:0x%x\n",
					skb, skb->len, skb->data_len, cons_idx, prod_idx, txcmpl->word2, txcmpl->word3);

			txcmpl_errors = EDMA_TXCOMP_RING_ERROR_GET(txcmpl->word3);
			if (unlikely(txcmpl_errors)) {
				/*
				 * TODO : Demux and add a debug print per error type.
				 */
				if (net_ratelimit()) {
					edma_err("Error 0x%0x observed in tx complete %d ring\n",
							txcmpl_errors, txcmpl_ring->id);
				}

				u64_stats_update_begin(&txcmpl_stats->syncp);
				++txcmpl_stats->errors;
				u64_stats_update_end(&txcmpl_stats->syncp);
			}

			/*
			 * Fast-recycle the SKB with a list, if skb is originally allocated
			 * from recycler and has been fast trasmitted
			 */
			if (likely(skb->fast_xmit) && likely(skb->is_from_recycler)) {
				dev_check_skb_fast_recyclable(skb);
				__skb_queue_head(&h, skb);
			} else {
				dev_kfree_skb(skb);
			}
		}

		cons_idx = ((cons_idx + 1) & EDMA_TX_RING_SIZE_MASK);
		txcmpl = EDMA_TXCMPL_DESC(txcmpl_ring, cons_idx);
	}

	if (likely(!skb_queue_empty(&h))) {
		dev_kfree_skb_list_fast(&h);
	}

	txcmpl_ring->cons_idx = cons_idx;
	txcmpl_ring->avail_pkt -= count;

	edma_debug("TXCMPL:%u count:%u prod_idx:%u cons_idx:%u\n",
			txcmpl_ring->id, count, prod_idx, cons_idx);
	edma_reg_write(EDMA_REG_TXCMPL_CONS_IDX(txcmpl_ring->id), cons_idx);

	/*
	 * If tx_requeue_stop disabled (tx_requeue_stop = 0)
	 * Fetch the tx queue of interface and check if it is stopped.
	 * if queue is stopped and interface is up, wake up this queue.
	 */
	if (likely(!dp_global_ctx.tx_requeue_stop)) {
		cpu_id = smp_processor_id();
		nq = netdev_get_tx_queue(txcmpl_ring->napi.dev, cpu_id);
		if (unlikely(netif_tx_queue_stopped(nq)) && netif_carrier_ok(txcmpl_ring->napi.dev)) {
			edma_debug("Waking queue number %d, for interface %s\n", cpu_id, txcmpl_ring->napi.dev->name);
			__netif_tx_lock(nq, cpu_id);
			netif_tx_wake_queue(nq);
			__netif_tx_unlock(nq);
		}
	}

	return count;
}

/*
 * edma_tx_napi_poll()
 *	EDMA TX NAPI handler
 */
int edma_tx_napi_poll(struct napi_struct *napi, int budget)
{
	struct edma_txcmpl_ring *txcmpl_ring = (struct edma_txcmpl_ring *)napi;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	uint32_t txcmpl_intr_status;
	int work_done = 0;
	uint32_t reg_data;

	do {
		work_done += edma_tx_complete(budget - work_done, txcmpl_ring);
		if (work_done >= budget) {
			return work_done;
		}

		reg_data = edma_reg_read(EDMA_REG_TX_INT_STAT(txcmpl_ring->id));
		txcmpl_intr_status = reg_data & EDMA_TXCMPL_RING_INT_STATUS_MASK;
	} while (txcmpl_intr_status);

	/*
	 * No more packets to process. Finish NAPI processing.
	 */
	napi_complete(napi);

	/*
	 * Set TXCMPL ring interrupt mask
	 */
	edma_reg_write(EDMA_REG_TX_INT_MASK(txcmpl_ring->id),
			egc->txcmpl_intr_mask);

	return work_done;
}

/*
 * edma_tx_handle_irq()
 *	Process TX IRQ and schedule napi
 */
irqreturn_t edma_tx_handle_irq(int irq, void *ctx)
{
	struct edma_txcmpl_ring *txcmpl_ring = (struct edma_txcmpl_ring *)ctx;

	edma_debug("irq: irq=%d txcmpl_ring_id=%u\n", irq, txcmpl_ring->id);
	if (likely(napi_schedule_prep(&txcmpl_ring->napi))) {
		/*
		 * Disable TxCmpl intr
		 */
		edma_reg_write(EDMA_REG_TX_INT_MASK(txcmpl_ring->id),
				EDMA_MASK_INT_DISABLE);
		__napi_schedule(&txcmpl_ring->napi);
	}

	return IRQ_HANDLED;
}

/*
 * edma_tx_desc_init()
 *	Initializes the Tx descriptor
 */
static inline void edma_tx_desc_init(struct edma_pri_txdesc *txdesc)
{
	memset(txdesc, 0, sizeof(struct edma_pri_txdesc));
}

/*
 * edma_tx_skb_nr_frags()
 *	Process Tx for skb with nr_frags
 */
static uint32_t edma_tx_skb_nr_frags(struct edma_txdesc_ring *txdesc_ring, struct edma_pri_txdesc **txdesc,
		struct sk_buff *skb, uint32_t *hw_next_to_use)
{
	uint8_t i = 0;
	uint32_t nr_frags = 0, buf_len = 0, num_descs = 0, start_idx = 0, end_idx = 0;
	struct edma_pri_txdesc *txd = *txdesc;

	/*
	 * Hold onto the index mapped to *txdesc.
	 * This will be the index previous to that of current *hw_next_to_use
	 */
	start_idx = (((*hw_next_to_use) + EDMA_TX_RING_SIZE_MASK) & EDMA_TX_RING_SIZE_MASK);

	/*
	 * Handle if the skb has nr_frags
	 */
	nr_frags = skb_shinfo(skb)->nr_frags;
	num_descs = nr_frags;
	i = 0;
	while (nr_frags--) {
		skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
		buf_len = skb_frag_size(frag);

		/*
		 * Zero size segment can lead EDMA HW to hang so, we don't want to process them.
		 * Zero size segment can happen during TSO operation if there is nothing but header
		 * in the primary segment.
		 */
		if (unlikely(buf_len == 0)) {
			num_descs--;
			i++;
			continue;
		}

		/*
		 * Setting the MORE bit on the previous Tx descriptor.
		 * Note: We will flush this descriptor as well later.
		 */
		EDMA_TXDESC_MORE_BIT_SET(txd, 1);
		EDMA_TXDESC_ENDIAN_SET(txd);

		txd = EDMA_TXDESC_PRI_DESC(txdesc_ring, *hw_next_to_use);
		edma_tx_desc_init(txd);
		EDMA_TXDESC_BUFFER_ADDR_SET(txd, (dma_addr_t)virt_to_phys(skb_frag_address(frag)));
		dmac_clean_range_no_dsb((void *)skb_frag_address(frag),
				(void *)(skb_frag_address(frag) + buf_len));

		EDMA_TXDESC_DATA_LEN_SET(txd, buf_len);

		*hw_next_to_use = ((*hw_next_to_use + 1) & EDMA_TX_RING_SIZE_MASK);
		i++;
	}

	EDMA_TXDESC_ENDIAN_SET(txd);

	/*
	 * This will be the index previous to that of current *hw_next_to_use
	 */
	end_idx = (((*hw_next_to_use) + EDMA_TX_RING_SIZE_MASK) & EDMA_TX_RING_SIZE_MASK);

	/*
	 * Need to flush from initial *txdesc to accomodate for MORE bit change.
	 * Need to flush all the descriptors but last-one that we filled as well.
	 */
	if (end_idx > start_idx) {
		dmac_clean_range_no_dsb((void *)(*txdesc),
				((*txdesc) + num_descs));
	} else {
		dmac_clean_range_no_dsb(txdesc_ring->pdesc,
				txdesc_ring->pdesc + end_idx);
		dmac_clean_range_no_dsb((void *)(*txdesc),
				txdesc_ring->pdesc + EDMA_TX_RING_SIZE);
	}

	*txdesc = txd;
	return num_descs;
}

/*
 * edma_tx_fill_vp_desc()
 *	Enable PPE processing with VP as source port
 */
static inline void edma_tx_fill_vp_desc(struct nss_dp_dev *dp_dev, struct edma_pri_txdesc *txd,
			struct sk_buff *skb, struct nss_dp_vp_tx_info *dptxi)
{
	if (unlikely(skb->ip_summed == CHECKSUM_PARTIAL)) {
		EDMA_TXDESC_ADV_OFFLOAD_SET(txd);
		EDMA_TXDESC_IP_CSUM_SET(txd);
		EDMA_TXDESC_L4_CSUM_SET(txd);
	}

	EDMA_TXDESC_SERVICE_CODE_SET(txd, dptxi->sc);

	/*
	 * Set fake mac bit needed for L3 interfaces
	 */
	EDMA_TXDESC_FAKE_MAC_HDR_SET(txd, dptxi->fake_mac);

	/*
	 * Set Source port information in the descriptor
	 */
	EDMA_SRC_INFO_SET(txd, dptxi->svp);
	EDMA_DST_INFO_SET(txd, 0);
}

/*
 * edma_tx_fill_pp_desc()
 *	Populate descriptor fields to bypass PPE processing and forward
 */
static inline void edma_tx_fill_pp_desc(struct nss_dp_dev *dp_dev, struct edma_pri_txdesc *txd,
					struct sk_buff *skb, struct edma_tx_stats *stats)
{
	/*
	 * Offload L3/L4 checksum computation
	 */
	if (likely(skb->ip_summed == CHECKSUM_PARTIAL)) {
		EDMA_TXDESC_ADV_OFFLOAD_SET(txd);
		EDMA_TXDESC_IP_CSUM_SET(txd);
		EDMA_TXDESC_L4_CSUM_SET(txd);
	}

	/*
	 * Check if the packet needs TSO
	 * This will be mostly true for SG packets.
	 */
	if (unlikely(skb_is_gso(skb))) {
		if ((skb_shinfo(skb)->gso_type == SKB_GSO_TCPV4) ||
				(skb_shinfo(skb)->gso_type == SKB_GSO_TCPV6)){
			uint32_t mss;
			mss = skb_shinfo(skb)->gso_size;

			/*
			 * If MSS<256, HW will do TSO using MSS=256,
			 * if MSS>10K, HW will do TSO using MSS=10K,
			 * else HW will report error 0x200000 in Tx Cmpl
			 */
			if (mss < EDMA_TX_TSO_MSS_MIN)
				mss = EDMA_TX_TSO_MSS_MIN;
			else if (mss > EDMA_TX_TSO_MSS_MAX)
				mss = EDMA_TX_TSO_MSS_MAX;

			EDMA_TXDESC_TSO_ENABLE_SET(txd, 1);
			EDMA_TXDESC_MSS_SET(txd, mss);

			/*
			 * Update tso stats
			 */
			u64_stats_update_begin(&stats->syncp);
			stats->tx_tso_pkts++;
			u64_stats_update_end(&stats->syncp);
		}
	}

	/*
	 * Set destination information in the descriptor
	 */
	EDMA_TXDESC_SERVICE_CODE_SET(txd, PPE_DRV_SC_BYPASS_ALL);
	EDMA_DST_INFO_SET(txd, dp_dev->macid);

	/*
	 * Set the src info as destination dev in case if
	 * port mirroring is enabled - to receive the packet back in
	 * DP with valid source port.
	 */
#if defined NSS_DP_PORT_MIRROR_EN
	EDMA_SRC_INFO_SET(txd, dp_dev->macid);
#endif

	EDMA_TXDESC_INT_PRI_SET(txd, skb_get_int_pri(skb));
}

/*
 * edma_tx_skb_first_desc()
 *	Process the Tx for the first descriptor required for skb
 */
static struct edma_pri_txdesc *edma_tx_skb_first_desc(struct nss_dp_dev *dp_dev, struct edma_txdesc_ring *txdesc_ring,
					struct nss_dp_vp_tx_info *dptxi, struct sk_buff *skb, uint32_t *hw_next_to_use,
					struct edma_tx_stats *stats)
{
	uint32_t buf_len = 0;
	struct edma_pri_txdesc *txd = NULL;

	/*
	 * Get the packet length
	 */
	buf_len = skb_headlen(skb);

	txd = EDMA_TXDESC_PRI_DESC(txdesc_ring, *hw_next_to_use);

	edma_tx_desc_init(txd);

	/*
	 * Set the data pointer as the buffer address in the descriptor.
	 */
	EDMA_TXDESC_BUFFER_ADDR_SET(txd, (dma_addr_t)virt_to_phys(skb->data));
	dmac_clean_range_no_dsb((void *)skb->data, (void *)(skb->data + buf_len));

	if (dptxi) {
		edma_tx_fill_vp_desc(dp_dev, txd, skb, dptxi);
	} else {
		edma_tx_fill_pp_desc(dp_dev, txd, skb, stats);
	}

	/*
	 * Set packet length in the descriptor
	 */
	EDMA_TXDESC_DATA_LEN_SET(txd, buf_len);

	*hw_next_to_use = (*hw_next_to_use + 1) & EDMA_TX_RING_SIZE_MASK;

	return txd;
}

/*
 * edma_tx_skb_sg_fill_desc()
 *	API to fill SG skb into Tx descriptor. Handles both nr_frags and fraglist cases.
 */
static uint32_t edma_tx_skb_sg_fill_desc(struct nss_dp_dev *dp_dev, struct edma_txdesc_ring *txdesc_ring,
		struct edma_pri_txdesc **txdesc, struct sk_buff *skb, uint32_t *hw_next_to_use,
		struct edma_tx_stats *stats)
{
	uint32_t buf_len = 0, num_descs = 0;
	struct sk_buff *iter_skb = NULL;
	uint32_t num_sg_frag_list = 0;
	struct edma_pri_txdesc *txd = *txdesc;

	/*
	 * Head skb processed already
	 */
	num_descs++;

	/*
	 * Process skb with nr_frags
	 */
	if (unlikely(skb_shinfo(skb)->nr_frags)) {
		num_descs += edma_tx_skb_nr_frags(txdesc_ring, &txd, skb, hw_next_to_use);
		u64_stats_update_begin(&stats->syncp);
		stats->tx_nr_frag_pkts++;
		u64_stats_update_end(&stats->syncp);
	}

	if (unlikely(skb_has_frag_list(skb))) {
		struct edma_pri_txdesc *start_desc = NULL;
		uint32_t start_idx = 0, end_idx = 0;

		/*
		 * Hold onto the index mapped to txd.
		 * This will be the index previous to that of current *hw_next_to_use
		 */
		start_idx = (((*hw_next_to_use) + EDMA_TX_RING_SIZE_MASK) & EDMA_TX_RING_SIZE_MASK);
		start_desc = txd;

		/*
		 * Walk through all fraglist skbs
		 */
		skb_walk_frags(skb, iter_skb) {
			uint32_t num_nr_frag = 0;

			/*
			 * This case could happen during the packet decapsulation. All header content might be removed.
			 */
			buf_len = skb_headlen(iter_skb);
			if (unlikely(buf_len == 0)) {
				goto skip_primary;
			}

			/*
			 * We make sure to flush this descriptor later
			 */
			EDMA_TXDESC_MORE_BIT_SET(txd, 1);
			EDMA_TXDESC_ENDIAN_SET(txd);

			txd = EDMA_TXDESC_PRI_DESC(txdesc_ring, *hw_next_to_use);
			edma_tx_desc_init(txd);
			EDMA_TXDESC_BUFFER_ADDR_SET(txd, (dma_addr_t)virt_to_phys(iter_skb->data));
			dmac_clean_range_no_dsb((void *)iter_skb->data,
					(void *)(iter_skb->data + buf_len));

			EDMA_TXDESC_DATA_LEN_SET(txd, buf_len);

			*hw_next_to_use = (*hw_next_to_use + 1) & EDMA_TX_RING_SIZE_MASK;
			num_descs += 1;
			num_sg_frag_list += 1;

			/*
			 * skb fraglist skb can have nr_frags
			 */
skip_primary:
			if (unlikely(skb_shinfo(iter_skb)->nr_frags)) {
				num_nr_frag = edma_tx_skb_nr_frags(txdesc_ring, &txd, iter_skb, hw_next_to_use);
				num_descs += num_nr_frag;
				num_sg_frag_list += num_nr_frag;

				/*
				 * Update fraglist with nr_frag stats
				 */
				u64_stats_update_begin(&stats->syncp);
				stats->tx_fraglist_with_nr_frags_pkts++;
				u64_stats_update_end(&stats->syncp);
			}
		}

		EDMA_TXDESC_ENDIAN_SET(txd);

		/*
		 * This will be the index previous to
		 * that of current *hw_next_to_use
		 */
		end_idx = (((*hw_next_to_use) + EDMA_TX_RING_SIZE_MASK) &
					EDMA_TX_RING_SIZE_MASK);

		/*
		 * Need to flush from initial txd to accomodate for MORE bit change.
		 * Need to flush all the descriptors but last-one that we filled as well.
		 * This may result double flush if fraglist iter_skb has nr_frags which is rare.
		 */
		if (end_idx > start_idx) {
			dmac_clean_range_no_dsb((void *)(start_desc),
					((start_desc) + num_sg_frag_list));
		} else {
			dmac_clean_range_no_dsb(txdesc_ring->pdesc,
					txdesc_ring->pdesc + end_idx);
			dmac_clean_range_no_dsb((void *)(start_desc),
					txdesc_ring->pdesc + EDMA_TX_RING_SIZE);
		}

		/*
		 * Update frag_list stats
		 */
		u64_stats_update_begin(&stats->syncp);
		stats->tx_fraglist_pkts++;
		u64_stats_update_end(&stats->syncp);
	}

	edma_debug("skb:%px num_descs_filled: %u, nr_frags %u frag_list fragments %u\n",
			skb, num_descs, skb_shinfo(skb)->nr_frags, num_sg_frag_list);

	*txdesc = txd;
	return num_descs;
}

/*
 * edma_tx_avail_desc()
 *	Get available Tx descriptors
 */
static uint32_t edma_tx_avail_desc(struct edma_txdesc_ring *txdesc_ring, uint32_t hw_next_to_use)
{
	uint32_t data = 0, avail = 0, hw_next_to_clean = 0;

	data = edma_reg_read(EDMA_REG_TXDESC_CONS_IDX(txdesc_ring->id));
	hw_next_to_clean = data & EDMA_TXDESC_CONS_IDX_MASK;

	avail = EDMA_DESC_AVAIL_COUNT(hw_next_to_clean - 1, hw_next_to_use, EDMA_TX_RING_SIZE);

	return avail;
}

/*
 * edma_tx_phy_tstamp_buf()
 *	Send skb for PHY timestamping
 */
static inline void edma_tx_phy_tstamp_buf(struct net_device *ndev, struct sk_buff *skb)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	/*
	 * Function drv->txtstamp will create a clone of skb if necessary,
	 * the PTP_CLASS_ value 0 is passed to phy driver, which will be
	 * set to the correct PTP class value by calling ptp_classify_raw
	 * in the drv->txtstamp function.
	 */
	if (ndev && ndev->phydev && ndev->phydev->drv && ndev->phydev->drv->txtstamp) {
		ndev->phydev->drv->txtstamp(ndev->phydev, skb, 0);
	}
#else
	if (phy_has_txtstamp(ndev->phydev)) {
		phy_txtstamp(ndev->phydev, skb, 0);
	}
#endif
}

/*
 * edma_tx_ring_xmit()
 *	API to transmit a packet.
 */
enum edma_tx edma_tx_ring_xmit(struct net_device *netdev, struct nss_dp_vp_tx_info *dptxi, struct sk_buff *skb,
				struct edma_txdesc_ring *txdesc_ring,
				struct edma_tx_stats *stats)
{
	struct nss_dp_dev *dp_dev = netdev_priv(netdev);
	struct edma_tx_desc_stats *txdesc_stats = &txdesc_ring->tx_desc_stats;
	uint32_t hw_next_to_use = 0, cons_idx = 0, work_to_do = 0;
	uint32_t num_tx_desc_needed = 0, num_desc_filled = 0;
	struct edma_pri_txdesc *txdesc = NULL;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;

	hw_next_to_use = txdesc_ring->prod_idx;

	if (unlikely(!(txdesc_ring->avail_desc)))  {
		txdesc_ring->avail_desc = edma_tx_avail_desc(txdesc_ring, hw_next_to_use);
		if (unlikely(!txdesc_ring->avail_desc)) {
			edma_debug("No available descriptors are present at %d ring\n",
					txdesc_ring->id);

			u64_stats_update_begin(&txdesc_stats->syncp);
			++txdesc_stats->no_desc_avail;
			u64_stats_update_end(&txdesc_stats->syncp);
			return EDMA_TX_FAIL_NO_DESC;
		}

		if (unlikely(egc->enable_ring_util_stats)) {
			cons_idx = edma_reg_read(EDMA_REG_TXDESC_CONS_IDX(txdesc_ring->id)) & EDMA_TXDESC_CONS_IDX_MASK;
			work_to_do = EDMA_DESC_AVAIL_COUNT(txdesc_ring->prod_idx, cons_idx, txdesc_ring->count);
			edma_update_ring_stats(work_to_do, txdesc_ring->count,
					       &txdesc_stats->ring_stats);
		}
	}

	/*
	 * Deliver the ptp packet to phy driver for TX timestamping
	 */
	if (unlikely(skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP)) {
		edma_tx_phy_tstamp_buf(netdev, skb);
	}

	/*
	 * Process head skb for linear skb
	 * Process head skb + nr_frags + fraglist for non linear skb
	 */
	if (likely(!skb_is_nonlinear(skb))) {
		txdesc = edma_tx_skb_first_desc(dp_dev, txdesc_ring, dptxi, skb, &hw_next_to_use, stats);
		EDMA_TXDESC_ENDIAN_SET(txdesc);
		num_desc_filled++;

		/*
		 * We set fast_recycled flag if packet has taken the
		 * SFE fast transmit path, so that any Rx DMA driver
		 * that allocates this packet later can avoid
		 * an invalidate operation
		 */
		if (likely(skb->fast_xmit) && likely(skb->is_from_recycler)) {
			skb->fast_recycled = 1;
		}
	} else {
		num_tx_desc_needed = edma_tx_num_descs_for_sg(skb);

		/*
		 * HW does not support TSO for packets with more than 32 segments.
		 * HW hangs up if it sees more than 32 segments.
		 * Kernel Perform GSO for such packets with netdev gso_max_segs set to 32.
		 */
		if (unlikely(num_tx_desc_needed > EDMA_TX_TSO_SEG_MAX)) {
			edma_debug("Number of segments %u more than %u for %d ring\n",
					num_tx_desc_needed, EDMA_TX_TSO_SEG_MAX, txdesc_ring->id);
			u64_stats_update_begin(&txdesc_stats->syncp);
			++txdesc_stats->tso_max_seg_exceed;
			u64_stats_update_end(&txdesc_stats->syncp);

			u64_stats_update_begin(&stats->syncp);
			stats->tx_tso_drop_pkts++;
			u64_stats_update_end(&stats->syncp);

			return EDMA_TX_FAIL;
		}

		if (unlikely(num_tx_desc_needed > txdesc_ring->avail_desc)) {
			txdesc_ring->avail_desc = edma_tx_avail_desc(txdesc_ring, hw_next_to_use);
			if (num_tx_desc_needed > txdesc_ring->avail_desc) {
				u64_stats_update_begin(&txdesc_stats->syncp);
				++txdesc_stats->no_desc_avail;
				u64_stats_update_end(&txdesc_stats->syncp);
				edma_debug("Not enough available descriptors are present at %d ring for SG packet. Needed %d, currently available %d\n",
					txdesc_ring->id, num_tx_desc_needed, txdesc_ring->avail_desc);
				return EDMA_TX_FAIL_NO_DESC;
			}
		}

		txdesc = edma_tx_skb_first_desc(dp_dev, txdesc_ring, dptxi, skb, &hw_next_to_use, stats);
		num_desc_filled = edma_tx_skb_sg_fill_desc(dp_dev, txdesc_ring, &txdesc, skb, &hw_next_to_use, stats);
	}

	/*
	 * Set the skb pointer to the descriptor's opaque field/s
	 * on the last descriptor of the packet/SG packet.
	 */
	EDMA_TXDESC_OPAQUE_SET(txdesc, skb);

	/*
	 * Flush the last descriptor.
	 */
	dmac_clean_range_no_dsb(txdesc, txdesc + 1);

	/*
	 * Update producer index
	 */
	txdesc_ring->prod_idx = hw_next_to_use & EDMA_TXDESC_PROD_IDX_MASK;
	txdesc_ring->avail_desc -= num_desc_filled;

	edma_debug("%s: skb:%px tx_ring:%u proto:0x%x skb->len:%d\n"
			" port:%u prod_idx:%u ip_summed:0x%x\n",
			netdev->name, skb, txdesc_ring->id, ntohs(skb->protocol),
			skb->len, dp_dev->macid, hw_next_to_use, skb->ip_summed);

	/*
	 * Make sure the information written to the descriptors
	 * is updated before writing to the hardware.
	 */
	dsb(st);

	edma_reg_write(EDMA_REG_TXDESC_PROD_IDX(txdesc_ring->id),
			txdesc_ring->prod_idx);

	u64_stats_update_begin(&stats->syncp);
	stats->tx_pkts++;
	stats->tx_bytes += skb->len;
	u64_stats_update_end(&stats->syncp);

	return EDMA_TX_OK;
}
