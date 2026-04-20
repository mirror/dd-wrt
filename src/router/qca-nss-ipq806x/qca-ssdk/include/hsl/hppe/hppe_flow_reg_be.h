/*
 * Copyright (c) 2016-2017, 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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
#ifndef _HPPE_FLOW_REG_H_
#define _HPPE_FLOW_REG_H_

/*[register] FLOW_CTRL0*/
#define FLOW_CTRL0
#if defined(APPE)
#define FLOW_CTRL0_ADDRESS 0x4a8
#else
#define FLOW_CTRL0_ADDRESS 0x368
#endif
#define FLOW_CTRL0_NUM     1
#define FLOW_CTRL0_INC     0x4
#define FLOW_CTRL0_TYPE    REG_TYPE_RW
#if defined(APPE)
#define FLOW_CTRL0_DEFAULT 0x96000c89
#else
#define FLOW_CTRL0_DEFAULT 0xc89
#endif
	/*[field] FLOW_EN*/
	#define FLOW_CTRL0_FLOW_EN
	#define FLOW_CTRL0_FLOW_EN_OFFSET  0
	#define FLOW_CTRL0_FLOW_EN_LEN     1
	#define FLOW_CTRL0_FLOW_EN_DEFAULT 0x1
	/*[field] FLOW_HASH_MODE_0*/
	#define FLOW_CTRL0_FLOW_HASH_MODE_0
	#define FLOW_CTRL0_FLOW_HASH_MODE_0_OFFSET  1
	#define FLOW_CTRL0_FLOW_HASH_MODE_0_LEN     2
	#define FLOW_CTRL0_FLOW_HASH_MODE_0_DEFAULT 0x0
	/*[field] FLOW_HASH_MODE_1*/
	#define FLOW_CTRL0_FLOW_HASH_MODE_1
	#define FLOW_CTRL0_FLOW_HASH_MODE_1_OFFSET  3
	#define FLOW_CTRL0_FLOW_HASH_MODE_1_LEN     2
	#define FLOW_CTRL0_FLOW_HASH_MODE_1_DEFAULT 0x1
	/*[field] FLOW_AGE_TIMER*/
	#define FLOW_CTRL0_FLOW_AGE_TIMER
	#define FLOW_CTRL0_FLOW_AGE_TIMER_OFFSET  5
	#define FLOW_CTRL0_FLOW_AGE_TIMER_LEN     16
	#define FLOW_CTRL0_FLOW_AGE_TIMER_DEFAULT 0x64
	/*[field] FLOW_AGE_TIMER_UNIT*/
	#define FLOW_CTRL0_FLOW_AGE_TIMER_UNIT
	#define FLOW_CTRL0_FLOW_AGE_TIMER_UNIT_OFFSET  21
	#define FLOW_CTRL0_FLOW_AGE_TIMER_UNIT_LEN     2
	#define FLOW_CTRL0_FLOW_AGE_TIMER_UNIT_DEFAULT 0x0
#if defined(APPE)
	/*[field] CLK_FREQ_INMHZ*/
	#define FLOW_CTRL0_CLK_FREQ_INMHZ
	#define FLOW_CTRL0_CLK_FREQ_INMHZ_OFFSET  23
	#define FLOW_CTRL0_CLK_FREQ_INMHZ_LEN     9
	#define FLOW_CTRL0_CLK_FREQ_INMHZ_DEFAULT 0x12c
#endif

struct flow_ctrl0 {
#if defined(APPE)
	a_uint32_t  clk_freq_inmhz:9;
#else
	a_uint32_t  _reserved0:9;
#endif
        a_uint32_t  flow_age_timer_unit:2;
        a_uint32_t  flow_age_timer:16;
        a_uint32_t  flow_hash_mode_1:2;
        a_uint32_t  flow_hash_mode_0:2;
        a_uint32_t  flow_en:1;
};

union flow_ctrl0_u {
	a_uint32_t val;
	struct flow_ctrl0 bf;
};

/*[register] FLOW_CTRL1*/
#define FLOW_CTRL1
#if defined(APPE)
#define FLOW_CTRL1_ADDRESS 0x4bc
#else
#define FLOW_CTRL1_ADDRESS 0x36c
#endif
#define FLOW_CTRL1_NUM     3
#define FLOW_CTRL1_INC     0x4
#define FLOW_CTRL1_TYPE    REG_TYPE_RW
#define FLOW_CTRL1_DEFAULT 0x20000
	/*[field] FLOW_CTL0_MISS_ACTION*/
	#define FLOW_CTRL1_FLOW_CTL0_MISS_ACTION
	#define FLOW_CTRL1_FLOW_CTL0_MISS_ACTION_OFFSET  0
	#define FLOW_CTRL1_FLOW_CTL0_MISS_ACTION_LEN     2
	#define FLOW_CTRL1_FLOW_CTL0_MISS_ACTION_DEFAULT 0x0
	/*[field] FLOW_CTL0_FRAG_BYPASS*/
	#define FLOW_CTRL1_FLOW_CTL0_FRAG_BYPASS
	#define FLOW_CTRL1_FLOW_CTL0_FRAG_BYPASS_OFFSET  2
	#define FLOW_CTRL1_FLOW_CTL0_FRAG_BYPASS_LEN     1
	#define FLOW_CTRL1_FLOW_CTL0_FRAG_BYPASS_DEFAULT 0x0
	/*[field] FLOW_CTL0_TCP_SPECIAL*/
	#define FLOW_CTRL1_FLOW_CTL0_TCP_SPECIAL
	#define FLOW_CTRL1_FLOW_CTL0_TCP_SPECIAL_OFFSET  3
	#define FLOW_CTRL1_FLOW_CTL0_TCP_SPECIAL_LEN     1
	#define FLOW_CTRL1_FLOW_CTL0_TCP_SPECIAL_DEFAULT 0x0
	/*[field] FLOW_CTL0_BYPASS*/
	#define FLOW_CTRL1_FLOW_CTL0_BYPASS
	#define FLOW_CTRL1_FLOW_CTL0_BYPASS_OFFSET  4
	#define FLOW_CTRL1_FLOW_CTL0_BYPASS_LEN     1
	#define FLOW_CTRL1_FLOW_CTL0_BYPASS_DEFAULT 0x0
	/*[field] FLOW_CTL0_KEY_SEL*/
	#define FLOW_CTRL1_FLOW_CTL0_KEY_SEL
	#define FLOW_CTRL1_FLOW_CTL0_KEY_SEL_OFFSET  5
	#define FLOW_CTRL1_FLOW_CTL0_KEY_SEL_LEN     1
	#define FLOW_CTRL1_FLOW_CTL0_KEY_SEL_DEFAULT 0x0
	/*[field] FLOW_CTL1_MISS_ACTION*/
	#define FLOW_CTRL1_FLOW_CTL1_MISS_ACTION
	#define FLOW_CTRL1_FLOW_CTL1_MISS_ACTION_OFFSET  6
	#define FLOW_CTRL1_FLOW_CTL1_MISS_ACTION_LEN     2
	#define FLOW_CTRL1_FLOW_CTL1_MISS_ACTION_DEFAULT 0x0
	/*[field] FLOW_CTL1_FRAG_BYPASS*/
	#define FLOW_CTRL1_FLOW_CTL1_FRAG_BYPASS
	#define FLOW_CTRL1_FLOW_CTL1_FRAG_BYPASS_OFFSET  8
	#define FLOW_CTRL1_FLOW_CTL1_FRAG_BYPASS_LEN     1
	#define FLOW_CTRL1_FLOW_CTL1_FRAG_BYPASS_DEFAULT 0x0
	/*[field] FLOW_CTL1_TCP_SPECIAL*/
	#define FLOW_CTRL1_FLOW_CTL1_TCP_SPECIAL
	#define FLOW_CTRL1_FLOW_CTL1_TCP_SPECIAL_OFFSET  9
	#define FLOW_CTRL1_FLOW_CTL1_TCP_SPECIAL_LEN     1
	#define FLOW_CTRL1_FLOW_CTL1_TCP_SPECIAL_DEFAULT 0x0
	/*[field] FLOW_CTL1_BYPASS*/
	#define FLOW_CTRL1_FLOW_CTL1_BYPASS
	#define FLOW_CTRL1_FLOW_CTL1_BYPASS_OFFSET  10
	#define FLOW_CTRL1_FLOW_CTL1_BYPASS_LEN     1
	#define FLOW_CTRL1_FLOW_CTL1_BYPASS_DEFAULT 0x0
	/*[field] FLOW_CTL1_KEY_SEL*/
	#define FLOW_CTRL1_FLOW_CTL1_KEY_SEL
	#define FLOW_CTRL1_FLOW_CTL1_KEY_SEL_OFFSET  11
	#define FLOW_CTRL1_FLOW_CTL1_KEY_SEL_LEN     1
	#define FLOW_CTRL1_FLOW_CTL1_KEY_SEL_DEFAULT 0x0
	/*[field] FLOW_CTL2_MISS_ACTION*/
	#define FLOW_CTRL1_FLOW_CTL2_MISS_ACTION
	#define FLOW_CTRL1_FLOW_CTL2_MISS_ACTION_OFFSET  12
	#define FLOW_CTRL1_FLOW_CTL2_MISS_ACTION_LEN     2
	#define FLOW_CTRL1_FLOW_CTL2_MISS_ACTION_DEFAULT 0x0
	/*[field] FLOW_CTL2_FRAG_BYPASS*/
	#define FLOW_CTRL1_FLOW_CTL2_FRAG_BYPASS
	#define FLOW_CTRL1_FLOW_CTL2_FRAG_BYPASS_OFFSET  14
	#define FLOW_CTRL1_FLOW_CTL2_FRAG_BYPASS_LEN     1
	#define FLOW_CTRL1_FLOW_CTL2_FRAG_BYPASS_DEFAULT 0x0
	/*[field] FLOW_CTL2_TCP_SPECIAL*/
	#define FLOW_CTRL1_FLOW_CTL2_TCP_SPECIAL
	#define FLOW_CTRL1_FLOW_CTL2_TCP_SPECIAL_OFFSET  15
	#define FLOW_CTRL1_FLOW_CTL2_TCP_SPECIAL_LEN     1
	#define FLOW_CTRL1_FLOW_CTL2_TCP_SPECIAL_DEFAULT 0x0
	/*[field] FLOW_CTL2_BYPASS*/
	#define FLOW_CTRL1_FLOW_CTL2_BYPASS
	#define FLOW_CTRL1_FLOW_CTL2_BYPASS_OFFSET  16
	#define FLOW_CTRL1_FLOW_CTL2_BYPASS_LEN     1
	#define FLOW_CTRL1_FLOW_CTL2_BYPASS_DEFAULT 0x0
	/*[field] FLOW_CTL2_KEY_SEL*/
	#define FLOW_CTRL1_FLOW_CTL2_KEY_SEL
	#define FLOW_CTRL1_FLOW_CTL2_KEY_SEL_OFFSET  17
	#define FLOW_CTRL1_FLOW_CTL2_KEY_SEL_LEN     1
	#define FLOW_CTRL1_FLOW_CTL2_KEY_SEL_DEFAULT 0x1
	/*[field] FLOW_CTL3_MISS_ACTION*/
	#define FLOW_CTRL1_FLOW_CTL3_MISS_ACTION
	#define FLOW_CTRL1_FLOW_CTL3_MISS_ACTION_OFFSET  18
	#define FLOW_CTRL1_FLOW_CTL3_MISS_ACTION_LEN     2
	#define FLOW_CTRL1_FLOW_CTL3_MISS_ACTION_DEFAULT 0x0
	/*[field] FLOW_CTL3_FRAG_BYPASS*/
	#define FLOW_CTRL1_FLOW_CTL3_FRAG_BYPASS
	#define FLOW_CTRL1_FLOW_CTL3_FRAG_BYPASS_OFFSET  20
	#define FLOW_CTRL1_FLOW_CTL3_FRAG_BYPASS_LEN     1
	#define FLOW_CTRL1_FLOW_CTL3_FRAG_BYPASS_DEFAULT 0x0
	/*[field] FLOW_CTL3_TCP_SPECIAL*/
	#define FLOW_CTRL1_FLOW_CTL3_TCP_SPECIAL
	#define FLOW_CTRL1_FLOW_CTL3_TCP_SPECIAL_OFFSET  21
	#define FLOW_CTRL1_FLOW_CTL3_TCP_SPECIAL_LEN     1
	#define FLOW_CTRL1_FLOW_CTL3_TCP_SPECIAL_DEFAULT 0x0
	/*[field] FLOW_CTL3_BYPASS*/
	#define FLOW_CTRL1_FLOW_CTL3_BYPASS
	#define FLOW_CTRL1_FLOW_CTL3_BYPASS_OFFSET  22
	#define FLOW_CTRL1_FLOW_CTL3_BYPASS_LEN     1
	#define FLOW_CTRL1_FLOW_CTL3_BYPASS_DEFAULT 0x0
	/*[field] FLOW_CTL3_KEY_SEL*/
	#define FLOW_CTRL1_FLOW_CTL3_KEY_SEL
	#define FLOW_CTRL1_FLOW_CTL3_KEY_SEL_OFFSET  23
	#define FLOW_CTRL1_FLOW_CTL3_KEY_SEL_LEN     1
	#define FLOW_CTRL1_FLOW_CTL3_KEY_SEL_DEFAULT 0x0
	/*[field] FLOW_CTL4_MISS_ACTION*/
	#define FLOW_CTRL1_FLOW_CTL4_MISS_ACTION
	#define FLOW_CTRL1_FLOW_CTL4_MISS_ACTION_OFFSET  24
	#define FLOW_CTRL1_FLOW_CTL4_MISS_ACTION_LEN     2
	#define FLOW_CTRL1_FLOW_CTL4_MISS_ACTION_DEFAULT 0x0
	/*[field] FLOW_CTL4_FRAG_BYPASS*/
	#define FLOW_CTRL1_FLOW_CTL4_FRAG_BYPASS
	#define FLOW_CTRL1_FLOW_CTL4_FRAG_BYPASS_OFFSET  26
	#define FLOW_CTRL1_FLOW_CTL4_FRAG_BYPASS_LEN     1
	#define FLOW_CTRL1_FLOW_CTL4_FRAG_BYPASS_DEFAULT 0x0
	/*[field] FLOW_CTL4_TCP_SPECIAL*/
	#define FLOW_CTRL1_FLOW_CTL4_TCP_SPECIAL
	#define FLOW_CTRL1_FLOW_CTL4_TCP_SPECIAL_OFFSET  27
	#define FLOW_CTRL1_FLOW_CTL4_TCP_SPECIAL_LEN     1
	#define FLOW_CTRL1_FLOW_CTL4_TCP_SPECIAL_DEFAULT 0x0
	/*[field] FLOW_CTL4_BYPASS*/
	#define FLOW_CTRL1_FLOW_CTL4_BYPASS
	#define FLOW_CTRL1_FLOW_CTL4_BYPASS_OFFSET  28
	#define FLOW_CTRL1_FLOW_CTL4_BYPASS_LEN     1
	#define FLOW_CTRL1_FLOW_CTL4_BYPASS_DEFAULT 0x0
	/*[field] FLOW_CTL4_KEY_SEL*/
	#define FLOW_CTRL1_FLOW_CTL4_KEY_SEL
	#define FLOW_CTRL1_FLOW_CTL4_KEY_SEL_OFFSET  29
	#define FLOW_CTRL1_FLOW_CTL4_KEY_SEL_LEN     1
	#define FLOW_CTRL1_FLOW_CTL4_KEY_SEL_DEFAULT 0x0

