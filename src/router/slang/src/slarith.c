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

#include "slinclud.h"

#include <math.h>
#include <limits.h>

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#if SLANG_HAS_FLOAT
# include <float.h>
# ifdef HAVE_FLOATINGPOINT_H
#  include <floatingpoint.h>
# endif

# ifdef HAVE_IEEEFP_H
#  include <ieeefp.h>
# endif

# ifdef HAVE_NAN_H
#  include <nan.h>
# endif
#endif

#include <ctype.h>

#include "slang.h"
#include "_slang.h"

#if SLANG_HAS_FLOAT
double _pSLang_NaN;
double _pSLang_Inf;
#endif

/*
 * This file defines binary and unary operations on all integer types.
 * Supported types include:
 *
 *    SLANG_CHAR_TYPE     (char)
 *    SLANG_SHORT_TYPE    (short)
 *    SLANG_INT_TYPE      (int)
 *    SLANG_LONG_TYPE     (long)
 *    SLANG_FLOAT_TYPE    (float)
 *    SLANG_DOUBLE_TYPE   (double)
 *
 * as well as unsigned types.  The result-type of an arithmentic operation
 * will depend upon the data types involved.  I am going to distinguish
 * between the boolean operations such as `and' and `or' from the arithmetic
 * operations such as `plus'.  Since the result of a boolean operation is
 * either 1 or 0, a boolean result will be represented by SLANG_CHAR_TYPE.
 * Ordinarily I would use an integer but for arrays it makes more sense to
 * use a character data type.
 *
 * So, the following will be assumed (`+' is any arithmetic operator)
 *
 *    char + char = int
 *    char|short + short = int
 *    char|short|int + int = int
 *    char|short|int|long + long = long
 *    char|short|int|long|float + float = float
 *    char|short|int|long|float|double + double = double
 *
 * In the actual implementation, a brute force approach is avoided.  Such
 * an approach would mean defining different functions for all possible
 * combinations of types.  Including the unsigned types, and not including
 * the complex number type, there are 10 arithmetic types and 10*10=100
 * different combinations of types.  Clearly this would be too much.
 *
 * One approach would be to define binary functions only between operands of
 * the same type and then convert types as appropriate.  This would require
 * just 6 such functions (int, uint, long, ulong, float, double).
 * However, many conversion functions are going to be required, particularly
 * since we are going to allow typecasting from one arithmetic to another.
 * Since the bit pattern of signed and unsigned types are the same, and only
 * the interpretation differs, there will be no functions to convert between
 * signed and unsigned forms of a given type.
 */

#ifdef HAVE_LONG_LONG
# define MAX_SLARITH_INT_TYPE	SLANG_ULLONG_TYPE
#else
# define MAX_SLARITH_INT_TYPE	SLANG_ULONG_TYPE
#endif

#define MAX_SLARITH_TYPE	SLANG_LDOUBLE_TYPE

#define MAX_ARITHMETIC_TYPES	(MAX_SLARITH_TYPE-SLANG_CHAR_TYPE+1)
#define TYPE_TO_TABLE_INDEX(t)	((t)-SLANG_CHAR_TYPE)
#define TABLE_INDEX_TO_TYPE(i)  ((i)+SLANG_CHAR_TYPE)

#define IS_INTEGER_TYPE(t) \
   (((t) >= SLANG_CHAR_TYPE) && ((t) <= MAX_SLARITH_INT_TYPE))

/* This table contains the types that have been implemented here */
SLtype _pSLarith_Arith_Types [MAX_ARITHMETIC_TYPES+1] =
{
   SLANG_CHAR_TYPE,
   SLANG_UCHAR_TYPE,
   SLANG_SHORT_TYPE,
   SLANG_USHORT_TYPE,
   SLANG_INT_TYPE,
   SLANG_UINT_TYPE,
   SLANG_LONG_TYPE,
   SLANG_ULONG_TYPE,
#ifdef HAVE_LONG_LONG
     SLANG_LLONG_TYPE, SLANG_ULLONG_TYPE,
#endif
#ifdef SLANG_HAS_FLOAT
   SLANG_FLOAT_TYPE,
   SLANG_DOUBLE_TYPE,
# ifdef HAVE_LONG_DOUBLE
     SLANG_LDOUBLE_TYPE,
# endif
#endif
   0
};

static SLtype Alias_Map [MAX_ARITHMETIC_TYPES];

/* Here are a bunch of functions to convert from one type to another.  To
 * facilitate the process, macros will be used.
 */

#define DEFUN_1(f,from_type,to_type) \
static void f (to_type *y, from_type *x, unsigned int n) \
{ \
   unsigned int i; \
   for (i = 0; i < n; i++) y[i] = (to_type) x[i]; \
}

#define DEFUN_2(f,from_type,to_type,copy_fun) \
static VOID_STAR f (VOID_STAR xp, unsigned int n) \
{ \
   from_type *x; \
   to_type *y; \
   x = (from_type *) xp; \
   if (NULL == (y = (to_type *) _SLcalloc (n, sizeof (to_type)))) return NULL; \
   copy_fun (y, x, n); \
   return (VOID_STAR) y; \
}
typedef VOID_STAR (*Convert_Fun_Type)(VOID_STAR, unsigned int);

#if SLANG_HAS_FLOAT
#define TO_DOUBLE_FUN(name,type) \
   static double name (VOID_STAR x) { return (double) *(type *) x; }

typedef SLCONST struct
{
   unsigned int sizeof_type;
   double (*to_double_fun)(VOID_STAR);
}
To_Double_Fun_Table_Type;

#endif

/* Each element of the matrix determines how the row maps onto the column.
 * That is, let the matrix be B_ij.  Where the i,j indices refer to
 * precedence of the type.  Then,
 * B_ij->copy_function copies type i to type j.  Similarly,
 * B_ij->convert_function mallocs a new array of type j and copies i to it.
 *
 * Since types are always converted to higher levels of precedence for binary
 * operations, many of the elements are NULL.
 *
 * Is the idea clear?
 */
typedef struct
{
   FVOID_STAR copy_function;
   Convert_Fun_Type convert_function;
}
Binary_Matrix_Type;

#include "slarith2.inc"

#if SLANG_HAS_FLOAT
SLang_To_Double_Fun_Type
SLarith_get_to_double_fun (SLtype type, unsigned int *sizeof_type)
{
   To_Double_Fun_Table_Type *t;

   if ((type < SLANG_CHAR_TYPE) || (type > MAX_SLARITH_TYPE))
     return NULL;

   t = To_Double_Fun_Table + (type - SLANG_CHAR_TYPE);
   if ((sizeof_type != NULL)
       && (t->to_double_fun != NULL))
     *sizeof_type = t->sizeof_type;

   return t->to_double_fun;
}
#endif				       /* SLANG_HAS_FLOAT */

#define GENERIC_BINARY_FUNCTION int_int_bin_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE int
#define POW_FUNCTION(a,b) pow((double)(a),(double)(b))
#define POW_RESULT_TYPE double
#define ABS_FUNCTION abs
#define MOD_FUNCTION(a,b) ((a) % (b))
#define TRAP_DIV_ZERO	1
#define GENERIC_UNARY_FUNCTION int_unary_op
#define GENERIC_ARITH_UNARY_FUNCTION int_arith_unary_op
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#if SLANG_OPTIMIZE_FOR_SPEED
# define SCALAR_BINARY_FUNCTION int_int_scalar_bin_op
#endif
#define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_int_obj(SLANG_INT_TYPE,(x))
#define PUSH_POW_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE, (x))
#define CMP_FUNCTION int_cmp_function
#include "slarith.inc"

#define GENERIC_BINARY_FUNCTION uint_uint_bin_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE unsigned int
#define GENERIC_TYPE_IS_UNSIGNED
#define POW_FUNCTION(a,b) pow((double)(a),(double)(b))
#define POW_RESULT_TYPE double
#define MOD_FUNCTION(a,b) ((a) % (b))
#define TRAP_DIV_ZERO	1
#define GENERIC_UNARY_FUNCTION uint_unary_op
#define GENERIC_ARITH_UNARY_FUNCTION uint_arith_unary_op
#define ABS_FUNCTION(a) (a)
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : 0)
#if SLANG_OPTIMIZE_FOR_SPEED
# define SCALAR_BINARY_FUNCTION uint_uint_scalar_bin_op
#endif
#define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_int_obj(SLANG_UINT_TYPE,(int)(x))
#define PUSH_POW_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE, (x))
#define CMP_FUNCTION uint_cmp_function
#define TO_BINARY_FUNCTION uint_to_binary
#include "slarith.inc"

