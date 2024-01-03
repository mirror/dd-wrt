/*
 * drivers/crypto/al_crypto_alg.c
 *
 * Annapurna Labs Crypto driver - ablckcipher/aead algorithms
 *
 * Copyright (C) 2012 Annapurna Labs Ltd.
 *
 * Algorithm registration code and chained scatter/gather lists
 * handling based on caam driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/*
#ifndef DEBUG
#define DEBUG
#endif
*/

#include "linux/export.h"
#include "linux/crypto.h"
#include <linux/rtnetlink.h>
#include <linux/random.h>
#include <crypto/algapi.h>
#include <crypto/aes.h>
#include <crypto/des.h>
#include <linux/scatterlist.h>
#include <crypto/internal/skcipher.h>
#include <crypto/sha.h>
#include <crypto/hash.h>
#include <crypto/authenc.h>
#include <crypto/aead.h>

#include "al_crypto.h"
#include "al_hal_ssm_crypto.h"

#define AL_CRYPTO_CRA_PRIORITY	300




static int ablkcipher_setkey_des(struct crypto_ablkcipher *ablkcipher,
			     const u8 *key, unsigned int keylen);

static int ablkcipher_setkey_aes(struct crypto_ablkcipher *ablkcipher,
			     const u8 *key, unsigned int keylen);

static int ablkcipher_encrypt(struct ablkcipher_request *req);

static int ablkcipher_decrypt(struct ablkcipher_request *req);

static int aead_setkey(struct crypto_aead *aead, const u8 *key,
		       unsigned int keylen);

static int aead_setauthsize(struct crypto_aead *aead, unsigned int authsize);

static int aead_encrypt(struct aead_request *req);

static int aead_decrypt(struct aead_request *req);
static int aead_givencrypt(struct skcipher_givcrypt_request *req);

struct al_crypto_ablkcipher_req_ctx {
	enum al_crypto_dir dir;
};

struct al_crypto_aead_req_ctx {
	u8 iv[AL_CRYPTO_MAX_IV_LENGTH] ____cacheline_aligned;
};

struct al_crypto_alg_template {
	char name[CRYPTO_MAX_ALG_NAME];
	char driver_name[CRYPTO_MAX_ALG_NAME];
	unsigned int blocksize;
	uint32_t type;
	union {
		struct ablkcipher_alg ablkcipher;
		struct blkcipher_alg blkcipher;
		struct cipher_alg cipher;
		struct compress_alg compress;
	} template_u;
	enum al_crypto_sa_enc_type enc_type;
	enum al_crypto_sa_op sa_op;
	enum al_crypto_sa_auth_type auth_type;
	enum al_crypto_sa_sha2_mode sha2_mode;
	char sw_hash_name[CRYPTO_MAX_ALG_NAME];
	unsigned int sw_hash_interm_offset;
	unsigned int sw_hash_interm_size;
};

