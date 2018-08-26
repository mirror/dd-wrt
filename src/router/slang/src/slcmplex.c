/* Complex Data Type definition for S-Lang */
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

#include "slang.h"
#include "_slang.h"

/* The rest of the file is enclosed in this #if */
#if SLANG_HAS_COMPLEX

#if SLANG_HAS_FLOAT
# include <math.h>
#endif

#ifdef PI
# undef PI
#endif
#define PI 3.14159265358979323846

int SLang_pop_complex (double *r, double *i)
{
   double *c;

   switch (SLang_peek_at_stack ())
     {
      case SLANG_COMPLEX_TYPE:
	if (-1 == SLclass_pop_ptr_obj (SLANG_COMPLEX_TYPE, VOID_STAR_STAR(&c)))
	  return -1;
	*r = c[0];
	*i = c[1];
	SLfree ((char *) c);
	break;

      default:
	*i = 0.0;
	if (-1 == SLang_pop_double (r))
	  return -1;
	break;

      case -1:
	return -1;
     }
   return 0;
}

int SLang_push_complex (double r, double i)
{
   double *c;

   c = (double *) SLmalloc (2 * sizeof (double));
   if (c == NULL)
     return -1;

   c[0] = r;
   c[1] = i;

   if (-1 == SLclass_push_ptr_obj (SLANG_COMPLEX_TYPE, (VOID_STAR) c))
     {
	SLfree ((char *) c);
	return -1;
     }
   return 0;
}

double *SLcomplex_times (double *c, double *a, double *b)
{
   double a_real, b_real, a_imag, b_imag;

   a_real = a[0];
   b_real = b[0];
   a_imag = a[1];
   b_imag = b[1];

   c[0] = a_real * b_real - a_imag * b_imag;
   c[1] = a_imag * b_real + a_real * b_imag;

   return c;
}

double *SLcomplex_divide (double *c, double *a, double *b)
{
   double a_real, b_real, a_imag, b_imag;
   double ratio, invden;

   a_real = a[0];
   b_real = b[0];
   a_imag = a[1];
   b_imag = b[1];

   /* Do it this way to avoid overflow in the denom */
   if (fabs(b_real) > fabs(b_imag))
     {
	ratio = b_imag / b_real;
	invden = 1.0 / (b_real + b_imag * ratio);
	c[0] = (a_real + ratio * a_imag) * invden;
	c[1] = (a_imag - a_real * ratio) * invden;
     }
   else
     {
	ratio = b_real / b_imag;
	invden = 1.0 / (b_real * ratio + b_imag);
	c[0] = (a_real * ratio + a_imag) * invden;
	c[1] = (a_imag * ratio - a_real) * invden;
     }
   return c;
}

/* a^b = exp (b log a); */
double *SLcomplex_pow (double *c, double *a, double *b)
{
   if ((a[0] == 0.0) && (b[0] == 0.0) && (a[1] == 0.0) && (b[1] == 0.0))
     {
	c[0] = 1.0;
	c[1] = 0.0;
	return c;
     }
   return SLcomplex_exp (c, SLcomplex_times (c, b, SLcomplex_log (c, a)));
}

static double *complex_dpow (double *c, double *a, double b)
{
   if ((b == 0.0) && (a[0] == 0.0) && (a[1] == 0.0))
     {
	c[0] = 1.0;
	c[1] = 0.0;
	return c;
     }
   SLcomplex_log (c, a);
   c[0] *= b;
   c[1] *= b;
   return SLcomplex_exp (c, c);
}

static double *dcomplex_pow (double *c, double a, double *b)
{
   if ((a == 0.0) && (b[0] == 0.0) && (b[1] == 0.0))
     {
	c[0] = 1.0;
	c[1] = 0.0;
	return c;
     }
   a = log (a);
   c[0] = a * b[0];
   c[1] = a * b[1];
   return SLcomplex_exp (c, c);
}

double SLcomplex_abs (double *z)
{
   return SLmath_hypot (z[0], z[1]);
}

