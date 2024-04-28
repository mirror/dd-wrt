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

/*
 * nss_stats.h
 *	NSS driver stats header file.
 */

#ifndef __NSS_STATS_H
#define __NSS_STATS_H

#include <linux/debugfs.h>

/*
 * Maximum string length:
 * This should be equal to maximum string size of any stats
 * inclusive of stats value
 */
#define NSS_STATS_MAX_STR_LENGTH 96

/*
 * Node statistics
 */
enum nss_stats_node {
	NSS_STATS_NODE_RX_PKTS,		/* Accelerated node RX packets */
	NSS_STATS_NODE_RX_BYTES,	/* Accelerated node RX bytes */
	NSS_STATS_NODE_TX_PKTS,		/* Accelerated node TX packets */
	NSS_STATS_NODE_TX_BYTES,	/* Accelerated node TX bytes */
	NSS_STATS_NODE_RX_QUEUE_0_DROPPED,
					/* Accelerated node RX Queue 0 dropped */
	NSS_STATS_NODE_RX_QUEUE_1_DROPPED,
					/* Accelerated node RX Queue 1 dropped */
	NSS_STATS_NODE_RX_QUEUE_2_DROPPED,
					/* Accelerated node RX Queue 2 dropped */
	NSS_STATS_NODE_RX_QUEUE_3_DROPPED,
					/* Accelerated node RX Queue 3 dropped */

	NSS_STATS_NODE_MAX,
};

#define NSS_STATS_DECLARE_FILE_OPERATIONS(name) \
static const struct file_operations nss_##name##_stats_ops = { \
	.open = nss_stats_open, \
	.read = nss_##name##_stats_read, \
	.llseek = generic_file_llseek, \
	.release = nss_stats_release, \
};

/*
 * Private data for every file descriptor
 */
struct nss_stats_data {
	uint32_t if_num;	/**< Interface number for stats */
	uint32_t index;		/**< Index for GRE_REDIR stats */
	uint32_t edma_id;	/**< EDMA port ID or ring ID */
	struct nss_ctx_instance *nss_ctx;
				/**< The core for project stats */
};

int nss_stats_release(struct inode *inode, struct file *filp);
int nss_stats_open(struct inode *inode, struct file *filp);
void nss_stats_create_dentry(char *name, const struct file_operations *ops);
size_t nss_stats_fill_common_stats(uint32_t if_num, char *lbuf, size_t size_wr, size_t size_al);

#endif /* __NSS_STATS_H */
