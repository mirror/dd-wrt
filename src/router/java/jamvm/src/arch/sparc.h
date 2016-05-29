/*
 * Copyright (C) 2009 Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#define OS_ARCH "sparc"

#define HANDLER_TABLE_T static const void
#define DOUBLE_1_BITS 0x3ff0000000000000LL

#define READ_DBL(v,p,l)	v = ((u8)p[0]<<56)|((u8)p[1]<<48)|((u8)p[2]<<40)  \
                            |((u8)p[3]<<32)|((u8)p[4]<<24)|((u8)p[5]<<16) \
                            |((u8)p[6]<<8)|(u8)p[7]; p+=8; 

#define FPU_HACK

#define FULL_MBAR() __asm__ __volatile__ ("membar #LoadLoad | #LoadStore"\
                        " | #StoreLoad | #StoreStore" : : : "memory")

static inline uint32_t
compare_and_swap_32(volatile uint32_t *addr, uint32_t oldval, uint32_t newval)
{
   uint32_t result;
   FULL_MBAR();
   __asm__ __volatile__ ("cas [%4], %2, %0"
   			: "=r" (result), "=m" (*addr)
			: "r" (oldval), "m" (*addr), "r" (addr),
			"0" (newval));
  return result == oldval;
}

#define COMPARE_AND_SWAP32 compare_and_swap_32

static inline uint64_t
compare_and_swap_64(volatile uint64_t *addr, uint64_t oldval, uint64_t newval)
{
   uint64_t result;
   FULL_MBAR();
   __asm__ __volatile__ ("casx [%4], %2, %0"
   			: "=r" (result), "=m" (*addr)
			: "r" (oldval), "m" (*addr), "r" (addr),
			"0" (newval));
  return result == oldval;
}

#define COMPARE_AND_SWAP64 compare_and_swap_64    
#ifdef __arch64__
#  define COMPARE_AND_SWAP COMPARE_AND_SWAP64
#else
#  define COMPARE_AND_SWAP COMPARE_AND_SWAP32
#endif

#define LOCKWORD_READ(addr) *addr
#define LOCKWORD_WRITE(addr, value) *addr = value
#define LOCKWORD_COMPARE_AND_SWAP COMPARE_AND_SWAP

#define MBARRIER() FULL_MBAR()
#define JMM_LOCK_MBARRIER() MBARRIER()
#define JMM_UNLOCK_MBARRIER() MBARRIER()
