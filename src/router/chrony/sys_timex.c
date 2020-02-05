/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2009-2012, 2014-2015, 2017
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

  Driver for systems that implement the adjtimex()/ntp_adjtime() system call
  */

#include "config.h"

#include "sysincl.h"

#include "conf.h"
#include "privops.h"
#include "sys_generic.h"
#include "sys_timex.h"
#include "logging.h"

#ifdef PRIVOPS_ADJUSTTIMEX
#define NTP_ADJTIME PRV_AdjustTimex
#define NTP_ADJTIME_NAME "ntp_adjtime"
#else
#ifdef LINUX
#define NTP_ADJTIME adjtimex
#define NTP_ADJTIME_NAME "adjtimex"
#else
#define NTP_ADJTIME ntp_adjtime
#define NTP_ADJTIME_NAME "ntp_adjtime"
#endif
#endif

/* Maximum frequency offset accepted by the kernel (in ppm) */
#define MAX_FREQ 500.0

/* Frequency scale to convert from ppm to the timex freq */
#define FREQ_SCALE (double)(1 << 16)

/* Threshold for the timex maxerror when the kernel sets the UNSYNC flag */
#define MAX_SYNC_ERROR 16.0

/* Minimum assumed rate at which the kernel updates the clock frequency */
#define MIN_TICK_RATE 100

/* Saved timex status */
static int sys_status;

/* Saved TAI-UTC offset */
static int sys_tai_offset;

/* ================================================== */

static double
read_frequency(void)
{
  struct timex txc;

  txc.modes = 0;

  SYS_Timex_Adjust(&txc, 0);

  return txc.freq / -FREQ_SCALE;
}

/* ================================================== */

static double
set_frequency(double freq_ppm)
{
  struct timex txc;

  txc.modes = MOD_FREQUENCY;
  txc.freq = freq_ppm * -FREQ_SCALE;

  SYS_Timex_Adjust(&txc, 0);

  return txc.freq / -FREQ_SCALE;
}

/* ================================================== */

static void
set_leap(int leap, int tai_offset)
{
  struct timex txc;
  int applied, prev_status;

  txc.modes = 0;
  applied = SYS_Timex_Adjust(&txc, 0) == TIME_WAIT;

  prev_status = sys_status;
  sys_status &= ~(STA_INS | STA_DEL);

  if (leap > 0)
    sys_status |= STA_INS;
  else if (leap < 0)
    sys_status |= STA_DEL;

  txc.modes = MOD_STATUS;
  txc.status = sys_status;

#ifdef MOD_TAI
  if (tai_offset) {
    txc.modes |= MOD_TAI;
    txc.constant = tai_offset;

    if (applied && !(sys_status & (STA_INS | STA_DEL)))
      sys_tai_offset += prev_status & STA_INS ? 1 : -1;

    if (sys_tai_offset != tai_offset) {
      sys_tai_offset = tai_offset;
      LOG(LOGS_INFO, "System clock TAI offset set to %d seconds", tai_offset);
    }
  }
#endif

  SYS_Timex_Adjust(&txc, 0);

  if (prev_status != sys_status) {
    LOG(LOGS_INFO, "System clock status %s leap second",
        leap ? (leap > 0 ? "set to insert" : "set to delete") :
        (applied ? "reset after" : "set to not insert/delete"));
  }
}

/* ================================================== */

static void
set_sync_status(int synchronised, double est_error, double max_error)
{
  struct timex txc;

  if (synchronised) {
    if (est_error > MAX_SYNC_ERROR)
      est_error = MAX_SYNC_ERROR;
    if (max_error >= MAX_SYNC_ERROR) {
      max_error = MAX_SYNC_ERROR;
      synchronised = 0;
    }
  } else {
    est_error = max_error = MAX_SYNC_ERROR;
  }

#ifdef LINUX
  /* On Linux clear the UNSYNC flag only if rtcsync is enabled */
  if (!CNF_GetRtcSync())
    synchronised = 0;
#endif

  if (synchronised)
    sys_status &= ~STA_UNSYNC;
  else
    sys_status |= STA_UNSYNC;

  txc.modes = MOD_STATUS | MOD_ESTERROR | MOD_MAXERROR;
  txc.status = sys_status;
  txc.esterror = est_error * 1.0e6;
  txc.maxerror = max_error * 1.0e6;

  if (SYS_Timex_Adjust(&txc, 1) < 0)
    ;
}

/* ================================================== */

static void
initialise_timex(void)
{
  struct timex txc;

  sys_status = STA_UNSYNC;
  sys_tai_offset = 0;

  /* Reset PLL offset */
  txc.modes = MOD_OFFSET | MOD_STATUS;
  txc.status = STA_PLL | sys_status;
  txc.offset = 0;
  SYS_Timex_Adjust(&txc, 0);

  /* Turn PLL off */
  txc.modes = MOD_STATUS;
  txc.status = sys_status;
  SYS_Timex_Adjust(&txc, 0);
}

/* ================================================== */

void
SYS_Timex_Initialise(void)
{
  SYS_Timex_InitialiseWithFunctions(MAX_FREQ, 1.0 / MIN_TICK_RATE, NULL, NULL, NULL,
                                    0.0, 0.0, NULL, NULL);
}

/* ================================================== */

void
SYS_Timex_InitialiseWithFunctions(double max_set_freq_ppm, double max_set_freq_delay,
                                  lcl_ReadFrequencyDriver sys_read_freq,
                                  lcl_SetFrequencyDriver sys_set_freq,
                                  lcl_ApplyStepOffsetDriver sys_apply_step_offset,
                                  double min_fastslew_offset, double max_fastslew_rate,
                                  lcl_AccrueOffsetDriver sys_accrue_offset,
                                  lcl_OffsetCorrectionDriver sys_get_offset_correction)
{
  initialise_timex();

  SYS_Generic_CompleteFreqDriver(max_set_freq_ppm, max_set_freq_delay,
                                 sys_read_freq ? sys_read_freq : read_frequency,
                                 sys_set_freq ? sys_set_freq : set_frequency,
                                 sys_apply_step_offset,
                                 min_fastslew_offset, max_fastslew_rate,
                                 sys_accrue_offset, sys_get_offset_correction,
                                 set_leap, set_sync_status);
}

/* ================================================== */

void
SYS_Timex_Finalise(void)
{
  SYS_Generic_Finalise();
}

/* ================================================== */

int
SYS_Timex_Adjust(struct timex *txc, int ignore_error)
{
  int state;

#ifdef SOLARIS
  /* The kernel seems to check the constant even when it's not being set */
  if (!(txc->modes & MOD_TIMECONST))
    txc->constant = 10;
#endif

  state = NTP_ADJTIME(txc);

  if (state < 0) {
    if (!ignore_error)
      LOG_FATAL(NTP_ADJTIME_NAME"(0x%x) failed : %s", txc->modes, strerror(errno));
    else
      DEBUG_LOG(NTP_ADJTIME_NAME"(0x%x) failed : %s", txc->modes, strerror(errno));
  }

  return state;
}