struct flow_ctrl1 {
	a_uint32_t  _reserved0:2;
	a_uint32_t  flow_ctl4_key_sel:1;
	a_uint32_t  flow_ctl4_bypass:1;
	a_uint32_t  flow_ctl4_tcp_special:1;
	a_uint32_t  flow_ctl4_frag_bypass:1;
	a_uint32_t  flow_ctl4_miss_action:2;
	a_uint32_t  flow_ctl3_key_sel:1;
	a_uint32_t  flow_ctl3_bypass:1;
	a_uint32_t  flow_ctl3_tcp_special:1;
	a_uint32_t  flow_ctl3_frag_bypass:1;
	a_uint32_t  flow_ctl3_miss_action:2;
	a_uint32_t  flow_ctl2_key_sel:1;
	a_uint32_t  flow_ctl2_bypass:1;
	a_uint32_t  flow_ctl2_tcp_special:1;
	a_uint32_t  flow_ctl2_frag_bypass:1;
	a_uint32_t  flow_ctl2_miss_action:2;
	a_uint32_t  flow_ctl1_key_sel:1;
	a_uint32_t  flow_ctl1_bypass:1;
	a_uint32_t  flow_ctl1_tcp_special:1;
	a_uint32_t  flow_ctl1_frag_bypass:1;
	a_uint32_t  flow_ctl1_miss_action:2;
	a_uint32_t  flow_ctl0_key_sel:1;
	a_uint32_t  flow_ctl0_bypass:1;
	a_uint32_t  flow_ctl0_tcp_special:1;
	a_uint32_t  flow_ctl0_frag_bypass:1;
	a_uint32_t  flow_ctl0_miss_action:2;
};

union flow_ctrl1_u {
	a_uint32_t val;
	struct flow_ctrl1 bf;
};

/*[register] IN_FLOW_TBL_OP*/
#define IN_FLOW_TBL_OP
#if defined(APPE)
#define IN_FLOW_TBL_OP_ADDRESS 0x528
#else
#define IN_FLOW_TBL_OP_ADDRESS 0x3b8
#endif
#define IN_FLOW_TBL_OP_NUM     1
#define IN_FLOW_TBL_OP_INC     0x4
#define IN_FLOW_TBL_OP_TYPE    REG_TYPE_RW
#define IN_FLOW_TBL_OP_DEFAULT 0x0
	/*[field] CMD_ID*/
	#define IN_FLOW_TBL_OP_CMD_ID
	#define IN_FLOW_TBL_OP_CMD_ID_OFFSET  0
	#define IN_FLOW_TBL_OP_CMD_ID_LEN     4
	#define IN_FLOW_TBL_OP_CMD_ID_DEFAULT 0x0
	/*[field] BYP_RSLT_EN*/
	#define IN_FLOW_TBL_OP_BYP_RSLT_EN
	#define IN_FLOW_TBL_OP_BYP_RSLT_EN_OFFSET  4
	#define IN_FLOW_TBL_OP_BYP_RSLT_EN_LEN     1
	#define IN_FLOW_TBL_OP_BYP_RSLT_EN_DEFAULT 0x0
	/*[field] OP_TYPE*/
	#define IN_FLOW_TBL_OP_OP_TYPE
	#define IN_FLOW_TBL_OP_OP_TYPE_OFFSET  5
	#define IN_FLOW_TBL_OP_OP_TYPE_LEN     3
	#define IN_FLOW_TBL_OP_OP_TYPE_DEFAULT 0x0
	/*[field] HASH_BLOCK_BITMAP*/
	#define IN_FLOW_TBL_OP_HASH_BLOCK_BITMAP
	#define IN_FLOW_TBL_OP_HASH_BLOCK_BITMAP_OFFSET  8
	#define IN_FLOW_TBL_OP_HASH_BLOCK_BITMAP_LEN     2
	#define IN_FLOW_TBL_OP_HASH_BLOCK_BITMAP_DEFAULT 0x0
	/*[field] OP_MODE*/
	#define IN_FLOW_TBL_OP_OP_MODE
	#define IN_FLOW_TBL_OP_OP_MODE_OFFSET  10
	#define IN_FLOW_TBL_OP_OP_MODE_LEN     1
	#define IN_FLOW_TBL_OP_OP_MODE_DEFAULT 0x0
	/*[field] OP_HOST_EN*/
	#define IN_FLOW_TBL_OP_OP_HOST_EN
	#define IN_FLOW_TBL_OP_OP_HOST_EN_OFFSET  11
	#define IN_FLOW_TBL_OP_OP_HOST_EN_LEN     1
	#define IN_FLOW_TBL_OP_OP_HOST_EN_DEFAULT 0x0
	/*[field] ENTRY_INDEX*/
	#define IN_FLOW_TBL_OP_ENTRY_INDEX
	#define IN_FLOW_TBL_OP_ENTRY_INDEX_OFFSET  12
	#define IN_FLOW_TBL_OP_ENTRY_INDEX_LEN     12
	#define IN_FLOW_TBL_OP_ENTRY_INDEX_DEFAULT 0x0
	/*[field] OP_RESULT*/
	#define IN_FLOW_TBL_OP_OP_RESULT
	#define IN_FLOW_TBL_OP_OP_RESULT_OFFSET  24
	#define IN_FLOW_TBL_OP_OP_RESULT_LEN     1
	#define IN_FLOW_TBL_OP_OP_RESULT_DEFAULT 0x0
	/*[field] BUSY*/
	#define IN_FLOW_TBL_OP_BUSY
	#define IN_FLOW_TBL_OP_BUSY_OFFSET  25
	#define IN_FLOW_TBL_OP_BUSY_LEN     1
	#define IN_FLOW_TBL_OP_BUSY_DEFAULT 0x0

struct in_flow_tbl_op {
	a_uint32_t  _reserved0:6;
	a_uint32_t  busy:1;
	a_uint32_t  op_result:1;
	a_uint32_t  entry_index:12;
	a_uint32_t  op_host_en:1;
	a_uint32_t  op_mode:1;
	a_uint32_t  hash_block_bitmap:2;
	a_uint32_t  op_type:3;
	a_uint32_t  byp_rslt_en:1;
	a_uint32_t  cmd_id:4;
};

union in_flow_tbl_op_u {
	a_uint32_t val;
	struct in_flow_tbl_op bf;
};

/*[register] IN_FLOW_HOST_TBL_OP*/
#define IN_FLOW_HOST_TBL_OP
#if defined(APPE)
#define IN_FLOW_HOST_TBL_OP_ADDRESS 0x53c
#else
#define IN_FLOW_HOST_TBL_OP_ADDRESS 0x3bc
#endif
#define IN_FLOW_HOST_TBL_OP_NUM     1
#define IN_FLOW_HOST_TBL_OP_INC     0x4
#define IN_FLOW_HOST_TBL_OP_TYPE    REG_TYPE_RW
#define IN_FLOW_HOST_TBL_OP_DEFAULT 0x0
	/*[field] HASH_BLOCK_BITMAP*/
	#define IN_FLOW_HOST_TBL_OP_HASH_BLOCK_BITMAP
	#define IN_FLOW_HOST_TBL_OP_HASH_BLOCK_BITMAP_OFFSET  0
	#define IN_FLOW_HOST_TBL_OP_HASH_BLOCK_BITMAP_LEN     2
	#define IN_FLOW_HOST_TBL_OP_HASH_BLOCK_BITMAP_DEFAULT 0x0
	/*[field] HOST_ENTRY_INDEX*/
	#define IN_FLOW_HOST_TBL_OP_HOST_ENTRY_INDEX
	#define IN_FLOW_HOST_TBL_OP_HOST_ENTRY_INDEX_OFFSET  2
	#define IN_FLOW_HOST_TBL_OP_HOST_ENTRY_INDEX_LEN     13
	#define IN_FLOW_HOST_TBL_OP_HOST_ENTRY_INDEX_DEFAULT 0x0

struct in_flow_host_tbl_op {
	a_uint32_t  _reserved0:17;
	a_uint32_t  host_entry_index:13;
	a_uint32_t  hash_block_bitmap:2;
};

union in_flow_host_tbl_op_u {
	a_uint32_t val;
	struct in_flow_host_tbl_op bf;
};

/*[register] IN_FLOW_TBL_OP_DATA0*/
#define IN_FLOW_TBL_OP_DATA
#if defined(APPE)
#define IN_FLOW_TBL_OP_DATA_ADDRESS 0x550
#define IN_FLOW_TBL_OP_DATA_NUM     11
#else
#define IN_FLOW_TBL_OP_DATA_ADDRESS 0x3c0
#define IN_FLOW_TBL_OP_DATA_NUM     9
#endif
#define IN_FLOW_TBL_OP_DATA_INC     0x4
#define IN_FLOW_TBL_OP_DATA_TYPE    REG_TYPE_RW
#define IN_FLOW_TBL_OP_DATA_DEFAULT 0x0
	/*[field] DATA*/
	#define IN_FLOW_TBL_OP_DATA_DATA
	#define IN_FLOW_TBL_OP_DATA_DATA_OFFSET  0
	#define IN_FLOW_TBL_OP_DATA_DATA_LEN     32
	#define IN_FLOW_TBL_OP_DATA_DATA_DEFAULT 0x0

struct in_flow_tbl_op_data {
	a_uint32_t  data:32;
};

union in_flow_tbl_op_data_u {
	a_uint32_t val;
	struct in_flow_tbl_op_data bf;
};

/*[register] FLOW_HOST_TBL_OP_DATA*/
#define FLOW_HOST_TBL_OP_DATA
#if defined(APPE)
#define FLOW_HOST_TBL_OP_DATA_ADDRESS 0x58c
#else
#define FLOW_HOST_TBL_OP_DATA_ADDRESS 0x3e4
#endif
#define FLOW_HOST_TBL_OP_DATA_NUM     10
#define FLOW_HOST_TBL_OP_DATA_INC     0x4
#define FLOW_HOST_TBL_OP_DATA_TYPE    REG_TYPE_RW
#define FLOW_HOST_TBL_OP_DATA_DEFAULT 0x0
	/*[field] DATA*/
	#define FLOW_HOST_TBL_OP_DATA_DATA
	#define FLOW_HOST_TBL_OP_DATA_DATA_OFFSET  0
	#define FLOW_HOST_TBL_OP_DATA_DATA_LEN     32
	#define FLOW_HOST_TBL_OP_DATA_DATA_DEFAULT 0x0

struct flow_host_tbl_op_data {
	a_uint32_t  data:32;
};

union flow_host_tbl_op_data_u {
	a_uint32_t val;
	struct flow_host_tbl_op_data bf;
};

/*[register] IN_FLOW_TBL_OP_RSLT*/
#define IN_FLOW_TBL_OP_RSLT
#if defined(APPE)
#define IN_FLOW_TBL_OP_RSLT_ADDRESS 0x5c4
#else
#define IN_FLOW_TBL_OP_RSLT_ADDRESS 0x40c
#endif
#define IN_FLOW_TBL_OP_RSLT_NUM     10
#define IN_FLOW_TBL_OP_RSLT_INC     0x4
#define IN_FLOW_TBL_OP_RSLT_TYPE    REG_TYPE_RO
#define IN_FLOW_TBL_OP_RSLT_DEFAULT 0x0
	/*[field] CMD_ID*/
	#define IN_FLOW_TBL_OP_RSLT_CMD_ID
	#define IN_FLOW_TBL_OP_RSLT_CMD_ID_OFFSET  0
	#define IN_FLOW_TBL_OP_RSLT_CMD_ID_LEN     4
	#define IN_FLOW_TBL_OP_RSLT_CMD_ID_DEFAULT 0x0
	/*[field] OP_RSLT*/
	#define IN_FLOW_TBL_OP_RSLT_OP_RSLT
	#define IN_FLOW_TBL_OP_RSLT_OP_RSLT_OFFSET  4
	#define IN_FLOW_TBL_OP_RSLT_OP_RSLT_LEN     1
	#define IN_FLOW_TBL_OP_RSLT_OP_RSLT_DEFAULT 0x0
	/*[field] FLOW_ENTRY_INDEX*/
	#define IN_FLOW_TBL_OP_RSLT_FLOW_ENTRY_INDEX
	#define IN_FLOW_TBL_OP_RSLT_FLOW_ENTRY_INDEX_OFFSET  5
	#define IN_FLOW_TBL_OP_RSLT_FLOW_ENTRY_INDEX_LEN     12
	#define IN_FLOW_TBL_OP_RSLT_FLOW_ENTRY_INDEX_DEFAULT 0x0
	/*[field] VALID_CNT*/
	#define IN_FLOW_TBL_OP_RSLT_VALID_CNT
	#define IN_FLOW_TBL_OP_RSLT_VALID_CNT_OFFSET  17
	#define IN_FLOW_TBL_OP_RSLT_VALID_CNT_LEN     4
	#define IN_FLOW_TBL_OP_RSLT_VALID_CNT_DEFAULT 0x0

struct in_flow_tbl_op_rslt {
	a_uint32_t  _reserved0:11;
	a_uint32_t  valid_cnt:4;
	a_uint32_t  flow_entry_index:12;
	a_uint32_t  op_rslt:1;
	a_uint32_t  cmd_id:4;
};

union in_flow_tbl_op_rslt_u {
	a_uint32_t val;
	struct in_flow_tbl_op_rslt bf;
};

/*[register] FLOW_HOST_TBL_OP_RSLT*/
#define FLOW_HOST_TBL_OP_RSLT
#if defined(APPE)
#define FLOW_HOST_TBL_OP_RSLT_ADDRESS 0x5d8
#else
#define FLOW_HOST_TBL_OP_RSLT_ADDRESS 0x410
#endif
#define FLOW_HOST_TBL_OP_RSLT_NUM     1
#define FLOW_HOST_TBL_OP_RSLT_INC     0x4
#define FLOW_HOST_TBL_OP_RSLT_TYPE    REG_TYPE_RO
#define FLOW_HOST_TBL_OP_RSLT_DEFAULT 0x0
	/*[field] HOST_ENTRY_INDEX*/
	#define FLOW_HOST_TBL_OP_RSLT_HOST_ENTRY_INDEX
	#define FLOW_HOST_TBL_OP_RSLT_HOST_ENTRY_INDEX_OFFSET  0
	#define FLOW_HOST_TBL_OP_RSLT_HOST_ENTRY_INDEX_LEN     13
	#define FLOW_HOST_TBL_OP_RSLT_HOST_ENTRY_INDEX_DEFAULT 0x0

struct flow_host_tbl_op_rslt {
	a_uint32_t  _reserved0:19;
        a_uint32_t  host_entry_index:13;
};

union flow_host_tbl_op_rslt_u {
	a_uint32_t val;
	struct flow_host_tbl_op_rslt bf;
};

