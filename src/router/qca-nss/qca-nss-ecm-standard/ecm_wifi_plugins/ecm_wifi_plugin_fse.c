/*
 **************************************************************************
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
 **************************************************************************
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/skbuff.h>

#include "ecm_wifi_plugin.h"

/*
 * ecm_wifi_plugin_fill_fse_wlan_info()
 *	Fill the FSE info for wlan FSE rule add / delete.
 */
static inline void ecm_wifi_plugin_fill_fse_wlan_info(struct ecm_front_end_fse_info *fse_info,
						      struct qca_fse_flow_info *fse_wlan_info)
{
	/*
	 * Fill the wlan tuple info.
	 */
	memcpy(&fse_wlan_info->src_ip, &fse_info->src, sizeof(fse_wlan_info->src_ip));
	memcpy(&fse_wlan_info->dest_ip, &fse_info->dest, sizeof(fse_wlan_info->dest_ip));
	fse_wlan_info->src_dev = fse_info->src_dev;
	fse_wlan_info->dest_dev = fse_info->dest_dev;
	fse_wlan_info->src_port = fse_info->src_port;
	fse_wlan_info->dest_port = fse_info->dest_port;
	fse_wlan_info->protocol = fse_info->protocol;
	fse_wlan_info->protocol = fse_info->protocol;
	fse_wlan_info->version = fse_info->ip_version;
	fse_wlan_info->src_mac = fse_info->src_mac;
	fse_wlan_info->dest_mac = fse_info->dest_mac;
	fse_wlan_info->fw_svc_id = ECM_CLASSIFIER_EMESH_SAWF_INVALID_SVID;
	fse_wlan_info->rv_svc_id = ECM_CLASSIFIER_EMESH_SAWF_INVALID_SVID;
}

/*
 * ecm_wifi_plugin_fse_create_rule()
 *	Fill the FSE info and call into wlan driver to create FSE rule.
 */
bool ecm_wifi_plugin_fse_create_rule(struct ecm_front_end_fse_info *fse_info)
{
	struct qca_fse_flow_info fse_wlan_info = {0};

	/*
	 * Fill the wlan tuple info and call wlan callback.
	 */
	ecm_wifi_plugin_fill_fse_wlan_info(fse_info, &fse_wlan_info);
	return qca_fse_add_rule(&fse_wlan_info);
}

/*
 * ecm_wifi_plugin_fse_destroy_rule()
 *	Fill the FSE info and call into wlan driver to destroy FSE rule.
 */
bool ecm_wifi_plugin_fse_destroy_rule(struct ecm_front_end_fse_info *fse_info)
{
	struct qca_fse_flow_info fse_wlan_info = {0};

	/*
	 * Fill the wlan tuple info and call wlan callback.
	 */
	ecm_wifi_plugin_fill_fse_wlan_info(fse_info, &fse_wlan_info);
	return qca_fse_delete_rule(&fse_wlan_info);
}

/*
 * ecm_wifi_plugin_fse_ops
 *	Register Wi-Fi FSE (Flow search Engine) related callbacks with
 *	ECM frontend to program FSE rules from ECM.
 */
static struct ecm_front_end_fse_callbacks ecm_wifi_plugin_fse_ops = {
	.create_fse_rule = ecm_wifi_plugin_fse_create_rule,
	.destroy_fse_rule = ecm_wifi_plugin_fse_destroy_rule,
};

/*
 * ecm_wifi_plugin_fse_cb_register()
 *	Register FSE callbacks with ECM frontend.
 */
int ecm_wifi_plugin_fse_cb_register(void)
{
	if (ecm_front_end_fse_callbacks_register(&ecm_wifi_plugin_fse_ops)) {
		ecm_wifi_plugin_warning("FSE callback registration failed for ECM frontend\n");
		return -1;
	}

	ecm_wifi_plugin_info("ECM frontend FSE callbacks registered\n");
	return 0;
}

/*
 * ecm_wifi_plugin_fse_cb_unregister()
 *	Unregister FSE callbacks with ECM frontend.
 */
void ecm_wifi_plugin_fse_cb_unregister(void)
{
	ecm_front_end_fse_callbacks_unregister();
	ecm_wifi_plugin_info("ECM frontend FSE callbacks unregistered\n");
}
