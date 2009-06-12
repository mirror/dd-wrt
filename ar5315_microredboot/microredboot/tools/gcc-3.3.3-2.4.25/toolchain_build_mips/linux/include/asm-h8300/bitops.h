#ifndef _H8300_BITOPS_H
#define _H8300_BITOPS_H

/*
 * Copyright 1992, Linus Torvalds.
 * Copyright 2002, Yoshinori Sato
 */

#include <linux/config.h>
#include <asm/byteorder.h>	/* swab32 */
#include <asm/system.h>

#ifdef __KERNEL__
/*
 * Function prototypes to keep gcc -Wall happy
 */

/*
 * ffz = Find First Zero in word. Undefined if no zero exists,
 * so code should check against ~0UL first..
 */
static __inline__ unsigned long ffz(unsigned long word)
{
	register unsigned long result asm("er0");
	register unsigned long _word asm("er1");

	_word = word;
	__asm__("sub.l er0,er0\n\t"
		"dec.l #1,er0\n"
		"1:\n\t"
		"shlr.l %1\n\t"
		"adds #1,er0\n\t"
		"bcs 1b\n\t"
		: "=r" (result) : "r" (_word));
	return result;
}

static __inline__ void set_bit(int nr, volatile void * addr)
{
	volatile unsigned char *b_addr;
	b_addr = &(((volatile unsigned char *) addr)
	          [((nr >> 3) & ~3) + 3 - ((nr >> 3) & 3)]);
	__asm__("mov.l %1,er0\n\t"
		"bset r0l,%0"
		:"+m"(*b_addr)
		:"g"(nr & 7),"m"(*b_addr)
		:"er0");
}

#define __set_bit(nr, addr) set_bit((nr), (addr))

/*
 * clear_bit() doesn't provide any barrier for the compiler.
 */
#define smp_mb__before_clear_bit()	barrier()
#define smp_mb__after_clear_bit()	barrier()

static __inline__ void clear_bit(int nr, volatile void * addr)
{
	volatile unsigned char *b_addr;
	b_addr = &(((volatile unsigned char *) addr)
	          [((nr >> 3) & ~3) + 3 - ((nr >> 3) & 3)]);
	__asm__("mov.l %1,er0\n\t"
		"bclr r0l,%0"
		:"+m"(*b_addr)
		:"g"(nr & 7),"m"(*b_addr)
		:"er0");
}

#define __clear_bit(nr, addr) clear_bit((nr),(addr))

static __inline__ void change_bit(int nr, volatile void * addr)
{
	volatile unsigned char *b_addr;
	b_addr = &(((volatile unsigned char *) addr)
	          [((nr >> 3) & ~3) + 3 - ((nr >> 3) & 3)]);
	__asm__("mov.l %1,er0\n\t"
		"bnot r0l,%0"
		:"+m"(*b_addr)
		:"g"(nr & 7),"m"(*b_addr)
		:"er0");
}

#define __change_bit(nr, addr) change_bit((nr), (addr))

/*
 * This routine doesn't need to be atomic.
 */

static __inline__ int test_bit(int nr, const volatile void * addr)
{
	return ((1UL << (nr & 7)) & 
               (((const volatile unsigned char *) addr)
               [((nr >> 3) & ~3) + 3 - ((nr >> 3) & 3)])) != 0;
}

#define __test_bit(nr, addr) test_bit((nr), (addr))

static __inline__ int test_and_set_bit(int nr, volatile void * addr)
{
	register int retval __asm__("er0");
	register unsigned char *a __asm__("er1");
	a = (unsigned char *)addr;

	a += ((nr >> 3) & ~3) + (3 - ((nr >> 3) & 3));
	__asm__("mov.l %2,er3\n\t"
		"sub.l %0,%0\n\t"
		"stc ccr,r3h\n\t"
		"orc #0x80,ccr\n\t"
		"btst r3l,%1\n\t"
		"bset r3l,%1\n\t"
		"beq 1f\n\t"
		"inc.l #1,%0\n\t"
		"1:"
		"ldc r3h,ccr"
		: "=r"(retval),"=m"(*a) :"g"(nr & 7):"er3","memory");
	return retval;
}

