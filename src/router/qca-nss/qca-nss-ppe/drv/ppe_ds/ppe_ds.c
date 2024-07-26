/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>

#include "ppe_vp_public.h"
#include "ppe_ds.h"
#include "ppe_ds_stats.h"

#define IDX_MGMT_PERIOD max_t(u64, 10000, NSEC_PER_SEC / idx_mgmt_freq)

/*
 * ppe_ds_ppe2tcl_rx()
 *	PPE-DS PPE2TCL Rx processing API
 */
static void ppe_ds_ppe2tcl_rx(nss_dp_ppeds_handle_t *edma_handle, uint16_t hw_prod_idx)
{
	struct ppe_ds *node = nss_dp_ppeds_priv(edma_handle);
	ppe_ds_wlan_handle_t *wlan_handle = &node->wlan_handle;

	return node->wlan_ops->set_tcl_prod_idx(wlan_handle, hw_prod_idx);
}

/*
 * ppe_ds_ppe2tcl_fill()
 *	PPE-DS WLAN Tx descriptor and buffer fill API
 */
static uint32_t ppe_ds_ppe2tcl_fill(nss_dp_ppeds_handle_t *edma_handle, uint32_t num_buff_req, uint32_t buff_size,
		uint32_t headroom)
{
	struct ppe_ds *node = nss_dp_ppeds_priv(edma_handle);
	ppe_ds_wlan_handle_t *wlan_handle = &node->wlan_handle;

	struct ppe_ds_wlan_txdesc_elem *tx_desc_arr = (struct ppe_ds_wlan_txdesc_elem *)nss_dp_ppeds_get_rx_fill_arr(edma_handle);

	return node->wlan_ops->get_tx_desc_many(wlan_handle, tx_desc_arr, num_buff_req, buff_size, headroom);
}

/*
 * ppe_ds_reo2ppe_tx_cmpl()
 *	PPE-DS WLAN Rx descriptor and buffer fill API
 */
static void ppe_ds_reo2ppe_tx_cmpl(nss_dp_ppeds_handle_t *edma_handle, uint16_t count)
{
	struct ppe_ds *node = nss_dp_ppeds_priv(edma_handle);
	ppe_ds_wlan_handle_t *wlan_handle = &node->wlan_handle;

	struct ppe_ds_wlan_rxdesc_elem *rx_desc_arr = (struct ppe_ds_wlan_rxdesc_elem *)nss_dp_ppeds_get_tx_cmpl_arr(edma_handle);

	return node->wlan_ops->release_rx_desc(wlan_handle, rx_desc_arr, count);
}

/*
 * ppe_ds_ppe2tcl_rel()
 *	PPE-DS WLAN Tx descriptor and buffer release API
 */
static void ppe_ds_ppe2tcl_rel(nss_dp_ppeds_handle_t *edma_handle, uint64_t rx_opaque)
{
	struct ppe_ds *node = nss_dp_ppeds_priv(edma_handle);
	ppe_ds_wlan_handle_t *wlan_handle = &node->wlan_handle;
	uint32_t cookie = (uint32_t)(rx_opaque) & 0x000fffff;

	return node->wlan_ops->release_tx_desc_single(wlan_handle, cookie);
}

/*
 * ppe_ds_notify_napi_done()
 *	PPE-DS ring NAPI done notification to WLAN.
 */
static void ppe_ds_notify_napi_done(nss_dp_ppeds_handle_t *edma_handle)
{
	struct ppe_ds *node = nss_dp_ppeds_priv(edma_handle);
	ppe_ds_wlan_handle_t *wlan_handle = &node->wlan_handle;

	node->wlan_ops->notify_napi_done(wlan_handle);
}

/*
 * ppe_ds_timer()
 *	PPE-DS timer callback
 */
static enum hrtimer_restart ppe_ds_timer(struct hrtimer *hrtimer)
{
	uint32_t cons_idx, prod_idx, move;
	struct ppe_ds *node = container_of(hrtimer,  struct ppe_ds, timer);
	ppe_ds_wlan_handle_t *wlan_handle = &node->wlan_handle;
	nss_dp_ppeds_handle_t *edma_handle = node->edma_handle;
	uint32_t ppe2tcl_ring_size = edma_handle->ppe2tcl_num_desc;
	uint32_t reo2ppe_size = edma_handle->reo2ppe_num_desc;
	struct nss_dp_ppeds_ops *dp_ops = node->dp_ops;

	/*
	 * Move Prod Idx
	 */
	prod_idx = dp_ops->get_rx_prod_idx(edma_handle);
	cons_idx = node->wlan_ops->get_tcl_cons_idx(wlan_handle);
	move = (prod_idx - cons_idx  + ppe2tcl_ring_size) &
						(ppe2tcl_ring_size - 1);
	if (move > 0) {
		node->wlan_ops->set_tcl_prod_idx(wlan_handle, prod_idx);
	}

	/*
	 * Move Cons Index
	 */
	dp_ops->set_rx_cons_idx(edma_handle, cons_idx);

