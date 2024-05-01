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
 * @file nss_trustsec_rx.h
 *	NSS TrustSec interface definitions.
 */

#ifndef __NSS_TRUSTSEC_RX_H
#define __NSS_TRUSTSEC_RX_H

#define NSS_TRUSTSEC_RX_FLAG_IPV4 4         /**< L3 Protocol - IPv4 */
#define NSS_TRUSTSEC_RX_FLAG_IPV6 6         /**< L3 Protocol - IPv6 */

/**
 * @addtogroup nss_trustsec_rx_subsystem
 * @{
 */

/**
 * nss_trustsec_rx_msg_types
 *	Message types for TrustSec Tx requests and responses.
 */
enum nss_trustsec_rx_msg_types {
	NSS_TRUSTSEC_RX_MSG_CONFIGURE,		/**< Configure the TrustSec node. */
	NSS_TRUSTSEC_RX_MSG_UNCONFIGURE,	/**< Unconfigure the TrustSec node. */
	NSS_TRUSTSEC_RX_MSG_STATS_SYNC,		/**< Statistics sychronization. */
	NSS_TRUSTSEC_RX_MSG_CONFIG_VP,		/**< Configure the virtual port number. */
	NSS_TRUSTSEC_RX_MSG_UNCONFIG_VP,	/**< Unconfigure the virtual port number. */
	NSS_TRUSTSEC_RX_MSG_MAX			/**< Maximum message type. */
};

/**
 * nss_trustsec_rx_error_types
 *	Error types for the TrustSec Tx interface.
 */
enum nss_trustsec_rx_error_types {
	NSS_TRUSTSEC_RX_ERR_NONE,		/**< No error. */
	NSS_TRUSTSEC_RX_ERR_DEST_IF_NOT_FOUND,	/**< Destination interface is not found. */
	NSS_TRUSTSEC_RX_ERR_INCORRECT_IP,	/**< IP version is incorrect. */
	NSS_TRUSTSEC_RX_ERR_ENTRY_EXIST,	/**< Entry already exists. */
	NSS_TRUSTSEC_RX_ERR_ENTRY_ADD_FAILED,	/**< IP rule cannot be added. */
	NSS_TRUSTSEC_RX_ERR_ENTRY_NOT_FOUND,	/**< Entry cannot be found. */
	NSS_TRUSTSEC_RX_ERR_NOT_CONFIGURED,	/**< Source interface is not configured. */
	NSS_TRUSTSEC_RX_ERR_UNKNOWN,		/**< Unknown TrustSec message. */
	NSS_TRUSTSEC_RX_ERR_CONFIG_VP_NUM,	/**< Failed to configure TrustSec virtual port number. */
	NSS_TRUSTSEC_RX_ERR_UNCONFIG_VP_NUM,	/**< Failed to unconfigure TrustSec virtual port number. */
	NSS_TRUSTSEC_RX_ERR_MAX			/**< Maximum error type. */
};

/**
 * trustsec_rx_vp
 *	Message information to handle the TrustSec virtual port.
 */
struct nss_trustsec_rx_vp_msg {
	int16_t num;	/**< Virtual port number. */
};

/*
 * nss_trustsec_rx_ip
 *	trustsec_rx IP structure.
 */
struct nss_trustsec_rx_ip {
	union {
		uint32_t ipv4;		/**< IPv4 address. */
		uint32_t ipv6[4];	/**< IPv6 address. */
	} ip;
};

/**
 * nss_trustsec_rx_configure_msg
 *	Message information for configuring a TrustSec Rx interface.
 */
struct nss_trustsec_rx_configure_msg {
	struct nss_trustsec_rx_ip src_ip;	/**< Source IP address. */
	int32_t src_port;			/**< Source L4 port. */
	struct nss_trustsec_rx_ip dest_ip;	/**< Destination IP address. */
	int32_t dest_port;			/**< Destination L4 port. */
	uint16_t ip_version;			/**< Version of the IP address. */
	uint32_t dest;				/**< Destination interface number. */
};

/**
 * nss_trustsec_rx_unconfigure_msg
 *	Message information for unconfiguring a TrustSec Rx interface.
 */
