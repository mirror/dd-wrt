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

/*
 * nss_tlsmgr.h
 *	TLS manager interface definitions.
 */
#ifndef _NSS_TLSMGR_H_
#define _NSS_TLSMGR_H_

#define NSS_TLSMGR_REC_MAX 4
#define NSS_TLSMGR_FRAG_MAX 4

#define NSS_TLSMGR_REC_TYPE_CCS 20		/**< TLS packet is change cipher specification. */
#define NSS_TLSMGR_REC_TYPE_ALERT 21		/**< TLS packet is Alert.*/
#define NSS_TLSMGR_REC_TYPE_HANDSHAKE 22	/**< DTLS packet is Handshake.*/
#define NSS_TLSMGR_REC_TYPE_DATA 23		/**< TLS packet is Application data. */

struct nss_tlsmgr_buf;

/**
 * NSS TLS manager status
 */
typedef enum nss_tlsmgr_status {
	NSS_TLSMGR_OK,			/**< Status ok. */
	NSS_TLSMGR_FAIL,		/**< Failed due to unknown reason. */
	NSS_TLSMGR_INVALID_REC_TYPE,	/**< Unsupported Record Type. */
	NSS_TLSMGR_INVALID_ALGO,	/**< Invalid algorithm. */
	NSS_TLSMGR_INVALID_KEYLEN,	/**< Invalid key length for cipher/auth. */
	NSS_TLSMGR_FAIL_REC_VERSION,	/**< Invalid TLS version. */
	NSS_TLSMGR_FAIL_REC_LEN,	/**< Invalid Record length. */
	NSS_TLSMGR_FAIL_NOMEM,		/**< Failed to allocate memory. */
	NSS_TLSMGR_FAIL_NOCRYPTO,	/**< Failed to allocate crypto resource. */
	NSS_TLSMGR_FAIL_MESSAGE,	/**< Failed to message the NSS. */
	NSS_TLSMGR_FAIL_BUF,		/**< Failed to allocate buffer. */
	NSS_TLSMGR_FAIL_REC_RANGE,	/**< Record Index out of range. */
	NSS_TLSMGR_FAIL_LINEARIZE,	/**< Failed to linearize SKB. */
	NSS_TLSMGR_FAIL_DATA_QUEUE,	/**< NSS Queue Congested. */
	NSS_TLSMGR_FAIL_QUEUE_FULL,	/**< Data Enqueue to NSS failed. */
	NSS_TLSMGR_FAIL_TRANSFORM,	/**< Data transformation error. */
} nss_tlsmgr_status_t;

/**
 * NSS TLS manager supported cryptographic algorithms
 */
enum nss_tlsmgr_algo {
	NSS_TLSMGR_ALGO_NULL,			/**< NO Cipher, NO authentication. */
	NSS_TLSMGR_ALGO_NULL_SHA1_HMAC,		/**< NULL_SHA1_HMAC. */
	NSS_TLSMGR_ALGO_NULL_SHA256_HMAC,	/**< NULL_SHA256_HMAC. */
	NSS_TLSMGR_ALGO_AES_CBC_SHA1_HMAC,	/**< AES_CBC_SHA1_HMAC. */
	NSS_TLSMGR_ALGO_AES_CBC_SHA256_HMAC,	/**< AES_CBC_SHA256_HMAC. */
	NSS_TLSMGR_ALGO_3DES_CBC_SHA1_HMAC,	/**< 3DES_CBC_SHA1_HMAC. */
	NSS_TLSMGR_ALGO_3DES_CBC_SHA256_HMAC,	/**< 3DES_CBC_SHA256_HMAC. */
	NSS_TLSMGR_ALGO_MAX
};

/**
 * NSS tls manager per packet stattistics
 */
