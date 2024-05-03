/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @file nss_edma_lite.h
 *	NSS core-to-core transmission interface definitions.
 */

#ifndef __NSS_EDMA_LITE_H
#define __NSS_EDMA_LITE_H

/**
 * @addtogroup nss_edma_lite_subsystem
 * @{
 */

/**
 * nss_edma_lite_msg_type
 *	Supported message types.
 */
enum nss_edma_lite_msg_type {
	NSS_EDMA_LITE_MSG_TYPE_RING_MAP,		/**< Ring numbers. */
	NSS_EDMA_LITE_MSG_NODE_STATS_SYNC,		/**< Node statistics synchronization. */
	NSS_EDMA_LITE_MSG_RING_STATS_SYNC,		/**< Ring statistics synchronization. */
	NSS_EDMA_LITE_MSG_ERR_STATS_SYNC,		/**< Ring error statistics synchronization. */
	NSS_EDMA_LITE_MSG_TYPE_MAX			/**< Maximum message type. */
};

/**
 * nss_edma_lite_msg_error
 *	Message error types.
 */
enum nss_edma_lite_msg_error {
	NSS_EDMA_LITE_MSG_ERROR_NONE,		/**< No error. */
	NSS_EDMA_LITE_MSG_ERROR_INVAL_OP,	/**< Invalid operation. */
	NSS_EDMA_LITE_MSG_ERROR_MAX		/**< Maximum error type. */
};

/**
 * nss_edma_stats_tx_t
 *	Types of EDMA Tx ring statistics.
 */
enum nss_edma_lite_stats_tx_t {
	NSS_EDMA_LITE_STATS_TX_ERR,		/**< Transmit error statistics. */
	NSS_EDMA_LITE_STATS_TX_DROPPED,		/**< Transmit dropped statistics. */
	NSS_EDMA_LITE_STATS_TX_DESC,		/**< Trnasmit descriptor statistics. */
	NSS_EDMA_LITE_STATS_TX_MAX		/**< Maximum transmit statistics. */
};

/**
 * nss_edma_lite_stats_rx_t
 *	Types of EDMA Rx ring statistics.
 */
enum nss_edma_lite_stats_rx_t {
	NSS_EDMA_LITE_STATS_RX_DESC,			/**< Transmit completion statistics. */
	NSS_EDMA_LITE_STATS_RX_MAX			/**< Transmit completion statistics. */
};

/**
 * nss_edma_lite_stats_txcmpl_t
 *	Types of EDMA Tx complete statistics.
 */
enum nss_edma_lite_stats_txcmpl_t {
	NSS_EDMA_LITE_STATS_TXCMPL_DESC,	/**< Transmit completion statistics. */
	NSS_EDMA_LITE_STATS_TXCMPL_MAX		/**< Maximum transmit completion statistics. */
};

/**
 * nss_edma_lite_stats_rxfill_t
 *	Types of EDMA Rx fill statistics.
 */
enum nss_edma_lite_stats_rxfill_t {
	NSS_EDMA_LITE_STATS_RXFILL_DESC,	/**< Receive fill descriptor statistics. */
	NSS_EDMA_LITE_STATS_RXFILL_MAX		/**< Maximum receive fill statistics. */
};

/**
 * nss_edma_lite_err_t
 *	Types of EDMA error statistics.
 */
enum nss_edma_lite_err_t {
	NSS_EDMA_LITE_ALLOC_FAIL_CNT,		/**< EDMA allocation fail count statistics. */
	NSS_EDMA_LITE_UNKNOWN_PKT_CNT,		/**< EDMA unknown packet count statistics. */
	NSS_EDMA_LITE_ERR_STATS_MAX		/**< EDMA error statistics. */
};

/**
 * nss_edma_lite_rx_ring_stats
 *	EDMA Rx ring statistics.
 */
struct nss_edma_lite_rx_ring_stats {
	uint32_t desc_cnt;		/**< Number of descriptors processed. */
};

/**
 * nss_edma_lite_tx_ring_stats
 *	EDMA Tx ring statistics.
 */
struct nss_edma_lite_tx_ring_stats {
	uint32_t tx_err;		/**< Number of Tx errors. */
	uint32_t tx_dropped;		/**< Number of Tx dropped packets. */
	uint32_t desc_cnt;		/**< Number of descriptors processed. */
};

