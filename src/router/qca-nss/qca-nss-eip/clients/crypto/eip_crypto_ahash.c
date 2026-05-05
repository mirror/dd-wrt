/*
 * Copyright (c) 2022-2025, Qualcomm Innovation Center, Inc. All rights reserved.
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
 */

#include <linux/version.h>
#include <linux/crypto.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
#include <crypto/sha.h>
#else
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#endif
#include <crypto/hash.h>
#include <crypto/md5.h>
#include <crypto/algapi.h>
#include <crypto/internal/hash.h>
#include <crypto/scatterwalk.h>
#include "eip_crypto_ahash.h"

#define EIP_CRYPTO_AHASH_IN_PROG_BIT 	(1)

/*
 * eip_crypto_ahash_req_ctx
 *	Private context per ahash request.
 */
struct eip_crypto_ahash_req_ctx {
	uint32_t buf_count;			/* Buffer input count to limit 1 update per req */
	uint32_t diglen;			/* Digest length */
	unsigned long flags;			/* Update the result in final completion function */
	uint8_t digest[SHA512_DIGEST_SIZE];	/* Storage for resultant digest */
	struct scatterlist res;			/* Resultant scatterlist */
	bool restore_req;	/* Restore user request */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
	crypto_completion_t complete;		/* Callback function for finup */
	void *data;				/* Data for the callback function */
#endif
};

/*
 * Global AHASH context.
 */
static struct eip_crypto ahash = {0};

/*
 * Function prototypes.
 */
int eip_crypto_ahash_cra_init(struct crypto_tfm *tfm);
void eip_crypto_ahash_cra_exit(struct crypto_tfm *tfm);
int eip_crypto_ahash_update(struct ahash_request *req);
int eip_crypto_ahash_final(struct ahash_request *req);
int eip_crypto_ahash_digest(struct ahash_request *req);
int eip_crypto_ahash_req_init(struct ahash_request *req);
int eip_crypto_ahash_export(struct ahash_request *req, void *out);
int eip_crypto_ahash_import(struct ahash_request *req, const void *in);
int eip_crypto_ahash_setkey(struct crypto_ahash *ahash, const u8 *key, unsigned int keylen);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
int eip_crypto_ahash_finup(struct ahash_request *req);
#endif

/*
 * eip_crypto_ahash_algo.
 *	Template for ahash algorithms.
 */
