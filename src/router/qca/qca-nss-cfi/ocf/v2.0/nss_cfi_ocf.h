/* Copyright (c) 2014, 2017, The Linux Foundation. All rights reserved.
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

/**
 * nss_cfi_ocf.h
 * 	ocf specific nss cfi header file
 */

/**
 * @file Interface to communicate OCF specific data to Core specific data.
 */

#define NSS_CFI_DRV_NAME		"nss_cfi"
#define NSS_CFI_OCF_SES_MASK		0xffff
#define NSS_CFI_OCF_SESSION(_sid)	((_sid) & NSS_CFI_OCF_SES_MASK)

#define NSS_CFI_OCF_ALGO_SHIFT(algo)	(algo << 16)
#define NSS_CFI_OCF_GET_ALGO(sid)	((sid >> 16) & 0xFFFF)

#define NSS_CFI_OCF_BITS2BYTES(x) (x / 8) /**< Bits to Bytes */

/**
 * @brief Algorithm types supported by CFI layer
 */
enum nss_cfi_ocf_algo_type {
	NSS_CFI_OCF_ALGO_TYPE_IS_CIPHER = 1,	/**< Cipher algorithm */
	NSS_CFI_OCF_ALGO_TYPE_IS_AUTH,  	/**< Authentication algorithm */
	NSS_CFI_OCF_ALGO_TYPE_IS_CIPHER_AUTH,  	/**< AEAD algorithm */
	NSS_CFI_OCF_ALGO_TYPE_IS_AUTH_CIPHER,  	/**< AEAD algorithm */
};

/**
 * @brief Framework specific handle this will be used to communicate framework
 *        specific data to Core specific data
 */
struct nss_cfi_ocf {
	softc_device_decl dev;			/**< OCF crypto device instance */
	int32_t cid;				/**< crypto tag */
	struct nss_crypto_user *crypto;		/**< crypto handle */
	struct nss_crypto_user_ctx *user_ctx;	/**< OCF user context */

	nss_cfi_data_trap_t encrypt_fn;		/**< IPsec encryption trap function, Encap direction */
	nss_cfi_data_trap_t decrypt_fn;		/**< IPsec decryption trap function, Decap direction */
	nss_cfi_session_trap_t session_fn;	/**< IPsec session trap function */
};


/**
 * @brief Structure for storing Algorithm specific info
 */
struct nss_cfi_ocf_algo_map {
	uint32_t algo[2];			/**< ALGO provided by cfi */
	uint32_t max_keylen;			/**< max keylen supported by algo */
	uint32_t max_ivlen;			/**< max ivlen supported by algo */
	uint32_t max_blklen;			/**< max blklen supported by algo */
	uint32_t max_hashlen;			/**< max hashlen supported by algo */
	bool supported;				/**< Determine whether algo is supported or not */
};

/**
 * @brief initialize cfi and crypto core driver
 */
int nss_cfi_ocf_init(void);

/**
 *  @brief De-Initialize OCF CFI layer
 */
void nss_cfi_ocf_exit(void);


