/*
 **************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
 * ecm_wifi_plugin_emesh
 * 	Register EMESH client callback with ECM EMSH classifier to update peer mesh latency parameters.
 */
static struct ecm_classifier_emesh_sawf_callbacks ecm_wifi_plugin_emesh = {
	.update_peer_mesh_latency_params = qca_mesh_latency_update_peer_parameter,
	.update_service_id_get_msduq = qca_sawf_get_msduq,
	.update_sawf_ul = qca_sawf_config_ul,
};

/*
 * ecm_wifi_plugin_emesh_register()
 *	Register emesh callbacks.
 */
int ecm_wifi_plugin_emesh_register(void)
{
	if (ecm_classifier_emesh_latency_config_callback_register(&ecm_wifi_plugin_emesh)) {
		ecm_wifi_plugin_warning("ecm emesh classifier callback registration failed.\n");
		return -1;
	}

	if (ecm_classifier_emesh_sawf_msduq_callback_register(&ecm_wifi_plugin_emesh)) {
		ecm_classifier_emesh_latency_config_callback_unregister();
		ecm_wifi_plugin_warning("ecm emesh msduq callback registration failed.\n");
		return -1;
	}

	if (ecm_classifier_emesh_sawf_config_ul_callback_register(&ecm_wifi_plugin_emesh)) {
		ecm_classifier_emesh_latency_config_callback_unregister();
		ecm_classifier_emesh_sawf_msduq_callback_unregister();
		ecm_wifi_plugin_warning("ecm emesh config sawf ul callback registration failed.\n");
		return -1;
	}

	if (ecm_classifier_emesh_sawf_update_fse_flow_callback_register(&ecm_wifi_plugin_emesh)) {
		ecm_classifier_emesh_latency_config_callback_unregister();
		ecm_classifier_emesh_sawf_msduq_callback_unregister();
		ecm_classifier_emesh_sawf_config_ul_callback_unregister();
		ecm_wifi_plugin_warning("ecm emesh fse callback registration failed.\n");
		return -1;
	}
	ecm_wifi_plugin_info("EMESH classifier callbacks registered\n");
	return 0;
}

/*
 * ecm_wifi_plugin_emesh_unregister()
 *	unregister the emesh callbacks.
 */
void ecm_wifi_plugin_emesh_unregister(void)
{
	ecm_classifier_emesh_latency_config_callback_unregister();
	ecm_classifier_emesh_sawf_msduq_callback_unregister();
	ecm_classifier_emesh_sawf_update_fse_flow_callback_unregister();
	ecm_classifier_emesh_sawf_config_ul_callback_unregister();
	ecm_wifi_plugin_info("EMESH classifier callbacks unregistered\n");
}
