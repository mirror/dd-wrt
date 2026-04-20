/*
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

#ifndef _PPE_DRV_TUN_L2TP_H_
#define _PPE_DRV_TUN_L2TP_H_

#define PPE_DRV_TUN_L2TP_V2_PACKET_TYPE	0x2	/* L2TP v2 Packet Type with only version number set */
#define PPE_DRV_TUN_L2TP_PPP_ADDRESS	0xff	/* L2TP v2 PPP fixed address feild value */
#define PPE_DRV_TUN_L2TP_PPP_CONTROL	0x03	/* L2TP v2 PPP fixed control feild value */

bool ppe_drv_tun_prgm_prsr_l2tp_deconfigure(struct ppe_drv_tun_prgm_prsr *program);
bool ppe_drv_tun_l2tp_prgm_prsr_configure(struct ppe_drv_tun_prgm_prsr *program);
bool ppe_drv_tun_l2tp_port_set(uint16_t sport, uint16_t dport);
bool ppe_drv_tun_l2tp_port_get(uint16_t *sport, uint16_t *dport);
#endif
