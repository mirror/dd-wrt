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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <syslog.h>
#include <strings.h>
#include <sys/time.h>

#include "sfxhash.h"
#include "sf_dynamic_preprocessor.h"
#include "service_state.h"
#include "service_api.h"
#include "service_base.h"

/*#define DEBUG_SERVICE_STATE 1 */

static SFXHASH *serviceStateCache4;
static SFXHASH *serviceStateCache6;

#define SERVICE_STATE_CACHE_ROWS    65537

static int AppIdServiceStateFree(void *key, void *data)
{
    AppIdServiceIDState* id_state = (AppIdServiceIDState*)data;
    if (id_state->serviceList)
    {
        AppIdFreeServiceMatchList(id_state->serviceList);
        id_state->serviceList = NULL;
    }

    return 0;
}

int AppIdServiceStateInit(unsigned long memcap)
{
    serviceStateCache4 = sfxhash_new(-SERVICE_STATE_CACHE_ROWS,
                             sizeof(AppIdServiceStateKey4),
                             sizeof(AppIdServiceIDState),
                             memcap >> 1,
                             1,
                             &AppIdServiceStateFree,
                             &AppIdServiceStateFree,
                             1);
    if (!serviceStateCache4)
    {
        _dpd.errMsg( "Failed to allocate a hash table");
        return -1;
    }
    serviceStateCache6 = sfxhash_new(-SERVICE_STATE_CACHE_ROWS,
                             sizeof(AppIdServiceStateKey6),
                             sizeof(AppIdServiceIDState),
                             memcap >> 1,
                             1,
                             &AppIdServiceStateFree,
                             &AppIdServiceStateFree,
                             1);
    if (!serviceStateCache6)
    {
        _dpd.errMsg( "Failed to allocate a hash table");
        return -1;
    }
    return 0;
}

void AppIdServiceStateCleanup(void)
{
    if (serviceStateCache4)
    {
        sfxhash_delete(serviceStateCache4);
        serviceStateCache4 = NULL;
    }
    if (serviceStateCache6)
    {
        sfxhash_delete(serviceStateCache6);
        serviceStateCache6 = NULL;
    }
}

void AppIdRemoveServiceIDState(snort_ip *ip, uint16_t proto, uint16_t port)
{
    AppIdServiceStateKey k;
    SFXHASH *cache;

    if (ip->family == AF_INET6)
    {
        k.key6.proto = proto;
        k.key6.port = port;
        memcpy(k.key6.ip, ip->ip8, sizeof(k.key6.ip));
        cache = serviceStateCache6;
    }
    else
    {
        k.key4.proto = proto;
        k.key4.port = port;
        k.key4.ip = ip->ip32[0];
        cache = serviceStateCache4;
    }
    if (sfxhash_remove(cache, &k) != SFXHASH_OK)
    {
        char ipstr[INET6_ADDRSTRLEN];

        ipstr[0] = 0;
        inet_ntop(ip->family, (void *)ip->ip32, ipstr, sizeof(ipstr));
        _dpd.errMsg("Failed to remove from hash: %s:%u:%u\n",ipstr, (unsigned)proto, (unsigned)port);
    }
}

AppIdServiceIDState* AppIdGetServiceIDState(snort_ip *ip, uint16_t proto, uint16_t port)
{
    AppIdServiceStateKey k;
    SFXHASH *cache;
    AppIdServiceIDState* ss;

    if (ip->family == AF_INET6)
    {
        k.key6.proto = proto;
        k.key6.port = port;
        memcpy(k.key6.ip, ip->ip8, sizeof(k.key6.ip));
        cache = serviceStateCache6;
    }
    else
    {
        k.key4.proto = proto;
        k.key4.port = port;
        k.key4.ip = ip->ip32[0];
        cache = serviceStateCache4;
    }
    ss = sfxhash_find(cache, &k);

#ifdef DEBUG_SERVICE_STATE
    char ipstr[INET6_ADDRSTRLEN];

    ipstr[0] = 0;
    inet_ntop(ip->family, (void *)ip->ip32, ipstr, sizeof(ipstr));
    _dpd.logMsg("Read from hash: %s:%u:%u %p %u %p\n",ipstr, (unsigned)proto, (unsigned)port, ss, ss ? ss->state:0, ss ? ss->svc:NULL);
#endif

    if (ss && ss->svc && !ss->svc->ref_count)
    {
        ss->svc = NULL;
        ss->state = SERVICE_ID_NEW;
    }

    return ss;
}

AppIdServiceIDState* AppIdAddServiceIDState(snort_ip *ip, uint16_t proto, uint16_t port)
{
    AppIdServiceStateKey k;
    AppIdServiceIDState *ss;
    SFXHASH *cache;
    char ipstr[INET6_ADDRSTRLEN];

    if (ip->family == AF_INET6)
    {
        k.key6.proto = proto;
        k.key6.port = port;
        memcpy(k.key6.ip, ip->ip8, sizeof(k.key6.ip));
        cache = serviceStateCache6;
    }
    else
    {
        k.key4.proto = proto;
        k.key4.port = port;
        k.key4.ip = ip->ip32[0];
        cache = serviceStateCache4;
    }
#ifdef DEBUG_SERVICE_STATE
    ipstr[0] = 0;
    inet_ntop(ip->family, (void *)ip->ip32, ipstr, sizeof(ipstr));
#endif
    if (sfxhash_add_return_data_ptr(cache, &k, (void **)&ss))
    {
        ipstr[0] = 0;
        inet_ntop(ip->family, (void *)ip->ip32, ipstr, sizeof(ipstr));
        _dpd.errMsg("Failed to add to hash: %s:%u:%u\n",ipstr, (unsigned)proto, (unsigned)port);
        return NULL;
    }
#ifdef DEBUG_SERVICE_STATE
    _dpd.logMsg("Added to hash: %s:%u:%u %p\n",ipstr, (unsigned)proto, (unsigned)port, ss);
#endif
    return ss;
}

void AppIdServiceStateDumpStats(void)
{
    _dpd.logMsg("Service State:\n");
    if (serviceStateCache4)
    {
        _dpd.logMsg("           IPv4 Count: %u\n", sfxhash_count(serviceStateCache4));
        _dpd.logMsg("    IPv4 Memory Limit: %u\n", serviceStateCache4->mc.memcap);
        _dpd.logMsg("     IPv4 Memory Used: %u\n", serviceStateCache4->mc.memused);
    }
    if (serviceStateCache6)
    {
        _dpd.logMsg("           IPv6 Count: %u\n", sfxhash_count(serviceStateCache6));
        _dpd.logMsg("    IPv6 Memory Limit: %u\n", serviceStateCache6->mc.memcap);
        _dpd.logMsg("     IPv6 Memory Used: %u\n", serviceStateCache6->mc.memused);
    }
}

