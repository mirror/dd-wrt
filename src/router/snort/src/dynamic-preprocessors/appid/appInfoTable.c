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
#include "appId.h"
#include "appInfoTable.h"
#include "common_util.h"
#include "Unified2_common.h"

#define APP_MAPPING_FILE "appMapping.data"
#define APP_CONFIG_FILE "appid.conf"
#define USR_CONFIG_FILE "userappid.conf"

#define MAX_TABLE_LINE_LEN      1024
// Generic delimiter for conf file
#define CONF_SEPARATORS         "\t\n\r"
// Delimiter for appid.conf  and userappid.conf file
#define CONF_SEPARATORS_USR_APPID         " \t\n\r"
#define MIN_MAX_TP_FLOW_DEPTH   1
#define MAX_MAX_TP_FLOW_DEPTH   1000000
#define MIN_HOST_PORT_APP_CACHE_LOOKUP_INTERVAL    1
#define MAX_HOST_PORT_APP_CACHE_LOOKUP_INTERVAL    1000000
#define MIN_HOST_PORT_APP_CACHE_LOOKUP_RANGE       1
#define MAX_HOST_PORT_APP_CACHE_LOOKUP_RANGE       1000000
#define MIN_MAX_BYTES_BEFORE_SERVICE_FAIL 4096
#define MIN_MAX_PACKET_BEFORE_SERVICE_FAIL 5
#define MIN_MAX_PACKET_BEFORE_SERVICE_FAIL_IGNORE_BYTES 15
struct DynamicArray
{ 
    void **table;
    size_t  indexStart;
    size_t  indexCurrent;
    size_t  usedCount;
    size_t  allocatedCount;
    size_t  stepSize;
};
typedef struct DynamicArray tDynamicArray;

static inline struct DynamicArray* dynamicArrayCreate(unsigned indexStart)
{
    struct DynamicArray *array;

    if ((array = _dpd.snortAlloc(1, sizeof(*array), PP_APP_ID, PP_MEM_CATEGORY_CONFIG)))
    {
        array->stepSize = 1; 
        array->indexStart = indexStart;
    }
    return array;
}

static inline void dynamicArrayDestroy(struct DynamicArray *array)
{
    unsigned i;
    AppInfoTableEntry *entry;

    if (!array)
        return;
    for (i = 0; i < array->usedCount; i++)
    {
        entry = array->table[i];
        free(entry->appName);
        _dpd.snortFree(entry, sizeof(*entry), PP_APP_ID, PP_MEM_CATEGORY_CONFIG);
    }

    free(array->table);
    _dpd.snortFree(array, sizeof(*array), PP_APP_ID, PP_MEM_CATEGORY_CONFIG);
}

static inline void dynamicArraySetIndex(struct DynamicArray *array, unsigned index, void* data)
{
    if (index >= array->indexStart && index < (array->indexStart + array->usedCount))
        array->table[index - array->indexStart] = data;
} 
static inline void* dynamicArrayGetIndex(struct DynamicArray *array, unsigned index)
{
    if (index >= array->indexStart && index < (array->indexStart + array->usedCount))
        return array->table[index - array->indexStart];
    return NULL;
} 
static inline bool dynamicArrayCreateIndex(struct DynamicArray *array, unsigned *index)
{
    if (array->usedCount == array->allocatedCount)
    {
        void** tmp = realloc(array->table, (array->allocatedCount + array->stepSize)*sizeof(*tmp));
        if (!tmp)
        {
            return false;
        }
        array->table = tmp;
        array->allocatedCount += array->stepSize;
    }
    *index = array->indexStart + (array->usedCount++);
    return true;
} 

static inline void* dynamicArrayGetFirst(struct DynamicArray *array)
{
    AppInfoTableEntry *entry;
    for (array->indexCurrent = 0; array->indexCurrent < array->usedCount; array->indexCurrent++)
    {
        if ((entry = array->table[array->indexCurrent]))
            return entry;
    }
    return NULL;
}
static inline void* dynamicArrayGetNext(struct DynamicArray *array)
{
    AppInfoTableEntry *entry;
    for (array->indexCurrent++; array->indexCurrent < array->usedCount; array->indexCurrent++)
    {
        if ((entry = array->table[array->indexCurrent]))
            return entry;
    }
    return NULL;
}
// End of Dynamic array
SFGHASH*  appNameHashInit()
{
    SFGHASH  *appNameHash;
    appNameHash = sfghash_new(65, 0, 0 /* alloc copies of lowercased keys */, NULL);
    if (!appNameHash)
    {
        _dpd.fatalMsg("AppNameHash: Failed to Initialize\n");
    }
    return appNameHash;
}
void  appNameHashFini(SFGHASH *appNameHash)
{
    if (appNameHash)
    {
        sfghash_delete(appNameHash);
    }
}

