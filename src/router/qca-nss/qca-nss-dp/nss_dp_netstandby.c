/*
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2021-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netstandby.h>
#include <fal/fal_port_ctrl.h>
#include <ppe_drv.h>
#include <ppe_drv_port.h>
#include "nss_dp_hal.h"

#define DP_STANDBY_SWITCH_DEV_ID 0
#define DP_STANDBY_SWITCH_MHT_DEV_ID 1

struct nss_dp_netstandby_gbl_ctx standby_gbl_ctx;

/*
 * nss_dp_netstandby_exit_standby()
 *	Exit standby API()
 */
int nss_dp_netstandby_exit_standby(void *app_data, struct netstandby_exit_info *exit_info)
{
	struct nss_dp_netstandby_gbl_ctx *gbl_ctx = (struct nss_dp_netstandby_gbl_ctx *)app_data;
	sw_error_t sw_err;

	sw_err = fal_erp_standby_exit(DP_STANDBY_SWITCH_DEV_ID);
	if (sw_err != SW_OK) {
		pr_warn("Error in bringing device out of power state\n");
		return -ENOTSUPP;
	}

#if defined(NSS_DP_IPQ53XX)
	sw_err = fal_erp_standby_exit(DP_STANDBY_SWITCH_MHT_DEV_ID);
	if (sw_err != SW_OK) {
		pr_warn("Error in bringing MHT device out of power state\n");
		return -ENOTSUPP;
	}
#endif

	/*
	 * Invoke exit completion
	 */
	if (gbl_ctx->exit_cmp_cb) {
		struct netstandby_event_compl_info event_info = {0};
		event_info.system_type = NETSTANDBY_SUBSYSTEM_TYPE_NSS;
		event_info.event_type = NETSTANDBY_NOTIF_EXIT_COMPLETE;
		gbl_ctx->exit_cmp_cb(app_data, &event_info);
	}

	return 0;
}

/*
 * nss_dp_netstandby_enter_standby()
 *	Enter standby API()
 */
int nss_dp_netstandby_enter_standby(void *app_data, struct netstandby_entry_info *entry_info)
{
	struct nss_dp_netstandby_gbl_ctx *gbl_ctx = (struct nss_dp_netstandby_gbl_ctx *)app_data;
	struct nss_dp_global_ctx *ctx = gbl_ctx->ctx;
	sw_error_t sw_err;
	int i = 0;
	struct net_device *dev;
	int32_t mac_id = 0;
	uint32_t port_active_bmap = 0;
#if defined(NSS_DP_IPQ53XX)
	uint32_t port_mht_active_bmap = 0;
	bool all_mht_down = false;
	int index;
#endif

	if (entry_info->iface_cnt != 0) {
		for (i = 0; i < NSS_DP_HAL_MAX_PORTS; i++) {
			int j = 0;

			if (!ctx->nss_dp[i])
				continue;

			dev = ctx->nss_dp[i]->netdev;

			/*
			 * If netdevice is NULL; then continue to next index
			 */
			if (!dev) {
				continue;
			}

			/*
			 * List sent to subsystem contains devices that need to be in wakeup state to receive trigger;
			 * power down the remaining interfaces
			 */
			for (j = 0; j < entry_info->iface_cnt; j++) {
				if (entry_info->dev[j] == dev) {
					mac_id = ctx->nss_dp[i]->macid;
#if defined(NSS_DP_IPQ53XX)
					index = nss_dp_get_idx_from_macid(mac_id);
					if (ctx->nss_dp[index]->is_switch_connected) {
						break;
					}
#endif
					port_active_bmap |= (1 << mac_id);
					break;
				}
			}

		}
	}

	sw_err = fal_erp_standby_enter(DP_STANDBY_SWITCH_DEV_ID, port_active_bmap);
	if (sw_err != SW_OK) {
		pr_warn("%p Error in putting device(%d) to power state(%d)\n", ctx, mac_id, FAL_ERP_LOW_POWER);
		return -ENOTSUPP;
	}

#if defined(NSS_DP_IPQ53XX)
	if (entry_info->nss_info.port_id > NSS_DP_HAL_MHT_SWT_MAX_PORTS) {
		pr_warn("%p Port_id out of range(%d)\n", ctx, entry_info->nss_info.port_id);
		fal_erp_standby_exit(DP_STANDBY_SWITCH_DEV_ID);
		return -EINVAL;
	}

	if ((entry_info->nss_info.flags & NETSTANDBY_ENTER_NSS_FLAG_VALID_PORT_IDX) == NETSTANDBY_ENTER_NSS_FLAG_VALID_PORT_IDX)
		port_mht_active_bmap |= (1 << entry_info->nss_info.port_id);
	else if ((entry_info->nss_info.flags & NETSTANDBY_ENTER_NSS_FLAG_VALID_PORT_NONE) == NETSTANDBY_ENTER_NSS_FLAG_VALID_PORT_NONE)
		all_mht_down = true;

	/*
	 * TODO: remove this when APP sets NETSTANDBY_ENTER_NSS_FLAG_VALID_PORT_NONE for cases
	 * where enter command does not take any dev (all port should go down).
	 */
	if (entry_info->iface_cnt == 0) {
		all_mht_down = true;
	}

	if (port_mht_active_bmap || all_mht_down) {
		sw_err = fal_erp_standby_enter(DP_STANDBY_SWITCH_MHT_DEV_ID, port_mht_active_bmap);
		if (sw_err != SW_OK) {
			pr_warn("%p Error in putting device(%d) to power state(%d)\n", ctx, mac_id, FAL_ERP_LOW_POWER);
			fal_erp_standby_exit(DP_STANDBY_SWITCH_DEV_ID);
			return -EINVAL;
		}
	}
#endif

	/*
	 * Indicate completion to standby module
	 */
	if (gbl_ctx->enter_cmp_cb) {
		struct netstandby_event_compl_info event_info = {0};
		event_info.system_type = NETSTANDBY_SUBSYSTEM_TYPE_NSS;
		event_info.event_type = NETSTANDBY_NOTIF_ENTER_COMPLETE;
		gbl_ctx->enter_cmp_cb(app_data, &event_info);
	}

	return 0;
}

