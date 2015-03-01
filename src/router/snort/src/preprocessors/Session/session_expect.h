/* $Id$ */

/*
 * Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
** AUTHOR: Steven Sturges
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

/* session_expect.h
 *
 * Purpose: Handle hash table storage and lookups for ignoring
 *          entire data streams.
 *
 * Arguments:
 *
 * Effect:
 *
 * Comments: Used by Stream4 & Stream -- don't delete too soon.
 *
 * Any comments?
 *
 */

#ifndef SESSION_EXPECT_H_
#define SESSION_EXPECT_H_

#include "ipv6_port.h"
#include "session_common.h"
#include "stream_common.h"
#include "sfxhash.h"

int StreamExpectAddChannel(const Packet *ctrlPkt, snort_ip_p cliIP, uint16_t cliPort,
                           snort_ip_p srvIP, uint16_t srvPort, char direction, uint8_t flags,
                           uint8_t protocol, uint32_t timeout, int16_t appId, uint32_t preprocId,
                           void *appData, void (*appDataFreeFn)(void*));

int StreamExpectAddChannelPreassignCallback(const Packet *ctrlPkt, snort_ip_p cliIP, uint16_t cliPort,
                           snort_ip_p srvIP, uint16_t srvPort, char direction, uint8_t flags,
                           uint8_t protocol, uint32_t timeout, int16_t appId, uint32_t preprocId,
                           void *appData, void (*appDataFreeFn)(void*), unsigned cbId,
                           Stream_Event se);

int StreamExpectIsExpected(Packet *p, SFXHASH_NODE **expected_hash_node);
char StreamExpectProcessNode(Packet *p, SessionControlBlock* lws, SFXHASH_NODE *expected_hash_node);
char StreamExpectCheck(Packet *, SessionControlBlock *);
void StreamExpectInit(uint32_t maxExpectations);
void StreamExpectCleanup(void);

#endif /* SESSION_EXPECT_H */