#if LONG_IS_NOT_INT
#define GENERIC_BINARY_FUNCTION long_long_bin_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE long
#define POW_FUNCTION(a,b) pow((double)(a),(double)(b))
#define POW_RESULT_TYPE double
#define MOD_FUNCTION(a,b) ((a) % (b))
#define TRAP_DIV_ZERO	1
#define GENERIC_UNARY_FUNCTION long_unary_op
#define GENERIC_ARITH_UNARY_FUNCTION long_arith_unary_op
#define ABS_FUNCTION(a) (((a) >= 0) ? (a) : -(a))
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#if SLANG_OPTIMIZE_FOR_SPEED
# define SCALAR_BINARY_FUNCTION long_long_scalar_bin_op
#endif
#define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_long_obj(SLANG_LONG_TYPE,(x))
#define PUSH_POW_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE, (x))
#define CMP_FUNCTION long_cmp_function
#include "slarith.inc"

#define GENERIC_BINARY_FUNCTION ulong_ulong_bin_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE unsigned long
#define GENERIC_TYPE_IS_UNSIGNED
#define POW_FUNCTION(a,b) pow((double)(a),(double)(b))
#define POW_RESULT_TYPE double
#define MOD_FUNCTION(a,b) ((a) % (b))
#define TRAP_DIV_ZERO	1
#define GENERIC_UNARY_FUNCTION ulong_unary_op
#define GENERIC_ARITH_UNARY_FUNCTION ulong_arith_unary_op
#define ABS_FUNCTION(a) (a)
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : 0)
#if SLANG_OPTIMIZE_FOR_SPEED
# define SCALAR_BINARY_FUNCTION ulong_ulong_scalar_bin_op
#endif
#define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_long_obj(SLANG_ULONG_TYPE,(long)(x))
#define PUSH_POW_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE, (x))
#define CMP_FUNCTION ulong_cmp_function
#define TO_BINARY_FUNCTION ulong_to_binary
#include "slarith.inc"
#else
#define long_long_bin_op	int_int_bin_op
#define ulong_ulong_bin_op	uint_uint_bin_op
#define long_unary_op		int_unary_op
#define ulong_unary_op		uint_unary_op
#define long_cmp_function	int_cmp_function
#define ulong_cmp_function	uint_cmp_function
#define ulong_to_binary		uint_to_binary
#endif				       /* LONG_IS_NOT_INT */

#ifdef HAVE_LONG_LONG
# if LLONG_IS_NOT_LONG
#  define GENERIC_BINARY_FUNCTION llong_llong_bin_op
#  define GENERIC_BIT_OPERATIONS
#  define GENERIC_TYPE long long
#  define POW_FUNCTION(a,b) pow((double)(a),(double)(b))
#  define POW_RESULT_TYPE double
#  define MOD_FUNCTION(a,b) ((a) % (b))
#  define TRAP_DIV_ZERO	1
#  define GENERIC_UNARY_FUNCTION llong_unary_op
#  define GENERIC_ARITH_UNARY_FUNCTION llong_arith_unary_op
#  define ABS_FUNCTION(a) (((a) >= 0) ? (a) : -(a))
#  define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#  if SLANG_OPTIMIZE_FOR_SPEED
#   define SCALAR_BINARY_FUNCTION llong_llong_scalar_bin_op
#  endif
#  define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_llong_obj(SLANG_LLONG_TYPE,(x))
#  define PUSH_POW_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE, (x))
#  define CMP_FUNCTION llong_cmp_function
#  include "slarith.inc"

#  define GENERIC_BINARY_FUNCTION ullong_ullong_bin_op
#  define GENERIC_BIT_OPERATIONS
#  define GENERIC_TYPE unsigned long long
#  define GENERIC_TYPE_IS_UNSIGNED
#  define POW_FUNCTION(a,b) pow((double)(a),(double)(b))
#  define POW_RESULT_TYPE double
#  define MOD_FUNCTION(a,b) ((a) % (b))
#  define TRAP_DIV_ZERO	1
#  define GENERIC_UNARY_FUNCTION ullong_unary_op
#  define GENERIC_ARITH_UNARY_FUNCTION ullong_arith_unary_op
#  define ABS_FUNCTION(a) (a)
#  define SIGN_FUNCTION(x) (((x) > 0) ? 1 : 0)
#  if SLANG_OPTIMIZE_FOR_SPEED
#   define SCALAR_BINARY_FUNCTION ullong_ullong_scalar_bin_op
#  endif
#  define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_llong_obj(SLANG_ULLONG_TYPE,(long long)(x))
#  define PUSH_POW_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE, (x))
#  define CMP_FUNCTION ullong_cmp_function
#  define TO_BINARY_FUNCTION ullong_to_binary
#  include "slarith.inc"
# else
#  define llong_llong_bin_op long_long_bin_op
#  define ullong_ullong_bin_op ulong_ulong_bin_op
#  define llong_llong_scalar_bin_op long_long_scalar_bin_op
#  define ullong_ullong_scalar_bin_op ulong_ulong_scalar_bin_op
#  define ullong_to_binary ulong_to_binary
# endif				       /* LLONG_IS_NOT_LONG */
#endif				       /* HAVE_LONG_LONG */

#if SLANG_HAS_FLOAT
#define GENERIC_BINARY_FUNCTION float_float_bin_op
#define GENERIC_TYPE float
#define POW_FUNCTION(a,b) (float)pow((double)(a),(double)(b))
#define POW_RESULT_TYPE float
#define MOD_FUNCTION(a,b) (float)fmod((a),(b))
#define TRAP_DIV_ZERO	0
#define GENERIC_UNARY_FUNCTION float_unary_op
#define GENERIC_ARITH_UNARY_FUNCTION float_arith_unary_op
#define ABS_FUNCTION(a) (float)fabs((double) a)
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#if SLANG_OPTIMIZE_FOR_SPEED
# define SCALAR_BINARY_FUNCTION float_float_scalar_bin_op
#endif
#define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_float_obj(SLANG_FLOAT_TYPE,(x))
#define PUSH_POW_OBJ_FUN(x) SLclass_push_float_obj(SLANG_FLOAT_TYPE, (x))
#define CMP_FUNCTION float_cmp_function
#include "slarith.inc"

#define GENERIC_BINARY_FUNCTION double_double_bin_op
#define GENERIC_TYPE double
#define POW_FUNCTION(a,b) pow((double)(a),(double)(b))
#define POW_RESULT_TYPE double
#define MOD_FUNCTION(a,b) fmod((a),(b))
#define TRAP_DIV_ZERO	0
#define GENERIC_UNARY_FUNCTION double_unary_op
#define GENERIC_ARITH_UNARY_FUNCTION double_arith_unary_op
#define ABS_FUNCTION(a) fabs(a)
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#if SLANG_OPTIMIZE_FOR_SPEED
# define SCALAR_BINARY_FUNCTION double_double_scalar_bin_op
#endif
#define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE,(x))
#define PUSH_POW_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE, (x))
#define CMP_FUNCTION double_cmp_function
#include "slarith.inc"
#endif				       /* SLANG_HAS_FLOAT */

#define GENERIC_UNARY_FUNCTION char_unary_op
#define GENERIC_ARITH_UNARY_FUNCTION char_arith_unary_op
#define GENERIC_BINARY_FUNCTION char_char_arith_bin_op
#define JUST_BOOLEAN_BINARY_OPS
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE signed char
#define ABS_FUNCTION abs
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#define CMP_FUNCTION char_cmp_function
#include "slarith.inc"

#define GENERIC_UNARY_FUNCTION uchar_unary_op
#define GENERIC_ARITH_UNARY_FUNCTION uchar_arith_unary_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE unsigned char
#define GENERIC_TYPE_IS_UNSIGNED
#define ABS_FUNCTION(x) (x)
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : 0)
#define CMP_FUNCTION uchar_cmp_function
#define TO_BINARY_FUNCTION uchar_to_binary
#include "slarith.inc"

#if SHORT_IS_NOT_INT
#define GENERIC_UNARY_FUNCTION short_unary_op
#define GENERIC_ARITH_UNARY_FUNCTION short_arith_unary_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE short
#define ABS_FUNCTION abs
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#define CMP_FUNCTION short_cmp_function
#include "slarith.inc"

#define GENERIC_UNARY_FUNCTION ushort_unary_op
#define GENERIC_ARITH_UNARY_FUNCTION ushort_arith_unary_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE unsigned short
#define GENERIC_TYPE_IS_UNSIGNED
#define ABS_FUNCTION(x) (x)
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : 0)
#define CMP_FUNCTION ushort_cmp_function
#define TO_BINARY_FUNCTION ushort_to_binary
#include "slarith.inc"
#endif				       /* SHORT_IS_NOT_INT */

