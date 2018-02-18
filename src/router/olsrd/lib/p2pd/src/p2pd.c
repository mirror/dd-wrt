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


#include "p2pd.h"

/* System includes */
#include <stddef.h>             /* NULL */
#include <sys/types.h>          /* ssize_t */
#include <string.h>             /* strerror() */
#include <stdarg.h>             /* va_list, va_start, va_end */
#include <errno.h>              /* errno */
#include <assert.h>             /* assert() */
#include <unistd.h>
#include <fcntl.h>
#include <linux/if_ether.h>     /* ETH_P_IP */
#include <linux/if_packet.h>    /* struct sockaddr_ll, PACKET_MULTICAST */
#include <signal.h>             /* sigset_t, sigfillset(), sigdelset(), SIGINT */
#include <netinet/ip.h>         /* struct ip */
#include <netinet/udp.h>        /* struct udphdr */
#include <unistd.h>             /* close() */

#include <netinet/in.h>
#include <netinet/ip6.h>

#include <time.h>

/* OLSRD includes */
#include "plugin_util.h"        /* set_plugin_int */
#include "defs.h"               /* olsr_cnf, //OLSR_PRINTF */
#include "ipcalc.h"
#include "olsr.h"               /* //OLSR_PRINTF */
#include "mid_set.h"            /* mid_lookup_main_addr() */
#include "link_set.h"           /* get_best_link_to_neighbor() */
#include "net_olsr.h"           /* ipequal */
#include "parser.h"

/* plugin includes */
#include "NetworkInterfaces.h"  /* NonOlsrInterface,
                                   CreateBmfNetworkInterfaces(),
                                   CloseBmfNetworkInterfaces() */
//#include "Address.h"          /* IsMulticast() */
#include "Packet.h"             /* ENCAP_HDR_LEN,
                                   BMF_ENCAP_TYPE,
                                   BMF_ENCAP_LEN etc. */
#include "PacketHistory.h"
#include "dllist.h"

int P2pdTtl                        = 0;
int P2pdUseHash                    = 0;  /* Switch off hash filter by default */
int P2pdUseTtlDecrement            = 0;  /* No TTL decrement by default */
int P2pdDuplicateTimeout           = P2PD_VALID_TIME;

/* List of UDP destination address and port information */
struct UdpDestPort *                 UdpDestPortList = NULL;

/* List of filter entries to check for duplicate messages
 */
struct node *                        dupFilterHead = NULL;
struct node *                        dupFilterTail = NULL;

