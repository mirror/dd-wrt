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

/* System includes */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#ifdef LINUX
#include <stdint.h>
#endif
#include "sf_dynamic_preprocessor.h"

/* Sourcefire includes */
#include "NetworkSet.h"

/* Local includes */

#define MODULE_NAME "NetworkSet"

int NetworkSet_New(NetworkSet **network_set)
{
    NetworkSet *tmp = NULL;

    if(!network_set)
        return -1;

    if(!(tmp = calloc(1, sizeof(*tmp))))
    {
        _dpd.errMsg("NetworkSet:Out of memory (wanted %zu bytes)", sizeof(NetworkSet));
        NetworkSet_Destroy(tmp);
        return -1;
    }

    sflist_init(&tmp->networks);
    tmp->ids = sfxhash_new(64, sizeof(unsigned), 0, 0, 0, NULL, NULL, 1);
    if (tmp->ids == NULL)
    {
        _dpd.errMsg("NetworkSet:Out of memory (wanted %zu bytes)", sizeof(NetworkSet));
        NetworkSet_Destroy(tmp);
        return -1;
    }

    sflist_init(&tmp->networks6);
    tmp->ids6 = sfxhash_new(64, sizeof(unsigned), 0, 0, 0, NULL, NULL, 1);
    if (tmp->ids6 == NULL)
    {
        _dpd.errMsg("NetworkSet:Out of memory (wanted %zu bytes)", sizeof(NetworkSet));
        NetworkSet_Destroy(tmp);
        return -1;
    }

    *network_set = tmp;

    return 0;
}

int NetworkSet_Destroy(NetworkSet *network_set)
{
    if(!network_set)
        return -1;

    if(network_set->pnetwork)
    {
        free(network_set->pnetwork);
        network_set->pnetwork = NULL;
    }
    sflist_static_free_all(&network_set->networks, &free);
    sfxhash_delete(network_set->ids);
    if(network_set->pnetwork6)
    {
        free(network_set->pnetwork6);
        network_set->pnetwork6 = NULL;
    }
    sflist_static_free_all(&network_set->networks6, &free);
    sfxhash_delete(network_set->ids6);
    free(network_set);

    return 0;
}

int NetworkSet_AddNetworkRangeEx(NetworkSet *network_set, uint32_t range_min,
        uint32_t range_max, unsigned cidr_bits, int ip_not, unsigned id, unsigned type)
{
    Network *network;
    Network *iNetwork;
    int rval;

    if(!network_set)
        return -1;

    if(!(network = (Network *)calloc(1, sizeof(*network))))
    {
        _dpd.errMsg("NetworkSet:Out of memory (wanted %zu bytes)", sizeof(*network));
        return -1;
    }

    network->info.id = id;
    network->info.ip_not = ip_not;
    network->info.type = type;
    network->info.netmask = cidr_bits;
    if (range_min <= range_max)
    {
        network->range_min = range_min;
        network->range_max = range_max;
    }
    else
    {
        network->range_min = range_max;
        network->range_max = range_min;
    }

    if (!network->info.ip_not)
    {
        for (iNetwork = sflist_first(&network_set->networks); iNetwork; iNetwork = sflist_next(&network_set->networks))
        {
            if (iNetwork->info.id == network->info.id &&
                iNetwork->range_min == network->range_min &&
                iNetwork->range_max == network->range_max)
            {
                iNetwork->info.type |= network->info.type;
                free(network);
                return 0;
            }
        }
    }

    if(sflist_add_tail(&network_set->networks, (void *)network))
    {
        _dpd.errMsg("NetworkSet:Out of memory");
        free(network);
        return -1;
    }

    rval = sfxhash_add(network_set->ids, &network->info.id, &network->info.id);
    if (rval != SFXHASH_OK && rval != SFXHASH_INTABLE)
    {
        _dpd.errMsg("NetworkSet:Out of memory");
        free(network);
        return -1;
    }

    return 0;
}

int NetworkSet_AddNetworkRange(NetworkSet *network_set, uint32_t range_min,
        uint32_t range_max, unsigned cidr_bits, int ip_not, unsigned id)
{
    return NetworkSet_AddNetworkRangeEx(network_set, range_min, range_max, cidr_bits, ip_not, id, 0);
}

