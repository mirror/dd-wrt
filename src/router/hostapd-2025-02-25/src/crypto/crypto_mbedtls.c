/*
 * crypto wrapper functions for mbed TLS
 *
 * SPDX-FileCopyrightText: 2022 Glenn Strauss <gstrauss@gluelogic.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "utils/includes.h"
#include "utils/common.h"

#include <mbedtls/version.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/platform_util.h> /* mbedtls_platform_zeroize() */
#include <mbedtls/asn1.h>
#include <mbedtls/asn1write.h>
#include <mbedtls/aes.h>
#include <mbedtls/md.h>
#include <mbedtls/md5.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha512.h>

#ifndef MBEDTLS_PRIVATE
#define MBEDTLS_PRIVATE(x) x
#endif

/* hostapd/wpa_supplicant provides forced_memzero(),
 * but prefer mbedtls_platform_zeroize() */
#define forced_memzero(ptr,sz) mbedtls_platform_zeroize(ptr,sz)

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __GNUC_PREREQ
#define __GNUC_PREREQ(maj,min) 0
#endif

#ifndef __attribute_cold__
#if __has_attribute(cold) \
 || __GNUC_PREREQ(4,3)
#define __attribute_cold__  __attribute__((__cold__))
#else
#define __attribute_cold__
#endif
#endif

#ifndef __attribute_noinline__
#if __has_attribute(noinline) \
 || __GNUC_PREREQ(3,1)
#define __attribute_noinline__  __attribute__((__noinline__))
#else
#define __attribute_noinline__
#endif
#endif

#include "crypto.h"
#include "aes_wrap.h"
#include "aes.h"
#include "md5.h"
#include "sha1.h"
#include "sha256.h"
#include "sha384.h"
#include "sha512.h"


/*
 * selective code inclusion based on preprocessor defines
 *
 * future: additional code could be wrapped with preprocessor checks if
 * wpa_supplicant/Makefile and hostap/Makefile were more consistent with
 * setting preprocessor defines for named groups of functionality
 */

#if defined(CONFIG_FIPS)
#undef MBEDTLS_MD4_C     /* omit md4_vector() */
#undef MBEDTLS_MD5_C     /* omit md5_vector() hmac_md5_vector() hmac_md5() */
#undef MBEDTLS_DES_C     /* omit des_encrypt() */
#undef MBEDTLS_NIST_KW_C /* omit aes_wrap() aes_unwrap() */
#define CRYPTO_MBEDTLS_CONFIG_FIPS
#endif

#if !defined(CONFIG_FIPS)
#if defined(EAP_PWD) \
 || defined(EAP_LEAP) || defined(EAP_LEAP_DYNAMIC) \
 || defined(EAP_TTLS) || defined(EAP_TTLS_DYNAMIC) \
 || defined(EAP_MSCHAPv2) || defined(EAP_MSCHAPv2_DYNAMIC) \
 || defined(EAP_SERVER_MSCHAPV2)
#ifndef MBEDTLS_MD4_C    /* (MD4 not in mbedtls 3.x) */
#include "md4-internal.c"/* pull in hostap local implementation */
#endif /* md4_vector() */
#else
#undef MBEDTLS_MD4_C     /* omit md4_vector() */
#endif
#endif

#if !defined(CONFIG_NO_RC4) && !defined(CONFIG_NO_WPA)
#ifndef MBEDTLS_ARC4_C   /* (RC4 not in mbedtls 3.x) */
#include "rc4.c"         /* pull in hostap local implementation */
#endif /* rc4_skip() */
#else
#undef MBEDTLS_ARC4_C    /* omit rc4_skip() */
#endif

#if defined(CONFIG_MACSEC)     \
 || defined(CONFIG_NO_RADIUS)  \
 || defined(CONFIG_IEEE80211R) \
 || defined(EAP_SERVER_FAST)   \
 || defined(EAP_SERVER_TEAP)   \
 || !defined(CONFIG_NO_WPA)
       /* aes_wrap() aes_unwrap() */
#else
#undef MBEDTLS_NIST_KW_C /* omit aes_wrap() aes_unwrap() */
#endif

#if !defined(CONFIG_SHA256)
#undef MBEDTLS_SHA256_C
#endif

#if !defined(CONFIG_SHA384) && !defined(CONFIG_SHA512)
#undef MBEDTLS_SHA512_C
#endif

#if defined(CONFIG_HMAC_SHA256_KDF)
#define CRYPTO_MBEDTLS_HMAC_KDF_SHA256
#endif
#if defined(CONFIG_HMAC_SHA384_KDF)
#define CRYPTO_MBEDTLS_HMAC_KDF_SHA384
#endif
#if defined(CONFIG_HMAC_SHA512_KDF)
#define CRYPTO_MBEDTLS_HMAC_KDF_SHA512
#endif

#if defined(EAP_SIM) || defined(EAP_SIM_DYNAMIC) || defined(EAP_SERVER_SIM) \
 || defined(EAP_AKA) || defined(EAP_AKA_DYNAMIC) || defined(EAP_SERVER_AKA)
/* EAP_SIM=y EAP_AKA=y */
#define CRYPTO_MBEDTLS_FIPS186_2_PRF
#endif

#if defined(EAP_FAST) || defined(EAP_FAST_DYNAMIC) || defined(EAP_SERVER_FAST) \
 || defined(EAP_TEAP) || defined(EAP_TEAP_DYNAMIC) || defined(EAP_SERVER_FAST)
#define CRYPTO_MBEDTLS_SHA1_T_PRF
#endif

#if defined(CONFIG_DES)
#define CRYPTO_MBEDTLS_DES_ENCRYPT
#endif /* des_encrypt() */

#if !defined(CONFIG_NO_PBKDF2)
#define CRYPTO_MBEDTLS_PBKDF2_SHA1
#endif /* pbkdf2_sha1() */

#if defined(EAP_IKEV2) \
 || defined(EAP_IKEV2_DYNAMIC) \
 || defined(EAP_SERVER_IKEV2) /* CONFIG_EAP_IKEV2=y */
#define CRYPTO_MBEDTLS_CRYPTO_CIPHER
#endif /* crypto_cipher_*() */

#if defined(EAP_PWD) || defined(EAP_SERVER_PWD) /* CONFIG_EAP_PWD=y */
#define CRYPTO_MBEDTLS_CRYPTO_HASH
#endif /* crypto_hash_*() */

#if defined(EAP_PWD) || defined(EAP_SERVER_PWD) /* CONFIG_EAP_PWD=y */ \
 || defined(CONFIG_SAE) /* CONFIG_SAE=y */
#define CRYPTO_MBEDTLS_CRYPTO_BIGNUM
#endif /* crypto_bignum_*() */

#if defined(EAP_PWD)          /* CONFIG_EAP_PWD=y */    \
 || defined(EAP_EKE)          /* CONFIG_EAP_EKE=y */    \
 || defined(EAP_EKE_DYNAMIC)  /* CONFIG_EAP_EKE=y */    \
 || defined(EAP_SERVER_EKE)   /* CONFIG_EAP_EKE=y */    \
 || defined(EAP_IKEV2)        /* CONFIG_EAP_IKEV2y */   \
 || defined(EAP_IKEV2_DYNAMIC)/* CONFIG_EAP_IKEV2=y */  \
 || defined(EAP_SERVER_IKEV2) /* CONFIG_EAP_IKEV2=y */  \
 || defined(CONFIG_SAE)       /* CONFIG_SAE=y */        \
 || defined(CONFIG_WPS)       /* CONFIG_WPS=y */
#define CRYPTO_MBEDTLS_CRYPTO_DH
#if defined(CONFIG_WPS_NFC)
#define CRYPTO_MBEDTLS_DH5_INIT_FIXED
#endif /* dh5_init_fixed() */
#endif /* crypto_dh_*() */

#if !defined(CONFIG_NO_WPA) /* CONFIG_NO_WPA= */
#define CRYPTO_MBEDTLS_CRYPTO_ECDH
#endif /* crypto_ecdh_*() */

#if defined(CONFIG_ECC)
#define CRYPTO_MBEDTLS_CRYPTO_BIGNUM
#define CRYPTO_MBEDTLS_CRYPTO_EC
#endif /* crypto_ec_*() crypto_ec_key_*() */

#if defined(CONFIG_DPP) /* CONFIG_DPP=y */
#define CRYPTO_MBEDTLS_CRYPTO_EC_DPP /* extra for DPP */
#define CRYPTO_MBEDTLS_CRYPTO_CSR
#endif /* crypto_csr_*() */

#if defined(CONFIG_DPP3) /* CONFIG_DPP3=y */
#define CRYPTO_MBEDTLS_CRYPTO_HPKE
#endif

#if defined(CONFIG_DPP2) /* CONFIG_DPP2=y */
#define CRYPTO_MBEDTLS_CRYPTO_PKCS7
#endif /* crypto_pkcs7_*() */

#if defined(EAP_SIM) || defined(EAP_SIM_DYNAMIC) || defined(EAP_SERVER_SIM) \
 || defined(EAP_AKA) || defined(EAP_AKA_DYNAMIC) || defined(EAP_SERVER_AKA) \
 || defined(CONFIG_AP) || defined(HOSTAPD)
/* CONFIG_EAP_SIM=y CONFIG_EAP_AKA=y CONFIG_AP=y HOSTAPD */
#if defined(CRYPTO_RSA_OAEP_SHA256)
#define CRYPTO_MBEDTLS_CRYPTO_RSA
#endif
#endif /* crypto_rsa_*() */


static int ctr_drbg_init_state;
static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_entropy_context entropy;

#ifdef CRYPTO_MBEDTLS_CRYPTO_BIGNUM
#include <mbedtls/bignum.h>
static mbedtls_mpi mpi_sw_A;
#endif

__attribute_cold__
__attribute_noinline__
static mbedtls_ctr_drbg_context * ctr_drbg_init(void)
{
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);
	if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
	                          NULL, 0)) {
		wpa_printf(MSG_ERROR, "Init of random number generator failed");
		/* XXX: abort? */
	}
	else
		ctr_drbg_init_state = 1;

	return &ctr_drbg;
}

__attribute_cold__
void crypto_unload(void)
{
	if (ctr_drbg_init_state) {
		mbedtls_ctr_drbg_free(&ctr_drbg);
		mbedtls_entropy_free(&entropy);
	  #ifdef CRYPTO_MBEDTLS_CRYPTO_BIGNUM
		mbedtls_mpi_free(&mpi_sw_A);
	  #endif
		ctr_drbg_init_state = 0;
	}
}

/* init ctr_drbg on first use
 * crypto_global_init() and crypto_global_deinit() are not available here
 * (available only when CONFIG_TLS=internal, which is not CONFIG_TLS=mbedtls) */
mbedtls_ctr_drbg_context * crypto_mbedtls_ctr_drbg(void); /*(not in header)*/
inline
mbedtls_ctr_drbg_context * crypto_mbedtls_ctr_drbg(void)
{
	return ctr_drbg_init_state ? &ctr_drbg : ctr_drbg_init();
}

#ifdef CRYPTO_MBEDTLS_CONFIG_FIPS
int crypto_get_random(void *buf, size_t len)
{
	return mbedtls_ctr_drbg_random(crypto_mbedtls_ctr_drbg(),buf,len) ? -1 : 0;
}
#endif


#if 1

/* tradeoff: slightly smaller code size here at cost of slight increase
 * in instructions and function calls at runtime versus the expanded
 * per-message-digest code that follows in #else (~0.5 kib .text larger) */

__attribute_noinline__
static int md_vector(size_t num_elem, const u8 *addr[], const size_t *len,
                     u8 *mac, mbedtls_md_type_t md_type)
{
	if (TEST_FAIL())
		return -1;

	mbedtls_md_context_t ctx;
	mbedtls_md_init(&ctx);
	if (mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0) != 0){
		mbedtls_md_free(&ctx);
		return -1;
	}
	mbedtls_md_starts(&ctx);
	for (size_t i = 0; i < num_elem; ++i)
		mbedtls_md_update(&ctx, addr[i], len[i]);
	mbedtls_md_finish(&ctx, mac);
	mbedtls_md_free(&ctx);
	return 0;
}

#ifdef MBEDTLS_SHA512_C
int sha512_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	return md_vector(num_elem, addr, len, mac, MBEDTLS_MD_SHA512);
}

int sha384_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	return md_vector(num_elem, addr, len, mac, MBEDTLS_MD_SHA384);
}
#endif

#ifdef MBEDTLS_SHA256_C
int sha256_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	return md_vector(num_elem, addr, len, mac, MBEDTLS_MD_SHA256);
}
#endif

#ifdef MBEDTLS_SHA1_C
int sha1_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	return md_vector(num_elem, addr, len, mac, MBEDTLS_MD_SHA1);
}
#endif

#ifdef MBEDTLS_MD5_C
int md5_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	return md_vector(num_elem, addr, len, mac, MBEDTLS_MD_MD5);
}
#endif

#ifdef MBEDTLS_MD4_C
#include <mbedtls/md4.h>
int md4_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	return md_vector(num_elem, addr, len, mac, MBEDTLS_MD_MD4);
}
#endif

#else  /* expanded per-message-digest functions */

#ifdef MBEDTLS_SHA512_C
#include <mbedtls/sha512.h>
__attribute_noinline__
static int sha384_512_vector(size_t num_elem, const u8 *addr[],
                             const size_t *len, u8 *mac, int is384)
{
	if (TEST_FAIL())
		return -1;

	struct mbedtls_sha512_context ctx;
	mbedtls_sha512_init(&ctx);
  #if MBEDTLS_VERSION_MAJOR >= 3
	mbedtls_sha512_starts(&ctx, is384);
	for (size_t i = 0; i < num_elem; ++i)
		mbedtls_sha512_update(&ctx, addr[i], len[i]);
	mbedtls_sha512_finish(&ctx, mac);
  #else
	mbedtls_sha512_starts_ret(&ctx, is384);
	for (size_t i = 0; i < num_elem; ++i)
		mbedtls_sha512_update_ret(&ctx, addr[i], len[i]);
	mbedtls_sha512_finish_ret(&ctx, mac);
  #endif
	mbedtls_sha512_free(&ctx);
	return 0;
}

int sha512_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	return sha384_512_vector(num_elem, addr, len, mac, 0);
}

int sha384_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	return sha384_512_vector(num_elem, addr, len, mac, 1);
}
#endif

#ifdef MBEDTLS_SHA256_C
#include <mbedtls/sha256.h>
int sha256_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	if (TEST_FAIL())
		return -1;

	struct mbedtls_sha256_context ctx;
	mbedtls_sha256_init(&ctx);
  #if MBEDTLS_VERSION_MAJOR >= 3
	mbedtls_sha256_starts(&ctx, 0);
	for (size_t i = 0; i < num_elem; ++i)
		mbedtls_sha256_update(&ctx, addr[i], len[i]);
	mbedtls_sha256_finish(&ctx, mac);
  #else
	mbedtls_sha256_starts_ret(&ctx, 0);
	for (size_t i = 0; i < num_elem; ++i)
		mbedtls_sha256_update_ret(&ctx, addr[i], len[i]);
	mbedtls_sha256_finish_ret(&ctx, mac);
  #endif
	mbedtls_sha256_free(&ctx);
	return 0;
}
#endif

#ifdef MBEDTLS_SHA1_C
#include <mbedtls/sha1.h>
int sha1_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	if (TEST_FAIL())
		return -1;

	struct mbedtls_sha1_context ctx;
	mbedtls_sha1_init(&ctx);
  #if MBEDTLS_VERSION_MAJOR >= 3
	mbedtls_sha1_starts(&ctx);
	for (size_t i = 0; i < num_elem; ++i)
		mbedtls_sha1_update(&ctx, addr[i], len[i]);
	mbedtls_sha1_finish(&ctx, mac);
  #else
	mbedtls_sha1_starts_ret(&ctx);
	for (size_t i = 0; i < num_elem; ++i)
		mbedtls_sha1_update_ret(&ctx, addr[i], len[i]);
	mbedtls_sha1_finish_ret(&ctx, mac);
  #endif
	mbedtls_sha1_free(&ctx);
	return 0;
}
#endif

#ifdef MBEDTLS_MD5_C
#include <mbedtls/md5.h>
int md5_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	if (TEST_FAIL())
		return -1;

	struct mbedtls_md5_context ctx;
	mbedtls_md5_init(&ctx);
  #if MBEDTLS_VERSION_MAJOR >= 3
	mbedtls_md5_starts(&ctx);
	for (size_t i = 0; i < num_elem; ++i)
		mbedtls_md5_update(&ctx, addr[i], len[i]);
	mbedtls_md5_finish(&ctx, mac);
  #else
	mbedtls_md5_starts_ret(&ctx);
	for (size_t i = 0; i < num_elem; ++i)
		mbedtls_md5_update_ret(&ctx, addr[i], len[i]);
	mbedtls_md5_finish_ret(&ctx, mac);
  #endif
	mbedtls_md5_free(&ctx);
	return 0;
}
#endif

#ifdef MBEDTLS_MD4_C
#include <mbedtls/md4.h>
int md4_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	if (TEST_FAIL())
		return -1;

	struct mbedtls_md4_context ctx;
	mbedtls_md4_init(&ctx);
	mbedtls_md4_starts_ret(&ctx);
	for (size_t i = 0; i < num_elem; ++i)
		mbedtls_md4_update_ret(&ctx, addr[i], len[i]);
	mbedtls_md4_finish_ret(&ctx, mac);
	mbedtls_md4_free(&ctx);
	return 0;
}
#endif

#endif /* expanded per-message-digest functions */


__attribute_noinline__
static int hmac_vector(const u8 *key, size_t key_len, size_t num_elem,
                       const u8 *addr[], const size_t *len, u8 *mac,
                       mbedtls_md_type_t md_type)
{
	if (TEST_FAIL())
		return -1;

	mbedtls_md_context_t ctx;
	mbedtls_md_init(&ctx);
	if (mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1) != 0){
		mbedtls_md_free(&ctx);
		return -1;
	}
	mbedtls_md_hmac_starts(&ctx, key, key_len);
	for (size_t i = 0; i < num_elem; ++i)
		mbedtls_md_hmac_update(&ctx, addr[i], len[i]);
	mbedtls_md_hmac_finish(&ctx, mac);
	mbedtls_md_free(&ctx);
	return 0;
}

#ifdef MBEDTLS_SHA512_C
int hmac_sha512_vector(const u8 *key, size_t key_len, size_t num_elem,
                       const u8 *addr[], const size_t *len, u8 *mac)
{
	return hmac_vector(key, key_len, num_elem, addr, len, mac,
			   MBEDTLS_MD_SHA512);
}

int hmac_sha512(const u8 *key, size_t key_len, const u8 *data, size_t data_len,
                u8 *mac)
{
	return hmac_vector(key, key_len, 1, &data, &data_len, mac,
			   MBEDTLS_MD_SHA512);
}

int hmac_sha384_vector(const u8 *key, size_t key_len, size_t num_elem,
                       const u8 *addr[], const size_t *len, u8 *mac)
{
	return hmac_vector(key, key_len, num_elem, addr, len, mac,
			   MBEDTLS_MD_SHA384);
}

int hmac_sha384(const u8 *key, size_t key_len, const u8 *data, size_t data_len,
                u8 *mac)
{
	return hmac_vector(key, key_len, 1, &data, &data_len, mac,
			   MBEDTLS_MD_SHA384);
}
#endif

#ifdef MBEDTLS_SHA256_C
int hmac_sha256_vector(const u8 *key, size_t key_len, size_t num_elem,
                       const u8 *addr[], const size_t *len, u8 *mac)
{
	return hmac_vector(key, key_len, num_elem, addr, len, mac,
			   MBEDTLS_MD_SHA256);
}

int hmac_sha256(const u8 *key, size_t key_len, const u8 *data, size_t data_len,
                u8 *mac)
{
	return hmac_vector(key, key_len, 1, &data, &data_len, mac,
			   MBEDTLS_MD_SHA256);
}
#endif

#ifdef MBEDTLS_SHA1_C
int hmac_sha1_vector(const u8 *key, size_t key_len, size_t num_elem,
                     const u8 *addr[], const size_t *len, u8 *mac)
{
	return hmac_vector(key, key_len, num_elem, addr, len, mac,
			   MBEDTLS_MD_SHA1);
}

int hmac_sha1(const u8 *key, size_t key_len, const u8 *data, size_t data_len,
              u8 *mac)
{
	return hmac_vector(key, key_len, 1, &data, &data_len, mac,
			   MBEDTLS_MD_SHA1);
}
#endif

#ifdef MBEDTLS_MD5_C
int hmac_md5_vector(const u8 *key, size_t key_len, size_t num_elem,
                    const u8 *addr[], const size_t *len, u8 *mac)
{
	return hmac_vector(key, key_len, num_elem, addr, len, mac,
			   MBEDTLS_MD_MD5);
}

int hmac_md5(const u8 *key, size_t key_len, const u8 *data, size_t data_len,
             u8 *mac)
{
	return hmac_vector(key, key_len, 1, &data, &data_len, mac,
			   MBEDTLS_MD_MD5);
}
#endif


#if defined(MBEDTLS_SHA256_C) || defined(MBEDTLS_SHA512_C)

