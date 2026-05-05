/*
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <linux/of.h>

#include <crypto/aes.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
#include <crypto/sha.h>
#else
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#endif
#include <crypto/md5.h>
#include <crypto/aead.h>
#include <crypto/ctr.h>
#include <crypto/gcm.h>
#include <crypto/ghash.h>
#include <crypto/authenc.h>
#include <crypto/internal/aead.h>
#include <crypto/internal/des.h>

#include "eip_crypto.h"

/*
 * Global AEAD context.
 */
static struct eip_crypto aead_g = {0};

/*
 * Function declaration.
 */
static int eip_crypto_aead_setkey_noauth(struct crypto_aead *tfm, const u8 *key, unsigned int keylen);
static int eip_crypto_aead_setkey(struct crypto_aead *tfm, const u8 *key, unsigned int keylen);
static void eip_crypto_aead_tfm_exit(struct crypto_aead *aead_tfm);
static int eip_crypto_aead_tfm_init(struct crypto_aead *aead_tfm);
static int eip_crypto_aead_encrypt(struct aead_request *req);
static int eip_crypto_aead_decrypt(struct aead_request *req);

/*
 * eip_crypto_aead_algs
 *	Template for AEAD algorithms.
 */
static struct aead_alg eip_crypto_aead_algs[] = {
	{
		/*
		 * aes-cbc, MD5
		 */
		.base = {
			.cra_name 	= "authenc(hmac(md5),cbc(aes))",
			.cra_driver_name = "eip-aes-cbc-md5-hmac",
			.cra_priority 	= 10000,
			.cra_flags 	= CRYPTO_ALG_ASYNC,
			.cra_blocksize 	= AES_BLOCK_SIZE,
			.cra_ctxsize 	= sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask 	= 0,
			.cra_module 	= THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= AES_BLOCK_SIZE,
		.maxauthsize 	= MD5_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{
		/*
		 * aes-cbc, sha1
		 */
		.base = {
			.cra_name 	= "authenc(hmac(sha1),cbc(aes))",
			.cra_driver_name = "eip-aes-cbc-sha1-hmac",
			.cra_priority 	= 10000,
			.cra_flags 	= CRYPTO_ALG_ASYNC,
			.cra_blocksize 	= AES_BLOCK_SIZE,
			.cra_ctxsize 	= sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask 	= 0,
			.cra_module 	= THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= AES_BLOCK_SIZE,
		.maxauthsize 	= SHA1_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{
		/*
		 * aes-ctr-rfc3686, md5
		 */
		.base = {
			.cra_name       = "authenc(hmac(md5),rfc3686(ctr(aes)))",
			.cra_driver_name = "eip-aes-ctr-rfc3686-md5-hmac",
			.cra_priority   = 10000,
			.cra_flags      = CRYPTO_ALG_ASYNC,
			.cra_blocksize  = AES_BLOCK_SIZE,
			.cra_ctxsize    = sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask  = 0,
			.cra_module     = THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= CTR_RFC3686_IV_SIZE,
		.maxauthsize 	= MD5_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{
		/*
		 * aes-ctr-rfc3686, sha1
		 */
		.base = {
			.cra_name       = "authenc(hmac(sha1),rfc3686(ctr(aes)))",
			.cra_driver_name = "eip-aes-ctr-rfc3686-sha1-hmac",
			.cra_priority   = 10000,
			.cra_flags      = CRYPTO_ALG_ASYNC,
			.cra_blocksize  = AES_BLOCK_SIZE,
			.cra_ctxsize    = sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask  = 0,
			.cra_module     = THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= CTR_RFC3686_IV_SIZE,
		.maxauthsize 	= SHA1_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{	/*
		 * aes-cbc, sha256
		 */
		.base = {
			.cra_name = "authenc(hmac(sha256),cbc(aes))",
			.cra_driver_name = "eip-aes-cbc-sha256-hmac",
			.cra_priority = 10000,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= AES_BLOCK_SIZE,
		.maxauthsize 	= SHA256_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{	/*
		 * aes-ctr-rfc3686, sha256
		 */
		.base = {
			.cra_name       = "authenc(hmac(sha256),rfc3686(ctr(aes)))",
			.cra_driver_name = "eip-aes-ctr-rfc3686-sha256-hmac",
			.cra_priority   = 10000,
			.cra_flags      = CRYPTO_ALG_ASYNC,
			.cra_blocksize  = AES_BLOCK_SIZE,
			.cra_ctxsize    = sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask  = 0,
			.cra_module     = THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= CTR_RFC3686_IV_SIZE,
		.maxauthsize 	= SHA256_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{
		/*
		 * aes-ctr-rfc3686, sha384
		 */
		.base = {
			.cra_name       = "authenc(hmac(sha384),rfc3686(ctr(aes)))",
			.cra_driver_name = "eip-aes-ctr-rfc3686-sha384-hmac",
			.cra_priority   = 10000,
			.cra_flags      = CRYPTO_ALG_ASYNC,
			.cra_blocksize  = AES_BLOCK_SIZE,
			.cra_ctxsize    = sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask  = 0,
			.cra_module     = THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= CTR_RFC3686_IV_SIZE,
		.maxauthsize 	= SHA384_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{
		/*
		 * aes-ctr-rfc3686, sha512
		 */
		.base = {
			.cra_name       = "authenc(hmac(sha512),rfc3686(ctr(aes)))",
			.cra_driver_name = "eip-aes-ctr-rfc3686-sha512-hmac",
			.cra_priority   = 10000,
			.cra_flags      = CRYPTO_ALG_ASYNC,
			.cra_blocksize  = AES_BLOCK_SIZE,
			.cra_ctxsize    = sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask  = 0,
			.cra_module     = THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= CTR_RFC3686_IV_SIZE,
		.maxauthsize 	= SHA512_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{	/*
		 * aes-cbc, sha384
		 */
		.base = {
			.cra_name 	= "authenc(hmac(sha384),cbc(aes))",
			.cra_driver_name = "eip-aes-cbc-sha384-hmac",
			.cra_priority 	= 10000,
			.cra_flags 	= CRYPTO_ALG_ASYNC,
			.cra_blocksize 	= AES_BLOCK_SIZE,
			.cra_ctxsize 	= sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask 	= 0,
			.cra_module 	= THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= AES_BLOCK_SIZE,
		.maxauthsize 	= SHA384_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{	/*
		 * aes-cbc, sha512
		 */
		.base = {
			.cra_name 	= "authenc(hmac(sha512),cbc(aes))",
			.cra_driver_name = "eip-aes-cbc-sha512-hmac",
			.cra_priority 	= 10000,
			.cra_flags 	= CRYPTO_ALG_ASYNC,
			.cra_blocksize 	= AES_BLOCK_SIZE,
			.cra_ctxsize 	= sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask 	= 0,
			.cra_module 	= THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= AES_BLOCK_SIZE,
		.maxauthsize 	= SHA512_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{	/*
		 * RFC4106, GCM
		 */
		.base = {
			.cra_name 	= "rfc4106(gcm(aes))",
			.cra_driver_name = "eip-aes-gcm-rfc4106",
			.cra_priority 	= 10000,
			.cra_flags 	= CRYPTO_ALG_ASYNC,
			.cra_blocksize 	= 1,
			.cra_ctxsize 	= sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask 	= 0,
			.cra_module 	= THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= GCM_RFC4106_IV_SIZE,
		.maxauthsize 	= GHASH_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey_noauth,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{	/*
		 * GCM
		 */
		.base = {
			.cra_name 	= "gcm(aes)",
			.cra_driver_name = "eip-aes-gcm",
			.cra_priority 	= 10000,
			.cra_flags 	= CRYPTO_ALG_ASYNC,
			.cra_blocksize 	= 1,
			.cra_ctxsize 	= sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask 	= 0,
			.cra_module 	= THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= GCM_AES_IV_SIZE,
		.maxauthsize 	= GHASH_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey_noauth,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{
		/*
		 * 3des-cbc, md5
		 */
		.base = {
			.cra_name       = "authenc(hmac(md5),cbc(des3_ede))",
			.cra_driver_name = "eip-3des-cbc-md5-hmac",
			.cra_priority   = 10000,
			.cra_flags      = CRYPTO_ALG_ASYNC,
			.cra_blocksize  = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize    = sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask  = 0,
			.cra_module     = THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= DES3_EDE_BLOCK_SIZE,
		.maxauthsize 	= MD5_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{
		/*
		 * 3des-cbc, sha1
		 */
		.base = {
			.cra_name       = "authenc(hmac(sha1),cbc(des3_ede))",
			.cra_driver_name = "eip-3des-cbc-sha1-hmac",
			.cra_priority   = 10000,
			.cra_flags      = CRYPTO_ALG_ASYNC,
			.cra_blocksize  = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize    = sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask  = 0,
			.cra_module     = THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= DES3_EDE_BLOCK_SIZE,
		.maxauthsize 	= SHA1_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
	{
		/*
		 * 3des-cbc, sha256
		 */
		.base = {
			.cra_name       = "authenc(hmac(sha256),cbc(des3_ede))",
			.cra_driver_name = "eip-3des-cbc-sha256-hmac",
			.cra_priority   = 10000,
			.cra_flags      = CRYPTO_ALG_ASYNC,
			.cra_blocksize  = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize    = sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask  = 0,
			.cra_module     = THIS_MODULE,
		},

		.init 		= eip_crypto_aead_tfm_init,
		.exit 		= eip_crypto_aead_tfm_exit,
		.ivsize 	= DES3_EDE_BLOCK_SIZE,
		.maxauthsize 	= SHA256_DIGEST_SIZE,
		.setkey 	= eip_crypto_aead_setkey,
		.encrypt 	= eip_crypto_aead_encrypt,
		.decrypt 	= eip_crypto_aead_decrypt,
	},
};

/*
 * eip_crypto_aead_tfm_init()
 *	TFM initialization.
 */
static int eip_crypto_aead_tfm_init(struct crypto_aead *aead_tfm)
{
	struct crypto_tfm *base = crypto_aead_tfm(aead_tfm);
	struct eip_crypto_tfm_ctx *tfm_ctx = crypto_tfm_ctx(base);

	BUG_ON(!tfm_ctx);

	/*
	 * Initialize TFM.
	 */
	memset(tfm_ctx, 0, sizeof(*tfm_ctx));
	init_completion(&tfm_ctx->complete);
	INIT_LIST_HEAD(&tfm_ctx->entry);

	/*
	 * Decrement: eip_crypto_aead_tfm_exit()
	 */
	atomic_inc(&tfm_ctx->usage_count);

	/*
	 * Allocate memory for per cpu stats.
	 */
	tfm_ctx->stats = alloc_percpu_gfp(struct eip_crypto_tfm_stats, GFP_KERNEL | __GFP_ZERO);
	if (!tfm_ctx->stats) {
		pr_err("%px: Unable to allocate memory for tfm stats.\n", tfm_ctx);
		return -ENOMEM;
	}

	/*
	 * Add TFM to list of TFMs.
	 */
	eip_crypto_add_tfmctx(&aead_g, tfm_ctx);

	return 0;
}

/*
 * eip_crypto_aead_tfm_exit()
 *	TFM de-initialization.
 */
static void eip_crypto_aead_tfm_exit(struct crypto_aead *aead_tfm)
{
	struct crypto_tfm *base = crypto_aead_tfm(aead_tfm);
	struct eip_crypto_tfm_ctx *tfm_ctx = crypto_tfm_ctx(base);

	BUG_ON(!tfm_ctx);

	atomic_set(&tfm_ctx->active, 0);

	/*
	 * We wait for outstanding packets to get processed.
	 * This is possible if there are inflight packets
	 * while the unregister is called.
	 */
	if (!atomic_sub_and_test(1, &tfm_ctx->usage_count)) {
		pr_debug("%px: AEAD waiting for usage count(%d) to become 0.\n",
				tfm_ctx, atomic_read(&tfm_ctx->usage_count));
		wait_for_completion(&tfm_ctx->complete);
	}

	/*
	 * Free the TR & stats
	 * TR is not allocated when setkey is not called
	 */
	if(likely(tfm_ctx->tr)) {
		eip_tr_free(tfm_ctx->tr);
	}

	BUG_ON(!tfm_ctx->stats);
	free_percpu(tfm_ctx->stats);

	/*
	 * Remove the TFM from list of TFMs.
	 */
	eip_crypto_del_tfmctx(&aead_g, tfm_ctx);

	/*
	 * We signal when all the TFMs have been destroyed.
	 */
	if (!list_empty(&aead_g.tfms.head)) {
		complete(&aead_g.complete);
	}
}

/*
 * eip_crypto_aead_stats_read()
 *	Read AEAD statistics.
 */
static ssize_t eip_crypto_aead_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	uint64_t tfm_count = atomic_read(&aead_g.tfms.tfm_count);
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
		pr_warn("%px: Unable to allocate memory for stats.\n", fp);
		return 0;
	}

	/*
	 * Get summarized stats for global AEAD object.
	 */
	eip_crypto_get_summary_stats(aead_g.stats, &stats);

	/*
	 * Print stats.
	 */
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "\nAEAD service stats start:\n");
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t\t = %llu\n", "tx_reqs", stats.tx_reqs);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tx_bytes", stats.tx_bytes);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tx_failures", stats.tx_failures);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t\t = %llu\n", "rx_reqs", stats.rx_reqs);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "rx_bytes", stats.rx_bytes);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "rx_failures", stats.rx_failures);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t\t = %llu\n", "hw_err", stats.hw_err);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "invalid_key", stats.invalid_key);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "mem_alloc_err", stats.mem_alloc_err);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tr_alloc_err", stats.tr_alloc_err);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "inactive_tfm", stats.inactive_tfm);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tfm_count", tfm_count);
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "AEAD service stats end.\n");

	/*
	 * Per TFM stats.
	 */
	read_lock_bh(&aead_g.tfms.lock);
	list_for_each_entry(tfm, &aead_g.tfms.head, entry) {

		/*
		 * Get summarized stats for global AEAD object.
		 */
		eip_crypto_get_summary_stats(aead_g.stats, &stats);

		tfm_usage_count = atomic_read(&tfm->usage_count);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "\nTFM(%d) stats start.\n", count);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t\t = %llu\n", "tx_reqs", stats.tx_reqs);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tx_bytes", stats.tx_bytes);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tx_failures", stats.tx_failures);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t\t = %llu\n", "rx_reqs", stats.rx_reqs);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "rx_bytes", stats.rx_bytes);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "rx_failures", stats.rx_failures);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t\t = %llu\n", "hw_err", stats.hw_err);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "invalid_key", stats.invalid_key);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "mem_alloc_err", stats.mem_alloc_err);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tr_alloc_err", stats.tr_alloc_err);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "inactive_tfm", stats.inactive_tfm);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "%s\t = %llu\n", "tfm_usage_count",
				tfm_usage_count);
		size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "TFM(%d) stats end.\n", count);
		count++;
	}

	read_unlock_bh(&aead_g.tfms.lock);

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
	.read = eip_crypto_aead_stats_read
};

/*
 * eip_crypto_aead_done()
 *	AEAD callback function. Called after transformation is done.
 */
static void eip_crypto_aead_done(void *app_data, eip_req_t eip_req)
{
	struct eip_crypto_tfm_ctx *tfm_ctx = (struct eip_crypto_tfm_ctx *)app_data;
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *aead_stats = this_cpu_ptr(aead_g.stats);
	struct aead_request *req = eip_req2aead_request(eip_req);

	/*
	 * Increment stats.
	 */
	tfm_stats->rx_reqs++;
	aead_stats->rx_reqs++;
	tfm_stats->rx_bytes += req->cryptlen;
	aead_stats->rx_bytes += req->cryptlen;

	/*
	 * Signal linux about completion.
	 */
	aead_request_complete(req, 0);

	/*
	 * Signal tfm complete once tfm usage count becomes zero.
	 */
	if (atomic_sub_and_test(1, &tfm_ctx->usage_count)) {
		complete(&tfm_ctx->complete);
	}
}

/*
 * eip_crypto_aead_err()
 *	AEAD callback function. Called for transformation error.
 */
static void eip_crypto_aead_err(void *app_data, eip_req_t eip_req, int err)
{
	struct eip_crypto_tfm_ctx *tfm_ctx = (struct eip_crypto_tfm_ctx *)app_data;
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *aead_stats = this_cpu_ptr(aead_g.stats);

	/*
	 * Check for errors and increment stats.
	 */
	if (err) {
		tfm_stats->hw_err++;
		aead_stats->hw_err++;
	}

	eip_crypto_aead_done(app_data, eip_req);
}

/*
 * eip_crypto_aead_setkey_noauth()
 *      Sets key for transformation with no authentication.
 */
static int eip_crypto_aead_setkey_noauth(struct crypto_aead *tfm, const u8 *key, unsigned int keylen)
{
	struct eip_crypto_tfm_ctx *tfm_ctx = crypto_tfm_ctx(crypto_aead_tfm(tfm));
	const char *alg_name = (char *)crypto_tfm_alg_driver_name(crypto_aead_tfm(tfm));
	struct eip_tr_info_crypto *crypto;
	struct eip_tr_info info = {0};
	struct eip_tr *tr, *old_tr;
	struct eip_tr_base *base;
	uint32_t nonce = 0;

	BUG_ON(!tfm_ctx);

	base = &info.base;
	crypto = &info.crypto;

	/*
	 * In case of rfc4106 nonce is present as part of key in last 4 bytes.
	 * We copy it here so it can be used in data path.
	 */
	if (strstr(alg_name, "gcm-rfc4106")) {
		keylen -= CTR_RFC3686_NONCE_SIZE;
		memcpy(&nonce, key + keylen, CTR_RFC3686_NONCE_SIZE);
	}

	/*
	 * Fill info object.
	 */
	strscpy(base->alg_name, alg_name, CRYPTO_MAX_ALG_NAME);
	base->cipher.key_data = key;
	base->cipher.key_len = keylen;
	base->nonce = nonce;

	/*
	 * Set callbacks and app data.
	 */
	crypto->enc_cb = eip_crypto_aead_done;
	crypto->enc_err_cb = eip_crypto_aead_err;
	crypto->dec_cb = eip_crypto_aead_done;
	crypto->dec_err_cb = eip_crypto_aead_err;
	crypto->app_data = tfm_ctx;

	/*
	 * Allocate a new TR.
	 */
	tr = eip_tr_alloc(aead_g.dma_ctx, &info);
	if (!tr) {
		pr_warn("%px: Unable to allocate new TR.\n", tfm_ctx);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		crypto_aead_set_flags(tfm, CRYPTO_TFM_RES_BAD_FLAGS);
#endif
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
 * eip_crypto_aead_setkey()
 *	Used to set key for transformation.
 */
static int eip_crypto_aead_setkey(struct crypto_aead *tfm, const u8 *key, unsigned int keylen)
{
	struct eip_crypto_tfm_ctx *tfm_ctx = crypto_tfm_ctx(crypto_aead_tfm(tfm));
	char *alg_name = (char *)crypto_tfm_alg_driver_name(crypto_aead_tfm(tfm));
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *aead_stats = this_cpu_ptr(aead_g.stats);
	struct crypto_authenc_keys keys = {0};
	struct eip_tr_info_crypto *crypto;
	struct eip_tr_info info = {0};
	struct eip_tr_base *base;
	uint32_t nonce = 0;
	void *tr, *old_tr;

	BUG_ON(!tfm_ctx);

	/*
	 * Extract cipher and auth keys.
	 */
	if (crypto_authenc_extractkeys(&keys, key, keylen)) {
		pr_warn("%px: Unable to extract keys.\n", tfm_ctx);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		crypto_aead_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
#endif
		aead_stats->invalid_key++;
		tfm_stats->invalid_key++;
		return -EINVAL;
	}

	base = &info.base;
	crypto = &info.crypto;

	/*
	 * Fill info object.
	 */
	strscpy(base->alg_name, alg_name, CRYPTO_MAX_ALG_NAME);
	base->cipher.key_data = keys.enckey;
	base->auth.key_data = keys.authkey;
	base->auth.key_len = keys.authkeylen;

	/*
	 * Last 4 bytes are nonce in case of ctr rfc3686 mode.
	 */
	if (strstr(alg_name, "ctr-rfc3686")) {
		keys.enckeylen = keys.enckeylen - CTR_RFC3686_NONCE_SIZE;
		memcpy(&nonce, (keys.enckey + keys.enckeylen), CTR_RFC3686_NONCE_SIZE);
	}

	base->cipher.key_len = keys.enckeylen;
	base->nonce = nonce;

	/*
	 * Set callbacks and app data.
	 */
	crypto->enc_cb = eip_crypto_aead_done;
	crypto->enc_err_cb = eip_crypto_aead_err;
	crypto->dec_cb = eip_crypto_aead_done;
	crypto->dec_err_cb = eip_crypto_aead_err;
	crypto->app_data = tfm_ctx;
	crypto->auth_err_cb = NULL;
	crypto->auth_cb = NULL;

	tr = eip_tr_alloc(aead_g.dma_ctx, &info);
	if (!tr) {
		pr_warn("%px: Unable to allocate new TR.\n", tfm_ctx);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		crypto_aead_set_flags(tfm, CRYPTO_TFM_RES_BAD_FLAGS);
#endif
		tfm_stats->tr_alloc_err++;
		aead_stats->tr_alloc_err++;
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
 * eip_crypto_aead_encrypt()
 *	Used to perform AEAD encryption.
 */
static int eip_crypto_aead_encrypt(struct aead_request *req)
{
	struct crypto_aead *base = crypto_aead_reqtfm(req);
	struct eip_crypto_tfm_ctx *tfm_ctx = crypto_aead_ctx(base);
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *aead_stats = this_cpu_ptr(aead_g.stats);
	uint32_t cryptlen = req->cryptlen;
	struct eip_tr *tr = NULL;
	int ret;

	/*
	 * Check if tfm is active or not.
	 */
	if (!atomic_read(&tfm_ctx->active)) {
		pr_warn("%px: Tfm is not active.\n", tfm_ctx);
		tfm_stats->inactive_tfm++;
		aead_stats->inactive_tfm++;
		return -EPERM;
	}

	/*
	 * We take RCU lock to synchronize encrypt/decrypt and setkey operation.
	 */
	rcu_read_lock();
	tr = rcu_dereference(tfm_ctx->tr);

	/*
	 * Increment usage count for tfm.
	 */
	atomic_inc(&tfm_ctx->usage_count);

	/*
	 * Send for transformation.
	 */
	ret = eip_tr_aead_encauth(tr, req);
	if (ret) {
		atomic_dec(&tfm_ctx->usage_count);
		tfm_stats->tx_failures++;
		aead_stats->tx_failures++;
		rcu_read_unlock();
		return ret;
	}

	/*
	 * Update stats.
	 */
	tfm_stats->tx_reqs++;
	aead_stats->tx_reqs++;
	tfm_stats->tx_bytes += cryptlen;
	aead_stats->tx_bytes += cryptlen;

	rcu_read_unlock();

	return -EINPROGRESS;
}

/*
 * eip_crypto_aead_decrypt()
 *	Used to perform AEAD decryption.
 */
static int eip_crypto_aead_decrypt(struct aead_request *req)
{
	struct eip_crypto_tfm_ctx *tfm_ctx = crypto_aead_ctx(crypto_aead_reqtfm(req));
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *aead_stats = this_cpu_ptr(aead_g.stats);
	uint32_t cryptlen = req->cryptlen;
	struct eip_tr *tr = NULL;
	int ret;

	/*
	 * Check if tfm is active or not.
	 */
	if (!atomic_read(&tfm_ctx->active)) {
		pr_warn("%px: Tfm is not active.\n", tfm_ctx);
		tfm_stats->inactive_tfm++;
		aead_stats->inactive_tfm++;
		return -EPERM;
	}

	/*
	 * We take RCU lock to synchronize encrypt/decrypt and setkey operation.
	 */
	rcu_read_lock();
	tr = rcu_dereference(tfm_ctx->tr);

	/*
	 * Increment usage count for tfm.
	 */
	atomic_inc(&tfm_ctx->usage_count);

	/*
	 * Send for transformation.
	 */
	ret = eip_tr_aead_authdec(tr, req);
	if (ret) {
		atomic_dec(&tfm_ctx->usage_count);
		tfm_stats->tx_failures++;
		aead_stats->tx_failures++;
		rcu_read_unlock();
		return ret;
	}

	/*
	 * Update stats.
	 */
	tfm_stats->tx_reqs++;
	aead_stats->tx_reqs++;
	tfm_stats->tx_bytes += cryptlen;
	aead_stats->tx_bytes += cryptlen;

	rcu_read_unlock();

	return -EINPROGRESS;
}

/*
 * eip_crypto_aead_init()
 *	Initialize aead cipher.
 */
int eip_crypto_aead_init(struct device_node *node)
{
	struct aead_alg *algo = eip_crypto_aead_algs;
	struct dentry *parent;
	int i;

	/*
	 * Allocate DMA context.
	 */
	aead_g.dma_ctx = eip_ctx_alloc(EIP_SVC_AEAD, &parent);
	if (!aead_g.dma_ctx) {
		pr_warn("%px: Unable to allocate DMA context AEAD.\n", &aead_g);
		return -EINVAL;
	}

	/*
	 * Create debugfs entry for aead service.
	 */
	aead_g.root = debugfs_create_dir("aead", parent);
	if (!aead_g.root) {
		pr_warn("%px: Unable to create debug entry for aead.\n", &aead_g);
		goto free_ctx;
	}

	/*
	 * Create a file for stats.
	 */
	if (!debugfs_create_file("stats", S_IRUGO, aead_g.root, NULL, &file_ops)) {
		pr_warn("%px: Unable to create file for aead stats.\n", &aead_g);
		goto free_debugfs;
	}

	INIT_LIST_HEAD(&aead_g.tfms.head);
	init_completion(&aead_g.complete);
	rwlock_init(&aead_g.tfms.lock);

	/*
	 * Allocate memory for AEAD global stats object.
	 */
	aead_g.stats = alloc_percpu_gfp(struct eip_crypto_tfm_stats, GFP_KERNEL | __GFP_ZERO);
	if (!aead_g.stats) {
		pr_err("%px: Unable to allocate memory for aead stats.\n", &aead_g);
		goto free_debugfs;
	}

	/*
	 * Register aead templates with matching algo name with linux.
	 */
	for (i = 0; i < ARRAY_SIZE(eip_crypto_aead_algs); i++, algo++) {
		const char *alg_name = algo->base.cra_driver_name;
		if (!eip_ctx_algo_supported(aead_g.dma_ctx, alg_name)) {
			pr_info("node: %px alg_name: %s.\n", node, alg_name);
			continue;
		}

		if (crypto_register_aead(algo)) {
			pr_warn("%px: AEAD registration failed(%s)\n", node, alg_name);
			algo->base.cra_flags = CRYPTO_ALG_DEAD;
			continue;
		}

		pr_info("%px: Template registered: %s\n", node, alg_name);
	}

	return 0;

free_debugfs:
	debugfs_remove_recursive(aead_g.root);
free_ctx:
	eip_ctx_free(aead_g.dma_ctx);
	return -ENOMEM;
}

/*
 * eip_crypto_aead_exit()
 *	De-Initializes aead cipher.
 */
void eip_crypto_aead_exit(struct device_node *node)
{
	struct aead_alg *algo = eip_crypto_aead_algs;
	int i;

	/*
	 * Unregister template with linux.
	 */
	for (i = 0; i < ARRAY_SIZE(eip_crypto_aead_algs); i++, algo++) {
		const char *alg_name = algo->base.cra_driver_name;
		if (!eip_ctx_algo_supported(aead_g.dma_ctx, alg_name) ||
				(algo->base.cra_flags & CRYPTO_ALG_DEAD)) {
			pr_info("node: %px alg_name: %s.\n", node, alg_name);
			continue;
		}

		crypto_unregister_aead(algo);
		pr_info("%px: AEAD un-registration success. Algo: %s.\n", node, alg_name);
	}

	/*
	 * Remove debugfs stats entry and per cpu stats
	 */
	debugfs_remove_recursive(aead_g.root);

	BUG_ON(!aead_g.stats);
	free_percpu(aead_g.stats);

	/*
	 * Free the dma context.
	 */
	eip_ctx_free(aead_g.dma_ctx);
	memset(&aead_g, 0, sizeof(aead_g));
}
