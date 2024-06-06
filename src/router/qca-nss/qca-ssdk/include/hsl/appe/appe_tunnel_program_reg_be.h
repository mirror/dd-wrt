/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#ifndef APPE_TUNNEL_PROGRAM_REG_H
#define APPE_TUNNEL_PROGRAM_REG_H

/*[register] TPR_HDR_MATCH_0*/
#define TPR_HDR_MATCH_0
#define TPR_HDR_MATCH_0_ADDRESS 0x4b0
#define TPR_HDR_MATCH_0_NUM     6
#define TPR_HDR_MATCH_0_INC     0x4
#define TPR_HDR_MATCH_0_TYPE    REG_TYPE_RW
#define TPR_HDR_MATCH_0_DEFAULT 0x3f
	/*[field] CUR_HDR_TYPE*/
	#define TPR_HDR_MATCH_0_CUR_HDR_TYPE
	#define TPR_HDR_MATCH_0_CUR_HDR_TYPE_OFFSET  0
	#define TPR_HDR_MATCH_0_CUR_HDR_TYPE_LEN     4
	#define TPR_HDR_MATCH_0_CUR_HDR_TYPE_DEFAULT 0xf
	/*[field] IP_VER*/
	#define TPR_HDR_MATCH_0_IP_VER
	#define TPR_HDR_MATCH_0_IP_VER_OFFSET  4
	#define TPR_HDR_MATCH_0_IP_VER_LEN     2
	#define TPR_HDR_MATCH_0_IP_VER_DEFAULT 0x3

struct tpr_hdr_match_0 {
	a_uint32_t  _reserved0:26;
	a_uint32_t  ip_ver:2;
	a_uint32_t  cur_hdr_type:4;
};

union tpr_hdr_match_0_u {
	a_uint32_t val;
	struct tpr_hdr_match_0 bf;
};

/*[register] TPR_HDR_MATCH_1*/
#define TPR_HDR_MATCH_1
#define TPR_HDR_MATCH_1_ADDRESS 0x4d0
#define TPR_HDR_MATCH_1_NUM     6
#define TPR_HDR_MATCH_1_INC     0x4
#define TPR_HDR_MATCH_1_TYPE    REG_TYPE_RW
#define TPR_HDR_MATCH_1_DEFAULT 0x0
	/*[field] PROTOCOL*/
	#define TPR_HDR_MATCH_1_PROTOCOL
	#define TPR_HDR_MATCH_1_PROTOCOL_OFFSET  0
	#define TPR_HDR_MATCH_1_PROTOCOL_LEN     32
	#define TPR_HDR_MATCH_1_PROTOCOL_DEFAULT 0x0

struct tpr_hdr_match_1 {
	a_uint32_t  protocol:32;
};

union tpr_hdr_match_1_u {
	a_uint32_t val;
	struct tpr_hdr_match_1 bf;
};

/*[register] TPR_HDR_MATCH_2*/
#define TPR_HDR_MATCH_2
#define TPR_HDR_MATCH_2_ADDRESS 0x4e8
#define TPR_HDR_MATCH_2_NUM     6
#define TPR_HDR_MATCH_2_INC     0x4
#define TPR_HDR_MATCH_2_TYPE    REG_TYPE_RW
#define TPR_HDR_MATCH_2_DEFAULT 0x0
	/*[field] MASK*/
	#define TPR_HDR_MATCH_2_MASK
	#define TPR_HDR_MATCH_2_MASK_OFFSET  0
	#define TPR_HDR_MATCH_2_MASK_LEN     32
	#define TPR_HDR_MATCH_2_MASK_DEFAULT 0x0

struct tpr_hdr_match_2 {
	a_uint32_t  mask:32;
};

union tpr_hdr_match_2_u {
	a_uint32_t val;
	struct tpr_hdr_match_2 bf;
};

