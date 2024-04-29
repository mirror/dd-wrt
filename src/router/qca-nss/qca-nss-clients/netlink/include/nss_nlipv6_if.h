/*
 **************************************************************************
 * Copyright (c) 2016,2018-2020, The Linux Foundation. All rights reserved.
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
 * @file nss_nlipv6_if.h
 *	NSS Netlink IPv6 headers
 */
#ifndef __NSS_NLIPV6_IF_H
#define __NSS_NLIPV6_IF_H

/**
 * IPv6 forwarding Family
 */
#define NSS_NLIPV6_FAMILY "nss_nlipv6"
#define NSS_NLIPV6_MCAST_GRP "nss_nlipv6_mc"
#define NSS_NLIPV6_ARPHRD_IPSEC_TUNNEL_TYPE 0x31
#define NSS_NLIPV6_VLAN_ID_NOT_CONFIGURED 0xFFF
#define NSS_NLIPV6_MIN_MTU 1280
#define NSS_NLIPV6_MAX_MTU 65535

#define NSS_NLIPV6_ADDR_BITS (sizeof(uint32_t) * 4 * BITS_PER_BYTE)	/* 128 bits */
#define NSS_NLIPV6_SUBNET_BITS (sizeof(uint32_t) * 4 * BITS_PER_BYTE)	/* 128 bits */

/**
 * @brief IPv6 rule
 */
struct nss_nlipv6_rule {
	struct nss_nlcmn cm;				/**< common message header */

	char flow_ifname[IFNAMSIZ];			/**< ingress interface name */
	char return_ifname[IFNAMSIZ];			/**< egress interface name */

	enum nss_nl_iftype flow_iftype;			/**< ingress interface type */
	enum nss_nl_iftype return_iftype;		/**< egress interface type */

	struct nss_ipv6_msg nim;			/**< rule message */
	struct nss_ipv6_stats_notification stats;	/**< NSS statistics */
};

/**
 * @brief NETLINK IPv6 message init
 *
 * @param rule[IN] NSS NETLINK IPv6 rule
 * @param type[IN] IPv6 message type
 */
static inline void nss_nlipv6_rule_init(struct nss_nlipv6_rule *rule, enum nss_ipv6_message_types type)
{
	nss_nlcmn_set_ver(&rule->cm, NSS_NL_VER);
	nss_nlcmn_init_cmd(&rule->cm, sizeof(struct nss_nlipv6_rule), type);
}

/**
 * @brief convert ipv6 address to NSS format
 *
 * @param rule[IN] IPv6 address
 */
static inline void nss_nlipv6_swap_addr(uint32_t *src, uint32_t *dst)
{
	uint32_t temp[4];

	if (src == dst) {
		memcpy(temp, src, sizeof(temp));
		src = temp;
	}

	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];

}

#endif /* __NSS_NLIPV6_IF_H */