/*[register] IN_FLOW_TBL_RD_OP*/
#define IN_FLOW_TBL_RD_OP
#if defined(APPE)
#define IN_FLOW_TBL_RD_OP_ADDRESS 0x5ec
#else
#define IN_FLOW_TBL_RD_OP_ADDRESS 0x414
#endif
#define IN_FLOW_TBL_RD_OP_NUM     1
#define IN_FLOW_TBL_RD_OP_INC     0x4
#define IN_FLOW_TBL_RD_OP_TYPE    REG_TYPE_RW
#define IN_FLOW_TBL_RD_OP_DEFAULT 0x0
	/*[field] CMD_ID*/
	#define IN_FLOW_TBL_RD_OP_CMD_ID
	#define IN_FLOW_TBL_RD_OP_CMD_ID_OFFSET  0
	#define IN_FLOW_TBL_RD_OP_CMD_ID_LEN     4
	#define IN_FLOW_TBL_RD_OP_CMD_ID_DEFAULT 0x0
	/*[field] BYP_RSLT_EN*/
	#define IN_FLOW_TBL_RD_OP_BYP_RSLT_EN
	#define IN_FLOW_TBL_RD_OP_BYP_RSLT_EN_OFFSET  4
	#define IN_FLOW_TBL_RD_OP_BYP_RSLT_EN_LEN     1
	#define IN_FLOW_TBL_RD_OP_BYP_RSLT_EN_DEFAULT 0x0
	/*[field] OP_TYPE*/
	#define IN_FLOW_TBL_RD_OP_OP_TYPE
	#define IN_FLOW_TBL_RD_OP_OP_TYPE_OFFSET  5
	#define IN_FLOW_TBL_RD_OP_OP_TYPE_LEN     3
	#define IN_FLOW_TBL_RD_OP_OP_TYPE_DEFAULT 0x0
	/*[field] HASH_BLOCK_BITMAP*/
	#define IN_FLOW_TBL_RD_OP_HASH_BLOCK_BITMAP
	#define IN_FLOW_TBL_RD_OP_HASH_BLOCK_BITMAP_OFFSET  8
	#define IN_FLOW_TBL_RD_OP_HASH_BLOCK_BITMAP_LEN     2
	#define IN_FLOW_TBL_RD_OP_HASH_BLOCK_BITMAP_DEFAULT 0x0
	/*[field] OP_MODE*/
	#define IN_FLOW_TBL_RD_OP_OP_MODE
	#define IN_FLOW_TBL_RD_OP_OP_MODE_OFFSET  10
	#define IN_FLOW_TBL_RD_OP_OP_MODE_LEN     1
	#define IN_FLOW_TBL_RD_OP_OP_MODE_DEFAULT 0x0
	/*[field] OP_HOST_EN*/
	#define IN_FLOW_TBL_RD_OP_OP_HOST_EN
	#define IN_FLOW_TBL_RD_OP_OP_HOST_EN_OFFSET  11
	#define IN_FLOW_TBL_RD_OP_OP_HOST_EN_LEN     1
	#define IN_FLOW_TBL_RD_OP_OP_HOST_EN_DEFAULT 0x0
	/*[field] ENTRY_INDEX*/
	#define IN_FLOW_TBL_RD_OP_ENTRY_INDEX
	#define IN_FLOW_TBL_RD_OP_ENTRY_INDEX_OFFSET  12
	#define IN_FLOW_TBL_RD_OP_ENTRY_INDEX_LEN     12
	#define IN_FLOW_TBL_RD_OP_ENTRY_INDEX_DEFAULT 0x0
	/*[field] OP_RESULT*/
	#define IN_FLOW_TBL_RD_OP_OP_RESULT
	#define IN_FLOW_TBL_RD_OP_OP_RESULT_OFFSET  24
	#define IN_FLOW_TBL_RD_OP_OP_RESULT_LEN     1
	#define IN_FLOW_TBL_RD_OP_OP_RESULT_DEFAULT 0x0
	/*[field] BUSY*/
	#define IN_FLOW_TBL_RD_OP_BUSY
	#define IN_FLOW_TBL_RD_OP_BUSY_OFFSET  25
	#define IN_FLOW_TBL_RD_OP_BUSY_LEN     1
	#define IN_FLOW_TBL_RD_OP_BUSY_DEFAULT 0x0

struct in_flow_tbl_rd_op {
	a_uint32_t  _reserved0:6;
	a_uint32_t  busy:1;
	a_uint32_t  op_result:1;
	a_uint32_t  entry_index:12;
	a_uint32_t  op_host_en:1;
	a_uint32_t  op_mode:1;
	a_uint32_t  hash_block_bitmap:2;
	a_uint32_t  op_type:3;
	a_uint32_t  byp_rslt_en:1;
	a_uint32_t  cmd_id:4;
};


union in_flow_tbl_rd_op_u {
	a_uint32_t val;
	struct in_flow_tbl_rd_op bf;
};

/*[register] IN_FLOW_HOST_TBL_RD_OP*/
#define IN_FLOW_HOST_TBL_RD_OP
#if defined(APPE)
#define IN_FLOW_HOST_TBL_RD_OP_ADDRESS 0x600
#else
#define IN_FLOW_HOST_TBL_RD_OP_ADDRESS 0x418
#endif
#define IN_FLOW_HOST_TBL_RD_OP_NUM     1
#define IN_FLOW_HOST_TBL_RD_OP_INC     0x4
#define IN_FLOW_HOST_TBL_RD_OP_TYPE    REG_TYPE_RW
#define IN_FLOW_HOST_TBL_RD_OP_DEFAULT 0x0
	/*[field] HASH_BLOCK_BITMAP*/
	#define IN_FLOW_HOST_TBL_RD_OP_HASH_BLOCK_BITMAP
	#define IN_FLOW_HOST_TBL_RD_OP_HASH_BLOCK_BITMAP_OFFSET  0
	#define IN_FLOW_HOST_TBL_RD_OP_HASH_BLOCK_BITMAP_LEN     2
	#define IN_FLOW_HOST_TBL_RD_OP_HASH_BLOCK_BITMAP_DEFAULT 0x0
	/*[field] HOST_ENTRY_INDEX*/
	#define IN_FLOW_HOST_TBL_RD_OP_HOST_ENTRY_INDEX
	#define IN_FLOW_HOST_TBL_RD_OP_HOST_ENTRY_INDEX_OFFSET  2
	#define IN_FLOW_HOST_TBL_RD_OP_HOST_ENTRY_INDEX_LEN     13
	#define IN_FLOW_HOST_TBL_RD_OP_HOST_ENTRY_INDEX_DEFAULT 0x0

struct in_flow_host_tbl_rd_op {
	a_uint32_t  _reserved0:17;
	a_uint32_t  host_entry_index:13;
	a_uint32_t  hash_block_bitmap:2;
};

union in_flow_host_tbl_rd_op_u {
	a_uint32_t val;
	struct in_flow_host_tbl_rd_op bf;
};

/*[register] IN_FLOW_TBL_RD_OP_DATA*/
#define IN_FLOW_TBL_RD_OP_DATA
#if defined(APPE)
#define IN_FLOW_TBL_RD_OP_DATA_ADDRESS 0x614
#define IN_FLOW_TBL_RD_OP_DATA_NUM     11
#else
#define IN_FLOW_TBL_RD_OP_DATA_ADDRESS 0x41c
#define IN_FLOW_TBL_RD_OP_DATA_NUM     9
#endif
#define IN_FLOW_TBL_RD_OP_DATA_INC     0x4
#define IN_FLOW_TBL_RD_OP_DATA_TYPE    REG_TYPE_RW
#define IN_FLOW_TBL_RD_OP_DATA_DEFAULT 0x0
	/*[field] DATA*/
	#define IN_FLOW_TBL_RD_OP_DATA_DATA
	#define IN_FLOW_TBL_RD_OP_DATA_DATA_OFFSET  0
	#define IN_FLOW_TBL_RD_OP_DATA_DATA_LEN     32
	#define IN_FLOW_TBL_RD_OP_DATA_DATA_DEFAULT 0x0

struct in_flow_tbl_rd_op_data {
	a_uint32_t  data:32;
};

union in_flow_tbl_rd_op_data_u {
	a_uint32_t val;
	struct in_flow_tbl_rd_op_data bf;
};

/*[register] FLOW_HOST_TBL_RD_OP_DATA*/
#define FLOW_HOST_TBL_RD_OP_DATA
#if defined(APPE)
#define FLOW_HOST_TBL_RD_OP_DATA_ADDRESS 0x650
#else
#define FLOW_HOST_TBL_RD_OP_DATA_ADDRESS 0x440
#endif
#define FLOW_HOST_TBL_RD_OP_DATA_NUM     10
#define FLOW_HOST_TBL_RD_OP_DATA_INC     0x4
#define FLOW_HOST_TBL_RD_OP_DATA_TYPE    REG_TYPE_RW
#define FLOW_HOST_TBL_RD_OP_DATA_DEFAULT 0x0
	/*[field] DATA*/
	#define FLOW_HOST_TBL_RD_OP_DATA_DATA
	#define FLOW_HOST_TBL_RD_OP_DATA_DATA_OFFSET  0
	#define FLOW_HOST_TBL_RD_OP_DATA_DATA_LEN     32
	#define FLOW_HOST_TBL_RD_OP_DATA_DATA_DEFAULT 0x0

struct flow_host_tbl_rd_op_data {
	a_uint32_t  data:32;
};

union flow_host_tbl_rd_op_data_u {
	a_uint32_t val;
	struct flow_host_tbl_rd_op_data bf;
};

/*[register] IN_FLOW_TBL_RD_OP_RSLT*/
#define IN_FLOW_TBL_RD_OP_RSLT
#if defined(APPE)
#define IN_FLOW_TBL_RD_OP_RSLT_ADDRESS 0x688
#else
#define IN_FLOW_TBL_RD_OP_RSLT_ADDRESS 0x468
#endif
#define IN_FLOW_TBL_RD_OP_RSLT_NUM     1
#define IN_FLOW_TBL_RD_OP_RSLT_INC     0x4
#define IN_FLOW_TBL_RD_OP_RSLT_TYPE    REG_TYPE_RO
#define IN_FLOW_TBL_RD_OP_RSLT_DEFAULT 0x0
	/*[field] CMD_ID*/
	#define IN_FLOW_TBL_RD_OP_RSLT_CMD_ID
	#define IN_FLOW_TBL_RD_OP_RSLT_CMD_ID_OFFSET  0
	#define IN_FLOW_TBL_RD_OP_RSLT_CMD_ID_LEN     4
	#define IN_FLOW_TBL_RD_OP_RSLT_CMD_ID_DEFAULT 0x0
	/*[field] OP_RSLT*/
	#define IN_FLOW_TBL_RD_OP_RSLT_OP_RSLT
	#define IN_FLOW_TBL_RD_OP_RSLT_OP_RSLT_OFFSET  4
	#define IN_FLOW_TBL_RD_OP_RSLT_OP_RSLT_LEN     1
	#define IN_FLOW_TBL_RD_OP_RSLT_OP_RSLT_DEFAULT 0x0
	/*[field] FLOW_ENTRY_INDEX*/
	#define IN_FLOW_TBL_RD_OP_RSLT_FLOW_ENTRY_INDEX
	#define IN_FLOW_TBL_RD_OP_RSLT_FLOW_ENTRY_INDEX_OFFSET  5
	#define IN_FLOW_TBL_RD_OP_RSLT_FLOW_ENTRY_INDEX_LEN     12
	#define IN_FLOW_TBL_RD_OP_RSLT_FLOW_ENTRY_INDEX_DEFAULT 0x0
	/*[field] VALID_CNT*/
	#define IN_FLOW_TBL_RD_OP_RSLT_VALID_CNT
	#define IN_FLOW_TBL_RD_OP_RSLT_VALID_CNT_OFFSET  17
	#define IN_FLOW_TBL_RD_OP_RSLT_VALID_CNT_LEN     4
	#define IN_FLOW_TBL_RD_OP_RSLT_VALID_CNT_DEFAULT 0x0

struct in_flow_tbl_rd_op_rslt {
	a_uint32_t  _reserved0:11;
	a_uint32_t  valid_cnt:4;
	a_uint32_t  flow_entry_index:12;
	a_uint32_t  op_rslt:1;
	a_uint32_t  cmd_id:4;
};

union in_flow_tbl_rd_op_rslt_u {
	a_uint32_t val;
	struct in_flow_tbl_rd_op_rslt bf;
};

/*[register] FLOW_HOST_TBL_RD_OP_RSLT*/
#define FLOW_HOST_TBL_RD_OP_RSLT
#if defined(APPE)
#define FLOW_HOST_TBL_RD_OP_RSLT_ADDRESS 0x69c
#else
#define FLOW_HOST_TBL_RD_OP_RSLT_ADDRESS 0x46c
#endif
#define FLOW_HOST_TBL_RD_OP_RSLT_NUM     1
#define FLOW_HOST_TBL_RD_OP_RSLT_INC     0x4
#define FLOW_HOST_TBL_RD_OP_RSLT_TYPE    REG_TYPE_RO
#define FLOW_HOST_TBL_RD_OP_RSLT_DEFAULT 0x0
	/*[field] HOST_ENTRY_INDEX*/
	#define FLOW_HOST_TBL_RD_OP_RSLT_HOST_ENTRY_INDEX
	#define FLOW_HOST_TBL_RD_OP_RSLT_HOST_ENTRY_INDEX_OFFSET  0
	#define FLOW_HOST_TBL_RD_OP_RSLT_HOST_ENTRY_INDEX_LEN     13
	#define FLOW_HOST_TBL_RD_OP_RSLT_HOST_ENTRY_INDEX_DEFAULT 0x0

struct flow_host_tbl_rd_op_rslt {
	a_uint32_t  _reserved0:19;
        a_uint32_t  host_entry_index:13;
};

union flow_host_tbl_rd_op_rslt_u {
	a_uint32_t val;
	struct flow_host_tbl_rd_op_rslt bf;
};

/*[register] IN_FLOW_TBL_RD_RSLT_DATA*/
#define IN_FLOW_TBL_RD_RSLT_DATA
#if defined(APPE)
#define IN_FLOW_TBL_RD_RSLT_DATA_ADDRESS 0x6b0
#define IN_FLOW_TBL_RD_RSLT_DATA_NUM     11
#else
#define IN_FLOW_TBL_RD_RSLT_DATA_ADDRESS 0x470
#define IN_FLOW_TBL_RD_RSLT_DATA_NUM     9
#endif
#define IN_FLOW_TBL_RD_RSLT_DATA_INC     0x4
#define IN_FLOW_TBL_RD_RSLT_DATA_TYPE    REG_TYPE_RO
#define IN_FLOW_TBL_RD_RSLT_DATA_DEFAULT 0x0
	/*[field] DATA*/
	#define IN_FLOW_TBL_RD_RSLT_DATA_DATA
	#define IN_FLOW_TBL_RD_RSLT_DATA_DATA_OFFSET  0
	#define IN_FLOW_TBL_RD_RSLT_DATA_DATA_LEN     32
	#define IN_FLOW_TBL_RD_RSLT_DATA_DATA_DEFAULT 0x0

struct in_flow_tbl_rd_rslt_data {
	a_uint32_t  data:32;
};

union in_flow_tbl_rd_rslt_data_u {
	a_uint32_t val;
	struct in_flow_tbl_rd_rslt_data bf;
};

/*[register] FLOW_HOST_TBL_RD_RSLT_DATA*/
#define FLOW_HOST_TBL_RD_RSLT_DATA
#if defined(APPE)
#define FLOW_HOST_TBL_RD_RSLT_DATA_ADDRESS 0x6ec
#else
#define FLOW_HOST_TBL_RD_RSLT_DATA_ADDRESS 0x494
#endif
#define FLOW_HOST_TBL_RD_RSLT_DATA_NUM     10
#define FLOW_HOST_TBL_RD_RSLT_DATA_INC     0x4
#define FLOW_HOST_TBL_RD_RSLT_DATA_TYPE    REG_TYPE_RO
#define FLOW_HOST_TBL_RD_RSLT_DATA_DEFAULT 0x0
	/*[field] DATA*/
	#define FLOW_HOST_TBL_RD_RSLT_DATA_DATA
	#define FLOW_HOST_TBL_RD_RSLT_DATA_DATA_OFFSET  0
	#define FLOW_HOST_TBL_RD_RSLT_DATA_DATA_LEN     32
	#define FLOW_HOST_TBL_RD_RSLT_DATA_DATA_DEFAULT 0x0

struct flow_host_tbl_rd_rslt_data {
	a_uint32_t  data:32;
};

union flow_host_tbl_rd_rslt_data_u {
	a_uint32_t val;
	struct flow_host_tbl_rd_rslt_data bf;
};

