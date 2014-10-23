/*!
 * @brief Routines for handling CHAP authentication.
 *
 * @file sstp-chap.c
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
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/md4.h>
#include "sstp-private.h"
#include "sstp-chap.h"

#ifdef __SSTP_UNIT_TEST_CHAP
#undef log_err
#define log_err(x,args...) \
    printf(x"\n", ##args)
#endif

/*< Indicate that we are sending */
#define SSTP_CHAP_SENDING   0x01

/*< Indicating that we are acting as a server */
#define SSTP_CHAP_SERVER    0x02


/*!
 * @brief Create the a double MD4 hash from a password made into unicode
 * @param pass  The password as specified by command line
 * @param len   The length of the password
 * @param hash  The resulting hash from this operation.
 * 
 * @retval 0: success, -1: failure
 */
static int sstp_chap_hash_pass(const char *pass, int len, 
    uint8_t hash[16])
{
    uint8_t buf[512] = {};
    uint8_t inx;
    MD4_CTX ctx;

    if (len > 255)
    {
        return -1;
    }

    /* Convert to unicode */
    for (inx = 0; inx < len; inx++)
    {
        buf[(inx << 1)] = pass[inx];
    }
    
    /* Generate the hash */
    MD4_Init    (&ctx);
    MD4_Update  (&ctx, buf, (len << 1));
    MD4_Final   (hash, &ctx);

    /* Generate the hash hash */
    MD4_Init    (&ctx);
    MD4_Update  (&ctx, hash, 16);
    MD4_Final   (hash, &ctx);

    return 0;
}


/*!
 * @brief Create the master key given the nt_response from handshake
 *  and the double MD4 password hash
 * 
 * @param phash     The double MD4 password hash
 * @param nt_resp   The nt_response field from MSCHAP handshake
 * @param master    The master key
 * @param mlen      The length of the master key
 * 
 * @retval 0: success (always)
 */
