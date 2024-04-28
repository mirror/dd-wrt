/*
 **************************************************************************
 * Copyright (c) 2015-2016,2018 The Linux Foundation. All rights reserved.
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
 * @file nss_nlcrypto_if.h
 *	NSS Netlink Crypto headers
 */
#ifndef __NSS_NLCRPTO_IF_H
#define __NSS_NLCRPTO_IF_H

/**
 * @brief Crypto forwarding Family
 */
#define NSS_NLCRYPTO_MAX_KEYLEN 32

/**
 * @brief crypto commands types
 */
enum nss_nlcrypto_cmd {
	NSS_NLCRYPTO_CMD_UNSPEC,			/**< unspecified cmd */
	NSS_NLCRYPTO_CMD_CREATE_SESSION,		/**< session add */
	NSS_NLCRYPTO_CMD_UPDATE_SESSION,		/**< session update */
	NSS_NLCRYPTO_CMD_DESTROY_SESSION,		/**< session delete */
	NSS_NLCRYPTO_CMD_INFO_SESSION,	        	/**< session info */
	NSS_NLCRYPTO_CMD_CREATE_SESSION_NOKEY,  	/**< session add nokey */
	NSS_NLCRYPTO_CMD_MAX
};

/**
 * @brief crypto create session
 */
struct nss_nlcrypto_create_session {
	struct nss_crypto_key cipher;			/**< cipher key information */
	struct nss_crypto_key auth;			/**< auth key information */
	uint8_t cipher_key[NSS_NLCRYPTO_MAX_KEYLEN];	/**< cipher key data */
	uint8_t auth_key[NSS_NLCRYPTO_MAX_KEYLEN];	/**< auth key data */
	enum nss_crypto_cmn_algo algo;			/**< cipher algorithm */
};

/**
 * @brief crypto destroy session
 */
struct nss_nlcrypto_destroy_session {
	uint32_t session_idx;			/**< session index */
};

/**
 * @brief crypto update session
 */
struct nss_nlcrypto_update_session {
	uint32_t session_idx;			/**< session index */
	struct nss_crypto_params param;		/**< crypto parameters */
};

/**
 * @brief crypto session info
 */
struct nss_nlcrypto_info_session {
	uint32_t session_idx;			/**< session information */

	struct {
		uint16_t algo;			/**< algorithm */
		uint16_t key_len;		/**< key length */
	} cipher, auth;
};

/**
 * @brief crypto rule
 */
struct nss_nlcrypto_rule {
	struct nss_nlcmn cm;		/**< common message header */

	union {
		struct nss_nlcrypto_create_session create;	/**< session create */
		struct nss_nlcrypto_update_session update;	/**< session update */
		struct nss_nlcrypto_destroy_session destroy;	/**< session destroy */
		struct nss_nlcrypto_info_session info;		/**< session info */
	}msg;
};

/**
 * @brief NETLINK Crypto message init
 *
 * @param rule[IN] NSS NETLINK crypto rule
 * @param type[IN] Crypto message type
 */
static inline void nss_nlcrypto_rule_init(struct nss_nlcrypto_rule *rule, enum nss_nlcrypto_cmd type)
{
	nss_nlcmn_set_ver(&rule->cm, NSS_NL_VER);
	nss_nlcmn_init_cmd(&rule->cm, sizeof(struct nss_nlcrypto_rule), type);
}

#endif /* __NSS_NLCRPTO_IF_H */
