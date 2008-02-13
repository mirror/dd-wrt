/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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

#include "kernel_routes.h"
#include "ipc_frontend.h"

#if !LINUX_POLICY_ROUTING
#include "log.h"

static int delete_all_inet_gws(void);

#else /* !LINUX_POLICY_ROUTING */

#include <assert.h>
#include <linux/types.h>
#include <linux/rtnetlink.h>

struct olsr_rtreq
{
  struct nlmsghdr n;
  struct rtmsg    r;
  char            buf[512];
};

static void olsr_netlink_addreq(struct olsr_rtreq *req, int type, const void *data, int len)
{
  struct rtattr *rta = (struct rtattr*)(((char*)req) + NLMSG_ALIGN(req->n.nlmsg_len));
  req->n.nlmsg_len = NLMSG_ALIGN(req->n.nlmsg_len) + RTA_LENGTH(len);
  assert(req->n.nlmsg_len < sizeof(struct olsr_rtreq));
  rta->rta_type = type;
  rta->rta_len = RTA_LENGTH(len);
  memcpy(RTA_DATA(rta), data, len);
}

static int olsr_netlink_route(const struct rt_entry *rt, olsr_u8_t family, olsr_u8_t rttable, __u16 cmd)
{
  int ret = 0;
  struct olsr_rtreq req;
  struct iovec iov;
  struct sockaddr_nl nladdr;
  struct msghdr msg =
  {
    &nladdr,
    sizeof(nladdr),
    &iov,
    1,
    NULL,
    0,
    0
  };
  olsr_u32_t metric = FIBM_FLAT != olsr_cnf->fib_metric ? (RTM_NEWROUTE == cmd ?
    rt->rt_best->rtp_metric.hops : rt->rt_metric.hops): RT_METRIC_DEFAULT;
  const struct rt_nexthop* nexthop = (RTM_NEWROUTE == cmd) ?
    &rt->rt_best->rtp_nexthop : &rt->rt_nexthop;

  memset(&req, 0, sizeof(req));
  req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  req.n.nlmsg_flags = NLM_F_REQUEST|NLM_F_CREATE|NLM_F_EXCL|NLM_F_ACK;
  req.n.nlmsg_type = cmd;
  req.r.rtm_family = family;
  req.r.rtm_table = rttable;
  req.r.rtm_protocol = RTPROT_BOOT;
  req.r.rtm_scope = RT_SCOPE_LINK;
  req.r.rtm_type = RTN_UNICAST;
  req.r.rtm_dst_len = rt->rt_dst.prefix_len;

  if (AF_INET == family)
  {
    if (rt->rt_dst.prefix.v4.s_addr != nexthop->gateway.v4.s_addr)
    {
      olsr_netlink_addreq(&req, RTA_GATEWAY, &nexthop->gateway.v4, sizeof(nexthop->gateway.v4));
      req.r.rtm_scope = RT_SCOPE_UNIVERSE;
    }
    olsr_netlink_addreq(&req, RTA_DST, &rt->rt_dst.prefix.v4, sizeof(rt->rt_dst.prefix.v4));
  }
  else
  {
    if (0 != memcmp(&rt->rt_dst.prefix.v6, &nexthop->gateway.v6, sizeof(nexthop->gateway.v6)))
    {
      olsr_netlink_addreq(&req, RTA_GATEWAY, &nexthop->gateway.v6, sizeof(nexthop->gateway.v6));
      req.r.rtm_scope = RT_SCOPE_UNIVERSE;
    }
    olsr_netlink_addreq(&req, RTA_DST, &rt->rt_dst.prefix.v6, sizeof(rt->rt_dst.prefix.v6));
  }
  if (FIBM_APPROX != olsr_cnf->fib_metric || RTM_NEWROUTE == cmd)
  {
    olsr_netlink_addreq(&req, RTA_PRIORITY, &metric, sizeof(metric));
  }
  olsr_netlink_addreq(&req, RTA_OIF, &nexthop->iif_index, sizeof(nexthop->iif_index));
  iov.iov_base = &req.n;
  iov.iov_len = req.n.nlmsg_len;
  memset(&nladdr, 0, sizeof(nladdr));
  nladdr.nl_family = AF_NETLINK;
  if (0 <= (ret = sendmsg(olsr_cnf->rtnl_s, &msg, 0)))
  {
    iov.iov_base = req.buf;
    iov.iov_len = sizeof(req.buf);
    if (0 < (ret = recvmsg(olsr_cnf->rtnl_s, &msg, 0)))
    {
      struct nlmsghdr* h = (struct nlmsghdr*)req.buf;
      while (NLMSG_OK(h, (unsigned int)ret)) {
        if (NLMSG_DONE == h->nlmsg_type) break;
        if (NLMSG_ERROR == h->nlmsg_type)
        {
          if (NLMSG_LENGTH(sizeof(struct nlmsgerr) <= h->nlmsg_len))
          {
            struct nlmsgerr *l_err = (struct nlmsgerr*)NLMSG_DATA(h);
            errno = -l_err->error;
            if (0 != errno) ret = -1;
          }
          break;
        }
        h = NLMSG_NEXT(h, ret);
      }
    }
    if (0 <= ret && olsr_cnf->ipc_connections > 0)
    {
      ipc_route_send_rtentry(
        &rt->rt_dst.prefix,
        &nexthop->gateway,
        metric,
        RTM_NEWROUTE == cmd,
        if_ifwithindex_name(nexthop->iif_index));
    }
  }
  return ret;
}
#endif /* LINUX_POLICY_ROUTING */

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
#if !LINUX_POLICY_ROUTING
  struct rtentry kernel_route;
  union olsr_ip_addr mask;
  int rslt;
