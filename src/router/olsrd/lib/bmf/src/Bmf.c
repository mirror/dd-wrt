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
 * File       : Bmf.c
 * Description: Multicast forwarding functions
 * Created    : 29 Jun 2006
 *
 * $Id: Bmf.c,v 1.2 2007/02/10 17:05:55 bernd67 Exp $ 
 * ------------------------------------------------------------------------- */

#define _MULTI_THREADED

#include "Bmf.h"

/* System includes */
#include <stdio.h> /* NULL */
#include <sys/types.h> /* ssize_t */
#include <string.h> /* strerror() */
#include <errno.h> /* errno */
#include <assert.h> /* assert() */
#include <linux/if_packet.h> /* struct sockaddr_ll, PACKET_MULTICAST */
#include <pthread.h> /* pthread_t, pthread_create() */
#include <signal.h> /* sigset_t, sigfillset(), sigdelset(), SIGINT */
#include <netinet/ip.h> /* struct ip */

/* OLSRD includes */
#include "defs.h" /* olsr_cnf, OLSR_PRINTF */
#include "olsr.h" /* olsr_printf */
#include "scheduler.h" /* olsr_register_scheduler_event */
#include "mid_set.h" /* mid_lookup_main_addr() */
#include "mpr_selector_set.h" /* olsr_lookup_mprs_set() */
#include "link_set.h" /* get_best_link_to_neighbor() */

/* Plugin includes */
#include "NetworkInterfaces.h" /* TBmfInterface, CreateBmfNetworkInterfaces(), CloseBmfNetworkInterfaces() */
#include "Address.h" /* IsMulticast(), IsLocalBroadcast() */
#include "Packet.h" /* ETH_TYPE_OFFSET, IFHWADDRLEN etc. */
#include "PacketHistory.h" /* InitPacketHistory() */
#include "DropList.h" /* DropMac() */

static pthread_t BmfThread;
static int BmfThreadRunning = 0;

/* -------------------------------------------------------------------------
 * Function   : EncapsulateAndForwardPacket
 * Description: Encapsulate a captured raw IP packet and forward it
 * Input      : intf - the network interface on which to forward the packet
 *              buffer - space for the encapsulation header, followed by
 *                the captured packet
 *              len - the number of octets in the encapsulation header plus
 *                captured packet
 * Output     : none
 * Return     : none
 * Data Used  : none
 * ------------------------------------------------------------------------- */
static void EncapsulateAndForwardPacket(struct TBmfInterface* intf, unsigned char* buffer, ssize_t len)
{
  unsigned char* ethPkt = buffer + ENCAP_HDR_LEN;
  int nBytesWritten;
  struct sockaddr_in encapDest;

  /* Change encapsulated source MAC address to that of sending interface */
  memcpy(ethPkt + IFHWADDRLEN, intf->macAddr, IFHWADDRLEN);

  /* Destination address is local broadcast */
  memset(&encapDest, 0, sizeof(encapDest));
  encapDest.sin_family = AF_INET;
  encapDest.sin_port = htons(BMF_ENCAP_PORT);
  encapDest.sin_addr.s_addr = ((struct sockaddr_in*)&intf->olsrIntf->int_broadaddr)->sin_addr.s_addr;

  nBytesWritten = sendto(
    intf->encapsulatingSkfd,
    buffer,
    len,
    MSG_DONTROUTE,
    (struct sockaddr*) &encapDest,
    sizeof(encapDest));                   
  if (nBytesWritten != len)
  {
    olsr_printf(
      1,
      "%s: sendto() error forwarding pkt to \"%s\": %s\n",
      PLUGIN_NAME,
      intf->ifName,
      strerror(errno));
  }
  else
  {
    OLSR_PRINTF(
      9,
      "%s: --> encapsulated and forwarded to \"%s\"\n",
      PLUGIN_NAME_SHORT,
      intf->ifName);
  } /* if (nBytesWritten != len) */
}

/* -------------------------------------------------------------------------
 * Function   : BmfPacketCaptured
 * Description: Handle a captured raw IP packet
 * Input      : intf - the network interface on which the packet was captured
 *              sllPkttype - the type of packet. Either PACKET_OUTGOING,
 *                PACKET_BROADCAST or PACKET_MULTICAST.
 *              buffer - space for the encapsulation header, followed by
 *                the captured packet
 *              len - the number of octets in the encapsulation header plus
 *                captured packet
 * Output     : none
 * Return     : none
 * Data Used  : BmfInterfaces
 * Notes      : The packet is assumed to be captured on a socket of family
 *              PF_PACKET and type SOCK_RAW.
 * ------------------------------------------------------------------------- */
