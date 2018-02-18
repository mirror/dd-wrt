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

/* -------------------------------------------------------------------------
 * File       : Packet.c
 * Description: IP packet and Ethernet frame processing functions
 * Created    : 29 Jun 2006
 *
 * ------------------------------------------------------------------------- */

#include "olsr.h"
#include "Packet.h"

/* System includes */
#include <stddef.h> /* NULL */
#include <assert.h> /* assert() */
#include <string.h> /* memcpy() */
#include <sys/types.h> /* u_int8_t, u_int16_t, u_int32_t */
#include <netinet/in.h> /* ntohs(), htons() */
#include <netinet/ip.h> /* struct iphdr */

/* -------------------------------------------------------------------------
 * Function   : IsIpFragment
 * Description: Check if an IP packet is an IP fragment
 * Input      : ipPacket - the IP packet
 * Output     : none
 * Return     : true (1) or false (0)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int IsIpFragment(unsigned char* ipPacket)
{
  struct ip* iph;

  assert(ipPacket != NULL);

  iph = (struct ip*) ARM_NOWARN_ALIGN(ipPacket);
  if ((ntohs(iph->ip_off) & IP_OFFMASK) != 0)
  {
    return 1;
  }
  return 0;
} /* IsIpFragment */

/* -------------------------------------------------------------------------
 * Function   : GetIpTotalLength
 * Description: Retrieve the total length of the IP packet (in bytes) of
 *              an IP packet
 * Input      : ipPacket - the IP packet
 * Output     : none
 * Return     : IP packet length
 * Data Used  : none
 * ------------------------------------------------------------------------- */
u_int16_t GetIpTotalLength(unsigned char* ipPacket)
{
  struct iphdr* iph;

  assert(ipPacket != NULL);

  iph = (struct iphdr*) ARM_NOWARN_ALIGN(ipPacket);
  return ntohs(iph->tot_len);
} /* GetIpTotalLength */

/* -------------------------------------------------------------------------
 * Function   : GetIpHeaderLength
 * Description: Retrieve the IP header length (in bytes) of an IP packet
 * Input      : ipPacket - the IP packet
 * Output     : none
 * Return     : IP header length
 * Data Used  : none
 * ------------------------------------------------------------------------- */
unsigned int GetIpHeaderLength(unsigned char* ipPacket)
{
  struct iphdr* iph;

  assert(ipPacket != NULL);

  iph = (struct iphdr*) ARM_NOWARN_ALIGN(ipPacket);
  return iph->ihl << 2;
} /* GetIpHeaderLength */

/* -------------------------------------------------------------------------
 * Function   : GetTtl
 * Description: Retrieve the TTL (Time To Live) value from the IP header of
 *              an IP packet
 * Input      : ipPacket - the IP packet
 * Output     : none
 * Return     : TTL value
 * Data Used  : none
 * ------------------------------------------------------------------------- */
u_int8_t GetTtl(unsigned char* ipPacket)
{
  struct iphdr* iph;

  assert(ipPacket != NULL);

  iph = (struct iphdr*) ARM_NOWARN_ALIGN(ipPacket);
  return iph->ttl;
} /* GetTtl */

/* -------------------------------------------------------------------------
 * Function   : SaveTtlAndChecksum
 * Description: Save the TTL (Time To Live) value and IP checksum as found in
 *              the IP header of an IP packet
 * Input      : ipPacket - the IP packet
 * Output     : sttl - the TTL and checksum values
 * Return     : none
 * Data Used  : none
 * ------------------------------------------------------------------------- */
void SaveTtlAndChecksum(unsigned char* ipPacket, struct TSaveTtl* sttl)
{
  struct iphdr* iph;

  assert(ipPacket != NULL && sttl != NULL);

  iph = (struct iphdr*) ARM_NOWARN_ALIGN(ipPacket);
  sttl->ttl = iph->ttl;
  sttl->check = ntohs(iph->check);
} /* SaveTtlAndChecksum */

/* -------------------------------------------------------------------------
 * Function   : RestoreTtlAndChecksum
 * Description: Restore the TTL (Time To Live) value and IP checksum in
 *              the IP header of an IP packet
 * Input      : ipPacket - the IP packet
 *              sttl - the TTL and checksum values
 * Output     : none
 * Return     : none
 * Data Used  : none
 * ------------------------------------------------------------------------- */
void RestoreTtlAndChecksum(unsigned char* ipPacket, struct TSaveTtl* sttl)
{
  struct iphdr* iph;

  assert(ipPacket != NULL && sttl != NULL);

  iph = (struct iphdr*) ARM_NOWARN_ALIGN(ipPacket);
  iph->ttl = sttl->ttl;
  iph->check = htons(sttl->check);
} /* RestoreTtlAndChecksum */

/* -------------------------------------------------------------------------
 * Function   : DecreaseTtlAndUpdateHeaderChecksum
 * Description: For an IP packet, decrement the TTL value and update the IP header
 *              checksum accordingly.
 * Input      : ipPacket - the IP packet
 * Output     : none
 * Return     : none
 * Data Used  : none
 * Notes      : See also RFC1141
 * ------------------------------------------------------------------------- */
void DecreaseTtlAndUpdateHeaderChecksum(unsigned char* ipPacket)
{
  struct iphdr* iph;
  u_int32_t sum;

  assert(ipPacket != NULL);

  iph = (struct iphdr*) ARM_NOWARN_ALIGN(ipPacket);

  iph->ttl--; /* decrement ttl */
  sum = ntohs(iph->check) + 0x100; /* increment checksum high byte */
  iph->check = htons(sum + (sum>>16)); /* add carry */
} /* DecreaseTtlAndUpdateHeaderChecksum */

/* -------------------------------------------------------------------------
 * Function   : GetIpHeader
 * Description: Retrieve the IP header from BMF encapsulation UDP data
 * Input      : encapsulationUdpData - the encapsulation UDP data
 * Output     : none
 * Return     : IP header
 * Data Used  : none
 * ------------------------------------------------------------------------- */
struct ip* GetIpHeader(unsigned char* encapsulationUdpData)
{
  return (struct ip*) ARM_NOWARN_ALIGN((encapsulationUdpData + ENCAP_HDR_LEN));
} /* GetIpHeader */

/* -------------------------------------------------------------------------
 * Function   : GetIpPacket
 * Description: Retrieve the IP packet from BMF encapsulation UDP data
 * Input      : encapsulationUdpData - the encapsulation UDP data
 * Output     : none
 * Return     : The IP packet
 * Data Used  : none
 * ------------------------------------------------------------------------- */
unsigned char* GetIpPacket(unsigned char* encapsulationUdpData)
{
  return encapsulationUdpData + ENCAP_HDR_LEN;
} /* GetIpPacket */

/* -------------------------------------------------------------------------
 * Function   : GetEncapsulationUdpDataLength
 * Description: Return the length of BMF encapsulation UDP data
 * Input      : encapsulationUdpData - the encapsulation UDP data
 * Output     : none
 * Return     : The encapsulation data length
 * Data Used  : none
 * ------------------------------------------------------------------------- */
u_int16_t GetEncapsulationUdpDataLength(unsigned char* encapsulationUdpData)
{
  return GetIpTotalLength(GetIpPacket(encapsulationUdpData)) + ENCAP_HDR_LEN;
} /* GetEncapsulationUdpDataLength */

