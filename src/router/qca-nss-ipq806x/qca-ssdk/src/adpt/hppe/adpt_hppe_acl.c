/*
 * Copyright (c) 2016-2018, 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @defgroup
 * @{
 */
#include "sw.h"
#include "adpt.h"
#include "adpt_hppe.h"
#include "hppe_acl_reg.h"
#include "hppe_acl.h"
#include <linux/list.h>
#include "adpt_hppe_acl.h"
#if defined(APPE)
#include "adpt_appe_acl.h"
#endif

#define ADPT_ACL_HPPE_MAC_DA_RULE 0
#define ADPT_ACL_HPPE_MAC_SA_RULE 1
#define ADPT_ACL_HPPE_VLAN_RULE 2
#define ADPT_ACL_HPPE_L2_MISC_RULE 3
#define ADPT_ACL_HPPE_IPV4_DIP_RULE 4
#define ADPT_ACL_HPPE_IPV4_SIP_RULE 5
#define ADPT_ACL_HPPE_IPV6_DIP0_RULE 6
#define ADPT_ACL_HPPE_IPV6_DIP1_RULE 7
#define ADPT_ACL_HPPE_IPV6_DIP2_RULE 8
#define ADPT_ACL_HPPE_IPV6_SIP0_RULE 9
#define ADPT_ACL_HPPE_IPV6_SIP1_RULE 10
#define ADPT_ACL_HPPE_IPV6_SIP2_RULE 11
#define ADPT_ACL_HPPE_IPMISC_RULE 12

#if defined(MPPE)
#define ADPT_ACL_HW_LIST_NUM 16
#else
#define ADPT_ACL_HW_LIST_NUM 64
#endif
#define ADPT_ACL_ENTRY_NUM_PER_LIST 8 /* hw rule entries number per hw list */

#if defined(MPPE)
#define ADPT_ACL_SW_LIST_NUM 256
#elif defined(APPE)
#define ADPT_ACL_SW_LIST_NUM 1024
#else
#define ADPT_ACL_SW_LIST_NUM 512
#endif
#define ADPT_ACL_RULE_NUM_PER_LIST 8 /* can change this MACRO to support more rules per ACL list */

typedef struct{
	struct list_head list;
	a_uint16_t rule_id;
	a_uint8_t rule_type[2];
	a_uint8_t rule_hw_entry;
	a_uint8_t rule_hw_list_id;
	a_uint8_t ext1_val;
	a_uint8_t ext2_val;
	a_uint8_t ext4_val;
}ADPT_HPPE_ACL_SW_RULE;

typedef struct{
	struct list_head list;
	struct list_head list_sw_rule;
	a_uint32_t list_pri;
	a_uint16_t list_id;
}ADPT_HPPE_ACL_SW_LIST;

typedef struct{
	struct list_head list_sw_list;
}ADPT_HPPE_ACL_SW_LIST_HEAD;

typedef struct{
	a_bool_t hw_list_valid;
	a_uint8_t hw_list_id;
	a_uint8_t free_hw_entry_bitmap;
	a_uint8_t free_hw_entry_count;
}ADPT_HPPE_ACL_HW_LIST;

#if defined(APPE)
#define VLAN_RULE_SPCP_0_LEN 1
#define VLAN_RULE_SPCP_MASK_0_LEN 1
#endif

#if defined(CONFIG_CPU_BIG_ENDIAN)
typedef struct{
	a_uint32_t mac_addr_0:32;

	a_uint32_t _reserved0:11;
	a_uint32_t is_fake_mac_header:1;
	a_uint32_t is_snap:1;
	a_uint32_t is_ethernet:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t is_ip:1;
	a_uint32_t mac_addr_1:16;
}ADPT_HPPE_ACL_MAC_RULE;
typedef struct{
	a_uint32_t mac_addr_mask_0:32;

	a_uint32_t _reserved0:11;
	a_uint32_t is_fake_mac_header_mask:1;
	a_uint32_t is_snap_mask:1;
	a_uint32_t is_ethernet_mask:1;
	a_uint32_t is_ipv6_mask:1;
	a_uint32_t is_ip_mask:1;
	a_uint32_t mac_addr_mask_1:16;
}ADPT_HPPE_ACL_MAC_RULE_MASK;

typedef struct{
#if defined(APPE)
	a_uint32_t spcp_0:1;
#endif
	a_uint32_t svid:12;
	a_uint32_t cdei:1;
	a_uint32_t cpcp:3;
#if defined(APPE)
	a_uint32_t reserved:3;
#else
	a_uint32_t reserved:4;
#endif
	a_uint32_t cvid:12;/*it is min cvid when range is enable*/

	a_uint32_t _reserved0:11;
	a_uint32_t is_fake_mac_header:1;
	a_uint32_t is_snap:1;
	a_uint32_t is_ethernet:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t is_ip:1;
	a_uint32_t vsi_valid:1;
#if defined(APPE)
	a_uint32_t vsi:6;
#else
	a_uint32_t vsi:5;
#endif
	a_uint32_t stag_fmt:3;
	a_uint32_t ctag_fmt:3;
	a_uint32_t sdei:1;
#if defined(APPE)
	a_uint32_t spcp_1:2;
#else
	a_uint32_t spcp:3;
#endif
}ADPT_HPPE_ACL_VLAN_RULE;

typedef struct{
#if defined(APPE)
	a_uint32_t spcp_mask_0:1;
#endif
	a_uint32_t svid_mask:12;
	a_uint32_t cdei_mask:1;
	a_uint32_t cpcp_mask:3;
#if defined(APPE)
	a_uint32_t reserved:3;
#else
	a_uint32_t reserved:4;
#endif
	a_uint32_t cvid_mask:12;/*it is max cvid when range is enable*/

	a_uint32_t _reserved0:11;
	a_uint32_t is_fake_mac_header_mask:1;
	a_uint32_t is_snap_mask:1;
	a_uint32_t is_ethernet_mask:1;
	a_uint32_t is_ipv6_mask:1;
	a_uint32_t is_ip_mask:1;
	a_uint32_t vsi_valid_mask:1;
#if defined(APPE)
	a_uint32_t vsi_mask:6;
#else
	a_uint32_t vsi_mask:5;
#endif
	a_uint32_t stag_fmt_mask:3;
	a_uint32_t ctag_fmt_mask:3;
	a_uint32_t sdei_mask:1;
#if defined(APPE)
	a_uint32_t spcp_mask_1:2;
#else
	a_uint32_t spcp_mask:3;
#endif
}ADPT_HPPE_ACL_VLAN_RULE_MASK;

typedef struct{
	a_uint32_t l2prot:16;
	a_uint32_t reserved:4;
	a_uint32_t svid:12;/*it is min svid when range is enable*/

	a_uint32_t _reserved0:11;
	a_uint32_t is_fake_mac_header:1;
	a_uint32_t is_snap:1;
	a_uint32_t is_ethernet:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t is_ip:1;
	a_uint32_t pppoe_sessionid:16;
}ADPT_HPPE_ACL_L2MISC_RULE;

typedef struct{
	a_uint32_t l2prot_mask:16;
	a_uint32_t reserved:4;
	a_uint32_t svid_mask:12;/*it is max svid when range is enable*/

	a_uint32_t _reserved0:11;
	a_uint32_t is_fake_mac_header_mask:1;
	a_uint32_t is_snap_mask:1;
	a_uint32_t is_ethernet_mask:1;
	a_uint32_t is_ipv6_mask:1;
	a_uint32_t is_ip_mask:1;
	a_uint32_t pppoe_sessionid_mask:16;
}ADPT_HPPE_ACL_L2MISC_RULE_MASK;

typedef struct{
	a_uint32_t ip_0:16;
	a_uint32_t l4_port:16;/*it is min dport when range is enable*/

	a_uint32_t _reserved0:11;
	a_uint32_t is_ip:1;
	a_uint32_t l3_packet_type:3;
	a_uint32_t l3_fragment:1;
	a_uint32_t ip_1:16;
}ADPT_HPPE_ACL_IPV4_RULE;

typedef struct{
	a_uint32_t ip_mask_0:16;
	a_uint32_t l4_port_mask:16;/*it is min dport when range is enable*/

	a_uint32_t _reserved0:11;
	a_uint32_t is_ip_mask:1;
	a_uint32_t l3_packet_type_mask:3;
	a_uint32_t l3_fragment_mask:1;
	a_uint32_t ip_mask_1:16;
}ADPT_HPPE_ACL_IPV4_RULE_MASK;

typedef struct{
	a_uint32_t udf1:16;
	a_uint32_t udf0:16;

	a_uint32_t _reserved0:11;
	a_uint32_t is_ip:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t udf2_valid:1;
	a_uint32_t udf1_valid:1;
	a_uint32_t udf0_valid:1;
	a_uint32_t udf2:16;
}ADPT_HPPE_ACL_UDF_RULE;

typedef struct{
	a_uint32_t udf1_mask:16;
	a_uint32_t udf0_mask:16;

	a_uint32_t _reserved0:11;
	a_uint32_t is_ip:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t udf2_valid:1;
	a_uint32_t udf1_valid:1;
	a_uint32_t udf0_valid:1;
	a_uint32_t udf2_mask:16;
}ADPT_HPPE_ACL_UDF_RULE_MASK;

typedef struct{
	a_uint32_t ip_ext_1:16;
	a_uint32_t ip_port:16; /*it is port when DIP_2_RULE or SIP_2_RULE*/

	a_uint32_t _reserved0:12;
	a_uint32_t l3_packet_type:3;
	a_uint32_t l3_fragment:1;
	a_uint32_t ip_ext_2:16;
}ADPT_HPPE_ACL_IPV6_RULE;

typedef struct{
	a_uint32_t ip_ext_1_mask:16;
	a_uint32_t ip_port_mask:16; /*it is port when DIP_2_RULE or SIP_2_RULE*/

	a_uint32_t _reserved0:12;
	a_uint32_t l3_packet_type_mask:3;
	a_uint32_t l3_fragment_mask:1;
	a_uint32_t ip_ext_2_mask:16;
}ADPT_HPPE_ACL_IPV6_RULE_MASK;

typedef struct{
	a_uint32_t l3_dscp_tc:8;
	a_uint32_t l3_prot:8; /*ipv4 protocol or ipv6 next header*/
	a_uint32_t l3_length:16;/*it is min length when range is enable*/

	a_uint32_t reserved1:14;
	a_uint32_t is_ipv6:1;
	a_uint32_t l3_fragment:1;
	a_uint32_t reserved0:1;
	a_uint32_t other_header:1;
	a_uint32_t fragment_header:1;
	a_uint32_t mobility_header:1;
	a_uint32_t esp_header:1;
	a_uint32_t ah_header:1;
	a_uint32_t l3_ttl:2; /*ipv4 ttl, ipv6 hop limit*/
	a_uint32_t ipv4_option_state:1;
	a_uint32_t tcp_flags:6;
	a_uint32_t first_fragment:1;
}ADPT_HPPE_ACL_IPMISC_RULE;

typedef struct{
	a_uint32_t l3_dscp_tc_mask:8;
	a_uint32_t l3_prot_mask:8; /*ipv4 protocol or ipv6 next header*/
	a_uint32_t l3_length_mask:16;/*it is max length when range is enable*/

	a_uint32_t reserved1:14;
	a_uint32_t is_ipv6_mask:1;
	a_uint32_t l3_fragment_mask:1;
	a_uint32_t reserved0:1;
	a_uint32_t other_header_mask:1;
	a_uint32_t fragment_header_mask:1;
	a_uint32_t mobility_header_mask:1;
	a_uint32_t esp_header_mask:1;
	a_uint32_t ah_header_mask:1;
	a_uint32_t l3_ttl_mask:2; /*ipv4 ttl, ipv6 hop limit*/
	a_uint32_t ipv4_option_state_mask:1;
	a_uint32_t tcp_flags_mask:6;
	a_uint32_t first_fragment_mask:1;
}ADPT_HPPE_ACL_IPMISC_RULE_MASK;
#else
typedef struct{
	a_uint32_t  mac_addr_0:32;
	a_uint32_t  mac_addr_1:16;
	a_uint32_t is_ip:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t is_ethernet:1;
	a_uint32_t is_snap:1;
	a_uint32_t is_fake_mac_header:1;
}ADPT_HPPE_ACL_MAC_RULE;
typedef struct{
	a_uint32_t  mac_addr_mask_0:32;
	a_uint32_t  mac_addr_mask_1:16;
	a_uint32_t is_ip_mask:1;
	a_uint32_t is_ipv6_mask:1;
	a_uint32_t is_ethernet_mask:1;
	a_uint32_t is_snap_mask:1;
	a_uint32_t is_fake_mac_header_mask:1;
}ADPT_HPPE_ACL_MAC_RULE_MASK;

typedef struct{
	a_uint32_t cvid:12;/*it is min cvid when range is enable*/
#if defined(APPE)
	a_uint32_t reserved:3;
#else
	a_uint32_t reserved:4;
#endif
	a_uint32_t cpcp:3;
	a_uint32_t cdei:1;
	a_uint32_t svid:12;
#if defined(APPE)
	a_uint32_t spcp_0:1;
	a_uint32_t spcp_1:2;
#else
	a_uint32_t spcp:3;
#endif
	a_uint32_t sdei:1;
	a_uint32_t ctag_fmt:3;
	a_uint32_t stag_fmt:3;
#if defined(APPE)
	a_uint32_t vsi:6;
#else
	a_uint32_t vsi:5;
#endif
	a_uint32_t vsi_valid:1;
	a_uint32_t is_ip:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t is_ethernet:1;
	a_uint32_t is_snap:1;
	a_uint32_t is_fake_mac_header:1;
	a_uint32_t _reserved0:11;
}ADPT_HPPE_ACL_VLAN_RULE;

typedef struct{
	a_uint32_t cvid_mask:12;/*it is max cvid when range is enable*/
#if defined(APPE)
	a_uint32_t reserved:3;
#else
	a_uint32_t reserved:4;
#endif
	a_uint32_t cpcp_mask:3;
	a_uint32_t cdei_mask:1;
	a_uint32_t svid_mask:12;
#if defined(APPE)
	a_uint32_t spcp_mask_0:1;
	a_uint32_t spcp_mask_1:2;
#else
	a_uint32_t spcp_mask:3;
#endif
	a_uint32_t sdei_mask:1;
	a_uint32_t ctag_fmt_mask:3;
	a_uint32_t stag_fmt_mask:3;
#if defined(APPE)
	a_uint32_t vsi_mask:6;
#else
	a_uint32_t vsi_mask:5;
#endif
	a_uint32_t vsi_valid_mask:1;
	a_uint32_t is_ip_mask:1;
	a_uint32_t is_ipv6_mask:1;
	a_uint32_t is_ethernet_mask:1;
	a_uint32_t is_snap_mask:1;
	a_uint32_t is_fake_mac_header_mask:1;
	a_uint32_t _reserved0:11;
}ADPT_HPPE_ACL_VLAN_RULE_MASK;

typedef struct{
	a_uint32_t svid:12;/*it is min svid when range is enable*/
	a_uint32_t reserved:4;
	a_uint32_t l2prot:16;
	a_uint32_t pppoe_sessionid:16;
	a_uint32_t is_ip:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t is_ethernet:1;
	a_uint32_t is_snap:1;
	a_uint32_t is_fake_mac_header:1;
}ADPT_HPPE_ACL_L2MISC_RULE;

typedef struct{
	a_uint32_t svid_mask:12;/*it is max svid when range is enable*/
	a_uint32_t reserved:4;
	a_uint32_t l2prot_mask:16;
	a_uint32_t pppoe_sessionid_mask:16;
	a_uint32_t is_ip_mask:1;
	a_uint32_t is_ipv6_mask:1;
	a_uint32_t is_ethernet_mask:1;
	a_uint32_t is_snap_mask:1;
	a_uint32_t is_fake_mac_header_mask:1;
}ADPT_HPPE_ACL_L2MISC_RULE_MASK;

typedef struct{
	a_uint32_t l4_port:16;/*it is min dport when range is enable*/
	a_uint32_t ip_0:16;
	a_uint32_t ip_1:16;
	a_uint32_t l3_fragment:1;
	a_uint32_t l3_packet_type:3;
	a_uint32_t is_ip:1;
	a_uint32_t reserved:11;
}ADPT_HPPE_ACL_IPV4_RULE;

typedef struct{
	a_uint32_t l4_port_mask:16;/*it is min dport when range is enable*/
	a_uint32_t ip_mask_0:16;
	a_uint32_t ip_mask_1:16;
	a_uint32_t l3_fragment_mask:1;
	a_uint32_t l3_packet_type_mask:3;
	a_uint32_t is_ip_mask:1;
	a_uint32_t reserved:11;
}ADPT_HPPE_ACL_IPV4_RULE_MASK;

typedef struct{
	a_uint32_t udf0:16;
	a_uint32_t udf1:16;
	a_uint32_t udf2:16;
	a_uint32_t udf0_valid:1;
	a_uint32_t udf1_valid:1;
	a_uint32_t udf2_valid:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t is_ip:1;
}ADPT_HPPE_ACL_UDF_RULE;

typedef struct{
	a_uint32_t udf0_mask:16;
	a_uint32_t udf1_mask:16;
	a_uint32_t udf2_mask:16;
	a_uint32_t udf0_valid:1;
	a_uint32_t udf1_valid:1;
	a_uint32_t udf2_valid:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t is_ip:1;
}ADPT_HPPE_ACL_UDF_RULE_MASK;

typedef struct{
	a_uint32_t ip_port:16; /*it is port when DIP_2_RULE or SIP_2_RULE*/
	a_uint32_t ip_ext_1:16;
	a_uint32_t ip_ext_2:16;
	a_uint32_t l3_fragment:1;
	a_uint32_t l3_packet_type:3;
	a_uint32_t reserved:1;
}ADPT_HPPE_ACL_IPV6_RULE;

typedef struct{
	a_uint32_t ip_port_mask:16; /*it is port when DIP_2_RULE or SIP_2_RULE*/
	a_uint32_t ip_ext_1_mask:16;
	a_uint32_t ip_ext_2_mask:16;
	a_uint32_t l3_fragment_mask:1;
	a_uint32_t l3_packet_type_mask:3;
	a_uint32_t reserved:1;
}ADPT_HPPE_ACL_IPV6_RULE_MASK;

typedef struct{
	a_uint32_t l3_length:16;/*it is min length when range is enable*/
	a_uint32_t l3_prot:8; /*ipv4 protocol or ipv6 next header*/
	a_uint32_t l3_dscp_tc:8;
	a_uint32_t first_fragment:1;
	a_uint32_t tcp_flags:6;
	a_uint32_t ipv4_option_state:1;
	a_uint32_t l3_ttl:2; /*ipv4 ttl, ipv6 hop limit*/
	a_uint32_t ah_header:1;
	a_uint32_t esp_header:1;
	a_uint32_t mobility_header:1;
	a_uint32_t fragment_header:1;
	a_uint32_t other_header:1;
	a_uint32_t reserved0:1;
	a_uint32_t l3_fragment:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t reserved1:3;
}ADPT_HPPE_ACL_IPMISC_RULE;

typedef struct{
	a_uint32_t l3_length_mask:16;/*it is max length when range is enable*/
	a_uint32_t l3_prot_mask:8; /*ipv4 protocol or ipv6 next header*/
	a_uint32_t l3_dscp_tc_mask:8;
	a_uint32_t first_fragment_mask:1;
	a_uint32_t tcp_flags_mask:6;
	a_uint32_t ipv4_option_state_mask:1;
	a_uint32_t l3_ttl_mask:2; /*ipv4 ttl, ipv6 hop limit*/
	a_uint32_t ah_header_mask:1;
	a_uint32_t esp_header_mask:1;
	a_uint32_t mobility_header_mask:1;
	a_uint32_t fragment_header_mask:1;
	a_uint32_t other_header_mask:1;
	a_uint32_t reserved0:1;
	a_uint32_t l3_fragment_mask:1;
	a_uint32_t is_ipv6_mask:1;
	a_uint32_t reserved1:3;
}ADPT_HPPE_ACL_IPMISC_RULE_MASK;
#endif

static ADPT_HPPE_ACL_SW_LIST_HEAD g_acl_sw_list[SW_MAX_NR_DEV];
#if defined(APPE)
static ADPT_HPPE_ACL_HW_LIST g_acl_hw_list[SW_MAX_NR_DEV][ADPT_ACL_HW_LIST_NUM+ADPT_PRE_ACL_HW_LIST_NUM];
#else
static ADPT_HPPE_ACL_HW_LIST g_acl_hw_list[SW_MAX_NR_DEV][ADPT_ACL_HW_LIST_NUM];
#endif
static aos_lock_t hppe_acl_lock[SW_MAX_NR_DEV];

const a_uint8_t s_acl_ext2[7][2] = {
	{0,1},{2,3},{4,5},{6,7},{0,2},{4,6},{0,4}
};
typedef struct
{
	a_uint8_t num;
	a_uint8_t ext_1;
	a_uint8_t ext_2;
	a_uint8_t ext_4;
	a_uint8_t entries;
}ADPT_HPPE_ACL_ENTRY_EXTEND_INFO;
const ADPT_HPPE_ACL_ENTRY_EXTEND_INFO s_acl_entries[] = {
	/*num ext_1 ext_2 ext_4 entries*/
	{1, 0, 0, 0, 0x2},
	{1, 0, 0, 0, 0x8},
	{1, 0, 0, 0, 0x20},
	{1, 0, 0, 0, 0x80},
	{1, 0, 0, 0, 0x1},
	{1, 0, 0, 0, 0x4},
	{1, 0, 0, 0, 0x10},
	{1, 0, 0, 0, 0x40},
	{2, 0x1, 0, 0, 0x3},
	{2, 0x2, 0, 0, 0xc},
	{2, 0x4, 0, 0, 0x30},
	{2, 0x8, 0, 0, 0xc0},
	{2, 0, 0x1, 0, 0x5},
	{2, 0, 0x2, 0, 0x50},
	{2, 0, 0, 0x1, 0x11},
	{3, 0x1, 0x1, 0x0, 0x7},
	{3, 0x1, 0x0, 0x1, 0x13},
	{3, 0x2, 0x1, 0x0, 0xd},
	{3, 0x4, 0x2, 0x0, 0x70},
	{3, 0x4, 0x0, 0x1, 0x31},
	{3, 0x8, 0x2, 0x0, 0xd0},
	{3, 0x0, 0x1, 0x1, 0x15},
	{3, 0x0, 0x2, 0x1, 0x51},
	{4, 0x3, 0x1, 0x0, 0xf},
	{4, 0x5, 0x0, 0x1, 0x33},
	{4, 0x2, 0x1, 0x1, 0x1d},
	{4, 0xc, 0x2, 0x0, 0xf0},
	{4, 0x4, 0x1, 0x1, 0x35},
	{4, 0x8, 0x2, 0x1, 0xd1},
	{4, 0x0, 0x3, 0x1, 0x55},
	{5, 0x3, 0x1, 0x1, 0x1f},
	{5, 0x6, 0x1, 0x1, 0x3d},
	{5, 0xc, 0x2, 0x1, 0xf1},
	{5, 0x8, 0x3, 0x1, 0xd5},
	{6, 0x7, 0x1, 0x1, 0x3f},
	{6, 0x6, 0x3, 0x1, 0x7d},
	{6, 0xc, 0x3, 0x1, 0xf5},
	{7, 0x7, 0x3, 0x1, 0x7f},
	{7, 0xe, 0x3, 0x1, 0xfd},
	{8, 0xf, 0x3, 0x1, 0xff},
};

void _adpt_acl_reg_dump(a_uint8_t *reg, a_uint32_t len)
{
	a_int32_t i = 0;

	for(i = 0; i < len; i++)
	{
		printk(KERN_CONT "%02x ", reg[i]);
		if((i+1)%32 == 0 || (i == len-1))
			printk(KERN_CONT "\n");
	}

	return;
}

/*type = 0, count all; type = 1 count odd; type = 2 count even*/
static a_uint32_t _acl_bits_count(a_uint32_t bits, a_uint32_t max, a_uint32_t type)
{
	a_uint32_t i = 0, count = 0;
	while(i < max)
	{
		if((bits >> i) &0x1)
		{
			if(type == 1)
			{
				if(i%2!=0)
					count++;
			}
			else if(type == 2)
			{
				if(i%2==0)
					count++;
			}
			else
				count++;
		}
		i++;
	}
	return count;
}

/*type = 0, count all; type = 1 count odd; type = 2 count even*/
a_uint32_t _acl_bit_index(a_uint32_t bits, a_uint32_t max, a_uint32_t type)
{
	a_uint32_t i = 0;
	while(i < max)
	{
		if((bits >> i) &0x1)
		{
			if(type == 1)/*odd*/
			{
				if(i%2!=0)
					break;
			}
			else if(type == 2)/*even*/
			{
				if(i%2==0)
					break;
			}
			else
				break;
		}
		i++;
	}
	if(i<max)
		return i;
	return 0xff;
}

static a_bool_t
_adpt_acl_zero_addr(const fal_mac_addr_t addr)
{
    a_uint32_t i;

    for (i = 0; i < 6; i++)
    {
        if (addr.uc[i])
        {
            return A_FALSE;
        }
    }
    return A_TRUE;
}

#if 0
static void _acl_print_extend_slices(a_uint8_t extend,a_uint8_t extend_slices, u_int8_t slice_count)
{
	a_uint32_t i;
	a_uint32_t ext_1=0, ext_2=0, ext_4=0;
	for(i = 0; i < 8; i++)
	{
		if(extend & (1<<i))
		{
			if(i==6)
				ext_4 = 0x1;
			else if(i<4)
				ext_1 |= (1<<i);
			else
				ext_2 |= (1<<(i%4));
		}
	}
	printk("\n{%d, 0x%x, 0x%x, 0x%x, 0x%x},\n", slice_count, ext_1, ext_2, ext_4, extend_slices);
}
static void _acl_slice_ext_bitmap_gen(a_uint32_t ext_n)
{
	a_uint32_t i, j;
	a_uint8_t extend = 0;
	a_uint8_t full_extend = 0;
	a_uint8_t extend_slices = 0;
	a_uint8_t extend_count = 0;

	printk("########ext_n = %d\n", ext_n);

	for(i = 0; i < 7; i++)
	{
		extend = (1 << i);
		extend_slices = (1<<s_acl_ext2[i][0])|(1<<s_acl_ext2[i][1]);
		extend_count = 2;
		full_extend = 0;
		for(j = i+1; j < 7; j++)
		{
			int k;
			if((extend & (1<<j))||(full_extend & (1<<j)))
			{
				//printk("extend = %x, full_extend = %x\n", extend, full_extend);
				continue;
			}
			for(k = 0; k < extend_count; k++)
			{
				if(extend_slices &(1<<s_acl_ext2[j][0]))
				{
					extend |= (1<<j);
					extend_slices |= (1<<s_acl_ext2[j][1]);
					extend_count++;
					break;
				}
				else if(extend_slices &(1<<s_acl_ext2[j][1]))
				{
					extend |= (1<<j);
					extend_slices |= (1<<s_acl_ext2[j][0]);
					extend_count++;
					break;
				}
			}
			if(extend_count == ext_n)
			{
				_acl_print_extend_slices(extend, extend_slices, extend_count);
				full_extend |= extend;
				extend_count = 2;
				extend = (1 << i);
				extend_slices = (1<<s_acl_ext2[i][0])|(1<<s_acl_ext2[i][1]);
				j=i;
			}
			if(k < extend_count)
			{
				j=i;
			}
		}
	}
}
#endif