static void BmfPacketCaptured(
  struct TBmfInterface* intf,
  unsigned char sllPkttype,
  unsigned char* buffer,
  ssize_t len)
{
  unsigned char* srcMac;
  union olsr_ip_addr srcIp;
  union olsr_ip_addr destIp;
  union olsr_ip_addr* origIp;
  struct TBmfInterface* nextFwIntf;
  int isFromOlsrIntf;
  int isFromOlsrNeighbor;
  int iAmMpr;
  unsigned char* ethPkt = buffer + ENCAP_HDR_LEN;
  ssize_t ethPktLen = len - ENCAP_HDR_LEN;
  struct ip* ipData;
  u_int32_t crc32;
  struct TEncapHeader* encapHdr;

  /* Only forward IPv4 packets */
  u_int16_t type;
  memcpy(&type, ethPkt + ETH_TYPE_OFFSET, 2);
  if (ntohs(type) != IPV4_TYPE)
  {
    return;
  }

  ipData = (struct ip*)(ethPkt + IP_HDR_OFFSET);

  /* Only forward multicast packets. Also forward local broadcast packets,
   * if configured */
  COPY_IP(&destIp, &ipData->ip_dst);
  if (IsMulticast(&destIp) ||
      (EnableLocalBroadcast != 0 && IsLocalBroadcast(&destIp, &intf->broadAddr)))
  {
    /* continue */
  }
  else
  {
    return;
  }

  /* Discard OLSR packets (UDP port 698) and BMF encapsulated packets */
  if (IsOlsrOrBmfPacket(intf, ethPkt, ethPktLen))
  {
    return;
  }

  /* Check if this packet is captured on an OLSR-enabled interface */
  isFromOlsrIntf = (intf->olsrIntf != NULL);

  COPY_IP(&srcIp, &ipData->ip_src);
  OLSR_PRINTF(
    9,
    "%s: %s pkt of %d bytes captured on %s interface \"%s\": %s->%s\n",
    PLUGIN_NAME_SHORT,
    sllPkttype == PACKET_OUTGOING ? "outgoing" : "incoming",
    ethPktLen,
    isFromOlsrIntf ? "OLSR" : "non-OLSR",
    intf->ifName,
    olsr_ip_to_string(&srcIp),
    olsr_ip_to_string(&destIp));

  /* Apply drop list for testing purposes. */
  srcMac = ethPkt + IFHWADDRLEN;
  if (IsInDropList(srcMac))
  {
    OLSR_PRINTF(
      9,
      "%s: --> discarding: source MAC (%.2x:%.2x:%.2x:%.2x:%.2x:%.2x) found in drop list\n",
      PLUGIN_NAME_SHORT,
      *srcMac, *(srcMac + 1), *(srcMac + 2), *(srcMac + 3), *(srcMac + 4), *(srcMac + 5));
    return;
  }

  /* Lookup main address of source in the MID table of OLSR */
  origIp = mid_lookup_main_addr(&srcIp);
  if (origIp == NULL)
  {
    origIp = &srcIp;
  }

#ifdef DO_TTL_STUFF
  /* If this packet is captured on a non-OLSR interface, decrease
   * the TTL and re-calculate the IP header checksum. */
  if (! isFromOlsrIntf)
  {
    DecreaseTtlAndUpdateHeaderChecksum(ethPkt);
  } */

  /* If the resulting TTL is <= 0, this packet life has ended, so do not forward it */
  if (GetIpTtl(ethPkt) <= 0)
  {
    OLSR_PRINTF(
      9,
      "%s: --> discarding: TTL=0\n",
      PLUGIN_NAME_SHORT);
    return;
  } */
#endif

  /* Check if this packet was seen recently */
  crc32 = PacketCrc32(ethPkt, ethPktLen);
  if (CheckAndMarkRecentPacket(Hash16(crc32)))
  {
    OLSR_PRINTF(
      9,
      "%s: --> discarding: packet is duplicate\n",
      PLUGIN_NAME_SHORT);
    return;
  }

  /* Compose encapsulation header */
  encapHdr = (struct TEncapHeader*) buffer;
  memset (encapHdr, 0, ENCAP_HDR_LEN);
  encapHdr->crc32 = htonl(crc32);

  /* Check if this packet is captured on an OLSR interface from an OLSR neighbor */
  isFromOlsrNeighbor =
    (isFromOlsrIntf /* The packet is captured on an OLSR interface... */
    && get_best_link_to_neighbor(origIp) != NULL); /* ...from an OLSR neighbor */ 

  /* Check with OLSR if I am MPR for that neighbor */
  iAmMpr = olsr_lookup_mprs_set(origIp) != NULL;

  /* Check with each interface what needs to be done on it */
  nextFwIntf = BmfInterfaces;
  while (nextFwIntf != NULL)
  {
    int isToOlsrIntf;

    struct TBmfInterface* fwIntf = nextFwIntf;
    nextFwIntf = fwIntf->next;

    /* Is the forwarding interface OLSR-enabled? */
    isToOlsrIntf = (fwIntf->olsrIntf != NULL);

    /* Depending on certain conditions, we decide whether or not to forward
     * the packet, and if it is forwarded, in which form (encapsulated
     * or not, TTL decreased or not). These conditions are:
     * - is the packet is coming in on an OLSR interface or not? (isFromOlsrIntf)
     * - is the packet going out on an OLSR interface or not? (isToOlsrIntf)
     * - if the packet if coming in on an OLSR interface:
     *   - is the node that forwarded the packet my OLSR-neighbor? (isFromOlsrNeighbor)
     *   - has the node that forwarded the packet selected me as MPR? (iAmMpr)
     *
     * Based on these conditions, the following cases can be distinguished:
     *
     * - Case 1: Packet coming in on an OLSR interface. What to
     *   do with it on an OLSR interface?
     *   Answer:
     *   - Case 1.1: If the forwarding node is an OLSR neighbor that has *not*
     *     selected me as MPR: don't forward the packet.
     *   - Case 1.2: If the forwarding node is an OLSR neighbor that has selected
     *     me as MPR: encapsulate and forward the packet.
     *   - Case 1.3: If the forwarding node is not an OLSR neighbor: encapsulate
     *     and forward the packet.
     *     NOTE: Case 1.3 is a special case. In the perfect world, we expect to
     *     see only OLSR-neighbors on OLSR-enabled interfaces. Sometimes, however,
     *     ignorant users will connect a host not running OLSR, to a LAN in
     *     which there are hosts running OLSR. Of course these ignorant users,
     *     expecting magic, want to see their multicast packets being forwarded
     *     through the network.
     *
     * - Case 2: Packet coming in on an OLSR interface. What to do with it on a
     *   non-OLSR interface?
     *   Answer: [Decrease the packet's TTL and] forward it.
     *
     * - Case 3: Packet coming in on a non-OLSR interface. What to
     *   do with it on an OLSR interface?
     *   Answer: [Decrease the packet's TTL, then] encapsulate and forward it.
     *
     * - Case 4: Packet coming in on non-OLSR interface. What to do with it on a
     *   non-OLSR interface?
     *   Answer 1: nothing. Multicast routing between non-OLSR interfaces
     *   is to be done by other protocols (e.g. PIM, DVMRP).
     *   Answer 2 (better): [Decrease the packet's TTL, then] forward it.
     */

    if (isFromOlsrIntf && isToOlsrIntf)
    {
      /* Case 1: Forward from an OLSR interface to an OLSR interface */

      if (isFromOlsrNeighbor && !iAmMpr)
      {
        /* Case 1.1 */
        {
          OLSR_PRINTF(
            9,
            "%s: --> not encap-forwarding to \"%s\": I am not selected as MPR by neighbor %s\n",
            PLUGIN_NAME_SHORT,
            fwIntf->ifName,
            olsr_ip_to_string(&srcIp));
        }    
      }
      else if (sllPkttype == PACKET_OUTGOING && intf == fwIntf)
      {
        OLSR_PRINTF(
          9,
          "%s: --> not encap-forwarding to \"%s\": pkt was captured on that interface\n",
          PLUGIN_NAME_SHORT,
          fwIntf->ifName);
      }
      else
      {
        /* Case 1.2 and 1.3 */
        EncapsulateAndForwardPacket(fwIntf, buffer, len);
      }
    } /* if (isFromOlsrIntf && isToOlsrIntf) */

    else if (isFromOlsrIntf && !isToOlsrIntf)
    {
      /* Case 2: Forward from OLSR interface to non-OLSR interface.
       * [Decrease TTL and] forward */

#ifdef DO_TTL_STUFF
      /* If the TTL is to become 0, do not forward this packet */
      if (GetIpTtl(ethPkt) <= 1)
      {
        OLSR_PRINTF(
          9,
          "%s: --> not forwarding to \"%s\": TTL=0\n",
          PLUGIN_NAME_SHORT,
          fwIntf->ifName);
      }
      else
      {
        struct TSaveTtl sttl;
#endif
        int nBytesWritten;

        /* Change source MAC address to that of sending interface */
        memcpy(ethPkt + IFHWADDRLEN, fwIntf->macAddr, IFHWADDRLEN);

        /* If the destination address is not a multicast address, it is assumed to be
         * a local broadcast packet. Update the destination address to match the subnet
         * of the network interface on which the packet is being sent. */
        CheckAndUpdateLocalBroadcast(ethPkt, &fwIntf->broadAddr);

#ifdef DO_TTL_STUFF
        /* Save IP header checksum and the TTL-value of the packet */ 
        SaveTtlAndChecksum(ethPkt, &sttl);

        /* Decrease the TTL by 1 before writing */
        DecreaseTtlAndUpdateHeaderChecksum(ethPkt);
#endif

        nBytesWritten = write(fwIntf->capturingSkfd, ethPkt, ethPktLen);
        if (nBytesWritten != ethPktLen)
        {
          olsr_printf(
            1,
            "%s: write() error forwarding pkt for %s to \"%s\": %s\n",
            PLUGIN_NAME,
            olsr_ip_to_string(&destIp),
            fwIntf->ifName,
            strerror(errno));
        }
        else
        {
          OLSR_PRINTF(
            9,
            "%s: --> forwarded to \"%s\"\n",
            PLUGIN_NAME_SHORT,
            fwIntf->ifName);
        }

#ifdef DO_TTL_STUFF
        /* Restore the IP header checksum and the TTL-value of the packet */
        RestoreTtlAndChecksum(ethPkt, &sttl);

      } /* if (GetIpTtl(ethPkt) <= 1) */
#endif
    } /* else if (isFromOlsrIntf && !isToOlsrIntf) */

    else if (!isFromOlsrIntf && isToOlsrIntf)
    {
      /* Case 3: Forward from a non-OLSR interface to an OLSR interface.
       * Encapsulate and forward packet.
       * Note that packets from non-OLSR interfaces already had their TTL decreased. */

      EncapsulateAndForwardPacket(fwIntf, buffer, len);
    } /* else if (!isFromOlsrIntf && isToOlsrIntf) */

    else
    {
      /* Case 4: Forward from non-OLSR interface to non-OLSR interface.
       * Note that packets from non-OLSR interfaces already had their TTL decreased. */

      /* Don't forward on interface on which packet was received */
      if (intf == fwIntf)
      {
        OLSR_PRINTF(
          9,
          "%s: --> not forwarding to \"%s\": pkt was captured on that interface\n",
          PLUGIN_NAME_SHORT,
          fwIntf->ifName);
      }

#ifdef DO_TTL_STUFF
      /* If the TTL is <= 0, do not forward this packet */
      else if (GetIpTtl(ethPkt) <= 0)
      {
        OLSR_PRINTF(
          9,
          "%s: --> not forwarding to \"%s\": TTL=0\n",
          PLUGIN_NAME_SHORT,
          fwIntf->ifName);
      }
#endif
      else
      {
        int nBytesWritten;

        /* Change source MAC address to that of sending interface */
        memcpy(ethPkt + IFHWADDRLEN, fwIntf->macAddr, IFHWADDRLEN);

        /* If the destination address is not a multicast address, it is assumed to be
         * a local broadcast packet. Update the destination address to match the subnet
         * of the network interface on which the packet is being sent. */
        CheckAndUpdateLocalBroadcast(ethPkt, &fwIntf->broadAddr);

        nBytesWritten = write(fwIntf->capturingSkfd, ethPkt, ethPktLen);
        if (nBytesWritten != ethPktLen)
        {
          olsr_printf(
            1,
            "%s: write() error forwarding pkt for %s to \"%s\": %s\n",
            PLUGIN_NAME,
            olsr_ip_to_string(&destIp),
            fwIntf->ifName,
            strerror(errno));
        }
        else
        {
          OLSR_PRINTF(
            9,
            "%s: --> forwarded from non-OLSR to non-OLSR \"%s\"\n",
            PLUGIN_NAME_SHORT,
            fwIntf->ifName);
        }
      } /* if (intf == fwIntf) */
    }
  } /* while (nextFwIntf != NULL) */
}

