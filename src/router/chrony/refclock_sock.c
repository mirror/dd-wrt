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

  Unix domain socket refclock driver.

  */

#include "config.h"

#include "sysincl.h"

#include "refclock.h"
#include "logging.h"
#include "util.h"
#include "sched.h"
#include "socket.h"

#define SOCK_MAGIC 0x534f434b

struct sock_sample {
  /* Time of the measurement (system time) */
  struct timeval tv;

  /* Offset between the true time and the system time (in seconds) */
  double offset;

  /* Non-zero if the sample is from a PPS signal, i.e. another source
     is needed to obtain seconds */
  int pulse;

  /* 0 - normal, 1 - insert leap second, 2 - delete leap second */
  int leap;

  /* Padding, ignored */
  int _pad;

  /* Protocol identifier (0x534f434b) */
  int magic;
};

/* On 32-bit glibc-based systems enable conversion between timevals using
   32-bit and 64-bit time_t to support SOCK clients compiled with different
   time_t size than chrony */
#ifdef __GLIBC_PREREQ
#if __GLIBC_PREREQ(2, 34) && __TIMESIZE == 32
#define CONVERT_TIMEVAL 1
#if defined(_TIME_BITS) && _TIME_BITS == 64
typedef int32_t alt_time_t;
typedef int32_t alt_suseconds_t;
#else
typedef int64_t alt_time_t;
typedef int64_t alt_suseconds_t;
#endif
struct alt_timeval {
  alt_time_t tv_sec;
  alt_suseconds_t tv_usec;
};
#endif
#endif

static void read_sample(int sockfd, int event, void *anything)
{
  char buf[sizeof (struct sock_sample) + 16];
  struct timespec sys_ts, ref_ts;
  struct sock_sample sample;
  RCL_Instance instance;
  int s;

  instance = (RCL_Instance)anything;

  s = recv(sockfd, buf, sizeof (buf), 0);

  if (s < 0) {
    DEBUG_LOG("Could not read SOCK sample : %s", strerror(errno));
    return;
  }

  if (s == sizeof (sample)) {
    memcpy(&sample, buf, sizeof (sample));
#ifdef CONVERT_TIMEVAL
  } else if (s == sizeof (sample) - sizeof (struct timeval) + sizeof (struct alt_timeval)) {
    struct alt_timeval atv;
    memcpy(&atv, buf, sizeof (atv));
#ifndef HAVE_LONG_TIME_T
    if (atv.tv_sec > INT32_MAX || atv.tv_sec < INT32_MIN ||
        atv.tv_usec > INT32_MAX || atv.tv_usec < INT32_MIN) {
      DEBUG_LOG("Could not convert 64-bit timeval");
      return;
    }
#endif
    sample.tv.tv_sec = atv.tv_sec;
    sample.tv.tv_usec = atv.tv_usec;
    DEBUG_LOG("Converted %d-bit timeval", 8 * (int)sizeof (alt_time_t));
    memcpy((char *)&sample + sizeof (struct timeval), buf + sizeof (struct alt_timeval),
           sizeof (sample) - sizeof (struct timeval));
#endif
  } else {
    DEBUG_LOG("Unexpected length of SOCK sample : %d != %ld",
              s, (long)sizeof (sample));
    return;
  }

  if (sample.magic != SOCK_MAGIC) {
    DEBUG_LOG("Unexpected magic number in SOCK sample : %x != %x",
              (unsigned int)sample.magic, (unsigned int)SOCK_MAGIC);
    return;
  }

  UTI_TimevalToTimespec(&sample.tv, &sys_ts);
  UTI_NormaliseTimespec(&sys_ts);

  RCL_UpdateReachability(instance);

  if (!UTI_IsTimeOffsetSane(&sys_ts, sample.offset))
    return;

  UTI_AddDoubleToTimespec(&sys_ts, sample.offset, &ref_ts);

  if (sample.pulse) {
    RCL_AddPulse(instance, &sys_ts, sample.offset);
  } else {
    RCL_AddSample(instance, &sys_ts, &ref_ts, sample.leap);
  }
}

static int sock_initialise(RCL_Instance instance)
{
  int sockfd;
  char *path;

  RCL_CheckDriverOptions(instance, NULL);

  path = RCL_GetDriverParameter(instance);
 
  sockfd = SCK_OpenUnixDatagramSocket(NULL, path, 0);
  if (sockfd < 0)
    LOG_FATAL("Could not open socket %s", path);

  RCL_SetDriverData(instance, (void *)(long)sockfd);
  SCH_AddFileHandler(sockfd, SCH_FILE_INPUT, read_sample, instance);
  return 1;
}

static void sock_finalise(RCL_Instance instance)
{
  int sockfd;

  sockfd = (long)RCL_GetDriverData(instance);
  SCH_RemoveFileHandler(sockfd);
  SCK_RemoveSocket(sockfd);
  SCK_CloseSocket(sockfd);
}

RefclockDriver RCL_SOCK_driver = {
  sock_initialise,
  sock_finalise,
  NULL
};
