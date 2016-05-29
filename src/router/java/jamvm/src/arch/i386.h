/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011
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

#define OS_ARCH "i386"

/* On i386 prior to gcc 4.3, return types less than 4 bytes in size
   were zero or sign extended.  This no longer happens, and so when
   calling a method through FFI, we need to do the extension.  This
   is unnecessary on most architectures */
#define FFI_RET_EXTEND

/* The idivl instruction generates an exception on integer divide
   overflow (INT_MIN/-1 can't be represented as a positive number) */
#define CHECK_INTDIV_OVERFLOW

#define HANDLER_TABLE_T static const void
#define DOUBLE_1_BITS 0x3ff0000000000000LL

#define READ_DBL(v,p,l)	v = ((u8)p[0]<<56)|((u8)p[1]<<48)|((u8)p[2]<<40) \
                            |((u8)p[3]<<32)|((u8)p[4]<<24)|((u8)p[5]<<16) \
                            |((u8)p[6]<<8)|(u8)p[7]; p+=8

extern void setDoublePrecision();
#define FPU_HACK setDoublePrecision()

#define COMPARE_AND_SWAP_32(addr, old_val, new_val) \
({                                                  \
    char result;                                    \
    __asm__ __volatile__ ("                         \
        lock;                                       \
        cmpxchgl %4, %1;                            \
        sete %0"                                    \
    : "=q" (result), "=m" (*addr)                   \
    : "m" (*addr), "a" (old_val), "r" (new_val)     \
    : "memory");                                    \
    result;                                         \
})

#ifdef USE_CMPXCHG8B
#define COMPARE_AND_SWAP_64(addr, old_val, new_val) \
({                                                  \
    int ov_hi = old_val >> 32;                      \
    int ov_lo = old_val & 0xffffffff;               \
    int nv_hi = new_val >> 32;                      \
    int nv_lo = new_val & 0xffffffff;               \
    char result;                                    \
    __asm__ __volatile__ ("                         \
        lock;                                       \
        cmpxchg8b %1;                               \
        sete %0"                                    \
    : "=q" (result)                                 \
    : "m" (*addr), "d" (ov_hi), "a" (ov_lo),        \
      "c" (nv_hi), "b" (nv_lo)                      \
    : "memory");                                    \
    result;                                         \
})
#endif

#define COMPARE_AND_SWAP(addr, old_val, new_val)    \
        COMPARE_AND_SWAP_32(addr, old_val, new_val)

#define __GEN_REL_JMP(target_addr, patch_addr, opcode,       \
                      type, patch_size)                      \
({                                                           \
    int patched = FALSE;                                     \
                                                             \
    if(patch_size >= 1 + sizeof(type)) {                     \
        char *nxt_ins_ptr = (patch_addr) + 1 + sizeof(type); \
                                                             \
        /* Guard against the pointer difference being        \
           larger than the signed range */                   \
        long long disp = (uintptr_t)(target_addr) -          \
                         (uintptr_t)(nxt_ins_ptr);           \
        long long limit = 1LL << ((sizeof(type) * 8) - 1);   \
                                                             \
        if(disp >= -limit && disp < limit) {                 \
            *(patch_addr) = opcode;                          \
            *(type*)&(patch_addr)[1] = disp;                 \
            patched = TRUE;                                  \
        }                                                    \
    }                                                        \
    patched;                                                 \
})

#define GEN_REL_JMP(target_addr, patch_addr, patch_size) \
({                                                       \
    __GEN_REL_JMP(target_addr, patch_addr, 0xeb,         \
                  signed char, patch_size) ||            \
    __GEN_REL_JMP(target_addr, patch_addr, 0xe9,         \
                  signed int, patch_size);               \
})

#define FLUSH_CACHE(addr, length)

#define LOCKWORD_READ(addr) *addr
#define LOCKWORD_WRITE(addr, value) *addr = value
#define LOCKWORD_COMPARE_AND_SWAP(addr, old_val, new_val) \
        COMPARE_AND_SWAP(addr, old_val, new_val)

#define JMM_LOCK_MBARRIER() __asm__ __volatile__ ("" ::: "memory")
#define JMM_UNLOCK_MBARRIER() __asm__ __volatile__ ("" ::: "memory")
#define MBARRIER() __asm__ __volatile__ ("lock; addl $0,0(%%esp)" ::: "memory")