#if defined(CRYPTO_MBEDTLS_HMAC_KDF_SHA256) \
 || defined(CRYPTO_MBEDTLS_HMAC_KDF_SHA384) \
 || defined(CRYPTO_MBEDTLS_HMAC_KDF_SHA512)

#include <mbedtls/hkdf.h>

/* sha256-kdf.c sha384-kdf.c sha512-kdf.c */

/* HMAC-SHA256 KDF (RFC 5295) and HKDF-Expand(SHA256) (RFC 5869) */
/* HMAC-SHA384 KDF (RFC 5295) and HKDF-Expand(SHA384) (RFC 5869) */
/* HMAC-SHA512 KDF (RFC 5295) and HKDF-Expand(SHA512) (RFC 5869) */
__attribute_noinline__
static int hmac_kdf_expand(const u8 *prk, size_t prk_len,
                           const char *label, const u8 *info, size_t info_len,
                           u8 *okm, size_t okm_len, mbedtls_md_type_t md_type)
{
	if (TEST_FAIL())
		return -1;

	const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(md_type);
  #ifdef MBEDTLS_HKDF_C
	if (label == NULL)  /* RFC 5869 HKDF-Expand when (label == NULL) */
		return mbedtls_hkdf_expand(md_info, prk, prk_len, info,
		                           info_len, okm, okm_len) ? -1 : 0;
  #endif

	const size_t mac_len = mbedtls_md_get_size(md_info);
	/* okm_len must not exceed 255 times hash len (RFC 5869 Section 2.3) */
	if (okm_len > ((mac_len << 8) - mac_len))
		return -1;

	mbedtls_md_context_t ctx;
	mbedtls_md_init(&ctx);
	if (mbedtls_md_setup(&ctx, md_info, 1) != 0) {
		mbedtls_md_free(&ctx);
		return -1;
	}
	mbedtls_md_hmac_starts(&ctx, prk, prk_len);

	u8 iter = 1;
	const u8 *addr[4] = { okm, (const u8 *)label, info, &iter };
	size_t len[4] = { 0, label ? os_strlen(label)+1 : 0, info_len, 1 };

	for (; okm_len >= mac_len; okm_len -= mac_len, ++iter) {
		for (size_t i = 0; i < ARRAY_SIZE(addr); ++i)
			mbedtls_md_hmac_update(&ctx, addr[i], len[i]);
		mbedtls_md_hmac_finish(&ctx, okm);
		mbedtls_md_hmac_reset(&ctx);
		addr[0] = okm;
		okm += mac_len;
		len[0] = mac_len; /*(include digest in subsequent rounds)*/
	}

	if (okm_len) {
		u8 hash[MBEDTLS_MD_MAX_SIZE];
		for (size_t i = 0; i < ARRAY_SIZE(addr); ++i)
			mbedtls_md_hmac_update(&ctx, addr[i], len[i]);
		mbedtls_md_hmac_finish(&ctx, hash);
		os_memcpy(okm, hash, okm_len);
		forced_memzero(hash, mac_len);
	}

	mbedtls_md_free(&ctx);
	return 0;
}

#ifdef MBEDTLS_SHA512_C
#ifdef CRYPTO_MBEDTLS_HMAC_KDF_SHA512
int hmac_sha512_kdf(const u8 *secret, size_t secret_len,
		    const char *label, const u8 *seed, size_t seed_len,
		    u8 *out, size_t outlen)
{
	return hmac_kdf_expand(secret, secret_len, label, seed, seed_len,
	                       out, outlen, MBEDTLS_MD_SHA512);
}
#endif

#ifdef CRYPTO_MBEDTLS_HMAC_KDF_SHA384
int hmac_sha384_kdf(const u8 *secret, size_t secret_len,
		    const char *label, const u8 *seed, size_t seed_len,
		    u8 *out, size_t outlen)
{
	return hmac_kdf_expand(secret, secret_len, label, seed, seed_len,
	                       out, outlen, MBEDTLS_MD_SHA384);
}
#endif
#endif

#ifdef MBEDTLS_SHA256_C
#ifdef CRYPTO_MBEDTLS_HMAC_KDF_SHA256
int hmac_sha256_kdf(const u8 *secret, size_t secret_len,
		    const char *label, const u8 *seed, size_t seed_len,
		    u8 *out, size_t outlen)
{
	return hmac_kdf_expand(secret, secret_len, label, seed, seed_len,
	                       out, outlen, MBEDTLS_MD_SHA256);
}
#endif
#endif

#endif /* CRYPTO_MBEDTLS_HMAC_KDF_* */


/* sha256-prf.c sha384-prf.c sha512-prf.c */

/* hmac_prf_bits - IEEE Std 802.11ac-2013, 11.6.1.7.2 Key derivation function */
__attribute_noinline__
static int hmac_prf_bits(const u8 *key, size_t key_len, const char *label,
                         const u8 *data, size_t data_len, u8 *buf,
                         size_t buf_len_bits, mbedtls_md_type_t md_type)
{
	if (TEST_FAIL())
		return -1;

	mbedtls_md_context_t ctx;
	mbedtls_md_init(&ctx);
	const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(md_type);
	if (mbedtls_md_setup(&ctx, md_info, 1) != 0) {
		mbedtls_md_free(&ctx);
		return -1;
	}
	mbedtls_md_hmac_starts(&ctx, key, key_len);

	u16 ctr, n_le = host_to_le16(buf_len_bits);
	const u8 * const addr[] = { (u8 *)&ctr,(u8 *)label,data,(u8 *)&n_le };
	const size_t len[] =      { 2, os_strlen(label), data_len, 2 };
	const size_t mac_len = mbedtls_md_get_size(md_info);
	size_t buf_len = (buf_len_bits + 7) / 8;
	for (ctr = 1; buf_len >= mac_len; buf_len -= mac_len, ++ctr) {
	  #if __BYTE_ORDER == __BIG_ENDIAN
		ctr = host_to_le16(ctr);
	  #endif
		for (size_t i = 0; i < ARRAY_SIZE(addr); ++i)
			mbedtls_md_hmac_update(&ctx, addr[i], len[i]);
		mbedtls_md_hmac_finish(&ctx, buf);
		mbedtls_md_hmac_reset(&ctx);
		buf += mac_len;
	  #if __BYTE_ORDER == __BIG_ENDIAN
		ctr = le_to_host16(ctr);
	  #endif
	}

	if (buf_len) {
		u8 hash[MBEDTLS_MD_MAX_SIZE];
	  #if __BYTE_ORDER == __BIG_ENDIAN
		ctr = host_to_le16(ctr);
	  #endif
		for (size_t i = 0; i < ARRAY_SIZE(addr); ++i)
			mbedtls_md_hmac_update(&ctx, addr[i], len[i]);
		mbedtls_md_hmac_finish(&ctx, hash);
		os_memcpy(buf, hash, buf_len);
		buf += buf_len;
		forced_memzero(hash, mac_len);
	}

	/* Mask out unused bits in last octet if it does not use all the bits */
	if ((buf_len_bits &= 0x7))
		buf[-1] &= (u8)(0xff << (8 - buf_len_bits));

	mbedtls_md_free(&ctx);
	return 0;
}

#ifdef MBEDTLS_SHA512_C
int sha512_prf(const u8 *key, size_t key_len, const char *label,
               const u8 *data, size_t data_len, u8 *buf, size_t buf_len)
{
	return hmac_prf_bits(key, key_len, label, data, data_len, buf,
	                     buf_len * 8, MBEDTLS_MD_SHA512);
}

int sha384_prf(const u8 *key, size_t key_len, const char *label,
               const u8 *data, size_t data_len, u8 *buf, size_t buf_len)
{
	return hmac_prf_bits(key, key_len, label, data, data_len, buf,
	                     buf_len * 8, MBEDTLS_MD_SHA384);
}
#endif

#ifdef MBEDTLS_SHA256_C
int sha256_prf(const u8 *key, size_t key_len, const char *label,
               const u8 *data, size_t data_len, u8 *buf, size_t buf_len)
{
	return hmac_prf_bits(key, key_len, label, data, data_len, buf,
	                     buf_len * 8, MBEDTLS_MD_SHA256);
}

int sha256_prf_bits(const u8 *key, size_t key_len, const char *label,
                    const u8 *data, size_t data_len, u8 *buf,
                    size_t buf_len_bits)
{
	return hmac_prf_bits(key, key_len, label, data, data_len, buf,
	                     buf_len_bits, MBEDTLS_MD_SHA256);
}
#endif

#endif /* MBEDTLS_SHA256_C || MBEDTLS_SHA512_C */


#ifdef MBEDTLS_SHA1_C

/* sha1-prf.c */

/* sha1_prf - SHA1-based Pseudo-Random Function (PRF) (IEEE 802.11i, 8.5.1.1) */

int sha1_prf(const u8 *key, size_t key_len, const char *label,
	     const u8 *data, size_t data_len, u8 *buf, size_t buf_len)
{
	/*(note: algorithm differs from hmac_prf_bits() */
	/*(note: smaller code size instead of expanding hmac_sha1_vector()
	 * as is done in hmac_prf_bits(); not expecting large num of loops) */
	u8 counter = 0;
	const u8 *addr[] = { (u8 *)label, data, &counter };
	const size_t len[] = { os_strlen(label)+1, data_len, 1 };

	for (; buf_len >= SHA1_MAC_LEN; buf_len -= SHA1_MAC_LEN, ++counter) {
		if (hmac_sha1_vector(key, key_len, 3, addr, len, buf))
			return -1;
		buf += SHA1_MAC_LEN;
	}

	if (buf_len) {
		u8 hash[SHA1_MAC_LEN];
		if (hmac_sha1_vector(key, key_len, 3, addr, len, hash))
			return -1;
		os_memcpy(buf, hash, buf_len);
		forced_memzero(hash, sizeof(hash));
	}

	return 0;
}

#ifdef CRYPTO_MBEDTLS_SHA1_T_PRF

/* sha1-tprf.c */

/* sha1_t_prf - EAP-FAST Pseudo-Random Function (T-PRF) (RFC 4851,Section 5.5)*/

int sha1_t_prf(const u8 *key, size_t key_len, const char *label,
	       const u8 *seed, size_t seed_len, u8 *buf, size_t buf_len)
{
	/*(note: algorithm differs from hmac_prf_bits() and hmac_kdf() above)*/
	/*(note: smaller code size instead of expanding hmac_sha1_vector()
	 * as is done in hmac_prf_bits(); not expecting large num of loops) */
	u8 ctr;
	u16 olen = host_to_be16(buf_len);
	const u8 *addr[] = { buf, (u8 *)label, seed, (u8 *)&olen, &ctr };
	size_t len[] = { 0, os_strlen(label)+1, seed_len, 2, 1 };

	for (ctr = 1; buf_len >= SHA1_MAC_LEN; buf_len -= SHA1_MAC_LEN, ++ctr) {
		if (hmac_sha1_vector(key, key_len, 5, addr, len, buf))
			return -1;
		addr[0] = buf;
		buf += SHA1_MAC_LEN;
		len[0] = SHA1_MAC_LEN; /*(include digest in subsequent rounds)*/
	}

	if (buf_len) {
		u8 hash[SHA1_MAC_LEN];
		if (hmac_sha1_vector(key, key_len, 5, addr, len, hash))
			return -1;
		os_memcpy(buf, hash, buf_len);
		forced_memzero(hash, sizeof(hash));
	}

	return 0;
}

#endif /* CRYPTO_MBEDTLS_SHA1_T_PRF */

#ifdef CRYPTO_MBEDTLS_FIPS186_2_PRF

/* fips_prf_internal.c sha1-internal.c */

/* used only by src/eap_common/eap_sim_common.c:eap_sim_prf()
 * for eap_sim_derive_keys() and eap_sim_derive_keys_reauth()
 * where xlen is 160 */

int fips186_2_prf(const u8 *seed, size_t seed_len, u8 *x, size_t xlen)
{
	/* FIPS 186-2 + change notice 1 */

	mbedtls_sha1_context ctx;
	u8 * const xkey = ctx.MBEDTLS_PRIVATE(buffer);
	u32 * const xstate = ctx.MBEDTLS_PRIVATE(state);
	const u32 xstate_init[] =
	  { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };

	mbedtls_sha1_init(&ctx);
	os_memcpy(xkey, seed, seed_len < 64 ? seed_len : 64);

	/* note: does not fill extra bytes if (xlen % 20) (SHA1_MAC_LEN) */
	for (; xlen >= 20; xlen -= 20) {
		/* XSEED_j = 0 */
		/* XVAL = (XKEY + XSEED_j) mod 2^b */

		/* w_i = G(t, XVAL) */
		os_memcpy(xstate, xstate_init, sizeof(xstate_init));
		mbedtls_internal_sha1_process(&ctx, xkey);

	  #if __BYTE_ORDER == __LITTLE_ENDIAN
		xstate[0] = host_to_be32(xstate[0]);
		xstate[1] = host_to_be32(xstate[1]);
		xstate[2] = host_to_be32(xstate[2]);
		xstate[3] = host_to_be32(xstate[3]);
		xstate[4] = host_to_be32(xstate[4]);
	  #endif
		os_memcpy(x, xstate, 20);
		if (xlen == 20) /*(done; skip prep for next loop)*/
			break;

		/* XKEY = (1 + XKEY + w_i) mod 2^b */
		for (u32 carry = 1, k = 20; k-- > 0; carry >>= 8)
			xkey[k] = (carry += xkey[k] + x[k]) & 0xff;
		x += 20;
		/* x_j = w_0|w_1 (each pair of iterations through loop)*/
	}

	mbedtls_sha1_free(&ctx);
	return 0;
}

#endif /* CRYPTO_MBEDTLS_FIPS186_2_PRF */

#endif /* MBEDTLS_SHA1_C */


#ifdef CRYPTO_MBEDTLS_DES_ENCRYPT
#ifdef MBEDTLS_DES_C
#include <mbedtls/des.h>
int des_encrypt(const u8 *clear, const u8 *key, u8 *cypher)
{
	u8 pkey[8], next, tmp;
	int i;

	/* Add parity bits to the key */
	next = 0;
	for (i = 0; i < 7; i++) {
		tmp = key[i];
		pkey[i] = (tmp >> i) | next | 1;
		next = tmp << (7 - i);
	}
	pkey[i] = next | 1;

	mbedtls_des_context des;
	mbedtls_des_init(&des);
	int ret = mbedtls_des_setkey_enc(&des, pkey)
	       || mbedtls_des_crypt_ecb(&des, clear, cypher) ? -1 : 0;
	mbedtls_des_free(&des);
	return ret;
}
#else
#include "des-internal.c"/* pull in hostap local implementation */
#endif
#endif


#ifdef CRYPTO_MBEDTLS_PBKDF2_SHA1
/* sha1-pbkdf2.c */
#include <mbedtls/pkcs5.h>
int pbkdf2_sha1(const char *passphrase, const u8 *ssid, size_t ssid_len,
                int iterations, u8 *buf, size_t buflen)
{
  #if MBEDTLS_VERSION_NUMBER >= 0x03020200 /* mbedtls 3.2.2 */
	return mbedtls_pkcs5_pbkdf2_hmac_ext(MBEDTLS_MD_SHA1,
			(const u8 *)passphrase, os_strlen(passphrase),
			ssid, ssid_len, iterations, 32, buf) ? -1 : 0;
  #else
	const mbedtls_md_info_t *md_info;
	md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
	if (md_info == NULL)
		return -1;
	mbedtls_md_context_t ctx;
	mbedtls_md_init(&ctx);
	int ret = mbedtls_md_setup(&ctx, md_info, 1)
	       || mbedtls_pkcs5_pbkdf2_hmac(&ctx,
			(const u8 *)passphrase, os_strlen(passphrase),
			ssid, ssid_len, iterations, 32, buf) ? -1 : 0;
	mbedtls_md_free(&ctx);
	return ret;
  #endif
}
#endif


/*#include "aes.h"*/ /* prototypes also included in "crypto.h" */

static void *aes_crypt_init_mode(const u8 *key, size_t len, int mode)
{
	if (TEST_FAIL())
		return NULL;

	mbedtls_aes_context *aes = os_malloc(sizeof(*aes));
	if (!aes)
		return NULL;

	mbedtls_aes_init(aes);
	if ((mode == MBEDTLS_AES_ENCRYPT
	    ? mbedtls_aes_setkey_enc(aes, key, len * 8)
	    : mbedtls_aes_setkey_dec(aes, key, len * 8)) == 0)
		return aes;

	mbedtls_aes_free(aes);
	os_free(aes);
	return NULL;
}

void *aes_encrypt_init(const u8 *key, size_t len)
{
	return aes_crypt_init_mode(key, len, MBEDTLS_AES_ENCRYPT);
}

int aes_encrypt(void *ctx, const u8 *plain, u8 *crypt)
{
	return mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, plain, crypt);
}

void aes_encrypt_deinit(void *ctx)
{
	mbedtls_aes_free(ctx);
	os_free(ctx);
}

void *aes_decrypt_init(const u8 *key, size_t len)
{
	return aes_crypt_init_mode(key, len, MBEDTLS_AES_DECRYPT);
}

int aes_decrypt(void *ctx, const u8 *crypt, u8 *plain)
{
	return mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_DECRYPT, crypt, plain);
}

void aes_decrypt_deinit(void *ctx)
{
	mbedtls_aes_free(ctx);
	os_free(ctx);
}


#include "aes_wrap.h"


#ifdef MBEDTLS_NIST_KW_C

#include <mbedtls/nist_kw.h>

/* aes-wrap.c */
int aes_wrap(const u8 *kek, size_t kek_len, int n, const u8 *plain, u8 *cipher)
{
	if (TEST_FAIL())
		return -1;

	mbedtls_nist_kw_context ctx;
	mbedtls_nist_kw_init(&ctx);
	size_t olen;
	int ret = mbedtls_nist_kw_setkey(&ctx, MBEDTLS_CIPHER_ID_AES,
	                                 kek, kek_len*8, 1)
	       || mbedtls_nist_kw_wrap(&ctx, MBEDTLS_KW_MODE_KW, plain, n*8,
	                               cipher, &olen, (n+1)*8) ? -1 : 0;
	mbedtls_nist_kw_free(&ctx);
	return ret;
}

/* aes-unwrap.c */
int aes_unwrap(const u8 *kek, size_t kek_len, int n, const u8 *cipher, u8 *plain)
{
	if (TEST_FAIL())
		return -1;

	mbedtls_nist_kw_context ctx;
	mbedtls_nist_kw_init(&ctx);
	size_t olen;
	int ret = mbedtls_nist_kw_setkey(&ctx, MBEDTLS_CIPHER_ID_AES,
	                                 kek, kek_len*8, 0)
	       || mbedtls_nist_kw_unwrap(&ctx, MBEDTLS_KW_MODE_KW, cipher,
	                                 (n+1)*8, plain, &olen, n*8) ? -1 : 0;
	mbedtls_nist_kw_free(&ctx);
	return ret;
}

#else

#ifndef CRYPTO_MBEDTLS_CONFIG_FIPS
#include "aes-wrap.c"    /* pull in hostap local implementation */
#include "aes-unwrap.c"  /* pull in hostap local implementation */
#endif

#endif /* MBEDTLS_NIST_KW_C */


#ifdef MBEDTLS_CMAC_C

/* aes-omac1.c */

#include <mbedtls/cmac.h>

int omac1_aes_vector(
    const u8 *key, size_t key_len, size_t num_elem, const u8 *addr[],
    const size_t *len, u8 *mac)
{
	if (TEST_FAIL())
		return -1;

	mbedtls_cipher_type_t cipher_type;
	switch (key_len) {
	case 16: cipher_type = MBEDTLS_CIPHER_AES_128_ECB; break;
	case 24: cipher_type = MBEDTLS_CIPHER_AES_192_ECB; break;
	case 32: cipher_type = MBEDTLS_CIPHER_AES_256_ECB; break;
	default: return -1;
	}
	const mbedtls_cipher_info_t *cipher_info;
	cipher_info = mbedtls_cipher_info_from_type(cipher_type);
	if (cipher_info == NULL)
		return -1;

	mbedtls_cipher_context_t ctx;
	mbedtls_cipher_init(&ctx);
	int ret = -1;
	if (mbedtls_cipher_setup(&ctx, cipher_info) == 0
	    && mbedtls_cipher_cmac_starts(&ctx, key, key_len*8) == 0) {
		ret = 0;
		for (size_t i = 0; i < num_elem && ret == 0; ++i)
			ret = mbedtls_cipher_cmac_update(&ctx, addr[i], len[i]);
	}
	if (ret == 0)
		ret = mbedtls_cipher_cmac_finish(&ctx, mac);
	mbedtls_cipher_free(&ctx);
	return ret ? -1 : 0;
}

int omac1_aes_128_vector(const u8 *key, size_t num_elem,
			 const u8 *addr[], const size_t *len,
			 u8 *mac)
{
	return omac1_aes_vector(key, 16, num_elem, addr, len, mac);
}

int omac1_aes_128(const u8 *key, const u8 *data, size_t data_len, u8 *mac)
{
	return omac1_aes_vector(key, 16, 1, &data, &data_len, mac);
}

int omac1_aes_256(const u8 *key, const u8 *data, size_t data_len, u8 *mac)
{
	return omac1_aes_vector(key, 32, 1, &data, &data_len, mac);
}

