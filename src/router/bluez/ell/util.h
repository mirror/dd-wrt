/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_UTIL_H
#define __ELL_UTIL_H

#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>
#include <endian.h>
#include <byteswap.h>
#include <unistd.h>
#include <errno.h>
#include <sys/uio.h>
#include <ell/cleanup.h>

#ifdef __cplusplus
extern "C" {
#endif

#define l_container_of(ptr, type, member) ({				\
_Pragma("GCC diagnostic push")						\
_Pragma("GCC diagnostic ignored \"-Wcast-align\"")			\
		const __typeof__(((type *) 0)->member) *__mptr = (ptr);	\
		(type *)((char *) __mptr - offsetof(type, member));	\
_Pragma("GCC diagnostic pop")						\
	})

#define L_STRINGIFY(val) L_STRINGIFY_ARG(val)
#define L_STRINGIFY_ARG(contents) #contents

#define L_WARN_ON(condition) __extension__ ({				\
		bool r = !!(condition);					\
		if (__builtin_expect(r, 0))				\
			l_warn("WARNING: %s:%s() condition %s failed",	\
				__FILE__, __func__,			\
				#condition);				\
		r;							\
	})

/*
 * If ELL headers and iterfaces end up getting compiled in a C++
 * environment, even though ELL itself is a C source based and is
 * compiled as such, certain assignments may be flagged by the C++
 * compiler as errors or warnings. The following portable casts should
 * be used in such cases, with a preference towards L_PERMISSIVE_CAST
 * where possible since it is not a cast in C and, therefore, will not
 * mask otherwise-legitimate warnings in that environment.
 */
#ifdef __cplusplus
#define L_CONST_CAST(t, v)       const_cast<t>(v)
#define L_REINTERPRET_CAST(t, v) reinterpret_cast<t>(v)
#define L_STATIC_CAST(t, v)      static_cast<t>(v)
#define L_PERMISSIVE_CAST(t, v)  L_STATIC_CAST(t, v)
#else
#define L_CONST_CAST(t, v)       ((t)(v))
#define L_REINTERPRET_CAST(t, v) ((t)(v))
#define L_STATIC_CAST(t, v)      ((t)(v))
#define L_PERMISSIVE_CAST(t, v)  (v)
#endif

#define L_PTR_TO_UINT(p) ((unsigned int) ((uintptr_t) (p)))
#define L_UINT_TO_PTR(u) ((void *) ((uintptr_t) (u)))

#define L_PTR_TO_INT(p) ((int) ((intptr_t) (p)))
#define L_INT_TO_PTR(u) ((void *) ((intptr_t) (u)))

#define L_GET_UNALIGNED(ptr) __extension__	\
({						\
	struct __attribute__((packed)) {	\
            __typeof__(*(ptr)) __v;		\
	} *__p = (__typeof__(__p)) (ptr);	\
	__p->__v;				\
})

#define L_PUT_UNALIGNED(val, ptr)		\
do {						\
	struct __attribute__((packed)) {	\
		__typeof__(*(ptr)) __v;		\
	} *__p = (__typeof__(__p)) (ptr);	\
	__p->__v = (val);			\
} while(0)

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define L_LE16_TO_CPU(val) (val)
#define L_LE32_TO_CPU(val) (val)
#define L_LE64_TO_CPU(val) (val)
#define L_CPU_TO_LE16(val) (val)
#define L_CPU_TO_LE32(val) (val)
#define L_CPU_TO_LE64(val) (val)
#define L_BE16_TO_CPU(val) bswap_16(val)
#define L_BE32_TO_CPU(val) bswap_32(val)
#define L_BE64_TO_CPU(val) bswap_64(val)
#define L_CPU_TO_BE16(val) bswap_16(val)
#define L_CPU_TO_BE32(val) bswap_32(val)
#define L_CPU_TO_BE64(val) bswap_64(val)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define L_LE16_TO_CPU(val) bswap_16(val)
#define L_LE32_TO_CPU(val) bswap_32(val)
#define L_LE64_TO_CPU(val) bswap_64(val)
#define L_CPU_TO_LE16(val) bswap_16(val)
#define L_CPU_TO_LE32(val) bswap_32(val)
#define L_CPU_TO_LE64(val) bswap_64(val)
#define L_BE16_TO_CPU(val) (val)
#define L_BE32_TO_CPU(val) (val)
#define L_BE64_TO_CPU(val) (val)
#define L_CPU_TO_BE16(val) (val)
#define L_CPU_TO_BE32(val) (val)
#define L_CPU_TO_BE64(val) (val)
#else
#error "Unknown byte order"
#endif

#if __STDC_VERSION__ <= 199409L
#define inline __inline__
#endif