int _pSLarith_get_precedence (SLtype type)
{
   if ((type < SLANG_CHAR_TYPE) || (type > MAX_SLARITH_TYPE))
     return -1;

   type = Alias_Map[TYPE_TO_TABLE_INDEX(type)];
   return type - SLANG_CHAR_TYPE;
}

SLtype _pSLarith_promote_type (SLtype t)
{
   t = Alias_Map[TYPE_TO_TABLE_INDEX(t)];

   switch (t)
     {
      case SLANG_INT_TYPE:
      case SLANG_UINT_TYPE:
      case SLANG_LONG_TYPE:
      case SLANG_ULONG_TYPE:
#ifdef HAVE_LONG_LONG
      case SLANG_LLONG_TYPE:
      case SLANG_ULLONG_TYPE:
#endif
      case SLANG_FLOAT_TYPE:
      case SLANG_DOUBLE_TYPE:
	break;

      case SLANG_USHORT_TYPE:
#if SHORT_IS_INT
	t = SLANG_UINT_TYPE;
	break;
#endif
	/* drop */
      case SLANG_CHAR_TYPE:
      case SLANG_UCHAR_TYPE:
      case SLANG_SHORT_TYPE:
      default:
	t = SLANG_INT_TYPE;
     }

   return t;
}

static SLtype promote_to_common_type (SLtype a, SLtype b)
{
   SLtype a1;
   a1 = _pSLarith_promote_type (a);
   if (a == b)
     return a1;
   b = _pSLarith_promote_type (b);

   return (a1 > b) ? a1 : b;
}

static int arith_bin_op_result (int op, SLtype a_type, SLtype b_type,
				SLtype *c_type)
{
   switch (op)
     {
      case SLANG_EQ:
      case SLANG_NE:
      case SLANG_GT:
      case SLANG_GE:
      case SLANG_LT:
      case SLANG_LE:
      case SLANG_OR:
      case SLANG_AND:
	*c_type = SLANG_CHAR_TYPE;
	return 1;
#if SLANG_HAS_FLOAT
      case SLANG_POW:
	if (SLANG_FLOAT_TYPE == promote_to_common_type (a_type, b_type))
	  *c_type = SLANG_FLOAT_TYPE;
	else
	  *c_type = SLANG_DOUBLE_TYPE;
	return 1;
#endif
      case SLANG_BAND:
      case SLANG_BXOR:
      case SLANG_BOR:
      case SLANG_SHL:
      case SLANG_SHR:
	/* The bit-level operations are defined just for integer types */
	if ((0 == IS_INTEGER_TYPE (a_type))
	    || (0 == IS_INTEGER_TYPE(b_type)))
	  return 0;
	break;

      default:
	break;
     }

   *c_type = promote_to_common_type (a_type, b_type);
   return 1;
}

typedef int (*Bin_Fun_Type) (int,
			     SLtype, VOID_STAR, SLuindex_Type,
			     SLtype, VOID_STAR, SLuindex_Type,
			     VOID_STAR);

/* This array of functions must be indexed by precedence after arithmetic
 * promotions.
 */
static Bin_Fun_Type Bin_Fun_Map [MAX_ARITHMETIC_TYPES] =
{
   NULL,			       /* char */
     NULL,			       /* uchar */
     NULL,			       /* short */
     NULL,			       /* ushort */
     int_int_bin_op,		       /* int */
     uint_uint_bin_op,		       /* uint */
     long_long_bin_op,		       /* long */
     ulong_ulong_bin_op,	       /* ulong */
#ifdef HAVE_LONG_LONG
     llong_llong_bin_op,	       /* llong */
     ullong_ullong_bin_op,	       /* ullong */
#else
     NULL, NULL,
#endif
     float_float_bin_op,		       /* float */
     double_double_bin_op		       /* double */
};

static int arith_bin_op (int op,
			 SLtype a_type, VOID_STAR ap, SLuindex_Type na,
			 SLtype b_type, VOID_STAR bp, SLuindex_Type nb,
			 VOID_STAR cp)
{
   Bin_Fun_Type binfun;
   int c_indx;
   SLtype c_type;

   if ((a_type == b_type)
       && ((a_type == SLANG_CHAR_TYPE) || (a_type == SLANG_UCHAR_TYPE)))
     {
	switch (op)
	  {
	   case SLANG_EQ:
	   case SLANG_NE:
	   case SLANG_AND:
	   case SLANG_OR:
	     return char_char_arith_bin_op (op, a_type, ap, na, b_type, bp, nb, cp);
	  }
     }

   c_type = promote_to_common_type (a_type, b_type);
   c_indx = TYPE_TO_TABLE_INDEX(c_type);
   binfun = Bin_Fun_Map[c_indx];

   if ((c_type != a_type) || (c_type != b_type))
     {
	int ret;
	int a_indx = TYPE_TO_TABLE_INDEX(a_type);
	int b_indx = TYPE_TO_TABLE_INDEX(b_type);
	Convert_Fun_Type af = Binary_Matrix[a_indx][c_indx].convert_function;
	Convert_Fun_Type bf = Binary_Matrix[b_indx][c_indx].convert_function;

	if ((af != NULL)
	    && (NULL == (ap = (VOID_STAR) (*af) (ap, na))))
	  return -1;

	if ((bf != NULL)
	    && (NULL == (bp = (VOID_STAR) (*bf) (bp, nb))))
	  {
	     if (af != NULL) SLfree ((char *) ap);
	     return -1;
	  }

	ret = (*binfun) (op, a_type, ap, na, b_type, bp, nb, cp);
	if (af != NULL) SLfree ((char *) ap);
	if (bf != NULL) SLfree ((char *) bp);
	return ret;
     }

   return (*binfun) (op, a_type, ap, na, b_type, bp, nb, cp);
}

static int arith_unary_op_result (int op, SLtype a, SLtype *b)
{
   (void) a;
   switch (op)
     {
      default:
	return 0;

      case SLANG_SQR:
      case SLANG_MUL2:
      case SLANG_PLUSPLUS:
      case SLANG_MINUSMINUS:
      case SLANG_CHS:
      case SLANG_ABS:
	*b = a;
	break;

      case SLANG_BNOT:
	if (0 == IS_INTEGER_TYPE(a))
	  return 0;
	*b = a;
	break;

      case SLANG_SIGN:
	*b = SLANG_INT_TYPE;
	break;

      case SLANG_NOT:
      case SLANG_ISPOS:
      case SLANG_ISNEG:
      case SLANG_ISNONNEG:
	*b = SLANG_CHAR_TYPE;
	break;
     }
   return 1;
}

static int integer_pop (SLtype type, VOID_STAR ptr)
{
   SLang_Object_Type obj;
   int i, j;
   void (*f)(VOID_STAR, VOID_STAR, unsigned int);

   if (-1 == SLang_pop (&obj))
     return -1;

   if (0 == IS_INTEGER_TYPE(obj.o_data_type))
     {
	_pSLclass_type_mismatch_error (type, obj.o_data_type);
       	SLang_free_object (&obj);
	return -1;
     }

   i = TYPE_TO_TABLE_INDEX(type);
   j = TYPE_TO_TABLE_INDEX(obj.o_data_type);
   f = (void (*)(VOID_STAR, VOID_STAR, unsigned int))
     Binary_Matrix[j][i].copy_function;

   (*f) (ptr, (VOID_STAR)&obj.v, 1);

   return 0;
}

static int integer_push (SLtype type, VOID_STAR ptr)
{
   SLang_Object_Type obj;
   int i;
   void (*f)(VOID_STAR, VOID_STAR, unsigned int);

   i = TYPE_TO_TABLE_INDEX(type);
   f = (void (*)(VOID_STAR, VOID_STAR, unsigned int))
     Binary_Matrix[i][i].copy_function;

   obj.o_data_type = type;

   (*f) ((VOID_STAR)&obj.v, ptr, 1);

   return SLang_push (&obj);
}

int SLang_pop_char (char *i)
{
   return integer_pop (SLANG_CHAR_TYPE, (VOID_STAR) i);
}

int SLang_pop_uchar (unsigned char *i)
{
   return integer_pop (SLANG_UCHAR_TYPE, (VOID_STAR) i);
}

int SLang_pop_short (short *i)
{
   return integer_pop (_pSLANG_SHORT_TYPE, (VOID_STAR) i);
}

int SLang_pop_ushort (unsigned short *i)
{
   return integer_pop (_pSLANG_USHORT_TYPE, (VOID_STAR) i);
}

int SLang_pop_long (long *i)
{
   return integer_pop (_pSLANG_LONG_TYPE, (VOID_STAR) i);
}

int SLang_pop_ulong (unsigned long *i)
{
   return integer_pop (_pSLANG_ULONG_TYPE, (VOID_STAR) i);
}

