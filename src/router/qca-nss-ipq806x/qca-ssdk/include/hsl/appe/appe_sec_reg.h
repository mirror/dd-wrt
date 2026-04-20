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


#if defined(CONFIG_CPU_BIG_ENDIAN)
#include "appe_sec_reg_be.h"
#else
/**
 * @defgroup
 * @{
 */
#ifndef APPE_SEC_REG_H
#define APPE_SEC_REG_H


/*[register] L2_EXCEP_CTRL*/
#define L2_EXCEP_CTRL
#define L2_EXCEP_CTRL_ADDRESS 0xc
#define L2_EXCEP_CTRL_NUM     1
#define L2_EXCEP_CTRL_INC     0x4
#define L2_EXCEP_CTRL_TYPE    REG_TYPE_RW
#define L2_EXCEP_CTRL_DEFAULT 0x3
	/*[field] TUNNEL_EXCEP_FWD*/
	#define L2_EXCEP_CTRL_TUNNEL_EXCEP_FWD
	#define L2_EXCEP_CTRL_TUNNEL_EXCEP_FWD_OFFSET  0
	#define L2_EXCEP_CTRL_TUNNEL_EXCEP_FWD_LEN     2
	#define L2_EXCEP_CTRL_TUNNEL_EXCEP_FWD_DEFAULT 0x3

struct l2_excep_ctrl {
	a_uint32_t  tunnel_excep_fwd:2;
	a_uint32_t  _reserved0:30;
};

union l2_excep_ctrl_u {
	a_uint32_t val;
	struct l2_excep_ctrl bf;
};

/*[register] L2_FLOW_HIT_EXP_CTRL*/
#define L2_FLOW_HIT_EXP_CTRL
#define L2_FLOW_HIT_EXP_CTRL_ADDRESS 0x1548
#define L2_FLOW_HIT_EXP_CTRL_NUM     72
#define L2_FLOW_HIT_EXP_CTRL_INC     0x4
#define L2_FLOW_HIT_EXP_CTRL_TYPE    REG_TYPE_RW
#define L2_FLOW_HIT_EXP_CTRL_DEFAULT 0x0
	/*[field] EXCEP_EN*/
	#define L2_FLOW_HIT_EXP_CTRL_EXCEP_EN
	#define L2_FLOW_HIT_EXP_CTRL_EXCEP_EN_OFFSET  0
	#define L2_FLOW_HIT_EXP_CTRL_EXCEP_EN_LEN     1
	#define L2_FLOW_HIT_EXP_CTRL_EXCEP_EN_DEFAULT 0x0

struct l2_flow_hit_exp_ctrl {
	a_uint32_t  excep_en:1;
	a_uint32_t  _reserved0:31;
};

union l2_flow_hit_exp_ctrl_u {
	a_uint32_t val;
	struct l2_flow_hit_exp_ctrl bf;
};

/*[register] L3_FLOW_HIT_EXP_CTRL*/
#define L3_FLOW_HIT_EXP_CTRL
#define L3_FLOW_HIT_EXP_CTRL_ADDRESS 0x1678
#define L3_FLOW_HIT_EXP_CTRL_NUM     72
#define L3_FLOW_HIT_EXP_CTRL_INC     0x4
#define L3_FLOW_HIT_EXP_CTRL_TYPE    REG_TYPE_RW
#define L3_FLOW_HIT_EXP_CTRL_DEFAULT 0x0
	/*[field] EXCEP_EN*/
	#define L3_FLOW_HIT_EXP_CTRL_EXCEP_EN
	#define L3_FLOW_HIT_EXP_CTRL_EXCEP_EN_OFFSET  0
	#define L3_FLOW_HIT_EXP_CTRL_EXCEP_EN_LEN     1
	#define L3_FLOW_HIT_EXP_CTRL_EXCEP_EN_DEFAULT 0x0

struct l3_flow_hit_exp_ctrl {
	a_uint32_t  excep_en:1;
	a_uint32_t  _reserved0:31;
};

union l3_flow_hit_exp_ctrl_u {
	a_uint32_t val;
	struct l3_flow_hit_exp_ctrl bf;
};