/*
 * nss_dp_get_eth_info
 * 	Retrieve the info related to ethernet ports
 */
bool nss_dp_get_eth_info(struct nss_dp_eth_netdev_info ethinfo[], uint8_t array_size)
{
	struct nss_dp_global_ctx *dp_global = &dp_global_ctx;
	struct nss_dp_dev *dp_priv;
	uint8_t i  = 0;
#if defined(NSS_DP_IPQ53XX)
	int port_id = 0;
	sw_error_t ret;
	a_bool_t status;
#endif

	if (array_size != NSS_DP_MAX_INTERFACES - 1)
		return false;

	for (i = 0; i < array_size; i++) {
		dp_priv = dp_global->nss_dp[i];

		if (!dp_priv) {
			pr_warn("%p Error in retrieving netdev for ethernet port %d\n", dp_global, i);
			return false;
		}

		ethinfo[i].netdev = dp_priv->netdev;
#if defined(NSS_DP_IPQ53XX)
		if (dp_global->nss_dp[i]->is_switch_connected) {
			ethinfo[i].switch_connected = true;
			for (port_id = 1; port_id <= MAX_MHT_PORTS ; port_id++) {
				ret = fal_port_link_status_get(DP_STANDBY_SWITCH_MHT_DEV_ID, port_id,  &status);
				if (ret != SW_OK) {
					pr_warn("Link get status failed for port:%d ret:%d\n", port_id, ret);
					return false;
				}
				ethinfo[i].mht_port_status[port_id - 1] = status;
			}
		}
#endif
	}

	return true;
}
EXPORT_SYMBOL(nss_dp_get_eth_info);

/*
 * nss_dp_get_and_register_cb()
 *	Get and register cb
 */
int nss_dp_get_and_register_cb(struct netstandby_reg_info *info)
{
	struct nss_dp_netstandby_gbl_ctx *standby_ctx = &standby_gbl_ctx;
	info->enter_cb = nss_dp_netstandby_enter_standby;
	info->exit_cb = nss_dp_netstandby_exit_standby;

	standby_ctx->ctx = &dp_global_ctx;
	standby_ctx->enter_cmp_cb = info->enter_cmp_cb;
	standby_ctx->exit_cmp_cb = info->exit_cmp_cb;

	info->app_data = standby_ctx;
	return 0;
}
EXPORT_SYMBOL(nss_dp_get_and_register_cb);
