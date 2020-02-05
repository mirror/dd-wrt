/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2016-2018
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

  Tracking of hardware clocks (e.g. RTC, PHC)
  */

#include "config.h"

#include "sysincl.h"

#include "array.h"
#include "hwclock.h"
#include "local.h"
#include "logging.h"
#include "memory.h"
#include "regress.h"
#include "util.h"

/* Minimum and maximum number of samples per clock */
#define MIN_SAMPLES 2
#define MAX_SAMPLES 64

/* Maximum acceptable frequency offset of the clock */
#define MAX_FREQ_OFFSET (2.0 / 3.0)

struct HCL_Instance_Record {
  /* HW and local reference timestamp */
  struct timespec hw_ref;
  struct timespec local_ref;

  /* Samples stored as intervals (uncorrected for frequency error)
     relative to local_ref and hw_ref */
  double *x_data;
  double *y_data;

  /* Minimum, maximum and current number of samples */
  int min_samples;
  int max_samples;
  int n_samples;

  /* Maximum error of the last sample */
  double last_err;

  /* Minimum interval between samples */
  double min_separation;

  /* Flag indicating the offset and frequency values are valid */
  int valid_coefs;

  /* Estimated offset and frequency of HW clock relative to local clock */
  double offset;
  double frequency;
};

/* ================================================== */

static void
handle_slew(struct timespec *raw, struct timespec *cooked, double dfreq,
            double doffset, LCL_ChangeType change_type, void *anything)
{
  HCL_Instance clock;
  double delta;

  clock = anything;

  if (clock->n_samples)
    UTI_AdjustTimespec(&clock->local_ref, cooked, &clock->local_ref, &delta, dfreq, doffset);
  if (clock->valid_coefs)
    clock->frequency /= 1.0 - dfreq;
}

/* ================================================== */

HCL_Instance
HCL_CreateInstance(int min_samples, int max_samples, double min_separation)
{
  HCL_Instance clock;

  min_samples = CLAMP(MIN_SAMPLES, min_samples, MAX_SAMPLES);
  max_samples = CLAMP(MIN_SAMPLES, max_samples, MAX_SAMPLES);
  max_samples = MAX(min_samples, max_samples);

  clock = MallocNew(struct HCL_Instance_Record);
  clock->x_data = MallocArray(double, max_samples);
  clock->y_data = MallocArray(double, max_samples);
  clock->x_data[max_samples - 1] = 0.0;
  clock->y_data[max_samples - 1] = 0.0;
  clock->min_samples = min_samples;
  clock->max_samples = max_samples;
  clock->n_samples = 0;
  clock->valid_coefs = 0;
  clock->min_separation = min_separation;

  LCL_AddParameterChangeHandler(handle_slew, clock);

  return clock;
}

/* ================================================== */

void HCL_DestroyInstance(HCL_Instance clock)
{
  LCL_RemoveParameterChangeHandler(handle_slew, clock);
  Free(clock->y_data);
  Free(clock->x_data);
  Free(clock);
}

/* ================================================== */

int
HCL_NeedsNewSample(HCL_Instance clock, struct timespec *now)
{
  if (!clock->n_samples ||
      fabs(UTI_DiffTimespecsToDouble(now, &clock->local_ref)) >= clock->min_separation)
    return 1;

  return 0;
}

/* ================================================== */

void
HCL_AccumulateSample(HCL_Instance clock, struct timespec *hw_ts,
                     struct timespec *local_ts, double err)
{
  double hw_delta, local_delta, local_freq, raw_freq;
  int i, n_runs, best_start;

  local_freq = 1.0 - LCL_ReadAbsoluteFrequency() / 1.0e6;

  /* Shift old samples */
  if (clock->n_samples) {
    if (clock->n_samples >= clock->max_samples)
      clock->n_samples--;

    hw_delta = UTI_DiffTimespecsToDouble(hw_ts, &clock->hw_ref);
    local_delta = UTI_DiffTimespecsToDouble(local_ts, &clock->local_ref) / local_freq;

    if (hw_delta <= 0.0 || local_delta < clock->min_separation / 2.0) {
      clock->n_samples = 0;
      DEBUG_LOG("HW clock reset interval=%f", local_delta);
    }

    for (i = clock->max_samples - clock->n_samples; i < clock->max_samples; i++) {
      clock->y_data[i - 1] = clock->y_data[i] - hw_delta;
      clock->x_data[i - 1] = clock->x_data[i] - local_delta;
    }
  }

  clock->n_samples++;
  clock->hw_ref = *hw_ts;
  clock->local_ref = *local_ts;
  clock->last_err = err;

  /* Get new coefficients */
  clock->valid_coefs =
    RGR_FindBestRobustRegression(clock->x_data + clock->max_samples - clock->n_samples,
                                 clock->y_data + clock->max_samples - clock->n_samples,
                                 clock->n_samples, 1.0e-10, &clock->offset, &raw_freq,
                                 &n_runs, &best_start);

  if (!clock->valid_coefs) {
    DEBUG_LOG("HW clock needs more samples");
    return;
  }

  clock->frequency = raw_freq / local_freq;

  /* Drop unneeded samples */
  if (clock->n_samples > clock->min_samples)
    clock->n_samples -= MIN(best_start, clock->n_samples - clock->min_samples);

  /* If the fit doesn't cross the error interval of the last sample,
     or the frequency is not sane, drop all samples and start again */
  if (fabs(clock->offset) > err ||
      fabs(clock->frequency - 1.0) > MAX_FREQ_OFFSET) {
    DEBUG_LOG("HW clock reset");
    clock->n_samples = 0;
    clock->valid_coefs = 0;
  }

  DEBUG_LOG("HW clock samples=%d offset=%e freq=%e raw_freq=%e err=%e ref_diff=%e",
            clock->n_samples, clock->offset, clock->frequency - 1.0, raw_freq - 1.0, err,
            UTI_DiffTimespecsToDouble(&clock->hw_ref, &clock->local_ref));
}

/* ================================================== */

int
HCL_CookTime(HCL_Instance clock, struct timespec *raw, struct timespec *cooked, double *err)
{
  double offset, elapsed;

  if (!clock->valid_coefs)
    return 0;

  elapsed = UTI_DiffTimespecsToDouble(raw, &clock->hw_ref);
  offset = elapsed / clock->frequency - clock->offset;
  UTI_AddDoubleToTimespec(&clock->local_ref, offset, cooked);

  /* Fow now, just return the error of the last sample */
  if (err)
    *err = clock->last_err;

  return 1;
}
