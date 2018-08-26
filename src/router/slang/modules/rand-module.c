/* -*- mode: C; mode: fold -*-
Copyright (C) 2007-2011 John E. Davis

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
#include "config.h"

#include <stdio.h>
#include <math.h>
#include <slang.h>

#ifdef HAVE_PROCESS_H
# include <process.h>			/* for getpid */
#endif

#include <sys/types.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <time.h>

/* The random number generator used here uses a combination of 3 generators.
 * One of the generators was derived from mzran13 published in
 *
 *   G. Marsaglia and A. Zaman, ``Some portable very-long-period
 * 	random number generators'', Computers in Physics, Vol. 8,
 * 	No. 1, Jan/Feb 1994
 *
 * However, the algorithm (mzran13) published there contained a few
 * typos and as written was not very good.  The subtract-with-carry
 * component of mzran13 adopted for the purposes of the generator in
 * this file has a period of 4x10^28.
 *
 * The other two generators are described at
 * <http://www.helsbreth.org/random/rng_combo.html>.  Briefly, one is
 * a simple product generator, r_{k+2} = r_{k+1} r_k, and the other
 * is is a multiply with carry generator.  The period of this
 * combination exceeds 10^18.
 *
 * The total period of the combined generators exceeds 10^46.
 *
 * The generator was tested using dieharder-2.24.4, and the testing
 * indicates that it may perform a bit better than the Mersenne
 * twister mt19937_1999, which is thought to be a high-quality
 * generator.  The speed is about the same.
 *
 * Note: mt19937_1999 is available to S-Lang users via the GSL module.
 */

SLANG_MODULE(rand);

#ifdef PI
# undef PI
#endif
#define PI 3.14159265358979323846264338327950288
#define LOG_SQRT_2PI 0.9189385332046727417803297364

/*{{{ The basic generator */

#define SEED_X	521288629U
#define SEED_Y	362436069U
#define SEED_Z	16163801U
#define SEED_C  1    /* (SEED_Y > SEED_Z) */

#if (SIZEOF_INT == 4)
typedef unsigned int uint32;
# define UINT32_TYPE SLANG_UINT_TYPE
# define PUSH_UINT32 SLang_push_uint
#else
typedef unsigned long uint32
# define UINT32_TYPE SLANG_ULONG_TYPE
# define PUSH_UINT32 SLang_push_ulong
#endif

#define CACHE_SIZE 4		       /* do not change this */

typedef struct
{
   int cache_index;
   uint32 cache[CACHE_SIZE];
   uint32 x, y, z;
   uint32 cx, cy, cz;

   /* Gaussian Random fields */
   int one_available;
   double g2;
}
Rand_Type;

static uint32 generate_uint32_random (Rand_Type *);

#define NUM_SEEDS 3
static void seed_random (Rand_Type *rt, unsigned long seeds[NUM_SEEDS])
{
   int count;
   unsigned long s1 = seeds[0];
   unsigned long s2 = seeds[1];
   unsigned long s3 = seeds[2];

   rt->x = (uint32) (s1 + SEED_X);
   rt->y = (uint32) (s1/2 + SEED_Y);
   rt->z = (uint32) (2*s1+SEED_Z);
   rt->x += (rt->y > rt->z);

   rt->cache_index = CACHE_SIZE;

   rt->cx = s2 * 8 + 3;
   rt->cy = s2 * 2 + 1;
   rt->cz = s3 | 1;
   count = 32;
   while (count)
     {
	count--;
	(void) generate_uint32_random (rt);
     }

   rt->one_available = 0;
   rt->g2 = 0.0;
}

static uint32 generate_uint32_random (Rand_Type *rt)
{
   uint32 s0, s1, s2, s3, r;
   uint32 *cache;

   cache = rt->cache;
   if (rt->cache_index < CACHE_SIZE)
     return cache[rt->cache_index++];

   s1 = rt->x;
   s2 = rt->y;
   s3 = rt->z;

   r = s0 = (s2-s1) - 18*(s2<=s1); s2 += (s2 <= s1);
   rt->x = s1 = (s3-s2) - 18*(s3<=s2); s3 += (s3 <= s2);
   rt->y = s2 = (s0-s3) - 18*(s0<=s3); s0 += (s0 <= s3);
   rt->z = s3 = (s1-s0) - 18*(s1<=s0); s1 += (s1 <= s0);

#define COMBINE(y,x,a,b) y = ((x)+(a)+(b))
   s1 = rt->cx;
   s2 = rt->cy;
   s3 = rt->cz;

   s0 = s1*s2;
   s3 = 30903U*(s3 & 0xFFFFU) + (s3 >> 16);
   COMBINE(r,r,s0,s3);

   s1 = s2*s0;
   s3 = 30903U*(s3 & 0xFFFFU) + (s3 >> 16);
   COMBINE(cache[1],rt->x,s1,s3);

   rt->cx = s2 = s0*s1;
   s3 = 30903U*(s3 & 0xFFFFU) + (s3 >> 16);
   COMBINE(cache[2],rt->y,s2,s3);

   rt->cy = s0 = s1*s2;
   rt->cz = s3 = 30903U*(s3 & 0xFFFFU) + (s3 >> 16);
   COMBINE(cache[3],rt->z,s0,s3);

   rt->cache_index = 1;
   return r;
}

