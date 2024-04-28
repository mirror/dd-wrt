/*
 **************************************************************************
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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

#ifndef __NSS_IPV6_STATS_H
#define __NSS_IPV6_STATS_H

/*
 * IPV6 node statistics
 */
enum nss_ipv6_stats_types {
	NSS_IPV6_STATS_ACCELERATED_RX_PKTS = 0,
					/* Accelerated IPv6 RX packets */
	NSS_IPV6_STATS_ACCELERATED_RX_BYTES,
					/* Accelerated IPv6 RX bytes */
	NSS_IPV6_STATS_ACCELERATED_TX_PKTS,
					/* Accelerated IPv6 TX packets */
	NSS_IPV6_STATS_ACCELERATED_TX_BYTES,
					/* Accelerated IPv6 TX bytes */
	NSS_IPV6_STATS_CONNECTION_CREATE_REQUESTS,
					/* Number of IPv6 connection create requests */
	NSS_IPV6_STATS_CONNECTION_CREATE_COLLISIONS,
					/* Number of IPv6 connection create requests that collided with existing entries */
	NSS_IPV6_STATS_CONNECTION_CREATE_INVALID_INTERFACE,
					/* Number of IPv6 connection create requests that had invalid interface */
	NSS_IPV6_STATS_CONNECTION_DESTROY_REQUESTS,
					/* Number of IPv6 connection destroy requests */
	NSS_IPV6_STATS_CONNECTION_DESTROY_MISSES,
					/* Number of IPv6 connection destroy requests that missed the cache */
	NSS_IPV6_STATS_CONNECTION_HASH_HITS,
					/* Number of IPv6 connection hash hits */
	NSS_IPV6_STATS_CONNECTION_HASH_REORDERS,
					/* Number of IPv6 connection hash reorders */
	NSS_IPV6_STATS_CONNECTION_FLUSHES,
					/* Number of IPv6 connection flushes */
	NSS_IPV6_STATS_CONNECTION_EVICTIONS,
					/* Number of IPv6 connection evictions */
	NSS_IPV6_STATS_FRAGMENTATIONS,
					/* Number of successful IPv6 fragmentations performed */
	NSS_IPV6_STATS_FRAG_FAILS,
					/* Number of IPv6 fragmentation fails */
	NSS_IPV6_STATS_DROPPED_BY_RULE,
					/* Number of IPv6 packets dropped by a drop rule. */
	NSS_IPV6_STATS_MC_CONNECTION_CREATE_REQUESTS,
					/* Number of successful IPv6 Multicast create requests */
	NSS_IPV6_STATS_MC_CONNECTION_UPDATE_REQUESTS,
					/* Number of successful IPv6 Multicast update requests */
	NSS_IPV6_STATS_MC_CONNECTION_CREATE_INVALID_INTERFACE,
					/* Number of IPv6 Multicast connection create requests that had invalid interface */
	NSS_IPV6_STATS_MC_CONNECTION_DESTROY_REQUESTS,
					/* Number of IPv6 Multicast connection destroy requests */
	NSS_IPV6_STATS_MC_CONNECTION_DESTROY_MISSES,
					/* Number of IPv6 Multicast connection destroy requests that missed the cache */
	NSS_IPV6_STATS_MC_CONNECTION_FLUSHES,
					/* Number of IPv6 Multicast connection flushes */
	NSS_IPV6_STATS_MAX,
};

/*
 * IPV6 statistics APIs
 */
extern void nss_ipv6_stats_node_sync(struct nss_ctx_instance *nss_ctx, struct nss_ipv6_node_sync *nins);
extern void nss_ipv6_stats_conn_sync(struct nss_ctx_instance *nss_ctx, struct nss_ipv6_conn_sync *nics);
extern void nss_ipv6_stats_conn_sync_many(struct nss_ctx_instance *nss_ctx, struct nss_ipv6_conn_sync_many_msg *nicsm);
extern void nss_ipv6_stats_dentry_create(void);

#endif /* __NSS_IPV6_STATS_H */
