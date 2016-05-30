/* src/vm/jit/ir/icmd.hpp - Intermediate Commands

   Copyright (C) 2008-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#ifndef ICMD_HPP_
#define ICMD_HPP_ 1

#include "config.h"

#include <stdint.h>

#include "vm/jit/ir/bytecode.hpp"


// JavaVM operation codes (sorted).
enum ICMD {
	ICMD_NOP               = BC_nop,

	ICMD_ACONST            = BC_aconst_null,

	ICMD_CHECKNULL         = 2,

	ICMD_ICONST            = BC_iconst_0,

	/* 3 */
	/* 4 */

	ICMD_IDIVPOW2          = 5,
	ICMD_LDIVPOW2          = 6,

	/* 7 */
	/* 8 */

	ICMD_LCONST            = BC_lconst_0,

	ICMD_LCMPCONST         = 10,

	ICMD_FCONST            = BC_fconst_0,

	/* 12 */
	/* 13 */

	ICMD_DCONST            = BC_dconst_0,

	ICMD_COPY              = 15,
	ICMD_MOVE              = 16,

	/* 17 */
	/* 18 */
	/* 19 */
	/* 20 */

	/* Order of LOAD instructions must be equal to order of TYPE_*
	   defines. */

	ICMD_ILOAD            = BC_iload,
	ICMD_LLOAD            = BC_lload,
	ICMD_FLOAD            = BC_fload,
	ICMD_DLOAD            = BC_dload,
	ICMD_ALOAD            = BC_aload,

	ICMD_IADDCONST        = 26,
	ICMD_ISUBCONST        = 27,
	ICMD_IMULCONST        = 28,
	ICMD_IANDCONST        = 29,
	ICMD_IORCONST         = 30,
	ICMD_IXORCONST        = 31,

	ICMD_ISHLCONST        = 32,
	ICMD_ISHRCONST        = 33,
	ICMD_IUSHRCONST       = 34,

	ICMD_IREMPOW2         = 35,

	ICMD_LADDCONST        = 36,
	ICMD_LSUBCONST        = 37,
	ICMD_LMULCONST        = 38,
	ICMD_LANDCONST        = 39,
	ICMD_LORCONST         = 40,
	ICMD_LXORCONST        = 41,

	ICMD_LSHLCONST        = 42,
	ICMD_LSHRCONST        = 43,
	ICMD_LUSHRCONST       = 44,

	ICMD_LREMPOW2         = 45,

	ICMD_IALOAD           = BC_iaload,
	ICMD_LALOAD           = BC_laload,
	ICMD_FALOAD           = BC_faload,
	ICMD_DALOAD           = BC_daload,
	ICMD_AALOAD           = BC_aaload,
	ICMD_BALOAD           = BC_baload,
	ICMD_CALOAD           = BC_caload,
	ICMD_SALOAD           = BC_saload,

	/* Order of STORE instructions must be equal to order of TYPE_*
	   defines. */

	ICMD_ISTORE           = BC_istore,
	ICMD_LSTORE           = BC_lstore,
	ICMD_FSTORE           = BC_fstore,
	ICMD_DSTORE           = BC_dstore,
	ICMD_ASTORE           = BC_astore,

	ICMD_IF_LEQ           = 59,
	ICMD_IF_LNE           = 60,
	ICMD_IF_LLT           = 61,
	ICMD_IF_LGE           = 62,
	ICMD_IF_LGT           = 63,
	ICMD_IF_LLE           = 64,

	ICMD_IF_LCMPEQ        = 65,
	ICMD_IF_LCMPNE        = 66,
	ICMD_IF_LCMPLT        = 67,
	ICMD_IF_LCMPGE        = 68,
	ICMD_IF_LCMPGT        = 69,
	ICMD_IF_LCMPLE        = 70,

	/* 71 */
	/* 72 */
	/* 73 */
	/* 74 */
	/* 75 */
	/* 76 */
	/* 77 */
	/* 78 */

	ICMD_IASTORE          = BC_iastore,
	ICMD_LASTORE          = BC_lastore,
	ICMD_FASTORE          = BC_fastore,
	ICMD_DASTORE          = BC_dastore,
	ICMD_AASTORE          = BC_aastore,
	ICMD_BASTORE          = BC_bastore,
	ICMD_CASTORE          = BC_castore,
	ICMD_SASTORE          = BC_sastore,

	ICMD_POP              = BC_pop,
	ICMD_POP2             = BC_pop2,
	ICMD_DUP              = BC_dup,
	ICMD_DUP_X1           = BC_dup_x1,
	ICMD_DUP_X2           = BC_dup_x2,
	ICMD_DUP2             = BC_dup2,
	ICMD_DUP2_X1          = BC_dup2_x1,
	ICMD_DUP2_X2          = BC_dup2_x2,
	ICMD_SWAP             = BC_swap,

	ICMD_IADD             = BC_iadd,
	ICMD_LADD             = BC_ladd,
	ICMD_FADD             = BC_fadd,
	ICMD_DADD             = BC_dadd,

	ICMD_ISUB             = BC_isub,
	ICMD_LSUB             = BC_lsub,
	ICMD_FSUB             = BC_fsub,
	ICMD_DSUB             = BC_dsub,

	ICMD_IMUL             = BC_imul,
	ICMD_LMUL             = BC_lmul,
	ICMD_FMUL             = BC_fmul,
	ICMD_DMUL             = BC_dmul,

	ICMD_IDIV             = BC_idiv,
	ICMD_LDIV             = BC_ldiv,
	ICMD_FDIV             = BC_fdiv,
	ICMD_DDIV             = BC_ddiv,

	ICMD_IREM             = BC_irem,
	ICMD_LREM             = BC_lrem,
	ICMD_FREM             = BC_frem,
	ICMD_DREM             = BC_drem,

	ICMD_INEG             = BC_ineg,
	ICMD_LNEG             = BC_lneg,
	ICMD_FNEG             = BC_fneg,
	ICMD_DNEG             = BC_dneg,

	ICMD_ISHL             = BC_ishl,
	ICMD_LSHL             = BC_lshl,
	ICMD_ISHR             = BC_ishr,
	ICMD_LSHR             = BC_lshr,
	ICMD_IUSHR            = BC_iushr,
	ICMD_LUSHR            = BC_lushr,

	ICMD_IAND             = BC_iand,
	ICMD_LAND             = BC_land,
	ICMD_IOR              = BC_ior,
	ICMD_LOR              = BC_lor,
	ICMD_IXOR             = BC_ixor,
	ICMD_LXOR             = BC_lxor,

	ICMD_IINC             = BC_iinc,

	ICMD_I2L              = BC_i2l,
	ICMD_I2F              = BC_i2f,
	ICMD_I2D              = BC_i2d,
	ICMD_L2I              = BC_l2i,
	ICMD_L2F              = BC_l2f,
	ICMD_L2D              = BC_l2d,
	ICMD_F2I              = BC_f2i,
	ICMD_F2L              = BC_f2l,
	ICMD_F2D              = BC_f2d,
	ICMD_D2I              = BC_d2i,
	ICMD_D2L              = BC_d2l,
	ICMD_D2F              = BC_d2f,

	ICMD_INT2BYTE         = BC_int2byte,
	ICMD_INT2CHAR         = BC_int2char,
	ICMD_INT2SHORT        = BC_int2short,

	ICMD_LCMP             = BC_lcmp,
	ICMD_FCMPL            = BC_fcmpl,
	ICMD_FCMPG            = BC_fcmpg,
	ICMD_DCMPL            = BC_dcmpl,
	ICMD_DCMPG            = BC_dcmpg,

	ICMD_IFEQ             = BC_ifeq,
	ICMD_IFNE             = BC_ifne,
	ICMD_IFLT             = BC_iflt,
	ICMD_IFGE             = BC_ifge,
	ICMD_IFGT             = BC_ifgt,
	ICMD_IFLE             = BC_ifle,

	ICMD_IF_ICMPEQ        = BC_if_icmpeq,
	ICMD_IF_ICMPNE        = BC_if_icmpne,
	ICMD_IF_ICMPLT        = BC_if_icmplt,
	ICMD_IF_ICMPGE        = BC_if_icmpge,
	ICMD_IF_ICMPGT        = BC_if_icmpgt,
	ICMD_IF_ICMPLE        = BC_if_icmple,
	ICMD_IF_ACMPEQ        = BC_if_acmpeq,
	ICMD_IF_ACMPNE        = BC_if_acmpne,

	ICMD_GOTO             = BC_goto,
	ICMD_JSR              = BC_jsr,
	ICMD_RET              = BC_ret,

	ICMD_TABLESWITCH      = BC_tableswitch,
	ICMD_LOOKUPSWITCH     = BC_lookupswitch,

	ICMD_IRETURN          = BC_ireturn,
	ICMD_LRETURN          = BC_lreturn,
	ICMD_FRETURN          = BC_freturn,
	ICMD_DRETURN          = BC_dreturn,
	ICMD_ARETURN          = BC_areturn,
	ICMD_RETURN           = BC_return,

	ICMD_GETSTATIC        = BC_getstatic,
	ICMD_PUTSTATIC        = BC_putstatic,
	ICMD_GETFIELD         = BC_getfield,
	ICMD_PUTFIELD         = BC_putfield,

	ICMD_INVOKEVIRTUAL    = BC_invokevirtual,
	ICMD_INVOKESPECIAL    = BC_invokespecial,
	ICMD_INVOKESTATIC     = BC_invokestatic,
	ICMD_INVOKEINTERFACE  = BC_invokeinterface,

	/* 186 */

	ICMD_NEW              = BC_new,
	ICMD_NEWARRAY         = BC_newarray,
	ICMD_ANEWARRAY        = BC_anewarray,

	ICMD_ARRAYLENGTH      = BC_arraylength,

	ICMD_ATHROW           = BC_athrow,

	ICMD_CHECKCAST        = BC_checkcast,
	ICMD_INSTANCEOF       = BC_instanceof,

	ICMD_MONITORENTER     = BC_monitorenter,
	ICMD_MONITOREXIT      = BC_monitorexit,

	/* 196 */

	ICMD_MULTIANEWARRAY   = BC_multianewarray,

	ICMD_IFNULL           = BC_ifnull,
	ICMD_IFNONNULL        = BC_ifnonnull,

	/* 200 */
	/* 201 */

	ICMD_BREAKPOINT       = BC_breakpoint,

	ICMD_IASTORECONST     = 204,
	ICMD_LASTORECONST     = 205,
	ICMD_FASTORECONST     = 206,
	ICMD_DASTORECONST     = 207,
	ICMD_AASTORECONST     = 208,
	ICMD_BASTORECONST     = 209,
	ICMD_CASTORECONST     = 210,
	ICMD_SASTORECONST     = 211,

	ICMD_PUTSTATICCONST   = 212,
	ICMD_PUTFIELDCONST    = 213,

	ICMD_IMULPOW2         = 214,
	ICMD_LMULPOW2         = 215,

	ICMD_GETEXCEPTION     = 249,
	ICMD_PHI              = 250,

	ICMD_INLINE_START     = 251,        /* instruction before inlined method  */
	ICMD_INLINE_END       = 252,        /* instruction after inlined method   */
	ICMD_INLINE_BODY      = 253,        /* start of inlined body              */

	ICMD_BUILTIN          = 255         /* internal opcode                    */
};


