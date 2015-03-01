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
#include "sfghash.h"

static SFGHASH *hostPortCache;

typedef struct _tHostPortKey {
    struct in6_addr ip;
    uint16_t port;
    uint16_t proto;
} tHostPortKey;


void hostPortAppCacheInit()
{
    if (!(hostPortCache = sfghash_new(-2000,
                                      sizeof(tHostPortKey),
                                      0,
                                      NULL)))
    {
        _dpd.errMsg( "failed to allocate HostPort map");
    }
}

void hostPortAppCacheFini()
{
    if (hostPortCache)
    {
        sfghash_delete(hostPortCache);
        hostPortCache = NULL;
    }
}


tAppId hostPortAppCacheFind(const snort_ip *snort_ip, uint16_t port, uint16_t protocol)
{
    tHostPortKey hk;
    copySnortIpToIpv6Network(&hk.ip, snort_ip);
    hk.port = port;
    hk.proto = protocol;

    return  (tAppId)sfghash_find(hostPortCache, &hk);
}

int hostPortAppCacheAdd(const struct in6_addr *ip, uint16_t port, uint16_t proto, unsigned type, tAppId appId)
{
    tHostPortKey hk;
    memcpy(&hk.ip, ip, sizeof(hk.ip));
    hk.port = port;
    hk.proto = proto;
    if (sfghash_add(hostPortCache, &hk, (void*)appId))
    {
        return 0;
    }

    return 1;
}
void hostPortAppCacheDump(void)
{
    SFGHASH_NODE * node;

   for( node = sfghash_findfirst(hostPortCache); node; node = sfghash_findnext(hostPortCache) )
   {
      char inet_buffer[INET6_ADDRSTRLEN];
      tHostPortKey *key;

      key = (tHostPortKey *)node->key;
      inet_ntop(AF_INET6, &key->ip, inet_buffer, sizeof(inet_buffer));
      printf("\tip=%s, \tport %d, \tproto %d, \tappId=%d\n", inet_buffer, key->port, key->proto, (signed)(node->data));
   }
}