enum{
	HPPE_ACL_TYPE_PORTBITMAP = 0,
	HPPE_ACL_TYPE_PORT,
	HPPE_ACL_TYPE_SERVICE_CODE,
	HPPE_ACL_TYPE_L3_IF,
#if defined(APPE)
	APPE_ACL_TYPE_VP_GROUP,
	APPE_ACL_TYPE_SERVICE_PORTBITMAP,
#endif
	HPPE_ACL_TYPE_INVALID,
};

static a_uint32_t _adpt_hppe_acl_srctype_to_hw(fal_acl_bind_obj_t obj_t)
{
	a_uint32_t src_type = HPPE_ACL_TYPE_INVALID;

	switch(obj_t)
	{
		case FAL_ACL_BIND_PORTBITMAP:
			src_type = HPPE_ACL_TYPE_PORTBITMAP;
			break;
		case FAL_ACL_BIND_PORT:
			src_type = HPPE_ACL_TYPE_PORT;
			break;
		case FAL_ACL_BIND_SERVICE_CODE:
			src_type = HPPE_ACL_TYPE_SERVICE_CODE;
			break;
		case FAL_ACL_BIND_L3_IF:
			src_type = HPPE_ACL_TYPE_L3_IF;
			break;
#if defined(APPE)
		case FAL_ACL_BIND_VP_GROUP:
			src_type = APPE_ACL_TYPE_VP_GROUP;
			break;
		case FAL_ACL_BIND_SERVICE_PORTBITMAP:
			src_type = APPE_ACL_TYPE_SERVICE_PORTBITMAP;
			break;
#endif
		default:
			break;
	}
	return src_type;
}

static sw_error_t
_adpt_hppe_acl_rule_bind(a_uint32_t dev_id, a_uint32_t hw_list_id, a_uint32_t hw_entries,
	fal_acl_direc_t direc, fal_acl_bind_obj_t obj_t, a_uint32_t obj_idx)
{
	a_uint32_t hw_index = 0, hw_srctype = 0;
	union ipo_rule_reg_u hw_reg = {0};

	while(hw_entries != 0)
	{
		hw_index = _acl_bit_index(hw_entries, ADPT_ACL_ENTRY_NUM_PER_LIST, 0);
		if(hw_index >= ADPT_ACL_ENTRY_NUM_PER_LIST)
		{
			break;
		}

		hppe_ipo_rule_reg_get(dev_id, hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index,
			&hw_reg);

		if(obj_t == FAL_ACL_BIND_PORT && obj_idx < SSDK_MAX_PORT_NUM)
		{
			/*convert port to bitmap if it is physical port*/
			obj_t = FAL_ACL_BIND_PORTBITMAP;
			obj_idx = (1<<obj_idx);
		}

		hw_srctype = _adpt_hppe_acl_srctype_to_hw(obj_t);

		if(hw_srctype == HPPE_ACL_TYPE_INVALID)
		{
			SSDK_ERROR("Invalid source type %d\n", obj_t);
			return SW_BAD_PARAM;
		}
		else if(hw_srctype == HPPE_ACL_TYPE_PORTBITMAP &&
			hw_reg.bf.src_type == HPPE_ACL_TYPE_PORTBITMAP)
		{
			hw_reg.bf.src_0 |= obj_idx;
			hw_reg.bf.src_1 |= obj_idx>>
				SW_FIELD_OFFSET_IN_WORD(IPO_RULE_REG_SRC_OFFSET);
		}
#if defined(APPE)
		else if(hw_srctype == APPE_ACL_TYPE_SERVICE_PORTBITMAP &&
			hw_reg.bf.src_type == APPE_ACL_TYPE_SERVICE_PORTBITMAP)
		{
			hw_reg.bf.src_0 |= obj_idx;
			hw_reg.bf.src_1 |= obj_idx>>
				SW_FIELD_OFFSET_IN_WORD(IPO_RULE_REG_SRC_OFFSET);
		}
#endif
		else
		{
			hw_reg.bf.src_0 = obj_idx;
			hw_reg.bf.src_1 = obj_idx>>
				SW_FIELD_OFFSET_IN_WORD(IPO_RULE_REG_SRC_OFFSET);
		}
		hw_reg.bf.src_type = hw_srctype;

		hppe_ipo_rule_reg_set(dev_id, hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index,
			&hw_reg);
		SSDK_DEBUG("ACL bind entry %d source type %d, source value 0x%x\n",
			hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index, obj_t, obj_idx);
		hw_entries &= (~(1<<hw_index));
	}

	return SW_OK;
}

static ADPT_HPPE_ACL_SW_LIST *
_adpt_hppe_acl_list_entry_get(a_uint32_t dev_id, a_uint32_t list_id)
{
	ADPT_HPPE_ACL_SW_LIST *list_entry = NULL;
	struct list_head *list_pos = NULL;

	list_for_each(list_pos, &g_acl_sw_list[dev_id].list_sw_list)
	{
		list_entry = list_entry(list_pos, ADPT_HPPE_ACL_SW_LIST, list);
		if(list_entry->list_id == list_id)
		{
			break;
		}
	}
	if(list_pos == &g_acl_sw_list[dev_id].list_sw_list)
	{
		return NULL;
	}
	else
	{
		return list_entry;
	}
}

static sw_error_t
_adpt_ppe_acl_rule_bind(a_uint32_t dev_id, a_uint32_t list_id, ADPT_HPPE_ACL_SW_RULE *rule_entry,
	fal_acl_direc_t direc, fal_acl_bind_obj_t obj_t, a_uint32_t obj_idx)
{
	a_uint32_t hw_list_id = 0, hw_entries = 0;

	hw_entries = rule_entry->rule_hw_entry;
	hw_list_id = rule_entry->rule_hw_list_id;

	/* msg for debug */
	SSDK_DEBUG("ACL bind rule: list_id=%d, rule_id=%d, hw_entries=0x%x, hw_list_id=%d\n",
		list_id, rule_entry->rule_id, hw_entries, hw_list_id);

	if (hw_list_id < ADPT_ACL_HW_LIST_NUM)
	{
		return _adpt_hppe_acl_rule_bind(dev_id, hw_list_id,
				hw_entries, direc, obj_t, obj_idx);
	}
#if defined(APPE)
	else if (hw_list_id < ADPT_ACL_HW_LIST_NUM + ADPT_PRE_ACL_HW_LIST_NUM)
	{
		return _adpt_appe_pre_acl_rule_bind(dev_id, hw_list_id - ADPT_ACL_HW_LIST_NUM ,
				hw_entries, direc, obj_t, obj_idx);
	}
#endif
	return SW_OK;
}

sw_error_t
adpt_hppe_acl_list_bind(a_uint32_t dev_id, a_uint32_t list_id, fal_acl_direc_t direc,
		fal_acl_bind_obj_t obj_t, a_uint32_t obj_idx)
{
	struct list_head *rule_pos = NULL;
	ADPT_HPPE_ACL_SW_RULE *rule_bind_entry = NULL;
	ADPT_HPPE_ACL_SW_LIST *list_bind_entry = NULL;

	ADPT_DEV_ID_CHECK(dev_id);

	if(list_id >= ADPT_ACL_SW_LIST_NUM)
	{
		return SW_OUT_OF_RANGE;
	}

	aos_lock_bh(&hppe_acl_lock[dev_id]);
	list_bind_entry = _adpt_hppe_acl_list_entry_get(dev_id, list_id);
	if(list_bind_entry == NULL)
	{
		aos_unlock_bh(&hppe_acl_lock[dev_id]);
		return SW_NOT_FOUND;
	}

	list_for_each(rule_pos, &list_bind_entry->list_sw_rule)
	{
		rule_bind_entry = list_entry(rule_pos, ADPT_HPPE_ACL_SW_RULE, list);
		if(rule_bind_entry->rule_hw_entry)
		{
			sw_error_t rc;
			rc = _adpt_ppe_acl_rule_bind(dev_id, list_id, rule_bind_entry,
					direc, obj_t, obj_idx);
			if(rc != SW_OK)
			{
				SSDK_ERROR("rule %d bind fail\n", rule_bind_entry->rule_id);
				aos_unlock_bh(&hppe_acl_lock[dev_id]);
				return SW_FAIL;
			}
		}
	}
	aos_unlock_bh(&hppe_acl_lock[dev_id]);
	return SW_OK;
}

sw_error_t _adpt_hppe_acl_mac_rule_hw_2_sw(a_uint32_t is_mac_da,
	ADPT_HPPE_ACL_MAC_RULE * macrule, ADPT_HPPE_ACL_MAC_RULE_MASK * macrule_mask,
	a_uint8_t inverse_en, fal_acl_rule_t * rule)
{
	fal_acl_field_map_t field_flg = {0};

	if(is_mac_da)
	{
		rule->dest_mac_val.uc[5] = macrule->mac_addr_0;
		rule->dest_mac_val.uc[4] = macrule->mac_addr_0 >> 8;
		rule->dest_mac_val.uc[3] = macrule->mac_addr_0 >> 16;
		rule->dest_mac_val.uc[2] = macrule->mac_addr_0 >> 24;
		rule->dest_mac_val.uc[1] = macrule->mac_addr_1;
		rule->dest_mac_val.uc[0] = macrule->mac_addr_1 >> 8;

		rule->dest_mac_mask.uc[5] = macrule_mask->mac_addr_mask_0;
		rule->dest_mac_mask.uc[4] = macrule_mask->mac_addr_mask_0 >> 8;
		rule->dest_mac_mask.uc[3] = macrule_mask->mac_addr_mask_0 >> 16;
		rule->dest_mac_mask.uc[2] = macrule_mask->mac_addr_mask_0 >> 24;
		rule->dest_mac_mask.uc[1] = macrule_mask->mac_addr_mask_1;
		rule->dest_mac_mask.uc[0] = macrule_mask->mac_addr_mask_1 >> 8;
	}
	else
	{
		rule->src_mac_val.uc[5] = macrule->mac_addr_0;
		rule->src_mac_val.uc[4] = macrule->mac_addr_0 >> 8;
		rule->src_mac_val.uc[3] = macrule->mac_addr_0 >> 16;
		rule->src_mac_val.uc[2] = macrule->mac_addr_0 >> 24;
		rule->src_mac_val.uc[1] = macrule->mac_addr_1;
		rule->src_mac_val.uc[0] = macrule->mac_addr_1 >> 8;

		rule->src_mac_mask.uc[5] = macrule_mask->mac_addr_mask_0;
		rule->src_mac_mask.uc[4] = macrule_mask->mac_addr_mask_0 >> 8;
		rule->src_mac_mask.uc[3] = macrule_mask->mac_addr_mask_0 >> 16;
		rule->src_mac_mask.uc[2] = macrule_mask->mac_addr_mask_0 >> 24;
		rule->src_mac_mask.uc[1] = macrule_mask->mac_addr_mask_1;
		rule->src_mac_mask.uc[0] = macrule_mask->mac_addr_mask_1 >> 8;
	}
	if(A_FALSE == _adpt_acl_zero_addr(rule->dest_mac_mask))
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_DA);
	}
	if(A_FALSE == _adpt_acl_zero_addr(rule->src_mac_mask))
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_SA);
	}

	if(macrule_mask->is_ip_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP);
		rule->is_ip_val = macrule->is_ip;
	}

	if(macrule_mask->is_ipv6_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IPV6);
		rule->is_ipv6_val = macrule->is_ipv6;
	}

	if(macrule_mask->is_ethernet_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_ETHERNET);
		rule->is_ethernet_val = macrule->is_ethernet;
	}

	if(macrule_mask->is_snap_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_SNAP);
		rule->is_snap_val = macrule->is_snap;
	}

	if(macrule_mask->is_fake_mac_header_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_FAKE_MAC_HEADER);
		rule->is_fake_mac_header_val = macrule->is_fake_mac_header;
	}

	if (inverse_en)
		FAL_FIELD_FLG_CPY(rule->inverse_field_flg, field_flg);
	else
		FAL_FIELD_FLG_CPY(rule->field_flg, field_flg);

	return SW_OK;
}

sw_error_t _adpt_hppe_acl_vlan_rule_hw_2_sw(ADPT_HPPE_ACL_VLAN_RULE * vlanrule,
	ADPT_HPPE_ACL_VLAN_RULE_MASK * vlanrule_mask, a_uint8_t range_en,
	a_uint8_t inverse_en, fal_acl_rule_t * rule)
{
	fal_acl_field_map_t field_flg = {0};

	/*ctag*/
	if(vlanrule_mask->cvid_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_CTAG_VID);
		rule->ctag_vid_mask = vlanrule_mask->cvid_mask;
		if(range_en)
		{
			if(vlanrule->cvid == 0)
			{
				rule->ctag_vid_op = FAL_ACL_FIELD_LE;
				rule->ctag_vid_val = vlanrule_mask->cvid_mask;
			}
			else if(vlanrule_mask->cvid_mask == 0xfff)
			{
				rule->ctag_vid_op = FAL_ACL_FIELD_GE;
				rule->ctag_vid_val = vlanrule->cvid;
			}
			else
			{
				rule->ctag_vid_op = FAL_ACL_FIELD_RANGE;
				rule->ctag_vid_val = vlanrule->cvid;
			}

		}
		else
		{
			rule->ctag_vid_op = FAL_ACL_FIELD_MASK;
			rule->ctag_vid_val = vlanrule->cvid;
		}
	}

	if(vlanrule_mask->cpcp_mask)
	{

		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_CTAG_PRI);
		rule->ctag_pri_val = vlanrule->cpcp;
		rule->ctag_pri_mask = vlanrule_mask->cpcp_mask;
	}

	if(vlanrule_mask->cdei_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_CTAG_CFI);
		rule->ctag_cfi_val = vlanrule->cdei;
		rule->ctag_cfi_mask = vlanrule_mask->cdei_mask;
	}

	if(vlanrule_mask->ctag_fmt_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_CTAGGED);
		rule->ctagged_val = vlanrule->ctag_fmt;
		rule->ctagged_mask = vlanrule_mask->ctag_fmt_mask;
	}

	/*stag*/
	if(vlanrule_mask->svid_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_STAG_VID);
		rule->stag_vid_val = vlanrule->svid;
		rule->stag_vid_mask = vlanrule_mask->svid_mask;
	}
#if defined(APPE)
	if(vlanrule_mask->spcp_mask_0 || vlanrule_mask->spcp_mask_1)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_STAG_PRI);
		rule->stag_pri_val = (vlanrule->spcp_1<<VLAN_RULE_SPCP_0_LEN)
						|vlanrule->spcp_0;
		rule->stag_pri_mask = (vlanrule_mask->spcp_mask_1<<VLAN_RULE_SPCP_MASK_0_LEN)
						|vlanrule_mask->spcp_mask_0;
	}
#else
	if(vlanrule_mask->spcp_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_STAG_PRI);
		rule->stag_pri_val = vlanrule->spcp;
		rule->stag_pri_mask = vlanrule_mask->spcp_mask;
	}
#endif
	if(vlanrule_mask->sdei_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_STAG_DEI);
		rule->stag_dei_val = vlanrule->sdei;
		rule->stag_dei_mask = vlanrule_mask->sdei_mask;
	}
	if(vlanrule_mask->stag_fmt_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_STAGGED);
		rule->stagged_val = vlanrule->stag_fmt;
		rule->stagged_mask = vlanrule_mask->stag_fmt_mask;
	}
	/*vsi*/
	if(vlanrule_mask->vsi_valid_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_VSI_VALID);
		rule->vsi_valid = vlanrule->vsi_valid;
		rule->vsi_valid_mask = vlanrule_mask->vsi_valid_mask;
	}
	if(vlanrule_mask->vsi_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_VSI);
		rule->vsi = vlanrule->vsi;
		rule->vsi_mask = vlanrule_mask->vsi_mask;
	}

	if(vlanrule_mask->is_ip_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP);
		rule->is_ip_val = vlanrule->is_ip;
	}

	if(vlanrule_mask->is_ipv6_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IPV6);
		rule->is_ipv6_val = vlanrule->is_ipv6;
	}

	if(vlanrule_mask->is_ethernet_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_ETHERNET);
		rule->is_ethernet_val = vlanrule->is_ethernet;
	}

	if(vlanrule_mask->is_snap_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_SNAP);
		rule->is_snap_val = vlanrule->is_snap;
	}

	if(vlanrule_mask->is_fake_mac_header_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_FAKE_MAC_HEADER);
		rule->is_fake_mac_header_val = vlanrule->is_fake_mac_header;
	}

	if (inverse_en)
		FAL_FIELD_FLG_CPY(rule->inverse_field_flg, field_flg);
	else
		FAL_FIELD_FLG_CPY(rule->field_flg, field_flg);

	return SW_OK;
}
sw_error_t _adpt_hppe_acl_l2_misc_rule_hw_2_sw(ADPT_HPPE_ACL_L2MISC_RULE * l2misc_rule,
	ADPT_HPPE_ACL_L2MISC_RULE_MASK * l2misc_mask, a_uint8_t range_en,
	a_uint8_t inverse_en, fal_acl_rule_t * rule)
{
	fal_acl_field_map_t field_flg = {0};

	/*stag*/
	if(l2misc_mask->svid_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_STAG_VID);
		rule->stag_vid_mask = l2misc_mask->svid_mask;
		if(range_en)
		{
			if(l2misc_rule->svid == 0)
			{
				rule->stag_vid_op = FAL_ACL_FIELD_LE;
				rule->stag_vid_val = l2misc_mask->svid_mask;
			}
			else if(l2misc_mask->svid_mask == 0xfff)
			{
				rule->stag_vid_op = FAL_ACL_FIELD_GE;
				rule->stag_vid_val = l2misc_rule->svid;
			}
			else
			{
				rule->stag_vid_op = FAL_ACL_FIELD_RANGE;
				rule->stag_vid_val = l2misc_rule->svid;
			}

		}
		else
		{
			rule->stag_vid_op = FAL_ACL_FIELD_MASK;
			rule->stag_vid_val = l2misc_rule->svid;
		}
	}

	if(l2misc_mask->l2prot_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MAC_ETHTYPE);
		rule->ethtype_val = l2misc_rule->l2prot;
		rule->ethtype_mask = l2misc_mask->l2prot_mask;
	}

	if(l2misc_mask->pppoe_sessionid_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_PPPOE_SESSIONID);
		rule->pppoe_sessionid = l2misc_rule->pppoe_sessionid;
		rule->pppoe_sessionid_mask = l2misc_mask->pppoe_sessionid_mask;
	}

	if(l2misc_mask->is_ip_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP);
		rule->is_ip_val = l2misc_rule->is_ip;
	}

	if(l2misc_mask->is_ipv6_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IPV6);
		rule->is_ipv6_val = l2misc_rule->is_ipv6;
	}

	if(l2misc_mask->is_ethernet_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_ETHERNET);
		rule->is_ethernet_val = l2misc_rule->is_ethernet;
	}

	if(l2misc_mask->is_snap_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_SNAP);
		rule->is_snap_val = l2misc_rule->is_snap;
	}

	if(l2misc_mask->is_fake_mac_header_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_FAKE_MAC_HEADER);
		rule->is_fake_mac_header_val = l2misc_rule->is_fake_mac_header;
	}

	if (inverse_en)
		FAL_FIELD_FLG_CPY(rule->inverse_field_flg, field_flg);
	else
		FAL_FIELD_FLG_CPY(rule->field_flg, field_flg);

	return SW_OK;
}

sw_error_t _adpt_hppe_acl_ipv4_rule_hw_2_sw(a_uint32_t is_ip_da,
	ADPT_HPPE_ACL_IPV4_RULE * ipv4rule, ADPT_HPPE_ACL_IPV4_RULE_MASK * ipv4rule_mask,
	a_uint8_t range_en, a_uint8_t inverse_en, fal_acl_rule_t *rule)
{
	fal_acl_field_map_t field_flg = {0};

	if(is_ip_da)
	{
		if(ipv4rule_mask->ip_mask_0 || ipv4rule_mask->ip_mask_1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP4_DIP);
			rule->dest_ip4_val = ipv4rule->ip_1<<16|ipv4rule->ip_0;
			rule->dest_ip4_mask = (ipv4rule_mask->ip_mask_1<<16)|ipv4rule_mask->ip_mask_0;
		}
		if(ipv4rule_mask->l4_port_mask)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_L4_DPORT);
			rule->dest_l4port_mask = ipv4rule_mask->l4_port_mask;
		}
		if(range_en)
		{
			if(ipv4rule->l4_port == 0)
			{
				rule->dest_l4port_op = FAL_ACL_FIELD_LE;
				rule->dest_l4port_val = ipv4rule_mask->l4_port_mask;
			}
			else if(ipv4rule_mask->l4_port_mask == 0xffff)
			{
				rule->dest_l4port_op = FAL_ACL_FIELD_GE;
				rule->dest_l4port_val = ipv4rule->l4_port;
			}
			else
			{
				rule->dest_l4port_op = FAL_ACL_FIELD_RANGE;
				rule->dest_l4port_val = ipv4rule->l4_port;
			}
		}
		else
		{
			rule->dest_l4port_op = FAL_ACL_FIELD_MASK;
			rule->dest_l4port_val = ipv4rule->l4_port;
		}
	}
	else
	{
		if(ipv4rule_mask->ip_mask_0 || ipv4rule_mask->ip_mask_1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP4_SIP);
			rule->src_ip4_val = ipv4rule->ip_1<<16|ipv4rule->ip_0;
			rule->src_ip4_mask = ipv4rule_mask->ip_mask_1<<16|ipv4rule_mask->ip_mask_0;
		}
		if(ipv4rule_mask->l4_port_mask)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_L4_SPORT);
			rule->src_l4port_mask = ipv4rule_mask->l4_port_mask;
		}
		if(range_en)
		{
			if(ipv4rule->l4_port == 0)
			{
				rule->src_l4port_op = FAL_ACL_FIELD_LE;
				rule->src_l4port_val = ipv4rule_mask->l4_port_mask;
			}
			else if(ipv4rule_mask->l4_port_mask == 0xffff)
			{
				rule->src_l4port_op = FAL_ACL_FIELD_GE;
				rule->src_l4port_val = ipv4rule->l4_port;
			}
			else
			{
				rule->src_l4port_op = FAL_ACL_FIELD_RANGE;
				rule->src_l4port_val = ipv4rule->l4_port;
			}
		}
		else
		{
			rule->src_l4port_op = FAL_ACL_FIELD_MASK;
			rule->src_l4port_val = ipv4rule->l4_port;
		}
	}

	if(ipv4rule_mask->is_ip_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP);
		rule->is_ip_val = ipv4rule->is_ip;
	}
	if(ipv4rule_mask->l3_fragment_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_L3_FRAGMENT);
		rule->is_fragement_val = ipv4rule->l3_fragment;
		rule->is_fragement_mask = ipv4rule_mask->l3_fragment_mask;
	}
	if(ipv4rule_mask->l3_packet_type_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP_PKT_TYPE);
		rule->l3_pkt_type = ipv4rule->l3_packet_type;
		rule->l3_pkt_type_mask = ipv4rule_mask->l3_packet_type_mask;
	}

	if (inverse_en)
		FAL_FIELD_FLG_CPY(rule->inverse_field_flg, field_flg);
	else
		FAL_FIELD_FLG_CPY(rule->field_flg, field_flg);

	return SW_OK;
}

