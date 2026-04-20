/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _PPE_DRV_TUN_CMN_CTX_H_
#define _PPE_DRV_TUN_CMN_CTX_H_

#include <linux/if_pppox.h>

/*
 * L2 flags
 */
#define PPE_DRV_TUN_CMN_CTX_L2_INHERIT_SPCP	0x01	/**< Inherit SVLAN PCP from inner to outer >*/
#define PPE_DRV_TUN_CMN_CTX_L2_INHERIT_CPCP	0x02	/**< Inherit CVLAN PCP from inner to outer >*/
#define PPE_DRV_TUN_CMN_CTX_L2_INHERIT_SDEI	0x04	/**< Inherit SVLAN DEI from inner to outer >*/
#define PPE_DRV_TUN_CMN_CTX_L2_INHERIT_CDEI	0x08	/**< Inherit CVLAN DEI from inner to outer >*/
#define PPE_DRV_TUN_CMN_CTX_L2_SVLAN_VALID	0x10	/**< SVLAN header valid >*/
#define PPE_DRV_TUN_CMN_CTX_L2_CVLAN_VALID	0x20	/**< CVLAN header valid >*/
#define PPE_DRV_TUN_CMN_CTX_L2_PPPOE_VALID	0x40	/**< PPPoE header valid >*/

/*
 * L3 flags
 */
#define PPE_DRV_TUN_CMN_CTX_L3_IPV4		0x01	/**< Tunnel header is IPv4 >*/
#define PPE_DRV_TUN_CMN_CTX_L3_IPV6		0x02	/**< Tunnel header is IPv6 >*/
#define PPE_DRV_TUN_CMN_CTX_L3_INHERIT_DSCP	0x04	/**< Inherit DSCP from inner to outer >*/
#define PPE_DRV_TUN_CMN_CTX_L3_INHERIT_TTL	0x08	/**< Inherit TTL from inner to outer >*/
#define PPE_DRV_TUN_CMN_CTX_L3_UDP_ZERO_CSUM_TX	0x10	/**< Set UDP Checksum to zero for VXLAN IPV4 and IPV6 tunnels >*/
#define PPE_DRV_TUN_CMN_CTX_L3_UDP_ZERO_CSUM6_RX	0x20	/**< Allow zero UDP Checksum for VXLAN IPV6 tunnel only >*/

/*
 * GRE flags
 */
#define PPE_DRV_TUN_CMN_CTX_GRE_L_KEY		0x01	/**< Local key is enabled >*/
#define PPE_DRV_TUN_CMN_CTX_GRE_R_KEY		0x02	/**< Remote key is enabled >*/
#define PPE_DRV_TUN_CMN_CTX_GRE_L_CSUM		0x04	/**< Local checksum is enabled >*/
#define PPE_DRV_TUN_CMN_CTX_GRE_R_CSUM		0x08	/**< Remote checksum is enabled >*/

/*
 * ppe_drv_tun_cmn_ctx_encap_ecn
 *	PPE tunnel encap ecn mode
 */
enum ppe_drv_tun_cmn_ctx_encap_ecn {
	PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_NO_UPDATE,			 /**< no update ecn >*/
	PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_RFC3168_LIMIT_RFC6040_CMPAT_MODE,	 /**< RFC3168 limitation mode
									      RFC6040 compatibility mode for encapsulation >*/
	PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_RFC3168_FULL_MODE,		 /**< RFC3168 full mode for encapsulation >*/
	PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_RFC4301_RFC6040_NORMAL_MODE,	 /**< RFC4301 mode and RFC6040 normal mode for encapsulation >*/
};

/*
 * ppe_drv_tun_cmn_ctx_decap_ecn
 *	PPE tunnel decap ecn mode
 */
enum ppe_drv_tun_cmn_ctx_decap_ecn {
	PPE_DRV_TUN_CMN_CTX_DECAP_ECN_RFC3168_MODE = 0,	/* RFC3168 mode for decapsulation */
	PPE_DRV_TUN_CMN_CTX_DECAP_ECN_RFC4301_MODE = 1,	/* RFC4301 mode for decapsulation */
	PPE_DRV_TUN_CMN_CTX_DECAP_ECN_RFC6040_MODE = 2,	/* RFC6040 mode for decapsulation */
};

/*
 * ppe_drv_tun_cmn_ctx_type
 *	PPE Tunnel types
 */
enum ppe_drv_tun_cmn_ctx_type {
	PPE_DRV_TUN_CMN_CTX_TYPE_GRETAP = 1,	/**< PPE Tunnel header type GRETAP >*/
	PPE_DRV_TUN_CMN_CTX_TYPE_VXLAN,		/**< PPE Tunnel header type VxLAN >*/
	PPE_DRV_TUN_CMN_CTX_TYPE_IPIP6,		/**< PPE Tunnel header type DSLite/MAP-E >*/
	PPE_DRV_TUN_CMN_CTX_TYPE_MAPT,		/**< PPE Tunnel header type MAP-T >*/
	PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2,	/**< PPE Tunnel header type L2TP-V2 >*/
	PPE_DRV_TUN_CMN_CTX_TYPE_MAX
};

/*
 * ppe_drv_tun_cmn_ctx_l3
 *	Layer 3 header parameters
 */
struct ppe_drv_tun_cmn_ctx_l3 {
	uint32_t saddr[4];	/**< Source address >*/
	uint32_t daddr[4];	/**< Destination address >*/
	uint32_t flags;		/**< L3 header flags >*/
	uint16_t proto;		/**< IP protocol >*/
	uint8_t dscp;		/**< Static DSCP value for outer header >*/
	uint8_t ttl;		/**< Static TTL value for outer header >*/
	enum ppe_drv_tun_cmn_ctx_encap_ecn encap_ecn_mode;	/**< RFC for ECN in encap direction >*/
	enum ppe_drv_tun_cmn_ctx_decap_ecn decap_ecn_mode;	/**< RFC for ECN in decap direction >*/
};

