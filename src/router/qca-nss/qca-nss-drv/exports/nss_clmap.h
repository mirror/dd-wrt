/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
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
 * @file nss_clmap.h
 *	NSS client map interface definitions.
 */

#ifndef __NSS_CLMAP_H
#define __NSS_CLMAP_H

 /**
  * @addtogroup nss_clmap_subsystem
  * @{
  */

/**
 * Maximum number of supported client map interface.
 */
#define NSS_CLMAP_MAX_INTERFACES 1

/**
 * nss_clmap_msg_type
 *	Client map message types.
 */
typedef enum nss_clmap_msg_type {
	NSS_CLMAP_MSG_TYPE_SYNC_STATS,		/**< Statistics synchronization message. */
	NSS_CLMAP_MSG_TYPE_INTERFACE_ENABLE,	/**< Enable the interface. */
	NSS_CLMAP_MSG_TYPE_INTERFACE_DISABLE,	/**< Disable the interface. */
	NSS_CLMAP_MSG_TYPE_MAC_ADD,		/**< Add MAC rule to the database. */
	NSS_CLMAP_MSG_TYPE_MAC_DEL,		/**< Remove MAC rule from the database. */
	NSS_CLMAP_MSG_TYPE_MAC_FLUSH,		/**< Flush all the MAC rules for a tunnel. */
	NSS_CLMAP_MSG_TYPE_MAX,			/**< Maximum message type. */
} nss_clmap_msg_type_t;

/**
 * nss_clmap_error_types
 *	Error types for client map responses to messages from the host.
 */
typedef enum nss_clmap_error_types {
	NSS_CLMAP_ERROR_UNKNOWN_TYPE = 1,	/**< Unknown type error. */
	NSS_CLMAP_ERROR_INTERFACE_DISABLED,	/**< Interface is already disabled. */
	NSS_CLMAP_ERROR_INTERFACE_ENABLED,	/**< Interface is already enabled. */
	NSS_CLMAP_ERROR_INVALID_VLAN,		/**< Invalid VLAN. */
	NSS_CLMAP_ERROR_INVALID_TUNNEL,		/**< Invalid tunnel. */
	NSS_CLMAP_ERROR_MAC_TABLE_FULL,		/**< MAC table is full. */
	NSS_CLMAP_ERROR_MAC_EXIST,		/**< MAC does already exist in the table. */
	NSS_CLMAP_ERROR_MAC_NOT_EXIST,		/**< MAC does not exist in the table. */
	NSS_CLMAP_ERROR_MAC_ENTRY_UNHASHED,	/**< MAC entry is not hashed in table. */
	NSS_CLMAP_ERROR_MAC_ENTRY_INSERT_FAILED,
						/**< Insertion into MAC table failed. */
	NSS_CLMAP_ERROR_MAC_ENTRY_ALLOC_FAILED,	/**< MAC entry allocation failed. */
	NSS_CLMAP_ERROR_MAC_ENTRY_DELETE_FAILED,/**< MAC entry deletion failed. */
	NSS_CLMAP_ERROR_MAX,			/**< Maximum error type. */
} nss_clmap_error_t;

/**
 * nss_clmap_stats_msg
 *	Per-interface statistics messages from the NSS firmware.
 */
struct nss_clmap_stats_msg {
	struct nss_cmn_node_stats node_stats;		/**< Common firmware statistics. */
	uint32_t dropped_macdb_lookup_failed;		/**< Dropped due to MAC database look up failed. */
	uint32_t dropped_invalid_packet_size;		/**< Dropped due to invalid size packets. */
	uint32_t dropped_low_hroom;			/**< Dropped due to insufficent headroom. */
	uint32_t dropped_next_node_queue_full;		/**< Dropped due to next node queue full. */
	uint32_t dropped_pbuf_alloc_failed;		/**< Dropped due to buffer allocation failure. */
	uint32_t dropped_linear_failed;			/**< Dropped due to linear copy failure. */
	uint32_t shared_packet_count;			/**< Shared packet count. */
	uint32_t ethernet_frame_error;			/**< Ethernet frame error count. */
	uint32_t macdb_create_requests;			/**< MAC database create requests count. */
	uint32_t macdb_create_mac_exists;		/**< MAC database create failures, MAC exist count. */
	uint32_t macdb_create_table_full;		/**< MAC database create failures, MAC database full count. */
	uint32_t macdb_destroy_requests;		/**< MAC database destroy requests count. */
	uint32_t macdb_destroy_mac_notfound;		/**< MAC database destroy failures, MAC not found count. */
	uint32_t macdb_destroy_mac_unhashed;		/**< MAC database destroy failures, MAC unhashed count. */
	uint32_t macdb_flush_requests;			/**< MAC database flush requests count. */
};

/**
 * nss_clmap_mac_msg
 *	Client map MAC message structure.
 */
struct nss_clmap_mac_msg {
	uint32_t vlan_id;			/**< VLAN ID. */
	uint32_t nexthop_ifnum;			/**< Next hop interface number. */
	uint32_t needed_headroom;			/**< Headroom to be added. */
	uint16_t mac_addr[3];			/**< MAC address. */
	uint8_t flags;				/**< Flags that carry metadata information. */
	uint8_t reserved;			/**< Reserved. */
};

/**
 * nss_clmap_flush_mac_msg
 *	CLient flush map MAC message structure.
 */
struct nss_clmap_flush_mac_msg {
	uint32_t nexthop_ifnum;			/**< Next hop interface number. */
};

