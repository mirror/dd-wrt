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


#ifndef __HTTP_COMMON_H__
#define __HTTP_COMMON_H__

#include <sys/types.h>
#include <inttypes.h>
#include "sf_multi_mpse.h"
#include "sf_mlmp.h"
#include "commonAppMatcher.h"

typedef enum
{
    SINGLE,
    SKYPE_URL,
    SKYPE_VERSION,
    BT_ANNOUNCE,
    BT_OTHER,
/*  HOST_HEADER,
    CONTENT_TYPE_HEADER,
    SERVER_HEADER, */
    USER_AGENT_HEADER
} DHPSequence;


typedef struct {
    DHPSequence seq;
    tAppId service_id;
    tAppId client_app;
    tAppId payload;
    int pattern_size;
    uint8_t *pattern;
    tAppId   appId;
} DetectorHTTPPattern;

typedef struct HTTPListElementStruct
{
    DetectorHTTPPattern detectorHTTPPattern;
    struct HTTPListElementStruct* next;
} HTTPListElement;

#define APPL_VERSION_LENGTH   40

typedef struct
{
    uint32_t service_id;
    uint32_t client_app;
    uint32_t payload;
    tAppId   appId;
    tMlpPattern query;
} tUrlUserData;

typedef struct
{
    struct
    {
        tMlpPattern host;
        tMlpPattern path;
        tMlpPattern scheme;
    } patterns;

    tUrlUserData userData;

} DetectorAppUrlPattern;

typedef struct DetectorAppUrlListStruct
{
    DetectorAppUrlPattern **urlPattern;
    size_t                  usedCount;
    size_t                  allocatedCount;
} DetectorAppUrlList;


typedef struct _HttpPatternLists
{
    HTTPListElement* hostPayloadPatternList;
    HTTPListElement* urlPatternList;
    HTTPListElement* clientAgentPatternList;
    HTTPListElement* contentTypePatternList;
    DetectorAppUrlList appUrlList;
    DetectorAppUrlList RTMPUrlList;
} HttpPatternLists;

/**url parts extracted from http headers.
 * "http"
 */
typedef struct {
    tMlpPattern host;      /*from host header */
    tMlpPattern path;      /*from GET/POST request */
    tMlpPattern scheme;    /*hardcoded to "http:" */
    tMlpPattern query;     /*query match for version number */

} tUrlStruct;

typedef struct _HostUrlDetectorPattern {
    tMlpPattern host;
    tMlpPattern path;
    tMlpPattern query;
    uint32_t payload_id;
    uint32_t service_id;
    uint32_t client_id;
    tAppId  appId;
    DHPSequence seq;
    struct _HostUrlDetectorPattern *next;
} HostUrlDetectorPattern;

extern HttpPatternLists httpPatternLists;

extern tAppId getAppIdByHttpUrl( tUrlStruct *url, tUrlUserData **rnaData);
#endif

