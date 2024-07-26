/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/atomic.h>
#include <linux/debugfs.h>
#include "ppe_drv.h"

/*
 * ppe_drv_stats_sc_name_str
 */
static const char *ppe_drv_stats_sc_name_str[] = {
	"PPE_DRV_SC_NONE",                /* Normal PPE processing */
	"PPE_DRV_SC_BYPASS_ALL",          /* Bypasses all stages in PPE */
	"PPE_DRV_SC_ADV_QOS_BRIDGED",     /* Adv QoS redirection for bridged flow */
	"PPE_DRV_SC_LOOPBACK_QOS",        /* Bridge or IGS QoS redirection */
	"PPE_DRV_SC_BNC_0",               /* QoS bounce */
	"PPE_DRV_SC_BNC_CMPL_0",          /* QoS bounce complete */
	"PPE_DRV_SC_ADV_QOS_ROUTED",      /* Adv QoS redirection for routed flow */
	"PPE_DRV_SC_IPSEC_PPE2EIP",       /* Inline IPsec redirection from PPE TO EIP */
	"PPE_DRV_SC_IPSEC_EIP2PPE",       /* Inline IPsec redirection from EIP to PPE */
	"PPE_DRV_SC_PTP",                 /* Service Code for PTP packets */
	"PPE_DRV_SC_VLAN_FILTER_BYPASS",  /* VLAN filter bypass for bridge flows between 2 different VSIs */
	"PPE_DRV_SC_L3_EXCEPT",           /* Indicate exception post tunnel/tap operation */
	"PPE_DRV_SC_SPF_BYPASS",          /* Source port filtering bypass */
	"PPE_DRV_SC_NOEDIT_REDIR_CORE0",	/* PPE RFS service code for core0 for Passive VP */
	"PPE_DRV_SC_NOEDIT_REDIR_CORE1",	/* PPE RFS service code for core1 for Passive VP */
	"PPE_DRV_SC_NOEDIT_REDIR_CORE2",	/* PPE RFS service code for core2 for Passive VP */
	"PPE_DRV_SC_NOEDIT_REDIR_CORE3",	/* PPE RFS service code for core3 for Passive VP */
	"PPE_DRV_SC_EDIT_REDIR_CORE0",	  /* PPE RFS service code for core0 for Active VP */
	"PPE_DRV_SC_EDIT_REDIR_CORE1",	  /* PPE RFS service code for core1 for Active VP */
	"PPE_DRV_SC_EDIT_REDIR_CORE2",	  /* PPE RFS service code for core2 for Active VP */
	"PPE_DRV_SC_EDIT_REDIR_CORE3",	  /* PPE RFS service code for core3 for Active VP */
	"PPE_DRV_SC_VP_RPS",			/* PPE RPS service code for VP flow */
	"PPE_DRV_SC_NOEDIT_ACL_POLICER",	/* PPE policer service code for noedit */
	"PPE_DRV_SC_L2_TUNNEL_EXCEPTION",  	/* Service code to allow decapsulated VXLAN/GRE tunnel exception. */
	"PPE_DRV_SC_NOEDIT_PRIORITY_SET",	/* PPE Priority service code */
	"PPE_DRV_SC_NOEDIT_RULE",		/* PPE noedit rule service code */
	"PPE_DRV_SC_MAX",                 /* Max service code */
};

/*
 * ppe_drv_stats_conn_str
 * 	PPE DRV connection statistics
 */
static const char *ppe_drv_stats_conn_str[] = {
	"v4_l3_flows",				/* No of v4 routed flows */
	"v4_l2_flows",				/* No of v4 bridge flows */
	"v4_vp_wifi_flows",			/* No of v4 VP Wi-Fi flows */
	"v4_ds_flows",				/* No of v4 Direct Switch flows */
	"v4_host_add_fail",			/* v4 host table add failed */

	"v4_flush_req",				/* No of v4 flush requests */
	"v4_flush_fail",			/* No of v4 flush requests fail */
	"v4_flush_conn_not_found",		/* No of v4 connection not found during flush. */
	"v4_flush_skip_conn_rfs",		/* No of v4 rfs connection not found during flush. */

	"v6_l3_flows",				/* No of v6 routed flows */
	"v6_l2_flows",				/* No of v6 bridge flows */
	"v6_vp_wifi_flows",			/* No of v6 VP Wi-Fi flows */
	"v6_ds_flows",				/* No of v6 Direct Switch flows */
	"v6_host_add_fail",			/* v6 host table add failed */
	"v6_create_fail_bridge_nat",		/* No of v6 create failure due to NAT with bridge flow */

	"v6_flush_req",				/* No of v6 flush requests */
	"v6_flush_fail",			/* No of v6 flush requests fail */
	"v6_flush_conn_not_found",		/* No of v6 connection not found during flush. */
	"v6_flush_skip_conn_rfs",		/* No of v6 rfs connection not found during flush. */

	"fail_vp_full",					/* Create req fail due to virtual port table full */
	"fail_pp_full",					/* create req fail due to physical port table full */
	"fail_nh_full",					/* Create req fail due to nexthop table full */
	"fail_flow_full",				/* Create req fail due to flow table full */
	"fail_host_full",				/* Create req fail due to host table full */
	"fail_pubip_full",				/* Create req fail due to pub-ip table full */
	"fail_dev_port_map",				/* Create req fail due to PPE port not mapped to net-device */
	"fail_l3_if_full",				/* Create req fail due to L3_IF table full */
	"fail_vsi_full",				/* Create req fail due to VSI table full */
	"fail_vsi_reuse",				/* Create req fail due to VSI reuse */
	"fail_pppoe_full",				/* Create req fail due to PPPoE table full */
	"fail_rw_fifo_full",				/* Create req fail due to read-write fifo full */
	"fail_flow_command",				/* Create req fail due to PPE flow command failure */
	"fail_unknown_proto",				/* Create req fail due to unknown protocol */
	"fail_query_unknown_proto",			/* Query fail due to unknown protocol */
	"fail_ppe_unresponsive",			/* Fail due to PPE not responding */
	"ce_opaque_invalid",				/* Fail due to invalid opaque in CE */
	"fail_fqg_full",				/* Req fail due to flow qos group full */
	"fail_ingress_vlan_add",			/* Ingress VLAN add rule failed */
	"fail_egress_vlan_add",				/* Egress VLAN add rule failed */
	"fail_my_mac_full",				/* Fail due to MY_MAC table full */
	"fail_ingress_untag_vlan_add",			/* Ingress VLAN add rule failed */
	"fail_ingress_untag_vlan_del",			/* Ingress VLAN del rule failed */
	"fail_ingress_vlan_over_bridge_add",		/* Ingress VLAN over bridge add rule failed */
	"fail_ingress_vlan_over_bridge_del",		/* Ingress VLAN over bridge delete rule failed */
};

