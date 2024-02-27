/*
     This file is part of libmicrohttpd
     Copyright (C) 2019-2022 Evgeny Grin (Karlson2k)

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
 * @file microhttpd/sha256.h
 * @brief  Calculation of SHA-256 digest
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_SHA256_H
#define MHD_SHA256_H 1

#include "mhd_options.h"
#include <stdint.h>
#ifdef HAVE_STDDEF_H
#include <stddef.h>  /* for size_t */
#endif /* HAVE_STDDEF_H */


/**
 *  Digest is kept internally as 8 32-bit words.
 */
#define SHA256_DIGEST_SIZE_WORDS 8

/**
 * Number of bits in single SHA-256 word
 */
#define SHA256_WORD_SIZE_BITS 32

/**
 * Number of bytes in single SHA-256 word
 * used to process data
 */
#define SHA256_BYTES_IN_WORD (SHA256_WORD_SIZE_BITS / 8)

/**
 * Size of SHA-256 digest in bytes
 */
#define SHA256_DIGEST_SIZE (SHA256_DIGEST_SIZE_WORDS * SHA256_BYTES_IN_WORD)

/**
 * Size of SHA-256 digest string in chars including termination NUL
 */
#define SHA256_DIGEST_STRING_SIZE ((SHA256_DIGEST_SIZE) * 2 + 1)

/**
 * Size of single processing block in bits
 */
#define SHA256_BLOCK_SIZE_BITS 512

/**
 * Size of single processing block in bytes
 */
#define SHA256_BLOCK_SIZE (SHA256_BLOCK_SIZE_BITS / 8)

/**
 * Size of single processing block in bytes
 */
#define SHA256_BLOCK_SIZE_WORDS (SHA256_BLOCK_SIZE_BITS / SHA256_WORD_SIZE_BITS)


struct Sha256Ctx
{
  uint32_t H[SHA256_DIGEST_SIZE_WORDS];     /**< Intermediate hash value / digest at end of calculation */
  uint32_t buffer[SHA256_BLOCK_SIZE_WORDS]; /**< SHA256 input data buffer */
  uint64_t count;                           /**< number of bytes, mod 2^64 */
};

/**
 * Initialise structure for SHA256 calculation.
 *
 * @param ctx must be a `struct Sha256Ctx *`
 */
void
MHD_SHA256_init (struct Sha256Ctx *ctx);


/**
 * Process portion of bytes.
 *
 * @param ctx must be a `struct Sha256Ctx *`
 * @param data bytes to add to hash
 * @param length number of bytes in @a data
 */
void
MHD_SHA256_update (struct Sha256Ctx *ctx,
                   const uint8_t *data,
                   size_t length);


/**
 * Finalise SHA256 calculation, return digest.
 *
 * @param ctx must be a `struct Sha256Ctx *`
 * @param[out] digest set to the hash, must be #SHA256_DIGEST_SIZE bytes
 */
void
MHD_SHA256_finish (struct Sha256Ctx *ctx,
                   uint8_t digest[SHA256_DIGEST_SIZE]);

/**
 * Indicates that function MHD_SHA256_finish() (without context reset) is available
 */
#define MHD_SHA256_HAS_FINISH 1

#endif /* MHD_SHA256_H */
