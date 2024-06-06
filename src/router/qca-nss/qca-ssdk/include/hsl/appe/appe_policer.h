/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _APPE_POLICER_H_
#define _APPE_POLICER_H_

#define DSCP_REMAP_TBL_MAX_ENTRY	128

sw_error_t
appe_in_meter_head_reg_get(
		a_uint32_t dev_id,
		union in_meter_head_reg_u *value);

sw_error_t
appe_in_meter_head_reg_set(
		a_uint32_t dev_id,
		union in_meter_head_reg_u *value);

#ifndef IN_POLICER_MINI
sw_error_t
appe_dscp_remap_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union dscp_remap_tbl_u *value);

sw_error_t
appe_dscp_remap_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union dscp_remap_tbl_u *value);

sw_error_t
appe_in_meter_head_reg_mef10dot3_en_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_in_meter_head_reg_mef10dot3_en_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_in_meter_head_reg_meter_ll_head_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_in_meter_head_reg_meter_ll_head_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_in_meter_head_reg_meter_ll_tail_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_in_meter_head_reg_meter_ll_tail_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_dscp_remap_tbl_dei_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_dscp_remap_tbl_dei_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_dscp_remap_tbl_dscp_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_dscp_remap_tbl_dscp_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_dscp_remap_tbl_pcp_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_dscp_remap_tbl_pcp_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_in_acl_meter_cfg_tbl_exceed_chg_dscp_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_in_acl_meter_cfg_tbl_exceed_chg_dscp_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_in_acl_meter_cfg_tbl_exceed_dscp_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_in_acl_meter_cfg_tbl_exceed_dscp_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_in_acl_meter_cfg_tbl_violate_chg_dscp_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_in_acl_meter_cfg_tbl_violate_chg_dscp_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_in_acl_meter_cfg_tbl_violate_dscp_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_in_acl_meter_cfg_tbl_violate_dscp_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_in_acl_meter_cfg_tbl_violate_remap_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_in_acl_meter_cfg_tbl_violate_remap_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_in_acl_meter_cfg_tbl_exceed_remap_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_in_acl_meter_cfg_tbl_exceed_remap_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_in_acl_meter_cfg_tbl_cir_max_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_in_acl_meter_cfg_tbl_cir_max_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_in_acl_meter_cfg_tbl_eir_max_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_in_acl_meter_cfg_tbl_eir_max_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_in_acl_meter_cfg_tbl_grp_end_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_in_acl_meter_cfg_tbl_grp_end_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_in_acl_meter_cfg_tbl_grp_cf_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_in_acl_meter_cfg_tbl_grp_cf_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_in_acl_meter_cfg_tbl_nxt_ptr_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_in_acl_meter_cfg_tbl_nxt_ptr_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);
#endif
#endif