/*
 * ppe_drv_comm_stats_flow_conn_str
 *	PPE DRV common flow connection statistics
 */
static const char * const ppe_drv_comm_stats_flow_conn_str[] = {
	"v4_create_req",			/* No of v4 create requests */
	"v4_create_fail",			/* No of v4 create failure */
	"v4_destroy_req",			/* No of v4 delete requests */
	"v4_destroy_fail",			/* No of v4 delete failure */
	"v4_create_fail_bridge_nat",		/* No of v4 create failure due to NAT with bridge flow */
	"v4_create_fail_snat_dnat",		/* No of v4 create failure due to both SNAT and DNAT is requested */
	"v4_destroy_conn_not_found",		/* No of v4 delete failure due to connection not found */
	"v4_create_fail_mem",			/* No of v4 create failure due to OOM */
	"v4_create_fail_conn",			/* No of v4 create failure due to invalid parameters */
	"v4_create_fail_collision",		/* No of v4 create failure due to connection already exist */
	"v4_unknown_interface",			/* No of v4 create failure due to invalid IF */
	"v4_create_fail_invalid_rx_if",		/* No of v4 create failure due to invalid Rx IF */
	"v4_create_fail_invalid_tx_if",		/* No of v4 create failure due to invalid Tx IF */
	"v4_create_fail_invalid_rx_port",	/* No of v4 create failure due to invalid Rx Port */
	"v4_create_fail_invalid_tx_port",	/* No of v4 create failure due to invalid Tx Port */
	"v4_create_fail_if_hierarchy",		/* No of v4 create failure due to interface hierarchy walk fail */

	"v4_create_fail_vlan_filter",		/* No of v4 create failure due to interface not in bridge */
	"v4_create_fail_bridge_noexist",	/* No of v4 create failure due to bridge interface not created */
	"v4_stats_conn_not_found",		/* No of v4 stats sync failure due to connection not found */

	"v4_create_rfs_req",                    /* No of v4 RFS create requests */
	"v4_create_rfs_fail",                   /* No of v4 RFS create failure */
	"v4_destroy_rfs_req",                   /* No of v4 RFS delete requests */
	"v4_destroy_rfs_fail",                  /* No of v4 RFS delete failure */
	"v4_destroy_rfs_conn_not_found",                /* No of v4 RFS delete failure due to connection not found */
	"v4_create_rfs_fail_mem",                       /* No of v4 RFS create failure due to OOM */
	"v4_create_rfs_fail_conn",                      /* No of v4 RFS create failure due to invalid parameters */
	"v4_create_rfs_fail_collision",         /* No of v4 RFS create failure due to connection already exist */
	"v4_unknown_rfs_interface",                     /* No of v4 RFS create failure due to invalid IF */
	"v4_create_rfs_fail_invalid_rx_if",             /* No of v4 RFS create failure due to invalid Rx IF */
	"v4_create_rfs_fail_invalid_tx_if",             /* No of v4 RFS create failure due to invalid Tx IF */
	"v4_create_rfs_fail_invalid_rx_port",   /* No of v4 RFS create failure due to invalid Rx Port */
	"v4_create_rfs_fail_invalid_tx_port",   /* No of v4 RFS create failure due to invalid Tx Port */
	"v4_create_rfs_noedit_rule",    /* No of v4 rfs non edit rule create */

	"v4_assist_rule_create_req",		/* No of v4 assist rule create requests */
	"v4_assist_rule_create_fail_mem",	/* No of v4 assist rule create failure due to OOM */
	"v4_assist_rule_create_fail_collision", /* No of v4 assist rule create failure due to connection already exist */
	"v4_assist_rule_create_fail",		/* No of v4 assist rule create failure */
	"v4_assist_rule_destroy_req",		/* No of v4 Assist rule delete requests */
	"v4_assist_rule_destroy_conn_not_found",	/* No of v6 Assist rule delete failure due to connection not found */
	"v4_assist_rule_destroy_fail",			/* No of v6 Assist rule  delete failure*/
	"v4_assist_rule_create_rfs_fail_conn",	/* no of v4 assist rule create rfs failure */
	"v4_assist_rule_create_priority_fail_conn",	/* no of v4 assist rule create priority failure */
	"v4_create_priority_req",		/* no of v4 priority assist create requests */

	"v4_create_policer_req",			/* No of v4 Policer create requests */
	"v4_create_policer_fail",			/* No of v4 Policer create failure */
	"v4_create_policer_fail_acl",			/* No of v4 Policer create failure due to bind issue */
	"v4_destroy_policer_req",			/* No of v4 Policer delete requests */
	"v4_destroy_policer_fail",			/* No of v4 Policer delete failure */
	"v4_destroy_policer_fail_acl",			/* No of v4 Policer delete failure due to unbind issue */
	"v4_destroy_policer_conn_not_found",		/* No of v4 Policer delete failure due to connection not found */
	"v4_create_policer_fail_mem",			/* No of v4 Policer create failure due to OOM */
	"v4_create_policer_fail_conn",			/* No of v4 Policer create failure due to invalid parameters */
	"v4_create_policer_fail_collision",		/* No of v4 Policer create failure due to connection already exist */
	"v4_unknown_policer_interface",			/* No of v4 Policer create failure due to invalid IF */
	"v4_create_policer_noedit_rule",		/* No of v4 Policer non edit rule create */
	"v4_create_policer_fail_invalid_rx_if",		/* No of v4 Policer create failure due to invalid Rx IF */
	"v4_create_policer_fail_invalid_tx_if",		/* No of v4 Policer create failure due to invalid Tx IF */
	"v4_create_policer_fail_invalid_rx_port",	/* No of v4 Policer create failure due to invalid Rx Port */
	"v4_create_policer_fail_invalid_tx_port",	/* No of v4 Policer create failure due to invalid Tx Port */

	"v4_create_fse_success",			/* No of v4 Wi-Fi FSE rule create failure */
	"v4_create_fse_fail",			/* No of v4 Wi-Fi FSE rule create failure */
	"v4_destroy_fse_success",			/* No of v4 Wi-Fi FSE rule delete failure */
	"v4_destroy_fse_fail",			/* No of v4 Wi-Fi FSE rule delete failure */
	"v4_create_offload_disabled",		/* No of v4 rules where offload is disabled */
	"v4_create_fail_offload_disabled",	/* No of v4 create failure due to offload disable */

	"v4_create_fail_acl",		/* No of v4 create failure due to ACL linking */
	"v4_destroy_fail_acl",		/* No of v4 delete failure due to ACL unlinking */

	"v6_create_req",			/* No of v6 create requests */
	"v6_create_fail",			/* No of v6 create failure */
	"v6_destroy_req",			/* No of v6 delete requests */
	"v6_destroy_fail",			/* No of v6 delete failure */
	"v6_destroy_conn_not_found",		/* No of v4 delete failure due to connection not found */
	"v6_create_fail_mem",			/* No of v6 create failure due to OOM */
	"v6_create_fail_conn",			/* No of v6 create failure due to invalid parameters */
	"v6_create_fail_collision",		/* No of v6 create failure due to connection already exist */
	"v6_create_fail_invalid_rx_if",		/* No of v6 create failure due to invalid Rx IF */
	"v6_create_fail_invalid_tx_if",		/* No of v6 create failure due to invalid Tx IF */
	"v6_create_fail_invalid_rx_port",	/* No of v6 create failure due to invalid Rx Port */
	"v6_create_fail_invalid_tx_port",	/* No of v6 create failure due to invalid Tx Port */
	"v6_create_fail_if_hierarchy",		/* No of v6 create failure due to interface hierarchy walk fail */

	"v6_create_fail_vlan_filter",		/* No of v6 create failure due to interface not in bridge */
	"v6_create_fail_bridge_noexist",	/* No of v6 create failure due to bridge interface not created */
	"v6_stats_conn_not_found",		/* No of v6 stats sync failure due to connection not found */

	"v6_create_rfs_req",                    /* No of v6 RFS create requests */
	"v6_create_rfs_fail",                   /* No of v6 RFS create failure */
	"v6_destroy_rfs_req",                   /* No of v6 RFS delete requests */
	"v6_destroy_rfs_fail",                  /* No of v6 RFS delete failure */
	"v6_destroy_rfs_conn_not_found",                /* No of v6 RFS delete failure due to connection not found */
	"v6_create_rfs_fail_mem",                       /* No of v6 RFS create failure due to OOM */
	"v6_create_rfs_fail_conn",                      /* No of v6 RFS create failure due to invalid parameters */
	"v6_create_rfs_fail_collision",         /* No of v6 RFS create failure due to connection already exist */
	"v6_unknown_rfs_interface",                     /* No of v6 RFS create failure due to invalid IF */
	"v6_create_rfs_fail_invalid_rx_if",             /* No of v6 RFS create failure due to invalid Rx IF */
	"v6_create_rfs_fail_invalid_tx_if",             /* No of v6 RFS create failure due to invalid Tx IF */
	"v6_create_rfs_fail_invalid_rx_port",   /* No of v6 RFS create failure due to invalid Rx Port */
	"v6_create_rfs_fail_invalid_tx_port",   /* No of v6 RFS create failure due to invalid Tx Port */
	"v6_create_rfs_noedit_rule",    /* No of v6 rfs non edit rule create */

	"v6_assist_rule_create_req",		/* No of v6 assist rule create requests */
	"v6_assist_rule_create_fail_mem",	/* No of v6 assist rule create failure due to OOM */
	"v6_assist_rule_create_fail_collision", /* No of v6 assist rule create failure due to connection already exist */
	"v6_assist_rule_create_fail",		/* No of v6 assist rule create failure */
	"v6_assist_rule_destroy_req",		/* No of v6 Assist rule delete requests */
	"v6_assist_rule_destroy_conn_not_found",	/* No of v6 Assist rule delete failure due to connection not found */
	"v6_assist_rule_destroy_fail",			/* No of v6 Assist rule  delete failure*/
	"v6_assist_rule_create_rfs_fail_conn",	/* No of v6 assist rule create rfs failure */
	"v6_assist_rule_create_priority_fail_conn",	/* No of v6 assist rule create priority failure */
	"v6_create_priority_req",		/* No of v6 Priority Assist create requests */

	"v6_create_policer_req",			/* No of v6 Policer create requests */
	"v6_create_policer_fail",			/* No of v6 Policer create failure */
	"v6_create_policer_fail_acl",			/* No of v6 Policer create failure due to bind issue */
	"v6_destroy_policer_req",			/* No of v6 Policer delete requests */
	"v6_destroy_policer_fail",			/* No of v6 Policer delete failure */
	"v6_destroy_policer_fail_acl",			/* No of v6 Policer delete failure due to unbind issue */
	"v6_destroy_policer_conn_not_found",		/* No of v6 Policer delete failure due to connection not found */
	"v6_create_policer_fail_mem",			/* No of v6 Policer create failure due to OOM */
	"v6_create_policer_fail_conn",			/* No of v6 Policer create failure due to invalid parameters */
	"v6_create_policer_fail_collision",		/* No of v6 Policer create failure due to connection already exist */
	"v6_unknown_policer_interface",			/* No of v6 Policer create failure due to invalid IF */
	"v6_create_policer_noedit_rule",		/* No of v6 Policer non edit rule create */
	"v6_create_policer_fail_invalid_rx_if",		/* No of v6 Policer create failure due to invalid Rx IF */
	"v6_create_policer_fail_invalid_tx_if",		/* No of v6 Policer create failure due to invalid Tx IF */
	"v6_create_policer_fail_invalid_rx_port",	/* No of v6 Policer create failure due to invalid Rx Port */
	"v6_create_policer_fail_invalid_tx_port",	/* No of v6 Policer create failure due to invalid Tx Port */

	"v6_create_fse_success",			/* No of v4 Wi-Fi FSE rule create failure */
	"v6_create_fse_fail",			/* No of v6 Wi-Fi FSE rule create failure */
	"v6_destroy_fse_success",			/* No of v6 Wi-Fi FSE rule delete failure */
	"v6_destroy_fse_fail",			/* No of v6 Wi-Fi FSE rule delete failure */
	"v6_create_offload_disabled",		/* No of v6 rules where offload is disabled */
	"v6_create_fail_acl",		/* No of v6 create failure due to ACL linking */
	"v6_destroy_fail_acl",		/* No of v6 delete failure due to ACL unlinking */

	"v6_create_fail_offload_disabled",	/* No of v6 create failure due to offload disable */
};

