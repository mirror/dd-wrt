/* Copyright (c) 2015-2018 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 */

/**
 * nss_cryptoapi.c
 * 	Interface to communicate Native Linux crypto framework specific data
 * 	to Crypto core specific data
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/random.h>
#include <asm/scatterlist.h>
#include <linux/moduleparam.h>
#include <linux/spinlock.h>
#include <asm/cmpxchg.h>
#include <linux/delay.h>
#include <linux/crypto.h>
#include <linux/rtnetlink.h>
#include <linux/debugfs.h>

#include <crypto/ctr.h>
#include <crypto/des.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <crypto/hash.h>
#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/authenc.h>
#include <crypto/scatterwalk.h>
#include <crypto/internal/hash.h>

#include <nss_api_if.h>
#include <nss_crypto_if.h>
#include <nss_cfi_if.h>
#include "nss_cryptoapi_private.h"

struct nss_cryptoapi gbl_ctx;

bool enable_ahash = false;
module_param(enable_ahash, bool, S_IRUGO);
MODULE_PARM_DESC(enable_ahash, "Enable Cryptoapi AHASH support");

/*
 * crypto_alg structure initialization
 *	AEAD (Cipher and Authentication) and ABLK are supported by core crypto driver.
 */
static struct crypto_alg cryptoapi_algs[] = {
	{	/* sha1, aes-cbc */
		.cra_name       = "authenc(hmac(sha1),cbc(aes))",
		.cra_driver_name = "nss-hmac-sha1-cbc-aes",
		.cra_priority   = 10000,
		.cra_flags      = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG | CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize  = AES_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_aead_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_aead_init,
		.cra_exit       = nss_cryptoapi_aead_exit,
		.cra_u          = {
			.aead = {
				.ivsize         = AES_BLOCK_SIZE,
				.maxauthsize    = SHA1_DIGEST_SIZE,
				.setkey = nss_cryptoapi_aead_aes_setkey,
				.setauthsize = nss_cryptoapi_aead_setauthsize,
				.encrypt = nss_cryptoapi_aead_aes_encrypt,
				.decrypt = nss_cryptoapi_aead_aes_decrypt,
				.givencrypt = nss_cryptoapi_aead_aes_geniv_encrypt,
				.geniv = "<built-in>",
			}
		}
	},
	{	/* sha1, rfc3686-aes-ctr */
		.cra_name       = "authenc(hmac(sha1),rfc3686(ctr(aes)))",
		.cra_driver_name = "nss-hmac-sha1-rfc3686-ctr-aes",
		.cra_priority   = 10000,
		.cra_flags      = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG | CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize  = AES_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_aead_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_aead_init,
		.cra_exit       = nss_cryptoapi_aead_exit,
		.cra_u          = {
			.aead = {
				.ivsize         = CTR_RFC3686_IV_SIZE,
				.maxauthsize    = SHA1_DIGEST_SIZE,
				.setkey = nss_cryptoapi_aead_aes_setkey,
				.setauthsize = nss_cryptoapi_aead_setauthsize,
				.encrypt = nss_cryptoapi_aead_aes_encrypt,
				.decrypt = nss_cryptoapi_aead_aes_decrypt,
				.givencrypt = nss_cryptoapi_aead_aes_geniv_encrypt,
				.geniv = "<built-in>",
			}
		}
	},
	{	/* sha1, 3des */
		.cra_name       = "authenc(hmac(sha1),cbc(des3_ede))",
		.cra_driver_name = "nss-hmac-sha1-cbc-3des",
		.cra_priority   = 300,
		.cra_flags      = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG,
		.cra_blocksize  = DES3_EDE_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_aead_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_aead_init,
		.cra_exit       = nss_cryptoapi_aead_exit,
		.cra_u          = {
			.aead = {
				.ivsize         = DES3_EDE_BLOCK_SIZE,
				.maxauthsize    = SHA1_DIGEST_SIZE,
				.setkey = nss_cryptoapi_sha1_3des_setkey,
				.setauthsize = nss_cryptoapi_aead_setauthsize,
				.encrypt = nss_cryptoapi_sha1_3des_encrypt,
				.decrypt = nss_cryptoapi_sha1_3des_decrypt,
				.givencrypt = nss_cryptoapi_sha1_3des_geniv_encrypt,
				.geniv = "<built-in>",
			}
		}
	},
	{	/* sha256, aes-cbc */
		.cra_name       = "authenc(hmac(sha256),cbc(aes))",
		.cra_driver_name = "nss-hmac-sha256-cbc-aes",
		.cra_priority   = 10000,
		.cra_flags      = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG | CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize  = AES_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_aead_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_aead_init,
		.cra_exit       = nss_cryptoapi_aead_exit,
		.cra_u          = {
			.aead = {
				.ivsize         = AES_BLOCK_SIZE,
				.maxauthsize    = SHA256_DIGEST_SIZE,
				.setkey = nss_cryptoapi_aead_aes_setkey,
				.setauthsize = nss_cryptoapi_aead_setauthsize,
				.encrypt = nss_cryptoapi_aead_aes_encrypt,
				.decrypt = nss_cryptoapi_aead_aes_decrypt,
				.givencrypt = nss_cryptoapi_aead_aes_geniv_encrypt,
				.geniv = "<built-in>",
			}
		}
	},
	{	/* sha256, rfc3686-aes-ctr */
		.cra_name       = "authenc(hmac(sha256),rfc3686(ctr(aes)))",
		.cra_driver_name = "nss-hmac-sha256-rfc3686-ctr-aes",
		.cra_priority   = 10000,
		.cra_flags      = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG | CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize  = AES_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_aead_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_aead_init,
		.cra_exit       = nss_cryptoapi_aead_exit,
		.cra_u          = {
			.aead = {
				.ivsize         = CTR_RFC3686_IV_SIZE,
				.maxauthsize    = SHA256_DIGEST_SIZE,
				.setkey = nss_cryptoapi_aead_aes_setkey,
				.setauthsize = nss_cryptoapi_aead_setauthsize,
				.encrypt = nss_cryptoapi_aead_aes_encrypt,
				.decrypt = nss_cryptoapi_aead_aes_decrypt,
				.givencrypt = nss_cryptoapi_aead_aes_geniv_encrypt,
				.geniv = "<built-in>",
			}
		}
	},
	{	/* sha256, 3des */
		.cra_name       = "authenc(hmac(sha256),cbc(des3_ede))",
		.cra_driver_name = "nss-hmac-sha256-cbc-3des",
		.cra_priority   = 300,
		.cra_flags      = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG,
		.cra_blocksize  = DES3_EDE_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_aead_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_aead_init,
		.cra_exit       = nss_cryptoapi_aead_exit,
		.cra_u          = {
			.aead = {
				.ivsize         = DES3_EDE_BLOCK_SIZE,
				.maxauthsize    = SHA256_DIGEST_SIZE,
				.setkey = nss_cryptoapi_sha256_3des_setkey,
				.setauthsize = nss_cryptoapi_aead_setauthsize,
				.encrypt = nss_cryptoapi_sha256_3des_encrypt,
				.decrypt = nss_cryptoapi_sha256_3des_decrypt,
				.givencrypt = nss_cryptoapi_sha256_3des_geniv_encrypt,
				.geniv = "<built-in>",
			}
		}
	},
	{
		.cra_name       = "cbc(aes)",
		.cra_driver_name = "nss-cbc-aes",
		.cra_priority   = 10000,
		.cra_flags      = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG | CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize  = AES_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_ablkcipher_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_ablkcipher_init,
		.cra_exit       = nss_cryptoapi_ablkcipher_exit,
		.cra_u          = {
			.ablkcipher = {
				.ivsize         = AES_BLOCK_SIZE,
				.min_keysize    = AES_MIN_KEY_SIZE,
				.max_keysize    = AES_MAX_KEY_SIZE,
				.setkey         = nss_cryptoapi_ablk_aes_setkey,
				.encrypt        = nss_cryptoapi_ablk_aes_encrypt,
				.decrypt        = nss_cryptoapi_ablk_aes_decrypt,
			},
		},
	},
	{
		.cra_name       = "rfc3686(ctr(aes))",
		.cra_driver_name = "nss-rfc3686-ctr-aes",
		.cra_priority   = 10000,
		.cra_flags      = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG | CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize  = AES_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_ablkcipher_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_ablkcipher_init,
		.cra_exit       = nss_cryptoapi_ablkcipher_exit,
		.cra_u          = {
			.ablkcipher = {
				.ivsize         = CTR_RFC3686_IV_SIZE,
				.geniv          = "seqiv",
				.min_keysize    = AES_MIN_KEY_SIZE + CTR_RFC3686_NONCE_SIZE,
				.max_keysize    = AES_MAX_KEY_SIZE + CTR_RFC3686_NONCE_SIZE,
				.setkey         = nss_cryptoapi_ablk_aes_setkey,
				.encrypt        = nss_cryptoapi_ablk_aes_encrypt,
				.decrypt        = nss_cryptoapi_ablk_aes_decrypt,
			},
		},
	},
	{
		.cra_name       = "cbc(des3_ede)",
		.cra_driver_name = "nss-cbc-3des",
		.cra_priority   = 300,
		.cra_flags      = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG,
		.cra_blocksize  = DES3_EDE_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_ablkcipher_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_ablkcipher_init,
		.cra_exit       = nss_cryptoapi_ablkcipher_exit,
		.cra_u          = {
			.ablkcipher = {
				.ivsize         = DES3_EDE_BLOCK_SIZE,
				.min_keysize    = DES3_EDE_KEY_SIZE,
				.max_keysize    = DES3_EDE_KEY_SIZE,
				.setkey         = nss_cryptoapi_3des_cbc_setkey,
				.encrypt        = nss_cryptoapi_3des_cbc_encrypt,
				.decrypt        = nss_cryptoapi_3des_cbc_decrypt,
			},
		},
	},
};

