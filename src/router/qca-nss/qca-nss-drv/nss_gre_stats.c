/*
 **************************************************************************
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
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
 * nss_gre_stats.c
 *	NSS GRE statistics APIs
 *
 */

#include "nss_tx_rx_common.h"
#include "nss_stats.h"
#include "nss_gre_stats.h"

/*
 * Data structures to store GRE nss debug stats
 */
static DEFINE_SPINLOCK(nss_gre_stats_lock);
static struct nss_gre_stats_session_debug session_debug_stats[NSS_GRE_MAX_DEBUG_SESSION_STATS];
static struct nss_gre_stats_base_debug base_debug_stats;

/*
 * nss_gre_stats_base_debug_str
 *	GRE debug statistics strings for base types
 */
static int8_t *nss_gre_stats_base_debug_str[NSS_GRE_STATS_BASE_DEBUG_MAX] = {
	"GRE_BASE_RX_PACKETS",
	"GRE_BASE_RX_DROPPED",
	"GRE_BASE_EXP_ETH_HDR_MISSING",
	"GRE_BASE_EXP_ETH_TYPE_NON_IP",
	"GRE_BASE_EXP_IP_UNKNOWN_PROTOCOL",
	"GRE_BASE_EXP_IP_HEADER_INCOMPLETE",
	"GRE_BASE_EXP_IP_BAD_TOTAL_LENGTH",
	"GRE_BASE_EXP_IP_BAD_CHECKSUM",
	"GRE_BASE_EXP_IP_DATAGRAM_INCOMPLETE",
	"GRE_BASE_EXP_IP_FRAGMENT",
	"GRE_BASE_EXP_IP_OPTIONS_INCOMPLETE",
	"GRE_BASE_EXP_IP_WITH_OPTIONS",
	"GRE_BASE_EXP_IPV6_UNKNOWN_PROTOCOL",
	"GRE_BASE_EXP_IPV6_HEADER_INCOMPLETE",
	"GRE_BASE_EXP_GRE_UNKNOWN_SESSION",
	"GRE_BASE_EXP_GRE_NODE_INACTIVE",
};

/*
 * nss_gre_stats_session_debug_str
 *	GRE debug statistics strings for sessions
 */
static int8_t *nss_gre_stats_session_debug_str[NSS_GRE_STATS_SESSION_DEBUG_MAX] = {
	"GRE_SESSION_PBUF_ALLOC_FAIL",
	"GRE_SESSION_DECAP_FORWARD_ENQUEUE_FAIL",
	"GRE_SESSION_ENCAP_FORWARD_ENQUEUE_FAIL",
	"GRE_SESSION_DECAP_TX_FORWARDED",
	"GRE_SESSION_ENCAP_RX_RECEIVED",
	"GRE_SESSION_ENCAP_RX_DROPPED",
	"GRE_SESSION_ENCAP_RX_LINEAR_FAIL",
	"GRE_SESSION_EXP_RX_KEY_ERROR",
	"GRE_SESSION_EXP_RX_SEQ_ERROR",
	"GRE_SESSION_EXP_RX_CS_ERROR",
	"GRE_SESSION_EXP_RX_FLAG_MISMATCH",
	"GRE_SESSION_EXP_RX_MALFORMED",
	"GRE_SESSION_EXP_RX_INVALID_PROTOCOL",
	"GRE_SESSION_EXP_RX_NO_HEADROOM",
};

/*
 * GRE statistics APIs
 */

/*
 * nss_gre_stats_session_register()
 *	Register debug statistic for GRE session.
 */
void nss_gre_stats_session_register(uint32_t if_num, struct net_device *netdev)
{
	int i;

	spin_lock_bh(&nss_gre_stats_lock);
	for (i = 0; i < NSS_GRE_MAX_DEBUG_SESSION_STATS; i++) {
		if (!session_debug_stats[i].valid) {
			session_debug_stats[i].valid = true;
			session_debug_stats[i].if_num = if_num;
			session_debug_stats[i].if_index = netdev->ifindex;
			break;
		}
	}
	spin_unlock_bh(&nss_gre_stats_lock);
}

/*
 * nss_gre_stats_session_unregister()
 *	Unregister debug statistic for GRE session.
 */
void nss_gre_stats_session_unregister(uint32_t if_num)
{
	int i;

	spin_lock_bh(&nss_gre_stats_lock);
	for (i = 0; i < NSS_GRE_MAX_DEBUG_SESSION_STATS; i++) {
		if (session_debug_stats[i].if_num == if_num) {
			memset(&session_debug_stats[i], 0, sizeof(struct nss_gre_stats_session_debug));
			break;
		}
	}
	spin_unlock_bh(&nss_gre_stats_lock);
}

/*
 * nss_gre_stats_session_debug_sync()
 *	debug statistics sync for GRE session.
 */
void nss_gre_stats_session_debug_sync(struct nss_ctx_instance *nss_ctx, struct nss_gre_session_stats_msg *sstats, uint16_t if_num)
{
	int i, j;
	enum nss_dynamic_interface_type interface_type = nss_dynamic_interface_get_type(nss_ctx, if_num);

	spin_lock_bh(&nss_gre_stats_lock);
	for (i = 0; i < NSS_GRE_MAX_DEBUG_SESSION_STATS; i++) {
		if (session_debug_stats[i].if_num == if_num) {
			for (j = 0; j < NSS_GRE_STATS_SESSION_DEBUG_MAX; j++) {
				session_debug_stats[i].stats[j] += sstats->stats[j];
			}

			if (interface_type == NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER) {
				session_debug_stats[i].stats[NSS_GRE_STATS_SESSION_ENCAP_RX_RECEIVED] += sstats->node_stats.rx_packets;
			} else if (interface_type == NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER) {
				session_debug_stats[i].stats[NSS_GRE_STATS_SESSION_DECAP_TX_FORWARDED] += sstats->node_stats.tx_packets;
			}
			break;
		}
	}
	spin_unlock_bh(&nss_gre_stats_lock);
}

