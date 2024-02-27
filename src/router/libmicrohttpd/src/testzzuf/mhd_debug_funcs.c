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
 * @file testzzuf/mhd_debug_funcs.c
 * @brief  Implementations of MHD private debug functions
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_debug_funcs.h"
#include "internal.h"
#include "mhd_sockets.h"

/**
 * Checks whether MHD can use accept() syscall and
 * avoid accept4() syscall.
 * @return non-zero if accept() is possible,
 *         zero if accept4() is always used by MHD
 */
int
MHD_is_avoid_accept4_possible_ (void)
{
#if ! defined(USE_ACCEPT4)
  return ! 0;
#else  /* ! USE_ACCEPT4 */
  return (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_DEBUG_BUILD)) ?
         ! 0 : 0;
#endif /* ! USE_ACCEPT4 */
}


/**
 * Switch MHD daemon to use accept() syscalls for new connections.
 * @param daemon the daemon to operate
 */
void
MHD_avoid_accept4_ (struct MHD_Daemon *daemon)
{
  (void) daemon; /* Mute compiler warning */
#ifdef USE_ACCEPT4
#ifdef _DEBUG
  daemon->avoid_accept4 = true;
#else  /* ! _DEBUG */
  abort ();
#endif /* ! _DEBUG */
#endif /* USE_ACCEPT4 */
}

#ifdef MHD_ASAN_ACTIVE
#define MHD_ASAN_ENABLED_ 1
#else  /* ! MHD_ASAN_ACTIVE */
#ifdef __SANITIZE_ADDRESS__
#define MHD_ASAN_ENABLED_ 1
#else  /* ! __SANITIZE_ADDRESS__ */
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define MHD_ASAN_ENABLED_ 1
#endif /* __has_feature(address_sanitizer) */
#endif /* __has_feature */
#endif /* ! __SANITIZE_ADDRESS__ */
#endif /* ! MHD_ASAN_ACTIVE */
/**
 * Checks whether any know sanitizer is enabled for this build.
 * zzuf does not work together with sanitizers as both are intercepting
 * standard library calls.
 * @return non-zero if any sanitizer is enabled,
 *         zero otherwise
 */
int
MHD_are_sanitizers_enabled_ (void)
{
  int ret = 0;
#ifdef MHD_ASAN_ENABLED_
  ++ret;
#endif /* ! MHD_ASAN_ENABLED_ */
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
  ++ret;
#endif
#if __has_feature(memory_sanitizer)
  ++ret;
#endif
#if __has_feature(dataflow_sanitizer)
  ++ret;
#endif
#else  /* ! defined(__has_feature) */
#ifdef __SANITIZE_THREAD__
  ++ret;
#endif
#endif /* ! defined(__has_feature) */
  return ret;
}