/* -------------------------------------------------------------------------
 * Function   : BmfEncapsulatedPacketReceived
 * Description: Handle a received BMF-encapsulated IP packet
 * Input      : intf - the network interface on which the packet was received
 *              fromIp - the IP node that forwarded the packet to us
 *              buffer - the received encapsulated packet
 *              len - the number of octets in the received encapsulated packet
 * Output     : none
 * Return     : none
 * Data Used  : BmfInterfaces
 * Notes      : The packet is assumed to be received on a socket of family
 *              PF_INET and type SOCK_DGRAM (UDP).
 * ------------------------------------------------------------------------- */
static void BmfEncapsulatedPacketReceived(
  struct TBmfInterface* intf, 
  union olsr_ip_addr* fromIp,
  unsigned char* buffer,
  ssize_t len)
{
  union olsr_ip_addr* forwarder;
  int nBytesToWrite;
  unsigned char* bufferToWrite;
  int nBytesWritten;
  int iAmMpr;
  struct sockaddr_in encapDest;
  struct TBmfInterface* nextFwIntf;
  struct ip* ipData;
  unsigned char* ethPkt;
  ssize_t ethPktLen;
  struct TEncapHeader* encapHdr;

  /* Are we talking to ourselves? */
  if (if_ifwithaddr(fromIp) != NULL)
  {
    return;
  }

  /* Encapsulated packet received on non-OLSR interface? Then discard */
  if (intf->olsrIntf == NULL)
  {
    return;
  }

  /* Apply drop list? No, not needed: encapsulated packets are routed,
   * so filtering should be done by adding a rule to the iptables FORWARD
   * chain, e.g.:
   * iptables -A FORWARD -m mac --mac-source 00:0C:29:28:0E:CC -j DROP */

  ethPkt = buffer + ENCAP_HDR_LEN;
  ethPktLen = len - ENCAP_HDR_LEN;

  ipData = (struct ip*) (ethPkt + IP_HDR_OFFSET);

  OLSR_PRINTF(
    9,
    "%s: encapsulated pkt of %d bytes incoming on \"%s\": %s->",
    PLUGIN_NAME_SHORT,
    ethPktLen,
    intf->ifName,
    inet_ntoa(ipData->ip_src));
  OLSR_PRINTF(
    9,
    "%s, forwarded by %s\n",
    inet_ntoa(ipData->ip_dst), /* not possible to call inet_ntoa twice in same printf */
    olsr_ip_to_string(fromIp));

  /* Get encapsulation header */
  encapHdr = (struct TEncapHeader*) buffer;

  /* Check if this packet was seen recently */
  if (CheckAndMarkRecentPacket(Hash16(ntohl(encapHdr->crc32))))
  {
    OLSR_PRINTF(
      9,
      "%s: --> discarding: packet is duplicate\n",
      PLUGIN_NAME_SHORT);
    return;
  }

  if (EtherTunTapFd >= 0)
  {
    struct sockaddr broadAddr;

    /* Unpack encapsulated packet and send a copy to myself via the EtherTunTap interface */
    bufferToWrite = ethPkt;
    nBytesToWrite = ethPktLen;
    if (TunOrTap == TT_TUN)
    {
      bufferToWrite += IP_HDR_OFFSET;
      nBytesToWrite -= IP_HDR_OFFSET;
    }

    ((struct sockaddr_in*)&broadAddr)->sin_addr.s_addr = htonl(EtherTunTapIpBroadcast);
    CheckAndUpdateLocalBroadcast(ethPkt, &broadAddr);

    nBytesWritten = write(EtherTunTapFd, bufferToWrite, nBytesToWrite);
    if (nBytesWritten != nBytesToWrite)
    {
      olsr_printf(
        1,
        "%s: write() error forwarding encapsulated pkt to \"%s\": %s\n",
        PLUGIN_NAME,
        EtherTunTapIfName,
        strerror(errno));
    }
    else
    {
      OLSR_PRINTF(
        9,
        "%s: --> unpacked and forwarded to \"%s\"\n",
        PLUGIN_NAME_SHORT,
        EtherTunTapIfName);
    }
  }

  /* Lookup main address of forwarding node */
  forwarder = mid_lookup_main_addr(fromIp);
  if (forwarder == NULL)
  {
    forwarder = fromIp;
  }

  /* Check if I am MPR for the forwarder */
  iAmMpr = (olsr_lookup_mprs_set(forwarder) != NULL);

  memset(&encapDest, 0, sizeof(encapDest));
  encapDest.sin_family = AF_INET;
  encapDest.sin_port = htons(BMF_ENCAP_PORT);

  nextFwIntf = BmfInterfaces;
  while (nextFwIntf != NULL)
  {
    struct TBmfInterface* fwIntf = nextFwIntf;
    nextFwIntf = fwIntf->next;

    /* Depending on certain conditions, decide whether or not to forward
     * the packet, and if it is forwarded, in which form (encapsulated
     * or not). These conditions are:
     * - is the packet going out on an OLSR interface or not? (isToOlsrIntf)
     * - is this node selected as MPR by the node that forwarded the packet,
     *   or not? (iAmMpr)
     * Note that the packet is always coming in on an OLSR interface, because
     * it is an encapsulated BMF packet.
     *
     * Based on these conditions, 3 cases can be distinguished:
     * - Case 1: What to do with the packet on a non-OLSR interface?
     *   Answer: unpack encapsulated packet, [decrease its TTL and] forward it.
     * - Case 2: The forwarding node has selected us as MPR. What to
     *   do with the packet on an OLSR interface?
     *   Answer: Forward it.
     * - Case 3: The forwarding node has not selected us as MPR. What to
     *   do with the packet on an OLSR interface?
     *   Answer: nothing. Nodes not selected as MPR must not participate in
     *   the flooding.
     */

    /* Forward from OLSR interface to non-OLSR interface: unpack encapsulated
     * packet, [decrease TTL] and forward */
    if (fwIntf->olsrIntf == NULL)
    {
#ifdef DO_TTL_STUFF
      /* If the TTL is to become 0, do not forward this packet */
      if (GetIpTtl(ethPkt) <= 1)
      {
        OLSR_PRINTF(
          9,
          "%s: --> not forwarding on \"%s\": TTL=0\n",
          PLUGIN_NAME_SHORT,
          fwIntf->ifName);
      }
      else
      {
        struct TSaveTtl sttl;
#endif

        /* If the destination address is not a multicast address, it is assumed to be
         * a local broadcast packet. Update the destination address to match the subnet
         * of the network interface on which the packet is being sent. */
        CheckAndUpdateLocalBroadcast(ethPkt, &fwIntf->broadAddr);

#ifdef DO_TTL_STUFF
        /* Save IP header checksum and the TTL-value of the packet, then 
         * decrease the TTL by 1 before writing */
        SaveTtlAndChecksum(ethPkt, &sttl);

        /* Decrease the TTL by 1 before writing */
        DecreaseTtlAndUpdateHeaderChecksum(ethPkt);
#endif

        nBytesWritten = write(fwIntf->capturingSkfd, ethPkt, ethPktLen);
        if (nBytesWritten != ethPktLen)
        {
          olsr_printf(
            1,
            "%s: write() error forwarding unpacked encapsulated pkt to \"%s\": %s\n",
            PLUGIN_NAME,
            fwIntf->ifName,
            strerror(errno));
        }
        else
        {
          OLSR_PRINTF(
            9,
            "%s: --> unpacked and forwarded to \"%s\"\n",
            PLUGIN_NAME_SHORT,
            fwIntf->ifName);
        }

#ifdef DO_TTL_STUFF
        /* Restore the IP header checksum and the TTL-value of the packet */
        RestoreTtlAndChecksum(ethPkt, &sttl);

      } /* if (GetIpTtl(ethPkt) <= 1) */
#endif
    } /* if (fwIntf->olsrIntf == NULL) */

    /* Forward from OLSR interface to OLSR interface: forward the packet if this
     * node is selected as MPR by the forwarding node */
    else if (iAmMpr)
    {
      /* Change source MAC address to that of sending interface */
      memcpy(buffer + IFHWADDRLEN, fwIntf->macAddr, IFHWADDRLEN);

      /* Destination address is local broadcast */
      encapDest.sin_addr.s_addr = ((struct sockaddr_in*)&fwIntf->olsrIntf->int_broadaddr)->sin_addr.s_addr;

      nBytesWritten = sendto(
        fwIntf->encapsulatingSkfd,
        buffer,
        len,
        MSG_DONTROUTE,
        (struct sockaddr*) &encapDest,
        sizeof(encapDest));                   

      if (nBytesWritten != len)
      {
        olsr_printf(
          1,
          "%s: sendto() error forwarding encapsulated pkt via \"%s\": %s\n",
          PLUGIN_NAME,
          fwIntf->ifName,
          strerror(errno));
      }
      else
      {
        OLSR_PRINTF(
          9,
          "%s: --> forwarded to \"%s\"\n",
          PLUGIN_NAME_SHORT,
          fwIntf->ifName);
      }
    }  /* else if (iAmMpr) */
    else /* fwIntf->olsrIntf != NULL && !iAmMpr */
    {
      /* fwIntf is an OLSR interface and I am not selected as MPR */
      OLSR_PRINTF(
        9,
        "%s: --> not forwarding to \"%s\": I am not selected as MPR by %s\n",
        PLUGIN_NAME_SHORT,
        fwIntf->ifName,
        olsr_ip_to_string(fromIp));
    }
  } /* while (nextFwIntf != NULL) */
}

