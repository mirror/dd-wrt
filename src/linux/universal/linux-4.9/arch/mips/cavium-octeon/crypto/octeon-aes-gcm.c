/*
 * Accelerated GHASH amd AES-GCM implementation with Octeon HW Crypto. (based on arm64 ghash-ce-glue.c)
 *
 * Copyright (C) 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <asm/unaligned.h>
#include <crypto/aes.h>
#include <crypto/algapi.h>
#include <crypto/b128ops.h>
#include <crypto/gf128mul.h>
#include <crypto/internal/aead.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/skcipher.h>
#include <crypto/scatterwalk.h>
#include <linux/cpufeature.h>
#include <linux/crypto.h>
#include <linux/module.h>
#include <asm/octeon/octeon.h>
#include "octeon-crypto.h"

#define GHASH_BLOCK_SIZE 16
#define GHASH_DIGEST_SIZE 16
#define GCM_IV_SIZE 12

struct ghash_key {
	u64 k[2];
};

struct ghash_desc_ctx {
	u64 digest[GHASH_DIGEST_SIZE / sizeof(u64)];
	u8 buf[GHASH_BLOCK_SIZE];
	u32 count;
};

struct gcm_aes_ctx {
	struct crypto_aes_ctx aes_key;
	struct ghash_key ghash_key;
};

static int ghash_init(struct shash_desc *desc)
{
	struct ghash_desc_ctx *ctx = shash_desc_ctx(desc);
	*ctx = (struct ghash_desc_ctx){};
	return 0;
}

static __always_inline void ghash_do_update(int blocks, u64 dg[],
					    const char *src,
					    struct ghash_key *key,
					    const char *head)
{
	struct octeon_cop2_state state;
	unsigned long flags;
	flags = octeon_crypto_enable(&state);
	write_octeon_64bit_gfm_poly((uint64_t)0xe100);
	write_octeon_64bit_gfm_resinp(dg[0], 0);
	write_octeon_64bit_gfm_resinp(dg[1], 1);
	write_octeon_64bit_gfm_mul(key->k[0], 0);
	write_octeon_64bit_gfm_mul(key->k[1], 1);

	do {
		const uint64_t *bigin = (uint64_t *)src;
		if (head) {
			bigin = (uint64_t *)head;
			blocks++;
			head = NULL;
		} else {
			src += GHASH_BLOCK_SIZE;
		}
		write_octeon_64bit_gfm_xor0(bigin[0]);
		write_octeon_64bit_gfm_xormul1(bigin[1]);
	} while (--blocks);
	dg[0] = read_octeon_64bit_gfm_resinp(0);
	dg[1] = read_octeon_64bit_gfm_resinp(1);
	octeon_crypto_disable(&state, flags);
}

static int ghash_update(struct shash_desc *desc, const u8 *src,
			unsigned int len)
{
	struct ghash_desc_ctx *ctx = shash_desc_ctx(desc);
	unsigned int partial = ctx->count % GHASH_BLOCK_SIZE;
	ctx->count += len;

	if ((partial + len) >= GHASH_BLOCK_SIZE) {
		struct ghash_key *key = crypto_shash_ctx(desc->tfm);
		int blocks;

		if (partial) {
			int p = GHASH_BLOCK_SIZE - partial;

			memcpy(ctx->buf + partial, src, p);
			src += p;
			len -= p;
		}

		blocks = len / GHASH_BLOCK_SIZE;
		len %= GHASH_BLOCK_SIZE;

		ghash_do_update(blocks, ctx->digest, src, key,
				partial ? ctx->buf : NULL);

		src += blocks * GHASH_BLOCK_SIZE;
		partial = 0;
	}
	if (len)
		memcpy(ctx->buf + partial, src, len);
	return 0;
}

static int ghash_final(struct shash_desc *desc, u8 *dst)
{
	struct ghash_desc_ctx *ctx = shash_desc_ctx(desc);
	unsigned int partial = ctx->count % GHASH_BLOCK_SIZE;

	if (partial) {
		struct ghash_key *key = crypto_shash_ctx(desc->tfm);

		memset(ctx->buf + partial, 0, GHASH_BLOCK_SIZE - partial);

		ghash_do_update(1, ctx->digest, ctx->buf, key, NULL);
	}
	memcpy(dst, ctx->digest, 16);
	memzero_explicit(ctx, sizeof(*ctx));
	return 0;
}

static int __ghash_setkey(struct ghash_key *key, const u8 *inkey,
			  unsigned int keylen)
{
	/* needed for the fallback */
	memcpy(&key->k, inkey, GHASH_BLOCK_SIZE);
	return 0;
}

static int ghash_setkey(struct crypto_shash *tfm, const u8 *inkey,
			unsigned int keylen)
{
	struct ghash_key *key = crypto_shash_ctx(tfm);

	if (keylen != GHASH_BLOCK_SIZE)
		return -EINVAL;

	__ghash_setkey(key, inkey, keylen);

	return 0;
}

