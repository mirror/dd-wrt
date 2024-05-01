/*
 **************************************************************************
 * Copyright (c) 2015-2017, The Linux Foundation. All rights reserved.
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
 * @file nss_wifi_if.h
 *	NSS Wi-Fi interface message Structure and APIs.
 */

#ifndef __NSS_WIFI_IF_H
#define __NSS_WIFI_IF_H

/**
 * @addtogroup nss_wifi_subsystem
 * @{
 */

/**
 * nss_wifi_if_msg_types
 *	Message types for Wi-Fi interface requests and responses.
 */
enum nss_wifi_if_msg_types {
	NSS_WIFI_IF_OPEN = NSS_IF_OPEN,
	NSS_WIFI_IF_CLOSE = NSS_IF_CLOSE,
	NSS_WIFI_IF_LINK_STATE_NOTIFY = NSS_IF_LINK_STATE_NOTIFY,
	NSS_WIFI_IF_MTU_CHANGE = NSS_IF_MTU_CHANGE,
	NSS_WIFI_IF_MAC_ADDR_SET = NSS_IF_MAC_ADDR_SET,
	NSS_WIFI_IF_STATS_SYNC = NSS_IF_STATS,
	NSS_WIFI_IF_ISHAPER_ASSIGN = NSS_IF_ISHAPER_ASSIGN,
	NSS_WIFI_IF_BSHAPER_ASSIGN = NSS_IF_BSHAPER_ASSIGN,
	NSS_WIFI_IF_ISHAPER_UNASSIGN = NSS_IF_ISHAPER_UNASSIGN,
	NSS_WIFI_IF_BSHAPER_UNASSIGN = NSS_IF_BSHAPER_UNASSIGN,
	NSS_WIFI_IF_ISHAPER_CONFIG = NSS_IF_ISHAPER_CONFIG,
	NSS_WIFI_IF_BSHAPER_CONFIG = NSS_IF_BSHAPER_CONFIG,
	NSS_WIFI_IF_VSI_ASSIGN = NSS_IF_VSI_ASSIGN,
	NSS_WIFI_IF_VSI_UNASSIGN = NSS_IF_VSI_UNASSIGN,
	NSS_WIFI_IF_TX_CREATE_MSG = NSS_IF_MAX_MSG_TYPES + 1,
	NSS_WIFI_IF_TX_DESTROY_MSG,
	NSS_WIFI_IF_STATS_SYNC_MSG,
	NSS_WIFI_IF_MAX_MSG_TYPES
};

/**
 * nss_wifi_if_error_types
 *	Error types for the Wi-Fi interface.
 */
enum nss_wifi_if_error_types {
	NSS_WIFI_IF_SUCCESS,
	NSS_WIFI_IF_CORE_FAILURE,
	NSS_WIFI_IF_ALLOC_FAILURE,
	NSS_WIFI_IF_DYNAMIC_IF_FAILURE,
	NSS_WIFI_IF_MSG_TX_FAILURE,
	NSS_WIFI_IF_REG_FAILURE,
	NSS_WIFI_IF_CORE_NOT_INITIALIZED
};

/**
 * nss_wifi_if_create_msg
 *	Payload for configuring the Wi-Fi interface.
 */
struct nss_wifi_if_create_msg {
	uint32_t flags;				/**< Interface flags. */
	uint8_t mac_addr[ETH_ALEN];		/**< MAC address. */
};

/**
 * nss_wifi_if_destroy_msg
 *	Payload for destroying the Wi-Fi interface.
 */
struct nss_wifi_if_destroy_msg {
	int32_t reserved;			/**< Placeholder. */
};

/**
 * nss_wifi_if_stats
 *	Wi-Fi interface statistics received from the NSS.
 */
struct nss_wifi_if_stats {
	struct nss_cmn_node_stats node_stats;
				/**< Common statistics. */
	uint32_t tx_enqueue_failed;
				/**< Number of packets dropped when queuing to the next node in a network graph. */
	uint32_t shaper_enqueue_failed;
				/**< Number of packets dropped when queuing to the shaper node. */
};

/**
 * nss_wifi_if_msg
 *	Data for sending and receiving Wi-Fi interface messages.
 */
struct nss_wifi_if_msg {
	struct nss_cmn_msg cm;			/**< Common message header. */

	/**
	 * Payload of a Wi-Fi interface message.
	 */
	union {
		union nss_if_msgs if_msgs;
				/**< NSS interface messages. */
		struct nss_wifi_if_create_msg create;
				/**< Creates a Wi-Fi interface rule. */
		struct nss_wifi_if_destroy_msg destroy;
				/**< Destroys a Wi-Fi interface rule. */
		struct nss_wifi_if_stats stats;
				/**< Interface statistics. */
	} msg;			/**< Message payload. */
};

/**
 * nss_wifi_if_pvt
 *	Private data information for the Wi-Fi interface.
 */