/*
 * AHASH algorithms
 */
static struct ahash_alg cryptoapi_ahash_algs[] = {
	{
		.init   = nss_cryptoapi_ahash_init,
		.update = nss_cryptoapi_ahash_update,
		.final  = nss_cryptoapi_ahash_final,
		.export = nss_cryptoapi_ahash_export,
		.import = nss_cryptoapi_ahash_import,
		.digest = nss_cryptoapi_ahash_digest,
		.setkey = nss_cryptoapi_ahash_setkey,
		.halg   = {
			.digestsize = SHA1_DIGEST_SIZE,
			.statesize  = sizeof(struct sha1_state),
			.base       = {
				.cra_name        = "hmac(sha1)",
				.cra_driver_name = "nss-hmac-sha1",
				.cra_priority    = 300,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG,
				.cra_blocksize   = SHA1_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct nss_cryptoapi_ctx),
				.cra_alignmask   = 0,
				.cra_type        = &crypto_ahash_type,
				.cra_module      = THIS_MODULE,
				.cra_init        = nss_cryptoapi_ahash_cra_init,
				.cra_exit        = nss_cryptoapi_ahash_cra_exit,
			},
		},
	},
	{
		.init   = nss_cryptoapi_ahash_init,
		.update = nss_cryptoapi_ahash_update,
		.final  = nss_cryptoapi_ahash_final,
		.export = nss_cryptoapi_ahash_export,
		.import = nss_cryptoapi_ahash_import,
		.digest = nss_cryptoapi_ahash_digest,
		.setkey = nss_cryptoapi_ahash_setkey,
		.halg   = {
			.digestsize = SHA256_DIGEST_SIZE,
			.statesize  = sizeof(struct sha256_state),
			.base       = {
				.cra_name        = "hmac(sha256)",
				.cra_driver_name = "nss-hmac-sha256",
				.cra_priority    = 300,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG,
				.cra_blocksize   = SHA256_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct nss_cryptoapi_ctx),
				.cra_alignmask   = 0,
				.cra_type        = &crypto_ahash_type,
				.cra_module      = THIS_MODULE,
				.cra_init        = nss_cryptoapi_ahash_cra_init,
				.cra_exit        = nss_cryptoapi_ahash_cra_exit,
			},
		},
	},
};