#else

#include "aes-omac1.c"  /* pull in hostap local implementation */

#ifndef MBEDTLS_AES_BLOCK_SIZE
#define MBEDTLS_AES_BLOCK_SIZE 16
#endif

#endif /* MBEDTLS_CMAC_C */


/* These interfaces can be inefficient when used in loops, as the overhead of
 * initialization each call is large for each block input (e.g. 16 bytes) */


/* aes-encblock.c */
int aes_128_encrypt_block(const u8 *key, const u8 *in, u8 *out)
{
	if (TEST_FAIL())
		return -1;

	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);
	int ret = mbedtls_aes_setkey_enc(&aes, key, 128)
	       || mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, in, out)
	  ? -1
	  : 0;
	mbedtls_aes_free(&aes);
	return ret;
}


/* aes-ctr.c */
int aes_ctr_encrypt(const u8 *key, size_t key_len, const u8 *nonce,
		    u8 *data, size_t data_len)
{
	if (TEST_FAIL())
		return -1;

	unsigned char counter[MBEDTLS_AES_BLOCK_SIZE];
	unsigned char stream_block[MBEDTLS_AES_BLOCK_SIZE];
	os_memcpy(counter, nonce, MBEDTLS_AES_BLOCK_SIZE);/*(must be writable)*/

	mbedtls_aes_context ctx;
	mbedtls_aes_init(&ctx);
	size_t nc_off = 0;
	int ret = mbedtls_aes_setkey_enc(&ctx, key, key_len*8)
	       || mbedtls_aes_crypt_ctr(&ctx, data_len, &nc_off,
	                                counter, stream_block,
	                                data, data) ? -1 : 0;
	forced_memzero(stream_block, sizeof(stream_block));
	mbedtls_aes_free(&ctx);
	return ret;
}

int aes_128_ctr_encrypt(const u8 *key, const u8 *nonce,
			u8 *data, size_t data_len)
{
	return aes_ctr_encrypt(key, 16, nonce, data, data_len);
}


/* aes-cbc.c */
static int aes_128_cbc_oper(const u8 *key, const u8 *iv,
                            u8 *data, size_t data_len, int mode)
{
	unsigned char ivec[MBEDTLS_AES_BLOCK_SIZE];
	os_memcpy(ivec, iv, MBEDTLS_AES_BLOCK_SIZE); /*(must be writable)*/

	mbedtls_aes_context ctx;
	mbedtls_aes_init(&ctx);
	int ret = (mode == MBEDTLS_AES_ENCRYPT
	           ? mbedtls_aes_setkey_enc(&ctx, key, 128)
	           : mbedtls_aes_setkey_dec(&ctx, key, 128))
	       || mbedtls_aes_crypt_cbc(&ctx, mode, data_len, ivec, data, data);
	mbedtls_aes_free(&ctx);
	return ret ? -1 : 0;
}

int aes_128_cbc_encrypt(const u8 *key, const u8 *iv, u8 *data, size_t data_len)
{
	if (TEST_FAIL())
		return -1;

	return aes_128_cbc_oper(key, iv, data, data_len, MBEDTLS_AES_ENCRYPT);
}

int aes_128_cbc_decrypt(const u8 *key, const u8 *iv, u8 *data, size_t data_len)
{
	if (TEST_FAIL())
		return -1;

	return aes_128_cbc_oper(key, iv, data, data_len, MBEDTLS_AES_DECRYPT);
}


/*
 * Much of the following is documented in crypto.h as for CONFIG_TLS=internal
 * but such comments are not accurate:
 *
 * "This function is only used with internal TLSv1 implementation
 *  (CONFIG_TLS=internal). If that is not used, the crypto wrapper does not need
 *  to implement this."
 */


#ifdef CRYPTO_MBEDTLS_CRYPTO_CIPHER

#include <mbedtls/cipher.h>

struct crypto_cipher
{
	mbedtls_cipher_context_t ctx_enc;
	mbedtls_cipher_context_t ctx_dec;
};

struct crypto_cipher * crypto_cipher_init(enum crypto_cipher_alg alg,
					  const u8 *iv, const u8 *key,
					  size_t key_len)
{
	/* IKEv2 src/eap_common/ikev2_common.c:ikev2_{encr,decr}_encrypt()
	 * uses one of CRYPTO_CIPHER_ALG_AES or CRYPTO_CIPHER_ALG_3DES */

	mbedtls_cipher_type_t cipher_type;
	size_t iv_len;
	switch (alg) {
  #ifdef MBEDTLS_ARC4_C
  #if 0
	case CRYPTO_CIPHER_ALG_RC4:
		cipher_type = MBEDTLS_CIPHER_ARC4_128;
		iv_len = 0;
		break;
  #endif
  #endif
  #ifdef MBEDTLS_AES_C
	case CRYPTO_CIPHER_ALG_AES:
		if (key_len == 16) cipher_type = MBEDTLS_CIPHER_AES_128_CTR;
		if (key_len == 24) cipher_type = MBEDTLS_CIPHER_AES_192_CTR;
		if (key_len == 32) cipher_type = MBEDTLS_CIPHER_AES_256_CTR;
		iv_len = 16;
		break;
  #endif
  #ifdef MBEDTLS_DES_C
	case CRYPTO_CIPHER_ALG_3DES:
		cipher_type = MBEDTLS_CIPHER_DES_EDE3_CBC;
		iv_len = 8;
		break;
  #if 0
	case CRYPTO_CIPHER_ALG_DES:
		cipher_type = MBEDTLS_CIPHER_DES_CBC;
		iv_len = 8;
		break;
  #endif
  #endif
	default:
		return NULL;
	}

	const mbedtls_cipher_info_t *cipher_info;
	cipher_info = mbedtls_cipher_info_from_type(cipher_type);
	if (cipher_info == NULL)
		return NULL;

	key_len *= 8; /* key_bitlen */
  #if 0 /*(were key_bitlen not already available)*/
  #if MBEDTLS_VERSION_NUMBER >= 0x03010000 /* mbedtls 3.1.0 */
	key_len = mbedtls_cipher_info_get_key_bitlen(cipher_info);
  #else
	key_len = cipher_info->MBEDTLS_PRIVATE(key_bitlen);
  #endif
  #endif

  #if 0 /*(were iv_len not known above, would need MBEDTLS_PRIVATE(iv_size))*/
	iv_len = cipher_info->MBEDTLS_PRIVATE(iv_size);
  #endif

	struct crypto_cipher *ctx = os_malloc(sizeof(*ctx));
	if (!ctx)
		return NULL;

	mbedtls_cipher_init(&ctx->ctx_enc);
	mbedtls_cipher_init(&ctx->ctx_dec);
	if (   mbedtls_cipher_setup(&ctx->ctx_enc,cipher_info) == 0
	    && mbedtls_cipher_setup(&ctx->ctx_dec,cipher_info) == 0
	    && mbedtls_cipher_setkey(&ctx->ctx_enc,key,key_len,MBEDTLS_ENCRYPT) == 0
	    && mbedtls_cipher_setkey(&ctx->ctx_dec,key,key_len,MBEDTLS_DECRYPT) == 0
	    && mbedtls_cipher_set_iv(&ctx->ctx_enc,iv,iv_len) == 0
	    && mbedtls_cipher_set_iv(&ctx->ctx_dec,iv,iv_len) == 0
	    && mbedtls_cipher_reset(&ctx->ctx_enc) == 0
	    && mbedtls_cipher_reset(&ctx->ctx_dec) == 0) {
		return ctx;
	}

	mbedtls_cipher_free(&ctx->ctx_enc);
	mbedtls_cipher_free(&ctx->ctx_dec);
	os_free(ctx);
	return NULL;
}

int crypto_cipher_encrypt(struct crypto_cipher *ctx,
			  const u8 *plain, u8 *crypt, size_t len)
{
	size_t olen = 0; /*(poor interface above; unknown size of u8 *crypt)*/
	return (mbedtls_cipher_update(&ctx->ctx_enc, plain, len, crypt, &olen)
	        || mbedtls_cipher_finish(&ctx->ctx_enc, crypt + olen, &olen)) ? -1 : 0;
}

int crypto_cipher_decrypt(struct crypto_cipher *ctx,
			  const u8 *crypt, u8 *plain, size_t len)
{
	size_t olen = 0; /*(poor interface above; unknown size of u8 *plain)*/
	return (mbedtls_cipher_update(&ctx->ctx_dec, crypt, len, plain, &olen)
	        || mbedtls_cipher_finish(&ctx->ctx_dec, plain + olen, &olen)) ? -1 : 0;
}

void crypto_cipher_deinit(struct crypto_cipher *ctx)
{
	mbedtls_cipher_free(&ctx->ctx_enc);
	mbedtls_cipher_free(&ctx->ctx_dec);
	os_free(ctx);
}

#endif /* CRYPTO_MBEDTLS_CRYPTO_CIPHER */


#ifdef CRYPTO_MBEDTLS_CRYPTO_HASH

struct crypto_hash * crypto_hash_init(enum crypto_hash_alg alg, const u8 *key,
				      size_t key_len)
{
	mbedtls_md_type_t md_type;
	int is_hmac = 0;

	switch (alg) {
  #ifdef MBEDTLS_MD5_C
	case CRYPTO_HASH_ALG_MD5:
		md_type = MBEDTLS_MD_MD5;
		break;
  #endif
  #ifdef MBEDTLS_SHA1_C
	case CRYPTO_HASH_ALG_SHA1:
		md_type = MBEDTLS_MD_SHA1;
		break;
  #endif
  #ifdef MBEDTLS_MD5_C
	case CRYPTO_HASH_ALG_HMAC_MD5:
		md_type = MBEDTLS_MD_MD5;
		is_hmac = 1;
		break;
  #endif
  #ifdef MBEDTLS_SHA1_C
	case CRYPTO_HASH_ALG_HMAC_SHA1:
		md_type = MBEDTLS_MD_SHA1;
		is_hmac = 1;
		break;
  #endif
  #ifdef MBEDTLS_SHA256_C
	case CRYPTO_HASH_ALG_SHA256:
		md_type = MBEDTLS_MD_SHA256;
		break;
	case CRYPTO_HASH_ALG_HMAC_SHA256:
		md_type = MBEDTLS_MD_SHA256;
		is_hmac = 1;
		break;
  #endif
  #ifdef MBEDTLS_SHA512_C
	case CRYPTO_HASH_ALG_SHA384:
		md_type = MBEDTLS_MD_SHA384;
		break;
	case CRYPTO_HASH_ALG_SHA512:
		md_type = MBEDTLS_MD_SHA512;
		break;
  #endif
	default:
		return NULL;
	}

	const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(md_type);
	if (!md_info)
		return NULL;

	mbedtls_md_context_t *mctx = os_malloc(sizeof(*mctx));
	if (mctx == NULL)
		return NULL;

	mbedtls_md_init(mctx);
	if (mbedtls_md_setup(mctx, md_info, is_hmac) != 0) {
		os_free(mctx);
		return NULL;
	}

	if (is_hmac)
		mbedtls_md_hmac_starts(mctx, key, key_len);
	else
		mbedtls_md_starts(mctx);
	return (struct crypto_hash *)((uintptr_t)mctx | is_hmac);
}

void crypto_hash_update(struct crypto_hash *ctx, const u8 *data, size_t len)
{
	mbedtls_md_context_t *mctx = (mbedtls_md_context_t*)((uintptr_t)ctx & ~1uL);
  #if 0
	/*(mbedtls_md_hmac_update() and mbedtls_md_update()
	 * make same modifications under the hood in mbedtls)*/
	if ((uintptr_t)ctx & 1uL)
		mbedtls_md_hmac_update(mctx, data, len);
	else
  #endif
		mbedtls_md_update(mctx, data, len);
}

int crypto_hash_finish(struct crypto_hash *ctx, u8 *mac, size_t *len)
{
	mbedtls_md_context_t *mctx = (mbedtls_md_context_t*)((uintptr_t)ctx & ~1uL);
	if (mac != NULL && len != NULL) { /*(NULL if caller just freeing context)*/
	  #if MBEDTLS_VERSION_NUMBER >= 0x03020000 /* mbedtls 3.2.0 */
		const mbedtls_md_info_t *md_info = mbedtls_md_info_from_ctx(mctx);
	  #else
		const mbedtls_md_info_t *md_info = mctx->MBEDTLS_PRIVATE(md_info);
	  #endif
		size_t maclen = mbedtls_md_get_size(md_info);
		if (*len < maclen) {
			*len = maclen;
			/*(note: ctx not freed; can call again with larger *len)*/
			return -1;
		}
		*len = maclen;
		if ((uintptr_t)ctx & 1uL)
			mbedtls_md_hmac_finish(mctx, mac);
		else
			mbedtls_md_finish(mctx, mac);
	}
	mbedtls_md_free(mctx);
	os_free(mctx);

	if (TEST_FAIL())
		return -1;

	return 0;
}

#endif /* CRYPTO_MBEDTLS_CRYPTO_HASH */


#ifdef CRYPTO_MBEDTLS_CRYPTO_BIGNUM

#include <mbedtls/bignum.h>

/* crypto.h bignum interfaces */

struct crypto_bignum *crypto_bignum_init(void)
{
	if (TEST_FAIL())
		return NULL;

	mbedtls_mpi *bn = os_malloc(sizeof(*bn));
	if (bn)
		mbedtls_mpi_init(bn);
	return (struct crypto_bignum *)bn;
}

struct crypto_bignum *crypto_bignum_init_set(const u8 *buf, size_t len)
{
	if (TEST_FAIL())
		return NULL;

	mbedtls_mpi *bn = os_malloc(sizeof(*bn));
	if (bn) {
		mbedtls_mpi_init(bn);
		if (mbedtls_mpi_read_binary(bn, buf, len) == 0)
			return (struct crypto_bignum *)bn;
	}

	os_free(bn);
	return NULL;
}

struct crypto_bignum *crypto_bignum_init_uint(unsigned int val)
{
	if (TEST_FAIL())
		return NULL;

  #if 0 /*(hostap use of this interface passes int, not uint)*/
	val = host_to_be32(val);
	return crypto_bignum_init_set((const u8 *)&val, sizeof(val));
  #else
	mbedtls_mpi *bn = os_malloc(sizeof(*bn));
	if (bn) {
		mbedtls_mpi_init(bn);
		if (mbedtls_mpi_lset(bn, (int)val) == 0)
			return (struct crypto_bignum *)bn;
	}

	os_free(bn);
	return NULL;
  #endif
}

void crypto_bignum_deinit(struct crypto_bignum *n, int clear)
{
	mbedtls_mpi_free((mbedtls_mpi *)n);
	os_free(n);
}

int crypto_bignum_to_bin(const struct crypto_bignum *a,
			 u8 *buf, size_t buflen, size_t padlen)
{
	if (TEST_FAIL())
		return -1;

	size_t n = mbedtls_mpi_size((mbedtls_mpi *)a);
	if (n < padlen)
		n = padlen;
	return n > buflen || mbedtls_mpi_write_binary((mbedtls_mpi *)a, buf, n)
	  ? -1
	  : (int)(n);
}

int crypto_bignum_rand(struct crypto_bignum *r, const struct crypto_bignum *m)
{
	if (TEST_FAIL())
		return -1;

	/*assert(r != m);*//* r must not be same as m for mbedtls_mpi_random()*/
  #if MBEDTLS_VERSION_NUMBER >= 0x021B0000 /* mbedtls 2.27.0 */
	return mbedtls_mpi_random((mbedtls_mpi *)r, 0, (mbedtls_mpi *)m,
				  mbedtls_ctr_drbg_random,
				  crypto_mbedtls_ctr_drbg()) ? -1 : 0;
  #else
	/* (needed by EAP_PWD, SAE, DPP) */
	wpa_printf(MSG_ERROR,
	           "mbedtls 2.27.0 or later required for mbedtls_mpi_random()");
	return -1;
  #endif
}

int crypto_bignum_add(const struct crypto_bignum *a,
		      const struct crypto_bignum *b,
		      struct crypto_bignum *c)
{
	return mbedtls_mpi_add_mpi((mbedtls_mpi *)c,
				   (const mbedtls_mpi *)a,
				   (const mbedtls_mpi *)b) ? -1 : 0;
}

int crypto_bignum_mod(const struct crypto_bignum *a,
		      const struct crypto_bignum *b,
		      struct crypto_bignum *c)
{
	return mbedtls_mpi_mod_mpi((mbedtls_mpi *)c,
				   (const mbedtls_mpi *)a,
				   (const mbedtls_mpi *)b) ? -1 : 0;
}

int crypto_bignum_exptmod(const struct crypto_bignum *a,
			  const struct crypto_bignum *b,
			  const struct crypto_bignum *c,
			  struct crypto_bignum *d)
{
	if (TEST_FAIL())
		return -1;

	/* (check if input params match d; d is the result) */
	/* (a == d) is ok in current mbedtls implementation */
	if (b == d || c == d) { /*(not ok; store result in intermediate)*/
		mbedtls_mpi R;
		mbedtls_mpi_init(&R);
		int rc = mbedtls_mpi_exp_mod(&R,
		                             (const mbedtls_mpi *)a,
		                             (const mbedtls_mpi *)b,
		                             (const mbedtls_mpi *)c,
		                             NULL)
		      || mbedtls_mpi_copy((mbedtls_mpi *)d, &R) ? -1 : 0;
		mbedtls_mpi_free(&R);
		return rc;
	}
	else {
		return mbedtls_mpi_exp_mod((mbedtls_mpi *)d,
		                           (const mbedtls_mpi *)a,
		                           (const mbedtls_mpi *)b,
		                           (const mbedtls_mpi *)c,
		                           NULL) ? -1 : 0;
	}
}

int crypto_bignum_inverse(const struct crypto_bignum *a,
			  const struct crypto_bignum *b,
			  struct crypto_bignum *c)
{
	if (TEST_FAIL())
		return -1;

	return mbedtls_mpi_inv_mod((mbedtls_mpi *)c,
				   (const mbedtls_mpi *)a,
				   (const mbedtls_mpi *)b) ? -1 : 0;
}

int crypto_bignum_sub(const struct crypto_bignum *a,
		      const struct crypto_bignum *b,
		      struct crypto_bignum *c)
{
	if (TEST_FAIL())
		return -1;

	return mbedtls_mpi_sub_mpi((mbedtls_mpi *)c,
				   (const mbedtls_mpi *)a,
				   (const mbedtls_mpi *)b) ? -1 : 0;
}

int crypto_bignum_div(const struct crypto_bignum *a,
		      const struct crypto_bignum *b,
		      struct crypto_bignum *c)
{
	if (TEST_FAIL())
		return -1;

	/*(most current use of this crypto.h interface has a == c (result),
	 * so store result in an intermediate to avoid overwritten input)*/
	mbedtls_mpi R;
	mbedtls_mpi_init(&R);
	int rc = mbedtls_mpi_div_mpi(&R, NULL,
				     (const mbedtls_mpi *)a,
				     (const mbedtls_mpi *)b)
	      || mbedtls_mpi_copy((mbedtls_mpi *)c, &R) ? -1 : 0;
	mbedtls_mpi_free(&R);
	return rc;
}

int crypto_bignum_addmod(const struct crypto_bignum *a,
			 const struct crypto_bignum *b,
			 const struct crypto_bignum *c,
			 struct crypto_bignum *d)
{
	if (TEST_FAIL())
		return -1;

	return mbedtls_mpi_add_mpi((mbedtls_mpi *)d,
				   (const mbedtls_mpi *)a,
				   (const mbedtls_mpi *)b)
	    || mbedtls_mpi_mod_mpi((mbedtls_mpi *)d,
				   (mbedtls_mpi *)d,
				   (const mbedtls_mpi *)c) ? -1 : 0;
}

int crypto_bignum_mulmod(const struct crypto_bignum *a,
			 const struct crypto_bignum *b,
			 const struct crypto_bignum *c,
			 struct crypto_bignum *d)
{
	if (TEST_FAIL())
		return -1;

	return mbedtls_mpi_mul_mpi((mbedtls_mpi *)d,
				   (const mbedtls_mpi *)a,
				   (const mbedtls_mpi *)b)
	    || mbedtls_mpi_mod_mpi((mbedtls_mpi *)d,
				   (mbedtls_mpi *)d,
				   (const mbedtls_mpi *)c) ? -1 : 0;
}

int crypto_bignum_sqrmod(const struct crypto_bignum *a,
			 const struct crypto_bignum *b,
			 struct crypto_bignum *c)
{
	if (TEST_FAIL())
		return -1;

  #if 1
	return crypto_bignum_mulmod(a, a, b, c);
  #else
	mbedtls_mpi bn;
	mbedtls_mpi_init(&bn);
	if (mbedtls_mpi_lset(&bn, 2)) /* alt?: mbedtls_mpi_set_bit(&bn, 1) */
		return -1;
	int ret = mbedtls_mpi_exp_mod((mbedtls_mpi *)c,
				      (const mbedtls_mpi *)a, &bn,
				      (const mbedtls_mpi *)b, NULL) ? -1 : 0;
	mbedtls_mpi_free(&bn);
	return ret;
  #endif
}