int NetworkSet_AddNetworkRange6Ex(NetworkSet *network_set, NSIPv6Addr *range_min,
        NSIPv6Addr *range_max, unsigned cidr_bits, int ip_not, unsigned id, unsigned type)
{
    Network6 *network;
    Network6 *iNetwork;
    int rval;

    if(!network_set)
        return -1;

    if(!(network = (Network6 *)calloc(1, sizeof(*network))))
    {
        _dpd.errMsg("NetworkSet:Out of memory (wanted %zu bytes)", sizeof(*network));
        return -1;
    }

    network->info.id = id;
    network->info.ip_not = ip_not;
    network->info.type = type;
    network->info.netmask = cidr_bits;
    if (NSIPv6AddrCompare(range_min, range_max) <= 0)
    {
        network->range_min = *range_min;
        network->range_max = *range_max;
    }
    else
    {
        network->range_min = *range_max;
        network->range_max = *range_min;
    }

    if (!network->info.ip_not)
    {
        for (iNetwork = sflist_first(&network_set->networks6); iNetwork; iNetwork = sflist_next(&network_set->networks6))
        {
            if (iNetwork->info.id == network->info.id &&
                !NSIPv6AddrCompare(&iNetwork->range_min, &network->range_min) &&
                !NSIPv6AddrCompare(&iNetwork->range_max, &network->range_max))
            {
                iNetwork->info.type |= network->info.type;
                free(network);
                return 0;
            }
        }
    }

    if(sflist_add_tail(&network_set->networks6, (void *)network))
    {
        _dpd.errMsg("NetworkSet:Out of memory");
        free(network);
        return -1;
    }
    rval = sfxhash_add(network_set->ids6, &network->info.id, &network->info.id);
    if (rval != SFXHASH_OK && rval != SFXHASH_INTABLE)
    {
        _dpd.errMsg("NetworkSet:Out of memory");
        free(network);
        return -1;
    }

    return 0;
}

int NetworkSet_AddNetworkRange6(NetworkSet *network_set, NSIPv6Addr *range_min,
        NSIPv6Addr *range_max, unsigned cidr_bits, int ip_not, unsigned id)
{
    return NetworkSet_AddNetworkRange6Ex(network_set, range_min, range_max, cidr_bits, ip_not, id, 0);
}

int NetworkSet_AddNetworkRangeOnlyIPv6(NetworkSet *network_set, int ip_not, unsigned id, unsigned type)
{
    // Use two ranges to represent all of IPv6, excluding the IPv4-mapped range, ::FFFF:*.*.*.*
    int rval;
    NSIPv6Addr range_min, range_max;
    range_min.lo = 0;
    range_min.hi = 0;
    range_max.lo = 0x0000FFFEFFFFFFFFULL;  // 0x0000FFFF00000000 - 1
    range_max.hi = 0;
    rval = NetworkSet_AddNetworkRange6Ex(network_set, &range_min, &range_max, 0, ip_not, id, type);
    range_min.lo = 0x0001000000000000ULL;  // 0x0000FFFFFFFFFFFF + 1
    range_min.hi = 0;
    range_max.lo = 0xFFFFFFFFFFFFFFFFULL;
    range_max.hi = 0xFFFFFFFFFFFFFFFFULL;
    return rval ? rval : NetworkSet_AddNetworkRange6Ex(network_set, &range_min, &range_max, 0, ip_not, id, type);
}

static inline int NetworkSet_AddNetwork(NetworkSet *network_set, uint32_t ip,
        unsigned cidr_bits, uint32_t mask, int ip_not, unsigned id, unsigned type)
{
    uint32_t range_min;
    uint32_t range_max;

    range_min = ip & mask;
    range_max = range_min + ~mask;
    return NetworkSet_AddNetworkRangeEx(network_set, range_min, range_max, cidr_bits, ip_not, id, type);
}

int NetworkSet_AddCidrBlockEx(NetworkSet *network_set, uint32_t ip,
        unsigned cidr_bits, int ip_not, unsigned id, unsigned type)
{
    uint32_t mask;

    if(cidr_bits > 32)
        return -1;

    /* Convert cidr to netmask */
    if(cidr_bits == 0)
        mask = 0;
    else
        mask = 0xffffffff << (32 - cidr_bits);

    return NetworkSet_AddNetwork(network_set, ip, cidr_bits, mask, ip_not, id, type);
}

int NetworkSet_AddCidrBlock(NetworkSet *network_set, uint32_t ip,
        unsigned cidr_bits, int ip_not, unsigned id)
{
    return NetworkSet_AddCidrBlockEx(network_set, ip, cidr_bits, ip_not, id, 0);
}

static inline int NetworkSet_AddNetwork6(NetworkSet *network_set, NSIPv6Addr *ip,
        unsigned cidr_bits, NSIPv6Addr *mask, int ip_not, unsigned id, unsigned type)
{
    NSIPv6Addr range_min;
    NSIPv6Addr range_max;

    range_min.lo = ip->lo & mask->lo;
    range_min.hi = ip->hi & mask->hi;
    range_max.lo = range_min.lo + ~mask->lo;
    range_max.hi = range_min.hi + ~mask->hi;
    return NetworkSet_AddNetworkRange6Ex(network_set, &range_min, &range_max, cidr_bits, ip_not, id, type);
}

