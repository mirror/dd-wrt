/*
 **************************************************************************
 * Copyright (c) 2015, 2020, The Linux Foundation. All rights reserved.
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
 * nss_l2tpv2_stats.c
 *
 * This file is the NSS l2tpv2 stats handler functions
 */

#include <linux/types.h>
#if IS_ENABLED(CONFIG_NF_FLOW_TABLE)
#include <net/netfilter/nf_flow_table.h>
#endif
#include <linux/ppp_channel.h>
#include <nss_api_if.h>
#include <nss_dynamic_interface.h>
#include <linux/l2tp.h>
#include <net/sock.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <linux/version.h>
#include <linux/../../net/l2tp/l2tp_core.h>

#include "nss_connmgr_l2tpv2.h"

/*
 * NSS l2tpv2 debug macros
 */
#if (NSS_L2TP_DEBUG_LEVEL < 1)
#define nss_l2tpv2_stats_assert(fmt, args...)
#else
#define nss_l2tpv2_stats_assert(c)  BUG_ON(!(c));
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_l2tpv2_stats_warning(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_l2tpv2_stats_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_l2tpv2_stats_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels
 */
#if (NSS_L2TP_DEBUG_LEVEL < 2)
#define nss_l2tpv2_stats_warning(s, ...)
#else
#define nss_l2tpv2_stats_warning(s, ...) pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_L2TP_DEBUG_LEVEL < 3)
#define nss_l2tpv2_stats_info(s, ...)
#else
#define nss_l2tpv2_stats_info(s, ...)   pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_L2TP_DEBUG_LEVEL < 4)
#define nss_l2tpv2_stats_trace(s, ...)
#else
#define nss_l2tpv2_stats_trace(s, ...)  pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif

/*
 * nss_l2tpv2_update_dev_stats()
 *	Handle stats message for l2tpv2 data messages
 */
void nss_l2tpv2_update_dev_stats(struct net_device *dev, struct nss_l2tpv2_sync_session_stats_msg *sync_stats)
{
	struct l2tp_tunnel *tunnel;
	struct l2tp_session *session;
	struct nss_connmgr_l2tpv2_data data;
	struct l2tp_stats l2tp_stats;
	int err;

	if (!dev) {
		return;
	}

	dev_hold(dev);

	memset(&l2tp_stats, 0, sizeof(struct l2tp_stats));

	/*
	 * Get tunnel id
	 */
	err = nss_connmgr_l2tpv2_get_data(dev, &data);
	if (err) {
		dev_put(dev);
		return;
	}

	/*
	 * Update tunnel & session stats
	 */
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 12, 0))
	tunnel = l2tp_tunnel_find(dev_net(dev), data.l2tpv2.tunnel.tunnel_id);
	if (!tunnel) {
		dev_put(dev);
		return;
	}
	tunnel_hold(tunnel);

	session = l2tp_session_find(dev_net(dev), tunnel, data.l2tpv2.session.session_id);
	if (!session) {
		tunnel_put(tunnel);
		dev_put(dev);
		return;
	}
	session_hold(session);
#else
	tunnel = l2tp_tunnel_get(dev_net(dev), data.l2tpv2.tunnel.tunnel_id);
	if (!tunnel) {
		dev_put(dev);
		return;
	}
	session = l2tp_tunnel_get_session(tunnel, data.l2tpv2.session.session_id);
	if (!session) {
		tunnel_put(tunnel);
		dev_put(dev);
		return;
	}
#endif

	atomic_long_set(&l2tp_stats.tx_packets, (long)sync_stats->node_stats.tx_packets);
	atomic_long_set(&l2tp_stats.tx_bytes, (long)sync_stats->node_stats.tx_bytes);
	atomic_long_set(&l2tp_stats.tx_errors, (long)sync_stats->tx_errors);

	atomic_long_set(&l2tp_stats.rx_packets, (long)sync_stats->node_stats.rx_packets);
	atomic_long_set(&l2tp_stats.rx_bytes, (long)sync_stats->node_stats.rx_bytes);
	atomic_long_set(&l2tp_stats.rx_errors, (long)sync_stats->rx_errors);

	atomic_long_set(&l2tp_stats.rx_seq_discards, (long)sync_stats->rx_seq_discards);
	atomic_long_set(&l2tp_stats.rx_oos_packets, (long)(sync_stats->rx_oos_packets));

	l2tp_stats_update(tunnel, session, &l2tp_stats);

	session_put(session);
	tunnel_put(tunnel);

	/*
	 * Update ppp stats
	 */
	ppp_update_stats(dev,
			 (unsigned long)sync_stats->node_stats.rx_packets,
			 (unsigned long)sync_stats->node_stats.rx_bytes,
			 (unsigned long)sync_stats->node_stats.tx_packets,
			 (unsigned long)sync_stats->node_stats.tx_bytes,
			 (unsigned long)sync_stats->rx_errors,
			 (unsigned long)sync_stats->tx_errors,
			 (unsigned long)nss_cmn_rx_dropped_sum(&sync_stats->node_stats),
			 (unsigned long)sync_stats->tx_dropped);

	dev_put(dev);
}
