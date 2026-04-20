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
#if defined(CONFIG_CPU_BIG_ENDIAN)
#include "appe_acl_reg_be.h"
#else
#ifndef APPE_ACL_REG_H
#define APPE_ACL_REG_H

/*[register] IPR_UDF_CTRL*/
#define IPR_UDF_CTRL
#define IPR_UDF_CTRL_ADDRESS 0x400
#define IPR_UDF_CTRL_NUM     16
#define IPR_UDF_CTRL_INC     0x4
#define IPR_UDF_CTRL_TYPE    REG_TYPE_RW
#define IPR_UDF_CTRL_DEFAULT 0x0
	/*[field] L3_TYPE*/
	#define IPR_UDF_CTRL_L3_TYPE
	#define IPR_UDF_CTRL_L3_TYPE_OFFSET  0
	#define IPR_UDF_CTRL_L3_TYPE_LEN     2
	#define IPR_UDF_CTRL_L3_TYPE_DEFAULT 0x0
	/*[field] L3_TYPE_INCL*/
	#define IPR_UDF_CTRL_L3_TYPE_INCL
	#define IPR_UDF_CTRL_L3_TYPE_INCL_OFFSET  2
	#define IPR_UDF_CTRL_L3_TYPE_INCL_LEN     1
	#define IPR_UDF_CTRL_L3_TYPE_INCL_DEFAULT 0x0
	/*[field] L4_TYPE*/
	#define IPR_UDF_CTRL_L4_TYPE
	#define IPR_UDF_CTRL_L4_TYPE_OFFSET  4
	#define IPR_UDF_CTRL_L4_TYPE_LEN     3
	#define IPR_UDF_CTRL_L4_TYPE_DEFAULT 0x0
	/*[field] L4_TYPE_INCL*/
	#define IPR_UDF_CTRL_L4_TYPE_INCL
	#define IPR_UDF_CTRL_L4_TYPE_INCL_OFFSET  7
	#define IPR_UDF_CTRL_L4_TYPE_INCL_LEN     1
	#define IPR_UDF_CTRL_L4_TYPE_INCL_DEFAULT 0x0
	/*[field] UDF_PROFILE*/
	#define IPR_UDF_CTRL_UDF_PROFILE
	#define IPR_UDF_CTRL_UDF_PROFILE_OFFSET  8
	#define IPR_UDF_CTRL_UDF_PROFILE_LEN     3
	#define IPR_UDF_CTRL_UDF_PROFILE_DEFAULT 0x0
	/*[field] VALID*/
	#define IPR_UDF_CTRL_VALID
	#define IPR_UDF_CTRL_VALID_OFFSET  31
	#define IPR_UDF_CTRL_VALID_LEN     1
	#define IPR_UDF_CTRL_VALID_DEFAULT 0x0

struct ipr_udf_ctrl {
	a_uint32_t  l3_type:2;
	a_uint32_t  l3_type_incl:1;
	a_uint32_t  _reserved0:1;
	a_uint32_t  l4_type:3;
	a_uint32_t  l4_type_incl:1;
	a_uint32_t  udf_profile:3;
	a_uint32_t  _reserved1:20;
	a_uint32_t  valid:1;
};

union ipr_udf_ctrl_u {
	a_uint32_t val;
	struct ipr_udf_ctrl bf;
};

/*[register] IPR_UDF_PROFILE_BASE*/
#define IPR_UDF_PROFILE_BASE
#define IPR_UDF_PROFILE_BASE_ADDRESS 0x440
#define IPR_UDF_PROFILE_BASE_NUM     8
#define IPR_UDF_PROFILE_BASE_INC     0x4
#define IPR_UDF_PROFILE_BASE_TYPE    REG_TYPE_RW
#define IPR_UDF_PROFILE_BASE_DEFAULT 0x0
	/*[field] UDF0_BASE*/
	#define IPR_UDF_PROFILE_BASE_UDF0_BASE
	#define IPR_UDF_PROFILE_BASE_UDF0_BASE_OFFSET  0
	#define IPR_UDF_PROFILE_BASE_UDF0_BASE_LEN     2
	#define IPR_UDF_PROFILE_BASE_UDF0_BASE_DEFAULT 0x0
	/*[field] UDF1_BASE*/
	#define IPR_UDF_PROFILE_BASE_UDF1_BASE
	#define IPR_UDF_PROFILE_BASE_UDF1_BASE_OFFSET  8
	#define IPR_UDF_PROFILE_BASE_UDF1_BASE_LEN     3
	#define IPR_UDF_PROFILE_BASE_UDF1_BASE_DEFAULT 0x0
	/*[field] UDF2_BASE*/
	#define IPR_UDF_PROFILE_BASE_UDF2_BASE
	#define IPR_UDF_PROFILE_BASE_UDF2_BASE_OFFSET  16
	#define IPR_UDF_PROFILE_BASE_UDF2_BASE_LEN     3
	#define IPR_UDF_PROFILE_BASE_UDF2_BASE_DEFAULT 0x0
	/*[field] UDF3_BASE*/
	#define IPR_UDF_PROFILE_BASE_UDF3_BASE
	#define IPR_UDF_PROFILE_BASE_UDF3_BASE_OFFSET  24
	#define IPR_UDF_PROFILE_BASE_UDF3_BASE_LEN     3
	#define IPR_UDF_PROFILE_BASE_UDF3_BASE_DEFAULT 0x0

struct ipr_udf_profile_base {
	a_uint32_t  udf0_base:2;
	a_uint32_t  _reserved0:6;
	a_uint32_t  udf1_base:3;
	a_uint32_t  _reserved1:5;
	a_uint32_t  udf2_base:3;
	a_uint32_t  _reserved2:5;
	a_uint32_t  udf3_base:3;
	a_uint32_t  _reserved3:5;
};

union ipr_udf_profile_base_u {
	a_uint32_t val;
	struct ipr_udf_profile_base bf;
};