static struct al_crypto_alg_template driver_algs[] = {
	{
		.name = "cbc(aes)",
		.driver_name = "cbc-aes-al",
		.blocksize = AES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_ABLKCIPHER,
		.template_u.ablkcipher = {
			.setkey = ablkcipher_setkey_aes,
			.encrypt = ablkcipher_encrypt,
			.decrypt = ablkcipher_decrypt,
			.geniv = "eseqiv",
			.min_keysize = AES_MIN_KEY_SIZE,
			.max_keysize = AES_MAX_KEY_SIZE,
			.ivsize = AES_BLOCK_SIZE,
		},
		.enc_type = AL_CRYPT_AES_CBC,
		.sa_op = AL_CRYPT_ENC_ONLY,
	},
	{
		.name = "ecb(aes)",
		.driver_name = "ecb-aes-al",
		.blocksize = AES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_ABLKCIPHER,
		.template_u.ablkcipher = {
			.setkey = ablkcipher_setkey_aes,
			.encrypt = ablkcipher_encrypt,
			.decrypt = ablkcipher_decrypt,
			.min_keysize = AES_MIN_KEY_SIZE,
			.max_keysize = AES_MAX_KEY_SIZE,
			.ivsize = AES_BLOCK_SIZE,
		},
		.enc_type = AL_CRYPT_AES_ECB,
		.sa_op = AL_CRYPT_ENC_ONLY,
	},
/*	{
		.name = "ctr(aes)",
		.driver_name = "ctr-aes-al",
		.blocksize = AES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_ABLKCIPHER,
		.template_u.ablkcipher = {
			.setkey = ablkcipher_setkey_aes,
			.encrypt = ablkcipher_encrypt,
			.decrypt = ablkcipher_decrypt,
			.geniv = "eseqiv",
			.min_keysize = AES_MIN_KEY_SIZE,
			.max_keysize = AES_MAX_KEY_SIZE,
			.ivsize = AES_BLOCK_SIZE,
		},
		.enc_type = AL_CRYPT_AES_CTR,
		.sa_op = AL_CRYPT_ENC_ONLY,
	},*/
	{
		.name = "cbc(des)",
		.driver_name = "cbc-des-al",
		.blocksize = DES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_ABLKCIPHER,
		.template_u.ablkcipher = {
			.setkey = ablkcipher_setkey_des,
			.encrypt = ablkcipher_encrypt,
			.decrypt = ablkcipher_decrypt,
			.geniv = "eseqiv",
			.min_keysize = DES_KEY_SIZE,
			.max_keysize = DES_KEY_SIZE,
			.ivsize = DES_BLOCK_SIZE,
		},
		.enc_type = AL_CRYPT_DES_CBC,
		.sa_op = AL_CRYPT_ENC_ONLY,
	},
	{
		.name = "ecb(des)",
		.driver_name = "ecb-des-al",
		.blocksize = DES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_ABLKCIPHER,
		.template_u.ablkcipher = {
			.setkey = ablkcipher_setkey_des,
			.encrypt = ablkcipher_encrypt,
			.decrypt = ablkcipher_decrypt,
			.min_keysize = DES_KEY_SIZE,
			.max_keysize = DES_KEY_SIZE,
			.ivsize = 0,
		},
		.enc_type = AL_CRYPT_DES_ECB,
		.sa_op = AL_CRYPT_ENC_ONLY,
	},
	{
		.name = "ecb(des3_ede)",
		.driver_name = "ecb-des3-ede-al",
		.blocksize = DES3_EDE_KEY_SIZE,
		.type = CRYPTO_ALG_TYPE_ABLKCIPHER,
		.template_u.ablkcipher = {
			.setkey = ablkcipher_setkey_des,
			.encrypt = ablkcipher_encrypt,
			.decrypt = ablkcipher_decrypt,
			.min_keysize = DES3_EDE_KEY_SIZE,
			.max_keysize = DES3_EDE_KEY_SIZE,
			.ivsize = 0,
		},
		.enc_type = AL_CRYPT_TRIPDES_ECB,
		.sa_op = AL_CRYPT_ENC_ONLY,
	},
	{
		.name = "cbc(des3_ede)",
		.driver_name = "cbc-des3-ede-al",
		.blocksize = DES_BLOCK_SIZE,
		.type = CRYPTO_ALG_TYPE_ABLKCIPHER,
		.template_u.ablkcipher = {
			.setkey = ablkcipher_setkey_des,
			.encrypt = ablkcipher_encrypt,
			.decrypt = ablkcipher_decrypt,
			.geniv = "eseqiv",
			.min_keysize = DES3_EDE_KEY_SIZE,
			.max_keysize = DES3_EDE_KEY_SIZE,
			.ivsize = DES_BLOCK_SIZE,
		},
		.enc_type = AL_CRYPT_TRIPDES_CBC,
		.sa_op = AL_CRYPT_ENC_ONLY,
	},
};

