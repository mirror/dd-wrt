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

  PPSAPI refclock driver.

  */

#include "config.h"

#include "refclock.h"

#if FEAT_PPS

#if defined(HAVE_SYS_TIMEPPS_H)
#include <sys/timepps.h>
#elif defined(HAVE_TIMEPPS_H)
#include <timepps.h>
#endif

#include "logging.h"
#include "memory.h"
#include "util.h"

struct pps_instance {
  pps_handle_t handle;
  pps_seq_t last_seq;
  int edge_clear;
};

static int pps_initialise(RCL_Instance instance) {
  const char *options[] = {"clear", NULL};
  pps_handle_t handle;
  pps_params_t params;
  struct pps_instance *pps;
  int fd, edge_clear, mode;
  char *path;

  RCL_CheckDriverOptions(instance, options);

  path = RCL_GetDriverParameter(instance);
  edge_clear = RCL_GetDriverOption(instance, "clear") ? 1 : 0;

  fd = open(path, O_RDWR);
  if (fd < 0) {
    LOG_FATAL("Could not open %s : %s", path, strerror(errno));
    return 0;
  }

  UTI_FdSetCloexec(fd);

  if (time_pps_create(fd, &handle) < 0) {
    LOG_FATAL("time_pps_create() failed on %s : %s", path, strerror(errno));
    return 0;
  }

  if (time_pps_getcap(handle, &mode) < 0) {
    LOG_FATAL("time_pps_getcap() failed on %s : %s", path, strerror(errno));
    return 0;
  }

  if (time_pps_getparams(handle, &params) < 0) {
    LOG_FATAL("time_pps_getparams() failed on %s : %s", path, strerror(errno));
    return 0;
  }

  if (!edge_clear) {
    if (!(mode & PPS_CAPTUREASSERT)) {
      LOG_FATAL("CAPTUREASSERT not supported on %s", path);
      return 0;
    }
    params.mode |= PPS_CAPTUREASSERT;
    params.mode &= ~PPS_CAPTURECLEAR;
  } else {
    if (!(mode & PPS_CAPTURECLEAR)) {
      LOG_FATAL("CAPTURECLEAR not supported on %s", path);
      return 0;
    }
    params.mode |= PPS_CAPTURECLEAR;
    params.mode &= ~PPS_CAPTUREASSERT;
  }

  if (time_pps_setparams(handle, &params) < 0) {
    LOG_FATAL("time_pps_setparams() failed on %s : %s", path, strerror(errno));
    return 0;
  }


  pps = MallocNew(struct pps_instance);
  pps->handle = handle;
  pps->last_seq = 0;
  pps->edge_clear = edge_clear;

  RCL_SetDriverData(instance, pps);
  return 1;
}

static void pps_finalise(RCL_Instance instance)
{
  struct pps_instance *pps; 

  pps = (struct pps_instance *)RCL_GetDriverData(instance);
  time_pps_destroy(pps->handle);
  Free(pps);
}

static int pps_poll(RCL_Instance instance)
{
  struct pps_instance *pps; 
  struct timespec ts;
  pps_info_t pps_info;
  pps_seq_t seq;

  pps = (struct pps_instance *)RCL_GetDriverData(instance);

  ts.tv_sec = 0;
  ts.tv_nsec = 0;

  if (time_pps_fetch(pps->handle, PPS_TSFMT_TSPEC, &pps_info, &ts) < 0) {
    LOG(LOGS_ERR, "time_pps_fetch() failed : %s", strerror(errno));
    return 0;
  }

  if (!pps->edge_clear) {
    seq = pps_info.assert_sequence;
    ts = pps_info.assert_timestamp;
  } else {
    seq = pps_info.clear_sequence;
    ts = pps_info.clear_timestamp;
  }

  if (seq == pps->last_seq || UTI_IsZeroTimespec(&ts)) {
    DEBUG_LOG("PPS sample ignored seq=%lu ts=%s",
              (unsigned long)seq, UTI_TimespecToString(&ts));
    return 0;
  }

  pps->last_seq = seq;

  return RCL_AddPulse(instance, &ts, 1.0e-9 * ts.tv_nsec);
}

RefclockDriver RCL_PPS_driver = {
  pps_initialise,
  pps_finalise,
  pps_poll
};

#else

RefclockDriver RCL_PPS_driver = { NULL, NULL, NULL };

#endif
