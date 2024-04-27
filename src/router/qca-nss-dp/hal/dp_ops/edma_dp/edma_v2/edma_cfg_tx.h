/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __EDMA_CFG_TX_H__
#define __EDMA_CFG_TX_H__

#define EDMA_TX_NAPI_WORK_MIN   16
#define EDMA_TX_NAPI_WORK_MAX	512

#define EDMA_TX_MITIGATION_TIMER_MIN	0	/* Tx mitigation timer's minimum value in microseconds */
#define EDMA_TX_MITIGATION_TIMER_MAX	1000	/* Tx mitigation timer's maximum value in microseconds */
#define EDMA_TX_MITIGATION_PKT_CNT_MIN	0	/* Tx mitigation packet count's minimum value */
#define EDMA_TX_MITIGATION_PKT_CNT_MAX	256	/* Tx mitigation packet count's maximum value */

void edma_cfg_tx_rings(struct edma_gbl_ctx *egc);
int32_t edma_cfg_tx_rings_alloc(struct edma_gbl_ctx *egc);
void edma_cfg_tx_rings_cleanup(struct edma_gbl_ctx *egc);
void edma_cfg_tx_napi_enable(struct edma_gbl_ctx *egc);
void edma_cfg_tx_napi_disable(struct edma_gbl_ctx *egc);
void edma_cfg_tx_napi_delete(struct edma_gbl_ctx *egc);
void edma_cfg_tx_napi_add(struct edma_gbl_ctx *egc, struct net_device *netdev);
void edma_cfg_tx_mapping(struct edma_gbl_ctx *egc);
void edma_cfg_tx_rings_enable(struct edma_gbl_ctx *egc);
void edma_cfg_tx_rings_disable(struct edma_gbl_ctx *egc);
void edma_cfg_tx_fill_per_port_tx_map(struct net_device *netdev, uint32_t macid);
#if defined(NSS_DP_POINT_OFFLOAD)
void edma_cfg_tx_point_offload_mapping(struct edma_gbl_ctx *egc);
#endif
#endif	/* __EDMA_CFG_TX_H__ */
