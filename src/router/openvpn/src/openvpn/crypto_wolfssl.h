/*
 *  OpenVPN -- An application to securely tunnel IP networks
 *             over a single TCP/UDP port, with support for SSL/TLS-based
 *             session authentication and key exchange,
 *             packet encryption, packet authentication, and
 *             packet compression.
 *
 *  Copyright (C) 2002-2019 OpenVPN Inc <sales@openvpn.net>
 *  Copyright (C) 2010-2019 Fox Crypto B.V. <openvpn@fox-it.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * @file Data Channel Cryptography wolfSSL-specific backend interface
 */

#ifndef CRYPTO_WOLFSSL_H_
#define CRYPTO_WOLFSSL_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#elif defined(_MSC_VER)
#include "config-msvc.h"
#endif

#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/logging.h>
#include <wolfssl/wolfcrypt/wc_port.h>
#include <wolfssl/wolfcrypt/coding.h>
#include <wolfssl/wolfcrypt/types.h>
#include <wolfssl/ssl.h>

// Digests
#include <wolfssl/wolfcrypt/md4.h>
#include <wolfssl/wolfcrypt/md5.h>
#include <wolfssl/wolfcrypt/sha.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/wolfcrypt/sha3.h>
#include <wolfssl/wolfcrypt/sha512.h>

// Encryption ciphers
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/des3.h>
#include <wolfssl/wolfcrypt/chacha.h>
#include <wolfssl/wolfcrypt/chacha20_poly1305.h>
#include <wolfssl/wolfcrypt/poly1305.h>

#include <wolfssl/wolfcrypt/hmac.h>
#include <wolfssl/wolfcrypt/random.h>

#include <wolfssl/wolfcrypt/misc.h>

#include <stdbool.h>

# define SHA_DIGEST_LENGTH       WC_SHA_DIGEST_SIZE
# define SHA224_DIGEST_LENGTH    WC_SHA224_DIGEST_SIZE
# define SHA256_DIGEST_LENGTH    WC_SHA256_DIGEST_SIZE
# define SHA384_DIGEST_LENGTH    WC_SHA384_DIGEST_SIZE
# define SHA512_DIGEST_LENGTH    WC_SHA512_DIGEST_SIZE

#define NOT_IMPLEMENTED -0x666

/** Cipher should encrypt */
#define OPENVPN_OP_ENCRYPT      1

/** Cipher should decrypt */
#define OPENVPN_OP_DECRYPT      0

#ifdef HAVE_AESGCM
#define AESGCM_IV_SZ GCM_NONCE_MID_SZ
#endif

/** Generic cipher key type %context. */
typedef enum
{
    /* DO NOT CHANGE ORDER OF ELEMENTS */
#ifdef HAVE_AES_CBC
    OV_WC_AES_128_CBC_TYPE,
    OV_WC_AES_192_CBC_TYPE,
    OV_WC_AES_256_CBC_TYPE,
#endif
#ifdef WOLFSSL_AES_COUNTER
    OV_WC_AES_128_CTR_TYPE,
    OV_WC_AES_192_CTR_TYPE,
    OV_WC_AES_256_CTR_TYPE,
#endif
#ifdef HAVE_AES_ECB
    OV_WC_AES_128_ECB_TYPE,
    OV_WC_AES_192_ECB_TYPE,
    OV_WC_AES_256_ECB_TYPE,
#endif
#ifdef WOLFSSL_AES_DIRECT
    OV_WC_AES_128_OFB_TYPE,
    OV_WC_AES_192_OFB_TYPE,
    OV_WC_AES_256_OFB_TYPE,
#endif
#ifdef WOLFSSL_AES_CFB
    OV_WC_AES_128_CFB_TYPE,
    OV_WC_AES_192_CFB_TYPE,
    OV_WC_AES_256_CFB_TYPE,
#endif
#ifdef HAVE_AESGCM
    OV_WC_AES_128_GCM_TYPE,
    OV_WC_AES_192_GCM_TYPE,
    OV_WC_AES_256_GCM_TYPE,
#endif
#ifndef NO_DES3
    OV_WC_DES_CBC_TYPE,
    OV_WC_DES_ECB_TYPE,
    OV_WC_DES_EDE3_CBC_TYPE,
    OV_WC_DES_EDE3_ECB_TYPE,
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
    OV_WC_CHACHA20_POLY1305_TYPE,
#endif
    /* LEAVE NULL CIPHER AS LAST ELEMENT */
    OV_WC_NULL_CIPHER_TYPE,
} cipher_kt_t;

