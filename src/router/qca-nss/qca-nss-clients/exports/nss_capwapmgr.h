/*
 **************************************************************************
 * Copyright (c) 2014-2020, The Linux Foundation. All rights reserved.
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
 * nss_capwapmgr.h
 *	CAPWAP manager for NSS
 */
#ifndef __NSS_CAPWAPMGR_H
#define __NSS_CAPWAPMGR_H

#include <nss_dtlsmgr.h>

/*
 * Maximum number of tunnels currently supported
 */
#define NSS_CAPWAPMGR_MAX_TUNNELS		32

#define NSS_CAPWAPMGR_TUNNEL_STATE_CONFIGURED		0x1
					/**< Bit is set if tunnel has been configured */
#define NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED		0x2
					/**< Bit is set if tunnel has been enabled */
#define NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED	0x4
					/**< Bit is set if tunnel IP rule exist */

/*
 * Tunnel feature flags
 */
#define NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED		0x00000001	/* Tunnel enabled DTLS. */
#define NSS_CAPWAPMGR_FEATURE_INNER_TRUSTSEC_ENABLED	0x00000002	/* Tunnel enabled inner trustsec. */
#define NSS_CAPWAPMGR_FEATURE_OUTER_TRUSTSEC_ENABLED	0x00000004	/* Tunnel enabled outer trustsec. */
#define NSS_CAPWAPMGR_FEATURE_WIRELESS_QOS_ENABLED	0x00000008	/* Tunnel enabled wireless QoS. */

/*
 * All CAPWAP messages to NSS FW are sync in nature. It means we have
 * to wait for ACK/NACK from NSS FW before proceeding further.
 * Keep a NSS FW response table to wakeup sync message caller.
 */
struct nss_capwapmgr_response {
	struct semaphore sem;
	wait_queue_head_t wq;
	enum nss_cmn_response response;
	nss_capwap_msg_response_t error;
	atomic_t seq;
};

/**
 * Mapping table from tunnel-id to if_num and rule.
 */
struct nss_capwapmgr_tunnel {
	struct net_device *dtls_dev;		/**< DTLS netdevice */
	uint32_t if_num;			/**< Interface number of NSS */
	uint32_t tunnel_state;			/**< Tunnel state */
	union {
		struct nss_ipv4_create v4;	/**< IPv4 rule structure */
		struct nss_ipv6_create v6;	/**< IPv6 rule struture */
	} ip_rule;
	struct nss_capwap_rule_msg capwap_rule;	/**< Copy of CAPWAP rule */
};

/**
 * Private structure to store information needed by a nss_capwap net_device
 */
struct nss_capwapmgr_priv {
	struct nss_ctx_instance *nss_ctx;	/**< Pointer to NSS context */
	struct nss_capwapmgr_tunnel *tunnel;	/**< Pointer to tunnel data */
	uint8_t *if_num_to_tunnel_id;		/**< Mapping table from if_num to tunnel_id. */
	struct nss_capwapmgr_response *resp;	/**< Response housekeeping */
};

/**
 * CAPWAP status enums
 */
