/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2009-2011, 2014, 2016, 2018
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

  Routines implementing a median sample filter.

  */

#include "config.h"

#include "local.h"
#include "logging.h"
#include "memory.h"
#include "regress.h"
#include "samplefilt.h"
#include "util.h"

#define MIN_SAMPLES 1
#define MAX_SAMPLES 256

struct SPF_Instance_Record {
  int min_samples;
  int max_samples;
  int index;
  int used;
  int last;
  int avg_var_n;
  double avg_var;
  double max_var;
  double combine_ratio;
  NTP_Sample *samples;
  int *selected;
  double *x_data;
  double *y_data;
  double *w_data;
};

/* ================================================== */

SPF_Instance
SPF_CreateInstance(int min_samples, int max_samples, double max_dispersion, double combine_ratio)
{
  SPF_Instance filter;

  filter = MallocNew(struct SPF_Instance_Record);

  min_samples = CLAMP(MIN_SAMPLES, min_samples, MAX_SAMPLES);
  max_samples = CLAMP(MIN_SAMPLES, max_samples, MAX_SAMPLES);
  max_samples = MAX(min_samples, max_samples);
  combine_ratio = CLAMP(0.0, combine_ratio, 1.0);

  filter->min_samples = min_samples;
  filter->max_samples = max_samples;
  filter->index = -1;
  filter->used = 0;
  filter->last = -1;
  /* Set the first estimate to the system precision */
  filter->avg_var_n = 0;
  filter->avg_var = SQUARE(LCL_GetSysPrecisionAsQuantum());
  filter->max_var = SQUARE(max_dispersion);
  filter->combine_ratio = combine_ratio;
  filter->samples = MallocArray(NTP_Sample, filter->max_samples);
  filter->selected = MallocArray(int, filter->max_samples);
  filter->x_data = MallocArray(double, filter->max_samples);
  filter->y_data = MallocArray(double, filter->max_samples);
  filter->w_data = MallocArray(double, filter->max_samples);

  return filter;
}

/* ================================================== */

void
SPF_DestroyInstance(SPF_Instance filter)
{
  Free(filter->samples);
  Free(filter->selected);
  Free(filter->x_data);
  Free(filter->y_data);
  Free(filter->w_data);
  Free(filter);
}

/* ================================================== */

/* Check that samples times are strictly increasing */

static int
check_sample(SPF_Instance filter, NTP_Sample *sample)
{
  if (filter->used <= 0)
    return 1;

  if (UTI_CompareTimespecs(&filter->samples[filter->last].time, &sample->time) >= 0) {
    DEBUG_LOG("filter non-increasing sample time %s", UTI_TimespecToString(&sample->time));
    return 0;
  }

  return 1;
}

/* ================================================== */

int
SPF_AccumulateSample(SPF_Instance filter, NTP_Sample *sample)
{
  if (!check_sample(filter, sample))
      return 0;

  filter->index++;
  filter->index %= filter->max_samples;
  filter->last = filter->index;
  if (filter->used < filter->max_samples)
    filter->used++;

  filter->samples[filter->index] = *sample;

  DEBUG_LOG("filter sample %d t=%s offset=%.9f peer_disp=%.9f",
            filter->index, UTI_TimespecToString(&sample->time),
            sample->offset, sample->peer_dispersion);
  return 1;
}

/* ================================================== */

int
SPF_GetLastSample(SPF_Instance filter, NTP_Sample *sample)
{
  if (filter->last < 0)
    return 0;

  *sample = filter->samples[filter->last];
  return 1;
}

/* ================================================== */

int
SPF_GetNumberOfSamples(SPF_Instance filter)
{
  return filter->used;
}

/* ================================================== */

double
SPF_GetAvgSampleDispersion(SPF_Instance filter)
{
  return sqrt(filter->avg_var);
}

/* ================================================== */

void
SPF_DropSamples(SPF_Instance filter)
{
  filter->index = -1;
  filter->used = 0;
}

/* ================================================== */

static const NTP_Sample *tmp_sort_samples;

