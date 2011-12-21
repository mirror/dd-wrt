/*
 * OLSRd Quagga plugin
 *
 * Copyright (C) 2006-2008 Immo 'FaUl' Wehrenberg <immo@chaostreff-dortmund.de>
 * Copyright (C) 2007-2011 Vasilis Tsiligiannis <acinonyxs@yahoo.gr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation or - at your option - under
 * the terms of the GNU General Public Licence version 2 but can be
 * linked to any BSD-Licenced Software with public available sourcecode
 *
 */

/* -------------------------------------------------------------------------
 * File               : quagga.c
 * Description        : functions to interface zebra with olsrd
 * ------------------------------------------------------------------------- */

#include "defs.h"
#include "olsr.h"
#include "log.h"

#include "common.h"
#include "quagga.h"
#include "packet.h"
#include "client.h"

struct zebra zebra;

void
zebra_init(void)
{

  memset(&zebra, 0, sizeof zebra);
  zebra.sockpath = olsr_malloc(sizeof ZEBRA_SOCKPATH  + 1, "QUAGGA: New socket path");
  strscpy(zebra.sockpath, ZEBRA_SOCKPATH, sizeof ZEBRA_SOCKPATH);

}

void
zebra_fini(void)
{
  struct rt_entry *tmp;

  if (zebra.options & OPTION_EXPORT) {
    OLSR_FOR_ALL_RT_ENTRIES(tmp) {
      zebra_delroute(tmp);
    }
    OLSR_FOR_ALL_RT_ENTRIES_END(tmp);
  }
  zebra_redistribute(ZEBRA_REDISTRIBUTE_DELETE);

}

int
zebra_addroute(const struct rt_entry *r)
{
  struct zroute route;
  int retval;

  route.distance = 0;
  route.type = ZEBRA_ROUTE_OLSR;
  route.flags = zebra.flags;
  route.message = ZAPI_MESSAGE_NEXTHOP | ZAPI_MESSAGE_METRIC;
  route.prefixlen = r->rt_dst.prefix_len;
  if (olsr_cnf->ip_version == AF_INET)
    route.prefix.v4.s_addr = r->rt_dst.prefix.v4.s_addr;
  else
    memcpy(route.prefix.v6.s6_addr, r->rt_dst.prefix.v6.s6_addr, sizeof route.prefix.v6.s6_addr);
  route.ifindex_num = 0;
  route.ifindex = NULL;
  route.nexthop_num = 0;
  route.nexthop = NULL;

  if ((olsr_cnf->ip_version == AF_INET && r->rt_best->rtp_nexthop.gateway.v4.s_addr == r->rt_dst.prefix.v4.s_addr &&
        route.prefixlen == 32) ||
      (olsr_cnf->ip_version == AF_INET6 &&
        !memcmp(r->rt_best->rtp_nexthop.gateway.v6.s6_addr, r->rt_dst.prefix.v6.s6_addr, sizeof r->rt_best->rtp_nexthop.gateway.v6.s6_addr) &&
        route.prefixlen == 128)) {
    route.ifindex_num++;
    route.ifindex = olsr_malloc(sizeof *route.ifindex, "QUAGGA: New zebra route ifindex");
    *route.ifindex = r->rt_best->rtp_nexthop.iif_index;
  } else {
    route.nexthop_num++;
    route.nexthop = olsr_malloc(sizeof *route.nexthop, "QUAGGA: New zebra route nexthop");
    if (olsr_cnf->ip_version == AF_INET)
      route.nexthop->v4.s_addr = r->rt_best->rtp_nexthop.gateway.v4.s_addr;
    else
      memcpy(route.nexthop->v6.s6_addr, r->rt_best->rtp_nexthop.gateway.v6.s6_addr, sizeof route.nexthop->v6.s6_addr);
  }

  route.metric = r->rt_best->rtp_metric.hops;

  if (zebra.distance) {
    route.message |= ZAPI_MESSAGE_DISTANCE;
    route.distance = zebra.distance;
  }

  retval = zclient_write(zpacket_route(olsr_cnf->ip_version == AF_INET ? ZEBRA_IPV4_ROUTE_ADD : ZEBRA_IPV6_ROUTE_ADD, &route));

  return retval;
}

int
zebra_delroute(const struct rt_entry *r)
{
  struct zroute route;
  int retval;

  route.distance = 0;
  route.type = ZEBRA_ROUTE_OLSR;
  route.flags = zebra.flags;
  route.message = ZAPI_MESSAGE_NEXTHOP | ZAPI_MESSAGE_METRIC;
  route.prefixlen = r->rt_dst.prefix_len;
  if (olsr_cnf->ip_version == AF_INET)
    route.prefix.v4.s_addr = r->rt_dst.prefix.v4.s_addr;
  else
    memcpy(route.prefix.v6.s6_addr, r->rt_dst.prefix.v6.s6_addr, sizeof route.prefix.v6.s6_addr);
  route.ifindex_num = 0;
  route.ifindex = NULL;
  route.nexthop_num = 0;
  route.nexthop = NULL;

  if ((olsr_cnf->ip_version == AF_INET && r->rt_nexthop.gateway.v4.s_addr == r->rt_dst.prefix.v4.s_addr &&
        route.prefixlen == 32) ||
       (olsr_cnf->ip_version == AF_INET6 &&
        !memcmp(r->rt_nexthop.gateway.v6.s6_addr, r->rt_dst.prefix.v6.s6_addr, sizeof r->rt_nexthop.gateway.v6.s6_addr) &&
        route.prefixlen == 128)) {
    route.ifindex_num++;
    route.ifindex = olsr_malloc(sizeof *route.ifindex, "QUAGGA: New zebra route ifindex");
    *route.ifindex = r->rt_nexthop.iif_index;
  } else {
    route.nexthop_num++;
    route.nexthop = olsr_malloc(sizeof *route.nexthop, "QUAGGA: New zebra route nexthop");
    if (olsr_cnf->ip_version == AF_INET)
      route.nexthop->v4.s_addr = r->rt_nexthop.gateway.v4.s_addr;
    else
      memcpy(route.nexthop->v6.s6_addr, r->rt_nexthop.gateway.v6.s6_addr, sizeof route.nexthop->v6.s6_addr);
  }

  route.metric = 0;

  if (zebra.distance) {
    route.message |= ZAPI_MESSAGE_DISTANCE;
    route.distance = zebra.distance;
  }

  retval = zclient_write(zpacket_route(olsr_cnf->ip_version == AF_INET ? ZEBRA_IPV4_ROUTE_DELETE : ZEBRA_IPV6_ROUTE_DELETE, &route));

  return retval;
}

void
zebra_redistribute(uint16_t cmd)
{
  unsigned char type;

  for (type = 0; type < ZEBRA_ROUTE_MAX; type++)
    if (zebra.redistribute[type]) {
      if (zclient_write(zpacket_redistribute(cmd, type)) < 0)
        olsr_exit("(QUAGGA) Could not write redistribute packet!", EXIT_FAILURE);
    }

}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