struct nss_wifi_if_pvt {
	struct semaphore sem;
			/**< Semaphore for a specified Wi-Fi interface number. */
	struct completion complete;
			/**< Waits for the NSS to process a message on the specified Wi-Fi interface. */
	int response;	/**< Response received on a Wi-Fi interface number. */
	int sem_init_done;
			/**< Indicates whether the semaphore is initialized. */
};

/**
 * Callback function for receiving Wi-Fi data.
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
typedef void (*nss_wifi_if_data_callback_t)(struct net_device *netdev,
		struct sk_buff *skb, struct napi_struct *napi);

/**
 * Callback function for receiving Wi-Fi messages.
 *
 * @datatypes
 * nss_cmn_msg
 *
 * @param[in] app_data  Pointer to the application context of the message.
 * @param[in] msg       Pointer to the message data.
 */
typedef void (*nss_wifi_if_msg_callback_t)(void *app_data,
		struct nss_cmn_msg *msg);

/**
 * nss_wifi_if_handle
 *	Context for WLAN-to-NSS communication.
 */
struct nss_wifi_if_handle {
	struct nss_ctx_instance *nss_ctx;	/**< NSS context. */
	int32_t if_num;				/**< Interface number. */
	struct nss_wifi_if_pvt *pvt;		/**< Private data structure. */
	struct nss_wifi_if_stats stats;
			/**< Statistics corresponding to this handle. */
	nss_wifi_if_msg_callback_t cb;
			/**< Callback registered by other modules. */
	void *app_data;
			/**< Application context to be passed to that callback. */
};

/**
 * nss_wifi_if_tx_msg
 *	Sends a message to the Wi-Fi interface.
 *
 * @datatypes
 * nss_ctx_instance \n
 * nss_wifi_if_msg
 *
 * @param[in] nss_ctx  Pointer to the NSS context (provided during registration).
 * @param[in] nwim     Pointer to the Wi-Fi interface message.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_tx_status_t nss_wifi_if_tx_msg(struct nss_ctx_instance *nss_ctx,
					struct nss_wifi_if_msg *nwim);

/**
 * nss_wifi_if_register
 *	Registers a Wi-Fi interface with the NSS driver.
 *
 * @datatypes
 * nss_wifi_if_handle \n
 * nss_wifi_if_data_callback_t \n
 * net_device
 *
 * @param[in] handle       Pointer to the Wi-Fi context (provided during Wi-Fi
 *                             interface allocation).
 * @param[in] rx_callback  Callback handler for Wi-Fi data packets.
 * @param[in] netdev       Pointer to the associated network device.
 *
 * @return
 * None.
 */
extern void nss_wifi_if_register(struct nss_wifi_if_handle *handle,
				nss_wifi_if_data_callback_t rx_callback,
				struct net_device *netdev);

/**
 * nss_wifi_if_unregister
 *	Deregisters a Wi-Fi interface from the NSS driver.
 *
 * @datatypes
 * nss_wifi_if_handle
 *
 * @param[in] handle  Pointer to the Wi-Fi context.
 *
 * @return
 * None.
 */
extern void nss_wifi_if_unregister(struct nss_wifi_if_handle *handle);

/**
 * nss_wifi_if_create_sync
 *	Creates a Wi-Fi interface.
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdev  Pointer to the associated network device.
 *
 * @return
 * Pointer to the Wi-Fi handle.
 */
extern struct nss_wifi_if_handle *nss_wifi_if_create_sync(struct net_device *netdev);

/**
 * nss_wifi_if_destroy_sync
 *	Destroys the Wi-Fi interface associated with the interface number.
 *
 * @datatypes
 * nss_wifi_if_handle
 *
 * @param[in] handle  Pointer to the Wi-Fi handle.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_tx_status_t nss_wifi_if_destroy_sync(struct nss_wifi_if_handle *handle);

/**
 * nss_wifi_if_tx_buf
 *	Sends a data packet or buffer to the NSS.
 *
 * @datatypes
 * nss_wifi_if_handle \n
 * sk_buff
 *
 * @param[in] handle  Context associated with the interface.
 * @param[in] skb     Pointer to the data socket buffer.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_tx_status_t nss_wifi_if_tx_buf(struct nss_wifi_if_handle *handle,
					struct sk_buff *skb);

/**
 * nss_wifi_if_copy_stats
 *	Copies Wi-Fi interface statistics for display.
 *
 * @param[in]  if_num  NSS interface number.
 * @param[in]  index   Index in the statistics array.
 * @param[out] line    Pointer to the buffer into which the statistics are copied.
 *
 * @return
 * Number of bytes copied.
 */
int32_t nss_wifi_if_copy_stats(int32_t if_num, int index, char *line);

/**
 * @}
 */

#endif /* __NSS_WIFI_IF_H */
