/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
#ifndef _ADPT_APPE_ACL_H_
#define _ADPT_APPE_ACL_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#if defined(MPPE)
#define ADPT_PRE_ACL_HW_LIST_NUM 16
#else
#define ADPT_PRE_ACL_HW_LIST_NUM 64
#endif
#define ADPT_PRE_ACL_ENTRY_NUM_PER_LIST 8 /* hw rule entries number per hw list */

#define ADPT_ACL_APPE_TUNNEL_RULE 16
#define ADPT_ACL_APPE_EXT_UDF0_RULE 17
#define ADPT_ACL_APPE_EXT_UDF1_RULE 18

#if defined(CONFIG_CPU_BIG_ENDIAN)
typedef struct{
	a_uint32_t  tunnel_info:32;

	a_uint32_t  reserved1:12;
	a_uint32_t  l3_packet_type:3;
	a_uint32_t  l3_fragment:1;
	a_uint32_t  reserved0:7;
	a_uint32_t  tunnel_inner_type:2;
	a_uint32_t  tunnel_decap_en:1;
	a_uint32_t  tunnel_type:5;
	a_uint32_t  tunnel_info_valid:1;
}ADPT_APPE_ACL_TUNNEL_RULE;

typedef struct{
	a_uint32_t  tunnel_info_mask:32;

	a_uint32_t  reserved1:12;
	a_uint32_t  l3_packet_type_mask:3;
	a_uint32_t  l3_fragment_mask:1;
	a_uint32_t  reserved0:7;
	a_uint32_t  tunnel_inner_type_mask:2;
	a_uint32_t  tunnel_decap_en_mask:1;
	a_uint32_t  tunnel_type_mask:5;
	a_uint32_t  tunnel_info_valid_mask:1;
}ADPT_APPE_ACL_TUNNEL_RULE_MASK;

typedef struct{
	a_uint32_t udf1:16;
	a_uint32_t udf0:16;

	a_uint32_t reserved1:11;
	a_uint32_t is_ip:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t udfprofile_valid:1;
	a_uint32_t udf1_valid:1;
	a_uint32_t udf0_valid:1;
	a_uint32_t reserved:13;
	a_uint32_t udfprofile:3;
}ADPT_APPE_ACL_EXT_UDF_RULE;

typedef struct{
	a_uint32_t udf1_mask:16;
	a_uint32_t udf0_mask:16;

	a_uint32_t reserved1:11;
	a_uint32_t is_ip:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t udfprofile_valid:1;
	a_uint32_t udf1_valid:1;
	a_uint32_t udf0_valid:1;
	a_uint32_t reserved:13;
	a_uint32_t udfprofile_mask:3;
}ADPT_APPE_ACL_EXT_UDF_RULE_MASK;
#else
typedef struct{
	a_uint32_t  tunnel_info:32;
	a_uint32_t  tunnel_info_valid:1;
	a_uint32_t  tunnel_type:5;
	a_uint32_t  tunnel_decap_en:1;
	a_uint32_t  tunnel_inner_type:2;
	a_uint32_t  reserved0:7;
	a_uint32_t  l3_fragment:1;
	a_uint32_t  l3_packet_type:3;
	a_uint32_t  reserved1:1;
}ADPT_APPE_ACL_TUNNEL_RULE;

typedef struct{
	a_uint32_t  tunnel_info_mask:32;
	a_uint32_t  tunnel_info_valid_mask:1;
	a_uint32_t  tunnel_type_mask:5;
	a_uint32_t  tunnel_decap_en_mask:1;
	a_uint32_t  tunnel_inner_type_mask:2;
	a_uint32_t  reserved0:7;
	a_uint32_t  l3_fragment_mask:1;
	a_uint32_t  l3_packet_type_mask:3;
	a_uint32_t  reserved1:1;
}ADPT_APPE_ACL_TUNNEL_RULE_MASK;

