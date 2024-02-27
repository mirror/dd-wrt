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
 * @file microhttpd/md5.h
 * @brief  Calculation of MD5 digest
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_MD5_H
#define MHD_MD5_H 1

#include "mhd_options.h"
#include <stdint.h>
#ifdef HAVE_STDDEF_H
#include <stddef.h>  /* for size_t */
#endif /* HAVE_STDDEF_H */

/**
 * Number of bits in single MD5 word.
 */
#define MD5_WORD_SIZE_BITS 32

/**
 * Number of bytes in single MD5 word.
 */
#define MD5_BYTES_IN_WORD (MD5_WORD_SIZE_BITS / 8)

/**
 * Hash is kept internally as four 32-bit words.
 * This is intermediate hash size, used during computing the final digest.
 */
#define MD5_HASH_SIZE_WORDS 4

/**
 * Size of MD5 resulting digest in bytes.
 * This is the final digest size, not intermediate hash.
 */
#define MD5_DIGEST_SIZE_WORDS MD5_HASH_SIZE_WORDS

/**
 * Size of MD5 resulting digest in bytes
 * This is the final digest size, not intermediate hash.
 */
#define MD5_DIGEST_SIZE (MD5_DIGEST_SIZE_WORDS * MD5_BYTES_IN_WORD)

/**
 * Size of MD5 digest string in chars including termination NUL.
 */
#define MD5_DIGEST_STRING_SIZE ((MD5_DIGEST_SIZE) * 2 + 1)

/**
 * Size of MD5 single processing block in bits.
 */
#define MD5_BLOCK_SIZE_BITS 512

/**
 * Size of MD5 single processing block in bytes.
 */
#define MD5_BLOCK_SIZE (MD5_BLOCK_SIZE_BITS / 8)

/**
 * Size of MD5 single processing block in words.
 */
#define MD5_BLOCK_SIZE_WORDS (MD5_BLOCK_SIZE_BITS / MD5_WORD_SIZE_BITS)


/**
 * MD5 calculation context
 */
struct Md5Ctx
{
  uint32_t H[MD5_HASH_SIZE_WORDS];         /**< Intermediate hash value / digest at end of calculation */
  uint32_t buffer[MD5_BLOCK_SIZE_WORDS];   /**< MD5 input data buffer */
  uint64_t count;                          /**< number of bytes, mod 2^64 */
};

/**
 * Initialise structure for MD5 calculation.
 *
 * @param ctx the calculation context
 */
void
MHD_MD5_init (struct Md5Ctx *ctx);


/**
 * MD5 process portion of bytes.
 *
 * @param ctx the calculation context
 * @param data bytes to add to hash
 * @param length number of bytes in @a data
 */
void
MHD_MD5_update (struct Md5Ctx *ctx,
                const uint8_t *data,
                size_t length);


/**
 * Finalise MD5 calculation, return digest.
 *
 * @param ctx the calculation context
 * @param[out] digest set to the hash, must be #MD5_DIGEST_SIZE bytes
 */
void
MHD_MD5_finish (struct Md5Ctx *ctx,
                uint8_t digest[MD5_DIGEST_SIZE]);

/**
 * Indicates that function MHD_MD5_finish() (without context reset) is available
 */
#define MHD_MD5_HAS_FINISH 1

#endif /* MHD_MD5_H */
