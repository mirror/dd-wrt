/*
 **************************************************************************
 * Copyright (c) 2014-2016, 2018, 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <linux/module.h>
#include <linux/of.h>
#include <linux/debugfs.h>
#include <linux/inet.h>
#include <linux/etherdevice.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/ip.h>
#include <net/ipv6.h>
#ifdef ECM_FRONT_END_PPE_ENABLE
#include <ppe_drv.h>
#endif

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_INIT_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_db.h"
#include "ecm_front_end_ipv4.h"
#ifdef ECM_IPV6_ENABLE
#include "ecm_front_end_ipv6.h"
#endif
#include "ecm_front_end_common.h"
#include "ecm_conntrack_notifier.h"

enum ecm_front_end_type selected_front_end;

int front_end_selection;
module_param(front_end_selection, int, 0);
MODULE_PARM_DESC(front_end_selection, "Select front end for ECM");

struct dentry *ecm_dentry;	/* Dentry object for top level ecm debugfs directory */

extern int ecm_db_init(struct dentry *dentry);
extern void ecm_db_connection_defunct_all(void);
extern void ecm_db_exit(void);

extern int ecm_classifier_default_init(struct dentry *dentry);
extern void ecm_classifier_default_exit(void);

#ifdef ECM_CLASSIFIER_OVS_ENABLE
extern int ecm_classifier_ovs_init(struct dentry *dentry);
extern void ecm_classifier_ovs_exit(void);
#endif

#ifdef ECM_CLASSIFIER_MARK_ENABLE
extern int ecm_classifier_mark_init(struct dentry *dentry);
extern void ecm_classifier_mark_exit(void);
#endif

#ifdef ECM_CLASSIFIER_NL_ENABLE
extern int ecm_classifier_nl_rules_init(struct dentry *dentry);
extern void ecm_classifier_nl_rules_exit(void);
#endif

#ifdef ECM_CLASSIFIER_HYFI_ENABLE
extern int ecm_classifier_hyfi_rules_init(struct dentry *dentry);
extern void ecm_classifier_hyfi_rules_exit(void);
#endif

extern int ecm_interface_init(void);
extern void ecm_interface_exit(void);

#ifdef ECM_CLASSIFIER_DSCP_ENABLE
extern int ecm_classifier_dscp_init(struct dentry *dentry);
extern void ecm_classifier_dscp_exit(void);
#endif

#ifdef ECM_STATE_OUTPUT_ENABLE
extern int ecm_state_init(struct dentry *dentry);
extern void ecm_state_exit(void);
#endif

#ifdef ECM_CLASSIFIER_PCC_ENABLE
extern int ecm_classifier_pcc_init(struct dentry *dentry);
extern void ecm_classifier_pcc_exit(void);
#endif

#ifdef ECM_CLASSIFIER_EMESH_ENABLE
extern int ecm_classifier_emesh_sawf_init(struct dentry *dentry);
extern void ecm_classifier_emesh_sawf_exit(void);
#endif
#ifdef ECM_CLASSIFIER_MSCS_ENABLE
extern int ecm_classifier_mscs_init(struct dentry *dentry);
extern void ecm_classifier_mscs_exit(void);
#endif

/*
 * ecm_init()
 */