#ifdef HAVE_LONG_LONG
#if LLONG_IS_NOT_LONG
static void llong_byte_code_destroy (SLtype unused, VOID_STAR ptr)
{
   (void) unused;
   SLfree (*(char **) ptr);
}
#endif
int SLang_pop_long_long (long long *i)
{
   return integer_pop (_pSLANG_LLONG_TYPE, (VOID_STAR) i);
}

int SLang_pop_ulong_long (unsigned long long *i)
{
   return integer_pop (_pSLANG_ULLONG_TYPE, (VOID_STAR) i);
}
#endif

int SLang_pop_uint (unsigned int *i)
{
   return integer_pop (SLANG_UINT_TYPE, (VOID_STAR) i);
}

int SLang_push_int (int i)
{
   return SLclass_push_int_obj (SLANG_INT_TYPE, i);
}
int SLang_push_uint (unsigned int i)
{
   return SLclass_push_int_obj (SLANG_UINT_TYPE, (int) i);
}
int SLang_push_char (char i)
{
   return SLclass_push_char_obj (SLANG_CHAR_TYPE, i);
}

int SLang_push_uchar (unsigned char i)
{
   return SLclass_push_char_obj (SLANG_UCHAR_TYPE, (char) i);
}
int SLang_push_short (short i)
{
   return SLclass_push_short_obj (_pSLANG_SHORT_TYPE, i);
}
int SLang_push_ushort (unsigned short i)
{
   return SLclass_push_short_obj (_pSLANG_USHORT_TYPE, (unsigned short) i);
}
int SLang_push_long (long i)
{
   return SLclass_push_long_obj (_pSLANG_LONG_TYPE, i);
}
int SLang_push_ulong (unsigned long i)
{
   return SLclass_push_long_obj (_pSLANG_ULONG_TYPE, (long) i);
}

#ifdef HAVE_LONG_LONG
int SLang_push_long_long (long long i)
{
   return SLclass_push_llong_obj (_pSLANG_LLONG_TYPE, i);
}
int SLang_push_ulong_long (unsigned long long i)
{
   return SLclass_push_llong_obj (_pSLANG_ULLONG_TYPE, (long long) i);
}
#endif

int SLang_pop_strlen_type (SLstrlen_Type *ip)
{
   return SLang_pop_uint (ip);
}
int SLang_push_strlen_type (SLstrlen_Type i)
{
   return SLang_push_uint (i);
}

_INLINE_
int _pSLarith_typecast (SLtype a_type, VOID_STAR ap, SLuindex_Type na,
			SLtype b_type, VOID_STAR bp)
{
   int i, j;

   void (*copy)(VOID_STAR, VOID_STAR, SLuindex_Type);

   i = TYPE_TO_TABLE_INDEX (a_type);
   j = TYPE_TO_TABLE_INDEX (b_type);

   copy = (void (*)(VOID_STAR, VOID_STAR, SLuindex_Type))
     Binary_Matrix[i][j].copy_function;

   (*copy) (bp, ap, na);
   return 1;
}

#if SLANG_HAS_FLOAT

int SLang_pop_double (double *x)
{
   SLang_Object_Type obj;

   if (0 != SLang_pop (&obj))
     return -1;

   switch (obj.o_data_type)
     {
      case SLANG_FLOAT_TYPE:
	*x = (double) obj.v.float_val;
	break;

      case SLANG_DOUBLE_TYPE:
	*x = obj.v.double_val;
	break;

      case SLANG_INT_TYPE:
	*x = (double) obj.v.int_val;
	break;

      case SLANG_CHAR_TYPE: *x = (double) obj.v.char_val; break;
      case SLANG_UCHAR_TYPE: *x = (double) obj.v.uchar_val; break;
      case SLANG_SHORT_TYPE: *x = (double) obj.v.short_val; break;
      case SLANG_USHORT_TYPE: *x = (double) obj.v.ushort_val; break;
      case SLANG_UINT_TYPE: *x = (double) obj.v.uint_val; break;
      case SLANG_LONG_TYPE: *x = (double) obj.v.long_val; break;
      case SLANG_ULONG_TYPE: *x = (double) obj.v.ulong_val; break;
#ifdef HAVE_LONG_LONG
      case SLANG_LLONG_TYPE: *x = (double) obj.v.llong_val; break;
      case SLANG_ULLONG_TYPE: *x = (double) obj.v.ullong_val; break;
#endif
      default:
	_pSLclass_type_mismatch_error (SLANG_DOUBLE_TYPE, obj.o_data_type);
	SLang_free_object (&obj);
	return -1;
     }
   return 0;
}

int SLang_push_double (double x)
{
   return SLclass_push_double_obj (SLANG_DOUBLE_TYPE, x);
}

int SLang_pop_float (float *x)
{
   double d;

   /* Pop it as a double and let the double function do all the typcasting */
   if (-1 == SLang_pop_double (&d))
     return -1;

   *x = (float) d;
   return 0;
}

int SLang_push_float (float f)
{
   return SLclass_push_float_obj (SLANG_FLOAT_TYPE, (double) f);
}

/* Double */
static int double_push (SLtype type, VOID_STAR ptr)
{
#if SLANG_OPTIMIZE_FOR_SPEED
   SLang_Object_Type obj;
   obj.o_data_type = type;
   obj.v.double_val = *(double *)ptr;
   return SLang_push (&obj);
#else
   return SLclass_push_double_obj (type, *(double *) ptr);
#endif
}

static int double_push_literal (SLtype type, VOID_STAR ptr)
{
   return SLclass_push_double_obj (type, **(double **)ptr);
}

static int double_pop (SLtype unused, VOID_STAR ptr)
{
   (void) unused;
   return SLang_pop_double ((double *) ptr);
}

static void double_byte_code_destroy (SLtype unused, VOID_STAR ptr)
{
   (void) unused;
   SLfree (*(char **) ptr);
}

static int float_push (SLtype unused, VOID_STAR ptr)
{
   (void) unused;
   SLang_push_float (*(float *) ptr);
   return 0;
}

static int float_pop (SLtype unused, VOID_STAR ptr)
{
   (void) unused;
   return SLang_pop_float ((float *) ptr);
}

#endif				       /* SLANG_HAS_FLOAT */

#if SLANG_HAS_FLOAT
static char Double_Format[16] = "%g";
static char *Double_Format_Ptr = NULL;
static unsigned int Double_Format_Expon_Threshold = 6;

void _pSLset_double_format (SLCONST char *fmt)
{
   /* The only forms accepted by this function are:
    * "%[+ ][width][.precision][efgS]"
    */
   SLCONST char *s = fmt;
   int precision = 6;

   if (*s++ != '%')
     return;

   /* 0 or more flags */
   while ((*s == '#') || (*s == '0') || (*s == '-')
	  || (*s == ' ') || (*s == '+'))
     s++;

   /* field width */
   while (isdigit (*s)) s++;

   /* precision */
   if (*s == '.')
     {
	s++;
	precision = 0;
	while (isdigit (*s))
	  {
	     precision = precision * 10 + (*s - '0');
	     s++;
	  }
	if (precision < 0)
	  precision = 6;
     }

   if ((*s == 'e') || (*s == 'E')
       || (*s == 'f') || (*s == 'F')
       || (*s == 'g') || (*s == 'G'))
     {
	s++;
	if (*s != 0)
	  return;		       /* more junk-- unacceptable */

	if (strlen (fmt) >= sizeof (Double_Format))
	  return;

	strcpy (Double_Format, fmt);
	Double_Format_Ptr = Double_Format;
	return;
     }

   if ((*s == 'S') || (*s == 's'))
     {
	s++;
	if (*s != 0)
	  return;

	Double_Format_Ptr = NULL;
	Double_Format_Expon_Threshold = precision;
	return;
     }

   /* error */
}

SLCONST char *_pSLget_double_format (void)
{
   if (Double_Format_Ptr == NULL)
     return "%S";

   return Double_Format_Ptr;
}