	/*
	 * Move producer index for UL
	 */
	prod_idx = node->wlan_ops->get_reo_prod_idx(wlan_handle);
	cons_idx = dp_ops->get_tx_cons_idx(edma_handle);
	move = (prod_idx - cons_idx  + reo2ppe_size) & (reo2ppe_size - 1);
	if (move > 0) {
		dp_ops->set_tx_prod_idx(edma_handle, prod_idx);
	}

	/*
	 * Move consumer index for UL
	 */
	if (cons_idx != node->last_reo2ppe_cons_idx) {
		node->wlan_ops->set_reo_cons_idx(wlan_handle, cons_idx);
		node->last_reo2ppe_cons_idx = cons_idx;
	}

        hrtimer_forward(hrtimer, ktime_get(), ns_to_ktime(IDX_MGMT_PERIOD));
        return HRTIMER_RESTART;
}

/*
 * ppe_ds_ppe2tcl_ds_wlan_handle_intr()
 *	PPE-DS PPE2TCL IRQ Tx processing API
 *
 * This is an interrupt handler for PPE2TCL ring.
 * Gets trggered periodically at configured time.
 *
 */
int ppe_ds_ppe2tcl_wlan_handle_intr(void *ctxt)
{
	uint32_t cons_idx, prod_idx, move;
	struct ppe_ds *node = (struct ppe_ds *)ctxt;
	ppe_ds_wlan_handle_t *wlan_handle = &node->wlan_handle;
	nss_dp_ppeds_handle_t *edma_handle = node->edma_handle;
	struct nss_dp_ppeds_ops *dp_ops = node->dp_ops;

	if (!node->en_process_irq) {
		if (node->umac_reset_inprogress) {
			node->wlan_ops->enable_tx_consume_intr(wlan_handle,
							       false);
		}
		return 0;
	}

	prod_idx = dp_ops->get_rx_prod_idx(edma_handle);
	cons_idx = node->wlan_ops->get_tcl_cons_idx(wlan_handle);

	/*
	 * Move Consumer Index
	 */
	dp_ops->set_rx_cons_idx(edma_handle, cons_idx);
	if (unlikely(prod_idx == cons_idx)) {
		/* Disable the wlan interrupt */
		node->wlan_ops->enable_tx_consume_intr(wlan_handle, false);
		/* Enable the edma interrupt */
		dp_ops->enable_rx_reap_intr(edma_handle);
	} else {
		uint32_t ppe2tcl_ring_size = edma_handle->ppe2tcl_num_desc;

		move = (prod_idx - cons_idx  + ppe2tcl_ring_size) &
				(ppe2tcl_ring_size - 1);
		atomic64_add(move, &ppe_ds_node_stats[node->node_cfg_idx].tx_pkts);
		/*
		 * Move Producer Idx
		 */
		node->wlan_ops->set_tcl_prod_idx(wlan_handle, prod_idx);
	}

	return 0;
}
EXPORT_SYMBOL(ppe_ds_ppe2tcl_wlan_handle_intr);

/*
 * ppe_ds_reo2ppe_wlan_handle_intr()
 *	PPE-DS REO2PPE IRQ Rx processing API
 *
 * This is an interrupt handler for REO2PPE ring.
 * Gets trggered periodically at configured time,
 * whenever the head pointer moves.
 *
 */
int ppe_ds_reo2ppe_wlan_handle_intr(void *ctxt)
{
	uint16_t cons_idx, prod_idx, move;
	struct ppe_ds *node = (struct ppe_ds *)ctxt;
	ppe_ds_wlan_handle_t *wlan_handle = &node->wlan_handle;
	nss_dp_ppeds_handle_t *edma_handle = node->edma_handle;
	uint32_t reo2ppe_size = edma_handle->reo2ppe_num_desc;
	struct nss_dp_ppeds_ops *dp_ops = node->dp_ops;

	if (!node->en_process_irq) {
		return 0;
	}

	/*
	 * Move producer index for UL
	 */
	prod_idx = node->wlan_ops->get_reo_prod_idx(wlan_handle);
	cons_idx = dp_ops->get_tx_cons_idx(edma_handle);
	move = (prod_idx - cons_idx  + reo2ppe_size) & (reo2ppe_size - 1);
	if (move > 0) {
		dp_ops->set_tx_prod_idx(edma_handle, prod_idx);
	}

	/*
	 * Move consumer index for UL
	 */
	if (cons_idx != node->last_reo2ppe_cons_idx) {
		node->wlan_ops->set_reo_cons_idx(wlan_handle, cons_idx);
		node->last_reo2ppe_cons_idx = cons_idx;
	}
	atomic64_add(move, &ppe_ds_node_stats[node->node_cfg_idx].rx_pkts);

	return 0;
}
EXPORT_SYMBOL(ppe_ds_reo2ppe_wlan_handle_intr);

static void ppe_ds_enable_wlan_intr(nss_dp_ppeds_handle_t *edma_handle,
					bool enable)
{
	struct ppe_ds *node = nss_dp_ppeds_priv(edma_handle);
	ppe_ds_wlan_handle_t *wlan_handle = &node->wlan_handle;

	node->wlan_ops->enable_tx_consume_intr(wlan_handle, true);
}