/*[register] L3_FLOW_HIT_MISS_EXP_CTRL*/
#define L3_FLOW_HIT_MISS_EXP_CTRL
#define L3_FLOW_HIT_MISS_EXP_CTRL_ADDRESS 0x17a8
#define L3_FLOW_HIT_MISS_EXP_CTRL_NUM     72
#define L3_FLOW_HIT_MISS_EXP_CTRL_INC     0x4
#define L3_FLOW_HIT_MISS_EXP_CTRL_TYPE    REG_TYPE_RW
#define L3_FLOW_HIT_MISS_EXP_CTRL_DEFAULT 0x0
	/*[field] EXCEP_EN*/
	#define L3_FLOW_HIT_MISS_EXP_CTRL_EXCEP_EN
	#define L3_FLOW_HIT_MISS_EXP_CTRL_EXCEP_EN_OFFSET  0
	#define L3_FLOW_HIT_MISS_EXP_CTRL_EXCEP_EN_LEN     1
	#define L3_FLOW_HIT_MISS_EXP_CTRL_EXCEP_EN_DEFAULT 0x0

struct l3_flow_hit_miss_exp_ctrl {
	a_uint32_t  excep_en:1;
	a_uint32_t  _reserved0:31;
};

union l3_flow_hit_miss_exp_ctrl_u {
	a_uint32_t val;
	struct l3_flow_hit_miss_exp_ctrl bf;
};

/*[register] L2_FLOW_HIT_MISS_EXP_CTRL*/
#define L2_FLOW_HIT_MISS_EXP_CTRL
#define L2_FLOW_HIT_MISS_EXP_CTRL_ADDRESS 0x18d8
#define L2_FLOW_HIT_MISS_EXP_CTRL_NUM     72
#define L2_FLOW_HIT_MISS_EXP_CTRL_INC     0x4
#define L2_FLOW_HIT_MISS_EXP_CTRL_TYPE    REG_TYPE_RW
#define L2_FLOW_HIT_MISS_EXP_CTRL_DEFAULT 0x0
	/*[field] EXCEP_EN*/
	#define L2_FLOW_HIT_MISS_EXP_CTRL_EXCEP_EN
	#define L2_FLOW_HIT_MISS_EXP_CTRL_EXCEP_EN_OFFSET  0
	#define L2_FLOW_HIT_MISS_EXP_CTRL_EXCEP_EN_LEN     1
	#define L2_FLOW_HIT_MISS_EXP_CTRL_EXCEP_EN_DEFAULT 0x0

struct l2_flow_hit_miss_exp_ctrl {
	a_uint32_t  excep_en:1;
	a_uint32_t  _reserved0:31;
};

union l2_flow_hit_miss_exp_ctrl_u {
	a_uint32_t val;
	struct l2_flow_hit_miss_exp_ctrl bf;
};

/*[table] TL_EXCEPTION_CMD*/
#define TL_EXCEPTION_CMD
#define TL_EXCEPTION_CMD_ADDRESS 0x7400
#define TL_EXCEPTION_CMD_NUM     88
#define TL_EXCEPTION_CMD_INC     0x4
#define TL_EXCEPTION_CMD_TYPE    REG_TYPE_RW
#define TL_EXCEPTION_CMD_DEFAULT 0x0
	/*[field] TL_EXCEP_CMD*/
	#define TL_EXCEPTION_CMD_TL_EXCEP_CMD
	#define TL_EXCEPTION_CMD_TL_EXCEP_CMD_OFFSET  0
	#define TL_EXCEPTION_CMD_TL_EXCEP_CMD_LEN     2
	#define TL_EXCEPTION_CMD_TL_EXCEP_CMD_DEFAULT 0x0
	/*[field] DE_ACCE*/
	#define TL_EXCEPTION_CMD_DE_ACCE
	#define TL_EXCEPTION_CMD_DE_ACCE_OFFSET  2
	#define TL_EXCEPTION_CMD_DE_ACCE_LEN     1
	#define TL_EXCEPTION_CMD_DE_ACCE_DEFAULT 0x0

struct tl_exception_cmd {
	a_uint32_t  tl_excep_cmd:2;
	a_uint32_t  de_acce:1;
	a_uint32_t  _reserved0:29;
};

union tl_exception_cmd_u {
	a_uint32_t val;
	struct tl_exception_cmd bf;
};

