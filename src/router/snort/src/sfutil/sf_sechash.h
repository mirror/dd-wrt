
/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __SF_SECHASH_H__
#define __SF_SECHASH_H__

/*  D E F I N E S  *******************************************************/
#define SHA256_HASH_SIZE 32
#define SHA512_HASH_SIZE 64
#define MD5_HASH_SIZE    16

/*  I N C L U D E S  *****************************************************/
#include "config.h"
#include <stdlib.h>

#ifdef HAVE_OPENSSL_SHA
#include <openssl/sha.h>

#define SHA256CONTEXT SHA256_CTX
#define SHA512CONTEXT SHA512_CTX

#define SHA256INIT    SHA256_Init
#define SHA256UPDATE  SHA256_Update
#define SHA256FINAL   SHA256_Final
#define SHA256DIGEST  SHA256

#define SHA512INIT    SHA512_Init
#define SHA512UPDATE  SHA512_Update
#define SHA512FINAL   SHA512_Final
#define SHA512DIGEST  SHA512

#else
#include "sha2.h"

#define SHA256CONTEXT SHA256_CTX
#define SHA512CONTEXT SHA512_CTX

#define SHA256INIT         SHA256_Init
#define SHA256UPDATE       SHA256_Update
#define SHA256FINAL        SHA256_Final
#define SHA256DIGEST       SHA256

#define SHA512INIT         SHA512_Init
#define SHA512UPDATE       SHA512_Update
#define SHA512FINAL        SHA512_Final
#define SHA512DIGEST       SHA512

static inline unsigned char *SHA256(const unsigned char *data, size_t size, unsigned char *digest){
    static unsigned char d[SHA256_HASH_SIZE];
    SHA256CONTEXT c;

    if(!digest)
        digest = d;

    
    SHA256INIT(&c);
    SHA256UPDATE(&c, data, size);
    SHA256FINAL(digest, &c);
    return digest;
}

static inline unsigned char *SHA512(const unsigned char *data, size_t size, unsigned char *digest){
    static unsigned char d[SHA512_HASH_SIZE];
    SHA512CONTEXT c;

    if(!digest)
        digest = d;

    SHA512INIT(&c);
    SHA512UPDATE(&c, data, size);
    SHA512FINAL(digest, &c);
    return digest;
}

#endif /* HAVE_OPENSSL_SHA */

#ifdef HAVE_OPENSSL_MD5
#include <openssl/md5.h>

#define MD5CONTEXT MD5_CTX

#define MD5INIT    MD5_Init
#define MD5UPDATE  MD5_Update
#define MD5FINAL   MD5_Final
#define MD5DIGEST  MD5

#else
#include "md5.h"
#define MD5CONTEXT struct MD5Context

#define MD5INIT   MD5Init
#define MD5UPDATE MD5Update
#define MD5FINAL  MD5Final
#define MD5DIGEST MD5

static inline unsigned char *MD5(const unsigned char *data, size_t size, unsigned char *digest){
    static unsigned char d[MD5_HASH_SIZE];
    MD5CONTEXT c;

    if(!digest)
        digest = d;

    MD5INIT(&c);
    MD5UPDATE(&c, data, size);
    MD5FINAL(digest, &c);
    return digest;
}
#endif /* HAVE_OPENSSL_MD5 */

/* A set of secure hash types */
typedef enum
{
    SECHASH_NONE = 0,
    SECHASH_SHA256 = 1,
    SECHASH_SHA512,
    SECHASH_MD5
} Secure_Hash_Type;


/*  P R O T O T Y P E S  *************************************************/
unsigned int SecHash_Type2Length( const Secure_Hash_Type type );

Secure_Hash_Type SecHash_Name2Type( const char *name );

#endif  /* __SF_SECHASH_H__ */
