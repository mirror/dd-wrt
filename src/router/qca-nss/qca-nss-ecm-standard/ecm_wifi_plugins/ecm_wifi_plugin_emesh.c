/*
 **************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
/*
 * ecm_wifi_plugin_emesh_sawf_update_peer_mesh_params()
 *	Update mesh latency params.
 */
static inline void ecm_wifi_plugin_emesh_sawf_update_peer_mesh_params(struct ecm_classifer_emesh_sawf_mesh_latency_params *mesh_params)
{
	struct qca_mesh_latency_update_peer_param wlan_mesh_params = {0};

	wlan_mesh_params.dest_mac = mesh_params->peer_mac;
	wlan_mesh_params.src_dev = mesh_params->src_dev;
	wlan_mesh_params.dst_dev = mesh_params->dst_dev;
	wlan_mesh_params.service_interval_dl = mesh_params->service_interval_dl;
	wlan_mesh_params.service_interval_ul = mesh_params->service_interval_ul;
	wlan_mesh_params.burst_size_dl = mesh_params->burst_size_dl;
	wlan_mesh_params.burst_size_ul = mesh_params->burst_size_ul;
	wlan_mesh_params.priority = mesh_params->priority;
	wlan_mesh_params.add_or_sub = mesh_params->accel_or_decel;

	qca_mesh_latency_update_peer_parameter_v2(&wlan_mesh_params);
}

/*
 * ecm_wifi_plugin_emesh_sawf_update_fse_flow()
 *	Update FSE entry callback.
 */
static inline bool ecm_wifi_plugin_emesh_sawf_update_fse_flow(struct ecm_classifier_fse_info *fse_info)
{
	struct qca_fse_flow_info fse_wlan_info = {0};

	memcpy(&fse_wlan_info.src_ip, &fse_info->src, sizeof(fse_wlan_info.src_ip));
	memcpy(&fse_wlan_info.dest_ip, &fse_info->dest, sizeof(fse_wlan_info.dest_ip));
	fse_wlan_info.src_dev = fse_info->src_dev;
	fse_wlan_info.dest_dev = fse_info->dest_dev;
	fse_wlan_info.src_port = fse_info->src_port;
	fse_wlan_info.dest_port = fse_info->dest_port;
	fse_wlan_info.protocol = fse_info->protocol;
	fse_wlan_info.version = fse_info->ip_version;
	fse_wlan_info.src_mac = fse_info->src_mac;
	fse_wlan_info.dest_mac = fse_info->dest_mac;
	fse_wlan_info.fw_svc_id = fse_info->fw_svc_info;
	fse_wlan_info.rv_svc_id = fse_info->rv_svc_info;

	return qca_fse_add_rule(&fse_wlan_info);
}
#endif

/*
 * ecm_wifi_plugin_emesh_sawf_conn_sync()
 *	Connection sync callback for EMESH-SAWF classifier.
 */
static inline void ecm_wifi_plugin_emesh_sawf_conn_sync(struct ecm_classifer_emesh_sawf_sync_params *sawf_sync_params)
{
#ifdef ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
	struct ath_ul_params sawf_params = {0};
#else
	struct qca_sawf_connection_sync_param sawf_params = {0};
#endif

	sawf_params.src_dev = sawf_sync_params->src_dev;
	sawf_params.dst_dev = sawf_sync_params->dest_dev;
	sawf_params.dst_mac = sawf_sync_params->dest_mac;
	sawf_params.src_mac = sawf_sync_params->src_mac;
	sawf_params.fw_service_id = sawf_sync_params->fwd_service_id;
	sawf_params.rv_service_id = sawf_sync_params->rev_service_id;
	sawf_params.start_or_stop = sawf_sync_params->add_or_sub;

#ifdef ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
	ath_sawf_uplink(&sawf_params);
#else
	sawf_params.fw_mark_metadata = sawf_sync_params->fwd_mark_metadata;
	sawf_params.rv_mark_metadata = sawf_sync_params->rev_mark_metadata;
	qca_sawf_connection_sync(&sawf_params);
#endif
}

#ifndef ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
/*
 * ecm_wifi_plugin_emesh_ecm_valid_to_wifi_valid()
 *	Convert the ECM SAWF valid flags to Wi-Fi driver valid flags.
 */