struct nss_tlsmgr_pkt_stats {
	uint64_t tx_packets;		/**< Packets enqueued to Firmware. */
	uint64_t tx_bytes;		/**< Bytes enqueued to Firmware. */
	uint64_t tx_error;		/**< Error while enqueuing packet to hardware. */
	uint64_t rx_packets;		/**< Packets processed by Firmware. */
	uint64_t rx_bytes;		/**< Bytes processed by Firmware. */
	uint64_t rx_errors;		/**< Error in processing packet. */
};

/**
 * NSS TLS manager statistics
 */
struct nss_tlsmgr_stats {
	struct nss_tlsmgr_pkt_stats encap;	/**< TLS manager encapsulation statistics. */
	struct nss_tlsmgr_pkt_stats decap;	/**< TLS manager decapsulation statistics. */
};

/**
 * NSS tls manager record structure
 */
struct nss_tlsmgr_rec {
	struct scatterlist in[NSS_TLSMGR_FRAG_MAX];		/**< Scatterlist for input data. */
	struct scatterlist out[NSS_TLSMGR_FRAG_MAX];		/**< Scatterlist for output data. */
	uint8_t rec_type;					/**< Record type (20, 21, 22, 23). */
	uint8_t error;						/**< Record error. */
	uint8_t res[2];						/**< Reserved for Alignment. */
};

/**
 * NSS TLS manager callback
 */
typedef void (*nss_tlsmgr_notify_callback_t)(void *app_data, struct net_device *dev, struct nss_tlsmgr_stats *stats);
typedef void (*nss_tlsmgr_decongest_callback_t)(void *app_data, struct net_device *dev);
typedef void (*nss_tlsmgr_data_callback_t)(void *app_data, struct nss_tlsmgr_buf *buf, nss_tlsmgr_status_t status);

/**
 * NSS TLS manager cryptographic structure to represent key and its length.
 */
struct nss_tlsmgr_crypto_data {
	const uint8_t *data;            /**< Pointer to key or nonce. */
	uint16_t len;                   /**< Length of the key. */
};

/**
 * NSS tls manager base config
 */
struct nss_tlsmgr_config {
	struct nss_tlsmgr_crypto_data cipher_key;	/**< Cipher key. */
	struct nss_tlsmgr_crypto_data auth_key;		/**< Authentication key. */
	struct nss_tlsmgr_crypto_data nonce;		/**< Nonce. */
	enum nss_tlsmgr_algo algo;			/**< TLS manager cryptographic algorithm. */
	uint16_t hdr_ver;				/* TLS version 1.1 or 1.2 */
	uint16_t flags;					/* Configuration specific flags */
};

/**
 * nss_tlsmgr_tun_add
 *	Adds a new TLS tunnel.
 *
 * @datatypes
 * nss_tlsmgr_decongest_callback_t \n
 *
 * @param[in]  nss_tlsmgr_decongest_callback_t     Decongestion callback.
 * @param[in]  app_data      Pointer to Application Data.
 *
 * @return
 * Linux NETDEVICE or NULL.
 */
struct net_device *nss_tlsmgr_tun_add(nss_tlsmgr_decongest_callback_t cb, void *app_data);

/**
 * nss_tlsmgr_tun_del
 *	Unregister dynamic interface and deallocate inner and outer
 *	Context. Also unregister the TLS netdevice.
 *
 * @datatypes
 * struct net_device
 *
 * @param[IN] dev TLS network device
 *
 * @return
 */
void nss_tlsmgr_tun_del(struct net_device *tun);

/**
 * nss_tlsmgr_register_notify
 *	Register notification callback
 *
 * @datatypes
 * struct net_device
 * nss_tlsmgr_notify_callback_t
 * uint32_t
 *
 * @param[IN] dev       TLS network device
 * @param[IN] cb        TLS notification callback
 * @param[IN] app_data  Application data
 * @param[IN] msecs     Notificaiton time in milliseconds
 *
 * @return
 * true or false if it is already registered.
 */
bool nss_tlsmgr_register_notify(struct net_device *tun, nss_tlsmgr_notify_callback_t cb, void *app_data, uint32_t msecs);

