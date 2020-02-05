/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2011-2014, 2016-2018
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

  This file contains the routines that do the statistical
  analysis on the samples obtained from the sources,
  to determined frequencies and error bounds. */

#include "config.h"

#include "sysincl.h"

#include "sourcestats.h"
#include "memory.h"
#include "regress.h"
#include "util.h"
#include "conf.h"
#include "logging.h"
#include "local.h"

/* ================================================== */
/* Define the maxumum number of samples that we want
   to store per source */
#define MAX_SAMPLES 64

/* This is the assumed worst case bound on an unknown frequency,
   2000ppm, which would be pretty bad */
#define WORST_CASE_FREQ_BOUND (2000.0/1.0e6)

/* The minimum and maximum assumed skew */
#define MIN_SKEW 1.0e-12
#define MAX_SKEW 1.0e+02

/* The minimum standard deviation */
#define MIN_STDDEV 1.0e-9

/* The asymmetry of network jitter when all jitter is in one direction */
#define MAX_ASYMMETRY 0.5

/* The minimum estimated asymmetry that can activate the offset correction */
#define MIN_ASYMMETRY 0.45

/* The minimum number of consecutive asymmetries with the same sign needed
   to activate the offset correction */
#define MIN_ASYMMETRY_RUN 10

/* The maximum value of the counter */
#define MAX_ASYMMETRY_RUN 1000

/* ================================================== */

static LOG_FileID logfileid;

/* ================================================== */
/* This data structure is used to hold the history of data from the
   source */

struct SST_Stats_Record {

  /* Reference ID and IP address of source, used for logging to statistics log */
  uint32_t refid;
  IPAddr *ip_addr;

  /* User defined minimum and maximum number of samples */
  int min_samples;
  int max_samples;

  /* User defined minimum delay */
  double fixed_min_delay;

  /* User defined asymmetry of network jitter */
  double fixed_asymmetry;

  /* Number of samples currently stored.  The samples are stored in circular
     buffer. */
  int n_samples;

  /* Number of extra samples stored in sample_times, offsets and peer_delays
     arrays that are used to extend the runs test */
  int runs_samples;

  /* The index of the newest sample */
  int last_sample;

  /* Flag indicating whether last regression was successful */
  int regression_ok;

  /* The best individual sample that we are holding, in terms of the minimum
     root distance at the present time */
  int best_single_sample;

  /* The index of the sample with minimum delay in peer_delays */
  int min_delay_sample;

  /* This is the estimated offset (+ve => local fast) at a particular time */
  double estimated_offset;
  double estimated_offset_sd;
  struct timespec offset_time;

  /* Number of runs of the same sign amongst the residuals */
  int nruns;

  /* Number of consecutive estimated asymmetries with the same sign.
     The sign of the number encodes the sign of the asymmetry. */
  int asymmetry_run;

  /* This is the latest estimated asymmetry of network jitter */
  double asymmetry;

  /* This value contains the estimated frequency.  This is the number
     of seconds that the local clock gains relative to the reference
     source per unit local time.  (Positive => local clock fast,
     negative => local clock slow) */
  double estimated_frequency;
  double estimated_frequency_sd;

  /* This is the assumed worst case bounds on the estimated frequency.
     We assume that the true frequency lies within +/- half this much
     about estimated_frequency */
  double skew;

  /* This is the estimated standard deviation of the data points */
  double std_dev;

  /* This array contains the sample epochs, in terms of the local
     clock. */
  struct timespec sample_times[MAX_SAMPLES * REGRESS_RUNS_RATIO];

  /* This is an array of offsets, in seconds, corresponding to the
     sample times.  In this module, we use the convention that
     positive means the local clock is FAST of the source and negative
     means it is SLOW.  This is contrary to the convention in the NTP
     stuff. */
  double offsets[MAX_SAMPLES * REGRESS_RUNS_RATIO];

  /* This is an array of the offsets as originally measured.  Local
     clock fast of real time is indicated by positive values.  This
     array is not slewed to adjust the readings when we apply
     adjustments to the local clock, as is done for the array
     'offset'. */
  double orig_offsets[MAX_SAMPLES];

  /* This is an array of peer delays, in seconds, being the roundtrip
     measurement delay to the peer */
  double peer_delays[MAX_SAMPLES * REGRESS_RUNS_RATIO];