static __inline__ int __test_and_set_bit(int nr, volatile void * addr)
{
	register int retval __asm__("er0");
	register unsigned char *a __asm__("er1");
	a = (unsigned char *)addr;

	a += ((nr >> 3) & ~3) + (3 - ((nr >> 3) & 3));
	__asm__("mov.l %2,er3\n\t"
		"sub.l %0,%0\n\t"
		"btst r3l,%2\n\t"
		"bset r3l,%2\n\t"
		"beq 1f\n\t"
		"inc.l #1,%0\n\t"
		"1:"
		: "=r"(retval),"=m"(*a) :"g"(nr & 7):"er3","memory");
	return retval;
}

static __inline__ int test_and_clear_bit(int nr, volatile void * addr)
{
	register int retval __asm__("er0");
	register unsigned char *a __asm__("er1");
	a = (unsigned char *)addr;

	a += ((nr >> 3) & ~3) + (3 - ((nr >> 3) & 3));
	__asm__("mov.l %2,er3\n\t"
		"sub.l %0,%0\n\t"
		"stc ccr,r3h\n\t"
		"orc #0x80,ccr\n\t"
		"btst r3l,%1\n\t"
		"bclr r3l,%1\n\t"
		"beq 1f\n\t"
		"inc.l #1,%0\n\t"
		"1:"
		"ldc r3h,ccr"
		: "=r"(retval),"=m"(*a) :"g"(nr & 7):"er3","memory");
	return retval;
}

static __inline__ int __test_and_clear_bit(int nr, volatile void * addr)
{
	register int retval __asm__("er0");
	register unsigned char *a __asm__("er1");
	a = (unsigned char *)addr;

	a += ((nr >> 3) & ~3) + (3 - ((nr >> 3) & 3));
	__asm__("mov.l %2,er3\n\t"
		"sub.l %0,%0\n\t"
		"btst r3l,%1\n\t"
		"bclr r3l,%1\n\t"
		"beq 1f\n\t"
		"inc.l #1,%0\n\t"
		"1:"
		: "=r"(retval),"=m"(*a) :"g"(nr & 7):"er3","memory");
	return retval;
}

static __inline__ int test_and_change_bit(int nr, volatile void * addr)
{
	register int retval __asm__("er0");
	register unsigned char *a __asm__("er1");
	a = (unsigned char *)addr;

	a += ((nr >> 3) & ~3) + (3 - ((nr >> 3) & 3));
	__asm__("mov.l %2,er3\n\t"
		"sub.l %0,%0\n\t"
		"stc ccr,r3h\n\t"
		"orc #0x80,ccr\n\t"
		"btst r3l,%1\n\t"
		"bnot r3l,%1\n\t"
		"beq 1f\n\t"
		"inc.l #1,%0\n\t"
		"1:"
		"ldc r3h,ccr"
		: "=r"(retval),"=m"(*a) :"g"(nr & 7):"er3","memory");
	return retval;
}

static __inline__ int __test_and_change_bit(int nr, volatile void * addr)
{
	register int retval __asm__("er0");
	register unsigned char *a __asm__("er1");
	a = (unsigned char *)addr;

	a += ((nr >> 3) & ~3) + (3 - ((nr >> 3) & 3));
	__asm__("mov.l %2,er3\n\t"
		"sub.l %0,%0\n\t"
		"btst r3l,%1\n\t"
		"bnot r3l,%1\n\t"
		"beq 1f\n\t"
		"inc.l #1,%0\n\t"
		"1:"
		: "=r"(retval),"=m"(*a) :"g"(nr & 7):"er3","memory");
	return retval;
}


#define find_first_zero_bit(addr, size) \
        find_next_zero_bit((addr), (size), 0)

static __inline__ int find_next_zero_bit (void * addr, int size, int offset)
{
	unsigned long *p = ((unsigned long *) addr) + (offset >> 5);
	unsigned long result = offset & ~31UL;
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= 31UL;
	if (offset) {
		tmp = *(p++);
		tmp |= ~0UL >> (32-offset);
		if (size < 32)
			goto found_first;
		if (~tmp)
			goto found_middle;
		size -= 32;
		result += 32;
	}
	while (size & ~31UL) {
		if (~(tmp = *(p++)))
			goto found_middle;
		result += 32;
		size -= 32;
	}
	if (!size)
		return result;
	tmp = *p;

found_first:
	tmp |= ~0UL >> size;
found_middle:
	return result + ffz(tmp);
}