static void check_decimal (char *buf, unsigned int buflen, double x)
{
   char *b, *bstart = buf, *bufmax = buf + buflen;
   char ch;
   unsigned int count = 0, expon;
   int has_point = 0;
   unsigned int expon_threshold = Double_Format_Expon_Threshold;

   if (*bstart == '-')
     bstart++;

   b = bstart;
   while (1)
     {
	ch = *b;
	if (isdigit (ch))
	  {
	     count++;
	     b++;
	     continue;
	  }
	if (ch == 0)
	  break;		       /* all digits */

	if (ch != '.')
	  return;			       /* something else */

	/* We are at a decimal point.  If expon > 1, then buf does not contain
	 * an exponential formatted quantity.
	 */
	if (count <= expon_threshold)
	  return;
	/* We have something like: 1234567.123, which we want to
	 * write as 1.234567123e+6
	 */
	b += strlen(b);
	has_point = 1;
	break;			       /* handle below */
     }

   /* We get here only when *b==0. */

   if ((has_point == 0) && (count <= 6))
     {
	if (b + 3 >= bufmax)
	  {
	     sprintf (buf, "%e", x);
	     return;
	  }
	*b++ = '.';
	*b++ = '0';
	*b = 0;
	return;
     }

   expon = count-1;

   /* Now add on the exponent.  First move the decimal point but drop trailing 0s */
   while ((count > 1) && (*(b-1) == '0'))
     {
	b--;
	count--;
     }

   if (count > 1)
     {
	while (count > 1)
	  {
	     bstart[count] = bstart[count-1];
	     count--;
	  }
	bstart[count] = '.';
	if (has_point == 0)
	  b++;
     }

   if (EOF == SLsnprintf (b, bufmax-b, "e+%02d", expon))
     sprintf (buf, "%e", x);
}

static int massage_decimal_buffer (char *inbuf, char *buf,
				   unsigned int buflen, unsigned int min_slen)
{
   char *s;
   unsigned int slen, count;
   char c;

   slen = strlen(inbuf);
   if ((slen < min_slen) || (slen+1 > buflen))
     return 0;

   s = inbuf + slen;
   s -= 2;			       /* skip last digit */
   c = *s;
   if ((c != '0') && (c != '9'))
     return 0;
   s--;

   count = 0;
   while ((s > inbuf) && (*s == c))
     {
	count++;
	s--;
     }

   if ((count < 4) || (0 == isdigit (*s)))
     return 0;

   if (c == '9')
     {
	/* e.g., 9.699999999999999 */
	slen = s-inbuf;
	memcpy (buf, inbuf, slen);
	buf[slen] = *s + 1;	       /* assumes ascii */
	buf[slen+1] = 0;
     }
   else
     {
	/* 9.300000000000001 */
	slen = (s+1)-inbuf;
	memcpy (buf, inbuf, slen);
	buf[slen] = 0;
     }

   return 1;
}

static void massage_double_buffer (char *inbuf, double x)
{
   char buf[1024];

   if (massage_decimal_buffer (inbuf, buf, sizeof(buf), 16)
       && (atof(buf) == x))
     strcpy (inbuf, buf);
}

static void massage_float_buffer (char *inbuf, float x)
{
   char buf[1024];

   if (massage_decimal_buffer (inbuf, buf, sizeof(buf), 8)
       && ((float)atof(buf) == x))
     strcpy (inbuf, buf);
}

static void default_format_double (double x, char *buf, unsigned int buflen)
{
   if (EOF == SLsnprintf (buf, buflen, "%.16g", x))
     {
	sprintf (buf, "%e", x);
	return;
     }

   if (atof (buf) != x)
     {
	if (EOF == SLsnprintf (buf, buflen, "%.17g", x))
	  {
	     sprintf (buf, "%e", x);
	     return;
	  }
     }
   massage_double_buffer (buf,x);
   check_decimal (buf, buflen, x);
}

static void default_format_float (float x, char *buf, unsigned int buflen)
{
   if (EOF == SLsnprintf (buf, buflen, "%.8g", x))
     {
	sprintf (buf, "%e", x);
	return;
     }
   if ((float) atof (buf) != x)
     {
	if (EOF == SLsnprintf (buf, buflen, "%.9g", x))
	  {
	     sprintf (buf, "%e", x);
	     return;
	  }
     }
   massage_float_buffer (buf, x);
   check_decimal (buf, buflen, x);
}
#endif

static char *arith_string (SLtype type, VOID_STAR v)
{
   char buf [1024];
   char *s;

   s = buf;

   switch (type)
     {
      default:
	s = (char *) SLclass_get_datatype_name (type);
	break;

      case SLANG_CHAR_TYPE:
	sprintf (s, "%d", *(char *) v);
	break;
      case SLANG_UCHAR_TYPE:
	sprintf (s, "%u", *(unsigned char *) v);
	break;
      case SLANG_SHORT_TYPE:
	sprintf (s, "%d", *(short *) v);
	break;
      case SLANG_USHORT_TYPE:
	sprintf (s, "%u", *(unsigned short *) v);
	break;
      case SLANG_INT_TYPE:
	sprintf (s, "%d", *(int *) v);
	break;
      case SLANG_UINT_TYPE:
	sprintf (s, "%u", *(unsigned int *) v);
	break;
      case SLANG_LONG_TYPE:
	sprintf (s, "%ld", *(long *) v);
	break;
      case SLANG_ULONG_TYPE:
	sprintf (s, "%lu", *(unsigned long *) v);
	break;
#ifdef HAVE_LONG_LONG
      case SLANG_LLONG_TYPE:
	sprintf (s, "%lld", *(long long *) v);
	break;
      case SLANG_ULLONG_TYPE:
	sprintf (s, "%llu", *(unsigned long long *) v);
	break;
#endif
#if SLANG_HAS_FLOAT
      case SLANG_FLOAT_TYPE:
	if (Double_Format_Ptr == NULL)
	  default_format_float (*(float *)v, buf, sizeof(buf));
	else if (EOF == SLsnprintf (buf, sizeof (buf), Double_Format, *(float *) v))
	  sprintf (s, "%e", *(float *) v);
	break;
      case SLANG_DOUBLE_TYPE:
	if (Double_Format_Ptr == NULL)
	  default_format_double (*(double *)v, buf, sizeof(buf));
	else if (EOF == SLsnprintf (buf, sizeof (buf), Double_Format, *(double *) v))
	  sprintf (s, "%e", *(double *) v);
	break;
#endif
     }

   return SLmake_string (s);
}

static int integer_to_bool (SLtype type, int *t)
{
   (void) type;
   return SLang_pop_integer (t);
}

/* Note that integer literals are all stored in the byte-code as longs.  This
 * is why it is necessary to use *(long*).
 */
static int push_int_literal (SLtype type, VOID_STAR ptr)
{
   return SLclass_push_int_obj (type, (int) *(long *) ptr);
}

static int push_char_literal (SLtype type, VOID_STAR ptr)
{
   return SLclass_push_char_obj (type, (char) *(long *) ptr);
}

#if SHORT_IS_NOT_INT
static int push_short_literal (SLtype type, VOID_STAR ptr)
{
   return SLclass_push_short_obj (type, (short) *(long *) ptr);
}
#endif

#if LONG_IS_NOT_INT
static int push_long_literal (SLtype type, VOID_STAR ptr)
{
   return SLclass_push_long_obj (type, *(long *) ptr);
}
#endif

#ifdef HAVE_LONG_LONG
#if LLONG_IS_NOT_LONG
static int push_llong_literal (SLtype type, VOID_STAR ptr)
{
   return SLclass_push_llong_obj (type, **(long long **)ptr);
}
#endif
#endif
typedef struct
{
   SLFUTURE_CONST char *name;
   SLtype data_type;
   unsigned int sizeof_type;
   int (*unary_fun)(int, SLtype, VOID_STAR, SLuindex_Type, VOID_STAR);
   int (*push_literal) (SLtype, VOID_STAR);
   void (*byte_code_destroy)(SLtype, VOID_STAR);
   int (*cmp_fun) (SLtype, VOID_STAR, VOID_STAR, int *);
}
Integer_Info_Type;

#ifdef HAVE_LONG_LONG
# define NUM_INTEGER_TYPES 10
#else
# define NUM_INTEGER_TYPES 8
#endif
static Integer_Info_Type Integer_Types [NUM_INTEGER_TYPES] =
{
     {"Char_Type", SLANG_CHAR_TYPE, sizeof (char), char_unary_op, push_char_literal, NULL, char_cmp_function},
     {"UChar_Type", SLANG_UCHAR_TYPE, sizeof (unsigned char), uchar_unary_op, push_char_literal, NULL, uchar_cmp_function},
#if SHORT_IS_NOT_INT
     {"Short_Type", SLANG_SHORT_TYPE, sizeof (short), short_unary_op, push_short_literal, NULL, short_cmp_function},
     {"UShort_Type", SLANG_USHORT_TYPE, sizeof (unsigned short), ushort_unary_op, push_short_literal, NULL, ushort_cmp_function},
#else
     {NULL, SLANG_SHORT_TYPE, 0, NULL, NULL, NULL, NULL},
     {NULL, SLANG_USHORT_TYPE, 0, NULL, NULL, NULL, NULL},
#endif

     {"Integer_Type", SLANG_INT_TYPE, sizeof (int), int_unary_op, push_int_literal, NULL, int_cmp_function},
     {"UInteger_Type", SLANG_UINT_TYPE, sizeof (unsigned int), uint_unary_op, push_int_literal, NULL, uint_cmp_function},

#if LONG_IS_NOT_INT
     {"Long_Type", SLANG_LONG_TYPE, sizeof (long), long_unary_op, push_long_literal, NULL, long_cmp_function},
     {"ULong_Type", SLANG_ULONG_TYPE, sizeof (unsigned long), ulong_unary_op, push_long_literal, NULL, ulong_cmp_function},
#else
     {NULL, SLANG_LONG_TYPE, 0, NULL, NULL, NULL, NULL},
     {NULL, SLANG_ULONG_TYPE, 0, NULL, NULL, NULL, NULL},
#endif
#ifdef HAVE_LONG_LONG
# if LLONG_IS_NOT_LONG
     {"LLong_Type", SLANG_LLONG_TYPE, sizeof (long long), llong_unary_op, push_llong_literal, llong_byte_code_destroy, llong_cmp_function},
     {"ULLong_Type", SLANG_ULLONG_TYPE, sizeof (unsigned long long), ullong_unary_op, push_llong_literal, llong_byte_code_destroy, ullong_cmp_function},
# else
     {NULL, SLANG_LLONG_TYPE, 0, NULL, NULL, NULL, NULL},
     {NULL, SLANG_ULLONG_TYPE, 0, NULL, NULL, NULL, NULL},
# endif
#endif
};