/**
 * nss_clmap_msg
 *	Data for sending and receiving client map messages.
 */
struct nss_clmap_msg {
	struct nss_cmn_msg cm;		/**< Common message header. */

	/**
	 * Payload of a client map common message.
	 */
	union {
		struct nss_clmap_stats_msg stats;
				/**< Client map statistics. */
		struct nss_clmap_mac_msg mac_add;
				/**< MAC rule add message. */
		struct nss_clmap_mac_msg mac_del;
				/**< MAC rule delete message. */
		struct nss_clmap_flush_mac_msg mac_flush;
				/**< MAC rule flush message. */
	} msg;			/**< Message payload. */
};

/**
 * Callback function for receiving client map data.
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
typedef void (*nss_clmap_buf_callback_t)(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi);

/**
 * Callback function for receiving client map messages.
 *
 * @datatypes
 * nss_cmn_msg
 *
 * @param[in] app_data  Pointer to the application context of the message.
 * @param[in] msg       Pointer to the message data.
 */
typedef void (*nss_clmap_msg_callback_t)(void *app_data, struct nss_cmn_msg *msg);

/**
 * nss_clmap_tx_msg
 *	Sends client map messages to the NSS.
 *
 * Do not call this function from a softirq or interrupt because it
 * might sleep if the NSS firmware is busy serving another host thread.
 *
 * @datatypes
 * nss_ctx_instance \n
 * nss_clmap_msg
 *
 * @param[in]     nss_ctx  Pointer to the NSS context.
 * @param[in,out] msg      Pointer to the message data.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_tx_status_t nss_clmap_tx_msg(struct nss_ctx_instance *nss_ctx, struct nss_clmap_msg *msg);

/**
 * nss_clmap_tx_msg_sync
 *	Sends client map messages to the NSS.
 *
 * Do not call this function from a softirq or interrupt because it
 * might sleep if the NSS firmware is busy serving another host thread.
 *
 * @datatypes
 * nss_ctx_instance \n
 * nss_clmap_msg
 *
 * @param[in]     nss_ctx  Pointer to the NSS context.
 * @param[in,out] msg      Pointer to the message data.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_tx_status_t nss_clmap_tx_msg_sync(struct nss_ctx_instance *nss_ctx, struct nss_clmap_msg *msg);

/**
 * nss_clmap_tx_buf
 *	Sends a client map data buffer to the NSS interface.
 *
 * @datatypes
 * nss_ctx_instance \n
 * sk_buff
 *
 * @param[in] nss_ctx  Pointer to the NSS context.
 * @param[in] buf   Pointer to the data buffer.
 * @param[in] if_num   NSS interface number.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_tx_status_t nss_clmap_tx_buf(struct nss_ctx_instance *nss_ctx, struct sk_buff *buf, uint32_t if_num);

/**
 * nss_clmap_unregister
 *	Deregisters the client map interface from the NSS interface.
 *
 * @param[in] if_num  NSS interface number.
 *
 * @return
 * TRUE or FALSE
 *
 * @dependencies
 * The interface must have been previously registered.
 */
extern bool nss_clmap_unregister(uint32_t if_num);

/**
 * nss_clmap_register
 *	Registers the client map interface with the NSS for sending and
 *	receiving interface messages.
 *
 * @datatypes
 * nss_clmap_msg_callback_t \n
 * nss_clmap_buf_callback_t \n
 * net_device
 *
 * @param[in] if_num                    NSS interface number.
 * @param[in] dynamic_interface_type    NSS interface type.
 * @param[in] data_cb                   Data callback for the client map data.
 * @param[in] notify_cb                 Notify callback for the client map data.
 * @param[in] netdev                    Pointer to the associated network device.
 * @param[in] features                  Data socket buffer types supported by this interface.
 *
 * @return
 * Pointer to the NSS core context.
 */
extern struct nss_ctx_instance *nss_clmap_register(uint32_t if_num, uint32_t dynamic_interface_type,
			nss_clmap_buf_callback_t data_cb, nss_clmap_msg_callback_t notify_cb,
			struct net_device *netdev, uint32_t features);

/**
 * nss_clmap_get_ctx
 *	Get the NSS context.
 *
 * @return
 * Pointer to the NSS core context.
 */
extern struct nss_ctx_instance *nss_clmap_get_ctx(void);

/**
 * nss_clmap_ifnum_with_core_id
 *	Gets the client map interface number with the core ID.
 *
 * @param[in] if_num  NSS interface number.
 *
 * @return
 * Interface number with the core ID.
 */
extern int nss_clmap_ifnum_with_core_id(int if_num);

/**
 * nss_clmap_init
 *	Initializes the client map interface.
 *
 * @return
 * None.
 */
extern void nss_clmap_init(void);

/**
 * nss_clmap_msg_init
 *	Initializes a client map message.
 *
 * @datatypes
 * nss_clmap_msg \n
 * nss_clmap_msg_callback_t
 *
 * @param[in,out] ncm       Pointer to the message.
 * @param[in]     if_num    NSS interface number.
 * @param[in]     type      Type of message.
 * @param[in]     len       Size of the payload.
 * @param[in]     cb        Callback function for the message.
 * @param[in]     app_data  Pointer to the application context of the message.
 *
 * @return
 * None.
 */
extern void nss_clmap_msg_init(struct nss_clmap_msg *ncm, uint16_t if_num, uint32_t type, uint32_t len,
								nss_clmap_msg_callback_t cb, void *app_data);

/**
 * @}
 */

#endif /* __NSS_CLMAP_H */