static int __init ecm_init(void)
{
	int ret = -1;
	struct dentry *ecm_stats_dentry;

	printk(KERN_INFO "ECM init\n");

	selected_front_end = ecm_front_end_type_select();
	if (selected_front_end == ECM_FRONT_END_TYPE_MAX) {
		DEBUG_ERROR("Front-end couldn't be selected\n");
		return -1;
	}

	if (!ecm_front_end_set_ae_precendence_array(selected_front_end)) {
		DEBUG_ERROR("Acceleration engine precedence array couldn't be set\n");
		return -1;
	}

	ecm_dentry = debugfs_create_dir("ecm", NULL);
	if (!ecm_dentry) {
		DEBUG_ERROR("Failed to create ecm directory in debugfs\n");
		return -1;
	}

	ecm_stats_dentry = debugfs_create_dir("stats", ecm_dentry);
	if (!ecm_stats_dentry) {
		DEBUG_ERROR("Failed to create stats dir in ecm\n");
		goto err_db;
	}

#ifdef ECM_FRONT_END_PPE_ENABLE
	if (ecm_front_end_ppe_fse_enable) {
		ppe_drv_fse_feature_enable();
	}
#endif
	ret = ecm_db_init(ecm_dentry);
	if (0 != ret) {
		goto err_db;
	}

	ret = ecm_classifier_default_init(ecm_dentry);
	if (0 != ret) {
		goto err_cls_default;
	}

#ifdef ECM_CLASSIFIER_NL_ENABLE
	ret = ecm_classifier_nl_rules_init(ecm_dentry);
	if (0 != ret) {
		goto err_cls_nl;
	}
#endif

#ifdef ECM_CLASSIFIER_HYFI_ENABLE
	ret = ecm_classifier_hyfi_rules_init(ecm_dentry);
	if (0 != ret) {
		goto err_cls_hyfi;
	}
#endif

#ifdef ECM_CLASSIFIER_DSCP_ENABLE
	ret = ecm_classifier_dscp_init(ecm_dentry);
	if (0 != ret) {
		goto err_cls_dscp;
	}
#endif

#ifdef ECM_CLASSIFIER_PCC_ENABLE
	ret = ecm_classifier_pcc_init(ecm_dentry);
	if (0 != ret) {
		goto err_cls_pcc;
	}
#endif
#ifdef ECM_CLASSIFIER_MARK_ENABLE
	ret = ecm_classifier_mark_init(ecm_dentry);
	if (0 != ret) {
		goto err_cls_mark;
	}
#endif

#ifdef ECM_CLASSIFIER_OVS_ENABLE
	ret = ecm_classifier_ovs_init(ecm_dentry);
	if (0 != ret) {
		goto err_cls_ovs;
	}
#endif

#ifdef ECM_CLASSIFIER_EMESH_ENABLE
	ret = ecm_classifier_emesh_sawf_init(ecm_dentry);
	if (0 != ret) {
		goto err_cls_emesh;
	}
#endif

#ifdef ECM_CLASSIFIER_MSCS_ENABLE
	ret = ecm_classifier_mscs_init(ecm_dentry);
	if (0 != ret) {
		goto err_cls_mscs;
	}
#endif

	ret = ecm_interface_init();
	if (0 != ret) {
		goto err_iface;
	}

#ifdef ECM_INTERFACE_BOND_ENABLE
	ret = ecm_front_end_bond_notifier_init(ecm_dentry);
	if (0 != ret) {
		goto err_bond;
	}
#endif

	ret = ecm_front_end_ipv4_init(ecm_dentry);
	if (0 != ret) {
		goto err_fe_ipv4;
	}

#ifdef ECM_IPV6_ENABLE
	ret = ecm_front_end_ipv6_init(ecm_dentry);
	if (0 != ret) {
		goto err_fe_ipv6;
	}
#endif

	ret = ecm_conntrack_notifier_init(ecm_dentry);
	if (0 != ret) {
		goto err_ct;
	}

#ifdef ECM_STATE_OUTPUT_ENABLE
	ret = ecm_state_init(ecm_dentry);
	if (0 != ret) {
		goto err_state;
	}
#endif
	ecm_front_end_common_sysctl_register();

	printk(KERN_INFO "ECM init complete\n");
	return 0;

#ifdef ECM_STATE_OUTPUT_ENABLE
err_state:
	ecm_conntrack_notifier_exit();
#endif
err_ct:
#ifdef ECM_IPV6_ENABLE
	ecm_front_end_ipv6_exit();
err_fe_ipv6:
#endif
	ecm_front_end_ipv4_exit();
err_fe_ipv4:
#ifdef ECM_INTERFACE_BOND_ENABLE
	ecm_front_end_bond_notifier_exit();
err_bond:
#endif
	ecm_interface_exit();
err_iface:
#ifdef ECM_CLASSIFIER_EMESH_ENABLE
	ecm_classifier_emesh_sawf_exit();
err_cls_emesh:
#endif
#ifdef ECM_CLASSIFIER_OVS_ENABLE
	ecm_classifier_ovs_exit();
err_cls_ovs:
#endif
#ifdef ECM_CLASSIFIER_MARK_ENABLE
	ecm_classifier_mark_exit();
err_cls_mark:
#endif
#ifdef ECM_CLASSIFIER_PCC_ENABLE
	ecm_classifier_pcc_exit();
err_cls_pcc:
#endif
#ifdef ECM_CLASSIFIER_DSCP_ENABLE
	ecm_classifier_dscp_exit();
err_cls_dscp:
#endif
#ifdef ECM_CLASSIFIER_HYFI_ENABLE
	ecm_classifier_hyfi_rules_exit();
err_cls_hyfi:
#endif
#ifdef ECM_CLASSIFIER_NL_ENABLE
	ecm_classifier_nl_rules_exit();
err_cls_nl:
#endif
#ifdef ECM_CLASSIFIER_MSCS_ENABLE
	ecm_classifier_mscs_exit();
err_cls_mscs:
#endif
	ecm_classifier_default_exit();
err_cls_default:
	ecm_db_exit();
err_db:
#ifdef ECM_FRONT_END_PPE_ENABLE
	if (ecm_front_end_ppe_fse_enable) {
		ppe_drv_fse_feature_disable();
	}
#endif
	debugfs_remove_recursive(ecm_dentry);

	printk(KERN_INFO "ECM init failed: %d\n", ret);
	return ret;
}

