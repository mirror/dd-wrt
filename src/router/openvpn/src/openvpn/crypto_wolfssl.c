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

#ifdef HAVE_CONFIG_H
#include "config.h"
#elif defined(_MSC_VER)
#include "config-msvc.h"
#endif

#include "syshead.h"

#if defined(ENABLE_CRYPTO_WOLFSSL)

#include "basic.h"
#include "buffer.h"
#include "integer.h"
#include "crypto.h"
#include "crypto_backend.h"

/*
 *
 * Functions related to the core crypto library
 *
 */

void crypto_init_lib(void)
{
    int ret;

    if ((ret = wolfCrypt_Init()) != 0)
    {
        msg(D_CRYPT_ERRORS, "wolfCrypt_Init failed");
    }
}

void crypto_uninit_lib(void)
{
    int ret;
    if ((ret = wolfCrypt_Cleanup()) != 0)
    {
        msg(D_CRYPT_ERRORS, "wolfCrypt_Cleanup failed");
    }
}

void crypto_clear_error(void)
{
}

void crypto_init_lib_engine(const char *engine_name)
{
    msg(M_INFO, "Note: wolfSSL does not have an engine");
}
#if 0
void
print_cipher(const cipher_kt_t *cipher)
{
    const char *var_key_size = cipher_kt_var_key_size(cipher) ?
                               " by default" : "";

    printf("%s  (%d bit key%s, ",
           cipher_kt_name(cipher),
           cipher_kt_key_size(cipher) * 8, var_key_size);

    if (cipher_kt_block_size(cipher) == 1)
    {
        printf("stream cipher");
    }
    else
    {
        printf("%d bit block", cipher_kt_block_size(cipher) * 8);
    }

    if (!cipher_kt_mode_cbc(cipher))
    {
        printf(", TLS client/server mode only");
    }

    printf(")\n");
}
#endif

int
memcmp_constant_time(const void *a, const void *b, size_t size)
{
    const uint8_t *a1 = a;
    const uint8_t *b1 = b;
    int ret = 0;
    size_t i;

    for (i = 0; i < size; i++) {
        ret |= *a1++ ^ *b1++;
    }

    return ret;
}

void show_available_ciphers(void)
{
    cipher_kt_t cipher;
    for (cipher = 0; cipher < OV_WC_NULL_CIPHER_TYPE; cipher++)
    {
        if (cipher_kt_mode(&cipher) != OPENVPN_MODE_OTHER)
        { /* Hide other cipher types */
            print_cipher(&cipher);
        }
    }
}

void show_available_digests(void)
{
#ifndef NO_MD4
    printf("MD4 %d bit digest size\n", wc_HashGetDigestSize(WC_HASH_TYPE_MD4));
#endif
#ifndef NO_MD5
    printf("MD5 %d bit digest size\n", wc_HashGetDigestSize(WC_HASH_TYPE_MD5));
#endif
#ifndef NO_SHA
    printf("SHA1 %d bit digest size\n", wc_HashGetDigestSize(WC_HASH_TYPE_SHA));
#endif
#ifdef WOLFSSL_SHA224
        printf("SHA224 %d bit digest size\n", wc_HashGetDigestSize(WC_HASH_TYPE_SHA224));
    #endif
#ifndef NO_SHA256
    printf("SHA256 %d bit digest size\n",
            wc_HashGetDigestSize(WC_HASH_TYPE_SHA256));
#endif
#ifdef WOLFSSL_SHA384
        printf("SHA384 %d bit digest size\n", wc_HashGetDigestSize(WC_HASH_TYPE_SHA384));
    #endif
#ifdef WOLFSSL_SHA512
        printf("SHA512 %d bit digest size\n", wc_HashGetDigestSize(WC_HASH_TYPE_SHA512));
    #endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_224)
        printf("SHA3-224 %d bit digest size\n", wc_HashGetDigestSize(WC_HASH_TYPE_SHA3_224));
    #endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_256)
        printf("SHA3-256 %d bit digest size\n", wc_HashGetDigestSize(WC_HASH_TYPE_SHA3_256));
    #endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_384)
        printf("SHA3-384 %d bit digest size\n", wc_HashGetDigestSize(WC_HASH_TYPE_SHA3_384));
    #endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_512)
        printf("SHA3-512 %d bit digest size\n", wc_HashGetDigestSize(WC_HASH_TYPE_SHA3_512));
    #endif
}

void show_available_engines(void)
{
    msg(M_INFO, "Note: wolfSSL does not have an engine");
}

const cipher_name_pair cipher_name_translation_table[] =
{ };
const size_t cipher_name_translation_table_count =
        sizeof(cipher_name_translation_table)
                / sizeof(*cipher_name_translation_table);

#define PEM_BEGIN              "-----BEGIN "
#define PEM_BEGIN_LEN          11
#define PEM_LINE_END           "-----\n"
#define PEM_LINE_END_LEN       6
#define PEM_END                "-----END "
#define PEM_END_LEN            9

/*
 * This function calculates the length of the resulting base64 encoded string
 */
//static const int PEM_LINE_SZ = 64;
static uint32_t der_to_pem_len(uint32_t der_len)
{
    uint32_t pem_len;
    pem_len = (der_len + 2) / 3 * 4;
    pem_len += (pem_len + PEM_LINE_SZ - 1) / PEM_LINE_SZ; /* new lines */
    return pem_len;
}

bool crypto_pem_encode(const char *name, struct buffer *dst,
        const struct buffer *src, struct gc_arena *gc)
{
    uint8_t* pem_buf = NULL;
    uint32_t pem_len = der_to_pem_len(BLEN(src));
    uint8_t* out_buf = NULL;
    uint8_t* out_buf_ptr;
    bool ret = false;
    int err;
    int name_len = strlen(name);
    int out_len = PEM_BEGIN_LEN + PEM_LINE_END_LEN + name_len + pem_len +
    PEM_END_LEN + PEM_LINE_END_LEN + name_len;

    if (!(pem_buf = (uint8_t*) malloc(pem_len)))
    {
        return false;
    }

    if (!(out_buf = (uint8_t*) malloc(out_len)))
    {
        goto cleanup;
    }

    if ((err = Base64_Encode(BPTR(src), BLEN(src), pem_buf, &pem_len)) != 0)
    {
        msg(M_INFO, "Base64_Encode failed with Errno: %d", err);
        goto cleanup;
    }

    out_buf_ptr = out_buf;
    memcpy(out_buf_ptr, PEM_BEGIN, PEM_BEGIN_LEN);
    out_buf_ptr += PEM_BEGIN_LEN;
    memcpy(out_buf_ptr, name, name_len);
    out_buf_ptr += name_len;
    memcpy(out_buf_ptr, PEM_LINE_END, PEM_LINE_END_LEN);
    out_buf_ptr += PEM_LINE_END_LEN;
    memcpy(out_buf_ptr, pem_buf, pem_len);
    out_buf_ptr += pem_len;
    memcpy(out_buf_ptr, PEM_END, PEM_END_LEN);
    out_buf_ptr += PEM_END_LEN;
    memcpy(out_buf_ptr, name, name_len);
    out_buf_ptr += name_len;
    memcpy(out_buf_ptr, PEM_LINE_END, PEM_LINE_END_LEN);

    *dst = alloc_buf_gc(out_len + 1, gc);
    ASSERT(buf_write(dst, out_buf, out_len));
    buf_null_terminate(dst);

    ret = true;

    cleanup: if (out_buf)
    {
        free(out_buf);
    }
    if (pem_buf)
    {
        free(pem_buf);
    }

    return ret;
}

/*
 * This function calculates the length of the string decoded from base64
 */
static uint32_t pem_to_der_len(uint32_t pem_len)
{
    int plainSz = pem_len - ((pem_len + (PEM_LINE_SZ - 1)) / PEM_LINE_SZ);
    return (plainSz * 3 + 3) / 4;
}

