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
#include "appe_shaper_reg_be.h"
#else
#ifndef APPE_SHAPER_REG_H
#define APPE_SHAPER_REG_H

/*[register] MIN_MAX_MODE_CFG*/
#define MIN_MAX_MODE_CFG
#define MIN_MAX_MODE_CFG_ADDRESS 0x4
#define MIN_MAX_MODE_CFG_NUM     1
#define MIN_MAX_MODE_CFG_INC     0x4
#define MIN_MAX_MODE_CFG_TYPE    REG_TYPE_RW
#define MIN_MAX_MODE_CFG_DEFAULT 0x0
	/*[field] MIN_MAX_MODE*/
	#define MIN_MAX_MODE_CFG_MIN_MAX_MODE
	#define MIN_MAX_MODE_CFG_MIN_MAX_MODE_OFFSET  0
	#define MIN_MAX_MODE_CFG_MIN_MAX_MODE_LEN     1
	#define MIN_MAX_MODE_CFG_MIN_MAX_MODE_DEFAULT 0x0

struct min_max_mode_cfg {
	a_uint32_t  min_max_mode:1;
	a_uint32_t  _reserved0:31;
};

union min_max_mode_cfg_u {
	a_uint32_t val;
	struct min_max_mode_cfg bf;
};

/*[register] ECO_RESERVE_0*/
#define ECO_RESERVE_0
#define ECO_RESERVE_0_ADDRESS 0x28
#define ECO_RESERVE_0_NUM     1
#define ECO_RESERVE_0_INC     0x4
#define ECO_RESERVE_0_TYPE    REG_TYPE_RW
#define ECO_RESERVE_0_DEFAULT 0x0
	/*[field] ECO_RES_0*/
	#define ECO_RESERVE_0_ECO_RES_0
	#define ECO_RESERVE_0_ECO_RES_0_OFFSET  0
	#define ECO_RESERVE_0_ECO_RES_0_LEN     32
	#define ECO_RESERVE_0_ECO_RES_0_DEFAULT 0x0

struct eco_reserve_0 {
	a_uint32_t  eco_res_0:32;
};

union eco_reserve_0_u {
	a_uint32_t val;
	struct eco_reserve_0 bf;
};

/*[register] ECO_RESERVE_1*/
#define ECO_RESERVE_1
#define ECO_RESERVE_1_ADDRESS 0x2c
#define ECO_RESERVE_1_NUM     1
#define ECO_RESERVE_1_INC     0x4
#define ECO_RESERVE_1_TYPE    REG_TYPE_RW
#define ECO_RESERVE_1_DEFAULT 0x0
	/*[field] ECO_RES_1*/
	#define ECO_RESERVE_1_ECO_RES_1
	#define ECO_RESERVE_1_ECO_RES_1_OFFSET  0
	#define ECO_RESERVE_1_ECO_RES_1_LEN     32
	#define ECO_RESERVE_1_ECO_RES_1_DEFAULT 0x0

struct eco_reserve_1 {
	a_uint32_t  eco_res_1:32;
};

union eco_reserve_1_u {
	a_uint32_t val;
	struct eco_reserve_1 bf;
};

/*[register] SHP_CFG_L0*/
#define SHP_CFG_L0
#define SHP_CFG_L0_ADDRESS 0x30
#define SHP_CFG_L0_NUM     1
#define SHP_CFG_L0_INC     0x4
#define SHP_CFG_L0_TYPE    REG_TYPE_RW
#define SHP_CFG_L0_DEFAULT 0x0
	/*[field] L0_SHP_LL_TAIL*/
	#define SHP_CFG_L0_L0_SHP_LL_TAIL
	#define SHP_CFG_L0_L0_SHP_LL_TAIL_OFFSET  0
	#define SHP_CFG_L0_L0_SHP_LL_TAIL_LEN     9
	#define SHP_CFG_L0_L0_SHP_LL_TAIL_DEFAULT 0x0
	/*[field] L0_SHP_LL_HEAD*/
	#define SHP_CFG_L0_L0_SHP_LL_HEAD
	#define SHP_CFG_L0_L0_SHP_LL_HEAD_OFFSET  16
	#define SHP_CFG_L0_L0_SHP_LL_HEAD_LEN     9
	#define SHP_CFG_L0_L0_SHP_LL_HEAD_DEFAULT 0x0

struct shp_cfg_l0 {
	a_uint32_t  l0_shp_ll_tail:9;
	a_uint32_t  _reserved0:7;
	a_uint32_t  l0_shp_ll_head:9;
	a_uint32_t  _reserved1:7;
};

union shp_cfg_l0_u {
	a_uint32_t val;
	struct shp_cfg_l0 bf;
};

/*[register] SHP_CFG_L1*/
#define SHP_CFG_L1
#define SHP_CFG_L1_ADDRESS 0x34
#define SHP_CFG_L1_NUM     1
#define SHP_CFG_L1_INC     0x4
#define SHP_CFG_L1_TYPE    REG_TYPE_RW
#define SHP_CFG_L1_DEFAULT 0x0
	/*[field] L1_SHP_LL_TAIL*/
	#define SHP_CFG_L1_L1_SHP_LL_TAIL
	#define SHP_CFG_L1_L1_SHP_LL_TAIL_OFFSET  0
	#define SHP_CFG_L1_L1_SHP_LL_TAIL_LEN     6
	#define SHP_CFG_L1_L1_SHP_LL_TAIL_DEFAULT 0x0
	/*[field] L1_SHP_LL_HEAD*/
	#define SHP_CFG_L1_L1_SHP_LL_HEAD
	#define SHP_CFG_L1_L1_SHP_LL_HEAD_OFFSET  16
	#define SHP_CFG_L1_L1_SHP_LL_HEAD_LEN     6
	#define SHP_CFG_L1_L1_SHP_LL_HEAD_DEFAULT 0x0

struct shp_cfg_l1 {
	a_uint32_t  l1_shp_ll_tail:6;
	a_uint32_t  _reserved0:10;
	a_uint32_t  l1_shp_ll_head:6;
	a_uint32_t  _reserved1:10;
};

union shp_cfg_l1_u {
	a_uint32_t val;
	struct shp_cfg_l1 bf;
};

#endif
#endif
