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

#ifndef THREADED
#error Inlining interpreter cannot be built non-threaded
#endif

#include "interp-threading.h"
#include "interp-direct-common.h"

#define INTERPRETER_DEFINITIONS                                       \
    DEFINE_HANDLER_TABLES                                             \
    DEFINE_BRANCH_TABLES                                              \
    DEFINE_DUMMY_TABLE                                                \
                                                                      \
    static const void **handlers[] = {HNDLR_TBLS(ENTRY),              \
                                      HNDLR_TBLS(START),              \
                                      HNDLR_TBLS(END),                \
                                      HNDLR_TBLS(BRANCH)              \
                                      GUARD_TBLS                      \
                                      DUMMY_TABLE                     \
    };                                                                \
                                                                      \
    void *throwArithmeticExcepLabel = &&throwArithmeticExcep;         \
    void *throwNullLabel = &&throwNull;                               \
    void *throwOOBLabel = &&throwOOB;                                 \
    int oob_array_index = 0;                                          \
                                                                      \
    extern int inlining_inited;                                       \
    if(!inlining_inited) return (uintptr_t*)handlers;

/* First of three massive gcc hacks.  For inlining to work, the
   dispatch sequence contained between the labels "rewrite_lock"
   and "unused" must be relocatable (if it isn't, inlining is
   disabled).  On previous versions of gcc, even using
   no-reorder-blocks, gcc will replace it with a jump.  To
   prevent this, we ensure that the dispatch sequence is the
   first indirect jump (the initial dispatch falls through).
   However, on later versions of gcc, it moves part of the
   interpreter setup to _after_ the "rewrite_lock" label,
   leading to slower dispatch.  To prevent this, we use a
   seperate initial dispatch, but this doesn't work on earlier
   versions of gcc, hence the hack.
*/
#if (__GNUC__ < 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ == 0))
#define GCC_HACK /* fall through */
#else
#ifdef __x86_64__
#define GCC_HACK goto rewrite_lock;
#else
#define GCC_HACK DISPATCH_FIRST
#endif
#endif

/* Second gcc hack.  The more aggressive strict-aliasing
   optimisations in gcc 4.3 breaks the decaching of the
   operand stack cache at the start of floating-point
   opcodes.  With gcc prior to 4.3, an empty asm statement
   was sufficient as a guard to prevent optimisation.  On
   4.3, we need to insert a label, and ensure its address
   is taken (to stop it being optimised out).  However,
   this reduces performance on PowerPC by approx 1 - 2%.

   With gcc 5 and newer an asm statement with a "memory"
   clobber argument explicitly sets a memory barrier for the
   compiler, preventing it from reordering memory accesses
   in a way that breaks decaching.
*/
#if (__GNUC__ > 4)
#define DEF_GUARD_TABLE(level) /* none */
#define GUARD(opcode, level)   __asm__("" ::: "memory");
#define GUARD_TBLS             /* none */
#elif (__GNUC__ == 4) && (__GNUC_MINOR__ >= 3)
#define DEF_GUARD_TABLE(level) DEF_HANDLER_TABLE(level, GUARD)
#define GUARD(opcode, level)   label(opcode, level, GUARD)
#define GUARD_TBLS             , HNDLR_TBLS(GUARD)
#else
#define DEF_GUARD_TABLE(level) /* none */
#define GUARD(opcode, level)   __asm__("");
#define GUARD_TBLS             /* none */
#endif

#define INTERPRETER_PROLOGUE                                          \
    GCC_HACK                                                          \
                                                                      \
rewrite_lock:                                                         \
    DISPATCH_FIRST                                                    \
                                                                      \
unused:                                                               \
    throwOOBLabel = NULL;                                             \
    throwNullLabel = NULL;                                            \
    throwArithmeticExcepLabel = NULL;

#define I(opcode, level, label) L(opcode, level, label)
#define D(opcode, level, label) &&rewrite_lock
#define X(opcode, level, label) &&rewrite_lock

