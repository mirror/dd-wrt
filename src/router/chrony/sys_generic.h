/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2014
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

  Header file for generic driver
  */

#ifndef GOT_SYS_GENERIC_H
#define GOT_SYS_GENERIC_H

#include "localp.h"

/* Register a completed driver that implements offset functions on top of
   provided frequency functions */
extern void SYS_Generic_CompleteFreqDriver(double max_set_freq_ppm, double max_set_freq_delay,
                                           lcl_ReadFrequencyDriver sys_read_freq,
                                           lcl_SetFrequencyDriver sys_set_freq,
                                           lcl_ApplyStepOffsetDriver sys_apply_step_offset,
                                           double min_fastslew_offset, double max_fastslew_rate,
                                           lcl_AccrueOffsetDriver sys_accrue_offset,
                                           lcl_OffsetCorrectionDriver sys_get_offset_correction,
                                           lcl_SetLeapDriver sys_set_leap,
                                           lcl_SetSyncStatusDriver sys_set_sync_status);

extern void SYS_Generic_Finalise(void);

#endif  /* GOT_SYS_GENERIC_H */