/* It appears that FORTRAN assumes that the branch cut for the log function
 * is along the -x axis.  So, use this for atan2:
 */
static double my_atan2 (double y, double x)
{
   double val;

   val = atan (y/x);

   if (x >= 0)
     return val;		       /* I, IV */

   if (y <= 0)			       /* III */
     return val - PI;

   return PI + val;		       /* II */
}

static void polar_form (double *r, double *theta, double *z)
{
   double x, y;

   *r = SLcomplex_abs (z);

   x = z[0];
   y = z[1];

   if (x == 0.0)
     {
	if (y >= 0)
	  *theta = 0.5 * PI;
	else
	  *theta = 1.5 * PI;
     }
   else *theta = my_atan2 (y, x);
}

double *SLcomplex_sin (double *sinz, double *z)
{
   double x, y;

   x = z[0]; y = z[1];
   sinz[0] = sin (x) * cosh (y);
   sinz[1] = cos (x) * sinh (y);
   return sinz;
}

double *SLcomplex_cos (double *cosz, double *z)
{
   double x, y;

   x = z[0]; y = z[1];
   cosz[0] = cos (x) * cosh (y);
   cosz[1] = -sin (x) * sinh (y);
   return cosz;
}

double *_pSLcomplex_expm1 (double *f, double *z)
{
   /* exp(z)-1 = (e^x-1) + (cos(y)-1)*e^x + ie^x*sin(y) */
   double ex;
   double x = z[0], y = z[1];
   double siny2 = sin(0.5*y);

   ex = exp(x);
   f[0] = EXPM1_FUNC(x) - ex*(2.0*siny2*siny2);   /* cos(y)-1 = 2*sin^2(y/2) */
   f[1] = ex*sin(y);
   return f;
}

double *SLcomplex_exp (double *expz, double *z)
{
   double r, i;

   r = exp (z[0]);
   i = z[1];
   expz[0] = r * cos (i);
   expz[1] = r * sin (i);
   return expz;
}

double *SLcomplex_log (double *logz, double *z)
{
   double r, theta;

   polar_form (&r, &theta, z);	       /* log R.e^(ix) = log R + ix */
   logz[0] = log(r);
   logz[1] = theta;
   return logz;
}

double *_pSLcomplex_log1p (double *f, double *z)
{
   /* f(z) = log(1+z)
    *      = log(1+x+iy)
    *      = log(sqrt((1+x)^2 + y^2)) + i*theta
    *      = 0.5*log((1+x)^2 + y^2) + i*theta
    *      = 0.5*log(1 + 2*x + x^2 + y^2) + i*theta
    *      = 0.5*log(1 + r^2 + 2*x) + i*theta
    */
   double r, theta;

   polar_form (&r, &theta, z);
   if (r < 1)
     f[0] = 0.5*LOG1P_FUNC(r*r + 2.0*z[0]);
   else
     f[0] = log(SLmath_hypot(1.0+z[0], z[1]));

   f[1] = theta;
   return f;
}

double *SLcomplex_log10 (double *log10z, double *z)
{
   double l10 = log (10.0);
   (void) SLcomplex_log (log10z, z);
   log10z[0] = log10z[0] / l10;
   log10z[1] = log10z[1] / l10;
   return log10z;
}

double *SLcomplex_sqrt (double *sqrtz, double *z)
{
   double r, x, y;

   x = z[0];
   y = z[1];

   r = SLmath_hypot (x, y);

   if (r == 0.0)
     {
	sqrtz [0] = sqrtz [1] = 0.0;
	return sqrtz;
     }

   if (x >= 0.0)
     {
	x = sqrt (0.5 * (r + x));
	y = 0.5 * y / x;
     }
   else
     {
	r = sqrt (0.5 * (r - x));
	x = 0.5 * y / r;
	y = r;

	if (x < 0.0)
	  {
	     x = -x;
	     y = -y;
	  }
     }

   sqrtz[0] = x;
   sqrtz[1] = y;

   return sqrtz;
}

