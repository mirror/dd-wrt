/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
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
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/memory.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/reset.h>
#include <linux/bitmap.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <linux/log2.h>
#include <linux/completion.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
#include <crypto/sha.h>
#else
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#endif
#include <crypto/des.h>
#include <crypto/aes.h>
#include <crypto/md5.h>
#include <crypto/ctr.h>
#include <crypto/ghash.h>
#include <nss_crypto_hw.h>
#include <nss_crypto_hlos.h>
#include <nss_api_if.h>
#include <nss_crypto_api.h>
#include <nss_crypto_hdr.h>
#include <nss_crypto_ctrl.h>
#include <nss_crypto_cmn.h>
#include <nss_crypto_debugfs.h>

struct nss_crypto_ctrl g_control = { {0} };

#define NSS_CRYPTO_DELAYED_FREE_TICKS msecs_to_jiffies(10/* msecs */)
#define NSS_CRYPTO_DELAYED_INIT_TICKS msecs_to_jiffies(100/* msecs */)

/*
 * g_algo_info
 *	global structure for algorithm specific information
 */
static struct nss_crypto_algo_info g_algo_info[NSS_CRYPTO_CMN_ALGO_MAX] = {
	[NSS_CRYPTO_CMN_ALGO_NULL] = {
		.name = "qcom,NULL",
		.cipher_valid = false,
		.auth_valid = false,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_3DES_CBC] = {
		.name = "qcom,3des-cbc",
		.cipher_key_len = DES3_EDE_KEY_SIZE,
		.cipher_blk_len = DES3_EDE_BLOCK_SIZE,
		.iv_len = DES3_EDE_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CBC] = {
		.name = "qcom,aes128-cbc",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CTR] = {
		.name = "qcom,aes128-ctr",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_ECB] = {
		.name = "qcom,aes128-ecb",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_CBC] = {
		.name = "qcom,aes192-cbc",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_CTR] = {
		.name = "qcom,aes192-ctr",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_ECB] = {
		.name = "qcom,aes192-ecb",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CBC] = {
		.name = "qcom,aes256-cbc",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CTR] = {
		.name = "qcom,aes256-ctr",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_ECB] = {
		.name = "qcom,aes256-ecb",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_MD5_HASH] = {
		.name = "qcom,md5-hash",
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blk_len = MD5_HMAC_BLOCK_SIZE,
		.cipher_valid = false,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA160_HASH] = {
		.name = "qcom,sha160-hash",
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blk_len = SHA1_BLOCK_SIZE,
		.cipher_valid = false,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA224_HASH] = {
		.name = "qcom,sha224-hash",
		.digest_sz = SHA224_DIGEST_SIZE,
		.auth_blk_len = SHA224_BLOCK_SIZE,
		.cipher_valid = false,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA256_HASH] = {
		.name = "qcom,sha256-hash",
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blk_len = SHA256_BLOCK_SIZE,
		.cipher_valid = false,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA384_HASH] = {
		.name = "qcom,sha384-hash",
		.digest_sz = SHA384_DIGEST_SIZE,
		.auth_blk_len = SHA384_BLOCK_SIZE,
		.cipher_valid = false,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_MD5_HMAC] = {
		.name = "qcom,md5-hmac",
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blk_len = MD5_HMAC_BLOCK_SIZE,
		.cipher_valid = false,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA512_HASH] = {
		.name = "qcom,sha512-hash",
		.digest_sz = SHA512_DIGEST_SIZE,
		.auth_blk_len = SHA512_BLOCK_SIZE,
		.cipher_valid = false,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA160_HMAC] = {
		.name = "qcom,sha160-hmac",
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blk_len = SHA1_BLOCK_SIZE,
		.cipher_valid = false,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA224_HMAC] = {
		.name = "qcom,sha224-hmac",
		.digest_sz = SHA224_DIGEST_SIZE,
		.auth_blk_len = SHA224_BLOCK_SIZE,
		.cipher_valid = false,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA256_HMAC] = {
		.name = "qcom,sha256-hmac",
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blk_len = SHA256_BLOCK_SIZE,
		.cipher_valid = false,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA384_HMAC] = {
		.name = "qcom,sha384-hmac",
		.digest_sz = SHA384_DIGEST_SIZE,
		.auth_blk_len = SHA384_BLOCK_SIZE,
		.cipher_valid = false,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA512_HMAC] = {
		.name = "qcom,sha512-hmac",
		.digest_sz = SHA512_DIGEST_SIZE,
		.auth_blk_len = SHA512_BLOCK_SIZE,
		.cipher_valid = false,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_GCM_GMAC] = {
		.name = "qcom,aes128-gcm-gmac",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = 8,
		.digest_sz = GHASH_DIGEST_SIZE,
		.auth_blk_len = GHASH_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.nonce_sz = 4,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CBC_MD5_HMAC] = {
		.name = "qcom,aes128-cbc-md5-hmac",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blk_len = MD5_HMAC_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC] = {
		.name = "qcom,aes128-cbc-sha160-hmac",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blk_len = SHA1_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA256_HMAC] = {
		.name = "qcom,aes128-cbc-sha256-hmac",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blk_len = SHA256_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CTR_MD5_HMAC] = {
		.name = "qcom,aes128-ctr-md5-hmac",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blk_len = MD5_HMAC_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA384_HMAC] = {
		.name = "qcom,aes128-cbc-sha384-hmac",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA384_DIGEST_SIZE,
		.auth_blk_len = SHA384_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA512_HMAC] = {
		.name = "qcom,aes128-cbc-sha512-hmac",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA512_DIGEST_SIZE,
		.auth_blk_len = SHA512_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CTR_SHA160_HMAC] = {
		.name = "qcom,aes128-ctr-sha160-hmac",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blk_len = SHA1_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CTR_SHA256_HMAC] = {
		.name = "qcom,aes128-ctr-sha256-hmac",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blk_len = SHA256_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CTR_SHA384_HMAC] = {
		.name = "qcom,aes128-ctr-sha384-hmac",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA384_DIGEST_SIZE,
		.auth_blk_len = SHA384_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CTR_SHA512_HMAC] = {
		.name = "qcom,aes128-ctr-sha512-hmac",
		.cipher_key_len = AES_KEYSIZE_128,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA512_DIGEST_SIZE,
		.auth_blk_len = SHA512_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_GCM_GMAC] = {
		.name = "qcom,aes192-gcm-gmac",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = 8,
		.digest_sz = GHASH_DIGEST_SIZE,
		.auth_blk_len = GHASH_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.nonce_sz = 4,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_CBC_MD5_HMAC] = {
		.name = "qcom,aes192-cbc-md5-hmac",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blk_len = MD5_HMAC_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA160_HMAC] = {
		.name = "qcom,aes192-cbc-sha160-hmac",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blk_len = SHA1_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA256_HMAC] = {
		.name = "qcom,aes192-cbc-sha256-hmac",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blk_len = SHA256_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_CTR_MD5_HMAC] = {
		.name = "qcom,aes192-ctr-md5-hmac",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blk_len = MD5_HMAC_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA384_HMAC] = {
		.name = "qcom,aes192-cbc-sha384-hmac",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA384_DIGEST_SIZE,
		.auth_blk_len = SHA384_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA512_HMAC] = {
		.name = "qcom,aes192-cbc-sha512-hmac",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA512_DIGEST_SIZE,
		.auth_blk_len = SHA512_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_CTR_SHA160_HMAC] = {
		.name = "qcom,aes192-ctr-sha160-hmac",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blk_len = SHA1_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_CTR_SHA256_HMAC] = {
		.name = "qcom,aes192-ctr-sha256-hmac",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blk_len = SHA256_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_CTR_SHA384_HMAC] = {
		.name = "qcom,aes192-ctr-sha384-hmac",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA384_DIGEST_SIZE,
		.auth_blk_len = SHA384_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES192_CTR_SHA512_HMAC] = {
		.name = "qcom,aes192-ctr-sha512-hmac",
		.cipher_key_len = AES_KEYSIZE_192,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA512_DIGEST_SIZE,
		.auth_blk_len = SHA512_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_GCM_GMAC] = {
		.name = "qcom,aes256-gcm-gmac",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = 8,
		.digest_sz = GHASH_DIGEST_SIZE,
		.auth_blk_len = GHASH_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = false,
		.nonce_sz = 4,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CBC_MD5_HMAC] = {
		.name = "qcom,aes256-cbc-md5-hmac",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blk_len = MD5_HMAC_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA160_HMAC] = {
		.name = "qcom,aes256-cbc-sha160-hmac",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blk_len = SHA1_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA256_HMAC] = {
		.name = "qcom,aes256-cbc-sha256-hmac",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blk_len = SHA256_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CTR_MD5_HMAC] = {
		.name = "qcom,aes256-ctr-md5-hmac",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blk_len = MD5_HMAC_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA384_HMAC] = {
		.name = "qcom,aes256-cbc-sha384-hmac",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA384_DIGEST_SIZE,
		.auth_blk_len = SHA384_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA512_HMAC] = {
		.name = "qcom,aes256-cbc-sha512-hmac",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA512_DIGEST_SIZE,
		.auth_blk_len = SHA512_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CTR_SHA160_HMAC] = {
		.name = "qcom,aes256-ctr-sha160-hmac",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blk_len = SHA1_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CTR_SHA256_HMAC] = {
		.name = "qcom,aes256-ctr-sha256-hmac",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blk_len = SHA256_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_3DES_CBC_MD5_HMAC] = {
		.name = "qcom,3des-cbc-md5-hmac",
		.cipher_key_len = DES3_EDE_KEY_SIZE,
		.cipher_blk_len = DES3_EDE_BLOCK_SIZE,
		.iv_len = DES3_EDE_BLOCK_SIZE,
		.digest_sz = MD5_DIGEST_SIZE,
		.auth_blk_len = MD5_HMAC_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CTR_SHA384_HMAC] = {
		.name = "qcom,aes256-ctr-sha384-hmac",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA384_DIGEST_SIZE,
		.auth_blk_len = SHA384_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CTR_SHA512_HMAC] = {
		.name = "qcom,aes256-ctr-sha512-hmac",
		.cipher_key_len = AES_KEYSIZE_256,
		.cipher_blk_len = AES_BLOCK_SIZE,
		.iv_len = AES_BLOCK_SIZE,
		.digest_sz = SHA512_DIGEST_SIZE,
		.auth_blk_len = SHA512_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.nonce_sz = CTR_RFC3686_NONCE_SIZE,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA160_HMAC] = {
		.name = "qcom,3des-cbc-sha160-hmac",
		.cipher_key_len = DES3_EDE_KEY_SIZE,
		.cipher_blk_len = DES3_EDE_BLOCK_SIZE,
		.iv_len = DES3_EDE_BLOCK_SIZE,
		.digest_sz = SHA1_DIGEST_SIZE,
		.auth_blk_len = SHA1_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
	[NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA256_HMAC] = {
		.name = "qcom,3des-cbc-sha256-hmac",
		.cipher_key_len = DES3_EDE_KEY_SIZE,
		.cipher_blk_len = DES3_EDE_BLOCK_SIZE,
		.iv_len = DES3_EDE_BLOCK_SIZE,
		.digest_sz = SHA256_DIGEST_SIZE,
		.auth_blk_len = SHA256_BLOCK_SIZE,
		.cipher_valid = true,
		.auth_valid = true,
		.is_supp = false,
	},
};

/*
 * nss_crypto_algo_is_supp
 *	Check if the algo is supported by the HW
 */
bool nss_crypto_algo_is_supp(uint32_t algo)
{
	return g_algo_info[algo].is_supp;
}
EXPORT_SYMBOL(nss_crypto_algo_is_supp);

/*
 * nss_crypto_ctrl_stats_init
 *	Initialize control stats in crypto control
 */
static void nss_crypto_ctrl_stats_init(struct nss_crypto_ctrl_stats *stats)
{
	atomic_set(&stats->alloc, 0);
	atomic_set(&stats->alloc_fail_node, 0);
	atomic_set(&stats->alloc_fail_nomem, 0);
	atomic_set(&stats->alloc_fail_nospace, 0);
	atomic_set(&stats->alloc_fail_noalgo, 0);
	atomic_set(&stats->alloc_fail_noresp, 0);
	atomic_set(&stats->free, 0);
	atomic_set(&stats->free_fail_msg, 0);
	atomic_set(&stats->free_fail_inuse, 0);
	atomic_set(&stats->free_fail_queue, 0);
	atomic_set(&stats->free_delayed, 0);
}

/*
 * nss_crypto_get_cipher_block_len()
 *	Return the cipher block len associated with the index
 */
uint16_t nss_crypto_get_cipher_block_len(uint32_t session)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	uint16_t idx = NSS_CRYPTO_SESSION_INDEX(session, ctrl->max_contexts);
	struct nss_crypto_ctx *ctx;
	uint16_t blk_len;