/**
 * nss_edma_lite_rxfill_ring_stats
 *	EDMA Rx fill ring statistics.
 */
struct nss_edma_lite_rxfill_ring_stats {
	uint32_t desc_cnt;		/**< Number of descriptors processed. */
};

/**
 * nss_edma_lite_txcmpl_ring_stats
 *	EDMA Tx complete ring statistics.
 */
struct nss_edma_lite_txcmpl_ring_stats {
	uint32_t desc_cnt;		/**< Number of descriptors processed. */
};

/**
 * nss_edma_lite_node_stats_sync
 *	EDMA node statistics.
 */
struct nss_edma_lite_node_stats_sync {
	struct nss_cmn_node_stats node_stats;	/**< Common node statistics. */
};

/**
 * nss_edma_lite_ring_stats_sync
 *	EDMA ring statistics.
 */
struct nss_edma_lite_ring_stats_sync {
	struct nss_edma_lite_tx_ring_stats tx_ring;
			/**< EDMA Tx ring statistics. */
	struct nss_edma_lite_rx_ring_stats rx_ring;
			/**< EDMA Rx ring statistics. */
	struct nss_edma_lite_txcmpl_ring_stats txcmpl_ring;
			/**< EDMA Tx complete ring statistics. */
	struct nss_edma_lite_rxfill_ring_stats rxfill_ring;
			/**< EDMA Rx fill ring statistics. */
};

/**
 * nss_edma_lite_err_stats_sync
 *	Message for error statistics.
 */
struct nss_edma_lite_err_stats_sync {
	uint32_t alloc_fail_cnt;	/**< EDMA number of times the allocation of pbuf for statistics failed. */
	uint32_t unknown_pkt_cnt;	/**< Number of times the packet with non-virtual port source or destination received. */
};

/**
 * nss_edma_lite_ring_map
 *	Ring number.
 */
struct nss_edma_lite_ring_map {
	uint32_t txdesc_num;		/**< Transmit queue start address. */
	uint32_t txcmpl_num;		/**< Transmit queue completion start address. */
	uint32_t rxdesc_num;		/**< Receive queue start address. */
	uint32_t rxfill_num;		/**< Receive fill queue start address. */
};

/**
 * nss_edma_lite_msg
 *	Message structure to send/receive core-to-core transmission commands.
 */
struct nss_edma_lite_msg {
	struct nss_cmn_msg cm;			/**< Common message header. */

	/**
	 * Payload of a NSS core-to-core transmission rule or statistics message.
	 */
	union {
		struct nss_edma_lite_ring_map map;
			/**< EDMA rings memory map. */
		struct nss_edma_lite_node_stats_sync node_stats;
			/**< EDMA node statistics synchronization. */
		struct nss_edma_lite_ring_stats_sync ring_stats;
			/**< EDMA rings statistics synchronization. */
		struct nss_edma_lite_err_stats_sync err_stats;
			/**< EDMA rings error statistics synchronization. */

	} msg;	/**< Message payload. */
};

/**
 * nss_edma_lite_stats
 *	NSS EDMA LITE node statistics.
 */
struct nss_edma_lite_stats {
	uint64_t node_stats[NSS_STATS_NODE_MAX];
				/**< Common node statistics. */
	uint64_t tx_stats[NSS_EDMA_LITE_STATS_TX_MAX];
				/**< Physical EDMA Tx ring statistics. */
	uint64_t rx_stats[NSS_EDMA_LITE_STATS_RX_MAX];
				/**< Physical EDMA Rx ring statistics. */
	uint64_t txcmpl_stats[NSS_EDMA_LITE_STATS_TXCMPL_MAX];
				/**< Physical EDMA Tx complete statistics. */
	uint64_t rxfill_stats[NSS_EDMA_LITE_STATS_RXFILL_MAX];
				/**< Physical EDMA Rx fill statistics. */
	uint64_t err[NSS_EDMA_LITE_ERR_STATS_MAX];
				/**< EDMA error complete statistics. */
};

#ifdef __KERNEL__ /* only kernel will use. */