/*
 * ppe_drv_comm_stats_tun_conn_str
 *	PPE DRV common tun connection statistics
 */
static const char * const ppe_drv_comm_stats_tun_conn_str[] = {
	"v4_tun_create_req",			/* No of v4 create requests */
	"v4_tun_create_fail",			/* No of v4 create failure */
	"v4_tun_destroy_req",			/* No of v4 delete requests */
	"v4_tun_destroy_fail",			/* No of v4 delete failure */
	"v4_tun_create_fail_bridge_nat",	/* No of v4 create failure due to NAT with bridge flow */
	"v4_tun_create_fail_snat_dnat",		/* No of v4 create failure due to both SNAT and DNAT is requested */
	"v4_tun_destroy_conn_not_found",	/* No of v4 delete failure due to connection not found */
	"v4_tun_create_fail_mem",		/* No of v4 create failure due to OOM */
	"v4_tun_create_fail_conn",		/* No of v4 create failure due to invalid parameters */
	"v4_tun_create_fail_collision",		/* No of v4 create failure due to connection already exist */
	"v4_tun_unknown_interface",		/* No of v4 create failure due to invalid IF */
	"v4_tun_create_fail_invalid_rx_if",	/* No of v4 create failure due to invalid Rx IF */
	"v4_tun_create_fail_invalid_tx_if",	/* No of v4 create failure due to invalid Tx IF */
	"v4_tun_create_fail_invalid_rx_port",	/* No of v4 create failure due to invalid Rx Port */
	"v4_tun_create_fail_invalid_tx_port",	/* No of v4 create failure due to invalid Tx Port */
	"v4_tun_create_fail_if_hierarchy",	/* No of v4 create failure due to interface hierarchy walk fail */

	"v4_tun_create_fail_vlan_filter",	/* No of v4 create failure due to interface not in bridge */
	"v4_tun_create_fail_bridge_noexist",	/* No of v4 create failure due to bridge interface not created */
	"v4_tun_stats_conn_not_found",          /* No of v4 stats sync failure due to connection not found */

	"v4_tun_create_rfs_req",                        /* No of v4 RFS create requests */
	"v4_tun_create_rfs_fail",                       /* No of v4 RFS create failure */
	"v4_tun_destroy_rfs_req",                       /* No of v4 RFS delete requests */
	"v4_tun_destroy_rfs_fail",                      /* No of v4 RFS delete failure */
	"v4_tun_destroy_rfs_conn_not_found",            /* No of v4 RFS delete failure due to connection not found */
	"v4_tun_create_rfs_fail_mem",                   /* No of v4 RFS create failure due to OOM */
	"v4_tun_create_rfs_fail_conn",                  /* No of v4 RFS create failure due to invalid parameters */
	"v4_tun_create_rfs_fail_collision",             /* No of v4 RFS create failure due to connection already exist */
	"v4_tun_unknown_rfs_interface",                 /* No of v4 RFS create failure due to invalid IF */
	"v4_tun_create_rfs_fail_invalid_rx_if",         /* No of v4 RFS create failure due to invalid Rx IF */
	"v4_tun_create_rfs_fail_invalid_tx_if",         /* No of v4 RFS create failure due to invalid Tx IF */
	"v4_tun_create_rfs_fail_invalid_rx_port",       /* No of v4 RFS create failure due to invalid Rx Port */
	"v4_tun_create_rfs_fail_invalid_tx_port",       /* No of v4 RFS create failure due to invalid Tx Port */

	"v4_tun_assist_rule_create_req",		/* no of v4 assist rule create requests */
	"v4_tun_assist_rule_create_fail_mem",		/* no of v4 assist rule create failure due to oom */
	"v4_tun_assist_rule_create_fail_collision", 	/* no of v4 assist rule create failure due to connection already exist */
	"v4_tun_assist_rule_create_fail",		/* no of v4 assist rule create failure */
	"v4_tun_assist_rule_destroy_req",		/* no of v4 assist rule delete requests */
	"v4_tun_assist_rule_destroy_conn_not_found",	/* no of v4 assist rule delete failure due to connection not found */
	"v4_tun_assist_rule_destroy_fail",		/* no of v4 assist rule  delete failure*/
	"v4_tun_assist_rule_create_priority_fail_conn",	/* no of v4 assist rule create priority failure */
	"v4_tun_assist_rule_create_rfs_fail_conn",	/* no of v4 assist rule create failure */
	"v4_tun_create_priority_req",			/* No of v4 Priority create requests */
	"v4_tun_create_priority_fail_conn",		/* No of v4 Priority failure due to connection not found */


	"v4_tun_create_policer_req",			/* No of v4 Policer create requests */
	"v4_tun_create_policer_fail",			/* No of v4 Policer create failure */
	"v4_tun_create_policer_fail_acl",		/* No of v4 Policer create failure due to ACL bind */
	"v4_tun_destroy_policer_req",			/* No of v4 Policer delete requests */
	"v4_tun_destroy_policer_fail",			/* No of v4 Policer delete failure */
	"v4_tun_destroy_policer_fail_acl",			/* No of v4 Policer delete failure due to unbind issue */
	"v4_tun_destroy_policer_conn_not_found",		/* No of v4 Policer delete failure due to connection not found */
	"v4_tun_create_policer_fail_mem",			/* No of v4 Policer create failure due to OOM */
	"v4_tun_create_policer_fail_conn",			/* No of v4 Policer create failure due to invalid parameters */
	"v4_tun_create_policer_fail_collision",		/* No of v4 Policer create failure due to connection already exist */
	"v4_tun_unknown_policer_interface",			/* No of v4 Policer create failure due to invalid IF */
	"v4_tun_create_policer_noedit_rule",		/* No of v4 Policer non edit rule create */
	"v4_tun_create_policer_fail_invalid_rx_if",		/* No of v4 Policer create failure due to invalid Rx IF */
	"v4_tun_create_policer_fail_invalid_tx_if",		/* No of v4 Policer create failure due to invalid Tx IF */
	"v4_tun_create_policer_fail_invalid_rx_port",	/* No of v4 Policer create failure due to invalid Rx Port */
	"v4_tun_create_policer_fail_invalid_tx_port",	/* No of v4 Policer create failure due to invalid Tx Port */

	"v4_tun_create_fse_success",			/* No of v6 Wi-Fi FSE rule create failure */
	"v4_tun_create_fse_fail",			/* No of v6 Wi-Fi FSE rule create failure */
	"v4_tun_destroy_fse_success",			/* No of v6 Wi-Fi FSE rule delete failure */
	"v4_tun_destroy_fse_fail",			/* No of v6 Wi-Fi FSE rule delete failure */
	"v4_create_offload_disabled",		/* No of v4 rules where offload is disabled */
	"v4_create_fail_offload_disabled",	/* No of v4 create failure due to offload disable */

	"v4_tun_create_fail_acl",		/* No of v4 create failure due to ACL linking */
	"v4_tun_destroy_fail_acl",		/* No of v4 delete failure due to ACL unlinking */

	"v6_tun_create_req",			/* No of v6 create requests */
	"v6_tun_create_fail",			/* No of v6 create failure */
	"v6_tun_destroy_req",			/* No of v6 delete requests */
	"v6_tun_destroy_fail",			/* No of v6 delete failure */
	"v6_tun_destroy_conn_not_found",	/* No of v4 delete failure due to connection not found */
	"v6_tun_create_fail_mem",		/* No of v6 create failure due to OOM */
	"v6_tun_create_fail_conn",		/* No of v6 create failure due to invalid parameters */
	"v6_tun_create_fail_collision",		/* No of v6 create failure due to connection already exist */
	"v6_tun_create_fail_invalid_rx_if",	/* No of v6 create failure due to invalid Rx IF */
	"v6_tun_create_fail_invalid_tx_if",	/* No of v6 create failure due to invalid Tx IF */
	"v6_tun_create_fail_invalid_rx_port",	/* No of v6 create failure due to invalid Rx Port */
	"v6_tun_create_fail_invalid_tx_port",	/* No of v6 create failure due to invalid Tx Port */
	"v6_tun_create_fail_if_hierarchy",	/* No of v6 create failure due to interface hierarchy walk fail */

	"v6_tun_create_fail_vlan_filter",	/* No of v6 create failure due to interface not in bridge */
	"v6_tun_create_fail_bridge_noexist",	/* No of v6 create failure due to bridge interface not created */
	"v6_tun_stats_conn_not_found",		/* No of v6 stats sync failure due to connection not found */

	"v6_tun_create_rfs_req",                        /* No of v6 RFS create requests */
	"v6_tun_create_rfs_fail",                       /* No of v6 RFS create failure */
	"v6_tun_destroy_rfs_req",                       /* No of v6 RFS delete requests */
	"v6_tun_destroy_rfs_fail",                      /* No of v6 RFS delete failure */
	"v6_tun_destroy_rfs_conn_not_found",            /* No of v6 RFS delete failure due to connection not found */
	"v6_tun_create_rfs_fail_mem",                   /* No of v6 RFS create failure due to OOM */
	"v6_tun_create_rfs_fail_conn",                  /* No of v6 RFS create failure due to invalid parameters */
	"v6_tun_create_rfs_fail_collision",             /* No of v6 RFS create failure due to connection already exist */
	"v6_tun_unknown_rfs_interface",                 /* No of v6 RFS create failure due to invalid IF */
	"v6_tun_create_rfs_fail_invalid_rx_if",         /* No of v6 RFS create failure due to invalid Rx IF */
	"v6_tun_create_rfs_fail_invalid_tx_if",         /* No of v6 RFS create failure due to invalid Tx IF */
	"v6_tun_create_rfs_fail_invalid_rx_port",       /* No of v6 RFS create failure due to invalid Rx Port */
	"v6_tun_create_rfs_fail_invalid_tx_port",       /* No of v6 RFS create failure due to invalid Tx Port */

	"v6_tun_assist_rule_create_req",		/* no of v6 assist rule create requests */
	"v6_tun_assist_rule_create_fail_mem",		/* no of v6 assist rule create failure due to oom */
	"v6_tun_assist_rule_create_fail_collision", 	/* no of v6 assist rule create failure due to connection already exist */
	"v6_tun_assist_rule_create_fail",		/* no of v6 assist rule create failure */
	"v6_tun_assist_rule_destroy_req",		/* no of v6 assist rule delete requests */
	"v6_tun_assist_rule_destroy_conn_not_found",	/* no of v6 assist rule delete failure due to connection not found */
	"v6_tun_assist_rule_destroy_fail",		/* no of v6 assist rule  delete failure*/
	"v6_tun_assist_rule_create_priority_fail_conn",	/* no of v6 assist rule create priority failure */
	"v6_tun_assist_rule_create_rfs_fail_conn",	/* no of v6 assist rule create failure */
	"v6_tun_create_priority_req",			/* No of v6 Priority create requests */
	"v6_tun_create_priority_fail_conn",		/* No of v6 Priority failure due to connection not found */

	"v6_tun_create_fse_success",			/* No of v6 Wi-Fi FSE rule create failure */
	"v6_tun_create_fse_fail",			/* No of v6 Wi-Fi FSE rule create failure */
	"v6_tun_destroy_fse_success",			/* No of v6 Wi-Fi FSE rule delete failure */
	"v6_tun_destroy_fse_fail",			/* No of v6 Wi-Fi FSE rule delete failure */
	"v6_create_offload_disabled",		/* No of v6 rules where offload is disabled */

	"v6_tun_create_policer_req",			/* No of v6 Policer create requests */
	"v6_tun_create_policer_fail",			/* No of v6 Policer create failure */
	"v6_tun_create_policer_fail_acl",		/* No of v6 Policer create failure due to acl bind issue */
	"v6_tun_destroy_policer_req",			/* No of v6 Policer delete requests */
	"v6_tun_destroy_policer_fail",			/* No of v6 Policer delete failure */
	"v6_tun_destroy_policer_fail_acl",			/* No of v6 Policer delete failure due to unbind issue */
	"v6_tun_destroy_policer_conn_not_found",		/* No of v6 Policer delete failure due to connection not found */
	"v6_tun_create_policer_fail_mem",			/* No of v6 Policer create failure due to OOM */
	"v6_tun_create_policer_fail_conn",			/* No of v6 Policer create failure due to invalid parameters */
	"v6_tun_create_policer_fail_collision",		/* No of v6 Policer create failure due to connection already exist */
	"v6_tun_unknown_policer_interface",			/* No of v6 Policer create failure due to invalid IF */
	"v6_tun_create_policer_noedit_rule",		/* No of v6 Policer non edit rule create */
	"v6_tun_create_policer_fail_invalid_rx_if",		/* No of v6 Policer create failure due to invalid Rx IF */
	"v6_tun_create_policer_fail_invalid_tx_if",		/* No of v6 Policer create failure due to invalid Tx IF */
	"v6_tun_create_policer_fail_invalid_rx_port",	/* No of v6 Policer create failure due to invalid Rx Port */
	"v6_tun_create_policer_fail_invalid_tx_port",	/* No of v6 Policer create failure due to invalid Tx Port */

	"v6_tun_create_fail_acl",		/* No of v6 create failure due to ACL linking */
	"v6_tun_destroy_fail_acl",		/* No of v6 delete failure due to ACL unlinking */

	"v6_create_fail_offload_disabled",	/* No of v6 create failure due to offload disable */
};

