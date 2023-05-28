/*
     This file is part of libmicrohttpd
     Copyright (C) 2019-2021 Karlson2k (Evgeny Grin)

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library.
     If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file microhttpd/sha1.h
 * @brief  Calculation of SHA-1 digest
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_SHA1_H
#define MHD_SHA1_H 1

#include "mhd_options.h"
#include <stdint.h>
#ifdef HAVE_STDDEF_H
#include <stddef.h>  /* for size_t */
#endif /* HAVE_STDDEF_H */

/**
 *  SHA-1 digest is kept internally as 5 32-bit words.
 */
#define _SHA1_DIGEST_LENGTH 5

/**
 * Number of bits in single SHA-1 word
 */
#define SHA1_WORD_SIZE_BITS 32

/**
 * Number of bytes in single SHA-1 word
 */
#define SHA1_BYTES_IN_WORD (SHA1_WORD_SIZE_BITS / 8)

/**
 * Size of SHA-1 digest in bytes
 */
#define SHA1_DIGEST_SIZE (_SHA1_DIGEST_LENGTH * SHA1_BYTES_IN_WORD)

/**
 * Size of SHA-1 digest string in chars including termination NUL
 */
#define SHA1_DIGEST_STRING_SIZE ((SHA1_DIGEST_SIZE) * 2 + 1)

/**
 * Size of single processing block in bits
 */
#define SHA1_BLOCK_SIZE_BITS 512

/**
 * Size of single processing block in bytes
 */
#define SHA1_BLOCK_SIZE (SHA1_BLOCK_SIZE_BITS / 8)


struct sha1_ctx
{
  uint32_t H[_SHA1_DIGEST_LENGTH];    /**< Intermediate hash value / digest at end of calculation */
  uint8_t buffer[SHA1_BLOCK_SIZE];    /**< SHA256 input data buffer */
  uint64_t count;                     /**< number of bytes, mod 2^64 */
};

/**
 * Initialise structure for SHA-1 calculation.
 *
 * @param ctx must be a `struct sha1_ctx *`
 */
void
MHD_SHA1_init (void *ctx_);


/**
 * Process portion of bytes.
 *
 * @param ctx_ must be a `struct sha1_ctx *`
 * @param data bytes to add to hash
 * @param length number of bytes in @a data
 */
void
MHD_SHA1_update (void *ctx_,
                 const uint8_t *data,
                 size_t length);


/**
 * Finalise SHA-1 calculation, return digest.
 *
 * @param ctx_ must be a `struct sha1_ctx *`
 * @param[out] digest set to the hash, must be #SHA1_DIGEST_SIZE bytes
 */
void
MHD_SHA1_finish (void *ctx_,
                 uint8_t digest[SHA1_DIGEST_SIZE]);

#endif /* MHD_SHA1_H */
