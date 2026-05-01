// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

/* Provide APIs to alloc Tx Buffers, fill the Tx descriptors and transmit
 * Scatter Gather and linear packets, Tx complete to free the skb after transmit.
 */

#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <net/gso.h>
#include <linux/regmap.h>

#include "edma.h"
#include "edma_cfg_tx.h"
#include "edma_port.h"
#include "ppe.h"
#include "ppe_regs.h"

static u32 edma_tx_num_descs_for_sg(struct sk_buff *skb)
{
	u32 nr_frags_first = 0, num_tx_desc_needed = 0;

	/* Check if we have enough Tx descriptors for SG. */
	if (unlikely(skb_shinfo(skb)->nr_frags)) {
		nr_frags_first = skb_shinfo(skb)->nr_frags;
		WARN_ON_ONCE(nr_frags_first > MAX_SKB_FRAGS);
		num_tx_desc_needed += nr_frags_first;
	}

	/* Walk through fraglist skbs making a note of nr_frags
	 * One Tx desc for fraglist skb. Fraglist skb may have
	 * further nr_frags.
	 */
	if (unlikely(skb_has_frag_list(skb))) {
		struct sk_buff *iter_skb;

		skb_walk_frags(skb, iter_skb) {
			u32 nr_frags = skb_shinfo(iter_skb)->nr_frags;

			WARN_ON_ONCE(nr_frags > MAX_SKB_FRAGS);
			num_tx_desc_needed += (1 + nr_frags);
		}
	}

	return (num_tx_desc_needed + 1);
}

/**
 * edma_tx_gso_segment - Tx GSO.
 * @skb: Socket Buffer.
 * @netdev: Netdevice.
 * @segs: SKB segments from GSO.
 *
 * Format skbs into GSOs.
 *
 * Return 1 on success, error code on failure.
 */
enum edma_tx_gso_status edma_tx_gso_segment(struct sk_buff *skb,
					    struct net_device *netdev, struct sk_buff **segs)
{
	u32 num_tx_desc_needed;

	/* Check is skb is non-linear to proceed. */
	if (likely(!skb_is_nonlinear(skb)))
		return EDMA_TX_GSO_NOT_NEEDED;

	/* Check if TSO is enabled. If so, return as skb doesn't
	 * need to be segmented by linux.
	 */
	if (netdev->features & (NETIF_F_TSO | NETIF_F_TSO6)) {
		num_tx_desc_needed = edma_tx_num_descs_for_sg(skb);
		if (likely(num_tx_desc_needed <= EDMA_TX_TSO_SEG_MAX))
			return EDMA_TX_GSO_NOT_NEEDED;
	}

	/* GSO segmentation of the skb into multiple segments. */
	*segs = skb_gso_segment(skb, netdev->features
		& ~(NETIF_F_TSO | NETIF_F_TSO6));

	/* Check for error in GSO segmentation. */
	if (IS_ERR_OR_NULL(*segs)) {
		netdev_info(netdev, "Tx gso fail\n");
		return EDMA_TX_GSO_FAIL;
	}

	return EDMA_TX_GSO_SUCCEED;
}

/**
 * edma_tx_complete - Reap Tx completion descriptors.
 * @work_to_do: Work to do.
 * @txcmpl_ring: Tx Completion ring.
 *
 * Reap Tx completion descriptors of the transmitted
 * packets and free the corresponding SKBs.
 *
 * Return the number descriptors for which Tx complete is done.
 */
