/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
 * 2013, 2014 Robert Lougher <rob@jamvm.org.uk>.
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

#define _GNU_SOURCE
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "jam.h"
#include "thread.h"
#include "lock.h"
#include "excep.h"
#include "symbol.h"
#include "frame.h"

#include "interp.h"

#include "hash.h"
#include "class.h"
#include "classlib.h"

uintptr_t *executeJava() {

    /* Definitions specific to the particular
       interpreter variant */
    INTERPRETER_DEFINITIONS

   /* Caching is supported in all variants and
      may be enabled or disabled */
#ifdef USE_CACHE
    union {
        struct {
            uintptr_t v1;
            uintptr_t v2;
        } i;
        int64_t l;
    } cache;
#endif

    /* Variable definitions holding the interpreter
       state.  These are common to all interpreter
       variants */
    uintptr_t *arg1;
    register CodePntr pc;
    ExecEnv *ee = getExecEnv();
    Frame *frame = ee->last_frame;
    register uintptr_t *lvars = frame->lvars;
    register uintptr_t *ostack = frame->ostack;

    Object *this = (Object*)lvars[0];
    MethodBlock *new_mb, *mb = frame->mb;
    ConstantPool *cp = &(CLASS_CB(mb->class)->constant_pool);

    CACHED_POLY_OFFSETS

    /* Initialise pc to the start of the method.  If it
       hasn't been executed before it may need preparing */
    PREPARE_MB(mb);
    pc = (CodePntr)mb->code;

    /* The initial dispatch code - this is specific to
       the interpreter variant */
    INTERPRETER_PROLOGUE

    /* Definitions of the opcode handlers */

#define MULTI_LEVEL_OPCODES(level)                         \
    DEF_OPC(OPC_ICONST_M1, level,                          \
        PUSH_##level(-1, 1);                               \
    )                                                      \
                                                           \
    DEF_OPC_3(OPC_ICONST_0,                                \
              OPC_FCONST_0,                                \
              OPC_ACONST_NULL, level,                      \
        PUSH_##level(0, 1);                                \
    )                                                      \
                                                           \
    DEF_OPC(OPC_ICONST_1, level,                           \
        PUSH_##level(1, 1);                                \
    )                                                      \
                                                           \
    DEF_OPC(OPC_ICONST_2, level,                           \
        PUSH_##level(2, 1);                                \
    )                                                      \
                                                           \
    DEF_OPC(OPC_ICONST_3, level,                           \
        PUSH_##level(3, 1);                                \
    )                                                      \
                                                           \
    DEF_OPC(OPC_ICONST_4, level,                           \
        PUSH_##level(4, 1);                                \
    )                                                      \
                                                           \
    DEF_OPC(OPC_ICONST_5, level,                           \
        PUSH_##level(5, 1);                                \
    )                                                      \
                                                           \
    DEF_OPC(OPC_FCONST_1, level,                           \
        PUSH_##level(FLOAT_1_BITS, 1);                     \
    )                                                      \
                                                           \
    DEF_OPC(OPC_FCONST_2, level,                           \
        PUSH_##level(FLOAT_2_BITS, 1);                     \
    )                                                      \
                                                           \
    DEF_OPC(OPC_SIPUSH, level,                             \
        PUSH_##level(DOUBLE_SIGNED(pc), 3);                \
    )                                                      \
                                                           \
    DEF_OPC(OPC_BIPUSH, level,                             \
        PUSH_##level(SINGLE_SIGNED(pc), 2);                \
    )                                                      \
                                                           \
    DEF_OPC(OPC_LDC_QUICK, level,                          \
        PUSH_##level(RESOLVED_CONSTANT(pc), 2);            \
    )                                                      \
                                                           \
    DEF_OPC(OPC_LDC_W_QUICK, level,                        \
        PUSH_##level(CP_INFO(cp, DOUBLE_INDEX(pc)), 3);    \
    )                                                      \
                                                           \
    DEF_OPC_3(OPC_ILOAD,                                   \
              OPC_FLOAD,                                   \
              OPC_ALOAD, level,                            \
        PUSH_##level(lvars[SINGLE_INDEX(pc)], 2);          \
    )                                                      \
                                                           \
    DEF_OPC(OPC_ALOAD_THIS, level,                         \
        ALOAD_THIS(level);                                 \
    )                                                      \
                                                           \
    DEF_OPC_2(OPC_ILOAD_0,                                 \
              OPC_FLOAD_0, level,                          \
        PUSH_##level(lvars[0], 1)                          \
    )                                                      \
                                                           \
    DEF_OPC_3(OPC_ILOAD_1,                                 \
              OPC_FLOAD_1,                                 \
              OPC_ALOAD_1, level,                          \
        PUSH_##level(lvars[1], 1);                         \
    )                                                      \
                                                           \
    DEF_OPC_3(OPC_ILOAD_2,                                 \
              OPC_FLOAD_2,                                 \
              OPC_ALOAD_2, level,                          \
        PUSH_##level(lvars[2], 1);                         \
    )                                                      \
                                                           \
    DEF_OPC_3(OPC_ILOAD_3,                                 \
              OPC_FLOAD_3,                                 \
              OPC_ALOAD_3, level,                          \
        PUSH_##level(lvars[3], 1);                         \
    )                                                      \
                                                           \
    DEF_OPC_3(OPC_ISTORE,                                  \
              OPC_FSTORE,                                  \
              OPC_ASTORE, level,                           \
        POP_##level(lvars[SINGLE_INDEX(pc)], 2);           \
    )                                                      \
                                                           \
    DEF_OPC_3(OPC_ISTORE_0,                                \
              OPC_ASTORE_0,                                \
              OPC_FSTORE_0, level,                         \
        POP_##level(lvars[0], 1);                          \
    )                                                      \
                                                           \
    DEF_OPC_3(OPC_ISTORE_1,                                \
              OPC_ASTORE_1,                                \
              OPC_FSTORE_1, level,                         \
        POP_##level(lvars[1], 1);                          \
    )                                                      \
                                                           \
    DEF_OPC_3(OPC_ISTORE_2,                                \
              OPC_ASTORE_2,                                \
              OPC_FSTORE_2, level,                         \
        POP_##level(lvars[2], 1);                          \
    )                                                      \
                                                           \
    DEF_OPC_3(OPC_ISTORE_3,                                \
              OPC_ASTORE_3,                                \
              OPC_FSTORE_3, level,                         \
        POP_##level(lvars[3], 1);                          \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IADD, level,                               \
        BINARY_OP_##level(+);                              \
    )                                                      \
                                                           \
    DEF_OPC(OPC_ISUB, level,                               \
        BINARY_OP_##level(-);                              \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IMUL, level,                               \
        BINARY_OP_##level(*);                              \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IDIV, level,                               \
        ZERO_DIVISOR_CHECK_##level;                        \
        INTDIV_OP_##level(/);                              \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IREM, level,                               \
        ZERO_DIVISOR_CHECK_##level;                        \
        INTDIV_OP_##level(%);                              \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IAND, level,                               \
        BINARY_OP_##level(&);                              \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IOR, level,                                \
        BINARY_OP_##level(|);                              \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IXOR, level,                               \
        BINARY_OP_##level(^);                              \
    )                                                      \
                                                           \
    DEF_OPC(OPC_INEG, level,                               \
        UNARY_MINUS_##level;                               \
    )                                                      \
                                                           \
    DEF_OPC(OPC_ISHL, level,                               \
        SHIFT_OP_##level(int, <<);                         \
    )                                                      \
                                                           \
    DEF_OPC(OPC_ISHR, level,                               \
        SHIFT_OP_##level(int, >>);                         \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IUSHR, level,                              \
        SHIFT_OP_##level(unsigned int, >>);                \
    )                                                      \
                                                           \
    DEF_OPC_2(OPC_IF_ICMPEQ,                               \
              OPC_IF_ACMPEQ, level,                        \
        IF_ICMP_##level(CMPEQ, ==);                        \
    )                                                      \
                                                           \
    DEF_OPC_2(OPC_IF_ICMPNE,                               \
              OPC_IF_ACMPNE, level,                        \
        IF_ICMP_##level(CMPNE, !=);                        \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IF_ICMPLT, level,                          \
        IF_ICMP_##level(CMPLT, <);                         \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IF_ICMPGE, level,                          \
        IF_ICMP_##level(CMPGE, >=);                        \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IF_ICMPGT, level,                          \
        IF_ICMP_##level(CMPGT, >);                         \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IF_ICMPLE, level,                          \
        IF_ICMP_##level(CMPLE, <=);                        \
    )                                                      \
                                                           \
    DEF_OPC_2(OPC_IFNE,                                    \
              OPC_IFNONNULL, level,                        \
        IF_##level(NE, !=);                                \
    )                                                      \
                                                           \
    DEF_OPC_2(OPC_IFEQ,                                    \
              OPC_IFNULL, level,                           \
        IF_##level(EQ, ==);                                \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IFLT, level,                               \
        IF_##level(LT, <);                                 \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IFGE, level,                               \
        IF_##level(GE, >=);                                \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IFGT, level,                               \
        IF_##level(GT, >);                                 \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IFLE, level,                               \
        IF_##level(LE, <=);                                \
    )                                                      \
                                                           \
    DEF_OPC(OPC_IINC, level,                               \
        lvars[IINC_LVAR_IDX(pc)] += IINC_DELTA(pc);        \
        DISPATCH(level, 3);                                \
    )                                                      \
                                                           \
    DEF_OPC(OPC_POP, level,                                \
        POP1_##level;                                      \
    )                                                      \
                                                           \
    DEF_OPC(OPC_POP2, level,                               \
        ostack -= 2 - level;                               \
        DISPATCH(0, 1);                                    \
    )                                                      \
                                                           \
    DEF_OPC(OPC_DUP, level,                                \
        DUP_##level;                                       \
    )                                                      \
                                                           \
    DEF_OPC_3(OPC_IRETURN,                                 \
              OPC_ARETURN,                                 \
              OPC_FRETURN, level,                          \
        RETURN_##level;                                    \
    )                                                      \
                                                           \
    DEF_OPC(OPC_RETURN, level,                             \
        goto methodReturn;                                 \
    )                                                      \
                                                           \
    MULTI_LEVEL_FIELD_ACCESS(level)

#define FIELD_ACCESS_OPCODES(level, type, suffix)          \
                                                           \
    DEF_OPC(OPC_GETSTATIC_QUICK##suffix, level,            \
        PUSH_##level(*(type*)                              \
           (RESOLVED_FIELD(pc)->u.static_value.data), 3);  \
    )                                                      \
                                                           \
    DEF_OPC(OPC_PUTSTATIC_QUICK##suffix, level,            \
        POP_##level(*(type*)                               \
           (RESOLVED_FIELD(pc)->u.static_value.data), 3);  \
    )                                                      \
                                                           \
    DEF_OPC(OPC_GETFIELD_THIS##suffix, level,              \
        PUSH_##level(INST_DATA(this, type,                 \
                           GETFIELD_THIS_OFFSET(pc)), 4);  \
    )                                                      \
                                                           \
    DEF_OPC(OPC_GETFIELD_QUICK##suffix, level,             \
        GETFIELD_QUICK_##level(SINGLE_INDEX(pc), type);    \
    )