/* -------------------------------------------------------------------------
 * Function   : BmfTunPacketCaptured
 * Description: Handle a raw IP packet captured outgoing on the tuntap interface
 * Input      : buffer - space for the encapsulation header, followed by
 *                the captured packet
 *              len - the number of octets in the encapsulation header plus
 *                captured packet
 * Output     : none
 * Return     : none
 * Data Used  : none
 * Notes      : The packet is assumed to be captured on a socket of family
 *              PF_PACKET and type SOCK_RAW.
 * ------------------------------------------------------------------------- */
static void BmfTunPacketCaptured(
  unsigned char* buffer,
  ssize_t len)
{
  union olsr_ip_addr srcIp;
  union olsr_ip_addr destIp;
  struct TBmfInterface* nextFwIntf;
  unsigned char* ethPkt = buffer + ENCAP_HDR_LEN;
  ssize_t ethPktLen = len - ENCAP_HDR_LEN;
  struct ip* ipData;
  u_int32_t crc32;
  struct TEncapHeader* encapHdr;
  struct sockaddr broadAddr;
  u_int16_t type;

  /* Only forward IPv4 packets */
  memcpy(&type, ethPkt + ETH_TYPE_OFFSET, 2);
  if (ntohs(type) != IPV4_TYPE)
  {
    return;
  }

  ipData = (struct ip*)(ethPkt + IP_HDR_OFFSET);

  /* Only forward multicast packets, or local broadcast packets if specified */
  COPY_IP(&destIp, &ipData->ip_dst);
  ((struct sockaddr_in*)&broadAddr)->sin_addr.s_addr = htonl(EtherTunTapIpBroadcast);
  if (IsMulticast(&destIp) ||
      (EnableLocalBroadcast != 0 && IsLocalBroadcast(&destIp, &broadAddr)))
  {
    /* continue */
  }
  else
  {
    return;
  }

  COPY_IP(&srcIp, &ipData->ip_src);
  OLSR_PRINTF(
    9,
    "%s: outgoing pkt of %d bytes captured on tuntap interface \"%s\": %s->%s\n",
    PLUGIN_NAME_SHORT,
    ethPktLen,
    EtherTunTapIfName,
    olsr_ip_to_string(&srcIp),
    olsr_ip_to_string(&destIp));

  /* Check if this packet was seen recently */
  crc32 = PacketCrc32(ethPkt, ethPktLen);
  if (CheckAndMarkRecentPacket(Hash16(crc32)))
  {
    OLSR_PRINTF(
      9,
      "%s: --> discarding: packet is duplicate\n",
      PLUGIN_NAME_SHORT);
    return;
  }

  /* Compose encapsulation header */
  encapHdr = (struct TEncapHeader*) buffer;
  memset (encapHdr, 0, ENCAP_HDR_LEN);
  encapHdr->crc32 = htonl(crc32);

  /* Check with each interface what needs to be done on it */
  nextFwIntf = BmfInterfaces;
  while (nextFwIntf != NULL)
  {
    int isToOlsrIntf;

    struct TBmfInterface* fwIntf = nextFwIntf;
    nextFwIntf = fwIntf->next;

    /* Is the forwarding interface OLSR-enabled? */
    isToOlsrIntf = (fwIntf->olsrIntf != NULL);

    /* Depending on certain conditions, we decide whether or not to forward
     * the packet, and if it is forwarded, in which form (encapsulated
     * or not). These conditions are:
     * - is the packet going out on an OLSR interface or not? (isToOlsrIntf)
     *
     * Based on these conditions, the following cases can be distinguished:
     *
     * - Case 1: What to do with a packet for an OLSR interface?
     *   Answer: Encapsulate and forward it.
     *
     * - Case 2: What to do with a packet for a non-OLSR interface?
     *   Answer 1: nothing. Multicast routing between non-OLSR interfaces
     *   is to be done by other protocols (e.g. PIM, DVMRP).
     *   Answer 2 (better): Forward it.
     */

    if (isToOlsrIntf)
    {
      /* Case 1: Forward to an OLSR interface.
       * Encapsulate and forward packet. */

      EncapsulateAndForwardPacket(fwIntf, buffer, len);
    }
    else
    {
      /* Case 2: Forward to a non-OLSR interface. */

      int nBytesWritten;

      /* Change source MAC address to that of sending interface */
      memcpy(ethPkt + IFHWADDRLEN, fwIntf->macAddr, IFHWADDRLEN);

      /* If the destination address is not a multicast address, it is assumed to be
       * a local broadcast packet. Update the destination address to match the subnet
       * of the network interface on which the packet is being sent. */
      CheckAndUpdateLocalBroadcast(ethPkt, &fwIntf->broadAddr);

      nBytesWritten = write(fwIntf->capturingSkfd, ethPkt, ethPktLen);
      if (nBytesWritten != ethPktLen)
      {
        olsr_printf(
          1,
          "%s: write() error forwarding pkt for %s to \"%s\": %s\n",
          PLUGIN_NAME,
          olsr_ip_to_string(&destIp),
          fwIntf->ifName,
          strerror(errno));
      }
      else
      {
        OLSR_PRINTF(
          9,
          "%s: --> forwarded from non-OLSR to non-OLSR \"%s\"\n",
          PLUGIN_NAME_SHORT,
          fwIntf->ifName);
      }
    } /* if (isToOlsrIntf) */
  } /* while (nextFwIntf != NULL) */
}