double *SLcomplex_tan (double *tanz, double *z)
{
   double x, y, invden;

   x = 2 * z[0];
   y = 2 * z[1];
   invden = 1.0 / (cos (x) + cosh (y));
   tanz[0] = invden * sin (x);
   tanz[1] = invden * sinh (y);
   return tanz;
}

/* Utility Function */
static void compute_alpha_beta (double *z, double *alpha, double *beta)
{
   double x, y, a, b;

   x = z[0];
   y = z[1];
   a = 0.5 * SLmath_hypot (x + 1, y);
   b = 0.5 * SLmath_hypot (x - 1, y);

   *alpha = a + b;
   *beta = a - b;
}

double *SLcomplex_asin (double *asinz, double *z)
{
   double alpha, beta;

   compute_alpha_beta (z, &alpha, &beta);
   asinz[0] = asin (beta);
   asinz[1] = log (alpha + sqrt (alpha * alpha - 1));
   return asinz;
}

double *SLcomplex_acos (double *acosz, double *z)
{
   double alpha, beta;

   compute_alpha_beta (z, &alpha, &beta);
   acosz[0] = acos (beta);
   acosz[1] = -log (alpha + sqrt (alpha * alpha - 1));
   return acosz;
}

double *SLcomplex_atan (double *atanz, double *z)
{
   double x, y;
   double z1[2], z2[2];

   x = z[0]; y = z[1];
   z1[0] = x;
   z1[1] = 1 + y;
   z2[0] = -x;
   z2[1] = 1 - y;

   SLcomplex_log (z1, SLcomplex_divide (z2, z1, z2));
   atanz[0] = -0.5 * z1[1];
   atanz[1] = 0.5 * z1[0];

   return atanz;
}

double *SLcomplex_sinh (double *sinhz, double *z)
{
   double x, y;
   x = z[0]; y = z[1];
   sinhz[0] = sinh (x) * cos (y);
   sinhz[1] = cosh (x) * sin (y);
   return sinhz;
}

double *SLcomplex_cosh (double *coshz, double *z)
{
   double x, y;
   x = z[0]; y = z[1];
   coshz[0] = cosh (x) * cos (y);
   coshz[1] = sinh (x) * sin (y);
   return coshz;
}

double *SLcomplex_tanh (double *tanhz, double *z)
{
   double x, y, invden;
   x = 2 * z[0];
   y = 2 * z[1];
   invden = 1.0 / (cosh (x) + cos (y));
   tanhz[0] = invden * sinh (x);
   tanhz[1] = invden * sin (y);
   return tanhz;
}
#if 0
static double *not_implemented (char *fun, double *p)
{
   _pSLang_verror (SL_NOT_IMPLEMENTED, "%s for complex numbers has not been implemented",
		 fun);
   *p = -1.0;
   return p;
}
#endif
/* Use: asinh(z) = -i asin(iz) */
double *SLcomplex_asinh (double *asinhz, double *z)
{
   double iz[2];

   iz[0] = -z[1];
   iz[1] = z[0];

   (void) SLcomplex_asin (iz, iz);
   asinhz[0] = iz[1];
   asinhz[1] = -iz[0];

   return asinhz;
}

/* Use: acosh (z) = i acos(z) */
double *SLcomplex_acosh (double *acoshz, double *z)
{
   double iz[2];

   (void) SLcomplex_acos (iz, z);
   acoshz[0] = -iz[1];
   acoshz[1] = iz[0];

   return acoshz;
}

/* Use: atanh(z) = -i atan(iz) */
double *SLcomplex_atanh (double *atanhz, double *z)
{
   double iz[2];

   iz[0] = -z[1];
   iz[1] = z[0];

   (void) SLcomplex_atan (iz, iz);
   atanhz[0] = iz[1];
   atanhz[1] = -iz[0];

   return atanhz;
}

static int complex_binary_result (int op, SLtype a, SLtype b,
				  SLtype *c)
{
   (void) a; (void) b;

   switch (op)
     {
      default:
      case SLANG_POW:
      case SLANG_PLUS:
      case SLANG_MINUS:
      case SLANG_TIMES:
      case SLANG_DIVIDE:
	*c = SLANG_COMPLEX_TYPE;
	break;

      case SLANG_EQ:
      case SLANG_NE:
	*c = SLANG_CHAR_TYPE;
	break;
     }
   return 1;
}

