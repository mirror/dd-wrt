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
  \brief  Lantiq DEU driver module
*/

/**
  \file md5_hmac.c
  \ingroup LQ_DEU
  \brief MD5-HMAC encryption DEU driver file
*/

/**
 \defgroup LQ_MD5_HMAC_FUNCTIONS LQ_MD5_HMAC_FUNCTIONS
 \ingroup LQ_DEU
 \brief Lantiq md5-hmac driver functions
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
#define MD5_HMAC_DBN_TEMP_SIZE  1024 /* size in dword,
					needed for dbn workaround */

static spinlock_t cipher_lock;

struct md5_hmac_ctx {
	u32 hash[MD5_HASH_WORDS];
	u32 block[MD5_BLOCK_WORDS];
	u64 byte_count;
	u32 dbn;
	u32 temp[MD5_HMAC_DBN_TEMP_SIZE];
};

/** \fn static u32 md5_endian_swap(u32 input)
 *  \ingroup LQ_MD5_HMAC_FUNCTIONS
 *  \brief perform dword level endian swap
 *  \param input value of dword that requires to be swapped
*/
static u32 md5_endian_swap(u32 input)
{
	u8 *ptr = (u8 *)&input;

	return ((ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0]);
}

/** \fn static void md5_hmac_transform(struct crypto_tfm *tfm, u32 const *in)
 *  \ingroup LQ_MD5_HMAC_FUNCTIONS
 *  \brief save input block to context
 *  \param tfm linux crypto algo transform
 *  \param in 64-byte block of input
*/
static void md5_hmac_transform(struct shash_desc *desc, u32 const *in)
{
	struct md5_hmac_ctx *mctx = shash_desc_ctx(desc);

	memcpy(&mctx->temp[mctx->dbn<<4], in, 64); /* dbn workaround */
	mctx->dbn += 1;

	if ( (mctx->dbn<<4) > MD5_HMAC_DBN_TEMP_SIZE )
	{
		printk("MD5_HMAC_DBN_TEMP_SIZE exceeded\n");
	}
}

