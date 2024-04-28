/*
 **************************************************************************
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include "nss_core.h"
#include "nss_ppe.h"
#include "nss_ppe_stats.h"

static uint8_t ppe_cc_nonexception[NSS_PPE_STATS_CPU_CODE_NONEXCEPTION_MAX] = {
	NSS_PPE_STATS_CPU_CODE_EXP_FAKE_L2_PROT_ERR,
	NSS_PPE_STATS_CPU_CODE_EXP_FAKE_MAC_HEADER_ERR,
	NSS_PPE_STATS_CPU_CODE_EXP_BITMAP_MAX,
	NSS_PPE_STATS_CPU_CODE_L2_EXP_MRU_FAIL,
	NSS_PPE_STATS_CPU_CODE_L2_EXP_MTU_FAIL,
	NSS_PPE_STATS_CPU_CODE_L3_EXP_IP_PREFIX_BC,
	NSS_PPE_STATS_CPU_CODE_L3_EXP_MTU_FAIL,
	NSS_PPE_STATS_CPU_CODE_L3_EXP_MRU_FAIL,
	NSS_PPE_STATS_CPU_CODE_L3_EXP_ICMP_RDT,
	NSS_PPE_STATS_CPU_CODE_L3_EXP_IP_RT_TTL1_TO_ME,
	NSS_PPE_STATS_CPU_CODE_L3_EXP_IP_RT_TTL_ZERO,
	NSS_PPE_STATS_CPU_CODE_L3_FLOW_SERVICE_CODE_LOOP,
	NSS_PPE_STATS_CPU_CODE_L3_FLOW_DE_ACCELERATE,
	NSS_PPE_STATS_CPU_CODE_L3_EXP_FLOW_SRC_IF_CHK_FAIL,
	NSS_PPE_STATS_CPU_CODE_L3_FLOW_SYNC_TOGGLE_MISMATCH,
	NSS_PPE_STATS_CPU_CODE_L3_EXP_MTU_DF_FAIL,
	NSS_PPE_STATS_CPU_CODE_L3_EXP_PPPOE_MULTICAST,
	NSS_PPE_STATS_CPU_CODE_MGMT_OFFSET,
	NSS_PPE_STATS_CPU_CODE_MGMT_EAPOL,
	NSS_PPE_STATS_CPU_CODE_MGMT_PPPOE_DIS,
	NSS_PPE_STATS_CPU_CODE_MGMT_IGMP,
	NSS_PPE_STATS_CPU_CODE_MGMT_ARP_REQ,
	NSS_PPE_STATS_CPU_CODE_MGMT_ARP_REP,
	NSS_PPE_STATS_CPU_CODE_MGMT_DHCPv4,
	NSS_PPE_STATS_CPU_CODE_MGMT_MLD,
	NSS_PPE_STATS_CPU_CODE_MGMT_NS,
	NSS_PPE_STATS_CPU_CODE_MGMT_NA,
	NSS_PPE_STATS_CPU_CODE_MGMT_DHCPv6,
	NSS_PPE_STATS_CPU_CODE_PTP_OFFSET,
	NSS_PPE_STATS_CPU_CODE_PTP_SYNC,
	NSS_PPE_STATS_CPU_CODE_PTP_FOLLOW_UP,
	NSS_PPE_STATS_CPU_CODE_PTP_DELAY_REQ,
	NSS_PPE_STATS_CPU_CODE_PTP_DELAY_RESP,
	NSS_PPE_STATS_CPU_CODE_PTP_PDELAY_REQ,
	NSS_PPE_STATS_CPU_CODE_PTP_PDELAY_RESP,
	NSS_PPE_STATS_CPU_CODE_PTP_PDELAY_RESP_FOLLOW_UP,
	NSS_PPE_STATS_CPU_CODE_PTP_ANNOUNCE,
	NSS_PPE_STATS_CPU_CODE_PTP_MANAGEMENT,
	NSS_PPE_STATS_CPU_CODE_PTP_SIGNALING,
	NSS_PPE_STATS_CPU_CODE_PTP_PKT_RSV_MSG,
	NSS_PPE_STATS_CPU_CODE_IPV4_SG_UNKNOWN,
	NSS_PPE_STATS_CPU_CODE_IPV6_SG_UNKNOWN,
	NSS_PPE_STATS_CPU_CODE_ARP_SG_UNKNOWN,
	NSS_PPE_STATS_CPU_CODE_ND_SG_UNKNOWN,
	NSS_PPE_STATS_CPU_CODE_IPV4_SG_VIO,
	NSS_PPE_STATS_CPU_CODE_IPV6_SG_VIO,
	NSS_PPE_STATS_CPU_CODE_ARP_SG_VIO,
	NSS_PPE_STATS_CPU_CODE_ND_SG_VIO,
	NSS_PPE_STATS_CPU_CODE_L3_ROUTING_IP_TO_ME,
	NSS_PPE_STATS_CPU_CODE_L3_FLOW_SNAT_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_FLOW_DNAT_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_FLOW_RT_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_FLOW_BR_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_MC_BRIDGE_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_ROUTE_PREHEAD_RT_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_ROUTE_PREHEAD_SNAPT_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_ROUTE_PREHEAD_DNAPT_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_ROUTE_PREHEAD_SNAT_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_ROUTE_PREHEAD_DNAT_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_NO_ROUTE_PREHEAD_NAT_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_NO_ROUTE_PREHEAD_NAT_ERROR,
	NSS_PPE_STATS_CPU_CODE_L3_ROUTE_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_NO_ROUTE_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_NO_ROUTE_NH_INVALID_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_NO_ROUTE_PREHEAD_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_BRIDGE_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_FLOW_ACTION,
	NSS_PPE_STATS_CPU_CODE_L3_FLOW_MISS_ACTION,
	NSS_PPE_STATS_CPU_CODE_L2_NEW_MAC_ADDRESS,
	NSS_PPE_STATS_CPU_CODE_L2_HASH_COLLISION,
	NSS_PPE_STATS_CPU_CODE_L2_STATION_MOVE,
	NSS_PPE_STATS_CPU_CODE_L2_LEARN_LIMIT,
	NSS_PPE_STATS_CPU_CODE_L2_SA_LOOKUP_ACTION,
	NSS_PPE_STATS_CPU_CODE_L2_DA_LOOKUP_ACTION,
	NSS_PPE_STATS_CPU_CODE_APP_CTRL_ACTION,
	NSS_PPE_STATS_CPU_CODE_IN_VLAN_FILTER_ACTION,
	NSS_PPE_STATS_CPU_CODE_IN_VLAN_XLT_MISS,
	NSS_PPE_STATS_CPU_CODE_EG_VLAN_FILTER_DROP,
	NSS_PPE_STATS_CPU_CODE_ACL_PRE_ACTION,
	NSS_PPE_STATS_CPU_CODE_ACL_POST_ACTION,
	NSS_PPE_STATS_CPU_CODE_SERVICE_CODE_ACTION,
};

/*
 * nss_ppe_stats_str_conn
 *	PPE statistics strings for nss flow stats
 */
