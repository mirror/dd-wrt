/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2012-2014
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

  Real-time clock driver for linux.  This interfaces the program with
  the clock that keeps time when the machine is turned off.

  */

#include "config.h"

#include "sysincl.h"

#include <linux/rtc.h>

#include "logging.h"
#include "sched.h"
#include "local.h"
#include "util.h"
#include "sys_linux.h"
#include "reference.h"
#include "regress.h"
#include "rtc.h"
#include "rtc_linux.h"
#include "conf.h"
#include "memory.h"

/* ================================================== */
/* Forward prototypes */

static void measurement_timeout(void *any);

static void read_from_device(int fd_, int event, void *any);

/* ================================================== */

typedef enum {
  OM_NORMAL,
  OM_INITIAL,
  OM_AFTERTRIM
} OperatingMode;

static OperatingMode operating_mode = OM_NORMAL;

/* ================================================== */

static int fd = -1;

#define LOWEST_MEASUREMENT_PERIOD 15
#define HIGHEST_MEASUREMENT_PERIOD 480
#define N_SAMPLES_PER_REGRESSION 1

static int measurement_period = LOWEST_MEASUREMENT_PERIOD;

static SCH_TimeoutID timeout_id = 0;

static int skip_interrupts;

/* ================================================== */

/* Maximum number of samples held */
#define MAX_SAMPLES 64

/* Real time clock samples.  We store the seconds count as originally
   measured, together with a 'trim' that compensates these values for
   any steps made to the RTC to bring it back into line
   occasionally.  The trim is in seconds. */
static time_t *rtc_sec = NULL;
static double *rtc_trim = NULL;

/* Reference time, against which delta times on the RTC scale are measured */
static time_t rtc_ref;


/* System clock samples associated with the above samples. */
static struct timespec *system_times = NULL;

/* Number of samples currently stored. */
static int n_samples;   

/* Number of new samples since last regression */
static int n_samples_since_regression;

/* Number of runs of residuals in last regression (for logging) */
static int n_runs;

/* Coefficients */
/* Whether they are valid */
static int coefs_valid;

/* Reference time */
static time_t coef_ref_time;
/* Number of seconds by which RTC was fast of the system time at coef_ref_time */
static double coef_seconds_fast;

/* Estimated number of seconds that RTC gains relative to system time
   for each second of ITS OWN time */
static double coef_gain_rate;

/* Gain rate saved just before we step the RTC to correct it to the
   nearest second, so that we can write a useful set of coefs to the
   RTC data file once we have reacquired its offset after the step */
static double saved_coef_gain_rate;

/* Threshold for automatic RTC trimming in seconds, zero when disabled */
static double autotrim_threshold;

/* Filename supplied by config file where RTC coefficients are
   stored. */
static char *coefs_file_name;

/* ================================================== */
/* Coefficients read from file at start of run. */

/* Whether we have tried to load the coefficients */
static int tried_to_load_coefs = 0;

/* Whether valid coefficients were read */
static int valid_coefs_from_file = 0;

/* Coefs read in */
static time_t file_ref_time;
static double file_ref_offset, file_rate_ppm;

/* ================================================== */

/* Flag to remember whether to assume the RTC is running on UTC */
static int rtc_on_utc = 1;

/* ================================================== */

static LOG_FileID logfileid;

/* ================================================== */

static void (*after_init_hook)(void *) = NULL;
static void *after_init_hook_arg = NULL;

/* ================================================== */

static void
discard_samples(int new_first)
{
  int n_to_save;

  assert(new_first >= 0 && new_first < n_samples);

  n_to_save = n_samples - new_first;

  memmove(rtc_sec, rtc_sec + new_first, n_to_save * sizeof(time_t));
  memmove(rtc_trim, rtc_trim + new_first, n_to_save * sizeof(double));
  memmove(system_times, system_times + new_first, n_to_save * sizeof(struct timespec));

  n_samples = n_to_save;
}

/* ================================================== */

#define NEW_FIRST_WHEN_FULL 4