static int complex_complex_binary (int op,
				   SLtype a_type, VOID_STAR ap, SLuindex_Type na,
				   SLtype b_type, VOID_STAR bp, SLuindex_Type nb,
				   VOID_STAR cp)
{
   char *ic;
   double *a, *b, *c;
   SLuindex_Type n, n_max;
   SLuindex_Type da, db;

   (void) a_type;
   (void) b_type;

   a = (double *) ap;
   b = (double *) bp;
   c = (double *) cp;
   ic = (char *) cp;

   if (na == 1) da = 0; else da = 2;
   if (nb == 1) db = 0; else db = 2;

   if (na > nb) n_max = na; else n_max = nb;
   n_max = 2 * n_max;

   switch (op)
     {
      default:
	return 0;

      case SLANG_PLUS:
	for (n = 0; n < n_max; n += 2)
	  {
	     c[n] = a[0] + b[0];
	     c[n + 1] = a[1] + b[1];
	     a += da; b += db;
	  }
	break;

      case SLANG_MINUS:
	for (n = 0; n < n_max; n += 2)
	  {
	     c[n] = a[0] - b[0];
	     c[n + 1] = a[1] - b[1];
	     a += da; b += db;
	  }
	break;

      case SLANG_TIMES:
	for (n = 0; n < n_max; n += 2)
	  {
	     SLcomplex_times (c + n, a, b);
	     a += da; b += db;
	  }
	break;

      case SLANG_DIVIDE:	       /* / */
	for (n = 0; n < n_max; n += 2)
	  {
#if 0
	     if ((b[0] == 0.0) && (b[1] == 0.0))
	       {
		  SLang_set_error (SL_DIVIDE_ERROR);
		  return -1;
	       }
#endif
	     SLcomplex_divide (c + n, a, b);
	     a += da; b += db;
	  }
	break;

      case SLANG_EQ: 		       /* == */
	for (n = 0; n < n_max; n += 2)
	  {
	     ic[n/2] = ((a[0] == b[0]) && (a[1] == b[1]));
	     a += da; b += db;
	  }
	break;

      case SLANG_NE:		       /* != */
	for (n = 0; n < n_max; n += 2)
	  {
	     ic[n/2] = ((a[0] != b[0]) || (a[1] != b[1]));
	     a += da; b += db;
	  }
	break;

      case SLANG_POW:
	for (n = 0; n < n_max; n += 2)
	  {
	     SLcomplex_pow (c + n, a, b);
	     a += da; b += db;
	  }
	break;

     }

   return 1;
}

static int complex_double_binary (int op,
				  SLtype a_type, VOID_STAR ap, SLuindex_Type na,
				  SLtype b_type, VOID_STAR bp, SLuindex_Type nb,
				  VOID_STAR cp)
{
   char *ic;
   double *a, *b, *c;
   SLuindex_Type n, n_max;
   SLuindex_Type da, db;

   (void) a_type;
   (void) b_type;

   a = (double *) ap;
   b = (double *) bp;
   c = (double *) cp;
   ic = (char *) cp;

   if (na == 1) da = 0; else da = 2;
   if (nb == 1) db = 0; else db = 1;

   if (na > nb) n_max = na; else n_max = nb;
   n_max = 2 * n_max;

   switch (op)
     {
      default:
	return 0;

      case SLANG_PLUS:
	for (n = 0; n < n_max; n += 2)
	  {
	     c[n] = a[0] + b[0];
	     c[n + 1] = a[1];
	     a += da; b += db;
	  }
	break;

      case SLANG_MINUS:
	for (n = 0; n < n_max; n += 2)
	  {
	     c[n] = a[0] - b[0];
	     c[n + 1] = a[1];
	     a += da; b += db;
	  }
	break;

      case SLANG_TIMES:
	for (n = 0; n < n_max; n += 2)
	  {
	     double b0 = b[0];
	     c[n] = a[0] * b0;
	     c[n + 1] = a[1] * b0;
	     a += da; b += db;
	  }
	break;

      case SLANG_DIVIDE:	       /* / */
	for (n = 0; n < n_max; n += 2)
	  {
	     double b0 = b[0];
#if 0
	     if (b0 == 0.0)
	       {
		  SLang_set_error (SL_DIVIDE_ERROR);
		  return -1;
	       }
#endif
	     c[n] = a[0] / b0;
	     c[n + 1] = a[1] / b0;
	     a += da; b += db;
	  }
	break;

      case SLANG_EQ: 		       /* == */
	for (n = 0; n < n_max; n += 2)
	  {
	     ic[n/2] = ((a[0] == b[0]) && (a[1] == 0.0));
	     a += da; b += db;
	  }
	break;

      case SLANG_NE:		       /* != */
	for (n = 0; n < n_max; n += 2)
	  {
	     ic[n/2] = ((a[0] != b[0]) || (a[1] != 0.0));
	     a += da; b += db;
	  }
	break;

      case SLANG_POW:
	for (n = 0; n < n_max; n += 2)
	  {
	     complex_dpow (c + n, a, b[0]);
	     a += da; b += db;
	  }
	break;
     }

   return 1;
}