#define DEF_HANDLER_TABLES(level)    \
    DEF_HANDLER_TABLE(level, START); \
    DEF_HANDLER_TABLE(level, ENTRY); \
    DEF_HANDLER_TABLE(level, END);   \
    DEF_GUARD_TABLE(level);

#ifdef USE_CACHE
#define DEFINE_BRANCH_TABLES        \
    DEF_BRANCH_TABLE(0);            \
    DEF_BRANCH_TABLE(1);            \
    DEF_BRANCH_TABLE(2);
#else
#define DEFINE_BRANCH_TABLES        \
    DEF_BRANCH_TABLE(0);
#endif

#define B(level, cond) &&branch_##level##_##cond

#define DEF_BRANCH_TABLE(level)                                         \
    HANDLER_TABLE_T *TBL_NAME(level, BRANCH)[] = {                      \
        B(level,EQ),    B(level,NE),    B(level,LT),    B(level,GE),    \
        B(level,GT),    B(level,LE),    B(level,CMPEQ), B(level,CMPNE), \
        B(level,CMPLT), B(level,CMPGE), B(level,CMPGT), B(level,CMPLE), \
        B(level,CMPEQ), B(level,CMPNE), B(level,GOTO),  B(level,JSR)}

/* Third gcc hack.  On x86_64, higher performance is achieved when "lvars"
   is placed in a register.  However, gcc does not do this, putting it on the
   stack (it also ignores the register modifier).  The dummy handlers are
   used to force gcc to promote lvars, by increasing its apparent importance.
*/
#ifdef __x86_64__
#define DEFINE_DUMMY_TABLE                                                 \
    HANDLER_TABLE_T *dummy_table[] = {                                     \
        &&d1, &&d2, &&d3, &&d4, &&d5, &&d6, &&d7, &&d8, &&d9, &&d10,&&d11, \
        &&d12,&&d13,&&d14,&&d15,&&d16,&&d17,&&d18,&&d19,&&d20,&&d21,&&d22, \
        &&d23,&&d24,&&d25,&&d26,&&d27,&&d28,&&d29,&&d30,&&d31,&&d32,&&d33, \
        &&d34,&&d35,&&d36,&&d37,&&d38,&&d39,&&d40,&&d41,&&d42,&&d43,&&d44, \
        &&d45,&&d46,&&d47,&&d48,&&d49,&&d50,&&d51,&&d52,&&d53,&&d54,&&d55, \
        &&d56,&&d57,&&d58,&&d59,&&d60,&&d61,&&d62,&&d63,&&d64,&&d65,&&d66, \
        &&d67,&&d68,&&d69,&&d70,&&d71,&&d72,&&d73,&&d74,&&d75,&&d76,&&d77, \
        &&d78,&&d79,&&d80,&&d81,&&d82,&&d83,&&d84,&&d85,&&d86,&&d87,&&d88, \
        &&d89,&&d90,&&d91,&&d92,&&d93,&&d94,&&d95,&&d96,&&d97,&&d98,&&d99};

#define DUMMY_TABLE , dummy_table

#define H(X)           \
d##X: {                \
    lvars[X] = X;      \
    goto rewrite_lock; \
}

#define DEF_DUMMY_HANDLERS                                                \
    H(1);  H(2);  H(3);  H(4);  H(5);  H(6);  H(7);  H(8);  H(9);  H(10); \
    H(11); H(12); H(13); H(14); H(15); H(16); H(17); H(18); H(19); H(20); \
    H(21); H(22); H(23); H(24); H(25); H(26); H(27); H(28); H(29); H(31); \
    H(32); H(33); H(34); H(35); H(36); H(37); H(38); H(39); H(30); H(40); \
    H(41); H(42); H(43); H(44); H(45); H(46); H(47); H(48); H(49); H(50); \
    H(51); H(52); H(53); H(54); H(55); H(56); H(57); H(58); H(59); H(60); \
    H(61); H(62); H(63); H(64); H(65); H(66); H(67); H(68); H(69); H(70); \
    H(71); H(72); H(73); H(74); H(75); H(76); H(77); H(78); H(79); H(80); \
    H(81); H(82); H(83); H(84); H(85); H(86); H(87); H(88); H(89); H(90); \
    H(92); H(91); H(93); H(94); H(95); H(96); H(97); H(98); H(99); 