bool is_broadcast(const struct sockaddr_in addr);
bool is_multicast(const struct sockaddr_in addr);
char * get_ipv4_str(uint32_t address, char *s, size_t maxlen);
char * get_ipv6_str(unsigned char* address, char *s, size_t maxlen);
#ifdef INCLUDE_DEBUG_OUTPUT
void dump_packet(unsigned char* packet, int length);
#endif /* INCLUDE_DEBUG_OUTPUT */
bool check_and_mark_recent_packet(unsigned char *data, int len);

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
  struct ip *ipHeader;        /* IP header inside the encapsulated IP packet */
  struct ip6_hdr *ip6Header;  /* IP header inside the encapsulated IP packet */
  struct udphdr *udpHeader;
  struct NonOlsrInterface *walker;
  int stripped_len = 0;
  union olsr_ip_addr destAddr;
  int destPort;
  bool isInList = false;

  ipHeader = (struct ip *) ARM_NOWARN_ALIGN(encapsulationUdpData);
  ip6Header = (struct ip6_hdr *) ARM_NOWARN_ALIGN(encapsulationUdpData);
  //OLSR_DEBUG(LOG_PLUGINS, "P2PD PLUGIN got packet from OLSR message\n");

  if (check_and_mark_recent_packet(encapsulationUdpData, len))
    return;

  /* Check with each network interface what needs to be done on it */
  for (walker = nonOlsrInterfaces; walker != NULL; walker = walker->next) {
    /* To a non-OLSR interface: unpack encapsulated IP packet and forward it */
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
        stripped_len = 40 + ntohs(ip6Header->ip6_plen); // IPv6 Header size (40)
                                                        // + payload_len
      }

      // Sven-Ola: Don't know how to handle the "stripped_len is uninitialized"
      // condition, maybe olsr_exit is better...?
      if (0 == stripped_len)
        return;

      //TODO: if packet is not IP die here

      if (stripped_len > len) {
      }

      dest.sll_ifindex = if_nametoindex(walker->ifName);
      dest.sll_halen = IFHWADDRLEN;

      if (olsr_cnf->ip_version == AF_INET) {
        /* Use all-ones as destination MAC address. When the IP destination is
         * a multicast address, the destination MAC address should normally also
         * be a multicast address. E.g., when the destination IP is 224.0.0.1,
         * the destination MAC should be 01:00:5e:00:00:01. However, it does not
         * seem to matter when the destination MAC address is set to all-ones
         * in that case.
         */

        if (IsMulticastv4(ipHeader)) {
          in_addr_t addr = ntohl(ipHeader->ip_dst.s_addr);

          dest.sll_addr[0] = 0x01;
          dest.sll_addr[1] = 0x00;
          dest.sll_addr[2] = 0x5E;
          dest.sll_addr[3] = (addr >> 16) & 0x7F;
          dest.sll_addr[4] = (addr >>  8) & 0xFF;
          dest.sll_addr[5] = addr & 0xFF;
        } else {
          /* broadcast or whatever */
          memset(dest.sll_addr, 0xFF, IFHWADDRLEN);
        }
      } else /*(olsr_cnf->ip_version == AF_INET6) */ {
        if (IsMulticastv6(ip6Header)) {
          dest.sll_addr[0] = 0x33;
          dest.sll_addr[1] = 0x33;
          dest.sll_addr[2] = ip6Header->ip6_dst.s6_addr[12];
          dest.sll_addr[3] = ip6Header->ip6_dst.s6_addr[13];
          dest.sll_addr[4] = ip6Header->ip6_dst.s6_addr[14];
          dest.sll_addr[5] = ip6Header->ip6_dst.s6_addr[15];
        }
      }

      if (olsr_cnf->ip_version == AF_INET) {
        // Determine the IP address and the port from the header information
        if (ipHeader->ip_p == SOL_UDP && !IsIpv4Fragment(ipHeader)) {
          udpHeader = (struct udphdr*) ARM_NOWARN_ALIGN((encapsulationUdpData +
                                       GetIpHeaderLength(encapsulationUdpData)));
          destAddr.v4.s_addr = ipHeader->ip_dst.s_addr;
#if defined(__GLIBC__) || defined(__BIONIC__)
          destPort = htons(udpHeader->dest);
#else
          destPort = htons(udpHeader->uh_dport);
#endif
          isInList = InUdpDestPortList(AF_INET, &destAddr, destPort);
#ifdef INCLUDE_DEBUG_OUTPUT
          if (!isInList) {
            char tmp[32];
            OLSR_PRINTF(1,
                        "%s: Not in dest/port list: %s:%d\n",
                        PLUGIN_NAME_SHORT,
                        get_ipv4_str(destAddr.v4.s_addr,
                                     tmp,
                                     sizeof(tmp)),
                        destPort);
          }
#endif /* INCLUDE_DEBUG_OUTPUT */
        }
      } else /* (olsr_cnf->ip_version == AF_INET6) */ {
        if (ip6Header->ip6_nxt == SOL_UDP && !IsIpv6Fragment(ip6Header)) {
          udpHeader = (struct udphdr*) ARM_NOWARN_ALIGN((encapsulationUdpData + 40));
          memcpy(&destAddr.v6, &ip6Header->ip6_dst, sizeof(struct in6_addr));
#if defined(__GLIBC__) || defined(__BIONIC__)
          destPort = htons(udpHeader->dest);
#else
          destPort = htons(udpHeader->uh_dport);
#endif
          isInList = InUdpDestPortList(AF_INET6, &destAddr, destPort);
#ifdef INCLUDE_DEBUG_OUTPUT
          if (!isInList) {
            char tmp[64];
            OLSR_PRINTF(1,
                        "%s: Not in dest/port list: %s:%d\n",
                        PLUGIN_NAME_SHORT,
                        get_ipv6_str(destAddr.v6.s6_addr,
                                     tmp,
                                     sizeof(tmp)),
                        destPort);
          }
#endif /* INCLUDE_DEBUG_OUTPUT */
        }
      }

      if (!isInList) {
        /* Address/port combination of this packet is not in the UDP dest/port
         * list and will therefore be suppressed. I.e. just continue with the
         * next interface to emit on.
         */
        continue;
      }
      
      nBytesWritten = sendto(walker->capturingSkfd,
                             encapsulationUdpData,
                             stripped_len,
                             0,
                             (struct sockaddr *)&dest,
                             sizeof(dest));
      if (nBytesWritten != stripped_len) {
        P2pdPError("sendto() error forwarding unpacked encapsulated pkt on \"%s\"",
                   walker->ifName);
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

/* Highest-numbered open socket file descriptor. To be used as first
 * parameter in calls to select(...).
 */
int HighestSkfd = -1;

/* Set of socket file descriptors */
fd_set InputSet;

/* -------------------------------------------------------------------------
 * Function   : p2pd_message_seen
 * Description: Check whether the current message has been seen before
 * Input      : head - start of the list to check for the message
 *              tail - end of the list to check for the message
 *              m    - message to check for in the list
 * Output     : none
 * Return     : true if message was found, false otherwise
 * Data Used  : P2pdDuplicateTimeout
 * ------------------------------------------------------------------------- */
bool
p2pd_message_seen(struct node **head, struct node **tail, union olsr_message *m)
{
  struct node * curr;
  time_t now;

  now = time(NULL);

  // Check whether any entries have aged
  curr = *head;
  while (curr) {
    struct DupFilterEntry *filter;
    struct node * next = curr->next; // Save the current pointer since curr may
                                     // be destroyed

    filter = (struct DupFilterEntry*)curr->data;

    if ((filter->creationtime + P2pdDuplicateTimeout) < now)
      remove_node(head, tail, curr, true);

    // Skip to the next element
    curr = next;
  }

  // Now check whether there are any duplicates
  for (curr = *head; curr; curr = curr->next) {
    struct DupFilterEntry *filter = (struct DupFilterEntry*)curr->data;

    if (olsr_cnf->ip_version == AF_INET) {
      if (filter->address.v4.s_addr == m->v4.originator &&
          filter->msgtype == m->v4.olsr_msgtype &&
          filter->seqno == m->v4.seqno) {
          return true;
      }
    } else /* if (olsr_cnf->ip_version == AF_INET6) */ {
      if (memcmp(filter->address.v6.s6_addr,
                 m->v6.originator.s6_addr,
                 sizeof(m->v6.originator.s6_addr)) == 0 &&
          filter->msgtype == m->v6.olsr_msgtype &&
          filter->seqno == m->v6.seqno) {
          return true;
      }
    }
  }

  return false;
}

/* -------------------------------------------------------------------------
 * Function   : p2pd_store_message
 * Description: Store a new message in the duplicate message check list
 * Input      : head - start of the list to add the message to
 *              tail - end of the list to add the message to
 *              m    - message to add to the list
 * Output     : none
 * Return     : nothing
 * Data Used  : none
 * ------------------------------------------------------------------------- */
void
p2pd_store_message(struct node **head, struct node **tail, union olsr_message *m)
{
  time_t now;

  // Store a message into the database
  struct DupFilterEntry *new_dup = calloc(1, sizeof(struct DupFilterEntry));
  if (new_dup == NULL) {
    OLSR_PRINTF(1, "%s: Out of memory\n", PLUGIN_NAME_SHORT);
    return;
  }

  now = time(NULL);

  new_dup->creationtime = now;
  if (olsr_cnf->ip_version == AF_INET) {
    new_dup->address.v4.s_addr = m->v4.originator;
    new_dup->msgtype           = m->v4.olsr_msgtype;
    new_dup->seqno             = m->v4.seqno;
  } else /* if (olsr_cnf->ip_version == AF_INET6) */ {
    memcpy(new_dup->address.v6.s6_addr,
           m->v6.originator.s6_addr,
           sizeof(m->v6.originator.s6_addr));
    new_dup->msgtype           = m->v6.olsr_msgtype;
    new_dup->seqno             = m->v6.seqno;
  }

  // Add the element to the head of the list
  append_node(head, tail, new_dup);
}

/* -------------------------------------------------------------------------
 * Function   : p2pd_is_duplicate_message
 * Description: Check whether the specified message is a duplicate
 * Input      : msg - message to check for in the list of duplicate messages
 * Output     : none
 * Return     : true if message was found, false otherwise
 * Data Used  : none
 * ------------------------------------------------------------------------- */
bool
p2pd_is_duplicate_message(union olsr_message *msg)
{
  if(p2pd_message_seen(&dupFilterHead, &dupFilterTail, msg)) {
    return true;
  }

  p2pd_store_message(&dupFilterHead, &dupFilterTail, msg);

  return false;
}

/* -------------------------------------------------------------------------
 * Function   : olsr_parser
 * Description: Function to be passed to the parser engine. This function
 *              processes the incoming message and passes it on if necessary.
 * Input      : m      - message to parse
 *              in_if  - interface to use (unused in this application)
 *              ipaddr - IP-address to use (unused in this application)
 * Output     : none
 * Return     : false if message should be supressed, true otherwise
 * Data Used  : none
 * ------------------------------------------------------------------------- */
bool
olsr_parser(union olsr_message *m,
            struct interface_olsr *in_if __attribute__ ((unused)),
            union olsr_ip_addr *ipaddr __attribute__ ((unused)))
{
  union olsr_ip_addr originator;
  int size;

  //OLSR_DEBUG(LOG_PLUGINS, "P2PD PLUGIN: Received msg in parser\n");

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
    return false;          /* Don't forward either */

  /* Check for duplicate messages for processing */
  if (p2pd_is_duplicate_message(m))
    return true;  /* Don't process but allow to be forwarded */

  if (olsr_cnf->ip_version == AF_INET) {
    PacketReceivedFromOLSR((unsigned char *)&m->v4.message, size - 12);
  } else {
    PacketReceivedFromOLSR((unsigned char *)&m->v6.message, size - 12 - 96);
  }

	return true;
}

/* -------------------------------------------------------------------------
 * Function   : olsr_p2pd_gen
 * Description: Sends a packet in the OLSR network
 * Input      : packet - packet to send in the OLSR network
 *              len    - length of the packet to send
 * Output     : none
 * Return     : nothing
 * Data Used  : none
 * ------------------------------------------------------------------------- */
void
olsr_p2pd_gen(unsigned char *packet, int len)
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
    message->v4.olsr_msgtype  = P2PD_MESSAGE_TYPE;
    message->v4.olsr_vtime    = reltime_to_me(P2PD_VALID_TIME * MSEC_PER_SEC);
    memcpy(&message->v4.originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
    message->v4.ttl           = P2pdTtl ? P2pdTtl : MAX_TTL;
    message->v4.hopcnt        = 0;
    message->v4.seqno         = htons(get_msg_seqno());
    message->v4.olsr_msgsize  = htons(aligned_size + 12);
    memset(&message->v4.message, 0, aligned_size);
    memcpy(&message->v4.message, packet, len);
    aligned_size = aligned_size + 12;
  } else /* if (olsr_cnf->ip_version == AF_INET6) */ {
    /* IPv6 */
    message->v6.olsr_msgtype  = P2PD_MESSAGE_TYPE;
    message->v6.olsr_vtime    = reltime_to_me(P2PD_VALID_TIME * MSEC_PER_SEC);
    memcpy(&message->v6.originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
    message->v6.ttl           = P2pdTtl ? P2pdTtl : MAX_TTL;
    message->v6.hopcnt        = 0;
    message->v6.seqno         = htons(get_msg_seqno());
    message->v6.olsr_msgsize  = htons(aligned_size + 12 + 96);
    memset(&message->v6.message, 0, aligned_size);
    memcpy(&message->v6.message, packet, len);
    aligned_size = aligned_size + 12 + 96;
  }

  /* looping through interfaces */
  for (ifn = ifnet; ifn; ifn = ifn->int_next) {
    //OLSR_PRINTF(1, "%s: Generating packet - [%s]\n", PLUGIN_NAME_SHORT, ifn->int_name);

    if (net_outbuffer_push(ifn, message, aligned_size) != aligned_size) {
      /* send data and try again */
      net_output(ifn);
      if (net_outbuffer_push(ifn, message, aligned_size) != aligned_size) {
        //OLSR_PRINTF(1, "%s: could not send on interface: %s\n", PLUGIN_NAME_SHORT, ifn->int_name);
      }
    }
  }
}

/* -------------------------------------------------------------------------
 * Function   : P2pdPError
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
P2pdPError(const char *format, ...)
{
#define MAX_STR_DESC 255
  char strDesc[MAX_STR_DESC];

#if !defined REMOVE_LOG_DEBUG
  char *stringErr = strerror(errno);
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

#if !defined REMOVE_LOG_DEBUG
    OLSR_DEBUG(LOG_PLUGINS, "%s: %s\n", strDesc, stringErr);
#endif /* !defined REMOVE_LOG_DEBUG */
  }
}                               /* P2pdPError */

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
 * Function   : InUdpDestPortList
 * Description: Check whether the specified address and port is in the list of
 *              configured UDP destination/port entries
 * Input      : ip_version  - IP version to use for this check
 *              addr        - address to check for in the list
 *              port        - port to check for in the list
 * Output     : none
 * Return     : true if destination/port combination was found, false otherwise
 * Data Used  : UdpDestPortList
 * ------------------------------------------------------------------------- */
