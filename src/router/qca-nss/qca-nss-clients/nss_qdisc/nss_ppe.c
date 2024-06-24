/*
 **************************************************************************
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "nss_wred.h"

/*
 * HW scaling factor
 *
 * Change this value to 100 if testing on RUMI.
 * Frequency of RUMI is 100 times slower.
 */
#define NSS_PPE_HW_FREQ_SCALING	1

/*
 * Max Resources per port
 *
 * In case of Loopback port, the resources are reserved
 * for qdisc functionality.
 */
#define NSS_PPE_LOOPBACK_L0_SP_MAX		1
#define NSS_PPE_LOOPBACK_L0_CDRR_MAX		16
#define NSS_PPE_LOOPBACK_L0_EDRR_MAX		16
#define NSS_PPE_LOOPBACK_QUEUE_MAX		16
#define NSS_PPE_LOOPBACK_MCAST_QUEUE_MAX	1

#define NSS_PPE_LOOPBACK_L0_SP_BASE		35
#define NSS_PPE_LOOPBACK_L0_CDRR_BASE		32
#define NSS_PPE_LOOPBACK_L0_EDRR_BASE		32
#define NSS_PPE_LOOPBACK_QUEUE_BASE		128
#define NSS_PPE_LOOPBACK_MCAST_QUEUE_BASE	256

#define NSS_PPE_PRIORITY_MAX		7
#define NSS_PPE_PORT_MAX		8

/*
 * Max number of PRIO bands supported based on level.
 */
#define NSS_PPE_PORT_LEVEL_PRIO_BANDS_MAX	4
#define NSS_PPE_FLOW_LEVEL_PRIO_BANDS_MAX	8

static struct nss_ppe_port ppe_qdisc_port[NSS_PPE_PORT_MAX];

/*
 * nss_ppe_loobback_l1_conf_set()
 *	Sets default L1 queue scheduler in SSDK for port 0.
 */
static int nss_ppe_loobback_l1_conf_set(void)
{
	fal_qos_scheduler_cfg_t l1cfg;

	/*
	 * Set Level 1 Configuration
	 */
	memset(&l1cfg, 0, sizeof(l1cfg));
	l1cfg.sp_id = 0;
	l1cfg.c_pri = 0;
	l1cfg.e_pri = 0;
	l1cfg.c_drr_id = 0;
	l1cfg.e_drr_id = 0;
	l1cfg.c_drr_wt = 1;
	l1cfg.e_drr_wt = 1;

	nss_qdisc_trace("SSDK level1 configuration: Port:0, l0spid:%d, c_drrid:%d, c_pri:%d, c_drr_wt:%d, e_drrid:%d, e_pri:%d, e_drr_wt:%d, l1spid:%d\n",
			NSS_PPE_LOOPBACK_L0_SP_BASE, l1cfg.c_drr_id, l1cfg.c_pri, l1cfg.c_drr_wt, l1cfg.e_drr_id, l1cfg.e_pri, l1cfg.e_drr_wt, l1cfg.sp_id);
	if (fal_queue_scheduler_set(0, NSS_PPE_LOOPBACK_L0_SP_BASE, NSS_PPE_FLOW_LEVEL - 1, 0, &l1cfg) != 0) {
		nss_qdisc_error("SSDK level1 queue scheduler configuration failed\n");
		return -EINVAL;
	}

	nss_qdisc_info("SSDK L1 configuration successful\n");
	return 0;
}

/*
 * nss_ppe_res_entries_free()
 *	Free all the resource for a given port.
 */
static void nss_ppe_res_entries_free(uint32_t port, nss_ppe_res_type_t type)
{
	struct nss_ppe_res *res;
	struct nss_ppe_res *head = NULL;
	struct nss_ppe_port *ppe_port = &ppe_qdisc_port[port];

	spin_lock_bh(&ppe_port->lock);
	head = ppe_port->res_free[type];
	while (head) {
		res = head;
		head = head->next;
		kfree(res);
	}

	ppe_port->res_free[type] = NULL;
	spin_unlock_bh(&ppe_port->lock);

	nss_qdisc_info("port:%d, type:%d\n", port, type);
}

/*
 * nss_ppe_res_entries_alloc()
 *	Allocates all the resources for a given port.
 */
static struct nss_ppe_res *nss_ppe_res_entries_alloc(uint32_t port, nss_ppe_res_type_t type)
{
	struct nss_ppe_res *res = NULL;
	struct nss_ppe_res *next = NULL;
	uint32_t i;
	struct nss_ppe_port *ppe_port = &ppe_qdisc_port[port];
	uint32_t max = ppe_port->max[type];

	spin_lock_bh(&ppe_port->lock);
	for (i = max; i > 0; i--) {
		res = kzalloc(sizeof(struct nss_ppe_res), GFP_ATOMIC);
		if (!res) {
			nss_qdisc_error("Free queue list allocation failed for port %u\n", port);
			goto fail;
		}

		res->offset = i - 1;
		res->type = type;
		res->next = next;
		next = res;

	}
	spin_unlock_bh(&ppe_port->lock);

	nss_qdisc_trace("port:%d, type:%d\n", port, type);
	return res;

fail:
	while (next) {
		res = next;
		next = next->next;
		kfree(res);
	}
	spin_unlock_bh(&ppe_port->lock);
	return NULL;
}

/*
 * nss_ppe_sub_type_is_queue()
 *	Returns true is qdisc is of type queue.
 */
static bool nss_ppe_sub_type_is_queue(struct nss_ppe_qdisc *npq)
{
	if (npq->sub_type < NSS_SHAPER_CONFIG_PPE_SN_SCH_MAX) {
		return false;
	}

	return true;
}

/*
 * nss_ppe_max_get()
 *	Returns maximum number of the particular resource for a given port.
 */
static uint32_t nss_ppe_max_get(uint32_t port, nss_ppe_res_type_t type)
{
	uint32_t max = 0;
	struct nss_ppe_port *ppe_port = &ppe_qdisc_port[port];

	spin_lock_bh(&ppe_port->lock);
	max = ppe_port->max[type];
	spin_unlock_bh(&ppe_port->lock);

	nss_qdisc_info("port:%d, type:%d, max:%d\n", port, type, max);
	return max;
}

/*
 * nss_ppe_port_res_free()
 *	Free resources allocated to PPE ports
 */
int nss_ppe_port_res_free(void)
{
	uint32_t i, j;
	nss_qdisc_info("nss_ppe_port_res_free");

	for (i = 0; i < NSS_PPE_PORT_MAX - 1; i++) {
		for (j = 0; j < NSS_PPE_MAX_RES_TYPE; j++) {
			nss_ppe_res_entries_free(i, j);
		}
	}

	return 0;
}

/*
 * nss_ppe_port_res_alloc()
 *	Allocates per port resources
 *
 *	TODO: Replace the MAX with the configuration from device tree.
 */
int nss_ppe_port_res_alloc(void)
{
	int j, type;
	fal_portscheduler_resource_t cfg;
	int i = 0;
	nss_qdisc_info("nss_ppe_port_res_alloc");

	memset(&ppe_qdisc_port, 0, sizeof(struct nss_ppe_port) * NSS_PPE_PORT_MAX);

	nss_qdisc_info("Resource allocation for loopback port %u\n", i);

	/*
	 * Initialize lock
	 */
	spin_lock_init(&ppe_qdisc_port[i].lock);

	/*
	 * Loopback port configuration
	 * Loopback port requires only L0 resources.
	 */
	ppe_qdisc_port[i].max[NSS_PPE_UCAST_QUEUE] = NSS_PPE_LOOPBACK_QUEUE_MAX;
	ppe_qdisc_port[i].base[NSS_PPE_UCAST_QUEUE] = NSS_PPE_LOOPBACK_QUEUE_BASE;

	ppe_qdisc_port[i].max[NSS_PPE_MCAST_QUEUE] = NSS_PPE_LOOPBACK_MCAST_QUEUE_MAX;
	ppe_qdisc_port[i].base[NSS_PPE_MCAST_QUEUE] = NSS_PPE_LOOPBACK_MCAST_QUEUE_BASE;

	ppe_qdisc_port[i].max[NSS_PPE_L0_CDRR] = NSS_PPE_LOOPBACK_L0_CDRR_MAX;
	ppe_qdisc_port[i].base[NSS_PPE_L0_CDRR] = NSS_PPE_LOOPBACK_L0_CDRR_BASE;

	ppe_qdisc_port[i].max[NSS_PPE_L0_EDRR] = NSS_PPE_LOOPBACK_L0_EDRR_MAX;
	ppe_qdisc_port[i].base[NSS_PPE_L0_EDRR] = NSS_PPE_LOOPBACK_L0_EDRR_BASE;

	ppe_qdisc_port[i].max[NSS_PPE_L0_SP] = NSS_PPE_LOOPBACK_L0_SP_MAX;
	ppe_qdisc_port[i].base[NSS_PPE_L0_SP] = NSS_PPE_LOOPBACK_L0_SP_BASE;

	for (type = 0; type <= NSS_PPE_L0_SP; type++) {
		ppe_qdisc_port[i].res_free[type] = nss_ppe_res_entries_alloc(i, type);
		if (!ppe_qdisc_port[i].res_free[type]) {
			nss_qdisc_error("Resource list allocation failed for port:%u type:%u\n", i, type);
			goto failure;
		}
	}

	/*
	 * Allocate resources for GMAC switch ports.
	 */
	for (i = 1; i < NSS_PPE_PORT_MAX - 1; i++) {
		nss_qdisc_info("Resource allocation for port %u\n", i);

		/*
		 * Initialize locks
		 */
		spin_lock_init(&ppe_qdisc_port[i].lock);

		/*
		 * Resource configuration
		 */
		memset(&cfg, 0, sizeof(cfg));

		if (fal_port_scheduler_resource_get(0, i, &cfg) != 0) {
			nss_qdisc_error("Fetching of port scheduler resource information failed\n");
			goto failure;
		}

		ppe_qdisc_port[i].max[NSS_PPE_UCAST_QUEUE] = cfg.ucastq_num;
		ppe_qdisc_port[i].base[NSS_PPE_UCAST_QUEUE] = cfg.ucastq_start;

		/*
		 * Even though we reserve more mcast queues in the device tree, we only use 1 in qdiscs
		 * for the default queue.
		 */
		ppe_qdisc_port[i].max[NSS_PPE_MCAST_QUEUE] = cfg.mcastq_num;
		ppe_qdisc_port[i].base[NSS_PPE_MCAST_QUEUE] = cfg.mcastq_start;

		ppe_qdisc_port[i].max[NSS_PPE_L0_CDRR] = cfg.l0cdrr_num;
		ppe_qdisc_port[i].base[NSS_PPE_L0_CDRR] = cfg.l0cdrr_start;

		ppe_qdisc_port[i].max[NSS_PPE_L0_EDRR] = cfg.l0edrr_num;
		ppe_qdisc_port[i].base[NSS_PPE_L0_EDRR] = cfg.l0edrr_start;

		ppe_qdisc_port[i].max[NSS_PPE_L0_SP] = cfg.l0sp_num;
		ppe_qdisc_port[i].base[NSS_PPE_L0_SP] = cfg.l0sp_start;

		ppe_qdisc_port[i].max[NSS_PPE_L1_CDRR] = cfg.l1cdrr_num;
		ppe_qdisc_port[i].base[NSS_PPE_L1_CDRR] = cfg.l1cdrr_start;

		ppe_qdisc_port[i].max[NSS_PPE_L1_EDRR] = cfg.l1edrr_num;
		ppe_qdisc_port[i].base[NSS_PPE_L1_EDRR] = cfg.l1edrr_start;

		for (type = 0; type < NSS_PPE_MAX_RES_TYPE; type++) {
			ppe_qdisc_port[i].res_free[type] = nss_ppe_res_entries_alloc(i, type);
			if (!ppe_qdisc_port[i].res_free[type]) {
				nss_qdisc_error("Resource list allocation failed for port:%u type:%u\n", i, type);
				goto failure;
			}
		}
	}

	/*
	 * Set default level 1 configuration for loopback port.
	 */
	if (nss_ppe_loobback_l1_conf_set() != 0) {
		nss_qdisc_error("Failed default level 1 configuration for brige port\n");
		goto failure;
	}
	return 0;

failure:
	/*
	 * In case we have successfully allocated resources for all the ports
	 * and failure occurs while setting default level 1 configuration
	 * for loobpack port, then decrement the counter to deallocate resources.
	 */
	if (i == (NSS_PPE_PORT_MAX - 1)) {
		i--;
	}

	while (i >= 0) {
		for (j = 0; j < NSS_PPE_MAX_RES_TYPE; j++) {
			nss_ppe_res_entries_free(i, j);
		}
		i--;
	}
	return -EINVAL;
}

