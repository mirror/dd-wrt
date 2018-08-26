/* sin, cos, etc, for S-Lang */
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

#include <float.h>
#include <math.h>

#if SLANG_HAS_FLOAT

#ifdef HAVE_FLOATINGPOINT_H
# include <floatingpoint.h>
#endif

#ifdef HAVE_FENV_H
# include <fenv.h>
#endif

#ifdef HAVE_IEEEFP_H
# include <ieeefp.h>
#endif

#ifdef HAVE_NAN_H
# include <nan.h>
#endif

#include "slang.h"
#include "_slang.h"

#ifdef PI
# undef PI
#endif
#define PI 3.14159265358979323846264338327950288

#if defined(__unix__)
#include <signal.h>
#include <errno.h>

static void math_floating_point_exception (int sig)
{
   sig = errno;
   (void) SLsignal (SIGFPE, math_floating_point_exception);
   errno = sig;
}
#endif

double SLmath_hypot (double x, double y)
{
#ifdef HAVE_HYPOT
   return hypot (x,y);
#else
   double fr, fi, ratio;

   fr = fabs(x);
   fi = fabs(y);

   if (fr > fi)
     {
	ratio = y / x;
	x = fr * sqrt (1.0 + ratio * ratio);
     }
   else if (fi == 0.0) x = 0.0;
   else
     {
	ratio = x / y;
	x = fi * sqrt (1.0 + ratio * ratio);
     }

   return x;
#endif
}

static int double_math_op_result (int op, SLtype a, SLtype *b)
{
   switch (op)
     {
      case SLMATH_ISINF:
      case SLMATH_ISNAN:
	*b = SLANG_CHAR_TYPE;
	break;

      default:
	if (a != SLANG_FLOAT_TYPE)
	  *b = SLANG_DOUBLE_TYPE;
	else
	  *b = a;
	break;
     }
   return 1;
}

#ifdef HAVE_ISNAN
# define ISNAN_FUN	isnan
#else
# define ISNAN_FUN	my_isnan
static int my_isnan (double x)
{
   return (volatile double) x != (volatile double) x;
}
#endif

int _pSLmath_isnan (double x)
{
   return ISNAN_FUN (x);
}

#ifdef HAVE_ISINF
# define ISINF_FUN	isinf
#else
# define ISINF_FUN	my_isinf
static int my_isinf (double x)
{
#ifdef HAVE_FINITE
   return (0 == finite (x)) && (0 == ISNAN_FUN (x));
#else
   double y;
   if (x == 0.0)
     return 0;
   y = x * 0.5;
   return (x == y);
#endif
}
#endif

int _pSLmath_isinf (double x)
{
   return ISINF_FUN (x);
}

#ifdef HAVE_ROUND
# define ROUND_FUN	round
#else
# define ROUND_FUN	my_round
static double my_round (double x)
{
   double xf, xi;

   xf = modf (x, &xi);		       /* x = xi + xf */
   if (xi > 0)
     {
	if (xf >= 0.5)
	  return xi + 1.0;
     }
   else if (xi < 0)
     {
	if (xf <= -0.5)
	  return xi - 1.0;
     }
   else if (xf < 0)		       /* xi=0 */
     {
	if (xf <= -0.5)
	  return -1.0;
     }
   else if (xf >= 0.5)		       /* xi=0 */
     return 1.0;

   return xi;
}
#endif

#ifndef HAVE_LOG1P
double _pSLmath_log1p (double x)
{
   /*
    * log(1+x) ~= x for small x
    * Suppose the error in 1+x is e.  That is, 1+x produces 1+x+e.
    * Then log(1+x) will produce log(1+x+e).  The error is
    *   df = log(1+x+e)-log(1+x)
    *      = log (1 + e/(1+x))
    *      ~= e/(1+x)
    * Here e = (1+x) - (1+x)
    *        = ((1+x)-1) - x
    * So compute:
    *   log1p(x) = log(1+x) - (((1+x)-1)-x)/(1+x)
    */
   volatile double xp1, xx;
   if (ISINF_FUN(x))
     {
	if (x < 0) return _pSLang_NaN;
	return _pSLang_Inf;
     }
   xp1 = 1.0 + x;
   if (xp1 == 0.0)
     return -_pSLang_Inf;
   xx = xp1-1.0;
   return log(xp1) - (xx-x)/xp1;
}
#endif

#ifndef HAVE_EXPM1
double _pSLmath_expm1 (double x)
{
   /* f = exp(x)-1
    *    = (exp(x)-1) + x - x
    *    = (exp(x)-(1+x)) + x
    *
    * While the above works, it is not as accurate as the algroithm below,
    * which is due to Kahan (yes, that Kahan).
    */
   volatile double u;
   if (ISINF_FUN(x))
     {
	if (x < 0) return -1.0;
	return _pSLang_Inf;
     }

   u = exp(x);
   if (u == 1.0)
     return x;
   if (u - 1.0 == -1.0)
     return -1.0;
   return (u-1.0)*x/log(u);
}
#endif

#ifdef HAVE_ASINH
# define ASINH_FUN	asinh
#else
# define ASINH_FUN	my_asinh
static double my_asinh (double x)
{
   /* f(x) = log(x + sqrt(x^2+1))
    * Note: f(x)+f(-x)
    *          = log(x+sqrt(x^2+1))+log(-x+sqrt(x^2+1))
    *          = log(x^2+1 - x^2)
    *          = 0
    * Thus, f(-x)=-f(x) as required.
    *
    * So consider x>=0.
    * For large x
    *   = log(2x + sqrt(x^2+1)-x)
    *   = log(2x + ((x^2+1)-x^2)/(x+sqrt(x^2+1)))
    *   = log(2x + 1/(x+sqrt(x^2+1)))
    *   = log(2x + 1/x/(1+sqrt(1+1/x^2)))
    * For small x
    *   = log(1 + x + sqrt(1+x^2)-1)
    *   = log(1 + x + (1+x^2-1)/(sqrt(1+x^2)+1))
    *   = log(1 + x + x^2/(1+sqrt(1+x^2)))
    */
   double s = 1.0;
   double x2 = x*x;

   if (x < 0.0)
     {
	s = -1.0;
	x = -x;
     }

   if (x > 1.0)
     return s*log (2.0*x + 1.0/x/(1.0+sqrt(1.0+1.0/x2)));

   return LOG1P_FUNC(x+x2/(1.0+sqrt(1.0+x2)));
}
#endif

#ifdef HAVE_ACOSH
# define ACOSH_FUN	acosh
#else
# define ACOSH_FUN	my_acosh
static double my_acosh (double x)
{
   /* log (x + sqrt(x^2 - 1);     x >= 1
    *  = log(2*x + sqrt(x^2-1)-x)
    *  = log(2*x + (x^2-1-x^2)/(x+sqrt(x^2-1)))
    *  = log(2*x - 1/x/(1+sqrt(1-1/x^2)))
    */
   return log(2.0*x - 1.0/x/(1.0+sqrt(1.0-1/x/x)));
}
#endif
#ifdef HAVE_ATANH
# define ATANH_FUN	atanh
#else
# define ATANH_FUN	my_atanh

