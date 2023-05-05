/* $Id$ */
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

// @file    active.c
// @author  Russ Combs <rcombs@sourcefire.com>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_DUMBNET_H
#include <dumbnet.h>
#else
#include <dnet.h>
#endif

#include "active.h"
#include "session_api.h"
#include "stream_api.h"
#include "snort.h"

#include "preprocessors/spp_frag3.h"

#ifdef ACTIVE_RESPONSE
#include "encode.h"
#include "sfdaq.h"
#endif

// these can't be pkt flags because we do the handling
// of these flags following all processing and the drop
// or response may have been produced by a pseudopacket.
tActiveDrop active_drop_pkt = ACTIVE_ALLOW;
tActiveSsnDrop active_drop_ssn = ACTIVE_SSN_ALLOW;
// TBD consider performance of replacing active_drop_pkt/ssn
// with a active_verdict.  change over if it is a wash or better.

int active_tunnel_bypass = 0;
int active_suspend = 0;

#ifdef ACTIVE_RESPONSE
int active_have_rsp = 0;

#define MAX_ATTEMPTS 20
static uint8_t s_attempts = 0;
static int s_enabled = 0;

static eth_t* s_link = NULL;
static ip_t* s_ipnet = NULL;

static void* s_rejData, *s_rspData;
static Active_ResponseFunc s_rejFunc = NULL, s_rspFunc = NULL;

static int Active_Open(const char*);
static int Active_Close(void);

static int Active_SendEth(const DAQ_PktHdr_t*, int, const uint8_t*, uint32_t);
static int Active_SendIp(const DAQ_PktHdr_t*, int, const uint8_t*, uint32_t);

typedef int (*send_t) (
    const DAQ_PktHdr_t* h, int rev, const uint8_t* buf, uint32_t len);
static send_t s_send = DAQ_Inject;
static uint64_t s_injects = 0;

static inline PROTO_ID GetInnerProto (const Packet* p)
{
    if ( !p->next_layer ) return PROTO_MAX;
    return ( p->layers[p->next_layer-1].proto );
}

//--------------------------------------------------------------------
// this implementation ensures that flexible responses
// take precedence over active responses.

int Active_QueueReject (void)
{
    if ( Active_Suspended() )
        return 0;

    if ( !s_rejFunc )
    {
        s_rejFunc = (Active_ResponseFunc)Active_KillSession;
        s_rejData = NULL;
        active_have_rsp = 1;
    }
    return 0;
}

int Active_QueueResponse (Active_ResponseFunc f, void* pv)
{
    if ( Active_Suspended() )
        return 0;

    if ( !s_rspFunc )
    {
        s_rspFunc = f;
        s_rspData = pv;
        active_have_rsp = 1;
    }
    return 0;
}

// helper function
static inline void Active_ClearQueue (void)
{
    s_rejFunc = s_rspFunc = NULL;
    s_rejData = s_rspData = NULL;
}

int Active_ResetQueue ()
{
    Active_ClearQueue();
    return 0;
}

int Active_SendResponses (Packet* p)
{
    if ( s_rspFunc )
    {
        s_rspFunc(p, s_rspData);
    }
    else if ( s_rejFunc )
    {
        s_rejFunc(p, s_rejData);
    }
    else
    {
        return 0;
    }
    if ( p->ssnptr && session_api )
    {
        session_api->init_active_response(p, p->ssnptr);
    }
    Active_ClearQueue();
    return 1;
}

//--------------------------------------------------------------------

void Active_KillSession (Packet* p, EncodeFlags* pf)
{
    EncodeFlags flags = pf ? *pf : ENC_FLAG_FWD;

    if ( !IsIP(p) )
        return;

    switch ( GET_IPH_PROTO(p) )
    {
        case IPPROTO_TCP:
            Active_SendReset(p, 0);
            if ( flags & ENC_FLAG_FWD )
                Active_SendReset(p, ENC_FLAG_FWD);
            break;

        default:
            if ( Active_PacketForceDropped() )
                Active_SendUnreach(p, ENC_UNR_FW);
            else
                Active_SendUnreach(p, ENC_UNR_PORT);
            break;
    }
}

//--------------------------------------------------------------------

int Active_Init (SnortConfig* sc)
{
    s_attempts = sc->respond_attempts;
    if ( s_attempts > MAX_ATTEMPTS ) s_attempts = MAX_ATTEMPTS;
    if ( s_enabled && !s_attempts ) s_attempts = 1;

    if ( s_enabled && (!DAQ_CanInject() || sc->respond_device) )
    {

        if ( ScReadMode() || Active_Open(sc->respond_device) )
        {
            LogMessage("WARNING: active responses disabled since DAQ "
                "can't inject packets.\n");
#ifndef REG_TEST
            s_attempts = s_enabled = 0;
#endif
        }

        if (NULL != sc->eth_dst)
            Encode_SetDstMAC(sc->eth_dst);
    }
    return 0;
}

int Active_Term (void)
{
    Active_Close();
    return 0;
}

int Active_IsEnabled (void) { return s_enabled; }

