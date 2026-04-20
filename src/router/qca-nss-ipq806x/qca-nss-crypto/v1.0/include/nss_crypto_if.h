/* Copyright (c) 2014-2017, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 */
#ifndef __NSS_CRYPTO_IF_H
#define __NSS_CRYPTO_IF_H

/*
 * @file API interface definitions for crypto driver
 */

#define NSS_CRYPTO_BITS2BYTES(x)	(x / 8)	/**< Bits to Bytes */
#define NSS_CRYPTO_BYTES2BITS(x)	(x * 8)	/**< Bytes to bits */
#define NSS_CRYPTO_MAX_QDEPTH		128	/**< H/W queue depth per pipe */
#define NSS_CRYPTO_MAX_CACHED_IDXS	4	/**< Max supported sessions */
#define NSS_CRYPTO_BAM_PP		2 	/**< BAM Pipe Pairs */
#define NSS_CRYPTO_MAX_NAME		64	/**< Max supported name size */
#define NSS_CRYPTO_BUF_TAILROOM		128	/**< Tailroom required for crypto */

/**
 * @brief Crypto status for all nss_crypto_XXX api's
 */
typedef enum nss_crypto_status {
	NSS_CRYPTO_STATUS_OK,		/**< OK */
	NSS_CRYPTO_STATUS_FAIL,		/**< general failure status */
	NSS_CRYPTO_STATUS_EINVAL,	/**< invalid parameters */
	NSS_CRYPTO_STATUS_EBUSY,	/**< resource unavailable */
	NSS_CRYPTO_STATUS_ERESTART,	/**< resource unavailable, defer the operation */
	NSS_CRYPTO_STATUS_ENOMEM,	/**< out of memory */
	NSS_CRYPTO_STATUS_ENOSUPP,	/**< unsupported configuration */
} nss_crypto_status_t;

/**
 * @brief max key lengths for supported algorithms
 */
enum nss_crypto_max_keylen {
	NSS_CRYPTO_MAX_KEYLEN_AES = 32,		/**< max key size for AES (bytes) */
	NSS_CRYPTO_MAX_KEYLEN_SHA1 = 20,	/**< max key size for SHA1 (bytes) */
	NSS_CRYPTO_MAX_KEYLEN_SHA256 = 32,	/**< max key size for SHA256 (bytes) */
	NSS_CRYPTO_MAX_KEYLEN_DES = 24,		/**< max key size for DES */
};

/**
 * @brief max IV lengths for algorithms supported by the H/W
 */
enum nss_crypto_max_ivlen {
	NSS_CRYPTO_MAX_IVLEN_NULL = 0,		/**< amx IV size for NULL (bytes) */
	NSS_CRYPTO_MAX_IVLEN_DES = 8,		/**< max IV size for DES (bytes) */
	NSS_CRYPTO_MAX_IVLEN_AES = 16,		/**< max IV size for AES (bytes) */
};

/**
 * @brief supported cipher algorithms block lengths
 */
enum nss_crypto_max_blocklen {
	NSS_CRYPTO_MAX_BLKLEN_NULL = 4,		/**< max block length for NULL (bytes) */
	NSS_CRYPTO_MAX_BLKLEN_DES = 8,		/**< max block length for DES (bytes) */
	NSS_CRYPTO_MAX_BLKLEN_AES = 16,		/**< max block length for AES (bytes) */
};

/**
 * @brief max hash generated for a HMAC algorithm.
 */
enum nss_crypto_max_hashlen {
	NSS_CRYPTO_MAX_HASHLEN_SHA1 = 20,	/**< max hash size for SHA1 (bytes) */
	NSS_CRYPTO_MAX_HASHLEN_SHA256 = 32,	/**< max hash size for SHA256 (bytes) */
	NSS_CRYPTO_MAX_HASHLEN = 32,		/**< max hash size (bytes) */
};

/**
 * @brief crypto request type
 */
