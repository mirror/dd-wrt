/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2009
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

  Header file for refclocks.

  */

#ifndef GOT_REFCLOCK_H
#define GOT_REFCLOCK_H

#include "srcparams.h"
#include "sources.h"

typedef struct {
  char *driver_name;
  char *driver_parameter;
  int driver_poll;
  int poll;
  int filter_length;
  int pps_forced;
  int pps_rate;
  int min_samples;
  int max_samples;
  int sel_options;
  int max_lock_age;
  int stratum;
  int tai;
  uint32_t ref_id;
  uint32_t lock_ref_id;
  double offset;
  double delay;
  double precision;
  double max_dispersion;
  double pulse_width;
} RefclockParameters;

typedef struct RCL_Instance_Record *RCL_Instance;

typedef struct {
  int (*init)(RCL_Instance instance);
  void (*fini)(RCL_Instance instance);
  int (*poll)(RCL_Instance instance);
} RefclockDriver;

extern void RCL_Initialise(void);
extern void RCL_Finalise(void);
extern int RCL_AddRefclock(RefclockParameters *params);
extern void RCL_StartRefclocks(void);
extern void RCL_ReportSource(RPT_SourceReport *report, struct timespec *now);

/* functions used by drivers */
extern void RCL_SetDriverData(RCL_Instance instance, void *data);
extern void *RCL_GetDriverData(RCL_Instance instance);
extern char *RCL_GetDriverParameter(RCL_Instance instance);
extern void RCL_CheckDriverOptions(RCL_Instance instance, const char **options);
extern char *RCL_GetDriverOption(RCL_Instance instance, char *name);
extern int RCL_AddSample(RCL_Instance instance, struct timespec *sample_time, double offset, int leap);
extern int RCL_AddPulse(RCL_Instance instance, struct timespec *pulse_time, double second);
extern int RCL_AddCookedPulse(RCL_Instance instance, struct timespec *cooked_time,
                              double second, double dispersion, double raw_correction);
extern double RCL_GetPrecision(RCL_Instance instance);
extern int RCL_GetDriverPoll(RCL_Instance instance);

#endif
