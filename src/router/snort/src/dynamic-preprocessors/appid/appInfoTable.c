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


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>
#include <syslog.h>
#include <limits.h>
#include "appInfoTable.h"
#include "Unified2_common.h"

#define APP_MAPPING_FILE "appMapping.data"
#define APP_CONFIG_FILE "appid.conf"
#define USR_CONFIG_FILE "userappid.conf"

#define MAX_TABLE_LINE_LEN      1024
#define CONF_SEPARATORS         "\t\n\r"

typedef struct DynamicArray
{ 
    void **table;
    size_t  usedCount;
    size_t  allocatedCount;
    size_t  stepSize;
} tDynamicArray;

static AppInfoTableEntry *AppInfoList;
static AppInfoTableEntry *AppInfoTable[SF_APPID_MAX];
static AppInfoTableEntry *AppInfoTableByService[SF_APPID_MAX];
static AppInfoTableEntry *AppInfoTableByClient[SF_APPID_MAX];
static AppInfoTableEntry *AppInfoTableByPayload[SF_APPID_MAX];
static void appIdConfLoad (const char *path);
static tAppId appGetAppIdFromSnortId(int16_t snortId);

tDynamicArray AppInfoTableDyn;

AppInfoTableEntry* getAppInfoEntry(tAppId appId)
{
    if (appId > APP_ID_NONE && appId < SF_APPID_MAX)
        return AppInfoTable[appId];
    if (appId >= SF_APPID_DYNAMIC_MIN && appId < (SF_APPID_DYNAMIC_MIN + AppInfoTableDyn.usedCount))
        return AppInfoTableDyn.table[appId - SF_APPID_DYNAMIC_MIN];
    return NULL;
}

void appInfoTableFini(void)
{
    AppInfoTableEntry *entry;

    while ((entry = AppInfoList))
    {
        AppInfoList = entry->next;
#ifdef DEBUG_APP_NAME
        free(entry->appName);
#endif
        free(entry);
    }

    unsigned i;
    for (i = 0; i < AppInfoTableDyn.usedCount; i++)
    {
        entry = AppInfoTableDyn.table[i];
#ifdef DEBUG_APP_NAME
        free(entry->appName);
#endif
        free(entry);
    }

    free(AppInfoTableDyn.table);
    memset(&AppInfoTableDyn, 0, sizeof(AppInfoTableDyn));
}


AppInfoTableEntry* createAppInfoEntry(const char *appName)
{
    void **tmp;
    int16_t snortId;
    tAppId appId;
    AppInfoTableEntry *entry;
   
    if (!appName || strlen(appName) >= MAX_EVENT_APPNAME_LEN)
    {
        _dpd.errMsg("Appname invalid\n", appName);
        return NULL;
    }

    snortId = _dpd.findProtocolReference(appName);
    if (snortId == SFTARGET_UNKNOWN_PROTOCOL)
    {
        if (AppInfoTableDyn.usedCount == AppInfoTableDyn.allocatedCount)
        {
            tmp = realloc(AppInfoTableDyn.table, (AppInfoTableDyn.allocatedCount + AppInfoTableDyn.stepSize)*sizeof(*tmp));
            if (!tmp)
            {
                return NULL;
            }
            AppInfoTableDyn.table = tmp;
            AppInfoTableDyn.allocatedCount += AppInfoTableDyn.stepSize;
        }

        if ((entry = calloc(1, sizeof(*entry))))
        {
            entry->snortId = _dpd.addProtocolReference(appName);
            entry->appId = SF_APPID_DYNAMIC_MIN + (AppInfoTableDyn.usedCount);
            entry->serviceId = entry->appId;
            entry->clientId = entry->appId;
            entry->payloadId = entry->appId;
#ifdef DEBUG_APP_NAME
            entry->appName = strdup(appName);
            if (!entry->appName)
                _dpd.errMsg("failed to allocate appName");
#endif

            AppInfoTableDyn.table[AppInfoTableDyn.usedCount++] = entry;
        }
        else
        {
            _dpd.errMsg("calloc failure\n");
        }
    }
    else
    {
        appId = appGetAppIdFromSnortId(snortId);
        entry = getAppInfoEntry(appId); 
    }

    return entry;
}

