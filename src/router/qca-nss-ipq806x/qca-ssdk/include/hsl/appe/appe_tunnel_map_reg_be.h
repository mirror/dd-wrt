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
#ifndef APPE_TUNNEL_MAP_REG_H
#define APPE_TUNNEL_MAP_REG_H

/*[table] TL_MAP_LPM_COUNTER*/
#define TL_MAP_LPM_COUNTER
#define TL_MAP_LPM_COUNTER_ADDRESS 0x7000
#define TL_MAP_LPM_COUNTER_NUM     8
#define TL_MAP_LPM_COUNTER_INC     0x20
#define TL_MAP_LPM_COUNTER_TYPE    REG_TYPE_RW
#define TL_MAP_LPM_COUNTER_DEFAULT 0x0
	/*[field] PKT_CNT*/
	#define TL_MAP_LPM_COUNTER_PKT_CNT
	#define TL_MAP_LPM_COUNTER_PKT_CNT_OFFSET  0
	#define TL_MAP_LPM_COUNTER_PKT_CNT_LEN     32
	#define TL_MAP_LPM_COUNTER_PKT_CNT_DEFAULT 0x0
	/*[field] BYTE_CNT*/
	#define TL_MAP_LPM_COUNTER_BYTE_CNT
	#define TL_MAP_LPM_COUNTER_BYTE_CNT_OFFSET  32
	#define TL_MAP_LPM_COUNTER_BYTE_CNT_LEN     40
	#define TL_MAP_LPM_COUNTER_BYTE_CNT_DEFAULT 0x0

struct tl_map_lpm_counter {
	a_uint32_t  pkt_cnt:32;
	a_uint32_t  byte_cnt_0:32;
	a_uint32_t  _reserved0:24;
	a_uint32_t  byte_cnt_1:8;
};

union tl_map_lpm_counter_u {
	a_uint32_t val[3];
	struct tl_map_lpm_counter bf;
};