/*[register] IPR_UDF_PROFILE_OFFSET*/
#define IPR_UDF_PROFILE_OFFSET
#define IPR_UDF_PROFILE_OFFSET_ADDRESS 0x460
#define IPR_UDF_PROFILE_OFFSET_NUM     8
#define IPR_UDF_PROFILE_OFFSET_INC     0x4
#define IPR_UDF_PROFILE_OFFSET_TYPE    REG_TYPE_RW
#define IPR_UDF_PROFILE_OFFSET_DEFAULT 0x0
	/*[field] UDF0_OFFSET*/
	#define IPR_UDF_PROFILE_OFFSET_UDF0_OFFSET
	#define IPR_UDF_PROFILE_OFFSET_UDF0_OFFSET_OFFSET  0
	#define IPR_UDF_PROFILE_OFFSET_UDF0_OFFSET_LEN     6
	#define IPR_UDF_PROFILE_OFFSET_UDF0_OFFSET_DEFAULT 0x0
	/*[field] UDF1_OFFSET*/
	#define IPR_UDF_PROFILE_OFFSET_UDF1_OFFSET
	#define IPR_UDF_PROFILE_OFFSET_UDF1_OFFSET_OFFSET  8
	#define IPR_UDF_PROFILE_OFFSET_UDF1_OFFSET_LEN     6
	#define IPR_UDF_PROFILE_OFFSET_UDF1_OFFSET_DEFAULT 0x0
	/*[field] UDF2_OFFSET*/
	#define IPR_UDF_PROFILE_OFFSET_UDF2_OFFSET
	#define IPR_UDF_PROFILE_OFFSET_UDF2_OFFSET_OFFSET  16
	#define IPR_UDF_PROFILE_OFFSET_UDF2_OFFSET_LEN     6
	#define IPR_UDF_PROFILE_OFFSET_UDF2_OFFSET_DEFAULT 0x0
	/*[field] UDF3_OFFSET*/
	#define IPR_UDF_PROFILE_OFFSET_UDF3_OFFSET
	#define IPR_UDF_PROFILE_OFFSET_UDF3_OFFSET_OFFSET  24
	#define IPR_UDF_PROFILE_OFFSET_UDF3_OFFSET_LEN     6
	#define IPR_UDF_PROFILE_OFFSET_UDF3_OFFSET_DEFAULT 0x0

struct ipr_udf_profile_offset {
	a_uint32_t  udf0_offset:6;
	a_uint32_t  _reserved0:2;
	a_uint32_t  udf1_offset:6;
	a_uint32_t  _reserved1:2;
	a_uint32_t  udf2_offset:6;
	a_uint32_t  _reserved2:2;
	a_uint32_t  udf3_offset:6;
	a_uint32_t  _reserved3:2;
};

union ipr_udf_profile_offset_u {
	a_uint32_t val;
	struct ipr_udf_profile_offset bf;
};

/*[table] EG_IPO_EXT_TBL*/
#define EG_IPO_EXT_TBL
#define EG_IPO_EXT_TBL_ADDRESS 0x2c000
#if defined(MPPE)
#define EG_IPO_EXT_TBL_NUM     256
#else
#define EG_IPO_EXT_TBL_NUM     1024
#endif
#define EG_IPO_EXT_TBL_INC     0x10
#define EG_IPO_EXT_TBL_TYPE    REG_TYPE_RW
#define EG_IPO_EXT_TBL_DEFAULT 0x0
	/*[field] POLICY_ID*/
	#define EG_IPO_EXT_TBL_POLICY_ID
	#define EG_IPO_EXT_TBL_POLICY_ID_OFFSET  0
	#define EG_IPO_EXT_TBL_POLICY_ID_LEN     16
	#define EG_IPO_EXT_TBL_POLICY_ID_DEFAULT 0x0
#if defined(MPPE)
	/*[field] COOKIE*/
	#define EG_IPO_EXT_TBL_COOKIE
	#define EG_IPO_EXT_TBL_COOKIE_OFFSET  16
	#define EG_IPO_EXT_TBL_COOKIE_LEN     16
	#define EG_IPO_EXT_TBL_COOKIE_DEFAULT 0x0
	/*[field] COOKIE_PRI*/
	#define EG_IPO_EXT_TBL_COOKIE_PRI
	#define EG_IPO_EXT_TBL_COOKIE_PRI_OFFSET  32
	#define EG_IPO_EXT_TBL_COOKIE_PRI_LEN     4
	#define EG_IPO_EXT_TBL_COOKIE_PRI_DEFAULT 0x0
#endif

struct eg_ipo_ext_tbl {
	a_uint32_t  policy_id:16;
#if defined(MPPE)
	a_uint32_t  cookie:16;
	a_uint32_t  cookie_pri:4;
	a_uint32_t  _reserved0:28;
#else
	a_uint32_t  _reserved0:16;
#endif
};

union eg_ipo_ext_tbl_u {
#if defined(MPPE)
	a_uint32_t val[2];
#else
	a_uint32_t val[1];
#endif
	struct eg_ipo_ext_tbl bf;
};