static inline char *strdupToLower(const char *source)
{
    int index;
    char *dest = malloc(strlen(source)+1);
    
    if (dest) 
    {
        for(index = 0;; index++)
        {
            if (source[index])
            {
                dest[index] = tolower(source[index]);
                continue;
            }
            else
            {
                dest[index] = '\0';
                break;
            }
        }
    }
    else
        _dpd.errMsg("strdupToLower: Failed to allocate memory for destination\n");

    return dest;
}

void appNameHashAdd(SFGHASH *appNameHash, const char *appName, void *data)
{
    char *searchName;
    int errCode;

    if (!appName || !appNameHash)
        return;
    
    searchName = strdupToLower(appName);
    if (!searchName)
        return;
        
    if (SFGHASH_OK == (errCode = sfghash_add(appNameHash, searchName, data)))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_APPID, "App name added for %s\n", appName););
    }
    else if (SFGHASH_INTABLE == errCode)
    {
        /* Note that, although this entry is not placed in the hash table, 
           being a duplicate, it remains in the list of allocated entries
           for cleanup by appInfoTableFini() */
        
        // Rediscover the existing, hashed entry for the purpose of a complete error message.
        AppInfoTableEntry* tableEntry = (AppInfoTableEntry*)sfghash_find(appNameHash, searchName);

        if (tableEntry)
        {
            _dpd.errMsg("App name, \"%s\", is a duplicate of \"%s\" and has been ignored.\n", 
                appName, tableEntry->appName );
        }
        else
        {
            _dpd.errMsg("App name, \"%s\", has been ignored. Hash key \"%s\" is not unique.\n", 
                appName, searchName );
        }
    }
    free(searchName);
}

void* appNameHashFind(SFGHASH *appNameHash, const char *appName)
{
    void *data;
    char *searchName;

    if (!appName || !appNameHash)
        return NULL;

    searchName = strdupToLower(appName);
    if (!searchName)
        return NULL;
        
    data = sfghash_find(appNameHash, searchName);

    free(searchName);

    return data;
}
// End of appName hash

static void appIdConfLoad (tAppidStaticConfig* appidSC, const char *path);

static unsigned int getAppIdStaticIndex(tAppId appid)
{
    if (appid > 0 && appid < SF_APPID_BUILDIN_MAX) 
        return appid;
    if (appid >= SF_APPID_CSD_MIN && appid < SF_APPID_CSD_MIN+(SF_APPID_MAX-SF_APPID_BUILDIN_MAX))
        return (SF_APPID_BUILDIN_MAX + appid - SF_APPID_CSD_MIN);
    return 0;
}

AppInfoTableEntry* appInfoEntryGet(tAppId appId, const tAppIdConfig *pConfig)
{
    tAppId tmp;
    if ((tmp = getAppIdStaticIndex(appId)))
        return pConfig->AppInfoTable[tmp];
    return dynamicArrayGetIndex(pConfig->AppInfoTableDyn, appId);
}

AppInfoTableEntry* appInfoEntryCreate(const char *appName, tAppIdConfig *pConfig)
{
    tAppId appId;
    AppInfoTableEntry *entry;
   
    if (!appName || strlen(appName) >= MAX_EVENT_APPNAME_LEN)
    {
        _dpd.errMsg("Appname invalid\n", appName);
        return NULL;
    }

    entry = appNameHashFind(pConfig->AppNameHash, appName);
    if (!entry)
    {
        if (!dynamicArrayCreateIndex(pConfig->AppInfoTableDyn, (uint32_t *)&appId))
        {
            return NULL;
        }

        if ((entry = _dpd.snortAlloc(1, sizeof(*entry), PP_APP_ID, PP_MEM_CATEGORY_CONFIG)))
        {
            entry->appId = appId;
            entry->serviceId = entry->appId;
            entry->clientId = entry->appId;
            entry->payloadId = entry->appId;
            entry->appName = strdup(appName);
            if (!entry->appName)
            {
                _dpd.errMsg("failed to allocate appName");
                _dpd.snortFree(entry, sizeof(*entry), PP_APP_ID, PP_MEM_CATEGORY_CONFIG);
                return NULL;
            }

            dynamicArraySetIndex(pConfig->AppInfoTableDyn, appId, entry);
            if (pConfig->AppNameHash != NULL)
                appNameHashAdd(pConfig->AppNameHash, appName, entry);
        }
        else
        {
            _dpd.errMsg("calloc failure\n");
        }
    }
    return entry;
}