struct nss_trustsec_rx_unconfigure_msg {
	struct nss_trustsec_rx_ip src_ip;	/**< Source IP address. */
	int32_t src_port;			/**< Source L4 port. */
	struct nss_trustsec_rx_ip dest_ip;	/**< Destination IP address. */
	int32_t dest_port;			/**< Destination L4 port. */
	uint16_t ip_version;			/**< Version of the IP address. */
	uint32_t dest;				/**< Destination interface number. */
};

/**
 * nss_trustsec_rx_stats_sync_msg
 *	Statistics synchronization message for the TrustSec Rx interface.
 */
struct nss_trustsec_rx_stats_sync_msg {
	struct nss_cmn_node_stats node_stats;	/**< Common node statistics. */
	uint32_t unknown_eth_type;	/**< Unknown packet type. */
	uint32_t unknown_pkt;		/**< Unknown IP packet. */
	uint32_t unknown_dest;		/**< Unknown destination. */
	uint32_t ip_parse_failed;	/**< IP parse is failed. */
	uint32_t wrong_l4_type;		/**< L4 protocol is wrong. */
};

/**
 * nss_trustsec_rx_msg
 *	Data for sending and receiving TrustSec Tx messages.
 */
struct nss_trustsec_rx_msg {
	struct nss_cmn_msg cm;		/**< Common message header. */

	/**
	 * Payload of a TrustSec Tx message.
	 */
	union {
		struct nss_trustsec_rx_configure_msg configure;
				/**< Configure TrustSec Tx. */
		struct nss_trustsec_rx_unconfigure_msg unconfigure;
				/**< De-configure TrustSec Tx. */
		struct nss_trustsec_rx_stats_sync_msg stats_sync;
				/**< Synchronize TrustSec Tx statistics. */
		struct nss_trustsec_rx_vp_msg cfg;
				/**< Configure the TrustSec virtual port number. */
		struct nss_trustsec_rx_vp_msg uncfg;
				/**< Unconfigure the TrustSec virtual port number. */
	} msg;			/**< Message payload. */
};

/**
 * Callback function for receiving TrustSec Tx messages.
 *
 * @datatypes
 * nss_trustsec_rx_msg
 *
 * @param[in] app_data  Pointer to the application context of the message.
 * @param[in] msg       Pointer to the message data.
 */
typedef void (*nss_trustsec_rx_msg_callback_t)(void *app_data, struct nss_trustsec_rx_msg *npm);

/**
 * nss_trustsec_rx_msg_init
 *	Initializes a TrustSec Tx message.
 *
 * @datatypes
 * nss_trustsec_rx_msg
 *
 * @param[in,out] npm       Pointer to the NSS Profiler message.
 * @param[in]     if_num    NSS interface number.
 * @param[in]     type      Type of message.
 * @param[in]     len       Size of the message.
 * @param[in]     cb        Callback function for the message.
 * @param[in]     app_data  Pointer to the application context of the message.
 *
 * @return
 * TRUE or FALSE.
 */
extern void nss_trustsec_rx_msg_init(struct nss_trustsec_rx_msg *msg, uint16_t if_num, uint32_t type, uint32_t len,
							nss_trustsec_rx_msg_callback_t cb, void *app_data);

/**
 * nss_trustsec_rx_msg_sync
 *	Sends a TrustSec Tx message to the NSS and waits for a response.
 *
 * @datatypes
 * nss_ctx_instance \n
 * nss_trustsec_rx_msg
 *
 * @param[in] nss_ctx  Pointer to the NSS context.
 * @param[in] msg      Pointer to the message data.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_tx_status_t nss_trustsec_rx_msg_sync(struct nss_ctx_instance *nss_ctx, struct nss_trustsec_rx_msg *msg);

/**
 * nss_trustsec_rx_get_ctx
 *	Gets the NSS context.
 *
 * @return
 * Pointer to the NSS core context.
 */
extern struct nss_ctx_instance *nss_trustsec_rx_get_ctx(void);

/** @} */ /* end_addtogroup nss_trustsec_rx_subsystem */

#endif /* __NSS_TRUSTSEC_RX_H */
