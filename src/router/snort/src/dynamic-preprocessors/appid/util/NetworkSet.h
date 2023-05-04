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

#ifndef __NETWORK_SET_H__
#define __NETWORK_SET_H__

/* System includes */
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <sfutil.h>

#ifndef ULLONG_MAX
#   define ULLONG_MAX   18446744073709551615ULL
#endif

#define BYTE_SWAP_16(x) \
    ((uint16_t)((((uint16_t)(x) & 0xff00) >> 8) | \
                (((uint16_t)(x) & 0x00ff) << 8)))

#define BYTE_SWAP_32(x) \
    ((uint32_t)((((uint32_t)(x) & 0xff000000) >> 24) | \
                (((uint32_t)(x) & 0x00ff0000) >>  8) | \
                (((uint32_t)(x) & 0x0000ff00) <<  8) | \
                (((uint32_t)(x) & 0x000000ff) << 24)))

#define BYTE_SWAP_64(x) \
    ((uint64_t)((((uint64_t)(x) & 0xff00000000000000ULL) >> 56) | \
                (((uint64_t)(x) & 0x00ff000000000000ULL) >> 40) | \
                (((uint64_t)(x) & 0x0000ff0000000000ULL) >> 24) | \
                (((uint64_t)(x) & 0x000000ff00000000ULL) >>  8) | \
                (((uint64_t)(x) & 0x00000000ff000000ULL) <<  8) | \
                (((uint64_t)(x) & 0x0000000000ff0000ULL) << 24) | \
                (((uint64_t)(x) & 0x000000000000ff00ULL) << 40) | \
                (((uint64_t)(x) & 0x00000000000000ffULL) << 56)))

#if defined(WORDS_BIGENDIAN)
typedef struct _NSIPv6Addr
{
    uint64_t hi;
    uint64_t lo;
} NSIPv6Addr;
#else
typedef struct _NSIPv6Addr
{
    uint64_t lo;
    uint64_t hi;
} NSIPv6Addr;
#endif

//IPv6 address a must be in network order
#define NSIP_IS_ADDR_MULTICAST(a) \
    (IN6_IS_ADDR_MULTICAST(a) \
    || ((IN6_IS_ADDR_V4MAPPED(a) || IN6_IS_ADDR_V4COMPAT(a)) && (((__const uint32_t *) (a))[3] == 0xffffffff)))

static inline void NSIPv6PackIpv4(NSIPv6Addr *ipv6Addr, uint32_t ipv4Addr)
{
    ipv6Addr->hi = 0ULL;
    ipv6Addr->lo = (uint64_t)ipv4Addr | 0x0000FFFF00000000ULL;
}

static inline int NSIPv6UnpackIpv4(const NSIPv6Addr *ipv6Addr, uint32_t *ipv4Addr)
{
    if (!ipv6Addr->hi)
    {
        uint64_t lo = ipv6Addr->lo & 0xFFFFFFFF00000000ULL;
        if (!lo || lo == 0x0000FFFF00000000ULL)
        {
            *ipv4Addr = (uint32_t)ipv6Addr->lo;
            return 0;
        }
    }
    return -1;
}


static inline void NSIPv6AddrCopy(const NSIPv6Addr *src, NSIPv6Addr *dst)
{
    dst->hi = src->hi;
    dst->lo = src->lo;
}

static inline int NSIPv6AddrCompare(const NSIPv6Addr *a, const NSIPv6Addr *b)
{
    if (a->hi < b->hi)
        return -1;
    else if (a->hi > b->hi)
        return 1;
    if (a->lo < b->lo)
        return -1;
    else if (a->lo > b->lo)
        return 1;
    return 0;
}

#if defined(WORDS_BIGENDIAN)

#define NSIPv6AddrNtoH(ip6) do {} while(0)

#else

static inline void NSIPv6AddrNtoH(NSIPv6Addr *ip6)
{
    uint64_t tmp;

    tmp = BYTE_SWAP_64(ip6->hi);
    ip6->hi = BYTE_SWAP_64(ip6->lo);
    ip6->lo = tmp;
}

#endif

#if defined(WORDS_BIGENDIAN)

static inline void _NSIPv6AddrConv(const NSIPv6Addr *ip6, NSIPv6Addr *ip6h)
{
    ip6h->hi = ip6->hi;
    ip6h->lo = ip6->lo;
}

