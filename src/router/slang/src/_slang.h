#ifndef PRIVATE_SLANG_H_
#define PRIVATE_SLANG_H_
/* header file for S-Lang internal structures that users do not (should not)
   need.  Use slang.h for that purpose. */
/*
Copyright (C) 2004-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

/* #include "config.h" */
#include "jdmacros.h"
#include "sllimits.h"

#ifdef VMS
# define SLANG_SYSTEM_NAME "_VMS"
#else
# if defined (IBMPC_SYSTEM)
#  define SLANG_SYSTEM_NAME "_IBMPC"
# else
#  define SLANG_SYSTEM_NAME "_UNIX"
# endif
#endif  /* VMS */

/* These quantities are main_types for byte-compiled code.  They are used
 * by the inner_interp routine.  The _BC_ means byte-code.
 */

/* Nametype byte-codes --- these must correspond to those in slang.h */
typedef enum
{
   SLANG_BC_LAST_BLOCK		= 0,
   SLANG_BC_LVARIABLE		= SLANG_LVARIABLE,   /* 0x01 */
   SLANG_BC_GVARIABLE		= SLANG_GVARIABLE,   /* 0x02 */
   SLANG_BC_IVARIABLE		= SLANG_IVARIABLE,   /* 0x03 */
   SLANG_BC_RVARIABLE		= SLANG_RVARIABLE,   /* 0x04 */
   SLANG_BC_INTRINSIC		= SLANG_INTRINSIC,   /* 0x05 */
   SLANG_BC_FUNCTION		= SLANG_FUNCTION,   /* 0x06 */
   SLANG_BC_MATH_UNARY		= SLANG_MATH_UNARY,   /* 0x07 */
   SLANG_BC_APP_UNARY		= SLANG_APP_UNARY,   /* 0x08 */
   SLANG_BC_ARITH_UNARY		= SLANG_ARITH_UNARY,   /* 0x09 */
   SLANG_BC_ARITH_BINARY	= SLANG_ARITH_BINARY,   /* 0x0A */
   SLANG_BC_ICONST		= SLANG_ICONSTANT,   /* 0x0B */
   SLANG_BC_DCONST		= SLANG_DCONSTANT,   /* 0x0C */
   SLANG_BC_FCONST		= SLANG_FCONSTANT,   /* 0x0D */
   SLANG_BC_LLCONST		= SLANG_LLCONSTANT,   /* 0x0E */
   SLANG_BC_PVARIABLE		= SLANG_PVARIABLE,   /* 0x0F */
   SLANG_BC_PFUNCTION		= SLANG_PFUNCTION,   /* 0x10 */
   SLANG_BC_HCONST		= SLANG_HCONSTANT,   /* 0x11 */
   SLANG_BC_LCONST		= SLANG_LCONSTANT,   /* 0x12 */
   SLANG_BC_UNUSED_0x12		= 0x12,
   SLANG_BC_UNUSED_0x13		= 0x13,
   SLANG_BC_UNUSED_0x14		= 0x14,
   SLANG_BC_UNUSED_0x15		= 0x15,
   SLANG_BC_UNUSED_0x16		= 0x16,
   SLANG_BC_UNUSED_0x17		= 0x17,
   SLANG_BC_UNUSED_0x18		= 0x18,
   SLANG_BC_UNUSED_0x19		= 0x19,
   SLANG_BC_UNUSED_0x1A		= 0x1A,
   SLANG_BC_UNUSED_0x1B		= 0x1B,
   SLANG_BC_UNUSED_0x1C		= 0x1C,
   SLANG_BC_UNUSED_0x1D		= 0x1D,
   SLANG_BC_UNUSED_0x1E		= 0x1E,
   SLANG_BC_UNUSED_0x1F		= 0x1F,

   /* bytes codes for setting/assigning variables/arrays/structures/refs */
   SLANG_BC_SET_LOCAL_LVALUE	= 0x20,
   SLANG_BC_SET_GLOBAL_LVALUE	= 0x21,
   SLANG_BC_SET_INTRIN_LVALUE	= 0x22,
   SLANG_BC_SET_STRUCT_LVALUE	= 0x23,
   SLANG_BC_SET_ARRAY_LVALUE	= 0x24,
   SLANG_BC_SET_DEREF_LVALUE	= 0x25,
   SLANG_BC_FIELD		= 0x26,
   SLANG_BC_METHOD		= 0x27,
   SLANG_BC_LVARIABLE_AGET	= 0x28,
   SLANG_BC_LVARIABLE_APUT	= 0x29,
   SLANG_BC_LOBJPTR		= 0x2A,
   SLANG_BC_GOBJPTR		= 0x2B,
   SLANG_BC_FIELD_REF		= 0x2C,
   SLANG_BC_OBSOLETE_DEREF_FUN_CALL	= 0x2D,
   SLANG_BC_DEREF_FUN_CALL	= 0x2E,
   SLANG_BC_UNUSED_0x2F		= 0x2F,
   SLANG_BC_UNUSED_0x30		= 0x30,
   SLANG_BC_UNUSED_0x31		= 0x31,
   SLANG_BC_UNUSED_0x32		= 0x32,
   SLANG_BC_UNUSED_0x33		= 0x33,
   SLANG_BC_UNUSED_0x34		= 0x34,
   SLANG_BC_UNUSED_0x35		= 0x35,
   SLANG_BC_UNUSED_0x36		= 0x36,
   SLANG_BC_UNUSED_0x37		= 0x37,
   SLANG_BC_UNUSED_0x38		= 0x38,
   SLANG_BC_UNUSED_0x39		= 0x39,
   SLANG_BC_UNUSED_0x3A		= 0x3A,
   SLANG_BC_UNUSED_0x3B		= 0x3B,
   SLANG_BC_UNUSED_0x3C		= 0x3C,
   SLANG_BC_UNUSED_0x3D		= 0x3D,
   SLANG_BC_UNUSED_0x3E		= 0x3E,
   SLANG_BC_UNUSED_0x3F		= 0x3F,

   /* byte codes for literals */
   SLANG_BC_LITERAL		= 0x40,           /* constant objects */
   SLANG_BC_LITERAL_INT		= 0x41,
   SLANG_BC_LITERAL_DBL		= 0x42,
   SLANG_BC_LITERAL_STR		= 0x43,
   SLANG_BC_DOLLAR_STR		= 0x44,
   SLANG_BC_LITERAL_SHORT	= 0x45,
   SLANG_BC_LITERAL_LONG	= 0x46,
   SLANG_BC_UNUSED_0x45		= 0x45,
   SLANG_BC_UNUSED_0x46		= 0x46,
   SLANG_BC_UNUSED_0x47		= 0x47,
   SLANG_BC_UNUSED_0x48		= 0x48,
   SLANG_BC_UNUSED_0x49		= 0x49,
   SLANG_BC_UNUSED_0x4A		= 0x4A,
   SLANG_BC_UNUSED_0x4B		= 0x4B,
   SLANG_BC_UNUSED_0x4C		= 0x4C,
   SLANG_BC_UNUSED_0x4D		= 0x4D,
   SLANG_BC_UNUSED_0x4E		= 0x4E,
   SLANG_BC_UNUSED_0x4F		= 0x4F,

   /* Unary/Binary operation codes */
   SLANG_BC_UNARY		= 0x50,
   SLANG_BC_BINARY		= 0x51,
   SLANG_BC_INTEGER_PLUS	= 0x52,
   SLANG_BC_INTEGER_MINUS	= 0x53,
   SLANG_BC_UNUSED_0x54		= 0x54,
   SLANG_BC_UNUSED_0x55		= 0x55,
   SLANG_BC_UNUSED_0x56		= 0x56,
   SLANG_BC_UNUSED_0x57		= 0x57,
   SLANG_BC_UNUSED_0x58		= 0x58,
   SLANG_BC_UNUSED_0x59		= 0x59,
   SLANG_BC_UNUSED_0x5A		= 0x5A,
   SLANG_BC_UNUSED_0x5B		= 0x5B,
   SLANG_BC_UNUSED_0x5C		= 0x5C,
   SLANG_BC_UNUSED_0x5D		= 0x5D,
   SLANG_BC_UNUSED_0x5E		= 0x5E,
   SLANG_BC_UNUSED_0x5F		= 0x5F,

   /* byte codes associated with keywords and blocks */
   SLANG_BC_TMP			= 0x60,
   SLANG_BC_EXCH		= 0x61,
   SLANG_BC_LABEL		= 0x62,
   SLANG_BC_BLOCK		= 0x63,
   SLANG_BC_RETURN		= 0x64,
   SLANG_BC_BREAK		= 0x65,
   SLANG_BC_CONTINUE		= 0x66,
   SLANG_BC_UNUSED_0x67		= 0x67,
   SLANG_BC_CONTINUE_N		= 0x68,
   SLANG_BC_BREAK_N		= 0x69,
   SLANG_BC_X_ERROR		= 0x6A,
   SLANG_BC_X_USER0		= 0x6B,
   SLANG_BC_X_USER1		= 0x6C,
   SLANG_BC_X_USER2		= 0x6D,
   SLANG_BC_X_USER3		= 0x6E,
   SLANG_BC_X_USER4		= 0x6F,

   /* byte codes for dealing with the frame pointer and arg list */
   SLANG_BC_CALL_DIRECT		= 0x70,
   SLANG_BC_CALL_DIRECT_FRAME	= 0x71,
   SLANG_BC_CALL_DIRECT_NARGS	= 0x72,
   SLANG_BC_EARG_LVARIABLE	= 0x73,
#define USE_BC_LINE_NUM	0	       /* used in slang.c */
   /* SLANG_BC_LINE_NUM		= 0x74, */
   SLANG_BC_UNUSED_0x74		= 0x74,
   SLANG_BC_BOS			= 0x75,
   SLANG_BC_EOS			= 0x76,
   SLANG_BC_UNUSED_0x77		= 0x77,
   SLANG_BC_UNUSED_0x78		= 0x78,
   SLANG_BC_UNUSED_0x79		= 0x79,
   SLANG_BC_UNUSED_0x7A		= 0x7A,
   SLANG_BC_UNUSED_0x7B		= 0x7B,
   SLANG_BC_UNUSED_0x7C		= 0x7C,
   SLANG_BC_UNUSED_0x7D		= 0x7D,
   SLANG_BC_UNUSED_0x7E		= 0x7E,
   SLANG_BC_UNUSED_0x7F		= 0x7F,

   /* These are used only when compiled with USE_COMBINED_BYTECODES */
   SLANG_BC_CALL_DIRECT_INTRINSIC= 0x80,
   SLANG_BC_INTRINSIC_CALL_DIRECT= 0x81,
   SLANG_BC_CALL_DIRECT_LSTR	= 0x82,
   SLANG_BC_CALL_DIRECT_SLFUN	= 0x83,
   SLANG_BC_CALL_DIRECT_RETINTR	= 0x84,
   SLANG_BC_RET_INTRINSIC	= 0x85,
   SLANG_BC_CALL_DIRECT_EARG_LVAR= 0x86,
   SLANG_BC_CALL_DIRECT_LINT	= 0x87,
   SLANG_BC_CALL_DIRECT_LVAR	= 0x88,
   SLANG_BC_LLVARIABLE_BINARY	= 0x89,
   SLANG_BC_LGVARIABLE_BINARY	= 0x8A,
   SLANG_BC_GLVARIABLE_BINARY	= 0x8B,
   SLANG_BC_GGVARIABLE_BINARY	= 0x8C,
   SLANG_BC_LIVARIABLE_BINARY	= 0x8D,
   SLANG_BC_LDVARIABLE_BINARY	= 0x8E,
   SLANG_BC_ILVARIABLE_BINARY	= 0x8F,
   SLANG_BC_DLVARIABLE_BINARY	= 0x90,
   SLANG_BC_LVARIABLE_BINARY	= 0x91,
   SLANG_BC_GVARIABLE_BINARY	= 0x92,
   SLANG_BC_LITERAL_INT_BINARY	= 0x93,
   SLANG_BC_LITERAL_DBL_BINARY	= 0x94,
   SLANG_BC_LASSIGN_LLBINARY	= 0x95,
   SLANG_BC_LASSIGN_LIBINARY	= 0x96,
   SLANG_BC_LASSIGN_ILBINARY	= 0x97,
   SLANG_BC_LASSIGN_LDBINARY	= 0x98,
   SLANG_BC_LASSIGN_DLBINARY	= 0x99,
   SLANG_BC_RET_LVARIABLE	= 0x9A,
   SLANG_BC_RET_LITERAL_INT	= 0x9B,
   SLANG_BC_MANY_LVARIABLE	= 0x9C,
   SLANG_BC_MANY_LVARIABLE_DIR	= 0x9D,
   SLANG_BC_LVARIABLE_AGET1	= 0x9E,
   SLANG_BC_LITERAL_AGET1	= 0x9F,
   SLANG_BC_LVAR_LVAR_APUT1	= 0xA0,
   SLANG_BC_LVARIABLE_APUT1	= 0xA1,
   SLANG_BC_LITERAL_APUT1	= 0xA2,
   SLANG_BC_LLVARIABLE_BINARY2	= 0xA3,
   SLANG_BC_SET_LOCLV_LIT_INT	= 0xA4,
   SLANG_BC_SET_LOCLV_LIT_AGET1	= 0xA5,
   SLANG_BC_SET_LOCLV_LVAR	= 0xA6,
   SLANG_BC_SET_LOCLV_LASTBLOCK	= 0xA7,
   SLANG_BC_LVAR_EARG_LVAR	= 0xA8,
   SLANG_BC_LVAR_FIELD		= 0xA9,
   SLANG_BC_BINARY_LASTBLOCK	= 0xAA,
   SLANG_BC_EARG_LVARIABLE_INTRINSIC	= 0xAB,
   SLANG_BC_LVAR_LITERAL_INT	= 0xAC,
   SLANG_BC_BINARY_SET_LOCLVAL	= 0xAD,
   SLANG_BC_LVAR_AGET_SET_LOCLVAL	= 0xAE,
   SLANG_BC_LLVAR_BINARY_IF	= 0xAF,

   SLANG_BC_IF_BLOCK		= 0xB0,
   SLANG_BC_LVAR_SET_FIELD	= 0xB1,
   SLANG_BC_PVAR_SET_GLOB_LVAL	= 0xB2,
   SLANG_BC_LVAR_SET_GLOB_LVAL	= 0xB3,
   SLANG_BC_LIT_AGET1_INT_BINARY= 0xB4,
   SLANG_BC_BINARY2		= 0xB5,
   SLANG_BC_LVAR_LIT_AGET1	= 0xB6,
   SLANG_BC_UNUSED_0xB7		= 0xB7,
   SLANG_BC_UNUSED_0xB8		= 0xB8,
   SLANG_BC_UNUSED_0xB9		= 0xB9,
   SLANG_BC_UNUSED_0xBA		= 0xBA,
   SLANG_BC_UNUSED_0xBB		= 0xBB,
   SLANG_BC_UNUSED_0xBC		= 0xBC,
   SLANG_BC_UNUSED_0xBD		= 0xBD,
   SLANG_BC_UNUSED_0xBE		= 0xBE,
   SLANG_BC_UNUSED_0xBF		= 0xBF,

   /* The following do not actually occur in inner_interp.  They used
    * to signify the bytecode has been combined with another.
    */
   SLANG_BC_LVARIABLE_COMBINED	= 0xC0,
   SLANG_BC_GVARIABLE_COMBINED	= 0xC1,
   SLANG_BC_LITERAL_COMBINED	= 0xC2,
   SLANG_BC_CALL_DIRECT_COMB	= 0xC3,
   SLANG_BC_COMBINED		= 0xC4,
   SLANG_BC_BLOCK_COMBINED	= 0xC5,
#define SLANG_IS_BC_COMBINED(b) ((0xC0 <= (b)) && ((b) <= 0xC5))
   SLANG_BC_UNUSED_0xC6		= 0xC6,
   SLANG_BC_UNUSED_0xC7		= 0xC7,
   SLANG_BC_UNUSED_0xC8		= 0xC8,
   SLANG_BC_UNUSED_0xC9		= 0xC9,
   SLANG_BC_UNUSED_0xCA		= 0xCA,
   SLANG_BC_UNUSED_0xCB		= 0xCB,
   SLANG_BC_UNUSED_0xCC		= 0xCC,
   SLANG_BC_UNUSED_0xCD		= 0xCD,
   SLANG_BC_UNUSED_0xCE		= 0xCE,
   SLANG_BC_UNUSED_0xCF		= 0xCF,
   SLANG_BC_UNUSED_0xD0		= 0xD0,
   SLANG_BC_UNUSED_0xD1		= 0xD1,
   SLANG_BC_UNUSED_0xD2		= 0xD2,
   SLANG_BC_UNUSED_0xD3		= 0xD3,
   SLANG_BC_UNUSED_0xD4		= 0xD4,
   SLANG_BC_UNUSED_0xD5		= 0xD5,
   SLANG_BC_UNUSED_0xD6		= 0xD6,
   SLANG_BC_UNUSED_0xD7		= 0xD7,
   SLANG_BC_UNUSED_0xD8		= 0xD8,
   SLANG_BC_UNUSED_0xD9		= 0xD9,
   SLANG_BC_UNUSED_0xDA		= 0xDA,
   SLANG_BC_UNUSED_0xDB		= 0xDB,
   SLANG_BC_UNUSED_0xDC		= 0xDC,
   SLANG_BC_UNUSED_0xDD		= 0xDD,
   SLANG_BC_UNUSED_0xDE		= 0xDE,
   SLANG_BC_UNUSED_0xDF		= 0xDF,
   SLANG_BC_UNUSED_0xE0		= 0xE0,
   SLANG_BC_UNUSED_0xE1		= 0xE1,
   SLANG_BC_UNUSED_0xE2		= 0xE2,
   SLANG_BC_UNUSED_0xE3		= 0xE3,
   SLANG_BC_UNUSED_0xE4		= 0xE4,
   SLANG_BC_UNUSED_0xE5		= 0xE5,
   SLANG_BC_UNUSED_0xE6		= 0xE6,
   SLANG_BC_UNUSED_0xE7		= 0xE7,
   SLANG_BC_UNUSED_0xE8		= 0xE8,
   SLANG_BC_UNUSED_0xE9		= 0xE9,
   SLANG_BC_UNUSED_0xEA		= 0xEA,
   SLANG_BC_UNUSED_0xEB		= 0xEB,
   SLANG_BC_UNUSED_0xEC		= 0xEC,
   SLANG_BC_UNUSED_0xED		= 0xED,
   SLANG_BC_UNUSED_0xEE		= 0xEE,
   SLANG_BC_UNUSED_0xEF		= 0xEF,
   SLANG_BC_UNUSED_0xF0		= 0xF0,
   SLANG_BC_UNUSED_0xF1		= 0xF1,
   SLANG_BC_UNUSED_0xF2		= 0xF2,
   SLANG_BC_UNUSED_0xF3		= 0xF3,
   SLANG_BC_UNUSED_0xF4		= 0xF4,
   SLANG_BC_UNUSED_0xF5		= 0xF5,
   SLANG_BC_UNUSED_0xF6		= 0xF6,
   SLANG_BC_UNUSED_0xF7		= 0xF7,
   SLANG_BC_UNUSED_0xF8		= 0xF8,
   SLANG_BC_UNUSED_0xF9		= 0xF9,
   SLANG_BC_UNUSED_0xFA		= 0xFA,
   SLANG_BC_UNUSED_0xFB		= 0xFB,
   SLANG_BC_UNUSED_0xFC		= 0xFC,
   SLANG_BC_UNUSED_0xFD		= 0xFD,
   SLANG_BC_UNUSED_0xFE		= 0xFE,
   SLANG_BC_UNUSED_0xFF		= 0xFF
}
_pSLang_BC_Type;

