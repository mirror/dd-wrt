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


#ifndef __COMMON_UTIL_H__
#define __COMMON_UTIL_H__

#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <netinet/in.h>
#include <sf_types.h>
#include <sflsq.h>

typedef struct _FWDebugSessionConstraints
{
    struct in6_addr sip;
    int sip_flag;
    struct in6_addr dip;
    int dip_flag;
    uint16_t sport;
    uint16_t dport;
    uint8_t protocol;
} FWDebugSessionConstraints;

#define FW_DEBUG_SESSION_ID_SIZE    (39+1+5+5+39+1+5+1+3+1+1+1)

typedef struct _config_item
{
    char *name;                         /* name of the config item */
    char *value;                        /* config item value */
} ConfigItem;

#define MAX_LINE    2048
#define MAX_TOKS    256

void ProcessListDirective(const char *section, char *toklist[], SF_LIST *list);

extern time_t packetTimeOffset;
extern time_t packetTime;
extern int packetTimeOffsetSet;
#define GetPacketRealTime   packetTime
static inline void SetPacketRealTime(time_t pktTime)
{
    if (!packetTimeOffsetSet)
    {
        time_t tmp = time(NULL);

        packetTimeOffsetSet = 1;
        if (pktTime < tmp)
            packetTimeOffset = tmp - pktTime;
    }
    packetTime = pktTime + packetTimeOffset;
}
#endif  /* __COMMON_UTIL_H__ */

