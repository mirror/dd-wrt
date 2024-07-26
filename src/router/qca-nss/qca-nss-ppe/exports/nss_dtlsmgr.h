/*
 **************************************************************************
 * Copyright (c) 2016-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
 * nss_dtlsmgr.h
 *	DTLS manager interface definitions.
 */
#ifndef _NSS_DTLSMGR_H_
#define _NSS_DTLSMGR_H_

#include <ppe_vp_public.h>

/**
 * NSS DTLS manager flags
 */
#define NSS_DTLSMGR_HDR_IPV6		0x00000001	/**< L3 header is v6 or v4 */
#define NSS_DTLSMGR_HDR_UDPLITE		0x00000002	/**< L4 header is UDP-Lite or UDP */
#define NSS_DTLSMGR_HDR_CAPWAP		0x00000004	/**< CAPWAP-DTLS or DTLS header */
#define NSS_DTLSMGR_CIPHER_MODE_GCM	0x00000008	/**< Cipher mode is GCM */
#define NSS_DTLSMGR_ENCAP_UDPLITE_CSUM	0x00010000	/**< Checksum UDP-Lite header */
#define NSS_DTLSMGR_ENCAP_METADATA	0x00020000	/**< Packets will have metadata for encapsulation. */
#define NSS_DTLSMGR_DECAP_ACCEPT_ALL	0x00040000	/**< Send all error packets after DECAP */

/*
 * DTLS header mask
 */
#define NSS_DTLSMGR_HDR_MASK (NSS_DTLSMGR_HDR_IPV6 | NSS_DTLSMGR_HDR_UDPLITE | NSS_DTLSMGR_HDR_CAPWAP)

/*
 * DTLS crypto feature mask
 */
#define NSS_DTLSMGR_CRYPTO_MASK NSS_DTLSMGR_CIPHER_MODE_GCM

/*
 * DTLS encapsulation specific flags mask
 */
#define NSS_DTLSMGR_ENCAP_MASK (NSS_DTLSMGR_ENCAP_UDPLITE_CSUM | NSS_DTLSMGR_ENCAP_METADATA)

/*
 * DTLS decapsulation specific flags mask
 */
#define NSS_DTLSMGR_DECAP_MASK NSS_DTLSMGR_DECAP_ACCEPT_ALL

/**
 * NSS DTLS manager TX metadata flags
 */
#define NSS_DTLSMGR_METADATA_MAGIC 0x8993		/**< Magic in DTLS metadata */
#define NSS_DTLSMGR_METADATA_FLAG_ENC 0x0001		/**< Metadata valid for encapsulation. */
#define NSS_DTLSMGR_METADATA_FLAG_SEQ 0x0002		/**< Metadata has a valid sequence no. */
#define NSS_DTLSMGR_METADATA_FLAG_CTYPE 0x0004		/**< Metadata has a valid DTLS content type */

/*
 * NSS DTLS manager reserved size of header
 */
#define NSS_DTLSMGR_NEEDED_HEADROOM_SZ 128
#define NSS_DTLSMGR_NEEDED_TAILROOM_SZ 128

/**
 * NSS DTLS manager status
 */
typedef enum nss_dtlsmgr_status {
	NSS_DTLSMGR_OK,			/**< Status ok. */
	NSS_DTLSMGR_FAIL,		/**< Failed due to unknown reason. */
	NSS_DTLSMGR_FAIL_NOMEM,		/**< Failed to allocate memory. */
	NSS_DTLSMGR_FAIL_NOCRYPTO,	/**< Failed to allocate crypto resource. */
	NSS_DTLSMGR_FAIL_MESSAGE,	/**< Failed to message the NSS. */
	NSS_DTLSMGR_INVALID_VERSION,	/**< Invalid DTLS version. */
	NSS_DTLSMGR_INVALID_ALGO,	/**< Invalid algorithm. */
	NSS_DTLSMGR_INVALID_KEYLEN,	/**< Invalid key length for cipher/auth. */
	NSS_DTLSMGR_FAIL_VP_ALLOC,	/**< Failed to alloca VP for dtls. */
	NSS_DTLSMGR_FAIL_VP_FREE	/**< Failed to free VP for dtls. */
} nss_dtlsmgr_status_t;

/**
 * DTLS protocol version
 */