/*
 * ppe_drv_stats_acl_str
 *	PPE DRV ACL statistics
 */
static const char *ppe_drv_stats_acl_str[] = {
	"active_rules",		/* Number of active rules */
	"req_slices",		/* Number of request ACL slices */
	"total_slices",		/* Number of total ACL slices used */
	"list_id_full",		/* ACL list ID full */
	"list_create_fail",	/* ACL list create failures */
	"configure_fail",	/* ACL rule configure failures */
	"rule_add_fail",	/* ACL rule add failures */
	"rule_bind_fail",	/* ACL rule bind failures */
	"rule_delete_fail",	/* ACL rule delete failures */
	"list_delete_fail",	/* ACL list delete failures */
};

/*
 * ppe_drv_stats_policer_str
 *	PPE DRV policer statistics
 */
static const char *ppe_drv_stats_policer_str[] = {
	"fail_acl_policer_full",			/* Policer table full */
	"fail_port_policer_full",			/* Policer table full */
	"fail_hw_port_policer_free_cfg",		/* Fail to reset port policer config */
	"fail_hw_acl_policer_free_cfg",			/* Fail to reset ACL policer config */
	"fail_hw_port_policer_create_cfg",		/* Fail to reset port policer config */
	"fail_hw_acl_policer_create_cfg",		/* Fail to reset ACL policer config */
	"success_hw_port_policer_free_cfg",		/* Fail to reset port policer config */
	"success_hw_acl_policer_free_cfg",		/* Fail to reset ACL policer config */
	"success_hw_port_policer_create_cfg",		/* Fail to reset port policer config */
	"success_hw_acl_policer_create_cfg",		/* Fail to reset ACL policer config */
};

