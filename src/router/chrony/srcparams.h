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

  Header file defining parameters that can be set on a per source basis
  */

#ifndef GOT_SRCPARAMS_H
#define GOT_SRCPARAMS_H

#include "sources.h"

typedef enum {
  SRC_OFFLINE,
  SRC_ONLINE,
  SRC_MAYBE_ONLINE,
} SRC_Connectivity;

typedef struct {
  int minpoll;
  int maxpoll;
  SRC_Connectivity connectivity;
  int auto_offline;
  int presend_minpoll;
  int burst;
  int iburst;
  int min_stratum;
  int poll_target;
  int version;
  int max_sources;
  int min_samples;
  int max_samples;
  int filter_length;
  int interleaved;
  int sel_options;
  uint32_t authkey;
  double max_delay;
  double max_delay_ratio;
  double max_delay_dev_ratio;
  double min_delay;
  double asymmetry;
  double offset;
} SourceParameters;

#define SRC_DEFAULT_PORT 123
#define SRC_DEFAULT_MINPOLL 6
#define SRC_DEFAULT_MAXPOLL 10
#define SRC_DEFAULT_PRESEND_MINPOLL 100
#define SRC_DEFAULT_MAXDELAY 3.0
#define SRC_DEFAULT_MAXDELAYRATIO 0.0
#define SRC_DEFAULT_MAXDELAYDEVRATIO 10.0
#define SRC_DEFAULT_MINSTRATUM 0
#define SRC_DEFAULT_POLLTARGET 8
#define SRC_DEFAULT_MAXSOURCES 4
#define SRC_DEFAULT_MINSAMPLES (-1)
#define SRC_DEFAULT_MAXSAMPLES (-1)
#define SRC_DEFAULT_ASYMMETRY 1.0
#define INACTIVE_AUTHKEY 0

/* Flags for source selection */
#define SRC_SELECT_NOSELECT 0x1
#define SRC_SELECT_PREFER 0x2
#define SRC_SELECT_TRUST 0x4
#define SRC_SELECT_REQUIRE 0x8

#endif /* GOT_SRCPARAMS_H */