/* Make sure the order is the same as in cipher_kt_t */
const static cipher_kt_t cipher_static[] =
{
#ifdef HAVE_AES_CBC
        OV_WC_AES_128_CBC_TYPE, OV_WC_AES_192_CBC_TYPE, OV_WC_AES_256_CBC_TYPE,
#endif
#ifdef WOLFSSL_AES_COUNTER
        OV_WC_AES_128_CTR_TYPE, OV_WC_AES_192_CTR_TYPE, OV_WC_AES_256_CTR_TYPE,
#endif
#ifdef HAVE_AES_ECB
        OV_WC_AES_128_ECB_TYPE, OV_WC_AES_192_ECB_TYPE, OV_WC_AES_256_ECB_TYPE,
#endif
#ifdef WOLFSSL_AES_DIRECT
        OV_WC_AES_128_OFB_TYPE, OV_WC_AES_192_OFB_TYPE, OV_WC_AES_256_OFB_TYPE,
#endif
#ifdef WOLFSSL_AES_CFB
        OV_WC_AES_128_CFB_TYPE, OV_WC_AES_192_CFB_TYPE, OV_WC_AES_256_CFB_TYPE,
#endif
#ifdef HAVE_AESGCM
        OV_WC_AES_128_GCM_TYPE, OV_WC_AES_192_GCM_TYPE, OV_WC_AES_256_GCM_TYPE,
#endif
#ifndef NO_DES3
        OV_WC_DES_CBC_TYPE, OV_WC_DES_ECB_TYPE, OV_WC_DES_EDE3_CBC_TYPE,
        OV_WC_DES_EDE3_ECB_TYPE,
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
        OV_WC_CHACHA20_POLY1305_TYPE,
#endif
        OV_WC_NULL_CIPHER_TYPE, };

const static struct cipher
{
    cipher_kt_t type;
    const char *name;
} cipher_tbl[] =
{
/* Make sure the order is the same as in cipher_kt_t */
#ifdef HAVE_AES_CBC
        { OV_WC_AES_128_CBC_TYPE, "AES-128-CBC" },
        { OV_WC_AES_192_CBC_TYPE, "AES-192-CBC" },
        { OV_WC_AES_256_CBC_TYPE, "AES-256-CBC" },
#endif
#ifdef WOLFSSL_AES_COUNTER
        { OV_WC_AES_128_CTR_TYPE, "AES-128-CTR" },
        { OV_WC_AES_192_CTR_TYPE, "AES-192-CTR" },
        { OV_WC_AES_256_CTR_TYPE, "AES-256-CTR" },
#endif
#ifdef HAVE_AES_ECB
        { OV_WC_AES_128_ECB_TYPE, "AES-128-ECB" },
        { OV_WC_AES_192_ECB_TYPE, "AES-192-ECB" },
        { OV_WC_AES_256_ECB_TYPE, "AES-256-ECB" },
#endif
#ifdef WOLFSSL_AES_DIRECT
        { OV_WC_AES_128_OFB_TYPE, "AES-128-OFB" },
        { OV_WC_AES_192_OFB_TYPE, "AES-192-OFB" },
        { OV_WC_AES_256_OFB_TYPE, "AES-256-OFB" },
#endif
#ifdef WOLFSSL_AES_CFB
        { OV_WC_AES_128_CFB_TYPE, "AES-128-CFB" },
        { OV_WC_AES_192_CFB_TYPE, "AES-192-CFB" },
        { OV_WC_AES_256_CFB_TYPE, "AES-256-CFB" },
#endif
#ifdef HAVE_AESGCM
        { OV_WC_AES_128_GCM_TYPE, "AES-128-GCM" },
        { OV_WC_AES_192_GCM_TYPE, "AES-192-GCM" },
        { OV_WC_AES_256_GCM_TYPE, "AES-256-GCM" },
#endif
#ifndef NO_DES3
        { OV_WC_DES_CBC_TYPE, "DES-CBC" },
        { OV_WC_DES_ECB_TYPE, "DES-ECB" },
        { OV_WC_DES_EDE3_CBC_TYPE, "DES-EDE3-CBC" },
        { OV_WC_DES_EDE3_ECB_TYPE, "DES-EDE3-ECB" },
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
        { OV_WC_CHACHA20_POLY1305_TYPE, "CHACHA20-POLY1305" },
#endif
        { 0, NULL } };