static void free_random (Rand_Type *r)
{
   SLfree ((char *) r);
}

static Rand_Type *create_random (unsigned long seeds[NUM_SEEDS])
{
   Rand_Type *rt;

   if (NULL != (rt = (Rand_Type *) SLmalloc (sizeof (Rand_Type))))
     seed_random (rt, seeds);
   return rt;
}

/*}}}*/

/*{{{ Flat distribution */

static void generate_random_doubles (Rand_Type *rt, VOID_STAR ap,
				     SLuindex_Type num, VOID_STAR parms)
{
   double *a = (double *)ap;
   double *amax = a + num;
   uint32 *cache = rt->cache;

   (void) parms;

   while (a < amax)
     {
	uint32 u;

	if (rt->cache_index < CACHE_SIZE)
	  u = cache[rt->cache_index++];
	else
	  u = generate_uint32_random (rt);

	*a++ = u / 4294967296.0;
     }
}

static void generate_random_open_doubles (Rand_Type *rt, VOID_STAR ap,
					  SLuindex_Type num, VOID_STAR parms)
{
   double *a = (double *)ap;
   double *amax = a + num;
   uint32 *cache = rt->cache;

   (void) parms;

   while (a < amax)
     {
	uint32 u;

	if (rt->cache_index < CACHE_SIZE)
	  u = cache[rt->cache_index++];
	else
	  u = generate_uint32_random (rt);

	if (u == 0)
	  continue;

	*a++ = u / 4294967296.0;
     }
}

static void generate_random_uints (Rand_Type *rt, VOID_STAR ap,
				   SLuindex_Type num, VOID_STAR parms)
{
   uint32 *a = (uint32 *)ap;
   uint32 *amax = a + num;
   uint32 *cache = rt->cache;

   (void) parms;

   while (a < amax)
     {
	if (rt->cache_index < CACHE_SIZE)
	  *a++ = cache[rt->cache_index++];
	else
	  *a++ = generate_uint32_random (rt);
     }
}

/* produces a random number in the open interval (0,1) */
static double open_interval_random (Rand_Type *rt)
{
   uint32 u;

   do
     {
	if (rt->cache_index < CACHE_SIZE)
	  u = rt->cache[rt->cache_index++];
	else
	  u = generate_uint32_random (rt);
     }
   while (u == 0);

   return u / 4294967296.0;
}

static double uniform_random (Rand_Type *rt)
{
   uint32 u;

   if (rt->cache_index < CACHE_SIZE)
     u = rt->cache[rt->cache_index++];
   else
     u = generate_uint32_random (rt);

   return u / 4294967296.0;
}

/*}}}*/

/*{{{ Gaussian Distribution */

static double gaussian_box_muller (Rand_Type *rt)
{
   double s, g1, g2, g;

   if (rt->one_available)
     {
	rt->one_available = 0;
	return rt->g2;
     }

   do
     {
	uint32 u;
	if (rt->cache_index < CACHE_SIZE)
	  u = rt->cache[rt->cache_index++];
	else
	  u = generate_uint32_random (rt);

	g1 = 2.0*(u/4294967296.0) - 1.0;

	if (rt->cache_index < CACHE_SIZE)
	  u = rt->cache[rt->cache_index++];
	else
	  u = generate_uint32_random (rt);
	g2 = 2.0*(u/4294967296.0) - 1.0;
	g = g1*g1 + g2*g2;
     }
   while ((g >= 1.0) || (g == 0.0));

   s = sqrt (-2.0 * log (g) / g);
   rt->g2 = g2 * s;
   rt->one_available = 1;
   return g1*s;
}

static void generate_gaussian_randoms (Rand_Type *rt, VOID_STAR ap,
				       SLuindex_Type num, VOID_STAR parms)
{
   double *a = (double *)ap;
   double *amax = a + num;
   double sigma = *(double *)parms;

   if ((a < amax) && (rt->one_available))
     {
	*a++ = sigma*rt->g2;
	rt->one_available = 0;
     }

   while (a < amax)
     {
	*a++ = sigma*gaussian_box_muller (rt);
	if (a == amax)
	  break;

	*a++ = sigma*rt->g2;
	rt->one_available = 0;
     }
}

/*}}}*/

/*{{{ Poisson Distribution */

/*
 * The algorithm used is described in:
 * W. Hörmann "The Transformed Rejection Method for Generating
 * Poisson Random Variables", which is available from
 * <http://statistik.wu-wien.ac.at/papers/92-04-13.wh.ps.gz>.
 */
#define FACTORIAL_TABLE_SIZE 10
static double Log_Factorial_Table [FACTORIAL_TABLE_SIZE+1];

static void init_poisson (void)
{
   unsigned int i;
   double x = 1.0;
   Log_Factorial_Table[0] = 0.0;
   for (i = 1; i <= FACTORIAL_TABLE_SIZE; i++)
     {
	x *= i;
	Log_Factorial_Table[i] = log(x);
     }
}