struct al_crypto_alg {
	struct list_head entry;
	struct al_crypto_device *device;
	enum al_crypto_sa_enc_type enc_type;
	enum al_crypto_sa_op sa_op;
	enum al_crypto_sa_auth_type auth_type;
	enum al_crypto_sa_sha2_mode sha2_mode;
	char sw_hash_name[CRYPTO_MAX_ALG_NAME];
	unsigned int sw_hash_interm_offset;
	unsigned int sw_hash_interm_size;
	struct crypto_alg crypto_alg;
};

/******************************************************************************
 *****************************************************************************/
static int al_crypto_cra_init(struct crypto_tfm *tfm)
{
	struct crypto_alg *alg = tfm->__crt_alg;
	struct al_crypto_alg *al_crypto_alg =
		container_of(alg, struct al_crypto_alg, crypto_alg);
	struct al_crypto_ctx *ctx = crypto_tfm_ctx(tfm);
	struct al_crypto_device *device = al_crypto_alg->device;
	int chan_idx = atomic_inc_return(&device->tfm_count) %
				(device->num_channels - device->crc_channels);

	dev_dbg(&device->pdev->dev, "%s\n", __func__);

	memset(ctx, 0, sizeof(struct al_crypto_ctx));
	memset(&ctx->sa, 0, sizeof(struct al_crypto_sa));

	ctx->chan = device->channels[chan_idx];

	ctx->sa.enc_type = al_crypto_alg->enc_type;
	ctx->sa.sa_op = al_crypto_alg->sa_op;
	ctx->hw_sa = dma_alloc_coherent(&device->pdev->dev,
			sizeof(struct al_crypto_hw_sa),
			&ctx->hw_sa_dma_addr,
			GFP_KERNEL);

	return 0;
}

/******************************************************************************
 *****************************************************************************/
static int al_crypto_cra_init_ablkcipher(struct crypto_tfm *tfm)
{
	struct al_crypto_ctx *ctx = crypto_tfm_ctx(tfm);

	al_crypto_cra_init(tfm);

	tfm->crt_ablkcipher.reqsize =
			sizeof(struct al_crypto_ablkcipher_req_ctx);

	AL_CRYPTO_STATS_LOCK(&ctx->chan->stats_gen_lock);
	AL_CRYPTO_STATS_INC(ctx->chan->stats_gen.ablkcipher_tfms, 1);
	AL_CRYPTO_STATS_UNLOCK(&ctx->chan->stats_gen_lock);

	return 0;
}


/******************************************************************************
 *****************************************************************************/
static void al_crypto_cra_exit(struct crypto_tfm *tfm)
{
	struct crypto_alg *alg = tfm->__crt_alg;
	struct al_crypto_alg *al_crypto_alg =
		container_of(alg, struct al_crypto_alg, crypto_alg);
	struct al_crypto_ctx *ctx = crypto_tfm_ctx(tfm);
	struct al_crypto_device *device = al_crypto_alg->device;

	dev_dbg(&device->pdev->dev, "%s\n", __func__);

	/* LRU list access has to be protected */
	spin_lock_bh(&ctx->chan->prep_lock);
	if (ctx->cache_state.cached)
		al_crypto_cache_remove_lru(ctx->chan, &ctx->cache_state);
	spin_unlock_bh(&ctx->chan->prep_lock);

	if (ctx->hw_sa_dma_addr)
		dma_free_coherent(&device->pdev->dev,
				sizeof(struct al_crypto_hw_sa),
				ctx->hw_sa,
				ctx->hw_sa_dma_addr);

	return;
}

/******************************************************************************
 *****************************************************************************/
static void al_crypto_cra_exit_ablkcipher(struct crypto_tfm *tfm)
{
	struct al_crypto_ctx *ctx = crypto_tfm_ctx(tfm);

	al_crypto_cra_exit(tfm);

	AL_CRYPTO_STATS_LOCK(&ctx->chan->stats_gen_lock);
	AL_CRYPTO_STATS_DEC(ctx->chan->stats_gen.ablkcipher_tfms, 1);
	AL_CRYPTO_STATS_UNLOCK(&ctx->chan->stats_gen_lock);
}


