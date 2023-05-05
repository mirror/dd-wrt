/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

#ifndef SF_THRESHOLD
#define SF_THRESHOLD

#include "sfthd.h"
#include "ipv6_port.h"

typedef struct _ThresholdConfig
{
    int memcap;
    int enabled;
    ThresholdObjects *thd_objs;

} ThresholdConfig;

ThresholdConfig * ThresholdConfigNew(void);
void ThresholdConfigFree(ThresholdConfig *);
void sfthreshold_reset(void);
int sfthreshold_create(struct _SnortConfig *, ThresholdConfig *, THDX_STRUCT *);
int sfthreshold_test(unsigned int, unsigned int, sfaddr_t*, sfaddr_t*, long curtime);
void print_thresholding(ThresholdConfig*, unsigned shutdown);
void sfthreshold_reset_active(void);
void sfthreshold_free(void);

#endif
