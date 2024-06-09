/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Cryptographic utilities
 *
 * Copyright (c) 2023 Herbert Xu <herbert@gondor.apana.org.au>
 */
#ifndef _CRYPTO_UTILS_H
#define _CRYPTO_UTILS_H

#include <asm/unaligned.h>
#include <linux/bitops.h>
#include <linux/compiler_attributes.h>
#include <linux/types.h>

/*
 * XOR @len bytes from @src1 and @src2 together, writing the result to @dst
 * (which may alias one of the sources).  Don't call this directly; call
 * crypto_xor() or crypto_xor_cpy() instead.
 */
static inline void __crypto_xor(u8 *dst, const u8 *src1, const u8 *src2, unsigned int len)
{
	int relalign = 0;

	if (!IS_ENABLED(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)) {
		int size = sizeof(unsigned long);
		int d = (((unsigned long)dst ^ (unsigned long)src1) |
			 ((unsigned long)dst ^ (unsigned long)src2)) &
			(size - 1);

		relalign = d ? 1 << __ffs(d) : size;

		/*
		 * If we care about alignment, process as many bytes as
		 * needed to advance dst and src to values whose alignments
		 * equal their relative alignment. This will allow us to
		 * process the remainder of the input using optimal strides.
		 */
		while (((unsigned long)dst & (relalign - 1)) && len > 0) {
			*dst++ = *src1++ ^ *src2++;
			len--;
		}
	}

	while (IS_ENABLED(CONFIG_64BIT) && len >= 8 && !(relalign & 7)) {
		if (IS_ENABLED(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)) {
			u64 l = get_unaligned((u64 *)src1) ^
				get_unaligned((u64 *)src2);
			put_unaligned(l, (u64 *)dst);
		} else {
			*(u64 *)dst = *(u64 *)src1 ^ *(u64 *)src2;
		}
		dst += 8;
		src1 += 8;
		src2 += 8;
		len -= 8;
	}

	while (len >= 4 && !(relalign & 3)) {
		if (IS_ENABLED(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)) {
			u32 l = get_unaligned((u32 *)src1) ^
				get_unaligned((u32 *)src2);
			put_unaligned(l, (u32 *)dst);
		} else {
			*(u32 *)dst = *(u32 *)src1 ^ *(u32 *)src2;
		}
		dst += 4;
		src1 += 4;
		src2 += 4;
		len -= 4;
	}

	while (len >= 2 && !(relalign & 1)) {
		if (IS_ENABLED(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)) {
			u16 l = get_unaligned((u16 *)src1) ^
				get_unaligned((u16 *)src2);
			put_unaligned(l, (u16 *)dst);
		} else {
			*(u16 *)dst = *(u16 *)src1 ^ *(u16 *)src2;
		}
		dst += 2;
		src1 += 2;
		src2 += 2;
		len -= 2;
	}

	while (len--)
		*dst++ = *src1++ ^ *src2++;
}

static inline void crypto_xor(u8 *dst, const u8 *src, unsigned int size)
{
	if (IS_ENABLED(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS) &&
	    __builtin_constant_p(size) &&
	    (size % sizeof(unsigned long)) == 0) {
		unsigned long *d = (unsigned long *)dst;
		unsigned long *s = (unsigned long *)src;
		unsigned long l;

		while (size > 0) {
			l = get_unaligned(d) ^ get_unaligned(s++);
			put_unaligned(l, d++);
			size -= sizeof(unsigned long);
		}
	} else {
		__crypto_xor(dst, dst, src, size);
	}
}

static inline void crypto_xor_cpy(u8 *dst, const u8 *src1, const u8 *src2,
				  unsigned int size)
{
	if (IS_ENABLED(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS) &&
	    __builtin_constant_p(size) &&
	    (size % sizeof(unsigned long)) == 0) {
		unsigned long *d = (unsigned long *)dst;
		unsigned long *s1 = (unsigned long *)src1;
		unsigned long *s2 = (unsigned long *)src2;
		unsigned long l;

		while (size > 0) {
			l = get_unaligned(s1++) ^ get_unaligned(s2++);
			put_unaligned(l, d++);
			size -= sizeof(unsigned long);
		}
	} else {
		__crypto_xor(dst, src1, src2, size);
	}
}

noinline unsigned long __crypto_memneq(const void *a, const void *b, size_t size);

/**
 * crypto_memneq - Compare two areas of memory without leaking
 *		   timing information.
 *
 * @a: One area of memory
 * @b: Another area of memory
 * @size: The size of the area.
 *
 * Returns 0 when data is equal, 1 otherwise.
 */
static inline int crypto_memneq(const void *a, const void *b, size_t size)
{
	return __crypto_memneq(a, b, size) != 0UL ? 1 : 0;
}

#endif	/* _CRYPTO_UTILS_H */
