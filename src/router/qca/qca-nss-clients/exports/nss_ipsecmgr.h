/*
 **************************************************************************
 * Copyright (c) 2014-2019, The Linux Foundation. All rights reserved.
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
 * @file nss_ipsecmgr.h
 *	NSS IPSec Manager interface definitions.
 */

#ifndef __NSS_IPSECMGR_H
#define __NSS_IPSECMGR_H

/**
 * @addtogroup nss_ipsec_manager_subsystem
 * @{
 */
#define NSS_IPSECMGR_TUN_NAME "ipsectun%d"	/**< IPsec tunnel name. */

/**
 * Length of the header added after encapsulation.
 *
 * This estimate must be accurate but large enough to accomodate most use cases.
 */
#define NSS_IPSECMGR_TUN_MAX_HDR_LEN 96

/*
 * Space required in the head and tail of the buffer.
 */
#define NSS_IPSECMGR_TUN_HEADROOM 128		/**< Size of the buffer headroom. */
#define NSS_IPSECMGR_TUN_TAILROOM 192		/**< Size of the buffer tailroom. */

#define NSS_IPSECMGR_TUN_MTU(x) (x - NSS_IPSECMGR_TUN_MAX_HDR_LEN)
						/**< MTU of the IPsec tunnel. */

#define NSS_IPSECMGR_NATT_PORT_DATA 4500	/**< Number of the NATT port. */

#define NSS_IPSECMGR_CIPHER_KEYLEN_MAX 32	/**< Max cipher key length. */
#define NSS_IPSECMGR_AUTH_KEYLEN_MAX 32		/**< Max auth key length. */
#define NSS_IPSECMGR_NONCE_SIZE_MAX 4		/**< Max nonce size. */

/**
 * nss_ipsecmgr_event_type
 *	Event types for the IPsec manager.
 */
enum nss_ipsecmgr_event_type {
	NSS_IPSECMGR_EVENT_NONE = 0,
	NSS_IPSECMGR_EVENT_SA_STATS,	/**< Event type for IPsec mangager SA stats. */
	NSS_IPSECMGR_EVENT_MAX
};

/**
 * nss_ipsecmgr_status
 * 	Return statuses for IPsec manager.
 */
typedef enum nss_ipsecmgr_status {
	NSS_IPSECMGR_OK,		/**< Status ok. */
	NSS_IPSECMGR_FAIL,		/**< Failed due to unknown reason. */
	NSS_IPSECMGR_FAIL_NOMEM,	/**< Failed to allocate memory. */
	NSS_IPSECMGR_FAIL_NOCRYPTO,	/**< Failed to allocate crypto resource. */
	NSS_IPSECMGR_FAIL_MESSAGE,	/**< Failed to message the NSS. */
	NSS_IPSECMGR_FAIL_ADD_DB,	/**< Failed to add to database. */
	NSS_IPSECMGR_FAIL_FLOW_ALLOC,	/**< Failed to alloc flow. */
	NSS_IPSECMGR_FAIL_FLOW,		/**< Failed to find the flow. */
	NSS_IPSECMGR_INVALID_CTX,	/**< Invalid context */
	NSS_IPSECMGR_INVALID_ALGO,	/**< Invalid algorithm. */
	NSS_IPSECMGR_INVALID_IPVER,	/**< Invalid IP version. */
	NSS_IPSECMGR_INVALID_CRYPTO_IDX,/**< Invalid crypto index */
	NSS_IPSECMGR_INVALID_KEYLEN,	/**< Invalid key length for cipher or authentication. */
	NSS_IPSECMGR_INVALID_WINDOW,	/**< Invalid window size. */
	NSS_IPSECMGR_INVALID_SA,	/**< Ivalid SA. */
	NSS_IPSECMGR_DUPLICATE_SA,	/**< Duplicate SA allocation. */
	NSS_IPSECMGR_DUPLICATE_FLOW,	/**< Duplicate flow allocation. */
} nss_ipsecmgr_status_t;

/**
 * nss_ipsecmgr_algo
 * 	IPsec manager supported cryptographic algorithms.
 */