enum nss_dtlsmgr_dtlsver {
	NSS_DTLSMGR_VERSION_1_0,	/**< Protocol v1.0. */
	NSS_DTLSMGR_VERSION_1_2,	/**< Protocol v1.2. */
};

/**
 * DTLS interface type
 */
enum nss_dtlsmgr_interface_type {
	NSS_DTLSMGR_INTERFACE_TYPE_NONE,
	NSS_DTLSMGR_INTERFACE_TYPE_INNER,	/**< DTLS encapsulation interface */
	NSS_DTLSMGR_INTERFACE_TYPE_OUTER,	/**< DTLS decapsulation interface */
	NSS_DTLSMGR_INTERFACE_TYPE_MAX
};

/**
 * NSS DTLS manager supported cryptographic algorithms
 */
enum nss_dtlsmgr_algo {
	NSS_DTLSMGR_ALGO_AES_CBC_SHA1_HMAC,	/**< AES_CBC_SHA1_HMAC. */
	NSS_DTLSMGR_ALGO_AES_CBC_SHA256_HMAC,	/**< AES_CBC_SHA256_HMAC. */
	NSS_DTLSMGR_ALGO_3DES_CBC_SHA1_HMAC,	/**< 3DES_CBC_SHA1_HMAC. */
	NSS_DTLSMGR_ALGO_3DES_CBC_SHA256_HMAC,	/**< 3DES_CBC_SHA256_HMAC. */
	NSS_DTLSMGR_ALGO_AES_GCM,		/**< AES_GCM. */
	NSS_DTLSMGR_ALGO_MAX
};

/**
 * NSS DTLS manager metadata ctype
 */
enum nss_dtlsmgr_metadata_ctype {
	NSS_DTLSMGR_METADATA_CTYPE_CCS = 20,		/**< DTLS packet is change cipher specification.*/
	NSS_DTLSMGR_METADATA_CTYPE_ALERT = 21,		/**< DTLS packet is Alert.*/
	NSS_DTLSMGR_METADATA_CTYPE_HANDSHAKE = 22,	/**< DTLS packet is Handshake.*/
	NSS_DTLSMGR_METADATA_CTYPE_APP = 23,		/**< DTLS packet is Application data. */
};

/**
 * NSS DTLS manager metadata result type
 */
enum nss_dtlsmgr_metadata_result {
	NSS_DTLSMGR_METADATA_RESULT_OK = 0,		/**< Result OK. */
	NSS_DTLSMGR_METADATA_RESULT_AUTH_FAIL = 1,	/**< Authenication failure. */
	NSS_DTLSMGR_METADATA_RESULT_CIPHER_FAIL = 2,	/**< Cipher failure. */
	NSS_DTLSMGR_METADATA_RESULT_MAX,
};

/**
 * NSS DTLS manager cryptographic structure to represent key and its length.
 */
struct nss_dtlsmgr_crypto_data {
	const uint8_t *data;		/**< Pointer to key or nonce. */
	uint16_t len;			/**< Length of the key. */
};

/**
 * NSS DTLS manager cryptographic data
 */
struct nss_dtlsmgr_crypto {
	enum nss_dtlsmgr_algo algo;			/**< DTLS manager cryptographic algorithm. */
	struct nss_dtlsmgr_crypto_data cipher_key;	/**< Cipher key. */
	struct nss_dtlsmgr_crypto_data auth_key;	/**< Authentication key. */
	struct nss_dtlsmgr_crypto_data nonce;		/**< Nonce. */
};

/**
 * NSS DTLS manager session encapsulation data
 */
struct nss_dtlsmgr_encap_config {
	struct nss_dtlsmgr_crypto crypto;	/**< Encapsulation crypto configuration. */
	enum nss_dtlsmgr_dtlsver ver;		/**< Version used in DTLS header. */
	uint32_t sip[4];			/**< Source IP address. */
	uint32_t dip[4];			/**< Destination IP address. */
	uint16_t sport;				/**< Source UDP port. */
	uint16_t dport;				/**< Destination UDP port. */
	uint16_t epoch;				/**< Epoch. */
	uint8_t ip_ttl;				/**< IP time to live. */
	uint8_t dscp;				/**< DSCP. */
	bool dscp_copy;				/**< Flag to check if DSCP needs to be copied. */
	bool df;				/**< Flag to check fragmentation. */
};

/**
 * NSS DTLS manager session decapsulation data
 */
