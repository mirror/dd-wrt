/*
 **************************************************************************
 * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
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

#include "nss_stats.h"
#include "nss_core.h"
#include <nss_ipv4.h>
#include "nss_ipv4_stats.h"

/*
 * nss_ipv4_exception_stats_str
 *	Interface stats strings for ipv4 exceptions
 */
static int8_t *nss_ipv4_exception_stats_str[NSS_IPV4_EXCEPTION_EVENT_MAX] = {
	"IPV4_ICMP_HEADER_INCOMPLETE",
	"IPV4_ICMP_UNHANDLED_TYPE",
	"IPV4_ICMP_IPV4_HEADER_INCOMPLETE",
	"IPV4_ICMP_IPV4_UDP_HEADER_INCOMPLETE",
	"IPV4_ICMP_IPV4_TCP_HEADER_INCOMPLETE",
	"IPV4_ICMP_SIPV4_UNKNOWN_PROTOCOL",
	"IPV4_ICMP_NO_ICME",
	"IPV4_ICMP_FLUSH_TO_HOST",
	"IPV4_TCP_HEADER_INCOMPLETE",
	"IPV4_TCP_NO_ICME",
	"IPV4_TCP_IP_OPTION",
	"IPV4_TCP_IP_FRAGMENT",
	"IPV4_TCP_SMALL_TTL",
	"IPV4_TCP_NEEDS_FRAGMENTATION",
	"IPV4_TCP_FLAGS",
	"IPV4_TCP_SEQ_EXCEEDS_RIGHT_EDGE",
	"IPV4_TCP_SMALL_DATA_OFFS",
	"IPV4_TCP_BAD_SACK",
	"IPV4_TCP_BIG_DATA_OFFS",
	"IPV4_TCP_SEQ_BEFORE_LEFT_EDGE",
	"IPV4_TCP_ACK_EXCEEDS_RIGHT_EDGE",
	"IPV4_TCP_ACK_BEFORE_LEFT_EDGE",
	"IPV4_UDP_HEADER_INCOMPLETE",
	"IPV4_UDP_NO_ICME",
	"IPV4_UDP_IP_OPTION",
	"IPV4_UDP_IP_FRAGMENT",
	"IPV4_UDP_SMALL_TTL",
	"IPV4_UDP_NEEDS_FRAGMENTATION",
	"IPV4_WRONG_TARGET_MAC",
	"IPV4_HEADER_INCOMPLETE",
	"IPV4_BAD_TOTAL_LENGTH",
	"IPV4_BAD_CHECKSUM",
	"IPV4_NON_INITIAL_FRAGMENT",
	"IPV4_DATAGRAM_INCOMPLETE",
	"IPV4_OPTIONS_INCOMPLETE",
	"IPV4_UNKNOWN_PROTOCOL",
	"IPV4_ESP_HEADER_INCOMPLETE",
	"IPV4_ESP_NO_ICME",
	"IPV4_ESP_IP_OPTION",
	"IPV4_ESP_IP_FRAGMENT",
	"IPV4_ESP_SMALL_TTL",
	"IPV4_ESP_NEEDS_FRAGMENTATION",
	"IPV4_INGRESS_VID_MISMATCH",
	"IPV4_INGRESS_VID_MISSING",
	"IPV4_6RD_NO_ICME",
	"IPV4_6RD_IP_OPTION",
	"IPV4_6RD_IP_FRAGMENT",
	"IPV4_6RD_NEEDS_FRAGMENTATION",
	"IPV4_DSCP_MARKING_MISMATCH",
	"IPV4_VLAN_MARKING_MISMATCH",
	"IPV4_INTERFACE_MISMATCH",
	"IPV4_GRE_HEADER_INCOMPLETE",
	"IPV4_GRE_NO_ICME",
	"IPV4_GRE_IP_OPTION",
	"IPV4_GRE_IP_FRAGMENT",
	"IPV4_GRE_SMALL_TTL",
	"IPV4_GRE_NEEDS_FRAGMENTATION",
	"IPV4_PPTP_GRE_SESSION_MATCH_FAIL",
	"IPV4_PPTP_GRE_INVALID_PROTO",
	"IPV4_PPTP_GRE_NO_CME",
	"IPV4_PPTP_GRE_IP_OPTION",
	"IPV4_PPTP_GRE_IP_FRAGMENT",
	"IPV4_PPTP_GRE_SMALL_TTL",
	"IPV4_PPTP_GRE_NEEDS_FRAGMENTATION",
	"IPV4_DESTROY",
	"IPV4_FRAG_DF_SET",
	"IPV4_FRAG_FAIL",
	"IPV4_ICMP_IPV4_UDPLITE_HEADER_INCOMPLETE",
	"IPV4_UDPLITE_HEADER_INCOMPLETE",
	"IPV4_UDPLITE_NO_ICME",
	"IPV4_UDPLITE_IP_OPTION",
	"IPV4_UDPLITE_IP_FRAGMENT",
	"IPV4_UDPLITE_SMALL_TTL",
	"IPV4_UDPLITE_NEEDS_FRAGMENTATION",
	"IPV4_MC_UDP_NO_ICME",
	"IPV4_MC_MEM_ALLOC_FAILURE",
	"IPV4_MC_UPDATE_FAILURE",
	"IPV4_MC_PBUF_ALLOC_FAILURE"
};

