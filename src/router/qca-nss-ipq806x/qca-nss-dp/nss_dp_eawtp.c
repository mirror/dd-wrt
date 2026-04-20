/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 *
 */

#include <soc/qcom/eawtp.h>
#include <fal/fal_port_ctrl.h>
#include <ref/ref_port_ctrl.h>
#include "nss_dp_dev.h"

/* NSS DP EAWTP global data */
struct nss_dp_eawtp_ctx {
	struct eawtp_reg_info *reg_info;
}eawtp_ctx;

/*
 * eawtp_get_num_active_ports()
 *	Get the number of active ports
 */
int eawtp_get_num_active_ports(void *app_data, struct eawtp_port_info *pinfo)
{
	uint32_t port_id;
	uint32_t i;
	uint32_t port_cnt = 0;
	uint32_t ret;
	a_bool_t status = 0;

	for (port_id = 0; port_id < NSS_DP_HAL_MAX_PORTS; port_id++) {
		struct nss_dp_dev *dp_dev = dp_global_ctx.nss_dp[port_id];
		if (!dp_dev) {
			continue;
		}

		pr_debug("\nEAWTP dev:%p id:%d port:%d\n", dp_dev, dp_dev->macid, port_id);

		if (!dp_dev->is_switch_connected) {
			ret = fal_port_link_status_get(NSS_DP_DEV_ID, dp_dev->macid, &status);
			if (status) {
				++port_cnt;
			}
		} else {
			/* MHT ports  */
			for (i = 1; i <= NSS_DP_HAL_MHT_SWT_MAX_PORTS; i++) {
				status = 0;
				fal_port_link_status_get(NSS_DP_MHT_SW_ID, i, &status);
				if (status) {
					++port_cnt;
				}
			}
		}
	}

	pr_debug("\nEAWTP num active port cnt:%d\n", port_cnt);
	pinfo->num_active_port = port_cnt;
	return port_cnt;
}

/*
 * eawtp_link_event()
 *	Port link update event
 */
static int eawtp_link_event(struct notifier_block *unused, unsigned long event,
                void *ptr)
{
	struct eawtp_port_info pinfo;
	ssdk_port_status *link_status_p;
	struct eawtp_reg_info *ntfy_info = eawtp_ctx.reg_info;
	link_status_p = ptr;

	pr_debug("\nEAWTP link %s to notify\n",
			link_status_p->port_link ? "UP" : "DOWN");

	if (ntfy_info->ntfy_active_ports_cb) {
		eawtp_get_num_active_ports(ntfy_info, &pinfo);
		ntfy_info->ntfy_active_ports_cb(0, &pinfo);
	}

        return NOTIFY_DONE;
}

static struct notifier_block eawtp_link_notifier = { .notifier_call = eawtp_link_event };

/*
 * eawtp_port_link_status_ntfy_reg()
 *	Register the port link notifier.
 */
int eawtp_port_link_status_ntfy_reg(void *app_data)
{
	return ssdk_port_link_notify_register(&eawtp_link_notifier);
}

/*
 * eawtp_port_link_status_ntfy_unreg()
 *	Unregister the port link notifier.
 */
int eawtp_port_link_status_ntfy_unreg(void *app_data)
{
	return ssdk_port_link_notify_unregister(&eawtp_link_notifier);
}

/*
 * eawtp_nss_unregister_cb)
 *	Unregister the callback
 */
int eawtp_nss_unregister_cb(void)
{
	if (!eawtp_ctx.reg_info) {
		pr_info("\nEAWTP Unregister called without register\n");
		return -1;
	}

	eawtp_ctx.reg_info = NULL;
	return 0;
}
EXPORT_SYMBOL(eawtp_nss_unregister_cb);

/*
 * eawtp_nss_get_and_register_cb()
 *	Get and register cb
 */
int eawtp_nss_get_and_register_cb(struct eawtp_reg_info *reg_info)
{
	if (!reg_info) {
		pr_info("\nEAWTP Registration info cannot be NULL\n");
		return -1;
	}

	eawtp_ctx.reg_info = reg_info;
	reg_info->get_nss_active_port_cnt_cb = eawtp_get_num_active_ports;
	reg_info->port_link_notify_register_cb = eawtp_port_link_status_ntfy_reg;
	reg_info->port_link_notify_unregister_cb = eawtp_port_link_status_ntfy_unreg;
	return 0;
}
EXPORT_SYMBOL(eawtp_nss_get_and_register_cb);