static int double_complex_binary (int op,
				  SLtype a_type, VOID_STAR ap, SLuindex_Type na,
				  SLtype b_type, VOID_STAR bp, SLuindex_Type nb,
				  VOID_STAR cp)
{
   char *ic;
   double *a, *b, *c;
   SLuindex_Type n, n_max;
   SLuindex_Type da, db;

   (void) a_type;
   (void) b_type;

   a = (double *) ap;
   b = (double *) bp;
   c = (double *) cp;
   ic = (char *) cp;

   if (na == 1) da = 0; else da = 1;
   if (nb == 1) db = 0; else db = 2;

   if (na > nb) n_max = na; else n_max = nb;
   n_max = 2 * n_max;

   switch (op)
     {
      default:
	return 0;

      case SLANG_PLUS:
	for (n = 0; n < n_max; n += 2)
	  {
	     c[n] = a[0] + b[0];
	     c[n + 1] = b[1];
	     a += da; b += db;
	  }
	break;

      case SLANG_MINUS:
	for (n = 0; n < n_max; n += 2)
	  {
	     c[n] = a[0] - b[0];
	     c[n + 1] = -b[1];
	     a += da; b += db;
	  }
	break;

      case SLANG_TIMES:
	for (n = 0; n < n_max; n += 2)
	  {
	     double a0 = a[0];
	     c[n] = a0 * b[0];
	     c[n + 1] = a0 * b[1];
	     a += da; b += db;
	  }
	break;

      case SLANG_DIVIDE:	       /* / */
	for (n = 0; n < n_max; n += 2)
	  {
	     double z[2];
#if 0
	     if ((b[0] == 0.0) && (b[1] == 0.0))
	       {
		  SLang_set_error (SL_DIVIDE_ERROR);
		  return -1;
	       }
#endif
	     z[0] = a[0];
	     z[1] = 0.0;
	     SLcomplex_divide (c + n, z, b);
	     a += da; b += db;
	  }
	break;

      case SLANG_EQ: 		       /* == */
	for (n = 0; n < n_max; n += 2)
	  {
	     ic[n/2] = ((a[0] == b[0]) && (0.0 == b[1]));
	     a += da; b += db;
	  }
	break;

      case SLANG_NE:		       /* != */
	for (n = 0; n < n_max; n += 2)
	  {
	     ic[n/2] = ((a[0] != b[0]) || (0.0 != b[1]));
	     a += da; b += db;
	  }
	break;

      case SLANG_POW:
	for (n = 0; n < n_max; n += 2)
	  {
	     dcomplex_pow (c + n, a[0], b);
	     a += da; b += db;
	  }
	break;
     }

   return 1;
}

