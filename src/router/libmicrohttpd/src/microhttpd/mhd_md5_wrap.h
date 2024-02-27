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
 * @file microhttpd/mhd_md5_wrap.h
 * @brief  Simple wrapper for selection of built-in/external MD5 implementation
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_MD5_WRAP_H
#define MHD_MD5_WRAP_H 1

#include "mhd_options.h"
#ifndef MHD_MD5_SUPPORT
#error This file must be used only when MD5 is enabled
#endif
#ifndef MHD_MD5_TLSLIB
#include "md5.h"
#else  /* MHD_MD5_TLSLIB */
#include "md5_ext.h"
#endif /* MHD_MD5_TLSLIB */

#ifndef MD5_DIGEST_SIZE
/**
 * Size of MD5 resulting digest in bytes
 * This is the final digest size, not intermediate hash.
 */
#define MD5_DIGEST_SIZE (16)
#endif /* ! MD5_DIGEST_SIZE */

#ifndef MD5_DIGEST_STRING_SIZE
/**
 * Size of MD5 digest string in chars including termination NUL.
 */
#define MD5_DIGEST_STRING_SIZE ((MD5_DIGEST_SIZE) * 2 + 1)
#endif /* ! MD5_DIGEST_STRING_SIZE */

#ifndef MHD_MD5_TLSLIB
/**
 * Universal ctx type mapped for chosen implementation
 */
#define Md5CtxWr Md5Ctx
#else  /* MHD_MD5_TLSLIB */
/**
 * Universal ctx type mapped for chosen implementation
 */
#define Md5CtxWr Md5CtxExt
#endif /* MHD_MD5_TLSLIB */

#ifndef MHD_MD5_HAS_INIT_ONE_TIME
/**
 * Setup and prepare ctx for hash calculation
 */
#define MHD_MD5_init_one_time(ctx) MHD_MD5_init(ctx)
#endif /* ! MHD_MD5_HAS_INIT_ONE_TIME */

#ifndef MHD_MD5_HAS_FINISH_RESET
/**
 * Re-use the same ctx for the new hashing after digest calculated
 */
#define MHD_MD5_reset(ctx) MHD_MD5_init(ctx)
/**
 * Finalise MD5 calculation, return digest, reset hash calculation.
 */
#define MHD_MD5_finish_reset(ctx,digest) MHD_MD5_finish(ctx,digest), \
                                         MHD_MD5_reset(ctx)

#else  /* MHD_MD5_HAS_FINISH_RESET */
#define MHD_MD5_reset(ctx) (void)0
#endif /* MHD_MD5_HAS_FINISH_RESET */

#ifndef MHD_MD5_HAS_DEINIT
#define MHD_MD5_deinit(ignore) (void)0
#endif /* HAVE_MD5_DEINIT */

/* Sanity checks */

#if ! defined(MHD_MD5_HAS_FINISH_RESET) && ! defined(MHD_MD5_HAS_FINISH)
#error Required at least one of MHD_MD5_finish_reset(), MHD_MD5_finish_reset()
#endif /* ! MHD_MD5_HAS_FINISH_RESET && ! MHD_MD5_HAS_FINISH */

#endif /* MHD_MD5_WRAP_H */