/******************************************************************************
 *****************************************************************************/
static int ablkcipher_setkey_des(struct crypto_ablkcipher *ablkcipher,
	     const u8 *key, unsigned int keylen)
{
	u32 tmp[DES_EXPKEY_WORDS];
	struct al_crypto_ctx *ctx = crypto_ablkcipher_ctx(ablkcipher);
	u32 *flags = &ablkcipher->base.crt_flags;
	int ret;

	dev_dbg(to_dev(ctx->chan), "%s\n", __func__);

	if ((ctx->sa.enc_type == AL_CRYPT_TRIPDES_CBC) ||
			(ctx->sa.enc_type == AL_CRYPT_TRIPDES_ECB)) {
		ctx->sa.tripdes_m = AL_CRYPT_TRIPDES_EDE;
		if (keylen != DES3_EDE_KEY_SIZE)
			return -EINVAL;

	} else {
		ctx->sa.tripdes_m = 0;
		if (keylen != DES_KEY_SIZE)
			return -EINVAL;

		/* check for weak keys. */
		/* Weak keys are keys that cause the encryption mode of DES
		 * to act identically to the decryption mode of DES */
		ret = des_ekey(tmp, key);
		if (unlikely(ret == 0) && (*flags & CRYPTO_TFM_REQ_WEAK_KEY)) {
			*flags |= CRYPTO_TFM_RES_WEAK_KEY;
			return -EINVAL;
		}
	}

	/* TODO: optimize HAL to hold ptrs to save this memcpy */
	/* copy the key to the sa */
	memcpy(&ctx->sa.enc_key, key, keylen);

	al_crypto_hw_sa_init(&ctx->sa, ctx->hw_sa);

	/* mark the sa as not cached, will update in next xaction */
	spin_lock_bh(&ctx->chan->prep_lock);
	if (ctx->cache_state.cached)
		al_crypto_cache_remove_lru(ctx->chan, &ctx->cache_state);
	spin_unlock_bh(&ctx->chan->prep_lock);

	return 0;
}

/******************************************************************************
 *****************************************************************************/
static int ablkcipher_setkey_aes(struct crypto_ablkcipher *ablkcipher,
			     const u8 *key, unsigned int keylen)
{
	struct al_crypto_ctx *ctx = crypto_ablkcipher_ctx(ablkcipher);

	dev_dbg(to_dev(ctx->chan), "%s\n", __func__);

	switch (keylen) {
	case 16: /* 128 bit */
		ctx->sa.aes_ksize = AL_CRYPT_AES_128;
		break;
	case 24: /* 192 bit */
		ctx->sa.aes_ksize = AL_CRYPT_AES_192;
		break;
	case 32: /* 256 bit */
		ctx->sa.aes_ksize = AL_CRYPT_AES_256;
		break;
	default: /* Invalid key size */
		return -EINVAL;
		break;
	}

	/* As for now we don't support GCM or CCM modes */
	if ((ctx->sa.enc_type == AL_CRYPT_AES_GCM) ||
		(ctx->sa.enc_type == AL_CRYPT_AES_CCM)) {
		BUG();
	}

	/* TODO: optimize HAL to hold ptrs to save this memcpy */
	/* copy the key to the sa */
	memcpy(&ctx->sa.enc_key, key, keylen);

	/* Sets the counter increment to 128 bit to be aligned with the
	 * linux implementation. We know it contradicts the NIST spec.
	 * If and when the linux will be aligned with the spec we should fix it
	 * too.
	 * This variable is relevant only for CTR, GCM and CCM modes*/
	ctx->sa.cntr_size = AL_CRYPT_CNTR_128_BIT;

	al_crypto_hw_sa_init(&ctx->sa, ctx->hw_sa);

	/* mark the sa as not cached, will update in next xaction */
	spin_lock_bh(&ctx->chan->prep_lock);
	if (ctx->cache_state.cached)
		al_crypto_cache_remove_lru(ctx->chan, &ctx->cache_state);
	spin_unlock_bh(&ctx->chan->prep_lock);

	return 0;
}