/*ip_bit_range: 0 mean DIP0 or SIP0, 1 mean DIP1 or SIP1, 2 mean DIP2 or SIP2,*/
sw_error_t _adpt_hppe_acl_ipv6_rule_hw_2_sw(a_uint32_t is_ip_da, a_uint32_t ip_bit_range,
	ADPT_HPPE_ACL_IPV6_RULE * ipv6rule, ADPT_HPPE_ACL_IPV6_RULE_MASK * ipv6rule_mask,
	a_uint8_t range_en, a_uint8_t inverse_en, fal_acl_rule_t *rule)
{
	fal_acl_field_map_t field_flg = {0};

	if(is_ip_da)
	{
		if(ip_bit_range == 0)
		{
			if(ipv6rule_mask->ip_port_mask
				|| ipv6rule_mask->ip_ext_1_mask
				|| ipv6rule_mask->ip_ext_2_mask)
			{
				FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP6_DIP);
			}

			rule->dest_ip6_val.ul[3] = ipv6rule->ip_ext_1<<16|ipv6rule->ip_port;
			rule->dest_ip6_val.ul[2] |= (ipv6rule->ip_ext_2)&0xffff;
			rule->dest_ip6_mask.ul[3] =
				ipv6rule_mask->ip_ext_1_mask<<16|ipv6rule_mask->ip_port_mask;
			rule->dest_ip6_mask.ul[2] |= (ipv6rule_mask->ip_ext_2_mask)&0xffff;
		}
		else if(ip_bit_range == 1)
		{
			if(ipv6rule_mask->ip_port_mask
				|| ipv6rule_mask->ip_ext_1_mask
				|| ipv6rule_mask->ip_ext_2_mask)
			{
				FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP6_DIP);
			}
			rule->dest_ip6_val.ul[2] |= (ipv6rule->ip_port<<16)&0xffff0000;
			rule->dest_ip6_val.ul[1] = ipv6rule->ip_ext_2<<16|ipv6rule->ip_ext_1;
			rule->dest_ip6_mask.ul[2] |= (ipv6rule_mask->ip_port_mask<<16)&0xffff0000;
			rule->dest_ip6_mask.ul[1] =
				ipv6rule_mask->ip_ext_2_mask<<16|ipv6rule_mask->ip_ext_1_mask;
		}
		else if(ip_bit_range == 2)
		{
			if(ipv6rule_mask->ip_ext_1_mask
				|| ipv6rule_mask->ip_ext_2_mask)
			{
				FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP6_DIP);
				rule->dest_ip6_val.ul[0] =
					ipv6rule->ip_ext_2<<16|ipv6rule->ip_ext_1;
				rule->dest_ip6_mask.ul[0] = ipv6rule_mask->ip_ext_2_mask<<16|
					ipv6rule_mask->ip_ext_1_mask;
			}
			if(ipv6rule_mask->ip_port_mask)
			{
				FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_L4_DPORT);
				rule->dest_l4port_mask = ipv6rule_mask->ip_port_mask;
			}
			if(range_en)
			{
				if(ipv6rule->ip_port == 0)
				{
					rule->dest_l4port_op = FAL_ACL_FIELD_LE;
					rule->dest_l4port_val = ipv6rule_mask->ip_port_mask;
				}
				else if(ipv6rule_mask->ip_port_mask == 0xffff)
				{
					rule->dest_l4port_op = FAL_ACL_FIELD_GE;
					rule->dest_l4port_val= ipv6rule->ip_port;
				}
				else
				{
					rule->dest_l4port_op = FAL_ACL_FIELD_RANGE;
					rule->dest_l4port_val= ipv6rule->ip_port;
				}
			}
			else
			{
				rule->dest_l4port_op = FAL_ACL_FIELD_MASK;
				rule->dest_l4port_val = ipv6rule->ip_port;
			}

		}
	}
	else
	{
		if(ip_bit_range == 0)
		{
			if(ipv6rule_mask->ip_port_mask
				|| ipv6rule_mask->ip_ext_1_mask
				|| ipv6rule_mask->ip_ext_2_mask)
			{
				FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP6_SIP);
			}
			rule->src_ip6_val.ul[3] = ipv6rule->ip_ext_1<<16|ipv6rule->ip_port;
			rule->src_ip6_val.ul[2] |= (ipv6rule->ip_ext_2)&0xffff;
			rule->src_ip6_mask.ul[3] =
				ipv6rule_mask->ip_ext_1_mask<<16|ipv6rule_mask->ip_port_mask;
			rule->src_ip6_mask.ul[2] |= (ipv6rule_mask->ip_ext_2_mask)&0xffff;
		}
		else if(ip_bit_range == 1)
		{
			if(ipv6rule_mask->ip_port_mask
				|| ipv6rule_mask->ip_ext_1_mask
				|| ipv6rule_mask->ip_ext_2_mask)
			{
				FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP6_SIP);
			}
			rule->src_ip6_val.ul[2] |= (ipv6rule->ip_port<<16)&0xffff0000;
			rule->src_ip6_val.ul[1] = ipv6rule->ip_ext_2<<16|ipv6rule->ip_ext_1;
			rule->src_ip6_mask.ul[2] |= (ipv6rule_mask->ip_port_mask<<16)&0xffff0000;
			rule->src_ip6_mask.ul[1] =
				ipv6rule_mask->ip_ext_2_mask<<16|ipv6rule_mask->ip_ext_1_mask;
		}
		else if(ip_bit_range == 2)
		{
			if(ipv6rule_mask->ip_ext_1_mask
				|| ipv6rule_mask->ip_ext_2_mask)
			{
				FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP6_SIP);
				rule->src_ip6_val.ul[0] = ipv6rule->ip_ext_2<<16|ipv6rule->ip_ext_1;
				rule->src_ip6_mask.ul[0] = ipv6rule_mask->ip_ext_2_mask<<16|
					ipv6rule_mask->ip_ext_1_mask;
			}
			if(ipv6rule_mask->ip_port_mask)
			{
				FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_L4_SPORT);
				rule->src_l4port_mask = ipv6rule_mask->ip_port_mask;
			}
			if(range_en)
			{
				if(ipv6rule->ip_port == 0)
				{
					rule->src_l4port_op = FAL_ACL_FIELD_LE;
					rule->src_l4port_val = ipv6rule_mask->ip_port_mask;
				}
				else if(ipv6rule_mask->ip_port_mask == 0xffff)
				{
					rule->src_l4port_op = FAL_ACL_FIELD_GE;
					rule->src_l4port_val= ipv6rule->ip_port;
				}
				else
				{
					rule->src_l4port_op = FAL_ACL_FIELD_RANGE;
					rule->src_l4port_val= ipv6rule->ip_port;
				}
			}
			else
			{
				rule->src_l4port_op = FAL_ACL_FIELD_MASK;
				rule->src_l4port_val = ipv6rule->ip_port;
			}

		}
	}

	if(ipv6rule_mask->l3_fragment_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_L3_FRAGMENT);
		rule->is_fragement_val = ipv6rule->l3_fragment;
		rule->is_fragement_mask = ipv6rule_mask->l3_fragment_mask;
	}
	if(ipv6rule_mask->l3_packet_type_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP_PKT_TYPE);
		rule->l3_pkt_type = ipv6rule->l3_packet_type;
		rule->l3_pkt_type_mask = ipv6rule_mask->l3_packet_type_mask;
	}

	if (inverse_en)
		FAL_FIELD_FLG_CPY(rule->inverse_field_flg, field_flg);
	else
		FAL_FIELD_FLG_CPY(rule->field_flg, field_flg);

	return SW_OK;
}

sw_error_t _adpt_hppe_acl_ipmisc_rule_hw_2_sw(ADPT_HPPE_ACL_IPMISC_RULE * ipmisc_rule,
	ADPT_HPPE_ACL_IPMISC_RULE_MASK * ipmisc_mask, a_uint8_t range_en,
	a_uint8_t inverse_en, fal_acl_rule_t * rule)
{
	fal_acl_field_map_t field_flg = {0};

	if(ipmisc_mask->l3_length_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_L3_LENGTH);
		rule->l3_length_mask = ipmisc_mask->l3_length_mask;
	}
	if(range_en)
	{
		if(ipmisc_rule->l3_length == 0)
		{
			rule->l3_length_op = FAL_ACL_FIELD_LE;
			rule->l3_length = ipmisc_mask->l3_length_mask;
		}
		else if(ipmisc_mask->l3_length_mask == 0xffff)
		{
			rule->l3_length_op = FAL_ACL_FIELD_GE;
			rule->l3_length = ipmisc_rule->l3_length;
		}
		else
		{
			rule->l3_length_op = FAL_ACL_FIELD_RANGE;
			rule->l3_length = ipmisc_rule->l3_length;
		}
	}
	else
	{
		rule->l3_length_op = FAL_ACL_FIELD_MASK;
		rule->l3_length = ipmisc_rule->l3_length;
	}

	if(ipmisc_mask->l3_prot_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP_PROTO);
		rule->ip_proto_val = ipmisc_rule->l3_prot;
		rule->ip_proto_mask = ipmisc_mask->l3_prot_mask;
	}
	if(ipmisc_mask->l3_dscp_tc_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP_DSCP);
		rule->ip_dscp_val = ipmisc_rule->l3_dscp_tc;
		rule->ip_dscp_mask = ipmisc_mask->l3_dscp_tc_mask;
	}

	if(ipmisc_mask->first_fragment_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_FIRST_FRAGMENT);
		rule->is_first_frag_val = ipmisc_rule->first_fragment;
	}
	if(ipmisc_mask->tcp_flags_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_TCP_FLAG);
		rule->tcp_flag_val = ipmisc_rule->tcp_flags;
		rule->tcp_flag_mask = ipmisc_mask->tcp_flags_mask;
	}
	if(ipmisc_mask->ipv4_option_state_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IPV4_OPTION);
		rule->is_ipv4_option_val = ipmisc_rule->ipv4_option_state;
	}
	if(ipmisc_mask->l3_ttl_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_L3_TTL);
		rule->l3_ttl = ipmisc_rule->l3_ttl;
		rule->l3_ttl_mask = ipmisc_mask->l3_ttl_mask;
	}
	if(ipmisc_mask->ah_header_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_AH_HEADER);
		rule->is_ah_header_val = ipmisc_rule->ah_header;
	}
	if(ipmisc_mask->esp_header_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_ESP_HEADER);
		rule->is_esp_header_val = ipmisc_rule->esp_header;
	}
	if(ipmisc_mask->mobility_header_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_MOBILITY_HEADER);
		rule->is_mobility_header_val = ipmisc_rule->mobility_header;
	}
	if(ipmisc_mask->fragment_header_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_FRAGMENT_HEADER);
		rule->is_fragment_header_val = ipmisc_rule->fragment_header;
	}
	if(ipmisc_mask->other_header_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_OTHER_EXT_HEADER);
		rule->is_other_header_val = ipmisc_rule->other_header;
	}
	if(ipmisc_mask->is_ipv6_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IPV6);
		rule->is_ipv6_val = ipmisc_rule->is_ipv6;
	}
	if(ipmisc_mask->l3_fragment_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_L3_FRAGMENT);
		rule->is_fragement_val = ipmisc_rule->l3_fragment;
	}

	if (inverse_en)
		FAL_FIELD_FLG_CPY(rule->inverse_field_flg, field_flg);
	else
		FAL_FIELD_FLG_CPY(rule->field_flg, field_flg);

	return SW_OK;
}

sw_error_t _adpt_hppe_acl_udf_rule_hw_2_sw(a_uint32_t is_win1, ADPT_HPPE_ACL_UDF_RULE * udfrule,
	ADPT_HPPE_ACL_UDF_RULE_MASK * udfrule_mask, a_uint8_t range_en,
	a_uint8_t inverse_en, fal_acl_rule_t * rule)
{
	fal_acl_field_map_t field_flg = {0};

	if(is_win1)
	{
		if(udfrule->udf2_valid == 1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_UDF3);
			rule->udf3_val = udfrule->udf2;
			rule->udf3_mask = udfrule_mask->udf2_mask;
		}
		if(udfrule->udf1_valid == 1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_UDF2);
			rule->udf2_val = udfrule->udf1;
			rule->udf2_mask = udfrule_mask->udf1_mask;
		}
		if(udfrule->udf0_valid == 1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_UDF1);
			if(range_en == 1)
			{
				if(udfrule->udf0 == 0)
				{
					rule->udf1_op = FAL_ACL_FIELD_LE;
					rule->udf1_val = udfrule_mask->udf0_mask;
				}
				else if(rule->udf1_mask == 0xffff)
				{
					rule->udf1_op = FAL_ACL_FIELD_GE;
					rule->udf1_val = udfrule->udf0;
				}
				else
				{
					rule->udf1_op = FAL_ACL_FIELD_RANGE;
					rule->udf1_val = udfrule->udf0;
					rule->udf1_mask = udfrule_mask->udf0_mask;
				}
			}
			else
			{
				rule->udf1_op = FAL_ACL_FIELD_MASK;
				rule->udf1_val = udfrule->udf0;
				rule->udf1_mask = udfrule_mask->udf0_mask;
			}

		}
	}
	else
	{
		if(udfrule->udf2_valid == 1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_UDF2);
			rule->udf2_val = udfrule->udf2;
			rule->udf2_mask = udfrule_mask->udf2_mask;
		}
		if(udfrule->udf1_valid == 1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_UDF1);
			rule->udf1_val = udfrule->udf1;
			rule->udf1_mask = udfrule_mask->udf1_mask;
		}
		if(udfrule->udf0_valid == 1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_UDF0);
			if(range_en == 1)
			{
				if(udfrule->udf0 == 0)
				{
					rule->udf0_op = FAL_ACL_FIELD_LE;
					rule->udf0_val = udfrule_mask->udf0_mask;
				}
				else if(rule->udf0_mask == 0xffff)
				{
					rule->udf0_op = FAL_ACL_FIELD_GE;
					rule->udf0_val = udfrule->udf0;
				}
				else
				{
					rule->udf0_op = FAL_ACL_FIELD_RANGE;
					rule->udf0_val = udfrule->udf0;
					rule->udf0_mask = udfrule_mask->udf0_mask;
				}
			}
			else
			{
				rule->udf0_op = FAL_ACL_FIELD_MASK;
				rule->udf0_val = udfrule->udf0;
				rule->udf0_mask = udfrule_mask->udf0_mask;
			}

		}
	}

	if(udfrule_mask->is_ip)
	{
		rule->is_ip_val = udfrule->is_ip;
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP);
	}
	if(udfrule_mask->is_ipv6)
	{
		rule->is_ipv6_val = udfrule->is_ipv6;
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IPV6);
	}

	if (inverse_en)
		FAL_FIELD_FLG_CPY(rule->inverse_field_flg, field_flg);
	else
		FAL_FIELD_FLG_CPY(rule->field_flg, field_flg);

	return SW_OK;
}

static sw_error_t
_adpt_hppe_acl_action_hw_2_sw(a_uint32_t dev_id,union ipo_action_u *hw_act, fal_acl_rule_t *rule)
{
	if(hw_act->bf.dest_info_change_en)
	{
		a_uint32_t dest_type = HPPE_ACL_DEST_TYPE(hw_act->bf.dest_info);
		a_uint32_t dest_val = HPPE_ACL_DEST_VALUE(hw_act->bf.dest_info);
		SSDK_DEBUG("hw_act->bf.dest_info = %x\n", hw_act->bf.dest_info);
		if(dest_type == HPPE_ACL_DEST_NEXTHOP) /*nexthop*/
		{
			rule->ports = FAL_ACL_DEST_OFFSET(FAL_ACL_DEST_NEXTHOP,
					dest_val);
		}
		else if(dest_type == HPPE_ACL_DEST_PORT_ID) /*vp or trunk*/
		{
			rule->ports = FAL_ACL_DEST_OFFSET(FAL_ACL_DEST_PORT_ID,
					dest_val);
		}
		else if(dest_type == HPPE_ACL_DEST_PORT_BMP) /*bitmap*/
		{
			rule->ports = FAL_ACL_DEST_OFFSET(FAL_ACL_DEST_PORT_BMP,
					dest_val);
		}
		if(rule->ports != 0)
		{
			FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REDPT);
		}
		else if(hw_act->bf.fwd_cmd == HPPE_ACL_ACTION_RDTCPU)
		{
			FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_RDTCPU);
		}
		else if(hw_act->bf.fwd_cmd == HPPE_ACL_ACTION_COPYCPU)
		{
			FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_CPYCPU);
		}
		else if(hw_act->bf.fwd_cmd == HPPE_ACL_ACTION_DROP)
		{
			FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_DENY);
		}
		else if(hw_act->bf.fwd_cmd == HPPE_ACL_ACTION_FWD)
		{
			FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_PERMIT);
		}
	}

	if(hw_act->bf.mirror_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_MIRROR);
	}
	if(hw_act->bf.bypass_bitmap_0 != 0 ||
		hw_act->bf.bypass_bitmap_1 != 0)
	{
		rule->bypass_bitmap = (hw_act->bf.bypass_bitmap_1<<14)|hw_act->bf.bypass_bitmap_0;
	}
	if(hw_act->bf.svid_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_VID);
		rule->stag_fmt = hw_act->bf.stag_fmt;
		rule->stag_vid = hw_act->bf.svid;
	}
	if(hw_act->bf.stag_pcp_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_PRI);
		rule->stag_pri = hw_act->bf.stag_pcp;
	}
	if(hw_act->bf.stag_dei_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_DEI);
		rule->stag_dei = hw_act->bf.stag_dei;
	}
	if(hw_act->bf.cvid_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_VID);
		rule->ctag_fmt = hw_act->bf.ctag_fmt;
		rule->ctag_vid = hw_act->bf.cvid;
	}
	if(hw_act->bf.ctag_pcp_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_PRI);
		rule->ctag_pri = (hw_act->bf.ctag_pcp_1<<2)|hw_act->bf.ctag_pcp_0;
	}
	if(hw_act->bf.ctag_dei_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_CFI);
		rule->ctag_cfi = hw_act->bf.ctag_dei;
	}
	if(hw_act->bf.dscp_tc_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_DSCP);
		rule->dscp = hw_act->bf.dscp_tc;
#if defined(CPPE) || defined(APPE)
		if(adpt_ppe_type_get(dev_id) == CPPE_TYPE ||
			adpt_chip_type_get(dev_id) == CHIP_APPE)
		{
			rule->dscp_mask = hw_act->bf.dscp_tc_mask;
		}
#endif
	}
	if(hw_act->bf.int_dp_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_INT_DP);
		rule->int_dp = hw_act->bf.int_dp;
	}
	if(hw_act->bf.policer_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_POLICER_EN);
		rule->policer_ptr = hw_act->bf.policer_index;
	}
	if(hw_act->bf.qid_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_QUEUE);
		rule->queue = hw_act->bf.qid;
	}
	if(hw_act->bf.enqueue_pri_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_ENQUEUE_PRI);
		rule->enqueue_pri = hw_act->bf.enqueue_pri;
	}
	if(hw_act->bf.service_code_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_SERVICE_CODE);
		rule->service_code = (hw_act->bf.service_code_1<<1)|hw_act->bf.service_code_0;
	}
	if(hw_act->bf.syn_toggle)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_SYN_TOGGLE);
	}
	if(hw_act->bf.cpu_code_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_CPU_CODE);
		rule->cpu_code = hw_act->bf.cpu_code;
	}
	if(hw_act->bf.metadata_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_METADATA_EN);
#if defined(MPPE)
		rule->metadata_pri =
			(hw_act->bf.metadata_pri_1<<3)|hw_act->bf.metadata_pri_0;
#endif
	}
#if defined(CPPE) || defined(APPE)
	if(adpt_ppe_type_get(dev_id) == CPPE_TYPE ||
		adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
		rule->qos_res_prec = hw_act->bf.qos_res_prec;
	}
#endif
	return SW_OK;
}

