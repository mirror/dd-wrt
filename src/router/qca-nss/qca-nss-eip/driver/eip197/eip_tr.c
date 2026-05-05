/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/dma-mapping.h>
#include <linux/crypto.h>
#include <linux/cache.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <crypto/aes.h>

#include "eip_priv.h"

/* eip_set_disable_hw_ctx()
 *	Set disable bit in HW ctx
 */
static inline void eip_tr_disable_hw(struct eip_tr *tr)
{
	uint32_t *disable_word;

	switch (tr->tr_addr_type & EIP_REC_TYPE_MASK) {
	case EIP_HW_CTX_TYPE_LARGE:
		disable_word = &tr->hw_words[65];
		break;
	case EIP_HW_CTX_TYPE_SMALL:
		disable_word = &tr->hw_words[49];
		break;
	default:
		pr_err("%px : Invalid Record type\n", tr);
		return;
	}

	 *disable_word |= 0x1;
}

/*
 * eip_tr_append_stats()
 */
static void eip_tr_append_stats(struct eip_tr_stats *dst, const struct eip_tr_stats *src)
{
	int words = (sizeof(struct eip_tr_stats) / sizeof(uint64_t));
	uint64_t *dst_ptr = (uint64_t *)dst;
	uint64_t *src_ptr = (uint64_t *)src;
	int i;

	/*
	 * All statistics are 64bit. So we can just iterate by words.
	 */
	for (i = 0; i < words; i++, dst_ptr++, src_ptr++)
		*dst_ptr += *src_ptr;
}

/*
 * eip_tr_dummy_cb()
 *	Dummy callback for inactive TR.
 */
static void eip_tr_dummy_cb(void *app_data, eip_req_t req)
{
	/*
	 * This will only get called for SKB. As crypto client is ensuring TR free is issued only after
	 * all requests are processed.
	 */
	pr_debug("%px: TR dummy cb called\n", req);
	consume_skb(eip_req2skb(req));
	return;
}

/*
 * eip_tr_dummy_err()
 *	Dummy callback for inactive TR.
 */
static void eip_tr_dummy_err(void *app_data, eip_req_t req, int err)
{
	pr_debug("%px: TR dummy cb called with err(%u)\n", req, err);
	consume_skb(eip_req2skb(req));
	return;
}

/*
 * eip_tr_inval_done()
 *	Invalidation completion.
 */
static void eip_tr_inval_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	struct eip_pdev *ep;
	struct eip_ctx *ctx;

	ctx = tr->ctx;
	ep = ctx->ep;

	/*
	 * free the memory associated with the operation.
	 */
	kmem_cache_free(ctx->sw_cache, sw);
	eip_tr_deref(tr);
	pr_debug("%px: TR disabled successfully\n", tr);
}

/*
 * eip_tr_inval_n_free()
 *	Invalidation completion and free the TR.
 */
static void eip_tr_inval_n_free(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	struct eip_pdev *ep;
	struct eip_ctx *ctx;

	ctx = tr->ctx;
	ep = ctx->ep;

	/*
	 * free the memory associated with the operation.
	 */
	kmem_cache_free(ctx->sw_cache, sw);

	/*
	 * Free TR and all dependent resource.
	 * Reference: eip_tr_alloc()
	 */
	eip_ctx_del_tr(ctx, tr);
	eip_ctx_deref(ctx);
	tr->ctx = NULL;

	free_percpu(tr->stats_pcpu);
	kmem_cache_free(ep->tr_cache, tr);
	pr_debug("TR freed successfully\n");
}

/*
 * eip_tr_inval_err()
 *	Invalidation completion with error
 */
static void eip_tr_inval_err(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw, uint16_t cle_err, uint16_t tr_err)
{
	if (kref_read(&tr->ref)) {
		pr_err("%px: TR Invalidation for disable failed with cle_err(%u) tr_err(%u)\n", tr, cle_err, tr_err);
		eip_tr_inval_done(tr, hw, sw);
		return;
	}

	pr_err("%px: TR Invalidation failed with cle_err(%u) tr_err(%u)\n", tr, cle_err, tr_err);
	eip_tr_inval_n_free(tr, hw, sw);
}

/*
 * eip_tr_inval()
 */
