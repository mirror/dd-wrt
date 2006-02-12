/* $Id: bitops.h,v 1.39 2002/01/30 01:40:00 davem Exp $
 * bitops.h: Bit string operations on the V9.
 *
 * Copyright 1996, 1997 David S. Miller (davem@caip.rutgers.edu)
 */

#ifndef _SPARC64_BITOPS_H
#define _SPARC64_BITOPS_H

#include <linux/config.h>
#include <linux/compiler.h>
#include <asm/byteorder.h>

extern int test_and_set_bit(unsigned long nr, volatile void *addr);
extern int test_and_clear_bit(unsigned long nr, volatile void *addr);
extern int test_and_change_bit(unsigned long nr, volatile void *addr);
extern void set_bit(unsigned long nr, volatile void *addr);
extern void clear_bit(unsigned long nr, volatile void *addr);
extern void change_bit(unsigned long nr, volatile void *addr);

/* "non-atomic" versions... */

static __inline__ void __set_bit(int nr, volatile void *addr)
{
	unsigned long *m;

	m = ((unsigned long *)addr) + (nr >> 6);
	*m |= (1UL << (nr & 63));
}

static __inline__ void __clear_bit(int nr, volatile void *addr)
{
	unsigned long *m;

	m = ((unsigned long *)addr) + (nr >> 6);
	*m &= ~(1UL << (nr & 63));
}

static __inline__ void __change_bit(int nr, volatile void *addr)
{
	unsigned long *m;

	m = ((unsigned long *)addr) + (nr >> 6);
	*m ^= (1UL << (nr & 63));
}

static __inline__ int __test_and_set_bit(int nr, volatile void *addr)
{
	unsigned long *m = ((unsigned long *)addr) + (nr >> 6);
	unsigned long old = *m;
	unsigned long mask = (1UL << (nr & 63));

	*m = (old | mask);
	return ((old & mask) != 0);
}

static __inline__ int __test_and_clear_bit(int nr, volatile void *addr)
{
	unsigned long *m = ((unsigned long *)addr) + (nr >> 6);
	unsigned long old = *m;
	unsigned long mask = (1UL << (nr & 63));

	*m = (old & ~mask);
	return ((old & mask) != 0);
}

static __inline__ int __test_and_change_bit(int nr, volatile void *addr)
{
	unsigned long *m = ((unsigned long *)addr) + (nr >> 6);
	unsigned long old = *m;
	unsigned long mask = (1UL << (nr & 63));

	*m = (old ^ mask);
	return ((old & mask) != 0);
}

#ifdef CONFIG_SMP
#define smp_mb__before_clear_bit()	membar("#StoreLoad | #LoadLoad")
#define smp_mb__after_clear_bit()	membar("#StoreLoad | #StoreStore")
#else
#define smp_mb__before_clear_bit()	barrier()
#define smp_mb__after_clear_bit()	barrier()
#endif

static __inline__ int test_bit(int nr, __const__ volatile void *_addr)
{
	__const__ unsigned long *addr;

	addr = (__const__ unsigned long *) _addr;

	return (1UL & ((addr)[nr >> 6] >> (nr & 63))) != 0UL;
}

/* The easy/cheese version for now. */
static __inline__ unsigned long ffz(unsigned long word)
{
	unsigned long result;

	result = 0;
	while(word & 1) {
		result++;
		word >>= 1;
	}
	return result;
}

/**
 * __ffs - find first bit in word.
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static __inline__ unsigned long __ffs(unsigned long word)
{
	unsigned long result = 0;

	while (!(word & 1UL)) {
		result++;
		word >>= 1;
	}
	return result;
}

/*
 * fls: find last bit set.
 */

#define fls(x) generic_fls(x)

#ifdef __KERNEL__

/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */
static __inline__ int ffs(int x)
{
	if (!x)
		return 0;
	return __ffs((unsigned long)x) + 1;
}

/*
 * hweightN: returns the hamming weight (i.e. the number
 * of bits set) of a N-bit word
 */

#ifdef ULTRA_HAS_POPULATION_COUNT

static __inline__ unsigned int hweight64(unsigned long w)
{
	unsigned int res;

	__asm__ ("popc %1,%0" : "=r" (res) : "r" (w));
	return res;
}

static __inline__ unsigned int hweight32(unsigned int w)
{
	unsigned int res;

	__asm__ ("popc %1,%0" : "=r" (res) : "r" (w & 0xffffffff));
	return res;
}

static __inline__ unsigned int hweight16(unsigned int w)
{
	unsigned int res;

	__asm__ ("popc %1,%0" : "=r" (res) : "r" (w & 0xffff));
	return res;
}

static __inline__ unsigned int hweight8(unsigned int w)
{
	unsigned int res;

	__asm__ ("popc %1,%0" : "=r" (res) : "r" (w & 0xff));
	return res;
}

#else

#define hweight64(x) generic_hweight64(x)
#define hweight32(x) generic_hweight32(x)
#define hweight16(x) generic_hweight16(x)
#define hweight8(x) generic_hweight8(x)

