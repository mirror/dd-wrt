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
 * @defgroup
 * @{
 */
#ifndef APPE_TUNNEL_REG_H
#define APPE_TUNNEL_REG_H

/*[register] TPR_UDF_CTRL_0*/
#define TPR_UDF_CTRL_0
#define TPR_UDF_CTRL_0_ADDRESS 0x400
#define TPR_UDF_CTRL_0_NUM     16
#define TPR_UDF_CTRL_0_INC     0x4
#define TPR_UDF_CTRL_0_TYPE    REG_TYPE_RW
#define TPR_UDF_CTRL_0_DEFAULT 0x0
	/*[field] L3_TYPE*/
	#define TPR_UDF_CTRL_0_L3_TYPE
	#define TPR_UDF_CTRL_0_L3_TYPE_OFFSET  0
	#define TPR_UDF_CTRL_0_L3_TYPE_LEN     2
	#define TPR_UDF_CTRL_0_L3_TYPE_DEFAULT 0x0
	/*[field] L3_TYPE_INCL*/
	#define TPR_UDF_CTRL_0_L3_TYPE_INCL
	#define TPR_UDF_CTRL_0_L3_TYPE_INCL_OFFSET  2
	#define TPR_UDF_CTRL_0_L3_TYPE_INCL_LEN     1
	#define TPR_UDF_CTRL_0_L3_TYPE_INCL_DEFAULT 0x0
	/*[field] L4_TYPE*/
	#define TPR_UDF_CTRL_0_L4_TYPE
	#define TPR_UDF_CTRL_0_L4_TYPE_OFFSET  4
	#define TPR_UDF_CTRL_0_L4_TYPE_LEN     3
	#define TPR_UDF_CTRL_0_L4_TYPE_DEFAULT 0x0
	/*[field] L4_TYPE_INCL*/
	#define TPR_UDF_CTRL_0_L4_TYPE_INCL
	#define TPR_UDF_CTRL_0_L4_TYPE_INCL_OFFSET  7
	#define TPR_UDF_CTRL_0_L4_TYPE_INCL_LEN     1
	#define TPR_UDF_CTRL_0_L4_TYPE_INCL_DEFAULT 0x0
	/*[field] OVERLAY_TYPE*/
	#define TPR_UDF_CTRL_0_OVERLAY_TYPE
	#define TPR_UDF_CTRL_0_OVERLAY_TYPE_OFFSET  8
	#define TPR_UDF_CTRL_0_OVERLAY_TYPE_LEN     2
	#define TPR_UDF_CTRL_0_OVERLAY_TYPE_DEFAULT 0x0
	/*[field] OVERLAY_TYPE_INCL*/
	#define TPR_UDF_CTRL_0_OVERLAY_TYPE_INCL
	#define TPR_UDF_CTRL_0_OVERLAY_TYPE_INCL_OFFSET  10
	#define TPR_UDF_CTRL_0_OVERLAY_TYPE_INCL_LEN     1
	#define TPR_UDF_CTRL_0_OVERLAY_TYPE_INCL_DEFAULT 0x0
	/*[field] PROGRAM_TYPE*/
	#define TPR_UDF_CTRL_0_PROGRAM_TYPE
	#define TPR_UDF_CTRL_0_PROGRAM_TYPE_OFFSET  12
	#define TPR_UDF_CTRL_0_PROGRAM_TYPE_LEN     3
	#define TPR_UDF_CTRL_0_PROGRAM_TYPE_DEFAULT 0x0
	/*[field] PROGRAM_TYPE_INCL*/
	#define TPR_UDF_CTRL_0_PROGRAM_TYPE_INCL
	#define TPR_UDF_CTRL_0_PROGRAM_TYPE_INCL_OFFSET  15
	#define TPR_UDF_CTRL_0_PROGRAM_TYPE_INCL_LEN     1
	#define TPR_UDF_CTRL_0_PROGRAM_TYPE_INCL_DEFAULT 0x0
	/*[field] UDF_PROFILE*/
	#define TPR_UDF_CTRL_0_UDF_PROFILE
	#define TPR_UDF_CTRL_0_UDF_PROFILE_OFFSET  17
	#define TPR_UDF_CTRL_0_UDF_PROFILE_LEN     3
	#define TPR_UDF_CTRL_0_UDF_PROFILE_DEFAULT 0x0
	/*[field] VALID*/
	#define TPR_UDF_CTRL_0_VALID
	#define TPR_UDF_CTRL_0_VALID_OFFSET  31
	#define TPR_UDF_CTRL_0_VALID_LEN     1
	#define TPR_UDF_CTRL_0_VALID_DEFAULT 0x0

struct tpr_udf_ctrl_0 {
	a_uint32_t  valid:1;
	a_uint32_t  _reserved3:11;
	a_uint32_t  udf_profile:3;
	a_uint32_t  _reserved2:1;
	a_uint32_t  program_type_incl:1;
	a_uint32_t  program_type:3;
	a_uint32_t  _reserved1:1;
	a_uint32_t  overlay_type_incl:1;
	a_uint32_t  overlay_type:2;
	a_uint32_t  l4_type_incl:1;
	a_uint32_t  l4_type:3;
	a_uint32_t  _reserved0:1;
	a_uint32_t  l3_type_incl:1;
	a_uint32_t  l3_type:2;
};

union tpr_udf_ctrl_0_u {
	a_uint32_t val;
	struct tpr_udf_ctrl_0 bf;
};

/*[register] TPR_UDF_PROFILE_BASE*/
#define TPR_UDF_PROFILE_BASE
#define TPR_UDF_PROFILE_BASE_ADDRESS 0x440
#define TPR_UDF_PROFILE_BASE_NUM     8
#define TPR_UDF_PROFILE_BASE_INC     0x4
#define TPR_UDF_PROFILE_BASE_TYPE    REG_TYPE_RW
#define TPR_UDF_PROFILE_BASE_DEFAULT 0x0
	/*[field] UDF0_BASE*/
	#define TPR_UDF_PROFILE_BASE_UDF0_BASE
	#define TPR_UDF_PROFILE_BASE_UDF0_BASE_OFFSET  0
	#define TPR_UDF_PROFILE_BASE_UDF0_BASE_LEN     3
	#define TPR_UDF_PROFILE_BASE_UDF0_BASE_DEFAULT 0x0
	/*[field] UDF1_BASE*/
	#define TPR_UDF_PROFILE_BASE_UDF1_BASE
	#define TPR_UDF_PROFILE_BASE_UDF1_BASE_OFFSET  8
	#define TPR_UDF_PROFILE_BASE_UDF1_BASE_LEN     3
	#define TPR_UDF_PROFILE_BASE_UDF1_BASE_DEFAULT 0x0
	/*[field] UDF2_BASE*/
	#define TPR_UDF_PROFILE_BASE_UDF2_BASE
	#define TPR_UDF_PROFILE_BASE_UDF2_BASE_OFFSET  16
	#define TPR_UDF_PROFILE_BASE_UDF2_BASE_LEN     3
	#define TPR_UDF_PROFILE_BASE_UDF2_BASE_DEFAULT 0x0
	/*[field] UDF3_BASE*/
	#define TPR_UDF_PROFILE_BASE_UDF3_BASE
	#define TPR_UDF_PROFILE_BASE_UDF3_BASE_OFFSET  24
	#define TPR_UDF_PROFILE_BASE_UDF3_BASE_LEN     3
	#define TPR_UDF_PROFILE_BASE_UDF3_BASE_DEFAULT 0x0

struct tpr_udf_profile_base {
	a_uint32_t  _reserved3:5;
	a_uint32_t  udf3_base:3;
	a_uint32_t  _reserved2:5;
	a_uint32_t  udf2_base:3;
	a_uint32_t  _reserved1:5;
	a_uint32_t  udf1_base:3;
	a_uint32_t  _reserved0:5;
	a_uint32_t  udf0_base:3;
};

union tpr_udf_profile_base_u {
	a_uint32_t val;
	struct tpr_udf_profile_base bf;
};

/*[register] TPR_UDF_PROFILE_OFFSET*/
#define TPR_UDF_PROFILE_OFFSET
#define TPR_UDF_PROFILE_OFFSET_ADDRESS 0x460
#define TPR_UDF_PROFILE_OFFSET_NUM     8
#define TPR_UDF_PROFILE_OFFSET_INC     0x4
#define TPR_UDF_PROFILE_OFFSET_TYPE    REG_TYPE_RW
#define TPR_UDF_PROFILE_OFFSET_DEFAULT 0x0
	/*[field] UDF0_OFFSET*/
	#define TPR_UDF_PROFILE_OFFSET_UDF0_OFFSET
	#define TPR_UDF_PROFILE_OFFSET_UDF0_OFFSET_OFFSET  0
	#define TPR_UDF_PROFILE_OFFSET_UDF0_OFFSET_LEN     6
	#define TPR_UDF_PROFILE_OFFSET_UDF0_OFFSET_DEFAULT 0x0
	/*[field] UDF1_OFFSET*/
	#define TPR_UDF_PROFILE_OFFSET_UDF1_OFFSET
	#define TPR_UDF_PROFILE_OFFSET_UDF1_OFFSET_OFFSET  8
	#define TPR_UDF_PROFILE_OFFSET_UDF1_OFFSET_LEN     6
	#define TPR_UDF_PROFILE_OFFSET_UDF1_OFFSET_DEFAULT 0x0
	/*[field] UDF2_OFFSET*/
	#define TPR_UDF_PROFILE_OFFSET_UDF2_OFFSET
	#define TPR_UDF_PROFILE_OFFSET_UDF2_OFFSET_OFFSET  16
	#define TPR_UDF_PROFILE_OFFSET_UDF2_OFFSET_LEN     6
	#define TPR_UDF_PROFILE_OFFSET_UDF2_OFFSET_DEFAULT 0x0
	/*[field] UDF3_OFFSET*/
	#define TPR_UDF_PROFILE_OFFSET_UDF3_OFFSET
	#define TPR_UDF_PROFILE_OFFSET_UDF3_OFFSET_OFFSET  24
	#define TPR_UDF_PROFILE_OFFSET_UDF3_OFFSET_LEN     6
	#define TPR_UDF_PROFILE_OFFSET_UDF3_OFFSET_DEFAULT 0x0

struct tpr_udf_profile_offset {
	a_uint32_t  _reserved3:2;
	a_uint32_t  udf3_offset:6;
	a_uint32_t  _reserved2:2;
	a_uint32_t  udf2_offset:6;
	a_uint32_t  _reserved1:2;
	a_uint32_t  udf1_offset:6;
	a_uint32_t  _reserved0:2;
	a_uint32_t  udf0_offset:6;
};

union tpr_udf_profile_offset_u {
	a_uint32_t val;
	struct tpr_udf_profile_offset bf;
};

/*[register] TL_TBL_OP*/
#define TL_TBL_OP
#define TL_TBL_OP_ADDRESS 0x0
#define TL_TBL_OP_NUM     1
#define TL_TBL_OP_INC     0x4
#define TL_TBL_OP_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DEFAULT 0x0
	/*[field] OP_TYPE*/
	#define TL_TBL_OP_OP_TYPE
	#define TL_TBL_OP_OP_TYPE_OFFSET  0
	#define TL_TBL_OP_OP_TYPE_LEN     3
	#define TL_TBL_OP_OP_TYPE_DEFAULT 0x0
	/*[field] HASH_BLOCK_BITMAP*/
	#define TL_TBL_OP_HASH_BLOCK_BITMAP
	#define TL_TBL_OP_HASH_BLOCK_BITMAP_OFFSET  3
	#define TL_TBL_OP_HASH_BLOCK_BITMAP_LEN     2
	#define TL_TBL_OP_HASH_BLOCK_BITMAP_DEFAULT 0x0
	/*[field] OP_MODE*/
	#define TL_TBL_OP_OP_MODE
	#define TL_TBL_OP_OP_MODE_OFFSET  5
	#define TL_TBL_OP_OP_MODE_LEN     1
	#define TL_TBL_OP_OP_MODE_DEFAULT 0x0
	/*[field] ENTRY_INDEX*/
	#define TL_TBL_OP_ENTRY_INDEX
	#define TL_TBL_OP_ENTRY_INDEX_OFFSET  6
	#define TL_TBL_OP_ENTRY_INDEX_LEN     7
	#define TL_TBL_OP_ENTRY_INDEX_DEFAULT 0x0
	/*[field] OP_RSLT*/
	#define TL_TBL_OP_OP_RSLT
	#define TL_TBL_OP_OP_RSLT_OFFSET  13
	#define TL_TBL_OP_OP_RSLT_LEN     1
	#define TL_TBL_OP_OP_RSLT_DEFAULT 0x0
	/*[field] BUSY*/
	#define TL_TBL_OP_BUSY
	#define TL_TBL_OP_BUSY_OFFSET  14
	#define TL_TBL_OP_BUSY_LEN     1
	#define TL_TBL_OP_BUSY_DEFAULT 0x0

struct tl_tbl_op {
	a_uint32_t  _reserved0:17;
	a_uint32_t  busy:1;
	a_uint32_t  op_rslt:1;
	a_uint32_t  entry_index:7;
	a_uint32_t  op_mode:1;
	a_uint32_t  hash_block_bitmap:2;
	a_uint32_t  op_type:3;
};

union tl_tbl_op_u {
	a_uint32_t val;
	struct tl_tbl_op bf;
};

/*[register] TL_TBL_OP_DATA0*/
#define TL_TBL_OP_DATA0
#define TL_TBL_OP_DATA0_ADDRESS 0x4
#define TL_TBL_OP_DATA0_NUM     1
#define TL_TBL_OP_DATA0_INC     0x4
#define TL_TBL_OP_DATA0_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA0_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA0_DATA
	#define TL_TBL_OP_DATA0_DATA_OFFSET  0
	#define TL_TBL_OP_DATA0_DATA_LEN     32
	#define TL_TBL_OP_DATA0_DATA_DEFAULT 0x0

struct tl_tbl_op_data0 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data0_u {
	a_uint32_t val;
	struct tl_tbl_op_data0 bf;
};

/*[register] TL_TBL_OP_DATA1*/
#define TL_TBL_OP_DATA1
#define TL_TBL_OP_DATA1_ADDRESS 0x8
#define TL_TBL_OP_DATA1_NUM     1
#define TL_TBL_OP_DATA1_INC     0x4
#define TL_TBL_OP_DATA1_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA1_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA1_DATA
	#define TL_TBL_OP_DATA1_DATA_OFFSET  0
	#define TL_TBL_OP_DATA1_DATA_LEN     32
	#define TL_TBL_OP_DATA1_DATA_DEFAULT 0x0

struct tl_tbl_op_data1 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data1_u {
	a_uint32_t val;
	struct tl_tbl_op_data1 bf;
};

/*[register] TL_TBL_OP_DATA2*/
#define TL_TBL_OP_DATA2
#define TL_TBL_OP_DATA2_ADDRESS 0xc
#define TL_TBL_OP_DATA2_NUM     1
#define TL_TBL_OP_DATA2_INC     0x4
#define TL_TBL_OP_DATA2_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA2_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA2_DATA
	#define TL_TBL_OP_DATA2_DATA_OFFSET  0
	#define TL_TBL_OP_DATA2_DATA_LEN     32
	#define TL_TBL_OP_DATA2_DATA_DEFAULT 0x0

struct tl_tbl_op_data2 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data2_u {
	a_uint32_t val;
	struct tl_tbl_op_data2 bf;
};

/*[register] TL_TBL_OP_DATA3*/
#define TL_TBL_OP_DATA3
#define TL_TBL_OP_DATA3_ADDRESS 0x10
#define TL_TBL_OP_DATA3_NUM     1
#define TL_TBL_OP_DATA3_INC     0x4
#define TL_TBL_OP_DATA3_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA3_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA3_DATA
	#define TL_TBL_OP_DATA3_DATA_OFFSET  0
	#define TL_TBL_OP_DATA3_DATA_LEN     32
	#define TL_TBL_OP_DATA3_DATA_DEFAULT 0x0

struct tl_tbl_op_data3 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data3_u {
	a_uint32_t val;
	struct tl_tbl_op_data3 bf;
};

/*[register] TL_TBL_OP_DATA4*/
#define TL_TBL_OP_DATA4
#define TL_TBL_OP_DATA4_ADDRESS 0x14
#define TL_TBL_OP_DATA4_NUM     1
#define TL_TBL_OP_DATA4_INC     0x4
#define TL_TBL_OP_DATA4_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA4_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA4_DATA
	#define TL_TBL_OP_DATA4_DATA_OFFSET  0
	#define TL_TBL_OP_DATA4_DATA_LEN     32
	#define TL_TBL_OP_DATA4_DATA_DEFAULT 0x0

struct tl_tbl_op_data4 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data4_u {
	a_uint32_t val;
	struct tl_tbl_op_data4 bf;
};

/*[register] TL_TBL_OP_DATA5*/
#define TL_TBL_OP_DATA5
#define TL_TBL_OP_DATA5_ADDRESS 0x18
#define TL_TBL_OP_DATA5_NUM     1
#define TL_TBL_OP_DATA5_INC     0x4
#define TL_TBL_OP_DATA5_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA5_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA5_DATA
	#define TL_TBL_OP_DATA5_DATA_OFFSET  0
	#define TL_TBL_OP_DATA5_DATA_LEN     32
	#define TL_TBL_OP_DATA5_DATA_DEFAULT 0x0

struct tl_tbl_op_data5 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data5_u {
	a_uint32_t val;
	struct tl_tbl_op_data5 bf;
};

/*[register] TL_TBL_OP_DATA6*/
#define TL_TBL_OP_DATA6
#define TL_TBL_OP_DATA6_ADDRESS 0x1c
#define TL_TBL_OP_DATA6_NUM     1
#define TL_TBL_OP_DATA6_INC     0x4
#define TL_TBL_OP_DATA6_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA6_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA6_DATA
	#define TL_TBL_OP_DATA6_DATA_OFFSET  0
	#define TL_TBL_OP_DATA6_DATA_LEN     32
	#define TL_TBL_OP_DATA6_DATA_DEFAULT 0x0

struct tl_tbl_op_data6 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data6_u {
	a_uint32_t val;
	struct tl_tbl_op_data6 bf;
};

/*[register] TL_TBL_OP_DATA7*/
#define TL_TBL_OP_DATA7
#define TL_TBL_OP_DATA7_ADDRESS 0x20
#define TL_TBL_OP_DATA7_NUM     1
#define TL_TBL_OP_DATA7_INC     0x4
#define TL_TBL_OP_DATA7_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA7_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA7_DATA
	#define TL_TBL_OP_DATA7_DATA_OFFSET  0
	#define TL_TBL_OP_DATA7_DATA_LEN     32
	#define TL_TBL_OP_DATA7_DATA_DEFAULT 0x0

struct tl_tbl_op_data7 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data7_u {
	a_uint32_t val;
	struct tl_tbl_op_data7 bf;
};

/*[register] TL_TBL_OP_DATA8*/
#define TL_TBL_OP_DATA8
#define TL_TBL_OP_DATA8_ADDRESS 0x24
#define TL_TBL_OP_DATA8_NUM     1
#define TL_TBL_OP_DATA8_INC     0x4
#define TL_TBL_OP_DATA8_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA8_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA8_DATA
	#define TL_TBL_OP_DATA8_DATA_OFFSET  0
	#define TL_TBL_OP_DATA8_DATA_LEN     32
	#define TL_TBL_OP_DATA8_DATA_DEFAULT 0x0

struct tl_tbl_op_data8 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data8_u {
	a_uint32_t val;
	struct tl_tbl_op_data8 bf;
};

/*[register] TL_TBL_OP_DATA9*/
#define TL_TBL_OP_DATA9
#define TL_TBL_OP_DATA9_ADDRESS 0x28
#define TL_TBL_OP_DATA9_NUM     1
#define TL_TBL_OP_DATA9_INC     0x4
#define TL_TBL_OP_DATA9_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA9_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA9_DATA
	#define TL_TBL_OP_DATA9_DATA_OFFSET  0
	#define TL_TBL_OP_DATA9_DATA_LEN     32
	#define TL_TBL_OP_DATA9_DATA_DEFAULT 0x0

struct tl_tbl_op_data9 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data9_u {
	a_uint32_t val;
	struct tl_tbl_op_data9 bf;
};

/*[register] TL_TBL_OP_DATA10*/
#define TL_TBL_OP_DATA10
#define TL_TBL_OP_DATA10_ADDRESS 0x2c
#define TL_TBL_OP_DATA10_NUM     1
#define TL_TBL_OP_DATA10_INC     0x4
#define TL_TBL_OP_DATA10_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA10_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA10_DATA
	#define TL_TBL_OP_DATA10_DATA_OFFSET  0
	#define TL_TBL_OP_DATA10_DATA_LEN     32
	#define TL_TBL_OP_DATA10_DATA_DEFAULT 0x0

struct tl_tbl_op_data10 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data10_u {
	a_uint32_t val;
	struct tl_tbl_op_data10 bf;
};

/*[register] TL_TBL_OP_DATA11*/
#define TL_TBL_OP_DATA11
#define TL_TBL_OP_DATA11_ADDRESS 0x30
#define TL_TBL_OP_DATA11_NUM     1
#define TL_TBL_OP_DATA11_INC     0x4
#define TL_TBL_OP_DATA11_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA11_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA11_DATA
	#define TL_TBL_OP_DATA11_DATA_OFFSET  0
	#define TL_TBL_OP_DATA11_DATA_LEN     32
	#define TL_TBL_OP_DATA11_DATA_DEFAULT 0x0

struct tl_tbl_op_data11 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data11_u {
	a_uint32_t val;
	struct tl_tbl_op_data11 bf;
};

/*[register] TL_TBL_OP_DATA12*/
#define TL_TBL_OP_DATA12
#define TL_TBL_OP_DATA12_ADDRESS 0x34
#define TL_TBL_OP_DATA12_NUM     1
#define TL_TBL_OP_DATA12_INC     0x4
#define TL_TBL_OP_DATA12_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA12_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA12_DATA
	#define TL_TBL_OP_DATA12_DATA_OFFSET  0
	#define TL_TBL_OP_DATA12_DATA_LEN     32
	#define TL_TBL_OP_DATA12_DATA_DEFAULT 0x0

struct tl_tbl_op_data12 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data12_u {
	a_uint32_t val;
	struct tl_tbl_op_data12 bf;
};

/*[register] TL_TBL_OP_DATA13*/
#define TL_TBL_OP_DATA13
#define TL_TBL_OP_DATA13_ADDRESS 0x38
#define TL_TBL_OP_DATA13_NUM     1
#define TL_TBL_OP_DATA13_INC     0x4
#define TL_TBL_OP_DATA13_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA13_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA13_DATA
	#define TL_TBL_OP_DATA13_DATA_OFFSET  0
	#define TL_TBL_OP_DATA13_DATA_LEN     32
	#define TL_TBL_OP_DATA13_DATA_DEFAULT 0x0

struct tl_tbl_op_data13 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data13_u {
	a_uint32_t val;
	struct tl_tbl_op_data13 bf;
};

/*[register] TL_TBL_OP_DATA14*/
#define TL_TBL_OP_DATA14
#define TL_TBL_OP_DATA14_ADDRESS 0x3c
#define TL_TBL_OP_DATA14_NUM     1
#define TL_TBL_OP_DATA14_INC     0x4
#define TL_TBL_OP_DATA14_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA14_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA14_DATA
	#define TL_TBL_OP_DATA14_DATA_OFFSET  0
	#define TL_TBL_OP_DATA14_DATA_LEN     32
	#define TL_TBL_OP_DATA14_DATA_DEFAULT 0x0

struct tl_tbl_op_data14 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data14_u {
	a_uint32_t val;
	struct tl_tbl_op_data14 bf;
};

/*[register] TL_TBL_OP_DATA15*/
#define TL_TBL_OP_DATA15
#define TL_TBL_OP_DATA15_ADDRESS 0x40
#define TL_TBL_OP_DATA15_NUM     1
#define TL_TBL_OP_DATA15_INC     0x4
#define TL_TBL_OP_DATA15_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA15_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA15_DATA
	#define TL_TBL_OP_DATA15_DATA_OFFSET  0
	#define TL_TBL_OP_DATA15_DATA_LEN     32
	#define TL_TBL_OP_DATA15_DATA_DEFAULT 0x0

struct tl_tbl_op_data15 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data15_u {
	a_uint32_t val;
	struct tl_tbl_op_data15 bf;
};

/*[register] TL_TBL_OP_DATA16*/
#define TL_TBL_OP_DATA16
#define TL_TBL_OP_DATA16_ADDRESS 0x44
#define TL_TBL_OP_DATA16_NUM     1
#define TL_TBL_OP_DATA16_INC     0x4
#define TL_TBL_OP_DATA16_TYPE    REG_TYPE_RW
#define TL_TBL_OP_DATA16_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_OP_DATA16_DATA
	#define TL_TBL_OP_DATA16_DATA_OFFSET  0
	#define TL_TBL_OP_DATA16_DATA_LEN     32
	#define TL_TBL_OP_DATA16_DATA_DEFAULT 0x0

struct tl_tbl_op_data16 {
	a_uint32_t  data:32;
};

union tl_tbl_op_data16_u {
	a_uint32_t val;
	struct tl_tbl_op_data16 bf;
};

/*[register] TL_TBL_RD_OP*/
#define TL_TBL_RD_OP
#define TL_TBL_RD_OP_ADDRESS 0x54
#define TL_TBL_RD_OP_NUM     1
#define TL_TBL_RD_OP_INC     0x4
#define TL_TBL_RD_OP_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DEFAULT 0x0
	/*[field] OP_TYPE*/
	#define TL_TBL_RD_OP_OP_TYPE
	#define TL_TBL_RD_OP_OP_TYPE_OFFSET  0
	#define TL_TBL_RD_OP_OP_TYPE_LEN     3
	#define TL_TBL_RD_OP_OP_TYPE_DEFAULT 0x0
	/*[field] HASH_BLOCK_BITMAP*/
	#define TL_TBL_RD_OP_HASH_BLOCK_BITMAP
	#define TL_TBL_RD_OP_HASH_BLOCK_BITMAP_OFFSET  3
	#define TL_TBL_RD_OP_HASH_BLOCK_BITMAP_LEN     2
	#define TL_TBL_RD_OP_HASH_BLOCK_BITMAP_DEFAULT 0x0
	/*[field] OP_MODE*/
	#define TL_TBL_RD_OP_OP_MODE
	#define TL_TBL_RD_OP_OP_MODE_OFFSET  5
	#define TL_TBL_RD_OP_OP_MODE_LEN     1
	#define TL_TBL_RD_OP_OP_MODE_DEFAULT 0x0
	/*[field] ENTRY_INDEX*/
	#define TL_TBL_RD_OP_ENTRY_INDEX
	#define TL_TBL_RD_OP_ENTRY_INDEX_OFFSET  6
	#define TL_TBL_RD_OP_ENTRY_INDEX_LEN     7
	#define TL_TBL_RD_OP_ENTRY_INDEX_DEFAULT 0x0
	/*[field] OP_RSLT*/
	#define TL_TBL_RD_OP_OP_RSLT
	#define TL_TBL_RD_OP_OP_RSLT_OFFSET  13
	#define TL_TBL_RD_OP_OP_RSLT_LEN     1
	#define TL_TBL_RD_OP_OP_RSLT_DEFAULT 0x0
	/*[field] BUSY*/
	#define TL_TBL_RD_OP_BUSY
	#define TL_TBL_RD_OP_BUSY_OFFSET  14
	#define TL_TBL_RD_OP_BUSY_LEN     1
	#define TL_TBL_RD_OP_BUSY_DEFAULT 0x0

struct tl_tbl_rd_op {
	a_uint32_t  _reserved0:17;
	a_uint32_t  busy:1;
	a_uint32_t  op_rslt:1;
	a_uint32_t  entry_index:7;
	a_uint32_t  op_mode:1;
	a_uint32_t  hash_block_bitmap:2;
	a_uint32_t  op_type:3;
};

union tl_tbl_rd_op_u {
	a_uint32_t val;
	struct tl_tbl_rd_op bf;
};

/*[register] TL_TBL_RD_OP_DATA0*/
#define TL_TBL_RD_OP_DATA0
#define TL_TBL_RD_OP_DATA0_ADDRESS 0x58
#define TL_TBL_RD_OP_DATA0_NUM     1
#define TL_TBL_RD_OP_DATA0_INC     0x4
#define TL_TBL_RD_OP_DATA0_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA0_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA0_DATA
	#define TL_TBL_RD_OP_DATA0_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA0_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA0_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data0 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data0_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data0 bf;
};