static inline uint8_t l_get_u8(const void *ptr)
{
	return *((const uint8_t *) ptr);
}

static inline void l_put_u8(uint8_t val, void *ptr)
{
	*((uint8_t *) ptr) = val;
}

static inline uint16_t l_get_u16(const void *ptr)
{
	return L_GET_UNALIGNED((const uint16_t *) ptr);
}

static inline void l_put_u16(uint16_t val, void *ptr)
{
	L_PUT_UNALIGNED(val, (uint16_t *) ptr);
}

static inline uint32_t l_get_u32(const void *ptr)
{
	return L_GET_UNALIGNED((const uint32_t *) ptr);
}

static inline void l_put_u32(uint32_t val, void *ptr)
{
	L_PUT_UNALIGNED(val, (uint32_t *) ptr);
}

static inline uint64_t l_get_u64(const void *ptr)
{
	return L_GET_UNALIGNED((const uint64_t *) ptr);
}

static inline void l_put_u64(uint64_t val, void *ptr)
{
	L_PUT_UNALIGNED(val, (uint64_t *) ptr);
}

static inline int16_t l_get_s16(const void *ptr)
{
	return L_GET_UNALIGNED((const int16_t *) ptr);
}

static inline int32_t l_get_s32(const void *ptr)
{
	return L_GET_UNALIGNED((const int32_t *) ptr);
}

static inline int64_t l_get_s64(const void *ptr)
{
	return L_GET_UNALIGNED((const int64_t *) ptr);
}

static inline uint16_t l_get_le16(const void *ptr)
{
	return L_LE16_TO_CPU(L_GET_UNALIGNED((const uint16_t *) ptr));
}

static inline uint16_t l_get_be16(const void *ptr)
{
	return L_BE16_TO_CPU(L_GET_UNALIGNED((const uint16_t *) ptr));
}

static inline uint32_t l_get_le32(const void *ptr)
{
	return L_LE32_TO_CPU(L_GET_UNALIGNED((const uint32_t *) ptr));
}

static inline uint32_t l_get_be32(const void *ptr)
{
	return L_BE32_TO_CPU(L_GET_UNALIGNED((const uint32_t *) ptr));
}

static inline uint64_t l_get_le64(const void *ptr)
{
	return L_LE64_TO_CPU(L_GET_UNALIGNED((const uint64_t *) ptr));
}

static inline uint64_t l_get_be64(const void *ptr)
{
	return L_BE64_TO_CPU(L_GET_UNALIGNED((const uint64_t *) ptr));
}

static inline void l_put_le16(uint16_t val, void *ptr)
{
	L_PUT_UNALIGNED(L_CPU_TO_LE16(val), (uint16_t *) ptr);
}

static inline void l_put_be16(uint16_t val, const void *ptr)
{
	L_PUT_UNALIGNED(L_CPU_TO_BE16(val), (uint16_t *) ptr);
}

static inline void l_put_le32(uint32_t val, void *ptr)
{
	L_PUT_UNALIGNED(L_CPU_TO_LE32(val), (uint32_t *) ptr);
}

static inline void l_put_be32(uint32_t val, void *ptr)
{
	L_PUT_UNALIGNED(L_CPU_TO_BE32(val), (uint32_t *) ptr);
}

static inline void l_put_le64(uint64_t val, void *ptr)
{
	L_PUT_UNALIGNED(L_CPU_TO_LE64(val), (uint64_t *) ptr);
}

static inline void l_put_be64(uint64_t val, void *ptr)
{
	L_PUT_UNALIGNED(L_CPU_TO_BE64(val), (uint64_t *) ptr);
}

#define L_AUTO_FREE_VAR(vartype,varname) \
	vartype varname __attribute__((cleanup(auto_free)))

#define L_ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

void *l_malloc(size_t size) __attribute__ ((warn_unused_result, malloc));
void *l_memdup(const void *mem, size_t size)
			__attribute__ ((warn_unused_result, malloc));

static inline void * __attribute__((nonnull(1))) l_memcpy(void *dest,
						const void *src, size_t n)
{
	if (!n)
		return dest;

	return __builtin_memcpy(dest, src, n);
}

void l_free(void *ptr);
DEFINE_CLEANUP_FUNC(l_free);

void *l_realloc(void *mem, size_t size)
			__attribute__ ((warn_unused_result, malloc));

static inline void auto_free(void *a)
{
	void **p = (void **)a;
	l_free(*p);
}

#define l_steal_ptr(ptr) \
	(__extension__ ({ typeof(ptr) _tmp = (ptr); (ptr) = NULL; _tmp; }))