sw_error_t
_adpt_hppe_acl_rule_hw_2_sw(a_uint32_t dev_id, a_uint32_t rule_type,
	a_uint8_t range_en, a_uint8_t inverse_en, void * hw_rule,
	void * hw_rule_mask, fal_acl_rule_t * rule)
{
	if(rule_type == ADPT_ACL_HPPE_MAC_DA_RULE)
	{
		_adpt_hppe_acl_mac_rule_hw_2_sw(1, (ADPT_HPPE_ACL_MAC_RULE *)hw_rule,
		(ADPT_HPPE_ACL_MAC_RULE_MASK *)hw_rule_mask, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_MAC_SA_RULE)
	{
		_adpt_hppe_acl_mac_rule_hw_2_sw(0, (ADPT_HPPE_ACL_MAC_RULE *)hw_rule,
		(ADPT_HPPE_ACL_MAC_RULE_MASK *)hw_rule_mask, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_VLAN_RULE)
	{
		_adpt_hppe_acl_vlan_rule_hw_2_sw((ADPT_HPPE_ACL_VLAN_RULE *)hw_rule,
		(ADPT_HPPE_ACL_VLAN_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_L2_MISC_RULE)
	{
		_adpt_hppe_acl_l2_misc_rule_hw_2_sw((ADPT_HPPE_ACL_L2MISC_RULE *)hw_rule,
		(ADPT_HPPE_ACL_L2MISC_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_IPV4_DIP_RULE)
	{
		_adpt_hppe_acl_ipv4_rule_hw_2_sw(1, (ADPT_HPPE_ACL_IPV4_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV4_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_IPV4_SIP_RULE)
	{
		_adpt_hppe_acl_ipv4_rule_hw_2_sw(0, (ADPT_HPPE_ACL_IPV4_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV4_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_IPV6_DIP0_RULE)
	{
		_adpt_hppe_acl_ipv6_rule_hw_2_sw(1, 0, (ADPT_HPPE_ACL_IPV6_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV6_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_IPV6_DIP1_RULE)
	{
		_adpt_hppe_acl_ipv6_rule_hw_2_sw(1, 1, (ADPT_HPPE_ACL_IPV6_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV6_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_IPV6_DIP2_RULE)
	{
		_adpt_hppe_acl_ipv6_rule_hw_2_sw(1, 2, (ADPT_HPPE_ACL_IPV6_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV6_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_IPV6_SIP0_RULE)
	{
		_adpt_hppe_acl_ipv6_rule_hw_2_sw(0, 0, (ADPT_HPPE_ACL_IPV6_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV6_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_IPV6_SIP1_RULE)
	{
		_adpt_hppe_acl_ipv6_rule_hw_2_sw(0, 1, (ADPT_HPPE_ACL_IPV6_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV6_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_IPV6_SIP2_RULE)
	{
		_adpt_hppe_acl_ipv6_rule_hw_2_sw(0, 2, (ADPT_HPPE_ACL_IPV6_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV6_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_IPMISC_RULE)
	{
		_adpt_hppe_acl_ipmisc_rule_hw_2_sw((ADPT_HPPE_ACL_IPMISC_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPMISC_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_UDF0_RULE)
	{
		_adpt_hppe_acl_udf_rule_hw_2_sw(0, (ADPT_HPPE_ACL_UDF_RULE *)hw_rule,
		(ADPT_HPPE_ACL_UDF_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_HPPE_UDF1_RULE)
	{
		_adpt_hppe_acl_udf_rule_hw_2_sw(1, (ADPT_HPPE_ACL_UDF_RULE *)hw_rule,
		(ADPT_HPPE_ACL_UDF_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
#if defined(APPE)
	if(rule_type == ADPT_ACL_APPE_EXT_UDF0_RULE)
	{
		_adpt_appe_acl_ext_udf_rule_hw_2_sw(0,
		(ADPT_APPE_ACL_EXT_UDF_RULE *)hw_rule,
		(ADPT_APPE_ACL_EXT_UDF_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_APPE_EXT_UDF1_RULE)
	{
		_adpt_appe_acl_ext_udf_rule_hw_2_sw(1,
		(ADPT_APPE_ACL_EXT_UDF_RULE *)hw_rule,
		(ADPT_APPE_ACL_EXT_UDF_RULE_MASK *)hw_rule_mask, range_en, inverse_en, rule);
	}
	if(rule_type == ADPT_ACL_APPE_TUNNEL_RULE)
	{
		_adpt_appe_pre_acl_tunnel_rule_hw_2_sw((ADPT_APPE_ACL_TUNNEL_RULE *)hw_rule,
		(ADPT_APPE_ACL_TUNNEL_RULE_MASK *)hw_rule_mask, inverse_en, &rule->tunnel_info);
	}
#endif
	return SW_OK;
}

static sw_error_t
_adpt_hppe_acl_rule_sw_query(a_uint32_t dev_id,
	a_uint32_t hw_list_id, a_uint32_t hw_entries, fal_acl_rule_t * rule)
{
	a_uint32_t hw_index;
	a_uint64_t byte_cnt;
	sw_error_t rv = SW_OK;
	union ipo_rule_reg_u hw_reg = {0};
	union ipo_mask_reg_u hw_mask = {0};
	union ipo_action_u hw_act = {0};
	union ipo_cnt_tbl_u hw_match = {0};

	while(hw_entries != 0)
	{
		hw_index = _acl_bit_index(hw_entries, ADPT_ACL_ENTRY_NUM_PER_LIST, 0);
		if(hw_index >= ADPT_ACL_ENTRY_NUM_PER_LIST)
		{
			return SW_FAIL;
		}
		rv |= hppe_ipo_rule_reg_get(dev_id, hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index,
			&hw_reg);
		rv |= hppe_ipo_mask_reg_get(dev_id, hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index,
			&hw_mask);
		rv |= hppe_ipo_action_get(dev_id, hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index,
			&hw_act);

		rule->post_routing = hw_reg.bf.post_routing_en;
		rule->acl_pool = hw_reg.bf.res_chain;
		rule->pri = hw_reg.bf.pri&0x7;

		/*get sw rule info from first 53bit hw rule reg fields*/
		_adpt_hppe_acl_rule_hw_2_sw(dev_id, hw_reg.bf.rule_type,
			hw_reg.bf.range_en, hw_reg.bf.inverse_en, &hw_reg, &hw_mask, rule);

		_adpt_hppe_acl_action_hw_2_sw(dev_id, &hw_act, rule);

		rv |= hppe_ipo_cnt_tbl_get(dev_id, hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_match);
		rule->match_cnt += hw_match.bf.hit_pkt_cnt;
		byte_cnt = hw_match.bf.hit_byte_cnt_1;
		rule->match_bytes += byte_cnt<<32|hw_match.bf.hit_byte_cnt_0;

		hw_entries &= (~(1<<hw_index));
	}
	return rv;
}

static sw_error_t
_adpt_ppe_acl_rule_sw_query(a_uint32_t dev_id,
	a_uint32_t hw_list_id, a_uint32_t hw_entries, fal_acl_rule_t * rule)
{
	sw_error_t rv = SW_OK;
	if (hw_list_id < ADPT_ACL_HW_LIST_NUM)
	{
		rv = _adpt_hppe_acl_rule_sw_query(dev_id, hw_list_id,
				hw_entries, rule);
		SW_RTN_ON_ERROR(rv);
	}
#if defined(APPE)
	else if (hw_list_id < ADPT_ACL_HW_LIST_NUM + ADPT_PRE_ACL_HW_LIST_NUM)
	{
		rv = _adpt_appe_pre_acl_rule_sw_query(dev_id,
				hw_list_id - ADPT_ACL_HW_LIST_NUM , hw_entries, rule);
		SW_RTN_ON_ERROR(rv);
	}
	rv = _adpt_appe_acl_ext_get(dev_id, hw_list_id, hw_entries, rule);
#endif
	return rv;
}

sw_error_t
adpt_hppe_acl_rule_query(a_uint32_t dev_id, a_uint32_t list_id, a_uint32_t rule_id,
		fal_acl_rule_t * rule)
{
	sw_error_t rv = 0;
	a_uint32_t hw_entries = 0, hw_list_id = 0;
	struct list_head *rule_pos = NULL;
	ADPT_HPPE_ACL_SW_RULE *rule_query_entry = NULL;
	ADPT_HPPE_ACL_SW_LIST *list_query_entry = NULL;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(rule);

	if(list_id >= ADPT_ACL_SW_LIST_NUM)
	{
		return SW_OUT_OF_RANGE;
	}

	if(rule_id >= ADPT_ACL_RULE_NUM_PER_LIST)
	{
		return SW_OUT_OF_RANGE;
	}

	aos_lock_bh(&hppe_acl_lock[dev_id]);
	list_query_entry = _adpt_hppe_acl_list_entry_get(dev_id, list_id);
	if(list_query_entry == NULL)
	{
		aos_unlock_bh(&hppe_acl_lock[dev_id]);
		return SW_NOT_FOUND;
	}

	list_for_each(rule_pos, &list_query_entry->list_sw_rule)
	{
		rule_query_entry = list_entry(rule_pos, ADPT_HPPE_ACL_SW_RULE, list);
		if((rule_query_entry->rule_id == rule_id) && (rule_query_entry->rule_hw_entry != 0))
		{
			rule->rule_type = rule_query_entry->rule_type[0];
			rule->inner_rule_field.rule_type = rule_query_entry->rule_type[1];
			hw_entries = rule_query_entry->rule_hw_entry;
			hw_list_id = rule_query_entry->rule_hw_list_id;
			break;
		}
	}
	if(rule_pos == &list_query_entry->list_sw_rule)
	{
		aos_unlock_bh(&hppe_acl_lock[dev_id]);
		return SW_NOT_FOUND;
	}
	aos_unlock_bh(&hppe_acl_lock[dev_id]);
	/*query rule*/
	rv = _adpt_ppe_acl_rule_sw_query(dev_id, hw_list_id, hw_entries, rule);

	/*update hw rule info*/
	rule->hw_info.hw_rule_id = hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST + \
			_acl_bit_index(hw_entries, ADPT_ACL_ENTRY_NUM_PER_LIST, 0);
	rule->hw_info.hw_list_id = hw_list_id;
	rule->hw_info.hw_entries = hw_entries;

	return rv;
}

static sw_error_t
_adpt_hppe_acl_rule_unbind(a_uint32_t dev_id, a_uint32_t hw_list_id, a_uint32_t hw_entries,
	fal_acl_direc_t direc, fal_acl_bind_obj_t obj_t, a_uint32_t obj_idx)
{
	a_uint32_t hw_index = 0;
	union ipo_rule_reg_u hw_reg = {0};

	while(hw_entries != 0)
	{
		hw_index = _acl_bit_index(hw_entries, ADPT_ACL_ENTRY_NUM_PER_LIST, 0);
		if(hw_index >= ADPT_ACL_ENTRY_NUM_PER_LIST)
		{
			break;
		}

		hppe_ipo_rule_reg_get(dev_id, hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index,
			&hw_reg);

		if(obj_t == FAL_ACL_BIND_PORT && obj_idx < SSDK_MAX_PORT_NUM)
		{
			/*convert port to bitmap if it is physical port*/
			obj_t = FAL_ACL_BIND_PORTBITMAP;
			obj_idx = (1<<obj_idx);
		}

		if(hw_reg.bf.src_type != _adpt_hppe_acl_srctype_to_hw(obj_t))
		{
			SSDK_ERROR("ACL unbind fail obj_t %d\n", obj_t);
			return SW_NOT_FOUND;
		}
		if(hw_reg.bf.src_type == HPPE_ACL_TYPE_PORTBITMAP)
		{
			hw_reg.bf.src_0 &= ~obj_idx;
			hw_reg.bf.src_1 &= ~(obj_idx>>
				SW_FIELD_OFFSET_IN_WORD(IPO_RULE_REG_SRC_OFFSET));
		}
#if defined(APPE)
		else if(hw_reg.bf.src_type == APPE_ACL_TYPE_SERVICE_PORTBITMAP)
		{
			hw_reg.bf.src_0 &= ~obj_idx;
			hw_reg.bf.src_1 &= ~(obj_idx>>
				SW_FIELD_OFFSET_IN_WORD(IPO_RULE_REG_SRC_OFFSET));
		}
#endif
		else
		{
			hw_reg.bf.src_type = HPPE_ACL_TYPE_PORTBITMAP;
			hw_reg.bf.src_0 = 0;
			hw_reg.bf.src_1 = 0;
		}
		hppe_ipo_rule_reg_set(dev_id, hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index,
			&hw_reg);
		SSDK_DEBUG("ACL unbind entry %d source type %d, source value 0x%x\n",
			hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index, obj_t, obj_idx);
		hw_entries &= (~(1<<hw_index));
	}

	return SW_OK;
}

static sw_error_t
_adpt_ppe_acl_rule_unbind(a_uint32_t dev_id, a_uint32_t list_id, ADPT_HPPE_ACL_SW_RULE *rule_entry,
	fal_acl_direc_t direc, fal_acl_bind_obj_t obj_t, a_uint32_t obj_idx)
{
	a_uint32_t hw_list_id = 0, hw_entries = 0;

	hw_entries = rule_entry->rule_hw_entry;
	hw_list_id = rule_entry->rule_hw_list_id;

	/* msg for debug */
	SSDK_DEBUG("ACL unbind rule: list_id=%d, rule_id=%d, hw_entries=0x%x, hw_list_id=%d\n",
		list_id, rule_entry->rule_id, hw_entries, hw_list_id);

	if (hw_list_id < ADPT_ACL_HW_LIST_NUM)
	{
		return _adpt_hppe_acl_rule_unbind(dev_id, hw_list_id,
				hw_entries, direc, obj_t, obj_idx);
	}
#if defined(APPE)
	else if (hw_list_id < ADPT_ACL_HW_LIST_NUM + ADPT_PRE_ACL_HW_LIST_NUM)
	{
		return _adpt_appe_pre_acl_rule_unbind(dev_id, hw_list_id - ADPT_ACL_HW_LIST_NUM ,
				hw_entries, direc, obj_t, obj_idx);
	}
#endif
	return SW_OK;
}

sw_error_t
adpt_hppe_acl_list_unbind(a_uint32_t dev_id, a_uint32_t list_id, fal_acl_direc_t direc,
		fal_acl_bind_obj_t obj_t, a_uint32_t obj_idx)
{
	struct list_head *rule_pos = NULL;
	ADPT_HPPE_ACL_SW_RULE *rule_unbind_entry = NULL;
	ADPT_HPPE_ACL_SW_LIST *list_unbind_entry = NULL;

	ADPT_DEV_ID_CHECK(dev_id);

	if(list_id >= ADPT_ACL_SW_LIST_NUM)
	{
		return SW_OUT_OF_RANGE;
	}

	aos_lock_bh(&hppe_acl_lock[dev_id]);
	list_unbind_entry = _adpt_hppe_acl_list_entry_get(dev_id, list_id);
	if(list_unbind_entry != NULL)
	{
		list_for_each(rule_pos, &list_unbind_entry->list_sw_rule)
		{
			rule_unbind_entry = list_entry(rule_pos, ADPT_HPPE_ACL_SW_RULE, list);
			if(rule_unbind_entry->rule_hw_entry)
			{
				_adpt_ppe_acl_rule_unbind(dev_id, list_id, rule_unbind_entry,
						direc, obj_t, obj_idx);
			}
		}
	}
	aos_unlock_bh(&hppe_acl_lock[dev_id]);
	return SW_OK;
}
sw_error_t
adpt_hppe_acl_rule_active(a_uint32_t dev_id, a_uint32_t list_id,
		a_uint32_t rule_id, a_uint32_t rule_nr)
{

	ADPT_DEV_ID_CHECK(dev_id);

	return SW_NOT_SUPPORTED;
}

static a_uint8_t
_adpt_hppe_acl_rule_range_count(a_uint32_t dev_id,
		a_uint32_t rule_id, a_uint32_t rule_nr, fal_acl_rule_t * rule)
{
	a_uint8_t rangecount = 0;
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_STAG_VID) ||
		FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_STAG_VID))
	{
		if (FAL_ACL_FIELD_MASK != rule->stag_vid_op)
		{
			rangecount++;
		}
	}
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_CTAG_VID) ||
		FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_CTAG_VID))
	{
		if (FAL_ACL_FIELD_MASK != rule->ctag_vid_op)
		{
			rangecount++;
		}
	}
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L4_DPORT) ||
		FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_L4_DPORT))
	{
		if (FAL_ACL_FIELD_MASK != rule->dest_l4port_op)
		{
			rangecount++;
		}
	}
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L4_SPORT) ||
		FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_L4_SPORT))
	{
		if (FAL_ACL_FIELD_MASK != rule->src_l4port_op)
		{
			rangecount++;
		}
	}

	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L3_LENGTH) ||
		FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_L3_LENGTH))
	{
		if (FAL_ACL_FIELD_MASK != rule->l3_length_op)
		{
			rangecount++;
		}
	}

	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF0) ||
		FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_UDF0))
	{
		if (FAL_ACL_FIELD_MASK != rule->udf0_op)
		{
			rangecount++;
		}
	}

	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF1) ||
		FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_UDF1))
	{
		if (FAL_ACL_FIELD_MASK != rule->udf1_op)
		{
			rangecount++;
		}
	}

	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_ICMP_TYPE) ||
		FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_ICMP_TYPE))
	{
		if (FAL_ACL_FIELD_MASK != rule->icmp_type_code_op)
		{
			rangecount++;
		}
	}
#if defined(APPE)
	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF2) ||
			FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_UDF2))
		{
			if (FAL_ACL_FIELD_MASK != rule->udf2_op)
			{
				rangecount++;
			}
		}
	}
#endif
	return rangecount;
}

static sw_error_t _adpt_hppe_acl_rule_range_match(a_uint32_t dev_id, a_uint32_t hw_list_index,
		a_uint32_t rule_id, a_uint32_t rule_nr, fal_acl_rule_t * rule,
		fal_acl_rule_t *inner_rule, a_uint8_t entries)
{
	a_uint8_t rangecount = 0, even_entry_count = 0;

	rangecount = _adpt_hppe_acl_rule_range_count(dev_id, rule_id, rule_nr, rule);
#if defined(APPE)
	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
		if(hw_list_index >= ADPT_ACL_HW_LIST_NUM)
		{
			rangecount += _adpt_hppe_acl_rule_range_count(dev_id,
						rule_id, rule_nr, inner_rule);
		}
	}
#endif
	even_entry_count = _acl_bits_count(entries, ADPT_ACL_ENTRY_NUM_PER_LIST, 2);

	if(rangecount <= even_entry_count)
	{
		return SW_OK;
	}
	return SW_NO_RESOURCE;
}

sw_error_t _adpt_hppe_acl_alloc_entries(a_uint32_t dev_id, a_uint32_t *hw_list_index,
		a_uint32_t rule_id, a_uint32_t rule_nr, fal_acl_rule_t * rule,
		fal_acl_rule_t *inner_rule, a_uint32_t rule_type_count, a_uint32_t *index)
{
	a_uint8_t free_hw_entry_bitmap = 0, free_hw_entry_count = 0, i = 0;
	a_uint32_t j = 0, start = 0, end = ADPT_ACL_HW_LIST_NUM;
	a_uint8_t map_info_count = sizeof(s_acl_entries)/sizeof(ADPT_HPPE_ACL_ENTRY_EXTEND_INFO);

#if defined(APPE)
	if(rule->rule_type == FAL_ACL_RULE_TUNNEL_MAC ||
		rule->rule_type == FAL_ACL_RULE_TUNNEL_IP4 ||
		rule->rule_type == FAL_ACL_RULE_TUNNEL_IP6 ||
		rule->rule_type == FAL_ACL_RULE_TUNNEL_UDF)

	{ /* pre-ipo */
		start = ADPT_ACL_HW_LIST_NUM;
		end = ADPT_ACL_HW_LIST_NUM + ADPT_PRE_ACL_HW_LIST_NUM;
	}
#endif
	for(j = start ; j < end; j++)
	{
		free_hw_entry_bitmap = g_acl_hw_list[dev_id][j].free_hw_entry_bitmap;
		free_hw_entry_count = g_acl_hw_list[dev_id][j].free_hw_entry_count;
		/* msg for debug */
		SSDK_DEBUG("_adpt_hppe_acl_alloc_entries():hw_list_index=%d, hw_list_id=%d, "
			"free_hw_entry_bitmap=0x%x, free_hw_entry_count=%d\n", j,
			g_acl_hw_list[dev_id][j].hw_list_id, free_hw_entry_bitmap,
			free_hw_entry_count);
		if(free_hw_entry_count < rule_type_count)
		{
			continue;
		}
		for(i = 0; i < map_info_count; i++)
		{
			if((rule_type_count == s_acl_entries[i].num) &&
				((free_hw_entry_bitmap & s_acl_entries[i].entries) ==
				s_acl_entries[i].entries))
			{
				if(SW_OK == _adpt_hppe_acl_rule_range_match(dev_id, j, rule_id,
					rule_nr, rule, inner_rule, s_acl_entries[i].entries))
				{
					SSDK_DEBUG("\n{%d, 0x%x, 0x%x, 0x%x, 0x%x},\n",
						s_acl_entries[i].num, s_acl_entries[i].ext_1,
						s_acl_entries[i].ext_2, s_acl_entries[i].ext_4,
						s_acl_entries[i].entries);
					*index = i;
					*hw_list_index = j;
					return SW_OK;
				}
			}
		}
	}
	return SW_NO_RESOURCE;
}

static sw_error_t
_adpt_hppe_acl_l2_fields_check(a_uint32_t dev_id, a_uint32_t rule_id, a_uint32_t rule_nr,
				fal_acl_rule_t * rule, ADPT_HPPE_ACL_RULE_MAP *rule_map)
{
	a_uint32_t l2_rule_type_map = 0;
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_DA))
	{
		l2_rule_type_map |= (1<<ADPT_ACL_HPPE_MAC_DA_RULE);
	}
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_SA))
	{
		l2_rule_type_map |= (1<<ADPT_ACL_HPPE_MAC_SA_RULE);
	}
	if((FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_STAG_PRI)) ||
		(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_STAG_DEI)) ||
		(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_STAGGED)) ||
		(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_CTAG_VID)) ||
		(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_CTAG_PRI)) ||
		(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_CTAG_CFI)) ||
		(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_CTAGGED)) ||
		(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_VSI)) ||
		(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_VSI_VALID)))
	{
		l2_rule_type_map |= (1<<ADPT_ACL_HPPE_VLAN_RULE);
	}

	if((FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_ETHTYPE)) ||
		(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_PPPOE_SESSIONID)) ||
		((FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_STAG_VID)) &&
		(rule->stag_vid_op != FAL_ACL_FIELD_MASK)))
	{
		l2_rule_type_map |= (1<<ADPT_ACL_HPPE_L2_MISC_RULE);
	}

	if(!((l2_rule_type_map & (1<<ADPT_ACL_HPPE_VLAN_RULE))||
		(l2_rule_type_map & (1<<ADPT_ACL_HPPE_L2_MISC_RULE))))
	{
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_STAG_VID))
		{
			l2_rule_type_map |= (1<<ADPT_ACL_HPPE_VLAN_RULE);
		}
	}

	if(l2_rule_type_map == 0)
	{
		if((FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_SNAP)) ||
			(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_ETHERNET)) ||
			(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_FAKE_MAC_HEADER)))
		{
			l2_rule_type_map |= (1<<ADPT_ACL_HPPE_MAC_DA_RULE);
		}
	}

	rule_map->rule_type_map |= l2_rule_type_map;

	/* inverse MAC_DA rule count */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_DA))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_MAC_DA_RULE] ++;

	/* inverse MAC_SA rule count */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_SA))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_MAC_SA_RULE] ++;

	/* inverse L2_MISC rule count */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_ETHTYPE))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_L2_MISC_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_PPPOE_SESSIONID))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_L2_MISC_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_STAG_VID))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_L2_MISC_RULE] ++;

	/* inverse VLAN rule count */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_STAG_PRI))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_VLAN_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_CTAG_VID))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_VLAN_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_CTAG_PRI))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_VLAN_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_VSI))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_VLAN_RULE] ++;

	return SW_OK;
}
static sw_error_t
_adpt_hppe_acl_ipv4_fields_check(a_uint32_t dev_id, a_uint32_t rule_id, a_uint32_t rule_nr,
				fal_acl_rule_t * rule, ADPT_HPPE_ACL_RULE_MAP *rule_map)
{
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L4_SPORT) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP4_SIP) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_ICMP_TYPE) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_ICMP_CODE))
	{
		rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV4_SIP_RULE);
	}

	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L4_DPORT) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP4_DIP))
	{
		rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV4_DIP_RULE);
	}

	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L3_LENGTH) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP_DSCP) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_FIRST_FRAGMENT) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_TCP_FLAG) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IPV4_OPTION) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L3_TTL) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L3_FRAGMENT) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_AH_HEADER) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_ESP_HEADER) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP_PROTO))
	{
		rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPMISC_RULE);
	}
	if((!(rule_map->rule_type_map & (1<<ADPT_ACL_HPPE_IPV4_DIP_RULE))) &&
		(!(rule_map->rule_type_map & (1<<ADPT_ACL_HPPE_IPV4_SIP_RULE))))
	{/*both dip and sip rule are not selected, but ip_pkt_type field selected*/
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP_PKT_TYPE))
		{
			rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV4_DIP_RULE);
		}
	}
	if(rule_map->rule_type_map == 0)
	{
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L3_FRAGMENT))
		{
			rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV4_DIP_RULE);
		}
	}

	/* inverse IPV4_SIP rule */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_L4_SPORT))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV4_SIP_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_IP4_SIP))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV4_SIP_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_ICMP_TYPE))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV4_SIP_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_ICMP_CODE))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV4_SIP_RULE] ++;

	/* inverse IPV4_DIP rule */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_L4_DPORT))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV4_DIP_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_IP4_DIP))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV4_DIP_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_IP_PKT_TYPE))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV4_DIP_RULE] ++;

	/* inverse IPMISC rule */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_L3_LENGTH))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPMISC_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_IP_DSCP))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPMISC_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_TCP_FLAG))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPMISC_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_L3_TTL))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPMISC_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_IP_PROTO))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPMISC_RULE] ++;

	return SW_OK;
}

static sw_error_t
_adpt_hppe_acl_ipv6_fields_check(a_uint32_t dev_id, a_uint32_t rule_id, a_uint32_t rule_nr,
				fal_acl_rule_t * rule, ADPT_HPPE_ACL_RULE_MAP *rule_map)
{
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L4_SPORT) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_ICMP_TYPE) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_ICMP_CODE))
	{
		rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV6_SIP2_RULE);
	}

	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP6_SIP))
	{
		if(rule->src_ip6_mask.ul[3] != 0 || rule->src_ip6_mask.ul[2]&0x0000ffff)
			rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV6_SIP0_RULE);
		if(rule->src_ip6_mask.ul[1] != 0 || rule->src_ip6_mask.ul[2]&0xffff0000)
			rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV6_SIP1_RULE);
		if(rule->src_ip6_mask.ul[0] != 0 )
			rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV6_SIP2_RULE);
	}
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L4_DPORT))
	{
		rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV6_DIP2_RULE);
	}
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP6_DIP))
	{
		if(rule->dest_ip6_mask.ul[3] != 0 || rule->dest_ip6_mask.ul[2]&0x0000ffff)
			rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV6_DIP0_RULE);
		if(rule->dest_ip6_mask.ul[1] != 0 || rule->dest_ip6_mask.ul[2]&0xffff0000)
			rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV6_DIP1_RULE);
		if(rule->dest_ip6_mask.ul[0] != 0 )
			rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV6_DIP2_RULE);
	}

	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L3_LENGTH) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP_DSCP) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_FIRST_FRAGMENT) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_TCP_FLAG) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L3_TTL) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L3_FRAGMENT)||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_ESP_HEADER) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MOBILITY_HEADER) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_FRAGMENT_HEADER)||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_OTHER_EXT_HEADER)||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_AH_HEADER) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP_PROTO))
	{
		rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPMISC_RULE);
	}

	if((!(rule_map->rule_type_map & (1<<ADPT_ACL_HPPE_IPV6_DIP0_RULE))) &&
		(!(rule_map->rule_type_map & (1<<ADPT_ACL_HPPE_IPV6_DIP1_RULE))) &&
		(!(rule_map->rule_type_map & (1<<ADPT_ACL_HPPE_IPV6_DIP2_RULE))) &&
		(!(rule_map->rule_type_map & (1<<ADPT_ACL_HPPE_IPV6_SIP0_RULE))) &&
		(!(rule_map->rule_type_map & (1<<ADPT_ACL_HPPE_IPV6_SIP1_RULE))) &&
		(!(rule_map->rule_type_map & (1<<ADPT_ACL_HPPE_IPV6_SIP2_RULE))))
	{/*both dip and sip rule are not selected, but ip_pkt_type field selected*/
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP_PKT_TYPE))
		{
			rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV6_DIP0_RULE);
		}
	}

	if(rule_map->rule_type_map == 0)
	{
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L3_FRAGMENT))
		{
			rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_IPV6_DIP0_RULE);
		}
	}

	/* inverse IPV6 SIP rule */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_L4_SPORT))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV6_SIP2_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_ICMP_TYPE))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV6_SIP2_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_ICMP_CODE))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV6_SIP2_RULE] ++;

	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_IP6_SIP))
	{
		if(rule->src_ip6_mask.ul[3] != 0 || rule->src_ip6_mask.ul[2]&0x0000ffff)
			rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV6_SIP0_RULE] ++;
		if(rule->src_ip6_mask.ul[1] != 0 || rule->src_ip6_mask.ul[2]&0xffff0000)
			rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV6_SIP1_RULE] ++;
		if(rule->src_ip6_mask.ul[0] != 0 )
			rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV6_SIP2_RULE] ++;
	}

	/* inverse IPV6 DIP rule */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_L4_DPORT))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV6_DIP2_RULE] ++;

	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_IP_PKT_TYPE))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV6_DIP0_RULE] ++;

	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_IP6_DIP))
	{
		if(rule->dest_ip6_mask.ul[3] != 0 || rule->dest_ip6_mask.ul[2]&0x0000ffff)
			rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV6_DIP0_RULE] ++;
		if(rule->dest_ip6_mask.ul[1] != 0 || rule->dest_ip6_mask.ul[2]&0xffff0000)
			rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV6_DIP1_RULE] ++;
		if(rule->dest_ip6_mask.ul[0] != 0 )
			rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPV6_DIP2_RULE] ++;
	}

	/* inverse IPMISC rule */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_L3_LENGTH))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPMISC_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_IP_DSCP))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPMISC_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_TCP_FLAG))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPMISC_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_L3_TTL))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPMISC_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_IP_PROTO))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_IPMISC_RULE] ++;

	return SW_OK;
}

static sw_error_t
_adpt_hppe_acl_udf_fields_check(a_uint32_t dev_id, a_uint32_t rule_id, a_uint32_t rule_nr,
				fal_acl_rule_t * rule, ADPT_HPPE_ACL_RULE_MAP *rule_map)
{
	a_uint32_t udf_rule_type_map = 0;

	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF0))
	{
		udf_rule_type_map |= (1<<ADPT_ACL_HPPE_UDF0_RULE);
	}
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF3))
	{
		udf_rule_type_map |= (1<<ADPT_ACL_HPPE_UDF1_RULE);
	}
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF1) &&
		rule->udf1_op != FAL_ACL_FIELD_MASK)
	{
		udf_rule_type_map |= (1<<ADPT_ACL_HPPE_UDF1_RULE);
	}

	if(udf_rule_type_map == 0)
	{
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF1) ||
			FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF2))
		{
			udf_rule_type_map |= (1<<ADPT_ACL_HPPE_UDF0_RULE);
		}
	}
	rule_map->rule_type_map |= udf_rule_type_map;

	/* inverse UDF rule */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_UDF0))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_UDF0_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_UDF3))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_UDF1_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_UDF1))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_UDF1_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_UDF2))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_UDF1_RULE] ++;

	return SW_OK;
}

sw_error_t _adpt_hppe_acl_mac_rule_sw_2_hw(fal_acl_rule_t *rule, a_uint32_t is_mac_da,
		ADPT_HPPE_ACL_MAC_RULE * macrule, ADPT_HPPE_ACL_MAC_RULE_MASK *macrule_mask,
		a_uint8_t inverse_en)
{
	fal_acl_field_map_t field_flg = {0};

	if (inverse_en)
		FAL_FIELD_FLG_CPY(field_flg, rule->inverse_field_flg);
	else
		FAL_FIELD_FLG_CPY(field_flg, rule->field_flg);

	if(is_mac_da)
	{
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MAC_DA))
		{
			macrule->mac_addr_0 = rule->dest_mac_val.uc[5] | \
							rule->dest_mac_val.uc[4] << 8 | \
							rule->dest_mac_val.uc[3] << 16 | \
							rule->dest_mac_val.uc[2] << 24;
			macrule->mac_addr_1 = rule->dest_mac_val.uc[1] | \
							rule->dest_mac_val.uc[0] << 8;

			macrule_mask->mac_addr_mask_0 = rule->dest_mac_mask.uc[5] | \
							rule->dest_mac_mask.uc[4] << 8 | \
							rule->dest_mac_mask.uc[3] << 16 | \
							rule->dest_mac_mask.uc[2] << 24;
			macrule_mask->mac_addr_mask_1 = rule->dest_mac_mask.uc[1] | \
							rule->dest_mac_mask.uc[0] << 8;

			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_DA);
				return SW_OK;
			}
		}
	}
	else
	{
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MAC_SA))
		{
			macrule->mac_addr_0 = rule->src_mac_val.uc[5] | \
							rule->src_mac_val.uc[4] << 8 | \
							rule->src_mac_val.uc[3] << 16 | \
							rule->src_mac_val.uc[2] << 24;
			macrule->mac_addr_1 = rule->src_mac_val.uc[1] | \
							rule->src_mac_val.uc[0] << 8;

			macrule_mask->mac_addr_mask_0 = rule->src_mac_mask.uc[5] | \
							rule->src_mac_mask.uc[4] << 8 | \
							rule->src_mac_mask.uc[3] << 16 | \
							rule->src_mac_mask.uc[2] << 24;
			macrule_mask->mac_addr_mask_1 = rule->src_mac_mask.uc[1] | \
							rule->src_mac_mask.uc[0] << 8;

			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_SA);
				return SW_OK;
			}
		}
	}

	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP))
	{
		macrule->is_ip = rule->is_ip_val;
		macrule_mask->is_ip_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IPV6))
	{
		macrule->is_ipv6 = rule->is_ipv6_val;
		macrule_mask->is_ipv6_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_ETHERNET))
	{
		macrule->is_ethernet = rule->is_ethernet_val;
		macrule_mask->is_ethernet_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_SNAP))
	{
		macrule->is_snap = rule->is_snap_val;
		macrule_mask->is_snap_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_FAKE_MAC_HEADER))
	{
		macrule->is_fake_mac_header = rule->is_fake_mac_header_val;
		macrule_mask->is_fake_mac_header_mask = 1;
	}

	return SW_OK;
}