void appInfoTableInit(const char *path)
{
    FILE *tableFile;
    const char *token;
    char buf[MAX_TABLE_LINE_LEN];
    AppInfoTableEntry *entry;
    tAppId appId;
    uint32_t clientId, serviceId, payloadId;
    char filepath[PATH_MAX];
    char *appName;
    char *appKey;

    appInfoTableFini();
    memset(AppInfoTable, 0, sizeof(AppInfoTable));
    memset(AppInfoTableByService, 0, sizeof(AppInfoTableByService));
    memset(AppInfoTableByClient, 0, sizeof(AppInfoTableByClient));
    memset(AppInfoTableByPayload, 0, sizeof(AppInfoTableByPayload));

    snprintf(filepath, sizeof(filepath), "%s/odp/%s", path, APP_MAPPING_FILE);

    tableFile = fopen(filepath, "r");
    if (tableFile == NULL)
    {
        _dpd.logMsg("Could not open RnaAppMapping Table file: %s\n", filepath);
        return;
    }

    _dpd.logMsg("    AppInfo read from %s\n", filepath);

    while (fgets(buf, sizeof(buf), tableFile))
    {
        token = strtok(buf, CONF_SEPARATORS);
        if (!token)
        {
            _dpd.errMsg("Could not read id for Rna Id\n");
            continue;
        }

        appId = strtol(token, NULL, 10);

        token = strtok(NULL, CONF_SEPARATORS);
        if (!token)
        {
            _dpd.errMsg("Could not read appName. Line %s\n", buf);
            continue;
        }

        appName = strdup(token);
        if (!appName)
        {
            _dpd.errMsg("Could not allocate space for appName\n");
            continue;
        }

        token = strtok(NULL, CONF_SEPARATORS);
        if (!token)
        {
            _dpd.errMsg("Could not read service id for Rna Id\n");
            free(appName);
            continue;
        }

        serviceId = strtol(token, NULL, 10);

        token = strtok(NULL, CONF_SEPARATORS);
        if (!token)
        {
            _dpd.errMsg("Could not read client id for Rna Id\n");
            free(appName);
            continue;
        }

        clientId = strtol(token, NULL, 10);

        token = strtok(NULL, CONF_SEPARATORS);
        if (!token)
        {
            _dpd.errMsg("Could not read payload id for Rna Id\n");
            free(appName);
            continue;
        }

        payloadId = strtol(token, NULL, 10);

        /* snort service key, if it exists */
        strtok(NULL, CONF_SEPARATORS);
        appKey = strtok(NULL, CONF_SEPARATORS);
        if (strlen(appKey) >= MAX_EVENT_APPNAME_LEN)
        {
            appKey[MAX_EVENT_APPNAME_LEN-1] = 0;
            _dpd.errMsg("appKey \'%s\' truncated to \'%s\'\n", token, appKey);
        }

        if ((entry = calloc(1, sizeof(*entry))))
        {
            entry->next = AppInfoList;
            AppInfoList = entry;

            entry->snortId = _dpd.addProtocolReference(appKey);

#ifdef DEBUG_APP_NAME
            entry->appName = appName;
#else
            free(appName);
#endif


            entry->appId = appId;
            entry->serviceId = serviceId;
            entry->clientId = clientId;
            entry->payloadId = payloadId;
            if (entry->appId > 0 && entry->appId < SF_APPID_MAX)
                AppInfoTable[entry->appId] = entry;
            if (entry->serviceId > 0 && entry->serviceId < SF_APPID_MAX)
                AppInfoTableByService[entry->serviceId] = entry;
            if (entry->clientId > 0 && entry->clientId < SF_APPID_MAX)
                AppInfoTableByClient[entry->clientId] = entry;
            if (entry->payloadId > 0 && entry->payloadId < SF_APPID_MAX)
                AppInfoTableByPayload[entry->payloadId] = entry;
        }
    }
    fclose(tableFile);

    AppInfoTableDyn.table = NULL;
    AppInfoTableDyn.usedCount = 0;
    AppInfoTableDyn.allocatedCount = 0;
    AppInfoTableDyn.stepSize = 1; /*TBD increase it to 100 after testing */

    /* Configuration defaults. */
    appIdConfig.rtmp_max_packets = 15;

    snprintf(filepath, sizeof(filepath), "%s/odp/%s", path, APP_CONFIG_FILE);
    appIdConfLoad (filepath);
    snprintf(filepath, sizeof(filepath), "%s/custom/%s", path, USR_CONFIG_FILE);
    appIdConfLoad (filepath);
}

