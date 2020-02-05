/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2017
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

  Null clock driver for operation with no clock control.
  */

#include "config.h"

#include "sysincl.h"

#include "sys_null.h"

#include "local.h"
#include "localp.h"
#include "logging.h"
#include "util.h"

/* Current frequency offset of the system clock (in ppm) */
static double freq;

/* Offset of the system clock at the last update */
static double offset_register;

/* Time of the last update */
static struct timespec last_update;

/* Minimum interval between updates when frequency is constant */
#define MIN_UPDATE_INTERVAL 1000.0

/* ================================================== */

static void
update_offset(void)
{
  struct timespec now;
  double duration;

  LCL_ReadRawTime(&now);
  duration = UTI_DiffTimespecsToDouble(&now, &last_update);
  offset_register += 1.0e-6 * freq * duration;
  last_update = now;

  DEBUG_LOG("System clock offset=%e freq=%f", offset_register, freq);
}

/* ================================================== */

static double
read_frequency(void)
{
  return freq;
}

/* ================================================== */

static double
set_frequency(double freq_ppm)
{
  update_offset();
  freq = freq_ppm;

  return freq;
}

/* ================================================== */

static void
accrue_offset(double offset, double corr_rate)
{
  offset_register += offset;
}

/* ================================================== */

static int
apply_step_offset(double offset)
{
  return 0;
}

/* ================================================== */

static void
offset_convert(struct timespec *raw, double *corr, double *err)
{
  double duration;

  duration = UTI_DiffTimespecsToDouble(raw, &last_update);

  if (duration > MIN_UPDATE_INTERVAL) {
    update_offset();
    duration = 0.0;
  }

  *corr = -1.0e-6 * freq * duration - offset_register;

  if (err)
    *err = 0.0;
}

/* ================================================== */

void
SYS_Null_Initialise(void)
{
  offset_register = 0.0;
  LCL_ReadRawTime(&last_update);

  lcl_RegisterSystemDrivers(read_frequency, set_frequency, accrue_offset,
                            apply_step_offset, offset_convert, NULL, NULL);

  LOG(LOGS_INFO, "Disabled control of system clock");
}

/* ================================================== */

void
SYS_Null_Finalise(void)
{
}
