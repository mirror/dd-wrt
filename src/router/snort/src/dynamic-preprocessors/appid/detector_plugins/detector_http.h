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

int getAppidByViaPattern(const u_int8_t *data, unsigned size, char *version);
int getHTTPHeaderLocation(const uint8_t *data, unsigned size, HttpId id, int *start, int *end, HeaderMatchedPatterns *hmp);
tAppId getAppIdFromUrl(char *host, char *url, char *payloadVersion, size_t payloadVersionLen,
                       char *referer, tAppId *clientAppId, tAppId *serviceAppId, 
                       tAppId *payloadAppId, tAppId *referredPayloadAppId, unsigned from_rtmp);
tAppId getAppidByContentType(const uint8_t *data, int size);
tAppId scan_header_x_working_with(const uint8_t *data, uint32_t size, char * version);
void identifyUserAgent(const u_int8_t *start, int size, tAppId *serviceAppId, tAppId *clientAppId, char *version);
void getServerVendorVersion(const uint8_t *data, int len, char *version, char *vendor, RNAServiceSubtype **subtype);
int ScanURLForClientApp(const u_int8_t *url, int size, tAppId *clientAppId, tAppId *serviceAppId, char *clientVersion);
int http_detector_finalize(void);
void http_detector_clean(void);

#endif