/*[register] TPR_PROGRAM_HDR*/
#define TPR_PROGRAM_HDR
#define TPR_PROGRAM_HDR_ADDRESS 0x500
#define TPR_PROGRAM_HDR_NUM     6
#define TPR_PROGRAM_HDR_INC     0x4
#define TPR_PROGRAM_HDR_TYPE    REG_TYPE_RW
#define TPR_PROGRAM_HDR_DEFAULT 0x0
	/*[field] HDR_TYPE_MAP*/
	#define TPR_PROGRAM_HDR_HDR_TYPE_MAP
	#define TPR_PROGRAM_HDR_HDR_TYPE_MAP_OFFSET  0
	#define TPR_PROGRAM_HDR_HDR_TYPE_MAP_LEN     6
	#define TPR_PROGRAM_HDR_HDR_TYPE_MAP_DEFAULT 0x0

struct tpr_program_hdr {
	a_uint32_t  _reserved0:26;
	a_uint32_t  hdr_type_map:6;
};

union tpr_program_hdr_u {
	a_uint32_t val;
	struct tpr_program_hdr bf;
};

/*[register] TPR_PROGRAM_RESULT*/
#define TPR_PROGRAM_RESULT
#define TPR_PROGRAM_RESULT_ADDRESS 0x520
#define TPR_PROGRAM_RESULT_NUM     6
#define TPR_PROGRAM_RESULT_INC     0x4
#define TPR_PROGRAM_RESULT_TYPE    REG_TYPE_RW
#define TPR_PROGRAM_RESULT_DEFAULT 0x0
	/*[field] NEXT_HDR_MODE*/
	#define TPR_PROGRAM_RESULT_NEXT_HDR_MODE
	#define TPR_PROGRAM_RESULT_NEXT_HDR_MODE_OFFSET  0
	#define TPR_PROGRAM_RESULT_NEXT_HDR_MODE_LEN     1
	#define TPR_PROGRAM_RESULT_NEXT_HDR_MODE_DEFAULT 0x0
	/*[field] HDR_POS_MODE*/
	#define TPR_PROGRAM_RESULT_HDR_POS_MODE
	#define TPR_PROGRAM_RESULT_HDR_POS_MODE_OFFSET  1
	#define TPR_PROGRAM_RESULT_HDR_POS_MODE_LEN     1
	#define TPR_PROGRAM_RESULT_HDR_POS_MODE_DEFAULT 0x0
	/*[field] NEXT_HDR_TYPE*/
	#define TPR_PROGRAM_RESULT_NEXT_HDR_TYPE
	#define TPR_PROGRAM_RESULT_NEXT_HDR_TYPE_OFFSET  2
	#define TPR_PROGRAM_RESULT_NEXT_HDR_TYPE_LEN     2
	#define TPR_PROGRAM_RESULT_NEXT_HDR_TYPE_DEFAULT 0x0
	/*[field] HDR_LEN*/
	#define TPR_PROGRAM_RESULT_HDR_LEN
	#define TPR_PROGRAM_RESULT_HDR_LEN_OFFSET  8
	#define TPR_PROGRAM_RESULT_HDR_LEN_LEN     6
	#define TPR_PROGRAM_RESULT_HDR_LEN_DEFAULT 0x0
	/*[field] LEN_UNIT*/
	#define TPR_PROGRAM_RESULT_LEN_UNIT
	#define TPR_PROGRAM_RESULT_LEN_UNIT_OFFSET  14
	#define TPR_PROGRAM_RESULT_LEN_UNIT_LEN     2
	#define TPR_PROGRAM_RESULT_LEN_UNIT_DEFAULT 0x0
	/*[field] LEN_MASK*/
	#define TPR_PROGRAM_RESULT_LEN_MASK
	#define TPR_PROGRAM_RESULT_LEN_MASK_OFFSET  16
	#define TPR_PROGRAM_RESULT_LEN_MASK_LEN     16
	#define TPR_PROGRAM_RESULT_LEN_MASK_DEFAULT 0x0