#define MULTI_LEVEL_FIELD_ACCESS(level)                    \
    FIELD_ACCESS_OPCODES(level, u4, /* none */)            \
    FIELD_ACCESS_OPCODES(level, uintptr_t, _REF)

#define ZERO_DIVISOR_CHECK_0                               \
    ZERO_DIVISOR_CHECK((int)ostack[-1]);

#define ZERO_DIVISOR_CHECK_1                               \
    ZERO_DIVISOR_CHECK((int)cache.i.v1);

#define ZERO_DIVISOR_CHECK_2                               \
    ZERO_DIVISOR_CHECK((int)cache.i.v2);

#ifdef USE_CACHE
#define PUSH_0(value, ins_len)                             \
    cache.i.v1 = value;                                    \
    DISPATCH(1, ins_len);
#else
#define PUSH_0(value, ins_len) {                           \
    uintptr_t tos = value;                                 \
    *ostack++ = tos;                                       \
    DISPATCH(0, ins_len);                                  \
}
#endif

#define PUSH_1(value, ins_len)                             \
    cache.i.v2 = value;                                    \
    DISPATCH(2, ins_len);

#define PUSH_2(value, ins_len)                             \
    *ostack++ = cache.i.v1;                                \
    cache.i.v1 = cache.i.v2;                               \
    cache.i.v2 = value;                                    \
    DISPATCH(2, ins_len);

#define POP_0(dest, ins_len)                               \
    dest = *--ostack;                                      \
    DISPATCH(0, ins_len);

#define POP_1(dest, ins_len)                               \
    dest = cache.i.v1;                                     \
    DISPATCH(0, ins_len);

#define POP_2(dest, ins_len)                               \
    dest = cache.i.v2;                                     \
    DISPATCH(1, ins_len);

#define POP1_0                                             \
    ostack--;                                              \
    DISPATCH(0, 1);

#define POP1_1                                             \
    DISPATCH(0, 1);

#define POP1_2                                             \
    DISPATCH(1, 1);

#define DUP_0                                              \
    PUSH_0(ostack[-1], 1);

#define DUP_1                                              \
    PUSH_1(cache.i.v1, 1);

#define DUP_2                                              \
    PUSH_2(cache.i.v2, 1);

#define RETURN_0                                           \
    *lvars++ = *--ostack;                                  \
    goto methodReturn;

#define RETURN_1                                           \
    *lvars++ = cache.i.v1;                                 \
    goto methodReturn;

#define RETURN_2                                           \
    *lvars++ = cache.i.v2;                                 \
    goto methodReturn;

#define GETFIELD_QUICK_0(offset, type)                     \
{                                                          \
    Object *obj = (Object *)*--ostack;                     \
    NULL_POINTER_CHECK(obj);                               \
    PUSH_0(INST_DATA(obj, type, offset), 3);               \
}

#define GETFIELD_QUICK_1(offset, type)                     \
{                                                          \
    Object *obj = (Object *)cache.i.v1;                    \
    NULL_POINTER_CHECK(obj);                               \
    PUSH_0(INST_DATA(obj, type, offset), 3);               \
}

#define GETFIELD_QUICK_2(offset, type)                     \
{                                                          \
    Object *obj = (Object *)cache.i.v2;                    \
    NULL_POINTER_CHECK(obj);                               \
    PUSH_1(INST_DATA(obj, type, offset), 3);               \
}

#define UNARY_MINUS_0                                      \
    PUSH_0(-STACK_POP(int), 1);

#define UNARY_MINUS_1                                      \
    PUSH_0(-(int)cache.i.v1, 1);

#define UNARY_MINUS_2                                      \
    PUSH_1(-(int)cache.i.v2, 1);

#define BINARY_OP_0(OP)                                    \
    ostack -= 2;                                           \
    PUSH_0((int)ostack[0] OP (int)ostack[1], 1);

#define BINARY_OP_1(OP)                                    \
    PUSH_0((int)*--ostack OP (int)cache.i.v1, 1);

#define BINARY_OP_2(OP)                                    \
    PUSH_0((int)cache.i.v1 OP (int)cache.i.v2, 1);

#define INTDIV(OP, dividend, divisor)                      \
{                                                          \
    int v1 = dividend; int v2 = divisor;                   \
    PUSH_0(v1 OP (INTDIV_OVERFLOW(v1, v2) ? 1 : v2), 1);   \
}

#define INTDIV_OP_0(OP)                                    \
    ostack -= 2;                                           \
    INTDIV(OP, ostack[0], ostack[1]);

#define INTDIV_OP_1(OP)                                    \
    INTDIV(OP, *--ostack, cache.i.v1);

#define INTDIV_OP_2(OP)                                    \
    INTDIV(OP, cache.i.v1, cache.i.v2);

#define SHIFT_OP_0(TYPE, OP)                               \
    ostack -= 2;                                           \
    PUSH_0((TYPE)ostack[0] OP (ostack[1] & 0x1f), 1);

#define SHIFT_OP_1(TYPE, OP)                               \
    PUSH_0((TYPE)*--ostack OP (cache.i.v1 & 0x1f), 1);

#define SHIFT_OP_2(TYPE, OP)                               \
    PUSH_0((TYPE)cache.i.v1 OP (cache.i.v2 & 0x1f), 1);

#define IF_ICMP_0(TYPE, COND)                              \
    ostack -= 2;                                           \
    BRANCH(TYPE, 0, (int)ostack[0] COND (int)ostack[1]);

#define IF_ICMP_1(TYPE, COND)                              \
    BRANCH(TYPE, 1, (int)*--ostack COND (int)cache.i.v1);

#define IF_ICMP_2(TYPE, COND)                              \
    BRANCH(TYPE, 2, (int)cache.i.v1 COND (int)cache.i.v2);

#define IF_0(TYPE, COND)                                   \
    BRANCH(TYPE, 0, (int)*--ostack COND 0);

#define IF_1(TYPE, COND)                                   \
    BRANCH(TYPE, 1, (int)cache.i.v1 COND 0);

#define IF_2(TYPE, COND)                                   \
    *ostack++ = cache.i.v1;                                \
    BRANCH(TYPE, 2, (int)cache.i.v2 COND 0);

#ifdef DIRECT
#define ALOAD_THIS(level)

#else /* DIRECT */

#define ALOAD_THIS(level)                                  \
    if(pc[1] != OPC_GETFIELD) {                            \
        int opcode;                                        \
                                                           \
        if(pc[1] == OPC_GETFIELD_QUICK)                    \
            opcode = OPC_GETFIELD_THIS;                    \
        else                                               \
            if(pc[1] == OPC_GETFIELD_QUICK_REF)            \
                opcode = OPC_GETFIELD_THIS_REF;            \
            else                                           \
                opcode = OPC_ILOAD_0;                      \
                                                           \
        OPCODE_REWRITE(opcode);                            \
        DISPATCH(level, 0);                                \
    }
#endif /* DIRECT */

    MULTI_LEVEL_OPCODES(0);

#ifdef USE_CACHE
    MULTI_LEVEL_OPCODES(1);
    MULTI_LEVEL_OPCODES(2);
#endif

    DEF_OPC_210(OPC_NOP,
        DISPATCH(0, 1);
    )

#ifdef USE_CACHE
#define PUSH_LONG(value, ins_len) \
    cache.l = value;              \
    DISPATCH(2, ins_len);
#else
#define PUSH_LONG(value, ins_len) \
    STACK_PUSH(uint64_t, value);  \
    DISPATCH(0, ins_len);
#endif
    
    DEF_OPC_210_2(
            OPC_LCONST_0,
            OPC_DCONST_0,
        PUSH_LONG(0, 1);
    )

    DEF_OPC_210(OPC_DCONST_1,
        PUSH_LONG(DOUBLE_1_BITS, 1);
    )

    DEF_OPC_210(OPC_LCONST_1,
        PUSH_LONG(1, 1);
    )

    DEF_OPC_210(OPC_LDC2_W,
        PUSH_LONG(CP_LONG(cp, DOUBLE_INDEX(pc)), 3);
    )

    DEF_OPC_210_2(
            OPC_LLOAD,
            OPC_DLOAD,
        PUSH_LONG(*(u8*)(&lvars[SINGLE_INDEX(pc)]), 2);
    )

    DEF_OPC_210_2(
            OPC_LLOAD_0,
            OPC_DLOAD_0,
        PUSH_LONG(*(u8*)(&lvars[0]), 1);
    )

    DEF_OPC_210_2(
            OPC_LLOAD_1,
            OPC_DLOAD_1,
        PUSH_LONG(*(u8*)(&lvars[1]), 1);
    )

    DEF_OPC_210_2(
            OPC_LLOAD_2,
            OPC_DLOAD_2,
        PUSH_LONG(*(u8*)(&lvars[2]), 1);
    )

    DEF_OPC_210_2(
            OPC_LLOAD_3,
            OPC_DLOAD_3,
        PUSH_LONG(*(u8*)(&lvars[3]), 1);
    )

#ifdef USE_CACHE
#define POP_LONG(dest, ins_len) \
    dest = cache.l;             \
    DISPATCH(0, ins_len);
#else
#define POP_LONG(dest, ins_len) \
    dest = STACK_POP(uint64_t); \
    DISPATCH(0, ins_len);