/* Assumes x >= 0 */
static double log_factorial (double x)
{
   double x2;

   if (x <= FACTORIAL_TABLE_SIZE)
     return Log_Factorial_Table[(unsigned int) x];

   x2 = x*x;

   return LOG_SQRT_2PI + (x+0.5)*log(x) - x
     + (13860.0 - (462.0 - (132.0 - (99.0 - 140.0/x2)/x2)/x2)/x2)/x/166320.0;
}

static unsigned int hoermann_ptrd_poisson
  (Rand_Type *rt, double mu, double a, double b, double vr,
      double alphainv, double lnmu, double smu)
{
   while (1)
     {
	double u, v, fk, us;
	unsigned int k;

	v = open_interval_random (rt);
	if (v <= 0.86*vr)
	  {
	     u = v/vr - 0.43;
	     fk = floor((2.0*a/(0.5-fabs(u))+b)*u + mu + 0.445);
	     return (unsigned int)fk;
	  }
	if (v >= vr)
	  u = open_interval_random (rt) - 0.5;
	else
	  {
	     u = v/vr - 0.93; u = ((u < 0.0) ? -0.5 : 0.5) - u;
	     v = vr*open_interval_random (rt);
	  }
	us = 0.5 - fabs(u);
	if ((us < 0.013) && (v > us))
	  continue;

	fk = floor((2.0*a/us + b)*u + mu + 0.445);
	if (fk < 0.0)
	  continue;

	k = (unsigned int) fk;

	v = v * alphainv/(a/(us*us) + b);
	if ((k >= 10)
	    && (log(v*smu) <= (fk+0.5)*log(mu/fk)-mu-LOG_SQRT_2PI+fk
		- (1.0/12-1.0/(360.0*fk*fk))/fk))
	  return (unsigned int)fk;

	if ((k <= 9)
	    && (log(v)<=fk*lnmu-mu-Log_Factorial_Table[k]))
	  return k;
     }
}

static unsigned int knuth_poisson (Rand_Type *rt, double cutoff)
{
   double x = 1.0;
   unsigned int n = 0;

   do
     {
	if (rt->cache_index < CACHE_SIZE)
	  x *= rt->cache[rt->cache_index++] / 4294967296.0;
	else
	  x *= generate_uint32_random (rt) / 4294967296.0;
	n++;
     }
   while (x >= cutoff);

   return n - 1;
}

static void generate_poisson_randoms (Rand_Type *rt, VOID_STAR ap,
				      SLuindex_Type num, VOID_STAR parms)
{
   unsigned int *x = (unsigned int *)ap;
   unsigned int *xmax = x + num;
   double mu = *(double *)parms;
   double cutoff;

   if (mu > 10.0)
     {
	double smu = sqrt(mu);
	double b = 0.931 + 2.53*smu;
	double a = -0.059 + 0.02483*b;
	double vr = 0.9277 - 3.6224/(b-2.0);
	double alphainv = 1.1239 + 1.1328/(b-3.4);
	double lnmu = log(mu);

	while (x < xmax)
	  {
	     *x++ = hoermann_ptrd_poisson (rt, mu, a, b, vr, alphainv, lnmu, smu);
	  }
	return;
     }

   cutoff = exp(-mu);
   while (x < xmax)
     {
	*x++ = knuth_poisson (rt, cutoff);
     }
}

/*}}}*/

/*{{{ Gamma Distribution */

/* The gamma distribution is:
 *
 *  f(x; k,theta)dx = x^{k-1} \frac{e^{-x/theta}}{\theta^k \Gamma(k)} dx
 *    where x>0, k,theta>0.
 * The marsaglia_tsang_gamma algorithm is for x>0, k>=1, theta=1:
 *  f(y;k) = y^{k-1}\frac{e^{-y}}{\Gamma(k)}
 * Let x = theta*y.  dy = dx/theta
 *
 * Then:
 *
 *  f(y;k)dy = (x/theta)^{k-1}\frac{e^{-x/theta}}{\Gamma(k)} dx / \theta
 *   = x^{k-1}\frac{e^{-x/theta}}{\theta^k\Gamma(k)} dx
 *   = f(x; k,theta)
 *
 * Also the final note in the Marsaglia paper says that values for
 * k<1 may be obtained using X_k = X_{k+1}U^{1/k} where X is a random
 * value produced by the algorithm, and U is a uniform random number
 * on (0,1).
 *
 * Marsaglia, G. and Tsang, W. W. 2000a. A simple method for generating
 *  gamma variables.  ACM Transactions on Mathematical Software 26, 363–372.
 * <http://oldmill.uchicago.edu/~wilder/Code/random/Papers/Marsaglia_00_SMGGV.pdf>
 *
 */
static double marsaglia_tsang_gamma_internal (Rand_Type *rt, double c, double d)
{
   double v, u;

   while (1)
     {
	double x;
        do
          {
	     if (rt->one_available)
	       {
		  x = rt->g2;
		  rt->one_available = 0;
	       }
	     else x = gaussian_box_muller (rt);
	     v = 1.0 + c*x;
          }
        while (v <= 0.0);

        v = v*v*v;
        u = open_interval_random (rt);
	x = x*x;

        if ((u < 1.0 - 0.0331*(x*x))
	    || (log (u) < 0.5*x + d*(1.0 - v + log (v))))
          return d*v;
      }
}

