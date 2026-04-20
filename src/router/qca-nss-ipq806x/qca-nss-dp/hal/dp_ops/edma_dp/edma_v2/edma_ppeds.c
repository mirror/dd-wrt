/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <asm/cacheflush.h>
#include <linux/irq.h>
#include "edma.h"
#include "edma_regs.h"
#include "edma_debug.h"
#include "nss_dp_dev.h"
#include "edma_cfg_rx.h"
#include "edma_cfg_tx.h"
#include <ppe_drv.h>

static char edma_ppeds_txcmpl_irq_name[EDMA_PPEDS_MAX_NODES][EDMA_IRQ_NAME_SIZE];
static char edma_ppeds_rxdesc_irq_name[EDMA_PPEDS_MAX_NODES][EDMA_IRQ_NAME_SIZE];
static char edma_ppeds_rxfill_irq_name[EDMA_PPEDS_MAX_NODES][EDMA_IRQ_NAME_SIZE];
static void *edma_ppeds_tx_ring_sec_mem;
static void *edma_ppeds_rx_ring_sec_mem;
static int edma_ppeds_rx_ring_entries;
static int edma_ppeds_tx_ring_entries;

/*
 * edma_ppeds_rx_fill_ring_alloc()
 *	API to allocate Rxfill ring for PPE-DS node
 */
static int edma_ppeds_rx_fill_ring_alloc(struct edma_rxfill_ring *rxfill_ring)
{
	/*
	 * Allocate RxFill ring descriptors
	 */
	rxfill_ring->desc = dma_alloc_coherent(&edma_gbl_ctx.pdev->dev,
				(sizeof(struct edma_rxfill_desc) * rxfill_ring->count),
				&rxfill_ring->dma, GFP_KERNEL | __GFP_ZERO);
	if (!rxfill_ring->desc) {
		edma_err("Descriptor alloc for RXFILL ring %u failed\n",
							rxfill_ring->ring_id);
		return -ENOMEM;
	}

	return 0;
}

/*
 * edma_ppeds_rx_fill_ring_free()
 *	API to free Rxfill ring for PPE-DS node
 */
static void edma_ppeds_rx_fill_ring_free(struct edma_rxfill_ring *rxfill_ring)
{
	/*
	 * Free RXFILL ring descriptors
	 */
	dma_free_coherent(&edma_gbl_ctx.pdev->dev,
			(sizeof(struct edma_rxfill_desc) * rxfill_ring->count),
			rxfill_ring->desc, rxfill_ring->dma);
	rxfill_ring->desc = NULL;
	rxfill_ring->dma = (dma_addr_t)0;
}

/*
 * edma_ppeds_rx_secondary_alloc()
 *	API to allocate secondary Rx ring for PPE-DS node
 */
static int edma_ppeds_rx_secondary_alloc(struct edma_rxdesc_ring *rxdesc_ring)
{
	if (edma_ppeds_rx_ring_sec_mem) {
		rxdesc_ring->sdesc = edma_ppeds_rx_ring_sec_mem;
		if (edma_ppeds_rx_ring_entries < rxdesc_ring->count) {
			edma_err("Num of descs needed (%d) for Rx secondary ring are more than available (%d)",
					rxdesc_ring->count, edma_ppeds_rx_ring_entries);
			BUG();
		}
	} else {
		/*
		 * Allocate secondary RxDesc ring descriptors
		 */
		rxdesc_ring->sdesc = kmalloc(roundup((sizeof(struct edma_rxdesc_sec_desc) *  rxdesc_ring->count),
					SMP_CACHE_BYTES), GFP_KERNEL | __GFP_ZERO);

		edma_ppeds_rx_ring_sec_mem = rxdesc_ring->sdesc;
		edma_ppeds_rx_ring_entries = rxdesc_ring->count;
	}

	if (!rxdesc_ring->sdesc) {
		edma_err("Descriptor alloc for secondary RX ring %u failed\n",
				rxdesc_ring->ring_id);
		return -1;
	}

	rxdesc_ring->sdma = (dma_addr_t)virt_to_phys(rxdesc_ring->sdesc);
	return 0;
}

/*
 * edma_ppeds_tx_cmpl_ring_alloc()
 *	API to allocate Tx complete ring for PPE-DS node
 */
static int edma_ppeds_tx_cmpl_ring_alloc(struct edma_txcmpl_ring *txcmpl_ring)
{
	txcmpl_ring->desc = dma_alloc_coherent(&edma_gbl_ctx.pdev->dev,
				(sizeof(struct edma_txcmpl_desc) *  txcmpl_ring->count),
				&txcmpl_ring->dma, GFP_KERNEL | __GFP_ZERO);
	if (!txcmpl_ring->desc) {
		edma_err("Descriptor alloc for TXCMPL ring %u failed\n",
				txcmpl_ring->id);
		return -ENOMEM;
	}

	return 0;
}

/*
 * edma_ppeds_tx_cmpl_ring_free()
 *	API to free Tx complete ring for PPE-DS node
 */
static void edma_ppeds_tx_cmpl_ring_free(struct edma_txcmpl_ring *txcmpl_ring)
{
	dma_free_coherent(&edma_gbl_ctx.pdev->dev,
			(sizeof(struct edma_txcmpl_desc) *  txcmpl_ring->count),
			txcmpl_ring->desc, txcmpl_ring->dma);
	txcmpl_ring->desc = NULL;
	txcmpl_ring->dma = (dma_addr_t)0;
}

/*
 * edma_ppeds_tx_secondary_alloc()
 *	API to allocate secondary Tx ring for PPE-DS node
 */
static int edma_ppeds_tx_secondary_alloc(struct edma_txdesc_ring *txdesc_ring)
{
	if (edma_ppeds_tx_ring_sec_mem) {
		txdesc_ring->sdesc = edma_ppeds_tx_ring_sec_mem;
		if (edma_ppeds_tx_ring_entries < txdesc_ring->count) {
			edma_err("Num of descs needed (%d) for Tx secondary ring are more than available (%d)",
					txdesc_ring->count, edma_ppeds_tx_ring_entries);
			BUG();
		}
	} else {
		/*
		 * Allocate sencondary Tx ring descriptors
		 */
		txdesc_ring->sdesc = kmalloc(roundup((sizeof(struct edma_sec_txdesc) * txdesc_ring->count),
					SMP_CACHE_BYTES), GFP_KERNEL | __GFP_ZERO);

		edma_ppeds_tx_ring_sec_mem = txdesc_ring->sdesc;
		edma_ppeds_tx_ring_entries = txdesc_ring->count;
	}
	if (!txdesc_ring->sdesc) {
		edma_err("Descriptor alloc for secondary TX ring %u failed\n",
				txdesc_ring->id);
		return -1;
	}

	txdesc_ring->sdma = (dma_addr_t)virt_to_phys(txdesc_ring->sdesc);
	edma_debug("tx sec desc got allocated for Tx ring %d\n", txdesc_ring->id);
	return 0;
}

/*
 * edma_ppeds_tx_complete()
 *	PPE-DS EDMA Tx complete processing API
 */
static uint32_t edma_ppeds_tx_complete(uint32_t work_to_do, struct edma_txcmpl_ring *txcmpl_ring)
{
	struct edma_ppeds *ppeds_node = container_of(txcmpl_ring, struct edma_ppeds, txcmpl_ring);
	nss_dp_ppeds_handle_t *ppeds_handle = &ppeds_node->ppeds_handle;
	struct edma_txcmpl_desc *txcmpl;
	uint32_t cons_idx, prod_idx, data, avail;
	uint16_t count;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;

	cons_idx = txcmpl_ring->cons_idx;

	/*
	 * Get TXCMPL ring producer index
	 */
	data = edma_reg_read(EDMA_REG_TXCMPL_PROD_IDX(txcmpl_ring->id));
	prod_idx = data & EDMA_TXCMPL_PROD_IDX_MASK;

	avail = EDMA_DESC_AVAIL_COUNT(prod_idx, cons_idx, txcmpl_ring->count);
	if (!avail) {
		return 0;
	}

	if (unlikely(egc->enable_ring_util_stats)) {
		edma_update_ring_stats(avail, ppeds_node->txcmpl_ring.count,
				       &ppeds_node->txcmpl_ring.tx_cmpl_stats.ring_stats);
	}

	avail = min(avail, work_to_do);
	count = avail;

	txcmpl = EDMA_TXCMPL_DESC(txcmpl_ring, cons_idx);

	while (likely(avail--)) {
		ppeds_handle->tx_cmpl_arr[count - avail - 1].cookie = EDMA_TXCMPL_OPAQUE_GET(txcmpl);

		cons_idx = ((cons_idx + 1) & (txcmpl_ring->count - 1));
		txcmpl = EDMA_TXCMPL_DESC(txcmpl_ring, cons_idx);
	}

	txcmpl_ring->cons_idx = cons_idx;
	edma_reg_write(EDMA_REG_TXCMPL_CONS_IDX(txcmpl_ring->id), cons_idx);

	ppeds_node->ops->tx_cmpl(ppeds_handle, count);
	return count;
}

