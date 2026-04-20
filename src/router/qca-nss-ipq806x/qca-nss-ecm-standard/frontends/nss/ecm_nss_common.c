/*
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/inet.h>
#include <linux/etherdevice.h>

#define DEBUG_LEVEL ECM_NSS_COMMON_DEBUG_LEVEL

#include <nss_api_if.h>

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_tracker_datagram.h"
#include "ecm_tracker_udp.h"
#include "ecm_tracker_tcp.h"
#include "ecm_db.h"
#include "ecm_interface.h"
#include "ecm_nss_common.h"
#include "ecm_front_end_common.h"
#include "ecm_nss_ipv6.h"
#include "ecm_nss_ipv4.h"

#ifdef ECM_IPV6_ENABLE
/*
 * ecm_nss_ipv6_is_conn_limit_reached()
 *	Connection limit is reached or not ?
 */
bool ecm_nss_ipv6_is_conn_limit_reached(void)
{

#if !defined(ECM_FRONT_END_CONN_LIMIT_ENABLE)
	return false;
#endif

	if (likely(!((ecm_front_end_is_feature_supported(ECM_FE_FEATURE_CONN_LIMIT)) && ecm_front_end_conn_limit))) {
		return false;
	}

	if (ecm_nss_ipv6_accelerated_count == nss_ipv6_max_conn_count()) {
		DEBUG_INFO("ECM DB connection limit %d reached, for NSS frontend \
			   new flows cannot be accelerated.\n",
			   ecm_nss_ipv6_accelerated_count);
		return true;
	}

	return false;
}
EXPORT_SYMBOL(ecm_nss_ipv6_is_conn_limit_reached);
#endif

/*
 * ecm_nss_feature_check()
 *	Check some specific features for NSS acceleration
 */
bool ecm_nss_feature_check(struct sk_buff *skb, struct ecm_tracker_ip_header *ip_hdr)
{
	/*
	 * If the DSCP value of the packet maps to the NOT accel action type,
	 * do not accelerate the packet and let it go through the
	 * slow path.
	 */
	if (ip_hdr->protocol == IPPROTO_UDP) {
		uint8_t action = ip_hdr->is_v4 ?
			nss_ipv4_dscp_action_get(ip_hdr->dscp) : nss_ipv6_dscp_action_get(ip_hdr->dscp);
		if (action == NSS_IPV4_DSCP_MAP_ACTION_DONT_ACCEL || action == NSS_IPV6_DSCP_MAP_ACTION_DONT_ACCEL) {
			DEBUG_TRACE("%px: dscp: %d maps to action not accel type, skip acceleration\n", skb, ip_hdr->dscp);
			return false;
		}
	}

	return ecm_front_end_feature_check(skb, ip_hdr);
}

/*
 * ecm_nss_ipv4_is_conn_limit_reached()
 *	Connection limit is reached or not ?
 */
bool ecm_nss_ipv4_is_conn_limit_reached(void)
{

#if !defined(ECM_FRONT_END_CONN_LIMIT_ENABLE)
	return false;
#endif

	if (likely(!((ecm_front_end_is_feature_supported(ECM_FE_FEATURE_CONN_LIMIT)) && ecm_front_end_conn_limit))) {
		return false;
	}

	if (ecm_nss_ipv4_accelerated_count == nss_ipv4_max_conn_count()) {
		DEBUG_INFO("ECM DB connection limit %d reached, for NSS frontend \
			   new flows cannot be accelerated.\n",
			   ecm_nss_ipv4_accelerated_count);
		return true;
	}

	return false;
}
EXPORT_SYMBOL(ecm_nss_ipv4_is_conn_limit_reached);
