/*
 **************************************************************************
 * Copyright (c) 2014-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
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
 * nss_capwapmgr_public.h
 *	CAPWAP manager for NSS
 */
#ifndef __NSS_CAPWAPMGR_PUBLIC_H
#define __NSS_CAPWAPMGR_PUBLIC_H

#define NSS_CAPWAPMGR_MAX_TUNNELS		32
					/** Maximum number of tunnels currently supported. */
#define NSS_CAPWAPMGR_TUNNEL_STATE_CONFIGURED		0x1
					/**< Bit is set if tunnel has been configured. */
#define NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED		0x2
					/**< Bit is set if tunnel has been enabled. */
#define NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED	0x4
					/**< Bit is set if tunnel IP rule exist. */
#define NSS_CAPWAPMGR_TUNNEL_STATE_INNER_ALLOCATED		0x8
					/**< Bit is set if the inner node is allocated. */
#define NSS_CAPWAPMGR_TUNNEL_STATE_OUTER_ALLOCATED		0x10
					/**< Bit is set if the outer node is allocated. */
#define NSS_CAPWAPMGR_TUNNEL_STATE_TRUSTSEC_TX_CONFIGURED		0x11
					/**< Bit is set if trustsec_tx tunnel rules are configured. */

#define NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED              0x00000001      /**< Tunnel enabled DTLS. */
#define NSS_CAPWAPMGR_FEATURE_INNER_TRUSTSEC_ENABLED	0x00000002	/**< Tunnel enabled inner trustsec. */
#define NSS_CAPWAPMGR_FEATURE_OUTER_TRUSTSEC_ENABLED	0x00000004	/**< Tunnel enabled outer trustsec. */
#define NSS_CAPWAPMGR_FEATURE_WIRELESS_QOS_ENABLED	0x00000008	/**< Tunnel enabled wireless QoS. */
#define NSS_CAPWAPMGR_FEATURE_PPE_TO_HOST_ENABLED	0x00000010	/**< Tunnel enable PPE to host. */

/**
 * nss_capwapmgr_response
 *	NSS FW response table to wakeup sync message caller.
 *
 * All CAPWAP messages to NSS FW are sync in nature. It means we have
 * to wait for ACK/NACK from NSS FW before proceeding further.
 */
struct nss_capwapmgr_response {
	struct semaphore sem;			/**< Semaphore to limit one command per CAPWAP interface. */
	wait_queue_head_t wq;			/**< Wait queue to hold sync message caller. */
	enum nss_cmn_response response;		/**< Response from the NSS-FW. */
	nss_capwap_msg_response_t error;	/**< Error response in case of NACK. */
	atomic_t seq;				/**< Atomic variable to represent the waiting state. */
};

/**
 * nss_capwapmgr_tunnel
 *	Mapping table from tunnel-id to if_num and rule.
 */
struct nss_capwapmgr_tunnel {
	struct net_device *dtls_dev;		/**< DTLS netdevice */
	struct net_device *internal_dev_encap;	/**< Internal device for UL VP allocation. */
	struct net_device *internal_dev_decap;	/**< Internal device for DL VP allocation. */
	uint32_t if_num_inner;			/**< Interface number of the INNER CAPWAP node. */
	uint32_t if_num_outer;			/**< Interface number of the OUTER CAPWAP node. */
	uint32_t tunnel_state;			/**< Tunnel state. */
	uint16_t type_flags;			/**< Tunnel Type to determine header size. */
	ppe_vp_num_t vp_num_encap;		/**< UL VP number associated with the tunnel. */
	ppe_vp_num_t vp_num_decap;		/**< DL VP number associated with the tunnel. */
	uint8_t tunnel_id;			/**< TrustSec Tx tunnel id. */
	union {
		struct nss_ipv4_create v4;	/**< IPv4 rule structure. */
		struct nss_ipv6_create v6;	/**< IPv6 rule struture. */
	} ip_rule;
	struct nss_capwap_rule_msg capwap_rule;	/**< Copy of CAPWAP rule. */
};

/**
 * nss_capwapmgr_priv
 *	Private structure to store information needed by a nss_capwap net_device
 */
struct nss_capwapmgr_priv {
	struct nss_ctx_instance *nss_ctx;	/**< Pointer to NSS context. */
	struct nss_capwapmgr_tunnel *tunnel;	/**< Pointer to tunnel data. */
	uint8_t *if_num_to_tunnel_id;		/**< Mapping table from if_num to tunnel_id. */
	struct nss_capwapmgr_response *resp;	/**< Response housekeeping. */
};