/*
 * edma_ppeds_txcomp_napi_poll()
 *	PPE-DS EDMA TX NAPI handler
 */
static int edma_ppeds_txcomp_napi_poll(struct napi_struct *napi, int budget)
{
	struct edma_txcmpl_ring *txcmpl_ring = (struct edma_txcmpl_ring *)napi;
	struct edma_ppeds *ppeds_node = container_of(txcmpl_ring, struct edma_ppeds, txcmpl_ring);
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	uint32_t txcmpl_intr_status;
	int work_done = 0;
	uint32_t reg_data;

	set_bit(EDMA_PPEDS_TXCOMP_NAPI_BIT, &ppeds_node->service_running);
	do {
		work_done += edma_ppeds_tx_complete(budget - work_done, txcmpl_ring);
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
	clear_bit(EDMA_PPEDS_TXCOMP_NAPI_BIT, &ppeds_node->service_running);
	if (!ppeds_node->umac_reset_inprogress) {
		edma_reg_write(EDMA_REG_TX_INT_MASK(txcmpl_ring->id),
				egc->txcmpl_intr_mask);
	} else {
		if ((!ppeds_node->service_running) &&
			 (ppeds_node->ops->notify_napi_done)) {
			ppeds_node->ops->notify_napi_done(&ppeds_node->ppeds_handle);
		}
	}

	return work_done;
}

/*
 * edma_ppeds_rx_alloc_buffer()
 *	Alloc Rx buffers for RxFill ring
 */
static void edma_ppeds_rx_alloc_buffer(struct edma_rxfill_ring *rxfill_ring, int alloc_count, struct nss_dp_ppeds_rx_fill_elem *rx_fill_arr,
		uint32_t headroom)
{
	struct edma_rxfill_desc *rxfill_desc;
	uint16_t prod_idx, start_idx;
	uint16_t num_alloc = 0;
	uint32_t rx_alloc_size = rxfill_ring->alloc_size;
	uint32_t ring_size_mask = rxfill_ring->count -1 ;

	/*
	 * Get RXFILL ring producer index
	 */
	prod_idx = rxfill_ring->prod_idx;
	start_idx = prod_idx;

	while (likely(alloc_count--)) {
		/*
		 * Get RXFILL descriptor
		 */
		rxfill_desc = EDMA_RXFILL_DESC(rxfill_ring, prod_idx);

		EDMA_RXFILL_BUFFER_ADDR_SET(rxfill_desc, rx_fill_arr[num_alloc].buff_addr);
		rxfill_desc->word2 = rx_fill_arr[num_alloc].opaque_lo;
		rxfill_desc->word3 = rx_fill_arr[num_alloc].opaque_hi;
		EDMA_RXFILL_PACKET_LEN_SET(rxfill_desc,
			((uint32_t)(rx_alloc_size - headroom)
			& EDMA_RXFILL_BUF_SIZE_MASK));

		prod_idx = (prod_idx + 1) & ring_size_mask;
		EDMA_RXFILL_ENDIAN_SET(rxfill_desc);
		num_alloc++;
	}

	if (likely(num_alloc)) {
		edma_reg_write(EDMA_REG_RXFILL_PROD_IDX(rxfill_ring->ring_id),
								prod_idx);
		rxfill_ring->prod_idx = prod_idx;
	}
}

/*
 * edma_ppeds_rxfill_napi_poll()
 *	EDMA RXFill NAPI handler
 */
static int edma_ppeds_rxfill_napi_poll(struct napi_struct *napi, int budget)
{
	uint32_t cons_idx, work_to_do;
	uint32_t num_avail = 0;
	struct edma_rxfill_ring *rxfill_ring = (struct edma_rxfill_ring *)napi;
	struct edma_ppeds *ppeds_node = container_of(rxfill_ring, struct edma_ppeds, rxfill_ring);
	uint32_t alloc_size = rxfill_ring->alloc_size;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	uint32_t headroom = EDMA_RX_SKB_HEADROOM + NET_IP_ALIGN;

	cons_idx = edma_reg_read(EDMA_REG_RXFILL_CONS_IDX(rxfill_ring->ring_id)) &
				EDMA_RXFILL_CONS_IDX_MASK;
	work_to_do = (cons_idx - rxfill_ring->prod_idx + rxfill_ring->count - 1) & (rxfill_ring->count - 1);

	if (unlikely(egc->enable_ring_util_stats)) {
		edma_update_ring_stats(work_to_do, rxfill_ring->count,
				       &rxfill_ring->rx_fill_stats.ring_stats);
	}

	if (work_to_do > budget) {
		work_to_do = budget;
	}

	if (unlikely(!work_to_do)) {
		goto napi_complete;
	}

	num_avail = ppeds_node->ops->rx_fill(&ppeds_node->ppeds_handle, work_to_do,
						alloc_size, headroom);
	if (likely(num_avail))
		edma_ppeds_rx_alloc_buffer(rxfill_ring, num_avail,
						ppeds_node->ppeds_handle.rx_fill_arr, headroom);

	edma_reg_read(EDMA_REG_RXFILL_INT_STAT(rxfill_ring->ring_id));

	if (work_to_do < budget) {
		goto napi_complete;
	}

	return budget;

napi_complete:
	napi_complete(napi);
	if (!ppeds_node->umac_reset_inprogress) {
		edma_reg_write(EDMA_REG_RXFILL_INT_MASK(rxfill_ring->ring_id),
				EDMA_RXFILL_INT_MASK);
	}
	return 0;
}

/*
 * edma_ppeds_rx_napi_poll()
 *	EDMA RX NAPI handler
 */
static int edma_ppeds_rx_napi_poll(struct napi_struct *napi, int budget)
{
	struct edma_rxdesc_ring *rxdesc_ring = (struct edma_rxdesc_ring *)napi;
	struct edma_ppeds *ppeds_node = container_of(rxdesc_ring, struct edma_ppeds, rx_ring);
	uint16_t prod_idx;
	uint32_t status;

	/*
	 * Read EDMA Prod Idx.
	 */
	prod_idx = edma_reg_read(EDMA_REG_RXDESC_PROD_IDX(rxdesc_ring->ring_id)) &
		EDMA_RXDESC_PROD_IDX_MASK;

	/*
	 * Update Prod idx to DS interface
	 */
	ppeds_node->ops->rx(&ppeds_node->ppeds_handle, prod_idx);


	/*
	 * Clear on read
	 */
	status = EDMA_RXDESC_RING_INT_STATUS_MASK &
		edma_reg_read(EDMA_REG_RXDESC_INT_STAT(rxdesc_ring->ring_id));

	napi_complete(napi);

	/*
	 * Set RXDESC ring interrupt mask
	 */
	if (!ppeds_node->umac_reset_inprogress) {
		edma_reg_write(EDMA_REG_RXDESC_INT_MASK(rxdesc_ring->ring_id),
				EDMA_RXDESC_INT_MASK_PKT_INT);
	}

	return 0;
}

/*
 * edma_ppeds_set_rx_mapping()
 *	API for PPE-DS EDMA Rx ring mapping
 */
static void edma_ppeds_set_rx_mapping(uint32_t rxfill_ring_id, uint32_t rx_ring_id, uint32_t ppe_qid,
		uint32_t num_ppe_queues)
{
	uint32_t reg, data;
	uint8_t start_reg_index = ppe_qid/EDMA_QID2RID_NUM_PER_REG;
	uint8_t start_qnum = ppe_qid - (start_reg_index * EDMA_QID2RID_NUM_PER_REG);

	/*
	 * Setup RxFill to Rx mapping.
	 */
	if ((rx_ring_id >= 0) && (rx_ring_id <= 9)) {
		reg = EDMA_REG_RXDESC2FILL_MAP_0;
	} else if ((rx_ring_id >= 10) && (rx_ring_id <= 19)) {
		reg = EDMA_REG_RXDESC2FILL_MAP_1;
	} else {
		reg = EDMA_REG_RXDESC2FILL_MAP_2;
	}

	data = edma_reg_read(reg);
	data |= (rxfill_ring_id &
			EDMA_RXDESC2FILL_MAP_RXDESC_MASK) <<
		((rx_ring_id % 10) * 3);
	edma_reg_write(reg, data);

	/*
	 * Setup PPE Queue to Rx Ring mapping.
	 */
	while (num_ppe_queues > 0) {
		data = edma_reg_read(EDMA_QID2RID_TABLE_MEM(start_reg_index));
		switch (start_qnum) {
		case 0:
			data &= (~EDMA_RX_RING_ID_QUEUE0_MASK);
			data |= EDMA_RX_RING_ID_QUEUE0_SET(rx_ring_id);
			num_ppe_queues--;
			if (num_ppe_queues == 0) {
				break;
			}
#if GCC_VERSION > 70500
			fallthrough;
#endif
			/* fall through */
		case 1:
			data &= (~EDMA_RX_RING_ID_QUEUE1_MASK);
			data |= EDMA_RX_RING_ID_QUEUE1_SET(rx_ring_id);
			num_ppe_queues--;
			if (num_ppe_queues == 0) {
				break;
			}
#if GCC_VERSION > 70500
			fallthrough;
#endif
			/* fall through */
		case 2:
			data &= (~EDMA_RX_RING_ID_QUEUE2_MASK);
			data |= EDMA_RX_RING_ID_QUEUE2_SET(rx_ring_id);
			num_ppe_queues--;
			if (num_ppe_queues == 0) {
				break;
			}
#if GCC_VERSION > 70500
			fallthrough;
#endif
			/* fall through */
		case 3:
			data &= (~EDMA_RX_RING_ID_QUEUE3_MASK);
			data |= EDMA_RX_RING_ID_QUEUE3_SET(rx_ring_id);
			num_ppe_queues--;
			if (num_ppe_queues == 0) {
				break;
			}
		}
		start_qnum = 0;
		edma_reg_write(EDMA_QID2RID_TABLE_MEM(start_reg_index), data);
		start_reg_index++;
	}

	edma_debug("PPE Queue %d mapped to EDMA ring %d", ppe_qid, rx_ring_id);
}

/*
 * edma_ppeds_set_tx_mapping()
 *	API for PPE-DS EDMA Tx ring mapping
 */
static void edma_ppeds_set_tx_mapping(uint32_t tx_ring_id, uint32_t txcmpl_ring_id)
{
	uint32_t reg, data;

	/*
	 * Setup TxDesc to TxComplete mapping.
	 */
	if ((tx_ring_id >= 0) && (tx_ring_id <= 5)) {
		reg = EDMA_REG_TXDESC2CMPL_MAP_0;
	} else if ((tx_ring_id >= 6) && (tx_ring_id <= 11)) {
		reg = EDMA_REG_TXDESC2CMPL_MAP_1;
	} else if ((tx_ring_id >= 12) && (tx_ring_id <= 17)) {
		reg = EDMA_REG_TXDESC2CMPL_MAP_2;
	} else if ((tx_ring_id >= 18) && (tx_ring_id <= 23)) {
		reg = EDMA_REG_TXDESC2CMPL_MAP_3;
	} else if ((tx_ring_id >= 24) && (tx_ring_id <= 29)) {
		reg = EDMA_REG_TXDESC2CMPL_MAP_4;
	} else {
		reg = EDMA_REG_TXDESC2CMPL_MAP_5;
	}

	data = edma_reg_read(reg);
	data |= (txcmpl_ring_id & EDMA_TXDESC2CMPL_MAP_TXDESC_MASK) <<
		 ((tx_ring_id % EDMA_TXDESC2CMPL_MAP_NUM_IN_SINGLE_REG) *
		   EDMA_TXDESC2CMPL_MAP_TXDESC_ID_BIT_COUNT);
	edma_reg_write(reg, data);
}

/*
 * edma_ppeds_cfg_tx()
 *	API to configure PPE-DS EDMA Tx ring
 */
static void edma_ppeds_cfg_tx(struct edma_ppeds *ppeds_node)
{
	uint32_t tx_mod_timer;
	struct edma_txdesc_ring *txdesc_ring = &ppeds_node->tx_ring;
	struct edma_txcmpl_ring *txcmpl_ring = &ppeds_node->txcmpl_ring;

	/*
	 * Configure TXDESC ring
	 */
	edma_reg_write(EDMA_REG_TXDESC_BA(txdesc_ring->id),
			(uint32_t)(txdesc_ring->pdma &
				EDMA_RING_DMA_MASK));

	edma_reg_write(EDMA_REG_TXDESC_BA2(txdesc_ring->id),
			(uint32_t)(txdesc_ring->sdma &
				EDMA_RING_DMA_MASK));

	edma_reg_write(EDMA_REG_TXDESC_RING_SIZE(txdesc_ring->id),
			(uint32_t)(txdesc_ring->count &
				EDMA_TXDESC_RING_SIZE_MASK));

	edma_reg_write(EDMA_REG_TXDESC_PROD_IDX(txdesc_ring->id), EDMA_TX_INITIAL_PROD_IDX);

	/*
	 * Configure TxCmpl ring base address
	 */
	edma_reg_write(EDMA_REG_TXCMPL_BA(txcmpl_ring->id),
			(uint32_t)(txcmpl_ring->dma & EDMA_RING_DMA_MASK));
	edma_reg_write(EDMA_REG_TXCMPL_RING_SIZE(txcmpl_ring->id),
			(uint32_t)(txcmpl_ring->count & EDMA_TXDESC_RING_SIZE_MASK));

	/*
	 * Set TxCmpl ret mode to opaque
	 */
	edma_reg_write(EDMA_REG_TXCMPL_CTRL(txcmpl_ring->id),
			EDMA_TXCMPL_RETMODE_OPAQUE);

	/*
	 * Configure the default timer mitigation value
	 */
	tx_mod_timer = (EDMA_TX_MOD_TIMER & EDMA_TX_MOD_TIMER_INIT_MASK)
			<< EDMA_TX_MOD_TIMER_INIT_SHIFT;
	edma_reg_write(EDMA_REG_TX_MOD_TIMER(txcmpl_ring->id),
				tx_mod_timer);

	txcmpl_ring->cons_idx = edma_reg_read(EDMA_REG_TXCMPL_CONS_IDX(txcmpl_ring->id));

	edma_reg_write(EDMA_REG_TX_INT_CTRL(txcmpl_ring->id), EDMA_TX_NE_INT_EN);
}

/*
 * edma_ppeds_rx_desc_ring_to_queue_mapping()
 *	PPE-DS rx descriptor ring to queue mapping API
 */
static void edma_ppeds_rx_desc_ring_to_queue_mapping(struct edma_ppeds *ppeds_node)
{
	struct edma_rxdesc_ring *rxdesc_ring = &ppeds_node->rx_ring;
	uint32_t num_queues = ppeds_node->ppe_num_queues;
	uint32_t queue_id = ppeds_node->ppe_qid;
	fal_queue_bmp_t queue_bmp = {0};
	uint32_t word_idx, bit_idx, i;
	sw_error_t ret;

	while (num_queues) {
		word_idx = (queue_id / (EDMA_BITS_IN_WORD - 1));
		if (word_idx >= EDMA_RING_MAPPED_QUEUE_BM_WORD_COUNT) {
			edma_err("Invalid word index (%d) for %d queue\n", word_idx, queue_id);
			return;
		}

		bit_idx = (queue_id % EDMA_BITS_IN_WORD);
		queue_bmp.bmp[word_idx] |= (1 << bit_idx);
		num_queues--;
		queue_id++;
	}

	ret = fal_edma_ring_queue_map_set(EDMA_SWITCH_DEV_ID, rxdesc_ring->ring_id, &queue_bmp);
	if (ret != SW_OK) {
		edma_err("Error in configuring Rx ring to PPE queue mapping."
				" ret: %d, ring id: %d\n",
				ret, rxdesc_ring->ring_id);
		for (i = 0; i < EDMA_RING_MAPPED_QUEUE_BM_WORD_COUNT; i++) {
			edma_err("\tPPE queue bitmap[%d]: %0x\n", i, queue_bmp.bmp[i]);
		}
		return;
	}

	edma_debug("Rx desc ring %d to PPE queue mapping for backpressure:\n",
			rxdesc_ring->ring_id);
	for (i = 0; i < EDMA_RING_MAPPED_QUEUE_BM_WORD_COUNT; i++) {
		edma_debug("\tPPE queue bitmap[%d]: %0x\n", i, queue_bmp.bmp[i]);
	}
}

/*
 * edma_ppeds_rx_desc_ring_flow_control()
 *	PPE-DS Rx descriptor ring flow control configuration API
 */
static void edma_ppeds_rx_desc_ring_flow_control(struct edma_rxdesc_ring *rxdesc_ring)
{
	uint32_t data;

	data = (EDMA_PPEDS_RX_FC_XOFF_DEF & EDMA_RXDESC_FC_XOFF_THRE_MASK) <<
			 EDMA_RXDESC_FC_XOFF_THRE_SHIFT;
	data |= ((EDMA_PPEDS_RX_FC_XON_DEF & EDMA_RXDESC_FC_XON_THRE_MASK) <<
			 EDMA_RXDESC_FC_XON_THRE_SHIFT);

	edma_debug("Rxdesc flow control threshold value is %d for ring: %d\n",
			data, rxdesc_ring->ring_id);
	edma_reg_write(EDMA_REG_RXDESC_FC_THRE(rxdesc_ring->ring_id), data);
}

/*
 * edma_ppeds_rx_fill_ring_flow_control()
 *	PPE-DS Rx fill ring flow control configuration API
 */
static void edma_ppeds_rx_fill_ring_flow_control(struct edma_rxfill_ring *rxfill_ring)
{
	uint32_t data;

	data = (EDMA_PPEDS_RX_FC_XOFF_DEF & EDMA_RXFILL_FC_XOFF_THRE_MASK) <<
			 EDMA_RXFILL_FC_XOFF_THRE_SHIFT;
	data |= ((EDMA_PPEDS_RX_FC_XON_DEF & EDMA_RXFILL_FC_XON_THRE_MASK) <<
			 EDMA_RXFILL_FC_XON_THRE_SHIFT);

	edma_debug("Rxfill flow control threshold value is %d for ring: %d\n",
			data, rxfill_ring->ring_id);
	edma_reg_write(EDMA_REG_RXFILL_FC_THRE(rxfill_ring->ring_id), data);
}

/*
 * edma_ppeds_cfg_rx()
 *	API to configure PPE-DS EDMA Rx ring
 */
static void edma_ppeds_cfg_rx(struct edma_ppeds *ppeds_node)
{
	struct edma_rxfill_ring *rxfill_ring = &ppeds_node->rxfill_ring;
	struct edma_rxdesc_ring *rxdesc_ring = &ppeds_node->rx_ring;
	uint32_t ring_sz;
	uint32_t data;

	edma_reg_write(EDMA_REG_RXFILL_BA(rxfill_ring->ring_id),
			(uint32_t)(rxfill_ring->dma & EDMA_RING_DMA_MASK));

	ring_sz = rxfill_ring->count & EDMA_RXFILL_RING_SIZE_MASK;
	edma_reg_write(EDMA_RXFILL_RING_SIZE(rxfill_ring->ring_id), ring_sz);

	rxfill_ring->prod_idx = edma_reg_read(EDMA_REG_RXFILL_PROD_IDX(rxfill_ring->ring_id));

	edma_reg_write(EDMA_REG_RXDESC_BA(rxdesc_ring->ring_id),
			(uint32_t)(rxdesc_ring->pdma & EDMA_RXDESC_BA_MASK));

	edma_reg_write(EDMA_REG_RXDESC_PREHEADER_BA(rxdesc_ring->ring_id),
			(uint32_t)(rxdesc_ring->sdma & EDMA_RXDESC_PREHEADER_BA_MASK));

	data = rxdesc_ring->count & EDMA_RXDESC_RING_SIZE_MASK;

	/*
	 * For SOC's where Rxdesc ring register do not contain PL offset
	 * fields, skip writing that data into the Register.
	 */
#if !defined(NSS_DP_EDMA_SKIP_PL_OFFSET)
	data |= (EDMA_RXDESC_PL_DEFAULT_VALUE & EDMA_RXDESC_PL_OFFSET_MASK)
		 << EDMA_RXDESC_PL_OFFSET_SHIFT;
#endif
	edma_reg_write(EDMA_REG_RXDESC_RING_SIZE(rxdesc_ring->ring_id), data);

	/*
	 * Configure the default timer mitigation value
	 */
	data = (EDMA_RX_MOD_TIMER_INIT & EDMA_RX_MOD_TIMER_INIT_MASK)
			<< EDMA_RX_MOD_TIMER_INIT_SHIFT;
	edma_reg_write(EDMA_REG_RX_MOD_TIMER(rxdesc_ring->ring_id), data);

	/*
	 * Enable ring. Set ret mode to 'opaque'.
	 */
	edma_reg_write(EDMA_REG_RX_INT_CTRL(rxdesc_ring->ring_id), EDMA_RX_NE_INT_EN);

	/*
	 * Configure flow control and Rx ring to queue mapping
	 */
	edma_ppeds_rx_desc_ring_to_queue_mapping(ppeds_node);
	edma_ppeds_rx_desc_ring_flow_control(rxdesc_ring);
	edma_ppeds_rx_fill_ring_flow_control(rxfill_ring);
}

/*
 * edma_ppeds_rx_handle_irq
 *	Disable edma interrupt and enable wlan interrupt
 */
irqreturn_t edma_ppeds_rx_handle_irq(int irq, void *ctx)
{
	struct edma_rxdesc_ring *rxdesc_ring = (struct edma_rxdesc_ring *)ctx;
	struct edma_ppeds *ppeds_node =
			container_of(rxdesc_ring, struct edma_ppeds, rx_ring);

	/*
	 * Clear RxDesc ring interrupt mask
	 */
	edma_reg_write(EDMA_REG_RXDESC_INT_MASK(rxdesc_ring->ring_id),
			EDMA_MASK_INT_CLEAR);

	/*
	 * Enable wlan interrupt
	 */
	ppeds_node->ops->enable_wlan_intr(&ppeds_node->ppeds_handle, true);

	return IRQ_HANDLED;
}

/*
 * edma_ppeds_enable_rx_reap_intr()
 *	PPEDS enable edma interrupt
 */
static void edma_ppeds_enable_rx_reap_intr(nss_dp_ppeds_handle_t *ppeds_handle)
{
	struct edma_ppeds *ppeds_node =
		container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);
	struct edma_rxdesc_ring *rx_ring = &ppeds_node->rx_ring;
	uint32_t status;

	/*
	 * Clear on Read
	 */
	status = edma_reg_read(EDMA_REG_RXDESC_INT_STAT(rx_ring->ring_id)) &
			EDMA_RXDESC_RING_INT_STATUS_MASK;

	/*
	 * Set RXDESC ring interrupt mask
	 */
	edma_reg_write(EDMA_REG_RXDESC_INT_MASK(rx_ring->ring_id),
			EDMA_RXDESC_INT_MASK_PKT_INT);
}

