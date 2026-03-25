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

#include <linux/debugfs.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/reset.h>
#include "edma.h"
#include "edma_cfg_rx.h"
#include "edma_regs.h"
#include "edma_debug.h"
#include "nss_dp_dev.h"

uint32_t edma_cfg_rx_fc_enable = EDMA_RX_FC_ENABLE;
uint32_t edma_cfg_rx_queue_tail_drop_enable = EDMA_RX_QUEUE_TAIL_DROP_ENABLE;
uint32_t edma_cfg_rx_rps_num_cores = NR_CPUS;
uint32_t edma_cfg_rx_sec_desc_inval = 0;
uint32_t edma_cfg_rx_rps_bitmap_cores = EDMA_RX_DEFAULT_BITMAP;
extern uint32_t nss_dp_capwap_vp_rx_core;

/*
 * Rx ring queue offset
 */
#define EDMA_QUEUE_OFFSET(q_id)	(q_id / EDMA_MAX_PRI_PER_CORE)

/*
 * Rx EDMA maximum queue supported
 */
#define EDMA_CPU_PORT_QUEUE_MAX(queue_start)	queue_start + (EDMA_MAX_PRI_PER_CORE * NR_CPUS) - 1

/*
 * edma_cfg_rx_fill_ring_cleanup()
 *	Cleanup resources for one RxFill ring
 *
 * API expects ring to be disabled by caller
 */
static void edma_cfg_rx_fill_ring_cleanup(struct edma_gbl_ctx *egc,
				struct edma_rxfill_ring *rxfill_ring)
{
	uint16_t cons_idx, curr_idx, cons_idx_prev;
	uint32_t reg_data;

	/*
	 * Get RXFILL ring producer index
	 */
	cons_idx_prev = curr_idx = rxfill_ring->prod_idx & EDMA_RXFILL_PROD_IDX_MASK;

	/*
	 * Get RXFILL ring consumer index
	 */
	reg_data = edma_reg_read(EDMA_REG_RXFILL_CONS_IDX(rxfill_ring->ring_id));
	cons_idx = reg_data & EDMA_RXFILL_CONS_IDX_MASK;

	while (curr_idx != cons_idx) {
		struct sk_buff *skb;
		struct edma_rxfill_desc *rxfill_desc;

		/*
		 * Get RXFILL descriptor
		 */
		rxfill_desc = EDMA_RXFILL_DESC(rxfill_ring, cons_idx);

		cons_idx = (cons_idx + 1) & EDMA_RX_RING_SIZE_MASK;

		/*
		 * Get skb from opaque
		 */
		skb = (struct sk_buff *)EDMA_RXFILL_OPAQUE_GET(rxfill_desc);
		if (unlikely(!skb)) {
			edma_warn("Empty skb reference at index:%d\n",
					cons_idx);
			continue;
		}
		dev_kfree_skb_any(skb);
	}

	edma_reg_write(EDMA_REG_RXFILL_PROD_IDX(rxfill_ring->ring_id), cons_idx_prev);

	/*
	 * Free RXFILL ring descriptors
	 */
	dma_free_coherent(&egc->pdev->dev,
				(sizeof(struct edma_rxfill_desc)
				* rxfill_ring->count),
				rxfill_ring->desc, rxfill_ring->dma);
	rxfill_ring->desc = NULL;
	rxfill_ring->dma = (dma_addr_t)0;
}

/*
 * edma_cfg_rx_fill_ring_setup()
 *	Setup resources for one RxFill ring
 */
static int edma_cfg_rx_fill_ring_setup(struct edma_rxfill_ring *rxfill_ring)
{
	struct platform_device *pdev = edma_gbl_ctx.pdev;

	/*
	 * Allocate RxFill ring descriptors
	 */
	rxfill_ring->desc = dma_alloc_coherent(&pdev->dev,
				(sizeof(struct edma_rxfill_desc)
				* rxfill_ring->count),
				&rxfill_ring->dma, GFP_KERNEL | __GFP_ZERO);
	return 0;
}

/*
 * edma_cfg_rx_desc_ring_setup()
 *	Setup resources for one RxDesc ring
 */
static int edma_cfg_rx_desc_ring_setup(struct edma_rxdesc_ring *rxdesc_ring)
{
	/*
	 * Allocate RxDesc ring descriptors
	 */
	rxdesc_ring->pdesc = kmalloc(roundup((sizeof(struct edma_rxdesc_desc) * rxdesc_ring->count),
						SMP_CACHE_BYTES), GFP_KERNEL | __GFP_ZERO);
	if (!rxdesc_ring->pdesc) {
		edma_err("Descriptor alloc for RXDESC ring %u failed\n",
							rxdesc_ring->ring_id);
		return -ENOMEM;
	}

	rxdesc_ring->pdma = (dma_addr_t)virt_to_phys(rxdesc_ring->pdesc);

	/*
	 * Allocate secondary RxDesc ring descriptors
	 */
	rxdesc_ring->sdesc = kmalloc(roundup((sizeof(struct edma_rxdesc_sec_desc) * rxdesc_ring->count),
						SMP_CACHE_BYTES), GFP_KERNEL | __GFP_ZERO);
	if (!rxdesc_ring->sdesc) {
		edma_err("Descriptor alloc for secondary RX ring %u failed\n",
							rxdesc_ring->ring_id);
		kfree(rxdesc_ring->pdesc);
		rxdesc_ring->pdesc = NULL;
		rxdesc_ring->pdma = (dma_addr_t)0;
		return -ENOMEM;
	}

	rxdesc_ring->sdma = (dma_addr_t)virt_to_phys(rxdesc_ring->sdesc);

	return 0;
}

/*
 * edma_cfg_rx_desc_ring_cleanup()
 *	Cleanup resources for RxDesc ring
 *
 * API expects ring to be disabled by caller
 */
static void edma_cfg_rx_desc_ring_cleanup(struct edma_gbl_ctx *egc,
				struct edma_rxdesc_ring *rxdesc_ring)
{
	uint16_t prod_idx, cons_idx;

	/*
	 * Get Rxdesc consumer & producer indices
	 */
	cons_idx = rxdesc_ring->cons_idx & EDMA_RXDESC_CONS_IDX_MASK;

	prod_idx = edma_reg_read(EDMA_REG_RXDESC_PROD_IDX(rxdesc_ring->ring_id))
					& EDMA_RXDESC_PROD_IDX_MASK;

	/*
	 * Free any buffers assigned to any descriptors
	 */
	while (cons_idx != prod_idx) {
		struct sk_buff *skb;
		struct edma_rxdesc_desc *rxdesc_desc =
			EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx);

		/*
		 * Update consumer index
		 */
		cons_idx = (cons_idx + 1) & EDMA_RX_RING_SIZE_MASK;

		/*
		 * Get opaque from RXDESC
		 */
		skb = (struct sk_buff *)EDMA_RXDESC_OPAQUE_GET(rxdesc_desc);
		if (unlikely(!skb)) {
			edma_warn("Empty skb reference at index:%d\n",
								cons_idx);
			continue;
		}
		dev_kfree_skb_any(skb);
	}

	/*
	 * Update the consumer index to keep hardware upto date with latest state.
	 */
	edma_reg_write(EDMA_REG_RXDESC_CONS_IDX(rxdesc_ring->ring_id), cons_idx);

	/*
	 * Free RXDESC ring descriptors
	 */
	kfree(rxdesc_ring->pdesc);
	rxdesc_ring->pdesc = NULL;
	rxdesc_ring->pdma = (dma_addr_t)0;

	/*
	 * TODO:
	 * Free any buffers assigned to any secondary ring descriptors
	 */
	kfree(rxdesc_ring->sdesc);
	rxdesc_ring->sdesc = NULL;
	rxdesc_ring->sdma = (dma_addr_t)0;
}

/*
 * edma_cfg_rx_desc_rings_reset_queue_mapping()
 *	API to reset Rx descriptor rings to PPE queues mapping
 */
static int32_t edma_cfg_rx_desc_rings_reset_queue_mapping(void)
{
	fal_queue_bmp_t queue_bmp = {0};
	int32_t i;

	for (i = 0; i < EDMA_MAX_RXDESC_RINGS; i++) {
		if (fal_edma_ring_queue_map_set(EDMA_SWITCH_DEV_ID, i, &queue_bmp) != SW_OK) {
			edma_err("Error in unmapping rxdesc ring %d to PPE queue mapping to"
					" disable its backpressure configuration\n", i);
			return -1;
		}
	}

	return 0;
}

/*
 * edma_cfg_rx_desc_ring_reset_queue_priority()
 *	API to reset the priority for PPE queues mapped to Rx rings
 */