typedef enum {
	/*
	 * nss_tx_status_t enums
	 */
	NSS_CAPWAPMGR_SUCCESS = NSS_TX_SUCCESS,
	NSS_CAPWAPMGR_FAILURE = NSS_TX_FAILURE,
	NSS_CAPWAPMGR_FAILURE_QUEUE = NSS_TX_FAILURE_QUEUE,
	NSS_CAPWAPMGR_FAILURE_NOT_READY = NSS_TX_FAILURE_NOT_READY,
	NSS_CAPWAPMGR_FAILURE_TOO_LARGE = NSS_TX_FAILURE_TOO_LARGE,
	NSS_CAPWAPMGR_FAILURE_TOO_SHORT = NSS_TX_FAILURE_TOO_SHORT,
	NSS_CAPWAPMGR_FAILURE_NOT_SUPPORTED = NSS_TX_FAILURE_NOT_SUPPORTED,
	NSS_CAPWAPMGR_FAILURE_BAD_PARAM = NSS_TX_FAILURE_BAD_PARAM,

	/*
	 * CAPWAP specific ones.
	 */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_ENABLED = 100,	/**< Tunnel is enabled */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_DISABLED,		/**< Tunnel is disabled */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_NOT_CFG,		/**< Tunnel is not configured yet */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_EXISTS,		/**< Tunnel already exisits */
	NSS_CAPWAPMGR_FAILURE_DI_ALLOC_FAILED,		/**< Dynamic interface alloc failed */
	NSS_CAPWAPMGR_FAILURE_CAPWAP_RULE,		/**< Failed to create CAPWAP rule */
	NSS_CAPWAPMGR_FAILURE_IP_RULE,			/**< Failed to create IP rule */
	NSS_CAPWAPMGR_FAILURE_REGISTER_NSS,		/**< Failed to register with NSS */
	NSS_CAPWAPMGR_FAILURE_CMD_TIMEOUT,		/**< NSS Driver Command timed-out */
	NSS_CAPWAPMGR_FAILURE_INVALID_REASSEMBLY_TIMEOUT,/**< Invalid reasm timeout */
	NSS_CAPWAPMGR_FAILURE_INVALID_PATH_MTU,		/**< Invalid path mtu */
	NSS_CAPWAPMGR_FAILURE_INVALID_MAX_FRAGMENT,	/**< Invalid max fragment */
	NSS_CAPWAPMGR_FAILURE_INVALID_BUFFER_SIZE,	/**< Invalid buffer size */
	NSS_CAPWAPMGR_FAILURE_INVALID_L3_PROTO,		/**< Invalid Layer3 protocol */
	NSS_CAPWAPMGR_FAILURE_INVALID_UDP_PROTO,	/**< Invalid UDP protocol */
	NSS_CAPWAPMGR_FAILURE_INVALID_VERSION,		/**< Invalid capwap version */
	NSS_CAPWAPMGR_FAILURE_IP_DESTROY_RULE,		/**< Destroy IP rule failed */
	NSS_CAPWAPMGR_FAILURE_CAPWAP_DESTROY_RULE,	/**< Destroy capwap rule failed */
	NSS_CAPWAPMGR_FAILURE_INVALID_IP_NODE,		/**< Invalid tunnel IP node */
	NSS_CAPWAPMGR_FAILURE_INVALID_TYPE_FLAG,	/**< Invalid type */
	NSS_CAPWAPMGR_FAILURE_INVALID_DTLS_CFG,		/**< Invalid DTLS configuration */
	NSS_CAPWAPMGR_FAILURE_CONFIGURE_TRUSTSEC_TX,	/**< Failed to configure trustsectx */
	NSS_CAPWAPMGR_FAILURE_UNCONFIGURE_TRUSTSEC_TX,	/**< Failed to unconfigure trustsectx */
	NSS_CAPWAPMGR_FAILURE_CONFIGURE_DSCP_MAP,	/**< Failed to configure dscp_map */
	NSS_CAPWAPMGR_FAILURE_CREATE_UDF_PROFILE,	/**< Failed creating user defined profile */
	NSS_CAPWAPMGR_FAILURE_ACL_RULE_ALREADY_EXIST,	/**< ACL rule already exist */
	NSS_CAPWAPMGR_FAILURE_ADD_ACL_RULE,		/**< Failed adding ACL rule */
	NSS_CAPWAPMGR_FAILURE_BIND_ACL_LIST,		/**< Failed binding ACL list */
	NSS_CAPWAPMGR_FAILURE_ACL_UNAVAILABLE,		/**< ACL rule unavailable */
	NSS_CAPWAPMGR_FAILURE_MEM_UNAVAILABLE,		/**< Failed to alloc memory */
	NSS_CAPWAPMGR_FAILURE_DSCP_RULE_ID_INVALID,	/**< DSCP rule ID invalid */
	NSS_CAPWAPMGR_FAILURE_DSCP_RULE_ID_NOT_IN_USE,	/**< DSCP rule not in use */
	NSS_CAPWAPMGR_FAILURE_DSCP_RULE_DELETE_FAILED,	/**< DSCP rule delete failed */
} nss_capwapmgr_status_t;