static void eip_tr_inval(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct eip_sw_desc *sw;
	uint32_t tk_hdr;
	struct scatterlist sg;
	struct eip_dma *dma;
	struct eip_ctx *ctx;
	struct eip_tr *tr;

	tr = container_of(dwork, struct eip_tr, inval_work);
	ctx = tr->ctx;

	/*
	 * Initialize scatterlist with dummy data address.
	 * Although Invalidation does not use any data buffer. We do it for DMA.
	 */
	sg_init_one(&sg, &tr->inval_dummy_buf, 0);

	/*
	 * Initialize command token header.
	 */
	tk_hdr = EIP_HW_TOKEN_HDR_CMN;
	tk_hdr |= EIP_HW_TOKEN_HDR_REUSE_CTX;

	/*
	 * Allocate SW descriptor.
	 */
	sw = kmem_cache_alloc(ctx->sw_cache, GFP_NOWAIT | __GFP_NOWARN);
	if (!sw) {
		pr_warn("%px: Failed to allocate SW descriptor; Rescheduling..\n", tr);
		goto reschedule;
	}

	/*
	 * Fill software descriptor.
	 * We don't take TR reference as it is already reached zero and delinked from DB.
	 */
	sw->tr = tr;
	sw->tk = NULL;
	sw->err_comp = eip_tr_inval_err;
	sw->tk_hdr = tk_hdr;
	sw->tr_addr_type = tr->tr_addr_type;
	sw->tk_addr = 0;
	sw->tk_words = 0;
	sw->hw_svc = EIP_HW_CMD_HWSERVICE_INV_TRC;
	sw->req = NULL;

	if (kref_read(&tr->ref))
		sw->comp = eip_tr_inval_done;
	else
		sw->comp = eip_tr_inval_n_free;

	/*
	 * Schedule the transformation.
	 * There is potential chance that DMA fails to schedule this request during peak
	 * traffic. In that case we need to reschedule the invalidation.
	 */
	dma = &ctx->ep->la[smp_processor_id()];

	if (eip_dma_tx_sg(dma, sw, &sg, &sg)) {
		pr_warn("%px: DMA is busy for invalidation schedule\n", tr);

		kmem_cache_free(ctx->sw_cache, sw);
		goto reschedule;
	}

	return;

reschedule:
	/*
	 * Schedule TR invalidation.
	 * Use delayed invalidation for final teardown, otherwise run immediately
	 * for update or retry paths.
	 */
	if (kref_read(&tr->ref))
		schedule_delayed_work(&tr->inval_work, EIP_TR_INVAL_TIMEOUT);
	else
		schedule_delayed_work(&tr->inval_work, 0);
}

/*
 * eip_tr_get_stats()
 *	Update the summary stats.
 */
void eip_tr_get_stats(struct eip_tr *tr, struct eip_tr_stats *stats)
{
	int cpu;

	memset(stats, 0, sizeof(*stats));

	/*
	 * Interate over each cpu statistics and add it into the output stats.
	 */
	for_each_possible_cpu(cpu) {
		const struct eip_tr_stats *sp = per_cpu_ptr(tr->stats_pcpu, cpu);
		eip_tr_append_stats(stats, sp);
	}
}

/*
 * eip_tr_get_hw_stats()
 *	Get the delta TR states from HW
 */
void eip_tr_get_hw_stats(struct eip_tr *tr, struct eip_hw_stats *delta)
{
	uint32_t *words;

	if (!tr || !delta) {
		pr_warn("Failed to get HW stats : Transform Record/eip_stats is invalid\n");
		return;
	}

	words = tr->hw_words;
	eip_dmac_inv_range(words, words + EIP_HW_CTX_SIZE_LARGE_WORDS);

	/* Compute delta packet and bytes counts */
	delta->pkt_cnt = U64_FROM_U32(words[77], words[76])  - tr->tr_stats.pkt_cnt;
	delta->byte_cnt = U64_FROM_U32(words[79], words[78]) - tr->tr_stats.byte_cnt;

	/* Update latest TR counts */
	tr->tr_stats.pkt_cnt += delta->pkt_cnt;
	tr->tr_stats.byte_cnt += delta->byte_cnt;
}
EXPORT_SYMBOL(eip_tr_get_hw_stats);

