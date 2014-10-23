/*!
 * @brief API to obtain correct Crypto Binding (CMAC Field)
 *
 * @file sstp-cmac.c
 *
 * @author Copyright (C) 2011 Eivind Naess, 
 *      All Rights Reserved
 *
 * @par License:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#include "sstp-private.h"


void sstp_cmac_init(cmac_ctx_st *ctx, int flag)
{
    /* ASCII: "SSTP inner method derived CMK" */
    uint8_t seed [] =
    {
        0x53, 0x53, 0x54, 0x50, 0x20, 0x69, 0x6E, 0x6E, 
        0x65, 0x72, 0x20, 0x6d, 0x65, 0x74, 0x68, 0x6F,
        0x64, 0x20, 0x64, 0x65, 0x72, 0x69, 0x76, 0x65,
        0x64, 0x20, 0x43, 0x4D, 0x4B
    };

    /* Perform the initialization */
    memset(ctx, 0, sizeof(cmac_ctx_st));
    memcpy(ctx->seed, seed, sizeof(seed));
    ctx->flag = flag;
    ctx->slen = sizeof(seed);
}


void sstp_cmac_send_key(cmac_ctx_st *ctx, uint8_t *key, int len)
{
    int off = (SSTP_CMAC_SERVER & ctx->flag)
        ? 16
        :  0;

    memcpy(&ctx->key[off], key, len);
}


void sstp_cmac_recv_key(cmac_ctx_st *ctx, uint8_t *key, int len)
{
    int off = (SSTP_CMAC_SERVER & ctx->flag)
        ?  0
        : 16;

    memcpy(&ctx->key[off], key, len);
}


/*!
 * @brief Function to create Compound MAC Key (CMK)
 *
 * @par Note:
 *  The generation of the key to establish the Compound MAC Field is:
 *      T1 = HMAC(key, S  | LEN | 0x01);
 *      T2 = HMAC(key, T1 | LEN | 0x02);
 *      T3 = HMAC(key, T2 | LEN | 0x03);
 *      CMK = First 32 bytes of: T1 | T2 | T3 | T4 | ...
 *
 *  In simplicity, it really means the output of the first operation as the output 
 *  using the appropriate SHA1/256 always return the wanted output length.
 */
void sstp_cmac_result(cmac_ctx_st *ctx, uint8_t *msg, int mlen, uint8_t *result, int length)
{
    /* We start with the seed */
    HMAC_CTX hmac;
    uint8_t  key[EVP_MAX_MD_SIZE];
    unsigned int klen = sizeof(key);
    uint8_t  iter = 0x01;
    uint16_t len  = SHA_DIGEST_LENGTH;
    const EVP_MD *(*evp)() = EVP_sha1;

    /* The 256-bit keys are a bit different */
    if (SSTP_CMAC_SHA256 & ctx->flag)
    {
        len = SHA256_DIGEST_LENGTH;
        evp = EVP_sha256;
    }
    
    /*
     * Generate the Key first, using the T1 = HMAC(HLAK, S | LEN | 0x01),
     *   CMACK = T1a
     */
    HMAC_CTX_init(&hmac);
    HMAC_Init   (&hmac, ctx->key, sizeof(ctx->key), evp());
    HMAC_Update (&hmac, (uint8_t*) ctx->seed,  ctx->slen);
    HMAC_Update (&hmac, (uint8_t*) &len,  (int) sizeof(len));
    HMAC_Update (&hmac, (uint8_t*) &iter, (int) sizeof(iter));
    HMAC_Final  (&hmac, key, &klen);
    HMAC_CTX_cleanup(&hmac);

    /*
     * Generate the Compound MAC Field
     */
    HMAC_CTX_init(&hmac);
    HMAC_Init   (&hmac, key, klen, evp());
    HMAC_Update (&hmac, msg, mlen);
    HMAC_Final  (&hmac, result, (unsigned int*) &length);
    HMAC_CTX_cleanup(&hmac);
}



#ifdef __SSTP_UNIT_TEST_CMAC