/*
 * edma_ppeds_inst_register()
 *	PPE-DS EDMA instance registration API
 */
bool edma_ppeds_inst_register(nss_dp_ppeds_handle_t *ppeds_handle)
{
	int ret;
	uint32_t alloc_size;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	struct edma_ppeds_drv *drv = &edma_gbl_ctx.ppeds_drv;
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);
	struct edma_ppeds_node_cfg *node_cfg = &(drv->ppeds_node_cfg[ppeds_node->db_idx]);
	uint32_t rx_ring_size = ppeds_handle->ppe2tcl_num_desc;
	uint32_t tx_ring_size = ppeds_handle->reo2ppe_num_desc;

	write_lock_bh(&drv->lock);
	if (node_cfg->node_state != EDMA_PPEDS_NODE_STATE_ALLOC) {
		edma_err("%px: Invalid node state: %d, registration failed\n", ppeds_node,
				node_cfg->node_state);
		write_unlock_bh(&drv->lock);
		return false;
	}
	node_cfg->node_state = EDMA_PPEDS_NODE_STATE_REG_IN_PROG;
	write_unlock_bh(&drv->lock);

	if (egc->rx_jumbo_mru)
		alloc_size = egc->rx_jumbo_mru;
	else
		alloc_size = NSS_DP_RX_BUFFER_SIZE;

	ppeds_node->rxfill_ring.count = ppeds_handle->ppe2tcl_rxfill_num_desc;
	ppeds_node->rxfill_ring.alloc_size  = alloc_size;
	ppeds_node->rx_ring.count = rx_ring_size;
	ppeds_node->rx_ring.pdma = (dma_addr_t)ppeds_handle->ppe2tcl_ba;

	ret = edma_ppeds_rx_fill_ring_alloc(&ppeds_node->rxfill_ring);
	if (ret != 0) {
		return false;
	}

	ppeds_node->txcmpl_ring.count = ppeds_handle->reo2ppe_txcmpl_num_desc;
	ppeds_node->tx_ring.count = tx_ring_size;
	ppeds_node->tx_ring.pdma = (dma_addr_t)ppeds_handle->reo2ppe_ba;
	ppeds_node->tx_ring.pdesc = phys_to_virt(ppeds_node->tx_ring.pdma);

	memset(ppeds_node->tx_ring.pdesc, 0, 32 * tx_ring_size);

	ret = edma_ppeds_tx_cmpl_ring_alloc(&ppeds_node->txcmpl_ring);
	if (ret != 0) {
		edma_ppeds_rx_fill_ring_free(&ppeds_node->rxfill_ring);
		goto rx_fill_setup_failed;
	}

	ppeds_handle->rx_fill_arr =
		(struct nss_dp_ppeds_rx_fill_elem *)kzalloc(sizeof(struct nss_dp_ppeds_rx_fill_elem) * ppeds_node->rxfill_ring.count, GFP_KERNEL);
	if (!ppeds_handle->rx_fill_arr) {
		goto rx_fill_arr_alloc_failed;
		return false;
	}

	ppeds_handle->tx_cmpl_arr =
		(struct nss_dp_ppeds_tx_cmpl_elem *)kzalloc(sizeof(struct nss_dp_ppeds_tx_cmpl_elem) * ppeds_node->txcmpl_ring.count, GFP_KERNEL);
	if (!ppeds_handle->tx_cmpl_arr) {
		goto tx_cmpl_arr_alloc_failed;
		return false;
	}

	/*
	 * Setup dummy netdev for all the NAPIs associated with this node
	 */
	init_dummy_netdev(&ppeds_node->napi_ndev);
	snprintf(ppeds_node->napi_ndev.name, sizeof(ppeds_node->napi_ndev.name), "%s", "nss-ppe");

	/*
	 * Setup TxComp IRQ and NAPI
	 */
	irq_set_status_flags(ppeds_node->txcmpl_intr, IRQ_DISABLE_UNLAZY);
	snprintf(edma_ppeds_txcmpl_irq_name[ppeds_node->db_idx], 32,
			 "edma_ppeds_txcmpl_%d", ppeds_node->db_idx);
	ret = request_irq(ppeds_node->txcmpl_intr,
			edma_tx_handle_irq, IRQF_SHARED,
			edma_ppeds_txcmpl_irq_name[ppeds_node->db_idx],
			(void *)&ppeds_node->txcmpl_ring);
	if (ret) {
		edma_err("PPEDS TXCMPL ring IRQ:%d request failed for node %d\n",
				ppeds_node->txcmpl_intr, ppeds_node->db_idx);
		goto txcomp_irq_fail;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	netif_napi_add(&ppeds_node->napi_ndev, &ppeds_node->txcmpl_ring.napi,
			edma_ppeds_txcomp_napi_poll, ppeds_handle->eth_txcomp_budget);
#else
	netif_napi_add_weight(&ppeds_node->napi_ndev, &ppeds_node->txcmpl_ring.napi,
			edma_ppeds_txcomp_napi_poll, ppeds_handle->eth_txcomp_budget);
#endif

	/*
	 * Setup RxDesc IRQ and NAPI
	 */
	irq_set_status_flags(ppeds_node->rxdesc_intr, IRQ_DISABLE_UNLAZY);
	snprintf(edma_ppeds_rxdesc_irq_name[ppeds_node->db_idx], 32,
			 "edma_ppeds_rxdesc_%d", ppeds_node->db_idx);

	/*
	 * If poll mode is disabled, handle the rx_ring interrupts in irq mode,
	 * Otherwise handle them in polling mode using Napi.
	 */
	if (ppeds_handle->polling_for_idx_update) {
		ret = request_irq(ppeds_node->rxdesc_intr,
				edma_rx_handle_irq, IRQF_SHARED,
				edma_ppeds_rxdesc_irq_name[ppeds_node->db_idx],
				(void *)&ppeds_node->rx_ring);
	} else {
		ret = request_irq(ppeds_node->rxdesc_intr,
				edma_ppeds_rx_handle_irq, IRQF_SHARED,
				edma_ppeds_rxdesc_irq_name[ppeds_node->db_idx],
				(void *)&ppeds_node->rx_ring);
	}

	if (ret) {
		edma_err("PPEDS RXDESC ring IRQ:%d request failed for node %d\n",
				ppeds_node->rxdesc_intr, ppeds_node->db_idx);
		goto rxdesc_irq_fail;

	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	netif_napi_add(&ppeds_node->napi_ndev, &ppeds_node->rx_ring.napi,
			edma_ppeds_rx_napi_poll, EDMA_PPEDS_RX_WEIGHT);
#else
	netif_napi_add_weight(&ppeds_node->napi_ndev, &ppeds_node->rx_ring.napi,
		edma_ppeds_rx_napi_poll, EDMA_PPEDS_RX_WEIGHT);
#endif

	/*
	 * Setup RxFill IRQ and NAPI
	 */
	irq_set_status_flags(ppeds_node->rxfill_intr, IRQ_DISABLE_UNLAZY);
	snprintf(edma_ppeds_rxfill_irq_name[ppeds_node->db_idx], 32,
			 "edma_ppeds_rxfill_%d", ppeds_node->db_idx);
	ret = request_irq(ppeds_node->rxfill_intr,
			edma_rxfill_handle_irq, IRQF_SHARED,
			edma_ppeds_rxfill_irq_name[ppeds_node->db_idx],
			(void *)&ppeds_node->rxfill_ring);
	if (ret) {
		edma_err("PPEDS RXFILL ring IRQ:%d request failed for node %d\n",
				ppeds_node->rxfill_intr, ppeds_node->db_idx);
		goto rxfill_irq_fail;

	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	netif_napi_add(&ppeds_node->napi_ndev, &ppeds_node->rxfill_ring.napi,
			edma_ppeds_rxfill_napi_poll, EDMA_PPEDS_RXFILL_WEIGHT);
#else
	netif_napi_add_weight(&ppeds_node->napi_ndev, &ppeds_node->rxfill_ring.napi,
			edma_ppeds_rxfill_napi_poll, EDMA_PPEDS_RXFILL_WEIGHT);
#endif

	ret = edma_ppeds_rx_secondary_alloc(&ppeds_node->rx_ring);
	if (ret) {
		edma_err("Failed to setup secondary EDMA Rx Desc Ring\n");
		goto rx_sec_setup_failed;
	}

	ret = edma_ppeds_tx_secondary_alloc(&ppeds_node->tx_ring);
	if (ret) {
		edma_err("Failed to setup secondary EDMA Tx Desc Ring\n");
		goto tx_sec_setup_failed;
	}

	/*
	 * Setup Tx and Rx mappings
	 */
	edma_ppeds_set_tx_mapping(ppeds_node->tx_ring.id, ppeds_node->txcmpl_ring.id);
	edma_ppeds_set_rx_mapping(ppeds_node->rxfill_ring.ring_id, ppeds_node->rx_ring.ring_id, ppeds_node->ppe_qid,
			ppeds_node->ppe_num_queues);

	edma_ppeds_cfg_tx(ppeds_node);
	edma_ppeds_cfg_rx(ppeds_node);

	edma_debug("EDMA PPE-DS registration successfull."
			" PPE2TCL ring size: %d, REO2PPE ring size: %d,"
			" Rxfill ring size: %d, Txcmpl ring size: %d\n",
			ppeds_handle->ppe2tcl_num_desc,
			ppeds_handle->reo2ppe_num_desc,
			ppeds_handle->ppe2tcl_rxfill_num_desc,
			ppeds_handle->reo2ppe_txcmpl_num_desc);

	write_lock_bh(&drv->lock);
	node_cfg->node_state = EDMA_PPEDS_NODE_STATE_REG_DONE;
	write_unlock_bh(&drv->lock);

	return true;

tx_sec_setup_failed:
rx_sec_setup_failed:
	irq_clear_status_flags(ppeds_node->rxfill_intr, IRQ_DISABLE_UNLAZY);
	synchronize_irq(ppeds_node->rxfill_intr);
	free_irq(ppeds_node->rxfill_intr,
			(void *)&ppeds_node->rxfill_ring);
	netif_napi_del(&ppeds_node->rxfill_ring.napi);
rxfill_irq_fail:
	irq_clear_status_flags(ppeds_node->rxdesc_intr, IRQ_DISABLE_UNLAZY);
	synchronize_irq(ppeds_node->rxdesc_intr);
	free_irq(ppeds_node->rxdesc_intr,
			(void *)&ppeds_node->rx_ring);
	netif_napi_del(&ppeds_node->rx_ring.napi);
rxdesc_irq_fail:
	irq_clear_status_flags(ppeds_node->txcmpl_intr, IRQ_DISABLE_UNLAZY);
	synchronize_irq(ppeds_node->txcmpl_intr);
	free_irq(ppeds_node->txcmpl_intr,
			(void *)&ppeds_node->txcmpl_ring);
	netif_napi_del(&ppeds_node->txcmpl_ring.napi);

txcomp_irq_fail:
	kfree(ppeds_handle->tx_cmpl_arr);
	ppeds_handle->tx_cmpl_arr= NULL;
tx_cmpl_arr_alloc_failed:
	kfree(ppeds_handle->rx_fill_arr);
	ppeds_handle->rx_fill_arr = NULL;
rx_fill_arr_alloc_failed:
	edma_ppeds_rx_fill_ring_free(&ppeds_node->rxfill_ring);
rx_fill_setup_failed:
	edma_ppeds_tx_cmpl_ring_free(&ppeds_node->txcmpl_ring);

	return false;
}

/*
 * edma_ppeds_inst_refill()
 *	API to fill PPE-DS EDMA RxFill ring
 */
void edma_ppeds_inst_refill(nss_dp_ppeds_handle_t *ppeds_handle, int count)
{
	uint32_t num_avail;
	uint32_t headroom = EDMA_RX_SKB_HEADROOM + NET_IP_ALIGN;
	struct edma_ppeds_drv *drv = &edma_gbl_ctx.ppeds_drv;
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);
	struct edma_ppeds_node_cfg *node_cfg = &(drv->ppeds_node_cfg[ppeds_node->db_idx]);
	struct edma_rxfill_ring *rxfill_ring = &ppeds_node->rxfill_ring;

	read_lock_bh(&drv->lock);
	if ((node_cfg->node_state != EDMA_PPEDS_NODE_STATE_REG_DONE) &&
		(node_cfg->node_state != EDMA_PPEDS_NODE_STATE_STOP_DONE)) {
		edma_err("%px: Invalid node state: %d, PPE-DS rxfill failed\n", ppeds_node,
				node_cfg->node_state);
		read_unlock_bh(&drv->lock);
		return;
	}
	read_unlock_bh(&drv->lock);

	num_avail = ppeds_node->ops->rx_fill(&ppeds_node->ppeds_handle, count, rxfill_ring->alloc_size, headroom);

	if(count != num_avail) {
		edma_warn("Got %d less than what is asked for %d\n", num_avail, count);
	}

	edma_ppeds_rx_alloc_buffer(rxfill_ring, num_avail,
		       	ppeds_node->ppeds_handle.rx_fill_arr, headroom);
}

/*
 * edma_ppeds_get_ppe_queues()
 *	Get the associated PPE queues with the given instance
 */
bool edma_ppeds_get_ppe_queues(nss_dp_ppeds_handle_t *ppeds_handle, uint32_t *ppe_queue_start)
{
	struct edma_ppeds_drv *drv = &edma_gbl_ctx.ppeds_drv;
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);
	struct edma_ppeds_node_cfg *node_cfg = &(drv->ppeds_node_cfg[ppeds_node->db_idx]);

	read_lock_bh(&drv->lock);
	if (node_cfg->node_state != EDMA_PPEDS_NODE_STATE_START_DONE) {
		edma_err("%px: Invalid node state: %d, PPE-DS get queues failed\n", ppeds_node,
				node_cfg->node_state);
		read_unlock_bh(&drv->lock);
		return false;
	}
	read_unlock_bh(&drv->lock);

	*ppe_queue_start = ppeds_node->ppe_qid;
	return true;
}

