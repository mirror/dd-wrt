/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 */


/**
* @defgroup fal_gen _FAL_MAPT_H_
* @{
*/
#ifndef _FAL_MAPT_H_
#define _FAL_MAPT_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "sw.h"
#include "fal_type.h"
#include "fal_tunnel.h"

typedef struct {
	fal_fwd_cmd_t src_check_action; /* MAPT inbound src check failed
					 * 0x0 = Forward
					 * 0x1 = Drop
					 * 0x2 = COPY
					 * 0x3 = RDT_TO_CPU
					 */
	fal_fwd_cmd_t dst_check_action; /* MAPT inbound dst check failed
					 * 0x0 = Forward
					 * 0x1 = Drop
					 * 0x2 = COPY
					 * 0x3 = RDT_TO_CPU
					 */
	fal_fwd_cmd_t no_tcp_udp_action; /* MAPT inbound with no tcp or udp
					  * 0x0 = Forward
					  * 0x1 = Drop
					  * 0x2 = COPY
					  * 0x3 = RDT_TO_CPU
					  */
	fal_fwd_cmd_t udp_csum_zero_action; /* MAPT inbound with csum 0
					     * 0x0 = Forward
					     * 0x1 = Drop
					     * 0x2 = COPY
					     * 0x3 = RDT_TO_CPU
					     */
	a_uint8_t ipv4_df_set; /* 0 = always 0, 1= follow rfc7915, 2 = always 1 */
} fal_mapt_decap_ctrl_t;

typedef enum {
	FAL_TUNNEL_MAPT_ZERO_DATA = 0, /* select zero data */
	FAL_TUNNEL_MAPT_FROM_SRC = 1,  /* select src ip or port */
	FAL_TUNNEL_MAPT_FROM_DST = 2,  /* select dst ip or port */
	FAL_TUNNEL_MAPT_INVALID = 3,
} fal_tunnel_mapt_rule_src_t;

typedef struct {
	fal_ip4_addr_t ip4_addr; /* ipv4 address to be edited */
	fal_tunnel_mapt_rule_src_t ip6_addr_src; /* ipv6 address source
						   * 0:zero data,
						   * 1:src addr,
						   * 2:dst addr
						   * */
	fal_tunnel_mapt_rule_src_t proto_src; /* transport protocol source
					       * 0:zero data,
					       * 1:src port,
					       * 2:dst port
					       * */
	fal_tunnel_edit_rule_entry_t ip6_suffix_sel; /* select suffix from ipv6 addr */
	fal_tunnel_edit_rule_entry_t ip6_proto_sel; /* select psid from ipv6 addr */
	fal_tunnel_edit_rule_entry_t proto_sel; /* select psid from transport proto */
	a_bool_t check_proto_enable; /* check whether psid from ipv6 addr same as
				      * from transport proto
				      */
} fal_mapt_decap_edit_rule_entry_t;

typedef struct {
	fal_ip6_addr_t ip6_addr; /* ipv6 address */
	a_uint8_t ip6_prefix_len; /* ipv6 prefix length */
	a_uint8_t edit_rule_id; /* edit rule id */
	fal_tunnel_verify_entry_t verify_entry; /* l3_if, vlan verification */
	a_bool_t dst_is_local; /* ipv6 dst address for local */
	a_bool_t src_info_enable; /*enable new source info or not */
	a_uint8_t src_info_type; /*0 = Virtual port; 1 = L3_IF for tunnel payload */
	a_uint16_t src_info; /*Virtual port ID or L3_IF index as source info */
	a_uint8_t exp_profile; /*Exception profile ID */
	a_uint32_t pkt_counter; /* hit packet counter */
	a_uint64_t byte_counter; /* hit byte counter */
	a_uint32_t entry_index; /* mapt entry index */
	fal_tunnel_op_mode_t op_mode; /* hash or index */
	/* new add for IPQ53xx */
	a_bool_t service_code_en; /* enable new service code or not */
	a_uint8_t service_code; /* updated service code */
} fal_mapt_decap_entry_t;

sw_error_t
fal_mapt_decap_ctrl_set(a_uint32_t dev_id, fal_mapt_decap_ctrl_t *decap_ctrl);

sw_error_t
fal_mapt_decap_ctrl_get(a_uint32_t dev_id, fal_mapt_decap_ctrl_t *decap_ctrl);

sw_error_t
fal_mapt_decap_rule_entry_set(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry);

sw_error_t
fal_mapt_decap_rule_entry_get(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry);

sw_error_t
fal_mapt_decap_rule_entry_del(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry);

sw_error_t
fal_mapt_decap_entry_add(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry);

sw_error_t
fal_mapt_decap_entry_del(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry);

sw_error_t
fal_mapt_decap_entry_getfirst(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry);

sw_error_t
fal_mapt_decap_entry_getnext(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry);

sw_error_t
fal_mapt_decap_en_set(a_uint32_t dev_id, a_uint32_t mapt_index, a_bool_t en);

sw_error_t
fal_mapt_decap_en_get(a_uint32_t dev_id, a_uint32_t mapt_index, a_bool_t *en);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_MAPT_H_ */
/**
 * @}
 */
