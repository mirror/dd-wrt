/*
 * octeon-3des-cbc.c Accelerated 3DES amd 3DES-CBC implementation with Octeon HW Crypto. (based on s390 des_s390.c)
 *
 * Copyright (C) 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <crypto/ablk_helper.h>
#include <crypto/algapi.h>
#include <crypto/des.h>
#include <linux/module.h>
#include <asm/octeon/octeon.h>
#include "octeon-crypto.h"

#define DES_BLOCK_MASK (~(DES_BLOCK_SIZE - 1))
#define DES3_KEY_SIZE (DES_KEY_SIZE * 3)
struct octeon_des_ctx {
	u8 key[DES3_KEY_SIZE];
};

static int octeon_3des_set_key(struct crypto_tfm *tfm, const u8 *key,
			       unsigned int key_len)
{
	struct octeon_des_ctx *ctx = crypto_tfm_ctx(tfm);
	u32 tmp[DES_EXPKEY_WORDS];

	/* check for weak keys */
	if (!des_ekey(tmp, key) && (tfm->crt_flags & CRYPTO_TFM_REQ_WEAK_KEY)) {
		tfm->crt_flags |= CRYPTO_TFM_RES_WEAK_KEY;
		return -EINVAL;
	}

	memcpy(ctx->key, key, key_len);
	return 0;
}

static int octeon_3des_cbc_decrypt(struct blkcipher_desc *desc,
				   struct scatterlist *dst,
				   struct scatterlist *src, unsigned int nbytes)
{
	struct octeon_des_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	struct blkcipher_walk walk;
	struct octeon_cop2_state state;
	unsigned long flags;
	int err, i, todo;
	__be64 *iv;
	__be64 *key = (__be64 *)ctx->key;
	__be64 *dataout;
	__be64 *data;

	blkcipher_walk_init(&walk, dst, src, nbytes);
	err = blkcipher_walk_virt(desc, &walk);
	desc->flags &= ~CRYPTO_TFM_REQ_MAY_SLEEP;
	flags = octeon_crypto_enable(&state);
	write_octeon_64bit_3des_key(key[0], 0);
	write_octeon_64bit_3des_key(key[1], 1);
	write_octeon_64bit_3des_key(key[2], 2);
	iv = (__be64 *)walk.iv;
	write_octeon_64bit_3des_iv(*iv);
	octeon_crypto_disable(&state, flags);

	while ((nbytes = walk.nbytes)) {
		todo = nbytes & DES_BLOCK_MASK;
		dataout = (__be64 *)walk.dst.virt.addr;
		data = (__be64 *)walk.src.virt.addr;
	flags = octeon_crypto_enable(&state);
		for (i = 0; i < todo / DES_BLOCK_SIZE; i++) {
			write_octeon_64bit_3des_enc_cbc(*data++);
			*dataout++ = read_octeon_64bit_3des_result();
		}
	octeon_crypto_disable(&state, flags);
		nbytes &= DES_BLOCK_SIZE - 1;
		err = blkcipher_walk_done(desc, &walk, nbytes);
	}
	return 0;
}

static int octeon_3des_cbc_encrypt(struct blkcipher_desc *desc,
				   struct scatterlist *dst,
				   struct scatterlist *src, unsigned int nbytes)
{
	struct octeon_des_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	struct blkcipher_walk walk;
	struct octeon_cop2_state state;
	unsigned long flags;
	int err, i, todo;
	__be64 *iv;
	__be64 *key = (__be64 *)ctx->key;
	__be64 *dataout;
	__be64 *data;

	blkcipher_walk_init(&walk, dst, src, nbytes);
	err = blkcipher_walk_virt(desc, &walk);
	desc->flags &= ~CRYPTO_TFM_REQ_MAY_SLEEP;
	flags = octeon_crypto_enable(&state);
	write_octeon_64bit_3des_key(key[0], 0);
	write_octeon_64bit_3des_key(key[1], 1);
	write_octeon_64bit_3des_key(key[2], 2);
	iv = (__be64 *)walk.iv;
	write_octeon_64bit_3des_iv(*iv);
	octeon_crypto_disable(&state, flags);

	while ((nbytes = walk.nbytes)) {
		todo = nbytes & DES_BLOCK_MASK;
		dataout = (__be64 *)walk.dst.virt.addr;
		data = (__be64 *)walk.src.virt.addr;
	flags = octeon_crypto_enable(&state);
		for (i = 0; i < todo / DES_BLOCK_SIZE; i++) {
			write_octeon_64bit_3des_dec_cbc(*data++);
			*dataout++ = read_octeon_64bit_3des_result();
		}
	octeon_crypto_disable(&state, flags);
		nbytes &= DES_BLOCK_SIZE - 1;
		err = blkcipher_walk_done(desc, &walk, nbytes);
	}
	return 0;
}