static int
compare_samples(const void *a, const void *b)
{
  const NTP_Sample *s1, *s2;

  s1 = &tmp_sort_samples[*(int *)a];
  s2 = &tmp_sort_samples[*(int *)b];

  if (s1->offset < s2->offset)
    return -1;
  else if (s1->offset > s2->offset)
    return 1;
  return 0;
}

/* ================================================== */

static int
select_samples(SPF_Instance filter)
{
  int i, j, k, o, from, to, *selected;
  double min_dispersion;

  if (filter->used < filter->min_samples)
    return 0;

  selected = filter->selected;

  /* With 4 or more samples, select those that have peer dispersion smaller
     than 1.5x of the minimum dispersion */
  if (filter->used > 4) {
    for (i = 1, min_dispersion = filter->samples[0].peer_dispersion; i < filter->used; i++) {
      if (min_dispersion > filter->samples[i].peer_dispersion)
        min_dispersion = filter->samples[i].peer_dispersion;
    }

    for (i = j = 0; i < filter->used; i++) {
      if (filter->samples[i].peer_dispersion <= 1.5 * min_dispersion)
        selected[j++] = i;
    }
  } else {
    j = 0;
  }

  if (j < 4) {
    /* Select all samples */

    for (j = 0; j < filter->used; j++)
      selected[j] = j;
  }

  /* And sort their indices by offset */
  tmp_sort_samples = filter->samples;
  qsort(selected, j, sizeof (int), compare_samples);

  /* Select samples closest to the median */
  if (j > 2) {
    from = j * (1.0 - filter->combine_ratio) / 2.0;
    from = CLAMP(1, from, (j - 1) / 2);
  } else {
    from = 0;
  }

  to = j - from;

  /* Mark unused samples and sort the rest by their time */

  o = filter->used - filter->index - 1;

  for (i = 0; i < from; i++)
    selected[i] = -1;
  for (; i < to; i++)
    selected[i] = (selected[i] + o) % filter->used;
  for (; i < filter->used; i++)
    selected[i] = -1;

  for (i = from; i < to; i++) {
    j = selected[i];
    selected[i] = -1;
    while (j != -1 && selected[j] != j) {
      k = selected[j];
      selected[j] = j;
      j = k;
    }
  }

  for (i = j = 0, k = -1; i < filter->used; i++) {
    if (selected[i] != -1)
      selected[j++] = (selected[i] + filter->used - o) % filter->used;
  }

  assert(j > 0 && j <= filter->max_samples);

  return j;
}

/* ================================================== */

