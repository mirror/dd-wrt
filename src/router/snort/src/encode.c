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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <dnet.h>

#include "encode.h"
#include "sfdaq.h"
#include "sf_iph.h"
#include "snort.h"
#include "stream_api.h"

#define GET_IP_HDR_LEN(h) (((h)->ip_verhl & 0x0f) << 2)
#define GET_TCP_HDR_LEN(h) (((h)->th_offx2 & 0xf0) >> 2)
#define SET_TCP_HDR_LEN(h, n) (h)->th_offx2 = ((n << 2) & 0xF0)

#define MAX_TTL            255
#define ICMP_UNREACH_DATA    8  // (per RFC 792)
#define IP_ID_COUNT       8192

static INLINE int IsIcmp (int type)
{
    static int s_icmp[ENC_MAX] = { 0, 0, 1, 1, 1 };
    return ( s_icmp[type] );
}

//-------------------------------------------------------------------------
// encoders operate layer by layer:
// * base+off is start of packet
// * base+end is start of current layer
// * base+size-1 is last byte of packet (in) / buffer (out)
typedef blob_t Buffer;

typedef enum {
    ENC_OK, ENC_BAD_PROTO, ENC_BAD_OPT, ENC_OVERFLOW
} ENC_STATUS;

typedef struct {
    EncodeType type;
    EncodeFlags flags;

    uint8_t layer;
    const Packet* p;
    uint16_t ip_len;
    uint8_t* ip_hdr;

    const uint8_t* payLoad;
    uint32_t payLen;
    uint8_t proto;

} EncState;

#define FORWARD(e) (e->flags & ENC_FLAG_FWD)
#define REVERSE(f) (!(f & ENC_FLAG_FWD))

#define PKT_SZ (ETHERNET_HEADER_LEN + VLAN_HEADER_LEN + IP_MAXPACKET)

// all layer encoders look like this:
typedef ENC_STATUS (*Encoder)(EncState*, Buffer* in, Buffer* out);
typedef ENC_STATUS (*Updater)(Packet*, Layer*, uint32_t* len);
typedef void (*Formatter)(EncodeFlags, const Packet* p, Packet* c, Layer*);
// TBD implement other encoder functions

typedef struct {
    Encoder fencode;
    Updater fupdate;
    Formatter fformat;
} EncoderFunctions;

// forward declaration; definition at end of file
static EncoderFunctions encoders[PROTO_MAX];

static void IpId_Init();
static void IpId_Term();

static const uint8_t* Encode_Packet(
    EncState* enc, const Packet* p, uint32_t* len);

static ENC_STATUS UN6_Encode(EncState*, Buffer*, Buffer*);

//-------------------------------------------------------------------------

static INLINE PROTO_ID NextEncoder (EncState* enc)
{
    if ( enc->layer < enc->p->next_layer )
    {
        PROTO_ID next = enc->p->layers[enc->layer++].proto;
        if ( next < PROTO_MAX )
        {
            if ( encoders[next].fencode ) return next;
        }
    }
    return PROTO_MAX;
}

//-------------------------------------------------------------------------
// basic setup stuff
//-------------------------------------------------------------------------

void Encode_Init (void)
{
    IpId_Init();
}

void Encode_Term (void)
{
    IpId_Term();
}

//-------------------------------------------------------------------------
// encoders:
// - raw pkt data only, no need for Packet stuff except to facilitate
//   encoding
// - don't include original options
// - inner layer differs from original (eg tcp data segment becomes rst)
// - must ensure proper ttl/hop limit for reverse direction
// - sparc twiddle must be factored in packet start for transmission
//
// iterate over decoded layers and encode the response packet.  actually
// make nested calls.  on the way in we setup invariant stuff and as we
// unwind the stack we finish up encoding in a more normal fashion (now
// the outer layer knows the length of the inner layer, etc.).
//
// when multiple responses are sent, both forwards and backwards directions,
// or multiple ICMP types (unreachable port, host, net), it may be possible
// to reuse the 1st encoding and just tweak it.  optimization for later 
// consideration.

// pci is copied from in to out
// * addresses / ports are swapped if !fwd
// * options, etc. are stripped
// * checksums etc. are set
// * if next layer is udp, it is set to icmp unreachable w/udp
// * if next layer is tcp, it becomes a tcp rst or tcp fin w/opt data

//-------------------------------------------------------------------------

const uint8_t* Encode_Reject(
    EncodeType type, EncodeFlags flags, const Packet* p, uint32_t* len)
{
    EncState enc;

    enc.type = type;
    enc.flags = flags;

    enc.payLoad = NULL;
    enc.payLen = 0;

    enc.ip_hdr = NULL;
    enc.ip_len = 0;
    enc.proto = 0;

    return Encode_Packet(&enc, p, len);
}