int NetworkSet_AddCidrBlock6Ex(NetworkSet *network_set, NSIPv6Addr *ip,
        unsigned cidr_bits, int ip_not, unsigned id, unsigned type)
{
    NSIPv6Addr mask;

    if(cidr_bits > 128)
        return -1;

    /* Convert cidr to netmask */
    if(!cidr_bits)
    {
        mask.hi = 0;
        mask.lo = 0;
    }
    else if(cidr_bits < 64)
    {
        mask.hi = ULLONG_MAX << (64 - cidr_bits);
        mask.lo = 0;
    }
    else if(cidr_bits == 64)
    {
        mask.hi = ULLONG_MAX;
        mask.lo = 0;
    }
    else
    {
        mask.hi = ULLONG_MAX;
        mask.lo = ULLONG_MAX << (128 - cidr_bits);
    }

    return NetworkSet_AddNetwork6(network_set, ip, cidr_bits, &mask, ip_not, id, type);
}

int NetworkSet_AddCidrBlock6(NetworkSet *network_set, NSIPv6Addr *ip,
        unsigned cidr_bits, int ip_not, unsigned id)
{
    return NetworkSet_AddCidrBlock6Ex(network_set, ip, cidr_bits, ip_not, id, 0);
}

static inline void NetworkList_Fprintf(NetworkSet *network_set, const char *prefix, FILE *stream)
{
    Network *network;
    Network6 *network6;
    struct in_addr four;
    NSIPv6Addr six;
    char min_ip[INET6_ADDRSTRLEN];
    char max_ip[INET6_ADDRSTRLEN];

    for (network = (Network *)sflist_first(&network_set->networks);
         network;
         network = (Network *)sflist_next(&network_set->networks))
    {
        four.s_addr = htonl(network->range_min);
        inet_ntop(AF_INET, &four, min_ip, sizeof(min_ip));
        four.s_addr = htonl(network->range_max);
        inet_ntop(AF_INET, &four, max_ip, sizeof(max_ip));

        /* check containment for this network */
        fprintf(stream, "%s%s%s-%s for %u with %08X\n", prefix, network->info.ip_not ? "!":"", min_ip, max_ip,
                network->info.id, network->info.type);
    }

    for (network6 = (Network6 *)sflist_first(&network_set->networks6);
         network6;
         network6 = (Network6 *)sflist_next(&network_set->networks6))
    {
        six = network6->range_min;
        NSIPv6AddrHtoN(&six);
        inet_ntop(AF_INET6, (struct in6_addr *)&six, min_ip, sizeof(min_ip));
        six = network6->range_max;
        NSIPv6AddrHtoN(&six);
        inet_ntop(AF_INET6, (struct in6_addr *)&six, max_ip, sizeof(max_ip));

        /* check containment for this network */
        fprintf(stream, "%s%s%s-%s for %u with %08X\n", prefix, network6->info.ip_not ? "!":"", min_ip, max_ip,
                network6->info.id, network6->info.type);
    }
}

int NetworkSet_Fprintf(NetworkSet *network_set, const char *prefix, FILE *stream)
{
    if(!network_set)
        return -1;

    if(!prefix)
        prefix = "";

    if(!stream)
        stream = stdout;

    NetworkList_Fprintf(network_set, prefix, stream);

    return 0;
}

static inline int NetworkSet_OrderByNetmask(SF_LIST *ordered_networks, SF_LIST *networks, unsigned id)
{
    SF_LNODE *node;
    SF_LNODE *i_node;
    NSNetworkInfo *network;

    sflist_init(ordered_networks);
    do
    {
        node = NULL;
        for (i_node = sflist_first_node(networks); i_node; i_node = sflist_next_node(networks))
        {
            if ((network = SFLIST_NODE_TO_DATA(i_node)))
            {
                if (network->id == id &&
                    (node == NULL ||
                     network->netmask < ((NSNetworkInfo *)SFLIST_NODE_TO_DATA(node))->netmask ||
                     (network->netmask == ((NSNetworkInfo *)SFLIST_NODE_TO_DATA(node))->netmask && !network->ip_not)))
                {
                    node = i_node;
                }
            }
        }
        if (node)
        {
            if (sflist_add_tail(ordered_networks, SFLIST_NODE_TO_DATA(node)))
            {
                return -1;
            }
            sflist_remove_node(networks, node);
        }
    } while (node);
    return 0;
}

static inline int NetworkSet_AddList(SF_LIST *networks, SF_LIST *new_networks)
{
    void *network;

    while ((network = sflist_remove_head(new_networks)))
    {
        if (sflist_add_tail(networks, network))
            return -1;
    }
    return 0;
}

