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

#ifdef NORMALIZER

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#ifdef HAVE_DUMBNET_H
#include <dumbnet.h>
#else
#include <dnet.h>
#endif

#include "normalize.h"
#include "perf.h"
#include "sfdaq.h"

typedef enum {
    PC_IP4_TRIM,
    PC_IP4_TOS,
    PC_IP4_DF,
    PC_IP4_RF,
    PC_IP4_TTL,
    PC_IP4_OPTS,
    PC_ICMP4_ECHO,
    PC_IP6_TTL,
    PC_IP6_OPTS,
    PC_ICMP6_ECHO,
    PC_TCP_SYN_OPT,
    PC_TCP_OPT,
    PC_TCP_PAD,
    PC_TCP_RSV,
    PC_TCP_NS,
    PC_TCP_URP,
    PC_TCP_ECN_PKT,
    PC_TCP_TS_ECR,
    PC_TCP_REQ_URG,
    PC_TCP_REQ_PAY,
    PC_TCP_REQ_URP,
    PC_MAX
} PegCounts;

static const char* pegName[PC_MAX] = {
    "ip4::trim",
    "ip4::tos",
    "ip4::df",
    "ip4::rf",
    "ip4::ttl",
    "ip4::opts",
    "icmp4::echo",
    "ip6::ttl",
    "ip6::opts",
    "icmp6::echo",
    "tcp::syn_opt",
    "tcp::opt",
    "tcp::pad",
    "tcp::rsv",
    "tcp::ns",
    "tcp::urp",
    "tcp::ecn_pkt",
    "tcp::ts_ecr",
    "tcp::req_urg",
    "tcp::req_pay",
    "tcp::req_urp",
};

static PegCount normStats[PC_MAX][NORM_MODE_MAX];

//static int Norm_Eth(Packet*, uint8_t layer, int changes);
static int Norm_IP4(NormalizerContext*, Packet*, uint8_t layer, int changes);
static int Norm_ICMP4(NormalizerContext*, Packet*, uint8_t layer, int changes);
static int Norm_IP6(NormalizerContext*, Packet*, uint8_t layer, int changes);
static int Norm_ICMP6(NormalizerContext*, Packet*, uint8_t layer, int changes);
static int Norm_IP6_Opts(NormalizerContext*, Packet*, uint8_t layer, int changes);
//static int Norm_UDP(NormalizerContext*, Packet*, uint8_t layer, int changes);
static int Norm_TCP(NormalizerContext*, Packet*, uint8_t layer, int changes);

static const uint8_t MAX_EOL_PAD[TCP_OPTLENMAX] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// go from inner to outer
int Norm_Packet (NormalizerContext* c, Packet* p)
{
    uint8_t lyr = p->next_layer;
    int changes = 0;

    while ( lyr > 0 )
    {
        PROTO_ID proto = p->layers[--lyr].proto;
        Normalizer n = c->normalizers[proto];
        if ( n ) changes = n(c, p, lyr, changes);
    }

    if ( changes > 0 )
    {
        p->packet_flags |= PKT_MODIFIED;
        return 1;
    }
    if ( p->packet_flags & PKT_RESIZED )
    {
        return 1;
    }
    return 0;
}

//-----------------------------------------------------------------------
// in the following code, we mostly use the actual packet data and the
// packet layers[].  use of other decoded packet members is largely
// avoided to ensure that we don't get tripped up by nested protocols.
// TCP options count and length are a notable exception.
//
// also note that checksums are not calculated here.  they are only
// calculated once after all normalizations are done (here, stream5)
// and any replacements are made.
//-----------------------------------------------------------------------

#if 0
static int Norm_Eth (Packet * p, uint8_t layer, int changes)
{
    return 0;
}
#endif

//-----------------------------------------------------------------------

#define IP4_FLAG_RF 0x8000
#define IP4_FLAG_DF 0x4000
#define IP4_FLAG_MF 0x2000

// TBD support configurable minimum length / obtain from DAQ
// ether header + min payload (excludes FCS, which makes it 64 total)
#define ETH_MIN_LEN 60

static inline int getNormMode (NormalizerContext* c)
{
    return c->normMode;
}