/*[register] TL_TBL_RD_OP_DATA1*/
#define TL_TBL_RD_OP_DATA1
#define TL_TBL_RD_OP_DATA1_ADDRESS 0x5c
#define TL_TBL_RD_OP_DATA1_NUM     1
#define TL_TBL_RD_OP_DATA1_INC     0x4
#define TL_TBL_RD_OP_DATA1_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA1_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA1_DATA
	#define TL_TBL_RD_OP_DATA1_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA1_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA1_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data1 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data1_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data1 bf;
};

/*[register] TL_TBL_RD_OP_DATA2*/
#define TL_TBL_RD_OP_DATA2
#define TL_TBL_RD_OP_DATA2_ADDRESS 0x60
#define TL_TBL_RD_OP_DATA2_NUM     1
#define TL_TBL_RD_OP_DATA2_INC     0x4
#define TL_TBL_RD_OP_DATA2_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA2_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA2_DATA
	#define TL_TBL_RD_OP_DATA2_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA2_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA2_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data2 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data2_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data2 bf;
};

/*[register] TL_TBL_RD_OP_DATA3*/
#define TL_TBL_RD_OP_DATA3
#define TL_TBL_RD_OP_DATA3_ADDRESS 0x64
#define TL_TBL_RD_OP_DATA3_NUM     1
#define TL_TBL_RD_OP_DATA3_INC     0x4
#define TL_TBL_RD_OP_DATA3_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA3_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA3_DATA
	#define TL_TBL_RD_OP_DATA3_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA3_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA3_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data3 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data3_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data3 bf;
};

/*[register] TL_TBL_RD_OP_DATA4*/
#define TL_TBL_RD_OP_DATA4
#define TL_TBL_RD_OP_DATA4_ADDRESS 0x68
#define TL_TBL_RD_OP_DATA4_NUM     1
#define TL_TBL_RD_OP_DATA4_INC     0x4
#define TL_TBL_RD_OP_DATA4_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA4_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA4_DATA
	#define TL_TBL_RD_OP_DATA4_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA4_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA4_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data4 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data4_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data4 bf;
};

/*[register] TL_TBL_RD_OP_DATA5*/
#define TL_TBL_RD_OP_DATA5
#define TL_TBL_RD_OP_DATA5_ADDRESS 0x6c
#define TL_TBL_RD_OP_DATA5_NUM     1
#define TL_TBL_RD_OP_DATA5_INC     0x4
#define TL_TBL_RD_OP_DATA5_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA5_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA5_DATA
	#define TL_TBL_RD_OP_DATA5_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA5_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA5_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data5 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data5_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data5 bf;
};

/*[register] TL_TBL_RD_OP_DATA6*/
#define TL_TBL_RD_OP_DATA6
#define TL_TBL_RD_OP_DATA6_ADDRESS 0x70
#define TL_TBL_RD_OP_DATA6_NUM     1
#define TL_TBL_RD_OP_DATA6_INC     0x4
#define TL_TBL_RD_OP_DATA6_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA6_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA6_DATA
	#define TL_TBL_RD_OP_DATA6_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA6_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA6_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data6 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data6_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data6 bf;
};

/*[register] TL_TBL_RD_OP_DATA7*/
#define TL_TBL_RD_OP_DATA7
#define TL_TBL_RD_OP_DATA7_ADDRESS 0x74
#define TL_TBL_RD_OP_DATA7_NUM     1
#define TL_TBL_RD_OP_DATA7_INC     0x4
#define TL_TBL_RD_OP_DATA7_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA7_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA7_DATA
	#define TL_TBL_RD_OP_DATA7_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA7_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA7_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data7 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data7_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data7 bf;
};

/*[register] TL_TBL_RD_OP_DATA8*/
#define TL_TBL_RD_OP_DATA8
#define TL_TBL_RD_OP_DATA8_ADDRESS 0x78
#define TL_TBL_RD_OP_DATA8_NUM     1
#define TL_TBL_RD_OP_DATA8_INC     0x4
#define TL_TBL_RD_OP_DATA8_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA8_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA8_DATA
	#define TL_TBL_RD_OP_DATA8_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA8_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA8_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data8 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data8_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data8 bf;
};

/*[register] TL_TBL_RD_OP_DATA9*/
#define TL_TBL_RD_OP_DATA9
#define TL_TBL_RD_OP_DATA9_ADDRESS 0x7c
#define TL_TBL_RD_OP_DATA9_NUM     1
#define TL_TBL_RD_OP_DATA9_INC     0x4
#define TL_TBL_RD_OP_DATA9_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA9_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA9_DATA
	#define TL_TBL_RD_OP_DATA9_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA9_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA9_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data9 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data9_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data9 bf;
};

/*[register] TL_TBL_RD_OP_DATA10*/
#define TL_TBL_RD_OP_DATA10
#define TL_TBL_RD_OP_DATA10_ADDRESS 0x80
#define TL_TBL_RD_OP_DATA10_NUM     1
#define TL_TBL_RD_OP_DATA10_INC     0x4
#define TL_TBL_RD_OP_DATA10_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA10_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA10_DATA
	#define TL_TBL_RD_OP_DATA10_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA10_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA10_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data10 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data10_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data10 bf;
};