sw_error_t _adpt_hppe_acl_vlan_rule_sw_2_hw(fal_acl_rule_t *rule,
	ADPT_HPPE_ACL_VLAN_RULE * vlanrule, ADPT_HPPE_ACL_VLAN_RULE_MASK *vlanrule_mask,
	a_uint8_t *range_en, a_uint8_t inverse_en)
{
	fal_acl_field_map_t field_flg = {0};

	if (inverse_en)
		FAL_FIELD_FLG_CPY(field_flg, rule->inverse_field_flg);
	else
		FAL_FIELD_FLG_CPY(field_flg, rule->field_flg);

	/*ctag*/
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MAC_CTAG_VID))
	{
		if(FAL_ACL_FIELD_MASK == rule->ctag_vid_op)
		{
			vlanrule->cvid = rule->ctag_vid_val;
			vlanrule_mask->cvid_mask = rule->ctag_vid_mask;
		}
		else
		{
			a_uint16_t min, max;
			if(FAL_ACL_FIELD_LE == rule->ctag_vid_op)
			{
				min = 0;
				max = rule->ctag_vid_val;
			}
			else if(FAL_ACL_FIELD_GE == rule->ctag_vid_op)
			{
				min = rule->ctag_vid_val;
				max = 0xfff;
			}
			else if(FAL_ACL_FIELD_RANGE == rule->ctag_vid_op)
			{
				min = rule->ctag_vid_val;
				max = rule->ctag_vid_mask;
			}
			else
				return SW_NOT_SUPPORTED;
			vlanrule->cvid = min;
			vlanrule_mask->cvid_mask = max;
			*range_en = 1;
		}
		if (inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_CTAG_VID);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MAC_CTAG_PRI))
	{
		vlanrule->cpcp = rule->ctag_pri_val;
		vlanrule_mask->cpcp_mask = rule->ctag_pri_mask;
		if (inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_CTAG_PRI);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MAC_CTAG_CFI))
	{
		vlanrule->cdei = rule->ctag_cfi_val;
		vlanrule_mask->cdei_mask = rule->ctag_cfi_mask;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MAC_CTAGGED))
	{
		vlanrule->ctag_fmt = rule->ctagged_val;
		vlanrule_mask->ctag_fmt_mask = rule->ctagged_mask;
	}
	/*stag*/
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MAC_STAG_VID) &&
		(rule->stag_vid_op == FAL_ACL_FIELD_MASK))
	{
		vlanrule->svid = rule->stag_vid_val;
		vlanrule_mask->svid_mask = rule->stag_vid_mask;
		if (inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_STAG_VID);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MAC_STAG_PRI))
	{
#if defined(APPE)
		vlanrule->spcp_0 = rule->stag_pri_val;
		vlanrule->spcp_1 = rule->stag_pri_val>>VLAN_RULE_SPCP_0_LEN;
		vlanrule_mask->spcp_mask_0 = rule->stag_pri_mask;
		vlanrule_mask->spcp_mask_1 = rule->stag_pri_mask>>VLAN_RULE_SPCP_MASK_0_LEN;
#else
		vlanrule->spcp = rule->stag_pri_val;
		vlanrule_mask->spcp_mask = rule->stag_pri_mask;
#endif
		if (inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_STAG_PRI);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MAC_STAG_DEI))
	{
		vlanrule->sdei = rule->stag_dei_val;
		vlanrule_mask->sdei_mask = rule->stag_dei_mask;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MAC_STAGGED))
	{
		vlanrule->stag_fmt = rule->stagged_val;
		vlanrule_mask->stag_fmt_mask = rule->stagged_mask;
	}
	/*vsi*/
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_VSI_VALID))
	{
		vlanrule->vsi_valid= rule->vsi_valid;
		vlanrule_mask->vsi_valid_mask = rule->vsi_valid_mask;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_VSI))
	{
		vlanrule->vsi= rule->vsi;
		vlanrule_mask->vsi_mask = rule->vsi_mask;
		if (inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_VSI);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP))
	{
		vlanrule->is_ip = rule->is_ip_val;
		vlanrule_mask->is_ip_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IPV6))
	{
		vlanrule->is_ipv6 = rule->is_ipv6_val;
		vlanrule_mask->is_ipv6_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_ETHERNET))
	{
		vlanrule->is_ethernet = rule->is_ethernet_val;
		vlanrule_mask->is_ethernet_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_SNAP))
	{
		vlanrule->is_snap = rule->is_snap_val;
		vlanrule_mask->is_snap_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_FAKE_MAC_HEADER))
	{
		vlanrule->is_fake_mac_header = rule->is_fake_mac_header_val;
		vlanrule_mask->is_fake_mac_header_mask = 1;
	}

	return SW_OK;
}


sw_error_t _adpt_hppe_acl_l2_misc_rule_sw_2_hw(fal_acl_rule_t *rule,
	ADPT_HPPE_ACL_L2MISC_RULE * l2misc_rule, ADPT_HPPE_ACL_L2MISC_RULE_MASK * l2misc_mask,
	a_uint8_t *range_en, a_uint8_t inverse_en)
{
	fal_acl_field_map_t field_flg = {0};

	if (inverse_en)
		FAL_FIELD_FLG_CPY(field_flg, rule->inverse_field_flg);
	else
		FAL_FIELD_FLG_CPY(field_flg, rule->field_flg);

	/*stag*/
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MAC_STAG_VID))
	{
		if(FAL_ACL_FIELD_MASK == rule->stag_vid_op)
		{
			l2misc_rule->svid = rule->stag_vid_val;
			l2misc_mask->svid_mask = rule->stag_vid_mask;
		}
		else
		{
			a_uint16_t min, max;
			if(FAL_ACL_FIELD_LE == rule->stag_vid_op)
			{
				min = 0;
				max = rule->stag_vid_val;
			}
			else if(FAL_ACL_FIELD_GE == rule->stag_vid_op)
			{
				min = rule->stag_vid_val;
				max = 0xfff;
			}
			else if(FAL_ACL_FIELD_RANGE == rule->stag_vid_op)
			{
				min = rule->stag_vid_val;
				max = rule->stag_vid_mask;
			}
			else
			{
				return SW_NOT_SUPPORTED;
			}
			l2misc_rule->svid = min;
			l2misc_mask->svid_mask = max;
			*range_en = 1;
		}
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_STAG_VID);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MAC_ETHTYPE))
	{
		l2misc_rule->l2prot = rule->ethtype_val;
		l2misc_mask->l2prot_mask = rule->ethtype_mask;
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_MAC_ETHTYPE);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_PPPOE_SESSIONID))
	{
		l2misc_rule->pppoe_sessionid = rule->pppoe_sessionid;
		l2misc_mask->pppoe_sessionid_mask = rule->pppoe_sessionid_mask;
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_PPPOE_SESSIONID);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP))
	{
		l2misc_rule->is_ip = rule->is_ip_val;
		l2misc_mask->is_ip_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IPV6))
	{
		l2misc_rule->is_ipv6 = rule->is_ipv6_val;
		l2misc_mask->is_ipv6_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_ETHERNET))
	{
		l2misc_rule->is_ethernet = rule->is_ethernet_val;
		l2misc_mask->is_ethernet_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_SNAP))
	{
		l2misc_rule->is_snap = rule->is_snap_val;
		l2misc_mask->is_snap_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_FAKE_MAC_HEADER))
	{
		l2misc_rule->is_fake_mac_header = rule->is_fake_mac_header_val;
		l2misc_mask->is_fake_mac_header_mask = 1;
	}

	return SW_OK;
}

sw_error_t _adpt_hppe_acl_ipv4_rule_sw_2_hw(fal_acl_rule_t *rule, a_uint32_t is_ip_da,
	ADPT_HPPE_ACL_IPV4_RULE * ipv4rule, ADPT_HPPE_ACL_IPV4_RULE_MASK *ipv4rule_mask,
	a_uint8_t *range_en, a_uint8_t inverse_en)
{
	fal_acl_field_map_t field_flg = {0};

	if (inverse_en)
		FAL_FIELD_FLG_CPY(field_flg, rule->inverse_field_flg);
	else
		FAL_FIELD_FLG_CPY(field_flg, rule->field_flg);

	if(is_ip_da)
	{
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP4_DIP))
		{
			ipv4rule->ip_0 = rule->dest_ip4_val&0xffff;
			ipv4rule->ip_1 = (rule->dest_ip4_val>>16)&0xffff;
			ipv4rule_mask->ip_mask_0 = rule->dest_ip4_mask&0xffff;
			ipv4rule_mask->ip_mask_1 = (rule->dest_ip4_mask)>>16&0xffff;
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_IP4_DIP);
				return SW_OK;
			}
		}
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_L4_DPORT))
		{
			if(FAL_ACL_FIELD_MASK == rule->dest_l4port_op)
			{
				ipv4rule->l4_port = rule->dest_l4port_val;
				ipv4rule_mask->l4_port_mask = rule->dest_l4port_mask;
			}
			else
			{
				a_uint16_t min, max;
				if(FAL_ACL_FIELD_LE == rule->dest_l4port_op)
				{
					min = 0;
					max = rule->dest_l4port_val;
				}
				else if(FAL_ACL_FIELD_GE == rule->dest_l4port_op)
				{
					min = rule->dest_l4port_val;
					max = 0xffff;
				}
				else if(FAL_ACL_FIELD_RANGE == rule->dest_l4port_op)
				{
					min = rule->dest_l4port_val;
					max = rule->dest_l4port_mask;
				}
				else
					return SW_NOT_SUPPORTED;
				ipv4rule->l4_port = min;
				ipv4rule_mask->l4_port_mask = max;
				*range_en = 1;
			}
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_L4_DPORT);
				return SW_OK;
			}
		}
	}
	else
	{
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP4_SIP))
		{
			ipv4rule->ip_0 = rule->src_ip4_val&0xffff;
			ipv4rule->ip_1 = (rule->src_ip4_val>>16)&0xffff;
			ipv4rule_mask->ip_mask_0 = rule->src_ip4_mask&0xffff;
			ipv4rule_mask->ip_mask_1 = (rule->src_ip4_mask>>16)&0xffff;
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_IP4_SIP);
				return SW_OK;
			}
		}
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_L4_SPORT))
		{
			if(FAL_ACL_FIELD_MASK == rule->src_l4port_op)
			{
				ipv4rule->l4_port = rule->src_l4port_val;
				ipv4rule_mask->l4_port_mask = rule->src_l4port_mask;
			}
			else
			{
				a_uint16_t min, max;
				if(FAL_ACL_FIELD_LE == rule->src_l4port_op)
				{
					min = 0;
					max = rule->src_l4port_val;
				}
				else if(FAL_ACL_FIELD_GE == rule->src_l4port_op)
				{
					min = rule->src_l4port_val;
					max = 0xffff;
				}
				else if(FAL_ACL_FIELD_RANGE == rule->src_l4port_op)
				{
					min = rule->src_l4port_val;
					max = rule->src_l4port_mask;
				}
				else
					return SW_NOT_SUPPORTED;
				ipv4rule->l4_port = min;
				ipv4rule_mask->l4_port_mask = max;
				*range_en = 1;
			}
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_L4_SPORT);
				return SW_OK;
			}
		}
	}

	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_ICMP_CODE) ||
		FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_ICMP_TYPE))
	{
		if(FAL_ACL_FIELD_MASK == rule->icmp_type_code_op)
		{
			ipv4rule->l4_port = (rule->icmp_type_val<<8)|rule->icmp_code_val;
			ipv4rule_mask->l4_port_mask = (rule->icmp_type_mask<<8)|rule->icmp_code_mask;
		}
		else
		{
			a_uint16_t min, max;
			if(FAL_ACL_FIELD_LE == rule->icmp_type_code_op)
			{
				min = 0;
				max = (rule->icmp_type_val<<8)|rule->icmp_code_val;
			}
			else if(FAL_ACL_FIELD_GE == rule->icmp_type_code_op)
			{
				min = (rule->icmp_type_val<<8)|rule->icmp_code_val;
				max = 0xffff;
			}
			else if(FAL_ACL_FIELD_RANGE == rule->icmp_type_code_op)
			{
				min = (rule->icmp_type_val<<8)|rule->icmp_code_val;
				max = (rule->icmp_type_mask<<8)|rule->icmp_code_mask;
			}
			else
				return SW_NOT_SUPPORTED;
			ipv4rule->l4_port = min;
			ipv4rule_mask->l4_port_mask = max;
			*range_en = 1;
		}
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_ICMP_CODE);
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_ICMP_TYPE);
			return SW_OK;
		}
	}

	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP))
	{
		ipv4rule->is_ip = rule->is_ip_val;
		ipv4rule_mask->is_ip_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_L3_FRAGMENT))
	{
		ipv4rule->l3_fragment = rule->is_fragement_val;
		ipv4rule_mask->l3_fragment_mask = rule->is_fragement_mask;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP_PKT_TYPE))
	{
		ipv4rule->l3_packet_type = rule->l3_pkt_type;
		ipv4rule_mask->l3_packet_type_mask = rule->l3_pkt_type_mask;
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_IP_PKT_TYPE);
			return SW_OK;
		}
	}

	return SW_OK;
}

/*ip_bit_range: 0 mean DIP0 or SIP0, 1 mean DIP1 or SIP1, 2 mean DIP2 or SIP2,*/
sw_error_t _adpt_hppe_acl_ipv6_rule_sw_2_hw(fal_acl_rule_t *rule, a_uint32_t is_ip_da,
	a_uint32_t ip_bit_range, ADPT_HPPE_ACL_IPV6_RULE *ipv6rule,
	ADPT_HPPE_ACL_IPV6_RULE_MASK *ipv6rule_mask, a_uint8_t *range_en, a_uint8_t inverse_en)
{
	fal_acl_field_map_t field_flg = {0};

	if (inverse_en)
		FAL_FIELD_FLG_CPY(field_flg, rule->inverse_field_flg);
	else
		FAL_FIELD_FLG_CPY(field_flg, rule->field_flg);

	if(is_ip_da)
	{
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP6_DIP))
		{
			if(ip_bit_range == 0)
			{
				ipv6rule->ip_port = rule->dest_ip6_val.ul[3]&0xffff;
				ipv6rule->ip_ext_1 = (rule->dest_ip6_val.ul[3]>>16)&0xffff;
				ipv6rule->ip_ext_2 = (rule->dest_ip6_val.ul[2])&0xffff;
				ipv6rule_mask->ip_port_mask = rule->dest_ip6_mask.ul[3]&0xffff;
				ipv6rule_mask->ip_ext_1_mask = (rule->dest_ip6_mask.ul[3]>>16)&0xffff;
				ipv6rule_mask->ip_ext_2_mask = (rule->dest_ip6_mask.ul[2])&0xffff;
			}
			else if(ip_bit_range == 1)
			{
				ipv6rule->ip_port = (rule->dest_ip6_val.ul[2]>>16)&0xffff;
				ipv6rule->ip_ext_1 = (rule->dest_ip6_val.ul[1])&0xffff;
				ipv6rule->ip_ext_2 = (rule->dest_ip6_val.ul[1]>>16)&0xffff;
				ipv6rule_mask->ip_port_mask = (rule->dest_ip6_mask.ul[2]>>16)&0xffff;
				ipv6rule_mask->ip_ext_1_mask = (rule->dest_ip6_mask.ul[1])&0xffff;
				ipv6rule_mask->ip_ext_2_mask = (rule->dest_ip6_mask.ul[1]>>16)&0xffff;
			}
			else if(ip_bit_range == 2)
			{
				ipv6rule->ip_ext_1 = (rule->dest_ip6_val.ul[0])&0xffff;
				ipv6rule->ip_ext_2 = (rule->dest_ip6_val.ul[0]>>16)&0xffff;
				ipv6rule_mask->ip_ext_1_mask = (rule->dest_ip6_mask.ul[0])&0xffff;
				ipv6rule_mask->ip_ext_2_mask = (rule->dest_ip6_mask.ul[0]>>16)&0xffff;
			}
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_IP6_DIP);
				return SW_OK;
			}
		}
		if((ip_bit_range == 2) && (FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_L4_DPORT)))
		{
			if(FAL_ACL_FIELD_MASK == rule->dest_l4port_op)
			{
				ipv6rule->ip_port = rule->dest_l4port_val;
				ipv6rule_mask->ip_port_mask = rule->dest_l4port_mask;
			}
			else
			{
				a_uint16_t min, max;
				if(FAL_ACL_FIELD_LE == rule->dest_l4port_op)
				{
					min = 0;
					max = rule->dest_l4port_val;
				}
				else if(FAL_ACL_FIELD_GE == rule->dest_l4port_op)
				{
					min = rule->dest_l4port_val;
					max = 0xffff;
				}
				else if(FAL_ACL_FIELD_RANGE == rule->dest_l4port_op)
				{
					min = rule->dest_l4port_val;
					max = rule->dest_l4port_mask;
				}
				else
					return SW_NOT_SUPPORTED;
				ipv6rule->ip_port = min;
				ipv6rule_mask->ip_port_mask = max;
				*range_en = 1;
			}
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_L4_DPORT);
				return SW_OK;
			}
		}
	}
	else
	{
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP6_SIP))
		{
			if(ip_bit_range == 0)
			{
				ipv6rule->ip_port = rule->src_ip6_val.ul[3]&0xffff;
				ipv6rule->ip_ext_1 = (rule->src_ip6_val.ul[3]>>16)&0xffff;
				ipv6rule->ip_ext_2 = (rule->src_ip6_val.ul[2])&0xffff;
				ipv6rule_mask->ip_port_mask = rule->src_ip6_mask.ul[3]&0xffff;
				ipv6rule_mask->ip_ext_1_mask = (rule->src_ip6_mask.ul[3]>>16)&0xffff;
				ipv6rule_mask->ip_ext_2_mask = (rule->src_ip6_mask.ul[2])&0xffff;
			}
			else if(ip_bit_range == 1)
			{
				ipv6rule->ip_port = (rule->src_ip6_val.ul[2]>>16)&0xffff;
				ipv6rule->ip_ext_1 = (rule->src_ip6_val.ul[1])&0xffff;
				ipv6rule->ip_ext_2 = (rule->src_ip6_val.ul[1]>>16)&0xffff;
				ipv6rule_mask->ip_port_mask = (rule->src_ip6_mask.ul[2]>>16)&0xffff;
				ipv6rule_mask->ip_ext_1_mask = (rule->src_ip6_mask.ul[1])&0xffff;
				ipv6rule_mask->ip_ext_2_mask = (rule->src_ip6_mask.ul[1]>>16)&0xffff;
			}
			else if(ip_bit_range == 2)
			{
				ipv6rule->ip_ext_1 = (rule->src_ip6_val.ul[0])&0xffff;
				ipv6rule->ip_ext_2 = (rule->src_ip6_val.ul[0]>>16)&0xffff;
				ipv6rule_mask->ip_ext_1_mask = (rule->src_ip6_mask.ul[0])&0xffff;
				ipv6rule_mask->ip_ext_2_mask = (rule->src_ip6_mask.ul[0]>>16)&0xffff;
			}
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_IP6_SIP);
				return SW_OK;
			}
		}
		if((ip_bit_range == 2) && (FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_L4_SPORT)))
		{
			if(FAL_ACL_FIELD_MASK == rule->src_l4port_op)
			{
				ipv6rule->ip_port = rule->src_l4port_val;
				ipv6rule_mask->ip_port_mask = rule->src_l4port_mask;
			}
			else
			{
				a_uint16_t min, max;
				if(FAL_ACL_FIELD_LE == rule->src_l4port_op)
				{
					min = 0;
					max = rule->src_l4port_val;
				}
				else if(FAL_ACL_FIELD_GE == rule->src_l4port_op)
				{
					min = rule->src_l4port_val;
					max = 0xffff;
				}
				else if(FAL_ACL_FIELD_RANGE == rule->src_l4port_op)
				{
					min = rule->src_l4port_val;
					max = rule->src_l4port_mask;
				}
				else
					return SW_NOT_SUPPORTED;
				ipv6rule->ip_port = min;
				ipv6rule_mask->ip_port_mask = max;
				*range_en = 1;
			}
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_L4_SPORT);
				return SW_OK;
			}
		}
	}

	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_ICMP_CODE) ||
		FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_ICMP_TYPE))
	{
		if(FAL_ACL_FIELD_MASK == rule->icmp_type_code_op)
		{
			ipv6rule->ip_port = (rule->icmp_type_val<<8)|rule->icmp_code_val;
			ipv6rule_mask->ip_port_mask = (rule->icmp_type_mask<<8)|rule->icmp_code_mask;
		}
		else
		{
			a_uint16_t min, max;
			if(FAL_ACL_FIELD_LE == rule->icmp_type_code_op)
			{
				min = 0;
				max = (rule->icmp_type_val<<8)|rule->icmp_code_val;
			}
			else if(FAL_ACL_FIELD_GE == rule->icmp_type_code_op)
			{
				min = (rule->icmp_type_val<<8)|rule->icmp_code_val;
				max = 0xffff;
			}
			else if(FAL_ACL_FIELD_RANGE == rule->icmp_type_code_op)
			{
				min = (rule->icmp_type_val<<8)|rule->icmp_code_val;
				max = (rule->icmp_type_mask<<8)|rule->icmp_code_mask;
			}
			else
				return SW_NOT_SUPPORTED;
			ipv6rule->ip_port = min;
			ipv6rule_mask->ip_port_mask = max;
			*range_en = 1;
		}
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_ICMP_CODE);
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_ICMP_TYPE);
			return SW_OK;
		}	
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_L3_FRAGMENT))
	{
		ipv6rule->l3_fragment = rule->is_fragement_val;
		ipv6rule_mask->l3_fragment_mask = rule->is_fragement_mask;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP_PKT_TYPE))
	{
		ipv6rule->l3_packet_type = rule->l3_pkt_type;
		ipv6rule_mask->l3_packet_type_mask = rule->l3_pkt_type_mask;
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_IP_PKT_TYPE);
			return SW_OK;
		}
	}
	return SW_OK;
}

sw_error_t _adpt_hppe_acl_ipmisc_rule_sw_2_hw(fal_acl_rule_t *rule,
	ADPT_HPPE_ACL_IPMISC_RULE * ipmisc_rule, ADPT_HPPE_ACL_IPMISC_RULE_MASK *ipmisc_mask,
	a_uint8_t *range_en, a_uint8_t inverse_en)
{
	fal_acl_field_map_t field_flg = {0};

	if (inverse_en)
		FAL_FIELD_FLG_CPY(field_flg, rule->inverse_field_flg);
	else
		FAL_FIELD_FLG_CPY(field_flg, rule->field_flg);

	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_L3_LENGTH))
	{
		if(FAL_ACL_FIELD_MASK == rule->l3_length_op)
		{
			ipmisc_rule->l3_length = rule->l3_length;
			ipmisc_mask->l3_length_mask = rule->l3_length_mask;
		}
		else
		{
			a_uint16_t min, max;
			if(FAL_ACL_FIELD_LE == rule->l3_length_op)
			{
				min = 0;
				max = rule->l3_length;
			}
			else if(FAL_ACL_FIELD_GE == rule->l3_length_op)
			{
				min = rule->l3_length;
				max = 0xffff;
			}
			else if(FAL_ACL_FIELD_RANGE == rule->l3_length_op)
			{
				min = rule->l3_length;
				max = rule->l3_length_mask;
			}
			else
				return SW_NOT_SUPPORTED;
			ipmisc_rule->l3_length = min;
			ipmisc_mask->l3_length_mask = max;
			*range_en = 1;
		}
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_L3_LENGTH);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP_PROTO))
	{
		ipmisc_rule->l3_prot = rule->ip_proto_val;
		ipmisc_mask->l3_prot_mask = rule->ip_proto_mask;
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_IP_PROTO);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP_DSCP))
	{
		ipmisc_rule->l3_dscp_tc = rule->ip_dscp_val;
		ipmisc_mask->l3_dscp_tc_mask = rule->ip_dscp_mask;
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_IP_DSCP);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_FIRST_FRAGMENT))
	{
		ipmisc_rule->first_fragment = rule->is_first_frag_val;
		ipmisc_mask->first_fragment_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_TCP_FLAG))
	{
		ipmisc_rule->tcp_flags = rule->tcp_flag_val;
		ipmisc_mask->tcp_flags_mask = rule->tcp_flag_mask;
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_TCP_FLAG);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IPV4_OPTION))
	{
		ipmisc_rule->ipv4_option_state = rule->is_ipv4_option_val;
		ipmisc_mask->ipv4_option_state_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_L3_TTL))
	{
		ipmisc_rule->l3_ttl = rule->l3_ttl;
		ipmisc_mask->l3_ttl_mask = rule->l3_ttl_mask;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_AH_HEADER))
	{
		ipmisc_rule->ah_header = rule->is_ah_header_val;
		ipmisc_mask->ah_header_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_ESP_HEADER))
	{
		ipmisc_rule->esp_header = rule->is_esp_header_val;
		ipmisc_mask->esp_header_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_MOBILITY_HEADER))
	{
		ipmisc_rule->mobility_header = rule->is_mobility_header_val;
		ipmisc_mask->mobility_header_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_FRAGMENT_HEADER))
	{
		ipmisc_rule->fragment_header = rule->is_fragment_header_val;
		ipmisc_mask->fragment_header_mask= 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_OTHER_EXT_HEADER))
	{
		ipmisc_rule->other_header = rule->is_other_header_val;
		ipmisc_mask->other_header_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IPV6))
	{
		ipmisc_rule->is_ipv6 = rule->is_ipv6_val;
		ipmisc_mask->is_ipv6_mask = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_L3_FRAGMENT))
	{
		ipmisc_rule->l3_fragment = rule->is_fragement_val;
		ipmisc_mask->l3_fragment_mask = 1;
	}
	return SW_OK;
}

