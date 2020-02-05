/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2011, 2016-2017
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Regression algorithms.

  */

#include "config.h"

#include "sysincl.h"

#include "regress.h"
#include "logging.h"
#include "util.h"

#define MAX_POINTS 64

void
RGR_WeightedRegression
(double *x,                     /* independent variable */
 double *y,                     /* measured data */
 double *w,                     /* weightings (large => data
                                   less reliable) */
 
 int n,                         /* number of data points */

 /* And now the results */

 double *b0,                    /* estimated y axis intercept */
 double *b1,                    /* estimated slope */
 double *s2,                    /* estimated variance of data points */
 
 double *sb0,                   /* estimated standard deviation of
                                   intercept */
 double *sb1                    /* estimated standard deviation of
                                   slope */

 /* Could add correlation stuff later if required */
)
{
  double P, Q, U, V, W;
  double diff;
  double u, ui, aa;
  int i;

  assert(n >= 3);

  W = U = 0;
  for (i=0; i<n; i++) {
    U += x[i]        / w[i];
    W += 1.0         / w[i];
  }

  u = U / W;

  /* Calculate statistics from data */
  P = Q = V = 0.0;
  for (i=0; i<n; i++) {
    ui = x[i] - u;
    P += y[i]        / w[i];
    Q += y[i] * ui   / w[i];
    V += ui   * ui   / w[i];
  }

  *b1 = Q / V;
  *b0 = (P / W) - (*b1) * u;

  *s2 = 0.0;
  for (i=0; i<n; i++) {
    diff = y[i] - *b0 - *b1*x[i];
    *s2 += diff*diff / w[i];
  }

  *s2 /= (double)(n-2);

  *sb1 = sqrt(*s2 / V);
  aa = u * (*sb1);
  *sb0 = sqrt(*s2 / W + aa * aa);

  *s2 *= (n / W); /* Giving weighted average of variances */
}

/* ================================================== */
/* Get the coefficient to multiply the standard deviation by, to get a
   particular size of confidence interval (assuming a t-distribution) */
  
double
RGR_GetTCoef(int dof)
{
  /* Assuming now the 99.95% quantile */
  static const float coefs[] =
  { 636.6, 31.6, 12.92, 8.61, 6.869,
    5.959, 5.408, 5.041, 4.781, 4.587,
    4.437, 4.318, 4.221, 4.140, 4.073,
    4.015, 3.965, 3.922, 3.883, 3.850,
    3.819, 3.792, 3.768, 3.745, 3.725,
    3.707, 3.690, 3.674, 3.659, 3.646,
    3.633, 3.622, 3.611, 3.601, 3.591,
    3.582, 3.574, 3.566, 3.558, 3.551};

  if (dof <= 40) {
    return coefs[dof-1];
  } else {
    return 3.5; /* Until I can be bothered to do something better */
  }
}

/* ================================================== */
/* Get 90% quantile of chi-square distribution */

double
RGR_GetChi2Coef(int dof)
{
  static const float coefs[] = {
    2.706, 4.605, 6.251, 7.779, 9.236, 10.645, 12.017, 13.362,
    14.684, 15.987, 17.275, 18.549, 19.812, 21.064, 22.307, 23.542,
    24.769, 25.989, 27.204, 28.412, 29.615, 30.813, 32.007, 33.196,
    34.382, 35.563, 36.741, 37.916, 39.087, 40.256, 41.422, 42.585,
    43.745, 44.903, 46.059, 47.212, 48.363, 49.513, 50.660, 51.805,
    52.949, 54.090, 55.230, 56.369, 57.505, 58.641, 59.774, 60.907,
    62.038, 63.167, 64.295, 65.422, 66.548, 67.673, 68.796, 69.919,
    71.040, 72.160, 73.279, 74.397, 75.514, 76.630, 77.745, 78.860
  };

  if (dof <= 64) {
    return coefs[dof-1];
  } else {
    return 1.2 * dof; /* Until I can be bothered to do something better */
  }
}

