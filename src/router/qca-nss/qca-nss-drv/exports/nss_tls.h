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
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE
 **************************************************************************
 */

/**
 * @file nss_tls.h
 *	NSS TLS common interface definitions, supports inner/outer interface split.
 */

#ifndef _NSS_TLS_H_
#define _NSS_TLS_H_

/**
 * @addtogroup nss_tls_subsystem
 * @{
 */
#define NSS_TLS_VER_TLS_1_1 0x0301			/**< TLS version 1.1, major and minor version. */
#define NSS_TLS_VER_TLS_1_2 0x0302			/**< TLS version 1.2, major and minor version. */
#define NSS_TLS_CLE_MAX 32				/**< Maximum classification error. */

/**
 * tls_msg_types
 *	Message types for TLS requests and responses.
 */
enum nss_tls_msg_type {
	NSS_TLS_MSG_TYPE_NODE_CONFIG,		/**< Configure TLS firmware node. */
	NSS_TLS_MSG_TYPE_NODE_SYNC,		/**< Node statistics. */
	NSS_TLS_MSG_TYPE_CTX_CONFIG,		/**< Send exception interface number. */
	NSS_TLS_MSG_TYPE_CTX_DECONFIG,		/**< Context deconfigure message. */
	NSS_TLS_MSG_TYPE_CTX_SYNC,		/**< Synchronize statistics. */
	NSS_TLS_MSG_TYPE_CIPHER_UPDATE,		/**< Context session update.  */
	NSS_TLS_MSG_MAX,			/**< Maximum message. */
};

/**
 * nss_tls_error
 *	TLS error.
 */
enum nss_tls_error {
	NSS_TLS_ERROR_NONE = 0,             /**< No error. */
	NSS_TLS_ERROR_UNKNOWN_MSG,          /**< Unknown message. */
	NSS_TLS_ERROR_ALREADY_CONFIGURE,    /**< Node already configured. */
	NSS_TLS_ERROR_FAIL_REG_INNER_CTX,   /**< Register inner context error. */
	NSS_TLS_ERROR_FAIL_REG_OUTER_CTX,   /**< Register outer context error. */
	NSS_TLS_ERROR_FAIL_REQ_POOL_ALLOC,  /**< Request pool allocation failed. */
	NSS_TLS_ERROR_INVALID_BLK_LEN,      /**< Invalid block length. */
	NSS_TLS_ERROR_INVALID_HASH_LEN,     /**< Invalid hash length. */
	NSS_TLS_ERROR_INVALID_VER,          /**< Invalid TLS version. */
	NSS_TLS_ERROR_INVALID_CTX_WORDS,    /**< Context words size mismatch with TLS. */
	NSS_TLS_ERROR_FAIL_ALLOC_HWCTX,     /**< Failed to allocate hardware context. */
	NSS_TLS_ERROR_FAIL_COPY_CTX,        /**< Failed to copy context. */
	NSS_TLS_ERROR_FAIL_NOMEM,           /**< Failed memory allocation. */
	NSS_TLS_ERROR_FAIL_INVAL_ALGO,	    /**< Invalid algorithm. */
	NSS_TLS_ERROR_MAX,                  /**< Maximum TLS error. */
};

/**
 * nss_tls_hw_stats
 *	TLS HW statistics.
 */
struct nss_tls_hw_stats {
	/*
	 * Dont change the order below
	 */
	uint32_t hw_len_error;			/**< Length error. */
	uint32_t hw_token_error;		/**< Token error, unknown token command/instruction. */
	uint32_t hw_bypass_error;		/**< Token contains too much bypass data. */
	uint32_t hw_crypto_error;		/**< Cryptograhic block size error. */
	uint32_t hw_hash_error;			/**< Hash block size error. */
	uint32_t hw_config_error;		/**< Invalid command/algorithm/mode/combination. */
	uint32_t hw_algo_error;			/**< Unsupported algorithm. */
	uint32_t hw_hash_ovf_error;		/**< Hash input overflow. */
	uint32_t hw_auth_error;         	/**< Hash input overflow. */
	uint32_t hw_pad_verify_error;		/**< Pad verification error. */
	uint32_t hw_timeout_error;              /**< Data timed out. */
};

/**
 * nss_tls_ctx_perf_stats
 *	TLS performance statistics.
 */
struct nss_tls_ctx_perf_stats {
	uint32_t no_desc_in;		/**< Ingress DMA descriptor not available. */
	uint32_t no_desc_out;		/**< Egress DMA descriptor not available. */
	uint32_t no_reqs;		/**< Not enough requests available for records. */
};

/**
 * nss_tls_ctx_stats
 *	TLS session statistics.
 */