/** \fn int md5_hmac_setkey(struct crypto_tfm *tfm, const u8 *key, unsigned int keylen)
 *  \ingroup LQ_MD5_HMAC_FUNCTIONS
 *  \brief sets md5 hmac key
 *  \param tfm linux crypto algo transform
 *  \param key input key
 *  \param keylen key length greater than 64 bytes IS NOT SUPPORTED
*/
static int md5_hmac_setkey(struct crypto_shash *tfm,
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

/** \fn void md5_hmac_init(struct crypto_tfm *tfm)
 *  \ingroup LQ_MD5_HMAC_FUNCTIONS
 *  \brief initialize md5 hmac context
 *  \param tfm linux crypto algo transform
*/
static int md5_hmac_init(struct shash_desc *desc)
{
	struct md5_hmac_ctx *mctx = shash_desc_ctx(desc);

	memset(mctx, 0, sizeof(struct md5_hmac_ctx));
	mctx->dbn = 0; /* dbn workaround */
	return 0;
}

/** \fn void md5_hmac_update(struct crypto_tfm *tfm, const u8 *data, unsigned int len)
 *  \ingroup LQ_MD5_HMAC_FUNCTIONS
 *  \brief on-the-fly md5 hmac computation
 *  \param tfm linux crypto algo transform
 *  \param data input data
 *  \param len size of input data
*/
static int md5_hmac_update(struct shash_desc *desc,
			   const u8 *data,
			   unsigned int len)
{
	struct md5_hmac_ctx *mctx = shash_desc_ctx(desc);
	const u32 avail = sizeof(mctx->block) - (mctx->byte_count & 0x3f);

	mctx->byte_count += len;

	if (avail > len) {
		memcpy((char *)mctx->block + (sizeof(mctx->block) - avail),
		       data, len);
		return 0;
	}

	memcpy((char *)mctx->block + (sizeof(mctx->block) - avail),
	       data, avail);

	md5_hmac_transform(desc, mctx->block);
	data += avail;
	len -= avail;

	while (len >= sizeof(mctx->block)) {
		memcpy(mctx->block, data, sizeof(mctx->block));
		md5_hmac_transform(desc, mctx->block);
		data += sizeof(mctx->block);
		len -= sizeof(mctx->block);
	}

	memcpy(mctx->block, data, len);

	return 0;
}

/** \fn void md5_hmac_final(struct crypto_tfm *tfm, u8 *out)
 *  \ingroup LQ_MD5_HMAC_FUNCTIONS
 *  \brief compute final md5 hmac value
 *  \param tfm linux crypto algo transform
 *  \param out final md5 hmac output value
*/
static int md5_hmac_final(struct shash_desc *desc, u8 *out)
{
	struct md5_hmac_ctx *mctx = shash_desc_ctx(desc);
	const unsigned int offset = mctx->byte_count & 0x3f;
	char *p = (char *)mctx->block + offset;
	int padding = 56 - (offset + 1);
	volatile struct deu_hash *hashs = (struct deu_hash *) HASH_START;
	u32 flag;
	int i = 0;
	int dbn;
	u32 *in = &mctx->temp[0];

	*p++ = 0x80;
	if (padding < 0) {
		memset(p, 0x00, padding + sizeof (u64));
		md5_hmac_transform(desc, mctx->block);
		p = (char *)mctx->block;
		padding = 56;
	}

	memset(p, 0, padding);
	/* need to add 512 bit of the IPAD operation */
	mctx->block[14] = md5_endian_swap((mctx->byte_count + 64) << 3);
	mctx->block[15] = 0x00000000;

	md5_hmac_transform(desc, mctx->block);

	CRTCL_SECT_START;

	printk("dbn = %d\n", mctx->dbn);
	hashs->DBN = mctx->dbn;

	/* khs, go, init, ndc, endi, kyue, hmen, md5 */
	*LQ_HASH_CON = 0x0703002D;

	/* wait for processing */
	while (hashs->ctrl.BSY) {
		/* this will not take long */
	}

	for (dbn = 0; dbn < mctx->dbn; dbn++)
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

	return 0;
}

static int md5_hmac_export(struct shash_desc *desc, void *out)
{
	struct md5_hmac_ctx *sctx = shash_desc_ctx(desc);

	memcpy(out, sctx, sizeof(*sctx));
	return 0;
}

static int md5_hmac_import(struct shash_desc *desc, const void *in)
{
	struct md5_hmac_ctx *sctx = shash_desc_ctx(desc);

	memcpy(sctx, in, sizeof(*sctx));
	return 0;
}

/*
 * \brief MD5_HMAC function mappings
*/
static struct shash_alg md5_hmac_alg  = {
	.digestsize	= MD5_DIGEST_SIZE,
	.init		= md5_hmac_init,
	.update		= md5_hmac_update,
	.final		= md5_hmac_final,
	.setkey		= md5_hmac_setkey,
	.export		= md5_hmac_export,
	.import		= md5_hmac_import,
	.descsize	= sizeof(struct md5_hmac_ctx),
	.statesize	= sizeof(struct md5_hmac_ctx),
	.base		= {
		.cra_name		= "hmac(md5)",
		.cra_driver_name	= "lq_deu-md5_hmac",
		.cra_flags		= CRYPTO_ALG_TYPE_SHASH,
		.cra_blocksize		= MD5_HMAC_BLOCK_SIZE,
		.cra_module		= THIS_MODULE,
	}
};

/** \fn int lq_deu_init_md5_hmac(void)
 *  \ingroup LQ_MD5_HMAC_FUNCTIONS
 *  \brief initialize md5 hmac driver
*/
int lq_deu_init_md5_hmac(void)
{
	int ret;

	if ((ret = crypto_register_shash(&md5_hmac_alg)))
		goto md5_hmac_err;

	CRTCL_SECT_INIT;

	printk(KERN_NOTICE "Lantiq DEU MD5_HMAC initialized%s.\n",
	       disable_deudma ? "" : " (DMA)");
	return ret;

md5_hmac_err:
	printk(KERN_ERR "Lantiq DEU MD5_HMAC initialization failed!\n");
	return ret;
}

/** \fn void lq_deu_fini_md5_hmac(void)
 *  \ingroup LQ_MD5_HMAC_FUNCTIONS
 *  \brief unregister md5 hmac driver
*/
void lq_deu_fini_md5_hmac(void)
{
	crypto_unregister_shash(&md5_hmac_alg);
}

