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

#ifndef SERVICE_CONFIG_H_
#define SERVICE_CONFIG_H_

/****************************** INCLUDES **************************************/

#include <stdint.h>
#include <appId.h>

#include "service_api.h"

#define RNA_SERVICE_MAX_PORT 65536

/********************************* TYPES **************************************/

struct RNAServiceElement;
struct RNAServiceValidationModule;

typedef struct
{
    uint8_t type;
    tAppId  appId;
    uint8_t *pattern;
    int     pattern_size;
} SSLCertPattern;

typedef struct _DetectorSSLCertPattern
{
    SSLCertPattern  *dpattern;
    struct          _DetectorSSLCertPattern *next;
} DetectorSSLCertPattern;

typedef struct
{
    DetectorSSLCertPattern  *DetectorSSLCertPatternList;
    DetectorSSLCertPattern  *DetectorSSLCnamePatternList;
    void                    *ssl_host_matcher;
    void                    *ssl_cname_matcher;
} tServiceSslConfig;

/*DNS host pattern structure*/
typedef struct
{
    uint8_t type;
    tAppId  appId;
    uint8_t *pattern;
    int     pattern_size;
} DNSHostPattern;

typedef struct _DetectorDNSHostPattern
{
    DNSHostPattern                  *dpattern;
    struct _DetectorDNSHostPattern  *next;
} DetectorDNSHostPattern;

typedef struct
{
    DetectorDNSHostPattern  *DetectorDNSHostPatternList;
    void                    *dns_host_host_matcher;
} tServiceDnsConfig;

typedef struct servicePatternData_
{
    struct servicePatternData_  *next;
    int                         position;
    unsigned                    size;
    struct RNAServiceElement    *svc;
} tServicePatternData;

typedef struct
{
    struct RNAServiceValidationModule  *active_service_list;       ///< List of all services (Lua and C)
    struct RNAServiceElement    *tcp_service_list;          ///< List of all TCP services (Lua and C)
    struct RNAServiceElement    *udp_service_list;          ///< List of all UDP services (Lua and C)
    struct RNAServiceElement    *udp_reversed_service_list; ///< List of all UDP reversed services (Lua and C)

    //list nodes are RNAServiceElement*.
    SF_LIST                     *tcp_services[RNA_SERVICE_MAX_PORT];
    SF_LIST                     *udp_services[RNA_SERVICE_MAX_PORT];
    SF_LIST                     *udp_reversed_services[RNA_SERVICE_MAX_PORT];

    void                        *tcp_patterns;
    tServicePatternData         *tcp_pattern_data;
    int                         tcp_pattern_count;
    void                        *udp_patterns;
    tServicePatternData         *udp_pattern_data;
    int                         udp_pattern_count;
} tServiceConfig;

#endif // SERVICE_CONFIG_H_
