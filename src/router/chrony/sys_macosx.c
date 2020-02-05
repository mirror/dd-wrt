/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2001
 * Copyright (C) J. Hannken-Illjes  2001
 * Copyright (C) Bryan Christianson  2015, 2017
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

  Driver file for the macOS operating system.

  */

#include "config.h"

#ifdef MACOSX

#include "sysincl.h"

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <pthread.h>

#include "sys_macosx.h"
#include "conf.h"
#include "local.h"
#include "localp.h"
#include "logging.h"
#include "sched.h"
#include "privops.h"
#include "util.h"

#ifdef HAVE_MACOS_SYS_TIMEX
#include <dlfcn.h>
#include "sys_netbsd.h"

static int have_ntp_adjtime = 0;
#endif

/* ================================================== */

/* This register contains the number of seconds by which the local
   clock was estimated to be fast of reference time at the epoch when
   LCL_ReadRawTime() returned T0 */

static double offset_register;

/* This register contains the epoch to which the offset is referenced */

static struct timespec T0;

/* This register contains the current estimate of the system
   frequency, in absolute (NOT ppm) */

static double current_freq;

/* This register contains the number of seconds of adjustment that
   were passed to adjtime last time it was called. */

static double adjustment_requested;

/* Interval in seconds between adjustments to cancel systematic drift */

#define DRIFT_REMOVAL_INTERVAL (4.0)
#define DRIFT_REMOVAL_INTERVAL_MIN (0.5)

/* If current_drift_removal_interval / drift_removal_interval exceeds this
   ratio, then restart the drift removal timer */

#define DRIFT_REMOVAL_RESTART_RATIO (8.0)

static double drift_removal_interval;
static double current_drift_removal_interval;
static struct timespec Tdrift;

/* weighting applied to error in calculating drift_removal_interval */
#define ERROR_WEIGHT (0.5)

/* minimum resolution of current_frequency */
#define FREQUENCY_RES (1.0e-9)

#define NANOS_PER_MSEC (1000000ULL)

/* RTC synchronisation - once an hour */

static struct timespec last_rtc_sync;
#define RTC_SYNC_INTERVAL (60 * 60.0)

/* ================================================== */

static void
clock_initialise(void)
{
  struct timeval newadj, oldadj;

  offset_register = 0.0;
  adjustment_requested = 0.0;
  current_freq = 0.0;
  drift_removal_interval = DRIFT_REMOVAL_INTERVAL;
  current_drift_removal_interval = DRIFT_REMOVAL_INTERVAL;

  LCL_ReadRawTime(&T0);
  Tdrift = T0;
  last_rtc_sync = T0;

  newadj.tv_sec = 0;
  newadj.tv_usec = 0;

  if (PRV_AdjustTime(&newadj, &oldadj) < 0) {
    LOG_FATAL("adjtime() failed");
  }
}

/* ================================================== */

static void
clock_finalise(void)
{
  /* Nothing to do yet */
}

/* ================================================== */

static void
start_adjust(void)
{
  struct timeval newadj, oldadj;
  struct timespec T1;
  double elapsed, accrued_error, predicted_error, drift_removal_elapsed;
  double adjust_required;
  double rounding_error;
  double old_adjust_remaining;

  /* Determine the amount of error built up since the last adjustment */
  LCL_ReadRawTime(&T1);

  elapsed = UTI_DiffTimespecsToDouble(&T1, &T0);
  accrued_error = elapsed * current_freq;

  drift_removal_elapsed = UTI_DiffTimespecsToDouble(&T1, &Tdrift);

  /* To allow for the clock being stepped either forward or backwards, clamp
     the elapsed time to bounds [ 0.0, current_drift_removal_interval ] */
  drift_removal_elapsed = MIN(MAX(0.0, drift_removal_elapsed), current_drift_removal_interval);

  predicted_error = (current_drift_removal_interval - drift_removal_elapsed) / 2.0 * current_freq;

  DEBUG_LOG("drift_removal_elapsed: %.3f current_drift_removal_interval: %.3f predicted_error: %.3f",
            1.0e6 * drift_removal_elapsed, 1.0e6 * current_drift_removal_interval,
            1.0e6 * predicted_error);

  adjust_required = - (accrued_error + offset_register + predicted_error);

  UTI_DoubleToTimeval(adjust_required, &newadj);
  adjustment_requested = UTI_TimevalToDouble(&newadj);
  rounding_error = adjust_required - adjustment_requested;

  if (PRV_AdjustTime(&newadj, &oldadj) < 0) {
    LOG_FATAL("adjtime() failed");
  }

  old_adjust_remaining = UTI_TimevalToDouble(&oldadj);

  offset_register = rounding_error - old_adjust_remaining - predicted_error;

  T0 = T1;
}