/******************************************************************************
 *****************************************************************************/
/* DMA unmap buffers for ablkcipher request
 */
static inline void al_crypto_dma_unmap_ablkcipher(struct al_crypto_chan	*chan,
		struct ablkcipher_request *req,
		int src_nents, int dst_nents,
		struct al_crypto_sw_desc *desc)
{

	if (likely(req->src == req->dst)) {
		dma_unmap_sg(to_dev(chan), req->src, src_nents,
			DMA_BIDIRECTIONAL);
	} else {
		dma_unmap_sg(to_dev(chan), req->src, src_nents,
				DMA_TO_DEVICE);
		dma_unmap_sg(to_dev(chan), req->dst, dst_nents,
				DMA_FROM_DEVICE);
	}

	if (desc && desc->hal_xaction.enc_iv_in.len)
		dma_unmap_single(to_dev(chan),
				desc->hal_xaction.enc_iv_in.addr,
				desc->hal_xaction.enc_iv_in.len,
				DMA_TO_DEVICE);
}

/******************************************************************************
 *****************************************************************************/
/* Cleanup single ablkcipher request - invoked from cleanup tasklet (interrupt
 * handler)
 */
void al_crypto_cleanup_single_ablkcipher(
		struct al_crypto_chan		*chan,
		struct al_crypto_sw_desc	*desc,
		uint32_t			comp_status)
{
	struct ablkcipher_request *req =
				(struct ablkcipher_request *)desc->req;

	al_crypto_dma_unmap_ablkcipher(chan, req, desc->src_nents,
			desc->dst_nents, desc);

	req->base.complete(&req->base, 0);
}

/******************************************************************************
 *****************************************************************************/
static inline void ablkcipher_update_stats(
		struct al_crypto_transaction *xaction,
		struct al_crypto_chan *chan)
{
	if (xaction->dir == AL_CRYPT_ENCRYPT) {
		AL_CRYPTO_STATS_INC(chan->stats_prep.ablkcipher_encrypt_reqs,
				1);
		AL_CRYPTO_STATS_INC(chan->stats_prep.ablkcipher_encrypt_bytes,
				xaction->enc_in_len);
	} else {
		AL_CRYPTO_STATS_INC(chan->stats_prep.ablkcipher_decrypt_reqs,
				1);
		AL_CRYPTO_STATS_INC(chan->stats_prep.ablkcipher_decrypt_bytes,
				xaction->enc_in_len);
	}

	if (xaction->enc_in_len <= 512)
		AL_CRYPTO_STATS_INC(
				chan->stats_prep.ablkcipher_reqs_le512,	1);
	else if ((xaction->enc_in_len > 512) && (xaction->enc_in_len <= 2048))
		AL_CRYPTO_STATS_INC(
				chan->stats_prep.ablkcipher_reqs_512_2048, 1);
	else if ((xaction->enc_in_len > 2048) && (xaction->enc_in_len <= 4096))
		AL_CRYPTO_STATS_INC(
				chan->stats_prep.ablkcipher_reqs_2048_4096, 1);
	else
		AL_CRYPTO_STATS_INC(
				chan->stats_prep.ablkcipher_reqs_gt4096, 1);
}

/******************************************************************************
 *****************************************************************************/
static inline void ablkcipher_prepare_xaction_buffers(
		struct ablkcipher_request *req,
		struct al_crypto_sw_desc *desc)
{
	int i;
	int src_idx, dst_idx;
	struct al_crypto_transaction *xaction = &desc->hal_xaction;

	src_idx = 0;
	dst_idx = 0;

	sg_map_to_xaction_buffers(req->src, desc->src_bufs, req->nbytes,
			&src_idx);
	if (likely(req->src == req->dst)) {
		for (i = 0; i < src_idx; i++)
			desc->dst_bufs[i] = desc->src_bufs[i];
		dst_idx = src_idx;
	} else
		sg_map_to_xaction_buffers(req->dst, desc->dst_bufs,
				req->nbytes, &dst_idx);