const uint8_t* Encode_Response(
    EncodeType type, EncodeFlags flags, const Packet* p, uint32_t* len,
    const uint8_t* payLoad, uint32_t payLen
) {
    EncState enc;

    enc.type = type;
    enc.flags = flags;

    enc.payLoad = payLoad;
    enc.payLen = payLen;

    enc.ip_hdr = NULL;
    enc.ip_len = 0;
    enc.proto = 0;

    return Encode_Packet(&enc, p, len);
}

//-------------------------------------------------------------------------
// formatters:
// - these packets undergo detection
// - need to set Packet stuff except for frag3 which calls grinder
// - include original options except for frag3 inner ip
// - inner layer header is very similar but payload differs
// - original ttl is always used
//-------------------------------------------------------------------------

int Encode_Format (EncodeFlags f, const Packet* p, Packet* c)
{
    DAQ_PktHdr_t* pkth = (DAQ_PktHdr_t*)c->pkth;
    uint8_t* pkt = (uint8_t*)c->pkt;

    int i, next_layer = p->next_layer;
    Layer* lyr;
    size_t len;

    if ( next_layer < 1 ) return -1;

    memset(c, 0, PKT_ZERO_LEN);
    c->raw_ip6h = NULL;

    c->pkth = pkth;
    c->pkt = pkt;

    if ( f & ENC_FLAG_NET )
    {
        for ( i = next_layer-1; i >= 0; i-- )
            if ( p->layers[i].proto == PROTO_IP4
#ifdef SUP_IP6
              || p->layers[i].proto == PROTO_IP6
#endif
            )
                break;
         if ( i < next_layer ) next_layer = i + 1;
    }
    // copy raw packet data to clone
    lyr = (Layer*)p->layers + next_layer - 1;
    len = lyr->start - p->pkt + lyr->length;

    memcpy((void*)c->pkt, p->pkt, len);

    // set up layers
    for ( i = 0; i < next_layer; i++ )
    {
        const uint8_t* b = c->pkt + (p->layers[i].start - p->pkt);
        lyr = c->layers + i;

        lyr->proto = p->layers[i].proto;
        lyr->length = p->layers[i].length;
        lyr->start = (uint8_t*)b;

        if ( lyr->proto < PROTO_MAX )
            encoders[lyr->proto].fformat(f, p, c, lyr);

#ifdef DEBUG
        else
            FatalError("Encode_New() => unsupported proto = %d\n",
                lyr->proto);
#endif
    }
    c->next_layer = next_layer;

    // setup payload info
    c->data = lyr->start + lyr->length;
    len = c->data - c->pkt;
    c->max_dsize = PKT_SZ - len;
    c->proto_bits = p->proto_bits;
    c->packet_flags |= PKT_PSEUDO;

    // setup pkt capture header
    pkth->caplen = pkth->pktlen = len;
    pkth->ts = p->pkth->ts;

    if ( !c->max_dsize )
        return -1;

    return 0;
}

//-------------------------------------------------------------------------
// updaters:  these functions set length and checksum fields, only needed
// when a packet is modified.  some packets only have replacements so only
// the checksums need to be updated.  we always set the length rather than
// checking each time if needed.
//-------------------------------------------------------------------------

void Encode_Update (Packet* p)
{
    int i;
    uint32_t len = 0;
    DAQ_PktHdr_t* pkth = (DAQ_PktHdr_t*)p->pkth;

    p->actual_ip_len = 0;

    for ( i = p->next_layer - 1; i >= 0; i-- )
    {
        Layer* lyr = p->layers + i;
        encoders[lyr->proto].fupdate(p, lyr, &len);
    }
    // see IP6_Update() for an explanation of this ...
    if ( !(p->packet_flags & PKT_MODIFIED)
#ifdef NORMALIZER
        || (p->packet_flags & PKT_RESIZED)
#endif
    )
        pkth->caplen = pkth->pktlen = len;
}

//-------------------------------------------------------------------------
// internal packet support
//-------------------------------------------------------------------------

Packet* Encode_New ()
{
    Packet* p = SnortAlloc(sizeof(*p));
    uint8_t* b = SnortAlloc(sizeof(*p->pkth) + PKT_SZ + SPARC_TWIDDLE);

    if ( !p || !b )
        FatalError("Encode_New() => Failed to allocate packet\n");

    p->pkth = (void*)b;
    b += sizeof(*p->pkth);
    b += SPARC_TWIDDLE;
    p->pkt = b;

    return p;
}

void Encode_Delete (Packet* p)
{
    free((void*)p->pkth);  // cast away const!
    free(p);
}

//-------------------------------------------------------------------------
// private implementation stuff
//-------------------------------------------------------------------------

