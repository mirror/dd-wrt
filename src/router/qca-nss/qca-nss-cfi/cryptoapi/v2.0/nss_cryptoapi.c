/* Copyright (c) 2015-2019 The Linux Foundation. All rights reserved.
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
#include <crypto/internal/aead.h>
#include <linux/moduleparam.h>
#include <linux/spinlock.h>
#include <asm/cmpxchg.h>
#include <linux/delay.h>
#include <linux/crypto.h>
#include <linux/rtnetlink.h>
#include <linux/debugfs.h>
#include <linux/completion.h>

#include <crypto/aes.h>
#include <crypto/des.h>
#include <crypto/sha.h>
#include <crypto/hash.h>
#include <crypto/md5.h>
#include <crypto/ghash.h>
#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/authenc.h>
#include <crypto/ctr.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/skcipher.h>

#include <nss_api_if.h>
#include <nss_crypto_cmn.h>
#include <nss_cfi_if.h>
#include <nss_crypto_api.h>
#include <nss_crypto_defines.h>
#include <nss_crypto_hdr.h>
#include "nss_cryptoapi_private.h"

bool enable_aead = true;
module_param(enable_aead, bool, S_IRUGO);
MODULE_PARM_DESC(enable_aead, "Enable Cryptoapi AEAD support");

bool enable_ablk = true;
module_param(enable_ablk, bool, S_IRUGO);
MODULE_PARM_DESC(enable_ablk, "Enable Cryptoapi ABLK support");

bool enable_ahash = true;
module_param(enable_ahash, bool, S_IRUGO);
MODULE_PARM_DESC(enable_ahash, "Enable Cryptoapi AHASH support");

static int free_timeout = NSS_CRYPTOAPI_TIMEOUT;
module_param(free_timeout, int, 0644);
MODULE_PARM_DESC(free_timeout, "Timeout (msecs) for delayed session free; due to outstanding requests");

struct nss_cryptoapi_algo_info g_algo_info[] = {
	/*
	 * Aynsc Block ciphers
	 */
	{
		.cra_name = "cbc(aes)",
		.cipher_keylen = AES_KEYSIZE_128,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_CBC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
	},
	{
		.cra_name = "cbc(aes)",
		.cipher_keylen = AES_KEYSIZE_192,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_CBC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
	},
	{
		.cra_name = "cbc(aes)",
		.cipher_keylen = AES_KEYSIZE_256,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_CBC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
	},
	{
		.cra_name = "cbc(des3_ede)",
		.cipher_keylen = DES3_EDE_KEY_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_3DES_CBC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
	},
	{
		.cra_name = "rfc3686(ctr(aes))",
		.cipher_keylen = AES_KEYSIZE_128,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_CTR,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,
		.nonce = CTR_RFC3686_NONCE_SIZE,
	},
	{
		.cra_name = "rfc3686(ctr(aes))",
		.cipher_keylen = AES_KEYSIZE_192,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_CTR,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,
		.nonce = CTR_RFC3686_NONCE_SIZE,
	},
	{
		.cra_name = "rfc3686(ctr(aes))",
		.cipher_keylen = AES_KEYSIZE_256,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_CTR,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,
		.nonce = CTR_RFC3686_NONCE_SIZE,
	},
	{
		.cra_name = "ecb(aes)",
		.cipher_keylen = AES_KEYSIZE_128,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_ECB,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_ECB,
	},
	{
		.cra_name = "ecb(aes)",
		.cipher_keylen = AES_KEYSIZE_192,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_ECB,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_ECB,
	},
	{
		.cra_name = "ecb(aes)",
		.cipher_keylen = AES_KEYSIZE_256,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_ECB,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_ECB,
	},

	/*
	 * Aynsc non-keyed HASH digests
	 */
	{
		.cra_name = "md5",
		.algo = NSS_CRYPTO_CMN_ALGO_MD5_HASH,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HASH,
	},
	{
		.cra_name = "sha1",
		.algo = NSS_CRYPTO_CMN_ALGO_SHA160_HASH,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HASH,
	},
	{
		.cra_name = "sha224",
		.algo = NSS_CRYPTO_CMN_ALGO_SHA224_HASH,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HASH,
	},
	{
		.cra_name = "sha256",
		.algo = NSS_CRYPTO_CMN_ALGO_SHA256_HASH,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HASH,
	},
	{
		.cra_name = "sha384",
		.algo = NSS_CRYPTO_CMN_ALGO_SHA384_HASH,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HASH,
	},
	{
		.cra_name = "sha512",
		.algo = NSS_CRYPTO_CMN_ALGO_SHA512_HASH,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HASH,
	},

	/*
	 * Aynsc keyed HMAC digests
	 */
	{
		.cra_name = "hmac(md5)",
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blocksize = MD5_HMAC_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_MD5_HMAC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
	},
	{
		.cra_name = "hmac(sha1)",
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blocksize = SHA1_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_SHA160_HMAC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
	},
	{
		.cra_name = "hmac(sha256)",
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blocksize = SHA256_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_SHA256_HMAC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
	},
	{
		.cra_name = "hmac(sha384)",
		.digest_sz = SHA384_DIGEST_SIZE,
		.auth_blocksize = SHA384_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_SHA384_HMAC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
	},
	{
		.cra_name = "hmac(sha512)",
		.digest_sz = SHA512_DIGEST_SIZE,
		.auth_blocksize = SHA512_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_SHA512_HMAC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
	},

	/*
	 * AEAD AES-128
	 */
	{
		.cra_name = "echainiv(authenc(hmac(md5),cbc(aes)))",
		.cipher_keylen = AES_KEYSIZE_128,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blocksize = MD5_HMAC_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_CBC_MD5_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_echainiv_tx_proc,
	},
	{
		.cra_name = "echainiv(authenc(hmac(sha1),cbc(aes)))",
		.cipher_keylen = AES_KEYSIZE_128,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blocksize = SHA1_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_echainiv_tx_proc,
	},
	{
		.cra_name = "echainiv(authenc(hmac(sha256),cbc(aes)))",
		.cipher_keylen = AES_KEYSIZE_128,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blocksize = SHA256_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA256_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_echainiv_tx_proc,
	},
	{
		.cra_name = "seqiv(authenc(hmac(md5),rfc3686(ctr(aes))))",
		.cipher_keylen = AES_KEYSIZE_128,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blocksize = MD5_HMAC_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_CTR_MD5_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,
		.nonce = CTR_RFC3686_NONCE_SIZE,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_seqiv_tx_proc,
	},
	{
		.cra_name = "seqiv(authenc(hmac(sha1),rfc3686(ctr(aes))))",
		.cipher_keylen = AES_KEYSIZE_128,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blocksize = SHA1_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_CTR_SHA160_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,
		.nonce = CTR_RFC3686_NONCE_SIZE,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_seqiv_tx_proc,
	},
	{
		.cra_name = "seqiv(authenc(hmac(sha256),rfc3686(ctr(aes))))",
		.cipher_keylen = AES_KEYSIZE_128,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blocksize = SHA256_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_CTR_SHA256_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,
		.nonce = CTR_RFC3686_NONCE_SIZE,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_seqiv_tx_proc,
	},
	{
		.cra_name = "authenc(hmac(md5),cbc(aes))",
		.cipher_keylen = AES_KEYSIZE_128,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blocksize = MD5_HMAC_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_CBC_MD5_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
	},
	{
		.cra_name = "authenc(hmac(sha1),cbc(aes))",
		.cipher_keylen = AES_KEYSIZE_128,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blocksize = SHA1_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
	},
	{
		.cra_name = "authenc(hmac(sha256),cbc(aes))",
		.cipher_keylen = AES_KEYSIZE_128,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blocksize = SHA256_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA256_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
	},

	/*
	 * AEAD AES-192
	 */
	{
		.cra_name = "echainiv(authenc(hmac(md5),cbc(aes)))",
		.cipher_keylen = AES_KEYSIZE_192,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blocksize = MD5_HMAC_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_CBC_MD5_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_echainiv_tx_proc,
	},
	{
		.cra_name = "echainiv(authenc(hmac(sha1),cbc(aes)))",
		.cipher_keylen = AES_KEYSIZE_192,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blocksize = SHA1_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA160_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_echainiv_tx_proc,
	},
	{
		.cra_name = "echainiv(authenc(hmac(sha256),cbc(aes)))",
		.cipher_keylen = AES_KEYSIZE_192,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blocksize = SHA256_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA256_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_echainiv_tx_proc,
	},
	{
		.cra_name = "seqiv(authenc(hmac(md5),rfc3686(ctr(aes))))",
		.cipher_keylen = AES_KEYSIZE_192,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blocksize = MD5_HMAC_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_CTR_MD5_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,
		.nonce = CTR_RFC3686_NONCE_SIZE,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_seqiv_tx_proc,
	},
	{
		.cra_name = "seqiv(authenc(hmac(sha1),rfc3686(ctr(aes))))",
		.cipher_keylen = AES_KEYSIZE_192,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blocksize = SHA1_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_CTR_SHA160_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,
		.nonce = CTR_RFC3686_NONCE_SIZE,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_seqiv_tx_proc,
	},
	{
		.cra_name = "seqiv(authenc(hmac(sha256),rfc3686(ctr(aes))))",
		.cipher_keylen = AES_KEYSIZE_192,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blocksize = SHA256_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_CTR_SHA256_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,
		.nonce = CTR_RFC3686_NONCE_SIZE,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_seqiv_tx_proc,
	},
	{
		.cra_name = "authenc(hmac(md5),cbc(aes))",
		.cipher_keylen = AES_KEYSIZE_192,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blocksize = MD5_HMAC_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_CBC_MD5_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
	},
	{
		.cra_name = "authenc(hmac(sha1),cbc(aes))",
		.cipher_keylen = AES_KEYSIZE_192,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blocksize = SHA1_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA160_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
	},
	{
		.cra_name = "authenc(hmac(sha256),cbc(aes))",
		.cipher_keylen = AES_KEYSIZE_192,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blocksize = SHA256_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA256_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
	},

	/*
	 * AEAD AES-256
	 */
	{
		.cra_name = "echainiv(authenc(hmac(md5),cbc(aes)))",
		.cipher_keylen = AES_KEYSIZE_256,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blocksize = MD5_HMAC_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_CBC_MD5_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_echainiv_tx_proc,
	},
	{
		.cra_name = "echainiv(authenc(hmac(sha1),cbc(aes)))",
		.cipher_keylen = AES_KEYSIZE_256,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blocksize = SHA1_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA160_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_echainiv_tx_proc,
	},
	{
		.cra_name = "echainiv(authenc(hmac(sha256),cbc(aes)))",
		.cipher_keylen = AES_KEYSIZE_256,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blocksize = SHA256_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA256_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_echainiv_tx_proc,
	},
	{
		.cra_name = "seqiv(authenc(hmac(md5),rfc3686(ctr(aes))))",
		.cipher_keylen = AES_KEYSIZE_256,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blocksize = MD5_HMAC_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_CTR_MD5_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,
		.nonce = CTR_RFC3686_NONCE_SIZE,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_seqiv_tx_proc,
	},
	{
		.cra_name = "seqiv(authenc(hmac(sha1),rfc3686(ctr(aes))))",
		.cipher_keylen = AES_KEYSIZE_256,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blocksize = SHA1_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_CTR_SHA160_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,
		.nonce = CTR_RFC3686_NONCE_SIZE,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_seqiv_tx_proc,
	},
	{
		.cra_name = "seqiv(authenc(hmac(sha256),rfc3686(ctr(aes))))",
		.cipher_keylen = AES_KEYSIZE_256,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blocksize = SHA256_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_CTR_SHA256_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,
		.nonce = CTR_RFC3686_NONCE_SIZE,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_seqiv_tx_proc,
	},
	{
		.cra_name = "authenc(hmac(md5),cbc(aes))",
		.cipher_keylen = AES_KEYSIZE_256,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blocksize = MD5_HMAC_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_CBC_MD5_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
	},
	{
		.cra_name = "authenc(hmac(sha1),cbc(aes))",
		.cipher_keylen = AES_KEYSIZE_256,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blocksize = SHA1_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA160_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
	},
	{
		.cra_name = "authenc(hmac(sha256),cbc(aes))",
		.cipher_keylen = AES_KEYSIZE_256,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blocksize = SHA256_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA256_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
	},

	/*
	 * AEAD 3DES
	 */
	{
		.cra_name = "echainiv(authenc(hmac(md5),cbc(des3_ede)))",
		.cipher_keylen = DES3_EDE_KEY_SIZE,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blocksize = MD5_HMAC_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_3DES_CBC_MD5_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_echainiv_tx_proc,
	},
	{
		.cra_name = "echainiv(authenc(hmac(sha1),cbc(des3_ede)))",
		.cipher_keylen = DES3_EDE_KEY_SIZE,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blocksize = SHA1_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA160_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_echainiv_tx_proc,
	},
	{
		.cra_name = "echainiv(authenc(hmac(sha256),cbc(des3_ede)))",
		.cipher_keylen = DES3_EDE_KEY_SIZE,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blocksize = SHA256_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA256_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_echainiv_tx_proc,
	},
	{
		.cra_name = "authenc(hmac(md5),cbc(des3_ede))",
		.cipher_keylen = DES3_EDE_KEY_SIZE,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blocksize = MD5_HMAC_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_3DES_CBC_MD5_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
	},
	{
		.cra_name = "authenc(hmac(sha1),cbc(des3_ede))",
		.cipher_keylen = DES3_EDE_KEY_SIZE,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blocksize = SHA1_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA160_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
	},
	{
		.cra_name = "authenc(hmac(sha256),cbc(des3_ede))",
		.cipher_keylen = DES3_EDE_KEY_SIZE,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blocksize = SHA256_BLOCK_SIZE,
		.algo = NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA256_HMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_CBC,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_HMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
	},
	{
		.cra_name = "rfc4106(gcm(aes))",
		.cipher_keylen = AES_KEYSIZE_128,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_GCM_GMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_GCM_RFC4106,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_GMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
		.nonce = CTR_RFC3686_NONCE_SIZE,
	},
	{
		.cra_name = "rfc4106(gcm(aes))",
		.cipher_keylen = AES_KEYSIZE_192,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_GCM_GMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_GCM_RFC4106,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_GMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
		.nonce = CTR_RFC3686_NONCE_SIZE,
	},
	{
		.cra_name = "rfc4106(gcm(aes))",
		.cipher_keylen = AES_KEYSIZE_256,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_GCM_GMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_GCM_RFC4106,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_GMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
		.nonce = CTR_RFC3686_NONCE_SIZE,
	},
	{
		.cra_name = "seqiv(rfc4106(gcm(aes)))",
		.cipher_keylen = AES_KEYSIZE_128,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_GCM_GMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_GCM_RFC4106,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_GMAC,
		.aead_tx_proc = nss_cryptoapi_aead_seqiv_tx_proc,
		.nonce = CTR_RFC3686_NONCE_SIZE,
	},
	{
		.cra_name = "seqiv(rfc4106(gcm(aes)))",
		.cipher_keylen = AES_KEYSIZE_192,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_GCM_GMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_GCM_RFC4106,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_GMAC,
		.aead_tx_proc = nss_cryptoapi_aead_seqiv_tx_proc,
		.nonce = CTR_RFC3686_NONCE_SIZE,
	},
	{
		.cra_name = "seqiv(rfc4106(gcm(aes)))",
		.cipher_keylen = AES_KEYSIZE_256,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_GCM_GMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_GCM_RFC4106,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_GMAC,
		.aead_tx_proc = nss_cryptoapi_aead_seqiv_tx_proc,
		.nonce = CTR_RFC3686_NONCE_SIZE,
	},
	{
		.cra_name = "gcm(aes))",
		.cipher_keylen = AES_KEYSIZE_128,
		.algo = NSS_CRYPTO_CMN_ALGO_AES128_GCM_GMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_GCM,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_GMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
		.nonce = 0,
	},
	{
		.cra_name = "gcm(aes)",
		.cipher_keylen = AES_KEYSIZE_192,
		.algo = NSS_CRYPTO_CMN_ALGO_AES192_GCM_GMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_GCM,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_GMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
		.nonce = 0,
	},
	{
		.cra_name = "gcm(aes)",
		.cipher_keylen = AES_KEYSIZE_256,
		.algo = NSS_CRYPTO_CMN_ALGO_AES256_GCM_GMAC,
		.cipher_mode = NSS_CRYPTOAPI_CIPHER_MODE_GCM,
		.auth_mode = NSS_CRYPTOAPI_AUTH_MODE_GMAC,
		.aead_tx_proc = nss_cryptoapi_aead_tx_proc,
		.nonce = 0,
	}
};

