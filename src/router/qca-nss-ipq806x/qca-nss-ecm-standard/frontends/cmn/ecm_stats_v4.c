/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <net/ip.h>
#include <linux/inet.h>
#include <linux/atomic.h>
#include <linux/debugfs.h>
#include <linux/etherdevice.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_STATS_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_stats_v4.h"

/*
 * Structure variable for the v4 stats.
 */
struct ecm_stats_v4 v4_stats;

static const char* ecm_stats_v4_exception_cmn_name_str[] = {
	"v4_front_end_stopped",
	"v4_bcast_packet_ignored",
	"v4_pptp_disabled",
	"v4_l2tpv2_disabled",
	"v4_l2tpv3_protocol",
	"v4_local_packets_ignored",
	"v4_bridge_packet_wrong_hook",
	"v4_ovs_disabled",
	"v4_bridge_front_end_stopped",
	"v4_bridge_bcast_packet_ignored",
	"v4_bridge_pptp_accel_fail",
	"v4_bridge_malformed_ip_header",
	"v4_bridge_not_ip_pppoe_packet",
	"v4_bridge_master_not_found",
	"v4_bridge_local_packets_ignored",
	"v4_bridge_port_not_found",
	"v4_bridge_hairpin_not_enabled",
	"v4_bridge_routed_packet_wrong_hook",
	"v4_bridge_dest_mac_not_found",
	"v4_bridge_mac_entry_not_found",
	"v4_bridge_pppoe_accel_disabled",
	"v4_bridge_proto_not_pppoe",
	"v4_bridge_pppoe_malformed_header",
	"v4_bridge_pppoe_mcast_accel_not_supported",
	"v4_malformed_ip_headers",
	"v4_non_ipv4_header",
	"v4_fragmented_packets",
	"v4_conntrack_in_dying_state",
	"v4_untracked_conntrack",
	"v4_conn_has_helper",
	"v4_unsupported_gre_protocol",
	"v4_mcast_stopped",
	"v4_mcast_not_supported",
	"v4_mcast_feature_disabled",
	"v4_bridge_ecm_dir_mismatch",
	"v4_dest_ip_not_ucast",
	"v4_src_ip_not_ucast",
	"v4_non_ported_not_supported",
	"v4_non_ported_disabled",
	"v4_route_in_iff_offload_disabled",
	"v4_route_out_iff_offload_disabled",
	"v4_bridge_in_iff_offload_disabled",
	"v4_bridge_out_iff_offload_disabled",
	"v4_unsupported_l2tpv3_protocol",
};

static const char* ecm_stats_v4_exception_ported_name_str[] = {
	"v4_ported_tcp_not_confirm",
	"v4_ported_tcp_not_estab",
	"v4_ported_tcp_malformed_header",
	"v4_ported_tcp_denied_port",
	"v4_ported_udp_conn_not_confirm",
	"v4_ported_udp_malformed_header",
	"v4_ported_udp_denied_port",
	"v4_ported_protocol_unsupported",
	"v4_ported_ecm_in_terminating_state",
	"v4_ported_tcp_conn_in_terminating_state",
	"v4_ported_nss_accel_not_supported",
	"v4_ported_sfe_accel_not_supported",
	"v4_ported_ppe_ds_accel_not_supported",
	"v4_ported_ppe_vp_accel_not_supported",
	"v4_ported_ppe_accel_not_supported",
	"v4_ported_ae_type_not_assigned",
	"v4_ported_unknown_ae_type",
	"v4_ported_precedence_alloc_fail",
	"v4_ported_frontend_alloc_fail",
	"v4_ported_frontend_construct_fail",
	"v4_ported_from_hierarchy_creation_fail",
	"v4_ported_from_node_fail",
	"v4_ported_from_mapping_fail",
	"v4_ported_to_hierarchy_creation_fail",
	"v4_ported_to_node_fail",
	"v4_ported_to_mapping_fail",
	"v4_ported_from_nat_hierarchy_creation_fail",
	"v4_ported_from_nat_node_fail",
	"v4_ported_from_nat_mapping_fail",
	"v4_ported_to_nat_hierarchy_creation_fail",
	"v4_ported_to_nat_node_fail",
	"v4_ported_to_nat_mapping_fail",
	"v4_ported_vlan_filter_add_fail",
	"v4_ported_default_classifier_alloc_fail",
	"v4_ported_classifier_assign_fail",
	"v4_ported_ignore_bridge_packets",
	"v4_ported_igs_accel_denied",
	"v4_ported_nat_src_ip_changed",
	"v4_ported_touch_timer_not_set",
	"v4_ported_db_conn_timer_expired",
	"v4_ported_drop_by_classifier",
};