static uint8_t s_pkt[ETHERNET_HEADER_LEN+VLAN_HEADER_LEN+IP_MAXPACKET];

static const uint8_t* Encode_Packet(
    EncState* enc, const Packet* p, uint32_t* len)
{
    Buffer ibuf, obuf;
    ENC_STATUS status = ENC_BAD_PROTO;
    PROTO_ID next;

    ibuf.base = (uint8_t*)p->pkt;
    ibuf.off = ibuf.end = 0;
    ibuf.size = p->pkth->caplen;

    obuf.base = s_pkt;
    obuf.off = obuf.end = 0;
    obuf.size = sizeof(s_pkt);

    enc->layer = 0;
    enc->p = p;

    next = NextEncoder(enc);

    if ( next < PROTO_MAX )
    {
        Encoder e = encoders[next].fencode;
        status = (*e)(enc, &ibuf, &obuf);
    }
    if ( status != ENC_OK || enc->layer != p->next_layer )
    {
        *len = 0;
        return NULL;
    }
    *len = (uint32_t)obuf.end;
    return obuf.base + obuf.off;
}

//-------------------------------------------------------------------------
// ip id considerations:
//
// we use dnet's rand services to generate a vector of random 16-bit values and
// iterate over the vector as IDs are assigned.  when we wrap to the beginning,
// the vector is randomly reordered.
//-------------------------------------------------------------------------

static rand_t* s_rand = NULL;
static uint16_t s_id_index = 0;
static uint16_t s_id_pool[IP_ID_COUNT];

static void IpId_Init (void)
{
    if ( s_rand ) rand_close(s_rand);

    s_rand = rand_open();

    if ( !s_rand )
        FatalError("encode::IpId_Init: rand_open() failed.\n");

    rand_get(s_rand, s_id_pool, sizeof(s_id_pool));
}

static void IpId_Term (void)
{
    if ( s_rand ) rand_close(s_rand);
    s_rand = NULL;
}

static INLINE uint16_t IpId_Next ()
{
#ifdef REG_TEST
    uint16_t id = htons(s_id_index + 1);
#else
    uint16_t id = s_id_pool[s_id_index];
#endif
    s_id_index = (s_id_index + 1) % IP_ID_COUNT;

    if ( !s_id_index )
        rand_shuffle(s_rand, s_id_pool, sizeof(s_id_pool), 1);

    return id;
}

//-------------------------------------------------------------------------
// ttl considerations:
//
// we try to use the TTL captured for the session by the stream preprocessor
// when the session started.  if that is not available, we adjust the current
// TTL for forward packets and use the maximum for reverse packets.
//
// AdjTTL() was pulled from flexresp2.  there it was used for both directions.
// however, it doesn't make sense to try to calculate TTL for one direction
// from the other which is why the max is used as the fallback for the reverse
// direction.
//
// for reference, flexresp used a const rand >= 64 in both directions (the
// number was determined at startup and never changed) and react used a const
// 64 in both directions.
//
// note that the ip6 hop limit field is entirely equivalent to the ip4 TTL.
// hop limit is in fact a more accurrate name for the actual usage of this
// field.
//-------------------------------------------------------------------------

static INLINE uint8_t GetTTL (const EncState* enc)
{
    char dir;

    if ( enc->p->packet_flags & PKT_FROM_CLIENT )
        dir = FORWARD(enc) ? SSN_DIR_CLIENT : SSN_DIR_SERVER;
    else
        dir = FORWARD(enc) ? SSN_DIR_SERVER : SSN_DIR_CLIENT;

    return stream_api->get_session_ttl(
        enc->p->ssnptr, dir, !enc->ip_hdr);
}

static INLINE uint8_t AdjTTL (uint8_t ttl)
{
    switch (ttl / 64) 
    {   
    case 3: return 255;
    case 2: return 192;
    case 1: return 128;
    }   
    return 64; 
}

//-------------------------------------------------------------------------
// the if in UPDATE_BOUND can be defined out after testing because:
// 1. the packet was already decoded in decode.c so is structurally sound; and
// 2. encode takes at most the same space as decode.
#define UPDATE_BOUND(buf, n) \
    buf->end += n; \
    if ( buf->end > buf->size ) \
        return ENC_OVERFLOW
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// ethernet
//-------------------------------------------------------------------------

