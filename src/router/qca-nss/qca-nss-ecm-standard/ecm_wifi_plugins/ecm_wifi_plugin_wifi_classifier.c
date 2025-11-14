/*
 **************************************************************************
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "ecm_wifi_plugin.h"

/*
 * ecm_wifi_plugin_get_wifi_metadata()
 *	Fill the wifi metadata and call into wlan driver.
 */
static inline uint32_t ecm_wifi_plugin_get_wifi_metadata(struct ecm_classifier_wifi_metadata *wifi_metadata)
{
	struct qca_wifi_metadata_info metadata = {0};
	uint32_t skb_mark = 0;

	metadata.is_mlo_param_valid = (wifi_metadata->valid_params_flag & ECM_CLASSIFIER_WIFI_MLO_PARAM_VALID);
	metadata.is_sawf_param_valid = 0;
	metadata.mlo_param.in_dest_dev = wifi_metadata->wifi_mdata.dest_dev;
	metadata.mlo_param.in_dest_mac = wifi_metadata->wifi_mdata.dest_mac;
	metadata.mlo_param.out_ppe_ds_node_id = ECM_WIFI_PLUGIN_METADATA_INVALID_DS_NODE;
	skb_mark = qca_wifi_get_metadata_info(&metadata);
	wifi_metadata->wifi_mdata.out_ppe_ds_node_id = metadata.mlo_param.out_ppe_ds_node_id;

	return skb_mark;
}

/*
 * Register WIFI related callbacks with
 *	ECM Wifi classifier to get wifi metadata info.
 */
static struct ecm_classifier_wifi_callbacks ecm_wifi_plugin_wifi = {
	.get_wifi_metadata = ecm_wifi_plugin_get_wifi_metadata,
};

/*
 * ecm_wifi_plugin_wifi_cb_register()
 *	Register WIFI callbacks.
 */
int ecm_wifi_plugin_wifi_cb_register(void)
{
	if (ecm_classifier_wifi_callback_register(&ecm_wifi_plugin_wifi)) {
		ecm_wifi_plugin_warning("ecm wifi classifier Wifi callback registration failed\n");
		return -1;
	}

	ecm_wifi_plugin_info("Wifi classifier callbacks registered\n");
	return 0;
}

/*
 * ecm_wifi_plugin_wifi_cb_unregister()
 *	Unregister Wifi callbacks.
 */
void ecm_wifi_plugin_wifi_cb_unregister(void)
{
	ecm_classifier_wifi_callback_unregister();
	ecm_wifi_plugin_info("Wifi classifier callbacks unregistered\n");
}