static int32_t edma_cfg_rx_desc_ring_reset_queue_priority(struct edma_gbl_ctx *egc,
				uint32_t rxdesc_ring_idx)
{
	fal_qos_scheduler_cfg_t qsch;
	uint32_t i, queue_id, port_id;

	for (i = 0; i < EDMA_MAX_PRI_PER_CORE; i++) {
		queue_id = egc->rx_ring_queue_map[i][rxdesc_ring_idx];

		memset(&qsch, 0, sizeof(fal_qos_scheduler_cfg_t));
		if (fal_queue_scheduler_get(EDMA_SWITCH_DEV_ID, queue_id,
					EDMA_PPE_QUEUE_LEVEL, &port_id, &qsch) != SW_OK) {
			edma_err("Error in getting %u queue's priority information\n", queue_id);
			return -1;
		}

		qsch.e_pri = i;
		qsch.c_pri = i;

		if (fal_queue_scheduler_set(EDMA_SWITCH_DEV_ID, queue_id,
					EDMA_PPE_QUEUE_LEVEL, port_id, &qsch) != SW_OK) {
			edma_err("Error in resetting %u queue's priority\n", queue_id);
			return -1;
		}
	}

	return 0;
}

/*
 * edma_cfg_rx_desc_ring_reset_queue_config()
 *	API to reset the Rx descriptor rings configurations
 */
static int32_t edma_cfg_rx_desc_ring_reset_queue_config(struct edma_gbl_ctx *egc)
{
	int32_t i;

	if (unlikely(egc->num_rxdesc_rings > NR_CPUS)) {
		edma_err("Invalid count of rxdesc rings: %d\n", egc->num_rxdesc_rings);
		return -1;
	}

	/*
	 * Unmap Rxdesc ring to PPE queue mapping to reset its backpressure configuration
	 */
	if (edma_cfg_rx_desc_rings_reset_queue_mapping()) {
		edma_err("Error in resetting Rx desc ring backpressure configurations\n");
		return -1;
	}

	/*
	 * Reset the priority for PPE queues mapped to Rx rings
	 */
	for (i = 0; i < egc->num_rxdesc_rings; i++) {
		if(edma_cfg_rx_desc_ring_reset_queue_priority(egc, i)) {
			edma_err("Error in resetting ring:%d queue's priority\n",
					 i + egc->rxdesc_ring_start);
			return -1;
		}
	}

	return 0;
}

/*
 * edma_cfg_rx_desc_ring_to_queue_mapping()
 *	API to map Rx descriptor rings to PPE queue for backpressure
 */
static void edma_cfg_rx_desc_ring_to_queue_mapping(uint32_t enable)
{
	uint32_t i, j;
	sw_error_t ret;
	fal_queue_bmp_t queue_bmp = {0};

	/*
	 * Rxdesc ring to PPE queue mapping
	 */
	for (i = 0; i < edma_gbl_ctx.num_rxdesc_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;

		rxdesc_ring = &edma_gbl_ctx.rxdesc_rings[i];
		if (enable) {
			memcpy(queue_bmp.bmp, edma_gbl_ctx.rxdesc_ring_to_queue_bm[i],
					sizeof(uint32_t) * EDMA_RING_MAPPED_QUEUE_BM_WORD_COUNT);
		}

		ret = fal_edma_ring_queue_map_set(0, rxdesc_ring->ring_id, &queue_bmp);
		if (ret != SW_OK) {
			edma_err("Error in configuring Rx ring to PPE queue mapping."
					" ret: %d, id: %d, queue: %d\n",
					ret, rxdesc_ring->ring_id, queue_bmp.bmp[0]);
			if (edma_cfg_rx_desc_rings_reset_queue_mapping()) {
				edma_err("Error in resetting Rx desc ring backpressure configurations\n");
			}
			return;
		}

		edma_info("Rx desc ring %d to PPE queue mapping for backpressure:\n",
				rxdesc_ring->ring_id);
		for (j = 0; j < EDMA_RING_MAPPED_QUEUE_BM_WORD_COUNT; j++) {
			edma_info("\tPPE queue bitmap[%d]: %0x\n", j, queue_bmp.bmp[j]);
		}
	}
}

#if defined(NSS_DP_POINT_OFFLOAD)

/*
 * edma_cfg_rx_desc_point_offload_rings_reset_queue_mapping()
 *	API to reset Rx descriptor rings to PPE queues mapping
 */
static int32_t edma_cfg_rx_desc_point_offload_rings_reset_queue_mapping(struct edma_gbl_ctx *egc)
{
	fal_queue_bmp_t queue_bmp = {0};
	int32_t i = egc->rxdesc_point_offload_ring;

	if (fal_edma_ring_queue_map_set(EDMA_SWITCH_DEV_ID, i, &queue_bmp) != SW_OK) {
		edma_err("Error in unmapping rxdesc point offload ring %d to PPE queue mapping to"
				" disable its backpressure configuration\n", i);
		return -1;
	}

	return 0;
}

/*
 * edma_cfg_rx_desc_point_offload_ring_reset_queue_priority()
 *	API to reset the priority for PPE queues mapped to Rx rings
 */
static int32_t edma_cfg_rx_desc_point_offload_ring_reset_queue_priority(struct edma_gbl_ctx *egc)
{
	fal_qos_scheduler_cfg_t qsch;
	uint32_t queue_id, bit_set, port_id, cur_queue_word;
	int32_t i = egc->rxdesc_point_offload_ring;

	for (i = 0; i < EDMA_RING_MAPPED_QUEUE_BM_WORD_COUNT; i++) {
		cur_queue_word = egc->rxdesc_point_offload_ring_to_queue_bm[i];
		if (cur_queue_word == 0) {
			continue;
		}

		do {
			bit_set = ffs((uint32_t)cur_queue_word);
			queue_id = (((i * EDMA_BITS_IN_WORD) + bit_set) - 1);
			memset(&qsch, 0, sizeof(fal_qos_scheduler_cfg_t));
			if (fal_queue_scheduler_get(EDMA_SWITCH_DEV_ID, queue_id,
						EDMA_PPE_QUEUE_LEVEL, &port_id, &qsch) != SW_OK) {
				edma_err("Error in getting %u queue's priority information\n", queue_id);
				return -1;
			}

			/*
			 * Configure the default queue priority.
			 * In IPQ95xx, currently single queue is being mapped to the
			 * single Rx descriptor ring and each ring will be processed
			 * on the separate core. Therefore assigning same priority to
			 * all these mapped queues.
			 */
			qsch.e_pri = EDMA_RX_DEFAULT_QUEUE_PRI;
			qsch.c_pri = EDMA_RX_DEFAULT_QUEUE_PRI;
			if (fal_queue_scheduler_set(EDMA_SWITCH_DEV_ID, queue_id,
						EDMA_PPE_QUEUE_LEVEL, port_id, &qsch) != SW_OK) {
				edma_err("Error in resetting %u queue's priority\n", queue_id);
				return -1;
			}

			cur_queue_word &= ~(1 << (bit_set - 1));
		} while (cur_queue_word);
	}

	return 0;
}
/*
 * edma_cfg_rx_desc_point_offload_ring_reset_queue_config()
 *	API to reset the Rx descriptor rings configurations
 */
static int32_t edma_cfg_rx_desc_point_offload_ring_reset_queue_config(struct edma_gbl_ctx *egc)
{
	/*
	 * Unmap Rxdesc ring to PPE queue mapping to reset its backpressure configuration
	 */
	if (edma_cfg_rx_desc_point_offload_rings_reset_queue_mapping(egc)) {
		edma_err("Error in resetting Rx desc point offload ring backpressure configurations\n");
		return -1;
	}

	/*
	 * Reset the priority for PPE queues mapped to Rx rings
	 */
	if(edma_cfg_rx_desc_point_offload_ring_reset_queue_priority(egc)) {
		return -1;
	}

	return 0;
}

/*
 * edma_cfg_rx_desc_point_offload_ring_to_queue_mapping()
 *	API to map Rx descriptor rings to PPE queue for backpressure
 */
