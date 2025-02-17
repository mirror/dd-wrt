/*
 * Embedded Linux library
 * Copyright (C) 2021  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define align_len(len, boundary) (((len)+(boundary)-1) & ~((boundary)-1))

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define SWAP(l, r) \
	do { typeof(l) __tmp = (l); (l) = (r); (r) = __tmp; } while (0)

#ifndef __always_inline
#define __always_inline inline __attribute__((always_inline))
#endif

static inline size_t minsize(size_t a, size_t b)
{
	if (a <= b)
		return a;

	return b;
}

static inline size_t maxsize(size_t a, size_t b)
{
	if (a >= b)
		return a;

	return b;
}

static inline void set_bit(void *addr, unsigned int bit)
{
	unsigned char *field = addr;
	field[bit / 8] |= 1U << (bit % 8);
}

static inline int test_bit(const void *addr, unsigned int bit)
{
	const unsigned char *field = addr;
	return (field[bit / 8] & (1U << (bit % 8))) != 0;
}

static inline unsigned char bit_field(const unsigned char oct,
					unsigned int start, unsigned int n_bits)
{
	unsigned char mask = (1U << n_bits) - 1U;
	return (oct >> start) & mask;
}

/* Must be called with n >= 2 and n <= ULONG_MAX / 2 + 1 */
static inline unsigned long roundup_pow_of_two(unsigned long n)
{
	return 1UL << (sizeof(unsigned long) * 8 - __builtin_clzl(n - 1));
}

#define DIV_ROUND_CLOSEST(x, divisor)			\
({							\
	typeof(divisor) _d = (divisor);			\
	typeof(x) _x = (x) + _d / 2;			\
	_x / _d;					\
})

#ifndef _auto_
#define _auto_(func)					\
	__L_AUTODESTRUCT(func)
#endif

/*
 * Trick the compiler into thinking that var might be changed somehow by
 * the asm
 */
#define DO_NOT_OPTIMIZE(var) \
	__asm__ ("" : "=r" (var) : "0" (var));

static inline int secure_select(int select_left, int l, int r)
{
	int mask = -(!!select_left);

	return r ^ ((l ^ r) & mask);
}

#define struct_alloc(structname, ...) \
	(struct structname *) l_memdup(&(struct structname) { __VA_ARGS__ }, \
					sizeof(struct structname))