/* ================================================== */
/* Critical value for number of runs of residuals with same sign.
   5% critical region for now. */

static char critical_runs[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  2,  3,
  3,  3,  4,  4,  5,  5,  5,  6,  6,  7,
  7,  7,  8,  8,  9,  9,  9, 10, 10, 11,
 11, 11, 12, 12, 13, 13, 14, 14, 14, 15,
 15, 16, 16, 17, 17, 18, 18, 18, 19, 19,
 20, 20, 21, 21, 21, 22, 22, 23, 23, 24,
 24, 25, 25, 26, 26, 26, 27, 27, 28, 28,
 29, 29, 30, 30, 30, 31, 31, 32, 32, 33,
 33, 34, 34, 35, 35, 35, 36, 36, 37, 37,
 38, 38, 39, 39, 40, 40, 40, 41, 41, 42,
 42, 43, 43, 44, 44, 45, 45, 46, 46, 46,
 47, 47, 48, 48, 49, 49, 50, 50, 51, 51,
 52, 52, 52, 53, 53, 54, 54, 55, 55, 56
};

/* ================================================== */

static int
n_runs_from_residuals(double *resid, int n)
{
  int nruns;
  int i;
  
  nruns = 1;
  for (i=1; i<n; i++) {
    if (((resid[i-1] < 0.0) && (resid[i] < 0.0)) ||
        ((resid[i-1] > 0.0) && (resid[i] > 0.0))) {
      /* Nothing to do */
    } else {
      nruns++;
    }
  }
  
  return nruns;
}

/* ================================================== */
/* Return a boolean indicating whether we had enough points for
   regression */

int
RGR_FindBestRegression 
(double *x,                     /* independent variable */
 double *y,                     /* measured data */
 double *w,                     /* weightings (large => data
                                   less reliable) */
 
 int n,                         /* number of data points */
 int m,                         /* number of extra samples in x and y arrays
                                   (negative index) which can be used to
                                   extend runs test */
 int min_samples,               /* minimum number of samples to be kept after
                                   changing the starting index to pass the runs
                                   test */

 /* And now the results */

 double *b0,                    /* estimated y axis intercept */
 double *b1,                    /* estimated slope */
 double *s2,                    /* estimated variance of data points */
 
 double *sb0,                   /* estimated standard deviation of
                                   intercept */
 double *sb1,                   /* estimated standard deviation of
                                   slope */

 int *new_start,                /* the new starting index to make the
                                   residuals pass the two tests */
 
 int *n_runs,                   /* number of runs amongst the residuals */

 int *dof                       /* degrees of freedom in statistics (needed
                                   to get confidence intervals later) */

)
{
  double P, Q, U, V, W; /* total */
  double resid[MAX_POINTS * REGRESS_RUNS_RATIO];
  double ss;
  double a, b, u, ui, aa;

  int start, resid_start, nruns, npoints;
  int i;

  assert(n <= MAX_POINTS && m >= 0);
  assert(n * REGRESS_RUNS_RATIO < sizeof (critical_runs) / sizeof (critical_runs[0]));

  if (n < MIN_SAMPLES_FOR_REGRESS) {
    return 0;
  }

  start = 0;
  do {

    W = U = 0;
    for (i=start; i<n; i++) {
      U += x[i]        / w[i];
      W += 1.0         / w[i];
    }

    u = U / W;

    P = Q = V = 0.0;
    for (i=start; i<n; i++) {
      ui = x[i] - u;
      P += y[i]        / w[i];
      Q += y[i] * ui   / w[i];
      V += ui   * ui   / w[i];
    }

    b = Q / V;
    a = (P / W) - (b * u);

    /* Get residuals also for the extra samples before start */
    resid_start = n - (n - start) * REGRESS_RUNS_RATIO;
    if (resid_start < -m)
      resid_start = -m;

    for (i=resid_start; i<n; i++) {
      resid[i - resid_start] = y[i] - a - b*x[i];
    }

    /* Count number of runs */
    nruns = n_runs_from_residuals(resid, n - resid_start); 

    if (nruns > critical_runs[n - resid_start] ||
        n - start <= MIN_SAMPLES_FOR_REGRESS ||
        n - start <= min_samples) {
      if (start != resid_start) {
        /* Ignore extra samples in returned nruns */
        nruns = n_runs_from_residuals(resid + (start - resid_start), n - start);
      }
      break;
    } else {
      /* Try dropping one sample at a time until the runs test passes. */
      ++start;
    }

  } while (1);

  /* Work out statistics from full dataset */
  *b1 = b;
  *b0 = a;

  ss = 0.0;
  for (i=start; i<n; i++) {
    ss += resid[i - resid_start]*resid[i - resid_start] / w[i];
  }

  npoints = n - start;
  ss /= (double)(npoints - 2);
  *sb1 = sqrt(ss / V);
  aa = u * (*sb1);
  *sb0 = sqrt((ss / W) + (aa * aa));
  *s2 = ss * (double) npoints / W;

  *new_start = start;
  *dof = npoints - 2;
  *n_runs = nruns;

  return 1;

}