/*
 * crypto_alg structure initialization
 */

/*
 * AEAD (Cipher and Authentication)
 */
struct aead_alg cryptoapi_aead_algs[] = {
	{	/*
		 * md5, aes-cbc
		 */
		.base = {
			.cra_name = "echainiv(authenc(hmac(md5),cbc(aes)))",
			.cra_driver_name = "nss-hmac-md5-cbc-aes",
			.cra_priority = 10000,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = AES_BLOCK_SIZE,
		.maxauthsize = MD5_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/*
		 * sha1, aes-cbc
		 */
		.base = {
			.cra_name = "echainiv(authenc(hmac(sha1),cbc(aes)))",
			.cra_driver_name = "nss-hmac-sha1-cbc-aes",
			.cra_priority = 10000,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = AES_BLOCK_SIZE,
		.maxauthsize = SHA1_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/* sha1, rfc3686-aes-ctr */
		.base = {
			.cra_name       = "seqiv(authenc(hmac(md5),rfc3686(ctr(aes))))",
			.cra_driver_name = "nss-hmac-md5-rfc3686-ctr-aes",
			.cra_priority   = 10000,
			.cra_flags      = CRYPTO_ALG_ASYNC,
			.cra_blocksize  = AES_BLOCK_SIZE,
			.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask  = 0,
			.cra_module     = THIS_MODULE,
		},

		.init		= nss_cryptoapi_aead_init,
		.exit		= nss_cryptoapi_aead_exit,
		.ivsize         = CTR_RFC3686_IV_SIZE,
		.maxauthsize    = MD5_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/* sha1, rfc3686-aes-ctr */
		.base = {
			.cra_name       = "seqiv(authenc(hmac(sha1),rfc3686(ctr(aes))))",
			.cra_driver_name = "nss-hmac-sha1-rfc3686-ctr-aes",
			.cra_priority   = 10000,
			.cra_flags      = CRYPTO_ALG_ASYNC,
			.cra_blocksize  = AES_BLOCK_SIZE,
			.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask  = 0,
			.cra_module     = THIS_MODULE,
		},

		.init		= nss_cryptoapi_aead_init,
		.exit		= nss_cryptoapi_aead_exit,
		.ivsize         = CTR_RFC3686_IV_SIZE,
		.maxauthsize    = SHA1_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/*
		 * sha256, aes-cbc
		 */
		.base = {
			.cra_name = "echainiv(authenc(hmac(sha256),cbc(aes)))",
			.cra_driver_name = "nss-hmac-sha256-cbc-aes",
			.cra_priority = 10000,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = AES_BLOCK_SIZE,
		.maxauthsize = SHA256_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/* sha256, rfc3686-aes-ctr */
		.base = {
			.cra_name       = "seqiv(authenc(hmac(sha256),rfc3686(ctr(aes))))",
			.cra_driver_name = "nss-hmac-sha256-rfc3686-ctr-aes",
			.cra_priority   = 10000,
			.cra_flags      = CRYPTO_ALG_ASYNC,
			.cra_blocksize  = AES_BLOCK_SIZE,
			.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask  = 0,
			.cra_module     = THIS_MODULE,
		},

		.init		= nss_cryptoapi_aead_init,
		.exit		= nss_cryptoapi_aead_exit,
		.ivsize         = CTR_RFC3686_IV_SIZE,
		.maxauthsize    = SHA256_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/*
		 * md5, 3des
		 */
		.base = {
			.cra_name = "echainiv(authenc(hmac(md5),cbc(des3_ede)))",
			.cra_driver_name = "nss-hmac-md5-cbc-3des",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = DES3_EDE_BLOCK_SIZE,
		.maxauthsize = MD5_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/*
		 * sha1, 3des
		 */
		.base = {
			.cra_name = "echainiv(authenc(hmac(sha1),cbc(des3_ede)))",
			.cra_driver_name = "nss-hmac-sha1-cbc-3des",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = DES3_EDE_BLOCK_SIZE,
		.maxauthsize = SHA1_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/*
		 * sha256, 3des
		 */
		.base = {
			.cra_name = "echainiv(authenc(hmac(sha256),cbc(des3_ede)))",
			.cra_driver_name = "nss-hmac-sha256-cbc-3des",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = DES3_EDE_BLOCK_SIZE,
		.maxauthsize = SHA256_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{
		.base = {
			.cra_name = "authenc(hmac(sha1),cbc(aes))",
			.cra_driver_name = "nss-hmac-sha1-cbc-aes",
			.cra_priority = 10000,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = AES_BLOCK_SIZE,
		.maxauthsize = SHA1_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/*
		 * sha256, aes-cbc
		 */
		.base = {
			.cra_name = "authenc(hmac(sha256),cbc(aes))",
			.cra_driver_name = "nss-hmac-sha256-cbc-aes",
			.cra_priority = 10000,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = AES_BLOCK_SIZE,
		.maxauthsize = SHA256_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/*
		 * sha1, 3des
		 */
		.base = {
			.cra_name = "authenc(hmac(sha1),cbc(des3_ede))",
			.cra_driver_name = "nss-hmac-sha1-cbc-3des",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = DES3_EDE_BLOCK_SIZE,
		.maxauthsize = SHA1_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/*
		 * sha256, 3des
		 */
		.base = {
			.cra_name = "authenc(hmac(sha256),cbc(des3_ede))",
			.cra_driver_name = "nss-hmac-sha256-cbc-3des",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = DES3_EDE_BLOCK_SIZE,
		.maxauthsize = SHA256_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/*
		 * RFC4106, GCM
		 */
		.base = {
			.cra_name = "rfc4106(gcm(aes))",
			.cra_driver_name = "nss-rfc4106-gcm",
			.cra_priority = 10000,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = 8,
		.maxauthsize = GHASH_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey_noauth,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/*
		 * RFC4106, GCM
		 */
		.base = {
			.cra_name = "seqiv(rfc4106(gcm(aes)))",
			.cra_driver_name = "nss-rfc4106-gcm",
			.cra_priority = 10000,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = 8,
		.maxauthsize = GHASH_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey_noauth,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	},
	{	/*
		 * GCM
		 */
		.base = {
			.cra_name = "gcm(aes)",
			.cra_driver_name = "nss-gcm",
			.cra_priority = 10000,
			.cra_flags = CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
			.cra_alignmask = 0,
			.cra_module = THIS_MODULE,
		},

		.init = nss_cryptoapi_aead_init,
		.exit = nss_cryptoapi_aead_exit,
		.ivsize = 12,
		.maxauthsize = GHASH_DIGEST_SIZE,
		.setkey = nss_cryptoapi_aead_setkey_noauth,
		.setauthsize = nss_cryptoapi_aead_setauthsize,
		.encrypt = nss_cryptoapi_aead_encrypt,
		.decrypt = nss_cryptoapi_aead_decrypt,
	}
};

/*
 * ABLK cipher algorithms
 */
static struct crypto_alg cryptoapi_ablkcipher_algs[] = {
	{
		.cra_name = "cbc(aes)",
		.cra_driver_name = "nss-cbc-aes",
		.cra_priority = 10000,
		.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
		.cra_blocksize = AES_BLOCK_SIZE,
		.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask = 0,
		.cra_type = &crypto_ablkcipher_type,
		.cra_module = THIS_MODULE,
		.cra_init = nss_cryptoapi_ablkcipher_init,
		.cra_exit = nss_cryptoapi_ablkcipher_exit,
		.cra_u = {
			.ablkcipher = {
				.ivsize = AES_BLOCK_SIZE,
				.min_keysize = AES_MIN_KEY_SIZE,
				.max_keysize = AES_MAX_KEY_SIZE,
				.setkey = nss_cryptoapi_ablk_setkey,
				.encrypt = nss_cryptoapi_ablk_encrypt,
				.decrypt = nss_cryptoapi_ablk_decrypt,
			},
		},
	},
	{
		.cra_name       = "rfc3686(ctr(aes))",
		.cra_driver_name = "nss-rfc3686-ctr-aes",
		.cra_priority   = 30000,
		.cra_flags      = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
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
				.setkey         = nss_cryptoapi_ablk_setkey,
				.encrypt        = nss_cryptoapi_ablk_encrypt,
				.decrypt        = nss_cryptoapi_ablk_decrypt,
			},
		},
	},
	{
		.cra_name = "ecb(aes)",
		.cra_driver_name = "nss-ecb-aes",
		.cra_priority = 10000,
		.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
		.cra_blocksize = AES_BLOCK_SIZE,
		.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask = 0,
		.cra_type = &crypto_ablkcipher_type,
		.cra_module = THIS_MODULE,
		.cra_init = nss_cryptoapi_ablkcipher_init,
		.cra_exit = nss_cryptoapi_ablkcipher_exit,
		.cra_u = {
			.ablkcipher = {
				.min_keysize = AES_MIN_KEY_SIZE,
				.max_keysize = AES_MAX_KEY_SIZE,
				.setkey = nss_cryptoapi_ablk_setkey,
				.encrypt = nss_cryptoapi_ablk_encrypt,
				.decrypt = nss_cryptoapi_ablk_decrypt,
			},
		},
	},
	{
		.cra_name = "cbc(des3_ede)",
		.cra_driver_name = "nss-cbc-des-ede",
		.cra_priority = 10000,
		.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
		.cra_blocksize = DES3_EDE_BLOCK_SIZE,
		.cra_ctxsize = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask = 0,
		.cra_type = &crypto_ablkcipher_type,
		.cra_module = THIS_MODULE,
		.cra_init = nss_cryptoapi_ablkcipher_init,
		.cra_exit = nss_cryptoapi_ablkcipher_exit,
		.cra_u = {
			.ablkcipher = {
				.ivsize = DES3_EDE_BLOCK_SIZE,
				.min_keysize = DES3_EDE_KEY_SIZE,
				.max_keysize = DES3_EDE_KEY_SIZE,
				.setkey = nss_cryptoapi_ablk_setkey,
				.encrypt = nss_cryptoapi_ablk_encrypt,
				.decrypt = nss_cryptoapi_ablk_decrypt,
			},
		},
	}
};

/*
 * AHASH algorithms
 */
static struct ahash_alg cryptoapi_ahash_algs[] = {
	/*
	 * Async non-keyed HASH digests
	 */
	{
		.init   = nss_cryptoapi_ahash_init,
		.update = nss_cryptoapi_ahash_update,
		.final  = nss_cryptoapi_ahash_final,
		.export = nss_cryptoapi_ahash_export,
		.import = nss_cryptoapi_ahash_import,
		.digest = nss_cryptoapi_ahash_digest,
		.setkey = NULL,
		.halg   = {
			.digestsize = MD5_DIGEST_SIZE,
			.statesize  = sizeof(struct md5_state),
			.base       = {
				.cra_name        = "md5",
				.cra_driver_name = "nss-md5",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = MD5_HMAC_BLOCK_SIZE,
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
		.setkey = NULL,
		.halg   = {
			.digestsize = SHA1_DIGEST_SIZE,
			.statesize  = sizeof(struct sha1_state),
			.base       = {
				.cra_name        = "sha1",
				.cra_driver_name = "nss-sha1",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
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
		.setkey = NULL,
		.halg   = {
			.digestsize = SHA224_DIGEST_SIZE,
			.statesize  = sizeof(struct sha256_state),
			.base       = {
				.cra_name        = "sha224",
				.cra_driver_name = "nss-sha224",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA224_BLOCK_SIZE,
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
		.setkey = NULL,
		.halg   = {
			.digestsize = SHA256_DIGEST_SIZE,
			.statesize  = sizeof(struct sha256_state),
			.base       = {
				.cra_name        = "sha256",
				.cra_driver_name = "nss-sha256",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
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
	{
		.init   = nss_cryptoapi_ahash_init,
		.update = nss_cryptoapi_ahash_update,
		.final  = nss_cryptoapi_ahash_final,
		.export = nss_cryptoapi_ahash_export,
		.import = nss_cryptoapi_ahash_import,
		.digest = nss_cryptoapi_ahash_digest,
		.setkey = NULL,
		.halg   = {
			.digestsize = SHA384_DIGEST_SIZE,
			.statesize  = sizeof(struct sha512_state),
			.base       = {
				.cra_name        = "sha384",
				.cra_driver_name = "nss-sha384",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA384_BLOCK_SIZE,
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
		.setkey = NULL,
		.halg   = {
			.digestsize = SHA512_DIGEST_SIZE,
			.statesize  = sizeof(struct sha512_state),
			.base       = {
				.cra_name        = "sha512",
				.cra_driver_name = "nss-sha512",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA512_BLOCK_SIZE,
				.cra_ctxsize     = sizeof(struct nss_cryptoapi_ctx),
				.cra_alignmask   = 0,
				.cra_type        = &crypto_ahash_type,
				.cra_module      = THIS_MODULE,
				.cra_init        = nss_cryptoapi_ahash_cra_init,
				.cra_exit        = nss_cryptoapi_ahash_cra_exit,
			},
		},
	},

	/*
	 * Aynsc keyed HMAC digests
	 */
	{
		.init   = nss_cryptoapi_ahash_init,
		.update = nss_cryptoapi_ahash_update,
		.final  = nss_cryptoapi_ahash_final,
		.export = nss_cryptoapi_ahash_export,
		.import = nss_cryptoapi_ahash_import,
		.digest = nss_cryptoapi_ahash_digest,
		.setkey = nss_cryptoapi_ahash_setkey,
		.halg   = {
			.digestsize = MD5_DIGEST_SIZE,
			.statesize  = sizeof(struct md5_state),
			.base       = {
				.cra_name        = "hmac(md5)",
				.cra_driver_name = "nss-hmac-md5",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = MD5_HMAC_BLOCK_SIZE,
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
			.digestsize = SHA1_DIGEST_SIZE,
			.statesize  = sizeof(struct sha1_state),
			.base       = {
				.cra_name        = "hmac(sha1)",
				.cra_driver_name = "nss-hmac-sha1",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
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
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
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
	{
		.init   = nss_cryptoapi_ahash_init,
		.update = nss_cryptoapi_ahash_update,
		.final  = nss_cryptoapi_ahash_final,
		.export = nss_cryptoapi_ahash_export,
		.import = nss_cryptoapi_ahash_import,
		.digest = nss_cryptoapi_ahash_digest,
		.setkey = nss_cryptoapi_ahash_setkey,
		.halg   = {
			.digestsize = SHA384_DIGEST_SIZE,
			.statesize  = sizeof(struct sha512_state),
			.base       = {
				.cra_name        = "hmac(sha384)",
				.cra_driver_name = "nss-hmac-sha384",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA384_BLOCK_SIZE,
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
			.digestsize = SHA512_DIGEST_SIZE,
			.statesize  = sizeof(struct sha512_state),
			.base       = {
				.cra_name        = "hmac(sha512)",
				.cra_driver_name = "nss-hmac-sha512",
				.cra_priority    = 1000,
				.cra_flags       = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
				.cra_blocksize   = SHA512_BLOCK_SIZE,
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
 * nss_cryptoapi_ref_dec()
 *	Decrement cryptoapi context reference
 */
void nss_cryptoapi_ref_dec(struct nss_cryptoapi_ctx *ctx)
{
	BUG_ON(atomic_read(&ctx->refcnt) == 0);
	if (atomic_sub_and_test(1, &ctx->refcnt)) {
		complete(&ctx->complete);
	}
}

/*
 * nss_cryptoapi_cra_name2info()
 *     Lookup the associated algorithm in NSS for the given transformation
 */
struct nss_cryptoapi_algo_info *nss_cryptoapi_cra_name2info(const char *cra_name, uint16_t enc_keylen,
							uint16_t digest_sz)
{
	struct nss_cryptoapi_algo_info *info = g_algo_info;
	uint16_t klen;
	int i;

	for (i = 0; i < ARRAY_SIZE(g_algo_info); i++, info++) {
		if (strncmp(info->cra_name, cra_name, strlen(cra_name)))
			continue;

		klen = enc_keylen - info->nonce;
		if ((info->cipher_keylen == klen) && (info->digest_sz == digest_sz))
			return info;
	}

	return NULL;
}

/*
 * nss_cryptoapi_status2error()
 *	Convert the crypto status (if any) into Linux specific errno
 */
int nss_cryptoapi_status2error(struct nss_cryptoapi_ctx *ctx, uint8_t status)
{
	if (!status)
		return 0;

	if (status >= NSS_CRYPTO_CMN_RESP_ERROR_MAX)
		return -E2BIG;

	ctx->stats.error[status]++;

	switch (status) {
	case NSS_CRYPTO_CMN_RESP_ERROR_HDR_VERSION:
		return -EPERM;
	case NSS_CRYPTO_CMN_RESP_ERROR_CTX_NOUSE:
		return -ENOENT;
	case NSS_CRYPTO_CMN_RESP_ERROR_HASH_NOSPACE:
		return -ENOMEM;
	case NSS_CRYPTO_CMN_RESP_ERROR_DATA_TIMEOUT:
		return -EIO;
	case NSS_CRYPTO_CMN_RESP_ERROR_CTX_RANGE:
		return -ERANGE;
	case NSS_CRYPTO_CMN_RESP_ERROR_HASH_CHECK:
		return -EBADMSG;
	case NSS_CRYPTO_CMN_RESP_ERROR_DATA_LEN:
	case NSS_CRYPTO_CMN_RESP_ERROR_CIPHER_ALGO:
	case NSS_CRYPTO_CMN_RESP_ERROR_CIPHER_MODE:
	case NSS_CRYPTO_CMN_RESP_ERROR_CIPHER_BLK_LEN:
	default:
		return -EINVAL;
	}
}

/*
 * nss_cryptoapi_transform()
 * 	Transform routine for encryption and decryption operations.
 */
int nss_cryptoapi_transform(struct nss_cryptoapi_ctx *ctx, struct nss_cryptoapi_info *info, void *app_data,
				bool ahash)
{
	struct nss_crypto_hdr *ch;
	uint8_t *iv_addr;
	int error;

	/*
	 * Allocate crypto hdr
	 */
	ch = nss_crypto_hdr_alloc(ctx->user, ctx->sid, info->nsegs, info->iv_size, info->hmac_len, ahash);
	if (!ch) {
		ctx->stats.failed_nomem++;
		/*
		 * Decrement cryptoapi context reference
		 */
		nss_cryptoapi_ref_dec(ctx);
		return -ENOMEM;
	}

	/*
	 * 1. Set cipher skip
	 * 2. Set operation
	 * 3. Set HMAC length
	 */
	nss_crypto_hdr_set_skip(ch, info->skip);
	nss_crypto_hdr_set_op(ch, info->op_dir);

	/*
	 * For GCM, overwrite auth only len
	 */
	if ((ctx->info->cipher_mode == NSS_CRYPTOAPI_CIPHER_MODE_GCM) ||
	    (ctx->info->cipher_mode == NSS_CRYPTOAPI_CIPHER_MODE_GCM_RFC4106))
		nss_crypto_hdr_set_auth(ch, info->auth);

	/*
	 * Check if the last segment needs a reduction in length
	 * This is essential as the map API will map the entire length
	 * present in the SGLIST. During encryption process the length
	 * includes the space for producing the HASH. Hence, it would
	 * make the HW produce the hash outside of the designated
	 * location. In order to avoid this we reduce the length by the
	 * amount mentioned in the hmac_len. This allows the production
	 * of HMAC inside the packet length. Restore the length after
	 * mapping is done.
	 */
	if (unlikely(ahash)) {
		nss_crypto_hdr_map_sglist_ahash(ch, info->first_sg, info->ahash_skip);

		/*
		 * Skip IV since this is ahash
		 */
		goto skip_iv;
	}

	nss_crypto_hdr_map_sglist(ch, info->first_sg, info->ahash_skip);

	/*
	 * Get IV location and memcpy the IV
	 */
	iv_addr = (uint8_t *)nss_crypto_hdr_get_iv(ch);

	switch (ctx->info->cipher_mode) {
	case NSS_CRYPTOAPI_CIPHER_MODE_CBC:
		memcpy(iv_addr, info->iv, info->iv_size);
		break;

	case NSS_CRYPTOAPI_CIPHER_MODE_GCM:
		((uint32_t *)iv_addr)[0] = ((uint32_t *)info->iv)[0];
		((uint32_t *)iv_addr)[1] = ((uint32_t *)info->iv)[1];
		((uint32_t *)iv_addr)[2] = ((uint32_t *)info->iv)[2];
		((uint32_t *)iv_addr)[3] = ctx->ctx_iv[3];
		break;

	case NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686:
	case NSS_CRYPTOAPI_CIPHER_MODE_GCM_RFC4106:
		((uint32_t *)iv_addr)[0] = ctx->ctx_iv[0];
		((uint32_t *)iv_addr)[1] = ((uint32_t *)info->iv)[0];
		((uint32_t *)iv_addr)[2] = ((uint32_t *)info->iv)[1];
		((uint32_t *)iv_addr)[3] = ctx->ctx_iv[3];
		break;

	default:
		break;
	}

skip_iv:
	/*
	 * Send the buffer to CORE layer for processing
	 */
	error = nss_crypto_transform_payload(ctx->user, ch, info->cb, app_data);
	if (error < 0) {
		/*
		 * Decrement cryptoapi reference
		 */
		nss_cryptoapi_ref_dec(ctx);
		nss_crypto_hdr_free(ctx->user, ch);
		ctx->stats.failed_queue++;
		return error;
	}

	ctx->stats.queued++;
	return -EINPROGRESS;
}

/*
 * nss_cryptoapi_ctx_stats_read()
 * 	CryptoAPI context statistics read function
 */
ssize_t nss_cryptoapi_ctx_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_cryptoapi_ctx *ctx = fp->private_data;
	struct nss_cryptoapi_stats *stats = &ctx->stats;
	ssize_t max_buf_len;
	ssize_t len;
	ssize_t ret;
	int i;
	char *buf;

	/*
	 * We need to allocate space for the string and the value
	 */
	max_buf_len = NSS_CRYPTOAPI_DEBUGFS_MAX_STATS_ENTRY * NSS_CRYPTOAPI_DEBUGFS_MAX_NAME * 2;
	max_buf_len += (ARRAY_SIZE(stats->error) * NSS_CRYPTOAPI_DEBUGFS_MAX_NAME * 2);

	buf = vzalloc(max_buf_len);
	if (!buf)
		return 0;

	/*
	 * create context stats files
	 */
	len = snprintf(buf, max_buf_len, "queued - %llu\n", stats->queued);
	len += snprintf(buf + len, max_buf_len - len, "completed - %llu\n", stats->completed);
	len += snprintf(buf + len, max_buf_len - len, "init - %llu\n", stats->init);
	len += snprintf(buf + len, max_buf_len - len, "exit - %llu\n", stats->exit);
	len += snprintf(buf + len, max_buf_len - len, "failed_init - %llu\n", stats->failed_init);
	len += snprintf(buf + len, max_buf_len - len, "failed_exit - %llu\n", stats->failed_exit);
	len += snprintf(buf + len, max_buf_len - len, "failed_fallback - %llu\n", stats->failed_fallback);
	len += snprintf(buf + len, max_buf_len - len, "failed_queue - %llu\n", stats->failed_queue);
	len += snprintf(buf + len, max_buf_len - len, "failed_nomem - %llu\n", stats->failed_nomem);
	len += snprintf(buf + len, max_buf_len - len, "failed_req - %llu\n", stats->failed_req);
	len += snprintf(buf + len, max_buf_len - len, "failed_align - %llu\n", stats->failed_align);

	/*
	 * This will autoexpand as crypto adds new error codes
	 */
	for (i = 0; i < ARRAY_SIZE(stats->error); i++)
		len += snprintf(buf + len, max_buf_len - len, "resp_error(%d) - %llu\n", i, stats->error[i]);

	ret = simple_read_from_buffer(ubuf, sz, ppos, buf, len);
	vfree(buf);

	return ret;
}

/*
 * nss_cryptoapi_ctx_info_read()
 * 	CryptoAPI context info read function
 */
ssize_t nss_cryptoapi_ctx_info_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_cryptoapi_ctx *ctx = fp->private_data;
	ssize_t max_buf_len;
	ssize_t len;
	ssize_t ret;
	char *buf;

	/*
	 * We need to allocate space for the string and the value
	 */
	max_buf_len = NSS_CRYPTOAPI_DEBUGFS_MAX_CTX_ENTRY * NSS_CRYPTOAPI_DEBUGFS_MAX_NAME * 2;
	buf = vzalloc(max_buf_len);
	if (!buf)
		return 0;

	len = snprintf(buf, max_buf_len, "linux_algo - %s\n", ctx->info->cra_name);
	len += snprintf(buf + len, max_buf_len - len, "nss_algo - %d\n", ctx->info->algo);
	len += snprintf(buf + len, max_buf_len - len, "cipher_key_len - %d\n", ctx->info->cipher_keylen);
	len += snprintf(buf + len, max_buf_len - len, "digest_size - %d\n", ctx->info->digest_sz);
	len += snprintf(buf + len, max_buf_len - len, "auth_block_size - %d\n", ctx->info->auth_blocksize);
	len += snprintf(buf + len, max_buf_len - len, "fallback_req - %d\n", ctx->fallback_req);

	ret = simple_read_from_buffer(ubuf, sz, ppos, buf, len);
	vfree(buf);

	return ret;
}

/*
 * Context file operation structure instance
 */
static const struct file_operations ctx_stats_op = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = nss_cryptoapi_ctx_stats_read,
};

/*
 * Context file operation structure instance
 */
static const struct file_operations ctx_info_op = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = nss_cryptoapi_ctx_info_read,
};

/*
 * nss_cryptoapi_add_ctx2debugfs()
 * 	Creates per session debugfs entries
 */
void nss_cryptoapi_add_ctx2debugfs(struct nss_cryptoapi_ctx *ctx)
{
	char buf[NSS_CRYPTOAPI_DEBUGFS_MAX_NAME] = {0};

	if (!g_cryptoapi.root) {
		nss_cfi_err("%p: DebugFS root directory missing(%p)\n", &g_cryptoapi, ctx);
		return;
	}

	snprintf(buf, sizeof(buf), "ctx%d", ctx->sid);
	ctx->dentry = debugfs_create_dir(buf, g_cryptoapi.root);
	if (!ctx->dentry) {
		nss_cfi_err("%p: Unable to create context debugfs entry", ctx);
		return;
	}

	debugfs_create_file("info", S_IRUGO, ctx->dentry, ctx, &ctx_info_op);
	debugfs_create_file("stats", S_IRUGO, ctx->dentry, ctx, &ctx_stats_op);
}

/*
 * nss_cryptoapi_attach_user()
 * 	register crypto core with the cryptoapi CFI
 */
void nss_cryptoapi_attach_user(void *app_data, struct nss_crypto_user *user)
{
	struct crypto_alg *ablk = cryptoapi_ablkcipher_algs;
	struct aead_alg *aead = cryptoapi_aead_algs;
	struct ahash_alg *ahash = cryptoapi_ahash_algs;
	struct nss_cryptoapi *sc = app_data;
	int i;

	nss_cfi_info("%p: Registering algorithms with Linux\n", sc);

	BUG_ON(!user);

	/*
	 * It is possible that the attach gets called immediately by the
	 * crypto driver. This happens if NSS notifies the Crypto driver
	 * that it is ready for use. In that case the crypto_register_user
	 * would immediately call attach before returning the control to
	 * module_init. Thus we can enter a situation where the user
	 * is not loaded.
	 */
	if (!g_cryptoapi.user) {
		      g_cryptoapi.user = user;
	}

	for (i = 0; enable_ablk && (i < ARRAY_SIZE(cryptoapi_ablkcipher_algs)); i++, ablk++) {
		if (crypto_register_alg(ablk)) {
			nss_cfi_err("%p: ABLK registration failed(%s)\n", sc, ablk->cra_name);
			ablk->cra_flags = 0;
		}
	}

	for (i = 0; enable_aead && (i < ARRAY_SIZE(cryptoapi_aead_algs)); i++, aead++) {
		if (crypto_register_aead(aead)) {
			nss_cfi_err("%p: AEAD registration failed(%s)\n", sc, aead->base.cra_name);
			aead->base.cra_flags = 0;
		}
	}

	for (i = 0; enable_ahash && (i < ARRAY_SIZE(cryptoapi_ahash_algs)); i++, ahash++) {
		if (crypto_register_ahash(ahash)) {
			nss_cfi_err("%p: AHASH registration failed(%s)\n", sc, ahash->halg.base.cra_name);
			ahash->halg.base.cra_flags = 0;
		}
	}

	/*
	 * Set the registered flag
	 */
	atomic_set(&g_cryptoapi.registered, (enable_aead || enable_ablk || enable_ahash));
}

/*
 * nss_cryptoapi_detach_user()
 * 	Unregister crypto core with cryptoapi CFI layer
 */
void nss_cryptoapi_detach_user(void *app_data, struct nss_crypto_user *user)
{
	struct crypto_alg *ablk = cryptoapi_ablkcipher_algs;
	struct aead_alg *aead = cryptoapi_aead_algs;
	struct ahash_alg *ahash = cryptoapi_ahash_algs;
	struct nss_cryptoapi *sc = app_data;
	int i;

	nss_cfi_info("unregister nss_cryptoapi\n");

	/*
	 * Clear the registered flag
	 */
	atomic_set(&g_cryptoapi.registered, 0);

	for (i = 0; enable_ablk && (i < ARRAY_SIZE(cryptoapi_ablkcipher_algs)); i++, ablk++) {
		if (!ablk->cra_flags)
			continue;

		crypto_unregister_alg(ablk);
		nss_cfi_info("%p: ABLK unregister succeeded, algo: %s\n", sc, ablk->cra_name);
	}

	for (i = 0; enable_aead && (i < ARRAY_SIZE(cryptoapi_aead_algs)); i++, aead++) {
		if (!aead->base.cra_flags)
			continue;

		crypto_unregister_aead(aead);
		nss_cfi_info("%p: AEAD unregister succeeded, algo: %s\n", sc, aead->base.cra_name);
	}

	for (i = 0; enable_ahash && (i < ARRAY_SIZE(cryptoapi_ahash_algs)); i++, ahash++) {
		if (!ahash->halg.base.cra_flags)
			continue;

		crypto_unregister_ahash(ahash);
		nss_cfi_info("%p: AHASH unregister succeeded, algo: %s\n", sc, ahash->halg.base.cra_name);
	}
}

struct nss_cryptoapi g_cryptoapi = {
	.ctx = {
		.name = "nss_cryptoapi",
		.attach = nss_cryptoapi_attach_user,
		.detach = nss_cryptoapi_detach_user,
		.hdr_pool_sz = NSS_CRYPTOAPI_HDR_POOL_SZ,
		.default_hdr_sz = NSS_CRYPTOAPI_DEFAULT_HDR_SZ,
	},

	.algo_info = g_algo_info,
};

/*
 * nss_cryptoapi_is_registered()
 *	Cryptoapi function to check if crypto driver is registered
 */
bool nss_cryptoapi_is_registered(void)
{
	return !!atomic_read(&g_cryptoapi.registered);
}
EXPORT_SYMBOL(nss_cryptoapi_is_registered);

/*
 * nss_cryptoapi_init()
 * 	Initializing crypto core layer
 */
int nss_cryptoapi_init(void)
{
	nss_cfi_info("module loaded %s\n", NSS_CFI_BUILD_ID);

	/*
	 * Create debugfs root directory for cryptoapi.
	 */
	g_cryptoapi.root = debugfs_create_dir("qca-nss-cryptoapi", NULL);

	/*
	 * Fill context free timeout
	 */
	g_cryptoapi.ctx.timeout_ticks = msecs_to_jiffies(free_timeout);

	atomic_set(&g_cryptoapi.registered, 0);

	g_cryptoapi.user = nss_crypto_register_user(&g_cryptoapi.ctx, &g_cryptoapi);
	if (!g_cryptoapi.user) {
		nss_cfi_err("%p: Failed to register nss_cryptoapi\n", &g_cryptoapi);
		debugfs_remove_recursive(g_cryptoapi.root);
		return -ENODEV;
	}

	return 0;
}

/*
 * nss_cryptoapi_exit()
 * 	De-Initialize cryptoapi CFI layer
 */
void nss_cryptoapi_exit(void)
{
	if (g_cryptoapi.user)
		nss_crypto_unregister_user(g_cryptoapi.user);

	/*
	 * Cleanup cryptoapi debugfs.
	 */
	if (g_cryptoapi.root)
		debugfs_remove_recursive(g_cryptoapi.root);

	nss_cfi_info("module unloaded %s\n", NSS_CFI_BUILD_ID);
}

MODULE_LICENSE("Dual BSD/GPL");

module_init(nss_cryptoapi_init);
module_exit(nss_cryptoapi_exit);
