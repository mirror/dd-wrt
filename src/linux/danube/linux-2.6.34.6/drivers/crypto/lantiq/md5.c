/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2010 Ralph Hempel <ralph.hempel@lantiq.com>
 *   Copyright (C) 2009 Mohammad Firdaus
 */

/**
  \defgroup    LQ_DEU LQ_DEU_DRIVERS
  \ingroup API
  \brief Lantiq DEU driver module
*/

/**
  \file md5.c
  \ingroup LQ_DEU
  \brief MD5 encryption DEU driver file
*/

/**
  \defgroup LQ_MD5_FUNCTIONS LQ_MD5_FUNCTIONS
  \ingroup LQ_DEU
  \brief Lantiq DEU MD5 functions
*/

#include <crypto/internal/hash.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/crypto.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include "deu.h"

#define MD5_DIGEST_SIZE     16
#define MD5_HMAC_BLOCK_SIZE 64
#define MD5_BLOCK_WORDS     16
#define MD5_HASH_WORDS      4

static spinlock_t cipher_lock;

struct md5_ctx {
	u32 hash[MD5_HASH_WORDS];
	u32 block[MD5_BLOCK_WORDS];
	u64 byte_count;
};

/** \fn static u32 md5_endian_swap(u32 input)
 *  \ingroup LQ_MD5_FUNCTIONS
 *  \brief perform dword level endian swap
 *  \param input value of dword that requires to be swapped
*/
static u32 md5_endian_swap(u32 input)
{
	u8 *ptr = (u8 *)&input;

	return ((ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0]);
}

/** \fn static void md5_transform(u32 *hash, u32 const *in)
 *  \ingroup LQ_MD5_FUNCTIONS
 *  \brief main interface to md5 hardware
 *  \param hash current hash value
 *  \param in 64-byte block of input
*/
static void md5_transform(u32 *hash, u32 const *in)
{
	int i;
	volatile struct deu_hash *hashs = (struct deu_hash *) HASH_START;
	ulong flag;

	CRTCL_SECT_START;

	for (i = 0; i < 16; i++) {
		hashs->MR = md5_endian_swap(in[i]);
	};

	/* wait for processing */
	while (hashs->ctrl.BSY) {
		/* this will not take long */
	}

	CRTCL_SECT_END;
}

/** \fn static inline void md5_transform_helper(struct md5_ctx *ctx)
 *  \ingroup LQ_MD5_FUNCTIONS
 *  \brief interfacing function for md5_transform()
 *  \param ctx crypto context
*/
static inline void md5_transform_helper(struct md5_ctx *ctx)
{
	/* le32_to_cpu_array(ctx->block, sizeof(ctx->block) / sizeof(u32)); */
	md5_transform(ctx->hash, ctx->block);
}

/** \fn static void md5_init(struct crypto_tfm *tfm)
 *  \ingroup LQ_MD5_FUNCTIONS
 *  \brief initialize md5 hardware
 *  \param tfm linux crypto algo transform
*/
static int md5_init(struct shash_desc *desc)
{
	struct md5_ctx *mctx = shash_desc_ctx(desc);
	volatile struct deu_hash *hash = (struct deu_hash *) HASH_START;

	hash->ctrl.SM = 1;
	hash->ctrl.ALGO = 1;    /* 1 = md5  0 = sha1 */
	hash->ctrl.INIT = 1;    /* Initialize the hash operation by writing
				       a '1' to the INIT bit. */

	mctx->byte_count = 0;

	return 0;
}

/** \fn static void md5_update(struct crypto_tfm *tfm, const u8 *data, unsigned int len)
 *  \ingroup LQ_MD5_FUNCTIONS
 *  \brief on-the-fly md5 computation
 *  \param tfm linux crypto algo transform
 *  \param data input data
 *  \param len size of input data
*/
static int md5_update(struct shash_desc *desc, const u8 *data, unsigned int len)
{
	struct md5_ctx *mctx = shash_desc_ctx(desc);
	const u32 avail = sizeof(mctx->block) - (mctx->byte_count & 0x3f);

	mctx->byte_count += len;

	if (avail > len) {
		memcpy((char *)mctx->block + (sizeof(mctx->block) - avail),
		       data, len);
		return 0;
	}

	memcpy((char *)mctx->block + (sizeof(mctx->block) - avail),
	       data, avail);

	md5_transform_helper(mctx);
	data += avail;
	len -= avail;

	while (len >= sizeof(mctx->block)) {
		memcpy(mctx->block, data, sizeof(mctx->block));
		md5_transform_helper(mctx);
		data += sizeof(mctx->block);
		len -= sizeof(mctx->block);
	}

	memcpy(mctx->block, data, len);

	return 0;
}