/*
 * ppe_drv_tun_cmn_ctx_pppoe
 *	PPPoE header parameters
 */
struct ppe_drv_tun_cmn_ctx_pppoe {
	struct pppoe_hdr ph;	/**< PPPoE header >*/
	__be16 ppp_proto;	/**< PPPoE payload protocol >*/
	uint8_t server_mac[ETH_ALEN];	/** < PPPoE server mac >*/
};

/*
 * ppe_drv_tun_cmn_ctx_vlan
 * 	VLAN information
 */
struct ppe_drv_tun_cmn_ctx_vlan {
	uint16_t tpid;		/**< TPID in VLAN header. */
	uint16_t tci;		/**< TCI in VLAN header. */
};

/*
 * ppe_drv_tun_cmn_ctx_l2
 *	Layer 2 header parameters
 */
struct ppe_drv_tun_cmn_ctx_l2 {
	struct ppe_drv_tun_cmn_ctx_pppoe pppoe;		/**< PPPoE header >*/
	struct ppe_drv_tun_cmn_ctx_vlan vlan[2];	/**< VLAN header >*/
	uint16_t smac[3];				/**< Source MAC address >*/
	uint16_t dmac[3];				/**< Destination MAC address >*/
	uint8_t xmit_port;				/**< Xmit port number >*/
	uint32_t flags;					/**< L2 header flags >*/
	uint16_t eth_type;				/**< Ether type >*/
};

/*
 * ppe_drv_tun_cmn_ctx_gretap
 *	GRE header parameters
 */
struct ppe_drv_tun_cmn_ctx_gretap {
	uint32_t local_key;	/**< GRE local key >*/
	uint32_t remote_key;	/**< GRE remote key >*/
	uint16_t flags;		/**< flags to set optional fields >*/
};

/*
 * ppe_drv_tun_cmn_ctx_vxlan
 *	VxLAN header parameters
 */
struct ppe_drv_tun_cmn_ctx_vxlan {
	uint32_t vni;		/**< VxLAN Network Identifier >*/
	uint32_t flags;		/**< Header flags >*/
	uint16_t policy_id;	/**< Group Policy ID >*/
	uint16_t src_port_min;	/**< UDP source port min >*/
	uint16_t src_port_max;	/**< UDP source port max >*/
	uint16_t dest_port;	/**< UDP destination port >*/
};

/*
 * ppe_drv_tun_xlate_rule
 *	PPE tunnel transaltion rule for Map-t case
 */
struct ppe_drv_tun_cmn_ctx_xlate_rule {
	uint32_t ipv6_prefix[4];        /**< IPv6 prefix >*/
	uint32_t ipv4_prefix;           /**< IPv4 prefix >*/
	uint32_t ipv6_prefix_len;       /**< IPv6 prefix length >*/
	uint32_t ipv4_prefix_len;       /**< IPv4 prefix length >*/
	uint32_t ipv6_suffix[4];        /**< IPv6 suffix >*/
	uint32_t ipv6_suffix_len;       /**< IPv6 suffix length >*/
	uint32_t ea_len;                /**< EA bits length >*/
	uint32_t psid_offset;           /**< PSID offset >*/
};

/*
 * ppe_drv_tun_cmn_ctx_mapt
 *	MAP-T header parameters
 */
struct ppe_drv_tun_cmn_ctx_mapt {
	struct ppe_drv_tun_cmn_ctx_xlate_rule local;	/**< Local translation MAP rule >*/
	struct ppe_drv_tun_cmn_ctx_xlate_rule remote;	/**< Remote translation MAP rule >*/
};

/*
 * ppe_drv_tun_cmn_ctx_l2tp
 * 	L2TP header parameters
 */
struct ppe_drv_tun_cmn_ctx_l2tp {
	uint16_t sport;			/**< Source Port >*/
	uint16_t dport;			/**< Destination Port >*/
	uint32_t tunnel_id;		/**< Tunnel ID >*/
	uint32_t peer_tunnel_id;	/**< Peer Tunnel ID >*/
	uint32_t session_id;		/**< Session ID >*/
	uint32_t peer_session_id;	/**< Peer Session ID >*/
};

/*
 * ppe_drv_tun_cmn_ctx
 *	PPE tunnel header parameters
 */
struct ppe_drv_tun_cmn_ctx {
	struct ppe_drv_tun_cmn_ctx_l2 l2;	/**< L2 Header >*/
	struct ppe_drv_tun_cmn_ctx_l3 l3;	/**< L3 Header >*/
	union {
		struct ppe_drv_tun_cmn_ctx_gretap gre;	/**< GRE tunnel configuration >*/
		struct ppe_drv_tun_cmn_ctx_vxlan vxlan;	/**< VxLAN tunnel configuration >*/
		struct ppe_drv_tun_cmn_ctx_mapt mapt;	/**< Map-T tunnel configuration >*/
		struct ppe_drv_tun_cmn_ctx_l2tp l2tp;	/**< L2TP tunnel configuration >*/
	} tun;
	enum ppe_drv_tun_cmn_ctx_type type;
};

/*
 * ppe_drv_tun_cmn_ctx_tun_is_ipv6
 *	Check tunnel type is IPv6
 */
static inline bool ppe_drv_tun_cmn_ctx_tun_is_ipv6(struct ppe_drv_tun_cmn_ctx *th)
{
	return !!(th->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_IPV6);
}

#endif /* _PPE_DRV_TUN_CMN_CTX_H_ */
