/* Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
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

#ifndef __NSS_CRYPTOAPI_H
#define __NSS_CRYPTOAPI_H

/**
 * @brief Get the NSS crypto session ID corresponding to the AEAD cipher
 *
 * @param aead[IN] Crypto AEAD cipher
 * @param sid[IN] Pointer to store the session ID
 *
 * @return 0 for success
 */
int nss_cryptoapi_aead_ctx2session(struct crypto_aead *aead, uint32_t *sid);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
/**
 * @brief Get the NSS crypto session ID corresponding to the skcipher
 *
 * @param sk[IN] Crypto skcipher
 * @param sid[IN] Pointer to store the session ID
 *
 * @return 0 for success
 */
int nss_cryptoapi_skcipher_ctx2session(struct crypto_skcipher *sk, uint32_t *sid);
#else
/**
 * @brief Get the NSS crypto session ID corresponding to the ABLK cipher
 *
 * @param ablk[IN] Crypto ABLK cipher
 * @param sid[IN] Pointer to store the session ID
 *
 * @return 0 for success
 */
int nss_cryptoapi_ablk_ctx2session(struct crypto_ablkcipher *ablk, uint32_t *sid);
#endif

/**
 * @brief Get the NSS crypto session ID corresponding to AHASH
 *
 * @param ahash[IN] Crypto AHASH
 * @param sid[IN] Pointer to store the session ID
 *
 * @return 0 for success
 */
int nss_cryptoapi_ahash_ctx2session(struct crypto_ahash *ahash, uint32_t *sid);

/**
 * @brief Check if cryptoAPI driver is registered
 *
 * @return true if registration is done else false
 */
bool nss_cryptoapi_is_registered(void);

#endif /* !__NSS_CRYPTOAPI_H */
