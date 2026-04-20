/* Copyright (c) 2014-2015, 2017-2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 */

#include <linux/debugfs.h>

#include "nss_crypto_hlos.h"
#include <nss_api_if.h>

#include <nss_crypto_defines.h>
#include <nss_crypto_api.h>
#include <nss_crypto_hdr.h>

#include "nss_crypto_ctrl.h"
#include "nss_crypto_debugfs.h"

#define NSS_CRYPTO_DEBUGFS_NAME_SZ 128 /* bytes */

/*
 * nss_crypto_debugs_add_node_stats()
 *	Creates debugs entries for common statistics
 *
 * crypto will use only queue0, hence we capture dropped
 * statistics only for queue0.
 */
static void nss_crypto_debugfs_add_node_stats(struct dentry *parent, struct nss_crypto_stats_pvt *stats)
{
	debugfs_create_u64("completed", S_IRUGO, parent, &stats->rx_packets);
	debugfs_create_u64("processed", S_IRUGO, parent, &stats->tx_packets);
	debugfs_create_u64("dropped", S_IRUGO, parent, &stats->rx_dropped[0]);
}

/*
 * nss_crypto_debugs_add_stats()
 *	Creates debugs entries for common statistics
 *
 * crypto will use only queue0, hence we capture dropped
 * statistics only for queue0.
 */
static void nss_crypto_debugfs_add_ctx_stats(struct dentry *parent, struct nss_crypto_stats_pvt *stats)
{
	debugfs_create_u64("rx_packets", S_IRUGO, parent, &stats->rx_packets);
	debugfs_create_u64("rx_bytes", S_IRUGO, parent, &stats->rx_bytes);
	debugfs_create_u64("rx_dropped", S_IRUGO, parent, &stats->rx_dropped[0]);
	debugfs_create_u64("tx_packets", S_IRUGO, parent, &stats->tx_packets);
	debugfs_create_u64("tx_bytes", S_IRUGO, parent, &stats->tx_bytes);
	debugfs_create_u64("fail_version", S_IRUGO, parent, &stats->fail_version);
	debugfs_create_u64("fail_index", S_IRUGO, parent, &stats->fail_ctx);
	debugfs_create_u64("fail_dma", S_IRUGO, parent, &stats->fail_dma);
}

/*
 * nss_crypto_debugfs_add_ctrl_stats()
 * 	Creates debugfs entries for Host maintained control statistics
 */
static void nss_crypto_debugfs_add_ctrl_stats(struct dentry *parent, struct nss_crypto_ctrl_stats *stats)
{
	debugfs_create_atomic_t("ctx_alloc", S_IRUGO, parent, &stats->alloc);
	debugfs_create_atomic_t("ctx_alloc_fail_node", S_IRUGO, parent, &stats->alloc_fail_node);
	debugfs_create_atomic_t("ctx_alloc_fail_nomem", S_IRUGO, parent, &stats->alloc_fail_nomem);
	debugfs_create_atomic_t("ctx_alloc_fail_nospace", S_IRUGO, parent, &stats->alloc_fail_nospace);
	debugfs_create_atomic_t("ctx_alloc_fail_noalgo", S_IRUGO, parent, &stats->alloc_fail_noalgo);
	debugfs_create_atomic_t("ctx_alloc_fail_noresp", S_IRUGO, parent, &stats->alloc_fail_noresp);

	debugfs_create_atomic_t("ctx_free", S_IRUGO, parent, &stats->free);
	debugfs_create_atomic_t("ctx_free_fail_queue", S_IRUGO, parent, &stats->free_fail_queue);
	debugfs_create_atomic_t("ctx_free_fail_msg", S_IRUGO, parent, &stats->free_fail_msg);
	debugfs_create_atomic_t("ctx_free_fail_inuse", S_IRUGO, parent, &stats->free_fail_inuse);
	debugfs_create_atomic_t("ctx_free_delayed", S_IRUGO, parent, &stats->free_delayed);
}

