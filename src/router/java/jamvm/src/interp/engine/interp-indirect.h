/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2012, 2013, 2014
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

#ifdef THREADED
#include "interp-threading.h"

#define INTERPRETER_DEFINITIONS                            \
    DEFINE_HANDLER_TABLES

#define DISPATCH_PROLOGUE                                  \
    DISPATCH_FIRST                                         \
unused:

#define I(opcode, level, label) &&unused
#define D(opcode, level, label) L(opcode, level, label)
#define X(opcode, level, label) L(opcode, level, label)

#define DEF_HANDLER_TABLES(level)                          \
    DEF_HANDLER_TABLE(level, ENTRY);

#else /* THREADED */

#define INTERPRETER_DEFINITIONS                            \
    /* none */

#define DISPATCH_PROLOGUE                                  \
    while(TRUE) {                                          \
        switch(*pc) {                                      \
            default:

#endif /* THREADED */

#define INTERPRETER_PROLOGUE                               \
    DISPATCH_PROLOGUE                                      \
    jam_printf("Unrecognised opcode %d in: %s.%s\n",       \
               *pc, CLASS_CB(mb->class)->name, mb->name);  \
    exitVM(1);

/* Macros for handler/bytecode rewriting */

#define OPCODE_CHANGED(opcode) (pc[0] != opcode)

#define WITH_OPCODE_CHANGE_CP_DINDEX(opcode, index)        \
    index = DOUBLE_INDEX(pc);                              \
    MBARRIER();                                            \
    if(OPCODE_CHANGED(opcode))                             \
        DISPATCH(0, 0);

#define OPCODE_REWRITE(opcode)                             \
    pc[0] = opcode 

#define OPCODE_REWRITE_OPERAND1(opcode, operand1)          \
{                                                          \
    pc[0] = OPC_LOCK;                                      \
    MBARRIER();                                            \
    pc[1] = operand1;                                      \
    MBARRIER();                                            \
    pc[0] = opcode;                                        \
}

#define OPCODE_REWRITE_OPERAND2(opcode, operand1, operand2)\
{                                                          \
    pc[0] = OPC_LOCK;                                      \
    MBARRIER();                                            \
    pc[1] = operand1;                                      \
    pc[2] = operand2;                                      \
    MBARRIER();                                            \
    pc[0] = opcode;                                        \
}

#ifdef THREADED
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

#define DISPATCH(level, ins_len)                \
{                                               \
    pc += ins_len;                              \
    goto *handlers_##level##_ENTRY[*pc];        \
}

#else /* THREADED */

#define label(x, y, z)                          \
    case x:

#define DISPATCH(level, ins_len)                \
{                                               \
    pc += ins_len;                              \
    break;                                      \
}
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

#endif /* USE_CACHE */


#define DISPATCH_FIRST                          \
    DISPATCH(0, 0)

#define DISPATCH_RET(ins_len)                   \
    DISPATCH(0, ins_len)

#define DISPATCH_METHOD_RET(ins_len)            \
    DISPATCH_RET(ins_len)

#define BRANCH(type, level, TEST)               \
    DISPATCH(0, (TEST) ? READ_S2_OP(pc) : 3)

/* No method preparation is needed on the
   indirect interpreter */
#define PREPARE_MB(mb)

#define ARRAY_TYPE(pc)           READ_U1_OP(pc)
#define SINGLE_INDEX(pc)         READ_U1_OP(pc)
#define DOUBLE_INDEX(pc)         READ_U2_OP(pc)
#define SINGLE_SIGNED(pc)        READ_S1_OP(pc)
#define DOUBLE_SIGNED(pc)        READ_S2_OP(pc)
#define IINC_LVAR_IDX(pc)        SINGLE_INDEX(pc)
#define IINC_DELTA(pc)           SINGLE_SIGNED(pc + 1)
#define INV_QUICK_ARGS(pc)       READ_U1_OP(pc + 1)
#define INV_QUICK_IDX(pc)        READ_U1_OP(pc)
#define INV_INTF_IDX(pc)         DOUBLE_INDEX(pc)
#define INV_INTF_CACHE(pc)       READ_U1_OP(pc + 3)
#define MULTI_ARRAY_DIM(pc)      READ_U1_OP(pc + 2)
#define GETFIELD_THIS_OFFSET(pc) READ_U1_OP(pc + 1)
#define RESOLVED_CONSTANT(pc)    CP_INFO(cp, SINGLE_INDEX(pc))
#define RESOLVED_FIELD(pc)       ((FieldBlock*)CP_INFO(cp, DOUBLE_INDEX(pc)))
#define RESOLVED_METHOD(pc)      ((MethodBlock*)CP_INFO(cp, DOUBLE_INDEX(pc)))
#define RESOLVED_POLYMETHOD(pc)  ((PolyMethodBlock*)CP_INFO(cp, DOUBLE_INDEX(pc)))
#define RESOLVED_CLASS(pc)       ((Class *)CP_INFO(cp, DOUBLE_INDEX(pc)))
#define INTRINSIC_ARGS(pc)       (RESOLVED_METHOD(pc)->args_count)

#define RESOLVED_INVDYNMETHOD(pc)                           \
({                                                          \
    ResolvedInvDynCPEntry *entry = (ResolvedInvDynCPEntry*) \
                             CP_INFO(cp, DOUBLE_INDEX(pc)); \
    InvDynMethodBlock *idmb = entry->cache;                 \
    if(idmb->id != pc[3])                                   \
        idmb = resolvedCallSite(entry, pc[3]);              \
    idmb;                                                   \
})

/* Macros for checking for common exceptions */

#define THROW_EXCEPTION(excep_name, message)                               \
{                                                                          \
    frame->last_pc = pc;                                                   \
    signalException(excep_name, message);                                  \
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