u32 edma_tx_complete(u32 work_to_do, struct edma_txcmpl_ring *txcmpl_ring)
{
	struct edma_txcmpl_stats *txcmpl_stats = &txcmpl_ring->txcmpl_stats;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 cons_idx, end_idx, data, cpu_id;
	struct device *dev = ppe_dev->dev;
	u32 avail, count, txcmpl_errors;
	struct edma_txcmpl_desc *txcmpl;
	u32 prod_idx = 0, more_bit = 0;
	struct netdev_queue *nq;
	struct sk_buff *skb;
	u32 reg;

	cons_idx = txcmpl_ring->cons_idx;

	if (likely(txcmpl_ring->avail_pkt >= work_to_do)) {
		avail = work_to_do;
	} else {
		/* Get TXCMPL ring producer index. */
		reg = EDMA_BASE_OFFSET + EDMA_REG_TXCMPL_PROD_IDX(txcmpl_ring->id);
		regmap_read(regmap, reg, &data);
		prod_idx = data & EDMA_TXCMPL_PROD_IDX_MASK;

		avail = EDMA_DESC_AVAIL_COUNT(prod_idx, cons_idx, EDMA_TX_RING_SIZE);
		txcmpl_ring->avail_pkt = avail;

		if (unlikely(!avail)) {
			dev_dbg(dev, "No available descriptors are pending for %d txcmpl ring\n",
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

	/* Instead of freeing the skb, it might be better to save and use
	 * for Rxfill.
	 */
	while (likely(avail--)) {
		/* The last descriptor holds the SKB pointer for scattered frames.
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

		/* Find and free the skb for Tx completion. */
		skb = (struct sk_buff *)EDMA_TXCMPL_OPAQUE_GET(txcmpl);
		if (unlikely(!skb)) {
			if (net_ratelimit())
				dev_warn(dev, "Invalid cons_idx:%u prod_idx:%u word2:%x word3:%x\n",
					 cons_idx, prod_idx, txcmpl->word2, txcmpl->word3);

			u64_stats_update_begin(&txcmpl_stats->syncp);
			++txcmpl_stats->invalid_buffer;
			u64_stats_update_end(&txcmpl_stats->syncp);
		} else {
			dev_dbg(dev, "TXCMPL: skb:%p, skb->len %d, skb->data_len %d, cons_idx:%d prod_idx:%d word2:0x%x word3:0x%x\n",
				skb, skb->len, skb->data_len, cons_idx, prod_idx,
				txcmpl->word2, txcmpl->word3);

			txcmpl_errors = EDMA_TXCOMP_RING_ERROR_GET(txcmpl->word3);
			if (unlikely(txcmpl_errors)) {
				if (net_ratelimit())
					dev_err(dev, "Error 0x%0x observed in tx complete %d ring\n",
						txcmpl_errors, txcmpl_ring->id);

				u64_stats_update_begin(&txcmpl_stats->syncp);
				++txcmpl_stats->errors;
				u64_stats_update_end(&txcmpl_stats->syncp);
			}

			/* Retrieve pool id for unmapping.
			 * 0 for linear skb and (pool id - 1) represents nr_frag index.
			 */
			if (!EDMA_TXCOMP_POOL_ID_GET(txcmpl)) {
				dma_unmap_single(dev, virt_to_phys(skb->data),
						 skb->len, DMA_TO_DEVICE);
			} else {
				u8 frag_index = (EDMA_TXCOMP_POOL_ID_GET(txcmpl) - 1);
				skb_frag_t *frag = &skb_shinfo(skb)->frags[frag_index];

				dma_unmap_page(dev, virt_to_phys(frag),
					       PAGE_SIZE, DMA_TO_DEVICE);
			}

			dev_kfree_skb(skb);
		}

		cons_idx = ((cons_idx + 1) & EDMA_TX_RING_SIZE_MASK);
		txcmpl = EDMA_TXCMPL_DESC(txcmpl_ring, cons_idx);
	}

	txcmpl_ring->cons_idx = cons_idx;
	txcmpl_ring->avail_pkt -= count;

	dev_dbg(dev, "TXCMPL:%u count:%u prod_idx:%u cons_idx:%u\n",
		txcmpl_ring->id, count, prod_idx, cons_idx);
	reg = EDMA_BASE_OFFSET + EDMA_REG_TXCMPL_CONS_IDX(txcmpl_ring->id);
	regmap_write(regmap, reg, cons_idx);

	/* If tx_requeue_stop disabled (tx_requeue_stop = 0)
	 * Fetch the tx queue of interface and check if it is stopped.
	 * if queue is stopped and interface is up, wake up this queue.
	 */
	if (unlikely(!edma_ctx->tx_requeue_stop)) {
		cpu_id = smp_processor_id();
		nq = netdev_get_tx_queue(txcmpl_ring->napi.dev, cpu_id);
		if (unlikely(netif_tx_queue_stopped(nq)) &&
		    netif_carrier_ok(txcmpl_ring->napi.dev)) {
			dev_dbg(dev, "Waking queue number %d, for interface %s\n",
				cpu_id, txcmpl_ring->napi.dev->name);
			__netif_tx_lock(nq, cpu_id);
			netif_tx_wake_queue(nq);
			__netif_tx_unlock(nq);
		}
	}

	return count;
}

/**
 * edma_tx_napi_poll - EDMA TX NAPI handler.
 * @napi: NAPI structure.
 * @budget: Tx NAPI Budget.
 *
 * EDMA TX NAPI handler.
 */
int edma_tx_napi_poll(struct napi_struct *napi, int budget)
{
	struct edma_txcmpl_ring *txcmpl_ring = (struct edma_txcmpl_ring *)napi;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 txcmpl_intr_status;
	int work_done = 0;
	u32 data, reg;

	do {
		work_done += edma_tx_complete(budget - work_done, txcmpl_ring);
		if (work_done >= budget)
			return work_done;

		reg = EDMA_BASE_OFFSET + EDMA_REG_TX_INT_STAT(txcmpl_ring->id);
		regmap_read(regmap, reg, &data);
		txcmpl_intr_status = data & EDMA_TXCMPL_RING_INT_STATUS_MASK;
	} while (txcmpl_intr_status);

	/* No more packets to process. Finish NAPI processing. */
	napi_complete(napi);

	/* Set TXCMPL ring interrupt mask. */
	reg = EDMA_BASE_OFFSET + EDMA_REG_TX_INT_MASK(txcmpl_ring->id);
	regmap_write(regmap, reg, edma_ctx->intr_info.intr_mask_txcmpl);

	return work_done;
}

/**
 * edma_tx_handle_irq - Tx IRQ Handler.
 * @irq: Interrupt request.
 * @ctx: Context.
 *
 * Process TX IRQ and schedule NAPI.
 *
 * Return IRQ handler code.
 */
irqreturn_t edma_tx_handle_irq(int irq, void *ctx)
{
	struct edma_txcmpl_ring *txcmpl_ring = (struct edma_txcmpl_ring *)ctx;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 reg;

	pr_debug("irq: irq=%d txcmpl_ring_id=%u\n", irq, txcmpl_ring->id);
	if (likely(napi_schedule_prep(&txcmpl_ring->napi))) {
		/* Disable TxCmpl intr. */
		reg = EDMA_BASE_OFFSET + EDMA_REG_TX_INT_MASK(txcmpl_ring->id);
		regmap_write(regmap, reg, EDMA_MASK_INT_DISABLE);
		__napi_schedule(&txcmpl_ring->napi);
	}

	return IRQ_HANDLED;
}

static void edma_tx_dma_unmap_frags(struct sk_buff *skb, u32 nr_frags)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct device *dev = ppe_dev->dev;
	u32 buf_len = 0;
	u8 i = 0;

	for (i = 0; i < skb_shinfo(skb)->nr_frags - nr_frags; i++) {
		skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

		/* DMA mapping was not done for zero size segments. */
		buf_len = skb_frag_size(frag);
		if (unlikely(buf_len == 0))
			continue;

		dma_unmap_page(dev, virt_to_phys(frag), PAGE_SIZE,
			       DMA_TO_DEVICE);
	}
}

static u32 edma_tx_skb_nr_frags(struct edma_txdesc_ring *txdesc_ring,
				struct edma_txdesc_pri **txdesc, struct sk_buff *skb,
				u32 *hw_next_to_use, u32 *invalid_frag)
{
	u32 nr_frags = 0, buf_len = 0, num_descs = 0, start_idx = 0, end_idx = 0;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	u32 start_hw_next_to_use = *hw_next_to_use;
	struct edma_txdesc_pri *txd = *txdesc;
	struct device *dev = ppe_dev->dev;
	u8 i = 0;

	/* Hold onto the index mapped to *txdesc.
	 * This will be the index previous to that of current *hw_next_to_use.
	 */
	start_idx = (((*hw_next_to_use) + EDMA_TX_RING_SIZE_MASK)
		& EDMA_TX_RING_SIZE_MASK);

	/* Handle if the skb has nr_frags. */
	nr_frags = skb_shinfo(skb)->nr_frags;
	num_descs = nr_frags;
	i = 0;

	while (nr_frags--) {
		skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
		dma_addr_t buff_addr;

		buf_len = skb_frag_size(frag);

		/* Zero size segment can lead EDMA HW to hang so, we don't want to
		 * process them. Zero size segment can happen during TSO operation
		 * if there is nothing but header in the primary segment.
		 */
		if (unlikely(buf_len == 0)) {
			num_descs--;
			i++;
			continue;
		}

		/* Setting the MORE bit on the previous Tx descriptor.
		 * Note: We will flush this descriptor as well later.
		 */
		EDMA_TXDESC_MORE_BIT_SET(txd, 1);
		EDMA_TXDESC_ENDIAN_SET(txd);

		txd = EDMA_TXDESC_PRI_DESC(txdesc_ring, *hw_next_to_use);
		memset(txd, 0, sizeof(struct edma_txdesc_pri));
		buff_addr = skb_frag_dma_map(dev, frag, 0, buf_len,
					     DMA_TO_DEVICE);
		if (dma_mapping_error(dev, buff_addr)) {
			dev_dbg(dev, "Unable to dma first descriptor for nr_frags tx\n");
			*hw_next_to_use = start_hw_next_to_use;
			*invalid_frag = nr_frags;
			return 0;
		}

		EDMA_TXDESC_BUFFER_ADDR_SET(txd, buff_addr);
		EDMA_TXDESC_DATA_LEN_SET(txd, buf_len);
		EDMA_TXDESC_POOL_ID_SET(txd, (i + 1));

		*hw_next_to_use = ((*hw_next_to_use + 1) & EDMA_TX_RING_SIZE_MASK);
		i++;
	}

	EDMA_TXDESC_ENDIAN_SET(txd);

	/* This will be the index previous to that of current *hw_next_to_use. */
	end_idx = (((*hw_next_to_use) + EDMA_TX_RING_SIZE_MASK) & EDMA_TX_RING_SIZE_MASK);

	*txdesc = txd;

	return num_descs;
}

static void edma_tx_fill_pp_desc(struct edma_port_priv *port_priv,
				 struct edma_txdesc_pri *txd, struct sk_buff *skb,
	struct edma_port_tx_stats *stats)
{
	struct ppe_port *port = port_priv->ppe_port;
	int port_id = port->port_id;

	/* Offload L3/L4 checksum computation. */
	if (likely(skb->ip_summed == CHECKSUM_PARTIAL)) {
		EDMA_TXDESC_ADV_OFFLOAD_SET(txd);
		EDMA_TXDESC_IP_CSUM_SET(txd);
		EDMA_TXDESC_L4_CSUM_SET(txd);
	}

	/* Check if the packet needs TSO
	 * This will be mostly true for SG packets.
	 */
	if (unlikely(skb_is_gso(skb))) {
		if ((skb_shinfo(skb)->gso_type == SKB_GSO_TCPV4) ||
		    (skb_shinfo(skb)->gso_type == SKB_GSO_TCPV6)) {
			u32 mss = skb_shinfo(skb)->gso_size;

			/* If MSS<256, HW will do TSO using MSS=256,
			 * if MSS>10K, HW will do TSO using MSS=10K,
			 * else HW will report error 0x200000 in Tx Cmpl.
			 */
			if (mss < EDMA_TX_TSO_MSS_MIN)
				mss = EDMA_TX_TSO_MSS_MIN;
			else if (mss > EDMA_TX_TSO_MSS_MAX)
				mss = EDMA_TX_TSO_MSS_MAX;

			EDMA_TXDESC_TSO_ENABLE_SET(txd, 1);
			EDMA_TXDESC_MSS_SET(txd, mss);

			/* Update tso stats. */
			u64_stats_update_begin(&stats->syncp);
			stats->tx_tso_pkts++;
			u64_stats_update_end(&stats->syncp);
		}
	}

	/* Set destination information in the descriptor. */
	EDMA_TXDESC_SERVICE_CODE_SET(txd, PPE_EDMA_SC_BYPASS_ID);
	EDMA_DST_INFO_SET(txd, port_id);
}

static struct edma_txdesc_pri *edma_tx_skb_first_desc(struct edma_port_priv *port_priv,
						      struct edma_txdesc_ring *txdesc_ring,
						      struct sk_buff *skb, u32 *hw_next_to_use,
						      struct edma_port_tx_stats *stats)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct edma_txdesc_pri *txd = NULL;
	struct device *dev = ppe_dev->dev;
	dma_addr_t buff_addr;
	u32 buf_len = 0;

	/* Get the packet length. */
	buf_len = skb_headlen(skb);
	txd = EDMA_TXDESC_PRI_DESC(txdesc_ring, *hw_next_to_use);
	memset(txd, 0, sizeof(struct edma_txdesc_pri));

	/* Set the data pointer as the buffer address in the descriptor. */
	buff_addr = dma_map_single(dev, skb->data, buf_len, DMA_TO_DEVICE);
	if (dma_mapping_error(dev, buff_addr)) {
		dev_dbg(dev, "Unable to dma first descriptor for tx\n");
		return NULL;
	}

	EDMA_TXDESC_BUFFER_ADDR_SET(txd, buff_addr);
	EDMA_TXDESC_POOL_ID_SET(txd, 0);
	edma_tx_fill_pp_desc(port_priv, txd, skb, stats);

	/* Set packet length in the descriptor. */
	EDMA_TXDESC_DATA_LEN_SET(txd, buf_len);
	*hw_next_to_use = (*hw_next_to_use + 1) & EDMA_TX_RING_SIZE_MASK;

	return txd;
}

static void edma_tx_handle_dma_err(struct sk_buff *skb, u32 num_sg_frag_list)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct device *dev = ppe_dev->dev;
	struct sk_buff *iter_skb = NULL;
	u32 cnt_sg_frag_list = 0;

	/* Walk through all fraglist skbs. */
	skb_walk_frags(skb, iter_skb) {
		if (skb_headlen(iter_skb)) {
			dma_unmap_single(dev, virt_to_phys(iter_skb->data),
					 skb_headlen(iter_skb), DMA_TO_DEVICE);
			cnt_sg_frag_list += 1;
		}

		if (cnt_sg_frag_list == num_sg_frag_list)
			return;

		/* skb fraglist skb had nr_frags, unmap that memory. */
		u32 nr_frags = skb_shinfo(iter_skb)->nr_frags;

		if (nr_frags == 0)
			continue;

		for (int i = 0; i < nr_frags; i++) {
			skb_frag_t *frag = &skb_shinfo(iter_skb)->frags[i];

			/* DMA mapping was not done for zero size segments. */
			if (unlikely(skb_frag_size(frag) == 0))
				continue;

			dma_unmap_page(dev, virt_to_phys(frag),
				       PAGE_SIZE, DMA_TO_DEVICE);
			cnt_sg_frag_list += 1;
			if (cnt_sg_frag_list == num_sg_frag_list)
				return;
		}
	}
}