enum nss_ipsecmgr_algo {
	NSS_IPSECMGR_ALGO_AES_CBC_SHA1_HMAC,		/**< AES_CBC_SHA1_HMAC. */
	NSS_IPSECMGR_ALGO_AES_CBC_SHA256_HMAC,		/**< AES_CBC_SHA256_HMAC. */
	NSS_IPSECMGR_ALGO_3DES_CBC_SHA1_HMAC,		/**< 3DES_CBC_SHA1_HMAC. */
	NSS_IPSECMGR_ALGO_3DES_CBC_SHA256_HMAC,		/**< 3DES_CBC_SHA256_HMAC. */
	NSS_IPSECMGR_ALGO_NULL_CIPHER_SHA1_HMAC,	/**< NULL_CIPHER_SHA1_HMAC. */
	NSS_IPSECMGR_ALGO_NULL_CIPHER_SHA256_HMAC,	/**< NULL_CIPHER_SHA256_HMAC. */
	NSS_IPSECMGR_ALGO_AES_GCM_GMAC_RFC4106,		/**< AES GCM/GMAC based on RFC4106 */
	NSS_IPSECMGR_ALGO_AES_CBC_MD5_HMAC,		/**< AES_CBC_MD5_HMAC. */
	NSS_IPSECMGR_ALGO_3DES_CBC_MD5_HMAC,		/**< 3DES_CBC_MD5_HMAC. */
	NSS_IPSECMGR_ALGO_MAX
};

/**
 * nss_ipsecmgr_sa_type
 * 	Types of security associations in IPsec manager.
 */
enum nss_ipsecmgr_sa_type {
	NSS_IPSECMGR_SA_TYPE_NONE = 0,
	NSS_IPSECMGR_SA_TYPE_ENCAP,	/**< Encap type SA. */
	NSS_IPSECMGR_SA_TYPE_DECAP,	/**< Decap type SA. */
	NSS_IPSECMGR_SA_TYPE_MAX
};

/**
 * nss_ipsecmgr_crypto_keys
 * 	Information required to configure crypto session for IPsec.
 */
struct nss_ipsecmgr_crypto_keys {
	const uint8_t *cipher_key;	/**< Cipher key. */
	const uint8_t *auth_key;	/**< Authentication key. */
	const uint8_t *nonce;		/**< Nonce. */

	uint16_t cipher_keylen;		/**< Cipher key length. */
	uint16_t auth_keylen;		/**< Authentication key length. */
	uint16_t nonce_size;		/**< Nonce size. */
};

/**
 * nss_ipsecmgr_crypto_index
 * 	Information about a pre-configured crypto session.
 */
struct nss_ipsecmgr_crypto_index {
	uint16_t session;		/**< Crypto session index. */
	uint8_t blk_len;		/**< Cipher block length. */
	uint8_t iv_len;			/**< Cipher IV length. */
};

/**
 * nss_ipsecmgr_sa_cmn
 * 	Common information necessary to configure an SA.
 */
struct nss_ipsecmgr_sa_cmn {
	enum nss_ipsecmgr_algo algo;	/**< Supported crypto algorithms */
	struct nss_ipsecmgr_crypto_keys keys;	/**< Crypto keys */
	struct nss_ipsecmgr_crypto_index index;	/**< Crypto index or offset */

	uint8_t icv_len;	/**< Hash length. */
	bool skip_trailer;	/**< Skip the ESP trailer for encapsulation. */
	bool enable_esn;	/**< Enable the extended sequence number. */
	bool enable_natt;	/**< NAT-T is required. */
	bool crypto_has_keys;	/**< Crypto configured with keys. */
};

/**
 * nss_ipsecmgr_sa_encap
 *	SA information for an encapsulation flow.
 *
 * For DSCP marking, use the following settings:
 * - Copy inner header to outer header:
 *    - dscp_copy = 1
 *    - dscp = 0
 * - Fixed mark on outer header:
 *    - dscp_copy = 0
 *    - dscp = <0 to 63>
 * - Transmit default
 *	SA with TX default is used when host originating flows don't have
 *	an explicit inner flow rule programmed for IPsec.
 */
struct nss_ipsecmgr_sa_encap {
	uint32_t seq_start;	/**< Starting sequence number (Not used) */
	uint8_t ttl_hop_limit;	/**< Time-to-Live or hop limit. */
	uint8_t dscp;		/**< Default DSCP value of the security association. */
	uint8_t df;		/**< Don't-Fragment value for the outer header, if nocopy is selected. */

	bool copy_dscp;		/**< Copy DSCP from the inner header to the outer header. */
	bool copy_df;		/**< Copy DF from the inner header to the outer header. */
	bool tx_default;	/**< TX SA(one per tunnel) for host traffic, without a flow rule.*/
};

/**
 * nss_ipsecmgr_sa_decap
 *	SA information for a decapsulation flow.
 */
struct nss_ipsecmgr_sa_decap {
	uint32_t replay_fail_thresh;	/**< Threshold for consecutive hash failures in replay. */
	uint16_t replay_win;	/**< Sequence number window size for anti-replay. */
};

