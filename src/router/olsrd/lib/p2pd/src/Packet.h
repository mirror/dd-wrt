/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */


#ifndef _P2PD_PACKET_H
#define _P2PD_PACKET_H

/* System includes */
#include <netinet/ip.h>         /* struct ip */
#include <netinet/ip6.h>         /* struct ip6_hdr */
#include <net/if.h>             /* IFNAMSIZ, IFHWADDRLEN */
#include <sys/types.h>          /* u_int8_t, u_int16_t */

struct TSaveTtl
{
  u_int8_t ttl;
  u_int16_t check;
} __attribute__((__packed__));

/* P2PD-encapsulated packets are Ethernet-IP-UDP packets, which start
 * with a 8-bytes header (struct TEncapHeader), followed by the
 * encapsulated Ethernet-IP packet itself */

struct TEncapHeader {
  /* Use a standard Type-Length-Value (TLV) element */
  u_int8_t type;
  u_int8_t len;
  u_int16_t reserved;                  /* Always 0 */
  u_int32_t crc32;
} __attribute__ ((__packed__));

#define ENCAP_HDR_LEN ((int)sizeof(struct TEncapHeader))

int IsIpFragment(unsigned char* ipPacket);
u_int16_t GetIpTotalLength(unsigned char* ipPacket);
int IsIpv4Fragment(struct ip*);
int IsMulticastv4(struct ip*);
int IsIpv6Fragment(struct ip6_hdr*);
int IsMulticastv6(struct ip6_hdr*);
u_int8_t GetTtl(unsigned char* ipPacket);
void SaveTtlAndChecksum(unsigned char* ipPacket, struct TSaveTtl* sttl);
void RestoreTtlAndChecksum(unsigned char* ipPacket, struct TSaveTtl* sttl);
void DecreaseTtlAndUpdateHeaderChecksum(unsigned char* ipPacket);
unsigned int GetIpHeaderLength(unsigned char *ipPacket);
unsigned char *GetIpPacket(unsigned char *encapsulationUdpData);

#endif /* _P2PD_PACKET_H */