/*
 * edma_ppeds_set_tx_prod_idx()
 *	Set EDMA RX producer idx
 */
void edma_ppeds_set_tx_prod_idx(nss_dp_ppeds_handle_t *ppeds_handle, uint16_t tx_prod_idx)
{
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);
	uint32_t cons_idx;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	uint32_t work_to_do = 0;

	edma_reg_write(EDMA_REG_TXDESC_PROD_IDX(ppeds_node->tx_ring.id), tx_prod_idx);

	if (unlikely(egc->enable_ring_util_stats)) {
		cons_idx = edma_reg_read(EDMA_REG_TXDESC_CONS_IDX(ppeds_node->tx_ring.id)) & EDMA_TXDESC_CONS_IDX_MASK;
		work_to_do = EDMA_DESC_AVAIL_COUNT(tx_prod_idx, cons_idx, ppeds_node->tx_ring.count);
		edma_update_ring_stats(work_to_do, ppeds_node->tx_ring.count,
				       &ppeds_node->tx_ring.tx_desc_stats.ring_stats);
	}
}

/*
 * edma_ppeds_set_rx_cons_idx()
 *	Set EDMA RX consumer idx
 */
void edma_ppeds_set_rx_cons_idx(nss_dp_ppeds_handle_t *ppeds_handle, uint16_t rx_cons_idx)
{
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);
	uint32_t prod_idx;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	uint32_t work_to_do = 0;

	edma_reg_write(EDMA_REG_RXDESC_CONS_IDX(ppeds_node->rx_ring.ring_id), rx_cons_idx);

	if (unlikely(egc->enable_ring_util_stats)) {
		prod_idx = edma_reg_read(EDMA_REG_RXDESC_PROD_IDX(ppeds_node->rx_ring.ring_id)) & EDMA_RXDESC_PROD_IDX_MASK;
		work_to_do = EDMA_DESC_AVAIL_COUNT(prod_idx, rx_cons_idx, ppeds_node->rx_ring.count);
		edma_update_ring_stats(work_to_do, ppeds_node->rx_ring.count,
				       &ppeds_node->rx_ring.rx_desc_stats.ring_stats);
	}
}

