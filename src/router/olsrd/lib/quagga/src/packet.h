/*
 * OLSRd Quagga plugin
 *
 * Copyright (C) 2006-2008 Immo 'FaUl' Wehrenberg <immo@chaostreff-dortmund.de>
 * Copyright (C) 2007-2010 Vasilis Tsiligiannis <acinonyxs@yahoo.gr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation or - at your option - under
 * the terms of the GNU General Public Licence version 2 but can be
 * linked to any BSD-Licenced Software with public available sourcecode
 *
 */

/* -------------------------------------------------------------------------
 * File               : packet.h
 * Description        : header file for packet.c
 * ------------------------------------------------------------------------- */

/* Zebra packet size */
#define ZEBRA_MAX_PACKET_SIZ		4096

/* Zebra header marker */
#ifndef ZEBRA_HEADER_MARKER
#define ZEBRA_HEADER_MARKER 255
#endif

/* Zebra message types */
#define ZEBRA_IPV4_ROUTE_ADD		7
#define ZEBRA_IPV4_ROUTE_DELETE		8
#define ZEBRA_IPV6_ROUTE_ADD            9
#define ZEBRA_IPV6_ROUTE_DELETE         10
#define ZEBRA_REDISTRIBUTE_ADD         11
#define ZEBRA_REDISTRIBUTE_DELETE      12

/* Zebra nexthop flags */
#define ZEBRA_NEXTHOP_IFINDEX		1
#define ZEBRA_NEXTHOP_IPV4		3
#define ZEBRA_NEXTHOP_IPV6              6

/* Zebra message flags */
#define ZAPI_MESSAGE_NEXTHOP		0x01
#define ZAPI_MESSAGE_IFINDEX		0x02
#define ZAPI_MESSAGE_DISTANCE		0x04
#define ZAPI_MESSAGE_METRIC		0x08

/* Zebra flags */
#define ZEBRA_FLAG_SELECTED		0x10

struct zroute {
  unsigned char type;
  unsigned char flags;
  unsigned char message;
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

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