/* -------------------------------------------------------------------------
 * Function   : DoBmf
 * Description: Wait (blocking) for IP packets, then call the handler for each
 *              received packet
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  : BmfInterfaces
 * ------------------------------------------------------------------------- */
static void DoBmf(void)
{
  struct TBmfInterface* currIf;
  int nFdBitsSet;
  /*unsigned char* rxBuffer = malloc(BMF_BUFFER_SIZE);*/
  unsigned char rxBuffer[BMF_BUFFER_SIZE];
  fd_set rxFdSet;

  assert(HighestSkfd >= 0/* && rxBuffer != NULL*/);

  /* Make a local copy of the set of file descriptors that select() can
   * modify to indicate which descriptors actually changed status */
  rxFdSet = InputSet;

  /* Wait (blocking) for packets received on any of the sockets.
   * NOTE: don't use a timeout (last parameter). It causes a high system CPU load! */
  nFdBitsSet = select(HighestSkfd + 1, &rxFdSet, NULL, NULL, NULL);
  if (nFdBitsSet < 0)
  {
    if (errno != EINTR)
    {
      olsr_printf(1, "%s: select() error: %s\n", PLUGIN_NAME, strerror(errno));
    }
    return;
  }

  while (nFdBitsSet > 0)
  {
    /* Check if a packet was received on the capturing socket (if any)
     * of each network interface */
    struct TBmfInterface* nextIf = BmfInterfaces;
    while (nextIf != NULL)
    {
      int skfd;

      currIf = nextIf;
      nextIf = currIf->next;

      skfd = currIf->capturingSkfd;
      if (skfd >= 0 && (FD_ISSET(skfd, &rxFdSet)))
      {
        struct sockaddr_ll pktAddr;
        socklen_t addrLen = sizeof(pktAddr);
        int nBytes;
        unsigned char* ethPkt;

        /* A packet was captured. */

        nFdBitsSet--;

        /* Receive the packet, leaving space for the BMF encapsulation header */
        ethPkt = rxBuffer + ENCAP_HDR_LEN;
        nBytes = recvfrom(
          skfd,
          ethPkt,
          BMF_BUFFER_SIZE - ENCAP_HDR_LEN,
          0,
          (struct sockaddr*)&pktAddr,
          &addrLen);
        if (nBytes < 0)
        {
          olsr_printf(
            1,
            "%s: recvfrom() error on \"%s\": %s\n",
            PLUGIN_NAME,
            currIf->ifName,
            strerror(errno));
        }
        else if (nBytes < IP_HDR_OFFSET + (int)sizeof(struct iphdr))
        {
          olsr_printf(
            1,
            "%s: captured packet too short (%d bytes) on \"%s\"\n",
            PLUGIN_NAME,
            nBytes,
            currIf->ifName);
        }
        else
        {
          /* Don't trust the number of bytes as returned from 'recvfrom':
           * that number may be 4 bytes too large, in case the receiving
           * network interface is a VLAN interface. */
          nBytes = IP_HDR_OFFSET + GetIpPacketLength(ethPkt);

          /* Don't let BMF crash by sending too short packets */
          if (nBytes >= IP_HDR_OFFSET + GetIpHeaderLength(ethPkt))
          {
            if (pktAddr.sll_pkttype == PACKET_OUTGOING ||
                pktAddr.sll_pkttype == PACKET_MULTICAST ||
                pktAddr.sll_pkttype == PACKET_BROADCAST)
            {
              /* A multicast or broadcast packet was captured */
              BmfPacketCaptured(currIf, pktAddr.sll_pkttype, rxBuffer, nBytes + ENCAP_HDR_LEN);
            }
          }
          else
          {
            olsr_printf(
              1,
              "%s: captured packet too short (%d bytes) on \"%s\"\n",
              PLUGIN_NAME,
              nBytes,
              currIf->ifName);
          }
        } /* if (nBytes < 0) */
      } /* if (skfd >= 0 && (FD_ISSET...)) */
    } /* while (nextIf != NULL) */
    
    /* Check if a packet was received on the encapsulating socket (if any)
     * of each network interface */
    nextIf = BmfInterfaces;
    while (nextIf != NULL)
    {
      int skfd;

      currIf = nextIf;
      nextIf = currIf->next;

      skfd = currIf->encapsulatingSkfd;
      if (skfd >= 0 && (FD_ISSET(skfd, &rxFdSet)))
      {
        struct sockaddr_in from;
        socklen_t fromLen = sizeof(from);
        int nBytes;

        /* An encapsulated packet was received */

        nFdBitsSet--;

        nBytes = recvfrom(
          skfd,
          rxBuffer,
          BMF_BUFFER_SIZE,
          0,
          (struct sockaddr*)&from,
          &fromLen);
        if (nBytes < 0)
        {
          olsr_printf(
            1,
            "%s: recvfrom() error on \"%s\": %s\n",
            PLUGIN_NAME,
            currIf->ifName,
            strerror(errno));
        }
        else
        {
          /* Don't let BMF crash by sending too short packets. */
          if (nBytes >= IP_HDR_OFFSET + GetIpHeaderLength(rxBuffer))
          {
            union olsr_ip_addr srcIp;
            COPY_IP(&srcIp, &from.sin_addr.s_addr);
            BmfEncapsulatedPacketReceived(currIf, &srcIp, rxBuffer, nBytes);
          }
          else
          {
            olsr_printf(
              1,
              "%s: encapsulated packet too short (%d bytes) from %s on \"%s\"\n",
              PLUGIN_NAME,
              nBytes,
              inet_ntoa(from.sin_addr),
              currIf->ifName);
          }
        } /* if (nBytes < 0) */
      } /* if (skfd >= 0 && (FD_ISSET...)) */
    } /* while (nextIf != NULL) */

    if (nFdBitsSet > 0 && FD_ISSET(EtherTunTapFd, &rxFdSet))
    {
      /* Check if an application has sent a packet to the tuntap
       * network interface */

      int nBytes;
      unsigned char* ethPkt;
      unsigned char* bufferToRead;
      size_t nBytesToRead;

      nFdBitsSet--;

      /* Receive the packet, leaving space for the BMF encapsulation header */
      ethPkt = rxBuffer + ENCAP_HDR_LEN;
    
      bufferToRead = ethPkt;
      nBytesToRead = BMF_BUFFER_SIZE - ENCAP_HDR_LEN;
      if (TunOrTap == TT_TUN)
      {
        u_int16_t type;

        /* When using a tun-interface, also leave space for an Ethernet header */
        bufferToRead += IP_HDR_OFFSET;
        nBytesToRead -= IP_HDR_OFFSET;

        /* Compose an Ethernet header, in case other BMF-nodes are receiving
         * their BMF packets on a tap-interface. */

        /* Use all-ones as destination MAC address. When the IP destination is
         * a multicast address, the destination MAC address should normally also
         * be a multicast address. E.g., when the destination IP is 224.0.0.1,
         * the destination MAC should be 01:00:5e:00:00:01. However, it does not
         * seem to matter when the destination MAC address is set to all-ones
         * in that case. */
        memset(ethPkt, 0xFF, IFHWADDRLEN);

        /* Source MAC address is not important. Set to all-zeros */
        memset(ethPkt + IFHWADDRLEN, 0x00, IFHWADDRLEN);

        /* Ethertype = 0800 = IP */
        type = htons(0x0800);
        memcpy(ethPkt + ETH_TYPE_OFFSET, &type, 2);
      }

      nBytes = read(
        EtherTunTapFd,
        bufferToRead,
        nBytesToRead);

      if (nBytes < 0)
      {
        if (errno != EAGAIN)
        {
          olsr_printf(
            1,
            "%s: recvfrom() error on \"%s\": %s\n",
            PLUGIN_NAME,
            EtherTunTapIfName,
            strerror(errno));
        }
      }
      else if (nBytes < (int)sizeof(struct iphdr))
      {
        olsr_printf(
          1,
          "%s: captured packet too short (%d bytes) on \"%s\"\n",
          PLUGIN_NAME,
          nBytes,
          EtherTunTapIfName);
      }
      else
      {
        /* Don't trust the number of bytes as returned from 'recvfrom':
         * that number may be 4 bytes too large, in case the receiving
         * network interface is a VLAN interface. */
        nBytes = IP_HDR_OFFSET + GetIpPacketLength(ethPkt);

        /* Don't let BMF crash by sending too short packets */
        if (nBytes >= IP_HDR_OFFSET + GetIpHeaderLength(ethPkt))
        {
          /* An outbound packet was captured */
          BmfTunPacketCaptured(rxBuffer, nBytes + ENCAP_HDR_LEN);
        }
        else
        {
          olsr_printf(
            1,
            "%s: captured packet too short (%d bytes) on \"%s\"\n",
            PLUGIN_NAME,
            nBytes,
            EtherTunTapIfName);
        } /* if (nBytes >= IP_HDR_OFFSET... */
      } /* if (nBytes < 0) */
    } /* if (nFdBitsSet > 0 && ... */
  } /* while (nFdBitsSet > 0) */
  /*free(rxBuffer);*/
}

