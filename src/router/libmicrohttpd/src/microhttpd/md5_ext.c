/*
     This file is part of GNU libmicrohttpd
     Copyright (C) 2022-2023 Evgeny Grin (Karlson2k)

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
 * @file microhttpd/md5_ext.c
 * @brief  Wrapper for MD5 calculation performed by TLS library
 * @author Karlson2k (Evgeny Grin)
 */

#include <gnutls/crypto.h>
#include "md5_ext.h"
#include "mhd_assert.h"


/**
 * Initialise structure for MD5 calculation, allocate resources.
 *
 * This function must not be called more than one time for @a ctx.
 *
 * @param ctx the calculation context
 */
void
MHD_MD5_init_one_time (struct Md5CtxExt *ctx)
{
  ctx->handle = NULL;
  ctx->ext_error = gnutls_hash_init (&ctx->handle,
                                     GNUTLS_DIG_MD5);
  if ((0 != ctx->ext_error) && (NULL != ctx->handle))
  {
    /* GnuTLS may return initialisation error and set the handle at the
       same time. Such handle cannot be used for calculations.
       Note: GnuTLS may also return an error and NOT set the handle. */
    gnutls_free (ctx->handle);
    ctx->handle = NULL;
  }

  /* If handle is NULL, the error must be set */
  mhd_assert ((NULL != ctx->handle) || (0 != ctx->ext_error));
  /* If error is set, the handle must be NULL */
  mhd_assert ((0 == ctx->ext_error) || (NULL == ctx->handle));
}


/**
 * Process portion of bytes.
 *
 * @param ctx the calculation context
 * @param data bytes to add to hash
 * @param length number of bytes in @a data
 */
void
MHD_MD5_update (struct Md5CtxExt *ctx,
                const uint8_t *data,
                size_t length)
{
  if (0 == ctx->ext_error)
    ctx->ext_error = gnutls_hash (ctx->handle, data, length);
}


/**
 * Finalise MD5 calculation, return digest, reset hash calculation.
 *
 * @param ctx the calculation context
 * @param[out] digest set to the hash, must be #MD5_DIGEST_SIZE bytes
 */
void
MHD_MD5_finish_reset (struct Md5CtxExt *ctx,
                      uint8_t digest[MD5_DIGEST_SIZE])
{
  if (0 == ctx->ext_error)
    gnutls_hash_output (ctx->handle, digest);
}


/**
 * Free allocated resources.
 *
 * @param ctx the calculation context
 */
void
MHD_MD5_deinit (struct Md5CtxExt *ctx)
{
  if (NULL != ctx->handle)
    gnutls_hash_deinit (ctx->handle, NULL);
}
