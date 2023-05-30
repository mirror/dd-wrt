/*
  This file is part of libmicrohttpd
  Copyright (C) 2021 Karlson2k (Evgeny Grin)

  This library is free software; you can redistribute it and/or
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
 * @file microhttpd/mhd_align.h
 * @brief  types alignment macros
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_ALIGN_H
#define MHD_ALIGN_H 1

#include "mhd_options.h"
#include <stdint.h>
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#ifdef HAVE_C_ALIGNOF
#ifdef HAVE_STDALIGN_H
#include <stdalign.h>
#endif /* HAVE_STDALIGN_H */
#define _MHD_ALIGNOF(type) alignof(type)
#endif /* HAVE_C_ALIGNOF */

#ifndef _MHD_ALIGNOF
#if defined(_MSC_VER) && ! defined(__clang__) && _MSC_VER >= 1700
#define _MHD_ALIGNOF(type) __alignof(type)
#endif /* _MSC_VER >= 1700 */
#endif /* !_MHD_ALIGNOF */

#ifdef _MHD_ALIGNOF
#if (defined(__GNUC__) && __GNUC__ < 4 && __GNUC_MINOR__ < 9 && \
  ! defined(__clang__)) || \
  (defined(__clang__) && __clang_major__ < 8) || \
  (defined(__clang__) && __clang_major__ < 11 && \
  defined(__apple_build_version__))
/* GCC before 4.9 and clang before 8.0 have incorrect implementation of 'alignof()'
   which returns preferred alignment instead of minimal required alignment */
#define _MHD_ALIGNOF_UNRELIABLE 1
#endif

#if defined(_MSC_VER) && ! defined(__clang__) && _MSC_VER < 1900
/* MSVC has the same problem as old GCC versions:
   '__alignof()' may return "preferred" alignment instead of "required". */
#define _MHD_ALIGNOF_UNRELIABLE 1
#endif /* _MSC_VER < 1900 */
#endif /* _MHD_ALIGNOF */


#ifdef offsetof
#define _MHD_OFFSETOF(strct, membr) offsetof(strct, membr)
#else  /* ! offsetof */
#define _MHD_OFFSETOF(strct, membr) (size_t)(((char*)&(((strct*)0)->membr)) - \
                                     ((char*)((strct*)0)))
#endif /* ! offsetof */

/* Provide a limited set of alignment macros */
/* The set could be extended as needed */
#if defined(_MHD_ALIGNOF) && ! defined(_MHD_ALIGNOF_UNRELIABLE)
#define _MHD_UINT32_ALIGN _MHD_ALIGNOF(uint32_t)
#define _MHD_UINT64_ALIGN _MHD_ALIGNOF(uint64_t)
#else  /* ! _MHD_ALIGNOF */
struct _mhd_dummy_uint32_offset_test
{
  char dummy;
  uint32_t ui32;
};
#define _MHD_UINT32_ALIGN \
  _MHD_OFFSETOF(struct _mhd_dummy_uint32_offset_test, ui32)

struct _mhd_dummy_uint64_offset_test
{
  char dummy;
  uint64_t ui64;
};
#define _MHD_UINT64_ALIGN \
  _MHD_OFFSETOF(struct _mhd_dummy_uint64_offset_test, ui64)
#endif /* ! _MHD_ALIGNOF */

#endif /* ! MHD_ALIGN_H */