void Active_SetEnabled (int on_off)
{
    if ( !on_off || on_off > s_enabled )
        s_enabled = on_off;
}

static inline uint32_t GetFlags (void)
{
    uint32_t flags = ENC_FLAG_ID;
    if ( DAQ_RawInjection() || s_ipnet ) flags |= ENC_FLAG_RAW;
    return flags;
}

//--------------------------------------------------------------------

static uint32_t Strafe(int, uint32_t, const Packet*);

void Active_SendReset(Packet* p, EncodeFlags ef)
{
    int i;
    uint32_t flags = (GetFlags() | ef) & ~ENC_FLAG_VAL;
    uint32_t value = ef & ENC_FLAG_VAL;

    for ( i = 0; i < s_attempts; i++ )
    {
        uint32_t len = 0;
        const uint8_t* rej;

        value = Strafe(i, value, p);

        rej = Encode_Reject(ENC_TCP_RST, flags|value, p, &len);
        if ( !rej ) return;

        s_send(p->pkth, !(ef & ENC_FLAG_FWD), rej, len);
    }
}

void Active_SendUnreach(Packet* p, EncodeType type)
{
    uint32_t len;
    const uint8_t* rej;
    uint32_t flags = GetFlags();

    if ( !s_attempts )
        return;

    rej = Encode_Reject(type, flags, p, &len);
    if ( !rej ) return;

    s_send(p->pkth, 1, rej, len);
}

bool Active_SendData (
    Packet* p, EncodeFlags flags, const uint8_t* buf, uint32_t blen)
{
    uint16_t toSend;
    uint16_t sent;
    uint16_t maxPayload;
    const uint8_t* seg;
    uint32_t plen;
    EncodeFlags tmp_flags;

    flags |= GetFlags();
    flags &= ~ENC_FLAG_VAL;

    if (flags & ENC_FLAG_RST_SRVR)
    {
        plen = 0;
        tmp_flags = flags ^ ENC_FLAG_FWD;
        seg = Encode_Response(ENC_TCP_RST, tmp_flags, p, &plen, NULL, 0);

        if ( seg )
            s_send(p->pkth, !(tmp_flags & ENC_FLAG_FWD), seg, plen);
    }

    flags |= ENC_FLAG_SEQ;
    sent = 0;
    maxPayload = Encode_GetMaxPayload(p);

    if(maxPayload)
    {
        do{
            plen = 0;
            toSend = blen > maxPayload ? maxPayload : blen;
            flags = (flags & ~ENC_FLAG_VAL) | sent;
            seg = Encode_Response(ENC_TCP_PUSH, flags, p, &plen, buf, toSend);

            if ( !seg )
                return false;

            s_send(p->pkth, !(flags & ENC_FLAG_FWD), seg, plen);

            buf += toSend;
            sent += toSend;
        } while(blen -= toSend);
    }

    plen = 0;
    flags = (flags & ~ENC_FLAG_VAL) | sent;
    seg = Encode_Response(ENC_TCP_FIN, flags, p, &plen, NULL, 0);

    if ( !seg )
        return false;

    s_send(p->pkth, !(flags & ENC_FLAG_FWD), seg, plen);

    if (flags & ENC_FLAG_RST_CLNT)
    {
        sent++;
        plen = 0;
        flags = (flags & ~ENC_FLAG_VAL) | sent;
        seg = Encode_Response(ENC_TCP_RST, flags, p, &plen, NULL, 0);

        if ( seg )
            s_send(p->pkth, !(flags & ENC_FLAG_FWD), seg, plen);
    }

    return true;
}

void Active_InjectData (
    Packet* p, EncodeFlags flags, const uint8_t* buf, uint32_t blen)
{
    uint32_t plen = 0;
    const uint8_t* seg;

    if ( !s_attempts )
        return;

    flags |= GetFlags();
    flags &= ~ENC_FLAG_VAL;

    seg = Encode_Response(ENC_TCP_PUSH, flags, p, &plen, buf, blen);
    if ( !seg )
        return;

    s_send(p->pkth, !(flags & ENC_FLAG_FWD), seg, plen);
}

void Active_UDPInjectData (
    Packet* p, EncodeFlags flags, const uint8_t* buf, uint32_t blen)
{
    uint32_t plen = 0;
    const uint8_t* seg;

    if ( !s_attempts )
        return;

    flags |= GetFlags();

    seg = Encode_Response(ENC_UDP, flags, p, &plen, buf, blen);
    if ( !seg )
        return;

    s_send(p->pkth, !(flags & ENC_FLAG_FWD ), seg , plen);
}
//--------------------------------------------------------------------

int Active_IsRSTCandidate(const Packet* p)
{
    if ( GetInnerProto(p) != PROTO_TCP )
        return 0;

    if ( !p->tcph )
        return 0;

    /*
    **  This ensures that we don't reset packets that we just
    **  spoofed ourselves, thus inflicting a self-induced DOS
    **  attack.
    */
    return ( !(p->tcph->th_flags & TH_RST) );
}