/*[table] PRE_IPO_RULE_REG*/
#define PRE_IPO_RULE_REG
#define PRE_IPO_RULE_REG_ADDRESS 0x0
#if defined(MPPE)
#define PRE_IPO_RULE_REG_NUM     128
#else
#define PRE_IPO_RULE_REG_NUM     512
#endif
#define PRE_IPO_RULE_REG_INC     0x10
#define PRE_IPO_RULE_REG_TYPE    REG_TYPE_RW
#define PRE_IPO_RULE_REG_DEFAULT 0x0
	/*[field] RULE_FIELD*/
	#define PRE_IPO_RULE_REG_RULE_FIELD
	#define PRE_IPO_RULE_REG_RULE_FIELD_OFFSET  0
	#define PRE_IPO_RULE_REG_RULE_FIELD_LEN     52
	#define PRE_IPO_RULE_REG_RULE_FIELD_DEFAULT 0x0
	/*[field] FAKE_MAC_HEADER*/
	#define PRE_IPO_RULE_REG_FAKE_MAC_HEADER
	#define PRE_IPO_RULE_REG_FAKE_MAC_HEADER_OFFSET  52
	#define PRE_IPO_RULE_REG_FAKE_MAC_HEADER_LEN     1
	#define PRE_IPO_RULE_REG_FAKE_MAC_HEADER_DEFAULT 0x0
	/*[field] RANGE_EN*/
	#define PRE_IPO_RULE_REG_RANGE_EN
	#define PRE_IPO_RULE_REG_RANGE_EN_OFFSET  53
	#define PRE_IPO_RULE_REG_RANGE_EN_LEN     1
	#define PRE_IPO_RULE_REG_RANGE_EN_DEFAULT 0x0
	/*[field] INVERSE_EN*/
	#define PRE_IPO_RULE_REG_INVERSE_EN
	#define PRE_IPO_RULE_REG_INVERSE_EN_OFFSET  54
	#define PRE_IPO_RULE_REG_INVERSE_EN_LEN     1
	#define PRE_IPO_RULE_REG_INVERSE_EN_DEFAULT 0x0
	/*[field] RULE_TYPE*/
	#define PRE_IPO_RULE_REG_RULE_TYPE
	#define PRE_IPO_RULE_REG_RULE_TYPE_OFFSET  55
	#define PRE_IPO_RULE_REG_RULE_TYPE_LEN     5
	#define PRE_IPO_RULE_REG_RULE_TYPE_DEFAULT 0x0
	/*[field] SRC_TYPE*/
	#define PRE_IPO_RULE_REG_SRC_TYPE
	#define PRE_IPO_RULE_REG_SRC_TYPE_OFFSET  60
	#define PRE_IPO_RULE_REG_SRC_TYPE_LEN     3
	#define PRE_IPO_RULE_REG_SRC_TYPE_DEFAULT 0x0
	/*[field] SRC*/
	#define PRE_IPO_RULE_REG_SRC
	#define PRE_IPO_RULE_REG_SRC_OFFSET  63
	#define PRE_IPO_RULE_REG_SRC_LEN     8
	#define PRE_IPO_RULE_REG_SRC_DEFAULT 0x0
	/*[field] PRI*/
	#define PRE_IPO_RULE_REG_PRI
	#define PRE_IPO_RULE_REG_PRI_OFFSET  71
	#define PRE_IPO_RULE_REG_PRI_LEN     9
	#define PRE_IPO_RULE_REG_PRI_DEFAULT 0x0
	/*[field] RES_CHAIN*/
	#define PRE_IPO_RULE_REG_RES_CHAIN
	#define PRE_IPO_RULE_REG_RES_CHAIN_OFFSET  80
	#define PRE_IPO_RULE_REG_RES_CHAIN_LEN     1
	#define PRE_IPO_RULE_REG_RES_CHAIN_DEFAULT 0x0
	/*[field] INNER_OUTER_SEL*/
	#define PRE_IPO_RULE_REG_INNER_OUTER_SEL
	#define PRE_IPO_RULE_REG_INNER_OUTER_SEL_OFFSET  81
	#define PRE_IPO_RULE_REG_INNER_OUTER_SEL_LEN     1
	#define PRE_IPO_RULE_REG_INNER_OUTER_SEL_DEFAULT 0x0

struct pre_ipo_rule_reg {
	a_uint32_t  rule_field_0:32;
	a_uint32_t  rule_field_1:20;
	a_uint32_t  fake_mac_header:1;
	a_uint32_t  range_en:1;
	a_uint32_t  inverse_en:1;
	a_uint32_t  rule_type:5;
	a_uint32_t  src_type:3;
	a_uint32_t  src_0:1;
	a_uint32_t  src_1:7;
	a_uint32_t  pri:9;
	a_uint32_t  res_chain:1;
	a_uint32_t  inner_outer_sel:1;
	a_uint32_t  _reserved0:14;
};

union pre_ipo_rule_reg_u {
	a_uint32_t val[3];
	struct pre_ipo_rule_reg bf;
};

/*[table] PRE_IPO_MASK_REG*/
#define PRE_IPO_MASK_REG
#define PRE_IPO_MASK_REG_ADDRESS 0x2000
#if defined(MPPE)
#define PRE_IPO_MASK_REG_NUM     128
#else
#define PRE_IPO_MASK_REG_NUM     512
#endif
#define PRE_IPO_MASK_REG_INC     0x10
#define PRE_IPO_MASK_REG_TYPE    REG_TYPE_RW
#define PRE_IPO_MASK_REG_DEFAULT 0x0
	/*[field] MASKFIELD*/
	#define PRE_IPO_MASK_REG_MASKFIELD
	#define PRE_IPO_MASK_REG_MASKFIELD_OFFSET  0
	#define PRE_IPO_MASK_REG_MASKFIELD_LEN     53
	#define PRE_IPO_MASK_REG_MASKFIELD_DEFAULT 0x0

struct pre_ipo_mask_reg {
	a_uint32_t  maskfield_0:32;
	a_uint32_t  maskfield_1:21;
	a_uint32_t  _reserved0:11;
};

union pre_ipo_mask_reg_u {
	a_uint32_t val[2];
	struct pre_ipo_mask_reg bf;
};

/*[register] PRE_IPO_RULE_EXT_1*/
#define PRE_IPO_RULE_EXT_1
#define PRE_IPO_RULE_EXT_1_ADDRESS 0x4000
#if defined(MPPE)
#define PRE_IPO_RULE_EXT_1_NUM     16
#else
#define PRE_IPO_RULE_EXT_1_NUM     64
#endif
#define PRE_IPO_RULE_EXT_1_INC     0x4
#define PRE_IPO_RULE_EXT_1_TYPE    REG_TYPE_RW
#define PRE_IPO_RULE_EXT_1_DEFAULT 0x0
	/*[field] EXT2_0*/
	#define PRE_IPO_RULE_EXT_1_EXT2_0
	#define PRE_IPO_RULE_EXT_1_EXT2_0_OFFSET  0
	#define PRE_IPO_RULE_EXT_1_EXT2_0_LEN     1
	#define PRE_IPO_RULE_EXT_1_EXT2_0_DEFAULT 0x0
	/*[field] EXT2_1*/
	#define PRE_IPO_RULE_EXT_1_EXT2_1
	#define PRE_IPO_RULE_EXT_1_EXT2_1_OFFSET  1
	#define PRE_IPO_RULE_EXT_1_EXT2_1_LEN     1
	#define PRE_IPO_RULE_EXT_1_EXT2_1_DEFAULT 0x0
	/*[field] EXT2_2*/
	#define PRE_IPO_RULE_EXT_1_EXT2_2
	#define PRE_IPO_RULE_EXT_1_EXT2_2_OFFSET  2
	#define PRE_IPO_RULE_EXT_1_EXT2_2_LEN     1
	#define PRE_IPO_RULE_EXT_1_EXT2_2_DEFAULT 0x0
	/*[field] EXT2_3*/
	#define PRE_IPO_RULE_EXT_1_EXT2_3
	#define PRE_IPO_RULE_EXT_1_EXT2_3_OFFSET  3
	#define PRE_IPO_RULE_EXT_1_EXT2_3_LEN     1
	#define PRE_IPO_RULE_EXT_1_EXT2_3_DEFAULT 0x0

