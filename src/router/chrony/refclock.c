/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2009-2011, 2013-2014, 2016-2019
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

  Routines implementing reference clocks.

  */

#include "config.h"

#include "array.h"
#include "refclock.h"
#include "reference.h"
#include "conf.h"
#include "local.h"
#include "memory.h"
#include "util.h"
#include "sources.h"
#include "logging.h"
#include "regress.h"
#include "samplefilt.h"
#include "sched.h"

/* list of refclock drivers */
extern RefclockDriver RCL_SHM_driver;
extern RefclockDriver RCL_SOCK_driver;
extern RefclockDriver RCL_PPS_driver;
extern RefclockDriver RCL_PHC_driver;

struct FilterSample {
  double offset;
  double dispersion;
  struct timespec sample_time;
};

struct MedianFilter {
  int length;
  int index;
  int used;
  int last;
  int avg_var_n;
  double avg_var;
  double max_var;
  struct FilterSample *samples;
  int *selected;
  double *x_data;
  double *y_data;
  double *w_data;
};

struct RCL_Instance_Record {
  RefclockDriver *driver;
  void *data;
  char *driver_parameter;
  int driver_parameter_length;
  int driver_poll;
  int driver_polled;
  int poll;
  int leap_status;
  int pps_forced;
  int pps_rate;
  int pps_active;
  int max_lock_age;
  int stratum;
  int tai;
  uint32_t ref_id;
  uint32_t lock_ref;
  double offset;
  double delay;
  double precision;
  double pulse_width;
  SPF_Instance filter;
  SCH_TimeoutID timeout_id;
  SRC_Instance source;
};

/* Array of pointers to RCL_Instance_Record */
static ARR_Instance refclocks;

static LOG_FileID logfileid;

static int valid_sample_time(RCL_Instance instance, struct timespec *sample_time);
static int pps_stratum(RCL_Instance instance, struct timespec *ts);
static void poll_timeout(void *arg);
static void slew_samples(struct timespec *raw, struct timespec *cooked, double dfreq,
             double doffset, LCL_ChangeType change_type, void *anything);
static void add_dispersion(double dispersion, void *anything);
static void log_sample(RCL_Instance instance, struct timespec *sample_time, int filtered, int pulse, double raw_offset, double cooked_offset, double dispersion);

static RCL_Instance
get_refclock(unsigned int index)
{
  return *(RCL_Instance *)ARR_GetElement(refclocks, index);
}

void
RCL_Initialise(void)
{
  refclocks = ARR_CreateInstance(sizeof (RCL_Instance));

  CNF_AddRefclocks();

  if (ARR_GetSize(refclocks) > 0) {
    LCL_AddParameterChangeHandler(slew_samples, NULL);
    LCL_AddDispersionNotifyHandler(add_dispersion, NULL);
  }

  logfileid = CNF_GetLogRefclocks() ? LOG_FileOpen("refclocks",
      "   Date (UTC) Time         Refid  DP L P  Raw offset   Cooked offset      Disp.")
    : -1;
}

void
RCL_Finalise(void)
{
  unsigned int i;

  for (i = 0; i < ARR_GetSize(refclocks); i++) {
    RCL_Instance inst = get_refclock(i);

    if (inst->driver->fini)
      inst->driver->fini(inst);

    SPF_DestroyInstance(inst->filter);
    Free(inst->driver_parameter);
    SRC_DestroyInstance(inst->source);
    Free(inst);
  }

  if (ARR_GetSize(refclocks) > 0) {
    LCL_RemoveParameterChangeHandler(slew_samples, NULL);
    LCL_RemoveDispersionNotifyHandler(add_dispersion, NULL);
  }

  ARR_DestroyInstance(refclocks);
}