/*
 * ppe_drv_stats_acl_show()
 *	Read ppe acl statistics
 */
static int ppe_drv_stats_acl_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	uint64_t *stats_shadow, *acl_stats;
	int i;

	acl_stats = kzalloc(sizeof(struct ppe_drv_stats_acl), GFP_KERNEL);
	if (!acl_stats) {
		ppe_drv_warn("%p: failed to allocate acl stats buffer\n", p);
		return -ENOMEM;
	}

	spin_lock_bh(&p->lock);
	memcpy(acl_stats, &p->stats.acl_stats, sizeof(struct ppe_drv_stats_acl));
	spin_unlock_bh(&p->lock);

	seq_puts(m, "\nPPE ACL stats:\n\n");
	stats_shadow = acl_stats;
	for (i = 0; i < sizeof(struct ppe_drv_stats_acl) / sizeof(uint64_t); i++) {
		seq_printf(m, "\t\t [%s]:  %llu\n", ppe_drv_stats_acl_str[i], stats_shadow[i]);
	}

	kfree(acl_stats);
	return 0;
};

/*
 * ppe_drv_stats_policer_show()
 *	Read ppe acl statistics
 */
static int ppe_drv_stats_policer_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	uint64_t *stats_shadow, *policer_stats;
	int i;

	policer_stats = kzalloc(sizeof(struct ppe_drv_stats_policer), GFP_KERNEL);
	if (!policer_stats) {
		ppe_drv_warn("%p: failed to allocate acl stats buffer\n", p);
		return -ENOMEM;
	}

	spin_lock_bh(&p->lock);
	memcpy(policer_stats, &p->stats.policer_stats, sizeof(struct ppe_drv_stats_policer));
	spin_unlock_bh(&p->lock);

	seq_puts(m, "\nPPE policer stats:\n\n");
	stats_shadow = policer_stats;
	for (i = 0; i < sizeof(struct ppe_drv_stats_policer) / sizeof(uint64_t); i++) {
		seq_printf(m, "\t\t [%s]:  %llu\n", ppe_drv_stats_policer_str[i], stats_shadow[i]);
	}

	kfree(policer_stats);
	return 0;
};

