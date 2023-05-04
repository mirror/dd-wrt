/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2012-2013 Sourcefire, Inc.
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
 **
 ** Date: 02-27-2012
 ** Author: Hui Cao <hcao@sourcefire.com>
 */

#ifndef _OUTPUT_COMMON_H
#define _OUTPUT_COMMON_H

#include <stdint.h>
#include <unistd.h>
#ifndef WIN32
#else
#include <winsock2.h>
#include <windows.h>
#endif

#ifndef IP_MAXPACKET
#define IP_MAXPACKET    65535        /* maximum packet size */
#endif /* IP_MAXPACKET */

#define SNORT_SNPRINTF_SUCCESS 0

#ifndef OUTPUT_SO_PUBLIC
#if defined _WIN32 || defined __CYGWIN__
#  if defined OUTPUT_DLL
#    ifdef __GNUC__
#      define OUTPUT_SO_PUBLIC __attribute__((dllexport))
#    else
#      define OUTPUT_SO_PUBLIC __declspec(dllexport)
#    endif
#  else
#    ifdef __GNUC__
#      define OUTPUT_SO_PUBLIC __attribute__((dllimport))
#    else
#      define OUTPUT_SO_PUBLIC __declspec(dllimport)
#    endif
#  endif
#  define DLL_LOCAL
#else
#  ifdef HAVE_VISIBILITY
#    define OUTPUT_SO_PUBLIC  __attribute__ ((visibility("default")))
#    define OUTPUT_SO_PRIVATE __attribute__ ((visibility("hidden")))
#  else
#    define OUTPUT_SO_PUBLIC
#    define OUTPUT_SO_PRIVATE
#  endif
#endif
#endif

#ifdef _WIN32
# ifdef OUTPUT_DLL
#  define OUTPUT_LINKAGE OUTPUT_SO_PUBLIC
# else
#  define OUTPUT_LINKAGE
# endif
#else
# define OUTPUT_LINKAGE OUTPUT_SO_PUBLIC
#endif

#define OUTPUT_SUCCESS          0  /* Success! */
#define OUTPUT_ERROR           -1  /* Generic error */
#define OUTPUT_ERROR_NOMEM     -2  /* Out of memory error */
#define OUTPUT_ERROR_NOTSUP    -3  /* Functionality is unsupported error */
#define OUTPUT_ERROR_NOMOD     -4  /* No module specified error */
#define OUTPUT_ERROR_INVAL     -5  /* Invalid argument/request error */
#define OUTPUT_ERROR_EXISTS    -6  /* Argument or device already exists */

struct _SnortConfig;

#endif /* _OUTPUT_COMMON_H */
