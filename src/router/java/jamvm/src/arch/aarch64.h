/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008
 * Robert Lougher <rob@jamvm.org.uk>.
 * Copyright (C) 2020 Simon South <simon@simonsouth.net>.
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

#include <stdint.h>

#define OS_ARCH "aarch64"

#define HANDLER_TABLE_T static const void
#define DOUBLE_1_BITS 0x3ff0000000000000LL

#define READ_DBL(v,p,l) v = ((u8)p[0]<<56)|((u8)p[1]<<48)|((u8)p[2]<<40) \
                            |((u8)p[3]<<32)|((u8)p[4]<<24)|((u8)p[5]<<16) \
                            |((u8)p[6]<<8)|(u8)p[7]; p+=8

/* Needed for i386 -- empty here */
#define FPU_HACK

#define COMPARE_AND_SWAP_64(addr, old_val, new_val)             \
({                                                              \
    int result, read_val;                                       \
    __asm__ __volatile__ ("                                     \
        1:      ldaxr %2, %1;                                   \
                cmp %2, %3;                                     \
                b.ne 2f;                                        \
                stlxr %w0, %4, %1;                              \
                cmp %w0, wzr;                                   \
                b.ne 1b;                                        \
        2:      cset %w0, eq;"                                  \
    : "=&r" (result), "+Q" (*addr), "=&r" (read_val)            \
    : "r" (old_val), "r" (new_val)                              \
    : "cc");                                                    \
    result;                                                     \
})

#define COMPARE_AND_SWAP_32(addr, old_val, new_val)             \
({                                                              \
    int result, read_val;                                       \
    __asm__ __volatile__ ("                                     \
        1:      ldaxr %w2, %1;                                  \
                cmp %w2, %w3;                                   \
                b.ne 2f;                                        \
                stlxr %w0, %w4, %1;                             \
                cmp %w0, wzr;                                   \
                b.ne 1b;                                        \
        2:      cset %w0, eq;"                                  \
    : "=&r" (result), "+Q" (*addr), "=&r" (read_val)            \
    : "r" (old_val), "r" (new_val)                              \
    : "cc");                                                    \
    result;                                                     \
})

#define COMPARE_AND_SWAP(addr, old_val, new_val)                \
        COMPARE_AND_SWAP_64(addr, old_val, new_val)

#define LOCKWORD_READ(addr)                                     \
({                                                              \
    uintptr_t result;                                           \
    __asm__ __volatile__ ("                                     \
                ldar %0, %1;"                                   \
    : "=r" (result)                                             \
    : "Q" (*addr)                                               \
    : "cc");                                                    \
    result;                                                     \
})

#define LOCKWORD_WRITE(addr, value)                             \
({                                                              \
    __asm__ __volatile__ ("                                     \
                stlr %1, %0;"                                   \
    : "=Q" (*addr)                                              \
    : "r" (value)                                               \
    : "cc");                                                    \
})

#define LOCKWORD_COMPARE_AND_SWAP(addr, old_val, new_val)       \
        COMPARE_AND_SWAP_64(addr, old_val, new_val)

#define FLUSH_CACHE(addr, length)                               \
{                                                               \
    uintptr_t start = (uintptr_t) (addr);                       \
    uintptr_t end = start + length;                             \
    uintptr_t i;                                                \
                                                                \
    for(i = start & aarch64_data_cache_line_mask;               \
        i < end;                                                \
        i += aarch64_data_cache_line_len)                       \
        __asm__ ("dc cvau, %0" :: "r" (i));                     \
                                                                \
    __asm__ ("dsb ish");                                        \
                                                                \
    for(i = start & aarch64_instruction_cache_line_mask;        \
        i < end;                                                \
        i += aarch64_instruction_cache_line_len)                \
        __asm__ ("ic ivau, %0" :: "r" (i));                     \
                                                                \
    __asm__ ("dsb ish; isb");                                   \
}

#define GEN_REL_JMP(target_addr, patch_addr, patch_size)        \
({                                                              \
    int patched = FALSE;                                        \
                                                                \
    if(patch_size >= 4) {                                       \
        /* Guard against the pointer difference being           \
           larger than the signed range */                      \
        long long offset = (uintptr_t)(target_addr) -           \
                           (uintptr_t)(patch_addr);             \
                                                                \
        if(offset >= -1<<28 && offset < 1<<28) {                \
            *(uint32_t*)(patch_addr) = offset>>2 & 0x03ffffff   \
                                                 | 0x14000000;  \
            patched = TRUE;                                     \
        }                                                       \
    }                                                           \
    patched;                                                    \
})

#define MBARRIER() __asm__ ("dmb ish" ::: "memory")
#define RMBARRIER() __asm__ ("dmb ishld" ::: "memory")
#define WMBARRIER() __asm__ ("dmb ishst" ::: "memory")
#define JMM_LOCK_MBARRIER() __asm__ ("dmb ish" ::: "memory")
#define JMM_UNLOCK_MBARRIER() JMM_LOCK_MBARRIER()

/* Defined in src/os/linux/aarch64/init.c */
extern unsigned char aarch64_data_cache_line_len;
extern uintptr_t aarch64_data_cache_line_mask;
extern unsigned char aarch64_instruction_cache_line_len;
extern uintptr_t aarch64_instruction_cache_line_mask;