static struct ahash_alg eip_crypto_ahash_algo[] = {
	/*
	 * Async non-keyed HASH digests
	 */
	{
		.init   = eip_crypto_ahash_req_init,
		.update = eip_crypto_ahash_update,
		.final  = eip_crypto_ahash_final,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
		.finup = eip_crypto_ahash_finup,
#endif
		.export = eip_crypto_ahash_export,
		.import = eip_crypto_ahash_import,
		.digest = eip_crypto_ahash_digest,
		.setkey = NULL,
		.halg   = {
			.digestsize = MD5_DIGEST_SIZE,
			.statesize  = sizeof(struct md5_state),
			.base       = {
				.cra_name        = "md5",
				.cra_driver_name = "eip-md5",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = MD5_HMAC_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct eip_crypto_ahash_tfm_ctx),
				.cra_alignmask   = 0,
				.cra_module      = THIS_MODULE,
				.cra_init        = eip_crypto_ahash_cra_init,
				.cra_exit        = eip_crypto_ahash_cra_exit,
			},
		},
	},
	{
		.init   = eip_crypto_ahash_req_init,
		.update = eip_crypto_ahash_update,
		.final  = eip_crypto_ahash_final,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
		.finup = eip_crypto_ahash_finup,
#endif
		.export = eip_crypto_ahash_export,
		.import = eip_crypto_ahash_import,
		.digest = eip_crypto_ahash_digest,
		.setkey = NULL,
		.halg   = {
			.digestsize = SHA1_DIGEST_SIZE,
			.statesize  = sizeof(struct sha1_state),
			.base       = {
				.cra_name        = "sha1",
				.cra_driver_name = "eip-sha1",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA1_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct eip_crypto_ahash_tfm_ctx),
				.cra_alignmask   = 0,
				.cra_module      = THIS_MODULE,
				.cra_init        = eip_crypto_ahash_cra_init,
				.cra_exit        = eip_crypto_ahash_cra_exit,
			},
		},
	},
	{
		.init   = eip_crypto_ahash_req_init,
		.update = eip_crypto_ahash_update,
		.final  = eip_crypto_ahash_final,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
		.finup = eip_crypto_ahash_finup,
#endif
		.export = eip_crypto_ahash_export,
		.import = eip_crypto_ahash_import,
		.digest = eip_crypto_ahash_digest,
		.setkey = NULL,
		.halg   = {
			.digestsize = SHA224_DIGEST_SIZE,
			.statesize  = sizeof(struct sha256_state),
			.base       = {
				.cra_name        = "sha224",
				.cra_driver_name = "eip-sha224",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA224_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct eip_crypto_ahash_tfm_ctx),
				.cra_alignmask   = 0,
				.cra_module      = THIS_MODULE,
				.cra_init        = eip_crypto_ahash_cra_init,
				.cra_exit        = eip_crypto_ahash_cra_exit,
			},
		},
	},
	{
		.init   = eip_crypto_ahash_req_init,
		.update = eip_crypto_ahash_update,
		.final  = eip_crypto_ahash_final,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
		.finup = eip_crypto_ahash_finup,
#endif
		.export = eip_crypto_ahash_export,
		.import = eip_crypto_ahash_import,
		.digest = eip_crypto_ahash_digest,
		.setkey = NULL,
		.halg   = {
			.digestsize = SHA256_DIGEST_SIZE,
			.statesize  = sizeof(struct sha256_state),
			.base       = {
				.cra_name        = "sha256",
				.cra_driver_name = "eip-sha256",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA256_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct eip_crypto_ahash_tfm_ctx),
				.cra_alignmask   = 0,
				.cra_module      = THIS_MODULE,
				.cra_init        = eip_crypto_ahash_cra_init,
				.cra_exit        = eip_crypto_ahash_cra_exit,
			},
		},
	},
	{
		.init   = eip_crypto_ahash_req_init,
		.update = eip_crypto_ahash_update,
		.final  = eip_crypto_ahash_final,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
		.finup = eip_crypto_ahash_finup,
#endif
		.export = eip_crypto_ahash_export,
		.import = eip_crypto_ahash_import,
		.digest = eip_crypto_ahash_digest,
		.setkey = NULL,
		.halg   = {
			.digestsize = SHA384_DIGEST_SIZE,
			.statesize  = sizeof(struct sha512_state),
			.base       = {
				.cra_name        = "sha384",
				.cra_driver_name = "eip-sha384",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA384_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct eip_crypto_ahash_tfm_ctx),
				.cra_alignmask   = 0,
				.cra_module      = THIS_MODULE,
				.cra_init        = eip_crypto_ahash_cra_init,
				.cra_exit        = eip_crypto_ahash_cra_exit,
			},
		},
	},
	{
		.init   = eip_crypto_ahash_req_init,
		.update = eip_crypto_ahash_update,
		.final  = eip_crypto_ahash_final,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
		.finup = eip_crypto_ahash_finup,
#endif
		.export = eip_crypto_ahash_export,
		.import = eip_crypto_ahash_import,
		.digest = eip_crypto_ahash_digest,
		.setkey = NULL,
		.halg   = {
			.digestsize = SHA512_DIGEST_SIZE,
			.statesize  = sizeof(struct sha512_state),
			.base       = {
				.cra_name        = "sha512",
				.cra_driver_name = "eip-sha512",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA512_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct eip_crypto_ahash_tfm_ctx),
				.cra_alignmask   = 0,
				.cra_module      = THIS_MODULE,
				.cra_init        = eip_crypto_ahash_cra_init,
				.cra_exit        = eip_crypto_ahash_cra_exit,
			},
		},
	},

	/*
	 * Aynsc keyed HMAC digests
	 */
	{
		.init   = eip_crypto_ahash_req_init,
		.update = eip_crypto_ahash_update,
		.final  = eip_crypto_ahash_final,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
		.finup = eip_crypto_ahash_finup,
#endif
		.export = eip_crypto_ahash_export,
		.import = eip_crypto_ahash_import,
		.digest = eip_crypto_ahash_digest,
		.setkey = eip_crypto_ahash_setkey,
		.halg   = {
			.digestsize = MD5_DIGEST_SIZE,
			.statesize  = sizeof(struct md5_state),
			.base       = {
				.cra_name        = "hmac(md5)",
				.cra_driver_name = "eip-hmac-md5",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = MD5_HMAC_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct eip_crypto_ahash_tfm_ctx),
				.cra_alignmask   = 0,
				.cra_module      = THIS_MODULE,
				.cra_init        = eip_crypto_ahash_cra_init,
				.cra_exit        = eip_crypto_ahash_cra_exit,
			},
		},
	},
	{
		.init   = eip_crypto_ahash_req_init,
		.update = eip_crypto_ahash_update,
		.final  = eip_crypto_ahash_final,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
		.finup = eip_crypto_ahash_finup,
#endif
		.export = eip_crypto_ahash_export,
		.import = eip_crypto_ahash_import,
		.digest = eip_crypto_ahash_digest,
		.setkey = eip_crypto_ahash_setkey,
		.halg   = {
			.digestsize = SHA1_DIGEST_SIZE,
			.statesize  = sizeof(struct sha1_state),
			.base       = {
				.cra_name        = "hmac(sha1)",
				.cra_driver_name = "eip-hmac-sha1",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA1_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct eip_crypto_ahash_tfm_ctx),
				.cra_alignmask   = 0,
				.cra_module      = THIS_MODULE,
				.cra_init        = eip_crypto_ahash_cra_init,
				.cra_exit        = eip_crypto_ahash_cra_exit,
			},
		},
	},
	{
		.init   = eip_crypto_ahash_req_init,
		.update = eip_crypto_ahash_update,
		.final  = eip_crypto_ahash_final,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
		.finup = eip_crypto_ahash_finup,
#endif
		.export = eip_crypto_ahash_export,
		.import = eip_crypto_ahash_import,
		.digest = eip_crypto_ahash_digest,
		.setkey = eip_crypto_ahash_setkey,
		.halg   = {
			.digestsize = SHA224_DIGEST_SIZE,
			.statesize  = sizeof(struct sha256_state),
			.base       = {
				.cra_name        = "hmac(sha224)",
				.cra_driver_name = "eip-hmac-sha224",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA224_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct eip_crypto_ahash_tfm_ctx),
				.cra_alignmask   = 0,
				.cra_module      = THIS_MODULE,
				.cra_init        = eip_crypto_ahash_cra_init,
				.cra_exit        = eip_crypto_ahash_cra_exit,
			},
		},
	},
	{
		.init   = eip_crypto_ahash_req_init,
		.update = eip_crypto_ahash_update,
		.final  = eip_crypto_ahash_final,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
		.finup = eip_crypto_ahash_finup,
#endif
		.export = eip_crypto_ahash_export,
		.import = eip_crypto_ahash_import,
		.digest = eip_crypto_ahash_digest,
		.setkey = eip_crypto_ahash_setkey,
		.halg   = {
			.digestsize = SHA256_DIGEST_SIZE,
			.statesize  = sizeof(struct sha256_state),
			.base       = {
				.cra_name        = "hmac(sha256)",
				.cra_driver_name = "eip-hmac-sha256",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA256_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct eip_crypto_ahash_tfm_ctx),
				.cra_alignmask   = 0,
				.cra_module      = THIS_MODULE,
				.cra_init        = eip_crypto_ahash_cra_init,
				.cra_exit        = eip_crypto_ahash_cra_exit,
			},
		},
	},
	{
		.init   = eip_crypto_ahash_req_init,
		.update = eip_crypto_ahash_update,
		.final  = eip_crypto_ahash_final,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
		.finup = eip_crypto_ahash_finup,
#endif
		.export = eip_crypto_ahash_export,
		.import = eip_crypto_ahash_import,
		.digest = eip_crypto_ahash_digest,
		.setkey = eip_crypto_ahash_setkey,
		.halg   = {
			.digestsize = SHA384_DIGEST_SIZE,
			.statesize  = sizeof(struct sha512_state),
			.base       = {
				.cra_name        = "hmac(sha384)",
				.cra_driver_name = "eip-hmac-sha384",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA384_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct eip_crypto_ahash_tfm_ctx),
				.cra_alignmask   = 0,
				.cra_module      = THIS_MODULE,
				.cra_init        = eip_crypto_ahash_cra_init,
				.cra_exit        = eip_crypto_ahash_cra_exit,
			},
		},
	},
	{
		.init   = eip_crypto_ahash_req_init,
		.update = eip_crypto_ahash_update,
		.final  = eip_crypto_ahash_final,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
		.finup = eip_crypto_ahash_finup,
#endif
		.export = eip_crypto_ahash_export,
		.import = eip_crypto_ahash_import,
		.digest = eip_crypto_ahash_digest,
		.setkey = eip_crypto_ahash_setkey,
		.halg   = {
			.digestsize = SHA512_DIGEST_SIZE,
			.statesize  = sizeof(struct sha512_state),
			.base       = {
				.cra_name        = "hmac(sha512)",
				.cra_driver_name = "eip-hmac-sha512",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA512_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct eip_crypto_ahash_tfm_ctx),
				.cra_alignmask   = 0,
				.cra_module      = THIS_MODULE,
				.cra_init        = eip_crypto_ahash_cra_init,
				.cra_exit        = eip_crypto_ahash_cra_exit,
			},
		},
	},
};

