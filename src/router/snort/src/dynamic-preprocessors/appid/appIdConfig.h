/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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


#ifndef __APP_ID_CONFIG_H___
#define __APP_ID_CONFIG_H___

#include <stdint.h>
#include <limits.h>

struct  AppIdCommandConfig
{
    unsigned long app_id_memcap;
    int app_id_dump_ports;
    int app_id_debug;
    char app_stats_filename[PATH_MAX];
    unsigned long app_stats_period;
    unsigned long app_stats_rollover_size;
    unsigned long app_stats_rollover_time;
    char app_id_detector_path[PATH_MAX];
};

extern struct AppIdCommandConfig *appIdCommandConfig;
void appIdConfigParse(char *args);
void appIdConfigDump(void);
#endif