/*
 * nss_ppe_queue_disable()
 *	Disables a queue in SSDK.
 */
static void nss_ppe_queue_disable(struct nss_qdisc *nq)
{
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	if (!npq->q.ucast_valid) {
		return;
	}

	/*
	 * Disable queue enqueue, dequeue and flush the queue.
	 */
	fal_qm_enqueue_ctrl_set(0, npq->q.ucast_qid, false);
	fal_scheduler_dequeue_ctrl_set(0, npq->q.ucast_qid, false);
	fal_queue_flush(0, port_num, npq->q.ucast_qid);

	nss_qdisc_info("Disable SSDK level0 queue scheduler successful\n");
}

/*
 * nss_ppe_queue_enable()
 *	Enables a queue in SSDK.
 */
static void nss_ppe_queue_enable(struct nss_qdisc *nq)
{
	struct nss_ppe_qdisc *npq = &nq->npq;

	if (!npq->q.ucast_valid) {
		return;
	}

	/*
	 * Enable queue enqueue and dequeue.
	 */
	fal_qm_enqueue_ctrl_set(0, npq->q.ucast_qid, true);
	fal_scheduler_dequeue_ctrl_set(0, npq->q.ucast_qid, true);

	nss_qdisc_info("Enable SSDK level0 queue scheduler successful\n");
}

/*
 * nss_ppe_l1_res_free()
 *	Frees Level 1 scheduler resources.
 */
static int nss_ppe_l1_res_free(struct nss_qdisc *nq)
{
	uint32_t offset;
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	if (!npq->l1_valid) {
		return 0;
	}

	/*
	 * Loopback will have one level less than the max shaper levels.
	 * L1 scheduler was configured at init time, so resources were allocated.
	 */
	if (nq->needs_ppe_loopback) {
		offset = npq->l0spid - nss_ppe_base_get(port_num, NSS_PPE_L0_SP);
		if (nss_ppe_res_free(port_num, offset, NSS_PPE_L0_SP) != 0) {
			nss_qdisc_error("Used res:%d not found for port:%d, type:%d\n", npq->l0spid, port_num, NSS_PPE_L0_SP);
			return -EINVAL;
		}

		npq->l0spid = 0;
		npq->l1_valid = false;
		nss_qdisc_trace("loopback resource freed\n");
		return 0;
	}

	/*
	 * Free Level 1 DRR resource
	 */
	offset = npq->l1c_drrid - nss_ppe_base_get(port_num, NSS_PPE_L1_CDRR);
	if (nss_ppe_res_free(port_num, offset, NSS_PPE_L1_CDRR) != 0) {
		nss_qdisc_error("Used res:%d not found for port:%d, type:%d \n", npq->l1c_drrid, port_num, NSS_PPE_L1_CDRR);
		return -EINVAL;
	}

	offset = npq->l1e_drrid - nss_ppe_base_get(port_num, NSS_PPE_L1_EDRR);
	if (nss_ppe_res_free(port_num, offset, NSS_PPE_L1_EDRR) != 0) {
		nss_qdisc_error("Used res:%d not found for port:%d, type:%d \n", npq->l1e_drrid, port_num, NSS_PPE_L1_EDRR);
		return -EINVAL;
	}

	/*
	 * Free Level 0 SP resource
	 */
	offset = npq->l0spid - nss_ppe_base_get(port_num, NSS_PPE_L0_SP);
	if (nss_ppe_res_free(port_num, offset, NSS_PPE_L0_SP) != 0) {
		nss_qdisc_error("Used res:%d not found for port:%d, type:%d \n", npq->l0spid, port_num, NSS_PPE_L0_SP);
		return -EINVAL;
	}

	/*
	 * Reset Res id values in qdisc
	 */
	npq->l0spid = 0;
	npq->l1c_drrid = 0;
	npq->l1e_drrid = 0;
	npq->l1_valid = false;

	nss_qdisc_info("SSDK level1 queue scheduler configuration successful\n");
	return 0;
}

/*
 * nss_ppe_l1_res_alloc()
 *	Allocates Level 1 scheduler resources
 */
static int nss_ppe_l1_res_alloc(struct nss_qdisc *nq)
{
	struct nss_ppe_res *l1c_drr = NULL;
	struct nss_ppe_res *l1e_drr = NULL;
	struct nss_ppe_res *l0sp = NULL;
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	/*
	 * Get Level 0 SP resource
	 */
	l0sp = nss_ppe_res_alloc(port_num, NSS_PPE_L0_SP);
	if (!l0sp) {
		nss_qdisc_warning("Free res not found for port:%d, type:%d \n", port_num, NSS_PPE_L0_SP);
		goto fail;
	}

	/*
	 * Loopback has no dedicated L1 schedulers. L0 SP is the only resource we
	 * need to allocate.
	 */
	if (nq->needs_ppe_loopback) {
		npq->l0spid = nss_ppe_base_get(port_num, NSS_PPE_L0_SP) + l0sp->offset;
		npq->l1_valid = true;
		nss_qdisc_info("Level1 scheduler resource allocation successful\n");
		return 0;
	}

	/*
	 * Get Level 1 DRR resource
	 */
	l1c_drr = nss_ppe_res_alloc(port_num, NSS_PPE_L1_CDRR);
	if (!l1c_drr) {
		nss_qdisc_warning("Free res not found for port:%d, type:%d \n", port_num, NSS_PPE_L1_CDRR);
		goto fail;
	}

	l1e_drr = nss_ppe_res_alloc(port_num, NSS_PPE_L1_EDRR);
	if (!l1e_drr) {
		nss_qdisc_warning("Free res not found for port:%d, type:%d \n", port_num, NSS_PPE_L1_EDRR);
		goto fail;
	}

	/*
	 * Set Res id values in qdisc
	 */
	npq->q.ucast_qid = 0;
	npq->q.mcast_qid = 0;
	npq->q.ucast_valid = false;
	npq->l1_valid = true;
	npq->l0c_drrid = 0;
	npq->l0e_drrid = 0;
	npq->l0spid = nss_ppe_base_get(port_num, NSS_PPE_L0_SP) + l0sp->offset;
	npq->l1c_drrid = nss_ppe_base_get(port_num, NSS_PPE_L1_CDRR) + l1c_drr->offset;
	npq->l1e_drrid = nss_ppe_base_get(port_num, NSS_PPE_L1_EDRR) + l1e_drr->offset;

	nss_qdisc_info("Level1 scheduler resource allocation successful\n");
	return 0;

fail:
	if (l0sp) {
		nss_ppe_res_free(port_num, l0sp->offset, NSS_PPE_L0_SP);
	}

	if (l1e_drr) {
		nss_ppe_res_free(port_num, l1e_drr->offset, NSS_PPE_L1_EDRR);
	}

	if (l1c_drr) {
		nss_ppe_res_free(port_num, l1c_drr->offset, NSS_PPE_L1_CDRR);
	}
	return -EINVAL;
}

/*
 * nss_ppe_all_queue_disable()
 *	Disables all queues corresponding to a port in SSDK.
 */
static void nss_ppe_all_queue_disable(uint32_t port_num)
{
	uint32_t qid = nss_ppe_base_get(port_num, NSS_PPE_UCAST_QUEUE);
	uint32_t mcast_qid = nss_ppe_base_get(port_num, NSS_PPE_MCAST_QUEUE);
	uint32_t offset;

	/*
	 * Disable queue enqueue, dequeue and flush the queue.
	 */
	for (offset = 0; offset < nss_ppe_max_get(port_num, NSS_PPE_UCAST_QUEUE); offset++) {
		fal_qm_enqueue_ctrl_set(0, qid + offset, false);
		fal_scheduler_dequeue_ctrl_set(0, qid + offset, false);
		fal_queue_flush(0, port_num, qid + offset);
	}

	for (offset = 0; offset < nss_ppe_max_get(port_num, NSS_PPE_MCAST_QUEUE); offset++) {
		fal_qm_enqueue_ctrl_set(0, mcast_qid + offset, false);
		fal_scheduler_dequeue_ctrl_set(0, mcast_qid + offset, false);
		fal_queue_flush(0, port_num, mcast_qid + offset);
	}

	nss_qdisc_info("Disable SSDK level0 queue scheduler successful\n");
}

/*
 * nss_ppe_all_queue_enable()
 *	Enables all level L0 queues corresponding to a port in SSDK.
 */
static void nss_ppe_all_queue_enable(uint32_t port_num)
{
	uint32_t qid = nss_ppe_base_get(port_num, NSS_PPE_UCAST_QUEUE);
	uint32_t mcast_qid = nss_ppe_base_get(port_num, NSS_PPE_MCAST_QUEUE);
	uint32_t offset;

	/*
	 * Enable queue enqueue and dequeue.
	 */
	for (offset = 0; offset < nss_ppe_max_get(port_num, NSS_PPE_UCAST_QUEUE); offset++) {
		fal_qm_enqueue_ctrl_set(0, qid + offset, true);
		fal_scheduler_dequeue_ctrl_set(0, qid + offset, true);
	}

	for (offset = 0; offset < nss_ppe_max_get(port_num, NSS_PPE_MCAST_QUEUE); offset++) {
		fal_qm_enqueue_ctrl_set(0, mcast_qid + offset, true);
		fal_scheduler_dequeue_ctrl_set(0, mcast_qid + offset, true);
	}

	nss_qdisc_info("Enable SSDK level0 queue scheduler successful\n");
}

/*
 * nss_ppe_assigned_queue_enable()
 *	Enables all level L0 queues corresponding to a port in SSDK.
 */
static void nss_ppe_assigned_queue_enable(uint32_t port_num)
{
	uint32_t qid = nss_ppe_base_get(port_num, NSS_PPE_UCAST_QUEUE);
	uint32_t mcast_qid = nss_ppe_base_get(port_num, NSS_PPE_MCAST_QUEUE);
	struct nss_ppe_res *res;
	struct nss_ppe_port *ppe_port = &ppe_qdisc_port[port_num];

	spin_lock_bh(&ppe_port->lock);
	res = ppe_port->res_used[NSS_PPE_UCAST_QUEUE];
	while (res) {
		fal_qm_enqueue_ctrl_set(0, qid + res->offset, true);
		fal_scheduler_dequeue_ctrl_set(0, qid + res->offset, true);
		res = res->next;
	}

	res = ppe_port->res_used[NSS_PPE_MCAST_QUEUE];
	while (res) {
		fal_qm_enqueue_ctrl_set(0, mcast_qid + res->offset, true);
		fal_scheduler_dequeue_ctrl_set(0, mcast_qid + res->offset, true);
		res = res->next;
	}

	spin_unlock_bh(&ppe_port->lock);
	nss_qdisc_info("Enable SSDK level0 queue scheduler successful\n");
}

/*
 * nss_ppe_l1_queue_scheduler_configure()
 *	Configures Level 1 queue scheduler in SSDK.
 */