static void edma_cfg_rx_desc_point_offload_ring_to_queue_mapping(struct edma_gbl_ctx *egc)
{
	sw_error_t ret;
	fal_queue_bmp_t queue_bmp = {0};
	unsigned int pri_idx = 0;
	uint32_t word_idx = 0;
	uint32_t local_bmp[EDMA_RING_MAPPED_QUEUE_BM_WORD_COUNT] = {0};
	unsigned int queue_id = egc->point_offload_queue;

	/*
	 * Rxdesc ring to PPE queue mapping
	 */
	for (pri_idx = 0; pri_idx < EDMA_MAX_PRI_PER_CORE; pri_idx++, queue_id++) {
		word_idx = (queue_id / EDMA_BITS_IN_WORD);
		local_bmp[word_idx] |= 1 << queue_id;
		edma_debug("Queue_id: %d, word_idx: %d\n", queue_id, word_idx);
	}

	memcpy(queue_bmp.bmp, local_bmp, sizeof(uint32_t) * EDMA_RING_MAPPED_QUEUE_BM_WORD_COUNT);

	ret = fal_edma_ring_queue_map_set(0, egc->rxdesc_point_offload_ring, &queue_bmp);
	if (ret != SW_OK) {
		edma_err("Error in configuring Rx point offload ring to PPE queue mapping." " ret: %d, id: %d, queue: %d\n",
					ret, egc->rxdesc_point_offload_ring, queue_bmp.bmp[0]);
		if (edma_cfg_rx_desc_rings_reset_queue_mapping()) {
			edma_err("Error in resetting Rx desc point offload ring backpressure configurations\n");
		}
		return;
	}
}

/*
 * edma_cfg_rx_fill_point_offload_ring_flow_control()
 *	Configure Rx fill point offload ring flow control configuration
 */
static void edma_cfg_rx_fill_point_offload_ring_flow_control(struct edma_gbl_ctx *egc, uint32_t threshold_xoff, uint32_t threshold_xon)
{
	uint32_t data;

	data = (threshold_xoff & EDMA_RXFILL_FC_XOFF_THRE_MASK) << EDMA_RXFILL_FC_XOFF_THRE_SHIFT;
	data |= ((threshold_xon & EDMA_RXFILL_FC_XON_THRE_MASK) << EDMA_RXFILL_FC_XON_THRE_SHIFT);
	edma_reg_write(EDMA_REG_RXFILL_FC_THRE(egc->rxfill_point_offload_ring), data);
}

/*
 * edma_cfg_rx_desc_point_offload_ring_flow_control()
 *	Configure Rx descriptor ring flow control configuration
 */
static void edma_cfg_rx_desc_point_offload_ring_flow_control(struct edma_gbl_ctx *egc, uint32_t threshold_xoff, uint32_t threshold_xon)
{
	uint32_t data;

	data = (threshold_xoff & EDMA_RXDESC_FC_XOFF_THRE_MASK) << EDMA_RXDESC_FC_XOFF_THRE_SHIFT;
	data |= ((threshold_xon & EDMA_RXDESC_FC_XON_THRE_MASK) << EDMA_RXDESC_FC_XON_THRE_SHIFT);
	edma_reg_write(EDMA_REG_RXDESC_FC_THRE(egc->rxdesc_point_offload_ring), data);
}
#endif

/*
 * edma_cfg_rx_fill_ring_flow_control()
 *	Configure Rx fill ring flow control configuration
 */
static void edma_cfg_rx_fill_ring_flow_control(uint32_t threshold_xoff, uint32_t threshold_xon)
{
	uint32_t data, i;

	data = (threshold_xoff & EDMA_RXFILL_FC_XOFF_THRE_MASK) << EDMA_RXFILL_FC_XOFF_THRE_SHIFT;
	data |= ((threshold_xon & EDMA_RXFILL_FC_XON_THRE_MASK) << EDMA_RXFILL_FC_XON_THRE_SHIFT);

	for (i = 0; i < edma_gbl_ctx.num_rxfill_rings; i++) {
		struct edma_rxfill_ring *rxfill_ring;

		rxfill_ring = &edma_gbl_ctx.rxfill_rings[i];
		edma_reg_write(EDMA_REG_RXFILL_FC_THRE(rxfill_ring->ring_id), data);
	}
}

/*
 * edma_cfg_rx_desc_ring_flow_control()
 *	Configure Rx descriptor ring flow control configuration
 */
static void edma_cfg_rx_desc_ring_flow_control(uint32_t threshold_xoff, uint32_t threshold_xon)
{
	uint32_t data, i;

	data = (threshold_xoff & EDMA_RXDESC_FC_XOFF_THRE_MASK) << EDMA_RXDESC_FC_XOFF_THRE_SHIFT;
	data |= ((threshold_xon & EDMA_RXDESC_FC_XON_THRE_MASK) << EDMA_RXDESC_FC_XON_THRE_SHIFT);
	for (i = 0; i < edma_gbl_ctx.num_rxdesc_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;

		rxdesc_ring = &edma_gbl_ctx.rxdesc_rings[i];
		edma_reg_write(EDMA_REG_RXDESC_FC_THRE(rxdesc_ring->ring_id), data);
	}
}

/*
 * edma_cfg_rx_mapped_queue_ac_fc_configure()
 *	API to configure Rx descriptor mapped queue's AC FC threshold
 *
 * This will enable queue tail drop on the PPE queues mapped to Rx rings.
 */
static int32_t edma_cfg_rx_mapped_queue_ac_fc_configure(uint16_t threshold,
				uint32_t enable)
{
	uint32_t i, j, queue_id;
	fal_ac_dynamic_threshold_t cfg;
	fal_ac_obj_t obj;
	fal_ac_ctrl_t ac_ctrl;
	bool is_enable = (enable ? true: false);

	for (i = 0; i < edma_gbl_ctx.num_rxdesc_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;

		rxdesc_ring = &edma_gbl_ctx.rxdesc_rings[i];

		for (j = 0; j < EDMA_MAX_PRI_PER_CORE; j++) {
			queue_id = edma_gbl_ctx.rx_ring_queue_map[j][i];

			/*
			 * Configure the mapped queues AC FC configuration threshold
			 */
			memset(&cfg, 0, sizeof(fal_ac_dynamic_threshold_t));
			if (fal_ac_dynamic_threshold_get(EDMA_SWITCH_DEV_ID,
				queue_id, &cfg) != SW_OK) {
				edma_err("Error in getting %d queue's AC FC threshold\n",
						queue_id);
				return -1;
			}

			/*
			 * TODO:
			 * Check if the threshold configuration can be done
			 * once the time of initialization and then later the AC
			 * FC configuration can be enabled/disabled from this API
			 * or not.
			 */
			cfg.ceiling = threshold;
			if (fal_ac_dynamic_threshold_set(EDMA_SWITCH_DEV_ID,
					queue_id, &cfg) != SW_OK) {
				edma_err("Error in configuring queue(%d) threshold"
						" value:%d\n", queue_id, threshold);
				return -1;
			}

			/*
			 * Enable/disable the state of mapped queues AC FC configuration
			 */
			memset(&obj, 0, sizeof(fal_ac_obj_t));
			memset(&ac_ctrl, 0, sizeof(fal_ac_ctrl_t));
			obj.type = FAL_AC_QUEUE;
			obj.obj_id = queue_id;
			if (fal_ac_ctrl_get(EDMA_SWITCH_DEV_ID, &obj, &ac_ctrl) != SW_OK) {
				edma_err("Error in getting %d queue's AC configuration\n",
						obj.obj_id);
				return -1;
			}

			ac_ctrl.ac_fc_en = is_enable;
			if (fal_ac_ctrl_set(EDMA_SWITCH_DEV_ID, &obj, &ac_ctrl) != SW_OK) {
				edma_err("Error in changing queue's ac_fc state:%d"
						" for queue:%d", is_enable, obj.obj_id);
				return -1;
			}
		}
	}

	return 0;
}

/*
 * edma_cfg_rx_desc_ring_configure()
 *	Configure one RxDesc ring in EDMA HW
 */
