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
 * nss_pvxlanmgr.h
 *	Proxy VxLAN manager for NSS
 */
#ifndef __NSS_PVXLANMGR_H
#define __NSS_PVXLANMGR_H

/*
 * Maximum number of tunnels currently supported
 */
#define NSS_PVXLANMGR_MAX_TUNNELS		64

#define NSS_PVXLANMGR_TUNNEL_STATE_CONFIGURED	0x1
					/**< Bit is set if tunnel has been configured. */

/**
 * PVXLAN status enums
 */
typedef enum nss_pvxlanmgr_status{
	/*
	 * nss_tx_status_t enums
	 */
	NSS_PVXLANMGR_SUCCESS = NSS_TX_SUCCESS,
	NSS_PVXLANMGR_FAILURE = NSS_TX_FAILURE,
	NSS_PVXLANMGR_FAILURE_QUEUE = NSS_TX_FAILURE_QUEUE,
	NSS_PVXLANMGR_FAILURE_NOT_READY = NSS_TX_FAILURE_NOT_READY,
	NSS_PVXLANMGR_FAILURE_TOO_LARGE = NSS_TX_FAILURE_TOO_LARGE,
	NSS_PVXLANMGR_FAILURE_TOO_SHORT = NSS_TX_FAILURE_TOO_SHORT,
	NSS_PVXLANMGR_FAILURE_NOT_SUPPORTED = NSS_TX_FAILURE_NOT_SUPPORTED,
	NSS_PVXLANMGR_FAILURE_BAD_PARAM = NSS_TX_FAILURE_BAD_PARAM,

	/*
	 * PVXLAN specific ones.
	 */
	NSS_PVXLANMGR_FAILURE_TUNNEL_ENABLED = NSS_TX_FAILURE_MAX + 1,
							/**< Tunnel is enabled. */
	NSS_PVXLANMGR_FAILURE_TUNNEL_DISABLED,		/**< Tunnel is disabled. */
	NSS_PVXLANMGR_FAILURE_TUNNEL_NOT_CREATED,	/**< Tunnel is not configured yet. */
	NSS_PVXLANMGR_FAILURE_TUNNEL_EXISTS,		/**< Tunnel already exisits. */
	NSS_PVXLANMGR_FAILURE_UNREGISTER_DECONGESTION_CB,
							/**< Failed to unregister decongestion callback. */
	NSS_PVXLANMGR_FAILURE_DI_ALLOC_FAILED,		/**< Dynamic interface alloc failed. */
	NSS_PVXLANMGR_FAILURE_DI_DEALLOC_FAILED,	/**< Dynamic interface dealloc failed. */
	NSS_PVXLANMGR_FAILURE_PVXLAN_RULE,		/**< Failed to create PVXLAN rule. */
	NSS_PVXLANMGR_FAILURE_IP_RULE,			/**< Failed to create IP rule. */
	NSS_PVXLANMGR_FAILURE_REGISTER_NSS,		/**< Failed to register with NSS. */
	NSS_PVXLANMGR_FAILURE_CMD_TIMEOUT,		/**< NSS Driver Command timed-out. */
	NSS_PVXLANMGR_FAILURE_INVALID_L3_PROTO,		/**< Invalid Layer3 protocol. */
	NSS_PVXLANMGR_FAILURE_INVALID_UDP_PROTO,	/**< Invalid UDP protocol. */
	NSS_PVXLANMGR_FAILURE_INVALID_VERSION,		/**< Invalid pvxlan version. */
	NSS_PVXLANMGR_FAILURE_IP_DESTROY_RULE,		/**< Destroy IP rule failed. */
	NSS_PVXLANMGR_FAILURE_PVXLAN_DESTROY_RULE,	/**< Destroy pvxlan rule failed. */
	NSS_PVXLANMGR_FAILURE_INVALID_IP_NODE,		/**< Invalid tunnel IP node. */
	NSS_PVXLANMGR_FAILURE_INVALID_TYPE_FLAG,	/**< Invalid type. */
} nss_pvxlanmgr_status_t;

