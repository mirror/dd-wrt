/*
 **************************************************************************
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
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
 * @file nss_dtls_cmn.h
 *	NSS DTLS common interface definitions, supports inner/outer interface split.
 */

#ifndef _NSS_DTLS_CMN_H_
#define _NSS_DTLS_CMN_H_

/**
 * @addtogroup nss_dtls_subsystem
 * @{
 */

#define NSS_DTLS_CMN_CTX_HDR_IPV6 0x0001		/**< DTLS with IPv6. */
#define NSS_DTLS_CMN_CTX_HDR_UDPLITE 0x0002		/**< DTLS with UDPLite. */
#define NSS_DTLS_CMN_CTX_HDR_CAPWAP 0x0004		/**< DTLS with CAPWAP. */
#define NSS_DTLS_CMN_CTX_CIPHER_MODE_GCM 0x0008		/**< DTLS with GCM cipher mode. */
#define NSS_DTLS_CMN_CTX_ENCAP_UDPLITE_CSUM 0x10000	/**< Checksum only UDPLite header. */
#define NSS_DTLS_CMN_CTX_ENCAP_METADATA 0x20000		/**< Valid metadata in encapsulation direction. */
#define NSS_DTLS_CMN_CTX_DECAP_ACCEPT_ALL 0x40000	/**< Exception all error packets to host. */

#define NSS_DTLS_CMN_CLE_MAX 32				/**< Max classification error. */

/**
 * nss_dtls_cmn_metadata_types
 *	Message types for DTLS requests and responses.
 */
enum nss_dtls_cmn_msg_type {
	NSS_DTLS_CMN_MSG_TYPE_CONFIGURE_NODE,	/**< Configure DTLS firmware node. */
	NSS_DTLS_CMN_MSG_TYPE_CONFIGURE_HDR,	/**< Configure the base context parameter. */
	NSS_DTLS_CMN_MSG_TYPE_CONFIGURE_DTLS,	/**< Configure DTLS parameters. */
	NSS_DTLS_CMN_MSG_TYPE_SWITCH_DTLS,	/**< Switch to new DTLS transform. */
	NSS_DTLS_CMN_MSG_TYPE_DECONFIGURE,	/**< Deconfigure context. */
	NSS_DTLS_CMN_MSG_TYPE_SYNC_STATS,	/**< Synchronize statistics. */
	NSS_DTLS_CMN_MSG_TYPE_NODE_STATS,	/**< Node statistics. */
	NSS_DTLS_CMN_MSG_MAX
};

/**
 * nss_dtls_cmn_error_response_types
 *	Error types for DTLS responses.
 */
enum nss_dtls_cmn_error {
	NSS_DTLS_CMN_ERROR_NONE = 0,
	NSS_DTLS_CMN_ERROR_UNKNOWN_MSG,
	NSS_DTLS_CMN_ERROR_INVALID_DESTIF,
	NSS_DTLS_CMN_ERROR_INVALID_SRCIF,
	NSS_DTLS_CMN_ERROR_INVALID_CRYPTO,
	NSS_DTLS_CMN_ERROR_INVALID_VER,
	NSS_DTLS_CMN_ERROR_INVALID_CTX_TYPE,
	NSS_DTLS_CMN_ERROR_INVALID_CTX_WORDS,
	NSS_DTLS_CMN_ERROR_FAIL_ALLOC_HWCTX,
	NSS_DTLS_CMN_ERROR_FAIL_COPY_CTX,
	NSS_DTLS_CMN_ERROR_FAIL_SWITCH_HWCTX,
	NSS_DTLS_CMN_ERROR_ALREADY_CONFIGURED,
	NSS_DTLS_CMN_ERROR_FAIL_NOMEM,
	NSS_DTLS_CMN_ERROR_FAIL_COPY_NONCE,
	NSS_DTLS_CMN_ERROR_MAX,
};

/**
 * nss_dtls_cmn_node_stats
 * 	DTLS node statistics.
 */