static int8_t *nss_ppe_stats_str_conn[NSS_PPE_STATS_CONN_MAX] = {
	"v4 routed flows",
	"v4 bridge flows",
	"v4 conn create req",
	"v4 conn create fail",
	"v4 conn destroy req",
	"v4 conn destroy fail",
	"v4 conn MC create req",
	"v4 conn MC create fail",
	"v4 conn MC update req",
	"v4 conn MC update fail",
	"v4 conn MC delete req",
	"v4 conn MC delete fail",
	"v4 conn unknown if",

	"v6 routed flows",
	"v6 bridge flows",
	"v6 conn create req",
	"v6 conn create fail",
	"v6 conn destroy req",
	"v6 conn destroy fail",
	"v6 conn MC create req",
	"v6 conn MC create fail",
	"v6 conn MC update req",
	"v6 conn MC update fail",
	"v6 conn MC delete req",
	"v6 conn MC delete fail",
	"v6 conn unknown if",

	"conn fail - vp full",
	"conn fail - nexthop full",
	"conn fail - flow full",
	"conn fail - host full",
	"conn fail - pub-ip full",
	"conn fail - port not setup",
	"conn fail - rw fifo full",
	"conn fail - flow cmd failure",
	"conn fail - unknown proto",
	"conn fail - ppe not responding",
	"conn fail - CE opaque invalid",
	"conn fail - fqg full"
};

/*
 * nss_ppe_stats_str_l3
 *	PPE statistics strings for nss debug stats
 */
static int8_t *nss_ppe_stats_str_l3[NSS_PPE_STATS_L3_MAX] = {
	"PPE L3 dbg reg 0",
	"PPE L3 dbg reg 1",
	"PPE L3 dbg reg 2",
	"PPE L3 dbg reg 3",
	"PPE L3 dbg reg 4",
	"PPE L3 dbg reg port",
};

/*
 * nss_ppe_stats_str_code
 *	PPE statistics strings for nss debug stats
 */
static int8_t *nss_ppe_stats_str_code[NSS_PPE_STATS_CODE_MAX] = {
	"PPE CPU_CODE",
	"PPE DROP_CODE",
};

/*
 * nss_ppe_stats_str_dc
 *	PPE statistics strings for drop code
 */