sw_error_t _adpt_hppe_acl_udf_rule_sw_2_hw(fal_acl_rule_t *rule, a_uint32_t is_win1,
	ADPT_HPPE_ACL_UDF_RULE * udfrule, ADPT_HPPE_ACL_UDF_RULE_MASK *udfrule_mask,
	a_uint8_t *range_en, a_uint8_t inverse_en)
{
	fal_acl_field_map_t field_flg = {0};

	if (inverse_en)
		FAL_FIELD_FLG_CPY(field_flg, rule->inverse_field_flg);
	else
		FAL_FIELD_FLG_CPY(field_flg, rule->field_flg);

	if(is_win1)
	{
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_UDF3))
		{
			udfrule->udf2_valid = 1;
			udfrule->udf2 = rule->udf3_val;
			udfrule_mask->udf2_valid = 1;
			udfrule_mask->udf2_mask = rule->udf3_mask;
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_UDF3);
				return SW_OK;
			}
		}
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_UDF2) &&
			FAL_ACL_FIELD_MASK == rule->udf2_op)
		{
			udfrule->udf1_valid = 1;
			udfrule->udf1 = rule->udf2_val;
			udfrule_mask->udf1_valid = 1;
			udfrule_mask->udf1_mask = rule->udf2_mask;
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_UDF2);
				return SW_OK;
			}
		}

		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_UDF1))
		{
			udfrule->udf0_valid = 1;
			udfrule_mask->udf0_valid = 1;
			if(FAL_ACL_FIELD_MASK == rule->udf1_op)
			{
				udfrule->udf0 = rule->udf1_val;
				udfrule_mask->udf0_mask = rule->udf1_mask;
			}
			else
			{
				a_uint16_t min, max;
				if(FAL_ACL_FIELD_LE == rule->udf1_op)
				{
					min = 0;
					max = rule->udf1_val;
				}
				else if(FAL_ACL_FIELD_GE == rule->udf1_op)
				{
					min = rule->udf1_val;
					max = 0xffff;
				}
				else if(FAL_ACL_FIELD_RANGE == rule->udf1_op)
				{
					min = rule->udf1_val;
					max = rule->udf1_mask;
				}
				else
					return SW_NOT_SUPPORTED;
				udfrule->udf0 = min;
				udfrule_mask->udf0_mask = max;
				*range_en = 1;
			}
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_UDF1);
				return SW_OK;
			}
		}
	}
	else
	{
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_UDF1) &&
			FAL_ACL_FIELD_MASK == rule->udf1_op)
		{
			udfrule->udf1_valid = 1;
			udfrule->udf1 = rule->udf1_val;
			udfrule_mask->udf1_valid = 1;
			udfrule_mask->udf1_mask = rule->udf1_mask;
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_UDF1);
				return SW_OK;
			}
		}
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_UDF2) &&
			FAL_ACL_FIELD_MASK == rule->udf2_op)
		{
			udfrule->udf2_valid = 1;
			udfrule->udf2 = rule->udf2_val;
			udfrule_mask->udf2_valid = 1;
			udfrule_mask->udf2_mask = rule->udf2_mask;
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_UDF2);
				return SW_OK;
			}
		}

		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_UDF0))
		{
			udfrule->udf0_valid = 1;
			udfrule_mask->udf0_valid = 1;
			if(FAL_ACL_FIELD_MASK == rule->udf0_op)
			{
				udfrule->udf0 = rule->udf0_val;
				udfrule_mask->udf0_mask = rule->udf0_mask;
			}
			else
			{
				a_uint16_t min, max;
				if(FAL_ACL_FIELD_LE == rule->udf0_op)
				{
					min = 0;
					max = rule->udf0_val;
				}
				else if(FAL_ACL_FIELD_GE == rule->udf0_op)
				{
					min = rule->udf0_val;
					max = 0xffff;
				}
				else if(FAL_ACL_FIELD_RANGE == rule->udf0_op)
				{
					min = rule->udf0_val;
					max = rule->udf0_mask;
				}
				else
					return SW_NOT_SUPPORTED;
				udfrule->udf0 = min;
				udfrule_mask->udf0_mask = max;
				*range_en = 1;
			}
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_UDF0);
				return SW_OK;
			}
		}
	}

	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP))
	{
		udfrule->is_ip = rule->is_ip_val;
		udfrule_mask->is_ip = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IPV6))
	{
		udfrule->is_ipv6= rule->is_ipv6_val;
		udfrule_mask->is_ipv6 = 1;
	}
	return SW_OK;
}
static sw_error_t
_adpt_hppe_acl_action_sw_2_hw(a_uint32_t dev_id,fal_acl_rule_t *rule, union ipo_action_u *hw_act)
{
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REDPT))
	{
		a_uint32_t dest_type = FAL_ACL_DEST_TYPE(rule->ports);
		a_uint32_t dest_val = FAL_ACL_DEST_VALUE(rule->ports);

		SSDK_DEBUG("rule->ports = %x\n", rule->ports);

		hw_act->bf.dest_info_change_en = 1;
		if(dest_type == FAL_ACL_DEST_NEXTHOP) /*nexthop*/
		{
			hw_act->bf.dest_info =
				HPPE_ACL_DEST_INFO(HPPE_ACL_DEST_NEXTHOP, dest_val);
		}
		else if(FAL_ACL_DEST_TYPE(rule->ports) == FAL_ACL_DEST_PORT_ID)/*vp*/
		{
			hw_act->bf.dest_info =
				HPPE_ACL_DEST_INFO(HPPE_ACL_DEST_PORT_ID, dest_val);
		}
		else if(FAL_ACL_DEST_TYPE(rule->ports) == FAL_ACL_DEST_PORT_BMP)/*bitmap*/
		{
			hw_act->bf.dest_info =
				HPPE_ACL_DEST_INFO(HPPE_ACL_DEST_PORT_BMP, dest_val);
		}
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_PERMIT))
	{
		hw_act->bf.dest_info_change_en = 1;
		hw_act->bf.fwd_cmd = 0;/*forward*/
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_DENY))
	{
		hw_act->bf.dest_info_change_en = 1;
		hw_act->bf.fwd_cmd = 1;/*drop*/
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_CPYCPU))
	{
		hw_act->bf.dest_info_change_en = 1;
		hw_act->bf.fwd_cmd = 2;/*copy to cpu*/
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_RDTCPU))
	{
		hw_act->bf.dest_info_change_en = 1;
		hw_act->bf.fwd_cmd = 3;/*redirect to cpu*/
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_MIRROR))
	{
		hw_act->bf.mirror_en= 1;
	}
	hw_act->bf.bypass_bitmap_0 = rule->bypass_bitmap & 0x3fff;
	hw_act->bf.bypass_bitmap_1 = (rule->bypass_bitmap>>14) & 0x3ffff;
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_VID))
	{
		hw_act->bf.svid_change_en = 1;
		hw_act->bf.stag_fmt = rule->stag_fmt;
		hw_act->bf.svid = rule->stag_vid;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_PRI))
	{
		hw_act->bf.stag_pcp_change_en = 1;
		hw_act->bf.stag_pcp = rule->stag_pri;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_DEI))
	{
		hw_act->bf.stag_dei_change_en = 1;
		hw_act->bf.stag_dei = rule->stag_dei;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_VID))
	{
		hw_act->bf.cvid_change_en = 1;
		hw_act->bf.ctag_fmt = rule->ctag_fmt;
		hw_act->bf.cvid = rule->ctag_vid;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_PRI))
	{
		hw_act->bf.ctag_pcp_change_en = 1;
		hw_act->bf.ctag_pcp_0 = rule->ctag_pri&0x3;
		hw_act->bf.ctag_pcp_1 = (rule->ctag_pri>>2)&0x1;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_CFI))
	{
		hw_act->bf.ctag_dei_change_en = 1;
		hw_act->bf.ctag_dei = rule->ctag_cfi;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_DSCP))
	{
		hw_act->bf.dscp_tc_change_en = 1;
		hw_act->bf.dscp_tc = rule->dscp;
#if defined(CPPE) || defined(APPE)
		if(adpt_ppe_type_get(dev_id) == CPPE_TYPE ||
			adpt_chip_type_get(dev_id) == CHIP_APPE)
		{
			hw_act->bf.dscp_tc_mask = rule->dscp_mask;
		}
#endif
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_INT_DP))
	{
		hw_act->bf.int_dp_change_en = 1;
		hw_act->bf.int_dp = rule->int_dp;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_POLICER_EN))
	{
		hw_act->bf.policer_en = 1;
		hw_act->bf.policer_index = rule->policer_ptr;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_QUEUE))
	{
		hw_act->bf.qid_en = 1;
		hw_act->bf.qid = rule->queue;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_ENQUEUE_PRI))
	{
		hw_act->bf.enqueue_pri_change_en = 1;
		hw_act->bf.enqueue_pri = rule->enqueue_pri;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_SERVICE_CODE))
	{
		hw_act->bf.service_code_en = 1;
		hw_act->bf.service_code_0 = rule->service_code&0x1;
		hw_act->bf.service_code_1 = (rule->service_code>>1)&0x7f;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_SYN_TOGGLE))
	{
		hw_act->bf.syn_toggle = 1;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_CPU_CODE))
	{
		hw_act->bf.cpu_code_en = 1;
		hw_act->bf.cpu_code = rule->cpu_code;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_METADATA_EN))
	{
		hw_act->bf.metadata_en = 1;
#if defined(MPPE)
		hw_act->bf.metadata_pri_0 = rule->metadata_pri&0x7;
		hw_act->bf.metadata_pri_1 = (rule->metadata_pri>>3)&0x1;
#endif
	}
#if defined(CPPE) || defined(APPE)
	if(adpt_ppe_type_get(dev_id) == CPPE_TYPE ||
		adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
		hw_act->bf.qos_res_prec = rule->qos_res_prec;
	}
#endif
	return SW_OK;
}