struct nss_dtlsmgr_decap_config {
	struct nss_dtlsmgr_crypto crypto;	/**< Decap Crypto configuration. */
	uint32_t nexthop_ifnum;			/**< NSS I/F number to forward after de-capsulation. */
	uint16_t window_size;			/**< Anti-Replay window size. */
};

/*
 * NSS DTLS manager hardware statistics
 */
struct nss_dtlsmgr_hw_stats {
	uint64_t len_error;             /**< Length error. */
	uint64_t token_error;           /**< Token error, unknown token command/instruction. */
	uint64_t bypass_error;          /**< Token contains too much bypass data. */
	uint64_t config_error;          /**< Invalid command/algorithm/mode/combination. */
	uint64_t algo_error;            /**< Unsupported algorithm. */
	uint64_t hash_ovf_error;        /**< Hash input overflow. */
	uint64_t ttl_error;             /**< TTL or HOP-Limit underflow. */
	uint64_t csum_error;            /**< Checksum error. */
	uint64_t timeout_error;         /**< Data timed-out. */
};

/**
 * NSS DTLS manager session statistics
 */
struct nss_dtlsmgr_stats {
	uint64_t tx_packets;		/**< Tx packets. */
	uint64_t tx_bytes;		/**< Tx bytes. */
	uint64_t rx_packets;		/**< Rx packets. */
	uint64_t rx_bytes;		/**< Rx bytes. */
	uint64_t rx_dropped;		/**< Rx drops. */
	uint64_t rx_single_rec;		/**< Received single DTLS record datagrams. */
	uint64_t rx_multi_rec;		/**< Received multiple DTLS record datagrams. */
	uint64_t fail_crypto_resource;	/**< Failure in allocation of crypto resource. */
	uint64_t fail_crypto_enqueue;	/**< Failure due to queue full in crypto or hardware. */
	uint64_t fail_headroom;		/**< Failure in headroom check. */
	uint64_t fail_tailroom;		/**< Failure in tailroom check. */
	uint64_t fail_ver;		/**< Failure in DTLS version check. */
	uint64_t fail_epoch;		/**< Failure in DTLS epoch check. */
	uint64_t fail_dtls_record;	/**< Failure in reading DTLS record. */
	uint64_t fail_capwap;		/**< Failure in Capwap classification. */
	uint64_t fail_replay;		/**< Failure in anti-replay check. */
	uint64_t fail_replay_dup;	/**< Failure in anti-replay; duplicate records. */
	uint64_t fail_replay_win;	/**< Failure in anti-replay; packet outside the window. */
	uint64_t fail_queue;		/**< Failure due to queue full in DTLS. */
	uint64_t fail_queue_nexthop;	/**< Failure due to queue full in next_hop. */
	uint64_t fail_pbuf_alloc;	/**< Failure in pbuf allocation. */
	uint64_t fail_pbuf_linear;	/**< Failure in pbuf linearization. */
	uint64_t fail_pbuf_stats;	/**< Failure in pbuf allocation for stats. */
	uint64_t fail_pbuf_align;	/**< Failure in pbuf alignment. */
	uint64_t fail_ctx_active;	/**< Failure in enqueue due to no active context. */
	uint64_t fail_hwctx_active;	/**< Failure in enqueue due to no active HW context. */
	uint64_t fail_cipher;		/**< Failure in decrypting the data. */
	uint64_t fail_auth;		/**< Failure in authenticating the data. */
	uint64_t fail_seq_ovf;		/**< Failure due to sequence number overflow. */
	uint64_t fail_blk_len;		/**< Failure in decapsulation due to bad cipher block length. */
	uint64_t fail_hash_len;		/**< Failure in decapsulation due to bad hash block length. */

	struct nss_dtlsmgr_hw_stats fail_hw;
					/**< Hardware failure statistics. */

	uint64_t fail_cle[NSS_DTLS_CMN_CLE_MAX];
					/**< Classification errors. */

	uint64_t fail_host_tx;		/**< Failure at host TX. */
	uint64_t fail_host_rx;		/**< Failure at host RX. */

	uint32_t seq_low;		/**< Lower 32 bits of current Tx sequence number. */
	uint32_t seq_high;		/**< Upper 16 bits of current Tx sequence number. */

	uint16_t epoch;			/**< Current Epoch value. */
};

#ifdef __KERNEL__ /* only for kernel use. */