static inline int NetworkSet_ReduceNetworkSet(SF_LIST *networks)
{
    Network *ias;
    Network *i_ias;
    Network *new_ias;
    uint32_t tmp;
    SF_LNODE *node;
    SF_LNODE *inode;
    int changed;
    SF_LIST reduced_networks;

    if (!sflist_count(networks))
        return 0;
    sflist_init(&reduced_networks);
    while ((ias = (Network *)sflist_remove_head(networks)))
    {
        /* ias is lowest in the list, so it takes precedence */
        if (ias->info.ip_not)
        {
            node = sflist_first_node(&reduced_networks);
            while (node)
            {
                changed = 0;
                i_ias = SFLIST_NODE_TO_DATA(node);
                if (i_ias)
                {
                    /*
                       i_ias      ******
                       ias   ***************
                    */
                    if (ias->range_min <= i_ias->range_min && ias->range_max >= i_ias->range_max)
                    {
                        sflist_remove_node(&reduced_networks, node);
                        changed = 1;
                    }
                    /*
                        i_ias ************
                        ias        ***
                              or
                        i_ias ************
                        ias      ************
                    */
                    else if (ias->range_min > i_ias->range_min && ias->range_min <= i_ias->range_max)
                    {
                        tmp = i_ias->range_max;
                        i_ias->range_max = ias->range_min - 1;
                        if (ias->range_max < tmp)
                        {
                            if (!(new_ias = calloc(1, sizeof(*new_ias))))
                                return -1;
                            *new_ias = *i_ias;
                            new_ias->range_min = ias->range_max + 1;
                            new_ias->range_max = tmp;
                            if (sflist_add_tail(&reduced_networks, new_ias))
                            {
                                free(new_ias);
                                return -1;
                            }
                            changed = 1;
                        }
                    }
                    /*
                        i_ias      ************
                        ias   ************
                              or
                        i_ias ************
                        ias   ****
                    */
                    else if (ias->range_max >= i_ias->range_min && ias->range_max <= i_ias->range_max)
                    {
                        tmp = i_ias->range_min;
                        i_ias->range_min = ias->range_max + 1;
                        if (ias->range_min > tmp)
                        {
                            if (!(new_ias = calloc(1, sizeof(*new_ias))))
                                return -1;
                            *new_ias = *i_ias;
                            new_ias->range_min = tmp;
                            new_ias->range_max = ias->range_min - 1;
                            if (sflist_add_tail(&reduced_networks, new_ias))
                            {
                                free(new_ias);
                                return -1;
                            }
                            changed = 1;
                        }
                    }
                }
                else
                {
                    sflist_remove_node(&reduced_networks, node);
                    changed = 1;
                }
                if (changed) node = sflist_first_node(&reduced_networks);
                else node = sflist_next_node(&reduced_networks);
            }
            free(ias);
        }
        else
        {
            node = sflist_first_node(&reduced_networks);
            while (node)
            {
                changed = 0;
                i_ias = SFLIST_NODE_TO_DATA(node);
                if (i_ias)
                {
                    if (ias->info.type == i_ias->info.type)
                    {
                        /*
                            i_ias      ******
                            ias   ***************
                        */
                        if (ias->range_min <= i_ias->range_min && ias->range_max >= i_ias->range_max)
                        {
                            sflist_remove_node(&reduced_networks, node);
                            changed = 1;
                            free(i_ias);
                            i_ias = NULL;
                        }
                        /*
                            i_ias ***************
                            ias        ******
                        */
                        else if (i_ias->range_min <= ias->range_min && i_ias->range_max >= ias->range_max)
                        {
                            ias->range_min = i_ias->range_min;
                            ias->range_max = i_ias->range_max;
                            sflist_remove_node(&reduced_networks, node);
                            changed = 1;
                            free(i_ias);
                            i_ias = NULL;
                        }
                        /*
                            i_ias ************
                            ias         ************
                        */
                        else if (ias->range_min > i_ias->range_min && ias->range_min <= i_ias->range_max)
                        {
                            i_ias->range_max = ias->range_min - 1;
                        }
                        /*
                            i_ias      ************
                            ias   ************
                        */
                        else if (ias->range_max >= i_ias->range_min && ias->range_max < i_ias->range_max)
                        {
                            i_ias->range_min = ias->range_max + 1;
                        }
                    }
                    else /* different types */
                    {
                        /*
                            i_ias     ******
                            ias       ******
                        */
                        if (ias->range_min == i_ias->range_min && ias->range_max == i_ias->range_max)
                        {
                            i_ias->info.type = ias->info.type;
                            free(ias);
                            ias = NULL;
                            break;
                        }
                        /*
                            i_ias      ******
                            ias   ***************
                        */
                        else if (ias->range_min < i_ias->range_min && ias->range_max >= i_ias->range_max)
                        {
                            sflist_remove_node(&reduced_networks, node);
                            free(i_ias);
                            i_ias = NULL;
                            changed = 1;
                        }
                        /*
                            i_ias ************
                            ias         ***
                                  or
                            i_ias ************
                            ias         ************
                                  or
                            i_ias ************
                            ias         ******
                        */
                        else if (ias->range_min > i_ias->range_min && ias->range_min <= i_ias->range_max)
                        {
                            tmp = i_ias->range_max;
                            i_ias->range_max = ias->range_min - 1;
                            if (ias->range_max < tmp)
                            {
                                if (!(new_ias = calloc(1, sizeof(*new_ias))))
                                    return -1;
                                *new_ias = *i_ias;
                                new_ias->range_min = ias->range_max + 1;
                                new_ias->range_max = tmp;
                                if (sflist_add_tail(&reduced_networks, new_ias))
                                {
                                    free(new_ias);
                                    return -1;
                                }
                                changed = 1;
                            }
                        }
                        /*
                            i_ias      ************
                            ias   ************
                                  or
                            i_ias ************
                            ias   ****
                        */
                        else if (ias->range_max > i_ias->range_min && ias->range_max < i_ias->range_max)
                        {
                            i_ias->range_min = ias->range_max + 1;
                        }
                    }
                }
                else
                {
                    sflist_remove_node(&reduced_networks, node);
                    changed = 1;
                }
                if (changed)
                    node = sflist_first_node(&reduced_networks);
                else
                    node = sflist_next_node(&reduced_networks);
            }
            if (ias && sflist_add_tail(&reduced_networks, ias))
            {
                return -1;
            }
        }
    }

    /* Minimize the ranges */
    node = sflist_first_node(&reduced_networks);
    while(node)
    {
        /* i_ias is lowest in the list, so it takes precedence */
        changed = 0;
        if ((ias = SFLIST_NODE_TO_DATA(node)))
        {
            inode = sflist_next_node(&reduced_networks);
            while (inode)
            {
                if ((i_ias = SFLIST_NODE_TO_DATA(inode)))
                {
                    if (ias->info.type == i_ias->info.type)
                    {
                        /*
                            i_ias ************
                            ias               ***
                        */
                        if (ias->range_min && (i_ias->range_max+1) == ias->range_min)
                        {
                            i_ias->range_max = ias->range_max;
                            sflist_remove_node(&reduced_networks, node);
                            free(ias);
                            changed = 1;
                            break;
                        }
                        /*
                            i_ias      ************
                            ias   *****
                        */
                        else if (i_ias->range_min && (ias->range_max+1) == i_ias->range_min)
                        {
                            i_ias->range_min = ias->range_min;
                            sflist_remove_node(&reduced_networks, node);
                            free(ias);
                            changed = 1;
                            break;
                        }
                    }
                }
                inode = sflist_next_node(&reduced_networks);
            }
        }
        else
            sflist_remove_node(&reduced_networks, node);
        if (changed)
            node = sflist_first_node(&reduced_networks);
        else
            node = sflist_next_node(&reduced_networks);
    }
    sflist_static_free_all(networks, &free);
    while ((ias = (Network *)sflist_remove_head(&reduced_networks)))
    {
        if (sflist_add_tail(networks, ias))
        {
            return -1;
        }
    }
    return 0;
}