static u32 edma_tx_skb_sg_fill_desc(struct edma_txdesc_ring *txdesc_ring,
				    struct edma_txdesc_pri **txdesc,
				    struct sk_buff *skb, u32 *hw_next_to_use,
				    struct edma_port_tx_stats *stats)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	u32 start_hw_next_to_use = 0, invalid_frag = 0;
	struct edma_txdesc_pri *txd = *txdesc;
	struct device *dev = ppe_dev->dev;
	struct sk_buff *iter_skb = NULL;
	u32 buf_len = 0, num_descs = 0;
	u32 num_sg_frag_list = 0;

	/* Head skb processed already. */
	num_descs++;

	if (unlikely(skb_has_frag_list(skb))) {
		struct edma_txdesc_pri *start_desc = NULL;
		u32 start_idx = 0, end_idx = 0;

		/* Hold onto the index mapped to txd.
		 * This will be the index previous to that of current *hw_next_to_use.
		 */
		start_idx = (((*hw_next_to_use) + EDMA_TX_RING_SIZE_MASK)
			     & EDMA_TX_RING_SIZE_MASK);
		start_desc = txd;
		start_hw_next_to_use = *hw_next_to_use;

		/* Walk through all fraglist skbs. */
		skb_walk_frags(skb, iter_skb) {
			dma_addr_t buff_addr;
			u32 num_nr_frag = 0;

			/* This case could happen during the packet decapsulation.
			 * All header content might be removed.
			 */
			buf_len = skb_headlen(iter_skb);
			if (unlikely(buf_len == 0))
				goto skip_primary;

			/* We make sure to flush this descriptor later. */
			EDMA_TXDESC_MORE_BIT_SET(txd, 1);
			EDMA_TXDESC_ENDIAN_SET(txd);

			txd = EDMA_TXDESC_PRI_DESC(txdesc_ring, *hw_next_to_use);
			memset(txd, 0, sizeof(struct edma_txdesc_pri));
			buff_addr = dma_map_single(dev, iter_skb->data,
						   buf_len, DMA_TO_DEVICE);
			if (dma_mapping_error(dev, buff_addr)) {
				dev_dbg(dev, "Unable to dma for fraglist\n");
				goto dma_err;
			}

			EDMA_TXDESC_BUFFER_ADDR_SET(txd, buff_addr);
			EDMA_TXDESC_DATA_LEN_SET(txd, buf_len);
			EDMA_TXDESC_POOL_ID_SET(txd, 0);

			*hw_next_to_use = (*hw_next_to_use + 1) & EDMA_TX_RING_SIZE_MASK;
			num_descs += 1;
			num_sg_frag_list += 1;

			/* skb fraglist skb can have nr_frags. */
skip_primary:
			if (unlikely(skb_shinfo(iter_skb)->nr_frags)) {
				num_nr_frag = edma_tx_skb_nr_frags(txdesc_ring, &txd,
								   iter_skb, hw_next_to_use,
								   &invalid_frag);
				if (unlikely(!num_nr_frag)) {
					dev_dbg(dev, "No descriptor available for ring %d\n",
						txdesc_ring->id);
					edma_tx_dma_unmap_frags(iter_skb, invalid_frag);
					goto dma_err;
				}

				num_descs += num_nr_frag;
				num_sg_frag_list += num_nr_frag;

				/* Update fraglist with nr_frag stats. */
				u64_stats_update_begin(&stats->syncp);
				stats->tx_fraglist_with_nr_frags_pkts++;
				u64_stats_update_end(&stats->syncp);
			}
		}

		EDMA_TXDESC_ENDIAN_SET(txd);

		/* This will be the index previous to
		 * that of current *hw_next_to_use.
		 */
		end_idx = (((*hw_next_to_use) + EDMA_TX_RING_SIZE_MASK) &
			   EDMA_TX_RING_SIZE_MASK);

		/* Update frag_list stats. */
		u64_stats_update_begin(&stats->syncp);
		stats->tx_fraglist_pkts++;
		u64_stats_update_end(&stats->syncp);
	} else {
		/* Process skb with nr_frags. */
		num_descs += edma_tx_skb_nr_frags(txdesc_ring, &txd, skb,
						  hw_next_to_use, &invalid_frag);
		if (unlikely(!num_descs)) {
			dev_dbg(dev, "No descriptor available for ring %d\n", txdesc_ring->id);
			edma_tx_dma_unmap_frags(skb, invalid_frag);
			*txdesc = NULL;
			return num_descs;
		}

		u64_stats_update_begin(&stats->syncp);
		stats->tx_nr_frag_pkts++;
		u64_stats_update_end(&stats->syncp);
	}

	dev_dbg(dev, "skb:%p num_descs_filled: %u, nr_frags %u, frag_list fragments %u\n",
		skb, num_descs, skb_shinfo(skb)->nr_frags, num_sg_frag_list);

	*txdesc = txd;

	return num_descs;

