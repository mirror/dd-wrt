/*
 **************************************************************************
 * Copyright (c) 2013-2020, The Linux Foundation. All rights reserved.
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
 * @file nss_api_if.h
 *	NSS driver APIs and Declarations.
 *
 * This file declares all the public interfaces for NSS driver.
 */

#ifndef __NSS_API_IF_H
#define __NSS_API_IF_H

#ifdef __KERNEL__ /* only kernel will use. */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include "nss_arch.h"
#include "nss_def.h"
#include "nss_cmn.h"
#include "nss_tun6rd.h"
#include "nss_l2tpv2.h"
#include "nss_pptp.h"
#include "nss_map_t.h"
#include "nss_tunipip6.h"
#include "nss_lag.h"
#include "nss_stats_public.h"
#include "nss_ipv4.h"
#include "nss_ipv6.h"
#include "nss_shaper.h"
#include "nss_if.h"
#include "nss_phy_if.h"
#include "nss_virt_if.h"
#include "nss_pppoe.h"
#include "nss_crypto.h"
#include "nss_crypto_cmn.h"
#include "nss_dma.h"

#include "nss_profiler.h"
#include "nss_dynamic_interface.h"
#include "nss_ipsec.h"
#include "nss_ipsec_cmn.h"
#include "nss_gre.h"
#include "nss_gre_redir.h"
#include "nss_gre_redir_lag.h"
#include "nss_gre_tunnel.h"
#include "nss_sjack.h"
#include "nss_capwap.h"
#include "nss_wifi.h"
#include "nss_wifi_vdev.h"
#include "nss_n2h.h"
#include "nss_rps.h"
#include "nss_wifi_if.h"
#include "nss_portid.h"
#include "nss_oam.h"
#include "nss_dtls.h"
#include "nss_dtls_cmn.h"
#include "nss_tls.h"
#include "nss_edma.h"
#include "nss_bridge.h"
#include "nss_ppe.h"
#include "nss_trustsec_tx.h"
#include "nss_vlan.h"
#include "nss_igs.h"
#include "nss_mirror.h"
#include "nss_wifili_if.h"
#include "nss_project.h"
#include "nss_qrfs.h"
#include "nss_c2c_tx.h"
#include "nss_qvpn.h"
#include "nss_unaligned.h"
#include "nss_pvxlan.h"
#include "nss_vxlan.h"
#include "nss_pm.h"
#include "nss_freq.h"
#include "nss_tstamp.h"
#include "nss_gre_redir_mark.h"
#include "nss_clmap.h"
#include "nss_rmnet_rx.h"
#include "nss_match.h"
#include "nss_eth_rx.h"
#include "nss_c2c_rx.h"
#include "nss_ipv6_reasm.h"
#include "nss_ipv4_reasm.h"
#include "nss_lso_rx.h"
#include "nss_wifi_mac_db_if.h"
#endif

#endif /*__KERNEL__ */

/**
 * @addtogroup nss_driver_subsystem
 * @{
 */

#define NSS_MAX_CORES 2		/**< Maximum number of core interfaces. */

#define NSS_MAX_DEVICE_INTERFACES (NSS_MAX_PHYSICAL_INTERFACES + NSS_MAX_VIRTUAL_INTERFACES + NSS_MAX_TUNNEL_INTERFACES + NSS_MAX_DYNAMIC_INTERFACES)
		/**< Maximum number of device interfaces. */

#define NSS_MAX_NET_INTERFACES (NSS_MAX_DEVICE_INTERFACES + NSS_MAX_SPECIAL_INTERFACES)
		/**< Maximum number of network interfaces. */

#define NSS_MAX_PHYSICAL_INTERFACES 8	/**< Maximum number of physical interfaces. */
#define NSS_MAX_VIRTUAL_INTERFACES 16	/**< Maximum number of virtual interfaces. */
#define NSS_MAX_TUNNEL_INTERFACES 4	/**< Maximum number of tunnel interfaces. */
#if (NSS_FW_VERSION_CODE < NSS_FW_VERSION(11,1))
#define NSS_MAX_SPECIAL_INTERFACES 55	/**< Maximum number of special interfaces. */
#else
#define NSS_MAX_SPECIAL_INTERFACES 67	/**< Maximum number of special interfaces. */
#endif
#define NSS_MAX_WIFI_RADIO_INTERFACES 3	/**< Maximum number of radio interfaces. */

/*
 * Start of individual interface groups
 */
#define NSS_PHYSICAL_IF_START 0
		/**< Beginning of the physical interfaces. */

#define NSS_VIRTUAL_IF_START (NSS_PHYSICAL_IF_START + NSS_MAX_PHYSICAL_INTERFACES)
		/**< Beginning of the virtual interfaces. */

#define NSS_TUNNEL_IF_START (NSS_VIRTUAL_IF_START + NSS_MAX_VIRTUAL_INTERFACES)
		/**< Beginning of the tunnel interfaces. */

#define NSS_DYNAMIC_IF_START (NSS_TUNNEL_IF_START + NSS_MAX_TUNNEL_INTERFACES)
		/**< Beginning of the dynamic interfaces. */

#define NSS_SPECIAL_IF_START (NSS_DYNAMIC_IF_START + NSS_MAX_DYNAMIC_INTERFACES)
		/**< Beginning of the special interfaces. */