/*[table] IN_FLOW_3TUPLE_TBL*/
#define IN_FLOW_3TUPLE_TBL
#define IN_FLOW_3TUPLE_TBL_ADDRESS 0x40000
#define IN_FLOW_3TUPLE_TBL_NUM     4096
#define IN_FLOW_3TUPLE_TBL_INC     0x20
#define IN_FLOW_3TUPLE_TBL_TYPE    REG_TYPE_RW
#define IN_FLOW_3TUPLE_TBL_DEFAULT 0x0
	/*[field] VALID*/
	#define IN_FLOW_3TUPLE_TBL_VALID
	#define IN_FLOW_3TUPLE_TBL_VALID_OFFSET  0
	#define IN_FLOW_3TUPLE_TBL_VALID_LEN     1
	#define IN_FLOW_3TUPLE_TBL_VALID_DEFAULT 0x0
	/*[field] ENTRY_TYPE*/
	#define IN_FLOW_3TUPLE_TBL_ENTRY_TYPE
	#define IN_FLOW_3TUPLE_TBL_ENTRY_TYPE_OFFSET  1
	#define IN_FLOW_3TUPLE_TBL_ENTRY_TYPE_LEN     1
	#define IN_FLOW_3TUPLE_TBL_ENTRY_TYPE_DEFAULT 0x0
	/*[field] HOST_ADDR_INDEX_TYPE*/
	#define IN_FLOW_3TUPLE_TBL_HOST_ADDR_INDEX_TYPE
	#define IN_FLOW_3TUPLE_TBL_HOST_ADDR_INDEX_TYPE_OFFSET  2
	#define IN_FLOW_3TUPLE_TBL_HOST_ADDR_INDEX_TYPE_LEN     1
	#define IN_FLOW_3TUPLE_TBL_HOST_ADDR_INDEX_TYPE_DEFAULT 0x0
	/*[field] HOST_ADDR_INDEX*/
	#define IN_FLOW_3TUPLE_TBL_HOST_ADDR_INDEX
	#define IN_FLOW_3TUPLE_TBL_HOST_ADDR_INDEX_OFFSET  3
	#define IN_FLOW_3TUPLE_TBL_HOST_ADDR_INDEX_LEN     13
	#define IN_FLOW_3TUPLE_TBL_HOST_ADDR_INDEX_DEFAULT 0x0
	/*[field] PROTOCOL_TYPE*/
	#define IN_FLOW_3TUPLE_TBL_PROTOCOL_TYPE
	#define IN_FLOW_3TUPLE_TBL_PROTOCOL_TYPE_OFFSET  16
	#define IN_FLOW_3TUPLE_TBL_PROTOCOL_TYPE_LEN     2
	#define IN_FLOW_3TUPLE_TBL_PROTOCOL_TYPE_DEFAULT 0x0
	/*[field] AGE*/
	#define IN_FLOW_3TUPLE_TBL_AGE
	#define IN_FLOW_3TUPLE_TBL_AGE_OFFSET  18
	#define IN_FLOW_3TUPLE_TBL_AGE_LEN     2
	#define IN_FLOW_3TUPLE_TBL_AGE_DEFAULT 0x0
	/*[field] SRC_L3_IF_VALID*/
	#define IN_FLOW_3TUPLE_TBL_SRC_L3_IF_VALID
	#define IN_FLOW_3TUPLE_TBL_SRC_L3_IF_VALID_OFFSET  20
	#define IN_FLOW_3TUPLE_TBL_SRC_L3_IF_VALID_LEN     1
	#define IN_FLOW_3TUPLE_TBL_SRC_L3_IF_VALID_DEFAULT 0x0
	/*[field] SRC_L3_IF*/
	#define IN_FLOW_3TUPLE_TBL_SRC_L3_IF
	#define IN_FLOW_3TUPLE_TBL_SRC_L3_IF_OFFSET  21
	#define IN_FLOW_3TUPLE_TBL_SRC_L3_IF_LEN     8
	#define IN_FLOW_3TUPLE_TBL_SRC_L3_IF_DEFAULT 0x0
	/*[field] FWD_TYPE*/
	#define IN_FLOW_3TUPLE_TBL_FWD_TYPE
	#define IN_FLOW_3TUPLE_TBL_FWD_TYPE_OFFSET  29
	#define IN_FLOW_3TUPLE_TBL_FWD_TYPE_LEN     3
	#define IN_FLOW_3TUPLE_TBL_FWD_TYPE_DEFAULT 0x0
	/*[field] PORT_VP2 reuse FWD_TYPE[1]*/
	#define IN_FLOW_3TUPLE_TBL_PORT_VP2
	#define IN_FLOW_3TUPLE_TBL_PORT_VP2_OFFSET  32
	#define IN_FLOW_3TUPLE_TBL_PORT_VP2_LEN     8
	#define IN_FLOW_3TUPLE_TBL_PORT_VP2_DEFAULT 0x0
	/*[field] NEXT_HOP3 reuse FWD_TYPE[2]*/
	#define IN_FLOW_3TUPLE_TBL_NEXT_HOP3
	#define IN_FLOW_3TUPLE_TBL_NEXT_HOP3_OFFSET  32
	#define IN_FLOW_3TUPLE_TBL_NEXT_HOP3_LEN     12
	#define IN_FLOW_3TUPLE_TBL_NEXT_HOP3_DEFAULT 0x0
	/*[field] NEXT_HOP1 reuse FWD_TYPE[0]*/
	#define IN_FLOW_3TUPLE_TBL_NEXT_HOP1
	#define IN_FLOW_3TUPLE_TBL_NEXT_HOP1_OFFSET  32
	#define IN_FLOW_3TUPLE_TBL_NEXT_HOP1_LEN     12
	#define IN_FLOW_3TUPLE_TBL_NEXT_HOP1_DEFAULT 0x0
	/*[field] NEXT_HOP2 reuse FWD_TYPE[3]*/
	#define IN_FLOW_3TUPLE_TBL_NEXT_HOP2
	#define IN_FLOW_3TUPLE_TBL_NEXT_HOP2_OFFSET  32
	#define IN_FLOW_3TUPLE_TBL_NEXT_HOP2_LEN     12
	#define IN_FLOW_3TUPLE_TBL_NEXT_HOP2_DEFAULT 0x0
#if defined(APPE)
	/*[field] VLAN_FMT_VALID reuse FWD_TYPE[1]*/
	#define IN_FLOW_3TUPLE_TBL_VLAN_FMT_VALID
	#define IN_FLOW_3TUPLE_TBL_VLAN_FMT_VALID_OFFSET  40
	#define IN_FLOW_3TUPLE_TBL_VLAN_FMT_VALID_LEN     1
	#define IN_FLOW_3TUPLE_TBL_VLAN_FMT_VALID_DEFAULT 0x0
	/*[field] SVLAN_FMT reuse FWD_TYPE[1]*/
	#define IN_FLOW_3TUPLE_TBL_SVLAN_FMT
	#define IN_FLOW_3TUPLE_TBL_SVLAN_FMT_OFFSET  41
	#define IN_FLOW_3TUPLE_TBL_SVLAN_FMT_LEN     1
	#define IN_FLOW_3TUPLE_TBL_SVLAN_FMT_DEFAULT 0x0
	/*[field] CVLAN_FMT reuse FWD_TYPE[1]*/
	#define IN_FLOW_3TUPLE_TBL_CVLAN_FMT
	#define IN_FLOW_3TUPLE_TBL_CVLAN_FMT_OFFSET  42
	#define IN_FLOW_3TUPLE_TBL_CVLAN_FMT_LEN     1
	#define IN_FLOW_3TUPLE_TBL_CVLAN_FMT_DEFAULT 0x0
#endif
	/*[field] PORT_VP_VALID1 reuse FWD_TYPE[2]*/
	#define IN_FLOW_3TUPLE_TBL_PORT_VP_VALID1
	#define IN_FLOW_3TUPLE_TBL_PORT_VP_VALID1_OFFSET  44
	#define IN_FLOW_3TUPLE_TBL_PORT_VP_VALID1_LEN     1
	#define IN_FLOW_3TUPLE_TBL_PORT_VP_VALID1_DEFAULT 0x0
	/*[field] PORT_VP1 reuse FWD_TYPE[2]*/
	#define IN_FLOW_3TUPLE_TBL_PORT_VP1
	#define IN_FLOW_3TUPLE_TBL_PORT_VP1_OFFSET  45
	#define IN_FLOW_3TUPLE_TBL_PORT_VP1_LEN     8
	#define IN_FLOW_3TUPLE_TBL_PORT_VP1_DEFAULT 0x0
	/*[field] DE_ACCE*/
	#define IN_FLOW_3TUPLE_TBL_DE_ACCE
	#define IN_FLOW_3TUPLE_TBL_DE_ACCE_OFFSET  60
	#define IN_FLOW_3TUPLE_TBL_DE_ACCE_LEN     1
	#define IN_FLOW_3TUPLE_TBL_DE_ACCE_DEFAULT 0x0
	/*[field] COPY_TO_CPU_EN*/
	#define IN_FLOW_3TUPLE_TBL_COPY_TO_CPU_EN
	#define IN_FLOW_3TUPLE_TBL_COPY_TO_CPU_EN_OFFSET  61
	#define IN_FLOW_3TUPLE_TBL_COPY_TO_CPU_EN_LEN     1
	#define IN_FLOW_3TUPLE_TBL_COPY_TO_CPU_EN_DEFAULT 0x0
	/*[field] SYN_TOGGLE*/
	#define IN_FLOW_3TUPLE_TBL_SYN_TOGGLE
	#define IN_FLOW_3TUPLE_TBL_SYN_TOGGLE_OFFSET  62
	#define IN_FLOW_3TUPLE_TBL_SYN_TOGGLE_LEN     1
	#define IN_FLOW_3TUPLE_TBL_SYN_TOGGLE_DEFAULT 0x0
	/*[field] PRI_PROFILE*/
	#define IN_FLOW_3TUPLE_TBL_PRI_PROFILE
	#define IN_FLOW_3TUPLE_TBL_PRI_PROFILE_OFFSET  63
	#define IN_FLOW_3TUPLE_TBL_PRI_PROFILE_LEN     5
	#define IN_FLOW_3TUPLE_TBL_PRI_PROFILE_DEFAULT 0x0
	/*[field] SERVICE_CODE*/
	#define IN_FLOW_3TUPLE_TBL_SERVICE_CODE
	#define IN_FLOW_3TUPLE_TBL_SERVICE_CODE_OFFSET  68
	#define IN_FLOW_3TUPLE_TBL_SERVICE_CODE_LEN     8
	#define IN_FLOW_3TUPLE_TBL_SERVICE_CODE_DEFAULT 0x0
	/*[field] IP_ADDR*/
	#define IN_FLOW_3TUPLE_TBL_IP_ADDR
	#define IN_FLOW_3TUPLE_TBL_IP_ADDR_OFFSET  76
	#define IN_FLOW_3TUPLE_TBL_IP_ADDR_LEN     32
	#define IN_FLOW_3TUPLE_TBL_IP_ADDR_DEFAULT 0x0
	/*[field] IP_PROTOCOL*/
	#define IN_FLOW_3TUPLE_TBL_IP_PROTOCOL
	#define IN_FLOW_3TUPLE_TBL_IP_PROTOCOL_OFFSET  108
	#define IN_FLOW_3TUPLE_TBL_IP_PROTOCOL_LEN     8
	#define IN_FLOW_3TUPLE_TBL_IP_PROTOCOL_DEFAULT 0x0
#if defined(APPE)
	/*[field] VPN_ID*/
	#define IN_FLOW_3TUPLE_TBL_VPN_ID
	#define IN_FLOW_3TUPLE_TBL_VPN_ID_OFFSET  116
	#define IN_FLOW_3TUPLE_TBL_VPN_ID_LEN     8
	#define IN_FLOW_3TUPLE_TBL_VPN_ID_DEFAULT 0x0
	/*[field] PMTU_CHECK_TYPE*/
	#define IN_FLOW_3TUPLE_TBL_PMTU_CHECK_TYPE
	#define IN_FLOW_3TUPLE_TBL_PMTU_CHECK_TYPE_OFFSET  124
	#define IN_FLOW_3TUPLE_TBL_PMTU_CHECK_TYPE_LEN     1
	#define IN_FLOW_3TUPLE_TBL_PMTU_CHECK_TYPE_DEFAULT 0x0
	/*[field] PMTU*/
	#define IN_FLOW_3TUPLE_TBL_PMTU
	#define IN_FLOW_3TUPLE_TBL_PMTU_OFFSET  125
	#define IN_FLOW_3TUPLE_TBL_PMTU_LEN     14
	#define IN_FLOW_3TUPLE_TBL_PMTU_DEFAULT 0x0
#endif

struct in_flow_tbl_1 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
        a_uint32_t  age:2;
        a_uint32_t  protocol_type:2;
        a_uint32_t  host_addr_index:13;
        a_uint32_t  host_addr_index_type:1;
        a_uint32_t  entry_type:1;
        a_uint32_t  valid:1;

	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
#if defined(APPE)
#if defined(MPPE)
	a_uint32_t  _reserved0:4;
	a_uint32_t  next_hop4:12;
	a_uint32_t  next_hop4_valid:1;
#else
	a_uint32_t  _reserved0:17;
#endif
	a_uint32_t  cvlan_fmt:1;
	a_uint32_t  svlan_fmt:1;
	a_uint32_t  vlan_fmt_valid:1;
#else
	a_uint32_t  _reserved0:20;
#endif
        a_uint32_t  port_vp2:8;

	a_uint32_t  ip_addr_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  l4_dport_0:4;
        a_uint32_t  l4_sport:16;
        a_uint32_t  ip_addr_1:12;

#if defined(APPE)
	a_uint32_t  _reserved1:29;
	a_uint32_t  pmtu_1:3;
	a_uint32_t  pmtu_0:11;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
#else
	a_uint32_t  _reserved1:20;
#endif
        a_uint32_t  l4_dport_1:12;
};

struct in_flow_tbl_3 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
        a_uint32_t  age:2;
        a_uint32_t  protocol_type:2;
        a_uint32_t  host_addr_index:13;
        a_uint32_t  host_addr_index_type:1;
        a_uint32_t  entry_type:1;
        a_uint32_t  valid:1;

	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
        a_uint32_t  l4_port2:16;
        a_uint32_t  next_hop2:12;

	a_uint32_t  ip_addr_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  l4_dport_0:4;
        a_uint32_t  l4_sport:16;
        a_uint32_t  ip_addr_1:12;

#if defined(APPE)
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  pmtu_0:11;
	a_uint32_t  vpn_id:8;
        a_uint32_t  l4_dport_1:12;

	a_uint32_t  _reserved0:29;
	a_uint32_t  pmtu_1:3;
#else
	a_uint32_t  _reserved0:20;
        a_uint32_t  l4_dport_1:12;
#endif
};

struct in_flow_tbl_0 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
        a_uint32_t  age:2;
        a_uint32_t  protocol_type:2;
        a_uint32_t  host_addr_index:13;
	a_uint32_t  host_addr_index_type:1;
	a_uint32_t  entry_type:1;
	a_uint32_t  valid:1;
	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
        a_uint32_t  l4_port1:16;
        a_uint32_t  next_hop1:12;
	a_uint32_t  ip_addr_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  l4_dport_0:4;
        a_uint32_t  l4_sport:16;
        a_uint32_t  ip_addr_1:12;

#if defined(APPE)
	a_uint32_t  pmtu_0:11;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
        a_uint32_t  l4_dport_1:12;

	a_uint32_t  _reserved0:29;
	a_uint32_t  pmtu_1:3;
#else
	a_uint32_t  _reserved0:20;
        a_uint32_t  l4_dport_1:12;
#endif
};

struct in_flow_tbl_2 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
        a_uint32_t  age:2;
        a_uint32_t  protocol_type:2;
        a_uint32_t  host_addr_index:13;
        a_uint32_t  host_addr_index_type:1;
        a_uint32_t  entry_type:1;
        a_uint32_t  valid:1;
	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
	a_uint32_t  _reserved0:7;
        a_uint32_t  port_vp1:8;
        a_uint32_t  port_vp_valid1:1;
        a_uint32_t  next_hop3:12;
	a_uint32_t  ip_addr_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  l4_dport_0:4;
        a_uint32_t  l4_sport:16;
        a_uint32_t  ip_addr_1:12;

#if defined(APPE)
	a_uint32_t  pmtu_0:11;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
        a_uint32_t  l4_dport_1:12;

	a_uint32_t  _reserved1:29;
	a_uint32_t  pmtu_1:3;
#else
	a_uint32_t  _reserved1:20;
        a_uint32_t  l4_dport_1:12;
#endif
};

