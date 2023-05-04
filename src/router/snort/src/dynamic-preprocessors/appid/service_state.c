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
//#define DEBUG_APPID_MEMCAP_PRUNING 1

STATIC SFXHASH *serviceStateCache4;
STATIC SFXHASH *serviceStateCache6;

#define SERVICE_STATE_CACHE_ROWS    65536


bool AppIdServiceStateReloadAdjust(bool idle, unsigned long memcap)
{
    unsigned max_work = idle ? 512 : 8;
    static bool adjustStart = true;
    static unsigned numIpv4Entries = 0;
    static unsigned numIpv6Entries = 0;
    static unsigned numIpv4EntriesPruned = 0;
    static unsigned numIpv6EntriesPruned = 0;
    static unsigned ipv4MemUsed = 0;
    static unsigned ipv6MemUsed = 0;
    unsigned target = max_work;

    memcap >>= 1;

    if (adjustStart)
    {
        adjustStart = false;
        numIpv4Entries = sfxhash_count(serviceStateCache4);
        numIpv4EntriesPruned = 0;
        ipv4MemUsed = serviceStateCache4->mc.memused;
        numIpv6Entries = sfxhash_count(serviceStateCache6);
        numIpv6EntriesPruned = 0;
        ipv6MemUsed = serviceStateCache6->mc.memused;
    }

#ifdef DEBUG_APPID_MEMCAP_PRUNING
    if (memcap < serviceStateCache4->mc.memused)
    {
        unsigned count = sfxhash_count(serviceStateCache4);
        _dpd.logMsg("AppId: IPv4 cache mem used = %u, num entries = %u, pruning up to %u entries\n",
                    serviceStateCache4->mc.memused, count, (max_work < count) ? max_work : count);
    }
#endif // DEBUG_APPID_MEMCAP_PRUNING

    if (SFXHASH_OK != sfxhash_change_memcap(serviceStateCache4, memcap, &max_work))
    {
        numIpv4EntriesPruned += (target - max_work);
        return false;
    }

    numIpv4EntriesPruned += (target - max_work);
    if (target != max_work)
    {
        _dpd.logMsg("AppId: IPv4 cache pruning done - initial mem used = %u, initial entries = %u, pruned %u entries, current mem used = %u\n",
                    ipv4MemUsed, numIpv4Entries, numIpv4EntriesPruned, serviceStateCache4->mc.memused);
        target = max_work;
    }

#ifdef DEBUG_APPID_MEMCAP_PRUNING
    if (memcap < serviceStateCache6->mc.memused)
    {
        unsigned count = sfxhash_count(serviceStateCache6);
        _dpd.logMsg("AppId: IPv6 cache mem used = %u, num entries = %u, pruning up to %u entries\n",
                    serviceStateCache6->mc.memused, count, (max_work < count) ? max_work : count);
    }
#endif // DEBUG_APPID_MEMCAP_PRUNING

    if (SFXHASH_OK != sfxhash_change_memcap(serviceStateCache6, memcap, &max_work))
    {
        numIpv6EntriesPruned += (target - max_work);
        return false;
    }

    numIpv6EntriesPruned += (target - max_work);

    if (numIpv4EntriesPruned == 0)
        _dpd.logMsg("AppId: IPv4 cache pruning done - initial mem used = %u, initial entries = %u, pruned %u entries, current mem used = %u\n",
                    ipv4MemUsed, numIpv4Entries, numIpv4EntriesPruned, serviceStateCache4->mc.memused);
    _dpd.logMsg("AppId: IPv6 cache pruning done - initial mem used = %u, initial entries = %u, pruned %u entries, current mem used = %u\n",
                ipv6MemUsed, numIpv6Entries, numIpv6EntriesPruned, serviceStateCache6->mc.memused);

    adjustStart = true;
    return true;
}