struct nss_dtls_cmn_node_stats {
	uint32_t fail_ctx_alloc;	/**< Failure in allocating a context. */
	uint32_t fail_ctx_free;		/**< Failure in freeing up the context. */
	uint32_t fail_pbuf_stats;	/**< Failure in pbuf allocation for statistics. */
};

/**
 * nss_dtls_cmn_hw_stats
 * 	DTLS hardware statistics.
 */
struct nss_dtls_cmn_hw_stats {
	uint32_t len_error;             /**< Length error. */
	uint32_t token_error;           /**< Token error, unknown token command/instruction. */
	uint32_t bypass_error;          /**< Token contains too much bypass data. */
	uint32_t config_error;          /**< Invalid command/algorithm/mode/combination. */
	uint32_t algo_error;            /**< Unsupported algorithm. */
	uint32_t hash_ovf_error;        /**< Hash input overflow. */
	uint32_t ttl_error;             /**< TTL or HOP-Limit underflow. */
	uint32_t csum_error;            /**< Checksum error. */
	uint32_t timeout_error;         /**< Data timed-out. */
};

/**
 * nss_dtls_cmn_ctx_stats
 *	DTLS session statistics.
 */
struct nss_dtls_cmn_ctx_stats {
	struct nss_cmn_node_stats pkt;		/**< Common node statistics. */
	uint32_t rx_single_rec;			/**< Received single DTLS record datagrams. */
	uint32_t rx_multi_rec;			/**< Received multiple DTLS record datagrams. */
	uint32_t fail_crypto_resource;		/**< Failure in allocation of crypto resource. */
	uint32_t fail_crypto_enqueue;		/**< Failure due to queue full in crypto or hardware. */
	uint32_t fail_headroom;			/**< Failure in headroom check. */
	uint32_t fail_tailroom;			/**< Failure in tailroom check. */
	uint32_t fail_ver;			/**< Failure in DTLS version check. */
	uint32_t fail_epoch;			/**< Failure in DTLS epoch check. */
	uint32_t fail_dtls_record;		/**< Failure in reading DTLS record. */
	uint32_t fail_capwap;			/**< Failure in CAPWAP classification. */
	uint32_t fail_replay;			/**< Failure in anti-replay check. */
	uint32_t fail_replay_dup;		/**< Failure in anti-replay; duplicate records. */
	uint32_t fail_replay_win;		/**< Failure in anti-replay; packet outside the window. */
	uint32_t fail_queue;			/**< Failure due to queue full in DTLS. */
	uint32_t fail_queue_nexthop;		/**< Failure due to queue full in next_hop. */
	uint32_t fail_pbuf_alloc;		/**< Failure in pbuf allocation. */
	uint32_t fail_pbuf_linear;		/**< Failure in pbuf linearization. */
	uint32_t fail_pbuf_stats;		/**< Failure in pbuf allocation for statistics. */
	uint32_t fail_pbuf_align;		/**< Failure in pbuf alignment. */
	uint32_t fail_ctx_active;		/**< Failure in enqueue due to inactive context. */
	uint32_t fail_hwctx_active;		/**< Failure in enqueue due to inactive hardware context. */
	uint32_t fail_cipher;			/**< Failure in decrypting the data. */
	uint32_t fail_auth;			/**< Failure in authenticating the data. */
	uint32_t fail_seq_ovf;			/**< Failure due to sequence number overflow. */
	uint32_t fail_blk_len;			/**< Failure in decapsulation due to bad cipher block length. */
	uint32_t fail_hash_len;			/**< Failure in decapsulation due to bad hash block length. */

	struct nss_dtls_cmn_hw_stats fail_hw;	/**< Hardware failure statistics. */

	uint32_t fail_cle[NSS_DTLS_CMN_CLE_MAX];/**< Classification errors. */

	uint32_t seq_low;			/**< Lower 32 bits of current Tx sequence number. */
	uint32_t seq_high;			/**< Upper 16 bits of current Tx sequence number. */

	uint16_t epoch;				/**< Current epoch value. */
	uint8_t res1[2];			/**< Reserved for future use. */