/*
 * Tunnel interface numbers
 */
#define NSS_IPSEC_ENCAP_IF_NUMBER (NSS_TUNNEL_IF_START + 0)
		/**< Tunnel interface number for IPsec encapsulation interfaces. */
#define NSS_IPSEC_DECAP_IF_NUMBER (NSS_TUNNEL_IF_START + 1)
		/**< Tunnel interface number for IPsec decapsulation interfaces. */
#define NSS_TUN6RD_INTERFACE (NSS_TUNNEL_IF_START + 2)
		/**< Tunnel interface number for TUN6RD interfaces. */
#define NSS_TUNIPIP6_INTERFACE (NSS_TUNNEL_IF_START + 3)
		/**< Tunnel interface number for TUNIPIP6 interfaces. */

/*
 * Special interface numbers
 */
#define NSS_N2H_INTERFACE (NSS_SPECIAL_IF_START + 0)
		/**< Special interface number for N2H. */
#define NSS_ETH_RX_INTERFACE (NSS_SPECIAL_IF_START + 2)
		/**< Special interface number for Ethernet Rx. */
#define NSS_PPPOE_INTERFACE (NSS_SPECIAL_IF_START + 3)
		/**< Special interface number for PPPoE. */
#define NSS_IPV4_RX_INTERFACE (NSS_SPECIAL_IF_START + 5)
		/**< Special interface number for IPv4. */
#define NSS_IPV6_RX_INTERFACE (NSS_SPECIAL_IF_START + 7)
		/**< Special interface number for IPv6. */
#define NSS_PROFILER_INTERFACE (NSS_SPECIAL_IF_START + 8)
		/**< Special interface number for profile. */
#define NSS_CRYPTO_INTERFACE (NSS_SPECIAL_IF_START + 9)
		/**< Special interface number for crypto CE5. */
#define NSS_DTLS_INTERFACE (NSS_SPECIAL_IF_START + 10)
		/**< Special interface number for DTLS. */
#define NSS_CRYPTO_CMN_INTERFACE (NSS_SPECIAL_IF_START + 11)
		/**< Special interface number for crypto common. */
#define NSS_C2C_TX_INTERFACE (NSS_SPECIAL_IF_START + 12)
		/**< Virtual interface number for core-to-core transmissions. */
#define NSS_C2C_RX_INTERFACE (NSS_SPECIAL_IF_START + 13)
		/**< Virtual interface number for core-to-core reception. */
#define NSS_IPSEC_CMN_INTERFACE (NSS_SPECIAL_IF_START + 18)
		/**< Virtual interface number for IPSec rule. */
#define NSS_COREFREQ_INTERFACE (NSS_SPECIAL_IF_START + 19)
		/**< Virtual interface number for core frequency. */
#define NSS_DYNAMIC_INTERFACE (NSS_SPECIAL_IF_START + 20)
		/**< Special interface number for dynamic interfaces. */
#define NSS_GRE_REDIR_INTERFACE (NSS_SPECIAL_IF_START + 21)
		/**< Special interface number for GRE redirect base interfaces. */
#define NSS_LSO_RX_INTERFACE (NSS_SPECIAL_IF_START + 22)
		/**< Special interface number for LSO. */
#define NSS_SJACK_INTERFACE (NSS_SPECIAL_IF_START + 23)
		/**< Special interface number for GRE REDIR base interfaces. */
#define NSS_IPV4_REASM_INTERFACE (NSS_SPECIAL_IF_START + 24)
		/**< Special interface number for IPv4 reassembly interfaces. */
#define NSS_DEBUG_INTERFACE (NSS_SPECIAL_IF_START + 25)
		/**< Special interface number for debug. */
#define NSS_WIFI_INTERFACE0 (NSS_SPECIAL_IF_START + 26)
		/**< Special interface number for Wi-Fi radio 0. */
#define NSS_WIFI_INTERFACE1 (NSS_SPECIAL_IF_START + 27)
		/**< Special interface number for Wi-Fi radio 1. */
#define NSS_WIFI_INTERFACE2 (NSS_SPECIAL_IF_START + 28)
		/**< Special interface number for Wi-Fi radio 2. */
#define NSS_IPV6_REASM_INTERFACE (NSS_SPECIAL_IF_START + 29)
		/**< Special interface number for IPv6 reassembly. */
#define NSS_LAG0_INTERFACE_NUM (NSS_SPECIAL_IF_START + 30)
		/**< Special interface number for LAG0. */
#define NSS_LAG1_INTERFACE_NUM (NSS_SPECIAL_IF_START + 31)
		/**< Special interface number for LAG1. */
#define NSS_LAG2_INTERFACE_NUM (NSS_SPECIAL_IF_START + 32)
		/**< Special interface number for LAG2. */
#define NSS_LAG3_INTERFACE_NUM (NSS_SPECIAL_IF_START + 33)
		/**< Special interface number for LAG3. */
#define NSS_L2TPV2_INTERFACE (NSS_SPECIAL_IF_START + 34)
		/**< Special interface number for L2TPv2 UDP encapsulation. */
#define NSS_PPTP_INTERFACE (NSS_SPECIAL_IF_START + 36)
		/**< Special interface number for PPTP-to-decapsulation. */
#define NSS_PORTID_INTERFACE (NSS_SPECIAL_IF_START + 37)
		/**< Special interface number for port ID. */
