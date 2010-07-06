/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
 * Copyright (c) 2007, Sven-Ola for the policy routing stuff
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

#include "defs.h"
#include "kernel_routes.h"
#include "ipc_frontend.h"
#include "log.h"
#include "net_os.h"
#include "process_routes.h"

#include <assert.h>
#include <linux/types.h>
#include <linux/rtnetlink.h>

/*
 * This file contains the old ioctl version of the linux routing code.
 * You will find the current netlink version in kernel_routes_nl.c
 *
 * You can deactivate this code (and activating the netlink one)
 * by adding a -DLINUX_NETLINK_ROUTING to CPPFLAGS in make/Makefile.linux
 */
#ifndef LINUX_NETLINK_ROUTING

static int
delete_all_inet_gws(void)
{
  int s;
  char buf[BUFSIZ], *cp, *cplim;
  struct ifconf ifc;
  struct ifreq *ifr;

  OLSR_PRINTF(1, "Internet gateway detected...\nTrying to delete default gateways\n");

  /* Get a socket */
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    olsr_syslog(OLSR_LOG_ERR, "socket: %m");
    return -1;
  }

  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) {
    olsr_syslog(OLSR_LOG_ERR, "ioctl (get interface configuration)");
    close(s);
    return -1;
  }

  ifr = ifc.ifc_req;
  cplim = buf + ifc.ifc_len;    /*skip over if's with big ifr_addr's */
  for (cp = buf; cp < cplim; cp += sizeof(ifr->ifr_name) + sizeof(ifr->ifr_addr)) {
    struct rtentry kernel_route;
    ifr = (struct ifreq *)cp;

    if (strcmp(ifr->ifr_ifrn.ifrn_name, "lo") == 0) {
      OLSR_PRINTF(1, "Skipping loopback...\n");
      continue;
    }

    OLSR_PRINTF(1, "Trying 0.0.0.0/0 %s...", ifr->ifr_ifrn.ifrn_name);

    memset(&kernel_route, 0, sizeof(struct rtentry));

    ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_addr.s_addr = 0;
    ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_family = AF_INET;
    ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_addr.s_addr = 0;
    ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_family = AF_INET;

    ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_addr.s_addr = INADDR_ANY;
    ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_family = AF_INET;

    kernel_route.rt_flags = RTF_UP | RTF_GATEWAY;

    kernel_route.rt_dev = ifr->ifr_ifrn.ifrn_name;

    if ((ioctl(s, SIOCDELRT, &kernel_route)) < 0)
      OLSR_PRINTF(1, "NO\n");
    else
      OLSR_PRINTF(1, "YES\n");
  }
  close(s);
  return 0;
}

/**
 * Insert a route in the kernel routing table
 *
 * @param destination the route to add
 *
 * @return negative on error
 */
int
olsr_ioctl_add_route(const struct rt_entry *rt)
{
  char if_name[IFNAMSIZ];
  struct rtentry kernel_route;
  union olsr_ip_addr mask;
  int rslt;

  OLSR_PRINTF(2, "KERN: Adding %s\n", olsr_rtp_to_string(rt->rt_best));

  memset(&kernel_route, 0, sizeof(struct rtentry));

  ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_family = AF_INET;
  ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_family = AF_INET;
  ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_family = AF_INET;

  ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_addr = rt->rt_dst.prefix.v4;

  if (!olsr_prefix_to_netmask(&mask, rt->rt_dst.prefix_len)) {
    return -1;
  }
  ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_addr = mask.v4;

  if (rt->rt_dst.prefix.v4.s_addr != rt->rt_best->rtp_nexthop.gateway.v4.s_addr) {
    ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_addr = rt->rt_best->rtp_nexthop.gateway.v4;
  }

  kernel_route.rt_flags = olsr_rt_flags(rt);
  kernel_route.rt_metric = olsr_fib_metric(&rt->rt_best->rtp_metric);

  /*
   * Set interface
   */
  strcpy(if_name, if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index));
  kernel_route.rt_dev = if_name;

  /* delete existing default route before ? */
  if ((olsr_cnf->del_gws) && (rt->rt_dst.prefix.v4.s_addr == INADDR_ANY) && (rt->rt_dst.prefix_len == INADDR_ANY)) {
    delete_all_inet_gws();
    olsr_cnf->del_gws = false;
  }

  if ((rslt = ioctl(olsr_cnf->ioctl_s, SIOCADDRT, &kernel_route)) >= 0) {

    /*
     * Send IPC route update message
     */
    ipc_route_send_rtentry(&rt->rt_dst.prefix, &rt->rt_best->rtp_nexthop.gateway, rt->rt_best->rtp_metric.hops, 1,
                           if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index));
  }

  return rslt;
}

