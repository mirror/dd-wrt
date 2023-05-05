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


#ifndef __APP_INFO_TABLE_H__
#define __APP_INFO_TABLE_H__

#include "appId.h"
#include "client_app_api.h"
#include "detector_api.h"
#include "service_api.h"

#define APP_PRIORITY_DEFAULT 2
#define HTTP_TUNNEL_DETECT_RESTART 0
#define HTTP_TUNNEL_DETECT_RESTART_AND_RESET 1

typedef enum
{
    APPINFO_FLAG_SERVICE_ADDITIONAL = (1<<0),
    APPINFO_FLAG_SERVICE_UDP_REVERSED = (1<<1),
    APPINFO_FLAG_CLIENT_ADDITIONAL = (1<<2),
    APPINFO_FLAG_CLIENT_USER = (1<<3),
    APPINFO_FLAG_ACTIVE = (1<<4),
    APPINFO_FLAG_SSL_INSPECT =  (1<<5),
    APPINFO_FLAG_REFERRED =  (1<<6),
    APPINFO_FLAG_DEFER =  (1<<7),

    APPINFO_FLAG_IGNORE =  (1<<8),
    APPINFO_FLAG_SSL_SQUELCH =  (1<<9),
    APPINFO_FLAG_PERSISTENT =  (1<<10),
    APPINFO_FLAG_TP_CLIENT = (1<<11),
    APPINFO_FLAG_DEFER_PAYLOAD =  (1<<12),
    APPINFO_FLAG_SEARCH_ENGINE =  (1<<13),
    APPINFO_FLAG_SUPPORTED_SEARCH = (1<<14),

    APPINFO_FLAG_CLIENT_DETECTOR_CALLBACK = (1<<15),
    APPINFO_FLAG_SERVICE_DETECTOR_CALLBACK = (1<<16)
} tAppInfoFlags;

struct _AppInfoTableEntry
{
    struct _AppInfoTableEntry *next;
    tAppId     appId;
    uint32_t   serviceId;
    uint32_t   clientId;
    uint32_t   payloadId;
    int16_t    snortId;
    uint32_t   flags;
    tRNAClientAppModule *clntValidator;
    tRNAServiceElement *svrValidator;
    uint32_t  priority;
    char       *appName;
};
typedef struct _AppInfoTableEntry AppInfoTableEntry;

void appInfoTableInit(tAppidStaticConfig* appidSC, tAppIdConfig* pConfig);
void appInfoTableFini(tAppIdConfig *pConfig);
AppInfoTableEntry* appInfoEntryGet(tAppId appId, const tAppIdConfig *pConfig);
AppInfoTableEntry* appInfoEntryCreate(const char *appName, tAppIdConfig *pConfig);
tAppId appGetSnortIdFromAppId(tAppId appId);
void AppIdDumpStats(int exit_flag);
void appInfoTableDump(tAppIdConfig *pConfig);
void appInfoSetActive(tAppId appId, bool active);
const char * appGetAppName(int32_t appId);
int32_t appGetAppId(const char *appName);

static inline void appInfoEntryFlagSet (tAppId appId, unsigned flags, const tAppIdConfig *pConfig)
{
    AppInfoTableEntry* entry = appInfoEntryGet(appId, pConfig);
    if (entry)
        entry->flags |= flags;
}

static inline void appInfoEntryFlagClear (tAppId appId, unsigned flags, const tAppIdConfig *pConfig)
{
    AppInfoTableEntry* entry = appInfoEntryGet(appId, pConfig);
    if (entry)
        entry->flags &= (~flags);
}

static inline unsigned appInfoEntryFlagGet (tAppId app_id, unsigned flags, const tAppIdConfig *pConfig)
{
    AppInfoTableEntry* entry = appInfoEntryGet(app_id, pConfig);
    if (entry)
        return (entry->flags & flags);
    return 0;
}

static inline uint32_t appInfoEntryFlags (tAppId app_id, const tAppIdConfig *pConfig)
{
    AppInfoTableEntry* entry = appInfoEntryGet(app_id, pConfig);
    if (entry)
        return entry->flags;
    return 0;
}

static inline void appInfoEntryPrioritySet (tAppId appId, unsigned priority, const tAppIdConfig *pConfig)
{
    AppInfoTableEntry* entry = appInfoEntryGet(appId, pConfig);
    if (entry)
        entry->priority |= priority;
}

static inline unsigned appInfoEntryPriorityGet (tAppId app_id, const tAppIdConfig *pConfig)
{
    AppInfoTableEntry* entry = appInfoEntryGet(app_id, pConfig);
    if (entry)
        return (entry->priority);
    return 0;
}


#endif