static int nss_ppe_l1_queue_scheduler_configure(struct nss_qdisc *nq)
{
	fal_qos_scheduler_cfg_t l1cfg;
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	/*
	 * Disable all queues and set Level 1 SSDK configuration
	 * We need to disable and flush the queues before
	 * changing scheduler's sp_id/drr_id/priority.
	 */
	nss_ppe_all_queue_disable(port_num);

	if (npq->scheduler.drr_weight >= NSS_PPE_DRR_WEIGHT_MAX) {
		/*
		 * Currently assigned queues are enabled back by
		 * caller
		 */
		nss_qdisc_warning("DRR weight:%d should be less than 1024\n", npq->scheduler.drr_weight);
		return -EINVAL;
	}

	memset(&l1cfg, 0, sizeof(l1cfg));
	l1cfg.sp_id = port_num;

	l1cfg.c_drr_wt = npq->scheduler.drr_weight ? npq->scheduler.drr_weight : 1;
	l1cfg.c_drr_unit = npq->scheduler.drr_unit;
	l1cfg.e_drr_wt = npq->scheduler.drr_weight ? npq->scheduler.drr_weight : 1;
	l1cfg.e_drr_unit = npq->scheduler.drr_unit;
	l1cfg.c_pri = NSS_PPE_PRIORITY_MAX - npq->scheduler.priority;
	l1cfg.e_pri = NSS_PPE_PRIORITY_MAX - npq->scheduler.priority;
	l1cfg.c_drr_id = npq->l1c_drrid;
	l1cfg.e_drr_id = npq->l1e_drrid;
	l1cfg.drr_frame_mode = FAL_DRR_FRAME_CRC;

	nss_qdisc_trace("SSDK level1 configuration: Port:%d, l0spid:%d, c_drrid:%d, c_pri:%d, c_drr_wt:%d, e_drrid:%d, e_pri:%d, e_drr_wt:%d, l1spid:%d\n",
			port_num, npq->l0spid, l1cfg.c_drr_id, l1cfg.c_pri, l1cfg.c_drr_wt, l1cfg.e_drr_id, l1cfg.e_pri, l1cfg.e_drr_wt, l1cfg.sp_id);
	if (fal_queue_scheduler_set(0, npq->l0spid, NSS_PPE_FLOW_LEVEL - 1, port_num, &l1cfg) != 0) {
		nss_qdisc_error("SSDK level1 queue scheduler configuration failed\n");
		return -EINVAL;
	}

	nss_ppe_assigned_queue_enable(port_num);

	nss_qdisc_info("SSDK level1 queue scheduler configuration successful\n");
	return 0;
}

/*
 * nss_ppe_l1_queue_scheduler_set()
 *	Configures Level 1 queue scheduler in SSDK.
 */
static int nss_ppe_l1_queue_scheduler_set(struct nss_qdisc *nq)
{
	struct nss_ppe_qdisc *npq = &nq->npq;

	/*
	 * Loopback will have one level less than the max shaper levels.
	 * L1 scheduler was configured at init time, so no need to allocate resources.
	 */
	if (nq->needs_ppe_loopback) {
		/*
		 * Allocate resource if we have not already done so.
		 */
		if (!npq->l1_valid && nss_ppe_l1_res_alloc(nq) != 0) {
			nss_qdisc_warning("SSDK level1 queue scheduler configuration failed\n");
			return -EINVAL;
		}

		nss_qdisc_info("Allocated spid:%d expected:%d\n", npq->l0spid, NSS_PPE_LOOPBACK_L0_SP_BASE);

		/*
		 * L0 SP is the only resource we allocate in loopback, and that
		 * does not require additional configuration. The next level of
		 * C/E DRR will attach to the right priority slot.
		 *
		 * We simply return.
		 */
		nss_qdisc_info("SSDK level1 queue scheduler configuration successful\n");
		return 0;
	}

	/*
	 * Allocate resources if we have not already done so.
	 */
	if (!npq->l1_valid && nss_ppe_l1_res_alloc(nq) != 0) {
		nss_qdisc_warning("SSDK level1 queue scheduler configuration failed\n");
		return -EINVAL;
	}

	/*
	 * Set Level 1 configuration
	 */
	if (nss_ppe_l1_queue_scheduler_configure(nq) != 0) {
		nss_qdisc_error("SSDK level1 queue scheduler configuration failed\n");
		nss_ppe_l1_res_free(nq);
		nss_ppe_assigned_queue_enable(nss_ppe_port_num_get(nq));
		return -EINVAL;
	}

	nss_qdisc_info("SSDK level1 queue scheduler configuration successful\n");
	return 0;
}

/*
 * nss_ppe_l0_res_free()
 *	Frees level0 scheduler resources.
 */
static int nss_ppe_l0_res_free(struct nss_qdisc *nq)
{
	uint32_t offset;
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	if (!npq->l0_valid) {
		return 0;
	}

	/*
	 * Free Level 0 DRR resource
	 */
	offset = npq->l0c_drrid - nss_ppe_base_get(port_num, NSS_PPE_L0_CDRR);
	if (nss_ppe_res_free(port_num, offset, NSS_PPE_L0_CDRR) != 0) {
		nss_qdisc_error("Used res:%d not found for port:%d, type:%d \n", npq->l0c_drrid, port_num, NSS_PPE_L0_CDRR);
		return -EINVAL;
	}

	offset = npq->l0e_drrid - nss_ppe_base_get(port_num, NSS_PPE_L0_EDRR);
	if (nss_ppe_res_free(port_num, offset, NSS_PPE_L0_EDRR) != 0) {
		nss_qdisc_error("Used res:%d not found for port:%d, type:%d \n", npq->l0e_drrid, port_num, NSS_PPE_L0_EDRR);
		return -EINVAL;
	}

	/*
	 * Free Level 0 queue resource
	 */
	offset = npq->q.ucast_qid - nss_ppe_base_get(port_num, NSS_PPE_UCAST_QUEUE);
	if (nss_ppe_res_free(port_num, offset, NSS_PPE_UCAST_QUEUE) != 0) {
		nss_qdisc_error("Used res:%d not found for port:%d, type:%d \n", npq->q.ucast_qid, port_num, NSS_PPE_UCAST_QUEUE);
		return -EINVAL;
	}

	/*
	 * Reset Res id values in qdisc
	 */
	npq->q.ucast_qid = 0;
	npq->q.mcast_qid = 0;
	npq->q.mcast_valid = false;
	npq->q.ucast_valid = false;
	npq->l0c_drrid = 0;
	npq->l0e_drrid = 0;
	npq->l0_valid = false;

	nss_qdisc_info("Level0 scheduler resource de-allocation successful\n");
	return 0;
}

/*
 * nss_ppe_l0_queue_scheduler_deconfigure()
 *	De-configures Level 0 queue scheduler configuration in SSDK.
 */
static int nss_ppe_l0_queue_scheduler_deconfigure(struct nss_qdisc *nq)
{
	fal_qos_scheduler_cfg_t l0cfg;
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	nss_ppe_all_queue_disable(port_num);

	/*
	 * Reset Level 0 configuration
	 */
	memset(&l0cfg, 0, sizeof(l0cfg));
	l0cfg.sp_id = npq->l0spid;
	l0cfg.c_drr_wt = 0;
	l0cfg.e_drr_wt = 0;
	l0cfg.c_drr_id = npq->l0c_drrid;
	l0cfg.e_drr_id = npq->l0e_drrid;

	nss_qdisc_trace("SSDK level0 configuration: Port:%d, ucast_qid:%d, c_drrid:%d, c_pri:%d, c_drr_wt:%d, e_drrid:%d, e_pri:%d, e_drr_wt:%d, l0spid:%d\n",
			port_num, npq->q.ucast_qid, l0cfg.c_drr_id, l0cfg.c_pri, l0cfg.c_drr_wt, l0cfg.e_drr_id, l0cfg.e_pri, l0cfg.e_drr_wt, l0cfg.sp_id);
	if (fal_queue_scheduler_set(0, npq->q.ucast_qid, NSS_PPE_QUEUE_LEVEL - 1, port_num, &l0cfg) != 0) {
		nss_qdisc_error("SSDK level0 queue scheduler configuration failed\n");
		nss_ppe_assigned_queue_enable(port_num);
		return -EINVAL;
	}

	/*
	 * Assinged queues are enabled after the current resource is freed.
	 */

	nss_qdisc_info("SSDK level0 queue scheduler configuration successful\n");
	return 0;
}

/*
 * nss_ppe_l0_queue_scheduler_reset()
 *	Resets Level 0 queue scheduler configuration in SSDK.
 */
static int nss_ppe_l0_queue_scheduler_reset(struct nss_qdisc *nq)
{
	if (nss_ppe_l0_queue_scheduler_deconfigure(nq) != 0) {
		nss_qdisc_error("SSDK level0 queue scheduler configuration failed\n");
		return -EINVAL;
	}

	if (nss_ppe_l0_res_free(nq) != 0) {
		nss_qdisc_error("Level0 scheduler resources de-allocation failed\n");
		nss_ppe_assigned_queue_enable(nss_ppe_port_num_get(nq));
		return -EINVAL;
	}

	nss_ppe_assigned_queue_enable(nss_ppe_port_num_get(nq));
	nss_qdisc_info("SSDK level0 queue scheduler configuration successful\n");
	return 0;
}

/*
 * nss_ppe_l0_res_alloc()
 *	Allocates level 0 resources.
 */
static int nss_ppe_l0_res_alloc(struct nss_qdisc *nq)
{
	struct nss_ppe_res *l0c_drr = NULL;
	struct nss_ppe_res *l0e_drr = NULL;
	struct nss_ppe_res *q = NULL;
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	/*
	 * Get Level 0 DRR Resource
	 */
	l0c_drr = nss_ppe_res_alloc(port_num, NSS_PPE_L0_CDRR);
	if (!l0c_drr) {
		nss_qdisc_warning("Free res not found for port:%d, type:%d \n", port_num, NSS_PPE_L0_CDRR);
		goto fail;
	}

	l0e_drr = nss_ppe_res_alloc(port_num, NSS_PPE_L0_EDRR);
	if (!l0e_drr) {
		nss_qdisc_warning("Free res not found for port:%d, type:%d \n", port_num, NSS_PPE_L0_EDRR);
		goto fail;
	}

	/*
	 * Get Level 0 queue Resource
	 */
	q = nss_ppe_res_alloc(port_num, NSS_PPE_UCAST_QUEUE);
	if (!q) {
		nss_qdisc_warning("Free res not found for port:%d, type:%d \n", port_num, NSS_PPE_UCAST_QUEUE);
		goto fail;
	}

	/*
	 * Set res id values in qdisc
	 */
	npq->q.ucast_qid = nss_ppe_base_get(port_num, NSS_PPE_UCAST_QUEUE) + q->offset;
	npq->q.ucast_valid = true;
	npq->l0_valid = true;
	npq->l0c_drrid = nss_ppe_base_get(port_num, NSS_PPE_L0_CDRR) + l0c_drr->offset;
	npq->l0e_drrid = nss_ppe_base_get(port_num, NSS_PPE_L0_EDRR) + l0e_drr->offset;
	npq->q.mcast_qid = 0;

	/*
	 * If a qdisc or class is attached at queue level,
	 * we need to set the L0 SP Id from the parent qdisc
	 * to make connection between L0 and L1 resources.
	 */
	if (npq->level == NSS_PPE_QUEUE_LEVEL) {
		npq->l0spid = nq->parent->npq.l0spid;
	}

	nss_qdisc_info("Level0 scheduler resource allocation successful\n");
	return 0;

fail:
	if (q) {
		nss_ppe_res_free(port_num, q->offset, NSS_PPE_UCAST_QUEUE);
	}

	if (l0c_drr) {
		nss_ppe_res_free(port_num, l0c_drr->offset, NSS_PPE_L0_CDRR);
	}

	if (l0e_drr) {
		nss_ppe_res_free(port_num, l0e_drr->offset, NSS_PPE_L0_EDRR);
	}
	return -EINVAL;
}

/*
 * nss_ppe_l0_queue_scheduler_configure()
 *	Configures a level 0 queue scheduler in SSDK.
 */
