/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/cache.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/rwlock_types.h>

#include "eip_priv.h"

static const char *svcname[EIP_SVC_MAX] = {
	[EIP_SVC_SKCIPHER] = "eip_skcipher",
	[EIP_SVC_AHASH] = "eip_ahash",
	[EIP_SVC_AEAD] = "eip_aead",
	[EIP_SVC_IPSEC] = "eip_ipsec",
	[EIP_SVC_HYBRID_IPSEC] = "eip_hy_ipsec",
};

/*
 * eip_ctx_read_tr_stats()
 *	Read ctx & TR statistics.
 */
static ssize_t eip_ctx_read_tr_stats(struct file *filep, char __user *ubuf, size_t count, loff_t *ppos)
{
	struct eip_ctx *ctx = filep->private_data;
	struct eip_flow_tbl *tbl;
	struct eip_tr *tr;
	ssize_t max_buf_len;
	uint32_t i = 0;
	ssize_t len;
	ssize_t ret;
	char *buf;

	/*
	 * We need to allocate space for the string and the value
	 */
	max_buf_len = (sizeof(struct eip_tr_stats)/sizeof(uint64_t)) * EIP_DEBUGFS_MAX_NAME;
	max_buf_len = ctx->tr.count * max_buf_len;
	max_buf_len += (sizeof(ctx->tr.stats)/sizeof(uint64_t)) * EIP_DEBUGFS_MAX_NAME;
	max_buf_len += sizeof(struct eip_flow_tuple) * EIP_DEBUGFS_MAX_NAME + sizeof(uint32_t);
	max_buf_len += (sizeof(struct eip_flow_tbl_stats)/sizeof(uint64_t)) * EIP_DEBUGFS_MAX_NAME;

	buf = vzalloc(max_buf_len);
	if (!buf)
		return 0;
	tbl = &ctx->ep->flow_table;

	/*
	 * Context Statistics.
	 */
	len = snprintf(buf, max_buf_len, "TR allocated - %llu\n", ctx->tr.stats.alloc);
	len += snprintf(buf + len, max_buf_len - len, "TR deallocated - %llu\n", ctx->tr.stats.dealloc);
	len += snprintf(buf + len, max_buf_len - len, "TR Freed - %llu\n", ctx->tr.stats.free);

	/*
	 * Take read lock for TR list iteration.
	 * TODO: Evaluate whether this lock can be merged with ep->lock.
	 */
	read_lock_bh(&ctx->tr.lock);

	list_for_each_entry(tr, &ctx->tr.head, node) {

		struct eip_tr_stats stats;

		/*
		 * TR statistics.
		 */
		eip_tr_get_stats(tr, &stats);
		len += snprintf(buf + len, max_buf_len - len, "Transform Record (%u) (%X) ----\n", i++, tr->tr_addr_type);
		len += snprintf(buf + len, max_buf_len - len, "\tActive Ref (%u)  ----\n", kref_read(&tr->ref));
		len += snprintf(buf + len, max_buf_len - len, "\tTx Packets - %llu\n", stats.tx_pkts);
		len += snprintf(buf + len, max_buf_len - len, "\tTx SG fragments - %llu\n", stats.tx_frags);
		len += snprintf(buf + len, max_buf_len - len, "\tTx bytes - %llu\n", stats.tx_bytes);
		len += snprintf(buf + len, max_buf_len - len, "\tTx Error len - %llu\n", stats.tx_error_len);
		len += snprintf(buf + len, max_buf_len - len, "\tRx Packets - %llu\n", stats.rx_pkts);
		len += snprintf(buf + len, max_buf_len - len, "\tRx bytes - %llu\n", stats.rx_bytes);
		len += snprintf(buf + len, max_buf_len - len, "\tError - %llu\n", stats.rx_error);
	}

	read_unlock_bh(&ctx->tr.lock);

	spin_lock_bh(&ctx->ep->lock);
        len += snprintf(buf + len, max_buf_len - len, "Flows allocated - %llu \n", tbl->stats.alloc);
        len += snprintf(buf + len, max_buf_len - len, "Flows deallocated - %llu \n", tbl->stats.free);
        len += snprintf(buf + len, max_buf_len - len, "Total collisions - %llu \n", tbl->stats.collision);
        len += snprintf(buf + len, max_buf_len - len, "Active collisions - %llu \n", tbl->stats.active_collision);
	len += snprintf(buf + len, max_buf_len - len, "Fail collisions - %llu \n", tbl->stats.fail_collision);
	spin_unlock_bh(&ctx->ep->lock);

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);
	vfree(buf);

	return ret;
}