/**
 * nss_edma_lite_get_context
 *	Get EDMA Lite context.
 *
 * @datatypes
 * nss_ctx_instance
 *
 * @param[in] nss_ctx  Pointer to nss context.
 *
 * @return
 * true or false.
 */
bool nss_edma_lite_enabled(struct nss_ctx_instance *nss_ctx);

/**
 * nss_edma_lite_register_handler
 *	Registers the EDMA lite message handler.
 *
 * @datatypes
 * nss_ctx_instance
 *
 * @param[in] nss_ctx  Pointer to the NSS context.
 *
 * @return
 * None.
 */
void nss_edma_lite_register_handler(struct nss_ctx_instance *nss_ctx);

/**
 * Callback function for receiving core-to-core transmission messages.
 *
 * @datatypes
 * nss_edma_lite_msg
 *
 * @param[in] app_data  Pointer to the application context of the message.
 * @param[in] msg       Pointer to the message data.
 */
typedef void (*nss_edma_lite_msg_callback_t)(void *app_data, struct nss_edma_lite_msg *msg);

/**
 * nss_edma_lite_tx_msg
 *	Transmits a core-to-core transmission message to the NSS.
 *
 * @datatypes
 * nss_ctx_instance \n
 * nss_edma_lite_msg
 *
 * @param[in] nss_ctx   Pointer to the NSS context.
 * @param[in] nctm      Pointer to the message data.
 *
 * @return
 * Status of the transmit operation.
 */
extern nss_tx_status_t nss_edma_lite_tx_msg(struct nss_ctx_instance *nss_ctx, struct nss_edma_lite_msg *nelm);

/**
 * nss_edma_lite_msg_init
 *	Initializes core-to-core transmission messages.
 *
 * @datatypes
 * nss_edma_lite_msg \n
 * nss_edma_lite_msg_callback_t
 *
 * @param[in]     nct       Pointer to the NSS interface message.
 * @param[in]     if_num    NSS interface number.
 * @param[in]     type      Type of message.
 * @param[in]     len       Size of the payload.
 * @param[in]     cb        Callback function for the message.
 * @param[in]     app_data  Pointer to the application context of the message.
 *
 * @return
 * None.
 */
extern void nss_edma_lite_msg_init(struct nss_edma_lite_msg *nct, uint16_t if_num, uint32_t type, uint32_t len,
			nss_edma_lite_msg_callback_t cb, void *app_data);

/**
 * nss_edma_lite_notify_register
 *	Registers a notifier callback for core-to-core transmission messages with the NSS.
 *
 * @datatypes
 * nss_edma_lite_msg_callback_t
 *
 * @param[in] cb        Callback function for the message.
 * @param[in] app_data  Pointer to the application context of the message.
 *
 * @return
 * None.
 */
void nss_edma_lite_notify_register(nss_edma_lite_msg_callback_t cb, void *app_data);

/**
 * nss_edma_lite_notify_unregister
 *	Deregisters a core-to-core transmission message notifier callback from the NSS.
 *
 * @param[in] core NSS core number index to the notifier callback table.
 *
 * @return
 * None.
 *
 * @dependencies
 * The notifier callback must have been previously registered.
 */
void nss_edma_lite_notify_unregister(void);

/**
 * nss_edma_lite_msg_cfg_map
 *	Sends core-to-core transmissions map to NSS
 *
 * @datatypes
 * nss_ctx_instance \n
 *
 * @param[in] nss_ctx   Pointer to the NSS context.
 *
 * @return
 * Status of the transmit operation.
 */
extern nss_tx_status_t nss_edma_lite_msg_cfg_map(struct nss_ctx_instance *nss_ctx);

/**
 * nss_edma_lite_stats_register_notifier
 *	Registers a statistics notifier.
 *
 * @datatypes
 * notifier_block
 *
 * @param[in] nb Notifier block.
 *
 * @return
 * 0 on success or -2 on failure.
 */
extern int nss_edma_lite_stats_register_notifier(struct notifier_block *nb);

/**
 * nss_edma_lite_is_configured
 *	check if map message is configured.
 *
 * @return
 * true if configured
 */
extern bool nss_edma_lite_is_configured(void);
#endif /*__KERNEL__ */

/**
 * @}
 */

#endif /* __NSS_EDMA_LITE_H */