/*[table] TL_EXP_CTRL_PROFILE0*/
#define TL_EXP_CTRL_PROFILE0
#define TL_EXP_CTRL_PROFILE0_ADDRESS 0x7600
#define TL_EXP_CTRL_PROFILE0_NUM     88
#define TL_EXP_CTRL_PROFILE0_INC     0x4
#define TL_EXP_CTRL_PROFILE0_TYPE    REG_TYPE_RW
#define TL_EXP_CTRL_PROFILE0_DEFAULT 0x0
	/*[field] EXCEP_EN*/
	#define TL_EXP_CTRL_PROFILE0_EXCEP_EN
	#define TL_EXP_CTRL_PROFILE0_EXCEP_EN_OFFSET  0
	#define TL_EXP_CTRL_PROFILE0_EXCEP_EN_LEN     1
	#define TL_EXP_CTRL_PROFILE0_EXCEP_EN_DEFAULT 0x0

struct tl_exp_ctrl_profile0 {
	a_uint32_t  excep_en:1;
	a_uint32_t  _reserved0:31;
};

union tl_exp_ctrl_profile0_u {
	a_uint32_t val;
	struct tl_exp_ctrl_profile0 bf;
};

/*[table] TL_EXP_CTRL_PROFILE1*/
#define TL_EXP_CTRL_PROFILE1
#define TL_EXP_CTRL_PROFILE1_ADDRESS 0x7800
#define TL_EXP_CTRL_PROFILE1_NUM     88
#define TL_EXP_CTRL_PROFILE1_INC     0x4
#define TL_EXP_CTRL_PROFILE1_TYPE    REG_TYPE_RW
#define TL_EXP_CTRL_PROFILE1_DEFAULT 0x0
	/*[field] EXCEP_EN*/
	#define TL_EXP_CTRL_PROFILE1_EXCEP_EN
	#define TL_EXP_CTRL_PROFILE1_EXCEP_EN_OFFSET  0
	#define TL_EXP_CTRL_PROFILE1_EXCEP_EN_LEN     1
	#define TL_EXP_CTRL_PROFILE1_EXCEP_EN_DEFAULT 0x0

struct tl_exp_ctrl_profile1 {
	a_uint32_t  excep_en:1;
	a_uint32_t  _reserved0:31;
};

union tl_exp_ctrl_profile1_u {
	a_uint32_t val;
	struct tl_exp_ctrl_profile1 bf;
};

/*[table] TL_EXP_CTRL_PROFILE2*/
#define TL_EXP_CTRL_PROFILE2
#define TL_EXP_CTRL_PROFILE2_ADDRESS 0x7a00
#define TL_EXP_CTRL_PROFILE2_NUM     88
#define TL_EXP_CTRL_PROFILE2_INC     0x4
#define TL_EXP_CTRL_PROFILE2_TYPE    REG_TYPE_RW
#define TL_EXP_CTRL_PROFILE2_DEFAULT 0x0
	/*[field] EXCEP_EN*/
	#define TL_EXP_CTRL_PROFILE2_EXCEP_EN
	#define TL_EXP_CTRL_PROFILE2_EXCEP_EN_OFFSET  0
	#define TL_EXP_CTRL_PROFILE2_EXCEP_EN_LEN     1
	#define TL_EXP_CTRL_PROFILE2_EXCEP_EN_DEFAULT 0x0

struct tl_exp_ctrl_profile2 {
	a_uint32_t  excep_en:1;
	a_uint32_t  _reserved0:31;
};

union tl_exp_ctrl_profile2_u {
	a_uint32_t val;
	struct tl_exp_ctrl_profile2 bf;
};

/*[table] TL_EXP_CTRL_PROFILE3*/
#define TL_EXP_CTRL_PROFILE3
#define TL_EXP_CTRL_PROFILE3_ADDRESS 0x7c00
#define TL_EXP_CTRL_PROFILE3_NUM     88
#define TL_EXP_CTRL_PROFILE3_INC     0x4
#define TL_EXP_CTRL_PROFILE3_TYPE    REG_TYPE_RW
#define TL_EXP_CTRL_PROFILE3_DEFAULT 0x0
	/*[field] EXCEP_EN*/
	#define TL_EXP_CTRL_PROFILE3_EXCEP_EN
	#define TL_EXP_CTRL_PROFILE3_EXCEP_EN_OFFSET  0
	#define TL_EXP_CTRL_PROFILE3_EXCEP_EN_LEN     1
	#define TL_EXP_CTRL_PROFILE3_EXCEP_EN_DEFAULT 0x0

struct tl_exp_ctrl_profile3 {
	a_uint32_t  excep_en:1;
	a_uint32_t  _reserved0:31;
};