static void edma_cfg_rx_desc_ring_configure(struct edma_rxdesc_ring *rxdesc_ring)
{
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	uint32_t data;

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
	 * Validate mitigation timer value
	 */
	if ((nss_dp_rx_mitigation_timer < EDMA_RX_MITIGATION_TIMER_MIN) ||
			(nss_dp_rx_mitigation_timer > EDMA_RX_MITIGATION_TIMER_MAX)) {
		edma_err("Invalid Rx mitigation timer configured:%d for ring:%d."
				" Using the default timer value:%d\n",
				nss_dp_rx_mitigation_timer, rxdesc_ring->ring_id,
				NSS_DP_RX_MITIGATION_TIMER_DEF);
		nss_dp_rx_mitigation_timer = NSS_DP_RX_MITIGATION_TIMER_DEF;
	}

	/*
	 * Validate mitigation packet count value
	 */
	if ((nss_dp_rx_mitigation_pkt_cnt < EDMA_RX_MITIGATION_PKT_CNT_MIN) ||
			(nss_dp_rx_mitigation_pkt_cnt > EDMA_RX_MITIGATION_PKT_CNT_MAX)) {
		edma_err("Invalid Rx mitigation packet count configured:%d for ring:%d."
				" Using the default packet counter value:%d\n",
				nss_dp_rx_mitigation_timer, rxdesc_ring->ring_id,
				NSS_DP_RX_MITIGATION_PKT_CNT_DEF);
		nss_dp_rx_mitigation_pkt_cnt = NSS_DP_RX_MITIGATION_PKT_CNT_DEF;
	}

	/*
	 * Configure the Mitigation timer
	 */
	data = MICROSEC_TO_TIMER_UNIT(nss_dp_rx_mitigation_timer, egc->edma_timer_rate);
	data = ((data & EDMA_RX_MOD_TIMER_INIT_MASK)
			<< EDMA_RX_MOD_TIMER_INIT_SHIFT);
	edma_info("EDMA Rx mitigation timer value: %d\n", data);
	edma_reg_write(EDMA_REG_RX_MOD_TIMER(rxdesc_ring->ring_id), data);

	/*
	 * Configure the Mitigation packet count
	 */
	data = (nss_dp_rx_mitigation_pkt_cnt & EDMA_RXDESC_LOW_THRE_MASK)
			<< EDMA_RXDESC_LOW_THRE_SHIFT;
	edma_info("EDMA Rx mitigation packet count value: %d\n", data);
	edma_reg_write(EDMA_REG_RXDESC_UGT_THRE(rxdesc_ring->ring_id), data);

	/*
	 * Enable ring. Set ret mode to 'opaque'.
	 */
	edma_reg_write(EDMA_REG_RX_INT_CTRL(rxdesc_ring->ring_id), EDMA_RX_NE_INT_EN);
}

/*
 * edma_cfg_rx_fill_ring_configure()
 *	Configure one RxFill ring in EDMA HW
 */
static void edma_cfg_rx_fill_ring_configure(struct edma_rxfill_ring *rxfill_ring)
{
	uint32_t ring_sz;

	edma_reg_write(EDMA_REG_RXFILL_BA(rxfill_ring->ring_id),
			(uint32_t)(rxfill_ring->dma & EDMA_RING_DMA_MASK));

	ring_sz = rxfill_ring->count & EDMA_RXFILL_RING_SIZE_MASK;
	edma_reg_write(EDMA_RXFILL_RING_SIZE(rxfill_ring->ring_id), ring_sz);

	/*
	 * Alloc Rx buffers
	 */
	edma_rx_alloc_buffer(rxfill_ring, rxfill_ring->count - 1);
}

#if defined(NSS_DP_POINT_OFFLOAD)
/*
 * edma_cfg_rx_point_offload_ring_queue_get()
 *	Get rx point offload ring queue
 */
uint16_t edma_cfg_rx_point_offload_ring_queue_get(void)
{
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	return egc->point_offload_queue;
}
EXPORT_SYMBOL(edma_cfg_rx_point_offload_ring_queue_get);

/*
 * edma_cfg_rx_qid_to_rx_desc_point_offload_ring_mapping()
 *	Configure PPE queue id to Rx ring mapping
 */
static void edma_cfg_rx_qid_to_rx_desc_point_offload_ring_mapping(struct edma_gbl_ctx *egc)
{
	uint32_t desc_index, i;
	uint32_t reg_index, data;
	uint16_t start = EDMA_RX_POINT_OFFLOAD_QUEUE_BASE;
	uint16_t end = start + EDMA_RX_POINT_OFFLOAD_QUEUE_NUM;

	egc->point_offload_queue = EDMA_RX_POINT_OFFLOAD_QUEUE_BASE;

	/*
	 * Set PPE QID to EDMA Rx ring mapping.
	 * Each entry can hold mapping for 4 PPE queues and
	 * entry size is 4 bytes.
	 */
	desc_index = (egc->rxdesc_point_offload_ring & EDMA_RX_RING_ID_MASK);

	for (i = start; i <= end; i += EDMA_QID2RID_NUM_PER_REG) {
		reg_index = i/EDMA_QID2RID_NUM_PER_REG;
		data = EDMA_RX_RING_ID_QUEUE0_SET(desc_index) |
			EDMA_RX_RING_ID_QUEUE1_SET(desc_index) |
			EDMA_RX_RING_ID_QUEUE2_SET(desc_index) |
			EDMA_RX_RING_ID_QUEUE3_SET(desc_index);

		edma_reg_write(EDMA_QID2RID_TABLE_MEM(reg_index), data);
		desc_index += EDMA_QID2RID_NUM_PER_REG;

		edma_debug("Configure QID2RID(%d) reg:0x%x to 0x%x\n",
				i, EDMA_QID2RID_TABLE_MEM(reg_index), data);
	}
}

/*
 * edma_cfg_rx_point_offload_rings_to_rx_fill_mapping()
 *	Configure Rx rings to Rx fill mapping
 */
static void edma_cfg_rx_point_offload_rings_to_rx_fill_mapping(struct edma_gbl_ctx *egc)
{
	uint32_t data, reg, ring_id = egc->rxdesc_point_offload_ring;
	if ((ring_id >= 0) && (ring_id <= 9)) {
		reg = EDMA_REG_RXDESC2FILL_MAP_0;
	} else if ((ring_id >= 10) && (ring_id <= 19)) {
		reg = EDMA_REG_RXDESC2FILL_MAP_1;
	} else {
		reg = EDMA_REG_RXDESC2FILL_MAP_2;
	}

	edma_debug("Configure RXDESC point offload ring:%u to use RXFILL point offload ring:%u\n", ring_id, egc->rxfill_point_offload_ring);

	/*
	 * Set the Rx fill ring number in the
	 * mapping register.
	 */
	data = edma_reg_read(reg);
	data |= (egc->rxfill_point_offload_ring & EDMA_RXDESC2FILL_MAP_RXDESC_MASK) <<
				((ring_id % 10) * 3);
	edma_reg_write(reg, data);

	edma_debug("EDMA_REG_RXDESC2FILL_MAP_0: 0x%x\n", edma_reg_read(EDMA_REG_RXDESC2FILL_MAP_0));
	edma_debug("EDMA_REG_RXDESC2FILL_MAP_1: 0x%x\n", edma_reg_read(EDMA_REG_RXDESC2FILL_MAP_1));
	edma_debug("EDMA_REG_RXDESC2FILL_MAP_2: 0x%x\n", edma_reg_read(EDMA_REG_RXDESC2FILL_MAP_2));
}
#endif

/*
 * edma_cfg_rx_mcast_qid_to_core_mapping
 *	Configure mcast queue to core mapping.
 */
void edma_cfg_rx_mcast_qid_to_core_mapping(struct edma_gbl_ctx *egc, uint8_t core_id)
{
	uint32_t desc_index, q_id;
	uint32_t reg_index, data;

	/*
	 * Map PPE multicast queues to the Rx ring according to the core.
	 */
	desc_index = egc->rxdesc_ring_map[0][core_id];
	for (q_id = EDMA_CPU_PORT_MCAST_QUEUE_START;
		q_id <= EDMA_CPU_PORT_MCAST_QUEUE_END;
			q_id += EDMA_QID2RID_NUM_PER_REG) {
		reg_index = q_id/EDMA_QID2RID_NUM_PER_REG;
		data = EDMA_RX_RING_ID_QUEUE0_SET(desc_index) |
			EDMA_RX_RING_ID_QUEUE1_SET(desc_index) |
			EDMA_RX_RING_ID_QUEUE2_SET(desc_index) |
			EDMA_RX_RING_ID_QUEUE3_SET(desc_index);

		edma_reg_write(EDMA_QID2RID_TABLE_MEM(reg_index), data);

		edma_info("Configure QID2RID(%d) reg:0x%x to 0x%x\n",
				q_id, EDMA_QID2RID_TABLE_MEM(reg_index), data);
	}
}

/*
 * edma_cfg_rx_qid_to_rx_desc_ring_mapping()
 *	Configure PPE queue id to Rx ring mapping
 */
