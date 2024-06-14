/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
* @defgroup fal_gen _FAL_TUNNEL_H_
* @{
*/
#ifndef _FAL_TUNNEL_H_
#define _FAL_TUNNEL_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "sw.h"
#include "fal_type.h"

#define FAL_TUNNEL_DECAP_ENTRY_MAX     128
#define FAL_TUNNEL_ENCAP_ENTRY_MAX     128
enum {
	FUNC_TUNNEL_INTF_SET = 0,
	FUNC_TUNNEL_INTF_GET,
	FUNC_TUNNEL_ENCAP_RULE_ENTRY_SET,
	FUNC_TUNNEL_ENCAP_RULE_ENTRY_GET,
	FUNC_TUNNEL_ENCAP_RULE_ENTRY_DEL,
	FUNC_TUNNEL_ENCAP_INTF_TUNNELID_SET,
	FUNC_TUNNEL_ENCAP_INTF_TUNNELID_GET,
	FUNC_TUNNEL_VLAN_INTF_ADD,
	FUNC_TUNNEL_VLAN_INTF_GETFIRST,
	FUNC_TUNNEL_VLAN_INTF_GETNEXT,
	FUNC_TUNNEL_VLAN_INTF_DEL,
	FUNC_TUNNEL_ENCAP_PORT_TUNNELID_SET,
	FUNC_TUNNEL_ENCAP_PORT_TUNNELID_GET,
	FUNC_TUNNEL_DECAP_ENTRY_ADD,
	FUNC_TUNNEL_DECAP_ENTRY_GET,
	FUNC_TUNNEL_DECAP_ENTRY_GETNEXT,
	FUNC_TUNNEL_DECAP_ENTRY_DEL,
	FUNC_TUNNEL_DECAP_ENTRY_FLUSH,
	FUNC_TUNNEL_ENCAP_ENTRY_ADD,
	FUNC_TUNNEL_ENCAP_ENTRY_GET,
	FUNC_TUNNEL_ENCAP_ENTRY_GETNEXT,
	FUNC_TUNNEL_ENCAP_ENTRY_DEL,
	FUNC_TUNNEL_GLOBAL_CFG_SET,
	FUNC_TUNNEL_GLOBAL_CFG_GET,
	FUNC_TUNNEL_ENCAP_HEADER_CTRL_SET,
	FUNC_TUNNEL_ENCAP_HEADER_CTRL_GET,
	FUNC_TUNNEL_PORT_INTF_SET,
	FUNC_TUNNEL_PORT_INTF_GET,
	FUNC_TUNNEL_DECAP_ECN_SET,
	FUNC_TUNNEL_DECAP_ECN_GET,
	FUNC_TUNNEL_ENCAP_ECN_SET,
	FUNC_TUNNEL_ENCAP_ECN_GET,
	FUNC_TUNNEL_UDF_PROFILE_ENTRY_ADD,
	FUNC_TUNNEL_UDF_PROFILE_ENTRY_DEL,
	FUNC_TUNNEL_UDF_PROFILE_ENTRY_GETFIRST,
	FUNC_TUNNEL_UDF_PROFILE_ENTRY_GETNEXT,
	FUNC_TUNNEL_UDF_PROFILE_CFG_SET,
	FUNC_TUNNEL_UDF_PROFILE_CFG_GET,
	FUNC_TUNNEL_EXP_DECAP_SET,
	FUNC_TUNNEL_EXP_DECAP_GET,
	FUNC_TUNNEL_DECAP_KEY_SET,
	FUNC_TUNNEL_DECAP_KEY_GET,
	FUNC_TUNNEL_DECAP_EN_SET,
	FUNC_TUNNEL_DECAP_EN_GET,
	FUNC_TUNNEL_DECAP_ACTION_UPDATE,
	FUNC_TUNNEL_DECAP_COUNTER_GET,
};

/* tunnel type */
typedef enum {
	FAL_TUNNEL_TYPE_GRE_TAP_OVER_IPV4 = 0,
	FAL_TUNNEL_TYPE_GRE_TAP_OVER_IPV6,
	FAL_TUNNEL_TYPE_VXLAN_OVER_IPV4,
	FAL_TUNNEL_TYPE_VXLAN_OVER_IPV6,
	FAL_TUNNEL_TYPE_VXLAN_GPE_OVER_IPV4,
	FAL_TUNNEL_TYPE_VXLAN_GPE_OVER_IPV6,
	FAL_TUNNEL_TYPE_IPV4_OVER_IPV6 = 7,
	FAL_TUNNEL_TYPE_PROGRAM0,
	FAL_TUNNEL_TYPE_PROGRAM1,
	FAL_TUNNEL_TYPE_PROGRAM2,
	FAL_TUNNEL_TYPE_PROGRAM3,
	FAL_TUNNEL_TYPE_PROGRAM4,
	FAL_TUNNEL_TYPE_PROGRAM5,
	FAL_TUNNEL_TYPE_GENEVE_OVER_IPV4,
	FAL_TUNNEL_TYPE_GENEVE_OVER_IPV6,
	FAL_TUNNEL_TYPE_INVALID_TUNNEL,
} fal_tunnel_type_t;

/* decapsulation */
typedef enum {
	FAL_TUNNEL_OP_MODE_HASH = 0,
	FAL_TUNNEL_OP_MODE_INDEX = 1,
	FAL_TUNNEL_OP_MODE_MAX,
} fal_tunnel_op_mode_t;

typedef enum {
	FAL_TUNNEL_OP_RSLT_OK = 0,
	FAL_TUNNEL_OP_RSLT_FAIL = 1,
} fal_tunnel_op_rslt_t;

typedef enum {
	FAL_TUNNEL_OP_TYPE_ADD = 0,
	FAL_TUNNEL_OP_TYPE_DEL = 1,
	FAL_TUNNEL_OP_TYPE_GET = 2,
	FAL_TUNNEL_OP_TYPE_FLUSH = 3,
} fal_tunnel_op_type_t;

/* tunnel entry lookup key bmp */
enum {
	FAL_TUNNEL_KEY_SIP_EN = 0,
	FAL_TUNNEL_KEY_DIP_EN,
	FAL_TUNNEL_KEY_L4PROTO_EN,
	FAL_TUNNEL_KEY_SPORT_EN,
	FAL_TUNNEL_KEY_DPORT_EN,
	FAL_TUNNEL_KEY_TLINFO_EN,
	FAL_TUNNEL_KEY_UDF0_EN,
	FAL_TUNNEL_KEY_UDF1_EN,
	FAL_TUNNEL_KEY_MAX,
};

/* tunnel overlay type */
typedef enum
{
	FAL_TUNNEL_OVERLAY_TYPE_GRE_TAP = 0, /*gre tap*/
	FAL_TUNNEL_OVERLAY_TYPE_VXLAN,       /*vxlan*/
	FAL_TUNNEL_OVERLAY_TYPE_VXLAN_GPE,   /*vxlan-gpe*/
	FAL_TUNNEL_OVERLAY_TYPE_GENEVE,      /*geneve*/
	FAL_TUNNEL_OVERLAY_TYPE_BUTT,
} fal_tunnel_overlay_type_t;

/* tunnel program type */
typedef enum
{
	FAL_TUNNEL_PROGRAM_TYPE_0 = 0,     /*program0 type*/
	FAL_TUNNEL_PROGRAM_TYPE_1,         /*program1 type*/
	FAL_TUNNEL_PROGRAM_TYPE_2,         /*program2 type*/
	FAL_TUNNEL_PROGRAM_TYPE_3,         /*program3 type*/
	FAL_TUNNEL_PROGRAM_TYPE_4,         /*program4 type*/
	FAL_TUNNEL_PROGRAM_TYPE_5,         /*program5 type*/
	FAL_TUNNEL_PROGRAM_TYPE_BUTT,
} fal_tunnel_program_type_t;

/* tunnel udf type */
typedef enum
{
	FAL_TUNNEL_UDF_TYPE_L2 = 0, /*start from L2 */
	FAL_TUNNEL_UDF_TYPE_L3,     /*start from L3 */
	FAL_TUNNEL_UDF_TYPE_L4,    /*start from L4 */
	FAL_TUNNEL_UDF_TYPE_OVERLAY, /*start from overlay hdr*/
	FAL_TUNNEL_UDF_TYPE_PROGRAM, /*start from program hdr */
	FAL_TUNNEL_UDF_TYPE_PAYLOAD,  /*start from payload */
	FAL_TUNNEL_UDF_TYPE_BUTT,
} fal_tunnel_udf_type_t;

#define FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE 0
#define FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE 1
#define FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_OVERLAY_TYPE 2
#define FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_PROGRAM_TYPE 3

typedef a_uint32_t fal_tunnel_udf_profile_entry_field_map_t;

#define FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_SET(flag, field) \
    (flag) |= (0x1UL << (field))

#define FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_CLR(flag, field) \
    (flag) &= (~(0x1UL << (field)))

#define FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_TST(flag, field) \
((flag) & (0x1UL << (field))) ? 1 : 0

typedef struct {
		fal_tunnel_udf_profile_entry_field_map_t field_flag; /*Indicate
							*which fields are included*/
		fal_l3_type_t l3_type;
		fal_l4_type_t l4_type;
		fal_tunnel_overlay_type_t overlay_type;
		fal_tunnel_program_type_t program_type;
} fal_tunnel_udf_profile_entry_t;

/* tunnel udp entry */
typedef struct {
	a_uint8_t ip_ver;        /* 1 ipv4, 2 ipv6, 3 ipv4 or ipv6 */
	a_uint8_t udp_type;      /* 1 udp, 2 udp-lite, 3 udp or udp-lite*/
	a_uint8_t l4_port_type;  /* 1 dst port, 2 src port, 3 dst or src port*/
	a_uint16_t l4_port;      /* l4 port value*/
} fal_tunnel_udp_entry_t;

typedef struct {
	a_uint16_t key_bmp; /*tunnel key included bit map*/
	a_uint32_t tunnel_info_mask; /*mask value for tunnel_info */
	a_uint8_t udf0_idx; /*UDF0 id used to select one UDF from total four UDFs */
	a_uint16_t udf0_mask; /*UDF0 mask for udf0 */
	a_uint8_t udf1_idx; /*UDF1 id used to select one UDF from total four UDFs */
	a_uint16_t udf1_mask; /*UDF1 mask for udf1 */
} fal_tunnel_decap_key_t;

typedef struct {
	a_uint32_t entry_id; /*entry index*/
	a_uint8_t ip_ver; /*0 for ipv4 or 1 for ipv6*/
	fal_tunnel_type_t tunnel_type;/*tunnel type*/
	a_uint16_t key_bmp; /*tunnel valid bitmap for udf0, udf1 and tunnel_info */
	union {
		fal_ip4_addr_t ip4_addr;
		fal_ip6_addr_t ip6_addr;
	} sip; /*matched src ip*/
	union {
		fal_ip4_addr_t ip4_addr;
		fal_ip6_addr_t ip6_addr;
	} dip; /*matched dst ip*/
	a_uint8_t l4_proto; /*matched ip proto */
	a_uint16_t sport; /*matched l4 src port*/
	a_uint16_t dport; /*matched l4 dst port*/
	a_uint32_t tunnel_info; /*matched 32 bit vni and reserved or key filed of GRE */
	a_uint16_t udf0; /*UDF0 value */
	a_uint16_t udf1; /*UDF1 value */
} fal_tunnel_rule_t;

typedef enum {
	FAL_TUNNEL_PIPE_MODE = 0,
	FAL_TUNNEL_UNIFORM_MODE = 1,
	FAL_TUNNEL_MODE_MAX,
} fal_tunnel_mode_t;

#define FAL_TUNNEL_SVLAN_CHECK_EN		(0x1UL << 0)
#define FAL_TUNNEL_CVLAN_CHECK_EN		(0x1UL << 1)
#define FAL_TUNNEL_L3IF_CHECK_EN		(0x1UL << 2)
typedef struct {
	a_uint16_t verify_bmp; /* verify bit map */
	a_uint8_t tl_l3_if; /*Tunnel interface for source interface check */
	a_uint8_t svlan_fmt; /*0 = Untagged; 1 = Tagged or priority tag; Used for SVLAN check */
	a_uint16_t svlan_id; /*SVLAN ID used for SVLAN check */
	a_uint8_t cvlan_fmt; /*0 = Untagged; 1 = Tagged or priority tag; Used for CVLAN check */
	a_uint16_t cvlan_id; /*CVLAN ID used for CVLAN check */
} fal_tunnel_verify_entry_t;

enum {
	FAL_TUNNEL_SVLAN_UPDATE,
	FAL_TUNNEL_CVLAN_UPDATE,
	FAL_TUNNEL_L3IF_UPDATE,
	FAL_TUNNEL_DECAP_UPDATE,
	FAL_TUNNEL_DEACCE_UPDATE,
	FAL_TUNNEL_SRCINFO_UPDATE,
	FAL_TUNNEL_PKT_MODE_UPDATE,
	FAL_TUNNEL_SERVICE_CODE_UPDATE,
	FAL_TUNNEL_UDP_CSUM_ZERO_UPDATE,
	FAL_TUNNEL_EXP_PROFILE_UPDATE,
	FAL_TUNNEL_FWD_CMD_UPDATE,
};

typedef struct {
	a_uint32_t update_bmp; /* the bitmap for updating the field of this structure */
	fal_fwd_cmd_t fwd_cmd; /*forward type*/
	fal_tunnel_verify_entry_t verify_entry; /* l3_if, vlan verification */
	a_bool_t deacce_en; /*0 for disable and 1 for enable*/
	a_bool_t decap_en; /*decapsulation or not */
	a_bool_t udp_csum_zero; /*allow udp checksum to be zero */
	a_bool_t service_code_en; /*enable new service code or not */
	a_uint8_t service_code; /*updated service code */
	fal_tunnel_mode_t spcp_mode; /*uniform mode or pipe mode */
	fal_tunnel_mode_t sdei_mode; /*uniform mode or pipe mode */
	fal_tunnel_mode_t cpcp_mode; /*uniform mode or pipe mode */
	fal_tunnel_mode_t cdei_mode; /*uniform mode or pipe mode */
	fal_tunnel_mode_t ttl_mode; /*uniform mode or pipe mode */
	fal_tunnel_mode_t dscp_mode; /*uniform mode or pipe mode */
	a_uint8_t ecn_mode; /* 0 :RFC3168, 1: RFC4301; 2: RFC6040 */
	a_bool_t src_info_enable; /*enable new source info or not */
	a_uint8_t src_info_type; /*0 = Virtual port; 1 = L3_IF for tunnel payload */
	a_uint16_t src_info; /*Virtual port ID or L3_IF index as source info */
	a_uint8_t exp_profile; /*Exception profile ID */
	a_uint32_t pkt_counter; /* hit packet counter */
	a_uint64_t byte_counter; /* hit byte counter */
} fal_tunnel_action_t;

typedef struct {
	fal_tunnel_rule_t decap_rule; /* decapsulation entry rule */
	fal_tunnel_action_t decap_action; /* decapsulation entry rule */
} fal_tunnel_decap_entry_t;

#define FAL_ECN_VAL_LENGTH_MASK	3 /* 2 bits for ecn value */
#define FAL_ECN_EXP_LENGTH_MASK	1 /* 1 bits for ecn exception */
typedef enum {
	FAL_ENCAP_ECN_NO_UPDATE = 0, /* no update ecn */
	FAL_ENCAP_ECN_RFC3168_LIMIT_RFC6040_CMPAT_MODE = 1, /* RFC3168 limitation mode
							     * RFC6040 compatibility mode
							     * for encapsulation */
	FAL_ENCAP_ECN_RFC3168_FULL_MODE = 2, /* RFC3168 full mode for encapsulation */
	FAL_ENCAP_ECN_RFC4301_RFC6040_NORMAL_MODE = 3 /* RFC4301 mode and RFC6040 normal mode
							* for encapsulation */
} fal_tunnel_encap_ecn_mode_t;

typedef enum {
	FAL_TUNNEL_DECAP_ECN_RFC3168_MODE = 0, /* RFC3168 mode for decapsulation */
	FAL_TUNNEL_DECAP_ECN_RFC4301_MODE = 1, /* RFC4301 mode for decapsulation */
	FAL_TUNNEL_DECAP_ECN_RFC6040_MODE = 2 /* RFC6040 mode for decapsulation */
} fal_tunnel_decap_ecn_mode_t;

typedef enum {
	FAL_TUNNEL_ECN_NOT_ECT, /* Not ECN-capable transport */
	FAL_TUNNEL_ECN_ECT_0, /* ECN-capable transport */
	FAL_TUNNEL_ECN_ECT_1, /* ECN-capable transport */
	FAL_TUNNEL_ECN_CE, /* Congestion experienced */
	FAL_TUNNEL_ECN_INVALID /* Congestion experienced */
} fal_tunnel_ecn_val_t;

typedef struct {
	fal_tunnel_decap_ecn_mode_t ecn_mode; /* ecn mode including RFC 3168, RFC 4301, RFC 6040 */
	fal_tunnel_ecn_val_t outer_ecn; /* as described by fal_tunnel_ecn_val_t */
	fal_tunnel_ecn_val_t inner_ecn; /* as described by fal_tunnel_ecn_val_t */
} fal_tunnel_decap_ecn_rule_t;

typedef struct {
	fal_tunnel_ecn_val_t ecn_value; /* as described by fal_tunnel_ecn_val_t */
	a_bool_t ecn_exp; /* ecn exception or not */
} fal_tunnel_decap_ecn_action_t;

typedef struct {
	fal_tunnel_encap_ecn_mode_t ecn_mode; /* as described by fal_encap_ecn_mode_t */
	fal_tunnel_ecn_val_t inner_ecn; /* ecn value of incoming packet */
} fal_tunnel_encap_ecn_t;

typedef struct {
	fal_fwd_cmd_t deacce_action; /* DE_ACCE in the matched TT table is
				      * set to enable,
				      * 0x0 = Forward
				      * 0x1 = Drop
				      * 0x2 = COPY
				      * 0x3 = RDT_TO_CPU
				      */
	fal_fwd_cmd_t src_if_check_action; /* TT based source TL_IF check fail,
					    * 0x0 = Forward
					    * 0x1 = Drop
					    * 0x2 = COPY
					    * 0x3 = RDT_TO_CPU
					    */
	a_bool_t src_if_check_deacce_en; /* TT based source TL_IF check fail,
					  * de-acceleration enable or not
					  */
	fal_fwd_cmd_t vlan_check_action; /* TT based VLAN check fail,
					  * 0x0 = Forward
					  * 0x1 = Drop
					  * 0x2 = COPY
					  * 0x3 = RDT_TO_CPU
					  */
	a_bool_t vlan_check_deacce_en; /* TT based VLAN check fail,
					* de-acceleration enable or not
					*/
	fal_fwd_cmd_t udp_csum_zero_action; /* TT based udp checksum zero check fail,
					  * 0x0 = Forward
					  * 0x1 = Drop
					  * 0x2 = COPY
					  * 0x3 = RDT_TO_CPU
					  */
	a_bool_t udp_csum_zero_deacce_en; /* TT based udp checksum zero check fail,
					* de-acceleration enable or not
					*/
	fal_fwd_cmd_t pppoe_multicast_action; /* PPPOE packet with multicast IP
					       * address terminated,
					       * 0x0 = Forward
					       * 0x1 = Drop
					       * 0x2 = COPY
					       * 0x3 = RDT_TO_CPU
					       */

	a_bool_t pppoe_multicast_deacce_en; /* PPPOE packet with multicast IP
					     * address terminated,
					     * de-acceleration enable or not
					     */
	a_uint8_t hash_mode[2]; /* hash_mode: 0 crc10, 1 xor, 2 crc16*/
} fal_tunnel_global_cfg_t;

typedef struct {
	a_bool_t l3_if_valid; /*0 for disable and 1 for enable*/
	a_uint32_t l3_if_index; /*index for interface table*/
} fal_tunnel_intf_id_t;

typedef struct {
	fal_tunnel_intf_id_t l3_if; /* tunnel l3 interface */
	fal_mac_addr_t mac_addr; /* compare dst mac address of packet */
	a_bool_t pppoe_en; /* enable pppoe based on port */
	a_uint32_t vlan_group_id; /* vlan group id matched with tunnel vlan intf */
	a_uint32_t pppoe_group_id; /* pppoe group id matched with tunnel pppoe session */
} fal_tunnel_port_intf_t;

typedef struct {
	fal_pbmp_t port_id; /* port ID including port bitmap, port/vp, vp group */
	a_uint16_t key_bmp; /* key bit map */
	a_uint8_t svlan_fmt; /* 1: Untagged, 2: Untagged, 4: Untagged */
	a_uint16_t svlan_id; /* svlan id */
	a_uint8_t cvlan_fmt; /* 1: Untagged, 2: Untagged, 4: Untagged */
	a_uint16_t cvlan_id; /* cvlan id */
	fal_tunnel_intf_id_t l3_if; /* tunnel l3 interface */
	a_bool_t pppoe_en; /* enable pppoe based on vlan packet */
} fal_tunnel_vlan_intf_t;

typedef struct {
	a_bool_t ipv4_decap_en; /* ipv4 decapsulation or not */
	a_bool_t ipv6_decap_en; /* ipv6 decapsulation or not */
	a_bool_t dmac_check_en; /* dst mac check or not*/
	a_bool_t ttl_exceed_deacce_en; /* ttl is 0,
					  * de-acceleration enable or not
					  */
	fal_fwd_cmd_t ttl_exceed_action; /* ttl is 0,
					  * 0x0 = Forward
					  * 0x1 = Drop
					  * 0x2 = COPY
					  * 0x3 = RDT_TO_CPU
					  */
	a_bool_t lpm_en; /* do lpm lookup or not */
	a_uint32_t mini_ipv6_mtu; /* Minimum IPv6 MTU to determine the DF bit of IPv4 */
} fal_tunnel_intf_t;

typedef struct {
	a_bool_t tunnel_id_valid; /*0 for disable and 1 for enable*/
	a_uint32_t tunnel_id; /*tunnel id for encapusaltion lookup */
} fal_tunnel_id_t;

typedef enum {
	FAL_TUNNEL_ENCAP_TARGET_NO_UPDATE = 0,
	FAL_TUNNEL_ENCAP_TARGET_DIP,
	FAL_TUNNEL_ENCAP_TARGET_SIP,
	FAL_TUNNEL_ENCAP_TARGET_TUNNEL_INFO,
	FAL_TUNNEL_ENCAP_TARGET_MAX,
} fal_tunnel_encap_target_t;

typedef enum {
	FAL_TUNNEL_INNER_ETHERNET = 0,
	FAL_TUNNEL_INNER_IP,
	FAL_TUNNEL_INNER_TRANSPORT,
	FAL_TUNNEL_INNER_RESERVED,
} fal_tunnel_encap_payload_type_t;

#define FAL_TUNNEL_ENCAP_HEADER_MAX_LEN 128
typedef struct {
	fal_mac_addr_t smac_addr; /* source mac address */
	fal_mac_addr_t dmac_addr; /* destination mac address */
	a_uint16_t ether_type; /* ethernet type */
	union {
		fal_ip4_addr_t ip4_addr;
		fal_ip6_addr_t ip6_addr;
	} sip; /* src ip to update */
	union {
		fal_ip4_addr_t ip4_addr;
		fal_ip6_addr_t ip6_addr;
	} dip; /* dst ip to update */
	a_uint32_t tunnel_info; /* tunnel info */
} fal_pkt_header_t;

typedef union {
	a_uint8_t pkt_header_data[FAL_TUNNEL_ENCAP_HEADER_MAX_LEN];
	fal_pkt_header_t pkt_header_info;
} fal_tunnel_encap_header_t;

typedef struct {
	a_uint8_t encap_type; /* 0: encapsulation, 1:translation */
	a_uint8_t edit_rule_id; /* edit rule id */
	fal_tunnel_encap_target_t encap_target; /*0:No update,1:DIP,2:SIP,3:TUNNEL INFO */
	fal_tunnel_encap_payload_type_t payload_inner_type; /* 0:Ethernet;1:IP;2:Tansport*/
	a_uint8_t ip_ver; /* 0 = IPv4, 1= IPv6 */
	a_uint32_t tunnel_len; /* Tunnel length */
	a_uint32_t tunnel_offset; /* tunnel offset to edit */
	a_uint32_t vlan_offset; /* vlan offset to edit */
	a_uint32_t l3_offset; /* l3 offset to edit */
	a_uint32_t l4_offset; /* l4 offset to edti */
	fal_tunnel_mode_t dscp_mode; /*uniform mode or pipe mode */
	a_uint8_t svlan_fmt; /*0 = Untagged; 1 = Tagged for outer stag*/
	fal_tunnel_mode_t spcp_mode; /*uniform mode or pipe mode */
	fal_tunnel_mode_t sdei_mode; /*uniform mode or pipe mode */
	a_uint8_t cvlan_fmt; /*0 = Untagged; 1 = Tagged for outer ctag*/
	fal_tunnel_mode_t cpcp_mode; /*uniform mode or pipe mode */
	fal_tunnel_mode_t cdei_mode; /*uniform mode or pipe mode */
	a_uint8_t ecn_mode; /* 0x0 = no operation;
			       0x1 = Limited mode in RFC3168 and compatibility mode in RFC6040;
			       0x2 = Full mode in RFC3168;
			       0x3 = ECN mode in RFC4301 and RFC6040 normal mode;
			       */
	fal_tunnel_mode_t ttl_mode; /*uniform mode or pipe mode */
	a_uint8_t l4_proto; /* 0:Non;1:TCP;2:UDP;3:UDP-Lite;4:Reserved (ICMP);5:GRE; */
	a_bool_t l4_checksum_en; /*0 = Disable; 1 = Enable */
	a_bool_t pppoe_en; /* pppoe enable or not */
	a_uint8_t ipv4_df_mode; /* 0: fix value, 1: copy from inner */
	a_uint8_t ipv4_df_mode_ext; /* 0: random value, 1: fix value */
	a_uint8_t ipv4_id_mode; /* 0: fix value, 1: random value */
	a_uint8_t ipv6_flowlable_mode; /* 0: fix value, 1: from hash, 2: copy from inner */
	a_bool_t sport_entry_en; /* 1: apply entry, 0: not apply entry */
	a_uint8_t vni_mode; /* 0: from tunnel header data, 1: from vlan xlat */
	a_bool_t ip_proto_update; /* update ip proto according to transport type */
	a_bool_t vport_en; /* output virtual pot for cpu or not */
	a_uint16_t vport; /* output vitual port id */
	a_uint8_t rt_tlid_type; /*tunnel id type vp or l3_if by getnext cmd */
	a_uint32_t rt_tlid_index; /*tunnel id from the index of vp or l3_if by getnext cmd */
	a_uint32_t rt_tunnel_id; /*tunnel id returned by getnext cmd */
	fal_tunnel_encap_header_t pkt_header; /* packet header data for egress */
	/* new add for ipq53xx */
	a_bool_t mapt_udp_csm0_keep; /* keep udp checksum if original ipv4 udp checksum is zero
					for mapt */
} fal_tunnel_encap_cfg_t;

#define FAL_TUNNEL_ENCAP_SRC1_DATA_WORD_START	1
typedef enum {
	FAL_TUNNEL_RULE_SRC1_ZERO_DATA = 0,
	FAL_TUNNEL_RULE_SRC1_FROM_HEADER_DATA = 1,
	FAL_TUNNEL_RULE_SRC1_DATA_INVALID = 2,
} fal_tunnel_encap_rule_src1_t;

typedef enum {
	FAL_TUNNEL_RULE_SRC2_ZERO_DATA = 0,
	FAL_TUNNEL_RULE_SRC2_PKT_DATA0 = 1,
	FAL_TUNNEL_RULE_SRC2_NAPT_ADDR = 2,
	FAL_TUNNEL_RULE_SRC2_TUNNEL_VNI = 3,
	FAL_TUNNEL_RULE_SRC2_PROTO_MAP0 = 4,
	FAL_TUNNEL_RULE_SRC2_PROTO_MAP1 = 5,
	FAL_TUNNEL_RULE_SRC2_DATA_INVALID = 6,
} fal_tunnel_encap_rule_src2_t;

typedef enum {
	FAL_TUNNEL_RULE_SRC3_ZERO_DATA = 0,
	FAL_TUNNEL_RULE_SRC3_PKT_DATA1 = 1,
	FAL_TUNNEL_RULE_SRC3_NAPT_PORT = 2,
	FAL_TUNNEL_RULE_SRC3_POLICY_ID = 3,
	FAL_TUNNEL_RULE_SRC3_HASH_VALUE = 4,
	FAL_TUNNEL_RULE_SRC3_PROTO_MAP0 = 5,
	FAL_TUNNEL_RULE_SRC3_PROTO_MAP1 = 6,
	FAL_TUNNEL_RULE_SRC3_DATA_INVALID = 7,
} fal_tunnel_encap_rule_src3_t;

typedef struct {
	a_bool_t enable; /* enable or not */
	a_uint8_t src_start; /* the start positon to be selected on src */
	a_uint8_t src_width; /* the selected width on src */
	a_uint8_t dest_pos; /* the position for the selected data to rule_target */
} fal_tunnel_edit_rule_entry_t;

typedef struct {
	fal_tunnel_encap_rule_src1_t src1_sel; /* 0: 16bytes zero,
						* 1: copy 16 bytes from pkt_header_data
						* */
	a_uint8_t src1_start; /* select 16Bytes from position of pkt_header_data */
	fal_tunnel_encap_rule_src2_t src2_sel; /* select source2 to edit */
	fal_tunnel_edit_rule_entry_t src2_entry[2];
	fal_tunnel_encap_rule_src3_t src3_sel; /* select source3 to edit */
	fal_tunnel_edit_rule_entry_t src3_entry[2];
} fal_tunnel_encap_rule_t;

typedef struct {
	a_uint16_t ipv4_id_seed; /* used for pseudo random number generation */
	a_uint16_t ipv4_df_set; /* 0 = Always 0, 1= Follow RFC7915, 2 = Always 1 */
	a_uint16_t udp_sport_base; /* the hash base of the udp port */
	a_uint16_t udp_sport_mask; /* the mask of the udp port */
	a_uint32_t proto_map_data[4]; /* used by edit_rule when proto_map used */
} fal_tunnel_encap_header_ctrl_t;

sw_error_t
fal_tunnel_decap_key_set(a_uint32_t dev_id,
		fal_tunnel_type_t tunnel_type, fal_tunnel_decap_key_t *key_gen);

sw_error_t
fal_tunnel_decap_key_get(a_uint32_t dev_id,
		fal_tunnel_type_t tunnel_type, fal_tunnel_decap_key_t *key_gen);

sw_error_t
fal_tunnel_decap_en_set(a_uint32_t dev_id,
		a_uint32_t tunnel_index, a_bool_t en);

sw_error_t
fal_tunnel_decap_en_get(a_uint32_t dev_id,
		a_uint32_t tunnel_index, a_bool_t *en);

sw_error_t
fal_tunnel_decap_action_update(a_uint32_t dev_id,
		a_uint32_t tunnel_index, fal_tunnel_action_t *update_action);

sw_error_t
fal_tunnel_decap_counter_get(a_uint32_t dev_id,
		a_uint32_t tunnel_index, fal_entry_counter_t *decap_counter);

sw_error_t
fal_tunnel_decap_entry_add(a_uint32_t dev_id,
		fal_tunnel_op_mode_t add_mode, fal_tunnel_decap_entry_t *value);
sw_error_t
fal_tunnel_decap_entry_del(a_uint32_t dev_id,
		fal_tunnel_op_mode_t del_mode, fal_tunnel_decap_entry_t *value);
sw_error_t
fal_tunnel_decap_entry_get(a_uint32_t dev_id,
		fal_tunnel_op_mode_t get_mode, fal_tunnel_decap_entry_t *value);
sw_error_t
fal_tunnel_decap_entry_getnext(a_uint32_t dev_id,
		fal_tunnel_op_mode_t next_mode, fal_tunnel_decap_entry_t *value);
sw_error_t
fal_tunnel_decap_entry_flush(a_uint32_t dev_id);

sw_error_t
fal_tunnel_global_cfg_get(a_uint32_t dev_id,
		fal_tunnel_global_cfg_t *cfg);
sw_error_t
fal_tunnel_global_cfg_set(a_uint32_t dev_id,
		fal_tunnel_global_cfg_t *cfg);
sw_error_t
fal_tunnel_port_intf_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_port_intf_t *port_cfg);
sw_error_t
fal_tunnel_port_intf_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_port_intf_t *port_cfg);
sw_error_t
fal_tunnel_vlan_intf_add(a_uint32_t dev_id,
		fal_tunnel_vlan_intf_t *vlan_cfg);