struct pre_ipo_rule_ext_1 {
	a_uint32_t  ext2_0:1;
	a_uint32_t  ext2_1:1;
	a_uint32_t  ext2_2:1;
	a_uint32_t  ext2_3:1;
	a_uint32_t  _reserved0:28;
};

union pre_ipo_rule_ext_1_u {
	a_uint32_t val;
	struct pre_ipo_rule_ext_1 bf;
};

/*[register] PRE_IPO_RULE_EXT_2*/
#define PRE_IPO_RULE_EXT_2
#define PRE_IPO_RULE_EXT_2_ADDRESS 0x4100
#if defined(MPPE)
#define PRE_IPO_RULE_EXT_2_NUM     16
#else
#define PRE_IPO_RULE_EXT_2_NUM     64
#endif
#define PRE_IPO_RULE_EXT_2_INC     0x4
#define PRE_IPO_RULE_EXT_2_TYPE    REG_TYPE_RW
#define PRE_IPO_RULE_EXT_2_DEFAULT 0x0
	/*[field] EXT4_0*/
	#define PRE_IPO_RULE_EXT_2_EXT4_0
	#define PRE_IPO_RULE_EXT_2_EXT4_0_OFFSET  0
	#define PRE_IPO_RULE_EXT_2_EXT4_0_LEN     1
	#define PRE_IPO_RULE_EXT_2_EXT4_0_DEFAULT 0x0
	/*[field] EXT4_1*/
	#define PRE_IPO_RULE_EXT_2_EXT4_1
	#define PRE_IPO_RULE_EXT_2_EXT4_1_OFFSET  1
	#define PRE_IPO_RULE_EXT_2_EXT4_1_LEN     1
	#define PRE_IPO_RULE_EXT_2_EXT4_1_DEFAULT 0x0

struct pre_ipo_rule_ext_2 {
	a_uint32_t  ext4_0:1;
	a_uint32_t  ext4_1:1;
	a_uint32_t  _reserved0:30;
};

union pre_ipo_rule_ext_2_u {
	a_uint32_t val;
	struct pre_ipo_rule_ext_2 bf;
};

/*[register] PRE_IPO_RULE_EXT_4*/
#define PRE_IPO_RULE_EXT_4
#define PRE_IPO_RULE_EXT_4_ADDRESS 0x4200
#if defined(MPPE)
#define PRE_IPO_RULE_EXT_4_NUM     16
#else
#define PRE_IPO_RULE_EXT_4_NUM     64
#endif
#define PRE_IPO_RULE_EXT_4_INC     0x4
#define PRE_IPO_RULE_EXT_4_TYPE    REG_TYPE_RW
#define PRE_IPO_RULE_EXT_4_DEFAULT 0x0
	/*[field] EXT8*/
	#define PRE_IPO_RULE_EXT_4_EXT8
	#define PRE_IPO_RULE_EXT_4_EXT8_OFFSET  0
	#define PRE_IPO_RULE_EXT_4_EXT8_LEN     1
	#define PRE_IPO_RULE_EXT_4_EXT8_DEFAULT 0x0

struct pre_ipo_rule_ext_4 {
	a_uint32_t  ext8:1;
	a_uint32_t  _reserved0:31;
};

union pre_ipo_rule_ext_4_u {
	a_uint32_t val;
	struct pre_ipo_rule_ext_4 bf;
};

/*[register] PRE_IPO_DBG_ADDR*/
#define PRE_IPO_DBG_ADDR
#define PRE_IPO_DBG_ADDR_ADDRESS 0x4300
#define PRE_IPO_DBG_ADDR_NUM     1
#define PRE_IPO_DBG_ADDR_INC     0x4
#define PRE_IPO_DBG_ADDR_TYPE    REG_TYPE_RW
#define PRE_IPO_DBG_ADDR_DEFAULT 0x0
	/*[field] IPO_DBG_ADDR*/
	#define PRE_IPO_DBG_ADDR_IPO_DBG_ADDR
	#define PRE_IPO_DBG_ADDR_IPO_DBG_ADDR_OFFSET  0
	#define PRE_IPO_DBG_ADDR_IPO_DBG_ADDR_LEN     32
	#define PRE_IPO_DBG_ADDR_IPO_DBG_ADDR_DEFAULT 0x0

struct pre_ipo_dbg_addr {
	a_uint32_t  ipo_dbg_addr:32;
};

union pre_ipo_dbg_addr_u {
	a_uint32_t val;
	struct pre_ipo_dbg_addr bf;
};

/*[register] PRE_IPO_DBG_DATA*/
#define PRE_IPO_DBG_DATA
#define PRE_IPO_DBG_DATA_ADDRESS 0x4304
#define PRE_IPO_DBG_DATA_NUM     1
#define PRE_IPO_DBG_DATA_INC     0x4
#define PRE_IPO_DBG_DATA_TYPE    REG_TYPE_RO
#define PRE_IPO_DBG_DATA_DEFAULT 0x0
	/*[field] IPO_DBG_DATA*/
	#define PRE_IPO_DBG_DATA_IPO_DBG_DATA
	#define PRE_IPO_DBG_DATA_IPO_DBG_DATA_OFFSET  0
	#define PRE_IPO_DBG_DATA_IPO_DBG_DATA_LEN     32
	#define PRE_IPO_DBG_DATA_IPO_DBG_DATA_DEFAULT 0x0

struct pre_ipo_dbg_data {
	a_uint32_t  ipo_dbg_data:32;
};

union pre_ipo_dbg_data_u {
	a_uint32_t val;
	struct pre_ipo_dbg_data bf;
};

/*[register] PRE_IPO_SPARE_REG*/
#define PRE_IPO_SPARE_REG
#define PRE_IPO_SPARE_REG_ADDRESS 0x4308
#define PRE_IPO_SPARE_REG_NUM     1
#define PRE_IPO_SPARE_REG_INC     0x4
#define PRE_IPO_SPARE_REG_TYPE    REG_TYPE_RW
#define PRE_IPO_SPARE_REG_DEFAULT 0x0
	/*[field] SPARE_REG*/
	#define PRE_IPO_SPARE_REG_SPARE_REG
	#define PRE_IPO_SPARE_REG_SPARE_REG_OFFSET  0
	#define PRE_IPO_SPARE_REG_SPARE_REG_LEN     32
	#define PRE_IPO_SPARE_REG_SPARE_REG_DEFAULT 0x0