int crypto_bignum_rshift(const struct crypto_bignum *a, int n,
			 struct crypto_bignum *r)
{
	return mbedtls_mpi_copy((mbedtls_mpi *)r, (const mbedtls_mpi *)a)
	    || mbedtls_mpi_shift_r((mbedtls_mpi *)r, n) ? -1 : 0;
}

int crypto_bignum_cmp(const struct crypto_bignum *a,
		      const struct crypto_bignum *b)
{
	return mbedtls_mpi_cmp_mpi((const mbedtls_mpi *)a, (const mbedtls_mpi *)b);
}

int crypto_bignum_is_zero(const struct crypto_bignum *a)
{
	/* XXX: src/common/sae.c:sswu() contains comment:
	 * "TODO: Make sure crypto_bignum_is_zero() is constant time"
	 * Note: mbedtls_mpi_cmp_int() *is not* constant time */
	return (mbedtls_mpi_cmp_int((const mbedtls_mpi *)a, 0) == 0);
}

int crypto_bignum_is_one(const struct crypto_bignum *a)
{
	return (mbedtls_mpi_cmp_int((const mbedtls_mpi *)a, 1) == 0);
}

int crypto_bignum_is_odd(const struct crypto_bignum *a)
{
	return mbedtls_mpi_get_bit((const mbedtls_mpi *)a, 0);
}

#include "utils/const_time.h"
int crypto_bignum_legendre(const struct crypto_bignum *a,
			   const struct crypto_bignum *p)
{
	if (TEST_FAIL())
		return -2;

	/* Security Note:
	 * mbedtls_mpi_exp_mod() is not documented to run in constant time,
	 * though mbedtls/library/bignum.c uses constant_time_internal.h funcs.
	 * Compare to crypto_openssl.c:crypto_bignum_legendre()
	 * which uses openssl BN_mod_exp_mont_consttime()
	 * mbedtls/library/ecp.c has further countermeasures to timing attacks,
	 * (but ecp.c funcs are not used here) */

	mbedtls_mpi exp, tmp;
	mbedtls_mpi_init(&exp);
	mbedtls_mpi_init(&tmp);

	/* exp = (p-1) / 2 */
	int res;
	if (mbedtls_mpi_sub_int(&exp, (const mbedtls_mpi *)p, 1) == 0
	    && mbedtls_mpi_shift_r(&exp, 1) == 0
	    && mbedtls_mpi_exp_mod(&tmp, (const mbedtls_mpi *)a, &exp,
	                           (const mbedtls_mpi *)p, NULL) == 0) {
		/*(modified from crypto_openssl.c:crypto_bignum_legendre())*/
		/* Return 1 if tmp == 1, 0 if tmp == 0, or -1 otherwise. Need
		 * to use constant time selection to avoid branches here. */
		unsigned int mask;
		res = -1;
		mask = const_time_eq((mbedtls_mpi_cmp_int(&tmp, 1) == 0), 1);
		res = const_time_select_int(mask, 1, res);
		mask = const_time_eq((mbedtls_mpi_cmp_int(&tmp, 0) == 0), 1);
		res = const_time_select_int(mask, 0, res);
	} else {
		res = -2;
	}

	mbedtls_mpi_free(&tmp);
	mbedtls_mpi_free(&exp);
	return res;
}

#endif /* CRYPTO_MBEDTLS_CRYPTO_BIGNUM */


#ifdef CRYPTO_MBEDTLS_CRYPTO_DH

/* crypto_internal-modexp.c */

#include <mbedtls/bignum.h>
#include <mbedtls/dhm.h>

#if 0 /* crypto_dh_init() and crypto_dh_derive_secret() prefer to use mbedtls */
int crypto_mod_exp(const u8 *base, size_t base_len,
		   const u8 *power, size_t power_len,
		   const u8 *modulus, size_t modulus_len,
		   u8 *result, size_t *result_len)
{
	if (TEST_FAIL())
		return -1;

	mbedtls_mpi bn_base, bn_exp, bn_modulus, bn_result;
	mbedtls_mpi_init(&bn_base);
	mbedtls_mpi_init(&bn_exp);
	mbedtls_mpi_init(&bn_modulus);
	mbedtls_mpi_init(&bn_result);

	size_t len;
	int ret =  mbedtls_mpi_read_binary(&bn_base, base, base_len)
	        || mbedtls_mpi_read_binary(&bn_exp, power, power_len)
	        || mbedtls_mpi_read_binary(&bn_modulus, modulus, modulus_len)
	        || mbedtls_mpi_exp_mod(&bn_result,&bn_base,&bn_exp,&bn_modulus,NULL)
	        || (len = mbedtls_mpi_size(&bn_result)) > *result_len
	        || mbedtls_mpi_write_binary(&bn_result, result, (*result_len = len))
	  ? -1
	  : 0;

	mbedtls_mpi_free(&bn_base);
	mbedtls_mpi_free(&bn_exp);
	mbedtls_mpi_free(&bn_modulus);
	mbedtls_mpi_free(&bn_result);
	return ret;
}
#endif

static int crypto_mbedtls_dh_set_bin_pg(mbedtls_dhm_context *ctx, u8 generator,
                                        const u8 *prime, size_t prime_len)
{
	/*(could set these directly in MBEDTLS_PRIVATE members)*/
	mbedtls_mpi P, G;
	mbedtls_mpi_init(&P);
	mbedtls_mpi_init(&G);
	int ret = mbedtls_mpi_lset(&G, generator)
	       || mbedtls_mpi_read_binary(&P, prime, prime_len)
	       || mbedtls_dhm_set_group(ctx, &P, &G);
	mbedtls_mpi_free(&P);
	mbedtls_mpi_free(&G);
	return ret;
}

__attribute_noinline__
static int crypto_mbedtls_dh_init_public(mbedtls_dhm_context *ctx, u8 generator,
                                         const u8 *prime, size_t prime_len,
                                         u8 *privkey, u8 *pubkey)
{
	if (crypto_mbedtls_dh_set_bin_pg(ctx, generator, prime, prime_len)
	    || mbedtls_dhm_make_public(ctx, (int)prime_len, pubkey, prime_len,
	                               mbedtls_ctr_drbg_random,
	                               crypto_mbedtls_ctr_drbg()))
		return -1;

  /*(enable later when upstream mbedtls interface changes require)*/
  #if 0 && MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
	mbedtls_mpi X;
	mbedtls_mpi_init(&X);
	int ret = mbedtls_dhm_get_value(ctx, MBEDTLS_DHM_PARAM_X, &X)
	       || mbedtls_mpi_write_binary(&X, privkey, prime_len) ? -1 : 0;
	mbedtls_mpi_free(&X);
	return ret;
  #else
	return mbedtls_mpi_write_binary(&ctx->MBEDTLS_PRIVATE(X),
	                                privkey, prime_len) ? -1 : 0;
  #endif
}

int crypto_dh_init(u8 generator, const u8 *prime, size_t prime_len, u8 *privkey,
		   u8 *pubkey)
{
	if (TEST_FAIL())
		return -1;

  #if 0 /*(crypto_dh_init() duplicated (and identical) in crypto_*.c modules)*/
	size_t pubkey_len, pad;

	if (os_get_random(privkey, prime_len) < 0)
		return -1;
	if (os_memcmp(privkey, prime, prime_len) > 0) {
		/* Make sure private value is smaller than prime */
		privkey[0] = 0;
	}

	pubkey_len = prime_len;
	if (crypto_mod_exp(&generator, 1, privkey, prime_len, prime, prime_len,
			   pubkey, &pubkey_len) < 0)
		return -1;
	if (pubkey_len < prime_len) {
		pad = prime_len - pubkey_len;
		os_memmove(pubkey + pad, pubkey, pubkey_len);
		os_memset(pubkey, 0, pad);
	}

	return 0;
  #else
	/* Prefer to use mbedtls to derive our public/private key, as doing so
	 * leverages mbedtls to properly format output and to perform blinding*/
	mbedtls_dhm_context ctx;
	mbedtls_dhm_init(&ctx);
	int ret = crypto_mbedtls_dh_init_public(&ctx, generator, prime,
	                                        prime_len, privkey, pubkey);
	mbedtls_dhm_free(&ctx);
	return ret;
  #endif
}

/*(crypto_dh_derive_secret() could be implemented using crypto.h APIs
 * instead of being reimplemented in each crypto_*.c)*/
int crypto_dh_derive_secret(u8 generator, const u8 *prime, size_t prime_len,
			    const u8 *order, size_t order_len,
			    const u8 *privkey, size_t privkey_len,
			    const u8 *pubkey, size_t pubkey_len,
			    u8 *secret, size_t *len)
{
	if (TEST_FAIL())
		return -1;

  #if 0
	if (pubkey_len > prime_len ||
	    (pubkey_len == prime_len &&
	     os_memcmp(pubkey, prime, prime_len) >= 0))
		return -1;

	int res = 0;
	mbedtls_mpi pub;
	mbedtls_mpi_init(&pub);
	if (mbedtls_mpi_read_binary(&pub, pubkey, pubkey_len)
	    || mbedtls_mpi_cmp_int(&pub, 1) <= 0) {
		res = -1;
	} else if (order) {
		mbedtls_mpi p, q, tmp;
		mbedtls_mpi_init(&p);
		mbedtls_mpi_init(&q);
		mbedtls_mpi_init(&tmp);

		/* verify: pubkey^q == 1 mod p */
		res = (mbedtls_mpi_read_binary(&p, prime, prime_len)
		    || mbedtls_mpi_read_binary(&q, order, order_len)
		    || mbedtls_mpi_exp_mod(&tmp, &pub, &q, &p, NULL)
		    || mbedtls_mpi_cmp_int(&tmp, 1) != 0);

		mbedtls_mpi_free(&p);
		mbedtls_mpi_free(&q);
		mbedtls_mpi_free(&tmp);
	}
	mbedtls_mpi_free(&pub);

	return (res == 0)
	  ? crypto_mod_exp(pubkey, pubkey_len, privkey, privkey_len,
			   prime, prime_len, secret, len)
	  : -1;
  #else
	/* Prefer to use mbedtls to derive DH shared secret, as doing so
	 * leverages mbedtls to validate params and to perform blinding.
	 *
	 * Attempt to reconstitute DH context to derive shared secret
	 * (due to limitations of the interface, which ought to pass context).
	 * Force provided G (our private key) into context without validation.
	 * Regenerating GX (our public key) not needed to derive shared secret.
	 */
	/*(older compilers might not support VLAs)*/
	/*unsigned char buf[2+prime_len+2+1+2+pubkey_len];*/
	unsigned char buf[2+MBEDTLS_MPI_MAX_SIZE+2+1+2+MBEDTLS_MPI_MAX_SIZE];
	unsigned char *p = buf + 2 + prime_len;
	if (2+prime_len+2+1+2+pubkey_len > sizeof(buf))
		return -1;
	WPA_PUT_BE16(buf, prime_len);  /*(2-byte big-endian size of prime)*/
	p[0] = 0;                      /*(2-byte big-endian size of generator)*/
	p[1] = 1;
	p[2] = generator;
	WPA_PUT_BE16(p+3, pubkey_len); /*(2-byte big-endian size of pubkey)*/
	os_memcpy(p+5, pubkey, pubkey_len);
	os_memcpy(buf+2, prime, prime_len);

	mbedtls_dhm_context ctx;
	mbedtls_dhm_init(&ctx);
	p = buf;
	int ret = mbedtls_dhm_read_params(&ctx, &p, p+2+prime_len+5+pubkey_len)
	       || mbedtls_mpi_read_binary(&ctx.MBEDTLS_PRIVATE(X),
	                                  privkey, privkey_len)
	       || mbedtls_dhm_calc_secret(&ctx, secret, *len, len,
	                                  mbedtls_ctr_drbg_random,
	                                  crypto_mbedtls_ctr_drbg()) ? -1 : 0;
	mbedtls_dhm_free(&ctx);
	return ret;
  #endif
}

/* dh_group5.c */

#include "dh_group5.h"

/* RFC3526_PRIME_1536[] and RFC3526_GENERATOR_1536[] from crypto_wolfssl.c */

static const unsigned char RFC3526_PRIME_1536[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2,
	0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 0x02, 0x0B, 0xBE, 0xA6,
	0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D,
	0xF2, 0x5F, 0x14, 0x37, 0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6, 0xF4, 0x4C, 0x42, 0xE9,
	0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11,
	0x7C, 0x4B, 0x1F, 0xE6, 0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
	0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05, 0x98, 0xDA, 0x48, 0x36,
	0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
	0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56,
	0x20, 0x85, 0x52, 0xBB, 0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
	0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04, 0xF1, 0x74, 0x6C, 0x08,
	0xCA, 0x23, 0x73, 0x27, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const unsigned char RFC3526_GENERATOR_1536[] = {
	0x02
};

void * dh5_init(struct wpabuf **priv, struct wpabuf **publ)
{
	const unsigned char * const prime = RFC3526_PRIME_1536;
	const size_t prime_len = sizeof(RFC3526_PRIME_1536);
	const u8 generator = *RFC3526_GENERATOR_1536;
	struct wpabuf *wpubl = NULL, *wpriv = NULL;

	mbedtls_dhm_context *ctx = os_malloc(sizeof(*ctx));
	if (ctx == NULL)
		return NULL;
	mbedtls_dhm_init(ctx);

	if (   (wpubl = wpabuf_alloc(prime_len))
	    && (wpriv = wpabuf_alloc(prime_len))
	    && crypto_mbedtls_dh_init_public(ctx, generator, prime, prime_len,
	                                     wpabuf_put(wpriv, prime_len),
	                                     wpabuf_put(wpubl, prime_len))==0) {
		wpabuf_free(*publ);
		wpabuf_clear_free(*priv);
		*publ = wpubl;
		*priv = wpriv;
		return ctx;
	}

	wpabuf_clear_free(wpriv);
	wpabuf_free(wpubl);
	mbedtls_dhm_free(ctx);
	os_free(ctx);
	return NULL;
}

#ifdef CRYPTO_MBEDTLS_DH5_INIT_FIXED
void * dh5_init_fixed(const struct wpabuf *priv, const struct wpabuf *publ)
{
	const unsigned char * const prime = RFC3526_PRIME_1536;
	const size_t prime_len = sizeof(RFC3526_PRIME_1536);
	const u8 generator = *RFC3526_GENERATOR_1536;

	mbedtls_dhm_context *ctx = os_malloc(sizeof(*ctx));
	if (ctx == NULL)
		return NULL;
	mbedtls_dhm_init(ctx);

	if (crypto_mbedtls_dh_set_bin_pg(ctx, generator, prime, prime_len)==0
	   #if 0 /*(ignore; not required to derive shared secret)*/
	    && mbedtls_mpi_read_binary(&ctx->MBEDTLS_PRIVATE(GX),
				       wpabuf_head(publ),wpabuf_len(publ))==0
	   #endif
	    && mbedtls_mpi_read_binary(&ctx->MBEDTLS_PRIVATE(X),
				       wpabuf_head(priv),wpabuf_len(priv))==0) {
		return ctx;
	}

	mbedtls_dhm_free(ctx);
	os_free(ctx);
	return NULL;
}
#endif

struct wpabuf * dh5_derive_shared(void *ctx, const struct wpabuf *peer_public,
				  const struct wpabuf *own_private)
{
	/*((mbedtls_dhm_context *)ctx must already contain own_private)*/
	/* mbedtls 2.x: prime_len = ctx->len; */
	/* mbedtls 3.x: prime_len = mbedtls_dhm_get_len(ctx); */
	size_t olen = sizeof(RFC3526_PRIME_1536); /*(sizeof(); prime known)*/
	struct wpabuf *buf = wpabuf_alloc(olen);
	if (buf == NULL)
		return NULL;
	if (mbedtls_dhm_read_public((mbedtls_dhm_context *)ctx,
	                            wpabuf_head(peer_public),
	                            wpabuf_len(peer_public)) == 0
	    && mbedtls_dhm_calc_secret(ctx, wpabuf_mhead(buf), olen, &olen,
	                               mbedtls_ctr_drbg_random,
	                               crypto_mbedtls_ctr_drbg()) == 0) {
		wpabuf_put(buf, olen);
		return buf;
	}

	wpabuf_free(buf);
	return NULL;
}

void dh5_free(void *ctx)
{
	mbedtls_dhm_free(ctx);
	os_free(ctx);
}

#endif /* CRYPTO_MBEDTLS_CRYPTO_DH */


#if defined(CRYPTO_MBEDTLS_CRYPTO_ECDH) || defined(CRYPTO_MBEDTLS_CRYPTO_EC)

#include <mbedtls/ecp.h>

#define CRYPTO_EC_pbits(e) (((mbedtls_ecp_group *)(e))->pbits)
#define CRYPTO_EC_plen(e) ((((mbedtls_ecp_group *)(e))->pbits+7)>>3)
#define CRYPTO_EC_P(e)    (&((mbedtls_ecp_group *)(e))->P)
#define CRYPTO_EC_N(e)    (&((mbedtls_ecp_group *)(e))->N)
#define CRYPTO_EC_A(e)    (&((mbedtls_ecp_group *)(e))->A)
#define CRYPTO_EC_B(e)    (&((mbedtls_ecp_group *)(e))->B)
#define CRYPTO_EC_G(e)    (&((mbedtls_ecp_group *)(e))->G)

static mbedtls_ecp_group_id crypto_mbedtls_ecp_group_id_from_ike_id(int group)
{
	/* https://www.iana.org/assignments/ikev2-parameters/ikev2-parameters.xhtml */
	switch (group) {
  #ifdef MBEDTLS_ECP_DP_SECP256R1_ENABLED
	case 19: return MBEDTLS_ECP_DP_SECP256R1;
  #endif
  #ifdef MBEDTLS_ECP_DP_SECP384R1_ENABLED
	case 20: return MBEDTLS_ECP_DP_SECP384R1;
  #endif
  #ifdef MBEDTLS_ECP_DP_SECP521R1_ENABLED
	case 21: return MBEDTLS_ECP_DP_SECP521R1;
  #endif
  #ifdef MBEDTLS_ECP_DP_SECP192R1_ENABLED
	case 25: return MBEDTLS_ECP_DP_SECP192R1;
  #endif
  #ifdef MBEDTLS_ECP_DP_SECP224R1_ENABLED
	case 26: return MBEDTLS_ECP_DP_SECP224R1;
  #endif
  #ifdef MBEDTLS_ECP_DP_BP256R1_ENABLED
	case 28: return MBEDTLS_ECP_DP_BP256R1;
  #endif
  #ifdef MBEDTLS_ECP_DP_BP384R1_ENABLED
	case 29: return MBEDTLS_ECP_DP_BP384R1;
  #endif
  #ifdef MBEDTLS_ECP_DP_BP512R1_ENABLED
	case 30: return MBEDTLS_ECP_DP_BP512R1;
  #endif
  #ifdef MBEDTLS_ECP_DP_CURVE25519_ENABLED
	case 31: return MBEDTLS_ECP_DP_CURVE25519;
  #endif
  #ifdef MBEDTLS_ECP_DP_CURVE448_ENABLED
	case 32: return MBEDTLS_ECP_DP_CURVE448;
  #endif
	default: return MBEDTLS_ECP_DP_NONE;
	}
}

#ifdef CRYPTO_MBEDTLS_CRYPTO_EC
static int crypto_mbedtls_ike_id_from_ecp_group_id(mbedtls_ecp_group_id grp_id)
{
	/* https://www.iana.org/assignments/ikev2-parameters/ikev2-parameters.xhtml */
	/*(for crypto_ec_key_group())*/
	switch (grp_id) {
  #ifdef MBEDTLS_ECP_DP_SECP256R1_ENABLED
	case MBEDTLS_ECP_DP_SECP256R1:  return 19;
  #endif
  #ifdef MBEDTLS_ECP_DP_SECP384R1_ENABLED
	case MBEDTLS_ECP_DP_SECP384R1:  return 20;
  #endif
  #ifdef MBEDTLS_ECP_DP_SECP521R1_ENABLED
	case MBEDTLS_ECP_DP_SECP521R1:  return 21;
  #endif
  #ifdef MBEDTLS_ECP_DP_SECP192R1_ENABLED
	case MBEDTLS_ECP_DP_SECP192R1:  return 25;
  #endif
  #ifdef MBEDTLS_ECP_DP_SECP224R1_ENABLED
	case MBEDTLS_ECP_DP_SECP224R1:  return 26;
  #endif
  #ifdef MBEDTLS_ECP_DP_BP256R1_ENABLED
	case MBEDTLS_ECP_DP_BP256R1:    return 28;
  #endif
  #ifdef MBEDTLS_ECP_DP_BP384R1_ENABLED
	case MBEDTLS_ECP_DP_BP384R1:    return 29;
  #endif
  #ifdef MBEDTLS_ECP_DP_BP512R1_ENABLED
	case MBEDTLS_ECP_DP_BP512R1:    return 30;
  #endif
  #ifdef MBEDTLS_ECP_DP_CURVE25519_ENABLED
	case MBEDTLS_ECP_DP_CURVE25519: return 31;
  #endif
  #ifdef MBEDTLS_ECP_DP_CURVE448_ENABLED
	case MBEDTLS_ECP_DP_CURVE448:   return 32;
  #endif
	default: return -1;
	}
}
#endif

#endif /* CRYPTO_MBEDTLS_CRYPTO_ECDH || CRYPTO_MBEDTLS_CRYPTO_EC */


#if defined(CRYPTO_MBEDTLS_CRYPTO_ECDH) || defined(CRYPTO_MBEDTLS_CRYPTO_EC_DPP)

#include <mbedtls/ecp.h>
#include <mbedtls/pk.h>

static int crypto_mbedtls_keypair_gen(int group, mbedtls_pk_context *pk)
{
	mbedtls_ecp_group_id grp_id =
	  crypto_mbedtls_ecp_group_id_from_ike_id(group);
	if (grp_id == MBEDTLS_ECP_DP_NONE)
		return -1;
	const mbedtls_pk_info_t *pk_info =
	  mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY);
	if (pk_info == NULL)
		return -1;
	return mbedtls_pk_setup(pk, pk_info)
	    || mbedtls_ecp_gen_key(grp_id, mbedtls_pk_ec(*pk),
	                           mbedtls_ctr_drbg_random,
	                           crypto_mbedtls_ctr_drbg()) ? -1 : 0;
}