/* ================================================== */

static void
stop_adjust(void)
{
  struct timespec T1;
  struct timeval zeroadj, remadj;
  double adjustment_remaining, adjustment_achieved;
  double elapsed, elapsed_plus_adjust;

  zeroadj.tv_sec = 0;
  zeroadj.tv_usec = 0;

  if (PRV_AdjustTime(&zeroadj, &remadj) < 0) {
    LOG_FATAL("adjtime() failed");
  }

  LCL_ReadRawTime(&T1);

  elapsed = UTI_DiffTimespecsToDouble(&T1, &T0);
  adjustment_remaining = UTI_TimevalToDouble(&remadj);

  adjustment_achieved = adjustment_requested - adjustment_remaining;
  elapsed_plus_adjust = elapsed - adjustment_achieved;

  offset_register += current_freq * elapsed_plus_adjust - adjustment_remaining;

  adjustment_requested = 0.0;
  T0 = T1;
}

/* ================================================== */

/* Positive offset means system clock is fast of true time, therefore
   slew backwards */

static void
accrue_offset(double offset, double corr_rate)
{
  stop_adjust();
  offset_register += offset;
  start_adjust();
}

/* ================================================== */

/* Positive offset means system clock is fast of true time, therefore
   step backwards */

static int
apply_step_offset(double offset)
{
  struct timespec old_time, new_time, T1;
  struct timeval new_time_tv;

  stop_adjust();

  LCL_ReadRawTime(&old_time);

  UTI_AddDoubleToTimespec(&old_time, -offset, &new_time);
  UTI_TimespecToTimeval(&new_time, &new_time_tv);

  if (PRV_SetTime(&new_time_tv, NULL) < 0) {
    DEBUG_LOG("settimeofday() failed");
    return 0;
  }

  UTI_AddDoubleToTimespec(&T0, -offset, &T1);
  T0 = T1;

  start_adjust();

  return 1;
}

/* ================================================== */

static double
set_frequency(double new_freq_ppm)
{
  stop_adjust();
  current_freq = new_freq_ppm * 1.0e-6;
  start_adjust();

  return current_freq * 1.0e6;
}

/* ================================================== */

static double
read_frequency(void)
{
  return current_freq * 1.0e6;
}

/* ================================================== */

static void
get_offset_correction(struct timespec *raw,
                      double *corr, double *err)
{
  stop_adjust();
  *corr = -offset_register;
  start_adjust();
  if (err)
    *err = 0.0;
}

/* ================================================== */

/* Cancel systematic drift */

static SCH_TimeoutID drift_removal_id;

/* ================================================== */
/* This is the timer callback routine which is called periodically to
 invoke a time adjustment to take out the machine's drift.
 Otherwise, times reported through this software (e.g. by running
 ntpdate from another machine) show the machine being correct (since
 they correct for drift build-up), but any program on this machine
 that reads the system time will be given an erroneous value, the
 degree of error depending on how long it is since
 get_offset_correction was last called. */

static void
drift_removal_timeout(SCH_ArbitraryArgument not_used)
{

  stop_adjust();

  LCL_ReadRawTime(&Tdrift);

  current_drift_removal_interval = drift_removal_interval;

  start_adjust();

  drift_removal_id = SCH_AddTimeoutByDelay(drift_removal_interval, drift_removal_timeout, NULL);
}

/* ================================================== */

/* use est_error to calculate the drift_removal_interval and
   update the RTC */

