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

#ifndef __EDMA_CFG_RX_H__
#define __EDMA_CFG_RX_H__

#define EDMA_RX_NAPI_WORK_MIN		16
#define EDMA_RX_NAPI_WORK_MAX		512
#define EDMA_RX_PAGE_MODE_SKB_SIZE	256	/* SKB payload size used in page mode */
#define EDMA_RX_DEFAULT_QUEUE_PRI	0
#define EDMA_RX_FC_ENABLE		1	/* RX flow control default state */
#define EDMA_RX_QUEUE_TAIL_DROP_ENABLE	0	/* RX queue tail drop configuration default state */
#define EDMA_RX_FC_XOFF_THRE_MIN	0	/* Rx flow control minimum X-OFF value */
#define EDMA_RX_FC_XON_THRE_MIN		0	/* Rx flow control mininum X-ON value */
#define EDMA_RX_AC_FC_THRE_ORIG		0x190	/* Rx AC flow control original threshold */
#define EDMA_RX_AC_FC_THRE_MIN		0	/* Rx AC flow control minimum threshold */
#define EDMA_RX_AC_FC_THRE_MAX		0x7ff	/* Rx AC flow control maximum threshold.
						   AC FC threshold value is 11 bits long */

#define EDMA_RX_MITIGATION_TIMER_MIN	0	/* Rx mitigation timer's minimum value in microseconds */
#define EDMA_RX_MITIGATION_TIMER_MAX	1000	/* Rx mitigation timer's maximum value in microseconds */
#define EDMA_RX_MITIGATION_PKT_CNT_MIN	0	/* Rx mitigation packet count's minimum value */
#define EDMA_RX_MITIGATION_PKT_CNT_MAX	256	/* Rx mitigation packet count's maximum value */

#if defined(NSS_DP_POINT_OFFLOAD)
/* TODO: we need to close with ssdk team to close this numbers */
#define EDMA_RX_POINT_OFFLOAD_QUEUE_BASE 56
#define EDMA_RX_POINT_OFFLOAD_QUEUE_NUM 3
#endif

extern uint32_t edma_cfg_rx_fc_enable;
extern uint32_t edma_cfg_rx_queue_tail_drop_enable;
extern uint32_t edma_cfg_rx_rps_num_cores;

void edma_cfg_rx_rings(struct edma_gbl_ctx *egc);
#if defined(NSS_DP_POINT_OFFLOAD)
void edma_cfg_rx_point_offload_mapping(struct edma_gbl_ctx *egc);
void edma_cfg_rx_point_offload_rings(struct edma_gbl_ctx *egc);
#endif
int32_t edma_cfg_rx_rings_alloc(struct edma_gbl_ctx *egc);
void edma_cfg_rx_rings_cleanup(struct edma_gbl_ctx *egc);
void edma_cfg_rx_napi_disable(struct edma_gbl_ctx *egc);
void edma_cfg_rx_napi_enable(struct edma_gbl_ctx *egc);
void edma_cfg_rx_napi_delete(struct edma_gbl_ctx *egc);
void edma_cfg_rx_napi_add(struct edma_gbl_ctx *egc, struct net_device *netdev);
void edma_cfg_rx_mapping(struct edma_gbl_ctx *egc);
void edma_cfg_rx_rings_enable(struct edma_gbl_ctx *egc);
void edma_cfg_rx_rings_disable(struct edma_gbl_ctx *egc);
int edma_cfg_rx_fc_enable_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos);
void edma_cfg_rx_page_mode_and_jumbo(struct edma_gbl_ctx *egc);
int edma_cfg_rx_queue_tail_drop_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos);
int edma_cfg_rx_rps(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos);
#endif	/* __EDMA_CFG_RX_H__ */