#endif /* LINUX_POLICY_ROUTING */

  OLSR_PRINTF(2, "KERN: Adding %s\n", olsr_rtp_to_string(rt->rt_best));

#if !LINUX_POLICY_ROUTING
  memset(&kernel_route, 0, sizeof(struct rtentry));

  ((struct sockaddr_in*)&kernel_route.rt_dst)->sin_family = AF_INET;
  ((struct sockaddr_in*)&kernel_route.rt_gateway)->sin_family = AF_INET;
  ((struct sockaddr_in*)&kernel_route.rt_genmask)->sin_family = AF_INET;

  ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_addr = rt->rt_dst.prefix.v4;

  if (!olsr_prefix_to_netmask(&mask, rt->rt_dst.prefix_len)) {
    return -1;
  }
  ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_addr = mask.v4;

  if (rt->rt_dst.prefix.v4.s_addr != rt->rt_best->rtp_nexthop.gateway.v4.s_addr) {
    ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_addr =
      rt->rt_best->rtp_nexthop.gateway.v4;
  }

  kernel_route.rt_flags = olsr_rt_flags(rt);
  kernel_route.rt_metric = olsr_fib_metric(&rt->rt_best->rtp_metric.hops);

  /*
   * Set interface
   */
  kernel_route.rt_dev = if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index);

  /* delete existing default route before ? */
  if((olsr_cnf->del_gws) &&
     (rt->rt_dst.prefix.v4.s_addr == INADDR_ANY) &&
     (rt->rt_dst.prefix_len == INADDR_ANY)) {
    delete_all_inet_gws();
    olsr_cnf->del_gws = OLSR_FALSE;
  }

  if ((rslt = ioctl(olsr_cnf->ioctl_s, SIOCADDRT, &kernel_route)) >= 0) {

    /*
     * Send IPC route update message
     */
    ipc_route_send_rtentry(&rt->rt_dst.prefix, &rt->rt_best->rtp_nexthop.gateway,
                           rt->rt_best->rtp_metric.hops, 1,
                           if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index));
  }

  return rslt;