/**
 * nss_capwapmgr_flow_info
 *	Inner flow information.
 */
struct nss_capwapmgr_flow_info {
	uint16_t ip_version;	/**< IP version. */
	uint16_t protocol;	/**< Protocol. */
	uint32_t src_ip[4];	/**< Source IP address. */
	uint32_t dst_ip[4];	/**< Destination IP address. */
	uint16_t src_port;	/**< Source port. */
	uint16_t dst_port;	/**< Destination port. */
	struct nss_capwap_flow_attr flow_attr;
				/**< Flow attributes. */
};

/**
 * nss_capwapmgr_status_t
 *	CAPWAP status enums
 */
typedef enum {
	/*
	 * nss_tx_status_t enums
	 */
	NSS_CAPWAPMGR_SUCCESS = NSS_TX_SUCCESS,					/**< ACK response from NSS-FW. */
	NSS_CAPWAPMGR_FAILURE = NSS_TX_FAILURE,					/**< NACK response from NSS-FW. */
	NSS_CAPWAPMGR_FAILURE_NOT_READY = NSS_TX_FAILURE_NOT_READY,		/**< NSS Core is not ready. */
	NSS_CAPWAPMGR_FAILURE_BAD_PARAM = NSS_TX_FAILURE_BAD_PARAM,		/**< Invalid message parameter. */

	/*
	 * CAPWAP specific ones.
	 */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_ENABLED = 100,	/**< Tunnel is enabled. */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_DISABLED,		/**< Tunnel is disabled. */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_NOT_CFG,		/**< Tunnel is not configured yet. */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_EXISTS,		/**< Tunnel already exisits. */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_DOES_NOT_EXIST, /**< Tunnel does not exist. */
	NSS_CAPWAPMGR_MAX_TUNNEL_COUNT_EXCEEDED,	/**< Exceeding msximum allowed tunnels. */
	NSS_CAPWAPMGR_FAILURE_DI_ALLOC_FAILED,		/**< Dynamic interface alloc failed. */
	NSS_CAPWAPMGR_FAILURE_CAPWAP_RULE,		/**< Failed to create CAPWAP rule. */
	NSS_CAPWAPMGR_FAILURE_IP_RULE,			/**< Failed to create IP rule. */
	NSS_CAPWAPMGR_INVALID_IP_RULE,			/**< Invalid IP rule for the tunnel. */
	NSS_CAPWAPMGR_FAILURE_REGISTER_NSS,		/**< Failed to register with NSS. */
	NSS_CAPWAPMGR_FAILURE_CMD_TIMEOUT,		/**< NSS Driver Command timed-out. */
	NSS_CAPWAPMGR_FAILURE_INVALID_REASSEMBLY_TIMEOUT,/**< Invalid reasm timeout. */
	NSS_CAPWAPMGR_FAILURE_INVALID_PATH_MTU,		/**< Invalid path mtu. */
	NSS_CAPWAPMGR_FAILURE_INVALID_MAX_FRAGMENT,	/**< Invalid max fragment. */
	NSS_CAPWAPMGR_FAILURE_INVALID_BUFFER_SIZE,	/**< Invalid buffer size. */
	NSS_CAPWAPMGR_FAILURE_INVALID_L3_PROTO,		/**< Invalid Layer3 protocol. */
	NSS_CAPWAPMGR_FAILURE_INVALID_UDP_PROTO,	/**< Invalid UDP protocol. */
	NSS_CAPWAPMGR_FAILURE_INVALID_VERSION,		/**< Invalid capwap version. */
	NSS_CAPWAPMGR_FAILURE_IP_DESTROY_RULE,		/**< Destroy IP rule failed. */
	NSS_CAPWAPMGR_FAILURE_CAPWAP_DESTROY_RULE,	/**< Destroy capwap rule failed. */
	NSS_CAPWAPMGR_FAILURE_INVALID_TYPE_FLAG,	/**< Invalid type. */
	NSS_CAPWAPMGR_FAILRUE_INTERNAL_DECAP_NETDEV_ALLOC_FAILED,
							/**< Internal DL netdevice alloc failed. */
	NSS_CAPWAPMGR_FAILRUE_INTERNAL_ENCAP_NETDEV_ALLOC_FAILED,
  							/**< Internal UL netdevice alloc failed. */
	NSS_CAPWAPMGR_FAILURE_DECAP_VP_ALLOC,		/**< DL PPE VP alloc failed. */
	NSS_CAPWAPMGR_FAILURE_ENCAP_VP_ALLOC,		/**< UL PPE VP alloc failed. */
	NSS_CAPWAPMGR_FAILURE_VP_FREE,			/**<PPE VP free failed. */
	NSS_CAPWAPMGR_FAILURE_VP_MTU_SET,		/**< PPE VP MTU set failed. */
	NSS_CAPWAPMGR_FAILURE_UPDATE_VP_NUM,	/**< Update VP number failed. */
	NSS_CAPWAPMGR_INVALID_NETDEVICE,		/**< Invalid CAPWAP netdevice. */
	NSS_CAPWAPMGR_FAILURE_CONFIGURE_DSCP_MAP,	/**< Failed to configure dscp_map. */
	NSS_CAPWAPMGR_FAILURE_CREATE_UDF_PROFILE,	/**< Failed creating user defined profile. */
	NSS_CAPWAPMGR_FAILURE_ACL_RULE_ALREADY_EXIST,	/**< ACL rule already exist. */
	NSS_CAPWAPMGR_FAILURE_ADD_ACL_RULE,		/**< Failed adding ACL rule. */
	NSS_CAPWAPMGR_FAILURE_BIND_ACL_LIST,		/**< Failed to bind ACL list. */
	NSS_CAPWAPMGR_FAILURE_UNBIND_ACL_LIST,		/**< Failed to unbind ACL list. */
	NSS_CAPWAPMGR_FAILURE_ACL_UNAVAILABLE,		/**< ACL rule unavailable. */
	NSS_CAPWAPMGR_FAILURE_MEM_UNAVAILABLE,		/**< Failed to alloc memory. */
	NSS_CAPWAPMGR_FAILURE_DSCP_RULE_ID_INVALID,	/**< DSCP rule ID invalid. */
	NSS_CAPWAPMGR_FAILURE_DSCP_RULE_ID_NOT_IN_USE,	/**< DSCP rule not in use. */
	NSS_CAPWAPMGR_FAILURE_DSCP_RULE_DELETE_FAILED,	/**< DSCP rule delete failed. */
	NSS_CAPWAPMGR_FAILURE_CONFIG_TRUSTSEC_RX,	/**< Failed to configure trustsec receive node. */
	NSS_CAPWAPMGR_FAILURE_BIND_ACL_RULE,		/**< Failed to bind the acl to the physical port. */
	NSS_CAPWAPMGR_FAILURE_BIND_VPORT,		/**< Failed to bind the virtual port to the physical port. */
	NSS_CAPWAPMGR_FAILURE_UNBIND_VPORT,		/**< Failed to unbind the virtual port from the physical port. */
	NSS_CAPWAPMGR_FAILURE_TRUSTSEC_RULE_EXISTS,	/**< TrustSec rule already exists. */
	NSS_CAPWAPMGR_FAILURE_TX_PORT_GET,		/**< Failed to get the physical port associated to the UL virtual port. */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_ID_SET,		/**< Failed to set UL tunnel id. */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_ID_GET,		/**< Failed to get the tunnel id associated to the UL virtual port. */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_ENCAP_ENTRY_ADD,	/**< Failed to add tunnel encap entry. */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_ENCAP_ENTRY_GET,	/**< Failed to get tunnel encap entry. */
	NSS_CAPWAPMGR_FAILURE_TUNNEL_ENCAP_ENTRY_DELETE,	/**< Failed to delete tunnel encap entry. */
	NSS_CAPWAPMGR_FAILURE_TRUSTSEC_VP_NUM_UPDATE,	/**< Failed to update TrustSec virtual port number. */
	NSS_CAPWAPMGR_FAILURE_CONFIGURE_TRUSTSEC_TX,	/**< Failed to configure TrustSec Tx rule. */
	NSS_CAPWAPMGR_FAILURE_DSCP_ACL_INIT,		/**< Failed to initialize DSCP ACL related objects. */
	NSS_CAPWAPMGR_FAILURE_INVALID_DTLS_CFG,		/**< Invalid DTLS configuration. */
} nss_capwapmgr_status_t;

