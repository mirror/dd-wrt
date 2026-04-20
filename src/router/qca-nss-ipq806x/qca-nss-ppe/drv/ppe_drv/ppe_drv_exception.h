/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <fal/fal_init.h>

/*
 * PPE exception type.
 */
#define PPE_DRV_EXCEPTION_FLOW_TYPE_L2_ONLY		0x1
#define PPE_DRV_EXCEPTION_FLOW_TYPE_L3_ONLY		0x2
#define PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW		0x4
#define PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW		0x8
#define PPE_DRV_EXCEPTION_FLOW_TYPE_MULTICAST		0x10
#define PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT		0x20
#define PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT		0x40
#define PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_MISS	0x80
#define PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_MISS	0x100

/*
 * Deacceleration action for exception
 */
#define PPE_DRV_EXCEPTION_DEACCEL_DIS		0
#define PPE_DRV_EXCEPTION_DEACCEL_EN		1

/*
 * TCP flag definitions
 */
#define PPE_DRV_TCP_FLAG_FIN 0x01
#define PPE_DRV_TCP_FLAG_SYN 0x02
#define PPE_DRV_TCP_FLAG_RST 0x04
#define PPE_DRV_TCP_FLAG_PSH 0x08
#define PPE_DRV_TCP_FLAG_ACK 0x10
#define PPE_DRV_TCP_FLAG_URG 0x20

/*
 * ppe_drv_exception
 *	ppe per exception configuration.
 */
struct ppe_drv_exception {
	ppe_drv_cc_t code;
	fal_fwd_cmd_t action;
	uint8_t deaccel_en;
	uint8_t flow_type;
};

/*
 * ppe_drv_exception_tcpflag
 *	ppe tcp flag exception.
 */
struct ppe_drv_exception_tcpflag {
	uint8_t flags;				/* TCP Flag for which exception is configured */
};

/*
 * Current exception list.
 */
extern struct ppe_drv_exception ppe_drv_exception_list[];

const uint8_t ppe_drv_exception_max(void);
const uint8_t ppe_drv_exception_tcpflag_max(void);
void ppe_drv_exception_init(void);
