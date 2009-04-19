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

#include "kernel_routes.h"
#include "ipc_frontend.h"
#include "log.h"

/* values for control flag to handle recursive route corrections 
 *  currently only requires in linux specific kernel_routes.c */

#define RT_ORIG_REQUEST 0
#define RT_RETRY_AFTER_ADD_GATEWAY 1
#define RT_RETRY_AFTER_DELETE_SIMILAR 2
#define RT_DELETE_SIMILAR_ROUTE 3
#define RT_AUTO_ADD_GATEWAY_ROUTE 4
#define RT_DELETE_SIMILAR_AUTO_ROUTE 5

#if !LINUX_POLICY_ROUTING

static int delete_all_inet_gws(void);

#else /* !LINUX_POLICY_ROUTING */

#include <assert.h>
#include <linux/types.h>
#include <linux/rtnetlink.h>

struct olsr_rtreq {
  struct nlmsghdr n;
  struct rtmsg r;
  char buf[512];
};

static void
olsr_netlink_addreq(struct olsr_rtreq *req, int type, const void *data, int len)
{
  struct rtattr *rta = (struct rtattr *)(((char *)req) + NLMSG_ALIGN(req->n.nlmsg_len));
  req->n.nlmsg_len = NLMSG_ALIGN(req->n.nlmsg_len) + RTA_LENGTH(len);
  assert(req->n.nlmsg_len < sizeof(struct olsr_rtreq));
  rta->rta_type = type;
  rta->rta_len = RTA_LENGTH(len);
  memcpy(RTA_DATA(rta), data, len);
}

/* returns
 * -1 on unrecoverable error (calling function will have to handle it)
 *  0 on unexpected but recoverable rtnetlink behaviour
 *    but some of the implemented recovery methods only cure symptoms, 
 *    not the cause, like unintelligent ordering of inserted routes.
 *  1 on success */