/*[table] TL_MAP_RULE_TBL*/
#define TL_MAP_RULE_TBL
#define TL_MAP_RULE_TBL_ADDRESS 0x7100
#define TL_MAP_RULE_TBL_NUM     8
#define TL_MAP_RULE_TBL_INC     0x10
#define TL_MAP_RULE_TBL_TYPE    REG_TYPE_RW
#define TL_MAP_RULE_TBL_DEFAULT 0x0
	/*[field] SRC1*/
	#define TL_MAP_RULE_TBL_SRC1
	#define TL_MAP_RULE_TBL_SRC1_OFFSET  0
	#define TL_MAP_RULE_TBL_SRC1_LEN     32
	#define TL_MAP_RULE_TBL_SRC1_DEFAULT 0x0
	/*[field] SRC2*/
	#define TL_MAP_RULE_TBL_SRC2
	#define TL_MAP_RULE_TBL_SRC2_OFFSET  32
	#define TL_MAP_RULE_TBL_SRC2_LEN     2
	#define TL_MAP_RULE_TBL_SRC2_DEFAULT 0x0
	/*[field] START2_SUFFIX*/
	#define TL_MAP_RULE_TBL_START2_SUFFIX
	#define TL_MAP_RULE_TBL_START2_SUFFIX_OFFSET  34
	#define TL_MAP_RULE_TBL_START2_SUFFIX_LEN     7
	#define TL_MAP_RULE_TBL_START2_SUFFIX_DEFAULT 0x0
	/*[field] WIDTH2_SUFFIX*/
	#define TL_MAP_RULE_TBL_WIDTH2_SUFFIX
	#define TL_MAP_RULE_TBL_WIDTH2_SUFFIX_OFFSET  41
	#define TL_MAP_RULE_TBL_WIDTH2_SUFFIX_LEN     5
	#define TL_MAP_RULE_TBL_WIDTH2_SUFFIX_DEFAULT 0x0
	/*[field] POS2_SUFFIX*/
	#define TL_MAP_RULE_TBL_POS2_SUFFIX
	#define TL_MAP_RULE_TBL_POS2_SUFFIX_OFFSET  46
	#define TL_MAP_RULE_TBL_POS2_SUFFIX_LEN     5
	#define TL_MAP_RULE_TBL_POS2_SUFFIX_DEFAULT 0x0
	/*[field] SRC2_VALID*/
	#define TL_MAP_RULE_TBL_SRC2_VALID
	#define TL_MAP_RULE_TBL_SRC2_VALID_OFFSET  51
	#define TL_MAP_RULE_TBL_SRC2_VALID_LEN     1
	#define TL_MAP_RULE_TBL_SRC2_VALID_DEFAULT 0x0
	/*[field] START2_PSID*/
	#define TL_MAP_RULE_TBL_START2_PSID
	#define TL_MAP_RULE_TBL_START2_PSID_OFFSET  52
	#define TL_MAP_RULE_TBL_START2_PSID_LEN     7
	#define TL_MAP_RULE_TBL_START2_PSID_DEFAULT 0x0
	/*[field] WIDTH2_PSID*/
	#define TL_MAP_RULE_TBL_WIDTH2_PSID
	#define TL_MAP_RULE_TBL_WIDTH2_PSID_OFFSET  59
	#define TL_MAP_RULE_TBL_WIDTH2_PSID_LEN     4
	#define TL_MAP_RULE_TBL_WIDTH2_PSID_DEFAULT 0x0
	/*[field] SRC3*/
	#define TL_MAP_RULE_TBL_SRC3
	#define TL_MAP_RULE_TBL_SRC3_OFFSET  63
	#define TL_MAP_RULE_TBL_SRC3_LEN     2
	#define TL_MAP_RULE_TBL_SRC3_DEFAULT 0x0
	/*[field] START3_PSID*/
	#define TL_MAP_RULE_TBL_START3_PSID
	#define TL_MAP_RULE_TBL_START3_PSID_OFFSET  65
	#define TL_MAP_RULE_TBL_START3_PSID_LEN     4
	#define TL_MAP_RULE_TBL_START3_PSID_DEFAULT 0x0
	/*[field] WIDTH3_PSID*/
	#define TL_MAP_RULE_TBL_WIDTH3_PSID
	#define TL_MAP_RULE_TBL_WIDTH3_PSID_OFFSET  69
	#define TL_MAP_RULE_TBL_WIDTH3_PSID_LEN     4
	#define TL_MAP_RULE_TBL_WIDTH3_PSID_DEFAULT 0x0
	/*[field] SRC3_VALID*/
	#define TL_MAP_RULE_TBL_SRC3_VALID
	#define TL_MAP_RULE_TBL_SRC3_VALID_OFFSET  73
	#define TL_MAP_RULE_TBL_SRC3_VALID_LEN     1
	#define TL_MAP_RULE_TBL_SRC3_VALID_DEFAULT 0x0
	/*[field] CHECK_EN*/
	#define TL_MAP_RULE_TBL_CHECK_EN
	#define TL_MAP_RULE_TBL_CHECK_EN_OFFSET  74
	#define TL_MAP_RULE_TBL_CHECK_EN_LEN     1
	#define TL_MAP_RULE_TBL_CHECK_EN_DEFAULT 0x0

struct tl_map_rule_tbl {
	a_uint32_t  src1:32;
	a_uint32_t  src3_0:1;
	a_uint32_t  width2_psid:4;
	a_uint32_t  start2_psid:7;
	a_uint32_t  src2_valid:1;
	a_uint32_t  pos2_suffix:5;
	a_uint32_t  width2_suffix:5;
	a_uint32_t  start2_suffix:7;
	a_uint32_t  src2:2;
	a_uint32_t  _reserved0:21;
	a_uint32_t  check_en:1;
	a_uint32_t  src3_valid:1;
	a_uint32_t  width3_psid:4;
	a_uint32_t  start3_psid:4;
	a_uint32_t  src3_1:1;
};

union tl_map_rule_tbl_u {
	a_uint32_t val[3];
	struct tl_map_rule_tbl bf;
};

/*[table] TL_MAP_LPM_TBL*/
#define TL_MAP_LPM_TBL
#define TL_MAP_LPM_TBL_ADDRESS 0x7200
#define TL_MAP_LPM_TBL_NUM     8
#define TL_MAP_LPM_TBL_INC     0x20
#define TL_MAP_LPM_TBL_TYPE    REG_TYPE_RW
#define TL_MAP_LPM_TBL_DEFAULT 0x0
	/*[field] IPV6_ADDR*/
	#define TL_MAP_LPM_TBL_IPV6_ADDR
	#define TL_MAP_LPM_TBL_IPV6_ADDR_OFFSET  0
	#define TL_MAP_LPM_TBL_IPV6_ADDR_LEN     128
	#define TL_MAP_LPM_TBL_IPV6_ADDR_DEFAULT 0x0
	/*[field] PREFIX_LEN*/
	#define TL_MAP_LPM_TBL_PREFIX_LEN
	#define TL_MAP_LPM_TBL_PREFIX_LEN_OFFSET  128
	#define TL_MAP_LPM_TBL_PREFIX_LEN_LEN     8
	#define TL_MAP_LPM_TBL_PREFIX_LEN_DEFAULT 0x0
	/*[field] VALID*/
	#define TL_MAP_LPM_TBL_VALID
	#define TL_MAP_LPM_TBL_VALID_OFFSET  136
	#define TL_MAP_LPM_TBL_VALID_LEN     1
	#define TL_MAP_LPM_TBL_VALID_DEFAULT 0x0