/* -------------------------------------------------------------------------
 * Function   : BmfSignalHandler
 * Description: Signal handler function
 * Input      : signo - signal being handled
 * Output     : none
 * Return     : none
 * Data Used  : BmfThreadRunning
 * ------------------------------------------------------------------------- */
static void BmfSignalHandler(int signo)
{
  BmfThreadRunning = 0;
}

/* -------------------------------------------------------------------------
 * Function   : BmfRun
 * Description: Receiver thread function
 * Input      : useless - not used
 * Output     : none
 * Return     : not used
 * Data Used  : BmfThreadRunning
 * Notes      : Another thread can gracefully stop this thread by sending
 *              a SIGALRM signal.
 * ------------------------------------------------------------------------- */
static void* BmfRun(void* useless)
{
  /* Mask all signals except SIGALRM */
  sigset_t blockedSigs;
  sigfillset(&blockedSigs);
  sigdelset(&blockedSigs, SIGALRM);
  if (pthread_sigmask(SIG_BLOCK, &blockedSigs, NULL) != 0)
  {
    olsr_printf(1, "%s: pthread_sigmask() error: %s\n", PLUGIN_NAME, strerror(errno));
  }

  /* Set up the signal handler for the process: use SIGALRM to terminate
   * the BMF thread. Only if a signal handler is specified, does a blocking
   * system call return with errno set to EINTR; if a signal hander is not
   * specified, any system call in which the thread may be waiting will not
   * return. Note that the BMF thread is usually blocked in the select()
   * function (see DoBmf()). */
  if (signal(SIGALRM, BmfSignalHandler) == SIG_ERR)
  {
    olsr_printf(1, "%s: signal() error: %s\n", PLUGIN_NAME, strerror(errno));
  }

  /* Call the thread function until flagged to exit */
  while (BmfThreadRunning != 0)
  {
    DoBmf();
  }

  return NULL;
}

