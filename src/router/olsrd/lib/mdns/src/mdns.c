/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004-2009, the olsr.org team - see HISTORY file
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


#include "mdns.h"

/* System includes */
#include <stddef.h>             /* NULL */
#include <sys/types.h>          /* ssize_t */
#include <string.h>             /* strerror() */
#include <stdarg.h>             /* va_list, va_start, va_end */
#include <errno.h>              /* errno */
#include <assert.h>             /* assert() */
#include <linux/if_ether.h>     /* ETH_P_IP */
#include <linux/if_packet.h>    /* struct sockaddr_ll, PACKET_MULTICAST */
//#include <pthread.h> /* pthread_t, pthread_create() */
#include <signal.h>             /* sigset_t, sigfillset(), sigdelset(), SIGINT */
#include <netinet/ip.h>         /* struct ip */
#include <netinet/udp.h>        /* struct udphdr */
#include <unistd.h>             /* close() */

#include <netinet/in.h>
#include <netinet/ip6.h>

/* OLSRD includes */
#include "plugin_util.h"        /* set_plugin_int */
#include "defs.h"               /* olsr_cnf, //OLSR_PRINTF */
#include "ipcalc.h"
#include "olsr.h"               /* //OLSR_PRINTF */
#include "mid_set.h"            /* mid_lookup_main_addr() */
#include "link_set.h"           /* get_best_link_to_neighbor() */
#include "net_olsr.h"           /* ipequal */

/* plugin includes */
#include "NetworkInterfaces.h"  /* TBmfInterface, CreateBmfNetworkInterfaces(), CloseBmfNetworkInterfaces() */
#include "Address.h"            /* IsMulticast() */
#include "Packet.h"             /* ENCAP_HDR_LEN, BMF_ENCAP_TYPE, BMF_ENCAP_LEN etc. */

int my_DNS_TTL=0;

/* -------------------------------------------------------------------------
 * Function   : PacketReceivedFromOLSR
 * Description: Handle a received packet from a OLSR message
 * Input      : ipPacket into an unsigned char and the lenght of the packet
 * Output     : none
 * Return     : none
 * Data Used  : BmfInterfaces
 * ------------------------------------------------------------------------- */
static void
PacketReceivedFromOLSR(unsigned char *encapsulationUdpData, int len)
{
  struct ip *ipHeader;                 /* IP header inside the encapsulated IP packet */
  struct ip6_hdr *ip6Header;                 /* IP header inside the encapsulated IP packet */
  //union olsr_ip_addr mcSrc;            /* Original source of the encapsulated multicast packet */
  //union olsr_ip_addr mcDst;            /* Multicast destination of the encapsulated packet */
  struct TBmfInterface *walker;
  int stripped_len = 0;
  ipHeader = (struct ip *)ARM_NOWARN_ALIGN(encapsulationUdpData);
  ip6Header = (struct ip6_hdr *)ARM_NOWARN_ALIGN(encapsulationUdpData);

  //mcSrc.v4 = ipHeader->ip_src;
  //mcDst.v4 = ipHeader->ip_dst;
  //OLSR_DEBUG(LOG_PLUGINS, "MDNS PLUGIN got packet from OLSR message\n");


  /* Check with each network interface what needs to be done on it */
  for (walker = BmfInterfaces; walker != NULL; walker = walker->next) {
    /* To a non-OLSR interface: unpack the encapsulated IP packet and forward it */
    if (walker->olsrIntf == NULL) {
      int nBytesWritten;
      struct sockaddr_ll dest;

      memset(&dest, 0, sizeof(dest));
      dest.sll_family = AF_PACKET;
      if ((encapsulationUdpData[0] & 0xf0) == 0x40) {
        dest.sll_protocol = htons(ETH_P_IP);
	stripped_len = ntohs(ipHeader->ip_len);
	}
      if ((encapsulationUdpData[0] & 0xf0) == 0x60) {
        dest.sll_protocol = htons(ETH_P_IPV6);
        stripped_len = 40 + ntohs(ip6Header->ip6_plen); //IPv6 Header size (40) + payload_len 
        }
      // Sven-Ola: Don't know how to handle the "stripped_len is uninitialized" condition, maybe exit(1) is better...?
      if (0 == stripped_len) return;
      //TODO: if packet is not IP die here
      
      if (stripped_len > len) {
	//OLSR_DEBUG(LOG_PLUGINS, "MDNS: Stripped len bigger than len ??\n");
	}
      dest.sll_ifindex = if_nametoindex(walker->ifName);
      dest.sll_halen = IFHWADDRLEN;

      /* Use all-ones as destination MAC address. When the IP destination is
       * a multicast address, the destination MAC address should normally also
       * be a multicast address. E.g., when the destination IP is 224.0.0.1,
       * the destination MAC should be 01:00:5e:00:00:01. However, it does not
       * seem to matter when the destination MAC address is set to all-ones
       * in that case. */
      memset(dest.sll_addr, 0xFF, IFHWADDRLEN);

      nBytesWritten = sendto(walker->capturingSkfd, encapsulationUdpData, stripped_len, 0, (struct sockaddr *)&dest, sizeof(dest));
      if (nBytesWritten != stripped_len) {
        BmfPError("sendto() error forwarding unpacked encapsulated pkt on \"%s\"", walker->ifName);
      } else {

        //OLSR_PRINTF(
        //  2,
        //  "%s: --> unpacked and forwarded on \"%s\"\n",
        //  PLUGIN_NAME_SHORT,
        //  walker->ifName);
      }
    }                           /* if (walker->olsrIntf == NULL) */
  }
}                               /* PacketReceivedFromOLSR */