void *ppe_ds_wlan_get_intr_ctxt(ppe_ds_wlan_handle_t *wlan_handle)
{
	struct ppe_ds *node = container_of(wlan_handle, struct ppe_ds,
			wlan_handle);

	return (void *)node;
}
EXPORT_SYMBOL(ppe_ds_wlan_get_intr_ctxt);

/*
 * ppe_ds_wlan_service_status_update()
 *	Umac reset service update.
 */
void ppe_ds_wlan_service_status_update(ppe_ds_wlan_handle_t *wlan_handle, bool enable)
{
	struct ppe_ds *node = container_of(wlan_handle, struct ppe_ds,
			wlan_handle);
	nss_dp_ppeds_handle_t *edma_handle = node->edma_handle;
	struct nss_dp_ppeds_ops *dp_ops = node->dp_ops;

	if (!dp_ops || !dp_ops->service_status_update) {
		ppe_ds_err("NULL service status update API\n");
		return;
	}

	dp_ops->service_status_update(edma_handle, enable);
}
EXPORT_SYMBOL(ppe_ds_wlan_service_status_update);

/*
 * ppe_ds_get_cur_prod_cons_ring_idx()
 *	Get the current EDMA producer and consumer ring indices
 */
static void ppe_ds_get_cur_prod_cons_ring_idx(ppe_ds_wlan_handle_t *wlan_handle, struct ppe_ds_wlan_reg_info *reg_info)
{
	struct ppe_ds *node = container_of(wlan_handle, struct ppe_ds, wlan_handle);
	nss_dp_ppeds_handle_t *edma_handle = node->edma_handle;
	struct nss_dp_ppeds_ops *dp_ops = node->dp_ops;

	reg_info->reo2ppe_start_idx = dp_ops->get_tx_cons_idx(edma_handle);
	dp_ops->set_tx_prod_idx(edma_handle, reg_info->reo2ppe_start_idx);
	reg_info->ppe2tcl_start_idx = dp_ops->get_rx_prod_idx(edma_handle);
	dp_ops->set_rx_cons_idx(edma_handle, reg_info->ppe2tcl_start_idx);

	dp_ops->set_rxfill_prod_idx(edma_handle, dp_ops->get_rxfill_cons_idx(edma_handle));

	reg_info->ppe_ds_int_mode_enabled = !polling_for_idx_update;

	ppe_ds_info("%px: PPE-DS get current EDMA ring indices API call successful", node);
	return;
}

/*
 * ppe_ds_wlan_rx()
 *	PPE-DS REO2PPE Rx processing API
 */
void ppe_ds_wlan_rx(ppe_ds_wlan_handle_t *wlan_handle, uint16_t reo_prod_idx)
{
	struct ppe_ds *node = container_of(wlan_handle, struct ppe_ds, wlan_handle);
	struct ppe_ds_node_config *node_cfg = &(ppe_ds_node_cfg[node->node_cfg_idx]);
	nss_dp_ppeds_handle_t *edma_handle = node->edma_handle;
	struct nss_dp_ppeds_ops *dp_ops = node->dp_ops;

	read_lock_bh(&node_cfg->lock);
	if(node_cfg->node_state != PPE_DS_NODE_STATE_START_DONE) {
		ppe_ds_err("Invalid node state: %d, PPE-DS REO2PPE processing API failed\n",
				node_cfg->node_state);
		read_unlock_bh(&node_cfg->lock);
		return;
	}
	read_unlock_bh(&node_cfg->lock);

	if (!dp_ops || !dp_ops->set_tx_prod_idx) {
		ppe_ds_err("NULL EDMA operation in REO2PPE processing API\n");
		return;
	}

	dp_ops->set_tx_prod_idx(edma_handle, reo_prod_idx);
}
EXPORT_SYMBOL(ppe_ds_wlan_rx);

/*
 * ppe_ds_wlan_vp_free()
 *	PPE-DS WLAN VP free API
 */
ppe_vp_status_t ppe_ds_wlan_vp_free(ppe_ds_wlan_handle_t *wlan_handle, ppe_vp_num_t vp_num)
{
	return ppe_vp_free(vp_num);
}
EXPORT_SYMBOL(ppe_ds_wlan_vp_free);

/*
 * ppe_ds_wlan_vp_alloc()
 *	PPE-DS WLAN VP alloc API
 */