uint64_t nss_ipv4_stats[NSS_IPV4_STATS_MAX];
uint64_t nss_ipv4_exception_stats[NSS_IPV4_EXCEPTION_EVENT_MAX];

/*
 * nss_ipv4_stats_str
 *	IPv4 stats strings
 */
static int8_t *nss_ipv4_stats_str[NSS_IPV4_STATS_MAX] = {
	"rx_pkts",
	"rx_bytes",
	"tx_pkts",
	"tx_bytes",
	"create_requests",
	"create_collisions",
	"create_invalid_interface",
	"destroy_requests",
	"destroy_misses",
	"hash_hits",
	"hash_reorders",
	"flushes",
	"evictions",
	"fragmentations",
	"by_rule_drops",
	"mc_create_requests",
	"mc_update_requests",
	"mc_create_invalid_interface",
	"mc_destroy_requests",
	"mc_destroy_misses",
	"mc_flushes",
};

/*
 * nss_ipv4_stats_read()
 *	Read IPV4 stats
 */
static ssize_t nss_ipv4_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i;
	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = (NSS_STATS_NODE_MAX + 2) + (NSS_IPV4_STATS_MAX + 3) + (NSS_IPV4_EXCEPTION_EVENT_MAX + 3) + 5;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	/*
	 * Note: The assumption here is that exception event count is larger than other statistics count for IPv4
	 */
	stats_shadow = kzalloc(NSS_IPV4_EXCEPTION_EVENT_MAX * 8, GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr = scnprintf(lbuf, size_al, "ipv4 stats start:\n\n");

	size_wr = nss_stats_fill_common_stats(NSS_IPV4_RX_INTERFACE, lbuf, size_wr, size_al);

	/*
	 * IPv4 node stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nipv4 node stats:\n\n");

	spin_lock_bh(&nss_top_main.stats_lock);
	for (i = 0; (i < NSS_IPV4_STATS_MAX); i++) {
		stats_shadow[i] = nss_ipv4_stats[i];
	}

	spin_unlock_bh(&nss_top_main.stats_lock);

	for (i = 0; (i < NSS_IPV4_STATS_MAX); i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"%s = %llu\n", nss_ipv4_stats_str[i], stats_shadow[i]);
	}

	/*
	 * Exception stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nipv4 exception stats:\n\n");

	spin_lock_bh(&nss_top_main.stats_lock);
	for (i = 0; (i < NSS_IPV4_EXCEPTION_EVENT_MAX); i++) {
		stats_shadow[i] = nss_ipv4_exception_stats[i];
	}

	spin_unlock_bh(&nss_top_main.stats_lock);

	for (i = 0; (i < NSS_IPV4_EXCEPTION_EVENT_MAX); i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"%s = %llu\n", nss_ipv4_exception_stats_str[i], stats_shadow[i]);
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nipv4 stats end\n\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_ipv4_stats_conn_sync()
 *	Update driver specific information from the messsage.
 */
void nss_ipv4_stats_conn_sync(struct nss_ctx_instance *nss_ctx, struct nss_ipv4_conn_sync *nirs)
{
	struct nss_top_instance *nss_top = nss_ctx->nss_top;

	/*
	 * Update statistics maintained by NSS driver
	 */
	spin_lock_bh(&nss_top->stats_lock);
	nss_ipv4_stats[NSS_IPV4_STATS_ACCELERATED_RX_PKTS] += nirs->flow_rx_packet_count + nirs->return_rx_packet_count;
	nss_ipv4_stats[NSS_IPV4_STATS_ACCELERATED_RX_BYTES] += nirs->flow_rx_byte_count + nirs->return_rx_byte_count;
	nss_ipv4_stats[NSS_IPV4_STATS_ACCELERATED_TX_PKTS] += nirs->flow_tx_packet_count + nirs->return_tx_packet_count;
	nss_ipv4_stats[NSS_IPV4_STATS_ACCELERATED_TX_BYTES] += nirs->flow_tx_byte_count + nirs->return_tx_byte_count;
	spin_unlock_bh(&nss_top->stats_lock);
}

/*
 * nss_ipv4_stats_conn_sync_many()
 *	Update driver specific information from the conn_sync_many messsage.
 */
void nss_ipv4_stats_conn_sync_many(struct nss_ctx_instance *nss_ctx, struct nss_ipv4_conn_sync_many_msg *nicsm)
{
	int i;

	/*
	 * Sanity check for the stats count
	 */
	if (nicsm->count * sizeof(struct nss_ipv4_conn_sync) >= nicsm->size) {
		nss_warning("%p: stats sync count %u exceeds the size of this msg %u", nss_ctx, nicsm->count, nicsm->size);
		return;
	}

	for (i = 0; i < nicsm->count; i++) {
		nss_ipv4_stats_conn_sync(nss_ctx, &nicsm->conn_sync[i]);
	}
}

/*
 * nss_ipv4_stats_node_sync()
 *	Update driver specific information from the messsage.
 */
void nss_ipv4_stats_node_sync(struct nss_ctx_instance *nss_ctx, struct nss_ipv4_node_sync *nins)
{
	struct nss_top_instance *nss_top = nss_ctx->nss_top;
	uint32_t i;

	/*
	 * Update statistics maintained by NSS driver
	 */
	spin_lock_bh(&nss_top->stats_lock);
	nss_top->stats_node[NSS_IPV4_RX_INTERFACE][NSS_STATS_NODE_RX_PKTS] += nins->node_stats.rx_packets;
	nss_top->stats_node[NSS_IPV4_RX_INTERFACE][NSS_STATS_NODE_RX_BYTES] += nins->node_stats.rx_bytes;
	nss_top->stats_node[NSS_IPV4_RX_INTERFACE][NSS_STATS_NODE_TX_PKTS] += nins->node_stats.tx_packets;
	nss_top->stats_node[NSS_IPV4_RX_INTERFACE][NSS_STATS_NODE_TX_BYTES] += nins->node_stats.tx_bytes;

	for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
		nss_top->stats_node[NSS_IPV4_RX_INTERFACE][NSS_STATS_NODE_RX_QUEUE_0_DROPPED + i] += nins->node_stats.rx_dropped[i];
	}

	nss_ipv4_stats[NSS_IPV4_STATS_CONNECTION_CREATE_REQUESTS] += nins->ipv4_connection_create_requests;
	nss_ipv4_stats[NSS_IPV4_STATS_CONNECTION_CREATE_COLLISIONS] += nins->ipv4_connection_create_collisions;
	nss_ipv4_stats[NSS_IPV4_STATS_CONNECTION_CREATE_INVALID_INTERFACE] += nins->ipv4_connection_create_invalid_interface;
	nss_ipv4_stats[NSS_IPV4_STATS_CONNECTION_DESTROY_REQUESTS] += nins->ipv4_connection_destroy_requests;
	nss_ipv4_stats[NSS_IPV4_STATS_CONNECTION_DESTROY_MISSES] += nins->ipv4_connection_destroy_misses;
	nss_ipv4_stats[NSS_IPV4_STATS_CONNECTION_HASH_HITS] += nins->ipv4_connection_hash_hits;
	nss_ipv4_stats[NSS_IPV4_STATS_CONNECTION_HASH_REORDERS] += nins->ipv4_connection_hash_reorders;
	nss_ipv4_stats[NSS_IPV4_STATS_CONNECTION_FLUSHES] += nins->ipv4_connection_flushes;
	nss_ipv4_stats[NSS_IPV4_STATS_CONNECTION_EVICTIONS] += nins->ipv4_connection_evictions;
	nss_ipv4_stats[NSS_IPV4_STATS_FRAGMENTATIONS] += nins->ipv4_fragmentations;
	nss_ipv4_stats[NSS_IPV4_STATS_MC_CONNECTION_CREATE_REQUESTS] += nins->ipv4_mc_connection_create_requests;
	nss_ipv4_stats[NSS_IPV4_STATS_MC_CONNECTION_UPDATE_REQUESTS] += nins->ipv4_mc_connection_update_requests;
	nss_ipv4_stats[NSS_IPV4_STATS_MC_CONNECTION_CREATE_INVALID_INTERFACE] += nins->ipv4_mc_connection_create_invalid_interface;
	nss_ipv4_stats[NSS_IPV4_STATS_MC_CONNECTION_DESTROY_REQUESTS] += nins->ipv4_mc_connection_destroy_requests;
	nss_ipv4_stats[NSS_IPV4_STATS_MC_CONNECTION_DESTROY_MISSES] += nins->ipv4_mc_connection_destroy_misses;
	nss_ipv4_stats[NSS_IPV4_STATS_MC_CONNECTION_FLUSHES] += nins->ipv4_mc_connection_flushes;

	for (i = 0; i < NSS_IPV4_EXCEPTION_EVENT_MAX; i++) {
		nss_ipv4_exception_stats[i] += nins->exception_events[i];
	}
	spin_unlock_bh(&nss_top->stats_lock);
}

/*
 * nss_ipv4_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(ipv4)

/*
 * nss_ipv4_stats_dentry_create()
 *	Create IPv4 statistics debug entry.
 */
void nss_ipv4_stats_dentry_create(void)
{
	nss_stats_create_dentry("ipv4", &nss_ipv4_stats_ops);
}