/**
 * @brief Creates a CAPWAP netdevice
 *
 * @return Pointer to a newly created netdevice
 *
 * @note First CAPWAP interface name is capwap0 and so on
 */
extern struct net_device *nss_capwapmgr_netdev_create(void);

/**
 * @brief Creates a IPv4 CAPWAP tunnel
 *
 * @param netdevice
 * @param tunnel_id
 * @param IPv4 rule structure
 * @param CAPWAP rule structure
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_ipv4_tunnel_create(struct net_device *dev, uint8_t tunnel_id,
			struct nss_ipv4_create *ip_rule, struct nss_capwap_rule_msg *capwap_rule,
			struct nss_dtlsmgr_config *in_data);

/**
 * @brief Creates a IPv6 CAPWAP tunnel
 *
 * @param netdevice
 * @param tunnel_id
 * @param IPv6 rule structure
 * @param CAPWAP rule structure
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_ipv6_tunnel_create(struct net_device *dev, uint8_t tunnel_id,
			struct nss_ipv6_create *ip_rule, struct nss_capwap_rule_msg *capwap_rule,
			struct nss_dtlsmgr_config *in_data);

/**
 * @brief Enable a CAPWAP tunnel
 *
 * @param netdevice
 * @param tunnel_id
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_enable_tunnel(struct net_device *dev, uint8_t tunnel_id);

/**
 * @brief Enable a CAPWAP tunnel
 *
 * @param netdevice
 * @param tunnel_id
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_disable_tunnel(struct net_device *dev, uint8_t tunnel_id);

/**
 * @brief Updates Path MTU of a CAPWAP tunnel
 *
 * @param netdevice
 * @param tunnel_id
 * @param New Path MTU
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_update_path_mtu(struct net_device *dev, uint8_t tunnel_id, uint32_t mtu);

/**
 * @brief Updates Destination MAC Address of a CAPWAP tunnel
 *
 * @param netdevice
 * @param tunnel_id
 * @param New MAC Address
 *
 * @return nss_capwapmgr_status_t
 */
nss_capwapmgr_status_t nss_capwapmgr_update_dest_mac_addr(struct net_device *dev, uint8_t tunnel_id, uint8_t *mac_addr);

/**
 * @brief Updates Source Interface number
 *
 * @param netdevice
 * @param tunnel_id
 * @param source interface number
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_update_src_interface(struct net_device *dev, uint8_t tunnel_id, int32_t src_interface_num);

/**
 * @brief Delete a DSCP prioritization rule that was created.
 *
 * @param Rule ID
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_dscp_rule_destroy(uint8_t id);

/**
 * @brief Prioritize packets with certain dscp value. 0 - lowest priority, 3 - highest priority.
 *
 * @param DSCP value
 * @param DSCP mask
 * @param Priority[0-3]
 * @param[out] Return rule ID
 *
 * @return nss_capwapmgr_status_t
 */
nss_capwapmgr_status_t nss_capwapmgr_dscp_rule_create(uint8_t dscp_value, uint8_t dscp_mask, uint8_t pri, uint8_t *id);

/**
 * @brief Get the DTLS net_device associated to the CAPWAP tunnel
 *
 * @param netdevice
 * @param tunnel_id
 *
 * @return Pointer to struct net_device
 *
 * @note This API hold the NET_DEVICE reference; after use the caller must perform
 * "dev_put" to release the reference.
 */
struct net_device *nss_capwapmgr_get_dtls_netdev(struct net_device *dev, uint8_t tunnel_id);

