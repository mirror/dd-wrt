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
 * File               : packet.c
 * Description        : zebra packet creation functions
 * ------------------------------------------------------------------------- */

#include "defs.h"
#include "olsr.h"

#include "common.h"
#include "packet.h"

unsigned char
*zpacket_route(uint16_t cmd, struct zroute *r)
{
  int count;
  uint8_t len;
  uint16_t size;
  uint32_t ind, metric;
  unsigned char *cmdopt, *t;

  cmdopt = olsr_malloc(ZEBRA_MAX_PACKET_SIZ, "QUAGGA: New route packet");

  t = &cmdopt[2];
  if (zebra.version) {
    *t++ = ZEBRA_HEADER_MARKER;
    *t++ = zebra.version;
    cmd = htons(cmd);
    memcpy(t, &cmd, sizeof cmd);
    t += sizeof cmd;
  } else
      *t++ = (unsigned char) cmd;
  *t++ = r->type;
  *t++ = r->flags;
  *t++ = r->message;
  *t++ = r->prefixlen;
  len = (r->prefixlen + 7) / 8;
  if (olsr_cnf->ip_version == AF_INET)
    memcpy(t, &r->prefix.v4.s_addr, len);
  else
    memcpy(t, r->prefix.v6.s6_addr, len);
  t = t + len;

  if (r->message & ZAPI_MESSAGE_NEXTHOP) {
    *t++ = r->nexthop_num + r->ifindex_num;

      for (count = 0; count < r->nexthop_num; count++) {
        if (olsr_cnf->ip_version == AF_INET) {
          *t++ = ZEBRA_NEXTHOP_IPV4;
          memcpy(t, &r->nexthop[count].v4.s_addr, sizeof r->nexthop[count].v4.s_addr);
          t += sizeof r->nexthop[count].v4.s_addr;
        } else {
          *t++ = ZEBRA_NEXTHOP_IPV6;
          memcpy(t, r->nexthop[count].v6.s6_addr, sizeof r->nexthop[count].v6.s6_addr);
          t += sizeof r->nexthop[count].v6.s6_addr;
        }
      }
      for (count = 0; count < r->ifindex_num; count++) {
        *t++ = ZEBRA_NEXTHOP_IFINDEX;
        ind = htonl(r->ifindex[count]);
        memcpy(t, &ind, sizeof ind);
        t += sizeof ind;
      }
  }
  if ((r->message & ZAPI_MESSAGE_DISTANCE) > 0)
    *t++ = r->distance;
  if ((r->message & ZAPI_MESSAGE_METRIC) > 0) {
    metric = htonl(r->metric);
    memcpy(t, &metric, sizeof metric);
    t += sizeof metric;
  }
  size = htons(t - cmdopt);
  memcpy(cmdopt, &size, sizeof size);

  return cmdopt;
}

unsigned char
*zpacket_redistribute (uint16_t cmd, unsigned char type)
{
  unsigned char *data, *pnt;
  uint16_t size;

  data = olsr_malloc(ZEBRA_MAX_PACKET_SIZ , "QUAGGA: New redistribute packet");

  pnt = &data[2];
  if (zebra.version) {
    *pnt++ = ZEBRA_HEADER_MARKER;
    *pnt++ = zebra.version;
    cmd = htons(cmd);
    memcpy(pnt, &cmd, sizeof cmd);
    pnt += sizeof cmd;
  } else
      *pnt++ = (unsigned char) cmd;
  *pnt++ = type;
  size = htons(pnt - data);
  memcpy(data, &size, sizeof size);

  return data;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
