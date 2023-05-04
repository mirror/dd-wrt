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


#ifndef _HOST_PORT_APP_CACHE_
#define _HOST_PORT_APP_CACHE_

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "appId.h"
#include "appIdApi.h"
#include "sf_snort_packet.h"

// Forward declaration for AppId config. Cannot include appIdConfig.h because of
// circular dependency
struct appIdConfig_;

typedef struct _tHostPortKey {
    struct in6_addr ip;
    uint16_t port;
    uint16_t proto;
} tHostPortKey;

typedef struct _tHostPortVal {
    tAppId appId;
    APP_ID_TYPE type;
} tHostPortVal;

void hostPortAppCacheDynamicInit();
void hostPortAppCacheDynamicFini();
tHostPortVal *hostPortAppCacheDynamicFind(const sfaddr_t *ip, uint16_t port, uint16_t proto);
int hostPortAppCacheDynamicAdd(const struct in6_addr *ip, uint16_t port, uint16_t proto, APP_ID_TYPE type, tAppId appId, bool sendUpdate);
void hostPortAppCacheDynamicDump();

void hostPortAppCacheInit(struct appIdConfig_ *pConfig);
void hostPortAppCacheFini(struct appIdConfig_ *pConfig);
tHostPortVal *hostPortAppCacheFind(const sfaddr_t *ip, uint16_t port, uint16_t proto, const struct appIdConfig_ *pConfig);
int hostPortAppCacheAdd(const struct in6_addr *ip, uint16_t port, uint16_t proto, APP_ID_TYPE type, tAppId appId, struct appIdConfig_ *pConfig);
void hostPortAppCacheDump(const struct appIdConfig_ *pConfig);

void updateHostCacheVersion(uint16_t *session_version);
bool isHostCacheUpdated(uint16_t session_version);

#ifdef SIDE_CHANNEL
int ConsumeSSHostCache(const uint8_t *buf, uint32_t len);
#endif

#endif
