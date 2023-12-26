/*
 * octeon-aes-gcm.c
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
#include "octeon-crypto.h"

#define BIT_SLICED_KEY_MAXSIZE	(128 * (AES_MAXNR - 1) + 2 * AES_BLOCK_SIZE)


#define CVM_AES_RD_RESULT_WR_DATA(in1, in2, out1, out2) \
    asm volatile(\
            ".set noreorder    \n" \
            "dmfc2 %[r1],0x0100\n" \
            "dmfc2 %[r2],0x0101\n" \
            "dmtc2 %[r3],0x010a\n" \
            "dmtc2 %[r4],0x310b\n" \
            ".set reorder      \n" \
            : [r1] "=&d"(in1) , [r2] "=&d"(in2) \
            : [r3] "d"(out1),  [r4] "d"(out2))

static void octeon_ghash_restore(uint16_t poly, uint64_t *h)
{
    write_octeon_64bit_gfm_poly((uint64_t)poly);
    write_octeon_64bit_gfm_mul(h[0], 0);
    write_octeon_64bit_gfm_mul(h[1], 1);
}


static void octeon_ghash_Init(uint16_t poly, uint64_t *h)
{
    octeon_ghash_restore(poly, h);
    write_octeon_64bit_gfm_resinp(0, 0);
    write_octeon_64bit_gfm_resinp(0, 1);
}


static void octeon_ghash_update(const u8 *in)
{
    const uint64_t *bigin = (const uint64_t*)in;
    write_octeon_64bit_gfm_xor0(bigin[0]);
    write_octeon_64bit_gfm_xormul1(bigin[1]);
}


static void octeon_ghash_final(u8 *out, uint64_t authInSz, uint64_t inSz)
{
    uint64_t* bigOut = (uint64_t*)out;

    write_octeon_64bit_gfm_xor0(authInSz * 8);
    write_octeon_64bit_gfm_xormul1(inSz * 8);
    bigOut[0] = read_octeon_64bit_gfm_resinp(0);
    bigOut[1] = read_octeon_64bit_gfm_resinp(1);
}

struct crypto_aes_gcm_ctx {
	u32 key[AES_MAX_KEYLENGTH_U32];
	u32 key_length;
	u64 h[2];
	int keyset;
	uint32_t reg[4];
	uint32_t y0;
};

/* Sets the Octeon key with the key found in the Aes record. */

static int _octeon_aesgcm_set_key(struct crypto_aead *tfm)

{
	struct crypto_aes_gcm_ctx *ctx = crypto_aead_ctx(tfm);
        uint64_t* key = (uint64_t*)ctx->key;

	write_octeon_64bit_aes_key(key[0],0);
	write_octeon_64bit_aes_key(key[1],1);
	write_octeon_64bit_aes_key(key[2],2);
	write_octeon_64bit_aes_key(key[3],3);
	write_octeon_64bit_aes_keylength(ctx->key_length/8 - 1);

        if (!ctx->keyset) {
            write_octeon_64bit_aes_enc0(0);
            write_octeon_64bit_aes_enc1(0);
            ctx->h[0] = read_octeon_64bit_aes_result(0);
            ctx->h[1] = read_octeon_64bit_aes_result(1);
            ctx->keyset = 1;
        }

    return 0;
}

static int octeon_aesgcm_set_key(struct crypto_aead *tfm, const u8 *in_key,
			     unsigned int key_len)

{
	struct crypto_aes_gcm_ctx *ctx = crypto_aead_ctx(tfm);
	memset(ctx->key, 0, sizeof(ctx->key));
	memcpy(ctx->key, in_key, key_len);
	ctx->key_length = key_len;
	return 0;
}