/* data-flow constants for the ICMD table ************************************/

#define DF_0_TO_0      0
#define DF_1_TO_0      1
#define DF_2_TO_0      2
#define DF_3_TO_0      3

#define DF_DST_BASE    4      /* from this value on, iptr->dst is a variable */

#define DF_0_TO_1      (DF_DST_BASE + 0)
#define DF_1_TO_1      (DF_DST_BASE + 1)
#define DF_2_TO_1      (DF_DST_BASE + 2)
#define DF_3_TO_1      (DF_DST_BASE + 3)
#define DF_N_TO_1      (DF_DST_BASE + 4)

#define DF_INVOKE      (DF_DST_BASE + 5)
#define DF_BUILTIN     (DF_DST_BASE + 6)

#define DF_COPY        (DF_DST_BASE + 7)
#define DF_MOVE        (DF_DST_BASE + 8)

#define DF_DUP         -1
#define DF_DUP_X1      -1
#define DF_DUP_X2      -1
#define DF_DUP2        -1
#define DF_DUP2_X1     -1
#define DF_DUP2_X2     -1
#define DF_SWAP        -1

/* special data-flow recognized by verify/generate.pl: */
#define DF_LOAD        DF_COPY
#define DF_STORE       DF_MOVE
#define DF_IINC        DF_1_TO_1
#define DF_POP         DF_1_TO_0
#define DF_POP2        DF_2_TO_0