static ENC_STATUS Eth_Encode (EncState* enc, Buffer* in, Buffer* out)
{
    int outer = 0;
    int raw = enc->flags & ENC_FLAG_RAW;

    EtherHdr* hi = (EtherHdr*)enc->p->layers[enc->layer-1].start;
    PROTO_ID next = NextEncoder(enc);

    if ( raw && (out->off == out->end) )
    {
        // for alignment
        out->off = out->end = SPARC_TWIDDLE;
        outer = 1;  // encoding outermost eth
    }
    if ( raw || !outer )
    {
        // we get here for outer-most layer when raw is true;
        // we also get here for any encapsulated ethernet layer.
        EtherHdr* ho = (EtherHdr*)(out->base + out->end);
        UPDATE_BOUND(out, sizeof(*ho));

        if ( FORWARD(enc) )
        {
            memcpy(ho, hi, sizeof(*ho));
        }
        else
        {
            ho->ether_type = hi->ether_type;
            memcpy(ho->ether_src, hi->ether_dst, sizeof(ho->ether_src));
            memcpy(ho->ether_dst, hi->ether_src, sizeof(ho->ether_dst));
        }
    }
    if ( next < PROTO_MAX )
        return encoders[next].fencode(enc, in, out);

    return ENC_OK;
}

static ENC_STATUS Eth_Update (Packet* p, Layer* lyr, uint32_t* len)
{
    *len += lyr->length;
    return ENC_OK;
}

static void Eth_Format (EncodeFlags f, const Packet* p, Packet* c, Layer* lyr)
{
    EtherHdr* ch = (EtherHdr*)lyr->start;
    c->eh = ch;

    if ( REVERSE(f) )
    {
        int i = lyr - c->layers;
        EtherHdr* ph = (EtherHdr*)p->layers[i].start;

        memcpy(ch->ether_dst, ph->ether_src, sizeof(ch->ether_dst));
        memcpy(ch->ether_src, ph->ether_dst, sizeof(ch->ether_src));
    }
}

//-------------------------------------------------------------------------
// VLAN
//-------------------------------------------------------------------------

static void VLAN_Format (EncodeFlags f, const Packet* p, Packet* c, Layer* lyr)
{
    c->vh = (VlanTagHdr*)lyr->start;
}

//-------------------------------------------------------------------------
// GRE
//-------------------------------------------------------------------------
#ifdef GRE
static void GRE_Format (EncodeFlags f, const Packet* p, Packet* c, Layer* lyr)
{
    c->greh = (GREHdr*)lyr->start;
}
#endif

//-------------------------------------------------------------------------
// IP4
//-------------------------------------------------------------------------

static ENC_STATUS IP4_Encode (EncState* enc, Buffer* in, Buffer* out)
{
    int len;
    uint32_t start = out->end;

    IPHdr* hi = (IPHdr*)enc->p->layers[enc->layer-1].start;
    IPHdr* ho = (IPHdr*)(out->base + out->end);
    PROTO_ID next = NextEncoder(enc);
    UPDATE_BOUND(out, sizeof(*ho));

    len = GET_IP_HDR_LEN(hi) - sizeof(*hi);

    ho->ip_verhl = 0x45;
    ho->ip_off = 0;

    ho->ip_id = IpId_Next();
    ho->ip_tos = hi->ip_tos;
    ho->ip_proto = hi->ip_proto;

    if ( FORWARD(enc) )
    {
        uint8_t ttl = AdjTTL(hi->ip_ttl);

        ho->ip_src.s_addr = hi->ip_src.s_addr;
        ho->ip_dst.s_addr = hi->ip_dst.s_addr;

        if ( enc->p->ssnptr )
            ttl = GetTTL(enc);

#ifdef NORMALIZER
        if ( ttl < ScMinTTL() && ScNewTTL() )
            ttl = ScNewTTL();
#endif
        ho->ip_ttl = ttl;
    }
    else
    {
        uint8_t ttl = MAX_TTL;

        ho->ip_src.s_addr = hi->ip_dst.s_addr;
        ho->ip_dst.s_addr = hi->ip_src.s_addr;

        if ( enc->p->ssnptr )
            ttl = GetTTL(enc);

#ifdef NORMALIZER
        if ( ttl < ScMinTTL() && ScNewTTL() )
            ttl = ScNewTTL();
#endif
        ho->ip_ttl = ttl;
    }

    enc->ip_hdr = (uint8_t*)hi;
    enc->ip_len = IP_HLEN(hi) << 2;

    if ( next < PROTO_MAX )
    {
        int err = encoders[next].fencode(enc, in, out);
        if ( err ) return err;
    }  
    if ( enc->proto )
    {
        ho->ip_proto = enc->proto;
        enc->proto = 0;
    }
    len = out->end - start;
    ho->ip_len = htons((u_int16_t)len);
    ip_checksum(ho, len);

    return ENC_OK;
}

