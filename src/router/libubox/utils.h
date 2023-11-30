/*
 * utils - misc libubox utility functions
 *
 * Copyright (C) 2012 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __LIBUBOX_UTILS_H
#define __LIBUBOX_UTILS_H

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

/*
 * calloc_a(size_t len, [void **addr, size_t len,...], NULL)
 *
 * allocate a block of memory big enough to hold multiple aligned objects.
 * the pointer to the full object (starting with the first chunk) is returned,
 * all other pointers are stored in the locations behind extra addr arguments.
 * the last argument needs to be a NULL pointer
 */

#define calloc_a(len, ...) __calloc_a(len, ##__VA_ARGS__, NULL)

void *__calloc_a(size_t len, ...);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define __BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

#ifdef __OPTIMIZE__
extern int __BUILD_BUG_ON_CONDITION_FAILED;
#define BUILD_BUG_ON(condition)					\
	do {							\
		__BUILD_BUG_ON(condition);			\
		if (condition)					\
			__BUILD_BUG_ON_CONDITION_FAILED = 1;	\
	} while(0)
#else
#define BUILD_BUG_ON __BUILD_BUG_ON
#endif

#if defined(__APPLE__) && !defined(CLOCK_MONOTONIC)
#define LIBUBOX_COMPAT_CLOCK_GETTIME

#include <mach/clock_types.h>
#define CLOCK_REALTIME	CALENDAR_CLOCK
#define CLOCK_MONOTONIC	SYSTEM_CLOCK

int clock_gettime(int type, struct timespec *tv);

#endif

#ifdef __GNUC__
#define _GNUC_MIN_VER(maj, min) (((__GNUC__ << 8) + __GNUC_MINOR__) >= (((maj) << 8) + (min)))
#else
#define _GNUC_MIN_VER(maj, min) 0
#endif

#if defined(__linux__) || defined(__CYGWIN__)
#include <byteswap.h>
#include <endian.h>

#elif defined(__APPLE__)
#include <machine/endian.h>
#include <machine/byte_order.h>
#elif defined(__FreeBSD__)
#include <sys/endian.h>
#else
#include <machine/endian.h>
#endif

#ifndef __BYTE_ORDER
#define __BYTE_ORDER BYTE_ORDER
#endif
#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN BIG_ENDIAN
#endif
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#endif

#define __constant_swap16(x) ((uint16_t)(				\
	(((uint16_t)(x) & (uint16_t)0x00ffU) << 8) |			\
	(((uint16_t)(x) & (uint16_t)0xff00U) >> 8)))

