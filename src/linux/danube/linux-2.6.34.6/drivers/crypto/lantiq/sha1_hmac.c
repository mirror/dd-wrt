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
  \file sha1_hmac.c
  \ingroup LQ_DEU
  \brief SHA1-HMAC DEU driver file
*/

/**
  \defgroup LQ_SHA1_HMAC_FUNCTIONS LQ_SHA1_HMAC_FUNCTIONS
  \ingroup LQ_DEU
  \brief Lantiq sha1 hmac functions
*/


#include <crypto/internal/hash.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/crypto.h>
#include <linux/cryptohash.h>
#include <linux/types.h>
#include <asm/scatterlist.h>
#include <asm/byteorder.h>
#include <linux/delay.h>
#include "deu.h"

#ifdef CONFIG_CRYPTO_DEV_LANTIQ_SHA1_HMAC

#define SHA1_DIGEST_SIZE    20
#define SHA1_HMAC_BLOCK_SIZE    64
/* size in dword, needed for dbn workaround */
#define SHA1_HMAC_DBN_TEMP_SIZE 1024

static spinlock_t cipher_lock;

struct sha1_hmac_ctx {
	u64 count;
	u32 state[5];
	u8 buffer[64];
	u32 dbn;
	u32 temp[SHA1_HMAC_DBN_TEMP_SIZE];
};

/** \fn static void sha1_hmac_transform(struct crypto_tfm *tfm, u32 const *in)
 *  \ingroup LQ_SHA1_HMAC_FUNCTIONS
 *  \brief save input block to context
 *  \param tfm linux crypto algo transform
 *  \param in 64-byte block of input
*/
static void sha1_hmac_transform(struct shash_desc *desc, u32 const *in)
{
	struct sha1_hmac_ctx *sctx = shash_desc_ctx(desc);

	memcpy(&sctx->temp[sctx->dbn<<4], in, 64); /* dbn workaround */
	sctx->dbn += 1;

	if ((sctx->dbn<<4) > SHA1_HMAC_DBN_TEMP_SIZE) {
		printk("SHA1_HMAC_DBN_TEMP_SIZE exceeded\n");
	}
}

/** \fn int sha1_hmac_setkey(struct crypto_tfm *tfm, const u8 *key, unsigned int keylen)
 *  \ingroup LQ_SHA1_HMAC_FUNCTIONS
 *  \brief sets sha1 hmac key
 *  \param tfm linux crypto algo transform
 *  \param key input key
 *  \param keylen key length greater than 64 bytes IS NOT SUPPORTED
*/
static int sha1_hmac_setkey(struct crypto_shash *tfm,
			    const u8 *key,
			    unsigned int keylen)
{
	volatile struct deu_hash *hash = (struct deu_hash *) HASH_START;
	int i, j;
	u32 *in_key = (u32 *)key;

	hash->KIDX = 0x80000000; /* reset all 16 words of the key to '0' */
	asm("sync");

	j = 0;
	for (i = 0; i < keylen; i+=4)
	{
		hash->KIDX = j;
		asm("sync");
		hash->KEY = *((u32 *) in_key + j);
		j++;
	}

	return 0;
}

static int sha1_hmac_export(struct shash_desc *desc, void *out)
{
	struct sha1_hmac_ctx *sctx = shash_desc_ctx(desc);

	memcpy(out, sctx, sizeof(*sctx));
	return 0;
}

static int sha1_hmac_import(struct shash_desc *desc, const void *in)
{
	struct sha1_hmac_ctx *sctx = shash_desc_ctx(desc);

	memcpy(sctx, in, sizeof(*sctx));
	return 0;
}

/** \fn void sha1_hmac_init(struct crypto_tfm *tfm)
 *  \ingroup LQ_SHA1_HMAC_FUNCTIONS
 *  \brief initialize sha1 hmac context
 *  \param tfm linux crypto algo transform
*/
static int sha1_hmac_init(struct shash_desc *desc)
{
	struct sha1_hmac_ctx *sctx = shash_desc_ctx(desc);

	memset(sctx, 0, sizeof(struct sha1_hmac_ctx));
	sctx->dbn = 0; /* dbn workaround */

	return 0;
}

