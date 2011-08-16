/****************************************************************************
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/
 
#ifndef STREAM5_UDP_H_
#define STREAM5_UDP_H_

#include "ipv6_port.h"
#include "stream5_common.h"
#include "sfPolicy.h"

void Stream5CleanUdp(void);
void Stream5ResetUdp(void);
void Stream5InitUdp(Stream5GlobalConfig *);
void Stream5UdpPolicyInit(Stream5UdpConfig *, char *);
int Stream5VerifyUdpConfig(Stream5UdpConfig *, tSfPolicyId);
int Stream5ProcessUdp(Packet *, Stream5LWSession *, Stream5UdpPolicy *, SessionKey *);
void UdpUpdateDirection(Stream5LWSession *ssn, char dir,
                        snort_ip_p ip, uint16_t port);
Stream5LWSession *GetLWUdpSession(SessionKey *key);
void s5UdpSetPortFilterStatus(
        unsigned short port, 
        int status,
        tSfPolicyId policyId,
        int parsing
        );
int s5UdpGetPortFilterStatus(
        unsigned short port,
        tSfPolicyId policyId,
        int parsing
        );
void Stream5UdpConfigFree(Stream5UdpConfig *);

uint32_t Stream5GetUdpPrunes(void);
void Stream5ResetUdpPrunes(void);

#endif /* STREAM5_UDP_H_ */