	uint8_t res2[16];			/**< Reserved for future use. */
};

/**
 * nss_dtls_cmn_ctx_config_hdr
 *	Parameters for outer header transform.
 */
struct nss_dtls_cmn_ctx_config_hdr {
	uint32_t flags;		/**< Context flags. */
	uint32_t dest_ifnum;	/**< Destination interface for packets. */
	uint32_t src_ifnum;	/**< Source interface of packets. */
	uint32_t sip[4];	/**< Source IPv4/v6 address. */
	uint32_t dip[4];	/**< Destination IPv4/v6 address. */

	uint16_t sport;		/**< Source UDP/UDPLite port. */
	uint16_t dport;		/**< Destination UDP/UDPLite port. */

	uint8_t hop_limit_ttl;	/**< IP header TTL field. */
	uint8_t dscp;		/**< DSCP value. */
	uint8_t dscp_copy; 	/**< Copy DSCP value. */
	uint8_t df;		/**< Do not fragment DTLS over IPv4. */
};

/**
 * nss_dtls_cmn_ctx_config_dtls
 *	Parameters for DTLS transform.
 */
struct nss_dtls_cmn_ctx_config_dtls {
	uint32_t ver;		/**< Version (enum dtls_cmn_ver). */
	uint32_t crypto_idx;	/**< Crypto index for cipher context. */

	uint16_t window_size;	/**< Anti-replay window size. */
	uint16_t epoch;		/**< Initial epoch value. */

	uint8_t iv_len;		/**< Crypto IV length for encapsulation. */
	uint8_t hash_len;	/**< Auth hash length for encapsulation. */
	uint8_t blk_len;	/**< Cipher block length. */
	uint8_t res1;		/**< Reserved for alignment. */
};

/**
 * nss_dtls_cmn_msg
 *	Data for sending and receiving DTLS messages.
 */
struct nss_dtls_cmn_msg {
	struct nss_cmn_msg cm;		/**< Common message header. */

	/**
	 * Payload of a DTLS message.
	 */
	union {
		struct nss_dtls_cmn_ctx_config_hdr hdr_cfg;	/**< Session configuration. */
		struct nss_dtls_cmn_ctx_config_dtls dtls_cfg;	/**< Cipher update information. */
		struct nss_dtls_cmn_ctx_stats stats;		/**< Session statistics. */
		struct nss_dtls_cmn_node_stats node_stats;	/**< Node statistics. */
	} msg;			/**< Message payload for DTLS session messages exchanged with NSS core. */
};

#ifdef __KERNEL__ /* only for kernel use. */
/**
 * Callback function for receiving DTLS messages.
 *
 * @datatypes
 * nss_dtls_cmn_msg
 *
 * @param[in] app_data  Pointer to the application context of the message.
 * @param[in] msg       Pointer to the message data.
 */
typedef void (*nss_dtls_cmn_msg_callback_t)(void *app_data, struct nss_cmn_msg *msg);

/**
 * Callback function for receiving DTLS session data.
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
typedef void (*nss_dtls_cmn_data_callback_t)(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi);

/**
 * nss_dtls_cmn_tx_buf
 *	Sends a DTLS data packet to the NSS.
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
 * Status of Tx buffer forwarded to NSS for DTLS operation.
 */
nss_tx_status_t nss_dtls_cmn_tx_buf(struct sk_buff *os_buf, uint32_t if_num, struct nss_ctx_instance *nss_ctx);