/* ================================================== */

#define EXCH(a,b) temp=(a); (a)=(b); (b)=temp

/* ================================================== */
/* Find the index'th biggest element in the array x of n elements.
   flags is an array where a 1 indicates that the corresponding entry
   in x is known to be sorted into its correct position and a 0
   indicates that the corresponding entry is not sorted.  However, if
   flags[m] = flags[n] = 1 with m<n, then x[m] must be <= x[n] and for
   all i with m<i<n, x[m] <= x[i] <= x[n].  In practice, this means
   flags[] has to be the result of a previous call to this routine
   with the same array x, and is used to remember which parts of the
   x[] array we have already sorted.

   The approach used is a cut-down quicksort, where we only bother to
   keep sorting the partition that contains the index we are after.
   The approach comes from Numerical Recipes in C (ISBN
   0-521-43108-5). */

static double
find_ordered_entry_with_flags(double *x, int n, int index, char *flags)
{
  int u, v, l, r;
  double temp;
  double piv;
  int pivind;

  assert(index >= 0);

  /* If this bit of the array is already sorted, simple! */
  if (flags[index]) {
    return x[index];
  }
  
  /* Find subrange to look at */
  u = v = index;
  while (u > 0 && !flags[u]) u--;
  if (flags[u]) u++;

  while (v < (n-1) && !flags[v]) v++;
  if (flags[v]) v--;

  do {
    if (v - u < 2) {
      if (x[v] < x[u]) {
        EXCH(x[v], x[u]);
      }
      flags[v] = flags[u] = 1;
      return x[index];
    } else { 
      pivind = (u + v) >> 1;
      EXCH(x[u], x[pivind]);
      piv = x[u]; /* New value */
      l = u + 1;
      r = v;
      do {
        while (l < v && x[l] < piv) l++;
        while (x[r] > piv) r--;
        if (r <= l) break;
        EXCH(x[l], x[r]);
        l++;
        r--;
      } while (1);
      EXCH(x[u], x[r]);
      flags[r] = 1; /* Pivot now in correct place */
      if (index == r) {
        return x[r];
      } else if (index < r) {
        v = r - 1;
      } else if (index > r) {
        u = l;
      }
    }
  } while (1);
}

/* ================================================== */

#if 0
/* Not used, but this is how it can be done */
static double
find_ordered_entry(double *x, int n, int index)
{
  char flags[MAX_POINTS];

  memset(flags, 0, n * sizeof (flags[0]));
  return find_ordered_entry_with_flags(x, n, index, flags);
}
#endif

/* ================================================== */
/* Find the median entry of an array x[] with n elements. */

