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


#ifndef __IP_FUNCS_H__
#define __IP_FUNCS_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_snort_plugin_api.h"
#include "profiler.h"

#if  !defined(s6_addr32)
#define s6_addr8  __u6_addr.__u6_addr8
#define s6_addr16 __u6_addr.__u6_addr16
#define s6_addr32 __u6_addr.__u6_addr32
#endif

static inline void copyIpv4ToIpv6Network(struct in6_addr *keyIp, const uint32_t ip)
{
    keyIp->s6_addr32[0] = keyIp->s6_addr32[1] = 0;
    keyIp->s6_addr16[4] = 0;
    keyIp->s6_addr16[5] = 0xFFFF;
    keyIp->s6_addr32[3] = ip;
}

//these functions are needed since snort does not store IPv4 address in highest 4 bytes
//of 16 byte ip.
static inline void copySnortIpToIpv6Network(struct in6_addr *keyIp, const snort_ip *snortIp)
{
    if (IS_IP4(snortIp))
        copyIpv4ToIpv6Network(keyIp, snortIp->ip32[0]);
    else
        memcpy(keyIp, snortIp->ip8, sizeof(*keyIp));
}

static inline int cmpSnortIpToHostKey(struct in6_addr *keyIp, const snort_ip *snortIp)
{
    if (IS_IP4(snortIp))
    {
        return keyIp->s6_addr32[3] - snortIp->ip32[0];
    }
    else
        return memcmp(keyIp, snortIp->ip8, sizeof(*keyIp));
}
#endif  /* __IP_FUNCS_H__ */

