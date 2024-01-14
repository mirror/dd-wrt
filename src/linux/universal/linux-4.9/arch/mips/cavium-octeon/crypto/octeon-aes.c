/*
 * octeon-aes-cbc.c Accelerated AES & AES-CBC implementation with Octeon HW Crypto. (based on arm64 aes-glue.c)
 *
 * Copyright (C) 2023 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <crypto/aes.h>
#include <crypto/ablk_helper.h>
#include <crypto/algapi.h>
#include <linux/module.h>
#include <asm/octeon/octeon.h>
#include "octeon-crypto.h"

#define AES_BLOCK_MASK (~(AES_BLOCK_SIZE - 1))

struct octeon_crypto_aes_ctx {
	struct crypto_aes_ctx fallback;
	u32 key[AES_MAX_KEYLENGTH_U32];
	u32 key_length;
};

static int octeon_crypto_aes_set_key(struct crypto_tfm *tfm, const u8 *in_key,
				     unsigned int key_len)
{
	struct octeon_crypto_aes_ctx *ctx = crypto_tfm_ctx(tfm);
	memset(ctx->key, 0, sizeof(ctx->key));
	memcpy(ctx->key, in_key, key_len);
	ctx->key_length = key_len / 8 - 1;
	return crypto_aes_set_key(tfm, in_key, key_len);
}

static __always_inline void
octeon_crypto_aes_write_key(struct octeon_crypto_aes_ctx *ctx)
{
	__be64 *key = (__be64 *)ctx->key;
	write_octeon_64bit_aes_key(key[0], 0);
	write_octeon_64bit_aes_key(key[1], 1);
	write_octeon_64bit_aes_key(key[2], 2);
	write_octeon_64bit_aes_key(key[3], 3);
	write_octeon_64bit_aes_keylength(ctx->key_length);
}

static int octeon_crypto_aes_cbc_decrypt(struct blkcipher_desc *desc,
					 struct scatterlist *dst,
					 struct scatterlist *src,
					 unsigned int nbytes)
{
	struct octeon_cop2_state state;
	unsigned long flags;
	struct octeon_crypto_aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	struct crypto_tfm_ctx crypto;
	crypto.__crt_ctx = ctx;
	struct blkcipher_walk walk;
	int err, i, todo;
	__be64 *iv;
	__be64 *dataout;
	__be64 *data;
	union {
		size_t t[16 / sizeof(size_t)];
		__be64 c;
	} tmp;

	if (in_interrupt()) {
		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);
		desc->flags &= ~CRYPTO_TFM_REQ_MAY_SLEEP;
		iv = (__be64 *)walk.iv;
		while ((nbytes = walk.nbytes)) {
			todo = nbytes & AES_BLOCK_MASK;
			dataout = (__be64 *)walk.dst.virt.addr;
			data = (__be64 *)walk.src.virt.addr;
			__be64 c;
			aes_decrypt(&crypto, tmp.c, data);
			for (i = 0; i < todo / AES_BLOCK_SIZE; i++) {
				c = *data++;
				*dataout++ = tmp.c[0] ^ iv[0];
				iv[0] = c;
				c = *data++;
				*dataout++ = tmp.c[1] ^ iv[1];
				iv[1] = c;
			}
			nbytes &= AES_BLOCK_SIZE - 1;
			err = blkcipher_walk_done(desc, &walk, nbytes);
		}

	} else {
		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);
		desc->flags &= ~CRYPTO_TFM_REQ_MAY_SLEEP;
		flags = octeon_crypto_enable_no_irq_save(&state);
		octeon_crypto_aes_write_key(ctx);
		iv = (__be64 *)walk.iv;
		write_octeon_64bit_aes_iv(iv[0], 0);
		write_octeon_64bit_aes_iv(iv[1], 1);
		octeon_crypto_disable_no_irq_save(&state, flags);

		while ((nbytes = walk.nbytes)) {
			todo = nbytes & AES_BLOCK_MASK;
			dataout = (__be64 *)walk.dst.virt.addr;
			data = (__be64 *)walk.src.virt.addr;
			flags = octeon_crypto_enable_no_irq_save(&state);
			for (i = 0; i < todo / AES_BLOCK_SIZE; i++) {
				write_octeon_64bit_aes_dec_cbc0(*data++);
				write_octeon_64bit_aes_dec_cbc1(*data++);
				*dataout++ = read_octeon_64bit_aes_result(0);
				*dataout++ = read_octeon_64bit_aes_result(1);
			}
			octeon_crypto_disable_no_irq_save(&state, flags);
			nbytes &= AES_BLOCK_SIZE - 1;
			err = blkcipher_walk_done(desc, &walk, nbytes);
		}
	}
	return 0;
}

static int octeon_crypto_aes_cbc_encrypt(struct blkcipher_desc *desc,
					 struct scatterlist *dst,
					 struct scatterlist *src,
					 unsigned int nbytes)
{
	struct octeon_crypto_aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	struct crypto_tfm_ctx crypto;
	crypto.__crt_ctx = ctx;
	struct blkcipher_walk walk;
	struct octeon_cop2_state state;
	unsigned long flags;
	int err, i, todo;
	__be64 *iv;
	__be64 *dataout;
	__be64 *data;

	if (in_interrupt()) {
		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);
		desc->flags &= ~CRYPTO_TFM_REQ_MAY_SLEEP;
		iv = (__be64 *)walk.iv;
		while ((nbytes = walk.nbytes)) {
			todo = nbytes & AES_BLOCK_MASK;
			dataout = (__be64 *)walk.dst.virt.addr;
			data = (__be64 *)walk.src.virt.addr;
			for (i = 0; i < todo / AES_BLOCK_SIZE; i++) {
				*dataout++ = *data++ ^ iv[0];
				*dataout++ = *data++ ^ iv[1];
			}
			aes_encrypt(&crypto, dataout, dataout);
			iv = dataout;
			nbytes &= AES_BLOCK_SIZE - 1;
			err = blkcipher_walk_done(desc, &walk, nbytes);
		}
	} else {
		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);
		desc->flags &= ~CRYPTO_TFM_REQ_MAY_SLEEP;
		flags = octeon_crypto_enable_no_irq_save(&state);
		octeon_crypto_aes_write_key(ctx);
		iv = (__be64 *)walk.iv;
		write_octeon_64bit_aes_iv(iv[0], 0);
		write_octeon_64bit_aes_iv(iv[1], 1);
		octeon_crypto_disable_no_irq_save(&state, flags);

		while ((nbytes = walk.nbytes)) {
			todo = nbytes & AES_BLOCK_MASK;
			dataout = (__be64 *)walk.dst.virt.addr;
			data = (__be64 *)walk.src.virt.addr;
			flags = octeon_crypto_enable_no_irq_save(&state);
			for (i = 0; i < todo / AES_BLOCK_SIZE; i++) {
				write_octeon_64bit_aes_enc_cbc0(*data++);
				write_octeon_64bit_aes_enc_cbc1(*data++);
				*dataout++ = read_octeon_64bit_aes_result(0);
				*dataout++ = read_octeon_64bit_aes_result(1);
			}
			octeon_crypto_disable_no_irq_save(&state, flags);
			nbytes &= AES_BLOCK_SIZE - 1;
			err = blkcipher_walk_done(desc, &walk, nbytes);
		}
	}
	return 0;
}

static void octeon_crypto_aes_encrypt(struct crypto_tfm *tfm, u8 *out,
				      const u8 *in)
{
	struct octeon_cop2_state state;
	unsigned long flags;
	struct octeon_crypto_aes_ctx *ctx = crypto_tfm_ctx(tfm);
	__be64 *data = (__be64 *)in;
	__be64 *dataout = (__be64 *)out;
	if (in_interrupt()) {
		aes_encrypt(tfm, out, in);
	}
	flags = octeon_crypto_enable_no_irq_save(&state);
	octeon_crypto_aes_write_key(ctx);
	write_octeon_64bit_aes_enc0(*data++);
	write_octeon_64bit_aes_enc1(*data);
	*dataout++ = read_octeon_64bit_aes_result(0);
	*dataout = read_octeon_64bit_aes_result(1);

	octeon_crypto_disable_no_irq_save(&state, flags);
}

static void octeon_crypto_aes_decrypt(struct crypto_tfm *tfm, u8 *out,
				      const u8 *in)
{
	struct octeon_cop2_state state;
	unsigned long flags;
	struct octeon_crypto_aes_ctx *ctx = crypto_tfm_ctx(tfm);
	__be64 *data = (__be64 *)in;
	__be64 *dataout = (__be64 *)out;
	if (in_interrupt()) {
		aes_decrypt(tfm, out, in);
	}

	flags = octeon_crypto_enable_no_irq_save(&state);
	octeon_crypto_aes_write_key(ctx);

	write_octeon_64bit_aes_dec0(*data++);
	write_octeon_64bit_aes_dec1(*data);
	*dataout++ = read_octeon_64bit_aes_result(0);
	*dataout = read_octeon_64bit_aes_result(1);
	octeon_crypto_disable_no_irq_save(&state, flags);
}

static int ablk_cbc_init(struct crypto_tfm *tfm)
{
	return ablk_init_common(tfm, "__driver-cbc-aes-octeon");
}

static struct crypto_alg octeon_algs[] = { {
	.cra_name		= "__cbc-aes-octeon",
	.cra_driver_name	= "__driver-cbc-aes-octeon",
	.cra_priority		= 0,
	.cra_flags		= CRYPTO_ALG_TYPE_BLKCIPHER |
				  CRYPTO_ALG_INTERNAL,
	.cra_blocksize		= AES_BLOCK_SIZE,
	.cra_ctxsize		= sizeof(struct octeon_crypto_aes_ctx),
	.cra_alignmask		= 0,
	.cra_type		= &crypto_blkcipher_type,
	.cra_module		= THIS_MODULE,
	.cra_u = {
		.blkcipher = {
			.min_keysize	= AES_MIN_KEY_SIZE,
			.max_keysize	= AES_MAX_KEY_SIZE,
			.ivsize		= AES_BLOCK_SIZE,
			.setkey		= octeon_crypto_aes_set_key,
			.encrypt	= octeon_crypto_aes_cbc_encrypt,
			.decrypt	= octeon_crypto_aes_cbc_decrypt,
		}
	}
} , {
	.cra_name		= "cbc(aes)",
	.cra_driver_name	= "octeon-cbc-aes",
	.cra_priority		= 400,
	.cra_flags		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize		= AES_BLOCK_SIZE,
	.cra_ctxsize		= sizeof(struct async_helper_ctx),
	.cra_alignmask		= 0,
	.cra_type		= &crypto_ablkcipher_type,
	.cra_module		= THIS_MODULE,
	.cra_init		= ablk_cbc_init,
	.cra_exit		= ablk_exit,
	.cra_u = {
		.ablkcipher = {
			.min_keysize	= AES_MIN_KEY_SIZE,
			.max_keysize	= AES_MAX_KEY_SIZE,
			.ivsize		= AES_BLOCK_SIZE,
			.setkey		= ablk_set_key,
			.encrypt	= ablk_encrypt,
			.decrypt	= ablk_decrypt,
		},
	},
 } , {
	.cra_name		=	"aes",
	.cra_driver_name	=	"octeon-aes",
	.cra_priority		=	300,
	.cra_flags		=	CRYPTO_ALG_TYPE_CIPHER,
	.cra_blocksize		=	AES_BLOCK_SIZE,
	.cra_ctxsize		=	sizeof(struct octeon_crypto_aes_ctx),
	.cra_alignmask		=	0,
	.cra_module		=	THIS_MODULE,
	.cra_u			=	{
		.cipher = {
			.cia_min_keysize	=	AES_MIN_KEY_SIZE,
			.cia_max_keysize	=	AES_MAX_KEY_SIZE,
			.cia_setkey		=	octeon_crypto_aes_set_key,
			.cia_encrypt		=	octeon_crypto_aes_encrypt,
			.cia_decrypt		=	octeon_crypto_aes_decrypt
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

MODULE_DESCRIPTION("Rijndael (AES, AES-CBC) Cipher Algorithm");
MODULE_AUTHOR("Sebastian Gottschall <s.gottschall@dd-wrt.com>");
MODULE_LICENSE("GPL v2");