static ENC_STATUS IP4_Update (Packet* p, Layer* lyr, uint32_t* len)
{
    IPHdr* h = (IPHdr*)(lyr->start);
    int i = lyr - p->layers;

    *len += GET_IP_HDR_LEN(h);

    if ( i + 1 == p->next_layer )
    {
        *len += p->dsize;
    }
    h->ip_len = htons((u_int16_t)*len);

    if ( !PacketWasCooked(p) || (p->packet_flags & PKT_REBUILT_FRAG) )
        ip_checksum(h, *len);

    return ENC_OK;
}

static void IP4_Format (EncodeFlags f, const Packet* p, Packet* c, Layer* lyr)
{
    // TBD handle nested ip layers
    IPHdr* ch = (IPHdr*)lyr->start;
    c->iph = ch;

    if ( REVERSE(f) )
    {
        int i = lyr - c->layers;
        IPHdr* ph = (IPHdr*)p->layers[i].start;

        ch->ip_src.s_addr = ph->ip_dst.s_addr;
        ch->ip_dst.s_addr = ph->ip_src.s_addr;
    }
    if ( f & ENC_FLAG_DEF )
    {
        int i = lyr - c->layers;
        if ( i + 1 == p->next_layer )
        {
            lyr->length = sizeof(*ch);
            ch->ip_len = htons(lyr->length);
            SET_IP_HLEN(ch, lyr->length >> 2);
        }
    }
#ifdef SUP_IP6
    sfiph_build(c, c->iph, AF_INET);
#endif
}

//-------------------------------------------------------------------------
// ICMP
// UNR encoder creates ICMP unreachable
//-------------------------------------------------------------------------

static INLINE int IcmpCode (EncodeType et) {
    switch ( et ) {
    case ENC_UNR_NET:  return ICMP_UNREACH_NET;
    case ENC_UNR_HOST: return ICMP_UNREACH_HOST;
    case ENC_UNR_PORT: return ICMP_UNREACH_PORT;
    default: break;
    }
    return ICMP_UNREACH_PORT;
}

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t cksum;
    uint32_t unused;
} IcmpHdr;

static ENC_STATUS UN4_Encode (EncState* enc, Buffer* in, Buffer* out)
{
    uint8_t* p;

    uint8_t* hi = enc->p->layers[enc->layer-1].start;
    IcmpHdr* ho = (IcmpHdr*)(out->base + out->end);

#ifdef DEBUG
    if ( enc->type < ENC_UNR_NET )
        return ENC_BAD_OPT;
#endif

    enc->proto = IPPROTO_ICMP;

    UPDATE_BOUND(out, sizeof(*ho));
    ho->type = ICMP_UNREACH;
    ho->code = IcmpCode(enc->type);
    ho->cksum = 0;
    ho->unused = 0;

    // no need to set csum here because ip_checksum() will
    // take care of it.  see Encode_TCP() for details.

    // copy original ip header
    p = out->base + out->end;
    UPDATE_BOUND(out, enc->ip_len);
    memcpy(p, enc->ip_hdr, enc->ip_len);

    // copy first 8 octets of original ip data (ie udp header)
    p = out->base + out->end;
    UPDATE_BOUND(out, ICMP_UNREACH_DATA);
    memcpy(p, hi, ICMP_UNREACH_DATA);

    return ENC_OK;
}

static void ICMP4_Format (EncodeFlags f, const Packet* p, Packet* c, Layer* lyr)
{
    // TBD handle nested icmp4 layers
    c->icmph = (ICMPHdr*)lyr->start;
}

//-------------------------------------------------------------------------
// UDP
//-------------------------------------------------------------------------

static ENC_STATUS UDP_Encode (EncState* enc, Buffer* in, Buffer* out)
{
    if ( IP_VER((IPHdr*)enc->ip_hdr) == 4 )
        return UN4_Encode(enc, in, out);

    return UN6_Encode(enc, in, out);
}

static ENC_STATUS UDP_Update (Packet* p, Layer* lyr, uint32_t* len)
{
    UDPHdr* h = (UDPHdr*)(lyr->start);

    *len += sizeof(*h) + p->dsize;
    h->uh_len = htons((u_int16_t)*len);

    // don't calculate the UDP checksum here;
    // dnet's ip_checksum() will do it
    return ENC_OK;
}

static void UDP_Format (EncodeFlags f, const Packet* p, Packet* c, Layer* lyr)
{
    UDPHdr* ch = (UDPHdr*)lyr->start;
    c->udph = ch;

    if ( REVERSE(f) )
    {
        int i = lyr - c->layers;
        UDPHdr* ph = (UDPHdr*)p->layers[i].start;

        ch->uh_sport = ph->uh_dport;
        ch->uh_dport = ph->uh_sport;
    }
    c->sp = ntohs(ch->uh_sport);
    c->dp = ntohs(ch->uh_dport);
}