void appInfoTableInit(tAppidStaticConfig* appidSC, tAppIdConfig* pConfig)
{
    FILE *tableFile;
    const char *token;
    char buf[MAX_TABLE_LINE_LEN];
    AppInfoTableEntry *entry;
    tAppId appId;
    uint32_t clientId, serviceId, payloadId;
    char filepath[PATH_MAX];
    char *appName=NULL;
    char *snortName=NULL;

    pConfig->AppInfoTableDyn = dynamicArrayCreate(SF_APPID_DYNAMIC_MIN);

    snprintf(filepath, sizeof(filepath), "%s/odp/%s", appidSC->app_id_detector_path, APP_MAPPING_FILE);

    tableFile = fopen(filepath, "r");
    if (tableFile == NULL)
    {
        _dpd.errMsg("Could not open RnaAppMapping Table file: %s\n", filepath);
        return;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "    AppInfo read from %s\n", filepath););

    while (fgets(buf, sizeof(buf), tableFile))
    {
        token = strtok(buf, CONF_SEPARATORS );
        if (!token)
        {
            _dpd.errMsg("Could not read id for Rna Id\n");
            continue;
        }

        appId = strtol(token, NULL, 10);

        token = strtok(NULL, CONF_SEPARATORS );
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

        token = strtok(NULL, CONF_SEPARATORS );
        if (!token)
        {
            _dpd.errMsg("Could not read service id for Rna Id\n");
            free(appName);
            continue;
        }

        serviceId = strtol(token, NULL, 10);

        token = strtok(NULL, CONF_SEPARATORS );
        if (!token)
        {
            _dpd.errMsg("Could not read client id for Rna Id\n");
            free(appName);
            continue;
        }

        clientId = strtol(token, NULL, 10);

        token = strtok(NULL, CONF_SEPARATORS );
        if (!token)
        {
            _dpd.errMsg("Could not read payload id for Rna Id\n");
            free(appName);
            continue;
        }

        payloadId = strtol(token, NULL, 10);

        /* snort service key, if it exists */
        token = strtok(NULL, CONF_SEPARATORS );
        if (token)
        {
            snortName = strdup(token);
            if (!snortName)
            {
                _dpd.errMsg("malloc failure\n");
                free(appName);
                continue;
            }
        }

        entry = _dpd.snortAlloc(1, sizeof(*entry), PP_APP_ID, PP_MEM_CATEGORY_CONFIG);
        if (!entry)
        {
            _dpd.errMsg("AppInfoTable: Memory allocation failure\n");
            free(appName);
            free(snortName);
            continue;
        }

        entry->next = pConfig->AppInfoList;
        pConfig->AppInfoList = entry;

        if (snortName)
        {
#ifdef TARGET_BASED
            entry->snortId = _dpd.addProtocolReference(snortName);
            free(snortName);
            snortName = NULL;
#endif
        }

        entry->appName = appName;

        entry->appId = appId;
        entry->serviceId = serviceId;
        entry->clientId = clientId;
        entry->payloadId = payloadId;
        entry->priority = APP_PRIORITY_DEFAULT;

        if ((appId = getAppIdStaticIndex(entry->appId)))
            pConfig->AppInfoTable[appId] = entry;
        if ((appId = getAppIdStaticIndex(entry->serviceId)))
            pConfig->AppInfoTableByService[appId] = entry;
        if ((appId = getAppIdStaticIndex(entry->clientId)))
            pConfig->AppInfoTableByClient[appId] = entry;
        if ((appId = getAppIdStaticIndex(entry->payloadId)))
            pConfig->AppInfoTableByPayload[appId] = entry;

        if (!pConfig->AppNameHash)
        {
            pConfig->AppNameHash = appNameHashInit();
        }
        appNameHashAdd(pConfig->AppNameHash, appName, entry);
    }
    fclose(tableFile);

    /* Configuration defaults. */
    appidSC->rtmp_max_packets = 15;
    appidSC->mdns_user_reporting = 1;
    appidSC->dns_host_reporting = 1;
    appidSC->max_tp_flow_depth = 5;
    appidSC->http2_detection_enabled = 0;
    appidSC->host_port_app_cache_lookup_interval = 10;
    appidSC->host_port_app_cache_lookup_range = 100000;
    appidSC->is_host_port_app_cache_runtime = 1;
    appidSC->check_host_port_app_cache = 0;
    appidSC->check_host_cache_unknown_ssl = 0;
    appidSC->recheck_for_unknown_appid = 0;
    appidSC->send_state_sharing_updates = 1;
    appidSC->allow_port_wildcard_host_cache = 0;
    appidSC->recheck_for_portservice_appid = 0;
    appidSC->max_packet_before_service_fail = MIN_MAX_PACKET_BEFORE_SERVICE_FAIL;
    appidSC->max_bytes_before_service_fail = MIN_MAX_BYTES_BEFORE_SERVICE_FAIL;
    appidSC->max_packet_service_fail_ignore_bytes = MIN_MAX_PACKET_BEFORE_SERVICE_FAIL_IGNORE_BYTES;
    appidSC->http_tunnel_detect = HTTP_TUNNEL_DETECT_RESTART;
    snprintf(filepath, sizeof(filepath), "%s/odp/%s", appidSC->app_id_detector_path, APP_CONFIG_FILE);
    appIdConfLoad (appidSC, filepath);
    snprintf(filepath, sizeof(filepath), "%s/../%s", appidSC->app_id_detector_path, USR_CONFIG_FILE);
    appIdConfLoad (appidSC, filepath);
}

