/*
 **************************************************************************
 * Copyright (c) 2015,2018-2019 The Linux Foundation. All rights reserved.
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

#ifndef __NSS_NLIPSEC_IF_H
#define __NSS_NLIPSEC_IF_H

/**
 * @addtogroup nss_netlink
 * @{
 */

/*
 * @file nss_nlipsec_if.h
 *	NSS Netlink IPsec headers */

#define NSS_NLIPSEC_FAMILY "nss_nlipsec"	/**< IPsec family */
#define NSS_NLIPSEC_MAX_TUNNELS 16		/**< Max tunnels */
#define NSS_NLIPSEC_MCAST_GRP "nss_nlipsec_mc"
/**
 * @brief ipsec commands types
 */
enum nss_nlipsec_cmd {
	NSS_NLIPSEC_CMD_UNSPEC,			/**< Unspecified cmd. */
	NSS_NLIPSEC_CMD_CREATE_TUNNEL,		/**< Create tunnel. */
	NSS_NLIPSEC_CMD_DESTROY_TUNNEL,		/**< Destroy tunnel. */
	NSS_NLIPSEC_CMD_ADD_SA,			/**< Add Security AssociationA. */
	NSS_NLIPSEC_CMD_DEL_SA,			/**< Delete Security Association. */
	NSS_NLIPSEC_CMD_ADD_FLOW,		/**< Add flow. */
	NSS_NLIPSEC_CMD_DEL_FLOW,		/**< Delete flow. */
	NSS_NLIPSEC_CMD_MAX
};

/**
 * @brief IPsec message
 */
struct nss_nlipsec_rule {
	struct nss_nlcmn cm;			/**< Common message header. */
	uint8_t ifname[IFNAMSIZ];		/**< IPSec interface name. */
	struct nss_ipsecmgr_flow_outer outer;	/**< Outer flow data. */

	union {
		struct nss_ipsecmgr_sa sa;		/**< Security Association data. */
		struct nss_ipsecmgr_flow_inner flow;	/**< Inner flow.  */
		struct nss_ipsecmgr_event event;	/**< IPsec event. */
	} data;
};

/**
 * @brief NETLINK IPsec message init
 *
 * @param rule[IN] NSS NETLINK IPsec message
 * @param type[IN] IPsec message type
 */
static inline void nss_nlipsec_rule_init(struct nss_nlipsec_rule *rule, enum nss_nlipsec_cmd type)
{
	nss_nlcmn_set_ver(&rule->cm, NSS_NL_VER);
	nss_nlcmn_init_cmd(&rule->cm, sizeof(struct nss_nlipsec_rule), type);
}

/**@}*/
#endif /* __NSS_NLIPSEC_IF_H */