sw_error_t
_adpt_hppe_acl_rule_sw_2_hw(a_uint32_t dev_id, fal_acl_rule_t * rule, a_uint32_t rule_type,
	a_uint32_t *hw_entry, a_uint32_t *allocated_entries, a_uint8_t *range_en,
	void *hw_rule, void *hw_rule_mask, a_uint8_t inverse_en)
{
	*range_en = 0;
	if(rule_type == ADPT_ACL_HPPE_VLAN_RULE)
	{
		_adpt_hppe_acl_vlan_rule_sw_2_hw(rule, (ADPT_HPPE_ACL_VLAN_RULE *)hw_rule,
		(ADPT_HPPE_ACL_VLAN_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_MAC_DA_RULE)
	{
		_adpt_hppe_acl_mac_rule_sw_2_hw(rule, 1, (ADPT_HPPE_ACL_MAC_RULE *)hw_rule,
		(ADPT_HPPE_ACL_MAC_RULE_MASK *)hw_rule_mask, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_MAC_SA_RULE)
	{
		_adpt_hppe_acl_mac_rule_sw_2_hw(rule, 0, (ADPT_HPPE_ACL_MAC_RULE *)hw_rule,
		(ADPT_HPPE_ACL_MAC_RULE_MASK *)hw_rule_mask, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_L2_MISC_RULE)
	{
		_adpt_hppe_acl_l2_misc_rule_sw_2_hw(rule,
		(ADPT_HPPE_ACL_L2MISC_RULE *)hw_rule,
		(ADPT_HPPE_ACL_L2MISC_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_IPV4_DIP_RULE)
	{
		_adpt_hppe_acl_ipv4_rule_sw_2_hw(rule, 1,
		(ADPT_HPPE_ACL_IPV4_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV4_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_IPV4_SIP_RULE)
	{
		_adpt_hppe_acl_ipv4_rule_sw_2_hw(rule, 0,
		(ADPT_HPPE_ACL_IPV4_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV4_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_IPV6_DIP0_RULE)
	{
		_adpt_hppe_acl_ipv6_rule_sw_2_hw(rule, 1, 0,
		(ADPT_HPPE_ACL_IPV6_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV6_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_IPV6_DIP1_RULE)
	{
		_adpt_hppe_acl_ipv6_rule_sw_2_hw(rule, 1, 1,
		(ADPT_HPPE_ACL_IPV6_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV6_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_IPV6_DIP2_RULE)
	{
		_adpt_hppe_acl_ipv6_rule_sw_2_hw(rule, 1, 2,
		(ADPT_HPPE_ACL_IPV6_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV6_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_IPV6_SIP0_RULE)
	{
		_adpt_hppe_acl_ipv6_rule_sw_2_hw(rule, 0, 0,
		(ADPT_HPPE_ACL_IPV6_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV6_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_IPV6_SIP1_RULE)
	{
		_adpt_hppe_acl_ipv6_rule_sw_2_hw(rule, 0, 1,
		(ADPT_HPPE_ACL_IPV6_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV6_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_IPV6_SIP2_RULE)
	{
		_adpt_hppe_acl_ipv6_rule_sw_2_hw(rule, 0, 2,
		(ADPT_HPPE_ACL_IPV6_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPV6_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_IPMISC_RULE)
	{
		_adpt_hppe_acl_ipmisc_rule_sw_2_hw(rule,
		(ADPT_HPPE_ACL_IPMISC_RULE *)hw_rule,
		(ADPT_HPPE_ACL_IPMISC_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_UDF0_RULE)
	{
		_adpt_hppe_acl_udf_rule_sw_2_hw(rule, 0,
		(ADPT_HPPE_ACL_UDF_RULE *)hw_rule,
		(ADPT_HPPE_ACL_UDF_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_HPPE_UDF1_RULE)
	{
		_adpt_hppe_acl_udf_rule_sw_2_hw(rule, 1,
		(ADPT_HPPE_ACL_UDF_RULE *)hw_rule,
		(ADPT_HPPE_ACL_UDF_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
#if defined(APPE)
	else if(rule_type == ADPT_ACL_APPE_EXT_UDF0_RULE)
	{
		_adpt_appe_acl_ext_udf_rule_sw_2_hw(rule, 0,
		(ADPT_APPE_ACL_EXT_UDF_RULE *)hw_rule,
		(ADPT_APPE_ACL_EXT_UDF_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_APPE_EXT_UDF1_RULE)
	{
		_adpt_appe_acl_ext_udf_rule_sw_2_hw(rule, 1,
		(ADPT_APPE_ACL_EXT_UDF_RULE *)hw_rule,
		(ADPT_APPE_ACL_EXT_UDF_RULE_MASK *)hw_rule_mask, range_en, inverse_en);
	}
	else if(rule_type == ADPT_ACL_APPE_TUNNEL_RULE)
	{
		_adpt_appe_pre_acl_tunnel_rule_sw_2_hw(&rule->tunnel_info,
		(ADPT_APPE_ACL_TUNNEL_RULE *)hw_rule,
		(ADPT_APPE_ACL_TUNNEL_RULE_MASK *)hw_rule_mask, inverse_en);
	}
#endif

	if(*range_en == 0)
	{
		*hw_entry = _acl_bit_index(*allocated_entries, ADPT_ACL_ENTRY_NUM_PER_LIST, 1);
	}
	if(*hw_entry == 0xff || *range_en)
	{
		*hw_entry = _acl_bit_index(*allocated_entries, ADPT_ACL_ENTRY_NUM_PER_LIST, 2);
	}
	if(*hw_entry == 0xff)
	{
		printk("%s, %d, fail find hw_entry in 0x%x\n",
			__FUNCTION__, __LINE__, *allocated_entries);
		return SW_NO_RESOURCE;
	}
	*allocated_entries &= (~(1<<(*hw_entry)));

	return SW_OK;
}

sw_error_t
_adpt_hppe_acl_rule_hw_add(a_uint32_t dev_id, a_uint32_t list_pri,
		a_uint32_t hw_list_id, a_uint32_t rule_id, a_uint32_t rule_nr,
		fal_acl_rule_t * rule, ADPT_HPPE_ACL_RULE_MAP *rule_map,
		a_uint32_t allocated_entries)
{
	union ipo_rule_reg_u hw_reg = {0};
	union ipo_mask_reg_u hw_mask = {0};
	union ipo_action_u hw_act = {0};
	sw_error_t rv = 0;
	a_uint32_t hw_entry = 0, rule_type = 0;
	a_uint8_t range_en = 0;

	hw_reg.bf.post_routing_en = rule->post_routing;
	hw_reg.bf.res_chain = rule->acl_pool;
	hw_reg.bf.pri = (list_pri<<3)|rule->pri;

	for(rule_type = 0; rule_type < ADPT_ACL_HPPE_RULE_TYPE_NUM; rule_type++)
	{
		if ((BIT(rule_type)) & rule_map->rule_type_map) {
			hw_reg.bf.rule_field_0 = 0;
			hw_reg.bf.rule_field_1 = 0;
			memset(&hw_mask, 0, sizeof(hw_mask));
			memset(&hw_act, 0, sizeof(hw_act));

			/*set 53bit rule fields of hw rule reg*/
			_adpt_hppe_acl_rule_sw_2_hw(dev_id, rule, rule_type,
				&hw_entry, &allocated_entries, &range_en,
				&hw_reg, &hw_mask, 0);
			/*set rule_type, range_en, inverse_en fields of hw rule reg*/
			hw_reg.bf.rule_type = rule_type;
			hw_reg.bf.range_en = range_en;
			hw_reg.bf.inverse_en = 0;
			if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_INVERSE_ALL))
				hw_reg.bf.inverse_en = 1;

			SSDK_DEBUG("post_route %d, chain %d, pri %d, src_1 %d, src_0 %d, src_type %d"
				" rule_type %d, inverse %d, range %d\n", hw_reg.bf.post_routing_en,
				hw_reg.bf.res_chain, hw_reg.bf.pri, hw_reg.bf.src_1,
				hw_reg.bf.src_0, hw_reg.bf.src_type, hw_reg.bf.rule_type,
				hw_reg.bf.inverse_en, hw_reg.bf.range_en);
			SSDK_DEBUG("rule and mask set hw_entry = %d\n",
					hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST + hw_entry);

			_adpt_hppe_acl_action_sw_2_hw(dev_id,rule, &hw_act);
			/*_adpt_acl_reg_dump((a_uint8_t *)&hw_act, sizeof(hw_act));*/
			rv |= hppe_ipo_action_set(dev_id,
				hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST + hw_entry, &hw_act);
			/*_adpt_acl_reg_dump((a_uint8_t *)&hw_reg, sizeof(hw_reg));*/
			rv |= hppe_ipo_rule_reg_set(dev_id,
				hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST + hw_entry, &hw_reg);
			/*_adpt_acl_reg_dump((a_uint8_t *)&hw_mask, sizeof(hw_mask));*/
			rv |= hppe_ipo_mask_reg_set(dev_id,
				hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST + hw_entry, &hw_mask);
			SW_RTN_ON_ERROR(rv);
		}

		/* inverse rule */
		for (; rule_map->inverse_rule_type_count[rule_type] > 0;
				rule_map->inverse_rule_type_count[rule_type] --) {
			hw_reg.bf.rule_field_0 = 0;
			hw_reg.bf.rule_field_1 = 0;
			memset(&hw_mask, 0, sizeof(hw_mask));
			memset(&hw_act, 0, sizeof(hw_act));

			/*set 53bit rule fields of hw rule reg*/
			_adpt_hppe_acl_rule_sw_2_hw(dev_id, rule, rule_type,
				&hw_entry, &allocated_entries, &range_en,
				&hw_reg, &hw_mask, 1);

			/*set rule_type, range_en, inverse_en fields of hw rule reg*/
			hw_reg.bf.rule_type = rule_type;
			hw_reg.bf.range_en = range_en;
			hw_reg.bf.inverse_en = 1;

			SSDK_DEBUG("rule_type %d inverse_rule_type_count %d rule_hw_entry %d\n",
				rule_type, rule_map->inverse_rule_type_count[rule_type],
				hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST + hw_entry);

			_adpt_hppe_acl_action_sw_2_hw(dev_id, rule, &hw_act);

			/*_adpt_acl_reg_dump((a_uint8_t *)&hw_act, sizeof(hw_act));*/
			rv |= hppe_ipo_action_set(dev_id,
				hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST + hw_entry, &hw_act);
			/*_adpt_acl_reg_dump((a_uint8_t *)&hw_reg, sizeof(hw_reg));*/
			rv |= hppe_ipo_rule_reg_set(dev_id,
				hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST + hw_entry, &hw_reg);
			/*_adpt_acl_reg_dump((a_uint8_t *)&hw_mask, sizeof(hw_mask));*/
			rv |= hppe_ipo_mask_reg_set(dev_id,
				hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST + hw_entry, &hw_mask);
			SW_RTN_ON_ERROR(rv);
		}
	}

	return SW_OK;
}

static sw_error_t
_adpt_ppe_acl_rule_hw_add(a_uint32_t dev_id, a_uint32_t list_pri,
		a_uint32_t hw_list_id, a_uint32_t rule_id, a_uint32_t rule_nr,
		fal_acl_rule_t * rule, fal_acl_rule_t *inner_rule,
		ADPT_HPPE_ACL_RULE_MAP *rule_map,
		ADPT_HPPE_ACL_RULE_MAP *inner_rule_map,
		a_uint32_t allocated_entries)
{
	sw_error_t rv = SW_OK;

	if (hw_list_id < ADPT_ACL_HW_LIST_NUM)
	{
		rv = _adpt_hppe_acl_rule_hw_add(dev_id, list_pri, hw_list_id,
			rule_id, rule_nr, rule, rule_map, allocated_entries);
		SW_RTN_ON_ERROR(rv);
	}
#if defined(APPE)
	else if (hw_list_id < ADPT_ACL_HW_LIST_NUM + ADPT_PRE_ACL_HW_LIST_NUM)
	{
		rv = _adpt_appe_pre_acl_rule_hw_add(dev_id, list_pri,
			hw_list_id - ADPT_ACL_HW_LIST_NUM, rule_id, rule_nr, rule, inner_rule,
			rule_map, inner_rule_map, allocated_entries);
		SW_RTN_ON_ERROR(rv);
	}
	rv = _adpt_appe_acl_ext_set(dev_id, rule, hw_list_id, allocated_entries);
#endif
	return rv;
}

static sw_error_t
_adpt_hppe_acl_hw_list_resort(a_uint32_t dev_id, a_uint32_t hw_list_index, a_bool_t move_up)
{
	a_uint32_t i = 0, start = 0, end = ADPT_ACL_HW_LIST_NUM;
	ADPT_HPPE_ACL_HW_LIST temp = {0};
#if defined(APPE)
	if(hw_list_index >= ADPT_ACL_HW_LIST_NUM)
	{
		start = ADPT_ACL_HW_LIST_NUM;
		end += ADPT_PRE_ACL_HW_LIST_NUM;
	}
#endif
	if(hw_list_index >= end)
	{
		return SW_OUT_OF_RANGE;
	}

	if(move_up)
	{
		temp.hw_list_id = g_acl_hw_list[dev_id][hw_list_index].hw_list_id;
		temp.free_hw_entry_bitmap =
			g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_bitmap;
		temp.free_hw_entry_count =
			g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_count;
		for(i = hw_list_index; i > start; i--)
		{
			if(temp.free_hw_entry_count <
				g_acl_hw_list[dev_id][i-1].free_hw_entry_count)
			{
				g_acl_hw_list[dev_id][i].hw_list_id =
					g_acl_hw_list[dev_id][i-1].hw_list_id;
				g_acl_hw_list[dev_id][i].free_hw_entry_bitmap =
					g_acl_hw_list[dev_id][i-1].free_hw_entry_bitmap;
				g_acl_hw_list[dev_id][i].free_hw_entry_count =
					g_acl_hw_list[dev_id][i-1].free_hw_entry_count;
			}
			else
			{
				break;
			}
		}
		g_acl_hw_list[dev_id][i].hw_list_id = temp.hw_list_id;
		g_acl_hw_list[dev_id][i].free_hw_entry_bitmap = temp.free_hw_entry_bitmap;
		g_acl_hw_list[dev_id][i].free_hw_entry_count = temp.free_hw_entry_count;
	}
	else
	{
		temp.hw_list_id = g_acl_hw_list[dev_id][hw_list_index].hw_list_id;
		temp.free_hw_entry_bitmap =
			g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_bitmap;
		temp.free_hw_entry_count =
			g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_count;
		for(i = hw_list_index; i < end-1; i++)
		{
			if(temp.free_hw_entry_count >
				g_acl_hw_list[dev_id][i+1].free_hw_entry_count)
			{
				g_acl_hw_list[dev_id][i].hw_list_id =
					g_acl_hw_list[dev_id][i+1].hw_list_id;
				g_acl_hw_list[dev_id][i].free_hw_entry_bitmap =
					g_acl_hw_list[dev_id][i+1].free_hw_entry_bitmap;
				g_acl_hw_list[dev_id][i].free_hw_entry_count =
					g_acl_hw_list[dev_id][i+1].free_hw_entry_count;
			}
			else
			{
				break;
			}
		}
		g_acl_hw_list[dev_id][i].hw_list_id = temp.hw_list_id;
		g_acl_hw_list[dev_id][i].free_hw_entry_bitmap = temp.free_hw_entry_bitmap;
		g_acl_hw_list[dev_id][i].free_hw_entry_count = temp.free_hw_entry_count;
	}
	return SW_OK;
}

static sw_error_t
_adpt_hppe_acl_rule_ext_set(a_uint32_t dev_id, a_uint32_t hw_list_id,
		a_uint32_t ext_1, a_uint32_t ext_2, a_uint32_t ext_4)
{
	sw_error_t rv = SW_OK;
	union rule_ext_1_reg_u reg_ext_1 = {0};
	union rule_ext_2_reg_u reg_ext_2 = {0};
	union rule_ext_4_reg_u reg_ext_4 = {0};

	if(ext_1 != 0)
	{
		rv |= hppe_rule_ext_1_reg_get(dev_id, hw_list_id, &reg_ext_1);
		reg_ext_1.val |= ext_1;
		SSDK_DEBUG("ext_1.val = 0x%x\n", reg_ext_1.val);
		rv |= hppe_rule_ext_1_reg_set(dev_id, hw_list_id, &reg_ext_1);
	}
	if(ext_2 != 0)
	{
		rv |= hppe_rule_ext_2_reg_get(dev_id, hw_list_id, &reg_ext_2);
		reg_ext_2.val |= ext_2;
		SSDK_DEBUG("ext_2.val = 0x%x\n", reg_ext_2.val);
		rv |= hppe_rule_ext_2_reg_set(dev_id, hw_list_id, &reg_ext_2);
	}
	if(ext_4 != 0)
	{
		rv |= hppe_rule_ext_4_reg_get(dev_id, hw_list_id, &reg_ext_4);
		reg_ext_4.val |= ext_4;
		SSDK_DEBUG("ext_4.val = 0x%x\n", reg_ext_4.val);
		rv |= hppe_rule_ext_4_reg_set(dev_id, hw_list_id, &reg_ext_4);
	}
	return rv;
}

static sw_error_t
_adpt_hppe_acl_rule_ext_clear(a_uint32_t dev_id, a_uint32_t hw_list_id,
		a_uint32_t ext_1, a_uint32_t ext_2, a_uint32_t ext_4)
{
	sw_error_t rv = SW_OK;
	union rule_ext_1_reg_u reg_ext_1 = {0};
	union rule_ext_2_reg_u reg_ext_2 = {0};
	union rule_ext_4_reg_u reg_ext_4 = {0};

	if(ext_1 != 0)
	{
		rv |= hppe_rule_ext_1_reg_get(dev_id, hw_list_id, &reg_ext_1);
		reg_ext_1.val &= (~ext_1);
		SSDK_DEBUG("ext_1.val = 0x%x\n", reg_ext_1.val);
		rv |= hppe_rule_ext_1_reg_set(dev_id, hw_list_id, &reg_ext_1);
	}
	if(ext_2 != 0)
	{
		rv |= hppe_rule_ext_2_reg_get(dev_id, hw_list_id, &reg_ext_2);
		reg_ext_2.val &= (~ext_2);
		SSDK_DEBUG("ext_2.val = 0x%x\n", reg_ext_2.val);
		rv |= hppe_rule_ext_2_reg_set(dev_id, hw_list_id, &reg_ext_2);
	}
	if(ext_4 != 0)
	{
		rv |= hppe_rule_ext_4_reg_get(dev_id, hw_list_id, &reg_ext_4);
		reg_ext_4.val &= (~ext_4);
		SSDK_DEBUG("ext_4.val = 0x%x\n", reg_ext_4.val);
		rv |= hppe_rule_ext_4_reg_set(dev_id, hw_list_id, &reg_ext_4);
	}
	return rv;
}

static sw_error_t
_adpt_ppe_acl_rule_ext_set(a_uint32_t dev_id, a_uint32_t hw_list_id,
		a_uint32_t ext_1, a_uint32_t ext_2, a_uint32_t ext_4)
{
	if(hw_list_id < ADPT_ACL_HW_LIST_NUM)
	{
		return _adpt_hppe_acl_rule_ext_set(dev_id, hw_list_id, ext_1, ext_2, ext_4);
	}
#if defined(APPE)
	else if(hw_list_id < ADPT_ACL_HW_LIST_NUM + ADPT_PRE_ACL_HW_LIST_NUM)
	{
		return _adpt_appe_pre_acl_rule_ext_set(dev_id, hw_list_id - ADPT_ACL_HW_LIST_NUM,
				ext_1, ext_2, ext_4);
	}
#endif
	return SW_OK;
}

static sw_error_t
_adpt_ppe_acl_rule_ext_clear(a_uint32_t dev_id, a_uint32_t hw_list_id,
		a_uint32_t ext_1, a_uint32_t ext_2, a_uint32_t ext_4)
{
	if(hw_list_id < ADPT_ACL_HW_LIST_NUM)
	{
		return _adpt_hppe_acl_rule_ext_clear(dev_id, hw_list_id, ext_1, ext_2, ext_4);
	}
#if defined(APPE)
	else if(hw_list_id < ADPT_ACL_HW_LIST_NUM + ADPT_PRE_ACL_HW_LIST_NUM)
	{
		return _adpt_appe_pre_acl_rule_ext_clear(dev_id, hw_list_id - ADPT_ACL_HW_LIST_NUM,
				ext_1, ext_2, ext_4);
	}
#endif
	return SW_OK;
}

void acl_rule_field_convert(fal_acl_rule_t * rule,
            fal_acl_rule_field_t * rule_field, a_bool_t to_inner)
{
    if(to_inner)
    {
        /*fields flag*/
        aos_mem_copy(rule_field->field_flg,
            rule->field_flg, sizeof(fal_acl_field_map_t));
	aos_mem_copy(rule_field->inverse_field_flg,
            rule->inverse_field_flg, sizeof(fal_acl_field_map_t));

        /*mac fields*/
        rule_field->is_fake_mac_header_mask = rule->is_fake_mac_header_mask;
        rule_field->is_fake_mac_header_val = rule->is_fake_mac_header_val;
        rule_field->is_snap_mask = rule->is_snap_mask;
        rule_field->is_snap_val = rule->is_snap_val;
        rule_field->is_ethernet_mask = rule->is_ethernet_mask;
        rule_field->is_ethernet_val = rule->is_ethernet_val;
        rule_field->is_ip_mask = rule->is_ip_mask;
        rule_field->is_ip_val= rule->is_ip_val;
        rule_field->is_ipv6_mask = rule->is_ipv6_mask;
        rule_field->is_ipv6_val = rule->is_ipv6_val;
        aos_mem_copy(rule_field->dest_mac_val.uc,
            rule->dest_mac_val.uc, sizeof(fal_mac_addr_t));
        aos_mem_copy(rule_field->dest_mac_mask.uc,
            rule->dest_mac_mask.uc, sizeof(fal_mac_addr_t));
        aos_mem_copy(rule_field->src_mac_val.uc,
            rule->src_mac_val.uc, sizeof(fal_mac_addr_t));
        aos_mem_copy(rule_field->src_mac_mask.uc,
            rule->src_mac_mask.uc, sizeof(fal_mac_addr_t));
        rule_field->ethtype_val = rule->ethtype_val;
        rule_field->ethtype_mask = rule->ethtype_mask;
        rule_field->stagged_val = rule->stagged_val;
        rule_field->stagged_mask = rule->stagged_mask;
        rule_field->stag_vid_op = rule->stag_vid_op;
        rule_field->stag_vid_val = rule->stag_vid_val;
        rule_field->stag_vid_mask = rule->stag_vid_mask;
        rule_field->stag_pri_val = rule->stag_pri_val;
        rule_field->stag_pri_mask = rule->stag_pri_mask;
        rule_field->stag_dei_val = rule->stag_dei_val;
        rule_field->stag_dei_mask = rule->stag_dei_mask;
        rule_field->ctagged_val = rule->ctagged_val;
        rule_field->ctagged_mask = rule->ctagged_mask;
        rule_field->ctag_vid_op = rule->ctag_vid_op;
        rule_field->ctag_vid_val = rule->ctag_vid_val;
        rule_field->ctag_vid_mask = rule->ctag_vid_mask;
        rule_field->ctag_pri_val = rule->ctag_pri_val;
        rule_field->ctag_pri_mask = rule->ctag_pri_mask;
        rule_field->ctag_cfi_val = rule->ctag_cfi_val;
        rule_field->ctag_cfi_mask = rule->ctag_cfi_mask;
        rule_field->pppoe_sessionid = rule->pppoe_sessionid;
        rule_field->pppoe_sessionid_mask = rule->pppoe_sessionid_mask;

        /*ipv4 fields*/
        rule_field->src_ip4_val = rule->src_ip4_val;
        rule_field->src_ip4_mask = rule->src_ip4_mask;
        rule_field->dest_ip4_val = rule->dest_ip4_val;
        rule_field->dest_ip4_mask = rule->dest_ip4_mask;
        rule_field->is_ipv4_option_mask = rule->is_ipv4_option_mask;
        rule_field->is_ipv4_option_val = rule->is_ipv4_option_val;

        /*ipv6 fields*/
        aos_mem_copy(rule_field->src_ip6_val.ul,
            rule->src_ip6_val.ul, sizeof(fal_ip6_addr_t));
        aos_mem_copy(rule_field->src_ip6_mask.ul,
            rule->src_ip6_mask.ul, sizeof(fal_ip6_addr_t));
        aos_mem_copy(rule_field->dest_ip6_val.ul,
            rule->dest_ip6_val.ul, sizeof(fal_ip6_addr_t));
        aos_mem_copy(rule_field->dest_ip6_mask.ul,
            rule->dest_ip6_mask.ul, sizeof(fal_ip6_addr_t));
        rule_field->is_mobility_header_mask = rule->is_mobility_header_mask;
        rule_field->is_mobility_header_val = rule->is_mobility_header_val;
        rule_field->is_fragment_header_mask = rule->is_fragment_header_mask;
        rule_field->is_fragment_header_val = rule->is_fragment_header_val;
        rule_field->is_other_header_mask = rule->is_other_header_mask;
        rule_field->is_other_header_val = rule->is_other_header_val;

        /*ip fields*/
        rule_field->ip_proto_val = rule->ip_proto_val;
        rule_field->ip_proto_mask = rule->ip_proto_mask;
        rule_field->ip_dscp_val = rule->ip_dscp_val;
        rule_field->ip_dscp_mask = rule->ip_dscp_mask;
        rule_field->dest_l4port_op = rule->dest_l4port_op;
        rule_field->dest_l4port_val = rule->dest_l4port_val;
        rule_field->dest_l4port_mask = rule->dest_l4port_mask;
        rule_field->src_l4port_op = rule->src_l4port_op;
        rule_field->src_l4port_val = rule->src_l4port_val;
        rule_field->src_l4port_mask = rule->src_l4port_mask;
        rule_field->tcp_flag_val = rule->tcp_flag_val;
        rule_field->tcp_flag_mask = rule->tcp_flag_mask;
        rule_field->icmp_type_code_op = rule->icmp_type_code_op;
        rule_field->icmp_type_val = rule->icmp_type_val;
        rule_field->icmp_type_mask = rule->icmp_type_mask;
        rule_field->icmp_code_val = rule->icmp_code_val;
        rule_field->icmp_code_mask = rule->icmp_code_mask;
        rule_field->is_fragement_mask = rule->is_fragement_mask;
        rule_field->is_fragement_val = rule->is_fragement_val;
        rule_field->is_first_frag_mask = rule->is_first_frag_mask;
        rule_field->is_first_frag_val = rule->is_first_frag_val;
        rule_field->l3_ttl_mask = rule->l3_ttl_mask;
        rule_field->l3_ttl = rule->l3_ttl;
        rule_field->l3_length_op = rule->l3_length_op;
        rule_field->l3_length = rule->l3_length;
        rule_field->l3_length_mask = rule->l3_length_mask;
        rule_field->l3_pkt_type_mask = rule->l3_pkt_type_mask;
        rule_field->l3_pkt_type = rule->l3_pkt_type;
        rule_field->is_ah_header_mask = rule->is_ah_header_mask;
        rule_field->is_ah_header_val = rule->is_ah_header_val;
        rule_field->is_esp_header_mask = rule->is_esp_header_mask;
        rule_field->is_esp_header_val = rule->is_esp_header_val;

        /*udf fields*/
        rule_field->udf0_op = rule->udf0_op;
        rule_field->udf0_val = rule->udf0_val;
        rule_field->udf0_mask = rule->udf0_mask;
        rule_field->udf1_op = rule->udf1_op;
        rule_field->udf1_val = rule->udf1_val;
        rule_field->udf1_mask = rule->udf1_mask;
        rule_field->udf2_op = rule->udf2_op;
        rule_field->udf2_val = rule->udf2_val;
        rule_field->udf2_mask = rule->udf2_mask;
        rule_field->udf3_val = rule->udf3_val;
        rule_field->udf3_mask = rule->udf3_mask;
        rule_field->udfprofile_val = rule->udfprofile_val;
        rule_field->udfprofile_mask = rule->udfprofile_mask;
    }
    else
    {
        /*fields flag*/
        aos_mem_copy(rule->field_flg,
            rule_field->field_flg, sizeof(fal_acl_field_map_t));
	aos_mem_copy(rule->inverse_field_flg,
            rule_field->inverse_field_flg, sizeof(fal_acl_field_map_t));

        /*mac fields*/
        rule->is_fake_mac_header_mask = rule_field->is_fake_mac_header_mask;
        rule->is_fake_mac_header_val = rule_field->is_fake_mac_header_val;
        rule->is_snap_mask = rule_field->is_snap_mask;
        rule->is_snap_val = rule_field->is_snap_val;
        rule->is_ethernet_mask = rule_field->is_ethernet_mask;
        rule->is_ethernet_val = rule_field->is_ethernet_val;
        rule->is_ip_mask = rule_field->is_ip_mask;
        rule->is_ip_val = rule_field->is_ip_val;
        rule->is_ipv6_mask = rule_field->is_ipv6_mask;
        rule->is_ipv6_val = rule_field->is_ipv6_val;
        aos_mem_copy(rule->dest_mac_val.uc,
            rule_field->dest_mac_val.uc, sizeof(fal_mac_addr_t));
        aos_mem_copy(rule->dest_mac_mask.uc,
            rule_field->dest_mac_mask.uc, sizeof(fal_mac_addr_t));
        aos_mem_copy(rule->src_mac_val.uc,
            rule_field->src_mac_val.uc, sizeof(fal_mac_addr_t));
        aos_mem_copy(rule->src_mac_mask.uc,
            rule_field->src_mac_mask.uc, sizeof(fal_mac_addr_t));
        rule->ethtype_val = rule_field->ethtype_val;

        rule->ethtype_mask = rule_field->ethtype_mask;
        rule->stagged_val = rule_field->stagged_val;
        rule->stagged_mask = rule_field->stagged_mask;
        rule->stag_vid_op = rule_field->stag_vid_op;
        rule->stag_vid_val = rule_field->stag_vid_val;
        rule->stag_vid_mask = rule_field->stag_vid_mask;
        rule->stag_pri_val = rule_field->stag_pri_val;
        rule->stag_pri_mask = rule_field->stag_pri_mask;
        rule->stag_dei_val = rule_field->stag_dei_val;
        rule->stag_dei_mask = rule_field->stag_dei_mask;
        rule->ctagged_val = rule_field->ctagged_val;
        rule->ctagged_mask = rule_field->ctagged_mask;
        rule->ctag_vid_op = rule_field->ctag_vid_op;
        rule->ctag_vid_val = rule_field->ctag_vid_val;
        rule->ctag_vid_mask = rule_field->ctag_vid_mask;
        rule->ctag_pri_val = rule_field->ctag_pri_val;
        rule->ctag_pri_mask = rule_field->ctag_pri_mask;
        rule->ctag_cfi_val = rule_field->ctag_cfi_val;
        rule->ctag_cfi_mask = rule_field->ctag_cfi_mask;
        rule->pppoe_sessionid = rule_field->pppoe_sessionid;
        rule->pppoe_sessionid_mask = rule_field->pppoe_sessionid_mask;

        /*ipv4 fields*/
        rule->src_ip4_val = rule_field->src_ip4_val;
        rule->src_ip4_mask = rule_field->src_ip4_mask;
        rule->dest_ip4_val = rule_field->dest_ip4_val;
        rule->dest_ip4_mask = rule_field->dest_ip4_mask;
        rule->is_ipv4_option_mask = rule_field->is_ipv4_option_mask;
        rule->is_ipv4_option_val = rule_field->is_ipv4_option_val;

        /*ipv6 fields*/
        aos_mem_copy(rule->src_ip6_val.ul,
            rule_field->src_ip6_val.ul, sizeof(fal_ip6_addr_t));
        aos_mem_copy(rule->src_ip6_mask.ul,
            rule_field->src_ip6_mask.ul, sizeof(fal_ip6_addr_t));
        aos_mem_copy(rule->dest_ip6_val.ul,
            rule_field->dest_ip6_val.ul, sizeof(fal_ip6_addr_t));
        aos_mem_copy(rule->dest_ip6_mask.ul,
            rule_field->dest_ip6_mask.ul, sizeof(fal_ip6_addr_t));
        rule->is_mobility_header_mask = rule_field->is_mobility_header_mask;
        rule->is_mobility_header_val = rule_field->is_mobility_header_val;
        rule->is_fragment_header_mask = rule_field->is_fragment_header_mask;
        rule->is_fragment_header_val = rule_field->is_fragment_header_val;
        rule->is_other_header_mask = rule_field->is_other_header_mask;
        rule->is_other_header_val = rule_field->is_other_header_val;

        /*ip fields*/
        rule->ip_proto_val = rule_field->ip_proto_val;
        rule->ip_proto_mask = rule_field->ip_proto_mask;
        rule->ip_dscp_val = rule_field->ip_dscp_val;
        rule->ip_dscp_mask = rule_field->ip_dscp_mask;
        rule->dest_l4port_op = rule_field->dest_l4port_op;
        rule->dest_l4port_val = rule_field->dest_l4port_val;
        rule->dest_l4port_mask = rule_field->dest_l4port_mask;
        rule->src_l4port_op = rule_field->src_l4port_op;
        rule->src_l4port_val = rule_field->src_l4port_val;
        rule->src_l4port_mask = rule_field->src_l4port_mask;
        rule->tcp_flag_val = rule_field->tcp_flag_val;
        rule->tcp_flag_mask = rule_field->tcp_flag_mask;
        rule->icmp_type_code_op = rule_field->icmp_type_code_op;
        rule->icmp_type_val = rule_field->icmp_type_val;
        rule->icmp_type_mask = rule_field->icmp_type_mask;
        rule->icmp_code_val = rule_field->icmp_code_val;
        rule->icmp_code_mask = rule_field->icmp_code_mask;
        rule->is_fragement_mask = rule_field->is_fragement_mask;
        rule->is_fragement_val = rule_field->is_fragement_val;
        rule->is_first_frag_mask = rule_field->is_first_frag_mask;
        rule->is_first_frag_val = rule_field->is_first_frag_val;
        rule->l3_ttl_mask = rule_field->l3_ttl_mask;
        rule->l3_ttl = rule_field->l3_ttl;
        rule->l3_length_op = rule_field->l3_length_op;
        rule->l3_length = rule_field->l3_length;
        rule->l3_length_mask = rule_field->l3_length_mask;
        rule->l3_pkt_type_mask = rule_field->l3_pkt_type_mask;
        rule->l3_pkt_type = rule_field->l3_pkt_type;
        rule->is_ah_header_mask = rule_field->is_ah_header_mask;
        rule->is_ah_header_val = rule_field->is_ah_header_val;
        rule->is_esp_header_mask = rule_field->is_esp_header_mask;
        rule->is_esp_header_val = rule_field->is_esp_header_val;

        /*udf fields*/
        rule->udf0_op = rule_field->udf0_op;
        rule->udf0_val = rule_field->udf0_val;
        rule->udf0_mask = rule_field->udf0_mask;
        rule->udf1_op = rule_field->udf1_op;
        rule->udf1_val = rule_field->udf1_val;
        rule->udf1_mask = rule_field->udf1_mask;
        rule->udf2_op = rule_field->udf2_op;
        rule->udf2_val = rule_field->udf2_val;
        rule->udf2_mask = rule_field->udf2_mask;
        rule->udf3_val = rule_field->udf3_val;
        rule->udf3_mask = rule_field->udf3_mask;
        rule->udfprofile_val = rule_field->udfprofile_val;
        rule->udfprofile_mask = rule_field->udfprofile_mask;
    }
    return;
}

static a_uint32_t _adpt_hppe_rule_type_count(ADPT_HPPE_ACL_RULE_MAP *rule_map,
			ADPT_HPPE_ACL_RULE_MAP *inner_rule_map)
{
	a_uint32_t rule_type_count = 0;
	a_int32_t i = 0;

	rule_type_count = _acl_bits_count(rule_map->rule_type_map,
			ADPT_ACL_HPPE_RULE_TYPE_NUM, 0);
	rule_type_count += _acl_bits_count(inner_rule_map->rule_type_map,
			ADPT_ACL_HPPE_RULE_TYPE_NUM, 0);
	for (i = 0; i < ADPT_ACL_HPPE_RULE_TYPE_NUM; i++) {
		SSDK_DEBUG("rule_type %d inverse_rule_type_count 0x%x-0x%x\n",
				i,
				rule_map->inverse_rule_type_count[i],
				inner_rule_map->inverse_rule_type_count[i]);
		rule_type_count += rule_map->inverse_rule_type_count[i];
		rule_type_count += inner_rule_map->inverse_rule_type_count[i];
	}

	SSDK_DEBUG("rule_type_map 0x%x-0x%x rule_type_count 0x%x\n",
			rule_map->rule_type_map,
			inner_rule_map->rule_type_map,
			rule_type_count);

	return rule_type_count;
}

sw_error_t
_adpt_hppe_acl_rule_type_map(a_uint32_t dev_id, a_uint32_t rule_id, a_uint32_t rule_nr,
		fal_acl_rule_t * rule, fal_acl_rule_t * inner_rule,
		ADPT_HPPE_ACL_RULE_MAP *rule_map,
		ADPT_HPPE_ACL_RULE_MAP *inner_rule_map)
{
#if defined(APPE)
	a_uint32_t tunnel_rule_type_map = 0, tunnel_inverse_rule_type_count = 0;
#endif
	SSDK_DEBUG("fields 0x%x:0x%x-0x%x:0x%x, inverse_fileds 0x%x:0x%x-0x%x:0x%x\n",
		   rule->field_flg[0], rule->field_flg[1],
		   inner_rule->field_flg[0], inner_rule->field_flg[1],
		   rule->inverse_field_flg[0], rule->inverse_field_flg[1],
		   inner_rule->inverse_field_flg[0], inner_rule->inverse_field_flg[1]);

	if(rule->rule_type == FAL_ACL_RULE_IP4 ||
		rule->rule_type == FAL_ACL_RULE_TUNNEL_IP4)
	{
		_adpt_hppe_acl_ipv4_fields_check(dev_id, rule_id, rule_nr, rule,
			rule_map);
	}
	else if(rule->rule_type == FAL_ACL_RULE_IP6 ||
		rule->rule_type == FAL_ACL_RULE_TUNNEL_IP6)
	{
		_adpt_hppe_acl_ipv6_fields_check(dev_id, rule_id, rule_nr, rule,
			rule_map);
	}
	_adpt_hppe_acl_l2_fields_check(dev_id, rule_id, rule_nr, rule, rule_map);

	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
#if defined(APPE)
		_adpt_appe_acl_udf_fields_check(dev_id, rule_id, rule_nr, rule, rule_map);

		if(rule->rule_type == FAL_ACL_RULE_TUNNEL_MAC ||
			rule->rule_type == FAL_ACL_RULE_TUNNEL_IP4 ||
			rule->rule_type == FAL_ACL_RULE_TUNNEL_IP6 ||
			rule->rule_type == FAL_ACL_RULE_TUNNEL_UDF)
		{
			if(rule->inner_rule_field.rule_type == FAL_ACL_RULE_IP4)
			{
				_adpt_hppe_acl_ipv4_fields_check(dev_id, rule_id, rule_nr,
					inner_rule, inner_rule_map);
			}
			if(rule->inner_rule_field.rule_type == FAL_ACL_RULE_IP6)
			{
				_adpt_hppe_acl_ipv6_fields_check(dev_id, rule_id, rule_nr,
					inner_rule, inner_rule_map);
			}
			_adpt_hppe_acl_l2_fields_check(dev_id, rule_id, rule_nr, inner_rule,
					inner_rule_map);
			_adpt_appe_acl_udf_fields_check(dev_id, rule_id, rule_nr, inner_rule,
					inner_rule_map);
			_adpt_appe_pre_acl_tunnel_info_fields_check(dev_id, rule_id, rule_nr,
					&rule->tunnel_info, &tunnel_rule_type_map,
					&tunnel_inverse_rule_type_count);
			if(inner_rule_map->rule_type_map == 0)
			{ /*inner ip_nonip/ip_ver*/
				if((FAL_FIELD_FLG_TST(inner_rule->field_flg, FAL_ACL_FIELD_IP)) ||
				(FAL_FIELD_FLG_TST(inner_rule->field_flg, FAL_ACL_FIELD_IPV6)))
				{
					inner_rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_MAC_DA_RULE);
				}
			}
		}
#endif
	}
	else
	{
		_adpt_hppe_acl_udf_fields_check(dev_id, rule_id, rule_nr, rule, rule_map);
	}

	if(rule_map->rule_type_map == 0)
	{/*outer ip_nonip/ip_ver*/
		if((FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP)) ||
			(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IPV6)))
		{
			rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_MAC_DA_RULE);
		}
	}

#if defined(APPE)
	if(tunnel_rule_type_map != 0)
		rule_map->rule_type_map |= tunnel_rule_type_map;
	if(tunnel_inverse_rule_type_count)
		rule_map->inverse_rule_type_count[ADPT_ACL_APPE_TUNNEL_RULE] +=
			tunnel_inverse_rule_type_count;
#endif

	/* select one rule type to match all if none is slected */
	if (_adpt_hppe_rule_type_count(rule_map, inner_rule_map) == 0)
		rule_map->rule_type_map |= (1<<ADPT_ACL_HPPE_MAC_DA_RULE);

	return SW_OK;
}

sw_error_t
adpt_hppe_acl_rule_add(a_uint32_t dev_id, a_uint32_t list_id,
		a_uint32_t rule_id, a_uint32_t rule_nr, fal_acl_rule_t * rule)
{
	a_uint32_t rule_type_count = 0;
	a_uint32_t index = 0, hw_list_index = 0, hw_list_id = 0;
	sw_error_t rv = SW_OK;
	struct list_head *rule_pos = NULL;
	ADPT_HPPE_ACL_SW_RULE *rule_exist_entry = NULL, *rule_add_entry = NULL;
	ADPT_HPPE_ACL_SW_LIST *list_find_entry = NULL;
	fal_acl_rule_t *inner_rule;
	ADPT_HPPE_ACL_RULE_MAP rule_map, inner_rule_map;
	aos_mem_zero(&rule_map, sizeof(ADPT_HPPE_ACL_RULE_MAP));
	aos_mem_zero(&inner_rule_map, sizeof(ADPT_HPPE_ACL_RULE_MAP));


	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(rule);

	if(list_id >= ADPT_ACL_SW_LIST_NUM)
	{
		return SW_OUT_OF_RANGE;
	}

	if(rule_id >= ADPT_ACL_RULE_NUM_PER_LIST)
	{
		return SW_OUT_OF_RANGE;
	}

	inner_rule = (fal_acl_rule_t *)kzalloc(sizeof(fal_acl_rule_t), GFP_ATOMIC);
	if (inner_rule == NULL)
		return SW_FAIL;

	/*convert inner_rule_field*/
	if(rule->rule_type == FAL_ACL_RULE_TUNNEL_MAC ||
		rule->rule_type == FAL_ACL_RULE_TUNNEL_IP4 ||
		rule->rule_type == FAL_ACL_RULE_TUNNEL_IP6 ||
		rule->rule_type == FAL_ACL_RULE_TUNNEL_UDF)
	{
		acl_rule_field_convert(inner_rule, &rule->inner_rule_field, A_FALSE);
	}
	aos_lock_bh(&hppe_acl_lock[dev_id]);
	list_find_entry = _adpt_hppe_acl_list_entry_get(dev_id, list_id);
	if(list_find_entry == NULL)
	{
		SSDK_ERROR("List %d not create, no resource to insert rules into it\n", list_id);
		aos_unlock_bh(&hppe_acl_lock[dev_id]);
		rv = SW_NO_RESOURCE;
		goto free_rule;
	}

	list_for_each(rule_pos, &list_find_entry->list_sw_rule)
	{
		rule_exist_entry = list_entry(rule_pos, ADPT_HPPE_ACL_SW_RULE, list);
		if((rule_exist_entry->rule_id == rule_id) && (rule_exist_entry->rule_hw_entry != 0))
		{
			aos_unlock_bh(&hppe_acl_lock[dev_id]);
			rv = SW_ALREADY_EXIST;
			goto free_rule;
		}
	}

	/*caculate rule map*/
	_adpt_hppe_acl_rule_type_map(dev_id, rule_id, rule_nr, rule, inner_rule,
					&rule_map, &inner_rule_map);

	/*caculate rule map counts */
	rule_type_count = _adpt_hppe_rule_type_count(&rule_map, &inner_rule_map);

	if(rule_type_count == 0 || rule_type_count > ADPT_ACL_ENTRY_NUM_PER_LIST)
	{
		SSDK_ERROR("rule_type_count = %d\n", rule_type_count);
		aos_unlock_bh(&hppe_acl_lock[dev_id]);
		rv = SW_NOT_SUPPORTED;
		goto free_rule;
	}

	/*allocate hw entries */
	rv = _adpt_hppe_acl_alloc_entries(dev_id, &hw_list_index, rule_id, rule_nr,
			rule, inner_rule, rule_type_count, &index);
	if(rv != SW_OK)
	{
		SSDK_ERROR("Alloc hw entries fail for rule %d\n", rule_id);
		aos_unlock_bh(&hppe_acl_lock[dev_id]);
		goto free_rule;
	}
	/* msg for debug */
	SSDK_DEBUG("ACL rule add before:list_id=%d, hw_list_index=%d, hw_list_id=%d, "
		"free_hw_entry_bitmap=0x%x, free_hw_entry_count=%d\n", list_id, hw_list_index,
		g_acl_hw_list[dev_id][hw_list_index].hw_list_id,
		g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_bitmap,
		g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_count);

	hw_list_id = g_acl_hw_list[dev_id][hw_list_index].hw_list_id;
	/* set hw acl rule and action reg*/
	rv = _adpt_ppe_acl_rule_hw_add(dev_id, list_find_entry->list_pri, hw_list_id,
		rule_id, rule_nr, rule, inner_rule, &rule_map, &inner_rule_map,
		s_acl_entries[index].entries);
	if(rv != SW_OK)
	{
		aos_unlock_bh(&hppe_acl_lock[dev_id]);
		goto free_rule;
	}

	/* set hw rule ext reg */
	if(!(s_acl_entries[index].num > 1 && FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_INVERSE_ALL))) {
		rv = _adpt_ppe_acl_rule_ext_set(dev_id, hw_list_id, s_acl_entries[index].ext_1,
			s_acl_entries[index].ext_2, s_acl_entries[index].ext_4);
		if(rv != SW_OK)
		{
			aos_unlock_bh(&hppe_acl_lock[dev_id]);
			goto free_rule;
		}
	}

	/*record sw info and insert the sw rule entry to the sw list entry*/
	rule_add_entry = (ADPT_HPPE_ACL_SW_RULE*)kzalloc(sizeof(ADPT_HPPE_ACL_SW_RULE), GFP_ATOMIC);
	if(rule_add_entry == NULL)
	{
		SSDK_ERROR("%s, %d:malloc fail for rule add entry\n", __FUNCTION__, __LINE__);
		aos_unlock_bh(&hppe_acl_lock[dev_id]);
		rv = SW_FAIL;
		goto free_rule;
	}
	rule_add_entry->rule_id = rule_id;
	rule_add_entry->rule_type[0] = rule->rule_type;
	rule_add_entry->rule_type[1] = rule->inner_rule_field.rule_type;
	rule_add_entry->rule_hw_entry = s_acl_entries[index].entries;
	rule_add_entry->rule_hw_list_id = hw_list_id;
	if(!(s_acl_entries[index].num > 1 && FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_INVERSE_ALL))) {
		rule_add_entry->ext1_val = s_acl_entries[index].ext_1;
		rule_add_entry->ext2_val = s_acl_entries[index].ext_2;
		rule_add_entry->ext4_val = s_acl_entries[index].ext_4;
	}
	list_add(&rule_add_entry->list, &list_find_entry->list_sw_rule);

	/*update hw list info */
	g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_bitmap &=
		(~(s_acl_entries[index].entries));
	g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_count -= s_acl_entries[index].num;

	/* msg for debug */
	SSDK_DEBUG("ACL rule add after:list_id=%d, hw_list_index=%d, hw_list_id=%d, "
		"free_hw_entry_bitmap=0x%x, free_hw_entry_count=%d\n", list_id, hw_list_index,
		hw_list_id, g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_bitmap,
		g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_count);
	/*resort hw list */
	 _adpt_hppe_acl_hw_list_resort(dev_id, hw_list_index, A_TRUE);

	 /*return rule hw info*/
	rule->hw_info.hw_rule_id = hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST + \
		_acl_bit_index(s_acl_entries[index].entries, ADPT_ACL_ENTRY_NUM_PER_LIST, 0);
	rule->hw_info.hw_list_id = hw_list_id;
	rule->hw_info.hw_entries = s_acl_entries[index].entries;

	aos_unlock_bh(&hppe_acl_lock[dev_id]);

free_rule:
	kfree(inner_rule);
	inner_rule = NULL;

	return rv;
}

static sw_error_t
_adpt_hppe_acl_rule_hw_delete(a_uint32_t dev_id,
		a_uint32_t hw_list_id, a_uint32_t hw_entries, a_uint32_t rule_nr)
{
	a_uint32_t hw_index = 0;
	sw_error_t rv = SW_OK;
	union ipo_rule_reg_u hw_reg = {0};
	union ipo_mask_reg_u hw_mask = {0};
	union ipo_action_u hw_act = {0};
	union ipo_cnt_tbl_u counters = {0};
	while(hw_entries != 0)
	{
		hw_index = _acl_bit_index(hw_entries, ADPT_ACL_ENTRY_NUM_PER_LIST, 0);
		if(hw_index >= ADPT_ACL_ENTRY_NUM_PER_LIST)
		{
			break;
		}

		rv |= hppe_ipo_rule_reg_set(dev_id,
			hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_reg);
		rv |= hppe_ipo_mask_reg_set(dev_id,
			hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_mask);
		rv |= hppe_ipo_action_set(dev_id,
			hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_act);
		SSDK_DEBUG("ACL destroy entry %d\n",
			hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index);
		hw_entries &= (~(1<<hw_index));

		/*clean counters*/
		hppe_ipo_cnt_tbl_set(dev_id,
			hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+hw_index, &counters);
	}
	return rv;
}

static sw_error_t
_adpt_ppe_acl_rule_hw_delete(a_uint32_t dev_id,
		a_uint32_t hw_list_id, a_uint32_t hw_entries, a_uint32_t rule_nr)
{
	sw_error_t rv = SW_OK;
	if (hw_list_id < ADPT_ACL_HW_LIST_NUM)
	{
		rv = _adpt_hppe_acl_rule_hw_delete(dev_id, hw_list_id,
					hw_entries, rule_nr);
		SW_RTN_ON_ERROR(rv);
	}
#if defined(APPE)
	else if (hw_list_id < ADPT_ACL_HW_LIST_NUM + ADPT_PRE_ACL_HW_LIST_NUM)
	{
		rv = _adpt_appe_pre_acl_rule_hw_delete(dev_id, hw_list_id - ADPT_ACL_HW_LIST_NUM,
					hw_entries, rule_nr);
		SW_RTN_ON_ERROR(rv);
	}
	rv = _adpt_appe_acl_ext_clear(dev_id, hw_list_id, hw_entries);
#endif
	return rv;
}

static sw_error_t
_adpt_hppe_acl_rule_delete(a_uint32_t dev_id, a_uint32_t list_id,
		ADPT_HPPE_ACL_SW_RULE *rule_entry, a_uint32_t rule_nr)
{
	sw_error_t rv = 0;
	a_uint32_t hw_entries = 0, hw_list_id = 0, hw_list_index = 0;
	a_uint32_t i = 0, end = ADPT_ACL_HW_LIST_NUM;

	hw_entries = rule_entry->rule_hw_entry;
	hw_list_id = rule_entry->rule_hw_list_id;
	/* msg for debug */
	SSDK_DEBUG("ACL delete rule entry before:list_id=%d, rule_id=%d, "
		"hw_entries=0x%x, hw_list_id=%d\n", list_id,
		rule_entry->rule_id, rule_entry->rule_hw_entry,
		rule_entry->rule_hw_list_id);

	/*clear hw rule reg and action reg*/
	rv = _adpt_ppe_acl_rule_hw_delete(dev_id, hw_list_id, hw_entries, rule_nr);

	/*clear hw rule ext reg */
	rv |= _adpt_ppe_acl_rule_ext_clear(dev_id, hw_list_id,
			rule_entry->ext1_val, rule_entry->ext2_val, rule_entry->ext4_val);

	/*find hw_list_index*/
#if defined(APPE)
	end += ADPT_PRE_ACL_HW_LIST_NUM;
#endif
	for(i = 0; i < end; i++)
	{
		if(g_acl_hw_list[dev_id][i].hw_list_id == hw_list_id)
		{
			hw_list_index = i;
			break;
		}
	}

	/*update hw list info and resort hw list*/
	g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_bitmap |= rule_entry->rule_hw_entry;
	g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_count +=
		_acl_bits_count(rule_entry->rule_hw_entry, ADPT_ACL_ENTRY_NUM_PER_LIST, 0);
	/* msg for debug */
	SSDK_DEBUG("ACL delete rule entry after:hw_list_index=%d, list_id=%d, "
		"rule_id=%d, hw_entries=0x%x, hw_list_id=%d\n", hw_list_index,
		list_id, rule_entry->rule_id, rule_entry->rule_hw_entry,
		rule_entry->rule_hw_list_id);
	_adpt_hppe_acl_hw_list_resort(dev_id, hw_list_index, A_FALSE);

	/*delete rule entry from the sw list*/
	list_del(&rule_entry->list);
	aos_mem_free(rule_entry);
	rule_entry = NULL;

	return rv;
}

sw_error_t
adpt_hppe_acl_rule_delete(a_uint32_t dev_id, a_uint32_t list_id,
		a_uint32_t rule_id, a_uint32_t rule_nr)
{
	struct list_head *rule_pos = NULL;
	ADPT_HPPE_ACL_SW_RULE *rule_delete_entry = NULL;
	ADPT_HPPE_ACL_SW_LIST *list_find_entry = NULL;

	ADPT_DEV_ID_CHECK(dev_id);

	if(list_id >= ADPT_ACL_SW_LIST_NUM)
	{
		return SW_OUT_OF_RANGE;
	}

	if(rule_id >= ADPT_ACL_RULE_NUM_PER_LIST)
	{
		return SW_OUT_OF_RANGE;
	}

	aos_lock_bh(&hppe_acl_lock[dev_id]);
	list_find_entry = _adpt_hppe_acl_list_entry_get(dev_id, list_id);
	if(list_find_entry != NULL)
	{
		list_for_each(rule_pos, &list_find_entry->list_sw_rule)
		{
			rule_delete_entry = list_entry(rule_pos, ADPT_HPPE_ACL_SW_RULE, list);
			if(rule_delete_entry->rule_id == rule_id)
			{
				_adpt_hppe_acl_rule_delete(dev_id, list_id,
							rule_delete_entry, rule_nr);
				break;
			}
		}
	}
	aos_unlock_bh(&hppe_acl_lock[dev_id]);
	return SW_OK;
}

static sw_error_t
_adpt_hppe_acl_rule_dump(a_uint32_t dev_id, a_uint32_t hw_list_id, a_uint32_t hw_entries)
{
	a_uint8_t i = 0;
	union ipo_rule_reg_u hw_reg = {0};
	union ipo_mask_reg_u hw_mask = {0};
	union ipo_action_u hw_act = {0};

	if(hw_entries != 0)
	{
		for(i = 0; i < ADPT_ACL_ENTRY_NUM_PER_LIST; i++)
		{
			if((1<<i) & hw_entries)
			{
				hppe_ipo_rule_reg_get(dev_id,
					hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+i, &hw_reg);
				hppe_ipo_mask_reg_get(dev_id,
					hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+i, &hw_mask);
				hppe_ipo_action_get(dev_id,
					hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+i, &hw_act);
				printk("hw_entry %d\n", hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+i);
				_adpt_acl_reg_dump((u_int8_t *)&hw_reg, sizeof(hw_reg));
				printk("hw_entry_mask %d\n",
					hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+i);
				_adpt_acl_reg_dump((u_int8_t *)&hw_mask, sizeof(hw_mask));
				printk("hw_action %d\n", hw_list_id*ADPT_ACL_ENTRY_NUM_PER_LIST+i);
				_adpt_acl_reg_dump((u_int8_t *)&hw_act, sizeof(hw_act));
			}
		}
	}

	return SW_OK;
}

static sw_error_t
_adpt_ppe_acl_rule_dump(a_uint32_t dev_id, a_uint32_t list_id, ADPT_HPPE_ACL_SW_RULE *rule_entry)
{
	a_uint32_t hw_entries = rule_entry->rule_hw_entry;
	a_uint32_t hw_list_id = rule_entry->rule_hw_list_id;
	printk("######list_id %d, rule_id %d, hw_list_id %d, hw_entries 0x%x, "
		   "ext1_val 0x%x, ext2_val 0x%x, ext4_val 0x%x\n",
		   list_id, rule_entry->rule_id, hw_list_id, hw_entries,
		   rule_entry->ext1_val, rule_entry->ext2_val, rule_entry->ext4_val);
	if (hw_list_id < ADPT_ACL_HW_LIST_NUM)
	{
		return _adpt_hppe_acl_rule_dump(dev_id, hw_list_id, hw_entries);
	}
#if defined(APPE)
	else if (hw_list_id < ADPT_ACL_HW_LIST_NUM + ADPT_PRE_ACL_HW_LIST_NUM)
	{
		return _adpt_appe_pre_acl_rule_dump(dev_id,
				hw_list_id - ADPT_ACL_HW_LIST_NUM , hw_entries);
	}
#endif
	return SW_OK;
}

sw_error_t
adpt_hppe_acl_rule_dump(a_uint32_t dev_id)
{
	a_uint8_t hw_list_index = 0, end = ADPT_ACL_HW_LIST_NUM;
	struct list_head *list_pos = NULL, *rule_pos = NULL;
	ADPT_HPPE_ACL_SW_LIST *list_dump_entry = NULL;
	ADPT_HPPE_ACL_SW_RULE *rule_dump_entry = NULL;

	ADPT_DEV_ID_CHECK(dev_id);
#if defined(APPE)
	end += ADPT_PRE_ACL_HW_LIST_NUM;
#endif
	/*dump the hw list status for debug*/
	for(hw_list_index = 0; hw_list_index < end; hw_list_index++)
	{
		SSDK_DEBUG("hw_list_index=%d, hw_list_valid=%d, hw_list_id=%d, "
			"free_hw_entry_bitmap=0x%x, free_hw_entry_count=%d\n", hw_list_index,
			g_acl_hw_list[dev_id][ hw_list_index].hw_list_valid,
			g_acl_hw_list[dev_id][ hw_list_index].hw_list_id,
			g_acl_hw_list[dev_id][ hw_list_index].free_hw_entry_bitmap,
			g_acl_hw_list[dev_id][ hw_list_index].free_hw_entry_count);
	}

	aos_lock_bh(&hppe_acl_lock[dev_id]);
	list_for_each(list_pos, &g_acl_sw_list[dev_id].list_sw_list)
	{
		list_dump_entry = list_entry(list_pos, ADPT_HPPE_ACL_SW_LIST, list);
		list_for_each(rule_pos, &list_dump_entry->list_sw_rule)
		{
			rule_dump_entry = list_entry(rule_pos, ADPT_HPPE_ACL_SW_RULE, list);
			_adpt_ppe_acl_rule_dump(dev_id, list_dump_entry->list_id, rule_dump_entry);
		}
	}
	aos_unlock_bh(&hppe_acl_lock[dev_id]);
	return SW_OK;
}

sw_error_t
adpt_hppe_acl_list_dump(a_uint32_t dev_id)
{
	adpt_hppe_acl_rule_dump(dev_id);

	return SW_OK;
}


sw_error_t
adpt_hppe_acl_list_creat(a_uint32_t dev_id, a_uint32_t list_id, a_uint32_t list_pri)
{
	ADPT_HPPE_ACL_SW_LIST *list_create_entry = NULL;

	ADPT_DEV_ID_CHECK(dev_id);

	if(list_id >= ADPT_ACL_SW_LIST_NUM)
	{
		return SW_OUT_OF_RANGE;
	}

	aos_lock_bh(&hppe_acl_lock[dev_id]);
	list_create_entry = _adpt_hppe_acl_list_entry_get(dev_id, list_id);
	if(list_create_entry != NULL)
	{
		aos_unlock_bh(&hppe_acl_lock[dev_id]);
		return SW_ALREADY_EXIST;
	}

	list_create_entry = (ADPT_HPPE_ACL_SW_LIST*)kzalloc(sizeof(ADPT_HPPE_ACL_SW_LIST), GFP_ATOMIC);
	if(list_create_entry == NULL)
	{
		SSDK_ERROR("%s, %d:malloc fail for list create entry\n", __FUNCTION__, __LINE__);
		aos_unlock_bh(&hppe_acl_lock[dev_id]);
		return SW_FAIL;
	}
	INIT_LIST_HEAD(&list_create_entry->list_sw_rule);
	list_create_entry->list_id = list_id;
	list_create_entry->list_pri = list_pri;
	list_add(&list_create_entry->list, &g_acl_sw_list[dev_id].list_sw_list);
	aos_unlock_bh(&hppe_acl_lock[dev_id]);
	return SW_OK;
}

sw_error_t
adpt_hppe_acl_list_destroy(a_uint32_t dev_id, a_uint32_t list_id)
{
	struct list_head *rule_pos=NULL, *rule_pos_temp = NULL;
	ADPT_HPPE_ACL_SW_RULE *rule_delete_entry = NULL;
	ADPT_HPPE_ACL_SW_LIST *list_destroy_entry = NULL;

	ADPT_DEV_ID_CHECK(dev_id);

	if(list_id >= ADPT_ACL_SW_LIST_NUM)
	{
		return SW_OUT_OF_RANGE;
	}

	aos_lock_bh(&hppe_acl_lock[dev_id]);
	list_destroy_entry = _adpt_hppe_acl_list_entry_get(dev_id, list_id);
	if(list_destroy_entry != NULL)
	{
		list_for_each_safe(rule_pos, rule_pos_temp, &list_destroy_entry->list_sw_rule)
		{
			rule_delete_entry = list_entry(rule_pos, ADPT_HPPE_ACL_SW_RULE, list);
			_adpt_hppe_acl_rule_delete(dev_id, list_id, rule_delete_entry, 1);
		}
		list_del(&list_destroy_entry->list);
		aos_mem_free(list_destroy_entry);
		list_destroy_entry = NULL;
	}
	aos_unlock_bh(&hppe_acl_lock[dev_id]);
	return SW_OK;
}

typedef sw_error_t (*hppe_acl_udp_set_func)(a_uint32_t dev_id, union udf_ctrl_reg_u *udf_ctrl);
typedef sw_error_t (*hppe_acl_udp_get_func)(a_uint32_t dev_id, union udf_ctrl_reg_u *udf_ctrl);

hppe_acl_udp_set_func g_udf_set_func[FAL_ACL_UDF_BUTT][4] = {
	{hppe_non_ip_udf0_ctrl_reg_set, hppe_non_ip_udf1_ctrl_reg_set,
			hppe_non_ip_udf2_ctrl_reg_set, hppe_non_ip_udf3_ctrl_reg_set},
	{hppe_ipv4_udf0_ctrl_reg_set, hppe_ipv4_udf1_ctrl_reg_set, hppe_ipv4_udf2_ctrl_reg_set,
			hppe_ipv4_udf3_ctrl_reg_set},
	{hppe_ipv6_udf0_ctrl_reg_set, hppe_ipv6_udf1_ctrl_reg_set, hppe_ipv6_udf2_ctrl_reg_set,
			hppe_ipv6_udf3_ctrl_reg_set},
};

hppe_acl_udp_get_func g_udf_get_func[FAL_ACL_UDF_BUTT][4] = {
	{hppe_non_ip_udf0_ctrl_reg_get, hppe_non_ip_udf1_ctrl_reg_get,
			hppe_non_ip_udf2_ctrl_reg_get, hppe_non_ip_udf3_ctrl_reg_get},
	{hppe_ipv4_udf0_ctrl_reg_get, hppe_ipv4_udf1_ctrl_reg_get, hppe_ipv4_udf2_ctrl_reg_get,
			hppe_ipv4_udf3_ctrl_reg_get},
	{hppe_ipv6_udf0_ctrl_reg_get, hppe_ipv6_udf1_ctrl_reg_get, hppe_ipv6_udf2_ctrl_reg_get,
			hppe_ipv6_udf3_ctrl_reg_get},
};

sw_error_t
adpt_hppe_acl_udf_profile_get(a_uint32_t dev_id, fal_acl_udf_pkt_type_t pkt_type,a_uint32_t udf_idx,
			fal_acl_udf_type_t *udf_type, a_uint32_t *offset)
{
	union udf_ctrl_reg_u udf_ctrl = {0};
	sw_error_t rv = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(udf_type);
	ADPT_NULL_POINT_CHECK(offset);

	rv = g_udf_get_func[pkt_type][udf_idx](dev_id, &udf_ctrl);

	if(rv != SW_OK)
		return rv;

	if(udf_ctrl.bf.udf_base == 0)
	{
		*udf_type = FAL_ACL_UDF_TYPE_L2;
	}
	else if(udf_ctrl.bf.udf_base == 1)
	{
		*udf_type = FAL_ACL_UDF_TYPE_L3;
	}
	else if(udf_ctrl.bf.udf_base == 2)
	{
		*udf_type = FAL_ACL_UDF_TYPE_L4;
	}

	*offset = udf_ctrl.bf.udf_offset*2;

	return SW_OK;
}


sw_error_t
adpt_hppe_acl_udf_profile_set(a_uint32_t dev_id, fal_acl_udf_pkt_type_t pkt_type,a_uint32_t udf_idx,
			fal_acl_udf_type_t udf_type, a_uint32_t offset)
{
	union udf_ctrl_reg_u udf_ctrl = {0};
	ADPT_DEV_ID_CHECK(dev_id);

	if(udf_type == FAL_ACL_UDF_TYPE_L2)
	{
		udf_ctrl.bf.udf_base = 0;
	}
	else if(udf_type == FAL_ACL_UDF_TYPE_L3)
	{
		udf_ctrl.bf.udf_base = 1;
	}
	else if(udf_type == FAL_ACL_UDF_TYPE_L4)
	{
		udf_ctrl.bf.udf_base = 2;
	}
	else
		return SW_NOT_SUPPORTED;

	if(offset % 2)/*only support even data*/
		return SW_BAD_VALUE;
	udf_ctrl.bf.udf_offset = offset/2;

	return g_udf_set_func[pkt_type][udf_idx](dev_id, &udf_ctrl);
}

sw_error_t
adpt_ppe_acl_udf_profile_set(a_uint32_t dev_id, fal_acl_udf_pkt_type_t pkt_type, a_uint32_t udf_idx,
			fal_acl_udf_type_t udf_type, a_uint32_t offset)
{
	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
#if defined(APPE)
		return adpt_appe_acl_udf_profile_set(dev_id, pkt_type, udf_idx, udf_type, offset);
#endif
	}
	else
	{
		return adpt_hppe_acl_udf_profile_set(dev_id, pkt_type, udf_idx, udf_type, offset);
	}
	return SW_NOT_SUPPORTED;
}

sw_error_t
adpt_ppe_acl_udf_profile_get(a_uint32_t dev_id, fal_acl_udf_pkt_type_t pkt_type, a_uint32_t udf_idx,
			fal_acl_udf_type_t * udf_type, a_uint32_t * offset)
{
	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
#if defined(APPE)
		return adpt_appe_acl_udf_profile_get(dev_id, pkt_type, udf_idx, udf_type, offset);
#endif
	}
	else
	{
		return adpt_hppe_acl_udf_profile_get(dev_id, pkt_type, udf_idx, udf_type, offset);
	}
	return SW_NOT_SUPPORTED;
}

sw_error_t
adpt_hppe_acl_counter_get(a_uint32_t dev_id,
			a_uint32_t entry_index, fal_entry_counter_t *acl_counter)
{
	sw_error_t rv = SW_OK;
	union ipo_cnt_tbl_u ipo_cnt = {0};

	if (entry_index < ADPT_ACL_HW_LIST_NUM*ADPT_ACL_ENTRY_NUM_PER_LIST)
	{
		rv = hppe_ipo_cnt_tbl_get(dev_id, entry_index, &ipo_cnt);
		SW_RTN_ON_ERROR(rv);
		acl_counter->matched_pkts = ipo_cnt.bf.hit_pkt_cnt;
		acl_counter->matched_bytes = ipo_cnt.bf.hit_byte_cnt_0 |
			(a_uint64_t)ipo_cnt.bf.hit_byte_cnt_1 << SW_FIELD_OFFSET_IN_WORD(
				IPO_CNT_TBL_HIT_BYTE_CNT_OFFSET);
	}
#if defined(APPE)
	else if (entry_index < (ADPT_ACL_HW_LIST_NUM*ADPT_ACL_ENTRY_NUM_PER_LIST + \
			ADPT_PRE_ACL_HW_LIST_NUM*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST))
	{
		rv = _adpt_appe_pre_acl_counter_get(dev_id,
			entry_index - ADPT_ACL_HW_LIST_NUM*ADPT_ACL_ENTRY_NUM_PER_LIST, acl_counter);
	}
#endif
	else
	{
		return SW_OUT_OF_RANGE;
	}
	return rv;
}

static sw_error_t
_adpt_hppe_acl_hw_list_init(a_uint32_t dev_id, a_uint32_t hw_list_start, a_uint32_t hw_list_end)

{
	a_uint32_t hw_list_index;

	for(hw_list_index = hw_list_start; hw_list_index < hw_list_end; hw_list_index++)
	{
		if(g_acl_hw_list[dev_id][hw_list_index].hw_list_valid == A_FALSE)
		{
			g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_bitmap = 0xff;
			g_acl_hw_list[dev_id][hw_list_index].free_hw_entry_count =
				ADPT_ACL_ENTRY_NUM_PER_LIST;
			g_acl_hw_list[dev_id][hw_list_index].hw_list_id =
				hw_list_start + hw_list_end - 1 - hw_list_index;
			g_acl_hw_list [dev_id][hw_list_index].hw_list_valid = A_TRUE;
			INIT_LIST_HEAD(&g_acl_sw_list[dev_id].list_sw_list);
		}
	}

	return SW_OK;
}
sw_error_t adpt_hppe_acl_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;
	p_adpt_api = adpt_api_ptr_get(dev_id);

	ADPT_NULL_POINT_CHECK(p_adpt_api);

	_adpt_hppe_acl_hw_list_init(dev_id, 0, ADPT_ACL_HW_LIST_NUM);
#if defined(APPE)
	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
		_adpt_hppe_acl_hw_list_init(dev_id, ADPT_ACL_HW_LIST_NUM,
			ADPT_ACL_HW_LIST_NUM + ADPT_PRE_ACL_HW_LIST_NUM);
	}
#endif
	p_adpt_api->adpt_acl_list_bind = adpt_hppe_acl_list_bind;
	p_adpt_api->adpt_acl_list_dump = adpt_hppe_acl_list_dump;
	p_adpt_api->adpt_acl_rule_query = adpt_hppe_acl_rule_query;
	p_adpt_api->adpt_acl_list_unbind = adpt_hppe_acl_list_unbind;
	p_adpt_api->adpt_acl_rule_add = adpt_hppe_acl_rule_add;
	p_adpt_api->adpt_acl_rule_delete = adpt_hppe_acl_rule_delete;
	p_adpt_api->adpt_acl_rule_dump = adpt_hppe_acl_rule_dump;
	p_adpt_api->adpt_acl_list_creat = adpt_hppe_acl_list_creat;
	p_adpt_api->adpt_acl_list_destroy = adpt_hppe_acl_list_destroy;
	p_adpt_api->adpt_acl_udf_profile_set = adpt_ppe_acl_udf_profile_set;
	p_adpt_api->adpt_acl_udf_profile_get = adpt_ppe_acl_udf_profile_get;
#if defined(APPE)
	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
		p_adpt_api->adpt_acl_udf_profile_entry_add =
				adpt_appe_acl_udf_profile_entry_add;
		p_adpt_api->adpt_acl_udf_profile_entry_del =
				adpt_appe_acl_udf_profile_entry_del;
		p_adpt_api->adpt_acl_udf_profile_entry_getfirst =
				adpt_appe_acl_udf_profile_entry_getfirst;
		p_adpt_api->adpt_acl_udf_profile_entry_getnext =
				adpt_appe_acl_udf_profile_entry_getnext;
		p_adpt_api->adpt_acl_udf_profile_cfg_set =
				adpt_appe_acl_udf_profile_cfg_set;
		p_adpt_api->adpt_acl_udf_profile_cfg_get =
				adpt_appe_acl_udf_profile_cfg_get;
		p_adpt_api->adpt_acl_vpgroup_set =
				adpt_appe_acl_vpgroup_set;
		p_adpt_api->adpt_acl_vpgroup_get =
				adpt_appe_acl_vpgroup_get;
	}
#endif
	p_adpt_api->adpt_acl_counter_get = adpt_hppe_acl_counter_get;

	aos_lock_init(&hppe_acl_lock[dev_id]);

	return SW_OK;
}

/**
 * @}
 */