bool
InUdpDestPortList(int ip_version, union olsr_ip_addr *addr, uint16_t port)
{
  struct UdpDestPort *walker;

  for (walker = UdpDestPortList; walker; walker = walker->next) {
    if (walker->ip_version == ip_version) {
      if (ip_version == AF_INET) {
        if (addr->v4.s_addr == walker->address.v4.s_addr &&
            walker->port == port)
          return true;  // Found so we can stop here
      } else /* ip_version == AF_INET6 */ {
        if ((memcmp(addr->v6.s6_addr,
                   walker->address.v6.s6_addr,
                   sizeof(addr->v6.s6_addr)) == 0) &&
            (walker->port == port))
          return true;  // Found so we can stop here
      }
    }
  }
  return false;
}

/*
 * Function for checksum calculation.
 * From the RFC, the checksum algorithm is:
 *   "The checksum field is the 16 bit one's complement of the one's
 *   complement sum of all 16 bit words in the header. For purposes of
 *   computing the checksum, the value of the checksum field is zero."
 *
 * For example, consider Hex 4500003044224000800600008c7c19acae241e2b (20 bytes IP header):
 * - Step 1) 4500 + 0030 + 4422 + 4000 + 8006 + 0000 + 8c7c + 19ac + ae24 + 1e2b = 0002`BBCF (16-bit sum)
 * - Step 2) 0002 + BBCF = BBD1 = 1011101111010001 (1's complement 16-bit sum)
 * - Step 3) ~BBD1 = 0100010000101110 = 442E (1's complement of 1's complement 16-bit sum)
 */