static double my_atanh (double x)
{
   /* 0.5 * log ((1.0 + x)/(1.0 - x));  (0 <= x^2 < 1)
    * = 0.5*log((1-x+2x)/(1-x))
    * = 0.5*log(1 + 2x/(1-x))
    */
   return 0.5 * LOG1P_FUNC(2.0*x/(1-x));
}
#endif

static int double_math_op (int op,
			   SLtype type, VOID_STAR ap, SLuindex_Type na,
			   VOID_STAR bp)
{
   double *a, *b;
   unsigned int i;
   char *c;

   (void) type;
   a = (double *) ap;
   b = (double *) bp;

   switch (op)
     {
      default:
	return 0;

      case SLMATH_SINH:
	for (i = 0; i < na; i++) b[i] = sinh(a[i]);
	break;
      case SLMATH_COSH:
	for (i = 0; i < na; i++) b[i] = cosh(a[i]);
	break;
      case SLMATH_TANH:
	for (i = 0; i < na; i++) b[i] = tanh(a[i]);
	break;
      case SLMATH_TAN:
	for (i = 0; i < na; i++) b[i] = tan(a[i]);
	break;
      case SLMATH_ASIN:
	for (i = 0; i < na; i++) b[i] = asin(a[i]);
	break;
      case SLMATH_ACOS:
	for (i = 0; i < na; i++) b[i] = acos(a[i]);
	break;
      case SLMATH_ATAN:
	for (i = 0; i < na; i++) b[i] = atan(a[i]);
	break;
      case SLMATH_EXP:
	for (i = 0; i < na; i++) b[i] = exp(a[i]);
	break;
      case SLMATH_LOG:
	for (i = 0; i < na; i++) b[i] = log(a[i]);
	break;
      case SLMATH_LOG10:
	for (i = 0; i < na; i++) b[i] = log10(a[i]);
	break;
      case SLMATH_SQRT:
	for (i = 0; i < na; i++) b[i] = sqrt (a[i]);
	return 1;
      case SLMATH_SIN:
	for (i = 0; i < na; i++) b[i] = sin(a[i]);
	break;
      case SLMATH_COS:
	for (i = 0; i < na; i++) b[i] = cos(a[i]);
	break;
      case SLMATH_ASINH:
	for (i = 0; i < na; i++) b[i] = ASINH_FUN(a[i]);
	break;
      case SLMATH_ATANH:
	for (i = 0; i < na; i++) b[i] = ATANH_FUN(a[i]);
	break;
      case SLMATH_ACOSH:
	for (i = 0; i < na; i++) b[i] = ACOSH_FUN(a[i]);
	break;

      case SLMATH_CONJ:
      case SLMATH_REAL:
	for (i = 0; i < na; i++)
	  b[i] = a[i];
	return 1;
      case SLMATH_IMAG:
	for (i = 0; i < na; i++)
	  b[i] = 0.0;
	return 1;

      case SLMATH_ISINF:
	c = (char *) bp;
	for (i = 0; i < na; i++)
	  c[i] = (char) ISINF_FUN(a[i]);
	return 1;

      case SLMATH_ISNAN:
	c = (char *) bp;
	for (i = 0; i < na; i++)
	  c[i] = (char) ISNAN_FUN(a[i]);
	return 1;

      case SLMATH_FLOOR:
	for (i = 0; i < na; i++) b[i] = floor(a[i]);
	break;

      case SLMATH_CEIL:
	for (i = 0; i < na; i++) b[i] = ceil(a[i]);
	break;

      case SLMATH_ROUND:
	for (i = 0; i < na; i++) b[i] = ROUND_FUN(a[i]);
	break;

      case SLMATH_EXPM1:
	for (i = 0; i < na; i++) b[i] = EXPM1_FUNC(a[i]);
	break;

      case SLMATH_LOG1P:
	for (i = 0; i < na; i++) b[i] = LOG1P_FUNC(a[i]);
	break;
     }

   return 1;
}

static int float_math_op (int op,
			  SLtype type, VOID_STAR ap, SLuindex_Type na,
			  VOID_STAR bp)
{
   float *a, *b;
   unsigned int i;
   char *c;

   (void) type;
   a = (float *) ap;
   b = (float *) bp;

   switch (op)
     {
      default:
	return 0;

      case SLMATH_SINH:
	for (i = 0; i < na; i++) b[i] = sinh(a[i]);
	break;
      case SLMATH_COSH:
	for (i = 0; i < na; i++) b[i] = cosh(a[i]);
	break;
      case SLMATH_TANH:
	for (i = 0; i < na; i++) b[i] = tanh(a[i]);
	break;
      case SLMATH_TAN:
	for (i = 0; i < na; i++) b[i] = tan(a[i]);
	break;
      case SLMATH_ASIN:
	for (i = 0; i < na; i++) b[i] = asin(a[i]);
	break;
      case SLMATH_ACOS:
	for (i = 0; i < na; i++) b[i] = acos(a[i]);
	break;
      case SLMATH_ATAN:
	for (i = 0; i < na; i++) b[i] = atan(a[i]);
	break;
      case SLMATH_EXP:
	for (i = 0; i < na; i++) b[i] = exp(a[i]);
	break;
      case SLMATH_LOG:
	for (i = 0; i < na; i++) b[i] = log(a[i]);
	break;
      case SLMATH_LOG10:
	for (i = 0; i < na; i++) b[i] = log10(a[i]);
	break;
      case SLMATH_SQRT:
	for (i = 0; i < na; i++)
	  b[i] = (float) sqrt ((double) a[i]);
	return 1;

      case SLMATH_SIN:
	for (i = 0; i < na; i++) b[i] = sin(a[i]);
	break;
      case SLMATH_COS:
	for (i = 0; i < na; i++) b[i] = cos(a[i]);
	break;

      case SLMATH_ASINH:
	for (i = 0; i < na; i++) b[i] = ASINH_FUN(a[i]);
	break;
      case SLMATH_ATANH:
	for (i = 0; i < na; i++) b[i] = ATANH_FUN(a[i]);
	break;
      case SLMATH_ACOSH:
	for (i = 0; i < na; i++) b[i] = ACOSH_FUN(a[i]);
	break;

      case SLMATH_CONJ:
      case SLMATH_REAL:
	for (i = 0; i < na; i++)
	  b[i] = a[i];
	return 1;
      case SLMATH_IMAG:
	for (i = 0; i < na; i++)
	  b[i] = 0.0;
	return 1;
      case SLMATH_ISINF:
	c = (char *) bp;
	for (i = 0; i < na; i++)
	  c[i] = (char) ISINF_FUN((double) a[i]);
	return 1;

      case SLMATH_ISNAN:
	c = (char *) bp;
	for (i = 0; i < na; i++)
	  c[i] = (char) ISNAN_FUN((double) a[i]);
	return 1;

      case SLMATH_FLOOR:
	for (i = 0; i < na; i++) b[i] = floor(a[i]);
	break;
      case SLMATH_CEIL:
	for (i = 0; i < na; i++) b[i] = ceil(a[i]);
	break;
      case SLMATH_ROUND:
	for (i = 0; i < na; i++) b[i] = ROUND_FUN(a[i]);
	break;

      case SLMATH_EXPM1:
	for (i = 0; i < na; i++) b[i] = EXPM1_FUNC(a[i]);
	break;

      case SLMATH_LOG1P:
	for (i = 0; i < na; i++) b[i] = LOG1P_FUNC(a[i]);
	break;
     }

   return 1;
}