bool
olsr_parser(union olsr_message *m, struct interface *in_if __attribute__ ((unused)), union olsr_ip_addr *ipaddr)
{
  union olsr_ip_addr originator;
  int size;
  uint32_t vtime;
  //OLSR_DEBUG(LOG_PLUGINS, "MDNS PLUGIN: Received msg in parser\n");
  /* Fetch the originator of the messsage */
  if (olsr_cnf->ip_version == AF_INET) {
    memcpy(&originator, &m->v4.originator, olsr_cnf->ipsize);
    vtime = me_to_reltime(m->v4.olsr_vtime);
    size = ntohs(m->v4.olsr_msgsize);
  } else {
    memcpy(&originator, &m->v6.originator, olsr_cnf->ipsize);
    vtime = me_to_reltime(m->v6.olsr_vtime);
    size = ntohs(m->v6.olsr_msgsize);
  }

  /* Check if message originated from this node.
   *         If so - back off */
  if (ipequal(&originator, &olsr_cnf->main_addr))
    return false;

  /* Check that the neighbor this message was received from is symmetric.
   *         If not - back off*/
  if (check_neighbor_link(ipaddr) != SYM_LINK) {
    //struct ipaddr_str strbuf;
    //OLSR_PRINTF(3, "NAME PLUGIN: Received msg from NON SYM neighbor %s\n", olsr_ip_to_string(&strbuf, ipaddr));
    return false;
  }

  if (olsr_cnf->ip_version == AF_INET) {
    PacketReceivedFromOLSR((unsigned char *)&m->v4.message, size - 12);
  } else {
    PacketReceivedFromOLSR((unsigned char *)&m->v6.message, size - 12 - 96);
  }
//forward the message
return true;
}

