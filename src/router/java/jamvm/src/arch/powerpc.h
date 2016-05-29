/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2012
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

#define OS_ARCH "ppc"

#define HANDLER_TABLE_T static const void
#define DOUBLE_1_BITS 0x3ff0000000000000LL

#define READ_DBL(v,p,l)	v = ((u8)p[0]<<56)|((u8)p[1]<<48)|((u8)p[2]<<40) \
                            |((u8)p[3]<<32)|((u8)p[4]<<24)|((u8)p[5]<<16) \
                            |((u8)p[6]<<8)|(u8)p[7]; p+=8

/* Needed for i386 -- empty here */
#define FPU_HACK

#define COMPARE_AND_SWAP_32(addr, old_val, new_val) \
({                                                  \
    int result, read_val;                           \
    __asm__ __volatile__ ("                         \
                li %0,0\n                           \
        1:      lwarx %1,0,%2\n                     \
                cmpw %3,%1\n                        \
                bne- 2f\n                           \
                stwcx. %4,0,%2\n                    \
                bne- 1b\n                           \
                li %0,1\n                           \
        2:"                                         \
    : "=&r" (result), "=&r" (read_val)              \
    : "r" (addr), "r" (old_val), "r" (new_val)      \
    : "cc", "memory");                              \
    result;                                         \
})

#ifdef __ppc64__
#define COMPARE_AND_SWAP_64(addr, old_val, new_val) \
({                                                  \
    int result, read_val;                           \
    __asm__ __volatile__ ("                         \
                li %0,0\n                           \
        1:      ldarx %1,0,%2\n                     \
                cmpd %3,%1\n                        \
                bne- 2f\n                           \
                stdcx. %4,0,%2\n                    \
                bne- 1b\n                           \
                li %0,1\n                           \
        2:"                                         \
    : "=&r" (result), "=&r" (read_val)              \
    : "r" (addr), "r" (old_val), "r" (new_val)      \
    : "cc", "memory");                              \
    result;                                         \
})

#define COMPARE_AND_SWAP(addr, old_val, new_val)    \
        COMPARE_AND_SWAP_64(addr, old_val, new_val)
#else
#define COMPARE_AND_SWAP(addr, old_val, new_val)    \
        COMPARE_AND_SWAP_32(addr, old_val, new_val)
#endif

#define LOCKWORD_READ(addr) *addr
#define LOCKWORD_WRITE(addr, value) *addr = value
#define LOCKWORD_COMPARE_AND_SWAP(addr, old_val, new_val) \
        COMPARE_AND_SWAP(addr, old_val, new_val)

#define CACHE_LINE_LEN 32

#define FLUSH_CACHE(addr, length)                   \
{                                                   \
    uintptr_t end = ((uintptr_t) (addr)) + length;  \
    uintptr_t start = ((uintptr_t) (addr))          \
                      & ~(CACHE_LINE_LEN - 1);      \
    uintptr_t i;                                    \
                                                    \
    for(i = start; i < end; i += CACHE_LINE_LEN)    \
        __asm__ ("dcbst 0, %0" :: "r" (i));         \
                                                    \
    __asm__ ("sync");                               \
                                                    \
    for(i = start; i < end; i += CACHE_LINE_LEN)    \
        __asm__ ("icbi 0, %0" :: "r" (i));          \
                                                    \
    __asm__ ("sync; isync");                        \
}

#define GEN_REL_JMP(target_addr, patch_addr, patch_size)     \
({                                                           \
    int patched = FALSE;                                     \
                                                             \
    if(patch_size >= 4) {                                    \
        /* The check is done in two parts to ensure the      \
           result is always positive, to guard against       \
           the pointer difference being larger than the      \
           signed range */                                   \
        if(target_addr > patch_addr) {                       \
            uintptr_t offset = (target_addr) - (patch_addr); \
                                                             \
            if(offset < 1<<25) {                             \
                *(int*)(patch_addr) = offset & 0x3ffffff     \
                                             | 0x48000000;   \
                patched = TRUE;                              \
            }                                                \
        } else {                                             \
            uintptr_t offset = (patch_addr) - (target_addr); \
                                                             \
            if(offset <= 1<<25) {                            \
                *(int*)(patch_addr) = -offset & 0x3ffffff    \
                                              | 0x48000000;  \
                patched = TRUE;                              \
            }                                                \
        }                                                    \
    }                                                        \
    patched;                                                 \
})

#define MBARRIER() __asm__ __volatile__ ("sync" ::: "memory")
#define JMM_LOCK_MBARRIER() __asm__ __volatile__ ("isync" ::: "memory")
#ifdef __NO_LWSYNC__
#define JMM_UNLOCK_MBARRIER() __asm__ __volatile__ ("sync" ::: "memory")
#else
#define JMM_UNLOCK_MBARRIER() __asm__ __volatile__ ("lwsync" ::: "memory")
#endif