/**
 * l_new:
 * @type: type of structure
 * @count: amount of structures
 *
 * Returns: pointer to allocated memory
 **/
#define l_new(type, count)			\
	(type *) (__extension__ ({		\
		size_t __n = (size_t) (count);	\
		size_t __s = sizeof(type);	\
		void *__p;			\
		__p = l_malloc(__n * __s);	\
		memset(__p, 0, __n * __s);	\
		__p;				\
	}))

/**
 * l_newa:
 * @type: type of structure
 * @count: amount of structures
 *
 * Allocates stack space for @count structures of @type.  Memory is allocated
 * using alloca and initialized to 0.
 *
 * Returns: Pointer to memory allocated on the stack.
 */
#define l_newa(type, count)			\
	(type *) (__extension__ ({		\
		size_t __n = (size_t) (count);	\
		size_t __s = sizeof(type);	\
		void *__p;			\
		__p = alloca(__n * __s);	\
		memset(__p, 0, __n * __s);	\
		__p;				\
	}))

char *l_strdup(const char *str);
char *l_strndup(const char *str, size_t max);
char *l_strdup_printf(const char *format, ...)
			__attribute__((format(printf, 1, 2)));
char *l_strdup_vprintf(const char *format, va_list args)
			__attribute__((format(printf, 1, 0)));

size_t l_strlcpy(char* dst, const char *src, size_t len);

bool l_str_has_prefix(const char *str, const char *prefix);
bool l_str_has_suffix(const char *str, const char *suffix);
bool l_streq0(const char *a, const char *b);

char *l_util_oidstring(const void *buf, size_t len);

char *l_util_hexstring(const void *buf, size_t len);
char *l_util_hexstring_upper(const void *buf, size_t len);
char *l_util_hexstringv(const struct iovec *iov, size_t n_iov);
char *l_util_hexstringv_upper(const struct iovec *iov, size_t n_iov);
unsigned char *l_util_from_hexstring(const char *str, size_t *out_len);

typedef void (*l_util_hexdump_func_t) (const char *str, void *user_data);

void l_util_hexdump(bool in, const void *buf, size_t len,
			l_util_hexdump_func_t function, void *user_data);
void l_util_hexdump_two(bool in, const void *buf1, size_t len1,
			const void *buf2, size_t len2,
			l_util_hexdump_func_t function, void *user_data);
void l_util_hexdumpv(bool in, const struct iovec *iov, size_t n_iov,
					l_util_hexdump_func_t function,
					void *user_data);
void l_util_debug(l_util_hexdump_func_t function, void *user_data,
						const char *format, ...)
			__attribute__((format(printf, 3, 4)));

const char *l_util_get_debugfs_path(void);

#define L_TFR(expression)                          \
  (__extension__                                   \
    ({ long int __result;                          \
       do __result = (long int) (expression);      \
       while (__result == -1L && errno == EINTR);  \
       __result; }))

/* Enables declaring _auto_(close) int fd = <-1 or L_TFR(open(...))>; */
inline __attribute__((always_inline)) void _l_close_cleanup(void *p)
{
	int fd = *(int *) p;

	if (fd >= 0)
		L_TFR(close(fd));
}

#define _L_IN_SET_CMP(val, type, cmp, ...) __extension__ ({		\
		const type __v = (val);					\
		const typeof(__v) __elems[] = {__VA_ARGS__};		\
		size_t __i;						\
		const size_t __n = L_ARRAY_SIZE(__elems);		\
		bool __r = false;					\
		for (__i = 0; __i < __n && !__r; __i++)			\
			__r = (cmp);					\
		__r;							\
	})

/* Warning: evaluates all set elements even after @val has matched one */
#define L_IN_SET(val, ...)	\
	_L_IN_SET_CMP((val), __auto_type, __v == __elems[__i], ##__VA_ARGS__)

#define L_IN_STRSET(val, ...)						\
	_L_IN_SET_CMP((val), char *, __v == __elems[__i] ||		\
				(__v && __elems[__i] &&			\
				 !strcmp(__v, __elems[__i])), ##__VA_ARGS__)

#define _L_BIT_TO_MASK(bits, nr) __extension__ ({			\
	typeof(*(bits)) _one = 1U;					\
	const unsigned int _shift = (nr) % (sizeof(_one) * 8);		\
	_one << _shift;							\
})

#define _L_BIT_TO_OFFSET(bits, nr) __extension__ ({			\
	__auto_type _bits = (bits);					\
	const size_t _offset = (nr) / (sizeof(*_bits) * 8);		\
	_bits + _offset;						\
})

