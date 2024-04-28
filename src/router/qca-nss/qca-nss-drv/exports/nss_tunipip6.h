/*
 **************************************************************************
 * Copyright (c) 2014, 2017-2018, The Linux Foundation. All rights reserved.
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

/**
 * @file nss_tunipip6.h
 *	NSS TUNIPIP6 interface definitions.
 */

#ifndef __NSS_TUNIPIP6_H
#define __NSS_TUNIPIP6_H

/**
 * @addtogroup nss_tunipip6_subsystem
 * @{
 */

#define NSS_TUNIPIP6_MAX_FMR_NUMBER 4	/**< Maximum number of forward mapping rule (FMR). */

/**
 * nss_tunipip6_fmr
 *	Forward mapping rule (FMR) for direct forwarding traffic to the node in the same domain.
 */
struct nss_tunipip6_fmr {
	uint32_t ip6_prefix[4];		/**< An IPv6 prefix assigned by a mapping rule. */
	uint32_t ip4_prefix;		/**< An IPv4 prefix assigned by a mapping rule. */
	uint32_t ip6_prefix_len;	/**< IPv6 prefix length. */
	uint32_t ip4_prefix_len;	/**< IPv4 prefix length. */
	uint32_t ea_len;		/**< Embedded Address (EA) bits. */
	uint32_t offset;		/**< PSID offset default 6. */
};

/**
 * nss_tunipip6_metadata_types
 *	Message types for DS-Lite (IPv4 in IPv6) tunnel requests and responses.
 */
enum nss_tunipip6_metadata_types {
	NSS_TUNIPIP6_TX_ENCAP_IF_CREATE,
	NSS_TUNIPIP6_TX_DECAP_IF_CREATE,
	NSS_TUNIPIP6_RX_STATS_SYNC,
	NSS_TUNIPIP6_MAX,
};

/**
 * nss_tunipip6_create_msg
 *	Payload for configuring the DS-Lite interface.
 */
struct nss_tunipip6_create_msg {
	struct nss_tunipip6_fmr fmr[NSS_TUNIPIP6_MAX_FMR_NUMBER];	/**< Tunnel FMR array. */
	uint32_t saddr[4];						/**< Tunnel source address. */
	uint32_t daddr[4];						/**< Tunnel destination address. */
	uint32_t flowlabel;						/**< Tunnel IPv6 flow label. */
	uint32_t flags;							/**< Tunnel additional flags. */
	uint32_t fmr_number;						/**< Tunnel FMR number. */
	uint32_t sibling_if_num;					/**< Sibling interface number. */
	uint16_t reserved1;						/**< Reserved for alignment. */
	uint8_t hop_limit;						/**< Tunnel IPv6 hop limit. */
	uint8_t draft03;						/**< Use MAP-E draft03 specification. */
};

/**
 * nss_tunipip6_stats_sync_msg
 *	Message information for DS-Lite synchronization statistics.
 */
struct nss_tunipip6_stats_sync_msg {
	struct nss_cmn_node_stats node_stats;	/**< Common node statistics. */
};

/**
 * nss_tunipip6_msg
 *	Data for sending and receiving DS-Lite messages.
 */
struct nss_tunipip6_msg {
	struct nss_cmn_msg cm;			/**< Common message header. */

	/**
	 * Payload of a DS-Lite message.
	 */
	union {
		struct nss_tunipip6_create_msg tunipip6_create;
				/**< Create a DS-Lite tunnel. */
		struct nss_tunipip6_stats_sync_msg stats_sync;
				/**< Synchronized statistics for the DS-Lite interface. */
	} msg;			/**< Message payload for IPIP6 Tunnel messages exchanged with NSS core. */
};

/**
 * Callback function for receiving DS-Lite messages.
 *
 * @datatypes
 * nss_tunipip6_msg
 *
 * @param[in] app_data  Pointer to the application context of the message.
 * @param[in] msg       Pointer to the message data.
 */
typedef void (*nss_tunipip6_msg_callback_t)(void *app_data, struct nss_tunipip6_msg *msg);

/**
 * nss_tunipip6_tx
 *	Sends a DS-Lite message to NSS core.
 *
 * @datatypes
 * nss_ctx_instance \n
 * nss_tunipip6_msg
 *
 * @param[in] nss_ctx  Pointer to the NSS context.
 * @param[in] msg      Pointer to the message data.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_tx_status_t nss_tunipip6_tx(struct nss_ctx_instance *nss_ctx, struct nss_tunipip6_msg *msg);

/**
 * Callback function for receiving DS-Lite data.
 *
 * @datatypes
 * net_device \n
 * sk_buff \n
 * napi_struct
 *
 * @param[in] netdev  Pointer to the associated network device.
 * @param[in] skb     Pointer to the data socket buffer.
 * @param[in] napi    Pointer to the NAPI structure.
 */
typedef void (*nss_tunipip6_callback_t)(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi);

/*
 * nss_register_tunipip6_if
 *	Registers the TUNIPIP6 interface with the NSS for sending and receiving
 *	DS-Lite messages.
 *
 * @datatypes
 * nss_tunipip6_callback_t \n
 * nss_tunipip6_msg_callback_t \n
 * net_device
 *
 * @param[in] if_num             NSS interface number.
 * @param[in] type               Dynamic interface type.
 * @param[in] tunipip6_callback  Callback for the data.
 * @param[in] event_callback     Callback for the message.
 * @param[in] netdev             Pointer to the associated network device.
 * @param[in] features           Data socket buffer types supported by this interface.
 *
 * @return
 * Pointer to the NSS core context.
 */
extern struct nss_ctx_instance *nss_register_tunipip6_if(uint32_t if_num, uint32_t type, nss_tunipip6_callback_t tunipip6_callback,
					nss_tunipip6_msg_callback_t event_callback, struct net_device *netdev, uint32_t features);

/**
 * nss_unregister_tunipip6_if
 *	Deregisters the TUNIPIP6 interface from the NSS.
 *
 * @param[in] if_num  NSS interface number.
 *
 * @return
 * None.
 */
extern void nss_unregister_tunipip6_if(uint32_t if_num);

/**
 * nss_tunipip6_msg_init
 *	Initializes a TUNIPIP6 message.
 *
 * @datatypes
 * nss_tunipip6_msg
 *
 * @param[in,out] ntm       Pointer to the IPIP6 tunnel message.
 * @param[in]     if_num    NSS interface number.
 * @param[in]     type      Type of message.
 * @param[in]     len       Size of the message.
 * @param[in]     cb        Pointer to the message callback.
 * @param[in]     app_data  Pointer to the application context of the message.
 *
 * @return
 * None.
 */
extern void nss_tunipip6_msg_init(struct nss_tunipip6_msg *ntm, uint16_t if_num, uint32_t type,  uint32_t len, void *cb, void *app_data);

/*
 * nss_tunipip6_get_context()
 *	Get tunipip6 context.
 *
 * @return
 * Pointer to the NSS core context.
 */
extern struct nss_ctx_instance *nss_tunipip6_get_context(void);

/** @} */ /* end_addtogroup nss_tunipip6_subsystem */

#endif /* __NSS_TUN6RD_H */