struct pre_ipo_spare_reg {
	a_uint32_t  spare_reg:32;
};

union pre_ipo_spare_reg_u {
	a_uint32_t val;
	struct pre_ipo_spare_reg bf;
};

/*[register] PRE_IPO_GLB_HIT_COUNTER*/
#define PRE_IPO_GLB_HIT_COUNTER
#define PRE_IPO_GLB_HIT_COUNTER_ADDRESS 0x430c
#define PRE_IPO_GLB_HIT_COUNTER_NUM     1
#define PRE_IPO_GLB_HIT_COUNTER_INC     0x4
#define PRE_IPO_GLB_HIT_COUNTER_TYPE    REG_TYPE_RW
#define PRE_IPO_GLB_HIT_COUNTER_DEFAULT 0x0
	/*[field] HIT_COUNT*/
	#define PRE_IPO_GLB_HIT_COUNTER_HIT_COUNT
	#define PRE_IPO_GLB_HIT_COUNTER_HIT_COUNT_OFFSET  0
	#define PRE_IPO_GLB_HIT_COUNTER_HIT_COUNT_LEN     32
	#define PRE_IPO_GLB_HIT_COUNTER_HIT_COUNT_DEFAULT 0x0

struct pre_ipo_glb_hit_counter {
	a_uint32_t  hit_count:32;
};

union pre_ipo_glb_hit_counter_u {
	a_uint32_t val;
	struct pre_ipo_glb_hit_counter bf;
};

/*[register] PRE_IPO_GLB_MISS_COUNTER*/
#define PRE_IPO_GLB_MISS_COUNTER
#define PRE_IPO_GLB_MISS_COUNTER_ADDRESS 0x4310
#define PRE_IPO_GLB_MISS_COUNTER_NUM     1
#define PRE_IPO_GLB_MISS_COUNTER_INC     0x4
#define PRE_IPO_GLB_MISS_COUNTER_TYPE    REG_TYPE_RW
#define PRE_IPO_GLB_MISS_COUNTER_DEFAULT 0x0
	/*[field] MISS_COUNT*/
	#define PRE_IPO_GLB_MISS_COUNTER_MISS_COUNT
	#define PRE_IPO_GLB_MISS_COUNTER_MISS_COUNT_OFFSET  0
	#define PRE_IPO_GLB_MISS_COUNTER_MISS_COUNT_LEN     32
	#define PRE_IPO_GLB_MISS_COUNTER_MISS_COUNT_DEFAULT 0x0

struct pre_ipo_glb_miss_counter {
	a_uint32_t  miss_count:32;
};

union pre_ipo_glb_miss_counter_u {
	a_uint32_t val;
	struct pre_ipo_glb_miss_counter bf;
};

/*[register] PRE_IPO_GLB_BYPASS_COUNTER*/
#define PRE_IPO_GLB_BYPASS_COUNTER
#define PRE_IPO_GLB_BYPASS_COUNTER_ADDRESS 0x4314
#define PRE_IPO_GLB_BYPASS_COUNTER_NUM     1
#define PRE_IPO_GLB_BYPASS_COUNTER_INC     0x4
#define PRE_IPO_GLB_BYPASS_COUNTER_TYPE    REG_TYPE_RW
#define PRE_IPO_GLB_BYPASS_COUNTER_DEFAULT 0x0
	/*[field] BYPASS_COUNT*/
	#define PRE_IPO_GLB_BYPASS_COUNTER_BYPASS_COUNT
	#define PRE_IPO_GLB_BYPASS_COUNTER_BYPASS_COUNT_OFFSET  0
	#define PRE_IPO_GLB_BYPASS_COUNTER_BYPASS_COUNT_LEN     32
	#define PRE_IPO_GLB_BYPASS_COUNTER_BYPASS_COUNT_DEFAULT 0x0

struct pre_ipo_glb_bypass_counter {
	a_uint32_t  bypass_count:32;
};

union pre_ipo_glb_bypass_counter_u {
	a_uint32_t val;
	struct pre_ipo_glb_bypass_counter bf;
};

/*[table] PRE_IPO_CNT_TBL*/
#define PRE_IPO_CNT_TBL
#define PRE_IPO_CNT_TBL_ADDRESS 0x76000
#if defined(MPPE)
#define PRE_IPO_CNT_TBL_NUM     128
#else
#define PRE_IPO_CNT_TBL_NUM     512
#endif
#define PRE_IPO_CNT_TBL_INC     0x10
#define PRE_IPO_CNT_TBL_TYPE    REG_TYPE_RW
#define PRE_IPO_CNT_TBL_DEFAULT 0x0
	/*[field] HIT_PKT_CNT*/
	#define PRE_IPO_CNT_TBL_HIT_PKT_CNT
	#define PRE_IPO_CNT_TBL_HIT_PKT_CNT_OFFSET  0
	#define PRE_IPO_CNT_TBL_HIT_PKT_CNT_LEN     32
	#define PRE_IPO_CNT_TBL_HIT_PKT_CNT_DEFAULT 0x0
	/*[field] HIT_BYTE_CNT*/
	#define PRE_IPO_CNT_TBL_HIT_BYTE_CNT
	#define PRE_IPO_CNT_TBL_HIT_BYTE_CNT_OFFSET  32
	#define PRE_IPO_CNT_TBL_HIT_BYTE_CNT_LEN     40
	#define PRE_IPO_CNT_TBL_HIT_BYTE_CNT_DEFAULT 0x0

struct pre_ipo_cnt_tbl {
	a_uint32_t  hit_pkt_cnt:32;
	a_uint32_t  hit_byte_cnt_0:32;
	a_uint32_t  hit_byte_cnt_1:8;
	a_uint32_t  _reserved0:24;
};

union pre_ipo_cnt_tbl_u {
	a_uint32_t val[3];
	struct pre_ipo_cnt_tbl bf;
};

