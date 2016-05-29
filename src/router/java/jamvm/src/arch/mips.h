/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011
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

#ifdef INLINING
/* Include userspace cache interface.  Needed for cache-flushing 
   of generated code */
#include <asm/cachectl.h>
#endif

#ifdef __MIPSEL__
#define OS_ARCH "mipsel"
#else
#define OS_ARCH "mips"
#endif

/* Override default min heap size.  The initial heap size is a ratio
   of the physical memory, but it must be at least the default min
   size.  The normal setting is too large for MIPS machines as they
   are usually embedded. */
#define DEFAULT_MIN_HEAP 1*MB

#define HANDLER_TABLE_T static const void
#define DOUBLE_1_BITS 0x3ff0000000000000LL

#define READ_DBL(v,p,l)	v = ((u8)p[0]<<56)|((u8)p[1]<<48)|((u8)p[2]<<40) \
                            |((u8)p[3]<<32)|((u8)p[4]<<24)|((u8)p[5]<<16) \
                            |((u8)p[6]<<8)|(u8)p[7]; p+=8

/* needed for i386 -- empty here */
#define FPU_HACK 

#define COMPARE_AND_SWAP_32(addr, old_val, new_val)       \
({                                                        \
    int result, read_val;                                 \
    __asm__ __volatile__ (                                \
      "        .set      push\n"                          \
               __set_mips2                                \
      "1:      ll        %1,%2\n"                         \
      "        move      %0,$0\n"                         \
      "        bne       %1,%3,2f\n"                      \
      "        move      %0,%4\n"                         \
      "        sc        %0,%2\n"                         \
      "        .set      pop\n"                           \
      "        beqz      %0,1b\n"                         \
      "2:"                                                \
    : "=&r" (result), "=&r" (read_val)                    \
    : "m" (*addr), "r" (old_val), "r" (new_val)           \
    : "memory");                                          \
    result;                                               \
})

#ifdef __mips64
#define COMPARE_AND_SWAP_64(addr, old_val, new_val)       \
({                                                        \
    int result, read_val;                                 \
    __asm__ __volatile__ (                                \
      "1:      lld       %1,%2\n"                         \
      "        move      %0,$0\n"                         \
      "        bne       %1,%3,2f\n"                      \
      "        move      %0,%4\n"                         \
      "        scd       %0,%2\n"                         \
      "        beqz      %0,1b\n"                         \
      "2:"                                                \
    : "=&r" (result), "=&r" (read_val)                    \
    : "m" (*addr), "r" (old_val), "r" (new_val)           \
    : "memory");                                          \
    result;                                               \
})

/* MIPS64 is implicitly MIPS III so we don't need
   to specify anything, and specifying mips2 can
   break some gcc versions */
#define __set_mips2

#define COMPARE_AND_SWAP(addr, old_val, new_val)          \
        COMPARE_AND_SWAP_64(addr, old_val, new_val)
#else
#define __set_mips2 ".set mips2\n"

#define COMPARE_AND_SWAP(addr, old_val, new_val)          \
        COMPARE_AND_SWAP_32(addr, old_val, new_val)
#endif

#define LOCKWORD_READ(addr) *addr
#define LOCKWORD_WRITE(addr, value) *addr = value
#define LOCKWORD_COMPARE_AND_SWAP(addr, old_val, new_val) \
        COMPARE_AND_SWAP(addr, old_val, new_val)

#define MBARRIER()              \
    __asm__ __volatile__ (      \
        ".set push\n"           \
        __set_mips2             \
        "sync\n"                \
        ".set  pop\n"           \
    ::: "memory")

#define JMM_LOCK_MBARRIER()   MBARRIER()
#define JMM_UNLOCK_MBARRIER() MBARRIER()

/* Macros needed for inlining interpreter */

#define FLUSH_CACHE(addr, length)                                \
    cacheflush(addr, length, BCACHE)

/* Generate a relative jump via the b instruction, which has a range
   of +-128K.  We also insert a nop in the branch delay slot  */

#define GEN_REL_JMP(target_addr, patch_addr, patch_size)         \
({                                                               \
    int patched = FALSE;                                         \
                                                                 \
    if(patch_size >= 8) {                                        \
        /* The check is done in two parts to ensure the          \
           result is always positive, to guard against           \
           the pointer difference being larger than the          \
           signed range */                                       \
        if(target_addr > patch_addr + 4) {                       \
            uintptr_t offset = (target_addr) - (patch_addr) - 4; \
                                                                 \
            if(offset < 1<<17) {                                 \
                *(int*)(patch_addr) = offset>>2 | 0x10000000;    \
                *(int*)(patch_addr + 4) = 0;                     \
                patched = TRUE;                                  \
            }                                                    \
        } else {                                                 \
            uintptr_t offset = (patch_addr) - (target_addr) + 4; \
                                                                 \
            if(offset <= 1<<17) {                                \
                *(int*)(patch_addr) = -offset>>2 & 0x1000ffff;   \
                *(int*)(patch_addr + 4) = 0;                     \
                patched = TRUE;                                  \
            }                                                    \
        }                                                        \
    }                                                            \
    patched;                                                     \
})

