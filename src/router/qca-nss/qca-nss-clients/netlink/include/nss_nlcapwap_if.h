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
 * @file nss_nlcapwap_if.h
 *	NSS Netlink CAPWAP headers
 */
#ifndef __NSS_NLCAPWAP_IF_H
#define __NSS_NLCAPWAP_IF_H

#include <nss_nlcmn_if.h>

/**
 * Capwap family definitions
 */
#define NSS_NLCAPWAP_META_HEADER_SZ 32
#define NSS_NLCAPWAP_FAMILY "nss_nlcapwap"
#define NSS_NLCAPWAP_MCAST_GRP "nss_nlcapwap_mc"
#define NSS_NLCAPWAP_MODE_MAX_SZ 16
#define NSS_NLCAPWAP_PROTO_MAX_SZ 16
#define NSS_NLCAPWAP_KEY_SZ 32

/**
 * @brief Enumeration for all command types.
 */
enum nss_nlcapwap_cmd_type {
	NSS_NLCAPWAP_CMD_TYPE_UNKNOWN,			/**< Unknown command type */
	NSS_NLCAPWAP_CMD_TYPE_CREATE_TUN,		/**< Creates a tunnel. */
	NSS_NLCAPWAP_CMD_TYPE_DESTROY_TUN,		/**< Destroys the tunnel created. */
	NSS_NLCAPWAP_CMD_TYPE_UPDATE_MTU,		/**< Updates the mtu of the path. */
	NSS_NLCAPWAP_CMD_TYPE_PERF,			/**< Enables or disables performance for capwap. */
	NSS_NLCAPWAP_CMD_TYPE_DTLS,			/**< Enables or disables dtls for capwap tunnel. */
	NSS_NLCAPWAP_CMD_TYPE_TX_PACKETS,		/**< Helps in configuring parameters for sending traffic. */
	NSS_NLCAPWAP_CMD_TYPE_META_HEADER,		/**< Creates a meta header for capwap. */
	NSS_NLCAPWAP_CMD_TYPE_IP_FLOW,			/**< To add or delete an ip flow for capwap. */
	NSS_NLCAPWAP_CMD_TYPE_KEEPALIVE,		/**< To enable or disable keepalive for capwap. */
	NSS_NLCAPWAP_CMD_TYPE_MAX			/**< Max number of commands type. */
};

/**
 * @brief Enumeration for ip flow type.
 */
enum nss_nlcapwap_ip_flow_mode {
	NSS_NLCAPWAP_IP_FLOW_MODE_UNKNOWN,		/**< To add ip flow rule */
	NSS_NLCAPWAP_IP_FLOW_MODE_ADD,			/**< To add ip flow rule */
	NSS_NLCAPWAP_IP_FLOW_MODE_DEL,			/**< To delelte ip flow rule */
	NSS_NLCAPWAP_IP_FLOW_MODE_MAX			/**< To delelte ip flow rule */
};

/**
 * @brief Parameters for creating tunnel
 */
struct nss_nlcapwap_create_tun {
	struct nss_capwap_rule_msg rule;	/**< Rule to add capwap tunnel */
	char gmac_ifname[IFNAMSIZ];		/**< WAN interface name */
	bool inner_trustsec_en;			/**< Inner trustsec is enabled */
	bool outer_trustsec_en;			/**< Outer trustsec is enabled */
	bool wireless_qos_en;			/**< Wireless qos is enabled */
	uint8_t gmac_ifmac[ETH_ALEN];		/**< Mac address of the wan interface */
	uint8_t bssid[ETH_ALEN];		/**< BSSID of the AP */
	uint8_t vlan_config;			/**< Vlan is configured */
	uint8_t pppoe_config;			/**< PPPOE is configured */
	uint8_t csum_enable;			/**< Udp lite is configured */
};

/**
 * @brief parameters useful for destroying the tunnel
 */
struct nss_nlcapwap_destroy_tun {
	uint8_t tun_id;			/**< Tunnel associated with the netdev */
};

/**
 * @brief parameters useful for updating the mtu of tunnel
 */