sw_error_t
fal_tunnel_vlan_intf_getfirst(a_uint32_t dev_id,
		fal_tunnel_vlan_intf_t *vlan_cfg);
sw_error_t
fal_tunnel_vlan_intf_getnext(a_uint32_t dev_id,
		fal_tunnel_vlan_intf_t *vlan_cfg);
sw_error_t
fal_tunnel_vlan_intf_del(a_uint32_t dev_id,
		fal_tunnel_vlan_intf_t *vlan_cfg);
sw_error_t
fal_tunnel_intf_set(a_uint32_t dev_id,
		a_uint32_t l3_if, fal_tunnel_intf_t *intf_t);
sw_error_t
fal_tunnel_intf_get(a_uint32_t dev_id,
		a_uint32_t l3_if, fal_tunnel_intf_t *intf_t);
sw_error_t
fal_tunnel_encap_port_tunnelid_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_id_t *tunnel_id);
sw_error_t
fal_tunnel_encap_port_tunnelid_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_id_t *tunnel_id);
sw_error_t
fal_tunnel_encap_intf_tunnelid_set(a_uint32_t dev_id,
		a_uint32_t intf_id, fal_tunnel_id_t *tunnel_id);
sw_error_t
fal_tunnel_encap_intf_tunnelid_get(a_uint32_t dev_id,
		a_uint32_t intf_id, fal_tunnel_id_t *tunnel_id);