/*
 * eip_tr_final()
 *	Mark TR completion.
 */
void eip_tr_final(struct kref *kref)
{
	struct eip_tr *tr = container_of(kref, struct eip_tr, ref);

	/*
	 * Schedule invalidation. TR will be freed when Invalidation is done.
	 * Timeout must be enough for Driver to process all the packets seating in hardware queue.
	 *
	 * We make sure no new packet get schedule to EIP in following cases
	 * case 1: For lookaside Tx, Host does not schedule new buffer once TR became inactive (TR deletion has called)
	 * case 2: For inline Rx, Outer EIP flow is deleted once TR became inactive.
	 */
	schedule_delayed_work(&tr->inval_work, EIP_TR_INVAL_TIMEOUT);
}

/*
 * eip_tr_alloc()
 *	Allocate the transform record.
 *
 * This function may sleep and should not be called form atomic context.
 */
struct eip_tr *eip_tr_alloc(struct eip_ctx *ctx, struct eip_tr_info *info)
{
	const struct eip_svc_entry *algo;
	struct eip_pdev *ep = ctx->ep;
	struct eip_tr *tr;

	might_sleep();

	if (ctx->svc >= EIP_SVC_MAX) {
		pr_err("%px: Invalid EIP context\n", ctx);
		return NULL;
	}

	/*
	 * Fetch algorithm entry associated with service.
	 */
	algo = eip_ctx_algo_lookup(ctx, info->base.alg_name);
	if (!algo) {
		pr_err("%px: Invalid algo(%s)\n", ctx, info->base.alg_name);
		return NULL;
	}

	/*
	 * Allocate memory for transform record & HW context words.
	 */
	tr = kmem_cache_alloc(ep->tr_cache, GFP_KERNEL);
	if (!tr) {
		pr_err("%px: Failed to allocate EIP context\n", ctx);
		return NULL;
	}

	memset(tr, 0, EIP_TR_SIZE);

	/*
	 * hw_words must be cacheline aligned.
	 */
	BUG_ON(!IS_ALIGNED((uintptr_t)tr->hw_words, L1_CACHE_BYTES));

	/*
	 * Initialize common information for TR. Some specific information can be initialized
	 * in respective TR init.
	 */
	tr->ctx = eip_ctx_ref(ctx);
	tr->svc = ctx->svc;
	kref_init(&tr->ref);
	INIT_DELAYED_WORK(&tr->inval_work, eip_tr_inval);

	tr->stats_pcpu = alloc_percpu_gfp(struct eip_tr_stats, GFP_KERNEL | __GFP_ZERO);
	if (!tr->stats_pcpu) {
		pr_err("%px: Failed to allocate stats memory for TR\n", tr);
		goto fail_stats;
	}

	/*
	 * Initialize Transform records words based on service type.
	 */
	if (!algo->tr_init(tr, info, algo)) {
		pr_err("%px: TR initialization failed for svc(%u) algo(%s)\n", ctx, ctx->svc, algo->name);
		goto fail;
	}

	/*
	 * Add TR into context list.
	 */
	pr_debug("%px: Allocated transform record for service(%u) algo(%s)\n", tr, ctx->svc, algo->name);
	eip_ctx_add_tr(ctx, tr);

	/*
	 * Context is going to use hybrid ring, so refill it
	 */
	if (tr->svc == EIP_SVC_HYBRID_IPSEC && ep->dma_refill_req) {
		eip_dma_hy_refill_all(ctx);
		ep->dma_refill_req = false;
	}

	atomic_set(&tr->active, 1);
	return tr;

fail:
	free_percpu(tr->stats_pcpu);
fail_stats:
	kmem_cache_free(ep->tr_cache, tr);
	return NULL;
}
EXPORT_SYMBOL(eip_tr_alloc);

/*
 * eip_tr_disable()
 *	disable the Transform record.
 *
 * Disable TR to avoid any Driver to Client communication.
 */
