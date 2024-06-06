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
#include "appe_counter_reg_be.h"
#else

#ifndef APPE_COUNTER_REG_H
#define APPE_COUNTER_REG_H
/*[table] PORT_RX_CNT_TBL*/
#define PORT_RX_CNT_TBL
#define PORT_RX_CNT_TBL_ADDRESS 0x50000
#define PORT_RX_CNT_TBL_NUM     256
#define PORT_RX_CNT_TBL_INC     0x20
#define PORT_RX_CNT_TBL_TYPE    REG_TYPE_RW
#define PORT_RX_CNT_TBL_DEFAULT 0x0
	/*[field] RX_PKT_CNT*/
	#define PORT_RX_CNT_TBL_RX_PKT_CNT
	#define PORT_RX_CNT_TBL_RX_PKT_CNT_OFFSET  0
	#define PORT_RX_CNT_TBL_RX_PKT_CNT_LEN     32
	#define PORT_RX_CNT_TBL_RX_PKT_CNT_DEFAULT 0x0
	/*[field] RX_BYTE_CNT*/
	#define PORT_RX_CNT_TBL_RX_BYTE_CNT
	#define PORT_RX_CNT_TBL_RX_BYTE_CNT_OFFSET  32
	#define PORT_RX_CNT_TBL_RX_BYTE_CNT_LEN     40
	#define PORT_RX_CNT_TBL_RX_BYTE_CNT_DEFAULT 0x0
	/*[field] RX_DROP_PKT_CNT*/
	#define PORT_RX_CNT_TBL_RX_DROP_PKT_CNT
	#define PORT_RX_CNT_TBL_RX_DROP_PKT_CNT_OFFSET  72
	#define PORT_RX_CNT_TBL_RX_DROP_PKT_CNT_LEN     32
	#define PORT_RX_CNT_TBL_RX_DROP_PKT_CNT_DEFAULT 0x0
	/*[field] RX_DROP_BYTE_CNT*/
	#define PORT_RX_CNT_TBL_RX_DROP_BYTE_CNT
	#define PORT_RX_CNT_TBL_RX_DROP_BYTE_CNT_OFFSET  104
	#define PORT_RX_CNT_TBL_RX_DROP_BYTE_CNT_LEN     40
	#define PORT_RX_CNT_TBL_RX_DROP_BYTE_CNT_DEFAULT 0x0

struct port_rx_cnt_tbl {
	a_uint32_t  rx_pkt_cnt:32;
	a_uint32_t  rx_byte_cnt_0:32;
	a_uint32_t  rx_byte_cnt_1:8;
	a_uint32_t  rx_drop_pkt_cnt_0:24;
	a_uint32_t  rx_drop_pkt_cnt_1:8;
	a_uint32_t  rx_drop_byte_cnt_0:24;
	a_uint32_t  rx_drop_byte_cnt_1:16;
	a_uint32_t  _reserved0:16;
};

union port_rx_cnt_tbl_u {
	a_uint32_t val[5];
	struct port_rx_cnt_tbl bf;
};

/*[table] PHY_PORT_RX_CNT_TBL*/
#define PHY_PORT_RX_CNT_TBL
#define PHY_PORT_RX_CNT_TBL_ADDRESS 0x56000
#define PHY_PORT_RX_CNT_TBL_NUM     8
#define PHY_PORT_RX_CNT_TBL_INC     0x20
#define PHY_PORT_RX_CNT_TBL_TYPE    REG_TYPE_RW
#define PHY_PORT_RX_CNT_TBL_DEFAULT 0x0
	/*[field] RX_PKT_CNT*/
	#define PHY_PORT_RX_CNT_TBL_RX_PKT_CNT
	#define PHY_PORT_RX_CNT_TBL_RX_PKT_CNT_OFFSET  0
	#define PHY_PORT_RX_CNT_TBL_RX_PKT_CNT_LEN     32
	#define PHY_PORT_RX_CNT_TBL_RX_PKT_CNT_DEFAULT 0x0
	/*[field] RX_BYTE_CNT*/
	#define PHY_PORT_RX_CNT_TBL_RX_BYTE_CNT
	#define PHY_PORT_RX_CNT_TBL_RX_BYTE_CNT_OFFSET  32
	#define PHY_PORT_RX_CNT_TBL_RX_BYTE_CNT_LEN     40
	#define PHY_PORT_RX_CNT_TBL_RX_BYTE_CNT_DEFAULT 0x0
	/*[field] RX_DROP_PKT_CNT*/
	#define PHY_PORT_RX_CNT_TBL_RX_DROP_PKT_CNT
	#define PHY_PORT_RX_CNT_TBL_RX_DROP_PKT_CNT_OFFSET  72
	#define PHY_PORT_RX_CNT_TBL_RX_DROP_PKT_CNT_LEN     32
	#define PHY_PORT_RX_CNT_TBL_RX_DROP_PKT_CNT_DEFAULT 0x0
	/*[field] RX_DROP_BYTE_CNT*/
	#define PHY_PORT_RX_CNT_TBL_RX_DROP_BYTE_CNT
	#define PHY_PORT_RX_CNT_TBL_RX_DROP_BYTE_CNT_OFFSET  104
	#define PHY_PORT_RX_CNT_TBL_RX_DROP_BYTE_CNT_LEN     40
	#define PHY_PORT_RX_CNT_TBL_RX_DROP_BYTE_CNT_DEFAULT 0x0

struct phy_port_rx_cnt_tbl {
	a_uint32_t  rx_pkt_cnt:32;
	a_uint32_t  rx_byte_cnt_0:32;
	a_uint32_t  rx_byte_cnt_1:8;
	a_uint32_t  rx_drop_pkt_cnt_0:24;
	a_uint32_t  rx_drop_pkt_cnt_1:8;
	a_uint32_t  rx_drop_byte_cnt_0:24;
	a_uint32_t  rx_drop_byte_cnt_1:16;
	a_uint32_t  _reserved0:16;
};

union phy_port_rx_cnt_tbl_u {
	a_uint32_t val[5];
	struct phy_port_rx_cnt_tbl bf;
};

/*[table] PORT_VP_RX_CNT_MODE*/
#define PORT_VP_RX_CNT_MODE
#define PORT_VP_RX_CNT_MODE_ADDRESS 0x100
#define PORT_VP_RX_CNT_MODE_NUM     8
#define PORT_VP_RX_CNT_MODE_INC     0x4
#define PORT_VP_RX_CNT_MODE_TYPE    REG_TYPE_RW
#define PORT_VP_RX_CNT_MODE_DEFAULT 0x0
	/*[field] CNT_MODE*/
	#define PORT_VP_RX_CNT_MODE_CNT_MODE
	#define PORT_VP_RX_CNT_MODE_CNT_MODE_OFFSET  0
	#define PORT_VP_RX_CNT_MODE_CNT_MODE_LEN     32
	#define PORT_VP_RX_CNT_MODE_CNT_MODE_DEFAULT 0x0

struct port_vp_rx_cnt_mode_tbl {
	a_uint32_t  cnt_mode:32;
};

union port_vp_rx_cnt_mode_tbl_u {
	a_uint32_t val;
	struct port_vp_rx_cnt_mode_tbl bf;
};

#endif
#endif
