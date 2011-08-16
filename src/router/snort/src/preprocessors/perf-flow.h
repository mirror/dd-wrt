/* $Id$ */
/*
** perf-flow.h
**
** Copyright (C) 2002-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
*/


#ifndef _PERF_FLOW_H
#define _PERF_FLOW_H

#include "sf_types.h"
#include "sfxhash.h"
#include "ipv6_port.h"

#define SF_MAX_PKT_LEN 4500
#define SF_MAX_PORT (64*1024)

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

    double   totperc[SF_MAX_PORT];
    double   sport_rate[SF_MAX_PORT];
    double   dport_rate[SF_MAX_PORT];

} PORTFLOW;

typedef struct _icmpflow {

    double totperc[256];
    int    display[256];

} ICMPFLOW;

typedef struct _sfflow {

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

    double    pktLenPercent[SF_MAX_PKT_LEN];
   
    double    trafficTCP;
    double    trafficUDP;
    double    trafficICMP;
    double    trafficOTHER;

    PORTFLOW  portflowTCP;
    double    portflowHighTCP;

    PORTFLOW  portflowUDP;
    double    portflowHighUDP;

    ICMPFLOW  flowICMP;


}  SFFLOW_STATS;

/*
**  Functions for the performance functions to call
*/
int InitFlowStats   (SFFLOW *sfFlow);
int InitFlowIPStats   (SFFLOW *sfFlow);
int UpdateFlowStats (SFFLOW *sfFlow, const unsigned char *pucBuffer, uint32_t len,
        int iRebuiltPkt);
int ProcessFlowStats(SFFLOW *sfFlow);
int ProcessFlowIPStats(SFFLOW *sfFlow, FILE *fh);

/*
**  These functions wrap the perf-flow functionality within
**  decode.c so we don't have to decode the packet for our
**  own stats.  Helps speed.
*/
int UpdateUDPFlowStatsEx(SFFLOW *, int sport, int dport, int len);
int UpdateTCPFlowStatsEx(SFFLOW *, int sport, int dport, int len);
int UpdateICMPFlowStatsEx(SFFLOW *, int type, int len);
int UpdateFlowIPStats(SFFLOW *, snort_ip_p src_addr, snort_ip_p dst_addr, int len, SFSType type);
int UpdateFlowIPState(SFFLOW *, snort_ip_p src_addr, snort_ip_p dst_addr, SFSState state);
void FreeFlowStats(SFFLOW *sfFlow);

#endif