int
RCL_AddRefclock(RefclockParameters *params)
{
  RCL_Instance inst;

  inst = MallocNew(struct RCL_Instance_Record);
  *(RCL_Instance *)ARR_GetNewElement(refclocks) = inst;

  if (strcmp(params->driver_name, "SHM") == 0) {
    inst->driver = &RCL_SHM_driver;
  } else if (strcmp(params->driver_name, "SOCK") == 0) {
    inst->driver = &RCL_SOCK_driver;
  } else if (strcmp(params->driver_name, "PPS") == 0) {
    inst->driver = &RCL_PPS_driver;
  } else if (strcmp(params->driver_name, "PHC") == 0) {
    inst->driver = &RCL_PHC_driver;
  } else {
    LOG_FATAL("unknown refclock driver %s", params->driver_name);
  }

  if (!inst->driver->init && !inst->driver->poll)
    LOG_FATAL("refclock driver %s is not compiled in", params->driver_name);

  if (params->tai && !CNF_GetLeapSecTimezone())
    LOG_FATAL("refclock tai option requires leapsectz");

  inst->data = NULL;
  inst->driver_parameter = params->driver_parameter;
  inst->driver_parameter_length = 0;
  inst->driver_poll = params->driver_poll;
  inst->poll = params->poll;
  inst->driver_polled = 0;
  inst->leap_status = LEAP_Normal;
  inst->pps_forced = params->pps_forced;
  inst->pps_rate = params->pps_rate;
  inst->pps_active = 0;
  inst->max_lock_age = params->max_lock_age;
  inst->stratum = params->stratum;
  inst->tai = params->tai;
  inst->lock_ref = params->lock_ref_id;
  inst->offset = params->offset;
  inst->delay = params->delay;
  inst->precision = LCL_GetSysPrecisionAsQuantum();
  inst->precision = MAX(inst->precision, params->precision);
  inst->pulse_width = params->pulse_width;
  inst->timeout_id = -1;
  inst->source = NULL;

  if (inst->driver_parameter) {
    int i;

    inst->driver_parameter_length = strlen(inst->driver_parameter);
    for (i = 0; i < inst->driver_parameter_length; i++)
      if (inst->driver_parameter[i] == ':')
        inst->driver_parameter[i] = '\0';
  }

  if (inst->pps_rate < 1)
    inst->pps_rate = 1;

  if (params->ref_id)
    inst->ref_id = params->ref_id;
  else {
    unsigned char ref[5] = { 0, 0, 0, 0, 0 };
    unsigned int index = ARR_GetSize(refclocks) - 1;

    snprintf((char *)ref, sizeof (ref), "%3.3s", params->driver_name);
    ref[3] = index % 10 + '0';
    if (index >= 10)
      ref[2] = (index / 10) % 10 + '0';

    inst->ref_id = (uint32_t)ref[0] << 24 | ref[1] << 16 | ref[2] << 8 | ref[3];
  }

  if (inst->driver->poll) {
    int max_samples;

    if (inst->driver_poll > inst->poll)
      inst->driver_poll = inst->poll;

    max_samples = 1 << (inst->poll - inst->driver_poll);
    if (max_samples < params->filter_length) {
      if (max_samples < 4) {
        LOG(LOGS_WARN, "Setting filter length for %s to %d",
            UTI_RefidToString(inst->ref_id), max_samples);
      }
      params->filter_length = max_samples;
    }
  }

  if (inst->driver->init && !inst->driver->init(inst))
    LOG_FATAL("refclock %s initialisation failed", params->driver_name);

  /* Require the filter to have at least 4 samples to produce a filtered
     sample, or be full for shorter lengths, and combine 60% of samples
     closest to the median */
  inst->filter = SPF_CreateInstance(MIN(params->filter_length, 4), params->filter_length,
                                    params->max_dispersion, 0.6);

  inst->source = SRC_CreateNewInstance(inst->ref_id, SRC_REFCLOCK, params->sel_options, NULL,
                                       params->min_samples, params->max_samples, 0.0, 0.0);

  DEBUG_LOG("refclock %s refid=%s poll=%d dpoll=%d filter=%d",
      params->driver_name, UTI_RefidToString(inst->ref_id),
      inst->poll, inst->driver_poll, params->filter_length);

  Free(params->driver_name);

  return 1;
}

