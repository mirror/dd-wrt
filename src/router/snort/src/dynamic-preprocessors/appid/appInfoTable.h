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


#ifndef __APP_INFO_TABLE_H__
#define __APP_INFO_TABLE_H__

#include "appId.h"
#include "client_app_api.h"
#include "detector_api.h"
#include "service_api.h"
#include "fw_appid.h"
/*TBD comment the following preproc directive  */
#define DEBUG_APP_NAME

typedef struct _AppInfoTableEntry
{
    struct _AppInfoTableEntry *next;
    tAppId     appId;
    uint32_t   serviceId;
    uint32_t   clientId;
    uint32_t   payloadId;
    int16_t    snortId;
    unsigned   flags;
#ifdef DEBUG_APP_NAME
    char       *appName;
#endif
} AppInfoTableEntry;

void appInfoTableInit(const char *path);
AppInfoTableEntry* getAppInfoEntry(tAppId appId);
AppInfoTableEntry* createAppInfoEntry(const char *appName);
void appInfoTableFini(void);
tAppId appGetSnortIdFromAppId(tAppId appId);
void AppIdDumpStats(int exit_flag);
void appInfoTableDump(void);
void appInfoSetActive(tAppId appId, bool active);
#endif