struct tpr_program_result {
	a_uint32_t  len_mask:16;
	a_uint32_t  len_unit:2;
	a_uint32_t  hdr_len:6;
	a_uint32_t  _reserved0:4;
	a_uint32_t  next_hdr_type:2;
	a_uint32_t  hdr_pos_mode:1;
	a_uint32_t  next_hdr_mode:1;
};

union tpr_program_result_u {
	a_uint32_t val;
	struct tpr_program_result bf;
};

/*[register] TPR_PROGRAM_UDF_CTRL*/
#define TPR_PROGRAM_UDF_CTRL
#define TPR_PROGRAM_UDF_CTRL_ADDRESS 0x540
#define TPR_PROGRAM_UDF_CTRL_NUM     6
#define TPR_PROGRAM_UDF_CTRL_INC     0x4
#define TPR_PROGRAM_UDF_CTRL_TYPE    REG_TYPE_RW
#define TPR_PROGRAM_UDF_CTRL_DEFAULT 0x0
	/*[field] UDF0_OFFSET*/
	#define TPR_PROGRAM_UDF_CTRL_UDF0_OFFSET
	#define TPR_PROGRAM_UDF_CTRL_UDF0_OFFSET_OFFSET  0
	#define TPR_PROGRAM_UDF_CTRL_UDF0_OFFSET_LEN     6
	#define TPR_PROGRAM_UDF_CTRL_UDF0_OFFSET_DEFAULT 0x0
	/*[field] UDF1_OFFSET*/
	#define TPR_PROGRAM_UDF_CTRL_UDF1_OFFSET
	#define TPR_PROGRAM_UDF_CTRL_UDF1_OFFSET_OFFSET  8
	#define TPR_PROGRAM_UDF_CTRL_UDF1_OFFSET_LEN     6
	#define TPR_PROGRAM_UDF_CTRL_UDF1_OFFSET_DEFAULT 0x0
	/*[field] UDF2_OFFSET*/
	#define TPR_PROGRAM_UDF_CTRL_UDF2_OFFSET
	#define TPR_PROGRAM_UDF_CTRL_UDF2_OFFSET_OFFSET  16
	#define TPR_PROGRAM_UDF_CTRL_UDF2_OFFSET_LEN     6
	#define TPR_PROGRAM_UDF_CTRL_UDF2_OFFSET_DEFAULT 0x0

struct tpr_program_udf_ctrl {
	a_uint32_t  _reserved2:10;
	a_uint32_t  udf2_offset:6;
	a_uint32_t  _reserved1:2;
	a_uint32_t  udf1_offset:6;
	a_uint32_t  _reserved0:2;
	a_uint32_t  udf0_offset:6;
};

union tpr_program_udf_ctrl_u {
	a_uint32_t val;
	struct tpr_program_udf_ctrl bf;
};

/*[register] TPR_PROGRAM_UDF_DATA_0*/
#define TPR_PROGRAM_UDF_DATA_0
#define TPR_PROGRAM_UDF_DATA_0_ADDRESS 0x560
#define TPR_PROGRAM_UDF_DATA_0_NUM     16
#define TPR_PROGRAM_UDF_DATA_0_INC     0x4
#define TPR_PROGRAM_UDF_DATA_0_TYPE    REG_TYPE_RW
#define TPR_PROGRAM_UDF_DATA_0_DEFAULT 0x0
	/*[field] DATA0*/
	#define TPR_PROGRAM_UDF_DATA_0_DATA0
	#define TPR_PROGRAM_UDF_DATA_0_DATA0_OFFSET  0
	#define TPR_PROGRAM_UDF_DATA_0_DATA0_LEN     16
	#define TPR_PROGRAM_UDF_DATA_0_DATA0_DEFAULT 0x0
	/*[field] DATA1*/
	#define TPR_PROGRAM_UDF_DATA_0_DATA1
	#define TPR_PROGRAM_UDF_DATA_0_DATA1_OFFSET  16
	#define TPR_PROGRAM_UDF_DATA_0_DATA1_LEN     16
	#define TPR_PROGRAM_UDF_DATA_0_DATA1_DEFAULT 0x0

