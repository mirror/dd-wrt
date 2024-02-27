/*
  This file is part of libmicrohttpd
  Copyright (C) 2007-2018 Daniel Pittman and Christian Grothoff
  Copyright (C) 2014-2022 Evgeny Grin (Karlson2k)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * @file microhttpd/mhd_panic.h
 * @brief  Declaration and macros for MHD_panic()
 * @author Daniel Pittman
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_PANIC_H
#define MHD_PANIC_H 1

#include "mhd_options.h"

#ifdef MHD_PANIC
/* Override any possible defined MHD_PANIC macro with proper one */
#undef MHD_PANIC
#endif /* MHD_PANIC */

/* If we have Clang or gcc >= 4.5, use __builtin_unreachable() */
#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ > 4) || \
  (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#define BUILTIN_NOT_REACHED __builtin_unreachable ()
#elif defined(_MSC_FULL_VER)
#define BUILTIN_NOT_REACHED __assume (0)
#else
#define BUILTIN_NOT_REACHED
#endif

/* The MHD_PanicCallback type, but without main header. */
/**
 * Handler for fatal errors.
 */
extern void
(*mhd_panic) (void *cls,
              const char *file,
              unsigned int line,
              const char *reason);

/**
 * Closure argument for "mhd_panic".
 */
extern void *mhd_panic_cls;

#ifdef HAVE_MESSAGES
/**
 * Trigger 'panic' action based on fatal errors.
 *
 * @param msg error message (const char *)
 */
#define MHD_PANIC(msg) do { mhd_panic (mhd_panic_cls, __FILE__, __LINE__, msg); \
                            BUILTIN_NOT_REACHED; } while (0)
#else
/**
 * Trigger 'panic' action based on fatal errors.
 *
 * @param msg error message (const char *)
 */
#define MHD_PANIC(msg) do { mhd_panic (mhd_panic_cls, NULL, __LINE__, NULL); \
                            BUILTIN_NOT_REACHED; } while (0)
#endif

#endif /* MHD_PANIC_H */