static void edma_cfg_rx_qid_to_rx_desc_ring_mapping(struct edma_gbl_ctx *egc)
{
	uint32_t desc_index, q_id;
	uint32_t reg_index, data;
	uint32_t ring_index;

	/*
	 * Set PPE QID to EDMA Rx ring mapping.
	 * Each entry can hold mapping for 4 PPE queues and
	 * entry size is 4 bytes.
	 */
	desc_index = (egc->rxdesc_ring_start & EDMA_RX_RING_ID_MASK);

	/*
	 * Here map all the queues to ring.
	 */
	for (q_id = egc->rx_queue_start;
		q_id <= EDMA_CPU_PORT_QUEUE_MAX(egc->rx_queue_start);
			q_id += EDMA_QID2RID_NUM_PER_REG) {
		reg_index = q_id/EDMA_QID2RID_NUM_PER_REG;
		ring_index = desc_index + EDMA_QUEUE_OFFSET(q_id);

		data = EDMA_RX_RING_ID_QUEUE0_SET(ring_index) |
			EDMA_RX_RING_ID_QUEUE1_SET(ring_index) |
			EDMA_RX_RING_ID_QUEUE2_SET(ring_index) |
			EDMA_RX_RING_ID_QUEUE3_SET(ring_index);

		edma_reg_write(EDMA_QID2RID_TABLE_MEM(reg_index), data);
		edma_info("Configure QID2RID(%d) reg:0x%x to 0x%x, desc_index: %d, reg_index: %d\n",
				q_id, EDMA_QID2RID_TABLE_MEM(reg_index), data, desc_index, reg_index);
	}

	/*
	 * Map PPE multicast queues to the first Rx ring.
	 */
	desc_index = (egc->rxdesc_ring_start & EDMA_RX_RING_ID_MASK);
	for (q_id = EDMA_CPU_PORT_MCAST_QUEUE_START;
		q_id <= EDMA_CPU_PORT_MCAST_QUEUE_END;
			q_id += EDMA_QID2RID_NUM_PER_REG) {
		reg_index = q_id/EDMA_QID2RID_NUM_PER_REG;
		data = EDMA_RX_RING_ID_QUEUE0_SET(desc_index) |
			EDMA_RX_RING_ID_QUEUE1_SET(desc_index) |
			EDMA_RX_RING_ID_QUEUE2_SET(desc_index) |
			EDMA_RX_RING_ID_QUEUE3_SET(desc_index);

		edma_reg_write(EDMA_QID2RID_TABLE_MEM(reg_index), data);

		edma_debug("Configure QID2RID(%d) reg:0x%x to 0x%x\n",
				q_id, EDMA_QID2RID_TABLE_MEM(reg_index), data);
	}
}

/*
 * edma_cfg_rx_rings_to_rx_fill_mapping()
 *	Configure Rx rings to Rx fill mapping
 */
static void edma_cfg_rx_rings_to_rx_fill_mapping(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	/*
	 * Set RXDESC2FILL_MAP_xx reg.
	 * 3 registers hold the Rxfill mapping for total 24 Rxdesc rings.
	 * 3 bits holds the rx fill ring mapping for each of the
	 * rx descriptor ring.
	 */
	edma_reg_write(EDMA_REG_RXDESC2FILL_MAP_0, 0);
	edma_reg_write(EDMA_REG_RXDESC2FILL_MAP_1, 0);
	edma_reg_write(EDMA_REG_RXDESC2FILL_MAP_2, 0);

	for (i = 0; i < egc->num_rxdesc_rings; i++) {
		uint32_t data, reg, ring_id;
		struct edma_rxdesc_ring *rxdesc_ring = &egc->rxdesc_rings[i];

		ring_id = rxdesc_ring->ring_id;
		if ((ring_id >= 0) && (ring_id <= 9)) {
			reg = EDMA_REG_RXDESC2FILL_MAP_0;
		} else if ((ring_id >= 10) && (ring_id <= 19)) {
			reg = EDMA_REG_RXDESC2FILL_MAP_1;
		} else {
			reg = EDMA_REG_RXDESC2FILL_MAP_2;
		}

		edma_debug("Configure RXDESC:%u to use RXFILL:%u\n",
						ring_id,
						rxdesc_ring->rxfill->ring_id);

		/*
		 * Set the Rx fill ring number in the
		 * mapping register.
		 */
		data = edma_reg_read(reg);
		data |= (rxdesc_ring->rxfill->ring_id &
				EDMA_RXDESC2FILL_MAP_RXDESC_MASK) <<
				((ring_id % 10) * 3);
		edma_reg_write(reg, data);
	}

	edma_debug("EDMA_REG_RXDESC2FILL_MAP_0: 0x%x\n", edma_reg_read(EDMA_REG_RXDESC2FILL_MAP_0));
	edma_debug("EDMA_REG_RXDESC2FILL_MAP_1: 0x%x\n", edma_reg_read(EDMA_REG_RXDESC2FILL_MAP_1));
	edma_debug("EDMA_REG_RXDESC2FILL_MAP_2: 0x%x\n", edma_reg_read(EDMA_REG_RXDESC2FILL_MAP_2));
}

/*
 * edma_cfg_rx_rings_enable()
 *	API to enable Rx and Rxfill rings
 */
void edma_cfg_rx_rings_enable(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	/*
	 * Enable Rx rings
	 */
	for (i = egc->rxdesc_ring_start; i < egc->rxdesc_ring_end; i++) {
		uint32_t data;

		data = edma_reg_read(EDMA_REG_RXDESC_CTRL(i));
		data |= EDMA_RXDESC_RX_EN;
		edma_reg_write(EDMA_REG_RXDESC_CTRL(i), data);
	}

	for (i = egc->rxfill_ring_start; i < egc->rxfill_ring_end; i++) {
		uint32_t data;

		data = edma_reg_read(EDMA_REG_RXFILL_RING_EN(i));
		data |= EDMA_RXFILL_RING_EN;
		edma_reg_write(EDMA_REG_RXFILL_RING_EN(i), data);
	}
}

/*
 * edma_cfg_rx_ring_en_mapped_queues()
 *	Enable / Disable the queues associated to the RX rings.
 */
bool edma_cfg_rx_ring_en_mapped_queues(struct edma_gbl_ctx *egc, uint16_t ring_id, bool enable)
{
	uint16_t ring_idx, queue_id, i;
	sw_error_t ret;
	a_bool_t en = enable;

	ring_idx = ring_id - egc->rxdesc_ring_start;
	for (i = 0; i < EDMA_MAX_PRI_PER_CORE; i++) {
		queue_id = egc->rx_ring_queue_map[i][ring_idx];
		ret = fal_qm_enqueue_ctrl_set(0, queue_id, en);
		if (ret != SW_OK) {
			edma_err("%px: Failed queue operation en %d", egc, enable);
			return false;
		}

		ret = fal_scheduler_dequeue_ctrl_set(0, queue_id, en);
		if (ret != SW_OK) {
			edma_err("%px: Failed dequeue operation en %d", egc, enable);
			return false;
		}
	}

	return true;
}

/*
 * edma_cfg_rx_ring_reset()
 *	API to reset the individual RX ring
 *	NOTE: Caller is expected to ensure that the corresponding
 *	PPE queue is stopped and the ring is disabled.
 */
void edma_cfg_rx_ring_reset(struct edma_rxdesc_ring *ring)
{
	uint32_t data = 0;

	/*
	 * Reset the ring - wait untill the reset operation is done.
	 */
	data = edma_reg_read(EDMA_REG_RXDESC_RESET(ring->ring_id));
	data |= EDMA_RXDESC_RX_RESET;
	edma_reg_write(EDMA_REG_RXDESC_RESET(ring->ring_id), data);

	do {
		data = edma_reg_read(EDMA_REG_RXDESC_RESET(ring->ring_id));
	} while (data);

	/*
	 * Reset the software consumer index.
	 */
	ring->cons_idx = 0;
}

/*
 * edma_cfg_rx_rings_disable()
 *	API to disable Rx and Rxfill rings
 */
void edma_cfg_rx_rings_disable(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	/*
	 * Disable Rx rings
	 */
	for (i = 0; i < egc->num_rxdesc_rings; i++) {
		uint32_t data;
		struct edma_rxdesc_ring *rxdesc_ring = NULL;

		rxdesc_ring = &egc->rxdesc_rings[i];
		data = edma_reg_read(EDMA_REG_RXDESC_CTRL(rxdesc_ring->ring_id));
		data &= ~EDMA_RXDESC_RX_EN;
		edma_reg_write(EDMA_REG_RXDESC_CTRL(rxdesc_ring->ring_id), data);
	}

	/*
	 * Disable RxFill Rings
	 */
	for (i = 0; i < egc->num_rxfill_rings; i++) {
		uint32_t data;
		struct edma_rxfill_ring *rxfill_ring = NULL;

		rxfill_ring = &egc->rxfill_rings[i];
		data = edma_reg_read(EDMA_REG_RXFILL_RING_EN(rxfill_ring->ring_id));
		data &= ~EDMA_RXFILL_RING_EN;
		edma_reg_write(EDMA_REG_RXFILL_RING_EN(rxfill_ring->ring_id), data);
	}
}