struct tpr_program_udf_data_0 {
	a_uint32_t  data1:16;
	a_uint32_t  data0:16;
};

union tpr_program_udf_data_0_u {
	a_uint32_t val;
	struct tpr_program_udf_data_0 bf;
};

/*[register] TPR_PROGRAM_UDF_DATA_1*/
#define TPR_PROGRAM_UDF_DATA_1
#define TPR_PROGRAM_UDF_DATA_1_ADDRESS 0x5a0
#define TPR_PROGRAM_UDF_DATA_1_NUM     16
#define TPR_PROGRAM_UDF_DATA_1_INC     0x4
#define TPR_PROGRAM_UDF_DATA_1_TYPE    REG_TYPE_RW
#define TPR_PROGRAM_UDF_DATA_1_DEFAULT 0x0
	/*[field] DATA2*/
	#define TPR_PROGRAM_UDF_DATA_1_DATA2
	#define TPR_PROGRAM_UDF_DATA_1_DATA2_OFFSET  0
	#define TPR_PROGRAM_UDF_DATA_1_DATA2_LEN     16
	#define TPR_PROGRAM_UDF_DATA_1_DATA2_DEFAULT 0x0
	/*[field] UDF0_VALID*/
	#define TPR_PROGRAM_UDF_DATA_1_UDF0_VALID
	#define TPR_PROGRAM_UDF_DATA_1_UDF0_VALID_OFFSET  16
	#define TPR_PROGRAM_UDF_DATA_1_UDF0_VALID_LEN     1
	#define TPR_PROGRAM_UDF_DATA_1_UDF0_VALID_DEFAULT 0x0
	/*[field] UDF1_VALID*/
	#define TPR_PROGRAM_UDF_DATA_1_UDF1_VALID
	#define TPR_PROGRAM_UDF_DATA_1_UDF1_VALID_OFFSET  17
	#define TPR_PROGRAM_UDF_DATA_1_UDF1_VALID_LEN     1
	#define TPR_PROGRAM_UDF_DATA_1_UDF1_VALID_DEFAULT 0x0
	/*[field] UDF2_VALID*/
	#define TPR_PROGRAM_UDF_DATA_1_UDF2_VALID
	#define TPR_PROGRAM_UDF_DATA_1_UDF2_VALID_OFFSET  18
	#define TPR_PROGRAM_UDF_DATA_1_UDF2_VALID_LEN     1
	#define TPR_PROGRAM_UDF_DATA_1_UDF2_VALID_DEFAULT 0x0
	/*[field] COMP_MODE*/
	#define TPR_PROGRAM_UDF_DATA_1_COMP_MODE
	#define TPR_PROGRAM_UDF_DATA_1_COMP_MODE_OFFSET  19
	#define TPR_PROGRAM_UDF_DATA_1_COMP_MODE_LEN     1
	#define TPR_PROGRAM_UDF_DATA_1_COMP_MODE_DEFAULT 0x0
	/*[field] PROGRAM_ID*/
	#define TPR_PROGRAM_UDF_DATA_1_PROGRAM_ID
	#define TPR_PROGRAM_UDF_DATA_1_PROGRAM_ID_OFFSET  20
	#define TPR_PROGRAM_UDF_DATA_1_PROGRAM_ID_LEN     3
	#define TPR_PROGRAM_UDF_DATA_1_PROGRAM_ID_DEFAULT 0x0

struct tpr_program_udf_data_1 {
	a_uint32_t  _reserved0:9;
	a_uint32_t  program_id:3;
	a_uint32_t  comp_mode:1;
	a_uint32_t  udf2_valid:1;
	a_uint32_t  udf1_valid:1;
	a_uint32_t  udf0_valid:1;
	a_uint32_t  data2:16;
};

union tpr_program_udf_data_1_u {
	a_uint32_t val;
	struct tpr_program_udf_data_1 bf;
};