void appInfoTableDump(void)
{
    AppInfoTableEntry *entry;
    tAppId appId;

    _dpd.errMsg("Cisco provided detectors:\n");
    for (appId = 1; appId < SF_APPID_MAX; appId++)
    {
        entry = AppInfoTable[appId];
        if (entry)
#ifdef DEBUG_APP_NAME
            _dpd.errMsg("%s\t%d\t%s\n", entry->appName, entry->appId, (entry->flags & APPINFO_FLAG_ACTIVE)? "active":"inactive");
#else
            _dpd.errMsg("%s\t%d\t%s\n", entry->appId, entry->appId, (entry->flags & APPINFO_FLAG_ACTIVE)? "active":"inactive");
#endif
    }
    _dpd.errMsg("User provided detectors:\n");
    for (appId = SF_APPID_DYNAMIC_MIN; appId < (SF_APPID_DYNAMIC_MIN + AppInfoTableDyn.usedCount); appId++)
    {
        entry = AppInfoTableDyn.table[appId - SF_APPID_DYNAMIC_MIN];
        if (entry)
#ifdef DEBUG_APP_NAME
            _dpd.errMsg("%s\t%d\t%s\n", entry->appName, entry->appId, (entry->flags & APPINFO_FLAG_ACTIVE)? "active":"inactive");
#else
            _dpd.errMsg("%s\t%d\t%s\n", entry->appId, entry->appId, (entry->flags & APPINFO_FLAG_ACTIVE)? "active":"inactive");
#endif
    }
}
tAppId appGetAppFromServiceId(uint32_t appId)
{
    AppInfoTableEntry *entry;

    if (appId > APP_ID_NONE && appId < SF_APPID_MAX)
        entry = AppInfoTableByService[appId];
    else if (appId >= SF_APPID_DYNAMIC_MIN && appId < (SF_APPID_DYNAMIC_MIN + AppInfoTableDyn.usedCount))
        entry = AppInfoTableDyn.table[appId - SF_APPID_DYNAMIC_MIN];
    else
        entry = NULL;

    return entry ? entry->appId : APP_ID_NONE;
}

tAppId appGetAppFromClientId(uint32_t appId)
{
    AppInfoTableEntry *entry;

    if (appId > APP_ID_NONE && appId < SF_APPID_MAX)
        entry = AppInfoTableByClient[appId];
    else if (appId >= SF_APPID_DYNAMIC_MIN && appId < (SF_APPID_DYNAMIC_MIN + AppInfoTableDyn.usedCount))
        entry = AppInfoTableDyn.table[appId - SF_APPID_DYNAMIC_MIN];
    else
        entry = NULL;

    return entry ? entry->appId : APP_ID_NONE;
}
tAppId appGetAppFromPayloadId(uint32_t appId)
{
    AppInfoTableEntry *entry;

    if (appId > APP_ID_NONE && appId < SF_APPID_MAX)
        entry = AppInfoTableByPayload[appId];
    else if (appId >= SF_APPID_DYNAMIC_MIN && appId < (SF_APPID_DYNAMIC_MIN + AppInfoTableDyn.usedCount))
        entry = AppInfoTableDyn.table[appId - SF_APPID_DYNAMIC_MIN];
    else
        entry = NULL;

    return entry ? entry->appId : APP_ID_NONE;
}

tAppId appGetSnortIdFromAppId(tAppId appId)
{
    AppInfoTableEntry *entry;

    if (appId > APP_ID_NONE && appId < SF_APPID_MAX)
        entry = AppInfoTable[appId];
    else if (appId >= SF_APPID_DYNAMIC_MIN && appId < (SF_APPID_DYNAMIC_MIN + AppInfoTableDyn.usedCount))
        entry = AppInfoTableDyn.table[appId - SF_APPID_DYNAMIC_MIN];
    else
        entry = NULL;

    return entry ? entry->snortId : 0;
}