static __inline__ unsigned long ffs(unsigned long word)
{
	unsigned long result;

	__asm__("sub.l %0,%0\n\t"
		"dec.l #1,%0\n"
		"1:\n\t"
		"shlr.l %1\n\t"
		"adds #1,%0\n\t"
		"bcc 1b"
		: "=r" (result) : "r"(word));
	return result;
}

/*
 * hweightN: returns the hamming weight (i.e. the number
 * of bits set) of a N-bit word
 */

#define hweight32(x) generic_hweight32(x)
#define hweight16(x) generic_hweight16(x)
#define hweight8(x) generic_hweight8(x)

static __inline__ int ext2_set_bit(int nr, volatile void * addr)
{
	int		mask, retval;
	unsigned long	flags;
	volatile unsigned char	*ADDR = (unsigned char *) addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	save_and_cli(flags);
	retval = (mask & *ADDR) != 0;
	*ADDR |= mask;
	restore_flags(flags);
	return retval;
}

static __inline__ int ext2_clear_bit(int nr, volatile void * addr)
{
	int		mask, retval;
	unsigned long	flags;
	volatile unsigned char	*ADDR = (unsigned char *) addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	save_and_cli(flags);
	retval = (mask & *ADDR) != 0;
	*ADDR &= ~mask;
	restore_flags(flags);
	return retval;
}

static __inline__ int ext2_test_bit(int nr, const volatile void * addr)
{
	int			mask;
	const volatile unsigned char	*ADDR = (const unsigned char *) addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	return ((mask & *ADDR) != 0);
}

#define ext2_find_first_zero_bit(addr, size) \
        ext2_find_next_zero_bit((addr), (size), 0)

static __inline__ unsigned long ext2_find_next_zero_bit(void *addr, unsigned long size, unsigned long offset)
{
	unsigned long *p = ((unsigned long *) addr) + (offset >> 5);
	unsigned long result = offset & ~31UL;
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= 31UL;
	if(offset) {
		/* We hold the little endian value in tmp, but then the
		 * shift is illegal. So we could keep a big endian value
		 * in tmp, like this:
		 *
		 * tmp = __swab32(*(p++));
		 * tmp |= ~0UL >> (32-offset);
		 *
		 * but this would decrease preformance, so we change the
		 * shift:
		 */
		tmp = *(p++);
		tmp |= __swab32(~0UL >> (32-offset));
		if(size < 32)
			goto found_first;
		if(~tmp)
			goto found_middle;
		size -= 32;
		result += 32;
	}
	while(size & ~31UL) {
		if(~(tmp = *(p++)))
			goto found_middle;
		result += 32;
		size -= 32;
	}
	if(!size)
		return result;
	tmp = *p;

found_first:
	/* tmp is little endian, so we would have to swab the shift,
	 * see above. But then we have to swab tmp below for ffz, so
	 * we might as well do this here.
	 */
	return result + ffz(__swab32(tmp) | (~0UL << size));
found_middle:
	return result + ffz(__swab32(tmp));
}

/* Bitmap functions for the minix filesystem.  */
#define minix_test_and_set_bit(nr,addr) test_and_set_bit(nr,addr)
#define minix_set_bit(nr,addr) set_bit(nr,addr)
#define minix_test_and_clear_bit(nr,addr) test_and_clear_bit(nr,addr)
#define minix_test_bit(nr,addr) test_bit(nr,addr)
#define minix_find_first_zero_bit(addr,size) find_first_zero_bit(addr,size)

/**
 * hweightN - returns the hamming weight of a N-bit word
 * @x: the word to weigh
 *
 * The Hamming Weight of a number is the total number of bits set in it.
 */

#define hweight32(x) generic_hweight32(x)
#define hweight16(x) generic_hweight16(x)
#define hweight8(x) generic_hweight8(x)

#endif /* __KERNEL__ */

#endif /* _H8300_BITOPS_H */