/* Byte-Code Sub Types (_BCST_) */

/* These are sub_types of SLANG_BC_BLOCK */
#define SLANG_BCST_ERROR_BLOCK	0x01
#define SLANG_BCST_EXIT_BLOCK	0x02
#define SLANG_BCST_USER_BLOCK0	0x03
#define SLANG_BCST_USER_BLOCK1	0x04
#define SLANG_BCST_USER_BLOCK2	0x05
#define SLANG_BCST_USER_BLOCK3	0x06
#define SLANG_BCST_USER_BLOCK4	0x07
/* The user blocks MUST be in the above order */
#define SLANG_BCST_LOOP	0x10
#define SLANG_BCST_WHILE	0x11
#define SLANG_BCST_FOR		0x12
#define SLANG_BCST_FOREVER	0x13
#define SLANG_BCST_CFOR	0x14
#define SLANG_BCST_DOWHILE	0x15
#define SLANG_BCST_FOREACH	0x16   /* obsolete */
#define SLANG_BCST_TRY		0x17
#define SLANG_BCST_FOREACH_EARGS	0x18

#define SLANG_BCST_IF		0x20
#define SLANG_BCST_IFNOT	0x21
#define SLANG_BCST_ELSE		0x22
#define SLANG_BCST_ANDELSE	0x23
#define SLANG_BCST_ORELSE	0x24
#define SLANG_BCST_SWITCH	0x25
#define SLANG_BCST_NOTELSE	0x26
#define SLANG_BCST_SC_OR	0x27
#define SLANG_BCST_SC_AND	0x28