/*
 * ppe_drv_stats_sawf_sc_str
 *	PPE DRV connection statistics
 */
static const char *ppe_drv_stats_sawf_sc_str[] = {
	"rx_packets",		/* Per service-class counter for packets recieved on ethernet port */
	"rx_bytes",		/* Per service-class coutner for bytes recieved on ethernet port */
	"flow_count",		/* Per service-class counter for number of flows */
};

/*
 * ppe_drv_conn_stats_sawf_sc_show()
 *	Read ppe connection statistics
 */
static int ppe_drv_conn_stats_sawf_sc_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_stats_sawf_sc *sawf_sc_stats;
	uint64_t *stats_shadow;
	int i;

	sawf_sc_stats = kzalloc(PPE_DRV_SAWF_SC_MAX * sizeof(struct ppe_drv_stats_sawf_sc), GFP_KERNEL);
	if (!sawf_sc_stats) {
		ppe_drv_warn("Error in allocating the service stats buffer\n");
		return -ENOMEM;
	}

	spin_lock_bh(&p->lock);
	memcpy(sawf_sc_stats, p->stats.sawf_sc_stats, sizeof(struct ppe_drv_stats_sawf_sc) * PPE_DRV_SAWF_SC_MAX);
	spin_unlock_bh(&p->lock);

	seq_puts(m, "\nPPE_SAWF per service class stats:\n\n");
	stats_shadow = (uint64_t *)sawf_sc_stats;
	for (i = PPE_DRV_SAWF_SC_START; i <= PPE_DRV_SAWF_SC_END; i++) {
		uint64_t stats1 = *stats_shadow++;
		uint64_t stats2 = *stats_shadow++;
		uint64_t stats3 = *stats_shadow++;
		seq_printf(m, "\t\t For service class: %d \t\t %s:%llu  %s:%llu  %s:%llu\n", i, ppe_drv_stats_sawf_sc_str[0], stats1, ppe_drv_stats_sawf_sc_str[1], stats2, ppe_drv_stats_sawf_sc_str[2], stats3);
	}

	kfree(sawf_sc_stats);
	return 0;
};

