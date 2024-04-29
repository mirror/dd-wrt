/*
 **************************************************************************
 * Copyright (c) 2021 The Linux Foundation.  All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/version.h>
#include <linux/module.h>
#include <linux/in6.h>
#include <linux/inet.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <asm/cmpxchg.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_AE_CLASSIFIER_DEBUG_LEVEL

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
#include "ecm_ipv4.h"
#include "ecm_ipv6.h"

DEFINE_SPINLOCK(ecm_ae_classifier_lock);

/*
 * ecm_ae_classifier_dummy_get()
 *	Dummy acceleration engine get function.
 */
ecm_ae_classifier_result_t ecm_ae_classifier_dummy_get(struct ecm_ae_classifier_info *info)
{
	DEBUG_TRACE("%px: Dummy acceleration engine get is called\n", info);

	return ECM_AE_CLASSIFIER_RESULT_DONT_CARE;
}

/*
 * Acceleration engine operations object
 */
struct ecm_ae_classifier_ops ae_ops = {
	.ae_get = ecm_ae_classifier_dummy_get,
	.ae_flags = 0
};

/*
 * ecm_ae_classifier_select_info_fill()
 *	Fills the AE selction info object.
 */
void ecm_ae_classifier_select_info_fill(ip_addr_t src_ip, ip_addr_t dest_ip,
					int sport, int dport, int protocol, int ip_version,
					bool is_routed, bool is_multicast,
					struct ecm_ae_classifier_info *info)
{
	if (ip_version == 4) {
		ECM_IP_ADDR_TO_NIN4_ADDR(info->src.v4_addr, src_ip);
		ECM_IP_ADDR_TO_NIN4_ADDR(info->dest.v4_addr, dest_ip);
	} else {
		ECM_IP_ADDR_TO_NIN6_ADDR(info->src.v6_addr, src_ip);
		ECM_IP_ADDR_TO_NIN6_ADDR(info->dest.v6_addr, dest_ip);
	}

	info->src_port = sport;
	info->dst_port = dport;
	info->protocol = protocol;
	info->flag = 0;
	info->ip_ver = ip_version;

	if (is_routed) {
		info->flag |= ECM_AE_CLASSIFIER_FLOW_ROUTED;
	}

	if (is_multicast) {
		info->flag |= ECM_AE_CLASSIFIER_FLOW_MULTICAST;
	}
}

/*
 * ecm_ae_classifier_is_external()
 *	Check if the AE is external.
 */
bool ecm_ae_classifier_is_external(struct ecm_ae_classifier_ops *ops)
{
	return (ops->ae_flags & ECM_AE_CLASSIFIER_FLAG_EXTERNAL_AE_REGISTERED);
}

/*
 * ecm_ae_classifier_is_fallback_enabled
 *	Check if fallback to another AE is enabled.
 */
bool ecm_ae_classifier_is_fallback_enabled(struct ecm_ae_classifier_ops *ops)
{
	return (ops->ae_flags & ECM_AE_CLASSIFIER_FLAG_FALLBACK_ENABLE);
}

/*
 * ecm_ae_classifier_decelerate_v4_connection()
 *	Decelerates an IPv4 connection
 */
bool ecm_ae_classifier_decelerate_v4_connection(__be32 src_ip, int src_port,
						__be32 dest_ip, int dest_port, int protocol)
{
	return ecm_db_connection_decel_v4(src_ip, src_port, dest_ip, dest_port, protocol);
}
EXPORT_SYMBOL(ecm_ae_classifier_decelerate_v4_connection);

/*
 * ecm_ae_classifier_decelerate_v6_connection()
 *	Decelerates an IPv6 connection
 */
bool ecm_ae_classifier_decelerate_v6_connection(struct in6_addr src_ip, int src_port,
						struct in6_addr dest_ip, int dest_port, int protocol)
{
	return ecm_db_connection_decel_v6(&src_ip, src_port, &dest_ip, dest_port, protocol);
}
EXPORT_SYMBOL(ecm_ae_classifier_decelerate_v6_connection);

/*
 * ecm_ae_classifier_ops_register
 */
void ecm_ae_classifier_ops_register(struct ecm_ae_classifier_ops *ops)
{
	xchg(&ae_ops.ae_get, ops->ae_get);
	xchg(&ae_ops.ae_flags, ops->ae_flags);
}
EXPORT_SYMBOL(ecm_ae_classifier_ops_register);

/*
 * ecm_ae_classifier_ops_unregister
 */
void ecm_ae_classifier_ops_unregister(void)
{
	xchg(&ae_ops.ae_get, ecm_ae_classifier_dummy_get);
	xchg(&ae_ops.ae_flags, 0);
	synchronize_net();
}
EXPORT_SYMBOL(ecm_ae_classifier_ops_unregister);
