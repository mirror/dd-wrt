/* $Id$ */
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
#ifndef __ACTIVE_H__
#define __ACTIVE_H__

#include "decode.h"
#include "snort.h"

#ifdef ACTIVE_RESPONSE
#include "encode.h"

int Active_Init(SnortConfig*);
int Active_Term(void);

typedef void (*Active_ResponseFunc)(Packet*, void* data);

int Active_QueueReject(void);
int Active_QueueResponse(Active_ResponseFunc, void*);
int Active_ResetQueue(void);

// this must be called on the wire packet and not a
// reassembled packet so that encoding is correct.
int Active_SendResponses(Packet*);
uint64_t Active_GetInjects(void);

// NULL flags implies ENC_FLAG_FWD
void Active_KillSession(Packet*, EncodeFlags*);

void Active_SendReset(Packet*, EncodeFlags);
void Active_SendUnreach(Packet*, EncodeType);
void Active_SendData(Packet*, EncodeFlags, const uint8_t* buf, uint32_t len);

int Active_IsRSTCandidate(const Packet*);
int Active_IsUNRCandidate(const Packet*);

int Active_IsEnabled(void);
void Active_SetEnabled(int on_off);
#endif // ACTIVE_RESPONSE

extern int active_drop_pkt;
extern int active_drop_ssn;
#ifdef ACTIVE_RESPONSE
extern int active_have_rsp;
#endif

static INLINE void Active_Reset (void)
{
    active_drop_pkt = active_drop_ssn = 0;
#ifdef ACTIVE_RESPONSE
    active_have_rsp = 0;
#endif
}

static INLINE void Active_DropPacket (void)
{
    if ( ScInlineMode() )
    {
        active_drop_pkt = 1;
    }
    else if (ScInlineTestMode())
    {
        active_drop_pkt = 2;
    }
}

static INLINE void Active_DropSession (void)
{
    active_drop_ssn = 1;
    Active_DropPacket();
}

static INLINE int Active_PacketWasDropped (void)
{
    return ( active_drop_pkt == 1 );
}

static INLINE int Active_PacketWouldBeDropped (void)
{
    return (active_drop_pkt == 2 );
}

static INLINE int Active_SessionWasDropped (void)
{
    return ( active_drop_ssn != 0 );
}

#ifdef ACTIVE_RESPONSE
static INLINE int Active_ResponseQueued (void)
{
    return ( active_have_rsp != 0 );
}
#endif

// drops current session with active response invoked
// for rules with action = drop | sdrop | reject
int Active_DropAction(Packet*);

// drops current session w/o active response invoked
// for rules with custom response = resp3 | react
int Active_IgnoreSession(Packet*);

#endif // __ACTIVE_H__