#endif


#ifdef CRYPTO_MBEDTLS_CRYPTO_ECDH

#include <mbedtls/ecdh.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
#include <mbedtls/pk.h>

/* wrap mbedtls_ecdh_context for more future-proof direct access to components
 * (mbedtls_ecdh_context internal implementation may change between releases)
 *
 * If mbedtls_pk_context -- specifically underlying mbedtls_ecp_keypair --
 * lifetime were guaranteed to be longer than that of mbedtls_ecdh_context,
 * then mbedtls_pk_context or mbedtls_ecp_keypair could be stored in crypto_ecdh
 * (or crypto_ec_key could be stored in crypto_ecdh, and crypto_ec_key could
 *  wrap mbedtls_ecp_keypair and components, to avoid MBEDTLS_PRIVATE access) */
struct crypto_ecdh {
	mbedtls_ecdh_context ctx;
	mbedtls_ecp_group grp;
	mbedtls_ecp_point Q;
};

struct crypto_ecdh * crypto_ecdh_init(int group)
{
	mbedtls_pk_context pk;
	mbedtls_pk_init(&pk);
	struct crypto_ecdh *ecdh = crypto_mbedtls_keypair_gen(group, &pk) == 0
	  ? crypto_ecdh_init2(group, (struct crypto_ec_key *)&pk)
	  : NULL;
	mbedtls_pk_free(&pk);
	return ecdh;
}

struct crypto_ecdh * crypto_ecdh_init2(int group,
				       struct crypto_ec_key *own_key)
{
	mbedtls_ecp_group_id grp_id =
	  crypto_mbedtls_ecp_group_id_from_ike_id(group);
	if (grp_id == MBEDTLS_ECP_DP_NONE)
		return NULL;
	mbedtls_ecp_keypair *ecp_kp = mbedtls_pk_ec(*(mbedtls_pk_context *)own_key);
	struct crypto_ecdh *ecdh = os_malloc(sizeof(*ecdh));
	if (ecdh == NULL)
		return NULL;
	mbedtls_ecdh_init(&ecdh->ctx);
	mbedtls_ecp_group_init(&ecdh->grp);
	mbedtls_ecp_point_init(&ecdh->Q);
	if (mbedtls_ecdh_setup(&ecdh->ctx, grp_id) == 0
	    && mbedtls_ecdh_get_params(&ecdh->ctx,ecp_kp,MBEDTLS_ECDH_OURS) == 0) {
		/* copy grp and Q for later use
		 * (retrieving this info later is more convoluted
		 *  even if mbedtls_ecdh_make_public() is considered)*/
	  #if MBEDTLS_VERSION_NUMBER >= 0x03020000 /* mbedtls 3.2.0 */
		mbedtls_mpi d;
		mbedtls_mpi_init(&d);
		if (mbedtls_ecp_export(ecp_kp, &ecdh->grp, &d, &ecdh->Q) == 0) {
			mbedtls_mpi_free(&d);
			return ecdh;
		}
		mbedtls_mpi_free(&d);
	  #else
		if (mbedtls_ecp_group_load(&ecdh->grp, grp_id) == 0
		    && mbedtls_ecp_copy(&ecdh->Q, &ecp_kp->MBEDTLS_PRIVATE(Q)) == 0)
			return ecdh;
	  #endif
	}

	mbedtls_ecp_point_free(&ecdh->Q);
	mbedtls_ecp_group_free(&ecdh->grp);
	mbedtls_ecdh_free(&ecdh->ctx);
	os_free(ecdh);
	return NULL;
}

struct wpabuf * crypto_ecdh_get_pubkey(struct crypto_ecdh *ecdh, int inc_y)
{
	mbedtls_ecp_group *grp = &ecdh->grp;
	size_t prime_len = CRYPTO_EC_plen(grp);
	size_t output_len = prime_len;
	u8 output_offset = 0;
	u8 buf[256];

  #ifdef MBEDTLS_ECP_MONTGOMERY_ENABLED
	/* len */
  #endif
  #ifdef MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED
	if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS) {
		output_len = inc_y ? prime_len * 2 + 1 : prime_len + 1;
		output_offset = 1;
	}
  #endif

	if (output_len > sizeof(buf))
		return NULL;

	inc_y = inc_y ? MBEDTLS_ECP_PF_UNCOMPRESSED : MBEDTLS_ECP_PF_COMPRESSED;
	if (mbedtls_ecp_point_write_binary(grp, &ecdh->Q, inc_y, &output_len,
	                                   buf, output_len) == 0) {
		return wpabuf_alloc_copy(buf + output_offset, output_len - output_offset);
	}

	return NULL;
}

#if defined(MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED)
static int crypto_mbedtls_short_weierstrass_derive_y(mbedtls_ecp_group *grp,
                                                     mbedtls_mpi *bn,
                                                     int parity_bit)
{
	/* y^2 = x^3 + ax + b
	 * sqrt(w) = w^((p+1)/4) mod p   (for prime p where p = 3 mod 4) */
	mbedtls_mpi *cy2 = (mbedtls_mpi *)
	  crypto_ec_point_compute_y_sqr((struct crypto_ec *)grp,
	                                (const struct crypto_bignum *)bn); /*x*/
	if (cy2 == NULL)
		return -1;

	/*mbedtls_mpi_free(bn);*/
	/*(reuse bn to store result (y))*/

	mbedtls_mpi exp;
	mbedtls_mpi_init(&exp);
	int ret = mbedtls_mpi_get_bit(&grp->P, 0) != 1 /*(p = 3 mod 4)*/
	       || mbedtls_mpi_get_bit(&grp->P, 1) != 1 /*(p = 3 mod 4)*/
	       || mbedtls_mpi_add_int(&exp, &grp->P, 1)
	       || mbedtls_mpi_shift_r(&exp, 2)
	       || mbedtls_mpi_exp_mod(bn, cy2, &exp, &grp->P, NULL)
	       || (mbedtls_mpi_get_bit(bn, 0) != parity_bit
	           && mbedtls_mpi_sub_mpi(bn, &grp->P, bn));
	mbedtls_mpi_free(&exp);
	mbedtls_mpi_free(cy2);
	os_free(cy2);
	return ret;
}
#endif

struct wpabuf * crypto_ecdh_set_peerkey(struct crypto_ecdh *ecdh, int inc_y,
					const u8 *key, size_t len)
{
	if (len == 0) /*(invalid peer key)*/
		return NULL;

	mbedtls_ecp_group *grp = &ecdh->grp;

  #if defined(MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED)
	if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS) {
		/* add header for mbedtls_ecdh_read_public() */
		u8 buf[256];
		if (sizeof(buf)-1 < len)
			return NULL;
		buf[0] = (u8)(len);
		os_memcpy(buf+1, key, len);

		if (inc_y) {
			if (!(len & 1)) { /*(dpp code/tests does not include tag?!?)*/
				if (sizeof(buf)-2 < len)
					return NULL;
				buf[0] = (u8)(1+len);
				buf[1] = 0x04;
				os_memcpy(buf+2, key, len);
			}
			len >>= 1; /*(repurpose len to prime_len)*/
		} else { /* (inc_y == 0) */
			/* mbedtls_ecp_point_read_binary() does not currently support
			 * MBEDTLS_ECP_PF_COMPRESSED format (buf[1] = 0x02 or 0x03)
			 * (returns MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE) */

			/* derive y, amend buf[] with y for UNCOMPRESSED format */
			if (sizeof(buf)-2 < len*2 || len == 0)
				return NULL;

			buf[0] = (u8)(1+len*2);
			buf[1] = 0x04;
			os_memcpy(buf+2, key, len);

			mbedtls_mpi bn;
			mbedtls_mpi_init(&bn);
			int ret = mbedtls_mpi_read_binary(&bn, key, len)
			       || crypto_mbedtls_short_weierstrass_derive_y(grp, &bn, 0)
			       || mbedtls_mpi_write_binary(&bn, buf+2+len, len);
			mbedtls_mpi_free(&bn);
			if (ret != 0)
				return NULL;
		}

		if (mbedtls_ecdh_read_public(&ecdh->ctx, buf, buf[0]+1))
			return NULL;
	}
  #endif
  #if defined(MBEDTLS_ECP_MONTGOMERY_ENABLED)
	if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_MONTGOMERY) {
		if (mbedtls_ecdh_read_public(&ecdh->ctx, key, len))
			return NULL;
	}
  #endif

	struct wpabuf *buf = wpabuf_alloc(len);
	if (buf == NULL)
		return NULL;

	if (mbedtls_ecdh_calc_secret(&ecdh->ctx, &len,
	                             wpabuf_mhead(buf), len,
	                             mbedtls_ctr_drbg_random,
	                             crypto_mbedtls_ctr_drbg()) == 0) {
		wpabuf_put(buf, len);
		return buf;
	}

	wpabuf_clear_free(buf);
	return NULL;
}

void crypto_ecdh_deinit(struct crypto_ecdh *ecdh)
{
	if (ecdh == NULL)
		return;
	mbedtls_ecp_point_free(&ecdh->Q);
	mbedtls_ecp_group_free(&ecdh->grp);
	mbedtls_ecdh_free(&ecdh->ctx);
	os_free(ecdh);
}

size_t crypto_ecdh_prime_len(struct crypto_ecdh *ecdh)
{
	return CRYPTO_EC_plen(&ecdh->grp);
}

#endif /* CRYPTO_MBEDTLS_CRYPTO_ECDH */


#ifdef CRYPTO_MBEDTLS_CRYPTO_EC

#include <mbedtls/ecp.h>

struct crypto_ec *crypto_ec_init(int group)
{
	mbedtls_ecp_group_id grp_id =
	  crypto_mbedtls_ecp_group_id_from_ike_id(group);
	if (grp_id == MBEDTLS_ECP_DP_NONE)
		return NULL;
	mbedtls_ecp_group *e = os_malloc(sizeof(*e));
	if (e == NULL)
		return NULL;
	mbedtls_ecp_group_init(e);
	if (mbedtls_ecp_group_load(e, grp_id) == 0)
		return (struct crypto_ec *)e;

	mbedtls_ecp_group_free(e);
	os_free(e);
	return NULL;
}

void crypto_ec_deinit(struct crypto_ec *e)
{
	mbedtls_ecp_group_free((mbedtls_ecp_group *)e);
	os_free(e);
}

size_t crypto_ec_prime_len(struct crypto_ec *e)
{
	return CRYPTO_EC_plen(e);
}

size_t crypto_ec_prime_len_bits(struct crypto_ec *e)
{
	return CRYPTO_EC_pbits(e);
}

size_t crypto_ec_order_len(struct crypto_ec *e)
{
	return (mbedtls_mpi_bitlen(CRYPTO_EC_N(e)) + 7) / 8;
}

const struct crypto_bignum *crypto_ec_get_prime(struct crypto_ec *e)
{
	return (const struct crypto_bignum *)CRYPTO_EC_P(e);
}

const struct crypto_bignum *crypto_ec_get_order(struct crypto_ec *e)
{
	return (const struct crypto_bignum *)CRYPTO_EC_N(e);
}

const struct crypto_bignum *crypto_ec_get_a(struct crypto_ec *e)
{
	static const uint8_t secp256r1_a[] =
	  {0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x01,
	   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	   0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfc};
	static const uint8_t secp384r1_a[] =
	  {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,
	   0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,
	   0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xfc};
	static const uint8_t secp521r1_a[] =
	  {0x01,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xfc};
	static const uint8_t secp192r1_a[] =
	  {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfc};
	static const uint8_t secp224r1_a[] =
	  {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,
	   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	   0xff,0xff,0xff,0xfe};

	const uint8_t *bin = NULL;
	size_t len = 0;

	/* (mbedtls groups matching supported sswu_curve_param() IKE groups) */
	switch (((mbedtls_ecp_group *)e)->id) {
  #ifdef MBEDTLS_ECP_DP_SECP256R1_ENABLED
	case MBEDTLS_ECP_DP_SECP256R1:
		bin = secp256r1_a;
		len = sizeof(secp256r1_a);
		break;
  #endif
  #ifdef MBEDTLS_ECP_DP_SECP384R1_ENABLED
	case MBEDTLS_ECP_DP_SECP384R1:
		bin = secp384r1_a;
		len = sizeof(secp384r1_a);
		break;
  #endif
  #ifdef MBEDTLS_ECP_DP_SECP521R1_ENABLED
	case MBEDTLS_ECP_DP_SECP521R1:
		bin = secp521r1_a;
		len = sizeof(secp521r1_a);
		break;
  #endif
  #ifdef MBEDTLS_ECP_DP_SECP192R1_ENABLED
	case MBEDTLS_ECP_DP_SECP192R1:
		bin = secp192r1_a;
		len = sizeof(secp192r1_a);
		break;
  #endif
  #ifdef MBEDTLS_ECP_DP_SECP224R1_ENABLED
	case MBEDTLS_ECP_DP_SECP224R1:
		bin = secp224r1_a;
		len = sizeof(secp224r1_a);
		break;
  #endif
  #ifdef MBEDTLS_ECP_DP_BP256R1_ENABLED
	case MBEDTLS_ECP_DP_BP256R1:
		return (const struct crypto_bignum *)CRYPTO_EC_A(e);
  #endif
  #ifdef MBEDTLS_ECP_DP_BP384R1_ENABLED
	case MBEDTLS_ECP_DP_BP384R1:
		return (const struct crypto_bignum *)CRYPTO_EC_A(e);
  #endif
  #ifdef MBEDTLS_ECP_DP_BP512R1_ENABLED
	case MBEDTLS_ECP_DP_BP512R1:
		return (const struct crypto_bignum *)CRYPTO_EC_A(e);
  #endif
  #ifdef MBEDTLS_ECP_DP_CURVE25519_ENABLED
	case MBEDTLS_ECP_DP_CURVE25519:
		return (const struct crypto_bignum *)CRYPTO_EC_A(e);
  #endif
  #ifdef MBEDTLS_ECP_DP_CURVE448_ENABLED
	case MBEDTLS_ECP_DP_CURVE448:
		return (const struct crypto_bignum *)CRYPTO_EC_A(e);
  #endif
	default:
		return NULL;
	}

	/*(note: not thread-safe; returns file-scoped static storage)*/
	if (mbedtls_mpi_read_binary(&mpi_sw_A, bin, len) == 0)
		return (const struct crypto_bignum *)&mpi_sw_A;
	return NULL;
}

const struct crypto_bignum *crypto_ec_get_b(struct crypto_ec *e)
{
	return (const struct crypto_bignum *)CRYPTO_EC_B(e);
}

const struct crypto_ec_point * crypto_ec_get_generator(struct crypto_ec *e)
{
	return (const struct crypto_ec_point *)CRYPTO_EC_G(e);
}

struct crypto_ec_point *crypto_ec_point_init(struct crypto_ec *e)
{
	if (TEST_FAIL())
		return NULL;

	mbedtls_ecp_point *p = os_malloc(sizeof(*p));
	if (p != NULL)
		mbedtls_ecp_point_init(p);
	return (struct crypto_ec_point *)p;
}

void crypto_ec_point_deinit(struct crypto_ec_point *p, int clear)
{
	mbedtls_ecp_point_free((mbedtls_ecp_point *)p);
	os_free(p);
}

int crypto_ec_point_x(struct crypto_ec *e, const struct crypto_ec_point *p,
		      struct crypto_bignum *x)
{
	mbedtls_mpi *px = &((mbedtls_ecp_point *)p)->MBEDTLS_PRIVATE(X);
	return mbedtls_mpi_copy((mbedtls_mpi *)x, px)
	  ? -1
	  : 0;
}

int crypto_ec_point_to_bin(struct crypto_ec *e,
			   const struct crypto_ec_point *point, u8 *x, u8 *y)
{
	if (TEST_FAIL())
		return -1;

	/* crypto.h documents crypto_ec_point_to_bin() output is big-endian */
	size_t len = CRYPTO_EC_plen(e);
	if (x) {
		mbedtls_mpi *px = &((mbedtls_ecp_point *)point)->MBEDTLS_PRIVATE(X);
		if (mbedtls_mpi_write_binary(px, x, len))
			return -1;
	}
	if (y) {
	  #if 0 /*(should not be necessary; py mpi should be in initial state)*/
	  #ifdef MBEDTLS_ECP_MONTGOMERY_ENABLED
		if (mbedtls_ecp_get_type((mbedtls_ecp_group *)e)
		    == MBEDTLS_ECP_TYPE_MONTGOMERY) {
			os_memset(y, 0, len);
			return 0;
		}
	  #endif
	  #endif
		mbedtls_mpi *py = &((mbedtls_ecp_point *)point)->MBEDTLS_PRIVATE(Y);
		if (mbedtls_mpi_write_binary(py, y, len))
			return -1;
	}
	return 0;
}

struct crypto_ec_point * crypto_ec_point_from_bin(struct crypto_ec *e,
						  const u8 *val)
{
	if (TEST_FAIL())
		return NULL;

	size_t len = CRYPTO_EC_plen(e);
	mbedtls_ecp_point *p = os_malloc(sizeof(*p));
	u8 buf[1+MBEDTLS_MPI_MAX_SIZE*2];
	if (p == NULL)
		return NULL;
	mbedtls_ecp_point_init(p);

  #ifdef MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED
	if (mbedtls_ecp_get_type((mbedtls_ecp_group *)e)
	    == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS) {
	  #if 0 /* prefer alternative to MBEDTLS_PRIVATE() access */
		mbedtls_mpi *px = &((mbedtls_ecp_point *)p)->MBEDTLS_PRIVATE(X);
		mbedtls_mpi *py = &((mbedtls_ecp_point *)p)->MBEDTLS_PRIVATE(Y);
		mbedtls_mpi *pz = &((mbedtls_ecp_point *)p)->MBEDTLS_PRIVATE(Z);

		if (mbedtls_mpi_read_binary(px, val, len) == 0
		    && mbedtls_mpi_read_binary(py, val + len, len) == 0
		    && mbedtls_mpi_lset(pz, 1) == 0)
			return (struct crypto_ec_point *)p;
	  #else
		buf[0] = 0x04;
		os_memcpy(buf+1, val, len*2);
		if (mbedtls_ecp_point_read_binary((mbedtls_ecp_group *)e, p,
		                                  buf, 1+len*2) == 0)
			return (struct crypto_ec_point *)p;
	  #endif
	}
  #endif
  #ifdef MBEDTLS_ECP_MONTGOMERY_ENABLED
	if (mbedtls_ecp_get_type((mbedtls_ecp_group *)e)
	    == MBEDTLS_ECP_TYPE_MONTGOMERY) {
		/* crypto.h interface documents crypto_ec_point_from_bin()
		 * val is length: prime_len * 2 and is big-endian
		 * (Short Weierstrass is assumed by hostap)
		 * Reverse to little-endian format for Montgomery */
		for (unsigned int i = 0; i < len; ++i)
			buf[i] = val[len-1-i];
		if (mbedtls_ecp_point_read_binary((mbedtls_ecp_group *)e, p,
		                                  buf, len) == 0)
			return (struct crypto_ec_point *)p;
	}
  #endif

	mbedtls_ecp_point_free(p);
	os_free(p);
	return NULL;
}

int crypto_ec_point_add(struct crypto_ec *e, const struct crypto_ec_point *a,
			const struct crypto_ec_point *b,
			struct crypto_ec_point *c)
{
	if (TEST_FAIL())
		return -1;

	/* mbedtls does not provide an mbedtls_ecp_point add function */
	mbedtls_mpi one;
	mbedtls_mpi_init(&one);
	int ret = mbedtls_mpi_lset(&one, 1)
	       || mbedtls_ecp_muladd(
			(mbedtls_ecp_group *)e, (mbedtls_ecp_point *)c,
			&one, (const mbedtls_ecp_point *)a,
			&one, (const mbedtls_ecp_point *)b) ? -1 : 0;
	mbedtls_mpi_free(&one);
	return ret;
}

int crypto_ec_point_mul(struct crypto_ec *e, const struct crypto_ec_point *p,
			const struct crypto_bignum *b,
			struct crypto_ec_point *res)
{
	if (TEST_FAIL())
		return -1;

