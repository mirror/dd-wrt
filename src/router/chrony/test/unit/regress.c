/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2017
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
 */
#include <regress.c>
#include "test.h"

#define POINTS 64

void
test_unit(void)
{
  double x[POINTS], x2[POINTS], y[POINTS], w[POINTS];
  double b0, b1, b2, s2, sb0, sb1, slope, slope2, intercept, sd, median;
  double xrange, yrange, wrange, x2range;
  int i, j, n, m, c1, c2, c3, runs, best_start, dof;

  for (n = 3; n <= POINTS; n++) {
    for (i = 0; i < 200; i++) {
      slope = TST_GetRandomDouble(-0.1, 0.1);
      intercept = TST_GetRandomDouble(-1.0, 1.0);
      sd = TST_GetRandomDouble(1e-6, 1e-4);
      slope2 = (random() % 2 ? 1 : -1) * TST_GetRandomDouble(0.1, 0.5);

      DEBUG_LOG("iteration %d n=%d intercept=%e slope=%e sd=%e",
                i, n, intercept, slope, sd);

      for (j = 0; j < n; j++) {
        x[j] = -j;
        y[j] = intercept + slope * x[j] + (j % 2 ? 1 : -1) * TST_GetRandomDouble(1e-6, sd);
        w[j] = TST_GetRandomDouble(1.0, 2.0);
        x2[j] = (y[j] - intercept - slope * x[j]) / slope2;
      }

      RGR_WeightedRegression(x, y, w, n, &b0, &b1, &s2, &sb0, &sb1);
      DEBUG_LOG("WR b0=%e b1=%e s2=%e sb0=%e sb1=%e", b0, b1, s2, sb0, sb1);
      TEST_CHECK(fabs(b0 - intercept) < sd + 1e-3);
      TEST_CHECK(fabs(b1 - slope) < sd);

      if (RGR_FindBestRegression(x, y, w, n, 0, 3, &b0, &b1, &s2, &sb0, &sb1,
                                 &best_start, &runs, &dof)) {
        DEBUG_LOG("BR b0=%e b1=%e s2=%e sb0=%e sb1=%e runs=%d bs=%d dof=%d",
                  b0, b1, s2, sb0, sb1, runs, best_start, dof);

        TEST_CHECK(fabs(b0 - intercept) < sd + 1e-3);
        TEST_CHECK(fabs(b1 - slope) < sd);
      }

      if (RGR_MultipleRegress(x, x2, y, n, &b2)) {
        DEBUG_LOG("MR b2=%e", b2);
        TEST_CHECK(fabs(b2 - slope2) < 1e-6);
      }

      for (j = 0; j < n / 7; j++)
        y[random() % n] += 100 * sd;

      if (RGR_FindBestRobustRegression(x, y, n, 1e-8, &b0, &b1, &runs, &best_start)) {
        DEBUG_LOG("BRR b0=%e b1=%e runs=%d bs=%d", b0, b1, runs, best_start);

        TEST_CHECK(fabs(b0 - intercept) < sd + 1e-2);
        TEST_CHECK(fabs(b1 - slope) < 5.0 * sd);
      }

      for (j = 0; j < n; j++)
        x[j] = random() % 4 * TST_GetRandomDouble(-1000, 1000);

      median = RGR_FindMedian(x, n);

      for (j = c1 = c2 = c3 = 0; j < n; j++) {
        if (x[j] < median)
          c1++;
        if (x[j] > median)
          c3++;
        else
          c2++;
      }

      TEST_CHECK(c1 + c2 >= c3 && c1 <= c2 + c3);

      xrange = TST_GetRandomDouble(1e-6, pow(10.0, random() % 10));
      yrange = random() % 3 * TST_GetRandomDouble(0.0, pow(10.0, random() % 10));
      wrange = random() % 3 * TST_GetRandomDouble(0.0, pow(10.0, random() % 10));
      x2range = random() % 3 * TST_GetRandomDouble(0.0, pow(10.0, random() % 10));
      m = random() % n;

      for (j = 0; j < n; j++) {
        x[j] = (j ? x[j - 1] : 0.0) + TST_GetRandomDouble(1e-6, xrange);
        y[j] = TST_GetRandomDouble(-yrange, yrange);
        w[j] = 1.0 + TST_GetRandomDouble(0.0, wrange);
        x2[j] = TST_GetRandomDouble(-x2range, x2range);
      }

      RGR_WeightedRegression(x, y, w, n, &b0, &b1, &s2, &sb0, &sb1);

      if (RGR_FindBestRegression(x + m, y + m, w, n - m, m, 3, &b0, &b1, &s2, &sb0, &sb1,
                                 &best_start, &runs, &dof))
        ;
      if (RGR_MultipleRegress(x, x2, y, n, &b2))
        ;
      if (RGR_FindBestRobustRegression(x, y, n, 1e-8, &b0, &b1, &runs, &best_start))
        ;
    }
  }
}