/*
 * ecm_exit()
 */
static void __exit ecm_exit(void)
{
	printk(KERN_INFO "ECM exit\n");

	/* call stop on anything that requires a prepare-to-exit signal */
	DEBUG_INFO("stop conntrack notifier\n");
	ecm_conntrack_notifier_stop(1);
	DEBUG_INFO("stop front_end_ipv4\n");
	ecm_front_end_ipv4_stop(1);
#ifdef ECM_IPV6_ENABLE
	DEBUG_INFO("stop front_end_ipv6\n");
	ecm_front_end_ipv6_stop(1);
#endif
#ifdef ECM_INTERFACE_BOND_ENABLE
	DEBUG_INFO("stop bond notifier\n");
	ecm_front_end_bond_notifier_stop(1);
#endif
	DEBUG_INFO("defunct all db connections\n");
	ecm_db_connection_defunct_all();

	/* now call exit on each module */
#ifdef ECM_STATE_OUTPUT_ENABLE
	DEBUG_INFO("stop state\n");
	ecm_state_exit();
#endif
	DEBUG_INFO("exit conntrack notifier\n");
	ecm_conntrack_notifier_exit();
	DEBUG_INFO("exit front_end_ipv4\n");
	ecm_front_end_ipv4_exit();
#ifdef ECM_IPV6_ENABLE
	DEBUG_INFO("exit front_end_ipv6\n");
	ecm_front_end_ipv6_exit();
#endif
#ifdef ECM_INTERFACE_BOND_ENABLE
	DEBUG_INFO("exit bond notifier\n");
	ecm_front_end_bond_notifier_exit();
#endif
	DEBUG_INFO("exit interface\n");
	ecm_interface_exit();

#ifdef ECM_CLASSIFIER_PCC_ENABLE
	DEBUG_INFO("exit pcc classifier\n");
	ecm_classifier_pcc_exit();
#endif
#ifdef ECM_CLASSIFIER_DSCP_ENABLE
	DEBUG_INFO("exit dscp classifier\n");
	ecm_classifier_dscp_exit();
#endif
#ifdef ECM_CLASSIFIER_HYFI_ENABLE
	DEBUG_INFO("exit hyfi classifier\n");
	ecm_classifier_hyfi_rules_exit();
#endif
#ifdef ECM_CLASSIFIER_NL_ENABLE
	DEBUG_INFO("exit nl classifier\n");
	ecm_classifier_nl_rules_exit();
#endif
#ifdef ECM_CLASSIFIER_MARK_ENABLE
	DEBUG_INFO("exit mark classifier\n");
	ecm_classifier_mark_exit();
#endif
#ifdef ECM_CLASSIFIER_OVS_ENABLE
	DEBUG_INFO("exit ovs classifier\n");
	ecm_classifier_ovs_exit();
#endif
#ifdef ECM_CLASSIFIER_EMESH_ENABLE
	DEBUG_INFO("exit emesh classifier\n");
	ecm_classifier_emesh_sawf_exit();
#endif
#ifdef ECM_CLASSIFIER_MSCS_ENABLE
	DEBUG_INFO("exit mscs classifier\n");
	ecm_classifier_mscs_exit();
#endif
	DEBUG_INFO("exit default classifier\n");
	ecm_classifier_default_exit();
	DEBUG_INFO("exit db\n");
	ecm_db_exit();

	if (ecm_dentry != NULL) {
		DEBUG_INFO("remove ecm debugfs\n");
		debugfs_remove_recursive(ecm_dentry);
	}

	ecm_front_end_common_sysctl_unregister();
#ifdef ECM_FRONT_END_PPE_ENABLE
	if (ecm_front_end_ppe_fse_enable) {
		ppe_drv_fse_feature_disable();
	}
#endif

	printk(KERN_INFO "ECM exit complete\n");
}

module_init(ecm_init)
module_exit(ecm_exit)

MODULE_DESCRIPTION("ECM Core");
MODULE_LICENSE("Dual BSD/GPL");

