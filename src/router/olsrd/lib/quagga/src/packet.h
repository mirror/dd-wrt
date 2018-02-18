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
 * File               : packet.h
 * Description        : header file for packet.c
 * ------------------------------------------------------------------------- */

#ifndef _LIB_QUAGGA_PACKET_H_
#define _LIB_QUAGGA_PACKET_H_

/* Zebra packet size */
#define ZEBRA_MAX_PACKET_SIZ		4096

/* Zebra header marker */
#ifndef ZEBRA_HEADER_MARKER
#define ZEBRA_HEADER_MARKER 255
#endif /* ZEBRA_HEADER_MARKER */

/* Zebra message types */
#define ZEBRA_IPV4_ROUTE_ADD		7
#define ZEBRA_IPV4_ROUTE_DELETE		8
#define ZEBRA_IPV6_ROUTE_ADD            9
#define ZEBRA_IPV6_ROUTE_DELETE         10
#define ZEBRA_REDISTRIBUTE_ADD         11
#define ZEBRA_REDISTRIBUTE_DELETE      12
#define ZEBRA_HELLO                    23

/* Zebra nexthop flags */
#define ZEBRA_NEXTHOP_IFINDEX		1
#define ZEBRA_NEXTHOP_IPV4		3
#define ZEBRA_NEXTHOP_IPV6              6

/* Zebra message flags */
#define ZAPI_MESSAGE_NEXTHOP		0x01
#define ZAPI_MESSAGE_IFINDEX		0x02
#define ZAPI_MESSAGE_DISTANCE		0x04
#define ZAPI_MESSAGE_METRIC		0x08

/* Subsequent Address Family Identifier */
#define SAFI_UNICAST                    1

/* Zebra flags */
#define ZEBRA_FLAG_SELECTED		0x10

struct zroute {
  unsigned char type;
  unsigned char flags;
  unsigned char message;
  uint16_t safi;
  unsigned char prefixlen;
  union olsr_ip_addr prefix;
  unsigned char nexthop_num;
  union olsr_ip_addr *nexthop;
  unsigned char ifindex_num;
  uint32_t *ifindex;
  uint32_t metric;
  uint8_t distance;
};

unsigned char *zpacket_route(uint16_t, struct zroute *);
unsigned char *zpacket_redistribute(uint16_t, unsigned char);

#endif /* _LIB_QUAGGA_PACKET_H_ */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
