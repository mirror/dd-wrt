/*
 **************************************************************************
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

#ifndef _NSS_CONNMGR_GRE_PUBLIC_H_
#define _NSS_CONNMGR_GRE_PUBLIC_H_

/**
 * @brief tap/tun enums
 *
 * Only tap is supported
 */
enum nss_connmgr_gre_mode {
	GRE_MODE_TUN,		/**< Tun interface */
	GRE_MODE_TAP,		/**< Tap interface */
	GRE_MODE_MAX		/**< Max type */
};

/**
 * @brief ipv4/ipv6 tunnel
 *
 * Only ipv4 is supported
 */
enum nss_connmgr_gre_ip_type {
	GRE_OVER_IPV4,		/**< IPv4 tunnel */
	GRE_OVER_IPV6,		/**< IPv6 tunnel */
};

/**
 * @brief GRE err codes
 *
 * Error codes
 */
enum nss_connmgr_gre_err_codes {
	GRE_SUCCESS = 0,			/**< Success */
	GRE_ERR_ALLOC_NETDEV = 1,		/**< Alloc netdevice failed */
	GRE_ERR_NO_NETDEV = 2,			/**< No netdevice found */
	GRE_ERR_NO_LOCAL_NETDEV = 3,		/**< No netdevice found for corresonding src IP address in cfg */
	GRE_ERR_NO_NEXT_NETDEV = 4,		/**< No netdevice found for next device in cfg */
	GRE_ERR_RADDR_ROUTE_LOOKUP = 5,		/**< Route look up failed for destination IP address  */
	GRE_ERR_NOT_GRE_NETDEV = 6,		/**< Netdevice is not of type GRE */
	GRE_ERR_NETDEV_REG_FAILED = 7,		/**< Netdevice registration failed */
	GRE_ERR_INVALID_MODE = 8,		/**< Invalid GRE mode */
	GRE_ERR_INVALID_MAC = 9,		/**< Invalid MAC */
	GRE_ERR_NEIGH_CREATE = 10,		/**< Error in creating neigh entry for IP */
	GRE_ERR_NEIGH_LOOKUP = 11,		/**< Neighbour lookup failed */
	GRE_ERR_NEIGH_DEV_LOOPBACK = 12,	/**< Neighbour dev is loopback device */
	GRE_ERR_NEIGH_DEV_NOARP = 13,		/**< Neighbour dev is no ARP device */
	GRE_ERR_INVALID_IP = 14,		/**< Invalid IP address */
	GRE_ERR_NODE_UNREG_IN_AE = 15,		/**< Node is not recognized by NSS */
	GRE_ERR_NEXT_NODE_UNREG_IN_AE = 16,	/**< Next node is not registered in NSS */
	GRE_ERR_DYNMAIC_IFACE_CREATE = 17,	/**< Dynamic interface creation failed */
	GRE_ERR_DYNMAIC_IFACE_DESTROY = 18,	/**< Dynamic interface destroy failed */
	GRE_ERR_GRE_IFACE_REG = 19,		/**< GRE iface register with NSS drv failed */
	GRE_ERR_AE_CONFIG_FAILED = 20,		/**< AE engine config faild */
	GRE_ERR_AE_DECONFIG_FAILED = 21,	/**< AE engine delete command faild */
	GRE_ERR_AE_SET_NEXT_HOP = 22,		/**< Set next hop in AE failed */
	GRE_ERR_UNSUPPORTED_CFG = 23,		/**< Unsupported configuration */
	GRE_ERR_IN_INTERRUPT_CTX = 24,		/**< APIs invoked in interrupt context */
	GRE_ERR_MAX
};

/**
 * @brief User config structure
 *
 * User of this client driver needs to fill in this structure and call
 * nss_connmgr_gre_create_interface() API to create GRE tap interface.
 * There is no support for Keys, Sequence number and checksum.
 */
struct nss_connmgr_gre_cfg {
	enum nss_connmgr_gre_mode mode;		/**< GRE modes. (Mandatory Field) */
	enum nss_connmgr_gre_ip_type ip_type;	/**< IP types (Mandatory field) */

	bool set_df;				/**< Set DF flag ? (Optional field) */
	bool ikey_valid, okey_valid;		/**< Take care of keys ? (No support) */
	bool iseq_valid, oseq_valid;		/**< Take care of sequence number ? (No support) */
	bool icsum_valid, ocsum_valid;		/**< Take care of checksum ? (No support)*/
	bool tos_inherit;			/**< Inherit TOS ? (Optional) */
	bool ttl_inherit;			/**< Interit TTL ? (Optional) */
	bool use_mac_hdr;			/**< Add MAC header which is provided (Optional Field)*/
	bool add_padding;			/**< Add padding to make GRE 4 byte aligned ? (Optional Field) */
	bool copy_metadata;			/**< Copy metadata during alignment ? (Optional Field) */
	bool is_ipv6;				/**< Set if addr is IPv6 (Mandatory Field)*/

	uint32_t src_ip[4];			/**< Src IP address (Mandatory Field) */
	uint32_t dest_ip[4];			/**< Dest IP address (Mandatory Field)*/

	uint16_t src_mac[3];			/**< Src MAC address (Depends on use_mac_hdr field) */
	uint16_t dest_mac[3];			/**< Dest MAC address (Depends on use_mac_hdr field) */

	uint32_t ikey;				/**< In Key (No support) */
	uint32_t okey;				/**< Out Key (No support) */

	struct net_device *next_dev;		/**< Next hop netdevice (Mandatory Field) */
	char *name;				/**< Name of GRE Tap interface (Optional Field) */

	uint8_t ttl;				/**< Time to Live (Depends on ttl_inherit field) */
	uint8_t tos;				/**< Type of service (Depends on tos_inherit field) */
	uint16_t reserved;			/**< Reserved for future use */
};

/**
 * @brief GRE interface Create/Delete API
 *
 * API to create/delete GRE interface. These API should not
 * be invoked in interrupt/softirq context
 */
struct net_device *nss_connmgr_gre_create_interface(struct nss_connmgr_gre_cfg *cfg, enum nss_connmgr_gre_err_codes *err_code);
enum nss_connmgr_gre_err_codes nss_connmgr_gre_destroy_interface(struct net_device *dev);

#endif