#else /* !LINUX_POLICY_ROUTING */
  if (0 == olsr_cnf->rttable_default && 0 == rt->rt_dst.prefix_len && 253 > olsr_cnf->rttable)
  {
    /*
     * Users start whining about not having internet with policy
     * routing activated and no static default route in table 254.
     * We maintain a fallback defroute in the default=253 table.
     */
    olsr_netlink_route(rt, AF_INET, 253, RTM_NEWROUTE);
  }
  if (0 == rt->rt_dst.prefix_len && olsr_cnf->rttable_default != 0)
    return olsr_netlink_route(rt, AF_INET, olsr_cnf->rttable_default, RTM_NEWROUTE);
  else
    return olsr_netlink_route(rt, AF_INET, olsr_cnf->rttable, RTM_NEWROUTE);
#endif /* LINUX_POLICY_ROUTING */
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
#if !LINUX_POLICY_ROUTING
  struct in6_rtmsg kernel_route;
  int rslt;

  OLSR_PRINTF(2, "KERN: Adding %s\n", olsr_rtp_to_string(rt->rt_best));

  memset(&kernel_route, 0, sizeof(struct in6_rtmsg));

  kernel_route.rtmsg_dst     = rt->rt_dst.prefix.v6;
  kernel_route.rtmsg_dst_len = rt->rt_dst.prefix_len;

  kernel_route.rtmsg_gateway = rt->rt_best->rtp_nexthop.gateway.v6;

  kernel_route.rtmsg_flags = olsr_rt_flags(rt);
  kernel_route.rtmsg_metric = olsr_fib_metric(&rt->rt_best->rtp_metric.hops);
  
  /*
   * set interface
   */
  kernel_route.rtmsg_ifindex = rt->rt_best->rtp_nexthop.iif_index;
  
  /* XXX delete 0/0 route before ? */

  if((rslt = ioctl(olsr_cnf->ioctl_s, SIOCADDRT, &kernel_route)) >= 0) {

    /*
     * Send IPC route update message
     */
    ipc_route_send_rtentry(&rt->rt_dst.prefix, &rt->rt_best->rtp_nexthop.gateway, 
                           rt->rt_best->rtp_metric.hops, 1,
                           if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index));
  }

  return rslt;
#else /* !LINUX_POLICY_ROUTING */
  if (0 == rt->rt_dst.prefix_len && olsr_cnf->rttable_default != 0)
    return olsr_netlink_route(rt, AF_INET6, olsr_cnf->rttable_default, RTM_NEWROUTE);
  else
    return olsr_netlink_route(rt, AF_INET6, olsr_cnf->rttable, RTM_NEWROUTE);
#endif /* LINUX_POLICY_ROUTING */
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
#if !LINUX_POLICY_ROUTING
  struct rtentry kernel_route;
  union olsr_ip_addr mask;
  int rslt;
#endif /* LINUX_POLICY_ROUTING */

  OLSR_PRINTF(2, "KERN: Deleting %s\n", olsr_rt_to_string(rt));

#if !LINUX_POLICY_ROUTING
  memset(&kernel_route,0,sizeof(struct rtentry));

  ((struct sockaddr_in*)&kernel_route.rt_dst)->sin_family = AF_INET;
  ((struct sockaddr_in*)&kernel_route.rt_gateway)->sin_family = AF_INET;
  ((struct sockaddr_in*)&kernel_route.rt_genmask)->sin_family = AF_INET;

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
  kernel_route.rt_metric = olsr_fib_metric(&rt->rt_metric.hops);

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
#else /* !LINUX_POLICY_ROUTING */
  if (0 == olsr_cnf->rttable_default && 0 == rt->rt_dst.prefix_len && 253 > olsr_cnf->rttable)
  {
    /*
     * Also remove the fallback default route
     */
    olsr_netlink_route(rt, AF_INET, 253, RTM_DELROUTE);
  }
  if (0 == rt->rt_dst.prefix_len && olsr_cnf->rttable_default != 0 )
    return olsr_netlink_route(rt, AF_INET, olsr_cnf->rttable_default, RTM_DELROUTE);
  else
    return olsr_netlink_route(rt, AF_INET, olsr_cnf->rttable, RTM_DELROUTE);