int Active_IsUNRCandidate(const Packet* p)
{
    // FIXTHIS allow unr to tcp/udp/icmp4/icmp6 only or for all
    switch ( GetInnerProto(p) ) {
    case PROTO_UDP:
    case PROTO_TCP:
    case PROTO_ICMP4:
    case PROTO_ICMP6:
        return 1;

    default:
        break;
    }
    return 0;
}

//--------------------------------------------------------------------
// TBD strafed sequence numbers could be divided by window
// scaling if present.

static uint32_t Strafe (int i, uint32_t flags, const Packet* p)
{
    flags &= ENC_FLAG_VAL;

    switch ( i ) {
    case 0:
        flags |= ENC_FLAG_SEQ;
        break;

    case 1:
        flags = p->dsize;
        flags &= ENC_FLAG_VAL;
        flags |= ENC_FLAG_SEQ;
        break;

    case 2:
    case 3:
        flags += (p->dsize << 1);
        flags &= ENC_FLAG_VAL;
        flags |= ENC_FLAG_SEQ;
        break;

    case 4:
        flags += (p->dsize << 2);
        flags &= ENC_FLAG_VAL;
        flags |= ENC_FLAG_SEQ;
        break;

    default:
        flags += (ntohs(p->tcph->th_win) >> 1);
        flags &= ENC_FLAG_VAL;
        flags |= ENC_FLAG_SEQ;
        break;
    }
    return flags;
}
#endif  // ACTIVE_RESPONSE

//--------------------------------------------------------------------
// support for decoder and rule actions

static inline void _Active_ForceIgnoreSession(Packet *p)
{
    if (p->ssnptr && stream_api)
    {
        stream_api->drop_packet(p);
    }

    //drop this and all following fragments
    frag3DropAllFragments(p);
}

static inline void _Active_DoIgnoreSession(Packet *p)
{
    if ( ScIpsInlineMode() || ScTreatDropAsIgnore() )
    {
        _Active_ForceIgnoreSession(p);
    }
}

int Active_IgnoreSession (Packet* p)
{
    Active_DropPacket(p);

    _Active_DoIgnoreSession(p);

    return 0;
}

int Active_ForceDropAction(Packet *p)
{
    if ( !IsIP(p) )
        return 0;

    // explicitly drop packet
    Active_ForceDropPacket();

    switch ( GET_IPH_PROTO(p) )
    {
        case IPPROTO_TCP:
        case IPPROTO_UDP:
            Active_DropSession(p);
            _Active_ForceIgnoreSession(p);
    }
    return 0;
}

static inline int _Active_DoReset(Packet *p)
{
#ifdef ACTIVE_RESPONSE
    if ( !Active_IsEnabled() )
        return 0;

    if ( !IPH_IS_VALID(p) )
        return 0;

    switch ( GET_IPH_PROTO(p) )
    {
        case IPPROTO_TCP:
            if ( Active_IsRSTCandidate(p) )
                Active_QueueReject();
            break;

        // FIXTHIS send unr to udp/icmp4/icmp6 only or for all non-tcp?
        case IPPROTO_UDP:
        case IPPROTO_ICMP:
        case IPPROTO_ICMPV6:
            if ( Active_IsUNRCandidate(p) )
                Active_QueueReject();
            break;
    }
#endif

    return 0;
}

int Active_DropAction (Packet* p)
{
    Active_IgnoreSession(p);

#ifdef ACTIVE_RESPONSE
    if (( s_enabled < 2 ) || (active_drop_ssn == ACTIVE_SSN_DROP_WITHOUT_RESET))
        return 0;
#endif

    return _Active_DoReset(p);
}

int Active_ForceDropResetAction(Packet *p)
{
    Active_ForceDropAction(p);

    return _Active_DoReset(p);
}

//--------------------------------------------------------------------
// support for non-DAQ injection

#ifdef ACTIVE_RESPONSE
static int Active_Open (const char* dev)
{
    if ( dev && strcasecmp(dev, "ip") )
    {
        s_link = eth_open(dev);

        if ( !s_link )
            FatalError("%s: can't open %s!\n",
                "Active response", dev);
        s_send = Active_SendEth;
    }
    else
    {
        s_ipnet = ip_open();

        if ( !s_ipnet )
            FatalError("%s: can't open ip!\n",
                "Active response");
        s_send = Active_SendIp;
    }
    return ( s_link || s_ipnet ) ? 0 : -1;
}

static int Active_Close (void)
{
    if ( s_link )
        eth_close(s_link);

    if ( s_ipnet )
        ip_close(s_ipnet);

    s_link = NULL;
    s_ipnet = NULL;

    return 0;
}

static int Active_SendEth (
    const DAQ_PktHdr_t* h, int rev, const uint8_t* buf, uint32_t len)
{
    ssize_t sent = eth_send(s_link, buf, len);
    s_injects++;
    return ( (uint32_t) sent != len );
}

static int Active_SendIp (
    const DAQ_PktHdr_t* h, int rev, const uint8_t* buf, uint32_t len)
{
    ssize_t sent = ip_send(s_ipnet, buf, len);
    s_injects++;
    return ( (uint32_t) sent != len );
}

uint64_t Active_GetInjects (void) { return s_injects; }
#endif