static void
accumulate_sample(time_t rtc, struct timespec *sys)
{

  if (n_samples == MAX_SAMPLES) {
    /* Discard oldest samples */
    discard_samples(NEW_FIRST_WHEN_FULL);
  }

  /* Discard all samples if the RTC was stepped back (not our trim) */
  if (n_samples > 0 && rtc_sec[n_samples - 1] - rtc >= rtc_trim[n_samples - 1]) {
    DEBUG_LOG("RTC samples discarded");
    n_samples = 0;
  }

  /* Always use most recent sample as reference */
  /* use sample only if n_sample is not negative*/
  if(n_samples >=0)
  {
  rtc_ref = rtc;
  rtc_sec[n_samples] = rtc;
  rtc_trim[n_samples] = 0.0;
  system_times[n_samples] = *sys;
  ++n_samples_since_regression;
  }
  ++n_samples;
}

/* ================================================== */
/* The new_sample flag is to indicate whether to adjust the
   measurement period depending on the behaviour of the standard
   deviation. */

static void
run_regression(int new_sample,
               int *valid,
               time_t *ref,
               double *fast,
               double *slope)
{
  double rtc_rel[MAX_SAMPLES]; /* Relative times on RTC axis */
  double offsets[MAX_SAMPLES]; /* How much the RTC is fast of the system clock */
  int i;
  double est_intercept, est_slope;
  int best_new_start;

  if (n_samples > 0) {

    for (i=0; i<n_samples; i++) {
      rtc_rel[i] = rtc_trim[i] + (double)(rtc_sec[i] - rtc_ref);
      offsets[i] = ((double) (rtc_ref - system_times[i].tv_sec) -
                    (1.0e-9 * system_times[i].tv_nsec) +
                    rtc_rel[i]);

    }

    if (RGR_FindBestRobustRegression
        (rtc_rel, offsets,
         n_samples, 1.0e-9,
         &est_intercept, &est_slope,
         &n_runs,
         &best_new_start)) {

      /* Calculate and store coefficients.  We don't do any error
         bounds processing on any of these. */
      *valid = 1;
      *ref = rtc_ref;
      *fast = est_intercept;
      *slope = est_slope;

      if (best_new_start > 0) {
        discard_samples(best_new_start);
      }


    } else {
      /* Keep existing coefficients. */
    }
  } else {
    /* Keep existing coefficients. */
  }

}

/* ================================================== */

static void
slew_samples
(struct timespec *raw, struct timespec *cooked,
 double dfreq,
 double doffset,
 LCL_ChangeType change_type,
 void *anything)
{
  int i;
  double delta_time;
  double old_seconds_fast, old_gain_rate;

  if (change_type == LCL_ChangeUnknownStep) {
    /* Drop all samples. */
    n_samples = 0;
  }

  for (i=0; i<n_samples; i++) {
    UTI_AdjustTimespec(system_times + i, cooked, system_times + i, &delta_time,
        dfreq, doffset);
  }

  old_seconds_fast = coef_seconds_fast;
  old_gain_rate = coef_gain_rate;

  if (coefs_valid) {
    coef_seconds_fast += doffset;
    coef_gain_rate += dfreq * (1.0 - coef_gain_rate);
  }

  DEBUG_LOG("dfreq=%.8f doffset=%.6f old_fast=%.6f old_rate=%.3f new_fast=%.6f new_rate=%.3f",
      dfreq, doffset,
      old_seconds_fast, 1.0e6 * old_gain_rate,
      coef_seconds_fast, 1.0e6 * coef_gain_rate);
}

/* ================================================== */

/* Function to convert from a time_t value represenging UTC to the
   corresponding real time clock 'DMY HMS' form, taking account of
   whether the user runs his RTC on the local time zone or UTC */

static struct tm *
rtc_from_t(const time_t *t)
{
  if (rtc_on_utc) {
    return gmtime(t);
  } else {
    return localtime(t);
  }
}

/* ================================================== */

