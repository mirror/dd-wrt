/*
 * Copyright (C) 2000, 2004  Maciej W. Rozycki
 * Copyright (C) 2003, 07 Ralf Baechle (ralf@linux-mips.org)
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#ifndef _ASM_DIV64_H
#define _ASM_DIV64_H

#include <linux/types.h>

#if (_MIPS_SZLONG == 32)

#include <asm/compiler.h>
extern uint32_t __div64_32(uint64_t *dividend, uint32_t divisor);

# define do_div(n,base) ({				\
	uint32_t __base = (base);			\
	uint32_t __rem;					\
	(void)(((typeof((n)) *)0) == ((uint64_t *)0));	\
	if (likely(((n) >> 32) == 0)) {			\
		__rem = (uint32_t)(n) % __base;		\
		(n) = (uint32_t)(n) / __base;		\
	} else 						\
		__rem = __div64_32(&(n), __base);	\
	__rem;						\
 })

extern uint64_t div64_64(uint64_t dividend, uint64_t divisor);
#endif /* (_MIPS_SZLONG == 32) */

#if (_MIPS_SZLONG == 64)

/*
 * Hey, we're already 64-bit, no
 * need to play games..
 */
#define do_div(n, base) ({ \
	unsigned long __quot; \
	unsigned int __mod; \
	unsigned long __div; \
	unsigned int __base; \
	\
	__div = (n); \
	__base = (base); \
	\
	__mod = __div % __base; \
	__quot = __div / __base; \
	\
	(n) = __quot; \
	__mod; })

static inline uint64_t div64_64(uint64_t dividend, uint64_t divisor)
{
	return dividend / divisor;
}

#endif /* (_MIPS_SZLONG == 64) */

#endif /* _ASM_DIV64_H */