int AppIdServiceStateInit(unsigned long memcap)
{
    serviceStateCache4 = sfxhash_new(SERVICE_STATE_CACHE_ROWS,
                             sizeof(AppIdServiceStateKey4),
                             sizeof(AppIdServiceIDState),
                             memcap >> 1,
                             1,
                             NULL,
                             NULL,
                             1);
    if (!serviceStateCache4)
    {
        _dpd.errMsg( "Failed to allocate a hash table");
        return -1;
    }
    serviceStateCache6 = sfxhash_new(SERVICE_STATE_CACHE_ROWS,
                             sizeof(AppIdServiceStateKey6),
                             sizeof(AppIdServiceIDState),
                             memcap >> 1,
                             1,
                             NULL,
                             NULL,
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
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
void AppIdRemoveServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level,
                               uint16_t asId, uint32_t cid)
#else
void AppIdRemoveServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level, uint32_t cid)
#endif
#else /* No carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
void AppIdRemoveServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level,
                               uint16_t asId)
#else
void AppIdRemoveServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level)
#endif
#endif
{
    AppIdServiceStateKey k;
    SFXHASH *cache;

    if (sfaddr_family(ip) == AF_INET6)
    {
        k.key6.proto = proto;
        k.key6.port = port;
        memcpy(k.key6.ip, sfaddr_get_ip6_ptr(ip), sizeof(k.key6.ip));
        k.key6.level = level;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        k.key6.asId = asId;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        k.key6.cid = cid;
#endif
        cache = serviceStateCache6;
    }
    else
    {
        k.key4.proto = proto;
        k.key4.port = port;
        k.key4.ip = sfaddr_get_ip4_value(ip);
        k.key4.level = level;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        k.key4.asId = asId;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        k.key4.cid = cid;
#endif
        cache = serviceStateCache4;
    }
    if (sfxhash_remove(cache, &k) != SFXHASH_OK)
    {
        char ipstr[INET6_ADDRSTRLEN];

        ipstr[0] = 0;
        inet_ntop(sfaddr_family(ip), (void *)sfaddr_get_ptr(ip), ipstr, sizeof(ipstr));
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        _dpd.errMsg("Failed to remove from hash: %s:%u:%u:%u:%u\n", ipstr, (unsigned)proto,
                     (unsigned)port, asId, cid);
#else
        _dpd.errMsg("Failed to remove from hash: %s:%u:%u:%u\n",ipstr, (unsigned)proto, (unsigned)port, cid);
#endif
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        _dpd.errMsg("Failed to remove from hash: %s:%u:%u:%u\n", ipstr, (unsigned)proto,
                     (unsigned)port, asId);
#else
        _dpd.errMsg("Failed to remove from hash: %s:%u:%u\n",ipstr, (unsigned)proto, (unsigned)port);
#endif
#endif
    }
}

#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
AppIdServiceIDState* AppIdGetServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port,
                                            uint32_t level, uint16_t asId, uint32_t cid)
#else
AppIdServiceIDState* AppIdGetServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level, uint32_t cid)
#endif
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
AppIdServiceIDState* AppIdGetServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port,
                                            uint32_t level, uint16_t asId)
#else
AppIdServiceIDState* AppIdGetServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level)
#endif
#endif

{
    AppIdServiceStateKey k;
    SFXHASH *cache;
    AppIdServiceIDState* ss;

    if (sfaddr_family(ip) == AF_INET6)
    {
        k.key6.proto = proto;
        k.key6.port = port;
        memcpy(k.key6.ip, sfaddr_get_ip6_ptr(ip), sizeof(k.key6.ip));
        k.key6.level = level;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        k.key6.asId = asId;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        k.key6.cid = cid;
#endif
        cache = serviceStateCache6;
    }
    else
    {
        k.key4.proto = proto;
        k.key4.port = port;
        k.key4.ip = sfaddr_get_ip4_value(ip);
        k.key4.level = level;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        k.key4.asId = asId;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        k.key4.cid = cid;
#endif
        cache = serviceStateCache4;
    }
    ss = sfxhash_find(cache, &k);

#ifdef DEBUG_SERVICE_STATE
    char ipstr[INET6_ADDRSTRLEN];

    ipstr[0] = 0;
    inet_ntop(sfaddr_family(ip), (void *)sfaddr_get_ptr(ip), ipstr, sizeof(ipstr));
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    _dpd.logMsg("ServiceState: Read from hash: %s:%u:%u:%u:%u:%u %p %u %p\n",
                 ipstr, (unsigned)proto, (unsigned)port, level, asId, cid
                 ss, ss ? ss->state:0, ss ? ss->svc:NULL);
#else    
    _dpd.logMsg("ServiceState: Read from hash: %s:%u:%u:%u:%u %p %u %p\n",ipstr,
            (unsigned)proto, (unsigned)port, level, cid, ss, ss ? ss->state:0, ss ? ss->svc:NULL);
#endif
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)  
    _dpd.logMsg("ServiceState: Read from hash: %s:%u:%u:%u:%u %p %u %p\n",
                 ipstr, (unsigned)proto, (unsigned)port, level, asId,
                 ss, ss ? ss->state:0, ss ? ss->svc:NULL);