static int8_t *nss_ppe_stats_str_dc[NSS_PPE_STATS_DROP_CODE_MAX] = {
	"PPE_DROP_CODE_NONE",
	"PPE_DROP_CODE_EXP_UNKNOWN_L2_PORT",
	"PPE_DROP_CODE_EXP_PPPOE_WRONG_VER_TYPE",
	"PPE_DROP_CODE_EXP_PPPOE_WRONG_CODE",
	"PPE_DROP_CODE_EXP_PPPOE_UNSUPPORTED_PPP_PROT",
	"PPE_DROP_CODE_EXP_IPV4_WRONG_VER",
	"PPE_DROP_CODE_EXP_IPV4_SMALL_IHL",
	"PPE_DROP_CODE_EXP_IPV4_WITH_OPTION",
	"PPE_DROP_CODE_EXP_IPV4_HDR_INCOMPLETE",
	"PPE_DROP_CODE_EXP_IPV4_BAD_TOTAL_LEN",
	"PPE_DROP_CODE_EXP_IPV4_DATA_INCOMPLETE",
	"PPE_DROP_CODE_EXP_IPV4_FRAG",
	"PPE_DROP_CODE_EXP_IPV4_PING_OF_DEATH",
	"PPE_DROP_CODE_EXP_IPV4_SNALL_TTL",
	"PPE_DROP_CODE_EXP_IPV4_UNK_IP_PROT",
	"PPE_DROP_CODE_EXP_IPV4_CHECKSUM_ERR",
	"PPE_DROP_CODE_EXP_IPV4_INV_SIP",
	"PPE_DROP_CODE_EXP_IPV4_INV_DIP",
	"PPE_DROP_CODE_EXP_IPV4_LAND_ATTACK",
	"PPE_DROP_CODE_EXP_IPV4_AH_HDR_INCOMPLETE",
	"PPE_DROP_CODE_EXP_IPV4_AH_HDR_CROSS_BORDER",
	"PPE_DROP_CODE_EXP_IPV4_ESP_HDR_INCOMPLETE",
	"PPE_DROP_CODE_EXP_IPV6_WRONG_VER",
	"PPE_DROP_CODE_EXP_IPV6_HDR_INCOMPLETE",
	"PPE_DROP_CODE_EXP_IPV6_BAD_PAYLOAD_LEN",
	"PPE_DROP_CODE_EXP_IPV6_DATA_INCOMPLETE",
	"PPE_DROP_CODE_EXP_IPV6_WITH_EXT_HDR",
	"PPE_DROP_CODE_EXP_IPV6_SMALL_HOP_LIMIT",
	"PPE_DROP_CODE_EXP_IPV6_INV_SIP",
	"PPE_DROP_CODE_EXP_IPV6_INV_DIP",
	"PPE_DROP_CODE_EXP_IPV6_LAND_ATTACK",
	"PPE_DROP_CODE_EXP_IPV6_FRAG",
	"PPE_DROP_CODE_EXP_IPV6_PING_OF_DEATH",
	"PPE_DROP_CODE_EXP_IPV6_WITH_MORE_EXT_HDR",
	"PPE_DROP_CODE_EXP_IPV6_UNK_LAST_NEXT_HDR",
	"PPE_DROP_CODE_EXP_IPV6_MOBILITY_HDR_INCOMPLETE",
	"PPE_DROP_CODE_EXP_IPV6_MOBILITY_HDR_CROSS_BORDER",
	"PPE_DROP_CODE_EXP_IPV6_AH_HDR_INCOMPLETE",
	"PPE_DROP_CODE_EXP_IPV6_AH_HDR_CROSS_BORDER",
	"PPE_DROP_CODE_EXP_IPV6_ESP_HDR_INCOMPLETE",
	"PPE_DROP_CODE_EXP_IPV6_ESP_HDR_CROSS_BORDER",
	"PPE_DROP_CODE_EXP_IPV6_OTHER_EXT_HDR_INCOMPLETE",
	"PPE_DROP_CODE_EXP_IPV6_OTHER_EXT_HDR_CROSS_BORDER",
	"PPE_DROP_CODE_EXP_TCP_HDR_INCOMPLETE",
	"PPE_DROP_CODE_EXP_TCP_HDR_CROSS_BORDER",
	"PPE_DROP_CODE_EXP_TCP_SMAE_SP_DP",
	"PPE_DROP_CODE_EXP_TCP_SMALL_DATA_OFFSET",
	"PPE_DROP_CODE_EXP_TCP_FLAGS_0",
	"PPE_DROP_CODE_EXP_TCP_FLAGS_1",
	"PPE_DROP_CODE_EXP_TCP_FLAGS_2",
	"PPE_DROP_CODE_EXP_TCP_FLAGS_3",
	"PPE_DROP_CODE_EXP_TCP_FLAGS_4",
	"PPE_DROP_CODE_EXP_TCP_FLAGS_5",
	"PPE_DROP_CODE_EXP_TCP_FLAGS_6",
	"PPE_DROP_CODE_EXP_TCP_FLAGS_7",
	"PPE_DROP_CODE_EXP_TCP_CHECKSUM_ERR",
	"PPE_DROP_CODE_EXP_UDP_HDR_INCOMPLETE",
	"PPE_DROP_CODE_EXP_UDP_HDR_CROSS_BORDER",
	"PPE_DROP_CODE_EXP_UDP_SMAE_SP_DP",
	"PPE_DROP_CODE_EXP_UDP_BAD_LEN",
	"PPE_DROP_CODE_EXP_UDP_DATA_INCOMPLETE",
	"PPE_DROP_CODE_EXP_UDP_CHECKSUM_ERR",
	"PPE_DROP_CODE_EXP_UDP_LITE_HDR_INCOMPLETE",
	"PPE_DROP_CODE_EXP_UDP_LITE_HDR_CROSS_BORDER",
	"PPE_DROP_CODE_EXP_UDP_LITE_SMAE_SP_DP",
	"PPE_DROP_CODE_EXP_UDP_LITE_CSM_COV_1_TO_7",
	"PPE_DROP_CODE_EXP_UDP_LITE_CSM_COV_TOO_LONG",
	"PPE_DROP_CODE_EXP_UDP_LITE_CSM_COV_CROSS_BORDER",
	"PPE_DROP_CODE_EXP_UDP_LITE_CHECKSUM_ERR",
	"PPE_DROP_CODE_L3_MC_BRIDGE_ACTION",
	"PPE_DROP_CODE_L3_NO_ROUTE_PREHEAD_NAT_ACTION",
	"PPE_DROP_CODE_L3_NO_ROUTE_PREHEAD_NAT_ERROR",
	"PPE_DROP_CODE_L3_ROUTE_ACTION",
	"PPE_DROP_CODE_L3_NO_ROUTE_ACTION",
	"PPE_DROP_CODE_L3_NO_ROUTE_NH_INVALID_ACTION",
	"PPE_DROP_CODE_L3_NO_ROUTE_PREHEAD_ACTION",
	"PPE_DROP_CODE_L3_BRIDGE_ACTION",
	"PPE_DROP_CODE_L3_FLOW_ACTION",
	"PPE_DROP_CODE_L3_FLOW_MISS_ACTION",
	"PPE_DROP_CODE_L2_EXP_MRU_FAIL",
	"PPE_DROP_CODE_L2_EXP_MTU_FAIL",
	"PPE_DROP_CODE_L3_EXP_IP_PREFIX_BC",
	"PPE_DROP_CODE_L3_EXP_MTU_FAIL",
	"PPE_DROP_CODE_L3_EXP_MRU_FAIL",
	"PPE_DROP_CODE_L3_EXP_ICMP_RDT",
	"PPE_DROP_CODE_FAKE_MAC_HEADER_ERR",
	"PPE_DROP_CODE_L3_EXP_IP_RT_TTL_ZERO",
	"PPE_DROP_CODE_L3_FLOW_SERVICE_CODE_LOOP",
	"PPE_DROP_CODE_L3_FLOW_DE_ACCELEARTE",
	"PPE_DROP_CODE_L3_EXP_FLOW_SRC_IF_CHK_FAIL",
	"PPE_DROP_CODE_L3_FLOW_SYNC_TOGGLE_MISMATCH",
	"PPE_DROP_CODE_L3_EXP_MTU_DF_FAIL",
	"PPE_DROP_CODE_L3_EXP_PPPOE_MULTICAST",
	"PPE_DROP_CODE_IPV4_SG_UNKNOWN",
	"PPE_DROP_CODE_IPV6_SG_UNKNOWN",
	"PPE_DROP_CODE_ARP_SG_UNKNOWN",
	"PPE_DROP_CODE_ND_SG_UNKNOWN",
	"PPE_DROP_CODE_IPV4_SG_VIO",
	"PPE_DROP_CODE_IPV6_SG_VIO",
	"PPE_DROP_CODE_ARP_SG_VIO",
	"PPE_DROP_CODE_ND_SG_VIO",
	"PPE_DROP_CODE_L2_NEW_MAC_ADDRESS",
	"PPE_DROP_CODE_L2_HASH_COLLISION",
	"PPE_DROP_CODE_L2_STATION_MOVE",
	"PPE_DROP_CODE_L2_LEARN_LIMIT",
	"PPE_DROP_CODE_L2_SA_LOOKUP_ACTION",
	"PPE_DROP_CODE_L2_DA_LOOKUP_ACTION",
	"PPE_DROP_CODE_APP_CTRL_ACTION",
	"PPE_DROP_CODE_IN_VLAN_FILTER_ACTION",
	"PPE_DROP_CODE_IN_VLAN_XLT_MISS",
	"PPE_DROP_CODE_EG_VLAN_FILTER_DROP",
	"PPE_DROP_CODE_ACL_PRE_ACTION",
	"PPE_DROP_CODE_ACL_POST_ACTION",
	"PPE_DROP_CODE_MC_BC_SA",
	"PPE_DROP_CODE_NO_DESTINATION",
	"PPE_DROP_CODE_STG_IN_FILTER",
	"PPE_DROP_CODE_STG_EG_FILTER",
	"PPE_DROP_CODE_SOURCE_FILTER_FAIL",
	"PPE_DROP_CODE_TRUNK_SEL_FAIL",
	"PPE_DROP_CODE_TX_EN_FAIL",
	"PPE_DROP_CODE_VLAN_TAG_FMT",
	"PPE_DROP_CODE_CRC_ERR",
	"PPE_DROP_CODE_PAUSE_FRAME",
	"PPE_DROP_CODE_PROMISC",
	"PPE_DROP_CODE_ISOLATION",
	"PPE_DROP_CODE_MGMT_APP",
	"PPE_DROP_CODE_FAKE_L2_PROT_ERR",
	"PPE_DROP_CODE_POLICER",
};

/*
 * nss_ppe_stats_str_cc
 *      PPE statistics strings for cpu code
 */