static void recomputeIPv4HeaderChecksum(struct ip *header) {
  uint32_t sum;
  uint32_t nwords;
  u_short *headerWords;

  if (!header) {
    return;
  }

  header->ip_sum = 0;
  nwords = header->ip_hl << 1;
  headerWords = (u_short *) header;

  /* step 1 */
  for (sum = 0; nwords > 0; nwords--) {
    sum += ntohs(*headerWords);
    headerWords++;
  }

  /* step 2 */
  sum = (sum >> 16) + (sum & 0xffff);

  /* step 3 */
  sum = ~sum & 0xffff;

  header->ip_sum = htons((u_short)sum);
}

/* -------------------------------------------------------------------------
 * Function   : P2pdPacketCaptured
 * Description: Handle a captured IP packet
 * Input      : encapsulationUdpData - space for the encapsulation header,
 *              followed by the captured IP packet
 *              nBytes - The number of bytes in the data packet
 * Output     : none
 * Return     : none
 * Data Used  : P2pdInterfaces
 * Notes      : The IP packet is assumed to be captured on a socket of family
 *              PF_PACKET and type SOCK_DGRAM (cooked).
 * ------------------------------------------------------------------------- */
static void
P2pdPacketCaptured(unsigned char *encapsulationUdpData, int nBytes)
{
  union olsr_ip_addr dst;      /* Destination IP address in captured packet */
  struct ip *ipHeader = NULL;  /* The IP header inside the captured IP packet */
  struct ip6_hdr *ipHeader6;   /* The IP header inside the captured IP packet */
  struct udphdr *udpHeader;
  uint8_t * ttl = NULL;
  int recomputeChecksum = 0;
  u_int16_t destPort;

  if ((encapsulationUdpData[0] & 0xf0) == 0x40) {       //IPV4

    ipHeader = (struct ip *) ARM_NOWARN_ALIGN(encapsulationUdpData);

    dst.v4 = ipHeader->ip_dst;

    if (ipHeader->ip_p != SOL_UDP) {
      /* Not UDP */
#ifdef INCLUDE_DEBUG_OUTPUT
      OLSR_PRINTF(1,"%s: NON UDP PACKET\n", PLUGIN_NAME_SHORT);
#endif /* INCLUDE_DEBUG_OUTPUT */
      return;                   /* for */
    }

    // If we're dealing with a fragment we bail out here since there's no valid
    // UDP header in this message
    if (IsIpv4Fragment(ipHeader)) {
#ifdef INCLUDE_DEBUG_OUTPUT
      OLSR_PRINTF(1, "%s: Is IPv4 fragment\n", PLUGIN_NAME_SHORT);
#endif /* INCLUDE_DEBUG_OUTPUT */
      return;
    }

    if (check_and_mark_recent_packet(encapsulationUdpData, nBytes))
      return;

    udpHeader = (struct udphdr *) ARM_NOWARN_ALIGN((encapsulationUdpData +
                                  GetIpHeaderLength(encapsulationUdpData)));
#if defined(__GLIBC__) || defined(__BIONIC__)
    destPort = ntohs(udpHeader->dest);
#else
    destPort = ntohs(udpHeader->uh_dport);
#endif

    if (!InUdpDestPortList(AF_INET, &dst, destPort)) {
#ifdef INCLUDE_DEBUG_OUTPUT
      char tmp[32];
      OLSR_PRINTF(1, "%s: Not in dest/port list: %s:%d\n", PLUGIN_NAME_SHORT,
                  get_ipv4_str(dst.v4.s_addr, tmp, sizeof(tmp)), destPort);
#endif /* INCLUDE_DEBUG_OUTPUT */
       return;
    }

    ttl = &ipHeader->ip_ttl;
    recomputeChecksum = 1;
  }                            //END IPV4
  else if ((encapsulationUdpData[0] & 0xf0) == 0x60) {  //IPv6

    ipHeader6 = (struct ip6_hdr *) ARM_NOWARN_ALIGN(encapsulationUdpData);

    memcpy(&dst.v6, &ipHeader6->ip6_dst, sizeof(struct in6_addr));

    if (ipHeader6->ip6_dst.s6_addr[0] == 0xff)  //Multicast
    {
      //Continue
    } else {
      return;                   //not multicast
    }
    if (ipHeader6->ip6_nxt != SOL_UDP) {
      /* Not UDP */
      //OLSR_PRINTF(1,"%s: NON UDP PACKET\n", PLUGIN_NAME_SHORT);
      return;                   /* for */
    }

    // Check whether this is a IPv6 fragment
    if (IsIpv6Fragment(ipHeader6)) {
#ifdef INCLUDE_DEBUG_OUTPUT
      OLSR_PRINTF(1, "%s: Is IPv6 fragment\n", PLUGIN_NAME_SHORT);
#endif /* INCLUDE_DEBUG_OUTPUT */
      return;
    }

    if (check_and_mark_recent_packet(encapsulationUdpData, nBytes))
      return;

    udpHeader = (struct udphdr *) ARM_NOWARN_ALIGN((encapsulationUdpData + 40));
#if defined(__GLIBC__) || defined(__BIONIC__)
    destPort = ntohs(udpHeader->dest);
#else
    destPort = ntohs(udpHeader->uh_dport);
#endif

    if (!InUdpDestPortList(AF_INET6, &dst, destPort)) {
#ifdef INCLUDE_DEBUG_OUTPUT
      char tmp[64];
      OLSR_PRINTF(1, "%s: Not in dest/port list: %s:%d\n", PLUGIN_NAME_SHORT,
                  get_ipv6_str(dst.v6.s6_addr, tmp, sizeof(tmp)), destPort);
#endif /* INCLUDE_DEBUG_OUTPUT */
      return;
    }

    ttl = &ipHeader6->ip6_ctlun.ip6_un1.ip6_un1_hlim;
    recomputeChecksum = 0;
  }                             //END IPV6
  else {
    return;                     //Is not IP packet
  }

  if (P2pdUseTtlDecrement) {
    assert(ttl);
    if (!*ttl) {
      return;
    }
    *ttl -= 1;
    if (!*ttl) {
      return;
    }

    if (recomputeChecksum) {
      recomputeIPv4HeaderChecksum(ipHeader);
    }
  }

  // send the packet to OLSR forward mechanism
  olsr_p2pd_gen(encapsulationUdpData, nBytes);
}                               /* P2pdPacketCaptured */