void eip_tr_disable(struct eip_tr *tr)
{
	struct eip_tr_ops *ops;

	/*
	 * Hold a reference so the TR is not freed by final teardown
	 * until the invalidation worker has completed.
	 */
	eip_tr_ref(tr);

	/*
	 * Mark TR as inactive.
	 */
	atomic_set(&tr->active, 0);
	eip_tr_disable_hw(tr);

	/*
	 * Atomically change callback to dummy.
	 */
	switch (tr->svc) {
	case EIP_SVC_SKCIPHER:
	case EIP_SVC_AEAD:
		ops = &tr->crypto.enc;
		xchg(&ops->cb, eip_tr_dummy_cb);
		xchg(&ops->err_cb, eip_tr_dummy_err);

		ops = &tr->crypto.dec;
		xchg(&ops->cb, eip_tr_dummy_cb);
		xchg(&ops->err_cb, eip_tr_dummy_err);
		break;

	case EIP_SVC_AHASH:
		ops = &tr->crypto.auth;
		xchg(&ops->cb, eip_tr_dummy_cb);
		xchg(&ops->err_cb, eip_tr_dummy_err);
		break;

		/*
		 * TODO: Replace EIP_SVC_HYBRID_IPSEC with EIP_SVC_IPSEC.
		 * Use eip_features_t flags to derive remaining IPsec-related information.
		 */
	case EIP_SVC_HYBRID_IPSEC:
		ops = &tr->ipsec.ops;
		xchg(&ops->cb, eip_tr_dummy_cb);
		xchg(&ops->err_cb, eip_tr_dummy_err);
		break;

	default:
		pr_err("%px: Service database not found\n", tr);
		break;
	}

	/*
	 * Disable Record, To avoid any unanticipated record read.
	 */
	eip_dmac_clean_range(tr->hw_words, tr->hw_words + EIP_HW_CTX_SIZE_LARGE_WORDS);
	schedule_delayed_work(&tr->inval_work, 0);
}
EXPORT_SYMBOL(eip_tr_disable);

/*
 * eip_tr_free()
 *	Free the Transform record.
 *
 * Caller need to make sure to not schedule any new tranformation request on this tr.
 */
void eip_tr_free(struct eip_tr *tr)
{
	/*
	 * Disable if not already done.
	 */
	if (atomic_read(&tr->active)) {
		eip_tr_disable(tr);
	}

	/*
	 * Reference: eip_tr_alloc
	 */
	eip_tr_deref(tr);
}
EXPORT_SYMBOL(eip_tr_free);

/*
 * eip_tr_classify_err()
 *     API to classify hw errors to linux errors.
 */
int eip_tr_classify_err(uint16_t cle_err, uint16_t tr_err)
{
	/*
	 * On success we return 0.
	 */
	if (!cle_err && !tr_err) {
		return 0;
	}

	/*
	 * Check for tr_err.
	 */
	switch (tr_err) {
	case EIP_HW_RES_ERROR_E0_NUM:
	case EIP_HW_RES_ERROR_E1_NUM:
	case EIP_HW_RES_ERROR_E3_NUM:
	case EIP_HW_RES_ERROR_E4_NUM:
	case EIP_HW_RES_ERROR_E5_NUM:
	case EIP_HW_RES_ERROR_E6_NUM:
	case EIP_HW_RES_ERROR_E8_NUM:
	case EIP_HW_RES_ERROR_E12_NUM:
		return -EINVAL;
	case EIP_HW_RES_ERROR_E2_NUM:
		return -E2BIG;
	case EIP_HW_RES_ERROR_E7_NUM:
		return -EOVERFLOW;
	case EIP_HW_RES_ERROR_E9_NUM:
	case EIP_HW_RES_ERROR_E11_NUM:
	case EIP_HW_RES_ERROR_E13_NUM:
		return -ECANCELED;
	case EIP_HW_RES_ERROR_E10_NUM:
		return -EILSEQ;
	case EIP_HW_RES_ERROR_E14_NUM:
		return -ETIME;
	}

	/*
	 * Check for cle_err.
	 */
	switch(cle_err) {
	case EIP_HW_RES_CLE_ERROR_E0:
		return 0;
	case EIP_HW_RES_CLE_ERROR_E1:
	case EIP_HW_RES_CLE_ERROR_E2:
		return -EOPNOTSUPP;
	case EIP_HW_RES_CLE_ERROR_E3:
	case EIP_HW_RES_CLE_ERROR_E5:
	case EIP_HW_RES_CLE_ERROR_E6:
	case EIP_HW_RES_CLE_ERROR_E7:
	case EIP_HW_RES_CLE_ERROR_E8:
	case EIP_HW_RES_CLE_ERROR_E9:
	case EIP_HW_RES_CLE_ERROR_E10:
	case EIP_HW_RES_CLE_ERROR_E12:
	case EIP_HW_RES_CLE_ERROR_E13:
	case EIP_HW_RES_CLE_ERROR_E14:
	case EIP_HW_RES_CLE_ERROR_E15:
	case EIP_HW_RES_CLE_ERROR_E16:
	case EIP_HW_RES_CLE_ERROR_E18:
	case EIP_HW_RES_CLE_ERROR_E20:
	case EIP_HW_RES_CLE_ERROR_E21:
	case EIP_HW_RES_CLE_ERROR_E22:
	case EIP_HW_RES_CLE_ERROR_E23:
	case EIP_HW_RES_CLE_ERROR_E24:
	case EIP_HW_RES_CLE_ERROR_E25:
		return -ECANCELED;
	}

	return -EINVAL;
}