#define SLANG_BCST_LOOP_ELSE	0x29
#define SLANG_BCST_LOOP_THEN	0x30
#define SLANG_BCST_COMPARE      0x31

/* assignment (SLANG_BC_SET_*_LVALUE) subtypes.  The order MUST correspond
 * to the assignment token order with the ASSIGN_TOKEN as the first!
 */
#define SLANG_BCST_ASSIGN		0x01
#define SLANG_BCST_PLUSEQS		0x02
#define SLANG_BCST_MINUSEQS		0x03
#define SLANG_BCST_TIMESEQS		0x04
#define SLANG_BCST_DIVEQS		0x05
#define SLANG_BCST_BOREQS		0x06
#define SLANG_BCST_BANDEQS		0x07
#define SLANG_BCST_PLUSPLUS		0x08
#define SLANG_BCST_POST_PLUSPLUS	0x09
#define SLANG_BCST_MINUSMINUS		0x0A
#define SLANG_BCST_POST_MINUSMINUS	0x0B

/* These use SLANG_PLUS, SLANG_MINUS, SLANG_PLUSPLUS, etc... */

#define LONG_IS_INT (SIZEOF_INT == SIZEOF_LONG)
#define LONG_IS_NOT_INT (SIZEOF_INT != SIZEOF_LONG)
#define SHORT_IS_INT (SIZEOF_INT == SIZEOF_SHORT)
#define SHORT_IS_NOT_INT (SIZEOF_INT != SIZEOF_SHORT)
#define LLONG_IS_LONG (SIZEOF_LONG == SIZEOF_LONG_LONG)
#define LLONG_IS_NOT_LONG (SIZEOF_LONG != SIZEOF_LONG_LONG)

/* If long or short are ints, then map the slang types to ints.  This is
 * done because slang has some optimizations for ints.
 */
#if LONG_IS_INT
# define _pSLANG_LONG_TYPE SLANG_INT_TYPE
# define _pSLANG_ULONG_TYPE SLANG_UINT_TYPE
#else
# define _pSLANG_LONG_TYPE SLANG_LONG_TYPE
# define _pSLANG_ULONG_TYPE SLANG_ULONG_TYPE
#endif
#if SHORT_IS_INT
# define _pSLANG_SHORT_TYPE SLANG_INT_TYPE
# define _pSLANG_USHORT_TYPE SLANG_UINT_TYPE
#else
# define _pSLANG_SHORT_TYPE SLANG_SHORT_TYPE
# define _pSLANG_USHORT_TYPE SLANG_USHORT_TYPE
#endif
#if LLONG_IS_LONG
# define _pSLANG_LLONG_TYPE _pSLANG_LONG_TYPE
# define _pSLANG_ULLONG_TYPE _pSLANG_ULONG_TYPE
#else
# define _pSLANG_LLONG_TYPE SLANG_LLONG_TYPE
# define _pSLANG_ULLONG_TYPE SLANG_ULLONG_TYPE
#endif

/* Map off_t to a slang type */
#if defined(HAVE_LONG_LONG) && (SIZEOF_OFF_T == SIZEOF_LONG_LONG) && (SIZEOF_LONG_LONG > SIZEOF_LONG)
# define SLANG_C_OFF_T_TYPE _pSLANG_LLONG_TYPE
typedef long long _pSLc_off_t_Type;
# define SLANG_PUSH_OFF_T SLang_push_long_long
#else
# if (SIZEOF_OFF_T == SIZEOF_INT)
#  define SLANG_C_OFF_T_TYPE SLANG_INT_TYPE
#  define SLANG_PUSH_OFF_T SLang_push_int
typedef int _pSLc_off_t_Type;
# else
#  define SLANG_C_OFF_T_TYPE _pSLANG_LONG_TYPE
#  define SLANG_PUSH_OFF_T SLang_push_long
typedef long _pSLc_off_t_Type;
# endif
#endif

#if SIZEOF_INT == 2
# define _pSLANG_INT16_TYPE	SLANG_INT_TYPE
# define _pSLANG_UINT16_TYPE	SLANG_UINT_TYPE
typedef int _pSLint16_Type;
typedef unsigned int _pSLuint16_Type;
#else
# if SIZEOF_SHORT == 2
#  define _pSLANG_INT16_TYPE	SLANG_SHORT_TYPE
#  define _pSLANG_UINT16_TYPE	SLANG_USHORT_TYPE
typedef short _pSLint16_Type;
typedef unsigned short _pSLuint16_Type;
# else
#  if SIZEOF_LONG == 2
#   define _pSLANG_INT16_TYPE	SLANG_LONG_TYPE
#   define _pSLANG_UINT16_TYPE	SLANG_ULONG_TYPE
typedef long _pSLInt16_Type;
typedef unsigned long _pSLuint16_Type;
#  else
#   define _pSLANG_INT16_TYPE	0
#   define _pSLANG_UINT16_TYPE	0
#  endif
# endif
#endif

#if SIZEOF_INT == 4
# define _pSLANG_INT32_TYPE	SLANG_INT_TYPE
# define _pSLANG_UINT32_TYPE	SLANG_UINT_TYPE
typedef int _pSLint32_Type;
typedef unsigned int _pSLuint32_Type;
#else
# if SIZEOF_SHORT == 4
#  define _pSLANG_INT32_TYPE	SLANG_SHORT_TYPE
#  define _pSLANG_UINT32_TYPE	SLANG_USHORT_TYPE
typedef short _pSLInt32_Type;
typedef unsigned short _pSLuint32_Type;
# else
#  if SIZEOF_LONG == 4
#   define _pSLANG_INT32_TYPE	SLANG_LONG_TYPE
#   define _pSLANG_UINT32_TYPE	SLANG_ULONG_TYPE
typedef long _pSLInt32_Type;
typedef unsigned long _pSLuint32_Type;
#  else
#   define _pSLANG_INT32_TYPE	0
#   define _pSLANG_UINT32_TYPE	0
#  endif
# endif
#endif

#if SIZEOF_INT == 8
# define _pSLANG_INT64_TYPE	SLANG_INT_TYPE
# define _pSLANG_UINT64_TYPE	SLANG_UINT_TYPE
typedef int _pSLint64_Type;
typedef unsigned int _pSLuint64_Type;
#else
# if SIZEOF_SHORT == 8
#  define _pSLANG_INT64_TYPE	SLANG_SHORT_TYPE
#  define _pSLANG_UINT64_TYPE	SLANG_USHORT_TYPE
typedef int _pSLInt64_Type;
typedef unsigned int _pSLuint64_Type;
# else
#  if SIZEOF_LONG == 8
#   define _pSLANG_INT64_TYPE	SLANG_LONG_TYPE
#   define _pSLANG_UINT64_TYPE	SLANG_ULONG_TYPE
typedef long _pSLInt64_Type;
typedef unsigned long _pSLuint64_Type;
#  else
#   if SIZEOF_LONG_LONG == 8
#    define _pSLANG_INT64_TYPE	SLANG_LLONG_TYPE
#    define _pSLANG_UINT64_TYPE	SLANG_ULLONG_TYPE
typedef long long _pSLInt64_Type;
typedef unsigned long long _pSLuint64_Type;
#   else
#    define _pSLANG_INT64_TYPE	0
#    define _pSLANG_UINT64_TYPE	0
#   endif
#  endif
# endif
#endif

typedef union
{
#if SLANG_HAS_FLOAT
   double double_val;
   float float_val;
#endif
#ifdef HAVE_LONG_LONG
   long long llong_val;
   unsigned long long ullong_val;
#endif
   long long_val;
   unsigned long ulong_val;
   VOID_STAR ptr_val;
   char *s_val;
   int int_val;
   unsigned int uint_val;
   SLang_MMT_Type *ref;
   SLang_Name_Type *n_val;
   struct _pSLang_Struct_Type *struct_val;
   struct _pSLang_Array_Type *array_val;
   short short_val;
   unsigned short ushort_val;
   char char_val;
   unsigned char uchar_val;
   SLindex_Type index_val;
}
_pSL_Object_Union_Type;

typedef struct _pSLang_Object_Type
{
   SLtype o_data_type;	       /* SLANG_INT_TYPE, ... */
   _pSL_Object_Union_Type v;
}
SLang_Object_Type;

typedef struct
{
   char *name;
   SLang_Name_Type *next;
   char name_type;

   SLang_Object_Type obj;
}
SLang_Global_Var_Type;

struct _pSLang_MMT_Type
{
   SLtype data_type;	       /* int, string, etc... */
   VOID_STAR user_data;	       /* address of user structure */
   unsigned int count;		       /* number of references */
};