union in_flow_tbl_u {
#if defined(APPE)
	a_uint32_t val[6];
#else
	a_uint32_t val[5];
#endif
	struct in_flow_tbl_0 bf0;
	struct in_flow_tbl_1 bf1;
	struct in_flow_tbl_2 bf2;
	struct in_flow_tbl_3 bf3;
};

/*[table] IN_FLOW_IPV6_3TUPLE_TBL*/
#define IN_FLOW_IPV6_3TUPLE_TBL
#define IN_FLOW_IPV6_3TUPLE_TBL_ADDRESS 0x40000
#define IN_FLOW_IPV6_3TUPLE_TBL_NUM     2048
#define IN_FLOW_IPV6_3TUPLE_TBL_INC     0x40
#define IN_FLOW_IPV6_3TUPLE_TBL_TYPE    REG_TYPE_RW
#define IN_FLOW_IPV6_3TUPLE_TBL_DEFAULT 0x0
	/*[field] VALID*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_VALID
	#define IN_FLOW_IPV6_3TUPLE_TBL_VALID_OFFSET  0
	#define IN_FLOW_IPV6_3TUPLE_TBL_VALID_LEN     1
	#define IN_FLOW_IPV6_3TUPLE_TBL_VALID_DEFAULT 0x0
	/*[field] ENTRY_TYPE*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_ENTRY_TYPE
	#define IN_FLOW_IPV6_3TUPLE_TBL_ENTRY_TYPE_OFFSET  1
	#define IN_FLOW_IPV6_3TUPLE_TBL_ENTRY_TYPE_LEN     1
	#define IN_FLOW_IPV6_3TUPLE_TBL_ENTRY_TYPE_DEFAULT 0x0
	/*[field] HOST_ADDR_INDEX_TYPE*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_HOST_ADDR_INDEX_TYPE
	#define IN_FLOW_IPV6_3TUPLE_TBL_HOST_ADDR_INDEX_TYPE_OFFSET  2
	#define IN_FLOW_IPV6_3TUPLE_TBL_HOST_ADDR_INDEX_TYPE_LEN     1
	#define IN_FLOW_IPV6_3TUPLE_TBL_HOST_ADDR_INDEX_TYPE_DEFAULT 0x0
	/*[field] HOST_ADDR_INDEX*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_HOST_ADDR_INDEX
	#define IN_FLOW_IPV6_3TUPLE_TBL_HOST_ADDR_INDEX_OFFSET  3
	#define IN_FLOW_IPV6_3TUPLE_TBL_HOST_ADDR_INDEX_LEN     13
	#define IN_FLOW_IPV6_3TUPLE_TBL_HOST_ADDR_INDEX_DEFAULT 0x0
	/*[field] PROTOCOL_TYPE*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_PROTOCOL_TYPE
	#define IN_FLOW_IPV6_3TUPLE_TBL_PROTOCOL_TYPE_OFFSET  16
	#define IN_FLOW_IPV6_3TUPLE_TBL_PROTOCOL_TYPE_LEN     2
	#define IN_FLOW_IPV6_3TUPLE_TBL_PROTOCOL_TYPE_DEFAULT 0x0
	/*[field] AGE*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_AGE
	#define IN_FLOW_IPV6_3TUPLE_TBL_AGE_OFFSET  18
	#define IN_FLOW_IPV6_3TUPLE_TBL_AGE_LEN     2
	#define IN_FLOW_IPV6_3TUPLE_TBL_AGE_DEFAULT 0x0
	/*[field] SRC_L3_IF_VALID*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_SRC_L3_IF_VALID
	#define IN_FLOW_IPV6_3TUPLE_TBL_SRC_L3_IF_VALID_OFFSET  20
	#define IN_FLOW_IPV6_3TUPLE_TBL_SRC_L3_IF_VALID_LEN     1
	#define IN_FLOW_IPV6_3TUPLE_TBL_SRC_L3_IF_VALID_DEFAULT 0x0
	/*[field] SRC_L3_IF*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_SRC_L3_IF
	#define IN_FLOW_IPV6_3TUPLE_TBL_SRC_L3_IF_OFFSET  21
	#define IN_FLOW_IPV6_3TUPLE_TBL_SRC_L3_IF_LEN     8
	#define IN_FLOW_IPV6_3TUPLE_TBL_SRC_L3_IF_DEFAULT 0x0
	/*[field] FWD_TYPE*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_FWD_TYPE
	#define IN_FLOW_IPV6_3TUPLE_TBL_FWD_TYPE_OFFSET  29
	#define IN_FLOW_IPV6_3TUPLE_TBL_FWD_TYPE_LEN     3
	#define IN_FLOW_IPV6_3TUPLE_TBL_FWD_TYPE_DEFAULT 0x0
	/*[field] NEXT_HOP3 reuse FWD_TYPE[2]*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_NEXT_HOP3
	#define IN_FLOW_IPV6_3TUPLE_TBL_NEXT_HOP3_OFFSET  32
	#define IN_FLOW_IPV6_3TUPLE_TBL_NEXT_HOP3_LEN     12
	#define IN_FLOW_IPV6_3TUPLE_TBL_NEXT_HOP3_DEFAULT 0x0
	/*[field] NEXT_HOP1 reuse FWD_TYPE[1]*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_NEXT_HOP1
	#define IN_FLOW_IPV6_3TUPLE_TBL_NEXT_HOP1_OFFSET  32
	#define IN_FLOW_IPV6_3TUPLE_TBL_NEXT_HOP1_LEN     12
	#define IN_FLOW_IPV6_3TUPLE_TBL_NEXT_HOP1_DEFAULT 0x0
	/*[field] NEXT_HOP2 reuse FWD_TYPE[3]*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_NEXT_HOP2
	#define IN_FLOW_IPV6_3TUPLE_TBL_NEXT_HOP2_OFFSET  32
	#define IN_FLOW_IPV6_3TUPLE_TBL_NEXT_HOP2_LEN     12
	#define IN_FLOW_IPV6_3TUPLE_TBL_NEXT_HOP2_DEFAULT 0x0
	/*[field] PORT_VP2 reuse FWD_TYPE[0]*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_PORT_VP2
	#define IN_FLOW_IPV6_3TUPLE_TBL_PORT_VP2_OFFSET  32
	#define IN_FLOW_IPV6_3TUPLE_TBL_PORT_VP2_LEN     8
	#define IN_FLOW_IPV6_3TUPLE_TBL_PORT_VP2_DEFAULT 0x0
#if defined(APPE)
	/*[field] VLAN_FMT_VALID reuse FWD_TYPE[0]*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_VLAN_FMT_VALID
	#define IN_FLOW_IPV6_3TUPLE_TBL_VLAN_FMT_VALID_OFFSET  40
	#define IN_FLOW_IPV6_3TUPLE_TBL_VLAN_FMT_VALID_LEN     1
	#define IN_FLOW_IPV6_3TUPLE_TBL_VLAN_FMT_VALID_DEFAULT 0x0
	/*[field] SVLAN_FMT reuse FWD_TYPE[0]*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_SVLAN_FMT
	#define IN_FLOW_IPV6_3TUPLE_TBL_SVLAN_FMT_OFFSET  41
	#define IN_FLOW_IPV6_3TUPLE_TBL_SVLAN_FMT_LEN     1
	#define IN_FLOW_IPV6_3TUPLE_TBL_SVLAN_FMT_DEFAULT 0x0
	/*[field] CVLAN_FMT reuse FWD_TYPE[0]*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_CVLAN_FMT
	#define IN_FLOW_IPV6_3TUPLE_TBL_CVLAN_FMT_OFFSET  42
	#define IN_FLOW_IPV6_3TUPLE_TBL_CVLAN_FMT_LEN     1
	#define IN_FLOW_IPV6_3TUPLE_TBL_CVLAN_FMT_DEFAULT 0x0
#endif
	/*[field] PORT_VP_VALID1 reuse FWD_TYPE[2]*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_PORT_VP_VALID1
	#define IN_FLOW_IPV6_3TUPLE_TBL_PORT_VP_VALID1_OFFSET  44
	#define IN_FLOW_IPV6_3TUPLE_TBL_PORT_VP_VALID1_LEN     1
	#define IN_FLOW_IPV6_3TUPLE_TBL_PORT_VP_VALID1_DEFAULT 0x0
	/*[field] PORT_VP1 reuse FWD_TYPE[2]*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_PORT_VP1
	#define IN_FLOW_IPV6_3TUPLE_TBL_PORT_VP1_OFFSET  45
	#define IN_FLOW_IPV6_3TUPLE_TBL_PORT_VP1_LEN     8
	#define IN_FLOW_IPV6_3TUPLE_TBL_PORT_VP1_DEFAULT 0x0
	/*[field] DE_ACCE*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_DE_ACCE
	#define IN_FLOW_IPV6_3TUPLE_TBL_DE_ACCE_OFFSET  60
	#define IN_FLOW_IPV6_3TUPLE_TBL_DE_ACCE_LEN     1
	#define IN_FLOW_IPV6_3TUPLE_TBL_DE_ACCE_DEFAULT 0x0
	/*[field] COPY_TO_CPU_EN*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_COPY_TO_CPU_EN
	#define IN_FLOW_IPV6_3TUPLE_TBL_COPY_TO_CPU_EN_OFFSET  61
	#define IN_FLOW_IPV6_3TUPLE_TBL_COPY_TO_CPU_EN_LEN     1
	#define IN_FLOW_IPV6_3TUPLE_TBL_COPY_TO_CPU_EN_DEFAULT 0x0
	/*[field] SYN_TOGGLE*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_SYN_TOGGLE
	#define IN_FLOW_IPV6_3TUPLE_TBL_SYN_TOGGLE_OFFSET  62
	#define IN_FLOW_IPV6_3TUPLE_TBL_SYN_TOGGLE_LEN     1
	#define IN_FLOW_IPV6_3TUPLE_TBL_SYN_TOGGLE_DEFAULT 0x0
	/*[field] PRI_PROFILE*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_PRI_PROFILE
	#define IN_FLOW_IPV6_3TUPLE_TBL_PRI_PROFILE_OFFSET  63
	#define IN_FLOW_IPV6_3TUPLE_TBL_PRI_PROFILE_LEN     5
	#define IN_FLOW_IPV6_3TUPLE_TBL_PRI_PROFILE_DEFAULT 0x0
	/*[field] SERVICE_CODE*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_SERVICE_CODE
	#define IN_FLOW_IPV6_3TUPLE_TBL_SERVICE_CODE_OFFSET  68
	#define IN_FLOW_IPV6_3TUPLE_TBL_SERVICE_CODE_LEN     8
	#define IN_FLOW_IPV6_3TUPLE_TBL_SERVICE_CODE_DEFAULT 0x0
#if defined(APPE)
	/*[field] VPN_ID*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_VPN_ID
	#define IN_FLOW_IPV6_3TUPLE_TBL_VPN_ID_OFFSET  76
	#define IN_FLOW_IPV6_3TUPLE_TBL_VPN_ID_LEN     8
	#define IN_FLOW_IPV6_3TUPLE_TBL_VPN_ID_DEFAULT 0x0
	/*[field] PMTU_CHECK_TYPE*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_PMTU_CHECK_TYPE
	#define IN_FLOW_IPV6_3TUPLE_TBL_PMTU_CHECK_TYPE_OFFSET  84
	#define IN_FLOW_IPV6_3TUPLE_TBL_PMTU_CHECK_TYPE_LEN     1
	#define IN_FLOW_IPV6_3TUPLE_TBL_PMTU_CHECK_TYPE_DEFAULT 0x0
	/*[field] PMTU*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_PMTU
	#define IN_FLOW_IPV6_3TUPLE_TBL_PMTU_OFFSET  85
	#define IN_FLOW_IPV6_3TUPLE_TBL_PMTU_LEN     14
	#define IN_FLOW_IPV6_3TUPLE_TBL_PMTU_DEFAULT 0x0
#endif
	/*[field] IP_PROTOCOL*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_IP_PROTOCOL
	#define IN_FLOW_IPV6_3TUPLE_TBL_IP_PROTOCOL_OFFSET  108
	#define IN_FLOW_IPV6_3TUPLE_TBL_IP_PROTOCOL_LEN     8
	#define IN_FLOW_IPV6_3TUPLE_TBL_IP_PROTOCOL_DEFAULT 0x0
	/*[field] IP_ADDR*/
	#define IN_FLOW_IPV6_3TUPLE_TBL_IP_ADDR
	#define IN_FLOW_IPV6_3TUPLE_TBL_IP_ADDR_OFFSET  140
	#define IN_FLOW_IPV6_3TUPLE_TBL_IP_ADDR_LEN     128
	#define IN_FLOW_IPV6_3TUPLE_TBL_IP_ADDR_DEFAULT 0x0

struct in_flow_3tuple_tbl_3 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
        a_uint32_t  age:2;
        a_uint32_t  protocol_type:2;
        a_uint32_t  host_addr_index:13;
        a_uint32_t  host_addr_index_type:1;
        a_uint32_t  entry_type:1;
        a_uint32_t  valid:1;

	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
        a_uint32_t  _reserved0:16;
        a_uint32_t  next_hop2:12;

	a_uint32_t  ip_addr_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

#if defined(APPE)
	a_uint32_t  pmtu_0:3;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;

	a_uint32_t  _reserved1:21;
	a_uint32_t  pmtu_1:11;
#else
	a_uint32_t  _reserved1_0:12;
        a_uint32_t  ip_protocol:8;
        a_uint32_t  ip_addr_1:12;

	a_uint32_t  _reserved1_1:32;
#endif
};

struct in_flow_3tuple_tbl_1 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
        a_uint32_t  age:2;
        a_uint32_t  protocol_type:2;
        a_uint32_t  host_addr_index:13;
        a_uint32_t  host_addr_index_type:1;
        a_uint32_t  entry_type:1;
        a_uint32_t  valid:1;

	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
#if defined(APPE)
#if defined(MPPE)
	a_uint32_t  _reserved0:4;
	a_uint32_t  next_hop4:12;
	a_uint32_t  next_hop4_valid:1;
#else
	a_uint32_t  _reserved0:17;
#endif
	a_uint32_t  cvlan_fmt:1;
	a_uint32_t  svlan_fmt:1;
	a_uint32_t  vlan_fmt_valid:1;
#else
	a_uint32_t  _reserved0:20;
#endif
        a_uint32_t  port_vp2:8;

	a_uint32_t  ip_addr_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

#if defined(APPE)
	a_uint32_t  pmtu_0:3;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
        a_uint32_t  ip_protocol:8;
        a_uint32_t  ip_addr_1:12;

	a_uint32_t  _reserved1:21;
	a_uint32_t  pmtu_1:11;
#else
	a_uint32_t  _reserved1_0:12;
        a_uint32_t  ip_protocol:8;
        a_uint32_t  ip_addr_1:12;

	a_uint32_t  _reserved1_1:32;
#endif
};

struct in_flow_3tuple_tbl_2 {	
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
        a_uint32_t  age:2;
	a_uint32_t  protocol_type:2;
	a_uint32_t  host_addr_index:13;
	a_uint32_t  host_addr_index_type:1;
	a_uint32_t  entry_type:1;
	a_uint32_t  valid:1;
	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
        a_uint32_t  _reserved0:7;
        a_uint32_t  port_vp1:8;
        a_uint32_t  port_vp_valid1:1;
        a_uint32_t  next_hop3:12;
	a_uint32_t  ip_addr_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;
#if defined(APPE)
	a_uint32_t  pmtu_0:3;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
        a_uint32_t  ip_protocol:8;
        a_uint32_t  ip_addr_1:12;

	a_uint32_t  _reserved1:21;
	a_uint32_t  pmtu_1:11;
#else
	a_uint32_t  _reserved1_0:12;
        a_uint32_t  ip_protocol:8;
        a_uint32_t  ip_addr_1:12;
	a_uint32_t  _reserved1_1:32;
#endif
};