#define NSS_OAM_INTERFACE (NSS_SPECIAL_IF_START + 38)
		/**< Special interface number for OAM. */
#define NSS_MAP_T_INTERFACE (NSS_SPECIAL_IF_START + 39)
		/**< Special interface number for MAP-T. */
#define NSS_PPE_INTERFACE (NSS_SPECIAL_IF_START + 40)
		/**< Special interface number for PPE. */
#define NSS_EDMA_INTERFACE (NSS_SPECIAL_IF_START + 41)
		/**< Special interface number for EDMA. */
#define NSS_GRE_TUNNEL_INTERFACE (NSS_SPECIAL_IF_START + 42)
		/**< Special interface number for NSS GRE tunnel. */
#define NSS_TRUSTSEC_TX_INTERFACE (NSS_SPECIAL_IF_START + 43)
		/**< Special interface number for TrustSec Tx. */
#define NSS_VAP_INTERFACE (NSS_SPECIAL_IF_START + 44)
		/**< Special interface number for NSS Wi-Fi VAPs base interfaces. */
#define NSS_VLAN_INTERFACE (NSS_SPECIAL_IF_START + 45)
		/**< Special interface number for VLAN. */
#define NSS_GRE_INTERFACE (NSS_SPECIAL_IF_START + 46)
		/**< Special interface number for GRE. */
#define NSS_WIFILI_INTERNAL_INTERFACE (NSS_SPECIAL_IF_START + 47)
		/**< Special interface number for wifili internal instance. */
#define NSS_PROJECT_INTERFACE (NSS_SPECIAL_IF_START + 48)
		/**< Special interface number for project node. */
#define NSS_PBUF_MGR_FREE_INTERFACE (NSS_SPECIAL_IF_START + 49)
		/**< Special interface number for PBUF_MGR_FREE node. */
#define NSS_REDIR_RX_INTERFACE (NSS_SPECIAL_IF_START + 50)
		/**< Special interface number for 802.3 redirect node. */
#define NSS_QRFS_INTERFACE (NSS_SPECIAL_IF_START + 51)
		/**< Special interface number for QRFS. */
#define NSS_GRE_REDIR_LAG_INTERFACE (NSS_SPECIAL_IF_START + 52)
		/**< Special interface number for GRE redirect link aggregation interface. */
#define NSS_UNALIGNED_INTERFACE (NSS_SPECIAL_IF_START + 53)
		/**< Special interface number for unaligned handler. */
#define NSS_TSTAMP_TX_INTERFACE (NSS_SPECIAL_IF_START + 54)
		/**< Special interface number for timestamp transmit. */
#define NSS_TSTAMP_RX_INTERFACE (NSS_SPECIAL_IF_START + 55)
		/**< Special interface number for timestamp receive. */
#define NSS_GRE_REDIR_MARK_INTERFACE (NSS_SPECIAL_IF_START + 56)
		/**< Special interface number for GRE redirect mark. */
#if (NSS_FW_VERSION_CODE < NSS_FW_VERSION(11,1))
#define NSS_RMNET_RX_INTERFACE (NSS_SPECIAL_IF_START + 57)
		/**< Special interface number for RMNET receive handler. */
#else
#define NSS_VXLAN_INTERFACE (NSS_SPECIAL_IF_START + 57)
		/**< Special interface number for VxLAN handler. */
#define NSS_RMNET_RX_INTERFACE (NSS_SPECIAL_IF_START + 58)
		/**< Special interface number for remote wireless wide area network receive handler. */
#define NSS_WIFILI_EXTERNAL_INTERFACE0 (NSS_SPECIAL_IF_START + 59)
		/**< Special interface number for first external radio instance. */
#define NSS_WIFILI_EXTERNAL_INTERFACE1 (NSS_SPECIAL_IF_START + 60)
		/**< Special interface number for second external radio instance. */
#define NSS_TLS_INTERFACE (NSS_SPECIAL_IF_START + 61)
		/**< Special interface number for TLS. */
#define NSS_PPE_VP_INTERFACE (NSS_SPECIAL_IF_START + 62)
		/**< Special interface number for the virtual port (62, 63, 64) interface. */
#define NSS_WIFI_MAC_DB_INTERFACE (NSS_SPECIAL_IF_START + 65)
		/**< Special interface number for the Wi-Fi MAC database. */
#define NSS_DMA_INTERFACE (NSS_SPECIAL_IF_START + 66)
		/**< Special interface number for the DMA interface. */
#endif

#ifdef __KERNEL__ /* only kernel will use. */

/**
 * Wireless Multimedia Extention Access Category to TID. @hideinitializer
 */
#define NSS_WIFILI_WME_AC_TO_TID(_ac) (	\
		((_ac) == NSS_WIFILI_WME_AC_VO) ? 6 : \
		(((_ac) == NSS_WIFILI_WME_AC_VI) ? 5 : \
		(((_ac) == NSS_WIFILI_WME_AC_BK) ? 1 : \
		0)))

/**
 * Wireless TID to Wireless Extension Multimedia Access Category. @hideinitializer
 */
#define NSS_WIFILI_TID_TO_WME_AC(_tid) (	\
		(((_tid) == 0) || ((_tid) == 3)) ? NSS_WIFILI_WME_AC_BE : \
		((((_tid) == 1) || ((_tid) == 2)) ? NSS_WIFILI_WME_AC_BK : \
		((((_tid) == 4) || ((_tid) == 5)) ? NSS_WIFILI_WME_AC_VI : \
		NSS_WIFILI_WME_AC_VO)))