#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
static const char* ecm_stats_v4_exception_non_ported_name_str[] = {
	"v4_non_ported_protocol_unsupported",
	"v4_non_ported_ecm_in_terminating_state",
	"v4_non_ported_nss_accel_not_supported",
	"v4_non_ported_ppe_ds_accel_not_supportd",
	"v4_non_ported_ppe_vp_accel_not_supported",
	"v4_non_ported_ppe_accel_not_supported",
	"v4_non_ported_ae_not_assigned",
	"v4_non_ported_unknown_ae_type",
	"v4_non_ported_precedence_alloc_fail",
	"v4_non_ported_front_end_alloc_fail",
	"v4_non_ported_front_end_construction_fail",
	"v4_non_ported_from_hierarchy_creation_fail",
	"v4_non_ported_from_node_fail",
	"v4_non_ported_from_mapping_fail",
	"v4_non_ported_to_hierarchy_creation_fail",
	"v4_non_ported_to_node_fail",
	"v4_non_ported_to_mapping_fail",
	"v4_non_ported_from_nat_hierarchy_creation_fail",
	"v4_non_ported_from_nat_node_fail",
	"v4_non_ported_from_nat_mapping_fail",
	"v4_non_ported_to_nat_hierarchy_creation_fail",
	"v4_non_ported_to_nat_node_fail",
	"v4_non_ported_to_nat_mapping_fail",
	"v4_non_ported_default_classifier_alloc_fail",
	"v4_non_ported_classifier_assign_fail",
	"v4_non_ported_igs_accel_denied",
	"v4_non_ported_db_conn_timer_expired",
	"v4_non_ported_drop_by_classifier",
};
#endif

/*
 * ecm_stats_v4_inc()
 * 	Generic function to increment all ipv4 stats.
 */
void ecm_stats_v4_inc(ecm_stats_v4_type_t stat_type, int stat_idx)
{
	switch (stat_type) {
	case ECM_STATS_V4_EXCEPTION_CMN:
		atomic64_inc(&v4_stats.ecm_stats_v4_exeception_cmn[stat_idx]);
		break;
	case ECM_STATS_V4_EXCEPTION_PORTED:
		atomic64_inc(&v4_stats.ecm_stats_v4_exception_ported[stat_idx]);
		break;
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
	case ECM_STATS_V4_EXCEPTION_NON_PORTED:
		atomic64_inc(&v4_stats.ecm_stats_v4_exception_non_ported[stat_idx]);
		break;
#endif
	default:
		DEBUG_TRACE("Exception type unknown.\n");
		break;
	}
}

/*
 * ecm_stats_v4_exception_show()
 * 	Callback for the file open operation and print the output to the files.
 */
static int ecm_stats_v4_exception_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	int i = 0;

	seq_puts(m, "\nECM IPv4 EXCEPTION STATS\n\n");

	seq_puts(m, "Common Stats:\n\n");
	for(i = 0; i < ECM_STATS_V4_EXCEPTION_CMN_MAX; i++){
		seq_printf(m, "\t\t %-70s:  %-10llu\n", ecm_stats_v4_exception_cmn_name_str[i], \
					atomic64_read(&v4_stats.ecm_stats_v4_exeception_cmn[i]));
	}

	seq_puts(m, "\nPorted Stats:\n\n");
	for(i = 0; i < ECM_STATS_V4_EXCEPTION_PORTED_MAX; i++){
		seq_printf(m, "\t\t %-70s:  %-10llu\n", ecm_stats_v4_exception_ported_name_str[i], \
					atomic64_read(&v4_stats.ecm_stats_v4_exception_ported[i]));
	}

#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
	seq_puts(m, "\nNon-Ported Stats:\n\n");
	for(i = 0; i < ECM_STATS_V4_EXCEPTION_NON_PORTED_MAX; i++){
		seq_printf(m, "\t\t %-70s:  %-10llu\n", ecm_stats_v4_exception_non_ported_name_str[i], \
					atomic64_read(&v4_stats.ecm_stats_v4_exception_non_ported[i]));
	}
#endif

	return 0;
}

/*
 * ecm_stats_v4_exception_open()
 * 	Callback for open operation for exception stats.
 */
static int ecm_stats_v4_exception_open(struct inode *inode, struct file *file)
{
	return single_open(file, ecm_stats_v4_exception_show, inode->i_private);
}

const struct file_operations ecm_stats_v4_exception_file_ops = {
	.open = ecm_stats_v4_exception_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ecm_stats_debugfs_init()
 * Returns -1 on failure and 0 on success
 *
 * Initilization of the stats directory and creation of the ecm_exception_stats file in the ecm directory
 */
int ecm_stats_v4_debugfs_init(struct dentry *dentry)
{
	if (!debugfs_create_file("ecm_v4_exception_stats", S_IRUGO, dentry,
				NULL, &ecm_stats_v4_exception_file_ops)) {
		return -1;
	}

	return 0;
}