static int complex_generic_binary (int op,
				   SLtype a_type, VOID_STAR ap, SLuindex_Type na,
				   SLtype b_type, VOID_STAR bp, SLuindex_Type nb,
				   VOID_STAR cp)
{
   char *ic;
   char *b;
   double *a, *c;
   SLuindex_Type n, n_max;
   SLuindex_Type da, db;
   unsigned int sizeof_b;
   SLang_To_Double_Fun_Type to_double;

   if (NULL == (to_double = SLarith_get_to_double_fun (b_type, &sizeof_b)))
     return 0;

   (void) a_type;

   a = (double *) ap;
   b = (char *) bp;
   c = (double *) cp;
   ic = (char *) cp;

   if (na == 1) da = 0; else da = 2;
   if (nb == 1) db = 0; else db = sizeof_b;

   if (na > nb) n_max = na; else n_max = nb;
   n_max = 2 * n_max;

   switch (op)
     {
      default:
	return 0;

      case SLANG_POW:
	for (n = 0; n < n_max; n += 2)
	  {
	     complex_dpow (c + n, a, to_double((VOID_STAR)b));
	     a += da; b += db;
	  }
	break;

      case SLANG_PLUS:
	for (n = 0; n < n_max; n += 2)
	  {
	     c[n] = a[0] + to_double((VOID_STAR)b);
	     c[n + 1] = a[1];
	     a += da; b += db;
	  }
	break;

      case SLANG_MINUS:
	for (n = 0; n < n_max; n += 2)
	  {
	     c[n] = a[0] - to_double((VOID_STAR)b);
	     c[n + 1] = a[1];
	     a += da; b += db;
	  }
	break;

      case SLANG_TIMES:
	for (n = 0; n < n_max; n += 2)
	  {
	     double b0 = to_double((VOID_STAR)b);
	     c[n] = a[0] * b0;
	     c[n + 1] = a[1] * b0;
	     a += da; b += db;
	  }
	break;

      case SLANG_DIVIDE:	       /* / */
	for (n = 0; n < n_max; n += 2)
	  {
	     double b0 = to_double((VOID_STAR)b);
#if 0
	     if (b0 == 0)
	       {
		  SLang_set_error (SL_DIVIDE_ERROR);
		  return -1;
	       }
#endif
	     c[n] = a[0] / b0;
	     c[n + 1] = a[1] / b0;
	     a += da; b += db;
	  }
	break;

      case SLANG_EQ: 		       /* == */
	for (n = 0; n < n_max; n += 2)
	  {
	     ic[n/2] = ((a[0] == to_double((VOID_STAR)b)) && (a[1] == 0.0));
	     a += da; b += db;
	  }
	break;

      case SLANG_NE:		       /* != */
	for (n = 0; n < n_max; n += 2)
	  {
	     ic[n/2] = ((a[0] != to_double((VOID_STAR)b)) || (a[1] != 0.0));
	     a += da; b += db;
	  }
	break;
     }

   return 1;
}