struct nss_tls_ctx_stats {
	struct nss_cmn_node_stats pkt;		/**< Common node statistics. */
	uint32_t single_rec;			/**< Number of Tx single record datagrams. */
	uint32_t multi_rec;			/**< Number of multiple Tx record datagrams. */
	uint32_t tx_inval_reqs;			/**< Number of Tx invalidation successfully requested. */
	uint32_t rx_ccs_rec;			/**< Number of change cipher spec records received. */
	uint32_t fail_ccs;			/**< Failed to switch to new crypto. */
	uint32_t eth_node_deactive;		/**< Ethernet node deactivated as no crypto available. */
	uint32_t crypto_alloc_success;		/**< Number of crypto allocation succeeded. */
	uint32_t crypto_free_req;		/**< Number of crypto free request. */
	uint32_t crypto_free_success;		/**< Number of crypto free succeeded. */
	uint32_t fail_crypto_alloc;		/**< Number of crypto allocation failed. */
	uint32_t fail_crypto_lookup;		/**< Failed to find active crypto session. */
	uint32_t fail_req_alloc;		/**< Failure to allocate request memory pool.  */
	uint32_t fail_pbuf_stats;		/**< Failure in pbuf allocation for statistics. */
	uint32_t fail_ctx_active;		/**< Failure in enqueue due to inactive context. */

	struct nss_tls_hw_stats fail_hw;	/**< Hardware failure. */
	struct nss_tls_ctx_perf_stats perf;	/**< Performance related statistics. */
};

/**
 * nss_tls_node_stats
 * 	TLS node statistics.
 */
struct nss_tls_node_stats {
	uint32_t fail_ctx_alloc;	/**< Failure in allocating a context. */
	uint32_t fail_ctx_free;		/**< Failure in freeing up the context. */
	uint32_t fail_pbuf_stats;	/**< Failure in pbuf allocation for statistics. */
};

/**
 * nss_tls_ctx_config
 *	TLS context configuration.
 */
struct nss_tls_ctx_config {
	uint32_t except_ifnum; 		/**< Exception interface number. */
	uint32_t headroom;		/**< Headroom required for encapsulation. */
	uint32_t tailroom;		/**< Tailroom required for encapsulation. */
};

/**
 * nss_tls_cipher_update
 *	TLS cipher update message.
 *
 */
struct nss_tls_cipher_update {
	uint32_t crypto_idx;    /**< Crypto index for cipher context. */
	uint16_t ver;           /**< Version (TLS minor and major versions). */
	uint8_t skip;		/**< Skip hardware processing. */
	uint8_t reserved;	/**< Reserved for future use. */
};

/**
 * nss_tls_msg
 *	Data for sending and receiving TLS messages.
 */
struct nss_tls_msg {
	struct nss_cmn_msg cm;		/**< Common message header. */

	/**
	 * Payload of a TLS message.
	 */
	union {
		struct nss_tls_cipher_update cipher_update;	/**< Crypto configuration. */
		struct nss_tls_ctx_config ctx_cfg;	/**< Context configuration. */
		struct nss_tls_ctx_stats stats;		/**< Context statistics. */
		struct nss_tls_node_stats node_stats;	/**< Node statistics. */
	} msg;			/**< Message payload for TLS session messages exchanged with NSS core. */
};

/**
 * Callback function for receiving TLS messages.
 *
 * @datatypes
 * nss_cmn_msg
 *
 * @param[in] app_data  Pointer to the application context of the message.
 * @param[in] msg       Pointer to the message data.
 */
typedef void (*nss_tls_msg_callback_t)(void *app_data, struct nss_cmn_msg *msg);

/**
 * Callback function for receiving TLS session data.
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
typedef void (*nss_tls_data_callback_t)(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi);

/**
 * nss_tls_tx_buf
 *	Sends a TLS data packet to the NSS.
 *
 * @datatypes
 * sk_buff \n
 * nss_ctx_instance
 *
 * @param[in]     os_buf   Pointer to the OS data buffer.
 * @param[in]     if_num   NSS interface number.
 * @param[in]     nss_ctx  Pointer to the NSS core context.
 *
 * @return
 * Status of Tx buffer forwarded to NSS for TLS operation.
 */
nss_tx_status_t nss_tls_tx_buf(struct sk_buff *os_buf, uint32_t if_num, struct nss_ctx_instance *nss_ctx);

