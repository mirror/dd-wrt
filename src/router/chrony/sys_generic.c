/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2014-2015
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

  Generic driver functions to complete system-specific drivers
  */

#include "config.h"

#include "sysincl.h"

#include "sys_generic.h"

#include "conf.h"
#include "local.h"
#include "localp.h"
#include "logging.h"
#include "privops.h"
#include "sched.h"
#include "util.h"

/* ================================================== */

/* System clock drivers */
static lcl_ReadFrequencyDriver drv_read_freq;
static lcl_SetFrequencyDriver drv_set_freq;
static lcl_SetSyncStatusDriver drv_set_sync_status;
static lcl_AccrueOffsetDriver drv_accrue_offset;
static lcl_OffsetCorrectionDriver drv_get_offset_correction;

/* Current frequency as requested by the local module (in ppm) */
static double base_freq;

/* Maximum frequency that can be set by drv_set_freq (in ppm) */
static double max_freq;

/* Maximum expected delay in the actual frequency change (e.g. kernel ticks)
   in local time */
static double max_freq_change_delay;

/* Maximum allowed frequency offset relative to the base frequency */
static double max_corr_freq;

/* Amount of outstanding offset to process */
static double offset_register;

/* Minimum offset to correct */
#define MIN_OFFSET_CORRECTION 1.0e-9

/* Current frequency offset between base_freq and the real clock frequency
   as set by drv_set_freq (not in ppm) */
static double slew_freq;

/* Time (raw) of last update of slewing frequency and offset */
static struct timespec slew_start;

/* Limits for the slew timeout */
#define MIN_SLEW_TIMEOUT 1.0
#define MAX_SLEW_TIMEOUT 1.0e4

/* Scheduler timeout ID for ending of the currently running slew */
static SCH_TimeoutID slew_timeout_id;

/* Suggested offset correction rate (correction time * offset) */
static double correction_rate;

/* Maximum expected offset correction error caused by delayed change in the
   real frequency of the clock */
static double slew_error;

/* Minimum offset that the system driver can slew faster than the maximum
   frequency offset that it allows to be set directly */
static double fastslew_min_offset;

/* Maximum slew rate of the system driver */
static double fastslew_max_rate;

/* Flag indicating that the system driver is currently slewing */
static int fastslew_active;

/* ================================================== */

static void handle_end_of_slew(void *anything);
static void update_slew(void);

/* ================================================== */
/* Adjust slew_start on clock step */

static void
handle_step(struct timespec *raw, struct timespec *cooked, double dfreq,
            double doffset, LCL_ChangeType change_type, void *anything)
{
  if (change_type == LCL_ChangeUnknownStep) {
    /* Reset offset and slewing */
    slew_start = *raw;
    offset_register = 0.0;
    update_slew();
  } else if (change_type == LCL_ChangeStep) {
    UTI_AddDoubleToTimespec(&slew_start, -doffset, &slew_start);
  }
}

/* ================================================== */

static void
start_fastslew(void)
{
  if (!drv_accrue_offset)
    return;

  drv_accrue_offset(offset_register, 0.0);

  DEBUG_LOG("fastslew offset=%e", offset_register);

  offset_register = 0.0;
  fastslew_active = 1;
}

/* ================================================== */

static void
stop_fastslew(struct timespec *now)
{
  double corr;

  if (!drv_get_offset_correction || !fastslew_active)
    return;

  /* Cancel the remaining offset */
  drv_get_offset_correction(now, &corr, NULL);
  drv_accrue_offset(corr, 0.0);
  offset_register -= corr;
}

/* ================================================== */

static double
clamp_freq(double freq)
{
  if (freq > max_freq)
    return max_freq;
  if (freq < -max_freq)
    return -max_freq;
  return freq;
}

/* ================================================== */
/* End currently running slew and start a new one */

