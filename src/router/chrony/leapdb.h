/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Patrick Oppenlander 2024
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

  This module provides leap second information.

  */

#ifndef GOT_LEAPDB_H
#define GOT_LEAPDB_H

#include "ntp.h"

extern void LDB_Initialise(void);
extern NTP_Leap LDB_GetLeap(time_t when, int *tai_offset);
extern void LDB_Finalise(void);

#endif /* GOT_LEAPDB_H */
