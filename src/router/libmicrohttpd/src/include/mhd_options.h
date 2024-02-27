/*
  This file is part of libmicrohttpd
  Copyright (C) 2016-2021 Karlson2k (Evgeny Grin)

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
 * @file include/mhd_options.h
 * @brief  additional automatic macros for MHD_config.h
 * @author Karlson2k (Evgeny Grin)
 *
 * This file includes MHD_config.h and adds automatic macros based on values
 * in MHD_config.h, compiler built-in macros and commandline-defined macros
 * (but not based on values defined in other headers). Works also as a guard
 * to prevent double inclusion of MHD_config.h
 */

#ifndef MHD_OPTIONS_H
#define MHD_OPTIONS_H 1

#include "MHD_config.h"

/**
 * Macro to make it easy to mark text for translation. Note that
 * we do not actually call gettext() in MHD, but we do make it
 * easy to create a ".po" file so that applications that do want
 * to translate error messages can do so.
 */
#define _(String) (String)

#if defined(_MHD_EXTERN) && ! defined(BUILDING_MHD_LIB)
#undef _MHD_EXTERN
#endif /* _MHD_EXTERN && ! BUILDING_MHD_LIB */

#ifndef _MHD_EXTERN
#if defined(BUILDING_MHD_LIB) && defined(_WIN32) && \
  (defined(DLL_EXPORT) || defined(MHD_W32DLL))
#define _MHD_EXTERN __declspec(dllexport) extern
#else   /* !BUILDING_MHD_LIB || !_WIN32 || (!DLL_EXPORT && !MHD_W32DLL) */
#define _MHD_EXTERN extern
#endif  /* !BUILDING_MHD_LIB || !_WIN32 || (!DLL_EXPORT && !MHD_W32DLL) */
#endif  /* ! _MHD_EXTERN */

/* Some platforms (FreeBSD, Solaris, W32) allow to override
   default FD_SETSIZE by defining it before including
   headers. */
#ifdef FD_SETSIZE
/* FD_SETSIZE defined in command line or in MHD_config.h */
#elif defined(_WIN32) || defined(__CYGWIN__)
/* Platform with WinSock and without overridden FD_SETSIZE */
#define FD_SETSIZE 2048 /* Override default small value (64) */
#else  /* !FD_SETSIZE && !W32 */
/* System default value of FD_SETSIZE is used */
#define _MHD_FD_SETSIZE_IS_DEFAULT 1
#endif /* !FD_SETSIZE && !W32 */

#if defined(HAVE_LINUX_SENDFILE) || defined(HAVE_FREEBSD_SENDFILE) || \
  defined(HAVE_DARWIN_SENDFILE) || defined(HAVE_SOLARIS_SENDFILE)
/* Have any supported sendfile() function. */
#define _MHD_HAVE_SENDFILE
#endif /* HAVE_LINUX_SENDFILE || HAVE_FREEBSD_SENDFILE ||
          HAVE_DARWIN_SENDFILE || HAVE_SOLARIS_SENDFILE */
#if defined(HAVE_LINUX_SENDFILE) || defined(HAVE_SOLARIS_SENDFILE)
#define MHD_LINUX_SOLARIS_SENDFILE 1
#endif /* HAVE_LINUX_SENDFILE || HAVE_SOLARIS_SENDFILE */

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
#  ifndef MHD_USE_THREADS
#    define MHD_USE_THREADS 1
#  endif
#endif /* MHD_USE_POSIX_THREADS || MHD_USE_W32_THREADS */

#if defined(OS390)
#define _OPEN_THREADS
#define _OPEN_SYS_SOCK_IPV6
#define _OPEN_MSGQ_EXT
#define _LP64
#endif

#if defined(_WIN32) && ! defined(__CYGWIN__)
/* Declare POSIX-compatible names */
#define _CRT_DECLARE_NONSTDC_NAMES 1
/* Do not warn about POSIX name usage */
#define _CRT_NONSTDC_NO_WARNINGS 1
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#else /* _WIN32_WINNT */
#if _WIN32_WINNT < 0x0501
#error "Headers for Windows XP or later are required"
#endif /* _WIN32_WINNT < 0x0501 */
#endif /* _WIN32_WINNT */
#ifndef WIN32_LEAN_AND_MEAN
/* Do not include unneeded parts of W32 headers. */
#define WIN32_LEAN_AND_MEAN 1
#endif /* !WIN32_LEAN_AND_MEAN */
#endif /* _WIN32 && ! __CYGWIN__ */

#if defined(__VXWORKS__) || defined(__vxworks) || defined(OS_VXWORKS)
#define RESTRICT __restrict__
#endif /* __VXWORKS__ || __vxworks || OS_VXWORKS */

#if defined(LINUX) && (defined(HAVE_SENDFILE64) || defined(HAVE_LSEEK64)) && \
  ! defined(_LARGEFILE64_SOURCE)
/* On Linux, special macro is required to enable definitions of some xxx64 functions */
#define _LARGEFILE64_SOURCE 1
#endif