static int Norm_IP4 (NormalizerContext* c, Packet * p, uint8_t layer, int changes)
{
    IPHdr* h = (IPHdr*)(p->layers[layer].start);
    uint16_t fragbits = ntohs(h->ip_off);
    uint16_t origbits = fragbits;
    NormMode mode = getNormMode(c);

    if ( Norm_IsEnabled(c, NORM_IP4_TRIM) && (layer == 1) )
    {
        uint32_t len = p->layers[0].length + ntohs(h->ip_len);

        if ( (len < p->pkth->pktlen) && 
           ( (len >= ETH_MIN_LEN) || (p->pkth->pktlen > ETH_MIN_LEN) )
        ) {
            if ( mode == NORM_MODE_ON )
            {
                ((DAQ_PktHdr_t*)p->pkth)->pktlen = (len < ETH_MIN_LEN) ? ETH_MIN_LEN : len;
                p->packet_flags |= PKT_RESIZED;
                changes++;
            }
            normStats[PC_IP4_TRIM][mode]++;
            sfBase.iPegs[PERF_COUNT_IP4_TRIM][mode]++;
        }
    }
    if ( Norm_IsEnabled(c, NORM_IP4_TOS) )
    {
        if ( h->ip_tos )
        {
            if ( mode == NORM_MODE_ON )
            {
                h->ip_tos = 0;
                changes++;
            }
            normStats[PC_IP4_TOS][mode]++;
            sfBase.iPegs[PERF_COUNT_IP4_TOS][mode]++;
        }
    }
#if 0
    if ( Norm_IsEnabled(c, NORM_IP4_ID) )
    {
        // TBD implement IP ID normalization / randomization
    }
#endif
    if ( Norm_IsEnabled(c, NORM_IP4_DF) )
    {
        if ( fragbits & IP4_FLAG_DF )
        {
            if ( mode == NORM_MODE_ON )
            {
                fragbits &= ~IP4_FLAG_DF;
                changes++;
            }
            normStats[PC_IP4_DF][mode]++;
            sfBase.iPegs[PERF_COUNT_IP4_DF][mode]++;
        }
    }
    if ( Norm_IsEnabled(c, NORM_IP4_RF) )
    {
        if ( fragbits & IP4_FLAG_RF )
        {
            if ( mode == NORM_MODE_ON )
            {
                fragbits &= ~IP4_FLAG_RF;
                changes++;
            }
            normStats[PC_IP4_RF][mode]++;
            sfBase.iPegs[PERF_COUNT_IP4_RF][mode]++;
        }
    }
    if ( fragbits != origbits )
    {
        h->ip_off = htons(fragbits);
    }
    if ( Norm_IsEnabled(c, NORM_IP4_TTL) )
    {
        if ( h->ip_ttl < ScMinTTL() )
        {
            if ( mode == NORM_MODE_ON )
            {
                h->ip_ttl = ScNewTTL();
                p->error_flags &= ~PKT_ERR_BAD_TTL;
                changes++;
            }
            normStats[PC_IP4_TTL][mode]++;
            sfBase.iPegs[PERF_COUNT_IP4_TTL][mode]++;
        }
    }
    if ( p->layers[layer].length > IP_HEADER_LEN )
    {
        if ( mode == NORM_MODE_ON )
        {
            uint8_t* opts = p->layers[layer].start + IP_HEADER_LEN;
            uint8_t len = p->layers[layer].length - IP_HEADER_LEN;
            // expect len > 0 because IHL yields a multiple of 4
            memset(opts, IPOPT_NOP, len);
            changes++;
        }
        normStats[PC_IP4_OPTS][mode]++;
        sfBase.iPegs[PERF_COUNT_IP4_OPTS][mode]++;
    }
    return changes;
}

//-----------------------------------------------------------------------

static int Norm_ICMP4 (NormalizerContext* c, Packet * p, uint8_t layer, int changes)
{
    ICMPHdr* h = (ICMPHdr*)(p->layers[layer].start);
    NormMode mode = getNormMode(c);

    if ( (h->type == ICMP_ECHO || h->type == ICMP_ECHOREPLY) &&
         (h->code != 0) )
    {
        if ( mode == NORM_MODE_ON )
        {
            h->code = 0;
            changes++;
        }
        normStats[PC_ICMP4_ECHO][mode]++;
        sfBase.iPegs[PERF_COUNT_ICMP4_ECHO][mode]++;
    }
    return changes;
}

