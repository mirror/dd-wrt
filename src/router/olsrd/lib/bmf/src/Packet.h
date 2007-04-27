#ifndef _BMF_PACKET_H
#define _BMF_PACKET_H

/*
 * OLSR Basic Multicast Forwarding (BMF) plugin.
 * Copyright (c) 2005, 2006, Thales Communications, Huizen, The Netherlands.
 * Written by Erik Tromp.
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
 * * Neither the name of Thales, BMF nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* -------------------------------------------------------------------------
 * File       : Packet.h
 * Description: BMF and IP packet processing functions
 * Created    : 29 Jun 2006
 *
 * $Id: Packet.h,v 1.2 2007/02/10 17:05:56 bernd67 Exp $ 
 * ------------------------------------------------------------------------- */

/* System includes */
#include <net/if.h> /* IFNAMSIZ, IFHWADDRLEN */
#include <sys/types.h> /* u_int8_t, u_int16_t */

/* Offsets and sizes into IP-ethernet packets */
#define IPV4_ADDR_SIZE 4
#define ETH_TYPE_OFFSET (2*IFHWADDRLEN)
#define ETH_TYPE_LEN 2
#define IP_HDR_OFFSET (ETH_TYPE_OFFSET + ETH_TYPE_LEN)
#define IPV4_OFFSET_SRCIP 12
#define IPV4_OFFSET_DSTIP (IPV4_OFFSET_SRCIP + IPV4_ADDR_SIZE)

#define IPV4_TYPE 0x0800

/* BMF-encapsulated packets are Ethernet-IP-UDP packets, which start
 * with a 16-bytes BMF header (struct TEncapHeader), followed by the
 * encapsulated Ethernet-IP packet itself */

struct TEncapHeader
{
  u_int32_t crc32;
  u_int32_t futureExpansion1;
  u_int32_t futureExpansion2;
  u_int32_t futureExpansion3;
} __attribute__((__packed__));

#define	ENCAP_HDR_LEN (sizeof(struct TEncapHeader))

struct TSaveTtl
{
  u_int8_t ttl;
  u_int16_t check;
} __attribute__((__packed__));

int GetIpPacketLength(unsigned char* buffer);
int GetIpHeaderLength(unsigned char* buffer);
int GetIpTtl(unsigned char* buffer);
void SaveTtlAndChecksum(unsigned char* buffer, struct TSaveTtl* sttl);
void RestoreTtlAndChecksum(unsigned char* buffer, struct TSaveTtl* sttl);
void DecreaseTtlAndUpdateHeaderChecksum(unsigned char* buffer);

#endif /* _BMF_PACKET_H */
