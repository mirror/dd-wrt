/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
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
 
/*
 * @file    snort_stream_ip.h
 * @author  Russ Combs <rcombs@sourcefire.com>
 *
 */

#ifndef __STREAM_IP_H__
#define __STREAM_IP_H__

#include "session_common.h"
#include "stream_common.h"
#include "sfPolicy.h"

void StreamCleanIp(void);
void StreamResetIp(void);
void StreamInitIp(void);

void StreamIpPolicyInit(StreamIpConfig*, char*);
int StreamVerifyIpConfig(StreamIpConfig*, tSfPolicyId);
void StreamIpConfigFree(StreamIpConfig*);

int StreamProcessIp(Packet *p, SessionControlBlock *scb, SessionKey *skey);

uint32_t StreamGetIpPrunes(void);
void StreamResetIpPrunes(void);

void IpSessionCleanup (void* lws);

void SessionIPReload(uint32_t max_sessions, uint16_t pruningTimeout, uint16_t nominalTimeout);
unsigned SessionIPReloadAdjust(unsigned maxWork);

size_t get_ip_used_mempool();

#endif