static inline int NetworkSet_ReduceNetworkSet6(SF_LIST *networks)
{
    Network6 *ias;
    Network6 *i_ias;
    Network6 *new_ias;
    NSIPv6Addr tmp;
    NSIPv6Addr tmp2;
    SF_LNODE *node;
    SF_LNODE *inode;
    int changed;
    SF_LIST reduced_networks;

    if (!sflist_count(networks))
        return 0;
    sflist_init(&reduced_networks);
    while ((ias = (Network6 *)sflist_remove_head(networks)))
    {
        /* ias is lowest in the list, so it takes precedence */
        if (ias->info.ip_not)
        {
            node = sflist_first_node(&reduced_networks);
            while (node)
            {
                changed = 0;
                i_ias = SFLIST_NODE_TO_DATA(node);
                if (i_ias)
                {
                    /*
                       i_ias      ******
                       ias   ***************
                    */
                    if (NSIPv6AddrCompare(&ias->range_min, &i_ias->range_min) <= 0 &&
                        NSIPv6AddrCompare(&ias->range_max, &i_ias->range_max) >= 0)
                    {
                        sflist_remove_node(&reduced_networks, node);
                        changed = 1;
                    }
                    /*
                        i_ias ************
                        ias        ***
                              or
                        i_ias ************
                        ias      ************
                    */
                    else if (NSIPv6AddrCompare(&ias->range_min, &i_ias->range_min) > 0 &&
                             NSIPv6AddrCompare(&ias->range_min, &i_ias->range_max) <= 0)
                    {
                        tmp = i_ias->range_max;
                        i_ias->range_max = ias->range_min;
                        NSIPv6AddrDec(&i_ias->range_max);
                        if (NSIPv6AddrCompare(&ias->range_max, &tmp) < 0)
                        {
                            if (!(new_ias = calloc(1, sizeof(*new_ias))))
                                return -1;
                            *new_ias = *i_ias;
                            new_ias->range_min = ias->range_max;
                            NSIPv6AddrInc(&new_ias->range_min);
                            new_ias->range_max = tmp;
                            if (sflist_add_tail(&reduced_networks, new_ias))
                            {
                                free(new_ias);
                                return -1;
                            }
                            changed = 1;
                        }
                    }
                    /*
                        i_ias      ************
                        ias   ************
                              or
                        i_ias ************
                        ias   ****
                    */
                    else if (NSIPv6AddrCompare(&ias->range_max, &i_ias->range_min) >= 0 &&
                             NSIPv6AddrCompare(&ias->range_max, &i_ias->range_max) <= 0)
                    {
                        tmp = i_ias->range_min;
                        i_ias->range_min = ias->range_max;
                        NSIPv6AddrInc(&i_ias->range_min);
                        if (NSIPv6AddrCompare(&ias->range_min, &tmp) > 0)
                        {
                            if (!(new_ias = calloc(1, sizeof(*new_ias))))
                                return -1;
                            *new_ias = *i_ias;
                            new_ias->range_min = tmp;
                            new_ias->range_max = ias->range_min;
                            NSIPv6AddrDec(&new_ias->range_max);
                            if (sflist_add_tail(&reduced_networks, new_ias))
                            {
                                free(new_ias);
                                return -1;
                            }
                            changed = 1;
                        }
                    }
                }
                else
                {
                    sflist_remove_node(&reduced_networks, node);
                    changed = 1;
                }
                if (changed) node = sflist_first_node(&reduced_networks);
                else node = sflist_next_node(&reduced_networks);
            }
            free(ias);
        }
        else
        {
            node = sflist_first_node(&reduced_networks);
            while (node)
            {
                changed = 0;
                i_ias = SFLIST_NODE_TO_DATA(node);
                if (i_ias)
                {
                    if (ias->info.type == i_ias->info.type)
                    {
                        /*
                            i_ias      ******
                            ias   ***************
                        */
                        if (NSIPv6AddrCompare(&ias->range_min, &i_ias->range_min) <= 0 &&
                            NSIPv6AddrCompare(&ias->range_max, &i_ias->range_max) >= 0)
                        {
                            sflist_remove_node(&reduced_networks, node);
                            changed = 1;
                            free(i_ias);
                            i_ias = NULL;
                        }
                        /*
                            i_ias ***************
                            ias        ******
                        */
                        else if (NSIPv6AddrCompare(&i_ias->range_min, &ias->range_min) <= 0 &&
                                 NSIPv6AddrCompare(&i_ias->range_max, &ias->range_max) >= 0)
                        {
                            ias->range_min = i_ias->range_min;
                            ias->range_max = i_ias->range_max;
                            sflist_remove_node(&reduced_networks, node);
                            changed = 1;
                            free(i_ias);
                            i_ias = NULL;
                        }
                        /*
                            i_ias ************
                            ias         ************
                        */
                        else if (NSIPv6AddrCompare(&ias->range_min, &i_ias->range_min) > 0 &&
                                 NSIPv6AddrCompare(&ias->range_min, &i_ias->range_max) <= 0)
                        {
                            i_ias->range_max = ias->range_min;
                            NSIPv6AddrDec(&i_ias->range_max);
                        }
                        /*
                            i_ias      ************
                            ias   ************
                        */
                        else if (NSIPv6AddrCompare(&ias->range_max, &i_ias->range_min) >= 0 &&
                                 NSIPv6AddrCompare(&ias->range_max, &i_ias->range_max) < 0)
                        {
                            i_ias->range_min = ias->range_max;
                            NSIPv6AddrInc(&i_ias->range_min);
                        }
                    }
                    else /* different types */
                    {
                        /*
                            i_ias     ******
                            ias       ******
                        */
                        if (!NSIPv6AddrCompare(&ias->range_min, &i_ias->range_min) &&
                            !NSIPv6AddrCompare(&ias->range_max, &i_ias->range_max))
                        {
                            i_ias->info.type = ias->info.type;
                            free(ias);
                            ias = NULL;
                            break;
                        }
                        /*
                            i_ias      ******
                            ias   ***************
                        */
                        else if (NSIPv6AddrCompare(&ias->range_min, &i_ias->range_min) < 0 &&
                                 NSIPv6AddrCompare(&ias->range_max, &i_ias->range_max) >= 0)
                        {
                            sflist_remove_node(&reduced_networks, node);
                            free(i_ias);
                            i_ias = NULL;
                            changed = 1;
                        }
                        /*
                            i_ias ************
                            ias         ***
                                  or
                            i_ias ************
                            ias         ************
                                  or
                            i_ias ************
                            ias         ******
                        */
                        else if (NSIPv6AddrCompare(&ias->range_min, &i_ias->range_min) > 0 &&
                                 NSIPv6AddrCompare(&ias->range_min, &i_ias->range_max) <= 0)
                        {
                            tmp = i_ias->range_max;
                            i_ias->range_max = ias->range_min;
                            NSIPv6AddrDec(&i_ias->range_max);
                            if (NSIPv6AddrCompare(&ias->range_max, &tmp) < 0)
                            {
                                if (!(new_ias = calloc(1, sizeof(*new_ias))))
                                    return -1;
                                *new_ias = *i_ias;
                                new_ias->range_min = ias->range_max;
                                NSIPv6AddrInc(&new_ias->range_min);
                                new_ias->range_max = tmp;
                                if (sflist_add_tail(&reduced_networks, new_ias))
                                {
                                    free(new_ias);
                                    return -1;
                                }
                                changed = 1;
                            }
                        }
                        /*
                            i_ias      ************
                            ias   ************
                                  or
                            i_ias ************
                            ias   ****
                        */
                        else if (NSIPv6AddrCompare(&ias->range_max, &i_ias->range_min) > 0 &&
                                 NSIPv6AddrCompare(&ias->range_max, &i_ias->range_max) < 0)
                        {
                            i_ias->range_min = ias->range_max;
                            NSIPv6AddrInc(&i_ias->range_min);
                        }
                    }
                }
                else
                {
                    sflist_remove_node(&reduced_networks, node);
                    changed = 1;
                }
                if (changed)
                    node = sflist_first_node(&reduced_networks);
                else
                    node = sflist_next_node(&reduced_networks);
            }
            if (ias && sflist_add_tail(&reduced_networks, ias))
            {
                return -1;
            }
        }
    }

    /* Minimize the ranges */
    node = sflist_first_node(&reduced_networks);
    while(node)
    {
        /* i_ias is lowest in the list, so it takes precedence */
        changed = 0;
        if ((ias = SFLIST_NODE_TO_DATA(node)))
        {
            inode = sflist_next_node(&reduced_networks);
            while (inode)
            {
                if ((i_ias = SFLIST_NODE_TO_DATA(inode)))
                {
                    if (ias->info.type == i_ias->info.type)
                    {
                        /*
                            i_ias ************
                            ias               ***
                        */
                        tmp = i_ias->range_max;
                        NSIPv6AddrInc(&tmp);
                        tmp2 = ias->range_max;
                        NSIPv6AddrInc(&tmp2);
                        if ((ias->range_min.lo || ias->range_min.hi) && !NSIPv6AddrCompare(&tmp, &ias->range_min))
                        {
                            i_ias->range_max = ias->range_max;
                            sflist_remove_node(&reduced_networks, node);
                            free(ias);
                            changed = 1;
                            break;
                        }
                        /*
                            i_ias      ************
                            ias   *****
                        */
                        else if ((i_ias->range_min.lo || i_ias->range_min.hi) && !NSIPv6AddrCompare(&tmp2, &i_ias->range_min))
                        {
                            i_ias->range_min = ias->range_min;
                            sflist_remove_node(&reduced_networks, node);
                            free(ias);
                            changed = 1;
                            break;
                        }
                    }
                }
                inode = sflist_next_node(&reduced_networks);
            }
        }
        else
            sflist_remove_node(&reduced_networks, node);
        if (changed)
            node = sflist_first_node(&reduced_networks);
        else
            node = sflist_next_node(&reduced_networks);
    }
    sflist_static_free_all(networks, &free);
    while ((ias = (Network6 *)sflist_remove_head(&reduced_networks)))
    {
        if (sflist_add_tail(networks, ias))
        {
            return -1;
        }
    }
    return 0;
}

