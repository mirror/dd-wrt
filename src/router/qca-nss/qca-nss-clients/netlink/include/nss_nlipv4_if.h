/*
 **************************************************************************
 * Copyright (c) 2015-2016,2018-2019, The Linux Foundation. All rights reserved.
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
 * @file nss_nlipv4_if.h
 *	NSS Netlink IPv4 headers
 */
#ifndef __NSS_NLIPV4_IF_H
#define __NSS_NLIPV4_IF_H

/**
 * IPv4 forwarding Family
 */
#define NSS_NLIPV4_FAMILY "nss_nlipv4"
#define NSS_NLIPV4_MCAST_GRP "nss_nlipv4_mc"

#define NSS_NLIPV4_ARPHRD_IPSEC_TUNNEL_TYPE 0x31
#define NSS_NLIPV4_VLAN_ID_NOT_CONFIGURED 0xFFF

/**
 * @brief IPv4 rule
 */
struct nss_nlipv4_rule {
	struct nss_nlcmn cm;		/**< common message header */

	char flow_ifname[IFNAMSIZ];	/**< ingress interface name */
	char return_ifname[IFNAMSIZ];	/**< egress interface name */

	uint16_t flow_iftype;            /**< ingress interface type */
	uint16_t return_iftype;            /**< egress interface type */

	struct nss_ipv4_msg nim;	/**< rule message */
};

/**
 * @brief NETLINK IPv4 message init
 *
 * @param rule[IN] NSS NETLINK IPv4 rule
 * @param type[IN] IPv4 message type
 */
static inline void nss_nlipv4_rule_init(struct nss_nlipv4_rule *rule, enum nss_ipv4_message_types type)
{
	nss_nlcmn_set_ver(&rule->cm, NSS_NL_VER);
	nss_nlcmn_init_cmd(&rule->cm, sizeof(struct nss_nlipv4_rule), type);
}

#endif /* __NSS_NLIPV4_IF_H */