/*
 * eip_crypto_ahash_map_sg()
 *	Init and set pages to sg table from buffer.
 */
static inline int eip_crypto_ahash_map_sg(struct eip_crypto_ahash_tfm_ctx *actx)
{
	struct page *pages[EIP_CRYPTO_AHASH_MAX_PAGES];
	uint32_t data_len = actx->data_len;
	void *start = actx->data;
	int pg_cnt, len;

	for (pg_cnt = 0, len = 0; len < data_len; len += PAGE_SIZE, pg_cnt++)
		pages[pg_cnt] = vmalloc_to_page(start + len);

	return sg_alloc_table_from_pages(&actx->sgt, pages, pg_cnt, 0,
			actx->data_len, GFP_ATOMIC);
}

/*
 * eip_crypto_ahash_cra_init()
 *	Crypto ahash init
 */
int eip_crypto_ahash_cra_init(struct crypto_tfm *base)
{
	struct eip_crypto_ahash_tfm_ctx *actx = crypto_tfm_ctx(base);
	struct eip_crypto_tfm_ctx *tfm_ctx = &actx->ctx;
	struct crypto_ahash *tfm = __crypto_ahash_cast(base);

	BUG_ON(!tfm_ctx);

	/*
	 * Initialize TFM.
	 */
	memset(tfm_ctx, 0, sizeof(*tfm_ctx));
	init_completion(&tfm_ctx->complete);
	INIT_LIST_HEAD(&tfm_ctx->entry);

	/*
	 * Allocate buffer to store ahash data
	 */
	actx->data = vzalloc(EIP_CRYPTO_AHASH_MAX_SIZE);
	if (!actx->data) {
		pr_err("%px: AHASH unable to allocate memory for ahash data.\n", tfm_ctx);
		return -ENOMEM;
	}

	/*
	 * Allocate memory for per cpu stats.
	 */
	tfm_ctx->stats = alloc_percpu_gfp(struct eip_crypto_tfm_stats, GFP_KERNEL | __GFP_ZERO);
	if (!tfm_ctx->stats) {
		pr_err("%px: AHASH unable to allocate memory for tfm stats.\n", tfm_ctx);
		vfree(actx->data);
		return -ENOMEM;
	}

	/*
	 * Add TFM to list of TFMs.
	 */
	eip_crypto_add_tfmctx(&ahash, tfm_ctx);

	/*
	 * eip_crypto_ahash_cra_exit()
	 */
	atomic_inc(&tfm_ctx->usage_count);

	/*
	 * set this tfm reqsize to the transform specific structure
	 */
	crypto_ahash_set_reqsize(tfm, sizeof(struct eip_crypto_ahash_req_ctx));

	return 0;
}