static int nss_ppe_l0_queue_scheduler_configure(struct nss_qdisc *nq)
{
	fal_qos_scheduler_cfg_t l0cfg;
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	/*
	 * Disable all queues and set Level 0 SSDK configuration
	 * We need to disable and flush the queues before
	 * changing scheduler's sp_id/drr_id/priority.
	 */
	nss_ppe_all_queue_disable(port_num);

	if (npq->scheduler.drr_weight >= NSS_PPE_DRR_WEIGHT_MAX) {
		/*
		 * Currently assigned queues are enabled back by
		 * caller
		 */
		nss_qdisc_warning("DRR weight:%d should be less than 1024\n", npq->scheduler.drr_weight);
		return -EINVAL;
	}

	memset(&l0cfg, 0, sizeof(l0cfg));
	l0cfg.sp_id = npq->l0spid;
	l0cfg.c_drr_wt = npq->scheduler.drr_weight ? npq->scheduler.drr_weight : 1;
	l0cfg.c_drr_unit = npq->scheduler.drr_unit;
	l0cfg.e_drr_wt = npq->scheduler.drr_weight ? npq->scheduler.drr_weight : 1;
	l0cfg.e_drr_unit = npq->scheduler.drr_unit;
	l0cfg.c_pri = NSS_PPE_PRIORITY_MAX - npq->scheduler.priority;
	l0cfg.e_pri = NSS_PPE_PRIORITY_MAX - npq->scheduler.priority;
	l0cfg.c_drr_id = npq->l0c_drrid;
	l0cfg.e_drr_id = npq->l0e_drrid;
	l0cfg.drr_frame_mode = FAL_DRR_FRAME_CRC;

	nss_qdisc_trace("SSDK level0 configuration: Port:%d, ucast_qid:%d, c_drrid:%d, c_pri:%d, c_drr_wt:%d, e_drrid:%d, e_pri:%d, e_drr_wt:%d, l0spid:%d\n",
			port_num, npq->q.ucast_qid, l0cfg.c_drr_id, l0cfg.c_pri, l0cfg.c_drr_wt, l0cfg.e_drr_id, l0cfg.e_pri, l0cfg.e_drr_wt, l0cfg.sp_id);
	if (fal_queue_scheduler_set(0, npq->q.ucast_qid, NSS_PPE_QUEUE_LEVEL - 1, port_num, &l0cfg) != 0) {
		nss_qdisc_error("SSDK level0 queue scheduler configuration failed\n");
		return -EINVAL;
	}

	/*
	 * This function is called at the time of queue qdisc allocation at all
	 * levels and class allocation at NSS_PPE_QUEUE_LEVEL.
	 * Also invoked at time of change class at NSS_PPE_QUEUE_LEVEL and
	 * NSS_PPE_SUB_QUEUE_LEVEL levels. But, mcast will be valid only in
	 * case queue is attached at NSS_PPE_SUB_QUEUE_LEVEL and change class
	 * is invoked at NSS_PPE_QUEUE_LEVEL level.
	 * Other scenarios catering to mcast queue are handled in separate mcast APIs.
	 */
	if (npq->q.mcast_qid) {
		nss_qdisc_trace("SSDK level0 configuration: Port:%d, mcast_qid:%d, c_drrid:%d, c_pri:%d, c_drr_wt:%d, e_drrid:%d, e_pri:%d, e_drr_wt:%d, l0spid:%d\n",
				port_num, npq->q.mcast_qid, l0cfg.c_drr_id, l0cfg.c_pri, l0cfg.c_drr_wt, l0cfg.e_drr_id, l0cfg.e_pri, l0cfg.e_drr_wt, l0cfg.sp_id);
		if (fal_queue_scheduler_set(0, npq->q.mcast_qid, NSS_PPE_QUEUE_LEVEL - 1, port_num, &l0cfg) != 0) {
			nss_qdisc_error("SSDK level0 multicast queue scheduler configuration failed\n");
			return -EINVAL;
		}
	}

	nss_ppe_assigned_queue_enable(port_num);

	nss_qdisc_info("SSDK level0 queue scheduler configuration successful\n");
	return 0;
}

/*
 * nss_ppe_l0_queue_scheduler_set()
 *	Configures a level 0 queue scheduler in SSDK.
 */
static int nss_ppe_l0_queue_scheduler_set(struct nss_qdisc *nq)
{
	struct nss_ppe_qdisc *npq = &nq->npq;

	/*
	 * Allocate resources only if scheduler is not valid
	 */
	if (!npq->l0_valid) {
		/*
		 * Allocate Level 0 resources
		 */
		if (nss_ppe_l0_res_alloc(nq) != 0) {
			nss_qdisc_warning("SSDK level0 queue scheduler configuration failed\n");
			return -EINVAL;
		}
	}

	/*
	 * Set Level 0 configuration
	 */
	if (nss_ppe_l0_queue_scheduler_configure(nq) != 0) {
		nss_qdisc_error("SSDK level0 queue scheduler configuration failed\n");
		nss_ppe_l0_res_free(nq);
		nss_ppe_assigned_queue_enable(nss_ppe_port_num_get(nq));
		return -EINVAL;
	}

	nss_qdisc_info("SSDK level0 queue scheduler configuration successful\n");
	return 0;
}

/*
 * nss_ppe_port_shaper_reset()
 *	Resets a port shaper in SSDK.
 */
static int nss_ppe_port_shaper_reset(struct nss_qdisc *nq)
{
	fal_shaper_config_t cfg;
	uint32_t port_num = nss_ppe_port_num_get(nq);

	memset(&cfg, 0, sizeof(cfg));
	nss_qdisc_trace("SSDK port shaper reset : Port:%d\n", port_num);
	if (fal_port_shaper_set(0, port_num, &cfg) != 0) {
		nss_qdisc_error("SSDK port shaper configuration failed for port:%d\n", port_num);
		return -EINVAL;
	}

	nss_qdisc_info("SSDK port shaper configuration successful for port:%d\n", port_num);
	return 0;
}

/*
 * nss_ppe_port_shaper_set()
 *	Configures a port shaper in SSDK.
 */
static int nss_ppe_port_shaper_set(struct nss_qdisc *nq)
{
	fal_shaper_token_number_t token;
	fal_shaper_config_t cfg;
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	/*
	 * Set port shaper token number
	 */
	memset(&token, 0, sizeof(token));
	token.c_token_number = NSS_PPE_TOKEN_MAX;
	token.e_token_number = NSS_PPE_TOKEN_MAX;

	nss_qdisc_trace("SSDK port token set : Port:%d, c_token_number:%x, e_token_number:%x\n",
		port_num, token.c_token_number, token.e_token_number);
	if (fal_port_shaper_token_number_set(0, port_num, &token) != 0) {
		nss_qdisc_error("SSDK port shaper token configuration failed for port:%d\n", port_num);
		return -EINVAL;
	}

	/*
	 * Set port shaper configuration
	 * Note: SSDK API requires burst in bytes/sec
	 * while rate in kbits/sec.
	 */
	memset(&cfg, 0, sizeof(cfg));
	cfg.couple_en = 0;
	cfg.meter_unit = 0;
	cfg.c_shaper_en = 1;
	cfg.cbs = npq->shaper.cburst;
	cfg.cir = (npq->shaper.crate / 1000) * 8;
	cfg.shaper_frame_mode = FAL_FRAME_CRC;

	/*
	 * Take HW scaling into consideration
	 */
	cfg.cir = cfg.cir * NSS_PPE_HW_FREQ_SCALING;

	nss_qdisc_trace("SSDK port shaper configuration: Port:%u, couple_en:%u, meter_unit:%u, c_shaper_en:%u, cbs:%u, cir:%u, ebs:%u, eir:%u, shaper_frame_mode:%u\n",
			port_num, cfg.couple_en, cfg.meter_unit, cfg.c_shaper_en, cfg.cbs, cfg.cir, cfg.ebs, cfg.eir, cfg.shaper_frame_mode);
	if (fal_port_shaper_set(0, port_num, &cfg) != 0) {
		nss_qdisc_error("SSDK port shaper configuration failed for port:%d\n", port_num);
		return -EINVAL;
	}

	nss_qdisc_info("SSDK port shaper configuration successful for port:%d\n", port_num);
	return 0;
}

/*
 * nss_ppe_flow_shaper_reset()
 *	Resets a flow shaper in SSDK.
 */
static int nss_ppe_flow_shaper_reset(struct nss_qdisc *nq)
{
	fal_shaper_config_t cfg;
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	/*
	 * Reset flow shaper configuration
	 */
	memset(&cfg, 0, sizeof(cfg));
	nss_qdisc_trace("SSDK flow shaper reset : l0spid:%d\n", npq->l0spid);
	if (fal_flow_shaper_set(0, npq->l0spid, &cfg) != 0) {
		nss_qdisc_error("SSDK flow shaper configuration failed for port:%d, l0spid:%d\n",
			port_num, npq->l0spid);
		return -EINVAL;
	}

	nss_qdisc_info("SSDK flow shaper configuration successful for port:%d\n", port_num);
	return 0;
}

/*
 * nss_ppe_flow_shaper_set()
 *	Configures a flow shaper in SSDK.
 */
static int nss_ppe_flow_shaper_set(struct nss_qdisc *nq)
{
	fal_shaper_token_number_t token;
	fal_shaper_config_t cfg;
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	/*
	 * Set flow shaper token number
	 */
	memset(&token, 0, sizeof(token));
	token.c_token_number = NSS_PPE_TOKEN_MAX;
	token.e_token_number = NSS_PPE_TOKEN_MAX;

	nss_qdisc_trace("SSDK flow token set : l0spid:%d, c_token_number:%x, e_token_number:%x\n",
		npq->l0spid, token.c_token_number, token.e_token_number);
	if (fal_flow_shaper_token_number_set(0, npq->l0spid, &token) != 0) {
		nss_qdisc_error("SSDK flow shaper token configuration failed for port:%d, l0spid:%d\n",
			port_num, npq->l0spid);
		return -EINVAL;
	}

	/*
	 * Set flow shaper token configuration
	 * Note: SSDK API requires burst in bytes/sec
	 * while rate in kbits/sec.
	 */
	cfg.couple_en = 0;
	cfg.meter_unit = 0;
	cfg.c_shaper_en = 1;
	cfg.cbs = npq->shaper.burst;
	cfg.cir = (npq->shaper.rate / 1000) * 8;
	cfg.e_shaper_en = 1;
	cfg.ebs = npq->shaper.cburst;
	cfg.eir = ((npq->shaper.crate / 1000) * 8) - cfg.cir;
	cfg.shaper_frame_mode = FAL_FRAME_CRC;

	/*
	 * Take HW scaling into consideration
	 */
	cfg.cir = cfg.cir * NSS_PPE_HW_FREQ_SCALING;
	cfg.eir = cfg.eir * NSS_PPE_HW_FREQ_SCALING;

	nss_qdisc_trace("SSDK flow shaper configuration: l0spid:%u, couple_en:%u, meter_unit:%u, c_shaper_en:%u, cbs:%u, cir:%u, ebs:%u, eir:%u, shaper_frame_mode:%u\n",
		npq->l0spid, cfg.couple_en, cfg.meter_unit, cfg.c_shaper_en, cfg.cbs, cfg.cir, cfg.ebs, cfg.eir, cfg.shaper_frame_mode);
	if (fal_flow_shaper_set(0, npq->l0spid, &cfg) != 0) {
		nss_qdisc_error("SSDK flow shaper configuration failed for port:%d, l0spid:%d\n",
			port_num, npq->l0spid);
		return -EINVAL;
	}

	nss_qdisc_info("SSDK flow shaper configuration successful for port:%d\n", port_num);
	return 0;
}

/*
 * nss_ppe_queue_shaper_reset()
 *	Resets a queue shaper in SSDK.
 */
