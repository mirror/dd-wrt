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

#ifndef __PROTOCOLS_H__
#define __PROTOCOLS_H__

typedef enum {
    PROTO_ETH,        // DecodeEthPkt

    PROTO_IP4,        // DecodeIP
                      // DecodeIPOptions - handled with IP4
    PROTO_ICMP4,      // DecodeICMP
    PROTO_ICMP_IP4,   // DecodeICMPEmbeddedIP - cloned ARP/non-IP comment should be
                      // removed here since ICMP won't contain non-IP.

    PROTO_UDP,        // DecodeUDP
    PROTO_TCP,        // DecodeTCP
                      // DecodeTCPOptions - handled with TCP

#ifdef SUP_IP6
    PROTO_IP6,        // DecodeIPV6
                      // DecodeIPV6Extensions - nothing to do here, calls below
    PROTO_IP6_HOP_OPTS,  // DecodeIPV6Options - ip6 hop, dst, rte, and frag exts
    PROTO_IP6_DST_OPTS,
    PROTO_ICMP6,      // DecodeICMP6
    PROTO_ICMP_IP6,   // DecodeICMPEmbeddedIP6 - same ARP comment thing.
#endif
    PROTO_VLAN,       // DecodeVlan
#ifdef GRE
    PROTO_GRE,        // DecodeGRE
                      // DecodeTransBridging - basically same as DecodeEthPkt
#endif
    PROTO_PPP,        // DecodePppPkt - weird - optionally skips addr and cntl
                      // bytes; what about flag and protocol?
                      // calls only DecodePppPktEncapsulated.
    PROTO_MPLS,       // DecodeMPLS - decoder changes pkth len/caplen!
                      // DecodeEthOverMPLS - basically same as straight eth
    PROTO_ARP,        // DecodeARP - should remove setting PROTO_BIT__ARP
                      // since it is never checked anywhere.

#if 0
    PROTO_PPP_ENCAP,  // DecodePppPktEncapsulated
    PROTO_ETH_PPP,    // DecodePPPoEPkt - this looks broke; PPPoEHdr still contains
                      // an EtherHdr but this decoder was "fixed" to skip it


#ifndef NO_NON_ETHER_DECODER
    PROTO_TR,         // DecodeTRPkt
    PROTO_FDDI,       // DecodeFDDIPkt
    PROTO_LSLL,       // DecodeLinuxSLLPkt sockaddr_ll for "any" device and 
                      // certain misbehaving link layer encapsulations
    PROTO_80211,      // DecodeIEEE80211Pkt
    PROTO_SLIP,       // DecodeSlipPkt - actually, based on header size, this
                      // must be CSLIP (TCP/IP header compression) but all it
                      // does is skip over the presumed header w/o expanding
                      // and then jumps into IP4 decoding only; also, the actual
                      // esc/end sequences must already have been removed because
                      // there is no attempt to do that.
    PROTO_L2I4,       // DecodeI4LRawIPPkt - always skips 2 bytes and then does
                      // IP4 decoding only
    PROTO_L2I4C,      // DecodeI4LCiscoIPPkt -always skips 4 bytes and then does
                      // IP4 decoding only
    PROTO_CHDLC,      // DecodeChdlcPkt - skips 4 bytes and decodes IP4 only.
    PROTO_PFLOG,      // DecodePflog
    PROTO_OLD_PFLOG,  // DecodeOldPflog
    PROTO_PPP_SERIAL, // DecodePppSerialPkt - also weird - requires addr, cntl,
                      // and proto (no flag) but optionally skips only 2 bytes
                      // (presumably the trailer w/chksum is already stripped)
                      // Calls either DecodePppPktEncapsulated or DecodeChdlcPkt.
    PROTO_ENC,        // DecodeEncPkt - skips 12 bytes and decodes IP4 only.
                      // (add family + "spi" + "flags" - don't know what this is)
    PROTO_EAP,        // DecodeEAP
    PROTO_EAPOL,      // DecodeEapol - leaf decoder
    PROTO_EAPOL_KEY,  // DecodeEapolKey - leaf decoder
#endif // NO_NON_ETHER_DECODER
#endif // 0

    PROTO_MAX
} PROTO_ID;

                      // DecodeIPX - just counts; no decoding
                      // DecodeEthLoopback - same as ipx
                      // DecodeRawPkt - jumps straight into IP4 decoding
                      // there is nothing to do
                      // DecodeNullPkt - same as DecodeRawPkt

typedef struct {
    PROTO_ID proto;
    uint16_t length;
    uint8_t* start;
} Layer;

#endif // __PROTOCOLS_H__