//Sends a packet in the OLSR network
void
olsr_mdns_gen(unsigned char *packet, int len)
{
  /* send buffer: huge */
  char buffer[10240];
  int aligned_size;
  union olsr_message *message = (union olsr_message *)buffer;
  struct interface *ifn;
  
  aligned_size=len;

if ((aligned_size % 4) != 0) {
    aligned_size = (aligned_size - (aligned_size % 4)) + 4;
  }

  /* fill message */
  if (olsr_cnf->ip_version == AF_INET) {
    /* IPv4 */
    message->v4.olsr_msgtype = MESSAGE_TYPE;
    message->v4.olsr_vtime = reltime_to_me(MDNS_VALID_TIME * MSEC_PER_SEC);
    memcpy(&message->v4.originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
    //message->v4.ttl = MAX_TTL;
    if (my_MDNS_TTL) message->v4.ttl = my_MDNS_TTL;
    else message->v4.ttl = MAX_TTL;
    message->v4.hopcnt = 0;
    message->v4.seqno = htons(get_msg_seqno());

    message->v4.olsr_msgsize = htons(aligned_size + 12);

    memset(&message->v4.message, 0, aligned_size);
    memcpy(&message->v4.message, packet, len);
    aligned_size = aligned_size + 12;
  } else {
    /* IPv6 */
    message->v6.olsr_msgtype = MESSAGE_TYPE;
    message->v6.olsr_vtime = reltime_to_me(MDNS_VALID_TIME * MSEC_PER_SEC);
    memcpy(&message->v6.originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
    //message->v6.ttl = MAX_TTL;
    if (my_MDNS_TTL) message->v6.ttl = my_MDNS_TTL;
    else message->v6.ttl = MAX_TTL;
    message->v6.hopcnt = 0;
    message->v6.seqno = htons(get_msg_seqno());

    message->v6.olsr_msgsize = htons(aligned_size + 12 + 96);
    memset(&message->v6.message, 0, aligned_size);
    memcpy(&message->v6.message, packet, len);
    aligned_size = aligned_size + 12 + 96;
  }

  /* looping trough interfaces */
 for (ifn = ifnet; ifn; ifn = ifn->int_next) {
    //OLSR_PRINTF(1, "MDNS PLUGIN: Generating packet - [%s]\n", ifn->int_name);

    if (net_outbuffer_push(ifn, message, aligned_size) != aligned_size) {
      /* send data and try again */
      net_output(ifn);
      if (net_outbuffer_push(ifn, message, aligned_size) != aligned_size) {
        //OLSR_PRINTF(1, "MDNS PLUGIN: could not send on interface: %s\n", ifn->int_name);
      }
    }
  }
}

/* -------------------------------------------------------------------------
 * Function   : BmfPError
 * Description: Prints an error message at OLSR debug level 1.
 *              First the plug-in name is printed. Then (if format is not NULL
 *              and *format is not empty) the arguments are printed, followed
 *              by a colon and a blank. Then the message and a new-line.
 * Input      : format, arguments
 * Output     : none
 * Return     : none
 * Data Used  : none
 * ------------------------------------------------------------------------- */

void
BmfPError(const char *format, ...)
{
#define MAX_STR_DESC 255
  char strDesc[MAX_STR_DESC];

#if !defined REMOVE_LOG_DEBUG
  //char *stringErr = strerror(errno);
#endif
  /* Rely on short-circuit boolean evaluation */
  if (format == NULL || *format == '\0') {
    //OLSR_DEBUG(LOG_PLUGINS, "%s: %s\n", PLUGIN_NAME, stringErr);
  } else {
    va_list arglist;

    va_start(arglist, format);
    vsnprintf(strDesc, MAX_STR_DESC, format, arglist);
    va_end(arglist);

    strDesc[MAX_STR_DESC - 1] = '\0';   /* Ensures null termination */

    //OLSR_DEBUG(LOG_PLUGINS, "%s: %s\n", strDesc, stringErr);
  }
}                               /* BmfPError */

/* -------------------------------------------------------------------------
 * Function   : MainAddressOf
 * Description: Lookup the main address of a node
 * Input      : ip - IP address of the node
 * Output     : none
 * Return     : The main IP address of the node
 * Data Used  : none
 * ------------------------------------------------------------------------- */
union olsr_ip_addr *
MainAddressOf(union olsr_ip_addr *ip)
{
  union olsr_ip_addr *result;

  /* TODO: mid_lookup_main_addr() is not thread-safe! */
  result = mid_lookup_main_addr(ip);
  if (result == NULL) {
    result = ip;
  }
  return result;
}                               /* MainAddressOf */


/* -------------------------------------------------------------------------
 * Function   : BmfPacketCaptured
 * Description: Handle a captured IP packet
 * Input      : intf - the network interface on which the packet was captured
 *              sllPkttype - the type of packet. Either PACKET_OUTGOING,
 *                PACKET_BROADCAST or PACKET_MULTICAST.
 *              encapsulationUdpData - space for the encapsulation header, followed by
 *                the captured IP packet
 * Output     : none
 * Return     : none
 * Data Used  : BmfInterfaces
 * Notes      : The IP packet is assumed to be captured on a socket of family
 *              PF_PACKET and type SOCK_DGRAM (cooked).
 * ------------------------------------------------------------------------- */
static void
BmfPacketCaptured(
                   //struct TBmfInterface* intf,
                   //unsigned char sllPkttype,
                   unsigned char *encapsulationUdpData, int nBytes)
{
  union olsr_ip_addr src;              /* Source IP address in captured packet */
  union olsr_ip_addr dst;              /* Destination IP address in captured packet */
  union olsr_ip_addr *origIp;          /* Main OLSR address of source of captured packet */
  struct ip *ipHeader;                 /* The IP header inside the captured IP packet */
  struct ip6_hdr *ipHeader6;           /* The IP header inside the captured IP packet */
  struct udphdr *udpHeader;
  u_int16_t destPort;

  if ((encapsulationUdpData[0] & 0xf0) == 0x40) {       //IPV4

    ipHeader = (struct ip *)ARM_NOWARN_ALIGN(encapsulationUdpData);

    dst.v4 = ipHeader->ip_dst;

    /* Only forward multicast packets. If configured, also forward local broadcast packets */
    if (IsMulticast(&dst)) {
      /* continue */
    } else {
      return;
    }
    if (ipHeader->ip_p != SOL_UDP) {
      /* Not UDP */
      //OLSR_PRINTF(1,"NON UDP PACKET\n");
      return;                   /* for */
    }
    udpHeader = (struct udphdr *)ARM_NOWARN_ALIGN(encapsulationUdpData + GetIpHeaderLength(encapsulationUdpData));
    destPort = ntohs(udpHeader->dest);
    if (destPort != 5353) {
      return;
    }
  }                             //END IPV4

  else if ((encapsulationUdpData[0] & 0xf0) == 0x60) {  //IPv6

    ipHeader6 = (struct ip6_hdr *)ARM_NOWARN_ALIGN(encapsulationUdpData);
    if (ipHeader6->ip6_dst.s6_addr[0] == 0xff)  //Multicast
    {
      //Continua
    } else {
      return;                   //not multicast
    }
    if (ipHeader6->ip6_nxt != SOL_UDP) {
      /* Not UDP */
      //OLSR_PRINTF(1,"NON UDP PACKET\n");
      return;                   /* for */
    }
    udpHeader = (struct udphdr *)ARM_NOWARN_ALIGN(encapsulationUdpData + 40);
    destPort = ntohs(udpHeader->dest);
    if (destPort != 5353) {
      return;
    }
  }                             //END IPV6
  else
    return;                     //Is not IP packet

  /* Check if the frame is captured on an OLSR-enabled interface */
  //isFromOlsrIntf = (intf->olsrIntf != NULL); TODO: put again this check


  /* Lookup main address of source in the MID table of OLSR */
  origIp = MainAddressOf(&src);

  // send the packet to OLSR forward mechanism
  olsr_mdns_gen(encapsulationUdpData, nBytes);
}                               /* BmfPacketCaptured */


/* -------------------------------------------------------------------------
 * Function   : DoMDNS
 * Description: This function is registered with the OLSR scheduler and called when something is captured
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  :
 * ------------------------------------------------------------------------- */
void
DoMDNS(int skfd, void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused)))
{
  unsigned char rxBuffer[BMF_BUFFER_SIZE];
  if (skfd >= 0) {
    struct sockaddr_ll pktAddr;
    socklen_t addrLen = sizeof(pktAddr);
    int nBytes;
    unsigned char *ipPacket;

    /* Receive the captured Ethernet frame, leaving space for the BMF
     * encapsulation header */
    ipPacket = GetIpPacket(rxBuffer);
    nBytes = recvfrom(skfd, ipPacket, BMF_BUFFER_SIZE,  //TODO: understand how to change this
                      0, (struct sockaddr *)&pktAddr, &addrLen);
    if (nBytes < 0) {

      return;                   /* for */
    }

    /* if (nBytes < 0) */
    /* Check if the number of received bytes is large enough for an IP
     * packet which contains at least a minimum-size IP header.
     * Note: There is an apparent bug in the packet socket implementation in
     * combination with VLAN interfaces. On a VLAN interface, the value returned
     * by 'recvfrom' may (but need not) be 4 (bytes) larger than the value
     * returned on a non-VLAN interface, for the same ethernet frame. */
    if (nBytes < (int)sizeof(struct ip)) {
      ////OLSR_PRINTF(
      //              1,
      //              "%s: captured frame too short (%d bytes) on \"%s\"\n",
      //              PLUGIN_NAME,
      //              nBytes,
      //              walker->ifName);

      return;                   /* for */
    }

    if (pktAddr.sll_pkttype == PACKET_OUTGOING ||
        pktAddr.sll_pkttype == PACKET_MULTICAST || pktAddr.sll_pkttype == PACKET_BROADCAST) {
      /* A multicast or broadcast packet was captured */

      ////OLSR_PRINTF(
      //              1,
      //              "%s: captured frame (%d bytes) on \"%s\"\n",
      //              PLUGIN_NAME,
      //              nBytes,
      //              walker->ifName);
      //BmfPacketCaptured(walker, pktAddr.sll_pkttype, rxBuffer);
      BmfPacketCaptured(ipPacket, nBytes);

    }                           /* if (pktAddr.sll_pkttype == ...) */
  }                             /* if (skfd >= 0 && (FD_ISSET...)) */
}                               /* DoMDNS */

int
InitMDNS(struct interface *skipThisIntf)
{


  //Tells OLSR to launch olsr_parser when the packets for this plugin arrive
  olsr_parser_add_function(&olsr_parser, PARSER_TYPE);
  //Creates captures sockets and register them to the OLSR scheduler
  CreateBmfNetworkInterfaces(skipThisIntf);

  return 1;
}                               /* InitMDNS */

/* -------------------------------------------------------------------------
 * Function   : CloseMDNS
 * Description: Close the MDNS plugin and clean up
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  :
 * ------------------------------------------------------------------------- */
void
CloseMDNS(void)
{
  CloseBmfNetworkInterfaces();
}