static int nss_ppe_queue_shaper_reset(struct nss_qdisc *nq)
{
	fal_shaper_config_t cfg;
	uint32_t port_num = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	/*
	 * Reset queue shaper configuration
	 */
	memset(&cfg, 0, sizeof(cfg));
	nss_qdisc_trace("SSDK queue shaper reset : ucast_qid:%d\n", npq->q.ucast_qid);
	if (fal_queue_shaper_set(0, npq->q.ucast_qid, &cfg) != 0) {
		nss_qdisc_error("SSDK queue shaper configuration failed for port:%d, ucast_qid:%d\n",
			port_num, npq->q.ucast_qid);
		return -EINVAL;
	}

	nss_qdisc_info("SSDK queue shaper configuration successful for port:%d\n",
		port_num);
	return 0;
}

/*
 * nss_ppe_queue_shaper_set()
 *	Configures a queue shaper in SSDK.
 */
static int nss_ppe_queue_shaper_set(struct nss_qdisc *nq)
{
	fal_shaper_token_number_t token;
	fal_shaper_config_t cfg;
	uint32_t port_num __maybe_unused = nss_ppe_port_num_get(nq);
	struct nss_ppe_qdisc *npq = &nq->npq;

	/*
	 * Set queue shaper token number
	 */
	memset(&token, 0, sizeof(token));
	token.c_token_number = NSS_PPE_TOKEN_MAX;
	token.e_token_number = NSS_PPE_TOKEN_MAX;

	nss_qdisc_trace("SSDK queue token set : ucast_qid:%d, c_token_number:%x, e_token_number:%x\n",
		npq->q.ucast_qid, token.c_token_number, token.e_token_number);
	if (fal_queue_shaper_token_number_set(0, npq->q.ucast_qid, &token) != 0) {
		nss_qdisc_error("SSDK queue shaper token configuration failed\n");
		return -EINVAL;
	}

	/*
	 * Set queue shaper confguration
	 * Note: SSDK API requires burst in bytes/sec
	 * while rate in kbits/sec.
	 */
	cfg.couple_en = 0;
	cfg.meter_unit = 0;
	cfg.c_shaper_en = 1;
	cfg.cbs = npq->shaper.burst;
	cfg.cir = (npq->shaper.rate / 1000) * 8;
	cfg.e_shaper_en = 1;
	cfg.ebs = npq->shaper.cburst;
	cfg.eir = ((npq->shaper.crate / 1000) * 8) - cfg.cir;
	cfg.shaper_frame_mode = FAL_FRAME_CRC;

	/*
	 * Take HW scaling into consideration
	 */
	cfg.cir = cfg.cir * NSS_PPE_HW_FREQ_SCALING;
	cfg.eir = cfg.eir * NSS_PPE_HW_FREQ_SCALING;

	nss_qdisc_trace("SSDK queue shaper configuration: ucast_qid:%u, couple_en:%u, meter_unit:%u, c_shaper_en:%u, cbs:%u, cir:%u, ebs:%u, eir:%u, shaper_frame_mode:%u\n",
		npq->q.ucast_qid, cfg.couple_en, cfg.meter_unit, cfg.c_shaper_en, cfg.cbs, cfg.cir, cfg.ebs, cfg.eir, cfg.shaper_frame_mode);
	if (fal_queue_shaper_set(0, npq->q.ucast_qid, &cfg) != 0) {
		nss_qdisc_error("SSDK queue shaper configuration failed\n");
		return -EINVAL;
	}

	/*
	 * This function is called at the time of class allocation and change at
	 * NSS_PPE_QUEUE_LEVEL. But, mcast will be valid only in case queue
	 * is attached at NSS_PPE_SUB_QUEUE_LEVEL and change class is invoked
	 * at NSS_PPE_QUEUE_LEVEL level.
	 * Other scenarios catering to mcast queue are handled in separate mcast APIs.
	 */
	if (npq->q.mcast_qid) {
		nss_qdisc_trace("SSDK multicast queue shaper configuration: mcast_qid:%u, couple_en:%u, meter_unit:%u, c_shaper_en:%u, cbs:%u, cir:%u, ebs:%u, eir:%u, shaper_frame_mode:%u\n",
			npq->q.mcast_qid, cfg.couple_en, cfg.meter_unit, cfg.c_shaper_en, cfg.cbs, cfg.cir, cfg.ebs, cfg.eir, cfg.shaper_frame_mode);
		if (fal_queue_shaper_set(0, npq->q.mcast_qid, &cfg) != 0) {
			nss_qdisc_error("SSDK multicast queue shaper configuration failed\n");
			return -EINVAL;
		}
	}

	nss_qdisc_info("SSDK queue shaper configuration successful for port:%d\n", port_num);
	return 0;
}

/*
 * nss_ppe_max_level_get()
 *	Returns the max configuration level for the given prot.
 */
static int nss_ppe_max_level_get(struct nss_qdisc *nq)
{
	int level = NSS_PPE_MAX_LEVEL;

	/*
	 * Loopback uses one level.
	 */
	if (nq->needs_ppe_loopback) {
		level = level - 1;
	}

	nss_qdisc_info("level:%d\n", level);
	return level;
}

/*
 * nss_ppe_attach_free()
 *	Attaches a resource to free list.
 */
static void nss_ppe_attach_free(uint32_t port, struct nss_ppe_res *res)
{
	struct nss_ppe_port *ppe_port = &ppe_qdisc_port[port];

	spin_lock_bh(&ppe_port->lock);
	res->next = ppe_port->res_free[res->type];
	ppe_port->res_free[res->type] = res;
	spin_unlock_bh(&ppe_port->lock);

	nss_qdisc_info("port:%d, type:%d, res:%px\n", port, res->type, res);
	return;
}

/*
 * nss_ppe_base_get()
 *	Returns base of the particular resource for a given port.
 */
uint32_t nss_ppe_base_get(uint32_t port, nss_ppe_res_type_t type)
{
	uint32_t base = 0;
	struct nss_ppe_port *ppe_port = &ppe_qdisc_port[port];

	spin_lock_bh(&ppe_port->lock);
	base = ppe_port->base[type];
	spin_unlock_bh(&ppe_port->lock);

	nss_qdisc_info("port:%d, type:%d, base:%d\n", port, type, base);
	return base;
}

/*
 * nss_ppe_res_free()
 *	Frees the allocated resource and attach it to free list.
 */
int nss_ppe_res_free(uint32_t port, uint32_t offset, nss_ppe_res_type_t type)
{
	struct nss_ppe_res *temp = NULL;
	struct nss_ppe_res *res = NULL;
	struct nss_ppe_port *ppe_port = &ppe_qdisc_port[port];

	if (type >= NSS_PPE_MAX_RES_TYPE) {
		nss_qdisc_assert(false, "Resource type:%d not valid for port:%d", type, port);
		return -1;
	}

	spin_lock_bh(&ppe_port->lock);
	res = ppe_port->res_used[type];
	if (res->offset == offset) {
		ppe_port->res_used[type] = res->next;
		res->next = NULL;
		spin_unlock_bh(&ppe_port->lock);
		goto success;
	}

	temp = res;
	res = res->next;

	while (res) {
		if (res->offset == offset) {
			temp->next = res->next;
			res->next = NULL;
			break;
		} else {
			temp = res;
			res = res->next;
		}
	}
	spin_unlock_bh(&ppe_port->lock);

	if (!res) {
		nss_qdisc_assert(false, "Resource:%d type:%d not found for port:%d", offset, type, port);
		return -1;
	}

success:
	nss_ppe_attach_free(port, res);
	nss_qdisc_info("port:%d, type:%d, res:%px\n", port, type, res);
	return 0;
}

/*
 * nss_ppe_res_alloc()
 *	Allocates free resource for a given port.
 */
struct nss_ppe_res *nss_ppe_res_alloc(uint32_t port, nss_ppe_res_type_t type)
{
	struct nss_ppe_res *res = NULL;
	struct nss_ppe_port *ppe_port = &ppe_qdisc_port[port];

	/*
	 * Detach the resource from free list
	 * and attach to used list.
	 */
	spin_lock_bh(&ppe_port->lock);
	res = ppe_port->res_free[type];
	if (res) {
		ppe_port->res_free[type] = res->next;
		res->next = ppe_port->res_used[type];
		ppe_port->res_used[type] = res;
	}
	spin_unlock_bh(&ppe_port->lock);

	nss_qdisc_info("port:%d, type:%d, res:%px\n", port, type, res);
	return res;
}

/*
 * nss_ppe_default_conf_set()
 *	Sets default queue scheduler in SSDK.
 */
static int nss_ppe_default_conf_set(uint32_t port_num)
{
	fal_qos_scheduler_cfg_t l0cfg;
	fal_ac_obj_t obj;
	fal_ac_ctrl_t cfg;
	uint32_t l0spid;
	uint32_t ucast_qid;
	uint32_t mcast_qid;

	/*
	 * Disable all queues and set SSDK configuration
	 * We need to disable and flush the queues before
	 * reseting to the default configuration.
	 */
	nss_ppe_all_queue_disable(port_num);

	/*
	 * No resources were allocated for Port 0 (Loopback port).
	 * L1 scheduler was configured at init time.
	 */
	if (port_num == 0) {
		l0spid = NSS_PPE_LOOPBACK_L0_SP_BASE;
		goto conf;
	}

	/*
	 * Invoke SSDK API to reset the default configuration for a given port.
	 */
	if (fal_port_scheduler_cfg_reset(0, port_num) != 0) {
		nss_qdisc_error("SSDK reset default queue configuration failed\n");
		nss_ppe_assigned_queue_enable(port_num);
		return -EINVAL;
	}

	nss_ppe_all_queue_enable(port_num);

	nss_qdisc_info("SSDK queue configuration successful\n");
	return 0;

conf:
	/*
	 * Reset Level 0 Configuration
	 * We are not using SSDK reset API for port 0 as
	 * only 16 queues from this port are used for QoS.
	 */
	memset(&l0cfg, 0, sizeof(l0cfg));
	l0cfg.sp_id = l0spid;
	l0cfg.c_pri = NSS_PPE_PRIORITY_MAX;
	l0cfg.e_pri = NSS_PPE_PRIORITY_MAX;
	l0cfg.c_drr_wt = 1;
	l0cfg.e_drr_wt = 1;
	l0cfg.c_drr_id = nss_ppe_base_get(port_num, NSS_PPE_L0_CDRR);
	l0cfg.e_drr_id = nss_ppe_base_get(port_num, NSS_PPE_L0_EDRR);
	ucast_qid = nss_ppe_base_get(port_num, NSS_PPE_UCAST_QUEUE);
	mcast_qid = nss_ppe_base_get(port_num, NSS_PPE_MCAST_QUEUE);

	nss_qdisc_trace("SSDK level0 configuration: Port:%d, ucast_qid:%d, c_drrid:%d, c_pri:%d, c_drr_wt:%d, e_drrid:%d, e_pri:%d, e_drr_wt:%d, l0spid:%d\n",
			port_num, ucast_qid, l0cfg.c_drr_id, l0cfg.c_pri, l0cfg.c_drr_wt, l0cfg.e_drr_id, l0cfg.e_pri, l0cfg.e_drr_wt, l0cfg.sp_id);
	if (fal_queue_scheduler_set(0, ucast_qid, NSS_PPE_QUEUE_LEVEL - 1, port_num, &l0cfg) != 0) {
		nss_qdisc_error("SSDK level0 queue scheduler configuration failed\n");
		nss_ppe_all_queue_enable(port_num);
		return -EINVAL;
	}

	nss_qdisc_trace("SSDK level0 configuration: Port:%d, mcast_qid:%d, c_drrid:%d, c_pri:%d, c_drr_wt:%d, e_drrid:%d, e_pri:%d, e_drr_wt:%d, l0spid:%d\n",
			port_num, mcast_qid, l0cfg.c_drr_id, l0cfg.c_pri, l0cfg.c_drr_wt, l0cfg.e_drr_id, l0cfg.e_pri, l0cfg.e_drr_wt, l0cfg.sp_id);
	if (fal_queue_scheduler_set(0, mcast_qid, NSS_PPE_QUEUE_LEVEL - 1, port_num, &l0cfg) != 0) {
		nss_qdisc_error("SSDK level0 queue scheduler configuration failed\n");
		nss_ppe_all_queue_enable(port_num);
		return -EINVAL;
	}

	memset(&obj, 0, sizeof(obj));
	obj.obj_id = ucast_qid;
	fal_ac_prealloc_buffer_set(0, &obj, 0);
	obj.obj_id = mcast_qid;
	fal_ac_prealloc_buffer_set(0, &obj, 0);

	/*
	 * Disable force drop.
	 * Setting ac_fc_en as 0 means queue will
	 * honor flow control.
	 */
	memset(&cfg, 0, sizeof(cfg));
	obj.obj_id = ucast_qid;
	cfg.ac_en = 1;
	nss_qdisc_trace("SSDK queue flow control set: ucast_qid:%d, enable:%d\n", ucast_qid, cfg.ac_fc_en);
	if (fal_ac_ctrl_set(0, &obj, &cfg) != 0) {
		nss_qdisc_error("SSDK queue flow control set failed\n");
		nss_ppe_all_queue_enable(port_num);
		return -EINVAL;
	}

	nss_ppe_all_queue_enable(port_num);

	nss_qdisc_info("SSDK queue configuration successful\n");
	return 0;
}

