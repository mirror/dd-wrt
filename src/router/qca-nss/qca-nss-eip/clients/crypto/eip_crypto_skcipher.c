/*
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <crypto/ctr.h>
#include <crypto/aes.h>
#include <crypto/des.h>
#include <crypto/algapi.h>
#include <crypto/internal/skcipher.h>

#include "eip_crypto.h"

/*
 * Global SKCipher context.
 */
static struct eip_crypto skcipher = {0};

/*
 * Function prototypes.
 */
static int eip_crypto_skcipher_setkey(struct crypto_skcipher *tfm, const u8 *key, unsigned int keylen);
static void eip_crypto_skcipher_tfm_exit(struct crypto_skcipher *sk_tfm);
static int eip_crypto_skcipher_tfm_init(struct crypto_skcipher *sk_tfm);
static int eip_crypto_skcipher_encrypt(struct skcipher_request *req);
static int eip_crypto_skcipher_decrypt(struct skcipher_request *req);

/*
 * eip_crypto_skcipher_algo
 *	Template for skcipher algorithms.
 */
static struct skcipher_alg eip_crypto_skcipher_algo[] = {
	{
		/*
		 * aes(cbc)
		 */
		.base = {
			.cra_name		= "cbc(aes)",
			.cra_driver_name	= "eip-aes-cbc",
			.cra_priority 		= 10000,
			.cra_flags 		= CRYPTO_ALG_ASYNC,
			.cra_blocksize 		= AES_BLOCK_SIZE,
			.cra_ctxsize		= sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask 		= 0,
			.cra_module		= THIS_MODULE,
		},

		.init		= eip_crypto_skcipher_tfm_init,
		.exit 		= eip_crypto_skcipher_tfm_exit,
		.ivsize 	= AES_BLOCK_SIZE,
		.min_keysize 	= AES_MIN_KEY_SIZE,
		.max_keysize 	= AES_MAX_KEY_SIZE,
		.setkey 	= eip_crypto_skcipher_setkey,
		.encrypt 	= eip_crypto_skcipher_encrypt,
		.decrypt 	= eip_crypto_skcipher_decrypt,
	},
	{
		/*
		 * aes(ctr(rfc3686))
		 */
		.base = {
			.cra_name		= "rfc3686(ctr(aes))",
			.cra_driver_name 	= "eip-aes-ctr-rfc3686",
			.cra_priority 		= 10000,
			.cra_flags 		= CRYPTO_ALG_ASYNC,
			.cra_blocksize 		= AES_BLOCK_SIZE,
			.cra_ctxsize 		= sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask 		= 0,
			.cra_module 		= THIS_MODULE,
		},

		.init 		= eip_crypto_skcipher_tfm_init,
		.exit 		= eip_crypto_skcipher_tfm_exit,
		.ivsize 	= CTR_RFC3686_IV_SIZE,
		.min_keysize 	= AES_MIN_KEY_SIZE + CTR_RFC3686_NONCE_SIZE,
		.max_keysize 	= AES_MAX_KEY_SIZE + CTR_RFC3686_NONCE_SIZE,
		.setkey 	= eip_crypto_skcipher_setkey,
		.encrypt 	= eip_crypto_skcipher_encrypt,
		.decrypt 	= eip_crypto_skcipher_decrypt,
	},
	{
		/*
		 * aes(ecb)
		 */
		.base = {
			.cra_name		= "ecb(aes)",
			.cra_driver_name 	= "eip-aes-ecb",
			.cra_priority 		= 10000,
			.cra_flags 		= CRYPTO_ALG_ASYNC,
			.cra_blocksize 		= AES_BLOCK_SIZE,
			.cra_ctxsize 		= sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask 		= 0,
			.cra_module 		= THIS_MODULE,
		},

		.init 		= eip_crypto_skcipher_tfm_init,
		.exit 		= eip_crypto_skcipher_tfm_exit,
		.ivsize 	= 0,
		.min_keysize 	= AES_MIN_KEY_SIZE,
		.max_keysize 	= AES_MAX_KEY_SIZE,
		.setkey 	= eip_crypto_skcipher_setkey,
		.encrypt 	= eip_crypto_skcipher_encrypt,
		.decrypt 	= eip_crypto_skcipher_decrypt,
	},
	{
		/*
		 * 3des(cbc)
		 */
		.base = {
			.cra_name		= "cbc(des3_ede)",
			.cra_driver_name 	= "eip-des3_ede-cbc",
			.cra_priority 		= 10000,
			.cra_flags 		= CRYPTO_ALG_ASYNC,
			.cra_blocksize 		= DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize 		= sizeof(struct eip_crypto_tfm_ctx),
			.cra_alignmask 		= 0,
			.cra_module 		= THIS_MODULE,
		},

		.init 		= eip_crypto_skcipher_tfm_init,
		.exit 		= eip_crypto_skcipher_tfm_exit,
		.ivsize 	= DES3_EDE_BLOCK_SIZE,
		.min_keysize 	= DES3_EDE_KEY_SIZE,
		.max_keysize 	= DES3_EDE_KEY_SIZE,
		.setkey 	= eip_crypto_skcipher_setkey,
		.encrypt 	= eip_crypto_skcipher_encrypt,
		.decrypt 	= eip_crypto_skcipher_decrypt,
	},
};