/*[register] TPR_PROGRAM_UDF_MASK_0*/
#define TPR_PROGRAM_UDF_MASK_0
#define TPR_PROGRAM_UDF_MASK_0_ADDRESS 0x5e0
#define TPR_PROGRAM_UDF_MASK_0_NUM     16
#define TPR_PROGRAM_UDF_MASK_0_INC     0x4
#define TPR_PROGRAM_UDF_MASK_0_TYPE    REG_TYPE_RW
#define TPR_PROGRAM_UDF_MASK_0_DEFAULT 0x0
	/*[field] MASK0*/
	#define TPR_PROGRAM_UDF_MASK_0_MASK0
	#define TPR_PROGRAM_UDF_MASK_0_MASK0_OFFSET  0
	#define TPR_PROGRAM_UDF_MASK_0_MASK0_LEN     16
	#define TPR_PROGRAM_UDF_MASK_0_MASK0_DEFAULT 0x0
	/*[field] MASK1*/
	#define TPR_PROGRAM_UDF_MASK_0_MASK1
	#define TPR_PROGRAM_UDF_MASK_0_MASK1_OFFSET  16
	#define TPR_PROGRAM_UDF_MASK_0_MASK1_LEN     16
	#define TPR_PROGRAM_UDF_MASK_0_MASK1_DEFAULT 0x0

struct tpr_program_udf_mask_0 {
	a_uint32_t  mask1:16;
	a_uint32_t  mask0:16;
};

union tpr_program_udf_mask_0_u {
	a_uint32_t val;
	struct tpr_program_udf_mask_0 bf;
};

/*[register] TPR_PROGRAM_UDF_MASK_1*/
#define TPR_PROGRAM_UDF_MASK_1
#define TPR_PROGRAM_UDF_MASK_1_ADDRESS 0x620
#define TPR_PROGRAM_UDF_MASK_1_NUM     16
#define TPR_PROGRAM_UDF_MASK_1_INC     0x4
#define TPR_PROGRAM_UDF_MASK_1_TYPE    REG_TYPE_RW
#define TPR_PROGRAM_UDF_MASK_1_DEFAULT 0x0
	/*[field] MASK2*/
	#define TPR_PROGRAM_UDF_MASK_1_MASK2
	#define TPR_PROGRAM_UDF_MASK_1_MASK2_OFFSET  0
	#define TPR_PROGRAM_UDF_MASK_1_MASK2_LEN     16
	#define TPR_PROGRAM_UDF_MASK_1_MASK2_DEFAULT 0x0
	/*[field] UDF0_VALID_MASK*/
	#define TPR_PROGRAM_UDF_MASK_1_UDF0_VALID_MASK
	#define TPR_PROGRAM_UDF_MASK_1_UDF0_VALID_MASK_OFFSET  16
	#define TPR_PROGRAM_UDF_MASK_1_UDF0_VALID_MASK_LEN     1
	#define TPR_PROGRAM_UDF_MASK_1_UDF0_VALID_MASK_DEFAULT 0x0
	/*[field] UDF1_VALID_MASK*/
	#define TPR_PROGRAM_UDF_MASK_1_UDF1_VALID_MASK
	#define TPR_PROGRAM_UDF_MASK_1_UDF1_VALID_MASK_OFFSET  17
	#define TPR_PROGRAM_UDF_MASK_1_UDF1_VALID_MASK_LEN     1
	#define TPR_PROGRAM_UDF_MASK_1_UDF1_VALID_MASK_DEFAULT 0x0
	/*[field] UDF2_VALID_MASK*/
	#define TPR_PROGRAM_UDF_MASK_1_UDF2_VALID_MASK
	#define TPR_PROGRAM_UDF_MASK_1_UDF2_VALID_MASK_OFFSET  18
	#define TPR_PROGRAM_UDF_MASK_1_UDF2_VALID_MASK_LEN     1
	#define TPR_PROGRAM_UDF_MASK_1_UDF2_VALID_MASK_DEFAULT 0x0