/**
 * nss_tlsmgr_unregister_notify
 *	Register notification callback
 *
 * @datatypes
 * struct net_device
 *
 * @param[IN] dev TLS network device
 *
 * @return
 */
void nss_tlsmgr_unregister_notify(struct net_device *tun);

/**
 * nss_tlsmgr_crypto_update_encap
 *	Update encapsulation cipher state of a TLS session.
 *	Configures new parameters into the pending encapsulation cipher
 *	state of a TLS session. This has no effect on the current
 *	cipher state and its processing of packets.
 *
 * @datatypes
 * struct net_device \n
 * struct nss_tlsmgr_config \n
 *
 * @param[IN} dev TLS network device
 * @param[IN] cfg TLS crypto update parameters
 *
 * @return NSS_TLSMGR_OK for success
 */
nss_tlsmgr_status_t nss_tlsmgr_crypto_update_encap(struct net_device *dev, struct nss_tlsmgr_config *cfg);

/**
 * nss_tlsmgr_crypto_update_decap
 *	Update decapsulation cipher state of a TLS session.
 *	Configures new parameters into the pending decapsulation cipher
 *	state of a TLS session. This has no effect on the current
 *	cipher state and its processing of packets.
 *
 * @datatypes
 * struct net_device \n
 * struct nss_tlsmgr_config \n
 *
 * @param[IN] dev TLS network device
 * @param[IN] cfg TLS session update parameters
 *
 * @return NSS_TLSMGR_OK for success
 */
nss_tlsmgr_status_t nss_tlsmgr_crypto_update_decap(struct net_device *dev, struct nss_tlsmgr_config *cfg);

/**
 * nss_tlsmgr_tun_get_headroom
 *	TLS buffer headroom requirment.
 *
 * @datatypes
 * struct net_device \n
 *
 * @param[IN] dev TLS network device
 *
 * @return
 * Header length
 *
 * Note: Headroom is the sum of header lenght.
 */
uint16_t nss_tlsmgr_tun_get_headroom(struct net_device *dev);

/**
 * nss_tlsmgr_tun_get_tailroom
 *	TLS buffer tailroom requirment.
 *
 * @datatypes
 * struct net_device \n
 *
 * @param[IN] dev TLS network device
 *
 * @return
 * Header length + Trailer length
 *
 * Note: Tailroom is the sum of header lenght and trailer length.
 */
uint16_t nss_tlsmgr_tun_get_tailroom(struct net_device *dev);

/**
 * nss_tlsmgr_buf_alloc
 *	Allocate TLS buffer
 *
 * Update decapsulation cipher state of a TLS session.
 * Configures new parameters into the pending decapsulation cipher
 * state of a TLS session. This has no effect on the current
 * cipher state and its processing of packets.
 *
 * @datatypes
 *
 * @param[IN] priv User private data
 *
 * @return
 * TLS manager buffer or NULL.
 */
struct nss_tlsmgr_buf *nss_tlsmgr_buf_alloc(struct net_device *dev, void *priv);

/**
 * nss_tlsmgr_buf_free
 *	Free TLS buffer
 *
 * @datatypes
 * struct nss_tlsmgr_buf
 *
 * @param[IN] buf TLS manager buffer to free
 *
 * @return
 * TRUE or FALSE.
 *
 * Note: This does not ensure if any of the SG list is allocated by caller is freed or not.
 */
void nss_tlsmgr_buf_free(struct nss_tlsmgr_buf *buf);

/**
 * nss_tlsmgr_buf2skb
 *	Get a SKB pointer from buffer
 *
 * @datatypes
 *
 * @param[IN] buf TLS buffer
 *
 * @return
 * SKB pointer corresponding to the buffer.
 */
struct sk_buff *nss_tlsmgr_buf2skb(struct nss_tlsmgr_buf *buf);

/**
 * nss_tlsmgr_skb2buf
 *	Get a buf pointer from SKB
 *
 * @datatypes
 *
 * @param[IN] SKB sk_buff
 *
 * @return
 * TLS buffer corresponding to SKB.
 */
