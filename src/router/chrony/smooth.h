/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
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

  This module implements time smoothing.
  */

#ifndef GOT_SMOOTH_H
#define GOT_SMOOTH_H

#include "reports.h"

extern void SMT_Initialise(void);

extern void SMT_Finalise(void);

extern int SMT_IsEnabled(void);

extern double SMT_GetOffset(struct timespec *now);

extern void SMT_Activate(struct timespec *now);

extern void SMT_Reset(struct timespec *now);

extern void SMT_Leap(struct timespec *now, int leap);

extern int SMT_GetSmoothingReport(RPT_SmoothingReport *report, struct timespec *now);

#endif