  /* This is an array of peer dispersions, being the skew and local
     precision dispersion terms from sampling the peer */
  double peer_dispersions[MAX_SAMPLES];

  /* This array contains the root delays of each sample, in seconds */
  double root_delays[MAX_SAMPLES];

  /* This array contains the root dispersions of each sample at the
     time of the measurements */
  double root_dispersions[MAX_SAMPLES];

  /* The stratum from the last accumulated sample */
  int stratum;

  /* The leap status from the last accumulated sample */
  NTP_Leap leap;
};

/* ================================================== */

static void find_min_delay_sample(SST_Stats inst);
static int get_buf_index(SST_Stats inst, int i);

/* ================================================== */

void
SST_Initialise(void)
{
  logfileid = CNF_GetLogStatistics() ? LOG_FileOpen("statistics",
      "   Date (UTC) Time     IP Address    Std dev'n Est offset  Offset sd  Diff freq   Est skew  Stress  Ns  Bs  Nr  Asym")
    : -1;
}

/* ================================================== */

void
SST_Finalise(void)
{
}

/* ================================================== */
/* This function creates a new instance of the statistics handler */

SST_Stats
SST_CreateInstance(uint32_t refid, IPAddr *addr, int min_samples, int max_samples,
                   double min_delay, double asymmetry)
{
  SST_Stats inst;
  inst = MallocNew(struct SST_Stats_Record);

  inst->min_samples = min_samples;
  inst->max_samples = max_samples;
  inst->fixed_min_delay = min_delay;
  inst->fixed_asymmetry = asymmetry;

  SST_SetRefid(inst, refid, addr);
  SST_ResetInstance(inst);

  return inst;
}

/* ================================================== */
/* This function deletes an instance of the statistics handler. */

void
SST_DeleteInstance(SST_Stats inst)
{
  Free(inst);
}

/* ================================================== */

void
SST_ResetInstance(SST_Stats inst)
{
  inst->n_samples = 0;
  inst->runs_samples = 0;
  inst->last_sample = 0;
  inst->regression_ok = 0;
  inst->best_single_sample = 0;
  inst->min_delay_sample = 0;
  inst->estimated_frequency = 0;
  inst->estimated_frequency_sd = WORST_CASE_FREQ_BOUND;
  inst->skew = WORST_CASE_FREQ_BOUND;
  inst->estimated_offset = 0.0;
  inst->estimated_offset_sd = 86400.0; /* Assume it's at least within a day! */
  UTI_ZeroTimespec(&inst->offset_time);
  inst->std_dev = 4.0;
  inst->nruns = 0;
  inst->asymmetry_run = 0;
  inst->asymmetry = 0.0;
  inst->leap = LEAP_Unsynchronised;
}

/* ================================================== */

void
SST_SetRefid(SST_Stats inst, uint32_t refid, IPAddr *addr)
{
  inst->refid = refid;
  inst->ip_addr = addr;
}

/* ================================================== */
/* This function is called to prune the register down when it is full.
   For now, just discard the oldest sample.  */

static void
prune_register(SST_Stats inst, int new_oldest)
{
  if (!new_oldest)
    return;

  assert(inst->n_samples >= new_oldest);
  inst->n_samples -= new_oldest;
  inst->runs_samples += new_oldest;
  if (inst->runs_samples > inst->n_samples * (REGRESS_RUNS_RATIO - 1))
    inst->runs_samples = inst->n_samples * (REGRESS_RUNS_RATIO - 1);
  
  assert(inst->n_samples + inst->runs_samples <= MAX_SAMPLES * REGRESS_RUNS_RATIO);

  find_min_delay_sample(inst);
}

/* ================================================== */

