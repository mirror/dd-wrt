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
 * File               : parse.c
 * Description        : functions to parse received zebra packets
 * ------------------------------------------------------------------------- */

#include "defs.h"
#include "olsr.h"

#include "common.h"
#include "packet.h"
#include "client.h"
#include "parse.h"

static void free_zroute(struct zroute *);
static struct zroute *zparse_route(unsigned char *);

static void
free_zroute(struct zroute *r)
{

  if(r->ifindex_num)
    free(r->ifindex);
  if(r->nexthop_num)
    free(r->nexthop);

}

static struct zroute
*zparse_route(unsigned char *opt)
{
  struct zroute *r;
  int c;
  size_t size;
  uint16_t length;
  unsigned char *pnt;

  memcpy(&length, opt, sizeof length);
  length = ntohs (length);

  r = olsr_malloc(sizeof *r, "QUAGGA: New zebra route");
  pnt = (zebra.version ? &opt[6] : &opt[3]);
  r->type = *pnt++;
  r->flags = *pnt++;
  r->message = *pnt++;
  r->prefixlen = *pnt++;
  size = (r->prefixlen + 7) / 8;
  memset(&r->prefix, 0, sizeof r->prefix);
  if (olsr_cnf->ip_version == AF_INET)
    memcpy(&r->prefix.v4.s_addr, pnt, size);
  else
    memcpy(r->prefix.v6.s6_addr, pnt, size);
  pnt += size;

  switch (zebra.version) {
    case 0:
    case 1:
      if (r->message & ZAPI_MESSAGE_NEXTHOP) {
        r->nexthop_num = *pnt++;
        r->nexthop = olsr_malloc((sizeof *r->nexthop) * r->nexthop_num, "QUAGGA: New zebra route nexthop");
        for (c = 0; c < r->nexthop_num; c++) {
          if (olsr_cnf->ip_version == AF_INET) {
            memcpy(&r->nexthop[c].v4.s_addr, pnt, sizeof r->nexthop[c].v4.s_addr);
            pnt += sizeof r->nexthop[c].v4.s_addr;
          } else {
            memcpy(r->nexthop[c].v6.s6_addr, pnt, sizeof r->nexthop[c].v6.s6_addr);
            pnt += sizeof r->nexthop[c].v6.s6_addr;
          }
        }
      }

      if (r->message & ZAPI_MESSAGE_IFINDEX) {
        r->ifindex_num = *pnt++;
        r->ifindex = olsr_malloc(sizeof(uint32_t) * r->ifindex_num, "QUAGGA: New zebra route ifindex");
        for (c = 0; c < r->ifindex_num; c++) {
          memcpy(&r->ifindex[c], pnt, sizeof r->ifindex[c]);
          r->ifindex[c] = ntohl (r->ifindex[c]);
          pnt += sizeof r->ifindex[c];
        }
      }
      break;
    default:
      OLSR_PRINTF(1, "(QUAGGA) Unsupported zebra packet version!\n");
      break;
  }

  if (r->message & ZAPI_MESSAGE_DISTANCE) {
    r->distance = *pnt++;
  }

// Quagga v0.98.6 BUG workaround: metric is always sent by zebra
// even without ZAPI_MESSAGE_METRIC message.
//  if (r.message & ZAPI_MESSAGE_METRIC) {
    memcpy(&r->metric, pnt, sizeof r->metric);
    r->metric = ntohl(r->metric);
    pnt += sizeof r->metric;
//  }

  if (pnt - opt != length) {
    olsr_exit("(QUAGGA) Length does not match!", EXIT_FAILURE);
  }

  return r;
}

void
zparse(void *foo __attribute__ ((unused)))
{
  unsigned char *data, *f;
  uint16_t command;
  uint16_t length;
  ssize_t len;
  struct zroute *route;

  if (!(zebra.status & STATUS_CONNECTED)) {
    zclient_reconnect();
    return;
  }
  data = zclient_read(&len);
  if (data) {
    f = data;
    do {
      memcpy(&length, f, sizeof length);
      length = ntohs (length);
      if (!length) // something weired happened
        olsr_exit("(QUAGGA) Zero message length!", EXIT_FAILURE);
      if (zebra.version) {
        if ((f[2] != ZEBRA_HEADER_MARKER) || (f[3] != zebra.version))
          olsr_exit("(QUAGGA) Invalid zebra header received!", EXIT_FAILURE);
        memcpy(&command, &f[4], sizeof command);
        command = ntohs (command);
      } else
          command = f[2];
      if (olsr_cnf->ip_version == AF_INET) {
        switch (command) {
          case ZEBRA_IPV4_ROUTE_ADD:
            route = zparse_route(f);
            ip_prefix_list_add(&olsr_cnf->hna_entries, &route->prefix, route->prefixlen);
            free_zroute(route);
            free(route);
            break;
          case ZEBRA_IPV4_ROUTE_DELETE:
            route = zparse_route(f);
            ip_prefix_list_remove(&olsr_cnf->hna_entries, &route->prefix, route->prefixlen);
            free_zroute(route);
            free(route);
            break;
          default:
            break;
        }
      } else {
        switch (command) {
          case ZEBRA_IPV6_ROUTE_ADD:
            route = zparse_route(f);
            ip_prefix_list_add(&olsr_cnf->hna_entries, &route->prefix, route->prefixlen);
            free_zroute(route);
            free(route);
            break;
          case ZEBRA_IPV6_ROUTE_DELETE:
            route = zparse_route(f);
            ip_prefix_list_remove(&olsr_cnf->hna_entries, &route->prefix, route->prefixlen);
            free_zroute(route);
            free(route);
            break;
          default:
            break;
        }
      }

      f += length;
    }
    while ((f - data) < len);
    free(data);
  }

}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
