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
     License along with GNU libmicrohttpd.
     If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file microhttpd/sha256_ext.h
 * @brief  Wrapper declarations for SHA-256 calculation performed by TLS library
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_SHA256_EXT_H
#define MHD_SHA256_EXT_H 1

#include "mhd_options.h"
#include <stdint.h>
#ifdef HAVE_STDDEF_H
#include <stddef.h>  /* for size_t */
#endif /* HAVE_STDDEF_H */

/**
 * Size of SHA-256 resulting digest in bytes
 * This is the final digest size, not intermediate hash.
 */
#define SHA256_DIGEST_SIZE (32)

/* Actual declaration is in GnuTLS lib header */
struct hash_hd_st;

/**
 * Indicates that struct Sha256CtxExt has 'ext_error'
 */
#define MHD_SHA256_HAS_EXT_ERROR 1

/**
 * SHA-256 calculation context
 */
struct Sha256CtxExt
{
  struct hash_hd_st *handle; /**< Hash calculation handle */
  int ext_error; /**< Non-zero if external error occurs during init or hashing */
};

/**
 * Indicates that MHD_SHA256_init_one_time() function is present.
 */
#define MHD_SHA256_HAS_INIT_ONE_TIME 1

/**
 * Initialise structure for SHA-256 calculation, allocate resources.
 *
 * This function must not be called more than one time for @a ctx.
 *
 * @param ctx the calculation context
 */
void
MHD_SHA256_init_one_time (struct Sha256CtxExt *ctx);


/**
 * SHA-256 process portion of bytes.
 *
 * @param ctx the calculation context
 * @param data bytes to add to hash
 * @param length number of bytes in @a data
 */
void
MHD_SHA256_update (struct Sha256CtxExt *ctx,
                   const uint8_t *data,
                   size_t length);


/**
 * Indicates that MHD_SHA256_finish_reset() function is available
 */
#define MHD_SHA256_HAS_FINISH_RESET 1

/**
 * Finalise SHA-256 calculation, return digest, reset hash calculation.
 *
 * @param ctx the calculation context
 * @param[out] digest set to the hash, must be #SHA256_DIGEST_SIZE bytes
 */
void
MHD_SHA256_finish_reset (struct Sha256CtxExt *ctx,
                         uint8_t digest[SHA256_DIGEST_SIZE]);

/**
 * Indicates that MHD_SHA256_deinit() function is present
 */
#define MHD_SHA256_HAS_DEINIT 1

/**
 * Free allocated resources.
 *
 * @param ctx the calculation context
 */
void
MHD_SHA256_deinit (struct Sha256CtxExt *ctx);

#endif /* MHD_SHA256_EXT_H */