struct tl_map_lpm_tbl {
	a_uint32_t  ipv6_addr_0:32;
	a_uint32_t  ipv6_addr_1:32;
	a_uint32_t  ipv6_addr_2:32;
	a_uint32_t  ipv6_addr_3:32;
	a_uint32_t  _reserved0:23;
	a_uint32_t  valid:1;
	a_uint32_t  prefix_len:8;
};

union tl_map_lpm_tbl_u {
	a_uint32_t val[5];
	struct tl_map_lpm_tbl bf;
};

/*[table] TL_MAP_LPM_ACT*/
#define TL_MAP_LPM_ACT
#define TL_MAP_LPM_ACT_ADDRESS 0x7300
#define TL_MAP_LPM_ACT_NUM     8
#define TL_MAP_LPM_ACT_INC     0x10
#define TL_MAP_LPM_ACT_TYPE    REG_TYPE_RW
#define TL_MAP_LPM_ACT_DEFAULT 0x0
	/*[field] MAP_RULE_ID*/
	#define TL_MAP_LPM_ACT_MAP_RULE_ID
	#define TL_MAP_LPM_ACT_MAP_RULE_ID_OFFSET  0
	#define TL_MAP_LPM_ACT_MAP_RULE_ID_LEN     5
	#define TL_MAP_LPM_ACT_MAP_RULE_ID_DEFAULT 0x0
	/*[field] IP_TO_ME*/
	#define TL_MAP_LPM_ACT_IP_TO_ME
	#define TL_MAP_LPM_ACT_IP_TO_ME_OFFSET  5
	#define TL_MAP_LPM_ACT_IP_TO_ME_LEN     1
	#define TL_MAP_LPM_ACT_IP_TO_ME_DEFAULT 0x0
	/*[field] SVLAN_FMT*/
	#define TL_MAP_LPM_ACT_SVLAN_FMT
	#define TL_MAP_LPM_ACT_SVLAN_FMT_OFFSET  6
	#define TL_MAP_LPM_ACT_SVLAN_FMT_LEN     1
	#define TL_MAP_LPM_ACT_SVLAN_FMT_DEFAULT 0x0
	/*[field] SVLAN_ID*/
	#define TL_MAP_LPM_ACT_SVLAN_ID
	#define TL_MAP_LPM_ACT_SVLAN_ID_OFFSET  7
	#define TL_MAP_LPM_ACT_SVLAN_ID_LEN     12
	#define TL_MAP_LPM_ACT_SVLAN_ID_DEFAULT 0x0
	/*[field] CVLAN_FMT*/
	#define TL_MAP_LPM_ACT_CVLAN_FMT
	#define TL_MAP_LPM_ACT_CVLAN_FMT_OFFSET  19
	#define TL_MAP_LPM_ACT_CVLAN_FMT_LEN     1
	#define TL_MAP_LPM_ACT_CVLAN_FMT_DEFAULT 0x0
	/*[field] CVLAN_ID*/
	#define TL_MAP_LPM_ACT_CVLAN_ID
	#define TL_MAP_LPM_ACT_CVLAN_ID_OFFSET  20
	#define TL_MAP_LPM_ACT_CVLAN_ID_LEN     12
	#define TL_MAP_LPM_ACT_CVLAN_ID_DEFAULT 0x0
	/*[field] SVLAN_CHECK_EN*/
	#define TL_MAP_LPM_ACT_SVLAN_CHECK_EN
	#define TL_MAP_LPM_ACT_SVLAN_CHECK_EN_OFFSET  32
	#define TL_MAP_LPM_ACT_SVLAN_CHECK_EN_LEN     1
	#define TL_MAP_LPM_ACT_SVLAN_CHECK_EN_DEFAULT 0x0
	/*[field] CVLAN_CHECK_EN*/
	#define TL_MAP_LPM_ACT_CVLAN_CHECK_EN
	#define TL_MAP_LPM_ACT_CVLAN_CHECK_EN_OFFSET  33
	#define TL_MAP_LPM_ACT_CVLAN_CHECK_EN_LEN     1
	#define TL_MAP_LPM_ACT_CVLAN_CHECK_EN_DEFAULT 0x0
	/*[field] TL_L3_IF_CHECK_EN*/
	#define TL_MAP_LPM_ACT_TL_L3_IF_CHECK_EN
	#define TL_MAP_LPM_ACT_TL_L3_IF_CHECK_EN_OFFSET  34
	#define TL_MAP_LPM_ACT_TL_L3_IF_CHECK_EN_LEN     1
	#define TL_MAP_LPM_ACT_TL_L3_IF_CHECK_EN_DEFAULT 0x0
	/*[field] TL_L3_IF*/
	#define TL_MAP_LPM_ACT_TL_L3_IF
	#define TL_MAP_LPM_ACT_TL_L3_IF_OFFSET  35
	#define TL_MAP_LPM_ACT_TL_L3_IF_LEN     7
	#define TL_MAP_LPM_ACT_TL_L3_IF_DEFAULT 0x0
	/*[field] SRC_INFO_VALID*/
	#define TL_MAP_LPM_ACT_SRC_INFO_VALID
	#define TL_MAP_LPM_ACT_SRC_INFO_VALID_OFFSET  42
	#define TL_MAP_LPM_ACT_SRC_INFO_VALID_LEN     1
	#define TL_MAP_LPM_ACT_SRC_INFO_VALID_DEFAULT 0x0
	/*[field] SRC_INFO_TYPE*/
	#define TL_MAP_LPM_ACT_SRC_INFO_TYPE
	#define TL_MAP_LPM_ACT_SRC_INFO_TYPE_OFFSET  43
	#define TL_MAP_LPM_ACT_SRC_INFO_TYPE_LEN     1
	#define TL_MAP_LPM_ACT_SRC_INFO_TYPE_DEFAULT 0x0
	/*[field] SRC_INFO*/
	#define TL_MAP_LPM_ACT_SRC_INFO
	#define TL_MAP_LPM_ACT_SRC_INFO_OFFSET  44
	#define TL_MAP_LPM_ACT_SRC_INFO_LEN     8
	#define TL_MAP_LPM_ACT_SRC_INFO_DEFAULT 0x0
	/*[field] EXP_PROFILE*/
	#define TL_MAP_LPM_ACT_EXP_PROFILE
	#define TL_MAP_LPM_ACT_EXP_PROFILE_OFFSET  52
	#define TL_MAP_LPM_ACT_EXP_PROFILE_LEN     2
	#define TL_MAP_LPM_ACT_EXP_PROFILE_DEFAULT 0x0