/*
 * nss_ppe_queue_limit_set()
 *	Sets queue size in SSDK.
 */
static int nss_ppe_queue_limit_set(struct nss_qdisc *nq)
{
	fal_ac_obj_t obj;
	fal_ac_static_threshold_t cfg;
	fal_ac_ctrl_t ctrl_cfg;
	struct nss_ppe_qdisc *npq = &nq->npq;

	if (!npq->q.ucast_valid) {
		return 0;
	}

	/*
	 * Unicast queue configuration.
	 */
	memset(&obj, 0, sizeof(obj));
	obj.obj_id = npq->q.ucast_qid;

	nss_qdisc_trace("SSDK queue buffer set: ucast_qid:%d, qlimit:%d\n", npq->q.ucast_qid, npq->q.qlimit);
	if (fal_ac_prealloc_buffer_set(0, &obj, npq->q.qlimit) != 0) {
		nss_qdisc_error("SSDK queue configuration failed\n");
		return -EINVAL;
	}

	/*
	 * Enable force drop for PPE qdisc.
	 * When set to 1, the flow control will be overriden
	 * for that queue and packets drop gets enabled.
	 */
	memset(&ctrl_cfg, 0, sizeof(ctrl_cfg));
	ctrl_cfg.ac_en = 1;
	if (nq->mode == NSS_QDISC_MODE_PPE) {
		ctrl_cfg.ac_fc_en = 1;
	}

	nss_qdisc_trace("SSDK queue flow control set: ucast_qid:%d, enable:%d\n", npq->q.ucast_qid, ctrl_cfg.ac_fc_en);
	if (fal_ac_ctrl_set(0, &obj, &ctrl_cfg) != 0) {
		nss_qdisc_error("SSDK queue flow control set failed\n");
		return -EINVAL;
	}

	memset(&cfg, 0, sizeof(cfg));
	cfg.wred_enable = npq->q.red_en;
	cfg.green_max = npq->q.max_th[NSS_PPE_COLOR_GREEN];
	cfg.green_min_off = npq->q.min_th[NSS_PPE_COLOR_GREEN];
	nss_qdisc_trace("SSDK queue ac threshold set: ucast_qid:%d, wred_enable:%d, green_max:%d, green_min_off:%d\n",
		npq->q.ucast_qid, cfg.wred_enable, cfg.green_max, cfg.green_min_off);
	if (fal_ac_static_threshold_set(0, &obj, &cfg) != 0) {
		nss_qdisc_error("SSDK queue configuration failed\n");
		return -EINVAL;
	}

	/*
	 * Multicast queue configuration.
	 */
	if (npq->q.mcast_valid) {
		obj.obj_id = npq->q.mcast_qid;

		nss_qdisc_trace("SSDK multicast queue buffer set: mcast_qid:%d, qlimit:%d\n", npq->q.mcast_qid, npq->q.qlimit);
		if (fal_ac_prealloc_buffer_set(0, &obj, npq->q.qlimit) != 0) {
			nss_qdisc_error("SSDK multicast queue configuration failed\n");
			return -EINVAL;
		}

		nss_qdisc_trace("SSDK multicast queue ac threshold set: mcast_qid:%d, wred_enable:%d, green_max:%d, green_min_off:%d\n",
			npq->q.mcast_qid, cfg.wred_enable, cfg.green_max, cfg.green_min_off);
		if (fal_ac_static_threshold_set(0, &obj, &cfg) != 0) {
			nss_qdisc_error("SSDK multicast queue configuration failed\n");
			return -EINVAL;
		}
	}

	nss_qdisc_info("SSDK queue configuration successful\n");
	return 0;
}

/*
 * nss_ppe_scheduler_reset()
 *	Resets a configured scheduler in SSDK.
 */
static int nss_ppe_scheduler_reset(struct nss_qdisc *nq)
{
	struct nss_ppe_qdisc *npq = &nq->npq;

	if ((!npq->l0_valid) && (!npq->l1_valid)) {
		return 0;
	}

	if (npq->level == NSS_PPE_PORT_LEVEL) {
		if (!nss_ppe_sub_type_is_queue(npq)) {
			nss_qdisc_info("SSDK reset scheduler successful\n");
			return 0;
		}

		if (nss_ppe_l0_queue_scheduler_reset(nq) != 0) {
			nss_qdisc_warning("SSDK Level0 queue scheduler reset failed\n");
			return -EINVAL;
		}

		if (nss_ppe_l1_res_free(nq) != 0) {
			nss_qdisc_warning("SSDK Level1 queue scheduler reset failed\n");
			return -EINVAL;
		}

		nss_qdisc_info("SSDK reset scheduler successful\n");
		return 0;
	}

	if (npq->level == NSS_PPE_FLOW_LEVEL) {

		/*
		 * We have allocated L0 resources if this qdisc is of type queue or
		 * this is an hybrid (NSS) qdisc that is attached to PPE qdisc.
		 */
		if ((nss_ppe_sub_type_is_queue(npq)) || (nq->mode == NSS_QDISC_MODE_NSS)) {
			if (nss_ppe_l0_queue_scheduler_reset(nq) < 0) {
				nss_qdisc_warning("SSDK Level0 configuration for attach of new qdisc failed\n");
				return -EINVAL;
			}
		}

		if (nss_ppe_l1_res_free(nq) != 0) {
			nss_qdisc_warning("SSDK Level1 queue scheduler reset failed\n");
			return -EINVAL;
		}
	} else if (npq->level == NSS_PPE_QUEUE_LEVEL) {
		if (nss_ppe_l0_queue_scheduler_reset(nq) != 0) {
			nss_qdisc_warning("SSDK Level0 queue scheduler reset failed\n");
			return -EINVAL;
		}
	} else {

		/*
		 * When a classful qdisc say HTB is configured with max levels of hierarchy,
		 * and then if a qdisc say FIFO is attached at the last level, we will have all
		 * the resources allocated and we just need to enable/disable the queue.
		 */
		nss_ppe_queue_disable(nq);
	}

	nss_qdisc_info("SSDK reset scheduler successful\n");
	return 0;
}

/*
 * nss_ppe_scheduler_set()
 *	Configures a scheduler in SSDK.
 */
static int nss_ppe_scheduler_set(struct nss_qdisc *nq)
{
	struct nss_ppe_qdisc *npq = &nq->npq;

	if (npq->level == NSS_PPE_PORT_LEVEL) {

		/*
		 * Only L1 SP can be allocated at port level if qdisc is not of type queue.
		 * Since L1 SP is equal to port number, we don't explicitly allocate it.
		 */
		if (!nss_ppe_sub_type_is_queue(npq)) {
			nss_qdisc_info("SSDK scheduler configuration successful\n");
			return 0;
		}

		/*
		 * Set queue schedulers for queue qdiscs attached at port level.
		 */
		if (nss_ppe_l1_queue_scheduler_set(nq) < 0) {
			nss_qdisc_warning("SSDK Level1 configuration failed\n");
			return -EINVAL;
		}

		if (nss_ppe_l0_queue_scheduler_set(nq) < 0) {
			nss_ppe_l1_res_free(nq);
			nss_qdisc_warning("SSDK Level0 configuration failed\n");
			return -EINVAL;
		}

		nss_qdisc_info("SSDK scheduler configuration successful\n");
		return 0;
	}

	if (npq->level == NSS_PPE_FLOW_LEVEL) {
		if (nss_ppe_l1_queue_scheduler_set(nq) < 0) {
			nss_qdisc_warning("SSDK Level1 configuration failed\n");
			return -EINVAL;
		}

		/*
		 * We need to allocate L0 resources as well if qdisc is of type queue or
		 * we are attaching a hybrid (NSS) qdisc to PPE qdisc.
		 */
		if ((nss_ppe_sub_type_is_queue(npq)) || (nq->mode == NSS_QDISC_MODE_NSS)) {
			if (nss_ppe_l0_queue_scheduler_set(nq) < 0) {
				nss_qdisc_warning("SSDK Level0 configuration failed\n");
				return -EINVAL;
			}
		}
	} else if (npq->level == NSS_PPE_QUEUE_LEVEL) {

		/*
		 * This case is invoked when a Qdisc/class is attached/changed
		 * at queue level. If a class is attached at queue level, we
		 * disable the queue until a qdisc is attached. but if it is
		 * invoked at change configuration time, we just set the schedulers.
		 */
		if (nss_ppe_l0_queue_scheduler_set(nq) < 0) {
			nss_qdisc_warning("SSDK Level0 configuration failed\n");
			return -EINVAL;
		}

		if ((!npq->is_configured) && (!nss_ppe_sub_type_is_queue(npq))) {
			nss_ppe_queue_disable(nq);
		}
	} else {
		/*
		 * When a classful qdisc say HTB is configured with max levels of hierarchy,
		 * and then if a qdisc say FIFO is attached at the last level, we will have all
		 * the resources allocated and we just need to enable/disable the queue.
		 */
		nss_ppe_queue_enable(nq);
	}

	nss_qdisc_info("SSDK scheduler configuration successful\n");
	return 0;
}

/*
 * nss_ppe_shaper_reset()
 *	Resets a configured shaper in SSDK.
 */
static int nss_ppe_shaper_reset(struct nss_qdisc *nq)
{
	struct nss_ppe_qdisc *npq = &nq->npq;

	if (!npq->shaper_present) {
		return 0;
	}

	if (npq->level == NSS_PPE_PORT_LEVEL) {
		if (nss_ppe_port_shaper_reset(nq) != 0) {
				nss_qdisc_warning("Reset Port shaper failed\n");
			return -EINVAL;
		}
		nss_qdisc_info("SSDK reset shaper successful\n");
		return 0;
	}

	if (npq->level == NSS_PPE_FLOW_LEVEL) {
		if (nss_ppe_flow_shaper_reset(nq) != 0) {
			nss_qdisc_warning("Reset Flow shaper failed\n");
			return -EINVAL;
		}
		nss_qdisc_info("SSDK reset shaper successful\n");
		return 0;
	}

	if (npq->level == NSS_PPE_QUEUE_LEVEL) {
		if (nss_ppe_queue_shaper_reset(nq) != 0) {
			nss_qdisc_warning("Reset Queue shaper failed\n");
			return -EINVAL;
		}
	}

	npq->shaper_present = false;

	nss_qdisc_info("SSDK reset shaper successful\n");
	return 0;
}

/*
 * nss_ppe_shaper_set()
 *	Configures a shaper in SSDK.
 */
