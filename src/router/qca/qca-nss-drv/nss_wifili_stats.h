/*
 **************************************************************************
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
 **************************************************************************
 */

/*
 * nss_wifili_stats.h
 *	NSS wifili statistics header file.
 */

#ifndef __NSS_WIFILI_STATS_H
#define __NSS_WIFILI_STATS_H

#include "nss_core.h"
#include "nss_stats.h"
#include "nss_wifili_if.h"

/*
 * wifili txrx statistics
 *
 * WARNING: There is a 1:1 mapping between values below and corresponding
 *	stats string array in nss_stats.c
 */
enum nss_wifili_stats_txrx {
	NSS_WIFILI_STATS_RX_MSDU_ERROR,			/* Number of rx packets received from ring with msdu error */
	NSS_WIFILI_STATS_RX_INV_PEER_RCV,		/* Number of rx packets with invalid peer id */
	NSS_WIFILI_STATS_RX_WDS_SRCPORT_EXCEPTION,	/* Number of rx packets exceptioned to host because of src port learn fail */
	NSS_WIFILI_STATS_RX_WDS_SRCPORT_EXCEPTION_FAIL,	/* Number of rx src port learn fail packets failed to get enqueued to host */
	NSS_WIFILI_STATS_RX_DELIVERD,			/* Number of packets wifili has given to next node */
	NSS_WIFILI_STATS_RX_DELIVER_DROPPED,		/* Number of packets which wifili failed to enqueue to next node */
	NSS_WIFILI_STATS_RX_INTRA_BSS_UCAST,		/* Number of packets which wifili send for intra bss ucast packet */
	NSS_WIFILI_STATS_RX_INTRA_BSS_UCAST_FAIL,	/* Number of packets which wifili send for intra bss ucast packet failed */
	NSS_WIFILI_STATS_RX_INTRA_BSS_MCAST,		/* Number of packets which wifili send for intra bss mcast packet */
	NSS_WIFILI_STATS_RX_INTRA_BSS_MCAST_FAIL,	/* Number of packets which wifili send for intra bss mcast packet failed */
	NSS_WIFILI_STATS_RX_SG_RCV_SEND,		/* Number of packets sg send */
	NSS_WIFILI_STATS_RX_SG_RCV_FAIL,		/* Number of packets sg received failure*/
	NSS_STATS_WIFILI_RX_MCAST_ECHO,			/* Number of multicast echo packets received */
	NSS_STATS_WIFILI_RX_INV_TID,			/* Number of invalid tid */

	/*
	 * TODO: Move per tid based
	 */
	NSS_WIFILI_STATS_RX_FRAG_INV_SC,		/* Number of fragments with invalid sequence control */
	NSS_WIFILI_STATS_RX_FRAG_INV_FC,		/* Number of fragments with invalid frame control */
	NSS_WIFILI_STATS_RX_FRAG_NON_FRAG,		/* Number of non-fragments received in fragments */
	NSS_WIFILI_STATS_RX_FRAG_RETRY,			/* Number of retries for fragments */
	NSS_WIFILI_STATS_RX_FRAG_OOO,			/* Number of out of order fragments */
	NSS_WIFILI_STATS_RX_FRAG_OOO_SEQ,		/* Number of out of order sequence */
	NSS_WIFILI_STATS_RX_FRAG_ALL_FRAG_RCV,		/* Number of times all fragments for a sequence has been received */
	NSS_WIFILI_STATS_RX_FRAG_DELIVER,		/* Number of fragments delivered to host */
	NSS_WIFILI_STATS_TX_ENQUEUE,			/* Number of packets that got enqueued to wifili */
	NSS_WIFILI_STATS_TX_ENQUEUE_DROP,		/* Number of packets that dropped during enqueue to wifili */
	NSS_WIFILI_STATS_TX_DEQUEUE,			/* Number of packets that are dequeued by wifili */
	NSS_WIFILI_STATS_TX_HW_ENQUEUE_FAIL,		/* Number of rx packets that NSS wifi offload path could successfully process */
	NSS_WIFILI_STATS_TX_SENT_COUNT,			/* Number of Tx packets sent to hw */
	NSS_WIFILI_STATS_TXRX_MAX,			/* Number of max txrx stats*/
};

/*
 * wifili tcl stats
 *
 * WARNING: There is a 1:1 mapping between values below and corresponding
 *	stats string array in nss_stats.c
 */
