/*
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
#if defined(CONFIG_CPU_BIG_ENDIAN)
#include "mppe_athtag_reg_be.h"
#else
#ifndef _MPPE_ATHTAG_REG_H_
#define _MPPE_ATHTAG_REG_H_

/*[table] EG_HDR_XMIT_PRI_MAPPING*/
#define EG_HDR_XMIT_PRI_MAPPING
#define EG_HDR_XMIT_PRI_MAPPING_ADDRESS 0x88
#define EG_HDR_XMIT_PRI_MAPPING_INC     0x4
#define EG_HDR_XMIT_PRI_MAPPING_TYPE    REG_TYPE_RW
#define EG_HDR_XMIT_PRI_MAPPING_DEFAULT 0x0

struct eg_hdr_xmit_pri_mapping {
	a_uint32_t  pri:3;
	a_uint32_t  _reserved0:29;
};

union eg_hdr_xmit_pri_mapping_u {
	a_uint32_t val;
	struct eg_hdr_xmit_pri_mapping bf;
};

/*[register] EG_GEN_CTRL*/
#define EG_GEN_CTRL
#define EG_GEN_CTRL_ADDRESS 0xd0
#define EG_GEN_CTRL_NUM     1
#define EG_GEN_CTRL_INC     0x4
#define EG_GEN_CTRL_TYPE    REG_TYPE_RW
#define EG_GEN_CTRL_DEFAULT 0x0

struct eg_gen_ctrl {
	a_uint32_t  ath_hdr_type:16;
	a_uint32_t  flow_cookie_pri:4;
	a_uint32_t  _reserved0:12;
};

union eg_gen_ctrl_u {
	a_uint32_t val;
	struct eg_gen_ctrl bf;
};

/*[table] PRX_PORT_TO_VP_MAPPING*/
#define PRX_PORT_TO_VP_MAPPING
#define PRX_PORT_TO_VP_MAPPING_ADDRESS 0x90
#define PRX_PORT_TO_VP_MAPPING_INC     0x4
#define PRX_PORT_TO_VP_MAPPING_TYPE    REG_TYPE_RW
#define PRX_PORT_TO_VP_MAPPING_DEFAULT 0x0

struct prx_port_to_vp_mapping {
	a_uint32_t  prx_ath_hdr_type:16;
	a_uint32_t  prx_port_vp:8;
	a_uint32_t  prx_ath_hdr_en:1;
	a_uint32_t  _reserved0:7;
};

union prx_port_to_vp_mapping_u {
	a_uint32_t val;
	struct prx_port_to_vp_mapping bf;
};

/*[table] PRX_HDR_RCV_PRI_MAPPING*/
#define PRX_HDR_RCV_PRI_MAPPING
#define PRX_HDR_RCV_PRI_MAPPING_ADDRESS 0xB0
#define PRX_HDR_RCV_PRI_MAPPING_INC     0x4
#define PRX_HDR_RCV_PRI_MAPPING_TYPE    REG_TYPE_RW
#define PRX_HDR_RCV_PRI_MAPPING_DEFAULT 0x0

struct prx_hdr_rcv_pri_mapping {
	a_uint32_t  pri:4;
	a_uint32_t  _reserved0:28;
};

union prx_hdr_rcv_pri_mapping_u {
	a_uint32_t val;
	struct prx_hdr_rcv_pri_mapping bf;
};
#endif
#endif
