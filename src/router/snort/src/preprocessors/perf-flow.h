/* $Id$ */
/*
** perf-flow.h
**
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Marc Norton <mnorton@sourcefire.com>
** Dan Roelker <droelker@sourcefire.com>
**
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
**
*/


#ifndef _PERF_FLOW_H
#define _PERF_FLOW_H

#include "sf_types.h"
#include "sfxhash.h"
#include "ipv6_port.h"
#include "decode.h"

#define SF_MAX_PKT_LEN  9000
#define SF_MAX_PORT     UINT16_MAX

typedef enum {
    SFS_TYPE_TCP   = 0,
    SFS_TYPE_UDP   = 1,
    SFS_TYPE_OTHER = 2,
    SFS_TYPE_MAX   = 3
} SFSType;

typedef enum {
    SFS_STATE_TCP_ESTABLISHED = 0,
    SFS_STATE_TCP_CLOSED      = 1,
    SFS_STATE_UDP_CREATED     = 2,
    SFS_STATE_MAX             = 3
} SFSState;

typedef struct _portflow {

    double   totperc[SF_MAX_PORT+1];
    double   sport_rate[SF_MAX_PORT+1];
    double   dport_rate[SF_MAX_PORT+1];

} PORTFLOW;

typedef struct _icmpflow {

    double totperc[256];
    int    display[256];

} ICMPFLOW;

typedef struct _sfflow {

    time_t time;
    uint64_t   *pktLenCnt;
    uint64_t    pktTotal;

    uint64_t   byteTotal;

    uint64_t   *pktLenPercent;

    uint64_t   *portTcpSrc;
    uint64_t   *portTcpDst;
    uint64_t   *portUdpSrc;
    uint64_t   *portUdpDst;

    uint64_t   *typeIcmp;

    uint64_t    portTcpHigh;
    uint64_t    portTcpTotal;

    uint64_t    portUdpHigh;
    uint64_t    portUdpTotal;

    uint64_t    typeIcmpTotal;

    SFXHASH     *ipMap;
}  SFFLOW;

typedef struct _sfflow_stats {

    time_t time;
    double    pktLenPercent[SF_MAX_PKT_LEN + 2];
    int       pktLenPercentCount;

    double    trafficTCP;
    double    trafficUDP;
    double    trafficICMP;
    double    trafficOTHER;

    PORTFLOW  portflowTCP;
    double    portflowHighTCP;
    int       portflowTCPCount;

    PORTFLOW  portflowUDP;
    double    portflowHighUDP;
    int       portflowUDPCount;

    ICMPFLOW  flowICMP;
    int       flowICMPCount;


}  SFFLOW_STATS;

/*
**  Functions for the performance functions to call
*/
int InitFlowStats   (SFFLOW *sfFlow);
int InitFlowIPStats   (SFFLOW *sfFlow);
void UpdateFlowStats(SFFLOW *, Packet *);
void ProcessFlowStats(SFFLOW *sfFlow, FILE *fh, int console);
void ProcessFlowIPStats(SFFLOW *sfFlow, FILE *fh, int console);
int UpdateFlowIPStats(SFFLOW *, sfaddr_t* src_addr, sfaddr_t* dst_addr, int len, SFSType type);
int UpdateFlowIPState(SFFLOW *, sfaddr_t* src_addr, sfaddr_t* dst_addr, SFSState state);
void FreeFlowStats(SFFLOW *sfFlow);
void FreeFlowIPStats(SFFLOW *sfFlow);
void LogFlowPerfHeader(FILE *);

#endif