/* -------------------------------------------------------------------------
 * Function   : InterfaceChange
 * Description: Callback function passed to OLSRD for it to call whenever a
 *              network interface has been added, removed or updated
 * Input      : interf - the network interface to deal with
 *              action - indicates if the specified network interface was
 *                added, removed or updated.
 * Output     : none
 * Return     : always 0
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int InterfaceChange(struct interface* interf, int action)
{
  switch (action)
  {
  case (IFCHG_IF_ADD):
    AddInterface(interf);
    olsr_printf(1, "%s: interface %s added\n", PLUGIN_NAME, interf->int_name);
    break;

  case (IFCHG_IF_REMOVE):
    /* We cannot just remove the interface, because the receive-thread is likely
     * to be waiting in select(...) for packets coming in via the interface.
     * Therefore we first close BMF (CloseBmf()), interrupting and kiling the
     * receive-thread so that it is safe to remove this (and all other)
     * interfaces. After that, BMF is re-started (InitBmf(interf)). */
    CloseBmf();
    InitBmf(interf);
    olsr_printf(1, "%s: interface %s removed\n", PLUGIN_NAME, interf->int_name);
    break;

  case (IFCHG_IF_UPDATE):
    olsr_printf(1, "%s: interface %s updated\n", PLUGIN_NAME, interf->int_name);
    break;
      
  default:
    olsr_printf(
      1,
      "%s: interface %s: error - unknown action (%d)\n",
      PLUGIN_NAME,
      interf->int_name, action);
    break;
  }
  return 0;
}