/** Generic cipher %context. */
typedef struct
{
    union
    {
#ifndef NO_AES
        Aes aes;
#endif
#ifndef NO_DES3
        Des des;
        Des3 des3;
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
        uint8_t chacha20_poly1305_key[CHACHA20_POLY1305_AEAD_KEYSIZE];
#endif
    } cipher;

    union
    {
#ifndef NO_AES
        uint8_t aes_128[AES_128_KEY_SIZE];
        uint8_t aes_192[AES_192_KEY_SIZE];
        uint8_t aes_256[AES_256_KEY_SIZE];
#endif
#ifndef NO_DES3
        uint8_t des[DES_KEY_SIZE];
        uint8_t des3[DES3_KEY_SIZE];
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
        uint8_t chacha20_poly1305_key[CHACHA20_POLY1305_AEAD_KEYSIZE];
#endif
    } key;

    cipher_kt_t cipher_type;

    enum
    {
        OV_WC_DECRYPT = OPENVPN_OP_DECRYPT, OV_WC_ENCRYPT = OPENVPN_OP_ENCRYPT,
    } enc;

    union
    {
#ifndef NO_AES
        uint8_t aes[AES_BLOCK_SIZE];
#endif
#ifndef NO_DES3
        uint8_t des[DES_BLOCK_SIZE];
#endif
        uint8_t null_cipher[0];
    } buf;
    int buf_used;

    union
    {
#ifndef NO_AES
        uint8_t aes[AES_BLOCK_SIZE];
#endif
#ifndef NO_DES3
        uint8_t des[DES_BLOCK_SIZE];
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
        uint8_t chacha20_poly1305[CHACHA20_POLY1305_AEAD_IV_SIZE];
#endif
    } iv;
#define HAVE_AEAD_CIPHER_MODES

#ifdef HAVE_AEAD_CIPHER_MODES
bool aead_updated;
uint8_t aead_tag[OPENVPN_AEAD_TAG_LENGTH];

uint8_t* authIn;
int authInSz;

uint8_t* aead_buf;
int aead_buf_len;
#endif
} cipher_ctx_t;

/** Generic message digest key type %context. */
typedef enum
{
/* DO NOT CHANGE ORDER OF ELEMENTS */
#ifndef NO_MD4
OV_WC_MD4,
#endif
#ifndef NO_MD5
OV_WC_MD5,
#endif
#ifndef NO_SHA
OV_WC_SHA,
#endif
#ifdef WOLFSSL_SHA224
OV_WC_SHA224,
#endif
#ifndef NO_SHA256
OV_WC_SHA256,
#endif
#ifdef WOLFSSL_SHA384
OV_WC_SHA384,
#endif
#ifdef WOLFSSL_SHA512
OV_WC_SHA512,
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_224)
OV_WC_SHA3_224,
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_256)
OV_WC_SHA3_256,
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_384)
OV_WC_SHA3_384,
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_512)
OV_WC_SHA3_512,
#endif
/* LEAVE NULL DIGEST AS LAST ELEMENT */
OV_WC_NULL_DIGEST,
} md_kt_t;

static const enum wc_HashType OV_to_WC_hash_type[] =
{
/* Make sure the order is the same as in md_kt_t */
#ifndef NO_MD4
    WC_HASH_TYPE_MD4,
#endif
#ifndef NO_MD5
    WC_HASH_TYPE_MD5,
#endif
#ifndef NO_SHA
    WC_HASH_TYPE_SHA,
#endif
#ifdef WOLFSSL_SHA224
    WC_HASH_TYPE_SHA224,
#endif
#ifndef NO_SHA256
    WC_HASH_TYPE_SHA256,
#endif
#ifdef WOLFSSL_SHA384
    WC_HASH_TYPE_SHA384,
#endif
#ifdef WOLFSSL_SHA512
    WC_HASH_TYPE_SHA512,
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_224)
    WC_HASH_TYPE_SHA3_224,
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_256)
    WC_HASH_TYPE_SHA3_256,
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_384)
    WC_HASH_TYPE_SHA3_384,
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_512)
    WC_HASH_TYPE_SHA3_512,
#endif
    };