ppe_vp_num_t ppe_ds_wlan_vp_alloc(ppe_ds_wlan_handle_t *wlan_handle, struct net_device *dev, struct ppe_vp_ai *vpai)
{
	uint32_t ppe_queue_start;
	struct ppe_ds *node;
	struct ppe_ds_node_config *node_cfg;
	nss_dp_ppeds_handle_t *edma_handle;
	struct nss_dp_ppeds_ops *dp_ops;

	if ((!wlan_handle) && (vpai->net_dev_flags != PPE_VP_NET_DEV_FLAG_IS_MLD)) {
		ppe_ds_err("wlan_handle is NULL for non MLD VP\n");
		return PPE_VP_STATUS_FAILURE;
	}

	if (vpai->net_dev_flags == PPE_VP_NET_DEV_FLAG_IS_MLD) {
		ppe_ds_info("MLD VP so skipping node to queue mapping\n");
		goto vp_alloc;
	}

	node = container_of(wlan_handle, struct ppe_ds, wlan_handle);
	dp_ops = node->dp_ops;
	if (!dp_ops || !dp_ops->get_queues) {
		ppe_ds_err("NULL EDMA operation, in PPE-DS VP alloc API\n");
		return PPE_VP_STATUS_FAILURE;
	}

	node_cfg = &(ppe_ds_node_cfg[node->node_cfg_idx]);
	edma_handle = node->edma_handle;

	read_lock_bh(&node_cfg->lock);
	if(node_cfg->node_state != PPE_DS_NODE_STATE_START_DONE) {
		ppe_ds_err("Invalid node state: %d, PPE-DS vp alloc failed\n",
				node_cfg->node_state);
		read_unlock_bh(&node_cfg->lock);
		return PPE_VP_STATUS_FAILURE;
	}
	read_unlock_bh(&node_cfg->lock);

	dp_ops->get_queues(edma_handle, &ppe_queue_start);
	ppe_ds_info("%px: PPE-DS node mapped start queue-id: %d", node, ppe_queue_start);

	vpai->queue_num = ppe_queue_start;
vp_alloc:
	vpai->xmit_port = PPE_DRV_PORT_CPU;

	return ppe_vp_alloc(dev, vpai);
}
EXPORT_SYMBOL(ppe_ds_wlan_vp_alloc);

/*
 * ppe_ds_wlan_get_node_id()
 *	PPE-DS WLAN get node id API
 *	This API is called only in case of MLO
 */
uint32_t ppe_ds_wlan_get_node_id(ppe_ds_wlan_handle_t *wlan_handle)
{
	struct ppe_ds *node;
	struct ppe_ds_node_config *node_cfg;

	if (!wlan_handle) {
		ppe_ds_err("wlan_handle is NULL\n");
		return PPE_VP_DS_INVALID_NODE_ID;
	}

	node = container_of(wlan_handle, struct ppe_ds, wlan_handle);
	node_cfg = &(ppe_ds_node_cfg[node->node_cfg_idx]);

	read_lock_bh(&node_cfg->lock);
	if(node_cfg->node_state != PPE_DS_NODE_STATE_START_DONE) {
		ppe_ds_err("Invalid node state: %d, PPE-DS wlan get node id failed\n",
				node_cfg->node_state);
		read_unlock_bh(&node_cfg->lock);
		return PPE_VP_DS_INVALID_NODE_ID;
	}

	read_unlock_bh(&node_cfg->lock);

	ppe_ds_info("node id of the wlan_handle %d\n", node->node_cfg_idx);

	return node->node_cfg_idx;
}
EXPORT_SYMBOL(ppe_ds_wlan_get_node_id);

/*
 * ppe_ds_wlan_inst_register()
 *	PPE-DS WLAN instance registration API
 *
 * TODO: Currently the undone of works done in ppe_ds_wlan_inst_alloc and
 * ppe_ds_wlan_inst_register APIs is being happening in a single API,
 * (ppe_ds_wlan_inst_free). Try out to see if undone can be separately divided
 * properly.
 */