/* Inverse function to get back from RTC 'DMY HMS' form to time_t UTC
   form.  This essentially uses mktime(), but involves some awful
   complexity to cope with timezones.  The problem is that mktime's
   behaviour with regard to the daylight saving flag in the 'struct
   tm' does not seem to be reliable across all systems, unless that
   flag is set to zero. 

   tm_isdst = -1 does not seem to work with all libc's - it is treated
   as meaning there is DST, or fails completely.  (It is supposed to
   use the timezone info to work out whether summer time is active at
   the specified epoch).

   tm_isdst = 1 fails if the local timezone has no summer time defined.

   The approach taken is as follows.  Suppose the RTC is on localtime.
   We perform all mktime calls with the tm_isdst field set to zero.

   Let y be the RTC reading in 'DMY HMS' form.  Let M be the mktime
   function with tm_isdst=0 and L be the localtime function.

   We seek x such that y = L(x).  Now there will exist a value Z(t)
   such that M(L(t)) = t + Z(t) for all t, where Z(t) depends on
   whether daylight saving is active at time t.

   We want L(x) = y.  Therefore M(L(x)) = x + Z = M(y).  But
   M(L(M(y))) = M(y) + Z.  Therefore x = M(y) - Z = M(y) - (M(L(M(y)))
   - M(y)).

   The case for the RTC running on UTC is identical but without the
   potential complication that Z depends on t.
*/

static time_t
t_from_rtc(struct tm *stm) {
  struct tm temp1, temp2, *tm;
  long diff;
  time_t t1, t2;

  temp1 = *stm;
  temp1.tm_isdst = 0;
  
  t1 = mktime(&temp1);

  tm = rtc_on_utc ? gmtime(&t1) : localtime(&t1);
  if (!tm) {
    DEBUG_LOG("gmtime()/localtime() failed");
    return -1;
  }

  temp2 = *tm;
  temp2.tm_isdst = 0;
  t2 = mktime(&temp2);
  diff = t2 - t1;

  if (t1 - diff == -1)
    DEBUG_LOG("Could not convert RTC time");

  return t1 - diff;
}

/* ================================================== */

static void
read_hwclock_file(const char *hwclock_file)
{
  FILE *in;
  char line[256];
  int i;

  if (!hwclock_file || !hwclock_file[0])
    return;

  in = fopen(hwclock_file, "r");
  if (!in) {
    LOG(LOGS_WARN, "Could not open %s : %s",
        hwclock_file, strerror(errno));
    return;
  }

  /* Read third line from the file. */
  for (i = 0; i < 3; i++) {
    if (!fgets(line, sizeof(line), in))
      break;
  }

  fclose(in);

  if (i == 3 && !strncmp(line, "LOCAL", 5)) {
    rtc_on_utc = 0;
  } else if (i == 3 && !strncmp(line, "UTC", 3)) {
    rtc_on_utc = 1;
  } else {
    LOG(LOGS_WARN, "Could not read RTC LOCAL/UTC setting from %s", hwclock_file);
  }
}

/* ================================================== */

static void
setup_config(void)
{
  if (CNF_GetRtcOnUtc()) {
    rtc_on_utc = 1;
  } else {
    rtc_on_utc = 0;
  }

  read_hwclock_file(CNF_GetHwclockFile());

  autotrim_threshold = CNF_GetRtcAutotrim();
}

/* ================================================== */
/* Read the coefficients from the file where they were saved
   the last time the program was run. */

static void
read_coefs_from_file(void)
{
  FILE *in;

  if (!tried_to_load_coefs) {

    valid_coefs_from_file = 0; /* only gets set true if we succeed */

    tried_to_load_coefs = 1;

    if (coefs_file_name && (in = fopen(coefs_file_name, "r"))) {
      if (fscanf(in, "%d%ld%lf%lf",
                 &valid_coefs_from_file,
                 &file_ref_time,
                 &file_ref_offset,
                 &file_rate_ppm) == 4) {
      } else {
        LOG(LOGS_WARN, "Could not read coefficients from %s", coefs_file_name);
      }
      fclose(in);
    }
  }
}

/* ================================================== */
/* Write the coefficients to the file where they will be read
   the next time the program is run. */