/*
 * eip_crypto_ahash_cra_exit()
 *	Ahash exit function.
 */
void eip_crypto_ahash_cra_exit(struct crypto_tfm *tfm)
{
	struct eip_crypto_ahash_tfm_ctx *actx = crypto_tfm_ctx(tfm);
	struct eip_crypto_tfm_ctx *tfm_ctx = &actx->ctx;

	BUG_ON(!tfm_ctx);

	/*
	 * Mark tfm context as inactive
	 */
	atomic_set(&tfm_ctx->active, 0);

	/*
	 * We need to wait for any outstanding packet using this ctx.
	 * Once the last packet get processed, reference count will become
	 * 0 this ctx. We will wait for the reference to go down to 0.
	 */
	if (!atomic_sub_and_test(1, &tfm_ctx->usage_count)) {
		pr_debug("%px: AHASH waiting for usage count(%d) to become 0.\n", tfm_ctx,
				atomic_read(&tfm_ctx->usage_count));
		wait_for_completion(&tfm_ctx->complete);
	}

	/*
	 * Free the TR & stats
	 * TR may not be allocated if setkey is not called
	 */
	if(likely(tfm_ctx->tr)) {
		eip_tr_free(tfm_ctx->tr);
	}

	BUG_ON(!tfm_ctx->stats);
	free_percpu(tfm_ctx->stats);

	/*
	 * If any buffer or SG is allocated and not freed we free it here.
	 */
	sg_free_table(&actx->sgt);
	vfree(actx->data);
	actx->data_len = 0;
	actx->data = NULL;

	/*
	 * Remove the TFM from list of TFMs.
	 */
	eip_crypto_del_tfmctx(&ahash, tfm_ctx);

	/*
	 * We signal when all the TFMs have been destroyed.
	 */
	if (!list_empty(&ahash.tfms.head)) {
		complete(&ahash.complete);
	}
};

/*
 * eip_crypto_ahash_stats_read()
 *	Read AHASH statistics.
 */
static ssize_t eip_crypto_ahash_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	uint64_t tfm_count = atomic_read(&ahash.tfms.tfm_count);
	struct eip_crypto_tfm_stats stats;
	struct eip_crypto_tfm_ctx *tfm;
	uint64_t tfm_usage_count = 0;
	ssize_t bytes_read = 0;
	size_t size_wr = 0;
	int count = 0;
	char *buf;

	size_t stats_sz = EIP_CRYPTO_STATS_STR_LEN * (sizeof(struct eip_crypto_tfm_stats)/sizeof(uint64_t) + 5);
	stats_sz *= (tfm_count + 1); /* Stats for all TFMs + For combined tfm stats */

	buf = vzalloc(stats_sz);
	if (unlikely(buf == NULL)) {
		pr_warn("%px: AHASH unable to allocate memory for stats.\n", fp);
		return -ENOMEM;
	}

	/*
	 * Get summarized stats for global AHASH object.
	 */
	eip_crypto_get_summary_stats(ahash.stats, &stats);

	/*
	 * Print stats.
	 */
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "\nAHASH service stats start:\n");
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t\t = %llu\n", "tx_reqs", stats.tx_reqs);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tx_bytes", stats.tx_bytes);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tx_failures", stats.tx_failures);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t\t = %llu\n", "rx_reqs", stats.rx_reqs);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "rx_bytes", stats.rx_bytes);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "rx_failures", stats.rx_failures);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "hw_err", stats.hw_err);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "invalid_key", stats.invalid_key);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "mem_alloc_err", stats.mem_alloc_err);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tr_alloc_err", stats.tr_alloc_err);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "inactive_tfm", stats.inactive_tfm);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tfm_count", tfm_count);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "invalid_len", stats.invalid_len);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "AHASH service stats end.\n");

	/*
	 * TODO: Per TFM stats, this needs to be moved to a common function
	 */
	read_lock_bh(&ahash.tfms.lock);
	list_for_each_entry(tfm, &ahash.tfms.head, entry) {
		/*
		 * Get summarized stats for global AHASH object.
		 */
		eip_crypto_get_summary_stats(ahash.stats, &stats);

		tfm_usage_count = atomic_read(&tfm->usage_count);

		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "\nTFM(%d) stats start:\n", count);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t\t = %llu\n", "tx_reqs", stats.tx_reqs);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tx_bytes", stats.tx_bytes);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tx_failures", stats.tx_failures);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t\t = %llu\n", "rx_reqs", stats.rx_reqs);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "rx_bytes", stats.rx_bytes);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "rx_failures", stats.rx_failures);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "hw_err", stats.hw_err);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "invalid_key", stats.invalid_key);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "mem_alloc_err", stats.mem_alloc_err);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tr_alloc_err", stats.tr_alloc_err);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "inactive_tfm", stats.inactive_tfm);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tfm_usage_count", tfm_usage_count);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "invalid_len", stats.invalid_len);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "TFM(%d) stats end.\n", count);
		count++;
	}

	read_unlock_bh(&ahash.tfms.lock);
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, buf, size_wr);
	vfree(buf);
	return bytes_read;
}