extern int _pSLang_pop_object_of_type (SLtype, SLang_Object_Type *, int);

typedef struct
{
   SLFUTURE_CONST char *name;			       /* slstring */
   SLang_Object_Type obj;
}
_pSLstruct_Field_Type;

typedef struct _pSLang_Struct_Type
{
   _pSLstruct_Field_Type *fields;
   unsigned int nfields;	       /* number used */
   unsigned int num_refs;
   /* user-defined methods */
   SLang_Name_Type *destroy_method;
}
_pSLang_Struct_Type;

extern int _pSLstruct_init (void);
extern int _pSLstruct_define_struct (void);
extern int _pSLstruct_define_struct2 (void);
extern int _pSLstruct_define_typedef (void);

extern SLang_Object_Type *_pSLstruct_get_field_value (SLang_Struct_Type *, SLCONST char *);
extern int _pSLstruct_push_field_ref (SLFUTURE_CONST char *);
extern int _pSLstruct_push_field (SLang_Struct_Type *s, SLFUTURE_CONST char *name, int do_free);
extern int _pSLstruct_pop_field (SLang_Struct_Type *s, SLFUTURE_CONST char *name, int do_free);

extern int _pSLang_get_qualifiers_intrin (SLang_Struct_Type **);

/* The following qualifier functions will eventually be part of the public API */
extern int _pSLang_qualifier_exists (SLCONST char *);
extern int _pSLang_get_int_qualifier (SLCONST char *, int *, int);
extern int _pSLang_get_string_qualifier (SLCONST char *, char **p, SLFUTURE_CONST char *);

struct _pSLang_Ref_Type
{
   int num_refs;
   VOID_STAR data;
   unsigned int sizeof_data;
   int data_is_nametype;	       /* used for optimization */
   int (*deref_assign)(VOID_STAR);
   int (*deref) (VOID_STAR);
   char *(*string)(VOID_STAR);		       /* returns a malloced string */
   void (*destroy) (VOID_STAR);
   int (*is_initialized) (VOID_STAR);
   int (*uninitialize) (VOID_STAR);
};

extern int _pSLang_dereference_ref (SLang_Ref_Type *);
extern int _pSLang_deref_assign (SLang_Ref_Type *);
extern int SLang_push_ref (SLang_Ref_Type *);
extern int _pSLang_push_nt_as_ref (SLang_Name_Type *);
extern SLang_Ref_Type *_pSLang_new_ref (unsigned int);

extern int _pSL_increment_frame_pointer (void);
extern int _pSL_decrement_frame_pointer (void);

extern int SLang_pop(SLang_Object_Type *);
extern void SLang_free_object (SLang_Object_Type *);
extern int _pSLanytype_typecast (SLtype, VOID_STAR, SLuindex_Type,
				 SLtype, VOID_STAR);
extern void _pSLstring_intrinsic (void);
extern int _pSLformat_as_binary (unsigned int min_num_bits, int use_binary_prefix);

#if 0
/* NULL not allowed here */
extern char *_pSLmalloced_to_slstring (char *, unsigned int);
/* But here it is ok */
extern int _pSLpush_malloced_string (char *, unsigned int);
#endif

/* These functions are used to create slstrings of a fixed length.  Be
 * very careful how they are used.  In particular, if len bytes are allocated,
 * then the string must be len characters long, no more and no less.
 */
extern char *_pSLallocate_slstring (unsigned int);
extern char *_pSLcreate_via_alloced_slstring (char *, unsigned int);
extern void _pSLunallocate_slstring (char *, unsigned int);
extern int _pSLpush_alloced_slstring (char *, unsigned int);

extern unsigned int _pSLstring_bytelen (SLstr_Type *);
extern void _pSLang_free_slstring (SLstr_Type *);   /* slstring required and assumed */
extern unsigned long _pSLstring_get_hash (SLstr_Type *s);   /* slstring required */

typedef struct
{
   char **buf;
   unsigned int max_num;
   unsigned int num;
   unsigned int delta_num;
   int is_malloced;		       /* non-zero if object was malloced */
}
_pSLString_List_Type;

/* Note that _pSLstring_list_append makes no copy of the object-- it steals it.
 * For a copy, use _pSLstring_list_append_copy */
extern int _pSLstring_list_append (_pSLString_List_Type *, char *);
extern int _pSLstring_list_append_copy (_pSLString_List_Type *, char *);
extern int _pSLstring_list_init (_pSLString_List_Type *, unsigned int, unsigned int);
extern _pSLString_List_Type *_pSLstring_list_new (unsigned int, unsigned int);
extern void _pSLstring_list_delete (_pSLString_List_Type *);
extern int _pSLstring_list_push (_pSLString_List_Type *, int);
extern SLang_Array_Type *_pSLstrings_to_array (char **strs, unsigned int n);

/* This function assumes that s is an slstring. */
extern SLCONST char *_pSLstring_dup_slstring (SLCONST char *);
extern int _pSLang_dup_and_push_slstring (SLCONST char *);

extern int _pSLang_init_import (void);
extern int _pSLinit_exceptions (void);

/* This function checks to see if the referenced object is initialized */
extern int _pSLang_is_ref_initialized (SLang_Ref_Type *);
extern int _pSLcheck_identifier_syntax (SLCONST char *);
extern int _pSLang_uninitialize_ref (SLang_Ref_Type *);

extern int _pSLpush_slang_obj (SLang_Object_Type *);
extern int _pSLslang_copy_obj (SLang_Object_Type *obja, SLang_Object_Type *objb);

extern char *_pSLexpand_escaped_char(char *, char *, SLwchar_Type *, int *);
extern void _pSLexpand_escaped_string (char *, char *, char *);

extern int _pSLpush_dollar_string (SLFUTURE_CONST char *);

/* returns a pointer to an SLstring string-- use SLang_free_slstring */
extern char *_pSLstringize_object (SLang_Object_Type *);
extern int _pSLdump_objects (char *, SLang_Object_Type *, unsigned int, int);

extern SLang_Object_Type *_pSLang_get_run_stack_pointer (void);
extern SLang_Object_Type *_pSLang_get_run_stack_base (void);
extern int _pSLang_dump_stack (void);
extern int _pSLang_peek_at_stack2 (SLtype *);
extern int _pSLang_restart_arg_list (int nargs);

struct _pSLang_NameSpace_Type
{
   struct _pSLang_NameSpace_Type *next;
   SLFUTURE_CONST char *name;	       /* this is the load_type name */
   SLFUTURE_CONST char *namespace_name;/* this name is assigned by implements */
   SLFUTURE_CONST char *private_name;
   unsigned int table_size;
   SLang_Name_Type **table;
};
extern SLang_NameSpace_Type *_pSLns_new_namespace (SLFUTURE_CONST char *, unsigned int);
extern SLang_NameSpace_Type *_pSLns_allocate_namespace (SLFUTURE_CONST char *, unsigned int);
extern void _pSLns_deallocate_namespace (SLang_NameSpace_Type *);
extern SLang_NameSpace_Type *_pSLns_find_namespace (SLCONST char *);
extern int _pSLns_set_namespace_name (SLang_NameSpace_Type *, SLFUTURE_CONST char *);
extern SLang_Array_Type *_pSLnspace_apropos (SLang_NameSpace_Type *, SLFUTURE_CONST char *, unsigned int);
extern void _pSLang_use_namespace_intrinsic (char *name);
extern SLFUTURE_CONST char *_pSLang_cur_namespace_intrinsic (void);
extern SLang_Array_Type *_pSLang_apropos (SLFUTURE_CONST char *, SLFUTURE_CONST char *, unsigned int);
extern void _pSLang_implements_intrinsic (SLFUTURE_CONST char *);
extern SLang_Array_Type *_pSLns_list_namespaces (void);
extern SLang_Name_Type *_pSLns_locate_hashed_name (SLang_NameSpace_Type *, SLCONST char *, unsigned long);
extern int _pSLns_add_hashed_name (SLang_NameSpace_Type *, SLang_Name_Type *, unsigned long);
extern SLang_NameSpace_Type *_pSLns_find_object_namespace (SLang_Name_Type *nt);
extern SLang_Name_Type *_pSLns_locate_name (SLang_NameSpace_Type *, SLCONST char *);
extern SLang_NameSpace_Type *_pSLns_get_private_namespace (SLFUTURE_CONST char *name, SLFUTURE_CONST char *nsname);
extern SLang_NameSpace_Type *_pSLns_create_namespace2 (SLFUTURE_CONST char *name, SLFUTURE_CONST char *nsname);
extern void _pSLns_delete_namespaces (void);

/* The long-long constants functions need to be fixed in slang.h.  This will
 * have to wait for a major version change.
 */
#ifdef HAVE_LONG_LONG
typedef struct
{
   SLFUTURE_CONST char *name;
   SLang_Name_Type *next;
   char name_type;

   SLtype data_type;
   long long value;
}
_pSLang_LLConstant_Type;
SL_EXTERN int _pSLns_add_llconstant_table (SLang_NameSpace_Type *, _pSLang_LLConstant_Type *, SLFUTURE_CONST char *);
SL_EXTERN int _pSLadd_llconstant_table (_pSLang_LLConstant_Type *, SLFUTURE_CONST char *);
SL_EXTERN int _pSLns_add_llconstant (SLang_NameSpace_Type *, SLFUTURE_CONST char *, SLtype, long long);
# define _pMAKE_LLCONSTANT_T(n,val,T) \
    {(n),NULL, SLANG_LLCONSTANT, T, (long long)(val)}
# define _pSLANG_END_LLCONST_TABLE _pMAKE_LLCONSTANT_T(NULL,0,0)
#endif

extern int _pSLang_Trace;
extern char SLCONST *_pSLang_current_function_name (void);

extern int _pSLang_trace_fun(SLFUTURE_CONST char *);