static int
write_coefs_to_file(int valid,time_t ref_time,double offset,double rate)
{
  struct stat buf;
  char *temp_coefs_file_name;
  FILE *out;
  int r1, r2;

  /* Create a temporary file with a '.tmp' extension. */

  temp_coefs_file_name = (char*) Malloc(strlen(coefs_file_name)+8);

  if(!temp_coefs_file_name) {
    return RTC_ST_BADFILE;
  }

  strcpy(temp_coefs_file_name,coefs_file_name);
  strcat(temp_coefs_file_name,".tmp");

  out = fopen(temp_coefs_file_name, "w");
  if (!out) {
    Free(temp_coefs_file_name);
    LOG(LOGS_WARN, "Could not open temporary RTC file %s.tmp for writing",
        coefs_file_name);
    return RTC_ST_BADFILE;
  }

  /* Gain rate is written out in ppm */
  r1 = fprintf(out, "%1d %ld %.6f %.3f\n",
               valid, ref_time, offset, 1.0e6 * rate);
  r2 = fclose(out);
  if (r1 < 0 || r2) {
    Free(temp_coefs_file_name);
    LOG(LOGS_WARN, "Could not write to temporary RTC file %s.tmp",
        coefs_file_name);
    return RTC_ST_BADFILE;
  }

  /* Clone the file attributes from the existing file if there is one. */

  if (!stat(coefs_file_name,&buf)) {
    if (chown(temp_coefs_file_name,buf.st_uid,buf.st_gid) ||
        chmod(temp_coefs_file_name,buf.st_mode & 0777)) {
      LOG(LOGS_WARN,
          "Could not change ownership or permissions of temporary RTC file %s.tmp",
          coefs_file_name);
    }
  }

  /* Rename the temporary file to the correct location (see rename(2) for details). */

  if (rename(temp_coefs_file_name,coefs_file_name)) {
    unlink(temp_coefs_file_name);
    Free(temp_coefs_file_name);
    LOG(LOGS_WARN, "Could not replace old RTC file %s.tmp with new one %s",
        coefs_file_name, coefs_file_name);
    return RTC_ST_BADFILE;
  }

  Free(temp_coefs_file_name);

  return RTC_ST_OK;
}


/* ================================================== */
/* file_name is the name of the file where we save the RTC params
   between executions.  Return status is whether we could initialise
   on this version of the system. */

int
RTC_Linux_Initialise(void)
{
  rtc_sec = MallocArray(time_t, MAX_SAMPLES);
  rtc_trim = MallocArray(double, MAX_SAMPLES);
  system_times = MallocArray(struct timespec, MAX_SAMPLES);

  /* Setup details depending on configuration options */
  setup_config();

  /* In case it didn't get done by pre-init */
  coefs_file_name = CNF_GetRtcFile();

  /* Try to open device */

  fd = open (CNF_GetRtcDevice(), O_RDWR);
  if (fd < 0) {
    LOG(LOGS_ERR, "Could not open RTC device %s : %s",
        CNF_GetRtcDevice(), strerror(errno));
    return 0;
  }

  /* Close on exec */
  UTI_FdSetCloexec(fd);

  n_samples = 0;
  n_samples_since_regression = 0;
  n_runs = 0;
  coefs_valid = 0;

  measurement_period = LOWEST_MEASUREMENT_PERIOD;

  operating_mode = OM_NORMAL;

  /* Register file handler */
  SCH_AddFileHandler(fd, SCH_FILE_INPUT, read_from_device, NULL);

  /* Register slew handler */
  LCL_AddParameterChangeHandler(slew_samples, NULL);

  logfileid = CNF_GetLogRtc() ? LOG_FileOpen("rtc",
      "   Date (UTC) Time   RTC fast (s) Val   Est fast (s)   Slope (ppm)  Ns  Nr Meas")
    : -1;
  return 1;
}

/* ================================================== */

