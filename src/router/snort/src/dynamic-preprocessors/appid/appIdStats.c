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
#include "fw_avltree.h"
#include "OutputFile.h"
#include "Unified2_common.h"
#include <time.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#undef MAX_NAME_LEN
/*#include "session_record.h" */
#include <sflsq.h>
#include "appIdStats.h"
#include "common_util.h"
#include "sf_dynamic_preprocessor.h"
#include "fw_appid.h"
#include "appInfoTable.h"

#define URLCATBUCKETS   100
#define URLREPBUCKETS   5

/*#define DEBUG_STATS */

static time_t bucketStart;
static time_t bucketInterval;
static time_t bucketEnd;

struct AppIdStatRecord 
{
        uint32_t    app_id;
        uint32_t    initiatorBytes;
        uint32_t    responderBytes;
};

#ifdef WIN32
#pragma pack(push,app_stats,1)
#else
#pragma pack(1)
#endif

struct AppIdStatOutputRecord 
{
        char        appName[MAX_EVENT_APPNAME_LEN];
        uint32_t    initiatorBytes;
        uint32_t    responderBytes;
};
#ifdef WIN32
#pragma pack(pop,app_stats)
#else
#pragma pack()
#endif

struct StatsBucket
{
    uint32_t startTime;
    struct FwAvlTree* appsTree;
    struct _SessionStatsRecord 
    {
        size_t txByteCnt;
        size_t rxByteCnt;
    } totalStats;
    uint32_t appRecordCnt;
};

static SF_LIST* currBuckets;
static SF_LIST* logBuckets;

static char*  appFilePath;

static FILE*  appfp;

static size_t appSize;

static time_t appTime;

Serial_Unified2_Header  header;

static size_t rollSize;
static time_t rollPeriod;
static bool   enableAppStats;

static void endStats2Period(void);
static void startStats2Period(time_t startTime);
static struct StatsBucket* getStatsBucket(time_t startTime);
static void dumpStats2(void);

static void deleteRecord(void* record)
{
    _dpd.snortFree(record, sizeof(struct AppIdStatRecord),
           PP_APP_ID, PP_MEM_CATEGORY_MISC);
}