/*
 * ppe_drv_stats_sc_str
 * 	PPE DRV connection statistics
 */
static const char *ppe_drv_stats_sc_str[] = {
	"sc_cb_unregister",		/* Per service-code counter for callback not registered */
	"sc_cb_packet_consumed",	/* Per service-code coutner for successful packet consumption in callback */
	"sc_cb_packet_processed",	/* Per service-code counter for packet processed in callback */
	"sc_vp_cb_unregister",		/* Per service-code counter for vp callback not registered */
	"sc_vp_cb_packet_consumed",	/* Per service-code coutner for successful packet consumption in vp callback */
	"sc_vp_cb_packet_processed",	/* Per service-code counter for packet processed in vp callback */
};

/*
 * ppe_drv_conn_stats_sc_show()
 *	Read ppe connection statistics
 */
static int ppe_drv_conn_stats_sc_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_stats_sc *sc_stats;
	uint64_t *stats_shadow;
	int i;

	sc_stats = kzalloc(PPE_DRV_SC_MAX * sizeof(struct ppe_drv_stats_sc), GFP_KERNEL);
	if (!sc_stats) {
		ppe_drv_warn("Error in allocating the service stats buffer\n");
		return -ENOMEM;
	}

	spin_lock_bh(&p->lock);
	memcpy(sc_stats, p->stats.sc_stats, sizeof(struct ppe_drv_stats_sc) * PPE_DRV_SC_MAX);
	spin_unlock_bh(&p->lock);

	seq_puts(m, "\nPPE_sc_stats:\n\n");
	stats_shadow = (uint64_t *)sc_stats;
	for (i = 0; i <= PPE_DRV_SC_NOEDIT_RULE; i++) {
		uint64_t stats1 = *stats_shadow++;
		uint64_t stats2 = *stats_shadow++;
		uint64_t stats3 = *stats_shadow++;
		uint64_t stats4 = *stats_shadow++;
		uint64_t stats5 = *stats_shadow++;
		uint64_t stats6 = *stats_shadow++;
		seq_printf(m, "\t\t %s\t %s:%llu  %s:%llu  %s:%llu  %s:%llu  %s:%llu  %s:%llu\n", ppe_drv_stats_sc_name_str[i], ppe_drv_stats_sc_str[0], stats1, ppe_drv_stats_sc_str[1], stats2, ppe_drv_stats_sc_str[2], stats3, ppe_drv_stats_sc_str[3], stats4, ppe_drv_stats_sc_str[4], stats5, ppe_drv_stats_sc_str[5], stats6);
	}

	kfree(sc_stats);
	return 0;
}

/*
 * ppe_drv_conn_stats_show()
 *	Read ppe connection statistics
 */
