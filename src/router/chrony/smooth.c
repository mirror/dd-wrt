/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2015
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

  Routines implementing time smoothing.

  */

#include "config.h"

#include "sysincl.h"

#include "conf.h"
#include "local.h"
#include "logging.h"
#include "reference.h"
#include "smooth.h"
#include "util.h"

/*
  Time smoothing determines an offset that needs to be applied to the cooked
  time to make it smooth for external observers.  Observed offset and frequency
  change slowly and there are no discontinuities.  This can be used on an NTP
  server to make it easier for the clients to track the time and keep their
  clocks close together even when large offset or frequency corrections are
  applied to the server's clock (e.g. after being offline for longer time).

  Accumulated offset and frequency are smoothed out in three stages.  In the
  first stage, the frequency is changed at a constant rate (wander) up to a
  maximum, in the second stage the frequency stays at the maximum for as long
  as needed and in the third stage the frequency is brought back to zero.

              |
    max_freq  +-------/--------\-------------
              |      /|        |\
        freq  |     / |        | \
              |    /  |        |  \
              |   /   |        |   \
           0  +--/----+--------+----\--------
              | /     |        |    |    time
              |/      |        |    |

        stage     1       2      3

  Integral of this function is the smoothed out offset.  It's a continuous
  piecewise polynomial with two quadratic parts and one linear.
*/

struct stage {
  double wander;
  double length;
};

#define NUM_STAGES 3

static struct stage stages[NUM_STAGES];

/* Enabled/disabled smoothing */
static int enabled;

/* Enabled/disabled mode where only leap seconds are smoothed out and normal
   offset/frequency changes are ignored */
static int leap_only_mode;

/* Maximum skew/max_wander ratio to start updating offset and frequency */
#define UNLOCK_SKEW_WANDER_RATIO 10000

static int locked;

/* Maximum wander and frequency offset */
static double max_wander;
static double max_freq;

/* Frequency offset, time offset and the time of the last smoothing update */
static double smooth_freq;
static double smooth_offset;
static struct timespec last_update;


static void
get_smoothing(struct timespec *now, double *poffset, double *pfreq,
              double *pwander)
{
  double elapsed, length, offset, freq, wander;
  int i;

  elapsed = UTI_DiffTimespecsToDouble(now, &last_update);

  offset = smooth_offset;
  freq = smooth_freq;
  wander = 0.0;

  for (i = 0; i < NUM_STAGES; i++) {
    if (elapsed <= 0.0)
      break;

    length = stages[i].length;
    if (length >= elapsed)
      length = elapsed;

    wander = stages[i].wander;
    offset -= length * (2.0 * freq + wander * length) / 2.0;
    freq += wander * length;
    elapsed -= length;
  }

  if (elapsed > 0.0) {
    wander = 0.0;
    offset -= elapsed * freq;
  }

  *poffset = offset;
  *pfreq = freq;
  if (pwander)
    *pwander = wander;
}

static void
update_stages(void)
{
  double s1, s2, s, l1, l2, l3, lc, f, f2, l1t[2], l3t[2], err[2];
  int i, dir;

  /* Prepare the three stages so that the integral of the frequency offset
     is equal to the offset that should be smoothed out */

  s1 = smooth_offset / max_wander;
  s2 = SQUARE(smooth_freq) / (2.0 * SQUARE(max_wander));
  
  /* Calculate the lengths of the 1st and 3rd stage assuming there is no
     frequency limit.  The direction of the 1st stage is selected so that
     the lengths will not be negative.  With extremely small offsets both
     directions may give a negative length due to numerical errors, so select
     the one which gives a smaller error. */

  for (i = 0, dir = -1; i <= 1; i++, dir += 2) {
    err[i] = 0.0;
    s = dir * s1 + s2;

    if (s < 0.0) {
      err[i] += -s;
      s = 0.0;
    }

    l3t[i] = sqrt(s);
    l1t[i] = l3t[i] - dir * smooth_freq / max_wander;

    if (l1t[i] < 0.0) {
      err[i] += l1t[i] * l1t[i];
      l1t[i] = 0.0;
    }
  }

  if (err[0] < err[1]) {
    l1 = l1t[0];
    l3 = l3t[0];
    dir = -1;
  } else {
    l1 = l1t[1];
    l3 = l3t[1];
    dir = 1;
  }

  l2 = 0.0;

  /* If the limit was reached, shorten 1st+3rd stages and set a 2nd stage */
  f = dir * smooth_freq + l1 * max_wander - max_freq;
  if (f > 0.0) {
    lc = f / max_wander;

    /* No 1st stage if the frequency is already above the maximum */
    if (lc > l1) {
      lc = l1;
      f2 = dir * smooth_freq;
    } else {
      f2 = max_freq;
    }

    l2 = lc * (2.0 + f / f2);
    l1 -= lc;
    l3 -= lc;
  }

  stages[0].wander = dir * max_wander;
  stages[0].length = l1;
  stages[1].wander = 0.0;
  stages[1].length = l2;
  stages[2].wander = -dir * max_wander;
  stages[2].length = l3;

  for (i = 0; i < NUM_STAGES; i++) {
    DEBUG_LOG("Smooth stage %d wander %e length %f",
              i + 1, stages[i].wander, stages[i].length);
  }
}