void
RCL_StartRefclocks(void)
{
  unsigned int i, j, n;

  n = ARR_GetSize(refclocks);

  for (i = 0; i < n; i++) {
    RCL_Instance inst = get_refclock(i);

    SRC_SetActive(inst->source);
    inst->timeout_id = SCH_AddTimeoutByDelay(0.0, poll_timeout, (void *)inst);

    if (inst->lock_ref) {
      /* Replace lock refid with index to refclocks */
      for (j = 0; j < n && get_refclock(j)->ref_id != inst->lock_ref; j++)
        ;
      inst->lock_ref = j < n ? j : -1;
    } else
      inst->lock_ref = -1;
  }
}

void
RCL_ReportSource(RPT_SourceReport *report, struct timespec *now)
{
  unsigned int i;
  uint32_t ref_id;

  assert(report->ip_addr.family == IPADDR_INET4);
  ref_id = report->ip_addr.addr.in4;

  for (i = 0; i < ARR_GetSize(refclocks); i++) {
    RCL_Instance inst = get_refclock(i);
    if (inst->ref_id == ref_id) {
      report->poll = inst->poll;
      report->mode = RPT_LOCAL_REFERENCE;
      break;
    }
  }
}

void
RCL_SetDriverData(RCL_Instance instance, void *data)
{
  instance->data = data;
}

void *
RCL_GetDriverData(RCL_Instance instance)
{
  return instance->data;
}

char *
RCL_GetDriverParameter(RCL_Instance instance)
{
  return instance->driver_parameter;
}

static char *
get_next_driver_option(RCL_Instance instance, char *option)
{
  if (option == NULL)
    option = instance->driver_parameter;

  option += strlen(option) + 1;

  if (option >= instance->driver_parameter + instance->driver_parameter_length)
    return NULL;

  return option;
}

void
RCL_CheckDriverOptions(RCL_Instance instance, const char **options)
{
  char *option;
  int i, len;

  for (option = get_next_driver_option(instance, NULL);
       option;
       option = get_next_driver_option(instance, option)) {
    for (i = 0; options && options[i]; i++) {
      len = strlen(options[i]);
      if (!strncmp(options[i], option, len) &&
          (option[len] == '=' || option[len] == '\0'))
        break;
    }

    if (!options || !options[i])
      LOG_FATAL("Invalid refclock driver option %s", option);
  }
}

char *
RCL_GetDriverOption(RCL_Instance instance, char *name)
{
  char *option;
  int len;

  len = strlen(name);

  for (option = get_next_driver_option(instance, NULL);
       option;
       option = get_next_driver_option(instance, option)) {
    if (!strncmp(name, option, len)) {
      if (option[len] == '=')
        return option + len + 1;
      if (option[len] == '\0')
        return option + len;
    }
  }

  return NULL;
}

static int
convert_tai_offset(struct timespec *sample_time, double *offset)
{
  struct timespec tai_ts, utc_ts;
  int tai_offset;

  /* Get approximate TAI-UTC offset for the reference time in TAI */
  UTI_AddDoubleToTimespec(sample_time, *offset, &tai_ts);
  tai_offset = REF_GetTaiOffset(&tai_ts);

  /* Get TAI-UTC offset for the reference time in UTC +/- 1 second */
  UTI_AddDoubleToTimespec(&tai_ts, -tai_offset, &utc_ts);
  tai_offset = REF_GetTaiOffset(&utc_ts);

  if (!tai_offset)
    return 0;

  *offset -= tai_offset;

  return 1;
}

