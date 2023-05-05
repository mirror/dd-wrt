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


#ifndef __SERVICE_PATTERN_H__
#define __SERVICE_PATTERN_H__

#include "service_api.h"

extern tRNAServiceValidationModule pattern_service_mod;

typedef struct PortPatternNode
{
    tAppId         appId;
    unsigned char  protocol;
    unsigned short port;
    unsigned char *pattern;
    unsigned       length;
    int32_t        offset;
    char           *detectorName;
    struct PortPatternNode *next;

} tPortPatternNode;
struct _PATTERN_SERVICE;
typedef struct _PATTERN
{
    struct _PATTERN *next;
    unsigned length;
    int offset;
    uint8_t *data;
    struct _PATTERN_SERVICE *ps;
} Pattern;

typedef struct _PORT
{
    struct _PORT *next;
    uint16_t port;
} Port;

/**list for pattern services. Each pattern service is unique for a given uuid. */
typedef struct _PATTERN_SERVICE
{
    struct _PATTERN_SERVICE *next;
    tAppId id;
    Pattern *pattern;
    Port *port;
    unsigned proto;
    unsigned count;
    unsigned longest;
} PatternService;

typedef struct ServicePortPattern
{
    struct PortPatternNode        *luaInjectedPatterns;
    PatternService *servicePortPattern;
    void *tcp_patterns;
    void *udp_patterns;
    void *tcpPortPatternTree[65536];
    void *udpPortPatternTree[65536];
} tServicePortPattern;

typedef struct ClientPortPattern
{
    struct PortPatternNode        *luaInjectedPatterns;
    PatternService *servicePortPattern;
    void *tcp_patterns;
    void *udp_patterns;
} tClientPortPattern;

#endif  /* __SERVICE_PATTERN_H__ */

