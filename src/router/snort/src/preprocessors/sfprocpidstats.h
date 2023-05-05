/*
** $Id$
**
** sfprocpidstats.h
**
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Dan Roelker <droelker@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
**
**  DESCRIPTION
**    This file gets the correct CPU usage for SMP linux machines.
**
*/
#ifndef __SFPROCPIDSTATS__
#define __SFPROCPIDSTATS__

#include "config.h"

#ifdef LINUX_SMP

typedef struct _CPUSTAT {
    
    double user;
    double sys;
    double total;
    double idle;

} CPUSTAT;

typedef struct _SFPROCPIDSTATS {
    
    CPUSTAT *SysCPUs;

    int iCPUs;
    
} SFPROCPIDSTATS;

int sfInitProcPidStats(SFPROCPIDSTATS *sfProcPidStats);
int sfProcessProcPidStats(SFPROCPIDSTATS *sfProcPidStats);
void FreeProcPidStats(SFPROCPIDSTATS *sfProcPidStats);

#endif

#endif
