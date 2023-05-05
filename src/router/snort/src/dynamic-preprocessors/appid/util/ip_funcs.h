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

#ifndef __IP_FUNCS_H__
#define __IP_FUNCS_H__

#include <stdint.h>
#include <NetworkSet.h>
#include <ipv6_port.h>

#define IPFUNCS_EXCEPT_IP       0x01
#define IPFUNCS_SECONDARY_IP    0x02
#define IPFUNCS_APPID_SESSION_EXCLUDE_IP 0x04
#define IPFUNCS_USER_IP         0x08
#define IPFUNCS_HOSTS_IP        0x10
#define IPFUNCS_APPLICATION     0x20
#define IPFUNCS_CHECKED         0x80000000

typedef struct _RNAIpAddrSet
{
    uint32_t range_min;
    uint32_t range_max;
    uint32_t addr_flags;
    unsigned netmask;
    uint32_t netmask_mask;
} RNAIpAddrSet;

RNAIpAddrSet *ParseIpCidr(char *, uint32_t *);

typedef struct _RNAIpv6AddrSet
{
    NSIPv6Addr range_min;
    NSIPv6Addr range_max;
    uint32_t addr_flags;
    unsigned netmask;
    NSIPv6Addr netmask_mask;
} RNAIpv6AddrSet;

RNAIpv6AddrSet *ParseIpv6Cidr(char *);

static inline void copyIpv4ToIpv6Network(struct in6_addr *keyIp, const uint32_t ip)
{
    keyIp->s6_addr32[0] = keyIp->s6_addr32[1] = 0;
    keyIp->s6_addr16[4] = 0;
    keyIp->s6_addr16[5] = 0xFFFF;
    keyIp->s6_addr32[3] = ip;
}

//these functions are needed since snort does not store IPv4 address in highest 4 bytes
//of 16 byte ip.
static inline void copySnortIpToIpv6Network(struct in6_addr *keyIp, const sfaddr_t *snortIp)
{
    memcpy(keyIp, sfaddr_get_ip6_ptr(snortIp), sizeof(*keyIp));
}

static inline int cmpSnortIpToHostKey(struct in6_addr *keyIp, const sfaddr_t *snortIp)
{
    return memcmp(keyIp, sfaddr_get_ip6_ptr(snortIp), sizeof(*keyIp));
}

#endif  /* __IP_FUNCS_H__ */