/*
 * eip_crypto_skcipher_tfm_init()
 *	TFM initialization.
 */
static int eip_crypto_skcipher_tfm_init(struct crypto_skcipher *sk_tfm)
{
	struct crypto_tfm *base = crypto_skcipher_tfm(sk_tfm);
	struct eip_crypto_tfm_ctx *tfm_ctx = crypto_tfm_ctx(base);

	BUG_ON(!tfm_ctx);

	/*
	 * Initialize TFM.
	 */
	memset(tfm_ctx, 0, sizeof(*tfm_ctx));
	init_completion(&tfm_ctx->complete);
	INIT_LIST_HEAD(&tfm_ctx->entry);

	/*
	 * Decrement:eip_crypto_skcipher_tfm_exit()
	 */
	atomic_inc(&tfm_ctx->usage_count);

	/*
	 * Allocate memory for per cpu stats.
	 */
	tfm_ctx->stats = alloc_percpu_gfp(struct eip_crypto_tfm_stats, GFP_KERNEL | __GFP_ZERO);
	if (!tfm_ctx->stats) {
		pr_err("%px: SKCipher unable to allocate memory for tfm stats.\n", tfm_ctx);
		return -ENOMEM;
	}

	/*
	 * Add TFM to list of TFMs.
	 */
	eip_crypto_add_tfmctx(&skcipher, tfm_ctx);

	/*
	 * set this tfm reqsize to the transform specific structure
	 */
	crypto_skcipher_set_reqsize(sk_tfm, crypto_skcipher_ivsize(sk_tfm));

	return 0;
}

/*
 * eip_crypto_skcipher_tfm_exit()
 *	TFM de-initialization.
 */
static void eip_crypto_skcipher_tfm_exit(struct crypto_skcipher *sk_tfm)
{
	struct crypto_tfm *base = crypto_skcipher_tfm(sk_tfm);
	struct eip_crypto_tfm_ctx *tfm_ctx = crypto_tfm_ctx(base);

	BUG_ON(!tfm_ctx);

	atomic_set(&tfm_ctx->active, 0);

	/*
	 * We wait for outstanding packets to get processed.
	 * This is possible if there are inflight packets
	 * while the unregister is called.
	 */
	if (!atomic_sub_and_test(1, &tfm_ctx->usage_count)) {
		pr_debug("%px: SKCipher waiting for usage count(%d) to become 0.\n",
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
	eip_crypto_del_tfmctx(&skcipher, tfm_ctx);

	/*
	 * We signal when all the TFMs have been destroyed.
	 */
	if (!list_empty(&skcipher.tfms.head)) {
		complete(&skcipher.complete);
	}
}

/*
 * eip_crypto_skcipher_stats_read()
 *	Read SKCIPHER statistics.
 */
static ssize_t eip_crypto_skcipher_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	uint64_t tfm_count = atomic_read(&skcipher.tfms.tfm_count);
	struct eip_crypto_tfm_stats stats;
	struct eip_crypto_tfm_ctx *tfm;
	uint64_t tfm_usage_count = 0;
	ssize_t bytes_read = 0;
	size_t size_wr = 0;
	int count = 0;
	char *buf;

	size_t stats_sz = EIP_CRYPTO_STATS_STR_LEN * (sizeof(struct eip_crypto_tfm_stats)/sizeof(uint64_t) + 5);
	stats_sz *= (tfm_count + 1); /* Stats for all TFMs + For combined tfm stats */

	buf = kzalloc(stats_sz, GFP_KERNEL);
	if (unlikely(buf == NULL)) {
		pr_warn("%px: Unable to allocate memory for stats.\n", fp);
		return -ENOMEM;
	}

	/*
	 * Get summarized stats for global SKCipher object.
	 */
	eip_crypto_get_summary_stats(skcipher.stats, &stats);

	/*
	 * Print stats.
	 */
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "\nSKCipher service stats start:\n");
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
	size_wr += scnprintf(buf + size_wr, stats_sz - size_wr, "SKCipher service stats end.\n");

	/*
	 * Per TFM stats.
	 */
	read_lock_bh(&skcipher.tfms.lock);
	list_for_each_entry(tfm, &skcipher.tfms.head, entry) {
		/*
		 * Get summarized stats for global SKCIHPER object.
		 */
		eip_crypto_get_summary_stats(skcipher.stats, &stats);
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

	read_unlock_bh(&skcipher.tfms.lock);
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, buf, size_wr);
	kfree(buf);
	return bytes_read;
}