/*[table] PRE_IPO_ACTION*/
#define PRE_IPO_ACTION
#define PRE_IPO_ACTION_ADDRESS 0x8000
#if defined(MPPE)
#define PRE_IPO_ACTION_NUM     128
#else
#define PRE_IPO_ACTION_NUM     512
#endif
#define PRE_IPO_ACTION_INC     0x40
#define PRE_IPO_ACTION_TYPE    REG_TYPE_RW
#define PRE_IPO_ACTION_DEFAULT 0x0
	/*[field] DEST_INFO_CHANGE_EN*/
	#define PRE_IPO_ACTION_DEST_INFO_CHANGE_EN
	#define PRE_IPO_ACTION_DEST_INFO_CHANGE_EN_OFFSET  0
	#define PRE_IPO_ACTION_DEST_INFO_CHANGE_EN_LEN     1
	#define PRE_IPO_ACTION_DEST_INFO_CHANGE_EN_DEFAULT 0x0
	/*[field] FWD_CMD*/
	#define PRE_IPO_ACTION_FWD_CMD
	#define PRE_IPO_ACTION_FWD_CMD_OFFSET  1
	#define PRE_IPO_ACTION_FWD_CMD_LEN     2
	#define PRE_IPO_ACTION_FWD_CMD_DEFAULT 0x0
	/*[field] DEST_INFO*/
	#define PRE_IPO_ACTION_DEST_INFO
	#define PRE_IPO_ACTION_DEST_INFO_OFFSET  3
	#define PRE_IPO_ACTION_DEST_INFO_LEN     14
	#define PRE_IPO_ACTION_DEST_INFO_DEFAULT 0x0
	/*[field] MIRROR_EN*/
	#define PRE_IPO_ACTION_MIRROR_EN
	#define PRE_IPO_ACTION_MIRROR_EN_OFFSET  17
	#define PRE_IPO_ACTION_MIRROR_EN_LEN     1
	#define PRE_IPO_ACTION_MIRROR_EN_DEFAULT 0x0
	/*[field] SVID_CHANGE_EN*/
	#define PRE_IPO_ACTION_SVID_CHANGE_EN
	#define PRE_IPO_ACTION_SVID_CHANGE_EN_OFFSET  18
	#define PRE_IPO_ACTION_SVID_CHANGE_EN_LEN     1
	#define PRE_IPO_ACTION_SVID_CHANGE_EN_DEFAULT 0x0
	/*[field] STAG_FMT*/
	#define PRE_IPO_ACTION_STAG_FMT
	#define PRE_IPO_ACTION_STAG_FMT_OFFSET  19
	#define PRE_IPO_ACTION_STAG_FMT_LEN     1
	#define PRE_IPO_ACTION_STAG_FMT_DEFAULT 0x0
	/*[field] SVID*/
	#define PRE_IPO_ACTION_SVID
	#define PRE_IPO_ACTION_SVID_OFFSET  20
	#define PRE_IPO_ACTION_SVID_LEN     12
	#define PRE_IPO_ACTION_SVID_DEFAULT 0x0
	/*[field] CVID_CHANGE_EN*/
	#define PRE_IPO_ACTION_CVID_CHANGE_EN
	#define PRE_IPO_ACTION_CVID_CHANGE_EN_OFFSET  32
	#define PRE_IPO_ACTION_CVID_CHANGE_EN_LEN     1
	#define PRE_IPO_ACTION_CVID_CHANGE_EN_DEFAULT 0x0
	/*[field] CTAG_FMT*/
	#define PRE_IPO_ACTION_CTAG_FMT
	#define PRE_IPO_ACTION_CTAG_FMT_OFFSET  33
	#define PRE_IPO_ACTION_CTAG_FMT_LEN     1
	#define PRE_IPO_ACTION_CTAG_FMT_DEFAULT 0x0
	/*[field] CVID*/
	#define PRE_IPO_ACTION_CVID
	#define PRE_IPO_ACTION_CVID_OFFSET  34
	#define PRE_IPO_ACTION_CVID_LEN     12
	#define PRE_IPO_ACTION_CVID_DEFAULT 0x0
	/*[field] DSCP_TC_CHANGE_EN*/
	#define PRE_IPO_ACTION_DSCP_TC_CHANGE_EN
	#define PRE_IPO_ACTION_DSCP_TC_CHANGE_EN_OFFSET  46
	#define PRE_IPO_ACTION_DSCP_TC_CHANGE_EN_LEN     1
	#define PRE_IPO_ACTION_DSCP_TC_CHANGE_EN_DEFAULT 0x0
	/*[field] DSCP_TC*/
	#define PRE_IPO_ACTION_DSCP_TC
	#define PRE_IPO_ACTION_DSCP_TC_OFFSET  47
	#define PRE_IPO_ACTION_DSCP_TC_LEN     8
	#define PRE_IPO_ACTION_DSCP_TC_DEFAULT 0x0
	/*[field] STAG_PCP_CHANGE_EN*/
	#define PRE_IPO_ACTION_STAG_PCP_CHANGE_EN
	#define PRE_IPO_ACTION_STAG_PCP_CHANGE_EN_OFFSET  55
	#define PRE_IPO_ACTION_STAG_PCP_CHANGE_EN_LEN     1
	#define PRE_IPO_ACTION_STAG_PCP_CHANGE_EN_DEFAULT 0x0
	/*[field] STAG_PCP*/
	#define PRE_IPO_ACTION_STAG_PCP
	#define PRE_IPO_ACTION_STAG_PCP_OFFSET  56
	#define PRE_IPO_ACTION_STAG_PCP_LEN     3
	#define PRE_IPO_ACTION_STAG_PCP_DEFAULT 0x0
	/*[field] STAG_DEI_CHANGE_EN*/
	#define PRE_IPO_ACTION_STAG_DEI_CHANGE_EN
	#define PRE_IPO_ACTION_STAG_DEI_CHANGE_EN_OFFSET  59
	#define PRE_IPO_ACTION_STAG_DEI_CHANGE_EN_LEN     1
	#define PRE_IPO_ACTION_STAG_DEI_CHANGE_EN_DEFAULT 0x0
	/*[field] STAG_DEI*/
	#define PRE_IPO_ACTION_STAG_DEI
	#define PRE_IPO_ACTION_STAG_DEI_OFFSET  60
	#define PRE_IPO_ACTION_STAG_DEI_LEN     1
	#define PRE_IPO_ACTION_STAG_DEI_DEFAULT 0x0
	/*[field] CTAG_PCP_CHANGE_EN*/
	#define PRE_IPO_ACTION_CTAG_PCP_CHANGE_EN
	#define PRE_IPO_ACTION_CTAG_PCP_CHANGE_EN_OFFSET  61
	#define PRE_IPO_ACTION_CTAG_PCP_CHANGE_EN_LEN     1
	#define PRE_IPO_ACTION_CTAG_PCP_CHANGE_EN_DEFAULT 0x0
	/*[field] CTAG_PCP*/
	#define PRE_IPO_ACTION_CTAG_PCP
	#define PRE_IPO_ACTION_CTAG_PCP_OFFSET  62
	#define PRE_IPO_ACTION_CTAG_PCP_LEN     3
	#define PRE_IPO_ACTION_CTAG_PCP_DEFAULT 0x0
	/*[field] CTAG_DEI_CHANGE_EN*/
	#define PRE_IPO_ACTION_CTAG_DEI_CHANGE_EN
	#define PRE_IPO_ACTION_CTAG_DEI_CHANGE_EN_OFFSET  65
	#define PRE_IPO_ACTION_CTAG_DEI_CHANGE_EN_LEN     1
	#define PRE_IPO_ACTION_CTAG_DEI_CHANGE_EN_DEFAULT 0x0
	/*[field] CTAG_DEI*/
	#define PRE_IPO_ACTION_CTAG_DEI
	#define PRE_IPO_ACTION_CTAG_DEI_OFFSET  66
	#define PRE_IPO_ACTION_CTAG_DEI_LEN     1
	#define PRE_IPO_ACTION_CTAG_DEI_DEFAULT 0x0
	/*[field] ENQUEUE_PRI_CHANGE_EN*/
	#define PRE_IPO_ACTION_ENQUEUE_PRI_CHANGE_EN
	#define PRE_IPO_ACTION_ENQUEUE_PRI_CHANGE_EN_OFFSET  67
	#define PRE_IPO_ACTION_ENQUEUE_PRI_CHANGE_EN_LEN     1
	#define PRE_IPO_ACTION_ENQUEUE_PRI_CHANGE_EN_DEFAULT 0x0
	/*[field] ENQUEUE_PRI*/
	#define PRE_IPO_ACTION_ENQUEUE_PRI
	#define PRE_IPO_ACTION_ENQUEUE_PRI_OFFSET  68
	#define PRE_IPO_ACTION_ENQUEUE_PRI_LEN     4
	#define PRE_IPO_ACTION_ENQUEUE_PRI_DEFAULT 0x0
	/*[field] INT_DP_CHANGE_EN*/
	#define PRE_IPO_ACTION_INT_DP_CHANGE_EN
	#define PRE_IPO_ACTION_INT_DP_CHANGE_EN_OFFSET  72
	#define PRE_IPO_ACTION_INT_DP_CHANGE_EN_LEN     1
	#define PRE_IPO_ACTION_INT_DP_CHANGE_EN_DEFAULT 0x0
	/*[field] INT_DP*/
	#define PRE_IPO_ACTION_INT_DP
	#define PRE_IPO_ACTION_INT_DP_OFFSET  73
	#define PRE_IPO_ACTION_INT_DP_LEN     2
	#define PRE_IPO_ACTION_INT_DP_DEFAULT 0x0
	/*[field] POLICER_EN*/
	#define PRE_IPO_ACTION_POLICER_EN
	#define PRE_IPO_ACTION_POLICER_EN_OFFSET  75
	#define PRE_IPO_ACTION_POLICER_EN_LEN     1
	#define PRE_IPO_ACTION_POLICER_EN_DEFAULT 0x0
	/*[field] POLICER_INDEX*/
	#define PRE_IPO_ACTION_POLICER_INDEX
	#define PRE_IPO_ACTION_POLICER_INDEX_OFFSET  76
	#define PRE_IPO_ACTION_POLICER_INDEX_LEN     9
	#define PRE_IPO_ACTION_POLICER_INDEX_DEFAULT 0x0
	/*[field] QID_EN*/
	#define PRE_IPO_ACTION_QID_EN
	#define PRE_IPO_ACTION_QID_EN_OFFSET  85
	#define PRE_IPO_ACTION_QID_EN_LEN     1
	#define PRE_IPO_ACTION_QID_EN_DEFAULT 0x0
	/*[field] QID*/
	#define PRE_IPO_ACTION_QID
	#define PRE_IPO_ACTION_QID_OFFSET  86
	#define PRE_IPO_ACTION_QID_LEN     8
	#define PRE_IPO_ACTION_QID_DEFAULT 0x0
	/*[field] SERVICE_CODE_EN*/
	#define PRE_IPO_ACTION_SERVICE_CODE_EN
	#define PRE_IPO_ACTION_SERVICE_CODE_EN_OFFSET  94
	#define PRE_IPO_ACTION_SERVICE_CODE_EN_LEN     1
	#define PRE_IPO_ACTION_SERVICE_CODE_EN_DEFAULT 0x0
	/*[field] SERVICE_CODE*/
	#define PRE_IPO_ACTION_SERVICE_CODE
	#define PRE_IPO_ACTION_SERVICE_CODE_OFFSET  95
	#define PRE_IPO_ACTION_SERVICE_CODE_LEN     8
	#define PRE_IPO_ACTION_SERVICE_CODE_DEFAULT 0x0
	/*[field] SYN_TOGGLE*/
	#define PRE_IPO_ACTION_SYN_TOGGLE
	#define PRE_IPO_ACTION_SYN_TOGGLE_OFFSET  103
	#define PRE_IPO_ACTION_SYN_TOGGLE_LEN     1
	#define PRE_IPO_ACTION_SYN_TOGGLE_DEFAULT 0x0
	/*[field] CPU_CODE_EN*/
	#define PRE_IPO_ACTION_CPU_CODE_EN
	#define PRE_IPO_ACTION_CPU_CODE_EN_OFFSET  104
	#define PRE_IPO_ACTION_CPU_CODE_EN_LEN     1
	#define PRE_IPO_ACTION_CPU_CODE_EN_DEFAULT 0x0
	/*[field] CPU_CODE*/
	#define PRE_IPO_ACTION_CPU_CODE
	#define PRE_IPO_ACTION_CPU_CODE_OFFSET  105
	#define PRE_IPO_ACTION_CPU_CODE_LEN     8
	#define PRE_IPO_ACTION_CPU_CODE_DEFAULT 0x0
	/*[field] DSCP_TC_MASK*/
	#define PRE_IPO_ACTION_DSCP_TC_MASK
	#define PRE_IPO_ACTION_DSCP_TC_MASK_OFFSET  113
	#define PRE_IPO_ACTION_DSCP_TC_MASK_LEN     8
	#define PRE_IPO_ACTION_DSCP_TC_MASK_DEFAULT 0x0
	/*[field] QOS_RES_PREC*/
	#define PRE_IPO_ACTION_QOS_RES_PREC
	#define PRE_IPO_ACTION_QOS_RES_PREC_OFFSET  121
	#define PRE_IPO_ACTION_QOS_RES_PREC_LEN     3
	#define PRE_IPO_ACTION_QOS_RES_PREC_DEFAULT 0x0
	/*[field] CASCADE_EN*/
	#define PRE_IPO_ACTION_CASCADE_EN
	#define PRE_IPO_ACTION_CASCADE_EN_OFFSET  124
	#define PRE_IPO_ACTION_CASCADE_EN_LEN     1
	#define PRE_IPO_ACTION_CASCADE_EN_DEFAULT 0x0
	/*[field] CASCADE_DATA*/
	#define PRE_IPO_ACTION_CASCADE_DATA
	#define PRE_IPO_ACTION_CASCADE_DATA_OFFSET  125
	#define PRE_IPO_ACTION_CASCADE_DATA_LEN     7
	#define PRE_IPO_ACTION_CASCADE_DATA_DEFAULT 0x0
	/*[field] METADATA_EN*/
	#define PRE_IPO_ACTION_METADATA_EN
	#define PRE_IPO_ACTION_METADATA_EN_OFFSET  132
	#define PRE_IPO_ACTION_METADATA_EN_LEN     1
	#define PRE_IPO_ACTION_METADATA_EN_DEFAULT 0x0
	/*[field] NAT_ACTION*/
	#define PRE_IPO_ACTION_NAT_ACTION
	#define PRE_IPO_ACTION_NAT_ACTION_OFFSET  134
	#define PRE_IPO_ACTION_NAT_ACTION_LEN     3
	#define PRE_IPO_ACTION_NAT_ACTION_DEFAULT 0x0
	/*[field] L4_PORT*/
	#define PRE_IPO_ACTION_L4_PORT
	#define PRE_IPO_ACTION_L4_PORT_OFFSET  137
	#define PRE_IPO_ACTION_L4_PORT_LEN     16
	#define PRE_IPO_ACTION_L4_PORT_DEFAULT 0x0
	/*[field] VPN_VALID*/
	#define PRE_IPO_ACTION_VPN_VALID
	#define PRE_IPO_ACTION_VPN_VALID_OFFSET  153
	#define PRE_IPO_ACTION_VPN_VALID_LEN     1
	#define PRE_IPO_ACTION_VPN_VALID_DEFAULT 0x0
	/*[field] VPN_TYPE*/
	#define PRE_IPO_ACTION_VPN_TYPE
	#define PRE_IPO_ACTION_VPN_TYPE_OFFSET  154
	#define PRE_IPO_ACTION_VPN_TYPE_LEN     1
	#define PRE_IPO_ACTION_VPN_TYPE_DEFAULT 0x0
	/*[field] VPN_ID*/
	#define PRE_IPO_ACTION_VPN_ID
	#define PRE_IPO_ACTION_VPN_ID_OFFSET  155
	#define PRE_IPO_ACTION_VPN_ID_LEN     6
	#define PRE_IPO_ACTION_VPN_ID_DEFAULT 0x0
	/*[field] LEARN_DIS*/
	#define PRE_IPO_ACTION_LEARN_DIS
	#define PRE_IPO_ACTION_LEARN_DIS_OFFSET  161
	#define PRE_IPO_ACTION_LEARN_DIS_LEN     1
	#define PRE_IPO_ACTION_LEARN_DIS_DEFAULT 0x0
