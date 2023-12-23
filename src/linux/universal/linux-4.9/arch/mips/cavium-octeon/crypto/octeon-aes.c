#include <crypto/aes.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/crypto.h>
#include <asm/byteorder.h>
#include "octeon-crypto.h"


/**
 * crypto_aes_set_key - Set the AES key.
 * @tfm:	The %crypto_tfm that is used in the context.
 * @in_key:	The input key.
 * @key_len:	The size of the key.
 *
 * Returns 0 on success, on failure the %CRYPTO_TFM_RES_BAD_KEY_LEN flag in tfm
 * is set. The function uses crypto_aes_expand_key() to expand the key.
 * &crypto_aes_ctx _must_ be the private data embedded in @tfm which is
 * retrieved with crypto_tfm_ctx().
 */
static int octeon_crypto_aes_set_key(struct crypto_tfm *tfm, const u8 *in_key,
		unsigned int key_len)
{
	struct crypto_aes_ctx *ctx = crypto_tfm_ctx(tfm);
	memset(ctx->key_enc, 0, sizeof(ctx->key_enc));
	memcpy(ctx->key_enc, in_key, key_len);
	ctx->key_length = key_len;
	return 0;
}

static void octeon_aes_encrypt(struct crypto_tfm *tfm, u8 *out, const u8 *in)
{
	struct octeon_cop2_state state;
	unsigned long flags;
	struct crypto_aes_ctx *ctx = crypto_tfm_ctx(tfm);
	__be64 *data = (__be64*)in;
	__be64 *dataout = (__be64*)out;
	__be64 *key	= (__be64*)ctx->key_enc;
	flags = octeon_crypto_enable(&state);
	write_octeon_64bit_aes_key(key[0],0);
	write_octeon_64bit_aes_key(key[1],1);
	write_octeon_64bit_aes_key(key[2],2);
	write_octeon_64bit_aes_key(key[3],3);
	write_octeon_64bit_aes_keylength(ctx->key_length/8 - 1);

        write_octeon_64bit_aes_enc0(*data++);
        write_octeon_64bit_aes_enc1(*data);
        *dataout++ = read_octeon_64bit_aes_result(0);
        *dataout = read_octeon_64bit_aes_result(1);

	octeon_crypto_disable(&state, flags);
}


static void octeon_aes_decrypt(struct crypto_tfm *tfm, u8 *out, const u8 *in)
{
	struct octeon_cop2_state state;
	unsigned long flags;
	struct crypto_aes_ctx *ctx = crypto_tfm_ctx(tfm);
	__be64 *data = (__be64*)in;
	__be64 *dataout = (__be64*)out;
	__be64 *key	= (__be64*)ctx->key_enc;


	flags = octeon_crypto_enable(&state);
	write_octeon_64bit_aes_key(key[0],0);
	write_octeon_64bit_aes_key(key[1],1);
	write_octeon_64bit_aes_key(key[2],2);
	write_octeon_64bit_aes_key(key[3],3);
	write_octeon_64bit_aes_keylength(ctx->key_length/8 - 1);

        write_octeon_64bit_aes_dec0(*data++);
        write_octeon_64bit_aes_dec1(*data);
        *dataout++ = read_octeon_64bit_aes_result(0);
        *dataout = read_octeon_64bit_aes_result(1);
	octeon_crypto_disable(&state, flags);
}

static struct crypto_alg aes_alg = {
	.cra_name		=	"aes",
	.cra_driver_name	=	"octeon-aes",
	.cra_priority		=	300,
	.cra_flags		=	CRYPTO_ALG_TYPE_CIPHER,
	.cra_blocksize		=	AES_BLOCK_SIZE,
	.cra_ctxsize		=	sizeof(struct crypto_aes_ctx),
	.cra_alignmask		=	3,
	.cra_module		=	THIS_MODULE,
	.cra_u			=	{
		.cipher = {
			.cia_min_keysize	=	AES_MIN_KEY_SIZE,
			.cia_max_keysize	=	AES_MAX_KEY_SIZE,
			.cia_setkey		=	octeon_crypto_aes_set_key,
			.cia_encrypt		=	octeon_aes_encrypt,
			.cia_decrypt		=	octeon_aes_decrypt
		}
	}
};

static int __init aes_init(void)
{
	return crypto_register_alg(&aes_alg);
}

static void __exit aes_fini(void)
{
	crypto_unregister_alg(&aes_alg);
}

module_init(aes_init);
module_exit(aes_fini);

MODULE_DESCRIPTION("Rijndael (AES) Cipher Algorithm");
MODULE_LICENSE("Dual BSD/GPL");