/** \fn static void md5_final(struct crypto_tfm *tfm, u8 *out)
 *  \ingroup LQ_MD5_FUNCTIONS
 *  \brief compute final md5 value
 *  \param tfm linux crypto algo transform
 *  \param out final md5 output value
*/
static int md5_final(struct shash_desc *desc, u8 *out)
{
	struct md5_ctx *mctx = shash_desc_ctx(desc);
	const unsigned int offset = mctx->byte_count & 0x3f;
	char *p = (char *)mctx->block + offset;
	int padding = 56 - (offset + 1);
	volatile struct deu_hash *hashs = (struct deu_hash *) HASH_START;
	unsigned long flag;

	*p++ = 0x80;
	if (padding < 0) {
		memset(p, 0x00, padding + sizeof (u64));
		md5_transform_helper(mctx);
		p = (char *)mctx->block;
		padding = 56;
	}

	memset(p, 0, padding);
	mctx->block[14] = md5_endian_swap(mctx->byte_count << 3);
	mctx->block[15] = md5_endian_swap(mctx->byte_count >> 29);

#if 0
	le32_to_cpu_array(mctx->block, (sizeof(mctx->block) -
					sizeof(u64)) / sizeof(u32));
#endif

	md5_transform(mctx->hash, mctx->block);

	CRTCL_SECT_START;

	*((u32 *) out + 0) = md5_endian_swap(hashs->D1R);
	*((u32 *) out + 1) = md5_endian_swap(hashs->D2R);
	*((u32 *) out + 2) = md5_endian_swap(hashs->D3R);
	*((u32 *) out + 3) = md5_endian_swap(hashs->D4R);

	CRTCL_SECT_END;

	/* Wipe context */
	memset(mctx, 0, sizeof(*mctx));

	return 0;
}

static int md5_export(struct shash_desc *desc, void *out)
{
	struct md5_ctx *sctx = shash_desc_ctx(desc);

	memcpy(out, sctx, sizeof(*sctx));
	return 0;
}

static int md5_import(struct shash_desc *desc, const void *in)
{
	struct md5_ctx *sctx = shash_desc_ctx(desc);

	memcpy(sctx, in, sizeof(*sctx));
	return 0;
}

/*
 * \brief MD5 function mappings
*/
static struct shash_alg md5_alg  = {
	.digestsize	= MD5_DIGEST_SIZE,
	.init		= md5_init,
	.update		= md5_update,
	.final		= md5_final,
	.export		= md5_export,
	.import		= md5_import,
	.descsize	= sizeof(struct md5_ctx),
	.statesize	= sizeof(struct md5_ctx),
	.base		= {
		.cra_name		= "md5",
		.cra_driver_name	= "lq_deu-md5",
		.cra_flags		= CRYPTO_ALG_TYPE_SHASH,
		.cra_blocksize		= MD5_HMAC_BLOCK_SIZE,
		.cra_module		= THIS_MODULE,
	}
};

/** \fn int lq_deu_init_md5(void)
 *  \ingroup LQ_MD5_FUNCTIONS
 *  \brief initialize md5 driver
*/
int lq_deu_init_md5(void)
{
	int ret;

	if ((ret = crypto_register_shash(&md5_alg)))
		goto md5_err;

	CRTCL_SECT_INIT;

	printk(KERN_NOTICE "Lantiq DEU MD5 initialized%s.\n",
	       disable_deudma ? "" : " (DMA)");
	return ret;

md5_err:
	printk(KERN_ERR "Lantiq DEU MD5 initialization failed!\n");
	return ret;
}

/** \fn void lq_deu_fini_md5(void)
  * \ingroup LQ_MD5_FUNCTIONS
  * \brief unregister md5 driver
*/

void lq_deu_fini_md5(void)
{
	crypto_unregister_shash(&md5_alg);
}