/**
 * Converts the format of an IPv6 address from Linux to NSS. @hideinitializer
 */
#define IN6_ADDR_TO_IPV6_ADDR(ipv6, in6) \
	{ \
		((uint32_t *)ipv6)[0] = in6.in6_u.u6_addr32[0]; \
		((uint32_t *)ipv6)[1] = in6.in6_u.u6_addr32[1]; \
		((uint32_t *)ipv6)[2] = in6.in6_u.u6_addr32[2]; \
		((uint32_t *)ipv6)[3] = in6.in6_u.u6_addr32[3]; \
	}

/**
 * Converts the format of an IPv6 address from NSS to Linux. @hideinitializer
 */
#define IPV6_ADDR_TO_IN6_ADDR(in6, ipv6) \
	{ \
		in6.in6_u.u6_addr32[0] = ((uint32_t *)ipv6)[0]; \
		in6.in6_u.u6_addr32[1] = ((uint32_t *)ipv6)[1]; \
		in6.in6_u.u6_addr32[2] = ((uint32_t *)ipv6)[2]; \
		in6.in6_u.u6_addr32[3] = ((uint32_t *)ipv6)[3]; \
	}

/**
 * Format of an IPv6 address (16 * 8 bits).
 */
#define IPV6_ADDR_OCTAL_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"

/**
 * Prints an IPv6 address (16 * 8 bits).
 */
#define IPV6_ADDR_TO_OCTAL(ipv6) ((uint16_t *)ipv6)[0], ((uint16_t *)ipv6)[1], ((uint16_t *)ipv6)[2], ((uint16_t *)ipv6)[3], ((uint16_t *)ipv6)[4], ((uint16_t *)ipv6)[5], ((uint16_t *)ipv6)[6], ((uint16_t *)ipv6)[7]

/*
 * IPv4 rule sync reasons.
 */
#define NSS_IPV4_SYNC_REASON_STATS 0	/**< Rule for synchronizing statistics. */
#define NSS_IPV4_SYNC_REASON_FLUSH 1	/**< Rule for flushing a cache entry. */
#define NSS_IPV4_SYNC_REASON_EVICT 2	/**< Rule for evicting a cache entry. */
#define NSS_IPV4_SYNC_REASON_DESTROY 3
		/**< Rule for destroying a cache entry (requested by the host OS). */
#define NSS_IPV4_SYNC_REASON_PPPOE_DESTROY 4
		/**< Rule for destroying a cache entry that belongs to a PPPoE session. */

/**
 * nss_ipv4_create
 *	Information for an IPv4 flow or connection create rule.
 *
 * All fields must be passed in host-endian order.
 */
struct nss_ipv4_create {
	int32_t src_interface_num;
				/**< Source interface number (virtual or physical). */
	int32_t dest_interface_num;
				/**< Destination interface number (virtual or physical). */
	int32_t protocol;	/**< L4 protocol (e.g., TCP or UDP). */
	uint32_t flags;		/**< Flags (if any) associated with this rule. */
	uint32_t from_mtu;	/**< MTU of the incoming interface. */
	uint32_t to_mtu;	/**< MTU of the outgoing interface. */
	uint32_t src_ip;	/**< Source IP address. */
	int32_t src_port;	/**< Source L4 port (e.g., TCP or UDP port). */
	uint32_t src_ip_xlate;	/**< Translated source IP address (used with SNAT). */
	int32_t src_port_xlate;	/**< Translated source L4 port (used with SNAT). */
	uint32_t dest_ip;	/**< Destination IP address. */
	int32_t dest_port;	/**< Destination L4 port (e.g., TCP or UDP port). */
	uint32_t dest_ip_xlate;
			/**< Translated destination IP address (used with DNAT). */
	int32_t dest_port_xlate;
			/**< Translated destination L4 port (used with DNAT). */
	uint8_t src_mac[ETH_ALEN];
			/**< Source MAC address. */
	uint8_t dest_mac[ETH_ALEN];
			/**< Destination MAC address. */
	uint8_t src_mac_xlate[ETH_ALEN];
			/**< Translated source MAC address (post-routing). */
	uint8_t dest_mac_xlate[ETH_ALEN];
			/**< Translated destination MAC address (post-routing). */
	uint8_t flow_window_scale;	/**< Window scaling factor (TCP). */
	uint32_t flow_max_window;	/**< Maximum window size (TCP). */
	uint32_t flow_end;		/**< TCP window end. */
	uint32_t flow_max_end;		/**< TCP window maximum end. */
	uint32_t flow_pppoe_if_exist;
			/**< Flow direction: PPPoE interface exist flag. */
	int32_t flow_pppoe_if_num;
			/**< Flow direction: PPPoE interface number. */
	uint16_t ingress_vlan_tag;	/**< Ingress VLAN tag expected for this flow. */
	uint8_t return_window_scale;
			/**< Window scaling factor of the return direction (TCP). */
	uint32_t return_max_window;
			/**< Maximum window size of the return direction. */
	uint32_t return_end;
			/**< Flow end for the return direction. */
	uint32_t return_max_end;
			/**< Flow maximum end for the return direction. */
	uint32_t return_pppoe_if_exist;
			/**< Return direction: PPPoE interface existence flag. */
	int32_t return_pppoe_if_num;
			/**< Return direction: PPPoE interface number. */
	uint16_t egress_vlan_tag;	/**< Egress VLAN tag expected for this flow. */
	uint8_t spo_needed;		/**< Indicates whether SPO is required. */
	uint32_t param_a0;		/**< Custom parameter 0. */
	uint32_t param_a1;		/**< Custom parameter 1. */
	uint32_t param_a2;		/**< Custom parameter 2. */
	uint32_t param_a3;		/**< Custom parameter 3. */
	uint32_t param_a4;		/**< Custom parameter 4. */
	uint32_t qos_tag;		/**< Deprecated, will be removed soon. */
	uint32_t flow_qos_tag;		/**< QoS tag value for the flow direction. */
	uint32_t return_qos_tag;	/**< QoS tag value for the return direction. */
	uint8_t dscp_itag;		/**< DSCP marking tag. */
	uint8_t dscp_imask;		/**< DSCP marking input mask. */
	uint8_t dscp_omask;		/**< DSCP marking output mask. */
	uint8_t dscp_oval;		/**< DSCP marking output value. */
	uint16_t vlan_itag;		/**< VLAN marking tag. */
	uint16_t vlan_imask;		/**< VLAN marking input mask. */
	uint16_t vlan_omask;		/**< VLAN marking output mask. */
	uint16_t vlan_oval;		/**< VLAN marking output value. */
	uint32_t in_vlan_tag[MAX_VLAN_DEPTH];
			/**< Ingress VLAN tag expected for this flow. */
	uint32_t out_vlan_tag[MAX_VLAN_DEPTH];
			/**< Egress VLAN tag expected for this flow. */
	uint8_t flow_dscp;		/**< IP DSCP value for the flow direction. */
	uint8_t return_dscp;		/**< IP DSCP value for the return direction. */
};