static void
set_sync_status(int synchronised, double est_error, double max_error)
{
  double interval;

  if (!synchronised) {
    drift_removal_interval = MAX(drift_removal_interval, DRIFT_REMOVAL_INTERVAL);
  } else {
    if (CNF_GetRtcSync()) {
      struct timespec now;
      double rtc_sync_elapsed;

      SCH_GetLastEventTime(NULL, NULL, &now);
      rtc_sync_elapsed = UTI_DiffTimespecsToDouble(&now, &last_rtc_sync);
      if (fabs(rtc_sync_elapsed) >= RTC_SYNC_INTERVAL) {
        /* update the RTC by applying a step of 0.0 secs */
        apply_step_offset(0.0);
        last_rtc_sync = now;
        DEBUG_LOG("rtc synchronised");
      }
    }

    interval = ERROR_WEIGHT * est_error / (fabs(current_freq) + FREQUENCY_RES);
    drift_removal_interval = MAX(interval, DRIFT_REMOVAL_INTERVAL_MIN);

    DEBUG_LOG("est_error: %.3f current_freq: %.3f est drift_removal_interval: %.3f act drift_removal_interval: %.3f",
                est_error * 1.0e6, current_freq * 1.0e6, interval, drift_removal_interval);
  }

  if (current_drift_removal_interval / drift_removal_interval > DRIFT_REMOVAL_RESTART_RATIO) {
    /* recover from a large est_error by resetting the timer */
    SCH_ArbitraryArgument unused;
    SCH_RemoveTimeout(drift_removal_id);
    unused = NULL;
    drift_removal_timeout(unused);
  }
}

/* ================================================== */
/*
  Give chronyd real time priority so that time critical calculations
  are not pre-empted by the kernel.
*/

static int
set_realtime(void)
{
  /* https://developer.apple.com/library/ios/technotes/tn2169/_index.html */

  mach_timebase_info_data_t timebase_info;
  double clock2abs;
  thread_time_constraint_policy_data_t policy;
  int kr;

  mach_timebase_info(&timebase_info);
  clock2abs = ((double)timebase_info.denom / (double)timebase_info.numer) * NANOS_PER_MSEC;

  policy.period = 0;
  policy.computation = (uint32_t)(5 * clock2abs); /* 5 ms of work */
  policy.constraint = (uint32_t)(10 * clock2abs);
  policy.preemptible = 0;

  kr = thread_policy_set(
          pthread_mach_thread_np(pthread_self()),
          THREAD_TIME_CONSTRAINT_POLICY,
          (thread_policy_t)&policy,
          THREAD_TIME_CONSTRAINT_POLICY_COUNT);

  if (kr != KERN_SUCCESS) {
    LOG(LOGS_WARN, "Cannot set real-time priority: %d", kr);
    return -1;
  }
  return 0;
}

/* ================================================== */

void
SYS_MacOSX_SetScheduler(int SchedPriority)
{
  if (SchedPriority) {
    set_realtime();
  }
}

/* ================================================== */

#ifdef FEAT_PRIVDROP
void SYS_MacOSX_DropRoot(uid_t uid, gid_t gid)
{
  PRV_StartHelper();

  UTI_DropRoot(uid, gid);
}
#endif

/* ================================================== */

static void
legacy_MacOSX_Initialise(void)
{
  clock_initialise();

  lcl_RegisterSystemDrivers(read_frequency, set_frequency,
                            accrue_offset, apply_step_offset,
                            get_offset_correction,
                            NULL /* set_leap */,
                            set_sync_status);


  drift_removal_id = SCH_AddTimeoutByDelay(drift_removal_interval, drift_removal_timeout, NULL);
}

/* ================================================== */

static void
legacy_MacOSX_Finalise(void)
{
  SCH_RemoveTimeout(drift_removal_id);

  clock_finalise();
}

/* ================================================== */

void
SYS_MacOSX_Initialise(void)
{
#ifdef HAVE_MACOS_SYS_TIMEX
  have_ntp_adjtime = (dlsym(RTLD_NEXT, "ntp_adjtime") != NULL);
  if (have_ntp_adjtime) {
    SYS_NetBSD_Initialise();
    return;
  }
#endif
  legacy_MacOSX_Initialise();
}

/* ================================================== */

void
SYS_MacOSX_Finalise(void)
{
#ifdef HAVE_MACOS_SYS_TIMEX
  if (have_ntp_adjtime) {
    SYS_NetBSD_Finalise();
    return;
  }
#endif
  legacy_MacOSX_Finalise();
}

#endif
