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
#include "list_backport.h"
#include "RouterElection.h"
#include "list_backport.h"

#define OLSR_FOR_ALL_FILTEREDNODES_ENTRIES(n, iterator) listbackport_for_each_element_safe(&ListOfFilteredHosts, n, list, iterator)

#define IPH_HL(hdr) (((hdr)->ip_hl)*4)

struct list_entity ListOfFilteredHosts;
int FHListInit = 0;

static uint16_t ip_checksum(char* data, int len)
{
    uint sum = 0;
    if ((len & 1) == 0)
        len = len >> 1;
    else
        len = (len >> 1) + 1;
    while (len > 0) {
        sum += *((unsigned short int*)(void *)data);
        data += sizeof(unsigned short int);
        len--;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return(~sum);
}


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
  uint16_t csum_ip;
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
	if(my_TTL_Check)
		ipHeader->ip_ttl = (u_int8_t) 1; //setting up TTL to 1 to avoid mdns packets flood 
	}
	//Recalculate IP Checksum
	ipHeader->ip_sum=0x0000;
	csum_ip = ip_checksum((char*)ipHeader, IPH_HL(ipHeader));
	ipHeader->ip_sum=csum_ip;

      if ((encapsulationUdpData[0] & 0xf0) == 0x60) {
        dest.sll_protocol = htons(ETH_P_IPV6);
        stripped_len = 40 + ntohs(ip6Header->ip6_plen); //IPv6 Header size (40) + payload_len 
        if(my_TTL_Check)
		ip6Header->ip6_hops = (uint8_t) 1; //setting up Hop Limit to 1 to avoid mdns packets flood
        }
      // Sven-Ola: Don't know how to handle the "stripped_len is uninitialized" condition, maybe olsr_exit is better...?
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
      
      if(walker->isActive == 0) {            //Don't forward packet if isn't master router
       OLSR_PRINTF(1,"Not forwarding mDNS packet to interface %s because this router is not master\n",walker->ifName);
       return;
       }
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
olsr_parser(union olsr_message *m, struct interface_olsr *in_if __attribute__ ((unused)), union olsr_ip_addr *ipaddr)
{
  union olsr_ip_addr originator;
  int size;
  //OLSR_DEBUG(LOG_PLUGINS, "MDNS PLUGIN: Received msg in parser\n");
  /* Fetch the originator of the messsage */
  if (olsr_cnf->ip_version == AF_INET) {
    memcpy(&originator, &m->v4.originator, olsr_cnf->ipsize);
    size = ntohs(m->v4.olsr_msgsize);
  } else {
    memcpy(&originator, &m->v6.originator, olsr_cnf->ipsize);
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
  struct interface_olsr *ifn;
  
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
#endif /* !defined REMOVE_LOG_DEBUG */
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

int
AddFilteredHost(const char *FilteredHost, void *data __attribute__ ((unused)), 
		set_plugin_parameter_addon addon __attribute__ ((unused))){

  int res = 0;
  struct FilteredHost *tmp;
  tmp = (struct FilteredHost *) malloc(sizeof(struct FilteredHost));
  listbackport_init_node(&tmp->list);

  if(FHListInit == 0){
    listbackport_init_head(&ListOfFilteredHosts);
    FHListInit = 1;
  }

  if(olsr_cnf->ip_version == AF_INET){
    res = inet_pton(AF_INET, FilteredHost, &tmp->host.v4);
    if(res > 0){
      listbackport_add_tail(&ListOfFilteredHosts, &tmp->list);
    }
    else
      free(tmp);
  }
  else{
    res = inet_pton(AF_INET6, FilteredHost, &tmp->host.v6);
    if(res > 0)
      listbackport_add_tail(&ListOfFilteredHosts, &tmp->list);
    else
      free(tmp);
  }

  return 0;
}

int
isInFilteredList(union olsr_ip_addr *src){

  struct FilteredHost *tmp, *iterator;
  struct ipaddr_str buf1;
  struct ipaddr_str buf2;
  
  if(FHListInit == 0){
    listbackport_init_head(&ListOfFilteredHosts);
    FHListInit = 1;
  }


  if(listbackport_is_empty(&ListOfFilteredHosts)) {
    OLSR_PRINTF(2,"Accept packet captured because of filtered hosts ACL: List Empty\n");
    return 0;
  }

  OLSR_FOR_ALL_FILTEREDNODES_ENTRIES(tmp, iterator){
    OLSR_PRINTF(2, "Checking host: %s against list entry: %s\n", olsr_ip_to_string(&buf1, src), olsr_ip_to_string(&buf2, &tmp->host) );
    if(olsr_cnf->ip_version == AF_INET){
      if(memcmp(&tmp->host.v4, &src->v4, sizeof(struct in_addr)) == 0)
        return 1;
    }
    else{
      if(memcmp(&tmp->host.v6, &src->v6, sizeof(struct in6_addr)) == 0)
        return 1;
    }
  }

  OLSR_PRINTF(2,"Accept packet captured because of filtered hosts ACL: Did not find any match in list\n");
  return 0;
}

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
  union olsr_ip_addr dst;              /* Destination IP address in captured packet */
  union olsr_ip_addr src;              
  struct ip *ipHeader;                 /* The IP header inside the captured IP packet */
  struct ip6_hdr *ipHeader6;           /* The IP header inside the captured IP packet */
  struct udphdr *udpHeader;
  u_int16_t destPort;

  if ((encapsulationUdpData[0] & 0xf0) == 0x40) {       //IPV4

    ipHeader = (struct ip *)ARM_NOWARN_ALIGN(encapsulationUdpData);

    dst.v4 = ipHeader->ip_dst;
    src.v4 = ipHeader->ip_src;

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
#if defined(__GLIBC__) || defined(__BIONIC__)
    destPort = ntohs(udpHeader->dest);
#else
    destPort = ntohs(udpHeader->uh_dport);
#endif
    if (destPort != 5353) {
      return;
    }
    if(my_TTL_Check) {
	if(((u_int8_t) ipHeader->ip_ttl) <= ((u_int8_t) 1)) {   // Discard mdns packet with TTL limit 1 or less
	OLSR_PRINTF(1,"Discarding packet captured with TTL = 1\n");
      	return;
	}
    }

    if (isInFilteredList(&src)) {
      OLSR_PRINTF(1,"Discarding packet captured because of filtered hosts ACL\n");
      return;
    }

  }                             //END IPV4

  else if ((encapsulationUdpData[0] & 0xf0) == 0x60) {  //IPv6

    ipHeader6 = (struct ip6_hdr *)ARM_NOWARN_ALIGN(encapsulationUdpData);

    //TODO: mettere dentro src.v6 l'indirizzo IPv6

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
#if defined(__GLIBC__) || defined(__BIONIC__)
    destPort = ntohs(udpHeader->dest);
#else
    destPort = ntohs(udpHeader->uh_dport);
#endif
    if (destPort != 5353) {
      return;
    }
    if(my_TTL_Check) {
    	if(((uint8_t) ipHeader6->ip6_hops) <= ((uint8_t) 1)) { // Discard mdns packet with hop limit 1 or less
	OLSR_PRINTF(1,"Discarding packet captured with TTL = 1\n");
    	return;
	}
	}
  
    if (isInFilteredList(&src)) {
      OLSR_PRINTF(1,"Discarding packet captured because of filtered hosts ACL\n");
      return;
    }

  }                             //END IPV6
  else
    return;                     //Is not IP packet

  /* Check if the frame is captured on an OLSR-enabled interface */
  //isFromOlsrIntf = (intf->olsrIntf != NULL); TODO: put again this check

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
    struct TBmfInterface *walker;

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

    for(walker = BmfInterfaces; walker != NULL; walker = walker->next){	//if the router isn't the master for this interface
      if(skfd == walker->capturingSkfd && walker->isActive == 0) {	//discard mdns packets
         OLSR_PRINTF(1,"Not capturing mDNS packet from interface %s because this router is not master\n",walker->ifName);
	 return;
      }
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
InitMDNS(struct interface_olsr *skipThisIntf)
{
  //Tells OLSR to launch olsr_parser when the packets for this plugin arrive
  olsr_parser_add_function(&olsr_parser, PARSER_TYPE);
  //Creates captures sockets and register them to the OLSR scheduler
  CreateBmfNetworkInterfaces(skipThisIntf);
  InitRouterList(NULL);

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

void DoElection(int skfd, void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused)))
{
  static const char * rxBufferPrefix = "$REP";
  static const ssize_t rxBufferPrefixLength = 4;
  unsigned char rxBuffer[HELLO_BUFFER_SIZE];
  ssize_t rxCount;
  union olsr_sockaddr sender;
  socklen_t senderSize = sizeof(sender);
  struct RtElHelloPkt *rcvPkt;
  struct RouterListEntry *listEntry;
  struct RouterListEntry6 *listEntry6;

  OLSR_PRINTF(1,"Packet Received \n");

  if (skfd >= 0) {
    memset(&sender, 0, senderSize);
    rxCount = recvfrom(skfd, &rxBuffer[0], (sizeof(rxBuffer) - 1), 0,
		(struct sockaddr *)&sender, &senderSize);
    if(rxCount < 0){
      BmfPError("Receive error in %s, ignoring message.", __func__);
      return;
    }

  /* make sure the string is null terminated */
  rxBuffer[rxCount] = '\0';

  /* do not process when this message doesn't start with $REP */
  if ((rxCount < rxBufferPrefixLength) || (strncmp((char *) rxBuffer,
		  rxBufferPrefix, rxBufferPrefixLength) != 0))
    return;

  if ( rxCount < (int)sizeof(struct RtElHelloPkt))
    return;					// too small to be a hello pkt
  else
    rcvPkt = (struct RtElHelloPkt *)ARM_NOWARN_ALIGN(rxBuffer);

  if (rcvPkt->ipFamily == AF_INET){
    listEntry = (struct RouterListEntry *)malloc(sizeof(struct RouterListEntry));
    if(ParseElectionPacket(rcvPkt, listEntry, skfd)){
      OLSR_PRINTF(1,"processing ipv4 packet \n");
      if(UpdateRouterList(listEntry))
        free(listEntry);
    }
    else{
      free(listEntry);
      return;					//packet not valid
    }
  }
  else{
    listEntry6 = (struct RouterListEntry6 *)malloc(sizeof(struct RouterListEntry6));
    if(ParseElectionPacket6(rcvPkt, listEntry6, skfd)){
      OLSR_PRINTF(1,"processing ipv6 packet");
      if(UpdateRouterList6(listEntry6))
        free(listEntry6);
    }
    else{
      free(listEntry6);
      return;					//packet not valid
    }
  }
  
  }
 return;
}