#endif

    DEF_OPC_012_2(
            OPC_LSTORE,
            OPC_DSTORE,
        POP_LONG(*(u8*)(&lvars[SINGLE_INDEX(pc)]), 2);
    )

    DEF_OPC_012_2(
            OPC_LSTORE_0,
            OPC_DSTORE_0,
        POP_LONG(*(u8*)(&lvars[0]), 1);
    )

    DEF_OPC_012_2(
            OPC_LSTORE_1,
            OPC_DSTORE_1,
        POP_LONG(*(u8*)(&lvars[1]), 1);
    )

    DEF_OPC_012_2(
            OPC_LSTORE_2,
            OPC_DSTORE_2,
        POP_LONG(*(u8*)(&lvars[2]), 1);
    )

    DEF_OPC_012_2(
            OPC_LSTORE_3,
            OPC_DSTORE_3,
        POP_LONG(*(u8*)(&lvars[3]), 1);
    )

#ifdef USE_CACHE
#define ARRAY_LOAD_IDX cache.i.v2
#define ARRAY_LOAD_ARY cache.i.v1
#else
#define ARRAY_LOAD_IDX *--ostack
#define ARRAY_LOAD_ARY *--ostack
#endif

#define ARRAY_LOAD(TYPE)                       \
{                                              \
    int idx = ARRAY_LOAD_IDX;                  \
    Object *array = (Object *)ARRAY_LOAD_ARY;  \
                                               \
    NULL_POINTER_CHECK(array);                 \
    ARRAY_BOUNDS_CHECK(array, idx);            \
    PUSH_0(ARRAY_DATA(array, TYPE)[idx], 1);   \
}

    DEF_OPC_012_2(
            OPC_IALOAD,
            OPC_FALOAD,
        ARRAY_LOAD(int)
    )

    DEF_OPC_012(OPC_AALOAD,
        ARRAY_LOAD(uintptr_t)
    )

    DEF_OPC_012(OPC_BALOAD,
        ARRAY_LOAD(signed char)
    )

    DEF_OPC_012(OPC_CALOAD,
        ARRAY_LOAD(unsigned short)
    )

    DEF_OPC_012(OPC_SALOAD,
        ARRAY_LOAD(short)
    )

    DEF_OPC_012_2(
            OPC_LALOAD,
            OPC_DALOAD, {
        int idx = ARRAY_LOAD_IDX;
        Object *array = (Object *)ARRAY_LOAD_ARY;

        NULL_POINTER_CHECK(array);
        ARRAY_BOUNDS_CHECK(array, idx);
        PUSH_LONG(ARRAY_DATA(array, u8)[idx], 1);
    })

#ifdef USE_CACHE
#define ARRAY_STORE_VAL cache.i.v2
#define ARRAY_STORE_IDX cache.i.v1
#else
#define ARRAY_STORE_VAL *--ostack
#define ARRAY_STORE_IDX *--ostack
#endif

#define ARRAY_STORE(TYPE)                     \
{                                             \
    int val = ARRAY_STORE_VAL;                \
    int idx = ARRAY_STORE_IDX;                \
    Object *array = (Object *)*--ostack;      \
                                              \
    NULL_POINTER_CHECK(array);                \
    ARRAY_BOUNDS_CHECK(array, idx);           \
    ARRAY_DATA(array, TYPE)[idx] = val;       \
    DISPATCH(0, 1);                           \
}

    DEF_OPC_012_2(
            OPC_IASTORE,
            OPC_FASTORE,
        ARRAY_STORE(int)
    )

    DEF_OPC_012(OPC_BASTORE,
        ARRAY_STORE(char);
    )

    DEF_OPC_012_2(
            OPC_CASTORE,
            OPC_SASTORE,
        ARRAY_STORE(short);
    )

    DEF_OPC_012(OPC_AASTORE, { 
        Object *obj = (Object*)ARRAY_STORE_VAL;
        int idx = ARRAY_STORE_IDX;
        Object *array = (Object *)*--ostack;

        NULL_POINTER_CHECK(array);
        ARRAY_BOUNDS_CHECK(array, idx);

        if((obj != NULL) && !arrayStoreCheck(array->class, obj->class))
            THROW_EXCEPTION(java_lang_ArrayStoreException, NULL);

        ARRAY_DATA(array, Object*)[idx] = obj;
        DISPATCH(0, 1);
    })

#ifdef USE_CACHE
    DEF_OPC_012_2(
            OPC_LASTORE,
            OPC_DASTORE, {
        int idx = ostack[-1];
        Object *array = (Object *)ostack[-2];

        ostack -= 2;
        NULL_POINTER_CHECK(array);
        ARRAY_BOUNDS_CHECK(array, idx);

        ARRAY_DATA(array, u8)[idx] = cache.l;
        DISPATCH(0, 1);
    })
#else
    DEF_OPC_012_2(
            OPC_LASTORE,
            OPC_DASTORE, {
        int idx = ostack[-3];
        Object *array = (Object *)ostack[-4];

        ostack -= 4;
        NULL_POINTER_CHECK(array);
        ARRAY_BOUNDS_CHECK(array, idx);

        ARRAY_DATA(array, u8)[idx] = *(u8*)&ostack[2];
        DISPATCH(0, 1);
    })
#endif

#ifdef USE_CACHE
    DEF_OPC_012(OPC_DUP_X1, {
        *ostack++ = cache.i.v2;
        DISPATCH(2, 1);
    })

    DEF_OPC_012(OPC_DUP_X2, {
        ostack[0] = ostack[-1];
        ostack[-1] = cache.i.v2;
        ostack++;
        DISPATCH(2, 1);
    })

    DEF_OPC_012(OPC_DUP2, {
        *ostack++ = cache.i.v1;
        *ostack++ = cache.i.v2;
        DISPATCH(2, 1);
    })

    DEF_OPC_012(OPC_DUP2_X1, {
        ostack[0]  = cache.i.v2;
        ostack[1]  = ostack[-1];
        ostack[-1] = cache.i.v1;
        ostack += 2;
        DISPATCH(2, 1);
    })

    DEF_OPC_012(OPC_DUP2_X2,
        ostack[0] = ostack[-2];
        ostack[1] = ostack[-1];
        ostack[-2] = cache.i.v1;
        ostack[-1] = cache.i.v2;
        ostack += 2;
        DISPATCH(2, 1);
    )

    DEF_OPC_012(OPC_SWAP, {
        uintptr_t word1 = cache.i.v1;
        cache.i.v1 = cache.i.v2;
        cache.i.v2 = word1;
        DISPATCH(2, 1);
    })
#else /* USE_CACHE */
    DEF_OPC_012(OPC_DUP_X1, {
        uintptr_t word1 = ostack[-1];
        uintptr_t word2 = ostack[-2];
        ostack[-2] = word1;
        ostack[-1] = word2;
        *ostack++ = word1;
        DISPATCH(0, 1);
    })

    DEF_OPC_012(OPC_DUP_X2, {
        uintptr_t word1 = ostack[-1];
        uintptr_t word2 = ostack[-2];
        uintptr_t word3 = ostack[-3];
        ostack[-3] = word1;
        ostack[-2] = word3;
        ostack[-1] = word2;
        *ostack++ = word1;
        DISPATCH(0, 1);
    })

    DEF_OPC_012(OPC_DUP2, {
        ostack[0] = ostack[-2];
        ostack[1] = ostack[-1];
        ostack += 2;
        DISPATCH(0, 1);
    })

    DEF_OPC_012(OPC_DUP2_X1, {
        uintptr_t word1 = ostack[-1];
        uintptr_t word2 = ostack[-2];
        uintptr_t word3 = ostack[-3];
        ostack[-3] = word2;
        ostack[-2] = word1;
        ostack[-1] = word3;
        ostack[0]  = word2;
        ostack[1]  = word1;
        ostack += 2;
        DISPATCH(0, 1);
    })

    DEF_OPC_012(OPC_DUP2_X2, {
        uintptr_t word1 = ostack[-1];
        uintptr_t word2 = ostack[-2];
        uintptr_t word3 = ostack[-3];
        uintptr_t word4 = ostack[-4];
        ostack[-4] = word2;
        ostack[-3] = word1;
        ostack[-2] = word4;
        ostack[-1] = word3;
        ostack[0]  = word2;
        ostack[1]  = word1;
        ostack += 2;
        DISPATCH(0, 1);
    })

    DEF_OPC_012(OPC_SWAP, {
        uintptr_t word1 = ostack[-1];
        ostack[-1] = ostack[-2];
        ostack[-2] = word1;
        DISPATCH(0, 1)
    })
#endif /* USE_CACHE */

#define BINARY_OP_fp(TYPE, OP)             \
    STACK(TYPE, -2) OP##= STACK(TYPE, -1); \
    ostack -= SLOTS(TYPE);                 \
    DISPATCH(0, 1);

    DEF_OPC_FLOAT(OPC_FADD,
        BINARY_OP_fp(float, +);
    )

    DEF_OPC_FLOAT(OPC_DADD,
        BINARY_OP_fp(double, +);
    )

    DEF_OPC_FLOAT(OPC_FSUB,
        BINARY_OP_fp(float, -);
    )

    DEF_OPC_FLOAT(OPC_DSUB,
        BINARY_OP_fp(double, -);
    )

    DEF_OPC_FLOAT(OPC_FMUL,
        BINARY_OP_fp(float, *);
    )

    DEF_OPC_FLOAT(OPC_DMUL,
        BINARY_OP_fp(double, *);
    )

    DEF_OPC_FLOAT(OPC_FDIV,
        BINARY_OP_fp(float, /);
    )

    DEF_OPC_FLOAT(OPC_DDIV,
        BINARY_OP_fp(double, /);
    )

#ifdef USE_CACHE
#define BINARY_OP_long(OP)                   \
    cache.l = STACK_POP(int64_t) OP cache.l; \
    DISPATCH(2, 1);
#else
#define BINARY_OP_long(OP)                   \
    BINARY_OP_fp(int64_t, OP)
#endif

    DEF_OPC_012(OPC_LADD,
        BINARY_OP_long(+);
    )

    DEF_OPC_012(OPC_LSUB,
        BINARY_OP_long(-);
    )

    DEF_OPC_012(OPC_LMUL,
        BINARY_OP_long(*);
    )

    DEF_OPC_012(OPC_LAND,
        BINARY_OP_long(&);
    )

    DEF_OPC_012(OPC_LOR,
        BINARY_OP_long(|);
    )

    DEF_OPC_012(OPC_LXOR,
        BINARY_OP_long(^);
    )

