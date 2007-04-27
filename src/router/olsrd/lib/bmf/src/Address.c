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
 * File       : Address.c
 * Description: IP packet characterization functions
 * Created    : 29 Jun 2006
 *
 * $Id: Address.c,v 1.2 2007/02/10 17:05:55 bernd67 Exp $ 
 * ------------------------------------------------------------------------- */
 
#include "Address.h"

/* System includes */
#include <assert.h> /* assert() */
#include <netinet/ip.h> /* struct ip */
#include <netinet/udp.h> /* struct udphdr */

/* OLSRD includes */
#include "defs.h" /* COMP_IP */

/* Plugin includes */
#include "Bmf.h" /* BMF_ENCAP_PORT */
#include "NetworkInterfaces.h" /* TBmfInterface */

/* Whether or not to flood local broadcast packets (e.g. packets with IP
 * destination 192.168.1.255). May be overruled by setting the plugin
 * parameter "DoLocalBroadcast" to "no" */
int EnableLocalBroadcast = 1;

/* -------------------------------------------------------------------------
 * Function   : DoLocalBroadcast
 * Description: Overrule the default setting, enabling or disabling the
 *              flooding of local broadcast packets
 * Input      : enable - either "yes" or "no"
 * Output     : none
 * Return     : fail (0) or success (1)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int DoLocalBroadcast(const char* enable)
{
  if (strcmp(enable, "yes") == 0)
  {
    EnableLocalBroadcast = 1;
    return 1;
  }
  else if (strcmp(enable, "no") == 0)
  {
    EnableLocalBroadcast = 0;
    return 1;
  }

  /* Value not recognized */
  return 0;
}

/* -------------------------------------------------------------------------
 * Function   : IsMulticast
 * Description: Check if an IP address is a multicast address
 * Input      : ipAddress
 * Output     : none
 * Return     : true (1) or false (0)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int IsMulticast(union olsr_ip_addr* ipAddress)
{
  assert(ipAddress != NULL);

  return (ntohl(ipAddress->v4) & 0xF0000000) == 0xE0000000;
}

/* -------------------------------------------------------------------------
 * Function   : IsLocalBroadcast
 * Description: Check if an IP address is a local broadcast address for a
 *              given network interface
 * Input      : destIp, broadAddr
 * Output     : none
 * Return     : true (1) or false (0)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int IsLocalBroadcast(union olsr_ip_addr* destIp, struct sockaddr* broadAddr)
{
  struct sockaddr_in* sin;
  
  assert(destIp != NULL && broadAddr != NULL);

  /* Cast down to correct sockaddr subtype */
  sin = (struct sockaddr_in*)broadAddr;

  return COMP_IP(&(sin->sin_addr.s_addr), destIp);
}

/* -------------------------------------------------------------------------
 * Function   : IsOlsrOrBmfPacket
 * Description: Check if an ethernet packet is an OLSR packet or a BMF packet
 * Input      : intf, ethPkt, len
 * Output     : none
 * Return     : true (1) or false (0)
 * Data Used  : none
 * Assumption : len >= IP_HDR_OFFSET + GetIpHeaderLength(ethPkt)
 * ------------------------------------------------------------------------- */
int IsOlsrOrBmfPacket(struct TBmfInterface* intf, unsigned char* ethPkt, size_t len)
{
  struct ip* ipData;
  unsigned int ipHeaderLen;
  struct udphdr* udpData;
  u_int16_t destPort;

  assert(ethPkt != NULL);

  /* Consider OLSR and BMF packets not to be local broadcast
   * OLSR packets are UDP - port 698
   * OLSR-BMF packets are UDP - port 50698
   * OLSR-Autodetect probe packets are UDP - port 51698
   * Fragments of the above packets are also not local broadcast */

  ipData = (struct ip*) (ethPkt + IP_HDR_OFFSET);
  if (ipData->ip_p != SOL_UDP)
  {
    return 0;
  }

  /* Check if the packet is an IP-fragment */
  if ((ntohs(ipData->ip_off) & IP_OFFMASK) != 0)
  {
#if 0
    int i;
    for (i = 0; i < FRAGMENT_HISTORY_SIZE; i++)
    {
      /* Quick-access pointer */
      struct TFragmentHistory* entry = &intf->fragmentHistory[i];

      /* Match */
      if (entry->ipId == ntohs(ipData->ip_id) &&
          entry->ipProto == ipData->ip_p &&
          entry->ipSrc.s_addr == ntohl(ipData->ip_src.s_addr) &&
          entry->ipDst.s_addr == ntohl(ipData->ip_dst.s_addr))
      {
        /* Found matching history entry, so packet is assumed to be a fragment
         * of an earlier OLSR/OLSR-BMF/OLSR-Autodetect packet */

        /* More fragments? If not, invalidate entry */
        if (((ntohs(ipData->ip_off) & IP_MF) == 0))
        {
          memset(entry, 0, sizeof(struct TFragmentHistory));
        }

        return 1;
      }
    }

    /* Matching history entry not found, so packet is not assumed to be a fragment
     * of an earlier OLSR/OLSR-BMF/OLSR-Autodetect packet */
#endif
    /* OOPS! IP-fragments may come in earlier than their main packet. In that case,
     * their relation with the main packet is not detected by the above code, resulting
     * in stray fragments being forwarded all ove the network. Solution for now is to
     * not forward any IP-fragments at all. */
    /*return 0;*/
    return 1;
  }

  /* The packet is the first (or only) IP-fragment */

  /* Check length first */
  ipHeaderLen = ipData->ip_hl << 2;
  if (len < IP_HDR_OFFSET + ipHeaderLen + sizeof(struct udphdr))
  {
    return 0;
  }

  /* Go into the UDP header and check port number */
  udpData = (struct udphdr*) (ethPkt + IP_HDR_OFFSET + ipHeaderLen);
  destPort = ntohs(udpData->dest);

  if (destPort == OLSRPORT || destPort == BMF_ENCAP_PORT || destPort == 51698)
      /* TODO: #define for 51698 */
  {
#if 0
    /* If more fragments are expected, keep a record in the fragment history */
    if ((ntohs(ipData->ip_off) & IP_MF) != 0)
    {
      /* Quick-access pointer */
      struct TFragmentHistory* entry = &intf->fragmentHistory[intf->nextFragmentHistoryEntry];

      /* Store in fragment history */
      entry->ipId = ntohs(ipData->ip_id);
      entry->ipProto = ipData->ip_p;
      entry->ipSrc.s_addr = ntohl(ipData->ip_src.s_addr);
      entry->ipDst.s_addr = ntohl(ipData->ip_dst.s_addr);

      /* Advance to next entry */
      intf->nextFragmentHistoryEntry++;
      intf->nextFragmentHistoryEntry %= FRAGMENT_HISTORY_SIZE;
    }
#endif
    return 1;
  }

  return 0;
}