static int ppe_drv_conn_stats_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	uint64_t *stats, *stats_shadow;
	struct ppe_drv_comm_stats *comm_stats;
	uint32_t stats_size;
	int i;

	stats_size = (sizeof(p->stats.gen_stats) > sizeof(p->stats.comm_stats)) ?
			sizeof(p->stats.gen_stats) : sizeof(p->stats.comm_stats);

	stats = kzalloc(stats_size, GFP_KERNEL);
	if (!stats) {
		ppe_drv_warn("Error in allocating gen stats\n");
		return -ENOMEM;
	}

	spin_lock_bh(&p->lock);
	memcpy(stats, &p->stats.gen_stats, sizeof(struct ppe_drv_gen_stats));
	spin_unlock_bh(&p->lock);

	seq_puts(m, "\nPPE stats:\n\n");
	stats_shadow = stats;
	for (i = 0; i < sizeof(struct ppe_drv_gen_stats) / sizeof(uint64_t); i++) {
		seq_printf(m, "\t\t [%s]:  %llu\n", ppe_drv_stats_conn_str[i], stats_shadow[i]);
	}

	spin_lock_bh(&p->lock);
	memcpy(stats, &p->stats.comm_stats, sizeof(p->stats.comm_stats));
	spin_unlock_bh(&p->lock);

	comm_stats = (struct ppe_drv_comm_stats *)stats;

	seq_puts(m, "\nPPE flow stats:\n\n");
	stats_shadow = (uint64_t *)&comm_stats[PPE_DRV_CONN_TYPE_FLOW];
	for (i = 0; i < sizeof(struct ppe_drv_comm_stats) / sizeof(uint64_t); i++) {
		seq_printf(m, "\t\t [%s]:  %llu\n", ppe_drv_comm_stats_flow_conn_str[i], stats_shadow[i]);
	}

	seq_puts(m, "\nPPE tunnel stats:\n\n");
	stats_shadow = (uint64_t *)&comm_stats[PPE_DRV_CONN_TYPE_TUNNEL];
	for (i = 0; i < sizeof(struct ppe_drv_comm_stats) / sizeof(uint64_t); i++) {
		seq_printf(m, "\t\t [%s]:  %llu\n", ppe_drv_comm_stats_tun_conn_str[i], stats_shadow[i]);
	}

	kfree(stats);
	return 0;
}

/*
 * ppe_drv_stats_acl_open()
 *	PPE ACL stats open callback API
 */
static int ppe_drv_stats_acl_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_drv_stats_acl_show, inode->i_private);
}

/*
 * ppe_drv_stats_acl_file_ops
 *	File operations for ACL stats
 */
const struct file_operations ppe_drv_stats_acl_file_ops = {
	.open = ppe_drv_stats_acl_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_drv_stats_policer_open()
 *	PPE Policer stats open callback API
 */
static int ppe_drv_stats_policer_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_drv_stats_policer_show, inode->i_private);
}

/*
 * ppe_drv_stats_policer_file_ops
 *	File operations for Policer stats
 */
const struct file_operations ppe_drv_stats_policer_file_ops = {
	.open = ppe_drv_stats_policer_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_drv_conn_stats_sawf_sc_open()
 *	PPE drv conn open callback API
 */
static int ppe_drv_conn_stats_sawf_sc_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_drv_conn_stats_sawf_sc_show, inode->i_private);
}

/*
 * ppe_drv_conn_stats_sawf_sc_file_ops
 *	File operations for EDMA common stats
 */
const struct file_operations ppe_drv_conn_stats_sawf_sc_file_ops = {
	.open = ppe_drv_conn_stats_sawf_sc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_drv_conn_stats_sc_open()
 *	PPE drv conn open callback API
 */
static int ppe_drv_conn_stats_sc_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_drv_conn_stats_sc_show, inode->i_private);
}

/*
 * ppe_drv_conn_stats_sc_file_ops
 *	File operations for EDMA common stats
 */
const struct file_operations ppe_drv_conn_stats_sc_file_ops = {
	.open = ppe_drv_conn_stats_sc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_drv_conn_stats_general_open()
 *	PPE drv conn open callback API
 */
static int ppe_drv_conn_stats_general_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_drv_conn_stats_show, inode->i_private);
}

/*
 * ppe_drv_conn_stats_general_file_ops
 *	File operations for EDMA common stats
 */
const struct file_operations ppe_drv_conn_stats_general_file_ops = {
	.open = ppe_drv_conn_stats_general_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_drv_stats_debugfs_init()
 *	Create PPE statistics debug entry.
 */
int ppe_drv_stats_debugfs_init(void)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	p->dentry = debugfs_create_dir("qca-nss-ppe", NULL);
	if (!p->dentry) {
		ppe_drv_warn("%p: Unable to create debugfs stats directory in debugfs\n", p);
		return -1;
	}

	p->stats_dentry = debugfs_create_dir("stats", p->dentry);
	if (!p->stats_dentry) {
		ppe_drv_warn("%p: Unable to create debugfs stats directory in debugfs\n", p);
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_dir("clients", p->dentry)) {
		ppe_drv_warn("%p: Unable to create debugfs clients directory in debugfs\n", p);
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("common_stats", S_IRUGO, p->stats_dentry,
			NULL, &ppe_drv_conn_stats_general_file_ops)) {
		ppe_drv_warn("%p: Unable to create common statistics file entry in debugfs\n", p);
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("sc_stats", S_IRUGO, p->stats_dentry,
			NULL, &ppe_drv_conn_stats_sc_file_ops)) {
		ppe_drv_warn("%p: Unable to create sc_stats statistics file entry in debugfs\n", p);
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("sawf_sc_stats", S_IRUGO, p->stats_dentry,
			NULL, &ppe_drv_conn_stats_sawf_sc_file_ops)) {
		ppe_drv_warn("%p: Unable to create sawf_sc_stats file entry in debugfs\n", p);
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("acl_stats", S_IRUGO, p->stats_dentry,
			NULL, &ppe_drv_stats_acl_file_ops)) {
		ppe_drv_warn("%p: Unable to create acl_stats file entry in debugfs\n", p);
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("policer_stats", S_IRUGO, p->stats_dentry,
			NULL, &ppe_drv_stats_policer_file_ops)) {
		ppe_drv_warn("%p: Unable to create policer_stats file entry in debugfs\n", p);
		goto debugfs_dir_failed;
	}

	return 0;

debugfs_dir_failed:
	debugfs_remove_recursive(p->dentry);
	p->dentry = NULL;
	p->stats_dentry = NULL;
	return -1;
}

/*
 * ppe_drv_stats_debugfs_exit()
 *	PPE debugfs exit api
 */
void ppe_drv_stats_debugfs_exit(void)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	if (p->dentry) {
		debugfs_remove_recursive(p->dentry);
		p->dentry = NULL;
		p->stats_dentry = NULL;
	}
}