static void octeon_3des_encrypt(struct crypto_tfm *tfm, u8 *out, const u8 *in)
{
	struct octeon_cop2_state state;
	unsigned long flags;
	struct octeon_des_ctx *ctx = crypto_tfm_ctx(tfm);
	__be64 *data = (__be64 *)in;
	__be64 *dataout = (__be64 *)out;
	__be64 *key = (__be64 *)ctx->key;
	flags = octeon_crypto_enable(&state);
	write_octeon_64bit_3des_key(key[0], 0);
	write_octeon_64bit_3des_key(key[1], 1);
	write_octeon_64bit_3des_key(key[2], 2);

	write_octeon_64bit_3des_enc(*data++);
	*dataout = read_octeon_64bit_3des_result();

	octeon_crypto_disable(&state, flags);
}

static void octeon_3des_decrypt(struct crypto_tfm *tfm, u8 *out, const u8 *in)
{
	struct octeon_cop2_state state;
	unsigned long flags;
	struct octeon_des_ctx *ctx = crypto_tfm_ctx(tfm);
	__be64 *data = (__be64 *)in;
	__be64 *dataout = (__be64 *)out;
	__be64 *key = (__be64 *)ctx->key;

	flags = octeon_crypto_enable(&state);
	write_octeon_64bit_3des_key(key[0], 0);
	write_octeon_64bit_3des_key(key[1], 1);
	write_octeon_64bit_3des_key(key[2], 2);

	write_octeon_64bit_3des_dec(*data++);
	*dataout = read_octeon_64bit_3des_result();
	octeon_crypto_disable(&state, flags);
}

static int ablk_cbc_init(struct crypto_tfm *tfm)
{
	return ablk_init_common(tfm, "__driver-cbc-3des-octeon");
}

static struct crypto_alg octeon_algs[] = { {
	.cra_name		= "__cbc-3des-octeon",
	.cra_driver_name	= "__driver-cbc-3des-octeon",
	.cra_priority		= 0,
	.cra_flags		= CRYPTO_ALG_TYPE_BLKCIPHER |
				  CRYPTO_ALG_INTERNAL,
	.cra_blocksize		= DES_BLOCK_SIZE,
	.cra_ctxsize		= sizeof(struct octeon_des_ctx),
	.cra_alignmask		= 0,
	.cra_type		= &crypto_blkcipher_type,
	.cra_module		= THIS_MODULE,
	.cra_u = {
		.blkcipher = {
			.min_keysize	= DES3_KEY_SIZE,
			.max_keysize	= DES3_KEY_SIZE,
			.ivsize		= DES_BLOCK_SIZE,
			.setkey		= octeon_3des_set_key,
			.encrypt	= octeon_3des_cbc_encrypt,
			.decrypt	= octeon_3des_cbc_decrypt,
		}
	}
} , {
	.cra_name		= "cbc(des3_ede)",
	.cra_driver_name	= "octeon-cbc-3des",
	.cra_priority		= 400,
	.cra_flags		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize		= DES_BLOCK_SIZE,
	.cra_ctxsize		= sizeof(struct async_helper_ctx),
	.cra_alignmask		= 0,
	.cra_type		= &crypto_ablkcipher_type,
	.cra_module		= THIS_MODULE,
	.cra_init		= ablk_cbc_init,
	.cra_exit		= ablk_exit,
	.cra_u = {
		.ablkcipher = {
			.min_keysize	= DES3_KEY_SIZE,
			.max_keysize	= DES3_KEY_SIZE,
			.ivsize		= DES_BLOCK_SIZE,
			.setkey		= ablk_set_key,
			.encrypt	= ablk_encrypt,
			.decrypt	= ablk_decrypt,
		},
	},
 }, {
	.cra_name		=	"des3_ede",
	.cra_driver_name	=	"octeon-3des",
	.cra_priority		=	300,
	.cra_flags		=	CRYPTO_ALG_TYPE_CIPHER,
	.cra_blocksize		=	DES_BLOCK_SIZE,
	.cra_ctxsize		=	sizeof(struct octeon_des_ctx),
	.cra_alignmask		=	0,
	.cra_module		=	THIS_MODULE,
	.cra_u			=	{
		.cipher = {
			.cia_min_keysize	=	DES3_KEY_SIZE,
			.cia_max_keysize	=	DES3_KEY_SIZE,
			.cia_setkey		=	octeon_3des_set_key,
			.cia_encrypt		=	octeon_3des_encrypt,
			.cia_decrypt		=	octeon_3des_decrypt
		}
	}

 
 
}
};

static int __init octeon_mod_init(void)
{
	if (!octeon_has_crypto())
		return -ENOTSUPP;
	return crypto_register_algs(octeon_algs, ARRAY_SIZE(octeon_algs));
}

static void __exit octeon_mod_exit(void)
{
	crypto_unregister_algs(octeon_algs, ARRAY_SIZE(octeon_algs));
}

module_init(octeon_mod_init);
module_exit(octeon_mod_exit);

MODULE_DESCRIPTION("(3DES-CBC) Cipher Algorithm");
MODULE_AUTHOR("Sebastian Gottschall <s.gottschall@dd-wrt.com>");
MODULE_LICENSE("GPL v2");