#ifdef USE_CACHE
#define INTDIV_OP_long(OP)                         \
{                                                  \
    int64_t v1 = STACK_POP(int64_t);               \
    cache.l = v1 OP (LONGDIV_OVERFLOW(v1, cache.l) \
                              ? 1 : cache.l);      \
    DISPATCH(2, 1);                                \
}

#define ZERO_DIVISOR_CHECK_long                    \
    ZERO_DIVISOR_CHECK(cache.l);
#else
#define INTDIV_OP_long(OP)                         \
{                                                  \
    int64_t v1 = STACK(int64_t, -2);               \
    int64_t v2 = STACK(int64_t, -1);               \
    STACK(int64_t, -2) = v1 OP                     \
              (LONGDIV_OVERFLOW(v1, v2) ? 1 : v2); \
    ostack -= SLOTS(int64_t);                      \
    DISPATCH(0, 1);                                \
}

#define ZERO_DIVISOR_CHECK_long                    \
    ZERO_DIVISOR_CHECK(STACK(int64_t, -1));
#endif

    DEF_OPC_012(OPC_LDIV,
        ZERO_DIVISOR_CHECK_long;
        INTDIV_OP_long(/);
    )

    DEF_OPC_012(OPC_LREM,
        ZERO_DIVISOR_CHECK_long;
        INTDIV_OP_long(%);
    )

#ifdef USE_CACHE
#define SHIFT_OP_long(TYPE, OP)       \
{                                     \
    int shift = cache.i.v2 & 0x3f;    \
    cache.i.v2 = cache.i.v1;          \
    cache.i.v1 = *--ostack;           \
    cache.l = (TYPE)cache.l OP shift; \
    DISPATCH(2, 1);                   \
}
#else
#define SHIFT_OP_long(TYPE, OP)       \
{                                     \
    int shift = *--ostack & 0x3f;     \
    STACK(TYPE, -1) OP##= shift;      \
    DISPATCH(0, 1);                   \
}
#endif

    DEF_OPC_012(OPC_LSHL,
        SHIFT_OP_long(int64_t, <<);
    )

    DEF_OPC_012(OPC_LSHR,
        SHIFT_OP_long(int64_t, >>);
    )

    DEF_OPC_012(OPC_LUSHR,
        SHIFT_OP_long(uint64_t, >>);
    )

#define FREM(TYPE)                           \
    STACK(TYPE, -2) = fmod(STACK(TYPE, -2),  \
                           STACK(TYPE, -1)); \
    ostack -= SLOTS(TYPE);                   \
    DISPATCH(0, 1);

    DEF_OPC_FLOAT(OPC_FREM,
        FREM(float);
    )

    DEF_OPC_FLOAT(OPC_DREM,
        FREM(double);
    )

#define UNARY_MINUS(TYPE)               \
    STACK(TYPE, -1) = -STACK(TYPE, -1); \
    DISPATCH(0, 1);

    DEF_OPC_FLOAT(OPC_FNEG,
        UNARY_MINUS(float);
    )

    DEF_OPC_FLOAT(OPC_DNEG,
        UNARY_MINUS(double);
    )

    DEF_OPC_012(OPC_LNEG,
#ifdef USE_CACHE
        cache.l = -cache.l;
        DISPATCH(2, 1);
#else
        UNARY_MINUS(int64_t);
#endif
    )

    DEF_OPC_012(OPC_L2I, {
#ifdef USE_CACHE
        int64_t value = cache.l;
#else
        int64_t value = STACK_POP(int64_t);
#endif
        PUSH_0((int)value, 1);
    })

#define OPC_X2Y(SRC_TYPE, DEST_TYPE)         \
{                                            \
    SRC_TYPE value = STACK_POP(SRC_TYPE);    \
    STACK_PUSH(DEST_TYPE, (DEST_TYPE)value); \
    DISPATCH(0, 1);                          \
}

    DEF_OPC_FLOAT(OPC_I2F,
        OPC_X2Y(int, float);
    )

    DEF_OPC_FLOAT(OPC_I2D,
        OPC_X2Y(int, double);
    )

    DEF_OPC_FLOAT(OPC_L2F,
        OPC_X2Y(int64_t, float);
    )

    DEF_OPC_FLOAT(OPC_L2D,
        OPC_X2Y(int64_t, double);
    )

    DEF_OPC_FLOAT(OPC_F2D,
        OPC_X2Y(float, double);
    )

    DEF_OPC_FLOAT(OPC_D2F,
        OPC_X2Y(double, float);
    )

#define OPC_fp2int(SRC_TYPE)              \
{                                         \
    int res;                              \
    SRC_TYPE value = STACK_POP(SRC_TYPE); \
                                          \
    if(value >= (SRC_TYPE)INT_MAX)        \
        res = INT_MAX;                    \
    else if(value <= (SRC_TYPE)INT_MIN)   \
        res = INT_MIN;                    \
    else if(value != value)               \
        res = 0;                          \
    else                                  \
        res = (int) value;                \
                                          \
    PUSH_0(res, 1);                       \
}

    DEF_OPC_FLOAT(OPC_F2I,
        OPC_fp2int(float);
    )

    DEF_OPC_FLOAT(OPC_D2I,
        OPC_fp2int(double);
    )

#define OPC_fp2long(SRC_TYPE)             \
{                                         \
    int64_t res;                          \
    SRC_TYPE value = STACK_POP(SRC_TYPE); \
                                          \
    if(value >= (SRC_TYPE)LLONG_MAX)      \
        res = LLONG_MAX;                  \
    else if(value <= (SRC_TYPE)LLONG_MIN) \
        res = LLONG_MIN;                  \
    else if(value != value)               \
        res = 0;                          \
    else                                  \
        res = (int64_t) value;            \
                                          \
    PUSH_LONG(res, 1);                    \
}

    DEF_OPC_FLOAT(OPC_F2L,
        OPC_fp2long(float);
    )

    DEF_OPC_FLOAT(OPC_D2L,
        OPC_fp2long(double);
    )

    DEF_OPC_210(OPC_I2L,
        PUSH_LONG(STACK_POP(int), 1);
    )

    DEF_OPC_210(OPC_I2B,
        PUSH_0(STACK_POP(int8_t), 1);
    )

    DEF_OPC_210(OPC_I2C,
        PUSH_0(STACK_POP(uint16_t), 1);
    )

    DEF_OPC_210(OPC_I2S,
        PUSH_0(STACK_POP(int16_t), 1);
    )

    DEF_OPC_012(OPC_LCMP, {
#ifdef USE_CACHE
        int64_t v2 = cache.l;
#else
        int64_t v2 = STACK_POP(int64_t);
#endif
        int64_t v1 = STACK_POP(int64_t);
        PUSH_0(v1 == v2 ? 0 : (v1 < v2 ? -1 : 1), 1);
    })

#define FCMP(TYPE, isNan)           \
({                                  \
    int res;                        \
    TYPE v2 = STACK_POP(TYPE);      \
    TYPE v1 = STACK_POP(TYPE);      \
    if(v1 == v2)                    \
        res = 0;                    \
    else if(v1 < v2)                \
        res = -1;                   \
    else if(v1 > v2)                \
         res = 1;                   \
    else                            \
         res = isNan;               \
    PUSH_0(res, 1);                 \
})

    DEF_OPC_FLOAT(OPC_DCMPG,
        FCMP(double, 1);
    )

    DEF_OPC_FLOAT(OPC_DCMPL,
        FCMP(double, -1);
    )

    DEF_OPC_FLOAT(OPC_FCMPG,
        FCMP(float, 1);
    )

    DEF_OPC_FLOAT(OPC_FCMPL,
        FCMP(float, -1);
    )

    DEF_OPC_JMP(GOTO, /* Nothing */)

    DEF_OPC_JMP(JSR,
        *ostack++ = (uintptr_t)pc;
    )

    DEF_OPC_210(OPC_RET,
        pc = (CodePntr)lvars[SINGLE_INDEX(pc)];
        DISPATCH_RET(3);
    )

    DEF_OPC_012_2(
            OPC_LRETURN,
            OPC_DRETURN,
#ifdef USE_CACHE
        *(u8*)lvars = cache.l;
#else
        *(u8*)lvars = STACK_POP(uint64_t);
#endif
        lvars += 2;
        goto methodReturn;
    )

    DEF_OPC_210(OPC_ARRAYLENGTH, {
        Object *array = (Object *)*--ostack;

        NULL_POINTER_CHECK(array);
        PUSH_0(ARRAY_LEN(array), 1);
    })

    DEF_OPC_210(OPC_ATHROW, {
        Object *obj = (Object *)ostack[-1];
        frame->last_pc = pc;
        NULL_POINTER_CHECK(obj);
                
        ee->exception = obj;
        goto throwException;
    })

    DEF_OPC_210(OPC_NEWARRAY, {
        int type = ARRAY_TYPE(pc);
        int count = *--ostack;
        Object *obj;

        frame->last_pc = pc;
        if((obj = allocTypeArray(type, count)) == NULL)
            goto throwException;

        PUSH_0((uintptr_t)obj, 2);
    })

    DEF_OPC_210(OPC_MONITORENTER, {
        Object *obj = (Object *)*--ostack;
        NULL_POINTER_CHECK(obj);
        objectLock(obj);
        DISPATCH(0, 1);
    })

    DEF_OPC_210(OPC_MONITOREXIT, {
        Object *obj = (Object *)*--ostack;
        NULL_POINTER_CHECK(obj);
        objectUnlock(obj);
        DISPATCH(0, 1);
    })

