/*
 **************************************************************************
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
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
 * @file nss_nlcryptov2_if.h
 *	NSS Netlink Crypto headers
 */
#ifndef __NSS_NLCRPTOV2_IF_H
#define __NSS_NLCRPTOV2_IF_H

/**
 * @brief Crypto forwarding Family
 */
#define NSS_NLCRYPTOV2_FAMILY "nss_nlcryptov2"
#define NSS_NLCRYPTOV2_MAX_AUTH_KEYLEN 64
#define NSS_NLCRYPTOV2_MAX_CIPHER_KEYLEN 32

/**
 * @brief crypto commands types
 */
enum nss_nlcryptov2_cmd {
	NSS_NLCRYPTOV2_CMD_UNSPEC,			/**< unspecified cmd */
	NSS_NLCRYPTOV2_CMD_CREATE_SESSION,		/**< session add */
	NSS_NLCRYPTOV2_CMD_DESTROY_SESSION,		/**< session delete */
	NSS_NLCRYPTOV2_CMD_RESP_SESSION,		/**< session resp */
	NSS_NLCRYPTOV2_CMD_MAX
};

/**
 * @brief crypto create session
 */
struct nss_nlcryptov2_create_session {
	uint8_t auth_key[NSS_NLCRYPTOV2_MAX_AUTH_KEYLEN];	/**< auth key data */
	uint8_t cipher_key[NSS_NLCRYPTOV2_MAX_CIPHER_KEYLEN];	/**< cipher key data */
	uint16_t sec_key_offset;				/**< secure offset */
	enum nss_crypto_cmn_algo algo;				/**< algorithm */
	bool sec_key;						/**< secure offset is enabled */
};

/**
 * @brief crypto destroy session
 */
struct nss_nlcryptov2_destroy_session {
	uint32_t session_idx;			/**< session index */
};

/**
 * @brief crypto session info
 */
struct nss_nlcryptov2_resp_session {
	uint32_t session_idx;			/**< session index */
};

/**
 * @brief crypto rule
 */
struct nss_nlcryptov2_rule {
	struct nss_nlcmn cm;		/**< common message header */

	union {
		struct nss_nlcryptov2_create_session create;	/**< session create */
		struct nss_nlcryptov2_destroy_session destroy;	/**< session destroy */
		struct nss_nlcryptov2_resp_session resp;	/**< session resp */
	}msg;
};

/**
 * @brief NETLINK Crypto message init
 *
 * @param rule[IN] NSS NETLINK crypto rule
 * @param type[IN] Crypto message type
 */
static inline void nss_nlcryptov2_rule_init(struct nss_nlcryptov2_rule *rule, enum nss_nlcryptov2_cmd type)
{
	nss_nlcmn_set_ver(&rule->cm, NSS_NL_VER);
	nss_nlcmn_init_cmd(&rule->cm, sizeof(struct nss_nlcryptov2_rule), type);
}

#endif /* __NSS_NLCRPTOV2_IF_H */