	xaction->src_size = xaction->enc_in_len = req->nbytes;
	xaction->src.bufs = &desc->src_bufs[0];
	xaction->src.num = src_idx;
	xaction->dst.bufs = &desc->dst_bufs[0];
	xaction->dst.num = dst_idx;
}

/******************************************************************************
 *****************************************************************************/
/* Prepare crypto transaction to be processed by HAL and submit to HAL
 * Grabs and releases producer lock for relevant sw ring
 */
static int ablkcipher_do_crypt(struct ablkcipher_request *req, bool lock)
{
	int idx, rc;
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	struct al_crypto_ctx *ctx = crypto_ablkcipher_ctx(ablkcipher);
	struct al_crypto_ablkcipher_req_ctx *rctx = ablkcipher_request_ctx(req);
	enum al_crypto_dir dir = rctx->dir;
	struct al_crypto_chan *chan = ctx->chan;
	struct al_crypto_transaction *xaction;
	int src_nents = 0, dst_nents = 0;
	int ivsize = crypto_ablkcipher_ivsize(ablkcipher);
	struct al_crypto_sw_desc *desc;

	src_nents = sg_count(req->src, req->nbytes);

	if (req->dst != req->src)
		dst_nents = sg_count(req->dst, req->nbytes);
	else
		dst_nents = src_nents;

	/* Currently supported max sg chain length is
	 * AL_CRYPTO_OP_MAX_DATA_BUFS(12) which is minimum of descriptors left
	 * for data in a transaction:
	 * tx: 31(supported by HW) - 1(metadata) - 1(sa_in) -
	 *			1(enc_iv_in|auth_iv_in) - 1(auth_sign_in) = 27
	 * rx: 31(supported by HW) - 1(sa_out) - 1(enc_iv_out|auth_iv_out) -
	 *			1(next_enc_iv_out) - 1(auth_sign_out) = 27
	 */
	BUG_ON((src_nents > AL_CRYPTO_OP_MAX_BUFS) ||
			(dst_nents > AL_CRYPTO_OP_MAX_BUFS));

	if (likely(req->src == req->dst)) {
		dma_map_sg(to_dev(chan), req->src, src_nents,
			DMA_BIDIRECTIONAL);
	} else {
		dma_map_sg(to_dev(chan), req->src, src_nents,
			DMA_TO_DEVICE);
		dma_map_sg(to_dev(chan), req->dst, dst_nents,
			DMA_FROM_DEVICE);
	}

	if (likely(lock))
		spin_lock_bh(&chan->prep_lock);

	if (likely(al_crypto_get_sw_desc(chan, 1) == 0))
		idx = chan->head;
	else {
		rc = ablkcipher_enqueue_request(&chan->sw_queue, req);

		al_crypto_dma_unmap_ablkcipher(chan, req, src_nents, dst_nents,
				NULL);

		if (likely(lock))
			spin_unlock_bh(&chan->prep_lock);

		dev_dbg(
			to_dev(chan),
			"%s: al_crypto_get_sw_desc failed!\n",
			__func__);

		return rc;
	}

	chan->sw_desc_num_locked = 1;
	chan->tx_desc_produced = 0;

	desc = al_crypto_get_ring_ent(chan, idx);
	desc->req = (void *)req;
	desc->req_type = AL_CRYPTO_REQ_ABLKCIPHER;
	desc->src_nents = src_nents;
	desc->dst_nents = dst_nents;

	/* prepare hal transaction */
	xaction = &desc->hal_xaction;
	memset(xaction, 0, sizeof(struct al_crypto_transaction));
	xaction->dir = dir;

	ablkcipher_prepare_xaction_buffers(req, desc);

	if ((ctx->sa.enc_type != AL_CRYPT_AES_ECB) &&
			(ctx->sa.enc_type != AL_CRYPT_DES_ECB) &&
			(ctx->sa.enc_type != AL_CRYPT_TRIPDES_ECB)) {
		xaction->enc_iv_in.addr = dma_map_single(to_dev(chan),
					req->info, ivsize, DMA_TO_DEVICE);
		if (dma_mapping_error(to_dev(chan), xaction->enc_iv_in.addr)) {
			dev_err(to_dev(chan),
				"dma_map_single failed!\n");

			al_crypto_dma_unmap_ablkcipher(chan, req, src_nents, dst_nents,
					desc);

			if (likely(lock))
				spin_unlock_bh(&chan->prep_lock);
			return -ENOMEM;
		}
		xaction->enc_iv_in.len = ivsize;
	}

	if (!ctx->cache_state.cached) {
		xaction->sa_indx = al_crypto_cache_replace_lru(chan,
				&ctx->cache_state, NULL);
		xaction->sa_in.addr = ctx->hw_sa_dma_addr;
		xaction->sa_in.len = sizeof(struct al_crypto_hw_sa);
	} else {
		al_crypto_cache_update_lru(chan, &ctx->cache_state);
		xaction->sa_indx = ctx->cache_state.idx;
	}

	xaction->flags = AL_SSM_INTERRUPT;

	ablkcipher_update_stats(xaction, chan);

	/* send crypto transaction to engine */
	rc = al_crypto_dma_prepare(chan->hal_crypto, chan->idx,
				&desc->hal_xaction);
	if (unlikely(rc != 0)) {
		dev_err(to_dev(chan),
			"al_crypto_dma_prepare failed!\n");

		al_crypto_dma_unmap_ablkcipher(chan, req, src_nents, dst_nents,
				desc);

		if (likely(lock))
			spin_unlock_bh(&chan->prep_lock);
		return rc;
	}

	chan->tx_desc_produced += desc->hal_xaction.tx_descs_count;

	al_crypto_tx_submit(chan);

	if (likely(lock))
		spin_unlock_bh(&chan->prep_lock);

	return -EINPROGRESS;
}