sw_error_t
fal_tunnel_encap_entry_add(a_uint32_t dev_id, a_uint32_t tunnel_id,
		fal_tunnel_encap_cfg_t *tunnel_encap_cfg);
sw_error_t
fal_tunnel_encap_entry_del(a_uint32_t dev_id,
		a_uint32_t tunnel_id);
sw_error_t
fal_tunnel_encap_entry_get(a_uint32_t dev_id, a_uint32_t tunnel_id,
		fal_tunnel_encap_cfg_t *tunnel_encap_cfg);
sw_error_t
fal_tunnel_encap_entry_getnext(a_uint32_t dev_id, a_uint32_t tunnel_id,
		fal_tunnel_encap_cfg_t *tunnel_encap_cfg);
sw_error_t
fal_tunnel_encap_rule_entry_set(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_tunnel_encap_rule_t *rule_entry);
sw_error_t
fal_tunnel_encap_rule_entry_get(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_tunnel_encap_rule_t *rule_entry);
sw_error_t
fal_tunnel_encap_rule_entry_del(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_tunnel_encap_rule_t *rule_entry);
sw_error_t
fal_tunnel_udf_profile_entry_add(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry);
sw_error_t
fal_tunnel_udf_profile_entry_del(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry);
sw_error_t
fal_tunnel_udf_profile_entry_getfirst(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry);
sw_error_t
fal_tunnel_udf_profile_entry_getnext(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry);
sw_error_t
fal_tunnel_udf_profile_cfg_set(a_uint32_t dev_id, a_uint32_t profile_id,
		a_uint32_t udf_idx, fal_tunnel_udf_type_t udf_type, a_uint32_t offset);
