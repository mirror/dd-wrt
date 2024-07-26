/*
 **************************************************************************
 * Copyright (c) 2015-2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * nss_capwapmgr_user.h
 *	NSS CAPWAP definitions for kernel and user space.
 */

#ifndef __NSS_CAPWAPMGR_USER_H
#define __NSS_CAPWAPMGR_USER_H

/*
 * Maxmimum values for rule configuration parameters.
 */
#define NSS_CAPWAP_MAX_MTU			9000
				/**< Maximum MTU supported by NSS FW. */
#define NSS_CAPWAP_MAX_BUFFER_SIZE		9000
				/**< Maximum buffer-size supported by NSS FW. */
#define NSS_CAPWAP_MAX_FRAGMENTS		10
				/**< Maximum fragments for reassembly. */
#define NSS_CAPWAP_MAX_REASSEMBLY_TIMEOUT	(10 * 1000)
				/**< Maximum timeout for reassembly - 10 seconds. */

#define NSS_CAPWAP_PADDING 14	/**< Padded packet length. */


/*
 * CAPWAP Rule configure message flags
 */
#define NSS_CAPWAP_TUNNEL_IPV4		2
				/**< IPv4 tunnel. */
#define NSS_CAPWAP_TUNNEL_IPV6		3
				/**< IPv6 tunnel. */
#define NSS_CAPWAP_TUNNEL_UDP		4
				/**< UDP tunnel. */
#define NSS_CAPWAP_TUNNEL_UDPLite	5
				/**< UDPLite tunnel. */

/*
 * CAPWAP tunnel create and type flags. These flags are used
 * to determine packet header size during encapsulation.
 */
#define NSS_CAPWAP_RULE_CREATE_VLAN_CONFIGURED	0x1
				/**< VLAN Configured for CAPWAP tunnel. */
#define NSS_CAPWAP_RULE_CREATE_PPPOE_CONFIGURED	0x2
				/**< PPPoE configured for CAPWAP tunnel. */
#define NSS_CAPWAP_ENCAP_UDPLITE_HDR_CSUM	0x4
				/**< Generate only UDP-Lite header checksum. Otherwise whole UDP-Lite payload. */

/*
 * Flow rule add types. Mutually exclusive fields.
 * This indicates whether SCS or SDWF ID is configured for inner packet lookup.
 */
#define NSS_CAPWAP_FLOW_ATTR_SCS_VALID 0x01		/**< SCS Identification valid in flow attributes. */
#define NSS_CAPWAP_FLOW_ATTR_SDWF_VALID 0x02		/**< SDWF Identification valid in flow attributes. */

/*
 * CAPWAP version
 */
#define NSS_CAPWAP_VERSION_V1		0x1
				/**< RFC CAPWAP version. */
#define NSS_CAPWAP_VERSION_V2		0x2
				/**< Initial CAPWAP version for a customer. */

/*
 * Type of packet. These are mutually exclusive fields.
 */
#define NSS_CAPWAP_PKT_TYPE_UNKNOWN	0x0000
				/**< Don't know the type of CAPWAP packet. */
#define NSS_CAPWAP_PKT_TYPE_CONTROL	0x0001
				/** It's a control CAPWAP packet src_port=5247. */
#define NSS_CAPWAP_PKT_TYPE_DATA	0x0002
				/**< It's a data CAPWAP packet src_port=5246. */

/*
 * Addtional fields for identifying what's there in the packet.
 */
#define NSS_CAPWAP_PKT_TYPE_WIRELESS_INFO	0x0008
				/**< W=1, wireless info present. */
#define NSS_CAPWAP_PKT_TYPE_802_11		0x0010
				/**< T=1, then set wbid=1. */
#define NSS_CAPWAP_PKT_TYPE_802_3		0x0020
				/**< Data is in 802.3 format. */
#define NSS_CAPWAP_PKT_TYPE_SCS_ID_VALID	0x0040
				/**< Inner SCS ID valid. */
#define NSS_CAPWAP_PKT_TYPE_SDWF_ID_VALID	0x0080
				/**< Inner SDWF ID valid. */
#define NSS_CAPWAP_PKT_TYPE_PADDED	0x4000
				/**< Packet is padded to bypass EDMA length. */

/**
 * nss_capwap_metaheader
 *	CAPWAP metaheader per-packet for both encap (TX) and decap (RX).
 */
struct nss_capwap_metaheader {
	uint8_t version;	/**< CAPWAP version. */
	uint8_t rid;		/**< Radio ID. */
	uint16_t tunnel_id;	/**< Tunnel-ID. */
	uint8_t dscp;		/**< DSCP value. */
	uint8_t vlan_pcp;	/**< VLAN priority .P marking. */
	uint16_t type;		/**< Type of CAPWAP packet & What was there in CAPWAP header. */
	uint16_t nwireless;	/**< Number of wireless info sections in CAPWAP header. */
	uint16_t wireless_qos;	/**< 802.11e qos info. */
	uint16_t outer_sgt;	/**< Security Group Tag value in the TrustSec header. */
	uint16_t inner_sgt;	/**< Security Group Tag value in the TrustSec header. */
	uint32_t flow_id;	/**< Flow identification. */
	union {
		struct {
			uint16_t vapid;		/**< VAP ID info. */
			uint16_t magic;		/**< Magic for verification purpose. Use only for debugging. */
		};
			uint32_t scs_sdwf_id;	/**< SCS or SDWF Identification. */
	};

	/*
	 * Put the wl_info at last so we don't have to do copy if 802.11 to 802.3 conversion did not happen.
	 */
	uint8_t wl_info[8];	/**< Wireless info preserved from the original packet. */
} __packed __aligned(4);

#endif /* __NSS_CAPWAP_USER_H */