static int
olsr_netlink_route_int(const struct rt_entry *rt, uint8_t family, uint8_t rttable, __u16 cmd, uint8_t flag)
{
  int ret = 1; /* helper variable for rtnetlink_message processing */
  int rt_ret = -2;  /* if no response from rtnetlink it must be considered as failed! */
  struct olsr_rtreq req;
  struct iovec iov;
  struct sockaddr_nl nladdr;
  struct msghdr msg = {
    &nladdr,
    sizeof(nladdr),
    &iov,
    1,
    NULL,
    0,
    0
  };
  uint32_t metric = ((cmd != RTM_NEWRULE) | (cmd != RTM_DELRULE)) ?
    FIBM_FLAT != olsr_cnf->fib_metric ? 
      ((RTM_NEWROUTE == cmd) ? rt->rt_best->rtp_metric.hops : rt->rt_metric.hops) 
      : RT_METRIC_DEFAULT
      : 0;
  const struct rt_nexthop *nexthop = ( ( cmd != RTM_NEWRULE ) | ( cmd != RTM_DELRULE ) ) ? 
                                       ( RTM_NEWROUTE == cmd ) ? &rt->rt_best->rtp_nexthop : &rt->rt_nexthop 
                                       : NULL;
  memset(&req, 0, sizeof(req));

  req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK;
  req.n.nlmsg_type = cmd;
  req.r.rtm_family = family;
  req.r.rtm_table = rttable;
  /* RTN_UNSPEC would be the wildcard, but blackhole broadcast or nat roules should usually not conflict */
  req.r.rtm_type = RTN_UNICAST; /* -> olsr only adds deletes unicast routes */
  req.r.rtm_protocol = RTPROT_UNSPEC; /* wildcard to delete routes of all protos if no simlar-delete correct proto will get set below */
  req.r.rtm_scope = RT_SCOPE_NOWHERE; /* as wildcard for deletion */

  if (( cmd != RTM_NEWRULE ) & ( cmd != RTM_DELRULE ) ) {
    req.r.rtm_dst_len = rt->rt_dst.prefix_len;

    /* do not specify much as we wanna delete similar/conflicting routes */
    if ( ( flag != RT_DELETE_SIMILAR_ROUTE ) & ( flag != RT_DELETE_SIMILAR_AUTO_ROUTE )) {
      /* 0 gets replaced by OS-specifc default (3)
       * 1 is reserved so we take 0 instead (this really makes some sense)
       * other numbers are used 1:1 */
      req.r.rtm_protocol = ( (olsr_cnf->rtproto<1) ? RTPROT_BOOT : ( (olsr_cnf->rtproto==1) ? 0 : olsr_cnf->rtproto) );
      req.r.rtm_scope = RT_SCOPE_LINK;

      /*add interface*/
      olsr_netlink_addreq(&req, RTA_OIF, &nexthop->iif_index, sizeof(nexthop->iif_index));

      #if SOURCE_IP_ROUTES
      /* source ip here is based on now static olsr_cnf->main_addr in this olsr-0.5.6-r4, should be based on orignator-id in newer olsrds */
      if (AF_INET == family) olsr_netlink_addreq(&req, RTA_PREFSRC, &olsr_cnf->main_addr.v4.s_addr, sizeof(olsr_cnf->main_addr.v4.s_addr));
      else olsr_netlink_addreq(&req, RTA_PREFSRC, &olsr_cnf->main_addr.v6.s6_addr, sizeof(olsr_cnf->main_addr.v6.s6_addr));
      #endif
    }

    /* metric is specified always as we can only delete one route per iteration, and wanna hit the correct one first */
    if (FIBM_APPROX != olsr_cnf->fib_metric || (RTM_NEWROUTE == cmd) ) {
      olsr_netlink_addreq(&req, RTA_PRIORITY, &metric, sizeof(metric));
    }

    /* make sure that netmask = /32 as this is an autogenarated route */
    if (( flag == RT_AUTO_ADD_GATEWAY_ROUTE ) | (flag == RT_DELETE_SIMILAR_AUTO_ROUTE) ) req.r.rtm_dst_len = 32;

    /* for ipv4 or ipv6 we add gateway if one is specified, 
    * or leave gateway away if we want to delete similar routes aswell, 
    * or even use the gateway as target if we add a auto-generated route, 
    * or if delete-similar to make insertion of auto-generated route possible */
    if (AF_INET == family) {
      if ( ( flag != RT_AUTO_ADD_GATEWAY_ROUTE ) & (flag != RT_DELETE_SIMILAR_ROUTE) & 
           ( flag != RT_DELETE_SIMILAR_AUTO_ROUTE) & (rt->rt_dst.prefix.v4.s_addr != nexthop->gateway.v4.s_addr) ) {
        olsr_netlink_addreq(&req, RTA_GATEWAY, &nexthop->gateway.v4, sizeof(nexthop->gateway.v4));
        req.r.rtm_scope = RT_SCOPE_UNIVERSE;
      }
      olsr_netlink_addreq(&req, RTA_DST, ( (flag == RT_AUTO_ADD_GATEWAY_ROUTE) | (flag == RT_DELETE_SIMILAR_AUTO_ROUTE) ) ? 
                          &nexthop->gateway.v4 : &rt->rt_dst.prefix.v4, sizeof(rt->rt_dst.prefix.v4));
    } else {
      if ( ( flag != RT_AUTO_ADD_GATEWAY_ROUTE ) & (flag != RT_DELETE_SIMILAR_ROUTE ) & ( flag != RT_DELETE_SIMILAR_AUTO_ROUTE) 
          & (0 != memcmp(&rt->rt_dst.prefix.v6, &nexthop->gateway.v6, sizeof(nexthop->gateway.v6))) ) {
        olsr_netlink_addreq(&req, RTA_GATEWAY, &nexthop->gateway.v6, sizeof(nexthop->gateway.v6));
        req.r.rtm_scope = RT_SCOPE_UNIVERSE;
      }
      olsr_netlink_addreq(&req, RTA_DST, ( (flag == RT_AUTO_ADD_GATEWAY_ROUTE) | (flag == RT_DELETE_SIMILAR_AUTO_ROUTE) ) ? 
                          &nexthop->gateway.v6 : &rt->rt_dst.prefix.v6, sizeof(rt->rt_dst.prefix.v6));
    }
  } else {//add or delete a rule
    static uint32_t priority = 65535;
    req.r.rtm_scope = RT_SCOPE_UNIVERSE;
    olsr_netlink_addreq(&req, RTA_PRIORITY, &priority, sizeof(priority));
  }

  iov.iov_base = &req.n;
  iov.iov_len = req.n.nlmsg_len;
  memset(&nladdr, 0, sizeof(nladdr));
  nladdr.nl_family = AF_NETLINK;
  if (0 <= (ret = sendmsg(olsr_cnf->rtnl_s, &msg, 0))) {
    iov.iov_base = req.buf;
    iov.iov_len = sizeof(req.buf);
    if (0 < (ret = recvmsg(olsr_cnf->rtnl_s, &msg, 0))) {
      struct nlmsghdr *h = (struct nlmsghdr *)req.buf;
      while (NLMSG_OK(h, (unsigned int)ret)) {
        if (NLMSG_DONE == h->nlmsg_type) {
          //seems to reached never
          olsr_syslog(OLSR_LOG_INFO, "_received NLMSG_DONE");
          break;
        }
        if (NLMSG_ERROR == h->nlmsg_type) {
          if (NLMSG_LENGTH(sizeof(struct nlmsgerr) <= h->nlmsg_len)) {
            struct ipaddr_str ibuf;
            struct ipaddr_str gbuf;
            struct nlmsgerr *l_err = (struct nlmsgerr *)NLMSG_DATA(h);
            errno = -l_err->error;
            if (0 != errno) {
            const char *const err_msg = strerror(errno);
              struct ipaddr_str buf;
              rt_ret = -1;
              /* syslog debug output for various situations */
              if ( cmd == RTM_NEWRULE ) {
                olsr_syslog(OLSR_LOG_ERR,"Error '%s' (%d) on inserting empty policy rule aimed to activate RtTable %u!", err_msg, errno, rttable);
              }
              else if ( cmd == RTM_DELRULE ) {
                olsr_syslog(OLSR_LOG_ERR,"Error '%s' (%d) on deleting empty policy rule aimed to activate rtTable %u!", err_msg, errno, rttable);
              }
              else if ( flag <= RT_RETRY_AFTER_DELETE_SIMILAR ) {
                if (rt->rt_dst.prefix.v4.s_addr!=nexthop->gateway.v4.s_addr)
                  olsr_syslog(OLSR_LOG_ERR, "error '%s' (%d) %s route to %s/%d via %s dev %s", err_msg, errno, (cmd == RTM_NEWROUTE) ? "add" : "del",
                              olsr_ip_to_string(&ibuf,&rt->rt_dst.prefix), req.r.rtm_dst_len,
                              olsr_ip_to_string(&gbuf,&nexthop->gateway), if_ifwithindex_name(nexthop->iif_index));
                else olsr_syslog(OLSR_LOG_ERR, "error '%s' (%d) %s route to %s/%d dev %s", err_msg, errno, (cmd == RTM_NEWROUTE) ? "add" : "del",
                                 olsr_ip_to_string(&ibuf,&rt->rt_dst.prefix), req.r.rtm_dst_len, if_ifwithindex_name(nexthop->iif_index));
              }
              else if (flag == RT_AUTO_ADD_GATEWAY_ROUTE) 
                       olsr_syslog(OLSR_LOG_ERR, ". error '%s' (%d) auto-add route to %s dev %s", err_msg, errno,
                                   olsr_ip_to_string(&ibuf,&nexthop->gateway), if_ifwithindex_name(nexthop->iif_index));
              else if (flag == RT_DELETE_SIMILAR_ROUTE) 
                       olsr_syslog(OLSR_LOG_ERR, ". error '%s' (%d) auto-delete route to %s dev %s", err_msg, errno,
                                   olsr_ip_to_string(&ibuf,&rt->rt_dst.prefix), if_ifwithindex_name(nexthop->iif_index));
              else if (flag == RT_DELETE_SIMILAR_AUTO_ROUTE) 
                       olsr_syslog(OLSR_LOG_ERR, ". . error '%s' (%d) auto-delete similar route to %s dev %s", err_msg, errno,
                                   olsr_ip_to_string(&ibuf,&nexthop->gateway), if_ifwithindex_name(nexthop->iif_index));
              else { /* should never happen */
                olsr_syslog(OLSR_LOG_ERR, "# invalid internal route delete/add flag (%d) used!", flag);
              }
            }
            else { /* netlink acks requests with an errno=0 NLMSG_ERROR response! */
              rt_ret = 1;
            }

            /* resolve "File exist" (17) propblems (on orig and autogen routes)*/	
            if ((errno == 17) & ((flag == RT_ORIG_REQUEST) | (flag == RT_AUTO_ADD_GATEWAY_ROUTE)) & (cmd == RTM_NEWROUTE)) {
              /* a similar route going over another gateway may be present, which has to be deleted! */
              olsr_syslog(OLSR_LOG_ERR, ". auto-deleting similar routes to resolve 'File exists' (17) while adding route!");
              rt_ret = RT_DELETE_SIMILAR_ROUTE; /* processing will contiune after this loop */
            }
            /* report success on "No such process" (3) */
            else if ((errno == 3) & (cmd == RTM_DELROUTE) & (flag == RT_ORIG_REQUEST)) {
              /* another similar (but slightly different) route may be present at this point
              * , if so this will get solved when adding new route to this destination */
              olsr_syslog(OLSR_LOG_ERR, ". ignoring 'No such process' (3) while deleting route!");
              rt_ret = 0;
            }
            /* insert route to gateway on the fly if "Network unreachable" (128) on 2.4 kernels
             * or on 2.6 kernel No such process (3) is reported in rtnetlink response
             * do this only with flat metric, as using metric values inherited from 
             * a target behind the gateway is really strange, and could lead to multiple routes!
             * anyways if invalid gateway ips may happen we are f*cked up!!
             * but if not, these on the fly generated routes are no problem, and will only get used when needed */
            else if ( ((errno == 3)|(errno == 128)) & (flag == RT_ORIG_REQUEST) & (FIBM_FLAT == olsr_cnf->fib_metric) 
                     & (cmd == RTM_NEWROUTE) & (rt->rt_dst.prefix.v4.s_addr!=nexthop->gateway.v4.s_addr)) {
              if (errno == 128) olsr_syslog(OLSR_LOG_ERR, ". autogenerating route to handle 'Network unreachable' (128) while adding route!");
              else olsr_syslog(OLSR_LOG_ERR, ". autogenerating route to handle 'No such process' (3) while adding route!");

              rt_ret = RT_AUTO_ADD_GATEWAY_ROUTE; /* processing will contiune after this loop */
            }
          }
          /* report invalid message size */
          else olsr_syslog(OLSR_LOG_INFO,"_received invalid netlink message size %lu != %u",(unsigned long int)sizeof(struct nlmsgerr), h->nlmsg_len);
        }
        /* log all other messages */
        else olsr_syslog(OLSR_LOG_INFO,"_received %u Byte rtnetlink response of type %u with seqnr %u and flags %u from %u (%u)",
                         h->nlmsg_len, h->nlmsg_type, h->nlmsg_seq, h->nlmsg_flags, h->nlmsg_pid, NLMSG_ERROR);
        h = NLMSG_NEXT(h, ret);
      }
    }
  }
  if ( rt_ret == RT_DELETE_SIMILAR_ROUTE ) {//delete all routes that may collide
    /* recursive call to delete simlar routes, using flag 2 to invoke deletion of similar, not only exact matches*/
    rt_ret = olsr_netlink_route_int(rt, family, rttable, RTM_DELROUTE, 
                                    flag == RT_AUTO_ADD_GATEWAY_ROUTE ? RT_DELETE_SIMILAR_AUTO_ROUTE : RT_DELETE_SIMILAR_ROUTE);

    /* retry insert original route, if deleting similar succeeded, using flag=1 to prevent recursions */
    if (rt_ret > 0) rt_ret = olsr_netlink_route_int(rt, family, rttable, RTM_NEWROUTE, RT_RETRY_AFTER_DELETE_SIMILAR);
    else olsr_syslog(OLSR_LOG_ERR, ". failed on auto-deleting similar route conflicting with above route!");

    /* set appropriate return code for original request, while returning simple -1/1 if called recursive */
    if (flag != RT_AUTO_ADD_GATEWAY_ROUTE) {
      if (rt_ret > 0) rt_ret = 0; /* successful recovery */
      else rt_ret = -1; /* unrecoverable error */
    }
  }
  if ( rt_ret == RT_AUTO_ADD_GATEWAY_ROUTE ) { /* autoadd route via gateway */
    /* recursive call to invoke creation of a route to the gateway */
    rt_ret = olsr_netlink_route_int(rt, family, rttable, RTM_NEWROUTE, RT_AUTO_ADD_GATEWAY_ROUTE);

    /* retry insert original route, if above succeeded without problems */
    if (rt_ret > 0) rt_ret = olsr_netlink_route_int(rt, family, rttable, RTM_NEWROUTE, RT_RETRY_AFTER_ADD_GATEWAY);
    else olsr_syslog(OLSR_LOG_ERR, ". failed on inserting auto-generated route to gateway of above route!");

    /* set appropriate return code for original request*/
    if (rt_ret > 0) rt_ret = 0; /* successful recovery */
    else rt_ret = -1; /* unrecoverable error */
  }
  //send ipc update on success
  if ( ( cmd != RTM_NEWRULE ) & ( cmd != RTM_DELRULE ) & (flag = RT_ORIG_REQUEST) & (0 <= rt_ret && olsr_cnf->ipc_connections > 0)) {
    ipc_route_send_rtentry(&rt->rt_dst.prefix, &nexthop->gateway, metric, RTM_NEWROUTE == cmd,
                             if_ifwithindex_name(nexthop->iif_index));
  }
  if (rt_ret == -2) olsr_syslog(OLSR_LOG_ERR,"no rtnetlink response! (no system ressources left?, everything may happen now ...)");
  return rt_ret;
}

