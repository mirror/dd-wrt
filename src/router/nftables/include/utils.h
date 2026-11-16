#ifndef NFTABLES_UTILS_H
#define NFTABLES_UTILS_H

#include <asm/byteorder.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <list.h>
#include <gmputil.h>

#ifdef HAVE_VISIBILITY_HIDDEN
#       define __visible        __attribute__((visibility("default")))
#       define EXPORT_SYMBOL(x) typeof(x) (x) __visible;
#else
#       define EXPORT_SYMBOL
#endif

#define BITS_PER_BYTE	8

#define pr_debug(fmt, arg...) printf(fmt, ##arg)

#if defined(HAVE_LIBGMP)
#define pr_gmp_debug(fmt, arg...) gmp_printf(fmt, ##arg)
#else
#define pr_gmp_debug(fmt, arg...) ({ if (false) {}; 0; })
#endif

#define expr_print_debug(expr)					\
{								\
	struct output_ctx octx = { .output_fp = stdout };	\
	printf("%s:%d: ", __FILE__, __LINE__);			\
	printf("'" #expr "' type %s: ", expr_name(expr));	\
	expr_print((expr), &octx);				\
	printf("\n");						\
}

#define __fmtstring(x, y)	__attribute__((format(printf, x, y)))

#define __must_check		__attribute__((warn_unused_result))
#define __noreturn		__attribute__((__noreturn__))

#define BUG(fmt, arg...)	({ fprintf(stderr, "BUG: " fmt "\n", ##arg); assert(0); abort(); })

#define BUILD_BUG_ON(condition)	((void)sizeof(char[1 - 2*!!(condition)]))
#define BUILD_BUG_ON_ZERO(e)	(sizeof(char[1 - 2 * !!(e)]) - 1)

#define __must_be_array(a) \
	BUILD_BUG_ON_ZERO(__builtin_types_compatible_p(typeof(a), typeof(&a[0])))

#define assert_refcount_safe(refcnt) do {						\
	if ((refcnt) == 0)								\
		BUG("refcount was 0");							\
	if ((refcnt) >= INT_MAX)							\
		BUG("refcount saturated");						\
} while (0)

#define container_of(ptr, type, member) ({			\
	typeof( ((type *)0)->member ) *__mptr = (ptr);		\
	(type *)( (void *)__mptr - offsetof(type,member) );})

/**
 * Return a pointer to a constant variable of a size smaller than the variable.
 */
#ifdef __LITTLE_ENDIAN_BITFIELD
#define constant_data_ptr(val, len) \
	((void *)&(val))
#elif defined(__BIG_ENDIAN_BITFIELD)
#define constant_data_ptr(val, len) \
	((void *)&(val) + sizeof(val) - div_round_up(len, BITS_PER_BYTE))
#else
#error "byteorder undefined"
#endif

#define field_sizeof(t, f)	(sizeof(((t *)NULL)->f))
#define array_size(arr)		(sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))
#define div_round_up(n, d)	(((n) + (d) - 1) / (d))
#define round_up(n, b)		(div_round_up(n, b) * b)

#define min(_x, _y) ({				\
	_x < _y ? _x : _y; })

#define max(_x, _y) ({				\
	_x > _y ? _x : _y; })

#define SNPRINTF_BUFFER_SIZE(ret, len, offset)			\
	({ \
		const int _ret = (ret);				\
		size_t *const _len = (len);			\
		size_t *const _offset = (offset);		\
		bool _not_truncated = true;			\
		size_t _ret2;					\
								\
		assert(_ret >= 0);				\
								\
		if ((size_t) _ret >= *_len) {			\
			/* Truncated.
			 *
			 * We will leave "len" at zero and increment
			 * "offset" to point one byte after the buffer
			 * (after the terminating NUL byte). */	\
			_ret2 = *_len;				\
			_not_truncated = false;			\
		} else						\
			_ret2 = (size_t) _ret;			\
								\
		*_offset += _ret2;				\
		*_len -= _ret2;					\
								\
		_not_truncated;					\
	})

#define MSEC_PER_SEC	1000L

/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static inline int fls(uint32_t x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

extern void __memory_allocation_error(const char *filename, uint32_t line) __noreturn;

#define memory_allocation_error()		\
	__memory_allocation_error(__FILE__, __LINE__);

extern void *xmalloc(size_t size);
extern void *xmalloc_array(size_t nmemb, size_t size);
extern void *xrealloc(void *ptr, size_t size);
extern void *xzalloc(size_t size);
extern void *xzalloc_array(size_t nmemb, size_t size);
extern char *xstrdup(const char *s);
extern void xstrunescape(const char *in, char *out);
extern int round_pow_2(unsigned int value);

#endif /* NFTABLES_UTILS_H */
