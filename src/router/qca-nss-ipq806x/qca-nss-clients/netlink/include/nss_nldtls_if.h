/*
 **************************************************************************
 * Copyright (c) 2015,2018-2020 The Linux Foundation. All rights reserved.
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
 * @file nss_nldtls_if.h
 *	NSS Netlink Dtls headers
 */
#ifndef __NSS_NLDTLS_IF_H
#define __NSS_NLDTLS_IF_H

#include <nss_nlcmn_if.h>

/**
 * @brief Dtls definitions
 */
#define NSS_NLDTLS_FAMILY "nss_nldtls"
#define NSS_NLDTLS_MCAST_GRP "nss_nldtls_mc"
#define NSS_NLDTLS_MODE_MAX_SZ 16
#define NSS_NLDTLS_PROTO_MAX_SZ 16
#define NSS_NLDTLS_ENCAP_SIDE 0
#define NSS_NLDTLS_DECAP_SIDE 1
#define NSS_NLDTLS_TX_PKTS_MODE_END_TO_END 0
#define NSS_NLDTLS_TX_PKTS_MODE_HOST_TO_HOST 1
#define NSS_NLDTLS_CIPHER_KEY_MAX 32
#define NSS_NLDTLS_AUTH_KEY_MAX 64
#define NSS_NLDTLS_NONCE_SIZE_MAX 4

/**
 * @brief Enumeration for all command types.
 */
enum nss_nldtls_cmd_type {
	NSS_NLDTLS_CMD_TYPE_UNKNOWN,			/**< Unknown command type */
	NSS_NLDTLS_CMD_TYPE_CREATE_TUN,			/**< Creates a tunnel. */
	NSS_NLDTLS_CMD_TYPE_DESTROY_TUN,		/**< Destroys the tunnel created. */
	NSS_NLDTLS_CMD_TYPE_UPDATE_CONFIG,		/**< Updates the configuration of the dtls tunnel. */
	NSS_NLDTLS_CMD_TYPE_TX_PKTS,			/**< Helps in configuring parameters for sending traffic. */
	NSS_NLDTLS_CMD_TYPE_MAX				/**< Max number of commands type. */
};

/**
 * @brief Parameters for crypto keys
 */
struct nss_nldtls_crypto_keys {
	uint8_t cipher[NSS_NLDTLS_CIPHER_KEY_MAX];	/**< Cipher key data */
	uint8_t auth[NSS_NLDTLS_CIPHER_KEY_MAX];	/**< Cipher key data */
	uint8_t nonce[NSS_NLDTLS_CIPHER_KEY_MAX];	/**< Cipher key data */
};

/**
 * @brief Parameters for encap configuration
 */
struct nss_nldtls_encap_config {
	struct nss_dtlsmgr_encap_config cfg;
	struct nss_nldtls_crypto_keys keys;
};

/**
 * @brief Parameter for decap configuration
 */
struct nss_nldtls_decap_config {
	struct nss_dtlsmgr_decap_config cfg;
	struct nss_nldtls_crypto_keys keys;
};

/**
 * @brief Parameters to create a tunnel.
 */
struct nss_nldtls_create_tun {
	struct nss_nldtls_encap_config encap;	/**< Encap data. */
	struct nss_nldtls_decap_config decap;	/**< Decap data. */
	uint32_t flags;				/**< DTLS header flags. */
	uint32_t from_mtu;			/**< Mtu of incoming interface. */
	uint32_t to_mtu;			/**< Mtu of outgoing interface. */
	uint8_t ip_version;			/**< Version of IP address */
	char gmac_ifname[IFNAMSIZ];		/**< WAN interface name */
	uint8_t gmac_ifmac[ETH_ALEN];		/**< Mac address of the wan interface */
	uint8_t dir;				/**< Encap or decap side. */
};

/**
 * @brief parameters useful for destroying the tunnel
 */
struct nss_nldtls_destroy_tun {
	char dev_name[IFNAMSIZ];	/**< Device name associated with tunnel */
};

/**
 * @brief parameters useful for updating the tunnel configuration
 */
struct nss_nldtls_update_config {
	struct nss_dtlsmgr_config_update config_update;		/**< Update config params */
	struct nss_nldtls_crypto_keys keys;			/**< Crypto keys. */
	uint16_t epoch;						/**< Dtls encap epoch. */
	uint16_t window_sz;					/**< Dtls window size parameter. */
	char dev_name[IFNAMSIZ];				/**< Device whose config to be updated. */
	uint8_t dir;						/**< Encap or decap side. */
};

/**
 * @brief parameters to send traffic.
 */
struct nss_nldtls_tx_pkts {
	uint32_t num_pkts;		/**< Number of packets to be transmitted */
	uint32_t seq_num;		/**< starting sequence number */
	uint16_t pkt_sz;		/**< Size of packet to be transmitted */
	char dev_name[IFNAMSIZ];	/**< Device used for transmission */
	uint8_t mode;			/**< Can be end_to_end or host_to_host*/
	uint8_t ctype;			/**< dtls content type */
	bool log_en;			/**< Enable or disable wireless info */
};

/**
 * @brief dtls message
 */
struct nss_nldtls_rule {
	struct nss_nlcmn cm;		/**< Common message header */

	/**
	 * @brief payload of an DTLS netlink msg
	 */
	union {
		struct nss_nldtls_create_tun create;			/**< Create tunnel */
		struct nss_nldtls_destroy_tun destroy;			/**< Destroy tunnel */
		struct nss_nldtls_tx_pkts tx_pkts;			/**< Send traffic*/
		struct nss_nldtls_update_config update_config;		/**< Update configuration*/
	} msg;
};

/**
 * @brief NETLINK dtls rule init
 * @param rule[IN] NSS NETLINK dtls rule
 * @param type[IN] dtls command type
 */
static inline void nss_nldtls_rule_init(struct nss_nldtls_rule *rule, enum nss_nldtls_cmd_type type)
{
	nss_nlcmn_set_ver(&rule->cm, NSS_NL_VER);
	nss_nlcmn_init_cmd(&rule->cm, sizeof(struct nss_nldtls_rule), type);
}

/**@}*/
#endif /* __NSS_NLDTLS_IF_H */