	return mbedtls_ecp_mul(
		(mbedtls_ecp_group *)e, (mbedtls_ecp_point *)res,
		(const mbedtls_mpi *)b, (const mbedtls_ecp_point *)p,
		mbedtls_ctr_drbg_random, crypto_mbedtls_ctr_drbg()) ? -1 : 0;
}

int crypto_ec_point_invert(struct crypto_ec *e, struct crypto_ec_point *p)
{
	if (TEST_FAIL())
		return -1;

	if (mbedtls_ecp_get_type((mbedtls_ecp_group *)e)
	    == MBEDTLS_ECP_TYPE_MONTGOMERY) {
		/* e.g. MBEDTLS_ECP_DP_CURVE25519 and MBEDTLS_ECP_DP_CURVE448 */
		wpa_printf(MSG_ERROR,
		           "%s not implemented for Montgomery curves",__func__);
		return -1;
	}

	/* mbedtls does not provide an mbedtls_ecp_point invert function */
	/* below works for Short Weierstrass; incorrect for Montgomery curves */
	mbedtls_mpi *py = &((mbedtls_ecp_point *)p)->MBEDTLS_PRIVATE(Y);
	return mbedtls_ecp_is_zero((mbedtls_ecp_point *)p) /*point at infinity*/
	    || mbedtls_mpi_cmp_int(py, 0) == 0      /*point is its own inverse*/
	    || mbedtls_mpi_sub_abs(py, CRYPTO_EC_P(e), py) == 0 ? 0 : -1;
}

#ifdef MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED
static int
crypto_ec_point_y_sqr_weierstrass(mbedtls_ecp_group *e, const mbedtls_mpi *x,
                                  mbedtls_mpi *y2)
{
	/* MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS  y^2 = x^3 + a x + b    */

	/* Short Weierstrass elliptic curve group w/o A set treated as A = -3 */
	/* Attempt to match mbedtls/library/ecp.c:ecp_check_pubkey_sw() behavior
	 * and elsewhere in mbedtls/library/ecp.c where if A is not set, it is
	 * treated as if A = -3. */

  #if 0
	/* y^2 = x^3 + ax + b */
	mbedtls_mpi *A = &e->A;
	mbedtls_mpi t, A_neg3;
	if (&e->A.p == NULL) {
		mbedtls_mpi_init(&A_neg3);
		if (mbedtls_mpi_lset(&A_neg3, -3) != 0) {
			mbedtls_mpi_free(&A_neg3);
			return -1;
		}
		A = &A_neg3;
	}
	mbedtls_mpi_init(&t);
	int ret = /* x^3 */
	          mbedtls_mpi_lset(&t, 3)
	       || mbedtls_mpi_exp_mod(y2,  x, &t, &e->P, NULL)
		  /* ax */
	       || mbedtls_mpi_mul_mpi(y2, y2, A)
	       || mbedtls_mpi_mod_mpi(&t, &t, &e->P)
		  /* ax + b */
	       || mbedtls_mpi_add_mpi(&t, &t, &e->B)
	       || mbedtls_mpi_mod_mpi(&t, &t, &e->P)
		  /* x^3 + ax + b */
	       || mbedtls_mpi_add_mpi(&t, &t, y2) /* ax + b + x^3 */
	       || mbedtls_mpi_mod_mpi(y2, &t, &e->P);
	mbedtls_mpi_free(&t);
	if (A == &A_neg3)
		mbedtls_mpi_free(&A_neg3);
	return ret; /* 0: success, non-zero: failure */
  #else
	/* y^2 = x^3 + ax + b = (x^2 + a)x + b */
	return    /* x^2 */
	          mbedtls_mpi_mul_mpi(y2,  x, x)
	       || mbedtls_mpi_mod_mpi(y2, y2, &e->P)
		  /* x^2 + a */
	       || (e->A.MBEDTLS_PRIVATE(p)
	           ? mbedtls_mpi_add_mpi(y2, y2, &e->A)
	           : mbedtls_mpi_sub_int(y2, y2, 3))
	       || mbedtls_mpi_mod_mpi(y2, y2, &e->P)
		  /* (x^2 + a)x */
	       || mbedtls_mpi_mul_mpi(y2, y2, x)
	       || mbedtls_mpi_mod_mpi(y2, y2, &e->P)
		  /* (x^2 + a)x + b */
	       || mbedtls_mpi_add_mpi(y2, y2, &e->B)
	       || mbedtls_mpi_mod_mpi(y2, y2, &e->P);
  #endif
}
#endif /* MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED */

#if 0 /* not used by hostap */
#ifdef MBEDTLS_ECP_MONTGOMERY_ENABLED
static int
crypto_ec_point_y_sqr_montgomery(mbedtls_ecp_group *e, const mbedtls_mpi *x,
                                 mbedtls_mpi *y2)
{
	/* XXX: !!! must be reviewed and audited for correctness !!! */

	/* MBEDTLS_ECP_TYPE_MONTGOMERY         y^2 = x^3 + a x^2 + x  */

	/* y^2 = x^3 + a x^2 + x = (x + a)x^2 + x */
	mbedtls_mpi x2;
	mbedtls_mpi_init(&x2);
	int ret = /* x^2 */
	          mbedtls_mpi_mul_mpi(&x2, x, x)
	       || mbedtls_mpi_mod_mpi(&x2, &x2, &e->P)
		  /* x + a */
	       || mbedtls_mpi_add_mpi(y2,  x, &e->A)
	       || mbedtls_mpi_mod_mpi(y2, y2, &e->P)
		  /* (x + a)x^2 */
	       || mbedtls_mpi_mul_mpi(y2, y2, &x2)
	       || mbedtls_mpi_mod_mpi(y2, y2, &e->P)
		  /* (x + a)x^2 + x */
	       || mbedtls_mpi_add_mpi(y2, y2, x)
	       || mbedtls_mpi_mod_mpi(y2, y2, &e->P);
	mbedtls_mpi_free(&x2);
	return ret; /* 0: success, non-zero: failure */
}
#endif /* MBEDTLS_ECP_MONTGOMERY_ENABLED */
#endif

struct crypto_bignum *
crypto_ec_point_compute_y_sqr(struct crypto_ec *e,
			      const struct crypto_bignum *x)
{
	if (TEST_FAIL())
		return NULL;

	mbedtls_mpi *y2 = os_malloc(sizeof(*y2));
	if (y2 == NULL)
		return NULL;
	mbedtls_mpi_init(y2);

  #ifdef MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED
	if (mbedtls_ecp_get_type((mbedtls_ecp_group *)e)
	      == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS
	    && crypto_ec_point_y_sqr_weierstrass((mbedtls_ecp_group *)e,
	                                         (const mbedtls_mpi *)x,
	                                         y2) == 0)
		return (struct crypto_bignum *)y2;
  #endif
  #if 0 /* not used by hostap */
  #ifdef MBEDTLS_ECP_MONTGOMERY_ENABLED
	if (mbedtls_ecp_get_type((mbedtls_ecp_group *)e)
	      == MBEDTLS_ECP_TYPE_MONTGOMERY
	    && crypto_ec_point_y_sqr_montgomery((mbedtls_ecp_group *)e,
	                                        (const mbedtls_mpi *)x,
	                                        y2) == 0)
		return (struct crypto_bignum *)y2;
  #endif
  #endif

	mbedtls_mpi_free(y2);
	os_free(y2);
	return NULL;
}

int crypto_ec_point_is_at_infinity(struct crypto_ec *e,
				   const struct crypto_ec_point *p)
{
	return mbedtls_ecp_is_zero((mbedtls_ecp_point *)p);
}

int crypto_ec_point_is_on_curve(struct crypto_ec *e,
				const struct crypto_ec_point *p)
{
  #if 1
	return mbedtls_ecp_check_pubkey((const mbedtls_ecp_group *)e,
	                                (const mbedtls_ecp_point *)p) == 0;
  #else
	/* compute y^2 mod P and compare to y^2 mod P */
	/*(ref: src/eap_common/eap_pwd_common.c:compute_password_element())*/
	const mbedtls_mpi *px = &((const mbedtls_ecp_point *)p)->MBEDTLS_PRIVATE(X);
	mbedtls_mpi *cy2 = (mbedtls_mpi *)
	  crypto_ec_point_compute_y_sqr(e, (const struct crypto_bignum *)px);
	if (cy2 == NULL)
		return 0;

	mbedtls_mpi y2;
	mbedtls_mpi_init(&y2);
	const mbedtls_mpi *py = &((const mbedtls_ecp_point *)p)->MBEDTLS_PRIVATE(Y);
	int is_on_curve = mbedtls_mpi_mul_mpi(&y2, py, py) /* y^2 mod P */
	               || mbedtls_mpi_mod_mpi(&y2, &y2, CRYPTO_EC_P(e))
	               || mbedtls_mpi_cmp_mpi(&y2, cy2) != 0 ? 0 : 1;

	mbedtls_mpi_free(&y2);
	mbedtls_mpi_free(cy2);
	os_free(cy2);
	return is_on_curve;
  #endif
}

int crypto_ec_point_cmp(const struct crypto_ec *e,
			const struct crypto_ec_point *a,
			const struct crypto_ec_point *b)
{
	return mbedtls_ecp_point_cmp((const mbedtls_ecp_point *)a,
	                             (const mbedtls_ecp_point *)b);
}

#if !defined(CONFIG_NO_STDOUT_DEBUG)
void crypto_ec_point_debug_print(const struct crypto_ec *e,
				 const struct crypto_ec_point *p,
				 const char *title)
{
	u8 x[MBEDTLS_MPI_MAX_SIZE];
	u8 y[MBEDTLS_MPI_MAX_SIZE];
	size_t len = CRYPTO_EC_plen(e);
	/* crypto_ec_point_to_bin ought to take (const struct crypto_ec *e) */
	struct crypto_ec *ee;
	*(const struct crypto_ec **)&ee = e; /*(cast away const)*/
	if (crypto_ec_point_to_bin(ee, p, x, y) == 0) {
		if (title)
			wpa_printf(MSG_DEBUG, "%s", title);
		wpa_hexdump(MSG_DEBUG, "x:", x, len);
		wpa_hexdump(MSG_DEBUG, "y:", y, len);
	}
}
#endif


struct crypto_ec_key * crypto_ec_key_parse_priv(const u8 *der, size_t der_len)
{
	mbedtls_pk_context *ctx = os_malloc(sizeof(*ctx));
	if (ctx == NULL)
		return NULL;
	mbedtls_pk_init(ctx);
  #if MBEDTLS_VERSION_NUMBER < 0x03000000 /* mbedtls 3.0.0 */
	if (mbedtls_pk_parse_key(ctx, der, der_len, NULL, 0) == 0)
  #else
	if (mbedtls_pk_parse_key(ctx, der, der_len, NULL, 0,
	                         mbedtls_ctr_drbg_random,
	                         crypto_mbedtls_ctr_drbg()) == 0)
  #endif
		return (struct crypto_ec_key *)ctx;

	mbedtls_pk_free(ctx);
	os_free(ctx);
	return NULL;
}

#ifdef CRYPTO_MBEDTLS_CRYPTO_HPKE
#ifdef CONFIG_MODULE_TESTS
/*(for crypto_module_tests.c)*/
struct crypto_ec_key * crypto_ec_key_set_priv(int group,
					      const u8 *raw, size_t raw_len)
{
	mbedtls_ecp_group_id grp_id =
	  crypto_mbedtls_ecp_group_id_from_ike_id(group);
	if (grp_id == MBEDTLS_ECP_DP_NONE)
		return NULL;
	const mbedtls_pk_info_t *pk_info =
	  mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY);
	if (pk_info == NULL)
		return NULL;
	mbedtls_pk_context *ctx = os_malloc(sizeof(*ctx));
	if (ctx == NULL)
		return NULL;
	mbedtls_pk_init(ctx);
	if (mbedtls_pk_setup(ctx, pk_info) == 0
	    && mbedtls_ecp_read_key(grp_id,mbedtls_pk_ec(*ctx),raw,raw_len) == 0) {
		return (struct crypto_ec_key *)ctx;
	}

	mbedtls_pk_free(ctx);
	os_free(ctx);
	return NULL;
}
#endif
#endif

#include <mbedtls/error.h>
#include <mbedtls/oid.h>
static int crypto_mbedtls_pk_parse_subpubkey_compressed(mbedtls_pk_context *ctx, const u8 *der, size_t der_len)
{
    /* The following is modified from:
     *   mbedtls/library/pkparse.c:mbedtls_pk_parse_subpubkey()
     *   mbedtls/library/pkparse.c:pk_get_pk_alg()
     *   mbedtls/library/pkparse.c:pk_use_ecparams()
     */
    mbedtls_pk_type_t pk_alg = MBEDTLS_PK_NONE;
    const mbedtls_pk_info_t *pk_info;
    int ret;
    size_t len;
    const unsigned char *end = der+der_len;
    unsigned char *p;
    *(const unsigned char **)&p = der;

    if( ( ret = mbedtls_asn1_get_tag( &p, end, &len,
                    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE ) ) != 0 )
    {
        return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_PK_KEY_INVALID_FORMAT, ret ) );
    }

    end = p + len;

    /*
    if( ( ret = pk_get_pk_alg( &p, end, &pk_alg, &alg_params ) ) != 0 )
        return( ret );
    */
    mbedtls_asn1_buf alg_oid, params;
    memset( &params, 0, sizeof(mbedtls_asn1_buf) );
    if( ( ret = mbedtls_asn1_get_alg( &p, end, &alg_oid, &params ) ) != 0 )
        return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_PK_INVALID_ALG, ret ) );
    if( mbedtls_oid_get_pk_alg( &alg_oid, &pk_alg ) != 0 )
        return( MBEDTLS_ERR_PK_UNKNOWN_PK_ALG );

    if( ( ret = mbedtls_asn1_get_bitstring_null( &p, end, &len ) ) != 0 )
        return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_PK_INVALID_PUBKEY, ret ) );

    if( p + len != end )
        return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_PK_INVALID_PUBKEY,
                MBEDTLS_ERR_ASN1_LENGTH_MISMATCH ) );

    if( ( pk_info = mbedtls_pk_info_from_type( pk_alg ) ) == NULL )
        return( MBEDTLS_ERR_PK_UNKNOWN_PK_ALG );

    if( ( ret = mbedtls_pk_setup( ctx, pk_info ) ) != 0 )
        return( ret );

    /* assume mbedtls_pk_parse_subpubkey(&der, der+der_len, ctx)
     * has already run with ctx initialized up to pk_get_ecpubkey(),
     * and pk_get_ecpubkey() has returned MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE
     *
     * mbedtls mbedtls_ecp_point_read_binary()
     * does not handle point in COMPRESSED format
     *
     * (validate assumption that algorithm is EC) */
    mbedtls_ecp_keypair *ecp_kp = mbedtls_pk_ec(*ctx);
    if (ecp_kp == NULL)
        return( MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE );
    mbedtls_ecp_group *ecp_kp_grp = &ecp_kp->MBEDTLS_PRIVATE(grp);
    mbedtls_ecp_point *ecp_kp_Q = &ecp_kp->MBEDTLS_PRIVATE(Q);
    mbedtls_ecp_group_id grp_id;


    /* mbedtls/library/pkparse.c:pk_use_ecparams() */

    if( params.tag == MBEDTLS_ASN1_OID )
    {
        if( mbedtls_oid_get_ec_grp( &params, &grp_id ) != 0 )
            return( MBEDTLS_ERR_PK_UNKNOWN_NAMED_CURVE );
    }
    else
    {
#if defined(MBEDTLS_PK_PARSE_EC_EXTENDED)
        /*(large code block not copied from mbedtls; unsupported)*/
      #if 0
        if( ( ret = pk_group_id_from_specified( &params, &grp_id ) ) != 0 )
            return( ret );
      #endif
#endif
        return( MBEDTLS_ERR_PK_KEY_INVALID_FORMAT );
    }

    /*
     * grp may already be initialized; if so, make sure IDs match
     */
    if( ecp_kp_grp->id != MBEDTLS_ECP_DP_NONE && ecp_kp_grp->id != grp_id )
        return( MBEDTLS_ERR_PK_KEY_INVALID_FORMAT );

    if( ( ret = mbedtls_ecp_group_load( ecp_kp_grp, grp_id ) ) != 0 )
        return( ret );


    /* (validate assumption that EC point is in COMPRESSED format) */
    len = CRYPTO_EC_plen(ecp_kp_grp);
    if( mbedtls_ecp_get_type(ecp_kp_grp) != MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS
        || (end - p) != 1+len
        || (*p != 0x02 && *p != 0x03) )
        return( MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE );

    /* Instead of calling mbedtls/library/pkparse.c:pk_get_ecpubkey() to call
     * mbedtls_ecp_point_read_binary(), manually parse point into ecp_kp_Q */
    mbedtls_mpi *X = &ecp_kp_Q->MBEDTLS_PRIVATE(X);
    mbedtls_mpi *Y = &ecp_kp_Q->MBEDTLS_PRIVATE(Y);
    mbedtls_mpi *Z = &ecp_kp_Q->MBEDTLS_PRIVATE(Z);
    ret = mbedtls_mpi_lset(Z, 1);
    if (ret != 0)
        return( ret );
    ret = mbedtls_mpi_read_binary(X, p+1, len);
    if (ret != 0)
        return( ret );
    /* derive Y
     * (similar derivation of Y in crypto_mbedtls.c:crypto_ecdh_set_peerkey())*/
    ret = mbedtls_mpi_copy(Y, X) /*(Y is used as input and output obj below)*/
       || crypto_mbedtls_short_weierstrass_derive_y(ecp_kp_grp, Y, (*p & 1));
    if (ret != 0)
        return( ret );

    return mbedtls_ecp_check_pubkey( ecp_kp_grp, ecp_kp_Q );
}

struct crypto_ec_key * crypto_ec_key_parse_pub(const u8 *der, size_t der_len)
{
	mbedtls_pk_context *ctx = os_malloc(sizeof(*ctx));
	if (ctx == NULL)
		return NULL;
	mbedtls_pk_init(ctx);
	/*int rc = mbedtls_pk_parse_subpubkey(&der, der+der_len, ctx);*/
	int rc = mbedtls_pk_parse_public_key(ctx, der, der_len);
	if (rc == 0)
		return (struct crypto_ec_key *)ctx;
	else if (rc == MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE) {
		/* mbedtls mbedtls_ecp_point_read_binary()
		 * does not handle point in COMPRESSED format; parse internally */
		rc = crypto_mbedtls_pk_parse_subpubkey_compressed(ctx,der,der_len);
		if (rc == 0)
			return (struct crypto_ec_key *)ctx;
	}

	mbedtls_pk_free(ctx);
	os_free(ctx);
	return NULL;
}

#ifdef CRYPTO_MBEDTLS_CRYPTO_EC_DPP

static struct crypto_ec_key *
crypto_ec_key_set_pub_point_for_group(mbedtls_ecp_group_id grp_id,
                                      const mbedtls_ecp_point *pub,
                                      const u8 *buf, size_t len)
{
	const mbedtls_pk_info_t *pk_info =
	  mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY);
	if (pk_info == NULL)
		return NULL;
	mbedtls_pk_context *ctx = os_malloc(sizeof(*ctx));
	if (ctx == NULL)
		return NULL;
	mbedtls_pk_init(ctx);
	if (mbedtls_pk_setup(ctx, pk_info) == 0) {
		/* (Is private key generation necessary for callers?)
		 * alt: gen key then overwrite Q
		 *   mbedtls_ecp_gen_key(grp_id, ecp_kp,
	         *                       mbedtls_ctr_drbg_random,
	         *                       crypto_mbedtls_ctr_drbg()) == 0
	         */
		mbedtls_ecp_keypair *ecp_kp = mbedtls_pk_ec(*ctx);
		mbedtls_ecp_group *ecp_kp_grp = &ecp_kp->MBEDTLS_PRIVATE(grp);
		mbedtls_ecp_point *ecp_kp_Q = &ecp_kp->MBEDTLS_PRIVATE(Q);
		mbedtls_mpi *ecp_kp_d = &ecp_kp->MBEDTLS_PRIVATE(d);
		if (mbedtls_ecp_group_load(ecp_kp_grp, grp_id) == 0
		    && (pub
		         ? mbedtls_ecp_copy(ecp_kp_Q, pub) == 0
		         : mbedtls_ecp_point_read_binary(ecp_kp_grp, ecp_kp_Q,
		                                         buf, len) == 0)
		    && mbedtls_ecp_gen_privkey(ecp_kp_grp, ecp_kp_d,
		                               mbedtls_ctr_drbg_random,
		                               crypto_mbedtls_ctr_drbg()) == 0){
			return (struct crypto_ec_key *)ctx;
		}
	}

	mbedtls_pk_free(ctx);
	os_free(ctx);
	return NULL;
}

struct crypto_ec_key * crypto_ec_key_set_pub(int group, const u8 *x,
					     const u8 *y, size_t len)
{
	mbedtls_ecp_group_id grp_id =
	  crypto_mbedtls_ecp_group_id_from_ike_id(group);
	if (grp_id == MBEDTLS_ECP_DP_NONE)
		return NULL;
	if (len > MBEDTLS_MPI_MAX_SIZE)
		return NULL;
	u8 buf[1+MBEDTLS_MPI_MAX_SIZE*2];
	buf[0] = 0x04; /* assume x,y for Short Weierstrass */
	os_memcpy(buf+1, x, len);
	os_memcpy(buf+1+len, y, len);

	return crypto_ec_key_set_pub_point_for_group(grp_id,NULL,buf,1+len*2);
}