struct in_flow_3tuple_tbl_0 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
        a_uint32_t  age:2;
	a_uint32_t  protocol_type:2;
	a_uint32_t  host_addr_index:13;
	a_uint32_t  host_addr_index_type:1;
        a_uint32_t  entry_type:1;
	a_uint32_t  valid:1;
	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
        a_uint32_t  _reserved0:16;
        a_uint32_t  next_hop1:12;
	a_uint32_t  ip_addr_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;
#if defined(APPE)
	a_uint32_t  pmtu_0:3;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
        a_uint32_t  ip_protocol:8;
        a_uint32_t  ip_addr_1:12;
	a_uint32_t  _reserved1:21;
	a_uint32_t  pmtu_1:11;
#else
	a_uint32_t  _reserved1_0:12;
        a_uint32_t  ip_protocol:8;
        a_uint32_t  ip_addr_1:12;
	a_uint32_t  _reserved1_1:32;
#endif
};

union in_flow_3tuple_tbl_u {
	a_uint32_t val[5];
	struct in_flow_3tuple_tbl_0 bf0;
	struct in_flow_3tuple_tbl_1 bf1;
	struct in_flow_3tuple_tbl_2 bf2;
	struct in_flow_3tuple_tbl_3 bf3;
};

/*[table] IN_FLOW_IPV6_5TUPLE_TBL*/
#define IN_FLOW_IPV6_5TUPLE_TBL
#define IN_FLOW_IPV6_5TUPLE_TBL_ADDRESS 0x40000
#define IN_FLOW_IPV6_5TUPLE_TBL_NUM     2048
#define IN_FLOW_IPV6_5TUPLE_TBL_INC     0x40
#define IN_FLOW_IPV6_5TUPLE_TBL_TYPE    REG_TYPE_RW
#define IN_FLOW_IPV6_5TUPLE_TBL_DEFAULT 0x0
	/*[field] VALID*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_VALID
	#define IN_FLOW_IPV6_5TUPLE_TBL_VALID_OFFSET  0
	#define IN_FLOW_IPV6_5TUPLE_TBL_VALID_LEN     1
	#define IN_FLOW_IPV6_5TUPLE_TBL_VALID_DEFAULT 0x0
	/*[field] ENTRY_TYPE*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_ENTRY_TYPE
	#define IN_FLOW_IPV6_5TUPLE_TBL_ENTRY_TYPE_OFFSET  1
	#define IN_FLOW_IPV6_5TUPLE_TBL_ENTRY_TYPE_LEN     1
	#define IN_FLOW_IPV6_5TUPLE_TBL_ENTRY_TYPE_DEFAULT 0x0
	/*[field] HOST_ADDR_INDEX_TYPE*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_HOST_ADDR_INDEX_TYPE
	#define IN_FLOW_IPV6_5TUPLE_TBL_HOST_ADDR_INDEX_TYPE_OFFSET  2
	#define IN_FLOW_IPV6_5TUPLE_TBL_HOST_ADDR_INDEX_TYPE_LEN     1
	#define IN_FLOW_IPV6_5TUPLE_TBL_HOST_ADDR_INDEX_TYPE_DEFAULT 0x0
	/*[field] HOST_ADDR_INDEX*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_HOST_ADDR_INDEX
	#define IN_FLOW_IPV6_5TUPLE_TBL_HOST_ADDR_INDEX_OFFSET  3
	#define IN_FLOW_IPV6_5TUPLE_TBL_HOST_ADDR_INDEX_LEN     13
	#define IN_FLOW_IPV6_5TUPLE_TBL_HOST_ADDR_INDEX_DEFAULT 0x0
	/*[field] PROTOCOL_TYPE*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_PROTOCOL_TYPE
	#define IN_FLOW_IPV6_5TUPLE_TBL_PROTOCOL_TYPE_OFFSET  16
	#define IN_FLOW_IPV6_5TUPLE_TBL_PROTOCOL_TYPE_LEN     2
	#define IN_FLOW_IPV6_5TUPLE_TBL_PROTOCOL_TYPE_DEFAULT 0x0
	/*[field] AGE*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_AGE
	#define IN_FLOW_IPV6_5TUPLE_TBL_AGE_OFFSET  18
	#define IN_FLOW_IPV6_5TUPLE_TBL_AGE_LEN     2
	#define IN_FLOW_IPV6_5TUPLE_TBL_AGE_DEFAULT 0x0
	/*[field] SRC_L3_IF_VALID*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_SRC_L3_IF_VALID
	#define IN_FLOW_IPV6_5TUPLE_TBL_SRC_L3_IF_VALID_OFFSET  20
	#define IN_FLOW_IPV6_5TUPLE_TBL_SRC_L3_IF_VALID_LEN     1
	#define IN_FLOW_IPV6_5TUPLE_TBL_SRC_L3_IF_VALID_DEFAULT 0x0
	/*[field] SRC_L3_IF*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_SRC_L3_IF
	#define IN_FLOW_IPV6_5TUPLE_TBL_SRC_L3_IF_OFFSET  21
	#define IN_FLOW_IPV6_5TUPLE_TBL_SRC_L3_IF_LEN     8
	#define IN_FLOW_IPV6_5TUPLE_TBL_SRC_L3_IF_DEFAULT 0x0
	/*[field] FWD_TYPE*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_FWD_TYPE
	#define IN_FLOW_IPV6_5TUPLE_TBL_FWD_TYPE_OFFSET  29
	#define IN_FLOW_IPV6_5TUPLE_TBL_FWD_TYPE_LEN     3
	#define IN_FLOW_IPV6_5TUPLE_TBL_FWD_TYPE_DEFAULT 0x0
	/*[field] NEXT_HOP2 reuse FWD_TYPE[3]*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_NEXT_HOP2
	#define IN_FLOW_IPV6_5TUPLE_TBL_NEXT_HOP2_OFFSET  32
	#define IN_FLOW_IPV6_5TUPLE_TBL_NEXT_HOP2_LEN     12
	#define IN_FLOW_IPV6_5TUPLE_TBL_NEXT_HOP2_DEFAULT 0x0
	/*[field] PORT_VP2 reuse FWD_TYPE[1]*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_PORT_VP2
	#define IN_FLOW_IPV6_5TUPLE_TBL_PORT_VP2_OFFSET  32
	#define IN_FLOW_IPV6_5TUPLE_TBL_PORT_VP2_LEN     8
	#define IN_FLOW_IPV6_5TUPLE_TBL_PORT_VP2_DEFAULT 0x0
	/*[field] NEXT_HOP3 reuse FWD_TYPE[2]*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_NEXT_HOP3
	#define IN_FLOW_IPV6_5TUPLE_TBL_NEXT_HOP3_OFFSET  32
	#define IN_FLOW_IPV6_5TUPLE_TBL_NEXT_HOP3_LEN     12
	#define IN_FLOW_IPV6_5TUPLE_TBL_NEXT_HOP3_DEFAULT 0x0
	/*[field] NEXT_HOP1 reuse FWD_TYPE[0]*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_NEXT_HOP1
	#define IN_FLOW_IPV6_5TUPLE_TBL_NEXT_HOP1_OFFSET  32
	#define IN_FLOW_IPV6_5TUPLE_TBL_NEXT_HOP1_LEN     12
	#define IN_FLOW_IPV6_5TUPLE_TBL_NEXT_HOP1_DEFAULT 0x0
#if defined(APPE)
	/*[field] VLAN_FMT_VALID reuse FWD_TYPE[1]*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_VLAN_FMT_VALID
	#define IN_FLOW_IPV6_5TUPLE_TBL_VLAN_FMT_VALID_OFFSET  40
	#define IN_FLOW_IPV6_5TUPLE_TBL_VLAN_FMT_VALID_LEN     1
	#define IN_FLOW_IPV6_5TUPLE_TBL_VLAN_FMT_VALID_DEFAULT 0x0
	/*[field] SVLAN_FMT reuse FWD_TYPE[1]*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_SVLAN_FMT
	#define IN_FLOW_IPV6_5TUPLE_TBL_SVLAN_FMT_OFFSET  41
	#define IN_FLOW_IPV6_5TUPLE_TBL_SVLAN_FMT_LEN     1
	#define IN_FLOW_IPV6_5TUPLE_TBL_SVLAN_FMT_DEFAULT 0x0
	/*[field] CVLAN_FMT reuse FWD_TYPE[1]*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_CVLAN_FMT
	#define IN_FLOW_IPV6_5TUPLE_TBL_CVLAN_FMT_OFFSET  42
	#define IN_FLOW_IPV6_5TUPLE_TBL_CVLAN_FMT_LEN     1
	#define IN_FLOW_IPV6_5TUPLE_TBL_CVLAN_FMT_DEFAULT 0x0
#endif
	/*[field] PORT_VP_VALID1 reuse FWD_TYPE[2]*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_PORT_VP_VALID1
	#define IN_FLOW_IPV6_5TUPLE_TBL_PORT_VP_VALID1_OFFSET  44
	#define IN_FLOW_IPV6_5TUPLE_TBL_PORT_VP_VALID1_LEN     1
	#define IN_FLOW_IPV6_5TUPLE_TBL_PORT_VP_VALID1_DEFAULT 0x0
#if defined(APPE)
	/*[field] L4_PORT1 reuse FWD_TYPE[0]*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_PORT1
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_PORT1_OFFSET  44
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_PORT1_LEN     16
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_PORT1_DEFAULT 0x0
	/*[field] L4_PORT2 reuse FWD_TYPE[3]*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_PORT2
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_PORT2_OFFSET  44
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_PORT2_LEN     16
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_PORT2_DEFAULT 0x0
#endif
	/*[field] PORT_VP1 reuse FWD_TYPE[2]*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_PORT_VP1
	#define IN_FLOW_IPV6_5TUPLE_TBL_PORT_VP1_OFFSET  45
	#define IN_FLOW_IPV6_5TUPLE_TBL_PORT_VP1_LEN     8
	#define IN_FLOW_IPV6_5TUPLE_TBL_PORT_VP1_DEFAULT 0x0
	/*[field] DE_ACCE*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_DE_ACCE
	#define IN_FLOW_IPV6_5TUPLE_TBL_DE_ACCE_OFFSET  60
	#define IN_FLOW_IPV6_5TUPLE_TBL_DE_ACCE_LEN     1
	#define IN_FLOW_IPV6_5TUPLE_TBL_DE_ACCE_DEFAULT 0x0
	/*[field] COPY_TO_CPU_EN*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_COPY_TO_CPU_EN
	#define IN_FLOW_IPV6_5TUPLE_TBL_COPY_TO_CPU_EN_OFFSET  61
	#define IN_FLOW_IPV6_5TUPLE_TBL_COPY_TO_CPU_EN_LEN     1
	#define IN_FLOW_IPV6_5TUPLE_TBL_COPY_TO_CPU_EN_DEFAULT 0x0
	/*[field] SYN_TOGGLE*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_SYN_TOGGLE
	#define IN_FLOW_IPV6_5TUPLE_TBL_SYN_TOGGLE_OFFSET  62
	#define IN_FLOW_IPV6_5TUPLE_TBL_SYN_TOGGLE_LEN     1
	#define IN_FLOW_IPV6_5TUPLE_TBL_SYN_TOGGLE_DEFAULT 0x0
	/*[field] PRI_PROFILE*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_PRI_PROFILE
	#define IN_FLOW_IPV6_5TUPLE_TBL_PRI_PROFILE_OFFSET  63
	#define IN_FLOW_IPV6_5TUPLE_TBL_PRI_PROFILE_LEN     5
	#define IN_FLOW_IPV6_5TUPLE_TBL_PRI_PROFILE_DEFAULT 0x0
	/*[field] SERVICE_CODE*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_SERVICE_CODE
	#define IN_FLOW_IPV6_5TUPLE_TBL_SERVICE_CODE_OFFSET  68
	#define IN_FLOW_IPV6_5TUPLE_TBL_SERVICE_CODE_LEN     8
	#define IN_FLOW_IPV6_5TUPLE_TBL_SERVICE_CODE_DEFAULT 0x0
#if defined(APPE)
	/*[field] VPN_ID*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_VPN_ID
	#define IN_FLOW_IPV6_5TUPLE_TBL_VPN_ID_OFFSET  76
	#define IN_FLOW_IPV6_5TUPLE_TBL_VPN_ID_LEN     8
	#define IN_FLOW_IPV6_5TUPLE_TBL_VPN_ID_DEFAULT 0x0
	/*[field] PMTU_CHECK_TYPE*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_PMTU_CHECK_TYPE
	#define IN_FLOW_IPV6_5TUPLE_TBL_PMTU_CHECK_TYPE_OFFSET  84
	#define IN_FLOW_IPV6_5TUPLE_TBL_PMTU_CHECK_TYPE_LEN     1
	#define IN_FLOW_IPV6_5TUPLE_TBL_PMTU_CHECK_TYPE_DEFAULT 0x0
	/*[field] PMTU*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_PMTU
	#define IN_FLOW_IPV6_5TUPLE_TBL_PMTU_OFFSET  85
	#define IN_FLOW_IPV6_5TUPLE_TBL_PMTU_LEN     14
	#define IN_FLOW_IPV6_5TUPLE_TBL_PMTU_DEFAULT 0x0
#endif
	/*[field] L4_SPORT*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_SPORT
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_SPORT_OFFSET  108
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_SPORT_LEN     16
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_SPORT_DEFAULT 0x0
	/*[field] L4_DPORT*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_DPORT
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_DPORT_OFFSET  124
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_DPORT_LEN     16
	#define IN_FLOW_IPV6_5TUPLE_TBL_L4_DPORT_DEFAULT 0x0
	/*[field] IP_ADDR*/
	#define IN_FLOW_IPV6_5TUPLE_TBL_IP_ADDR
	#define IN_FLOW_IPV6_5TUPLE_TBL_IP_ADDR_OFFSET  140
	#define IN_FLOW_IPV6_5TUPLE_TBL_IP_ADDR_LEN     128
	#define IN_FLOW_IPV6_5TUPLE_TBL_IP_ADDR_DEFAULT 0x0

struct in_flow_ipv6_5tuple_tbl_1 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
	a_uint32_t  src_l3_if_valid:1;
	a_uint32_t  age:2;
	a_uint32_t  protocol_type:2;
        a_uint32_t  host_addr_index:13;
        a_uint32_t  host_addr_index_type:1;
        a_uint32_t  entry_type:1;
        a_uint32_t  valid:1;

	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
#if defined(APPE)
#if defined(MPPE)
	a_uint32_t  _reserved0:4;
	a_uint32_t  next_hop4:12;
	a_uint32_t  next_hop4_valid:1;
#else
	a_uint32_t  _reserved0:17;
#endif
	a_uint32_t  cvlan_fmt:1;
	a_uint32_t  svlan_fmt:1;
	a_uint32_t  vlan_fmt_valid:1;
#else
	a_uint32_t  _reserved0:20;
#endif
	a_uint32_t  port_vp2:8;

#if defined(APPE)
	a_uint32_t  pmtu_0:11;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  l4_dport_0:4;
        a_uint32_t  l4_sport:16;
	a_uint32_t  _reserved1:9;
	a_uint32_t  pmtu_1:3;
#else
	a_uint32_t  _reserved1_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  l4_dport_0:4;
        a_uint32_t  l4_sport:16;
        a_uint32_t  _reserved1_1:12;
#endif

	a_uint32_t  ip_addr_0:20;
        a_uint32_t  l4_dport_1:12; 

	a_uint32_t  ip_addr_1:32;
	a_uint32_t  ip_addr_2:32;
	a_uint32_t  ip_addr_3:32;

	a_uint32_t  _reserved2:20;
        a_uint32_t  ip_addr_4:12;
};

struct in_flow_ipv6_5tuple_tbl_0 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
        a_uint32_t  age:2;
        a_uint32_t  protocol_type:2;
        a_uint32_t  host_addr_index:13;
	a_uint32_t  host_addr_index_type:1;
	a_uint32_t  entry_type:1;
	a_uint32_t  valid:1;
	
	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
#if defined(APPE)
	a_uint32_t  l4_port1:16;
#else
	a_uint32_t  _reserved0:16;
#endif
        a_uint32_t  next_hop1:12;

