/*
     This file is part of GNU libmicrohttpd
     Copyright (C) 2022-2024 Evgeny Grin (Karlson2k)

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
 * @file microhttpd/md5_ext.h
 * @brief  Wrapper declarations for MD5 calculation performed by TLS library
 * @author Karlson2k (Evgeny Grin)
 */
#ifndef MHD_MD5_EXT_H
#define MHD_MD5_EXT_H 1

#include "mhd_options.h"
#include <stdint.h>
#ifdef HAVE_STDDEF_H
#include <stddef.h>  /* for size_t */
#endif /* HAVE_STDDEF_H */

/**
 * Size of MD5 resulting digest in bytes
 * This is the final digest size, not intermediate hash.
 */
#define MD5_DIGEST_SIZE (16)

/* Actual declaration is in GnuTLS lib header */
struct hash_hd_st;

/**
 * Indicates that struct Md5CtxExt has 'ext_error'
 */
#define MHD_MD5_HAS_EXT_ERROR 1

/**
 * MD5 calculation context
 */
struct Md5CtxExt
{
  struct hash_hd_st *handle; /**< Hash calculation handle */
  int ext_error; /**< Non-zero if external error occurs during init or hashing */
};

/**
 * Indicates that MHD_MD5_init_one_time() function is present.
 */
#define MHD_MD5_HAS_INIT_ONE_TIME 1

/**
 * Initialise structure for MD5 calculation, allocate resources.
 *
 * This function must not be called more than one time for @a ctx.
 *
 * @param ctx the calculation context
 */
void
MHD_MD5_init_one_time (struct Md5CtxExt *ctx);


/**
 * MD5 process portion of bytes.
 *
 * @param ctx the calculation context
 * @param data bytes to add to hash
 * @param length number of bytes in @a data
 */
void
MHD_MD5_update (struct Md5CtxExt *ctx,
                const uint8_t *data,
                size_t length);


/**
 * Indicates that MHD_MD5_finish_reset() function is available
 */
#define MHD_MD5_HAS_FINISH_RESET 1

/**
 * Finalise MD5 calculation, return digest, reset hash calculation.
 *
 * @param ctx the calculation context
 * @param[out] digest set to the hash, must be #MD5_DIGEST_SIZE bytes
 */
void
MHD_MD5_finish_reset (struct Md5CtxExt *ctx,
                      uint8_t digest[MD5_DIGEST_SIZE]);

/**
 * Indicates that MHD_MD5_deinit() function is present
 */
#define MHD_MD5_HAS_DEINIT 1

/**
 * Free allocated resources.
 *
 * @param ctx the calculation context
 */
void
MHD_MD5_deinit (struct Md5CtxExt *ctx);

#endif /* MHD_MD5_EXT_H */