sw_error_t
fal_tunnel_udf_profile_cfg_get(a_uint32_t dev_id, a_uint32_t profile_id,
		a_uint32_t udf_idx, fal_tunnel_udf_type_t * udf_type, a_uint32_t * offset);

sw_error_t
fal_tunnel_encap_header_ctrl_set(a_uint32_t dev_id, fal_tunnel_encap_header_ctrl_t *header_ctrl);

sw_error_t
fal_tunnel_encap_header_ctrl_get(a_uint32_t dev_id, fal_tunnel_encap_header_ctrl_t *header_ctrl);

sw_error_t
fal_tunnel_decap_ecn_mode_set(a_uint32_t dev_id, fal_tunnel_decap_ecn_rule_t *ecn_rule,
		fal_tunnel_decap_ecn_action_t *ecn_action);

sw_error_t
fal_tunnel_decap_ecn_mode_get(a_uint32_t dev_id, fal_tunnel_decap_ecn_rule_t *ecn_rule,
		fal_tunnel_decap_ecn_action_t *ecn_action);

sw_error_t
fal_tunnel_encap_ecn_mode_set(a_uint32_t dev_id, fal_tunnel_encap_ecn_t *ecn_rule,
		fal_tunnel_ecn_val_t *ecn_value);

sw_error_t
fal_tunnel_encap_ecn_mode_get(a_uint32_t dev_id, fal_tunnel_encap_ecn_t *ecn_rule,
		fal_tunnel_ecn_val_t *ecn_value);

sw_error_t
fal_tunnel_exp_decap_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);

sw_error_t
fal_tunnel_exp_decap_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);
#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_TUNNEL_H_ */
/**
 * @}
 */
