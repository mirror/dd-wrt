/* src/vm/jit/ir/bytecode.hpp - Java byte code definitions

   Copyright (C) 2007
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


#ifndef BYTECODE_HPP_
#define BYTECODE_HPP_ 1

#include "config.h"


/* bytecode_t *****************************************************************/

typedef struct bytecode_t bytecode_t;

struct bytecode_t {
	int         length;                 /* length of the instruction in bytes */
	int         slots;                  /* required stack slots               */
	const char *mnemonic;
};

extern bytecode_t bytecode[256];


/* Java bytecodes *************************************************************/

enum ByteCode {
	BC_nop             = 0,

	BC_aconst_null     = 1,

	BC_iconst_m1       = 2,
	BC_iconst_0        = 3,
	BC_iconst_1        = 4,
	BC_iconst_2        = 5,
	BC_iconst_3        = 6,
	BC_iconst_4        = 7,
	BC_iconst_5        = 8,

	BC_lconst_0        = 9,
	BC_lconst_1        = 10,

	BC_fconst_0        = 11,
	BC_fconst_1        = 12,
	BC_fconst_2        = 13,

	BC_dconst_0        = 14,
	BC_dconst_1        = 15,

	BC_bipush          = 16,
	BC_sipush          = 17,

	BC_ldc1            = 18,
	BC_ldc2            = 19,
	BC_ldc2w           = 20,

	BC_iload           = 21,
	BC_lload           = 22,
	BC_fload           = 23,
	BC_dload           = 24,
	BC_aload           = 25,

	BC_iload_0         = 26,
	BC_iload_1         = 27,
	BC_iload_2         = 28,
	BC_iload_3         = 29,

	BC_lload_0         = 30,
	BC_lload_1         = 31,
	BC_lload_2         = 32,
	BC_lload_3         = 33,

	BC_fload_0         = 34,
	BC_fload_1         = 35,
	BC_fload_2         = 36,
	BC_fload_3         = 37,

	BC_dload_0         = 38,
	BC_dload_1         = 39,
	BC_dload_2         = 40,
	BC_dload_3         = 41,

	BC_aload_0         = 42,
	BC_aload_1         = 43,
	BC_aload_2         = 44,
	BC_aload_3         = 45,

	BC_iaload          = 46,
	BC_laload          = 47,
	BC_faload          = 48,
	BC_daload          = 49,
	BC_aaload          = 50,
	BC_baload          = 51,
	BC_caload          = 52,
	BC_saload          = 53,

	BC_istore          = 54,
	BC_lstore          = 55,
	BC_fstore          = 56,
	BC_dstore          = 57,
	BC_astore          = 58,

	BC_istore_0        = 59,
	BC_istore_1        = 60,
	BC_istore_2        = 61,
	BC_istore_3        = 62,

	BC_lstore_0        = 63,
	BC_lstore_1        = 64,
	BC_lstore_2        = 65,
	BC_lstore_3        = 66,

	BC_fstore_0        = 67,
	BC_fstore_1        = 68,
	BC_fstore_2        = 69,
	BC_fstore_3        = 70,

	BC_dstore_0        = 71,
	BC_dstore_1        = 72,
	BC_dstore_2        = 73,
	BC_dstore_3        = 74,

	BC_astore_0        = 75,
	BC_astore_1        = 76,
	BC_astore_2        = 77,
	BC_astore_3        = 78,

	BC_iastore         = 79,
	BC_lastore         = 80,
	BC_fastore         = 81,
	BC_dastore         = 82,
	BC_aastore         = 83,
	BC_bastore         = 84,
	BC_castore         = 85,
	BC_sastore         = 86,

	BC_pop             = 87,
	BC_pop2            = 88,
	BC_dup             = 89,
	BC_dup_x1          = 90,
	BC_dup_x2          = 91,
	BC_dup2            = 92,
	BC_dup2_x1         = 93,
	BC_dup2_x2         = 94,
	BC_swap            = 95,