	ctx = &ctrl->ctx_tbl[idx];
	if (!kref_get_unless_zero(&ctx->ref))
		return 0;

	blk_len = ctx->info->cipher_blk_len;
	kref_put(&ctx->ref, nss_crypto_ctx_free);
	return blk_len;
}
EXPORT_SYMBOL(nss_crypto_get_cipher_block_len);

/*
 * nss_crypto_get_iv_len()
 *     Return the iv associated with the index
 */
uint16_t nss_crypto_get_iv_len(uint32_t session)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	uint16_t idx = NSS_CRYPTO_SESSION_INDEX(session, ctrl->max_contexts);
	struct nss_crypto_ctx *ctx;
	uint16_t iv_len;

	ctx = &ctrl->ctx_tbl[idx];
	if (!kref_get_unless_zero(&ctx->ref))
		return 0;

	iv_len = ctx->info->iv_len;
	kref_put(&ctx->ref, nss_crypto_ctx_free);
	return iv_len;
}
EXPORT_SYMBOL(nss_crypto_get_iv_len);

/*
 * nss_crypto_ctx_free()
 *	put the reference down for the context
 *
 * Note: this will schedule the delayed free
 */
void nss_crypto_ctx_free(struct kref *ref)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	atomic_inc(&ctrl->cstats.free);
}
EXPORT_SYMBOL(nss_crypto_ctx_free);