/*
 * eip_ctx_final()
 *	Final ref cleanup.
 */
void eip_ctx_final(struct kref *kref)
{
	struct eip_ctx *ctx = container_of(kref, struct eip_ctx, ref);

	/*
	 * For each allocated token/sw, we increment TR's ref. And for each TR, we increment ctx's ref.
	 * So, TR head must be empty to ensure there is no kmem object leak.
	 */
	WARN_ON(!list_empty(&ctx->tr.head));

	debugfs_remove(ctx->dentry);
	kmem_cache_destroy(ctx->sw_cache);
	kmem_cache_destroy(ctx->tk_cache);
	memset(ctx, 0, sizeof(*ctx));
	kfree(ctx);
}

/*
 * eip_ctx_add_tr()
 *	Add TR from database.
 */
void eip_ctx_add_tr(struct eip_ctx *ctx, struct eip_tr *tr)
{
	write_lock_bh(&ctx->tr.lock);
	list_add(&tr->node, &ctx->tr.head);
	ctx->tr.count++;
	ctx->tr.stats.alloc++;
	write_unlock_bh(&ctx->tr.lock);
}

/*
 * eip_ctx_del_tr()
 *	Remove TR from database.
 */
void eip_ctx_del_tr(struct eip_ctx *ctx, struct eip_tr *tr)
{
	write_lock_bh(&ctx->tr.lock);
	list_del(&tr->node);
	ctx->tr.count--;
	ctx->tr.stats.free++;
	write_unlock_bh(&ctx->tr.lock);
}

/*
 * eip_ctx_algo_lookup()
 *	Lookup algo using given name.
 */
const struct eip_svc_entry *eip_ctx_algo_lookup(struct eip_ctx *ctx, char *algo_name)
{
	const struct eip_svc_entry *algo_list = ctx->svc_db;
	uint32_t size = ctx->db_size;
	uint32_t i;

	for (i = 0; i < size; i++) {
		if (!strncmp(algo_list[i].name, algo_name, CRYPTO_MAX_ALG_NAME)) {
			return &algo_list[i];
		}
	}

	return NULL;
}

static struct file_operations ctx_stats_ops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.llseek = default_llseek,
	.read = eip_ctx_read_tr_stats
};

/*
 * eip_ctx_alloc()
 *	Allocate the DMA context for given service.
 *
 * This function may sleep and should not be called form atomic context.
 * All DMA operations between clients and driver requires a valid EIP context.
 */
struct eip_ctx *eip_ctx_alloc(enum eip_svc svc, struct dentry **dentry)
{
	struct eip_pdev *ep = platform_get_drvdata(eip_drv_g.pdev);
	struct eip_ctx *ctx;
	char name[32];

	/*
	 * Validate the input.
	 */
	if (svc >= EIP_SVC_MAX) {
		pr_err("%px: Invalid EIP service\n", ep);
		return NULL;
	}

