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


/**
 * @defgroup
 * @{
 */
#ifndef _APPE_SEC_H_
#define _APPE_SEC_H_

#define L2_FLOW_HIT_EXP_CTRL_MAX_ENTRY  72
#define L3_FLOW_HIT_EXP_CTRL_MAX_ENTRY  72
#define L3_FLOW_HIT_MISS_EXP_CTRL_MAX_ENTRY  72
#define L2_FLOW_HIT_MISS_EXP_CTRL_MAX_ENTRY  72
#define TL_EXCEPTION_CMD_MAX_ENTRY  88
#define TL_EXP_CTRL_PROFILE0_MAX_ENTRY  88
#define TL_EXP_CTRL_PROFILE1_MAX_ENTRY  88
#define TL_EXP_CTRL_PROFILE2_MAX_ENTRY  88
#define TL_EXP_CTRL_PROFILE3_MAX_ENTRY  88
#define TPR_EXCEPTION_CTRL_0_MAX_ENTRY  16
#define TPR_EXCEPTION_CTRL_1_MAX_ENTRY  16


sw_error_t
appe_l2_flow_hit_exp_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l2_flow_hit_exp_ctrl_u *value);

sw_error_t
appe_l2_flow_hit_exp_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l2_flow_hit_exp_ctrl_u *value);

sw_error_t
appe_l3_flow_hit_exp_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l3_flow_hit_exp_ctrl_u *value);

sw_error_t
appe_l3_flow_hit_exp_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l3_flow_hit_exp_ctrl_u *value);

sw_error_t
appe_l3_flow_hit_miss_exp_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l3_flow_hit_miss_exp_ctrl_u *value);

sw_error_t
appe_l3_flow_hit_miss_exp_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l3_flow_hit_miss_exp_ctrl_u *value);

sw_error_t
appe_l2_flow_hit_miss_exp_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l2_flow_hit_miss_exp_ctrl_u *value);

sw_error_t
appe_l2_flow_hit_miss_exp_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l2_flow_hit_miss_exp_ctrl_u *value);

#ifndef IN_SEC_MINI
sw_error_t
appe_l2_excep_ctrl_get(
		a_uint32_t dev_id,
		union l2_excep_ctrl_u *value);

sw_error_t
appe_l2_excep_ctrl_set(
		a_uint32_t dev_id,
		union l2_excep_ctrl_u *value);

sw_error_t
appe_l2_excep_ctrl_tunnel_excep_fwd_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_l2_excep_ctrl_tunnel_excep_fwd_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_l2_flow_hit_exp_ctrl_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l2_flow_hit_exp_ctrl_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_l3_flow_hit_exp_ctrl_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l3_flow_hit_exp_ctrl_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_l3_flow_hit_miss_exp_ctrl_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l3_flow_hit_miss_exp_ctrl_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_l2_flow_hit_miss_exp_ctrl_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l2_flow_hit_miss_exp_ctrl_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tl_exception_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exception_cmd_u *value);

sw_error_t
appe_tl_exception_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exception_cmd_u *value);

sw_error_t
appe_tl_exp_ctrl_profile0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile0_u *value);

sw_error_t
appe_tl_exp_ctrl_profile0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile0_u *value);

sw_error_t
appe_tl_exp_ctrl_profile1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile1_u *value);

sw_error_t
appe_tl_exp_ctrl_profile1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile1_u *value);

sw_error_t
appe_tl_exp_ctrl_profile2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile2_u *value);

sw_error_t
appe_tl_exp_ctrl_profile2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile2_u *value);

sw_error_t
appe_tl_exp_ctrl_profile3_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile3_u *value);

sw_error_t
appe_tl_exp_ctrl_profile3_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile3_u *value);

sw_error_t
appe_tl_exception_cmd_tl_excep_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tl_exception_cmd_tl_excep_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tl_exception_cmd_de_acce_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tl_exception_cmd_de_acce_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tl_exp_ctrl_profile0_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tl_exp_ctrl_profile0_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tl_exp_ctrl_profile1_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tl_exp_ctrl_profile1_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tl_exp_ctrl_profile2_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tl_exp_ctrl_profile2_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tl_exp_ctrl_profile3_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tl_exp_ctrl_profile3_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_l3_exception_parsing_ctrl_get(
		a_uint32_t dev_id,
		union tpr_l3_exception_parsing_ctrl_u *value);

sw_error_t
appe_tpr_l3_exception_parsing_ctrl_set(
		a_uint32_t dev_id,
		union tpr_l3_exception_parsing_ctrl_u *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_get(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_0_u *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_set(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_0_u *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_get(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_1_u *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_set(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_1_u *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_get(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_2_u *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_set(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_2_u *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_get(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_3_u *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_set(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_3_u *value);

sw_error_t
appe_tpr_exception_ctrl_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_exception_ctrl_0_u *value);

sw_error_t
appe_tpr_exception_ctrl_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_exception_ctrl_0_u *value);

sw_error_t
appe_tpr_exception_ctrl_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_exception_ctrl_1_u *value);

sw_error_t
appe_tpr_exception_ctrl_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_exception_ctrl_1_u *value);

sw_error_t
appe_tpr_l3_exception_parsing_ctrl_small_hop_limit_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l3_exception_parsing_ctrl_small_hop_limit_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l3_exception_parsing_ctrl_small_ttl_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l3_exception_parsing_ctrl_small_ttl_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags1_mask_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags1_mask_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags0_mask_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags0_mask_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags0_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags0_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags1_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags1_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags2_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags2_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags3_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags3_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags3_mask_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags3_mask_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags2_mask_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags2_mask_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags4_mask_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags4_mask_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags4_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags4_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags5_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags5_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags5_mask_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags5_mask_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags6_mask_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags6_mask_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags7_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags7_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags7_mask_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags7_mask_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags6_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags6_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_exception_ctrl_0_flags_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_exception_ctrl_0_flags_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_exception_ctrl_0_mask_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_exception_ctrl_0_mask_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_exception_ctrl_1_hdr_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_exception_ctrl_1_hdr_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_exception_ctrl_1_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_exception_ctrl_1_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_tpr_exception_ctrl_1_comp_mode_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_tpr_exception_ctrl_1_comp_mode_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);
#endif

#endif

