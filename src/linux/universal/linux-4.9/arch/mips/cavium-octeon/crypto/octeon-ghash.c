/*
 * octeon-ghash.c
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

#include <crypto/md5.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/string.h>
#include <asm/byteorder.h>
#include <linux/cryptohash.h>
#include <asm/octeon/octeon.h>
#include <crypto/internal/hash.h>

#include "octeon-crypto.h"



#define GHASH_BLOCK_SIZE	16
#define GHASH_DIGEST_SIZE	16
#define GCM_IV_SIZE		12


struct ghash_key {
	u64			k[2];
};

struct ghash_desc_ctx {
	u64 digest[GHASH_DIGEST_SIZE/sizeof(u64)];
	u8 buf[GHASH_BLOCK_SIZE];
	u32 count;
};

static int ghash_init(struct shash_desc *desc)
{
	struct ghash_desc_ctx *ctx = shash_desc_ctx(desc);

	*ctx = (struct ghash_desc_ctx){};
	return 0;
}
static __always_inline void ghash_do_update(int blocks, u64 dg[], const char *src,
			    struct ghash_key *key, const char *head)
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
			bigin = (uint64_t*)head;
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

		src += blocks;
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

static int ghash_setkey(struct crypto_shash *tfm,
			const u8 *inkey, unsigned int keylen)
{
	struct ghash_key *key = crypto_shash_ctx(tfm);

	if (keylen != GHASH_BLOCK_SIZE)
		return -EINVAL;

	/* needed for the fallback */
	memcpy(&key->k, inkey, GHASH_BLOCK_SIZE);

	return 0;
}

static struct shash_alg ghash_alg = {
	.base.cra_name		= "ghash",
	.base.cra_driver_name	= "octeon-ghash",
	.base.cra_priority	= 200,
	.base.cra_flags		= CRYPTO_ALG_TYPE_SHASH,
	.base.cra_blocksize	= GHASH_BLOCK_SIZE,
	.base.cra_ctxsize	= sizeof(struct ghash_key),
	.base.cra_module	= THIS_MODULE,

	.digestsize		= GHASH_DIGEST_SIZE,
	.init			= ghash_init,
	.update			= ghash_update,
	.final			= ghash_final,
	.setkey			= ghash_setkey,
	.descsize		= sizeof(struct ghash_desc_ctx),
};

static int __init ghash_mod_init(void)
{
	if (!octeon_has_crypto())
		return -ENOTSUPP;
	return crypto_register_shash(&ghash_alg);
}

static void __exit ghash_mod_fini(void)
{
	crypto_unregister_shash(&ghash_alg);
}

module_init(ghash_mod_init);
module_exit(ghash_mod_fini);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("GHASH Message Digest Algorithm (OCTEON)");
MODULE_AUTHOR("Sebastian Gottschall@dd-wrt.com");
MODULE_ALIAS_CRYPTO("ghash");