/**
 * nss_capwapmgr_netdev_create
 *	Creates a CAPWAP netdevice.
 *
 * @return
 * Pointer to a newly created netdevice.
 *
 * @note
 * First CAPWAP interface name is capwap0 and so on.
 */
extern struct net_device *nss_capwapmgr_netdev_create(void);

/**
 * nss_capwapmgr_ipv4_tunnel_create
 *	Creates an IPv4 CAPWAP tunnel.
 *
 * @datatypes
 * net_device \n
 * nss_ipv4_create \n
 * nss_capwap_rule_msg \n
 * nss_dtlsmgr_config
 *
 * @param[in] netdevice	CAPWAP netdevice.
 * @param[in] tunnel_id	Tunnel ID of the tunnel.
 * @param[in] IPv4	IPv4 rule structure.
 * @param[in] CAPWAP	CAPWAP rule structure.
 * @param[in] DTLS	DTLS config data.
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_ipv4_tunnel_create(struct net_device *dev, uint8_t tunnel_id,
			struct nss_ipv4_create *ip_rule, struct nss_capwap_rule_msg *capwap_rule, struct nss_dtlsmgr_config *dtls_data);

/**
 * nss_capwapmgr_ipv6_tunnel_create
 *	Creates an IPv6 CAPWAP tunnel.
 *
 * @datatypes
 * net_device \n
 * nss_ipv6_create \n
 * nss_capwap_rule_msg \n
 * nss_dtlsmgr_config
 *
 * @param[in] netdevice	CAPWAP netdevice.
 * @param[in] tunnel_id	Tunnel ID of thethe tunnel.
 * @param[in] IPv6	IPv6 rule structure.
 * @param[in] CAPWAP	CAPWAP rule structure.
 * @param[in] DTLS	DTLS config data.
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_ipv6_tunnel_create(struct net_device *dev, uint8_t tunnel_id,
			struct nss_ipv6_create *ip_rule, struct nss_capwap_rule_msg *capwap_rule, struct nss_dtlsmgr_config *dtls_data);

/**
 * nss_capwapmgr_enable_tunnel
 *	Enable a CAPWAP tunnel.
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice	CAPWAP netdevice.
 * @param[in] tunnel_id	Tunnel ID of thethe tunnel.
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_enable_tunnel(struct net_device *dev, uint8_t tunnel_id);

/**
 * nss_capwapmgr_disable_tunnel
 *	Enable a CAPWAP tunnel.
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice	CAPWAP netdevice.
 * @param[in] tunnel_id	Tunnel ID of the tunnel.
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_disable_tunnel(struct net_device *dev, uint8_t tunnel_id);

/**
 *nss_capwapmgr_update_path_mtu
 *	Updates Path MTU of a CAPWAP tunnel.
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice	CAPWAP netdevice.
 * @param[in] tunnel_id	Tunnel ID of the tunnel.
 * @param[in] mtu	New path MTU.
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_update_path_mtu(struct net_device *dev, uint8_t tunnel_id, uint32_t mtu);

/**
 * nss_capwapmgr_update_dest_mac_addr
 *	Updates Destination MAC Address of a CAPWAP tunnel.
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice	CAPWAP netdevice.
 * @param[in] tunnel_id	Tunnel ID of the tunnel.
 * @param[in] mac_addr	New MAC Address.
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_update_dest_mac_addr(struct net_device *dev, uint8_t tunnel_id, uint8_t *mac_addr);

/**
 * nss_capwapmgr_update_src_interface
 *	Updates Source Interface number.
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice	CAPWAP netdevice.
 * @param[in] tunnel_id	Tunnel ID of the tunnel.
 * @param[in] src_interface_number	New interface number.
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_update_src_interface(struct net_device *dev, uint8_t tunnel_id, int32_t src_interface_num);

/**
 * nss_capwapmgr_change_version
 *	Changes version of a CAPWAP tunnel.
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice	CAPWAP netdevice.
 * @param[in] tunnel_id	Tunnel ID of the tunnel.
 * @param[in] ver	CAPWAP version.
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_change_version(struct net_device *dev, uint8_t tunnel_id, uint8_t ver);

/**
 * nss_capwapmgr_tunnel_destroy
 *	Destroy a CAPWAP tunnel.
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice	CAPWAP netdevice.
 * @param[in] tunnel_id	Tunnel ID of the tunnel.
 *
 * @return
 * nss_capwapmgr_status_t
 *
 * @note
 * CAPWAP tunnel must be disabled before destroy operation.
 */