/*
 * nss_crypto_process_event()
 *	process incoming messages
 */
void nss_crypto_process_event(void *app_data, struct nss_crypto_cmn_msg *nim)
{
	uint32_t *msg_stats = (uint32_t *)&nim->msg.stats;
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct nss_crypto_node *node;
	struct nss_crypto_ctx *ctx;
	bool process = false;
	uint64_t *stats;
	uint16_t idx;
	int num;

	/*
	 * find the node associated with message
	 */
	list_for_each_entry(node, &ctrl->node_head, list) {
		if (node->nss_ifnum == nim->cm.interface) {
			process = true;
			break;
		}
	}

	/*
	 * Mark NSS FW Crypto as active.
	 */
	atomic_set(&ctrl->nss_active, 1);

	/*
	 * check if we need to process the message
	 */
	if (!process)
		return;

	switch (nim->cm.type) {
	case NSS_CRYPTO_CMN_MSG_TYPE_SYNC_NODE_STATS:
		stats = (uint64_t *)&node->stats;

		for (num = 0; num < sizeof(node->stats)/sizeof(*stats); num++) {
			stats[num] += msg_stats[num];
		}

		break;

	case NSS_CRYPTO_CMN_MSG_TYPE_SYNC_CTX_STATS:
		idx = NSS_CRYPTO_SESSION_INDEX(nim->uid, ctrl->max_contexts);
		ctx = &ctrl->ctx_tbl[idx];
		if (!kref_get_unless_zero(&ctx->ref))
			break;

		stats = (uint64_t *)&ctx->stats;
		for (num = 0; num < sizeof(ctx->stats)/sizeof(*stats); num++) {
			stats[num] += msg_stats[num];
		}

		kref_put(&ctx->ref, nss_crypto_ctx_free);
		break;

	default:
		nss_crypto_warn("%px: unsupported message (%d), uid (%d)\n", node, nim->cm.type, nim->uid);
		break;
	}
}

/*
 * nss_crypto_free()
 *	Free crypto context
 */
void nss_crypto_free(struct nss_crypto_ctx *ctx)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	int32_t status;

	/*
	 * If, the NSS queue is congested then try few more times to see
	 * if the message goes through. Otherwise the index will remain
	 * in use and will not be allocated to any other user
	 */
	status = nss_crypto_cmn_tx_msg_sync(ctrl->nss_msg_hdl, &ctx->nim);
	switch (status) {
	case NSS_TX_SUCCESS:
		break;

	case NSS_TX_FAILURE_QUEUE:
		/*
		 * Queue congestion is a intermediate state. We should retry
		 * again to see if it has cleared.
		 */
		atomic_inc(&ctrl->cstats.free_fail_queue);
		schedule_delayed_work(&ctx->free_work, NSS_CRYPTO_DELAYED_FREE_TICKS);
		return;

	default:
		atomic_inc(&ctrl->cstats.free_fail_msg);
		nss_crypto_warn("%px: Failed to send crypto session(%d) deletion msg\n", ctx, ctx->idx);
		goto free;
	}

	/*
	 * Check if the NSS is successful in deleting the context; otherwise
	 * retry after sometime to see if the context can be released. This
	 * will happen if the NSS crypto context has outstanding packets
	 */
	if (ctx->nim.cm.response != NSS_CMN_RESPONSE_ACK) {
		atomic_inc(&ctrl->cstats.free_fail_inuse);
		ctx->nim.cm.type = NSS_CRYPTO_CMN_MSG_TYPE_VERIFY_CTX;
		schedule_delayed_work(&ctx->free_work, ctx->free_timeout);
		return;
	}