struct tpr_program_udf_mask_1 {
	a_uint32_t  _reserved0:13;
	a_uint32_t  udf2_valid_mask:1;
	a_uint32_t  udf1_valid_mask:1;
	a_uint32_t  udf0_valid_mask:1;
	a_uint32_t  mask2:16;
};

union tpr_program_udf_mask_1_u {
	a_uint32_t val;
	struct tpr_program_udf_mask_1 bf;
};

/*[register] TPR_PROGRAM_UDF_ACTION*/
#define TPR_PROGRAM_UDF_ACTION
#define TPR_PROGRAM_UDF_ACTION_ADDRESS 0x660
#define TPR_PROGRAM_UDF_ACTION_NUM     16
#define TPR_PROGRAM_UDF_ACTION_INC     0x4
#define TPR_PROGRAM_UDF_ACTION_TYPE    REG_TYPE_RW
#define TPR_PROGRAM_UDF_ACTION_DEFAULT 0x0
	/*[field] NEXT_HDR_TYPE_VALID*/
	#define TPR_PROGRAM_UDF_ACTION_NEXT_HDR_TYPE_VALID
	#define TPR_PROGRAM_UDF_ACTION_NEXT_HDR_TYPE_VALID_OFFSET  0
	#define TPR_PROGRAM_UDF_ACTION_NEXT_HDR_TYPE_VALID_LEN     1
	#define TPR_PROGRAM_UDF_ACTION_NEXT_HDR_TYPE_VALID_DEFAULT 0x0
	/*[field] HDR_LEN_VALID*/
	#define TPR_PROGRAM_UDF_ACTION_HDR_LEN_VALID
	#define TPR_PROGRAM_UDF_ACTION_HDR_LEN_VALID_OFFSET  1
	#define TPR_PROGRAM_UDF_ACTION_HDR_LEN_VALID_LEN     1
	#define TPR_PROGRAM_UDF_ACTION_HDR_LEN_VALID_DEFAULT 0x0
	/*[field] NEXT_HDR_TYPE*/
	#define TPR_PROGRAM_UDF_ACTION_NEXT_HDR_TYPE
	#define TPR_PROGRAM_UDF_ACTION_NEXT_HDR_TYPE_OFFSET  2
	#define TPR_PROGRAM_UDF_ACTION_NEXT_HDR_TYPE_LEN     2
	#define TPR_PROGRAM_UDF_ACTION_NEXT_HDR_TYPE_DEFAULT 0x0
	/*[field] HDR_LEN*/
	#define TPR_PROGRAM_UDF_ACTION_HDR_LEN
	#define TPR_PROGRAM_UDF_ACTION_HDR_LEN_OFFSET  8
	#define TPR_PROGRAM_UDF_ACTION_HDR_LEN_LEN     4
	#define TPR_PROGRAM_UDF_ACTION_HDR_LEN_DEFAULT 0x0
	/*[field] EXCEPTION_EN*/
	#define TPR_PROGRAM_UDF_ACTION_EXCEPTION_EN
	#define TPR_PROGRAM_UDF_ACTION_EXCEPTION_EN_OFFSET  16
	#define TPR_PROGRAM_UDF_ACTION_EXCEPTION_EN_LEN     1
	#define TPR_PROGRAM_UDF_ACTION_EXCEPTION_EN_DEFAULT 0x0

struct tpr_program_udf_action {
	a_uint32_t  _reserved2:15;
	a_uint32_t  exception_en:1;
	a_uint32_t  _reserved1:4;
	a_uint32_t  hdr_len:4;
	a_uint32_t  _reserved0:4;
	a_uint32_t  next_hdr_type:2;
	a_uint32_t  hdr_len_valid:1;
	a_uint32_t  next_hdr_type_valid:1;
};

union tpr_program_udf_action_u {
	a_uint32_t val;
	struct tpr_program_udf_action bf;
};

#endif