/*[register] TL_TBL_RD_OP_DATA11*/
#define TL_TBL_RD_OP_DATA11
#define TL_TBL_RD_OP_DATA11_ADDRESS 0x84
#define TL_TBL_RD_OP_DATA11_NUM     1
#define TL_TBL_RD_OP_DATA11_INC     0x4
#define TL_TBL_RD_OP_DATA11_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA11_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA11_DATA
	#define TL_TBL_RD_OP_DATA11_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA11_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA11_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data11 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data11_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data11 bf;
};

/*[register] TL_TBL_RD_OP_DATA12*/
#define TL_TBL_RD_OP_DATA12
#define TL_TBL_RD_OP_DATA12_ADDRESS 0x88
#define TL_TBL_RD_OP_DATA12_NUM     1
#define TL_TBL_RD_OP_DATA12_INC     0x4
#define TL_TBL_RD_OP_DATA12_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA12_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA12_DATA
	#define TL_TBL_RD_OP_DATA12_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA12_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA12_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data12 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data12_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data12 bf;
};

/*[register] TL_TBL_RD_OP_DATA13*/
#define TL_TBL_RD_OP_DATA13
#define TL_TBL_RD_OP_DATA13_ADDRESS 0x8c
#define TL_TBL_RD_OP_DATA13_NUM     1
#define TL_TBL_RD_OP_DATA13_INC     0x4
#define TL_TBL_RD_OP_DATA13_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA13_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA13_DATA
	#define TL_TBL_RD_OP_DATA13_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA13_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA13_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data13 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data13_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data13 bf;
};

/*[register] TL_TBL_RD_OP_DATA14*/
#define TL_TBL_RD_OP_DATA14
#define TL_TBL_RD_OP_DATA14_ADDRESS 0x90
#define TL_TBL_RD_OP_DATA14_NUM     1
#define TL_TBL_RD_OP_DATA14_INC     0x4
#define TL_TBL_RD_OP_DATA14_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA14_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA14_DATA
	#define TL_TBL_RD_OP_DATA14_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA14_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA14_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data14 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data14_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data14 bf;
};

/*[register] TL_TBL_RD_OP_DATA15*/
#define TL_TBL_RD_OP_DATA15
#define TL_TBL_RD_OP_DATA15_ADDRESS 0x94
#define TL_TBL_RD_OP_DATA15_NUM     1
#define TL_TBL_RD_OP_DATA15_INC     0x4
#define TL_TBL_RD_OP_DATA15_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA15_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA15_DATA
	#define TL_TBL_RD_OP_DATA15_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA15_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA15_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data15 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data15_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data15 bf;
};

/*[register] TL_TBL_RD_OP_DATA16*/
#define TL_TBL_RD_OP_DATA16
#define TL_TBL_RD_OP_DATA16_ADDRESS 0x98
#define TL_TBL_RD_OP_DATA16_NUM     1
#define TL_TBL_RD_OP_DATA16_INC     0x4
#define TL_TBL_RD_OP_DATA16_TYPE    REG_TYPE_RW
#define TL_TBL_RD_OP_DATA16_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_OP_DATA16_DATA
	#define TL_TBL_RD_OP_DATA16_DATA_OFFSET  0
	#define TL_TBL_RD_OP_DATA16_DATA_LEN     32
	#define TL_TBL_RD_OP_DATA16_DATA_DEFAULT 0x0

struct tl_tbl_rd_op_data16 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_op_data16_u {
	a_uint32_t val;
	struct tl_tbl_rd_op_data16 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA0*/
#define TL_TBL_RD_RSLT_DATA0
#define TL_TBL_RD_RSLT_DATA0_ADDRESS 0xa0
#define TL_TBL_RD_RSLT_DATA0_NUM     1
#define TL_TBL_RD_RSLT_DATA0_INC     0x4
#define TL_TBL_RD_RSLT_DATA0_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA0_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA0_DATA
	#define TL_TBL_RD_RSLT_DATA0_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA0_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA0_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data0 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data0_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data0 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA1*/
#define TL_TBL_RD_RSLT_DATA1
#define TL_TBL_RD_RSLT_DATA1_ADDRESS 0xa4
#define TL_TBL_RD_RSLT_DATA1_NUM     1
#define TL_TBL_RD_RSLT_DATA1_INC     0x4
#define TL_TBL_RD_RSLT_DATA1_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA1_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA1_DATA
	#define TL_TBL_RD_RSLT_DATA1_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA1_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA1_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data1 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data1_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data1 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA2*/
#define TL_TBL_RD_RSLT_DATA2
#define TL_TBL_RD_RSLT_DATA2_ADDRESS 0xa8
#define TL_TBL_RD_RSLT_DATA2_NUM     1
#define TL_TBL_RD_RSLT_DATA2_INC     0x4
#define TL_TBL_RD_RSLT_DATA2_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA2_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA2_DATA
	#define TL_TBL_RD_RSLT_DATA2_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA2_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA2_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data2 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data2_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data2 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA3*/
#define TL_TBL_RD_RSLT_DATA3
#define TL_TBL_RD_RSLT_DATA3_ADDRESS 0xac
#define TL_TBL_RD_RSLT_DATA3_NUM     1
#define TL_TBL_RD_RSLT_DATA3_INC     0x4
#define TL_TBL_RD_RSLT_DATA3_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA3_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA3_DATA
	#define TL_TBL_RD_RSLT_DATA3_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA3_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA3_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data3 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data3_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data3 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA4*/
#define TL_TBL_RD_RSLT_DATA4
#define TL_TBL_RD_RSLT_DATA4_ADDRESS 0xb0
#define TL_TBL_RD_RSLT_DATA4_NUM     1
#define TL_TBL_RD_RSLT_DATA4_INC     0x4
#define TL_TBL_RD_RSLT_DATA4_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA4_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA4_DATA
	#define TL_TBL_RD_RSLT_DATA4_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA4_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA4_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data4 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data4_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data4 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA5*/
#define TL_TBL_RD_RSLT_DATA5
#define TL_TBL_RD_RSLT_DATA5_ADDRESS 0xb4
#define TL_TBL_RD_RSLT_DATA5_NUM     1
#define TL_TBL_RD_RSLT_DATA5_INC     0x4
#define TL_TBL_RD_RSLT_DATA5_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA5_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA5_DATA
	#define TL_TBL_RD_RSLT_DATA5_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA5_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA5_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data5 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data5_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data5 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA6*/
#define TL_TBL_RD_RSLT_DATA6
#define TL_TBL_RD_RSLT_DATA6_ADDRESS 0xb8
#define TL_TBL_RD_RSLT_DATA6_NUM     1
#define TL_TBL_RD_RSLT_DATA6_INC     0x4
#define TL_TBL_RD_RSLT_DATA6_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA6_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA6_DATA
	#define TL_TBL_RD_RSLT_DATA6_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA6_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA6_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data6 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data6_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data6 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA7*/
#define TL_TBL_RD_RSLT_DATA7
#define TL_TBL_RD_RSLT_DATA7_ADDRESS 0xbc
#define TL_TBL_RD_RSLT_DATA7_NUM     1
#define TL_TBL_RD_RSLT_DATA7_INC     0x4
#define TL_TBL_RD_RSLT_DATA7_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA7_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA7_DATA
	#define TL_TBL_RD_RSLT_DATA7_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA7_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA7_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data7 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data7_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data7 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA8*/
#define TL_TBL_RD_RSLT_DATA8
#define TL_TBL_RD_RSLT_DATA8_ADDRESS 0xc0
#define TL_TBL_RD_RSLT_DATA8_NUM     1
#define TL_TBL_RD_RSLT_DATA8_INC     0x4
#define TL_TBL_RD_RSLT_DATA8_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA8_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA8_DATA
	#define TL_TBL_RD_RSLT_DATA8_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA8_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA8_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data8 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data8_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data8 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA9*/
#define TL_TBL_RD_RSLT_DATA9
#define TL_TBL_RD_RSLT_DATA9_ADDRESS 0xc4
#define TL_TBL_RD_RSLT_DATA9_NUM     1
#define TL_TBL_RD_RSLT_DATA9_INC     0x4
#define TL_TBL_RD_RSLT_DATA9_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA9_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA9_DATA
	#define TL_TBL_RD_RSLT_DATA9_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA9_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA9_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data9 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data9_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data9 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA10*/
#define TL_TBL_RD_RSLT_DATA10
#define TL_TBL_RD_RSLT_DATA10_ADDRESS 0xc8
#define TL_TBL_RD_RSLT_DATA10_NUM     1
#define TL_TBL_RD_RSLT_DATA10_INC     0x4
#define TL_TBL_RD_RSLT_DATA10_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA10_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA10_DATA
	#define TL_TBL_RD_RSLT_DATA10_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA10_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA10_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data10 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data10_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data10 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA11*/
#define TL_TBL_RD_RSLT_DATA11
#define TL_TBL_RD_RSLT_DATA11_ADDRESS 0xcc
#define TL_TBL_RD_RSLT_DATA11_NUM     1
#define TL_TBL_RD_RSLT_DATA11_INC     0x4
#define TL_TBL_RD_RSLT_DATA11_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA11_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA11_DATA
	#define TL_TBL_RD_RSLT_DATA11_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA11_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA11_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data11 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data11_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data11 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA12*/
#define TL_TBL_RD_RSLT_DATA12
#define TL_TBL_RD_RSLT_DATA12_ADDRESS 0xd0
#define TL_TBL_RD_RSLT_DATA12_NUM     1
#define TL_TBL_RD_RSLT_DATA12_INC     0x4
#define TL_TBL_RD_RSLT_DATA12_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA12_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA12_DATA
	#define TL_TBL_RD_RSLT_DATA12_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA12_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA12_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data12 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data12_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data12 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA13*/
#define TL_TBL_RD_RSLT_DATA13
#define TL_TBL_RD_RSLT_DATA13_ADDRESS 0xd4
#define TL_TBL_RD_RSLT_DATA13_NUM     1
#define TL_TBL_RD_RSLT_DATA13_INC     0x4
#define TL_TBL_RD_RSLT_DATA13_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA13_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA13_DATA
	#define TL_TBL_RD_RSLT_DATA13_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA13_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA13_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data13 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data13_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data13 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA14*/
#define TL_TBL_RD_RSLT_DATA14
#define TL_TBL_RD_RSLT_DATA14_ADDRESS 0xd8
#define TL_TBL_RD_RSLT_DATA14_NUM     1
#define TL_TBL_RD_RSLT_DATA14_INC     0x4
#define TL_TBL_RD_RSLT_DATA14_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA14_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA14_DATA
	#define TL_TBL_RD_RSLT_DATA14_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA14_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA14_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data14 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data14_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data14 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA15*/
#define TL_TBL_RD_RSLT_DATA15
#define TL_TBL_RD_RSLT_DATA15_ADDRESS 0xdc
#define TL_TBL_RD_RSLT_DATA15_NUM     1
#define TL_TBL_RD_RSLT_DATA15_INC     0x4
#define TL_TBL_RD_RSLT_DATA15_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA15_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA15_DATA
	#define TL_TBL_RD_RSLT_DATA15_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA15_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA15_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data15 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data15_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data15 bf;
};

/*[register] TL_TBL_RD_RSLT_DATA16*/
#define TL_TBL_RD_RSLT_DATA16
#define TL_TBL_RD_RSLT_DATA16_ADDRESS 0xe0
#define TL_TBL_RD_RSLT_DATA16_NUM     1
#define TL_TBL_RD_RSLT_DATA16_INC     0x4
#define TL_TBL_RD_RSLT_DATA16_TYPE    REG_TYPE_RO
#define TL_TBL_RD_RSLT_DATA16_DEFAULT 0x0
	/*[field] DATA*/
	#define TL_TBL_RD_RSLT_DATA16_DATA
	#define TL_TBL_RD_RSLT_DATA16_DATA_OFFSET  0
	#define TL_TBL_RD_RSLT_DATA16_DATA_LEN     32
	#define TL_TBL_RD_RSLT_DATA16_DATA_DEFAULT 0x0

struct tl_tbl_rd_rslt_data16 {
	a_uint32_t  data:32;
};

union tl_tbl_rd_rslt_data16_u {
	a_uint32_t val;
	struct tl_tbl_rd_rslt_data16 bf;
};

/*[register] TL_CTRL*/
#define TL_CTRL
#define TL_CTRL_ADDRESS 0xe4
#define TL_CTRL_NUM     1
#define TL_CTRL_INC     0x4
#define TL_CTRL_TYPE    REG_TYPE_RW
#define TL_CTRL_DEFAULT 0xff11b03
	/*[field] TL_DE_ACCE_CMD*/
	#define TL_CTRL_TL_DE_ACCE_CMD
	#define TL_CTRL_TL_DE_ACCE_CMD_OFFSET  0
	#define TL_CTRL_TL_DE_ACCE_CMD_LEN     2
	#define TL_CTRL_TL_DE_ACCE_CMD_DEFAULT 0x3
	/*[field] TL_SRC_IF_CHECK_CMD*/
	#define TL_CTRL_TL_SRC_IF_CHECK_CMD
	#define TL_CTRL_TL_SRC_IF_CHECK_CMD_OFFSET  2
	#define TL_CTRL_TL_SRC_IF_CHECK_CMD_LEN     2
	#define TL_CTRL_TL_SRC_IF_CHECK_CMD_DEFAULT 0x0
	/*[field] TL_SRC_IF_CHECK_DE_ACCE*/
	#define TL_CTRL_TL_SRC_IF_CHECK_DE_ACCE
	#define TL_CTRL_TL_SRC_IF_CHECK_DE_ACCE_OFFSET  4
	#define TL_CTRL_TL_SRC_IF_CHECK_DE_ACCE_LEN     1
	#define TL_CTRL_TL_SRC_IF_CHECK_DE_ACCE_DEFAULT 0x0
	/*[field] TL_VLAN_CHECK_CMD*/
	#define TL_CTRL_TL_VLAN_CHECK_CMD
	#define TL_CTRL_TL_VLAN_CHECK_CMD_OFFSET  5
	#define TL_CTRL_TL_VLAN_CHECK_CMD_LEN     2
	#define TL_CTRL_TL_VLAN_CHECK_CMD_DEFAULT 0x0
	/*[field] TL_VLAN_CHECK_DE_ACCE*/
	#define TL_CTRL_TL_VLAN_CHECK_DE_ACCE
	#define TL_CTRL_TL_VLAN_CHECK_DE_ACCE_OFFSET  7
	#define TL_CTRL_TL_VLAN_CHECK_DE_ACCE_LEN     1
	#define TL_CTRL_TL_VLAN_CHECK_DE_ACCE_DEFAULT 0x0
	/*[field] PPPOE_MULTICAST_CMD*/
	#define TL_CTRL_PPPOE_MULTICAST_CMD
	#define TL_CTRL_PPPOE_MULTICAST_CMD_OFFSET  8
	#define TL_CTRL_PPPOE_MULTICAST_CMD_LEN     2
	#define TL_CTRL_PPPOE_MULTICAST_CMD_DEFAULT 0x3
	/*[field] PPPOE_MULTICAST_DE_ACCE*/
	#define TL_CTRL_PPPOE_MULTICAST_DE_ACCE
	#define TL_CTRL_PPPOE_MULTICAST_DE_ACCE_OFFSET  10
	#define TL_CTRL_PPPOE_MULTICAST_DE_ACCE_LEN     1
	#define TL_CTRL_PPPOE_MULTICAST_DE_ACCE_DEFAULT 0x0
	/*[field] UDP_CSUM_ZERO_CMD*/
	#define TL_CTRL_UDP_CSUM_ZERO_CMD
	#define TL_CTRL_UDP_CSUM_ZERO_CMD_OFFSET  11
	#define TL_CTRL_UDP_CSUM_ZERO_CMD_LEN     2
	#define TL_CTRL_UDP_CSUM_ZERO_CMD_DEFAULT 0x3
	/*[field] UDP_CSUM_ZERO_DE_ACCE*/
	#define TL_CTRL_UDP_CSUM_ZERO_DE_ACCE
	#define TL_CTRL_UDP_CSUM_ZERO_DE_ACCE_OFFSET  13
	#define TL_CTRL_UDP_CSUM_ZERO_DE_ACCE_LEN     1
	#define TL_CTRL_UDP_CSUM_ZERO_DE_ACCE_DEFAULT 0x0
	/*[field] TL_HASH_MODE_0*/
	#define TL_CTRL_TL_HASH_MODE_0
	#define TL_CTRL_TL_HASH_MODE_0_OFFSET  14
	#define TL_CTRL_TL_HASH_MODE_0_LEN     2
	#define TL_CTRL_TL_HASH_MODE_0_DEFAULT 0x0
	/*[field] TL_HASH_MODE_1*/
	#define TL_CTRL_TL_HASH_MODE_1
	#define TL_CTRL_TL_HASH_MODE_1_OFFSET  16
	#define TL_CTRL_TL_HASH_MODE_1_LEN     2
	#define TL_CTRL_TL_HASH_MODE_1_DEFAULT 0x1
	/*[field] IPV4_DF_SET*/
	#define TL_CTRL_IPV4_DF_SET
	#define TL_CTRL_IPV4_DF_SET_OFFSET  18
	#define TL_CTRL_IPV4_DF_SET_LEN     2
	#define TL_CTRL_IPV4_DF_SET_DEFAULT 0x0
	/*[field] TL_MAP_SRC_CHECK_CMD*/
	#define TL_CTRL_TL_MAP_SRC_CHECK_CMD
	#define TL_CTRL_TL_MAP_SRC_CHECK_CMD_OFFSET  20
	#define TL_CTRL_TL_MAP_SRC_CHECK_CMD_LEN     2
	#define TL_CTRL_TL_MAP_SRC_CHECK_CMD_DEFAULT 0x3
	/*[field] TL_MAP_DST_CHECK_CMD*/
	#define TL_CTRL_TL_MAP_DST_CHECK_CMD
	#define TL_CTRL_TL_MAP_DST_CHECK_CMD_OFFSET  22
	#define TL_CTRL_TL_MAP_DST_CHECK_CMD_LEN     2
	#define TL_CTRL_TL_MAP_DST_CHECK_CMD_DEFAULT 0x3
	/*[field] TL_MAP_NON_TCP_UDP_CMD*/
	#define TL_CTRL_TL_MAP_NON_TCP_UDP_CMD
	#define TL_CTRL_TL_MAP_NON_TCP_UDP_CMD_OFFSET  24
	#define TL_CTRL_TL_MAP_NON_TCP_UDP_CMD_LEN     2
	#define TL_CTRL_TL_MAP_NON_TCP_UDP_CMD_DEFAULT 0x3
	/*[field] TL_MAP_UDP_CSUM_ZERO_CMD*/
	#define TL_CTRL_TL_MAP_UDP_CSUM_ZERO_CMD
	#define TL_CTRL_TL_MAP_UDP_CSUM_ZERO_CMD_OFFSET  26
	#define TL_CTRL_TL_MAP_UDP_CSUM_ZERO_CMD_LEN     2
	#define TL_CTRL_TL_MAP_UDP_CSUM_ZERO_CMD_DEFAULT 0x3