void appInfoTableFini(tAppIdConfig *pConfig)
{
    AppInfoTableEntry *entry;

    while ((entry = pConfig->AppInfoList))
    {
        pConfig->AppInfoList = entry->next;
        if (entry->appName)	
            free(entry->appName);
        _dpd.snortFree(entry, sizeof(*entry), PP_APP_ID, PP_MEM_CATEGORY_CONFIG);
    }

    dynamicArrayDestroy(pConfig->AppInfoTableDyn);
    pConfig->AppInfoTableDyn = NULL;

    appNameHashFini(pConfig->AppNameHash);
}

void appInfoTableDump(tAppIdConfig *pConfig)
{
    AppInfoTableEntry *entry;
    tAppId appId;

    _dpd.errMsg("Cisco provided detectors:\n");
    for (appId = 1; appId < SF_APPID_MAX; appId++)
    {
        entry = pConfig->AppInfoTable[appId];
        if (entry)
            _dpd.errMsg("%s\t%d\t%s\n", entry->appName, entry->appId, (entry->flags & APPINFO_FLAG_ACTIVE)? "active":"inactive");
    }
    _dpd.errMsg("User provided detectors:\n");
    for (entry = dynamicArrayGetFirst(pConfig->AppInfoTableDyn); entry; entry = dynamicArrayGetNext(pConfig->AppInfoTableDyn))
    {
        _dpd.errMsg("%s\t%d\t%s\n", entry->appName, entry->appId, (entry->flags & APPINFO_FLAG_ACTIVE)? "active":"inactive");
    }
}
tAppId appGetAppFromServiceId(uint32_t appId, tAppIdConfig *pConfig)
{
    AppInfoTableEntry *entry;
    tAppId tmp;

    if ((tmp = getAppIdStaticIndex(appId)))
        entry = pConfig->AppInfoTableByService[tmp];
    else 
        entry = dynamicArrayGetIndex(pConfig->AppInfoTableDyn, appId);

    return entry ? entry->appId : APP_ID_NONE;
}

tAppId appGetAppFromClientId(uint32_t appId, tAppIdConfig *pConfig)
{
    AppInfoTableEntry *entry;
    tAppId tmp;

    if ((tmp = getAppIdStaticIndex(appId)))
        entry = pConfig->AppInfoTableByClient[tmp];
    else 
        entry = dynamicArrayGetIndex(pConfig->AppInfoTableDyn, appId);

    return entry ? entry->appId : APP_ID_NONE;
}
tAppId appGetAppFromPayloadId(uint32_t appId, tAppIdConfig *pConfig)
{
    AppInfoTableEntry *entry;
    tAppId tmp;

    if ((tmp = getAppIdStaticIndex(appId)))
        entry = pConfig->AppInfoTableByPayload[tmp];
    else 
        entry = dynamicArrayGetIndex(pConfig->AppInfoTableDyn, appId);

    return entry ? entry->appId : APP_ID_NONE;
}
const char * appGetAppName(int32_t appId)
{
    AppInfoTableEntry *entry;
    tAppIdConfig *pConfig = appIdActiveConfigGet();
    tAppId tmp;

    if ((tmp = getAppIdStaticIndex(appId)))
        entry = pConfig->AppInfoTable[tmp];
    else 
        entry = dynamicArrayGetIndex(pConfig->AppInfoTableDyn, appId);

    return entry ? entry->appName : NULL;
}

int32_t appGetAppId(const char *appName)
{
    AppInfoTableEntry *entry;
    tAppIdConfig *pConfig = appIdActiveConfigGet();

    entry = appNameHashFind(pConfig->AppNameHash, appName);
    return entry?entry->appId:0;
}