bool crypto_pem_decode(const char *name, struct buffer *dst,
        const struct buffer *src)
{
    int name_len = strlen(name);
    int err;
    uint8_t* src_buf;
    bool ret = false;
    unsigned int der_len = BLEN(src) - PEM_BEGIN_LEN - PEM_LINE_END_LEN -
    PEM_END_LEN - PEM_LINE_END_LEN - name_len - name_len - 1;
    unsigned int pem_len = pem_to_der_len(der_len);

    ASSERT(
            BLEN(src) > PEM_BEGIN_LEN + PEM_LINE_END_LEN + PEM_END_LEN + PEM_LINE_END_LEN);

    if (!(src_buf = (uint8_t*) malloc(BLEN(src))))
    {
        msg(M_FATAL, "Cannot allocate memory for PEM decode");
        return false;
    }
    memcpy(src_buf, BPTR(src), BLEN(src));

    src_buf[PEM_BEGIN_LEN + name_len] = '\0';

    if (strcmp((char*) (src_buf + PEM_BEGIN_LEN), name))
    {
        msg(D_CRYPT_ERRORS, "%s: unexpected PEM name (got '%s', expected '%s')",
                __func__, src_buf + PEM_BEGIN_LEN, name);
        goto cleanup;
    }

    if ((err = Base64_Decode(
    BPTR(src) + PEM_BEGIN_LEN + PEM_LINE_END_LEN + name_len, der_len,
            src_buf, &pem_len)) != 0)
    {
        msg(M_INFO, "Base64_Decode failed with Errno: %d", err);
        goto cleanup;
    }

    uint8_t *dst_data = buf_write_alloc(dst, pem_len);
    if (!dst_data)
    {
        msg(D_CRYPT_ERRORS, "%s: dst too small (%i, needs %i)", __func__,
                BCAP(dst), pem_len);
        goto cleanup;
    }

    memcpy(dst_data, src_buf, pem_len);

    ret = true;

    cleanup: free(src_buf);
    return ret;
}

/*
 * Generate strong cryptographic random numbers
 */
int rand_bytes(uint8_t *output, int len)
{
    static WC_RNG rng;
    static bool rng_init = false;
    int ret;

    if (!rng_init)
    {
        if ((ret = wc_InitRng(&rng)) != 0)
        {
            msg(D_CRYPT_ERRORS, "wc_InitRng failed Errno: %d", ret);
            return 0;
        }
        rng_init = true;
    }

    if ((ret = wc_RNG_GenerateBlock(&rng, output, len)) != 0)
    {
        msg(D_CRYPT_ERRORS, "wc_RNG_GenerateBlock failed Errno: %d", ret);
        return 0;
    }

    return 1;
}

/*
 *
 * Key functions, allow manipulation of keys.
 *
 */

int key_des_num_cblocks(const cipher_kt_t *kt)
{
    int ret = 0;

    if (kt)
    {
        switch (*kt)
        {
        case OV_WC_DES_CBC_TYPE:
        case OV_WC_DES_ECB_TYPE:
            ret = DES_KEY_SIZE / DES_BLOCK_SIZE;
            break;
        case OV_WC_DES_EDE3_CBC_TYPE:
        case OV_WC_DES_EDE3_ECB_TYPE:
            ret = DES3_KEY_SIZE / DES_BLOCK_SIZE;
            break;
        default:
            ret = 0;
        }
    }

    msg(D_CRYPTO_DEBUG, "CRYPTO INFO: n_DES_cblocks=%d", ret);
    return ret;
}

static const unsigned char odd_parity[256] =
{ 1, 1, 2, 2, 4, 4, 7, 7, 8, 8, 11, 11, 13, 13, 14, 14, 16, 16, 19, 19, 21, 21,
        22, 22, 25, 25, 26, 26, 28, 28, 31, 31, 32, 32, 35, 35, 37, 37, 38, 38,
        41, 41, 42, 42, 44, 44, 47, 47, 49, 49, 50, 50, 52, 52, 55, 55, 56, 56,
        59, 59, 61, 61, 62, 62, 64, 64, 67, 67, 69, 69, 70, 70, 73, 73, 74, 74,
        76, 76, 79, 79, 81, 81, 82, 82, 84, 84, 87, 87, 88, 88, 91, 91, 93, 93,
        94, 94, 97, 97, 98, 98, 100, 100, 103, 103, 104, 104, 107, 107, 109,
        109, 110, 110, 112, 112, 115, 115, 117, 117, 118, 118, 121, 121, 122,
        122, 124, 124, 127, 127, 128, 128, 131, 131, 133, 133, 134, 134, 137,
        137, 138, 138, 140, 140, 143, 143, 145, 145, 146, 146, 148, 148, 151,
        151, 152, 152, 155, 155, 157, 157, 158, 158, 161, 161, 162, 162, 164,
        164, 167, 167, 168, 168, 171, 171, 173, 173, 174, 174, 176, 176, 179,
        179, 181, 181, 182, 182, 185, 185, 186, 186, 188, 188, 191, 191, 193,
        193, 194, 194, 196, 196, 199, 199, 200, 200, 203, 203, 205, 205, 206,
        206, 208, 208, 211, 211, 213, 213, 214, 214, 217, 217, 218, 218, 220,
        220, 223, 223, 224, 224, 227, 227, 229, 229, 230, 230, 233, 233, 234,
        234, 236, 236, 239, 239, 241, 241, 242, 242, 244, 244, 247, 247, 248,
        248, 251, 251, 253, 253, 254, 254 };

static int DES_check_key_parity(const uint8_t *key)
{
    unsigned int i;

    for (i = 0; i < DES_BLOCK_SIZE; i++)
    {
        if (key[i] != odd_parity[key[i]])
            return 0;
    }
    return 1;
}

/* return true in fail case (1) */
static int DES_check(word32 mask, word32 mask2, uint8_t* key)
{
    word32 value[2];

    value[0] = mask;
    value[1] = mask2;
    return (memcmp(value, key, sizeof(value)) == 0) ? 1 : 0;
}

static inline uint32_t ByteReverseWord32(uint32_t value)
{
    /* 6 instructions with rotate instruction, 8 without */
    value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
    return value << 16U | value >> 16U;
}

/* check if not weak. Weak key list from Nist "Recommendation for the Triple
 * Data Encryption Algorithm (TDEA) Block Cipher"
 *
 * returns 1 if is weak 0 if not
 */
static int wolfSSL_DES_is_weak_key(uint8_t* key)
{
    word32 mask, mask2;

    mask = 0x01010101;
    mask2 = 0x01010101;
    if (DES_check(mask, mask2, key))
    {
        return 1;
    }

    mask = 0xFEFEFEFE;
    mask2 = 0xFEFEFEFE;
    if (DES_check(mask, mask2, key))
    {
        return 1;
    }

    mask = 0xE0E0E0E0;
    mask2 = 0xF1F1F1F1;
    if (DES_check(mask, mask2, key))
    {
        return 1;
    }

    mask = 0x1F1F1F1F;
    mask2 = 0x0E0E0E0E;
    if (DES_check(mask, mask2, key))
    {
        return 1;
    }

    /* semi-weak *key check (list from same Nist paper) */
    mask = 0x011F011F;
    mask2 = 0x010E010E;
    if (DES_check(mask, mask2, key)
            || DES_check(ByteReverseWord32(mask), ByteReverseWord32(mask2),
                    key))
    {
        return 1;
    }

    mask = 0x01E001E0;
    mask2 = 0x01F101F1;
    if (DES_check(mask, mask2, key)
            || DES_check(ByteReverseWord32(mask), ByteReverseWord32(mask2),
                    key))
    {
        return 1;
    }

    mask = 0x01FE01FE;
    mask2 = 0x01FE01FE;
    if (DES_check(mask, mask2, key)
            || DES_check(ByteReverseWord32(mask), ByteReverseWord32(mask2),
                    key))
    {
        return 1;
    }

    mask = 0x1FE01FE0;
    mask2 = 0x0EF10EF1;
    if (DES_check(mask, mask2, key)
            || DES_check(ByteReverseWord32(mask), ByteReverseWord32(mask2),
                    key))
    {
        return 1;
    }

    mask = 0x1FFE1FFE;
    mask2 = 0x0EFE0EFE;
    if (DES_check(mask, mask2, key)
            || DES_check(ByteReverseWord32(mask), ByteReverseWord32(mask2),
                    key))
    {
        return 1;
    }

    return 0;
}

bool key_des_check(uint8_t *key, int key_len, int ndc)
{
    int i;
    struct buffer b;

    buf_set_read(&b, key, key_len);

    for (i = 0; i < ndc; ++i)
    {
        uint8_t *dc = (uint8_t *) buf_read_alloc(&b, DES_KEY_SIZE);
        if (!dc)
        {
            msg(D_CRYPT_ERRORS,
                    "CRYPTO INFO: check_key_DES: insufficient key material");
            return false;
        }
        if (wolfSSL_DES_is_weak_key(dc))
        {
            msg(D_CRYPT_ERRORS,
                    "CRYPTO INFO: check_key_DES: weak key detected");
            return false;
        }
        if (!DES_check_key_parity(dc))
        {
            msg(D_CRYPT_ERRORS,
                    "CRYPTO INFO: check_key_DES: bad parity detected");
            return false;
        }
    }
    return true;

}

