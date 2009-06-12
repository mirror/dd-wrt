/*
 * Copyright 1995, Russell King.
 * Various bits and pieces copyrights include:
 *  Linus Torvalds (test_bit).
 *
 * bit 0 is the LSB of addr; bit 32 is the LSB of (addr+1).
 *
 * Please note that the code in this file should never be included
 * from user space.  Many of these are not implemented in assembler
 * since they would be too costly.  Also, they require priviledged
 * instructions (which are not available from user mode) to ensure
 * that they are atomic.
 */

#ifndef __ASM_ARM_BITOPS_H
#define __ASM_ARM_BITOPS_H

#ifdef __KERNEL__

#ifndef __ARMEB__

#define smp_mb__before_clear_bit()	do { } while (0)
#define smp_mb__after_clear_bit()	do { } while (0)

/*
 * Function prototypes to keep gcc -Wall happy.
 */
extern void set_bit(int nr, volatile void * addr);

static inline void __set_bit(int nr, volatile void *addr)
{
	((unsigned char *) addr)[nr >> 3] |= (1U << (nr & 7));
}

extern void clear_bit(int nr, volatile void * addr);

static inline void __clear_bit(int nr, volatile void *addr)
{
	((unsigned char *) addr)[nr >> 3] &= ~(1U << (nr & 7));
}

extern void change_bit(int nr, volatile void * addr);

static inline void __change_bit(int nr, volatile void *addr)
{
	((unsigned char *) addr)[nr >> 3] ^= (1U << (nr & 7));
}

extern int test_and_set_bit(int nr, volatile void * addr);

static inline int __test_and_set_bit(int nr, volatile void *addr)
{
	unsigned int mask = 1 << (nr & 7);
	unsigned int oldval;

	oldval = ((unsigned char *) addr)[nr >> 3];
	((unsigned char *) addr)[nr >> 3] = oldval | mask;
	return oldval & mask;
}

extern int test_and_clear_bit(int nr, volatile void * addr);

static inline int __test_and_clear_bit(int nr, volatile void *addr)
{
	unsigned int mask = 1 << (nr & 7);
	unsigned int oldval;

	oldval = ((unsigned char *) addr)[nr >> 3];
	((unsigned char *) addr)[nr >> 3] = oldval & ~mask;
	return oldval & mask;
}

extern int test_and_change_bit(int nr, volatile void * addr);

static inline int __test_and_change_bit(int nr, volatile void *addr)
{
	unsigned int mask = 1 << (nr & 7);
	unsigned int oldval;

	oldval = ((unsigned char *) addr)[nr >> 3];
	((unsigned char *) addr)[nr >> 3] = oldval ^ mask;
	return oldval & mask;
}

extern int find_first_zero_bit(void * addr, unsigned size);
extern int find_next_zero_bit(void * addr, int size, int offset);

/*
 * This routine doesn't need to be atomic.
 */
extern __inline__ int test_bit(int nr, const void * addr)
{
    return ((unsigned char *) addr)[nr >> 3] & (1U << (nr & 7));
}	

/*
 * ffz = Find First Zero in word. Undefined if no zero exists,
 * so code should check against ~0UL first..
 */
extern __inline__ unsigned long ffz(unsigned long word)
{
	int k;

	word = ~word;
	k = 31;
	if (word & 0x0000ffff) { k -= 16; word <<= 16; }
	if (word & 0x00ff0000) { k -= 8;  word <<= 8;  }
	if (word & 0x0f000000) { k -= 4;  word <<= 4;  }
	if (word & 0x30000000) { k -= 2;  word <<= 2;  }
	if (word & 0x40000000) { k -= 1; }
        return k;
}

/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */

#define ffs(x) generic_ffs(x)

#define ext2_set_bit			test_and_set_bit
#define ext2_clear_bit			test_and_clear_bit
#define ext2_test_bit			test_bit
#define ext2_find_first_zero_bit	find_first_zero_bit
#define ext2_find_next_zero_bit		find_next_zero_bit

/* Bitmap functions for the minix filesystem. */
#define minix_test_and_set_bit(nr,addr)	test_and_set_bit(nr,addr)
#define minix_set_bit(nr,addr)		set_bit(nr,addr)
#define minix_test_and_clear_bit(nr,addr)	test_and_clear_bit(nr,addr)
#define minix_test_bit(nr,addr)		test_bit(nr,addr)
#define minix_find_first_zero_bit(addr,size)	find_first_zero_bit(addr,size)



#else /******* __ARMEB__ *******/



/*
 * All these stolen from the m68knommu header,  why ? because
 * they implement everything and it saves having to fix the
 * assembler routines in arch/armnommu/lib ;-)
 */

#include <linux/config.h>
#include <asm/byteorder.h>	/* swab32 */
#include <asm/system.h>		/* save_flags */

#ifndef save_and_cli
#define save_and_cli(x) do { save_flags(flags); cli(); } while(0)
#endif

#ifndef save_and_set
#define save_and_set(x) do { save_flags(flags); set(); } while(0)
#endif


extern void set_bit(int nr, volatile void * addr);
extern void __set_bit(int nr, volatile void * addr);
extern void clear_bit(int nr, volatile void * addr);
extern void change_bit(int nr, volatile void * addr);
extern void __change_bit(int nr, volatile void * addr);
extern int test_and_set_bit(int nr, volatile void * addr);
extern int __test_and_set_bit(int nr, volatile void * addr);
extern int test_and_clear_bit(int nr, volatile void * addr);
extern int __test_and_clear_bit(int nr, volatile void * addr);
extern int test_and_change_bit(int nr, volatile void * addr);
extern int __test_and_change_bit(int nr, volatile void * addr);
extern int __constant_test_bit(int nr, const volatile void * addr);
extern int __test_bit(int nr, volatile void * addr);
extern int find_first_zero_bit(void * addr, unsigned size);
extern int find_next_zero_bit (void * addr, int size, int offset);