#endif /* LINUX_POLICY_ROUTING */
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
#if !LINUX_POLICY_ROUTING
  struct in6_rtmsg kernel_route;
  int rslt;
#endif /* LINUX_POLICY_ROUTING */

  OLSR_PRINTF(2, "KERN: Deleting %s\n", olsr_rt_to_string(rt));

#if !LINUX_POLICY_ROUTING
  memset(&kernel_route,0,sizeof(struct in6_rtmsg));


  kernel_route.rtmsg_dst     = rt->rt_dst.prefix.v6;
  kernel_route.rtmsg_dst_len = rt->rt_dst.prefix_len;

  kernel_route.rtmsg_gateway = rt->rt_best->rtp_nexthop.gateway.v6;

  kernel_route.rtmsg_flags = olsr_rt_flags(rt);
  kernel_route.rtmsg_metric = olsr_fib_metric(&rt->rt_best->rtp_metric.hops);

  if ((rslt = ioctl(olsr_cnf->ioctl_s, SIOCDELRT, &kernel_route) >= 0)) {

    /*
     * Send IPC route update message
     */
    ipc_route_send_rtentry(&rt->rt_dst.prefix, NULL, 0, 0, NULL);
  }

  return rslt;
#else /* !LINUX_POLICY_ROUTING */
  if (0 == rt->rt_dst.prefix_len && olsr_cnf->rttable_default != 0)
    return olsr_netlink_route(rt, AF_INET6, olsr_cnf->rttable_default, RTM_DELROUTE);
  else
    return olsr_netlink_route(rt, AF_INET6, olsr_cnf->rttable, RTM_DELROUTE);
#endif /* LINUX_POLICY_ROUTING */
}

#if !LINUX_POLICY_ROUTING
static int delete_all_inet_gws(void)
{  
  int s;
  char buf[BUFSIZ], *cp, *cplim;
  struct ifconf ifc;
  struct ifreq *ifr;
  
  OLSR_PRINTF(1, "Internet gateway detected...\nTrying to delete default gateways\n");
  
  /* Get a socket */
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
      olsr_syslog(OLSR_LOG_ERR, "socket: %m");
      return -1;
    }
  
  ifc.ifc_len = sizeof (buf);
  ifc.ifc_buf = buf;
  if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) 
    {
      olsr_syslog(OLSR_LOG_ERR, "ioctl (get interface configuration)");
      close(s);
      return -1;
    }

  ifr = ifc.ifc_req;
  cplim = buf + ifc.ifc_len; /*skip over if's with big ifr_addr's */
  for (cp = buf; cp < cplim; cp += sizeof (ifr->ifr_name) + sizeof(ifr->ifr_addr)) 
    {
      struct rtentry kernel_route;
      ifr = (struct ifreq *)cp;
      
      
      if(strcmp(ifr->ifr_ifrn.ifrn_name, "lo") == 0)
	{
          OLSR_PRINTF(1, "Skipping loopback...\n");
	  continue;
	}

      OLSR_PRINTF(1, "Trying 0.0.0.0/0 %s...", ifr->ifr_ifrn.ifrn_name);
      
      
      memset(&kernel_route,0,sizeof(struct rtentry));
      
      ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_addr.s_addr = 0;
      ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_family=AF_INET;
      ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_addr.s_addr = 0;
      ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_family=AF_INET;

      ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_addr.s_addr = INADDR_ANY;
      ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_family=AF_INET;
      

      kernel_route.rt_flags = RTF_UP | RTF_GATEWAY;
	   
      kernel_route.rt_dev = ifr->ifr_ifrn.ifrn_name;

      if((ioctl(s, SIOCDELRT, &kernel_route)) < 0)
         OLSR_PRINTF(1, "NO\n");
      else
         OLSR_PRINTF(1, "YES\n");
    }  
  close(s);
  return 0;       
}
#endif /* LINUX_POLICY_ROUTING */

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