/* k > 0. theta > 0 */
static double rand_gamma (Rand_Type *rt, double k, double theta)
{
   double c, d;

#ifdef HAVE_ISNAN
   if (isnan(k) || isnan(theta))
     return k*theta;
#endif

   if (k < 1.0)
     {
	d = k + 2.0/3.0;
	c = (1.0/3.0)/sqrt(d);
	return theta * marsaglia_tsang_gamma_internal (rt, c, d)
	  * pow (open_interval_random(rt), 1.0/k);
     }

   d = k - 1.0/3.0;
   c = (1.0/3.0)/sqrt(d);
   return theta * marsaglia_tsang_gamma_internal (rt, c, d);
}

static void generate_gamma_randoms (Rand_Type *rt, VOID_STAR ap,
				    SLuindex_Type num, VOID_STAR parms)
{
   double *x = (double *)ap;
   double *xmax = x + num;
   double k, theta;
   double c, d;

   k = ((double *)parms)[0];
   theta = ((double *)parms)[1];

#ifdef HAVE_ISNAN
   if (isnan(k) || isnan(theta))
     {
	while (x < xmax)
	  *x++ = k*theta;
	return;
     }
#endif

   if (k < 1.0)
     {
	double kinv = 1.0/k;
	d = k + 2.0/3.0;
	c = (1.0/3.0)/sqrt(d);

	while (x < xmax)
	  {
	     *x++ = theta * marsaglia_tsang_gamma_internal (rt, c, d)
	       * pow (open_interval_random(rt), kinv);
	  }
	return;
     }

   d = k - 1.0/3.0;
   c = (1.0/3.0)/sqrt(d);
   while (x < xmax)
     *x++ = theta * marsaglia_tsang_gamma_internal (rt, c, d);
}

/*}}}*/

/*{{{ Beta Distribution */

static double knuth_beta (Rand_Type *rt, double alpha, double beta)
{
   if (0.0 == (alpha = rand_gamma (rt, alpha, 1.0)))
     return 0.0;

   beta = rand_gamma (rt, beta, 1.0);

   return alpha/(alpha+beta);
}

static void generate_beta_randoms (Rand_Type *rt, VOID_STAR ap,
				   SLuindex_Type num, VOID_STAR parms)
{
   double *x = (double *)ap;
   double *xmax = x + num;
   double alpha, beta;

   alpha = ((double *)parms)[0];
   beta = ((double *)parms)[1];

   while (x < xmax)
     *x++ = knuth_beta (rt, alpha, beta);
}

/*}}}*/

/*{{{ Binomial Distribution */

/* This algorithm is from:
 *  Hormann, W. (1993), The generation of binomial random variates,
 *    Journal of Statistical Computation and Simulation 46, 101-110.
 * I obtained it from the preprint, but not the actual journal article.
 * The preprint contains a typo in the algorithm that I managed to
 * find and correct.  It pays to read the paper.
 */

typedef struct
{
   double a, b, c, vr, alpha, lpq, fm, h, p;
   unsigned int n;
}
BTRS_Type;

static void init_btrs (BTRS_Type *btrs, double p, unsigned int n)
{
   double spq = sqrt (n*p*(1.0-p));

   btrs->p = p;
   btrs->n = n;

   btrs->b = 1.15 + 2.53*spq;
   btrs->a = -0.0873 + 0.0248*btrs->b + 0.01*p;
   btrs->c = n*p+0.5;
   btrs->vr = 0.92 - 4.2/btrs->b;
   btrs->alpha = (2.83+5.1/btrs->b) * spq;
   btrs->lpq = log (p/(1.0-p));
   btrs->fm = floor ((n+1)*p);
   btrs->h = log_factorial (btrs->fm) + log_factorial(n-btrs->fm);
}

/* This algorithm assumes that p <= 0.5 and p*n >= 10.0 */
static double binomial_btrs (Rand_Type *rt, BTRS_Type *btrs)
{
   double a = btrs->a;
   double b = btrs->b;
   double c = btrs->c;
   double vr = btrs->vr;
   double h = btrs->h;
   double lpq = btrs->lpq;
   double fm = btrs->fm;
   double alpha = btrs->alpha;
   unsigned int n = btrs->n;

   while (1)
     {
	double u = open_interval_random (rt) - 0.5;
	double v = open_interval_random (rt);
	double us = 0.5 - fabs(u);
	double fk = floor ((2.0*a/us + b)*u + c);

	if ((fk < 0.0) || ((unsigned int) fk > n))
	  continue;

	if ((us >= 0.07) && (v <= vr))
	  return (unsigned int) fk;

	v = log (v*alpha/(a/(us*us) + b));
	if (v <= (h - log_factorial(fk) - log_factorial(n-fk)
		  + (fk-fm)*lpq))
	  return (unsigned int) fk;
     }
}

typedef struct
{
   unsigned int n;
   double p;
}
Binomial_Parms_Type;