//-------------------------------------------------------------------------
// TCP
// encoder creates TCP RST
// should always try to use acceptable ack since we send RSTs in a
// stateless fashion ... from rfc 793:
//
// In all states except SYN-SENT, all reset (RST) segments are validated
// by checking their SEQ-fields.  A reset is valid if its sequence number
// is in the window.  In the SYN-SENT state (a RST received in response
// to an initial SYN), the RST is acceptable if the ACK field
// acknowledges the SYN.
//-------------------------------------------------------------------------

static ENC_STATUS TCP_Encode (EncState* enc, Buffer* in, Buffer* out)
{
    int len, ctl;

    TCPHdr* hi = (TCPHdr*)enc->p->layers[enc->layer-1].start;
    TCPHdr* ho = (TCPHdr*)(out->base + out->end);

    UPDATE_BOUND(out, sizeof(*ho));

    len = GET_TCP_HDR_LEN(hi) - sizeof(*hi);
    UPDATE_BOUND(in, len);
    ctl = (hi->th_flags & TH_SYN) ? 1 : 0;

    if ( FORWARD(enc) )
    {
        ho->th_sport = hi->th_sport;
        ho->th_dport = hi->th_dport;

        // th_seq depends on whether the data passes or drops
        if ( (enc->type == ENC_TCP_FIN) || !ScAdapterInlineMode() )
            ho->th_seq = htonl(ntohl(hi->th_seq) + enc->p->dsize + ctl);
        else
            ho->th_seq = hi->th_seq;

        ho->th_ack = hi->th_ack;
    }
    else
    {
        ho->th_sport = hi->th_dport;
        ho->th_dport = hi->th_sport;

        ho->th_seq = hi->th_ack;
        ho->th_ack = htonl(ntohl(hi->th_seq) + enc->p->dsize + ctl);
    }

    if ( enc->flags & ENC_FLAG_SEQ )
    {
        uint32_t seq = ntohl(ho->th_seq);
        seq += (enc->flags & ENC_FLAG_VAL);
        ho->th_seq = htonl(seq);
    }
    ho->th_offx2 = 0;
    SET_TCP_OFFSET(ho, (TCP_HDR_LEN >> 2));
    ho->th_win = ho->th_urp = 0;

    if ( enc->type == ENC_TCP_FIN )
    {
        if ( enc->payLoad && enc->payLen > 0 )
        {
            uint8_t* pdu = out->base + out->end;
            UPDATE_BOUND(out, enc->payLen);
            memcpy(pdu, enc->payLoad, enc->payLen);
        }
        ho->th_flags = TH_FIN | TH_ACK;
    }
    else
    {
        ho->th_flags = TH_RST | TH_ACK;
    }

    // we don't need to set th_sum here because dnet's
    // ip_checksum() sets both IP and TCP checksums and
    // ip6_checksum() sets the TCP checksum.
    return ENC_OK;
}

static ENC_STATUS TCP_Update (Packet* p, Layer* lyr, uint32_t* len)
{
    TCPHdr* h = (TCPHdr*)(lyr->start);
    *len += GET_TCP_HDR_LEN(h) + p->dsize;

    // don't calculate the TCP checksum here;
    // dnet's ip_checksum() will do it

    return ENC_OK;
}

static void TCP_Format (EncodeFlags f, const Packet* p, Packet* c, Layer* lyr)
{
    TCPHdr* ch = (TCPHdr*)lyr->start;
    c->tcph = ch;

    if ( REVERSE(f) )
    {
        int i = lyr - c->layers;
        TCPHdr* ph = (TCPHdr*)p->layers[i].start;

        ch->th_sport = ph->th_dport;
        ch->th_dport = ph->th_sport;
    }
    c->sp = ntohs(ch->th_sport);
    c->dp = ntohs(ch->th_dport);
}

//-------------------------------------------------------------------------
// IP6 encoder
//-------------------------------------------------------------------------