#define __constant_swap32(x) ((uint32_t)(				\
	(((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) |		\
	(((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) |		\
	(((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) |		\
	(((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))

#define __constant_swap64(x) ((uint64_t)(				\
	(((uint64_t)(x) & (uint64_t)0x00000000000000ffULL) << 56) |	\
	(((uint64_t)(x) & (uint64_t)0x000000000000ff00ULL) << 40) |	\
	(((uint64_t)(x) & (uint64_t)0x0000000000ff0000ULL) << 24) |	\
	(((uint64_t)(x) & (uint64_t)0x00000000ff000000ULL) <<  8) |	\
	(((uint64_t)(x) & (uint64_t)0x000000ff00000000ULL) >>  8) |	\
	(((uint64_t)(x) & (uint64_t)0x0000ff0000000000ULL) >> 24) |	\
	(((uint64_t)(x) & (uint64_t)0x00ff000000000000ULL) >> 40) |	\
	(((uint64_t)(x) & (uint64_t)0xff00000000000000ULL) >> 56)))

/*
 * This returns a constant expression while determining if an argument is
 * a constant expression, most importantly without evaluating the argument.
 */
#define __is_constant(x)						\
	(sizeof(int) == sizeof(*(1 ? ((void*)((long)(x) * 0l)) : (int*)1)))

#define __eval_once(func, x)						\
	({ __typeof__(x) __x = x; func(__x); })

#ifdef __cplusplus
/*
 * g++ does not support __builtin_choose_expr, so always use __eval_once.
 * Unfortunately this means that the byte order functions can't be used
 * as a constant expression anymore
 */
#define __eval_safe(func, x) __eval_once(func, x)
#else
#define __eval_safe(func, x)						\
	__builtin_choose_expr(__is_constant(x),				\
			      func(x), __eval_once(func, x))
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN

#define const_cpu_to_be64(x) __constant_swap64(x)
#define const_cpu_to_be32(x) __constant_swap32(x)
#define const_cpu_to_be16(x) __constant_swap16(x)

#define const_be64_to_cpu(x) __constant_swap64(x)
#define const_be32_to_cpu(x) __constant_swap32(x)
#define const_be16_to_cpu(x) __constant_swap16(x)

#define const_cpu_to_le64(x) (x)
#define const_cpu_to_le32(x) (x)
#define const_cpu_to_le16(x) (x)

#define const_le64_to_cpu(x) (x)
#define const_le32_to_cpu(x) (x)
#define const_le16_to_cpu(x) (x)

#define cpu_to_be64(x) __eval_safe(__constant_swap64, x)
#define cpu_to_be32(x) __eval_safe(__constant_swap32, x)
#define cpu_to_be16(x) __eval_safe(__constant_swap16, x)

#define be64_to_cpu(x) __eval_safe(__constant_swap64, x)
#define be32_to_cpu(x) __eval_safe(__constant_swap32, x)
#define be16_to_cpu(x) __eval_safe(__constant_swap16, x)

#define cpu_to_le64(x) (x)
#define cpu_to_le32(x) (x)
#define cpu_to_le16(x) (x)

#define le64_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#define le16_to_cpu(x) (x)

#else /* __BYTE_ORDER == __LITTLE_ENDIAN */

#define const_cpu_to_le64(x) __constant_swap64(x)
#define const_cpu_to_le32(x) __constant_swap32(x)
#define const_cpu_to_le16(x) __constant_swap16(x)

#define const_le64_to_cpu(x) __constant_swap64(x)
#define const_le32_to_cpu(x) __constant_swap32(x)
#define const_le16_to_cpu(x) __constant_swap16(x)

#define const_cpu_to_be64(x) (x)
#define const_cpu_to_be32(x) (x)
#define const_cpu_to_be16(x) (x)

#define const_be64_to_cpu(x) (x)
#define const_be32_to_cpu(x) (x)
#define const_be16_to_cpu(x) (x)

#define cpu_to_le64(x) __eval_safe(__constant_swap64, x)
#define cpu_to_le32(x) __eval_safe(__constant_swap32, x)
#define cpu_to_le16(x) __eval_safe(__constant_swap16, x)

#define le64_to_cpu(x) __eval_safe(__constant_swap64, x)
#define le32_to_cpu(x) __eval_safe(__constant_swap32, x)
#define le16_to_cpu(x) __eval_safe(__constant_swap16, x)

#define cpu_to_be64(x) (x)
#define cpu_to_be32(x) (x)
#define cpu_to_be16(x) (x)

#define be64_to_cpu(x) (x)
#define be32_to_cpu(x) (x)
#define be16_to_cpu(x) (x)

#endif

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#ifndef __constructor
#define __constructor __attribute__((constructor))
#endif

#ifndef __destructor
#define __destructor __attribute__((destructor))
#endif

#ifndef __hidden
#define __hidden __attribute__((visibility("hidden")))
#endif

#ifndef __has_attribute
# define __has_attribute(x) 0
#endif

#ifndef fallthrough
# if __has_attribute(__fallthrough__)
#  define fallthrough __attribute__((__fallthrough__))
# else
#  define fallthrough do {} while (0)  /* fallthrough */
# endif
#endif

int b64_encode(const void *src, size_t src_len,
	       void *dest, size_t dest_len);

int b64_decode(const void *src, void *dest, size_t dest_len);

#define B64_ENCODE_LEN(_len)	((((_len) + 2) / 3) * 4 + 1)
#define B64_DECODE_LEN(_len)	(((_len) / 4) * 3 + 1)

static inline unsigned int cbuf_order(unsigned int x)
{
	return 32 - __builtin_clz(x - 1);
}

static inline unsigned long cbuf_size(int order)
{
	unsigned long page_size = sysconf(_SC_PAGESIZE);
	unsigned long ret = 1ULL << order;

	if (ret < page_size)
		ret = page_size;

	return ret;
}

void *cbuf_alloc(unsigned int order);
void cbuf_free(void *ptr, unsigned int order);
int mkdir_p(char *dir, mode_t mask);

#endif