/*
 * edma_ppeds_get_rx_prod_idx()
 *	Get EDMA RX producer idx
 */
uint16_t edma_ppeds_get_rx_prod_idx(nss_dp_ppeds_handle_t *ppeds_handle)
{
	uint32_t data;
	uint16_t prod_idx;
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);

	data = edma_reg_read(EDMA_REG_RXDESC_PROD_IDX(ppeds_node->rx_ring.ring_id));
	prod_idx = data & EDMA_RXDESC_PROD_IDX_MASK;

	return prod_idx;
}

/*
 * edma_ppeds_get_tx_cons_idx()
 *	Get EDMA TX consumer idx
 */
uint16_t edma_ppeds_get_tx_cons_idx(nss_dp_ppeds_handle_t *ppeds_handle)
{
	uint32_t data;
	uint16_t cons_idx;
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);

	data = edma_reg_read(EDMA_REG_TXDESC_CONS_IDX(ppeds_node->tx_ring.id));
	cons_idx = data & EDMA_TXDESC_CONS_IDX_MASK;

	return cons_idx;
}

/*
 * edma_ppeds_get_rxfill_cons_idx()
 *	Get rxfill ring consumer index
 */
static uint16_t edma_ppeds_get_rxfill_cons_idx(nss_dp_ppeds_handle_t *ppeds_handle)
{
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);
	struct edma_rxfill_ring *rxfill_ring = &ppeds_node->rxfill_ring;

	return edma_reg_read(EDMA_REG_RXFILL_CONS_IDX(rxfill_ring->ring_id)) &
				EDMA_RXFILL_CONS_IDX_MASK;
}

