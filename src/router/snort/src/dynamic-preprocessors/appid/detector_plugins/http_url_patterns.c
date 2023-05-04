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


#include "string.h"
#include "sf_multi_mpse.h"
#include "httpCommon.h"
#include "http_url_patterns.h"
#include "commonAppMatcher.h"

#define FP_OPERATION_AND   "%&%"
#define PATTERN_PART_MAX 10

static void destroyHostUrlDetectorPattern(HostUrlDetectorPattern *pattern)
{
    if (!pattern)
        return;

    destroyHostUrlDetectorPattern(pattern->next);

    if (pattern->host.pattern)
        free(*(void **)&pattern->host.pattern);
    if (pattern->path.pattern)
        free(*(void **)&pattern->path.pattern);
    if (pattern->query.pattern)
        free(*(void **)&pattern->query.pattern);
    free(pattern);
}

static int addHostUrlPatternToList(HostUrlDetectorPattern *detector, HostUrlPatternsList **hostUrlPatternsList)
{
    if (!detector)
        return -1;

    if (!(*hostUrlPatternsList))
    {
        if ((*hostUrlPatternsList = malloc(sizeof(HostUrlPatternsList))) == NULL)
                return -1;

        (*hostUrlPatternsList)->head = detector;
        (*hostUrlPatternsList)->tail = detector;
    }
    else
    {
        (*hostUrlPatternsList)->tail->next = detector;
        (*hostUrlPatternsList)->tail = detector;
    }

    return 0;
}

void destroyHostUrlPatternList(HostUrlPatternsList **pHostUrlPatternsList)
{
    if (!(*pHostUrlPatternsList))
        return;

    destroyHostUrlDetectorPattern((*pHostUrlPatternsList)->head);
    free(*pHostUrlPatternsList);
    *pHostUrlPatternsList = NULL;
}

int addMlmpPattern(void *hostUrlMatcher, HostUrlPatternsList **hostUrlPatternsList,
        const uint8_t *host_pattern, int host_pattern_size,
        const uint8_t *path_pattern, int path_pattern_size, const uint8_t *query_pattern, int query_pattern_size,
        tAppId appId, uint32_t payload_id, uint32_t service_id, uint32_t client_id, DHPSequence seq)
{
    static tMlmpPattern patterns[PATTERN_PART_MAX];
    int num_patterns;

    if (!host_pattern)
        return -1;

    if (!hostUrlMatcher)
        return -1;

    HostUrlDetectorPattern *detector = malloc(sizeof(*detector));
    if (!detector)
        return -1;

    detector->host.pattern = (uint8_t *)strdup((char *)host_pattern);
    if (!detector->host.pattern)
    {
        free(detector);
        return -1;
    }

    if (path_pattern)
    {
        detector->path.pattern = (uint8_t *)strdup((char *)path_pattern);
        if (!detector->path.pattern)
        {
            free((void*)detector->host.pattern);
            free(detector);
            return -1;
        }
    }
    else
    {
        detector->path.pattern = NULL;
    }

    if (query_pattern)
    {
        detector->query.pattern = (uint8_t *)strdup((char *)query_pattern);
        if (!detector->query.pattern)
        {
            free((void*)detector->host.pattern);
            free((void*)detector->path.pattern);
            free(detector);
            return -1;
        }
    }
    else
    {
        detector->query.pattern = NULL;
    }

    detector->host.patternSize = host_pattern_size;
    detector->path.patternSize = path_pattern_size;
    detector->query.patternSize = query_pattern_size;
    detector->payload_id = payload_id;
    detector->service_id = service_id;
    detector->client_id = client_id;
    detector->seq = seq;
    detector->next = NULL;
    if (appId > APP_ID_NONE)
        detector->appId = appId;
    else if (payload_id > APP_ID_NONE)
        detector->appId = payload_id;
    else if (client_id > APP_ID_NONE)
        detector->appId = client_id;
    else
        detector->appId = service_id;


    num_patterns = parseMultipleHTTPPatterns((const char *)host_pattern, patterns,  PATTERN_PART_MAX, 0);
    if (path_pattern)
        num_patterns += parseMultipleHTTPPatterns((const char *)path_pattern, patterns+num_patterns,  PATTERN_PART_MAX-num_patterns, 1);

    patterns[num_patterns].pattern = NULL;

    if (addHostUrlPatternToList(detector, hostUrlPatternsList))
        return -1;

    return mlmpAddPattern(hostUrlMatcher, patterns, detector);
}

u_int32_t parseMultipleHTTPPatterns(const char *pattern, tMlmpPattern *parts, u_int32_t numPartLimit, int level)
{
    u_int32_t partNum = 0;
    const char *tmp;
    char *tmp2;
    u_int32_t i;

    if (!pattern)
        return 0;

    tmp = pattern;
    while  (tmp && (partNum < numPartLimit))
    {
        tmp2 = strstr(tmp, FP_OPERATION_AND);
        if (tmp2)
        {
            parts[partNum].pattern = (uint8_t *)strndup(tmp, tmp2-tmp);
            if (parts[partNum].pattern)
            {
                parts[partNum].patternSize = strlen((const char*)parts[partNum].pattern);
                tmp = tmp2+strlen(FP_OPERATION_AND);
            }
        }
        else
        {
            parts[partNum].pattern = (uint8_t *)strdup(tmp);
            if (parts[partNum].pattern)
            {
                parts[partNum].patternSize = strlen((const char*)parts[partNum].pattern);
                tmp = NULL;
            }
        }
        parts[partNum].level = level;

        if (!parts[partNum].pattern)
        {
            for (i = 0; i <= partNum; i++)
                free((void*)parts[i].pattern);

            _dpd.errMsg( "Failed to allocate memory");
            return 0;
        }
        partNum++;
    }

    return partNum;
}

/**recursively destroy matcher.
 */
void destroyHostUrlMatcher(void **hostUrlMatcher)
{
    if (hostUrlMatcher && *hostUrlMatcher)
    {
        mlmpDestroy(*hostUrlMatcher);
        *hostUrlMatcher = NULL;
    }
}

int matchQueryElements(
        tMlpPattern *packetData,
        tMlpPattern *userPattern,
        char *appVersion,
        size_t appVersionSize
        )
{
    const u_int8_t *index, *endKey;
    const u_int8_t  *queryEnd;
    u_int32_t extractedSize;
    u_int32_t copySize = 0;

    if (appVersion == NULL)
        return 0;

    appVersion[0] = '\0';

    if (!userPattern->pattern || !packetData->pattern) return 0;

    /*queryEnd is 1 past the end. */
    queryEnd = packetData->pattern + packetData->patternSize;
    index = packetData->pattern;
    endKey = queryEnd;

    /*?key1=value1&key2=value2 */
    for (index = packetData->pattern; index < queryEnd; index = endKey+1)
    {
        /*find end of query tuple */
        endKey = memchr ( index, '&',  queryEnd - index );
        if (!endKey) endKey = queryEnd;

        if (userPattern->patternSize < (u_int32_t)(endKey-index))
        {
            if (memcmp(index, userPattern->pattern, userPattern->patternSize) == 0)
            {
                index += userPattern->patternSize;
                extractedSize = (endKey-index);
                appVersionSize--;
                copySize = (extractedSize < appVersionSize) ? extractedSize: appVersionSize;
                memcpy(appVersion, index, copySize);
                appVersion[copySize] = '\0';
                break;
            }
        }
    }
    return copySize;
}