static int
accumulate_sample(RCL_Instance instance, struct timespec *sample_time, double offset, double dispersion)
{
  NTP_Sample sample;

  sample.time = *sample_time;
  sample.offset = offset;
  sample.peer_delay = instance->delay;
  sample.root_delay = instance->delay;
  sample.peer_dispersion = dispersion;
  sample.root_dispersion = dispersion;
  sample.leap = instance->leap_status;

  /* Handle special case when PPS is used with the local reference */
  if (instance->pps_active && instance->lock_ref == -1)
    sample.stratum = pps_stratum(instance, &sample.time);
  else
    sample.stratum = instance->stratum;

  return SPF_AccumulateSample(instance->filter, &sample);
}

int
RCL_AddSample(RCL_Instance instance, struct timespec *sample_time, double offset, int leap)
{
  double correction, dispersion;
  struct timespec cooked_time;

  if (instance->pps_forced)
    return RCL_AddPulse(instance, sample_time, -offset);

  LCL_GetOffsetCorrection(sample_time, &correction, &dispersion);
  UTI_AddDoubleToTimespec(sample_time, correction, &cooked_time);
  dispersion += instance->precision;

  /* Make sure the timestamp and offset provided by the driver are sane */
  if (!UTI_IsTimeOffsetSane(sample_time, offset) ||
      !valid_sample_time(instance, &cooked_time))
    return 0;

  switch (leap) {
    case LEAP_Normal:
    case LEAP_InsertSecond:
    case LEAP_DeleteSecond:
      instance->leap_status = leap;
      break;
    default:
      DEBUG_LOG("refclock sample ignored bad leap %d", leap);
      return 0;
  }

  if (instance->tai && !convert_tai_offset(sample_time, &offset)) {
    DEBUG_LOG("refclock sample ignored unknown TAI offset");
    return 0;
  }

  if (!accumulate_sample(instance, &cooked_time,
                         offset - correction + instance->offset, dispersion))
    return 0;

  instance->pps_active = 0;

  log_sample(instance, &cooked_time, 0, 0, offset, offset - correction + instance->offset, dispersion);

  /* for logging purposes */
  if (!instance->driver->poll)
    instance->driver_polled++;

  return 1;
}

int
RCL_AddPulse(RCL_Instance instance, struct timespec *pulse_time, double second)
{
  double correction, dispersion;
  struct timespec cooked_time;

  LCL_GetOffsetCorrection(pulse_time, &correction, &dispersion);
  UTI_AddDoubleToTimespec(pulse_time, correction, &cooked_time);
  second += correction;

  if (!UTI_IsTimeOffsetSane(pulse_time, 0.0))
    return 0;

  return RCL_AddCookedPulse(instance, &cooked_time, second, dispersion, correction);
}

static int
check_pulse_edge(RCL_Instance instance, double offset, double distance)
{
  double max_error;

  if (instance->pulse_width <= 0.0)
    return 1;

  max_error = 1.0 / instance->pps_rate - instance->pulse_width;
  max_error = MIN(instance->pulse_width, max_error);
  max_error *= 0.5;

  if (fabs(offset) > max_error || distance > max_error) {
      DEBUG_LOG("refclock pulse ignored offset=%.9f distance=%.9f max_error=%.9f",
                offset, distance, max_error);
      return 0;
  }

  return 1;
}