bool ppe_ds_wlan_inst_register(ppe_ds_wlan_handle_t *wlan_handle, struct ppe_ds_wlan_reg_info *reg_info)
{
	static unsigned int ppeds_node_iter_cnt;
	ppe_ds_node_state_t priv_node_state;
	struct ppe_ds_node_config *node_cfg;
	nss_dp_ppeds_handle_t *edma_handle;
	struct nss_dp_ppeds_ops *dp_ops;
	struct ppe_ds *node;
	unsigned int cpu;
	bool ret;

	if (!wlan_handle) {
		ppe_ds_err("wlan_handle is NULL\n");
		return false;
	}

	node = container_of(wlan_handle, struct ppe_ds, wlan_handle);
	dp_ops = node->dp_ops;

	/*
	 * Validate EDMA operations which are part of data path also before
	 * enabling the HR timer callback
	 */
	if (!dp_ops || !dp_ops->reg || !dp_ops->set_rx_cons_idx ||
			 !dp_ops->set_tx_prod_idx || !dp_ops->get_tx_cons_idx ||
			 !dp_ops->get_rx_prod_idx) {
		ppe_ds_err("NULL EDMA operation in PPE-DS registration API\n");
		return false;
	}

	edma_handle = node->edma_handle;
	node_cfg = &(ppe_ds_node_cfg[node->node_cfg_idx]);

	write_lock_bh(&node_cfg->lock);
	priv_node_state = node_cfg->node_state;
	if ((priv_node_state != PPE_DS_NODE_STATE_ALLOC) &&
		(priv_node_state != PPE_DS_NODE_STATE_STOP_DONE)) {
		printk("Invalid node state: %d, PPE-DS registration failed\n",
				node_cfg->node_state);
		write_unlock_bh(&node_cfg->lock);
		return false;
	}
	node_cfg->node_state = PPE_DS_NODE_STATE_REG_IN_PROG;
	write_unlock_bh(&node_cfg->lock);

	/*
	 * During wifi up/down we could come here, get the current
	 * EDMA producer and consumer indices.
	 */
	if (priv_node_state == PPE_DS_NODE_STATE_STOP_DONE) {
		ppe_ds_get_cur_prod_cons_ring_idx(wlan_handle, reg_info);

		write_lock_bh(&node_cfg->lock);
		node_cfg->node_state = PPE_DS_NODE_STATE_REG_DONE;
		write_unlock_bh(&node_cfg->lock);

		/* Enable the edma interrupt */
		if (!polling_for_idx_update) {
			dp_ops->enable_rx_reap_intr(edma_handle);
		}

		return true;
	}

	if (polling_for_idx_update) {
		/*
		 * Currently assuming the below PPE-DS node to SoC mapping:
		 * 1st PPE-DS node is used by 2G SoC
		 * 2nd PPE-DS node is used by 6g SoC
		 * 3rd PPE-DS node is used by 5g high SoC
		 * 4th PPE-DS node is used by 5g low SoC
		 */
		ppeds_node_iter_cnt++;
		if (ppeds_node_iter_cnt > PPE_DS_MAX_NODE) {
			ppeds_node_iter_cnt = 1;
		}

		if (ppeds_node_iter_cnt == 1) {
			cpu = cpu_mask_2g;
		} else if (ppeds_node_iter_cnt == 2) {
			cpu = cpu_mask_6g;
		} else if (ppeds_node_iter_cnt == 3) {
			cpu = cpu_mask_5g_hi;
		} else if (ppeds_node_iter_cnt == 4) {
			cpu = cpu_mask_5g_lo;
		} else {
			ppe_ds_err("Invalid PPE-DS iteration count: %d\n",
						ppeds_node_iter_cnt);
			return false;
		}

		/*
		 * Setup dummy netdev for all the NAPIs associated with this node
		 */
		init_dummy_netdev(&node->napi_ndev);

		/*
		 * Init high res timer.
		 */
		hrtimer_init_and_bind(&node->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL, cpu);
		node->timer.function = ppe_ds_timer;
		ppe_ds_info("For PPE-DS node iteration count: %d, cpu mask is 0x%x\n",
				ppeds_node_iter_cnt, cpu);
	}

	edma_handle->ppe2tcl_ba = reg_info->ppe2tcl_ba;
	edma_handle->reo2ppe_ba = reg_info->reo2ppe_ba;
	edma_handle->ppe2tcl_num_desc = reg_info->ppe2tcl_num_desc;
	edma_handle->reo2ppe_num_desc = reg_info->reo2ppe_num_desc;
	edma_handle->polling_for_idx_update = polling_for_idx_update;

	if ((ppe2tcl_rxfill_num_desc < PPE_DS_RXFILL_NUM_DESC_MIN) ||
			(ppe2tcl_rxfill_num_desc > PPE_DS_RXFILL_NUM_DESC_MAX)) {
		edma_handle->ppe2tcl_rxfill_num_desc = PPE_DS_RXFILL_NUM_DESC_DEF;
	} else {
		edma_handle->ppe2tcl_rxfill_num_desc = ppe2tcl_rxfill_num_desc;
	}

	if ((reo2ppe_txcmpl_num_desc < PPE_DS_TXCMPL_NUM_DESC_MIN) ||
			(reo2ppe_txcmpl_num_desc > PPE_DS_TXCMPL_NUM_DESC_MAX)) {
		edma_handle->reo2ppe_txcmpl_num_desc = PPE_DS_TXCMPL_NUM_DESC_DEF;
	} else {
		edma_handle->reo2ppe_txcmpl_num_desc = reo2ppe_txcmpl_num_desc;
	}

	if (rxfill_low_threshold >= reg_info->ppe2tcl_num_desc) {
		edma_handle->eth_rxfill_low_thr =
			reg_info->ppe2tcl_num_desc >> PPE_DS_RXFILL_LOW_THRES_DIVISOR;
	} else {
		edma_handle->eth_rxfill_low_thr = rxfill_low_threshold;
	}

	if ((txcmpl_budget < PPE_DS_TXCMPL_MIN_BUDGET) ||
			(txcmpl_budget > edma_handle->reo2ppe_num_desc)) {
		edma_handle->eth_txcomp_budget = PPE_DS_TXCMPL_DEF_BUDGET;
	} else {
		edma_handle->eth_txcomp_budget = txcmpl_budget;
	}

	ppe_ds_info(" ppe2tcl num desc: %d, reo2ppe num desc: %d, txcmpl budget: %d"
			" rxfill low threshold value: %d\n", edma_handle->ppe2tcl_num_desc,
				edma_handle->reo2ppe_num_desc,
				edma_handle->eth_txcomp_budget,
				edma_handle->eth_rxfill_low_thr);

	ret = dp_ops->reg(edma_handle);

	ppe_ds_get_cur_prod_cons_ring_idx(wlan_handle, reg_info);

	write_lock_bh(&node_cfg->lock);
	node_cfg->node_state = PPE_DS_NODE_STATE_REG_DONE;
	write_unlock_bh(&node_cfg->lock);

	/* Enable the edma interrupt */
	if (!polling_for_idx_update) {
		dp_ops->enable_rx_reap_intr(edma_handle);
	}

	ppe_ds_info("%px: PPE-DS register successful", node);
	return ret;
}
EXPORT_SYMBOL(ppe_ds_wlan_inst_register);