int _pSLformat_as_binary (unsigned int min_num_bits, int use_binary_prefix)
{
#ifdef HAVE_LONG_LONG
   char buf [2*8*SIZEOF_LONG_LONG];
#else
   char buf [2*8*SIZEOF_LONG];
#endif
   char *bufp;
   int ret;
   unsigned int buflen;

   bufp = buf;
   buflen = sizeof(buf);
   if (use_binary_prefix)
     {
	*bufp++ = '0';
	*bufp++ = 'b';
	buflen -= 2;
     }

   switch (SLang_peek_at_stack ())
     {
      default:
      case SLANG_INT_TYPE:
      case SLANG_UINT_TYPE:
	  {
	     unsigned int u;
	     if (-1 == SLang_pop_uint (&u))
	       return -1;
	     ret = uint_to_binary (u, bufp, buflen, min_num_bits);
	  }
	break;

      case SLANG_CHAR_TYPE:
      case SLANG_UCHAR_TYPE:
	  {
	     unsigned char u;
	     if (-1 == SLang_pop_uchar (&u))
	       return -1;
	     ret = uchar_to_binary (u, bufp, buflen, min_num_bits);
	  }
	break;

      case SLANG_SHORT_TYPE:
      case SLANG_USHORT_TYPE:
	  {
	     unsigned short u;
	     if (-1 == SLang_pop_ushort (&u))
	       return -1;
	     ret = ushort_to_binary (u, bufp, buflen, min_num_bits);
	  }
	break;

      case SLANG_LONG_TYPE:
      case SLANG_ULONG_TYPE:
	  {
	     unsigned long u;
	     if (-1 == SLang_pop_ulong (&u))
	       return -1;
	     ret = ulong_to_binary (u, bufp, buflen, min_num_bits);
	  }
	break;

#ifdef HAVE_LONG_LONG
      case SLANG_LLONG_TYPE:
      case SLANG_ULLONG_TYPE:
	  {
	     unsigned long long u;
	     if (-1 == SLang_pop_ulong_long (&u))
	       return -1;
	     ret = ullong_to_binary (u, bufp, buflen, min_num_bits);
	  }
	break;
#endif
     }
   if (ret == -1)
     {
	SLang_verror (SL_INTERNAL_ERROR, "Buffer is not large enough for the binary representations");
	return -1;
     }

   (void) SLang_push_string (buf);
   return 0;
}

#if 0
static void to_binary_string_intrin (void)
{
   unsigned int min_num_bits = 0;

   if (SLang_Num_Function_Args == 2)
     {
	int n;
	if (-1 == SLang_pop_int (&n))
	  return;
	if (n > 0)
	  min_num_bits = (unsigned int) n;
     }

   (void) _pSLformat_as_binary (min_num_bits, 0);
}
#endif
static int create_synonyms (void)
{
   static SLFUTURE_CONST char *names[8] =
     {
	"Int16_Type", "UInt16_Type", "Int32_Type", "UInt32_Type",
	"Int64_Type", "UInt64_Type",
	"Float32_Type", "Float64_Type"
     };
   int types[8];
   unsigned int i;

   memset ((char *) types, 0, sizeof (types));
   /* The assumption is that sizeof(unsigned X) == sizeof (X) */
   types[0] = _pSLANG_INT16_TYPE;
   types[1] = _pSLANG_UINT16_TYPE;
   types[2] = _pSLANG_INT32_TYPE;
   types[3] = _pSLANG_UINT32_TYPE;
   types[4] = _pSLANG_INT64_TYPE;
   types[5] = _pSLANG_UINT64_TYPE;

#if SLANG_HAS_FLOAT

#if SIZEOF_FLOAT == 4
   types[6] = SLANG_FLOAT_TYPE;
#else
# if SIZEOF_DOUBLE == 4
   types[6] = SLANG_DOUBLE_TYPE;
# endif
#endif
#if SIZEOF_FLOAT == 8
   types[7] = SLANG_FLOAT_TYPE;
#else
# if SIZEOF_DOUBLE == 8
   types[7] = SLANG_DOUBLE_TYPE;
# endif
#endif

#endif

   if ((-1 == SLclass_create_synonym ("Int_Type", SLANG_INT_TYPE))
       || (-1 == SLclass_create_synonym ("UInt_Type", SLANG_UINT_TYPE)))
     return -1;

   for (i = 0; i < 8; i++)
     {
	if (types[i] == 0) continue;

	if (-1 == SLclass_create_synonym (names[i], types[i]))
	  return -1;
     }

   for (i = 0; i < MAX_ARITHMETIC_TYPES; i++)
     {
	Alias_Map[i] = TABLE_INDEX_TO_TYPE(i);
     }
#if SHORT_IS_INT
   Alias_Map [TYPE_TO_TABLE_INDEX(SLANG_SHORT_TYPE)] = SLANG_INT_TYPE;
   Alias_Map [TYPE_TO_TABLE_INDEX(SLANG_USHORT_TYPE)] = SLANG_UINT_TYPE;
   if ((-1 == SLclass_create_synonym ("Short_Type", SLANG_INT_TYPE))
       || (-1 == SLclass_create_synonym ("UShort_Type", SLANG_UINT_TYPE))
       || (-1 == _pSLclass_copy_class (SLANG_SHORT_TYPE, SLANG_INT_TYPE))
       || (-1 == _pSLclass_copy_class (SLANG_USHORT_TYPE, SLANG_UINT_TYPE)))
     return -1;
#endif
#if LONG_IS_INT
   Alias_Map [TYPE_TO_TABLE_INDEX(SLANG_LONG_TYPE)] = SLANG_INT_TYPE;
   Alias_Map [TYPE_TO_TABLE_INDEX(SLANG_ULONG_TYPE)] = SLANG_UINT_TYPE;
   if ((-1 == SLclass_create_synonym ("Long_Type", SLANG_INT_TYPE))
       || (-1 == SLclass_create_synonym ("ULong_Type", SLANG_UINT_TYPE))
       || (-1 == _pSLclass_copy_class (SLANG_LONG_TYPE, SLANG_INT_TYPE))
       || (-1 == _pSLclass_copy_class (SLANG_ULONG_TYPE, SLANG_UINT_TYPE)))
     return -1;
#endif
#if LLONG_IS_LONG
   Alias_Map [TYPE_TO_TABLE_INDEX(SLANG_LLONG_TYPE)] = _pSLANG_LONG_TYPE;
   Alias_Map [TYPE_TO_TABLE_INDEX(SLANG_ULLONG_TYPE)] = _pSLANG_ULONG_TYPE;
   if ((-1 == SLclass_create_synonym ("LLong_Type", _pSLANG_LONG_TYPE))
       || (-1 == SLclass_create_synonym ("ULLong_Type", _pSLANG_ULONG_TYPE))
       || (-1 == _pSLclass_copy_class (SLANG_LLONG_TYPE, _pSLANG_LONG_TYPE))
       || (-1 == _pSLclass_copy_class (SLANG_ULLONG_TYPE, _pSLANG_ULONG_TYPE)))
     return -1;
#endif

   return 0;
}

static SLang_Arith_Unary_Type Unary_Table [] =
{
   MAKE_ARITH_UNARY("abs", SLANG_ABS),
   MAKE_ARITH_UNARY("sign", SLANG_SIGN),
   MAKE_ARITH_UNARY("sqr", SLANG_SQR),
   MAKE_ARITH_UNARY("mul2", SLANG_MUL2),
   MAKE_ARITH_UNARY("chs", SLANG_CHS),
   MAKE_ARITH_UNARY("_ispos", SLANG_ISPOS),
   MAKE_ARITH_UNARY("_isneg", SLANG_ISNEG),
   MAKE_ARITH_UNARY("_isnonneg", SLANG_ISNONNEG),
   SLANG_END_ARITH_UNARY_TABLE
};