#else

static inline void _NSIPv6AddrConv(const NSIPv6Addr *ip6, NSIPv6Addr *ip6h)
{
    ip6h->hi = BYTE_SWAP_64(ip6->lo);
    ip6h->lo = BYTE_SWAP_64(ip6->hi);
}

#endif

static inline void NSIPv6AddrNtoHConv(const struct in6_addr *ip6, NSIPv6Addr *ip6h)
{
    _NSIPv6AddrConv((const NSIPv6Addr *)ip6, ip6h);
}

static inline void NSIPv6AddrHtoNConv(const NSIPv6Addr *ip6, struct in6_addr *ip6h)
{
    _NSIPv6AddrConv(ip6, (NSIPv6Addr *)ip6h);
}

#define NSIPv6AddrHtoN(ip6) NSIPv6AddrNtoH(ip6)

static inline void NSIPv6AddrInc(NSIPv6Addr *ip6)
{
    if (ip6->lo == ULLONG_MAX)
    {
        ip6->lo = 0;
        ip6->hi++;
    }
    else
        ip6->lo++;
}

static inline void NSIPv6AddrDec(NSIPv6Addr *ip6)
{
    if (!ip6->lo)
    {
        ip6->lo = ULLONG_MAX;
        ip6->hi--;
    }
    else
        ip6->lo--;
}

typedef struct _NSNetworkInfo
{
    unsigned id;
    unsigned netmask;
    int ip_not;
    unsigned type;
} NSNetworkInfo;

typedef struct _Network
{
    NSNetworkInfo info;
    uint32_t range_min;
    uint32_t range_max;
} Network;

typedef struct _Network6
{
    NSNetworkInfo info;
    NSIPv6Addr range_min;
    NSIPv6Addr range_max;
} Network6;

typedef struct _NetworkSet
{
    struct _NetworkSet *next;
    SF_LIST networks;
    SFXHASH *ids;
    Network **pnetwork;
    unsigned count;
    SF_LIST networks6;
    SFXHASH *ids6;
    Network6 **pnetwork6;
    unsigned count6;
} NetworkSet;

/**
 * Create a new network set
 */
int NetworkSet_New(struct _NetworkSet **network_set);

/**
 * Destroy a network set
 */
int NetworkSet_Destroy(struct _NetworkSet *network_set);

/**
 * Copy a network set
 */
NetworkSet *NetworkSet_Copy(NetworkSet *network_set);

/**
 * Add a network set to another network set
 */
int NetworkSet_AddSet(NetworkSet *dest_set, NetworkSet *src_set);

/**
 * Add a network to the set using cidr block notation
 */
int NetworkSet_AddCidrBlockEx(struct _NetworkSet *network_set, uint32_t ip,
        unsigned cidr_bits, int ip_not, unsigned id, unsigned type);

/**
 * Add a network to the set using cidr block notation
 */
int NetworkSet_AddCidrBlock6Ex(struct _NetworkSet *network_set, NSIPv6Addr *ip,
        unsigned cidr_bits, int ip_not, unsigned id, unsigned type);

/**
 * Add a network to the set using cidr block notation
 */
int NetworkSet_AddCidrBlock(struct _NetworkSet *network_set, uint32_t ip,
        unsigned cidr_bits, int ip_not, unsigned id);

/**
 * Add a network to the set using cidr block notation
 */
int NetworkSet_AddCidrBlock6(struct _NetworkSet *network_set, NSIPv6Addr *ip,
        unsigned cidr_bits, int ip_not, unsigned id);

/**
 * Add a network to the set using a range
 */
int NetworkSet_AddNetworkRangeEx(NetworkSet *network_set, uint32_t range_min,
        uint32_t range_max, unsigned cidr_bits, int ip_not, unsigned id, unsigned type);

/**
 * Add a network to the set using a range
 */
int NetworkSet_AddNetworkRange6Ex(NetworkSet *network_set, NSIPv6Addr *range_min,
        NSIPv6Addr *range_max, unsigned cidr_bits, int ip_not, unsigned id, unsigned type);

/**
 * Add a network to the set using a range
 */
int NetworkSet_AddNetworkRange(NetworkSet *network_set, uint32_t range_min,
        uint32_t range_max, unsigned cidr_bits, int ip_not, unsigned id);

/**
 * Add a network to the set using a range
 */
