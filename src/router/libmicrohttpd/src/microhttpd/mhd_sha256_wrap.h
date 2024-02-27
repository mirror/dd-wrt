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
 * @file microhttpd/mhd_sha256_wrap.h
 * @brief  Simple wrapper for selection of built-in/external SHA-256
 *         implementation
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_SHA256_WRAP_H
#define MHD_SHA256_WRAP_H 1

#include "mhd_options.h"
#include "mhd_options.h"
#ifndef MHD_SHA256_SUPPORT
#error This file must be used only when SHA-256 is enabled
#endif
#ifndef MHD_SHA256_TLSLIB
#include "sha256.h"
#else  /* MHD_SHA256_TLSLIB */
#include "sha256_ext.h"
#endif /* MHD_SHA256_TLSLIB */

#ifndef SHA256_DIGEST_SIZE
/**
 * Size of SHA-256 resulting digest in bytes
 * This is the final digest size, not intermediate hash.
 */
#define SHA256_DIGEST_SIZE (32)
#endif /* ! SHA256_DIGEST_SIZE */

#ifndef SHA256_DIGEST_STRING_SIZE
/**
 * Size of MD5 digest string in chars including termination NUL.
 */
#define SHA256_DIGEST_STRING_SIZE ((SHA256_DIGEST_SIZE) * 2 + 1)
#endif /* ! SHA256_DIGEST_STRING_SIZE */

#ifndef MHD_SHA256_TLSLIB
/**
 * Universal ctx type mapped for chosen implementation
 */
#define Sha256CtxWr Sha256Ctx
#else  /* MHD_SHA256_TLSLIB */
/**
 * Universal ctx type mapped for chosen implementation
 */
#define Sha256CtxWr Sha256CtxExt
#endif /* MHD_SHA256_TLSLIB */

#ifndef MHD_SHA256_HAS_INIT_ONE_TIME
/**
 * Setup and prepare ctx for hash calculation
 */
#define MHD_SHA256_init_one_time(ctx) MHD_SHA256_init(ctx)
#endif /* ! MHD_SHA256_HAS_INIT_ONE_TIME */

#ifndef MHD_SHA256_HAS_FINISH_RESET
/**
 * Re-use the same ctx for the new hashing after digest calculated
 */
#define MHD_SHA256_reset(ctx) MHD_SHA256_init(ctx)
/**
 * Finalise MD5 calculation, return digest, reset hash calculation.
 */
#define MHD_SHA256_finish_reset(ctx,digest) MHD_SHA256_finish(ctx,digest), \
                                            MHD_SHA256_reset(ctx)

#else  /* MHD_SHA256_HAS_FINISH_RESET */
#define MHD_SHA256_reset(ctx) (void)0
#endif /* MHD_SHA256_HAS_FINISH_RESET */

#ifndef MHD_SHA256_HAS_DEINIT
#define MHD_SHA256_deinit(ignore) (void)0
#endif /* HAVE_SHA256_DEINIT */

/* Sanity checks */

#if ! defined(MHD_SHA256_HAS_FINISH_RESET) && ! defined(MHD_SHA256_HAS_FINISH)
#error Required MHD_SHA256_finish_reset() or MHD_SHA256_finish_reset()
#endif /* ! MHD_SHA256_HAS_FINISH_RESET && ! MHD_SHA256_HAS_FINISH */

#endif /* MHD_SHA256_WRAP_H */
