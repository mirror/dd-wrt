/*
 * Copyright (c) 2017-2018, 2020, The Linux Foundation. All rights reserved.
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
 */

#ifndef __NSS_CRYPTO_DEFINES_H
#define __NSS_CRYPTO_DEFINES_H

#define NSS_CRYPTO_NAME_MAX 128 /* bytes */
#define NSS_CRYPTO_SESSION_MAX U16_MAX

/**
 * nss_crypto_op_dir
 *	crypto operation direction
 */
enum nss_crypto_op_dir {
	NSS_CRYPTO_OP_DIR_NONE = 0,		/**< Bypass */
	NSS_CRYPTO_OP_DIR_ENC,			/**< Outbound encrypt */
	NSS_CRYPTO_OP_DIR_DEC,			/**< Inbound encrypt */
	NSS_CRYPTO_OP_DIR_AUTH,			/**< Authentication */
	NSS_CRYPTO_OP_DIR_ENC_AUTH,		/**< Outbound encrypt + auth */
	NSS_CRYPTO_OP_DIR_AUTH_DEC,		/**< Inbound auth + decrypt */
	NSS_CRYPTO_OP_DIR_AUTH_ENC,		/**< Outbound auth + encrypt */
	NSS_CRYPTO_OP_DIR_DEC_AUTH,		/**< Inbound decrypt + auth */
	NSS_CRYPTO_MAX_OP_DIR
};

struct nss_crypto_user;
struct nss_crypto_hdr;

/**
 * nss_crypto_user_method_t
 *	user operation method type
 *
 * @datatype
 * nss_crypto_user
 *
 * @param[in] app_data context for the user
 * @param[in] user user of nss_crypto
 *
 * @return
 * None
 */
typedef void (*nss_crypto_user_method_t)(void *app_data, struct nss_crypto_user *user);

/**
 * nss_crypto_req_callback_t
 *	request completion callback
 *
 * @datatype
 * nss_crypto_hdr
 *
 * @param[in] app_data context of the callback
 * @param[in] ch crypto header
 * @param[in] error error filled by FW
 *
 * @return
 * None
 */
typedef void (*nss_crypto_req_callback_t)(void *app_data, struct nss_crypto_hdr *ch, uint8_t error);

/**
 * nss_crypto_user_ctx
 *	user context
 */
struct nss_crypto_user_ctx {
	char name[NSS_CRYPTO_NAME_MAX];		/**< Name of the user */
	nss_crypto_user_method_t attach;	/**< Attach user to crypto */
	nss_crypto_user_method_t detach;	/**< Deach user from crypto */
	uint16_t hdr_pool_sz;			/**< Parameter to set nss_crypto_hdr for a particular user */
	uint16_t default_hdr_sz;		/**< Maximum size of crypto header for the user */
	unsigned long timeout_ticks;			/**< Timeout in ticks for delaying session free */
};

/**
 * nss_crypto_user
 *	user handle
 */
struct nss_crypto_user {
	struct sk_buff_head sk_head;		/**< List of crypto headers */
	struct list_head list;			/**< User list */
	spinlock_t lock;			/**< Lock */
	struct nss_crypto_user_ctx ctx;		/**< User context */
	struct kref ref;			/**< Reference object */
	void *app_data;				/**< App data for user */
	bool active;				/**< User is valid */
};

/**
 * nss_crypto_session_data
 *	session data
 */
struct nss_crypto_session_data {
	const uint8_t *cipher_key;		/**< Cipher key */
	const uint8_t *auth_key;		/**< Authentication key */
	const uint8_t *nonce;			/**< Nonce */
	uint16_t auth_keylen;			/**< Authentication key length */
	uint16_t sec_key_offset;		/**< Secure offset */
	enum nss_crypto_cmn_algo algo;		/**< Algorithm */
	bool sec_key;				/**< Secure offset is enabled */
};
#endif