enum nss_crypto_req_type {
	NSS_CRYPTO_REQ_TYPE_NONE = 0x0,			/**< Reset request */
	NSS_CRYPTO_REQ_TYPE_AUTH = 0x0001,		/**< authentication request */
	NSS_CRYPTO_REQ_TYPE_ENCRYPT = 0x0002,		/**< encryption request*/
	NSS_CRYPTO_REQ_TYPE_DECRYPT = 0x0004,		/**< decryption request*/
};

/**
 * @brief crypto config msg type
 */
enum nss_crypto_config_type {
	NSS_CRYPTO_CONFIG_TYPE_NONE = 0,		/**< No config */
	NSS_CRYPTO_CONFIG_TYPE_OPEN_ENG,		/**< open engine config */
	NSS_CRYPTO_CONFIG_TYPE_CLOSE_ENG,		/**< close engine config */
	NSS_CRYPTO_CONFIG_TYPE_RESET_SESSION,		/**< reset session state config */
	NSS_CRYPTO_CONFIG_TYPE_MAX
};

enum nss_crypto_sync_type {
	NSS_CRYPTO_SYNC_TYPE_NONE = 0,			/**< sync type none */
	NSS_CRYPTO_SYNC_TYPE_OPEN_ENG = 1,		/**< open engine sync */
	NSS_CRYPTO_SYNC_TYPE_CLOSE_ENG = 2,		/**< close engine sync */
	NSS_CRYPTO_SYNC_TYPE_STATS = 3,			/**< stats sync */
	NSS_CRYPTO_SYNC_TYPE_MAX
};


struct nss_crypto_buf;
/**
 * @brief Cipher/Auth operation completion callback function type
 */
typedef void (*nss_crypto_comp_t)(struct nss_crypto_buf *buf);

/**
 * @brief describe a cipher key
 */
struct nss_crypto_key {
	uint32_t algo;			/**< algorithm for Cipher or Auth*/
	uint32_t key_len;		/**< key length */
	uint32_t index;			/**< index of the key stored in secure memory */
	uint8_t *key;			/**< Location of key stored in memory */
};

/**
 * @brief crypto parameters info
 */
struct nss_crypto_params {
	uint16_t cipher_skip;		/**< start encrypt/decrypt from here */
	uint16_t auth_skip;		/**< skip bytes from data to start authenticating */

	uint16_t req_type;		/**< nss_crypto_req_type */
	uint8_t  res[2];		/**< reserve for 4 byte alignment */
};

#ifdef __KERNEL__  /*protect the code which kernel only  uses */

/**
 * @brief crypto request buffer for doing operation with the crypto driver
 *
 *        Buffer elements and its use within data
 *
 *        <-- skip ---><-- cipher len --><-- hash len ->
 *        +------+----+-----------------+--------------+----------------------+
 *        |      | IV |     CIPHER      |     HASH     | extra space for H/W  |
 *        +------+----+-----------------+--------------+----------------------+
 *        <----------------- data len ----------------->
 *      				<------- 128 bytes of tailroom ------->
 */
struct nss_crypto_buf {
	struct nss_crypto_buf *next;	/**< next buffer */

	uint32_t ctx_0; 		/**< private context(0) per buf */
	uint32_t ctx_1;  		/**< private context(1) per buf */
	uint32_t cb_ctx;		/**< completion callback context */

	nss_crypto_comp_t cb_fn;	/**< completion callback function */

	uint32_t data_in;		/**< Data IN address (virtual/physical) */
	uint32_t data_out;		/**< Data OUT address (virtual/physical) */

	uint32_t iv_addr;		/**< location inside data where IV is available */
	uint32_t hash_addr;		/**< location inside data where HASH is generated */

	uint16_t data_len;		/**< Data length */
	uint16_t cipher_len;		/**< Length of data to encrypt */

	uint16_t auth_len;		/**< bytes to authenticate inside data */
	uint16_t session_idx;		/**< session index */

	uint8_t state;  		/**< buffer operation specific state */
	uint8_t origin;			/**< crypto_req originator */
	uint8_t res[2];			/**< reserved for NSS usage */
};


typedef void *nss_crypto_user_ctx_t;	/**< crypto driver user's context */
typedef void *nss_crypto_handle_t;	/**< crypto driver handle for its users */