static struct shash_alg ghash_alg = {
	.base.cra_name = "ghash",
	.base.cra_driver_name = "octeon-ghash",
	.base.cra_priority = 200,
	.base.cra_flags = CRYPTO_ALG_TYPE_SHASH,
	.base.cra_blocksize = GHASH_BLOCK_SIZE,
	.base.cra_ctxsize = sizeof(struct ghash_key),
	.base.cra_module = THIS_MODULE,

	.digestsize = GHASH_DIGEST_SIZE,
	.init = ghash_init,
	.update = ghash_update,
	.final = ghash_final,
	.setkey = ghash_setkey,
	.descsize = sizeof(struct ghash_desc_ctx),
};

static __always_inline void __octeon_aes_encrypt(u32 *rk, u8 *out, u8 *in,
						 u32 keylen)
{
	struct octeon_cop2_state state;
	unsigned long flags;
	__be64 *dataout = (__be64 *)out;
	__be64 *data = (__be64 *)in;
	__be64 *key = (__be64 *)rk;
	flags = octeon_crypto_enable(&state);
	write_octeon_64bit_aes_key(key[0], 0);
	write_octeon_64bit_aes_key(key[1], 1);
	write_octeon_64bit_aes_key(key[2], 2);
	write_octeon_64bit_aes_key(key[3], 3);
	write_octeon_64bit_aes_keylength(keylen);
	write_octeon_64bit_aes_enc0(*data++);
	write_octeon_64bit_aes_enc1(*data);
	*dataout++ = read_octeon_64bit_aes_result(0);
	*dataout = read_octeon_64bit_aes_result(1);
	octeon_crypto_disable(&state, flags);
}

static int gcm_setkey(struct crypto_aead *tfm, const u8 *inkey,
		      unsigned int keylen)
{
	struct gcm_aes_ctx *ctx = crypto_aead_ctx(tfm);
	u8 key[GHASH_BLOCK_SIZE];
	int ret, i;
	ret = crypto_aes_expand_key(&ctx->aes_key, inkey, keylen);
	if (ret) {
		tfm->base.crt_flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
		return -EINVAL;
	}
	for (i = 0; i < ctx->aes_key.key_length / 4; i++)
		ctx->aes_key.key_enc[i] = cpu_to_le32(ctx->aes_key.key_enc[i]);
	ctx->aes_key.key_length = ctx->aes_key.key_length / 8 - 1;
	__octeon_aes_encrypt(ctx->aes_key.key_enc, key, (u8[AES_BLOCK_SIZE]){},
			     ctx->aes_key.key_length);

	return __ghash_setkey(&ctx->ghash_key, key, sizeof(be128));
}

