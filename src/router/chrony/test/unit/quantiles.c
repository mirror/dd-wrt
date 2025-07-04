/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2022, 2025
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

#include <local.h>
#include "test.h"

#include <quantiles.c>

void
test_unit(void)
{
  int i, j, k, min_k, max_k, q, r, in_order, out_order;
  QNT_Instance inst;
  double x, x2;

  in_order = out_order = 0;

  for (i = 0; i < 100; i++) {
    r = random() % 10 + 1;
    q = random() % 20 + 2;
    do {
      min_k = random() % (q - 1) + 1;
      max_k = random() % (q - 1) + 1;
    } while (min_k > max_k);

    inst = QNT_CreateInstance(min_k, max_k, q, r, 900, 1e-9);

    TEST_CHECK(min_k == QNT_GetMinK(inst));
    TEST_CHECK(max_k == QNT_GetMaxK(inst));
    TEST_CHECK(fabs(QNT_GetMinStep(inst) - 1e-9) < 1e-12);

    for (j = 0; j < 3000; j++) {
      x = TST_GetRandomDouble(0.0, 2e-6);
      QNT_Accumulate(inst, x);
      for (k = min_k; k < max_k; k++)
        if (j < max_k - min_k) {
          TEST_CHECK(QNT_GetQuantile(inst, k) <= QNT_GetQuantile(inst, k + 1));
        } else if (j > 1000) {
          if (QNT_GetQuantile(inst, k) <= QNT_GetQuantile(inst, k + 1))
            in_order++;
          else
            out_order++;
        }
    }

    QNT_Reset(inst);
    TEST_CHECK(inst->n_set == 0);

    for (j = 0; j < 3; j++) {
      QNT_Reset(inst);
      x = (i + 950) / 1e9;
      if (random() % 10)
        x2 = (i + 951) / 1e9;
      else
        x2 = x;
      for (k = 0; k < 1000; k++)
        QNT_Accumulate(inst, random() % 2 ? x : x2);
      for (k = 0; k < inst->n_quants; k++) {
        TEST_CHECK(inst->quants[k].est > x - 0.4e-9);
        TEST_CHECK(inst->quants[k].est < x2 + 0.4e-9);
        TEST_CHECK(inst->quants[k].step < -15e-9);
        TEST_CHECK(inst->quants[k].step > -901e-9);
        if (min_k * 2 == q && k < inst->repeat) {
          if (x == x2) {
            TEST_CHECK(inst->quants[k].step < -750e-9);
            TEST_CHECK(inst->quants[k].step > -901e-9);
          } else {
            TEST_CHECK(inst->quants[k].step < -350e-9);
            TEST_CHECK(inst->quants[k].step > -600e-9);
          }
        }
      }
    }

    QNT_DestroyInstance(inst);
  }

  TEST_CHECK(in_order > 100 * out_order);
}