void
SST_AccumulateSample(SST_Stats inst, NTP_Sample *sample)
{
  int n, m;

  /* Make room for the new sample */
  if (inst->n_samples > 0 &&
      (inst->n_samples == MAX_SAMPLES || inst->n_samples == inst->max_samples)) {
    prune_register(inst, 1);
  }

  /* Make sure it's newer than the last sample */
  if (inst->n_samples &&
      UTI_CompareTimespecs(&inst->sample_times[inst->last_sample], &sample->time) >= 0) {
    LOG(LOGS_WARN, "Out of order sample detected, discarding history for %s",
        inst->ip_addr ? UTI_IPToString(inst->ip_addr) : UTI_RefidToString(inst->refid));
    SST_ResetInstance(inst);
  }

  n = inst->last_sample = (inst->last_sample + 1) %
    (MAX_SAMPLES * REGRESS_RUNS_RATIO);
  m = n % MAX_SAMPLES;

  /* WE HAVE TO NEGATE OFFSET IN THIS CALL, IT IS HERE THAT THE SENSE OF OFFSET
     IS FLIPPED */
  inst->sample_times[n] = sample->time;
  inst->offsets[n] = -sample->offset;
  inst->orig_offsets[m] = -sample->offset;
  inst->peer_delays[n] = sample->peer_delay;
  inst->peer_dispersions[m] = sample->peer_dispersion;
  inst->root_delays[m] = sample->root_delay;
  inst->root_dispersions[m] = sample->root_dispersion;
  inst->stratum = sample->stratum;
  inst->leap = sample->leap;
 
  if (inst->peer_delays[n] < inst->fixed_min_delay)
    inst->peer_delays[n] = 2.0 * inst->fixed_min_delay - inst->peer_delays[n];

  if (!inst->n_samples || inst->peer_delays[n] < inst->peer_delays[inst->min_delay_sample])
    inst->min_delay_sample = n;

  ++inst->n_samples;
}

/* ================================================== */
/* Return index of the i-th sample in the sample_times and offset buffers,
   i can be negative down to -runs_samples */

static int
get_runsbuf_index(SST_Stats inst, int i)
{
  return (unsigned int)(inst->last_sample + 2 * MAX_SAMPLES * REGRESS_RUNS_RATIO -
      inst->n_samples + i + 1) % (MAX_SAMPLES * REGRESS_RUNS_RATIO);
}

/* ================================================== */
/* Return index of the i-th sample in the other buffers */

static int
get_buf_index(SST_Stats inst, int i)
{
  return (unsigned int)(inst->last_sample + MAX_SAMPLES * REGRESS_RUNS_RATIO -
      inst->n_samples + i + 1) % MAX_SAMPLES;
}

/* ================================================== */
/* This function is used by both the regression routines to find the
   time interval between each historical sample and the most recent
   one */

static void
convert_to_intervals(SST_Stats inst, double *times_back)
{
  struct timespec *ts;
  int i;

  ts = &inst->sample_times[inst->last_sample];
  for (i = -inst->runs_samples; i < inst->n_samples; i++) {
    /* The entries in times_back[] should end up negative */
    times_back[i] = UTI_DiffTimespecsToDouble(&inst->sample_times[get_runsbuf_index(inst, i)], ts);
  }
}

/* ================================================== */

static void
find_best_sample_index(SST_Stats inst, double *times_back)
{
  /* With the value of skew that has been computed, see which of the
     samples offers the tightest bound on root distance */

  double root_distance, best_root_distance;
  double elapsed;
  int i, j, best_index;

  if (!inst->n_samples)
    return;

  best_index = -1;
  best_root_distance = DBL_MAX;

  for (i = 0; i < inst->n_samples; i++) {
    j = get_buf_index(inst, i);

    elapsed = -times_back[i];
    assert(elapsed >= 0.0);

    root_distance = inst->root_dispersions[j] + elapsed * inst->skew + 0.5 * inst->root_delays[j];
    if (root_distance < best_root_distance) {
      best_root_distance = root_distance;
      best_index = i;
    }
  }

  assert(best_index >= 0);
  inst->best_single_sample = best_index;
}

/* ================================================== */

static void
find_min_delay_sample(SST_Stats inst)
{
  int i, index;

  inst->min_delay_sample = get_runsbuf_index(inst, -inst->runs_samples);

  for (i = -inst->runs_samples + 1; i < inst->n_samples; i++) {
    index = get_runsbuf_index(inst, i);
    if (inst->peer_delays[index] < inst->peer_delays[inst->min_delay_sample])
      inst->min_delay_sample = index;
  }
}

/* ================================================== */
/* This function estimates asymmetry of network jitter on the path to the
   source as a slope of offset against network delay in multiple linear
   regression.  If the asymmetry is significant and its sign doesn't change
   frequently, the measured offsets (which are used later to estimate the
   offset and frequency of the clock) are corrected to correspond to the
   minimum network delay.  This can significantly improve the accuracy and
   stability of the estimated offset and frequency. */

