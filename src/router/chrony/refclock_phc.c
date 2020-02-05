/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2013, 2017
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
  int mode;
  int nocrossts;
  int extpps;
  int pin;
  int channel;
  HCL_Instance clock;
};

static void read_ext_pulse(int sockfd, int event, void *anything);

static int phc_initialise(RCL_Instance instance)
{
  const char *options[] = {"nocrossts", "extpps", "pin", "channel", "clear", NULL};
  struct phc_instance *phc;
  int phc_fd, rising_edge;
  char *path, *s;

  RCL_CheckDriverOptions(instance, options);

  path = RCL_GetDriverParameter(instance);
 
  phc_fd = SYS_Linux_OpenPHC(path, 0);
  if (phc_fd < 0) {
    LOG_FATAL("Could not open PHC");
    return 0;
  }

  phc = MallocNew(struct phc_instance);
  phc->fd = phc_fd;
  phc->mode = 0;
  phc->nocrossts = RCL_GetDriverOption(instance, "nocrossts") ? 1 : 0;
  phc->extpps = RCL_GetDriverOption(instance, "extpps") ? 1 : 0;

  if (phc->extpps) {
    s = RCL_GetDriverOption(instance, "pin");
    phc->pin = s ? atoi(s) : 0;
    s = RCL_GetDriverOption(instance, "channel");
    phc->channel = s ? atoi(s) : 0;
    rising_edge = RCL_GetDriverOption(instance, "clear") ? 0 : 1;
    phc->clock = HCL_CreateInstance(0, 16, UTI_Log2ToDouble(RCL_GetDriverPoll(instance)));

    if (!SYS_Linux_SetPHCExtTimestamping(phc->fd, phc->pin, phc->channel,
                                         rising_edge, !rising_edge, 1))
      LOG_FATAL("Could not enable external PHC timestamping");

    SCH_AddFileHandler(phc->fd, SCH_FILE_INPUT, read_ext_pulse, instance);
  } else {
    phc->pin = phc->channel = 0;
    phc->clock = NULL;
  }

  RCL_SetDriverData(instance, phc);
  return 1;
}

static void phc_finalise(RCL_Instance instance)
{
  struct phc_instance *phc;

  phc = (struct phc_instance *)RCL_GetDriverData(instance);

  if (phc->extpps) {
    SCH_RemoveFileHandler(phc->fd);
    SYS_Linux_SetPHCExtTimestamping(phc->fd, phc->pin, phc->channel, 0, 0, 0);
    HCL_DestroyInstance(phc->clock);
  }

  close(phc->fd);
  Free(phc);
}

static void read_ext_pulse(int fd, int event, void *anything)
{
  RCL_Instance instance;
  struct phc_instance *phc;
  struct timespec phc_ts, local_ts;
  double local_err;
  int channel;

  instance = anything;
  phc = RCL_GetDriverData(instance);

  if (!SYS_Linux_ReadPHCExtTimestamp(phc->fd, &phc_ts, &channel))
    return;

  if (channel != phc->channel) {
    DEBUG_LOG("Unexpected extts channel %d\n", channel);
    return;
  }

  if (!HCL_CookTime(phc->clock, &phc_ts, &local_ts, &local_err))
    return;

  RCL_AddCookedPulse(instance, &local_ts, 1.0e-9 * local_ts.tv_nsec, local_err,
                     UTI_DiffTimespecsToDouble(&phc_ts, &local_ts));
}

static int phc_poll(RCL_Instance instance)
{
  struct phc_instance *phc;
  struct timespec phc_ts, sys_ts, local_ts;
  double offset, phc_err, local_err;

  phc = (struct phc_instance *)RCL_GetDriverData(instance);

  if (!SYS_Linux_GetPHCSample(phc->fd, phc->nocrossts, RCL_GetPrecision(instance),
                              &phc->mode, &phc_ts, &sys_ts, &phc_err))
    return 0;

  if (phc->extpps) {
    LCL_CookTime(&sys_ts, &local_ts, &local_err);
    HCL_AccumulateSample(phc->clock, &phc_ts, &local_ts, phc_err + local_err);
    return 0;
  }

  offset = UTI_DiffTimespecsToDouble(&phc_ts, &sys_ts);

  DEBUG_LOG("PHC offset: %+.9f err: %.9f", offset, phc_err);

  return RCL_AddSample(instance, &sys_ts, offset, LEAP_Normal);
}

RefclockDriver RCL_PHC_driver = {
  phc_initialise,
  phc_finalise,
  phc_poll
};

#else

RefclockDriver RCL_PHC_driver = { NULL, NULL, NULL };

#endif