/* Sets the parity of the DES key for use */
static void wolfSSL_DES_set_odd_parity(uint8_t* myDes)
{
    int i;

    for (i = 0; i < DES_BLOCK_SIZE; i++)
    {
        myDes[i] = odd_parity[myDes[i]];
    }
}

void key_des_fixup(uint8_t *key, int key_len, int ndc)
{
    int i;
    struct buffer b;

    buf_set_read(&b, key, key_len);
    for (i = 0; i < ndc; ++i)
    {
        uint8_t *dc = (uint8_t *) buf_read_alloc(&b, DES_BLOCK_SIZE);
        if (!dc)
        {
            msg(D_CRYPT_ERRORS,
                    "CRYPTO INFO: fixup_key_DES: insufficient key material");
            return;
        }
        wolfSSL_DES_set_odd_parity(dc);
    }
}

void cipher_des_encrypt_ecb(const unsigned char key[DES_KEY_LENGTH],
        unsigned char src[DES_KEY_LENGTH], unsigned char dst[DES_KEY_LENGTH])
{
    Des myDes;

    if (src == NULL || dst == NULL || key == NULL)
    {
        msg(D_CRYPT_ERRORS, "Bad argument passed to cipher_des_encrypt_ecb");
    }

    wc_Des_SetKey(&myDes, key, NULL, DES_ENCRYPTION);
    wc_Des_EcbEncrypt(&myDes, dst, src, DES_KEY_LENGTH);
}

/*
 *
 * Generic cipher key type functions
 *
 */

const cipher_kt_t *cipher_kt_get(const char *ciphername)
{
    const struct cipher* cipher;

    for (cipher = cipher_tbl; cipher->name != NULL; cipher++)
    {
        if (strncmp(ciphername, cipher->name, strlen(cipher->name) + 1) == 0)
        {
            return &cipher_static[cipher->type];
        }
    }
    return NULL;
}

const char *cipher_kt_name(const cipher_kt_t *cipher_kt)
{
    if (!cipher_kt)
    {
        return "[null-digest]";
    }
    else
    {
        return cipher_tbl[*cipher_kt].name;
    }
}

int cipher_kt_key_size(const cipher_kt_t *cipher_kt)
{
    if (cipher_kt == NULL)
    {
        return 0;
    }
    switch (*cipher_kt)
    {
#ifdef HAVE_AES_CBC
    case OV_WC_AES_128_CBC_TYPE:
        return AES_128_KEY_SIZE;
    case OV_WC_AES_192_CBC_TYPE:
        return AES_192_KEY_SIZE;
    case OV_WC_AES_256_CBC_TYPE:
        return AES_256_KEY_SIZE;
#endif
#ifdef WOLFSSL_AES_COUNTER
    case OV_WC_AES_128_CTR_TYPE:
        return AES_128_KEY_SIZE;
    case OV_WC_AES_192_CTR_TYPE:
        return AES_192_KEY_SIZE;
    case OV_WC_AES_256_CTR_TYPE:
        return AES_256_KEY_SIZE;
#endif
#ifdef HAVE_AES_ECB
    case OV_WC_AES_128_ECB_TYPE:
        return AES_128_KEY_SIZE;
    case OV_WC_AES_192_ECB_TYPE:
        return AES_192_KEY_SIZE;
    case OV_WC_AES_256_ECB_TYPE:
        return AES_256_KEY_SIZE;
#endif
#ifdef WOLFSSL_AES_DIRECT
    case OV_WC_AES_128_OFB_TYPE:
        return AES_128_KEY_SIZE;
    case OV_WC_AES_192_OFB_TYPE:
        return AES_192_KEY_SIZE;
    case OV_WC_AES_256_OFB_TYPE:
        return AES_256_KEY_SIZE;
#endif
#ifdef WOLFSSL_AES_CFB
    case OV_WC_AES_128_CFB_TYPE:
        return AES_128_KEY_SIZE;
    case OV_WC_AES_192_CFB_TYPE:
        return AES_192_KEY_SIZE;
    case OV_WC_AES_256_CFB_TYPE:
        return AES_256_KEY_SIZE;
#endif
#ifdef HAVE_AESGCM
    case OV_WC_AES_128_GCM_TYPE:
        return AES_128_KEY_SIZE;
    case OV_WC_AES_192_GCM_TYPE:
        return AES_192_KEY_SIZE;
    case OV_WC_AES_256_GCM_TYPE:
        return AES_256_KEY_SIZE;
#endif
#ifndef NO_DES3
    case OV_WC_DES_CBC_TYPE:
    case OV_WC_DES_ECB_TYPE:
        return DES_KEY_SIZE;
    case OV_WC_DES_EDE3_CBC_TYPE:
    case OV_WC_DES_EDE3_ECB_TYPE:
        return DES3_KEY_SIZE;
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
    case OV_WC_CHACHA20_POLY1305_TYPE:
        return CHACHA20_POLY1305_AEAD_KEYSIZE;
#endif
    case OV_WC_NULL_CIPHER_TYPE:
        return 0;
    }
    return 0;
}

int cipher_kt_iv_size(const cipher_kt_t *cipher_kt)
{
    if (cipher_kt == NULL)
    {
        return 0;
    }
    switch (*cipher_kt)
    {
#ifdef HAVE_AES_CBC
    case OV_WC_AES_128_CBC_TYPE:
    case OV_WC_AES_192_CBC_TYPE:
    case OV_WC_AES_256_CBC_TYPE:
#endif
#ifdef WOLFSSL_AES_COUNTER
    case OV_WC_AES_128_CTR_TYPE:
    case OV_WC_AES_192_CTR_TYPE:
    case OV_WC_AES_256_CTR_TYPE:
#endif
#ifdef HAVE_AES_ECB
    case OV_WC_AES_128_ECB_TYPE:
    case OV_WC_AES_192_ECB_TYPE:
    case OV_WC_AES_256_ECB_TYPE:
#endif
#ifdef WOLFSSL_AES_DIRECT
    case OV_WC_AES_128_OFB_TYPE:
    case OV_WC_AES_192_OFB_TYPE:
    case OV_WC_AES_256_OFB_TYPE:
#endif
#ifdef WOLFSSL_AES_CFB
    case OV_WC_AES_128_CFB_TYPE:
    case OV_WC_AES_192_CFB_TYPE:
    case OV_WC_AES_256_CFB_TYPE:
#endif
#if defined(HAVE_AES_CBC) || defined(WOLFSSL_AES_COUNTER) || defined(HAVE_AES_ECB) || defined(WOLFSSL_AES_DIRECT) || defined(WOLFSSL_AES_CFB)
        return AES_IV_SIZE;
#endif
#ifdef HAVE_AESGCM
    case OV_WC_AES_128_GCM_TYPE:
    case OV_WC_AES_192_GCM_TYPE:
    case OV_WC_AES_256_GCM_TYPE:
        return AESGCM_IV_SZ;
#endif
#ifndef NO_DES3
    case OV_WC_DES_CBC_TYPE:
    case OV_WC_DES_ECB_TYPE:
    case OV_WC_DES_EDE3_CBC_TYPE:
    case OV_WC_DES_EDE3_ECB_TYPE:
        return DES_IV_SIZE;
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
    case OV_WC_CHACHA20_POLY1305_TYPE:
        return CHACHA20_POLY1305_AEAD_IV_SIZE;
#endif
    case OV_WC_NULL_CIPHER_TYPE:
        return 0;
    }
    return 0;
}