/* -------------------------------------------------------------------------
 * Function   : DoP2pd
 * Description: This function is registered with the OLSR scheduler and called
 *              when something is captured
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  :
 * ------------------------------------------------------------------------- */
void
DoP2pd(int skfd,
       void *data __attribute__ ((unused)),
       unsigned int flags __attribute__ ((unused)))
{
  unsigned char rxBuffer[P2PD_BUFFER_SIZE];
  if (skfd >= 0) {
    struct sockaddr_ll pktAddr;
    socklen_t addrLen = sizeof(pktAddr);
    int nBytes;
    unsigned char *ipPacket;

    /* Receive the captured Ethernet frame, leaving space for the BMF
     * encapsulation header */
    ipPacket = GetIpPacket(rxBuffer);
    nBytes = recvfrom(skfd, ipPacket, P2PD_BUFFER_SIZE,
                      0, (struct sockaddr *)&pktAddr, &addrLen);
#ifdef INCLUDE_DEBUG_OUTPUT
    OLSR_PRINTF(1, "%s: Received %d bytes\n", PLUGIN_NAME_SHORT, nBytes);
#endif /* INCLUDE_DEBUG_OUTPUT */

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
      //              PLUGIN_NAME_SHORT,
      //              nBytes,
      //              walker->ifName);

      return;                   /* for */
    }

    if (pktAddr.sll_pkttype == PACKET_OUTGOING ||
        pktAddr.sll_pkttype == PACKET_MULTICAST ||
        pktAddr.sll_pkttype == PACKET_BROADCAST) {
#ifdef INCLUDE_DEBUG_OUTPUT
      OLSR_PRINTF(1, "%s: Multicast or broadcast packet was captured.\n",
                  PLUGIN_NAME_SHORT);
      dump_packet(ipPacket, nBytes);
#endif /* INCLUDE_DEBUG_OUTPUT */
      /* A multicast or broadcast packet was captured */
      P2pdPacketCaptured(ipPacket, nBytes);

    }                           /* if (pktAddr.sll_pkttype == ...) */
  }                             /* if (skfd >= 0 && (FD_ISSET...)) */
}                               /* DoP2pd */

