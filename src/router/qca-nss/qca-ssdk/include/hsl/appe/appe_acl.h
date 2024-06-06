/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#ifndef _APPE_ACL_H_
#define _APPE_ACL_H_

#define IPR_UDF_CTRL_MAX_ENTRY	16
#define IPR_UDF_PROFILE_BASE_MAX_ENTRY	8
#define IPR_UDF_PROFILE_OFFSET_MAX_ENTRY	8

#if defined(MPPE)
#define EG_IPO_EXT_TBL_MAX_ENTRY	256
#define PRE_IPO_RULE_REG_MAX_ENTRY	128
#define PRE_IPO_MASK_REG_MAX_ENTRY	128
#define PRE_IPO_RULE_EXT_1_MAX_ENTRY	16
#define PRE_IPO_RULE_EXT_2_MAX_ENTRY	16
#define PRE_IPO_RULE_EXT_4_MAX_ENTRY	16
#define PRE_IPO_ACTION_MAX_ENTRY	128
#define PRE_IPO_CNT_TBL_MAX_ENTRY	128
#else
#define EG_IPO_EXT_TBL_MAX_ENTRY	1024
#define PRE_IPO_RULE_REG_MAX_ENTRY	512
#define PRE_IPO_MASK_REG_MAX_ENTRY	512
#define PRE_IPO_RULE_EXT_1_MAX_ENTRY	64
#define PRE_IPO_RULE_EXT_2_MAX_ENTRY	64
#define PRE_IPO_RULE_EXT_4_MAX_ENTRY	64
#define PRE_IPO_ACTION_MAX_ENTRY	512
#define PRE_IPO_CNT_TBL_MAX_ENTRY	512
#endif

sw_error_t
appe_ipr_udf_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipr_udf_ctrl_u *value);

sw_error_t
appe_ipr_udf_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipr_udf_ctrl_u *value);

sw_error_t
appe_ipr_udf_profile_base_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipr_udf_profile_base_u *value);

sw_error_t
appe_ipr_udf_profile_base_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipr_udf_profile_base_u *value);

sw_error_t
appe_ipr_udf_profile_offset_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipr_udf_profile_offset_u *value);

sw_error_t
appe_ipr_udf_profile_offset_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipr_udf_profile_offset_u *value);

#if 0
sw_error_t
appe_ipr_udf_ctrl_l3_type_incl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_ctrl_l3_type_incl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_ctrl_l4_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_ctrl_l4_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_ctrl_udf_profile_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_ctrl_udf_profile_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_ctrl_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_ctrl_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_ctrl_l3_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_ctrl_l3_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_ctrl_l4_type_incl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_ctrl_l4_type_incl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_profile_base_udf3_base_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_profile_base_udf3_base_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_profile_base_udf0_base_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_profile_base_udf0_base_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_profile_base_udf1_base_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_profile_base_udf1_base_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_profile_base_udf2_base_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_profile_base_udf2_base_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_profile_offset_udf1_offset_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_profile_offset_udf1_offset_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_profile_offset_udf3_offset_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_profile_offset_udf3_offset_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_profile_offset_udf0_offset_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_profile_offset_udf0_offset_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_ipr_udf_profile_offset_udf2_offset_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_ipr_udf_profile_offset_udf2_offset_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

#endif

sw_error_t
appe_eg_ipo_ext_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_ipo_ext_tbl_u *value);

sw_error_t
appe_eg_ipo_ext_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_ipo_ext_tbl_u *value);

#if 0
sw_error_t
appe_eg_ipo_ext_tbl_policy_id_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_eg_ipo_ext_tbl_policy_id_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

#if defined(MPPE)
sw_error_t
mppe_eg_ipo_ext_tbl_cookie_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
mppe_eg_ipo_ext_tbl_cookie_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
mppe_eg_ipo_ext_tbl_cookie_pri_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
mppe_eg_ipo_ext_tbl_cookie_pri_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);
#endif
#endif

sw_error_t
appe_pre_ipo_rule_reg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_rule_reg_u *value);

sw_error_t
appe_pre_ipo_rule_reg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_rule_reg_u *value);