void
RTC_Linux_Finalise(void)
{
  SCH_RemoveTimeout(timeout_id);
  timeout_id = 0;

  /* Remove input file handler */
  if (fd >= 0) {
    SCH_RemoveFileHandler(fd);
    close(fd);

    /* Save the RTC data */
    (void) RTC_Linux_WriteParameters();

  }
  Free(rtc_sec);
  Free(rtc_trim);
  Free(system_times);
}

/* ================================================== */

static void
switch_interrupts(int onoff)
{
  int status;

  if (onoff) {
    status = ioctl(fd, RTC_UIE_ON, 0);
    if (status < 0) {
      LOG(LOGS_ERR, "Could not %s RTC interrupt : %s", "enable", strerror(errno));
      return;
    }
    skip_interrupts = 1;
  } else {
    status = ioctl(fd, RTC_UIE_OFF, 0);
    if (status < 0) {
      LOG(LOGS_ERR, "Could not %s RTC interrupt : %s", "disable", strerror(errno));
      return;
    }
  }
}    

/* ================================================== */

static void
measurement_timeout(void *any)
{
  timeout_id = 0;
  switch_interrupts(1);
}

/* ================================================== */

static void
set_rtc(time_t new_rtc_time)
{
  struct tm rtc_tm;
  struct rtc_time rtc_raw;
  int status;

  rtc_tm = *rtc_from_t(&new_rtc_time);

  rtc_raw.tm_sec = rtc_tm.tm_sec;
  rtc_raw.tm_min = rtc_tm.tm_min;
  rtc_raw.tm_hour = rtc_tm.tm_hour;
  rtc_raw.tm_mday = rtc_tm.tm_mday;
  rtc_raw.tm_mon = rtc_tm.tm_mon;
  rtc_raw.tm_year = rtc_tm.tm_year;
  rtc_raw.tm_wday = rtc_tm.tm_wday;
  rtc_raw.tm_yday = rtc_tm.tm_yday;
  rtc_raw.tm_isdst = rtc_tm.tm_isdst;

  status = ioctl(fd, RTC_SET_TIME, &rtc_raw);
  if (status < 0) {
    LOG(LOGS_ERR, "Could not set RTC time");
  }

}

/* ================================================== */

static void
handle_initial_trim(void)
{
  double rate;
  long delta_time;
  double rtc_error_now, sys_error_now;

    /* The idea is to accumulate some number of samples at 1 second
       intervals, then do a robust regression fit to this.  This
       should give a good fix on the intercept (=system clock error
       rel to RTC) at a particular time, removing risk of any
       particular sample being an outlier.  We can then look at the
       elapsed interval since the epoch recorded in the RTC file,
       and correct the system time accordingly. */
    
  run_regression(1, &coefs_valid, &coef_ref_time, &coef_seconds_fast, &coef_gain_rate);

  n_samples_since_regression = 0;

  /* Set sample number to -1 so the next sample is not used, as it will not yet be corrected for System Trim*/

  n_samples = -1;


  read_coefs_from_file();

  if (valid_coefs_from_file) {
    /* Can process data */
    delta_time = coef_ref_time - file_ref_time;
    rate = 1.0e-6 * file_rate_ppm;
    rtc_error_now = file_ref_offset + rate * (double) delta_time;
          
    /* sys_error_now is positive if the system clock is fast */
    sys_error_now = rtc_error_now - coef_seconds_fast;
          
    LCL_AccumulateOffset(sys_error_now, 0.0);
    LOG(LOGS_INFO, "System clock off from RTC by %f seconds (slew)",
        sys_error_now);
  } else {
    LOG(LOGS_WARN, "No valid rtcfile coefficients");
  }
  
  coefs_valid = 0;
  
  (after_init_hook)(after_init_hook_arg);
  
  operating_mode = OM_NORMAL;
}

/* ================================================== */

static void
handle_relock_after_trim(void)
{
  int valid;
  time_t ref;
  double fast, slope;

  valid = 0;
  run_regression(1, &valid, &ref, &fast, &slope);

  if (valid) {
    write_coefs_to_file(1,ref,fast,saved_coef_gain_rate);
  } else {
    DEBUG_LOG("Could not do regression after trim");
  }

  coefs_valid = 0;
  n_samples = 0;
  n_samples_since_regression = 0;
  operating_mode = OM_NORMAL;
  measurement_period = LOWEST_MEASUREMENT_PERIOD;
}