static void
update_slew(void)
{
  struct timespec now, end_of_slew;
  double old_slew_freq, total_freq, corr_freq, duration;

  /* Remove currently running timeout */
  SCH_RemoveTimeout(slew_timeout_id);

  LCL_ReadRawTime(&now);

  /* Adjust the offset register by achieved slew */
  duration = UTI_DiffTimespecsToDouble(&now, &slew_start);
  offset_register -= slew_freq * duration;

  stop_fastslew(&now);

  /* Estimate how long should the next slew take */
  if (fabs(offset_register) < MIN_OFFSET_CORRECTION) {
    duration = MAX_SLEW_TIMEOUT;
  } else {
    duration = correction_rate / fabs(offset_register);
    if (duration < MIN_SLEW_TIMEOUT)
      duration = MIN_SLEW_TIMEOUT;
  }

  /* Get frequency offset needed to slew the offset in the duration
     and clamp it to the allowed maximum */
  corr_freq = offset_register / duration;
  if (corr_freq < -max_corr_freq)
    corr_freq = -max_corr_freq;
  else if (corr_freq > max_corr_freq)
    corr_freq = max_corr_freq;

  /* Let the system driver perform the slew if the requested frequency
     offset is too large for the frequency driver */
  if (drv_accrue_offset && fabs(corr_freq) >= fastslew_max_rate &&
      fabs(offset_register) > fastslew_min_offset) {
    start_fastslew();
    corr_freq = 0.0;
  }

  /* Get the new real frequency and clamp it */
  total_freq = clamp_freq(base_freq + corr_freq * (1.0e6 - base_freq));

  /* Set the new frequency (the actual frequency returned by the call may be
     slightly different from the requested frequency due to rounding) */
  total_freq = (*drv_set_freq)(total_freq);

  /* Compute the new slewing frequency, it's relative to the real frequency to
     make the calculation in offset_convert() cheaper */
  old_slew_freq = slew_freq;
  slew_freq = (total_freq - base_freq) / (1.0e6 - total_freq);

  /* Compute the dispersion introduced by changing frequency and add it
     to all statistics held at higher levels in the system */
  slew_error = fabs((old_slew_freq - slew_freq) * max_freq_change_delay);
  if (slew_error >= MIN_OFFSET_CORRECTION)
    lcl_InvokeDispersionNotifyHandlers(slew_error);

  /* Compute the duration of the slew and clamp it.  If the slewing frequency
     is zero or has wrong sign (e.g. due to rounding in the frequency driver or
     when base_freq is larger than max_freq, or fast slew is active), use the
     maximum timeout and try again on the next update. */
  if (fabs(offset_register) < MIN_OFFSET_CORRECTION ||
      offset_register * slew_freq <= 0.0) {
    duration = MAX_SLEW_TIMEOUT;
  } else {
    duration = offset_register / slew_freq;
    if (duration < MIN_SLEW_TIMEOUT)
      duration = MIN_SLEW_TIMEOUT;
    else if (duration > MAX_SLEW_TIMEOUT)
      duration = MAX_SLEW_TIMEOUT;
  }

  /* Restart timer for the next update */
  UTI_AddDoubleToTimespec(&now, duration, &end_of_slew);
  slew_timeout_id = SCH_AddTimeout(&end_of_slew, handle_end_of_slew, NULL);
  slew_start = now;

  DEBUG_LOG("slew offset=%e corr_rate=%e base_freq=%f total_freq=%f slew_freq=%e duration=%f slew_error=%e",
      offset_register, correction_rate, base_freq, total_freq, slew_freq,
      duration, slew_error);
}

/* ================================================== */

static void
handle_end_of_slew(void *anything)
{
  slew_timeout_id = 0;
  update_slew();
}

/* ================================================== */

static double
read_frequency(void)
{
  return base_freq;
}

/* ================================================== */

static double
set_frequency(double freq_ppm)
{
  base_freq = freq_ppm;
  update_slew();

  return base_freq;
}

/* ================================================== */

static void
accrue_offset(double offset, double corr_rate)
{
  offset_register += offset;
  correction_rate = corr_rate;

  update_slew();
}

/* ================================================== */
/* Determine the correction to generate the cooked time for given raw time */