static int
estimate_asymmetry(double *times_back, double *offsets, double *delays, int n,
                   double *asymmetry, int *asymmetry_run)
{
  double a;

  /* Reset the counter when the regression fails or the sign changes */
  if (!RGR_MultipleRegress(times_back, delays, offsets, n, &a) ||
      a * *asymmetry_run < 0.0) {
    *asymmetry = 0;
    *asymmetry_run = 0.0;
    return 0;
  }

  if (a <= -MIN_ASYMMETRY && *asymmetry_run > -MAX_ASYMMETRY_RUN)
    (*asymmetry_run)--;
  else if (a >= MIN_ASYMMETRY && *asymmetry_run < MAX_ASYMMETRY_RUN)
    (*asymmetry_run)++;

  if (abs(*asymmetry_run) < MIN_ASYMMETRY_RUN)
    return 0;

  *asymmetry = CLAMP(-MAX_ASYMMETRY, a, MAX_ASYMMETRY);

  return 1;
}

/* ================================================== */

static void
correct_asymmetry(SST_Stats inst, double *times_back, double *offsets)
{
  double min_delay, delays[MAX_SAMPLES * REGRESS_RUNS_RATIO];
  int i, n;

  /* Check if the asymmetry was not specified to be zero */
  if (inst->fixed_asymmetry == 0.0)
    return;

  min_delay = SST_MinRoundTripDelay(inst);
  n = inst->runs_samples + inst->n_samples;

  for (i = 0; i < n; i++)
    delays[i] = inst->peer_delays[get_runsbuf_index(inst, i - inst->runs_samples)] -
                min_delay;

  if (fabs(inst->fixed_asymmetry) <= MAX_ASYMMETRY) {
    inst->asymmetry = inst->fixed_asymmetry;
  } else {
    if (!estimate_asymmetry(times_back, offsets, delays, n,
                            &inst->asymmetry, &inst->asymmetry_run))
      return;
  }

  /* Correct the offsets */
  for (i = 0; i < n; i++)
    offsets[i] -= inst->asymmetry * delays[i];
}

/* ================================================== */

/* This defines the assumed ratio between the standard deviation of
   the samples and the peer distance as measured from the round trip
   time.  E.g. a value of 4 means that we think the standard deviation
   is four times the fluctuation  of the peer distance */

#define SD_TO_DIST_RATIO 0.7

/* ================================================== */
/* This function runs the linear regression operation on the data.  It
   finds the set of most recent samples that give the tightest
   confidence interval for the frequency, and truncates the register
   down to that number of samples */