/**
 * nss_ipsecmgr_sa_data
 *	Security association information for the IPsec manager.
 */
struct nss_ipsecmgr_sa_data {
	struct nss_ipsecmgr_sa_cmn cmn;		/**< Common configuration information for SA. */

	enum nss_ipsecmgr_sa_type type;		/**< Type of SA. */
	struct nss_ipsecmgr_sa_encap encap;	/**< Information for encap type SA. */
	struct nss_ipsecmgr_sa_decap decap;	/**< Information for decap type SA. */
};

/**
 * nss_ipsecmgr_sa_tuple
 *	SA information for the IPsec manager.
 *
 * Note: Protocol/Next Header defaults to ESP for outer.
 */
struct nss_ipsecmgr_sa_tuple {
	uint32_t src_ip[4];	/**< IPv6 source IP. */
	uint32_t dest_ip[4];	/**< IPv6 destination IP. */
	uint32_t spi_index;	/**< SPI index of the encapsulating security payload (ESP). */
	uint16_t sport;		/**< Source port. */
	uint16_t dport;		/**< Destination port. */
	uint8_t proto_next_hdr;	/**< Protocol (ESP or NAT-T) */
	uint8_t ip_version;	/**< IP version 4/6. */
	uint8_t res[2];		/**< Reserved */
};

/**
 * nss_ipsecmgr_flow_tuple
 *	Flow information for the IPsec manager.
 */
struct nss_ipsecmgr_flow_tuple {
	uint32_t src_ip[4];		/**< Source IP. */
	uint32_t dest_ip[4];		/**< Destination IP. */
	uint32_t spi_index;		/**< ESP SPI index for decapsulation flows. */
	uint16_t sport;			/**< Source Port (unused). */
	uint16_t dport;			/**< Destination Port (unused). */
	uint8_t proto_next_hdr;		/**< Transport layer proto_next_hdr. */
	uint8_t ip_version;		/**< IP version 4/6. */
	uint8_t use_pattern;		/**< User defined flow identifier. */
};

/**
 * nss_ipsecmgr_sa_stats
 *	Security association statistics exported by the IPsec manager.
 */
struct nss_ipsecmgr_sa_stats {
	struct nss_ipsecmgr_sa_tuple sa;	/**< Security association information. */
	uint64_t seq_start;			/**< Starting sequence number. */
	uint64_t seq_cur;			/**< Current sequence number. */
	uint32_t pkt_bytes;			/**< Number of bytes processed. */
	uint32_t pkt_count;			/**< Number of packets processed. */
	uint32_t pkt_failed;			/**< Number of packets failed in processing. */
	uint16_t window_size;			/**< Current size of the window. */
	bool replay_fail_alarm;			/**< Alarm for consecutive hash fail. */
};

/**
 * nss_ipsecmgr_event
 *	Event information for the IPsec manager.
 */
struct nss_ipsecmgr_event {
	enum nss_ipsecmgr_event_type type;		/**< Event type. */
	union {
		struct nss_ipsecmgr_sa_stats stats; 	/**< Security association statistics. */
	} data;						/**< Event information. */
};

/**
 * nss_ipsecmgr_sa_cmn_init_keys
 * 	Fill and initialize common information for SA creation with crypto keys.
 *
 * @datatypes
 * nss_ipsecmgr_sa_cmn \n
 * nss_ipsecmgr_algo \n
 *
 * @param[in/out] cmn       Pointer to the common IPsec manager SA configuration information.
 * @param[in] algo          The AEAD algorithm combination.
 * @param[in] cipher_key    Pointer to the cipher key.
 * @param[in] cipher_keylen Cipher key length.
 * @param[in] auth_key      Pointer to the authentication key.
 * @param[in] auth_keylen   Authentication key length.
 * @param[in] nonce         Pointer to the nonce.
 * @param[in] nonce_size    Size of the nonce.
 * @param[in] hash_len      Length of hash to be computed.
 * @param[in] no_trailer    ESP trailer is required or not.
 * @param[in] esn           Extended Sequence Number is required or not.
 * @param[in] natt          NAT-T is required or not.
 *
 * @return
 * Success or failure.
 */