static int sstp_chap_hash_master(uint8_t phash[16], uint8_t nt_resp[24],
    uint8_t master[16], uint8_t mlen)
{
    uint8_t buf[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    
    /* The magic constant used in key derivations for chap */
    static unsigned char magic1[27] =
    {
        0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74,
        0x68, 0x65, 0x20, 0x4d, 0x50, 0x50, 0x45, 0x20, 0x4d,
        0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x4b, 0x65, 0x79
    };
    
    /* Perform the SHA1 operation */
    SHA1_Init   (&ctx);
    SHA1_Update (&ctx, phash,   16);
    SHA1_Update (&ctx, nt_resp, 24);
    SHA1_Update (&ctx, magic1,  27);
    SHA1_Final  (buf, &ctx);
    
    /* Copy the result to the output */
    memcpy(master, buf, mlen);
    return 0;
}


/*!
 * @brief Create the session's MPPE send / receive keys
 * @param key    The output key
 * @param master The master key
 * @param flag   Indication if we are sending, and acting as a server
 * 
 * @retval 0: success, -1: failure
 */
static int sstp_chap_hash_session(uint8_t key[16], uint8_t master[16],
    uint8_t flag)
{
    uint8_t buf[SHA_DIGEST_LENGTH];
    uint8_t pad[40];
    uint8_t *magic;
    SHA_CTX ctx;

    /* Magic2:
     * "On the client side, this is the send key; on the server side, 
     *  it is the receive key."
     */
    uint8_t magic2[84] =
    {
        0x4f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x69,
        0x65, 0x6e, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2c, 0x20,
        0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
        0x65, 0x20, 0x73, 0x65, 0x6e, 0x64, 0x20, 0x6b, 0x65, 0x79,
        0x3b, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73,
        0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73, 0x69, 0x64, 0x65,
        0x2c, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
        0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
        0x6b, 0x65, 0x79, 0x2e
    };

    /* Magic3: 
     * "On the client side, this is the receive key; on the server side,
     "  it is the send key."
     */
    uint8_t magic3[84] =
    {
        0x4f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x69,
        0x65, 0x6e, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2c, 0x20,
        0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
        0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
        0x6b, 0x65, 0x79, 0x3b, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68,
        0x65, 0x20, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73,
        0x69, 0x64, 0x65, 0x2c, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73,
        0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x65, 0x6e, 0x64, 0x20,
        0x6b, 0x65, 0x79, 0x2e
    };

    memset(buf, 0, 20);

    if (SSTP_CHAP_SENDING & flag) 
    {
        magic = (SSTP_CHAP_SERVER & flag) 
            ? magic3
            : magic2;
    } 
    else 
    {
        magic = (SSTP_CHAP_SERVER & flag)
            ? magic2
            : magic3;
    }

    /* Compute the session key */
    SHA1_Init   (&ctx);
    SHA1_Update (&ctx, master, 16);

    /* Add the padding: 40-bytes of 0x00 */
    memset(&pad, 0x00, sizeof(pad));
    SHA1_Update (&ctx, pad, sizeof(pad));

    /* Add the 84-bytes of magic */
    SHA1_Update (&ctx, magic, 84);

    /* Add the padding: 40-bytes of 0xf2 */
    memset(pad, 0xf2, sizeof(pad));
    SHA1_Update (&ctx, pad, sizeof(pad));

    /* Get the final update */
    SHA1_Final  (buf, &ctx);

    /* Keep the 16 first bytes of the digest */
    memcpy(key, buf, 16);
    return 0;
}


int sstp_chap_mppe_get(sstp_chap_st *ctx, const char *password, 
        uint8_t skey[16], uint8_t rkey[16], char server)
{
    uint8_t phash [16]; // Password Hash Hash
    uint8_t master[16]; // Master Key
    uint8_t flag = 0;
    int ret = -1;

    /* Set the flag */
    if (server)
    {
        flag |= SSTP_CHAP_SENDING;
    }

    /* Get password */
    ret = sstp_chap_hash_pass(password, strlen(password), phash);
    if (ret < 0)
    {
        log_err("Could not create password hash");
        goto done;
    }

    /* Get the master key */
    ret = sstp_chap_hash_master(phash, ctx->nt_response, 
            master, sizeof(master));
    if (ret < 0)
    {
        log_err("Could not create master key");
        goto done;
    }

    /* Get the receiving key */
    ret = sstp_chap_hash_session(rkey, master, flag);
    if (ret < 0)
    {
        log_err("Could not create receiving MPPE key");
        goto done;
    }

    /* Get the sending Key */
    flag |= SSTP_CHAP_SENDING;
    ret = sstp_chap_hash_session(skey, master, flag);
    if (ret < 0)
    {
        log_err("Could not create sending MPPE key");
        goto done;
    }

    /* Success */
    ret = 0;

done:

    return ret;
}


#ifdef __SSTP_UNIT_TEST_CHAP

#include <stdlib.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
    int retval = EXIT_FAILURE;

    sstp_chap_st ctx =
    {
        .challenge =
        { 
            0x21, 0x60, 0x6e, 0x07, 0x38, 0x35, 0x6f, 0xec,
            0xc6, 0x03, 0xf5, 0xa0, 0x5d, 0x88, 0x64, 0xa1
        },
        .nt_response =
        {
            0x85, 0x9a, 0x0c, 0x0e, 0xce, 0x47, 0x4d, 0xf2,
            0x0d, 0x0a, 0xe8, 0x31, 0xac, 0x3a, 0xe3, 0xd2,
            0x4f, 0x82, 0x6e, 0x93, 0x67, 0x9e, 0x36, 0xbc
        },
    };

    uint8_t cmp1[16] = 
    {
        0x00, 0x0b, 0xc1, 0xde, 0xa2, 0xcb, 0x85, 0x16, 
        0xbc, 0x77, 0xf5, 0x52, 0xb9, 0xec, 0x5a, 0x03
    };

    uint8_t cmp2[16] = 
    {
        0x93, 0xd9, 0x27, 0x06, 0xf5, 0x13, 0xa2, 0xea,
        0x50, 0xf8, 0xcd, 0x94, 0x69, 0x57, 0x3c, 0xdb
    };

    uint8_t skey[16];
    uint8_t rkey[16];

    /* Get the MPPE keys */
    sstp_chap_mppe_get(&ctx, "DukeNuke3D", skey, rkey, false);

    /* Check the send key */
    if (memcmp(skey, cmp1, 16))
    {
        printf("Send key Failed!\n");
        goto done;
    }

    printf("The MPPE send key is correct\n");

    /* Check the receive key */
    if (memcmp(rkey, cmp2, 16))
    {
        printf("Receive Key Failed!\n");
        goto done;
    }

    printf("The MPPE recv key is correct\n");

    /* Success! */
    retval = EXIT_SUCCESS;

done:

    return retval;
}

#endif
