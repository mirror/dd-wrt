/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2001
 * Copyright (C) J. Hannken-Illjes  2001
 * Copyright (C) Miroslav Lichvar  2015
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

  Driver file for the NetBSD and FreeBSD operating system.
  */

#include "config.h"

#include "sysincl.h"

#include "sys_netbsd.h"
#include "sys_timex.h"
#include "logging.h"
#include "privops.h"
#include "util.h"

/* Maximum frequency offset accepted by the kernel (in ppm) */
#define MAX_FREQ 500.0

/* Minimum assumed rate at which the kernel updates the clock frequency */
#define MIN_TICK_RATE 100

/* Interval between kernel updates of the adjtime() offset */
#define ADJTIME_UPDATE_INTERVAL 1.0

/* Maximum adjtime() slew rate (in ppm) */
#define MAX_ADJTIME_SLEWRATE 5000.0

/* Minimum offset adjtime() slews faster than MAX_FREQ */
#define MIN_FASTSLEW_OFFSET 1.0

/* ================================================== */

/* Positive offset means system clock is fast of true time, therefore
   slew backwards */

static void
accrue_offset(double offset, double corr_rate)
{
  struct timeval newadj, oldadj;
  double doldadj;

  UTI_DoubleToTimeval(-offset, &newadj);

  if (PRV_AdjustTime(&newadj, &oldadj) < 0)
    LOG_FATAL("adjtime() failed");

  /* Add the old remaining adjustment if not zero */
  doldadj = UTI_TimevalToDouble(&oldadj);
  if (doldadj != 0.0) {
    UTI_DoubleToTimeval(-offset + doldadj, &newadj);
    if (PRV_AdjustTime(&newadj, NULL) < 0)
      LOG_FATAL("adjtime() failed");
  }
}

/* ================================================== */

static void
get_offset_correction(struct timespec *raw,
                      double *corr, double *err)
{
  struct timeval remadj;
  double adjustment_remaining;
#ifdef MACOSX
  struct timeval tv = {0, 0};

  if (PRV_AdjustTime(&tv, &remadj) < 0)
    LOG_FATAL("adjtime() failed");

  if (PRV_AdjustTime(&remadj, NULL) < 0)
    LOG_FATAL("adjtime() failed");
#else
  if (PRV_AdjustTime(NULL, &remadj) < 0)
    LOG_FATAL("adjtime() failed");
#endif

  adjustment_remaining = UTI_TimevalToDouble(&remadj);

  *corr = adjustment_remaining;
  if (err) {
    if (*corr != 0.0)
      *err = 1.0e-6 * MAX_ADJTIME_SLEWRATE / ADJTIME_UPDATE_INTERVAL;
    else
      *err = 0.0;
  }
}

/* ================================================== */

void
SYS_NetBSD_Initialise(void)
{
  SYS_Timex_InitialiseWithFunctions(MAX_FREQ, 1.0 / MIN_TICK_RATE,
                                    NULL, NULL, NULL,
                                    MIN_FASTSLEW_OFFSET, MAX_ADJTIME_SLEWRATE,
                                    accrue_offset, get_offset_correction);
}

/* ================================================== */

void
SYS_NetBSD_Finalise(void)
{
  SYS_Timex_Finalise();
}

/* ================================================== */

#ifdef FEAT_PRIVDROP
void
SYS_NetBSD_DropRoot(uid_t uid, gid_t gid)
{
#ifdef NETBSD
  int fd;
#endif

  /* On NetBSD the helper is used only for socket binding, but on FreeBSD
     it's used also for setting and adjusting the system clock */
  PRV_StartHelper();

  UTI_DropRoot(uid, gid);

#ifdef NETBSD
  /* Check if we have write access to /dev/clockctl */
  fd = open("/dev/clockctl", O_WRONLY);
  if (fd < 0)
    LOG_FATAL("Can't write to /dev/clockctl");
  close(fd);
#endif
}
#endif