free:
	if (ctx->dentry)
		debugfs_remove_recursive(ctx->dentry);

	kref_put(&ctx->ref, nss_crypto_ctx_free);
}

/*
 * nss_crypto_delayed_free()
 *	delayed free worker for freeing up context later
 *
 * Note: The expectation is that if the context is in use
 * then it cannot be freed immediately at host and NSS.
 * We should wait in a delayed thread context to attempt
 * freeing at NSS first followed by host. It is also
 * possible that the host to NSS queue is busy in which
 * case we need to retry.
 */
void nss_crypto_delayed_free(struct work_struct *work)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct nss_crypto_ctx *ctx;

	ctx = container_of(to_delayed_work(work), struct nss_crypto_ctx, free_work);
	atomic_inc(&ctrl->cstats.free_delayed);
	nss_crypto_free(ctx);
}

/*
 * nss_crypto_session_alloc()
 *	allocate the session index and fill other context related information
 */
int nss_crypto_session_alloc(struct nss_crypto_user *user, struct nss_crypto_session_data *data,
				uint32_t *session)
{
	enum nss_crypto_cmn_msg_type type = NSS_CRYPTO_CMN_MSG_TYPE_SETUP_CTX;
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct nss_crypto_cmn_msg nim = { {0} };
	struct nss_crypto_cmn_ctx *msg = &nim.msg.ctx;
	struct nss_crypto_node *node;
	struct nss_crypto_ctx *ctx;
	nss_tx_status_t status;
	bool found = false;
	uint16_t idx;
	int error;

	BUG_ON(in_atomic());

	/*
	 * Incase of failure to alloc crypto session in FW, free function see
	 * this big value and return;
	 */
	*session = NSS_CRYPTO_SESSION_MAX;

	if (data->algo >= NSS_CRYPTO_CMN_ALGO_MAX) {
		nss_crypto_warn("%px: invalid configuration (algo - %d)\n", user, data->algo);
		atomic_inc(&ctrl->cstats.alloc_fail_noalgo);
		return -ERANGE;
	}

	/*
	 * this only allows one outstanding session_alloc
	 * to run on any core
	 */
	mutex_lock(&ctrl->mutex); /* lock*/
	for (idx = 0, ctx = ctrl->ctx_tbl; idx < ctrl->max_contexts; idx++, ctx++) {
		/*
		 * To check whether the context is available, we would
		 * acquire the reference to see whether the context is
		 * already in use or not. In case, it is not in use
		 * kref_init would acquire the reference thus holding it
		 * for the current session
		 */
		if (!kref_get_unless_zero(&ctx->ref)) {
			memset(ctx, 0, sizeof(*ctx));
			ctx->idx = idx;
			atomic_set(&ctx->active, 0);
			kref_init(&ctx->ref);
			break;
		}

		kref_put(&ctx->ref, nss_crypto_ctx_free);
	}

	mutex_unlock(&ctrl->mutex); /* unlock */

	/*
	 * if there is nothing available indicate it to the caller
	 */
	if (idx == ctrl->max_contexts) {
		atomic_inc(&ctrl->cstats.alloc_fail_nospace);
		return -ENOSPC;
	}

	/*
	 * Walk through each node and call node specific context fill routine
	 */
	list_for_each_entry(node, &ctrl->node_head, list) {
		/*
		 * check if the node supports requested algorithm;
		 * if the current node does not support the requested
		 * algorithm then try the next one. It is desired that
		 * only one of the nodes will be capable of handling a
		 * particular algorithm
		 */
		if (node->algo[data->algo]) {
			found = true;
			break;
		}
	}

	if (!found) {
		atomic_inc(&ctrl->cstats.alloc_fail_noalgo);
		return -ENOENT;
	}

	/*
	 * Fill common information for context
	 */
	ctx->info = &g_algo_info[data->algo];
	ctx->node = node;
	ctx->algo = data->algo;
	ctx->nss_ifnum = node->nss_ifnum;

	/*
	 * Fill cipher keys
	 */
	if (data->cipher_key) {
		BUG_ON(!ctx->info->cipher_valid);
		BUG_ON(ctx->info->cipher_key_len > NSS_CRYPTO_CIPHER_KEYLEN_MAX);

		memcpy((uint8_t *)&msg->cipher_key[0], data->cipher_key, ctx->info->cipher_key_len);
	}

	/*
	 * Fill auth digest for HMAC mode ops
	 */
	if (data->auth_key) {
		BUG_ON(!ctx->info->auth_valid);
		BUG_ON(ARRAY_SIZE(msg->auth_key) < data->auth_keylen);
		BUG_ON(ctx->info->auth_blk_len < data->auth_keylen);

		memcpy((uint8_t *)&msg->auth_key[0], data->auth_key, data->auth_keylen);
	}

	/*
	 * Copy nonce from message data, we will do a copy here irrespective of any check,
	 * NSS FW will use it as per algorithm
	 */
	if (data->nonce) {
		BUG_ON(ctx->info->nonce_sz > NSS_CRYPTO_NONCE_SIZE_MAX);
		memcpy((uint8_t *)&msg->nonce[0], data->nonce, ctx->info->nonce_sz);
	}

	/*
	 * Fill Hardware specific information
	 */
	error = node->fill_ctx(ctx, data, msg);
	if (error < 0) {
		atomic_inc(&ctrl->cstats.alloc_fail_node);
		kref_put(&ctx->ref, nss_crypto_ctx_free);
		return error;
	}

	/*
	 * Fill other fields of context message
	 */
	msg->index = idx;
	msg->algo = data->algo;
	msg->auth_keylen = data->auth_keylen;

	if (data->sec_key) {
		msg->sec_offset = data->sec_key_offset;
		msg->flags |= NSS_CRYPTO_CMN_CTX_FLAGS_SEC_OFFSET;
	}

	/*
	 * Prepare a sync message
	 */
	nss_cmn_msg_init(&nim.cm, node->nss_ifnum, type, sizeof(nim) - sizeof(nim.cm), NULL, ctx);

	/*
	 * context alloc is a synchronous call which will
	 * query the NSS and see if the context can be
	 * allocated or not
	 */
	status = nss_crypto_cmn_tx_msg_sync(ctrl->nss_msg_hdl, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_crypto_warn("%px: unable to send(%d), status(%d), response(%d), error(%d)\n",
				ctx, nim.cm.type, status, nim.cm.response, nim.cm.error);
		atomic_inc(&ctrl->cstats.alloc_fail_noresp);
		kref_put(&ctx->ref, nss_crypto_ctx_free);
		return -EBUSY;
	}

	/*
	 * Initialize the delayed free
	 */
	ctx->free_timeout = user->ctx.timeout_ticks;
	INIT_DELAYED_WORK(&ctx->free_work, nss_crypto_delayed_free);

	/*
	 * create context debugfs entry
	 */
	nss_crypto_debugfs_add_ctx(ctx, node->dentry);

	atomic_inc(&ctrl->cstats.alloc);

	*session = idx;
	atomic_set(&ctx->active, 1);

	return 0;
}
EXPORT_SYMBOL(nss_crypto_session_alloc);