/**
 * nss_tls_tx_msg
 *	Sends an asynchronous IPsec message to the NSS.
 *
 * @datatypes
 * nss_ctx_instance \n
 * nss_tls_msg
 *
 * @param[in] nss_ctx  Pointer to the NSS HLOS driver context.
 * @param[in] msg      Pointer to the message data.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_tx_status_t nss_tls_tx_msg(struct nss_ctx_instance *nss_ctx, struct nss_tls_msg *msg);

/**
 * nss_tls_tx_msg_sync
 *	Sends a synchronous IPsec message to the NSS.
 *
 * @datatypes
 * nss_ctx_instance \n
 * nss_tls_msg_type \n
 * nss_tls_msg
 *
 * @param[in]  nss_ctx  Pointer to the NSS HLOS driver context.
 * @param[in]  if_num   NSS interface number.
 * @param[in]  type     Type of message.
 * @param[in]  len      Size of the payload.
 * @param[in]  nicm     Pointer to the NSS IPsec message.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_tx_status_t nss_tls_tx_msg_sync(struct nss_ctx_instance *nss_ctx, uint32_t if_num,
						enum nss_tls_msg_type type, uint16_t len,
						struct nss_tls_msg *ntcm);

/**
 * nss_tls_register_if
 *	Registers a TLS session interface with the NSS for sending and receiving
 *	messages.
 *
 * @datatypes
 * nss_tls_data_callback_t \n
 * nss_tls_msg_callback_t \n
 * net_device
 *
 * @param[in] if_num    NSS interface number.
 * @param[in] data_cb   Callback function for the message.
 * @param[in] msg_cb    Callback for TLS tunnel message.
 * @param[in] netdev    Pointer to the associated network device.
 * @param[in] features  Data socket buffer types supported by this interface.
 * @param[in] type      Type of message.
 * @param[in] app_ctx   Pointer to the application context.
 *
 * @return
 * Pointer to the NSS core context.
 */
extern struct nss_ctx_instance *nss_tls_register_if(uint32_t if_num,
							 nss_tls_data_callback_t data_cb,
							 nss_tls_msg_callback_t msg_cb,
							 struct net_device *netdev,
							 uint32_t features,
							 uint32_t type,
							 void *app_ctx);

/**
 * nss_tls_unregister_if
 *	Deregisters a TLS session interface from the NSS.
 *
 * @param[in] if_num  NSS interface number.
 *
 * @return
 * None.
 *
 * @dependencies
 * The TLS session interface must have been previously registered.
 */
extern void nss_tls_unregister_if(uint32_t if_num);

/**
 * nss_tls_notify_register
 *	Register an event callback to handle notification from TLS firmware package.
 *
 * @datatypes
 * nss_tls_msg_callback_t
 *
 * @param[in] ifnum     NSS interface number.
 * @param[in] ev_cb     Callback for TLS tunnel message.
 * @param[in] app_data  Pointer to the application context.
 *
 * @return
 * Pointer to NSS core context.
 */
extern struct nss_ctx_instance *nss_tls_notify_register(uint32_t ifnum, nss_tls_msg_callback_t ev_cb, void *app_data);

/**
 * nss_tls_notify_unregister
 *	Unregister an event callback.
 *
 * @param[in] ifnum  NSS interface number.
 *
 * @return
 * None.
 */
extern void nss_tls_notify_unregister(uint32_t ifnum);

/**
 * nss_tls_msg_init
 *	Initializes a TLS message sent asynchronously.
 *
 * @datatypes
 * nss_tls_msg
 *
 * @param[in,out] ncm       Pointer to the message.
 * @param[in]     if_num    NSS interface number.
 * @param[in]     type      Type of message.
 * @param[in]     len       Size of the payload.
 * @param[in]     cb        Pointer to the message callback.
 * @param[in]     app_data  Pointer to the application context.
 *
 * @return
 * None.
 */
extern void nss_tls_msg_init(struct nss_tls_msg *ncm, uint32_t if_num, uint32_t type, uint32_t len, void *cb, void *app_data);

/**
 * nss_tls_msg_sync_init
 *	Initializes a TLS message.
 *
 * @datatypes
 * nss_tls_msg
 *
 * @param[in,out] ncm       Pointer to the message.
 * @param[in]     if_num    NSS interface number.
 * @param[in]     type      Type of message.
 * @param[in]     len       Size of the payload.
 *
 * @return
 * None.
 */
extern void nss_tls_msg_sync_init(struct nss_tls_msg *ncm, uint32_t if_num, uint32_t type, uint32_t len);

/**
 * nss_tls_get_context
 *	Gets the NSS core context for the TLS session.
 *
 * @return
 * Pointer to the NSS core context.
 */
extern struct nss_ctx_instance *nss_tls_get_context(void);

/**
 * nss_tls_get_device
 *	Gets the original device from probe.
 *
 * @return
 * Pointer to the device.
 */
extern struct device *nss_tls_get_dev(struct nss_ctx_instance *nss_ctx);
/**
 * @}
 */

#endif /* _NSS_TLS_H_. */
