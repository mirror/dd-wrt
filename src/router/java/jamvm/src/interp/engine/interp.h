/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013
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

#include <limits.h>
#include "properties.h"

/* _GNU_SOURCE doesn't bring in C99 long long constants,
   but we do get the GNU constants */
#ifndef LLONG_MAX
#define LLONG_MAX LONG_LONG_MAX
#endif

#ifndef LLONG_MIN
#define LLONG_MIN LONG_LONG_MIN
#endif

#define FLOAT_1_BITS 0x3f800000
#define FLOAT_2_BITS 0x40000000

#define READ_U1_OP(p)    ((p)[1])
#define READ_U2_OP(p)    (((p)[1]<<8)|(p)[2])
#define READ_U4_OP(p)    (((p)[1]<<24)|((p)[2]<<16)|((p)[3]<<8)|(p)[4])
#define READ_S1_OP(p)    (signed char)READ_U1_OP(p)
#define READ_S2_OP(p)    (signed short)READ_U2_OP(p)
#define READ_S4_OP(p)    (signed int)READ_U4_OP(p)

/* Stack related macros */

#define STACK_float(offset)    *((float*)&ostack[offset] + IS_BE64)
#define STACK_uint64_t(offset) *(uint64_t*)&ostack[offset * 2]
#define STACK_int64_t(offset)  *(int64_t*)&ostack[offset * 2]
#define STACK_double(offset)   *(double*)&ostack[offset * 2]
#define STACK_uint16_t(offset) (uint16_t)ostack[offset]
#define STACK_int16_t(offset)  (int16_t)ostack[offset]
#define STACK_int8_t(offset)   (int8_t)ostack[offset]
#define STACK_int(offset)      (int)ostack[offset]

#define STACK(type, offset)  STACK_##type(offset)

#define SLOTS(type) (sizeof(type) + 3)/4

#define STACK_POP(type) ({        \
    ostack -= SLOTS(type);        \
    STACK(type, 0);               \
})

/* In the macro below we assign 'value' to a temporary
   to ensure any modification of ostack within value
   is done before pushing */

#define STACK_PUSH(type, value) { \
    type val = value;             \
    STACK(type, 0) = val;         \
    ostack += SLOTS(type);        \
}

#ifdef CHECK_INTDIV_OVERFLOW
#define INTDIV_OVERFLOW(dividend, divisor) \
    (dividend == INT_MIN && divisor == -1)
#else
#define INTDIV_OVERFLOW(dividend, divisor) FALSE
#endif

#ifdef CHECK_LONGDIV_OVERFLOW
#define LONGDIV_OVERFLOW(dividend, divisor) \
    (dividend == LLONG_MIN && divisor == -1)
#else
#define LONGDIV_OVERFLOW(dividend, divisor) FALSE
#endif

#ifndef JSR292
#define CACHE_POLY_OFFSETS
#define CACHED_POLY_OFFSETS

#define ID_invokeBasic   1
#define ID_linkToStatic  1
#define ID_linkToVirtual 1

#define mbPolymorphicNameID(mb)       0
#define isPolymorphicRef(class, idx)  FALSE
#define resolvePolyMethod(class, idx) ({ NULL; })
#endif

/* Include the interpreter variant header */

#ifdef DIRECT
#ifdef INLINING
#include "interp-inlining.h"
#else
#include "interp-direct.h"
#endif /* INLINING */
#else
#include "interp-indirect.h"
#endif /* DIRECT */
