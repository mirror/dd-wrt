/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2018
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

  Header file for sample filter.

  */

#ifndef GOT_SAMPLEFILT_H
#define GOT_SAMPLEFILT_H

#include "ntp.h"

typedef struct SPF_Instance_Record *SPF_Instance;

extern SPF_Instance SPF_CreateInstance(int min_samples, int max_samples,
                                       double max_dispersion, double combine_ratio);
extern void SPF_DestroyInstance(SPF_Instance filter);

extern int SPF_AccumulateSample(SPF_Instance filter, NTP_Sample *sample);
extern int SPF_GetLastSample(SPF_Instance filter, NTP_Sample *sample);
extern int SPF_GetNumberOfSamples(SPF_Instance filter);
extern double SPF_GetAvgSampleDispersion(SPF_Instance filter);
extern void SPF_DropSamples(SPF_Instance filter);
extern int SPF_GetFilteredSample(SPF_Instance filter, NTP_Sample *sample);
extern void SPF_SlewSamples(SPF_Instance filter, struct timespec *when,
                            double dfreq, double doffset);
extern void SPF_AddDispersion(SPF_Instance filter, double dispersion);

#endif
