/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "nss_core.h"
#include <nss_dp_api_if.h>
#include "nss_tx_rx_common.h"
#include "nss_data_plane_lite.h"

static struct delayed_work nss_data_plane_lite_work;
static struct workqueue_struct *nss_data_plane_lite_workqueue;

extern uint16_t pn_qlimits[NSS_MAX_NUM_PRI];
extern bool pn_mq_en;

/*
 * nss_data_plane_lite_register_to_nss_dp()
 */
static bool nss_data_plane_lite_register_to_nss_dp(struct nss_ctx_instance *nss_ctx, int if_num)
{
	struct nss_top_instance *nss_top = nss_ctx->nss_top;
	struct net_device *netdev;
	int core;

	netdev = nss_dp_get_netdev_by_nss_if_num(if_num);
	if (!netdev) {
		nss_info("%px: Platform doesn't have data plane%d enabled, \
				don't bring up nss_phys_if and don't register to nss-dp\n",
				nss_ctx, if_num);
		return false;
	}

	/*
	 * Setup the receive callback so that data pkts received form NSS-FW will
	 * be redirected to the nss-dp driver as we are overriding the data plane
	 */
	nss_top->phys_if_handler_id[if_num] = nss_ctx->id;

	/*
	 * Packets recieved on physical interface can be exceptioned to HLOS
	 * from any NSS core so we need to register data plane for all
	 */
	for (core = 0; core < nss_top->num_nss; core++) {
		nss_core_register_subsys_dp(&nss_top->nss[core], if_num, nss_dp_receive, NULL, NULL, netdev, 0);
	}

	return true;
}

/*
 * nss_data_plane_lite_register()
 *	Register the lite version of data plane
 */
void nss_data_plane_lite_register(struct nss_ctx_instance *nss_ctx)
{
	int i;

	for (i = NSS_DP_START_IFNUM; i < NSS_DP_MAX_INTERFACES; i++) {
		if (!nss_data_plane_lite_register_to_nss_dp(nss_ctx, i)) {
			nss_warning("%px: Register data plane failed for data plane %d\n", nss_ctx, i);
		} else {
			nss_info("%px: Register data plan to data plane %d successful\n", nss_ctx, i);
		}
	}
}

/*
 * nss_data_plane_lite_unregister()
 *	Unregister the lite version of data plane.
 */
void nss_data_plane_lite_unregister(void)
{
	int i, core;

	for (core = 0; core < nss_top_main.num_nss; core++) {
		for (i = NSS_DP_START_IFNUM; i < NSS_DP_MAX_INTERFACES; i++) {
			if (nss_top_main.nss[core].subsys_dp_register[i].ndev) {
				nss_core_unregister_subsys_dp(&nss_top_main.nss[core], i);
			}
		}
	}
}

/*
 * nss_data_plane_lite_work_function()
 *	Work function that gets queued to configure multi - queue support.
 */
static void nss_data_plane_lite_work_function(struct work_struct *work)
{
	int ret;
	struct nss_ctx_instance *nss_ctx = &nss_top_main.nss[NSS_CORE_0];

	/*
	 * The queue config command is a synchronous command and needs to be issued
	 * in process context, before NSS data plane switch.
	 */
	ret = nss_n2h_update_queue_config_sync(nss_ctx, pn_mq_en, pn_qlimits);
	if (ret != NSS_TX_SUCCESS) {
		nss_warning("%px: Failed to send pnode queue config to core 0\n", nss_ctx);
	}
}

/*
 * nss_data_plane_lite_schedule_registration()
 *	Called from nss_init to schedule a work to do data_plane_lite register to data plane host
 */
bool nss_data_plane_lite_schedule_registration(void)
{
	if (!queue_work_on(1, nss_data_plane_lite_workqueue, &nss_data_plane_lite_work.work)) {
		nss_warning("Failed to register workqueue to configure data plane lite\n");
		return false;
	}

	nss_info("Register workqueue to configure data plane lite successful\n");
	return true;
}

/*
 * nss_data_plane_lite_init_delay_work()
 *	Called from nss_init to initialize the create and initialize the work queue.
 */
int nss_data_plane_lite_init_delay_work(void)
{
	nss_data_plane_lite_workqueue = create_singlethread_workqueue("nss_data_plane_lite_workqueue");
	if (!nss_data_plane_lite_workqueue) {
		nss_warning("Can't allocate workqueue\n");
		return -ENOMEM;
	}

	INIT_DELAYED_WORK(&nss_data_plane_lite_work, nss_data_plane_lite_work_function);
	return 0;
}

/*
 * nss_data_plane_destroy_delay_work()
 */
void nss_data_plane_lite_destroy_delay_work(void)
{
	destroy_workqueue(nss_data_plane_lite_workqueue);
}