/* This is a bitmapped variable */
/* extern int _pSLang_Compile_Line_Num_Info; */
#if SLANG_HAS_BOSEOS
extern int _pSLang_Compile_BOSEOS;
#define SLANG_BOSEOS_VALUE_BITS		0x0FF
#define SLANG_BOSEOS_PREPROC		0x100
extern int _pSLang_Compile_BOFEOF;
extern int _pSLang_init_boseos (void);
extern int _pSLcall_bos_handler (SLFUTURE_CONST char *, int);
extern int _pSLcall_eos_handler (void);
extern int _pSLcall_bof_handler (SLFUTURE_CONST char *, SLFUTURE_CONST char *);
extern int _pSLcall_eof_handler (void);
extern int _pSLcall_debug_hook (SLFUTURE_CONST char *file, int linenum);
/* extern int _pSLcall_debug_hook (char *file, int linenum, char *funct); */
#endif

extern char *_pSLstring_dup_hashed_string (SLCONST char *, unsigned long);
extern unsigned long _pSLcompute_string_hash (SLCONST char *);
extern char *_pSLstring_make_hashed_string (SLCONST char *, unsigned int, unsigned long *);
extern void _pSLfree_hashed_string (char *, unsigned int, unsigned long);
unsigned long _pSLstring_hash (SLCONST unsigned char *, SLCONST unsigned char *);
extern int _pSLinit_slcomplex (void);

extern int _pSLang_init_slstrops (void);
extern int _pSLstrops_do_sprintf_n (int);
extern int _pSLang_sscanf (void);
extern double _pSLang_atof (SLFUTURE_CONST char *);

extern int _pSLang_init_bstring (void);
extern SLang_Foreach_Context_Type *_pSLbstring_foreach_open (SLtype type, unsigned int num);
extern void _pSLbstring_foreach_close (SLtype type, SLang_Foreach_Context_Type *c);
extern int _pSLbstring_foreach (SLtype type, SLang_Foreach_Context_Type *c);

extern int _pSLang_init_sltime (void);
extern void _pSLpack (void);
extern void _pSLunpack (char *, SLang_BString_Type *);
extern void _pSLpack_pad_format (char *);
extern unsigned int _pSLpack_compute_size (char *);
extern int _pSLusleep (unsigned long);

/* frees upon error.  NULL __NOT__ ok. */
extern int _pSLang_push_slstring (char *);

extern SLtype _pSLarith_promote_type (SLtype);
extern int _pSLarith_get_precedence (SLtype);
extern int _pSLarith_typecast (SLtype, VOID_STAR, SLuindex_Type,
			       SLtype, VOID_STAR);

extern int SLang_push(SLang_Object_Type *);
extern int SLadd_global_variable (SLCONST char *);

extern int _pSLdo_pop (void);
extern unsigned int _pSLsys_getkey (void);
extern int _pSLsys_input_pending (int);
#ifdef IBMPC_SYSTEM
extern unsigned int _pSLpc_convert_scancode (unsigned int, unsigned int, int);
#define _pSLTT_KEY_SHIFT	1
#define _pSLTT_KEY_CTRL		2
#define _pSLTT_KEY_ALT		4
#endif

typedef struct _pSLterminfo_Type SLterminfo_Type;
extern SLterminfo_Type *_pSLtt_tigetent (SLCONST char *);
extern void _pSLtt_tifreeent (SLterminfo_Type *);
extern char *_pSLtt_tigetstr (SLterminfo_Type *, SLCONST char *);
extern int _pSLtt_tigetnum (SLterminfo_Type *, SLCONST char *);
extern int _pSLtt_tigetflag (SLterminfo_Type *, SLCONST char *);

#if defined(REAL_UNIX_SYSTEM) || defined(VMS)
extern void SLtt_init_keypad (void);
extern void SLtt_deinit_keypad (void);
#endif

#if SLTT_HAS_NON_BCE_SUPPORT
extern int _pSLtt_get_bce_color_offset (void);
#endif
extern void (*_pSLtt_color_changed_hook)(void);
extern int _pSLtt_init_cmdline_mode (void);
extern void _pSLtt_cmdline_mode_reset (void);

extern int _pSLsmg_init_smg_cmdline (void);

extern unsigned char SLang_Input_Buffer [SL_MAX_INPUT_BUFFER_LEN];

typedef struct SL_OOBinary_Type
{
   SLtype data_type;	       /* partner type for binary op */

    int (*binary_function)_PROTO((int,
				 SLtype, VOID_STAR, SLuindex_Type,
				 SLtype, VOID_STAR, SLuindex_Type,
				 VOID_STAR));

   int (*binary_result) _PROTO((int, SLtype, SLtype, SLtype *));
   struct SL_OOBinary_Type *next;
}
SL_OOBinary_Type;

typedef struct _pSL_Typecast_Type
{
   SLtype data_type;	       /* to_type */
   int allow_implicit;

   int (*typecast)_PROTO((SLtype, VOID_STAR, SLuindex_Type,
			  SLtype, VOID_STAR));
   struct _pSL_Typecast_Type *next;
}
SL_Typecast_Type;

struct _pSLang_Class_Type
{
   SLclass_Type cl_class_type;	       /* vector, scalar, mmt, pointer */

   unsigned int cl_data_type;	       /* SLANG_INTEGER_TYPE, etc... */
   char *cl_name;			       /* slstring type */

   size_t cl_sizeof_type;
   VOID_STAR cl_transfer_buf;	       /* cl_sizeof_type bytes*/

   /* Methods */

   /* Most of the method functions are prototyped:
    * int method (SLtype type, VOID_STAR addr);
    * Here, @type@ represents the type of object that the method is asked
    * to deal with.  The second parameter @addr@ will contain the ADDRESS of
    * the object.  For example, if type is SLANG_INT_TYPE, then @addr@ will
    * actually be int *.  Similary, if type is SLANG_STRING_TYPE,
    * then @addr@ will contain the address of the string, i.e., char **.
    */

   void (*cl_destroy)_PROTO((SLtype, VOID_STAR));
   /* Prototype: void destroy(unsigned type, VOID_STAR val)
    * Called to delete/free the object */

   char *(*cl_string)_PROTO((SLtype, VOID_STAR));
   /* Prototype: char *to_string (SLtype t, VOID_STAR p);
    * Here p is a pointer to the object for which a string representation
    * is to be returned.  The returned pointer is to be a MALLOCED string.
    */

   /* Prototype: void push(SLtype type, VOID_STAR v);
    * Push a copy of the object of type @type@ at address @v@ onto the
    * stack.
    */
   int (*cl_push)_PROTO((SLtype, VOID_STAR));

   /* Prototype: int pop(SLtype type, VOID_STAR v);
    * Pops value from stack and assign it to object, whose address is @v@.
    */
   int (*cl_pop)_PROTO((SLtype, VOID_STAR));

   /* mul2, sign, etc... */
   int (*cl_unary_op_result_type)_PROTO((int, SLtype, SLtype *));
   int (*cl_unary_op)_PROTO((int, SLtype, VOID_STAR, SLuindex_Type, VOID_STAR));

#if 0
   int (*cl_arith_unary_op_result_type)_PROTO((int, SLtype, SLtype *));
   int (*cl_arith_unary_op)_PROTO((int, SLtype, VOID_STAR, SLuindex_Type, VOID_STAR));
#endif
   int (*cl_app_unary_op_result_type)_PROTO((int, SLtype, SLtype *));
   int (*cl_app_unary_op)_PROTO((int, SLtype, VOID_STAR, SLuindex_Type, VOID_STAR));

   /* If this function is non-NULL, it will be called for sin, cos, etc... */

   int (*cl_math_op)_PROTO((int, SLtype, VOID_STAR, SLuindex_Type, VOID_STAR));
   int (*cl_math_op_result_type)_PROTO((int, SLtype, SLtype *));

   SL_OOBinary_Type *cl_binary_ops;
   SL_Typecast_Type *cl_typecast_funs;

   void (*cl_byte_code_destroy)_PROTO((SLtype, VOID_STAR));
   void (*cl_user_destroy_fun)_PROTO((SLtype, VOID_STAR));
   int (*cl_init_array_object)_PROTO((SLtype, VOID_STAR));
   int (*cl_datatype_deref)_PROTO((SLtype));
   SLang_Struct_Type *cl_struct_def;
   int (*cl_dereference) _PROTO((SLtype, VOID_STAR));

   /* cl_a* functions deal with functions that have array addresses.  For
    * scalar and vector types, there is no difference.  However, for vector
    * types such as Complex, which is stored as double[2], the address is
    * the address of the first byte, and not the object itself.
    */
   int (*cl_acopy) (SLtype, VOID_STAR, VOID_STAR);
   int (*cl_apop) _PROTO((SLtype, VOID_STAR));
   int (*cl_apush) _PROTO((SLtype, VOID_STAR));
   int (*cl_push_literal) _PROTO((SLtype, VOID_STAR));
   void (*cl_adestroy)_PROTO((SLtype, VOID_STAR));
   int (*cl_push_intrinsic)_PROTO((SLtype, VOID_STAR));
   int (*cl_void_typecast)_PROTO((SLtype, VOID_STAR, SLuindex_Type, SLtype, VOID_STAR));

   int (*cl_anytype_typecast)_PROTO((SLtype, VOID_STAR, SLuindex_Type, SLtype, VOID_STAR));

   /* Array access functions */
   int (*cl_aput) (SLtype, unsigned int);
   int (*cl_aget) (SLtype, unsigned int);
   int (*cl_anew) (SLtype, unsigned int);

   /* length method */
   int (*cl_length) (SLtype, VOID_STAR, SLuindex_Type *);

   /* foreach */
   SLang_Foreach_Context_Type *(*cl_foreach_open) (SLtype, unsigned int);
   void (*cl_foreach_close) (SLtype, SLang_Foreach_Context_Type *);
   int (*cl_foreach) (SLtype, SLang_Foreach_Context_Type *);

   /* Structure access: get and put (assign to) fields */
   int (*cl_sput) (SLtype, SLFUTURE_CONST char *);
   int (*cl_sget) (SLtype, SLFUTURE_CONST char *);

