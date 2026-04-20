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

/**
 * @addtogroup ppe_drv_cc_subsystem
 * @{
 */

#ifndef _PPE_DRV_CC_H_
#define _PPE_DRV_CC_H_

struct ppe_drv;

/*
 * ppe_drv_cc_type
 *	PPE CPU codes
 */
typedef enum ppe_drv_cc_type {
	PPE_DRV_CC_UNKNOWN_L2_PROT			= 1,	/**< CPU Code For Unknown L2 Prot */
	PPE_DRV_CC_PPPOE_WRONG_VER_TYPE			= 2,	/**< CPU Code For Pppoe Wrong Ver Type */
	PPE_DRV_CC_PPPOE_WRONG_CODE			= 3,	/**< CPU Code For Pppoe Wrong Code */
	PPE_DRV_CC_PPPOE_UNSUPPORTED_PPP_PROT		= 4,	/**< CPU Code For Pppoe Unsupported Ppp Prot */
	PPE_DRV_CC_IPV4_WRONG_VER			= 5,	/**< CPU Code For Ipv4 Wrong Ver */
	PPE_DRV_CC_IPV4_SMALL_IHL			= 6,	/**< CPU Code For Ipv4 Small Ihl */
	PPE_DRV_CC_IPV4_WITH_OPTION			= 7,	/**< CPU Code For Ipv4 With Option */
	PPE_DRV_CC_IPV4_HDR_INCOMPLETE			= 8,	/**< CPU Code For Ipv4 Hdr Incomplete */
	PPE_DRV_CC_IPV4_BAD_TOTAL_LEN			= 9,	/**< CPU Code For Ipv4 Bad Total Len */
	PPE_DRV_CC_IPV4_DATA_INCOMPLETE			= 10,	/**< CPU Code For Ipv4 Data Incomplete */
	PPE_DRV_CC_IPV4_FRAG				= 11,	/**< CPU Code For Ipv4 Frag */
	PPE_DRV_CC_IPV4_PING_OF_DEATH			= 12,	/**< CPU Code For Ipv4 Ping Of Death */
	PPE_DRV_CC_IPV4_SMALL_TTL			= 13,	/**< CPU Code For Ipv4 Small Ttl */
	PPE_DRV_CC_IPV4_UNK_IP_PROT			= 14,	/**< CPU Code For Ipv4 Unk Ip Prot */
	PPE_DRV_CC_IPV4_CHECKSUM_ERR			= 15,	/**< CPU Code For Ipv4 Checksum Err */
	PPE_DRV_CC_IPV4_INV_SIP				= 16,	/**< CPU Code For Ipv4 Inv Sip */
	PPE_DRV_CC_IPV4_INV_DIP				= 17,	/**< CPU Code For Ipv4 Inv Dip */
	PPE_DRV_CC_IPV4_LAND_ATTACK			= 18,	/**< CPU Code For Ipv4 Land Attack */
	PPE_DRV_CC_IPV4_AH_HDR_INCOMPLETE		= 19,	/**< CPU Code For Ipv4 Ah Hdr Incomplete */
	PPE_DRV_CC_IPV4_AH_HDR_CROSS_BORDER		= 20,	/**< CPU Code For Ipv4 Ah Hdr Cross Border */
	PPE_DRV_CC_IPV4_ESP_HDR_INCOMPLETE		= 21,	/**< CPU Code For Ipv4 Esp Hdr Incomplete */
	PPE_DRV_CC_IPV6_WRONG_VER			= 22,	/**< CPU Code For Ipv6 Wrong Ver */
	PPE_DRV_CC_IPV6_HDR_INCOMPLETE			= 23,	/**< CPU Code For Ipv6 Hdr Incomplete */
	PPE_DRV_CC_IPV6_BAD_PAYLOAD_LEN			= 24,	/**< CPU Code For Ipv6 Bad Payload Len */
	PPE_DRV_CC_IPV6_DATA_INCOMPLETE			= 25,	/**< CPU Code For Ipv6 Data Incomplete */
	PPE_DRV_CC_IPV6_WITH_EXT_HDR			= 26,	/**< CPU Code For Ipv6 With Ext Hdr */
	PPE_DRV_CC_IPV6_SMALL_HOP_LIMIT			= 27,	/**< CPU Code For Ipv6 Small Hop Limit */
	PPE_DRV_CC_IPV6_INV_SIP				= 28,	/**< CPU Code For Ipv6 Inv Sip */
	PPE_DRV_CC_IPV6_INV_DIP				= 29,	/**< CPU Code For Ipv6 Inv Dip */
	PPE_DRV_CC_IPV6_LAND_ATTACK			= 30,	/**< CPU Code For Ipv6 Land Attack */
	PPE_DRV_CC_IPV6_FRAG				= 31,	/**< CPU Code For Ipv6 Frag */
	PPE_DRV_CC_IPV6_PING_OF_DEATH			= 32,	/**< CPU Code For Ipv6 Ping Of Death */
	PPE_DRV_CC_IPV6_WITH_MORE_EXT_HDR		= 33,	/**< CPU Code For Ipv6 With More Ext Hdr */
	PPE_DRV_CC_IPV6_UNK_LAST_NEXT_HDR		= 34,	/**< CPU Code For Ipv6 Unk Last Next Hdr */
	PPE_DRV_CC_IPV6_MOBILITY_HDR_INCOMPLETE		= 35,	/**< CPU Code For Ipv6 Mobility Hdr Incomplete */
	PPE_DRV_CC_IPV6_MOBILITY_HDR_CROSS_BORDER	= 36,	/**< CPU Code For Ipv6 Mobility Hdr Cross Border */
	PPE_DRV_CC_IPV6_AH_HDR_INCOMPLETE		= 37,	/**< CPU Code For Ipv6 Ah Hdr Incomplete */
	PPE_DRV_CC_IPV6_AH_HDR_CROSS_BORDER		= 38,	/**< CPU Code For Ipv6 Ah Hdr Cross Border */
	PPE_DRV_CC_IPV6_ESP_HDR_INCOMPLETE		= 39,	/**< CPU Code For Ipv6 Esp Hdr Incomplete */
	PPE_DRV_CC_IPV6_ESP_HDR_CROSS_BORDER		= 40,	/**< CPU Code For Ipv6 Esp Hdr Cross Border */
	PPE_DRV_CC_IPV6_OTHER_EXT_HDR_INCOMPLETE	= 41,	/**< CPU Code For Ipv6 Other Ext Hdr Incomplete */
	PPE_DRV_CC_IPV6_OTHER_EXT_HDR_CROSS_BORDER	= 42,	/**< CPU Code For Ipv6 Other Ext Hdr Cross Border */
	PPE_DRV_CC_TCP_HDR_INCOMPLETE			= 43,	/**< CPU Code For Tcp Hdr Incomplete */
	PPE_DRV_CC_TCP_HDR_CROSS_BORDER			= 44,	/**< CPU Code For Tcp Hdr Cross Border */
	PPE_DRV_CC_TCP_SAME_SP_DP			= 45,	/**< CPU Code For Tcp Same Sp Dp */
	PPE_DRV_CC_TCP_SMALL_DATA_OFFSET		= 46,	/**< CPU Code For Tcp Small Data Offset */
	PPE_DRV_CC_TCP_FLAGS_0				= 47,	/**< CPU Code For Tcp Flags 0 */
	PPE_DRV_CC_TCP_FLAGS_1				= 48,	/**< CPU Code For Tcp Flags 1 */
	PPE_DRV_CC_TCP_FLAGS_2				= 49,	/**< CPU Code For Tcp Flags 2 */
	PPE_DRV_CC_TCP_FLAGS_3				= 50,	/**< CPU Code For Tcp Flags 3 */
	PPE_DRV_CC_TCP_FLAGS_4				= 51,	/**< CPU Code For Tcp Flags 4 */
	PPE_DRV_CC_TCP_FLAGS_5				= 52,	/**< CPU Code For Tcp Flags 5 */
	PPE_DRV_CC_TCP_FLAGS_6				= 53,	/**< CPU Code For Tcp Flags 6 */
	PPE_DRV_CC_TCP_FLAGS_7				= 54,	/**< CPU Code For Tcp Flags 7 */
	PPE_DRV_CC_TCP_CHECKSUM_ERR			= 55,	/**< CPU Code For Tcp Checksum Err */
	PPE_DRV_CC_UDP_HDR_INCOMPLETE			= 56,	/**< CPU Code For Udp Hdr Incomplete */
	PPE_DRV_CC_UDP_HDR_CROSS_BORDER			= 57,	/**< CPU Code For Udp Hdr Cross Border */
	PPE_DRV_CC_UDP_SAME_SP_DP			= 58,	/**< CPU Code For Udp Same Sp Dp */
	PPE_DRV_CC_UDP_BAD_LEN				= 59,	/**< CPU Code For Udp Bad Len */
	PPE_DRV_CC_UDP_DATA_INCOMPLETE			= 60,	/**< CPU Code For Udp Data Incomplete */
	PPE_DRV_CC_UDP_CHECKSUM_ERR			= 61,	/**< CPU Code For Udp Checksum Err */
	PPE_DRV_CC_UDP_LITE_HDR_INCOMPLETE		= 62,	/**< CPU Code For Udp Lite Hdr Incomplete */
	PPE_DRV_CC_UDP_LITE_HDR_CROSS_BORDER		= 63,	/**< CPU Code For Udp Lite Hdr Cross Border */
	PPE_DRV_CC_UDP_LITE_SAME_SP_DP			= 64,	/**< CPU Code For Udp Lite Same Sp Dp */
	PPE_DRV_CC_UDP_LITE_CSM_COV_1_TO_7		= 65,	/**< CPU Code For Udp Lite Csm Cov 1 To 7 */
	PPE_DRV_CC_UDP_LITE_CSM_COV_TOO_LONG		= 66,	/**< CPU Code For Udp Lite Csm Cov Too Long */
	PPE_DRV_CC_UDP_LITE_CSM_COV_CROSS_BORDER	= 67,	/**< CPU Code For Udp Lite Csm Cov Cross Border */
	PPE_DRV_CC_UDP_LITE_CHECKSUM_ERR	        = 68,	/**< CPU Code For Udp Lite Checksum Err */
	PPE_DRV_CC_FAKE_L2_PROT_ERR			= 69,	/**< CPU Code For Fake L2 Prot Err */
	PPE_DRV_CC_FAKE_MAC_HEADER_ERR			= 70,	/**< CPU Code For Fake Mac Header Err */
	PPE_DRV_CC_BITMAP_MAX				= 78,	/**< CPU Code For Bitmap Max */
	PPE_DRV_CC_L2_EXP_MRU_FAIL			= 79,	/**< L2 CPU Code For Mru Fail */
	PPE_DRV_CC_L2_EXP_MTU_FAIL			= 80,	/**< L2 CPU Code For Mtu Fail */
	PPE_DRV_CC_L3_EXP_IP_PREFIX_BC			= 81,	/**< L3 CPU Code For Ip Prefix Bc */
	PPE_DRV_CC_L3_EXP_MTU_FAIL			= 82,	/**< L3 CPU Code For Mtu Fail */
	PPE_DRV_CC_L3_EXP_MRU_FAIL			= 83,	/**< L3 CPU Code For Mru Fail */
	PPE_DRV_CC_L3_EXP_ICMP_RDT			= 84,	/**< L3 CPU Code For Icmp Rdt */
	PPE_DRV_CC_L3_EXP_IP_RT_TTL1_TO_ME		= 85,	/**< L3 CPU Code For Ip Rt Ttl1 To Me */
	PPE_DRV_CC_L3_EXP_IP_RT_TTL_ZERO		= 86,	/**< L3 CPU Code For Ip Rt Ttl Zero */
	PPE_DRV_CC_L3_FLOW_SERVICE_CODE_LOOP		= 87,	/**< L3 Flow Service Code Loop */
	PPE_DRV_CC_L3_FLOW_DE_ACCELEARTE		= 88,	/**< L3 Flow De Accelearte */
	PPE_DRV_CC_L3_EXP_FLOW_SRC_IF_CHK_FAIL		= 89,	/**< L3 CPU Code For Flow Src If Chk Fail */
	PPE_DRV_CC_L3_FLOW_SYNC_TOGGLE_MISMATCH		= 90,	/**< L3 Flow Sync Toggle Mismatch */
	PPE_DRV_CC_L3_EXP_MTU_DF_FAIL			= 91,	/**< L3 CPU Code For Mtu Df Fail */
	PPE_DRV_CC_L3_EXP_PPPOE_MULTICAST		= 92,	/**< L3 CPU Code For Pppoe Multicast */
	PPE_DRV_CC_L3_EXP_FLOW_MTU_FAIL			= 93,	/**< L3 CPU Code For Flow Mtu Fail */
	PPE_DRV_CC_L3_EXP_FLOW_MTU_DF_FAIL		= 94,	/**< L3 CPU Code For Flow Mtu Df Fail */
	PPE_DRV_CC_L3_UDP_CHECKSUM_0_EXP		= 95,	/**< L3 Udp Checksum 0 CPU Code For */
	PPE_DRV_CC_MGMT_OFFSET				= 96,	/**< Mgmt Offset */
	PPE_DRV_CC_MGMT_EAPOL				= 97,	/**< Mgmt Eapol */
	PPE_DRV_CC_MGMT_PPPOE_DIS			= 98,	/**< Mgmt Pppoe Dis */
	PPE_DRV_CC_MGMT_IGMP				= 99,	/**< Mgmt Igmp */
	PPE_DRV_CC_MGMT_ARP_REQ				= 100,	/**< Mgmt Arp Req */
	PPE_DRV_CC_MGMT_ARP_REP				= 101,	/**< Mgmt Arp Rep */
	PPE_DRV_CC_MGMT_DHCPv4				= 102,	/**< Mgmt Dhcpv4 */
	PPE_DRV_CC_MGMT_LINKOAM				= 104,	/**< Mgmt Linkoam */
	PPE_DRV_CC_MGMT_MLD				= 107,	/**< Mgmt Mld */
	PPE_DRV_CC_MGMT_NS				= 108,	/**< Mgmt Ns */
	PPE_DRV_CC_MGMT_NA				= 109,	/**< Mgmt Na */
	PPE_DRV_CC_MGMT_DHCPv6				= 110,	/**< Mgmt Dhcpv6 */
	PPE_DRV_CC_PTP_OFFSET				= 112,	/**< Ptp Offset */
	PPE_DRV_CC_PTP_SYNC				= 113,	/**< Ptp Sync */
	PPE_DRV_CC_PTP_FOLLOW_UP			= 114,	/**< Ptp Follow Up */
	PPE_DRV_CC_PTP_DELAY_REQ			= 115,	/**< Ptp Delay Req */
	PPE_DRV_CC_PTP_DELAY_RESP			= 116,	/**< Ptp Delay Resp */
	PPE_DRV_CC_PTP_PDELAY_REQ			= 117,	/**< Ptp Pdelay Req */
	PPE_DRV_CC_PTP_PDELAY_RESP			= 118,	/**< Ptp Pdelay Resp */
	PPE_DRV_CC_PTP_PDELAY_RESP_FOLLOW_UP		= 119,	/**< Ptp Pdelay Resp Follow Up */
	PPE_DRV_CC_PTP_ANNONCE				= 120,	/**< Ptp Annonce */
	PPE_DRV_CC_PTP_MANAGEMENT			= 121,	/**< Ptp Management */
	PPE_DRV_CC_PTP_SIGNALING			= 122,	/**< Ptp Signaling */
	PPE_DRV_CC_PTP_PKT_RSV_MSG			= 127,	/**< Ptp Pkt Rsv Msg */
	PPE_DRV_CC_IPV4_SG_UNKNOWN			= 136,	/**< Ipv4 Sg Unknown */
	PPE_DRV_CC_IPV6_SG_UNKNOWN			= 137,	/**< Ipv6 Sg Unknown */
	PPE_DRV_CC_ARP_SG_UNKNOWN			= 138,	/**< Arp Sg Unknown */
	PPE_DRV_CC_ND_SG_UNKNOWN			= 139,	/**< Nd Sg Unknown */
	PPE_DRV_CC_IPV4_SG_VIO				= 140,	/**< Ipv4 Sg Vio */
	PPE_DRV_CC_IPV6_SG_VIO				= 141,	/**< Ipv6 Sg Vio */
	PPE_DRV_CC_ARP_SG_VIO				= 142,	/**< Arp Sg Vio */
	PPE_DRV_CC_ND_SG_VIO				= 143,	/**< Nd Sg Vio */
	PPE_DRV_CC_L3_ROUTING_HOST_MISMATCH		= 148,	/**< L3 Routing Host Mismatch */
	PPE_DRV_CC_L3_FLOW_SNAT_ACTION			= 149,	/**< L3 Flow Snat Action */
	PPE_DRV_CC_L3_FLOW_DNAT_ACTION			= 150,	/**< L3 Flow Dnat Action */
	PPE_DRV_CC_L3_FLOW_RT_ACTION			= 151,	/**< L3 Flow Rt Action */
	PPE_DRV_CC_L3_FLOW_BR_ACTION			= 152,	/**< L3 Flow Br Action */
	PPE_DRV_CC_L3_MC_BRIDGE_ACTION			= 153,	/**< L3 Mc Bridge Action */
	PPE_DRV_CC_L3_ROUTE_PREHEAD_RT_ACTION		= 154,	/**< L3 Route Prehead Rt Action */
	PPE_DRV_CC_L3_ROUTE_PREHEAD_SNAPT_ACTION	= 155,	/**< L3 Route Prehead Snapt Action */
	PPE_DRV_CC_L3_ROUTE_PREHEAD_DNAPT_ACTION	= 156,	/**< L3 Route Prehead Dnapt Action */
	PPE_DRV_CC_L3_ROUTE_PREHEAD_SNAT_ACTION		= 157,	/**< L3 Route Prehead Snat Action */
	PPE_DRV_CC_L3_ROUTE_PREHEAD_DNAT_ACTION		= 158,	/**< L3 Route Prehead Dnat Action */
	PPE_DRV_CC_L3_NO_ROUTE_PREHEAD_NAT_ACTION	= 159,	/**< L3 No Route Prehead Nat Action */
	PPE_DRV_CC_L3_NO_ROUTE_PREHEAD_NAT_ERROR	= 160,	/**< L3 No Route Prehead Nat Error */
	PPE_DRV_CC_L3_ROUTE_ACTION			= 161,	/**< L3 Route Action */
	PPE_DRV_CC_L3_NO_ROUTE_ACTION			= 162,	/**< L3 No Route Action */
	PPE_DRV_CC_L3_NO_ROUTE_NH_INVALID_ACTION	= 163,	/**< L3 No Route Nh Invalid Action */
	PPE_DRV_CC_L3_NO_ROUTE_PREHEAD_ACTION		= 164,	/**< L3 No Route Prehead Action */
	PPE_DRV_CC_L3_BRIDGE_ACTION			= 165,	/**< L3 Bridge Action */
	PPE_DRV_CC_L3_FLOW_ACTION			= 166,	/**< L3 Flow Action */
	PPE_DRV_CC_L3_FLOW_MISS_ACTION			= 167,	/**< L3 Flow Miss Action */
	PPE_DRV_CC_L2_NEW_MAC_ADDRESS			= 168,	/**< L2 New Mac Address */
	PPE_DRV_CC_L2_HASH_COLLOSION			= 169,	/**< L2 Hash Collosion */
	PPE_DRV_CC_L2_STATION_MOVE			= 170,	/**< L2 Station Move */
	PPE_DRV_CC_L2_LEARN_LIMIT			= 171,	/**< L2 Learn Limit */
	PPE_DRV_CC_L2_SA_LOOKUP_ACTION			= 172,	/**< L2 Sa Lookup Action */
	PPE_DRV_CC_L2_DA_LOOKUP_ACTION			= 173,	/**< L2 Da Lookup Action */
	PPE_DRV_CC_APP_CTRL_ACTION			= 174,	/**< App Ctrl Action */
	PPE_DRV_CC_IN_VLAN_FILTER_ACTION		= 175,	/**< In Vlan Filter Action */
	PPE_DRV_CC_IN_VLAN_XLT_MISS			= 176,	/**< In Vlan Xlt Miss */
	PPE_DRV_CC_EG_VLAN_FILTER_DROP			= 177,	/**< Eg Vlan Filter Drop */
	PPE_DRV_CC_ACL_PRE_ACTION			= 178,	/**< Acl Pre Action */
	PPE_DRV_CC_ACL_POST_ACTION			= 179,	/**< Acl Post Action */
	PPE_DRV_CC_SERVICE_CODE_ACTION			= 180,	/**< Service Code Action */
	PPE_DRV_CC_L3_ROUTE_PRE_IPO_RT_ACTION		= 181,	/**< L3 Route Pre Ipo Rt Action */
	PPE_DRV_CC_L3_ROUTE_PRE_IPO_SNAPT_ACTION	= 182,	/**< L3 Route Pre Ipo Snapt Action */
	PPE_DRV_CC_L3_ROUTE_PRE_IPO_DNAPT_ACTION	= 183,	/**< L3 Route Pre Ipo Dnapt Action */
	PPE_DRV_CC_L3_ROUTE_PRE_IPO_SNAT_ACTION		= 184,	/**< L3 Route Pre Ipo Snat Action */
	PPE_DRV_CC_L3_ROUTE_PRE_IPO_DNAT_ACTION		= 185,	/**< L3 Route Pre Ipo Dnat Action */
	PPE_DRV_CC_TL_EXP_IF_CHECK_FAIL			= 186,	/**< Tl CPU Code For If Check Fail */
	PPE_DRV_CC_TL_EXP_VLAN_CHECK_FAIL		= 187,	/**< Tl CPU Code For Vlan Check Fail */
	PPE_DRV_CC_TL_EXP_PPPOE_MC_TERM			= 188,	/**< Tl CPU Code For Pppoe Mc Term */
	PPE_DRV_CC_TL_EXP_DE_ACCE			= 189,	/**< Tl CPU Code For De Acce */
	PPE_DRV_CC_TL_UDP_CSUM_ZERO			= 190,	/**< Tl Udp Csum Zero */
	PPE_DRV_CC_TL_TTL_EXCEED			= 191,	/**< Tl Ttl Exceed */
	PPE_DRV_CC_TL_EXP_LPM_IF_CHECK_FAIL		= 192,	/**< Tl CPU Code For Lpm If Check Fail */
	PPE_DRV_CC_TL_EXP_LPM_VLAN_CHECK_FAIL		= 193,	/**< Tl CPU Code For Lpm Vlan Check Fail */
	PPE_DRV_CC_TL_EXP_MAP_SRC_CHECK_FAIL		= 194,	/**< Tl CPU Code For Map Src Check Fail */
	PPE_DRV_CC_TL_EXP_MAP_DST_CHECK_FAIL		= 195,	/**< Tl CPU Code For Map Dst Check Fail */
	PPE_DRV_CC_TL_EXP_MAP_UDP_CSUM_ZERO		= 196,	/**< Tl CPU Code For Map Udp Csum Zero */
	PPE_DRV_CC_TL_EXP_MAP_NON_TCP_UDP		= 197,	/**< Tl CPU Code For Map Non Tcp Udp */
	PPE_DRV_CC_TL_FWD_CMD				= 198,	/**< Tl Fwd Cmd */
	PPE_DRV_CC_L2_PRE_IPO_ACTION			= 210,	/**< L2 Pre Ipo Action */
	PPE_DRV_CC_L2_TUNL_CONTEXT_INVALID		= 211,	/**< L2 Tunl Context Invalid */
	PPE_DRV_CC_RESERVE0				= 212,	/**< CPU Code For Reserve0 */
	PPE_DRV_CC_RESERVE1				= 213,	/**< CPU Code For Reserve1 */
	PPE_DRV_CC_TUNNEL_DECAP_ECN			= 214,	/**< CPU Code For Tunnel Decap Ecn */
	PPE_DRV_CC_INNER_PACKET_TOO_SHORT		= 215,	/**< CPU Code For Inner Packet Too Short */
	PPE_DRV_CC_VXLAN_HDR				= 216,	/**< CPU Code For Vxlan Hdr */
	PPE_DRV_CC_VXLAN_GPE_HDR			= 217,	/**< CPU Code For Vxlan Gpe Hdr */
	PPE_DRV_CC_GENEVE_HDR				= 218,	/**< CPU Code For Geneve Hdr */
	PPE_DRV_CC_GRE_HDR				= 219,	/**< CPU Code For Gre Hdr */
	PPE_DRV_CC_RESERVED				= 220,	/**< CPU Code For Reserved */
	PPE_DRV_CC_UNKNOWN_INNER_TYPE			= 221,	/**< CPU Code For Unknown Inner Type */
	PPE_DRV_CC_FLAG_VXLAN				= 222,	/**< CPU Code For Flag Vxlan */
	PPE_DRV_CC_FLAG_VXLAN_GPE			= 223,	/**< CPU Code For Flag Vxlan Gpe */
	PPE_DRV_CC_FLAG_GRE				= 224,	/**< CPU Code For Flag Gre */
	PPE_DRV_CC_FLAG_GENEVE				= 225,	/**< CPU Code For Flag Geneve */
	PPE_DRV_CC_PROGRAM0				= 226,	/**< CPU Code For Program0 */
	PPE_DRV_CC_PROGRAM1				= 227,	/**< CPU Code For Program1 */
	PPE_DRV_CC_PROGRAM2				= 228,	/**< CPU Code For Program2 */
	PPE_DRV_CC_PROGRAM3				= 229,	/**< CPU Code For Program3 */
	PPE_DRV_CC_PROGRAM4				= 230,	/**< CPU Code For Program4 */
	PPE_DRV_CC_PROGRAM5				= 231,	/**< CPU Code For Program5 */
	PPE_DRV_CC_CPU_CODE_EG_MIRROR			= 254,	/**< Cpu Code Eg Mirror */
	PPE_DRV_CC_CPU_CODE_IN_MIRROR			= 255,	/**< Cpu Code In Mirror */
	PPE_DRV_CC_MAX						/**< Max */
} ppe_drv_cc_t;