int
RCL_AddCookedPulse(RCL_Instance instance, struct timespec *cooked_time,
                   double second, double dispersion, double raw_correction)
{
  double offset;
  int rate;
  NTP_Leap leap;

  if (!UTI_IsTimeOffsetSane(cooked_time, second) ||
      !valid_sample_time(instance, cooked_time))
    return 0;

  leap = LEAP_Normal;
  dispersion += instance->precision;
  rate = instance->pps_rate;

  offset = -second + instance->offset;

  /* Adjust the offset to [-0.5/rate, 0.5/rate) interval */
  offset -= (long)(offset * rate) / (double)rate;
  if (offset < -0.5 / rate)
    offset += 1.0 / rate;
  else if (offset >= 0.5 / rate)
    offset -= 1.0 / rate;

  if (instance->lock_ref != -1) {
    RCL_Instance lock_refclock;
    NTP_Sample ref_sample;
    double sample_diff, shift;

    lock_refclock = get_refclock(instance->lock_ref);

    if (!SPF_GetLastSample(lock_refclock->filter, &ref_sample)) {
      DEBUG_LOG("refclock pulse ignored no ref sample");
      return 0;
    }

    ref_sample.root_dispersion += SPF_GetAvgSampleDispersion(lock_refclock->filter);

    sample_diff = UTI_DiffTimespecsToDouble(cooked_time, &ref_sample.time);
    if (fabs(sample_diff) >= (double)instance->max_lock_age / rate) {
      DEBUG_LOG("refclock pulse ignored samplediff=%.9f",
          sample_diff);
      return 0;
    }

    /* Align the offset to the reference sample */
    if ((ref_sample.offset - offset) >= 0.0)
      shift = (long)((ref_sample.offset - offset) * rate + 0.5) / (double)rate;
    else
      shift = (long)((ref_sample.offset - offset) * rate - 0.5) / (double)rate;

    offset += shift;

    if (fabs(ref_sample.offset - offset) +
        ref_sample.root_dispersion + dispersion >= 0.2 / rate) {
      DEBUG_LOG("refclock pulse ignored offdiff=%.9f refdisp=%.9f disp=%.9f",
                ref_sample.offset - offset, ref_sample.root_dispersion, dispersion);
      return 0;
    }

    if (!check_pulse_edge(instance, ref_sample.offset - offset, 0.0))
      return 0;

    leap = lock_refclock->leap_status;

    DEBUG_LOG("refclock pulse offset=%.9f offdiff=%.9f samplediff=%.9f",
              offset, ref_sample.offset - offset, sample_diff);
  } else {
    struct timespec ref_time;
    int is_synchronised, stratum;
    double root_delay, root_dispersion, distance;
    uint32_t ref_id;

    /* Ignore the pulse if we are not well synchronized and the local
       reference is not active */

    REF_GetReferenceParams(cooked_time, &is_synchronised, &leap, &stratum,
        &ref_id, &ref_time, &root_delay, &root_dispersion);
    distance = fabs(root_delay) / 2 + root_dispersion;

    if (leap == LEAP_Unsynchronised || distance >= 0.5 / rate) {
      DEBUG_LOG("refclock pulse ignored offset=%.9f sync=%d dist=%.9f",
                offset, leap != LEAP_Unsynchronised, distance);
      /* Drop also all stored samples */
      SPF_DropSamples(instance->filter);
      return 0;
    }

    if (!check_pulse_edge(instance, offset, distance))
      return 0;
  }

  if (!accumulate_sample(instance, cooked_time, offset, dispersion))
    return 0;

  instance->leap_status = leap;
  instance->pps_active = 1;

  log_sample(instance, cooked_time, 0, 1, offset + raw_correction - instance->offset,
             offset, dispersion);

  /* for logging purposes */
  if (!instance->driver->poll)
    instance->driver_polled++;

  return 1;
}

double
RCL_GetPrecision(RCL_Instance instance)
{
  return instance->precision;
}

int
RCL_GetDriverPoll(RCL_Instance instance)
{
  return instance->driver_poll;
}

static int
valid_sample_time(RCL_Instance instance, struct timespec *sample_time)
{
  struct timespec now;
  double diff;

  LCL_ReadCookedTime(&now, NULL);
  diff = UTI_DiffTimespecsToDouble(&now, sample_time);

  if (diff < 0.0 || diff > UTI_Log2ToDouble(instance->poll + 1)) {
    DEBUG_LOG("%s refclock sample time %s not valid age=%.6f",
              UTI_RefidToString(instance->ref_id),
              UTI_TimespecToString(sample_time), diff);
    return 0;
  }

  return 1;
}