/**
 * Private structure to store vxlan header
. */
struct nss_pvxlanmgr_vxlan_hdr {
	uint16_t flags;					/**< VxLAN specific flags. */
	uint16_t gpid;					/**< Group Policy ID. */
	uint32_t vnet_id;				/**< Virtual Net ID. */
};

/**
 * @brief Send a MAC remove message to NSS
 *
 * @param netdevice
 * @param tunnel_id
 * @param mac_addr
 *
 * @return nss_pvxlanmgr_status_t
 */
extern nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_mac_remove(struct net_device *dev, uint32_t tunnel_id, uint8_t *mac_addr);

/**
 * @brief Send a MAC add message to NSS
 *
 * @param netdevice
 * @param tunnel_id
 * @param mac_addr
 * @param pvxch
 *
 * @return nss_pvxlanmgr_status_t
 */
extern nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_mac_add(struct net_device *dev, uint32_t tunnel_id, uint8_t *mac_addr, struct nss_pvxlanmgr_vxlan_hdr *pvxch);

/**
 * @brief Disable a PVXLAN tunnel
 *
 * @param netdevice
 *
 * @return nss_pvxlanmgr_status_t
 */
extern nss_pvxlanmgr_status_t nss_pvxlanmgr_netdev_disable(struct net_device *dev);

/**
 * @brief Enable a PVXLAN tunnel
 *
 * @param netdevice
 *
 * @return nss_pvxlanmgr_status_t
 */
extern nss_pvxlanmgr_status_t nss_pvxlanmgr_netdev_enable(struct net_device *dev);

/**
 * @brief Destroy a PVXLAN tunnel
 *
 * @param netdevice
 * @param tunnel_id
 *
 * @return nss_pvxlanmgr_status_t
 *
 * @note PVxLAN tunnel must be disabled before destroy operation.
 */
extern nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_destroy(struct net_device *dev, uint32_t tunnel_id);

/**
 * @brief Creates a IPv4 Pvxlan tunnel
 *
 * Flow direction is expected to be ingress from the WAN port.
 *
 * @param netdevice
 * @param tunnel_id
 * @param IPv4 rule structure
 * @param tunnel UDP source port for encapsulation
 *
 * @return nss_pvxlanmgr_status_t
 */
extern nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_ipv4_create(struct net_device *dev, uint32_t tunnel_id,
							struct nss_ipv4_rule_create_msg *nircm, uint32_t pvxlan_src_port);

/**
 * @brief Creates a IPv6 Pvxlan tunnel
 *
 * Flow direction is expected to be ingress from the WAN port.
 *
 * @param netdevice
 * @param tunnel_id
 * @param IPv6 rule structure
 * @param tunnel UDP source port for encapsulation
 *
 * @return nss_pvxlanmgr_status_t
 */
extern nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_ipv6_create(struct net_device *dev, uint32_t tunnel_id,
							struct nss_ipv6_rule_create_msg *nircm, uint32_t pvxlan_src_port);

/**
 * @brief Return PVxLAN outer interface number
 *
 * @param netdevice
 *
 * @return uint32_t
 */
extern uint32_t nss_pvxlanmgr_get_if_num_outer(struct net_device *dev);

/**
 * @brief Return PVxLAN host inner interface number
 *
 * @param netdevice
 *
 * @return uint32_t
 */
extern uint32_t nss_pvxlanmgr_get_if_num_host_inner(struct net_device *dev);

/**
 * @brief Destroy a PVxLAN netdevice
 *
 * @param netdevice
 *
 * @return nss_pvxlanmgr_status_t
 *
 * @note PVXLAN tunnel and any mac rule must be destroyed first.
 */
extern nss_pvxlanmgr_status_t nss_pvxlanmgr_netdev_destroy(struct net_device *dev);

/**
 * @brief Creates a PVXLAN netdevice
 *
 * @return Pointer to a newly created netdevice
 *
 * @note First Pvxlan interface name is pvxlan0 and so on
 */
extern struct net_device *nss_pvxlanmgr_netdev_create(void);

#endif /* __NSS_PVXLANMGR_H */