/* ================================================== */

static void
maybe_autotrim(void)
{
  /* Trim only when in normal mode, the coefficients are fresh, the current
     offset is above the threshold and the system clock is synchronized */

  if (operating_mode != OM_NORMAL || !coefs_valid || n_samples_since_regression)
    return;
  
  if (autotrim_threshold <= 0.0 || fabs(coef_seconds_fast) < autotrim_threshold)
    return;

  if (REF_GetOurStratum() >= 16)
    return;

  RTC_Linux_Trim();
}

/* ================================================== */

static void
process_reading(time_t rtc_time, struct timespec *system_time)
{
  double rtc_fast;

  accumulate_sample(rtc_time, system_time);

  switch (operating_mode) {
    case OM_NORMAL:

      if (n_samples_since_regression >= N_SAMPLES_PER_REGRESSION) {
        run_regression(1, &coefs_valid, &coef_ref_time, &coef_seconds_fast, &coef_gain_rate);
        n_samples_since_regression = 0;
        maybe_autotrim();
      }
      
      break;
    case OM_INITIAL:
      if (n_samples_since_regression >= 8) {
        handle_initial_trim();
      }
      break;
    case OM_AFTERTRIM:
      if (n_samples_since_regression >= 8) {
        handle_relock_after_trim();
      }
      break;
    default:
      assert(0);
      break;
  }  


  if (logfileid != -1) {
    rtc_fast = (rtc_time - system_time->tv_sec) - 1.0e-9 * system_time->tv_nsec;

    LOG_FileWrite(logfileid, "%s %14.6f %1d  %14.6f  %12.3f  %2d  %2d %4d",
            UTI_TimeToLogForm(system_time->tv_sec),
            rtc_fast,
            coefs_valid,
            coef_seconds_fast, coef_gain_rate * 1.0e6, n_samples, n_runs, measurement_period);
  }    

}

/* ================================================== */

static void
read_from_device(int fd_, int event, void *any)
{
  int status;
  unsigned long data;
  struct timespec sys_time;
  struct rtc_time rtc_raw;
  struct tm rtc_tm;
  time_t rtc_t;
  int error = 0;

  status = read(fd, &data, sizeof(data));

  if (status < 0) {
    /* This looks like a bad error : the file descriptor was indicating it was
     * ready to read but we couldn't read anything.  Give up. */
    LOG(LOGS_ERR, "Could not read flags %s : %s", CNF_GetRtcDevice(), strerror(errno));
    SCH_RemoveFileHandler(fd);
    switch_interrupts(0); /* Likely to raise error too, but just to be sure... */
    close(fd);
    fd = -1;
    return;
  }    

  if (skip_interrupts > 0) {
    /* Wait for the next interrupt, this one may be bogus */
    skip_interrupts--;
    return;
  }

  if ((data & RTC_UF) == RTC_UF) {
    /* Update interrupt detected */
    
    /* Read RTC time, sandwiched between two polls of the system clock
       so we can bound any error. */

    SCH_GetLastEventTime(&sys_time, NULL, NULL);

    status = ioctl(fd, RTC_RD_TIME, &rtc_raw);
    if (status < 0) {
      LOG(LOGS_ERR, "Could not read time from %s : %s", CNF_GetRtcDevice(), strerror(errno));
      error = 1;
      goto turn_off_interrupt;
    }

    /* Convert RTC time into a struct timespec */
    rtc_tm.tm_sec = rtc_raw.tm_sec;
    rtc_tm.tm_min = rtc_raw.tm_min;
    rtc_tm.tm_hour = rtc_raw.tm_hour;
    rtc_tm.tm_mday = rtc_raw.tm_mday;
    rtc_tm.tm_mon = rtc_raw.tm_mon;
    rtc_tm.tm_year = rtc_raw.tm_year;

    rtc_t = t_from_rtc(&rtc_tm);

    if (rtc_t == (time_t)(-1)) {
      error = 1;
      goto turn_off_interrupt;
    }      

    process_reading(rtc_t, &sys_time);

    if (n_samples < 4) {
      measurement_period = LOWEST_MEASUREMENT_PERIOD;
    } else if (n_samples < 6) {
      measurement_period = LOWEST_MEASUREMENT_PERIOD << 1;
    } else if (n_samples < 10) {
      measurement_period = LOWEST_MEASUREMENT_PERIOD << 2;
    } else if (n_samples < 14) {
      measurement_period = LOWEST_MEASUREMENT_PERIOD << 3;
    } else {
      measurement_period = LOWEST_MEASUREMENT_PERIOD << 4;
    }

  }

turn_off_interrupt:

  switch (operating_mode) {
    case OM_INITIAL:
      if (error) {
        DEBUG_LOG("Could not complete initial step due to errors");
        operating_mode = OM_NORMAL;
        (after_init_hook)(after_init_hook_arg);

        switch_interrupts(0);
    
        timeout_id = SCH_AddTimeoutByDelay((double) measurement_period, measurement_timeout, NULL);
      }

      break;

    case OM_AFTERTRIM:
      if (error) {
        DEBUG_LOG("Could not complete after trim relock due to errors");
        operating_mode = OM_NORMAL;

        switch_interrupts(0);
    
        timeout_id = SCH_AddTimeoutByDelay((double) measurement_period, measurement_timeout, NULL);
      }
      
      break;

    case OM_NORMAL:
      switch_interrupts(0);
    
      timeout_id = SCH_AddTimeoutByDelay((double) measurement_period, measurement_timeout, NULL);

      break;
    default:
      assert(0);
      break;
  }

}