//-----------------------------------------------------------------------

static int Norm_IP6 (NormalizerContext* c, Packet * p, uint8_t layer, int changes)
{
    IP6RawHdr* h = (IP6RawHdr*)(p->layers[layer].start);
    NormMode mode = getNormMode(c);

    if ( Norm_IsEnabled(c, NORM_IP6_TTL) )
    {
        if ( h->ip6hops < ScMinTTL() )
        {
            if ( mode == NORM_MODE_ON )
            {
                h->ip6hops = ScNewTTL();
                p->error_flags &= ~PKT_ERR_BAD_TTL;
                changes++;
            }
            normStats[PC_IP6_TTL][mode]++;
            sfBase.iPegs[PERF_COUNT_IP6_TTL][mode]++;
        }
    }
    return changes;
}

//-----------------------------------------------------------------------

static int Norm_ICMP6 (NormalizerContext* c, Packet * p, uint8_t layer, int changes)
{
    ICMPHdr* h = (ICMPHdr*)(p->layers[layer].start);
    NormMode mode = getNormMode(c);

    if ( (h->type == ICMP6_ECHO || h->type == ICMP6_REPLY) &&
         (h->code != 0) )
    {
        if ( mode == NORM_MODE_ON )
        {
            h->code = 0;
            changes++;
        }
        normStats[PC_ICMP6_ECHO][mode]++;
        sfBase.iPegs[PERF_COUNT_ICMP6_ECHO][mode]++;
    }
    return changes;
}

//-----------------------------------------------------------------------
// we assume here that the decoder has not pushed ip6 option extension
// headers unless the basic sizing is correct (size = N*8 octetes, N>0).

typedef struct
{
    uint8_t next;
    uint8_t xlen;
    uint8_t type;
    uint8_t olen;
} ExtOpt;

#define IP6_OPT_PAD_N 1

static int Norm_IP6_Opts (NormalizerContext* c, Packet * p, uint8_t layer, int changes)
{
    NormMode mode = getNormMode(c);

    if ( mode == NORM_MODE_ON )
    {
        uint8_t* b = p->layers[layer].start;
        ExtOpt* x = (ExtOpt*)b;
    
        // whatever was here, turn it into one PADN option
        x->type = IP6_OPT_PAD_N;
        x->olen = (x->xlen * 8) + 8 - sizeof(*x);
        memset(b+sizeof(*x), 0, x->olen);
    
        changes++;
    }
    normStats[PC_IP6_OPTS][mode]++;
    sfBase.iPegs[PERF_COUNT_IP6_OPTS][mode]++;

    return changes;
}

//-----------------------------------------------------------------------

#if 0
static int Norm_UDP (Packet * p, uint8_t layer, int changes)
{
    return 0;
}
#endif

//-----------------------------------------------------------------------

static inline void NopDaOpt (uint8_t* opt, uint8_t len)
{
    memset(opt, TCPOPT_NOP, len);
}

#define TS_ECR_OFFSET 6
#define TS_ECR_LENGTH 4