/*
 * IPv4 connection flags (to be used with nss_ipv4_create::flags).
 */
#define NSS_IPV4_CREATE_FLAG_NO_SEQ_CHECK 0x01
		/**< Rule for not checking sequence numbers. */
#define NSS_IPV4_CREATE_FLAG_BRIDGE_FLOW 0x02
		/**< Rule that indicates pure bridge flow (no routing is involved). */
#define NSS_IPV4_CREATE_FLAG_ROUTED 0x04	/**< Rule for a routed connection. */

#define NSS_IPV4_CREATE_FLAG_DSCP_MARKING 0x08	/**< Rule for DSCP marking. */
#define NSS_IPV4_CREATE_FLAG_VLAN_MARKING 0x10	/**< Rule for VLAN marking. */
#define NSS_IPV4_CREATE_FLAG_QOS_VALID 0x20	/**< Rule for QoS is valid. */

/**
 * nss_ipv4_destroy
 *	Information for an IPv4 flow or connection destroy rule.
 */
struct nss_ipv4_destroy {
	int32_t protocol;	/**< L4 protocol ID. */
	uint32_t src_ip;	/**< Source IP address. */
	int32_t src_port;	/**< Source L4 port (e.g., TCP or UDP port). */
	uint32_t dest_ip;	/**< Destination IP address. */
	int32_t dest_port;	/**< Destination L4 port (e.g., TCP or UDP port). */
};

/*
 * IPv6 rule sync reasons.
 */
#define NSS_IPV6_SYNC_REASON_STATS 0	/**< Rule for synchronizing statistics. */
#define NSS_IPV6_SYNC_REASON_FLUSH 1	/**< Rule for flushing a cache entry. */
#define NSS_IPV6_SYNC_REASON_EVICT 2	/**< Rule for evicting a cache entry. */
#define NSS_IPV6_SYNC_REASON_DESTROY 3
		/**< Rule for destroying a cache entry (requested by the host OS). */
#define NSS_IPV6_SYNC_REASON_PPPOE_DESTROY 4
		/**< Rule for destroying a cache entry that belongs to a PPPoE session. */

/**
 * nss_ipv6_create
 *	Information for an IPv6 flow or connection create rule.
 *
 * All fields must be passed in host-endian order.
 */
