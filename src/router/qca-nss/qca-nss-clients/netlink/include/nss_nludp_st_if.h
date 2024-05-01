/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

/*
 * @file nss_nludp_st_if.h
 *	NSS Netlink udp_st headers
 */
#ifndef __NSS_NLUDP_ST_IF_H
#define __NSS_NLUDP_ST_IF_H
#include "nss_udp_st.h"

#define NSS_NLUDP_ST_MAX_MTU 65535

/**
 * UDP speed test forwarding Family
 */
#define NSS_NLUDP_ST_FAMILY "nss_nludp_st"
#define NSS_NLUDP_ST_MCAST_GRP "nss_nludp_st_mc"

/**
 * UDP speed test rule.
 */
struct nss_nludp_st_rule {
        struct nss_nlcmn cm;				/**< Common message header. */
	char gmac_ifname[IFNAMSIZ];			/**< GMAC interface name. */
	struct nss_udp_st_msg num;			/**< UDP ST Rule message. */
};

/**
 * nss_nludp_st_rule_init
 *	NETLINK udp_st message init.
 *
 * @param[in] rule NSS Netlink UDP speed test rule.
 * @param[in] type UDP speed test message type.
 *
 * @return
 * None.
 */
static inline void nss_nludp_st_rule_init(struct nss_nludp_st_rule *rule, enum nss_udp_st_message_types type)
{
	nss_nlcmn_set_ver(&rule->cm, NSS_NL_VER);
	nss_nlcmn_init_cmd(&rule->cm, sizeof(struct nss_nludp_st_rule), type);
}

/**
 * nss_nludp_st_swap_addr_ipv6
 *	Converts IPv6 address to NSS format.
 *
 * @param[in] rule IPv6 address.
 *
 * @return
 * None.
 */
static inline void nss_nludp_st_swap_addr_ipv6(uint32_t *src, uint32_t *dst)
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

#endif /* __NSS_NLUDP_ST_IF_H */
