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
 * ecm_wifi_plugin_get_peer_priority()
 *	Get peer priority callback into wlan driver.
 */
static inline int ecm_wifi_plugin_get_peer_priority(struct ecm_classifier_mscs_get_priority_info *get_priority_info)
{
	struct qca_mscs_get_priority_param wlan_get_priority_param = {0};

	wlan_get_priority_param.dst_mac = get_priority_info->dst_mac;
	wlan_get_priority_param.src_mac = get_priority_info->src_mac;
	wlan_get_priority_param.src_dev = get_priority_info->src_dev;
	wlan_get_priority_param.dst_dev = get_priority_info->dst_dev;
	wlan_get_priority_param.skb = get_priority_info->skb;

	return qca_mscs_peer_lookup_n_get_priority_v2(&wlan_get_priority_param);
}

/*
 * ecm_wifi_plugin_update_skb_priority()
 *	Update skb priority callback into wlan driver.
 */
static inline bool ecm_wifi_plugin_update_skb_priority(struct ecm_classifier_mscs_rule_match_info *match_info)
{
	struct qca_scs_peer_lookup_n_rule_match rule_match_param = {0};

	rule_match_param.dst_mac_addr = match_info->dst_mac;
	rule_match_param.rule_id = match_info->rule_id;
	rule_match_param.src_dev = match_info->src_dev;
	rule_match_param.dst_dev = match_info->dst_dev;

	return qca_scs_peer_lookup_n_rule_match_v2(&rule_match_param);
}

/*
 * ecm_wifi_plugin
 * 	Register MSCS client callback with ECM MSCS classifier to support MSCS wifi peer lookup.
 */
static struct ecm_classifier_mscs_callbacks ecm_wifi_plugin_mscs = {
	.get_peer_priority = ecm_wifi_plugin_get_peer_priority,
#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
	.update_skb_priority = ecm_wifi_plugin_update_skb_priority,
#endif
};

/*
 * ecm_wifi_plugin_mscs_register()
 *	register mscs callbacks.
 */
int ecm_wifi_plugin_mscs_register(void)
{
	if (ecm_classifier_mscs_callback_register(&ecm_wifi_plugin_mscs)) {
		ecm_wifi_plugin_warning("ecm mscs classifier callback registration failed.\n");
		return -1;
	}

	ecm_wifi_plugin_info("MSCS callbacks registered\n");
	return 0;
}

/*
 * ecm_wifi_plugin_mscs_unregister()
 *	unregister the mscs callbacks.
 */
void ecm_wifi_plugin_mscs_unregister(void)
{
	ecm_classifier_mscs_callback_unregister();
	ecm_wifi_plugin_info("MSCS callbacks unregistered\n");
}