	BC_iadd            = 96,
	BC_ladd            = 97,
	BC_fadd            = 98,
	BC_dadd            = 99,

	BC_isub            = 100,
	BC_lsub            = 101,
	BC_fsub            = 102,
	BC_dsub            = 103,

	BC_imul            = 104,
	BC_lmul            = 105,
	BC_fmul            = 106,
	BC_dmul            = 107,

	BC_idiv            = 108,
	BC_ldiv            = 109,
	BC_fdiv            = 110,
	BC_ddiv            = 111,

	BC_irem            = 112,
	BC_lrem            = 113,
	BC_frem            = 114,
	BC_drem            = 115,

	BC_ineg            = 116,
	BC_lneg            = 117,
	BC_fneg            = 118,
	BC_dneg            = 119,

	BC_ishl            = 120,
	BC_lshl            = 121,
	BC_ishr            = 122,
	BC_lshr            = 123,
	BC_iushr           = 124,
	BC_lushr           = 125,

	BC_iand            = 126,
	BC_land            = 127,
	BC_ior             = 128,
	BC_lor             = 129,
	BC_ixor            = 130,
	BC_lxor            = 131,

	BC_iinc            = 132,

	BC_i2l             = 133,
	BC_i2f             = 134,
	BC_i2d             = 135,
	BC_l2i             = 136,
	BC_l2f             = 137,
	BC_l2d             = 138,
	BC_f2i             = 139,
	BC_f2l             = 140,
	BC_f2d             = 141,
	BC_d2i             = 142,
	BC_d2l             = 143,
	BC_d2f             = 144,

	BC_int2byte        = 145,
	BC_int2char        = 146,
	BC_int2short       = 147,

	BC_lcmp            = 148,
	BC_fcmpl           = 149,
	BC_fcmpg           = 150,
	BC_dcmpl           = 151,
	BC_dcmpg           = 152,

	BC_ifeq            = 153,
	BC_ifne            = 154,
	BC_iflt            = 155,
	BC_ifge            = 156,
	BC_ifgt            = 157,
	BC_ifle            = 158,

	BC_if_icmpeq       = 159,
	BC_if_icmpne       = 160,
	BC_if_icmplt       = 161,
	BC_if_icmpge       = 162,
	BC_if_icmpgt       = 163,
	BC_if_icmple       = 164,
	BC_if_acmpeq       = 165,
	BC_if_acmpne       = 166,

	BC_goto            = 167,
	BC_jsr             = 168,
	BC_ret             = 169,

	BC_tableswitch     = 170,
	BC_lookupswitch    = 171,

	BC_ireturn         = 172,
	BC_lreturn         = 173,
	BC_freturn         = 174,
	BC_dreturn         = 175,
	BC_areturn         = 176,
	BC_return          = 177,

	BC_getstatic       = 178,
	BC_putstatic       = 179,
	BC_getfield        = 180,
	BC_putfield        = 181,

	BC_invokevirtual   = 182,
	BC_invokespecial   = 183,
	BC_invokestatic    = 184,
	BC_invokeinterface = 185,

	/* xxxunusedxxx 186 */

	BC_new             = 187,
	BC_newarray        = 188,
	BC_anewarray       = 189,

	BC_arraylength     = 190,

	BC_athrow          = 191,

	BC_checkcast       = 192,
	BC_instanceof      = 193,

	BC_monitorenter    = 194,
	BC_monitorexit     = 195,

	BC_wide            = 196,

	BC_multianewarray  = 197,

	BC_ifnull          = 198,
	BC_ifnonnull       = 199,

	BC_goto_w          = 200,
	BC_jsr_w           = 201,

	/* Reserved opcodes. */

	BC_breakpoint      = 202,

	BC_impdep1         = 254,
	BC_impdep2         = 255
};

#endif // BYTECODE_HPP_


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