/*
 * ppe_ds_wlan_instance_stop()
 *	PPE-DS WLAN instance stop API
 */
void ppe_ds_wlan_instance_stop(ppe_ds_wlan_handle_t *wlan_handle,
			struct ppe_ds_wlan_ctx_info_handle *wlan_info_hdl)
{
	struct ppe_ds *node;
	struct ppe_ds_node_config *node_cfg;
	nss_dp_ppeds_handle_t *edma_handle;
	struct nss_dp_ppeds_ops *dp_ops;
	struct nss_ppe_ds_ctx_info_handle *info_hdl =
		(struct nss_ppe_ds_ctx_info_handle *)wlan_info_hdl;

	if (!wlan_handle) {
		ppe_ds_err("wlan_handle is NULL\n");
		return;
	}

	node = container_of(wlan_handle, struct ppe_ds, wlan_handle);
	dp_ops = node->dp_ops;
	if (!dp_ops || !dp_ops->stop) {
		ppe_ds_err("NULL EDMA operation in PPE-DS stop API\n");
		return;
	}

	node_cfg = &(ppe_ds_node_cfg[node->node_cfg_idx]);
	edma_handle = node->edma_handle;

	node->en_process_irq = false;
	node->umac_reset_inprogress = info_hdl->umac_reset_inprogress;

	write_lock_bh(&node_cfg->lock);
	if(node_cfg->node_state != PPE_DS_NODE_STATE_START_DONE) {
		ppe_ds_err("Invalid node state: %d, PPE-DS stop API failed\n",
				node_cfg->node_state);
		write_unlock_bh(&node_cfg->lock);
		return;
	}
	node_cfg->node_state = PPE_DS_NODE_STATE_STOP_IN_PROG;
	write_unlock_bh(&node_cfg->lock);

	if (polling_for_idx_update) {
		node->timer_enabled = false;
		hrtimer_cancel(&node->timer);
	}

	/*
	 * Stop EDMA.
	 */
	dp_ops->stop(edma_handle, PPE_DS_INTR_ENABLE, info_hdl);

	write_lock_bh(&node_cfg->lock);
	node_cfg->node_state = PPE_DS_NODE_STATE_STOP_DONE;
	write_unlock_bh(&node_cfg->lock);

	ppe_ds_info("%px: PPE-DS stop successful", node);
}
EXPORT_SYMBOL(ppe_ds_wlan_instance_stop);

/*
 * ppe_ds_wlan_inst_stop()
 *	PPE-DS WLAN instance stop API
 */
void ppe_ds_wlan_inst_stop(ppe_ds_wlan_handle_t *wlan_handle)
{
	struct ppe_ds *node = container_of(wlan_handle, struct ppe_ds, wlan_handle);
	struct ppe_ds_node_config *node_cfg = &(ppe_ds_node_cfg[node->node_cfg_idx]);
	nss_dp_ppeds_handle_t *edma_handle = node->edma_handle;
	struct nss_dp_ppeds_ops *dp_ops = node->dp_ops;
	struct nss_ppe_ds_ctx_info_handle info_hdl;

	info_hdl.umac_reset_inprogress = 0;
	node->en_process_irq = false;

	write_lock_bh(&node_cfg->lock);
	if(node_cfg->node_state != PPE_DS_NODE_STATE_START_DONE) {
		ppe_ds_err("Invalid node state: %d, PPE-DS stop API failed\n",
				node_cfg->node_state);
		write_unlock_bh(&node_cfg->lock);
		return;
	}
	node_cfg->node_state = PPE_DS_NODE_STATE_STOP_IN_PROG;
	write_unlock_bh(&node_cfg->lock);

	if (polling_for_idx_update) {
		node->timer_enabled = false;
		hrtimer_cancel(&node->timer);
	}

	if (!dp_ops || !dp_ops->stop) {
		ppe_ds_err("NULL EDMA operation in PPE-DS stop API\n");
		return;
	}

	/*
	 * Stop EDMA.
	 */
	dp_ops->stop(edma_handle, PPE_DS_INTR_ENABLE, &info_hdl);

	write_lock_bh(&node_cfg->lock);
	node_cfg->node_state = PPE_DS_NODE_STATE_STOP_DONE;
	write_unlock_bh(&node_cfg->lock);

	ppe_ds_info("%px: PPE-DS stop successful", node);
}
EXPORT_SYMBOL(ppe_ds_wlan_inst_stop);