union tl_exp_ctrl_profile3_u {
	a_uint32_t val;
	struct tl_exp_ctrl_profile3 bf;
};

/*[register] TPR_L3_EXCEPTION_PARSING_CTRL*/
#define TPR_L3_EXCEPTION_PARSING_CTRL
#define TPR_L3_EXCEPTION_PARSING_CTRL_ADDRESS 0x24
#define TPR_L3_EXCEPTION_PARSING_CTRL_NUM     1
#define TPR_L3_EXCEPTION_PARSING_CTRL_INC     0x4
#define TPR_L3_EXCEPTION_PARSING_CTRL_TYPE    REG_TYPE_RW
#define TPR_L3_EXCEPTION_PARSING_CTRL_DEFAULT 0x0
	/*[field] SMALL_TTL*/
	#define TPR_L3_EXCEPTION_PARSING_CTRL_SMALL_TTL
	#define TPR_L3_EXCEPTION_PARSING_CTRL_SMALL_TTL_OFFSET  0
	#define TPR_L3_EXCEPTION_PARSING_CTRL_SMALL_TTL_LEN     8
	#define TPR_L3_EXCEPTION_PARSING_CTRL_SMALL_TTL_DEFAULT 0x0
	/*[field] SMALL_HOP_LIMIT*/
	#define TPR_L3_EXCEPTION_PARSING_CTRL_SMALL_HOP_LIMIT
	#define TPR_L3_EXCEPTION_PARSING_CTRL_SMALL_HOP_LIMIT_OFFSET  8
	#define TPR_L3_EXCEPTION_PARSING_CTRL_SMALL_HOP_LIMIT_LEN     8
	#define TPR_L3_EXCEPTION_PARSING_CTRL_SMALL_HOP_LIMIT_DEFAULT 0x0

struct tpr_l3_exception_parsing_ctrl {
	a_uint32_t  small_ttl:8;
	a_uint32_t  small_hop_limit:8;
	a_uint32_t  _reserved0:16;
};

union tpr_l3_exception_parsing_ctrl_u {
	a_uint32_t val;
	struct tpr_l3_exception_parsing_ctrl bf;
};