// Control-flow constants for the ICMD table.
#define CF_NORMAL      0
#define CF_IF          1

#define CF_END_BASE    2  /* from here on, they mark the end of a superblock */

#define CF_END         (CF_END_BASE + 0)
#define CF_GOTO        (CF_END_BASE + 1)
#define CF_TABLE       (CF_END_BASE + 2)
#define CF_LOOKUP      (CF_END_BASE + 3)
#define CF_JSR         (CF_END_BASE + 4)
#define CF_RET         (CF_END_BASE + 5)


// Flag constants for the ICMD table.
#define ICMDTABLE_PEI    0x0001               /* ICMD may throw an exception */
#define ICMDTABLE_CALLS  0x0002     /* needs registers to be saved, may call */


// ICMD table entry.
typedef struct icmdtable_entry_t icmdtable_entry_t;

struct icmdtable_entry_t {
#if !defined(NDEBUG)
	const char* name;                           /* name, without ICMD_ prefix */
#endif
	int32_t dataflow;                            /* a DF_ constant, see above */
	int32_t controlflow;                         /* a CF_ constant, see above */
	int32_t flags;                       /* a combination of ICMDTABLE_ flags */
};


// The ICMD table.
extern icmdtable_entry_t icmd_table[256];

#endif // ICMD_HPP_


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