/**
 * @brief handler called when the crypto device is ready, this is
 * 	  provide the user the crypto handle for future transactions
 */
typedef nss_crypto_user_ctx_t (*nss_crypto_attach_t)(nss_crypto_handle_t crypto);

/**
 * @brief handler called when the crypto device has stopped
 */
typedef void (*nss_crypto_detach_t)(nss_crypto_user_ctx_t ctx);


/**
 * @brief register user attach/detach routines to the crypto driver
 *
 * @param attach[IN] called when device is ready
 * @param detach[IN] called when device has stopped or user has unregistered
 * @param user_name[IN] user name
 */
void nss_crypto_register_user(nss_crypto_attach_t attach, nss_crypto_detach_t detach, uint8_t *user_name);

/**
 * @brief unregister user from the list of crypto device users
 *
 * @param crypto[IN] handle of the crypto device
 *
 * @note detach will be called before this returns
 */
void nss_crypto_unregister_user(nss_crypto_handle_t crypto);

/**
 * @brief get a crypto operation buffer from the pool
 *
 * @param crypto[IN] crypto device handle
 *
 * @return NULL means out of memory
 *
 */
struct nss_crypto_buf *nss_crypto_buf_alloc(nss_crypto_handle_t crypto);

/**
 * @brief return the crypto operation buffer to the pool
 *
 * @param buf[IN] buffer
 */
void nss_crypto_buf_free(nss_crypto_handle_t crypto, struct nss_crypto_buf *buf);

/**
 * @brief Allocate a new session index, this should create the necessary state
 *        across all the layers
 *
 * @param crypto[IN] crypto device handle
 * @param cipher[IN] cipher specific elements {cipher_algo, key & key_length}
 * @param auth[IN] auth specific elememts {auth_algo, key & key_length}
 * @param session_idx[OUT] session index for the crypto transform
 *
 * @return status of the call
 *
 * ENOMEM implies out of index
 * ENOSUPP implies unsupported configuration
 *
 */
nss_crypto_status_t nss_crypto_session_alloc(nss_crypto_handle_t crypto, struct nss_crypto_key *cipher, struct nss_crypto_key *auth,
						uint32_t *session_idx);

/**
 * @brief Allocate a new session index with key information in secure memory
 *        this should create the necessary state across all the layers
 *
 * @param crypto[IN] crypto device handle
 * @param cipher[IN] cipher specific elements {cipher_algo, key_idx & key_length}
 * @param auth[IN] auth specific elememts {auth_algo, key_idx & key_length}
 * @param session_idx[OUT] session index for the crypto transform
 *
 * @return status of the call
 *
 * ENOMEM implies out of index
 * ENOSUPP implies unsupported configuration
 *
 */
nss_crypto_status_t nss_crypto_session_alloc_nokey(nss_crypto_handle_t crypto, struct nss_crypto_key *cipher, struct nss_crypto_key *auth,
						uint32_t *session_idx);

/**
 * @brief update per session global config inforamtion,this API is used configure
 * cipher/authentication skip lengths and crypto mode (encryption/decryption)
 *
 * @param crypto[IN] crypto device handle
 * @param session_idx[IN] session index for the crypto transform
 * @param params[IN] session specific configuration information
 *
 * @return status of the call
 *
 * ENOSUPP implies unsupported configuration
 */
nss_crypto_status_t nss_crypto_session_update(nss_crypto_handle_t crypto, uint32_t session_idx, struct nss_crypto_params *params);

/**
 * @brief Free an existing session, this flushes all state related to the session
 *        including keys, algorithms
 *
 * @param crypto[IN] crypto device handle
 * @param session_idx[IN] session index to free
 *
 * @return status of the call
 *
 * @note When changing/altering the configuration of a session such as new keys,
 *       algorithm etc. the procedure should be to free the older session and
 *       then allocate a newer session with the new parameters
 */
nss_crypto_status_t nss_crypto_session_free(nss_crypto_handle_t crypto, uint32_t session_idx);

/**
 * @brief Apply cipher (as in encrypt or decrypt) and or authenticate the given
 *        buf
 *
 * @param crypto[IN] crypto device handle
 * @param buf[IN] buffer for crypto operation
 *
 * @return status of the call
 * @note completion callback will happen after the crypto operation on the
 *       buffer has completed
 */