/******************************************************************************
 *****************************************************************************/
int ablkcipher_process_queue(struct al_crypto_chan *chan)
{
	struct crypto_async_request *async_req, *backlog;
	struct ablkcipher_request *req;
	int err = 0;

	spin_lock_bh(&chan->prep_lock);

	while (al_crypto_ring_space(chan) > 0) {
		backlog = crypto_get_backlog(&chan->sw_queue);
		async_req = crypto_dequeue_request(&chan->sw_queue);

		if (!async_req)
			break;

		if (backlog)
			backlog->complete(backlog, -EINPROGRESS);

		req = container_of(async_req, struct ablkcipher_request, base);

		err = ablkcipher_do_crypt(req, false);
		if (err != -EINPROGRESS)
			break;
	}

	spin_unlock_bh(&chan->prep_lock);

	return err;
}

/******************************************************************************
 *****************************************************************************/
static int ablkcipher_encrypt(struct ablkcipher_request *req)
{
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	struct al_crypto_ctx *ctx = crypto_ablkcipher_ctx(ablkcipher);
	struct al_crypto_ablkcipher_req_ctx *rctx = ablkcipher_request_ctx(req);
	struct al_crypto_chan *chan = ctx->chan;

	dev_dbg(to_dev(chan), "ablkcipher_encrypt %p\n", req);

	rctx->dir = AL_CRYPT_ENCRYPT;
	return ablkcipher_do_crypt(req, true);
}

/******************************************************************************
 *****************************************************************************/
static int ablkcipher_decrypt(struct ablkcipher_request *req)
{
	struct crypto_ablkcipher *ablkcipher = crypto_ablkcipher_reqtfm(req);
	struct al_crypto_ctx *ctx = crypto_ablkcipher_ctx(ablkcipher);
	struct al_crypto_ablkcipher_req_ctx *rctx = ablkcipher_request_ctx(req);
	struct al_crypto_chan *chan = ctx->chan;

	dev_dbg(to_dev(chan), "ablkcipher_decrypt %p\n", req);

	rctx->dir = AL_CRYPT_DECRYPT;
	return ablkcipher_do_crypt(req, true);
}



