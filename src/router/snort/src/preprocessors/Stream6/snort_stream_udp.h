/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

#ifndef STREAM_UDP_H_
#define STREAM_UDP_H_

#include "ipv6_port.h"
#include "session_api.h"
#include "stream_common.h"
#include "sfPolicy.h"

void StreamCleanUdp(void);
void StreamResetUdp(void);
void StreamInitUdp(void);
void StreamUdpPolicyInit(StreamUdpConfig *, char *);
int StreamVerifyUdpConfig(struct _SnortConfig *, StreamUdpConfig *, tSfPolicyId);
int StreamProcessUdp(Packet *, SessionControlBlock *, StreamUdpPolicy *, SessionKey *);
void UdpUpdateDirection(SessionControlBlock *ssn, char dir,
                        sfaddr_t* ip, uint16_t port);
SessionControlBlock *GetLWUdpSession(const SessionKey *key);
void s5UdpSetPortFilterStatus(
        struct _SnortConfig *sc,
        unsigned short port,
        uint16_t status,
        tSfPolicyId policyId,
        int parsing
        );
void s5UdpUnsetPortFilterStatus(
        struct _SnortConfig *sc,
        unsigned short port,
        uint16_t status,
        tSfPolicyId policyId,
        int parsing
        );
int s5UdpGetPortFilterStatus(
        struct _SnortConfig *sc,
        unsigned short port,
        tSfPolicyId policyId,
        int parsing
        );
int s5UdpGetIPSPortFilterStatus(
        struct _SnortConfig *sc, 
        unsigned short sport, 
        unsigned short dport, 
        tSfPolicyId policyId);
void InspectPortFilterUdp (Packet *p);
void StreamUdpConfigFree(StreamUdpConfig *);

uint32_t StreamGetUdpPrunes(void);
void StreamResetUdpPrunes(void);
void UdpSessionCleanup(void *scb);

void SessionUDPReload(uint32_t max_sessions, uint16_t pruningTimeout, uint16_t nominalTimeout);
unsigned SessionUDPReloadAdjust(unsigned maxWork);

size_t get_udp_used_mempool();

#endif /* STREAM_UDP_H_ */