static const md_kt_t digest_static[] =
{
/* Make sure the order is the same as in md_kt_t */
#ifndef NO_MD4
    OV_WC_MD4,
#endif
#ifndef NO_MD5
    OV_WC_MD5,
#endif
#ifndef NO_SHA
    OV_WC_SHA,
#endif
#ifdef WOLFSSL_SHA224
    OV_WC_SHA224,
#endif
#ifndef NO_SHA256
    OV_WC_SHA256,
#endif
#ifdef WOLFSSL_SHA384
    OV_WC_SHA384,
#endif
#ifdef WOLFSSL_SHA512
    OV_WC_SHA512,
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_224)
    OV_WC_SHA3_224,
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_256)
    OV_WC_SHA3_256,
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_384)
    OV_WC_SHA3_384,
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_512)
    OV_WC_SHA3_512,
#endif
    };

static const struct digest
{
md_kt_t type;
const char *name;
} digest_tbl[] =
{
/* Make sure the order is the same as in md_kt_t */
#ifndef NO_MD4
    { OV_WC_MD4, "MD4" },
#endif
#ifndef NO_MD5
    { OV_WC_MD5, "MD5" },
#endif
#ifndef NO_SHA
    { OV_WC_SHA, "SHA1" },
#endif
#ifdef WOLFSSL_SHA224
    { OV_WC_SHA224, "SHA224" },
#endif
#ifndef NO_SHA256
    { OV_WC_SHA256, "SHA256" },
#endif
#ifdef WOLFSSL_SHA384
    { OV_WC_SHA384, "SHA384" },
#endif
#ifdef WOLFSSL_SHA512
    { OV_WC_SHA512, "SHA512" },
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_224)
    { OV_WC_SHA3_224, "SHA3-224" },
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_256)
    { OV_WC_SHA3_256, "SHA3-256" },
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_384)
    { OV_WC_SHA3_384, "SHA3-384" },
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_512)
    { OV_WC_SHA3_512, "SHA3-512" },
#endif
    { 0, NULL } };

/** Generic message digest %context. */
typedef struct
{
wc_HashAlg hash;
md_kt_t hash_type;
} md_ctx_t;

/** Generic HMAC %context. */
typedef struct
{
Hmac hmac;
union
{
#ifndef NO_MD4
    uint8_t md4[MD4_DIGEST_SIZE];
#endif
#ifndef NO_MD5
    uint8_t md5[WC_MD5_DIGEST_SIZE];
#endif
#ifndef NO_SHA
    uint8_t sha[WC_SHA_DIGEST_SIZE];
#endif
#ifdef WOLFSSL_SHA224
    uint8_t sha224[WC_SHA224_DIGEST_SIZE];
#endif
#ifndef NO_SHA256
    uint8_t sha256[WC_SHA256_DIGEST_SIZE];
#endif
#ifdef WOLFSSL_SHA384
    uint8_t sha384[WC_SHA384_DIGEST_SIZE];
#endif
#ifdef WOLFSSL_SHA512
    uint8_t sha512[WC_SHA512_DIGEST_SIZE];
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_224)
    uint8_t sha3_224[WC_SHA3_224_DIGEST_SIZE];
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_256)
    uint8_t sha3_256[WC_SHA3_256_DIGEST_SIZE];
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_384)
    uint8_t sha3_384[WC_SHA3_384_DIGEST_SIZE];
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_512)
    uint8_t sha3_512[WC_SHA3_512_DIGEST_SIZE];
#endif
} key;
int key_len;
} hmac_ctx_t;

/** Maximum length of an IV */
#define OPENVPN_MAX_IV_LENGTH   16

typedef enum
{
OPENVPN_MODE_CBC, OPENVPN_MODE_CFB, OPENVPN_MODE_OFB, // this needs to be implemented using CBC with a stream of 0's
OPENVPN_MODE_GCM,
OPENVPN_MODE_OTHER,
} cipher_modes;

#define DES_KEY_LENGTH          DES_KEY_SIZE
#define MD4_DIGEST_LENGTH       MD4_DIGEST_SIZE
#ifndef MD5_DIGEST_LENGTH
#define MD5_DIGEST_LENGTH       WC_MD5_DIGEST_SIZE
#endif

/* Set if variable length cipher */
#define EVP_CIPH_VARIABLE_LENGTH 0x8

static inline bool cipher_kt_var_key_size(const cipher_kt_t *cipher)
{
return false;
}

#define CIPHER_LIST_SIZE 1000

/* Use a dummy type for the provider */
typedef void provider_t;

#endif /* CRYPTO_WOLFSSL_H_ */