struct nss_ipv6_create {
	int32_t src_interface_num;
			/**< Source interface number (virtual or physical). */
	int32_t dest_interface_num;
			/**< Destination interface number (virtual or physical). */
	int32_t protocol;	/**< L4 protocol (e.g., TCP or UDP). */
	uint32_t flags;		/**< Flags (if any) associated with this rule. */
	uint32_t from_mtu;	/**< MTU of the incoming interface. */
	uint32_t to_mtu;	/**< MTU of the outgoing interface. */
	uint32_t src_ip[4];	/**< Source IP address. */
	int32_t src_port;	/**< Source L4 port (e.g., TCP or UDP port). */
	uint32_t dest_ip[4];	/**< Destination IP address. */
	int32_t dest_port;	/**< Destination L4 port (e.g., TCP or UDP port). */
	uint8_t src_mac[ETH_ALEN];	/**< Source MAC address. */
	uint8_t dest_mac[ETH_ALEN];	/**< Destination MAC address. */
	uint8_t flow_window_scale;	/**< Window scaling factor (TCP). */
	uint32_t flow_max_window;	/**< Maximum window size (TCP). */
	uint32_t flow_end;		/**< TCP window end. */
	uint32_t flow_max_end;		/**< TCP window maximum end. */
	uint32_t flow_pppoe_if_exist;
			/**< Flow direction: PPPoE interface existence flag. */
	int32_t flow_pppoe_if_num;
			/**< Flow direction: PPPoE interface number. */
	uint16_t ingress_vlan_tag;
			/**< Ingress VLAN tag expected for this flow. */
	uint8_t return_window_scale;
			/**< Window scaling factor (TCP) for the return direction. */
	uint32_t return_max_window;
			/**< Maximum window size (TCP) for the return direction. */
	uint32_t return_end;
			/**< End for the return direction. */
	uint32_t return_max_end;
			/**< Maximum end for the return direction. */
	uint32_t return_pppoe_if_exist;
			/**< Return direction: PPPoE interface exist flag. */
	int32_t return_pppoe_if_num;
			/**< Return direction: PPPoE interface number. */
	uint16_t egress_vlan_tag;	/**< Egress VLAN tag expected for this flow. */
	uint32_t qos_tag;		/**< Deprecated; will be removed soon. */
	uint32_t flow_qos_tag;		/**< QoS tag value for flow direction. */
	uint32_t return_qos_tag;	/**< QoS tag value for the return direction. */
	uint8_t dscp_itag;		/**< DSCP marking tag. */
	uint8_t dscp_imask;		/**< DSCP marking input mask. */
	uint8_t dscp_omask;		/**< DSCP marking output mask. */
	uint8_t dscp_oval;		/**< DSCP marking output value. */
	uint16_t vlan_itag;		/**< VLAN marking tag. */
	uint16_t vlan_imask;		/**< VLAN marking input mask. */
	uint16_t vlan_omask;		/**< VLAN marking output mask. */
	uint16_t vlan_oval;		/**< VLAN marking output value. */
	uint32_t in_vlan_tag[MAX_VLAN_DEPTH];
					/**< Ingress VLAN tag expected for this flow. */
	uint32_t out_vlan_tag[MAX_VLAN_DEPTH];
					/**< Egress VLAN tag expected for this flow. */
	uint8_t flow_dscp;		/**< IP DSCP value for flow direction. */
	uint8_t return_dscp;		/**< IP DSCP value for the return direction. */
};

/*
 * IPv6 connection flags (to be used with nss_ipv6_create::flags.
 */
#define NSS_IPV6_CREATE_FLAG_NO_SEQ_CHECK 0x1
		/**< Indicates that sequence numbers are not to be checked. */
#define NSS_IPV6_CREATE_FLAG_BRIDGE_FLOW 0x02
		/**< Indicates that this is a pure bridge flow (no routing is involved). */
#define NSS_IPV6_CREATE_FLAG_ROUTED 0x04	/**< Rule is for a routed connection. */
#define NSS_IPV6_CREATE_FLAG_DSCP_MARKING 0x08	/**< Rule for DSCP marking. */
#define NSS_IPV6_CREATE_FLAG_VLAN_MARKING 0x10	/**< Rule for VLAN marking. */
#define NSS_IPV6_CREATE_FLAG_QOS_VALID 0x20	/**< Rule for Valid QoS. */

/**
 * nss_ipv6_destroy
 *	Information for an IPv6 flow or connection destroy rule.
 */
struct nss_ipv6_destroy {
	int32_t protocol;	/**< L4 protocol (e.g., TCP or UDP). */
	uint32_t src_ip[4];	/**< Source IP address. */
	int32_t src_port;	/**< Source L4 port (e.g., TCP or UDP port). */
	uint32_t dest_ip[4];	/**< Destination IP address. */
	int32_t dest_port;	/**< Destination L4 port (e.g., TCP or UDP port). */
};

/**
 * nss_ipv4_sync
 *	Defines packet statistics for IPv4 and also keeps the connection entry alive.
 *
 * Statistics are bytes and packets seen over a connection.
 *
 * The addresses are NON-NAT addresses (i.e., true endpoint
 * addressing).
 *
 * The source (src) creates the connection.
 */
struct nss_ipv4_sync {
	uint32_t index;		/**< Slot ID for cache statistics to host OS. */
			/*TODO: use an opaque information as host and NSS
			  may be using a different mechanism to store rules. */
	int32_t protocol;	/**< L4 protocol (e.g., TCP or UDP). */
	uint32_t src_ip;	/**< Source IP address. */
	int32_t src_port;	/**< Source L4 port (e.g., TCP or UDP port). */
	uint32_t src_ip_xlate;	/**< Translated source IP address (used with SNAT). */
	int32_t src_port_xlate;	/**< Translated source L4 port (used with SNAT). */
	uint32_t dest_ip;	/**< Destination IP address. */
	int32_t dest_port;	/**< Destination L4 port (e.g., TCP or UDP port). */
	uint32_t dest_ip_xlate;
			/**< Translated destination IP address (used with DNAT). */
	int32_t dest_port_xlate;
			/**< Translated destination L4 port (used with DNAT). */
	uint32_t flow_max_window;	/**< Maximum window size (TCP). */
	uint32_t flow_end;		/**< TCP window end. */
	uint32_t flow_max_end;		/**< TCP window maximum end. */
	uint32_t flow_rx_packet_count;	/**< Rx packet count for the flow interface. */
	uint32_t flow_rx_byte_count;	/**< Rx byte count for the flow interface. */
	uint32_t flow_tx_packet_count;	/**< Tx packet count for the flow interface. */
	uint32_t flow_tx_byte_count;	/**< Tx byte count for the flow interface. */
	uint32_t return_max_window;
			/**< Maximum window size (TCP) for the return direction. */
	uint32_t return_end;
			/**< End for the return direction. */
	uint32_t return_max_end;
			/**< Maximum end for the return direction. */
	uint32_t return_rx_packet_count;
			/**< Rx packet count for the return direction. */
	uint32_t return_rx_byte_count;
			/**< Rx byte count for the return direction. */
	uint32_t return_tx_packet_count;
			/**< Tx packet count for the return direction. */
	uint32_t return_tx_byte_count;
			/**< Tx byte count for the return direction. */