/**
 * NSS DTLS manager session stats update callback
 */
typedef void (*nss_dtlsmgr_notify_callback_t)(void *app_data, struct net_device *dev,
						struct nss_dtlsmgr_stats *stats, bool encap);
typedef void (*nss_dtlsmgr_data_callback_t)(void *app_data, struct sk_buff *skb);

/**
 * NSS DTLS manager session definition
 */
struct nss_dtlsmgr_config {
	uint32_t flags;					/**< DTLS header flags. */
	void *app_data;					/**< Opaque data returned in callback. */

	nss_dtlsmgr_notify_callback_t notify;		/**< Statistics notifcation callback. */
	nss_dtlsmgr_data_callback_t data;		/**< Data callback. */

	struct nss_dtlsmgr_encap_config encap;		/**< Encap data. */
	struct nss_dtlsmgr_decap_config decap;		/**< Decap data. */

	ppe_vp_num_t vp_num_encap;			/**< UL VP number. */
};

#endif /* __KERNEL__ */
/**
 * NSS DTLS manager session tx/rx cipher update parameters
 */
struct nss_dtlsmgr_config_update {
	struct nss_dtlsmgr_crypto crypto;	/**< Crypto algorithm and key data. */
	uint16_t epoch;				/**< Epoch. */
	uint16_t window_size;			/**< Anti-Replay window size. */
};

/**
 * NSS DTLS manager metadata
 */
struct nss_dtlsmgr_metadata {
	uint8_t ctype;		/**< Type of DTLS packet. */
	uint8_t result;		/**< Error during DTLS decapsulation. */
	uint16_t len;		/**< Length of DTLS payload. */
	uint32_t seq;		/**< Sequence for encapsulation. */
	uint16_t flags;		/**< Metadata flags. */
	uint16_t magic;		/**< Magic. */
};

#ifdef __KERNEL__ /* only for kernel use. */

/**
 * nss_dtlsmgr_metadata_init
 *	Initializes the metadata at SKB head
 *
 * @param skb[IN] Socket buffer
 *
 * @return
 * Return NSS DTLSMGR metadata start address
 */
static inline struct nss_dtlsmgr_metadata *nss_dtlsmgr_metadata_init(struct sk_buff *skb)
{
	struct nss_dtlsmgr_metadata *ndm;

	if (unlikely(skb_headroom(skb) < sizeof(*ndm)))
		return NULL;

	ndm = (struct nss_dtlsmgr_metadata *)skb_push(skb, sizeof(*ndm));
	/*
	 * Initialize the metadata with default values
	 */
	ndm->flags = NSS_DTLSMGR_METADATA_FLAG_ENC;
	ndm->ctype = NSS_DTLSMGR_METADATA_CTYPE_APP;
	ndm->magic = NSS_DTLSMGR_METADATA_MAGIC;
	ndm->len = skb->len - sizeof(*ndm);
	ndm->seq = U32_MAX;
	ndm->result = 0;

	return ndm;
}

/**
 * nss_dtlsmgr_metadata_set_seq
 *	Update the metadata with sequence number for the first encap packet
 *
 * @param ndtm[IN] NSS DTLSMGR metadata
 * @param seq[IN]  Starting sequence number for the tunnel encapsulation
 */
static inline void nss_dtlsmgr_metadata_set_seq(struct nss_dtlsmgr_metadata *ndm, uint32_t seq)
{
	ndm->flags |= NSS_DTLSMGR_METADATA_FLAG_SEQ;
	ndm->seq = seq;
}

/**
 * nss_dtlsmgr_metadata_set_ctype
 *	Update the metadata with DTLS content type
 *
 * @param ndtm[IN]  NSS DTLSMGR metadata
 * @param ctype[IN] DTLS content type for encapsulation
 */
static inline void nss_dtlsmgr_metadata_set_ctype(struct nss_dtlsmgr_metadata *ndm, enum nss_dtlsmgr_metadata_ctype ctype)
{
	ndm->flags |= NSS_DTLSMGR_METADATA_FLAG_CTYPE;
	ndm->ctype = ctype;
}

/**
 * nss_dtlsmgr_metadata_get_ctype
 *	Returns the type of DTLS payload
 *
 * @param ndm[IN] DTLS metadata header
 *
 * @return
 * NSS_DTLSMGR_METADATA_CTYPE_APP for normal data
 */
