/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2007-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __SF_TYPES_H__
#define __SF_TYPES_H__

#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"

#ifdef WIN32
#  include "stdint.h"
#  include "inttypes.h"
#else
/* Autoconf uses <sys/types.h>, <inttypes.h> and <stdint.h> as standard includes for
 * determining if these exist so there shouldn't be any typedef conflicts with
 * including <sys/types.h>, <inttypes.h> or <stdint.h> since these would be
 * defined already */
#  if !defined(HAVE_UINT8_T) || !defined(HAVE_U_INT8_T)
#    if !defined(HAVE_UINT8_T) && !defined(HAVE_U_INT8_T)
typedef unsigned char   u_int8_t;
typedef unsigned char    uint8_t;
#    elif defined(HAVE_UINT8_T)
typedef uint8_t   u_int8_t;
#    else
typedef u_int8_t   uint8_t;
#    endif  /* !defined(HAVE_UINT8_T) && !defined(HAVE_U_INT8_T) */
#  endif  /* !defined(HAVE_UINT8_T) || !defined(HAVE_U_INT8_T) */
#  if !defined(HAVE_UINT16_T) || !defined(HAVE_U_INT16_T)
#    if !defined(HAVE_UINT16_T) && !defined(HAVE_U_INT16_T)
typedef unsigned short    u_int16_t;
typedef unsigned short     uint16_t;
#    elif defined(HAVE_UINT16_T)
typedef uint16_t    u_int16_t;
#    else
typedef u_int16_t    uint16_t;
#    endif  /* !defined(HAVE_UINT16_T) && !defined(HAVE_U_INT16_T) */
#  endif  /* !defined(HAVE_UINT16_T) || !defined(HAVE_U_INT16_T) */
#  if !defined(HAVE_UINT32_T) || !defined(HAVE_U_INT32_T)
#    if !defined(HAVE_UINT32_T) && !defined(HAVE_U_INT32_T)
#      if SIZEOF_UNSIGNED_LONG_INT == 4
typedef unsigned long int    u_int32_t;
typedef unsigned long int     uint32_t;
#      elif SIZEOF_UNSIGNED_INT == 4
typedef unsigned int    u_int32_t;
typedef unsigned int     uint32_t;
#      endif  /* SIZEOF_UNSIGNED_LONG_INT == 4 */
#    elif defined(HAVE_UINT32_T)
typedef uint32_t    u_int32_t;
#    else
typedef u_int32_t    uint32_t;
#    endif  /* !defined(HAVE_UINT32_T) && !defined(HAVE_U_INT32_T) */
#  endif  /* !defined(HAVE_UINT32_T) || !defined(HAVE_U_INT32_T) */
#  if !defined(HAVE_UINT64_T) || !defined(HAVE_U_INT64_T)
#    if !defined(HAVE_UINT64_T) && !defined(HAVE_U_INT64_T)
#      if SIZEOF_UNSIGNED_LONG_LONG_INT == 8
typedef unsigned long long int    u_int64_t;
typedef unsigned long long int     uint64_t;
#      elif SIZEOF_UNSIGNED_LONG_INT == 8
typedef unsigned long int    u_int64_t;
typedef unsigned long int     uint64_t;
#      endif
#    elif defined(HAVE_UINT64_T)
typedef uint64_t    u_int64_t;
#    else
typedef u_int64_t    uint64_t;
#    endif  /* !defined(HAVE_UINT64_T) && !defined(HAVE_U_INT64_T) */
#  endif  /* !defined(HAVE_UINT64_T) || !defined(HAVE_U_INT64_T) */
#  ifndef HAVE_INT8_T
typedef char     int8_t;
#  endif
#  ifndef HAVE_INT16_T
typedef short   int16_t;
#  endif
#  ifndef HAVE_INT32_T
#    if SIZEOF_LONG_INT == 4
typedef long int   int32_t;
#    else
typedef int        int32_t;
#    endif
#  endif
#  ifndef HAVE_INT64_T
#    if SIZEOF_LONG_LONG_INT == 8
typedef long long int    int64_t;
#    else
typedef long int         int64_t;
#    endif
#  endif
#  ifndef WIN32
#    ifdef HAVE_INTTYPES_H
/* <inttypes.h> includes <stdint.h> */
#      include <inttypes.h>
#    elif HAVE_STDINT_H
#      include <stdint.h>
#    else
/* Solaris - if inttypes.h is present, it should bring this in */
#      ifndef SYS_INT_TYPES_H
#        if defined(_LP64) || defined(_I32LPx)
typedef long int           intptr_t;
typedef unsigned long int  uintptr_t;
#        else
typedef int           intptr_t;
typedef unsigned int  uintptr_t;
#        endif  /* defined(_LP64) || defined(_I32LPx) */
#      endif  /* SYS_INT_TYPES_H */
#    endif  /* HAVE_INTTYPES_H elseif HAVE_STDINT_H */
#  endif
#endif  /* WIN32 */
#endif  /* HAVE_CONFIG_H */

/* if PRIu64 isn't in <inttypes.h>
 * we define it and similar here */
#ifndef PRIu64
#  if SIZEOF_UNSIGNED_LONG_INT == 8
#    define _SF_PREFIX "l"
#  else
#    define _SF_PREFIX "ll"
#  endif  /* SIZEOF_UNSIGNED_LONG_INT == 8 */
#  define PRIu64 _SF_PREFIX "u"
#  define PRIi64 _SF_PREFIX "i"
#  define PRIx64 _SF_PREFIX "x"
#endif  /* PRIu64 */

/* use these macros (and those in <inttypes.h>)
 * for 64 bit format portability
 */
#define STDu64 "%" PRIu64
#define CSVu64 STDu64 ","
#define FMTu64(fmt) "%" fmt PRIu64

#define STDi64 "%" PRIi64
#define CSVi64 STDi64 ","
#define FMTi64(fmt) "%" fmt PRIi64

#define STDx64 "%" PRIx64
#define CSVx64 STDx64 ","
#define FMTx64(fmt) "%" fmt PRIx64

#ifndef UINT8_MAX
#  define UINT8_MAX 0xff
#endif
#ifndef USHRT_MAX
#  define USHRT_MAX  0xffff
#endif
#ifndef UINT16_MAX
#  define UINT16_MAX 0xffff
#endif
#ifndef UINT32_MAX
#  define UINT32_MAX (4294967295U)
#endif
#ifndef UINT64_MAX
#  if SIZEOF_UNSIGNED_LONG_INT == 8
#    define UINT64_MAX (18446744073709551615UL)
#  else
#    define UINT64_MAX (18446744073709551615ULL)
#  endif  /* SIZEOF_UNSIGNED_LONG_INT == 8 */
#endif  /* UINT64_MAX */

/* Somewhat arbitrary, but should be enough for this application
 * since files shouldn't be buried too deep.  This provides about
 * 15 levels of 255 character path components */
#ifndef PATH_MAX
#  define PATH_MAX 4096
#endif

/* utilities */
#ifndef boolean
#ifndef HAVE_BOOLEAN
typedef unsigned char boolean;
#endif
#endif

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  ifdef __cplusplus
typedef bool _Bool;
#  else
#   define _Bool signed char
#  endif
# endif
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif

#endif  /* __SF_TYPES_H__ */