static bool needs_padding(const cipher_kt_t *cipher_kt)
{
    if (cipher_kt == NULL)
    {
        return false;
    }
    switch (*cipher_kt)
    {
#ifdef HAVE_AES_CBC
    case OV_WC_AES_128_CBC_TYPE:
    case OV_WC_AES_192_CBC_TYPE:
    case OV_WC_AES_256_CBC_TYPE:
#endif
#ifdef HAVE_AES_ECB
    case OV_WC_AES_128_ECB_TYPE:
    case OV_WC_AES_192_ECB_TYPE:
    case OV_WC_AES_256_ECB_TYPE:
#endif
#ifndef NO_DES3
    case OV_WC_DES_CBC_TYPE:
    case OV_WC_DES_ECB_TYPE:
    case OV_WC_DES_EDE3_CBC_TYPE:
    case OV_WC_DES_EDE3_ECB_TYPE:
#endif
#if defined(HAVE_AES_CBC) || defined(HAVE_AES_ECB) || !defined(NO_DES3)
        return true;
#endif
#ifdef WOLFSSL_AES_DIRECT
    case OV_WC_AES_128_OFB_TYPE:
    case OV_WC_AES_192_OFB_TYPE:
    case OV_WC_AES_256_OFB_TYPE:
#endif
#ifdef WOLFSSL_AES_CFB
    case OV_WC_AES_128_CFB_TYPE:
    case OV_WC_AES_192_CFB_TYPE:
    case OV_WC_AES_256_CFB_TYPE:
#endif
#ifdef WOLFSSL_AES_COUNTER
    case OV_WC_AES_128_CTR_TYPE:
    case OV_WC_AES_192_CTR_TYPE:
    case OV_WC_AES_256_CTR_TYPE:
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
    case OV_WC_CHACHA20_POLY1305_TYPE:
#endif
#ifdef HAVE_AESGCM
    case OV_WC_AES_128_GCM_TYPE:
    case OV_WC_AES_192_GCM_TYPE:
    case OV_WC_AES_256_GCM_TYPE:
#endif
#if defined(WOLFSSL_AES_DIRECT) || defined(WOLFSSL_AES_CFB) || defined(HAVE_AESGCM) || defined(WOLFSSL_AES_COUNTER) || (defined(HAVE_CHACHA) && defined(HAVE_POLY1305))
        return false;
#endif
    case OV_WC_NULL_CIPHER_TYPE:
    default:
        return false;
    }
}

int cipher_kt_block_size(const cipher_kt_t *cipher_kt)
{
    if (cipher_kt == NULL)
    {
        return 0;
    }
    if (!needs_padding(cipher_kt))
    {
        return 1;
    }

    switch (*cipher_kt)
    {
#ifdef HAVE_AES_CBC
    case OV_WC_AES_128_CBC_TYPE:
    case OV_WC_AES_192_CBC_TYPE:
    case OV_WC_AES_256_CBC_TYPE:
#endif
#ifdef HAVE_AES_ECB
    case OV_WC_AES_128_ECB_TYPE:
    case OV_WC_AES_192_ECB_TYPE:
    case OV_WC_AES_256_ECB_TYPE:
#endif
#ifdef HAVE_AESGCM
    case OV_WC_AES_128_GCM_TYPE:
    case OV_WC_AES_192_GCM_TYPE:
    case OV_WC_AES_256_GCM_TYPE:
#endif
#if defined(HAVE_AES_CBC) || defined(WOLFSSL_AES_COUNTER) || defined(HAVE_AES_ECB) || defined(WOLFSSL_AES_DIRECT) || defined(WOLFSSL_AES_CFB) || defined(HAVE_AESGCM)
        return AES_BLOCK_SIZE;
#endif
#ifndef NO_DES3
    case OV_WC_DES_CBC_TYPE:
    case OV_WC_DES_ECB_TYPE:
    case OV_WC_DES_EDE3_CBC_TYPE:
    case OV_WC_DES_EDE3_ECB_TYPE:
        return DES_BLOCK_SIZE;
#endif
    case OV_WC_NULL_CIPHER_TYPE:
    default:
        return 0;
    }
}

int cipher_kt_tag_size(const cipher_kt_t *cipher_kt)
{
    if (cipher_kt_mode_aead(cipher_kt))
    {
        return OPENVPN_AEAD_TAG_LENGTH;
    }
    else
    {
        return 0;
    }
}

bool cipher_kt_insecure(const cipher_kt_t *cipher)
{
    if (needs_padding(cipher))
    {
        return !(cipher_kt_block_size(cipher) >= 128 / 8);
    }
    else
    {
        /* For ciphers without padding check key size instead */
        return !(cipher_kt_key_size(cipher) >= 128 / 8);
    }
}

int cipher_kt_mode(const cipher_kt_t *cipher_kt)
{
    if (cipher_kt == NULL)
    {
        return 0;
    }
    switch (*cipher_kt)
    {
    /* Not all cases included since OpenVPN only recognizes CBC, OFB, CFB, and GCM */
#ifdef HAVE_AES_CBC
    case OV_WC_AES_128_CBC_TYPE:
    case OV_WC_AES_192_CBC_TYPE:
    case OV_WC_AES_256_CBC_TYPE:
#endif
#ifndef NO_DES3
    case OV_WC_DES_CBC_TYPE:
    case OV_WC_DES_EDE3_CBC_TYPE:
#endif
#if defined(HAVE_AES_CBC) || defined(NO_DES3)
        return OPENVPN_MODE_CBC;
#endif
#ifdef WOLFSSL_AES_DIRECT
    case OV_WC_AES_128_OFB_TYPE:
    case OV_WC_AES_192_OFB_TYPE:
    case OV_WC_AES_256_OFB_TYPE:
        return OPENVPN_MODE_OFB;
#endif
#ifdef WOLFSSL_AES_CFB
    case OV_WC_AES_128_CFB_TYPE:
    case OV_WC_AES_192_CFB_TYPE:
    case OV_WC_AES_256_CFB_TYPE:
        return OPENVPN_MODE_CFB;
#endif
#ifdef HAVE_AESGCM
    case OV_WC_AES_128_GCM_TYPE:
    case OV_WC_AES_192_GCM_TYPE:
    case OV_WC_AES_256_GCM_TYPE:
        return OPENVPN_MODE_GCM;
#endif
    case OV_WC_NULL_CIPHER_TYPE:
    default:
        return OPENVPN_MODE_OTHER;
    }
}

bool cipher_kt_mode_cbc(const cipher_kt_t *cipher)
{
    return cipher && cipher_kt_mode(cipher) == OPENVPN_MODE_CBC;
}

bool cipher_kt_mode_ofb_cfb(const cipher_kt_t *cipher)
{
    return cipher
            && (cipher_kt_mode(cipher) == OPENVPN_MODE_OFB
                    || cipher_kt_mode(cipher) == OPENVPN_MODE_CFB);
}

bool cipher_kt_mode_aead(const cipher_kt_t *cipher)
{
#ifdef HAVE_AEAD_CIPHER_MODES
    if (cipher)
    {
        switch (*cipher)
        {
        case OV_WC_AES_128_GCM_TYPE:
        case OV_WC_AES_192_GCM_TYPE:
        case OV_WC_AES_256_GCM_TYPE:
        case OV_WC_CHACHA20_POLY1305_TYPE:
            return true;
        default:
            return false;
        }
    }
#endif
    return false;
}

/*
 *
 * Generic cipher context functions
 *
 */

static void wc_cipher_init(cipher_ctx_t* ctx)
{
    ctx->cipher_type = OV_WC_NULL_CIPHER_TYPE;
    ctx->enc = -1;
    ctx->buf_used = 0;
#ifdef HAVE_AEAD_CIPHER_MODES
    ctx->aead_buf_len = 0;
    ctx->aead_buf = NULL;
    ctx->authInSz = 0;
    ctx->authIn = NULL;
    ctx->aead_updated = false;
#endif
}

cipher_ctx_t *cipher_ctx_new(void)
{
    cipher_ctx_t *ctx = (cipher_ctx_t*) malloc(sizeof *ctx);
    check_malloc_return(ctx);
    wc_cipher_init(ctx);
    return ctx;
}

void cipher_ctx_free(cipher_ctx_t *ctx)
{
    if (ctx)
    {
#ifdef HAVE_AEAD_CIPHER_MODES
        if (ctx->authIn)
        {
            free(ctx->authIn);
            ctx->authIn = NULL;
            ctx->authInSz = 0;
        }
        if (ctx->aead_buf)
        {
            free(ctx->aead_buf);
            ctx->aead_buf = NULL;
            ctx->aead_buf_len = 0;
        }
#endif
        free(ctx);
    }
}

static void check_key_length(const cipher_kt_t kt, int key_len)
{
    int correct_key_len;

    if (!kt)
    {
        return;
    }

    correct_key_len = cipher_kt_key_size(&kt);

    if (key_len != correct_key_len)
    {
        msg(M_FATAL, "Wrong key length for chosen cipher.\n"
                "Cipher chosen: %s\n"
                "Key length expected: %d\n"
                "Key length provided: %d\n", cipher_kt_name(&kt),
                correct_key_len, key_len);
    }
}