static void
offset_convert(struct timespec *raw,
               double *corr, double *err)
{
  double duration, fastslew_corr, fastslew_err;

  duration = UTI_DiffTimespecsToDouble(raw, &slew_start);

  if (drv_get_offset_correction && fastslew_active) {
    drv_get_offset_correction(raw, &fastslew_corr, &fastslew_err);
    if (fastslew_corr == 0.0 && fastslew_err == 0.0)
      fastslew_active = 0;
  } else {
    fastslew_corr = fastslew_err = 0.0;
  }

  *corr = slew_freq * duration + fastslew_corr - offset_register;

  if (err) {
    *err = fastslew_err;
    if (fabs(duration) <= max_freq_change_delay)
      *err += slew_error;
  }
}

/* ================================================== */
/* Positive means currently fast of true time, i.e. jump backwards */

static int
apply_step_offset(double offset)
{
  struct timespec old_time, new_time;
  struct timeval new_time_tv;
  double err;

  LCL_ReadRawTime(&old_time);
  UTI_AddDoubleToTimespec(&old_time, -offset, &new_time);
  UTI_TimespecToTimeval(&new_time, &new_time_tv);

  if (PRV_SetTime(&new_time_tv, NULL) < 0) {
    DEBUG_LOG("settimeofday() failed");
    return 0;
  }

  LCL_ReadRawTime(&old_time);
  err = UTI_DiffTimespecsToDouble(&old_time, &new_time);

  lcl_InvokeDispersionNotifyHandlers(fabs(err));

  return 1;
}

/* ================================================== */

static void
set_sync_status(int synchronised, double est_error, double max_error)
{
  double offset;

  offset = fabs(offset_register);
  if (est_error < offset)
    est_error = offset;
  max_error += offset;

  if (drv_set_sync_status)
    drv_set_sync_status(synchronised, est_error, max_error);
}

/* ================================================== */

void
SYS_Generic_CompleteFreqDriver(double max_set_freq_ppm, double max_set_freq_delay,
                               lcl_ReadFrequencyDriver sys_read_freq,
                               lcl_SetFrequencyDriver sys_set_freq,
                               lcl_ApplyStepOffsetDriver sys_apply_step_offset,
                               double min_fastslew_offset, double max_fastslew_rate,
                               lcl_AccrueOffsetDriver sys_accrue_offset,
                               lcl_OffsetCorrectionDriver sys_get_offset_correction,
                               lcl_SetLeapDriver sys_set_leap,
                               lcl_SetSyncStatusDriver sys_set_sync_status)
{
  max_freq = max_set_freq_ppm;
  max_freq_change_delay = max_set_freq_delay * (1.0 + max_freq / 1.0e6);
  drv_read_freq = sys_read_freq;
  drv_set_freq = sys_set_freq;
  drv_accrue_offset = sys_accrue_offset;
  drv_get_offset_correction = sys_get_offset_correction;
  drv_set_sync_status = sys_set_sync_status;

  base_freq = (*drv_read_freq)();
  slew_freq = 0.0;
  offset_register = 0.0;

  max_corr_freq = CNF_GetMaxSlewRate() / 1.0e6;

  fastslew_min_offset = min_fastslew_offset;
  fastslew_max_rate = max_fastslew_rate / 1.0e6;
  fastslew_active = 0;

  lcl_RegisterSystemDrivers(read_frequency, set_frequency,
                            accrue_offset, sys_apply_step_offset ?
                              sys_apply_step_offset : apply_step_offset,
                            offset_convert, sys_set_leap, set_sync_status);

  LCL_AddParameterChangeHandler(handle_step, NULL);
}

/* ================================================== */

void
SYS_Generic_Finalise(void)
{
  struct timespec now;

  /* Must *NOT* leave a slew running - clock could drift way off
     if the daemon is not restarted */

  SCH_RemoveTimeout(slew_timeout_id);
  slew_timeout_id = 0;

  (*drv_set_freq)(clamp_freq(base_freq));

  LCL_ReadRawTime(&now);
  stop_fastslew(&now);
}

/* ================================================== */