struct tl_ctrl {
	a_uint32_t  _reserved0:4;
	a_uint32_t  tl_map_udp_csum_zero_cmd:2;
	a_uint32_t  tl_map_non_tcp_udp_cmd:2;
	a_uint32_t  tl_map_dst_check_cmd:2;
	a_uint32_t  tl_map_src_check_cmd:2;
	a_uint32_t  ipv4_df_set:2;
	a_uint32_t  tl_hash_mode_1:2;
	a_uint32_t  tl_hash_mode_0:2;
	a_uint32_t  udp_csum_zero_de_acce:1;
	a_uint32_t  udp_csum_zero_cmd:2;
	a_uint32_t  pppoe_multicast_de_acce:1;
	a_uint32_t  pppoe_multicast_cmd:2;
	a_uint32_t  tl_vlan_check_de_acce:1;
	a_uint32_t  tl_vlan_check_cmd:2;
	a_uint32_t  tl_src_if_check_de_acce:1;
	a_uint32_t  tl_src_if_check_cmd:2;
	a_uint32_t  tl_de_acce_cmd:2;
};

union tl_ctrl_u {
	a_uint32_t val;
	struct tl_ctrl bf;
};

/*[table] TL_L3_IF_TBL*/
#define TL_L3_IF_TBL
#define TL_L3_IF_TBL_ADDRESS 0x3000
#define TL_L3_IF_TBL_NUM     128
#define TL_L3_IF_TBL_INC     0x10
#define TL_L3_IF_TBL_TYPE    REG_TYPE_RW
#define TL_L3_IF_TBL_DEFAULT 0x0
	/*[field] IPV4_DECAP_EN*/
	#define TL_L3_IF_TBL_IPV4_DECAP_EN
	#define TL_L3_IF_TBL_IPV4_DECAP_EN_OFFSET  0
	#define TL_L3_IF_TBL_IPV4_DECAP_EN_LEN     1
	#define TL_L3_IF_TBL_IPV4_DECAP_EN_DEFAULT 0x0
	/*[field] IPV6_DECAP_EN*/
	#define TL_L3_IF_TBL_IPV6_DECAP_EN
	#define TL_L3_IF_TBL_IPV6_DECAP_EN_OFFSET  1
	#define TL_L3_IF_TBL_IPV6_DECAP_EN_LEN     1
	#define TL_L3_IF_TBL_IPV6_DECAP_EN_DEFAULT 0x0
	/*[field] DMAC_CHECK_DIS*/
	#define TL_L3_IF_TBL_DMAC_CHECK_DIS
	#define TL_L3_IF_TBL_DMAC_CHECK_DIS_OFFSET  2
	#define TL_L3_IF_TBL_DMAC_CHECK_DIS_LEN     1
	#define TL_L3_IF_TBL_DMAC_CHECK_DIS_DEFAULT 0x0
	/*[field] TTL_EXCEED_CMD*/
	#define TL_L3_IF_TBL_TTL_EXCEED_CMD
	#define TL_L3_IF_TBL_TTL_EXCEED_CMD_OFFSET  3
	#define TL_L3_IF_TBL_TTL_EXCEED_CMD_LEN     2
	#define TL_L3_IF_TBL_TTL_EXCEED_CMD_DEFAULT 0x0
	/*[field] TTL_EXCEED_DE_ACCE*/
	#define TL_L3_IF_TBL_TTL_EXCEED_DE_ACCE
	#define TL_L3_IF_TBL_TTL_EXCEED_DE_ACCE_OFFSET  5
	#define TL_L3_IF_TBL_TTL_EXCEED_DE_ACCE_LEN     1
	#define TL_L3_IF_TBL_TTL_EXCEED_DE_ACCE_DEFAULT 0x0
	/*[field] LPM_EN*/
	#define TL_L3_IF_TBL_LPM_EN
	#define TL_L3_IF_TBL_LPM_EN_OFFSET  6
	#define TL_L3_IF_TBL_LPM_EN_LEN     1
	#define TL_L3_IF_TBL_LPM_EN_DEFAULT 0x0
	/*[field] MIN_IPV6_MTU*/
	#define TL_L3_IF_TBL_MIN_IPV6_MTU
	#define TL_L3_IF_TBL_MIN_IPV6_MTU_OFFSET  7
	#define TL_L3_IF_TBL_MIN_IPV6_MTU_LEN     14
	#define TL_L3_IF_TBL_MIN_IPV6_MTU_DEFAULT 0x0

struct tl_l3_if_tbl {
	a_uint32_t  _reserved0:11;
	a_uint32_t  min_ipv6_mtu:14;
	a_uint32_t  lpm_en:1;
	a_uint32_t  ttl_exceed_de_acce:1;
	a_uint32_t  ttl_exceed_cmd:2;
	a_uint32_t  dmac_check_dis:1;
	a_uint32_t  ipv6_decap_en:1;
	a_uint32_t  ipv4_decap_en:1;
};

union tl_l3_if_tbl_u {
	a_uint32_t val;
	struct tl_l3_if_tbl bf;
};

/*[table] TL_KEY_GEN*/
#define TL_KEY_GEN
#define TL_KEY_GEN_ADDRESS 0x3800
#define TL_KEY_GEN_NUM     16
#define TL_KEY_GEN_INC     0x10
#define TL_KEY_GEN_TYPE    REG_TYPE_RW
#define TL_KEY_GEN_DEFAULT 0x0
	/*[field] SIP_INC*/
	#define TL_KEY_GEN_SIP_INC
	#define TL_KEY_GEN_SIP_INC_OFFSET  0
	#define TL_KEY_GEN_SIP_INC_LEN     1
	#define TL_KEY_GEN_SIP_INC_DEFAULT 0x0
	/*[field] DIP_INC*/
	#define TL_KEY_GEN_DIP_INC
	#define TL_KEY_GEN_DIP_INC_OFFSET  1
	#define TL_KEY_GEN_DIP_INC_LEN     1
	#define TL_KEY_GEN_DIP_INC_DEFAULT 0x0
	/*[field] IP_PROT_INC*/
	#define TL_KEY_GEN_IP_PROT_INC
	#define TL_KEY_GEN_IP_PROT_INC_OFFSET  2
	#define TL_KEY_GEN_IP_PROT_INC_LEN     1
	#define TL_KEY_GEN_IP_PROT_INC_DEFAULT 0x0
	/*[field] SPORT_INC*/
	#define TL_KEY_GEN_SPORT_INC
	#define TL_KEY_GEN_SPORT_INC_OFFSET  3
	#define TL_KEY_GEN_SPORT_INC_LEN     1
	#define TL_KEY_GEN_SPORT_INC_DEFAULT 0x0
	/*[field] DPORT_INC*/
	#define TL_KEY_GEN_DPORT_INC
	#define TL_KEY_GEN_DPORT_INC_OFFSET  4
	#define TL_KEY_GEN_DPORT_INC_LEN     1
	#define TL_KEY_GEN_DPORT_INC_DEFAULT 0x0
	/*[field] VNI_RESV_INC*/
	#define TL_KEY_GEN_VNI_RESV_INC
	#define TL_KEY_GEN_VNI_RESV_INC_OFFSET  5
	#define TL_KEY_GEN_VNI_RESV_INC_LEN     1
	#define TL_KEY_GEN_VNI_RESV_INC_DEFAULT 0x0
	/*[field] UDF0_INC*/
	#define TL_KEY_GEN_UDF0_INC
	#define TL_KEY_GEN_UDF0_INC_OFFSET  6
	#define TL_KEY_GEN_UDF0_INC_LEN     1
	#define TL_KEY_GEN_UDF0_INC_DEFAULT 0x0
	/*[field] UDF0_ID*/
	#define TL_KEY_GEN_UDF0_ID
	#define TL_KEY_GEN_UDF0_ID_OFFSET  7
	#define TL_KEY_GEN_UDF0_ID_LEN     2
	#define TL_KEY_GEN_UDF0_ID_DEFAULT 0x0
	/*[field] UDF1_INC*/
	#define TL_KEY_GEN_UDF1_INC
	#define TL_KEY_GEN_UDF1_INC_OFFSET  9
	#define TL_KEY_GEN_UDF1_INC_LEN     1
	#define TL_KEY_GEN_UDF1_INC_DEFAULT 0x0
	/*[field] UDF1_ID*/
	#define TL_KEY_GEN_UDF1_ID
	#define TL_KEY_GEN_UDF1_ID_OFFSET  10
	#define TL_KEY_GEN_UDF1_ID_LEN     2
	#define TL_KEY_GEN_UDF1_ID_DEFAULT 0x0
	/*[field] VNI_RESV_MASK*/
	#define TL_KEY_GEN_VNI_RESV_MASK
	#define TL_KEY_GEN_VNI_RESV_MASK_OFFSET  12
	#define TL_KEY_GEN_VNI_RESV_MASK_LEN     32
	#define TL_KEY_GEN_VNI_RESV_MASK_DEFAULT 0x0
	/*[field] UDF0_MASK*/
	#define TL_KEY_GEN_UDF0_MASK
	#define TL_KEY_GEN_UDF0_MASK_OFFSET  44
	#define TL_KEY_GEN_UDF0_MASK_LEN     16
	#define TL_KEY_GEN_UDF0_MASK_DEFAULT 0x0
	/*[field] UDF1_MASK*/
	#define TL_KEY_GEN_UDF1_MASK
	#define TL_KEY_GEN_UDF1_MASK_OFFSET  60
	#define TL_KEY_GEN_UDF1_MASK_LEN     16
	#define TL_KEY_GEN_UDF1_MASK_DEFAULT 0x0
	/*[field] KEY_TYPE*/
	#define TL_KEY_GEN_KEY_TYPE
	#define TL_KEY_GEN_KEY_TYPE_OFFSET  76
	#define TL_KEY_GEN_KEY_TYPE_LEN     4
	#define TL_KEY_GEN_KEY_TYPE_DEFAULT 0x0

struct tl_key_gen {
	a_uint32_t  vni_resv_mask_0:20;
	a_uint32_t  udf1_id:2;
	a_uint32_t  udf1_inc:1;
	a_uint32_t  udf0_id:2;
	a_uint32_t  udf0_inc:1;
	a_uint32_t  vni_resv_inc:1;
	a_uint32_t  dport_inc:1;
	a_uint32_t  sport_inc:1;
	a_uint32_t  ip_prot_inc:1;
	a_uint32_t  dip_inc:1;
	a_uint32_t  sip_inc:1;
	a_uint32_t  udf1_mask_0:4;
	a_uint32_t  udf0_mask:16;
	a_uint32_t  vni_resv_mask_1:12;
	a_uint32_t  _reserved0:16;
	a_uint32_t  key_type:4;
	a_uint32_t  udf1_mask_1:12;
};

union tl_key_gen_u {
	a_uint32_t val[3];
	struct tl_key_gen bf;
};