static void reset_aead(cipher_ctx_t *ctx)
{
#ifdef HAVE_AEAD_CIPHER_MODES
    ctx->aead_updated = false;
    memset(&ctx->aead_tag, 0, sizeof(ctx->aead_tag));
    if (ctx->authIn)
    {
        free(ctx->authIn);
        ctx->authIn = NULL;
        ctx->authInSz = 0;
    }
    if (ctx->aead_buf)
    {
        free(ctx->aead_buf);
        ctx->aead_buf = NULL;
        ctx->aead_buf_len = 0;
    }
#endif
}

/*
 * Function to setup context for cipher streams
 */
static int wolfssl_ctx_init(cipher_ctx_t *ctx, const uint8_t *key, int key_len,
        const uint8_t* iv, const cipher_kt_t *kt, int enc)
{
    int ret;

    switch (*kt)
    {
#ifdef HAVE_AES_CBC
    case OV_WC_AES_128_CBC_TYPE:
    case OV_WC_AES_192_CBC_TYPE:
    case OV_WC_AES_256_CBC_TYPE:
        if (key) {
            if ((ret = wc_AesSetKey(
                    &ctx->cipher.aes, key, key_len, iv,
                    enc == OPENVPN_OP_ENCRYPT ? AES_ENCRYPTION : AES_DECRYPTION
                )) != 0) {
                msg(M_FATAL, "wc_AesSetKey failed with Errno: %d", ret);
                return 0;
            }
        }
        if (iv && !key) {
            if ((ret = wc_AesSetIV(&ctx->cipher.aes, iv))) {
                msg(M_FATAL, "wc_AesSetIV failed with Errno: %d", ret);
                return 0;
            }
        }
        break;
#endif
#ifdef WOLFSSL_AES_CFB
    case OV_WC_AES_128_CFB_TYPE:
    case OV_WC_AES_192_CFB_TYPE:
    case OV_WC_AES_256_CFB_TYPE:
#endif
#ifdef WOLFSSL_AES_COUNTER
    case OV_WC_AES_128_CTR_TYPE:
    case OV_WC_AES_192_CTR_TYPE:
    case OV_WC_AES_256_CTR_TYPE:
#endif
#ifdef WOLFSSL_AES_DIRECT
    case OV_WC_AES_128_OFB_TYPE:
    case OV_WC_AES_192_OFB_TYPE:
    case OV_WC_AES_256_OFB_TYPE:
#endif
#ifdef HAVE_AES_ECB
    case OV_WC_AES_128_ECB_TYPE:
    case OV_WC_AES_192_ECB_TYPE:
    case OV_WC_AES_256_ECB_TYPE:
#endif
#if defined(WOLFSSL_AES_DIRECT) || defined(WOLFSSL_AES_COUNTER) || defined(WOLFSSL_AES_CFB) || defined(HAVE_AES_ECB)
        if (key) {
            if ((ret = wc_AesSetKeyDirect(
                    &ctx->cipher.aes, key, key_len, iv, AES_ENCRYPTION
                )) != 0) {
                msg(M_FATAL, "wc_AesSetKey failed with Errno: %d", ret);
                return 0;
            }
        }
        if (iv && !key) {
            if ((ret = wc_AesSetIV(&ctx->cipher.aes, iv))) {
                msg(M_FATAL, "wc_AesSetIV failed with Errno: %d", ret);
                return 0;
            }
        }
        break;
#endif
#ifdef HAVE_AESGCM
    case OV_WC_AES_128_GCM_TYPE:
    case OV_WC_AES_192_GCM_TYPE:
    case OV_WC_AES_256_GCM_TYPE:
        if (key) {
            if ((ret = wc_AesGcmSetKey(&ctx->cipher.aes, key, key_len)) != 0) {
                msg(M_FATAL, "wc_AesGcmSetKey failed with Errno: %d", ret);
            }
        }
        if (iv) {
            memcpy(ctx->iv.aes, iv, AESGCM_IV_SZ);
        }
        break;
#endif
#ifndef NO_DES3
    case OV_WC_DES_CBC_TYPE:
    case OV_WC_DES_ECB_TYPE:
        if (key)
        {
            if ((ret = wc_Des_SetKey(&ctx->cipher.des, key, iv,
                    enc == OPENVPN_OP_ENCRYPT ? DES_ENCRYPTION : DES_DECRYPTION))
                    != 0)
            {
                msg(M_FATAL, "wc_Des_SetKey failed with Errno: %d", ret);
                return 0;
            }
        }
        if (iv && !key)
        {
            wc_Des_SetIV(&ctx->cipher.des, iv);
        }
        break;
    case OV_WC_DES_EDE3_CBC_TYPE:
    case OV_WC_DES_EDE3_ECB_TYPE:
        if (key)
        {
            if ((ret = wc_Des3_SetKey(&ctx->cipher.des3, key, iv,
                    enc == OPENVPN_OP_ENCRYPT ? DES_ENCRYPTION : DES_DECRYPTION))
                    != 0)
            {
                msg(M_FATAL, "wc_Des3_SetKey failed with Errno: %d", ret);
                return 0;
            }
        }
        if (iv && !key)
        {
            if ((ret = wc_Des3_SetIV(&ctx->cipher.des3, iv)) != 0)
            {
                msg(M_FATAL, "wc_Des3_SetIV failed with Errno: %d", ret);
                return 0;
            }
        }
        break;
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
    case OV_WC_CHACHA20_POLY1305_TYPE:
        ASSERT(CHACHA20_POLY1305_AEAD_AUTHTAG_SIZE == OPENVPN_AEAD_TAG_LENGTH);
        if (iv) {
            memcpy(ctx->iv.chacha20_poly1305, iv, CHACHA20_POLY1305_AEAD_IV_SIZE);
        }
        break;
#endif
    case OV_WC_NULL_CIPHER_TYPE:
        return 0;
    }

    if (key)
    {
        memcpy(&ctx->key, key, key_len);
    }

    ctx->cipher_type = *kt;
    switch (enc)
    {
    case OPENVPN_OP_ENCRYPT:
        ctx->enc = OV_WC_ENCRYPT;
        break;
    case OPENVPN_OP_DECRYPT:
        ctx->enc = OV_WC_DECRYPT;
        break;
    }
    ctx->buf_used = 0;
#ifdef HAVE_AEAD_CIPHER_MODES
    ctx->aead_updated = false;
    if (ctx->aead_buf)
    {
        free(ctx->aead_buf);
        ctx->aead_buf = NULL;
        ctx->aead_buf_len = 0;
    }
#endif
    return 1;
}

void cipher_ctx_init(cipher_ctx_t *ctx, const uint8_t *key, int key_len,
        const cipher_kt_t *kt, int enc)
{
    int ret;
    ASSERT(NULL != kt && NULL != ctx && NULL != key);

    check_key_length(*kt, key_len);
    if ((ret = wolfssl_ctx_init(ctx, key, key_len, NULL, kt, enc)) != 1)
    {
        msg(M_FATAL, "wolfssl_ctx_init failed with Errno: %d", ret);
    }
}

/*
 * Reset and zero values in cipher context
 */
void cipher_ctx_cleanup(cipher_ctx_t *ctx)
{
    if (ctx)
    {
        ctx->cipher_type = OV_WC_NULL_CIPHER_TYPE;
        ctx->enc = -1;
        ctx->buf_used = 0;
        memset(&ctx->cipher, 0, sizeof(ctx->cipher));
        memset(&ctx->buf, 0, sizeof(ctx->buf));
        memset(&ctx->key, 0, sizeof(ctx->key));
        reset_aead(ctx);
    }
}

int cipher_ctx_iv_length(const cipher_ctx_t *ctx)
{
    return cipher_kt_iv_size(&ctx->cipher_type);
}

int cipher_ctx_get_tag(cipher_ctx_t *ctx, uint8_t *tag, int tag_len)
{
#ifdef HAVE_AEAD_CIPHER_MODES
    if (!ctx || !tag)
    {
        return 0;
    }
    ASSERT(tag_len == OPENVPN_AEAD_TAG_LENGTH);
    memcpy(tag, ctx->aead_tag, OPENVPN_AEAD_TAG_LENGTH);
    return 1;
#else
    msg(M_FATAL, "%s called without AEAD functionality compiled in.", __func__);
#endif
}

