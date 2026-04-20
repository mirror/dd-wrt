/*
 * Copyright (c) 2016-2017, 2021, The Linux Foundation. All rights reserved.
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
#ifndef _HPPE_ACL_H_
#define _HPPE_ACL_H_

#if defined(MPPE)
#define IPO_RULE_REG_MAX_ENTRY		128
#define IPO_MASK_REG_MAX_ENTRY		128
#define RULE_EXT_1_REG_MAX_ENTRY	16
#define RULE_EXT_2_REG_MAX_ENTRY	16
#define RULE_EXT_4_REG_MAX_ENTRY	16
#define IPO_ACTION_MAX_ENTRY		128
#define IPO_CNT_TBL_MAX_ENTRY		128
#else
#define IPO_RULE_REG_MAX_ENTRY		512
#define IPO_MASK_REG_MAX_ENTRY		512
#define RULE_EXT_1_REG_MAX_ENTRY	64
#define RULE_EXT_2_REG_MAX_ENTRY	64
#define RULE_EXT_4_REG_MAX_ENTRY	64
#define IPO_ACTION_MAX_ENTRY		512
#define IPO_CNT_TBL_MAX_ENTRY		512
#endif

sw_error_t
hppe_non_ip_udf0_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_non_ip_udf0_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_non_ip_udf1_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_non_ip_udf1_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_non_ip_udf2_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_non_ip_udf2_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_non_ip_udf3_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_non_ip_udf3_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv4_udf0_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv4_udf0_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv4_udf1_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv4_udf1_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv4_udf2_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv4_udf2_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv4_udf3_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv4_udf3_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv6_udf0_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv6_udf0_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv6_udf1_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv6_udf1_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv6_udf2_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv6_udf2_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv6_udf3_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

sw_error_t
hppe_ipv6_udf3_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value);

#if 0
sw_error_t
hppe_non_ip_udf0_ctrl_reg_udf0_base_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_non_ip_udf0_ctrl_reg_udf0_base_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_non_ip_udf0_ctrl_reg_udf0_offset_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_non_ip_udf0_ctrl_reg_udf0_offset_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_non_ip_udf1_ctrl_reg_udf1_base_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_non_ip_udf1_ctrl_reg_udf1_base_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_non_ip_udf1_ctrl_reg_udf1_offset_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_non_ip_udf1_ctrl_reg_udf1_offset_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_non_ip_udf2_ctrl_reg_udf2_offset_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_non_ip_udf2_ctrl_reg_udf2_offset_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_non_ip_udf2_ctrl_reg_udf2_base_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_non_ip_udf2_ctrl_reg_udf2_base_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_non_ip_udf3_ctrl_reg_udf3_base_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_non_ip_udf3_ctrl_reg_udf3_base_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_non_ip_udf3_ctrl_reg_udf3_offset_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_non_ip_udf3_ctrl_reg_udf3_offset_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv4_udf0_ctrl_reg_udf0_base_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv4_udf0_ctrl_reg_udf0_base_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv4_udf0_ctrl_reg_udf0_offset_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv4_udf0_ctrl_reg_udf0_offset_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv4_udf1_ctrl_reg_udf1_base_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv4_udf1_ctrl_reg_udf1_base_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv4_udf1_ctrl_reg_udf1_offset_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv4_udf1_ctrl_reg_udf1_offset_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv4_udf2_ctrl_reg_udf2_offset_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv4_udf2_ctrl_reg_udf2_offset_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv4_udf2_ctrl_reg_udf2_base_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv4_udf2_ctrl_reg_udf2_base_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv4_udf3_ctrl_reg_udf3_base_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv4_udf3_ctrl_reg_udf3_base_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv4_udf3_ctrl_reg_udf3_offset_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv4_udf3_ctrl_reg_udf3_offset_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv6_udf0_ctrl_reg_udf0_base_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv6_udf0_ctrl_reg_udf0_base_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv6_udf0_ctrl_reg_udf0_offset_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv6_udf0_ctrl_reg_udf0_offset_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv6_udf1_ctrl_reg_udf1_base_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv6_udf1_ctrl_reg_udf1_base_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv6_udf1_ctrl_reg_udf1_offset_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv6_udf1_ctrl_reg_udf1_offset_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv6_udf2_ctrl_reg_udf2_offset_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv6_udf2_ctrl_reg_udf2_offset_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv6_udf2_ctrl_reg_udf2_base_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv6_udf2_ctrl_reg_udf2_base_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv6_udf3_ctrl_reg_udf3_base_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv6_udf3_ctrl_reg_udf3_base_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
hppe_ipv6_udf3_ctrl_reg_udf3_offset_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
hppe_ipv6_udf3_ctrl_reg_udf3_offset_set(
		a_uint32_t dev_id,
		unsigned int value);
#endif

sw_error_t
hppe_ipo_rule_reg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_rule_reg_u *value);

sw_error_t
hppe_ipo_rule_reg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_rule_reg_u *value);

sw_error_t
hppe_ipo_mask_reg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_mask_reg_u *value);

sw_error_t
hppe_ipo_mask_reg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_mask_reg_u *value);

sw_error_t
hppe_rule_ext_1_reg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union rule_ext_1_reg_u *value);

sw_error_t
hppe_rule_ext_1_reg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union rule_ext_1_reg_u *value);

sw_error_t
hppe_rule_ext_2_reg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union rule_ext_2_reg_u *value);

sw_error_t
hppe_rule_ext_2_reg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union rule_ext_2_reg_u *value);

sw_error_t
hppe_rule_ext_4_reg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union rule_ext_4_reg_u *value);

sw_error_t
hppe_rule_ext_4_reg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union rule_ext_4_reg_u *value);

#if 0
sw_error_t
hppe_ipo_dbg_addr_reg_get(
		a_uint32_t dev_id,
		union ipo_dbg_addr_reg_u *value);

sw_error_t
hppe_ipo_dbg_addr_reg_set(
		a_uint32_t dev_id,
		union ipo_dbg_addr_reg_u *value);

sw_error_t
hppe_ipo_dbg_data_reg_get(
		a_uint32_t dev_id,
		union ipo_dbg_data_reg_u *value);

sw_error_t
hppe_ipo_dbg_data_reg_set(
		a_uint32_t dev_id,
		union ipo_dbg_data_reg_u *value);

sw_error_t
hppe_ipo_spare_reg_reg_get(
		a_uint32_t dev_id,
		union ipo_spare_reg_reg_u *value);

sw_error_t
hppe_ipo_spare_reg_reg_set(
		a_uint32_t dev_id,
		union ipo_spare_reg_reg_u *value);

sw_error_t
hppe_ipo_glb_hit_counter_reg_get(
		a_uint32_t dev_id,
		union ipo_glb_hit_counter_reg_u *value);

sw_error_t
hppe_ipo_glb_hit_counter_reg_set(
		a_uint32_t dev_id,
		union ipo_glb_hit_counter_reg_u *value);

sw_error_t
hppe_ipo_glb_miss_counter_reg_get(
		a_uint32_t dev_id,
		union ipo_glb_miss_counter_reg_u *value);

sw_error_t
hppe_ipo_glb_miss_counter_reg_set(
		a_uint32_t dev_id,
		union ipo_glb_miss_counter_reg_u *value);

sw_error_t
hppe_ipo_glb_bypass_counter_reg_get(
		a_uint32_t dev_id,
		union ipo_glb_bypass_counter_reg_u *value);

sw_error_t
hppe_ipo_glb_bypass_counter_reg_set(
		a_uint32_t dev_id,
		union ipo_glb_bypass_counter_reg_u *value);
#endif

sw_error_t
hppe_ipo_cnt_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_cnt_tbl_u *value);

sw_error_t
hppe_ipo_cnt_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_cnt_tbl_u *value);

#if 0
sw_error_t
hppe_ipo_cnt_tbl_hit_byte_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t *value);

sw_error_t
hppe_ipo_cnt_tbl_hit_byte_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t value);

sw_error_t
hppe_ipo_cnt_tbl_hit_pkt_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
hppe_ipo_cnt_tbl_hit_pkt_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
hppe_rule_ext_1_reg_ext2_2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
hppe_rule_ext_1_reg_ext2_2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
hppe_rule_ext_1_reg_ext2_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
hppe_rule_ext_1_reg_ext2_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
hppe_rule_ext_1_reg_ext2_3_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
hppe_rule_ext_1_reg_ext2_3_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
hppe_rule_ext_1_reg_ext2_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
hppe_rule_ext_1_reg_ext2_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
hppe_rule_ext_2_reg_ext4_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
hppe_rule_ext_2_reg_ext4_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
hppe_rule_ext_2_reg_ext4_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
hppe_rule_ext_2_reg_ext4_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
hppe_rule_ext_4_reg_ext8_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
hppe_rule_ext_4_reg_ext8_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);
#endif

sw_error_t
hppe_ipo_action_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_action_u *value);

sw_error_t
hppe_ipo_action_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_action_u *value);
#endif