enum nss_wifili_stats_tcl {
	NSS_WIFILI_STATS_TCL_NO_HW_DESC,		/* Number of tcl hw desc*/
	NSS_WIFILI_STATS_TCL_RING_FULL,			/* Number of times tcl ring full*/
	NSS_WIFILI_STATS_TCL_RING_SENT,			/* Number of times tcl desc sent*/
	NSS_WIFILI_STATS_TCL_MAX,			/* Number of max tcl stats*/
};

/*
 * wifili tx comp stats
 *
 * WARNING: There is a 1:1 mapping between values below and corresponding
 *  	stats string array in nss_stats.c
 */
enum nss_wifili_stats_tx_comp {
	NSS_WIFILI_STATS_TX_DESC_FREE_INV_BUFSRC,	/* Number of invalid bufsrc packets */
	NSS_WIFILI_STATS_TX_DESC_FREE_INV_COOKIE,	/* Number of invalid cookie packets */
	NSS_WIFILI_STATS_TX_DESC_FREE_HW_RING_EMPTY,	/* Number of time times hw ring empty found*/
	NSS_WIFILI_STATS_TX_DESC_FREE_REAPED,		/* Number of tx packets that are reaped out of tx completion ring */
	NSS_WIFILI_STATS_TX_DESC_FREE_MAX,		/* Number of tx comp stats */
};

/*
 * wifili tx reo stats
 *
 * WARNING: There is a 1:1 mapping between values below and corresponding
 *  	stats string array in nss_stats.c
 */
enum nss_wifili_stats_reo {
	NSS_WIFILI_STATS_REO_ERROR,			/* Number of reo error*/
	NSS_WIFILI_STATS_REO_REAPED,			/* Number of reo reaped*/
	NSS_WIFILI_STATS_REO_INV_COOKIE,		/* Number of invalid cookie*/
	NSS_WIFILI_STATS_REO_FRAG_RCV,			/* Number of fragmented received */
	NSS_WIFILI_STATS_REO_MAX,			/* Number of reo stats*/
};

/*
 * wifili tx desc stats
 *
 * WARNING: There is a 1:1 mapping between values below and corresponding
 *	stats string array in nss_stats.c
 */
enum nss_wifili_stats_txsw_pool {
	NSS_WIFILI_STATS_TX_DESC_IN_USE,		/* Number of tx packets that are currently in flight */
	NSS_WIFILI_STATS_TX_DESC_ALLOC_FAIL,		/* Number of tx sw desc alloc failures */
	NSS_WIFILI_STATS_TX_DESC_ALREADY_ALLOCATED,	/* Number of tx sw desc already allocated*/
	NSS_WIFILI_STATS_TX_DESC_INVALID_FREE,		/* Number of tx sw desc invalid free*/
	NSS_WIFILI_STATS_TX_DESC_FREE_SRC_FW,		/* Number of tx desc for which release src is fw */
	NSS_WIFILI_STATS_TX_DESC_FREE_COMPLETION,	/* Number of tx desc completion*/
	NSS_WIFILI_STATS_TX_DESC_NO_PB,			/* Number of tx desc pb is null*/
	NSS_WIFILI_STATS_TX_QUEUELIMIT_DROP,	/* Number of tx dropped because of queue limit */
	NSS_WIFILI_STATS_TX_DESC_MAX,			/* Number of tx desc stats*/
};

/*
 * wifili tx ext desc stats
 *
 * WARNING: There is a 1:1 mapping between values below and corresponding
 *	stats string array in nss_stats.c
 */
enum nss_wifili_stats_ext_txsw_pool {
	NSS_WIFILI_STATS_EXT_TX_DESC_IN_USE,		/* Number of ext tx packets that are currently in flight */
	NSS_WIFILI_STATS_EXT_TX_DESC_ALLOC_FAIL,	/* Number of ext tx sw desc alloc failures */
	NSS_WIFILI_STATS_EXT_TX_DESC_ALREADY_ALLOCATED,	/* Number of ext tx sw desc already allocated*/
	NSS_WIFILI_STATS_EXT_TX_DESC_INVALID_FREE,	/* Number of ext tx sw desc invalid free*/
	NSS_WIFILI_STATS_EXT_TX_DESC_MAX,		/* Number of ext tx desc stats*/
};

/*
 * wifili rx desc stats
 *
 * WARNING: There is a 1:1 mapping between values below and corresponding
 *	stats string array in nss_stats.c
 */