/*
 * eip_tr_genkey()
 *      Generates auth keys
 */
int eip_tr_genkey(struct eip_tr *tr, struct eip_tr_info *info, uint8_t *key, uint16_t len)
{
        struct crypto_sync_skcipher *sk_tfm = NULL;
        struct eip_tr_base *base = &info->base;
        struct skcipher_request *req = NULL;
        uint8_t iv[AES_BLOCK_SIZE] = {0};
        struct scatterlist sg[1];
	uint8_t *genkey;
        int err = 0;

	/*
	 * Validate if the key generation is supported
	 * currently it is only supported for aes-gcm and aes-gcm-rfc4106
	 */
	if (!strstr(base->alg_name, "gcm")) {
		pr_warn("Auth key generation not supported for algo <%s>\n", base->alg_name);
		return -ENOSYS;
	}

	/*
	 * Allocate skcipher tfm
	 */
	sk_tfm = crypto_alloc_sync_skcipher("ctr(aes-generic)", 0, 0);
	if (IS_ERR(sk_tfm)) {
		pr_err("%px: Error allocating tfm for skcipher: ctr(aes-generic)\n", sk_tfm);
		return PTR_ERR(sk_tfm);
	}

	/*
	 * Setup cipher keys
	 */
	err = crypto_sync_skcipher_setkey(sk_tfm, base->cipher.key_data, base->cipher.key_len);
	if (err) {
		pr_err("%X: Failed to set the key for skcipher: ctr(aes)\n", err);
		goto fail3;
	}

	/*
	 * Allocate memory for auth key
	 */
	genkey = kzalloc(len, GFP_KERNEL);
	if (!genkey) {
		pr_warn("%px: Unable to allocate memory for auth key\n", req);
		err = -ENOMEM;
		goto fail3;
	}

	/*
	 * Allocate skcipher request
	 */
	req = kzalloc(sizeof(struct skcipher_request), GFP_KERNEL);
	if (!req) {
		pr_warn("%px: Failed to allocate req\n", req);
		err = -ENOMEM;
		goto fail2;
	}

	sg_init_one(sg, genkey, len);
	skcipher_request_set_sync_tfm(req, sk_tfm);
	skcipher_request_set_callback(req, 0, NULL, NULL);
	skcipher_request_set_crypt(req, sg, sg, len, iv);

	err = crypto_skcipher_encrypt(req);
	if (err) {
		pr_debug("%px: Failed to encrypt; err(%d)\n", sk_tfm, err);
		goto fail1;
	}

	memcpy(key, genkey, len);
fail1:
	kfree(req);
fail2:
	kfree(genkey);
fail3:
	crypto_free_sync_skcipher(sk_tfm);
        return err;
}

/*
 * eip_tr_get_algo_info()
 * 	Get algo info.
 */
void eip_tr_get_algo_info(struct eip_tr *tr, struct eip_tr_algo_info *algo)
{
	algo->iv_len = tr->iv_len;
	algo->blk_len = tr->blk_len;
}
EXPORT_SYMBOL(eip_tr_get_algo_info);
