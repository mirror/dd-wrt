/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2016
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

  Header for tracking of hardware clocks */

#ifndef GOT_HWCLOCK_H
#define GOT_HWCLOCK_H

typedef struct HCL_Instance_Record *HCL_Instance;

/* Create a new HW clock instance */
extern HCL_Instance HCL_CreateInstance(int min_samples, int max_samples,
                                       double min_separation);

/* Destroy a HW clock instance */
extern void HCL_DestroyInstance(HCL_Instance clock);

/* Check if a new sample should be accumulated at this time */
extern int HCL_NeedsNewSample(HCL_Instance clock, struct timespec *now);

/* Accumulate a new sample */
extern void HCL_AccumulateSample(HCL_Instance clock, struct timespec *hw_ts,
                                 struct timespec *local_ts, double err);

/* Convert raw hardware time to cooked local time */
extern int HCL_CookTime(HCL_Instance clock, struct timespec *raw, struct timespec *cooked,
                        double *err);

#endif