nss_crypto_status_t nss_crypto_transform_payload(nss_crypto_handle_t crypto, struct nss_crypto_buf *buf);

/**
 * @brief retrieve the cipher algorithm associated with the session index
 *
 * @param session_idx[IN] session index
 *
 * @return cipher algorithm; for unallocated session the algorithm will be NONE
 */
enum nss_crypto_cipher nss_crypto_get_cipher(uint32_t session_idx);

/**
 * @brief retrieve the cipher block len associated with the session index
 *
 * @param session_idx[IN] session index
 *
 * @return cipher block len; for unallocated session cipher blk len will be zero
 */
enum nss_crypto_max_blocklen nss_crypto_get_cipher_block_len(uint32_t session_idx);

/**
 * @brief retrieve the iv len associated with the session index
 *
 * @param session_idx[IN] session index
 *
 * @return iv len; for unallocated session iv len will be zero
 */
enum nss_crypto_max_ivlen nss_crypto_get_iv_len(uint32_t session_idx);

/**
 * @brief retrieve the cipher key length associated with the session index
 *
 * @param session_idx[IN] session index
 *
 * @return key length; for unallocated sessions the key length will be '0'
 */
uint32_t nss_crypto_get_cipher_keylen(uint32_t session_idx);

/**
 * @brief retrieve the transform type associated with the session index
 *
 * @param session_idx[IN] session index
 *
 * @return transform type
 */
uint32_t nss_crypto_get_reqtype(uint32_t session_idx);

/**
 * @brief retrieve the auth algorithm associated with the session index
 *
 * @param session_idx[IN] session index
 *
 * @return auth algorithm; for unallocated session the algorithm will be NONE
 */
enum nss_crypto_auth nss_crypto_get_auth(uint32_t session_idx);

/**
 * @brief retreive the auth key length associated with the session index
 *
 * @param session_idx[IN] session index
 *
 * @return key length; for unallocated sessions the key length will be '0'
 */
uint32_t nss_crypto_get_auth_keylen(uint32_t session_idx);

/**
 * @brief api for obtaining iv_addr
 *
 * @param buf crypto buffer
 *
 * @return address where iv_addr should be copied
 */
uint8_t *nss_crypto_get_ivaddr(struct nss_crypto_buf *buf);

/**
 * @brief api for obtaining hash address
 *
 * @param buf crypto buffer
 *
 * @return address where hash is generated by crypto hardware
 */
uint8_t *nss_crypto_get_hash_addr(struct nss_crypto_buf *buf);

/**
 * @brief crypto buf get api for callback context
 */
static inline uint8_t *nss_crypto_get_cb_ctx(struct nss_crypto_buf *buf)
{
	return (uint8_t *)buf->cb_ctx;
}

/**
 * @brief crypto buf set api for registering callback
 */
static inline void nss_crypto_set_cb(struct nss_crypto_buf *buf, nss_crypto_comp_t fn, void *ctx)
{
	buf->cb_fn = fn;
	buf->cb_ctx = (uint32_t)ctx;
}

/**
 * @brief crypto buf set api for data in and out addresses
 */
static inline void nss_crypto_set_data(struct nss_crypto_buf *buf, uint8_t *in, uint8_t *out, uint16_t len)
{
	buf->data_in  = (uint32_t)in;
	buf->data_out = (uint32_t)out;
	buf->data_len = len;
}

/**
 * @brief crypto buf set api for session index
 */
static inline void nss_crypto_set_session_idx(struct nss_crypto_buf *buf, uint32_t session_idx)
{
	buf->session_idx = session_idx;
}

/**
 * @brief crypto buf set api for cipher, auth and total len
 */
static inline void nss_crypto_set_transform_len(struct nss_crypto_buf *buf, uint16_t cip_len, uint16_t auth_len)
{
	buf->cipher_len = cip_len;
	buf->auth_len = auth_len;
}

#endif /* __KERNEL__ */
#endif /* __NSS_CRYPTO_IF_H */