static int8_t *nss_ppe_stats_str_cc[NSS_PPE_STATS_CPU_CODE_MAX] = {
	"PPE_CPU_CODE_FORWARDING",
	"PPE_CPU_CODE_EXP_UNKNOWN_L2_PROT",
	"PPE_CPU_CODE_EXP_PPPOE_WRONG_VER_TYPE",
	"PPE_CPU_CODE_EXP_WRONG_CODE",
	"PPE_CPU_CODE_EXP_PPPOE_UNSUPPORTED_PPP_PROT",
	"PPE_CPU_CODE_EXP_WRONG_VER",
	"PPE_CPU_CODE_EXP_SMALL_IHL",
	"PPE_CPU_CODE_EXP_WITH_OPTION",
	"PPE_CPU_CODE_EXP_HDR_INCOMPLETE",
	"PPE_CPU_CODE_EXP_IPV4_BAD_TOTAL_LEN",
	"PPE_CPU_CODE_EXP_DATA_INCOMPLETE",
	"PPE_CPU_CODE_IPV4_FRAG",
	"PPE_CPU_CODE_EXP_IPV4_PING_OF_DEATH",
	"PPE_CPU_CODE_EXP_SNALL_TTL",
	"PPE_CPU_CODE_EXP_IPV4_UNK_IP_PROT",
	"PPE_CPU_CODE_EXP_CHECKSUM_ERR",
	"PPE_CPU_CODE_EXP_INV_SIP",
	"PPE_CPU_CODE_EXP_INV_DIP",
	"PPE_CPU_CODE_EXP_LAND_ATTACK",
	"PPE_CPU_CODE_EXP_IPV4_AH_HDR_INCOMPLETE",
	"PPE_CPU_CODE_EXP_IPV4_AH_CROSS_BORDER",
	"PPE_CPU_CODE_EXP_IPV4_ESP_HDR_INCOMPLETE",
	"PPE_CPU_CODE_EXP_WRONG_VER",
	"PPE_CPU_CODE_EXP_HDR_INCOMPLETE",
	"PPE_CPU_CODE_EXP_IPV6_BAD_PAYLOAD_LEN",
	"PPE_CPU_CODE_EXP_DATA_INCOMPLETE",
	"PPE_CPU_CODE_EXP_IPV6_WITH_EXT_HDR",
	"PPE_CPU_CODE_EXP_IPV6_SMALL_HOP_LIMIT",
	"PPE_CPU_CODE_EXP_INV_SIP",
	"PPE_CPU_CODE_EXP_INV_DIP",
	"PPE_CPU_CODE_EXP_LAND_ATTACK",
	"PPE_CPU_CODE_IPV6_FRAG",
	"PPE_CPU_CODE_EXP_IPV6_PING_OF_DEATH",
	"PPE_CPU_CODE_EXP_IPV6_WITH_EXT_HDR",
	"PPE_CPU_CODE_EXP_IPV6_UNK_NEXT_HDR",
	"PPE_CPU_CODE_EXP_IPV6_MOBILITY_HDR_INCOMPLETE",
	"PPE_CPU_CODE_EXP_IPV6_MOBILITY_CROSS_BORDER",
	"PPE_CPU_CODE_EXP_IPV6_AH_HDR_INCOMPLETE",
	"PPE_CPU_CODE_EXP_IPV6_AH_CROSS_BORDER",
	"PPE_CPU_CODE_EXP_IPV6_ESP_HDR_INCOMPLETE",
	"PPE_CPU_CODE_EXP_IPV6_ESP_CROSS_BORDER",
	"PPE_CPU_CODE_EXP_IPV6_OTHER_HDR_INCOMPLETE",
	"PPE_CPU_CODE_EXP_IPV6_OTHER_EXT_CROSS_BORDER",
	"PPE_CPU_CODE_EXP_HDR_INCOMPLETE",
	"PPE_CPU_CODE_EXP_TCP_HDR_CROSS_BORDER",
	"PPE_CPU_CODE_EXP_TCP_SMAE_SP_DP",
	"PPE_CPU_CODE_EXP_TCP_SMALL_DATA_OFFSET",
	"PPE_CPU_CODE_EXP_FLAGS_0",
	"PPE_CPU_CODE_EXP_FLAGS_1",
	"PPE_CPU_CODE_EXP_FLAGS_2",
	"PPE_CPU_CODE_EXP_FLAGS_3",
	"PPE_CPU_CODE_EXP_FLAGS_4",
	"PPE_CPU_CODE_EXP_FLAGS_5",
	"PPE_CPU_CODE_EXP_FLAGS_6",
	"PPE_CPU_CODE_EXP_FLAGS_7",
	"PPE_CPU_CODE_EXP_CHECKSUM_ERR",
	"PPE_CPU_CODE_EXP_HDR_INCOMPLETE",
	"PPE_CPU_CODE_EXP_UDP_HDR_CROSS_BORDER",
	"PPE_CPU_CODE_EXP_UDP_SMAE_SP_DP",
	"PPE_CPU_CODE_EXP_BAD_LEN",
	"PPE_CPU_CODE_EXP_DATA_INCOMPLETE",
	"PPE_CPU_CODE_EXP_CHECKSUM_ERR",
	"PPE_CPU_CODE_EXP_UDP_LITE_HDR_INCOMPLETE",
	"PPE_CPU_CODE_EXP_UDP_LITE_CROSS_BORDER",
	"PPE_CPU_CODE_EXP_UDP_LITE_SP_DP",
	"PPE_CPU_CODE_EXP_UDP_LITE_CSM_COV_TO_7",
	"PPE_CPU_CODE_EXP_UDP_LITE_CSM_TOO_LONG",
	"PPE_CPU_CODE_EXP_UDP_LITE_CSM_CROSS_BORDER",
	"PPE_CPU_CODE_EXP_UDP_LITE_CHECKSUM_ERR",
	"PPE_CPU_CODE_EXP_FAKE_L2_PROT_ERR",
	"PPE_CPU_CODE_EXP_FAKE_MAC_HEADER_ERR",
	"PPE_CPU_CODE_BITMAP_MAX",
	"PPE_CPU_CODE_L2_MRU_FAIL",
	"PPE_CPU_CODE_L2_MTU_FAIL",
	"PPE_CPU_CODE_L3_EXP_IP_PREFIX_BC",
	"PPE_CPU_CODE_L3_MTU_FAIL",
	"PPE_CPU_CODE_L3_MRU_FAIL",
	"PPE_CPU_CODE_L3_ICMP_RDT",
	"PPE_CPU_CODE_L3_EXP_IP_RT_TO_ME",
	"PPE_CPU_CODE_L3_EXP_IP_TTL_ZERO",
	"PPE_CPU_CODE_L3_FLOW_SERVICE_CODE_LOOP",
	"PPE_CPU_CODE_L3_DE_ACCELERATE",
	"PPE_CPU_CODE_L3_EXP_FLOW_SRC_CHK_FAIL",
	"PPE_CPU_CODE_L3_FLOW_SYNC_TOGGLE_MISMATCH",
	"PPE_CPU_CODE_L3_EXP_MTU_DF_FAIL",
	"PPE_CPU_CODE_L3_PPPOE_MULTICAST",
	"PPE_CPU_CODE_MGMT_OFFSET",
	"PPE_CPU_CODE_MGMT_EAPOL",
	"PPE_CPU_CODE_PPPOE_DIS",
	"PPE_CPU_CODE_MGMT_IGMP",
	"PPE_CPU_CODE_ARP_REQ",
	"PPE_CPU_CODE_ARP_REP",
	"PPE_CPU_CODE_MGMT_DHCPv4",
	"PPE_CPU_CODE_MGMT_MLD",
	"PPE_CPU_CODE_MGMT_NS",
	"PPE_CPU_CODE_MGMT_NA",
	"PPE_CPU_CODE_MGMT_DHCPv6",
	"PPE_CPU_CODE_PTP_OFFSET",
	"PPE_CPU_CODE_PTP_SYNC",
	"PPE_CPU_CODE_FOLLOW_UP",
	"PPE_CPU_CODE_DELAY_REQ",
	"PPE_CPU_CODE_DELAY_RESP",
	"PPE_CPU_CODE_PDELAY_REQ",
	"PPE_CPU_CODE_PDELAY_RESP",
	"PPE_CPU_CODE_PTP_PDELAY_RESP_FOLLOW_UP",
	"PPE_CPU_CODE_PTP_ANNOUNCE",
	"PPE_CPU_CODE_PTP_MANAGEMENT",
	"PPE_CPU_CODE_PTP_SIGNALING",
	"PPE_CPU_CODE_PTP_RSV_MSG",
	"PPE_CPU_CODE_SG_UNKNOWN",
	"PPE_CPU_CODE_SG_UNKNOWN",
	"PPE_CPU_CODE_SG_UNKNOWN",
	"PPE_CPU_CODE_SG_UNKNOWN",
	"PPE_CPU_CODE_SG_VIO",
	"PPE_CPU_CODE_SG_VIO",
	"PPE_CPU_CODE_SG_VIO",
	"PPE_CPU_CODE_SG_VIO",
	"PPE_CPU_CODE_L3_ROUTING_IP_TO_ME",
	"PPE_CPU_CODE_L3_SNAT_ACTION",
	"PPE_CPU_CODE_L3_DNAT_ACTION",
	"PPE_CPU_CODE_L3_RT_ACTION",
	"PPE_CPU_CODE_L3_BR_ACTION",
	"PPE_CPU_CODE_L3_BRIDGE_ACTION",
	"PPE_CPU_CODE_L3_ROUTE_PREHEAD_RT_ACTION",
	"PPE_CPU_CODE_L3_ROUTE_PREHEAD_SNAPT_ACTION",
	"PPE_CPU_CODE_L3_ROUTE_PREHEAD_DNAPT_ACTION",
	"PPE_CPU_CODE_L3_ROUTE_PREHEAD_SNAT_ACTION",
	"PPE_CPU_CODE_L3_ROUTE_PREHEAD_DNAT_ACTION",
	"PPE_CPU_CODE_L3_NO_ROUTE_NAT_ACTION",
	"PPE_CPU_CODE_L3_NO_ROUTE_NAT_ERROR",
	"PPE_CPU_CODE_ROUTE_ACTION",
	"PPE_CPU_CODE_L3_ROUTE_ACTION",
	"PPE_CPU_CODE_L3_NO_ROUTE_INVALID_ACTION",
	"PPE_CPU_CODE_L3_NO_ROUTE_PREHEAD_ACTION",
	"PPE_CPU_CODE_BRIDGE_ACTION",
	"PPE_CPU_CODE_FLOW_ACTION",
	"PPE_CPU_CODE_L3_MISS_ACTION",
	"PPE_CPU_CODE_L2_MAC_ADDRESS",
	"PPE_CPU_CODE_HASH_COLLISION",
	"PPE_CPU_CODE_STATION_MOVE",
	"PPE_CPU_CODE_LEARN_LIMIT",
	"PPE_CPU_CODE_L2_LOOKUP_ACTION",
	"PPE_CPU_CODE_L2_LOOKUP_ACTION",
	"PPE_CPU_CODE_CTRL_ACTION",
	"PPE_CPU_CODE_IN_FILTER_ACTION",
	"PPE_CPU_CODE_IN_XLT_MISS",
	"PPE_CPU_CODE_EG_FILTER_DROP",
	"PPE_CPU_CODE_PRE_ACTION",
	"PPE_CPU_CODE_POST_ACTION",
	"PPE_CPU_CODE_CODE_ACTION",
};

