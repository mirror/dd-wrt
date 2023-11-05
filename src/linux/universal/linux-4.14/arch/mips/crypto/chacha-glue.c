// SPDX-License-Identifier: GPL-2.0
/*
 * MIPS accelerated ChaCha and XChaCha stream ciphers,
 * including ChaCha20 (RFC7539)
 *
 * Copyright (C) 2019 Linaro, Ltd. <ard.biesheuvel@linaro.org>
 */

#include <asm/byteorder.h>
#include <crypto/algapi.h>
#include <crypto/chacha20.h>
#include <crypto/internal/skcipher.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/unaligned.h>

asmlinkage void chacha_crypt_arch(u32 *state, u8 *dst, const u8 *src,
				  unsigned int bytes, int nrounds);


static inline int chacha20_setkey(struct crypto_skcipher *tfm, const u8 *key,
			   unsigned int keysize)
{
	struct chacha20_ctx *ctx = crypto_skcipher_ctx(tfm);
	int i;

	if (keysize != CHACHA20_KEY_SIZE)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(ctx->key); i++)
		ctx->key[i] = get_unaligned_le32(key + i * sizeof(u32));

	return 0;
}

static inline void chacha_init(u32 *state, const u32 *key, const u8 *iv)
{
	state[0]  = CHACHA_CONSTANT_EXPA;
	state[1]  = CHACHA_CONSTANT_ND_3;
	state[2]  = CHACHA_CONSTANT_2_BY;
	state[3]  = CHACHA_CONSTANT_TE_K;
	state[4]  = key[0];
	state[5]  = key[1];
	state[6]  = key[2];
	state[7]  = key[3];
	state[8]  = key[4];
	state[9]  = key[5];
	state[10] = key[6];
	state[11] = key[7];
	state[12] = get_unaligned_le32(iv +  0);
	state[13] = get_unaligned_le32(iv +  4);
	state[14] = get_unaligned_le32(iv +  8);
	state[15] = get_unaligned_le32(iv + 12);
}

static int chacha20_mips(struct skcipher_request *req)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(req);
	struct chacha20_ctx *ctx = crypto_skcipher_ctx(tfm);
	struct skcipher_walk walk;
	u32 state[16];
	int err;
	err = skcipher_walk_virt(&walk, req, true);
	chacha_init(state, ctx->key, walk.iv);

	while (walk.nbytes > 0) {
		unsigned int nbytes = walk.nbytes;

		if (nbytes < walk.total)
			nbytes = round_down(nbytes, walk.stride);

		chacha_crypt_arch(state, walk.dst.virt.addr, walk.src.virt.addr, nbytes, 20);
		err = skcipher_walk_done(&walk, walk.nbytes - nbytes);
	}

	return err;
}


static struct skcipher_alg alg = {
	.base.cra_name		= "chacha20",
	.base.cra_driver_name	= "chacha20-mips",
	.base.cra_priority	= 200,
	.base.cra_blocksize	= 1,
	.base.cra_ctxsize	= sizeof(struct chacha20_ctx),
	.base.cra_module	= THIS_MODULE,

	.min_keysize		= CHACHA20_KEY_SIZE,
	.max_keysize		= CHACHA20_KEY_SIZE,
	.ivsize			= CHACHA20_IV_SIZE,
	.chunksize		= CHACHA20_BLOCK_SIZE,
	.setkey			= chacha20_setkey,
	.encrypt		= chacha20_mips,
	.decrypt		= chacha20_mips,
};

static int __init chacha20_simd_mod_init(void)
{
	return crypto_register_skcipher(&alg);
}

static void __exit chacha20_simd_mod_fini(void)
{
	crypto_unregister_skcipher(&alg);
}


module_init(chacha20_simd_mod_init);
module_exit(chacha20_simd_mod_fini);

MODULE_DESCRIPTION("ChaCha and XChaCha stream ciphers (MIPS accelerated)");
MODULE_AUTHOR("Ard Biesheuvel <ard.biesheuvel@linaro.org>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS_CRYPTO("chacha20");
MODULE_ALIAS_CRYPTO("chacha20-mips");