   /* File I/O */
   int (*cl_fread) (SLtype, FILE *, VOID_STAR, unsigned int, unsigned int *);
   int (*cl_fwrite) (SLtype, FILE *, VOID_STAR, unsigned int, unsigned int *);
   int (*cl_fdread) (SLtype, int, VOID_STAR, unsigned int, unsigned int *);
   int (*cl_fdwrite) (SLtype, int, VOID_STAR, unsigned int, unsigned int *);

   int (*cl_to_bool) (SLtype, int *);

   int (*cl_cmp)(SLtype, VOID_STAR, VOID_STAR, int *);
   int (*cl_eqs)(SLtype, VOID_STAR, SLtype, VOID_STAR);

   /* required for any object that takes advantage of __tmp optimization */
   void (*cl_inc_ref)(SLtype, VOID_STAR, int);

   SL_OOBinary_Type *cl_void_binary_this;
   SL_OOBinary_Type *cl_this_binary_void;

   int is_container;
   int is_struct;
};
#define SLANG_CLASS_IS_SLSTRUCT(cl) ((cl)->is_struct != 0)

extern int _pSLregister_types (void);
extern SLang_Class_Type *_pSLclass_get_class (SLtype);
extern VOID_STAR _pSLclass_get_ptr_to_value (SLang_Class_Type *, SLang_Object_Type *);
extern void _pSLclass_type_mismatch_error (SLtype, SLtype);
extern int _pSLclass_init (void);
extern int _pSLclass_copy_class (SLtype, SLtype);

extern int (*_pSLclass_get_typecast (SLtype, SLtype, int))
(SLtype, VOID_STAR, SLuindex_Type,
 SLtype, VOID_STAR);

extern int (*_pSLclass_get_binary_fun (int, SLang_Class_Type *, SLang_Class_Type *, SLang_Class_Type **, int))
(int,
 SLtype, VOID_STAR, SLuindex_Type,
 SLtype, VOID_STAR, SLuindex_Type,
 VOID_STAR);

extern int (*_pSLclass_get_unary_fun (int, SLang_Class_Type *, SLang_Class_Type **, int))
(int, SLtype, VOID_STAR, SLuindex_Type, VOID_STAR);

#if 0
extern int _pSLclass_add_arith_unary_op (SLtype type,
					int (*f)(int,
						 SLtype, VOID_STAR, unsigned int,
						 VOID_STAR),
					int (*r)(int, SLtype, SLtype *));
#endif

extern int _pSLclass_get_unary_opcode (SLCONST char *name);
extern int _pSLclass_get_binary_opcode (SLCONST char *name);
extern int _pSLclass_is_same_obj (SLang_Object_Type *a, SLang_Object_Type *b);
extern int _pSLclass_obj_eqs (SLang_Object_Type *a, SLang_Object_Type *b);

extern int _pSLarith_register_types (void);
extern SLtype _pSLarith_Arith_Types [];

extern int _pSLang_ref_is_callable (SLang_Ref_Type *);
extern int _pSLang_is_arith_type (SLtype);
extern void _pSLang_set_arith_type (SLtype, unsigned char);
#if SLANG_OPTIMIZE_FOR_SPEED
extern SLclass_Type _pSLang_get_class_type (SLtype);
extern void _pSLang_set_class_info (SLtype, SLang_Class_Type *);
extern int _pSLarray_bin_op (SLang_Object_Type *, SLang_Object_Type *, int);
extern int _pSLarray1d_push_elem (SLang_Array_Type *at, SLindex_Type idx);
#endif
extern int _pSLarith_bin_op (SLang_Object_Type *, SLang_Object_Type *, int);

/* Does not perform any range checking.  It is up to the caller.
 * If *atp!=NULL, then it is an array of indices.
 */
extern int _pSLarray_pop_index (unsigned int num_elements, SLang_Array_Type **ind_atp, SLindex_Type *ind);

extern int _pSLarray_add_bin_op (SLtype);
extern int _pSLarray_push_elem_ref (void);
extern int _pSLang_push_array (SLang_Array_Type *, int);   /* NULL not allowed */

typedef struct
{
   SLCONST char *name;
   SLang_Name_Type *next;
   char name_type;

   int unary_op;
}
SLang_Arith_Unary_Type;
extern int _pSLadd_arith_unary_table (SLang_Arith_Unary_Type *tbl, SLFUTURE_CONST char *);

typedef struct
{
   SLCONST char *name;
   SLang_Name_Type *next;
   char name_type;

   int binary_op;
}
SLang_Arith_Binary_Type;
extern int _pSLadd_arith_binary_table (SLang_Arith_Binary_Type *tbl, SLFUTURE_CONST char *);

extern int _pSLang_do_binary_ab (int op, SLang_Object_Type *obja, SLang_Object_Type *objb);

extern int _pSLang_call_funptr (SLang_Name_Type *);
extern void _pSLset_double_format (SLCONST char *);
extern SLCONST char *_pSLget_double_format (void);
extern SLang_Name_Type *_pSLlocate_global_name (SLCONST char *);
extern SLang_Name_Type *_pSLlocate_name (SLCONST char *);

extern SLFUTURE_CONST char *_pSLdefines[];

#define SL_ERRNO_NOT_IMPLEMENTED	0x7FFF
extern int _pSLerrno_errno;
extern int _pSLerrno_init (void);
extern char *_SLcalloc (unsigned int, unsigned int);
extern char *_SLrecalloc (char *, unsigned int, unsigned int);

extern int _pSLstdio_fdopen (char *, int, char *);
extern void _pSLfclose_fdopen_fp (SLang_MMT_Type *);

extern void _pSLstruct_pop_args (int *);
extern void _pSLstruct_push_args (SLang_Array_Type *);

extern int _pSLang_init_sllist (void);
extern int _pSLlist_inline_list (void);

extern int _pSLarray_aput1 (unsigned int);
extern int _pSLarray_aput (void);
extern int _pSLarray_aget (void);
extern int _pSLarray_aget1 (unsigned int);
extern int _pSLarray_inline_implicit_array (void);
extern int _pSLarray_inline_implicit_arrayn (void);
extern int _pSLarray_inline_array (void);
extern int _pSLarray_wildcard_array (void);

extern int
_pSLarray_typecast (SLtype, VOID_STAR, SLuindex_Type,
		    SLtype, VOID_STAR, int);

extern int _pSLarray_aput_transfer_elem (SLang_Array_Type *, SLindex_Type *,
					VOID_STAR, size_t, int);
extern int _pSLarray_aget_transfer_elem (SLang_Array_Type *, SLindex_Type *,
					VOID_STAR, size_t, int);
extern void _pSLarray_free_array_elements (SLang_Class_Type *, VOID_STAR, SLuindex_Type);

extern SLang_Foreach_Context_Type *
_pSLarray_cl_foreach_open (SLtype, unsigned int);
extern void _pSLarray_cl_foreach_close (SLtype, SLang_Foreach_Context_Type *);
extern int _pSLarray_cl_foreach (SLtype, SLang_Foreach_Context_Type *);

extern int _pSLarray_matrix_multiply (void);
extern void (*_pSLang_Matrix_Multiply)(void);

extern int _pSLarray_next_index (SLindex_Type *, SLindex_Type *, unsigned int);

extern int _pSLarray_init_slarray (void);

extern int _pSLassoc_aput (SLtype, unsigned int);
extern int _pSLassoc_aget (SLtype, unsigned int);
extern int _pSLassoc_inc_value (unsigned int, int val);

extern int _pSLcompile_push_context (SLang_Load_Type *);
extern int _pSLcompile_pop_context (void);
extern int _pSLang_Auto_Declare_Globals;
extern int _pSLang_Load_File_Verbose;

typedef struct _pSLtoken_String_List_Type
{
   struct _pSLtoken_String_List_Type *next;
   unsigned int len;
   char buf[1];			       /* rest of bytes follow */
}
_pSLtoken_String_List_Type;

typedef unsigned char _pSLtok_Type;
typedef struct
{
   _pSLtok_Type type;			       /* [b]string_token */
   unsigned int num;

   _pSLtoken_String_List_Type *list;
   union
     {
	SLFUTURE_CONST char *s_val;		       /* (SLstring) concatenated strings */
	SLang_BString_Type *b_val;     /* concatenated bstrings */
     } v;
   unsigned long hash;		       /* for v.s_val */
   unsigned int len;		       /* length of v.?_val */
}
_pSLang_Multiline_String_Type;

typedef struct _pSLang_Token_Type
{
   union
     {
	long long_val;
	unsigned long ulong_val;
#ifdef HAVE_LONG_LONG
	long long llong_val;
	unsigned long long ullong_val;
#endif
	/* Note that it is not wise to put a double field in the union because
	 * when reading a preparsed file, I want to keep the user-specified
	 * form to preserve the precision.  Using a union member would mean
	 * converting the double to a string when creating a preparsed file.
	 * We can avoid the issues associated with this by just storing
	 * floating point values as strings.
	 */
	SLFUTURE_CONST char *s_val;		       /* Used for IDENT_TOKEN, DOUBLE_TOKEN, etc...  */

	SLang_BString_Type *b_val;
	_pSLang_Multiline_String_Type *multistring_val;
     } v;
   void (*free_val_func)(struct _pSLang_Token_Type *);
   unsigned int num_refs;
   unsigned long hash;		       /* hash for slstring, and length for _BSTRING */
#define SLTOKEN_IS_NEGATIVE		0x001
#define SLTOKEN_OVERFLOW_CHECKED	0x002
#define SLTOKEN_IS_HEX			0x004
#define SLTOKEN_IS_BINARY		0x008
#define SLTOKEN_TYPE_INTEGER		0x100
#define SLTOKEN_TYPE_FLOAT		0x200
#define SLTOKEN_TYPE_NUMBER		(SLTOKEN_TYPE_INTEGER|SLTOKEN_TYPE_FLOAT)
   int flags;

#if SLANG_HAS_DEBUG_CODE
   int line_number;
#endif
   struct _pSLang_Token_Type *next;     /* used for token lists */
   _pSLtok_Type type;
}
_pSLang_Token_Type;