/*
 * nss_cryptoapi_register()
 * 	register crypto core with the cryptoapi CFI
 */
static nss_crypto_user_ctx_t nss_cryptoapi_register(nss_crypto_handle_t crypto)
{
	int i;
	int rc = 0;
	struct nss_cryptoapi *sc = &gbl_ctx;

	nss_cfi_info("register nss_cryptoapi with core\n");

	sc->crypto = crypto;

	for (i = 0; i < ARRAY_SIZE(cryptoapi_algs); i++) {

		rc = crypto_register_alg(&cryptoapi_algs[i]);
		if (rc) {
			nss_cfi_err("Aead registeration failed, algo: %s\n", cryptoapi_algs[i].cra_name);
			continue;
		}
		nss_cfi_info("Aead registeration succeed, algo: %s\n", cryptoapi_algs[i].cra_name);
	}

	for (i = 0; enable_ahash && i < ARRAY_SIZE(cryptoapi_ahash_algs); i++) {

		rc = crypto_register_ahash(&cryptoapi_ahash_algs[i]);
		if (rc) {
			nss_cfi_err("Ahash registeration failed, algo: %s\n",
				cryptoapi_ahash_algs[i].halg.base.cra_name);
			continue;
		}
		nss_cfi_info("Ahash registeration succeed, algo: %s\n",
			cryptoapi_ahash_algs[i].halg.base.cra_name);
	}

	/*
	 * Set the registered flag
	 */
	atomic_set(&gbl_ctx.registered, 1);

	/*
	 * Initialize debugfs for cryptoapi.
	 */
	nss_cryptoapi_debugfs_init(sc);

	return sc;
}