/*
 * edma_ppeds_set_rxfill_prod_idx()
 *	Set rxfill ring producer index
 */
static void edma_ppeds_set_rxfill_prod_idx(nss_dp_ppeds_handle_t *ppeds_handle,
					   uint16_t prod_idx)
{
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);
	struct edma_rxfill_ring *rxfill_ring = &ppeds_node->rxfill_ring;

	edma_reg_write(EDMA_REG_RXFILL_PROD_IDX(rxfill_ring->ring_id), prod_idx);
}

/*
 * edma_ppeds_inst_start()
 *	PPE-DS EDMA instance start API
 */
int edma_ppeds_inst_start(nss_dp_ppeds_handle_t *ppeds_handle, uint8_t intr_enable,
				struct nss_ppe_ds_ctx_info_handle *info_hdl)
{
	uint32_t data;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	struct edma_ppeds_drv *drv = &egc->ppeds_drv;
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);
	struct edma_ppeds_node_cfg *node_cfg = &(drv->ppeds_node_cfg[ppeds_node->db_idx]);

	write_lock_bh(&drv->lock);
	if ((node_cfg->node_state != EDMA_PPEDS_NODE_STATE_REG_DONE) &&
		(node_cfg->node_state != EDMA_PPEDS_NODE_STATE_STOP_DONE)) {
		edma_err("%px: Invalid node state: %d, PPE-DS start failed\n", ppeds_node,
				node_cfg->node_state);
		write_unlock_bh(&drv->lock);
		return -1;
	}
	node_cfg->node_state = EDMA_PPEDS_NODE_STATE_START_IN_PROG;
	write_unlock_bh(&drv->lock);

	/*
	 * Configure RxFill Low threshold value and
	 * enable RXFILL Low threshold interrupt along with the
	 * associated NAPI
	 */
	edma_debug("Setting low threshold to %d\n", ppeds_handle->eth_rxfill_low_thr);

	edma_reg_write(EDMA_REG_RXFILL_UGT_THRE(ppeds_node->rxfill_ring.ring_id),
			EDMA_RXFILL_LOW_THRE_MASK & ppeds_handle->eth_rxfill_low_thr);
	edma_reg_write(EDMA_REG_RXFILL_INT_MASK(ppeds_node->rxfill_ring.ring_id),
			EDMA_RXFILL_INT_MASK);
	if (!ppeds_node->umac_reset_inprogress) {
		napi_enable(&ppeds_node->rxfill_ring.napi);
	}

	/*
	 * Enable TxComp interrupt along with the associated NAPI
	 */
	edma_reg_write(EDMA_REG_TX_INT_MASK(ppeds_node->txcmpl_ring.id),
			EDMA_TX_INT_MASK_PKT_INT);
	if (!ppeds_node->umac_reset_inprogress) {
		napi_enable(&ppeds_node->txcmpl_ring.napi);
	}

	/*
	 * Enable RxDesc Ring.
	 */
	data = edma_reg_read(EDMA_REG_RXDESC_CTRL(ppeds_node->rx_ring.ring_id));
	data |= EDMA_RXDESC_RX_EN;
	edma_reg_write(EDMA_REG_RXDESC_CTRL(ppeds_node->rx_ring.ring_id), data);

	/*
	 * Reset RxDesc disable Reg.
	 */
	data = edma_reg_read(EDMA_REG_RXDESC_DISABLE(ppeds_node->rx_ring.ring_id));
	data &= ~EDMA_RXDESC_RX_DISABLE;
	edma_reg_write(EDMA_REG_RXDESC_DISABLE(ppeds_node->rx_ring.ring_id), data);

	/*
	 * Enable RxFill Ring.
	 */
	data = edma_reg_read(EDMA_REG_RXFILL_RING_EN(ppeds_node->rxfill_ring.ring_id));
	data |= EDMA_RXFILL_RING_EN;
	edma_reg_write(EDMA_REG_RXFILL_RING_EN(ppeds_node->rxfill_ring.ring_id), data);

	/*
	 * Reset RxFill disable Reg.
	 */
	data = edma_reg_read(EDMA_REG_RXFILL_DISABLE(ppeds_node->rxfill_ring.ring_id));
	data &= ~EDMA_RXFILL_RING_DISABLE;
	edma_reg_write(EDMA_REG_RXFILL_DISABLE(ppeds_node->rxfill_ring.ring_id), data);

	/*
	 * Enable Tx Ring.
	 */
	data = edma_reg_read(EDMA_REG_TXDESC_CTRL(ppeds_node->tx_ring.id));
	data |= EDMA_TXDESC_TX_ENABLE;
	edma_reg_write(EDMA_REG_TXDESC_CTRL(ppeds_node->tx_ring.id), data);

	/*
	 * If the ring reset is supported,
	 * Enable the queues that are disabled at the time of inst stop.
	 */
	if (edma_dp_per_ring_reset_support()) {
		if (!edma_cfg_rx_ring_en_mapped_queues(egc, ppeds_node->rx_ring.ring_id, true)) {
			edma_err("%px: Failed to enable the queue", ppeds_node);
			return 0;
		}
	}

	write_lock_bh(&drv->lock);
	node_cfg->node_state = EDMA_PPEDS_NODE_STATE_START_DONE;
	write_unlock_bh(&drv->lock);
	ppeds_node->umac_reset_inprogress = info_hdl->umac_reset_inprogress;

	return 0;
}