/*[table] TL_TBL*/
#define TL_TBL
#define TL_TBL_ADDRESS 0x4000
#define TL_TBL_NUM     128
#define TL_TBL_INC     0x40
#define TL_TBL_TYPE    REG_TYPE_RW
#define TL_TBL_DEFAULT 0x0
	/*[field] VALID*/
	#define TL_TBL_VALID
	#define TL_TBL_VALID_OFFSET  0
	#define TL_TBL_VALID_LEN     1
	#define TL_TBL_VALID_DEFAULT 0x0
	/*[field] ENTRY_TYPE*/
	#define TL_TBL_ENTRY_TYPE
	#define TL_TBL_ENTRY_TYPE_OFFSET  1
	#define TL_TBL_ENTRY_TYPE_LEN     1
	#define TL_TBL_ENTRY_TYPE_DEFAULT 0x0
	/*[field] KEY_TYPE*/
	#define TL_TBL_KEY_TYPE
	#define TL_TBL_KEY_TYPE_OFFSET  2
	#define TL_TBL_KEY_TYPE_LEN     4
	#define TL_TBL_KEY_TYPE_DEFAULT 0x0
	/*[field] PROTOCOL*/
	#define TL_TBL_PROTOCOL
	#define TL_TBL_PROTOCOL_OFFSET  6
	#define TL_TBL_PROTOCOL_LEN     8
	#define TL_TBL_PROTOCOL_DEFAULT 0x0
	/*[field] IPV4_SRC_ADDR reuse ENTRY_TYPE[1]*/
	#define TL_TBL_IPV4_SRC_ADDR
	#define TL_TBL_IPV4_SRC_ADDR_OFFSET  14
	#define TL_TBL_IPV4_SRC_ADDR_LEN     32
	#define TL_TBL_IPV4_SRC_ADDR_DEFAULT 0x0
	/*[field] IPV6_SRC_ADDR reuse ENTRY_TYPE[0]*/
	#define TL_TBL_IPV6_SRC_ADDR
	#define TL_TBL_IPV6_SRC_ADDR_OFFSET  14
	#define TL_TBL_IPV6_SRC_ADDR_LEN     128
	#define TL_TBL_IPV6_SRC_ADDR_DEFAULT 0x0
	/*[field] IPV4_DST_ADDR reuse ENTRY_TYPE[1]*/
	#define TL_TBL_IPV4_DST_ADDR
	#define TL_TBL_IPV4_DST_ADDR_OFFSET  46
	#define TL_TBL_IPV4_DST_ADDR_LEN     32
	#define TL_TBL_IPV4_DST_ADDR_DEFAULT 0x0
	/*[field] IPV6_DST_ADDR reuse ENTRY_TYPE[0]*/
	#define TL_TBL_IPV6_DST_ADDR
	#define TL_TBL_IPV6_DST_ADDR_OFFSET  142
	#define TL_TBL_IPV6_DST_ADDR_LEN     128
	#define TL_TBL_IPV6_DST_ADDR_DEFAULT 0x0
	/*[field] L4_SPORT*/
	#define TL_TBL_L4_SPORT
	#define TL_TBL_L4_SPORT_OFFSET  270
	#define TL_TBL_L4_SPORT_LEN     16
	#define TL_TBL_L4_SPORT_DEFAULT 0x0
	/*[field] L4_DPORT*/
	#define TL_TBL_L4_DPORT
	#define TL_TBL_L4_DPORT_OFFSET  286
	#define TL_TBL_L4_DPORT_LEN     16
	#define TL_TBL_L4_DPORT_DEFAULT 0x0
	/*[field] VNI_RESV*/
	#define TL_TBL_VNI_RESV
	#define TL_TBL_VNI_RESV_OFFSET  302
	#define TL_TBL_VNI_RESV_LEN     32
	#define TL_TBL_VNI_RESV_DEFAULT 0x0
	/*[field] UDF0*/
	#define TL_TBL_UDF0
	#define TL_TBL_UDF0_OFFSET  334
	#define TL_TBL_UDF0_LEN     16
	#define TL_TBL_UDF0_DEFAULT 0x0
	/*[field] UDF1*/
	#define TL_TBL_UDF1
	#define TL_TBL_UDF1_OFFSET  350
	#define TL_TBL_UDF1_LEN     16
	#define TL_TBL_UDF1_DEFAULT 0x0
	/*[field] VNI_RESV_VALID*/
	#define TL_TBL_VNI_RESV_VALID
	#define TL_TBL_VNI_RESV_VALID_OFFSET  366
	#define TL_TBL_VNI_RESV_VALID_LEN     1
	#define TL_TBL_VNI_RESV_VALID_DEFAULT 0x0
	/*[field] UDF0_VALID*/
	#define TL_TBL_UDF0_VALID
	#define TL_TBL_UDF0_VALID_OFFSET  367
	#define TL_TBL_UDF0_VALID_LEN     1
	#define TL_TBL_UDF0_VALID_DEFAULT 0x0
	/*[field] UDF1_VALID*/
	#define TL_TBL_UDF1_VALID
	#define TL_TBL_UDF1_VALID_OFFSET  368
	#define TL_TBL_UDF1_VALID_LEN     1
	#define TL_TBL_UDF1_VALID_DEFAULT 0x0
	/*[field] FWD_TYPE*/
	#define TL_TBL_FWD_TYPE
	#define TL_TBL_FWD_TYPE_OFFSET  369
	#define TL_TBL_FWD_TYPE_LEN     2
	#define TL_TBL_FWD_TYPE_DEFAULT 0x0
	/*[field] DE_ACCE*/
	#define TL_TBL_DE_ACCE
	#define TL_TBL_DE_ACCE_OFFSET  371
	#define TL_TBL_DE_ACCE_LEN     1
	#define TL_TBL_DE_ACCE_DEFAULT 0x0
	/*[field] DECAP_EN*/
	#define TL_TBL_DECAP_EN
	#define TL_TBL_DECAP_EN_OFFSET  372
	#define TL_TBL_DECAP_EN_LEN     1
	#define TL_TBL_DECAP_EN_DEFAULT 0x0
	/*[field] UDP_CSUM_ZERO*/
	#define TL_TBL_UDP_CSUM_ZERO
	#define TL_TBL_UDP_CSUM_ZERO_OFFSET  373
	#define TL_TBL_UDP_CSUM_ZERO_LEN     1
	#define TL_TBL_UDP_CSUM_ZERO_DEFAULT 0x0
	/*[field] SERVICE_CODE_EN*/
	#define TL_TBL_SERVICE_CODE_EN
	#define TL_TBL_SERVICE_CODE_EN_OFFSET  374
	#define TL_TBL_SERVICE_CODE_EN_LEN     1
	#define TL_TBL_SERVICE_CODE_EN_DEFAULT 0x0
	/*[field] SERVICE_CODE*/
	#define TL_TBL_SERVICE_CODE
	#define TL_TBL_SERVICE_CODE_OFFSET  375
	#define TL_TBL_SERVICE_CODE_LEN     8
	#define TL_TBL_SERVICE_CODE_DEFAULT 0x0
	/*[field] SPCP_MODE*/
	#define TL_TBL_SPCP_MODE
	#define TL_TBL_SPCP_MODE_OFFSET  383
	#define TL_TBL_SPCP_MODE_LEN     1
	#define TL_TBL_SPCP_MODE_DEFAULT 0x0
	/*[field] SDEI_MODE*/
	#define TL_TBL_SDEI_MODE
	#define TL_TBL_SDEI_MODE_OFFSET  384
	#define TL_TBL_SDEI_MODE_LEN     1
	#define TL_TBL_SDEI_MODE_DEFAULT 0x0
	/*[field] CPCP_MODE*/
	#define TL_TBL_CPCP_MODE
	#define TL_TBL_CPCP_MODE_OFFSET  385
	#define TL_TBL_CPCP_MODE_LEN     1
	#define TL_TBL_CPCP_MODE_DEFAULT 0x0
	/*[field] CDEI_MODE*/
	#define TL_TBL_CDEI_MODE
	#define TL_TBL_CDEI_MODE_OFFSET  386
	#define TL_TBL_CDEI_MODE_LEN     1
	#define TL_TBL_CDEI_MODE_DEFAULT 0x0
	/*[field] TTL_MODE*/
	#define TL_TBL_TTL_MODE
	#define TL_TBL_TTL_MODE_OFFSET  387
	#define TL_TBL_TTL_MODE_LEN     1
	#define TL_TBL_TTL_MODE_DEFAULT 0x0
	/*[field] DSCP_MODE*/
	#define TL_TBL_DSCP_MODE
	#define TL_TBL_DSCP_MODE_OFFSET  388
	#define TL_TBL_DSCP_MODE_LEN     1
	#define TL_TBL_DSCP_MODE_DEFAULT 0x0
	/*[field] ECN_MODE*/
	#define TL_TBL_ECN_MODE
	#define TL_TBL_ECN_MODE_OFFSET  389
	#define TL_TBL_ECN_MODE_LEN     2
	#define TL_TBL_ECN_MODE_DEFAULT 0x0
	/*[field] SRC_INFO_VALID*/
	#define TL_TBL_SRC_INFO_VALID
	#define TL_TBL_SRC_INFO_VALID_OFFSET  391
	#define TL_TBL_SRC_INFO_VALID_LEN     1
	#define TL_TBL_SRC_INFO_VALID_DEFAULT 0x0
	/*[field] SRC_INFO_TYPE*/
	#define TL_TBL_SRC_INFO_TYPE
	#define TL_TBL_SRC_INFO_TYPE_OFFSET  392
	#define TL_TBL_SRC_INFO_TYPE_LEN     1
	#define TL_TBL_SRC_INFO_TYPE_DEFAULT 0x0
	/*[field] SRC_INFO*/
	#define TL_TBL_SRC_INFO
	#define TL_TBL_SRC_INFO_OFFSET  393
	#define TL_TBL_SRC_INFO_LEN     8
	#define TL_TBL_SRC_INFO_DEFAULT 0x0
	/*[field] TL_L3_IF*/
	#define TL_TBL_TL_L3_IF
	#define TL_TBL_TL_L3_IF_OFFSET  401
	#define TL_TBL_TL_L3_IF_LEN     7
	#define TL_TBL_TL_L3_IF_DEFAULT 0x0
	/*[field] SVLAN_FMT*/
	#define TL_TBL_SVLAN_FMT
	#define TL_TBL_SVLAN_FMT_OFFSET  408
	#define TL_TBL_SVLAN_FMT_LEN     1
	#define TL_TBL_SVLAN_FMT_DEFAULT 0x0
	/*[field] SVLAN_ID*/
	#define TL_TBL_SVLAN_ID
	#define TL_TBL_SVLAN_ID_OFFSET  409
	#define TL_TBL_SVLAN_ID_LEN     12
	#define TL_TBL_SVLAN_ID_DEFAULT 0x0
	/*[field] CVLAN_FMT*/
	#define TL_TBL_CVLAN_FMT
	#define TL_TBL_CVLAN_FMT_OFFSET  421
	#define TL_TBL_CVLAN_FMT_LEN     1
	#define TL_TBL_CVLAN_FMT_DEFAULT 0x0
	/*[field] CVLAN_ID*/
	#define TL_TBL_CVLAN_ID
	#define TL_TBL_CVLAN_ID_OFFSET  422
	#define TL_TBL_CVLAN_ID_LEN     12
	#define TL_TBL_CVLAN_ID_DEFAULT 0x0
	/*[field] SVLAN_CHECK_EN*/
	#define TL_TBL_SVLAN_CHECK_EN
	#define TL_TBL_SVLAN_CHECK_EN_OFFSET  434
	#define TL_TBL_SVLAN_CHECK_EN_LEN     1
	#define TL_TBL_SVLAN_CHECK_EN_DEFAULT 0x0
	/*[field] CVLAN_CHECK_EN*/
	#define TL_TBL_CVLAN_CHECK_EN
	#define TL_TBL_CVLAN_CHECK_EN_OFFSET  435
	#define TL_TBL_CVLAN_CHECK_EN_LEN     1
	#define TL_TBL_CVLAN_CHECK_EN_DEFAULT 0x0
	/*[field] TL_L3_IF_CHECK_EN*/
	#define TL_TBL_TL_L3_IF_CHECK_EN
	#define TL_TBL_TL_L3_IF_CHECK_EN_OFFSET  436
	#define TL_TBL_TL_L3_IF_CHECK_EN_LEN     1
	#define TL_TBL_TL_L3_IF_CHECK_EN_DEFAULT 0x0
	/*[field] EXP_PROFILE*/
	#define TL_TBL_EXP_PROFILE
	#define TL_TBL_EXP_PROFILE_OFFSET  437
	#define TL_TBL_EXP_PROFILE_LEN     2
	#define TL_TBL_EXP_PROFILE_DEFAULT 0x0

struct tl_tbl_0 {
	a_uint32_t  ipv6_src_addr_0:18;
	a_uint32_t  protocol:8;
	a_uint32_t  key_type:4;
	a_uint32_t  entry_type:1;
	a_uint32_t  valid:1;

	a_uint32_t  ipv6_src_addr_1:32;
	a_uint32_t  ipv6_src_addr_2:32;
	a_uint32_t  ipv6_src_addr_3:32;

	a_uint32_t  ipv6_dst_addr_0:18;
	a_uint32_t  ipv6_src_addr_4:14;

	a_uint32_t  ipv6_dst_addr_1:32;
	a_uint32_t  ipv6_dst_addr_2:32;
	a_uint32_t  ipv6_dst_addr_3:32;

	a_uint32_t  l4_dport_0:2;
	a_uint32_t  l4_sport:16;
	a_uint32_t  ipv6_dst_addr_4:14;

	a_uint32_t  vni_resv_0:18;
	a_uint32_t  l4_dport_1:14;

	a_uint32_t  udf1_0:2;
	a_uint32_t  udf0:16;
	a_uint32_t  vni_resv_1:14;

	a_uint32_t  spcp_mode:1;
	a_uint32_t  service_code:8;
	a_uint32_t  service_code_en:1;
	a_uint32_t  udp_csum_zero:1;
	a_uint32_t  decap_en:1;
	a_uint32_t  de_acce:1;
	a_uint32_t  fwd_type:2;
	a_uint32_t  udf1_valid:1;
	a_uint32_t  udf0_valid:1;
	a_uint32_t  vni_resv_valid:1;
	a_uint32_t  udf1_1:14;

	a_uint32_t  svlan_id_0:7;
	a_uint32_t  svlan_fmt:1;
	a_uint32_t  tl_l3_if:7;
	a_uint32_t  src_info:8;
	a_uint32_t  src_info_type:1;
	a_uint32_t  src_info_valid:1;
	a_uint32_t  ecn_mode:2;
	a_uint32_t  dscp_mode:1;
	a_uint32_t  ttl_mode:1;
	a_uint32_t  cdei_mode:1;
	a_uint32_t  cpcp_mode:1;
	a_uint32_t  sdei_mode:1;

	a_uint32_t  _reserved0:9;
	a_uint32_t  exp_profile:2;
	a_uint32_t  tl_l3_if_check_en:1;
	a_uint32_t  cvlan_check_en:1;
	a_uint32_t  svlan_check_en:1;
	a_uint32_t  cvlan_id:12;
	a_uint32_t  cvlan_fmt:1;
	a_uint32_t  svlan_id_1:5;
};

struct tl_tbl_1 {
	a_uint32_t  ipv4_src_addr_0:18;
	a_uint32_t  protocol:8;
	a_uint32_t  key_type:4;
	a_uint32_t  entry_type:1;
	a_uint32_t  valid:1;

	a_uint32_t  ipv4_dst_addr_0:18;
	a_uint32_t  ipv4_src_addr_1:14;

	a_uint32_t  _reserved0_0:18;
	a_uint32_t  ipv4_dst_addr_1:14;

	a_uint32_t  _reserved0_1:32;
	a_uint32_t  _reserved0_2:32;
	a_uint32_t  _reserved0_3:32;
	a_uint32_t  _reserved0_4:32;
	a_uint32_t  _reserved0_5:32;

	a_uint32_t  l4_dport_0:2;
	a_uint32_t  l4_sport:16;
	a_uint32_t  _reserved0_6:14;

	a_uint32_t  vni_resv_0:18;
	a_uint32_t  l4_dport_1:14;

	a_uint32_t  udf1_0:2;
	a_uint32_t  udf0:16;
	a_uint32_t  vni_resv_1:14;

	a_uint32_t  spcp_mode:1;
	a_uint32_t  service_code:8;
	a_uint32_t  service_code_en:1;
	a_uint32_t  udp_csum_zero:1;
	a_uint32_t  decap_en:1;
	a_uint32_t  de_acce:1;
	a_uint32_t  fwd_type:2;
	a_uint32_t  udf1_valid:1;
	a_uint32_t  udf0_valid:1;
	a_uint32_t  vni_resv_valid:1;
	a_uint32_t  udf1_1:14;

	a_uint32_t  svlan_id_0:7;
	a_uint32_t  svlan_fmt:1;
	a_uint32_t  tl_l3_if:7;
	a_uint32_t  src_info:8;
	a_uint32_t  src_info_type:1;
	a_uint32_t  src_info_valid:1;
	a_uint32_t  ecn_mode:2;
	a_uint32_t  dscp_mode:1;
	a_uint32_t  ttl_mode:1;
	a_uint32_t  cdei_mode:1;
	a_uint32_t  cpcp_mode:1;
	a_uint32_t  sdei_mode:1;

	a_uint32_t  _reserved1:9;
	a_uint32_t  exp_profile:2;
	a_uint32_t  tl_l3_if_check_en:1;
	a_uint32_t  cvlan_check_en:1;
	a_uint32_t  svlan_check_en:1;
	a_uint32_t  cvlan_id:12;
	a_uint32_t  cvlan_fmt:1;
	a_uint32_t  svlan_id_1:5;
};

union tl_tbl_u {
	a_uint32_t val[14];
	struct tl_tbl_0 bf0;
	struct tl_tbl_1 bf1;
};

/*[register] EG_IPV4_HDR_CTRL*/
#define EG_IPV4_HDR_CTRL
#define EG_IPV4_HDR_CTRL_ADDRESS 0x50
#define EG_IPV4_HDR_CTRL_NUM     1
#define EG_IPV4_HDR_CTRL_INC     0x4
#define EG_IPV4_HDR_CTRL_TYPE    REG_TYPE_RW
#define EG_IPV4_HDR_CTRL_DEFAULT 0x0
	/*[field] IPV4_ID_SEED*/
	#define EG_IPV4_HDR_CTRL_IPV4_ID_SEED
	#define EG_IPV4_HDR_CTRL_IPV4_ID_SEED_OFFSET  0
	#define EG_IPV4_HDR_CTRL_IPV4_ID_SEED_LEN     16
	#define EG_IPV4_HDR_CTRL_IPV4_ID_SEED_DEFAULT 0x0
	/*[field] IPV4_DF_SET*/
	#define EG_IPV4_HDR_CTRL_IPV4_DF_SET
	#define EG_IPV4_HDR_CTRL_IPV4_DF_SET_OFFSET  16
	#define EG_IPV4_HDR_CTRL_IPV4_DF_SET_LEN     2
	#define EG_IPV4_HDR_CTRL_IPV4_DF_SET_DEFAULT 0x0

struct eg_ipv4_hdr_ctrl {
	a_uint32_t  _reserved0:14;
	a_uint32_t  ipv4_df_set:2;
	a_uint32_t  ipv4_id_seed:16;
};

union eg_ipv4_hdr_ctrl_u {
	a_uint32_t val;
	struct eg_ipv4_hdr_ctrl bf;
};

/*[register] EG_UDP_ENTROPY_CTRL*/
#define EG_UDP_ENTROPY_CTRL
#define EG_UDP_ENTROPY_CTRL_ADDRESS 0x54
#define EG_UDP_ENTROPY_CTRL_NUM     1
#define EG_UDP_ENTROPY_CTRL_INC     0x4
#define EG_UDP_ENTROPY_CTRL_TYPE    REG_TYPE_RW
#define EG_UDP_ENTROPY_CTRL_DEFAULT 0x0
	/*[field] PORT_BASE*/
	#define EG_UDP_ENTROPY_CTRL_PORT_BASE
	#define EG_UDP_ENTROPY_CTRL_PORT_BASE_OFFSET  0
	#define EG_UDP_ENTROPY_CTRL_PORT_BASE_LEN     16
	#define EG_UDP_ENTROPY_CTRL_PORT_BASE_DEFAULT 0x0
	/*[field] PORT_MASK*/
	#define EG_UDP_ENTROPY_CTRL_PORT_MASK
	#define EG_UDP_ENTROPY_CTRL_PORT_MASK_OFFSET  16
	#define EG_UDP_ENTROPY_CTRL_PORT_MASK_LEN     16
	#define EG_UDP_ENTROPY_CTRL_PORT_MASK_DEFAULT 0x0