/*
 * nss_ppe_stats_str_sc
 *      PPE statistics strings for service-code stats
 */
static int8_t *nss_ppe_stats_str_sc[NSS_PPE_SC_MAX] = {
	"SC_NONE           ",
	"SC_BYPASS_ALL     ",
	"SC_ADV_QOS_BRIDGED",
	"SC_BR_QOS         ",
	"SC_BNC_0          ",
	"SC_BNC_CMPL_0     ",
	"SC_ADV_QOS_ROUTED ",
	"SC_IPSEC_PPE2EIP  ",
	"SC_IPSEC_EIP2PPE  ",
	"SC_PTP            ",
	"SC_VLAN_FILTER	   ",
};

/*
 * nss_ppe_stats_sync
 *	PPE connection sync statistics from NSS
 */
void nss_ppe_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_ppe_sync_stats_msg *stats_msg, uint16_t if_num)
{
	uint32_t sc;
	spin_lock_bh(&nss_ppe_stats_lock);
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_L3_FLOWS] += stats_msg->stats.nss_ppe_v4_l3_flows;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_L2_FLOWS] += stats_msg->stats.nss_ppe_v4_l2_flows;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_CREATE_REQ] += stats_msg->stats.nss_ppe_v4_create_req;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_CREATE_FAIL] += stats_msg->stats.nss_ppe_v4_create_fail;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_DESTROY_REQ] += stats_msg->stats.nss_ppe_v4_destroy_req;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_DESTROY_FAIL] += stats_msg->stats.nss_ppe_v4_destroy_fail;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_MC_CREATE_REQ] += stats_msg->stats.nss_ppe_v4_mc_create_req;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_MC_CREATE_FAIL] += stats_msg->stats.nss_ppe_v4_mc_create_fail;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_MC_UPDATE_REQ] += stats_msg->stats.nss_ppe_v4_mc_update_req;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_MC_UPDATE_FAIL] += stats_msg->stats.nss_ppe_v4_mc_update_fail;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_MC_DESTROY_REQ] += stats_msg->stats.nss_ppe_v4_mc_destroy_req;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_MC_DESTROY_FAIL] += stats_msg->stats.nss_ppe_v4_mc_destroy_fail;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V4_UNKNOWN_INTERFACE] += stats_msg->stats.nss_ppe_v4_unknown_interface;

	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_L3_FLOWS] += stats_msg->stats.nss_ppe_v6_l3_flows;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_L2_FLOWS] += stats_msg->stats.nss_ppe_v6_l2_flows;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_CREATE_REQ] += stats_msg->stats.nss_ppe_v6_create_req;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_CREATE_FAIL] += stats_msg->stats.nss_ppe_v6_create_fail;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_DESTROY_REQ] += stats_msg->stats.nss_ppe_v6_destroy_req;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_DESTROY_FAIL] += stats_msg->stats.nss_ppe_v6_destroy_fail;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_MC_CREATE_REQ] += stats_msg->stats.nss_ppe_v6_mc_create_req;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_MC_CREATE_FAIL] += stats_msg->stats.nss_ppe_v6_mc_create_fail;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_MC_UPDATE_REQ] += stats_msg->stats.nss_ppe_v6_mc_update_req;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_MC_UPDATE_FAIL] += stats_msg->stats.nss_ppe_v6_mc_update_fail;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_MC_DESTROY_REQ] += stats_msg->stats.nss_ppe_v6_mc_destroy_req;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_MC_DESTROY_FAIL] += stats_msg->stats.nss_ppe_v6_mc_destroy_fail;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_V6_UNKNOWN_INTERFACE] += stats_msg->stats.nss_ppe_v6_unknown_interface;

	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_FAIL_VP_FULL] += stats_msg->stats.nss_ppe_fail_vp_full;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_FAIL_NH_FULL] += stats_msg->stats.nss_ppe_fail_nh_full;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_FAIL_FLOW_FULL] += stats_msg->stats.nss_ppe_fail_flow_full;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_FAIL_HOST_FULL] += stats_msg->stats.nss_ppe_fail_host_full;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_FAIL_PUBIP_FULL] += stats_msg->stats.nss_ppe_fail_pubip_full;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_FAIL_PORT_SETUP] += stats_msg->stats.nss_ppe_fail_port_setup;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_FAIL_RW_FIFO_FULL] += stats_msg->stats.nss_ppe_fail_rw_fifo_full;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_FAIL_FLOW_COMMAND] += stats_msg->stats.nss_ppe_fail_flow_command;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_FAIL_UNKNOWN_PROTO] += stats_msg->stats.nss_ppe_fail_unknown_proto;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_FAIL_PPE_UNRESPONSIVE] += stats_msg->stats.nss_ppe_fail_ppe_unresponsive;
	nss_ppe_debug_stats.conn_stats[NSS_PPE_STATS_FAIL_FQG_FULL] += stats_msg->stats.nss_ppe_fail_fqg_full;

	/*
	 * Update service-code stats.
	 */
	for (sc = 0; sc < NSS_PPE_SC_MAX; sc++) {
		nss_ppe_debug_stats.sc_stats[sc].nss_ppe_sc_cb_unregister += stats_msg->sc_stats[sc].nss_ppe_sc_cb_unregister;
		nss_ppe_debug_stats.sc_stats[sc].nss_ppe_sc_cb_success += stats_msg->sc_stats[sc].nss_ppe_sc_cb_success;
		nss_ppe_debug_stats.sc_stats[sc].nss_ppe_sc_cb_failure += stats_msg->sc_stats[sc].nss_ppe_sc_cb_failure;
	}

	spin_unlock_bh(&nss_ppe_stats_lock);
}