/*
 * nss_cryptoapi_unregister()
 * 	Unregister crypto core with cryptoapi CFI layer
 */
static void nss_cryptoapi_unregister(nss_crypto_user_ctx_t cfi)
{
	struct nss_cryptoapi *sc = &gbl_ctx;
	int rc = 0;
	int i;

	nss_cfi_info("unregister nss_cryptoapi\n");

	/*
	 * Clear the registered flag
	 */
	atomic_set(&gbl_ctx.registered, 0);

	for (i = 0; i < ARRAY_SIZE(cryptoapi_algs); i++) {

		rc = crypto_unregister_alg(&cryptoapi_algs[i]);
		if (rc) {
			nss_cfi_err("Aead unregister failed, algo: %s\n", cryptoapi_algs[i].cra_name);
			continue;
		}
		nss_cfi_info("Aead unregister succeed, algo: %s\n", cryptoapi_algs[i].cra_name);
	}

	for (i = 0; i < ARRAY_SIZE(cryptoapi_ahash_algs); i++) {
		rc = crypto_unregister_ahash(&cryptoapi_ahash_algs[i]);
		if (rc) {
			nss_cfi_err("Ahash unregister failed, algo: %s\n",
				cryptoapi_ahash_algs[i].halg.base.cra_name);
			continue;
		}
		nss_cfi_info("Ahash unregister succeed, algo: %s\n",
			cryptoapi_ahash_algs[i].halg.base.cra_name);
	}

	/*
	 * cleanup cryptoapi debugfs.
	 */
	nss_cryptoapi_debugfs_exit(sc);
}

/*
 * nss_cryptoapi_is_registered()
 *	Cryptoapi function to check if crypto driver is registered
 */
bool nss_cryptoapi_is_registered(void)
{
	return !!atomic_read(&gbl_ctx.registered);
}
EXPORT_SYMBOL(nss_cryptoapi_is_registered);

/*
 * nss_cryptoapi_init()
 * 	Initializing crypto core layer
 */
int nss_cryptoapi_init(void)
{
	struct nss_cryptoapi *sc = &gbl_ctx;

	sc->crypto = NULL;

	atomic_set(&gbl_ctx.registered, 0);

	nss_crypto_register_user(nss_cryptoapi_register, nss_cryptoapi_unregister, "nss_cryptoapi");
	nss_cfi_info("initialize nss_cryptoapi\n");

	return 0;
}

/*
 * nss_cryptoapi_exit()
 * 	De-Initialize cryptoapi CFI layer
 */
void nss_cryptoapi_exit(void)
{
	struct nss_cryptoapi *sc = &gbl_ctx;

	if (sc->crypto) {
		nss_crypto_unregister_user(sc->crypto);
	}
	nss_cfi_info("exiting nss_cryptoapi\n");
}

MODULE_LICENSE("Dual BSD/GPL");

module_init(nss_cryptoapi_init);
module_exit(nss_cryptoapi_exit);