/**
 * nss_dtls_cmn_tx_msg
 *	Sends DTLS messages.
 *
 * @param[in]     nss_ctx  Pointer to the NSS core context.
 * @param[in,out] msg      Pointer to the message data.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_tx_status_t nss_dtls_cmn_tx_msg(struct nss_ctx_instance *nss_ctx, struct nss_dtls_cmn_msg *msg);

/**
 * nss_dtls_cmn_tx_msg_sync
 *	Sends DTLS messages synchronously.
 *
 * @datatypes
 * nss_ctx_instance \n
 * nss_dtls_cmn_msg_type \n
 * nss_dtls_cmn_msg \n
 * nss_dtls_cmn_error
 *
 * @param[in]     nss_ctx  Pointer to the NSS context.
 * @param[in]     if_num   NSS interface number.
 * @param[in]     type     Type of message.
 * @param[in]     len      Size of the payload.
 * @param[in]     ndcm     Pointer to the message data.
 * @param[in,out] resp     Response for the configuration.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_tx_status_t nss_dtls_cmn_tx_msg_sync(struct nss_ctx_instance *nss_ctx, uint32_t if_num,
						enum nss_dtls_cmn_msg_type type, uint16_t len,
						struct nss_dtls_cmn_msg *ndcm, enum nss_dtls_cmn_error *resp);

/**
 * nss_dtls_cmn_register_if
 *	Registers a DTLS session interface with the NSS for sending and receiving
 *	messages.
 *
 * @datatypes
 * nss_dtls_cmn_data_callback_t \n
 * nss_dtls_cmn_msg_callback_t
 *
 * @param[in] if_num    NSS interface number.
 * @param[in] data_cb   Callback function for the message.
 * @param[in] msg_cb    Callback for DTLS tunnel message.
 * @param[in] netdev    Pointer to the associated network device.
 * @param[in] features  Data socket buffer types supported by this interface.
 * @param[in] type      Type of message.
 * @param[in] app_ctx   Pointer to the application context.
 *
 * @return
 * Pointer to the NSS core context.
 */
extern struct nss_ctx_instance *nss_dtls_cmn_register_if(uint32_t if_num,
							 nss_dtls_cmn_data_callback_t data_cb,
							 nss_dtls_cmn_msg_callback_t msg_cb,
							 struct net_device *netdev,
							 uint32_t features,
							 uint32_t type,
							 void *app_ctx);

/**
 * nss_dtls_cmn_unregister_if
 *	Deregisters a DTLS session interface from the NSS.
 *
 * @param[in] if_num  NSS interface number.
 *
 * @return
 * None.
 *
 * @dependencies
 * The DTLS session interface must have been previously registered.
 */
extern void nss_dtls_cmn_unregister_if(uint32_t if_num);

/**
 * nss_dtls_cmn_notify_register
 *	Register an event callback to handle notification from DTLS firmware package.
 *
 * @param[in] ifnum     NSS interface number.
 * @param[in] ev_cb     Callback for DTLS tunnel message.
 * @param[in] app_data  Pointer to the application context.
 *
 * @return
 * Pointer to NSS core context.
 */
extern struct nss_ctx_instance *nss_dtls_cmn_notify_register(uint32_t ifnum, nss_dtls_cmn_msg_callback_t ev_cb,
							     void *app_data);

/**
 * nss_dtls_cmn_notify_unregister
 *	Unregister an event callback.
 *
 * @param[in] ifnum  NSS interface number.
 *
 * @return
 * None.
 */
extern void nss_dtls_cmn_notify_unregister(uint32_t ifnum);

/**
 * nss_dtls_cmn_msg_init
 *	Initializes a DTLS message.
 *
 * @datatypes
 * nss_dtls_cmn_msg
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
extern void nss_dtls_cmn_msg_init(struct nss_dtls_cmn_msg *ncm, uint32_t if_num, uint32_t type, uint32_t len, void *cb,
				void *app_data);

/**
 * nss_dtls_cmn_get_context
 *	Gets the NSS core context for the DTLS session.
 *
 * @return
 * Pointer to the NSS core context.
 */
extern struct nss_ctx_instance *nss_dtls_cmn_get_context(void);

/**
 * nss_dtls_cmn_get_ifnum
 *	Gets the DTLS interface number with a core ID.
 *
 * @param[in] if_num  NSS interface number.
 *
 * @return
 * Interface number with the core ID.
 */
extern int32_t nss_dtls_cmn_get_ifnum(int32_t if_num);

/**
 * @}
 */

#endif /* __KERNEL__ */
#endif /* _NSS_DTLS_CMN_H_. */