int NetworkSet_AddNetworkRange6(NetworkSet *network_set, NSIPv6Addr *range_min,
        NSIPv6Addr *range_max, unsigned cidr_bits, int ip_not, unsigned id);

/**
 * Add a network to the set using a range of all IPv6, excluding IPv4
 */
int NetworkSet_AddNetworkRangeOnlyIPv6(NetworkSet *network_set, int ip_not, unsigned id, unsigned type);

/**
 * Reduce the networks to a list of existing ranges
 */
int NetworkSet_Reduce(struct _NetworkSet *network_set);

/**
 * Print the network to the specified stream
 */
int NetworkSet_Fprintf(struct _NetworkSet *network_set, const char *prefix, FILE *stream);

/*
 * Test is the set contains the specied address
 */
static inline int NetworkSet_ContainsEx(NetworkSet *network_set, uint32_t ipaddr, unsigned *type)
{
    int low=0;
    int middle=0;
    int high=0;

    *type = 0;
    if(!network_set)
        return 0;
    if(!network_set->count)
        return 0;
    high = network_set->count - 1;
    if(ipaddr < network_set->pnetwork[low]->range_min || ipaddr > network_set->pnetwork[high]->range_max)
        return 0;
    while(low <= high)
    {
        middle = low + ((high - low)>>1);
        if(ipaddr < network_set->pnetwork[middle]->range_min)
            high = middle - 1;
        else if(ipaddr > network_set->pnetwork[middle]->range_max)
            low = middle + 1;
        else
        {
            *type = network_set->pnetwork[middle]->info.type;
            return 1;
        }
    }
    return 0;
}

/*
 * Test is the set contains the specied address
 */
static inline int NetworkSet_Contains6Ex(NetworkSet *network_set, NSIPv6Addr *ipaddr, unsigned *type)
{
    int low=0;
    int middle=0;
    int high=0;

    *type = 0;
    if(!network_set)
        return 0;
    if(!network_set->count6)
        return 0;
    high = network_set->count6 - 1;
    if(NSIPv6AddrCompare(ipaddr, &network_set->pnetwork6[low]->range_min) < 0 ||
       NSIPv6AddrCompare(ipaddr, &network_set->pnetwork6[high]->range_max) > 0)
    {
        return 0;
    }
    while(low <= high)
    {
        middle = low + ((high - low)>>1);
        if(NSIPv6AddrCompare(ipaddr, &network_set->pnetwork6[middle]->range_min) < 0)
            high = middle - 1;
        else if(NSIPv6AddrCompare(ipaddr, &network_set->pnetwork6[middle]->range_max) > 0)
            low = middle + 1;
        else
        {
            *type = network_set->pnetwork6[middle]->info.type;
            return 1;
        }
    }
    return 0;
}

/*
 * Test is the set contains the specied address
 */
static inline int NetworkSet_Contains(NetworkSet *network_set, uint32_t ipaddr)
{
    unsigned type;
    return NetworkSet_ContainsEx(network_set, ipaddr, &type);
}

/*
 * Test is the set contains the specied address
 */
static inline int NetworkSet_Contains6(NetworkSet *network_set, NSIPv6Addr *ipaddr)
{
    unsigned type;
    return NetworkSet_Contains6Ex(network_set, ipaddr, &type);
}

/**
 * Get a count of the number of networks in the set
 * */
static inline int NetworkSet_Count(NetworkSet *network_set, unsigned *count)
{
    if (!network_set || !count)
        return -1;

    *count = sflist_count(&network_set->networks);

    return 0;
}

/**
 * Get a count of the number of networks in the set
 * */
static inline int NetworkSet_Count6(NetworkSet *network_set, unsigned *count)
{
    if (!network_set || !count)
        return -1;

    *count = sflist_count(&network_set->networks6);

    return 0;
}

/**
 * Get a count of the number of networks in the set
 * */
static inline unsigned NetworkSet_CountEx(NetworkSet *network_set)
{
    if (!network_set)
        return 0;

    return sflist_count(&network_set->networks);
}

/**
 * Get a count of the number of networks in the set
 * */
static inline unsigned NetworkSet_Count6Ex(NetworkSet *network_set)
{
    if (!network_set)
        return 0;

    return sflist_count(&network_set->networks6);
}

#endif /* __NETWORK_SET_H__ */