/*
 * File operation structure instance
 */
static const struct file_operations file_ops = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = eip_crypto_ahash_stats_read
};

/*
 * eip_crypto_ahash_done()
 *	Hash request completion callback function
 */
void eip_crypto_ahash_done(void *app_data, eip_req_t eip_req)
{
	struct eip_crypto_ahash_tfm_ctx *actx = (struct eip_crypto_ahash_tfm_ctx *)app_data;
	struct eip_crypto_tfm_ctx *tfm_ctx = &actx->ctx;

	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *ahash_stats = this_cpu_ptr(ahash.stats);
	struct ahash_request *req = eip_req2ahash_request(eip_req);
	struct eip_crypto_ahash_req_ctx *rctx = ahash_request_ctx(req);
	uint32_t data_len = 0;

	BUG_ON(!tfm_ctx);

	memcpy(req->result, rctx->digest, rctx->diglen);

	/*
	 * Restore original src and nbytes in request
	 */
	if (rctx->restore_req) {
		req->src = actx->sg_user;
		req->nbytes = actx->sg_user_len;
		data_len = actx->data_len;
	}

	/*
	 * free sg table and reset data length.
	 */
	if (data_len) {
		sg_free_table(&actx->sgt);
		actx->data_len = 0;
	}
	/*
	 * Clear the in progress flag
	 */
	clear_bit(EIP_CRYPTO_AHASH_IN_PROG_BIT, &rctx->flags);

	/*
	 * Increment stats.
	 */
	tfm_stats->rx_reqs++;
	ahash_stats->rx_reqs++;
	tfm_stats->rx_bytes += data_len;
	ahash_stats->rx_bytes += data_len;

	/*
	 * Signal linux about completion.
	 */
	ahash_request_complete(req, 0);

	/*
	 * Decrement tfm usage count and signal complete
	 * once usage count becomes zero.
	 */
	BUG_ON(atomic_read(&tfm_ctx->usage_count) == 0);
	if (atomic_sub_and_test(1, &tfm_ctx->usage_count)) {
	        complete(&tfm_ctx->complete);
	}
}

/*
 * eip_crypto_ahash_done()
 *	Hash request completion callback function
 */
void eip_crypto_ahash_err_done(void *app_data, eip_req_t eip_req, int err)
{
	struct eip_crypto_ahash_tfm_ctx *actx = (struct eip_crypto_ahash_tfm_ctx *)app_data;
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(actx->ctx.stats);
	struct eip_crypto_tfm_stats *ahash_stats = this_cpu_ptr(ahash.stats);

	BUG_ON(!actx);

	/*
	 * Increment hardware stats.
	 */
	if (err) {
		tfm_stats->hw_err += !!err;
		ahash_stats->hw_err += !!err;
	}

	eip_crypto_ahash_done(app_data, eip_req);
}

/*
 * eip_crypto_ahash_setkey()
 *	Ahash setkey routine for hmac(sha1).
 */
int eip_crypto_ahash_setkey(struct crypto_ahash *tfm, const u8 *key, unsigned int keylen)
{
	struct eip_crypto_ahash_tfm_ctx *actx = crypto_tfm_ctx(crypto_ahash_tfm(tfm));
	char *alg_name = (char *)crypto_tfm_alg_driver_name(crypto_ahash_tfm(tfm));
	struct eip_crypto_tfm_ctx *tfm_ctx = &actx->ctx;

	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *ahash_stats = this_cpu_ptr(ahash.stats);
	struct eip_tr_info_crypto *crypto;
	struct eip_tr_info info = {0};
	struct eip_tr_base *base;
	void *tr, *old_tr;


	base = &info.base;
	crypto = &info.crypto;

	/*
	 * Fill EIP crypto session data
	 */
	strscpy(base->alg_name, alg_name, CRYPTO_MAX_ALG_NAME);
	base->auth.key_data = key;
	base->auth.key_len = keylen;

	/*
	 * Set callbacks and app data.
	 */
	crypto->auth_cb = eip_crypto_ahash_done;
	crypto->auth_err_cb = eip_crypto_ahash_err_done;
	crypto->app_data = actx;
	crypto->enc_cb = NULL;
	crypto->dec_cb = NULL;

	/*
	 * Allocate a new TR.
	 */
	tr = eip_tr_alloc(ahash.dma_ctx, &info);
	if (!tr) {
		pr_warn("%px: Unable to allocate new TR.\n", tfm_ctx);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		crypto_ahash_set_flags(tfm, CRYPTO_TFM_RES_BAD_FLAGS);
#endif
		tfm_stats->tr_alloc_err++;
		ahash_stats->tr_alloc_err++;
		return -EBUSY;
	}

	/*
	 * There cannot be race condition between setkey and tfm exit,
	 * hence we are not taking any write lock.
	 */
	old_tr = tfm_ctx->tr;
	rcu_assign_pointer(tfm_ctx->tr, tr);
	if (old_tr) {
		synchronize_rcu();
		eip_tr_free(old_tr);
	}

	/*
	 * Mark tfm as active.
	 */
	atomic_set(&tfm_ctx->active, 1);

	return 0;
}