int NetworkSet_Reduce(NetworkSet *network_set)
{
    SFXHASH_NODE *hnode;
    unsigned id;
    int rval;
    SF_LIST ordered_networks;
    Network *network;
    Network6 *network6;
    unsigned tmp;
    int count;
    int i;
    int j;

    if(!network_set)
        return -1;

    for (hnode=sfxhash_gfindfirst(network_set->ids); hnode; hnode=sfxhash_gfindnext(network_set->ids))
    {
        id = *(unsigned *)(hnode->data);
        if ((rval = NetworkSet_OrderByNetmask(&ordered_networks, &network_set->networks, id)) != 0)
        {
            sflist_free_all(&ordered_networks, &free);
            return rval;
        }
        if ((rval = NetworkSet_ReduceNetworkSet(&ordered_networks)) != 0)
        {
            sflist_free_all(&ordered_networks, &free);
            return rval;
        }
        if ((rval = NetworkSet_AddList(&network_set->networks, &ordered_networks)) != 0)
        {
            sflist_free_all(&ordered_networks, &free);
            return rval;
        }
    }
    if ((rval = NetworkSet_ReduceNetworkSet(&network_set->networks)) != 0)
    {
        sflist_free_all(&ordered_networks, &free);
        return rval;
    }

    tmp = 0;
    if ((rval = NetworkSet_Count(network_set, &tmp)) != 0)
        return rval;

    count = (int)tmp;
    if (count > 0)
    {
        network_set->count = count;
        if (network_set->pnetwork)
        {
            free(network_set->pnetwork);
            network_set->pnetwork = NULL;
        }
        if (!(network_set->pnetwork = calloc(count, sizeof(*network_set->pnetwork))))
            return -1;
        for (network = (Network *)sflist_first(&network_set->networks), i = 0;
             network && i < count;
             network = (Network *)sflist_next(&network_set->networks))
        {
            network_set->pnetwork[i++] = network;
        }
        /* bubble sort this array */
        for(i = (count - 1); i >= 0; i--)
        {
            for(j = 1; j <= i ; j++ )
            {
                if(network_set->pnetwork[j-1]->range_min > network_set->pnetwork[j]->range_min)
                {
                    network = network_set->pnetwork[j-1];
                    network_set->pnetwork[j-1] = network_set->pnetwork[j];
                    network_set->pnetwork[j] = network;
                }
            }
        }
    }

    for (hnode=sfxhash_gfindfirst(network_set->ids6); hnode; hnode=sfxhash_gfindnext(network_set->ids6))
    {
        id = *(unsigned *)(hnode->data);
        if ((rval = NetworkSet_OrderByNetmask(&ordered_networks, &network_set->networks6, id)) != 0)
        {
            sflist_free_all(&ordered_networks, &free);
            return rval;
        }
        if ((rval = NetworkSet_ReduceNetworkSet6(&ordered_networks)) != 0)
        {
            sflist_free_all(&ordered_networks, &free);
            return rval;
        }
        if ((rval = NetworkSet_AddList(&network_set->networks6, &ordered_networks)) != 0)
        {
            sflist_free_all(&ordered_networks, &free);
            return rval;
        }
    }
    if ((rval = NetworkSet_ReduceNetworkSet6(&network_set->networks6)) != 0)
    {
        sflist_free_all(&ordered_networks, &free);
        return rval;
    }

    tmp = 0;
    if ((rval = NetworkSet_Count6(network_set, &tmp)) != 0)
        return rval;

    count = (int)tmp;
    if (count > 0)
    {
        network_set->count6 = count;
        if(network_set->pnetwork6)
        {
            free(network_set->pnetwork6);
            network_set->pnetwork6 = NULL;
        }
        if (!(network_set->pnetwork6 = calloc(count, sizeof(*network_set->pnetwork6))))
            return -1;
        for (network6 = (Network6 *)sflist_first(&network_set->networks6), i = 0;
             network6 && i < count;
             network6 = (Network6 *)sflist_next(&network_set->networks6))
        {
            network_set->pnetwork6[i++] = network6;
        }
        /* bubble sort this array */
        for(i = (count - 1); i >= 0; i--)
        {
            for(j = 1; j <= i ; j++ )
            {
                if(NSIPv6AddrCompare(&network_set->pnetwork6[j-1]->range_min, &network_set->pnetwork6[j]->range_min) > 0)
                {
                    network6 = network_set->pnetwork6[j-1];
                    network_set->pnetwork6[j-1] = network_set->pnetwork6[j];
                    network_set->pnetwork6[j] = network6;
                }
            }
        }
    }
    return 0;
}