static int nss_ppe_shaper_set(struct nss_qdisc *nq)
{
	struct nss_ppe_qdisc *npq = &nq->npq;

	if (!npq->shaper_present) {
		return 0;
	}

	if (npq->level == NSS_PPE_PORT_LEVEL) {
		if (nss_ppe_port_shaper_set(nq) != 0) {
			nss_qdisc_warning("Port shaper configuration failed\n");
			return -EINVAL;
		}
		nss_qdisc_info("SSDK set shaper successful\n");
		return 0;
	}

	if (npq->level == NSS_PPE_FLOW_LEVEL) {
		if (nss_ppe_flow_shaper_set(nq) != 0) {
			nss_qdisc_warning("Port shaper configuration failed\n");
			return -EINVAL;
		}
		nss_qdisc_info("SSDK set shaper successful\n");
		return 0;
	}

	if (npq->level == NSS_PPE_QUEUE_LEVEL) {
		if (nss_ppe_queue_shaper_set(nq) != 0) {
			nss_qdisc_warning("Queue shaper configuration failed\n");
			return -EINVAL;
		}
	}

	nss_qdisc_info("SSDK set shaper successful\n");
	return 0;
}

/*
 * nss_ppe_set_parent()
 *	Sets the parent of given qdisc.
 */
int nss_ppe_set_parent(struct Qdisc *sch, struct nss_qdisc *nq, uint32_t parent)
{
	struct net_device *dev = qdisc_dev(sch);
	struct nss_qdisc *parent_nq = NULL;
	struct Qdisc *parent_qdisc = NULL;
	unsigned long parent_class = 0;

	/*
	 * PPE Qdisc cannot be attached to NSS Qdisc.
	 */
	if (parent != TC_H_ROOT) {
		parent_qdisc = qdisc_lookup(dev, TC_H_MAJ(parent));
		parent_nq = qdisc_priv(parent_qdisc);
	} else if (nq->is_class) {
		parent_nq = qdisc_priv(nq->qdisc);
	}

	if ((parent_nq) && (parent_nq->mode == NSS_QDISC_MODE_NSS)) {
		nss_qdisc_info("HW qdisc/class %px cannot be attached to nss qdisc/class\n", nq->qdisc);
		return NSS_PPE_QDISC_PARENT_NOT_PPE;
	}

	/*
	 * Set the parent if current Qdisc is not a class.
	 * For class, parent is set before invoking nss_qdisc_init in respective qdisc files.
	 */
	if ((parent_nq) && (!nq->is_class)) {
		nq->parent = parent_nq;

		/*
		 * If parent is a class.
		 *
		 * Though PRIO is classless but PRIO bands are treated as classes by Qdisc
		 * infrastructure i.e TC_H_MIN(parent) is true for PRIO band.
		 * But, we donot allocate any resources for PRIO bands separately so in
		 * case of any qdisc attached to PRIO band, the parent is set as PRIO qdisc itself.
		 * And the below class check is applicable only for the classful qdiscs.
		 */
		if ((parent_nq) && (parent_nq->npq.sub_type != NSS_SHAPER_CONFIG_PPE_SN_TYPE_PRIO) && (TC_H_MIN(parent))) {
			if (!parent_qdisc) {
				nss_qdisc_info("HW qdisc/class %px cannot be attached to non-existing class %x\n", nq->qdisc, parent);
				return NSS_PPE_QDISC_PARENT_NOT_EXISTING;
			}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
			parent_class = parent_qdisc->ops->cl_ops->get(parent_qdisc, parent);
#else
			parent_class = parent_qdisc->ops->cl_ops->find(parent_qdisc, parent);
#endif
			if (!parent_class) {
				nq->parent = NULL;
				nss_qdisc_info("HW qdisc/class %px cannot be attached to non-existing class %x\n", nq->qdisc, parent);
				return NSS_PPE_QDISC_PARENT_NOT_EXISTING;

			}

			nq->parent = (struct nss_qdisc *)parent_class;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
			parent_qdisc->ops->cl_ops->put(parent_qdisc, parent_class);
#endif
		}
	}

	return 0;
}

/*
 * nss_ppe_get_max_prio_bands()
 *	Returns the number of PRIO bands supported based on qdisc level.
 */
int nss_ppe_get_max_prio_bands(struct nss_qdisc *nq)
{
	nss_ppe_level_t level = nq->npq.level;

	switch (level) {
	case NSS_PPE_PORT_LEVEL:
		return NSS_PPE_PORT_LEVEL_PRIO_BANDS_MAX;

	case NSS_PPE_FLOW_LEVEL:
		return NSS_PPE_FLOW_LEVEL_PRIO_BANDS_MAX;

	default:
		nss_qdisc_warning("HW Qdisc not supported at this level\n");
		return 0;
	}
}

/*
 * nss_ppe_is_depth_valid()
 *	Checks the depth of Qdisc tree.
 */
static int nss_ppe_is_depth_valid(struct nss_qdisc *nq)
{
	enum nss_shaper_config_ppe_sn_type sub_type = nq->npq.sub_type;
	nss_ppe_level_t valid_level;
	nss_qdisc_trace("level:%d\n", nq->npq.level);

	switch (sub_type) {
	case NSS_SHAPER_CONFIG_PPE_SN_TYPE_FIFO:
	case NSS_SHAPER_CONFIG_PPE_SN_TYPE_RED:
		valid_level = NSS_PPE_SUB_QUEUE_LEVEL;
		break;

	case NSS_SHAPER_CONFIG_PPE_SN_TYPE_HTB:
	case NSS_SHAPER_CONFIG_PPE_SN_TYPE_WRR:
	case NSS_SHAPER_CONFIG_PPE_SN_TYPE_PRIO:
		valid_level = NSS_PPE_FLOW_LEVEL;
		break;

	case NSS_SHAPER_CONFIG_PPE_SN_TYPE_HTB_GROUP:
	case NSS_SHAPER_CONFIG_PPE_SN_TYPE_TBL:
	case NSS_SHAPER_CONFIG_PPE_SN_TYPE_WRR_GROUP:
		valid_level = NSS_PPE_QUEUE_LEVEL;
		break;

	default:
		nss_qdisc_warning("HW Qdisc not supported\n");
		return false;
	}

	if (nq->npq.level < valid_level) {
		nss_qdisc_warning("sub_type:%d not supported at level:%d\n", sub_type, nq->npq.level);
		return false;
	}

	return true;
}

/*
 * nss_ppe_drr_weight_get()
 *	Returns the DRR weight corresponding to quantum.
 */
int nss_ppe_drr_weight_get(uint32_t quantum, nss_ppe_drr_unit_t drr_unit)
{
	switch (drr_unit) {
	case NSS_PPE_DRR_UNIT_BYTE:
		if (quantum < NSS_PPE_DRR_WEIGHT_MAX) {
			return 1;
		} else {
			return quantum / NSS_PPE_DRR_WEIGHT_MAX;
		}

	case NSS_PPE_DRR_UNIT_PACKET:
		return quantum;

	default:
		nss_qdisc_warning("DRR unit %d not supported\n", drr_unit);
	}

	return 0;
}

/*
 * nss_ppe_port_num_get()
 *	Returns the port number.
 */
int nss_ppe_port_num_get(struct nss_qdisc *nq)
{
	int port_num = 0;

	/*
	 * Fetch port number based on interface type.
	 * TODO: Change this when API from DP is available
	 */
	if (!nq->needs_ppe_loopback) {
		port_num = nq->nss_interface_number;
	}

	nss_qdisc_info("port:%d\n", port_num);
	return port_num;
}

/*
 * nss_ppe_all_queue_enable_hybrid()
 *	Enables PPE queues when NSS queuing Qdiscs are attached in the hieracrchy.
 */
void nss_ppe_all_queue_enable_hybrid(struct nss_qdisc *nq)
{
	struct nss_qdisc *nq_root = qdisc_priv(qdisc_root(nq->qdisc));

	if (!nq_root->hybrid_configured) {
		return;
	}

	/*
	 * In case of hybrid mode, we disable the PPE queues until
	 * queueing Qdisc is attached in the hierarchy.
	 */
	if((nq->type == NSS_SHAPER_NODE_TYPE_CODEL)
		|| (nq->type == NSS_SHAPER_NODE_TYPE_FIFO)
		|| (nq->type == NSS_SHAPER_NODE_TYPE_BF)
		|| (nq->type == NSS_SHAPER_NODE_TYPE_WRED)) {
		uint32_t port_num = nss_ppe_port_num_get(nq);
		nss_ppe_assigned_queue_enable(port_num);
		nss_qdisc_info("Queues in hybrid mode enabled successfully for Qdisc %px (type %d)\n", nq, nq->type);
	}
}

/*
 * nss_ppe_node_detach()
 *	Configuration function that helps detach a child shaper node from a parent.
 */
int nss_ppe_node_detach(struct nss_qdisc *nq, struct nss_qdisc *nq_child)
{
	struct nss_qdisc *nq_root = qdisc_priv(qdisc_root(nq->qdisc));

	/*
	 * If child qdisc is of type NSS, reset the hybrid mode and
	 * hybrid_configured flag.
	 */
	if (nq_child->mode != NSS_QDISC_MODE_PPE) {
		if (nss_qdisc_set_hybrid_mode(nq_child, NSS_QDISC_HYBRID_MODE_DISABLE, 0) < 0) {
			nss_qdisc_warning("detach of old qdisc %px failed\n", nq_child->qdisc);
			return -EINVAL;
		}

		nq_root->hybrid_configured = false;
	}

	nss_ppe_destroy(nq_child);

	nss_qdisc_info("Qdisc:%px, node:%px\n", nq, nq_child);
	return 0;
}

/*
 * nss_ppe_node_attach()
 *	Configuration function that helps attach a child shaper node to a parent.
 */
int nss_ppe_node_attach(struct nss_qdisc *nq, struct nss_qdisc *nq_child)
{
	struct net_device *dev = qdisc_dev(nq_child->qdisc);
	struct nss_ppe_qdisc *npq = &nq->npq;
	struct nss_ppe_qdisc *npq_child = &nq_child->npq;
	int ucast_qbase = nss_ppe_base_get(nss_ppe_port_num_get(nq), NSS_PPE_UCAST_QUEUE);
	struct nss_qdisc *nq_root = qdisc_priv(qdisc_root(nq->qdisc));

	/*
	 * Configuration is not required if child node is a class.
	 */
	if (nq_child->is_class)  {
		return 0;
	}

	/*
	 * In case of chile attached to PRIO qdisc, scheduler configuration is
	 * required to set the priority of child qdisc in SSDK.
	 */
	if (nq->npq.sub_type == NSS_SHAPER_CONFIG_PPE_SN_TYPE_PRIO) {
		if (nss_ppe_scheduler_set(nq_child) != 0)  {
			nss_qdisc_warning("SSDK scheduler configuration failed\n");
			return -EINVAL;
		}
	}

	if (nq_child->mode == NSS_QDISC_MODE_PPE) {
		return 0;
	}

	/*
	 * Return error in case NSS Qdisc is attached to loopback.
	 */
	if (nq->needs_ppe_loopback) {
		nss_qdisc_warning("NSS Qdisc cannot be attached to PPE Qdisc on loopback interface.\n");
		return -EINVAL;
	}

	/*
	 * Return error if hybrid QoS is already configured on the port.
	 */
	if (nq_root->hybrid_configured) {
		nss_qdisc_warning("More than one NSS qdisc cannot be attached to PPE qdsic.\n");
		return -EINVAL;
	}

	/*
	 * If new qdisc is not of type PPE, then configure a simple fifo
	 * queue in HW. This is where packets coming from the Adv QoS
	 * portion of the tree will get enqueued to.
	 */
	nq_child->parent = nq;
	npq_child->level = npq->level - 1;

	if (npq_child->level == NSS_PPE_SUB_QUEUE_LEVEL) {
		npq_child->q.ucast_valid = npq->q.ucast_valid;
		npq_child->q.ucast_qid = npq->q.ucast_qid;
	}

	npq->q.qlimit = dev->tx_queue_len ? : 1;
	if (nss_ppe_scheduler_set(nq_child) < 0) {
		nss_qdisc_warning("SSDK scheduler configuration failed\n");
		return -EINVAL;
	}

	/*
	 * Program PPE queue parameters
	 */
	if (nss_ppe_queue_limit_set(nq_child) < 0) {
		nss_qdisc_warning("SSDK queue configuration failed\n");
		nss_ppe_mcast_queue_reset(nq_child);
		nss_ppe_scheduler_reset(nq_child);
		return -EINVAL;
	}

	if (nss_qdisc_set_hybrid_mode(nq_child, NSS_QDISC_HYBRID_MODE_ENABLE, npq_child->q.ucast_qid - ucast_qbase) < 0) {
		nss_qdisc_warning("nss qdisc configuration failed\n");
		nss_ppe_mcast_queue_reset(nq_child);
		nss_ppe_scheduler_reset(nq_child);
		return -EINVAL;
	}

	nq_root->hybrid_configured = true;

	nss_qdisc_info("Qdisc:%px, node:%px\n", nq, nq_child);
	return 0;
}

