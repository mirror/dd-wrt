/*
 **************************************************************************
 * Copyright (c) 2017, 2020, The Linux Foundation. All rights reserved.
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
 * nss_dtlsmgr_ctx_stats.c
 *	NSS DTLS Manager context statistics
 */

#include <linux/atomic.h>
#include <linux/crypto.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/rtnetlink.h>
#include <linux/types.h>
#include <linux/version.h>

#include <nss_api_if.h>
#include <nss_dtlsmgr.h>

#include "nss_dtlsmgr_private.h"

#define NSS_DTLSMGR_STATS_MAX_STR_LENGTH 64
#define NSS_DTLSMGR_STATS_EXTRA_LINES 16

extern struct nss_dtlsmgr g_dtls;

/*
 * nss_dtlsmgr_ctx_fill_hw_error_stats()
 *	Fill hardware error statistics
 */
static ssize_t nss_dtlsmgr_ctx_fill_hw_error_stats(struct nss_dtlsmgr_stats *stats, char *buf,
						   ssize_t max_buf_len, ssize_t len)
{
	int i;

	len += snprintf(buf + len, max_buf_len, "\nHardware Errors\n---------------\n");
	len += snprintf(buf + len, max_buf_len, "length_error        = %lld\n", stats->fail_hw.len_error);
	len += snprintf(buf + len, max_buf_len, "token_error         = %lld\n", stats->fail_hw.token_error);
	len += snprintf(buf + len, max_buf_len, "bypass_error        = %lld\n", stats->fail_hw.bypass_error);
	len += snprintf(buf + len, max_buf_len, "config_error        = %lld\n", stats->fail_hw.config_error);
	len += snprintf(buf + len, max_buf_len, "algo_error          = %lld\n", stats->fail_hw.algo_error);
	len += snprintf(buf + len, max_buf_len, "hash_ovf_error      = %lld\n", stats->fail_hw.hash_ovf_error);
	len += snprintf(buf + len, max_buf_len, "ttl_error           = %lld\n", stats->fail_hw.ttl_error);
	len += snprintf(buf + len, max_buf_len, "csum_error          = %lld\n", stats->fail_hw.csum_error);
	len += snprintf(buf + len, max_buf_len, "timeout_error       = %lld\n", stats->fail_hw.timeout_error);

	len += snprintf(buf + len, max_buf_len, "\nClassifcation Errors\n---------------------\n");
	for (i = 0; i < NSS_DTLS_CMN_CLE_MAX; i++) {
		/*
		 * Don't print if there are no errors
		 */
		if (!stats->fail_cle[i])
			continue;
		len += snprintf(buf + len, max_buf_len, "cle_error_%02d   - %lld\n", i, stats->fail_cle[i]);
	}

	return len;
}

/*
 * nss_dtlsmgr_ctx_encap_stats_read()
 *	Read the DTLS encapsulation statistics.
 */