#ifdef DIRECT
    DEF_OPC_RW(OPC_LDC, ({
        int idx, cache;
        Operand operand;

        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_LDC, idx, cache);

        frame->last_pc = pc;

        operand.u = resolveSingleConstant(mb->class, idx);

        if(exceptionOccurred0(ee))
            goto throwException;

        if(CP_TYPE(cp, idx) == CONSTANT_ResolvedClass ||
           CP_TYPE(cp, idx) == CONSTANT_ResolvedString) {
            operand.i = idx;
            OPCODE_REWRITE(OPC_LDC_W_QUICK, cache, operand);
        } else
            OPCODE_REWRITE(OPC_LDC_QUICK, cache, operand);

        REDISPATCH
    });)

    DEF_OPC_210(OPC_TABLESWITCH, {
        SwitchTable *table = (SwitchTable*)pc->operand.pntr;
        int index = *--ostack;

        if(index < table->low || index > table->high)
            pc = table->deflt;
        else
            pc = table->entries[index - table->low];

        DISPATCH_SWITCH
    })

    DEF_OPC_210(OPC_LOOKUPSWITCH, {
        LookupTable *table = (LookupTable*)pc->operand.pntr;
        int key = *--ostack;
        int i;

        for(i = 0; (i < table->num_entries) &&
                   (key != table->entries[i].key); i++);

        pc = (i == table->num_entries ? table->deflt
                                      : table->entries[i].handler);
        DISPATCH_SWITCH
    })

    DEF_OPC_RW(OPC_GETSTATIC, ({
        int idx, cache, opcode;
        FieldBlock *fb;
        Operand operand;
               
        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_GETSTATIC, idx, cache);

        frame->last_pc = pc;
        fb = resolveField(mb->class, idx);

        if(fb != NULL)
            initClass(fb->class);

        if(exceptionOccurred0(ee))
            goto throwException;

        if((*fb->type == 'J') || (*fb->type == 'D'))
            opcode = OPC_GETSTATIC2_QUICK;
        else
            if(*fb->type == 'L' || *fb->type == '[')
                opcode = OPC_GETSTATIC_QUICK_REF;
            else
                opcode = OPC_GETSTATIC_QUICK;

        operand.pntr = fb;
        OPCODE_REWRITE(opcode, cache, operand);

        REDISPATCH
    });)

    DEF_OPC_RW(OPC_PUTSTATIC, ({
        int idx, cache, opcode;
        FieldBlock *fb;
        Operand operand;

        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_PUTSTATIC, idx, cache);

        frame->last_pc = pc;
        fb = resolveField(mb->class, idx);

        if(fb != NULL)
            initClass(fb->class);

        if(exceptionOccurred0(ee))
            goto throwException;

        if((*fb->type == 'J') || (*fb->type == 'D'))
            opcode = OPC_PUTSTATIC2_QUICK;
        else
            if(*fb->type == 'L' || *fb->type == '[')
                opcode = OPC_PUTSTATIC_QUICK_REF;
            else
                opcode = OPC_PUTSTATIC_QUICK;

        operand.pntr = fb;
        OPCODE_REWRITE(opcode, cache, operand);

        REDISPATCH
    });)

    DEF_OPC_RW(OPC_GETFIELD, ({
        int idx, cache, opcode;
        Operand operand;
        FieldBlock *fb;

        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_GETFIELD, idx, cache);

        frame->last_pc = pc;
        fb = resolveField(mb->class, idx);

        if(exceptionOccurred0(ee))
            goto throwException;

        if((*fb->type == 'J') || (*fb->type == 'D'))
            opcode = OPC_GETFIELD2_QUICK;
        else
            if(*fb->type == 'L' || *fb->type == '[')
                opcode = OPC_GETFIELD_QUICK_REF;
            else
                opcode = OPC_GETFIELD_QUICK;

        operand.i = fb->u.offset;
        OPCODE_REWRITE(opcode, cache, operand);

        REDISPATCH
    });)

    DEF_OPC_RW(OPC_PUTFIELD, ({
        int idx, cache, opcode;
        FieldBlock *fb;
        Operand operand;

        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_PUTFIELD, idx, cache);

        frame->last_pc = pc;
        fb = resolveField(mb->class, idx);

        if(exceptionOccurred0(ee))
            goto throwException;

        if((*fb->type == 'J') || (*fb->type == 'D'))
            opcode = OPC_PUTFIELD2_QUICK;
        else
            if(*fb->type == 'L' || *fb->type == '[')
                opcode = OPC_PUTFIELD_QUICK_REF;
            else
                opcode = OPC_PUTFIELD_QUICK;

        operand.i = fb->u.offset;
        OPCODE_REWRITE(opcode, cache, operand);

        REDISPATCH
    });)

    DEF_OPC_RW(OPC_INVOKEVIRTUAL, ({
        int idx, cache;
        Operand operand;

        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_INVOKEVIRTUAL, idx, cache);

        frame->last_pc = pc;
        new_mb = resolveMethod(mb->class, idx);
 
        if(new_mb == NULL || exceptionOccurred0(ee)) {
            if(isPolymorphicRef(mb->class, idx)) {
                PolyMethodBlock *pmb;

                clearException();
                pmb = resolvePolyMethod(mb->class, idx);

                if(pmb == NULL)
                    goto throwException;

                operand.pntr = pmb;
                OPCODE_REWRITE(OPC_INVOKEHANDLE, cache, operand);
            } else
                goto throwException;

        } else if(new_mb->access_flags & ACC_PRIVATE) {
            if(mbPolymorphicNameID(new_mb) == ID_invokeBasic) {
                CACHE_POLY_OFFSETS
                operand.i = new_mb->args_count;
                OPCODE_REWRITE(OPC_INVOKEBASIC, cache, operand);
            } else {
                operand.pntr = new_mb;
                OPCODE_REWRITE(OPC_INVOKENONVIRTUAL_QUICK, cache, operand);
            }
        } else {
            operand.uu.u1 = new_mb->args_count;
            operand.uu.u2 = new_mb->method_table_index;
            OPCODE_REWRITE(OPC_INVOKEVIRTUAL_QUICK, cache, operand);
        }

        REDISPATCH
    });)

    DEF_OPC_RW(OPC_INVOKESPECIAL, ({
        int idx, cache;
        Operand operand;

        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_INVOKESPECIAL, idx, cache);

        frame->last_pc = pc;
        new_mb = resolveMethod(mb->class, idx);
 
        if(exceptionOccurred0(ee))
            goto throwException;

        /* Check if invoking a super method... */
        if((CLASS_CB(mb->class)->access_flags & ACC_SUPER)
              && isSubClassOf(new_mb->class, CLASS_CB(mb->class)->super)
              && (new_mb->name[0] != '<')) {

            operand.i = new_mb->method_table_index;
            OPCODE_REWRITE(OPC_INVOKESUPER_QUICK, cache, operand);
        } else {
            operand.pntr = new_mb;
            OPCODE_REWRITE(OPC_INVOKENONVIRTUAL_QUICK, cache, operand);
        }

        REDISPATCH
    });)

    DEF_OPC_RW(OPC_INVOKESTATIC, ({
        int id, idx, cache;
        Operand operand;

        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_INVOKESTATIC, idx, cache);

        frame->last_pc = pc;
        new_mb = resolveMethod(mb->class, idx);
 
        if(new_mb != NULL)
            initClass(new_mb->class);

        if(exceptionOccurred0(ee))
            goto throwException;

        if((id = mbPolymorphicNameID(new_mb)) >= ID_linkToStatic) {
            int opcode;

            if(id < ID_linkToVirtual)
                 opcode = OPC_LINKTOSPECIAL;
            else
                opcode = OPC_LINKTOVIRTUAL + id - ID_linkToVirtual;

            CACHE_POLY_OFFSETS
            operand.i = new_mb->args_count;
            OPCODE_REWRITE(opcode, cache, operand);
        } else {
            operand.pntr = new_mb;
            OPCODE_REWRITE(OPC_INVOKESTATIC_QUICK, cache, operand);
        }
        REDISPATCH
    });)

    DEF_OPC_RW(OPC_INVOKEINTERFACE, ({
        int idx, cache;
        Operand operand;

        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_INVOKEINTERFACE, idx, cache);

        frame->last_pc = pc;
        new_mb = resolveInterfaceMethod(mb->class, idx);
 
        if(exceptionOccurred0(ee))
            goto throwException;

        if(CLASS_CB(new_mb->class)->access_flags & ACC_INTERFACE) {
            operand.uu.u1 = idx;
            operand.uu.u2 = 0;
            OPCODE_REWRITE(OPC_INVOKEINTERFACE_QUICK, cache, operand);
        } else {
            operand.uu.u1 = new_mb->args_count;
            operand.uu.u2 = new_mb->method_table_index;
            OPCODE_REWRITE(OPC_INVOKEVIRTUAL_QUICK, cache, operand);
        }

        REDISPATCH
    });)

#ifdef JSR292
    DEF_OPC_RW(OPC_INVOKEDYNAMIC, ({
        int idx, cache;
        Operand operand;
        Object *appendix;
        MethodBlock *invoker;
        Thread *self = threadSelf();
        ResolvedInvDynCPEntry *entry;

        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_INVOKEDYNAMIC, idx, cache);

        frame->last_pc = pc;
        entry = resolveInvokeDynamic(mb->class, idx);
        invoker = findInvokeDynamicInvoker(mb->class, entry, &appendix);

        if(invoker == NULL)
            goto throwException;

        resolveLock(self);
        if(!OPCODE_CHANGED(OPC_INVOKEDYNAMIC)) {
            InvDynMethodBlock *idmb = resolveCallSite(entry, invoker,
                                                      appendix);

            operand.pntr = idmb;
            OPCODE_REWRITE(OPC_INVOKEDYNAMIC_QUICK, cache, operand);
        }
        resolveUnlock(self);
        REDISPATCH
    });)
#endif

    DEF_OPC_RW(OPC_MULTIANEWARRAY, ({
        int idx = pc->operand.uui.u1;
        int cache = pc->operand.uui.i;

        frame->last_pc = pc;
        resolveClass(mb->class, idx, TRUE, FALSE);

        if(exceptionOccurred0(ee))
            goto throwException;
        
        OPCODE_REWRITE(OPC_MULTIANEWARRAY_QUICK, cache, pc->operand);
        REDISPATCH
    });)

    DEF_OPC_RW_4(OPC_NEW, OPC_ANEWARRAY, OPC_CHECKCAST, OPC_INSTANCEOF, ({
        int idx = pc->operand.uui.u1;
        int opcode = pc->operand.uui.u2;
        int cache = pc->operand.uui.i;
        Class *class;

        frame->last_pc = pc;
        class = resolveClass(mb->class, idx, TRUE, opcode == OPC_NEW);

        if(exceptionOccurred0(ee))
            goto throwException;
        
        if(opcode == OPC_NEW) {
            ClassBlock *cb = CLASS_CB(class);
            if(cb->access_flags & (ACC_INTERFACE | ACC_ABSTRACT)) {
                signalException(java_lang_InstantiationError, cb->name);
                goto throwException;
            }
        }

        OPCODE_REWRITE((opcode + OPC_NEW_QUICK-OPC_NEW), cache, pc->operand);
        REDISPATCH
    });)
