/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#ifndef _NSS_CONNMGR_TUNIPIP6_
#define _NSS_CONNMGR_TUNIPIP6_

/*
 * Error codes.
 */
enum nss_connmgr_tunipip6_err_codes {
	NSS_CONNMGR_TUNIPIP6_SUCCESS,			/**< Success. */
	NSS_CONNMGR_TUNIPIP6_TUN_CREATE_FAILURE,	/**< Tunnel create failure. */
	NSS_CONNMGR_TUNIPIP6_TUN_DESTROY_FAILURE,	/**< Tunnel destroy failure. */
	NSS_CONNMGR_TUNIPIP6_TUN_NONE,			/**< Invalid tunnel type */
	NSS_CONNMGR_TUNIPIP6_NETDEV_TYPE_FAILURE,	/**< Netdevice is not of type ipv6-in-ipv4. */
	NSS_CONNMGR_TUNIPIP6_MAPRULE_ADD_FAILURE,	/**< BMR/FMR addition failure. */
	NSS_CONNMGR_TUNIPIP6_MAPRULE_DEL_FAILURE,	/**< BMR/FMR deletion failure. */
	NSS_CONNMGR_TUNIPIP6_FMR_RULE_FLUSH_FAILURE,	/**< FMR flush failure. */
	NSS_CONNMGR_TUNIPIP6_NO_DEV,			/**< No NSS node found. */
	NSS_CONNMGR_TUNIPIP6_INVALID_PARAM,		/**< Invalid tunnel parameters. */
	NSS_CONNMGR_TUNIPIP6_INVALID_RULE_TYPE,		/**< Invalid maprule type. */
	NSS_CONNMGR_TUNIPIP6_CONTEXT_FAILURE,		/**< Tunnel host context not found. */
};

/*
 * IPIP6 tunnel types.
 */
enum nss_connmgr_tunipip6_type {
	NSS_CONNMGR_TUNIPIP6_TUNNEL_4RD,		/**< Tunnel type 4RD. */
	NSS_CONNMGR_TUNIPIP6_TUNNEL_MAPE,		/**< Tunnel type MAP-E. */
	NSS_CONNMGR_TUNIPIP6_TUNNEL_MAPE_DRAFT03,	/**< Tunnel type MAP-E with draft03 enable. */
};

/*
 * IPIP6 maprule types.
 */
typedef enum nss_connmgr_tunipip6_maprule_type {
	NSS_CONNMGR_TUNIPIP6_RULE_BMR = 1,
	NSS_CONNMGR_TUNIPIP6_RULE_FMR,
} nss_connmgr_tunipip6_maprule_type_t;

/**
 * @brief User config structure
 *
 * User of this client driver needs to fill in this structure and call
 * nss_connmgr_tunipip6_create_interface() API to create IPIP6 interface in NSS.
 */
struct nss_connmgr_tunipip6_tunnel_cfg {
	uint32_t saddr[4];				/**< Tunnel source address. */
	uint32_t daddr[4];				/**< Tunnel destination address. */
	uint32_t flowlabel;				/**< Tunnel IPv6 flow label. */
	uint32_t flags;					/**< Tunnel additional flags. */
	uint32_t sibling_if_num;			/**< Sibling interface number. */
	enum nss_connmgr_tunipip6_type tunnel_type;	/**< Tunnel type. */
	uint8_t hop_limit;				/**< Tunnel IPv6 hop limit. */
	bool ttl_inherit;				/**< Inherit IPv4 ttl to hoplimit. */
	bool tos_inherit;				/**< Inherit IPv4 tos. */
	bool frag_id_update;				/**< Enable fragment ID support. Applicable for MAP-E/4RD only. */
	uint32_t fmr_max;				/**< Maximum number of FMR that can be configured. */
};

/**
 * @brief User config structure
 *
 * User of this client driver needs to fill in this structure and call
 * the below function to add/delete FMR rules.
 *
 * Add:	nss_connmgr_tunipip6_add_maprule()
 * Delete: nss_connmgr_tunipip6_del_maprule()
 */
struct nss_connmgr_tunipip6_maprule_cfg {
	nss_connmgr_tunipip6_maprule_type_t rule_type;	/**< Rule type. */
	uint32_t ipv6_prefix[4];		/**< IPv6 prefix assigned by a mapping rule. */
	uint32_t ipv6_prefix_len;		/**< IPv6 prefix length. */
	uint32_t ipv4_prefix;			/**< IPv4 prefix assigned by a mapping rule. */
	uint32_t ipv4_prefix_len;		/**< IPv4 prefix length. */
	uint32_t ipv6_suffix[4];		/**< IPv6 suffix. */
	uint32_t ipv6_suffix_len;		/**< IPv6 suffix length. */
	uint32_t ea_len;			/**< Embedded Address (EA) bits. */
	uint32_t psid_offset;			/**< PSID offset default 6. */
};

enum nss_connmgr_tunipip6_err_codes nss_connmgr_tunipip6_create_interface(struct net_device *netdev, struct nss_connmgr_tunipip6_tunnel_cfg *tnlcfg);
enum nss_connmgr_tunipip6_err_codes nss_connmgr_tunipip6_destroy_interface(struct net_device *netdev);
enum nss_connmgr_tunipip6_err_codes nss_connmgr_tunipip6_add_maprule(struct net_device *netdev, struct nss_connmgr_tunipip6_maprule_cfg *rulecfg);
enum nss_connmgr_tunipip6_err_codes nss_connmgr_tunipip6_del_maprule(struct net_device *netdev, struct nss_connmgr_tunipip6_maprule_cfg *rulecfg);
enum nss_connmgr_tunipip6_err_codes nss_connmgr_tunipip6_flush_fmr_rule(struct net_device *netdev);
#endif