void appIdStatsUpdate(tAppIdData* session)
{
    struct StatsBucket* bucket = NULL;
    struct AppIdStatRecord * record = NULL;
    time_t now = time(NULL);
    time_t bucketTime;
    uint32_t app_id;
    tAppId web_app_id;
    tAppId service_app_id;
    tAppId client_app_id;

    if (!enableAppStats)
        return;

    now = now - (now % bucketInterval);
    if(now >= bucketEnd)
    {
        endStats2Period();
        dumpStats2();
        startStats2Period(now);
    }

    bucketTime = session->stats.firstPktsecond -
        (session->stats.firstPktsecond % bucketInterval);
    if((bucket = getStatsBucket(bucketTime)) == NULL)
        return;

    bucket->totalStats.txByteCnt += session->stats.initiatorBytes;
    bucket->totalStats.rxByteCnt += session->stats.responderBytes;


    web_app_id = pickPayloadId(session);
    if(web_app_id > APP_ID_NONE)
    {
        app_id = web_app_id;
        if(!(record = fwAvlLookup(app_id, bucket->appsTree)))
        {
            record = _dpd.snortAlloc(1, sizeof(*record), PP_APP_ID, PP_MEM_CATEGORY_MISC);
            if(record)
            {
                if (fwAvlInsert(app_id, record, bucket->appsTree) == 0)
                {
                    record->app_id = app_id;
                    bucket->appRecordCnt += 1;
#                   ifdef DEBUG_STATS
                    fprintf(SF_DEBUG_FILE, "New App: %u Count %u\n", record->app_id,
                            bucket->appRecordCnt);
#                   endif
                }
                else
                {
                    _dpd.snortFree(record, sizeof(*record), PP_APP_ID, PP_MEM_CATEGORY_MISC);
                    record = NULL;
                }
            }
        }

        if(record)
        {
            record->initiatorBytes += session->stats.initiatorBytes;
            record->responderBytes += session->stats.responderBytes;
        }
    }

    service_app_id = pickServiceAppId(session);
    if((service_app_id) &&
       (service_app_id != web_app_id))
    {
        app_id = service_app_id;

        if(!(record = fwAvlLookup(app_id, bucket->appsTree)))
        {
            record = _dpd.snortAlloc(1, sizeof(*record), PP_APP_ID, PP_MEM_CATEGORY_MISC);
            if(record)
            {
                if (fwAvlInsert(app_id, record, bucket->appsTree) == 0)
                {
                    record->app_id = app_id;
                    bucket->appRecordCnt += 1;
#                   ifdef DEBUG_STATS
                    fprintf(SF_DEBUG_FILE, "New App: %u Count %u\n", record->app_id,
                            bucket->appRecordCnt);
#                   endif
                }
                else
                {
                    _dpd.snortFree(record, sizeof(*record), PP_APP_ID, PP_MEM_CATEGORY_MISC);
                    record = NULL;
                }
            }
        }

        if(record)
        {
            record->initiatorBytes += session->stats.initiatorBytes;
            record->responderBytes += session->stats.responderBytes;
        }
    }

    client_app_id = pickClientAppId(session);
    if(client_app_id > APP_ID_NONE  
            && client_app_id != service_app_id 
            && client_app_id != web_app_id)
    {
        app_id = client_app_id;

        if(!(record = fwAvlLookup(app_id, bucket->appsTree)))
        {
            record = _dpd.snortAlloc(1, sizeof(*record), PP_APP_ID, PP_MEM_CATEGORY_MISC);
            if(record)
            {
                if (fwAvlInsert(app_id, record, bucket->appsTree) == 0)
                {
                    record->app_id = app_id;
                    bucket->appRecordCnt += 1;
#                   ifdef DEBUG_STATS
                    fprintf(SF_DEBUG_FILE, "New App: %u Count %u\n", record->app_id,
                            bucket->appRecordCnt);
#                   endif
                }
                else
                {
                    _dpd.snortFree(record, sizeof(*record), PP_APP_ID, PP_MEM_CATEGORY_MISC);
                    record = NULL;
                }
            }
        }

        if(record)
        {
            record->initiatorBytes += session->stats.initiatorBytes;
            record->responderBytes += session->stats.responderBytes;
        }
    }
}

void appIdStatsInit(char* appFileName, time_t statsPeriod, size_t rolloverSize, time_t rolloverPeriod)
{
    time_t now;
    size_t pathLength;
    char *path;

    if (!appFileName || !*appFileName)
    {
        enableAppStats = false;
        return;
    }
    enableAppStats = true;

    path = _dpd.getLogDirectory();
    rollPeriod = rolloverPeriod;
    rollSize = rolloverSize;

    pathLength = strlen(path) + strlen(appFileName) + 2;
    appFilePath = calloc(pathLength, 1);
    if(appFilePath != NULL)
        snprintf(appFilePath, pathLength, "%s/%s", path, appFileName);

    free(path);

    bucketInterval = statsPeriod;
    now = time(NULL);
    now = now - (now % bucketInterval);
    startStats2Period(now);
    appfp = NULL;
}

static void appIdStatsCloseFiles(void)
{
    if(appfp)
    {
        fclose(appfp);
        appfp = NULL;
    }

}

void appIdStatsReinit(void)
{
    if (!enableAppStats)
        return;
    appIdStatsCloseFiles();
}

void appIdStatsIdleFlush(void)
{
    time_t now;

    if (!enableAppStats)
        return;
    
    now = time(NULL);
    now = now - (now % bucketInterval);
    if(now >= bucketEnd)
    {
        endStats2Period();
        dumpStats2();
        startStats2Period(now);
    }
}

