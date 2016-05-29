/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2013, 2014
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

#ifndef THREADED
#error Direct interpreter cannot be built non-threaded
#endif

#include "interp-threading.h"
#include "interp-direct-common.h"

#ifdef PREFETCH
#define INTERPRETER_DEFINITIONS                            \
    DEFINE_HANDLER_TABLES                                  \
    const void *next_handler;                              \
    static const void **handlers[] = {HNDLR_TBLS(ENTRY)};
#else
#define INTERPRETER_DEFINITIONS                            \
    DEFINE_HANDLER_TABLES                                  \
    static const void **handlers[] = {HNDLR_TBLS(ENTRY)};
#endif

#define INTERPRETER_PROLOGUE                               \
unused:                                                    \
rewrite_lock:                                              \
    DISPATCH_FIRST

#define I(opcode, level, label) &&unused
#define D(opcode, level, label) &&unused
#define X(opcode, level, label) L(opcode, level, label)

#define DEF_HANDLER_TABLES(level)                          \
    DEF_HANDLER_TABLE(level, ENTRY);

/* Macros for handler/bytecode rewriting */

#define OPCODE_REWRITE(opcode, cache, new_operand)         \
{                                                          \
    pc->handler = &&rewrite_lock;                          \
    MBARRIER();                                            \
    pc->operand = new_operand;                             \
    MBARRIER();                                            \
    pc->handler = handlers[cache][opcode];                 \
}

/* Two levels of macros are needed to correctly produce the label
 * from the OPC_xxx macro passed into DEF_OPC as cpp doesn't 
 * prescan when concatenating with ##...
 *
 * On gcc <= 2.95, we also get a space inserted before the :
 * e.g DEF_OPC(OPC_NULL) -> opc0 : - the ##: is a hack to fix
 * this, but this generates warnings on >= 2.96...
 */
#if (__GNUC__ == 2) && (__GNUC_MINOR__ <= 95)
#define label(x, y, z)                          \
opc##x##_##y##_##z##:
#else
#define label(x, y, z)                          \
opc##x##_##y##_##z:
#endif

#define DEF_OPC(opcode, level, BODY)            \
    label(opcode, level, ENTRY)                 \
        BODY

#define DEF_OPC_2(op1, op2, level, BODY)        \
    label(op1, level, ENTRY)                    \
    label(op2, level, ENTRY)                    \
        BODY

#define DEF_OPC_3(op1, op2, op3, level, BODY)   \
    label(op1, level, ENTRY)                    \
    label(op2, level, ENTRY)                    \
    label(op3, level, ENTRY)                    \
        BODY

#define DEF_OPC_JMP(TYPE, BODY)                 \
    DEF_OPC_210(OPC_##TYPE, ({                  \
        BODY                                    \
        BRANCH(TYPE, 0, TRUE);                  \
    });)

#define DEF_OPC_FLOAT(opcode, BODY)             \
    DEF_OPC_210(opcode, BODY)

#ifdef USE_CACHE
#define DEF_OPC_012(opcode, BODY)               \
    label(opcode, 0, ENTRY)                     \
        cache.i.v1 = *--ostack;                 \
                                                \
    label(opcode, 1, ENTRY)                     \
        cache.i.v2 = cache.i.v1;                \
        cache.i.v1 = *--ostack;                 \
                                                \
    label(opcode, 2, ENTRY)                     \
        BODY
        
#define DEF_OPC_012_2(op1, op2, BODY)           \
    label(op1, 0, ENTRY)                        \
    label(op2, 0, ENTRY)                        \
        cache.i.v1 = *--ostack;                 \
                                                \
    label(op1, 1, ENTRY)                        \
    label(op2, 1, ENTRY)                        \
        cache.i.v2 = cache.i.v1;                \
        cache.i.v1 = *--ostack;                 \
                                                \
    label(op1, 2, ENTRY)                        \
    label(op2, 2, ENTRY)                        \
        BODY

#define DEF_OPC_012_3(op1, op2, op3, BODY)      \
    label(op1, 0, ENTRY)                        \
    label(op2, 0, ENTRY)                        \
    label(op3, 0, ENTRY)                        \
        cache.i.v1 = *--ostack;                 \
                                                \
    label(op1, 1, ENTRY)                        \
    label(op2, 1, ENTRY)                        \
    label(op3, 1, ENTRY)                        \
        cache.i.v2 = cache.i.v1;                \
        cache.i.v1 = *--ostack;                 \
                                                \
    label(op1, 2, ENTRY)                        \
    label(op2, 2, ENTRY)                        \
    label(op3, 2, ENTRY)                        \
        BODY

#define DEF_OPC_012_4(op1, op2, op3, op4, BODY) \
    label(op1, 0, ENTRY)                        \
    label(op2, 0, ENTRY)                        \
    label(op3, 0, ENTRY)                        \
    label(op4, 0, ENTRY)                        \
        cache.i.v1 = *--ostack;                 \
                                                \
    label(op1, 1, ENTRY)                        \
    label(op2, 1, ENTRY)                        \
    label(op3, 1, ENTRY)                        \
    label(op4, 1, ENTRY)                        \
        cache.i.v2 = cache.i.v1;                \
        cache.i.v1 = *--ostack;                 \
                                                \
    label(op1, 2, ENTRY)                        \
    label(op2, 2, ENTRY)                        \
    label(op3, 2, ENTRY)                        \
    label(op4, 2, ENTRY)                        \
        BODY

