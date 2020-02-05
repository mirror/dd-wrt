/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
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

  Routines for implementing manual input of real time.

  The daemon accepts manual time input over the control connection,
  and adjusts the system time to match.  Besides this, though, it can
  determine the average rate of time loss or gain of the local system
  and adjust the frequency accordingly.

  */

#include "config.h"

#include "sysincl.h"

#include "manual.h"
#include "logging.h"
#include "local.h"
#include "conf.h"
#include "util.h"
#include "ntp.h"
#include "reference.h"
#include "regress.h"

static int enabled = 0;

/* More recent samples at highest indices */
typedef struct {
  struct timespec when; /* This is our 'cooked' time */
  double orig_offset; /*+ Not modified by slew samples */
  double offset; /*+ if we are fast of the supplied reference */
  double residual; /*+ regression residual (sign convention given by
                     (measured-predicted)) */
} Sample;

#define MIN_SAMPLE_SEPARATION 1.0

#define MAX_SAMPLES 16

static Sample samples[16];
static int n_samples;

/* ================================================== */

static void
slew_samples(struct timespec *raw,
             struct timespec *cooked,
             double dfreq,
             double doffset,
             LCL_ChangeType change_type,
             void *not_used);

/* ================================================== */

void
MNL_Initialise(void)
{
  if (CNF_GetManualEnabled()) {
    enabled = 1;
  } else {
    enabled = 0;
  }

  n_samples = 0;

  LCL_AddParameterChangeHandler(slew_samples, NULL);
}

/* ================================================== */

void
MNL_Finalise(void)
{
}

/* ================================================== */

static void
estimate_and_set_system(struct timespec *now, int offset_provided, double offset,
                        double *reg_offset, double *dfreq_ppm, double *new_afreq_ppm)
{
  double agos[MAX_SAMPLES], offsets[MAX_SAMPLES];
  double b0, b1;
  int n_runs, best_start; /* Unused results from regression analyser */
  int i;
  double freq = 0.0;
  double skew = 0.099999999; /* All 9's when printed to log file */
  int found_freq;
  double slew_by;

  b0 = offset_provided ? offset : 0.0;
  b1 = freq = 0.0;
  found_freq = 0;

  if (n_samples > 1) {
    for (i=0; i<n_samples; i++) {
      agos[i] = UTI_DiffTimespecsToDouble(&samples[n_samples - 1].when, &samples[i].when);
      offsets[i] = samples[i].offset;
    }
    
    if (RGR_FindBestRobustRegression(agos, offsets, n_samples, 1.0e-8,
                                     &b0, &b1, &n_runs, &best_start)) {
      /* Ignore b0 from regression; treat offset as being the most
         recently entered value.  (If the administrator knows he's put
         an outlier in, he will rerun the settime operation.)   However,
         the frequency estimate comes from the regression. */
      freq = -b1;
      found_freq = 1;
    }
  } else {
    agos[0] = 0.0;
    offsets[0] = b0;
  }

  if (offset_provided) {
    slew_by = offset;
  } else {
    slew_by = b0;
  }
  
  if (found_freq) {
    LOG(LOGS_INFO, "Making a frequency change of %.3f ppm and a slew of %.6f",
        1.0e6 * freq, slew_by);
    
    REF_SetManualReference(now,
                           slew_by,
                           freq, skew);
  } else {
    LOG(LOGS_INFO, "Making a slew of %.6f", slew_by);
    REF_SetManualReference(now,
                           slew_by,
                           0.0, skew);
  }
  
  if (reg_offset) *reg_offset = b0;
  if (dfreq_ppm) *dfreq_ppm = 1.0e6 * freq;
  if (new_afreq_ppm) *new_afreq_ppm = LCL_ReadAbsoluteFrequency();
  
  /* Calculate residuals to store them */
  for (i=0; i<n_samples; i++) {
    samples[i].residual = offsets[i] - (b0 + agos[i] * b1);
  }
  
}

/* ================================================== */

int
MNL_AcceptTimestamp(struct timespec *ts, double *reg_offset, double *dfreq_ppm, double *new_afreq_ppm)
{
  struct timespec now;
  double offset, diff;
  int i;

  if (enabled) {
    LCL_ReadCookedTime(&now, NULL);

    /* Make sure the provided timestamp is sane and the sample
       is not too close to the last one */

    if (!UTI_IsTimeOffsetSane(ts, 0.0))
     return 0;

    if (n_samples) {
      diff = UTI_DiffTimespecsToDouble(&now, &samples[n_samples - 1].when);
      if (diff < MIN_SAMPLE_SEPARATION)
        return 0;
    }

    offset = UTI_DiffTimespecsToDouble(&now, ts);

    /* Check if buffer full up */
    if (n_samples == MAX_SAMPLES) {
      /* Shift samples down */
      for (i=1; i<n_samples; i++) {
        samples[i-1] = samples[i];
      }
      --n_samples;
    }
    
    samples[n_samples].when = now;
    samples[n_samples].offset = offset;
    samples[n_samples].orig_offset = offset;
    ++n_samples;

    estimate_and_set_system(&now, 1, offset, reg_offset, dfreq_ppm, new_afreq_ppm);

    return 1;

  } else {
  
    return 0;

  }
}

/* ================================================== */

static void
slew_samples(struct timespec *raw,
             struct timespec *cooked,
             double dfreq,
             double doffset,
             LCL_ChangeType change_type,
             void *not_used)
{
  double delta_time;
  int i;

  if (change_type == LCL_ChangeUnknownStep) {
    MNL_Reset();
  }

  for (i=0; i<n_samples; i++) {
    UTI_AdjustTimespec(&samples[i].when, cooked, &samples[i].when, &delta_time,
        dfreq, doffset);
    samples[i].offset += delta_time;
  }
}

/* ================================================== */

void
MNL_Enable(void)
{
  enabled = 1;
}


/* ================================================== */

void
MNL_Disable(void)
{
  enabled = 0;
}

/* ================================================== */

void
MNL_Reset(void)
{
  n_samples = 0;
}

/* ================================================== */

int
MNL_IsEnabled(void)
{
  return enabled;
}

/* ================================================== */
/* Generate report data for the REQ_MANUAL_LIST command/monitoring
   protocol */

void
MNL_ReportSamples(RPT_ManualSamplesReport *report, int max, int *n)
{
  int i;

  if (n_samples > max) {
    *n = max;
  } else {
    *n = n_samples;
  }

  for (i=0; i<n_samples && i<max; i++) {
    report[i].when = samples[i].when;
    report[i].slewed_offset = samples[i].offset;
    report[i].orig_offset = samples[i].orig_offset;
    report[i].residual = samples[i].residual;
  }
}

/* ================================================== */
/* Delete a sample if it's within range, re-estimate the error and
   drift and apply it to the system clock. */

int
MNL_DeleteSample(int index)
{
  int i;
  struct timespec now;

  if ((index < 0) || (index >= n_samples)) {
    return 0;
  }

  /* Crunch the samples down onto the one being deleted */

  for (i=index; i<(n_samples-1); i++) {
    samples[i] = samples[i+1];
  }
  
  n_samples -= 1;

  /* Now re-estimate.  NULLs because we don't want the parameters back
     in this case. */
  LCL_ReadCookedTime(&now, NULL);
  estimate_and_set_system(&now, 0, 0.0, NULL, NULL, NULL);

  return 1;

}

/* ================================================== */