static SLang_Arith_Binary_Type Binary_Table [] =
{
   MAKE_ARITH_BINARY("_op_plus", SLANG_PLUS),
   MAKE_ARITH_BINARY("_op_minus", SLANG_MINUS),
   MAKE_ARITH_BINARY("_op_times", SLANG_TIMES),
   MAKE_ARITH_BINARY("_op_divide", SLANG_DIVIDE),
   MAKE_ARITH_BINARY("_op_eqs", SLANG_EQ),
   MAKE_ARITH_BINARY("_op_neqs", SLANG_NE),
   MAKE_ARITH_BINARY("_op_gt", SLANG_GT),
   MAKE_ARITH_BINARY("_op_ge", SLANG_GE),
   MAKE_ARITH_BINARY("_op_lt", SLANG_LT),
   MAKE_ARITH_BINARY("_op_le", SLANG_LE),
   MAKE_ARITH_BINARY("_op_pow", SLANG_POW),
   MAKE_ARITH_BINARY("_op_or", SLANG_OR),
   MAKE_ARITH_BINARY("_op_and", SLANG_AND),
   MAKE_ARITH_BINARY("_op_band", SLANG_BAND),
   MAKE_ARITH_BINARY("_op_bor", SLANG_BOR),
   MAKE_ARITH_BINARY("_op_xor", SLANG_BXOR),
   MAKE_ARITH_BINARY("_op_shl", SLANG_SHL),
   MAKE_ARITH_BINARY("_op_shr", SLANG_SHR),
   MAKE_ARITH_BINARY("_op_mod", SLANG_MOD),
   SLANG_END_ARITH_BINARY_TABLE
};

static SLang_Intrin_Fun_Type Intrinsic_Table [] =
{
#if 0				       /* need to think of a better name */
   MAKE_INTRINSIC_0("to_binary_string", to_binary_string_intrin, SLANG_VOID_TYPE),
#endif
   SLANG_END_INTRIN_FUN_TABLE
};

static SLang_IConstant_Type IConst_Table [] =
{
#if defined(SHRT_MIN) && defined(SHRT_MAX)
   MAKE_HCONSTANT_T("SHORT_MIN", SHRT_MIN, SLANG_SHORT_TYPE),
   MAKE_HCONSTANT_T("SHORT_MAX", SHRT_MAX, SLANG_SHORT_TYPE),
#endif
#if defined(USHRT_MAX)
   MAKE_HCONSTANT_T("USHORT_MAX", USHRT_MAX, SLANG_USHORT_TYPE),
#endif
#if defined(INT_MIN) && defined(INT_MAX)
   MAKE_ICONSTANT_T("INT_MIN", INT_MIN, SLANG_INT_TYPE),
   MAKE_ICONSTANT_T("INT_MAX", INT_MAX, SLANG_INT_TYPE),
#endif
#if defined(UINT_MAX)
   MAKE_ICONSTANT_T("UINT_MAX", UINT_MAX, SLANG_UINT_TYPE),
#endif
   SLANG_END_ICONST_TABLE
};

static SLang_LConstant_Type LConst_Table [] =
{
#if defined(LONG_MIN) && defined(LONG_MAX)
   MAKE_LCONSTANT_T("LONG_MIN", LONG_MIN, _pSLANG_LONG_TYPE),
   MAKE_LCONSTANT_T("LONG_MAX", LONG_MAX, _pSLANG_LONG_TYPE),
#endif
#if defined(ULONG_MAX)
   MAKE_LCONSTANT_T("ULONG_MAX", ULONG_MAX, _pSLANG_ULONG_TYPE),
#endif
   SLANG_END_LCONST_TABLE
};

#ifdef HAVE_LONG_LONG
# ifndef LLONG_MAX
#  if (SIZEOF_LONG_LONG == 8)
#   define LLONG_MAX 9223372036854775807LL
/* C90 does not have positive constants-- only negated negative ones.  Hence,
 * LLONG_MIN is -9223372036854775808LL, but 9223372036854775808LL is too big.
 */
#   define LLONG_MIN (-LLONG_MAX - 1LL)
#   define ULLONG_MAX 18446744073709551615ULL
#  endif
# endif
static _pSLang_LLConstant_Type LLConst_Table[] =
{
#if defined(LLONG_MIN) && defined(LLONG_MAX)
   _pMAKE_LLCONSTANT_T("LLONG_MIN", LLONG_MIN, SLANG_LLONG_TYPE),
   _pMAKE_LLCONSTANT_T("LLONG_MAX", LLONG_MAX, SLANG_LLONG_TYPE),
#endif
#if defined(ULLONG_MAX)
   _pMAKE_LLCONSTANT_T("ULLONG_MAX", ULLONG_MAX, SLANG_ULLONG_TYPE),
#endif
   _pSLANG_END_LLCONST_TABLE
};
#endif

static SLang_FConstant_Type FConst_Table [] =
{
#if defined(FLT_MIN) && defined(FLT_MAX)
   MAKE_FCONSTANT("FLOAT_MIN", FLT_MIN),
   MAKE_FCONSTANT("FLOAT_MAX", FLT_MAX),
#endif
#if defined(FLT_EPSILON)
   MAKE_FCONSTANT("FLOAT_EPSILON", FLT_EPSILON),
#endif
   SLANG_END_FCONST_TABLE
};

static SLang_DConstant_Type DConst_Table [] =
{
#if defined(DBL_MIN) && defined(DBL_MAX)
   MAKE_DCONSTANT("DOUBLE_MIN", DBL_MIN),
   MAKE_DCONSTANT("DOUBLE_MAX", DBL_MAX),
#endif
#if defined(DBL_EPSILON)
   MAKE_DCONSTANT("DOUBLE_EPSILON", DBL_EPSILON),
#endif
   SLANG_END_DCONST_TABLE
};

static void compute_inf_an_nan (void)
{
#if SLANG_HAS_FLOAT
   volatile double nan_val, inf_val;
# if SLANG_HAS_IEEE_FP
   volatile double big;
   unsigned int max_loops = 256;

   big = 1e16;
   inf_val = 1.0;

   while (max_loops)
     {
	max_loops--;
	big *= 1e16;
	if (inf_val == big)
	  break;
	inf_val = big;
     }
   if (max_loops == 0)
     {
	inf_val = DBL_MAX;
	nan_val = DBL_MAX;
     }
   else nan_val = inf_val/inf_val;
# else
   inf_val = DBL_MAX;
   nan_val = DBL_MAX;
# endif
   _pSLang_NaN = nan_val;
   _pSLang_Inf = inf_val;
#endif
}