/*
 * eip_crypto_ahash_req_init()
 *	Ahash request to initialize hash operation.
 *	Initialize the private context for the specific request.
 *	Since this is called for non-keyed hash instead of setkey we need to allocate TR here.
 */
int eip_crypto_ahash_req_init(struct ahash_request *req)
{
	struct crypto_tfm *tfm = crypto_ahash_tfm(crypto_ahash_reqtfm(req));
	struct eip_crypto_ahash_tfm_ctx *actx = crypto_tfm_ctx(tfm);
	struct eip_crypto_ahash_req_ctx *rctx = ahash_request_ctx(req);
	struct eip_crypto_tfm_ctx *tfm_ctx = &actx->ctx;
	char *alg_name = (char *)crypto_tfm_alg_driver_name(tfm);
	struct eip_crypto_tfm_stats *ahash_stats;
	struct eip_crypto_tfm_stats *tfm_stats;
	struct eip_tr_info_crypto *crypto;
	struct eip_tr_info info = {0};
	struct eip_tr_base *base;
	void *tr, *old_tr;

	rctx->restore_req = false;
	tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	ahash_stats = this_cpu_ptr(ahash.stats);

	/*
	 * Check if TR already exists.
	 */
	if(tfm_ctx->tr){
		goto tr_allocated;
	}

	base = &info.base;
	crypto = &info.crypto;

	/*
	 * Fill EIP crypto session data
	 */
	strscpy(base->alg_name, alg_name, CRYPTO_MAX_ALG_NAME);
	base->auth.key_data = NULL;

	/*
	 * Set callbacks and app data.
	 */
	crypto->auth_cb = eip_crypto_ahash_done;
	crypto->auth_err_cb = eip_crypto_ahash_err_done;
	crypto->app_data = actx;
	crypto->enc_cb = NULL;
	crypto->dec_cb = NULL;

	/*
	 * Allocate a new TR.
	 */
	tr = eip_tr_alloc(ahash.dma_ctx, &info);
	if (!tr) {
		pr_warn("%px: Unable to allocate new TR.\n", tfm_ctx);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		crypto_ahash_set_flags(crypto_ahash_reqtfm(req), CRYPTO_TFM_RES_BAD_FLAGS);
#endif
		tfm_stats->tr_alloc_err++;
		ahash_stats->tr_alloc_err++;

		return -EBUSY;
	}

	/*
	 * There cannot be race condition between setkey and tfm exit,
	 * hence we are not taking any write lock.
	 */
	old_tr = tfm_ctx->tr;
	rcu_assign_pointer(tfm_ctx->tr, tr);
	if (old_tr) {
		synchronize_rcu();
		eip_tr_free(old_tr);
	}

	/*
	 * Mark tfm as active.
	 */
	atomic_set(&tfm_ctx->active, 1);

tr_allocated:
	/*
	 * Initialze the private context
	 */
	rctx->buf_count = 0;
	rctx->diglen = crypto_ahash_digestsize(crypto_ahash_reqtfm(req));
	clear_bit(EIP_CRYPTO_AHASH_IN_PROG_BIT, &rctx->flags);
	memset(rctx->digest, 0, ARRAY_SIZE(rctx->digest));
	actx->data_len = 0;

	return 0;
};

/*
 * eip_crypto_ahash_send()
 *	Set request and send stored data to HW for processing.
 */
static int eip_crypto_ahash_send(struct ahash_request *req)
{
	struct eip_crypto_ahash_tfm_ctx *actx = crypto_ahash_ctx(crypto_ahash_reqtfm(req));
	struct eip_crypto_ahash_req_ctx *rctx = ahash_request_ctx(req);
	struct eip_crypto_tfm_ctx *tfm_ctx = &actx->ctx;

	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *ahash_stats = this_cpu_ptr(ahash.stats);
	struct eip_tr *tr = NULL;
	int ret;

	/*
	 * Initialize scatterlist for hash result and data.
	 */
	sg_init_one(&rctx->res, rctx->digest, rctx->diglen);

	/*
	 * We take RCU lock to synchronize encrypt/decrypt and setkey operation.
	 */
	rcu_read_lock_bh();
	tr = rcu_dereference(tfm_ctx->tr);

	/*
	 * Increment the usage count for tfm & set flag for ahash in progress
	 */
	atomic_inc(&tfm_ctx->usage_count);
	set_bit(EIP_CRYPTO_AHASH_IN_PROG_BIT, &rctx->flags);

	/*
	 * Call for hash generation.
	 */
	ret = eip_tr_ahash_auth(tr, req, &rctx->res);
	if (ret) {
		pr_debug("%px: Authentication error: %d\n", rctx, ret);
		atomic_dec(&tfm_ctx->usage_count);
		clear_bit(EIP_CRYPTO_AHASH_IN_PROG_BIT, &rctx->flags);
		rcu_read_unlock_bh();
		tfm_stats->tx_failures++;
		ahash_stats->tx_failures++;

		return ret;
	}

	rcu_read_unlock_bh();

	/*
	 * Update stats.
	 */
	tfm_stats->tx_reqs++;
	ahash_stats->tx_reqs++;
	tfm_stats->tx_bytes += actx->data_len;
	ahash_stats->tx_bytes += actx->data_len;

	return -EINPROGRESS;
}