int cipher_ctx_block_size(const cipher_ctx_t *ctx)
{
    return cipher_kt_block_size(&ctx->cipher_type);
}

int cipher_ctx_mode(const cipher_ctx_t *ctx)
{
    return cipher_kt_mode(&ctx->cipher_type);
}

const cipher_kt_t *cipher_ctx_get_cipher_kt(const cipher_ctx_t *ctx)
{
    return ctx ? &ctx->cipher_type : NULL;
}

/*
 * Reset the cipher context to the initial settings used in cipher_ctx_init
 * and set a new IV
 */
int cipher_ctx_reset(cipher_ctx_t *ctx, const uint8_t *iv_buf)
{
    int ret;
    if ((ret = wolfssl_ctx_init(ctx, (uint8_t*) &ctx->key,
            cipher_kt_key_size(&ctx->cipher_type), iv_buf, &ctx->cipher_type,
            ctx->enc)) != 1)
    {
        msg(M_FATAL, "wolfssl_ctx_init failed with Errno: %d", ret);
    }
    return 1;
}

int cipher_ctx_update_ad(cipher_ctx_t *ctx, const uint8_t *src, int src_len)
{
#ifdef HAVE_AEAD_CIPHER_MODES
    if (!ctx || !src || src_len <= 0)
    {
        msg(M_FATAL, "Invalid parameter(s) for cipher_ctx_update_ad");
    }
    if (ctx->authIn)
    {
        free(ctx->authIn);
    }
    ctx->authIn = (uint8_t*) malloc(src_len);
    check_malloc_return(ctx->authIn);
    memcpy(ctx->authIn, src, src_len);
    ctx->authInSz = src_len;
#else
    msg(M_FATAL, "%s called without AEAD functionality compiiled in.", __func__);
#endif
    return 1;
}

/*
 * Update cipher blocks. The data stream in src has to be padded outside of
 * this function. Do not call this function directly, use wolfssl_ctx_update
 * instead.
 */
static int wolfssl_ctx_update_blocks(cipher_ctx_t *ctx, uint8_t *dst,
        int *dst_len, uint8_t *src, int src_len)
{
    int ret, i, j;
    if (needs_padding(&ctx->cipher_type))
    {
        /* make sure src is correctly padded */
        ASSERT((src_len % cipher_kt_block_size(&ctx->cipher_type)) == 0);
    }

    switch (ctx->cipher_type)
    {
#ifdef HAVE_AES_CBC
    case OV_WC_AES_128_CBC_TYPE:
    case OV_WC_AES_192_CBC_TYPE:
    case OV_WC_AES_256_CBC_TYPE:
        if (ctx->enc == OV_WC_ENCRYPT) {
            if ((ret = wc_AesCbcEncrypt(&ctx->cipher.aes, dst, src, src_len)) != 0) {
                msg(M_FATAL, "wc_AesCbcEncrypt failed with Errno: %d", ret);
                return 0;
            }
        } else {
            if ((ret = wc_AesCbcDecrypt(&ctx->cipher.aes, dst, src, src_len)) != 0) {
                msg(M_FATAL, "wc_AesCbcDecrypt failed with Errno: %d", ret);
                return 0;
            }
        }
        break;
#endif
#ifdef WOLFSSL_AES_COUNTER
    case OV_WC_AES_128_CTR_TYPE:
    case OV_WC_AES_192_CTR_TYPE:
    case OV_WC_AES_256_CTR_TYPE:
        /* encryption and decryption are the same for CTR */
        if ((ret = wc_AesCtrEncrypt(&ctx->cipher.aes, dst, src, src_len)) != 0) {
            msg(M_FATAL, "wc_AesCtrEncrypt failed with Errno: %d", ret);
            return 0;
        }
        break;
#endif
#ifdef HAVE_AES_ECB
    case OV_WC_AES_128_ECB_TYPE:
    case OV_WC_AES_192_ECB_TYPE:
    case OV_WC_AES_256_ECB_TYPE:
        if (ctx->enc == OV_WC_ENCRYPT) {
            if ((ret = wc_AesEcbEncrypt(&ctx->cipher.aes, dst, src, src_len)) != 0) {
                msg(M_FATAL, "wc_AesEcbEncrypt failed with Errno: %d", ret);
                return 0;
            }
        } else {
            if ((ret = wc_AesEcbDecrypt(&ctx->cipher.aes, dst, src, src_len)) != 0) {
                msg(M_FATAL, "wc_AesEcbDecrypt failed with Errno: %d", ret);
                return 0;
            }
        }
        break;
#endif
#ifdef WOLFSSL_AES_DIRECT
    case OV_WC_AES_128_OFB_TYPE:
    case OV_WC_AES_192_OFB_TYPE:
    case OV_WC_AES_256_OFB_TYPE:
        /* encryption and decryption are the same for OFB */
        for (i = 0; i < src_len; i += AES_BLOCK_SIZE) {
            wc_AesEncryptDirect(&ctx->cipher.aes, (uint8_t*)ctx->cipher.aes.reg,
                                                  (uint8_t*)ctx->cipher.aes.reg);
            for (j = i; j < MIN(i + AES_BLOCK_SIZE, src_len); j++) {
                dst[j] = ((uint8_t*)ctx->cipher.aes.reg)[j - i] ^ src[j];
            }
        }
        break;
#endif
#ifdef WOLFSSL_AES_CFB
    case OV_WC_AES_128_CFB_TYPE:
    case OV_WC_AES_192_CFB_TYPE:
    case OV_WC_AES_256_CFB_TYPE:
        if (ctx->enc == OV_WC_ENCRYPT) {
            if ((ret = wc_AesCfbEncrypt(&ctx->cipher.aes, dst, src, src_len)) != 0) {
                msg(M_FATAL, "wc_AesCfbEncrypt failed with Errno: %d", ret);
                return 0;
            }
        } else {
            if ((ret = wc_AesCfbDecrypt(&ctx->cipher.aes, dst, src, src_len)) != 0) {
                msg(M_FATAL, "wc_AesCfbDecrypt failed with Errno: %d", ret);
                return 0;
            }
        }
        break;
#endif
#ifdef HAVE_AESGCM
    case OV_WC_AES_128_GCM_TYPE:
    case OV_WC_AES_192_GCM_TYPE:
    case OV_WC_AES_256_GCM_TYPE:
        if (ctx->aead_updated) {
            msg(M_FATAL, "AEAD ALGORITHMS MAY ONLY CALL UPDATE ONCE");
        }
        if (ctx->enc == OV_WC_ENCRYPT) {
            if ((ret = wc_AesGcmEncrypt(&ctx->cipher.aes, dst, src, src_len,
                                        ctx->iv.aes, AESGCM_IV_SZ, ctx->aead_tag,
                                        OPENVPN_AEAD_TAG_LENGTH, ctx->authIn,
                                        ctx->authInSz)) != 0) {
                msg(M_FATAL, "wc_AesGcmEncrypt failed with Errno: %d", ret);
            }
        } else {
            /*
             * Decryption needs to be handled in Final call since wolfSSL also checks
             * that the auth tag is correct.
             */
            ASSERT(!ctx->aead_buf);
            ctx->aead_buf = (uint8_t*) malloc(src_len);
            check_malloc_return(ctx->aead_buf);
            memcpy(ctx->aead_buf, src, src_len);
            ctx->aead_buf_len = src_len;

            *dst_len -= src_len;
        }
        ctx->aead_updated = true;
        break;
#endif
#ifndef NO_DES3
    case OV_WC_DES_CBC_TYPE:
        if (ctx->enc == OV_WC_ENCRYPT)
        {
            if ((ret = wc_Des_CbcEncrypt(&ctx->cipher.des, dst, src, src_len))
                    != 0)
            {
                msg(M_FATAL, "wc_Des3_CbcEncrypt failed with Errno: %d", ret);
                return 0;
            }
        }
        else
        {
            if ((ret = wc_Des_CbcDecrypt(&ctx->cipher.des, dst, src, src_len))
                    != 0)
            {
                msg(M_FATAL, "wc_Des3_CbcDecrypt failed with Errno: %d", ret);
                return 0;
            }
        }
        break;
    case OV_WC_DES_EDE3_CBC_TYPE:
        if (ctx->enc == OV_WC_ENCRYPT)
        {
            if ((ret = wc_Des3_CbcEncrypt(&ctx->cipher.des3, dst, src, src_len))
                    != 0)
            {
                msg(M_FATAL, "wc_Des3_CbcEncrypt failed with Errno: %d", ret);
                return 0;
            }
        }
        else
        {
            if ((ret = wc_Des3_CbcDecrypt(&ctx->cipher.des3, dst, src, src_len))
                    != 0)
            {
                msg(M_FATAL, "wc_Des3_CbcDecrypt failed with Errno: %d", ret);
                return 0;
            }
        }
        break;
    case OV_WC_DES_ECB_TYPE:
        if ((ret = wc_Des_EcbEncrypt(&ctx->cipher.des, dst, src, src_len)) != 0)
        {
            msg(M_FATAL, "wc_Des_EcbEncrypt failed with Errno: %d", ret);
            return 0;
        }
        break;
    case OV_WC_DES_EDE3_ECB_TYPE:
        if ((ret = wc_Des3_EcbEncrypt(&ctx->cipher.des3, dst, src, src_len))
                != 0)
        {
            msg(M_FATAL, "wc_Des3_EcbEncrypt failed with Errno: %d", ret);
            return 0;
        }
        break;
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
    case OV_WC_CHACHA20_POLY1305_TYPE:
        if (ctx->aead_updated) {
            msg(M_FATAL, "AEAD ALGORITHMS MAY ONLY CALL UPDATE ONCE");
            return 0;
        }
        if (ctx->enc == OV_WC_ENCRYPT) {
            if ((ret = wc_ChaCha20Poly1305_Encrypt(ctx->key.chacha20_poly1305_key,
                                                   ctx->iv.chacha20_poly1305, ctx->authIn, ctx->authInSz,
                                                   src, src_len, dst, ctx->aead_tag)) != 0) {
                msg(M_FATAL, "wc_ChaCha20Poly1305_Encrypt failed with Errno: %d", ret);
                return 0;
            }
        } else {
            /*
             * Store for later since wc_ChaCha20Poly1305_Decrypt also takes in the
             * correct tag as a parameter and automatically checks it.
             */
            ASSERT(!ctx->aead_buf);
            ctx->aead_buf = (uint8_t*) malloc(src_len);
            check_malloc_return(ctx->aead_buf);
            memcpy(ctx->aead_buf, src, src_len);
            ctx->aead_buf_len = src_len;

            *dst_len -= src_len;
        }
        ctx->aead_updated = true;
        break;
#endif
    case OV_WC_NULL_CIPHER_TYPE:
        return 0;
    }
    *dst_len += src_len;
    return 1;
}

