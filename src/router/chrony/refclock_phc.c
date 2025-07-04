/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2013, 2017, 2023
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

  PTP hardware clock (PHC) refclock driver.

  */

#include "config.h"

#include "refclock.h"

#ifdef FEAT_PHC

#include "sysincl.h"

#include <sys/sysmacros.h>

#include "array.h"
#include "refclock.h"
#include "hwclock.h"
#include "local.h"
#include "logging.h"
#include "memory.h"
#include "util.h"
#include "sched.h"
#include "sys_linux.h"

struct phc_instance {
  int fd;
  int dev_index;
  int mode;
  int nocrossts;
  int extpps;
  int pin;
  int channel;
  struct timespec last_extts;
  HCL_Instance clock;
};

/* Array of RCL_Instance with enabled extpps */
static ARR_Instance extts_phcs = NULL;

static void read_ext_pulse(int sockfd, int event, void *anything);

static int phc_initialise(RCL_Instance instance)
{
  const char *options[] = {"nocrossts", "extpps", "pin", "channel", "clear", NULL};
  struct phc_instance *phc;
  int phc_fd, rising_edge;
  struct stat st;
  char *path, *s;

  RCL_CheckDriverOptions(instance, options);

  path = RCL_GetDriverParameter(instance);
 
  phc_fd = SYS_Linux_OpenPHC(path);
  if (phc_fd < 0)
    LOG_FATAL("Could not open PHC");

  phc = MallocNew(struct phc_instance);
  phc->fd = phc_fd;
  if (fstat(phc_fd, &st) < 0 || !S_ISCHR(st.st_mode))
    LOG_FATAL("Could not get PHC index");
  phc->dev_index = minor(st.st_rdev);
  phc->mode = 0;
  phc->nocrossts = RCL_GetDriverOption(instance, "nocrossts") ? 1 : 0;
  phc->extpps = RCL_GetDriverOption(instance, "extpps") ? 1 : 0;
  UTI_ZeroTimespec(&phc->last_extts);
  phc->clock = HCL_CreateInstance(0, 16, UTI_Log2ToDouble(RCL_GetDriverPoll(instance)),
                                  RCL_GetPrecision(instance));

  if (phc->extpps) {
    s = RCL_GetDriverOption(instance, "pin");
    phc->pin = s ? atoi(s) : 0;
    s = RCL_GetDriverOption(instance, "channel");
    phc->channel = s ? atoi(s) : 0;
    rising_edge = RCL_GetDriverOption(instance, "clear") ? 0 : 1;

    if (!SYS_Linux_SetPHCExtTimestamping(phc->fd, phc->pin, phc->channel,
                                         rising_edge, !rising_edge, 1))
      LOG_FATAL("Could not enable external PHC timestamping");

    SCH_AddFileHandler(phc->fd, SCH_FILE_INPUT, read_ext_pulse, instance);

    if (!extts_phcs)
      extts_phcs = ARR_CreateInstance(sizeof (RCL_Instance));
    ARR_AppendElement(extts_phcs, &instance);
  } else {
    phc->pin = phc->channel = 0;
  }

  RCL_SetDriverData(instance, phc);
  return 1;
}

static void phc_finalise(RCL_Instance instance)
{
  struct phc_instance *phc;
  unsigned int i;

  phc = (struct phc_instance *)RCL_GetDriverData(instance);

  if (phc->extpps) {
    SCH_RemoveFileHandler(phc->fd);
    SYS_Linux_SetPHCExtTimestamping(phc->fd, phc->pin, phc->channel, 0, 0, 0);

    for (i = 0; i < ARR_GetSize(extts_phcs); i++) {
      if ((*(RCL_Instance *)ARR_GetElement(extts_phcs, i)) == instance)
        ARR_RemoveElement(extts_phcs, i--);
    }
    if (ARR_GetSize(extts_phcs) == 0) {
      ARR_DestroyInstance(extts_phcs);
      extts_phcs = NULL;
    }
  }

  HCL_DestroyInstance(phc->clock);
  close(phc->fd);
  Free(phc);
}

static void process_ext_pulse(RCL_Instance instance, struct timespec *phc_ts)
{
  struct phc_instance *phc;
  struct timespec local_ts;
  double local_err;

  phc = RCL_GetDriverData(instance);

  if (UTI_CompareTimespecs(&phc->last_extts, phc_ts) == 0) {
    DEBUG_LOG("Ignoring duplicated PHC timestamp");
    return;
  }
  phc->last_extts = *phc_ts;

  RCL_UpdateReachability(instance);

  if (!HCL_CookTime(phc->clock, phc_ts, &local_ts, &local_err))
    return;

  RCL_AddCookedPulse(instance, &local_ts, 1.0e-9 * local_ts.tv_nsec, local_err,
                     UTI_DiffTimespecsToDouble(phc_ts, &local_ts));
}

static void read_ext_pulse(int fd, int event, void *anything)
{
  RCL_Instance instance;
  struct phc_instance *phc1, *phc2;
  struct timespec phc_ts;
  unsigned int i;
  int channel;

  if (!SYS_Linux_ReadPHCExtTimestamp(fd, &phc_ts, &channel))
    return;

  instance = anything;
  phc1 = RCL_GetDriverData(instance);

  /* Linux versions before 6.7 had one shared queue of timestamps for all
     descriptors of the same PHC.  Search for all refclocks that expect
     the timestamp. */

  for (i = 0; i < ARR_GetSize(extts_phcs); i++) {
    instance = *(RCL_Instance *)ARR_GetElement(extts_phcs, i);
    phc2 = RCL_GetDriverData(instance);
    if (!phc2->extpps || phc2->dev_index != phc1->dev_index || phc2->channel != channel)
      continue;
    process_ext_pulse(instance, &phc_ts);
  }
}

#define PHC_READINGS 25

static int phc_poll(RCL_Instance instance)
{
  struct timespec phc_ts, sys_ts, local_ts, readings[PHC_READINGS][3];
  struct phc_instance *phc;
  double phc_err, local_err;
  int n_readings;

  phc = (struct phc_instance *)RCL_GetDriverData(instance);

  n_readings = SYS_Linux_GetPHCReadings(phc->fd, phc->nocrossts, &phc->mode,
                                        PHC_READINGS, readings);
  if (n_readings < 1)
    return 0;

  if (!phc->extpps)
    RCL_UpdateReachability(instance);

  if (!HCL_ProcessReadings(phc->clock, n_readings, readings, &phc_ts, &sys_ts, &phc_err))
    return 0;

  LCL_CookTime(&sys_ts, &local_ts, &local_err);
  HCL_AccumulateSample(phc->clock, &phc_ts, &local_ts, phc_err + local_err);

  if (phc->extpps)
    return 0;

  DEBUG_LOG("PHC offset: %+.9f err: %.9f",
            UTI_DiffTimespecsToDouble(&phc_ts, &sys_ts), phc_err);

  return RCL_AddSample(instance, &sys_ts, &phc_ts, LEAP_Normal);
}

RefclockDriver RCL_PHC_driver = {
  phc_initialise,
  phc_finalise,
  phc_poll
};

#else

RefclockDriver RCL_PHC_driver = { NULL, NULL, NULL };

#endif