/*
 * edma_ppeds_inst_stop()
 *	PPE-DS EDMA instance stop API
 */
void edma_ppeds_inst_stop(nss_dp_ppeds_handle_t *ppeds_handle, uint8_t intr_enable,
				struct nss_ppe_ds_ctx_info_handle *info_hdl)
{
	uint32_t data;
	struct edma_gbl_ctx *gbl_ctx = &edma_gbl_ctx;
	struct edma_ppeds_drv *drv = &gbl_ctx->ppeds_drv;
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);
	struct edma_ppeds_node_cfg *node_cfg = &(drv->ppeds_node_cfg[ppeds_node->db_idx]);

	ppeds_node->umac_reset_inprogress = info_hdl->umac_reset_inprogress;
	write_lock_bh(&drv->lock);
	if (node_cfg->node_state != EDMA_PPEDS_NODE_STATE_START_DONE) {
		edma_err("%px: Invalid node state: %d, PPE-DS stop failed\n", ppeds_node,
				node_cfg->node_state);
		write_unlock_bh(&drv->lock);
		return;
	}
	node_cfg->node_state = EDMA_PPEDS_NODE_STATE_STOP_IN_PROG;
	write_unlock_bh(&drv->lock);

	/*
	 * Disable TxDesc rings.
	 */
	data = edma_reg_read(EDMA_REG_TXDESC_CTRL(ppeds_node->tx_ring.id));
	data &= ~EDMA_TXDESC_TX_ENABLE;
	edma_reg_write(EDMA_REG_TXDESC_CTRL(ppeds_node->tx_ring.id), data);
	do {
		data = edma_reg_read(EDMA_REG_TXDESC_CTRL(ppeds_node->tx_ring.id));
		data &= EDMA_TXDESC_TX_ENABLE;
	} while (data);

	/*
	 * Reset the TX ring if Hardware support is present.
	 */
	if (edma_dp_per_ring_reset_support()) {
		edma_cfg_tx_ring_reset(&ppeds_node->tx_ring);

		/*
		 * Disable the PPE queues corresponding to RX ring to stop the incoming
		 * traffic on the ring.
		 */
		if (!edma_cfg_rx_ring_en_mapped_queues(gbl_ctx, ppeds_node->rx_ring.ring_id, false)) {
			edma_err("%px: Failed to disable the queue", ppeds_node);
			return;
		}
	}

	/*
	 * Clear enable bit, set disable bit and wait untill Rx Desc ring is disabled.
	 */
	data = edma_reg_read(EDMA_REG_RXDESC_CTRL(ppeds_node->rx_ring.ring_id));
	data &= ~EDMA_RXDESC_RX_EN;
	edma_reg_write(EDMA_REG_RXDESC_CTRL(ppeds_node->rx_ring.ring_id), data);

	data = edma_reg_read(EDMA_REG_RXDESC_DISABLE(ppeds_node->rx_ring.ring_id));
	data |= EDMA_RXDESC_RX_DISABLE;
	edma_reg_write(EDMA_REG_RXDESC_DISABLE(ppeds_node->rx_ring.ring_id), data);

	do {
		data = edma_reg_read(EDMA_REG_RXDESC_DISABLE_DONE(ppeds_node->rx_ring.ring_id));
	} while (!data);

	/*
	 * Reset the ring if Hardware support is present.
	 */
	if (edma_dp_per_ring_reset_support())
		edma_cfg_rx_ring_reset(&ppeds_node->rx_ring);

	/*
	 * Disable Tx complete interrupt and NAPI
	 */
	edma_reg_write(EDMA_REG_TX_INT_MASK(ppeds_node->txcmpl_ring.id),
			EDMA_MASK_INT_CLEAR);
	if (!ppeds_node->umac_reset_inprogress) {
		synchronize_irq(ppeds_node->txcmpl_intr);
		napi_disable(&ppeds_node->txcmpl_ring.napi);
	}

	/*
	 * Clear enable bit, set the disable bit and wait until the RxFill ring is disabled.
	 */
	data = edma_reg_read(EDMA_REG_RXFILL_RING_EN(ppeds_node->rxfill_ring.ring_id));
	data &= ~EDMA_RXFILL_RING_EN;
	edma_reg_write(EDMA_REG_RXFILL_RING_EN(ppeds_node->rxfill_ring.ring_id), data);

	data = edma_reg_read(EDMA_REG_RXFILL_DISABLE(ppeds_node->rxfill_ring.ring_id));
	data |= EDMA_RXFILL_RING_DISABLE;
	edma_reg_write(EDMA_REG_RXFILL_DISABLE(ppeds_node->rxfill_ring.ring_id), data);

	do {
		data = edma_reg_read(EDMA_REG_RXFILL_DISABLE_DONE(ppeds_node->rxfill_ring.ring_id));
	} while (!data);

	/*
	 * Disable Rxfill interrupt and NAPI
	 */
	edma_reg_write(EDMA_REG_RXFILL_INT_MASK(ppeds_node->rxfill_ring.ring_id),
			EDMA_MASK_INT_CLEAR);
	if (!ppeds_node->umac_reset_inprogress) {
		synchronize_irq(ppeds_node->rxfill_intr);
		napi_disable(&ppeds_node->rxfill_ring.napi);
	}

	/*
	 * Wait for 5ms and then clean the tx complete ring
	 */
	mdelay(5);
	edma_ppeds_tx_complete(ppeds_node->txcmpl_ring.count, &ppeds_node->txcmpl_ring);

	write_lock_bh(&drv->lock);
	node_cfg->node_state = EDMA_PPEDS_NODE_STATE_STOP_DONE;
	write_unlock_bh(&drv->lock);
}

/*
 * edma_ppeds_inst_free()
 *	PPE-DS EDMA instance free API
 */
void edma_ppeds_inst_free(nss_dp_ppeds_handle_t *ppeds_handle)
{
	struct edma_ppeds_drv *drv = &edma_gbl_ctx.ppeds_drv;
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);
	struct edma_ppeds_node_cfg *node_cfg = &(drv->ppeds_node_cfg[ppeds_node->db_idx]);

	write_lock_bh(&drv->lock);
	if (node_cfg->node_state != EDMA_PPEDS_NODE_STATE_STOP_DONE) {
		edma_err("%px: Invalid node state: %d, PPE-DS free failed\n", ppeds_node,
				node_cfg->node_state);
		write_unlock_bh(&drv->lock);
		return;
	}
	node_cfg->node_state = EDMA_PPEDS_NODE_STATE_FREE_IN_PROG;
	write_unlock_bh(&drv->lock);

	kfree(ppeds_handle->rx_fill_arr);
	ppeds_handle->rx_fill_arr = NULL;

	kfree(ppeds_handle->tx_cmpl_arr);
	ppeds_handle->tx_cmpl_arr= NULL;

	irq_clear_status_flags(ppeds_node->rxdesc_intr, IRQ_DISABLE_UNLAZY);
	free_irq(ppeds_node->rxdesc_intr,
			(void *)&ppeds_node->rx_ring);
	netif_napi_del(&ppeds_node->rx_ring.napi);

	irq_clear_status_flags(ppeds_node->txcmpl_intr, IRQ_DISABLE_UNLAZY);
	free_irq(ppeds_node->txcmpl_intr,
			(void *)&ppeds_node->txcmpl_ring);
	netif_napi_del(&ppeds_node->txcmpl_ring.napi);

	irq_clear_status_flags(ppeds_node->rxfill_intr, IRQ_DISABLE_UNLAZY);
	free_irq(ppeds_node->rxfill_intr,
			(void *)&ppeds_node->rxfill_ring);
	netif_napi_del(&ppeds_node->rxfill_ring.napi);

	edma_ppeds_rx_fill_ring_free(&ppeds_node->rxfill_ring);
	edma_ppeds_tx_cmpl_ring_free(&ppeds_node->txcmpl_ring);

	kfree(ppeds_node);

	/*
	 * Remove from DB
	 */
	write_lock_bh(&drv->lock);
	node_cfg->ppeds_db = NULL;
	node_cfg->node_state = EDMA_PPEDS_NODE_STATE_AVAIL;
	write_unlock_bh(&drv->lock);
}