/*
 * eip_crypto_ahash_update()
 *	Ahash .update operation for a specific request.
 *
 * Buffering the data to sg_list.
 * Do not return the hash result to user.
 */
int eip_crypto_ahash_update(struct ahash_request *req)
{
	struct eip_crypto_ahash_tfm_ctx *actx = crypto_ahash_ctx(crypto_ahash_reqtfm(req));
	struct eip_crypto_tfm_ctx *tfm_ctx = &actx->ctx;

	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *ahash_stats = this_cpu_ptr(ahash.stats);
	uint32_t data_len = actx->data_len;
	uint32_t req_len = req->nbytes;
	int ret = 0;

	/*
	 * Check if tfm is active or not.
	 */
	if (!atomic_read(&tfm_ctx->active)) {
		pr_warn("%px: Tfm is not active.\n", tfm_ctx);
		tfm_stats->inactive_tfm++;
		ahash_stats->inactive_tfm++;
		ret = -EPERM;
		goto fail;
	}

	if (req->nbytes == 0) {
		return 0;
	}

	if (data_len + req_len > EIP_CRYPTO_AHASH_MAX_SIZE) {
		pr_warn("%p: Unable to handle data length more then 64Kb\n", tfm_ctx);
		tfm_stats->invalid_len++;
		ahash_stats->invalid_len++;

		ret = -EFBIG;
		goto fail;
	}

	scatterwalk_map_and_copy(actx->data + data_len, req->src, 0, req_len, 0);
	actx->data_len += req_len;

	return 0;

fail:
	/*
	 * Reset data len.
	 */
	actx->data_len = 0;

	return ret;
}

/*
 * eip_crypto_ahash_final()
 *	Ahash .final operation for a specific request.
 *	Copy the hash result to user
 */