struct crypto_ec_key *
crypto_ec_key_set_pub_point(struct crypto_ec *e,
			    const struct crypto_ec_point *pub)
{
	mbedtls_ecp_group_id grp_id = ((mbedtls_ecp_group *)e)->id;
	mbedtls_ecp_point *p = (mbedtls_ecp_point *)pub;
	return crypto_ec_key_set_pub_point_for_group(grp_id, p, NULL, 0);
}


struct crypto_ec_key * crypto_ec_key_gen(int group)
{
	mbedtls_pk_context *ctx = os_malloc(sizeof(*ctx));
	if (ctx == NULL)
		return NULL;
	mbedtls_pk_init(ctx);
	if (crypto_mbedtls_keypair_gen(group, ctx) == 0)
		return (struct crypto_ec_key *)ctx;
	mbedtls_pk_free(ctx);
	os_free(ctx);
	return NULL;
}

#endif /* CRYPTO_MBEDTLS_CRYPTO_EC_DPP */

void crypto_ec_key_deinit(struct crypto_ec_key *key)
{
	mbedtls_pk_free((mbedtls_pk_context *)key);
	os_free(key);
}

struct wpabuf * crypto_ec_key_get_subject_public_key(struct crypto_ec_key *key)
{
	/* (similar to crypto_ec_key_get_pubkey_point(),
	 *  but compressed point format and ASN.1 DER wrapping)*/
#ifndef MBEDTLS_PK_ECP_PUB_DER_MAX_BYTES /*(mbedtls/library/pkwrite.h)*/
#define MBEDTLS_PK_ECP_PUB_DER_MAX_BYTES    ( 30 + 2 * MBEDTLS_ECP_MAX_BYTES )
#endif
	unsigned char buf[MBEDTLS_PK_ECP_PUB_DER_MAX_BYTES];
	int len = mbedtls_pk_write_pubkey_der((mbedtls_pk_context *)key,
	                                      buf, sizeof(buf));
	if (len < 0)
		return NULL;
	/*  Note: data is written at the end of the buffer! Use the
	 *        return value to determine where you should start
	 *        using the buffer */
	unsigned char *p = buf+sizeof(buf)-len;