#ifdef HAVE_C11_GMTIME_S
/* Special macro is required to enable C11 definition of gmtime_s() function */
#define __STDC_WANT_LIB_EXT1__ 1
#endif /* HAVE_C11_GMTIME_S */

#if defined(MHD_FAVOR_FAST_CODE) && defined(MHD_FAVOR_SMALL_CODE)
#error \
  MHD_FAVOR_FAST_CODE and MHD_FAVOR_SMALL_CODE are both defined. Cannot favor speed and size at the same time.
#endif /* MHD_FAVOR_FAST_CODE && MHD_FAVOR_SMALL_CODE */

/* Define MHD_FAVOR_FAST_CODE to force fast code path or
   define MHD_FAVOR_SMALL_CODE to choose compact code path */
#if ! defined(MHD_FAVOR_FAST_CODE) && ! defined(MHD_FAVOR_SMALL_CODE)
/* Try to detect user preferences */
/* Defined by GCC and many compatible compilers */
#if defined(__OPTIMIZE_SIZE__)
#define MHD_FAVOR_SMALL_CODE 1
#elif defined(__OPTIMIZE__)
#define MHD_FAVOR_FAST_CODE 1
#endif /* __OPTIMIZE__ */
#endif /* !MHD_FAVOR_FAST_CODE && !MHD_FAVOR_SMALL_CODE */

#if ! defined(MHD_FAVOR_FAST_CODE) && ! defined(MHD_FAVOR_SMALL_CODE)
/* Use faster code by default */
#define MHD_FAVOR_FAST_CODE 1
#endif /* !MHD_FAVOR_FAST_CODE && !MHD_FAVOR_SMALL_CODE */

#ifndef MHD_ASAN_ACTIVE
#if (defined(__GNUC__) || defined(_MSC_VER)) && defined(__SANITIZE_ADDRESS__)
#define MHD_ASAN_ACTIVE 1
#elif defined(__has_feature)
#if __has_feature (address_sanitizer)
#define MHD_ASAN_ACTIVE 1
#endif /* __has_feature(address_sanitizer) */
#endif /* __has_feature */
#endif /* MHD_ASAN_ACTIVE */

#if defined(MHD_ASAN_ACTIVE) && defined(HAVE_SANITIZER_ASAN_INTERFACE_H) && \
  (defined(FUNC_PTRCOMPARE_CAST_WORKAROUND_WORKS) || \
  (defined(FUNC_ATTR_PTRCOMPARE_WORKS) && \
  defined(FUNC_ATTR_PTRSUBTRACT_WORKS)) || \
  defined(FUNC_ATTR_NOSANITIZE_WORKS))
#ifndef MHD_ASAN_POISON_ACTIVE
/* User ASAN poisoning could be used */
#warning User memory poisoning is not active
#endif /* ! MHD_ASAN_POISON_ACTIVE */
#else  /* ! (MHD_ASAN_ACTIVE && HAVE_SANITIZER_ASAN_INTERFACE_H &&
           (FUNC_ATTR_PTRCOMPARE_WORKS || FUNC_ATTR_NOSANITIZE_WORKS))   */
#ifdef MHD_ASAN_POISON_ACTIVE
#error User memory poisoning is active, but conditions are not suitable
#endif /* MHD_ASAN_POISON_ACTIVE */
#endif /* ! (MHD_ASAN_ACTIVE && HAVE_SANITIZER_ASAN_INTERFACE_H &&
           (FUNC_ATTR_PTRCOMPARE_WORKS || FUNC_ATTR_NOSANITIZE_WORKS))   */

#ifndef _MSC_FULL_VER
#  define MHD_DATA_TRUNCATION_RUNTIME_CHECK_DISABLE_ /* empty */
#  define MHD_DATA_TRUNCATION_RUNTIME_CHECK_RESTORE_ /* empty */
#else  /* _MSC_FULL_VER */
#  define MHD_DATA_TRUNCATION_RUNTIME_CHECK_DISABLE_ \
   __pragma(runtime_checks("c", off))
#  define MHD_DATA_TRUNCATION_RUNTIME_CHECK_RESTORE_ \
   __pragma(runtime_checks("c", restore))
#endif /* _MSC_FULL_VER */

/**
 * Automatic string with the name of the current function
 */
#if defined(HAVE___FUNC__)
#define MHD_FUNC_       __func__
#define MHD_HAVE_MHD_FUNC_ 1
#elif defined(HAVE___FUNCTION__)
#define MHD_FUNC_       __FUNCTION__
#define MHD_HAVE_MHD_FUNC_ 1
#elif defined(HAVE___PRETTY_FUNCTION__)
#define MHD_FUNC_       __PRETTY_FUNCTION__
#define MHD_HAVE_MHD_FUNC_ 1
#else
#define MHD_FUNC_       "**name unavailable**"
#ifdef MHD_HAVE_MHD_FUNC_
#undef MHD_HAVE_MHD_FUNC_
#endif /* MHD_HAVE_MHD_FUNC_ */
#endif

