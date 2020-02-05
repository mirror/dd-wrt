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

static void read_sample(int sockfd, int event, void *anything)
{
  struct sock_sample sample;
  struct timespec ts;
  RCL_Instance instance;
  int s;

  instance = (RCL_Instance)anything;

  s = recv(sockfd, &sample, sizeof (sample), 0);

  if (s < 0) {
    DEBUG_LOG("Could not read SOCK sample : %s", strerror(errno));
    return;
  }

  if (s != sizeof (sample)) {
    DEBUG_LOG("Unexpected length of SOCK sample : %d != %ld",
              s, (long)sizeof (sample));
    return;
  }

  if (sample.magic != SOCK_MAGIC) {
    DEBUG_LOG("Unexpected magic number in SOCK sample : %x != %x",
              (unsigned int)sample.magic, (unsigned int)SOCK_MAGIC);
    return;
  }

  UTI_TimevalToTimespec(&sample.tv, &ts);
  UTI_NormaliseTimespec(&ts);

  if (sample.pulse) {
    RCL_AddPulse(instance, &ts, sample.offset);
  } else {
    RCL_AddSample(instance, &ts, sample.offset, sample.leap);
  }
}

static int sock_initialise(RCL_Instance instance)
{
  struct sockaddr_un s;
  int sockfd;
  char *path;

  RCL_CheckDriverOptions(instance, NULL);

  path = RCL_GetDriverParameter(instance);
 
  s.sun_family = AF_UNIX;
  if (snprintf(s.sun_path, sizeof (s.sun_path), "%s", path) >= sizeof (s.sun_path)) {
    LOG_FATAL("Path %s too long", path);
    return 0;
  }

  sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    LOG_FATAL("socket() failed");
    return 0;
  }

  UTI_FdSetCloexec(sockfd);

  unlink(path);
  if (bind(sockfd, (struct sockaddr *)&s, sizeof (s)) < 0) {
    LOG_FATAL("bind(%s) failed : %s", path, strerror(errno));
    return 0;
  }

  RCL_SetDriverData(instance, (void *)(long)sockfd);
  SCH_AddFileHandler(sockfd, SCH_FILE_INPUT, read_sample, instance);
  return 1;
}

static void sock_finalise(RCL_Instance instance)
{
  int sockfd;

  sockfd = (long)RCL_GetDriverData(instance);
  SCH_RemoveFileHandler(sockfd);
  close(sockfd);
}

RefclockDriver RCL_SOCK_driver = {
  sock_initialise,
  sock_finalise,
  NULL
};