static void generate_binomial_randoms (Rand_Type *rt, VOID_STAR ap,
				       SLuindex_Type num, VOID_STAR parms)
{
   unsigned int *x = (unsigned int *)ap;
   unsigned *xmax = x + num;
   Binomial_Parms_Type *s = (Binomial_Parms_Type *)parms;
   double p, qn, r, q, g;
   unsigned int n;
   int swapped = 0;

   n = s->n;
   p = s->p;

   if (p > 0.5)
     {
	p = 1.0 - p;
	swapped = 1;
     }

   if (n*p > 10.0)
     {
	BTRS_Type btrs;
	init_btrs (&btrs, p, n);
	if (swapped)
	  {
	     while (x < xmax)
	       *x++ = n - binomial_btrs (rt, &btrs);
	  }
	else
	  {
	     while (x < xmax)
	       *x++ = binomial_btrs (rt, &btrs);
	  }
	return;
     }

   /* Inverse CDF method (Adapted from RANLIB) */
   q = 1.0 - p;
   qn = pow (q, n);
   r = p/q;
   g = r*(n+1);
   while (x < xmax)
     {
#define MAX_INV_BINOMIAL_CDF_LOOPS 110
	double f = qn;
	double u = uniform_random (rt);
	unsigned int k = 0;
	unsigned kmax = (n > MAX_INV_BINOMIAL_CDF_LOOPS
			 ? MAX_INV_BINOMIAL_CDF_LOOPS : n);

	while (k <= kmax)
	  {
	     if (u < f)
	       {
		  if (swapped)
		    k = n - k;
		  *x++ = k;
		  break;
	       }
	     u -= f;
	     k++;
	     f *= (g/k - r);
	  }
     }
}

/*}}}*/

static void generate_cauchy_randoms (Rand_Type *rt, VOID_STAR ap,
				     SLuindex_Type num, VOID_STAR parms)
{
   double *x = (double *)ap;
   double *xmax = x + num;
   double a = *(double *)parms;

   while (x < xmax)
     {
	double u;
	do
	  {
	     u = uniform_random (rt);
	  }
	while (u == 0.5);
	*x++ = a * tan (PI*u);
     }
}

static void generate_geometric_randoms (Rand_Type *rt, VOID_STAR ap,
					SLuindex_Type num, VOID_STAR parms)
{
   unsigned int *x = (unsigned int *)ap;
   unsigned int *xmax = x + num;
   double a = *(double *)parms;

   if (a == 1.0)
     {
	while (x < xmax)
	  *x++ = 1;
	return;
     }

   a = 1.0 / log (1.0-a);	       /* is negative */

   while (x < xmax)
     *x++ = (unsigned int) (1.0 + a*log(open_interval_random (rt)));
}

/* Interpreter Interface */

static Rand_Type *Default_Rand;
static int Rand_Type_Id = -1;

static int pop_seeds (unsigned long seeds[NUM_SEEDS])
{
   SLang_Array_Type *at;
   unsigned long *s;
   SLuindex_Type i;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_ULONG_TYPE))
     return -1;

   if (at->num_elements == 0)
     {
	SLang_verror (SL_InvalidParm_Error, "The seed array has no elements");
	SLang_free_array (at);
	return -1;
     }

   s = (unsigned long *)at->data;
   i = 0;
   while (i < NUM_SEEDS)
     {
	seeds[i] = *s;
	i++;
	if (i < at->num_elements)
	  s++;
     }
   SLang_free_array (at);
   return 0;
}

static void generate_seeds (unsigned long seeds[NUM_SEEDS])
{
   unsigned int i;
   unsigned long s = (unsigned long) time(NULL)*(unsigned long) getpid ();
   for (i = 0; i < NUM_SEEDS; i++)
     {
	s = s*69069UL + 1013904243UL;
	seeds[i] = s;
     }
}

static void new_rand_intrin (void) /*{{{*/
{
   unsigned long seeds[NUM_SEEDS];
   Rand_Type *r;
   SLang_MMT_Type *mmt;

   if (SLang_Num_Function_Args == 1)
     {
	if (-1 == pop_seeds (seeds))
	  return;
     }
   else generate_seeds (seeds);

   if (NULL == (r = create_random (seeds)))
     return;

   if (NULL == (mmt = SLang_create_mmt (Rand_Type_Id, (VOID_STAR) r)))
     {
	free_random (r);
	return;
     }

   if (0 == SLang_push_mmt (mmt))
     return;

   SLang_free_mmt (mmt);
}

/*}}}*/

/*{{{ Utility Functions */

static void destroy_rand_type (SLtype type, VOID_STAR vr)
{
   (void) type;
   free_random ((Rand_Type *) vr);
}

