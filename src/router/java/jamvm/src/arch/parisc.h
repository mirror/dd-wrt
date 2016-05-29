/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009
 * Robert Lougher <rob@jamvm.org.uk>.
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

#define OS_ARCH "parisc"

#define HANDLER_TABLE_T static const void
#define DOUBLE_1_BITS 0x3ff0000000000000LL

#define READ_DBL(v,p,l)	v = ((u8)p[0]<<56)|((u8)p[1]<<48)|((u8)p[2]<<40) \
                            |((u8)p[3]<<32)|((u8)p[4]<<24)|((u8)p[5]<<16) \
                            |((u8)p[6]<<8)|(u8)p[7]; p+=8

/* Needed for i386 -- empty here */
#define FPU_HACK

typedef struct {
  volatile unsigned int __attribute__ ((aligned (16))) lock;
} CasLock;

extern CasLock cas_lock;

#define LDCW(addr)                                                   \
({                                                                   \
    unsigned int ret;                                                \
    __asm__ __volatile__("ldcw 0(%1),%0" : "=r" (ret) : "r" (addr)); \
    ret;                                                             \
})

#define COMPARE_AND_SWAP(addr, old_val, new_val) \
({                                               \
    int result;                                  \
                                                 \
    while(LDCW(&cas_lock.lock) == 0);            \
                                                 \
    if(*addr == old_val) {                       \
        *addr = new_val;                         \
        result = 1;                              \
    } else                                       \
        result = 0;                              \
                                                 \
    cas_lock.lock = 1;                           \
    __asm__ __volatile__ ("":::"memory");        \
    result;                                      \
})

#define LOCKWORD_READ(addr) *addr
#define LOCKWORD_WRITE(addr, value) *addr = value
#define LOCKWORD_COMPARE_AND_SWAP(addr, old_val, new_val) \
        COMPARE_AND_SWAP(addr, old_val, new_val)

#ifdef INLINING
#error Inlining not supported as FLUSH_CACHE unimplemented \
       on this architecture
#endif

#define MBARRIER() __asm__ __volatile__ ("" ::: "memory")
#define JMM_LOCK_MBARRIER() __asm__ __volatile__ ("" ::: "memory")
#define JMM_UNLOCK_MBARRIER() __asm__ __volatile__ ("" ::: "memory")