/*inefficient but used only when user overrides cisco provided detector.  */
static tAppId appGetAppIdFromSnortId(int16_t snortId)
{
    tAppId i;

    for (i = 1; i < SF_APPID_MAX; i++)
        if (AppInfoTable[i] && AppInfoTable[i]->snortId == snortId)
            return i;

    for (i = SF_APPID_DYNAMIC_MIN; i < (SF_APPID_DYNAMIC_MIN + AppInfoTableDyn.usedCount); i++)
    {
        AppInfoTableEntry * entry = AppInfoTableDyn.table[i - SF_APPID_DYNAMIC_MIN];
        if (entry && entry->snortId == snortId)
            return i;
    }

    return APP_ID_NONE;
}

void appInfoSetActive(tAppId appId, bool active)
{
    AppInfoTableEntry *entry = NULL;

    if (appId == APP_ID_NONE)
        return;

    if (appId > APP_ID_NONE && appId < SF_APPID_MAX)
        entry =  AppInfoTable[appId];
    if (appId >= SF_APPID_DYNAMIC_MIN && appId < (SF_APPID_DYNAMIC_MIN + AppInfoTableDyn.usedCount))
        entry =  AppInfoTableDyn.table[appId - SF_APPID_DYNAMIC_MIN];
    if (entry)
    {
        if (active)
            entry->flags |= APPINFO_FLAG_ACTIVE;
        else
            entry->flags &= ~APPINFO_FLAG_ACTIVE;
    }
    else
    {
        _dpd.errMsg("AppInfo: AppId %d is UNKNOWN\n", appId);
    }
}


static void appIdConfLoad (const char *path)
{
    FILE *config_file;
    char *token;
    char buf[1024];
    char referred_app_list[4096];
    int  referred_app_index;
    char *conf_type;
    char *conf_key;
    char *conf_val;
    unsigned line = 0;

    config_file = fopen(path, "r");
    if (config_file == NULL)
    {
        _dpd.logMsg("Could not read configuration file %s\n", path);
        return;
    }
    else
    {
        _dpd.logMsg("Loading configuration file %s\n", path);
    }

    while (fgets(buf, sizeof(buf), config_file) != NULL)
    {
        line++;
        token = strtok(buf, CONF_SEPARATORS);
        if (token == NULL)
        {
            _dpd.errMsg("Could not read configuration at line %s:%u\n", path, line);
            continue;
        }
        conf_type = token;

        token = strtok(NULL, CONF_SEPARATORS);
        if (token == NULL)
        {
            _dpd.errMsg("Could not read configuration value at line %s:%u\n", path, line);
            continue;
        }
        conf_key = token;

        token = strtok(NULL, CONF_SEPARATORS);
        if (token == NULL)
        {
            _dpd.errMsg("Could not read configuration value at line %s:%u\n", path, line);
            continue;
        }
        conf_val = token;

        /* APPID configurations are for anything else - currently we only have ssl_reinspect */ 
        if (!(strcasecmp(conf_type, "appid")))
        {
            if (!(strcasecmp(conf_key, "ssl_reinspect")))
            {
                _dpd.logMsg("AppId: adding app %d to list of SSL apps that get more granular inspection.\n", atoi(conf_val));
                bitListAddApp(atoi(conf_val));
            }
            else if (!(strcasecmp(conf_key, "referred_appId")))
            {

                if (!(strcasecmp(conf_val, "disabled")))
                {
                    appIdConfig.referred_appId_disabled = 1;
                    continue;
                }

                else if (!appIdConfig.referred_appId_disabled)
                {
                    referred_app_index=0;
                    referred_app_index += sprintf(referred_app_list, "%d ", atoi(conf_val));
                    referredAppIdAddApp(atoi(conf_val));

                    while ((token = strtok(NULL, CONF_SEPARATORS)) != NULL)
                    {
                        referred_app_index += sprintf(referred_app_list+referred_app_index, "%d ", atoi(token));
                        referredAppIdAddApp(atoi(token));
                    }
                    _dpd.logMsg("AppId: adding appIds to list of referred web apps: %s\n", referred_app_list);
                }
            }
            else if (!(strcasecmp(conf_key, "rtmp_max_packets")))
            {
                appIdConfig.rtmp_max_packets = atoi(conf_val);
            }
            else /*(!(strcasecmp(conf_key, "mdns_user_report"))) */
            {
                appIdConfig.mdns_user_reporting = atoi(conf_val);
            }
        }
    }
    fclose(config_file);
}