static int generic_complex_binary (int op,
				   SLtype a_type, VOID_STAR ap, SLuindex_Type na,
				   SLtype b_type, VOID_STAR bp, SLuindex_Type nb,
				   VOID_STAR cp)
{
   double *b, *c;
   char *a, *ic;
   SLuindex_Type n, n_max;
   SLuindex_Type da, db;
   unsigned int sizeof_a;
   SLang_To_Double_Fun_Type to_double;

   if (NULL == (to_double = SLarith_get_to_double_fun (a_type, &sizeof_a)))
     return 0;

   (void) b_type;

   a = (char *) ap;
   b = (double *) bp;
   c = (double *) cp;
   ic = (char *) cp;

   if (na == 1) da = 0; else da = sizeof_a;
   if (nb == 1) db = 0; else db = 2;

   if (na > nb) n_max = na; else n_max = nb;
   n_max = 2 * n_max;

   switch (op)
     {
      default:
	return 0;
      case SLANG_POW:
	for (n = 0; n < n_max; n += 2)
	  {
	     dcomplex_pow (c + n, to_double((VOID_STAR)a), b);
	     a += da; b += db;
	  }
	break;

      case SLANG_PLUS:
	for (n = 0; n < n_max; n += 2)
	  {
	     c[n] = to_double((VOID_STAR)a) + b[0];
	     c[n + 1] = b[1];
	     a += da; b += db;
	  }
	break;

      case SLANG_MINUS:
	for (n = 0; n < n_max; n += 2)
	  {
	     c[n] = to_double((VOID_STAR)a) - b[0];
	     c[n + 1] = -b[1];
	     a += da; b += db;
	  }
	break;

      case SLANG_TIMES:
	for (n = 0; n < n_max; n += 2)
	  {
	     double a0 = to_double((VOID_STAR)a);
	     c[n] = a0 * b[0];
	     c[n + 1] = a0 * b[1];
	     a += da; b += db;
	  }
	break;

      case SLANG_DIVIDE:	       /* / */
	for (n = 0; n < n_max; n += 2)
	  {
	     double z[2];
#if 0
	     if ((b[0] == 0.0) && (b[1] == 0.0))
	       {
		  SLang_set_error (SL_DIVIDE_ERROR);
		  return -1;
	       }
#endif
	     z[0] = to_double((VOID_STAR)a);
	     z[1] = 0.0;
	     SLcomplex_divide (c + n, z, b);
	     a += da; b += db;
	  }
	break;

      case SLANG_EQ: 		       /* == */
	for (n = 0; n < n_max; n += 2)
	  {
	     ic[n/2] = ((to_double((VOID_STAR)a) == b[0]) && (0.0 == b[1]));
	     a += da; b += db;
	  }
	break;

      case SLANG_NE:		       /* != */
	for (n = 0; n < n_max; n += 2)
	  {
	     ic[n/2] = ((to_double((VOID_STAR)a) != b[0]) || (0.0 != b[1]));
	     a += da; b += db;
	  }
	break;
     }

   return 1;
}

static int complex_unary_result (int op, SLtype a, SLtype *b)
{
   (void) a;

   switch (op)
     {
      default:
	return 0;

      case SLANG_PLUSPLUS:
      case SLANG_MINUSMINUS:
      case SLANG_CHS:
      case SLANG_MUL2:
	*b = SLANG_COMPLEX_TYPE;
	break;

      case SLANG_SQR:		       /* |Real|^2 + |Imag|^2 ==> double */
      case SLANG_ABS:		       /* |z| ==> double */
	*b = SLANG_DOUBLE_TYPE;
	break;

      case SLANG_SIGN:
	*b = SLANG_INT_TYPE;
	break;
     }
   return 1;
}

static int complex_unary (int op,
			  SLtype a_type, VOID_STAR ap, SLuindex_Type na,
			  VOID_STAR bp)
{
   SLuindex_Type n;
   double *a, *b;
   int *ic;

   (void) a_type;

   a = (double *) ap;
   b = (double *) bp;
   ic = (int *) bp;

   na = 2 * na;

   switch (op)
     {
      default:
	return 0;

      case SLANG_PLUSPLUS:
	for (n = 0; n < na; n += 2) b[n] = (a[n] + 1);
	break;
      case SLANG_MINUSMINUS:
	for (n = 0; n < na; n += 2) b[n] = (a[n] - 1);
	break;
      case SLANG_CHS:
	for (n = 0; n < na; n += 2)
	  {
	     b[n] = -(a[n]);
	     b[n + 1] = -(a[n + 1]);
	  }
	break;
      case SLANG_SQR:		       /* |Real|^2 + |Imag|^2 ==> double */
	for (n = 0; n < na; n += 2)
	  b[n/2] = (a[n] * a[n] + a[n + 1] * a[n + 1]);
	break;

      case SLANG_MUL2:
	for (n = 0; n < na; n += 2)
	  {
	     b[n] = (2 * a[n]);
	     b[n + 1] = (2 * a[n + 1]);
	  }
	break;

      case SLANG_ABS:		       /* |z| ==> double */
	for (n = 0; n < na; n += 2)
	  b[n/2] = SLcomplex_abs (a + n);
	break;

      case SLANG_SIGN:
	/* Another creative extension.  Lets return an integer which indicates
	 * whether the complex number is in the upperhalf plane or not.
	 */
	for (n = 0; n < na; n += 2)
	  {
	     if (a[n + 1] < 0.0) ic[n/2] = -1;
	     else if (a[n + 1] > 0.0) ic[n/2] = 1;
	     else ic[n/2] = 0;
	  }
	break;
     }

   return 1;
}