static void startStats2Period(time_t startTime)
{
    bucketStart = startTime;
    bucketEnd = bucketStart + bucketInterval;
}

static void endStats2Period(void)
{
    SF_LIST* bucketList = logBuckets;
    logBuckets = currBuckets;
    currBuckets = bucketList;
}

static struct StatsBucket* getStatsBucket(time_t startTime)
{
    struct StatsBucket* bucket = NULL;
    struct StatsBucket* lBucket;
    SF_LNODE* lNode;

    if(currBuckets == NULL)
    {
        currBuckets = sflist_new();
#       ifdef DEBUG_STATS
        fprintf(SF_DEBUG_FILE, "New Stats Bucket List\n");
#       endif
    }

    if(currBuckets == NULL)
        return(NULL);

    for(lNode = sflist_first_node(currBuckets); lNode != NULL;
            lNode = sflist_next_node(currBuckets))
    {
        lBucket = SFLIST_NODE_TO_DATA(lNode);

        if(startTime == lBucket->startTime)
        {
            bucket = lBucket;
            break;
        }
        else if(startTime < lBucket->startTime)
        {
            bucket = (struct StatsBucket*)_dpd.snortAlloc(1, sizeof(*bucket),
                     PP_APP_ID, PP_MEM_CATEGORY_MISC);
            if(bucket != NULL)
            {
                bucket->startTime = startTime;
                bucket->appsTree = fwAvlInit();

                sflist_add_before(currBuckets, lNode, bucket);
#               ifdef DEBUG_STATS
                fprintf(SF_DEBUG_FILE, "New Bucket Time: %u before %u\n",
                        bucket->startTime, lBucket->startTime);
#               endif
            }
            break;
        }
    }

    if(lNode == NULL)
    {
        bucket = (struct StatsBucket*)_dpd.snortAlloc(1, sizeof(*bucket),
                 PP_APP_ID, PP_MEM_CATEGORY_MISC);
        if(bucket != NULL)
        {
            bucket->startTime = startTime;
            bucket->appsTree = fwAvlInit();


            sflist_add_tail(currBuckets, bucket);
#           ifdef DEBUG_STATS
            fprintf(SF_DEBUG_FILE, "New Bucket Time: %u at tail\n", bucket->startTime);
#           endif
        }
    }

    return(bucket);
}

