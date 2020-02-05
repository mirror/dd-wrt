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

  =======================================================================

  Header file for manual time input module.

  */

#ifndef GOT_MANUAL_H
#define GOT_MANUAL_H

#include "sysincl.h"
#include "reports.h"

extern void MNL_Initialise(void);
extern void MNL_Finalise(void);
extern int MNL_AcceptTimestamp(struct timespec *ts, double *reg_offset, double *dfreq_ppm, double *new_afreq_ppm);

extern void MNL_Enable(void);
extern void MNL_Disable(void);
extern void MNL_Reset(void);
extern int MNL_IsEnabled(void);

extern void MNL_ReportSamples(RPT_ManualSamplesReport *report, int max, int *n);
extern int MNL_DeleteSample(int index);

#endif /* GOT_MANUAL_H */