/******************************************************************************
 *****************************************************************************/
static struct al_crypto_alg *al_crypto_alg_alloc(
		struct al_crypto_device *device,
		struct al_crypto_alg_template *template)
{
	struct al_crypto_alg *t_alg;
	struct crypto_alg *alg;

	t_alg = kzalloc(sizeof(struct al_crypto_alg), GFP_KERNEL);
	if (!t_alg) {
		dev_err(&device->pdev->dev, "failed to allocate t_alg\n");
		return ERR_PTR(-ENOMEM);
	}

	alg = &t_alg->crypto_alg;

	snprintf(alg->cra_name, CRYPTO_MAX_ALG_NAME, "%s", template->name);
	snprintf(alg->cra_driver_name, CRYPTO_MAX_ALG_NAME, "%s",
		 template->driver_name);
	alg->cra_module = THIS_MODULE;
	alg->cra_priority = AL_CRYPTO_CRA_PRIORITY;
	alg->cra_blocksize = template->blocksize;
	alg->cra_alignmask = 0;
	alg->cra_ctxsize = sizeof(struct al_crypto_ctx);
	alg->cra_flags = CRYPTO_ALG_ASYNC | CRYPTO_ALG_KERN_DRIVER_ONLY |
			 template->type;

	switch (template->type) {
	case CRYPTO_ALG_TYPE_ABLKCIPHER:
		alg->cra_init = al_crypto_cra_init_ablkcipher;
		alg->cra_exit = al_crypto_cra_exit_ablkcipher;
		alg->cra_type = &crypto_ablkcipher_type;
		alg->cra_ablkcipher = template->template_u.ablkcipher;
		break;
	}

	t_alg->enc_type = template->enc_type;
	t_alg->auth_type = template->auth_type;
	t_alg->sha2_mode = template->sha2_mode;
	t_alg->sa_op = template->sa_op;
	t_alg->device = device;

	return t_alg;
}

/******************************************************************************
 *****************************************************************************/
int al_crypto_alg_init(struct al_crypto_device *device)
{
	int i;
	int err = 0;

	INIT_LIST_HEAD(&device->alg_list);

	atomic_set(&device->tfm_count, -1);

	/* register crypto algorithms the device supports */
	for (i = 0; i < ARRAY_SIZE(driver_algs); i++) {
		struct al_crypto_alg *t_alg;

		t_alg = al_crypto_alg_alloc(device, &driver_algs[i]);
		if (IS_ERR(t_alg)) {
			err = PTR_ERR(t_alg);
			dev_warn(&device->pdev->dev,
					"%s:%s alg allocation failed %d\n",__func__,
					driver_algs[i].driver_name, t_alg);
			continue;
		}

		err = crypto_register_alg(&t_alg->crypto_alg);
		if (err) {
			dev_warn(&device->pdev->dev,
					"%s:%s alg registration failed %d\n",__func__,
					t_alg->crypto_alg.cra_driver_name, err);
			kfree(t_alg);
		} else
			list_add_tail(&t_alg->entry, &device->alg_list);
	}

	if (!list_empty(&device->alg_list))
		dev_info(&device->pdev->dev,
				"algorithms registered in /proc/crypto\n");

	return err;
}

/******************************************************************************
 *****************************************************************************/
void al_crypto_alg_terminate(struct al_crypto_device *device)
{
	struct al_crypto_alg *t_alg, *n;

	if (!device->alg_list.next)
		return;

	list_for_each_entry_safe(t_alg, n, &device->alg_list, entry) {
		crypto_unregister_alg(&t_alg->crypto_alg);
		list_del(&t_alg->entry);
		kfree(t_alg);
	}
}