/*
 * File operation structure instance
 */
static const struct file_operations file_ops = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = eip_crypto_skcipher_stats_read
};

/*
 * eip_crypto_skcipher_copy_reverse()
 *	Reverse copy
 *
 * We need to copy the IV which is typically part of the last segment.
 * However, we don't know whether the source is linear or split for the
 * length of the IV. Hence, we copy in reverse direction and stop where
 * the length ends. This way we can resume from the last location and
 * continue till the IV length worth of data is copied.
 */
static inline uint8_t *eip_crypto_skcipher_copy_reverse(uint8_t *dest, uint8_t *src, uint16_t len)
{
	/*
	 * If, length is zero then it will simply return without copying anything
	 */
	while (len--) {
		*(--dest) = *(--src);
	}

	return dest;
}

/*
 * eip_crypto_skcipher_copy_iv()
 *	Copy IV
 */
void eip_crypto_skcipher_copy_iv(struct scatterlist *sg, uint8_t *iv, uint8_t iv_len)
{
	struct scatterlist *last_sg;
	uint8_t *iv_end = iv + iv_len;
	int nsegs = sg_nents(sg);
	uint8_t *sg_end;
	int copy_len = 0;

	/*
	 * Invalid last SG
	 */
	last_sg = sg_last(sg, nsegs);
	if (!last_sg) {
		return;
	}

	/*
	 * Walk the SG list in reverse direction
	 */
	for ( ; nsegs && iv_len; nsegs--, iv_len -= copy_len, last_sg--) {
		if (sg_is_chain(last_sg)) {
			last_sg = sg_chain_ptr(last_sg);
		}

		copy_len = last_sg->length >= iv_len ? iv_len : last_sg->length;
		sg_end = sg_virt(last_sg) + last_sg->length;

		pr_debug("%p: copy iv=%p, iv_len=%u, last_sg=%p", &skcipher, iv_end, iv_len, last_sg);
		iv_end = eip_crypto_skcipher_copy_reverse(iv_end, sg_end, copy_len);
	}
}

/*
 * eip_crypto_skcipher_done()
 *	SKCIPHER callback function. Called after transformation is done.
 */