void
SST_DoNewRegression(SST_Stats inst)
{
  double times_back[MAX_SAMPLES * REGRESS_RUNS_RATIO];
  double offsets[MAX_SAMPLES * REGRESS_RUNS_RATIO];
  double peer_distances[MAX_SAMPLES];
  double weights[MAX_SAMPLES];

  int degrees_of_freedom;
  int best_start, times_back_start;
  double est_intercept, est_slope, est_var, est_intercept_sd, est_slope_sd;
  int i, j, nruns;
  double min_distance, median_distance;
  double sd_weight, sd;
  double old_skew, old_freq, stress;
  double precision;

  convert_to_intervals(inst, times_back + inst->runs_samples);

  if (inst->n_samples > 0) {
    for (i = -inst->runs_samples; i < inst->n_samples; i++) {
      offsets[i + inst->runs_samples] = inst->offsets[get_runsbuf_index(inst, i)];
    }
  
    for (i = 0, min_distance = DBL_MAX; i < inst->n_samples; i++) {
      j = get_buf_index(inst, i);
      peer_distances[i] = 0.5 * inst->peer_delays[get_runsbuf_index(inst, i)] +
                          inst->peer_dispersions[j];
      if (peer_distances[i] < min_distance) {
        min_distance = peer_distances[i];
      }
    }

    /* And now, work out the weight vector */

    precision = LCL_GetSysPrecisionAsQuantum();
    median_distance = RGR_FindMedian(peer_distances, inst->n_samples);

    sd = (median_distance - min_distance) / SD_TO_DIST_RATIO;
    sd = CLAMP(precision, sd, min_distance);
    min_distance += precision;

    for (i=0; i<inst->n_samples; i++) {
      sd_weight = 1.0;
      if (peer_distances[i] > min_distance)
        sd_weight += (peer_distances[i] - min_distance) / sd;
      weights[i] = SQUARE(sd_weight);
    }
  }

  correct_asymmetry(inst, times_back, offsets);

  inst->regression_ok = RGR_FindBestRegression(times_back + inst->runs_samples,
                                         offsets + inst->runs_samples, weights,
                                         inst->n_samples, inst->runs_samples,
                                         inst->min_samples,
                                         &est_intercept, &est_slope, &est_var,
                                         &est_intercept_sd, &est_slope_sd,
                                         &best_start, &nruns, &degrees_of_freedom);

  if (inst->regression_ok) {

    old_skew = inst->skew;
    old_freq = inst->estimated_frequency;
  
    inst->estimated_frequency = est_slope;
    inst->estimated_frequency_sd = CLAMP(MIN_SKEW, est_slope_sd, MAX_SKEW);
    inst->skew = est_slope_sd * RGR_GetTCoef(degrees_of_freedom);
    inst->estimated_offset = est_intercept;
    inst->offset_time = inst->sample_times[inst->last_sample];
    inst->estimated_offset_sd = est_intercept_sd;
    inst->std_dev = MAX(MIN_STDDEV, sqrt(est_var));
    inst->nruns = nruns;

    inst->skew = CLAMP(MIN_SKEW, inst->skew, MAX_SKEW);
    stress = fabs(old_freq - inst->estimated_frequency) / old_skew;

    DEBUG_LOG("off=%e freq=%e skew=%e n=%d bs=%d runs=%d asym=%f arun=%d",
              inst->estimated_offset, inst->estimated_frequency, inst->skew,
              inst->n_samples, best_start, inst->nruns,
              inst->asymmetry, inst->asymmetry_run);

    if (logfileid != -1) {
      LOG_FileWrite(logfileid, "%s %-15s %10.3e %10.3e %10.3e %10.3e %10.3e %7.1e %3d %3d %3d %5.2f",
              UTI_TimeToLogForm(inst->offset_time.tv_sec),
              inst->ip_addr ? UTI_IPToString(inst->ip_addr) : UTI_RefidToString(inst->refid),
              inst->std_dev,
              inst->estimated_offset, inst->estimated_offset_sd,
              inst->estimated_frequency, inst->skew, stress,
              inst->n_samples, best_start, inst->nruns,
              inst->asymmetry);
    }

    times_back_start = inst->runs_samples + best_start;
    prune_register(inst, best_start);
  } else {
    inst->estimated_frequency = 0.0;
    inst->estimated_frequency_sd = WORST_CASE_FREQ_BOUND;
    inst->skew = WORST_CASE_FREQ_BOUND;
    times_back_start = 0;
  }

  find_best_sample_index(inst, times_back + times_back_start);

}

/* ================================================== */
/* Return the assumed worst case range of values that this source's
   frequency lies within.  Frequency is defined as the amount of time
   the local clock gains relative to the source per unit local clock
   time. */
void
SST_GetFrequencyRange(SST_Stats inst,
                      double *lo, double *hi)
{
  double freq, skew;
  freq = inst->estimated_frequency;
  skew = inst->skew;
  *lo = freq - skew;
  *hi = freq + skew;

  /* This function is currently used only to determine the values of delta
     and epsilon in the ntp_core module. Limit the skew to a reasonable maximum
     to avoid failing the dispersion test too easily. */
  if (skew > WORST_CASE_FREQ_BOUND) {
    *lo = -WORST_CASE_FREQ_BOUND;
    *hi = WORST_CASE_FREQ_BOUND;
  }
}

/* ================================================== */