struct nss_nlcapwap_update_mtu {
	struct nss_capwap_path_mtu_msg mtu;	/** Update mtu params */
	uint8_t tun_id;				/**< Tunnel associated with the netdev */
};

/**
 * @brief parameters useful for enabling or disabling dtls
 */
struct nss_nlcapwap_dtls {
	uint32_t flags;				/**< DTLS header flags. */
	void *app_data;				/**< Opaque data returned in callback. */
	uint16_t tun_id;			/**< Tunnel for which dtls to be enabled. */
	bool enable_dtls;			/**< Enables or disables dtls for capwap tunnel. */
	uint8_t ip_version;			/**< Version of IP address */
	struct nss_dtlsmgr_encap_config encap;	/**< Encap data. */
	struct nss_dtlsmgr_decap_config decap;	/**< Decap data. */
};

/**
 * @brief parameters useful for enabling or disabling performance
 */
struct nss_nlcapwap_perf {
	bool perf_en;			/**< Enables or disables performance for capwap tunnel */
};

/**
 * @brief parameters to send traffic.
 */
struct nss_nlcapwap_tx_packets {
	uint32_t pkt_size;				/**< Packet size */
	uint32_t num_of_packets;			/**< Number of packets to be transmitted */
	uint16_t tun_id;				/**< Tunnel used for transmission */
};

/**
 * @brief parameters used to add or delete IP flow
 */
struct nss_nlcapwap_ip_flow {
	enum nss_nlcapwap_ip_flow_mode ip_flow_mode;	/**< Add or delete flow */
	struct nss_capwap_flow_rule_msg flow;		/**< IP flow rule */
	uint16_t tun_id;				/**< Tunnel for which the flow is added */
};

/**
 * @brief parameters used to create meta header
 */
struct nss_nlcapwap_meta_header {
	uint8_t meta_header_blob[NSS_NLCAPWAP_META_HEADER_SZ];	/**< Binary blob of meta header. */
	uint16_t type;						/**< Type of meta header. */
	uint16_t tun_id;					/**< Tunnel for which meta header used */
};

/**
 * @brief parameters to enable or disable keepalive pkt transmission
 */
struct nss_nlcapwap_keepalive {
	uint16_t tun_id;			/**< Tunnel used for transmission */
	bool tx_keepalive;			/**< Flag to check for dtls keepalive ON/OFF status */
};

/**
 * @brief capwap message
 */
struct nss_nlcapwap_rule {
	struct nss_nlcmn cm;		/**< Common message header */

	/**
	 * @brief payload of an CAPWAP netlink msg
	 */
	union {
		struct nss_nlcapwap_create_tun create;		/**< Create tunnel */
		struct nss_nlcapwap_destroy_tun destroy;	/**< Destroy tunnel */
		struct nss_nlcapwap_dtls dtls;			/**< Enables/creates or disables/destroys dtls tunnel. */
		struct nss_nlcapwap_perf perf;			/**< Enable or disables performance. */
		struct nss_nlcapwap_tx_packets tx_packets;	/**< Send traffic from host_to_host or end_to_end*/
		struct nss_nlcapwap_meta_header meta_header;	/**< Creates meta header */
		struct nss_nlcapwap_ip_flow ip_flow;		/**< Add or delete ip flow rules */
		struct nss_nlcapwap_update_mtu update_mtu;	/**< Update mtu of the path */
		struct nss_nlcapwap_keepalive kalive;		/**< Enable or disable keepalive transmission */
	} msg;
};

/**
 * @brief NETLINK capwap message init
 * @param rule[IN] NSS NETLINK capwap rule
 * @param type[IN] capwap cmd type
 */
static inline void nss_nlcapwap_rule_init(struct nss_nlcapwap_rule *rule, enum nss_nlcapwap_cmd_type type)
{
	nss_nlcmn_set_ver(&rule->cm, NSS_NL_VER);
	nss_nlcmn_init_cmd(&rule->cm, sizeof(struct nss_nlcapwap_rule), type);
}

/**@}*/
#endif /* __NSS_NLCAPWAP_IF_H */