#endif
#endif /* __KERNEL__ */

/* find_next_zero_bit() finds the first zero bit in a bit string of length
 * 'size' bits, starting the search at bit 'offset'. This is largely based
 * on Linus's ALPHA routines, which are pretty portable BTW.
 */

static __inline__ unsigned long find_next_zero_bit(void *addr, unsigned long size, unsigned long offset)
{
	unsigned long *p = ((unsigned long *) addr) + (offset >> 6);
	unsigned long result = offset & ~63UL;
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= 63UL;
	if (offset) {
		tmp = *(p++);
		tmp |= ~0UL >> (64-offset);
		if (size < 64)
			goto found_first;
		if (~tmp)
			goto found_middle;
		size -= 64;
		result += 64;
	}
	while (size & ~63UL) {
		if (~(tmp = *(p++)))
			goto found_middle;
		result += 64;
		size -= 64;
	}
	if (!size)
		return result;
	tmp = *p;

found_first:
	tmp |= ~0UL << size;
	if (tmp == ~0UL)        /* Are any bits zero? */
		return result + size; /* Nope. */
found_middle:
	return result + ffz(tmp);
}

#define find_first_zero_bit(addr, size) \
        find_next_zero_bit((addr), (size), 0)

#define test_and_set_le_bit(nr,addr)	\
	test_and_set_bit((nr) ^ 0x38, (addr))
#define test_and_clear_le_bit(nr,addr)	\
	test_and_clear_bit((nr) ^ 0x38, (addr))

static __inline__ int test_le_bit(int nr, __const__ void *addr)
{
	int			mask;
	__const__ unsigned char	*ADDR = (__const__ unsigned char *) addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	return ((mask & *ADDR) != 0);
}

#define find_first_zero_le_bit(addr, size) \
        find_next_zero_le_bit((addr), (size), 0)

static __inline__ unsigned long find_next_zero_le_bit(void *addr, unsigned long size, unsigned long offset)
{
	unsigned long *p = ((unsigned long *) addr) + (offset >> 6);
	unsigned long result = offset & ~63UL;
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= 63UL;
	if(offset) {
		tmp = __swab64p(p++);
		tmp |= (~0UL >> (64-offset));
		if(size < 64)
			goto found_first;
		if(~tmp)
			goto found_middle;
		size -= 64;
		result += 64;
	}
	while(size & ~63) {
		if(~(tmp = __swab64p(p++)))
			goto found_middle;
		result += 64;
		size -= 64;
	}
	if(!size)
		return result;
	tmp = __swab64p(p);
found_first:
	tmp |= (~0UL << size);
	if (tmp == ~0UL)        /* Are any bits zero? */
		return result + size; /* Nope. */
found_middle:
	return result + ffz(tmp);
}

#ifdef __KERNEL__

#define __set_le_bit(nr, addr) \
	__set_bit((nr) ^ 0x38, (addr))
#define __clear_le_bit(nr, addr) \
	__clear_bit((nr) ^ 0x38, (addr))
#define __test_and_clear_le_bit(nr, addr) \
	__test_and_clear_bit((nr) ^ 0x38, (addr))
#define __test_and_set_le_bit(nr, addr) \
	__test_and_set_bit((nr) ^ 0x38, (addr))

#define ext2_set_bit(nr,addr)	\
	__test_and_set_le_bit((nr),(unsigned long *)(addr))
#define ext2_set_bit_atomic(lock,nr,addr) \
	test_and_set_le_bit((nr),(unsigned long *)(addr))
#define ext2_clear_bit(nr,addr)	\
	__test_and_clear_le_bit((nr),(unsigned long *)(addr))
#define ext2_clear_bit_atomic(lock,nr,addr) \
	test_and_clear_le_bit((nr),(unsigned long *)(addr))
#define ext2_test_bit(nr,addr)	\
	test_le_bit((nr),(unsigned long *)(addr))
#define ext2_find_first_zero_bit(addr, size) \
	find_first_zero_le_bit((unsigned long *)(addr), (size))
#define ext2_find_next_zero_bit(addr, size, off) \
	find_next_zero_le_bit((unsigned long *)(addr), (size), (off))

/* Bitmap functions for the minix filesystem.  */
#define minix_test_and_set_bit(nr,addr)	\
	test_and_set_bit((nr),(unsigned long *)(addr))
#define minix_set_bit(nr,addr)	\
	set_bit((nr),(unsigned long *)(addr))
#define minix_test_and_clear_bit(nr,addr) \
	test_and_clear_bit((nr),(unsigned long *)(addr))
#define minix_test_bit(nr,addr)	\
	test_bit((nr),(unsigned long *)(addr))
#define minix_find_first_zero_bit(addr,size) \
	find_first_zero_bit((unsigned long *)(addr),(size))

#endif /* __KERNEL__ */

#endif /* defined(_SPARC64_BITOPS_H) */