	/**
	 * Time in Linux jiffies to be added to the current timeout to keep the
	 * connection alive.
	 */
	unsigned long int delta_jiffies;

	uint8_t reason;		/**< Reason for synchronization. */
	uint32_t param_a0;	/**< Custom parameter 0. */
	uint32_t param_a1;	/**< Custom parameter 1. */
	uint32_t param_a2;	/**< Custom parameter 2. */
	uint32_t param_a3;	/**< Custom parameter 3. */
	uint32_t param_a4;	/**< Custom parameter 4. */

	uint8_t flags;		/**< Flags indicating the status of the flow. */
	uint32_t qos_tag;	/**< QoS value of the flow. */
};

/**
 * nss_ipv4_establish
 *	Defines connection-established message parameters for IPv4.
 */
struct nss_ipv4_establish {
	uint32_t index;			/**< Slot ID for cache statistics to host OS. */
			/*TODO: use an opaque information as host and NSS
			  may be using a different mechanism to store rules. */
	uint8_t protocol;		/**< Protocol number. */
	uint8_t reserved[3];		/**< Padding for word alignment. */
	int32_t flow_interface;		/**< Flow interface number. */
	uint32_t flow_mtu;		/**< MTU for the flow interface. */
	uint32_t flow_ip;		/**< Flow IP address. */
	uint32_t flow_ip_xlate;		/**< Translated flow IP address. */
	uint32_t flow_ident;		/**< Flow identifier (e.g., port). */
	uint32_t flow_ident_xlate;	/**< Translated flow identifier (e.g., port). */
	uint16_t flow_mac[3];		/**< Source MAC address for the flow direction. */
	uint32_t flow_pppoe_if_exist;	/**< Flow direction: PPPoE interface existence flag. */
	int32_t flow_pppoe_if_num;	/**< Flow direction: PPPoE interface number. */
	uint16_t ingress_vlan_tag;	/**< Ingress VLAN tag. */
	int32_t return_interface;	/**< Return interface number. */
	uint32_t return_mtu;		/**< MTU for the return interface. */
	uint32_t return_ip;		/**< Return IP address. */
	uint32_t return_ip_xlate;	/**< Translated return IP address. */
	uint32_t return_ident;		/**< Return identier (e.g., port). */
	uint32_t return_ident_xlate;	/**< Translated return identifier (e.g., port). */
	uint16_t return_mac[3];		/**< Source MAC address for the return direction. */
	uint32_t return_pppoe_if_exist;	/**< Return direction: PPPoE interface existence flag. */
	int32_t return_pppoe_if_num;	/**< Return direction: PPPoE interface number. */
	uint16_t egress_vlan_tag;	/**< Egress VLAN tag. */
	uint8_t flags;			/**< Flags indicating the status of the flow. */
	uint32_t qos_tag;		/**< QoS value of the flow. */
};

/**
 * nss_ipv4_cb_reason
 *	Reasons for an IPv4 callback.
 */
enum nss_ipv4_cb_reason {
	NSS_IPV4_CB_REASON_ESTABLISH = 0,
	NSS_IPV4_CB_REASON_SYNC,
	NSS_IPV4_CB_REASON_ESTABLISH_FAIL,
};

/**
 * nss_ipv4_cb_params
 *	Message parameters for an IPv4 callback.
 */
struct nss_ipv4_cb_params {
	enum nss_ipv4_cb_reason reason;		/**< Reason for the callback. */

	/**
	 * Message parameters for an IPv4 callback.
	 */
	union {
		struct nss_ipv4_sync sync;
				/**< Parameters for synchronization. */
		struct nss_ipv4_establish establish;
				/**< Parameters for establishing a connection. */
	} params;		/**< Payload of parameters. */
};

/**
 * nss_ipv6_sync
 *	Update packet statistics (bytes and packets seen over a connection) and also keep the connection entry alive.
 *
 * The addresses are NON-NAT addresses (i.e., true endpoint addressing).
 *
 * The source (src) creates the connection.
 */