int _pSLarith_register_types (void)
{
   SLang_Class_Type *cl;
   SLtype a_type, b_type;
   int i, j;

#if defined(HAVE_SETLOCALE) && defined(LC_NUMERIC)
   /* make sure decimal point it used --- the parser requires it */
   (void) setlocale (LC_NUMERIC, "C");
#endif

   for (i = 0; i < NUM_INTEGER_TYPES; i++)
     {
	Integer_Info_Type *info;

	info = Integer_Types + i;

	_pSLang_set_arith_type (info->data_type, 1);

	if (info->name == NULL)
	  {
	     /* This happens when the object is the same size as an integer
	      * For this case, we really want to copy the integer class.
	      * We will handle that when the synonym is created.
	      */
	     continue;
	  }

	if (NULL == (cl = SLclass_allocate_class (info->name)))
	  return -1;

	(void) SLclass_set_string_function (cl, arith_string);
	(void) SLclass_set_push_function (cl, integer_push);
	(void) SLclass_set_pop_function (cl, integer_pop);
	cl->cl_push_literal = info->push_literal;
	cl->cl_to_bool = integer_to_bool;
	cl->cl_byte_code_destroy = info->byte_code_destroy;

	cl->cl_cmp = info->cmp_fun;

	if (-1 == SLclass_register_class (cl, info->data_type, info->sizeof_type,
					  SLANG_CLASS_TYPE_SCALAR))
	  return -1;
	if (-1 == SLclass_add_unary_op (info->data_type, info->unary_fun, arith_unary_op_result))
	  return -1;
#if 0
	if (-1 == _pSLclass_add_arith_unary_op (info->data_type, info->arith_unary_fun, arith_unary_arith_op_result))
	  return -1;
#endif
     }

#if SLANG_HAS_FLOAT
   if (NULL == (cl = SLclass_allocate_class ("Double_Type")))
     return -1;
   (void) SLclass_set_push_function (cl, double_push);
   (void) SLclass_set_pop_function (cl, double_pop);
   (void) SLclass_set_string_function (cl, arith_string);
   cl->cl_byte_code_destroy = double_byte_code_destroy;
   cl->cl_push_literal = double_push_literal;
   cl->cl_cmp = double_cmp_function;

   if (-1 == SLclass_register_class (cl, SLANG_DOUBLE_TYPE, sizeof (double),
				     SLANG_CLASS_TYPE_SCALAR))
     return -1;
   if (-1 == SLclass_add_unary_op (SLANG_DOUBLE_TYPE, double_unary_op, arith_unary_op_result))
     return -1;
#if 0
   if (-1 == _pSLclass_add_arith_unary_op (SLANG_DOUBLE_TYPE, double_arith_unary_op, arith_unary_op_result))
     return -1;
#endif
   _pSLang_set_arith_type (SLANG_DOUBLE_TYPE, 2);

   if (NULL == (cl = SLclass_allocate_class ("Float_Type")))
     return -1;
   (void) SLclass_set_string_function (cl, arith_string);
   (void) SLclass_set_push_function (cl, float_push);
   (void) SLclass_set_pop_function (cl, float_pop);
   cl->cl_cmp = float_cmp_function;

   if (-1 == SLclass_register_class (cl, SLANG_FLOAT_TYPE, sizeof (float),
				     SLANG_CLASS_TYPE_SCALAR))
     return -1;
   if (-1 == SLclass_add_unary_op (SLANG_FLOAT_TYPE, float_unary_op, arith_unary_op_result))
     return -1;
#if 0
   if (-1 == _pSLclass_add_arith_unary_op (SLANG_FLOAT_TYPE, float_arith_unary_op, arith_unary_op_result))
     return -1;
#endif
   _pSLang_set_arith_type (SLANG_FLOAT_TYPE, 2);
#endif

   if (-1 == create_synonyms ())
     return -1;

   for (i = 0; i < MAX_ARITHMETIC_TYPES; i++)
     {
	a_type = _pSLarith_Arith_Types[i];
#if 0
	if (Alias_Map[TYPE_TO_TABLE_INDEX(a_type)] != a_type)
	  continue;
#endif
	if (a_type == 0)
	  continue;

	for (j = 0; j < MAX_ARITHMETIC_TYPES; j++)
	  {
	     int implicit_ok;

	     b_type = _pSLarith_Arith_Types[j];
	     if (b_type == 0)
	       continue;
	     /* Allow implicit typecast, except from int to float */
	     implicit_ok = ((b_type >= SLANG_FLOAT_TYPE)
			    || (a_type < SLANG_FLOAT_TYPE));

	     if (-1 == SLclass_add_binary_op (a_type, b_type, arith_bin_op, arith_bin_op_result))
	       return -1;

	     if (a_type != b_type)
	       if (-1 == SLclass_add_typecast (a_type, b_type, _pSLarith_typecast, implicit_ok))
		 return -1;
	  }
     }

   if (-1 == SLadd_intrin_fun_table (Intrinsic_Table, NULL))
     return -1;
   if (-1 == _pSLadd_arith_unary_table (Unary_Table, NULL))
     return -1;
   if (-1 == _pSLadd_arith_binary_table (Binary_Table, NULL))
     return -1;

   if ((-1 == SLadd_iconstant_table (IConst_Table, NULL))
       || (-1 == SLadd_lconstant_table (LConst_Table, NULL))
#if SLANG_HAS_FLOAT
       || (-1 == SLadd_fconstant_table (FConst_Table, NULL))
       || (-1 == SLadd_dconstant_table (DConst_Table, NULL))
#endif
#ifdef HAVE_LONG_LONG
       || (-1 == _pSLadd_llconstant_table (LLConst_Table, NULL))
#endif
       )
     return -1;

   compute_inf_an_nan ();

   return 0;
}

#if SLANG_OPTIMIZE_FOR_SPEED

static void promote_objs (SLang_Object_Type *a, SLang_Object_Type *b,
			  SLang_Object_Type *c, SLang_Object_Type *d)
{
   SLtype ia, ib, ic, id;
   int i, j;
   void (*copy)(VOID_STAR, VOID_STAR, unsigned int);

   ia = a->o_data_type;
   ib = b->o_data_type;

   ic = _pSLarith_promote_type (ia);

   if (ic == ib) id = ic;	       /* already promoted */
   else id = _pSLarith_promote_type (ib);

   i = TYPE_TO_TABLE_INDEX(ic);
   j = TYPE_TO_TABLE_INDEX(id);
   if (i > j)
     {
	id = ic;
	j = i;
     }

   c->o_data_type = d->o_data_type = id;

   i = TYPE_TO_TABLE_INDEX(ia);
   copy = (void (*)(VOID_STAR, VOID_STAR, unsigned int))
     Binary_Matrix[i][j].copy_function;
   (*copy) ((VOID_STAR) &c->v, (VOID_STAR)&a->v, 1);

   i = TYPE_TO_TABLE_INDEX(ib);
   copy = (void (*)(VOID_STAR, VOID_STAR, unsigned int))
     Binary_Matrix[i][j].copy_function;
   (*copy) ((VOID_STAR) &d->v, (VOID_STAR)&b->v, 1);
}

/* Crazy return value: returns 1 if operation not supported by this function,
 * 0 if it is and was sucessful, or -1 if something went wrong
 */
int _pSLarith_bin_op (SLang_Object_Type *oa, SLang_Object_Type *ob, int op)
{
   SLtype a_type, b_type;
   SLang_Object_Type obj_a, obj_b;

   a_type = oa->o_data_type;
   b_type = ob->o_data_type;

   if (a_type != b_type)
     {
	/* Handle common cases */
#if SLANG_HAS_FLOAT
	if ((a_type == SLANG_INT_TYPE)
	    && (b_type == SLANG_DOUBLE_TYPE))
	  return double_double_scalar_bin_op (oa->v.int_val, ob->v.double_val, op);

	if ((a_type == SLANG_DOUBLE_TYPE)
	    && (b_type == SLANG_INT_TYPE))
	  return double_double_scalar_bin_op (oa->v.double_val, ob->v.int_val, op);
#endif
	/* Otherwise do it the hard way */
	promote_objs (oa, ob, &obj_a, &obj_b);
	oa = &obj_a;
	ob = &obj_b;

	a_type = oa->o_data_type;
	/* b_type = ob->data_type; */
     }

   switch (a_type)
     {
      case SLANG_CHAR_TYPE:
	return int_int_scalar_bin_op (oa->v.char_val, ob->v.char_val, op);

      case SLANG_UCHAR_TYPE:
	return int_int_scalar_bin_op (oa->v.uchar_val, ob->v.uchar_val, op);

      case SLANG_SHORT_TYPE:
	return int_int_scalar_bin_op (oa->v.short_val, ob->v.short_val, op);

      case SLANG_USHORT_TYPE:
# if SHORT_IS_INT
	return uint_uint_scalar_bin_op (oa->v.ushort_val, ob->v.ushort_val, op);
# else
	return int_int_scalar_bin_op ((int)oa->v.ushort_val, (int)ob->v.ushort_val, op);
# endif

#if LONG_IS_INT
      case SLANG_LONG_TYPE:
#endif
      case SLANG_INT_TYPE:
	return int_int_scalar_bin_op (oa->v.int_val, ob->v.int_val, op);

#if LONG_IS_INT
      case SLANG_ULONG_TYPE:
#endif
      case SLANG_UINT_TYPE:
	return uint_uint_scalar_bin_op (oa->v.uint_val, ob->v.uint_val, op);

#if LONG_IS_NOT_INT
      case SLANG_LONG_TYPE:
	return long_long_scalar_bin_op (oa->v.long_val, ob->v.long_val, op);
      case SLANG_ULONG_TYPE:
	return ulong_ulong_scalar_bin_op (oa->v.ulong_val, ob->v.ulong_val, op);
#endif
#ifdef HAVE_LONG_LONG
      case SLANG_LLONG_TYPE:
	return llong_llong_scalar_bin_op (oa->v.llong_val, ob->v.llong_val, op);
      case SLANG_ULLONG_TYPE:
	return ullong_ullong_scalar_bin_op (oa->v.ullong_val, ob->v.ullong_val, op);
#endif
#if SLANG_HAS_FLOAT
      case SLANG_FLOAT_TYPE:
	return float_float_scalar_bin_op (oa->v.float_val, ob->v.float_val, op);
      case SLANG_DOUBLE_TYPE:
	return double_double_scalar_bin_op (oa->v.double_val, ob->v.double_val, op);
#endif
     }

   return 1;
}
#endif