static int
combine_selected_samples(SPF_Instance filter, int n, NTP_Sample *result)
{
  double mean_peer_dispersion, mean_root_dispersion, mean_peer_delay, mean_root_delay;
  double mean_x, mean_y, disp, var, prev_avg_var;
  NTP_Sample *sample, *last_sample;
  int i, dof;

  last_sample = &filter->samples[filter->selected[n - 1]];

  /* Prepare data */
  for (i = 0; i < n; i++) {
    sample = &filter->samples[filter->selected[i]];

    filter->x_data[i] = UTI_DiffTimespecsToDouble(&sample->time, &last_sample->time);
    filter->y_data[i] = sample->offset;
    filter->w_data[i] = sample->peer_dispersion;
  }

  /* Calculate mean offset and interval since the last sample */
  for (i = 0, mean_x = mean_y = 0.0; i < n; i++) {
    mean_x += filter->x_data[i];
    mean_y += filter->y_data[i];
  }
  mean_x /= n;
  mean_y /= n;

  if (n >= 4) {
    double b0, b1, s2, sb0, sb1;

    /* Set y axis to the mean sample time */
    for (i = 0; i < n; i++)
      filter->x_data[i] -= mean_x;

    /* Make a linear fit and use the estimated standard deviation of the
       intercept as dispersion */
    RGR_WeightedRegression(filter->x_data, filter->y_data, filter->w_data, n,
                           &b0, &b1, &s2, &sb0, &sb1);
    var = s2;
    disp = sb0;
    dof = n - 2;
  } else if (n >= 2) {
    for (i = 0, disp = 0.0; i < n; i++)
      disp += (filter->y_data[i] - mean_y) * (filter->y_data[i] - mean_y);
    var = disp / (n - 1);
    disp = sqrt(var);
    dof = n - 1;
  } else {
    var = filter->avg_var;
    disp = sqrt(var);
    dof = 1;
  }

  /* Avoid working with zero dispersion */
  if (var < 1e-20) {
    var = 1e-20;
    disp = sqrt(var);
  }

  /* Drop the sample if the variance is larger than the maximum */
  if (filter->max_var > 0.0 && var > filter->max_var) {
    DEBUG_LOG("filter dispersion too large disp=%.9f max=%.9f",
              sqrt(var), sqrt(filter->max_var));
    return 0;
  }

  prev_avg_var = filter->avg_var;

  /* Update the exponential moving average of the variance */
  if (filter->avg_var_n > 50) {
    filter->avg_var += dof / (dof + 50.0) * (var - filter->avg_var);
  } else {
    filter->avg_var = (filter->avg_var * filter->avg_var_n + var * dof) /
      (dof + filter->avg_var_n);
    if (filter->avg_var_n == 0)
      prev_avg_var = filter->avg_var;
    filter->avg_var_n += dof;
  }

  /* Use the long-term average of variance instead of the estimated value
     unless it is significantly smaller in order to reduce the noise in
     sourcestats weights */
  if (var * dof / RGR_GetChi2Coef(dof) < prev_avg_var)
    disp = sqrt(filter->avg_var) * disp / sqrt(var);

  mean_peer_dispersion = mean_root_dispersion = mean_peer_delay = mean_root_delay = 0.0;

  for (i = 0; i < n; i++) {
    sample = &filter->samples[filter->selected[i]];

    mean_peer_dispersion += sample->peer_dispersion;
    mean_root_dispersion += sample->root_dispersion;
    mean_peer_delay += sample->peer_delay;
    mean_root_delay += sample->root_delay;
  }

  mean_peer_dispersion /= n;
  mean_root_dispersion /= n;
  mean_peer_delay /= n;
  mean_root_delay /= n;

  UTI_AddDoubleToTimespec(&last_sample->time, mean_x, &result->time);
  result->offset = mean_y;
  result->peer_dispersion = MAX(disp, mean_peer_dispersion);
  result->root_dispersion = MAX(disp, mean_root_dispersion);
  result->peer_delay = mean_peer_delay;
  result->root_delay = mean_root_delay;
  result->stratum = last_sample->stratum;
  result->leap = last_sample->leap;

  return 1;
}

/* ================================================== */

int
SPF_GetFilteredSample(SPF_Instance filter, NTP_Sample *sample)
{
  int n;

  n = select_samples(filter);

  if (n < 1)
    return 0;

  if (!combine_selected_samples(filter, n, sample))
    return 0;

  SPF_DropSamples(filter);

  return 1;
}

/* ================================================== */

void
SPF_SlewSamples(SPF_Instance filter, struct timespec *when, double dfreq, double doffset)
{
  int i, first, last;
  double delta_time;

  if (filter->last < 0)
    return;

  /* Always slew the last sample as it may be returned even if no new
     samples were accumulated */
  if (filter->used > 0) {
    first = 0;
    last = filter->used - 1;
  } else {
    first = last = filter->last;
  }

  for (i = first; i <= last; i++) {
    UTI_AdjustTimespec(&filter->samples[i].time, when, &filter->samples[i].time,
                       &delta_time, dfreq, doffset);
    filter->samples[i].offset -= delta_time;
  }
}

/* ================================================== */

void
SPF_AddDispersion(SPF_Instance filter, double dispersion)
{
  int i;

  for (i = 0; i < filter->used; i++) {
    filter->samples[i].peer_dispersion += dispersion;
    filter->samples[i].root_dispersion += dispersion;
  }
}