static ssize_t nss_dtlsmgr_ctx_encap_stats_read(struct file *filep, char __user *buffer, size_t count, loff_t *ppos)
{
	struct nss_dtlsmgr_ctx *ctx = filep->private_data;
	struct nss_dtlsmgr_stats *stats;
	ssize_t max_buf_len, len, ret;
	char *buf;

	NSS_DTLSMGR_VERIFY_MAGIC(ctx);

	/*
	 * (Lines of output) * (Max string length)
	 */
	max_buf_len = ((sizeof(*stats) / sizeof(uint64_t)) + NSS_DTLSMGR_STATS_EXTRA_LINES) *
		      NSS_DTLSMGR_STATS_MAX_STR_LENGTH;

	buf = vzalloc(max_buf_len);
	if (!buf) {
		nss_dtlsmgr_warn("Failed to allocate memory for statistic buffer");
		return 0;
	}

	/*
	 * Fill encap statistics
	 */
	stats = &ctx->encap.stats;
	len = snprintf(buf, max_buf_len, "ENCAPSULATION INTERFACE (%d) STATISTICS\n", ctx->encap.ifnum);
	len += snprintf(buf + len, max_buf_len - len, "rx_packets           = %lld\n", stats->rx_packets);
	len += snprintf(buf + len, max_buf_len - len, "rx_bytes             = %lld\n", stats->rx_bytes);
	len += snprintf(buf + len, max_buf_len - len, "rx_dropped           = %lld\n", stats->rx_dropped);
	len += snprintf(buf + len, max_buf_len - len, "rx_single_rec        = %lld\n", stats->rx_single_rec);
	len += snprintf(buf + len, max_buf_len - len, "rx_multi_rec         = %lld\n", stats->rx_multi_rec);
	len += snprintf(buf + len, max_buf_len - len, "tx_packets           = %lld\n", stats->tx_packets);
	len += snprintf(buf + len, max_buf_len - len, "tx_bytes             = %lld\n", stats->tx_bytes);
	len += snprintf(buf + len, max_buf_len - len, "fail_crypto_resource = %lld\n", stats->fail_crypto_resource);
	len += snprintf(buf + len, max_buf_len - len, "fail_crypto_enqueue  = %lld\n", stats->fail_crypto_enqueue);
	len += snprintf(buf + len, max_buf_len - len, "fail_headroom        = %lld\n", stats->fail_headroom);
	len += snprintf(buf + len, max_buf_len - len, "fail_tailroom        = %lld\n", stats->fail_tailroom);
	len += snprintf(buf + len, max_buf_len - len, "fail_queue           = %lld\n", stats->fail_queue);
	len += snprintf(buf + len, max_buf_len - len, "fail_queue_nexthop   = %lld\n", stats->fail_queue_nexthop);
	len += snprintf(buf + len, max_buf_len - len, "fail_pbuf_alloc      = %lld\n", stats->fail_pbuf_alloc);
	len += snprintf(buf + len, max_buf_len - len, "fail_pbuf_linear     = %lld\n", stats->fail_pbuf_linear);
	len += snprintf(buf + len, max_buf_len - len, "fail_pbuf_stats      = %lld\n", stats->fail_pbuf_stats);
	len += snprintf(buf + len, max_buf_len - len, "fail_pbuf_align      = %lld\n", stats->fail_pbuf_align);
	len += snprintf(buf + len, max_buf_len - len, "fail_ctx_active      = %lld\n", stats->fail_ctx_active);
	len += snprintf(buf + len, max_buf_len - len, "fail_hwctx_active    = %lld\n", stats->fail_hwctx_active);
	len += snprintf(buf + len, max_buf_len - len, "fail_cipher          = %lld\n", stats->fail_cipher);
	len += snprintf(buf + len, max_buf_len - len, "fail_auth            = %lld\n", stats->fail_auth);
	len += snprintf(buf + len, max_buf_len - len, "fail_seq_overflow    = %lld\n", stats->fail_seq_ovf);
	len += snprintf(buf + len, max_buf_len - len, "fail_host_tx         = %lld\n", stats->fail_host_tx);
	len += snprintf(buf + len, max_buf_len - len, "fail_host_rx         = %lld\n", stats->fail_host_rx);

	/* Returns total number of bytes written to the buffer */
	len = nss_dtlsmgr_ctx_fill_hw_error_stats(stats, buf, max_buf_len, len);

	ret = simple_read_from_buffer(buffer, count, ppos, buf, len);
	vfree(buf);

	return ret;
}
/*
 * nss_dtlsmgr_ctx_decap_stats_read()
 *	Read the DTLS decapsulation statistics.
 */
