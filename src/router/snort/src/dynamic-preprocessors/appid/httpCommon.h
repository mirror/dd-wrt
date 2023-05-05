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


#ifndef __HTTP_COMMON_H__
#define __HTTP_COMMON_H__

#include <sys/types.h>
#include <inttypes.h>
#include "sf_multi_mpse.h"
#include "sf_mlmp.h"
#include "flow.h"

#define MAX_USERNAME_SIZE   64
#define MAX_URL_SIZE    65535

// These values are used in Lua code as raw numbers. Do NOT reassign new values.
typedef enum
{
    SINGLE = 0,
    SKYPE_URL = 1,
    SKYPE_VERSION = 2,
    BT_ANNOUNCE = 3,
    BT_OTHER = 4,
    USER_AGENT_HEADER = 5
/*  HOST_HEADER,
    CONTENT_TYPE_HEADER,
    SERVER_HEADER */
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

// These values are used in Lua code as raw numbers. Do NOT reassign new values.
#define APP_TYPE_SERVICE    0x1
#define APP_TYPE_CLIENT     0x2
#define APP_TYPE_PAYLOAD    0x4

// These values are used in Lua code as raw numbers. Do NOT reassign new values.
typedef enum {
    NO_ACTION,                              //0
    COLLECT_VERSION,                        //1
    EXTRACT_USER,                           //2
    REWRITE_FIELD,                          //3
    INSERT_FIELD,                           //4
    ALTERNATE_APPID,                        //5
    FUTURE_APPID_SESSION_SIP,               //6
    FUTURE_APPID_SESSION_DIP,               //7
    FUTURE_APPID_SESSION_SPORT,             //8
    FUTURE_APPID_SESSION_DPORT,             //9
    FUTURE_APPID_SESSION_PROTOCOL,          //10
    FUTURE_APPID_SESSION_CREATE,            //11
    HOLD_FLOW,                              //12
    GET_OFFSETS_FROM_REBUILT,               //13
    SEARCH_UNSUPPORTED,                     //14
    DEFER_TO_SIMPLE_DETECT,                 //15
    MAX_ACTION_TYPE = DEFER_TO_SIMPLE_DETECT,
} ActionType;

// These values are used in Lua code as raw numbers. Do NOT reassign new values.
typedef enum {
    // Request-side headers
    AGENT_PT,          // 0
    HOST_PT,           // 1
    REFERER_PT,        // 2
    URI_PT,            // 3
    COOKIE_PT,         // 4
    REQ_BODY_PT,       // 5
    // Response-side headers
    CONTENT_TYPE_PT,   // 6
    LOCATION_PT,       // 7
    BODY_PT,           // 8
    MAX_PATTERN_TYPE = BODY_PT,
    MAX_KEY_PATTERN = URI_PT,
} PatternType;

#define CHP_APPID_BITS_FOR_INSTANCE  7
#define CHP_APPID_INSTANCE_MAX (1 << CHP_APPID_BITS_FOR_INSTANCE)
#define CHP_APPIDINSTANCE_TO_ID(_appIdInstance) \
        (_appIdInstance >> CHP_APPID_BITS_FOR_INSTANCE)
#define CHP_APPIDINSTANCE_TO_INSTANCE(_appIdInstance) \
        (_appIdInstance & (CHP_APPID_INSTANCE_MAX-1))
/*
  NOTE: The following structures have a field called appIdInstance.
    The low-order CHP_APPID_BITS_FOR_INSTANCE bits of appIdInstance field are used
    for the instance value while the remaining bits are used for the appId, shifted left
    that same number of bits. The legacy value for older apis is generated with the
    macro below.
*/
#define CHP_APPID_SINGLE_INSTANCE(_appId) \
        ((_appId << CHP_APPID_BITS_FOR_INSTANCE) + (CHP_APPID_INSTANCE_MAX-1))

typedef struct _CHPApp {
    tAppId appIdInstance; // * see note above
    unsigned app_type_flags;
    int num_matches;
    int num_scans;
    int key_pattern_count;
    int key_pattern_length_sum;
    int ptype_scan_counts[NUMBER_OF_PTYPES];
    int ptype_req_counts[NUMBER_OF_PTYPES];
} CHPApp;

typedef struct _CHPAction {
    tAppId appIdInstance; // * see note above
    uint precedence; // order of creation
    int key_pattern;
    PatternType ptype;
    int psize;
    char *pattern;
    ActionType action;
    char *action_data;
    CHPApp *chpapp;
} CHPAction;

typedef struct _CHPListElement
{
    CHPAction chp_action;
    struct _CHPListElement *next;
} CHPListElement;

typedef struct _HttpPatternLists
{
    HTTPListElement* hostPayloadPatternList;
    HTTPListElement* urlPatternList;
    HTTPListElement* clientAgentPatternList;
    HTTPListElement* contentTypePatternList;
    CHPListElement*     chpList;
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
    uint32_t    payload_id;
    uint32_t    service_id;
    uint32_t    client_id;
    tAppId      appId;
    DHPSequence seq;
    struct _HostUrlDetectorPattern *next;
} HostUrlDetectorPattern;

typedef struct _HostUrlPatternsList
{
    HostUrlDetectorPattern *head;
    HostUrlDetectorPattern *tail;
} HostUrlPatternsList;

struct DetectorHttpConfig
{
    void *url_matcher;
    void *client_agent_matcher;
    void *via_matcher;
    void *hostUrlMatcher;
    void *RTMPHostUrlMatcher;
    void *header_matcher;
    void *content_type_matcher;
    void *field_matcher;

    // CHP matchers
    // TODO: Is there a need for these variables? They just point to the pointers in the
    // array chp_matchers[]. They are used only in the function http_detector_clean(). But
    // there we could easily traverse through the members of chp_matchers instead of using
    // these variables.
    void *chp_user_agent_matcher;
    void *chp_host_matcher;
    void *chp_referer_matcher;
    void *chp_uri_matcher;
    void *chp_cookie_matcher;
    void *chp_content_type_matcher;
    void *chp_location_matcher;
    void *chp_body_matcher;
    // TODO: chp_req_body_matcher is not being used anywhere in the code, should it be removed?
    void *chp_req_body_matcher;

    void *chp_matchers[MAX_PATTERN_TYPE+1];

    HostUrlPatternsList *hostUrlPatternsList;
};
typedef struct DetectorHttpConfig tDetectorHttpConfig;

extern tAppId getAppIdByHttpUrl( tUrlStruct *url, tUrlUserData **rnaData);
#endif