static inline bool nss_ipsecmgr_sa_cmn_init_keys(struct nss_ipsecmgr_sa_cmn *cmn, enum nss_ipsecmgr_algo algo,
						const uint8_t *cipher_key, uint16_t cipher_keylen,
						const uint8_t *auth_key, uint16_t auth_keylen,
						const uint8_t *nonce, uint16_t nonce_size, uint8_t hash_len,
						bool no_trailer, bool esn, bool natt)
{
	if (algo >= NSS_IPSECMGR_ALGO_MAX)
		return false;

	if (cipher_keylen > NSS_IPSECMGR_CIPHER_KEYLEN_MAX)
		return false;

	if (auth_keylen > NSS_IPSECMGR_AUTH_KEYLEN_MAX)
		return false;

	if (nonce_size > NSS_IPSECMGR_NONCE_SIZE_MAX)
		return false;

	if (!cipher_key || !auth_key)
		return false;

	cmn->algo = algo;
	cmn->keys.cipher_key = cipher_key;
	cmn->keys.auth_key = auth_key;
	cmn->keys.nonce = nonce;

	cmn->keys.cipher_keylen = cipher_keylen;
	cmn->keys.auth_keylen = auth_keylen;
	cmn->keys.nonce_size = nonce_size;

	cmn->icv_len = hash_len;
	cmn->skip_trailer = no_trailer;
	cmn->enable_esn = esn;
	cmn->enable_natt = natt;

	cmn->crypto_has_keys = true;

	return true;
}

/**
 * nss_ipsecmgr_sa_cmn_init_idx
 * 	Fill and initialize common information for SA creation with crypto index or secure offset.
 *
 * @datatypes
 * nss_ipsecmgr_sa_cmn \n
 * nss_ipsecmgr_algo \n
 *
 * @param[in/out] cmn    Pointer to the common IPsec manager SA configuration information.
 * @param[in] algo       The AEAD algorithm combination.
 * @param[in] crypto_idx Crypto session index to be associated with this context.
 * @param[in] hash_len   Length of hash to be computed.
 * @param[in] secure_key Keys used for cipher and authentication are secure or not.
 * @param[in] no_trailer ESP trailer is required or not.
 * @param[in] esn        Extended Sequence Number is required or not.
 * @param[in] natt       NAT-T is required or not.
 *
 * @return
 * Success or failure.
 */
static inline bool nss_ipsecmgr_sa_cmn_init_idx(struct nss_ipsecmgr_sa_cmn *cmn, enum nss_ipsecmgr_algo algo,
						uint16_t crypto_idx,  uint8_t blk_len, uint8_t iv_len,
						uint8_t hash_len, bool secure_key, bool no_trailer, bool esn,
						bool natt)
{
	if (algo >= NSS_IPSECMGR_ALGO_MAX)
		return false;

	cmn->algo = algo;
	cmn->index.session = crypto_idx;
	cmn->index.blk_len = blk_len;
	cmn->index.iv_len = iv_len;

	cmn->icv_len = hash_len;
	cmn->enable_esn = esn;
	cmn->enable_natt = natt;
	cmn->skip_trailer = no_trailer;

	cmn->crypto_has_keys = false;

	return true;
}

#ifdef __KERNEL__ /* only kernel will use. */

/**
 * Callback function for receiving IPsec data.
 *
 * @datatypes
 * sk_buff
 *
 * @param[in] app_data  Pointer to the application data.
 * @param[in] skb       Pointer to the data socket buffer.
 */
typedef void (*nss_ipsecmgr_data_callback_t)(void *app_data, struct sk_buff *skb);

/**
 * Callback function for receiving IPsec events.
 *
 * @datatypes
 * nss_ipsecmgr_event
 *
 * @param[in] app_data  Pointer to the application data.
 * @param[in] ev        Pointer to the event.
 */
typedef void (*nss_ipsecmgr_event_callback_t)(void *app_data, struct nss_ipsecmgr_event *ev);

/**
 * nss_ipsecmgr_callback
 *	Callback information.
 */
struct nss_ipsecmgr_callback {
	void *app_data;				/**< Context of the caller. */
	struct net_device *skb_dev;		/**< Net device to use for Socket Buffer. */
	nss_ipsecmgr_data_callback_t data_cb;	/**< Data callback function. */
	nss_ipsecmgr_event_callback_t event_cb;	/**< Event callback function. */
};

/**
 * nss_ipsecmgr_tunnel_add
 *	Adds a new IPsec tunnel.
 *
 * @datatypes
 * nss_ipsecmgr_callback
 *
 * @param[in] cb  Pointer to the message callback.
 *
 * @return
 * Linux NETDEVICE or NULL.
 */
struct net_device *nss_ipsecmgr_tunnel_add(struct nss_ipsecmgr_callback *cb);

/**
 * nss_ipsecmgr_tunnel_del
 *	Deletes an existing IPsec tunnel.
 *
 * @datatypes
 * net_device
 *
 * @param[in] tun  Pointer to the network device associated with the tunnel.
 *
 * @return
 * Success or failure.
 */