#else
    _dpd.logMsg("ServiceState: Read from hash: %s:%u:%u:%u %p %u %p\n",ipstr,
            (unsigned)proto, (unsigned)port, level, ss, ss ? ss->state:0, ss ? ss->svc:NULL);
#endif
#endif
#endif

    if (ss && ss->svc && !ss->svc->ref_count)
    {
        ss->svc = NULL;
        ss->state = SERVICE_ID_NEW;
    }

    return ss;
}
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
AppIdServiceIDState* AppIdAddServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, 
                                            uint32_t level, uint16_t asId, uint32_t cid)
#else
AppIdServiceIDState* AppIdAddServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level, uint32_t cid)
#endif
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
AppIdServiceIDState* AppIdAddServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port,
                                            uint32_t level, uint16_t asId)
#else
AppIdServiceIDState* AppIdAddServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level)
#endif
#endif
{
    AppIdServiceStateKey k;
    AppIdServiceIDState *ss = NULL;
    SFXHASH *cache;
    char ipstr[INET6_ADDRSTRLEN];

    if (sfaddr_family(ip) == AF_INET6)
    {
        k.key6.proto = proto;
        k.key6.port = port;
        memcpy(k.key6.ip, sfaddr_get_ip6_ptr(ip), sizeof(k.key6.ip));
        k.key6.level = level;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        k.key6.asId = asId;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        k.key6.cid = cid;
#endif
        cache = serviceStateCache6;
    }
    else
    {
        k.key4.proto = proto;
        k.key4.port = port;
        k.key4.ip = sfaddr_get_ip4_value(ip);
        k.key4.level = level;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        k.key4.asId = asId;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        k.key4.cid = cid;
#endif
        cache = serviceStateCache4;
    }
#ifdef DEBUG_SERVICE_STATE
    ipstr[0] = 0;
    inet_ntop(sfaddr_family(ip), (void *)sfaddr_get_ptr(ip), ipstr, sizeof(ipstr));
#endif
    if ((sfxhash_add_return_data_ptr(cache, &k, (void **)&ss) < 0) || !ss)
    {
        ipstr[0] = 0;
        inet_ntop(sfaddr_family(ip), (void *)sfaddr_get_ptr(ip), ipstr, sizeof(ipstr));
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        _dpd.errMsg("ServiceState: Failed to add to hash: %s:%u:%u:%u:%u:%u\n",ipstr, 
                     (unsigned)proto, (unsigned)port, level, asId, cid);
#else
        _dpd.errMsg("ServiceState: Failed to add to hash: %s:%u:%u:%u:%u\n",ipstr, 
                (unsigned)proto, (unsigned)port, level, cid);
#endif
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        _dpd.errMsg("ServiceState: Failed to add to hash: %s:%u:%u:%u:%u\n",ipstr,
                     (unsigned)proto, (unsigned)port, level, asId);
#else
        _dpd.errMsg("ServiceState: Failed to add to hash: %s:%u:%u:%u\n",ipstr,
                (unsigned)proto, (unsigned)port, level);
#endif
#endif
        return NULL;
    }
#ifdef DEBUG_SERVICE_STATE
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    _dpd.logMsg("ServiceState: Added to hash: %s:%u:%u:%u:%u %p\n", ipstr,
                 (unsigned)proto, (unsigned)port, level, ss, asId);
#else
    _dpd.logMsg("ServiceState: Added to hash: %s:%u:%u:%u %p\n", ipstr,
            (unsigned)proto, (unsigned)port, level, ss);
#endif
#endif
    if (ss)
        memset(ss, 0, sizeof(*ss));
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