static void eip_crypto_skcipher_enc_done(void *app_data, eip_req_t eip_req)
{
	struct eip_crypto_tfm_ctx *tfm_ctx = (struct eip_crypto_tfm_ctx *)app_data;
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *sk_stats = this_cpu_ptr(skcipher.stats);
	struct skcipher_request *req = eip_req2skcipher_request(eip_req);
	struct crypto_skcipher *base = crypto_skcipher_reqtfm(req);

	/*
	 * Copy the IV.
	 */
	eip_crypto_skcipher_copy_iv(req->dst, req->iv, crypto_skcipher_ivsize(base));

	/*
	 * Increment stats.
	 */
	tfm_stats->rx_reqs++;
	sk_stats->rx_reqs++;
	tfm_stats->rx_bytes += req->cryptlen;
	sk_stats->rx_bytes += req->cryptlen;

	skcipher_request_complete(req, 0);

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
 * eip_crypto_skcipher_err_done()
 *	SKCIPHER decryption error callback function. Called after transformation is done.
 */
static void eip_crypto_skcipher_enc_err(void *app_data, eip_req_t eip_req, int err)
{
	struct eip_crypto_tfm_ctx *tfm_ctx = (struct eip_crypto_tfm_ctx *)app_data;
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *sk_stats = this_cpu_ptr(skcipher.stats);

	/*
	 * Increment hardware stats.
	 */
	if (err) {
		tfm_stats->hw_err++;
		sk_stats->hw_err++;
	}

	eip_crypto_skcipher_enc_done(app_data, eip_req);
}

/*
 * eip_crypto_skcipher_dec_done()
 *	SKCIPHER decryption callback function. Called after transformation is done.
 */
static void eip_crypto_skcipher_dec_done(void *app_data, eip_req_t eip_req)
{
	struct eip_crypto_tfm_ctx *tfm_ctx = (struct eip_crypto_tfm_ctx *)app_data;
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *sk_stats = this_cpu_ptr(skcipher.stats);
	struct skcipher_request *req = eip_req2skcipher_request(eip_req);
	struct crypto_skcipher *base = crypto_skcipher_reqtfm(req);
	uint8_t *rctx_iv = skcipher_request_ctx(req);

	/*
	 * Copy IV.
	 */
	memcpy(req->iv, rctx_iv, crypto_skcipher_ivsize(base));

	/*
	 * Increment stats.
	 */
	tfm_stats->rx_reqs++;
	sk_stats->rx_reqs++;
	tfm_stats->rx_bytes += req->cryptlen;
	sk_stats->rx_bytes += req->cryptlen;

	skcipher_request_complete(req, 0);

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
 * eip_crypto_err_skcipher_dec_done()
 *	SKCIPHER decryption error callback function. Called after transformation error.
 */
static void eip_crypto_skcipher_dec_err(void *app_data, eip_req_t eip_req, int err)
{
	struct eip_crypto_tfm_ctx *tfm_ctx = (struct eip_crypto_tfm_ctx *)app_data;
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *sk_stats = this_cpu_ptr(skcipher.stats);

	/*
	 * Increment hardware stats.
	 */
	if (err) {
		tfm_stats->hw_err++;
		sk_stats->hw_err++;
	}

	eip_crypto_skcipher_dec_done(app_data, eip_req);
}

/*
 * eip_crypto_skcipher_setkey()
 *	Sets key for transformation with no authentication.
 */
static int eip_crypto_skcipher_setkey(struct crypto_skcipher *tfm, const u8 *key, unsigned int keylen)
{
	struct eip_crypto_tfm_ctx *tfm_ctx = crypto_tfm_ctx(crypto_skcipher_tfm(tfm));
	char *alg_name = (char *)crypto_tfm_alg_driver_name(crypto_skcipher_tfm(tfm));
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *sk_stats = this_cpu_ptr(skcipher.stats);
	struct eip_tr_info_crypto *crypto;
	struct eip_tr_info info = {0};
	struct eip_tr_base *base;
	uint32_t nonce = 0;
	void *tr, *old_tr;

	BUG_ON(!tfm_ctx);

	base = &info.base;
	crypto = &info.crypto;

	/*
	 * Fill algo info data
	 */
	strscpy(base->alg_name, alg_name, CRYPTO_MAX_ALG_NAME);
	base->cipher.key_data = key;

	/*
	 * In case of ctr-rfc3686 mode nonce is present as part of key.
	 * We copy last 4 bytes of key.
	 */
	if (strstr(alg_name, "ctr-rfc3686")) {
		keylen = keylen - CTR_RFC3686_NONCE_SIZE;
		memcpy(&nonce, key + keylen, CTR_RFC3686_NONCE_SIZE);
	}

	base->cipher.key_len = keylen;
	base->nonce = nonce;

	/*
	 * Set callbacks and app data.
	 */
	crypto->enc_cb = eip_crypto_skcipher_enc_done;
	crypto->enc_err_cb = eip_crypto_skcipher_enc_err;
	crypto->dec_cb = eip_crypto_skcipher_dec_done;
	crypto->dec_err_cb = eip_crypto_skcipher_dec_err;
	crypto->app_data = tfm_ctx;
	crypto->auth_cb = NULL;

	/*
	 * Allocate a new TR.
	 */
	tr = eip_tr_alloc(skcipher.dma_ctx, &info);
	if (!tr) {
		pr_warn("%px: Unable to allocate new TR.\n", tfm_ctx);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		crypto_skcipher_set_flags(tfm, CRYPTO_TFM_RES_BAD_FLAGS);
#endif
		tfm_stats->tr_alloc_err++;
		sk_stats->tr_alloc_err++;
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
 * eip_crypto_skcipher_encrypt()
 *	Used to perform SKCIPHER encryption.
 */
static int eip_crypto_skcipher_encrypt(struct skcipher_request *req)
{
	struct eip_crypto_tfm_ctx *tfm_ctx = crypto_skcipher_ctx(crypto_skcipher_reqtfm(req));
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *sk_stats = this_cpu_ptr(skcipher.stats);
	uint16_t cryptlen = req->cryptlen;
	struct eip_tr *tr = NULL;
	int ret;

	/*
	 * Check if tfm is active or not.
	 */
	if (!atomic_read(&tfm_ctx->active)) {
		pr_warn("%px: Tfm is not active.\n", tfm_ctx);
		tfm_stats->inactive_tfm++;
		sk_stats->inactive_tfm++;
		return -EPERM;
	}

	/*
	 * We only support (2^16 - 1) length
	 */
	if (req->cryptlen > U16_MAX) {
		pr_warn("%px: Invalid length %u, not supported.\n", tfm_ctx, req->cryptlen);
		tfm_stats->invalid_len++;
		sk_stats->invalid_len++;
		return -EFBIG;
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
	ret = eip_tr_skcipher_enc(tr, req);
	if (ret) {
		rcu_read_unlock();
		pr_debug("%px: Encryption error: %d\n", tfm_ctx, ret);
		atomic_dec(&tfm_ctx->usage_count);
		tfm_stats->tx_failures++;
		sk_stats->tx_failures++;
		return ret;
	}

	/*
	 * Update stats.
	 */
	tfm_stats->tx_reqs++;
	sk_stats->tx_reqs++;
	tfm_stats->tx_bytes += cryptlen;
	sk_stats->tx_bytes += cryptlen;

	rcu_read_unlock();

	return -EINPROGRESS;
}

/*
 * eip_crypto_skcipher_decrypt()
 *	Used to perform SKCIPHER decryption.
 */
static int eip_crypto_skcipher_decrypt(struct skcipher_request *req)
{
	struct eip_crypto_tfm_ctx *tfm_ctx = crypto_skcipher_ctx(crypto_skcipher_reqtfm(req));
	struct eip_crypto_tfm_stats *tfm_stats = this_cpu_ptr(tfm_ctx->stats);
	struct eip_crypto_tfm_stats *sk_stats = this_cpu_ptr(skcipher.stats);
	uint8_t *rctx_iv = skcipher_request_ctx(req);
	uint16_t cryptlen = req->cryptlen;
	struct eip_tr *tr = NULL;
	int ret;

	/*
	 * Check if tfm is active or not.
	 */
	if (!atomic_read(&tfm_ctx->active)) {
		pr_warn("%px: Tfm is not active.\n", tfm_ctx);
		tfm_stats->inactive_tfm++;
		sk_stats->inactive_tfm++;
		return -EPERM;
	}

	/*
	 * We only support (2^16 - 1) length
	 */
	if (req->cryptlen > U16_MAX) {
		pr_warn("%px: Invalid length %u, not supported.\n", tfm_ctx, req->cryptlen);
		tfm_stats->invalid_len++;
		sk_stats->invalid_len++;
		return -EFBIG;
	}

	/*
	 * We take RCU lock to synchronize encrypt/decrypt and setkey operation.
	 */
	rcu_read_lock();
	tr = rcu_dereference(tfm_ctx->tr);

	/*
	 * Copy IV in rctx_iv. We copy it back in req->iv when decryption is done.
	 */
	eip_crypto_skcipher_copy_iv(req->src, rctx_iv, crypto_skcipher_ivsize(crypto_skcipher_reqtfm(req)));

	/*
	 * Increment usage count for tfm.
	 */
	atomic_inc(&tfm_ctx->usage_count);

	/*
	 * Send for transformation.
	 */
	ret = eip_tr_skcipher_dec(tr, req);
	if (ret) {
		rcu_read_unlock();
		atomic_dec(&tfm_ctx->usage_count);
		pr_debug("%px: Decryption error: %d.\n", tfm_ctx, ret);
		tfm_stats->tx_failures++;
		sk_stats->tx_failures++;
		return ret;
	}

	/*
	 * Update stats.
	 */
	tfm_stats->tx_reqs++;
	sk_stats->tx_reqs++;
	tfm_stats->tx_bytes += cryptlen;
	sk_stats->tx_bytes += cryptlen;

	rcu_read_unlock();

	return -EINPROGRESS;
}

/*
 * eip_crypto_skcipher_init()
 *	Initialize skcipher.
 */
int eip_crypto_skcipher_init(struct device_node *node)
{
	struct skcipher_alg *algo = eip_crypto_skcipher_algo;
	struct dentry *parent;
	int i = 0;

	/*
	 * Allocate DMA context.
	 */
	skcipher.dma_ctx = eip_ctx_alloc(EIP_SVC_SKCIPHER, &parent);
	if (!skcipher.dma_ctx) {
		pr_err("%px: Unable to allocate DMA context for SKCIPHER.\n", &skcipher);
		return -EINVAL;
	}

	/*
	 * Create debugfs entry for skcipher service.
	 */
	skcipher.root = debugfs_create_dir("skcipher", parent);
	if (!skcipher.root) {
		pr_err("%px: Unable to create debug entry for skcipher.\n", &skcipher);
		goto free_ctx;
	}

	/*
	 * Create a file for stats.
	 */
	if (!debugfs_create_file("stats", S_IRUGO, skcipher.root, NULL, &file_ops)) {
		pr_err("%px: Unable to create stats debug entry.\n", &skcipher);
		goto free_debugfs;
	}

	INIT_LIST_HEAD(&skcipher.tfms.head);
	init_completion(&skcipher.complete);
	rwlock_init(&skcipher.tfms.lock);

	/*
	 * Allocate memory for SKCipher global stats object.
	 */
	skcipher.stats = alloc_percpu_gfp(struct eip_crypto_tfm_stats, GFP_KERNEL | __GFP_ZERO);
	if (!skcipher.stats) {
		pr_err("%px: Unable to allocate memory for skcipher stats.\n", &skcipher);
		goto free_debugfs;
	}

	/*
	 * Register skcipher templates with matching algo name with linux.
	 */
	for (i = 0; i < ARRAY_SIZE(eip_crypto_skcipher_algo); i++, algo++) {
		const char *alg_name = algo->base.cra_driver_name;

		if (!eip_ctx_algo_supported(skcipher.dma_ctx, alg_name)) {
			pr_info("node: %px alg_name: %s.\n", node, alg_name);
			continue;
		}

		if (crypto_register_skcipher(algo)) {
			pr_warn("%px: SKCIPHER registration failed(%s)\n", node, alg_name);
			algo->base.cra_flags = CRYPTO_ALG_DEAD;
			continue;
		}

		pr_info("%px: Template registered: %s\n", node, alg_name);
	}

	return 0;
free_debugfs:
	debugfs_remove_recursive(skcipher.root);
free_ctx:
	eip_ctx_free(skcipher.dma_ctx);
	return -ENOMEM;
}

/*
 * eip_crypto_skcipher_exit()
 *	De-Initializes skcipher cipher.
 */
void eip_crypto_skcipher_exit(struct device_node *node)
{
	struct skcipher_alg *algo = eip_crypto_skcipher_algo;
	int i;

	/*
	 * Unregister template with linux.
	 */
	for (i = 0; i < ARRAY_SIZE(eip_crypto_skcipher_algo); i++, algo++) {
		const char *alg_name = algo->base.cra_driver_name;
		if (!eip_ctx_algo_supported(skcipher.dma_ctx, alg_name) ||
				(algo->base.cra_flags & CRYPTO_ALG_DEAD)) {
			pr_info("node: %px alg_name: %s.\n", node, alg_name);
			continue;
		}

		crypto_unregister_skcipher(algo);
		pr_info("%px: SKCIPHER un-registration success. Algo: %s.\n", node, alg_name);
	}

	/*
	 * Remove debug entry & stats
	 */
	debugfs_remove_recursive(skcipher.root);

	BUG_ON(!skcipher.stats);
	free_percpu(skcipher.stats);

	/*
	 * Free the dma context.
	 */
	eip_ctx_free(skcipher.dma_ctx);
	memset(&skcipher, 0, sizeof(skcipher));
}