/*
 * nss_ppe_configure()
 *	Configures the SSDK schedulers and NSS shapers.
 */
int nss_ppe_configure(struct nss_qdisc *nq, struct nss_ppe_qdisc *prev_npq)
{
	struct nss_if_msg nim;
	int ucast_qbase = nss_ppe_base_get(nss_ppe_port_num_get(nq), NSS_PPE_UCAST_QUEUE);
	int mcast_qbase = nss_ppe_base_get(nss_ppe_port_num_get(nq), NSS_PPE_MCAST_QUEUE);
	struct nss_ppe_qdisc *npq = &nq->npq;

	/*
	 * Change the configuration in SSDK
	 */
	if (nss_ppe_scheduler_set(nq) != 0)  {
		nss_qdisc_warning("SSDK scheduler configuration failed\n");
		goto fail;
	}

	if (nss_ppe_shaper_set(nq) != 0)  {
		nss_qdisc_warning("SSDK shaper configuration failed\n");
		goto fail;
	}

	/*
	 * NSS configuration is not needed if Qdisc is not of queue type
	 * because firmware proxy nodes don't care about scheduler parameters.
	 */
	if (!nss_ppe_sub_type_is_queue(npq)) {
		nq->npq.is_configured = true;
		return 0;
	}

	if (nss_ppe_mcast_queue_set(nq) < 0) {
		nss_qdisc_warning("SSDK multicast queueue configuration failed\n");
		goto fail;
	}

	/*
	 * Program PPE queue parameters
	 */
	if (nss_ppe_queue_limit_set(nq) < 0) {
		nss_qdisc_warning("SSDK queue configuration failed\n");
		goto fail;
	}

	memset(&nim, 0, sizeof(struct nss_if_msg));
	nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = nq->qos_tag;
	nim.msg.shaper_configure.config.msg.shaper_node_config.snc.ppe_sn_param.type = npq->sub_type;
	nim.msg.shaper_configure.config.msg.shaper_node_config.snc.ppe_sn_param.limit = npq->q.qlimit;

	if (npq->q.ucast_valid) {
		nim.msg.shaper_configure.config.msg.shaper_node_config.snc.ppe_sn_param.ucast_base = ucast_qbase;
		nim.msg.shaper_configure.config.msg.shaper_node_config.snc.ppe_sn_param.ucast_offset = npq->q.ucast_qid - ucast_qbase;
	}

	if (npq->q.mcast_valid) {
		nim.msg.shaper_configure.config.msg.shaper_node_config.snc.ppe_sn_param.mcast_base = mcast_qbase;
		nim.msg.shaper_configure.config.msg.shaper_node_config.snc.ppe_sn_param.mcast_offset = npq->q.mcast_qid - mcast_qbase;
	}

	nim.msg.shaper_configure.config.msg.shaper_node_config.snc.ppe_sn_param.port = nss_ppe_port_num_get(nq);

	if (nss_qdisc_configure(nq, &nim, NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_CHANGE_PARAM) < 0) {
		nss_qdisc_warning("Qdisc configuration failed\n");
		goto fail;
	}

	/*
	 * There is nothing we need to do if the qdisc is not
	 * set as default qdisc.
	 * TODO: Currently multicast is associated with set_default configuration.
	 * Once multicast will have its own confuration parameter, move this code to
	 * respective qdisc.
	 */
	if ((npq->q.mcast_enable) && (nss_qdisc_set_default(nq) < 0)) {
		nss_qdisc_error("set_default failed\n");
		goto fail;
	}

	nq->npq.is_configured = true;

	nss_qdisc_info("Qdisc configured successfully\n");
	return 0;

fail:
	if (!nq->npq.is_configured) {
		return -EINVAL;
	}

	if (nq->is_class) {
		return -EINVAL;
	}

	/*
	 * Restore to previous configuration if exists.
	 * In case set_default is toggled and we have allocated/deallocated the
	 * mcast resources during change configuration, we need to restore them
	 * to old configuration by again deallocating/allocating them.
	 */
	if ((npq->q.mcast_enable != prev_npq->q.mcast_enable)
		&& (npq->q.mcast_valid != prev_npq->q.mcast_valid)) {
		prev_npq->q.mcast_valid = npq->q.mcast_valid;
		prev_npq->q.mcast_qid = npq->q.mcast_qid;
	}

	memcpy(npq, prev_npq, sizeof(struct nss_ppe_qdisc));
	nss_ppe_queue_limit_set(nq);
	nss_ppe_mcast_queue_set(nq);

	nss_ppe_shaper_set(nq);
	nss_ppe_scheduler_set(nq);

	return -EINVAL;
}

/*
 * nss_ppe_fallback_to_nss()
 *	Calls the initialization of NSS Qdisc when PPE initialization fails.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
int nss_ppe_fallback_to_nss(struct nss_qdisc *nq, struct nlattr *opt)
#else
int nss_ppe_fallback_to_nss(struct nss_qdisc *nq, struct nlattr *opt, struct netlink_ext_ack *extack)
#endif
{
	nss_qdisc_destroy(nq);

	memset(&nq->npq, 0, sizeof(struct nss_ppe_qdisc));
	nq->ppe_init_failed = true;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	if (nq->qdisc->ops->init(nq->qdisc, opt) < 0) {
#else
	if (nq->qdisc->ops->init(nq->qdisc, opt, extack) < 0) {
#endif
			nss_qdisc_warning("Fallback to NSS Qdisc failed.\n");
			return -EINVAL;
	}

	return 0;
}

/*
 * nss_ppe_destroy()
 *	Destroys NSS PPE Qdisc.
 */
void nss_ppe_destroy(struct nss_qdisc *nq)
{
	nss_ppe_mcast_queue_reset(nq);
	nss_ppe_shaper_reset(nq);
	nss_ppe_scheduler_reset(nq);

	if (nq->is_root) {
		nss_ppe_default_conf_set(nss_ppe_port_num_get(nq));
	}
}

/*
 * nss_ppe_init()
 *	Initializes NSS Qdisc and sets the level of this qdisc.
 */
int nss_ppe_init(struct Qdisc *sch, struct nss_qdisc *nq, nss_shaper_node_type_t type)
{
	bool is_red;
	bool alloc_scheduler = true;
	struct net_device *dev = qdisc_dev(sch);
	uint32_t port_num = nss_ppe_port_num_get(nq);

	/*
	 * HW qdisc is supported only on physical and loopbaack ports.
	 */
	if (port_num >= NSS_PPE_PORT_MAX) {
		nss_qdisc_info("HW qdisc not supported on port %d\n", port_num);
		return -1;
	}

	memset(&nq->npq, 0, sizeof(struct nss_ppe_qdisc));

	switch (type) {
	case NSS_SHAPER_NODE_TYPE_FIFO:
		nq->npq.sub_type = NSS_SHAPER_CONFIG_PPE_SN_TYPE_FIFO;
		break;

	case NSS_SHAPER_NODE_TYPE_WRED:
		is_red = sch->ops == &nss_red_qdisc_ops;
		if (!is_red) {
			nss_qdisc_info("HW WRED qdisc not supported %x\n", sch->handle);
			return -1;
		} else {
			nq->npq.sub_type = NSS_SHAPER_CONFIG_PPE_SN_TYPE_RED;
		}
		break;

	case NSS_SHAPER_NODE_TYPE_HTB:

		/*
		 * Currently we only allow one ppehtb qdisc and
		 * that should be at the root.
		 */
		if (sch->parent != TC_H_ROOT) {
			nss_qdisc_info("HW HTB qdisc needs to be root %x\n", sch->handle);
			return -1;
		}

		nq->npq.sub_type = NSS_SHAPER_CONFIG_PPE_SN_TYPE_HTB;
		alloc_scheduler = false;
		break;

	case NSS_SHAPER_NODE_TYPE_HTB_GROUP:
		nq->npq.sub_type = NSS_SHAPER_CONFIG_PPE_SN_TYPE_HTB_GROUP;
		break;

	case NSS_SHAPER_NODE_TYPE_TBL:
		nq->npq.sub_type = NSS_SHAPER_CONFIG_PPE_SN_TYPE_TBL;
		break;

	case NSS_SHAPER_NODE_TYPE_WRR:
		nq->npq.sub_type = NSS_SHAPER_CONFIG_PPE_SN_TYPE_WRR;
		break;

	case NSS_SHAPER_NODE_TYPE_WRR_GROUP:
		nq->npq.sub_type = NSS_SHAPER_CONFIG_PPE_SN_TYPE_WRR_GROUP;
		break;

	case NSS_SHAPER_NODE_TYPE_PRIO:
		nq->npq.sub_type = NSS_SHAPER_CONFIG_PPE_SN_TYPE_PRIO;
		break;

	default:
		nss_qdisc_warning("HW Qdisc not supported\n");
		return -1;
	}

	nq->npq.q.qlimit = dev->tx_queue_len ? : 1;
	nq->npq.scheduler.priority = NSS_PPE_PRIORITY_MAX;

	if (nq->is_root) {
		nq->npq.level = nss_ppe_max_level_get(nq) - 1;
	} else {
		if (!nq->parent) {
			nss_qdisc_warning("Qdisc is not root and has no parent\n");
			return -1;
		}

		/*
		 * First HTB class has a same level as HTB qdisc.
		 * While all other qdiscs and classes wil be at one level less than the parent level.
		 * This is because all other scheduler qdisc (such as TBL, WRR, etc) have some logic
		 * in qdisc that needs resource allocation in PPE. HTB qdisc on the other hand does
		 * nothing useful and thus we don't allocate any resource".
		 */
		nss_qdisc_trace("Qdisc parent = %px, handle=%x\n", nq->parent,  nq->parent->qos_tag);
		if ((nq->parent->npq.sub_type == NSS_SHAPER_CONFIG_PPE_SN_TYPE_HTB)) {
			nq->npq.level = nq->parent->npq.level;
		} else {
			nq->npq.level = nq->parent->npq.level - 1;
		}

		if (!nss_ppe_is_depth_valid(nq)) {
			nss_qdisc_warning("tree is too deep\n");
			memset(&nq->npq, 0, sizeof(struct nss_ppe_qdisc));
			return -1;
		}
	}

	nq->mode = NSS_QDISC_MODE_PPE;
	if (alloc_scheduler) {
		if (nss_ppe_scheduler_set(nq) < 0) {
			nss_qdisc_warning("%px SSDK scheduler configuration failed\n", sch);
			memset(&nq->npq, 0, sizeof(struct nss_ppe_qdisc));
			nq->mode = NSS_QDISC_MODE_NSS;
			return -1;
		}
	}

	if (nq->npq.level == NSS_PPE_SUB_QUEUE_LEVEL) {
		nq->npq.q = nq->parent->npq.q;
	}

	nq->type = NSS_SHAPER_NODE_TYPE_PPE_SN;

	nss_qdisc_info("Qdisc initialization successful\n");
	return 0;
}