/* -------------------------------------------------------------------------
 * Function   : InitBmf
 * Description: Initialize the BMF plugin
 * Input      : skipThisIntf - specifies which network interface should not
 *              be enabled for BMF. Pass NULL if not applicable.
 * Output     : none
 * Return     : fail (0) or success (1)
 * Data Used  : BmfThreadRunning, BmfThread
 * ------------------------------------------------------------------------- */
int InitBmf(struct interface* skipThisIntf)
{
  CreateBmfNetworkInterfaces(skipThisIntf);

  /* Start running the multicast packet processing thread */
  BmfThreadRunning = 1;
  if (pthread_create(&BmfThread, NULL, BmfRun, NULL) != 0)
  {
    olsr_printf(1, "%s: pthread_create error: %s\n", PLUGIN_NAME, strerror(errno));
    return 0;
  }
  return 1;
}

/* -------------------------------------------------------------------------
 * Function   : CloseBmf
 * Description: Close the BMF plugin and clean up
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  : BmfThreadRunning, BmfThread
 * ------------------------------------------------------------------------- */
void CloseBmf()
{
  /* Signal BmfThread to exit */
  if (pthread_kill(BmfThread, SIGALRM) != 0)
  /* Strangely enough, all running threads receive the SIGALRM signal. But only the
   * BMF thread is affected by this signal, having specified a handler for this
   * signal in its thread entry function BmfRun(...). */
  {
    olsr_printf(1, "%s: pthread_kill() error: %s\n", PLUGIN_NAME, strerror(errno));
  }

  /* Wait for BmfThread to acknowledge */
  if (pthread_join(BmfThread, NULL) != 0)
  {
    olsr_printf(1, "%s: pthread_join() error: %s\n", PLUGIN_NAME, strerror(errno));
  }

  /* Time to clean up */
  CloseBmfNetworkInterfaces();
}

/* -------------------------------------------------------------------------
 * Function   : RegisterBmfParameter
 * Description: Register a configuration parameter with the BMF process
 * Input      : key - the parameter name, e.g. "DropMac" or "NonOlsrIf"
 *              value - the parameter value
 * Output     : none
 * Return     : fatal error (<0), minor error (0) or success (>0)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int RegisterBmfParameter(char* key, char* value)
{
  if (strcmp(key, "DropMac") == 0)
  {
    return DropMac(value);
  }
  else if (strcmp(key, "NonOlsrIf") == 0)
  {
    return AddNonOlsrBmfIf(value);
  }
  else if (strcmp(key, "DoLocalBroadcast") == 0)
  {
    return DoLocalBroadcast(value);
  }
  else if (strcmp(key, "BmfInterface") == 0)
  {
    return SetBmfInterfaceName(value);
  }
  else if (strcmp(key, "BmfInterfaceType") == 0)
  {
    return SetBmfInterfaceType(value);
  }
  else if (strcmp(key, "BmfInterfaceIp") == 0)
  {
    return SetBmfInterfaceIp(value);
  }
  else if (strcmp(key, "CapturePacketsOnOlsrInterfaces") == 0)
  {
    return SetCapturePacketsOnOlsrInterfaces(value);
  }

  /* Key not recognized */
  return 0;
}