static int
complex_typecast (SLtype from_type, VOID_STAR from, SLuindex_Type num,
		  SLtype to_type, VOID_STAR to)
{
   double *z;
   double *d;
   char *i;
   SLuindex_Type n;
   unsigned int sizeof_i;
   SLang_To_Double_Fun_Type to_double;

   (void) to_type;

   z = (double *) to;

   switch (from_type)
     {
      default:
	if (NULL == (to_double = SLarith_get_to_double_fun (from_type, &sizeof_i)))
	  return 0;
	i = (char *) from;
	for (n = 0; n < num; n++)
	  {
	     *z++ = to_double ((VOID_STAR) i);
	     *z++ = 0.0;

	     i += sizeof_i;
	  }
	break;

      case SLANG_DOUBLE_TYPE:
	d = (double *) from;
	for (n = 0; n < num; n++)
	  {
	     *z++ = d[n];
	     *z++ = 0.0;
	  }
	break;
     }

   return 1;
}

static void complex_destroy (SLtype type, VOID_STAR ptr)
{
   (void) type;
   SLfree ((char *)*(double **) ptr);
}

static int complex_push (SLtype type, VOID_STAR ptr)
{
   double *z;

   (void) type;
   z = *(double **) ptr;
   return SLang_push_complex (z[0], z[1]);
}

static int complex_pop (SLtype type, VOID_STAR ptr)
{
   double *z;

   (void) type;
   z = *(double **) ptr;
   return SLang_pop_complex (&z[0], &z[1]);
}

int _pSLinit_slcomplex (void)
{
   SLang_Class_Type *cl;
   SLtype *types;

   if (NULL == (cl = SLclass_allocate_class ("Complex_Type")))
     return -1;

   (void) SLclass_set_destroy_function (cl, complex_destroy);
   (void) SLclass_set_push_function (cl, complex_push);
   (void) SLclass_set_pop_function (cl, complex_pop);

   if (-1 == SLclass_register_class (cl, SLANG_COMPLEX_TYPE, 2 * sizeof (double),
				     SLANG_CLASS_TYPE_VECTOR))
     return -1;

   types = _pSLarith_Arith_Types;
   while (*types != SLANG_DOUBLE_TYPE)
     {
	SLtype t = *types++;

	if ((-1 == SLclass_add_binary_op (t, SLANG_COMPLEX_TYPE, generic_complex_binary, complex_binary_result))
	    || (-1 == SLclass_add_binary_op (SLANG_COMPLEX_TYPE, t, complex_generic_binary, complex_binary_result))
	    || (-1 == (SLclass_add_typecast (t, SLANG_COMPLEX_TYPE, complex_typecast, 1))))
	  return -1;
     }

   if ((-1 == (SLclass_add_binary_op (SLANG_COMPLEX_TYPE, SLANG_COMPLEX_TYPE, complex_complex_binary, complex_binary_result)))
       || (-1 == (SLclass_add_binary_op (SLANG_COMPLEX_TYPE, SLANG_DOUBLE_TYPE, complex_double_binary, complex_binary_result)))
       || (-1 == (SLclass_add_binary_op (SLANG_DOUBLE_TYPE, SLANG_COMPLEX_TYPE, double_complex_binary, complex_binary_result)))
       || (-1 == (SLclass_add_unary_op (SLANG_COMPLEX_TYPE, complex_unary, complex_unary_result)))
       || (-1 == (SLclass_add_typecast (SLANG_DOUBLE_TYPE, SLANG_COMPLEX_TYPE, complex_typecast, 1))))
     return -1;

   return 0;
}

#endif				       /* if SLANG_HAS_COMPLEX */