/*
 * edma_cfg_rx_mapping()
 *	API to setup RX ring mapping
 */
void edma_cfg_rx_mapping(struct edma_gbl_ctx *egc)
{
	edma_cfg_rx_qid_to_rx_desc_ring_mapping(egc);
	edma_cfg_rx_rings_to_rx_fill_mapping(egc);
}

#if defined(NSS_DP_POINT_OFFLOAD)
/*
 * edma_cfg_rx_point_offload_mapping()
 *	API to setup RX ring mapping
 */
void edma_cfg_rx_point_offload_mapping(struct edma_gbl_ctx *egc)
{
	uint32_t queue_id = egc->rx_queue_start;
	uint32_t word_idx, bit_idx;

	/*
	 * MAP Rx descriptor ring to PPE queues.
	 *
	 * TODO:
	 * Currently one Rx descriptor ring can get mapped to only
	 * single PPE queue. Multiple queues getting mapped to the
	 * single Rx descriptor ring is not yet supported.
	 * In future, we can support this by getting the Rx descriptor
	 * ring to queue mapping from the dtsi.
	 */
	word_idx = (queue_id / (EDMA_BITS_IN_WORD - 1));
	bit_idx = (queue_id % EDMA_BITS_IN_WORD);
	egc->rxdesc_point_offload_ring_to_queue_bm[word_idx] = 1 << bit_idx;

	/*
	 * Reset Rx descriptor ring mapped queue's configurations
	 */
	edma_cfg_rx_desc_point_offload_ring_reset_queue_config(egc);
	edma_cfg_rx_qid_to_rx_desc_point_offload_ring_mapping(egc);
	edma_cfg_rx_point_offload_rings_to_rx_fill_mapping(egc);
}
#endif

/*
 * edma_cfg_rx_rings_setup()
 *	Allocate/setup resources for EDMA rings
 */
static int edma_cfg_rx_rings_setup(struct edma_gbl_ctx *egc)
{
	int32_t ring_idx, alloc_size, buf_len, pri_idx;

	/*
	 * Set buffer allocation size
	 */
	if (egc->rx_jumbo_mru) {
		alloc_size = egc->rx_jumbo_mru + EDMA_RX_SKB_HEADROOM + NET_IP_ALIGN;
		buf_len = alloc_size - EDMA_RX_SKB_HEADROOM - NET_IP_ALIGN;
	} else if (egc->rx_page_mode) {
		alloc_size = EDMA_RX_PAGE_MODE_SKB_SIZE + EDMA_RX_SKB_HEADROOM + NET_IP_ALIGN;
		buf_len = PAGE_SIZE;
	} else {
		alloc_size = dp_global_ctx.rx_buf_size;
		buf_len = alloc_size - EDMA_RX_SKB_HEADROOM - NET_IP_ALIGN;
	}

	edma_debug("EDMA ctx:%px rx_ring alloc_size=%d, buf_len=%d\n", egc, alloc_size, buf_len);

	/*
	 * Allocate Rx fill ring descriptors
	 */
	for (ring_idx = 0; ring_idx < egc->num_rxfill_rings; ring_idx++) {
		int32_t ret;
		struct edma_rxfill_ring *rxfill_ring = NULL;

		rxfill_ring = &egc->rxfill_rings[ring_idx];
		rxfill_ring->count = EDMA_RX_RING_SIZE;
		rxfill_ring->ring_id = egc->rxfill_ring_start + ring_idx;
		rxfill_ring->alloc_size = alloc_size;
		rxfill_ring->buf_len = buf_len;
		rxfill_ring->page_mode = egc->rx_page_mode;

		ret = edma_cfg_rx_fill_ring_setup(rxfill_ring);
		if (ret != 0) {
			edma_err("Error in setting up %d rxfill ring. ret: %d",
					 rxfill_ring->ring_id, ret);
			while (--ring_idx >= 0) {
				edma_cfg_rx_fill_ring_cleanup(egc,
					&egc->rxfill_rings[ring_idx]);
			}

			return -ENOMEM;
		}
	}

	/*
	 * Allocate RxDesc ring descriptors
	 */
	for (ring_idx = 0; ring_idx < egc->num_rxdesc_rings; ring_idx++) {
		uint32_t index, word_idx, queue_id = egc->rx_queue_start;
		int32_t ret;
		struct edma_rxdesc_ring *rxdesc_ring = NULL;

		rxdesc_ring = &egc->rxdesc_rings[ring_idx];
		rxdesc_ring->count = EDMA_RX_RING_SIZE;
		rxdesc_ring->ring_id = egc->rxdesc_ring_start + ring_idx;

		if (queue_id > EDMA_CPU_PORT_QUEUE_MAX(egc->rx_queue_start)) {
			edma_err("Invalid queue_id: %d\n", queue_id);
			while (--ring_idx >= 0) {
				edma_cfg_rx_desc_ring_cleanup(egc, &egc->rxdesc_rings[ring_idx]);
			}

			goto rxdesc_mem_alloc_fail;
		}

		for (pri_idx = 0; pri_idx < EDMA_MAX_PRI_PER_CORE; pri_idx++) {
			queue_id = egc->rx_ring_queue_map[pri_idx][ring_idx];
			word_idx = (queue_id / EDMA_BITS_IN_WORD);

			/*
			 * To-do: Use ring queue map from dtsi to build a bitmap
			 */
			egc->rxdesc_ring_to_queue_bm[ring_idx][word_idx] |= 1 << queue_id;
			edma_debug("Queue_id: %d, ring_id: %d\n", queue_id, ring_idx);
		}

		/*
		 * Create a mapping between RX Desc ring and Rx fill ring.
		 * Number of fill rings are lesser than the descriptor rings
		 * Share the fill rings across descriptor rings.
		 */
		index = egc->rxfill_ring_start + (ring_idx % egc->num_rxfill_rings);
		rxdesc_ring->rxfill = &egc->rxfill_rings[index - egc->rxfill_ring_start];

		ret = edma_cfg_rx_desc_ring_setup(rxdesc_ring);
		if (ret != 0) {
			edma_err("Error in setting up %d rxdesc ring. ret: %d",
					 rxdesc_ring->ring_id, ret);
			while (--ring_idx >= 0) {
				edma_cfg_rx_desc_ring_cleanup(egc, &egc->rxdesc_rings[ring_idx]);
			}

			goto rxdesc_mem_alloc_fail;
		}
	}

	edma_info("Rx descriptor count for Rx desc and Rx fill rings : %d\n", EDMA_RX_RING_SIZE);

	return 0;

rxdesc_mem_alloc_fail:
	for (ring_idx = 0; ring_idx < egc->num_rxfill_rings; ring_idx++) {
		edma_cfg_rx_fill_ring_cleanup(egc, &egc->rxfill_rings[ring_idx]);
	}

	return -ENOMEM;
}

/*
 * edma_cfg_rx_page_mode_and_jumbo()
 *	Configure EDMA Rx rings page mode and jumbo MRU
 */
void edma_cfg_rx_page_mode_and_jumbo(struct edma_gbl_ctx *egc)
{
	/*
	 * Over-ride EDMA global page mode if specified in module param
	 */
	if (dp_global_ctx.overwrite_mode) {
		edma_debug("Page mode is overwritten: %d\n", dp_global_ctx.page_mode);
		egc->rx_page_mode = dp_global_ctx.page_mode;
	}

	if (dp_global_ctx.jumbo_mru) {
		egc->rx_page_mode = false;
		egc->rx_jumbo_mru = dp_global_ctx.jumbo_mru;
		edma_debug("Jumbo mru is enabled: %d\n", egc->rx_jumbo_mru);
	}
}

/*
 * edma_cfg_rx_rings_alloc()
 *	Allocate EDMA Rx rings
 */