static void dumpStats2(void)
{
    struct StatsBucket* bucket = NULL;
    uint8_t*  buffer;
    uint32_t* buffPtr;
    struct    FwAvlNode* node;
    struct AppIdStatRecord* record;
    size_t    buffSize;
    time_t    currTime = time(NULL);

    if(logBuckets == NULL)
        return;

    while((bucket = (struct StatsBucket*) sflist_remove_head(logBuckets)) != NULL)
    {
        if(bucket->appRecordCnt)
        {
            buffSize = bucket->appRecordCnt * sizeof(struct AppIdStatOutputRecord) +
                            4 * sizeof(uint32_t);
            header.type = UNIFIED2_IDS_EVENT_APPSTAT;
            header.length = buffSize - 2*sizeof(uint32_t);
            buffer = malloc(buffSize);
            if(!buffer)
            {
                _dpd.errMsg("dumpStats2: "
                        "Failed to allocate memory for appRecord in StatsBucket\n");
                return;
            }
#           ifdef DEBUG_STATS
            fprintf(SF_DEBUG_FILE, "Write App Records %u Size: %lu\n",
                    bucket->appRecordCnt, buffSize);
#           endif
        }
        else
        {
            buffer = NULL;
        }

        if(buffer)
        {
            buffPtr = (uint32_t*) buffer;
            *buffPtr++ = htonl(header.type);
            *buffPtr++ = htonl(header.length);
            *buffPtr++ = htonl(bucket->startTime);
            *buffPtr++ = htonl(bucket->appRecordCnt);

            for(node = fwAvlFirst(bucket->appsTree); node != NULL; node = fwAvlNext(node))
            {
                struct AppIdStatOutputRecord *recBuffPtr;
                const char *appName;
                bool cooked_client = false;
                tAppId app_id;
                char tmpBuff[MAX_EVENT_APPNAME_LEN];

                record = (struct AppIdStatRecord *) node->data;
                app_id = record->app_id;

                recBuffPtr = (struct AppIdStatOutputRecord*)buffPtr;

                if (app_id >= 2000000000)
                {
                    cooked_client = true;
                    app_id -= 2000000000;
                }

                AppInfoTableEntry* entry = appInfoEntryGet(app_id, appIdActiveConfigGet());
                if (entry)
                {
                    appName = entry->appName;
                    if (cooked_client)
                    {

                        snprintf(tmpBuff, MAX_EVENT_APPNAME_LEN, "_cl_%s",appName);
                        tmpBuff[MAX_EVENT_APPNAME_LEN-1] = 0;
                        appName = tmpBuff;
                    }
                }
                else if (app_id == APP_ID_UNKNOWN || app_id == APP_ID_UNKNOWN_UI)
                    appName = "__unknown";
                else if (app_id == APP_ID_NONE)
                    appName = "__none";
                else
                {
                    _dpd.errMsg("invalid appid in appStatRecord (%u)\n", record->app_id);
                    if (cooked_client)
                    {
                        snprintf(tmpBuff, MAX_EVENT_APPNAME_LEN, "_err_cl_%u",app_id);
                    }
                    else
                    {
                        snprintf(tmpBuff, MAX_EVENT_APPNAME_LEN, "_err_%u",app_id); // ODP out of sync?
                    }
                    tmpBuff[MAX_EVENT_APPNAME_LEN-1] = 0;
                    appName = tmpBuff;
                }

                memcpy(recBuffPtr->appName, appName, MAX_EVENT_APPNAME_LEN);

                /**buffPtr++ = htonl(record->app_id); */
                recBuffPtr->initiatorBytes = htonl(record->initiatorBytes);
                recBuffPtr->responderBytes = htonl(record->responderBytes);

                buffPtr += sizeof(*recBuffPtr)/sizeof(*buffPtr);
            }

            if(appFilePath)
            {
                if(!appfp)
                {
                    appfp = openOutputFile(appFilePath, currTime);
                    appTime = currTime;
                    appSize = 0;
                }
                else if(((currTime - appTime) > rollPeriod) ||
                        ((appSize + buffSize) > rollSize))
                {
                    appfp = rolloverOutputFile(appFilePath, appfp, currTime);
                    appTime = currTime;
                    appSize = 0;
                }
                if(appfp)
                {
                    if((fwrite(buffer, buffSize, 1, appfp) == 1) && (fflush(appfp) == 0))
                    {
                        appSize += buffSize;
                    }
                    else
                    {
                        _dpd.errMsg("NGFW Rule Engine Failed to write to statistics file (%s): %s\n", appFilePath, strerror(errno));
                        fclose(appfp);
                        appfp = NULL;
                    }
                }
            }
            free(buffer);
        }
        fwAvlDeleteTree(bucket->appsTree, deleteRecord);
        _dpd.snortFree(bucket, sizeof(*bucket), PP_APP_ID, PP_MEM_CATEGORY_MISC);
    }
}

void appIdStatsFini()
{
    struct StatsBucket* bucket = NULL;

    if (!enableAppStats)
        return;

    /*flush the last stats period. */
    endStats2Period();
    dumpStats2();

    if(!currBuckets)
        return;
    
    while((bucket = (struct StatsBucket*) sflist_remove_head(currBuckets)) != NULL)
    {
       fwAvlDeleteTree(bucket->appsTree, deleteRecord);
       _dpd.snortFree(bucket, sizeof(*bucket), PP_APP_ID, PP_MEM_CATEGORY_MISC);
    }
    free(currBuckets);
    if(logBuckets)
        free(logBuckets);
    if(appFilePath)
        free(appFilePath);
    appIdStatsCloseFiles();
}

