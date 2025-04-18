/*
 * octeon-crc.c Accelerated CRC32 and CRC32C implementation with Octeon HW Crypto.
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

#include <linux/crc32.h>
#include <linux/cpufeature.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#include <crypto/internal/hash.h>
#include <asm/octeon/octeon.h>
#include "octeon-crypto.h"

extern void octeon_cop2_crc_save(struct octeon_cop2_state *);
extern void octeon_cop2_crc_restore(struct octeon_cop2_state *);

static unsigned long octeon_crypto_crc_enable(struct octeon_cop2_state *state)
{
	int status;
	preempt_disable();
	status = read_c0_status();
	write_c0_status(status | ST0_CU2);
	if (KSTK_STATUS(current) & ST0_CU2) {
		octeon_cop2_crc_save(&(current->thread.cp2));
		KSTK_STATUS(current) &= ~ST0_CU2;
		status &= ~ST0_CU2;
	} else if (status & ST0_CU2) {
		octeon_cop2_crc_save(state);
	}
	return status & ST0_CU2;
}

static void octeon_crypto_crc_disable(struct octeon_cop2_state *state,
				      unsigned long crypto_flags)
{
	if (crypto_flags & ST0_CU2)
		octeon_cop2_crc_restore(state);
	else
		write_c0_status(read_c0_status() & ~ST0_CU2);
	preempt_enable();
}

static u32 crc32_octeon_le_hw(u32 crc, const u8 *p, unsigned int len)
{
	struct octeon_cop2_state state;
	unsigned long flags;
	s32 length = len;

	if (in_interrupt())
		return crc32_le(crc, p, len);

	flags = octeon_crypto_crc_enable(&state);
	crc = read_octeon_64bit_es32(crc);
	write_octeon_64bit_crc_polynominal(0x04c11db7);
	write_octeon_64bit_crc_iv_reflect(crc);

	while (length >= 64) {
		u64 *p64 = (void *)p;
		write_octeon_64bit_crc_dword_reflect(p64[0]);
		write_octeon_64bit_crc_dword_reflect(p64[1]);
		write_octeon_64bit_crc_dword_reflect(p64[2]);
		write_octeon_64bit_crc_dword_reflect(p64[3]);
		write_octeon_64bit_crc_dword_reflect(p64[4]);
		write_octeon_64bit_crc_dword_reflect(p64[5]);
		write_octeon_64bit_crc_dword_reflect(p64[6]);
		write_octeon_64bit_crc_dword_reflect(p64[7]);
		p += 64;
		length -= 64;
	}
	while ((length -= sizeof(u64)) >= 0) {
		write_octeon_64bit_crc_dword_reflect(*(u64 *)p);
		p += sizeof(u64);
	}

	/* The following is more efficient than the straight loop */
	if (length & sizeof(u32)) {
		write_octeon_64bit_crc_word_reflect(*(u32 *)p);
		p += sizeof(u32);
	}
	if (length & sizeof(u16)) {
		write_octeon_64bit_crc_half_reflect(*(u16 *)p);
		p += sizeof(u16);
	}
	if (length & sizeof(u8))
		write_octeon_64bit_crc_byte_reflect(*p);

	crc = read_octeon_64bit_crc_iv_reflect();

	octeon_crypto_crc_disable(&state, flags);

	return crc;
}