/*
 * nss_ppe_stats_conn_get()
 *	Get PPE connection statistics.
 */
static void nss_ppe_stats_conn_get(uint32_t *stats)
{
	if (!stats) {
		nss_warning("No memory to copy ppe connection stats");
		return;
	}

	/*
	 * Get flow stats
	 */
	spin_lock_bh(&nss_ppe_stats_lock);
	memcpy(stats, nss_ppe_debug_stats.conn_stats, (sizeof(uint32_t) * NSS_PPE_STATS_CONN_MAX));
	spin_unlock_bh(&nss_ppe_stats_lock);
}

/*
 * nss_ppe_stats_sc_get()
 *	Get PPE service-code statistics.
 */
static void nss_ppe_stats_sc_get(struct nss_ppe_sc_stats_debug *sc_stats)
{
	if (!sc_stats) {
		nss_warning("No memory to copy ppe connection stats");
		return;
	}

	/*
	 * Get flow stats
	 */
	spin_lock_bh(&nss_ppe_stats_lock);
	memcpy(sc_stats, nss_ppe_debug_stats.sc_stats, (sizeof(struct nss_ppe_sc_stats_debug) * NSS_PPE_SC_MAX));
	spin_unlock_bh(&nss_ppe_stats_lock);
}

/*
 * nss_ppe_stats_l3_get()
 *	Get PPE L3 debug statistics.
 */
static void nss_ppe_stats_l3_get(uint32_t *stats)
{
	if (!stats) {
		nss_warning("No memory to copy ppe l3 dbg stats\n");
		return;
	}

	spin_lock_bh(&nss_ppe_stats_lock);
	nss_ppe_reg_write(PPE_L3_DBG_WR_OFFSET, PPE_L3_DBG0_OFFSET);
	nss_ppe_reg_read(PPE_L3_DBG_RD_OFFSET, &stats[NSS_PPE_STATS_L3_DBG_0]);

	nss_ppe_reg_write(PPE_L3_DBG_WR_OFFSET, PPE_L3_DBG1_OFFSET);
	nss_ppe_reg_read(PPE_L3_DBG_RD_OFFSET, &stats[NSS_PPE_STATS_L3_DBG_1]);

	nss_ppe_reg_write(PPE_L3_DBG_WR_OFFSET, PPE_L3_DBG2_OFFSET);
	nss_ppe_reg_read(PPE_L3_DBG_RD_OFFSET, &stats[NSS_PPE_STATS_L3_DBG_2]);

	nss_ppe_reg_write(PPE_L3_DBG_WR_OFFSET, PPE_L3_DBG3_OFFSET);
	nss_ppe_reg_read(PPE_L3_DBG_RD_OFFSET, &stats[NSS_PPE_STATS_L3_DBG_3]);

	nss_ppe_reg_write(PPE_L3_DBG_WR_OFFSET, PPE_L3_DBG4_OFFSET);
	nss_ppe_reg_read(PPE_L3_DBG_RD_OFFSET, &stats[NSS_PPE_STATS_L3_DBG_4]);

	nss_ppe_reg_write(PPE_L3_DBG_WR_OFFSET, PPE_L3_DBG_PORT_OFFSET);
	nss_ppe_reg_read(PPE_L3_DBG_RD_OFFSET, &stats[NSS_PPE_STATS_L3_DBG_PORT]);
	spin_unlock_bh(&nss_ppe_stats_lock);
}

/*
 * nss_ppe_stats_code_get()
 *	Get PPE CPU and DROP code for last packet processed.
 */
static void nss_ppe_stats_code_get(uint32_t *stats)
{
	uint32_t drop_0, drop_1, cpu_code;

	nss_trace("%s(%d) Start\n", __func__, __LINE__);
	if (!stats) {
		nss_warning("No memory to copy ppe code\n");
		return;
	}

	spin_lock_bh(&nss_ppe_stats_lock);
	nss_ppe_reg_write(PPE_PKT_CODE_WR_OFFSET, PPE_PKT_CODE_DROP0_OFFSET);
	nss_ppe_reg_read(PPE_PKT_CODE_RD_OFFSET, &drop_0);

	nss_ppe_reg_write(PPE_PKT_CODE_WR_OFFSET, PPE_PKT_CODE_DROP1_OFFSET);
	nss_ppe_reg_read(PPE_PKT_CODE_RD_OFFSET, &drop_1);

	stats[NSS_PPE_STATS_CODE_DROP] = PPE_PKT_CODE_DROP_GET(drop_0, drop_1);

	nss_ppe_reg_write(PPE_PKT_CODE_WR_OFFSET, PPE_PKT_CODE_CPU_OFFSET);
	nss_ppe_reg_read(PPE_PKT_CODE_RD_OFFSET, &cpu_code);

	stats[NSS_PPE_STATS_CODE_CPU] = PPE_PKT_CODE_CPU_GET(cpu_code);

	spin_unlock_bh(&nss_ppe_stats_lock);
}

/*
 * nss_ppe_port_drop_code_get()
 *	Get ppe per port drop code.
 */
static void nss_ppe_port_drop_code_get(uint32_t *stats, uint8_t port_id)
{
	uint8_t i;
	nss_trace("%s(%d) Start\n", __func__, __LINE__);
	if (!stats) {
		nss_warning("No memory to copy ppe code\n");
		return;
	}

	if (port_id > NSS_PPE_NUM_PHY_PORTS_MAX) {
		nss_warning("Port id is out of range\n");
		return;
	}

	spin_lock_bh(&nss_ppe_stats_lock);

	for (i = 0; i < NSS_PPE_STATS_DROP_CODE_MAX; i++) {
		nss_ppe_reg_read(PPE_DROP_CODE_OFFSET(i, port_id), &stats[i]);
	}

	spin_unlock_bh(&nss_ppe_stats_lock);
}

/*
 * nss_ppe_cpu_code_exception_get()
 *	Get ppe cpu code specific for flow exceptions.
 */
