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

  SHM refclock driver.

  */

#include "config.h"

#include "sysincl.h"

#include "refclock.h"
#include "logging.h"
#include "util.h"

#define SHMKEY 0x4e545030

struct shmTime {
  int    mode; /* 0 - if valid set
                *       use values, 
                *       clear valid
                * 1 - if valid set 
                *       if count before and after read of values is equal,
                *         use values 
                *       clear valid
                */
  volatile int count;
  time_t clockTimeStampSec;
  int    clockTimeStampUSec;
  time_t receiveTimeStampSec;
  int    receiveTimeStampUSec;
  int    leap;
  int    precision;
  int    nsamples;
  volatile int valid;
  int    clockTimeStampNSec;
  int    receiveTimeStampNSec;
  int    dummy[8]; 
};

static int shm_initialise(RCL_Instance instance) {
  const char *options[] = {"perm", NULL};
  int id, param, perm;
  char *s;
  struct shmTime *shm;

  RCL_CheckDriverOptions(instance, options);

  param = atoi(RCL_GetDriverParameter(instance));
  s = RCL_GetDriverOption(instance, "perm");
  perm = s ? strtol(s, NULL, 8) & 0777 : 0600;

  id = shmget(SHMKEY + param, sizeof (struct shmTime), IPC_CREAT | perm);
  if (id == -1) {
    LOG_FATAL("shmget() failed : %s", strerror(errno));
    return 0;
  }
   
  shm = (struct shmTime *)shmat(id, 0, 0);
  if ((long)shm == -1) {
    LOG_FATAL("shmat() failed : %s", strerror(errno));
    return 0;
  }

  RCL_SetDriverData(instance, shm);
  return 1;
}

static void shm_finalise(RCL_Instance instance)
{
  shmdt(RCL_GetDriverData(instance));
}

static int shm_poll(RCL_Instance instance)
{
  struct timespec receive_ts, clock_ts;
  struct shmTime t, *shm;
  double offset;

  shm = (struct shmTime *)RCL_GetDriverData(instance);

  t = *shm;
  
  if ((t.mode == 1 && t.count != shm->count) ||
    !(t.mode == 0 || t.mode == 1) || !t.valid) {
    DEBUG_LOG("SHM sample ignored mode=%d count=%d valid=%d",
        t.mode, t.count, t.valid);
    return 0;
  }

  shm->valid = 0;

  receive_ts.tv_sec = t.receiveTimeStampSec;
  clock_ts.tv_sec = t.clockTimeStampSec;

  if (t.clockTimeStampNSec / 1000 == t.clockTimeStampUSec &&
      t.receiveTimeStampNSec / 1000 == t.receiveTimeStampUSec) {
    receive_ts.tv_nsec = t.receiveTimeStampNSec;
    clock_ts.tv_nsec = t.clockTimeStampNSec;
  } else {
    receive_ts.tv_nsec = 1000 * t.receiveTimeStampUSec;
    clock_ts.tv_nsec = 1000 * t.clockTimeStampUSec;
  }

  UTI_NormaliseTimespec(&clock_ts);
  UTI_NormaliseTimespec(&receive_ts);
  offset = UTI_DiffTimespecsToDouble(&clock_ts, &receive_ts);

  return RCL_AddSample(instance, &receive_ts, offset, t.leap);
}

RefclockDriver RCL_SHM_driver = {
  shm_initialise,
  shm_finalise,
  shm_poll
};