static double
find_median(double *x, int n)
{
  int k;
  char flags[MAX_POINTS];

  memset(flags, 0, n * sizeof (flags[0]));
  k = n>>1;
  if (n&1) {
    return find_ordered_entry_with_flags(x, n, k, flags);
  } else {
    return 0.5 * (find_ordered_entry_with_flags(x, n, k, flags) +
                  find_ordered_entry_with_flags(x, n, k-1, flags));
  }
}

/* ================================================== */

double
RGR_FindMedian(double *x, int n)
{
  double tmp[MAX_POINTS];

  assert(n > 0 && n <= MAX_POINTS);
  memcpy(tmp, x, n * sizeof (tmp[0]));

  return find_median(tmp, n);
}

/* ================================================== */
/* This function evaluates the equation

   \sum_{i=0}^{n-1} x_i sign(y_i - a - b x_i)

   and chooses the value of a that minimises the absolute value of the
   result.  (See pp703-704 of Numerical Recipes in C). */

static void
eval_robust_residual
(double *x,                     /* The independent points */
 double *y,                     /* The dependent points */
 int n,                         /* Number of points */
 double b,                      /* Slope */
 double *aa,                    /* Intercept giving smallest absolute
                                   value for the above equation */
 double *rr                     /* Corresponding value of equation */
)
{
  int i;
  double a, res, del;
  double d[MAX_POINTS];

  for (i=0; i<n; i++) {
    d[i] = y[i] - b * x[i];
  }
  
  a = find_median(d, n);

  res = 0.0;
  for (i=0; i<n; i++) {
    del = y[i] - a - b * x[i];
    if (del > 0.0) {
      res += x[i];
    } else if (del < 0.0) {
      res -= x[i];
    }
  }

  *aa = a;
  *rr = res;
}

/* ================================================== */
/* This routine performs a 'robust' regression, i.e. one which has low
   susceptibility to outliers amongst the data.  If one thinks of a
   normal (least squares) linear regression in 2D being analogous to
   the arithmetic mean in 1D, this algorithm in 2D is roughly
   analogous to the median in 1D.  This algorithm seems to work quite
   well until the number of outliers is approximately half the number
   of data points.

   The return value is a status indicating whether there were enough
   data points to run the routine or not. */