struct eg_udp_entropy_ctrl {
	a_uint32_t  port_mask:16;
	a_uint32_t  port_base:16;
};

union eg_udp_entropy_ctrl_u {
	a_uint32_t val;
	struct eg_udp_entropy_ctrl bf;
};

#ifndef IN_TUNNEL_MINI
/*[register] ECN_PROFILE*/
#define ECN_PROFILE
#define ECN_PROFILE_ADDRESS 0x60
#define ECN_PROFILE_NUM     1
#define ECN_PROFILE_INC     0x4
#define ECN_PROFILE_TYPE    REG_TYPE_RW
#define ECN_PROFILE_DEFAULT 0xe4a400
	/*[field] PROFILE0*/
	#define ECN_PROFILE_PROFILE0
	#define ECN_PROFILE_PROFILE0_OFFSET  0
	#define ECN_PROFILE_PROFILE0_LEN     8
	#define ECN_PROFILE_PROFILE0_DEFAULT 0x0
	/*[field] PROFILE1*/
	#define ECN_PROFILE_PROFILE1
	#define ECN_PROFILE_PROFILE1_OFFSET  8
	#define ECN_PROFILE_PROFILE1_LEN     8
	#define ECN_PROFILE_PROFILE1_DEFAULT 0xa4
	/*[field] PROFILE2*/
	#define ECN_PROFILE_PROFILE2
	#define ECN_PROFILE_PROFILE2_OFFSET  16
	#define ECN_PROFILE_PROFILE2_LEN     8
	#define ECN_PROFILE_PROFILE2_DEFAULT 0xe4

struct ecn_profile {
	a_uint32_t  _reserved0:8;
	a_uint32_t  profile2:8;
	a_uint32_t  profile1:8;
	a_uint32_t  profile0:8;
};

union ecn_profile_u {
	a_uint32_t val;
	struct ecn_profile bf;
};
#endif

/*[register] EG_PROTO_MAPPING0*/
#define EG_PROTO_MAPPING0
#define EG_PROTO_MAPPING0_ADDRESS 0x68
#define EG_PROTO_MAPPING0_NUM     2
#define EG_PROTO_MAPPING0_INC     0x4
#define EG_PROTO_MAPPING0_TYPE    REG_TYPE_RW
#define EG_PROTO_MAPPING0_DEFAULT 0x0
	/*[field] PROTOCOL0*/
	#define EG_PROTO_MAPPING0_PROTOCOL0
	#define EG_PROTO_MAPPING0_PROTOCOL0_OFFSET  0
	#define EG_PROTO_MAPPING0_PROTOCOL0_LEN     32
	#define EG_PROTO_MAPPING0_PROTOCOL0_DEFAULT 0x0

struct eg_proto_mapping0 {
	a_uint32_t  protocol0:32;
};

union eg_proto_mapping0_u {
	a_uint32_t val;
	struct eg_proto_mapping0 bf;
};

/*[register] EG_PROTO_MAPPING1*/
#define EG_PROTO_MAPPING1
#define EG_PROTO_MAPPING1_ADDRESS 0x70
#define EG_PROTO_MAPPING1_NUM     2
#define EG_PROTO_MAPPING1_INC     0x4
#define EG_PROTO_MAPPING1_TYPE    REG_TYPE_RW
#define EG_PROTO_MAPPING1_DEFAULT 0x0
	/*[field] PROTOCOL1*/
	#define EG_PROTO_MAPPING1_PROTOCOL1
	#define EG_PROTO_MAPPING1_PROTOCOL1_OFFSET  0
	#define EG_PROTO_MAPPING1_PROTOCOL1_LEN     32
	#define EG_PROTO_MAPPING1_PROTOCOL1_DEFAULT 0x0

struct eg_proto_mapping1 {
	a_uint32_t  protocol1:32;
};

union eg_proto_mapping1_u {
	a_uint32_t val;
	struct eg_proto_mapping1 bf;
};

/*[register] DBG_ADDR*/
#define DBG_ADDR
#define DBG_ADDR_ADDRESS 0x80
#define DBG_ADDR_NUM     1
#define DBG_ADDR_INC     0x4
#define DBG_ADDR_TYPE    REG_TYPE_RW
#define DBG_ADDR_DEFAULT 0x0
	/*[field] DBG_ADDR*/
	#define DBG_ADDR_DBG_ADDR
	#define DBG_ADDR_DBG_ADDR_OFFSET  0
	#define DBG_ADDR_DBG_ADDR_LEN     16
	#define DBG_ADDR_DBG_ADDR_DEFAULT 0x0

struct dbg_addr {
	a_uint32_t  _reserved0:16;
	a_uint32_t  dbg_addr:16;
};

union dbg_addr_u {
	a_uint32_t val;
	struct dbg_addr bf;
};

/*[register] DBG_DATA*/
#define DBG_DATA
#define DBG_DATA_ADDRESS 0x84
#define DBG_DATA_NUM     1
#define DBG_DATA_INC     0x4
#define DBG_DATA_TYPE    REG_TYPE_RO
#define DBG_DATA_DEFAULT 0x0
	/*[field] DBG_DATA*/
	#define DBG_DATA_DBG_DATA
	#define DBG_DATA_DBG_DATA_OFFSET  0
	#define DBG_DATA_DBG_DATA_LEN     32
	#define DBG_DATA_DBG_DATA_DEFAULT 0x0

struct dbg_data {
	a_uint32_t  dbg_data:32;
};

union dbg_data_u {
	a_uint32_t val;
	struct dbg_data bf;
};

/*[table] EG_HEADER_DATA*/
#define EG_HEADER_DATA
#define EG_HEADER_DATA_ADDRESS 0x34000
#define EG_HEADER_DATA_NUM     128
#define EG_HEADER_DATA_INC     0x80
#define EG_HEADER_DATA_TYPE    REG_TYPE_RW
#define EG_HEADER_DATA_DEFAULT 0x0
	/*[field] DATA*/
	#define EG_HEADER_DATA_DATA
	#define EG_HEADER_DATA_DATA_OFFSET  0
	#define EG_HEADER_DATA_DATA_LEN     1024
	#define EG_HEADER_DATA_DATA_DEFAULT 0x0

struct eg_header_data {
	a_uint32_t  data_0:32;
	a_uint32_t  data_1:32;
	a_uint32_t  data_2:32;
	a_uint32_t  data_3:32;
	a_uint32_t  data_4:32;
	a_uint32_t  data_5:32;
	a_uint32_t  data_6:32;
	a_uint32_t  data_7:32;
	a_uint32_t  data_8:32;
	a_uint32_t  data_9:32;
	a_uint32_t  data_10:32;
	a_uint32_t  data_11:32;
	a_uint32_t  data_12:32;
	a_uint32_t  data_13:32;
	a_uint32_t  data_14:32;
	a_uint32_t  data_15:32;
	a_uint32_t  data_16:32;
	a_uint32_t  data_17:32;
	a_uint32_t  data_18:32;
	a_uint32_t  data_19:32;
	a_uint32_t  data_20:32;
	a_uint32_t  data_21:32;
	a_uint32_t  data_22:32;
	a_uint32_t  data_23:32;
	a_uint32_t  data_24:32;
	a_uint32_t  data_25:32;
	a_uint32_t  data_26:32;
	a_uint32_t  data_27:32;
	a_uint32_t  data_28:32;
	a_uint32_t  data_29:32;
	a_uint32_t  data_30:32;
	a_uint32_t  data_31:32;
};

union eg_header_data_u {
	a_uint32_t val[32];
	struct eg_header_data bf;
};

/*[table] EG_XLAT_TUN_CTRL*/
#define EG_XLAT_TUN_CTRL
#define EG_XLAT_TUN_CTRL_ADDRESS 0x3a000
#define EG_XLAT_TUN_CTRL_NUM     128
#define EG_XLAT_TUN_CTRL_INC     0x10
#define EG_XLAT_TUN_CTRL_TYPE    REG_TYPE_RW
#define EG_XLAT_TUN_CTRL_DEFAULT 0x0
	/*[field] TYPE*/
	#define EG_XLAT_TUN_CTRL_TYPE_F
	#define EG_XLAT_TUN_CTRL_TYPE_F_OFFSET  0
	#define EG_XLAT_TUN_CTRL_TYPE_F_LEN     1
	#define EG_XLAT_TUN_CTRL_TYPE_F_DEFAULT 0x0
#if defined(MPPE)
	/*[field] MAPT_UDP_CSM0_DIS*/
	#define EG_XLAT_TUN_CTRL_MAPT_UDP_CSM0_DIS
	#define EG_XLAT_TUN_CTRL_MAPT_UDP_CSM0_DIS_OFFSET 1
	#define EG_XLAT_TUN_CTRL_MAPT_UDP_CSM0_DIS_LEN 1
	#define EG_XLAT_TUN_CTRL_MAPT_UDP_CSM0_DIS_DEFAULT 0x0
	/*[field] RESV*/
	#define EG_XLAT_TUN_CTRL_RESV
	#define EG_XLAT_TUN_CTRL_RESV_OFFSET  2
	#define EG_XLAT_TUN_CTRL_RESV_LEN     1
	#define EG_XLAT_TUN_CTRL_RESV_DEFAULT 0x0
#else
	/*[field] RESV*/
	#define EG_XLAT_TUN_CTRL_RESV
	#define EG_XLAT_TUN_CTRL_RESV_OFFSET  1
	#define EG_XLAT_TUN_CTRL_RESV_LEN     2
	#define EG_XLAT_TUN_CTRL_RESV_DEFAULT 0x0
#endif
	/*[field] IPV4_DF_MODE_EXT*/
	#define EG_XLAT_TUN_CTRL_IPV4_DF_EXT_MODE
	#define EG_XLAT_TUN_CTRL_IPV4_DF_EXT_MODE_OFFSET  3
	#define EG_XLAT_TUN_CTRL_IPV4_DF_EXT_MODE_LEN     1
	#define EG_XLAT_TUN_CTRL_IPV4_DF_EXT_MODE_DEFAULT 0x0
	/*[field] EDIT_RULE_ID*/
	#define EG_XLAT_TUN_CTRL_EDIT_RULE_ID
	#define EG_XLAT_TUN_CTRL_EDIT_RULE_ID_OFFSET  4
	#define EG_XLAT_TUN_CTRL_EDIT_RULE_ID_LEN     4
	#define EG_XLAT_TUN_CTRL_EDIT_RULE_ID_DEFAULT 0x0
	/*[field] EDIT_RULE_TARGET*/
	#define EG_XLAT_TUN_CTRL_EDIT_RULE_TARGET
	#define EG_XLAT_TUN_CTRL_EDIT_RULE_TARGET_OFFSET  8
	#define EG_XLAT_TUN_CTRL_EDIT_RULE_TARGET_LEN     2
	#define EG_XLAT_TUN_CTRL_EDIT_RULE_TARGET_DEFAULT 0x0
	/*[field] DATA_LENGTH*/
	#define EG_XLAT_TUN_CTRL_DATA_LENGTH
	#define EG_XLAT_TUN_CTRL_DATA_LENGTH_OFFSET  10
	#define EG_XLAT_TUN_CTRL_DATA_LENGTH_LEN     8
	#define EG_XLAT_TUN_CTRL_DATA_LENGTH_DEFAULT 0x0
	/*[field] VLAN_OFFSET*/
	#define EG_XLAT_TUN_CTRL_VLAN_OFFSET
	#define EG_XLAT_TUN_CTRL_VLAN_OFFSET_OFFSET  18
	#define EG_XLAT_TUN_CTRL_VLAN_OFFSET_LEN     5
	#define EG_XLAT_TUN_CTRL_VLAN_OFFSET_DEFAULT 0x0
	/*[field] L3_OFFSET*/
	#define EG_XLAT_TUN_CTRL_L3_OFFSET
	#define EG_XLAT_TUN_CTRL_L3_OFFSET_OFFSET  23
	#define EG_XLAT_TUN_CTRL_L3_OFFSET_LEN     6
	#define EG_XLAT_TUN_CTRL_L3_OFFSET_DEFAULT 0x0
	/*[field] PPPOE_EN*/
	#define EG_XLAT_TUN_CTRL_PPPOE_EN
	#define EG_XLAT_TUN_CTRL_PPPOE_EN_OFFSET  29
	#define EG_XLAT_TUN_CTRL_PPPOE_EN_LEN     1
	#define EG_XLAT_TUN_CTRL_PPPOE_EN_DEFAULT 0x0
	/*[field] IP_VER*/
	#define EG_XLAT_TUN_CTRL_IP_VER
	#define EG_XLAT_TUN_CTRL_IP_VER_OFFSET  30
	#define EG_XLAT_TUN_CTRL_IP_VER_LEN     1
	#define EG_XLAT_TUN_CTRL_IP_VER_DEFAULT 0x0
	/*[field] DSCP_MODE*/
	#define EG_XLAT_TUN_CTRL_DSCP_MODE
	#define EG_XLAT_TUN_CTRL_DSCP_MODE_OFFSET  31
	#define EG_XLAT_TUN_CTRL_DSCP_MODE_LEN     1
	#define EG_XLAT_TUN_CTRL_DSCP_MODE_DEFAULT 0x0
	/*[field] L4_OFFSET*/
	#define EG_XLAT_TUN_CTRL_L4_OFFSET
	#define EG_XLAT_TUN_CTRL_L4_OFFSET_OFFSET  32
	#define EG_XLAT_TUN_CTRL_L4_OFFSET_LEN     7
	#define EG_XLAT_TUN_CTRL_L4_OFFSET_DEFAULT 0x0
	/*[field] TUNNEL_OFFSET*/
	#define EG_XLAT_TUN_CTRL_TUNNEL_OFFSET
	#define EG_XLAT_TUN_CTRL_TUNNEL_OFFSET_OFFSET  39
	#define EG_XLAT_TUN_CTRL_TUNNEL_OFFSET_LEN     7
	#define EG_XLAT_TUN_CTRL_TUNNEL_OFFSET_DEFAULT 0x0
	/*[field] STAG_FMT*/
	#define EG_XLAT_TUN_CTRL_STAG_FMT
	#define EG_XLAT_TUN_CTRL_STAG_FMT_OFFSET  46
	#define EG_XLAT_TUN_CTRL_STAG_FMT_LEN     1
	#define EG_XLAT_TUN_CTRL_STAG_FMT_DEFAULT 0x0
	/*[field] CTAG_FMT*/
	#define EG_XLAT_TUN_CTRL_CTAG_FMT
	#define EG_XLAT_TUN_CTRL_CTAG_FMT_OFFSET  47
	#define EG_XLAT_TUN_CTRL_CTAG_FMT_LEN     1
	#define EG_XLAT_TUN_CTRL_CTAG_FMT_DEFAULT 0x0
	/*[field] SPCP_MODE*/
	#define EG_XLAT_TUN_CTRL_SPCP_MODE
	#define EG_XLAT_TUN_CTRL_SPCP_MODE_OFFSET  48
	#define EG_XLAT_TUN_CTRL_SPCP_MODE_LEN     1
	#define EG_XLAT_TUN_CTRL_SPCP_MODE_DEFAULT 0x0
	/*[field] SDEI_MODE*/
	#define EG_XLAT_TUN_CTRL_SDEI_MODE
	#define EG_XLAT_TUN_CTRL_SDEI_MODE_OFFSET  49
	#define EG_XLAT_TUN_CTRL_SDEI_MODE_LEN     1
	#define EG_XLAT_TUN_CTRL_SDEI_MODE_DEFAULT 0x0
	/*[field] CPCP_MODE*/
	#define EG_XLAT_TUN_CTRL_CPCP_MODE
	#define EG_XLAT_TUN_CTRL_CPCP_MODE_OFFSET  50
	#define EG_XLAT_TUN_CTRL_CPCP_MODE_LEN     1
	#define EG_XLAT_TUN_CTRL_CPCP_MODE_DEFAULT 0x0
	/*[field] CDEI_MODE*/
	#define EG_XLAT_TUN_CTRL_CDEI_MODE
	#define EG_XLAT_TUN_CTRL_CDEI_MODE_OFFSET  51
	#define EG_XLAT_TUN_CTRL_CDEI_MODE_LEN     1
	#define EG_XLAT_TUN_CTRL_CDEI_MODE_DEFAULT 0x0
	/*[field] ECN_MODE*/
	#define EG_XLAT_TUN_CTRL_ECN_MODE
	#define EG_XLAT_TUN_CTRL_ECN_MODE_OFFSET  52
	#define EG_XLAT_TUN_CTRL_ECN_MODE_LEN     2
	#define EG_XLAT_TUN_CTRL_ECN_MODE_DEFAULT 0x0
	/*[field] TTL_MODE*/
	#define EG_XLAT_TUN_CTRL_TTL_MODE
	#define EG_XLAT_TUN_CTRL_TTL_MODE_OFFSET  54
	#define EG_XLAT_TUN_CTRL_TTL_MODE_LEN     1
	#define EG_XLAT_TUN_CTRL_TTL_MODE_DEFAULT 0x0
	/*[field] IPV4_DF_MODE*/
	#define EG_XLAT_TUN_CTRL_IPV4_DF_MODE
	#define EG_XLAT_TUN_CTRL_IPV4_DF_MODE_OFFSET  55
	#define EG_XLAT_TUN_CTRL_IPV4_DF_MODE_LEN     1
	#define EG_XLAT_TUN_CTRL_IPV4_DF_MODE_DEFAULT 0x0
	/*[field] IPV4_ID_MODE*/
	#define EG_XLAT_TUN_CTRL_IPV4_ID_MODE
	#define EG_XLAT_TUN_CTRL_IPV4_ID_MODE_OFFSET  56
	#define EG_XLAT_TUN_CTRL_IPV4_ID_MODE_LEN     1
	#define EG_XLAT_TUN_CTRL_IPV4_ID_MODE_DEFAULT 0x0
	/*[field] IPV6_FL_MODE*/
	#define EG_XLAT_TUN_CTRL_IPV6_FL_MODE
	#define EG_XLAT_TUN_CTRL_IPV6_FL_MODE_OFFSET  57
	#define EG_XLAT_TUN_CTRL_IPV6_FL_MODE_LEN     2
	#define EG_XLAT_TUN_CTRL_IPV6_FL_MODE_DEFAULT 0x0
	/*[field] IP_PROTO_UPDATE*/
	#define EG_XLAT_TUN_CTRL_IP_PROTO_UPDATE
	#define EG_XLAT_TUN_CTRL_IP_PROTO_UPDATE_OFFSET  59
	#define EG_XLAT_TUN_CTRL_IP_PROTO_UPDATE_LEN     1
	#define EG_XLAT_TUN_CTRL_IP_PROTO_UPDATE_DEFAULT 0x0
	/*[field] L4_TYPE*/
	#define EG_XLAT_TUN_CTRL_L4_TYPE
	#define EG_XLAT_TUN_CTRL_L4_TYPE_OFFSET  60
	#define EG_XLAT_TUN_CTRL_L4_TYPE_LEN     3
	#define EG_XLAT_TUN_CTRL_L4_TYPE_DEFAULT 0x0
	/*[field] SPORT_ENTROPY_EN*/
	#define EG_XLAT_TUN_CTRL_SPORT_ENTROPY_EN
	#define EG_XLAT_TUN_CTRL_SPORT_ENTROPY_EN_OFFSET  63
	#define EG_XLAT_TUN_CTRL_SPORT_ENTROPY_EN_LEN     1
	#define EG_XLAT_TUN_CTRL_SPORT_ENTROPY_EN_DEFAULT 0x0
	/*[field] L4_CHECKSUM_EN*/
	#define EG_XLAT_TUN_CTRL_L4_CHECKSUM_EN
	#define EG_XLAT_TUN_CTRL_L4_CHECKSUM_EN_OFFSET  64
	#define EG_XLAT_TUN_CTRL_L4_CHECKSUM_EN_LEN     1
	#define EG_XLAT_TUN_CTRL_L4_CHECKSUM_EN_DEFAULT 0x0
	/*[field] VNI_MODE*/
	#define EG_XLAT_TUN_CTRL_VNI_MODE
	#define EG_XLAT_TUN_CTRL_VNI_MODE_OFFSET  65
	#define EG_XLAT_TUN_CTRL_VNI_MODE_LEN     1
	#define EG_XLAT_TUN_CTRL_VNI_MODE_DEFAULT 0x0
	/*[field] PAYLOAD_TYPE*/
	#define EG_XLAT_TUN_CTRL_PAYLOAD_TYPE
	#define EG_XLAT_TUN_CTRL_PAYLOAD_TYPE_OFFSET  66
	#define EG_XLAT_TUN_CTRL_PAYLOAD_TYPE_LEN     2
	#define EG_XLAT_TUN_CTRL_PAYLOAD_TYPE_DEFAULT 0x0
	/*[field] OUTPUT_VP_VALID*/
	#define EG_XLAT_TUN_CTRL_OUTPUT_VP_VALID
	#define EG_XLAT_TUN_CTRL_OUTPUT_VP_VALID_OFFSET  68
	#define EG_XLAT_TUN_CTRL_OUTPUT_VP_VALID_LEN     1
	#define EG_XLAT_TUN_CTRL_OUTPUT_VP_VALID_DEFAULT 0x0
	/*[field] OUTPUT_VP*/
	#define EG_XLAT_TUN_CTRL_OUTPUT_VP
	#define EG_XLAT_TUN_CTRL_OUTPUT_VP_OFFSET  69
	#define EG_XLAT_TUN_CTRL_OUTPUT_VP_LEN     8
	#define EG_XLAT_TUN_CTRL_OUTPUT_VP_DEFAULT 0x0