/*
 * nss_crypto_session_free()
 *	free the crypto session, that was previously allocated
 */
void nss_crypto_session_free(struct nss_crypto_user *user, uint32_t session)
{
	enum nss_crypto_cmn_msg_type type = NSS_CRYPTO_CMN_MSG_TYPE_CLEAR_CTX;
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct nss_crypto_ctx *ctx;
	uint16_t idx;

	if (session >= ctrl->max_contexts) {
		nss_crypto_warn("%px: Invalid session id=%d (max= %d)\n", user, session, ctrl->max_contexts);
		return;
	}

	idx = NSS_CRYPTO_SESSION_INDEX(session, ctrl->max_contexts);
	mutex_lock(&ctrl->mutex);

	ctx = &ctrl->ctx_tbl[idx];
	if (!kref_get_unless_zero(&ctx->ref)) {
		mutex_unlock(&ctrl->mutex);
		nss_crypto_warn("%px: session id(%d) not in use\n", user, idx);
		return;
	}

	kref_put(&ctx->ref, nss_crypto_ctx_free);
	mutex_unlock(&ctrl->mutex);

	nss_cmn_msg_init(&ctx->nim.cm, ctx->nss_ifnum, type, sizeof(ctx->nim) - sizeof(ctx->nim.cm), NULL, ctx);
	ctx->nim.msg.ctx.index = idx;

	/*
	 * Free is deferred if the call is in non-sleepable context
	 */
	if (in_atomic()) {
		nss_crypto_info("%px: session free for idx(%d) is deferred\n", user, idx);
		schedule_delayed_work(&ctx->free_work, NSS_CRYPTO_DELAYED_FREE_TICKS);
		return;
	}

	nss_crypto_free(ctx);
}
EXPORT_SYMBOL(nss_crypto_session_free);

/*
 * nss_crypto_engine_init()
 *	initialize the crypto engine
 */
int nss_crypto_engine_init(struct nss_crypto_node *node, struct nss_crypto_engine *eng)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct nss_crypto_cmn_msg nim = {0};
	struct nss_crypto_cmn_engine *msg = &nim.msg.eng;
	struct nss_crypto_cmn_dma *dma_msg = &nim.msg.dma;
	nss_tx_status_t status;
	unsigned long *mask;
	uint16_t id;

	/*
	 * We should not enter this function in atomic state
	 */
	BUG_ON(in_atomic());

	nss_cmn_msg_init(&nim.cm, node->nss_ifnum, NSS_CRYPTO_CMN_MSG_TYPE_SETUP_ENG,
			 sizeof(nim) - sizeof(nim.cm), NULL, eng);

	nim.uid = eng->id;

	status = nss_crypto_cmn_tx_msg_sync(ctrl->nss_msg_hdl, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_crypto_warn("%px: unable to send(%d), status(%d), response(%d), error(%d)\n",
			node, nim.cm.type, status, nim.cm.response, nim.cm.error);
		return (status == NSS_TX_FAILURE_QUEUE) ? -EBUSY : -ENODEV;
	}

	eng->dma_mask = msg->dma_mask;
	if (!eng->dma_mask)
		return -ENOENT;

	/*
	 * Walk and find the DMA rings to initialize
	 */
	mask = (unsigned long *)&eng->dma_mask;
	for_each_set_bit(id, mask, BITS_PER_BYTE * sizeof(eng->dma_mask)) {
		memset(&nim, 0, sizeof(nim));

		/*
		 * Initialize Engine Specific DMA structures
		 */
		nss_cmn_msg_init(&nim.cm, node->nss_ifnum, NSS_CRYPTO_CMN_MSG_TYPE_SETUP_DMA,
			 sizeof(nim) - sizeof(nim.cm), NULL, eng);

		dma_msg->pair_id = id;

		status = nss_crypto_cmn_tx_msg_sync(ctrl->nss_msg_hdl, &nim);
		if (status != NSS_TX_SUCCESS) {
			nss_crypto_warn("%px: unable to send(%d), status(%d), rsp(%d), error(%d)\n",
					node, nim.cm.type, status, nim.cm.response, nim.cm.error);
			continue;
		}
	}

	/*
	 * Add engine to node head list and create a
	 * debugfs entry
	 */
	list_add_tail(&eng->list, &node->eng_head);

	nss_crypto_debugfs_add_engine(eng, node->dentry);

	return 0;
}

/*
 * nss_crypto_engine_alloc()
 *	This will allocate engine pointer
 */
struct nss_crypto_engine *nss_crypto_engine_alloc(struct platform_device *pdev)
{
	struct nss_crypto_engine *eng;

	eng = vzalloc(sizeof(struct nss_crypto_engine));
	if (!eng)
		return NULL;

	eng->dev = &pdev->dev;
	INIT_LIST_HEAD(&eng->list);

	return eng;
}

