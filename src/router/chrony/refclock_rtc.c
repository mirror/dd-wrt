/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Uwe Kleine-König, Pengutronix  2021
 * Copyright (C) Ahmad Fatoum, Pengutronix  2024
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

  RTC refclock driver.

  */

#include "config.h"

#include "refclock.h"

#ifdef FEAT_RTC

#include <linux/rtc.h>

#include "conf.h"
#include "local.h"

#include "logging.h"
#include "memory.h"
#include "sched.h"
#include "util.h"
#include "rtc_linux.h"

struct refrtc_instance {
  int fd;
  int polling;
  int utc;
};

static int refrtc_add_sample(RCL_Instance instance, struct timespec *now,
                             time_t rtc_sec, long rtc_nsec)
{
  struct timespec rtc_ts;
  int status;

  rtc_ts.tv_sec = rtc_sec;
  rtc_ts.tv_nsec = rtc_nsec;

  RCL_UpdateReachability(instance);

  status = RCL_AddSample(instance, now, &rtc_ts, LEAP_Normal);

  return status;
}

static void refrtc_read_after_uie(int rtcfd, int event, void *data)
{
  RCL_Instance instance = (RCL_Instance)data;
  struct refrtc_instance *rtc = RCL_GetDriverData(instance);
  struct timespec now;
  time_t rtc_sec;
  int status;

  status = RTC_Linux_CheckInterrupt(rtcfd);
  if (status < 0) {
    SCH_RemoveFileHandler(rtcfd);
    RTC_Linux_SwitchInterrupt(rtcfd, 0); /* Likely to raise error too, but just to be sure... */
    close(rtcfd);
    rtc->fd = -1;
    return;
  } else if (status == 0) {
    /* Wait for the next interrupt, this one may be bogus */
    return;
  }

  rtc_sec = RTC_Linux_ReadTimeAfterInterrupt(rtcfd, rtc->utc, NULL, &now);
  if (rtc_sec == (time_t)-1)
    return;

  refrtc_add_sample(instance, &now, rtc_sec, 0);
}

static int refrtc_initialise(RCL_Instance instance)
{
  const char *options[] = {"utc", NULL};
  struct refrtc_instance *rtc;
  int rtcfd, status;
  const char *path;

  RCL_CheckDriverOptions(instance, options);

  if (CNF_GetRtcSync() || CNF_GetRtcFile())
    LOG_FATAL("RTC refclock cannot be used together with rtcsync or rtcfile");

  path = RCL_GetDriverParameter(instance);

  rtcfd = open(path, O_RDONLY);
  if (rtcfd < 0)
    LOG_FATAL("Could not open RTC device %s : %s", path, strerror(errno));

  /* Close on exec */
  UTI_FdSetCloexec(rtcfd);

  rtc = MallocNew(struct refrtc_instance);
  rtc->fd = rtcfd;
  rtc->utc = RCL_GetDriverOption(instance, "utc") ? 1 : 0;

  RCL_SetDriverData(instance, rtc);

  /* Try to enable update interrupts */
  status = RTC_Linux_SwitchInterrupt(rtcfd, 1);
  if (status) {
    SCH_AddFileHandler(rtcfd, SCH_FILE_INPUT, refrtc_read_after_uie, instance);
    rtc->polling = 0;
  } else {
    LOG(LOGS_INFO, "Falling back to polling for %s", path);
    rtc->polling = 1;
  }

  return 1;
}

static void refrtc_finalise(RCL_Instance instance)
{
  struct refrtc_instance *rtc;

  rtc = RCL_GetDriverData(instance);

  if (rtc->fd >= 0) {
    if (!rtc->polling) {
      SCH_RemoveFileHandler(rtc->fd);
      RTC_Linux_SwitchInterrupt(rtc->fd, 0);
    }

    close(rtc->fd);
  }

  Free(rtc);
}

static int refrtc_poll(RCL_Instance instance)
{
  struct refrtc_instance *rtc;
  struct timespec now;
  time_t rtc_sec;

  rtc = RCL_GetDriverData(instance);

  if (!rtc->polling)
    return 0;

  rtc_sec = RTC_Linux_ReadTimeNow(rtc->fd, rtc->utc, NULL, &now);
  if (rtc_sec == (time_t)-1)
    return 0;

  /* As the rtc has a resolution of 1s, only add half a second */
  return refrtc_add_sample(instance, &now, rtc_sec, 500000000);
}

RefclockDriver RCL_RTC_driver = {
  refrtc_initialise,
  refrtc_finalise,
  refrtc_poll
};

#else

RefclockDriver RCL_RTC_driver = { NULL, NULL, NULL };

#endif
