/**
 * @file compat.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief compatibility functions header
 *
 * Copyright (c) 2021 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef _COMPAT_H_
#define _COMPAT_H_

#ifdef _WIN32
/* headers are broken on Windows, which means that some of them simply *have* to come first */
# include <winsock2.h>
# include <ws2tcpip.h>
#endif

#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifndef __WORDSIZE
#  if defined __x86_64__ && !defined __ILP32__
#   define __WORDSIZE 64
#  else
#   define __WORDSIZE 32
#  endif
#endif

#ifndef __INT64_C
#  if __WORDSIZE == 64
#    define __INT64_C(c) c ## L
#    define __UINT64_C(c) c ## UL
#  else
#    define __INT64_C(c) c ## LL
#    define __UINT64_C(c) c ## ULL
#  endif
#endif

#if defined (__GNUC__) || defined (__llvm__)
# define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
# define _PACKED __attribute__((__packed__))
#else
# define UNUSED(x) UNUSED_ ## x
# define _PACKED
#endif

#define HAVE_VDPRINTF
#define HAVE_ASPRINTF
#define HAVE_VASPRINTF
#define HAVE_GETLINE
#define HAVE_STRNDUP
/* #undef HAVE_STRNSTR */
#define HAVE_STRDUPA
#define HAVE_STRCHRNUL
#define HAVE_GET_CURRENT_DIR_NAME
#define HAVE_PTHREAD_MUTEX_TIMEDLOCK
#define HAVE_REALPATH
#define HAVE_LOCALTIME_R
#define HAVE_GMTIME_R
#define HAVE_TIMEGM
#define HAVE_STRPTIME
#define HAVE_MMAP
/* #undef HAVE_STRCASECMP */
#define HAVE_SETENV

#ifndef bswap64
#define bswap64(val) \
    ( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | \
    (((val) >> 24) & 0x0000000000FF0000) | (((val) >>  8) & 0x00000000FF000000) | \
    (((val) <<  8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | \
    (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000) )
#endif

#undef le64toh
#undef htole64

#define IS_BIG_ENDIAN

#ifdef IS_BIG_ENDIAN
# define le64toh(x) bswap64(x)
# define htole64(x) bswap64(x)
#else
# define le64toh(x) (x)
# define htole64(x) (x)
#endif

#define HAVE_STDATOMIC

#ifdef HAVE_STDATOMIC
# include <stdatomic.h>

# define ATOMIC_T atomic_uint_fast32_t
# define ATOMIC_T_MAX UINT_FAST32_MAX

# define ATOMIC_STORE_RELAXED(var, x) atomic_store_explicit(&(var), x, memory_order_relaxed)
# define ATOMIC_LOAD_RELAXED(var) atomic_load_explicit(&(var), memory_order_relaxed)
# define ATOMIC_INC_RELAXED(var) atomic_fetch_add_explicit(&(var), 1, memory_order_relaxed)
# define ATOMIC_ADD_RELAXED(var, x) atomic_fetch_add_explicit(&(var), x, memory_order_relaxed)
# define ATOMIC_DEC_RELAXED(var) atomic_fetch_sub_explicit(&(var), 1, memory_order_relaxed)
# define ATOMIC_SUB_RELAXED(var, x) atomic_fetch_sub_explicit(&(var), x, memory_order_relaxed)
#else
# include <stdint.h>

# define ATOMIC_T uint32_t
# define ATOMIC_T_MAX UINT32_MAX

# define ATOMIC_STORE_RELAXED(var, x) ((var) = (x))
# define ATOMIC_LOAD_RELAXED(var) (var)
# ifndef _WIN32
#  define ATOMIC_INC_RELAXED(var) __sync_fetch_and_add(&(var), 1)
#  define ATOMIC_ADD_RELAXED(var, x) __sync_fetch_and_add(&(var), x)
#  define ATOMIC_DEC_RELAXED(var) __sync_fetch_and_sub(&(var), 1)
#  define ATOMIC_SUB_RELAXED(var, x) __sync_fetch_and_sub(&(var), x)
# else
#  include <windows.h>
#  define ATOMIC_INC_RELAXED(var) InterlockedExchangeAdd(&(var), 1)
#  define ATOMIC_ADD_RELAXED(var, x) InterlockedExchangeAdd(&(var), x)
#  define ATOMIC_DEC_RELAXED(var) InterlockedExchangeAdd(&(var), -1)
#  define ATOMIC_SUB_RELAXED(var, x) InterlockedExchangeAdd(&(var), -(x))
# endif
#endif

#ifndef HAVE_VDPRINTF
int vdprintf(int fd, const char *format, va_list ap);
#endif

#ifndef HAVE_ASPRINTF
int asprintf(char **strp, const char *fmt, ...);
#endif

#ifndef HAVE_VASPRINTF
int vasprintf(char **strp, const char *fmt, va_list ap);
#endif

#ifndef HAVE_GETLINE
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t n);
#endif

#ifndef HAVE_STRNSTR
char *strnstr(const char *s, const char *find, size_t slen);
#endif

#ifndef HAVE_STRDUPA
#define strdupa(s) (             \
{                                \
    char *buf;                   \
    size_t len = strlen(s);      \
    buf = alloca(len + 1);       \
    buf[len] = '\0';             \
    (char *)memcpy(buf, s, len); \
})
#endif

#ifndef HAVE_STRCHRNUL
char *strchrnul(const char *s, int c);
#endif

#ifndef HAVE_GET_CURRENT_DIR_NAME
char *get_current_dir_name(void);
#endif

#ifndef HAVE_PTHREAD_MUTEX_TIMEDLOCK
int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime);
#endif

#ifndef HAVE_REALPATH
char *realpath(const char *path, char *resolved_path);
#endif

#ifndef HAVE_LOCALTIME_R
struct tm *localtime_r(const time_t *timep, struct tm *result);
#endif

#ifndef HAVE_GMTIME_R
struct tm *gmtime_r(const time_t *timep, struct tm *result);
#endif

#ifndef HAVE_TIMEGM
# ifdef _WIN32
#  define timegm _mkgmtime
#  define HAVE_TIMEGM

# else
time_t timegm(struct tm *tm);
# endif
#endif

#ifndef HAVE_STRPTIME
char *strptime(const char *s, const char *format, struct tm *tm);
#endif

#ifdef _WIN32
# define strtok_r strtok_s
#endif

#ifndef HAVE_SETENV
int setenv(const char *name, const char *value, int overwrite);
#endif

#endif /* _COMPAT_H_ */
