/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
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

  ======================================================================

  */

#ifndef _GOT_RTC_LINUX_H
#define _GOT_RTC_LINUX_H

#include "reports.h"

extern int RTC_Linux_Initialise(void);
extern void RTC_Linux_Finalise(void);
extern int RTC_Linux_TimePreInit(time_t driftile_time);
extern void RTC_Linux_TimeInit(void (*after_hook)(void *), void *anything);
extern void RTC_Linux_StartMeasurements(void);

/* 0=success, 1=no driver, 2=can't write file */
extern int RTC_Linux_WriteParameters(void);

extern int RTC_Linux_GetReport(RPT_RTC_Report *report);
extern int RTC_Linux_Trim(void);

extern void RTC_Linux_CycleLogFile(void);

#endif /* _GOT_RTC_LINUX_H */
