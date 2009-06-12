#ifndef _ASM_NIOS_BITOPS_H_
#define _ASM_NIOS_BITOPS_H_

#ifdef __KERNEL__
#include <asm/system.h>
#endif

/*
 * Adapted to NIOS from generic bitops.h:
 *
 * For the benefit of those who are trying to port Linux to another
 * architecture, here are some C-language equivalents.  You should
 * recode these in the native assembly language, if at all possible.
 * To guarantee atomicity, these routines call cli() and sti() to
 * disable interrupts while they operate.  (You have to provide inline
 * routines to cli() and sti().)
 *
 * Also note, these routines assume that you have 32 bit integers.
 * You will have to change this if you are trying to port Linux to the
 * Alpha architecture or to a Cray.  :-)
 * 
 * C language equivalents written by Theodore Ts'o, 9/26/92
 */

extern __inline__ void set_bit(int nr, volatile void * a)
{
	int * addr = (int *) a;
	int	mask;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	_disable_interrupts();
	*addr |= mask;
	_enable_interrupts();
}

extern __inline__ void __set_bit(int nr, volatile void * a)
{
	int * addr = (int *) a;
	int	mask;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	*addr |= mask;
}

/*
 * clear_bit() doesn't provide any barrier for the compiler.
 */
#define smp_mb__before_clear_bit()	barrier()
#define smp_mb__after_clear_bit()	barrier()

extern __inline__ void clear_bit(int nr, volatile void * a)
{
	int * addr = (int *) a;
	int	mask;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	_disable_interrupts();
	*addr &= ~mask;
	_enable_interrupts();
}

extern __inline__ void __clear_bit(int nr, volatile void * a)
{
	int * addr = (int *) a;
	int	mask;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	*addr &= ~mask;
}

extern __inline__ int test_and_clear_bit(int nr, volatile void * a)
{
	int * addr = (int *) a;
	int	mask, retval;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	_disable_interrupts();
	retval = (mask & *addr) != 0;
	*addr &= ~mask;
	_enable_interrupts();
	return retval;
}

extern __inline__ int __test_and_clear_bit(int nr, volatile void * a)
{
	int * addr = (int *) a;
	int	mask, retval;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *addr) != 0;
	*addr &= ~mask;
	return retval;
}

extern __inline__ void change_bit(unsigned long nr, volatile void *addr)
{
	int mask;
	unsigned long *ADDR = (unsigned long *) addr;

	ADDR += nr >> 5;
	mask = 1 << (nr & 31);
	_disable_interrupts();
	*ADDR ^= mask;
	_enable_interrupts();
}

extern __inline__ void __change_bit(unsigned long nr, volatile void *addr)
{
	int mask;
	unsigned long *ADDR = (unsigned long *) addr;

	ADDR += nr >> 5;
	mask = 1 << (nr & 31);
	*ADDR ^= mask;
}

extern __inline__ int test_and_set_bit(int nr, volatile void * a)
{
	volatile unsigned int *addr = (volatile unsigned int *) a;
	int	mask,retval;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	_disable_interrupts();
	retval = (mask & *addr) != 0;
	*addr |= mask;
	_enable_interrupts();
	return retval;
}

extern __inline__ int __test_and_set_bit(int nr, volatile void * a)
{
	volatile unsigned int *addr = (volatile unsigned int *) a;
	int	mask,retval;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *addr) != 0;
	*addr |= mask;
	return retval;
}

extern __inline__ int test_and_change_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile unsigned int *a = (volatile unsigned int *) addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	_disable_interrupts();
	retval = (mask & *a) != 0;
	*a ^= mask;
	_enable_interrupts();

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
	int * a = (int *) addr;
	int	mask;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	return ((mask & *a) != 0);
}

#define test_bit(nr,addr) \
(__builtin_constant_p(nr) ? \
 __constant_test_bit((nr),(addr)) : \
 __test_bit((nr),(addr)))


/* The easy/cheese version for now. */
extern __inline__ unsigned long ffz(unsigned long word)
{
	unsigned long result = 0;

	while(word & 1) {
		result++;
		word >>= 1;
	}
	return result;
}

/* find_next_zero_bit() finds the first zero bit in a bit string of length
 * 'size' bits, starting the search at bit 'offset'. This is largely based
 * on Linus's ALPHA routines, which are pretty portable BTW.
 */

extern __inline__ unsigned long find_next_zero_bit(void *addr, unsigned long size, unsigned long offset)
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

/* Linus sez that gcc can optimize the following correctly, we'll see if this
 * holds on the Sparc as it does for the ALPHA.
 */

#define find_first_zero_bit(addr, size) \
        find_next_zero_bit((addr), (size), 0)

/* Now for the ext2 filesystem bit operations and helper routines.
 *
 * Both NIOS and ext2 are little endian, so these are the same as above.
 */

#define ext2_set_bit   test_and_set_bit
#define ext2_clear_bit test_and_clear_bit
#define ext2_test_bit  test_bit

#define ext2_find_first_zero_bit find_first_zero_bit
#define ext2_find_next_zero_bit  find_next_zero_bit

#endif /* _ASM_NIOS_BITOPS_H */