struct eg_xlat_tun_ctrl {
	a_uint32_t  dscp_mode:1;
	a_uint32_t  ip_ver:1;
	a_uint32_t  pppoe_en:1;
	a_uint32_t  l3_offset:6;
	a_uint32_t  vlan_offset:5;
	a_uint32_t  data_length:8;
	a_uint32_t  edit_rule_target:2;
	a_uint32_t  edit_rule_id:4;
	a_uint32_t  ipv4_df_mode_ext:1;
#if defined(MPPE)
	a_uint32_t  resv:1;
	a_uint32_t  mapt_udp_csm0_dis:1;
#else
	a_uint32_t  resv:2;
#endif
	a_uint32_t  type:1;

	a_uint32_t  sport_entropy_en:1;
	a_uint32_t  l4_type:3;
	a_uint32_t  ip_proto_update:1;
	a_uint32_t  ipv6_fl_mode:2;
	a_uint32_t  ipv4_id_mode:1;
	a_uint32_t  ipv4_df_mode:1;
	a_uint32_t  ttl_mode:1;
	a_uint32_t  ecn_mode:2;
	a_uint32_t  cdei_mode:1;
	a_uint32_t  cpcp_mode:1;
	a_uint32_t  sdei_mode:1;
	a_uint32_t  spcp_mode:1;
	a_uint32_t  ctag_fmt:1;
	a_uint32_t  stag_fmt:1;
	a_uint32_t  tunnel_offset:7;
	a_uint32_t  l4_offset:7;

	a_uint32_t  _reserved0:19;
	a_uint32_t  output_vp:8;
	a_uint32_t  output_vp_valid:1;
	a_uint32_t  payload_type:2;
	a_uint32_t  vni_mode:1;
	a_uint32_t  l4_checksum_en:1;
};

union eg_xlat_tun_ctrl_u {
	a_uint32_t val[3];
	struct eg_xlat_tun_ctrl bf;
};

/*[table] EG_EDIT_RULE*/
#define EG_EDIT_RULE
#define EG_EDIT_RULE_ADDRESS 0x3c000
#define EG_EDIT_RULE_NUM     16
#define EG_EDIT_RULE_INC     0x10
#define EG_EDIT_RULE_TYPE    REG_TYPE_RW
#define EG_EDIT_RULE_DEFAULT 0x0
	/*[field] SRC1*/
	#define EG_EDIT_RULE_SRC1
	#define EG_EDIT_RULE_SRC1_OFFSET  0
	#define EG_EDIT_RULE_SRC1_LEN     6
	#define EG_EDIT_RULE_SRC1_DEFAULT 0x0
	/*[field] SRC2*/
	#define EG_EDIT_RULE_SRC2
	#define EG_EDIT_RULE_SRC2_OFFSET  6
	#define EG_EDIT_RULE_SRC2_LEN     3
	#define EG_EDIT_RULE_SRC2_DEFAULT 0x0
	/*[field] VALID2_0*/
	#define EG_EDIT_RULE_VALID2_0
	#define EG_EDIT_RULE_VALID2_0_OFFSET  9
	#define EG_EDIT_RULE_VALID2_0_LEN     1
	#define EG_EDIT_RULE_VALID2_0_DEFAULT 0x0
	/*[field] START2_0*/
	#define EG_EDIT_RULE_START2_0
	#define EG_EDIT_RULE_START2_0_OFFSET  10
	#define EG_EDIT_RULE_START2_0_LEN     5
	#define EG_EDIT_RULE_START2_0_DEFAULT 0x0
	/*[field] WIDTH2_0*/
	#define EG_EDIT_RULE_WIDTH2_0
	#define EG_EDIT_RULE_WIDTH2_0_OFFSET  15
	#define EG_EDIT_RULE_WIDTH2_0_LEN     5
	#define EG_EDIT_RULE_WIDTH2_0_DEFAULT 0x0
	/*[field] POS2_0*/
	#define EG_EDIT_RULE_POS2_0
	#define EG_EDIT_RULE_POS2_0_OFFSET  20
	#define EG_EDIT_RULE_POS2_0_LEN     7
	#define EG_EDIT_RULE_POS2_0_DEFAULT 0x0
	/*[field] VALID2_1*/
	#define EG_EDIT_RULE_VALID2_1
	#define EG_EDIT_RULE_VALID2_1_OFFSET  27
	#define EG_EDIT_RULE_VALID2_1_LEN     1
	#define EG_EDIT_RULE_VALID2_1_DEFAULT 0x0
	/*[field] START2_1*/
	#define EG_EDIT_RULE_START2_1
	#define EG_EDIT_RULE_START2_1_OFFSET  28
	#define EG_EDIT_RULE_START2_1_LEN     5
	#define EG_EDIT_RULE_START2_1_DEFAULT 0x0
	/*[field] WIDTH2_1*/
	#define EG_EDIT_RULE_WIDTH2_1
	#define EG_EDIT_RULE_WIDTH2_1_OFFSET  33
	#define EG_EDIT_RULE_WIDTH2_1_LEN     5
	#define EG_EDIT_RULE_WIDTH2_1_DEFAULT 0x0
	/*[field] POS2_1*/
	#define EG_EDIT_RULE_POS2_1
	#define EG_EDIT_RULE_POS2_1_OFFSET  38
	#define EG_EDIT_RULE_POS2_1_LEN     7
	#define EG_EDIT_RULE_POS2_1_DEFAULT 0x0
	/*[field] SRC3*/
	#define EG_EDIT_RULE_SRC3
	#define EG_EDIT_RULE_SRC3_OFFSET  45
	#define EG_EDIT_RULE_SRC3_LEN     3
	#define EG_EDIT_RULE_SRC3_DEFAULT 0x0
	/*[field] VALID3_0*/
	#define EG_EDIT_RULE_VALID3_0
	#define EG_EDIT_RULE_VALID3_0_OFFSET  48
	#define EG_EDIT_RULE_VALID3_0_LEN     1
	#define EG_EDIT_RULE_VALID3_0_DEFAULT 0x0
	/*[field] START3_0*/
	#define EG_EDIT_RULE_START3_0
	#define EG_EDIT_RULE_START3_0_OFFSET  49
	#define EG_EDIT_RULE_START3_0_LEN     4
	#define EG_EDIT_RULE_START3_0_DEFAULT 0x0
	/*[field] WIDTH3_0*/
	#define EG_EDIT_RULE_WIDTH3_0
	#define EG_EDIT_RULE_WIDTH3_0_OFFSET  53
	#define EG_EDIT_RULE_WIDTH3_0_LEN     4
	#define EG_EDIT_RULE_WIDTH3_0_DEFAULT 0x0
	/*[field] POS3_0*/
	#define EG_EDIT_RULE_POS3_0
	#define EG_EDIT_RULE_POS3_0_OFFSET  57
	#define EG_EDIT_RULE_POS3_0_LEN     7
	#define EG_EDIT_RULE_POS3_0_DEFAULT 0x0
	/*[field] VALID3_1*/
	#define EG_EDIT_RULE_VALID3_1
	#define EG_EDIT_RULE_VALID3_1_OFFSET  64
	#define EG_EDIT_RULE_VALID3_1_LEN     1
	#define EG_EDIT_RULE_VALID3_1_DEFAULT 0x0
	/*[field] START3_1*/
	#define EG_EDIT_RULE_START3_1
	#define EG_EDIT_RULE_START3_1_OFFSET  65
	#define EG_EDIT_RULE_START3_1_LEN     4
	#define EG_EDIT_RULE_START3_1_DEFAULT 0x0
	/*[field] WIDTH3_1*/
	#define EG_EDIT_RULE_WIDTH3_1
	#define EG_EDIT_RULE_WIDTH3_1_OFFSET  69
	#define EG_EDIT_RULE_WIDTH3_1_LEN     4
	#define EG_EDIT_RULE_WIDTH3_1_DEFAULT 0x0
	/*[field] POS3_1*/
	#define EG_EDIT_RULE_POS3_1
	#define EG_EDIT_RULE_POS3_1_OFFSET  73
	#define EG_EDIT_RULE_POS3_1_LEN     7
	#define EG_EDIT_RULE_POS3_1_DEFAULT 0x0

struct eg_edit_rule {
	a_uint32_t  start2_1_0:4;
	a_uint32_t  valid2_1:1;
	a_uint32_t  pos2_0:7;
	a_uint32_t  width2_0:5;
	a_uint32_t  start2_0:5;
	a_uint32_t  valid2_0:1;
	a_uint32_t  src2:3;
	a_uint32_t  src1:6;

	a_uint32_t  pos3_0:7;
	a_uint32_t  width3_0:4;
	a_uint32_t  start3_0:4;
	a_uint32_t  valid3_0:1;
	a_uint32_t  src3:3;
	a_uint32_t  pos2_1:7;
	a_uint32_t  width2_1:5;
	a_uint32_t  start2_1_1:1;

	a_uint32_t  _reserved0:16;
	a_uint32_t  pos3_1:7;
	a_uint32_t  width3_1:4;
	a_uint32_t  start3_1:4;
	a_uint32_t  valid3_1:1;
};

union eg_edit_rule_u {
	a_uint32_t val[3];
	struct eg_edit_rule bf;
};

/*[table] TL_CNT_TBL*/
#define TL_CNT_TBL
#define TL_CNT_TBL_ADDRESS 0x54000
#define TL_CNT_TBL_NUM     128
#define TL_CNT_TBL_INC     0x10
#define TL_CNT_TBL_TYPE    REG_TYPE_RW
#define TL_CNT_TBL_DEFAULT 0x0
	/*[field] RX_PKT_CNT*/
	#define TL_CNT_TBL_RX_PKT_CNT
	#define TL_CNT_TBL_RX_PKT_CNT_OFFSET  0
	#define TL_CNT_TBL_RX_PKT_CNT_LEN     32
	#define TL_CNT_TBL_RX_PKT_CNT_DEFAULT 0x0
	/*[field] RX_BYTE_CNT*/
	#define TL_CNT_TBL_RX_BYTE_CNT
	#define TL_CNT_TBL_RX_BYTE_CNT_OFFSET  32
	#define TL_CNT_TBL_RX_BYTE_CNT_LEN     40
	#define TL_CNT_TBL_RX_BYTE_CNT_DEFAULT 0x0

struct tl_cnt_tbl {
	a_uint32_t  rx_pkt_cnt:32;
	a_uint32_t  rx_byte_cnt_0:32;

	a_uint32_t  _reserved0:24;
	a_uint32_t  rx_byte_cnt_1:8;
};

union tl_cnt_tbl_u {
	a_uint32_t val[3];
	struct tl_cnt_tbl bf;
};