typedef struct{
	a_uint32_t udf0:16;
	a_uint32_t udf1:16;
	a_uint32_t udfprofile:3;
	a_uint32_t reserved:13;
	a_uint32_t udf0_valid:1;
	a_uint32_t udf1_valid:1;
	a_uint32_t udfprofile_valid:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t is_ip:1;
}ADPT_APPE_ACL_EXT_UDF_RULE;

typedef struct{
	a_uint32_t udf0_mask:16;
	a_uint32_t udf1_mask:16;
	a_uint32_t udfprofile_mask:3;
	a_uint32_t reserved:13;
	a_uint32_t udf0_valid:1;
	a_uint32_t udf1_valid:1;
	a_uint32_t udfprofile_valid:1;
	a_uint32_t is_ipv6:1;
	a_uint32_t is_ip:1;
}ADPT_APPE_ACL_EXT_UDF_RULE_MASK;
#endif

sw_error_t
_adpt_appe_acl_ext_set(a_uint32_t dev_id, fal_acl_rule_t * rule,
		a_uint32_t hw_list_id, a_uint32_t hw_entries);

sw_error_t
_adpt_appe_acl_ext_get(a_uint32_t dev_id,
		a_uint32_t hw_list_id, a_uint32_t hw_entries, fal_acl_rule_t * rule);

sw_error_t
_adpt_appe_acl_ext_clear(a_uint32_t dev_id,
		a_uint32_t hw_list_id, a_uint32_t hw_entries);

sw_error_t
_adpt_appe_pre_acl_tunnel_rule_sw_2_hw(fal_acl_tunnel_info_t *rule,
	ADPT_APPE_ACL_TUNNEL_RULE *tunnelrule, ADPT_APPE_ACL_TUNNEL_RULE_MASK *tunnelrule_mask);

sw_error_t
_adpt_appe_pre_acl_tunnel_rule_hw_2_sw(ADPT_APPE_ACL_TUNNEL_RULE *tunnelrule,
	ADPT_APPE_ACL_TUNNEL_RULE_MASK *tunnelrule_mask, fal_acl_tunnel_info_t * rule);

sw_error_t
_adpt_appe_acl_ext_udf_rule_sw_2_hw(fal_acl_rule_t *rule, a_uint32_t is_win1,
	ADPT_APPE_ACL_EXT_UDF_RULE * extudfrule, ADPT_APPE_ACL_EXT_UDF_RULE_MASK *extudfrule_mask,
	a_uint8_t *range_en);

sw_error_t
_adpt_appe_acl_ext_udf_rule_hw_2_sw(a_uint32_t is_win1, ADPT_APPE_ACL_EXT_UDF_RULE * extudfrule,
	ADPT_APPE_ACL_EXT_UDF_RULE_MASK *extudfrule_mask, a_uint8_t range_en, fal_acl_rule_t *rule);

sw_error_t
_adpt_appe_acl_udf_fields_check(a_uint32_t dev_id, a_uint32_t rule_id,
	a_uint32_t rule_nr, fal_acl_rule_t * rule, a_uint32_t * rule_type_map);

sw_error_t
_adpt_appe_pre_acl_tunnel_info_fields_check(a_uint32_t dev_id, a_uint32_t rule_id,
	a_uint32_t rule_nr, fal_acl_tunnel_info_t * rule, a_uint32_t * rule_type_map);

sw_error_t
_adpt_appe_pre_acl_rule_ext_set(a_uint32_t dev_id, a_uint32_t hw_list_id,
	a_uint32_t ext_1, a_uint32_t ext_2, a_uint32_t ext_4);

sw_error_t
_adpt_appe_pre_acl_rule_ext_clear(a_uint32_t dev_id, a_uint32_t hw_list_id,
	a_uint32_t ext_1, a_uint32_t ext_2, a_uint32_t ext_4);

sw_error_t
_adpt_appe_pre_acl_rule_hw_delete(a_uint32_t dev_id,
	a_uint32_t hw_list_id, a_uint32_t hw_entries, a_uint32_t rule_nr);

