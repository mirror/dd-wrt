/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
#ifndef _APPE_TUNNEL_PROGRAM_H_
#define _APPE_TUNNEL_PROGRAM_H_

#define TPR_HDR_MATCH_0_MAX_ENTRY	6
#define TPR_HDR_MATCH_1_MAX_ENTRY	6
#define TPR_HDR_MATCH_2_MAX_ENTRY	6
#define TPR_PROGRAM_HDR_MAX_ENTRY	6
#define TPR_PROGRAM_RESULT_MAX_ENTRY	6
#define TPR_PROGRAM_UDF_CTRL_MAX_ENTRY	6
#define TPR_PROGRAM_UDF_DATA_0_MAX_ENTRY	16
#define TPR_PROGRAM_UDF_DATA_1_MAX_ENTRY	16
#define TPR_PROGRAM_UDF_MASK_0_MAX_ENTRY	16
#define TPR_PROGRAM_UDF_MASK_1_MAX_ENTRY	16
#define TPR_PROGRAM_UDF_ACTION_MAX_ENTRY	16

sw_error_t
appe_tpr_hdr_match_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_hdr_match_0_u *value);

sw_error_t
appe_tpr_hdr_match_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_hdr_match_0_u *value);

sw_error_t
appe_tpr_hdr_match_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_hdr_match_1_u *value);

sw_error_t
appe_tpr_hdr_match_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_hdr_match_1_u *value);

sw_error_t
appe_tpr_hdr_match_2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_hdr_match_2_u *value);

sw_error_t
appe_tpr_hdr_match_2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_hdr_match_2_u *value);

sw_error_t
appe_tpr_program_hdr_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_hdr_u *value);

sw_error_t
appe_tpr_program_hdr_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_hdr_u *value);

sw_error_t
appe_tpr_program_result_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_result_u *value);

sw_error_t
appe_tpr_program_result_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_result_u *value);

sw_error_t
appe_tpr_program_udf_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_ctrl_u *value);

sw_error_t
appe_tpr_program_udf_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_ctrl_u *value);

sw_error_t
appe_tpr_program_udf_data_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_data_0_u *value);

sw_error_t
appe_tpr_program_udf_data_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_data_0_u *value);

sw_error_t
appe_tpr_program_udf_data_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_data_1_u *value);

sw_error_t
appe_tpr_program_udf_data_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_data_1_u *value);

sw_error_t
appe_tpr_program_udf_mask_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_mask_0_u *value);

sw_error_t
appe_tpr_program_udf_mask_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_mask_0_u *value);

sw_error_t
appe_tpr_program_udf_mask_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_mask_1_u *value);

sw_error_t
appe_tpr_program_udf_mask_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_mask_1_u *value);

sw_error_t
appe_tpr_program_udf_action_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_action_u *value);

sw_error_t
appe_tpr_program_udf_action_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_action_u *value);

#if 0
sw_error_t
appe_tpr_hdr_match_0_ip_ver_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_hdr_match_0_ip_ver_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_hdr_match_0_cur_hdr_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_hdr_match_0_cur_hdr_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_hdr_match_1_protocol_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_hdr_match_1_protocol_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_hdr_match_2_mask_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_hdr_match_2_mask_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);
#endif

sw_error_t
appe_tpr_program_hdr_hdr_type_map_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_hdr_hdr_type_map_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

#if 0
sw_error_t
appe_tpr_program_result_len_unit_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_result_len_unit_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_result_next_hdr_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_result_next_hdr_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_result_next_hdr_mode_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_result_next_hdr_mode_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_result_hdr_len_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_result_hdr_len_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_result_len_mask_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_result_len_mask_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_result_hdr_pos_mode_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_result_hdr_pos_mode_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_ctrl_udf1_offset_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_ctrl_udf1_offset_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_ctrl_udf0_offset_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_ctrl_udf0_offset_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_ctrl_udf2_offset_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_ctrl_udf2_offset_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_data_0_data1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_data_0_data1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_data_0_data0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_data_0_data0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_data_1_program_id_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_data_1_program_id_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_data_1_data2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_data_1_data2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_data_1_udf1_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_data_1_udf1_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_data_1_udf2_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_data_1_udf2_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_data_1_udf0_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_data_1_udf0_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_data_1_comp_mode_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_data_1_comp_mode_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_mask_0_mask1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_mask_0_mask1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_mask_0_mask0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_mask_0_mask0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_mask_1_udf2_valid_mask_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_mask_1_udf2_valid_mask_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_mask_1_udf0_valid_mask_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_mask_1_udf0_valid_mask_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_mask_1_udf1_valid_mask_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_mask_1_udf1_valid_mask_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_mask_1_mask2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_mask_1_mask2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_action_next_hdr_type_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_action_next_hdr_type_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_action_exception_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_action_exception_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_action_next_hdr_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_action_next_hdr_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_action_hdr_len_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_action_hdr_len_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_program_udf_action_hdr_len_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_program_udf_action_hdr_len_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);
#endif
#endif