/*
 * edma_ppeds_inst_alloc()
 *	PPE-DS EDMA instance allocation API
 */
nss_dp_ppeds_handle_t *edma_ppeds_inst_alloc(const struct nss_dp_ppeds_cb *ops, size_t priv_size)
{
	struct edma_ppeds *ppeds_node;
	uint32_t i;
	struct edma_ppeds_drv *drv = &edma_gbl_ctx.ppeds_drv;
	int size = priv_size + sizeof(struct edma_ppeds);

	if (!ops || !ops->rx || !ops->rx_fill || !ops->rx_release
			|| !ops->tx_cmpl) {
		edma_err("Invalid PPE-DS operations\n");
		return NULL;
	}

	edma_debug("ppeds node size %lu total size %d\n", sizeof(struct edma_ppeds),  size);
	ppeds_node = (struct edma_ppeds *)kzalloc(size, GFP_KERNEL);
	if (!ppeds_node) {
		edma_err("Cannot allocate memory for ppeds node\n");
		return NULL;
	}

	ppeds_node->ops = ops;

	/*
	 * Add to Database
	 */
	write_lock_bh(&drv->lock);
	for (i = 0; i < drv->num_nodes; i++) {
		if (drv->ppeds_node_cfg[i].node_state == EDMA_PPEDS_NODE_STATE_AVAIL) {
			break;
		}
	}

	if (i == drv->num_nodes) {
		write_unlock_bh(&drv->lock);
		edma_err("Cannot get a free edma PPE-DS resource set\n");
		kfree(ppeds_node);
		return NULL;
	}

	drv->ppeds_node_cfg[i].ppeds_db = ppeds_node;
	ppeds_node->rxfill_ring.ring_id = drv->ppeds_node_cfg[i].node_map[EDMA_PPEDS_ENTRY_RXFILL_IDX];
	ppeds_node->txcmpl_ring.id = drv->ppeds_node_cfg[i].node_map[EDMA_PPEDS_ENTRY_TXCMPL_IDX];
	ppeds_node->rx_ring.ring_id = drv->ppeds_node_cfg[i].node_map[EDMA_PPEDS_ENTRY_RX_IDX];
	ppeds_node->tx_ring.id = drv->ppeds_node_cfg[i].node_map[EDMA_PPEDS_ENTRY_TX_IDX];
	ppeds_node->ppe_qid = drv->ppeds_node_cfg[i].node_map[EDMA_PPEDS_ENTRY_QID_START_IDX];
	ppeds_node->ppe_num_queues = drv->ppeds_node_cfg[i].node_map[EDMA_PPEDS_ENTRY_QID_COUNT_IDX];
	ppeds_node->txcmpl_intr = drv->ppeds_node_cfg[i].irq_map[EDMA_PPEDS_TXCOMP_IRQ_IDX];
	ppeds_node->rxfill_intr = drv->ppeds_node_cfg[i].irq_map[EDMA_PPEDS_RXFILL_IRQ_IDX];
	ppeds_node->rxdesc_intr = drv->ppeds_node_cfg[i].irq_map[EDMA_PPEDS_RXDESC_IRQ_IDX];
	ppeds_node->db_idx = i;
	edma_debug("PPE-DS node(%d): Rxfill ring: %d, Tx complete ring: %d,"
			" Rx ring: %d, Tx ring: %d, Queue start id: %d,"
			" Queue count: %d, Tx complete interrupt: %d,"
			" Rxfill interrupt: %d, Rx interrupt: %d\n", i,
			ppeds_node->rxfill_ring.ring_id, ppeds_node->txcmpl_ring.id,
			ppeds_node->rx_ring.ring_id, ppeds_node->tx_ring.id,
			ppeds_node->ppe_qid, ppeds_node->ppe_num_queues,
			ppeds_node->txcmpl_intr, ppeds_node->rxfill_intr,
			ppeds_node->rxdesc_intr);
	drv->ppeds_node_cfg[i].node_state = EDMA_PPEDS_NODE_STATE_ALLOC;
	write_unlock_bh(&drv->lock);

	return &ppeds_node->ppeds_handle;
}

/*
 * edma_ppeds_deinit()
 *	PPEDS deinit
 */
void edma_ppeds_deinit(struct edma_ppeds_drv *drv)
{
	uint32_t i;

	if (edma_ppeds_tx_ring_sec_mem) {
		kfree(edma_ppeds_tx_ring_sec_mem);
		edma_ppeds_tx_ring_sec_mem = NULL;
	}

	if (edma_ppeds_rx_ring_sec_mem) {
		kfree(edma_ppeds_rx_ring_sec_mem);
		edma_ppeds_rx_ring_sec_mem = NULL;
	}

	for (i = 0; i < EDMA_PPEDS_MAX_NODES; i++) {
		drv->ppeds_node_cfg[i].ppeds_db = NULL;
		drv->ppeds_node_cfg[i].node_state = EDMA_PPEDS_NODE_STATE_AVAIL;
	}
}

/*
 * edma_ppeds_get_queue_start_for_node()
 *	PPEDS get queue_start for a particular node id.
 */
static int edma_ppeds_get_queue_start_for_node(struct edma_ppeds_drv *drv, int i)
{
	return ((uint8_t) drv->ppeds_node_cfg[i].node_map[EDMA_PPEDS_ENTRY_QID_START_IDX]);
}

/*
 * edma_ppeds_init()
 *	PPEDS init
 */
int edma_ppeds_init(struct edma_ppeds_drv *drv)
{
	uint32_t i;
	uint8_t queue_start;

	rwlock_init(&drv->lock);

	for (i = 0; i < EDMA_PPEDS_MAX_NODES; i++) {
		drv->ppeds_node_cfg[i].ppeds_db = NULL;
		if (i < drv->num_nodes) {
			int j;
			for (j = 0; j < EDMA_PPEDS_NUM_ENTRY; j++) {
				drv->ppeds_node_cfg[i].node_map[j] =
					 edma_gbl_ctx.ppeds_node_map[i][j];
			}

			queue_start = edma_ppeds_get_queue_start_for_node(drv, i);
			ppe_drv_ds_map_node_to_queue(i, queue_start);

			drv->ppeds_node_cfg[i].node_state = EDMA_PPEDS_NODE_STATE_AVAIL;
			continue;
		}
		drv->ppeds_node_cfg[i].node_state = EDMA_PPEDS_NODE_STATE_NOT_AVAIL;
	}

	return 0;
}

/*
 * edma_ppeds_service_status_update()
 *	Set/Unset EDMA ring usage service and notify NAPI done.
 */
void edma_ppeds_service_status_update(nss_dp_ppeds_handle_t *ppeds_handle, bool enable)
{
	struct edma_ppeds *ppeds_node = container_of(ppeds_handle, struct edma_ppeds, ppeds_handle);
	if (enable) {
		set_bit(EDMA_PPEDS_SERVICE_STOP_BIT, &ppeds_node->service_running);
	} else {
		clear_bit(EDMA_PPEDS_SERVICE_STOP_BIT, &ppeds_node->service_running);

		if ((!ppeds_node->service_running) &&
				(ppeds_node->ops->notify_napi_done)) {
			ppeds_node->ops->notify_napi_done(&ppeds_node->ppeds_handle);
		}
	}
}

/*
 * edma_ppeds_ops
 *	PPE-DS operations
 */
struct nss_dp_ppeds_ops edma_ppeds_ops = {
	.alloc			=	edma_ppeds_inst_alloc,
	.reg			=	edma_ppeds_inst_register,
	.start 			=	edma_ppeds_inst_start,
	.refill			=	edma_ppeds_inst_refill,
	.stop			=	edma_ppeds_inst_stop,
	.free			=	edma_ppeds_inst_free,
	.get_queues		=	edma_ppeds_get_ppe_queues,
	.set_rx_cons_idx	=	edma_ppeds_set_rx_cons_idx,
	.set_tx_prod_idx	=	edma_ppeds_set_tx_prod_idx,
	.get_tx_cons_idx	=	edma_ppeds_get_tx_cons_idx,
	.get_rx_prod_idx	=	edma_ppeds_get_rx_prod_idx,
	.get_rxfill_cons_idx	=	edma_ppeds_get_rxfill_cons_idx,
	.set_rxfill_prod_idx	=	edma_ppeds_set_rxfill_prod_idx,
	.enable_rx_reap_intr	=	edma_ppeds_enable_rx_reap_intr,
	.service_status_update	=	edma_ppeds_service_status_update,
};
