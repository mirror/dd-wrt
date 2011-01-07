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
  \defgroup LQ_DEU LQ_DEU_DRIVERS
  \ingroup API
  \brief Lantiq DEU driver module
*/

/**
  \file sha1.c
  \ingroup LQ_DEU
  \brief SHA1 encryption DEU driver file
*/

/**
  \defgroup LQ_SHA1_FUNCTIONS LQ_SHA1_FUNCTIONS
  \ingroup LQ_DEU
  \brief Lantiq DEU sha1 functions
*/


#include <crypto/internal/hash.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/crypto.h>
#include <linux/cryptohash.h>
#include <crypto/sha.h>
#include <linux/types.h>
#include <asm/scatterlist.h>
#include <asm/byteorder.h>
#include "deu.h"

#define SHA1_DIGEST_SIZE	20
#define SHA1_HMAC_BLOCK_SIZE	64

static spinlock_t cipher_lock;

/*
 * \brief SHA1 private structure
*/
struct sha1_ctx {
	u64 count;
	u32 state[5];
	u8 buffer[64];
};

/** \fn static void sha1_transform(u32 *state, const u32 *in)
 *  \ingroup LQ_SHA1_FUNCTIONS
 *  \brief main interface to sha1 hardware
 *  \param state current state
 *  \param in 64-byte block of input
*/
static void sha1_transform(u32 *state, const u32 *in)
{
	int i = 0;
	volatile struct deu_hash *hashs = (struct deu_hash *) HASH_START;
	unsigned long flag;

	CRTCL_SECT_START;

	for (i = 0; i < 16; i++) {
		hashs->MR = in[i];
	};

	/* wait for processing */
	while (hashs->ctrl.BSY) {
		/* this will not take long */
	}

	CRTCL_SECT_END;
}

/** \fn static void sha1_init(struct crypto_tfm *tfm)
 *  \ingroup LQ_SHA1_FUNCTIONS
 *  \brief initialize sha1 hardware
 *  \param tfm linux crypto algo transform
*/
static int sha1_init(struct shash_desc *desc)
{
	struct sha1_ctx *sctx = shash_desc_ctx(desc);

	SHA_HASH_INIT;

	sctx->count = 0;

	return 0;
}

/** \fn static void sha1_update(struct crypto_tfm *tfm, const u8 *data, unsigned int len)
 *  \ingroup LQ_SHA1_FUNCTIONS
 *  \brief on-the-fly sha1 computation
 *  \param tfm linux crypto algo transform
 *  \param data input data
 *  \param len size of input data
*/
static int sha1_update(struct shash_desc *desc, const u8 *data, unsigned int len)
{
	struct sha1_ctx *sctx = shash_desc_ctx(desc);
	unsigned int i, j;

	j = (sctx->count >> 3) & 0x3f;
	sctx->count += len << 3;

	if ((j + len) > 63) {
		memcpy(&sctx->buffer[j], data, (i = 64 - j));
		sha1_transform(sctx->state, (const u32 *)sctx->buffer);
		for (; i + 63 < len; i += 64) {
			sha1_transform(sctx->state, (const u32 *)&data[i]);
		}

		j = 0;
	} else {
		i = 0;
	}

	memcpy(&sctx->buffer[j], &data[i], len - i);

	return 0;
}

/** \fn static void sha1_final(struct crypto_tfm *tfm, u8 *out)
 *  \ingroup LQ_SHA1_FUNCTIONS
 *  \brief compute final sha1 value
 *  \param tfm linux crypto algo transform
 *  \param out final md5 output value
*/
static int sha1_final(struct shash_desc *desc, u8 *out)
{
	struct sha1_ctx *sctx = shash_desc_ctx(desc);
	u32 index, padlen;
	u64 t;
	u8 bits[8] = { 0, };
	static const u8 padding[64] = { 0x80, };
	volatile struct deu_hash *hashs = (struct deu_hash *) HASH_START;
	ulong flag;

	t = sctx->count;
	bits[7] = 0xff & t;
	t >>= 8;
	bits[6] = 0xff & t;
	t >>= 8;
	bits[5] = 0xff & t;
	t >>= 8;
	bits[4] = 0xff & t;
	t >>= 8;
	bits[3] = 0xff & t;
	t >>= 8;
	bits[2] = 0xff & t;
	t >>= 8;
	bits[1] = 0xff & t;
	t >>= 8;
	bits[0] = 0xff & t;

	/* Pad out to 56 mod 64 */
	index = (sctx->count >> 3) & 0x3f;
	padlen = (index < 56) ? (56 - index) : ((64 + 56) - index);
	sha1_update(desc, padding, padlen);

	/* Append length */
	sha1_update(desc, bits, sizeof bits);

	CRTCL_SECT_START;

	*((u32 *) out + 0) = hashs->D1R;
	*((u32 *) out + 1) = hashs->D2R;
	*((u32 *) out + 2) = hashs->D3R;
	*((u32 *) out + 3) = hashs->D4R;
	*((u32 *) out + 4) = hashs->D5R;

	CRTCL_SECT_END;

	/* Wipe context*/
	memset(sctx, 0, sizeof *sctx);

	return 0;
}

static int sha1_export(struct shash_desc *desc, void *out)
{
	struct sha1_ctx *sctx = shash_desc_ctx(desc);

	memcpy(out, sctx, sizeof(*sctx));
	return 0;
}

static int sha1_import(struct shash_desc *desc, const void *in)
{
	struct sha1_ctx *sctx = shash_desc_ctx(desc);

	memcpy(sctx, in, sizeof(*sctx));
	return 0;
}

/*
 * \brief SHA1 function mappings
*/
static struct shash_alg deu_sha1_alg  = {
	.digestsize	= SHA1_DIGEST_SIZE,
	.init		= sha1_init,
	.update		= sha1_update,
	.final		= sha1_final,
	.export		= sha1_export,
	.import		= sha1_import,
	.descsize	= sizeof(struct sha1_ctx),
	.statesize	= sizeof(struct sha1_ctx),
	.base		= {
		.cra_name		= "sha1",
		.cra_driver_name	= "lq_deu-sha1",
		.cra_flags		= CRYPTO_ALG_TYPE_SHASH,
		.cra_blocksize		= SHA1_HMAC_BLOCK_SIZE,
		.cra_module		= THIS_MODULE,
	}
};

/** \fn int lq_deu_init_sha1(void)
 *  \ingroup LQ_SHA1_FUNCTIONS
 *  \brief initialize sha1 driver
*/
int lq_deu_init_sha1(void)
{
	int ret;

	if ((ret = crypto_register_shash(&deu_sha1_alg)))
		goto sha1_err;

	CRTCL_SECT_INIT;

	printk(KERN_NOTICE "Lantiq DEU SHA1 initialized%s.\n",
	       disable_deudma ? "" : " (DMA)");
	return ret;

sha1_err:
	printk(KERN_ERR "Lantiq DEU SHA1 initialization failed!\n");
	return ret;
}

/** \fn void lq_deu_fini_sha1(void)
 *  \ingroup LQ_SHA1_FUNCTIONS
 *  \brief unregister sha1 driver
*/
void lq_deu_fini_sha1(void)
{
	crypto_unregister_shash(&deu_sha1_alg);
}