/*[register] TPR_L4_EXCEPTION_PARSING_CTRL_0*/
#define TPR_L4_EXCEPTION_PARSING_CTRL_0
#define TPR_L4_EXCEPTION_PARSING_CTRL_0_ADDRESS 0x28
#define TPR_L4_EXCEPTION_PARSING_CTRL_0_NUM     1
#define TPR_L4_EXCEPTION_PARSING_CTRL_0_INC     0x4
#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TYPE    REG_TYPE_RW
#define TPR_L4_EXCEPTION_PARSING_CTRL_0_DEFAULT 0x0
	/*[field] TCP_FLAGS0*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS0
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS0_OFFSET  0
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS0_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS0_DEFAULT 0x0
	/*[field] TCP_FLAGS0_MASK*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS0_MASK
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS0_MASK_OFFSET  8
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS0_MASK_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS0_MASK_DEFAULT 0x0
	/*[field] TCP_FLAGS1*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS1
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS1_OFFSET  16
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS1_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS1_DEFAULT 0x0
	/*[field] TCP_FLAGS1_MASK*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS1_MASK
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS1_MASK_OFFSET  24
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS1_MASK_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_0_TCP_FLAGS1_MASK_DEFAULT 0x0

struct tpr_l4_exception_parsing_ctrl_0 {
	a_uint32_t  tcp_flags0:6;
	a_uint32_t  _reserved0:2;
	a_uint32_t  tcp_flags0_mask:6;
	a_uint32_t  _reserved1:2;
	a_uint32_t  tcp_flags1:6;
	a_uint32_t  _reserved2:2;
	a_uint32_t  tcp_flags1_mask:6;
	a_uint32_t  _reserved3:2;
};

union tpr_l4_exception_parsing_ctrl_0_u {
	a_uint32_t val;
	struct tpr_l4_exception_parsing_ctrl_0 bf;
};

/*[register] TPR_L4_EXCEPTION_PARSING_CTRL_1*/
#define TPR_L4_EXCEPTION_PARSING_CTRL_1
#define TPR_L4_EXCEPTION_PARSING_CTRL_1_ADDRESS 0x2c
#define TPR_L4_EXCEPTION_PARSING_CTRL_1_NUM     1
#define TPR_L4_EXCEPTION_PARSING_CTRL_1_INC     0x4
#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TYPE    REG_TYPE_RW
#define TPR_L4_EXCEPTION_PARSING_CTRL_1_DEFAULT 0x0
	/*[field] TCP_FLAGS2*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS2
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS2_OFFSET  0
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS2_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS2_DEFAULT 0x0
	/*[field] TCP_FLAGS2_MASK*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS2_MASK
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS2_MASK_OFFSET  8
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS2_MASK_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS2_MASK_DEFAULT 0x0
	/*[field] TCP_FLAGS3*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS3
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS3_OFFSET  16
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS3_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS3_DEFAULT 0x0
	/*[field] TCP_FLAGS3_MASK*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS3_MASK
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS3_MASK_OFFSET  24
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS3_MASK_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_1_TCP_FLAGS3_MASK_DEFAULT 0x0

struct tpr_l4_exception_parsing_ctrl_1 {
	a_uint32_t  tcp_flags2:6;
	a_uint32_t  _reserved0:2;
	a_uint32_t  tcp_flags2_mask:6;
	a_uint32_t  _reserved1:2;
	a_uint32_t  tcp_flags3:6;
	a_uint32_t  _reserved2:2;
	a_uint32_t  tcp_flags3_mask:6;
	a_uint32_t  _reserved3:2;
};

union tpr_l4_exception_parsing_ctrl_1_u {
	a_uint32_t val;
	struct tpr_l4_exception_parsing_ctrl_1 bf;
};

/*[register] TPR_L4_EXCEPTION_PARSING_CTRL_2*/
#define TPR_L4_EXCEPTION_PARSING_CTRL_2
#define TPR_L4_EXCEPTION_PARSING_CTRL_2_ADDRESS 0x30
#define TPR_L4_EXCEPTION_PARSING_CTRL_2_NUM     1
#define TPR_L4_EXCEPTION_PARSING_CTRL_2_INC     0x4
#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TYPE    REG_TYPE_RW
#define TPR_L4_EXCEPTION_PARSING_CTRL_2_DEFAULT 0x0
	/*[field] TCP_FLAGS4*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS4
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS4_OFFSET  0
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS4_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS4_DEFAULT 0x0
	/*[field] TCP_FLAGS4_MASK*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS4_MASK
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS4_MASK_OFFSET  8
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS4_MASK_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS4_MASK_DEFAULT 0x0
	/*[field] TCP_FLAGS5*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS5
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS5_OFFSET  16
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS5_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS5_DEFAULT 0x0
	/*[field] TCP_FLAGS5_MASK*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS5_MASK
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS5_MASK_OFFSET  24
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS5_MASK_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_2_TCP_FLAGS5_MASK_DEFAULT 0x0

struct tpr_l4_exception_parsing_ctrl_2 {
	a_uint32_t  tcp_flags4:6;
	a_uint32_t  _reserved0:2;
	a_uint32_t  tcp_flags4_mask:6;
	a_uint32_t  _reserved1:2;
	a_uint32_t  tcp_flags5:6;
	a_uint32_t  _reserved2:2;
	a_uint32_t  tcp_flags5_mask:6;
	a_uint32_t  _reserved3:2;
};

union tpr_l4_exception_parsing_ctrl_2_u {
	a_uint32_t val;
	struct tpr_l4_exception_parsing_ctrl_2 bf;
};

/*[register] TPR_L4_EXCEPTION_PARSING_CTRL_3*/
#define TPR_L4_EXCEPTION_PARSING_CTRL_3
#define TPR_L4_EXCEPTION_PARSING_CTRL_3_ADDRESS 0x34
#define TPR_L4_EXCEPTION_PARSING_CTRL_3_NUM     1
#define TPR_L4_EXCEPTION_PARSING_CTRL_3_INC     0x4
#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TYPE    REG_TYPE_RW
#define TPR_L4_EXCEPTION_PARSING_CTRL_3_DEFAULT 0x0
	/*[field] TCP_FLAGS6*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS6_OFFSET  0
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS6_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS6_DEFAULT 0x0
	/*[field] TCP_FLAGS6_MASK*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS6_MASK
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS6_MASK_OFFSET  8
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS6_MASK_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS6_MASK_DEFAULT 0x0
	/*[field] TCP_FLAGS7*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS7
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS7_OFFSET  16
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS7_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS7_DEFAULT 0x0
	/*[field] TCP_FLAGS7_MASK*/
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS7_MASK
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS7_MASK_OFFSET  24
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS7_MASK_LEN     6
	#define TPR_L4_EXCEPTION_PARSING_CTRL_3_TCP_FLAGS7_MASK_DEFAULT 0x0

