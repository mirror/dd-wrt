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

#ifndef __PPE_DS__
#define __PPE_DS__
#include <linux/netdevice.h>

#include "nss_dp_ppeds.h"
#include "ppe_ds_wlan.h"

#define PPE_DS_POLL_MODE	1
#define PPE_DS_INTR_MODE	0

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * If dynamic debug is enabled, use pr_debug.
 */
#define ppe_ds_err(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_ds_warn(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_ds_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_ds_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels, when dynamic debug is disabled.
 */
#if (PPE_DS_DEBUG_LEVEL < 1)
#define ppe_ds_err(s, ...)
#else
#define ppe_ds_err(s, ...) pr_err("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_DS_DEBUG_LEVEL < 2)
#define ppe_ds_warn(s, ...)
#else
#define ppe_ds_warn(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_DS_DEBUG_LEVEL < 3)
#define ppe_ds_info(s, ...)
#else
#define ppe_ds_info(s, ...) pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_DS_DEBUG_LEVEL < 4)
#define ppe_ds_trace(s, ...)
#else
#define ppe_ds_trace(s, ...) pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif

#define PPE_DS_INTR_ENABLE	0	/* Flag for PPE-DS mode.
					   0 means timer mode and 1 means interrupt mode */
#define PPE_DS_MAX_NODE		4	/* Max DS node supported */
#define PPE_DS_RXFILL_LOW_THRES_DIVISOR	3	/* PPE-DS node's Rxfill low threshold divisor */
#define PPE_DS_RXFILL_NUM_DESC_MAX	65535	/* PPE-DS node's Rxfill maximum descriptor count */
#define PPE_DS_RXFILL_NUM_DESC_MIN	1024	/* PPE-DS node's Rxfill minimum descriptor count */
#define PPE_DS_RXFILL_NUM_DESC_DEF	2048	/* PPE-DS node's Rxfill default descriptor count */
#define PPE_DS_TXCMPL_NUM_DESC_MAX	65535	/* PPE-DS node's Txcmpl maximum descriptor count */
#define PPE_DS_TXCMPL_NUM_DESC_MIN	1024	/* PPE-DS node's Txcmpl minimum descriptor count */
#define PPE_DS_TXCMPL_NUM_DESC_DEF	8192	/* PPE-DS node's Txcmpl default descriptor count */
#define PPE_DS_TXCMPL_MIN_BUDGET	16	/* PPE-DS node's Txcmpl minimum budget */
#define PPE_DS_TXCMPL_DEF_BUDGET	256	/* PPE-DS node's Txcmpl default budget */

extern unsigned int polling_for_idx_update;
extern unsigned int idx_mgmt_freq;
extern unsigned int cpu_mask_2g;
extern unsigned int cpu_mask_5g_lo;
extern unsigned int cpu_mask_5g_hi;
extern unsigned int cpu_mask_6g;
extern unsigned int ppe2tcl_rxfill_num_desc;
extern unsigned int reo2ppe_txcmpl_num_desc;
extern unsigned int rxfill_low_threshold;
extern unsigned int txcmpl_budget;

/*
 * ppe_ds_node_state_t
 *	PPE-DS node states
 */
typedef enum {
	PPE_DS_NODE_STATE_NOT_AVAIL,
	PPE_DS_NODE_STATE_AVAIL,
	PPE_DS_NODE_STATE_ALLOC,
	PPE_DS_NODE_STATE_REG_IN_PROG,
	PPE_DS_NODE_STATE_REG_DONE,
	PPE_DS_NODE_STATE_START_IN_PROG,
	PPE_DS_NODE_STATE_START_DONE,
	PPE_DS_NODE_STATE_STOP_IN_PROG,
	PPE_DS_NODE_STATE_STOP_DONE,
	PPE_DS_NODE_STATE_FREE_IN_PROG,
} ppe_ds_node_state_t;

/*
 * ppe_ds_node_config
 *	PPE-DS node configuration
 */
struct ppe_ds_node_config {
	ppe_ds_node_state_t node_state;		/* PPE-DS node state information */
	rwlock_t lock;				/* Lock for accessing node configuration */
};

/*
 * ppe_ds
 *	PPE-DS node information
 */
struct ppe_ds {
	struct napi_struct napi;		/* NAPI structure */
	struct hrtimer timer;			/* HR timer */
	struct net_device napi_ndev;		/* Dummy NAPI device */
	bool timer_enabled;			/* Timer enabled flag */
	bool en_process_irq;			/* Safe to handle irq */
	struct ppe_ds_wlan_ops *wlan_ops;	/* PPE-DS WLAN operations */
	struct nss_dp_ppeds_ops *dp_ops;	/* PPE-DS EDMA operations */
	uint32_t node_cfg_idx;			/* Index of PPE-DS node configuration */
	uint16_t last_edma_rx_cons_idx;		/* Last read EDMA Rx consumer index */
	uint16_t last_reo2ppe_cons_idx;		/* Last read WLAN REO2PPE consumer index */
	uint16_t last_edma_tx_prod_idx;		/* Last read EDMA Tx producer index */
	uint16_t umac_reset_inprogress;		/* Umac reset in progress information */
	nss_dp_ppeds_handle_t *edma_handle;	/* EDMA handle */
	ppe_ds_wlan_handle_t wlan_handle;	/* WLAN handle */
};

extern struct ppe_ds_node_config ppe_ds_node_cfg[PPE_DS_MAX_NODE];
#endif	/* __PPE_DS__ */
