/*
 * Copyright (C) 2008, 2009, 2013 Robert Lougher <rob@jamvm.org.uk>.
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

/* Two levels of macros are needed to correctly produce the label
   from the OPC_xxx macro passed in opcode as cpp doesn't 
   prescan when concatenating with ##... */
#define HNDLR_LBL(opcode, level, label) &&opc##opcode##_##level##_##label
#define L(opcode, level, label) HNDLR_LBL(opcode, level, label)

#define TBL_NAME(level, label) handlers_##level##_##label

#ifdef USE_CACHE
#define HNDLR_TBLS(label) TBL_NAME(0,label), TBL_NAME(1,label), \
                          TBL_NAME(2,label)
#else
#define HNDLR_TBLS(label) TBL_NAME(0,label)
#endif

#ifdef USE_CACHE
#define DEFINE_HANDLER_TABLES \
    DEF_HANDLER_TABLES(0);    \
    DEF_HANDLER_TABLES(1);    \
    DEF_HANDLER_TABLES(2);
#else
#define DEFINE_HANDLER_TABLES \
    DEF_HANDLER_TABLES(0);
#endif

#ifdef JSR292
#define J(opcode, level, label) L(opcode, level, label)
#else
#define J(opcode, level, label) &&unused
#endif

#define DEF_HANDLER_TABLE(level,label)               \
    HANDLER_TABLE_T *TBL_NAME(level,label)[] = {     \
        L(OPC_NOP,                    level, label), \
        X(OPC_ACONST_NULL,            level, label), \
        L(OPC_ICONST_M1,              level, label), \
        L(OPC_ICONST_0,               level, label), \
        L(OPC_ICONST_1,               level, label), \
        L(OPC_ICONST_2,               level, label), \
        L(OPC_ICONST_3,               level, label), \
        L(OPC_ICONST_4,               level, label), \
        L(OPC_ICONST_5,               level, label), \
        L(OPC_LCONST_0,               level, label), \
        L(OPC_LCONST_1,               level, label), \
        X(OPC_FCONST_0,               level, label), \
        L(OPC_FCONST_1,               level, label), \
        L(OPC_FCONST_2,               level, label), \
        X(OPC_DCONST_0,               level, label), \
        L(OPC_DCONST_1,               level, label), \
        L(OPC_BIPUSH,                 level, label), \
        L(OPC_SIPUSH,                 level, label), \
        L(OPC_LDC,                    level, label), \
        D(OPC_LDC_W,                  level, label), \
        L(OPC_LDC2_W,                 level, label), \
        L(OPC_ILOAD,                  level, label), \
        L(OPC_LLOAD,                  level, label), \
        X(OPC_FLOAD,                  level, label), \
        X(OPC_DLOAD,                  level, label), \
        X(OPC_ALOAD,                  level, label), \
        L(OPC_ILOAD_0,                level, label), \
        L(OPC_ILOAD_1,                level, label), \
        L(OPC_ILOAD_2,                level, label), \
        L(OPC_ILOAD_3,                level, label), \
        L(OPC_LLOAD_0,                level, label), \
        L(OPC_LLOAD_1,                level, label), \
        L(OPC_LLOAD_2,                level, label), \
        L(OPC_LLOAD_3,                level, label), \
        X(OPC_FLOAD_0,                level, label), \
        X(OPC_FLOAD_1,                level, label), \
        X(OPC_FLOAD_2,                level, label), \
        X(OPC_FLOAD_3,                level, label), \
        X(OPC_DLOAD_0,                level, label), \
        X(OPC_DLOAD_1,                level, label), \
        X(OPC_DLOAD_2,                level, label), \
        X(OPC_DLOAD_3,                level, label), \
        D(OPC_ALOAD_0,                level, label), \
        X(OPC_ALOAD_1,                level, label), \
        X(OPC_ALOAD_2,                level, label), \
        X(OPC_ALOAD_3,                level, label), \
        L(OPC_IALOAD,                 level, label), \
        L(OPC_LALOAD,                 level, label), \
        X(OPC_FALOAD,                 level, label), \
        X(OPC_DALOAD,                 level, label), \
        L(OPC_AALOAD,                 level, label), \
        L(OPC_BALOAD,                 level, label), \
        L(OPC_CALOAD,                 level, label), \
        L(OPC_SALOAD,                 level, label), \
        L(OPC_ISTORE,                 level, label), \
        L(OPC_LSTORE,                 level, label), \
        X(OPC_FSTORE,                 level, label), \
        X(OPC_DSTORE,                 level, label), \
        X(OPC_ASTORE,                 level, label), \
        L(OPC_ISTORE_0,               level, label), \
        L(OPC_ISTORE_1,               level, label), \
        L(OPC_ISTORE_2,               level, label), \
        L(OPC_ISTORE_3,               level, label), \
        L(OPC_LSTORE_0,               level, label), \
        L(OPC_LSTORE_1,               level, label), \
        L(OPC_LSTORE_2,               level, label), \
        L(OPC_LSTORE_3,               level, label), \
        X(OPC_FSTORE_0,               level, label), \
        X(OPC_FSTORE_1,               level, label), \
        X(OPC_FSTORE_2,               level, label), \
        X(OPC_FSTORE_3,               level, label), \
        X(OPC_DSTORE_0,               level, label), \
        X(OPC_DSTORE_1,               level, label), \
        X(OPC_DSTORE_2,               level, label), \
        X(OPC_DSTORE_3,               level, label), \
        X(OPC_ASTORE_0,               level, label), \
        X(OPC_ASTORE_1,               level, label), \
        X(OPC_ASTORE_2,               level, label), \
        X(OPC_ASTORE_3,               level, label), \
        L(OPC_IASTORE,                level, label), \
        L(OPC_LASTORE,                level, label), \
        X(OPC_FASTORE,                level, label), \
        X(OPC_DASTORE,                level, label), \
        L(OPC_AASTORE,                level, label), \
        L(OPC_BASTORE,                level, label), \
        L(OPC_CASTORE,                level, label), \
        X(OPC_SASTORE,                level, label), \
        L(OPC_POP,                    level, label), \
        L(OPC_POP2,                   level, label), \
        L(OPC_DUP,                    level, label), \
        L(OPC_DUP_X1,                 level, label), \
        L(OPC_DUP_X2,                 level, label), \
        L(OPC_DUP2,                   level, label), \
        L(OPC_DUP2_X1,                level, label), \
        L(OPC_DUP2_X2,                level, label), \
        L(OPC_SWAP,                   level, label), \
        L(OPC_IADD,                   level, label), \
        L(OPC_LADD,                   level, label), \
        L(OPC_FADD,                   level, label), \
        L(OPC_DADD,                   level, label), \
        L(OPC_ISUB,                   level, label), \
        L(OPC_LSUB,                   level, label), \
        L(OPC_FSUB,                   level, label), \
        L(OPC_DSUB,                   level, label), \
        L(OPC_IMUL,                   level, label), \
        L(OPC_LMUL,                   level, label), \
        L(OPC_FMUL,                   level, label), \
        L(OPC_DMUL,                   level, label), \
        L(OPC_IDIV,                   level, label), \
        L(OPC_LDIV,                   level, label), \
        L(OPC_FDIV,                   level, label), \
        L(OPC_DDIV,                   level, label), \
        L(OPC_IREM,                   level, label), \
        L(OPC_LREM,                   level, label), \
        L(OPC_FREM,                   level, label), \
        L(OPC_DREM,                   level, label), \
        L(OPC_INEG,                   level, label), \
        L(OPC_LNEG,                   level, label), \
        L(OPC_FNEG,                   level, label), \
        L(OPC_DNEG,                   level, label), \
        L(OPC_ISHL,                   level, label), \
        L(OPC_LSHL,                   level, label), \
        L(OPC_ISHR,                   level, label), \
        L(OPC_LSHR,                   level, label), \
        L(OPC_IUSHR,                  level, label), \
        L(OPC_LUSHR,                  level, label), \
        L(OPC_IAND,                   level, label), \
        L(OPC_LAND,                   level, label), \
        L(OPC_IOR,                    level, label), \
        L(OPC_LOR,                    level, label), \
        L(OPC_IXOR,                   level, label), \
        L(OPC_LXOR,                   level, label), \
        L(OPC_IINC,                   level, label), \
        L(OPC_I2L,                    level, label), \
        L(OPC_I2F,                    level, label), \
        L(OPC_I2D,                    level, label), \
        L(OPC_L2I,                    level, label), \
        L(OPC_L2F,                    level, label), \
        L(OPC_L2D,                    level, label), \
        L(OPC_F2I,                    level, label), \
        L(OPC_F2L,                    level, label), \
        L(OPC_F2D,                    level, label), \
        L(OPC_D2I,                    level, label), \
        L(OPC_D2L,                    level, label), \
        L(OPC_D2F,                    level, label), \
        L(OPC_I2B,                    level, label), \
        L(OPC_I2C,                    level, label), \
        L(OPC_I2S,                    level, label), \
        L(OPC_LCMP,                   level, label), \
        L(OPC_FCMPL,                  level, label), \
        L(OPC_FCMPG,                  level, label), \
        L(OPC_DCMPL,                  level, label), \
        L(OPC_DCMPG,                  level, label), \
        L(OPC_IFEQ,                   level, label), \
        L(OPC_IFNE,                   level, label), \
        L(OPC_IFLT,                   level, label), \
        L(OPC_IFGE,                   level, label), \
        L(OPC_IFGT,                   level, label), \
        L(OPC_IFLE,                   level, label), \
        L(OPC_IF_ICMPEQ,              level, label), \
        L(OPC_IF_ICMPNE,              level, label), \
        L(OPC_IF_ICMPLT,              level, label), \
        L(OPC_IF_ICMPGE,              level, label), \
        L(OPC_IF_ICMPGT,              level, label), \
        L(OPC_IF_ICMPLE,              level, label), \
        X(OPC_IF_ACMPEQ,              level, label), \
        X(OPC_IF_ACMPNE,              level, label), \
        L(OPC_GOTO,                   level, label), \
        L(OPC_JSR,                    level, label), \
        L(OPC_RET,                    level, label), \
        L(OPC_TABLESWITCH,            level, label), \
        L(OPC_LOOKUPSWITCH,           level, label), \
        L(OPC_IRETURN,                level, label), \
        L(OPC_LRETURN,                level, label), \
        X(OPC_FRETURN,                level, label), \
        X(OPC_DRETURN,                level, label), \
        X(OPC_ARETURN,                level, label), \
        L(OPC_RETURN,                 level, label), \
        L(OPC_GETSTATIC,              level, label), \
        L(OPC_PUTSTATIC,              level, label), \
        L(OPC_GETFIELD,               level, label), \
        L(OPC_PUTFIELD,               level, label), \
        L(OPC_INVOKEVIRTUAL,          level, label), \
        L(OPC_INVOKESPECIAL,          level, label), \
        L(OPC_INVOKESTATIC,           level, label), \
        L(OPC_INVOKEINTERFACE,        level, label), \
        J(OPC_INVOKEDYNAMIC,          level, label), \
        L(OPC_NEW,                    level, label), \
        L(OPC_NEWARRAY,               level, label), \
        L(OPC_ANEWARRAY,              level, label), \
        L(OPC_ARRAYLENGTH,            level, label), \
        L(OPC_ATHROW,                 level, label), \
        L(OPC_CHECKCAST,              level, label), \
        L(OPC_INSTANCEOF,             level, label), \
        L(OPC_MONITORENTER,           level, label), \
        L(OPC_MONITOREXIT,            level, label), \
        D(OPC_WIDE,                   level, label), \
        L(OPC_MULTIANEWARRAY,         level, label), \
        X(OPC_IFNULL,                 level, label), \
        X(OPC_IFNONNULL,              level, label), \
        D(OPC_GOTO_W,                 level, label), \
        D(OPC_JSR_W,                  level, label), \
        &&unused,                                    \
        L(OPC_LDC_QUICK,              level, label), \
        L(OPC_LDC_W_QUICK,            level, label), \
        &&unused,                                    \
        L(OPC_GETFIELD_QUICK,         level, label), \
        L(OPC_PUTFIELD_QUICK,         level, label), \
        L(OPC_GETFIELD2_QUICK,        level, label), \
        L(OPC_PUTFIELD2_QUICK,        level, label), \
        L(OPC_GETSTATIC_QUICK,        level, label), \
        L(OPC_PUTSTATIC_QUICK,        level, label), \
        L(OPC_GETSTATIC2_QUICK,       level, label), \
        L(OPC_PUTSTATIC2_QUICK,       level, label), \
        L(OPC_INVOKEVIRTUAL_QUICK,    level, label), \
        L(OPC_INVOKENONVIRTUAL_QUICK, level, label), \
        L(OPC_INVOKESUPER_QUICK,      level, label), \
        L(OPC_GETFIELD_QUICK_REF,     level, label), \
        L(OPC_PUTFIELD_QUICK_REF,     level, label), \
        L(OPC_GETSTATIC_QUICK_REF,    level, label), \
        L(OPC_PUTSTATIC_QUICK_REF,    level, label), \
        L(OPC_GETFIELD_THIS_REF,      level, label), \
        L(OPC_MIRANDA_BRIDGE,         level, label), \
        L(OPC_ABSTRACT_METHOD_ERROR,  level, label), \
        I(OPC_INLINE_REWRITER,        level, label), \
        I(OPC_PROFILE_REWRITER,       level, label), \
        D(OPC_INVOKEVIRTUAL_QUICK_W,  level, label), \
        D(OPC_GETFIELD_QUICK_W,       level, label), \
        D(OPC_PUTFIELD_QUICK_W,       level, label), \
        L(OPC_GETFIELD_THIS,          level, label), \
        D(OPC_LOCK,                   level, label), \
        L(OPC_ALOAD_THIS,             level, label), \
        L(OPC_INVOKESTATIC_QUICK,     level, label), \
        L(OPC_NEW_QUICK,              level, label), \
        &&unused,                                    \
        L(OPC_ANEWARRAY_QUICK,        level, label), \
        &&unused,                                    \
        &&unused,                                    \
        L(OPC_CHECKCAST_QUICK,        level, label), \
        L(OPC_INSTANCEOF_QUICK,       level, label), \
        &&unused,                                    \
        &&unused,                                    \
        &&unused,                                    \
        L(OPC_MULTIANEWARRAY_QUICK,   level, label), \
        J(OPC_INVOKEHANDLE,           level, label), \
        J(OPC_INVOKEBASIC,            level, label), \
        J(OPC_LINKTOSPECIAL,          level, label), \
        J(OPC_LINKTOVIRTUAL,          level, label), \
        J(OPC_LINKTOINTERFACE,        level, label), \
        L(OPC_INVOKEINTERFACE_QUICK,  level, label), \
        J(OPC_INVOKEDYNAMIC_QUICK,    level, label), \
        &&unused,                                    \
        &&unused,                                    \
        &&unused,                                    \
        &&unused,                                    \
        &&unused};