/*[table] TL_PORT_VP_TBL*/
#define TL_PORT_VP_TBL
#define TL_PORT_VP_TBL_ADDRESS 0x2000
#define TL_PORT_VP_TBL_NUM     256
#define TL_PORT_VP_TBL_INC     0x10
#define TL_PORT_VP_TBL_TYPE    REG_TYPE_RW
#define TL_PORT_VP_TBL_DEFAULT 0x0
	/*[field] VLAN_PROFILE*/
	#define TL_PORT_VP_TBL_VLAN_PROFILE
	#define TL_PORT_VP_TBL_VLAN_PROFILE_OFFSET  0
	#define TL_PORT_VP_TBL_VLAN_PROFILE_LEN     8
	#define TL_PORT_VP_TBL_VLAN_PROFILE_DEFAULT 0x0
	/*[field] PPPOE_PROFILE*/
	#define TL_PORT_VP_TBL_PPPOE_PROFILE
	#define TL_PORT_VP_TBL_PPPOE_PROFILE_OFFSET  8
	#define TL_PORT_VP_TBL_PPPOE_PROFILE_LEN     8
	#define TL_PORT_VP_TBL_PPPOE_PROFILE_DEFAULT 0x0
	/*[field] PRE_IPO_PROFILE*/
	#define TL_PORT_VP_TBL_PRE_IPO_PROFILE
	#define TL_PORT_VP_TBL_PRE_IPO_PROFILE_OFFSET  16
	#define TL_PORT_VP_TBL_PRE_IPO_PROFILE_LEN     8
	#define TL_PORT_VP_TBL_PRE_IPO_PROFILE_DEFAULT 0x0
	/*[field] TL_L3_IF_VALID*/
	#define TL_PORT_VP_TBL_TL_L3_IF_VALID
	#define TL_PORT_VP_TBL_TL_L3_IF_VALID_OFFSET  24
	#define TL_PORT_VP_TBL_TL_L3_IF_VALID_LEN     1
	#define TL_PORT_VP_TBL_TL_L3_IF_VALID_DEFAULT 0x0
	/*[field] TL_L3_IF_INDEX*/
	#define TL_PORT_VP_TBL_TL_L3_IF_INDEX
	#define TL_PORT_VP_TBL_TL_L3_IF_INDEX_OFFSET  25
	#define TL_PORT_VP_TBL_TL_L3_IF_INDEX_LEN     7
	#define TL_PORT_VP_TBL_TL_L3_IF_INDEX_DEFAULT 0x0
	/*[field] MAC_ADDR*/
	#define TL_PORT_VP_TBL_MAC_ADDR
	#define TL_PORT_VP_TBL_MAC_ADDR_OFFSET  32
	#define TL_PORT_VP_TBL_MAC_ADDR_LEN     48
	#define TL_PORT_VP_TBL_MAC_ADDR_DEFAULT 0x0
	/*[field] PPPOE_EN*/
	#define TL_PORT_VP_TBL_PPPOE_EN
	#define TL_PORT_VP_TBL_PPPOE_EN_OFFSET  80
	#define TL_PORT_VP_TBL_PPPOE_EN_LEN     1
	#define TL_PORT_VP_TBL_PPPOE_EN_DEFAULT 0x0
	/*[field] RX_CNT_EN*/
	#define TL_PORT_VP_TBL_RX_CNT_EN
	#define TL_PORT_VP_TBL_RX_CNT_EN_OFFSET  81
	#define TL_PORT_VP_TBL_RX_CNT_EN_LEN     1
	#define TL_PORT_VP_TBL_RX_CNT_EN_DEFAULT 0x0

struct tl_port_vp_tbl {
	a_uint32_t  tl_l3_if_index:7;
	a_uint32_t  tl_l3_if_valid:1;
	a_uint32_t  pre_ipo_profile:8;
	a_uint32_t  pppoe_profile:8;
	a_uint32_t  vlan_profile:8;

	a_uint32_t  mac_addr_0:32;

	a_uint32_t  _reserved0:14;
	a_uint32_t  rx_cnt_en:1;
	a_uint32_t  pppoe_en:1;
	a_uint32_t  mac_addr_1:16;
};

union tl_port_vp_tbl_u {
	a_uint32_t val[3];
	struct tl_port_vp_tbl bf;
};

#ifndef IN_TUNNEL_MINI
/*[table] TL_VLAN_TBL*/
#define TL_VLAN_TBL
#define TL_VLAN_TBL_ADDRESS 0x1000
#define TL_VLAN_TBL_NUM     8
#define TL_VLAN_TBL_INC     0x10
#define TL_VLAN_TBL_TYPE    REG_TYPE_RW
#define TL_VLAN_TBL_DEFAULT 0x0
	/*[field] VALID*/
	#define TL_VLAN_TBL_VALID
	#define TL_VLAN_TBL_VALID_OFFSET  0
	#define TL_VLAN_TBL_VALID_LEN     1
	#define TL_VLAN_TBL_VALID_DEFAULT 0x0
	/*[field] TYPE*/
	#define TL_VLAN_TBL_TYPE_F
	#define TL_VLAN_TBL_TYPE_F_OFFSET  1
	#define TL_VLAN_TBL_TYPE_F_LEN     2
	#define TL_VLAN_TBL_TYPE_F_DEFAULT 0x0
	/*[field] PORT_BITMAP reuse TYPE[1]*/
	#define TL_VLAN_TBL_PORT_BITMAP
	#define TL_VLAN_TBL_PORT_BITMAP_OFFSET  3
	#define TL_VLAN_TBL_PORT_BITMAP_LEN     8
	#define TL_VLAN_TBL_PORT_BITMAP_DEFAULT 0x0
	/*[field] VP_PROFILE reuse TYPE[2]*/
	#define TL_VLAN_TBL_VP_PROFILE
	#define TL_VLAN_TBL_VP_PROFILE_OFFSET  3
	#define TL_VLAN_TBL_VP_PROFILE_LEN     8
	#define TL_VLAN_TBL_VP_PROFILE_DEFAULT 0x0
	/*[field] PORT_VP_ID reuse TYPE[0]*/
	#define TL_VLAN_TBL_PORT_VP_ID
	#define TL_VLAN_TBL_PORT_VP_ID_OFFSET  3
	#define TL_VLAN_TBL_PORT_VP_ID_LEN     8
	#define TL_VLAN_TBL_PORT_VP_ID_DEFAULT 0x0
	/*[field] SKEY_FMT*/
	#define TL_VLAN_TBL_SKEY_FMT
	#define TL_VLAN_TBL_SKEY_FMT_OFFSET  11
	#define TL_VLAN_TBL_SKEY_FMT_LEN     3
	#define TL_VLAN_TBL_SKEY_FMT_DEFAULT 0x0
	/*[field] SKEY_VID_INCL*/
	#define TL_VLAN_TBL_SKEY_VID_INCL
	#define TL_VLAN_TBL_SKEY_VID_INCL_OFFSET  14
	#define TL_VLAN_TBL_SKEY_VID_INCL_LEN     1
	#define TL_VLAN_TBL_SKEY_VID_INCL_DEFAULT 0x0
	/*[field] SKEY_VID*/
	#define TL_VLAN_TBL_SKEY_VID
	#define TL_VLAN_TBL_SKEY_VID_OFFSET  15
	#define TL_VLAN_TBL_SKEY_VID_LEN     12
	#define TL_VLAN_TBL_SKEY_VID_DEFAULT 0x0
	/*[field] CKEY_FMT*/
	#define TL_VLAN_TBL_CKEY_FMT
	#define TL_VLAN_TBL_CKEY_FMT_OFFSET  27
	#define TL_VLAN_TBL_CKEY_FMT_LEN     3
	#define TL_VLAN_TBL_CKEY_FMT_DEFAULT 0x0
	/*[field] CKEY_VID_INCL*/
	#define TL_VLAN_TBL_CKEY_VID_INCL
	#define TL_VLAN_TBL_CKEY_VID_INCL_OFFSET  30
	#define TL_VLAN_TBL_CKEY_VID_INCL_LEN     1
	#define TL_VLAN_TBL_CKEY_VID_INCL_DEFAULT 0x0
	/*[field] CKEY_VID*/
	#define TL_VLAN_TBL_CKEY_VID
	#define TL_VLAN_TBL_CKEY_VID_OFFSET  31
	#define TL_VLAN_TBL_CKEY_VID_LEN     12
	#define TL_VLAN_TBL_CKEY_VID_DEFAULT 0x0
	/*[field] TL_L3_IF_VALID*/
	#define TL_VLAN_TBL_TL_L3_IF_VALID
	#define TL_VLAN_TBL_TL_L3_IF_VALID_OFFSET  43
	#define TL_VLAN_TBL_TL_L3_IF_VALID_LEN     1
	#define TL_VLAN_TBL_TL_L3_IF_VALID_DEFAULT 0x0
	/*[field] TL_L3_IF_INDEX*/
	#define TL_VLAN_TBL_TL_L3_IF_INDEX
	#define TL_VLAN_TBL_TL_L3_IF_INDEX_OFFSET  44
	#define TL_VLAN_TBL_TL_L3_IF_INDEX_LEN     7
	#define TL_VLAN_TBL_TL_L3_IF_INDEX_DEFAULT 0x0
	/*[field] PPPOE_EN*/
	#define TL_VLAN_TBL_PPPOE_EN
	#define TL_VLAN_TBL_PPPOE_EN_OFFSET  51
	#define TL_VLAN_TBL_PPPOE_EN_LEN     1
	#define TL_VLAN_TBL_PPPOE_EN_DEFAULT 0x0

struct tl_vlan_tbl_2 {
	a_uint32_t  ckey_vid_0:1;
	a_uint32_t  ckey_vid_incl:1;
	a_uint32_t  ckey_fmt:3;
	a_uint32_t  skey_vid:12;
	a_uint32_t  skey_vid_incl:1;
	a_uint32_t  skey_fmt:3;
	a_uint32_t  vp_profile:8;
	a_uint32_t  type:2;
	a_uint32_t  valid:1;

	a_uint32_t  _reserved0:12;
	a_uint32_t  pppoe_en:1;
	a_uint32_t  tl_l3_if_index:7;
	a_uint32_t  tl_l3_if_valid:1;
	a_uint32_t  ckey_vid_1:11;
};

struct tl_vlan_tbl_0 {
	a_uint32_t  ckey_vid_0:1;
	a_uint32_t  ckey_vid_incl:1;
	a_uint32_t  ckey_fmt:3;
	a_uint32_t  skey_vid:12;
	a_uint32_t  skey_vid_incl:1;
	a_uint32_t  skey_fmt:3;
	a_uint32_t  port_vp_id:8;
	a_uint32_t  type:2;
	a_uint32_t  valid:1;

	a_uint32_t  _reserved0:12;
	a_uint32_t  pppoe_en:1;
	a_uint32_t  tl_l3_if_index:7;
	a_uint32_t  tl_l3_if_valid:1;
	a_uint32_t  ckey_vid_1:11;
};

struct tl_vlan_tbl_1 {
	a_uint32_t  ckey_vid_0:1;
	a_uint32_t  ckey_vid_incl:1;
	a_uint32_t  ckey_fmt:3;
	a_uint32_t  skey_vid:12;
	a_uint32_t  skey_vid_incl:1;
	a_uint32_t  skey_fmt:3;
	a_uint32_t  port_bitmap:8;
	a_uint32_t  type:2;
	a_uint32_t  valid:1;

	a_uint32_t  _reserved0:12;
	a_uint32_t  pppoe_en:1;
	a_uint32_t  tl_l3_if_index:7;
	a_uint32_t  tl_l3_if_valid:1;
	a_uint32_t  ckey_vid_1:11;
};

union tl_vlan_tbl_u {
	a_uint32_t val[2];
	struct tl_vlan_tbl_0 bf0;
	struct tl_vlan_tbl_1 bf1;
	struct tl_vlan_tbl_2 bf2;
};

/*[register] ECN_MAP_MODE0_0*/
#define ECN_MAP_MODE0_0
#define ECN_MAP_MODE0_0_ADDRESS 0x43c0
#define ECN_MAP_MODE0_0_NUM     1
#define ECN_MAP_MODE0_0_INC     0x4
#define ECN_MAP_MODE0_0_TYPE    REG_TYPE_RW
#define ECN_MAP_MODE0_0_DEFAULT 0x57abff
	/*[field] NEW_ECN*/
	#define ECN_MAP_MODE0_0_NEW_ECN
	#define ECN_MAP_MODE0_0_NEW_ECN_OFFSET  0
	#define ECN_MAP_MODE0_0_NEW_ECN_LEN     32
	#define ECN_MAP_MODE0_0_NEW_ECN_DEFAULT 0x57abff

struct ecn_map_mode0_0 {
	a_uint32_t  new_ecn:32;
};

union ecn_map_mode0_0_u {
	a_uint32_t val;
	struct ecn_map_mode0_0 bf;
};

/*[register] ECN_MAP_MODE0_1*/
#define ECN_MAP_MODE0_1
#define ECN_MAP_MODE0_1_ADDRESS 0x43c4
#define ECN_MAP_MODE0_1_NUM     1
#define ECN_MAP_MODE0_1_INC     0x4
#define ECN_MAP_MODE0_1_TYPE    REG_TYPE_RW
#define ECN_MAP_MODE0_1_DEFAULT 0x1000
	/*[field] EXCEPTION_EN*/
	#define ECN_MAP_MODE0_1_EXCEPTION_EN
	#define ECN_MAP_MODE0_1_EXCEPTION_EN_OFFSET  0
	#define ECN_MAP_MODE0_1_EXCEPTION_EN_LEN     16
	#define ECN_MAP_MODE0_1_EXCEPTION_EN_DEFAULT 0x1000

struct ecn_map_mode0_1 {
	a_uint32_t  _reserved0:16;
	a_uint32_t  exception_en:16;
};

union ecn_map_mode0_1_u {
	a_uint32_t val;
	struct ecn_map_mode0_1 bf;
};

/*[register] ECN_MAP_MODE1_0*/
#define ECN_MAP_MODE1_0
#define ECN_MAP_MODE1_0_ADDRESS 0x43d0
#define ECN_MAP_MODE1_0_NUM     1
#define ECN_MAP_MODE1_0_INC     0x4
#define ECN_MAP_MODE1_0_TYPE    REG_TYPE_RW
#define ECN_MAP_MODE1_0_DEFAULT 0x57abff
	/*[field] NEW_ECN*/
	#define ECN_MAP_MODE1_0_NEW_ECN
	#define ECN_MAP_MODE1_0_NEW_ECN_OFFSET  0
	#define ECN_MAP_MODE1_0_NEW_ECN_LEN     32
	#define ECN_MAP_MODE1_0_NEW_ECN_DEFAULT 0x57abff

struct ecn_map_mode1_0 {
	a_uint32_t  new_ecn:32;
};

union ecn_map_mode1_0_u {
	a_uint32_t val;
	struct ecn_map_mode1_0 bf;
};

/*[register] ECN_MAP_MODE1_1*/
#define ECN_MAP_MODE1_1
#define ECN_MAP_MODE1_1_ADDRESS 0x43d4
#define ECN_MAP_MODE1_1_NUM     1
#define ECN_MAP_MODE1_1_INC     0x4
#define ECN_MAP_MODE1_1_TYPE    REG_TYPE_RW
#define ECN_MAP_MODE1_1_DEFAULT 0x0
	/*[field] EXCEPTION_EN*/
	#define ECN_MAP_MODE1_1_EXCEPTION_EN
	#define ECN_MAP_MODE1_1_EXCEPTION_EN_OFFSET  0
	#define ECN_MAP_MODE1_1_EXCEPTION_EN_LEN     16
	#define ECN_MAP_MODE1_1_EXCEPTION_EN_DEFAULT 0x0

struct ecn_map_mode1_1 {
	a_uint32_t  _reserved0:16;
	a_uint32_t  exception_en:16;
};

union ecn_map_mode1_1_u {
	a_uint32_t val;
	struct ecn_map_mode1_1 bf;
};

/*[register] ECN_MAP_MODE2_0*/
#define ECN_MAP_MODE2_0
#define ECN_MAP_MODE2_0_ADDRESS 0x43e0
#define ECN_MAP_MODE2_0_NUM     1
#define ECN_MAP_MODE2_0_INC     0x4
#define ECN_MAP_MODE2_0_TYPE    REG_TYPE_RW
#define ECN_MAP_MODE2_0_DEFAULT 0x5babff
	/*[field] NEW_ECN*/
	#define ECN_MAP_MODE2_0_NEW_ECN
	#define ECN_MAP_MODE2_0_NEW_ECN_OFFSET  0
	#define ECN_MAP_MODE2_0_NEW_ECN_LEN     32
	#define ECN_MAP_MODE2_0_NEW_ECN_DEFAULT 0x5babff

struct ecn_map_mode2_0 {
	a_uint32_t  new_ecn:32;
};

union ecn_map_mode2_0_u {
	a_uint32_t val;
	struct ecn_map_mode2_0 bf;
};

/*[register] ECN_MAP_MODE2_1*/
#define ECN_MAP_MODE2_1
#define ECN_MAP_MODE2_1_ADDRESS 0x43e4
#define ECN_MAP_MODE2_1_NUM     1
#define ECN_MAP_MODE2_1_INC     0x4
#define ECN_MAP_MODE2_1_TYPE    REG_TYPE_RW
#define ECN_MAP_MODE2_1_DEFAULT 0x1000
	/*[field] EXCEPTION_EN*/
	#define ECN_MAP_MODE2_1_EXCEPTION_EN
	#define ECN_MAP_MODE2_1_EXCEPTION_EN_OFFSET  0
	#define ECN_MAP_MODE2_1_EXCEPTION_EN_LEN     16
	#define ECN_MAP_MODE2_1_EXCEPTION_EN_DEFAULT 0x1000

struct ecn_map_mode2_1 {
	a_uint32_t  _reserved0:16;
	a_uint32_t  exception_en:16;
};

union ecn_map_mode2_1_u {
	a_uint32_t val;
	struct ecn_map_mode2_1 bf;
};
#endif
#endif