dma_err:
	if (!num_sg_frag_list)
		goto reset_state;

	edma_tx_handle_dma_err(skb, num_sg_frag_list);

reset_state:
	*hw_next_to_use = start_hw_next_to_use;
	*txdesc = NULL;

	return 0;
}

static u32 edma_tx_avail_desc(struct edma_txdesc_ring *txdesc_ring,
			      u32 hw_next_to_use)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	u32 data = 0, avail = 0, hw_next_to_clean = 0;
	struct regmap *regmap = ppe_dev->regmap;
	u32 reg;

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC_CONS_IDX(txdesc_ring->id);
	regmap_read(regmap, reg, &data);
	hw_next_to_clean = data & EDMA_TXDESC_CONS_IDX_MASK;

	avail = EDMA_DESC_AVAIL_COUNT(hw_next_to_clean - 1,
				      hw_next_to_use, EDMA_TX_RING_SIZE);

	return avail;
}

/**
 * edma_tx_ring_xmit - Transmit a packet.
 * @netdev: Netdevice.
 * @skb: Socket Buffer.
 * @txdesc_ring: Tx Descriptor ring.
 * @stats: EDMA Tx Statistics.
 *
 * Check for available descriptors, fill the descriptors
 * and transmit both linear and non linear packets.
 *
 * Return 0 on success, negative error code on failure.
 */
