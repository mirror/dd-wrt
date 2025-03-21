/*
 * Copyright 1995-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * MD5 low level APIs are deprecated for public use, but still ok for
 * internal use.
 */
#include "internal/deprecated.h"

#ifdef OCTEON_OPENSSL
#include "md5_one_octeon.c"
#else
#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>
#include <openssl/crypto.h>

#ifdef CHARSET_EBCDIC
# include <openssl/ebcdic.h>
#endif

unsigned char *MD5(const unsigned char *d, size_t n, unsigned char *md)
{
    MD5_CTX c;
    static unsigned char m[MD5_DIGEST_LENGTH];

    if (md == NULL)
        md = m;
    if (!MD5_Init(&c))
        return NULL;
#ifndef CHARSET_EBCDIC
    MD5_Update(&c, d, n);
#else
    {
        char temp[1024];
        unsigned long chunk;

        while (n > 0) {
            chunk = (n > sizeof(temp)) ? sizeof(temp) : n;
            ebcdic2ascii(temp, d, chunk);
            MD5_Update(&c, temp, chunk);
            n -= chunk;
            d += chunk;
        }
    }
#endif
    MD5_Final(md, &c);
    OPENSSL_cleanse(&c, sizeof(c)); /* security consideration */
    return md;
}
#endif
