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


#ifndef __DETECTOR_HTTP_H__
#define __DETECTOR_HTTP_H__

#include "detector_api.h"

#define MAX_VERSION_SIZE    64

typedef enum
{
    /* Only store Content-Type, Server, User-Agent & Via headers now. */
    HTTP_ID_CONTENT_TYPE,
    HTTP_ID_SERVER,
    /* HTTP Responses */
    HTTP_ID_COPY,
    HTTP_ID_DELETE,
    HTTP_ID_GET,
    HTTP_ID_HEAD,
    HTTP_ID_OPTIONS,
    HTTP_ID_PROPFIND,
    HTTP_ID_PROPPATCH,
    HTTP_ID_MKCOL,
    HTTP_ID_LOCK,
    HTTP_ID_MOVE,
    HTTP_ID_PUT,
    HTTP_ID_POST,
    HTTP_ID_TRACE,
    HTTP_ID_UNLOCK,
    HTTP_ID_X_WORKING_WITH,
    /* When others are needed, move value to be above HTTP_ID_LEN */
    HTTP_ID_ACCEPT,
    HTTP_ID_ACCEPT_CHARSET,
    HTTP_ID_ACCEPT_ENCODING,
    HTTP_ID_ACCEPT_LANGUAGE,
    HTTP_ID_ACCEPT_RANGES,
    HTTP_ID_AGE,
    HTTP_ID_ALLOW,
    HTTP_ID_AUTHORIZATION,
    HTTP_ID_CACHE_CONTROL,
    HTTP_ID_CONNECTION,
    HTTP_ID_COOKIE,
    HTTP_ID_CONTENT_DISPOSITION,
    HTTP_ID_CONTENT_ENCODING,
    HTTP_ID_CONTENT_LANGUAGE,
    HTTP_ID_CONTENT_LENGTH,
    HTTP_ID_CONTENT_LOCATION,
    HTTP_ID_CONTENT_MD5,
    HTTP_ID_CONTENT_RANGE,
    HTTP_ID_DATE,
    HTTP_ID_ETAG,
    HTTP_ID_EXPECT,
    HTTP_ID_EXPIRES,
    HTTP_ID_FROM,
    HTTP_ID_HOST,
    HTTP_ID_IF_MATCH,
    HTTP_ID_IF_MODIFIED_SINCE,
    HTTP_ID_IF_NONE_MATCH,
    HTTP_ID_IF_RANGE,
    HTTP_ID_IF_UNMODIFIED_SINCE,
    HTTP_ID_LAST_MODIFIED,
    HTTP_ID_LINK,
    HTTP_ID_LOCATION,
    HTTP_ID_MAX_FORWARDS,
    HTTP_ID_P3P,
    HTTP_ID_PRAGMA,
    HTTP_ID_PROXY_AUTHORIZATION,
    HTTP_ID_PROXY_AUTHENTICATE,
    HTTP_ID_RANGE,
    HTTP_ID_REFERER,
    HTTP_ID_REFRESH,
    HTTP_ID_RETRY_AFTER,
    HTTP_ID_SET_COOKIE,
    HTTP_ID_STRICT_TRANSPORT_SECURITY,
    HTTP_ID_TE,
    HTTP_ID_TRAILER,
    HTTP_ID_TRANSFER_ENCODING,
    HTTP_ID_UPGRADE,
    HTTP_ID_USER_AGENT,
    HTTP_ID_VARY,
    HTTP_ID_VIA,
    HTTP_ID_WARNING,
    HTTP_ID_WWW_AUTHENTICATE,
    HTTP_ID_LEN
} HttpId;

typedef struct _HTTP_HEADER_INDICES
{
    int start;
    int end;
} HTTPHeaderIndices;

#if 1
typedef struct _HEADER_MATCHED_PATTERNS
{
    HTTPHeaderIndices headers[HTTP_ID_LEN];
    int last_match;
    int last_index_end;
    int searched;
} HeaderMatchedPatterns;
#endif

typedef struct _MatchedCHPAction {
    CHPAction *mpattern;
    int index;
    struct _MatchedCHPAction *next;
} MatchedCHPAction;

// This is an array element for the dynamically growing tally below
typedef struct _CHPMatchCandidate {
    CHPApp *chpapp;
    int key_pattern_length_sum;
    int key_pattern_countdown;
} CHPMatchCandidate;

// This is a structure which will grow using realloc as needed to keep all candidates
typedef struct _CHPMatchTally {
    int allocated_elements;
    int in_use_elements;
    CHPMatchCandidate item[0];
} CHPMatchTally;

int getAppidByViaPattern(const u_int8_t *data, unsigned size, char **version, const tDetectorHttpConfig *pHttpConfig);
int getHTTPHeaderLocation(const uint8_t *data, unsigned size, HttpId id, int *start, int *end, HeaderMatchedPatterns *hmp,
                          const tDetectorHttpConfig *pHttpConfig);

static inline void FreeMatchedCHPActions(MatchedCHPAction *ma)
{
    MatchedCHPAction *tmp;

    while(ma)
    {
        tmp = ma;
        ma = ma->next;
        free(tmp);
    }
}

void httpGetNewOffsetsFromPacket(SFSnortPacket *pkt, httpSession *hsession, tAppIdConfig *pConfig);

int scanKeyCHP (PatternType ptype, char *buf, int buf_size,
                CHPMatchTally **ppTally, MatchedCHPAction **ppmatches, const tDetectorHttpConfig *pHttpConfig);

tAppId scanCHP (PatternType ptype, char *buf, int buf_size, MatchedCHPAction *mp,
                char **version, char **user, char **new_field,
                int *total_found, httpSession *hsession, SFSnortPacket *p, const tDetectorHttpConfig *pHttpConfig);
tAppId getAppIdFromUrl(char *host, char *url, char **version,
                       char *referer, tAppId *clientAppId, tAppId *serviceAppId,
                       tAppId *payloadAppId, tAppId *referredPayloadAppId, unsigned from_rtmp,
                       const tDetectorHttpConfig *pHttpConfig);
tAppId getAppidByContentType(const uint8_t *data, int size, const tDetectorHttpConfig *pHttpConfig);
tAppId scan_header_x_working_with(const uint8_t *data, uint32_t size, char **version);
void identifyUserAgent(const u_int8_t *start, int size, tAppId *serviceAppId, tAppId *clientAppId, char **version,
                       const tDetectorHttpConfig *pHttpConfig);
void getServerVendorVersion(const uint8_t *data, int len, char **version, char **vendor, RNAServiceSubtype **subtype);
int webdav_found(HeaderMatchedPatterns *hmp);
int http_detector_finalize(tAppIdConfig *pConfig);
void http_detector_clean(tDetectorHttpConfig *pHttpConfig);
void finalizeFflow (fflow_info *fflow, unsigned app_type_flags, tAppId target_appId, SFSnortPacket *p);

#endif