static int pop_rand_type_and_dims (int argc, SLang_MMT_Type **mmtp,
				   SLindex_Type *dims, unsigned int *ndims,
				   int *is_scalarp)
{
   int type;
   SLang_MMT_Type *mmt;
   unsigned int i, imax;

   *mmtp = NULL;

   switch (argc)
     {
      default:
	SLang_verror (SL_NumArgs_Error, "Expecting 0, 1, or 2 arguments");
	return -1;

      case 0:
	*is_scalarp = 1;
	return 0;

      case 1:
	type = SLang_peek_at_stack ();
	if (type == Rand_Type_Id)
	  {
	     if (NULL == (mmt = SLang_pop_mmt (Rand_Type_Id)))
	       return -1;
	     *is_scalarp = 1;
	     *mmtp = mmt;
	     return 0;
 	  }
	break;

      case 2:
	type = SLang_peek_at_stack ();
	break;
     }

   *is_scalarp = 0;

   if (type != SLANG_ARRAY_TYPE)
     {
	if (-1 == SLang_pop_array_index (dims))
	  return -1;

	*ndims = 1;
     }
   else
     {
	SLang_Array_Type *at;
	if (-1 == SLang_pop_array (&at, 1))
	  return -1;

	*ndims = imax = at->num_dims;
	for (i = 0; i < imax; i++)
	  dims[i] = at->dims[i];
	SLang_free_array (at);
     }

   if (argc == 2)
     {
	if (NULL == (mmt = SLang_pop_mmt (Rand_Type_Id)))
	  return -1;

	*mmtp = mmt;
     }
   return 0;
}

static int do_xxxrand (int argc, SLtype type,
		       void (*func)(Rand_Type *, VOID_STAR, SLuindex_Type, VOID_STAR),
		       VOID_STAR parms,
		       int *is_scalar_p, VOID_STAR scalar_addr)
{
   SLang_Array_Type *at;
   SLindex_Type dims [SLARRAY_MAX_DIMS];
   unsigned int ndims;
   SLang_MMT_Type *mmt;
   Rand_Type *rt;
   int is_scalar;
   int status = -1;

   if (-1 == pop_rand_type_and_dims (argc, &mmt, dims, &ndims, &is_scalar))
     return -1;

   if (mmt != NULL)
     {
	if (NULL == (rt = (Rand_Type *) SLang_object_from_mmt (mmt)))
	  goto free_return;
     }
   else
     rt = Default_Rand;

   *is_scalar_p = is_scalar;

   if (is_scalar)
     {
	(*func) (rt, scalar_addr, 1, parms);
	status = 0;
	goto free_return;
     }

   if (NULL == (at = SLang_create_array (type, 0, NULL, dims, ndims)))
     goto free_return;

   (*func) (rt, at->data, at->num_elements, parms);
   status = SLang_push_array (at, 0);
   SLang_free_array (at);
   /* drop */

   free_return:
   if (mmt != NULL)
     SLang_free_mmt (mmt);

   return status;
}

/* The calling syntax for the generators is:
 *   rand_foo ([Rand_Type,] args... [,num]);
 */
static int check_stack_args (int num_args, int num_parms, char *usage, int *nargsp)
{
   if ((num_args < num_parms) || (num_args > num_parms + 2))
     goto usage_error;

   *nargsp = num_args - num_parms;

   if ((num_args == num_parms) || (num_parms == 0))
     return 0;			       /* rand_foo (parms...) */

   if (num_args == num_parms + 2)
     {
	if (Rand_Type_Id != SLang_peek_at_stack_n (num_args-1))
	  goto usage_error;

	/* rand_foo (r, parms..., num) */
     }
   else if (Rand_Type_Id == SLang_peek_at_stack_n (num_args-1))
     return 0;		       /* rand_foo (r, parms...) */

   return SLroll_stack (num_parms + 1);

usage_error:
   SLang_verror (SL_Usage_Error, "Usage: %s", usage);
   return -1;
}

/*}}}*/

static void urand_intrin (void) /*{{{*/
{
   char *usage = "r = rand_uniform ([Rand_Type] [num])";
   int is_scalar;
   int nargs;
   double d;

   if (-1 == check_stack_args (SLang_Num_Function_Args, 0, usage, &nargs))
     return;

   if (-1 == do_xxxrand (nargs, SLANG_DOUBLE_TYPE,
			 generate_random_doubles, NULL, &is_scalar, &d))
     return;

   if (is_scalar)
     (void) SLang_push_double (d);
}

/*}}}*/

static void urand_pos_intrin (void) /*{{{*/
{
   char *usage = "r = rand_uniform_pos ([Rand_Type] [num])";
   int is_scalar;
   int nargs;
   double d;

   if (-1 == check_stack_args (SLang_Num_Function_Args, 0, usage, &nargs))
     return;

   if (-1 == do_xxxrand (nargs, SLANG_DOUBLE_TYPE,
			 generate_random_open_doubles, NULL, &is_scalar, &d))
     return;

   if (is_scalar)
     (void) SLang_push_double (d);
}

/*}}}*/

static void rand_intrin (void) /*{{{*/
{
   char *usage = "r = rand ([Rand_Type] [num])";
   int is_scalar;
   int nargs;
   uint32 u;

   if (-1 == check_stack_args (SLang_Num_Function_Args, 0, usage, &nargs))
     return;

   if (-1 == do_xxxrand (nargs, UINT32_TYPE,
			 generate_random_uints, NULL, &is_scalar, &u))
     return;
   if (is_scalar)
     (void) PUSH_UINT32 (u);
}