int sstp_test_sha1(void)
{
    int flag = SSTP_CMAC_SHA1;

    /* Call Connected Message wo/MAC */
    uint8_t sstp_msg [] =
    {
        /* SSTP Header */
        0x10, 0x01, 0x00, 0x70, 0x00, 0x04, 0x00, 0x01, 
        0x00, 0x03, 0x00, 0x68, 0x00, 0x00, 0x00, 0x01,

        /* Nounce */
        0x0F, 0x1A, 0x2D, 0x58, 0xD4, 0xA3, 0xE3, 0x00,
        0x0F, 0xAD, 0x3C, 0xE4, 0x90, 0x6E, 0x07, 0xB7,
        0x07, 0xAA, 0x9E, 0x44, 0x1C, 0xCE, 0xAC, 0x5C,
        0xBD, 0x7B, 0x2C, 0xC1, 0xC9, 0xD8, 0x6C, 0xDF,

        /* Certificate Hash */
        0x58, 0x26, 0xB6, 0x29, 0xBD, 0xA5, 0x9B, 0x8E,
        0x6F, 0xD8, 0xDC, 0xD2, 0x62, 0x2F, 0xD3, 0x4C, 
        0x53, 0x48, 0x05, 0xA5, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        /* MAC Field Zeroed out */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    };

    uint8_t mppe_send_key [] =
    {
        0x4B, 0x31, 0x28, 0xF4, 0x39, 0x25, 0xD9, 0x00, 
        0x6E, 0xEF, 0xB1, 0xC4, 0xE8, 0x65, 0x15, 0xA1
    };

    uint8_t mppe_recv_key [] =
    {
        0xD8, 0x8E, 0x56, 0xBA, 0xB3, 0xCA, 0x2B, 0xDF,
        0x03, 0x73, 0xB7, 0xF5, 0xA8, 0xA1, 0x3B, 0x19
    };

    uint8_t cmac[] =
    {
        0x69, 0x91, 0x5D, 0xD5, 0x83, 0xD8, 0x06, 0x2F, 
        0xEF, 0x16, 0xF6, 0x1D, 0xB2, 0xF0, 0x32, 0x90, 
        0xEC, 0x27, 0xCB, 0x6C
    };

    uint8_t ret = 0;
    uint8_t res[20];
    cmac_ctx_st ctx;

    /* Use the API as defined */
    sstp_cmac_init(&ctx, flag);
    sstp_cmac_send_key(&ctx, mppe_send_key, sizeof(mppe_send_key));
    sstp_cmac_recv_key(&ctx, mppe_recv_key, sizeof(mppe_recv_key));
    sstp_cmac_result(&ctx, sstp_msg, sizeof(sstp_msg), res, sizeof(res));

    /* Compare the result */
    ret = memcmp(res, cmac, sizeof(res));
    if (ret != 0)
    {
        printf("The Compound MAC Field (HMAC-SHA1-160) Failed!\n");
        return -1;
    }

    /* Success! */
    printf("The Compound MAC Field (HMAC-SHA1-160) Succeeded!\n");
    return 0;
}


int sstp_test_sha256(void)
{
    int flag = SSTP_CMAC_SHA256;

    /* Call Connected Message wo/MAC */
    uint8_t sstp_msg [] =
    {
        /* Call Connected */
        0x10, 0x01, 0x00, 0x70, 0x00, 0x04, 0x00, 0x01, 
        0x00, 0x03, 0x00, 0x68, 0x00, 0x00, 0x00, 0x02,

        /* Nounce */
        0x41, 0x2B, 0x48, 0x9A, 0xEB, 0xD7, 0xEC, 0xC7, 
        0xD0, 0x89, 0x66, 0xF2, 0x6B, 0xE7, 0xCD, 0x72,
        0xB2, 0x31, 0xA0, 0xE9, 0x21, 0x0D, 0x7C, 0x91, 
        0xB3, 0x08, 0x86, 0x2B, 0x03, 0x44, 0xC4, 0x35,

        /* Certificate Hash */
        0x79, 0x93, 0xEF, 0x31, 0x4C, 0x49, 0x3D, 0xAC, 
        0xE9, 0xF0, 0x2D, 0x60, 0xE7, 0xE6, 0x1C, 0x84,
        0xB6, 0x69, 0x0A, 0xAF, 0xE9, 0xD7, 0xAE, 0xEA, 
        0x92, 0xCB, 0xBE, 0x8A, 0xD5, 0x99, 0x42, 0x2D,

        /* MAC Field Zero'd out */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    /* HLAK send */
    uint8_t mppe_send_key [] = 
    {
        0x2A, 0x1B, 0xB4, 0x0D, 0x55, 0xAB, 0x0F, 0x5E,
        0xF3, 0x2F, 0x06, 0xF2, 0xB3, 0xCC, 0x73, 0xC4,
    };

    /* HLAK recv */
    uint8_t mppe_recv_key [] =
    {
        0x8F, 0xD3, 0xFA, 0xC4, 0x1D, 0x7A, 0x13, 0x15,
        0xA1, 0x92, 0x28, 0xD9, 0x02, 0x4C, 0xA1, 0x64
    };

    /* Expected result */
    uint8_t cmac [] =
    {
        0x52, 0xA6, 0x8E, 0xFD, 0x8C, 0xFF, 0xBF, 0x52, 
        0x77, 0x0B, 0x8F, 0x0F, 0xE8, 0xEC, 0x73, 0x71,
        0x65, 0x83, 0xAF, 0x6D, 0x61, 0x1E, 0xB6, 0xD1, 
        0x79, 0xB3, 0xB2, 0x08, 0x40, 0x98, 0x54, 0x49
    };

    uint8_t ret = 0;
    uint8_t res[32];
    cmac_ctx_st ctx;

    /* Use the API as defined */
    sstp_cmac_init(&ctx, flag);
    sstp_cmac_send_key(&ctx, mppe_send_key, sizeof(mppe_send_key));
    sstp_cmac_recv_key(&ctx, mppe_recv_key, sizeof(mppe_recv_key));
    sstp_cmac_result(&ctx, sstp_msg, sizeof(sstp_msg), res, sizeof(res));

    /* Compare the result */
    ret = memcmp(res, cmac, sizeof(res));
    if (ret != 0)
    {
        printf("The Compound MAC Field (HMAC-SHA256-256) Failed!\n");
        return -1;
    }

    /* Success! */
    printf("The Compound MAC Field (HMAC-SHA256-256) Succeeded!\n");
    return 0;
}


/*!
 * @brief Unit test per MS-SSTP specification
 */
int main(void)
{
    int retval = 0;
    int ret    = 0;

    /* Test the SHA1 function */
    ret = sstp_test_sha1();
    if (ret != 0)
    {
        retval--;
    }

    /* Test the SHA256 function */
    ret = sstp_test_sha256();
    if (ret != 0)
    {
        retval--;
    }

    if (retval != 0)
    {
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

#endif