enum edma_tx_status edma_tx_ring_xmit(struct net_device *netdev,
				      struct sk_buff *skb, struct edma_txdesc_ring *txdesc_ring,
				struct edma_port_tx_stats *stats)
{
	struct edma_txdesc_stats *txdesc_stats = &txdesc_ring->txdesc_stats;
	struct edma_port_priv *port_priv = netdev_priv(netdev);
	u32 num_tx_desc_needed = 0, num_desc_filled = 0;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct ppe_port *port = port_priv->ppe_port;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_txdesc_pri *txdesc = NULL;
	struct device *dev = ppe_dev->dev;
	int port_id = port->port_id;
	u32 hw_next_to_use = 0;
	u32 reg;

	hw_next_to_use = txdesc_ring->prod_idx;

	if (unlikely(!(txdesc_ring->avail_desc)))  {
		txdesc_ring->avail_desc = edma_tx_avail_desc(txdesc_ring,
							     hw_next_to_use);
		if (unlikely(!txdesc_ring->avail_desc)) {
			netdev_dbg(netdev, "No available descriptors are present at %d ring\n",
				   txdesc_ring->id);

			u64_stats_update_begin(&txdesc_stats->syncp);
			++txdesc_stats->no_desc_avail;
			u64_stats_update_end(&txdesc_stats->syncp);
			return EDMA_TX_FAIL_NO_DESC;
		}
	}

