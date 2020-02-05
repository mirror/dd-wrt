/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) John G. Hasler  2009
 * Copyright (C) Miroslav Lichvar  2009-2012, 2014-2018
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

  This module is for POSIX compliant operating systems.

  */

#include "config.h"

#include "sysincl.h"

#include <sys/utsname.h>

#if defined(HAVE_PTHREAD_SETSCHEDPARAM)
#include <pthread.h>
#include <sched.h>
#endif

#if defined(HAVE_MLOCKALL)
#include <sys/mman.h>
#endif
#if defined(HAVE_SETRLIMIT_MEMLOCK)
#include <sys/resource.h>
#endif

#include "sys_posix.h"
#include "conf.h"
#include "local.h"
#include "logging.h"
#include "util.h"

/* ================================================== */

#if defined(HAVE_PTHREAD_SETSCHEDPARAM)
/* Install SCHED_FIFO real-time scheduler with specified priority */
void
SYS_Posix_SetScheduler(int priority)
{
  struct sched_param sched;
  int pmax, pmin;

  if (priority < 1 || priority > 99)
    LOG_FATAL("Bad scheduler priority: %d", priority);

  sched.sched_priority = priority;
  pmax = sched_get_priority_max(SCHED_FIFO);
  pmin = sched_get_priority_min(SCHED_FIFO);
  if (priority > pmax) {
    sched.sched_priority = pmax;
  } else if (priority < pmin) {
    sched.sched_priority = pmin;
  }

  if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sched) < 0) {
    LOG(LOGS_ERR, "pthread_setschedparam() failed");
  } else {
    DEBUG_LOG("Enabled SCHED_FIFO with priority %d", sched.sched_priority);
  }
}
#endif /* HAVE_PTHREAD_SETSCHEDPARAM  */

/* ================================================== */

#if defined(HAVE_MLOCKALL)
/* Lock the process into RAM so that it will never be swapped out */
void
SYS_Posix_MemLockAll(void)
{
#if defined(HAVE_SETRLIMIT_MEMLOCK)
  struct rlimit rlim;

  /* Ensure we can reserve as much as we need */
  rlim.rlim_max = RLIM_INFINITY;
  rlim.rlim_cur = RLIM_INFINITY;
  if (setrlimit(RLIMIT_MEMLOCK, &rlim) < 0) {
    LOG(LOGS_ERR, "setrlimit() failed");
    return;
  }
#endif

  if (mlockall(MCL_CURRENT|MCL_FUTURE) < 0) {
    LOG(LOGS_ERR, "mlockall() failed");
  } else {
    DEBUG_LOG("Successfully locked into RAM");
  }
}
#endif /* HAVE_MLOCKALL */
