/*
 **************************************************************************
 * Copyright (c) 2015,2018-2019, The Linux Foundation. All rights reserved.
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

#ifndef __NSS_NLGRE_REDIR_IF_H
#define __NSS_NLGRE_REDIR_IF_H

#define NSS_NLGRE_REDIR_TUN_TYPE_MAX_SZ 16		/**< Maximum length of tunnel type */
#define NSS_NLGRE_REDIR_MODE_MAX_SZ 16			/**< Maximum length of mode */
#define NSS_NLGRE_REDIR_FAMILY "nss_nlgre_redir"	/**< Family */
#define NSS_NLGRE_REDIR_MCAST_GRP "nss_nlgrerdr_mc"	/**< Multicast group */

/**
 * @brief Enumeration for all command types.
 */
enum nss_nlgre_redir_cmd_type {
	NSS_NLGRE_REDIR_CMD_TYPE_UNKNOWN,		/**< Unknown command type */
	NSS_NLGRE_REDIR_CMD_TYPE_CREATE_TUN,		/**< Create tunnel */
	NSS_NLGRE_REDIR_CMD_TYPE_DESTROY_TUN,		/**< Destroy tunnel */
	NSS_NLGRE_REDIR_CMD_TYPE_MAP,			/**< Map the vap interface to tunnel id */
	NSS_NLGRE_REDIR_CMD_TYPE_UNMAP,			/**< Unmap vap and tunnel id */
	NSS_NLGRE_REDIR_CMD_TYPE_SET_NEXT_HOP,		/**< Set the next hop of inner interface */
	NSS_NLGRE_REDIR_CMD_TYPE_ADD_HASH,		/**< Add a hash entry*/
	NSS_NLGRE_REDIR_CMD_TYPE_DEL_HASH,		/**< Delete a hash entry */
	NSS_NLGRE_REDIR_CMD_TYPE_MAX,			/**< Max number of commands */
};

/**
 * @brief Parameters to create a tunnel.
 */
struct nss_nlgre_redir_create_tun {
	uint32_t sip[4];				/**< Src address of tunnel */
	uint32_t dip[4];				/**< Dest address of tunnel */
	uint32_t ssip[4];				/**< Src address of second tunnel */
	uint32_t sdip[4];				/**< Dest address of second tunnel */
	uint8_t hash_mode;				/**< Indicates how the traffic should be mapped */
	uint8_t iptype;					/**< IPv4 = 1 and IPV6 = 2 */
	char mode[NSS_NLGRE_REDIR_MODE_MAX_SZ];		/**< Mode can be sjack or wifi */
	bool lag_enable;				/**< Indicates whether lag is enabled or not */
	uint8_t res;					/**< Padding to make size multiple of 4 */
};

/**
 * @brief parameters useful for destroying the tunnel
 */
struct nss_nlgre_redir_destroy_tun {
	char netdev[IFNAMSIZ];				/**< Dev to be destroyed */
};

/**
 * @brief parameters to create interface map message.
 */
struct nss_nlgre_redir_map {
	uint32_t ipsec_sa_pattern;				/**< Ipsec association parameters */
	uint16_t rid;						/**< Radio id */
	uint16_t vid;						/**< Vap id */
	char vap_nss_if[IFNAMSIZ];				/**< Vap interface name */
	char tun_type[NSS_NLGRE_REDIR_TUN_TYPE_MAX_SZ];		/**< Tunnel type{tun, dtun, split} */
};

/**
 * @brief parameters used to unmap the device
 */
struct nss_nlgre_redir_unmap {
	char vap_nss_if[IFNAMSIZ];			/**< Dev name to be unmapped */
	uint16_t rid;					/**< Radio id */
	uint16_t vid;					/**< Vap id */
	uint8_t res[2];					/**< Reserve for padding purpose */
};

/**
 * @brief parameters used to set the next hop
 */
struct nss_nlgre_redir_set_next {
	char dev_name[IFNAMSIZ];			/**< Dev whose next hop to be set */
	char next_dev_name[IFNAMSIZ];			/**< Dev set as the set next */
	char mode[NSS_NLGRE_REDIR_MODE_MAX_SZ];		/**< Split or wifi */
	uint8_t res[2];					/**< Reserve for padding purpose */
};

/**
 * @brief parameters to perform hash operations.
 */
struct nss_nlgre_redir_hash_ops {
	uint16_t slave;					/**< Tunnel to which the traffic should be mapped */
	uint8_t smac[6];				/**< Source mac address */
	uint8_t dmac[6];				/**< Destination mac address */
	char mode[NSS_NLGRE_REDIR_MODE_MAX_SZ];    	/**< Sjack or wifi */
};

/**@}*/

/**
 * @brief gre_redir message
 */
struct nss_nlgre_redir_rule {
	struct nss_nlcmn cm;				/**< Common message header */

	/**
	 * @brief payload of an GRE_REDIR netlink msg
	 */
	union {
		struct nss_nlgre_redir_create_tun create;	/**< Create tunnel */
		struct nss_nlgre_redir_destroy_tun destroy;	/**< Destroy tunnel */
		struct nss_nlgre_redir_map map;			/**< Map the interface */
		struct nss_nlgre_redir_unmap unmap;		/**< Unmap the interface */
		struct nss_nlgre_redir_set_next snext;		/**< Set next hop */
		struct nss_nlgre_redir_hash_ops hash_ops;	/**< Add and del hash value(s) */
	} msg;
};

/**
 * @brief NETLINK gre_redir message init
 * @param rule[IN] NSS NETLINK gre_redir message
 * @param type[IN] gre_redir message type
 */
static inline void nss_nlgre_redir_rule_init(struct nss_nlgre_redir_rule *rule, enum nss_nlgre_redir_cmd_type type)
{
	nss_nlcmn_set_ver(&rule->cm, NSS_NL_VER);
	nss_nlcmn_init_cmd(&rule->cm, sizeof(struct nss_nlgre_redir_rule), type);
}

/**@}*/
#endif /* __NSS_NLGRE_REDIR_IF_H */
