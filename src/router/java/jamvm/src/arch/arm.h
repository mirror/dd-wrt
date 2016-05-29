/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012,
 * 2014 Robert Lougher <rob@jamvm.org.uk>.
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

#define OS_ARCH "arm"

/* Override minimum min heap size.  The initial heap size is a ratio
   of the physical memory, but it must be at least the minimum min
   size.  The normal setting is too large for ARM machines as they
   are usually embedded. */
#define MIN_MIN_HEAP 1*MB

/* Likewise, override the default min/max heap sizes used when the
   size of physical memory is not available */
#define DEFAULT_MIN_HEAP 1*MB
#define DEFAULT_MAX_HEAP 64*MB

#ifdef DIRECT
#define HANDLER_TABLE_T static const void
#else
#define HANDLER_TABLE_T void
#endif

#if defined(__VFP_FP__) || defined(__ARMEB__)
#define DOUBLE_1_BITS 0x3ff0000000000000LL
#else
#define DOUBLE_1_BITS 0x000000003ff00000LL
#endif

#if defined(__VFP_FP__) || defined(__ARMEB__)
#define READ_DBL(v,p,l)	v = ((u8)p[0]<<56)|((u8)p[1]<<48)|((u8)p[2]<<40) \
                            |((u8)p[3]<<32)|((u8)p[4]<<24)|((u8)p[5]<<16) \
                            |((u8)p[6]<<8)|(u8)p[7]; p+=8
#else
#define READ_DBL(v,p,l)	v = ((u8)p[4]<<56)|((u8)p[5]<<48)|((u8)p[6]<<40) \
                            |((u8)p[7]<<32)|((u8)p[0]<<24)|((u8)p[1]<<16) \
                            |((u8)p[2]<<8)|(u8)p[3]; p+=8
#endif

/* Needed for i386 -- empty here */
#define FPU_HACK

#if defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_7A__)
#define COMPARE_AND_SWAP_32(addr, old_val, new_val)       \
({                                                        \
    int result, read_val;                                 \
    __asm__ __volatile__ ("                               \
        1:      mov %0, #0;                               \
                ldrex %1, [%2];                           \
                cmp %3, %1;                               \
                bne 2f;                                   \
                strex %0, %4, [%2];                       \
                cmp %0, #1;                               \
                beq 1b;                                   \
                mov %0, #1;                               \
        2:"                                               \
    : "=&r" (result), "=&r" (read_val)                    \
    : "r" (addr), "r" (old_val), "r" (new_val)            \
    : "cc", "memory");                                    \
    result;                                               \
})

#define COMPARE_AND_SWAP(addr, old_val, new_val)          \
        COMPARE_AND_SWAP_32(addr, old_val, new_val)

#define LOCKWORD_READ(addr) *addr
#define LOCKWORD_WRITE(addr, value) *addr = value
#define LOCKWORD_COMPARE_AND_SWAP(addr, old_val, new_val) \
        COMPARE_AND_SWAP(addr, old_val, new_val)

#else

#define LOCKWORD_COMPARE_AND_SWAP(addr, old_val, new_val) \
({                                                        \
    int result, read_val;                                 \
    __asm__ __volatile__ ("                               \
                mvn %1, #1;                               \
        1:      swp %1, %1, [%2];                         \
                cmn %1, #2;                               \
                beq 1b;                                   \
                cmp %3, %1;                               \
                strne %1, [%2];                           \
                movne %0, #0;                             \
                streq %4, [%2];                           \
                moveq %0, #1;"                            \
    : "=&r" (result), "=&r" (read_val)                    \
    : "r" (addr), "r" (old_val), "r" (new_val)            \
    : "cc", "memory");                                    \
    result;                                               \
})

#define LOCKWORD_READ(addr)                               \
({                                                        \
    int read_val;                                         \
    __asm__ __volatile__ ("                               \
        1:      ldr %0, [%1];                             \
                cmn %0, #2;                               \
                beq 1b;"                                  \
    : "=&r" (read_val) : "r" (addr) : "cc");              \
    read_val;                                             \
})

#define LOCKWORD_WRITE(addr, new_val)                     \
do {                                                      \
    int read_val;                                         \
    __asm__ __volatile__ ("                               \
                mvn %0, #1;                               \
        1:      swp %0, %0, [%1];                         \
                cmn %0, #2;                               \
                beq 1b;                                   \
                str %2, [%1];"                            \
    : "=&r" (read_val)                                    \
    : "r" (addr), "r" (new_val)                           \
    : "cc", "memory");                                    \
} while(0)
#endif

#ifdef __ARM_EABI__
#define FLUSH_CACHE(addr, length)                         \
{                                                         \
    __asm__ __volatile__ ("                               \
        mov r0, %0\n                                      \
        mov r1, %1\n                                      \
        mov r2, #0\n                                      \
        mov r7, #0xf0000\n                                \
        add r7, r7, #2\n                                  \
        swi 0\n                                           \
    ":                                                    \
     : "r" (addr), "r" (addr + length - 1)                \
     : "r0", "r1", "r2", "r7");                           \
}
#else
#define FLUSH_CACHE(addr, length)                         \
{                                                         \
    __asm__ __volatile__ ("                               \
        mov r0, %0\n                                      \
        mov r1, %1\n                                      \
        mov r2, #0\n                                      \
        swi 0x9f0002\n                                    \
    ":                                                    \
     : "r" (addr), "r" (addr + length - 1)                \
     : "r0", "r1", "r2");                                 \
}
#endif

#define GEN_REL_JMP(target_addr, patch_addr, patch_size)  \
({                                                        \
    int patched = FALSE;                                  \
                                                          \
    if(patch_size >= 4) {                                 \
        /* Guard against the pointer difference being     \
           larger than the signed range */                \
        long long offset = (uintptr_t)(target_addr) -     \
                           (uintptr_t)(patch_addr) - 8;   \
                                                          \
        if(offset >= -1<<25 && offset < 1<<25) {          \
            *(int*)(patch_addr) = offset>>2 & 0x00ffffff  \
                                            | 0xea000000; \
            patched = TRUE;                               \
        }                                                 \
    }                                                     \
    patched;                                              \
})

#ifdef __ARM_ARCH_7A__
#define MBARRIER() __asm__ __volatile__ ("dmb" ::: "memory")
#define JMM_LOCK_MBARRIER() __asm__ __volatile__ ("dmb" ::: "memory")
#define JMM_UNLOCK_MBARRIER() __asm__ __volatile__ ("dmb" ::: "memory")
#else
#define MBARRIER() __asm__ __volatile__ ("" ::: "memory")
#define JMM_LOCK_MBARRIER() __asm__ __volatile__ ("" ::: "memory")
#define JMM_UNLOCK_MBARRIER() __asm__ __volatile__ ("" ::: "memory")
#endif
