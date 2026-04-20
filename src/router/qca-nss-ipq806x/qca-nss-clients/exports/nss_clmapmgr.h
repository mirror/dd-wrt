/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

/**
 * nss_clmapmgr.h
 *	Client map manager for NSS
 */
#ifndef __NSS_CLMAPMGR_H
#define __NSS_CLMAPMGR_H

#define NSS_CLMAPMGR_FLAG_VLAN_VALID 0x00000001

/**
 * Clmapmgr status enums.
 */
typedef enum nss_clmapmgr_status {
	NSS_CLMAPMGR_SUCCESS,				/**< Success. */
	NSS_CLMAPMGR_ERR_ALLOC_NETDEV_FAILED,		/**< Alloc netdevice failed. */
	NSS_CLMAPMGR_ERR_NSSIF_UNREGISTER_FAILED,	/**< Unregister netdevice failed. */
	NSS_CLMAPMGR_ERR_NETDEV_UNKNOWN,		/**< Netdev is not recognized by NSS. */
	NSS_CLMAPMGR_ERR_NSSIF_DEALLOC_FAILED,		/**< Dynamic interface destroy failed. */
	NSS_CLMAPMGR_ERR_BAD_PARAM,			/**< Clmapmgr bad parameter. */
	NSS_CLMAPMGR_ERR_TUNNEL_NOT_FOUND,		/**< Clmapmgr tunnel already exist. */
	NSS_CLMAPMGR_ERR_ALLOC_TUNNEL_FAILED,		/**< Clmapmgr tunnel alloc failure. */
	NSS_CLMAPMGR_ERR_MAC_ADD_FAILED,		/**< Clmapmgr MAC addtion failure. */
	NSS_CLMAPMGR_ERR_MAC_DEL_FAILED,		/**< Clmapmgr MAC deletion failure. */
	NSS_CLMAPMGR_ERR_MAC_FLUSH_FAILED,		/**< Clmapmgr tunnel rule flush failure. */
	NSS_CLMAPMGR_ERR_MAX
} nss_clmapmgr_status_t;

/**
 * Clmapmgr tunnel type enums.
 */
typedef enum nss_clmapmgr_tunnel_type {
	NSS_CLMAPMGR_TUNNEL_EOGRE = 0,			/**< EoGRE tunnel type. */
} nss_clmapmgr_tunnel_type_t;

/**
 * Structure to provide MAC entry message parameters
 */
struct nss_clmapmgr_msg {
	nss_clmapmgr_tunnel_type_t tunnel_type;		/**< Tunnel type. */
	uint32_t vlan_id;				/**< VLAN id if present. */
	uint32_t tunnel_id;				/**< Tunnel id. */
	uint32_t needed_headroom;			/**< Minimum headroom needed in packet for upstream. */
	uint16_t mac_addr[3];				/**< MAC address. */
	uint8_t flags;					/**< Flags. */
	uint8_t reserved;				/**< Reserved. */
};

/**
 * @brief Get upstream NSS interface number for the clmap netdevice
 *
 * @param dev
 * 	clmap netdevice
 *
 * @return upstream NSS interface number
 */
extern int nss_clmapmgr_us_get_if_num(struct net_device *dev);

/**
 * @brief Get downstream NSS interface number for the clmap netdevice
 *
 * @param dev
 * 	clmap netdevice
 *
 * @return downstream NSS interface number
 */
extern int nss_clmapmgr_ds_get_if_num(struct net_device *dev);

/**
 * @brief Send a MAC flush message to NSS
 *
 * @param dev
 * 	clmap netdevice
 * @param tunnel_id
 * 	tunnel id for which the MAC addresses needs to be flushed
 * @param tunnel_type
 * 	clmapmgr tunnel type of the tunnel
 *
 * @return nss_clmapmgr_status_t
 * 	NSS clmapmgr command status
 *
 * @note The API should be used used to flush MAC entries for a tunnel before a tunnel destroy
 */
extern nss_clmapmgr_status_t nss_clmapmgr_mac_flush(struct net_device *dev, uint32_t tunnel_id, nss_clmapmgr_tunnel_type_t tunnel_type);

/**
 * @brief Send a MAC entry remove message to NSS
 *
 * @param dev
 * 	clmap netdevice
 * @param mac_addr
 * 	MAC address to be removed
 *
 * @return nss_clmapmgr_status_t
 * 	NSS clmapmgr command status
 */
extern nss_clmapmgr_status_t nss_clmapmgr_mac_remove(struct net_device *dev, uint8_t *mac_addr);

/**
 * @brief Send a MAC entry add message to NSS
 *
 * @param dev
 * 	clmap netdevice
 * @param clmapmsg
 * 	tunnel parameters attached to a MAC address
 *
 * @return nss_clmapmgr_status_t
 * 	NSS clmapmgr command status
 */
extern nss_clmapmgr_status_t nss_clmapmgr_mac_add(struct net_device *dev, struct nss_clmapmgr_msg *clmapmsg);

/**
 * @brief Disable a clmapmgr interface
 *
 * @param dev
 * 	clmap netdevice
 *
 * @return int
 * 	Return netdevice command status
 */
extern int nss_clmapmgr_netdev_disable(struct net_device *dev);

/**
 * @brief Enable a clmapmgr interface
 *
 * @param dev
 * 	clmap netdevice
 *
 * @return int
 * 	Return netdevice command status
 */
extern int nss_clmapmgr_netdev_enable(struct net_device *dev);

/**
 * @brief Destroy a clmapmgr netdevice
 *
 * @param dev
 * 	clmap netdevice
 *
 * @return nss_clmapmgr_status_t
 * 	NSS clmapmgr command status
 *
 * @note clmap MAC rules for the tunnels must be destroyed before calling this API
 */
extern nss_clmapmgr_status_t nss_clmapmgr_netdev_destroy(struct net_device *dev);

/**
 * @brief Creates a clmapmgr netdevice
 *
 * @return Pointer to a newly created netdevice
 *
 * @note First Client map interface name is nssclmap0 and so on
 */
extern struct net_device *nss_clmapmgr_netdev_create(void);

#endif /* __NSS_CLMAPMGR_H */