static int gcm_setauthsize(struct crypto_aead *tfm, unsigned int authsize)
{
	switch (authsize) {
	case 4:
	case 8:
	case 12 ... 16:
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void gcm_update_mac(u64 dg[], const u8 *src, int count, u8 buf[],
			   int *buf_count, struct gcm_aes_ctx *ctx)
{
	if (*buf_count > 0) {
		int buf_added = min(count, GHASH_BLOCK_SIZE - *buf_count);

		memcpy(&buf[*buf_count], src, buf_added);

		*buf_count += buf_added;
		src += buf_added;
		count -= buf_added;
	}

	if (count >= GHASH_BLOCK_SIZE || *buf_count == GHASH_BLOCK_SIZE) {
		int blocks = count / GHASH_BLOCK_SIZE;

		ghash_do_update(blocks, dg, src, &ctx->ghash_key,
				*buf_count ? buf : NULL);

		src += blocks * GHASH_BLOCK_SIZE;
		count %= GHASH_BLOCK_SIZE;
		*buf_count = 0;
	}

	if (count > 0) {
		memcpy(buf, src, count);
		*buf_count = count;
	}
}

static void gcm_calculate_auth_mac(struct aead_request *req, u64 dg[])
{
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct gcm_aes_ctx *ctx = crypto_aead_ctx(aead);
	u8 buf[GHASH_BLOCK_SIZE];
	struct scatter_walk walk;
	u32 len = req->assoclen;
	int buf_count = 0;

	scatterwalk_start(&walk, req->src);

	do {
		u32 n = scatterwalk_clamp(&walk, len);
		u8 *p;

		if (!n) {
			scatterwalk_start(&walk, sg_next(walk.sg));
			n = scatterwalk_clamp(&walk, len);
		}
		p = scatterwalk_map(&walk);

		gcm_update_mac(dg, p, n, buf, &buf_count, ctx);
		len -= n;

		scatterwalk_unmap(p);
		scatterwalk_advance(&walk, n);
		scatterwalk_done(&walk, 0, len);
	} while (len);

	if (buf_count) {
		memset(&buf[buf_count], 0, GHASH_BLOCK_SIZE - buf_count);
		ghash_do_update(1, dg, buf, &ctx->ghash_key, NULL);
	}
}

static void gcm_final(struct aead_request *req, struct gcm_aes_ctx *ctx,
		      u64 dg[], u8 tag[], int cryptlen)
{
	u8 mac[AES_BLOCK_SIZE];
	u128 lengths;
	lengths.a = cpu_to_be64(req->assoclen * 8);
	lengths.b = cpu_to_be64(cryptlen * 8);

	ghash_do_update(1, dg, (void *)&lengths, &ctx->ghash_key, NULL);
	memcpy(mac, dg, AES_BLOCK_SIZE);

	crypto_xor(tag, mac, AES_BLOCK_SIZE);
}

static int gcm_encrypt(struct aead_request *req)
{
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct gcm_aes_ctx *ctx = crypto_aead_ctx(aead);
	struct skcipher_walk walk;
	u8 iv[AES_BLOCK_SIZE];
	u64 ks[2 * AES_BLOCK_SIZE / 8];
	u8 tag[AES_BLOCK_SIZE];
	u64 dg[2] = {};
	int err;

	if (req->assoclen)
		gcm_calculate_auth_mac(req, dg);

	memcpy(iv, req->iv, GCM_IV_SIZE);
	put_unaligned_be32(1, iv + GCM_IV_SIZE);

	err = skcipher_walk_aead_encrypt(&walk, req, false);

	__octeon_aes_encrypt(ctx->aes_key.key_enc, tag, iv,
			     ctx->aes_key.key_length);
	put_unaligned_be32(2, iv + GCM_IV_SIZE);

	while (walk.nbytes >= (2 * AES_BLOCK_SIZE)) {
		const int blocks = walk.nbytes / (2 * AES_BLOCK_SIZE) * 2;
		__be16 *dst = walk.dst.virt.addr;
		__be16 *src = walk.src.virt.addr;
		int remaining = blocks;
		do {
			__octeon_aes_encrypt(ctx->aes_key.key_enc, (u8 *)ks, iv,
					     ctx->aes_key.key_length);
			dst[0] = src[0] ^ ks[0];
			dst[1] = src[1] ^ ks[1];
			crypto_inc(iv, AES_BLOCK_SIZE);

			dst += 2;
			src += 2;
		} while (--remaining > 0);

		ghash_do_update(blocks, dg, walk.dst.virt.addr, &ctx->ghash_key,
				NULL);

		err = skcipher_walk_done(&walk,
					 walk.nbytes % (2 * AES_BLOCK_SIZE));
	}

	if (walk.nbytes) {
		u8 buf[GHASH_BLOCK_SIZE];
		unsigned int nbytes = walk.nbytes;
		__be16 *dst = walk.dst.virt.addr;
		u8 *head = NULL;
		__octeon_aes_encrypt(ctx->aes_key.key_enc, (u8 *)ks, iv,
				     ctx->aes_key.key_length);
		if (walk.nbytes > AES_BLOCK_SIZE) {
			crypto_inc(iv, AES_BLOCK_SIZE);
			__octeon_aes_encrypt(ctx->aes_key.key_enc,
					     ((u8 *)ks) + AES_BLOCK_SIZE, iv,
					     ctx->aes_key.key_length);
		}

		crypto_xor_cpy((u8 *)dst, walk.src.virt.addr, ks, walk.nbytes);

		if (walk.nbytes > GHASH_BLOCK_SIZE) {
			head = dst;
			dst += GHASH_BLOCK_SIZE / 8;
			nbytes %= GHASH_BLOCK_SIZE;
		}

		memcpy(buf, dst, nbytes);
		memset(buf + nbytes, 0, GHASH_BLOCK_SIZE - nbytes);
		ghash_do_update(!!nbytes, dg, buf, &ctx->ghash_key, head);

		err = skcipher_walk_done(&walk, 0);
	}

	if (err) {
		return err;
	}
	gcm_final(req, ctx, dg, tag, req->cryptlen);

	/* copy authtag to end of dst */
	scatterwalk_map_and_copy(tag, req->dst, req->assoclen + req->cryptlen,
				 crypto_aead_authsize(aead), 1);

	return 0;
}

static int gcm_decrypt(struct aead_request *req)
{
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct gcm_aes_ctx *ctx = crypto_aead_ctx(aead);
	unsigned int authsize = crypto_aead_authsize(aead);
	struct skcipher_walk walk;
	u8 iv[2 * AES_BLOCK_SIZE];
	u8 tag[AES_BLOCK_SIZE];
	u64 buf[2 * GHASH_BLOCK_SIZE / 8];
	u64 dg[2] = {};
	int err;

	if (req->assoclen)
		gcm_calculate_auth_mac(req, dg);

	memcpy(iv, req->iv, GCM_IV_SIZE);
	put_unaligned_be32(1, iv + GCM_IV_SIZE);

	err = skcipher_walk_aead_decrypt(&walk, req, false);

	__octeon_aes_encrypt(ctx->aes_key.key_enc, tag, iv,
			     ctx->aes_key.key_length);
	put_unaligned_be32(2, iv + GCM_IV_SIZE);

	while (walk.nbytes >= (2 * AES_BLOCK_SIZE)) {
		int blocks = walk.nbytes / (2 * AES_BLOCK_SIZE) * 2;
		__be64 *dst = walk.dst.virt.addr;
		__be64 *src = walk.src.virt.addr;

		ghash_do_update(blocks, dg, walk.src.virt.addr, &ctx->ghash_key,
				NULL);

		do {
			__octeon_aes_encrypt(ctx->aes_key.key_enc, (u8 *)buf,
					     iv, ctx->aes_key.key_length);
			dst[0] = src[0] ^ buf[0];
			dst[1] = src[1] ^ buf[1];
			crypto_inc(iv, AES_BLOCK_SIZE);
			dst += 2;
			src += 2;
		} while (--blocks > 0);

		err = skcipher_walk_done(&walk,
					 walk.nbytes % (2 * AES_BLOCK_SIZE));
	}
	if (walk.nbytes) {
		const u8 *src = walk.src.virt.addr;
		const u8 *head = NULL;
		unsigned int nbytes = walk.nbytes;
		if (walk.nbytes > AES_BLOCK_SIZE) {
			u8 *iv2 = iv + AES_BLOCK_SIZE;

			memcpy(iv2, iv, AES_BLOCK_SIZE);
			crypto_inc(iv2, AES_BLOCK_SIZE);

			__octeon_aes_encrypt(ctx->aes_key.key_enc, iv2, iv2,
					     ctx->aes_key.key_length);
		}
		__octeon_aes_encrypt(ctx->aes_key.key_enc, iv, iv,
				     ctx->aes_key.key_length);

		if (walk.nbytes > GHASH_BLOCK_SIZE) {
			head = src;
			src += GHASH_BLOCK_SIZE;
			nbytes %= GHASH_BLOCK_SIZE;
		}

		memcpy(buf, src, nbytes);
		memset(buf + nbytes, 0, GHASH_BLOCK_SIZE - nbytes);
		ghash_do_update(!!nbytes, dg, buf, &ctx->ghash_key, head);

		crypto_xor_cpy(walk.dst.virt.addr, walk.src.virt.addr, iv,
			       walk.nbytes);

		err = skcipher_walk_done(&walk, 0);
	}

	if (err) {
		return err;
	}
	gcm_final(req, ctx, dg, tag, req->cryptlen - authsize);

	/* compare calculated auth tag with the stored one */
	scatterwalk_map_and_copy(buf, req->src,
				 req->assoclen + req->cryptlen - authsize,
				 authsize, 0);

	if (crypto_memneq(tag, buf, authsize))
		return -EBADMSG;
	return 0;
}

static struct aead_alg gcm_aes_alg = {
	.ivsize = GCM_IV_SIZE,
	.chunksize = 2 * AES_BLOCK_SIZE,
	.maxauthsize = AES_BLOCK_SIZE,
	.setkey = gcm_setkey,
	.setauthsize = gcm_setauthsize,
	.encrypt = gcm_encrypt,
	.decrypt = gcm_decrypt,

	.base.cra_name = "gcm(aes)",
	.base.cra_driver_name = "octeon-gcm-aes",
	.base.cra_priority = 300,
	.base.cra_blocksize = 1,
	.base.cra_ctxsize = sizeof(struct gcm_aes_ctx),
	.base.cra_module = THIS_MODULE,
};

static int __init ghash_ce_mod_init(void)
{
	int ret;
	if (!octeon_has_crypto())
		return -ENOTSUPP;

	ret = crypto_register_shash(&ghash_alg);
	if (ret)
		return ret;

	ret = crypto_register_aead(&gcm_aes_alg);
	if (ret)
		crypto_unregister_shash(&ghash_alg);
	return ret;
}

static void __exit ghash_ce_mod_exit(void)
{
	crypto_unregister_shash(&ghash_alg);
	crypto_unregister_aead(&gcm_aes_alg);
}

module_init(ghash_ce_mod_init);
module_exit(ghash_ce_mod_exit);

MODULE_DESCRIPTION("GHASH & AES-GCM using Octeon HW Crypto");
MODULE_AUTHOR("Sebastian Gottschall <s.gottschall@dd-wrt.com>");
MODULE_LICENSE("GPL v2");