/* -------------------------------------------------------------------------
 * Function   : InitP2pd
 * Description: Initialize the P2pd plugin
 * Input      : skipThisInterface - pointer to interface to skip
 * Output     : none
 * Return     : Always 0
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int
InitP2pd(struct interface_olsr *skipThisIntf)
{
  if (P2pdUseHash) {
    // Initialize hash table for hash based duplicate IP packet check
    InitPacketHistory();
  }

  //Tells OLSR to launch olsr_parser when the packets for this plugin arrive
  //olsr_parser_add_function(&olsr_parser, PARSER_TYPE,1);
  olsr_parser_add_function(&olsr_parser, PARSER_TYPE);

  //Creates captures sockets and register them to the OLSR scheduler
  CreateNonOlsrNetworkInterfaces(skipThisIntf);

  return 0;
}                               /* InitP2pd */

/* -------------------------------------------------------------------------
 * Function   : CloseP2pd
 * Description: Close the P2pd plugin and clean up
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  :
 * ------------------------------------------------------------------------- */
void
CloseP2pd(void)
{
  CloseNonOlsrNetworkInterfaces();
}

/* -------------------------------------------------------------------------
 * Function   : SetP2pdTtl
 * Description: Set the TTL for message from this plugin
 * Input      : value - parameter value to evaluate
 * Output     : none
 * Return     : Always 0
 * Data Used  : P2pdTtl
 * ------------------------------------------------------------------------- */