/*}}}*/

static void rand_gauss_intrin (void) /*{{{*/
{
   char *usage = "r = rand_gauss ([Rand_Type,] sigma [,num])";
   int is_scalar;
   double d, sigma;
   int nargs;

   if (-1 == check_stack_args (SLang_Num_Function_Args, 1, usage, &nargs))
     return;

   if (-1 == SLang_pop_double (&sigma))
     return;
   sigma = fabs(sigma);

   if (-1 == do_xxxrand (nargs, SLANG_DOUBLE_TYPE,
			 generate_gaussian_randoms, (VOID_STAR) &sigma,
			 &is_scalar, &d))
     return;
   if (is_scalar)
     (void) SLang_push_double (d);
}

/*}}}*/

static void rand_beta_intrin (void) /*{{{*/
{
   char *usage = "r = rand_beta ([Rand_Type,] a, b [,num])";
   int is_scalar;
   double d;
   double parms[2];
   int nargs;

   if (-1 == check_stack_args (SLang_Num_Function_Args, 2, usage, &nargs))
     return;

   if ((-1 == SLang_pop_double (parms+1))
       || (-1 == SLang_pop_double (parms)))
     return;

   if ((parms[0] <= 0.0) || (parms[1] <= 0.0))
     {
	SLang_verror (SL_Domain_Error, "rand_beta parameters must be > 0");
	return;
     }

   if (-1 == do_xxxrand (nargs, SLANG_DOUBLE_TYPE,
			 generate_beta_randoms, (VOID_STAR) parms,
			 &is_scalar, &d))
     return;
   if (is_scalar)
     (void) SLang_push_double (d);
}

/*}}}*/

static void rand_cauchy_intrin (void) /*{{{*/
{
   char *usage = "r = rand_cauchy ([Rand_Type,] gamma, [,num])";
   int is_scalar;
   double d;
   double a;
   int nargs;

   if (-1 == check_stack_args (SLang_Num_Function_Args, 1, usage, &nargs))
     return;

   if (-1 == SLang_pop_double (&a))
     return;

   a = fabs(a);

   if (-1 == do_xxxrand (nargs, SLANG_DOUBLE_TYPE,
			 generate_cauchy_randoms, (VOID_STAR) &a,
			 &is_scalar, &d))
     return;
   if (is_scalar)
     (void) SLang_push_double (d);
}

/*}}}*/

static void rand_geometric_intrin (void) /*{{{*/
{
   char *usage = "r = rand_geometric ([Rand_Type,] p, [,num])";
   int is_scalar;
   double p;
   unsigned int d;
   int nargs;

   if (-1 == check_stack_args (SLang_Num_Function_Args, 1, usage, &nargs))
     return;

   if (-1 == SLang_pop_double (&p))
     return;

   if ((p < 0.0) || (p > 1.0))
     {
	SLang_verror (SL_Domain_Error, "rand_geometric parameter must be beteen 0 and 1");
	return;
     }

   if (-1 == do_xxxrand (nargs, SLANG_UINT_TYPE,
			 generate_geometric_randoms, (VOID_STAR) &p,
			 &is_scalar, &d))
     return;
   if (is_scalar)
     (void) SLang_push_uint (d);
}

/*}}}*/

static void rand_poisson_intrin (void) /*{{{*/
{
   char *usage = "r = rand_poisson ([Rand_Type,] mu [,num])";
   unsigned int p;
   int is_scalar;
   int nargs;
   double mu;

   if (-1 == check_stack_args (SLang_Num_Function_Args, 1, usage, &nargs))
     return;

   if (-1 == SLang_pop_double (&mu))
     return;

   if (mu < 0.0)
     SLang_verror (SL_InvalidParm_Error, "The poisson rate must be non-negative");

   if (-1 == do_xxxrand (nargs, SLANG_UINT_TYPE,
			 generate_poisson_randoms, (VOID_STAR) &mu,
			 &is_scalar, &p))
     return;

   if (is_scalar)
     (void) SLang_push_uint (p);
}

/*}}}*/

static void rand_gamma_intrin (void) /*{{{*/
{
   char *usage = "r = rand_gamma([Rand_Type,] k, theta [,num])";
   int is_scalar, nargs;
   double parms[2];
   double p, k, theta;

   if (-1 == check_stack_args (SLang_Num_Function_Args, 2, usage, &nargs))
     return;

   if ((-1 == SLang_pop_double (&theta))
       || (-1 == SLang_pop_double (&k)))
     return;

   if ((theta <= 0) || (k <= 0))
     {
	SLang_verror (SL_InvalidParm_Error, "rand_gamma assumes k,theta>0");
	return;
     }
   parms[0] = k;
   parms[1] = theta;

   if (-1 == do_xxxrand (nargs, SLANG_DOUBLE_TYPE,
			 generate_gamma_randoms, (VOID_STAR) parms,
			 &is_scalar, &p))
     return;

   if (is_scalar)
     (void) SLang_push_double (p);
}

/*}}}*/