void
SST_GetSelectionData(SST_Stats inst, struct timespec *now,
                     int *stratum, NTP_Leap *leap,
                     double *offset_lo_limit,
                     double *offset_hi_limit,
                     double *root_distance,
                     double *std_dev,
                     double *first_sample_ago,
                     double *last_sample_ago,
                     int *select_ok)
{
  double offset, sample_elapsed;
  int i, j;
  
  if (!inst->n_samples) {
    *select_ok = 0;
    return;
  }

  i = get_runsbuf_index(inst, inst->best_single_sample);
  j = get_buf_index(inst, inst->best_single_sample);

  *stratum = inst->stratum;
  *leap = inst->leap;
  *std_dev = inst->std_dev;

  sample_elapsed = fabs(UTI_DiffTimespecsToDouble(now, &inst->sample_times[i]));
  offset = inst->offsets[i] + sample_elapsed * inst->estimated_frequency;
  *root_distance = 0.5 * inst->root_delays[j] +
    inst->root_dispersions[j] + sample_elapsed * inst->skew;

  *offset_lo_limit = offset - *root_distance;
  *offset_hi_limit = offset + *root_distance;

#if 0
  double average_offset, elapsed;
  int average_ok;
  /* average_ok ignored for now */
  elapsed = UTI_DiffTimespecsToDouble(now, &inst->offset_time);
  average_offset = inst->estimated_offset + inst->estimated_frequency * elapsed;
  if (fabs(average_offset - offset) <=
      inst->peer_dispersions[j] + 0.5 * inst->peer_delays[i]) {
    average_ok = 1;
  } else {
    average_ok = 0;
  }
#endif

  i = get_runsbuf_index(inst, 0);
  *first_sample_ago = UTI_DiffTimespecsToDouble(now, &inst->sample_times[i]);
  i = get_runsbuf_index(inst, inst->n_samples - 1);
  *last_sample_ago = UTI_DiffTimespecsToDouble(now, &inst->sample_times[i]);

  *select_ok = inst->regression_ok;

  DEBUG_LOG("n=%d off=%f dist=%f sd=%f first_ago=%f last_ago=%f selok=%d",
            inst->n_samples, offset, *root_distance, *std_dev,
            *first_sample_ago, *last_sample_ago, *select_ok);
}

/* ================================================== */

void
SST_GetTrackingData(SST_Stats inst, struct timespec *ref_time,
                    double *average_offset, double *offset_sd,
                    double *frequency, double *frequency_sd, double *skew,
                    double *root_delay, double *root_dispersion)
{
  int i, j;
  double elapsed_sample;

  assert(inst->n_samples > 0);

  i = get_runsbuf_index(inst, inst->best_single_sample);
  j = get_buf_index(inst, inst->best_single_sample);

  *ref_time = inst->offset_time;
  *average_offset = inst->estimated_offset;
  *offset_sd = inst->estimated_offset_sd;
  *frequency = inst->estimated_frequency;
  *frequency_sd = inst->estimated_frequency_sd;
  *skew = inst->skew;
  *root_delay = inst->root_delays[j];

  elapsed_sample = UTI_DiffTimespecsToDouble(&inst->offset_time, &inst->sample_times[i]);
  *root_dispersion = inst->root_dispersions[j] + inst->skew * elapsed_sample + *offset_sd;

  DEBUG_LOG("n=%d off=%f offsd=%f freq=%e freqsd=%e skew=%e delay=%f disp=%f",
            inst->n_samples, *average_offset, *offset_sd,
            *frequency, *frequency_sd, *skew, *root_delay, *root_dispersion);
}

/* ================================================== */

void
SST_SlewSamples(SST_Stats inst, struct timespec *when, double dfreq, double doffset)
{
  int m, i;
  double delta_time;
  struct timespec *sample, prev;
  double prev_offset, prev_freq;

  if (!inst->n_samples)
    return;

  for (m = -inst->runs_samples; m < inst->n_samples; m++) {
    i = get_runsbuf_index(inst, m);
    sample = &inst->sample_times[i];
    prev = *sample;
    UTI_AdjustTimespec(sample, when, sample, &delta_time, dfreq, doffset);
    inst->offsets[i] += delta_time;
  }

  /* Update the regression estimates */
  prev = inst->offset_time;
  prev_offset = inst->estimated_offset;
  prev_freq = inst->estimated_frequency;
  UTI_AdjustTimespec(&inst->offset_time, when, &inst->offset_time,
      &delta_time, dfreq, doffset);
  inst->estimated_offset += delta_time;
  inst->estimated_frequency = (inst->estimated_frequency - dfreq) / (1.0 - dfreq);

  DEBUG_LOG("n=%d m=%d old_off_time=%s new=%s old_off=%f new_off=%f old_freq=%.3f new_freq=%.3f",
            inst->n_samples, inst->runs_samples,
            UTI_TimespecToString(&prev), UTI_TimespecToString(&inst->offset_time),
            prev_offset, inst->estimated_offset,
            1.0e6 * prev_freq, 1.0e6 * inst->estimated_frequency);
}

/* ================================================== */