/**
 *Insert a route in the kernel routing table
 *
 *@param destination the route to add
 *
 *@return negative on error
 */
int
olsr_ioctl_add_route6(const struct rt_entry *rt)
{
  struct in6_rtmsg kernel_route;
  int rslt;

  OLSR_PRINTF(2, "KERN: Adding %s\n", olsr_rtp_to_string(rt->rt_best));

  memset(&kernel_route, 0, sizeof(struct in6_rtmsg));

  kernel_route.rtmsg_dst = rt->rt_dst.prefix.v6;
  kernel_route.rtmsg_dst_len = rt->rt_dst.prefix_len;

  kernel_route.rtmsg_gateway = rt->rt_best->rtp_nexthop.gateway.v6;

  kernel_route.rtmsg_flags = olsr_rt_flags(rt);
  kernel_route.rtmsg_metric = olsr_fib_metric(&rt->rt_best->rtp_metric);

  /*
   * set interface
   */
  kernel_route.rtmsg_ifindex = rt->rt_best->rtp_nexthop.iif_index;

  /* XXX delete 0/0 route before ? */

  if ((rslt = ioctl(olsr_cnf->ioctl_s, SIOCADDRT, &kernel_route)) >= 0) {

    /*
     * Send IPC route update message
     */
    ipc_route_send_rtentry(&rt->rt_dst.prefix, &rt->rt_best->rtp_nexthop.gateway, rt->rt_best->rtp_metric.hops, 1,
                           if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index));
  }
  return rslt;
}

/**
 *Remove a route from the kernel
 *
 *@param destination the route to remove
 *
 *@return negative on error
 */
int
olsr_ioctl_del_route(const struct rt_entry *rt)
{
  struct rtentry kernel_route;
  union olsr_ip_addr mask;
  int rslt;

  OLSR_PRINTF(2, "KERN: Deleting %s\n", olsr_rt_to_string(rt));

  memset(&kernel_route, 0, sizeof(struct rtentry));

  ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_family = AF_INET;
  ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_family = AF_INET;
  ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_family = AF_INET;

  ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_addr = rt->rt_dst.prefix.v4;

  if (rt->rt_dst.prefix.v4.s_addr != rt->rt_nexthop.gateway.v4.s_addr) {
    ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_addr = rt->rt_nexthop.gateway.v4;
  }

  if (!olsr_prefix_to_netmask(&mask, rt->rt_dst.prefix_len)) {
    return -1;
  } else {
    ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_addr = mask.v4;
  }

  kernel_route.rt_flags = olsr_rt_flags(rt);
  kernel_route.rt_metric = olsr_fib_metric(&rt->rt_metric);

  /*
   * Set interface
   */
  kernel_route.rt_dev = NULL;

  if ((rslt = ioctl(olsr_cnf->ioctl_s, SIOCDELRT, &kernel_route)) >= 0) {

    /*
     * Send IPC route update message
     */
    ipc_route_send_rtentry(&rt->rt_dst.prefix, NULL, 0, 0, NULL);
  }

  return rslt;
}

/**
 *Remove a route from the kernel
 *
 *@param destination the route to remove
 *
 *@return negative on error
 */
int
olsr_ioctl_del_route6(const struct rt_entry *rt)
{
  struct in6_rtmsg kernel_route;
  int rslt;

  OLSR_PRINTF(2, "KERN: Deleting %s\n", olsr_rt_to_string(rt));

  memset(&kernel_route, 0, sizeof(struct in6_rtmsg));

  kernel_route.rtmsg_dst = rt->rt_dst.prefix.v6;
  kernel_route.rtmsg_dst_len = rt->rt_dst.prefix_len;

  kernel_route.rtmsg_gateway = rt->rt_best->rtp_nexthop.gateway.v6;

  kernel_route.rtmsg_flags = olsr_rt_flags(rt);
  kernel_route.rtmsg_metric = olsr_fib_metric(&rt->rt_best->rtp_metric);

  if ((rslt = ioctl(olsr_cnf->ioctl_s, SIOCDELRT, &kernel_route) >= 0)) {

    /*
     * Send IPC route update message
     */
    ipc_route_send_rtentry(&rt->rt_dst.prefix, NULL, 0, 0, NULL);
  }

  return rslt;
}

#endif /* LINUX_NETLINK_ROUTING */