/**
 * @brief Changes version of a CAPWAP tunnel
 *
 * @param netdevice
 * @param tunnel_id
 * @param New version
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_change_version(struct net_device *dev, uint8_t tunnel_id, uint8_t ver);

/**
 * @brief Configure dtls settings of a CAPWAP tunnel
 *
 * @param netdevice
 * @param tunnel_id
 * @param enable or disable
 * @param dtls configuration
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_configure_dtls(struct net_device *dev, uint8_t tunnel_id,
							uint8_t enable_dtls, struct nss_dtlsmgr_config *in_data);

/**
 * @brief RX cipher update for a CAPWAP DTLS tunnel
 *
 * @param netdevice
 * @param tunnel_id
 * @param dtls configuration update
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_rx_cipher_update(struct net_device *dev, uint8_t tunnel_id,
									struct nss_dtlsmgr_config_update *udata);

/**
 * @brief TX cipher update for a CAPWAP DTLS tunnel
 *
 * @param netdevice
 * @param tunnel_id
 * @param dtls configuration update
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_tx_cipher_update(struct net_device *dev, uint8_t tunnel_id,
									struct nss_dtlsmgr_config_update *udata);

/**
 * @brief RX cipher switch for a CAPWAP DTLS tunnel
 *
 * @param netdevice
 * @param tunnel_id
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_rx_cipher_switch(struct net_device *dev, uint8_t tunnel_id);

/**
 * @brief TX cipher switch for a CAPWAP DTLS tunnel
 *
 * @param netdevice
 * @param tunnel_id
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_tx_cipher_switch(struct net_device *dev, uint8_t tunnel_id);

/**
 * @brief Destroy a CAPWAP tunnel
 *
 * @param netdevice
 * @param tunnel_id
 *
 * @return nss_capwapmgr_status_t
 *
 * @note CAPWAP tunnel must be disabled before destroy operation.
 */
extern nss_capwapmgr_status_t nss_capwapmgr_tunnel_destroy(struct net_device *dev, uint8_t tunnel_id);

/**
 * @brief Send a flow rule add message to NSS
 *
 * @param netdevice
 * @param tunnel_id
 * @param ip_version
 * @param protocol
 * @param src_ip
 * @param dst_ip
 * @param src_port
 * @param dst_port
 * @param flow_id
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_add_flow_rule(struct net_device *dev, uint8_t tunnel_id, uint16_t ip_version,
						uint16_t protocol, uint32_t *src_ip, uint32_t *dst_ip,
						uint16_t src_port, uint16_t dst_port, uint32_t flow_id);

/**
 * @brief Send a flow rule delete message to NSS
 *
 * @param netdevice
 * @param tunnel_id
 * @param ip_version
 * @param protocol
 * @param src_ip
 * @param dst_ip
 * @param src_port
 * @param dst_port
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_del_flow_rule(struct net_device *dev, uint8_t tunnel_id, uint16_t ip_version,
						uint16_t protocol, uint32_t *src_ip, uint32_t *dst_ip,
						uint16_t src_port, uint16_t dst_port);

/**
 * @brief Destroy a netdevice
 *
 * @param netdevice
 *
 * @return nss_capwapmgr_status_t
 *
 * @note CAPWAP tunnel must be destroyed first.
 */
extern nss_capwapmgr_status_t nss_capwapmgr_netdev_destroy(struct net_device *netdev);

/**
 * @brief Gets CAPWAP tunnel stats
 *
 * @param netdevice
 * @param tunnel_id
 * @param pointer to struct nss_capwap_tunnel_stats
 *
 * @return nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_tunnel_stats(struct net_device *dev, uint8_t tunnel_id,
							struct nss_capwap_tunnel_stats *stats);

#if defined(NSS_CAPWAPMGR_ONE_NETDEV)
/**
 * @brief Returns netdevice used by NSS CAPWAP manager
 *
 * @param void
 *
 * @return Pointer to struct net_device
 */
extern struct net_device *nss_capwapmgr_get_netdev(void);
#endif /* NSS_CAPWAPMGR_ONE_NETDEV */
#endif /* __NSS_CAPWAPMGR_H */