	/* Process head skb for linear skb.
	 * Process head skb + nr_frags + fraglist for non linear skb.
	 */
	if (likely(!skb_is_nonlinear(skb))) {
		txdesc = edma_tx_skb_first_desc(port_priv, txdesc_ring, skb,
						&hw_next_to_use, stats);
		if (unlikely(!txdesc)) {
			netdev_dbg(netdev, "No descriptor available for ring %d\n",
				   txdesc_ring->id);
			u64_stats_update_begin(&txdesc_stats->syncp);
			++txdesc_stats->no_desc_avail;
			u64_stats_update_end(&txdesc_stats->syncp);
			return EDMA_TX_FAIL_NO_DESC;
		}

		EDMA_TXDESC_ENDIAN_SET(txdesc);
		num_desc_filled++;
	} else {
		num_tx_desc_needed = edma_tx_num_descs_for_sg(skb);

		/* HW does not support TSO for packets with more than 32 segments.
		 * HW hangs up if it sees more than 32 segments. Kernel Perform GSO
		 * for such packets with netdev gso_max_segs set to 32.
		 */
		if (unlikely(num_tx_desc_needed > EDMA_TX_TSO_SEG_MAX)) {
			netdev_dbg(netdev, "Number of segments %u more than %u for %d ring\n",
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
			txdesc_ring->avail_desc = edma_tx_avail_desc(txdesc_ring,
								     hw_next_to_use);
			if (num_tx_desc_needed > txdesc_ring->avail_desc) {
				u64_stats_update_begin(&txdesc_stats->syncp);
				++txdesc_stats->no_desc_avail;
				u64_stats_update_end(&txdesc_stats->syncp);
				netdev_dbg(netdev, "Not enough available descriptors are present at %d ring for SG packet. Needed %d, currently available %d\n",
					   txdesc_ring->id, num_tx_desc_needed,
					   txdesc_ring->avail_desc);
				return EDMA_TX_FAIL_NO_DESC;
			}
		}

		txdesc = edma_tx_skb_first_desc(port_priv, txdesc_ring, skb,
						&hw_next_to_use, stats);
		if (unlikely(!txdesc)) {
			netdev_dbg(netdev, "No non-linear descriptor available for ring %d\n",
				   txdesc_ring->id);
			u64_stats_update_begin(&txdesc_stats->syncp);
			++txdesc_stats->no_desc_avail;
			u64_stats_update_end(&txdesc_stats->syncp);
			return EDMA_TX_FAIL_NO_DESC;
		}

		num_desc_filled = edma_tx_skb_sg_fill_desc(txdesc_ring,
							   &txdesc, skb, &hw_next_to_use, stats);
		if (unlikely(!txdesc)) {
			netdev_dbg(netdev, "No descriptor available for ring %d\n",
				   txdesc_ring->id);
			dma_unmap_single(dev, virt_to_phys(skb->data),
					 skb->len, DMA_TO_DEVICE);
			u64_stats_update_begin(&txdesc_stats->syncp);
			++txdesc_stats->no_desc_avail;
			u64_stats_update_end(&txdesc_stats->syncp);
			return EDMA_TX_FAIL_NO_DESC;
		}
	}

	/* Set the skb pointer to the descriptor's opaque field/s
	 * on the last descriptor of the packet/SG packet.
	 */
	EDMA_TXDESC_OPAQUE_SET(txdesc, skb);

	/* Update producer index. */
	txdesc_ring->prod_idx = hw_next_to_use & EDMA_TXDESC_PROD_IDX_MASK;
	txdesc_ring->avail_desc -= num_desc_filled;

	netdev_dbg(netdev, "%s: skb:%p tx_ring:%u proto:0x%x skb->len:%d\n port:%u prod_idx:%u ip_summed:0x%x\n",
		   netdev->name, skb, txdesc_ring->id, ntohs(skb->protocol),
		 skb->len, port_id, hw_next_to_use, skb->ip_summed);

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC_PROD_IDX(txdesc_ring->id);
	regmap_write(regmap, reg, txdesc_ring->prod_idx);

	u64_stats_update_begin(&stats->syncp);
	stats->tx_pkts++;
	stats->tx_bytes += skb->len;
	u64_stats_update_end(&stats->syncp);

	return EDMA_TX_OK;
}
