/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2022
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

  Estimation of quantiles using the Frugal-2U streaming algorithm
  (https://arxiv.org/pdf/1407.1121v1.pdf)
  */

#include "config.h"

#include "logging.h"
#include "memory.h"
#include "quantiles.h"
#include "regress.h"
#include "util.h"

/* Maximum number of repeated estimates for stabilisation */
#define MAX_REPEAT 64

struct Quantile {
  double est;
  double step;
  int sign;
};

struct QNT_Instance_Record {
  struct Quantile *quants;
  int n_quants;
  int repeat;
  int q;
  int min_k;
  double min_step;
  double neg_step_limit;
  int n_set;
};

/* ================================================== */

QNT_Instance
QNT_CreateInstance(int min_k, int max_k, int q, int repeat,
                   int large_step_delay, double min_step)
{
  QNT_Instance inst;
  long seed;

  BRIEF_ASSERT(q >= 2 && min_k <= max_k && min_k >= 1 && max_k < q && repeat >= 1 &&
               repeat <= MAX_REPEAT && min_step > 0.0 && large_step_delay >= 0);

  inst = MallocNew(struct QNT_Instance_Record);
  inst->n_quants = (max_k - min_k + 1) * repeat;
  inst->quants = MallocArray(struct Quantile, inst->n_quants);
  inst->repeat = repeat;
  inst->q = q;
  inst->min_k = min_k;
  inst->min_step = min_step;
  inst->neg_step_limit = -large_step_delay * min_step;

  QNT_Reset(inst);

  /* Seed the random number generator, which will not be isolated from
     other instances and other random() users */
  UTI_GetRandomBytes(&seed, sizeof (seed));
  srandom(seed);

  return inst;
}

/* ================================================== */

void
QNT_DestroyInstance(QNT_Instance inst)
{
  Free(inst->quants);
  Free(inst);
}

/* ================================================== */

void
QNT_Reset(QNT_Instance inst)
{
  int i;

  inst->n_set = 0;

  for (i = 0; i < inst->n_quants; i++) {
    inst->quants[i].est = 0.0;
    inst->quants[i].step = inst->min_step;
    inst->quants[i].sign = 1;
  }
}

/* ================================================== */

static void
insert_initial_value(QNT_Instance inst, double value)
{
  int i, j, r = inst->repeat;

  BRIEF_ASSERT(inst->n_set * r < inst->n_quants);

  /* Keep the initial estimates repeated and ordered */
  for (i = inst->n_set; i > 0 && inst->quants[(i - 1) * r].est > value; i--) {
    for (j = 0; j < r; j++)
      inst->quants[i * r + j].est = inst->quants[(i - 1) * r].est;
  }

  for (j = 0; j < r; j++)
    inst->quants[i * r + j].est = value;
  inst->n_set++;

  /* Duplicate the largest value in unset quantiles */
  for (i = inst->n_set * r; i < inst->n_quants; i++)
    inst->quants[i].est = inst->quants[i - 1].est;
}

/* ================================================== */

static void
update_estimate(struct Quantile *quantile, double value, double p, double rand,
                double min_step, double neg_step_limit)
{
  if (value >= quantile->est) {
    if (rand < (1.0 - p))
        return;
    quantile->step += quantile->sign > 0 ? min_step : -min_step;
    quantile->est += quantile->step > min_step ? quantile->step : min_step;
    if (quantile->est > value) {
      quantile->step += value - quantile->est;
      quantile->est = value + min_step / 4.0;
    }
    if (quantile->sign < 0 && quantile->step > min_step)
      quantile->step = min_step;
    quantile->sign = 1;
  } else {
    if (rand < p)
      return;
    quantile->step += quantile->sign < 0 ? min_step : -min_step;
    quantile->est -= quantile->step > min_step ? quantile->step : min_step;
    if (quantile->est < value) {
      quantile->step += quantile->est - value;
      quantile->est = value - min_step / 4.0;
    }
    if (quantile->sign > 0 && quantile->step > min_step)
      quantile->step = min_step;
    quantile->sign = -1;
  }

  if (quantile->step < neg_step_limit)
    quantile->step = neg_step_limit;
}

/* ================================================== */

void
QNT_Accumulate(QNT_Instance inst, double value)
{
  double p, rand;
  int i;

  /* Initialise the estimates with first received values */
  if (inst->n_set * inst->repeat < inst->n_quants) {
    insert_initial_value(inst, value);
    return;
  }

  for (i = 0; i < inst->n_quants; i++) {
    p = (double)(i / inst->repeat + inst->min_k) / inst->q;
    rand = (double)random() / ((1U << 31) - 1);

    update_estimate(&inst->quants[i], value, p, rand, inst->min_step, inst->neg_step_limit);
  }
}

/* ================================================== */

int
QNT_GetMinK(QNT_Instance inst)
{
  return inst->min_k;
}

/* ================================================== */

int
QNT_GetMaxK(QNT_Instance inst)
{
  return inst->min_k + (inst->n_quants / inst->repeat) - 1;
}

/* ================================================== */

double
QNT_GetMinStep(QNT_Instance inst)
{
  return inst->min_step;
}

/* ================================================== */

double
QNT_GetQuantile(QNT_Instance inst, int k)
{
  double estimates[MAX_REPEAT];
  int i;

  BRIEF_ASSERT(k >= inst->min_k && (k - inst->min_k) * inst->repeat < inst->n_quants);

  for (i = 0; i < inst->repeat; i++)
    estimates[i] = inst->quants[(k - inst->min_k) * inst->repeat + i].est;

  return RGR_FindMedian(estimates, inst->repeat);
}