sw_error_t
appe_pre_ipo_mask_reg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_mask_reg_u *value);

sw_error_t
appe_pre_ipo_mask_reg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_mask_reg_u *value);

sw_error_t
appe_pre_ipo_rule_ext_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_rule_ext_1_u *value);

sw_error_t
appe_pre_ipo_rule_ext_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_rule_ext_1_u *value);

sw_error_t
appe_pre_ipo_rule_ext_2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_rule_ext_2_u *value);

sw_error_t
appe_pre_ipo_rule_ext_2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_rule_ext_2_u *value);

sw_error_t
appe_pre_ipo_rule_ext_4_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_rule_ext_4_u *value);

sw_error_t
appe_pre_ipo_rule_ext_4_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_rule_ext_4_u *value);

#if 0
sw_error_t
appe_pre_ipo_dbg_addr_get(
		a_uint32_t dev_id,
		union pre_ipo_dbg_addr_u *value);

sw_error_t
appe_pre_ipo_dbg_addr_set(
		a_uint32_t dev_id,
		union pre_ipo_dbg_addr_u *value);

sw_error_t
appe_pre_ipo_dbg_data_get(
		a_uint32_t dev_id,
		union pre_ipo_dbg_data_u *value);

sw_error_t
appe_pre_ipo_dbg_data_set(
		a_uint32_t dev_id,
		union pre_ipo_dbg_data_u *value);

sw_error_t
appe_pre_ipo_spare_reg_get(
		a_uint32_t dev_id,
		union pre_ipo_spare_reg_u *value);

sw_error_t
appe_pre_ipo_spare_reg_set(
		a_uint32_t dev_id,
		union pre_ipo_spare_reg_u *value);

sw_error_t
appe_pre_ipo_glb_hit_counter_get(
		a_uint32_t dev_id,
		union pre_ipo_glb_hit_counter_u *value);

sw_error_t
appe_pre_ipo_glb_hit_counter_set(
		a_uint32_t dev_id,
		union pre_ipo_glb_hit_counter_u *value);

sw_error_t
appe_pre_ipo_glb_miss_counter_get(
		a_uint32_t dev_id,
		union pre_ipo_glb_miss_counter_u *value);

sw_error_t
appe_pre_ipo_glb_miss_counter_set(
		a_uint32_t dev_id,
		union pre_ipo_glb_miss_counter_u *value);

sw_error_t
appe_pre_ipo_glb_bypass_counter_get(
		a_uint32_t dev_id,
		union pre_ipo_glb_bypass_counter_u *value);

sw_error_t
appe_pre_ipo_glb_bypass_counter_set(
		a_uint32_t dev_id,
		union pre_ipo_glb_bypass_counter_u *value);
#endif

sw_error_t
appe_pre_ipo_cnt_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_cnt_tbl_u *value);

sw_error_t
appe_pre_ipo_cnt_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_cnt_tbl_u *value);

sw_error_t
appe_pre_ipo_action_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_action_u *value);

sw_error_t
appe_pre_ipo_action_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pre_ipo_action_u *value);

#if 0
sw_error_t
appe_pre_ipo_rule_ext_1_ext2_2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_pre_ipo_rule_ext_1_ext2_2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_pre_ipo_rule_ext_1_ext2_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_pre_ipo_rule_ext_1_ext2_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_pre_ipo_rule_ext_1_ext2_3_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_pre_ipo_rule_ext_1_ext2_3_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_pre_ipo_rule_ext_1_ext2_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_pre_ipo_rule_ext_1_ext2_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_pre_ipo_rule_ext_2_ext4_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_pre_ipo_rule_ext_2_ext4_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_pre_ipo_rule_ext_2_ext4_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_pre_ipo_rule_ext_2_ext4_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_pre_ipo_rule_ext_4_ext8_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_pre_ipo_rule_ext_4_ext8_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_pre_ipo_cnt_tbl_hit_byte_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t *value);

sw_error_t
appe_pre_ipo_cnt_tbl_hit_byte_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t value);

sw_error_t
appe_pre_ipo_cnt_tbl_hit_pkt_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_pre_ipo_cnt_tbl_hit_pkt_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);
#endif
#endif