static u32 crc32c_octeon_le_hw(u32 crc, const u8 *p, unsigned int len)
{
	struct octeon_cop2_state state;
	unsigned long flags;
	s32 length = len;

	if (in_interrupt())
		return __crc32c_le(crc, p, len);

	flags = octeon_crypto_crc_enable(&state);
	crc = read_octeon_64bit_es32(crc);
	write_octeon_64bit_crc_polynominal(0x1edc6f41);
	write_octeon_64bit_crc_iv_reflect(crc);

	while (length >= 64) {
		u64 *p64 = (void *)p;
		write_octeon_64bit_crc_dword_reflect(p64[0]);
		write_octeon_64bit_crc_dword_reflect(p64[1]);
		write_octeon_64bit_crc_dword_reflect(p64[2]);
		write_octeon_64bit_crc_dword_reflect(p64[3]);
		write_octeon_64bit_crc_dword_reflect(p64[4]);
		write_octeon_64bit_crc_dword_reflect(p64[5]);
		write_octeon_64bit_crc_dword_reflect(p64[6]);
		write_octeon_64bit_crc_dword_reflect(p64[7]);
		p += 64;
		length -= 64;
	}

	while ((length -= sizeof(u64)) >= 0) {
		write_octeon_64bit_crc_dword_reflect(*(u64 *)p);
		p += sizeof(u64);
	}

	/* The following is more efficient than the straight loop */
	if (length & sizeof(u32)) {
		write_octeon_64bit_crc_word_reflect(*(u32 *)p);
		p += sizeof(u32);
	}
	if (length & sizeof(u16)) {
		write_octeon_64bit_crc_half_reflect(*(u16 *)p);
		p += sizeof(u16);
	}
	if (length & sizeof(u8))
		write_octeon_64bit_crc_byte_reflect(*p);

	crc = read_octeon_64bit_crc_iv_reflect();

	octeon_crypto_crc_disable(&state, flags);

	return crc;
}

#define CHKSUM_BLOCK_SIZE 1
#define CHKSUM_DIGEST_SIZE 4

struct chksum_ctx {
	u32 key;
};

struct chksum_desc_ctx {
	u32 crc;
};

static int chksum_init(struct shash_desc *desc)
{
	struct chksum_ctx *mctx = crypto_shash_ctx(desc->tfm);
	struct chksum_desc_ctx *ctx = shash_desc_ctx(desc);

	ctx->crc = mctx->key;

	return 0;
}

/*
 * Setting the seed allows arbitrary accumulators and flexible XOR policy
 * If your algorithm starts with ~0, then XOR with ~0 before you set
 * the seed.
 */
