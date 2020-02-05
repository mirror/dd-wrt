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

  Header file for a driver based on the adjtimex()/ntp_adjtime() function
  */

#ifndef GOT_SYS_TIMEX_H
#define GOT_SYS_TIMEX_H

#include "localp.h"

extern void SYS_Timex_Initialise(void);

/* Initialise with some driver functions replaced with special versions */
extern void SYS_Timex_InitialiseWithFunctions(double max_set_freq_ppm, double max_set_freq_delay,
                                              lcl_ReadFrequencyDriver sys_read_freq,
                                              lcl_SetFrequencyDriver sys_set_freq,
                                              lcl_ApplyStepOffsetDriver sys_apply_step_offset,
                                              double min_fastslew_offset, double max_fastslew_rate,
                                              lcl_AccrueOffsetDriver sys_accrue_offset,
                                              lcl_OffsetCorrectionDriver sys_get_offset_correction);

extern void SYS_Timex_Finalise(void);

/* Wrapper for adjtimex()/ntp_adjtime() */
extern int SYS_Timex_Adjust(struct timex *txc, int ignore_error);

#endif  /* GOT_SYS_GENERIC_H */