/* return token type or EOF_TOKEN upon error */
extern _pSLtok_Type _pSLtoken_init_slstring_token (_pSLang_Token_Type *, _pSLtok_Type,
						   SLFUTURE_CONST char *, unsigned int);

extern void _pSLcompile (_pSLang_Token_Type *);
extern void (*_pSLcompile_ptr)(_pSLang_Token_Type *);

/* slmisc.c */
extern char *_pSLskip_whitespace (SLCONST char *s);

/* slospath.c */
extern char *_pSLpath_find_file (SLFUTURE_CONST char *, int);   /* slstring returned */

/* Read but do not set this variable. */
extern volatile int _pSLang_Error;

extern int _pSLutf8_mode;
extern int _pSLinterp_UTF8_Mode;	       /* non-zero for interpreter */
extern int _pSLtt_UTF8_Mode;

extern SLuchar_Type *_pSLinterp_decode_wchar (SLuchar_Type *u,
					     SLuchar_Type *umax,
					     SLwchar_Type *chp);

extern SLuchar_Type *_pSLinterp_encode_wchar (SLwchar_Type wch,
					     SLuchar_Type *buf,
					     unsigned int *encoded_lenp);

/* *** TOKENS *** */

/* Note that that tokens corresponding to ^J, ^M, and ^Z should not be used.
 * This is because a file that contains any of these characters will
 * have an OS dependent interpretation, e.g., ^Z is EOF on MSDOS.
 */

/* Special tokens */
#define ILLEGAL_TOKEN	0x00	       /* no token has this value */
#define EOF_TOKEN	0x01
#define RPN_TOKEN	0x02
#define NL_TOKEN	0x03
#define NOP_TOKEN	0x05
#define FARG_TOKEN	0x06
#define TMP_TOKEN	0x07
#define QUALIFIER_TOKEN	0x08

#define RESERVED1_TOKEN	0x0A	       /* \n */
#define RESERVED2_TOKEN	0x0D	       /* \r */

/* Literal tokens */
#define CHAR_TOKEN	0x10
#define UCHAR_TOKEN	0x11
#define SHORT_TOKEN	0x12
#define USHORT_TOKEN	0x13
#define INT_TOKEN	0x14
#define UINT_TOKEN	0x15
#define LONG_TOKEN	0x16
#define ULONG_TOKEN	0x17
#define IS_INTEGER_TOKEN(x) ((x >= CHAR_TOKEN) && (x <= ULONG_TOKEN))
#define FLOAT_TOKEN	0x18
#define DOUBLE_TOKEN	0x19
#define RESERVED3_TOKEN	0x1A	       /* ^Z */
#define COMPLEX_TOKEN	0x1B
#define STRING_TOKEN    0x1C
#define BSTRING_TOKEN	0x1D
#define _BSTRING_TOKEN	0x1E	       /* byte-compiled BSTRING */
#define STRING_DOLLAR_TOKEN	0x1F

/* Tokens that can be LVALUES */
#define IDENT_TOKEN	0x20
#define ARRAY_TOKEN	0x21
#define DOT_TOKEN	0x22
#define IS_LVALUE_TOKEN (((t) <= DOT_TOKEN) && ((t) >= IDENT_TOKEN))
#define DOT_METHOD_CALL_TOKEN		0x23

#define ESC_STRING_TOKEN	0x24   /* only appears in slc file */
#define ESC_BSTRING_TOKEN	0x25   /* only appears in slc file */

/* Flags for struct fields */
#define STATIC_TOKEN	0x26
#define READONLY_TOKEN	0x27
#define PRIVATE_TOKEN	0x28
#define PUBLIC_TOKEN	0x29

/* Punctuation tokens */
#define OBRACKET_TOKEN	0x2a
#define CBRACKET_TOKEN	0x2b
#define OPAREN_TOKEN	0x2c
#define CPAREN_TOKEN	0x2d
#define OBRACE_TOKEN	0x2e
#define CBRACE_TOKEN	0x2f

#define COMMA_TOKEN	0x31
#define SEMICOLON_TOKEN	0x32
#define COLON_TOKEN	0x33
#define NAMESPACE_TOKEN	0x34
#define QUESTION_TOKEN	0x35

/* Operators */
/* The order here must match the order in the Binop_Level table in slparse.c */
#define FIRST_BINARY_OP	0x36
#define SC_AND_TOKEN	0x36
#define SC_OR_TOKEN	0x37
#define POW_TOKEN	0x38
#define ADD_TOKEN	0x39
#define SUB_TOKEN	0x3a
#define TIMES_TOKEN	0x3b
#define DIV_TOKEN	0x3c

#define LT_TOKEN	0x3d
#define LE_TOKEN	0x3e
#define GT_TOKEN	0x3f
#define GE_TOKEN	0x40
#define EQ_TOKEN	0x41
#define NE_TOKEN	0x42
#define IS_COMPARE_OP(x) (((x) >= LT_TOKEN) && ((x) <= NE_TOKEN))

#define AND_TOKEN	0x43
#define OR_TOKEN	0x44
#define MOD_TOKEN	0x45
#define BAND_TOKEN	0x46
#define SHL_TOKEN	0x47
#define SHR_TOKEN	0x48
#define BXOR_TOKEN	0x49
#define BOR_TOKEN	0x4a
#define POUND_TOKEN	0x4b	       /* matrix multiplication */
#define LAST_BINARY_OP	 0x4b
#define IS_BINARY_OP(t)	 ((t >= FIRST_BINARY_OP) && (t <= LAST_BINARY_OP))

/* unary tokens -- but not all of them (see grammar) */
#define DEREF_TOKEN	 0x4d
#define NOT_TOKEN	 0x4e
#define BNOT_TOKEN	 0x4f

#define IS_INTERNAL_FUNC(t)	((t >= 0x50) && (t <= 0x52))
#define POP_TOKEN	 0x50
#define CHS_TOKEN	 0x51
#define EXCH_TOKEN	 0x52

#define LLONG_TOKEN	0x53
#define ULLONG_TOKEN	0x54
#define LDOUBLE_TOKEN	0x55

/* Assignment tokens.  Note: these must appear with sequential values.
 * The order here must match the specific lvalue assignments below.
 * These tokens are used by rpn routines in slang.c.  slparse.c maps them
 * onto the specific lvalue tokens while parsing infix.
 * Also the assignment SLANG_BCST_ assumes this order
 */
#define ASSIGN_TOKEN		0x57
#define PLUSEQS_TOKEN	 	0x58
#define MINUSEQS_TOKEN		0x59
#define TIMESEQS_TOKEN		0x5A
#define DIVEQS_TOKEN		0x5B
#define BOREQS_TOKEN		0x5C
#define BANDEQS_TOKEN		0x5D
#define PLUSPLUS_TOKEN		0x5E
#define POST_PLUSPLUS_TOKEN	0x5F
#define MINUSMINUS_TOKEN	0x60
#define POST_MINUSMINUS_TOKEN	0x61

/* Directives */
#define IFNOT_TOKEN	0x62
#define IF_TOKEN	0x63
#define ELSE_TOKEN	0x64
#define FOREVER_TOKEN	0x65
#define WHILE_TOKEN	0x66
#define FOR_TOKEN	0x67
#define _FOR_TOKEN	0x68
#define LOOP_TOKEN	0x69
#define SWITCH_TOKEN	0x6A
#define DOWHILE_TOKEN	0x6B
#define ANDELSE_TOKEN	0x6C
#define ORELSE_TOKEN	0x6D
#define ERRBLK_TOKEN	0x6E
#define EXITBLK_TOKEN	0x6F
/* These must be sequential */
#define USRBLK0_TOKEN	0x70
#define USRBLK1_TOKEN	0x71
#define USRBLK2_TOKEN	0x72
#define USRBLK3_TOKEN	0x73
#define USRBLK4_TOKEN	0x74

#define CONT_TOKEN	0x75
#define BREAK_TOKEN	0x76
#define RETURN_TOKEN	0x77

#define CASE_TOKEN	0x78
#define DEFINE_TOKEN	0x79
#define DO_TOKEN	0x7a
#define VARIABLE_TOKEN	0x7b
#define GVARIABLE_TOKEN	0x7c
#define _REF_TOKEN	0x7d
#define PUSH_TOKEN	0x7e
#define STRUCT_TOKEN	0x7f
#define TYPEDEF_TOKEN	0x80
#define NOTELSE_TOKEN	0x81
#define DEFINE_STATIC_TOKEN	0x82
#define FOREACH_TOKEN	0x83
#define USING_TOKEN	0x84
#define DEFINE_PRIVATE_TOKEN	0x85
#define DEFINE_PUBLIC_TOKEN	0x86

#define TRY_TOKEN	0x87
#define CATCH_TOKEN	0x88
#define THROW_TOKEN	0x89
#define FINALLY_TOKEN	0x8a

#define BREAK_N_TOKEN	0x8b
#define CONT_N_TOKEN	0x8c
#define THEN_TOKEN	0x8d
#define STRUCT_WITH_ASSIGN_TOKEN	0x8e
#define FOREACH_EARGS_TOKEN	0x8f

/* Note: the order here must match the order of the generic assignment tokens.
 * Also, the first token of each group must be the ?_ASSIGN_TOKEN.
 * slparse.c exploits this order, as well as slang.h.
 */
#define FIRST_ASSIGN_TOKEN		0x90
#define _STRUCT_ASSIGN_TOKEN		0x90
#define _STRUCT_PLUSEQS_TOKEN		0x91
#define _STRUCT_MINUSEQS_TOKEN		0x92
#define _STRUCT_TIMESEQS_TOKEN		0x93
#define _STRUCT_DIVEQS_TOKEN		0x94
#define _STRUCT_BOREQS_TOKEN		0x95
#define _STRUCT_BANDEQS_TOKEN		0x96
#define _STRUCT_PLUSPLUS_TOKEN		0x97
#define _STRUCT_POST_PLUSPLUS_TOKEN	0x98
#define _STRUCT_MINUSMINUS_TOKEN	0x99
#define _STRUCT_POST_MINUSMINUS_TOKEN	0x9A