void appInfoSetActive(tAppId appId, bool active)
{
    AppInfoTableEntry *entry = NULL;
    tAppIdConfig *pConfig = appIdActiveConfigGet();
    tAppId tmp;

    if (appId == APP_ID_NONE)
        return;

    if ((tmp = getAppIdStaticIndex(appId)))
        entry =  pConfig->AppInfoTable[tmp];
    else 
        entry = dynamicArrayGetIndex(pConfig->AppInfoTableDyn, appId);

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


static void appIdConfLoad (tAppidStaticConfig* appidSC, const char *path)
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
    tAppIdConfig *pConfig = appIdNewConfigGet();
    int max_tp_flow_depth;
    int host_port_app_cache_lookup_interval;
    int host_port_app_cache_lookup_range;

    config_file = fopen(path, "r");
    if (config_file == NULL)
    {
        return;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_APPID, "Loading configuration file %s\n", path););
    }

    while (fgets(buf, sizeof(buf), config_file) != NULL)
    {
        line++;
        token = strtok(buf, CONF_SEPARATORS_USR_APPID);
        if (token == NULL)
        {
            _dpd.errMsg("Could not read configuration at line %s:%u\n", path, line);
            continue;
        }
        conf_type = token;

        token = strtok(NULL, CONF_SEPARATORS_USR_APPID);
        if (token == NULL)
        {
            _dpd.errMsg("Could not read configuration value at line %s:%u\n", path, line);
            continue;
        }
        conf_key = token;

        token = strtok(NULL, CONF_SEPARATORS_USR_APPID);
        if (token == NULL)
        {
            _dpd.errMsg("Could not read configuration value at line %s:%u\n", path, line);
            continue;
        }
        conf_val = token;

        /* APPID configurations are for anything else - currently we only have ssl_reinspect */ 
        if (!(strcasecmp(conf_type, "appid")))
        {
            if (!(strcasecmp(conf_key, "max_tp_flow_depth")))
            {
                max_tp_flow_depth = atoi(conf_val);
                if (max_tp_flow_depth < MIN_MAX_TP_FLOW_DEPTH || max_tp_flow_depth > MAX_MAX_TP_FLOW_DEPTH)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: invalid max_tp_flow_depth %d, must be between %d and %d\n.", max_tp_flow_depth, MIN_MAX_TP_FLOW_DEPTH, MAX_MAX_TP_FLOW_DEPTH););
                }
                else
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: setting max thirdparty inspection flow depth to %d packets.\n", max_tp_flow_depth););
                    appidSC->max_tp_flow_depth = max_tp_flow_depth;
                }
            }
            else if (!(strcasecmp(conf_key, "host_port_app_cache_lookup_interval")))
            {
                host_port_app_cache_lookup_interval = atoi(conf_val);
                if (host_port_app_cache_lookup_interval < MIN_HOST_PORT_APP_CACHE_LOOKUP_INTERVAL || host_port_app_cache_lookup_interval > MAX_HOST_PORT_APP_CACHE_LOOKUP_INTERVAL)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: invalid host_port_app_cache_lookup_interval %d, must be between %d and %d\n.",
                                            host_port_app_cache_lookup_interval, MIN_HOST_PORT_APP_CACHE_LOOKUP_INTERVAL, MAX_HOST_PORT_APP_CACHE_LOOKUP_INTERVAL););
                }
                else
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: hostPortCache lookup performed every %d packet(s).\n", host_port_app_cache_lookup_interval););
                    appidSC->host_port_app_cache_lookup_interval = host_port_app_cache_lookup_interval;
                }
            }
            else if (!(strcasecmp(conf_key, "host_port_app_cache_lookup_range")))
            {
                host_port_app_cache_lookup_range = atoi(conf_val);
                if (host_port_app_cache_lookup_range < MIN_HOST_PORT_APP_CACHE_LOOKUP_RANGE || host_port_app_cache_lookup_range > MAX_HOST_PORT_APP_CACHE_LOOKUP_RANGE)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: invalid host_port_app_cache_lookup_range %d, must be between %d and %d\n.",
                                            host_port_app_cache_lookup_range, MIN_HOST_PORT_APP_CACHE_LOOKUP_RANGE, MAX_HOST_PORT_APP_CACHE_LOOKUP_RANGE););
                }
                else
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: hostPortCache lookup performed till maximum of %d packet(s) per session.\n", host_port_app_cache_lookup_range););
                    appidSC->host_port_app_cache_lookup_range = host_port_app_cache_lookup_range;
                }
            }
            else if (!(strcasecmp(conf_key, "is_host_port_app_cache_runtime")))
            {
                if (!(strcasecmp(conf_val, "disabled")))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: hostPortCache not configured for runtime modification.\n"););
                    appidSC->is_host_port_app_cache_runtime = 0;
                }
            }
            else if (!(strcasecmp(conf_key, "check_host_port_app_cache")))
            {
                if (!(strcasecmp(conf_val, "enabled")))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: hostPortCache lookup performed for every flow.\n"););
                    appidSC->check_host_port_app_cache = 1;
                }
            }
            else if (!(strcasecmp(conf_key, "check_host_cache_unknown_ssl")))
            {
                if (!(strcasecmp(conf_val, "enabled")))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: hostPortCache lookup performed for SSL flows not having either of server name or certificate.\n"););
                    appidSC->check_host_cache_unknown_ssl = 1;
                }
            }
            else if (!(strcasecmp(conf_key, "recheck_for_unknown_appid")))
            {
                if (!(strcasecmp(conf_val, "enabled")))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: Flows with unknown AppID's will not be ignored.\n"););
                    appidSC->recheck_for_unknown_appid = 1;
                }
            }
            else if (!(strcasecmp(conf_key, "recheck_for_portservice_appid")))
            {
                if (!(strcasecmp(conf_val, "enabled")))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: Checking hostPortCache for flows with only port service AppID.\n"););
                    appidSC->recheck_for_portservice_appid = 1;
                }
            }
            else if (!(strcasecmp(conf_key, "tp_allow_probes")))
            {
                if (!(strcasecmp(conf_val, "enabled")))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: TCP probes will be analyzed by NAVL.\n"););
                    appidSC->tp_allow_probes = 1;
                }
            }
            else if (!(strcasecmp(conf_key, "tp_client_app")))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: if thirdparty reports app %d, we will use it as a client.\n", atoi(conf_val)););
                appInfoEntryFlagSet(atoi(conf_val), APPINFO_FLAG_TP_CLIENT, pConfig);
            }
            else if (!(strcasecmp(conf_key, "ssl_reinspect")))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: adding app %d to list of SSL apps that get more granular inspection.\n", atoi(conf_val)););
                appInfoEntryFlagSet(atoi(conf_val), APPINFO_FLAG_SSL_INSPECT, pConfig);
            }
            else if (!(strcasecmp(conf_key, "disable_safe_search")))
            {
                if (!(strcasecmp(conf_val, "disabled")))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: disabling safe search enforcement.\n"););
                    appidSC->disable_safe_search = 1;
                }
            }
            else if (!(strcasecmp(conf_key, "ssl_squelch")))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: adding app %d to list of SSL apps that may open a second SSL connection.\n", atoi(conf_val)););
                appInfoEntryFlagSet(atoi(conf_val), APPINFO_FLAG_SSL_SQUELCH, pConfig);
            }
            else if (!(strcasecmp(conf_key, "defer_to_thirdparty")))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: adding app %d to list of apps where we should take thirdparty ID over the NDE's.\n", atoi(conf_val)););
                appInfoEntryFlagSet(atoi(conf_val), APPINFO_FLAG_DEFER, pConfig);
            }
            else if (!(strcasecmp(conf_key, "defer_payload_to_thirdparty")))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: adding app %d to list of apps where we should take thirdparty payload ID over the NDE's.\n", atoi(conf_val)););
                appInfoEntryFlagSet(atoi(conf_val), APPINFO_FLAG_DEFER_PAYLOAD, pConfig);
            }
            else if (!(strcasecmp(conf_key, "chp_userid")))
            {
                if (!(strcasecmp(conf_val, "disabled")))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: HTTP UserID collection disabled.\n"););
                    appidSC->chp_userid_disabled = 1;
                    continue;
                }
            }
            else if (!(strcasecmp(conf_key, "chp_body_collection")))
            {
                if (!(strcasecmp(conf_val, "disabled")))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: HTTP Body header reading disabled.\n"););
                    appidSC->chp_body_collection_disabled = 1;
                    continue;
                }
            }
            else if (!(strcasecmp(conf_key, "chp_fflow")))
            {
                if (!(strcasecmp(conf_val, "disabled")))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: HTTP future flow creation disabled.\n"););
                    appidSC->chp_fflow_disabled = 1;
                    continue;
                }
            }
            else if (!(strcasecmp(conf_key, "ftp_userid")))
            {
                if (!(strcasecmp(conf_val, "disabled")))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: FTP userID disabled.\n"););
                    appidSC->ftp_userid_disabled = 1;
                    continue;
                }
            }
            else if (!(strcasecmp(conf_key, "max_bytes_before_service_fail")))
            {
                uint64_t max_bytes_before_service_fail = atoi(conf_val);
                if (max_bytes_before_service_fail < MIN_MAX_BYTES_BEFORE_SERVICE_FAIL)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: invalid max_bytes_before_service_fail %"PRIu64" must be greater than %u\n.", max_bytes_before_service_fail, MIN_MAX_BYTES_BEFORE_SERVICE_FAIL ););
                }
                else
                    appidSC->max_bytes_before_service_fail = max_bytes_before_service_fail;
            }
            else if (!(strcasecmp(conf_key, "max_packet_before_service_fail")))
            {
                uint16_t max_packet_before_service_fail = atoi(conf_val);
                if (max_packet_before_service_fail < MIN_MAX_PACKET_BEFORE_SERVICE_FAIL)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: invalid max_packet_before_service_fail %"PRIu16", must be greater than  %u \n.", max_packet_before_service_fail, MIN_MAX_PACKET_BEFORE_SERVICE_FAIL););
                }
                else
                    appidSC->max_packet_before_service_fail = max_packet_before_service_fail;
            }
            else if (!(strcasecmp(conf_key, "max_packet_service_fail_ignore_bytes")))
            {
                uint16_t max_packet_service_fail_ignore_bytes = atoi(conf_val);
                if (max_packet_service_fail_ignore_bytes < MIN_MAX_PACKET_BEFORE_SERVICE_FAIL_IGNORE_BYTES)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: invalid max_packet_service_fail_ignore_bytes  %"PRIu16", must be greater than %u\n.", max_packet_service_fail_ignore_bytes, MIN_MAX_PACKET_BEFORE_SERVICE_FAIL_IGNORE_BYTES););
                }
                else
                    appidSC->max_packet_service_fail_ignore_bytes= max_packet_service_fail_ignore_bytes;
            }
            else if (!(strcasecmp(conf_key, "http_tunnel_detect")))
            {
                if (!(strcasecmp(conf_val, "restart_and_reset")))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: HTTP tunnel detect set to restart and reset.\n"););
                    appidSC->http_tunnel_detect = HTTP_TUNNEL_DETECT_RESTART_AND_RESET;
                    continue;
                }
            }
            /* App Priority bit set*/
            else if (!(strcasecmp(conf_key, "app_priority")))
            {
                int temp_appid;
                temp_appid = strtol(conf_val, NULL, 10 );
                token = strtok(NULL, CONF_SEPARATORS_USR_APPID);
                if (token == NULL)
                {
                    _dpd.errMsg("Could not read app_priority at line %u\n", line);
                    continue;
                }
                conf_val = token;
                uint8_t temp_val;
                temp_val = strtol(conf_val, NULL, 10 );
                appInfoEntryPrioritySet (temp_appid, temp_val, pConfig);
                DEBUG_WRAP(DebugMessage(DEBUG_APPID,"AppId: %d Setting priority bit %d .\n", temp_appid, temp_val););
            }
            else if (!(strcasecmp(conf_key, "referred_appId")))
            {

                if (!(strcasecmp(conf_val, "disabled")))
                {
                    appidSC->referred_appId_disabled = 1;
                    continue;
                }

                else if (!appidSC->referred_appId_disabled)
                {
                    referred_app_index=0;
                    referred_app_index += sprintf(referred_app_list, "%d ", atoi(conf_val));
                    appInfoEntryFlagSet(atoi(conf_val), APPINFO_FLAG_REFERRED, pConfig);

                    while ((token = strtok(NULL, CONF_SEPARATORS_USR_APPID)) != NULL)
                    {
                        referred_app_index += sprintf(referred_app_list+referred_app_index, "%d ", atoi(token));
                        appInfoEntryFlagSet(atoi(token), APPINFO_FLAG_REFERRED, pConfig);
                    }
                    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppId: adding appIds to list of referred web apps: %s\n", referred_app_list););
                }
            }
            else if (!(strcasecmp(conf_key, "rtmp_max_packets")))
            {
                appidSC->rtmp_max_packets = atoi(conf_val);
            }
            else if (!(strcasecmp(conf_key, "mdns_user_report")))
            {
                appidSC->mdns_user_reporting = atoi(conf_val);
            }
            else if (!(strcasecmp(conf_key, "dns_host_report")))
            {
                appidSC->dns_host_reporting = atoi(conf_val);
            }
            else if (!(strcasecmp(conf_key, "chp_body_max_bytes")))
            {
                appidSC->chp_body_collection_max = atoi(conf_val);
            }
            else if (!(strcasecmp(conf_key, "ignore_thirdparty_appid")))
            {
                _dpd.logMsg("AppId: adding app %d to list of ignore thirdparty apps.\n", atoi(conf_val));
                appInfoEntryFlagSet(atoi(conf_val), APPINFO_FLAG_IGNORE, pConfig);
            }
            else if (!(strcasecmp(conf_key, "http2_detection")))
            {
                // This option will control our own HTTP/2 detection.  We can
                // still be told externally, though, that it's HTTP/2 (either
                // from HTTP Inspect or 3rd Party).  This is intended to be
                // used to ask AppID to detect unencrypted HTTP/2 on non-std
                // ports.
                if (!(strcasecmp(conf_val, "disabled")))
                {
                    _dpd.logMsg("AppId: disabling internal HTTP/2 detection.\n");
                    appidSC->http2_detection_enabled = 0;
                }
                else if (!(strcasecmp(conf_val, "enabled")))
                {
                    _dpd.logMsg("AppId: enabling internal HTTP/2 detection.\n");
                    appidSC->http2_detection_enabled = 1;
                }
                else
                {
                    _dpd.logMsg("AppId: ignoring invalid option for http2_detection: %s\n", conf_val);
                }
            }
            else if (!(strcasecmp(conf_key, "send_state_sharing_updates")))
            {
                if (!(strcasecmp(conf_val, "disabled")))
		{
                    _dpd.logMsg("AppId: Disabling state sharing updates.\n");
                    appidSC->send_state_sharing_updates = 0;
                }
            }
            else if (!(strcasecmp(conf_key, "allow_port_wildcard_host_cache")))
            {
                if (!(strcasecmp(conf_val, "enabled")))
                {
                    _dpd.logMsg("AppId: Enabling wild card for port numbers in hostPortAppCache.\n");
                    appidSC->allow_port_wildcard_host_cache = 1;
                }
            }
            else if (!(strcasecmp(conf_key, "bittorrent_aggressiveness")))
            {
                int aggressiveness = atoi(conf_val);
                _dpd.logMsg("AppId: bittorrent_aggressiveness %d\n", aggressiveness);
                if (aggressiveness >= 50)
                {
                    appidSC->recheck_for_unknown_appid = 1;
                    appidSC->recheck_for_portservice_appid = 1;
                    appidSC->host_port_app_cache_lookup_interval = 5;
                    appidSC->max_tp_flow_depth = 25;
                    appInfoEntryFlagSet(APP_ID_BITTORRENT, APPINFO_FLAG_DEFER, pConfig);
                    appInfoEntryFlagSet(APP_ID_BITTORRENT, APPINFO_FLAG_DEFER_PAYLOAD, pConfig);
                }
                if (aggressiveness >= 80)
                {
                    appidSC->allow_port_wildcard_host_cache = 1;
                }
            }
            else if (!(strcasecmp(conf_key, "ultrasurf_aggressiveness")))
            {
                int aggressiveness = atoi(conf_val);
                _dpd.logMsg("AppId: ultrasurf_aggressiveness %d\n", aggressiveness);
                if (aggressiveness >= 50)
                {
                    appidSC->check_host_cache_unknown_ssl = 1;
                    appidSC->max_tp_flow_depth = 25;
                    appInfoEntryFlagSet(APP_ID_ULTRASURF, APPINFO_FLAG_DEFER, pConfig);
                    appInfoEntryFlagSet(APP_ID_ULTRASURF, APPINFO_FLAG_DEFER_PAYLOAD, pConfig);
                }
                if (aggressiveness >= 80)
                {
                    appidSC->recheck_for_unknown_appid = 1;
                    appidSC->allow_port_wildcard_host_cache = 1;
                }
            }
            else if (!(strcasecmp(conf_key, "psiphon_aggressiveness")))
            {
                int aggressiveness = atoi(conf_val);
                _dpd.logMsg("AppId: psiphon_aggressiveness %d\n", aggressiveness);
                if (aggressiveness >= 50)
                {
                    appidSC->check_host_cache_unknown_ssl = 1;
                    appidSC->max_tp_flow_depth = 25;
                    appInfoEntryFlagSet(APP_ID_PSIPHON, APPINFO_FLAG_DEFER, pConfig);
                    appInfoEntryFlagSet(APP_ID_PSIPHON, APPINFO_FLAG_DEFER_PAYLOAD, pConfig);
                }
                if (aggressiveness >= 80)
                {
                    appidSC->recheck_for_unknown_appid = 1;
                    appidSC->allow_port_wildcard_host_cache = 1;
                }
            }
            else if (!(strcasecmp(conf_key, "multipayload_max_packets")))
            {
                appidSC->multipayload_max_packets = atoi(conf_val);
                _dpd.logMsg("AppId: Multipayload feature will scan up to %d packets.\n", 
                    appidSC->multipayload_max_packets);
            }
        }
    }
    fclose(config_file);
}

