#ifndef _HYPERSTONE_EXT2_BITOPS_H_
#define _HYPERSTONE_EXT2_BITOPS_H_

/* ext2 bitops - Taken from MIPS and PPC implementations of bitops.h */

static __inline__ int ext2_set_bit(int nr, volatile void * addr)
{
        int mask, retval;
        volatile unsigned char *ADDR = (unsigned char *) addr;

        ADDR += nr >> 3;
        mask = 1 << (nr & 7);
        cli();
        retval = (mask & *ADDR) != 0;
        *ADDR |= mask;
        sti();
        return retval;
}

static __inline__ int ext2_clear_bit(int nr, volatile void * addr)
{
        int mask, retval;
        volatile unsigned char *ADDR = (unsigned char *) addr;

        ADDR += nr >> 3;
        mask = 1 << (nr & 7);
        cli();
        retval = (mask & *ADDR) != 0;
        *ADDR &= ~mask;
        sti();
        return retval;
}

static __inline__ int ext2_test_bit(int nr, const volatile void * addr)
{
        int mask;
        const volatile unsigned char *ADDR = (const unsigned char *) addr;

        ADDR += nr >> 3;
        mask = 1 << (nr & 7);
        return ((mask & *ADDR) != 0);
}

/* This implementation is originally taken from PPC
 */
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
                tmp = *(p++);
                tmp |= (~0UL >> (32-offset));
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
        return result + ffz((tmp) | (~0UL << size));
found_middle:
        return result + ffz((tmp));
}

/* Bitmap functions for the minix filesystem.  */
#define minix_test_and_set_bit(nr,addr) test_and_set_bit(nr,addr)
#define minix_set_bit(nr,addr) set_bit(nr,addr)
#define minix_test_and_clear_bit(nr,addr) test_and_clear_bit(nr,addr)
#define minix_test_bit(nr,addr) test_bit(nr,addr)
#define minix_find_first_zero_bit(addr,size) find_first_zero_bit(addr,size)

#endif