static void nss_ppe_cpu_code_exception_get(uint32_t *stats)
{
	uint8_t i;
	nss_trace("%s(%d) Start\n", __func__, __LINE__);
	if (!stats) {
		nss_warning("No memory to copy ppe code\n");
		return;
	}

	spin_lock_bh(&nss_ppe_stats_lock);

	for (i = 0; i < NSS_PPE_STATS_CPU_CODE_EXCEPTION_MAX ; i++) {
		nss_ppe_reg_read(PPE_CPU_CODE_OFFSET(i), &stats[i]);
	}

	spin_unlock_bh(&nss_ppe_stats_lock);
}

/*
 * nss_ppe_cpu_code_nonexception_get()
 *	Get ppe cpu code specific for flow exceptions.
 */
static void nss_ppe_cpu_code_nonexception_get(uint32_t *stats)
{
	uint8_t i;
	nss_trace("%s(%d) Start\n", __func__, __LINE__);
	if (!stats) {
		nss_warning("No memory to copy ppe code\n");
		return;
	}

	spin_lock_bh(&nss_ppe_stats_lock);

	for (i = 0; i < NSS_PPE_STATS_CPU_CODE_NONEXCEPTION_MAX; i++) {
		nss_ppe_reg_read(PPE_CPU_CODE_OFFSET(ppe_cc_nonexception[i]), &stats[i]);
	}

	spin_unlock_bh(&nss_ppe_stats_lock);
}

/*
 * nss_ppe_conn_stats_read()
 *	Read ppe connection statistics
 */
static ssize_t nss_ppe_conn_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int i;
	char *lbuf = NULL;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint32_t ppe_stats[NSS_PPE_STATS_CONN_MAX];
	uint32_t max_output_lines = 2 /* header & footer for session stats */
				+ NSS_PPE_STATS_CONN_MAX /* PPE flow counters */
				+ 2;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;

	lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	memset(ppe_stats, 0, sizeof(uint32_t) * NSS_PPE_STATS_CONN_MAX);

	/*
	 * Get all stats
	 */
	nss_ppe_stats_conn_get(ppe_stats);

	/*
	 * flow stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nppe flow counters start:\n\n");

	for (i = 0; i < NSS_PPE_STATS_CONN_MAX; i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
				"\t%s = %u\n", nss_ppe_stats_str_conn[i],
				ppe_stats[i]);
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nppe flow counters end\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, size_wr);

	kfree(lbuf);
	return bytes_read;
}

/*
 * nss_ppe_sc_stats_read()
 *	Read ppe service code statistics
 */
static ssize_t nss_ppe_sc_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int i;
	char *lbuf = NULL;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	struct nss_ppe_sc_stats_debug sc_stats[NSS_PPE_SC_MAX];
	uint32_t max_output_lines = 2 /* header & footer for sc stats */
				+ NSS_PPE_SC_MAX /* PPE service-code counters */
				+ 2;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;

	lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	memset(sc_stats, 0, sizeof(sc_stats));

	/*
	 * Get stats
	 */
	nss_ppe_stats_sc_get(sc_stats);

	/*
	 * service code stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nppe service code counters start:\n\n");

	for (i = 0; i < NSS_PPE_SC_MAX; i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
				"\t%s \tcb_unregister:%llu    process_ok:%llu    process_fail:%llu\n",
				nss_ppe_stats_str_sc[i], sc_stats[i].nss_ppe_sc_cb_unregister,
				sc_stats[i].nss_ppe_sc_cb_success, sc_stats[i].nss_ppe_sc_cb_failure);
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nppe service code counters end\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, size_wr);

	kfree(lbuf);
	return bytes_read;
}

/*
 * nss_ppe_l3_stats_read()
 *	Read PPE L3 debug statistics
 */
static ssize_t nss_ppe_l3_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int i;
	char *lbuf = NULL;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint32_t ppe_stats[NSS_PPE_STATS_L3_MAX];
	uint32_t max_output_lines = 2 /* header & footer for session stats */
				+ NSS_PPE_STATS_L3_MAX /* PPE flow counters */
				+ 2;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;

	lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(!lbuf)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	memset(ppe_stats, 0, sizeof(uint32_t) * NSS_PPE_STATS_L3_MAX);

	/*
	 * Get all stats
	 */
	nss_ppe_stats_l3_get(ppe_stats);

	/*
	 * flow stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nppe l3 debug stats start:\n\n");

	for (i = 0; i < NSS_PPE_STATS_L3_MAX; i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
				"\t%s = 0x%x\n", nss_ppe_stats_str_l3[i],
				ppe_stats[i]);
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nppe l3 debug stats end\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, size_wr);

	kfree(lbuf);
	return bytes_read;
}

/*
 * nss_ppe_code_stats_read()
 *	Read ppe CPU & DROP code
 */
static ssize_t nss_ppe_code_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int i;
	char *lbuf = NULL;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint32_t ppe_stats[NSS_PPE_STATS_CODE_MAX];
	uint32_t max_output_lines = 2 /* header & footer for session stats */
				+ NSS_PPE_STATS_CODE_MAX /* PPE flow counters */
				+ 2;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;

	lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(!lbuf)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	memset(ppe_stats, 0, sizeof(uint32_t) * NSS_PPE_STATS_CODE_MAX);

	/*
	 * Get all stats
	 */
	nss_ppe_stats_code_get(ppe_stats);

	/*
	 * flow stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nppe session stats start:\n\n");

	for (i = 0; i < NSS_PPE_STATS_CODE_MAX; i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
				"\t%s = %u\n", nss_ppe_stats_str_code[i],
				ppe_stats[i]);
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nppe session stats end\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, size_wr);

	kfree(lbuf);
	return bytes_read;
}

/*
 * nss_ppe_port_dc_stats_read()
 *      Read PPE per port drop code stats
 */
static ssize_t nss_ppe_port_dc_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i;

	/*
	 * max output lines = #stats + 2 start tag line + 2 end tag line + five blank lines
	 */
	uint32_t max_output_lines = (NSS_PPE_STATS_DROP_CODE_MAX + 4) + 5;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	struct nss_stats_data *data = fp->private_data;
	uint32_t *ppe_stats;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	ppe_stats = kzalloc(sizeof(uint32_t) * NSS_PPE_STATS_DROP_CODE_MAX, GFP_KERNEL);
	if (unlikely(ppe_stats == NULL)) {
		kfree(lbuf);
		nss_warning("Could not allocate memory for ppe stats buffer");
		return 0;
	}

	/*
	 * Get drop code counters for specific port
	 */
	nss_ppe_port_drop_code_get(ppe_stats, data->edma_id);
	size_wr = scnprintf(lbuf, size_al, "ppe no drop code stats start:\n\n");
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
				"\t%s = %u\n", nss_ppe_stats_str_dc[0],
				ppe_stats[0]);
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nppe no drop code stats end\n\n");

	/*
	 * Drop code stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "ppe non-zero drop code stats start:\n\n");
	for (i = 1; i < NSS_PPE_STATS_DROP_CODE_MAX; i++) {
		/*
		 * Print only non-zero stats.
		 */
		if (!ppe_stats[i]) {
			continue;
		}

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
				"\t%s = %u\n", nss_ppe_stats_str_dc[i],
				ppe_stats[i]);
	}
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nppe non-zero drop code stats end\n\n");

	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(ppe_stats);
	kfree(lbuf);

	return bytes_read;
}