/*
 * nss_gre_stats_base_debug_sync()
 *	Debug statistics sync for GRE base node.
 */
void nss_gre_stats_base_debug_sync(struct nss_ctx_instance *nss_ctx, struct nss_gre_base_stats_msg *bstats)
{
	int i;

	spin_lock_bh(&nss_gre_stats_lock);
	for (i = 0; i < NSS_GRE_STATS_BASE_DEBUG_MAX; i++) {
		base_debug_stats.stats[i] += bstats->stats[i];
	}
	spin_unlock_bh(&nss_gre_stats_lock);
}

/*
 * nss_gre_stats_session_debug_get()
 *	Get GRE session debug statistics.
 */
static void nss_gre_stats_session_debug_get(void *stats_mem, int size)
{
	struct nss_gre_stats_session_debug *stats = (struct nss_gre_stats_session_debug *)stats_mem;
	int i;

	if (!stats || (size < (sizeof(struct nss_gre_stats_session_debug) * NSS_GRE_MAX_DEBUG_SESSION_STATS)))  {
		nss_warning("No memory to copy gre stats");
		return;
	}

	spin_lock_bh(&nss_gre_stats_lock);
	for (i = 0; i < NSS_GRE_MAX_DEBUG_SESSION_STATS; i++) {
		if (session_debug_stats[i].valid) {
			memcpy(stats, &session_debug_stats[i], sizeof(struct nss_gre_stats_session_debug));
			stats++;
		}
	}
	spin_unlock_bh(&nss_gre_stats_lock);
}

/*
 * nss_gre_stats_base_debug_get()
 *	Get GRE debug base statistics.
 */
static void nss_gre_stats_base_debug_get(void *stats_mem, int size)
{
	struct nss_gre_stats_base_debug *stats = (struct nss_gre_stats_base_debug *)stats_mem;

	if (!stats) {
		nss_warning("No memory to copy GRE base stats\n");
		return;
	}

	if (size < sizeof(struct nss_gre_stats_base_debug)) {
		nss_warning("Not enough memory to copy GRE base stats\n");
		return;
	}

	spin_lock_bh(&nss_gre_stats_lock);
	memcpy(stats, &base_debug_stats, sizeof(struct nss_gre_stats_base_debug));
	spin_unlock_bh(&nss_gre_stats_lock);
}

/*
 * nss_gre_stats_read()
 *	Read GRE statistics
 */
static ssize_t nss_gre_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	uint32_t max_output_lines = 2 /* header & footer for base debug stats */
		+ 2 /* header & footer for session debug stats */
		+ NSS_GRE_STATS_BASE_DEBUG_MAX  /* Base debug */
		+ NSS_GRE_MAX_DEBUG_SESSION_STATS * (NSS_GRE_STATS_SESSION_DEBUG_MAX + 2) /*session stats */
		+ 2;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	struct net_device *dev;
	struct nss_gre_stats_session_debug *sstats;
	struct nss_gre_stats_base_debug *bstats;
	int id, i;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(!lbuf)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	bstats = kzalloc(sizeof(struct nss_gre_stats_base_debug), GFP_KERNEL);
	if (unlikely(!bstats)) {
		nss_warning("Could not allocate memory for base debug statistics buffer");
		kfree(lbuf);
		return 0;
	}

	sstats = kzalloc(sizeof(struct nss_gre_stats_session_debug) * NSS_GRE_MAX_DEBUG_SESSION_STATS, GFP_KERNEL);
	if (unlikely(!sstats)) {
		nss_warning("Could not allocate memory for base debug statistics buffer");
		kfree(lbuf);
		kfree(bstats);
		return 0;
	}

	/*
	 * Get all base stats
	 */
	nss_gre_stats_base_debug_get((void *)bstats, sizeof(struct nss_gre_stats_base_debug));
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\ngre Base stats start:\n\n");
	for (i = 0; i < NSS_GRE_STATS_BASE_DEBUG_MAX; i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
				     "\t%s = %llu\n", nss_gre_stats_base_debug_str[i],
				     bstats->stats[i]);
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\ngre Base stats End\n\n");

	/*
	 * Get all session stats
	 */
	nss_gre_stats_session_debug_get(sstats, sizeof(struct nss_gre_stats_session_debug) * NSS_GRE_MAX_DEBUG_SESSION_STATS);
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\ngre Session stats start:\n\n");

	for (id = 0; id < NSS_GRE_MAX_DEBUG_SESSION_STATS; id++) {

		if (!((sstats + id)->valid)) {
			continue;
		}

		dev = dev_get_by_index(&init_net, (sstats + id)->if_index);
		if (likely(dev)) {

			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "%d. nss interface id=%d, netdevice=%s\n", id,
					     (sstats + id)->if_num, dev->name);
			dev_put(dev);
		} else {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "%d. nss interface id=%d\n", id,
					     (sstats + id)->if_num);
		}

		for (i = 0; i < NSS_GRE_STATS_SESSION_DEBUG_MAX; i++) {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					     "\t%s = %llu\n", nss_gre_stats_session_debug_str[i],
					     (sstats + id)->stats[i]);
		}
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\ngre Session stats end\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, size_wr);

	kfree(sstats);
	kfree(bstats);
	kfree(lbuf);
	return bytes_read;
}

/*
 * nss_gre_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(gre)

/*
 * nss_gre_stats_dentry_create()
 *	Create gre statistics debug entry.
 */
void nss_gre_stats_dentry_create(void)
{
	nss_stats_create_dentry("gre", &nss_gre_stats_ops);
}

