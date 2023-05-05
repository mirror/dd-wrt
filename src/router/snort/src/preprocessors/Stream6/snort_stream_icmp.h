/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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

#ifndef STREAM_ICMP_H_
#define STREAM_ICMP_H_

#include "session_common.h"
#include "stream_common.h"
#include "sfPolicy.h"

void StreamCleanIcmp(void);
void StreamResetIcmp(void);
void StreamInitIcmp(void);
void StreamIcmpPolicyInit(StreamIcmpConfig *, char *);
int StreamVerifyIcmpConfig(StreamIcmpConfig *, tSfPolicyId);
int StreamProcessIcmp(Packet *p);
void IcmpUpdateDirection(SessionControlBlock *ssn, char dir,
                         sfaddr_t* ip, uint16_t port);
void StreamIcmpConfigFree(StreamIcmpConfig *);

uint32_t StreamGetIcmpPrunes(void);
void StreamResetIcmpPrunes(void);
void IcmpSessionCleanup(SessionControlBlock *ssn);

void SessionICMPReload(uint32_t max_sessions, uint16_t pruningTimeout, uint16_t nominalTimeout);
unsigned SessionICMPReloadAdjust(unsigned maxWork);

size_t get_icmp_used_mempool();

#endif /* STREAM_ICMP_H_ */