int eip_crypto_ahash_final(struct ahash_request *req)
{
	struct eip_crypto_ahash_tfm_ctx *actx = crypto_ahash_ctx(crypto_ahash_reqtfm(req));
	struct eip_crypto_ahash_req_ctx *rctx = ahash_request_ctx(req);
	struct scatterlist *src = &rctx->res;
	int err;

	/*
	 * store original src and nbytes from req.
	 */
	rctx->restore_req = true;
	actx->sg_user = req->src;
	actx->sg_user_len = req->nbytes;

	/*
	 * if eip_crypto_ahash_map_sg returns error same error return to user
	 */
	if (actx->data_len) {
		err = eip_crypto_ahash_map_sg(actx);
		if (err) {
			goto fail;
		}

		src = actx->sgt.sgl;
	}

	req->src = src;
	req->nbytes = actx->data_len;

	return eip_crypto_ahash_send(req);

fail:
	/*
	 * Reset data length.
	 */
	actx->data_len = 0;


	return err;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
/*
 * eip_crypto_ahash_finup_done
 * Ahash finup completion callback
 */
static void eip_crypto_ahash_finup_done(void *data, int err)
{
	struct ahash_request *req = data;
	struct eip_crypto_ahash_req_ctx *rctx = ahash_request_ctx(req);

	/*
	 * Call final if there are no errors
	 * Restore the original complete callback and data in the req
	 */
	req->base.complete = rctx->complete;
	req->base.data = rctx->data;
	ahash_request_complete(req, err);

	return;
}

/*
 * eip_crypto_ahash_finup
 * Ahash .finup operation
 */
int eip_crypto_ahash_finup(struct ahash_request *req)
{
	struct eip_crypto_ahash_req_ctx *rctx = ahash_request_ctx(req);
	int err;

	/*
	 * Store the original complete callback and data in rctx
	 */
	rctx->complete = req->base.complete;
	rctx->data = req->base.data;

	req->base.complete = eip_crypto_ahash_finup_done;
	req->base.data = req;

	/*
	 * If update returns 0, call final.
	 */
	err = eip_crypto_ahash_update(req);
	if (!err) {
		err = eip_crypto_ahash_final(req);
	}

	/*
	 * If final returns -EINPROGRESS, return the same to user
	 */
	if (err == -EINPROGRESS) {
		return err;
	}

	/*
	 * In case of errors, restore the complete callback and data in req
	 */
	req->base.complete = rctx->complete;
	req->base.data = rctx->data;

	return err;
}
#endif

/*
 * eip_crypto_ahash_digest()
 *	Ahash digest operation.
 */
int eip_crypto_ahash_digest(struct ahash_request *req)
{
	struct eip_crypto_ahash_tfm_ctx *actx = crypto_ahash_ctx(crypto_ahash_reqtfm(req));
	struct eip_crypto_tfm_ctx *tfm_ctx = &actx->ctx;

	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *ahash_stats = this_cpu_ptr(ahash.stats);
	int err = 0;

	/*
	 * Initialization is followed by sending data to the hardware.
	 *
	 * In case of a digest operation, copying user data to a local buffer
	 * is not required as its required for multiple update calls.
	 * Digest process follows the sequence: init --> update --> final,
	 * we can directly send data to EIP HW.
	 */
	err = eip_crypto_ahash_req_init(req);
	if (err) {
		pr_warn("%px: Tfm init failed with error(%d) for digest operation.\n",
				tfm_ctx, err);
		tfm_stats->tx_failures++;
		ahash_stats->tx_failures++;
		return -EINVAL;
	}

	return eip_crypto_ahash_send(req);
}

/*
 * eip_crypto_ahash_export()
 *	Ahash .export operation.
 *	Used to export data to other transform.
 *
 * Note: This API is not supported.
 */
int eip_crypto_ahash_export(struct ahash_request *req, void *out)
{
	struct eip_crypto_ahash_tfm_ctx *tfm_ctx __attribute__((unused)) = crypto_tfm_ctx(req->base.tfm);

	pr_debug("%px: ahash .export is not supported", tfm_ctx);
	return -ENOSYS;
};

/*
 * eip_crypto_ahash_import()
 *	Ahash .import operation.
 *	Used to import data from other transform.
 *
 * Note: This API is not supported.
 */
int eip_crypto_ahash_import(struct ahash_request *req, const void *in)
{
	struct eip_crypto_ahash_tfm_ctx *tfm_ctx __attribute__((unused)) = crypto_tfm_ctx(req->base.tfm);

	pr_debug("%px: ahash .import is not supported", tfm_ctx);
	return -ENOSYS;
}

/*
 * eip_crypto_ahash_init()
 *	Initialize ahash.
 */
int eip_crypto_ahash_init(struct device_node *node)
{
	struct ahash_alg *algo = eip_crypto_ahash_algo;
	struct dentry *parent;
	int i = 0;

	/*
	 * Allocate DMA context.
	 */
	ahash.dma_ctx = eip_ctx_alloc(EIP_SVC_AHASH, &parent);
	if (!ahash.dma_ctx) {
		pr_warn("%px: Unable to allocate DMA context for AHASH.\n", &ahash);
		return -EINVAL;
	}

	/*
	 * Create debugfs entry for ahash service.
	 */
	ahash.root = debugfs_create_dir("ahash", parent);
	if (!ahash.root) {
		pr_warn("%px: Unable to create debug entry for ahash.\n", &ahash);
		goto free_ctx;
	}

	if (!debugfs_create_file("stats", S_IRUGO, ahash.root, NULL, &file_ops)) {
		pr_warn("%px: Unable to create stats debug entry.\n", &ahash);
		goto free_debugfs;
	}

	INIT_LIST_HEAD(&ahash.tfms.head);
	init_completion(&ahash.complete);
	rwlock_init(&ahash.tfms.lock);

	/*
	 * Allocate memory for SKCipher global stats object.
	 */
	ahash.stats = alloc_percpu_gfp(struct eip_crypto_tfm_stats, GFP_KERNEL | __GFP_ZERO);
	if (!ahash.stats) {
		pr_err("%px: Unable to allocate memory for skcipher stats.\n", &ahash);
		goto free_debugfs;
	}


	/*
	 * Register ahash templates with matching algo name with linux.
	 */
	for (i = 0; i < ARRAY_SIZE(eip_crypto_ahash_algo); i++, algo++) {
		const char *alg_name = algo->halg.base.cra_driver_name;

		if (!eip_ctx_algo_supported(ahash.dma_ctx, alg_name)) {
			continue;
		}

		if (crypto_register_ahash(algo)) {
			pr_warn("%px: AHASH registration failed(%s)\n", node, alg_name);
			algo->halg.base.cra_flags = CRYPTO_ALG_DEAD;
			continue;
		}

		pr_info("%px: Template registered: %s\n", node, alg_name);
	}

	return 0;
free_debugfs:
	debugfs_remove_recursive(ahash.root);
free_ctx:
	eip_ctx_free(ahash.dma_ctx);
	return -ENOMEM;
}

/*
 * eip_crypto_ahash_exit()
 *	De-Initializes ahash cipher.
 */
void eip_crypto_ahash_exit(struct device_node *node)
{
	struct ahash_alg *algo = eip_crypto_ahash_algo;
	int i;

	/*
	 * Unregister template with linux.
	 */
	for (i = 0; i < ARRAY_SIZE(eip_crypto_ahash_algo); i++, algo++) {
		const char *alg_name = algo->halg.base.cra_driver_name;

		if (!eip_ctx_algo_supported(ahash.dma_ctx, alg_name) ||
				(algo->halg.base.cra_flags & CRYPTO_ALG_DEAD)) {
			pr_info("node: %px alg_name: %s.\n", node, alg_name);
			continue;
		}

		crypto_unregister_ahash(algo);
		pr_info("%px: AHASH un-registration success. Algo: %s.\n", node, alg_name);
	}

	/*
	 * Remove debug entry & stats
	 */
	debugfs_remove_recursive(ahash.root);

	BUG_ON(!ahash.stats);
	free_percpu(ahash.stats);

	/*
	 * Free the dma context.
	 */
	eip_ctx_free(ahash.dma_ctx);
	memset(&ahash, 0, sizeof(ahash));
}