/*
 * nss_crypto_engine_free()
 *	deallocate the engine
 */
void nss_crypto_engine_free(struct nss_crypto_engine *eng)
{

	list_del(&eng->list);

	nss_crypto_debugfs_del_engine(eng);

	/*
	 * unmap previously mapped addresses
	 */
	iounmap(eng->crypto_vaddr);
	vfree(eng);
}

/*
 * nss_crypto_ndev_setup()
 *	setup the dummy netdevice
 */
void nss_crypto_ndev_setup(struct net_device *dev)
{
	nss_crypto_info("%px: dummy netdevice for crypto\n", dev);
}

/*
 * nss_crypto_node_alloc()
 *	Allocate the crypto node
 */
struct nss_crypto_node *nss_crypto_node_alloc(struct platform_device *pdev, uint32_t if_num, const char *name)
{
	struct nss_crypto_node *node;
	struct net_device *ndev;

	/*
	 * Create a dummy netdevice required for registering data callback with NSS driver
	 */
	ndev = alloc_netdev(sizeof(struct nss_crypto_node), name, NET_NAME_UNKNOWN, nss_crypto_ndev_setup);
	if (!ndev) {
		nss_crypto_warn("%px: unable to allocate node\n", pdev);
		return NULL;
	}

	node = netdev_priv(ndev);
	node->dev = &pdev->dev;

	INIT_LIST_HEAD(&node->list);
	INIT_LIST_HEAD(&node->eng_head);

	node->nss_ifnum = if_num;
	node->ndev = ndev;

	/*
	 * TODO Need to register a netdevice if we desire to
	 * communicate data to crypto from userspace
	 */

	return node;
}

/*
 * nss_crypto_node_init()
 *	initialize the crypto node
 */
int nss_crypto_node_init(struct nss_crypto_node *node, struct device_node *np)
{
	struct nss_crypto_cmn_msg nim = { {0} };
	struct nss_crypto_cmn_node *msg = &nim.msg.node;
	const size_t msg_sz = sizeof(nim) - sizeof(nim.cm);
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct nss_crypto_algo_info *info;
	nss_tx_status_t status;
	int i;

	/*
	 * We should not enter this function in atomic state
	 */
	BUG_ON(in_atomic());

	/*
	 * read maximum DMA rings
	 */
	if (of_property_read_u32(np, "qcom,dma-mask", &node->dma_mask) < 0) {
		nss_crypto_warn("%px: unable to read node DMA mask\n", np);
		return -ENOENT;
	}

	node->tx_enabled = of_property_read_bool(np, "qcom,transform-enabled");

	/*
	 * we need to map and mark the algorithms suggested by DTSI
	 * and supported by the driver
	 */
	for (i = 0, info = g_algo_info; i < ARRAY_SIZE(g_algo_info); i++, info++) {
		if (info->name && of_property_read_bool(np, info->name)) {
			node->algo[i] = info->is_supp = true;
			nss_crypto_info("node_init: %s is supported in HW\n", info->name);
			continue;
		}

		node->algo[i] = false;
		nss_crypto_info("node_init: %s is not supported in HW\n", info->name);
	}

	/*
	 * This is a synchronous message; hence there is nothing to callback
	 */
	nss_cmn_msg_init(&nim.cm, node->nss_ifnum, NSS_CRYPTO_CMN_MSG_TYPE_SETUP_NODE,
				msg_sz, NULL, node);

	msg->max_dma_rings = node->dma_mask;
	msg->max_ctx = ctrl->max_contexts;
	msg->max_ctx_size = ctrl->max_ctx_size;

	status = nss_crypto_cmn_tx_msg_sync(ctrl->nss_msg_hdl, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_crypto_warn("%px: unable to send(%d), status(%d), response(%d), error(%d)\n",
				node, nim.cm.type, status, nim.cm.response, nim.cm.error);
		return (status == NSS_TX_FAILURE_QUEUE) ? -EBUSY : -ENODEV;
	}

	list_add(&node->list, &ctrl->node_head);

	nss_crypto_debugfs_add_node(node, ctrl->dentry, node->ndev->name);

	node->nss_data_hdl = nss_crypto_cmn_data_register(node->nss_ifnum,
							nss_crypto_transform_done,
							node->ndev, 0);

	return 0;
}

/*
 * nss_crypto_node_free()
 *	free a crypto node
 */
void nss_crypto_node_free(struct nss_crypto_node *node)
{
	struct nss_crypto_engine *eng;
	struct nss_crypto_engine *tmp;

	list_for_each_entry_safe(eng, tmp, &node->eng_head, list)
		nss_crypto_engine_free(eng);

	list_del(&node->list);
	if (node->dentry)
		debugfs_remove_recursive(node->dentry);

	free_netdev(node->ndev);
}

/*
 * nss_crypto_user_free()
 *	free a crypto user
 */
void nss_crypto_user_free(struct kref *ref)
{
	struct nss_crypto_user *user = container_of(ref, struct nss_crypto_user, ref);
	struct sk_buff_head *sk_head = &user->sk_head;
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct sk_buff *skb;

	/*
	 * wake up the user unregister if its waiting for pending SKB(s)
	 */
	complete(&ctrl->complete);

	/*
	 * Note: we don't need a lock here to access the skb pool
	 * the fact that this is called when the references to the
	 * user has dropped to zero protects this list from being
	 * accessed by any other legitimate caller
	 */
	do {
		skb = __skb_dequeue(sk_head);
		if (!skb)
			break;

		dev_kfree_skb_any(skb);
	} while (!skb_queue_empty(sk_head));

	vfree(user);
}

/*
 * nss_crypto_register_user()
 *	register a new user of the crypto driver
 */