	/*
	 * Allocate Context.
	 */
	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		pr_err("%px: Failed to allocate EIP context\n", ep);
		return NULL;
	}

	/*
	 * Initialize per service algorithm databse.
	 */
	switch (svc) {
	case EIP_SVC_SKCIPHER:
		ctx->svc_db = eip_tr_skcipher_get_svc();
		ctx->db_size = eip_tr_skcipher_get_svc_len();
		ctx->dma = ep->la;
		break;
	case EIP_SVC_AHASH:
		ctx->svc_db = eip_tr_ahash_get_svc();
		ctx->db_size = eip_tr_ahash_get_svc_len();
		ctx->dma = ep->la;
		break;
	case EIP_SVC_AEAD:
		ctx->svc_db = eip_tr_aead_get_svc();
		ctx->db_size = eip_tr_aead_get_svc_len();
		ctx->dma = ep->la;
		break;
	/*
	 * TODO: Replace EIP_SVC_HYBRID_IPSEC with EIP_SVC_IPSEC.
	 * Use eip_features_t flags to derive remaining IPsec-related information.
	 */
	case EIP_SVC_HYBRID_IPSEC:
		ctx->svc_db = eip_tr_ipsec_get_svc();
		ctx->db_size = eip_tr_ipsec_get_svc_len();
		ctx->dma = ep->hy;
		break;
	default:
		pr_err("%px: Service database not found\n", ep);
		goto fail;
	}

	/*
	 * Allocate token cache. For IPsec we need tokens for TR auth key generation.
	 */
	snprintf(name, sizeof(name), "%s_tk@%p", svcname[svc], ctx);
	ctx->tk_cache = kmem_cache_create(name, sizeof(struct eip_tk), 0,
			SLAB_HWCACHE_ALIGN, NULL);
	if (!ctx->tk_cache) {
		pr_err("%px: Failed to allocate kmem cache\n", ep);
		goto fail;
	}

	/*
	 * Allocate sw descriptor cache.
	 */
	snprintf(name, sizeof(name), "%s_sw@%p", svcname[svc], ctx);
	ctx->sw_cache = kmem_cache_create(name, sizeof(struct eip_sw_desc), 0,
			SLAB_HWCACHE_ALIGN, NULL);
	if (!ctx->sw_cache) {
		pr_err("%px: Failed to allocate kmem cache\n", ep);
		goto fail_sw;
	}

	/*
	 * Initialize the object
	 */
	INIT_LIST_HEAD(&ctx->tr.head);
	rwlock_init(&ctx->tr.lock);
	kref_init(&ctx->ref);
	ctx->ep = ep;
	ctx->svc = svc;

	/*
	 * Add debugfs file.
	 */
	snprintf(name, sizeof(name), "%s_ctx@%p", svcname[svc], ctx);
	ctx->dentry = debugfs_create_file(name, S_IRUGO, ep->dentry, ctx, &ctx_stats_ops);

	*dentry = ep->dentry;
	return ctx;

fail_sw:
	kmem_cache_destroy(ctx->tk_cache);
fail:
	kfree(ctx);
	return NULL;
}
EXPORT_SYMBOL(eip_ctx_alloc);

/*
 * eip_ctx_free()
 *	Free the DMA context for given service.
 *
 * Caller need to ensure that, All old transform record is freed and No new allocation is scheduled.
 */
void eip_ctx_free(struct eip_ctx *ctx)
{
	/*
	 * Reference: eip_ctx_alloc()
	 */
	eip_ctx_deref(ctx);
}
EXPORT_SYMBOL(eip_ctx_free);

/*
 * eip_ctx_algo_supported()
 *     Check if the algo is supported by DMA.
 */
bool eip_ctx_algo_supported(struct eip_ctx *ctx, const char *algo_name)
{
	if (!eip_ctx_algo_lookup(ctx, (char *)algo_name)) {
		return false;
	}

	return true;
}
EXPORT_SYMBOL(eip_ctx_algo_supported);

/*
 * eip_tun_update
 *	Update Tunnel.
 */
int eip_tun_update(struct eip_tun *tun, struct eip_tun_update_info *info)
{
	/* Currently only TR update is supported for Encap flows. */
	if (!(info->valid_flags & EIP_TUN_TR_VALID)) {
		return -ENOTSUPP;
	}

	if (!info->new_tr) {
		pr_debug("%px : New TR is not valid\n", tun);
		return -EINVAL;
	}

	if (!eip_feature_check(tun->ctx, EIP_OFFLOAD_INNER_FLOW)) {
		pr_debug("%px : Context doesn't support inner flow acceleration", tun->ctx);
		return -EINVAL;
	}

	return eip_flow_update_by_tr(tun, info->tr, info->new_tr);
}
EXPORT_SYMBOL(eip_tun_update);

/*
 * eip_tun_alloc()
 *	Allocate and initialize tunnel context.
 */
struct eip_tun *eip_tun_alloc(struct eip_ctx *ctx)
{
	struct eip_tun *tun;

	tun = kzalloc(sizeof(struct eip_tun), GFP_KERNEL | __GFP_ZERO);
	if (!tun) {
		pr_err("%px : Failed to allocate tunnel context\n", ctx);
		return NULL;
	}

	tun->ctx = ctx;
	INIT_LIST_HEAD(&tun->flows);

	return tun;
}
EXPORT_SYMBOL(eip_tun_alloc);

/*
 * eip_tun_free()
 *	Free tunnel context.
 * NOTE:
 *      Caller must ensure that all tunnel-associated resources
 *      (including flows) are fully cleaned up before invoking
 *      this function.
 */
void eip_tun_free(struct eip_tun *tun)
{
	WARN_ON(!list_empty(&tun->flows));
	kfree(tun);
}
EXPORT_SYMBOL(eip_tun_free);