static int
pps_stratum(RCL_Instance instance, struct timespec *ts)
{
  struct timespec ref_time;
  int is_synchronised, stratum;
  unsigned int i;
  double root_delay, root_dispersion;
  NTP_Leap leap;
  uint32_t ref_id;
  RCL_Instance refclock;

  REF_GetReferenceParams(ts, &is_synchronised, &leap, &stratum,
      &ref_id, &ref_time, &root_delay, &root_dispersion);

  /* Don't change our stratum if the local reference is active
     or this is the current source */
  if (ref_id == instance->ref_id ||
      (!is_synchronised && leap != LEAP_Unsynchronised))
    return stratum - 1;

  /* Or the current source is another PPS refclock */ 
  for (i = 0; i < ARR_GetSize(refclocks); i++) {
    refclock = get_refclock(i);
    if (refclock->ref_id == ref_id &&
        refclock->pps_active && refclock->lock_ref == -1)
      return stratum - 1;
  }

  return 0;
}

static void
poll_timeout(void *arg)
{
  NTP_Sample sample;
  int poll;

  RCL_Instance inst = (RCL_Instance)arg;

  poll = inst->poll;

  if (inst->driver->poll) {
    poll = inst->driver_poll;
    inst->driver->poll(inst);
    inst->driver_polled++;
  }
  
  if (!(inst->driver->poll && inst->driver_polled < (1 << (inst->poll - inst->driver_poll)))) {
    inst->driver_polled = 0;

    if (SPF_GetFilteredSample(inst->filter, &sample)) {
      SRC_UpdateReachability(inst->source, 1);
      SRC_AccumulateSample(inst->source, &sample);
      SRC_SelectSource(inst->source);

      log_sample(inst, &sample.time, 1, 0, 0.0, sample.offset, sample.peer_dispersion);
    } else {
      SRC_UpdateReachability(inst->source, 0);
    }
  }

  inst->timeout_id = SCH_AddTimeoutByDelay(UTI_Log2ToDouble(poll), poll_timeout, arg);
}

static void
slew_samples(struct timespec *raw, struct timespec *cooked, double dfreq,
             double doffset, LCL_ChangeType change_type, void *anything)
{
  unsigned int i;

  for (i = 0; i < ARR_GetSize(refclocks); i++) {
    if (change_type == LCL_ChangeUnknownStep)
      SPF_DropSamples(get_refclock(i)->filter);
    else
      SPF_SlewSamples(get_refclock(i)->filter, cooked, dfreq, doffset);
  }
}

static void
add_dispersion(double dispersion, void *anything)
{
  unsigned int i;

  for (i = 0; i < ARR_GetSize(refclocks); i++)
    SPF_AddDispersion(get_refclock(i)->filter, dispersion);
}

static void
log_sample(RCL_Instance instance, struct timespec *sample_time, int filtered, int pulse, double raw_offset, double cooked_offset, double dispersion)
{
  char sync_stats[4] = {'N', '+', '-', '?'};

  if (logfileid == -1)
    return;

  if (!filtered) {
    LOG_FileWrite(logfileid, "%s.%06d %-5s %3d %1c %1d %13.6e %13.6e %10.3e",
      UTI_TimeToLogForm(sample_time->tv_sec),
      (int)sample_time->tv_nsec / 1000,
      UTI_RefidToString(instance->ref_id),
      instance->driver_polled,
      sync_stats[instance->leap_status],
      pulse,
      raw_offset,
      cooked_offset,
      dispersion);
  } else {
    LOG_FileWrite(logfileid, "%s.%06d %-5s   - %1c -       -       %13.6e %10.3e",
      UTI_TimeToLogForm(sample_time->tv_sec),
      (int)sample_time->tv_nsec / 1000,
      UTI_RefidToString(instance->ref_id),
      sync_stats[instance->leap_status],
      cooked_offset,
      dispersion);
  }
}