static int octeon_aesgcm_setiv(struct crypto_aead *tfm, const u8 *iv, uint64_t ivSz)
{
    struct crypto_aes_gcm_ctx *ctx = crypto_aead_ctx(tfm);

    int ret = 0;

    if (ctx == NULL || iv == NULL)
        ret = -1;

    if (ret == 0) {
        if (ivSz == 12) {
    	    memset((u8*)ctx->reg, 0, sizeof(ctx->reg));
    	    memcpy((u8*)ctx->reg, iv, ivSz);
            ctx->reg[3] = 1;
        }
        else {
            int blocks, remainder, i;
            u8 aesBlock[16];

            blocks = ivSz / 16;
            remainder = ivSz % 16;

            for (i = 0; i < blocks; i++, iv += 16)
                octeon_ghash_update(iv);

            memset(aesBlock, 0, sizeof(aesBlock));
            for (i = 0; i < remainder; i++)
                aesBlock[i] = iv[i];
            octeon_ghash_update(aesBlock);

            octeon_ghash_final((u8*)ctx->reg, 0, ivSz);
        }

        ctx->y0 = ctx->reg[3];
        ctx->reg[3]++;

        octeon_ghash_Init(0xe100, ctx->h);
    }

    return ret;
}


static int octeon_aesgcm_setaad(struct crypto_aead *tfm, u8* aad, uint64_t aadSz)
{
    struct crypto_aes_gcm_ctx *ctx = crypto_aead_ctx(tfm);
    uint64_t *p;
    u8 aesBlock[16];
    int blocks, remainder, i;

    if (ctx == NULL || (aadSz != 0 && aad == NULL))
        return -1;

    if (aadSz == 0)
        return 0;

    blocks = aadSz / 16;
    remainder = aadSz % 16;

    octeon_ghash_restore(0xe100, ctx->h);

    p = (uint64_t*)aesBlock;

    for (i = 0; i < blocks; i++, aad += 16) {
	octeon_loaduna_int64(p[0], aad, 0);
	octeon_loaduna_int64(p[1], aad, 8);
	write_octeon_64bit_gfm_xor0(p[0]);
	write_octeon_64bit_gfm_xormul1(p[1]);
    }

    memset(aesBlock, 0, sizeof(aesBlock));

    for (i = 0; i < remainder; i++)
        aesBlock[i] = aad[i];

    write_octeon_64bit_gfm_xor0(p[0]);
    write_octeon_64bit_gfm_xormul1(p[1]);

    return 0;
}


static int octeon_aesgcm_setencrypt(struct crypto_aead *tfm, u8* in, u8* out, uint32_t inSz,
        int encrypt)
{
    struct crypto_aes_gcm_ctx *ctx = crypto_aead_ctx(tfm);
    uint32_t i, blocks, remainder;
    u8 aesBlockIn[AES_BLOCK_SIZE];
    u8 aesBlockOut[AES_BLOCK_SIZE];
    uint64_t* pIn;
    uint64_t* pOut;
    uint64_t* pIv;

    if (ctx == NULL || in == NULL || out == NULL)
        return -1;

    pIn = (uint64_t*)aesBlockIn;
    pOut = (uint64_t*)aesBlockOut;
    pIv = (uint64_t*)ctx->reg;

    octeon_prefetch0(in);

    write_octeon_64bit_aes_enc0(pIv[0]);
    write_octeon_64bit_aes_enc1(pIv[1]);

    blocks = inSz / 16;
    remainder = inSz % 16;

    for (i = 0; i < blocks;
            i++, in += 16, out += 16) {
        octeon_prefetch128(in);
        ctx->reg[3]++;

        octeon_loaduna_int64(pIn[0], in, 0);
        octeon_loaduna_int64(pIn[1], in, 8);

        write_octeon_64bit_aes_result_wr_data(pOut[0], pOut[1], pIv[0], pIv[1]);

        if (encrypt) {
            pOut[0] ^= pIn[0];
            pOut[1] ^= pIn[1];
            write_octeon_64bit_gfm_xor0(pOut[0]);
            write_octeon_64bit_gfm_xormul1(pOut[1]);
        }
        else {
            write_octeon_64bit_gfm_xor0(pIn[0]);
    	    write_octeon_64bit_gfm_xormul1(pIn[1]);
            pOut[0] ^= pIn[0];
            pOut[1] ^= pIn[1];
        }
	octeon_storeuna_int64(pOut[0], out, 0);
        octeon_storeuna_int64(pOut[1], out, 8);
    }

    if (remainder > 0) {
        u8 aesBlockMask[AES_BLOCK_SIZE];
        uint64_t* pMask = (uint64_t*)aesBlockMask;

        memset(aesBlockOut, 0, sizeof(aesBlockOut));
        memset(aesBlockMask, 0, sizeof(aesBlockMask));
        for (i = 0; i < remainder; i++) {
            aesBlockIn[i] = in[i];
            aesBlockMask[i] = 0xFF;
        }

        if (encrypt) {
    	    pOut[0] = read_octeon_64bit_aes_result(0);
    	    pOut[1] = read_octeon_64bit_aes_result(1);

            pOut[0] ^= pIn[0];
            pOut[1] ^= pIn[1];

            pOut[0] &= pMask[0];
            pOut[1] &= pMask[1];

            write_octeon_64bit_gfm_xor0(pOut[0]);
    	    write_octeon_64bit_gfm_xormul1(pOut[1]);
        }
        else {
            write_octeon_64bit_gfm_xor0(pIn[0]);
    	    write_octeon_64bit_gfm_xormul1(pIn[1]);

    	    pOut[0] = read_octeon_64bit_aes_result(0);
    	    pOut[1] = read_octeon_64bit_aes_result(1);


            pOut[0] ^= pIn[0];
            pOut[1] ^= pIn[1];

            pOut[0] &= pMask[0];
            pOut[1] &= pMask[1];
        }

        for (i = 0; i < remainder; i++)
            out[i] = aesBlockOut[i];
    }

    return 0;
}