static int chksum_setkey(struct crypto_shash *tfm, const u8 *key,
			 unsigned int keylen)
{
	struct chksum_ctx *mctx = crypto_shash_ctx(tfm);

	if (keylen != sizeof(mctx->key)) {
		crypto_shash_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	mctx->key = le32_to_cpup((__le32 *)key);
	return 0;
}

static int chksum_update(struct shash_desc *desc, const u8 *data,
			 unsigned int length)
{
	struct chksum_desc_ctx *ctx = shash_desc_ctx(desc);

	ctx->crc = crc32_octeon_le_hw(ctx->crc, data, length);
	return 0;
}

static int chksum_final(struct shash_desc *desc, u8 *out)
{
	struct chksum_desc_ctx *ctx = shash_desc_ctx(desc);

	*(__le32 *)out = cpu_to_le32p(&ctx->crc);
	return 0;
}

static int __chksum_finup(u32 crc, const u8 *data, unsigned int len, u8 *out)
{
	*(__le32 *)out = cpu_to_le32(crc32_octeon_le_hw(crc, data, len));
	return 0;
}

static int chksum_finup(struct shash_desc *desc, const u8 *data,
			unsigned int len, u8 *out)
{
	struct chksum_desc_ctx *ctx = shash_desc_ctx(desc);

	return __chksum_finup(ctx->crc, data, len, out);
}

static int chksum_digest(struct shash_desc *desc, const u8 *data,
			 unsigned int length, u8 *out)
{
	struct chksum_ctx *mctx = crypto_shash_ctx(desc->tfm);

	return __chksum_finup(mctx->key, data, length, out);
}

static int chksumc_update(struct shash_desc *desc, const u8 *data,
			  unsigned int length)
{
	struct chksum_desc_ctx *ctx = shash_desc_ctx(desc);
	ctx->crc = crc32c_octeon_le_hw(ctx->crc, data, length);
	return 0;
}

static int chksumc_final(struct shash_desc *desc, u8 *out)
{
	struct chksum_desc_ctx *ctx = shash_desc_ctx(desc);

	*(__le32 *)out = ~cpu_to_le32p(&ctx->crc);
	return 0;
}

static int __chksumc_finup(u32 crc, const u8 *data, unsigned int len, u8 *out)
{
	*(__le32 *)out = ~cpu_to_le32(crc32c_octeon_le_hw(crc, data, len));
	return 0;
}

static int chksumc_finup(struct shash_desc *desc, const u8 *data,
			 unsigned int len, u8 *out)
{
	struct chksum_desc_ctx *ctx = shash_desc_ctx(desc);

	return __chksumc_finup(ctx->crc, data, len, out);
}

static int chksumc_digest(struct shash_desc *desc, const u8 *data,
			  unsigned int length, u8 *out)
{
	struct chksum_ctx *mctx = crypto_shash_ctx(desc->tfm);

	return __chksumc_finup(mctx->key, data, length, out);
}

static int crc32_cra_init(struct crypto_tfm *tfm)
{
	struct chksum_ctx *mctx = crypto_tfm_ctx(tfm);

	mctx->key = 0;
	return 0;
}

static int crc32c_cra_init(struct crypto_tfm *tfm)
{
	struct chksum_ctx *mctx = crypto_tfm_ctx(tfm);

	mctx->key = ~0;
	return 0;
}

static struct shash_alg
	crc32_alg = { .digestsize = CHKSUM_DIGEST_SIZE,
		      .setkey = chksum_setkey,
		      .init = chksum_init,
		      .update = chksum_update,
		      .final = chksum_final,
		      .finup = chksum_finup,
		      .digest = chksum_digest,
		      .descsize = sizeof(struct chksum_desc_ctx),
		      .base = {
			      .cra_name = "crc32",
			      .cra_driver_name = "octeon-crc32",
			      .cra_priority = 300,
			      .cra_flags = CRYPTO_ALG_OPTIONAL_KEY,
			      .cra_blocksize = CHKSUM_BLOCK_SIZE,
			      .cra_alignmask = 3,
			      .cra_ctxsize = sizeof(struct chksum_ctx),
			      .cra_module = THIS_MODULE,
			      .cra_init = crc32_cra_init,
		      } };

static struct shash_alg
	crc32c_alg = { .digestsize = CHKSUM_DIGEST_SIZE,
		       .setkey = chksum_setkey,
		       .init = chksum_init,
		       .update = chksumc_update,
		       .final = chksumc_final,
		       .finup = chksumc_finup,
		       .digest = chksumc_digest,
		       .descsize = sizeof(struct chksum_desc_ctx),
		       .base = {
			       .cra_name = "crc32c",
			       .cra_driver_name = "octeon-crc32c",
			       .cra_priority = 300,
			       .cra_flags = CRYPTO_ALG_OPTIONAL_KEY,
			       .cra_blocksize = CHKSUM_BLOCK_SIZE,
			       .cra_alignmask = 3,
			       .cra_ctxsize = sizeof(struct chksum_ctx),
			       .cra_module = THIS_MODULE,
			       .cra_init = crc32c_cra_init,
		       } };

static int __init crc32_mod_init(void)
{
	int err;

	err = crypto_register_shash(&crc32_alg);

	if (err)
		return err;

	err = crypto_register_shash(&crc32c_alg);

	if (err) {
		crypto_unregister_shash(&crc32_alg);
		return err;
	}

	return 0;
}

static void __exit crc32_mod_exit(void)
{
	crypto_unregister_shash(&crc32_alg);
	crypto_unregister_shash(&crc32c_alg);
}

module_init(crc32_mod_init);
module_exit(crc32_mod_exit);

MODULE_AUTHOR("Sebastian Gottschall <s.gottschall@dd-wrt.com");
MODULE_DESCRIPTION("CRC32 and CRC32C using Octeon HW Crypto");
MODULE_LICENSE("GPL v2");