struct nss_tlsmgr_buf *nss_tlsmgr_skb2buf(struct sk_buff *skb);

/**
 * nss_tlsmgr_buf_get_priv
 *	Get User private information.
 *
 * @datatypes
 * struct nss_tlsmgr_buf \n
 *
 * @param[in] buf Buffer holding Packet information.
 *
 * @return
 */
void *nss_tlsmgr_buf_get_priv(struct nss_tlsmgr_buf *buf);

/**
 * nss_tlsmgr_buf_get_rec_cnt
 *	Get number of records attached to buffer.
 *
 * @datatypes
 * struct nss_tlsmgr_buf \n
 *
 * @param[in] buf Buffer holding Packet information.
 *
 * @return
 * number of records.
 */
uint8_t nss_tlsmgr_buf_get_rec_cnt(struct nss_tlsmgr_buf *buf);

/**
 * nss_tlsmgr_buf_get_rec
 *	Get a record from a buffer.
 *
 * @datatypes
 * struct nss_tlsmgr_buf \n
 * uint8_t rec_idx
 *
 * @param[in] buf      Buffer holding Packet information.
 * @param[in] rec_idx  Index to a particular record.
 *
 * @return
 * Pointer to a record structure.
 * NULL if the rec_idx is invalid.
 */
struct nss_tlsmgr_rec *nss_tlsmgr_buf_get_rec(struct nss_tlsmgr_buf *buf, uint8_t rec_idx);

/**
 * nss_tlsmgr_buf_set_rec
 *	Set a record in a buffer.
 *
 * @datatypes
 * struct nss_tlsmgr_buf \n
 *
 * @param[in] buf Buffer holding Packet information.
 *
 * @return
 * Pointer to a record structure.
 * NULL if the set is attempted beyond the supported maximum size.
 */
struct nss_tlsmgr_rec *nss_tlsmgr_buf_set_rec(struct nss_tlsmgr_buf *buf, uint8_t in_segs, uint8_t out_segs);

/**
 * nss_tlsmgr_buf_decap_skb2rec
 *	API used to add records to buffer for decapsulation.
 *
 * @datatypes
 * struct nss_tlsmgr_buf \n
 * struct sk_buff
 *
 * @param[in] buf Buffer holding Packet information.
 * @param[in] SKB skb containing payload.
 *
 * @return
 * TRUE or FALSE.
 */
nss_tlsmgr_status_t nss_tlsmgr_buf_decap_skb2recs(struct sk_buff *skb, struct nss_tlsmgr_buf *buf);

/**
 * nss_tlsmgr_buf_encap
 *	API used to schedule TLS encapsulation.
 *
 * @datatypes
 * struct nss_tlsmgr_buf \n
 * nss_tlsmgr_data_callback_t
 *
 * @param[in] buf       Buffer holding Packet information.
 * @param[in] cb        Application data callback handler.
 * @param[in] app_data  Application data.
 *
 * @return
 * NSS_TLSMGR_OK for success
 */
nss_tlsmgr_status_t nss_tlsmgr_buf_encap(struct nss_tlsmgr_buf *buf, nss_tlsmgr_data_callback_t cb, void *app_data);

/**
 * nss_tlsmgr_buf_decap
 *	API used to schedule TLS decapsulation.
 *
 * @datatypes
 * struct nss_tlsmgr_buf \n
 * nss_tlsmgr_data_callback_t \n
 *
 * @param[in] buf   	Buffer holding Packet information.
 * @param[in] cb    	Application data callback handler.
 * @param[in] app_data  Application data.
 *
 * @return
 * NSS_TLSMGR_OK for success
 */
nss_tlsmgr_status_t nss_tlsmgr_buf_decap(struct nss_tlsmgr_buf *buf, nss_tlsmgr_data_callback_t cb, void *app_data);

#endif /* _NSS_TLSMGR_H_ */