sw_error_t
_adpt_appe_pre_acl_rule_hw_add(a_uint32_t dev_id, a_uint32_t list_pri,
	a_uint32_t hw_list_id, a_uint32_t rule_id, a_uint32_t rule_nr,
	fal_acl_rule_t * rule, fal_acl_rule_t * tmp_rule,
	ADPT_HPPE_ACL_RULE_MAP *rule_map,
	ADPT_HPPE_ACL_RULE_MAP *inner_rule_map,
	a_uint32_t allocated_entries);

sw_error_t
_adpt_appe_pre_acl_rule_sw_query(a_uint32_t dev_id,
	a_uint32_t hw_list_id, a_uint32_t hw_entries, fal_acl_rule_t * rule);

sw_error_t
_adpt_appe_pre_acl_rule_bind(a_uint32_t dev_id, a_uint32_t hw_list_id, a_uint32_t hw_entries,
	fal_acl_direc_t direc, fal_acl_bind_obj_t obj_t, a_uint32_t obj_idx);

sw_error_t
_adpt_appe_pre_acl_rule_unbind(a_uint32_t dev_id, a_uint32_t hw_list_id, a_uint32_t hw_entries,
	fal_acl_direc_t direc, fal_acl_bind_obj_t obj_t, a_uint32_t obj_idx);

sw_error_t
_adpt_appe_pre_acl_rule_dump(a_uint32_t dev_id, a_uint32_t hw_list_id, a_uint32_t hw_entries);

sw_error_t
_adpt_appe_pre_acl_counter_get(a_uint32_t dev_id,
	a_uint32_t entry_index, fal_entry_counter_t *acl_counter);

sw_error_t
adpt_appe_acl_udf_profile_set(a_uint32_t dev_id, fal_acl_udf_pkt_type_t pkt_type,
	a_uint32_t udf_idx, fal_acl_udf_type_t udf_type, a_uint32_t offset);
sw_error_t
adpt_appe_acl_udf_profile_get(a_uint32_t dev_id, fal_acl_udf_pkt_type_t pkt_type,
	a_uint32_t udf_idx, fal_acl_udf_type_t * udf_type, a_uint32_t * offset);
sw_error_t
adpt_appe_acl_udf_profile_entry_add(a_uint32_t dev_id, a_uint32_t profile_id,
	fal_acl_udf_profile_entry_t * entry);
sw_error_t
adpt_appe_acl_udf_profile_entry_del(a_uint32_t dev_id, a_uint32_t profile_id,
	fal_acl_udf_profile_entry_t * entry);
sw_error_t
adpt_appe_acl_udf_profile_entry_getfirst(a_uint32_t dev_id, a_uint32_t profile_id,
	fal_acl_udf_profile_entry_t * entry);
sw_error_t
adpt_appe_acl_udf_profile_entry_getnext(a_uint32_t dev_id, a_uint32_t profile_id,
	fal_acl_udf_profile_entry_t * entry);
sw_error_t
adpt_appe_acl_udf_profile_cfg_set(a_uint32_t dev_id, a_uint32_t profile_id,
	a_uint32_t udf_idx, fal_acl_udf_type_t udf_type, a_uint32_t offset);
sw_error_t
adpt_appe_acl_udf_profile_cfg_get(a_uint32_t dev_id, a_uint32_t profile_id,
	a_uint32_t udf_idx, fal_acl_udf_type_t * udf_type, a_uint32_t * offset);

sw_error_t
adpt_appe_acl_vpgroup_set(a_uint32_t dev_id, a_uint32_t vport_id,
	fal_vport_type_t vport_type, a_uint32_t vpgroup_id);
sw_error_t
adpt_appe_acl_vpgroup_get(a_uint32_t dev_id, a_uint32_t vport_id,
        fal_vport_type_t vport_type, a_uint32_t * vpgroup_id);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif
/**
 * @}
 */