static ssize_t nss_dtlsmgr_ctx_decap_stats_read(struct file *filep, char __user *buffer, size_t count, loff_t *ppos)
{
	struct nss_dtlsmgr_ctx *ctx = filep->private_data;
	struct nss_dtlsmgr_stats *stats;
	ssize_t max_buf_len, ret;
	ssize_t len = 0;
	char *buf;

	NSS_DTLSMGR_VERIFY_MAGIC(ctx);

	/*
	 * (Lines of output) * (Max string length)
	 */
	max_buf_len = ((sizeof(*stats) / sizeof(uint64_t)) + NSS_DTLSMGR_STATS_EXTRA_LINES) *
		      NSS_DTLSMGR_STATS_MAX_STR_LENGTH;

	buf = vzalloc(max_buf_len);
	if (!buf) {
		nss_dtlsmgr_warn("Failed to allocate memory for statistic buffer");
		return 0;
	}

	/*
	 * Fill decap statistics
	 */
	stats = &ctx->decap.stats;
	len += snprintf(buf, max_buf_len - len, "DECAPSULATION INTERFACE (%d) STATISTICS\n", ctx->decap.ifnum);
	len += snprintf(buf + len, max_buf_len - len, "rx_packets           = %lld\n", stats->rx_packets);
	len += snprintf(buf + len, max_buf_len - len, "rx_bytes             = %lld\n", stats->rx_bytes);
	len += snprintf(buf + len, max_buf_len - len, "rx_dropped           = %lld\n", stats->rx_dropped);
	len += snprintf(buf + len, max_buf_len - len, "rx_single_rec        = %lld\n", stats->rx_single_rec);
	len += snprintf(buf + len, max_buf_len - len, "rx_multi_rec         = %lld\n", stats->rx_multi_rec);
	len += snprintf(buf + len, max_buf_len - len, "tx_packets           = %lld\n", stats->tx_packets);
	len += snprintf(buf + len, max_buf_len - len, "tx_bytes             = %lld\n", stats->tx_bytes);
	len += snprintf(buf + len, max_buf_len - len, "fail_crypto_resource = %lld\n", stats->fail_crypto_resource);
	len += snprintf(buf + len, max_buf_len - len, "fail_crypto_enqueue  = %lld\n", stats->fail_crypto_enqueue);
	len += snprintf(buf + len, max_buf_len - len, "fail_version         = %lld\n", stats->fail_ver);
	len += snprintf(buf + len, max_buf_len - len, "fail_epoch           = %lld\n", stats->fail_epoch);
	len += snprintf(buf + len, max_buf_len - len, "fail_dtls_record     = %lld\n", stats->fail_dtls_record);
	len += snprintf(buf + len, max_buf_len - len, "fail_capwap          = %lld\n", stats->fail_capwap);
	len += snprintf(buf + len, max_buf_len - len, "fail_replay          = %lld\n", stats->fail_replay);
	len += snprintf(buf + len, max_buf_len - len, "fail_replay_dup      = %lld\n", stats->fail_replay_dup);
	len += snprintf(buf + len, max_buf_len - len, "fail_replay_window   = %lld\n", stats->fail_replay_win);
	len += snprintf(buf + len, max_buf_len - len, "fail_queue           = %lld\n", stats->fail_queue);
	len += snprintf(buf + len, max_buf_len - len, "fail_queue_nexthop   = %lld\n", stats->fail_queue_nexthop);
	len += snprintf(buf + len, max_buf_len - len, "fail_pbuf_alloc      = %lld\n", stats->fail_pbuf_alloc);
	len += snprintf(buf + len, max_buf_len - len, "fail_pbuf_linear     = %lld\n", stats->fail_pbuf_linear);
	len += snprintf(buf + len, max_buf_len - len, "fail_pbuf_stats      = %lld\n", stats->fail_pbuf_stats);
	len += snprintf(buf + len, max_buf_len - len, "fail_pbuf_align      = %lld\n", stats->fail_pbuf_align);
	len += snprintf(buf + len, max_buf_len - len, "fail_ctx_active      = %lld\n", stats->fail_ctx_active);
	len += snprintf(buf + len, max_buf_len - len, "fail_hwctx_active    = %lld\n", stats->fail_hwctx_active);
	len += snprintf(buf + len, max_buf_len - len, "fail_cipher          = %lld\n", stats->fail_cipher);
	len += snprintf(buf + len, max_buf_len - len, "fail_auth            = %lld\n", stats->fail_auth);
	len += snprintf(buf + len, max_buf_len - len, "fail_seq_overflow    = %lld\n", stats->fail_seq_ovf);
	len += snprintf(buf + len, max_buf_len - len, "fail_block_length    = %lld\n", stats->fail_blk_len);
	len += snprintf(buf + len, max_buf_len - len, "fail_hash_length     = %lld\n", stats->fail_hash_len);
	len += snprintf(buf + len, max_buf_len - len, "fail_host_tx         = %lld\n", stats->fail_host_tx);
	len += snprintf(buf + len, max_buf_len - len, "fail_host_rx         = %lld\n", stats->fail_host_rx);

	/* Returns total number of bytes written to the buffer */
	len = nss_dtlsmgr_ctx_fill_hw_error_stats(stats, buf, max_buf_len, len);

	ret = simple_read_from_buffer(buffer, count, ppos, buf, len);
	vfree(buf);

	return ret;
}

/*
 * Context file operation structure instance
 */
static const struct file_operations nss_dtlsmgr_encap_stats_op = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = nss_dtlsmgr_ctx_encap_stats_read,
};

static const struct file_operations nss_dtlsmgr_decap_stats_op = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = nss_dtlsmgr_ctx_decap_stats_read,
};

/*
 * nss_dtlmsgr_create_debugfs()
 *	Create debugfs files to display session statistics.
 */
int nss_dtlsmgr_create_debugfs(struct nss_dtlsmgr_ctx *ctx)
{
	struct nss_dtlsmgr *drv = &g_dtls;

	ctx->dentry = debugfs_create_dir(ctx->dev->name, drv->root_dir);
	if (!ctx->dentry) {
		nss_dtlsmgr_warn("failed to create debugfs directory");
		return -1;
	}

	debugfs_create_file("encap_stats", S_IRUGO, ctx->dentry, ctx, &nss_dtlsmgr_encap_stats_op);
	debugfs_create_file("decap_stats", S_IRUGO, ctx->dentry, ctx, &nss_dtlsmgr_decap_stats_op);
	return 0;
}
