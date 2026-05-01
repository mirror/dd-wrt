/* SPDX-License-Identifier: GPL-2.0-only
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __EDMA_CFG_RX__
#define __EDMA_CFG_RX__

/* Rx default NAPI budget */
#define EDMA_RX_NAPI_WORK_DEF		128

/* RX minimum NAPI budget */
#define EDMA_RX_NAPI_WORK_MIN		16

/* Rx maximum NAPI budget */
#define EDMA_RX_NAPI_WORK_MAX		512

/* SKB payload size used in page mode */
#define EDMA_RX_PAGE_MODE_SKB_SIZE	256

/* Rx flow control X-OFF default value */
#define EDMA_RX_FC_XOFF_DEF	32

/* Rx flow control X-ON default value */
#define EDMA_RX_FC_XON_DEF	64

/* Rx AC flow control original threshold */
#define EDMA_RX_AC_FC_THRE_ORIG		0x190

/* Rx AC flow control default threshold */
#define EDMA_RX_AC_FC_THRES_DEF		0x104
/* Rx mitigation timer's default value in microseconds */
#define EDMA_RX_MITIGATION_TIMER_DEF	25

/* Rx mitigation timer's minimum value in microseconds */
#define EDMA_RX_MITIGATION_TIMER_MIN	0

/* Rx mitigation timer's maximum value in microseconds */
#define EDMA_RX_MITIGATION_TIMER_MAX	1000

/* Rx mitigation packet count's default value */
#define EDMA_RX_MITIGATION_PKT_CNT_DEF	16

/* Rx mitigation packet count's minimum value */
#define EDMA_RX_MITIGATION_PKT_CNT_MIN	0

/* Rx mitigation packet count's maximum value */
#define EDMA_RX_MITIGATION_PKT_CNT_MAX	256

/* Default bitmap of cores for RPS to ARM cores */
#define EDMA_RX_DEFAULT_BITMAP	((1 << EDMA_MAX_CORE) - 1)

extern u32 edma_cfg_rx_rps_bitmap_cores;

int edma_cfg_rx_rings(void);
int edma_cfg_rx_rings_alloc(void);
void edma_cfg_rx_ring_mappings(void);
void edma_cfg_rx_rings_cleanup(void);
void edma_cfg_rx_disable_interrupts(void);
void edma_cfg_rx_enable_interrupts(void);
void edma_cfg_rx_napi_disable(void);
void edma_cfg_rx_napi_enable(void);
void edma_cfg_rx_napi_delete(void);
void edma_cfg_rx_napi_add(void);
void edma_cfg_rx_mapping(void);
void edma_cfg_rx_rings_enable(void);
void edma_cfg_rx_rings_disable(void);
void edma_cfg_rx_buff_size_setup(void);
int edma_cfg_rx_rps_hash_map(void);
int edma_cfg_rx_rps(const struct ctl_table *table, int write,
		    void *buffer, size_t *lenp, loff_t *ppos);
int edma_cfg_rx_rps_bitmap(const struct ctl_table *table, int write,
			   void *buffer, size_t *lenp, loff_t *ppos);
#endif