struct nss_ipv6_sync {
	uint32_t index;		/**< Slot ID for cache statistics to the host OS. */
	int32_t protocol;	/**< L4 protocol (e.g., TCP or UDP). */
	uint32_t src_ip[4];	/**< Source IP address. */
	int32_t src_port;	/**< Source L4 port (e.g., TCP or UDP port). */
	uint32_t dest_ip[4];	/**< Destination IP address. */
	int32_t dest_port;	/**< Destination L4 port (e.g., TCP or UDP port). */
	uint32_t flow_max_window;	/**< Maximum window size (TCP). */
	uint32_t flow_end;		/**< TCP window end. */
	uint32_t flow_max_end;		/**< TCP window maximum end. */
	uint32_t flow_rx_packet_count;	/**< Rx packet count for the flow interface. */
	uint32_t flow_rx_byte_count;	/**< Rx byte count for the flow interface. */
	uint32_t flow_tx_packet_count;	/**< Tx packet count for the flow interface. */
	uint32_t flow_tx_byte_count;	/**< Tx byte count for the flow interface. */
	uint32_t return_max_window;
			/**< Maximum window size (TCP) for the return direction. */
	uint32_t return_end;
			/**< End for the return direction. */
	uint32_t return_max_end;
			/**< Maximum end for the return direction. */
	uint32_t return_rx_packet_count;
			/**< Rx packet count for the return direction. */
	uint32_t return_rx_byte_count;
			/**< Rx byte count for the return direction. */
	uint32_t return_tx_packet_count;
			/**< Tx packet count for the return direction. */
	uint32_t return_tx_byte_count;
			/**< Tx byte count for the return direction. */

	/**
	 * Time in Linux jiffies to be added to the current timeout to keep the
	 * connection alive.
	 */
	unsigned long int delta_jiffies;

	/**
	 * Non-zero when the NA has ceased to accelerate the given connection.
	 */
	uint8_t final_sync;

	uint8_t evicted;	/**< Non-zero if the connection is evicted. */

	uint8_t flags;		/**< Flags indicating the status of the flow. */
	uint32_t qos_tag;	/**< QoS value of the flow. */
};

/**
 * nss_ipv6_establish
 *	Defines connection-established message parameters for IPv6.
 */
struct nss_ipv6_establish {
	uint32_t index;		/**< Slot ID for cache statistics to the host OS. */
	uint8_t protocol;	/**< Protocol number. */
	int32_t flow_interface;	/**< Flow interface number. */
	uint32_t flow_mtu;	/**< MTU for the flow interface. */
	uint32_t flow_ip[4];	/**< Flow IP address. */
	uint32_t flow_ident;	/**< Flow identifier (e.g., port). */
	uint16_t flow_mac[3];	/**< Source MAC address for the flow direction. */
	uint32_t flow_pppoe_if_exist;	/**< Flow direction: PPPoE interface existence flag. */
	int32_t flow_pppoe_if_num;	/**< Flow direction: PPPoE interface number. */
	uint16_t ingress_vlan_tag;	/**< Ingress VLAN tag. */
	int32_t return_interface;	/**< Return interface number. */
	uint32_t return_mtu;		/**< MTU for the return interface. */
	uint32_t return_ip[4];		/**< Return IP address. */
	uint32_t return_ident;		/**< Return identier (e.g., port). */
	uint16_t return_mac[3];		/**< Source MAC address for the return direction. */
	uint32_t return_pppoe_if_exist;	/**< Return direction: PPPoE interface existence flag. */
	int32_t return_pppoe_if_num;	/**< Return direction: PPPoE interface number. */
	uint16_t egress_vlan_tag;	/**< VLAN tag to be inserted for egress direction. */
	uint8_t flags;			/**< Flags indicating the status of the flow. */
	uint32_t qos_tag;		/**< QoS value of the flow. */
};

/**
 * nss_ipv6_cb_reason
 *	Reasons for an IPv6 callback.
 */
enum nss_ipv6_cb_reason {
	NSS_IPV6_CB_REASON_ESTABLISH = 0,
	NSS_IPV6_CB_REASON_SYNC,
	NSS_IPV6_CB_REASON_ESTABLISH_FAIL,
};

/**
 * nss_ipv6_cb_params
 *	Message parameters for an IPv6 callback.
 */
struct nss_ipv6_cb_params {
	enum nss_ipv6_cb_reason reason;		/**< Reason for the callback. */

	/**
	 * Message parameters for an IPv6 callback.
	 */
	union {
		struct nss_ipv6_sync sync;
				/**< Parameters for synchronization. */
		struct nss_ipv6_establish establish;
				/**< Parameters for establishing a connection. */
	} params;		/**< Callback parameters. */
};

/*
 * General utilities
 */

/**
 * General callback function for all interface messages.
 *
 * @datatypes
 * nss_cmn_msg
 *
 * @param[in] app_data  Pointer to the application context of the message.
 * @param[in] msg       Pointer to the message data.
 */
typedef void (*nss_if_rx_msg_callback_t)(void *app_data, struct nss_cmn_msg *msg);

/**
 * Callback function for IPv4 connection synchronization messages.
 *
 * @datatypes
 * nss_ipv4_cb_params
 *
 * @param[in] nicb  Pointer to the parameter structure for an NSS IPv4 callback.
 */
typedef void (*nss_ipv4_callback_t)(struct nss_ipv4_cb_params *nicb);

/**
 * nss_get_state
 *	Gets the NSS state.
 *
 * @param[in] nss_ctx  Pointer to the NSS context.
 *
 * @return
 * NSS state.
 */
extern nss_state_t nss_get_state(void *nss_ctx);

#endif /*__KERNEL__ */

/*
 * Once Everything is arranged correctly, will be placed at top
 */

/**
 *@}
 */

#endif /** __NSS_API_IF_H */
