/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 **************************************************************************
 */

/*
 * nss_ppe_vp.h
 *      NSS PPE virtual port header file
 */

#include <net/sock.h>
#include "nss_tx_rx_common.h"

/*
 * Maximum number of virtual port supported by PPE hardware
 */
#define NSS_PPE_VP_MAX_NUM 192
#define NSS_PPE_VP_START 64
#define NSS_PPE_VP_NODE_STATS_MAX 32


/*
 * ppe_vp nss debug stats lock
 */
extern spinlock_t nss_ppe_vp_stats_lock;

/*
 * nss_ppe_vp_msg_error_type
 *	ppe_vp message errors
 */
enum nss_ppe_vp_msg_error_type {
	NSS_PPE_VP_MSG_ERROR_TYPE_UNKNOWN,	/* Unknown message error */
	NSS_PPE_VP_MSG_ERROR_TYPE_INVALID_DI,	/* Invalid dynamic interface type error */
	NSS_PPE_VP_MSG_ERROR_TYPE_MAX		/* Maximum error type */
};

/*
 * nss_ppe_vp_message_types
 *	Message types for Packet Processing Engine (PPE) requests and responses.
 */
enum nss_ppe_vp_message_types {
	NSS_PPE_VP_MSG_CONFIG,
	NSS_PPE_VP_MSG_SYNC_STATS,
	NSS_PPE_VP_MSG_MAX,
};

/*
 * nss_ppe_vp_config_msg
 *	Message to enable/disable VP support for a specific dynamic interface type.
 */
struct nss_ppe_vp_config_msg {
	enum nss_dynamic_interface_type type;	/* Interface type */
	bool vp_enable;				/* VP support enable */
};

/*
 * nss_ppe_vp_statistics
 *	Message structure for ppe_vp statistics
 */
struct nss_ppe_vp_statistics {
	uint32_t nss_if;			/* NSS interface number corresponding to VP */
	uint32_t vp_num;			/* VP number */
	uint32_t rx_drop;			/* Rx drops due to VP node inactive */
	uint32_t tx_drop;			/* Tx drops due to VP node inactive */
	uint32_t packet_big_err;		/* Number of packets not sent to PPE because packet was too large */
	struct nss_cmn_node_stats stats;	/* Common node statistics */
};

/*
 * nss_ppe_vp_sync_stats_msg
 *	Message information for ppe_vp synchronization statistics.
 */
struct nss_ppe_vp_sync_stats_msg {
	uint16_t count;				/* Number of VP node stats with the sync message */
	uint32_t rx_dropped[NSS_MAX_NUM_PRI];	/* Rx packet dropped due to queue full */
	struct nss_ppe_vp_statistics vp_stats[NSS_PPE_VP_NODE_STATS_MAX];
						/* Per service-code stats */
};

/*
 * nss_ppe_vp_msg
 *	Message for receiving ppe_vp NSS to host messages.
 */
struct nss_ppe_vp_msg {
	struct nss_cmn_msg cm;		/**< Common message header. */

	/*
	 * Payload.
	 */
	union {
		struct nss_ppe_vp_config_msg vp_config;
				/**< Enable/disable VP support for specific type */
		struct nss_ppe_vp_sync_stats_msg stats;
				/**< Synchronization statistics. */
	} msg;			/**< Message payload. */
};

typedef void (*nss_ppe_vp_msg_callback_t)(void *app_data, struct nss_ppe_vp_msg *msg);

/*
 * Logging APIs.
 */
void nss_ppe_vp_log_tx_msg(struct nss_ppe_vp_msg *npvm);
void nss_ppe_vp_log_rx_msg(struct nss_ppe_vp_msg *npvm);