#if defined(APPE)
	a_uint32_t  pmtu_0:11;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  l4_dport_0:4;
        a_uint32_t  l4_sport:16;
	a_uint32_t  _reserved1:9;
	a_uint32_t  pmtu_1:3;
#else
	a_uint32_t  _reserved1_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  l4_dport_0:4;
        a_uint32_t  l4_sport:16;
        a_uint32_t  _reserved1_1:12;
#endif

	a_uint32_t  ip_addr_0:20;
        a_uint32_t  l4_dport_1:12;

	a_uint32_t  ip_addr_1:32;
	a_uint32_t  ip_addr_2:32;
	a_uint32_t  ip_addr_3:32;

	a_uint32_t  _reserved2:20;
        a_uint32_t  ip_addr_4:12;
};

struct in_flow_ipv6_5tuple_tbl_2 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
        a_uint32_t  age:2;
        a_uint32_t  protocol_type:2;
        a_uint32_t  host_addr_index:13;
        a_uint32_t  host_addr_index_type:1;
	a_uint32_t  entry_type:1;
	a_uint32_t  valid:1;

	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
        a_uint32_t  _reserved0:7;
        a_uint32_t  port_vp1:8;
        a_uint32_t  port_vp_valid1:1;
        a_uint32_t  next_hop3:12;

#if defined(APPE)
	a_uint32_t  pmtu_0:11;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  l4_dport_0:4;
        a_uint32_t  l4_sport:16;
	a_uint32_t  _reserved1:9;
	a_uint32_t  pmtu_1:3;
#else
	a_uint32_t  _reserved1_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  l4_dport_0:4;
        a_uint32_t  l4_sport:16;
        a_uint32_t  _reserved1_1:12;
#endif

	a_uint32_t  ip_addr_0:20;
        a_uint32_t  l4_dport_1:12;
	a_uint32_t  ip_addr_1:32;
	a_uint32_t  ip_addr_2:32;
	a_uint32_t  ip_addr_3:32;
	a_uint32_t  _reserved2:20;
        a_uint32_t  ip_addr_4:12;
};

struct in_flow_ipv6_5tuple_tbl_3 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
        a_uint32_t  age:2;
        a_uint32_t  protocol_type:2;
        a_uint32_t  host_addr_index:13;
	a_uint32_t  host_addr_index_type:1;
	a_uint32_t  entry_type:1;
	a_uint32_t  valid:1;
	
	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
#if defined(APPE)
	a_uint32_t  l4_port2:16;
#else
	a_uint32_t  _reserved0:16;
#endif
        a_uint32_t  next_hop2:12;

#if defined(APPE)
	a_uint32_t  pmtu_0:11;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  l4_dport_0:4;
        a_uint32_t  l4_sport:16;
	a_uint32_t  _reserved1:9;
	a_uint32_t  pmtu_1:3;
#else
	a_uint32_t  _reserved1_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  l4_dport_0:4;
        a_uint32_t  l4_sport:16;
        a_uint32_t  _reserved1_1:12;
#endif
	a_uint32_t  ip_addr_0:20;
        a_uint32_t  l4_dport_1:12;
	a_uint32_t  ip_addr_1:32;
	a_uint32_t  ip_addr_2:32;
	a_uint32_t  ip_addr_3:32;
	a_uint32_t  _reserved2:20;
        a_uint32_t  ip_addr_4:12;
};

union in_flow_ipv6_5tuple_tbl_u {
	a_uint32_t val[9];
	struct in_flow_ipv6_5tuple_tbl_0 bf0;
	struct in_flow_ipv6_5tuple_tbl_1 bf1;
	struct in_flow_ipv6_5tuple_tbl_2 bf2;
	struct in_flow_ipv6_5tuple_tbl_3 bf3;
};

/*[table] IN_FLOW_TBL*/
#define IN_FLOW_TBL
#define IN_FLOW_TBL_ADDRESS 0x40000
#if defined(MPPE) || defined(CPPE)
#define IN_FLOW_TBL_NUM     2048
#else
#define IN_FLOW_TBL_NUM     4096
#endif
#define IN_FLOW_TBL_INC     0x20
#define IN_FLOW_TBL_TYPE    REG_TYPE_RW
#define IN_FLOW_TBL_DEFAULT 0x0
	/*[field] VALID*/
	#define IN_FLOW_TBL_VALID
	#define IN_FLOW_TBL_VALID_OFFSET  0
	#define IN_FLOW_TBL_VALID_LEN     1
	#define IN_FLOW_TBL_VALID_DEFAULT 0x0
	/*[field] ENTRY_TYPE*/
	#define IN_FLOW_TBL_ENTRY_TYPE
	#define IN_FLOW_TBL_ENTRY_TYPE_OFFSET  1
	#define IN_FLOW_TBL_ENTRY_TYPE_LEN     1
	#define IN_FLOW_TBL_ENTRY_TYPE_DEFAULT 0x0
	/*[field] HOST_ADDR_INDEX_TYPE*/
	#define IN_FLOW_TBL_HOST_ADDR_INDEX_TYPE
	#define IN_FLOW_TBL_HOST_ADDR_INDEX_TYPE_OFFSET  2
	#define IN_FLOW_TBL_HOST_ADDR_INDEX_TYPE_LEN     1
	#define IN_FLOW_TBL_HOST_ADDR_INDEX_TYPE_DEFAULT 0x0
	/*[field] HOST_ADDR_INDEX*/
	#define IN_FLOW_TBL_HOST_ADDR_INDEX
	#define IN_FLOW_TBL_HOST_ADDR_INDEX_OFFSET  3
	#define IN_FLOW_TBL_HOST_ADDR_INDEX_LEN     13
	#define IN_FLOW_TBL_HOST_ADDR_INDEX_DEFAULT 0x0
	/*[field] PROTOCOL_TYPE*/
	#define IN_FLOW_TBL_PROTOCOL_TYPE
	#define IN_FLOW_TBL_PROTOCOL_TYPE_OFFSET  16
	#define IN_FLOW_TBL_PROTOCOL_TYPE_LEN     2
	#define IN_FLOW_TBL_PROTOCOL_TYPE_DEFAULT 0x0
	/*[field] AGE*/
	#define IN_FLOW_TBL_AGE
	#define IN_FLOW_TBL_AGE_OFFSET  18
	#define IN_FLOW_TBL_AGE_LEN     2
	#define IN_FLOW_TBL_AGE_DEFAULT 0x0
	/*[field] SRC_L3_IF_VALID*/
	#define IN_FLOW_TBL_SRC_L3_IF_VALID
	#define IN_FLOW_TBL_SRC_L3_IF_VALID_OFFSET  20
	#define IN_FLOW_TBL_SRC_L3_IF_VALID_LEN     1
	#define IN_FLOW_TBL_SRC_L3_IF_VALID_DEFAULT 0x0
	/*[field] SRC_L3_IF*/
	#define IN_FLOW_TBL_SRC_L3_IF
	#define IN_FLOW_TBL_SRC_L3_IF_OFFSET  21
	#define IN_FLOW_TBL_SRC_L3_IF_LEN     8
	#define IN_FLOW_TBL_SRC_L3_IF_DEFAULT 0x0
	/*[field] FWD_TYPE*/
	#define IN_FLOW_TBL_FWD_TYPE
	#define IN_FLOW_TBL_FWD_TYPE_OFFSET  29
	#define IN_FLOW_TBL_FWD_TYPE_LEN     3
	#define IN_FLOW_TBL_FWD_TYPE_DEFAULT 0x0
	/*[field] PORT_VP2 reuse FWD_TYPE[1]*/
	#define IN_FLOW_TBL_PORT_VP2
	#define IN_FLOW_TBL_PORT_VP2_OFFSET  32
	#define IN_FLOW_TBL_PORT_VP2_LEN     8
	#define IN_FLOW_TBL_PORT_VP2_DEFAULT 0x0
	/*[field] NEXT_HOP2 reuse FWD_TYPE[3]*/
	#define IN_FLOW_TBL_NEXT_HOP2
	#define IN_FLOW_TBL_NEXT_HOP2_OFFSET  32
	#define IN_FLOW_TBL_NEXT_HOP2_LEN     12
	#define IN_FLOW_TBL_NEXT_HOP2_DEFAULT 0x0
	/*[field] NEXT_HOP3 reuse FWD_TYPE[2]*/
	#define IN_FLOW_TBL_NEXT_HOP3
	#define IN_FLOW_TBL_NEXT_HOP3_OFFSET  32
	#define IN_FLOW_TBL_NEXT_HOP3_LEN     12
	#define IN_FLOW_TBL_NEXT_HOP3_DEFAULT 0x0
	/*[field] NEXT_HOP1 reuse FWD_TYPE[0]*/
	#define IN_FLOW_TBL_NEXT_HOP1
	#define IN_FLOW_TBL_NEXT_HOP1_OFFSET  32
	#define IN_FLOW_TBL_NEXT_HOP1_LEN     12
	#define IN_FLOW_TBL_NEXT_HOP1_DEFAULT 0x0
#if defined(APPE)
	/*[field] VLAN_FMT_VALID reuse FWD_TYPE[1]*/
	#define IN_FLOW_TBL_VLAN_FMT_VALID
	#define IN_FLOW_TBL_VLAN_FMT_VALID_OFFSET  40
	#define IN_FLOW_TBL_VLAN_FMT_VALID_LEN     1
	#define IN_FLOW_TBL_VLAN_FMT_VALID_DEFAULT 0x0
	/*[field] SVLAN_FMT reuse FWD_TYPE[1]*/
	#define IN_FLOW_TBL_SVLAN_FMT
	#define IN_FLOW_TBL_SVLAN_FMT_OFFSET  41
	#define IN_FLOW_TBL_SVLAN_FMT_LEN     1
	#define IN_FLOW_TBL_SVLAN_FMT_DEFAULT 0x0
	/*[field] CVLAN_FMT reuse FWD_TYPE[1]*/
	#define IN_FLOW_TBL_CVLAN_FMT
	#define IN_FLOW_TBL_CVLAN_FMT_OFFSET  42
	#define IN_FLOW_TBL_CVLAN_FMT_LEN     1
	#define IN_FLOW_TBL_CVLAN_FMT_DEFAULT 0x0
#endif
	/*[field] L4_PORT2 reuse FWD_TYPE[3]*/
	#define IN_FLOW_TBL_L4_PORT2
	#define IN_FLOW_TBL_L4_PORT2_OFFSET  44
	#define IN_FLOW_TBL_L4_PORT2_LEN     16
	#define IN_FLOW_TBL_L4_PORT2_DEFAULT 0x0
	/*[field] PORT_VP_VALID1 reuse FWD_TYPE[2]*/
	#define IN_FLOW_TBL_PORT_VP_VALID1
	#define IN_FLOW_TBL_PORT_VP_VALID1_OFFSET  44
	#define IN_FLOW_TBL_PORT_VP_VALID1_LEN     1
	#define IN_FLOW_TBL_PORT_VP_VALID1_DEFAULT 0x0
	/*[field] L4_PORT1 reuse FWD_TYPE[0]*/
	#define IN_FLOW_TBL_L4_PORT1
	#define IN_FLOW_TBL_L4_PORT1_OFFSET  44
	#define IN_FLOW_TBL_L4_PORT1_LEN     16
	#define IN_FLOW_TBL_L4_PORT1_DEFAULT 0x0
	/*[field] PORT_VP1 reuse FWD_TYPE[2]*/
	#define IN_FLOW_TBL_PORT_VP1
	#define IN_FLOW_TBL_PORT_VP1_OFFSET  45
	#define IN_FLOW_TBL_PORT_VP1_LEN     8
	#define IN_FLOW_TBL_PORT_VP1_DEFAULT 0x0
	/*[field] DE_ACCE*/
	#define IN_FLOW_TBL_DE_ACCE
	#define IN_FLOW_TBL_DE_ACCE_OFFSET  60
	#define IN_FLOW_TBL_DE_ACCE_LEN     1
	#define IN_FLOW_TBL_DE_ACCE_DEFAULT 0x0
	/*[field] COPY_TO_CPU_EN*/
	#define IN_FLOW_TBL_COPY_TO_CPU_EN
	#define IN_FLOW_TBL_COPY_TO_CPU_EN_OFFSET  61
	#define IN_FLOW_TBL_COPY_TO_CPU_EN_LEN     1
	#define IN_FLOW_TBL_COPY_TO_CPU_EN_DEFAULT 0x0
	/*[field] SYN_TOGGLE*/
	#define IN_FLOW_TBL_SYN_TOGGLE
	#define IN_FLOW_TBL_SYN_TOGGLE_OFFSET  62
	#define IN_FLOW_TBL_SYN_TOGGLE_LEN     1
	#define IN_FLOW_TBL_SYN_TOGGLE_DEFAULT 0x0
	/*[field] PRI_PROFILE*/
	#define IN_FLOW_TBL_PRI_PROFILE
	#define IN_FLOW_TBL_PRI_PROFILE_OFFSET  63
	#define IN_FLOW_TBL_PRI_PROFILE_LEN     5
	#define IN_FLOW_TBL_PRI_PROFILE_DEFAULT 0x0
	/*[field] SERVICE_CODE*/
	#define IN_FLOW_TBL_SERVICE_CODE
	#define IN_FLOW_TBL_SERVICE_CODE_OFFSET  68
	#define IN_FLOW_TBL_SERVICE_CODE_LEN     8
	#define IN_FLOW_TBL_SERVICE_CODE_DEFAULT 0x0
	/*[field] IP_ADDR*/
	#define IN_FLOW_TBL_IP_ADDR
	#define IN_FLOW_TBL_IP_ADDR_OFFSET  76
	#define IN_FLOW_TBL_IP_ADDR_LEN     32
	#define IN_FLOW_TBL_IP_ADDR_DEFAULT 0x0
	/*[field] L4_SPORT*/
	#define IN_FLOW_TBL_L4_SPORT
	#define IN_FLOW_TBL_L4_SPORT_OFFSET  108
	#define IN_FLOW_TBL_L4_SPORT_LEN     16
	#define IN_FLOW_TBL_L4_SPORT_DEFAULT 0x0
	/*[field] L4_DPORT*/
	#define IN_FLOW_TBL_L4_DPORT
	#define IN_FLOW_TBL_L4_DPORT_OFFSET  124
	#define IN_FLOW_TBL_L4_DPORT_LEN     16
	#define IN_FLOW_TBL_L4_DPORT_DEFAULT 0x0
#if defined(APPE)
	/*[field] VPN_ID*/
	#define IN_FLOW_TBL_VPN_ID
	#define IN_FLOW_TBL_VPN_ID_OFFSET  140
	#define IN_FLOW_TBL_VPN_ID_LEN     8
	#define IN_FLOW_TBL_VPN_ID_DEFAULT 0x0
	/*[field] PMTU_CHECK_TYPE*/
	#define IN_FLOW_TBL_PMTU_CHECK_TYPE
	#define IN_FLOW_TBL_PMTU_CHECK_TYPE_OFFSET  148
	#define IN_FLOW_TBL_PMTU_CHECK_TYPE_LEN     1
	#define IN_FLOW_TBL_PMTU_CHECK_TYPE_DEFAULT 0x0
	/*[field] PMTU*/
	#define IN_FLOW_TBL_PMTU
	#define IN_FLOW_TBL_PMTU_OFFSET  149
	#define IN_FLOW_TBL_PMTU_LEN     14
	#define IN_FLOW_TBL_PMTU_DEFAULT 0x0
#endif

struct in_flow_ipv6_3tuple_tbl_3 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
	a_uint32_t  age:2;
	a_uint32_t  protocol_type:2;
	a_uint32_t  host_addr_index:13;
	a_uint32_t  host_addr_index_type:1;
        a_uint32_t  entry_type:1;
        a_uint32_t  valid:1;
	
	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
        a_uint32_t  _reserved0:16;
        a_uint32_t  next_hop2:12;

#if defined(APPE)
	a_uint32_t  pmtu_0:11;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  _reserved2_0:12;
        a_uint32_t  ip_protocol:8;
	a_uint32_t  _reserved1:9;
	a_uint32_t  pmtu_1:3;
