/*
** Copyright (C) 2014-2017 Cisco and/or its affiliates. All rights reserved.
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


#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include <errno.h>
#include "sf_dynamic_preprocessor.h"
#include "hostPortAppCache.h"
#include "ip_funcs.h"
#include "sfxhash.h"
#include "appIdConfig.h"

#define HOST_PORT_DYNAMIC_CACHE_ROWS        2048
#define MAX_HOST_PORT_DYNAMIC_CACHE_ENTRIES 4096

static SFXHASH *hostPortCacheDynamic;
static uint16_t hostPortCacheDynamicVersion = 0;

void hostPortAppCacheInit(tAppIdConfig *pConfig)
{
    if (!(pConfig->hostPortCache = sfxhash_new(1024,
                                      sizeof(tHostPortKey),
                                      sizeof(tHostPortVal),
                                      0,
                                      0,
                                      NULL,
                                      NULL,
                                      0)))
    {
        _dpd.errMsg( "failed to allocate HostPort map");
    }
}

void hostPortAppCacheFini(tAppIdConfig *pConfig)
{
    if (pConfig->hostPortCache)
    {
        sfxhash_delete(pConfig->hostPortCache);
        pConfig->hostPortCache = NULL;
    }
}

tHostPortVal *hostPortAppCacheFind(const sfaddr_t *snort_ip, uint16_t port, uint16_t protocol, const tAppIdConfig *pConfig)
{
    tHostPortKey hk;
    tHostPortVal *hv;
    sfip_set_ip((sfaddr_t *)&hk.ip, snort_ip);
    hk.port = port;
    hk.proto = protocol;

    hv = (tHostPortVal*)sfxhash_find(pConfig->hostPortCache, &hk);
    return hv;
}

int hostPortAppCacheAdd(const struct in6_addr *ip, uint16_t port, uint16_t proto, APP_ID_TYPE type, tAppId appId, tAppIdConfig *pConfig)
{
    tHostPortKey hk;
    tHostPortVal hv;
    memcpy(&hk.ip, ip, sizeof(hk.ip));
    hk.port = port;
    hk.proto = proto;
    hv.appId = appId;
    hv.type = type;
    if (sfxhash_add(pConfig->hostPortCache, &hk, &hv))
    {
        return 0;
    }

    return 1;
}
void hostPortAppCacheDump(const tAppIdConfig *pConfig)
{
    SFXHASH_NODE * node;

   for (node = sfxhash_findfirst(pConfig->hostPortCache); 
         node;
         node = sfxhash_findnext(pConfig->hostPortCache))
   {
       char inet_buffer[INET6_ADDRSTRLEN];
       tHostPortKey *hk;
       tHostPortVal *hv;

       hk = (tHostPortKey *)node->key;
       hv = (tHostPortVal *)node->data;

       inet_ntop(AF_INET6, &hk->ip, inet_buffer, sizeof(inet_buffer));
       printf("\tip=%s, \tport %d, \tproto %d, \ttype=%u, \tappId=%d\n", inet_buffer, hk->port, hk->proto, (unsigned)(hv->type), hv->appId);
   }
}

void hostPortAppCacheDynamicInit()
{
    // number of entries * overhead per entry
    unsigned long maxmem = sfxhash_calc_maxmem(MAX_HOST_PORT_DYNAMIC_CACHE_ENTRIES, sizeof(tHostPortKey) + sizeof(tHostPortVal));

    // add in fixed cost of hash table
    maxmem += (sizeof(SFXHASH_NODE**) * HOST_PORT_DYNAMIC_CACHE_ROWS) + sizeof(long);

    if (!(hostPortCacheDynamic = sfxhash_new(HOST_PORT_DYNAMIC_CACHE_ROWS,
                                      sizeof(tHostPortKey),
                                      sizeof(tHostPortVal),
                                      maxmem,
                                      1,
                                      NULL,
                                      NULL,
                                      0)))
    {
        _dpd.errMsg( "failed to allocate Dynamic HostPort map");
    }
}

void hostPortAppCacheDynamicFini()
{
    if (hostPortCacheDynamic)
    {
        sfxhash_delete(hostPortCacheDynamic);
        hostPortCacheDynamic = NULL;
    }
}

tHostPortVal *hostPortAppCacheDynamicFind(const sfaddr_t *snort_ip, uint16_t port, uint16_t protocol)
{
    tHostPortKey hk;
    tHostPortVal *hv;
    sfip_set_ip((sfaddr_t *)&hk.ip, snort_ip);
    hk.port = port;
    hk.proto = protocol;

    hv = (tHostPortVal*)sfxhash_find(hostPortCacheDynamic, &hk);
    return hv;
}

int hostPortAppCacheDynamicAdd(const struct in6_addr *ip, uint16_t port, uint16_t proto, APP_ID_TYPE type, tAppId appId)
{
    int rval;
    tHostPortKey hk;
    tHostPortVal hv;
    memcpy(&hk.ip, ip, sizeof(hk.ip));
    hk.port = port;
    hk.proto = proto;
    hv.appId = appId;
    hv.type = type;

    rval = sfxhash_add(hostPortCacheDynamic, &hk, &hv);

    if (rval == SFXHASH_OK)
    {
        hostPortCacheDynamicVersion++;
        if (hostPortCacheDynamicVersion == 0)
            hostPortCacheDynamicVersion++;
    }

    if (rval != SFXHASH_OK && rval != SFXHASH_INTABLE)
    {
        return 0;
    }
    return 1;
}

void hostPortAppCacheDynamicDump()
{
    SFXHASH_NODE * node;

    for (node = sfxhash_findfirst(hostPortCacheDynamic);
         node;
         node = sfxhash_findnext(hostPortCacheDynamic))
    {
        char inet_buffer[INET6_ADDRSTRLEN];
        tHostPortKey *hk;
        tHostPortVal *hv;

        hk = (tHostPortKey *)node->key;
        hv = (tHostPortVal *)node->data;

        inet_ntop(AF_INET6, &hk->ip, inet_buffer, sizeof(inet_buffer));
        printf("\tip=%s, \tport %d, \tproto %d, \ttype=%u, \tappId=%d\n", inet_buffer, hk->port, hk->proto, (unsigned)(hv->type), hv->appId);
    }
}

void updateHostCacheVersion(uint16_t *session_version)
{
    *session_version = hostPortCacheDynamicVersion;
    return;
}

bool isHostCacheUpdated(uint16_t session_version)
{
    if(session_version != hostPortCacheDynamicVersion)
        return true;
    else
        return false;
}