extern nss_capwapmgr_status_t nss_capwapmgr_tunnel_destroy(struct net_device *dev, uint8_t tunnel_id);

/**
 * nss_capwapmgr_add_flow_rule
 *	Send a flow rule add message to NSS.
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice	CAPWAP netdevice.
 * @param[in] tunnel_id	Tunnel ID of the tunnel.
 * @param[in] flow_info	Flow information.
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_add_flow_rule(struct net_device *dev, uint8_t tunnel_id, struct nss_capwapmgr_flow_info *flow_info);

/**
 * nss_capwapmgr_del_flow_rule
 *	Send a flow rule delete message to NSS.
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice	CAPWAP netdevice.
 * @param[in] tunnel_id	Tunnel ID of the tunnel.
 * @param[in] flow_info Flow information.
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_del_flow_rule(struct net_device *dev, uint8_t tunnel_id, struct nss_capwapmgr_flow_info *flow_info);

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
extern nss_capwapmgr_status_t nss_capwapmgr_dscp_rule_create(uint8_t dscp_value, uint8_t dscp_mask, uint8_t pri, uint8_t *id);

/**
 * nss_capwapmgr_netdev_destroy
 *	Destroy a netdevice.
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice	CAPWAP netdevice.
 *
 * @return
 * nss_capwapmgr_status_t
 *
 * @note
 * CAPWAP tunnel must be destroyed first.
 */