/*external wrapper function for above patched multi purpose rtnetlink function*/
int
olsr_netlink_rule(uint8_t family, uint8_t rttable, uint16_t cmd)
{
  struct rt_entry rt;
  return olsr_netlink_route_int(&rt, family, rttable, cmd, RT_ORIG_REQUEST);
}

/*internal wrapper function for above patched function*/
static int
olsr_netlink_route(const struct rt_entry *rt, uint8_t family, uint8_t rttable, __u16 cmd)
{
  return olsr_netlink_route_int(rt, family, rttable, cmd, RT_ORIG_REQUEST);
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
  kernel_route.rt_metric = olsr_fib_metric(&rt->rt_best->rtp_metric.hops);

  /*
   * Set interface
   */
  kernel_route.rt_dev = if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index);

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
#else /* !LINUX_POLICY_ROUTING */
  if (0 == olsr_cnf->rttable_default && 0 == rt->rt_dst.prefix_len && 253 > olsr_cnf->rttable) {
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

  kernel_route.rtmsg_dst = rt->rt_dst.prefix.v6;
  kernel_route.rtmsg_dst_len = rt->rt_dst.prefix_len;

  kernel_route.rtmsg_gateway = rt->rt_best->rtp_nexthop.gateway.v6;

  kernel_route.rtmsg_flags = olsr_rt_flags(rt);
  kernel_route.rtmsg_metric = olsr_fib_metric(&rt->rt_best->rtp_metric.hops);

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
  if (0 == olsr_cnf->rttable_default && 0 == rt->rt_dst.prefix_len && 253 > olsr_cnf->rttable) {
    /*
     * Also remove the fallback default route
     */
    olsr_netlink_route(rt, AF_INET, 253, RTM_DELROUTE);
  }
  if (0 == rt->rt_dst.prefix_len && olsr_cnf->rttable_default != 0)
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
  memset(&kernel_route, 0, sizeof(struct in6_rtmsg));

  kernel_route.rtmsg_dst = rt->rt_dst.prefix.v6;
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
#endif /* LINUX_POLICY_ROUTING */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