#define DEF_OPC_210(opcode, BODY)               \
    label(opcode, 2, ENTRY)                     \
        *ostack++ = cache.i.v1;                 \
        cache.i.v1 = cache.i.v2;                \
                                                \
    label(opcode, 1, ENTRY)                     \
        *ostack++ = cache.i.v1;                 \
                                                \
    label(opcode, 0, ENTRY)                     \
        BODY
        
#define DEF_OPC_210_2(op1, op2, BODY)           \
    label(op1, 2, ENTRY)                        \
    label(op2, 2, ENTRY)                        \
        *ostack++ = cache.i.v1;                 \
        cache.i.v1 = cache.i.v2;                \
                                                \
    label(op1, 1, ENTRY)                        \
    label(op2, 1, ENTRY)                        \
        *ostack++ = cache.i.v1;                 \
                                                \
    label(op1, 0, ENTRY)                        \
    label(op2, 0, ENTRY)                        \
        BODY

#define LABELS(opcode)                          \
    label(opcode, 0, ENTRY)                     \
    label(opcode, 1, ENTRY)                     \
    label(opcode, 2, ENTRY)

#define DEF_OPC_RW(opcode, BODY)                \
    LABELS(opcode)                              \
        BODY

#define DEF_OPC_RW_4(op1, op2, op3, op4, BODY)  \
    LABELS(op1)                                 \
    LABELS(op2)                                 \
    LABELS(op3)                                 \
    LABELS(op4)                                 \
        BODY

#else /* USE_CACHE */

#define DEF_OPC_012(opcode, BODY)               \
    DEF_OPC(opcode, 0, BODY)

#define DEF_OPC_012_2(op1, op2, BODY)           \
    DEF_OPC_2(op1, op2, 0, BODY)

#define DEF_OPC_012_3(op1, op2, op3, BODY)      \
    DEF_OPC_3(op1, op2, op3, 0, BODY)

#define DEF_OPC_012_4(op1, op2, op3, op4, BODY) \
    label(op1, 0, ENTRY)                        \
    label(op2, 0, ENTRY)                        \
    label(op3, 0, ENTRY)                        \
    label(op4, 0, ENTRY)                        \
        BODY

#define DEF_OPC_210(opcode, BODY)               \
    DEF_OPC_012(opcode, ({BODY});)

#define DEF_OPC_210_2(op1, op2, BODY)           \
    DEF_OPC_012_2(op1, op2, ({BODY});)

#define DEF_OPC_RW(opcode, BODY)                \
    DEF_OPC_012(opcode, ({BODY});)

#define DEF_OPC_RW_4(op1, op2, op3, op4, BODY)  \
    DEF_OPC_012_4(op1, op2, op3, op4, ({BODY});)

#endif /* USE_CACHE */

#ifdef PREFETCH
#define DISPATCH_FIRST                          \
{                                               \
    next_handler = pc[1].handler;               \
    REDISPATCH                                  \
}

#define DISPATCH(level, ins_len)                \
{                                               \
    const void *dispatch = next_handler;        \
    next_handler = (++pc)[1].handler;           \
    goto *dispatch;                             \
}
#else
#define DISPATCH_FIRST                          \
    REDISPATCH

#define DISPATCH(level, ins_len)                \
    goto *(++pc)->handler;

#endif /* PREFETCH */

#define BRANCH(type, level, TEST)               \
    if(TEST) {                                  \
        pc = (Instruction*)pc->operand.pntr;    \
        DISPATCH_FIRST                          \
    } else                                      \
        DISPATCH(0,0)

#define REDISPATCH                              \
    goto *pc->handler;

#define DISPATCH_RET(ins_len)                   \
    pc++;                                       \
    DISPATCH_FIRST

#define DISPATCH_METHOD_RET(ins_len)            \
    DISPATCH_RET(ins_len)

#define DISPATCH_SWITCH                         \
    DISPATCH_FIRST

/* Macros for checking for common exceptions */

#define THROW_EXCEPTION(excep_enum, message)                               \
{                                                                          \
    frame->last_pc = pc;                                                   \
    signalException(excep_enum, message);                                  \
    goto throwException;                                                   \
}

#define NULL_POINTER_CHECK(ref)                                            \
    if(!ref) THROW_EXCEPTION(java_lang_NullPointerException, NULL);

#define MAX_INT_DIGITS 11

#define ARRAY_BOUNDS_CHECK(array, idx)                                     \
{                                                                          \
    if(idx >= ARRAY_LEN(array)) {                                          \
        char buff[MAX_INT_DIGITS];                                         \
        snprintf(buff, MAX_INT_DIGITS, "%d", idx);                         \
        THROW_EXCEPTION(java_lang_ArrayIndexOutOfBoundsException, buff);   \
    }                                                                      \
}

#define ZERO_DIVISOR_CHECK(value)                                          \
    if(value == 0)                                                         \
        THROW_EXCEPTION(java_lang_ArithmeticException, "division by zero");


extern void initialiseDirect(InitArgs *args);
extern void prepare(MethodBlock *mb, const void ***handlers);