#ifdef SUP_IP6
static ENC_STATUS IP6_Encode (EncState* enc, Buffer* in, Buffer* out)
{
    int len;
    uint32_t start = out->end;

    IP6RawHdr* hi = (IP6RawHdr*)enc->p->layers[enc->layer-1].start;
    IP6RawHdr* ho = (IP6RawHdr*)(out->base + out->end);
    PROTO_ID next = NextEncoder(enc);

    UPDATE_BOUND(out, sizeof(*ho));

    ho->ip6flow = htonl(ntohl(hi->ip6flow) & 0xFFF00000);
    ho->ip6nxt = hi->ip6nxt;

    if ( FORWARD(enc) )
    {
        uint8_t ttl = AdjTTL(hi->ip6hops);

        memcpy(ho->ip6_src.s6_addr, hi->ip6_src.s6_addr, sizeof(ho->ip6_src.s6_addr));
        memcpy(ho->ip6_dst.s6_addr, hi->ip6_dst.s6_addr, sizeof(ho->ip6_dst.s6_addr));

        if ( enc->p->ssnptr )
            ttl = GetTTL(enc);

#ifdef NORMALIZER
        if ( ttl < ScMinTTL() && ScNewTTL() )
            ttl = ScNewTTL();
#endif
        ho->ip6hops = ttl;
    }
    else
    {
        uint8_t ttl = MAX_TTL;

        memcpy(ho->ip6_src.s6_addr, hi->ip6_dst.s6_addr, sizeof(ho->ip6_src.s6_addr));
        memcpy(ho->ip6_dst.s6_addr, hi->ip6_src.s6_addr, sizeof(ho->ip6_dst.s6_addr));

        if ( enc->p->ssnptr )
            ttl = GetTTL(enc);

#ifdef NORMALIZER
        if ( ttl < ScMinTTL() && ScNewTTL() )
            ttl = ScNewTTL();
#endif
        ho->ip6hops = ttl;
    }

    enc->ip_hdr = (uint8_t*)hi;
    enc->ip_len = sizeof(*hi);

    if ( next < PROTO_MAX )
    {
        int err = encoders[next].fencode(enc, in, out);
        if ( err ) return err;
    }
    if ( enc->proto )
    {
        ho->ip6nxt = enc->proto;
        enc->proto = 0;
    }
    len = out->end - start;
    ho->ip6plen = htons((uint16_t)(len - sizeof(*ho)));
    ip6_checksum(ho, len);

    return ENC_OK;
}

static ENC_STATUS IP6_Update (Packet* p, Layer* lyr, uint32_t* len)
{
    IP6RawHdr* h = (IP6RawHdr*)(lyr->start);
    int i = lyr - p->layers;

    // if we didn't trim payload or format this packet,
    // we may not know the actual lengths because not all
    // extension headers are decoded and we stop at frag6.
    // in such case we do not modify the packet length.
    if ( (p->packet_flags & PKT_MODIFIED)
#ifdef NORMALIZER
        && !(p->packet_flags & PKT_RESIZED)
#endif
    ) {
        *len = ntohs(h->ip6plen) + sizeof(*h);
    }
    else
    {
        if ( i + 1 == p->next_layer )
            *len += lyr->length + p->dsize;

        // w/o all extension headers, can't use just the
        // fixed ip6 header length so we compute header delta
        else
            *len += lyr[1].start - lyr->start;

        // len includes header, remove for payload
        h->ip6plen = htons((uint16_t)(*len - sizeof(*h)));
    }

    if ( !PacketWasCooked(p) || (p->packet_flags & PKT_REBUILT_FRAG) )
        ip6_checksum(h, *len);

    return ENC_OK;
}

static void IP6_Format (EncodeFlags f, const Packet* p, Packet* c, Layer* lyr)
{
    IP6RawHdr* ch = (IP6RawHdr*)lyr->start;

    if ( REVERSE(f) )
    {
        int i = lyr - c->layers;
        IP6RawHdr* ph = (IP6RawHdr*)p->layers[i].start;

        memcpy(ch->ip6_src.s6_addr, ph->ip6_dst.s6_addr, sizeof(ch->ip6_src.s6_addr));
        memcpy(ch->ip6_dst.s6_addr, ph->ip6_src.s6_addr, sizeof(ch->ip6_dst.s6_addr));
    }
    if ( f & ENC_FLAG_DEF )
    {
        int i = lyr - c->layers;
        if ( i + 1 == p->next_layer )
        {
            uint8_t* b = (uint8_t*)p->ip6_extensions[p->ip6_frag_index].data;
            if ( b ) lyr->length = b - p->layers[i].start;
        }
    }
    sfiph_build(c, ch, AF_INET6);

    // set outer to inner so this will always wind pointing to inner
    c->raw_ip6h = ch;
}

//-------------------------------------------------------------------------
// IP6 options functions
//-------------------------------------------------------------------------

static ENC_STATUS Opt6_Encode (EncState* enc, Buffer* in, Buffer* out)
{
    // we don't encode ext headers
    PROTO_ID next = NextEncoder(enc);

    if ( next < PROTO_MAX )
    {
        int err = encoders[next].fencode(enc, in, out);
        if ( err ) return err;
    }
    return ENC_OK;
}

static ENC_STATUS Opt6_Update (Packet* p, Layer* lyr, uint32_t* len)
{
    int i = lyr - p->layers;
    *len += lyr->length;

    if ( i + 1 == p->next_layer )
        *len += p->dsize;

    return ENC_OK;
}
#endif

//-------------------------------------------------------------------------
// ICMP6 functions
//-------------------------------------------------------------------------