/* ================================================== */

void
RTC_Linux_TimeInit(void (*after_hook)(void *), void *anything)
{
  after_init_hook = after_hook;
  after_init_hook_arg = anything;

  operating_mode = OM_INITIAL;
  timeout_id = 0;
  switch_interrupts(1);
}

/* ================================================== */

void
RTC_Linux_StartMeasurements(void)
{
  measurement_timeout(NULL);
}

/* ================================================== */

int
RTC_Linux_WriteParameters(void)
{
  int retval;

  if (fd < 0) {
    return RTC_ST_NODRV;
  }
  
  if (coefs_valid) {
    retval = write_coefs_to_file(1,coef_ref_time, coef_seconds_fast, coef_gain_rate);
  } else {
   /* Don't change the existing file, it may not be 100% valid but is our
      current best guess. */
    retval = RTC_ST_OK; /*write_coefs_to_file(0,0,0.0,0.0); */
  }

  return(retval);
}

/* ================================================== */
/* Try to set the system clock from the RTC, in the same manner as
   /sbin/hwclock -s would do.  We're not as picky about OS version
   etc in this case, since we have fewer requirements regarding the
   RTC behaviour than we do for the rest of the module. */

int
RTC_Linux_TimePreInit(time_t driftfile_time)
{
  int fd, status;
  struct rtc_time rtc_raw, rtc_raw_retry;
  struct tm rtc_tm;
  time_t rtc_t;
  double accumulated_error, sys_offset;
  struct timespec new_sys_time, old_sys_time;

  coefs_file_name = CNF_GetRtcFile();

  setup_config();
  read_coefs_from_file();

  fd = open(CNF_GetRtcDevice(), O_RDONLY);

  if (fd < 0) {
    return 0; /* Can't open it, and won't be able to later */
  }

  /* Retry reading the rtc until both read attempts give the same sec value.
     This way the race condition is prevented that the RTC has updated itself
     during the first read operation. */
  do {
    status = ioctl(fd, RTC_RD_TIME, &rtc_raw);
    if (status >= 0) {
      status = ioctl(fd, RTC_RD_TIME, &rtc_raw_retry);
    }
  } while (status >= 0 && rtc_raw.tm_sec != rtc_raw_retry.tm_sec);

  /* Read system clock */
  LCL_ReadCookedTime(&old_sys_time, NULL);

  close(fd);

  if (status >= 0) {
    /* Convert to seconds since 1970 */
    rtc_tm.tm_sec = rtc_raw.tm_sec;
    rtc_tm.tm_min = rtc_raw.tm_min;
    rtc_tm.tm_hour = rtc_raw.tm_hour;
    rtc_tm.tm_mday = rtc_raw.tm_mday;
    rtc_tm.tm_mon = rtc_raw.tm_mon;
    rtc_tm.tm_year = rtc_raw.tm_year;
    
    rtc_t = t_from_rtc(&rtc_tm);

    if (rtc_t != (time_t)(-1)) {

      /* Work out approximatation to correct time (to about the
         nearest second) */
      if (valid_coefs_from_file) {
        accumulated_error = file_ref_offset +
          (rtc_t - file_ref_time) * 1.0e-6 * file_rate_ppm;
      } else {
        accumulated_error = 0.0;
      }

      /* Correct time */

      new_sys_time.tv_sec = rtc_t;
      /* Average error in the RTC reading */
      new_sys_time.tv_nsec = 500000000;

      UTI_AddDoubleToTimespec(&new_sys_time, -accumulated_error, &new_sys_time);

      if (new_sys_time.tv_sec < driftfile_time) {
        LOG(LOGS_WARN, "RTC time before last driftfile modification (ignored)");
        return 0;
      }

      sys_offset = UTI_DiffTimespecsToDouble(&old_sys_time, &new_sys_time);

      /* Set system time only if the step is larger than 1 second */
      if (fabs(sys_offset) >= 1.0) {
        if (LCL_ApplyStepOffset(sys_offset))
          LOG(LOGS_INFO, "System time set from RTC");
      }
    } else {
      return 0;
    }
  } else {
    return 0;
  }

  return 1;
}