static inline int Norm_TCPOptions (NormalizerContext* context, uint8_t* opts, size_t len, const TCPHdr* h, uint8_t numOpts, int changes)
{
    size_t i = 0;
    uint8_t c = 0;
    NormMode mode = getNormMode(context);

    while ( (i < len) && (opts[i] != TCPOPT_EOL) &&
        (c++ < numOpts) )
    {
        uint8_t olen = ( opts[i] <= 1 ) ? 1 : opts[i+1];

        // we know that the first numOpts options have valid lengths
        // so we should not need to check individual or total option lengths.
        // however, we keep this as a sanity check.
        if ( i + olen > len)
            break;

        switch ( opts[i] )
        {
        case TCPOPT_NOP:
            break;

        case TCPOPT_MAXSEG:
        case TCPOPT_WSCALE:
            if ( !(h->th_flags & TH_SYN) )
            {
                if ( mode == NORM_MODE_ON )
                {
                    NopDaOpt(opts+i, olen);
                    changes++;
                }
                normStats[PC_TCP_SYN_OPT][mode]++;
                sfBase.iPegs[PERF_COUNT_TCP_SYN_OPT][mode]++;
            }
            break;

        case TCPOPT_TIMESTAMP:
            if ( !(h->th_flags & TH_ACK) &&
                // use memcmp because opts have arbitrary alignment
                memcmp(opts+i+TS_ECR_OFFSET, MAX_EOL_PAD, TS_ECR_LENGTH) )
            {
                if ( mode == NORM_MODE_ON )
                {
                    // TSecr should be zero unless ACK is set
                    memset(opts+i+TS_ECR_OFFSET, 0, TS_ECR_LENGTH);
                    changes++;
                }
                normStats[PC_TCP_TS_ECR][mode]++;
                sfBase.iPegs[PERF_COUNT_TCP_TS_ECR][mode]++;
            }
            break;

        default:
            if ( !Norm_TcpIsOptional(context, opts[i]) )
            {
                if ( mode == NORM_MODE_ON )
                {
                    NopDaOpt(opts+i, olen);
                    changes++;
                }
                normStats[PC_TCP_OPT][mode]++;
                sfBase.iPegs[PERF_COUNT_TCP_OPT][mode]++;
            }
        }
        i += olen;
    }
    if ( ++i < len && memcmp(opts+i, MAX_EOL_PAD, len-i) )
    {
        if ( mode == NORM_MODE_ON )
        {
            memset(opts+i, 0, len-i);
            changes++;
        }
        normStats[PC_TCP_PAD][mode]++;
        sfBase.iPegs[PERF_COUNT_TCP_PAD][mode]++;
    }
    return changes;
}

static inline int Norm_TCPPadding (NormalizerContext* context, uint8_t* opts, size_t len, uint8_t numOpts, int changes)
{
    size_t i = 0;
    uint8_t c = 0;
    NormMode mode = getNormMode(context);

    while ( (i < len) && (opts[i] != TCPOPT_EOL) && (c++ < numOpts) )
    {
        i += ( opts[i] <= 1 ) ? 1 : opts[i+1];
    }
    if ( ++i < len && memcmp(opts+i, MAX_EOL_PAD, len-i) )
    {
        if ( mode == NORM_MODE_ON )
        {
            memset(opts+i, 0, len-i);
            changes++;
        }
        normStats[PC_TCP_PAD][mode]++;
        sfBase.iPegs[PERF_COUNT_TCP_PAD][mode]++;
    }
    return changes;
}

static int Norm_TCP (NormalizerContext* c, Packet * p, uint8_t layer, int changes)
{
    NormMode mode = getNormMode(c);
    TCPHdr* h = (TCPHdr*)(p->layers[layer].start);

    if ( Norm_IsEnabled(c, NORM_TCP_RSV) )
    {
        if ( h->th_offx2 & TH_RSV )
        {
            if ( mode == NORM_MODE_ON )
            {
                h->th_offx2 &= ~TH_RSV;
                changes++;
            }
            normStats[PC_TCP_RSV][mode]++;
            sfBase.iPegs[PERF_COUNT_TCP_RSV][mode]++;
        }
    }
    if ( Norm_IsEnabled(c, NORM_TCP_ECN_PKT) )
    {
        if ( h->th_flags & (TH_CWR|TH_ECE) )
        {
            if ( mode == NORM_MODE_ON )
            {
                h->th_flags &= ~(TH_CWR|TH_ECE);
                changes++;
            }
            normStats[PC_TCP_ECN_PKT][mode]++;
            sfBase.iPegs[PERF_COUNT_TCP_ECN_PKT][mode]++;
        }
        if ( h->th_offx2 & TH_NS )
        {
            if ( mode == NORM_MODE_ON )
            {
                h->th_offx2 &= ~TH_NS;
                changes++;
            }
            normStats[PC_TCP_NS][mode]++;
            sfBase.iPegs[PERF_COUNT_TCP_NS][mode]++;
        }
    }
    if ( h->th_urp )
    {
        if ( !(h->th_flags & TH_URG) )
        {
            if ( Norm_IsEnabled(c, NORM_TCP_REQ_URG) )
            {
                if ( mode == NORM_MODE_ON )
                {
                    h->th_urp = 0;
                    changes++;
                }
                normStats[PC_TCP_REQ_URG][mode]++;
                sfBase.iPegs[PERF_COUNT_TCP_REQ_URG][mode]++;
            }
        }
        else if ( !p->dsize )
        {
            if ( Norm_IsEnabled(c, NORM_TCP_REQ_PAY) )
            {
                if ( mode == NORM_MODE_ON )
                {
                    h->th_flags &= ~TH_URG;
                    h->th_urp = 0;
                    changes++;
                }
                normStats[PC_TCP_REQ_PAY][mode]++;
                sfBase.iPegs[PERF_COUNT_TCP_REQ_PAY][mode]++;
            }
        }
        else if ( (ntohs(h->th_urp) > p->dsize) )
        {
            if ( Norm_IsEnabled(c, NORM_TCP_URP) )
            {
                if ( mode == NORM_MODE_ON )
                {
                    h->th_urp = ntohs(p->dsize);
                    changes++;
                }
                normStats[PC_TCP_URP][mode]++;
                sfBase.iPegs[PERF_COUNT_TCP_URP][mode]++;
            }
        }
    }
    else if ( Norm_IsEnabled(c, NORM_TCP_REQ_URP) &&
        h->th_flags & TH_URG )
    {
        if ( mode == NORM_MODE_ON )
        {
            h->th_flags &= ~TH_URG;
            changes++;
        }
        normStats[PC_TCP_REQ_URP][mode]++;
        sfBase.iPegs[PERF_COUNT_TCP_REQ_URP][mode]++;
    }
    if ( p->tcp_options_len > 0 )
    {
        uint8_t* opts = p->layers[layer].start + TCP_HEADER_LEN;

        if ( Norm_IsEnabled(c, NORM_TCP_OPT) )
        {
            changes = Norm_TCPOptions(c, opts, p->tcp_options_len,
                h, p->tcp_option_count, changes);
        }
        else if ( Norm_IsEnabled(c, NORM_TCP_PAD) )
        {
            changes = Norm_TCPPadding(c, opts, p->tcp_options_len,
                p->tcp_option_count, changes);
        }
    }
    return changes;
}