struct nss_crypto_user *nss_crypto_register_user(struct nss_crypto_user_ctx *ctx, void *app_data)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	uint16_t hdr_pool_sz, default_hdr_sz;
	struct nss_crypto_user *user;
	size_t aligned_offset;
	struct sk_buff *skb;
	size_t aligned_size;
	int num_skb;

	if (!of_find_compatible_node(NULL, NULL, "qcom,nss-crypto"))
		return NULL;

	if (!ctx->attach || !ctx->detach) {
		nss_crypto_warn("%px: callbacks missing\n", ctx);
		return NULL;
	}

	hdr_pool_sz = ctx->hdr_pool_sz;
	default_hdr_sz = ctx->default_hdr_sz;

	if (!hdr_pool_sz || !default_hdr_sz) {
		nss_crypto_warn("%px: pool_sz and hdr_sz missing\n", ctx);
		return NULL;
	}

	user = vzalloc(sizeof(struct nss_crypto_user));
	if (!user) {
		nss_crypto_warn("%px: unable to allocate user\n", ctx);
		return NULL;
	}

	memcpy(&user->ctx, ctx, sizeof(user->ctx));

	spin_lock_init(&user->lock);
	skb_queue_head_init(&user->sk_head);

        INIT_LIST_HEAD(&user->list);
	kref_init(&user->ref);

	user->app_data = app_data;

	aligned_size = ALIGN(default_hdr_sz, SMP_CACHE_BYTES) + SMP_CACHE_BYTES;

	/*
	 * Try allocating atleast 1 SKB for the pool
	 */
	skb = alloc_skb(aligned_size, GFP_KERNEL);
	if (!skb) {
		nss_crypto_warn("%px:unable to allocate atleast 1 skb\n", ctx);
		vfree(user);
		return NULL;
	}

	/*
	 * We are aligning the skb->data to SMP_CACHE_BYTES and
	 * adding an extra headroom of SMP_CACHE_BYTES
	 */
	aligned_offset = PTR_ALIGN(skb->data, SMP_CACHE_BYTES) - skb->head;
	skb_reserve(skb, SMP_CACHE_BYTES + aligned_offset);

	/*
	 * We dont need to take any locks at this point as
	 * the user pointer has not been notified to the caller
	 */
	__skb_queue_head(&user->sk_head, skb);

	/*
	 * Now we know that we have atleast 1 skb in the pool
	 * with which we can work with. Now try allocating
	 * hdr pool size worth of SKBs
	 */
	for (num_skb = 1; num_skb < hdr_pool_sz; num_skb++) {
		skb = alloc_skb(aligned_size, GFP_KERNEL);
		if (!skb)
			break;

		/*
		 * We are aligning the skb->data to SMP_CACHE_BYTES and
		 * adding an extra headroom of SMP_CACHE_BYTES
		 */
		aligned_offset = PTR_ALIGN(skb->data, SMP_CACHE_BYTES) - skb->head;
		skb_reserve(skb, SMP_CACHE_BYTES + aligned_offset);
		__skb_queue_head(&user->sk_head, skb);
	}

	nss_crypto_info("%px: registered user(%s), pool size(%d)\n", user, ctx->name, num_skb);

	/*
	 * We only register ourselves for a pending attach if the
	 * driver is not active
	 */
	mutex_lock(&ctrl->mutex);

	if (!ctrl->active) {
		list_add(&user->list, &ctrl->user_head);
		mutex_unlock(&ctrl->mutex);
		return user;
	}

	mutex_unlock(&ctrl->mutex);

	ctx->attach(app_data, user);
	return user;
}
EXPORT_SYMBOL(nss_crypto_register_user);

/*
 * nss_crypto_unregister_user()
 *	unregister a user from the crypto driver
 */
void nss_crypto_unregister_user(struct nss_crypto_user *user)
{
	unsigned long timeout_ticks = user->ctx.timeout_ticks;
	struct nss_crypto_ctrl *ctrl = &g_control;

	user->ctx.detach(user->app_data, user);

	/*
	 * If there are packets which are still getting
	 * processed for this particular user then kref
	 * put will fail
	 */
	if (kref_put(&user->ref, nss_crypto_user_free))
		return;

	/*
	 * We wait for some ticks worth of time to see if the
	 * outstanding SKBs' return to the pool. This situation
	 * will happen if there are inflight SKBs' while the unregister
	 * has happened.
	 */
	if (wait_for_completion_timeout(&ctrl->complete, timeout_ticks))
		return;
}
EXPORT_SYMBOL(nss_crypto_unregister_user);

static const struct of_device_id nss_crypto_dev_dt_ids[] = {
	{ .compatible =  "qcom,ce5" },
	{ .compatible =  "qcom,eip197" },
	{},
};
MODULE_DEVICE_TABLE(of, nss_crypto_dev_dt_ids);

static const struct of_device_id nss_crypto_dt_ids[] = {
	{ .compatible =  "qcom,nss-crypto" },
	{},
};
MODULE_DEVICE_TABLE(of, nss_crypto_dt_ids);

/*
 * nss_crypto_device_probe()
 *	probe each device node
 */
static int nss_crypto_device_probe(struct platform_device *pdev)
{
	return nss_crypto_hw_init(pdev);
}

/*
 * nss_crypto_device_remove()
 *	remove crypto device and deregister everything
 */
static int nss_crypto_device_remove(struct platform_device *pdev)
{
	nss_crypto_hw_deinit(pdev);
	nss_crypto_node_free(platform_get_drvdata(pdev));
	return 0;
};

/*
 * nss_crypto_device
 *	platform device instance
 */
static struct platform_driver nss_crypto_device = {
	.probe		= nss_crypto_device_probe,
	.remove		= nss_crypto_device_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "nss-crypto-device",
		.of_match_table = of_match_ptr(nss_crypto_dev_dt_ids),
	},
};

/*
 * nss_crypto_probe()
 *	probe the crypto driver
 */