static void rand_binomial_intrin (void) /*{{{*/
{
   char *usage = "r = rand_binomial ([Rand_Type,] p, n [,num])";
   int is_scalar, nargs;
   Binomial_Parms_Type bp;
   unsigned int u;
   int n;

   if (-1 == check_stack_args (SLang_Num_Function_Args, 2, usage, &nargs))
     return;

   if ((-1 == SLang_pop_int (&n))
       || (-1 == SLang_pop_double (&bp.p)))
     return;

   if ((n < 0) || (bp.p < 0.0) || (bp.p > 1.0))
     {
	SLang_verror (SL_InvalidParm_Error, "rand_binomial assumes 0<=p<=1 and n>=0");
	return;
     }
   bp.n = (unsigned int)n;

   if (-1 == do_xxxrand (nargs, SLANG_UINT_TYPE,
			 generate_binomial_randoms, (VOID_STAR) &bp,
			 &is_scalar, &u))
     return;

   if (is_scalar)
     (void) SLang_push_uint (u);
}

/*}}}*/

static void srand_intrin (void) /*{{{*/
{
   SLang_MMT_Type *mmt = NULL;
   Rand_Type *r = Default_Rand;
   unsigned long seeds[NUM_SEEDS];
   int nargs = SLang_Num_Function_Args;

   if (-1 == pop_seeds (seeds))
     return;

   if (nargs == 2)
     {
	if (NULL == (mmt = SLang_pop_mmt (Rand_Type_Id)))
	  return;
	r = (Rand_Type *)SLang_object_from_mmt (mmt);
     }

   if (r != NULL)
     seed_random (r, seeds);

   if (mmt != NULL)
     SLang_free_mmt (mmt);
}

/*}}}*/

static void rand_permutation_intrin (void)
{
   int nargs = SLang_Num_Function_Args;
   Rand_Type *rt = Default_Rand;
   SLang_MMT_Type *mmt = NULL;
   SLang_Array_Type *at = NULL;
   int *data;
   SLindex_Type i, n;

   switch (nargs)
     {
      default:
	SLang_verror (SL_Usage_Error, "Usage: p = rand_permutation([Rand_Type,], n)");
	return;

      case 2:
      case 1:
	if (-1 == SLang_pop_array_index (&n))
	  return;
	if (nargs == 2)
	  {
	     if (NULL == (mmt = SLang_pop_mmt (Rand_Type_Id)))
	       return;

	     if (NULL == (rt = (Rand_Type *) SLang_object_from_mmt (mmt)))
	       goto free_return;
	  }
     }

   if (n < 0)
     {
	SLang_verror (SL_InvalidParm_Error, "rand_permutation: expected n>=0");
	goto free_return;
     }

   if (NULL == (at = SLang_create_array (SLANG_INT_TYPE, 0, NULL, &n, 1)))
     goto free_return;

   data = (int *) at->data;
   for (i = 0; i < n; i++)
     data[i] = i;

   /* Fisher-Yates */
   while (n > 1)
     {
	int k, p;

	k = (int) (n*uniform_random (rt));     /* 0 <= k < n */
	n--;
	p = data[n];
	data[n] = data[k];
	data[k] = p;
     }

   (void) SLang_push_array (at, 0);

free_return:
   if (at != NULL)
     SLang_free_array (at);
   if (mmt != NULL)
     SLang_free_mmt (mmt);
}

static SLang_Intrin_Fun_Type Module_Intrinsics [] =
{
   MAKE_INTRINSIC_0("rand", rand_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("srand", srand_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("rand_uniform", urand_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("rand_uniform_pos", urand_pos_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("rand_gauss", rand_gauss_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("rand_poisson", rand_poisson_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("rand_gamma", rand_gamma_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("rand_binomial", rand_binomial_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("rand_beta", rand_beta_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("rand_cauchy", rand_cauchy_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("rand_geometric", rand_geometric_intrin, SLANG_VOID_TYPE),

   MAKE_INTRINSIC_0("rand_permutation", rand_permutation_intrin, SLANG_VOID_TYPE),

   MAKE_INTRINSIC_0("rand_new", new_rand_intrin, SLANG_VOID_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

int init_rand_module_ns (char *ns_name)
{
   SLang_Class_Type *cl;

   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if (Default_Rand == NULL)
     {
	unsigned long seeds[NUM_SEEDS];
	generate_seeds (seeds);
	Default_Rand = create_random (seeds);
	if (Default_Rand == NULL)
	  return -1;

	init_poisson ();
     }

   if (Rand_Type_Id == -1)
     {
	if (NULL == (cl = SLclass_allocate_class ("Rand_Type")))
	  return -1;

	(void) SLclass_set_destroy_function (cl, destroy_rand_type);

	if (-1 == SLclass_register_class (cl, SLANG_VOID_TYPE,
					  sizeof (Rand_Type),
					  SLANG_CLASS_TYPE_MMT))
	  return -1;

	Rand_Type_Id = SLclass_get_class_id (cl);
     }

   if (-1 == SLns_add_intrin_fun_table (ns, Module_Intrinsics, NULL))
     return -1;

   return 0;
}

/* This function is optional */
void deinit_rand_module (void)
{
}