#else /* DIRECT */
    DEF_OPC_210(OPC_LDC, {
        frame->last_pc = pc;

        resolveSingleConstant(mb->class, SINGLE_INDEX(pc));

        if(exceptionOccurred0(ee))
            goto throwException;

        OPCODE_REWRITE(OPC_LDC_QUICK);
        DISPATCH(0, 0);
    })

    DEF_OPC_210(OPC_LDC_W, {
        frame->last_pc = pc;

        resolveSingleConstant(mb->class, DOUBLE_INDEX(pc));

        if(exceptionOccurred0(ee))
            goto throwException;

        OPCODE_REWRITE(OPC_LDC_W_QUICK);
        DISPATCH(0, 0);
    })

    DEF_OPC_210(OPC_ALOAD_0, {
        if(mb->access_flags & ACC_STATIC)
            OPCODE_REWRITE(OPC_ILOAD_0);
        else
            OPCODE_REWRITE(OPC_ALOAD_THIS);
        DISPATCH(0, 0);
    })

    DEF_OPC_210(OPC_TABLESWITCH, {
        int *aligned_pc = (int*)((uintptr_t)(pc + 4) & ~0x3);
        int deflt = ntohl(aligned_pc[0]);
        int low   = ntohl(aligned_pc[1]);
        int high  = ntohl(aligned_pc[2]);
        int index = *--ostack;

        DISPATCH(0, (index < low || index > high) ?
                        deflt : ntohl(aligned_pc[index - low + 3]));
    })

    DEF_OPC_210(OPC_LOOKUPSWITCH, {
        int *aligned_pc = (int*)((uintptr_t)(pc + 4) & ~0x3);
        int deflt  = ntohl(aligned_pc[0]);
        int npairs = ntohl(aligned_pc[1]);
        int key    = *--ostack;
        int i;

        for(i = 2; (i < npairs*2+2) && (key != ntohl(aligned_pc[i])); i += 2);

        DISPATCH(0, i == npairs*2+2 ? deflt : ntohl(aligned_pc[i+1]));
    })

    DEF_OPC_210(OPC_GETSTATIC, {
        FieldBlock *fb;
        int opcode;
               
        frame->last_pc = pc;
        fb = resolveField(mb->class, DOUBLE_INDEX(pc));

        if(fb != NULL)
            initClass(fb->class);

        if(exceptionOccurred0(ee))
            goto throwException;

        if((*fb->type == 'J') || (*fb->type == 'D'))
            opcode = OPC_GETSTATIC2_QUICK;
        else
            if(*fb->type == 'L' || *fb->type == '[')
                opcode = OPC_GETSTATIC_QUICK_REF;
            else
                opcode = OPC_GETSTATIC_QUICK;

        OPCODE_REWRITE(opcode);

        DISPATCH(0, 0);
    })

    DEF_OPC_210(OPC_PUTSTATIC, {
        FieldBlock *fb;
        int opcode;
               
        frame->last_pc = pc;
        fb = resolveField(mb->class, DOUBLE_INDEX(pc));

        if(fb != NULL)
            initClass(fb->class);

        if(exceptionOccurred0(ee))
            goto throwException;

        if((*fb->type == 'J') || (*fb->type == 'D'))
            opcode = OPC_PUTSTATIC2_QUICK;
        else
            if(*fb->type == 'L' || *fb->type == '[')
                opcode = OPC_PUTSTATIC_QUICK_REF;
            else
                opcode = OPC_PUTSTATIC_QUICK;

        OPCODE_REWRITE(opcode);

        DISPATCH(0, 0);
    })

    DEF_OPC_210(OPC_GETFIELD, ({
        int idx;
        FieldBlock *fb;

        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_GETFIELD, idx);

        frame->last_pc = pc;
        fb = resolveField(mb->class, idx);

        if(exceptionOccurred0(ee))
            goto throwException;

        if(fb->u.offset > 255)
            OPCODE_REWRITE(OPC_GETFIELD_QUICK_W);
        else {
            int opcode;

            if((*fb->type == 'J') || (*fb->type == 'D'))
                opcode = OPC_GETFIELD2_QUICK;
            else
                if(*fb->type == 'L' || *fb->type == '[')
                    opcode = OPC_GETFIELD_QUICK_REF;
                else
                    opcode = OPC_GETFIELD_QUICK;

            OPCODE_REWRITE_OPERAND1(opcode, fb->u.offset);
        }

        DISPATCH(0, 0);
    });)

    DEF_OPC_210(OPC_PUTFIELD, {
        int idx;
        FieldBlock *fb;

        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_PUTFIELD, idx);

        frame->last_pc = pc;
        fb = resolveField(mb->class, idx);

        if(exceptionOccurred0(ee))
            goto throwException;

        if(fb->u.offset > 255)
            OPCODE_REWRITE(OPC_PUTFIELD_QUICK_W);
        else {
            int opcode;

            if((*fb->type == 'J') || (*fb->type == 'D'))
                opcode = OPC_PUTFIELD2_QUICK;
            else
                if(*fb->type == 'L' || *fb->type == '[')
                    opcode = OPC_PUTFIELD_QUICK_REF;
                else
                    opcode = OPC_PUTFIELD_QUICK;

            OPCODE_REWRITE_OPERAND1(opcode, fb->u.offset);
        }

        DISPATCH(0, 0);
    })

    DEF_OPC_210(OPC_GETFIELD_QUICK_W, {
        FieldBlock *fb = RESOLVED_FIELD(pc);
        Object *obj = (Object *)*--ostack;

        NULL_POINTER_CHECK(obj);

        if((*fb->type == 'J') || (*fb->type == 'D')) {
            PUSH_LONG(INST_DATA(obj, u8, fb->u.offset), 3);
        } else {
            if(*fb->type == 'L' || *fb->type == '[') {
                PUSH_0(INST_DATA(obj, uintptr_t, fb->u.offset), 3);
            } else {
                PUSH_0(INST_DATA(obj, u4, fb->u.offset), 3);
            }
        }
    })

#ifdef USE_CACHE
    DEF_OPC_012(OPC_PUTFIELD_QUICK_W, {
        FieldBlock *fb = RESOLVED_FIELD(pc);
 
        if((*fb->type == 'J') || (*fb->type == 'D')) {
            Object *obj = (Object *)*--ostack;

            NULL_POINTER_CHECK(obj);
            INST_DATA(obj, u8, fb->u.offset) = cache.l;
        } else {
            Object *obj = (Object *)cache.i.v1;

            NULL_POINTER_CHECK(obj);

            if(*fb->type == 'L' || *fb->type == '[')
                INST_DATA(obj, uintptr_t, fb->u.offset) = cache.i.v2;
            else
                INST_DATA(obj, u4, fb->u.offset) = cache.i.v2;
        }
        DISPATCH(0, 3);
    })
#else
    DEF_OPC_012(OPC_PUTFIELD_QUICK_W, {
        FieldBlock *fb = RESOLVED_FIELD(pc);
 
        if((*fb->type == 'J') || (*fb->type == 'D')) {
            Object *obj = (Object *)ostack[-3];

            ostack -= 3;
            NULL_POINTER_CHECK(obj);
            INST_DATA(obj, u8, fb->u.offset) = *(u8*)&ostack[1];
        } else {
            Object *obj = (Object *)ostack[-2];

            ostack -= 2;
            NULL_POINTER_CHECK(obj);

            if(*fb->type == 'L' || *fb->type == '[')
                INST_DATA(obj, uintptr_t, fb->u.offset) = ostack[1];
            else
                INST_DATA(obj, u4, fb->u.offset) = ostack[1];
        }
        DISPATCH(0, 3);
    })
#endif

    DEF_OPC_210(OPC_INVOKEVIRTUAL, {
        int idx;
        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_INVOKEVIRTUAL, idx);

        frame->last_pc = pc;
        new_mb = resolveMethod(mb->class, idx);
 
        if(new_mb == NULL || exceptionOccurred0(ee)) {
            if(isPolymorphicRef(mb->class, idx)) {
                PolyMethodBlock *pmb;

                clearException();
                pmb = resolvePolyMethod(mb->class, idx);

                if(pmb == NULL)
                    goto throwException;

                OPCODE_REWRITE(OPC_INVOKEHANDLE);
            } else
                goto throwException;

        } else if(new_mb->access_flags & ACC_PRIVATE) {
            if(mbPolymorphicNameID(new_mb) == ID_invokeBasic) {
                CACHE_POLY_OFFSETS
                OPCODE_REWRITE(OPC_INVOKEBASIC);
            } else
                OPCODE_REWRITE(OPC_INVOKENONVIRTUAL_QUICK);

        } else if((new_mb->args_count < 256) &&
                           (new_mb->method_table_index < 256)) {
            OPCODE_REWRITE_OPERAND2(OPC_INVOKEVIRTUAL_QUICK,
                                    new_mb->method_table_index,
                                    new_mb->args_count);
        } else
            OPCODE_REWRITE(OPC_INVOKEVIRTUAL_QUICK_W);

        DISPATCH(0, 0);
    })

    DEF_OPC_210(OPC_INVOKEVIRTUAL_QUICK_W, {
        Class *new_class;

        new_mb = RESOLVED_METHOD(pc);
        arg1 = ostack - new_mb->args_count;
        NULL_POINTER_CHECK(*arg1);

        new_class = (*(Object **)arg1)->class;
        new_mb = CLASS_CB(new_class)->method_table[new_mb->method_table_index];

        goto invokeMethod;
    })

    DEF_OPC_210(OPC_INVOKESPECIAL, {
        int idx;
        WITH_OPCODE_CHANGE_CP_DINDEX(OPC_INVOKESPECIAL, idx);

        frame->last_pc = pc;
        new_mb = resolveMethod(mb->class, idx);
 
        if(exceptionOccurred0(ee))
            goto throwException;

        /* Check if invoking a super method... */
        if((CLASS_CB(mb->class)->access_flags & ACC_SUPER)
              && isSubClassOf(new_mb->class, CLASS_CB(mb->class)->super)
              && (new_mb->name[0] != '<')) {

            OPCODE_REWRITE_OPERAND2(OPC_INVOKESUPER_QUICK,
                    new_mb->method_table_index >> 8,
                    new_mb->method_table_index & 0xff);
        } else
            OPCODE_REWRITE(OPC_INVOKENONVIRTUAL_QUICK);
        DISPATCH(0, 0);
    })

    DEF_OPC_210(OPC_INVOKESTATIC, {
        int id;
        frame->last_pc = pc;
        new_mb = resolveMethod(mb->class, DOUBLE_INDEX(pc));
 
        if(new_mb != NULL)
            initClass(new_mb->class);

        if(exceptionOccurred0(ee))
            goto throwException;

        if((id = mbPolymorphicNameID(new_mb)) >= ID_linkToStatic) {
            int opcode;

            if(id < ID_linkToVirtual)
                 opcode = OPC_LINKTOSPECIAL;
            else
                opcode = OPC_LINKTOVIRTUAL + id - ID_linkToVirtual;

            CACHE_POLY_OFFSETS
            OPCODE_REWRITE(opcode);
        } else
            OPCODE_REWRITE(OPC_INVOKESTATIC_QUICK);

        DISPATCH(0, 0);
    })

    DEF_OPC_210(OPC_INVOKEINTERFACE, {
        frame->last_pc = pc;
        new_mb = resolveInterfaceMethod(mb->class, DOUBLE_INDEX(pc));
 
        if(exceptionOccurred0(ee))
            goto throwException;

        if(CLASS_CB(new_mb->class)->access_flags & ACC_INTERFACE)
            OPCODE_REWRITE(OPC_INVOKEINTERFACE_QUICK);
        else {
            pc[3] = pc[4] = OPC_NOP;
            OPCODE_REWRITE(OPC_INVOKEVIRTUAL);
        }

        DISPATCH(0, 0);
    })