void nss_ipsecmgr_tunnel_del(struct net_device *tun);

/**
 * nss_ipsecmgr_sa_add
 *	Adds a security association to the offload database.
 *
 * @datatypes
 * net_device \n
 * nss_ipsecmgr_sa_tuple \n
 * nss_ipsecmgr_sa_data \n
 *
 * @param[in] tun     Pointer to the network device associated with the tunnel.
 * @param[in] sa      Pointer to tuple representing the SA.
 * @param[in] data    Pointer to the security association data to add.
 * @param[out] if_num Pointer to the IPsec inner or outer interface number.
 *
 * @return
 * nss_ipsecmgr_status.
 */
nss_ipsecmgr_status_t nss_ipsecmgr_sa_add(struct net_device *tun, struct nss_ipsecmgr_sa_tuple *sa,
				struct nss_ipsecmgr_sa_data *data, uint32_t *if_num);

/**
 * nss_ipsecmgr_sa_add_sync
 *	Adds a security association to the offload database synchronously.
 *
 * @datatypes
 * net_device \n
 * nss_ipsecmgr_sa_tuple \n
 * nss_ipsecmgr_sa_data \n
 *
 * @param[in] tun     Pointer to the network device associated with the tunnel.
 * @param[in] sa      Pointer to tuple representing the SA.
 * @param[in] data    Pointer to the security association data to add.
 * @param[out] if_num Pointer to the IPsec inner or outer interface number.
 *
 * @return
 * nss_ipsecmgr_status.
 */
nss_ipsecmgr_status_t nss_ipsecmgr_sa_add_sync(struct net_device *tun, struct nss_ipsecmgr_sa_tuple *sa,
				struct nss_ipsecmgr_sa_data *data, uint32_t *if_num);

/**
 * nss_ipsecmgr_sa_del
 *	Deletes a security association.
 *
 * @datatypes
 * net_device \n
 * nss_ipsecmgr_sa_tuple
 *
 * @param[in] tun   Pointer to the network device associated with the tunnel.
 * @param[in] tuple Pointer to SA tuple to delete.
 *
 * @return
 */
void nss_ipsecmgr_sa_del(struct net_device *tun, struct nss_ipsecmgr_sa_tuple *tuple);

/**
 * nss_ipsecmgr_flow_add
 *	Adds an encapsulation flow rule to the IPsec offload database.
 *
 * @datatypes
 * net_device \n
 * nss_ipsecmgr_flow_tuple \n
 * nss_ipsecmgr_sa_tuple
 *
 * @param[in] tun   Pointer to the network device associated with the tunnel.
 * @param[in] flow  Pointer to the inner flow to add.
 * @param[in] sa    Pointer to the outer flow of the SA to be added to.
 *
 * @return
 * nss_ipsecmgr_status.
 */
nss_ipsecmgr_status_t nss_ipsecmgr_flow_add(struct net_device *tun, struct nss_ipsecmgr_flow_tuple *flow,
					struct nss_ipsecmgr_sa_tuple *sa);

/**
 * nss_ipsecmgr_flow_add_sync
 *	Adds an encapsulation flow rule to the IPsec offload database.
 *
 * @datatypes
 * net_device \n
 * nss_ipsecmgr_flow_tuple \n
 * nss_ipsecmgr_sa_tuple
 *
 * @param[in] tun   Pointer to the network device associated with the tunnel.
 * @param[in] flow  Pointer to the inner flow to add.
 * @param[in] sa    Pointer to the outer flow of the SA to be added to.
 *
 * @return
 * nss_ipsecmgr_status.
 */
nss_ipsecmgr_status_t nss_ipsecmgr_flow_add_sync(struct net_device *tun, struct nss_ipsecmgr_flow_tuple *flow,
					struct nss_ipsecmgr_sa_tuple *sa);

/**
 * nss_ipsecmgr_flow_del
 *	Deletes an encapsulation flow rule from the IPsec offload database.
 *
 * @datatypes
 * net_device \n
 * nss_ipsecmgr_flow_tuple \n
 * nss_ipsecmgr_sa_tuple
 *
 * @param[in] tun   Pointer to the network device associated with the tunnel.
 * @param[in] flow  Pointer to the inner flow to delete.
 * @param[in] sa    Pointer to the SA tuple to be deleted from.
 *
 * @return
 */
void nss_ipsecmgr_flow_del(struct net_device *tun, struct nss_ipsecmgr_flow_tuple *flow,
			struct nss_ipsecmgr_sa_tuple *sa);
#endif /* __KERNEL__ */
#endif /* __NSS_IPSECMGR_H */