/*
 * ppe_ds_wlan_instance_start()
 *	PPE-DS WLAN instance start API
 */
int ppe_ds_wlan_instance_start(ppe_ds_wlan_handle_t *wlan_handle,
			struct ppe_ds_wlan_ctx_info_handle *wlan_info_hdl)
{
	int ret;
	struct ppe_ds *node;
	struct ppe_ds_node_config *node_cfg;
	nss_dp_ppeds_handle_t *edma_handle;
	struct nss_dp_ppeds_ops *dp_ops;
	struct nss_ppe_ds_ctx_info_handle *info_hdl =
		(struct nss_ppe_ds_ctx_info_handle *)wlan_info_hdl;

	if (!wlan_handle) {
		ppe_ds_err("wlan_handle is NULL\n");
		return -1;
	}

	node = container_of(wlan_handle, struct ppe_ds, wlan_handle);
	dp_ops = node->dp_ops;
	if (!dp_ops || !dp_ops->refill || !dp_ops->start) {
		ppe_ds_err("NULL EDMA operation in PPE-DS start API\n");
		return -1;
	}
	node_cfg = &(ppe_ds_node_cfg[node->node_cfg_idx]);
	edma_handle = node->edma_handle;

	write_lock_bh(&node_cfg->lock);
	if(node_cfg->node_state != PPE_DS_NODE_STATE_REG_DONE) {
		ppe_ds_err("Invalid node state: %d, PPE-DS start failed\n",
				node_cfg->node_state);
		write_unlock_bh(&node_cfg->lock);
		return -1;
	}
	node_cfg->node_state = PPE_DS_NODE_STATE_START_IN_PROG;
	write_unlock_bh(&node_cfg->lock);

	dp_ops->refill(edma_handle, edma_handle->ppe2tcl_rxfill_num_desc - 1);

	if (polling_for_idx_update) {
		node->timer_enabled = true;
		hrtimer_start_range_ns_on_cpu(&node->timer,
						ns_to_ktime(IDX_MGMT_PERIOD),
						0, HRTIMER_MODE_REL_PINNED);
	}

	ret = dp_ops->start(edma_handle, PPE_DS_INTR_ENABLE, info_hdl);
	if ((ret != 0) && polling_for_idx_update) {
		node->timer_enabled = false;
		hrtimer_cancel(&node->timer);
	}

	node->umac_reset_inprogress = info_hdl->umac_reset_inprogress;
	write_lock_bh(&node_cfg->lock);
	node_cfg->node_state = PPE_DS_NODE_STATE_START_DONE;
	write_unlock_bh(&node_cfg->lock);

	node->en_process_irq = true;

	ppe_ds_info("%px: PPE-DS start successful\n", node);
	return ret;
}
EXPORT_SYMBOL(ppe_ds_wlan_instance_start);

/*
 * ppe_ds_wlan_inst_start()
 *	PPE-DS WLAN instance start API
 */
int ppe_ds_wlan_inst_start(ppe_ds_wlan_handle_t *wlan_handle)
{
	int ret;
	struct ppe_ds *node = container_of(wlan_handle, struct ppe_ds, wlan_handle);
	struct ppe_ds_node_config *node_cfg = &(ppe_ds_node_cfg[node->node_cfg_idx]);
	nss_dp_ppeds_handle_t *edma_handle = node->edma_handle;
	struct nss_dp_ppeds_ops *dp_ops = node->dp_ops;
	struct nss_ppe_ds_ctx_info_handle info_hdl;

	info_hdl.umac_reset_inprogress = 0;

	write_lock_bh(&node_cfg->lock);
	if(node_cfg->node_state != PPE_DS_NODE_STATE_REG_DONE) {
		ppe_ds_err("Invalid node state: %d, PPE-DS start failed\n",
				node_cfg->node_state);
		write_unlock_bh(&node_cfg->lock);
		return -1;
	}
	node_cfg->node_state = PPE_DS_NODE_STATE_START_IN_PROG;
	write_unlock_bh(&node_cfg->lock);

	if (!dp_ops || !dp_ops->refill || !dp_ops->start) {
		ppe_ds_err("NULL EDMA operation in PPE-DS start API\n");
		return -1;
	}

	dp_ops->refill(edma_handle, edma_handle->ppe2tcl_rxfill_num_desc - 1);

	if (polling_for_idx_update) {
		node->timer_enabled = true;
		hrtimer_start_range_ns_on_cpu(&node->timer,
						ns_to_ktime(IDX_MGMT_PERIOD),
						0, HRTIMER_MODE_REL_PINNED);
	}

	ret = dp_ops->start(edma_handle, PPE_DS_INTR_ENABLE, &info_hdl);
	if ((ret != 0) && polling_for_idx_update) {
		node->timer_enabled = false;
		hrtimer_cancel(&node->timer);
	}

	write_lock_bh(&node_cfg->lock);
	node_cfg->node_state = PPE_DS_NODE_STATE_START_DONE;
	write_unlock_bh(&node_cfg->lock);

	node->en_process_irq = true;

	ppe_ds_info("%px: PPE-DS start successful\n", node);
	return ret;
}
EXPORT_SYMBOL(ppe_ds_wlan_inst_start);

