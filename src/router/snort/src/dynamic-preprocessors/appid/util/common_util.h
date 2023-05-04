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

#define FW_DEBUG_SESSION_ID_SIZE    (39+1+5+4+39+1+5+1+3+1+1+1+2+1+10+1+1+1+10+1)

typedef struct _config_item
{
    char *name;                         /* name of the config item */
    char *value;                        /* config item value */
} ConfigItem;

#define MAX_LINE    2048
#define MAX_TOKS    256

/* This structure should be same as struct define in util.h */
typedef struct _ThrottleInfo
{
    time_t lastUpdate;
    /*Within this duration (in seconds), maximal one distinct message is logged*/
    uint32_t duration_to_log;
    uint64_t count;
    /*Till the message count reaches to count_to_log, maximal one distinct message is logged*/
    uint64_t count_to_log;
}ThrottleInfo;

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

static inline void DumpHex(FILE *fp, const uint8_t *data, unsigned len)
{
    char str[18];
    unsigned i;
    unsigned pos;
    char c;

    for (i=0, pos=0; i<len; i++, pos++)
    {
        if (pos == 17)
        {
            str[pos] = 0;
            fprintf(fp, "  %s\n", str);
            pos = 0;
        }
        else if (pos == 8)
        {
            str[pos] = ' ';
            pos++;
            fprintf(fp, "%s", " ");
        }
        c = (char)data[i];
        if (isprint(c) && !isspace(c))
            str[pos] = c;
        else
            str[pos] = '.';
        fprintf(fp, "%02X ", data[i]);
    }
    if (pos)
    {
        str[pos] = 0;
        for (; pos < 17; pos++)
        {
            if (pos == 8)
            {
                str[pos] = ' ';
                pos++;
                fprintf(fp, "%s", "    ");
            }
            else
            {
                fprintf(fp, "%s", "   ");
            }
        }
        fprintf(fp, "  %s\n", str);
    }
}

#endif  /* __COMMON_UTIL_H__ */

