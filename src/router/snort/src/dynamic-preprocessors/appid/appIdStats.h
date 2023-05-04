/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
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
*/


#ifndef  _FW_STATS2_H_
#define  _FW_STATS2_H_
#include <stdint.h>
#include <sys/types.h>
#include <flow.h>


void appIdStatsUpdate(tAppIdData* session);
void appIdStatsInit(char* appFileName, time_t statsPeriod, size_t rolloverSize, time_t rolloverPeriod);
void appIdStatsReinit(void);
void appIdStatsIdleFlush(void);
void appIdStatsFini(void);

#endif
