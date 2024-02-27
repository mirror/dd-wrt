/*
     This file is part of GNU libmicrohttpd
     Copyright (C) 2022 Evgeny Grin (Karlson2k)

     GNU libmicrohttpd is free software; you can redistribute it and/or
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
 * @file microhttpd/sha512_256.h
 * @brief  Calculation of SHA-512/256 digest
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_SHA512_256_H
#define MHD_SHA512_256_H 1

#include "mhd_options.h"
#include <stdint.h>
#ifdef HAVE_STDDEF_H
#include <stddef.h>  /* for size_t */
#endif /* HAVE_STDDEF_H */


/**
 * Number of bits in single SHA-512/256 word.
 */
#define SHA512_256_WORD_SIZE_BITS 64

/**
 * Number of bytes in single SHA-512/256 word.
 */
#define SHA512_256_BYTES_IN_WORD (SHA512_256_WORD_SIZE_BITS / 8)

/**
 * Hash is kept internally as 8 64-bit words.
 * This is intermediate hash size, used during computing the final digest.
 */
#define SHA512_256_HASH_SIZE_WORDS 8

/**
 * Size of SHA-512/256 resulting digest in bytes.
 * This is the final digest size, not intermediate hash.
 */
#define SHA512_256_DIGEST_SIZE_WORDS (SHA512_256_HASH_SIZE_WORDS  / 2)

/**
 * Size of SHA-512/256 resulting digest in bytes
 * This is the final digest size, not intermediate hash.
 */
#define SHA512_256_DIGEST_SIZE \
  (SHA512_256_DIGEST_SIZE_WORDS * SHA512_256_BYTES_IN_WORD)

/**
 * Size of SHA-512/256 digest string in chars including termination NUL.
 */
#define SHA512_256_DIGEST_STRING_SIZE ((SHA512_256_DIGEST_SIZE) * 2 + 1)

/**
 * Size of SHA-512/256 single processing block in bits.
 */
#define SHA512_256_BLOCK_SIZE_BITS 1024

/**
 * Size of SHA-512/256 single processing block in bytes.
 */
#define SHA512_256_BLOCK_SIZE (SHA512_256_BLOCK_SIZE_BITS / 8)

/**
 * Size of SHA-512/256 single processing block in words.
 */
#define SHA512_256_BLOCK_SIZE_WORDS \
 (SHA512_256_BLOCK_SIZE_BITS / SHA512_256_WORD_SIZE_BITS)


/**
 * SHA-512/256 calculation context
 */
struct Sha512_256Ctx
{
  uint64_t H[SHA512_256_HASH_SIZE_WORDS];       /**< Intermediate hash value  */
  uint64_t buffer[SHA512_256_BLOCK_SIZE_WORDS]; /**< SHA512_256 input data buffer */
  /**
   * The number of bytes, lower part
   */
  uint64_t count;
  /**
   * The number of bits, high part.
   * Unlike lower part, this counts the number of bits, not bytes.
   */
  uint64_t count_bits_hi;
};

/**
 * Initialise structure for SHA-512/256 calculation.
 *
 * @param ctx the calculation context
 */
void
MHD_SHA512_256_init (struct Sha512_256Ctx *ctx);


/**
 * Process portion of bytes.
 *
 * @param ctx the calculation context
 * @param data bytes to add to hash
 * @param length number of bytes in @a data
 */
void
MHD_SHA512_256_update (struct Sha512_256Ctx *ctx,
                       const uint8_t *data,
                       size_t length);


/**
 * Finalise SHA-512/256 calculation, return digest.
 *
 * @param ctx the calculation context
 * @param[out] digest set to the hash, must be #SHA512_256_DIGEST_SIZE bytes
 */
void
MHD_SHA512_256_finish (struct Sha512_256Ctx *ctx,
                       uint8_t digest[SHA512_256_DIGEST_SIZE]);

#endif /* MHD_SHA512_256_H */