static int octeon_aesgcm_finalize(struct crypto_aead *tfm, uint32_t inSz, uint32_t aadSz,
        u8* tag)
{
    struct crypto_aes_gcm_ctx *ctx = crypto_aead_ctx(tfm);
    uint64_t bigSz;
    uint64_t* pIv;
    uint64_t* pIn;
    uint64_t* pOut;
    uint32_t countSave;
    u8 aesBlockIn[16];
    u8 aesBlockOut[16];

    countSave = ctx->reg[3];
    ctx->reg[3] = ctx->y0;

    pIv = (uint64_t*)ctx->reg;
    write_octeon_64bit_aes_enc0(pIv[0]);
    write_octeon_64bit_aes_enc1(pIv[1]);

    bigSz = (uint64_t)aadSz * 8;
    write_octeon_64bit_gfm_xor0(bigSz);
    bigSz = (uint64_t)inSz * 8;
    write_octeon_64bit_gfm_xormul1(bigSz);

    ctx->reg[3] = countSave;

    pIn = (uint64_t*)aesBlockIn;
    pIn[0] = read_octeon_64bit_aes_result(0);
    pIn[1] = read_octeon_64bit_aes_result(1);

    pOut = (uint64_t*)aesBlockOut;
    pOut[0] = read_octeon_64bit_gfm_resinp(0);
    pOut[1] = read_octeon_64bit_gfm_resinp(1);

    pOut[0] ^= pIn[0];
    pOut[1] ^= pIn[1];

    octeon_storeuna_int64(pOut[0], tag, 0);
    octeon_storeuna_int64(pOut[1], tag, 8);

    return 0;
}


static int _octeon_aesgcm_encrypt(struct crypto_aead *tfm, u8* in, u8* out, uint32_t inSz,
        u8* iv, uint32_t ivSz, u8* aad, uint32_t aadSz, u8* tag)
{
    struct crypto_aes_gcm_ctx *ctx = crypto_aead_ctx(tfm);
    int ret = 0;

    if (ctx == NULL)
	ret = -1;

    if (ret == 0)
        ret = _octeon_aesgcm_set_key(tfm);

    if (ret == 0)
        ret = octeon_aesgcm_setiv(tfm, iv, ivSz);

    if (ret == 0)
        ret = octeon_aesgcm_setaad(tfm, aad, aadSz);

    if (ret == 0)
        ret = octeon_aesgcm_setencrypt(tfm, in, out, inSz, 1);

    if (ret == 0)
        ret = octeon_aesgcm_finalize(tfm, inSz, aadSz, tag);

    return ret;
}


static int _octeon_aesgcm_decrypt(struct crypto_aead *tfm, u8* in, u8* out, uint32_t inSz,
        u8* iv, uint32_t ivSz, u8* aad, uint32_t aadSz, u8* tag)
{
    struct crypto_aes_gcm_ctx *ctx = crypto_aead_ctx(tfm);
    int ret = 0;

    if (ctx == NULL)
        ret = -1;

    if (ret == 0)
        ret = _octeon_aesgcm_set_key(tfm);

    if (ret == 0)
        ret = octeon_aesgcm_setiv(tfm, iv, ivSz);

    if (ret == 0)
        ret = octeon_aesgcm_setaad(tfm, aad, aadSz);

    if (ret == 0)
        ret = octeon_aesgcm_setencrypt(tfm, in, out, inSz, 0);

    if (ret == 0)
        ret = octeon_aesgcm_finalize(tfm, inSz, aadSz, tag);

    return ret;
}