#if defined(MPPE)
	/*[field] SERVICE_CODE*/
	#define TL_MAP_LPM_ACT_SERVICE_CODE
	#define TL_MAP_LPM_ACT_SERVICE_CODE_OFFSET  54
	#define TL_MAP_LPM_ACT_SERVICE_CODE_LEN     8
	#define TL_MAP_LPM_ACT_SERVICE_CODE_DEFAULT 0x0
	/*[field] SERVICE_CODE_EN*/
	#define TL_MAP_LPM_ACT_SERVICE_CODE_EN
	#define TL_MAP_LPM_ACT_SERVICE_CODE_EN_OFFSET  62
	#define TL_MAP_LPM_ACT_SERVICE_CODE_EN_LEN     1
	#define TL_MAP_LPM_ACT_SERVICE_CODE_EN_DEFAULT 0x0
#endif

struct tl_map_lpm_act {
	a_uint32_t  cvlan_id:12;
	a_uint32_t  cvlan_fmt:1;
	a_uint32_t  svlan_id:12;
	a_uint32_t  svlan_fmt:1;
	a_uint32_t  ip_to_me:1;
	a_uint32_t  map_rule_id:5;
#if defined(MPPE)
	a_uint32_t  _reserved0:1;
	a_uint32_t  service_code_en:1;
	a_uint32_t  service_code:8;
#else
	a_uint32_t  _reserved0:10;
#endif
	a_uint32_t  exp_profile:2;
	a_uint32_t  src_info:8;
	a_uint32_t  src_info_type:1;
	a_uint32_t  src_info_valid:1;
	a_uint32_t  tl_l3_if:7;
	a_uint32_t  tl_l3_if_check_en:1;
	a_uint32_t  cvlan_check_en:1;
	a_uint32_t  svlan_check_en:1;
};

union tl_map_lpm_act_u {
	a_uint32_t val[2];
	struct tl_map_lpm_act bf;
};
#endif
