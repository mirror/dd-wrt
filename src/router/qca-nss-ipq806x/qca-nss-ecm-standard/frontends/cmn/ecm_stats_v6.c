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
#include "ecm_stats_v6.h"

struct ecm_stats_v6 v6_stats;

static const char* ecm_stats_v6_exception_name_str[] = {
	"v6_front_end_stopped",
	"v6_bcast_packet_ignored",
	"v6_pptp_disabled",
	"v6_l2tpv2_disabled",
	"v6_l2tpv3_protocol",
	"v6_local_packets_ignored",
	"v6_bridge_packet_wrong_hook",
	"v6_ovs_disabled",
	"v6_bridge_front_end_stopped",
	"v6_bridge_bcast_packet_ignored",
	"v6_bridge_pptp_accel_fail",
	"v6_bridge_malformed_ip_header",
	"v6_bridge_not_ip_pppoe_packets",
	"v6_bridge_master_not_found",
	"v6_bridge_local_packets_ignored",
	"v6_bridge_port_not_found",
	"v6_bridge_hairpin_not_enabled",
	"v6_bridge_routed_packet_wrong_hook",
	"v6_bridge_dest_mac_not_found",
	"v6_bridge_mac_entry_not_found",
	"v6_bridge_pppoe_accel_disabled",
	"v6_bridge_proto_not_pppoe",
	"v6_bridge_pppoe_malformed_header",
	"v6_bridge_pppoe_mcast_accel_not_supported",
	"v6_malformed_ip_headers",
	"v6_non_ipv6_hdr",
	"v6_fragmented_packets",
	"v6_conntrack_in_dying_state",
	"v6_untracked_conntrack",
	"v6_conn_has_helper",
	"v6_unsupported_gre_protocol",
	"v6_mcast_stopped",
	"v6_mucast_not_supported",
	"v6_mcast_feature_disabled",
	"v6_dest_ip_not_ucast",
	"v6_src_ip_not_ucast",
	"v6_non_ported_not_supported",
	"v6_non_ported_disabled",
	"v6_route_in_iff_offload_disabled",
	"v6_route_out_iff_offload_disabled",
	"v6_bridge_in_iff_offload_disabled",
	"v6_bridge_out_iff_offload_disabled",
	"v6_unsupported_l2tpv3_protocol",
};

static const char* ecm_stats_v6_exception_ported_name_str[] = {
    "v6_ported_tcp_connect_not_confirm",
    "v6_ported_tcp_not_estab",
    "v6_ported_tcp_malformed_header",
    "v6_ported_tcp_denied_port",
    "v6_ported_udp_conn_not_confirm",
    "v6_ported_malformed_udp_header",
    "v6_ported_udp_denied_port",
    "v6_ported_protocol_unsupported",
    "v6_ported_ecm_in_terminating_state",
    "v6_ported_tcp_in_terminating_state",
    "v6_ported_nss_accel_not_supported",
    "v6_ported_sfe_accel_not_supported",
    "v6_ported_ppe_ds_accel_not_supported",
    "v6_ported_ppe_vp_accel_not_supported",
    "v6_ported_ppe_accel_not_supported",
    "v6_ported_ae_type_not_assigned",
    "v6_ported_unknown_ae_type",
    "v6_ported_precedence_alloc_fail",
    "v6_ported_frontend_alloc_fail",
    "v6_ported_frontend_construct_fail",
    "v6_ported_from_hierarchy_construct_fail",
    "v6_ported_from_node_fail",
    "v6_ported_from_mappping_fail",
    "v6_ported_to_hierarchy_creation_fail",
    "v6_ported_to_node_fail",
    "v6_ported_to_mapping_fail",
    "v6_ported_from_nat_hierarcy_creation_fail",
    "v6_ported_from_nat_node_fail",
    "v6_ported_from_nat_mapping_fail",
    "v6_ported_to_nat_hierarchy_creation_fail",
    "v6_ported_to_nat_node_fail",
    "v6_ported_to_nat_mapping_fail",
    "v6_ported_vlan_filter_add_fail",
    "v6_ported_default_classifier_alloc_fail",
    "v6_ported_classifier_assign_fail",
    "v6_ported_ignore_bridge_packets",
    "v6_ported_igs_accel_denied",
    "v6_ported_touch_timer_not_set",
    "v6_ported_src_nat_ip_changed",
    "v6_ported_db_conn_timer_expired",
    "v6_ported_drop_by_classifier",
};