#else
#define DEFINE_DUMMY_TABLE
#define DEF_DUMMY_HANDLERS
#define DUMMY_TABLE
#endif

/* Macros for handler/bytecode rewriting */

#define OPCODE_REWRITE(opcode, cache, new_operand)         \
{                                                          \
    pc->handler = &&rewrite_lock;                          \
    MBARRIER();                                            \
    pc->operand = new_operand;                             \
    MBARRIER();                                            \
    pc->handler = handlers[cache][opcode];                 \
                                                           \
    checkInliningQuickenedInstruction(pc, mb);             \
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

#define DEF_OPC_LBLS(opcode, level, PRE, BODY)  \
    label(opcode, level, START)                 \
        PAD                                     \
    label(opcode, level, ENTRY)                 \
        PRE                                     \
    GUARD(opcode, level)                        \
        BODY                                    \
    label(opcode, level, END)

#define DEF_OPC(opcode, level, BODY)            \
    DEF_OPC_LBLS(opcode, level, /**/, BODY)     \
    goto *pc->handler;

#define DEF_OPC_2(op1, op2, level, BODY)        \
    DEF_OPC(op1, level, BODY)

#define DEF_OPC_3(op1, op2, op3, level, BODY)   \
    DEF_OPC(op1, level, BODY)

#define DEF_OPC_012_2(op1, op2, BODY)           \
    DEF_OPC_012(op1, BODY)

#define DEF_OPC_012_3(op1, op2, op3, BODY)      \
    DEF_OPC_012(op1, BODY)

#define DEF_OPC_012_4(op1, op2, op3, op4, BODY) \
    DEF_OPC_012(op1, BODY)

#define DEF_OPC_210_2(op1, op2, BODY)           \
    DEF_OPC_210(op1, BODY)

#define RW_LABELS(opcode)                       \
    RW_LABEL(opcode, START)                     \
    RW_LABEL(opcode, ENTRY)                     \
    RW_LABEL(opcode, GUARD)                     \
    RW_LABEL(opcode, END) 

#define DEF_OPC_RW(opcode, BODY)                \
    RW_LABELS(opcode)                           \
        BODY                                    \
        goto *pc->handler;

#define DEF_OPC_RW_4(op1, op2, op3, op4, BODY)  \
    RW_LABELS(op1)                              \
    RW_LABELS(op2)                              \
    RW_LABELS(op3)                              \
    RW_LABELS(op4)                              \
        BODY                                    \
        goto *pc->handler;

#ifdef USE_CACHE

#define DEF_OPC_012(opcode, BODY)               \
    DEF_OPC(opcode, 0, ({                       \
        cache.i.v2 = *--ostack;                 \
        cache.i.v1 = *--ostack;                 \
        BODY                                    \
    });)                                        \
                                                \
    DEF_OPC(opcode, 1, ({                       \
        cache.i.v2 = cache.i.v1;                \
        cache.i.v1 = *--ostack;                 \
        BODY                                    \
    });)                                        \
                                                \
    DEF_OPC(opcode, 2, ({BODY});)
        
#define DEF_OPC_210(opcode, BODY)               \
    DEF_OPC(opcode, 2, ({                       \
        *ostack++ = cache.i.v1;                 \
        *ostack++ = cache.i.v2;                 \
        BODY                                    \
    });)                                        \
                                                \
    DEF_OPC(opcode, 1, ({                       \
        *ostack++ = cache.i.v1;                 \
        BODY                                    \
    });)                                        \
                                                \
    DEF_OPC(opcode, 0, ({BODY});)
        
#define DEF_OPC_GRD(opcode, level, PRE, BODY)   \
    DEF_OPC_LBLS(opcode, level, PRE, BODY)      \
    goto *pc->handler;

#define DEF_OPC_FLOAT(opcode, BODY)             \
    DEF_OPC_GRD(opcode, 2, ({                   \
        *ostack++ = cache.i.v1;                 \
        *ostack++ = cache.i.v2;                 \
    });, ({                                     \
        BODY                                    \
    });)                                        \
                                                \
    DEF_OPC_GRD(opcode, 1, ({                   \
        *ostack++ = cache.i.v1;                 \
    });, ({                                     \
        BODY                                    \
    });)                                        \
                                                \
    DEF_OPC(opcode, 0, ({BODY});)
        
#define DEF_OPC_JMP(TYPE, BODY)                 \
    DEF_OPC_LBLS(OPC_##TYPE, 2, /**/, ({        \
        *ostack++ = cache.i.v1;                 \
        *ostack++ = cache.i.v2;                 \
        BODY                                    \
        BRANCH(TYPE, 2, TRUE);                  \
    });)                                        \
                                                \
    DEF_OPC_LBLS(OPC_##TYPE, 1, /**/, ({        \
        *ostack++ = cache.i.v1;                 \
        BODY                                    \
        BRANCH(TYPE, 1, TRUE);                  \
    });)                                        \
                                                \
    DEF_OPC_LBLS(OPC_##TYPE, 0, /**/, ({        \
        BODY                                    \
        BRANCH(TYPE, 0, TRUE);                  \
    });)