/*
 * ppe_ds_wlan_inst_free)
 *	PPE-DS WLAN instance free API
 */
void ppe_ds_wlan_inst_free(ppe_ds_wlan_handle_t *wlan_handle)
{
	struct ppe_ds *node;
	struct ppe_ds_node_config *node_cfg;
	struct nss_dp_ppeds_ops *dp_ops;
	nss_dp_ppeds_handle_t *edma_handle;

	if (!wlan_handle) {
		ppe_ds_err("wlan_handle is NULL\n");
		return;
	}

	node = container_of(wlan_handle, struct ppe_ds, wlan_handle);
	dp_ops = node->dp_ops;
	if (!dp_ops || !dp_ops->free) {
		ppe_ds_err("NULL EDMA operation in PPE-DS free API\n");
		return;
	}

	node_cfg = &(ppe_ds_node_cfg[node->node_cfg_idx]);
	edma_handle = node->edma_handle;

	write_lock_bh(&node_cfg->lock);
	if(node_cfg->node_state != PPE_DS_NODE_STATE_STOP_DONE) {
		ppe_ds_err("Invalid node state: %d, PPE-DS free failed\n",
				node_cfg->node_state);
		write_unlock_bh(&node_cfg->lock);
		return;
	}
	node_cfg->node_state = PPE_DS_NODE_STATE_FREE_IN_PROG;
	write_unlock_bh(&node_cfg->lock);

	dp_ops->free(edma_handle);

	write_lock_bh(&node_cfg->lock);
	node_cfg->node_state = PPE_DS_NODE_STATE_AVAIL;
	write_unlock_bh(&node_cfg->lock);
	ppe_ds_info("PPE-DS free successful");
}
EXPORT_SYMBOL(ppe_ds_wlan_inst_free);

/*
 * edma_ops
 *	EDMA PPE-DS operation callbacks
 */
static const struct nss_dp_ppeds_cb edma_ops =
{
	.rx = ppe_ds_ppe2tcl_rx,
	.rx_fill = ppe_ds_ppe2tcl_fill,
	.rx_release = ppe_ds_ppe2tcl_rel,
	.tx_cmpl = ppe_ds_reo2ppe_tx_cmpl,
	.enable_wlan_intr = ppe_ds_enable_wlan_intr,
	.notify_napi_done = ppe_ds_notify_napi_done,
};

/*
 * ppe_ds_wlan_inst_alloc()
 *	PPE-DS WLAN instance allocation API
 */
ppe_ds_wlan_handle_t *ppe_ds_wlan_inst_alloc(struct ppe_ds_wlan_ops *ops, size_t priv_size)
{
	struct ppe_ds *node;
	nss_dp_ppeds_handle_t *edma_handle;
	int size = priv_size + sizeof(struct ppe_ds);
	struct nss_dp_ppeds_ops *dp_ops = NULL;
	uint32_t i;

	dp_ops = nss_dp_ppeds_get_ops();
	if (!dp_ops || !dp_ops->alloc) {
		printk("NULL EDMA operation in PPE-DS alloc API\n");
		return NULL;
	}

	for (i = 0; i < PPE_DS_MAX_NODE; i++) {
		write_lock_bh(&ppe_ds_node_cfg[i].lock);
		if (ppe_ds_node_cfg[i].node_state != PPE_DS_NODE_STATE_AVAIL) {
			write_unlock_bh(&ppe_ds_node_cfg[i].lock);
			continue;
		}
		break;
	}

	if(i == PPE_DS_MAX_NODE) {
		printk("Could not get a free PPE-DS node entry\n");
		return NULL;
	}

	ppe_ds_node_cfg[i].node_state = PPE_DS_NODE_STATE_NOT_AVAIL;
	write_unlock_bh(&ppe_ds_node_cfg[i].lock);

	edma_handle = dp_ops->alloc(&edma_ops, size);
	if (!edma_handle) {
		write_lock_bh(&ppe_ds_node_cfg[i].lock);
		ppe_ds_node_cfg[i].node_state = PPE_DS_NODE_STATE_AVAIL;
		write_unlock_bh(&ppe_ds_node_cfg[i].lock);
		printk("Failed to get edma handle. alloc size requested: %d\n", size);
		return NULL;
	}

	node = (struct ppe_ds *)nss_dp_ppeds_priv(edma_handle);
	node->wlan_ops = ops;
	node->dp_ops = dp_ops;
	node->edma_handle = edma_handle;
	node->node_cfg_idx = i;
	node->en_process_irq = false;
	node->umac_reset_inprogress = 0;

	write_lock_bh(&ppe_ds_node_cfg[i].lock);
	ppe_ds_node_cfg[i].node_state = PPE_DS_NODE_STATE_ALLOC;
	write_unlock_bh(&ppe_ds_node_cfg[i].lock);

	printk("%px: PPE-DS alloc successful\n", node);
	return &node->wlan_handle;
}
EXPORT_SYMBOL(ppe_ds_wlan_inst_alloc);