static ENC_STATUS UN6_Encode (EncState* enc, Buffer* in, Buffer* out)
{
    uint8_t* p;
    uint8_t* hi = enc->p->layers[enc->layer-1].start;
    IcmpHdr* ho = (IcmpHdr*)(out->base + out->end);

#ifdef DEBUG
    if ( enc->type < ENC_UNR_NET )
        return ENC_BAD_OPT;
#endif

    enc->proto = IPPROTO_ICMPV6;

    UPDATE_BOUND(out, sizeof(*ho));
    ho->type = 1;   // dest unreachable
    ho->code = 4;   // port unreachable
    ho->cksum = 0;
    ho->unused = 0;

    // no need to set csum here because ip6_checksum() will
    // take care of it.  see Encode_TCP() for details.

    // ip + udp headers are copied separately because there
    // may be intervening extension headers which aren't copied

    // copy original ip header
    p = out->base + out->end;
    UPDATE_BOUND(out, enc->ip_len);
    // TBD should be able to elminate enc->ip_hdr by using layer-2
    memcpy(p, enc->ip_hdr, enc->ip_len);
    ((IP6RawHdr*)p)->ip6nxt = IPPROTO_UDP;

    // copy first 8 octets of original ip data (ie udp header)
    // TBD: copy up to minimum MTU worth of data
    p = out->base + out->end;
    UPDATE_BOUND(out, ICMP_UNREACH_DATA);
    memcpy(p, hi, ICMP_UNREACH_DATA);

    return ENC_OK;
}

#ifdef SUP_IP6
static void ICMP6_Format (EncodeFlags f, const Packet* p, Packet* c, Layer* lyr)
{
    // TBD handle nested icmp6 layers
    c->icmp6h = (ICMP6Hdr*)lyr->start;
}
#endif

//-------------------------------------------------------------------------
// XXX (generic) functions
//-------------------------------------------------------------------------

static ENC_STATUS XXX_Encode (EncState* enc, Buffer* in, Buffer* out)
{
    int n = enc->p->layers[enc->layer-1].length;

    uint8_t* hi = enc->p->layers[enc->layer-1].start;
    uint8_t* ho = (uint8_t*)(out->base + out->end);
    PROTO_ID next = NextEncoder(enc);

    UPDATE_BOUND(out, n);
    memcpy(ho, hi, n);

    if ( next < PROTO_MAX )
    {
        int err = encoders[next].fencode(enc, in, out);
        if ( err ) return err;
    }
    return ENC_OK;
}

// for general cases, may need to move dsize out of top, tcp, and
// udp and put in Encode_Update() (then this can be eliminated and
// xxx called instead).  (another thought is to add data as a "layer").

static ENC_STATUS Top_Update (Packet* p, Layer* lyr, uint32_t* len)
{
    *len += lyr->length + p->dsize;
    return ENC_OK;
}

static ENC_STATUS XXX_Update (Packet* p, Layer* lyr, uint32_t* len)
{
    *len += lyr->length;
    return ENC_OK;
}

static void XXX_Format (EncodeFlags f, const Packet* p, Packet* c, Layer* lyr)
{
    // nop
}

//-------------------------------------------------------------------------
// function table:
// these must be in the same order PROTO_IDs are defined!
// all entries must have a function
//-------------------------------------------------------------------------

static EncoderFunctions encoders[PROTO_MAX] = {
    { Eth_Encode,  Eth_Update,   Eth_Format   },
    { IP4_Encode,  IP4_Update,   IP4_Format   },
    { UN4_Encode,  Top_Update,   ICMP4_Format },
    { XXX_Encode,  XXX_Update,   XXX_Format,  },  // ICMP_IP4
    { UDP_Encode,  UDP_Update,   UDP_Format   },
    { TCP_Encode,  TCP_Update,   TCP_Format   },
#ifdef SUP_IP6
    { IP6_Encode,  IP6_Update,   IP6_Format   },
    { Opt6_Encode, Opt6_Update,  XXX_Format   },  // IP6 Hop Opts
    { Opt6_Encode, Opt6_Update,  XXX_Format   },  // IP6 Dst Opts
    { UN6_Encode,  Top_Update,   ICMP6_Format },
    { XXX_Encode,  XXX_Update,   XXX_Format,  },  // ICMP_IP6
#endif
    { XXX_Encode,  XXX_Update,   VLAN_Format  },
#ifdef GRE
    { XXX_Encode,  XXX_Update,   GRE_Format   },
#endif
    { XXX_Encode,  XXX_Update,   XXX_Format   },  // PPP
#ifdef MPLS
    { XXX_Encode,  XXX_Update,   XXX_Format   },  // MPLS
#endif
    { XXX_Encode,  XXX_Update,   XXX_Format,  }   // ARP
};