static int generic_math_op (int op,
			    SLtype type, VOID_STAR ap, SLuindex_Type na,
			    VOID_STAR bp)
{
   double *b;
   unsigned int i;
   SLang_To_Double_Fun_Type to_double;
   unsigned int da;
   char *a, *c;

   if (NULL == (to_double = SLarith_get_to_double_fun (type, &da)))
     return 0;

   b = (double *) bp;
   a = (char *) ap;

   switch (op)
     {
      default:
	return 0;

      case SLMATH_SINH:
	for (i = 0; i < na; i++)
	  {
	     b[i] = sinh (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_COSH:
	for (i = 0; i < na; i++)
	  {
	     b[i] = cosh (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_TANH:
	for (i = 0; i < na; i++)
	  {
	     b[i] = tanh (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_TAN:
	for (i = 0; i < na; i++)
	  {
	     b[i] = tan (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_ASIN:
	for (i = 0; i < na; i++)
	  {
	     b[i] = asin (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_ACOS:
	for (i = 0; i < na; i++)
	  {
	     b[i] = acos (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_ATAN:
	for (i = 0; i < na; i++)
	  {
	     b[i] = atan (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_EXP:
	for (i = 0; i < na; i++)
	  {
	     b[i] = exp (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_LOG:
	for (i = 0; i < na; i++)
	  {
	     b[i] = log (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_LOG10:
	for (i = 0; i < na; i++)
	  {
	     b[i] = log10 (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_SQRT:
	for (i = 0; i < na; i++)
	  {
	     b[i] = sqrt (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_SIN:
	for (i = 0; i < na; i++)
	  {
	     b[i] = sin (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_COS:
	for (i = 0; i < na; i++)
	  {
	     b[i] = cos (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_ASINH:
	for (i = 0; i < na; i++)
	  {
	     b[i] = ASINH_FUN (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_ATANH:
	for (i = 0; i < na; i++)
	  {
	     b[i] = ATANH_FUN (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_ACOSH:
	for (i = 0; i < na; i++)
	  {
	     b[i] = ACOSH_FUN (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_CONJ:
      case SLMATH_REAL:
	for (i = 0; i < na; i++)
	  {
	     b[i] = to_double((VOID_STAR) a);
	     a += da;
	  }
	return 1;

      case SLMATH_IMAG:
	for (i = 0; i < na; i++)
	  b[i] = 0.0;
	return 1;

      case SLMATH_ISINF:
	c = (char *) bp;
	for (i = 0; i < na; i++)
	  {
	     c[i] = (char) ISINF_FUN(to_double((VOID_STAR) a));
	     a += da;
	  }
	return 1;
      case SLMATH_ISNAN:
	c = (char *) bp;
	for (i = 0; i < na; i++)
	  {
	     c[i] = (char) ISNAN_FUN(to_double((VOID_STAR) a));
	     a += da;
	  }
	return 1;
      case SLMATH_FLOOR:
	for (i = 0; i < na; i++)
	  {
	     b[i] = floor (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_CEIL:
	for (i = 0; i < na; i++)
	  {
	     b[i] = ceil (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
      case SLMATH_ROUND:
	for (i = 0; i < na; i++)
	  {
	     b[i] = ROUND_FUN (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_EXPM1:
	for (i = 0; i < na; i++)
	  {
	     b[i] = EXPM1_FUNC (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_LOG1P:
	for (i = 0; i < na; i++)
	  {
	     b[i] = LOG1P_FUNC (to_double ((VOID_STAR) a));
	     a += da;
	  }
	break;
     }

   return 1;
}

#if SLANG_HAS_COMPLEX
static int complex_math_op_result (int op, SLtype a, SLtype *b)
{
   (void) a;
   switch (op)
     {
      default:
	*b = SLANG_COMPLEX_TYPE;
	break;

      case SLMATH_ISINF:
      case SLMATH_ISNAN:
	*b = SLANG_CHAR_TYPE;
	break;

      case SLMATH_REAL:
      case SLMATH_IMAG:
	*b = SLANG_DOUBLE_TYPE;
	break;
     }
   return 1;
}

static int complex_math_op (int op,
			    SLtype type, VOID_STAR ap, SLuindex_Type na,
			    VOID_STAR bp)
{
   double *a, *b;
   unsigned int i;
   unsigned int na2 = na * 2;
   double *(*fun) (double *, double *);
   char *c;

   (void) type;
   a = (double *) ap;
   b = (double *) bp;

   switch (op)
     {
      default:
	return 0;

      case SLMATH_REAL:
	for (i = 0; i < na; i++)
	  b[i] = a[2 * i];
	return 1;

      case SLMATH_IMAG:
	for (i = 0; i < na; i++)
	  b[i] = a[2 * i + 1];
	return 1;

      case SLMATH_CONJ:
	for (i = 0; i < na2; i += 2)
	  {
	     b[i] = a[i];
	     b[i+1] = -a[i+1];
	  }
	return 1;

      case SLMATH_ATANH:
	fun = SLcomplex_atanh;
	break;
      case SLMATH_ACOSH:
	fun = SLcomplex_acosh;
	break;
      case SLMATH_ASINH:
	fun = SLcomplex_asinh;
	break;
      case SLMATH_EXP:
	fun = SLcomplex_exp;
	break;
      case SLMATH_LOG:
	fun = SLcomplex_log;
	break;
      case SLMATH_LOG10:
	fun = SLcomplex_log10;
	break;
      case SLMATH_SQRT:
	fun = SLcomplex_sqrt;
	break;
      case SLMATH_SIN:
	fun = SLcomplex_sin;
	break;
      case SLMATH_COS:
	fun = SLcomplex_cos;
	break;
      case SLMATH_SINH:
	fun = SLcomplex_sinh;
	break;
      case SLMATH_COSH:
	fun = SLcomplex_cosh;
	break;
      case SLMATH_TANH:
	fun = SLcomplex_tanh;
	break;
      case SLMATH_TAN:
	fun = SLcomplex_tan;
	break;
      case SLMATH_ASIN:
	fun = SLcomplex_asin;
	break;
      case SLMATH_ACOS:
	fun = SLcomplex_acos;
	break;
      case SLMATH_ATAN:
	fun = SLcomplex_atan;
	break;
      case SLMATH_ISINF:
	c = (char *) bp;
	for (i = 0; i < na; i++)
	  {
	     unsigned int j = 2*i;
	     c[i] = (char) (ISINF_FUN(a[j]) || ISINF_FUN(a[j+1]));
	  }
	return 1;
      case SLMATH_ISNAN:
	c = (char *) bp;
	for (i = 0; i < na; i++)
	  {
	     unsigned int j = 2*i;
	     c[i] = (char) (ISNAN_FUN(a[j]) || ISNAN_FUN(a[j+1]));
	  }
	return 1;

      case SLMATH_FLOOR:
      case SLMATH_ROUND:
      case SLMATH_CEIL:
	return double_math_op (op, type, ap, na2, bp);
     }

   for (i = 0; i < na2; i += 2)
     (void) (*fun) (b + i, a + i);

   return 1;
}
#endif

typedef struct
{
   SLang_Array_Type *at;
   int is_float;		       /* if non-zero, use fptr */
   float f;
   double d;
   char c;
   float *fptr;			       /* points at at->data or &f */
   double *dptr;
   char *cptr;
   unsigned int inc;		       /* inc = 0, if at==NULL */
   SLuindex_Type num;
}
Array_Or_Scalar_Type;

static void free_array_or_scalar (Array_Or_Scalar_Type *ast)
{
   if (ast->at != NULL)
     SLang_free_array (ast->at);
}

static int pop_array_or_scalar (Array_Or_Scalar_Type *ast)
{
   SLang_Array_Type *at;

   ast->at = NULL;
   ast->inc = 0;
   ast->num = 1;
   switch (SLang_peek_at_stack1 ())
     {
      case -1:
	return -1;

      case SLANG_FLOAT_TYPE:
	ast->is_float = 1;
	if (SLang_peek_at_stack () == SLANG_ARRAY_TYPE)
	  {
	     if (-1 == SLang_pop_array_of_type (&at, SLANG_FLOAT_TYPE))
	       return -1;
	     ast->fptr = (float *) at->data;
	     ast->inc = 1;
	     ast->num = at->num_elements;
	     ast->at = at;
	     return 0;
	  }

	ast->fptr = &ast->f;
	if (-1 == SLang_pop_float (ast->fptr))
	  return -1;
	return 0;

      default:
	ast->is_float = 0;
	if (SLang_peek_at_stack () == SLANG_ARRAY_TYPE)
	  {
	     if (-1 == SLang_pop_array_of_type (&at, SLANG_DOUBLE_TYPE))
	       return -1;
	     ast->dptr = (double *) at->data;
	     ast->inc = 1;
	     ast->num = at->num_elements;
	     ast->at = at;
	     return 0;
	  }

	ast->dptr = &ast->d;
	if (-1 == SLang_pop_double (ast->dptr))
	  return -1;
	return 0;
     }
}

static int pop_2_arrays_or_scalar (Array_Or_Scalar_Type *a, Array_Or_Scalar_Type *b)
{
   if (-1 == pop_array_or_scalar (b))
     return -1;

   if (-1 == pop_array_or_scalar (a))
     {
	free_array_or_scalar (b);
	return -1;
     }

   if ((a->at != NULL) && (b->at != NULL))
     {
	if (a->num != b->num)
	  {
	     _pSLang_verror (SL_TypeMismatch_Error, "Array sizes do not match");
	     free_array_or_scalar (a);
	     free_array_or_scalar (b);
	     return -1;
	  }
     }
   return 0;
}

static SLang_Array_Type *create_from_tmp_array (SLang_Array_Type *a, SLang_Array_Type *b, SLtype type)
{
   SLang_Array_Type *c;

   if ((a != NULL) && (a->data_type == type) && (a->num_refs == 1))
     {
	a->num_refs += 1;
	return a;
     }
   if ((b != NULL) && (b->data_type == type) && (b->num_refs == 1))
     {
	b->num_refs += 1;
	return b;
     }

   if (a != NULL)
     c = a;
   else
     c = b;

   return SLang_create_array1 (type, 0, NULL, c->dims, c->num_dims, 1);
}

static int do_dd_fun (double (*f)(double, double),
		      Array_Or_Scalar_Type *a_ast,
		      Array_Or_Scalar_Type *b_ast,
		      Array_Or_Scalar_Type *c_ast)
{
   double *a = a_ast->dptr;
   double *b = b_ast->dptr;
   double *c = c_ast->dptr;
   unsigned int a_inc = a_ast->inc;
   unsigned int b_inc = b_ast->inc;
   unsigned int i, n = c_ast->num;

   for (i = 0; i < n; i++)
     {
	c[i] = (*f)(*a, *b);
	a += a_inc;
	b += b_inc;
     }
   return 0;
}

static int do_ff_fun (double (*f)(double, double),
		      Array_Or_Scalar_Type *a_ast,
		      Array_Or_Scalar_Type *b_ast,
		      Array_Or_Scalar_Type *c_ast)
{
   float *a = a_ast->fptr;
   float *b = b_ast->fptr;
   float *c = c_ast->fptr;
   unsigned int a_inc = a_ast->inc;
   unsigned int b_inc = b_ast->inc;
   unsigned int i, n = c_ast->num;

   for (i = 0; i < n; i++)
     {
	c[i] = (float) (*f)(*a, *b);
	a += a_inc;
	b += b_inc;
     }
   return 0;
}

static int do_fd_fun (double (*f)(double, double),
		      Array_Or_Scalar_Type *a_ast,
		      Array_Or_Scalar_Type *b_ast,
		      Array_Or_Scalar_Type *c_ast)
{
   float *a = a_ast->fptr;
   double *b = b_ast->dptr;
   double *c = c_ast->dptr;
   unsigned int a_inc = a_ast->inc;
   unsigned int b_inc = b_ast->inc;
   unsigned int i, n = c_ast->num;

   for (i = 0; i < n; i++)
     {
	c[i] = (*f)(*a, *b);
	a += a_inc;
	b += b_inc;
     }
   return 0;
}

static int do_df_fun (double (*f)(double, double),
		      Array_Or_Scalar_Type *a_ast,
		      Array_Or_Scalar_Type *b_ast,
		      Array_Or_Scalar_Type *c_ast)
{
   double *a = a_ast->dptr;
   float *b = b_ast->fptr;
   double *c = c_ast->dptr;
   unsigned int a_inc = a_ast->inc;
   unsigned int b_inc = b_ast->inc;
   unsigned int i, n = c_ast->num;

   for (i = 0; i < n; i++)
     {
	c[i] = (*f)(*a, *b);
	a += a_inc;
	b += b_inc;
     }
   return 0;
}

static int do_binary_function (double (*f)(double, double))
{
   SLtype type;
   Array_Or_Scalar_Type a_ast, b_ast, c_ast;

   if (-1 == pop_2_arrays_or_scalar (&a_ast, &b_ast))
     return -1;

   c_ast.is_float = (a_ast.is_float && b_ast.is_float);
   c_ast.at = NULL;
   c_ast.num = 1;
   c_ast.inc = 0;
   if (c_ast.is_float)
     {
	type = SLANG_FLOAT_TYPE;
	c_ast.fptr = &c_ast.f;
     }
   else
     {
	type = SLANG_DOUBLE_TYPE;
	c_ast.dptr = &c_ast.d;
     }

   if ((a_ast.at != NULL) || (b_ast.at != NULL))
     {
	if (NULL == (c_ast.at = create_from_tmp_array (a_ast.at, b_ast.at, type)))
	  {
	     free_array_or_scalar (&a_ast);
	     free_array_or_scalar (&b_ast);
	     return -1;
	  }
	c_ast.fptr = (float *) c_ast.at->data;
	c_ast.dptr = (double *) c_ast.at->data;
	c_ast.num = c_ast.at->num_elements;
	c_ast.inc = 1;
     }

   if (a_ast.is_float)
     {
	if (b_ast.is_float)
	  (void) do_ff_fun (f, &a_ast, &b_ast, &c_ast);
	else
	  (void) do_fd_fun (f, &a_ast, &b_ast, &c_ast);
     }
   else if (b_ast.is_float)
     (void) do_df_fun (f, &a_ast, &b_ast, &c_ast);
   else
     (void) do_dd_fun (f, &a_ast, &b_ast, &c_ast);

   free_array_or_scalar (&a_ast);
   free_array_or_scalar (&b_ast);

   if (c_ast.at != NULL)
     return SLang_push_array (c_ast.at, 1);

   if (c_ast.is_float)
     return SLang_push_float (c_ast.f);

   return SLang_push_double (c_ast.d);
}

/* num is assumed to be at least 2 here */
static int do_binary_function_on_nargs (double (*f)(double, double), int num)
{
   int depth = SLstack_depth () - num;

   num--;			       /* first time consumes 2 args */
   while (num > 0)
     {
	if (-1 == do_binary_function (f))
	  {
	     num = SLstack_depth () - depth;
	     if (num > 0)
	       (void) SLdo_pop_n ((unsigned int) num);
	     return -1;
	  }
	num--;
     }
   return 0;
}

static void hypot_fun (void)
{
   Array_Or_Scalar_Type ast;
   unsigned int num;

   if (SLang_Num_Function_Args >= 2)
     {
	(void) do_binary_function_on_nargs (SLmath_hypot, SLang_Num_Function_Args);
	return;
     }

   if (-1 == pop_array_or_scalar (&ast))
     return;

   if (0 == (num = ast.num))
     {
	SLang_verror (SL_InvalidParm_Error, "An empty array was passed to hypot");
	free_array_or_scalar (&ast);
	return;
     }

   if (ast.is_float)
     {
	float *f = ast.fptr;
	double sum, esum, max;
	unsigned int i, imax;

	max = fabs((double)*f);
	imax = 0;
	for (i = 1; i < num; i++)
	  {
	     double max1 = fabs((double)f[i]);
	     if (max1 > max)
	       {
		  imax = i;
		  max  = max1;
	       }
	  }
	sum = 0.0; esum = 0.0;
	if (max > 0.0)
	  {
	     for (i = 0; i < imax; i++)
	       {
		  double term = f[i]/max;
		  double new_sum;

		  term = term*term - esum;
		  new_sum = sum + term;
		  esum = (new_sum - sum) - term;
		  sum = new_sum;
	       }
	     for (i = imax+1; i < num; i++)
	       {
		  double term = f[i]/max;
		  double new_sum;

		  term = term*term - esum;
		  new_sum = sum + term;
		  esum = (new_sum - sum) - term;
		  sum = new_sum;
	       }
	  }
	(void) SLang_push_float ((float) max*sqrt(1.0+sum));
     }
   else
     {
	unsigned int i, imax;
	double *d = ast.dptr;
	double sum, esum, max;
	max = fabs(*d);
	imax = 0;
	for (i = 1; i < num; i++)
	  {
	     double max1 = fabs(d[i]);
	     if (max1 > max)
	       {
		  max  = max1;
		  imax = i;
	       }
	  }
	sum = 0.0; esum = 0.0;
	if (max > 0.0)
	  {
	     for (i = 0; i < imax; i++)
	       {
		  double term = d[i]/max;
		  double new_sum;

		  term = term*term - esum;
		  new_sum = sum + term;
		  esum = (new_sum - sum) - term;
		  sum = new_sum;
	       }
	     for (i = imax+1; i < num; i++)
	       {
		  double term = d[i]/max;
		  double new_sum;

		  term = term*term - esum;
		  new_sum = sum + term;
		  esum = (new_sum - sum) - term;
		  sum = new_sum;
	       }
	  }
	(void) SLang_push_double (max*sqrt(1.0+sum));
     }
   free_array_or_scalar (&ast);
}

static void math_poly (void)
{
   Array_Or_Scalar_Type ast;
   SLang_Array_Type *coeff_at, *y_at;
   SLuindex_Type i, k, n;
   SLuindex_Type num;
   double *a;
   double x, y;
   int use_factorial = 0;

   switch (SLang_Num_Function_Args)
     {
      case 3:
	if (-1 == SLang_pop_int (&use_factorial))
	  return;
	/* drop */
      case 2:
	break;

      default:
	SLang_verror (SL_Usage_Error, "\
Usage: y = polynom([a0,a1,...], x [,use_factorial_form])");
	return;
     }

   if (-1 == pop_array_or_scalar (&ast))
     return;

   if (-1 == SLang_pop_array_of_type (&coeff_at, SLANG_DOUBLE_TYPE))
     {
	free_array_or_scalar (&ast);
	return;
     }
   a = (double *) coeff_at->data;
   n = coeff_at->num_elements;

   if (ast.inc == 0)
     {
	if (ast.is_float)
	  x = (double) ast.f;
	else
	  x = ast.d;

	y = 0.0;
	k = n;

	if (use_factorial)
	  {
	     while (k != 0)
	       {
		  y = a[k-1] + (x/k)*y;
		  k--;
	       }
	  }
	else while (k != 0)
	  {
	     k--;
	     y = a[k] + x*y;
	  }

	if (ast.is_float)
	  (void) SLang_push_float ((float) y);
	else
	  (void) SLang_push_double (y);

	goto free_and_return;
     }

   if (NULL == (y_at = create_from_tmp_array (ast.at, NULL, ast.at->data_type)))
     goto free_and_return;

   num = ast.num;

   if (ast.is_float)
     {
	float *f = ast.fptr;
	float *yf = (float *)y_at->data;

	for (i = 0; i < num; i++)
	  {
	     x = (double) f[i];
	     y = 0.0;
	     k = n;
	     if (use_factorial)
	       {
		  while (k != 0)
		    {
		       y = a[k-1] + (x/k)*y;
		       k--;
		    }
	       }
	     else while (k != 0)
	       {
		  k--;
		  y = a[k] + x*y;
	       }
	     yf[i] = (float) y;
	  }
     }
   else
     {
	double *d = ast.dptr;
	double *yd = (double *)y_at->data;

	for (i = 0; i < num; i++)
	  {
	     x = d[i];
	     y = 0.0;
	     k = n;
	     if (use_factorial)
	       {
		  while (k != 0)
		    {
		       y = a[k-1] + (x/k)*y;
		       k--;
		    }
	       }
	     else while (k != 0)
	       {
		  k--;
		  y = a[k] + x*y;
	       }
	     yd[i] = y;
	  }
     }

   (void) SLang_push_array (y_at, 1);
   /* drop */
free_and_return:
   free_array_or_scalar (&ast);
   SLang_free_array (coeff_at);
}

static void atan2_fun (void)
{
   (void) do_binary_function (atan2);
}

static double do_min (double a, double b)
{
   if ((a >= b) || ISNAN_FUN(a))
     return b;
   return a;
}
static double do_max (double a, double b)
{
   if ((a <= b) || ISNAN_FUN(a))
     return b;
   return a;
}
static double do_diff (double a, double b)
{
   return fabs(a-b);
}

static void min_fun (void)
{
   int num = SLang_Num_Function_Args;

   if (num <= 0)
     {
	SLang_verror (SL_Usage_Error, "_min: Expecting at least one argument");
	return;
     }

   if (num > 1)
     (void) do_binary_function_on_nargs (do_min, num);
}

static void max_fun (void)
{
   int num = SLang_Num_Function_Args;

   if (num <= 0)
     {
	SLang_verror (SL_Usage_Error, "_max: Expecting at least one argument");
	return;
     }

   if (num > 1)
     (void) do_binary_function_on_nargs (do_max, num);
}

static void diff_fun (void)
{
   (void) do_binary_function (do_diff);
}

#if 1

static int do_c_dd_fun (int (*f)(double, double, VOID_STAR),
			VOID_STAR cd,
			Array_Or_Scalar_Type *a_ast,
			Array_Or_Scalar_Type *b_ast,
			Array_Or_Scalar_Type *c_ast)
{
   double *a = a_ast->dptr;
   double *b = b_ast->dptr;
   char *c = c_ast->cptr;
   unsigned int a_inc = a_ast->inc;
   unsigned int b_inc = b_ast->inc;
   unsigned int i, n = c_ast->num;

   for (i = 0; i < n; i++)
     {
	c[i] = (char) (*f)(*a, *b, cd);
	a += a_inc;
	b += b_inc;
     }
   return 0;
}

static int do_c_ff_fun (int (*f)(double, double, VOID_STAR),
			VOID_STAR cd,
			Array_Or_Scalar_Type *a_ast,
			Array_Or_Scalar_Type *b_ast,
			Array_Or_Scalar_Type *c_ast)
{
   float *a = a_ast->fptr;
   float *b = b_ast->fptr;
   char *c = c_ast->cptr;
   unsigned int a_inc = a_ast->inc;
   unsigned int b_inc = b_ast->inc;
   unsigned int i, n = c_ast->num;

   for (i = 0; i < n; i++)
     {
	c[i] = (char) (*f)(*a, *b, cd);
	a += a_inc;
	b += b_inc;
     }
   return 0;
}

static int do_c_fd_fun (int (*f)(double, double, VOID_STAR),
			VOID_STAR cd,
			Array_Or_Scalar_Type *a_ast,
			Array_Or_Scalar_Type *b_ast,
			Array_Or_Scalar_Type *c_ast)
{
   float *a = a_ast->fptr;
   double *b = b_ast->dptr;
   char *c = c_ast->cptr;
   unsigned int a_inc = a_ast->inc;
   unsigned int b_inc = b_ast->inc;
   unsigned int i, n = c_ast->num;

   for (i = 0; i < n; i++)
     {
	c[i] = (char) (*f)(*a, *b, cd);
	a += a_inc;
	b += b_inc;
     }
   return 0;
}

static int do_c_df_fun (int (*f)(double, double, VOID_STAR),
			VOID_STAR cd,
			Array_Or_Scalar_Type *a_ast,
			Array_Or_Scalar_Type *b_ast,
			Array_Or_Scalar_Type *c_ast)
{
   double *a = a_ast->dptr;
   float *b = b_ast->fptr;
   char *c = c_ast->cptr;
   unsigned int a_inc = a_ast->inc;
   unsigned int b_inc = b_ast->inc;
   unsigned int i, n = c_ast->num;

   for (i = 0; i < n; i++)
     {
	c[i] = (char) (*f)(*a, *b, cd);
	a += a_inc;
	b += b_inc;
     }
   return 0;
}

static int do_binary_function_c (int (*f)(double, double,VOID_STAR), VOID_STAR cd)
{
   Array_Or_Scalar_Type a_ast, b_ast, c_ast;

   if (-1 == pop_2_arrays_or_scalar (&a_ast, &b_ast))
     return -1;

   c_ast.at = NULL;
   c_ast.num = 1;
   c_ast.inc = 0;
   c_ast.cptr = &c_ast.c;

   if ((a_ast.at != NULL) || (b_ast.at != NULL))
     {
	if (a_ast.at != NULL)
	  c_ast.at = SLang_create_array1 (SLANG_CHAR_TYPE, 0, NULL, a_ast.at->dims, a_ast.at->num_dims, 1);
	else
	  c_ast.at = SLang_create_array1 (SLANG_CHAR_TYPE, 0, NULL, b_ast.at->dims, b_ast.at->num_dims, 1);

	if (c_ast.at == NULL)
	  {
	     free_array_or_scalar (&a_ast);
	     free_array_or_scalar (&b_ast);
	     return -1;
	  }
	c_ast.cptr = (char *) c_ast.at->data;
	c_ast.num = c_ast.at->num_elements;
	c_ast.inc = 1;
     }

   if (a_ast.is_float)
     {
	if (b_ast.is_float)
	  (void) do_c_ff_fun (f, cd, &a_ast, &b_ast, &c_ast);
	else
	  (void) do_c_fd_fun (f, cd, &a_ast, &b_ast, &c_ast);
     }
   else if (b_ast.is_float)
     (void) do_c_df_fun (f, cd, &a_ast, &b_ast, &c_ast);
   else
     (void) do_c_dd_fun (f, cd, &a_ast, &b_ast, &c_ast);

   free_array_or_scalar (&a_ast);
   free_array_or_scalar (&b_ast);

   if (c_ast.at != NULL)
     return SLang_push_array (c_ast.at, 1);

   return SLang_push_char (c_ast.c);
}

typedef struct
{
   double relerr;
   double abserr;
}
Feqs_Err_Type;

static int get_tolorances (int nargs, Feqs_Err_Type *ep)
{
   switch (nargs)
     {
      case 2:
	if ((-1 == SLang_pop_double (&ep->abserr))
	    || (-1 == SLang_pop_double (&ep->relerr)))
	  return -1;
	break;

      case 1:
	if (-1 == SLang_pop_double (&ep->relerr))
	  return -1;
	ep->abserr = 0.0;
	break;

      default:
	ep->relerr = 0.01;
	ep->abserr = 1e-6;
	break;
     }
   return 0;
}

static int do_feqs (double a, double b, VOID_STAR cd)
{
   Feqs_Err_Type *e = (Feqs_Err_Type *)cd;

   if (fabs (a-b) <= e->abserr)
     return 1;

   if (fabs(a) > fabs(b))
     return fabs ((a-b)/a) <= e->relerr;

   return fabs ((b-a)/b) <= e->relerr;
}

static int do_fneqs (double a, double b, VOID_STAR cd)
{
   return !do_feqs (a, b, cd);
}

static int do_fleqs (double a, double b, VOID_STAR cd)
{
   return (a < b) || do_feqs (a, b, cd);
}

static int do_fgeqs (double a, double b, VOID_STAR cd)
{
   return (a > b) || do_feqs (a, b, cd);
}

static void do_an_feqs_fun (int (*f)(double, double, VOID_STAR))
{
   Feqs_Err_Type e;
   if (-1 == get_tolorances (SLang_Num_Function_Args-2, &e))
     return;

   do_binary_function_c (f, (VOID_STAR)&e);
}

static void feqs_fun (void)
{
   do_an_feqs_fun (do_feqs);
}

static void fgeqs_fun (void)
{
   do_an_feqs_fun (do_fgeqs);
}

static void fleqs_fun (void)
{
   do_an_feqs_fun (do_fleqs);
}

static void fneqs_fun (void)
{
   do_an_feqs_fun (do_fneqs);
}
#endif

static int do_nint (double x)
{
   double xf, xi;

   xf = modf (x, &xi);		       /* x = xi + xf */
   if (x >= 0)
     {
	if (xf >= 0.5)
	  return xi + 1;
     }
   else
     {
	if (xf <= -0.5)
	  return xi - 1;
     }
   return xi;
}

static int float_to_nint (SLang_Array_Type *at, SLang_Array_Type *bt)
{
   unsigned int n, i;
   int *ip;
   float *fp;

   fp = (float *) at->data;
   ip = (int *) bt->data;
   n = at->num_elements;

   for (i = 0; i < n; i++)
     ip[i] = do_nint ((double) fp[i]);

   return 0;
}

static int double_to_nint (SLang_Array_Type *at, SLang_Array_Type *bt)
{
   unsigned int n, i;
   int *ip;
   double *dp;

   dp = (double *) at->data;
   ip = (int *) bt->data;
   n = at->num_elements;

   for (i = 0; i < n; i++)
     ip[i] = do_nint (dp[i]);

   return 0;
}

static void nint_intrin (void)
{
   double x;
   SLang_Array_Type *at, *bt;
   int (*at_to_int_fun)(SLang_Array_Type *, SLang_Array_Type *);

   if (SLang_peek_at_stack () != SLANG_ARRAY_TYPE)
     {
	if (-1 == SLang_pop_double (&x))
	  return;
	(void) SLang_push_int (do_nint (x));
	return;
     }
   switch (SLang_peek_at_stack1 ())
     {
      case -1:
	return;

      case SLANG_INT_TYPE:
	return;

      case SLANG_FLOAT_TYPE:
	if (-1 == SLang_pop_array_of_type (&at, SLANG_FLOAT_TYPE))
	  return;
	at_to_int_fun = float_to_nint;
	break;

      case SLANG_DOUBLE_TYPE:
      default:
	if (-1 == SLang_pop_array_of_type (&at, SLANG_DOUBLE_TYPE))
	  return;
	at_to_int_fun = double_to_nint;
	break;
     }

   if (NULL == (bt = SLang_create_array1 (SLANG_INT_TYPE, 0, NULL, at->dims, at->num_dims, 1)))
     {
	SLang_free_array (at);
	return;
     }
   if (0 == (*at_to_int_fun) (at, bt))
     (void) SLang_push_array (bt, 0);

   SLang_free_array (bt);
   SLang_free_array (at);
}

#ifdef HAVE_FREXP
# define FREXP_FUN(x,e) frexp(x,e)
# ifdef HAVE_FREXPF
#  define FREXPF_FUN(x,e) frexpf(x,e)
# else
#  define FREXPF_FUN(x,e) (float)FREXP_FUN(x,e)
# endif

static void frexp_intrin (void)
{
   double d;
   float f;
   int e, *ep;
   SLuindex_Type i, imax;
   SLang_Array_Type *at, *bt, *et;

   switch (SLang_peek_at_stack ())
     {
      case SLANG_ARRAY_TYPE:
	break;

      case SLANG_FLOAT_TYPE:
	if (-1 == SLang_pop_float (&f))
	  return;
	f = FREXPF_FUN(f, &e);
	(void) SLang_push_float (f);
	(void) SLang_push_int (e);
	return;

      default:
      case SLANG_DOUBLE_TYPE:
	if (-1 == SLang_pop_double (&d))
	  return;
	d = FREXP_FUN(d, &e);
	(void) SLang_push_double (d);
	(void) SLang_push_int (e);
	return;
     }

   switch (SLang_peek_at_stack1 ())
     {
      case SLANG_FLOAT_TYPE:
	if (-1 == SLang_pop_array_of_type (&at, SLANG_FLOAT_TYPE))
	  return;
	break;

      default:
      case SLANG_DOUBLE_TYPE:
	if (-1 == SLang_pop_array_of_type (&at, SLANG_DOUBLE_TYPE))
	  return;
	break;
     }

   if (NULL == (bt = SLang_create_array1 (at->data_type, 0, NULL, at->dims, at->num_dims, 1)))
     {
	SLang_free_array (at);
	return;
     }
   if (NULL == (et = SLang_create_array1 (SLANG_INT_TYPE, 0, NULL, at->dims, at->num_dims, 1)))
     {
	SLang_free_array (at);
	SLang_free_array (bt);
	return;
     }

   imax = at->num_elements;
   ep = (int *)et->data;

   if (at->data_type == SLANG_DOUBLE_TYPE)
     {
	double *a = (double *)at->data;
	double *b = (double *)bt->data;
	for (i = 0; i < imax; i++)
	  {
	     b[i] = FREXP_FUN(a[i], ep+i);
	  }
     }
   else
     {
	float *a = (float *)at->data;
	float *b = (float *)bt->data;
	for (i = 0; i < imax; i++)
	  {
	     b[i] = FREXPF_FUN(a[i], ep+i);
	  }
     }

   (void) SLang_push_array (bt, 0);
   (void) SLang_push_array (et, 0);
   SLang_free_array (et);
   SLang_free_array (bt);
   SLang_free_array (at);
}
#endif				       /* HAVE_FREXP */

#ifdef HAVE_LDEXP
# define LDEXP_FUN(a,b) ldexp(a,b)
# ifdef HAVE_LDEXPF
#  define LDEXPF_FUN(a,b) ldexpf(a,b)
# else
#  define LDEXPF_FUN(a,b) (float)LDEXP_FUN(a,b)
# endif
static void ldexp_intrin (void)
{
   Array_Or_Scalar_Type ast;
   SLang_Array_Type *e_at = NULL, *c_at;
   int *e_ptr, e_data;
   SLuindex_Type i, imax;

   if (SLang_peek_at_stack () == SLANG_ARRAY_TYPE)
     {
	if (-1 == SLang_pop_array_of_type (&e_at, SLANG_INT_TYPE))
	  return;
	e_ptr = (int *)e_at->data;
     }
   else
     {
	if (-1 == SLang_pop_int (&e_data))
	  return;
	e_ptr = &e_data;
     }

   if (-1 == pop_array_or_scalar (&ast))
     {
	if (e_at != NULL)
	  SLang_free_array (e_at);
	return;
     }

   if ((e_at == NULL) && (ast.at == NULL))
     {
	/* Scalar case */
	if (ast.is_float)
	  (void) SLang_push_float (LDEXPF_FUN(ast.f, *e_ptr));
	else
	  (void) SLang_push_double (LDEXP_FUN(ast.d, *e_ptr));
	/* free_array_or_scalar (&ast);   Nothing to free */
	return;
     }

   if (NULL == (c_at = create_from_tmp_array (ast.at, e_at, ast.is_float ? SLANG_FLOAT_TYPE : SLANG_DOUBLE_TYPE)))
     {
	free_array_or_scalar (&ast);
	SLang_free_array (e_at);
	return;
     }

   if (e_at == NULL)
     {
	int e = *e_ptr;
	imax = ast.num;

	if (ast.is_float)
	  {
	     float *c, *a;
	     a = ast.fptr;
	     c = (float *)c_at->data;
	     for (i = 0; i < imax; i++)
	       c[i] = LDEXPF_FUN(a[i], e);
	  }
	else
	  {
	     double *c, *a;
	     a = ast.dptr;
	     c = (double *)c_at->data;
	     for (i = 0; i < imax; i++)
	       c[i] = LDEXP_FUN(a[i], e);
	  }
	goto push_free_and_return;
     }

   if (ast.at == NULL)
     {
	imax = e_at->num_elements;

	if (ast.is_float)
	  {
	     float *c, a;
	     a = ast.f;
	     c = (float *)c_at->data;
	     for (i = 0; i < imax; i++)
	       c[i] = LDEXPF_FUN(a, e_ptr[i]);
	  }
	else
	  {
	     double *c, a;
	     a = ast.d;
	     c = (double *)c_at->data;
	     for (i = 0; i < imax; i++)
	       c[i] = LDEXP_FUN(a, e_ptr[i]);
	  }
	goto push_free_and_return;
     }

   if (e_at->num_elements != ast.num)
     {
	SLang_verror (SL_TypeMismatch_Error, "ldexp: Array sizes do not match");
	goto free_and_return;
     }

   imax = ast.num;
   if (ast.is_float)
     {
	float *c = (float *)c_at->data;
	float *a = ast.fptr;
	for (i = 0; i < imax; i++)
	  c[i] = LDEXPF_FUN(a[i], e_ptr[i]);
     }
   else
     {
	double *c = (double *)c_at->data;
	double *a = ast.dptr;
	for (i = 0; i < imax; i++)
	  c[i] = LDEXP_FUN(a[i], e_ptr[i]);
     }
   /* drop */
push_free_and_return:
   (void) SLang_push_array (c_at, 0);
   /* drop */
free_and_return:
   if (e_at != NULL) SLang_free_array (e_at);
   SLang_free_array (c_at);
   free_array_or_scalar (&ast);
}
#endif				       /* HAVE_LDEXPF */

static void fpu_clear_except_bits (void)
{
   SLfpu_clear_except_bits ();
}

static int fpu_test_except_bits (int *bits)
{
   return (int) SLfpu_test_except_bits ((unsigned int) *bits);
}

static SLang_DConstant_Type DConst_Table [] =
{
   MAKE_DCONSTANT("E", 2.718281828459045),
   MAKE_DCONSTANT("PI", PI),
   SLANG_END_DCONST_TABLE
};

static SLang_Math_Unary_Type SLmath_Table [] =
{
   MAKE_MATH_UNARY("sinh", SLMATH_SINH),
   MAKE_MATH_UNARY("asinh", SLMATH_ASINH),
   MAKE_MATH_UNARY("cosh", SLMATH_COSH),
   MAKE_MATH_UNARY("acosh", SLMATH_ACOSH),
   MAKE_MATH_UNARY("tanh", SLMATH_TANH),
   MAKE_MATH_UNARY("atanh", SLMATH_ATANH),
   MAKE_MATH_UNARY("sin", SLMATH_SIN),
   MAKE_MATH_UNARY("cos", SLMATH_COS),
   MAKE_MATH_UNARY("tan", SLMATH_TAN),
   MAKE_MATH_UNARY("atan", SLMATH_ATAN),
   MAKE_MATH_UNARY("acos", SLMATH_ACOS),
   MAKE_MATH_UNARY("asin", SLMATH_ASIN),
   MAKE_MATH_UNARY("exp", SLMATH_EXP),
   MAKE_MATH_UNARY("log", SLMATH_LOG),
   MAKE_MATH_UNARY("sqrt", SLMATH_SQRT),
   MAKE_MATH_UNARY("log10", SLMATH_LOG10),
   MAKE_MATH_UNARY("isinf", SLMATH_ISINF),
   MAKE_MATH_UNARY("isnan", SLMATH_ISNAN),
   MAKE_MATH_UNARY("floor", SLMATH_FLOOR),
   MAKE_MATH_UNARY("ceil", SLMATH_CEIL),
   MAKE_MATH_UNARY("round", SLMATH_ROUND),
   MAKE_MATH_UNARY("expm1", SLMATH_EXPM1),
   MAKE_MATH_UNARY("log1p", SLMATH_LOG1P),

#if SLANG_HAS_COMPLEX
   MAKE_MATH_UNARY("Real", SLMATH_REAL),
   MAKE_MATH_UNARY("Imag", SLMATH_IMAG),
   MAKE_MATH_UNARY("Conj", SLMATH_CONJ),
#endif
   SLANG_END_MATH_UNARY_TABLE
};

static SLang_Intrin_Fun_Type SLang_Math_Table [] =
{
   MAKE_INTRINSIC_0("nint", nint_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("polynom", math_poly, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("hypot", hypot_fun, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("atan2", atan2_fun, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_min", min_fun, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_max", max_fun, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_diff", diff_fun, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("feqs", feqs_fun, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("fneqs", fneqs_fun, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("flteqs", fleqs_fun, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("fgteqs", fgeqs_fun, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("fpu_clear_except_bits", fpu_clear_except_bits, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_1("fpu_test_except_bits", fpu_test_except_bits, _pSLANG_LONG_TYPE, _pSLANG_LONG_TYPE),
#ifdef HAVE_FREXP
   MAKE_INTRINSIC_0("frexp", frexp_intrin, SLANG_VOID_TYPE),
#endif
#ifdef HAVE_LDEXP
   MAKE_INTRINSIC_0("ldexp", ldexp_intrin, SLANG_VOID_TYPE),
#endif
   SLANG_END_INTRIN_FUN_TABLE
};

static SLang_IConstant_Type IConsts [] =
{
   MAKE_ICONSTANT("FE_DIVBYZERO", SL_FE_DIVBYZERO),
   MAKE_ICONSTANT("FE_INVALID", SL_FE_INVALID),
   MAKE_ICONSTANT("FE_OVERFLOW", SL_FE_OVERFLOW),
   MAKE_ICONSTANT("FE_UNDERFLOW", SL_FE_UNDERFLOW),
   MAKE_ICONSTANT("FE_INEXACT", SL_FE_INEXACT),
   MAKE_ICONSTANT("FE_ALL_EXCEPT", SL_FE_ALLEXCEPT),
   SLANG_END_ICONST_TABLE
};

static int add_nan_and_inf (void)
{
   if ((-1 == SLns_add_dconstant (NULL, "_NaN", _pSLang_NaN))
       || (-1 == SLns_add_dconstant (NULL, "_Inf", _pSLang_Inf)))
     return -1;

   SLfpu_clear_except_bits ();

   return 0;
}

int SLang_init_slmath (void)
{
   SLtype *int_types;

#if SLANG_HAS_COMPLEX
   if (-1 == _pSLinit_slcomplex ())
     return -1;
#endif
   int_types = _pSLarith_Arith_Types;

   while (*int_types != SLANG_FLOAT_TYPE)
     {
	if (-1 == SLclass_add_math_op (*int_types, generic_math_op, double_math_op_result))
	  return -1;
	int_types++;
     }

   if ((-1 == SLclass_add_math_op (SLANG_FLOAT_TYPE, float_math_op, double_math_op_result))
       || (-1 == SLclass_add_math_op (SLANG_DOUBLE_TYPE, double_math_op, double_math_op_result))
#if SLANG_HAS_COMPLEX
       || (-1 == SLclass_add_math_op (SLANG_COMPLEX_TYPE, complex_math_op, complex_math_op_result))
#endif
       )
     return -1;

   if ((-1 == SLadd_math_unary_table (SLmath_Table, "__SLMATH__"))
       || (-1 == SLadd_intrin_fun_table (SLang_Math_Table, NULL))
       || (-1 == SLadd_dconstant_table (DConst_Table, NULL))
       || (-1 == SLadd_iconstant_table (IConsts, NULL))
       || (-1 == add_nan_and_inf ()))
     return -1;

#if defined(__unix__)
   (void) SLsignal (SIGFPE, math_floating_point_exception);
#endif

   return 0;
}
#endif				       /* SLANG_HAS_FLOAT */