NetworkSet *NetworkSet_Copy(NetworkSet *network_set)
{
    NetworkSet *new_set;
    Network *network;
    Network6 *network6;

    if(!network_set)
        return NULL;

    if (NetworkSet_New(&new_set) != 0)
        return NULL;

    for (network = (Network *)sflist_first(&network_set->networks);
         network;
         network = (Network *)sflist_next(&network_set->networks))
    {
        if (NetworkSet_AddNetworkRangeEx(new_set, network->range_min, network->range_max,
                                         network->info.netmask, network->info.ip_not,
                                         network->info.id, network->info.type) != 0)
        {
            NetworkSet_Destroy(new_set);
            return NULL;
        }
    }
    for (network6 = (Network6 *)sflist_first(&network_set->networks6);
         network6;
         network6 = (Network6 *)sflist_next(&network_set->networks6))
    {
        if (NetworkSet_AddNetworkRange6Ex(new_set, &network6->range_min, &network6->range_max,
                                          network6->info.netmask, network6->info.ip_not,
                                          network6->info.id, network6->info.type) != 0)
        {
            NetworkSet_Destroy(new_set);
            return NULL;
        }
    }
    return new_set;
}

int NetworkSet_AddSet(NetworkSet *dest_set, NetworkSet *src_set)
{
    Network *network;
    Network6 *network6;
    int rval;

    if(!src_set || !dest_set)
        return -1;

    for (network = (Network *)sflist_first(&src_set->networks);
         network;
         network = (Network *)sflist_next(&src_set->networks))
    {
        if ((rval=NetworkSet_AddNetworkRangeEx(dest_set, network->range_min, network->range_max,
                                               network->info.netmask, network->info.ip_not,
                                               network->info.id, network->info.type)) != 0)
        {
            return rval;
        }
    }
    for (network6 = (Network6 *)sflist_first(&src_set->networks6);
         network6;
         network6 = (Network6 *)sflist_next(&src_set->networks6))
    {
        if ((rval=NetworkSet_AddNetworkRange6Ex(dest_set, &network6->range_min, &network6->range_max,
                                                network6->info.netmask, network6->info.ip_not,
                                                network6->info.id, network6->info.type)) != 0)
        {
            return rval;
        }
    }
    return 0;
}

