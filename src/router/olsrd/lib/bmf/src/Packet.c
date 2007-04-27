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
 * File       : Packet.c
 * Description: BMF and IP packet processing functions
 * Created    : 29 Jun 2006
 *
 * $Id: Packet.c,v 1.3 2007/02/10 17:05:56 bernd67 Exp $ 
 * ------------------------------------------------------------------------- */

#include "Packet.h"

/* System includes */
#include <assert.h> /* assert() */
#include <sys/types.h> /* u_int32_t */
#include <netinet/in.h> /* ntohs(), htons() */
#include <netinet/ip.h> /* struct iphdr */

#include <stdio.h>
/* -------------------------------------------------------------------------
 * Function   : GetIpPacketLength
 * Description: Retrieve the IP packet length (in bytes) of the passed
 *              ethernet-IP packet
 * Input      : buffer - the ethernet-IP packet
 * Output     : none
 * Return     : IP packet length
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int GetIpPacketLength(unsigned char* buffer)
{
  struct iphdr* iph;

  assert(buffer != NULL);

  iph = (struct iphdr*) (buffer + IP_HDR_OFFSET);
  return ntohs(iph->tot_len);
}

/* -------------------------------------------------------------------------
 * Function   : GetIpHeaderLength
 * Description: Retrieve the IP header length (in bytes) of the passed
 *              ethernet-IP packet
 * Input      : buffer - the ethernet-IP packet
 * Output     : none
 * Return     : IP header length
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int GetIpHeaderLength(unsigned char* buffer)
{
  struct iphdr* iph;

  assert(buffer != NULL);

  iph = (struct iphdr*) (buffer + IP_HDR_OFFSET);
  return iph->ihl << 2;
}

/* -------------------------------------------------------------------------
 * Function   : GetIpTtl
 * Description: Retrieve the TTL (Time To Live) value from the IP header of
 *              the passed ethernet-IP packet
 * Input      : buffer - the ethernet-IP packet
 * Output     : none
 * Return     : TTL value
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int GetIpTtl(unsigned char* buffer)
{
  struct iphdr* iph;

  assert(buffer != NULL);

  iph = (struct iphdr*) (buffer + IP_HDR_OFFSET);
  return iph->ttl;
}

/* -------------------------------------------------------------------------
 * Function   : SaveTtlAndChecksum
 * Description: Save the TTL (Time To Live) value and IP checksum as found in
 *              the IP header of the passed ethernet-IP packet
 * Input      : buffer - the ethernet-IP packet
 * Output     : sttl - the TTL and checksum values
 * Return     : none
 * Data Used  : none
 * ------------------------------------------------------------------------- */
void SaveTtlAndChecksum(unsigned char* buffer, struct TSaveTtl* sttl)
{
  struct iphdr* iph;

  assert(buffer != NULL && sttl != NULL);

  iph = (struct iphdr*) (buffer + IP_HDR_OFFSET);
  sttl->ttl = iph->ttl;
  sttl->check = ntohs(iph->check);
}

/* -------------------------------------------------------------------------
 * Function   : RestoreTtlAndChecksum
 * Description: Restore the TTL (Time To Live) value and IP checksum in
 *              the IP header of the passed ethernet-IP packet
 * Input      : buffer - the ethernet-IP packet
 *              sttl - the TTL and checksum values
 * Output     : none
 * Return     : none
 * Data Used  : none
 * ------------------------------------------------------------------------- */
void RestoreTtlAndChecksum(unsigned char* buffer, struct TSaveTtl* sttl)
{
  struct iphdr* iph;

  assert(buffer != NULL && sttl != NULL);

  iph = (struct iphdr*) (buffer + IP_HDR_OFFSET);
  iph->ttl = sttl->ttl;
  iph->check = htons(sttl->check);
}

/* -------------------------------------------------------------------------
 * Function   : DecreaseTtlAndUpdateHeaderChecksum
 * Description: For an IP packet, decrement the TTL value and update the IP header
 *              checksum accordingly.
 * Input      : buffer - the ethernet-IP packet
 * Output     : none
 * Return     : none
 * Data Used  : none
 * Notes      : See also RFC1141
 * ------------------------------------------------------------------------- */
void DecreaseTtlAndUpdateHeaderChecksum(unsigned char* buffer)
{
  struct iphdr* iph;
  u_int32_t sum;

  assert(buffer != NULL);

  iph = (struct iphdr*) (buffer + IP_HDR_OFFSET);

  iph->ttl--; /* decrement ttl */
  sum = ntohs(iph->check) + 0x100; /* increment checksum high byte */
  iph->check = htons(sum + (sum>>16)); /* add carry */
}