static void
update_smoothing(struct timespec *now, double offset, double freq)
{
  /* Don't accept offset/frequency until the clock has stabilized */
  if (locked) {
    if (REF_GetSkew() / max_wander < UNLOCK_SKEW_WANDER_RATIO || leap_only_mode)
      SMT_Activate(now);
    return;
  }

  get_smoothing(now, &smooth_offset, &smooth_freq, NULL);
  smooth_offset += offset;
  smooth_freq = (smooth_freq - freq) / (1.0 - freq);
  last_update = *now;

  update_stages();

  DEBUG_LOG("Smooth offset %e freq %e", smooth_offset, smooth_freq);
}

static void
handle_slew(struct timespec *raw, struct timespec *cooked, double dfreq,
            double doffset, LCL_ChangeType change_type, void *anything)
{
  double delta;

  if (change_type == LCL_ChangeAdjust) {
    if (leap_only_mode)
      update_smoothing(cooked, 0.0, 0.0);
    else
      update_smoothing(cooked, doffset, dfreq);
  }

  if (!UTI_IsZeroTimespec(&last_update))
    UTI_AdjustTimespec(&last_update, cooked, &last_update, &delta, dfreq, doffset);
}

void SMT_Initialise(void)
{
  CNF_GetSmooth(&max_freq, &max_wander, &leap_only_mode);
  if (max_freq <= 0.0 || max_wander <= 0.0) {
      enabled = 0;
      return;
  }

  enabled = 1;
  locked = 1;

  /* Convert from ppm */
  max_freq *= 1e-6;
  max_wander *= 1e-6;

  UTI_ZeroTimespec(&last_update);

  LCL_AddParameterChangeHandler(handle_slew, NULL);
}

void SMT_Finalise(void)
{
}

int SMT_IsEnabled(void)
{
  return enabled;
}

double
SMT_GetOffset(struct timespec *now)
{
  double offset, freq;

  if (!enabled)
    return 0.0;
  
  get_smoothing(now, &offset, &freq, NULL);

  return offset;
}

void
SMT_Activate(struct timespec *now)
{
  if (!enabled || !locked)
    return;

  LOG(LOGS_INFO, "Time smoothing activated%s", leap_only_mode ?
      " (leap seconds only)" : "");
  locked = 0;
  last_update = *now;
}

void
SMT_Reset(struct timespec *now)
{
  int i;

  if (!enabled)
    return;

  smooth_offset = 0.0;
  smooth_freq = 0.0;
  last_update = *now;

  for (i = 0; i < NUM_STAGES; i++)
    stages[i].wander = stages[i].length = 0.0;
}

void
SMT_Leap(struct timespec *now, int leap)
{
  /* When the leap-only mode is disabled, the leap second will be accumulated
     in handle_slew() as a normal offset */
  if (!enabled || !leap_only_mode)
    return;

  update_smoothing(now, leap, 0.0);
}

int
SMT_GetSmoothingReport(RPT_SmoothingReport *report, struct timespec *now)
{
  double length, elapsed;
  int i;

  if (!enabled)
    return 0;

  report->active = !locked;
  report->leap_only = leap_only_mode;

  get_smoothing(now, &report->offset, &report->freq_ppm, &report->wander_ppm);

  /* Convert to ppm and negate (positive values mean faster/speeding up) */
  report->freq_ppm *= -1.0e6;
  report->wander_ppm *= -1.0e6;

  elapsed = UTI_DiffTimespecsToDouble(now, &last_update);
  if (!locked && elapsed >= 0.0) {
    for (i = 0, length = 0.0; i < NUM_STAGES; i++)
      length += stages[i].length;
    report->last_update_ago = elapsed;
    report->remaining_time = elapsed < length ? length - elapsed : 0.0;
  } else {
    report->last_update_ago = 0.0;
    report->remaining_time = 0.0;
  }

  return 1;
}