typedef struct {
	__be64 a, b;
} be128;

static int octeon_aesgcm_decrypt(struct aead_request *req)
static int octeon_aesgcm_decrypt(struct blkcipher_desc *desc,
		       struct scatterlist *dst, struct scatterlist *src,
		       unsigned int nbytes)
{
	struct crypto_aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	struct blkcipher_walk walk;
	struct octeon_cop2_state state;
	unsigned long flags;
	int err;
	be128 lengths;
	__be64 *iv;
	__be64 *key	= (__be64*)ctx->key_enc;
	__be64 *dataout;
	__be64 *data;
	u8 *aad;

	blkcipher_walk_init(&walk, dst, src, nbytes);
	err = blkcipher_walk_virt(desc, &walk);
	desc->flags &= ~CRYPTO_TFM_REQ_MAY_SLEEP;
	iv = (__be64*)walk.iv;
	lengths.a = cpu_to_be64(req->assoclen * 8);
	lengths.b = cpu_to_be64(req->cryptlen * 8);
	u8 *tag;
	tag = (u8 *)&lengths;

	flags = octeon_crypto_enable(&state);

	dataout = (__be64*)walk.dst.virt.addr;
	data = (__be64*)walk.src.virt.addr;
	aad = data;
	aad+=req->cryptlen;
	_octeon_aesgcm_decrypt(tfm, data, dataout, walk.nbytes, iv, req->cryptlen, aad, req->assoclen, tag);
	err = blkcipher_walk_done(desc, &walk, nbytes);
	octeon_crypto_disable(&state, flags);

	return err;
}


static int octeon_aesgcm_encrypt(struct blkcipher_desc *desc,
			     struct scatterlist *dst,
			     struct scatterlist *src, unsigned int nbytes)
{
	struct crypto_aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	struct blkcipher_walk walk;
	struct octeon_cop2_state state;
	unsigned long flags;
	int err;
	be128 lengths;
	__be64 *iv;
	__be64 *key	= (__be64*)ctx->key_enc;
	__be64 *dataout;
	__be64 *data;
	u8 *aad;

	blkcipher_walk_init(&walk, dst, src, nbytes);
	err = blkcipher_walk_virt(desc, &walk);
	desc->flags &= ~CRYPTO_TFM_REQ_MAY_SLEEP;
	iv = (__be64*)walk.iv;
	lengths.a = cpu_to_be64(req->assoclen * 8);
	lengths.b = cpu_to_be64(req->cryptlen * 8);
	u8 *tag;
	tag = (u8 *)&lengths;

	flags = octeon_crypto_enable(&state);

	dataout = (__be64*)walk.dst.virt.addr;
	data = (__be64*)walk.src.virt.addr;
	aad = data;
	aad+=req->cryptlen;
	_octeon_aesgcm_encrypt(tfm, data, dataout, walk.nbytes, iv, req->cryptlen, aad, req->assoclen, tag);
	err = blkcipher_walk_done(desc, &walk, nbytes);
	octeon_crypto_disable(&state, flags);
	return 0;
}

static struct aead_alg ccm_aes_alg = {
	.base = {
		.cra_name		= "gcm(aes)",
		.cra_driver_name	= "octeon-gcm-aes",
		.cra_priority		= 300,
		.cra_blocksize		= 1,
		.cra_ctxsize		= sizeof(struct crypto_aes_gcm_ctx),
		.cra_alignmask		= 7,
		.cra_module		= THIS_MODULE,
	},
	.ivsize		= AES_BLOCK_SIZE,
	.maxauthsize	= AES_BLOCK_SIZE,
	.setkey		= octeon_aesgcm_set_key,
	.setauthsize	= octeon_aesgcm_setauthsize,
	.encrypt	= octeon_aesgcm_encrypt,
	.decrypt	= octeon_aesgcm_decrypt,
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

