/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
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

  Header file for regression routine(s)

  */

#ifndef GOT_REGRESS_H
#define GOT_REGRESS_H

extern void
RGR_WeightedRegression
(double *x,                     /* independent variable */
 double *y,                     /* measured data */
 double *w,                     /* weightings (large => data
                                   less reliable) */
 
 int n,                         /* number of data points */

 /* And now the results */

 double *b0,                    /* estimated y axis intercept */
 double *b1,                    /* estimated slope */
 double *s2,                    /* estimated variance (weighted) of
                                   data points */
 
 double *sb0,                   /* estimated standard deviation of
                                   intercept */
 double *sb1                    /* estimated standard deviation of
                                   slope */

 /* Could add correlation stuff later if required */
);

/* Return the weighting to apply to the standard deviation to get a
   given size of confidence interval assuming a T distribution */

extern double RGR_GetTCoef(int dof);

/* Return the value to apply to the variance to make an upper one-sided
   test assuming a chi-square distribution. */

extern double RGR_GetChi2Coef(int dof);

/* Maximum ratio of number of points used for runs test to number of regression
   points */
#define REGRESS_RUNS_RATIO 2

/* Minimum number of samples for regression */
#define MIN_SAMPLES_FOR_REGRESS 3

/* Return a status indicating whether there were enough points to
   carry out the regression */

extern int
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

);

int
RGR_FindBestRobustRegression
(double *x,
 double *y,
 int n,
 double tol,
 double *b0,
 double *b1,
 int *n_runs,
 int *best_start);

int
RGR_MultipleRegress
(double *x1,                    /* first independent variable */
 double *x2,                    /* second independent variable */
 double *y,                     /* measured data */

 int n,                         /* number of data points */

 /* The results */
 double *b2                     /* estimated second slope */
);

/* Return the median value from an array */
extern double RGR_FindMedian(double *x, int n);

#endif /* GOT_REGRESS_H */