#else
	a_uint32_t  _reserved1_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  _reserved2_0:12;
        a_uint32_t  ip_protocol:8;
        a_uint32_t  _reserved1_1:12;
#endif

	a_uint32_t  ip_addr_0:20;
        a_uint32_t  _reserved2_1:12;
	a_uint32_t  ip_addr_1:32;
	a_uint32_t  ip_addr_2:32;
	a_uint32_t  ip_addr_3:32;
	a_uint32_t  _reserved3:20;
        a_uint32_t  ip_addr_4:12;
};

struct in_flow_ipv6_3tuple_tbl_1 {
	a_uint32_t  fwd_type:3;
	a_uint32_t  src_l3_if:8;
	a_uint32_t  src_l3_if_valid:1;
	a_uint32_t  age:2;
	a_uint32_t  protocol_type:2;
	a_uint32_t  host_addr_index:13;
	a_uint32_t  host_addr_index_type:1;
	a_uint32_t  entry_type:1;
	a_uint32_t  valid:1;

	a_uint32_t  pri_profile_0:1;
	a_uint32_t  syn_toggle:1;
	a_uint32_t  copy_to_cpu_en:1;
	a_uint32_t  de_acce:1;
	a_uint32_t  _reserved0:16;
	a_uint32_t  next_hop1:12;


#if defined(APPE)
	a_uint32_t  pmtu_0:11;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
	a_uint32_t  service_code:8;
	a_uint32_t  pri_profile_1:4;

	a_uint32_t  _reserved2_0:12;
	a_uint32_t  ip_protocol:8;
	a_uint32_t  _reserved1:9;
	a_uint32_t  pmtu_1:3;
#else
	a_uint32_t  _reserved1_0:20;
	a_uint32_t  service_code:8;
	a_uint32_t  pri_profile_1:4;

	a_uint32_t  _reserved2_0:12;
	a_uint32_t  ip_protocol:8;
	a_uint32_t  _reserved1_1:12;
#endif


	a_uint32_t  ip_addr_0:20;
	a_uint32_t  _reserved2_1:12;

	a_uint32_t  ip_addr_1:32;
	a_uint32_t  ip_addr_2:32;
	a_uint32_t  ip_addr_3:32;

	a_uint32_t  _reserved3:20;
	a_uint32_t  ip_addr_4:12;
};

struct in_flow_ipv6_3tuple_tbl_0 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
	a_uint32_t  age:2;
	a_uint32_t  protocol_type:2;
	a_uint32_t  host_addr_index:13;
	a_uint32_t  host_addr_index_type:1;
        a_uint32_t  entry_type:1;
        a_uint32_t  valid:1;

	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
#if defined(APPE)
#if defined(MPPE)
	a_uint32_t  _reserved0:4;
	a_uint32_t  next_hop4:12;
	a_uint32_t  next_hop4_valid:1;
#else
	a_uint32_t  _reserved0:17;
#endif
	a_uint32_t  cvlan_fmt:1;
	a_uint32_t  svlan_fmt:1;
	a_uint32_t  vlan_fmt_valid:1;
#else
	a_uint32_t  _reserved0:20;
#endif
	a_uint32_t  port_vp2:8;

#if defined(APPE)
	a_uint32_t  pmtu_0:11;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  _reserved2_0:12;
        a_uint32_t  ip_protocol:8;
	a_uint32_t  _reserved1:9;
	a_uint32_t  pmtu_1:3;
#else
	a_uint32_t  _reserved1_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  _reserved2_0:12;
        a_uint32_t  ip_protocol:8;
        a_uint32_t  _reserved1_1:12;
#endif

	a_uint32_t  ip_addr_0:20;
        a_uint32_t  _reserved2_1:12;

	a_uint32_t  ip_addr_1:32;
	a_uint32_t  ip_addr_2:32;
	a_uint32_t  ip_addr_3:32;
	a_uint32_t  _reserved3:20;
        a_uint32_t  ip_addr_4:12;
};


struct in_flow_ipv6_3tuple_tbl_2 {
	a_uint32_t  fwd_type:3;
        a_uint32_t  src_l3_if:8;
        a_uint32_t  src_l3_if_valid:1;
        a_uint32_t  age:2;
        a_uint32_t  protocol_type:2;
        a_uint32_t  host_addr_index:13;
        a_uint32_t  host_addr_index_type:1;
        a_uint32_t  entry_type:1;
        a_uint32_t  valid:1;

	a_uint32_t  pri_profile_0:1;
        a_uint32_t  syn_toggle:1;
        a_uint32_t  copy_to_cpu_en:1;
        a_uint32_t  de_acce:1;
        a_uint32_t  _reserved0:7;
        a_uint32_t  port_vp1:8;
        a_uint32_t  port_vp_valid1:1;
        a_uint32_t  next_hop3:12;

#if defined(APPE)
	a_uint32_t  pmtu_0:11;
	a_uint32_t  pmtu_check_type:1;
	a_uint32_t  vpn_id:8;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  _reserved2_0:12;
        a_uint32_t  ip_protocol:8;
	a_uint32_t  _reserved1:9;
	a_uint32_t  pmtu_1:3;
#else
	a_uint32_t  _reserved1_0:20;
        a_uint32_t  service_code:8;
        a_uint32_t  pri_profile_1:4;

	a_uint32_t  _reserved2_0:12;
        a_uint32_t  ip_protocol:8;
        a_uint32_t  _reserved1_1:12;
#endif

	a_uint32_t  ip_addr_0:20;
        a_uint32_t  _reserved2_1:12;

	a_uint32_t  ip_addr_1:32;
	a_uint32_t  ip_addr_2:32;
	a_uint32_t  ip_addr_3:32;
	a_uint32_t  _reserved3:20;
        a_uint32_t  ip_addr_4:12;
};

union in_flow_ipv6_3tuple_tbl_u {
	a_uint32_t val[9];
	struct in_flow_ipv6_3tuple_tbl_0 bf0;
	struct in_flow_ipv6_3tuple_tbl_1 bf1;
	struct in_flow_ipv6_3tuple_tbl_2 bf2;
	struct in_flow_ipv6_3tuple_tbl_3 bf3;
};

/*[table] EG_FLOW_TREE_MAP_TBL*/
#define EG_FLOW_TREE_MAP_TBL
#if defined(APPE)
#define EG_FLOW_TREE_MAP_TBL_ADDRESS 0x10000
#else
#define EG_FLOW_TREE_MAP_TBL_ADDRESS 0x8000
#endif
#define EG_FLOW_TREE_MAP_TBL_NUM     4096
#if defined(APPE)
#define EG_FLOW_TREE_MAP_TBL_INC     0x10
#else
#define EG_FLOW_TREE_MAP_TBL_INC     0x4
#endif
#define EG_FLOW_TREE_MAP_TBL_TYPE    REG_TYPE_RW
#define EG_FLOW_TREE_MAP_TBL_DEFAULT 0x0
	/*[field] TREE_ID*/
	#define EG_FLOW_TREE_MAP_TBL_TREE_ID
	#define EG_FLOW_TREE_MAP_TBL_TREE_ID_OFFSET  0
	#define EG_FLOW_TREE_MAP_TBL_TREE_ID_LEN     24
	#define EG_FLOW_TREE_MAP_TBL_TREE_ID_DEFAULT 0x0
#if defined(APPE)
	/*[field] WIFI_QOS*/
	#define EG_FLOW_TREE_MAP_TBL_WIFI_QOS
	#define EG_FLOW_TREE_MAP_TBL_WIFI_QOS_OFFSET  24
	#define EG_FLOW_TREE_MAP_TBL_WIFI_QOS_LEN     8
	#define EG_FLOW_TREE_MAP_TBL_WIFI_QOS_DEFAULT 0x0
	/*[field] WIFI_QOS_FLAG*/
	#define EG_FLOW_TREE_MAP_TBL_WIFI_QOS_FLAG
	#define EG_FLOW_TREE_MAP_TBL_WIFI_QOS_FLAG_OFFSET  32
	#define EG_FLOW_TREE_MAP_TBL_WIFI_QOS_FLAG_LEN     1
	#define EG_FLOW_TREE_MAP_TBL_WIFI_QOS_FLAG_DEFAULT 0x0
#endif

struct eg_flow_tree_map_tbl {
#if defined(APPE)
#if defined(MPPE)
	a_uint32_t  _reserved0:29;
	a_uint32_t  type:2;
#else
	a_uint32_t  _reserved0:31;
#endif
	a_uint32_t  wifi_qos_flag:1;
	a_uint32_t  wifi_qos:8;
#else
	a_uint32_t  _reserved0:8;
#endif
	a_uint32_t  tree_id:24;

};

union eg_flow_tree_map_tbl_u {
#if defined(APPE)
	a_uint32_t val[2];
#else
	a_uint32_t val;
#endif
	struct eg_flow_tree_map_tbl bf;
};

/*[table] IN_FLOW_CNT_TBL*/
#define IN_FLOW_CNT_TBL
#define IN_FLOW_CNT_TBL_ADDRESS 0x20000
#define IN_FLOW_CNT_TBL_NUM     4096
#define IN_FLOW_CNT_TBL_INC     0x10
#define IN_FLOW_CNT_TBL_TYPE    REG_TYPE_RW
#define IN_FLOW_CNT_TBL_DEFAULT 0x0
	/*[field] HIT_PKT_COUNTER*/
	#define IN_FLOW_CNT_TBL_HIT_PKT_COUNTER
	#define IN_FLOW_CNT_TBL_HIT_PKT_COUNTER_OFFSET  0
	#define IN_FLOW_CNT_TBL_HIT_PKT_COUNTER_LEN     32
	#define IN_FLOW_CNT_TBL_HIT_PKT_COUNTER_DEFAULT 0x0
	/*[field] HIT_BYTE_COUNTER*/
	#define IN_FLOW_CNT_TBL_HIT_BYTE_COUNTER
	#define IN_FLOW_CNT_TBL_HIT_BYTE_COUNTER_OFFSET  32
	#define IN_FLOW_CNT_TBL_HIT_BYTE_COUNTER_LEN     40
	#define IN_FLOW_CNT_TBL_HIT_BYTE_COUNTER_DEFAULT 0x0

struct in_flow_cnt_tbl {
	a_uint32_t  hit_pkt_counter:32;
	a_uint32_t  hit_byte_counter_0:32;
	a_uint32_t  _reserved0:24;
        a_uint32_t  hit_byte_counter_1:8;
};

union in_flow_cnt_tbl_u {
	a_uint32_t val[3];
	struct in_flow_cnt_tbl bf;
};

/*[table] IN_L3_IF_TBL*/
#define EG_IPV6_PREFIX_TBL
#define EG_IPV6_PREFIX_TBL_ADDRESS 0x3e000
#define EG_IPV6_PREFIX_TBL_INC     0x20


#define EG_IPV6_PREFIX_TBL_NUM     256
#define EG_IPV6_PREFIX_TBL_TYPE    REG_TYPE_RW
#define EG_IPV6_PREFIX_TBL_DEFAULT 0x0
	/*[field] PREFIX*/
	#define EG_IPV6_PREFIX_TBL_PREFIX
	#define EG_IPV6_PREFIX_TBL_PREFIX_OFFSET  0
	#define EG_IPV6_PREFIX_TBL_PREFIX_LEN     128
	#define EG_IPV6_PREFIX_TBL_PREFIX_DEFAULT 0x0

	/*[field] LENGTH*/
	#define EG_IPV6_PREFIX_TBL_LENGTH
	#define EG_IPV6_PREFIX_TBL_LENGTH_OFFSET  128
	#define EG_IPV6_PREFIX_TBL_LENGTH_LEN	  7
	#define EG_IPV6_PREFIX_TBL_LENGTH_DEFAULT 0x0


#define EG_FLOW_IPV6_IID_TBL_ADDRESS 0x30000
#define EG_FLOW_IPV6_IID_TBL_INC     0x4


#define EG_FLOW_IPV6_IID_TBL_NUM     2047
#define EG_FLOW_IPV6_IID_TBL_TYPE    REG_TYPE_RW
#define EG_FLOW_IPV6_IID_TBL_DEFAULT 0x0
	/*[field] IID*/
	#define EG_FLOW_IPV6_IID_TBL_IID
	#define EG_FLOW_IPV6_IID_TBL_IID_OFFSET  0
	#define EG_FLOW_IPV6_IID_TBL_IID_LEN     16
	#define EG_FLOW_IPV6_IID_TBL_IID_DEFAULT 0x0

	/*[field] OFFSET*/
	#define EG_FLOW_IPV6_IID_TBL_OFFSET
	#define EG_FLOW_IPV6_IID_TBL_OFFSET_OFFSET  16
	#define EG_FLOW_IPV6_IID_TBL_OFFSET_LEN	  	3
	#define EG_FLOW_IPV6_IID_TBL_OFFSET_DEFAULT 0x0

	/*[field] IID_UPDATE*/
	#define EG_FLOW_IPV6_IID_TBL_IID_UPDATE
	#define EG_FLOW_IPV6_IID_TBL_IID_UPDATE_OFFSET	19
	#define EG_FLOW_IPV6_IID_TBL_IID_UPDATE_LEN   	1
	#define EG_FLOW_IPV6_IID_TBL_IID_UPDATE_DEFAULT 0x0

	/*[field] PREFIX_UPDATE*/
	#define EG_FLOW_IPV6_IID_TBL_PREFIX_UPDATE
	#define EG_FLOW_IPV6_IID_TBL_PREFIX_UPDATE_OFFSET	20
	#define EG_FLOW_IPV6_IID_TBL_PREFIX_UPDATE_LEN   	1
	#define EG_FLOW_IPV6_IID_TBL_PREFIX_UPDATE_DEFAULT 	0x0

	/*[field] SRC_DST*/
	#define EG_FLOW_IPV6_IID_TBL_SRC_DST
	#define EG_FLOW_IPV6_IID_TBL_SRC_DST_OFFSET			21
	#define EG_FLOW_IPV6_IID_TBL_SRC_DST_LEN	 		1
	#define EG_FLOW_IPV6_IID_TBL_SRC_DST_DEFAULT 		0x0

/*[register] EG_GLOBAL_CTRL*/
#define EG_GLOBAL_CTRL
#define EG_GLOBAL_CTRL_ADDRESS 0xe0

#define EG_GLOBAL_CTRL_NUM     1
#define EG_GLOBAL_CTRL_INC     0x4
#define EG_GLOBAL_CTRL_TYPE    REG_TYPE_RW
	/*[field] PREFIX_XLT_EN*/
	#define EG_GLOBAL_CTRL_PREFIX_XLT_EN
	#define EG_GLOBAL_CTRL_PREFIX_XLT_EN_OFFSET  0
	#define EG_GLOBAL_CTRL_PREFIX_XLT_EN_LEN     1
	#define EG_GLOBAL_CTRL_PREFIX_XLT_EN_DEFAULT 0x0

struct eg_ipv6_prefix_tbl {
	a_uint32_t  _reserved0:25;
	a_uint32_t  length:7;
	a_uint32_t  prefix[4];
};

union eg_ipv6_prefix_tbl_u {
	a_uint32_t val[5];
	struct eg_ipv6_prefix_tbl bf;
};

struct eg_flow_ipv6_iid_tbl {
	a_uint32_t  _reserved0:10;
	a_uint32_t  src_dst:1;
	a_uint32_t  prefix_update:1;
	a_uint32_t  iid_update:1;
	a_uint32_t  offset:3;
	a_uint32_t  iid:16;
};

union eg_flow_ipv6_iid_tbl_u {
	a_uint32_t val[1];
	struct eg_flow_ipv6_iid_tbl bf;
};

struct eg_global_ctrl {
	a_uint32_t  _reserved0:31;
	a_uint32_t  prefix_xlt_en:1;
};

union eg_global_ctrl_u {
	a_uint32_t val;
	struct eg_global_ctrl bf;
};


#endif
