/* $Id$ */
/*
** Copyright (C) 1998-2003 Chris Reid <chris.reid@codecraftconsultants.com>
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

#ifndef __STDINT_H__
#define __STDINT_H__

#include <limits.h>

typedef __int8    int8_t;
typedef __int16   int16_t;
typedef __int32   int32_t;

typedef unsigned __int8    uint8_t;
#define UINT8_T_DEFINED
typedef unsigned __int16   uint16_t;
#define UINT16_T_DEFINED
typedef unsigned __int32   uint32_t;
#define UINT32_T_DEFINED

#ifdef _MSC_VER
# if _MSC_VER <= 1200  /* Visual C++ 6.0 */
/* Visual C++ 6.0 and before can't convert an unsigned __int64
 * to a double, but can convert a signed __int64 to a double.
 * Alot of the code converts the UINT64 to a double for percent
 * calculations so we have to make this check */
typedef __int64   uint64_t;
typedef __int64    int64_t;
# else
typedef unsigned __int64   uint64_t;
typedef          __int64    int64_t;
# endif  /* _MSC_VER <= 1200 */
#endif  /* _MSC_VER */

/* win32 is ILP32 so a long will hold a ptr */
#ifdef _MSC_VER
/* Visual C++ 6.0 - only 32 bit
 * Later Visual Studio versions define these */
# if _MSC_VER <= 1200
typedef long int           intptr_t;
typedef unsigned long int  uintptr_t;
#define _INTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
# endif  /* #if _MSC_VER <= 1200 */
#else
typedef long int           intptr_t;
typedef unsigned long int  uintptr_t;
#define _INTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
#endif  /* #ifdef _MSC_VER */

#ifndef HAVE_U_INT8_T
typedef uint8_t   u_int8_t;
#define HAVE_U_INT8_T
#endif
#ifndef HAVE_U_INT16_T
typedef uint16_t  u_int16_t;
#define HAVE_U_INT16_T
#endif
#ifndef HAVE_U_INT32_T
typedef uint32_t  u_int32_t;
#define HAVE_U_INT32_T
#endif
#ifndef HAVE_U_INT64_T
typedef uint64_t  u_int64_t;
#define HAVE_U_INT64_T
#endif

#ifndef INT8_MAX
# define INT8_MAX     _I8_MAX
#endif
#ifndef INT16_MAX
# define INT16_MAX    _I16_MAX
#endif
#ifndef INT32_MAX
# define INT32_MAX    _I32_MAX
#endif
#ifndef INT64_MAX
# define INT64_MAX    _I64_MAX
#endif
#ifndef UINT8_MAX
# define UINT8_MAX    _UI8_MAX
#endif
#ifndef UINT16_MAX
# define UINT16_MAX   _UI16_MAX
#endif
#ifndef UINT32_MAX
# define UINT32_MAX   _UI32_MAX
#endif

#ifndef UINT64_MAX
# ifdef _MSC_VER
#  if _MSC_VER <= 1200  /* Visual C++ 6.0 */
    /* because we have to use a signed 64 bit type for double conversion */
#   define UINT64_MAX _I64_MAX
#  else
#   define UINT64_MAX _UI64_MAX
#  endif
# else
   /* No MSC_VER, make it signed */
#  define UINT64_MAX _I64_MAX
# endif
#endif

#ifndef SIZE_MAX
#define SIZE_MAX UINT32_MAX
#endif

#endif  /* __STDINT_H__ */