/* ================================================== */

int
RTC_Linux_GetReport(RPT_RTC_Report *report)
{
  report->ref_time.tv_sec = coef_ref_time;
  report->ref_time.tv_nsec = 0;
  report->n_samples = n_samples;
  report->n_runs = n_runs;
  if (n_samples > 1) {
    report->span_seconds = ((rtc_sec[n_samples-1] - rtc_sec[0]) +
                            (long)(rtc_trim[n_samples-1] - rtc_trim[0]));
  } else {
    report->span_seconds = 0;
  }
  report->rtc_seconds_fast = coef_seconds_fast;
  report->rtc_gain_rate_ppm = 1.0e6 * coef_gain_rate;
  return 1;
}

/* ================================================== */

int
RTC_Linux_Trim(void)
{
  struct timespec now;

  /* Remember the slope coefficient - we won't be able to determine a
     good one in a few seconds when we determine the new offset! */
  saved_coef_gain_rate = coef_gain_rate;

  if (fabs(coef_seconds_fast) > 1.0) {

    LOG(LOGS_INFO, "RTC wrong by %.3f seconds (step)",
        coef_seconds_fast);

    /* Do processing to set clock.  Let R be the value we set the
       RTC to, then in 500ms the RTC ticks (R+1) (see comments in
       arch/i386/kernel/time.c about the behaviour of the real time
       clock chip).  If S is the system time now, the error at the
       next RTC tick is given by E = (R+1) - (S+0.5).  Ideally we
       want |E| <= 0.5, which implies R <= S <= R+1, i.e. R is just
       the rounded down part of S, i.e. the seconds part. */

    LCL_ReadCookedTime(&now, NULL);
    
    set_rtc(now.tv_sec);

    /* All old samples will now look bogus under the new
           regime. */
    n_samples = 0;
    operating_mode = OM_AFTERTRIM;

    /* Estimate the offset in case writertc is called or chronyd
       is terminated during rapid sampling */
    coef_seconds_fast = -now.tv_nsec / 1.0e9 + 0.5;
    coef_ref_time = now.tv_sec;

    /* And start rapid sampling, interrupts on now */
    SCH_RemoveTimeout(timeout_id);
    timeout_id = 0;
    switch_interrupts(1);
  }

  return 1;
  
}