static int nss_crypto_probe(struct platform_device *pdev)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	unsigned long max_contexts;
	struct device_node *np;
	size_t ctx_tbl_sz;

	np = of_node_get(pdev->dev.of_node);

	if (of_property_read_u32(np, "qcom,max-contexts", &ctrl->max_contexts) < 0) {
		nss_crypto_err("%px: max session undefined\n", pdev);
		return -EINVAL;
	}

	/*
	 * max contexts should be a power of 2
	 */
	max_contexts = ctrl->max_contexts;
	if (!is_power_of_2(max_contexts))
		max_contexts = roundup_pow_of_two(max_contexts);

	ctrl->max_contexts = (uint32_t)max_contexts;

	if (!ctrl->max_contexts) {
		nss_crypto_warn("%px: Empty context\n", np);
		return -ENOSPC;
	}

	if (of_property_read_u32(np, "qcom,max-context-size", &ctrl->max_ctx_size) < 0) {
		nss_crypto_err("%px: max session undefined\n", pdev);
		return -EINVAL;
	}

	ctx_tbl_sz = ctrl->max_contexts * sizeof(struct nss_crypto_ctx);
	ctrl->ctx_tbl = vzalloc(ctx_tbl_sz);
	if (!ctrl->ctx_tbl) {
		nss_crypto_err("%px: context table allocation fails\n", pdev);
		return -ENOMEM;
	}

	/*
	 * Add control debugfs and reset control stats
	 */
	nss_crypto_debugfs_add_control(ctrl);
	nss_crypto_ctrl_stats_init(&ctrl->cstats);

	INIT_LIST_HEAD(&ctrl->node_head);
	platform_set_drvdata(pdev, ctrl);

	return of_platform_populate(np, NULL, NULL, &pdev->dev);
}

/*
 * nss_crypto_remove()
 *	remove the crypto driver
 */
static int nss_crypto_remove(struct platform_device *pdev)
{
	struct nss_crypto_ctrl *ctrl = platform_get_drvdata(pdev);

	if (ctrl->ctrl_dentry)
		debugfs_remove_recursive(ctrl->ctrl_dentry);

	if (ctrl->ctx_tbl)
		vfree(ctrl->ctx_tbl);

	/*
	 * Clear the active state of driver
	 */
	ctrl->active = false;
	return 0;
}

/*
 * nss_crypto_drv
 *	platform device instance
 */
static struct platform_driver nss_crypto_drv = {
	.probe		= nss_crypto_probe,
	.remove		= nss_crypto_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "nss-crypto",
		.of_match_table = of_match_ptr(nss_crypto_dt_ids),
	},
};

/*
 * nss_crypto_delayed_probe()
 * 	delayed sequence to initialize crypto after NSS FW is initialized
 */
void nss_crypto_delayed_probe(struct work_struct *work)
{
	struct nss_crypto_ctrl *ctrl;
	struct nss_crypto_user *user;
	struct nss_crypto_user *tmp;
	struct list_head tmp_head;

	ctrl = container_of(to_delayed_work(work), struct nss_crypto_ctrl, probe_work);

	/*
	 * Check if NSS FW is active
	 */
	if (!atomic_read(&ctrl->nss_active)) {
		schedule_delayed_work(&ctrl->probe_work, NSS_CRYPTO_DELAYED_INIT_TICKS);
		return;
	}

	if (platform_driver_register(&nss_crypto_drv)) {
		nss_crypto_warn("%px: unable to register the driver\n", ctrl);
		return;
	}

	if (platform_driver_register(&nss_crypto_device)) {
		nss_crypto_warn("%px: unable to register the device\n", ctrl);
		return;
	}

	/*
	 * We walk the user list to find out those users which have registered
	 * with the crypto driver before the driver was initialized. This is
	 * very much possible as the firmware can take some time to boot and
	 * initialize itself. If, a user registering with crypto driver sees
	 * the driver is not ready would add itself to the pending list and return.
	 * Once, the driver is becomes ready (marked in active flag) all these user
	 * will get notified with the attach. This will be called from the context
	 * of delayed_probe. For any new users registering after the driver is marked
	 * active will not require the list addition. It would get attached in the
	 * same context in which they are registering.
	 */
	mutex_lock(&ctrl->mutex);
	ctrl->active = true;
	INIT_LIST_HEAD(&tmp_head);
	list_splice_tail_init(&ctrl->user_head, &tmp_head);
	mutex_unlock(&ctrl->mutex);

	list_for_each_entry_safe(user, tmp, &tmp_head, list) {
		user->ctx.attach(user->app_data, user);
	}
}

/*
 * nss_crypto_module_init()
 *	module init for crypto driver
 */
static int __init nss_crypto_module_init(void)
{
	struct nss_crypto_ctrl *ctrl = &g_control;

	nss_crypto_info_always("module loaded %s\n", NSS_CRYPTO_BUILD_ID);

	/*
	 * Mark the crypto driver inactive at start. When
	 * probe completes successfully it will mark this
	 * active
	 */
	ctrl->active = false;
	atomic_set(&ctrl->nss_active, 0);

	if (!of_find_compatible_node(NULL, NULL, "qcom,nss-crypto")) {
		nss_crypto_info_always("module loaded for symbol link\n");
		return 0;
	}

	INIT_LIST_HEAD(&ctrl->node_head);
	INIT_LIST_HEAD(&ctrl->user_head);

	mutex_init(&ctrl->mutex);

	ctrl->nss_msg_hdl = nss_crypto_cmn_notify_register(nss_crypto_process_event, ctrl);

	init_completion(&ctrl->complete);
	INIT_DELAYED_WORK(&ctrl->probe_work, nss_crypto_delayed_probe);
	schedule_delayed_work(&ctrl->probe_work, NSS_CRYPTO_DELAYED_INIT_TICKS);

	/*
	 * Non availability of debugfs directory is not a catastrophy
	 * We can still go ahead with other initialization
	 */
	ctrl->dentry = debugfs_create_dir("qca-nss-crypto", NULL);

	return 0;
}
module_init(nss_crypto_module_init);

/*
 * nss_crypto_module_exit()
 *	module exit for crypto driver
 */
static void __exit nss_crypto_module_exit(void)
{
	platform_driver_unregister(&nss_crypto_drv);
	platform_driver_unregister(&nss_crypto_device);
}
module_exit(nss_crypto_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("QCA NSS Crypto driver");