int
SetP2pdTtl(const char *value,
           void *data __attribute__ ((unused)),
           set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  assert(value != NULL);
  P2pdTtl = atoi(value);

  return 0;
}

/* -------------------------------------------------------------------------
 * Function   : SetP2pdUseHashFilter
 * Description: Set the Hash filter flag for this plug-in
 * Input      : value - parameter value to evaluate
 *              data  - data associated with this parameter (unused in this app)
 *              addon - additional parameter data
 * Output     : none
 * Return     : Always 0
 * Data Used  : P2pdUseHash
 * ------------------------------------------------------------------------- */
int
SetP2pdUseHashFilter(const char *value,
                     void *data __attribute__ ((unused)),
                     set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  assert(value != NULL);
  P2pdUseHash = atoi(value);
  
  return 0;
}

/* -------------------------------------------------------------------------
 * Function   : SetP2pdUseTtlDecrement
 * Description: Set the TTL decrement lag for this plug-in
 * Input      : value - parameter value to evaluate
 *              data  - data associated with this parameter (unused in this app)
 *              addon - additional parameter data
 * Output     : none
 * Return     : Always 0
 * Data Used  : P2pdUseTtlDecrement
 * ------------------------------------------------------------------------- */
int
SetP2pdUseTtlDecrement(const char *value,
                     void *data __attribute__ ((unused)),
                     set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  assert(value != NULL);
  P2pdUseTtlDecrement = atoi(value);

  return 0;
}

/* -------------------------------------------------------------------------
 * Function   : AddUdpDestPort
 * Description: Set the UDP destination/port combination as an entry in the
 *              UdpDestPortList
 * Input      : value - parameter value to evaluate
 * Output     : none
 * Return     : -1 on error condition, 0 if all is ok
 * Data Used  : UdpDestPortList
 * ------------------------------------------------------------------------- */
int
AddUdpDestPort(const char *value,
               void *data __attribute__ ((unused)),
               set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  char destAddr[INET6_ADDRSTRLEN];
  uint16_t destPort;
  int num;
  struct UdpDestPort *    new;
  struct sockaddr_in      addr4;
  struct sockaddr_in6     addr6;
  int                     ip_version	= AF_INET;
  int                     res;

  assert(value != NULL);

  // Retrieve the data from the argument string passed
  memset(destAddr, 0, sizeof(destAddr));
  num = sscanf(value, "%45s %hd", destAddr, &destPort);
  if (num != 2) {
    OLSR_PRINTF(1, "%s: Invalid argument for \"UdpDestPort\"",
                PLUGIN_NAME_SHORT);
    return -1;
  }

  // Check whether we're dealing with an IPv4 or IPv6 address
  // When the string contains a ':' we can assume we're dealing with IPv6
  if (strchr(destAddr, (int)':')) {
    ip_version = AF_INET6;
  }

  // Check whether the specified address was either IPv4 multicast,
  // IPv4 broadcast or IPv6 multicast.

  switch (ip_version) {
  case AF_INET6:
    res = inet_pton(AF_INET6, destAddr, &addr6.sin6_addr);
    if (addr6.sin6_addr.s6_addr[0] != 0xFF) {
      OLSR_PRINTF(1,"WARNING: IPv6 address must be multicast... ");
      return -1;
    }
    break;
  default:
    res = inet_pton(AF_INET, destAddr, &addr4.sin_addr);
    if (!is_broadcast(addr4) && !is_multicast(addr4)) {
      OLSR_PRINTF(1,"WARNING: IPv4 address must be multicast or broadcast... ");
    }
    break;
  }
  // Determine whether it is a valid IP address
  if (res == 0) {
    OLSR_PRINTF(1, "Invalid address specified for \"UdpDestPort\"");
    return -1;
  }

  // Create a new entry and link it into the chain
  new = calloc(1, sizeof(struct UdpDestPort));
  if (new == NULL) {
    OLSR_PRINTF(1, "%s: Out of memory", PLUGIN_NAME_SHORT);
    return -1;
  }

  new->ip_version = ip_version;
  switch (ip_version) {
  case AF_INET6:
    memcpy(&new->address.v6.s6_addr,
           &addr6.sin6_addr.s6_addr,
           sizeof(addr6.sin6_addr.s6_addr));
    break;
  default:
    new->address.v4.s_addr = addr4.sin_addr.s_addr;
    break;
  }
  new->port = destPort;
  new->next = UdpDestPortList;
  UdpDestPortList = new;

  // And then we're done
  return 0;
}