/*
 * This function wraps wolfssl_ctx_update_blocks by checking and storing input data
 * that is not properly padded. The stored data is later concatenated with new blocks
 * of data that are passed to this function.
 */
static int wolfssl_ctx_update(cipher_ctx_t *ctx, uint8_t *dst, int *dst_len,
        uint8_t *src, int src_len)
{
    int ret;
    int block_size = cipher_kt_block_size(&ctx->cipher_type);
    int block_leftover;

    if (!ctx || !src || (src_len < 0) || !dst_len || !dst)
        return 0;

    *dst_len = 0;

    if (!src_len)
    {
        /* nothing to do */
        return 1;
    }

    if (!needs_padding(&ctx->cipher_type))
    {
        /*
         * In case of AEAD and no padding needed send data straight to wolfssl_ctx_update_blocks
         * and don't process padding
         */
        if ((ret = wolfssl_ctx_update_blocks(ctx, dst, dst_len, src, src_len)
                != 1))
        {
            msg(M_FATAL, "%s: wolfssl_ctx_update_blocks() failed", __func__);
        }
        return 1;
    }

    if (ctx->buf_used)
    {
        if ((ctx->buf_used + src_len) < block_size)
        {
            memcpy((&ctx->buf) + ctx->buf_used, src, src_len);
            ctx->buf_used += src_len;
            return 1;
        }
        else
        {
            memcpy((&ctx->buf) + ctx->buf_used, src,
                    block_size - ctx->buf_used);
            src += block_size - ctx->buf_used;
            src_len -= block_size - ctx->buf_used;
            if ((ret = wolfssl_ctx_update_blocks(ctx, dst, dst_len,
                    (uint8_t*) &(ctx->buf), block_size) != 1))
            {
                msg(M_FATAL, "%s: wolfssl_ctx_update_blocks() failed",
                        __func__);
            }
            ctx->buf_used = 0;
            dst += block_size;
            *dst_len += block_size;
        }
    }

    ASSERT(ctx->buf_used == 0);

    if (src_len < block_size)
    {
        memcpy(&ctx->buf, src, src_len);
        ctx->buf_used = src_len;
        return 1;
    }

    block_leftover = src_len % block_size;
    if ((ret = wolfssl_ctx_update_blocks(ctx, dst, dst_len, src,
            src_len - block_leftover) != 1))
    {
        msg(M_FATAL, "%s: wolfssl_ctx_update_blocks() failed", __func__);
    }

    if (block_leftover)
    {
        memcpy(&ctx->buf, src + (src_len - block_leftover), block_leftover);
        ctx->buf_used = block_leftover;
    }
    else if (ctx->enc == OV_WC_DECRYPT)
    {
        /* copy last decrypted block to check padding in final call */
        memcpy(&ctx->buf, dst + (src_len - block_size), block_size);
    }

    return 1;
}

int cipher_ctx_update(cipher_ctx_t *ctx, uint8_t *dst, int *dst_len,
        uint8_t *src, int src_len)
{
    if (!wolfssl_ctx_update(ctx, dst, dst_len, src, src_len))
    {
        msg(M_FATAL, "%s: wolfssl_ctx_update() failed", __func__);
    }
    return 1;
}

/*
 * Pads the buffer of the cipher context with PKCS#7 padding
 */
static void pad_block(cipher_ctx_t *ctx)
{
    int i, block_size, n;
    uint8_t* buf = (uint8_t*) &ctx->buf;
    block_size = cipher_kt_block_size(&ctx->cipher_type);
    n = block_size - ctx->buf_used;
    ASSERT(block_size >= ctx->buf_used);
    ASSERT(n < 256); // nothing more can fit in a byte
    for (i = ctx->buf_used; i < block_size; i++)
    {
        buf[i] = (uint8_t) (n);
    }
}

/*
 * Verifies the PKCS#7 padding of the block in the cipher context and
 * returns the number of padding blocks.
 */
static int check_pad(cipher_ctx_t *ctx)
{
    int i;
    int n;
    int block_size = cipher_kt_block_size(&ctx->cipher_type);
    uint8_t* buf = (uint8_t*) &ctx->buf;
    n = buf[block_size - 1];
    if (n > block_size)
        return -1;
    for (i = 0; i < n; i++)
    {
        if (buf[block_size - i - 1] != n)
            return -1;
    }
    return n;
}

/*
 * Verify or add necessary padding and return final block. In case of decryption
 * the length returned in dst_len is negative so as to remove the final padding
 * blocks.
 */
static int wolfssl_ctx_final(cipher_ctx_t *ctx, uint8_t *dst, int *dst_len)
{
    int block_size;
    int pad_left;

    if (!ctx || !dst_len || !dst)
    {
        return 0;
    }

    *dst_len = 0;

    if (ctx->buf_used == 0 && ctx->enc != OV_WC_DECRYPT
            && !needs_padding(&ctx->cipher_type))
    {
        return 1;
    }

    block_size = cipher_kt_block_size(&ctx->cipher_type);

    if (!cipher_kt_mode_aead(&ctx->cipher_type))
    {
        if (ctx->enc == OV_WC_ENCRYPT)
        {
            if (needs_padding(&ctx->cipher_type))
            {
                pad_block(ctx);
            }
            if (wolfssl_ctx_update_blocks(ctx, dst, dst_len,
                    (uint8_t*) &ctx->buf, block_size) != 1)
            {
                return 0;
            }
        }
        else if (needs_padding(&ctx->cipher_type))
        {
            if (ctx->buf_used != 0)
            {
                *dst_len = 0;
                msg(M_FATAL, "%s: not enough padding for decrypt", __func__);
                return 0;
            }
            if ((pad_left = check_pad(ctx)) >= 0)
            {
                *dst_len = -pad_left;
            }
            else
            {
                msg(M_FATAL, "%s: padding is incorrect", __func__);
                return 0;
            }
        }
    }

    return 1;
}

int cipher_ctx_final(cipher_ctx_t *ctx, uint8_t *dst, int *dst_len)
{
    return wolfssl_ctx_final(ctx, dst, dst_len);
}

