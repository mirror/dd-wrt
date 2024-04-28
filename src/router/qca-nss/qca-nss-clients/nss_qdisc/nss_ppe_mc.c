/*
 **************************************************************************
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include "nss_qdisc.h"

/*
 * nss_ppe_mcast_queue_reset()
 *	Deconfigures and deallocates a multicast queue in SSDK.
 */
int nss_ppe_mcast_queue_reset(struct nss_qdisc *nq)
{
	fal_shaper_config_t cfg;
	uint32_t offset;
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;


	if (!npq->q.mcast_valid) {
		return 0;
	}

	fal_qm_enqueue_ctrl_set(0, npq->q.mcast_qid, false);
	fal_scheduler_dequeue_ctrl_set(0, npq->q.mcast_qid, false);
	fal_queue_flush(0, port_num, npq->q.mcast_qid);

	/*
	 * In case, multicast queue is attached to a classful qdisc at
	 * its maximum depth, then reset the multicast queue ID in parent qdisc
	 * and reset queue shaper.
	 */
	if (npq->level == NSS_PPE_SUB_QUEUE_LEVEL) {
		nq->parent->npq.q.mcast_qid = 0;
		memset(&cfg, 0, sizeof(cfg));
		nss_qdisc_trace("SSDK multicast queue shaper reset : mcast_qid:%d\n", npq->q.mcast_qid);
		if (fal_queue_shaper_set(0, npq->q.mcast_qid, &cfg) != 0) {
			nss_qdisc_error("SSDK multicast queue shaper configuration failed for port:%d, mcast_qid:%d\n",
				port_num, npq->q.mcast_qid);
			return -EINVAL;
		}
	}

	/*
	 * Free Level 0 multicast queue resource
	 */
	offset = npq->q.mcast_qid - nss_ppe_base_get(port_num, NSS_PPE_MCAST_QUEUE);
	if (nss_ppe_res_free(port_num, offset, NSS_PPE_MCAST_QUEUE) != 0) {
		nss_qdisc_error("Used res:%d not found for port:%d, type:%d \n", npq->q.mcast_qid, port_num, NSS_PPE_MCAST_QUEUE);
		return -EINVAL;
	}

	npq->q.mcast_qid = 0;
	npq->q.mcast_valid = false;

	nss_qdisc_info("SSDK multicast queue reset successful for port:%d\n", port_num);
	return 0;
}

/*
 * nss_ppe_mcast_queue_set()
 *	Allocates and configures a multicast queue in SSDK.
 */
int nss_ppe_mcast_queue_set(struct nss_qdisc *nq)
{
	fal_qos_scheduler_cfg_t l0cfg;
	fal_shaper_token_number_t token;
	fal_shaper_config_t cfg;
	struct nss_ppe_qdisc *npq = &nq->npq;
	struct nss_ppe_res *mcast_q = NULL;
	uint32_t port_num = nss_ppe_port_num_get(nq);

	/*
	 * If multicast queue configuartion is not enabled,
	 * it could be possible that queue's multicast configuration
	 * has changed from enabled to disabled.
	 * So, we need to reset it.
	 */
	 if (!npq->q.mcast_enable) {
		nss_ppe_mcast_queue_reset(nq);
		return 0;
	 }

	/*
	 * If multicast queue configuartion is enabled,
	 * and we have a valid mcast queue, then nothing to do.
	 */
	if (npq->q.mcast_valid) {
		return 0;
	}

	/*
	 * Get Level 0 multicast queue Resource
	 */
	mcast_q = nss_ppe_res_alloc(port_num, NSS_PPE_MCAST_QUEUE);
	if (!mcast_q) {
		nss_qdisc_warning("Free res not found for port:%d, type:%d \n", port_num, NSS_PPE_MCAST_QUEUE);
		return -EINVAL;
	}
	npq->q.mcast_qid = nss_ppe_base_get(port_num, NSS_PPE_MCAST_QUEUE) + mcast_q->offset;
	npq->q.mcast_valid = true;

	nss_qdisc_trace("SSDK level0 configuration get: ucast_qid:%d\n", npq->q.ucast_qid);
	if (fal_queue_scheduler_get(0, npq->q.ucast_qid, NSS_PPE_QUEUE_LEVEL - 1, &port_num, &l0cfg) != 0) {
		nss_qdisc_error("SSDK level0 unicast queue scheduler configuration fetch failed\n");
		goto fail;
	}

	nss_qdisc_trace("SSDK level0 configuration: Port:%d, mcast_qid:%d, c_drrid:%d, c_pri:%d, c_drr_wt:%d, e_drrid:%d, e_pri:%d, e_drr_wt:%d, l0spid:%d\n",
			port_num, npq->q.mcast_qid, l0cfg.c_drr_id, l0cfg.c_pri, l0cfg.c_drr_wt, l0cfg.e_drr_id, l0cfg.e_pri, l0cfg.e_drr_wt, l0cfg.sp_id);
	if (fal_queue_scheduler_set(0, npq->q.mcast_qid, NSS_PPE_QUEUE_LEVEL - 1, port_num, &l0cfg) != 0) {
		nss_qdisc_error("SSDK level0 multicast queue scheduler configuration failed\n");
		goto fail;
	}

	/*
	 * In case, multicast queue is attached at NSS_PPE_SUB_QUEUE_LEVEL level,
	 * set queue shaper for multicast queue.
	 */
	if (npq->level == NSS_PPE_SUB_QUEUE_LEVEL) {
		memset(&token, 0, sizeof(token));
		token.c_token_number = NSS_PPE_TOKEN_MAX;
		token.e_token_number = NSS_PPE_TOKEN_MAX;

		nss_qdisc_trace("SSDK multicast queue token set : mcast_qid:%d, c_token_number:%x, e_token_number:%x\n",
			npq->q.mcast_qid, token.c_token_number, token.e_token_number);
		if (fal_queue_shaper_token_number_set(0, npq->q.mcast_qid, &token) != 0) {
			nss_qdisc_error("SSDK multicast queue shaper token configuration failed\n");
			goto fail;
		}

		if (fal_queue_shaper_get(0, npq->q.ucast_qid, &cfg) != 0) {
			nss_qdisc_error("SSDK queue shaper configuration fetch failed\n");
			goto fail;
		}

		nss_qdisc_trace("SSDK multicast queue shaper configuration: mcast_qid:%d, couple_en:%d, meter_unit:%d, c_shaper_en:%d, cbs:%d, cir:%d, ebs:%d, eir:%d, shaper_frame_mode:%d\n",
			npq->q.mcast_qid, cfg.couple_en, cfg.meter_unit, cfg.c_shaper_en, cfg.cbs, cfg.cir, cfg.ebs, cfg.eir, cfg.shaper_frame_mode);
		if (fal_queue_shaper_set(0, npq->q.mcast_qid, &cfg) != 0) {
			nss_qdisc_error("SSDK multicast queue shaper configuration failed\n");
			goto fail;
		}
	}

	fal_qm_enqueue_ctrl_set(0, npq->q.mcast_qid, true);
	fal_scheduler_dequeue_ctrl_set(0, npq->q.mcast_qid, true);

	nss_qdisc_info("SSDK multicast queue configuration successful for port:%d\n", port_num);
	return 0;

fail:
	nss_ppe_res_free(port_num, mcast_q->offset, NSS_PPE_MCAST_QUEUE);
	npq->q.mcast_qid = 0;
	npq->q.mcast_valid = 0;
	return -EINVAL;
}