/** \fn static void sha1_hmac_update(struct crypto_tfm *tfm, const u8 *data, unsigned int len)
 *  \ingroup LQ_SHA1_HMAC_FUNCTIONS
 *  \brief on-the-fly sha1 hmac computation
 *  \param tfm linux crypto algo transform
 *  \param data input data
 *  \param len size of input data
*/
static int sha1_hmac_update(struct shash_desc *desc, const u8 *data,
			    unsigned int len)
{
	struct sha1_hmac_ctx *sctx = shash_desc_ctx(desc);
	unsigned int i, j;

	j = (sctx->count >> 3) & 0x3f;
	sctx->count += len << 3;
	/* printk("sctx->count = %d\n", (sctx->count >> 3)); */

	if ((j + len) > 63) {
		memcpy(&sctx->buffer[j], data, (i = 64 - j));
		sha1_hmac_transform(desc, (const u32 *)sctx->buffer);
		for (; i + 63 < len; i += 64) {
			sha1_hmac_transform(desc, (const u32 *)&data[i]);
		}

		j = 0;
	} else {
		i = 0;
	}

	memcpy(&sctx->buffer[j], &data[i], len - i);

	return 0;
}

/** \fn static void sha1_hmac_final(struct crypto_tfm *tfm, u8 *out)
 *  \ingroup LQ_SHA1_HMAC_FUNCTIONS
 *  \brief ompute final sha1 hmac value
 *  \param tfm linux crypto algo transform
 *  \param out final sha1 hmac output value
*/
static int sha1_hmac_final(struct shash_desc *desc, u8 *out)
{
	struct sha1_hmac_ctx *sctx = shash_desc_ctx(desc);
	u32 index, padlen;
	u64 t;
	u8 bits[8] = { 0, };
	static const u8 padding[64] = { 0x80, };
	volatile struct deu_hash *hashs = (struct deu_hash *) HASH_START;
	ulong flag;
	int i = 0;
	int dbn;
	u32 *in = &sctx->temp[0];

	t = sctx->count + 512; /* need to add 512 bit of the IPAD operation */
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
	sha1_hmac_update(desc, padding, padlen);

	/* Append length */
	sha1_hmac_update(desc, bits, sizeof bits);

	CRTCL_SECT_START;

	hashs->DBN = sctx->dbn;

	/* for vr9 change, ENDI = 1 */
	*LQ_HASH_CON = HASH_CON_VALUE;

	/* wait for processing */
	while (hashs->ctrl.BSY) {
		/* this will not take long */
	}

	for (dbn = 0; dbn < sctx->dbn; dbn++)
	{
		for (i = 0; i < 16; i++) {
			hashs->MR = in[i];
		};

		hashs->ctrl.GO = 1;
		asm("sync");

		/* wait for processing */
		while (hashs->ctrl.BSY) {
			/* this will not take long */
		}

		in += 16;

		return 0;
	}

#if 1
	/* wait for digest ready */
	while (! hashs->ctrl.DGRY) {
		/* this will not take long */
	}
#endif

	*((u32 *) out + 0) = hashs->D1R;
	*((u32 *) out + 1) = hashs->D2R;
	*((u32 *) out + 2) = hashs->D3R;
	*((u32 *) out + 3) = hashs->D4R;
	*((u32 *) out + 4) = hashs->D5R;

	CRTCL_SECT_END;
}

/*
 * \brief SHA1-HMAC function mappings
*/
static struct shash_alg sha1_hmac_alg  = {
	.digestsize	= SHA1_DIGEST_SIZE,
	.init		= sha1_hmac_init,
	.update		= sha1_hmac_update,
	.final		= sha1_hmac_final,
	.export		= sha1_hmac_export,
	.import		= sha1_hmac_import,
	.setkey		= sha1_hmac_setkey,
	.descsize	= sizeof(struct sha1_hmac_ctx),
	.statesize	= sizeof(struct sha1_hmac_ctx),
	.base		= {
		.cra_name		= "hmac(sha1)",
		.cra_driver_name	= "lq_deu-sha1_hmac",
		.cra_flags		= CRYPTO_ALG_TYPE_SHASH,
		.cra_blocksize		= SHA1_HMAC_BLOCK_SIZE,
		.cra_module		= THIS_MODULE,
	}
};

/** \fn int lq_deu_init_sha1_hmac(void)
 *  \ingroup LQ_SHA1_HMAC_FUNCTIONS
 *  \brief initialize sha1 hmac driver
*/
int lq_deu_init_sha1_hmac(void)
{
	int ret;

	if ((ret = crypto_register_shash(&sha1_hmac_alg)))
		goto sha1_err;

	CRTCL_SECT_INIT;

	printk(KERN_NOTICE "Lantiq DEU SHA1_HMAC initialized%s.\n",
	       disable_deudma ? "" : " (DMA)");
	return ret;

sha1_err:
	printk(KERN_ERR "Lantiq DEU SHA1_HMAC initialization failed!\n");
	return ret;
}

/** \fn void lq_deu_fini_sha1_hmac(void)
 *  \ingroup LQ_SHA1_HMAC_FUNCTIONS
 *  \brief unregister sha1 hmac driver
*/
void lq_deu_fini_sha1_hmac(void)
{
	crypto_unregister_shash(&sha1_hmac_alg);
}

#endif