#define L_BIT_SET(bits, nr) __extension__ ({				\
	size_t _nr = (nr);						\
	__auto_type _offset = _L_BIT_TO_OFFSET(bits, _nr);		\
	*_offset |= _L_BIT_TO_MASK(_offset, _nr);			\
})

#define L_BIT_CLEAR(bits, nr) __extension__ ({				\
	size_t _nr = (nr);						\
	__auto_type _offset = _L_BIT_TO_OFFSET(bits, _nr);		\
	*_offset &= ~_L_BIT_TO_MASK(_offset, _nr);			\
})

#define L_BIT_TEST(bits, nr) __extension__ ({				\
	size_t _nr = (nr);						\
	__auto_type _offset = _L_BIT_TO_OFFSET(bits, _nr);		\
	(*_offset & _L_BIT_TO_MASK(_offset, _nr)) != 0;			\
})

#define L_BITS_SET(bits, ...) __extension__ ({				\
	const unsigned int __elems[] = {__VA_ARGS__};			\
	size_t __i;							\
	for (__i = 0; __i < L_ARRAY_SIZE(__elems); __i++)		\
		L_BIT_SET(bits, __elems[__i]);				\
})

#define L_BITS_CLEAR(bits, ...) __extension__ ({			\
	const unsigned int __elems[] = {__VA_ARGS__};			\
	size_t __i;							\
	for (__i = 0; __i < L_ARRAY_SIZE(__elems); __i++)		\
		L_BIT_CLEAR(bits, __elems[__i]);			\
})

/*
 * Taken from https://github.com/chmike/cst_time_memcmp, adding a volatile to
 * ensure the compiler does not try to optimize the constant time behavior.
 * The code has been modified to add comments and project specific code
 * styling.
 * This specific piece of code is subject to the following copyright:
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Christophe Meessen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * This function performs a secure memory comparison of two buffers of size
 * bytes, representing an integer (byte order is big endian). It returns
 * a negative, zero or positif value if a < b, a == b or a > b respectively.
 */
static inline int l_secure_memcmp(const void *a, const void *b,
					size_t size)
{
	const volatile uint8_t *aa =
		L_PERMISSIVE_CAST(const volatile uint8_t *, a);
	const volatile uint8_t *bb =
		L_PERMISSIVE_CAST(const volatile uint8_t *, b);
	int res = 0, diff, mask;

	/*
	 * We will compare all bytes, starting with the less significant. When
	 * we find a non-zero difference, we update the result accordingly.
	 */
	if (size > 0) {
		/*
		 * The following couple of lines can be summarized as a
		 * constant time/memory access version of:
		 * if (diff != 0) res = diff;
		 *
		 * From the previous operation, we know that diff is in
		 * [-255, 255]
		 *
		 * The following figure show the possible value of mask, based
		 * on different cases of diff:
		 *
		 * diff  |   diff-1   |   ~diff    | ((diff-1) & ~diff) |  mask
		 * ------|------------|------------|--------------------|------
		 *   < 0 | 0xFFFFFFXX | 0x000000YY |     0x000000ZZ     |   0
		 *  == 0 | 0xFFFFFFFF | 0xFFFFFFFF |     0xFFFFFFFF     | 0xF..F
		 *  > 0  | 0x000000XX | 0xFFFFFFYY |     0x000000ZZ     |   0
		 *
		 * Hence, the mask allows to keep res when diff == 0, and to
		 * set res to diff otherwise.
		*/
		do {
			--size;
			diff = aa[size] - bb[size];
			mask = (((diff - 1) & ~diff) >> 8);
			res = (res & mask) | diff;
		} while (size != 0);
	}

	return res;
}

bool l_memeq(const void *field, size_t size, uint8_t byte);
bool l_secure_memeq(const void *field, size_t size, uint8_t byte);

static inline bool l_memeqzero(const void *field, size_t size)
{
	return l_memeq(field, size, 0);
}

static inline void l_secure_select(bool select_left,
				const void *left, const void *right,
				void *out, size_t len)
{
	const uint8_t *l = L_PERMISSIVE_CAST(const uint8_t *, left);
	const uint8_t *r = L_PERMISSIVE_CAST(const uint8_t *, right);
	uint8_t *o = L_PERMISSIVE_CAST(uint8_t *, out);
	uint8_t mask = -(!!select_left);
	size_t i;

	for (i = 0; i < len; i++)
		o[i] = r[i] ^ ((l[i] ^ r[i]) & mask);
}

int l_safe_atou32(const char *s, uint32_t *out_u);
int l_safe_atox32(const char *s, uint32_t *out_u);
int l_safe_atox16(const char *s, uint16_t *out_u);
int l_safe_atox8(const char *s, uint8_t *out_u);

size_t l_util_pagesize(void);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_UTIL_H */
