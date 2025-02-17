/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <sys/syscall.h>
#include <sys/socket.h>

#ifndef __NR_getrandom
#  if defined __x86_64__
#    define __NR_getrandom 318
#  elif defined(__i386__)
#    define __NR_getrandom 355
#  elif defined(__arm__)
#    define __NR_getrandom 384
# elif defined(__aarch64__)
#    define __NR_getrandom 278
#  elif defined(__ia64__)
#    define __NR_getrandom 1339
#  elif defined(__m68k__)
#    define __NR_getrandom 352
#  elif defined(__s390x__)
#    define __NR_getrandom 349
#  elif defined(__powerpc__)
#    define __NR_getrandom 359
#  elif defined _MIPS_SIM
#    if _MIPS_SIM == _MIPS_SIM_ABI32
#      define __NR_getrandom 4353
#    endif
#    if _MIPS_SIM == _MIPS_SIM_NABI32
#      define __NR_getrandom 6317
#    endif
#    if _MIPS_SIM == _MIPS_SIM_ABI64
#      define __NR_getrandom 5313
#    endif
#  else
#    warning "__NR_getrandom unknown for your architecture"
#    define __NR_getrandom 0xffffffff
#  endif
#endif

#ifndef HAVE_EXPLICIT_BZERO
static inline void explicit_bzero(void *s, size_t n)
{
	memset(s, 0, n);
	__asm__ __volatile__ ("" : : "r"(s) : "memory");
}
#endif

#ifndef SO_BINDTOIFINDEX
#define SO_BINDTOIFINDEX 62
#endif

#ifndef HAVE_RAWMEMCHR
static inline void *rawmemchr(const void *s, int c)
{
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wstringop-overflow=\"")
	return memchr(s, c, PTRDIFF_MAX);
_Pragma("GCC diagnostic pop")
}
#endif
