/*
 ******************************************************************************
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * ****************************************************************************
 */

/*
 * nss_edma_stats.h
 *	NSS EDMA statistics header file.
 */

#ifndef __NSS_EDMA_STATS_H
#define __NSS_EDMA_STATS_H

#include "nss_core.h"

/*
 * Types of EDMA Tx ring stats
 */
enum nss_edma_stats_tx_t {
	NSS_EDMA_STATS_TX_ERR,
	NSS_EDMA_STATS_TX_DROPPED,
	NSS_EDMA_STATS_TX_DESC,
	NSS_EDMA_STATS_TX_MAX
};

/*
 * Types of EDMA Rx ring stats
 */
enum nss_edma_stats_rx_t {
	NSS_EDMA_STATS_RX_CSUM_ERR,
	NSS_EDMA_STATS_RX_DESC,
	NSS_EDMA_STATS_RX_QOS_ERR,
	NSS_EDMA_STATS_RX_MAX
};

/*
 * Types of EDMA Tx complete stats
 */
enum nss_edma_stats_txcmpl_t {
	NSS_EDMA_STATS_TXCMPL_DESC,
	NSS_EDMA_STATS_TXCMPL_MAX
};

/*
 * Types of EDMA Rx fill stats
 */
enum nss_edma_stats_rxfill_t {
	NSS_EDMA_STATS_RXFILL_DESC,
	NSS_EDMA_STATS_RXFILL_MAX
};

/*
 * Port to EDMA ring map
 */
enum nss_edma_port_ring_map_t {
	NSS_EDMA_PORT_RX_RING,
	NSS_EDMA_PORT_TX_RING,
	NSS_EDMA_PORT_RING_MAP_MAX
};

/*
 * Types of EDMA ERROR STATS
 */
enum nss_edma_err_t {
	NSS_EDMA_AXI_RD_ERR,
	NSS_EDMA_AXI_WR_ERR,
	NSS_EDMA_RX_DESC_FIFO_FULL_ERR,
	NSS_EDMA_RX_BUF_SIZE_ERR,
	NSS_EDMA_TX_SRAM_FULL_ERR,
	NSS_EDMA_TX_CMPL_BUF_FULL_ERR,
	NSS_EDMA_PKT_LEN_LA64K_ERR,
	NSS_EDMA_PKT_LEN_LE33_ERR,
	NSS_EDMA_DATA_LEN_ERR,
	NSS_EDMA_ALLOC_FAIL_CNT,
	NSS_EDMA_QOS_INVAL_DST_DROPS,
	NSS_EDMA_ERR_STATS_MAX
};

/*
 * NSS EDMA port stats
 */
struct nss_edma_port_info {
	uint64_t port_stats[NSS_STATS_NODE_MAX];
	uint64_t port_type;
	uint64_t port_ring_map[NSS_EDMA_PORT_RING_MAP_MAX];
};

/*
 * NSS EDMA node statistics
 */
struct nss_edma_stats {
	struct nss_edma_port_info port[NSS_EDMA_NUM_PORTS_MAX];
	uint64_t tx_stats[NSS_EDMA_NUM_TX_RING_MAX][NSS_EDMA_STATS_TX_MAX];
	uint64_t rx_stats[NSS_EDMA_NUM_RX_RING_MAX][NSS_EDMA_STATS_RX_MAX];
	uint64_t txcmpl_stats[NSS_EDMA_NUM_TXCMPL_RING_MAX][NSS_EDMA_STATS_TXCMPL_MAX];
	uint64_t rxfill_stats[NSS_EDMA_NUM_RXFILL_RING_MAX][NSS_EDMA_STATS_RXFILL_MAX];
	uint64_t misc_err[NSS_EDMA_ERR_STATS_MAX];
};

/*
 * NSS EDMA statistics APIs
 */
extern void nss_edma_metadata_port_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_edma_port_stats_sync *nepss);
extern void nss_edma_metadata_ring_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_edma_ring_stats_sync *nerss);
extern void nss_edma_metadata_err_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_edma_err_stats_sync *nerss);
extern void nss_edma_stats_dentry_create(void);

#endif /* __NSS_EDMA_STATS_H */
