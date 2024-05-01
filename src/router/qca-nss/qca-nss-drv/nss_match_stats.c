/*
 ***************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 ***************************************************************************
 */

/*
 * nss_match_stats.c
 */
#include "nss_core.h"
#include "nss_stats.h"
#include <nss_match.h>
#include "nss_match_stats.h"

#define NSS_MATCH_STATS_SIZE_PER_IF (NSS_STATS_MAX_STR_LENGTH * NSS_STATS_NODE_MAX)
                                        /* Total number of statistics per match interface. */

int match_ifnum[NSS_MATCH_INSTANCE_MAX] = {0};
static DEFINE_SPINLOCK(nss_match_stats_lock);

/*
 * nss_match_ifnum_add()
 */
bool nss_match_ifnum_add(int if_num)
{
	int index = 0;

	spin_lock(&nss_match_stats_lock);

	for (index = 0; index < NSS_MATCH_INSTANCE_MAX; index++) {
		if (match_ifnum[index]) {
			continue;
		}

		match_ifnum[index] = if_num;

		spin_unlock(&nss_match_stats_lock);
		return true;
	}

	spin_unlock(&nss_match_stats_lock);
	return false;
}

/*
 * nss_match_ifnum_delete()
 */
bool nss_match_ifnum_delete(int if_num)
{
	int index = 0;

	spin_lock(&nss_match_stats_lock);

	for (index = 0; index < NSS_MATCH_INSTANCE_MAX; index++) {
		if (match_ifnum[index] != if_num) {
			continue;
		}

		match_ifnum[index] = 0;

		spin_unlock(&nss_match_stats_lock);
		return true;
	}

	spin_unlock(&nss_match_stats_lock);
	return false;
}


/*
 * nss_match_stats_read()
 *	Read match node statiistics.
 */
static ssize_t nss_match_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	ssize_t bytes_read = 0;
	uint32_t index, if_num;
	char *lbuf;
        size_t size_al = NSS_MATCH_STATS_SIZE_PER_IF * NSS_MATCH_INSTANCE_MAX;
	size_t size_wr = 0;

	lbuf = kzalloc(size_al, GFP_KERNEL);
	if (!lbuf) {
		nss_warning("Could not allocate memory for local statistics buffer\n");
		return 0;
	}

	size_wr += nss_stats_banner(lbuf, size_wr, size_al, "match", NSS_STATS_SINGLE_CORE);

	/*
	 * Common node stats for each match dynamic interface.
	 */
	for (index = 0; index < NSS_MATCH_INSTANCE_MAX; index++) {

		spin_lock_bh(&nss_match_stats_lock);
		if_num = match_ifnum[index];
		spin_unlock_bh(&nss_match_stats_lock);

		if (if_num) {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nMatch node if_num:%03u", if_num);
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n ---------------------- \n");
			size_wr += nss_stats_fill_common_stats(if_num, NSS_STATS_SINGLE_INSTANCE, lbuf, size_wr, size_al, "match");
			continue;
		}
	}

	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	return bytes_read;
}


/*
 * nss_match_stats_sync()
 *	Update match common node statistics.
 */
void nss_match_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_match_msg *nmm)
{
	struct nss_top_instance *nss_top = nss_ctx->nss_top;
	struct nss_match_stats_sync *msg_stats = &nmm->msg.stats;
	uint64_t *if_stats;
	int index;

	spin_lock_bh(&nss_top->stats_lock);

	/*
	 * Update common node stats
	 */
	if_stats = nss_top->stats_node[nmm->cm.interface];
	if_stats[NSS_STATS_NODE_RX_PKTS] += msg_stats->p_stats.rx_packets;
	if_stats[NSS_STATS_NODE_RX_BYTES] += msg_stats->p_stats.rx_bytes;
	if_stats[NSS_STATS_NODE_TX_PKTS] += msg_stats->p_stats.tx_packets;
	if_stats[NSS_STATS_NODE_TX_BYTES] += msg_stats->p_stats.tx_bytes;

	for (index = 0; index < NSS_MAX_NUM_PRI; index++) {
		if_stats[NSS_STATS_NODE_RX_QUEUE_0_DROPPED + index] += msg_stats->p_stats.rx_dropped[index];
	}

	spin_unlock_bh(&nss_top->stats_lock);
}

/*
 * nss_match_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(match)

/*
 * nss_match_stats_dentry_create()
 *	Create match statistics debug entry.
 */
void nss_match_stats_dentry_create(void)
{
	nss_stats_create_dentry("match", &nss_match_stats_ops);
}