/*
 * nss_crypto_debugfs_add_control()
 *	control specific stats entry
 */
void nss_crypto_debugfs_add_control(struct nss_crypto_ctrl *ctrl)
{
	/*
	 * Create a debugfs entry corresponding to host stats
	 */
	ctrl->ctrl_dentry = debugfs_create_dir("control", ctrl->dentry);
	if (ctrl->ctrl_dentry == NULL) {
		nss_crypto_err("Unable to create qca-nss-crypto/control directory in debugfs");
		return;
	}

	/*
	 * create host stats files
	 */
	nss_crypto_debugfs_add_ctrl_stats(ctrl->ctrl_dentry, &ctrl->cstats);
}

/*
 * nss_crypto_debugfs_add_node()
 *	node specific stats entry
 */
void nss_crypto_debugfs_add_node(struct nss_crypto_node *node, struct dentry *root, const char *name)
{
	/*
	 * Non availability of debugfs directory is not a catastrophy
	 * We can still go ahead with other initialization
	 */
	if (!root)
		return;

	node->dentry = debugfs_create_dir(name, root);
	if (!node->dentry)
		return;

	/*
	 * create node stats files
	 */
	nss_crypto_debugfs_add_node_stats(node->dentry, &node->stats);
}

/*
 * nss_crypto_debugfs_add_engine()
 *	Creates per engine debugfs entries
 */
void nss_crypto_debugfs_add_engine(struct nss_crypto_engine *eng, struct dentry *root)
{
	char buf[NSS_CRYPTO_DEBUGFS_NAME_SZ] = {0};
	struct nss_crypto_debugfs_entry *entry;
	size_t num_entries;
	int i = 0;

	if (!root) {
		nss_crypto_warn("%px: no root debugfs entry found\n", eng);
		return;
	}

	scnprintf(buf, sizeof(buf), "engine-0x%x", eng->id);

	eng->dentry = debugfs_create_dir(buf, root);
	if (!eng->dentry) {
		nss_crypto_warn("%px: failed to create engine debugfs dentry\n", eng);
		return;
	}

	entry = eng->debugfs_entries;
	num_entries = eng->debugfs_num_entries;
	for ( ; i < num_entries; i++, entry++) {
		debugfs_create_file(entry->name, S_IRUGO, eng->dentry, eng, &entry->ops);
	}

	return;
}

/*
 * nss_crypto_debugfs_del_engine()
 *	Delete per engine debugfs entries
 */
void nss_crypto_debugfs_del_engine(struct nss_crypto_engine *eng)
{
	if (eng->dentry)
		debugfs_remove_recursive(eng->dentry);
}

/*
 * nss_crypto_debugfs_add()
 * 	add a named debugfs stats entry
 */
void nss_crypto_debugfs_add_ctx(struct nss_crypto_ctx *ctx, struct dentry *root)
{
	char buf[NSS_CRYPTO_DEBUGFS_NAME_SZ] = {0};

	if (!root)
		return;

	if (ctx->algo > NSS_CRYPTO_CMN_ALGO_MAX)
		return;

	scnprintf(buf, sizeof(buf), "ctx%d", ctx->idx);

	ctx->dentry = debugfs_create_dir(buf, root);
	if (!ctx->dentry)
		return;

	/*
	 * publish the supported algorithm
	 */
	debugfs_create_u32(ctx->info->name, S_IRUGO, ctx->dentry, &ctx->algo);

	/*
	 * create context stats files
	 */
	nss_crypto_debugfs_add_ctx_stats(ctx->dentry, &ctx->stats);
}

/*
 * nss_crypto_debugfs_del_ctx()
 *	delete a debugfs context entry
 */
void nss_crypto_debugfs_del_ctx(struct nss_crypto_ctx *ctx)
{
	if (!ctx->dentry)
		return;

	debugfs_remove_recursive(ctx->dentry);
	ctx->dentry = NULL;
}