#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
static const char* ecm_stats_v4_exception_non_ported_name_str[] = {
	"v6_non_ported_nat_accel_not_supported",
	"v6_non_ported_protocol_unsupported",
	"v6_non_ported_ecm_in_terminating_state",
	"v6_non_ported_nss_accel_not_supported",
	"v6_non_ported_ppe_ds_accel_not_supportd",
	"v6_non_ported_ppe_vp_accel_not_supported",
	"v6_non_ported_ppe_accel_not_supported",
	"v6_non_ported_ae_type_not_assigned",
	"v6_non_ported_unknown_ae_type",
	"v6_non_ported_precedence_alloc_fail",
	"v6_non_ported_front_end_alloc_fail",
	"v6_non_ported_front_end_construction_fail",
	"v6_non_ported_from_hierarchy_creation_fail",
	"v6_non_ported_from_node_fail",
	"v6_non_ported_from_mapping_fail",
	"v6_non_ported_to_hierarchy_creation_fail",
	"v6_non_ported_to_node_fail",
	"v6_non_ported_to_mapping_fail",
	"v6_non_ported_default_classifier_alloc_fail",
	"v6_non_ported_classifier_assign_fail",
	"v6_non_ported_igs_accel_denied",
	"v6_non_ported_db_conn_timer_expired",
	"v6_non_ported_drop_by_classifier",
};
#endif

/*
 * ecm_stats_v6_inc()
 * 	Generic function to increment all v6 stats.
 */
void ecm_stats_v6_inc(ecm_stats_v6_type_t stat_type, int stat_idx)
{
	switch (stat_type) {
	case ECM_STATS_V6_EXCEPTION_CMN:
		atomic64_inc(&v6_stats.ecm_stats_v6_exception_cmn[stat_idx]);
		break;
	case ECM_STATS_V6_EXCEPTION_PORTED:
		atomic64_inc(&v6_stats.ecm_stats_v6_exception_ported[stat_idx]);
		break;
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
	case ECM_STATS_V6_EXCEPTION_NON_PORTED:
		atomic64_inc(&v6_stats.ecm_stats_v6_exception_non_ported[stat_idx]);
		break;
#endif
	default:
		DEBUG_TRACE("Exception type unknown.\n");
		break;
	}
}

/*
 * ecm_stats_v6_exception_show()
 * 	Callback for the file open operation and print the output to the files.
 */
static int ecm_stats_v6_exception_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	int i = 0;

	seq_puts(m, "\nECM IPv6 EXCEPTION STATS\n\n");

	seq_puts(m, "Common stats:\n");
	for(i = 0; i < ECM_STATS_V6_EXCEPTION_CMN_MAX; i++){
		seq_printf(m, "\t\t %-70s:  %-10llu\n", ecm_stats_v6_exception_name_str[i], \
					atomic64_read(&v6_stats.ecm_stats_v6_exception_cmn[i]));
	}

	seq_puts(m, "\nPorted stats:\n");
	for(i = 0; i < ECM_STATS_V6_EXCEPTION_PORTED_MAX; i++){
		seq_printf(m, "\t\t %-70s:  %-10llu\n", ecm_stats_v6_exception_ported_name_str[i], \
					atomic64_read(&v6_stats.ecm_stats_v6_exception_ported[i]));
	}

#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
	seq_puts(m, "\nNon-ported stats:\n");
	for(i = 0; i < ECM_STATS_V6_EXCEPTION_NON_PORTED_MAX; i++){
		seq_printf(m, "\t\t %-70s:  %-10llu\n", ecm_stats_v4_exception_non_ported_name_str[i], \
					atomic64_read(&v6_stats.ecm_stats_v6_exception_non_ported[i]));
	}
#endif

	return 0;
}

/*
 * ecm_stats_v6_exception_open()
 * 	Callback for open operation for exception stats.
 */
static int ecm_stats_v6_exception_open(struct inode *inode, struct file *file)
{
    return single_open(file, ecm_stats_v6_exception_show, inode->i_private);
}

const struct file_operations ecm_stats_v6_exception_file_ops = {
    .open = ecm_stats_v6_exception_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};

/*
 * ecm_stats_debugfs_init()
 * Return -1 on failure and 0 on success
 *
 * Initilization of the stats directory and creation of the ecm_exception_stats file in the ecm directory
 */
int ecm_stats_v6_debugfs_init(struct dentry *dentry)
{
	if (!debugfs_create_file("ecm_v6_exception_stats", S_IRUGO, dentry,
				NULL, &ecm_stats_v6_exception_file_ops)) {
		return -1;
	}

	return 0;
}