#define RW_LABEL(opcode, lbl)                   \
    label(opcode, 0, lbl)                       \
    label(opcode, 1, lbl)                       \
    label(opcode, 2, lbl)

#else /* USE_CACHE */

#define DEF_OPC_012(opcode, BODY)               \
    DEF_OPC(opcode, 0, BODY)

#define DEF_OPC_210(opcode, BODY)               \
    DEF_OPC(opcode, 0, BODY)

#define DEF_OPC_FLOAT(opcode, BODY)             \
    DEF_OPC(opcode, 0, BODY)

#define DEF_OPC_JMP(TYPE, BODY)                 \
    DEF_OPC_LBLS(OPC_##TYPE, 0, /**/, ({        \
        BODY                                    \
        BRANCH(TYPE, 0, TRUE);                  \
    });)

#define RW_LABEL(opcode, lbl)                   \
    label(opcode, 0, lbl)

#endif /* USE_CACHE */

#define DISPATCH_FIRST                          \
    goto *pc->handler;

#define DISPATCH_SWITCH

#define REDISPATCH ;

#define DISPATCH_RET(ins_len)                   \
    pc++;

#define DISPATCH_METHOD_RET(ins_len)            \
    goto *(++pc)->handler;

#define DISPATCH(level, ins_len)                \
    pc++;

#define BRANCH(type, level, TEST)               \
    if(TEST) {                                  \
        pc = (Instruction*) pc->operand.pntr;   \
branch_##level##_##type:                        \
        goto *pc->handler;                      \
    } else                                      \
        pc++;

/* Macros for checking for common exceptions */

#define THROW_EXCEPTION(excep_enum, message)   \
{                                              \
    frame->last_pc = pc;                       \
    signalException(excep_enum, message);      \
    goto throwException;                       \
}

#define NULL_POINTER_CHECK(ref)                \
    if(!ref) {                                 \
        __asm__("");                           \
        goto *throwNullLabel;                  \
    }

#define ZERO_DIVISOR_CHECK(value)              \
    if(value == 0) {                           \
        __asm__("");                           \
        goto *throwArithmeticExcepLabel;       \
    }

#define ARRAY_BOUNDS_CHECK(array, idx)         \
    if(idx >= ARRAY_LEN(array)) {              \
        __asm__("");                           \
        oob_array_index = idx;                 \
        goto *throwOOBLabel;                   \
    }

#define MAX_INT_DIGITS 11

#ifndef PAD
#define PAD __asm__("");
#endif

extern void initialiseDirect(InitArgs *args);
extern void inlineBlockWrappedOpcode(MethodBlock *mb, Instruction *pc);
extern void prepare(MethodBlock *mb, const void ***handlers);
extern void checkInliningQuickenedInstruction(Instruction *pc, MethodBlock *mb);
extern void *inlineProfiledBlock(Instruction *pc, MethodBlock *mb,
                                 int force_inlining);