#if defined(MPPE)
	/*[field] METADATA_PRI*/
	#define IPO_ACTION_METADATA_PRI
	#define IPO_ACTION_METADATA_PRI_OFFSET  162
	#define IPO_ACTION_METADATA_PRI_LEN     4
	#define IPO_ACTION_METADATA_PRI_DEFAULT 0x0
#endif

struct pre_ipo_action {
	a_uint32_t  dest_info_change_en:1;
	a_uint32_t  fwd_cmd:2;
	a_uint32_t  dest_info:14;
	a_uint32_t  mirror_en:1;
	a_uint32_t  svid_change_en:1;
	a_uint32_t  stag_fmt:1;
	a_uint32_t  svid:12;
	a_uint32_t  cvid_change_en:1;
	a_uint32_t  ctag_fmt:1;
	a_uint32_t  cvid:12;
	a_uint32_t  dscp_tc_change_en:1;
	a_uint32_t  dscp_tc:8;
	a_uint32_t  stag_pcp_change_en:1;
	a_uint32_t  stag_pcp:3;
	a_uint32_t  stag_dei_change_en:1;
	a_uint32_t  stag_dei:1;
	a_uint32_t  ctag_pcp_change_en:1;
	a_uint32_t  ctag_pcp_0:2;
	a_uint32_t  ctag_pcp_1:1;
	a_uint32_t  ctag_dei_change_en:1;
	a_uint32_t  ctag_dei:1;
	a_uint32_t  enqueue_pri_change_en:1;
	a_uint32_t  enqueue_pri:4;
	a_uint32_t  int_dp_change_en:1;
	a_uint32_t  int_dp:2;
	a_uint32_t  policer_en:1;
	a_uint32_t  policer_index:9;
	a_uint32_t  qid_en:1;
	a_uint32_t  qid:8;
	a_uint32_t  service_code_en:1;
	a_uint32_t  service_code_0:1;
	a_uint32_t  service_code_1:7;
	a_uint32_t  syn_toggle:1;
	a_uint32_t  cpu_code_en:1;
	a_uint32_t  cpu_code:8;
	a_uint32_t  dscp_tc_mask:8;
	a_uint32_t  qos_res_prec:3;
	a_uint32_t  cascade_en:1;
	a_uint32_t  cascade_data_0:3;
	a_uint32_t  cascade_data_1:4;
	a_uint32_t  metadata_en:1;
	a_uint32_t  _reserved0:1;
	a_uint32_t  nat_action:3;
	a_uint32_t  l4_port:16;
	a_uint32_t  vpn_valid:1;
	a_uint32_t  vpn_type:1;
	a_uint32_t  vpn_id_0:5;
	a_uint32_t  vpn_id_1:1;
	a_uint32_t  learn_dis:1;
#if defined(MPPE)
	a_uint32_t metadata_pri:4;
	a_uint32_t  _reserved1:26;
#else
	a_uint32_t  _reserved1:30;
#endif
};

union pre_ipo_action_u {
	a_uint32_t val[6];
	struct pre_ipo_action bf;
};

#endif
#endif