#ifdef JSR292
    DEF_OPC_210(OPC_INVOKEDYNAMIC, {
        Object *appendix;
        MethodBlock *invoker;
        Thread *self = threadSelf();
        ResolvedInvDynCPEntry *entry;

        frame->last_pc = pc;
        entry = resolveInvokeDynamic(mb->class, DOUBLE_INDEX(pc));
        invoker = findInvokeDynamicInvoker(mb->class, entry, &appendix);

        if(invoker == NULL)
            goto throwException;

        resolveLock(self);
        if(!OPCODE_CHANGED(OPC_INVOKEDYNAMIC)) {
            InvDynMethodBlock *idmb = resolveCallSite(entry, invoker,
                                                      appendix);

            pc[3] = idmb->id;
            OPCODE_REWRITE(OPC_INVOKEDYNAMIC_QUICK);
        }
        resolveUnlock(self);
        DISPATCH(0, 0);
    })
#endif

#define REWRITE_RESOLVE_CLASS(opcode)                                     \
    DEF_OPC_210(opcode, {                                                 \
        frame->last_pc = pc;                                              \
        resolveClass(mb->class, DOUBLE_INDEX(pc), TRUE, FALSE);           \
                                                                          \
        if(exceptionOccurred0(ee))                                        \
            goto throwException;                                          \
                                                                          \
        OPCODE_REWRITE((opcode + OPC_ANEWARRAY_QUICK-OPC_ANEWARRAY));     \
        DISPATCH(0, 0);                                                   \
    })

   REWRITE_RESOLVE_CLASS(OPC_ANEWARRAY)
   REWRITE_RESOLVE_CLASS(OPC_CHECKCAST)
   REWRITE_RESOLVE_CLASS(OPC_INSTANCEOF)
   REWRITE_RESOLVE_CLASS(OPC_MULTIANEWARRAY)

    DEF_OPC_210(OPC_NEW, {
        Class *class;
        ClassBlock *cb;

        frame->last_pc = pc;
        class = resolveClass(mb->class, DOUBLE_INDEX(pc), TRUE, TRUE);

        if(exceptionOccurred0(ee))
            goto throwException;
        
        cb = CLASS_CB(class);
        if(cb->access_flags & (ACC_INTERFACE | ACC_ABSTRACT)) {
            signalException(java_lang_InstantiationError, cb->name);
            goto throwException;
        }

        OPCODE_REWRITE(OPC_NEW_QUICK);
        DISPATCH(0, 0);
    })

    DEF_OPC_210(OPC_WIDE, {
       int opcode = pc[1];
        switch(opcode) {
            case OPC_ILOAD:
            case OPC_FLOAD:
            case OPC_ALOAD:
                *ostack++ = lvars[DOUBLE_INDEX(pc+1)];
                pc += 4;
                break;

            case OPC_LLOAD:
            case OPC_DLOAD:
                *(u8*)ostack = *(u8*)(&lvars[DOUBLE_INDEX(pc+1)]);
                ostack += 2;
                pc += 4;
                break;

            case OPC_ISTORE:
            case OPC_FSTORE:
            case OPC_ASTORE:
                lvars[DOUBLE_INDEX(pc+1)] = *--ostack;
                pc += 4;
                break;

            case OPC_LSTORE:
            case OPC_DSTORE:
                ostack -= 2;
                *(u8*)(&lvars[DOUBLE_INDEX(pc+1)]) = *(u8*)ostack;
                pc += 4;
                break;

            case OPC_RET:
                pc = (unsigned char*)lvars[DOUBLE_INDEX((pc+1))];
                break;

            case OPC_IINC:
                lvars[DOUBLE_INDEX(pc+1)] += DOUBLE_SIGNED(pc+3);
                pc += 6;
                break;
        }
        DISPATCH(0, 0);
    })

    DEF_OPC_210(OPC_GOTO_W, {
        DISPATCH(0, READ_S4_OP(pc));
    })

    DEF_OPC_210(OPC_JSR_W, {
        PUSH_0((uintptr_t)pc + 2, READ_S4_OP(pc));
    })

    DEF_OPC_210(OPC_LOCK, {
        DISPATCH(0, 0);
    })
#endif /* DIRECT */

    DEF_OPC_210(OPC_GETSTATIC2_QUICK, {
        FieldBlock *fb = RESOLVED_FIELD(pc);
        PUSH_LONG(fb->u.static_value.l, 3);
    })

    DEF_OPC_012(OPC_PUTSTATIC2_QUICK, {
        FieldBlock *fb = RESOLVED_FIELD(pc);
        POP_LONG(fb->u.static_value.l, 3);
    })

    DEF_OPC_210(OPC_GETFIELD2_QUICK, {
        Object *obj = (Object *)*--ostack;
        NULL_POINTER_CHECK(obj);
                
        PUSH_LONG(INST_DATA(obj, u8, SINGLE_INDEX(pc)), 3);
    })

#ifdef USE_CACHE
    DEF_OPC_012(OPC_PUTFIELD2_QUICK, {
        Object *obj = (Object *)*--ostack;
        NULL_POINTER_CHECK(obj);

        INST_DATA(obj, u8, SINGLE_INDEX(pc)) = cache.l;
        DISPATCH(0, 3);
    })

#define PUTFIELD_QUICK(type, suffix)                         \
    DEF_OPC_012(OPC_PUTFIELD_QUICK##suffix, {                \
        Object *obj = (Object *)cache.i.v1;                  \
        NULL_POINTER_CHECK(obj);                             \
                                                             \
        INST_DATA(obj, type, SINGLE_INDEX(pc)) = cache.i.v2; \
        DISPATCH(0, 3);                                      \
    })
#else
    DEF_OPC_012(OPC_PUTFIELD2_QUICK, {
        Object *obj = (Object *)ostack[-3];

        ostack -= 3;
        NULL_POINTER_CHECK(obj);
        INST_DATA(obj, u8, SINGLE_INDEX(pc)) = *(u8*)&ostack[1];
        DISPATCH(0, 3);
    })

#define PUTFIELD_QUICK(type, suffix)                        \
    DEF_OPC_012(OPC_PUTFIELD_QUICK##suffix, {               \
        Object *obj = (Object *)ostack[-2];                 \
                                                            \
        ostack -= 2;                                        \
        NULL_POINTER_CHECK(obj);                            \
        INST_DATA(obj, type, SINGLE_INDEX(pc)) = ostack[1]; \
        DISPATCH(0, 3);                                     \
    })
#endif

    PUTFIELD_QUICK(u4, /* none */)
    PUTFIELD_QUICK(uintptr_t, _REF)

    DEF_OPC_210(OPC_INVOKESUPER_QUICK, {
        new_mb = CLASS_CB(CLASS_CB(mb->class)->super)->
                                      method_table[DOUBLE_INDEX(pc)];
        arg1 = ostack - new_mb->args_count;
        NULL_POINTER_CHECK(*arg1);
        goto invokeMethod;
    })

    DEF_OPC_210(OPC_INVOKENONVIRTUAL_QUICK, {
        new_mb = RESOLVED_METHOD(pc);
        arg1 = ostack - new_mb->args_count;
        NULL_POINTER_CHECK(*arg1);
        goto invokeMethod;
    })

    DEF_OPC_210(OPC_INVOKESTATIC_QUICK, {
        new_mb = RESOLVED_METHOD(pc);
        arg1 = ostack - new_mb->args_count;
        goto invokeMethod;
    })

    DEF_OPC_210(OPC_INVOKEINTERFACE_QUICK, {
        int mtbl_idx;
        ClassBlock *cb;
        int cache = INV_INTF_CACHE(pc);

        new_mb = (MethodBlock *)CP_INFO(cp, INV_INTF_IDX(pc));
        arg1 = ostack - new_mb->args_count;

        NULL_POINTER_CHECK(*arg1);

        cb = CLASS_CB((*(Object **)arg1)->class);

        if(cache >= cb->imethod_table_size ||
                  new_mb->class != cb->imethod_table[cache].interface) {
            for(cache = 0; cache < cb->imethod_table_size &&
                           new_mb->class != cb->imethod_table[cache].interface;
                cache++);

            if(cache == cb->imethod_table_size)
                THROW_EXCEPTION(java_lang_IncompatibleClassChangeError,
                                 "unimplemented interface");

            INV_INTF_CACHE(pc) = cache;
        }

        mtbl_idx = cb->imethod_table[cache].offsets[new_mb->method_table_index];
        new_mb = cb->method_table[mtbl_idx];

        goto invokeMethod;
    })