extern nss_capwapmgr_status_t nss_capwapmgr_netdev_destroy(struct net_device *netdev);

/**
 * nss_capwapmgr_tunnel_stats
 *	Gets CAPWAP tunnel stats.
 *
 * @datatypes
 * net_device \n
 * nss_capwap_tunnel_stats
 *
 * @param[in] netdevice	CAPWAP netdevice.
 * @param[in] tunnel_id	Tunnel ID of the tunnel.
 * @param[in] pointer	to struct nss_capwap_tunnel_stats
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_tunnel_stats(struct net_device *dev, uint8_t tunnel_id,
							struct nss_capwap_tunnel_stats *stats);

#if defined(NSS_CAPWAPMGR_ONE_NETDEV)
/**
 * nss_capwapmgr_get_netdev
 *	Returns netdevice used by NSS CAPWAP manager.
 *
 * @return
 * Pointer to struct net_device
 */
extern struct net_device *nss_capwapmgr_get_netdev(void);
#endif /* NSS_CAPWAPMGR_ONE_NETDEV */

/**
 * nss_capwapmgr_get_dtls_netdev
 *	Get the DTLS net_device associated to the CAPWAP tunnel
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice
 * @param[in] tunnel_id
 *
 * @return
 * Pointer to struct net_device
 *
 * @note This API hold the NET_DEVICE reference; after use the caller must perform
 * "dev_put" to release the reference.
 */
struct net_device *nss_capwapmgr_get_dtls_netdev(struct net_device *dev, uint8_t tunnel_id);

/**
 * nss_capwapmgr_configure_dtls
 *	Configure dtls settings of a capwap tunnel.
 *
 * @datatypes
 * net_device \n
 * nss_dtlsmgr_config
 *
 * @param[in] netdevice.
 * @param[in] tunnel_id.
 * @param[in] enable or disable
 * @param[in] dtls configuration
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_configure_dtls(struct net_device *dev, uint8_t tunnel_id,
		uint8_t enable_dtls, struct nss_dtlsmgr_config *in_data);

/**
 * nss_capwapmgr_dtls_rekey_rx_cipher_update
 *	RX cipher update for a CAPWAP DTLS tunnel
 *
 * @datatypes
 * net_device \n
 * nss_dtlsmgr_config_update
 *
 * @param[in] netdevice
 * @param[in] tunnel_id
 * @param[in] dtls configuration update
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_rx_cipher_update(struct net_device *dev, uint8_t tunnel_id,
		struct nss_dtlsmgr_config_update *udata);

/**
 * nss_capwapmgr_dtls_rekey_tx_cipher_update
 *	TX cipher update for a CAPWAP DTLS tunnel
 *
 * @datatypes
 * net_device \n
 * nss_dtlsmgr_config_update
 *
 * @param[in] netdevice
 * @param[in] tunnel_id
 * @param[in] dtls configuration update
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_tx_cipher_update(struct net_device *dev, uint8_t tunnel_id,
		struct nss_dtlsmgr_config_update *udata);

/**
 * nss_capwapmgr_dtls_rekey_rx_cipher_switch
 *	RX cipher switch for a CAPWAP DTLS tunnel
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice
 * @param[in] tunnel_id
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_rx_cipher_switch(struct net_device *dev, uint8_t tunnel_id);

/**
 * nss_capwapmgr_dtls_rekey_tx_cipher_switch
 *	TX cipher switch for a CAPWAP DTLS tunnel
 *
 * @datatypes
 * net_device
 *
 * @param[in] netdevice
 * @param[in] tunnel_id
 *
 * @return
 * nss_capwapmgr_status_t
 */
extern nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_tx_cipher_switch(struct net_device *dev, uint8_t tunnel_id);

#endif /* __NSS_CAPWAPMGR_H */