/*
 * ffz = Find First Zero in word. Undefined if no zero exists,
 * so code should check against ~0UL first..
 */
extern __inline__ unsigned long ffz(unsigned long word)
{
	unsigned long result = 0;

	while(word & 1) {
		result++;
		word >>= 1;
	}
	return result;
}


extern __inline__ void set_bit(int nr, volatile void * addr)
{
	int 	* a = (int *) addr;
	int	mask;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	save_flags(flags); cli();
	*a |= mask;
	restore_flags(flags);
}

extern __inline__ void __set_bit(int nr, volatile void * addr)
{
	int 	* a = (int *) addr;
	int	mask;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	*a |= mask;
}

/*
 * clear_bit() doesn't provide any barrier for the compiler.
 */
#define smp_mb__before_clear_bit()	barrier()
#define smp_mb__after_clear_bit()	barrier()

extern __inline__ void clear_bit(int nr, volatile void * addr)
{
	int 	* a = (int *) addr;
	int	mask;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	save_flags(flags); cli();
	*a &= ~mask;
	restore_flags(flags);
}

extern __inline__ void change_bit(int nr, volatile void * addr)
{
	int mask, flags;
	unsigned long *ADDR = (unsigned long *) addr;

	ADDR += nr >> 5;
	mask = 1 << (nr & 31);
	save_flags(flags); cli();
	*ADDR ^= mask;
	restore_flags(flags);
}

extern __inline__ void __change_bit(int nr, volatile void * addr)
{
	int mask;
	unsigned long *ADDR = (unsigned long *) addr;

	ADDR += nr >> 5;
	mask = 1 << (nr & 31);
	*ADDR ^= mask;
}

extern __inline__ int test_and_set_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile unsigned int *a = (volatile unsigned int *) addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	save_and_cli(flags);
	retval = (mask & *a) != 0;
	*a |= mask;
	restore_flags(flags);

	return retval;
}

extern __inline__ int __test_and_set_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile unsigned int *a = (volatile unsigned int *) addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a |= mask;
	return retval;
}

extern __inline__ int test_and_clear_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile unsigned int *a = (volatile unsigned int *) addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	save_and_cli(flags);
	retval = (mask & *a) != 0;
	*a &= ~mask;
	restore_flags(flags);

	return retval;
}

extern __inline__ int __test_and_clear_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile unsigned int *a = (volatile unsigned int *) addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a &= ~mask;
	return retval;
}

extern __inline__ int test_and_change_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile unsigned int *a = (volatile unsigned int *) addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	save_and_cli(flags);
	retval = (mask & *a) != 0;
	*a ^= mask;
	restore_flags(flags);

	return retval;
}

extern __inline__ int __test_and_change_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile unsigned int *a = (volatile unsigned int *) addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a ^= mask;
	return retval;
}

/*
 * This routine doesn't need to be atomic.
 */
extern __inline__ int __constant_test_bit(int nr, const volatile void * addr)
{
	return ((1UL << (nr & 31)) & (((const volatile unsigned int *) addr)[nr >> 5])) != 0;
}

extern __inline__ int __test_bit(int nr, volatile void * addr)
{
	int 	* a = (int *) addr;
	int	mask;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	return ((mask & *a) != 0);
}

#define test_bit(nr,addr) \
(__builtin_constant_p(nr) ? \
 __constant_test_bit((nr),(addr)) : \
 __test_bit((nr),(addr)))

#define find_first_zero_bit(addr, size) \
        find_next_zero_bit((addr), (size), 0)

extern __inline__ int find_next_zero_bit (void * addr, int size, int offset)
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

#define ffs(x) generic_ffs(x)

extern __inline__ int ext2_set_bit(int nr, volatile void * addr)
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

extern __inline__ int ext2_clear_bit(int nr, volatile void * addr)
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

extern __inline__ int ext2_test_bit(int nr, const volatile void * addr)
{
	int			mask;
	const volatile unsigned char	*ADDR = (const unsigned char *) addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	return ((mask & *ADDR) != 0);
}

#define ext2_find_first_zero_bit(addr, size) \
        ext2_find_next_zero_bit((addr), (size), 0)

extern __inline__ unsigned long ext2_find_next_zero_bit(void *addr, unsigned long size, unsigned long offset)
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
#define minix_test_and_set_bit(nr,addr) ext2_test_and_set_bit(nr,addr)
#define minix_set_bit(nr,addr) ext2_set_bit(nr,addr)
#define minix_test_and_clear_bit(nr,addr) ext2_test_and_clear_bit(nr,addr)
#define minix_test_bit(nr,addr) ext2_test_bit(nr,addr)
#define minix_find_first_zero_bit(addr,size) ext2_find_first_zero_bit(addr,size)


#endif /* __ARMEB__ */

/*
 * hweightN: returns the hamming weight (i.e. the number
 * of bits set) of a N-bit word
 */

#define hweight32(x) generic_hweight32(x)
#define hweight16(x) generic_hweight16(x)
#define hweight8(x) generic_hweight8(x)


#endif /* __KERNEL__ */

#endif /* _ARM_BITOPS_H */