/* -------------------------------------------------------------------------
 * Function   : get_ipv4_str
 * Description: Convert the specified address to an IPv4 compatible string
 * Input      : address - IPv4 address to convert to string
 *              s       - string buffer to contain the resulting string
 *              maxlen  - maximum length of the string buffer
 * Output     : none
 * Return     : Pointer to the string buffer containing the result
 * Data Used  : none
 * ------------------------------------------------------------------------- */
char *
get_ipv4_str(uint32_t address, char *s, size_t maxlen)
{
  struct sockaddr_in v4;

  v4.sin_addr.s_addr = address;
  inet_ntop(AF_INET, &v4.sin_addr, s, maxlen);

  return s;
}

/* -------------------------------------------------------------------------
 * Function   : get_ipv6_str
 * Description: Convert the specified address to an IPv4 compatible string
 * Input      : address - IPv6 address to convert to string
 *              s       - string buffer to contain the resulting string
 *              maxlen  - maximum length of the string buffer
 * Output     : none
 * Return     : Pointer to the string buffer containing the result
 * Data Used  : none
 * ------------------------------------------------------------------------- */
char *
get_ipv6_str(unsigned char* address, char *s, size_t maxlen)
{
  struct sockaddr_in6 v6;

  memcpy(v6.sin6_addr.s6_addr, address, sizeof(v6.sin6_addr.s6_addr));
  inet_ntop(AF_INET6, &v6.sin6_addr, s, maxlen);

  return s;
}

/* -------------------------------------------------------------------------
 * Function   : is_broadcast
 * Description: Check whether the address represents a broadcast address
 * Input      : addr - IPv4 address to check
 * Output     : none
 * Return     : true if broadcast address, false otherwise
 * Data Used  : none
 * ------------------------------------------------------------------------- */
bool
is_broadcast(const struct sockaddr_in addr)
{
  if (addr.sin_addr.s_addr == 0xFFFFFFFF)
    return true;

  return false;
}

/* -------------------------------------------------------------------------
 * Function   : is_multicast
 * Description: Check whether the address represents a multicast address
 * Input      : addr - IPv4 address to check
 * Output     : none
 * Return     : true if broadcast address, false otherwise
 * Data Used  : none
 * ------------------------------------------------------------------------- */
bool
is_multicast(const struct sockaddr_in addr)
{
  if ((htonl(addr.sin_addr.s_addr) & 0xE0000000) == 0xE0000000)
    return true;

  return false;
}

#ifdef INCLUDE_DEBUG_OUTPUT
/* -------------------------------------------------------------------------
 * Function   : dump_packet
 * Description: Dump the specified data as hex output
 * Input      : packet - packet to dump to output
 *              length - length of the data in the packet
 * Output     : none
 * Return     : nothing
 * Data Used  : none
 * ------------------------------------------------------------------------- */
void
dump_packet(unsigned char* packet, int length)
{
  int idx;

  OLSR_PRINTF(1, "%s: ", PLUGIN_NAME_SHORT);
  for (idx = 0; idx < length; idx++) {
    if (idx > 0 && ((idx % 16) == 0))
      OLSR_PRINTF(1, "\n%s: ", PLUGIN_NAME_SHORT);
    OLSR_PRINTF(1, "%2.2X ", packet[idx]);
  }
  OLSR_PRINTF(1, "\n");
}
#endif /* INCLUDE_DEBUG_OUTPUT */

/* -------------------------------------------------------------------------
 * Function   : check_and_mark_recent_packet
 * Description: Wrapper function for the Hash based duplicate check
 * Input      : data - pointer to a packet of data to be checked
 * Output     : none
 * Return     : true if duplicate packet, false otherwise
 * Data Used  : P2pdUseHash
 * ------------------------------------------------------------------------- */
bool
check_and_mark_recent_packet(unsigned char *data,
                             int len __attribute__ ((unused)))
{
  unsigned char * ipPacket;
  uint16_t ipPacketLen;
  uint32_t crc32;

  /* If we don't use this filter bail out here */
  if (!P2pdUseHash)
    return false;
    
  /* Clean up the hash table each time before we check it */
  PrunePacketHistory(NULL);

  /* Check for duplicate IP packets now based on a hash */
  ipPacket = GetIpPacket(data);
  ipPacketLen = GetIpTotalLength(ipPacket);

  /* Calculate packet fingerprint */
  crc32 = PacketCrc32(ipPacket, ipPacketLen);

  /* Check if this packet was seen recently */
  if (CheckAndMarkRecentPacket(crc32))
  {
    OLSR_PRINTF(
      8,
      "%s: --> discarding: packet is duplicate\n",
      PLUGIN_NAME_SHORT);
    return true;
  }

  return false;
}