enum nss_wifili_stats_rxdma_pool {
	NSS_WIFILI_STATS_RX_DESC_NO_PB,			/* Number of rx desc no pb*/
	NSS_WIFILI_STATS_RX_DESC_ALLOC_FAIL,		/* Number of rx desc alloc failures */
	NSS_WIFILI_STATS_RX_DESC_IN_USE,		/* Number of rx desc alloc in use*/
	NSS_WIFILI_STATS_RX_DESC_MAX,			/* Number of rx desc stats*/
};

/*
 * wifili rx dma ring stats
 *
 * WARNING: There is a 1:1 mapping between values below and corresponding
 *	stats string array in nss_stats.c
 */
enum nss_wifili_stats_rxdma_ring {
	NSS_WIFILI_STATS_RXDMA_DESC_UNAVAILABLE, 	/* Number of rx dma desc unavailable */
	NSS_WIFILI_STATS_RXDMA_BUF_REPLENISHED,		/* Number of rx dma buf replished */
	NSS_WIFILI_STATS_RXDMA_DESC_MAX,	 	/* Number of rx dma desc stast*/
};

/*
 * wifili wbm ring stats
 *
 * WARNING: There is a 1:1 mapping between values below and corresponding
 *	stats string array in nss_stats.c
 */
enum nss_wifili_stats_wbm {
	NSS_WIFILI_STATS_WBM_SRC_DMA,			/* Number of rx invalid src dma*/
	NSS_WIFILI_STATS_WBM_SRC_DMA_CODE_INV,		/* Number of rx invalid src dma*/
	NSS_WIFILI_STATS_WBM_SRC_REO,			/* Number of rx invalid src reo*/
	NSS_WIFILI_STATS_WBM_SRC_REO_CODE_NULLQ,	/* Number of rx invalid reo error with null q*/
	NSS_WIFILI_STATS_WBM_SRC_REO_CODE_INV,		/* Number of rx invalid reo error with null q*/
	NSS_WIFILI_STATS_WBM_SRC_INV,			/* Number of rx invalid reo code invalid*/
	NSS_WIFILI_STATS_WBM_MAX,			/* Number of rx wbm stats*/
};

/*
 * NSS wifili stats
 */
struct nss_wifili_stats {
	uint64_t stats_txrx[NSS_WIFILI_MAX_PDEV_NUM_MSG][NSS_WIFILI_STATS_TXRX_MAX];
							/* Number of txrx stats*/
	uint64_t stats_tcl_ring[NSS_WIFILI_MAX_TCL_DATA_RINGS_MSG][NSS_WIFILI_STATS_TCL_MAX];
							/* Tcl stats for each ring*/
	uint64_t stats_tx_comp[NSS_WIFILI_MAX_TCL_DATA_RINGS_MSG][NSS_WIFILI_STATS_TX_DESC_FREE_MAX];
							/* Tx comp ring stats*/
	uint64_t stats_tx_desc[NSS_WIFILI_MAX_TXDESC_POOLS_MSG][NSS_WIFILI_STATS_TX_DESC_MAX];
							/* Tx desc pool stats*/
	uint64_t stats_ext_tx_desc[NSS_WIFILI_MAX_TX_EXT_DESC_POOLS_MSG][NSS_WIFILI_STATS_EXT_TX_DESC_MAX];
							/* Tx ext desc pool stats*/
	uint64_t stats_reo[NSS_WIFILI_MAX_REO_DATA_RINGS_MSG][NSS_WIFILI_STATS_REO_MAX];
							/* Rx  reo ring stats*/
	uint64_t stats_rx_desc[NSS_WIFILI_MAX_PDEV_NUM_MSG][NSS_WIFILI_STATS_RX_DESC_MAX];
							/* Rx  rx sw pool stats*/
	uint64_t stats_rxdma[NSS_WIFILI_MAX_PDEV_NUM_MSG][NSS_WIFILI_STATS_RXDMA_DESC_MAX];
							/* Rx  dma ring stats*/
	uint64_t stats_wbm[NSS_WIFILI_STATS_WBM_MAX];
							/* Wbm  error ring stats*/
};

/*
 * NSS wifili statistics APIs
 */
extern void nss_wifili_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_wifili_stats_sync_msg *wlsoc_stats, uint16_t interface);
extern void nss_wifili_stats_dentry_create(void);

#endif /* __NSS_WIFILI_STATS_H */