int32_t edma_cfg_rx_rings_alloc(struct edma_gbl_ctx *egc)
{
	egc->rxfill_rings = kzalloc((sizeof(struct edma_rxfill_ring) *
				egc->num_rxfill_rings), GFP_KERNEL);
	if (!egc->rxfill_rings) {
		edma_err("Error in allocating rxfill ring\n");
		return -ENOMEM;
	}

	egc->rxdesc_rings = kzalloc((sizeof(struct edma_rxdesc_ring) *
				egc->num_rxdesc_rings), GFP_KERNEL);
	if (!egc->rxdesc_rings) {
		edma_err("Error in allocating rxdesc ring\n");
		goto rxdesc_ring_alloc_fail;
	}

	edma_info("RxDesc:%u (%u-%u) RxFill:%u (%u-%u)\n",
		egc->num_rxdesc_rings, egc->rxdesc_ring_start,
		(egc->rxdesc_ring_start + egc->num_rxdesc_rings - 1),
		egc->num_rxfill_rings, egc->rxfill_ring_start,
		(egc->rxfill_ring_start + egc->num_rxfill_rings - 1));

	egc->rxdesc_ring_to_queue_bm = kzalloc(egc->num_rxdesc_rings * sizeof(uint32_t) * \
			EDMA_RING_MAPPED_QUEUE_BM_WORD_COUNT, GFP_KERNEL);
	if (!egc->rxdesc_ring_to_queue_bm) {
		edma_err("Error in allocating mapped queue area for Rxdesc rings\n");
		goto rx_rings_mapped_queue_alloc_failed;
	}

	if (edma_cfg_rx_rings_setup(egc)) {
		edma_err("Error in setting up rx rings\n");
		goto rx_rings_setup_fail;
	}

	/*
	 * Reset Rx descriptor ring mapped queue's configurations
	 */
	if (edma_cfg_rx_desc_ring_reset_queue_config(egc)) {
		edma_err("Error in resetting the Rx descriptor rings configurations\n");
		edma_cfg_rx_rings_cleanup(egc);
		return -EINVAL;
	}

	return 0;

rx_rings_setup_fail:
	kfree(egc->rxdesc_ring_to_queue_bm);
	egc->rxdesc_ring_to_queue_bm = NULL;
rx_rings_mapped_queue_alloc_failed:
	kfree(egc->rxdesc_rings);
	egc->rxdesc_rings = NULL;
rxdesc_ring_alloc_fail:
	kfree(egc->rxfill_rings);
	egc->rxfill_rings = NULL;
	return -ENOMEM;
}

/*
 * edma_cfg_rx_rings_cleanup()
 *	Cleanup EDMA rings
 */
void edma_cfg_rx_rings_cleanup(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	/*
	 * Free Rx fill ring descriptors
	 */
	for (i = 0; i < egc->num_rxfill_rings; i++) {
		edma_cfg_rx_fill_ring_cleanup(egc, &egc->rxfill_rings[i]);
	}

	/*
	 * Free Rx completion ring descriptors
	 */
	for (i = 0; i < egc->num_rxdesc_rings; i++) {
		edma_cfg_rx_desc_ring_cleanup(egc, &egc->rxdesc_rings[i]);
	}

	kfree(egc->rxfill_rings);
	kfree(egc->rxdesc_rings);
	egc->rxfill_rings = NULL;
	egc->rxdesc_rings = NULL;

	if (egc->rxdesc_ring_to_queue_bm) {
		kfree(egc->rxdesc_ring_to_queue_bm);
		egc->rxdesc_ring_to_queue_bm = NULL;
	}
}

#if defined(NSS_DP_POINT_OFFLOAD)
/*
 * edma_cfg_rx_point_offload_rings()
 *	Configure EDMA point offload rings
 */
void edma_cfg_rx_point_offload_rings(struct edma_gbl_ctx *egc)
{
	if (edma_cfg_rx_fc_enable) {
		/*
		 * Validate flow control X-OFF and X-ON configurations
		 */
		if ((nss_dp_rx_fc_xoff < EDMA_RX_FC_XOFF_THRE_MIN) ||
				(nss_dp_rx_fc_xoff > EDMA_RX_RING_SIZE)) {
			edma_err("Incorrect Rx Xoff flow control value: %d. Setting\n"
					" it to default value: %d", nss_dp_rx_fc_xoff,
					NSS_DP_RX_FC_XOFF_DEF);
			nss_dp_rx_fc_xoff = NSS_DP_RX_FC_XOFF_DEF;
		}

		if ((nss_dp_rx_fc_xon < EDMA_RX_FC_XON_THRE_MIN) ||
				(nss_dp_rx_fc_xon > EDMA_RX_RING_SIZE) ||
				(nss_dp_rx_fc_xon < nss_dp_rx_fc_xoff)) {
			edma_err("Incorrect Rx Xon flow control value: %d. Setting\n"
					" it to default value: %d", nss_dp_rx_fc_xon,
					NSS_DP_RX_FC_XON_DEF);
			nss_dp_rx_fc_xon = NSS_DP_RX_FC_XON_DEF;
		}

		/*
		 * Configure Rx flow control configurations
		 */
		edma_cfg_rx_desc_point_offload_ring_flow_control(egc, nss_dp_rx_fc_xoff, nss_dp_rx_fc_xon);
		edma_cfg_rx_fill_point_offload_ring_flow_control(egc, nss_dp_rx_fc_xoff, nss_dp_rx_fc_xon);
		edma_cfg_rx_desc_point_offload_ring_to_queue_mapping(egc);
	}
}
#endif

/*
 * edma_cfg_rx_rings()
 *	Configure EDMA rings
 */
void edma_cfg_rx_rings(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	/*
	 * Configure RXFILL rings
	 */
	for (i = 0; i < egc->num_rxfill_rings; i++) {
		edma_cfg_rx_fill_ring_configure(&egc->rxfill_rings[i]);
	}

	/*
	 * Configure RXDESC ring
	 */
	for (i = 0; i < egc->num_rxdesc_rings; i++) {
		edma_cfg_rx_desc_ring_configure(&egc->rxdesc_rings[i]);
	}

	if (edma_cfg_rx_fc_enable) {
		/*
		 * Validate flow control X-OFF and X-ON configurations
		 */
		if ((nss_dp_rx_fc_xoff < EDMA_RX_FC_XOFF_THRE_MIN) ||
				(nss_dp_rx_fc_xoff > EDMA_RX_RING_SIZE)) {
			edma_err("Incorrect Rx Xoff flow control value: %d. Setting\n"
					" it to default value: %d", nss_dp_rx_fc_xoff,
					NSS_DP_RX_FC_XOFF_DEF);
			nss_dp_rx_fc_xoff = NSS_DP_RX_FC_XOFF_DEF;
		}

		if ((nss_dp_rx_fc_xon < EDMA_RX_FC_XON_THRE_MIN) ||
				(nss_dp_rx_fc_xon > EDMA_RX_RING_SIZE) ||
				(nss_dp_rx_fc_xon < nss_dp_rx_fc_xoff)) {
			edma_err("Incorrect Rx Xon flow control value: %d. Setting\n"
					" it to default value: %d", nss_dp_rx_fc_xon,
					NSS_DP_RX_FC_XON_DEF);
			nss_dp_rx_fc_xon = NSS_DP_RX_FC_XON_DEF;
		}

		/*
		 * Configure Rx flow control configurations
		 */
		edma_cfg_rx_desc_ring_flow_control(nss_dp_rx_fc_xoff, nss_dp_rx_fc_xon);
		edma_cfg_rx_fill_ring_flow_control(nss_dp_rx_fc_xoff, nss_dp_rx_fc_xon);
		edma_cfg_rx_desc_ring_to_queue_mapping(edma_cfg_rx_fc_enable);
	}
}

/*
 * edma_cfg_rx_napi_disable()
 *	Disable RX NAPI
 */
void edma_cfg_rx_napi_disable(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	for (i = 0; i < egc->num_rxdesc_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;

		rxdesc_ring = &egc->rxdesc_rings[i];

		if (!rxdesc_ring->napi_added) {
			continue;
		}

		napi_disable(&rxdesc_ring->napi);
	}
}

/*
 * edma_cfg_rx_napi_enable()
 *	Enable RX NAPI
 */
void edma_cfg_rx_napi_enable(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	for (i = 0; i < egc->num_rxdesc_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;

		rxdesc_ring = &egc->rxdesc_rings[i];

		if (!rxdesc_ring->napi_added) {
			continue;
		}

		napi_enable(&rxdesc_ring->napi);
	}
}

/*
 * edma_cfg_rx_napi_delete()
 *	Delete RX NAPI
 */
void edma_cfg_rx_napi_delete(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	for (i = 0; i < egc->num_rxdesc_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;

		rxdesc_ring = &egc->rxdesc_rings[i];

		if (!rxdesc_ring->napi_added) {
			continue;
		}

		netif_napi_del(&rxdesc_ring->napi);
		rxdesc_ring->napi_added = false;
	}
}

/*
 * edma_cfg_rx_napi_add()
 *	RX NAPI add API
 */