static inline enum nss_dtlsmgr_metadata_ctype nss_dtlsmgr_metadata_get_ctype(struct nss_dtlsmgr_metadata *ndm)
{
	return ndm->ctype;
}

/**
 * nss_dtlsmgr_metadata_get_error
 *	Returns the error seen during decapsulation
 *
 * @param ndm[IN] DTLS metadata header
 *
 * @return
 * NSS_DTLSMGR_METADATA_RESULT_OK for success
 */
static inline enum nss_dtlsmgr_metadata_result nss_dtlsmgr_metadata_get_result(struct nss_dtlsmgr_metadata *ndm)
{
	return ndm->result;
}

/**
 * nss_dtlsmgr_metadata_verify_magic
 *	Returns true if magic pattern matches
 *
 * @param ndm[IN] DTLS metadata header
 *
 * @return
 * true for success
 */
static inline bool nss_dtlsmgr_metadata_verify_magic(struct nss_dtlsmgr_metadata *ndm)
{
	return ndm->magic == NSS_DTLSMGR_METADATA_MAGIC;
}

/**
 * nss_dtlsmgr_session_create
 *	Create a NSS DTLS session and associated crypto sessions
 *
 * @param in_data[IN] DTLS session create data
 * @param out_data[out] Return parameters from DTLS session create operation
 *
 * @return
 * NSS_DTLSMGR_OK for success
 */
struct net_device *nss_dtlsmgr_session_create(struct nss_dtlsmgr_config *cfg);

/**
 * nss_dtlsmgr_session_destroy
 *	Destroy a NSS DTLS session and associated crypto sessions
 *
 * @param dtls_if[IN] NSS DTLS I/F
 *
 * @return
 * NSS_DTLSMGR_OK for success
 */
nss_dtlsmgr_status_t nss_dtlsmgr_session_destroy(struct net_device *dev);

/**
 * nss_dtlsmgr_session_update_encap
 *	Update encapsulation cipher state of a DTLS session.
 *	Configures new parameters into the pending encapsulation cipher
 *	state of a DTLS session. This has no effect on the current
 *	cipher state and its processing of packets.
 *
 * @param dev[IN] DTLS network device
 * @param cfg[IN] DTLS session update parameters
 *
 * @return NSS_DTLSMGR_OK for success
 */
nss_dtlsmgr_status_t nss_dtlsmgr_session_update_encap(struct net_device *dev, struct nss_dtlsmgr_config_update *cfg);

/**
 * nss_dtlsmgr_session_update_decap
 *	Update decapsulation cipher state of a DTLS session.
 *	Configures new parameters into the pending decapsulation cipher
 *	state of a DTLS session. This has no effect on the current
 *	cipher state and its processing of packets.
 *
 * @param dev[IN] DTLS network device
 * @param cfg[IN] DTLS session update parameters
 *
 * @return NSS_DTLSMGR_OK for success
 */
nss_dtlsmgr_status_t nss_dtlsmgr_session_update_decap(struct net_device *dev, struct nss_dtlsmgr_config_update *cfg);

/**
 * nss_dtlsmgr_session_switch_encap
 *	Switch DTLS session from current to new for encapsulation
 *
 * @param dev[in] DTLS network device
 *
 * @return TRUE for success
 */
bool nss_dtlsmgr_session_switch_encap(struct net_device *dev);

/**
 * nss_dtlsmgr_session_switch_decap
 *	Switch DTLS session from current to new for de-capsulation
 *
 * @param dev[in] DTLS network device
 *
 * @return TRUE for success
 */
bool nss_dtlsmgr_session_switch_decap(struct net_device *dev);

/**
 * nss_dtlmsgr_get_interface
 *	Get the NSS interface number for encap/decap interface.
 *
 * @param dev[in] DTLS network device
 * @param type[in] DTLS interface type
 *
 * @return interface number for success
 */
int32_t nss_dtlsmgr_get_interface(struct net_device *dev, enum nss_dtlsmgr_interface_type type);

/**
 * nss_dtlsmgr_encap_overhead
 *	Get the DTLS overhead.
 *
 * @param dev[in] DTLS network device
 *
 * @return Overhead for dtls netdevice
 */
uint32_t nss_dtlsmgr_encap_overhead(struct net_device *dev);

#endif /* __KERNEL__ */
#endif /* _NSS_DTLSMGR_H_ */