int cipher_ctx_final_check_tag(cipher_ctx_t *ctx, uint8_t *dst, int *dst_len,
        uint8_t *tag, size_t tag_len)
{
#ifdef HAVE_AEAD_CIPHER_MODES
    int ret;

    if (!ctx || !dst_len || !dst || !tag || tag_len <= 0)
    {
        return 0;
    }

    ASSERT(ctx->enc == OV_WC_DECRYPT);

    switch (ctx->cipher_type)
    {
#ifdef HAVE_AESGCM
    case OV_WC_AES_128_GCM_TYPE:
    case OV_WC_AES_192_GCM_TYPE:
    case OV_WC_AES_256_GCM_TYPE:
        if ((ret = wc_AesGcmDecrypt(&ctx->cipher.aes, dst, ctx->aead_buf, ctx->aead_buf_len,
                                    ctx->iv.aes, AESGCM_IV_SZ, tag, tag_len, ctx->authIn,
                                    ctx->authInSz)) != 0) {
            msg(M_FATAL, "wc_AesGcmDecrypt failed with Errno: %d", ret);
            return 0;
        }
        break;
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
    case OV_WC_CHACHA20_POLY1305_TYPE:
        if (tag_len != CHACHA20_POLY1305_AEAD_AUTHTAG_SIZE) {
            msg(M_FATAL, "Incorrect tag length for Chacha20_Poly1305. Got: %d ; Need: %d",
                (int)tag_len, CHACHA20_POLY1305_AEAD_AUTHTAG_SIZE);
            return 0;
        }
        if ((ret = wc_ChaCha20Poly1305_Decrypt(ctx->key.chacha20_poly1305_key, ctx->iv.chacha20_poly1305,
                                               ctx->authIn, ctx->authInSz, ctx->aead_buf, ctx->aead_buf_len,
                                               tag, dst)) != 0) {
            msg(M_FATAL, "wc_ChaCha20Poly1305_Decrypt failed with Errno: %d", ret);
            return 0;
        }
        break;
#endif
    default:
        msg(M_FATAL,
                "cipher_ctx_final_check_tag called with none AEAD cipher.");
        return 0;
    }
    *dst_len = ctx->aead_buf_len;
    return 1;
#else
    msg(M_FATAL, "%s called without AEAD functionality compiiled in.", __func__);
#endif
}

/*
 *
 * Generic message digest information functions
 *
 */

const md_kt_t *md_kt_get(const char *digest)
{
    const struct digest* digest_;

    for (digest_ = digest_tbl; digest_->name != NULL; digest_++)
    {
        if (strncmp(digest, digest_->name, strlen(digest_->name) + 1) == 0)
        {
#ifndef NO_MD4
            if (digest_->type == OV_WC_MD4)
            {
                msg(M_FATAL, "MD4 not supported in wolfssl generic functions.");
            }
#endif
            return &digest_static[digest_->type];
        }
    }
    return NULL;
}

const char *md_kt_name(const md_kt_t *kt)
{
    if (!kt)
    {
        return "[null-digest]";
    }
    else
    {
        return digest_tbl[*kt].name;
    }
}

unsigned char md_kt_size(const md_kt_t *kt)
{
    if (!kt || *kt >= OV_WC_NULL_DIGEST)
    {
        return 0;
    }
    return wc_HashGetDigestSize(OV_to_WC_hash_type[*kt]);
}

/*
 *
 * Generic message digest functions
 *
 */

int md_full(const md_kt_t *kt, const uint8_t *src, int src_len, uint8_t *dst)
{
    int ret;

    if (!kt || !src || !dst)
    {
        return 0;
    }

    if ((ret = wc_Hash(OV_to_WC_hash_type[*kt], src, src_len, dst, -1)) != 0)
    {
        msg(M_FATAL, "md_full failed with Errno: %d", ret);
        return 0;
    }
    return 1;
}

md_ctx_t *md_ctx_new(void)
{
    md_ctx_t *ctx = (md_ctx_t*) malloc(sizeof(md_ctx_t));
    check_malloc_return(ctx);
    return ctx;
}

void md_ctx_free(md_ctx_t *ctx)
{
    if (ctx)
    {
        free(ctx);
    }
}

void md_ctx_init(md_ctx_t *ctx, const md_kt_t *kt)
{
    ASSERT(NULL != ctx && NULL != kt);

    wc_HashInit(&ctx->hash, OV_to_WC_hash_type[*kt]);
    ctx->hash_type = *kt;
}

void md_ctx_cleanup(md_ctx_t *ctx)
{
    if (ctx)
    {
        wc_HashFree(&ctx->hash, OV_to_WC_hash_type[ctx->hash_type]);
    }
}

int md_ctx_size(const md_ctx_t *ctx)
{
    return md_kt_size(&ctx->hash_type);
}

void md_ctx_update(md_ctx_t *ctx, const uint8_t *src, int src_len)
{
    int ret;

    if ((ret = wc_HashUpdate(&ctx->hash, OV_to_WC_hash_type[ctx->hash_type],
            src, src_len)) != 0)
    {
        msg(M_FATAL, "wc_HashUpdate failed with Errno: %d", ret);
    }
}

void md_ctx_final(md_ctx_t *ctx, uint8_t *dst)
{
    int ret;

    if ((ret = wc_HashFinal(&ctx->hash, OV_to_WC_hash_type[ctx->hash_type], dst))
            != 0)
    {
        msg(M_FATAL, "wc_HashFinal failed with Errno: %d", ret);
    }
}

/*
 *
 * Generic HMAC functions
 *
 */

hmac_ctx_t *hmac_ctx_new(void)
{
    hmac_ctx_t *ctx = (hmac_ctx_t*) calloc(sizeof(hmac_ctx_t), 1);
    check_malloc_return(ctx);
    return ctx;
}

void hmac_ctx_free(hmac_ctx_t *ctx)
{
    if (ctx)
    {
        wc_HmacFree(&ctx->hmac);
    }
}

void hmac_ctx_init(hmac_ctx_t *ctx, const uint8_t *key, int key_length,
        const md_kt_t *kt)
{
    int ret;
    ASSERT(NULL != kt && NULL != ctx);

    if ((ret = wc_HmacSetKey(&ctx->hmac, OV_to_WC_hash_type[*kt], key,
            key_length)) != 0)
    {
        msg(M_FATAL, "wc_HmacSetKey failed. Errno: %d", ret);
    }

    /* Hold key for later reseting of hmac instance */
    memcpy(&ctx->key, key, key_length);
    ctx->key_len = key_length;

    /* make sure we used a big enough key */
    ASSERT(md_kt_size(kt) <= key_length);
}

void hmac_ctx_cleanup(hmac_ctx_t *ctx)
{
    hmac_ctx_free(ctx);
}

int hmac_ctx_size(const hmac_ctx_t *ctx)
{
    if (!ctx)
    {
        return 0;
    }
    return wc_HashGetDigestSize(ctx->hmac.macType);
}

void hmac_ctx_reset(hmac_ctx_t *ctx)
{
    int ret;
    if (ctx)
    {
        if ((ret = wc_HmacSetKey(&ctx->hmac, ctx->hmac.macType,
                (uint8_t*) &ctx->key, ctx->key_len)) != 0)
        {
            msg(M_FATAL, "wc_HmacSetKey failed. Errno: %d", ret);
        }
    }
}

void hmac_ctx_update(hmac_ctx_t *ctx, const uint8_t *src, int src_len)
{
    int ret;
    if (ctx && src)
    {
        if ((ret = wc_HmacUpdate(&ctx->hmac, src, src_len)) != 0)
        {
            msg(M_FATAL, "wc_HmacUpdate failed. Errno: %d", ret);
        }
    }
}

void hmac_ctx_final(hmac_ctx_t *ctx, uint8_t *dst)
{
    int ret;
    if (ctx && dst)
    {
        if ((ret = wc_HmacFinal(&ctx->hmac, dst)) != 0)
        {
            msg(M_FATAL, "wc_HmacFinal failed. Errno: %d", ret);
        }
    }
}

provider_t *crypto_load_provider(const char *provider)
{
    if (provider)
    {
        msg(M_WARN, "Note: wolfssl TLS provider functionality is not available");
    }
    return NULL;
}

void crypto_unload_provider(const char *provname, provider_t *provider)
{
}

#endif /* ENABLE_CRYPTO_WOLFSSL */
