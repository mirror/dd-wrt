/*
 * Copyright (C) 2007 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LVM_UTIL_H
#define _LVM_UTIL_H

#include <inttypes.h>

#define min(a, b) ({ typeof(a) _a = (a); \
		     typeof(b) _b = (b); \
		     (void) (&_a == &_b); \
		     _a < _b ? _a : _b; })

#define max(a, b) ({ typeof(a) _a = (a); \
		     typeof(b) _b = (b); \
		     (void) (&_a == &_b); \
		     _a > _b ? _a : _b; })

#define is_power_of_2(n) ((n) && !((n) & ((n) - 1)))

#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 6)
#define uninitialized_var(x) x
#else
#define uninitialized_var(x) x = x
#endif

/*
 * GCC 3.4 adds a __builtin_clz, which uses the count leading zeros (clz)
 * instruction on arches that have one. Provide a fallback using shifts
 * and comparisons for older compilers.
 */
#ifdef HAVE___BUILTIN_CLZ
#define clz(x) __builtin_clz((x))
#else /* ifdef HAVE___BUILTIN_CLZ */
static unsigned _dm_clz(unsigned x)
{
	int n;

	if ((int)x <= 0) return (~x >> 26) & 32;

	n = 1;

	if ((x >> 16) == 0) {
		n = n + 16;
		x = x << 16;
	}

	if ((x >> 24) == 0) {
		n = n + 8;
		x = x << 8;
	}

	if ((x >> 28) == 0) {
		n = n + 4;
		x = x << 4;
	}

	if ((x >> 30) == 0) {
		n = n + 2;
		x = x << 2;
	}
	n = n - (x >> 31);
	return n;
}
#define clz(x) _dm_clz((x))
#endif /* ifdef HAVE___BUILTIN_CLZ */

#ifdef HAVE___BUILTIN_CLZLL
#define clzll(x) __builtin_clzll((x))
#else /* ifdef HAVE___BUILTIN_CLZ */
static unsigned _dm_clzll(unsigned long long x)
{
	if (x <= 0xffffffff)
		return 32 + clz((unsigned) (x & 0xffffffff));

	return clz(x >> 32);
}
#define clzll(x) _dm_clzll((x))
#endif /* ifdef HAVE___BUILTIN_CLZLL */

#define KERNEL_VERSION(major, minor, release) (((major) << 16) + ((minor) << 8) + (release))

/* Define some portable printing types */
#define PRIsize_t "zu"
#define PRIssize_t "zd"
#define PRIptrdiff_t "td"
#define PRIpid_t PRId32

/* For convenience */
#define FMTsize_t "%" PRIsize_t
#define FMTssize_t "%" PRIssize_t
#define FMTptrdiff_t "%" PRIptrdiff_t
#define FMTpid_t "%" PRIpid_t

#define FMTd8  "%" PRId8
#define FMTd16 "%" PRId16
#define FMTd32 "%" PRId32
#define FMTd64 "%" PRId64

#define FMTi8  "%" PRIi8
#define FMTi16 "%" PRIi16
#define FMTi32 "%" PRIi32
#define FMTi64 "%" PRIi64

#define FMTo8  "%" PRIo8
#define FMTo16 "%" PRIo16
#define FMTo32 "%" PRIo32
#define FMTo64 "%" PRIo64

#define FMTu8  "%" PRIu8
#define FMTu16 "%" PRIu16
#define FMTu32 "%" PRIu32
#define FMTu64 "%" PRIu64

#define FMTx8  "%" PRIx8
#define FMTx16 "%" PRIx16
#define FMTx32 "%" PRIx32
#define FMTx64 "%" PRIx64

#define FMTVGID "%." DM_TO_STRING(ID_LEN) "s"

#endif