  #ifdef MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED
	mbedtls_ecp_keypair *ecp_kp = mbedtls_pk_ec(*(mbedtls_pk_context *)key);
	if (ecp_kp == NULL)
		return NULL;
	mbedtls_ecp_group *grp = &ecp_kp->MBEDTLS_PRIVATE(grp);
	/*  Note: sae_pk.c expects pubkey point in compressed format,
	 *        but mbedtls_pk_write_pubkey_der() writes uncompressed format.
	 *        Manually translate format and update lengths in DER format */
	if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS) {
		unsigned char *end = buf+sizeof(buf);
		size_t n;
		/* SubjectPublicKeyInfo SEQUENCE */
		mbedtls_asn1_get_tag(&p, end, &n,
		    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
		/* algorithm AlgorithmIdentifier */
		unsigned char *a = p;
		size_t alen;
		mbedtls_asn1_get_tag(&p, end, &alen,
		    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
		p += alen;
		alen = (size_t)(p - a);
		/* subjectPublicKey BIT STRING */
		mbedtls_asn1_get_tag(&p, end, &n, MBEDTLS_ASN1_BIT_STRING);
		/* rewrite into compressed point format and rebuild ASN.1 */
		p[1] = (buf[sizeof(buf)-1] & 1) ? 0x03 : 0x02;
		n = 1 + 1 + (n-2)/2;
		len = mbedtls_asn1_write_len(&p, buf, n) + (int)n;
		len += mbedtls_asn1_write_tag(&p, buf, MBEDTLS_ASN1_BIT_STRING);
		os_memmove(p-alen, a, alen);
		len += alen;
		p -= alen;
		len += mbedtls_asn1_write_len(&p, buf, (size_t)len);
		len += mbedtls_asn1_write_tag(&p, buf,
		    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
	}
  #endif
	return wpabuf_alloc_copy(p, (size_t)len);
}

#ifdef CRYPTO_MBEDTLS_CRYPTO_EC_DPP

struct wpabuf * crypto_ec_key_get_ecprivate_key(struct crypto_ec_key *key,
						bool include_pub)
{
#ifndef MBEDTLS_PK_ECP_PRV_DER_MAX_BYTES /*(mbedtls/library/pkwrite.h)*/
#define MBEDTLS_PK_ECP_PRV_DER_MAX_BYTES    ( 29 + 3 * MBEDTLS_ECP_MAX_BYTES )
#endif
	unsigned char priv[MBEDTLS_PK_ECP_PRV_DER_MAX_BYTES];
	int privlen = mbedtls_pk_write_key_der((mbedtls_pk_context *)key,
	                                       priv, sizeof(priv));
	if (privlen < 0)
		return NULL;

	struct wpabuf *wbuf;

	/*  Note: data is written at the end of the buffer! Use the
	 *        return value to determine where you should start
	 *        using the buffer */
	/* mbedtls_pk_write_key_der() includes publicKey in DER */
	if (include_pub)
		wbuf = wpabuf_alloc_copy(priv+sizeof(priv)-privlen, privlen);
	else {
		/* calculate publicKey offset and skip from end of buffer */
		unsigned char *p = priv+sizeof(priv)-privlen;
		unsigned char *end = priv+sizeof(priv);
		size_t len;
		/* ECPrivateKey SEQUENCE */
		mbedtls_asn1_get_tag(&p, end, &len,
		    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
		/* version INTEGER */
		unsigned char *v = p;
		mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_INTEGER);
		p += len;
		/* privateKey OCTET STRING */
		mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
		p += len;
		/* parameters ECParameters */
		mbedtls_asn1_get_tag(&p, end, &len,
		    MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED);
		p += len;

		/* write new SEQUENCE header (we know that it fits in priv[]) */
		len = (size_t)(p - v);
		p = v;
		len += mbedtls_asn1_write_len(&p, priv, len);
		len += mbedtls_asn1_write_tag(&p, priv,
		    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
		wbuf = wpabuf_alloc_copy(p, len);
	}

	forced_memzero(priv, sizeof(priv));
	return wbuf;
}

struct wpabuf * crypto_ec_key_get_pubkey_point(struct crypto_ec_key *key,
					       int prefix)
{
	/*(similarities to crypto_ecdh_get_pubkey(), but different struct)*/
	mbedtls_ecp_keypair *ecp_kp = mbedtls_pk_ec(*(mbedtls_pk_context *)key);
	if (ecp_kp == NULL)
		return NULL;
	mbedtls_ecp_group *grp = &ecp_kp->MBEDTLS_PRIVATE(grp);
	size_t len = CRYPTO_EC_plen(grp);
  #ifdef MBEDTLS_ECP_MONTGOMERY_ENABLED
	/* len */
  #endif
  #ifdef MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED
	if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS)
		len = len*2+1;
  #endif
	struct wpabuf *buf = wpabuf_alloc(len);
	if (buf == NULL)
		return NULL;
	mbedtls_ecp_point *ecp_kp_Q = &ecp_kp->MBEDTLS_PRIVATE(Q);
	if (mbedtls_ecp_point_write_binary(grp, ecp_kp_Q,
	                                   MBEDTLS_ECP_PF_UNCOMPRESSED, &len,
	                                   wpabuf_mhead_u8(buf), len) == 0) {
		if (!prefix) /* Remove 0x04 prefix if requested */
			os_memmove(wpabuf_mhead(buf),wpabuf_mhead(buf)+1,--len);
		wpabuf_put(buf, len);
		return buf;
	}

	wpabuf_free(buf);
	return NULL;
}

struct crypto_ec_point *
crypto_ec_key_get_public_key(struct crypto_ec_key *key)
{
	mbedtls_ecp_keypair *ecp_kp = mbedtls_pk_ec(*(mbedtls_pk_context *)key);
	if (ecp_kp == NULL)
		return NULL;
	mbedtls_ecp_point *p = os_malloc(sizeof(*p));
	if (p != NULL) {
		/*(mbedtls_ecp_export() uses &ecp_kp->MBEDTLS_PRIVATE(grp))*/
		mbedtls_ecp_point_init(p);
		mbedtls_ecp_point *ecp_kp_Q = &ecp_kp->MBEDTLS_PRIVATE(Q);
		if (mbedtls_ecp_copy(p, ecp_kp_Q)) {
			mbedtls_ecp_point_free(p);
			os_free(p);
			p = NULL;
		}
	}
	return (struct crypto_ec_point *)p;
}

struct crypto_bignum *
crypto_ec_key_get_private_key(struct crypto_ec_key *key)
{
	mbedtls_ecp_keypair *ecp_kp = mbedtls_pk_ec(*(mbedtls_pk_context *)key);
	if (ecp_kp == NULL)
		return NULL;
	mbedtls_mpi *bn = os_malloc(sizeof(*bn));
	if (bn) {
		/*(mbedtls_ecp_export() uses &ecp_kp->MBEDTLS_PRIVATE(grp))*/
		mbedtls_mpi_init(bn);
		mbedtls_mpi *ecp_kp_d = &ecp_kp->MBEDTLS_PRIVATE(d);
		if (mbedtls_mpi_copy(bn, ecp_kp_d)) {
			mbedtls_mpi_free(bn);
			os_free(bn);
			bn = NULL;
		}
	}
	return (struct crypto_bignum *)bn;
}

#endif /* CRYPTO_MBEDTLS_CRYPTO_EC_DPP */

static mbedtls_md_type_t crypto_ec_key_sign_md(size_t len)
{
	/* get mbedtls_md_type_t from length of hash data to be signed */
	switch (len) {
	case 64: return MBEDTLS_MD_SHA512;
	case 48: return MBEDTLS_MD_SHA384;
	case 32: return MBEDTLS_MD_SHA256;
	case 20: return MBEDTLS_MD_SHA1;
	case 16: return MBEDTLS_MD_MD5;
	default: return MBEDTLS_MD_NONE;
	}
}

struct wpabuf * crypto_ec_key_sign(struct crypto_ec_key *key, const u8 *data,
				   size_t len)
{
  #ifndef MBEDTLS_PK_SIGNATURE_MAX_SIZE /*(defined since mbedtls 2.20.0)*/
  #if MBEDTLS_ECDSA_MAX_LEN > MBEDTLS_MPI_MAX_SIZE
  #define MBEDTLS_PK_SIGNATURE_MAX_SIZE MBEDTLS_ECDSA_MAX_LEN
  #else
  #define MBEDTLS_PK_SIGNATURE_MAX_SIZE MBEDTLS_MPI_MAX_SIZE
  #endif
  #endif
	size_t sig_len = MBEDTLS_PK_SIGNATURE_MAX_SIZE;
	struct wpabuf *buf = wpabuf_alloc(sig_len);
	if (buf == NULL)
		return NULL;
	if (mbedtls_pk_sign((mbedtls_pk_context *)key,
	                    crypto_ec_key_sign_md(len), data, len,
	                    wpabuf_mhead_u8(buf),
  #if MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
	                    sig_len,
  #endif
	                    &sig_len,
	                    mbedtls_ctr_drbg_random,
	                    crypto_mbedtls_ctr_drbg()) == 0) {
		wpabuf_put(buf, sig_len);
		return buf;
	}

	wpabuf_free(buf);
	return NULL;
}

#ifdef CRYPTO_MBEDTLS_CRYPTO_EC_DPP
struct wpabuf * crypto_ec_key_sign_r_s(struct crypto_ec_key *key,
				       const u8 *data, size_t len)
{
	mbedtls_ecp_keypair *ecp_kp = mbedtls_pk_ec(*(mbedtls_pk_context *)key);
	if (ecp_kp == NULL)
		return NULL;

	size_t sig_len = MBEDTLS_ECDSA_MAX_LEN;
	u8 buf[MBEDTLS_ECDSA_MAX_LEN];
	if (mbedtls_ecdsa_write_signature(ecp_kp, crypto_ec_key_sign_md(len),
	                                  data, len, buf,
  #if MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
	                                  sig_len,
  #endif
	                                  &sig_len,
	                                  mbedtls_ctr_drbg_random,
	                                  crypto_mbedtls_ctr_drbg())) {
		return NULL;
	}

	/*(mbedtls_ecdsa_write_signature() writes signature in ASN.1)*/
	/* parse ASN.1 to get r and s and lengths */
	u8 *p = buf, *r, *s;
	u8 *end = p + sig_len;
	size_t rlen, slen;
	mbedtls_asn1_get_tag(&p, end, &rlen,
	  MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
	mbedtls_asn1_get_tag(&p, end, &rlen, MBEDTLS_ASN1_INTEGER);
	r = p;
	p += rlen;
	mbedtls_asn1_get_tag(&p, end, &slen, MBEDTLS_ASN1_INTEGER);
	s = p;

	/* write raw r and s into out
	 * (including removal of leading 0 if added for ASN.1 integer)
	 * note: DPP caller expects raw r, s each padded to prime len */
	mbedtls_ecp_group *ecp_kp_grp = &ecp_kp->MBEDTLS_PRIVATE(grp);
	size_t plen = CRYPTO_EC_plen(ecp_kp_grp);
	if (rlen > plen) {
		r += (rlen - plen);
		rlen = plen;
	}
	if (slen > plen) {
		s += (slen - plen);
		slen = plen;
	}
	struct wpabuf *out = wpabuf_alloc(plen*2);
	if (out) {
		wpabuf_put(out, plen*2);
		p = wpabuf_mhead_u8(out);
		os_memset(p, 0, plen*2);
		os_memcpy(p+plen*1-rlen, r, rlen);
		os_memcpy(p+plen*2-slen, s, slen);
	}
	return out;
}
#endif /* CRYPTO_MBEDTLS_CRYPTO_EC_DPP */

int crypto_ec_key_verify_signature(struct crypto_ec_key *key, const u8 *data,
				   size_t len, const u8 *sig, size_t sig_len)
{
	switch (mbedtls_pk_verify((mbedtls_pk_context *)key,
	                          crypto_ec_key_sign_md(len), data, len,
	                          sig, sig_len)) {
	case 0:
	/*case MBEDTLS_ERR_ECP_SIG_LEN_MISMATCH:*//* XXX: allow? */
		return 1;
	case MBEDTLS_ERR_ECP_VERIFY_FAILED:
		return 0;
	default:
		return -1;
	}
}

#ifdef CRYPTO_MBEDTLS_CRYPTO_EC_DPP
int crypto_ec_key_verify_signature_r_s(struct crypto_ec_key *key,
				       const u8 *data, size_t len,
				       const u8 *r, size_t r_len,
				       const u8 *s, size_t s_len)
{
	/* reimplement mbedtls_ecdsa_read_signature() without encoding r and s
	 * into ASN.1 just for mbedtls_ecdsa_read_signature() to decode ASN.1 */
	mbedtls_ecp_keypair *ecp_kp = mbedtls_pk_ec(*(mbedtls_pk_context *)key);
	if (ecp_kp == NULL)
		return -1;
	mbedtls_ecp_group *ecp_kp_grp = &ecp_kp->MBEDTLS_PRIVATE(grp);
	mbedtls_ecp_point *ecp_kp_Q = &ecp_kp->MBEDTLS_PRIVATE(Q);

	mbedtls_mpi mpi_r;
	mbedtls_mpi mpi_s;
	mbedtls_mpi_init(&mpi_r);
	mbedtls_mpi_init(&mpi_s);
	int ret = mbedtls_mpi_read_binary(&mpi_r, r, r_len)
	       || mbedtls_mpi_read_binary(&mpi_s, s, s_len) ? -1 : 0;
	if (ret == 0) {
		ret = mbedtls_ecdsa_verify(ecp_kp_grp, data, len,
		                           ecp_kp_Q, &mpi_r, &mpi_s);
		ret = ret ? ret == MBEDTLS_ERR_ECP_BAD_INPUT_DATA ? 0 : -1 : 1;
	}
	mbedtls_mpi_free(&mpi_r);
	mbedtls_mpi_free(&mpi_s);
	return ret;
}
#endif /* CRYPTO_MBEDTLS_CRYPTO_EC_DPP */

int crypto_ec_key_group(struct crypto_ec_key *key)
{
	mbedtls_ecp_keypair *ecp_kp = mbedtls_pk_ec(*(mbedtls_pk_context *)key);
	if (ecp_kp == NULL)
		return -1;
	mbedtls_ecp_group *ecp_group = &ecp_kp->MBEDTLS_PRIVATE(grp);
	return crypto_mbedtls_ike_id_from_ecp_group_id(ecp_group->id);
}

#ifdef CRYPTO_MBEDTLS_CRYPTO_EC_DPP

int crypto_ec_key_cmp(struct crypto_ec_key *key1, struct crypto_ec_key *key2)
{
#if 0 /*(DPP is passing two public keys; unable to use pk_check_pair())*/
  #if MBEDTLS_VERSION_NUMBER < 0x03000000 /* mbedtls 3.0.0 */
	return mbedtls_pk_check_pair((const mbedtls_pk_context *)key1,
	                             (const mbedtls_pk_context *)key2) ? -1 : 0;
  #else
	return mbedtls_pk_check_pair((const mbedtls_pk_context *)key1,
	                             (const mbedtls_pk_context *)key2,
	                             mbedtls_ctr_drbg_random,
	                             crypto_mbedtls_ctr_drbg()) ? -1 : 0;
  #endif
#else
	mbedtls_ecp_keypair *ecp_kp1=mbedtls_pk_ec(*(mbedtls_pk_context *)key1);
	mbedtls_ecp_keypair *ecp_kp2=mbedtls_pk_ec(*(mbedtls_pk_context *)key2);
	if (ecp_kp1 == NULL || ecp_kp2 == NULL)
		return -1;
	mbedtls_ecp_group *ecp_kp1_grp = &ecp_kp1->MBEDTLS_PRIVATE(grp);
	mbedtls_ecp_group *ecp_kp2_grp = &ecp_kp2->MBEDTLS_PRIVATE(grp);
	mbedtls_ecp_point *ecp_kp1_Q = &ecp_kp1->MBEDTLS_PRIVATE(Q);
	mbedtls_ecp_point *ecp_kp2_Q = &ecp_kp2->MBEDTLS_PRIVATE(Q);
	return ecp_kp1_grp->id != ecp_kp2_grp->id
	    || mbedtls_ecp_point_cmp(ecp_kp1_Q, ecp_kp2_Q) ? -1 : 0;
#endif
}

void crypto_ec_key_debug_print(const struct crypto_ec_key *key,
			       const char *title)
{
	/* TBD: what info is desirable here and in what human readable format?*/
	/*(crypto_openssl.c prints a human-readably public key and attributes)*/
  #if 0
	struct mbedtls_pk_debug_item debug_item;
	if (mbedtls_pk_debug((const mbedtls_pk_context *)key, &debug_item))
		return;
	/* ... */
  #endif
	wpa_printf(MSG_DEBUG, "%s: %s not implemented", title, __func__);
}

#endif /* CRYPTO_MBEDTLS_CRYPTO_EC_DPP */

#endif /* CRYPTO_MBEDTLS_CRYPTO_EC */


#ifdef CRYPTO_MBEDTLS_CRYPTO_CSR

#include <mbedtls/x509_csr.h>
#include <mbedtls/oid.h>

struct crypto_csr * crypto_csr_init(void)
{
	mbedtls_x509write_csr *csr = os_malloc(sizeof(*csr));
	if (csr != NULL)
		mbedtls_x509write_csr_init(csr);
	return (struct crypto_csr *)csr;
}

struct crypto_csr * crypto_csr_verify(const struct wpabuf *req)
{
	/* future: look for alternatives to MBEDTLS_PRIVATE() access */

	/* sole caller src/common/dpp_crypto.c:dpp_validate_csr()
	 * uses (mbedtls_x509_csr *) to obtain CSR_ATTR_CHALLENGE_PASSWORD
	 * so allocate different object (mbedtls_x509_csr *) and special-case
	 * object when used in crypto_csr_get_attribute() and when free()d in
	 * crypto_csr_deinit(). */

	mbedtls_x509_csr *csr = os_malloc(sizeof(*csr));
	if (csr == NULL)
		return NULL;
	mbedtls_x509_csr_init(csr);
	const mbedtls_md_info_t *md_info;
	unsigned char digest[MBEDTLS_MD_MAX_SIZE];
	if (mbedtls_x509_csr_parse_der(csr,wpabuf_head(req),wpabuf_len(req))==0
	    && (md_info=mbedtls_md_info_from_type(csr->MBEDTLS_PRIVATE(sig_md)))
	       != NULL
	    && mbedtls_md(md_info, csr->cri.p, csr->cri.len, digest) == 0) {
		switch (mbedtls_pk_verify(&csr->pk,csr->MBEDTLS_PRIVATE(sig_md),
		                          digest, mbedtls_md_get_size(md_info),
		                          csr->MBEDTLS_PRIVATE(sig).p,
		                          csr->MBEDTLS_PRIVATE(sig).len)) {
		case 0:
		/*case MBEDTLS_ERR_ECP_SIG_LEN_MISMATCH:*//* XXX: allow? */
			return (struct crypto_csr *)((uintptr_t)csr | 1uL);
		default:
			break;
		}
	}

	mbedtls_x509_csr_free(csr);
	os_free(csr);
	return NULL;
}

void crypto_csr_deinit(struct crypto_csr *csr)
{
	if ((uintptr_t)csr & 1uL) {
		csr = (struct crypto_csr *)((uintptr_t)csr & ~1uL);
		mbedtls_x509_csr_free((mbedtls_x509_csr *)csr);
	}
	else
		mbedtls_x509write_csr_free((mbedtls_x509write_csr *)csr);
	os_free(csr);
}

int crypto_csr_set_ec_public_key(struct crypto_csr *csr,
				 struct crypto_ec_key *key)
{
	mbedtls_x509write_csr_set_key((mbedtls_x509write_csr *)csr,
	                              (mbedtls_pk_context *)key);
	return 0;
}

int crypto_csr_set_name(struct crypto_csr *csr, enum crypto_csr_name type,
			const char *name)
{
	/* specialized for src/common/dpp_crypto.c */

	/* sole caller src/common/dpp_crypto.c:dpp_build_csr()
	 * calls this function only once, using type == CSR_NAME_CN
	 * (If called more than once, this code would need to append
	 *  components to the subject name, which we could do by
	 *  appending to (mbedtls_x509write_csr *) private member
	 *  mbedtls_asn1_named_data *MBEDTLS_PRIVATE(subject)) */

	const char *label;
	switch (type) {
	case CSR_NAME_CN: label = "CN="; break;
	case CSR_NAME_SN: label = "SN="; break;
	case CSR_NAME_C:  label = "C=";  break;
	case CSR_NAME_O:  label = "O=";  break;
	case CSR_NAME_OU: label = "OU="; break;
	default: return -1;
	}

	size_t len = strlen(name);
	struct wpabuf *buf = wpabuf_alloc(3+len+1);
	if (buf == NULL)
		return -1;
	wpabuf_put_data(buf, label, strlen(label));
	wpabuf_put_data(buf, name, len+1); /*(include trailing '\0')*/
	/* Note: 'name' provided is set as given and should be backslash-escaped
	 * by caller when necessary, e.g. literal ',' which are not separating
	 * components should be backslash-escaped */

	int ret =
	  mbedtls_x509write_csr_set_subject_name((mbedtls_x509write_csr *)csr,
	                                         wpabuf_head(buf)) ? -1 : 0;
	wpabuf_free(buf);
	return ret;
}

/* OBJ_pkcs9_challengePassword  1 2 840 113549 1 9 7 */
static const char OBJ_pkcs9_challengePassword[] = MBEDTLS_OID_PKCS9 "\x07";

int crypto_csr_set_attribute(struct crypto_csr *csr, enum crypto_csr_attr attr,
			     int attr_type, const u8 *value, size_t len)
{
	/* specialized for src/common/dpp_crypto.c */
	/* sole caller src/common/dpp_crypto.c:dpp_build_csr() passes
	 *   attr      == CSR_ATTR_CHALLENGE_PASSWORD
	 *   attr_type == ASN1_TAG_UTF8STRING */

	const char *oid;
	size_t oid_len;
	switch (attr) {
	case CSR_ATTR_CHALLENGE_PASSWORD:
		oid = OBJ_pkcs9_challengePassword;
		oid_len = sizeof(OBJ_pkcs9_challengePassword)-1;
		break;
	default:
		return -1;
	}

  #if 0 /*(incorrect; sets an extension, not an attribute)*/
	return mbedtls_x509write_csr_set_extension((mbedtls_x509write_csr *)csr,
	                                           oid, oid_len,
	  #if MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
	                                           0, /*(critical flag)*/
	  #endif
	                                           value, len) ? -1 : 0;
  #else
	(void)oid;
	(void)oid_len;
  #endif

	/* mbedtls does not currently provide way to set an attribute in a CSR:
	 *   https://github.com/Mbed-TLS/mbedtls/issues/4886 */
	wpa_printf(MSG_ERROR,
	  "mbedtls does not currently support setting challengePassword "
	  "attribute in CSR");
	return -1;
}

const u8 * mbedtls_x509_csr_attr_oid_value(mbedtls_x509_csr *csr,
                                           const char *oid, size_t oid_len,
                                           size_t *vlen, int *vtype)
{
	/* Note: mbedtls_x509_csr_parse_der() has parsed and validated CSR,
	 *	   so validation checks are not repeated here
	 *
	 * It would be nicer if (mbedtls_x509_csr *) had an mbedtls_x509_buf of
	 * Attributes (or at least a pointer) since mbedtls_x509_csr_parse_der()
	 * already parsed the rest of CertificationRequestInfo, some of which is
	 * repeated here to step to Attributes.  Since csr->subject_raw.p points
	 * into csr->cri.p, which points into csr->raw.p, step over version and
	 * subject of CertificationRequestInfo (SEQUENCE) */
	unsigned char *p = csr->subject_raw.p + csr->subject_raw.len;
	unsigned char *end = csr->cri.p + csr->cri.len, *ext;
	size_t len;

	/* step over SubjectPublicKeyInfo */
	mbedtls_asn1_get_tag(&p, end, &len,
	    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
	p += len;

	/* Attributes
	 *   { ATTRIBUTE:IOSet } ::= SET OF { SEQUENCE { OID, value } }
	 */
	if (mbedtls_asn1_get_tag(&p, end, &len,
	      MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_CONTEXT_SPECIFIC) != 0) {
		return NULL;
	}
	while (p < end) {
		if (mbedtls_asn1_get_tag(&p, end, &len,
		      MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE) != 0) {
			return NULL;
		}
		ext = p;
		p += len;

		if (mbedtls_asn1_get_tag(&ext,end,&len,MBEDTLS_ASN1_OID) != 0)
			return NULL;
		if (oid_len != len || 0 != memcmp(ext, oid, oid_len))
			continue;

		/* found oid; return value */
		*vtype = *ext++; /* tag */
		return (mbedtls_asn1_get_len(&ext,end,vlen) == 0) ? ext : NULL;
	}

	return NULL;
}

const u8 * crypto_csr_get_attribute(struct crypto_csr *csr,
				    enum crypto_csr_attr attr,
				    size_t *len, int *type)
{
	/* specialized for src/common/dpp_crypto.c */
	/* sole caller src/common/dpp_crypto.c:dpp_build_csr() passes
	 *   attr == CSR_ATTR_CHALLENGE_PASSWORD */

	const char *oid;
	size_t oid_len;
	switch (attr) {
	case CSR_ATTR_CHALLENGE_PASSWORD:
		oid = OBJ_pkcs9_challengePassword;
		oid_len = sizeof(OBJ_pkcs9_challengePassword)-1;
		break;
	default:
		return NULL;
	}

	/* see crypto_csr_verify(); expecting (mbedtls_x509_csr *) tagged |=1 */
	if (!((uintptr_t)csr & 1uL))
		return NULL;
	csr = (struct crypto_csr *)((uintptr_t)csr & ~1uL);

	return mbedtls_x509_csr_attr_oid_value((mbedtls_x509_csr *)csr,
	                                       oid, oid_len, len, type);
}

struct wpabuf * crypto_csr_sign(struct crypto_csr *csr,
				struct crypto_ec_key *key,
				enum crypto_hash_alg algo)
{
	mbedtls_md_type_t sig_md;
	switch (algo) {
  #ifdef MBEDTLS_SHA256_C
	case CRYPTO_HASH_ALG_SHA256: sig_md = MBEDTLS_MD_SHA256; break;
  #endif
  #ifdef MBEDTLS_SHA512_C
	case CRYPTO_HASH_ALG_SHA384: sig_md = MBEDTLS_MD_SHA384; break;
	case CRYPTO_HASH_ALG_SHA512: sig_md = MBEDTLS_MD_SHA512; break;
  #endif
	default:
		return NULL;
	}
	mbedtls_x509write_csr_set_md_alg((mbedtls_x509write_csr *)csr, sig_md);

  #if 0
	unsigned char key_usage = MBEDTLS_X509_KU_DIGITAL_SIGNATURE
	                        | MBEDTLS_X509_KU_KEY_CERT_SIGN;
	if (mbedtls_x509write_csr_set_key_usage((mbedtls_x509write_csr *)csr,
	                                        key_usage))
		return NULL;
  #endif

  #if 0
	unsigned char ns_cert_type = MBEDTLS_X509_NS_CERT_TYPE_SSL_CLIENT
	                           | MBEDTLS_X509_NS_CERT_TYPE_EMAIL;
	if (mbedtls_x509write_csr_set_ns_cert_type((mbedtls_x509write_csr *)csr,
	                                           ns_cert_type))
		return NULL;
  #endif

  #if 0
	/* mbedtls does not currently provide way to set an attribute in a CSR:
	 *   https://github.com/Mbed-TLS/mbedtls/issues/4886
	 * XXX: hwsim dpp_enterprise test fails due to this limitation.
	 *
	 * Current usage of this function is solely by dpp_build_csr(),
	 * so as a kludge, might consider custom (struct crypto_csr *)
	 * containing (mbedtls_x509write_csr *) and a list of attributes
	 * (i.e. challengePassword).  Might have to totally reimplement
	 * mbedtls_x509write_csr_der(); underlying x509write_csr_der_internal()
	 * handles signing the CSR.  (This is more work that appending an
	 * Attributes section to end of CSR and adjusting ASN.1 length of CSR.)
	 */
  #endif

	unsigned char buf[4096]; /* XXX: large enough?  too large? */
	int len = mbedtls_x509write_csr_der((mbedtls_x509write_csr *)csr,
	                                    buf, sizeof(buf),
	                                    mbedtls_ctr_drbg_random,
	                                    crypto_mbedtls_ctr_drbg());
	if (len < 0)
		return NULL;
	/*  Note: data is written at the end of the buffer! Use the
	 *        return value to determine where you should start
	 *        using the buffer */
	return wpabuf_alloc_copy(buf+sizeof(buf)-len, (size_t)len);
}

#endif /* CRYPTO_MBEDTLS_CRYPTO_CSR */


#ifdef CRYPTO_MBEDTLS_CRYPTO_PKCS7

#if 0
#include <mbedtls/pkcs7.h> /* PKCS7 is not currently supported in mbedtls */
#include <mbedtls/pem.h>
#endif

struct wpabuf * crypto_pkcs7_get_certificates(const struct wpabuf *pkcs7)
{
	/* PKCS7 is not currently supported in mbedtls */
	return NULL;

#if 0
	/* https://github.com/naynajain/mbedtls-1 branch: development-pkcs7
	 * (??? potential future contribution to mbedtls ???) */

	/* Note: PKCS7 signature *is not* verified by this function.
	 * The function interface does not provide for passing a certificate */

	mbedtls_pkcs7 mpkcs7;
	mbedtls_pkcs7_init(&mpkcs7);
	int pkcs7_type = mbedtls_pkcs7_parse_der(wpabuf_head(pkcs7),
	                                         wpabuf_len(pkcs7),
	                                         &mpkcs7);
	wpabuf *buf = NULL;
	do {
		if (pkcs7_type < 0)
			break;

		/* src/common/dpp.c:dpp_parse_cred_dot1x() interested in certs
		 * for wpa_supplicant/dpp_supplicant.c:wpas_dpp_add_network()
		 * (? are adding certificate headers and footers desired ?) */

		/* development-pkcs7 branch does not currently provide
		 * additional interfaces to retrieve the parsed data */

		mbedtls_x509_crt *certs =
		  &mpkcs7.MBEDTLS_PRIVATE(signed_data).MBEDTLS_PRIVATE(certs);
		int ncerts =
		  mpkcs7.MBEDTLS_PRIVATE(signed_data).MBEDTLS_PRIVATE(no_of_certs);

		/* allocate buffer for PEM (base64-encoded DER)
		 * plus header, footer, newlines, and some extra */
		buf = wpabuf_alloc((wpabuf_len(pkcs7)+2)/3*4 + ncerts*64);
		if (buf == NULL)
			break;

		#define PEM_BEGIN_CRT "-----BEGIN CERTIFICATE-----\n"
		#define PEM_END_CRT   "-----END CERTIFICATE-----\n"
		size_t olen;
		for (int i = 0; i < ncerts; ++i) {
			int ret = mbedtls_pem_write_buffer(
			            PEM_BEGIN_CRT, PEM_END_CRT,
			            certs[i].raw.p, certs[i].raw.len,
			            wpabuf_mhead(buf, 0), wpabuf_tailroom(buf),
			            &olen));
			if (ret == 0)
				wpabuf_put(buf, olen);
			} else {
				if (ret == MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL)
					ret = wpabuf_resize(
					        &buf,olen-wpabuf_tailroom(buf));
				if (ret == 0) {
					--i;/*(adjust loop iterator for retry)*/
					continue;
				}
				wpabuf_free(buf);
				buf = NULL;
				break;
			}
		}
	} while (0);

	mbedtls_pkcs7_free(&mpkcs7);
	return buf;
#endif
}

#endif /* CRYPTO_MBEDTLS_CRYPTO_PKCS7 */


#ifdef MBEDTLS_ARC4_C
#include <mbedtls/arc4.h>
int rc4_skip(const u8 *key, size_t keylen, size_t skip,
	     u8 *data, size_t data_len)
{
	mbedtls_arc4_context ctx;
	mbedtls_arc4_init(&ctx);
	mbedtls_arc4_setup(&ctx, key, keylen);

	if (skip) {
		/*(prefer [16] on ancient hardware with smaller cache lines)*/
		unsigned char skip_buf[64]; /*('skip' is generally small)*/
		/*os_memset(skip_buf, 0, sizeof(skip_buf));*/ /*(necessary?)*/
		size_t len;
		do {
			len = skip > sizeof(skip_buf) ? sizeof(skip_buf) : skip;
			mbedtls_arc4_crypt(&ctx, len, skip_buf, skip_buf);
		} while ((skip -= len));
	}

	int ret = mbedtls_arc4_crypt(&ctx, data_len, data, data);
	mbedtls_arc4_free(&ctx);
	return ret;
}
#endif


/* duplicated in tls_mbedtls.c:tls_mbedtls_readfile()*/
__attribute_noinline__
static int crypto_mbedtls_readfile(const char *path, u8 **buf, size_t *n)
{
  #if 0 /* #ifdef MBEDTLS_FS_IO */
	/*(includes +1 for '\0' needed by mbedtls PEM parsing funcs)*/
	if (mbedtls_pk_load_file(path, (unsigned char **)buf, n) != 0) {
		wpa_printf(MSG_ERROR, "error: mbedtls_pk_load_file %s", path);
		return -1;
	}
  #else
	/*(use os_readfile() so that we can use os_free()
	 *(if we use mbedtls_pk_load_file() above, macros prevent calling free()
	 * directly #if defined(OS_REJECT_C_LIB_FUNCTIONS) and calling os_free()
	 * on buf aborts in tests if buf not allocated via os_malloc())*/
	*buf = (u8 *)os_readfile(path, n);
	if (!*buf) {
		wpa_printf(MSG_ERROR, "error: os_readfile %s", path);
		return -1;
	}
	u8 *buf0 = os_realloc(*buf, *n+1);
	if (!buf0) {
		bin_clear_free(*buf, *n);
		*buf = NULL;
		return -1;
	}
	buf0[(*n)++] = '\0';
	*buf = buf0;
  #endif
	return 0;
}


#ifdef CRYPTO_MBEDTLS_CRYPTO_RSA
#ifdef MBEDTLS_RSA_C

#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>

struct crypto_rsa_key * crypto_rsa_key_read(const char *file, bool private_key)
{
	/* mbedtls_pk_parse_keyfile() and mbedtls_pk_parse_public_keyfile()
	 * require #ifdef MBEDTLS_FS_IO in mbedtls library.  Prefer to use
	 * crypto_mbedtls_readfile(), which wraps os_readfile() */
	u8 *data;
	size_t len;
	if (crypto_mbedtls_readfile(file, &data, &len) != 0)
		return NULL;

	mbedtls_pk_context *ctx = os_malloc(sizeof(*ctx));
	if (ctx == NULL) {
		bin_clear_free(data, len);
		return NULL;
	}
	mbedtls_pk_init(ctx);

	int rc;
	rc = (private_key
	      ? mbedtls_pk_parse_key(ctx, data, len, NULL, 0
	  #if MBEDTLS_VERSION_NUMBER >= 0x03000000 /* mbedtls 3.0.0 */
	                            ,mbedtls_ctr_drbg_random,
	                             crypto_mbedtls_ctr_drbg()
	  #endif
	                            )
	      : mbedtls_pk_parse_public_key(ctx, data, len)) == 0
	    && mbedtls_pk_can_do(ctx, MBEDTLS_PK_RSA);

	bin_clear_free(data, len);

	if (rc) {
		/* use MBEDTLS_RSA_PKCS_V21 padding for RSAES-OAEP */
		/* use MBEDTLS_MD_SHA256 for these hostap interfaces */
	  #if MBEDTLS_VERSION_NUMBER < 0x03000000 /* mbedtls 3.0.0 */
		/*(no return value in mbedtls 2.x)*/
		mbedtls_rsa_set_padding(mbedtls_pk_rsa(*ctx),
		                        MBEDTLS_RSA_PKCS_V21,
		                        MBEDTLS_MD_SHA256);
	  #else
		if (mbedtls_rsa_set_padding(mbedtls_pk_rsa(*ctx),
		                            MBEDTLS_RSA_PKCS_V21,
		                            MBEDTLS_MD_SHA256) == 0)
	  #endif
			return (struct crypto_rsa_key *)ctx;
	}

	mbedtls_pk_free(ctx);
	os_free(ctx);
	return NULL;
}

struct wpabuf * crypto_rsa_oaep_sha256_encrypt(struct crypto_rsa_key *key,
					       const struct wpabuf *in)
{
	mbedtls_rsa_context *pk_rsa = mbedtls_pk_rsa(*(mbedtls_pk_context*)key);
	size_t olen = mbedtls_rsa_get_len(pk_rsa);
	struct wpabuf *buf = wpabuf_alloc(olen);
	if (buf == NULL)
		return NULL;

	/* mbedtls_pk_encrypt() takes a few more hops to get to same func */
	if (mbedtls_rsa_rsaes_oaep_encrypt(pk_rsa,
	                                   mbedtls_ctr_drbg_random,
	                                   crypto_mbedtls_ctr_drbg(),
	  #if MBEDTLS_VERSION_NUMBER < 0x03000000 /* mbedtls 3.0.0 */
	                                   MBEDTLS_RSA_PRIVATE,
	  #endif
	                                   NULL, 0,
	                                   wpabuf_len(in), wpabuf_head(in),
	                                   wpabuf_put(buf, olen)) == 0) {
		return buf;
	}

	wpabuf_clear_free(buf);
	return NULL;
}

struct wpabuf * crypto_rsa_oaep_sha256_decrypt(struct crypto_rsa_key *key,
					       const struct wpabuf *in)
{
	mbedtls_rsa_context *pk_rsa = mbedtls_pk_rsa(*(mbedtls_pk_context*)key);
	size_t olen = mbedtls_rsa_get_len(pk_rsa);
	struct wpabuf *buf = wpabuf_alloc(olen);
	if (buf == NULL)
		return NULL;

	/* mbedtls_pk_decrypt() takes a few more hops to get to same func */
	if (mbedtls_rsa_rsaes_oaep_decrypt(pk_rsa,
	                                   mbedtls_ctr_drbg_random,
	                                   crypto_mbedtls_ctr_drbg(),
	  #if MBEDTLS_VERSION_NUMBER < 0x03000000 /* mbedtls 3.0.0 */
	                                   MBEDTLS_RSA_PUBLIC,
	  #endif
	                                   NULL, 0, &olen, wpabuf_head(in),
	                                   wpabuf_mhead(buf), olen) == 0) {
		wpabuf_put(buf, olen);
		return buf;
	}

	wpabuf_clear_free(buf);
	return NULL;
}

void crypto_rsa_key_free(struct crypto_rsa_key *key)
{
	mbedtls_pk_free((mbedtls_pk_context *)key);
	os_free(key);
}

#endif /* MBEDTLS_RSA_C */
#endif /* CRYPTO_MBEDTLS_CRYPTO_RSA */

#ifdef CRYPTO_MBEDTLS_CRYPTO_HPKE

struct wpabuf * hpke_base_seal(enum hpke_kem_id kem_id,
			       enum hpke_kdf_id kdf_id,
			       enum hpke_aead_id aead_id,
			       struct crypto_ec_key *peer_pub,
			       const u8 *info, size_t info_len,
			       const u8 *aad, size_t aad_len,
			       const u8 *pt, size_t pt_len)
{
	/* not yet implemented */
	return NULL;
}

struct wpabuf * hpke_base_open(enum hpke_kem_id kem_id,
			       enum hpke_kdf_id kdf_id,
			       enum hpke_aead_id aead_id,
			       struct crypto_ec_key *own_priv,
			       const u8 *info, size_t info_len,
			       const u8 *aad, size_t aad_len,
			       const u8 *enc_ct, size_t enc_ct_len)
{
	/* not yet implemented */
	return NULL;
}

#endif