#define _ARRAY_ASSIGN_TOKEN		0xA0
#define _ARRAY_PLUSEQS_TOKEN		0xA1
#define _ARRAY_MINUSEQS_TOKEN		0xA2
#define _ARRAY_TIMESEQS_TOKEN		0xA3
#define _ARRAY_DIVEQS_TOKEN		0xA4
#define _ARRAY_BOREQS_TOKEN		0xA5
#define _ARRAY_BANDEQS_TOKEN		0xA6
#define _ARRAY_PLUSPLUS_TOKEN		0xA7
#define _ARRAY_POST_PLUSPLUS_TOKEN	0xA8
#define _ARRAY_MINUSMINUS_TOKEN		0xA9
#define _ARRAY_POST_MINUSMINUS_TOKEN	0xAA

#define _SCALAR_ASSIGN_TOKEN		0xB0
#define _SCALAR_PLUSEQS_TOKEN		0xB1
#define _SCALAR_MINUSEQS_TOKEN		0xB2
#define _SCALAR_TIMESEQS_TOKEN		0xB3
#define _SCALAR_DIVEQS_TOKEN		0xB4
#define _SCALAR_BOREQS_TOKEN		0xB5
#define _SCALAR_BANDEQS_TOKEN		0xB6
#define _SCALAR_PLUSPLUS_TOKEN		0xB7
#define _SCALAR_POST_PLUSPLUS_TOKEN	0xB8
#define _SCALAR_MINUSMINUS_TOKEN	0xB9
#define _SCALAR_POST_MINUSMINUS_TOKEN	0xBA

#define _DEREF_ASSIGN_TOKEN		0xC0
#define _DEREF_PLUSEQS_TOKEN		0xC1
#define _DEREF_MINUSEQS_TOKEN		0xC2
#define _DEREF_TIMESEQS_TOKEN		0xC3
#define _DEREF_DIVEQS_TOKEN		0xC4
#define _DEREF_BOREQS_TOKEN		0xC5
#define _DEREF_BANDEQS_TOKEN		0xC6
#define _DEREF_PLUSPLUS_TOKEN		0xC7
#define _DEREF_POST_PLUSPLUS_TOKEN	0xC8
#define _DEREF_MINUSMINUS_TOKEN		0xC9
#define _DEREF_POST_MINUSMINUS_TOKEN	0xCA

#define LAST_ASSIGN_TOKEN		0xCA
#define IS_ASSIGN_TOKEN(t) (((t)>=FIRST_ASSIGN_TOKEN)&&((t)<=LAST_ASSIGN_TOKEN))

#define _DEREF_OBSOLETE_FUNCALL_TOKEN	0xCE
#define _DEREF_FUNCALL_TOKEN		0xCF

#define LOOP_THEN_TOKEN			0xD0
/* #define LOOP_ELSE_TOKEN			0xD1 */

#define _COMPARE_TOKEN			0xD8   /* (a < b < c ...) */
#define _ARRAY_ELEM_REF_TOKEN		0xD9   /* &X[i] */
#define _STRUCT_FIELD_REF_TOKEN		0xDA   /* &S.a */

#define _INLINE_ARRAY_TOKEN		0xE0
#define _INLINE_IMPLICIT_ARRAY_TOKEN	0xE1   /* [a:b:c] */
#define _NULL_TOKEN			0xE2
#define _INLINE_WILDCARD_ARRAY_TOKEN	0xE3
#define _INLINE_LIST_TOKEN		0xE4
#define _INLINE_IMPLICIT_ARRAYN_TOKEN	0xE5   /* [a:b:#n] */

#define ESC_STRING_DOLLAR_TOKEN		0xF0   /* appears only in .slc files */
#define MULTI_STRING_TOKEN		0xF1

#define BOS_TOKEN			0xFA
#define EOS_TOKEN			0xFB
#define LINE_NUM_TOKEN			0xFC
#define ARG_TOKEN	 		0xFD
#define EARG_TOKEN	 		0xFE
#define NO_OP_LITERAL			0xFF

typedef struct
{
   /* sltoken.c */
   /* SLang_eval_object */
   SLang_Load_Type *llt;
   SLprep_Type *this_slpp;
   /* prep_get_char() */
   char *input_line;
   char cchar;
   /* get_token() */
   int want_nl_token;

   /* slparse.c */
   _pSLang_Token_Type ctok;
   int block_depth;
   int assignment_expression;

   /* slang.c : SLcompile() */
   _pSLang_Token_Type save_token;
   _pSLang_Token_Type next_token;
   void (*slcompile_ptr)(_pSLang_Token_Type *);
}
_pSLEval_Context;

extern int _pSLget_token (_pSLang_Token_Type *);
extern void _pSLparse_error (int, SLCONST char *, _pSLang_Token_Type *, int);
extern void _pSLparse_start (SLang_Load_Type *);
extern int _pSLget_rpn_token (_pSLang_Token_Type *);
extern void _pSLcompile_byte_compiled (void);

extern int (*_pSLprep_eval_hook) (SLFUTURE_CONST char *);

extern int _pSLsecure_issetugid (void);
extern char *_pSLsecure_getenv (SLCONST char *);

/* Error Handling */
extern int _pSLang_init_exceptions (void);
extern int _pSLerr_init (void);
extern void _pSLerr_deinit (void);
extern int _pSLerr_suspend_messages (void);
extern int _pSLerr_resume_messages (void);
extern int _pSLerr_traceback_msg (SLFUTURE_CONST char *, ...) SLATTRIBUTE_PRINTF(1,2);
extern void _pSLerr_dump_msg (SLFUTURE_CONST char *, ...) SLATTRIBUTE_PRINTF(1,2);
extern void _pSLerr_clear_error (int);
extern void _pSLang_verror (int, SLCONST char *, ...) SLATTRIBUTE_PRINTF(2,3);
extern int _pSLsnprintf (char *, unsigned int, SLFUTURE_CONST char *, ...) SLATTRIBUTE_PRINTF(3,4);

typedef struct _pSLerr_Error_Queue_Type _pSLerr_Error_Queue_Type;
extern _pSLerr_Error_Queue_Type *_pSLerr_new_error_queue (int);
extern int _pSLerr_set_error_queue (_pSLerr_Error_Queue_Type *);
extern void _pSLerr_delete_error_queue (_pSLerr_Error_Queue_Type *);

extern void _pSLerr_print_message_queue (void);

extern int _pSLerr_set_line_info (SLFUTURE_CONST char *, int, SLFUTURE_CONST char *);
extern int _pSLang_pop_error_context (int);
extern int _pSLang_push_error_context (void);
extern void (*_pSLinterpreter_Error_Hook)(int);
extern int _pSLerr_init_interp_exceptions (void);
extern int _pSLerr_get_last_error (void);
extern int _pSLerr_pop_exception (int *);
extern void _pSLerr_free_queued_messages (void);

#define _SLERR_MSG_ERROR	1
#define _SLERR_MSG_WARNING	2
#define _SLERR_MSG_TRACEBACK	3
extern char *_pSLerr_get_error_from_queue (_pSLerr_Error_Queue_Type *, int);

extern int _pSLerr_throw (void);
extern int (*_pSLerr_New_Exception_Hook)(SLFUTURE_CONST char *name, SLFUTURE_CONST char *desc, int error_code);

#if SLANG_HAS_DEBUGGER_SUPPORT
typedef struct
{
   char **locals;
   unsigned int nlocals;
   SLCONST char *file;
   unsigned int line;
   SLCONST char *function;
   SLCONST char *ns;
}
_pSLang_Frame_Info_Type;

extern int _pSLang_get_frame_fun_info (int depth, _pSLang_Frame_Info_Type *info);
extern int _pSLang_set_frame_variable (int depth, char *name);
extern int _pSLang_get_frame_variable (int depth, char *name);
extern void _pSLang_use_frame_namespace (int depth);
extern int _pSLang_get_frame_depth (void);
#endif

#if SLANG_HAS_FLOAT
extern int _pSLmath_isnan (double x);
extern int _pSLmath_isinf (double x);
extern double _pSLang_NaN;
extern double _pSLang_Inf;

# ifdef HAVE_LOG1P
#  define LOG1P_FUNC log1p
# else
extern double _pSLmath_log1p (double);
#  define LOG1P_FUNC _pSLmath_log1p
# endif

# ifdef HAVE_EXPM1
#  define EXPM1_FUNC expm1
# else
extern double _pSLmath_expm1 (double);
#  define EXPM1_FUNC _pSLmath_expm1
# endif

# if SLANG_HAS_COMPLEX
extern double *_pSLcomplex_expm1 (double *f, double *z);
extern double *_pSLcomplex_log1p (double *f, double *z);
# endif
#endif

#if SLANG_HAS_SIGNALS
extern void _pSLang_signal_interrupt (void);
extern int _pSLsig_block_and_call (int (*)(VOID_STAR), VOID_STAR);
extern int _pSLsig_handle_signals (void);
extern int _pSLang_check_signals_hook (VOID_STAR);
#else
#define _pSLsig_block_and_call(f,v) f(v)
#endif

#undef _INLINE_
#if defined(__GNUC__) && SLANG_USE_INLINE_CODE
# define _INLINE_ __inline__
#else
# define _INLINE_
#endif

#if SLANG_OPTIMIZE_FOR_SPEED && defined(__GNUC__) && (__GNUC__ >= 3)
# define IF_LIKELY(x) if (__builtin_expect((x),1))
# define IF_UNLIKELY(x) if (__builtin_expect ((x), 0))
#else
# define IF_LIKELY(x) if (x)
# define IF_UNLIKELY(x) if (x)
#endif

/* This is a macro that permits:
 *
 *   extern fun (void **addr);
 *   double x;
 *   fun ((void **) &x);   <--- warning
 *   fun (VOID_STAR_STAR(&x));
 *
 * Otherwise a type-punning warning could result due to strict-aliasing.
 */
#define VOID_STAR_STAR(addr) (VOID_STAR *)(VOID_STAR)(addr)

#endif				       /* PRIVATE_SLANG_H_ */