//-----------------------------------------------------------------------

void Norm_PrintStats (void)
{
    int i;
    LogMessage("Normalizer statistics:\n");

    for ( i = 0; i < PC_MAX; i++ )
    {
        // for now, 23 aligns with frag3
        LogMessage("%23s: " STDu64 "\n", pegName[i], normStats[i][NORM_MODE_ON]);
        LogMessage("Would %17s: " STDu64 "\n", pegName[i], normStats[i][NORM_MODE_WOULDA]);
    }
}

void Norm_ResetStats (void)
{
    memset(normStats, 0, sizeof(normStats));
}

//-----------------------------------------------------------------------

int Norm_SetConfig (NormalizerContext* nc)
{
    if ( !DAQ_CanReplace() )
    {
        LogMessage("WARNING: normalizations disabled because DAQ"
            " can't replace packets.\n");
        nc->normalizer_flags = 0x0;
        return -1;
    }
    if ( !nc->normalizer_flags )
    {
        return 0;
    }
    if ( Norm_IsEnabled(nc, NORM_IP4) )
    {
        nc->normalizers[PROTO_IP4] = Norm_IP4;
    }
    if ( Norm_IsEnabled(nc, NORM_IP4_TRIM) )
    {
        if ( !DAQ_CanInject() )
        {
            LogMessage("WARNING: normalize_ip4: trim disabled since DAQ "
                "can't inject packets.\n");
            Norm_Disable(nc, NORM_IP4_TRIM);
        }
    }
    if ( Norm_IsEnabled(nc, NORM_ICMP4) )
    {
        nc->normalizers[PROTO_ICMP4] = Norm_ICMP4;
    }
    if ( Norm_IsEnabled(nc, NORM_IP6) )
    {
        nc->normalizers[PROTO_IP6] = Norm_IP6;
        nc->normalizers[PROTO_IP6_HOP_OPTS] = Norm_IP6_Opts;
        nc->normalizers[PROTO_IP6_DST_OPTS] = Norm_IP6_Opts;
    }
    if ( Norm_IsEnabled(nc, NORM_ICMP6) )
    {
        nc->normalizers[PROTO_ICMP6] = Norm_ICMP6;
    }
    if ( Norm_IsEnabled(nc, NORM_TCP) )
    {
        nc->normalizers[PROTO_TCP] = Norm_TCP;
    }
    return 0;
}
#endif  // NORMALIZER

