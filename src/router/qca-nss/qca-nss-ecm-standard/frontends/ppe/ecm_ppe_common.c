/*
 **************************************************************************
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 **************************************************************************
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <net/ipv6.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/inet.h>
#include <linux/etherdevice.h>

#define DEBUG_LEVEL ECM_PPE_COMMON_DEBUG_LEVEL

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
#include "ecm_ppe_common.h"
#include "ecm_front_end_common.h"
#include "ecm_ppe_ipv6.h"
#include "ecm_ppe_ipv4.h"

#ifdef ECM_IPV6_ENABLE
/*
 * ecm_ppe_ipv6_is_conn_limit_reached()
 *	Connection limit is reached or not ?
 */
bool ecm_ppe_ipv6_is_conn_limit_reached(void)
{
#if !defined(ECM_FRONT_END_CONN_LIMIT_ENABLE)
	return false;
#endif

	if (likely(!((ecm_front_end_is_feature_supported(ECM_FE_FEATURE_CONN_LIMIT)) && ecm_front_end_conn_limit))) {
		return false;
	}

	spin_lock_bh(&ecm_ppe_ipv6_lock);
	if (ecm_ppe_ipv6_accelerated_count == PPE_DRV_V6_MAX_CONN_COUNT) {
		spin_unlock_bh(&ecm_ppe_ipv6_lock);
		DEBUG_INFO("ECM DB connection limit %d reached, for PPE frontend \
			   new flows cannot be accelerated.\n",
			   ecm_ppe_ipv6_accelerated_count);
		return true;
	}
	spin_unlock_bh(&ecm_ppe_ipv6_lock);
	return false;
}
#endif

/*
 * ecm_ppe_ipv4_is_conn_limit_reached()
 *	Connection limit is reached or not ?
 */
bool ecm_ppe_ipv4_is_conn_limit_reached(void)
{
#if !defined(ECM_FRONT_END_CONN_LIMIT_ENABLE)
	return false;
#endif

	if (likely(!((ecm_front_end_is_feature_supported(ECM_FE_FEATURE_CONN_LIMIT)) && ecm_front_end_conn_limit))) {
		return false;
	}

	spin_lock_bh(&ecm_ppe_ipv4_lock);
	if (ecm_ppe_ipv4_accelerated_count == PPE_DRV_V4_MAX_CONN_COUNT) {
		spin_unlock_bh(&ecm_ppe_ipv4_lock);
		DEBUG_INFO("ECM DB connection limit %d reached, for PPE frontend \
			   new flows cannot be accelerated.\n",
			   ecm_ppe_ipv4_accelerated_count);
		return true;
	}
	spin_unlock_bh(&ecm_ppe_ipv4_lock);
	return false;
}