/* Un-define some HAVE_DECL_* macro if they equal zero.
   This should allow safely use #ifdef in the code.
   Define HAS_DECL_* macros only if matching HAVE_DECL_* macro
   has non-zero value. Unlike HAVE_DECL_*, macros HAS_DECL_*
   cannot have zero value. */
#ifdef HAVE_DECL__SC_NPROCESSORS_ONLN
#  if 0 == HAVE_DECL__SC_NPROCESSORS_ONLN
#    undef HAVE_DECL__SC_NPROCESSORS_ONLN
#  else  /* 0 != HAVE_DECL__SC_NPROCESSORS_ONLN */
#    define HAS_DECL__SC_NPROCESSORS_ONLN 1
#  endif /* 0 != HAVE_DECL__SC_NPROCESSORS_ONLN */
#endif /* HAVE_DECL__SC_NPROCESSORS_ONLN */

#ifdef HAVE_DECL__SC_NPROCESSORS_CONF
#  if 0 == HAVE_DECL__SC_NPROCESSORS_CONF
#    undef HAVE_DECL__SC_NPROCESSORS_CONF
#  else  /* 0 != HAVE_DECL__SC_NPROCESSORS_CONF */
#    define HAS_DECL__SC_NPROCESSORS_CONF 1
#  endif /* 0 != HAVE_DECL__SC_NPROCESSORS_CONF */
#endif /* HAVE_DECL__SC_NPROCESSORS_CONF */

#ifdef HAVE_DECL__SC_NPROC_ONLN
#  if 0 == HAVE_DECL__SC_NPROC_ONLN
#    undef HAVE_DECL__SC_NPROC_ONLN
#  else  /* 0 != HAVE_DECL__SC_NPROC_ONLN */
#    define HAS_DECL__SC_NPROC_ONLN 1
#  endif /* 0 != HAVE_DECL__SC_NPROC_ONLN */
#endif /* HAVE_DECL__SC_NPROC_ONLN */

#ifdef HAVE_DECL__SC_CRAY_NCPU
#  if 0 == HAVE_DECL__SC_CRAY_NCPU
#    undef HAVE_DECL__SC_CRAY_NCPU
#  else  /* 0 != HAVE_DECL__SC_CRAY_NCPU */
#    define HAS_DECL__SC_CRAY_NCPU 1
#  endif /* 0 != HAVE_DECL__SC_CRAY_NCPU */
#endif /* HAVE_DECL__SC_CRAY_NCPU */

#ifdef HAVE_DECL_CTL_HW
#  if 0 == HAVE_DECL_CTL_HW
#    undef HAVE_DECL_CTL_HW
#  else  /* 0 != HAVE_DECL_CTL_HW */
#    define HAS_DECL_CTL_HW 1
#  endif /* 0 != HAVE_DECL_CTL_HW */
#endif /* HAVE_DECL_CTL_HW */

#ifdef HAVE_DECL_HW_NCPUONLINE
#  if 0 == HAVE_DECL_HW_NCPUONLINE
#    undef HAVE_DECL_HW_NCPUONLINE
#  else  /* 0 != HAVE_DECL_HW_NCPUONLINE */
#    define HAS_DECL_HW_NCPUONLINE 1
#  endif /* 0 != HAVE_DECL_HW_NCPUONLINE */
#endif /* HAVE_DECL_HW_NCPUONLINE */

#ifdef HAVE_DECL_HW_AVAILCPU
#  if 0 == HAVE_DECL_HW_AVAILCPU
#    undef HAVE_DECL_HW_AVAILCPU
#  else  /* 0 != HAVE_DECL_HW_AVAILCPU */
#    define HAS_DECL_HW_AVAILCPU 1
#  endif /* 0 != HAVE_DECL_HW_AVAILCPU */
#endif /* HAVE_DECL_HW_AVAILCPU */

#ifdef HAVE_DECL_HW_NCPU
#  if 0 == HAVE_DECL_HW_NCPU
#    undef HAVE_DECL_HW_NCPU
#  else  /* 0 != HAVE_DECL_HW_NCPU */
#    define HAS_DECL_HW_NCPU 1
#  endif /* 0 != HAVE_DECL_HW_NCPU */
#endif /* HAVE_DECL_HW_NCPU */

#ifdef HAVE_DECL_CPU_SETSIZE
#  if 0 == HAVE_DECL_CPU_SETSIZE
#    undef HAVE_DECL_CPU_SETSIZE
#  else  /* 0 != HAVE_DECL_CPU_SETSIZE */
#    define HAS_DECL_CPU_SETSIZE 1
#  endif /* 0 != HAVE_DECL_CPU_SETSIZE */
#endif /* HAVE_DECL_CPU_SETSIZE */

#ifndef MHD_DAUTH_DEF_TIMEOUT_
#  define MHD_DAUTH_DEF_TIMEOUT_ 90
#endif /* ! MHD_DAUTH_DEF_TIMEOUT_ */
#ifndef MHD_DAUTH_DEF_MAX_NC_
#  define MHD_DAUTH_DEF_MAX_NC_ 1000
#endif /* ! MHD_DAUTH_DEF_MAX_NC_ */

#endif /* MHD_OPTIONS_H */