#ifdef JSR292
    DEF_OPC_210(OPC_INVOKEHANDLE, {
        PolyMethodBlock *pmb = RESOLVED_POLYMETHOD(pc);

        if(pmb->appendix)
            *ostack++ = (uintptr_t)pmb->appendix;

        new_mb = pmb->invoker;
        arg1 = ostack - new_mb->args_count;
        NULL_POINTER_CHECK(*arg1);

        goto invokeMethod;
    })

    DEF_OPC_210(OPC_INVOKEDYNAMIC_QUICK, {
        InvDynMethodBlock *idmb = RESOLVED_INVDYNMETHOD(pc);

        if(idmb->appendix)
            *ostack++ = (uintptr_t)idmb->appendix;

        new_mb = idmb->invoker;
        arg1 = ostack - new_mb->args_count;

        goto invokeMethod;
    })

    DEF_OPC_210(OPC_INVOKEBASIC, {
        arg1 = ostack - INTRINSIC_ARGS(pc);
        new_mb = getInvokeBasicTarget((Object*)*arg1);

        goto invokeMethod;
    })

    DEF_OPC_210(OPC_LINKTOSPECIAL, {
        new_mb = getLinkToSpecialTarget((Object*)ostack[-1]);
        arg1 = ostack - INTRINSIC_ARGS(pc);

        goto invokeMethod;
    })

    DEF_OPC_210(OPC_LINKTOVIRTUAL, {
        Object *mem_name = (Object*)ostack[-1];

        arg1 = ostack - INTRINSIC_ARGS(pc);
        new_mb = getLinkToVirtualTarget((Object*)*arg1, mem_name);

        goto invokeMethod;
    })

    DEF_OPC_210(OPC_LINKTOINTERFACE, {
        Object *mem_name = (Object*)ostack[-1];

        arg1 = ostack - INTRINSIC_ARGS(pc);
        new_mb = getLinkToInterfaceTarget((Object*)*arg1, mem_name);

        if(new_mb == NULL)
            goto throwException;

        goto invokeMethod;
    })
#endif

    DEF_OPC_210(OPC_NEW_QUICK, {
        Class *class = RESOLVED_CLASS(pc);
        Object *obj;

        frame->last_pc = pc;
        if((obj = allocObject(class)) == NULL)
            goto throwException;

        PUSH_0((uintptr_t)obj, 3);
    })
 
    DEF_OPC_210(OPC_ANEWARRAY_QUICK, {
        Class *class = RESOLVED_CLASS(pc);
        int count = *--ostack;
        Object *obj;

        frame->last_pc = pc;

        if(count < 0) {
            signalException(java_lang_NegativeArraySizeException, NULL);
            goto throwException;
        }

        obj = allocObjectArray(class, count);
        if(obj == NULL)
            goto throwException;

        PUSH_0((uintptr_t)obj, 3);
    })

    DEF_OPC_210(OPC_CHECKCAST_QUICK, {
        Class *class = RESOLVED_CLASS(pc);
        Object *obj = (Object*)ostack[-1]; 
               
        if((obj != NULL) && !isInstanceOf(class, obj->class))
            THROW_EXCEPTION(java_lang_ClassCastException,
                            CLASS_CB(obj->class)->name);
    
        DISPATCH(0, 3);
    })

    DEF_OPC_210(OPC_INSTANCEOF_QUICK, {
        Class *class = RESOLVED_CLASS(pc);
        Object *obj = (Object*)ostack[-1]; 
               
        if(obj != NULL)
            ostack[-1] = isInstanceOf(class, obj->class); 

        DISPATCH(0, 3);
    })

    DEF_OPC_210(OPC_MULTIANEWARRAY_QUICK, ({
        Class *class = RESOLVED_CLASS(pc);
        int i, dim = MULTI_ARRAY_DIM(pc);
        Object *obj;

        ostack -= dim;
        frame->last_pc = pc;

        for(i = 0; i < dim; i++)
            if((intptr_t)ostack[i] < 0) {
                signalException(java_lang_NegativeArraySizeException, NULL);
                goto throwException;
            }

        if((obj = allocMultiArray(class, dim, (intptr_t *)ostack)) == NULL)
            goto throwException;

        PUSH_0((uintptr_t)obj, 4);
    });)

    /* Special bytecode which forms the body of a Miranda method.
       If it is invoked it executes the interface method that it
       represents. */

    DEF_OPC_210(OPC_MIRANDA_BRIDGE, {
        frame->mb = mb = mb->miranda_mb;
        PREPARE_MB(mb);

        pc = (CodePntr)mb->code;
        cp = &(CLASS_CB(mb->class)->constant_pool);

        DISPATCH_FIRST
    })

    /* Special bytecode which forms the body of an abstract method.
       If it is invoked it'll throw an abstract method exception. */

    /* A class which has conflicting default interface methods also
       throws an abstract method exception if the method is invoked */

    #define MESSAGE ": conflicting default methods"

    DEF_OPC_210(OPC_ABSTRACT_METHOD_ERROR, {
        /* As the method has been invoked, a frame will exist for
           the abstract method itself.  Pop this to get the correct
           exception stack trace. */
        ee->last_frame = frame->prev;

        /* Throw the exception */
        if(mb->flags & MB_DEFAULT_CONFLICT) {
            char buff[strlen(mb->name) + sizeof(MESSAGE)];

            strcat(strcpy(buff, mb->name), MESSAGE);
            signalException(java_lang_AbstractMethodError, buff);
        } else
            signalException(java_lang_AbstractMethodError, mb->name);

        goto throwException;
    })

#ifdef INLINING
    DEF_OPC_RW(OPC_INLINE_REWRITER, ({
        inlineBlockWrappedOpcode(mb, pc);
    });)

    DEF_OPC_RW(OPC_PROFILE_REWRITER, ({
        void *handler = inlineProfiledBlock(pc, mb, FALSE);

        if(handler != NULL)
            goto *handler;
    });)
#endif

    DEF_OPC_210(OPC_INVOKEVIRTUAL_QUICK, {
        Class *new_class;

        arg1 = ostack - INV_QUICK_ARGS(pc);
        NULL_POINTER_CHECK(*arg1);

        new_class = (*(Object **)arg1)->class;
        new_mb = CLASS_CB(new_class)->method_table[INV_QUICK_IDX(pc)];

        goto invokeMethod;
    })

invokeMethod:
{
    /* Create new frame first.  This is also created for natives
       so that they appear correctly in the stack trace */

    Frame *new_frame = (Frame *)(arg1 + new_mb->max_locals);
    Object *sync_ob = NULL;

    frame->last_pc = pc;
    ostack = ALIGN_OSTACK(new_frame + 1);

    if((char*)(ostack + new_mb->max_stack) > ee->stack_end) {
        if(ee->overflow++) {
            /* Overflow when we're already throwing stack overflow.
               Stack extension should be enough to throw exception,
               so something's seriously gone wrong - abort the VM! */
            jam_printf("Fatal stack overflow!  Aborting VM.\n");
            exitVM(1);
        }
        ee->stack_end += STACK_RED_ZONE_SIZE;
        THROW_EXCEPTION(java_lang_StackOverflowError, NULL);
    }

    new_frame->mb = new_mb;
    new_frame->lvars = arg1;
    new_frame->ostack = ostack;
    new_frame->prev = frame;

    ee->last_frame = new_frame;

    if(new_mb->access_flags & ACC_SYNCHRONIZED) {
        sync_ob = new_mb->access_flags & ACC_STATIC ? (Object*)new_mb->class
                                                    : (Object*)*arg1;
        objectLock(sync_ob);
    }

    if(new_mb->access_flags & ACC_NATIVE) {
        ostack = (*new_mb->native_invoker)(new_mb->class, new_mb, arg1);

        if(sync_ob)
            objectUnlock(sync_ob);

        ee->last_frame = frame;

        if(exceptionOccurred0(ee))
            goto throwException;
        DISPATCH(0, *pc >= OPC_INVOKEINTERFACE_QUICK ? 5 : 3);
    } else {
        PREPARE_MB(new_mb);

        frame = new_frame;
        mb = new_mb;
        lvars = new_frame->lvars;
        this = (Object*)lvars[0];
        pc = (CodePntr)mb->code;
        cp = &(CLASS_CB(mb->class)->constant_pool);
    }
    DISPATCH_FIRST
}

methodReturn:
    /* Set interpreter state to previous frame */
    frame = frame->prev;

    if(frame->mb == NULL) {
        /* The previous frame is a dummy frame - this indicates
           top of this Java invocation. */
        return ostack;
    }

    if(mb->access_flags & ACC_SYNCHRONIZED) {
        Object *sync_ob = mb->access_flags & ACC_STATIC ? (Object*)mb->class
                                                        : this;
        objectUnlock(sync_ob);
    }

    mb = frame->mb;
    ostack = lvars;
    lvars = frame->lvars;
    this = (Object*)lvars[0];
    pc = frame->last_pc;
    cp = &(CLASS_CB(mb->class)->constant_pool);

    /* Pop frame */ 
    ee->last_frame = frame;

    DISPATCH_METHOD_RET(*pc >= OPC_INVOKEINTERFACE_QUICK ? 5 : 3);

#ifdef INLINING
    DEF_DUMMY_HANDLERS

throwNull:
    THROW_EXCEPTION(java_lang_NullPointerException, NULL);

throwArithmeticExcep:
    THROW_EXCEPTION(java_lang_ArithmeticException, "division by zero");

throwOOB:
    {
        char buff[MAX_INT_DIGITS];
        snprintf(buff, MAX_INT_DIGITS, "%d", oob_array_index);
        THROW_EXCEPTION(java_lang_ArrayIndexOutOfBoundsException, buff);
    }
#endif

throwException:
    {
        Object *excep = ee->exception;
        ee->exception = NULL;

        pc = findCatchBlock(excep->class);

        /* If we didn't find a handler, restore exception and
           return to previous invocation */

        if(pc == NULL) {
            ee->exception = excep;
            return NULL;
        }

        /* If we're handling a stack overflow, reduce the stack
           back past the red zone to enable handling of further
           overflows */

        if(ee->overflow) {
            ee->overflow = FALSE;
            ee->stack_end -= STACK_RED_ZONE_SIZE;
        }

        /* Setup interpreter to run the found catch block */

        frame = ee->last_frame;
        mb = frame->mb;
        ostack = frame->ostack;
        lvars = frame->lvars;
        this = (Object*)lvars[0];
        cp = &(CLASS_CB(mb->class)->constant_pool);

        *ostack++ = (uintptr_t)excep;

        /* Dispatch to the first bytecode */

        DISPATCH_FIRST
    }
#ifndef THREADED
  }}
#endif
}

#ifndef executeJava
int initialiseInterpreter(InitArgs *args) {
#ifdef DIRECT
    initialiseDirect(args);
#endif
    return TRUE;
}

void shutdownInterpreter() {
#ifdef INLINING
    shutdownInlining();
#endif
}
#endif