void edma_cfg_rx_napi_add(struct edma_gbl_ctx *egc, struct net_device *netdev)
{
	uint32_t i;

	if ((nss_dp_rx_napi_budget < EDMA_RX_NAPI_WORK_MIN) ||
		(nss_dp_rx_napi_budget > EDMA_RX_NAPI_WORK_MAX)) {
		edma_err("Incorrect Rx NAPI budget: %d, setting to default: %d",
			nss_dp_rx_napi_budget, NSS_DP_HAL_RX_NAPI_BUDGET);
		nss_dp_rx_napi_budget = NSS_DP_HAL_RX_NAPI_BUDGET;
	}

	for (i = 0; i < egc->num_rxdesc_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring = &egc->rxdesc_rings[i];
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		if (nss_dp_capwap_vp_rx_core == i) {
			edma_info("Adding capwap napi for ring_id %d for core3\n", nss_dp_capwap_vp_rx_core);
			netif_threaded_napi_add(netdev, &rxdesc_ring->napi,
				edma_rx_napi_capwap_poll, nss_dp_rx_napi_budget);
		} else {
			netif_threaded_napi_add(netdev, &rxdesc_ring->napi,
				edma_rx_napi_poll, nss_dp_rx_napi_budget);
		}
#else
		if (nss_dp_capwap_vp_rx_core == i) {
			edma_info("Adding capwap napi for ring_id %d for core3\n", nss_dp_capwap_vp_rx_core);
			netif_threaded_napi_add_weight(netdev, &rxdesc_ring->napi,
				edma_rx_napi_capwap_poll, nss_dp_rx_napi_budget);
		} else {
			netif_threaded_napi_add_weight(netdev, &rxdesc_ring->napi,
				 edma_rx_napi_poll, nss_dp_rx_napi_budget);
		}
#endif
		rxdesc_ring->napi_added = true;
	}
	edma_info("%s: Rx NAPI budget: %d\n", netdev->name, nss_dp_rx_napi_budget);
}

/*
 * edma_cfg_rx_fc_enable_handler()
 *	API to enable/disable Rx rings flow control configurations
 *
 * The Rx flow control has the X-OFF and the X-ON threshold values.
 * Whenever the free Rx ring descriptor count falls below the X-OFF value, the
 * ring level flow control will kick in and the mapped PPE queues will be backpressured.
 * Similarly, whenever the free Rx ring descriptor count crosses the X-ON value,
 * the ring level flow control will be disabled.
 */
int edma_cfg_rx_fc_enable_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);

	if (!write) {
		return ret;
	}

	if (edma_cfg_rx_fc_enable) {
		/*
		 * Validate flow control X-OFF and X-ON configurations
		 */
		if ((nss_dp_rx_fc_xoff < EDMA_RX_FC_XOFF_THRE_MIN) ||
				(nss_dp_rx_fc_xoff > EDMA_RX_RING_SIZE)) {
			edma_err("Incorrect Rx Xoff flow control value: %d. Setting\n"
					" it to default value: %d", nss_dp_rx_fc_xoff,
					NSS_DP_RX_FC_XOFF_DEF);
			nss_dp_rx_fc_xoff = NSS_DP_RX_FC_XOFF_DEF;
		}
		if ((nss_dp_rx_fc_xon < EDMA_RX_FC_XON_THRE_MIN) ||
				(nss_dp_rx_fc_xon > EDMA_RX_RING_SIZE) ||
				(nss_dp_rx_fc_xon < nss_dp_rx_fc_xoff)) {
			edma_err("Incorrect Rx Xon flow control value: %d. Setting\n"
					" it to default value: %d", nss_dp_rx_fc_xon,
					NSS_DP_RX_FC_XON_DEF);
			nss_dp_rx_fc_xon = NSS_DP_RX_FC_XON_DEF;
		}

		/*
		 * Configure Rx flow control configurations
		 */
		edma_cfg_rx_desc_ring_flow_control(nss_dp_rx_fc_xoff, nss_dp_rx_fc_xon);
		edma_cfg_rx_fill_ring_flow_control(nss_dp_rx_fc_xoff, nss_dp_rx_fc_xon);
		edma_cfg_rx_desc_ring_to_queue_mapping(edma_cfg_rx_fc_enable);
	} else {
		/*
		 * De-configure Rx flow control configurations:
		 *
		 * 1. Reset  the Rx descriptor and Rx fill rings flow control
		 * configurations with original X-OFF and X-ON threshold values.
		 * 2. Reset the backpressure configuration by un-mapping the Rx
		 * descriptor rings with any mapped PPE queues
		 */
		edma_cfg_rx_desc_ring_flow_control(0, 0);
		edma_cfg_rx_fill_ring_flow_control(0, 0);
		edma_cfg_rx_desc_ring_to_queue_mapping(edma_cfg_rx_fc_enable);
	}

	return ret;
}

/*
 * edma_cfg_rx_queue_tail_drop_handler()
 *	API to enable/disable queue tail drop configuration
 */
int edma_cfg_rx_queue_tail_drop_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);

	if (!write) {
		return ret;
	}

	if (edma_cfg_rx_queue_tail_drop_enable) {
		/*
		 * Validate queue's AC FC threshold configuration value
		 */
		if ((nss_dp_rx_ac_fc_threshold < EDMA_RX_AC_FC_THRE_MIN) ||
				(nss_dp_rx_ac_fc_threshold > EDMA_RX_AC_FC_THRE_MAX)) {
			edma_err("Incorrect AC FC threshold value: %d. Setting"
					" it to default value: %d", nss_dp_rx_ac_fc_threshold,
					NSS_DP_RX_AC_FC_THRES_DEF);
			nss_dp_rx_ac_fc_threshold = NSS_DP_RX_AC_FC_THRES_DEF;
		}

		/*
		 * Configure queue tail drop by enabling and configuring the
		 * AC FC threshold for mapped PPE queues of the Rx descriptor rings
		 */
		edma_cfg_rx_mapped_queue_ac_fc_configure(nss_dp_rx_ac_fc_threshold,
					edma_cfg_rx_queue_tail_drop_enable);
	} else {
		/*
		 * De-configure mapped queues queue tail drop configurations:
		 *
		 * Reset the threshold configuration and disable the Rx descriptor's
		 * mapped queues AC FC configurataion.
		 */
		edma_cfg_rx_mapped_queue_ac_fc_configure(EDMA_RX_AC_FC_THRE_ORIG,
					edma_cfg_rx_queue_tail_drop_enable);
	}

	return ret;
}

/*
 * edma_cfg_rx_rps()
 *	API to configure RPS hash mapping for the given number of cores
 */
int edma_cfg_rx_rps(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);

	if (!write) {
		return ret;
	}

	if (!edma_cfg_rx_rps_num_cores ||
			(edma_cfg_rx_rps_num_cores > NR_CPUS)) {
		edma_err("Incorrect queue count: %d. Setting it to default"
			       " value: %d", edma_cfg_rx_rps_num_cores, NR_CPUS);
		edma_cfg_rx_rps_num_cores = NR_CPUS;
	}

	/*
	 * Set bitmap based on given number of cores.
	 */
	edma_cfg_rx_rps_bitmap_cores = (1 << edma_cfg_rx_rps_num_cores) - 1;
	edma_configure_rps_hash_map(&edma_gbl_ctx);

	edma_warn("EDMA RPS configured to use %d cores\n", edma_cfg_rx_rps_num_cores);
	return ret;
}

/*
 * edma_cfg_rx_rps_bitmap()
 * 	API to configure RPS hash mapping based on bitmap
 *
 * The core to use is selected on the basis of set bit in the function
 * edma_configure_rps_hash_map. We are using edma_cfg_rx_rps_bitmap as it will help
 * us to distinguish between rps_num_core and rps_bitmap_core as both
 * are calling the same configure hash function.
 */
int edma_cfg_rx_rps_bitmap(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);

	if (!write) {
		return ret;
	}

	/*
	 * Check for a valid bitmap core value
	 * Set to use 4 cores in case of invalid core
	 */
	if (!edma_cfg_rx_rps_bitmap_cores ||
			(edma_cfg_rx_rps_bitmap_cores > EDMA_RX_DEFAULT_BITMAP)) {
		edma_err("Incorrect CPU bitmap: %d. Setting it to default"
				" value: %d", edma_cfg_rx_rps_bitmap_cores, EDMA_RX_DEFAULT_BITMAP);
		edma_cfg_rx_rps_bitmap_cores = EDMA_RX_DEFAULT_BITMAP;
	}

	edma_configure_rps_hash_map(&edma_gbl_ctx);

	edma_warn("EDMA RPS bitmap value: %d\n", edma_cfg_rx_rps_bitmap_cores);
	return ret;
}