void 
SST_AddDispersion(SST_Stats inst, double dispersion)
{
  int m, i;

  for (m = 0; m < inst->n_samples; m++) {
    i = get_buf_index(inst, m);
    inst->root_dispersions[i] += dispersion;
    inst->peer_dispersions[i] += dispersion;
  }
}

/* ================================================== */

double
SST_PredictOffset(SST_Stats inst, struct timespec *when)
{
  double elapsed;
  
  if (inst->n_samples < 3) {
    /* We don't have any useful statistics, and presumably the poll
       interval is minimal.  We can't do any useful prediction other
       than use the latest sample or zero if we don't have any samples */
    if (inst->n_samples > 0) {
      return inst->offsets[inst->last_sample];
    } else {
      return 0.0;
    }
  } else {
    elapsed = UTI_DiffTimespecsToDouble(when, &inst->offset_time);
    return inst->estimated_offset + elapsed * inst->estimated_frequency;
  }

}

/* ================================================== */

double
SST_MinRoundTripDelay(SST_Stats inst)
{
  if (inst->fixed_min_delay > 0.0)
    return inst->fixed_min_delay;

  if (!inst->n_samples)
    return DBL_MAX;

  return inst->peer_delays[inst->min_delay_sample];
}

/* ================================================== */

int
SST_GetDelayTestData(SST_Stats inst, struct timespec *sample_time,
                     double *last_sample_ago, double *predicted_offset,
                     double *min_delay, double *skew, double *std_dev)
{
  if (inst->n_samples < 6)
    return 0;

  *last_sample_ago = UTI_DiffTimespecsToDouble(sample_time, &inst->offset_time);
  *predicted_offset = inst->estimated_offset +
                      *last_sample_ago * inst->estimated_frequency;
  *min_delay = SST_MinRoundTripDelay(inst);
  *skew = inst->skew;
  *std_dev = inst->std_dev;

  return 1;
}

/* ================================================== */
/* This is used to save the register to a file, so that we can reload
   it after restarting the daemon */

void
SST_SaveToFile(SST_Stats inst, FILE *out)
{
  int m, i, j;

  fprintf(out, "%d\n", inst->n_samples);

  for(m = 0; m < inst->n_samples; m++) {
    i = get_runsbuf_index(inst, m);
    j = get_buf_index(inst, m);

    fprintf(out,
#ifdef HAVE_LONG_TIME_T
            "%08"PRIx64" %08lx %.6e %.6e %.6e %.6e %.6e %.6e %.6e %d\n",
            (uint64_t)inst->sample_times[i].tv_sec,
#else
            "%08lx %08lx %.6e %.6e %.6e %.6e %.6e %.6e %.6e %d\n",
            (unsigned long)inst->sample_times[i].tv_sec,
#endif
            (unsigned long)inst->sample_times[i].tv_nsec / 1000,
            inst->offsets[i],
            inst->orig_offsets[j],
            inst->peer_delays[i],
            inst->peer_dispersions[j],
            inst->root_delays[j],
            inst->root_dispersions[j],
            1.0, /* used to be inst->weights[i] */
            inst->stratum /* used to be an array */);

  }

  fprintf(out, "%d\n", inst->asymmetry_run);
}

/* ================================================== */
/* This is used to reload samples from a file */