struct tpr_l4_exception_parsing_ctrl_3 {
	a_uint32_t  tcp_flags6:6;
	a_uint32_t  _reserved0:2;
	a_uint32_t  tcp_flags6_mask:6;
	a_uint32_t  _reserved1:2;
	a_uint32_t  tcp_flags7:6;
	a_uint32_t  _reserved2:2;
	a_uint32_t  tcp_flags7_mask:6;
	a_uint32_t  _reserved3:2;
};

union tpr_l4_exception_parsing_ctrl_3_u {
	a_uint32_t val;
	struct tpr_l4_exception_parsing_ctrl_3 bf;
};

/*[register] TPR_EXCEPTION_CTRL_0*/
#define TPR_EXCEPTION_CTRL_0
#define TPR_EXCEPTION_CTRL_0_ADDRESS 0x700
#define TPR_EXCEPTION_CTRL_0_NUM     16
#define TPR_EXCEPTION_CTRL_0_INC     0x4
#define TPR_EXCEPTION_CTRL_0_TYPE    REG_TYPE_RW
#define TPR_EXCEPTION_CTRL_0_DEFAULT 0x0
	/*[field] FLAGS*/
	#define TPR_EXCEPTION_CTRL_0_FLAGS
	#define TPR_EXCEPTION_CTRL_0_FLAGS_OFFSET  0
	#define TPR_EXCEPTION_CTRL_0_FLAGS_LEN     16
	#define TPR_EXCEPTION_CTRL_0_FLAGS_DEFAULT 0x0
	/*[field] MASK*/
	#define TPR_EXCEPTION_CTRL_0_MASK
	#define TPR_EXCEPTION_CTRL_0_MASK_OFFSET  16
	#define TPR_EXCEPTION_CTRL_0_MASK_LEN     16
	#define TPR_EXCEPTION_CTRL_0_MASK_DEFAULT 0x0

struct tpr_exception_ctrl_0 {
	a_uint32_t  flags:16;
	a_uint32_t  mask:16;
};

union tpr_exception_ctrl_0_u {
	a_uint32_t val;
	struct tpr_exception_ctrl_0 bf;
};

/*[register] TPR_EXCEPTION_CTRL_1*/
#define TPR_EXCEPTION_CTRL_1
#define TPR_EXCEPTION_CTRL_1_ADDRESS 0x740
#define TPR_EXCEPTION_CTRL_1_NUM     16
#define TPR_EXCEPTION_CTRL_1_INC     0x4
#define TPR_EXCEPTION_CTRL_1_TYPE    REG_TYPE_RW
#define TPR_EXCEPTION_CTRL_1_DEFAULT 0x0
	/*[field] HDR_TYPE*/
	#define TPR_EXCEPTION_CTRL_1_HDR_TYPE
	#define TPR_EXCEPTION_CTRL_1_HDR_TYPE_OFFSET  0
	#define TPR_EXCEPTION_CTRL_1_HDR_TYPE_LEN     2
	#define TPR_EXCEPTION_CTRL_1_HDR_TYPE_DEFAULT 0x0
	/*[field] COMP_MODE*/
	#define TPR_EXCEPTION_CTRL_1_COMP_MODE
	#define TPR_EXCEPTION_CTRL_1_COMP_MODE_OFFSET  2
	#define TPR_EXCEPTION_CTRL_1_COMP_MODE_LEN     1
	#define TPR_EXCEPTION_CTRL_1_COMP_MODE_DEFAULT 0x0
	/*[field] VALID*/
	#define TPR_EXCEPTION_CTRL_1_VALID
	#define TPR_EXCEPTION_CTRL_1_VALID_OFFSET  3
	#define TPR_EXCEPTION_CTRL_1_VALID_LEN     1
	#define TPR_EXCEPTION_CTRL_1_VALID_DEFAULT 0x0

struct tpr_exception_ctrl_1 {
	a_uint32_t  hdr_type:2;
	a_uint32_t  comp_mode:1;
	a_uint32_t  valid:1;
	a_uint32_t  _reserved0:28;
};

union tpr_exception_ctrl_1_u {
	a_uint32_t val;
	struct tpr_exception_ctrl_1 bf;
};


#endif
#endif