#define PPE_DRV_CC_RANGE1			PPE_DRV_CC_UDP_LITE_CHECKSUM_ERR	/* CPU Code For Udp Lite Checksum Err */
#define PPE_DRV_CC_RANGE2			PPE_DRV_CC_RESERVE0			/* PPE_DRV_CC_RESERVE0 */
#define PPE_DRV_CC_RANGE3			PPE_DRV_CC_INNER_PACKET_TOO_SHORT	/* CPU Code For Inner Packet Too Short */
#define PPE_DRV_EXP_RANGE1_BASE			1
#define PPE_DRV_EXP_RANGE2_BASE			144

/**
 * ppe_drv_cc_metadata
 *	Metadata for CPU codes.
 */
struct ppe_drv_cc_metadata {
	uint16_t cpu_code;			/**< CPU code for the packet */
	uint16_t acl_hw_index;			/**< Hardware ACL index */
	bool acl_index_valid;			/**< ACL rule Valid bit */
	bool fake_mac;				/**< Packet with fake MAC header */
};

typedef bool (*ppe_drv_cc_callback_t)(void *app_data, struct sk_buff *skb, void *cc_info);

/*
 * ppe_drv_cc_process_skbuff()
 *	Register callback for a specific CPU code
 *
 * @param[IN] cc_info		CPU code metadata.
 * @param[IN] skb		Socket buffer with CPU code.
 *
 * @return
 * true if packet is consumed by the API or false if the packet is not consumed.
 */
extern bool ppe_drv_cc_process_skbuff(struct ppe_drv_cc_metadata *cc_info, struct sk_buff *skb);

/*
 * ppe_drv_cc_unregister_cb()
 *	Unregister callback for a specific CPU code
 *
 * @param[IN] cc   CPU code number.
 *
 * @return
 * void
 */
extern void ppe_drv_cc_unregister_cb(ppe_drv_cc_t cc);

/*
 * ppe_drv_cc_register_cb()
 *	Register callback for a specific CPU code
 *
 * @param[IN] cc   Service code number.
 * @param[IN] cb   Callback API.
 * @param[IN] app_data   Application data to be passed to callback.
 *
 * @return
 * void
 */
extern void ppe_drv_cc_register_cb(ppe_drv_cc_t cc, ppe_drv_cc_callback_t cb, void *app_data);

/** @} */ /* end_addtogroup ppe_drv_cc_subsystem */

#endif /* _PPE_DRV_CC_H_ */