/*
 * nss_ppe_exception_cc_stats_read()
 *	Read PPE CPU code stats specific to flow exceptions
 */
static ssize_t nss_ppe_exception_cc_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i;

	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = (NSS_PPE_STATS_CPU_CODE_EXCEPTION_MAX + 2) + 3;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint32_t *ppe_stats;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	ppe_stats = kzalloc(sizeof(uint32_t) * NSS_PPE_STATS_CPU_CODE_EXCEPTION_MAX, GFP_KERNEL);
	if (unlikely(ppe_stats == NULL)) {
		kfree(lbuf);
		nss_warning("Could not allocate memory for ppe stats buffer");
		return 0;
	}

	/*
	 * Get CPU code counters for flow specific exceptions
	 */
	nss_ppe_cpu_code_exception_get(ppe_stats);

	size_wr = scnprintf(lbuf, size_al, "ppe non-zero cpu code flow-exception stats start:\n\n");

	/*
	 * CPU code stats
	 */
	for (i = 0; i < NSS_PPE_STATS_CPU_CODE_EXCEPTION_MAX; i++) {
		/*
		 * Print only non-zero stats.
		 */
		if (!ppe_stats[i]) {
			continue;
		}

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
				"\t%s = %u\n", nss_ppe_stats_str_cc[i],
				ppe_stats[i]);
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nppe non-zero cpu code flow-exception stats end\n\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(ppe_stats);
	kfree(lbuf);

	return bytes_read;
}

/*
 * nss_ppe_nonexception_cc_stats_read()
 *      Read PPE CPU code stats for other than flow exceptions
 */
static ssize_t nss_ppe_nonexception_cc_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i;

	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = (NSS_PPE_STATS_CPU_CODE_NONEXCEPTION_MAX + 2) + 3;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint32_t *ppe_stats;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	ppe_stats = kzalloc(sizeof(uint32_t) * NSS_PPE_STATS_CPU_CODE_NONEXCEPTION_MAX, GFP_KERNEL);
	if (unlikely(ppe_stats == NULL)) {
		kfree(lbuf);
		nss_warning("Could not allocate memory for ppe stats buffer");
		return 0;
	}

	/*
	 * Get CPU code counters for non flow exceptions
	 */
	nss_ppe_cpu_code_nonexception_get(ppe_stats);

	/*
	 * CPU code stats
	 */
	size_wr = scnprintf(lbuf, size_al, "ppe non-zero cpu code non-flow exception stats start:\n\n");
	for (i = 0; i < NSS_PPE_STATS_CPU_CODE_NONEXCEPTION_MAX; i++) {
		/*
		 * Print only non-zero stats.
		 */
		if (!ppe_stats[i]) {
			continue;
		}

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
				"\t%s = %u\n", nss_ppe_stats_str_cc[i + NSS_PPE_STATS_CPU_CODE_NONEXCEPTION_START],
				ppe_stats[i]);
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nppe non-zero cpu code non-flow exception stats end\n\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(ppe_stats);
	kfree(lbuf);

	return bytes_read;
}

/*
 * nss_ppe_conn_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(ppe_conn)

/*
 * nss_ppe_l3_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(ppe_l3)

/*
 * nss_ppe_code_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(ppe_code)

/*
 * nss_ppe_port_dc_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(ppe_port_dc)
/*
 *  nss_ppe_exception_cc_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(ppe_exception_cc)

/*
 *  nss_ppe_nonexception_cc_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(ppe_nonexception_cc)

/*
 * nss_ppe_sc_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(ppe_sc)

/*
 * nss_ppe_stats_dentry_create()
 *	Create PPE statistics debug entry.
 */
void nss_ppe_stats_dentry_create(void)
{
	int i;
	struct dentry *ppe_dentry = NULL;
	struct dentry *ppe_conn_d = NULL;
	struct dentry *ppe_sc_d = NULL;
	struct dentry *ppe_l3_d = NULL;
	struct dentry *ppe_ppe_code_d = NULL;
	struct dentry *ppe_code_d = NULL;
	struct dentry *ppe_drop_d = NULL;
	struct dentry *ppe_port_dc_d = NULL;
	struct dentry *ppe_cpu_d = NULL;
	struct dentry *ppe_exception_d = NULL;
	struct dentry *ppe_nonexception_d = NULL;
	char file_name[10];

	ppe_dentry = debugfs_create_dir("ppe", nss_top_main.stats_dentry);
	if (unlikely(ppe_dentry == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/ppe directory");
		return;
	}

	ppe_conn_d = debugfs_create_file("connection", 0400, ppe_dentry,
					&nss_top_main, &nss_ppe_conn_stats_ops);
	if (unlikely(ppe_conn_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/ppe/connection file");
		return;
	}

	ppe_sc_d = debugfs_create_file("sc_stats", 0400, ppe_dentry,
					&nss_top_main, &nss_ppe_sc_stats_ops);
	if (unlikely(ppe_sc_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/ppe/sc_stats file");
		return;
	}

	ppe_l3_d = debugfs_create_file("l3", 0400, ppe_dentry,
					&nss_top_main, &nss_ppe_l3_stats_ops);
	if (unlikely(ppe_l3_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/ppe/l3 filed");
		return;
	}

	ppe_ppe_code_d = debugfs_create_file("ppe_code", 0400, ppe_dentry,
						&nss_top_main, &nss_ppe_code_stats_ops);
	if (unlikely(ppe_ppe_code_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/ppe/ppe_code file");
		return;
	}

	/*
	 * ppe exception and drop code stats
	 */
	ppe_code_d = debugfs_create_dir("code", ppe_dentry);
	if (unlikely(ppe_code_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/ppe/code directory");
		return;
	}

	ppe_cpu_d = debugfs_create_dir("cpu", ppe_code_d);
	if (unlikely(ppe_cpu_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/ppe/code/cpu directory");
		return;
	}

	ppe_exception_d = debugfs_create_file("exception", 0400, ppe_cpu_d,
					&nss_top_main, &nss_ppe_exception_cc_stats_ops);
	if (unlikely(ppe_exception_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/ppe/code/exception file");
		return;
	}

	ppe_nonexception_d = debugfs_create_file("non-exception", 0400, ppe_cpu_d,
					&nss_top_main, &nss_ppe_nonexception_cc_stats_ops);
	if (unlikely(ppe_nonexception_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/ppe/code/non-exception file");
		return;
	}

	ppe_drop_d = debugfs_create_dir("drop", ppe_code_d);
	if (unlikely(ppe_drop_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/ppe/code/drop directory");
		return;
	}

	for (i = 0; i < NSS_PPE_NUM_PHY_PORTS_MAX; i++) {
		if (i > 0) {
			memset(file_name, 0, sizeof(file_name));
			snprintf(file_name, sizeof(file_name), "%d", i);
		}

		ppe_port_dc_d = debugfs_create_file((i == 0) ? "cpu" : file_name, 0400, ppe_drop_d,
					(void *)(nss_ptr_t)i, &nss_ppe_port_dc_stats_ops);
		if (unlikely(ppe_port_dc_d == NULL)) {
			nss_warning("Failed to create qca-nss-drv/stats/ppe/code/drop/%d file", i);
			return;
		}
	}
}