static inline uint32_t ecm_wifi_plugin_emesh_ecm_valid_to_wifi_valid(uint32_t valid_flag)
{
	if (valid_flag & ECM_CLASSIFIER_EMESH_SAWF_SVID_VALID) {
		return QCA_SAWF_SVID_VALID;
	}

	if (valid_flag & ECM_CLASSIFIER_EMESH_SAWF_DSCP_VALID) {
		return QCA_SAWF_DSCP_VALID;
	}

	if (valid_flag & ECM_CLASSIFIER_EMESH_SAWF_VLAN_PCP_VALID) {
		return QCA_SAWF_PCP_VALID;
	}

	return 0;
}
#endif

/*
 * ecm_wifi_plugin_emesh_sawf_get_mark_data()
 * 	get skb mark callback for EMESH-SAWF classifier.
 */
static inline uint32_t ecm_wifi_plugin_emesh_sawf_get_mark_data(struct ecm_classifier_emesh_sawf_flow_info *sawf_flow_info)
{
#ifdef ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
	uint32_t msduq = 0;
	uint32_t sawf_mark = 0;
	struct ath_dl_params sawf_params = {0};
#else
	struct qca_sawf_metadata_param sawf_params = {0};
#endif

	sawf_params.netdev = sawf_flow_info->netdev;
	sawf_params.peer_mac = sawf_flow_info->peer_mac;
	sawf_params.service_id = sawf_flow_info->service_id;
	sawf_params.dscp = sawf_flow_info->dscp;
	sawf_params.rule_id = sawf_flow_info->rule_id;

#ifdef ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
	/*
	 * For upstream driver we can call the query only for the SVID valid case.
	 */
	if (!(sawf_flow_info->valid_flag & ECM_CLASSIFIER_EMESH_SAWF_SVID_VALID)) {
		return ECM_CLASSIFIER_EMESH_SAWF_INVALID_MSDUQ;
	}

	msduq = ath_sawf_downlink(&sawf_params);
	sawf_mark |= ECM_WIFI_PLUGIN_SAWF_TAG;
	sawf_mark <<= ECM_WIFI_PLUGIN_SAWF_TAG_SHIFT;
	sawf_mark |= (sawf_params.service_id & ECM_WIFI_PLUGIN_SAWF_SERVICE_CLASS_MASK);
	sawf_mark <<= ECM_WIFI_PLUGIN_SAWF_SERVICE_CLASS_SHIFT;
	sawf_mark |= (msduq & ECM_WIFI_PLUGIN_SAWF_MSDUQ_MASK);

	return sawf_mark;
#else
	sawf_params.sawf_rule_type = sawf_flow_info->sawf_rule_type;
	sawf_params.pcp = sawf_flow_info->vlan_pcp;
	sawf_params.dscp = sawf_flow_info->dscp;
	sawf_params.valid_flag = ecm_wifi_plugin_emesh_ecm_valid_to_wifi_valid(sawf_flow_info->valid_flag);

	return qca_sawf_get_mark_metadata(&sawf_params);
#endif
}

/*
 * ecm_wifi_plugin_emesh
 * 	Register EMESH client callback with ECM EMSH classifier to update peer mesh latency parameters.
 */
static struct ecm_classifier_emesh_sawf_callbacks ecm_wifi_plugin_emesh = {
#ifndef ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
	.update_peer_mesh_latency_params = ecm_wifi_plugin_emesh_sawf_update_peer_mesh_params,
	.update_fse_flow_info = ecm_wifi_plugin_emesh_sawf_update_fse_flow,
#endif
	.update_service_id_get_msduq = ecm_wifi_plugin_emesh_sawf_get_mark_data,
	.sawf_conn_sync = ecm_wifi_plugin_emesh_sawf_conn_sync,
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

	if (ecm_classifier_emesh_sawf_conn_sync_callback_register(&ecm_wifi_plugin_emesh)) {
		ecm_classifier_emesh_latency_config_callback_unregister();
		ecm_classifier_emesh_sawf_msduq_callback_unregister();
		ecm_wifi_plugin_warning("ecm emesh config sawf ul callback registration failed.\n");
		return -1;
	}

	if (ecm_classifier_emesh_sawf_update_fse_flow_callback_register(&ecm_wifi_plugin_emesh)) {
		ecm_classifier_emesh_latency_config_callback_unregister();
		ecm_classifier_emesh_sawf_msduq_callback_unregister();
		ecm_classifier_emesh_sawf_conn_sync_callback_unregister();
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
	ecm_classifier_emesh_sawf_conn_sync_callback_unregister();
	ecm_wifi_plugin_info("EMESH classifier callbacks unregistered\n");
}