int
RGR_FindBestRobustRegression
(double *x,                     /* The independent axis points */
 double *y,                     /* The dependent axis points (which
                                   may contain outliers). */
 int n,                         /* The number of points */
 double tol,                    /* The tolerance required in
                                   determining the value of b1 */
 double *b0,                    /* The estimated Y-axis intercept */
 double *b1,                    /* The estimated slope */
 int *n_runs,                   /* The number of runs of residuals */
 int *best_start                /* The best starting index */
)
{
  int i;
  int start;
  int n_points;
  double a, b;
  double P, U, V, W, X;
  double resid, resids[MAX_POINTS];
  double blo, bhi, bmid, rlo, rhi, rmid;
  double s2, sb, incr;
  double mx, dx, my, dy;
  int nruns = 0;

  assert(n <= MAX_POINTS);

  if (n < 2) {
    return 0;
  } else if (n == 2) {
    /* Just a straight line fit (we need this for the manual mode) */
    *b1 = (y[1] - y[0]) / (x[1] - x[0]);
    *b0 = y[0] - (*b1) * x[0];
    *n_runs = 0;
    *best_start = 0;
    return 1;
  }

  /* else at least 3 points, apply normal algorithm */

  start = 0;

  /* Loop to strip oldest points that cause the regression residuals
     to fail the number of runs test */
  do {

    n_points = n - start;

    /* Use standard least squares regression to get starting estimate */

    P = U = 0.0;
    for (i=start; i<n; i++) {
      P += y[i];
      U += x[i];
    }

    W = (double) n_points;

    my = P/W;
    mx = U/W;

    X = V = 0.0;
    for (i=start; i<n; i++) {
      dy = y[i] - my;
      dx = x[i] - mx;
      X += dy * dx;
      V += dx * dx;
    }

    b = X / V;
    a = my - b*mx;

    s2 = 0.0;
    for (i=start; i<n; i++) {
      resid = y[i] - a - b * x[i];
      s2 += resid * resid;
    }

    /* Need to expand range of b to get a root in the interval.
       Estimate standard deviation of b and expand range about b based
       on that. */
    sb = sqrt(s2 * W/V);
    incr = MAX(sb, tol);
  
    do {
      incr *= 2.0;

      /* Give up if the interval is too large */
      if (incr > 100.0)
        return 0;

      blo = b - incr;
      bhi = b + incr;

      /* We don't want 'a' yet */
      eval_robust_residual(x + start, y + start, n_points, blo, &a, &rlo);
      eval_robust_residual(x + start, y + start, n_points, bhi, &a, &rhi);

    } while (rlo * rhi >= 0.0); /* fn vals have same sign or one is zero,
                                   i.e. root not in interval (rlo, rhi). */

    /* OK, so the root for b lies in (blo, bhi). Start bisecting */
    do {
      bmid = 0.5 * (blo + bhi);
      if (!(blo < bmid && bmid < bhi))
        break;
      eval_robust_residual(x + start, y + start, n_points, bmid, &a, &rmid);
      if (rmid == 0.0) {
        break;
      } else if (rmid * rlo > 0.0) {
        blo = bmid;
        rlo = rmid;
      } else if (rmid * rhi > 0.0) {
        bhi = bmid;
        rhi = rmid;
      } else {
        assert(0);
      }
    } while (bhi - blo > tol);

    *b0 = a;
    *b1 = bmid;

    /* Number of runs test, but not if we're already down to the
       minimum number of points */
    if (n_points == MIN_SAMPLES_FOR_REGRESS) {
      break;
    }

    for (i=start; i<n; i++) {
      resids[i] = y[i] - a - bmid * x[i];
    }

    nruns = n_runs_from_residuals(resids + start, n_points);

    if (nruns > critical_runs[n_points]) {
      break;
    } else {
      start++;
    }

  } while (1);

  *n_runs = nruns;
  *best_start = start;

  return 1;

}

/* ================================================== */
/* This routine performs linear regression with two independent variables.
   It returns non-zero status if there were enough data points and there
   was a solution. */

int
RGR_MultipleRegress
(double *x1,                    /* first independent variable */
 double *x2,                    /* second independent variable */
 double *y,                     /* measured data */

 int n,                         /* number of data points */

 /* The results */
 double *b2                     /* estimated second slope */
                                /* other values are not needed yet */
)
{
  double Sx1, Sx2, Sx1x1, Sx1x2, Sx2x2, Sx1y, Sx2y, Sy;
  double U, V, V1, V2, V3;
  int i;

  if (n < 4)
    return 0;

  Sx1 = Sx2 = Sx1x1 = Sx1x2 = Sx2x2 = Sx1y = Sx2y = Sy = 0.0;

  for (i = 0; i < n; i++) {
    Sx1 += x1[i];
    Sx2 += x2[i];
    Sx1x1 += x1[i] * x1[i];
    Sx1x2 += x1[i] * x2[i];
    Sx2x2 += x2[i] * x2[i];
    Sx1y += x1[i] * y[i];
    Sx2y += x2[i] * y[i];
    Sy += y[i];
  }

  U = n * (Sx1x2 * Sx1y - Sx1x1 * Sx2y) +
      Sx1 * Sx1 * Sx2y - Sx1 * Sx2 * Sx1y +
      Sy * (Sx2 * Sx1x1 - Sx1 * Sx1x2);

  V1 = n * (Sx1x2 * Sx1x2 - Sx1x1 * Sx2x2);
  V2 = Sx1 * Sx1 * Sx2x2 + Sx2 * Sx2 * Sx1x1;
  V3 = -2.0 * Sx1 * Sx2 * Sx1x2;
  V = V1 + V2 + V3;

  /* Check if there is a (numerically stable) solution */
  if (fabs(V) * 1.0e10 <= -V1 + V2 + fabs(V3))
    return 0;

  *b2 = U / V;

  return 1;
}