int
SST_LoadFromFile(SST_Stats inst, FILE *in)
{
#ifdef HAVE_LONG_TIME_T
  uint64_t sec;
#else
  unsigned long sec;
#endif
  unsigned long usec;
  int i;
  char line[1024];
  double weight;

  SST_ResetInstance(inst);

  if (fgets(line, sizeof(line), in) &&
      sscanf(line, "%d", &inst->n_samples) == 1 &&
      inst->n_samples >= 0 && inst->n_samples <= MAX_SAMPLES) {

    for (i=0; i<inst->n_samples; i++) {
      if (!fgets(line, sizeof(line), in) ||
          (sscanf(line,
#ifdef HAVE_LONG_TIME_T
                  "%"SCNx64"%lx%lf%lf%lf%lf%lf%lf%lf%d\n",
#else
                  "%lx%lx%lf%lf%lf%lf%lf%lf%lf%d\n",
#endif
                  &(sec), &(usec),
                  &(inst->offsets[i]),
                  &(inst->orig_offsets[i]),
                  &(inst->peer_delays[i]),
                  &(inst->peer_dispersions[i]),
                  &(inst->root_delays[i]),
                  &(inst->root_dispersions[i]),
                  &weight, /* not used anymore */
                  &inst->stratum) != 10)) {

        /* This is the branch taken if the read FAILED */

        inst->n_samples = 0; /* Load abandoned if any sign of corruption */
        return 0;
      } else {

        /* This is the branch taken if the read is SUCCESSFUL */
        inst->sample_times[i].tv_sec = sec;
        inst->sample_times[i].tv_nsec = 1000 * usec;
        UTI_NormaliseTimespec(&inst->sample_times[i]);
      }
    }

    /* This field was not saved in older versions */
    if (!fgets(line, sizeof(line), in) || sscanf(line, "%d\n", &inst->asymmetry_run) != 1)
      inst->asymmetry_run = 0;
  } else {
    inst->n_samples = 0; /* Load abandoned if any sign of corruption */
    return 0;
  }

  if (!inst->n_samples)
    return 1;

  inst->last_sample = inst->n_samples - 1;

  find_min_delay_sample(inst);
  SST_DoNewRegression(inst);

  return 1;
}

/* ================================================== */

void
SST_DoSourceReport(SST_Stats inst, RPT_SourceReport *report, struct timespec *now)
{
  int i, j;
  struct timespec last_sample_time;

  if (inst->n_samples > 0) {
    i = get_runsbuf_index(inst, inst->n_samples - 1);
    j = get_buf_index(inst, inst->n_samples - 1);
    report->orig_latest_meas = inst->orig_offsets[j];
    report->latest_meas = inst->offsets[i];
    report->latest_meas_err = 0.5*inst->root_delays[j] + inst->root_dispersions[j];
    report->stratum = inst->stratum;

    /* Align the sample time to reduce the leak of the receive timestamp */
    last_sample_time = inst->sample_times[i];
    last_sample_time.tv_nsec = 0;
    report->latest_meas_ago = UTI_DiffTimespecsToDouble(now, &last_sample_time);
  } else {
    report->latest_meas_ago = (uint32_t)-1;
    report->orig_latest_meas = 0;
    report->latest_meas = 0;
    report->latest_meas_err = 0;
    report->stratum = 0;
  }
}

/* ================================================== */

int
SST_Samples(SST_Stats inst)
{
  return inst->n_samples;
}

/* ================================================== */

void
SST_DoSourcestatsReport(SST_Stats inst, RPT_SourcestatsReport *report, struct timespec *now)
{
  double dspan;
  double elapsed, sample_elapsed;
  int li, lj, bi, bj;

  report->n_samples = inst->n_samples;
  report->n_runs = inst->nruns;

  if (inst->n_samples > 1) {
    li = get_runsbuf_index(inst, inst->n_samples - 1);
    lj = get_buf_index(inst, inst->n_samples - 1);
    dspan = UTI_DiffTimespecsToDouble(&inst->sample_times[li],
        &inst->sample_times[get_runsbuf_index(inst, 0)]);
    report->span_seconds = (unsigned long) (dspan + 0.5);

    if (inst->n_samples > 3) {
      elapsed = UTI_DiffTimespecsToDouble(now, &inst->offset_time);
      bi = get_runsbuf_index(inst, inst->best_single_sample);
      bj = get_buf_index(inst, inst->best_single_sample);
      sample_elapsed = UTI_DiffTimespecsToDouble(now, &inst->sample_times[bi]);
      report->est_offset = inst->estimated_offset + elapsed * inst->estimated_frequency;
      report->est_offset_err = (inst->estimated_offset_sd +
                 sample_elapsed * inst->skew +
                 (0.5*inst->root_delays[bj] + inst->root_dispersions[bj]));
    } else {
      report->est_offset = inst->offsets[li];
      report->est_offset_err = 0.5*inst->root_delays[lj] + inst->root_dispersions[lj];
    }
  } else {
    report->span_seconds = 0;
    report->est_offset = 0;
    report->est_offset_err = 0;
  }

  report->resid_freq_ppm = 1.0e6 * inst->estimated_frequency;
  report->skew_ppm = 1.0e6 * inst->skew;
  report->sd = inst->std_dev;
}

/* ================================================== */

double
SST_GetJitterAsymmetry(SST_Stats inst)
{
  return inst->asymmetry;
}

/* ================================================== */
