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
  zebra.sockpath = olsr_malloc(sizeof (ZEBRA_SOCKPATH), "QUAGGA: New socket path");
  strscpy(zebra.sockpath, ZEBRA_SOCKPATH, sizeof (ZEBRA_SOCKPATH));

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
  route.safi = SAFI_UNICAST;
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
  if(!retval && (zebra.options & OPTION_ROUTE_ADDITIONAL))
    retval = olsr_cnf->ip_version == AF_INET ? zebra.orig_addroute_function(r) : zebra.orig_addroute6_function(r);

  free(route.ifindex);
  free(route.nexthop);

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
  route.safi = SAFI_UNICAST;
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
  if(!retval && (zebra.options & OPTION_ROUTE_ADDITIONAL))
    retval = olsr_cnf->ip_version == AF_INET ? zebra.orig_delroute_function(r) : zebra.orig_delroute6_function(r);

  free(route.ifindex);
  free(route.nexthop);

  return retval;
}

void
zebra_redistribute(uint16_t cmd)
{
  unsigned char type;

  for (type = 0; type < ZEBRA_ROUTE_MAX; type++)
    if (zebra.redistribute[type]) {
      if (zclient_write(zpacket_redistribute(cmd, type)) < 0)
        olsr_exit("QUAGGA: Could not write redistribute packet", EXIT_FAILURE);
    }

}

void
zebra_hello(uint16_t cmd)
{

  if (zclient_write(zpacket_redistribute(cmd, ZEBRA_ROUTE_OLSR)) < 0)
    olsr_exit("QUAGGA: Could not write hello packet", EXIT_FAILURE);

}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
