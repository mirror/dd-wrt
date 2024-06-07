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

#ifdef __linux__

#include "kernel_routes.h"
#include "ipc_frontend.h"
#include "log.h"
#include "net_os.h"
#include "ifnet.h"

#include <assert.h>
#include <linux/types.h>
#include <linux/rtnetlink.h>

//ipip includes
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/ip.h>
#include <linux/if_tunnel.h>

//ifup includes
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>

/*
 * The ARM compile complains about alignment. Copied
 * from /usr/include/linux/netlink.h and adapted for ARM
 */
#define MY_NLMSG_NEXT(nlh,len)   ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
          (struct nlmsghdr*)ARM_NOWARN_ALIGN((((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))


static void rtnetlink_read(int sock, void *, unsigned int);

struct olsr_rtreq {
  struct nlmsghdr n;
  struct rtmsg r;
  char buf[1024];
};

struct olsr_ipadd_req {
  struct nlmsghdr n;
  struct ifaddrmsg ifa;
  char buf[256];
};

int rtnetlink_register_socket(int rtnl_mgrp)
{
  int sock = socket(AF_NETLINK,SOCK_RAW,NETLINK_ROUTE);
  struct sockaddr_nl addr;

  if (sock<0) {
    OLSR_PRINTF(1,"could not create rtnetlink socket! %s (%d)", strerror(errno), errno);
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  addr.nl_pid = 0; //kernel will assign appropriate number instead of pid (which is already used by primary rtnetlink socket to add/delete routes)
  addr.nl_groups = rtnl_mgrp;

  if (bind(sock,(struct sockaddr *)&addr,sizeof(addr))<0) {
    OLSR_PRINTF(1,"could not bind rtnetlink socket! %s (%d)",strerror(errno), errno);
    close (sock);
    return -1;
  }

  add_olsr_socket(sock, NULL, &rtnetlink_read, NULL, SP_IMM_READ);
  return sock;
}

static void netlink_process_link(struct nlmsghdr *h)
{
  struct ifinfomsg *ifi = (struct ifinfomsg *) NLMSG_DATA(h);
  struct interface_olsr *iface;
  struct olsr_if *oif;
  char namebuffer[IF_NAMESIZE];
  char * ifaceName = NULL;
  bool up;

  iface = if_ifwithindex(ifi->ifi_index);

  if (!iface) {
    ifaceName = if_indextoname(ifi->ifi_index, namebuffer);
  } else {
    ifaceName = iface->int_name;
  }

  oif = ifaceName ? olsrif_ifwithname(ifaceName) : NULL;
  up = (getInterfaceLinkState(ifaceName) != LINKSTATE_DOWN) && ((ifi->ifi_flags & IFF_UP) != 0);

  if (!iface && up) {
    if (oif) {
      /* try to take interface up, will trigger ifchange */
      chk_if_up(oif, 3);
    }
  } else if (iface && !up) {
    /* try to take interface down, will trigger ifchange */
    olsr_remove_interface(iface->olsr_if);
  }

  if (!iface && !oif) {
    /* this is not an OLSR interface */
    olsr_trigger_ifchange(ifi->ifi_index, NULL, up ? IFCHG_IF_ADD : IFCHG_IF_REMOVE);
  }
}

static void rtnetlink_read(int sock, void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused)))
{
  int len, plen;
  struct iovec iov;
  struct sockaddr_nl nladdr;

  struct msghdr msg = {
    .msg_name = &nladdr,
    .msg_namelen = sizeof(nladdr),
    .msg_iov = &iov,
    .msg_iovlen = 1,
    .msg_control = NULL,
    .msg_controllen = 0,
    .msg_flags = 0
  };

  char buffer[4096];
  struct nlmsghdr *nlh = (struct nlmsghdr *)ARM_NOWARN_ALIGN(buffer);
  int ret;

  iov.iov_base = (void *) buffer;
  iov.iov_len = sizeof(buffer);

  while ((ret = recvmsg(sock, &msg, MSG_DONTWAIT)) >= 0) {
    /*check message*/
    len = nlh->nlmsg_len;
    plen = len - sizeof(nlh);
    if (len > ret || plen < 0) {
      OLSR_PRINTF(1,"Malformed netlink message: "
             "len=%d left=%d plen=%d\n",
              len, ret, plen);
      return;
    }

    OLSR_PRINTF(3, "Netlink message received: type 0x%x\n", nlh->nlmsg_type);
    if ((nlh->nlmsg_type == RTM_NEWLINK) || ( nlh->nlmsg_type == RTM_DELLINK)) {
      /* handle ifup/ifdown */
      netlink_process_link(nlh);
    }
  }

  if (errno != EAGAIN) {
    OLSR_PRINTF(1,"netlink listen error %u - %s\n",errno,strerror(errno));
  }
}

static void
olsr_netlink_addreq(struct nlmsghdr *n, size_t reqSize __attribute__ ((unused)), int type, const void *data, int len)
{
  struct rtattr *rta = (struct rtattr *)ARM_NOWARN_ALIGN(((char *)n) + NLMSG_ALIGN(n->nlmsg_len));
  n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_LENGTH(len);
  //produces strange compile error
  //assert(n->nlmsg_len < reqSize);
  rta->rta_type = type;
  rta->rta_len = RTA_LENGTH(len);
  memcpy(RTA_DATA(rta), data, len);
}

/*rt_entry and nexthop and family and table must only be specified with an flag != RT_NONE  && != RT_LO_IP*/
static int
olsr_netlink_send(struct nlmsghdr *nl_hdr)
{
  char rcvbuf[1024];
  struct iovec iov;
  struct sockaddr_nl nladdr;
  struct msghdr msg;
  struct nlmsghdr *h;
  struct nlmsgerr *l_err;
  int ret;

  memset(&nladdr, 0, sizeof(nladdr));
  memset(&msg, 0, sizeof(msg));

  nladdr.nl_family = AF_NETLINK;

  msg.msg_name = &nladdr;
  msg.msg_namelen = sizeof(nladdr);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  iov.iov_base = nl_hdr;
  iov.iov_len = nl_hdr->nlmsg_len;
  ret = sendmsg(olsr_cnf->rtnl_s, &msg, 0);
  if (ret <= 0) {
    olsr_syslog(OLSR_LOG_ERR, "Cannot send data to netlink socket (%d: %s)", errno, strerror(errno));
    return -1;
  }

  iov.iov_base = rcvbuf;
  iov.iov_len = sizeof(rcvbuf);
  ret = recvmsg(olsr_cnf->rtnl_s, &msg, 0);
  if (ret <= 0) {
    olsr_syslog(OLSR_LOG_ERR, "Error while reading answer to netlink message (%d: %s)", errno, strerror(errno));
    return -1;
  }

  h = (struct nlmsghdr *)ARM_NOWARN_ALIGN(rcvbuf);
  if (!NLMSG_OK(h, (unsigned int)ret)) {
    olsr_syslog(OLSR_LOG_ERR, "Received netlink message was malformed (ret=%d, %u)", ret, h->nlmsg_len);
    return -1;
  }

  if (h->nlmsg_type != NLMSG_ERROR) {
    olsr_syslog(OLSR_LOG_INFO,
        "Received unknown netlink response: %u bytes, type %u (not %u) with seqnr %u and flags %u from %u",
        h->nlmsg_len, h->nlmsg_type, NLMSG_ERROR, h->nlmsg_seq, h->nlmsg_flags, h->nlmsg_pid);
    return -1;
  }
  if (NLMSG_LENGTH(sizeof(struct nlmsgerr)) > h->nlmsg_len) {
    olsr_syslog(OLSR_LOG_INFO,"Received invalid netlink message size %lu != %u",
        (unsigned long int)sizeof(struct nlmsgerr), h->nlmsg_len);
    return -1;
  }

  l_err = (struct nlmsgerr *)NLMSG_DATA(h);

  if (l_err->error) {
    olsr_syslog(OLSR_LOG_INFO,"Received netlink error code %s (%d)", strerror(-l_err->error), l_err->error);
  }
  return -l_err->error;
}

int olsr_os_policy_rule(int family, int rttable, uint32_t priority, const char *if_name, bool set) {
  struct olsr_rtreq req;
  int err;

  memset(&req, 0, sizeof(req));

  req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK;

  req.n.nlmsg_type = set ? RTM_NEWRULE : RTM_DELRULE;
  req.r.rtm_family = family;
  req.r.rtm_table = rttable;

  /* probably unneeded */
  req.r.rtm_type = RTN_UNICAST;

  olsr_netlink_addreq(&req.n, sizeof(req), RTA_PRIORITY, &priority, sizeof(priority));

  if (if_name != NULL) {
    /*add interface name to rule*/
    olsr_netlink_addreq(&req.n, sizeof(req), RTA_IIF, if_name, strlen(if_name)+1);
  }

  err = olsr_netlink_send(&req.n);
  if (err) {
    olsr_syslog(OLSR_LOG_ERR,"Error on %s policy rule aimed to activate RtTable %u!",
        set ? "inserting" : "deleting", rttable);
  }

  return err;
}

static int
olsr_add_ip(int ifindex, union olsr_ip_addr *ip, const char *l, bool create)
{
  struct olsr_ipadd_req req;

  memset(&req, 0, sizeof(req));

  req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
  if (create) {
   req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE | NLM_F_ACK;
   req.n.nlmsg_type = RTM_NEWADDR;
  } else {
   req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
   req.n.nlmsg_type = RTM_DELADDR;
  }
  req.ifa.ifa_family = olsr_cnf->ip_version;

  olsr_netlink_addreq(&req.n, sizeof(req), IFA_LOCAL, ip, olsr_cnf->ipsize);
  if (l) {
    olsr_netlink_addreq(&req.n, sizeof(req), IFA_LABEL, l, strlen(l) + 1);
  }

  req.ifa.ifa_prefixlen = olsr_cnf->ipsize * 8;

  req.ifa.ifa_index = ifindex;

  return olsr_netlink_send(&req.n);
}

int
olsr_os_localhost_if(union olsr_ip_addr *ip, bool create)
{
  static char l[] = "lo:olsr";
  return olsr_add_ip(if_nametoindex("lo"), ip, l, create);
}

int olsr_os_ifip(int ifindex, union olsr_ip_addr *ip, bool create) {
  return olsr_add_ip(ifindex, ip, NULL, create);
}

int olsr_new_netlink_route(unsigned char family, uint32_t rttable, unsigned int flags, unsigned char scope, int if_index, int metric, int protocol,
    const union olsr_ip_addr *src, const union olsr_ip_addr *gw, const struct olsr_ip_prefix *dst,
    bool set, bool del_similar, bool blackhole) {

  struct olsr_rtreq req;
  int family_size;
  int err;

  if (0) {
    struct ipaddr_str buf1, buf2;

    olsr_syslog(OLSR_LOG_INFO, "new_netlink_route: family=%d,rttable=%d,if_index=%d,metric=%d,protocol=%d,src=%s,gw=%s,dst=%s,set=%s,del_similar=%s",
        family, rttable, if_index, metric, protocol, src == NULL ? "" : olsr_ip_to_string(&buf1, src),
        gw == NULL ? "" : olsr_ip_to_string(&buf2, gw), olsr_ip_prefix_to_string(dst),
        set ? "true" : "false", del_similar ? "true" : "false");
  }
  family_size = family == AF_INET ? sizeof(struct in_addr) : sizeof(struct in6_addr);

  memset(&req, 0, sizeof(req));

  req.r.rtm_flags = flags;
  req.r.rtm_family = family;
#ifdef RTM_GETDCB
  if (rttable < 256)
    req.r.rtm_table = rttable;
  else {
    req.r.rtm_table = RT_TABLE_UNSPEC;
    olsr_netlink_addreq(&req.n, sizeof(req), RTA_TABLE, &rttable, sizeof(rttable));
  }
#else
  req.r.rtm_table = rttable;
#endif

  req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;

  if (set) {
    req.n.nlmsg_flags |= NLM_F_CREATE | NLM_F_REPLACE;
    req.n.nlmsg_type = RTM_NEWROUTE;
  } else {
    req.n.nlmsg_type = RTM_DELROUTE;
  }

  /* RTN_UNSPEC would be the wildcard, but blackhole broadcast or nat roules should usually not conflict */
  /* -> olsr only adds deletes unicast routes */
  if (blackhole) {
    req.r.rtm_type = RTN_BLACKHOLE;
  } else {
    req.r.rtm_type = RTN_UNICAST;
  }

  req.r.rtm_dst_len = dst->prefix_len;

  if (set) {
    /* add protocol for setting a route */
    req.r.rtm_protocol = protocol;
  }

  /* calculate scope of operation */
  if (!set && del_similar) {
    /* as wildcard for fuzzy deletion */
    req.r.rtm_scope = RT_SCOPE_NOWHERE;
  }
  else {
    /* for all our routes */
    req.r.rtm_scope = scope;
  }

  if ((set || !del_similar) && !blackhole) {
    /* add interface*/
    olsr_netlink_addreq(&req.n, sizeof(req), RTA_OIF, &if_index, sizeof(if_index));
  }

  if (set && src != NULL) {
    /* add src-ip */
    olsr_netlink_addreq(&req.n, sizeof(req), RTA_PREFSRC, src, family_size);
  }

  if (metric >= 0) {
    /* add metric */
    olsr_netlink_addreq(&req.n, sizeof(req), RTA_PRIORITY, &metric, sizeof(metric));
  }

  if (gw) {
    /* add gateway */
    olsr_netlink_addreq(&req.n, sizeof(req), RTA_GATEWAY, gw, family_size);
  }
  else {
    if ( dst->prefix_len == 32 ) {
      /* use destination as gateway, to 'force' linux kernel to do proper source address selection */
      olsr_netlink_addreq(&req.n, sizeof(req), RTA_GATEWAY, &dst->prefix, family_size);
    }
    else {
      /*do not use onlink on such routes(no gateway, but no hostroute aswell) -  e.g. smartgateway default route over an ptp tunnel interface*/
      req.r.rtm_flags &= (~RTNH_F_ONLINK);
    }
  }

   /* add destination */
  olsr_netlink_addreq(&req.n, sizeof(req), RTA_DST, &dst->prefix, family_size);

  err = olsr_netlink_send(&req.n);
  if (err) {
      struct ipaddr_str buf;
    if (gw) {
      olsr_syslog(OLSR_LOG_ERR, ". error: %s route to %s via %s dev %s onlink (%s %d)",
          set ? "add" : "del",
          olsr_ip_prefix_to_string(dst), olsr_ip_to_string(&buf, gw),
          if_ifwithindex_name(if_index), strerror(errno), errno);
    }
    else {
      olsr_syslog(OLSR_LOG_ERR, ". error: %s route to %s via %s dev %s onlink (%s %d)",
          set ? "add" : "del",
          olsr_ip_prefix_to_string(dst), olsr_ip_to_string(&buf, &dst->prefix), if_ifwithindex_name(if_index),
          strerror(errno), errno);
    }
  }

  return err;
}

void olsr_os_niit_6to4_route(const struct olsr_ip_prefix *dst_v6, bool set) {
  if (olsr_new_netlink_route(AF_INET6,
      ip_prefix_is_mappedv4_inetgw(dst_v6) ? olsr_cnf->rt_table_default : olsr_cnf->rt_table,
      RTNH_F_ONLINK, RT_SCOPE_UNIVERSE, olsr_cnf->niit6to4_if_index,
      olsr_cnf->fib_metric_default, olsr_cnf->rt_proto, NULL, NULL, dst_v6, set, false, false)) {
    olsr_syslog(OLSR_LOG_ERR, ". error while %s static niit route to %s",
        set ? "setting" : "removing", olsr_ip_prefix_to_string(dst_v6));
  }
}

void olsr_os_niit_4to6_route(const struct olsr_ip_prefix *dst_v4, bool set) {
  if (olsr_new_netlink_route(AF_INET,
      ip_prefix_is_v4_inetgw(dst_v4) ? olsr_cnf->rt_table_default : olsr_cnf->rt_table,
      RTNH_F_ONLINK, RT_SCOPE_UNIVERSE, olsr_cnf->niit4to6_if_index,
      olsr_cnf->fib_metric_default, olsr_cnf->rt_proto, NULL, NULL, dst_v4, set, false, false)) {
    olsr_syslog(OLSR_LOG_ERR, ". error while %s niit route to %s",
        set ? "setting" : "removing", olsr_ip_prefix_to_string(dst_v4));
  }
}

void olsr_os_inetgw_tunnel_route(uint32_t if_idx, bool ipv4, bool set, uint8_t table) {
  const struct olsr_ip_prefix *dst;

  assert(olsr_cnf->ip_version == AF_INET6 || ipv4);

  dst = ipv4 ? &ipv4_internet_route : &ipv6_internet_route;

  if (olsr_new_netlink_route(ipv4 ? AF_INET : AF_INET6, table, RTNH_F_ONLINK, RT_SCOPE_UNIVERSE,
      if_idx, olsr_cnf->fib_metric_default, olsr_cnf->rt_proto, NULL, NULL, dst, set, false, false)) {
    olsr_syslog(OLSR_LOG_ERR, ". error while %s inetgw tunnel route to %s for if %d",
        set ? "setting" : "removing", olsr_ip_prefix_to_string(dst), if_idx);
  }
}

static int olsr_os_process_rt_entry(unsigned char af_family, const struct rt_entry *rt, bool set) {
  int metric;
  uint32_t table;
  const struct rt_nexthop *nexthop;
  union olsr_ip_addr *src;
  bool hostRoute;
  int err;

  /* calculate metric */
  if (FIBM_FLAT == olsr_cnf->fib_metric) {
    metric = olsr_cnf->fib_metric_default;
  }
  else {
    metric = set ? rt->rt_best->rtp_metric.hops : rt->rt_metric.hops;
  }

  if (olsr_cnf->smart_gw_active && is_prefix_inetgw(&rt->rt_dst)) {
    /* make space for the tunnel gateway route */
    metric += 2;
  }

  /* get table */
  table = is_prefix_inetgw(&rt->rt_dst)
      ? olsr_cnf->rt_table_default : olsr_cnf->rt_table;

  /* get next hop */
  if (rt->rt_best && set) {
    nexthop = &rt->rt_best->rtp_nexthop;
  }
  else {
    nexthop = &rt->rt_nexthop;
  }

  /* detect 1-hop hostroute */
  hostRoute = rt->rt_dst.prefix_len == olsr_cnf->ipsize * 8
      && ipequal(&nexthop->gateway, &rt->rt_dst.prefix);

  /* get src ip */
  if (olsr_cnf->use_src_ip_routes) {
    src = &olsr_cnf->unicast_src_ip;
  }
  else {
    src = NULL;
  }

  /* create route */
  err = olsr_new_netlink_route(af_family, table, RTNH_F_ONLINK, RT_SCOPE_UNIVERSE, nexthop->iif_index, metric, olsr_cnf->rt_proto,
      src, hostRoute ? NULL : &nexthop->gateway, &rt->rt_dst, set, false, false);

  /* resolve "File exist" (17) propblems (on orig and autogen routes)*/
  if (set && err == 17) {
    /* a similar route going over another gateway may be present, which has to be deleted! */
    olsr_syslog(OLSR_LOG_ERR, ". auto-deleting similar routes to resolve 'File exists' (17) while adding route!");

    /* erase similar rule */
    err = olsr_new_netlink_route(af_family, table, RTNH_F_ONLINK, RT_SCOPE_UNIVERSE, 0, 0, -1, NULL, NULL, &rt->rt_dst, false, true, false);

    if (!err) {
      /* create this rule a second time if delete worked*/
      err = olsr_new_netlink_route(af_family, table, RTNH_F_ONLINK, RT_SCOPE_UNIVERSE, nexthop->iif_index, metric, olsr_cnf->rt_proto,
          src, hostRoute ? NULL : &nexthop->gateway, &rt->rt_dst, set, false, false);
    }
    olsr_syslog(OLSR_LOG_ERR, ". %s (%d)", err == 0 ? "successful" : "failed", err);
  }

  /* report success on "No such process" (3) */
  else if (!set && err == 3) {
    /* another similar (but slightly different) route may be present at this point,
     * if so this will get solved when adding new route to this destination */
    olsr_syslog(OLSR_LOG_ERR, ". ignoring 'No such process' (3) while deleting route!");
    err = 0;
  }
  /* insert route to gateway on the fly if "Network unreachable" (128) on 2.4 kernels
   * or on 2.6 kernel No such process (3) or Network unreachable (101) is reported in rtnetlink response
   * do this only with flat metric, as using metric values inherited from
   * a target behind the gateway is really strange, and could lead to multiple routes!
   * anyways if invalid gateway ips may happen we are f*cked up!!
   * but if not, these on the fly generated routes are no problem, and will only get used when needed */
  else if (!hostRoute && olsr_cnf->fib_metric == FIBM_FLAT
      && (err == 128 || err == 101 || err == 3)) {
    struct olsr_ip_prefix hostPrefix;

    if (err == 128)  {
      olsr_syslog(OLSR_LOG_ERR, ". autogenerating route to handle 'Network unreachable' (128) while adding route!");
    }
    else if (err == 101) {
      olsr_syslog(OLSR_LOG_ERR, ". autogenerating route to handle 'Network unreachable' (101) while adding route!");
    }
    else {
      olsr_syslog(OLSR_LOG_ERR, ". autogenerating route to handle 'No such process' (3) while adding route!");
    }

    /* create hostroute */
    hostPrefix.prefix = nexthop->gateway;
    hostPrefix.prefix_len = olsr_cnf->ipsize * 8;

    err = olsr_new_netlink_route(af_family, olsr_cnf->rt_table, RTNH_F_ONLINK, RT_SCOPE_UNIVERSE, nexthop->iif_index,
        metric, olsr_cnf->rt_proto, src, NULL, &hostPrefix, true, false, false);
    if (err == 0) {
      /* create this rule a second time if hostrule generation was successful */
      err = olsr_new_netlink_route(af_family, table, RTNH_F_ONLINK, RT_SCOPE_UNIVERSE, nexthop->iif_index, metric, olsr_cnf->rt_proto,
          src, &nexthop->gateway, &rt->rt_dst, set, false, false);
    }
    olsr_syslog(OLSR_LOG_ERR, ". %s (%d)", err == 0 ? "successful" : "failed", err);
  }

  return err;
}

/**
 * Insert a route in the kernel routing table
 *
 * @param rt the route to add
 *
 * @return negative on error
 */
int
olsr_ioctl_add_route(const struct rt_entry *rt)
{
  OLSR_PRINTF(2, "KERN: Adding %s\n", olsr_rtp_to_string(rt->rt_best));
  return olsr_os_process_rt_entry(AF_INET, rt, true);
}

/**
 *Insert a route in the kernel routing table
 *
 *@param rt the route to add
 *
 *@return negative on error
 */
int
olsr_ioctl_add_route6(const struct rt_entry *rt)
{
  OLSR_PRINTF(2, "KERN: Adding %s\n", olsr_rtp_to_string(rt->rt_best));
  return olsr_os_process_rt_entry(AF_INET6, rt, true);
}

/**
 *Remove a route from the kernel
 *
 *@param rt the route to remove
 *
 *@return negative on error
 */
int
olsr_ioctl_del_route(const struct rt_entry *rt)
{
  OLSR_PRINTF(2, "KERN: Deleting %s\n", olsr_rt_to_string(rt));
  return olsr_os_process_rt_entry(AF_INET, rt, false);
}

/**
 *Remove a route from the kernel
 *
 *@param rt the route to remove
 *
 *@return negative on error
 */
int
olsr_ioctl_del_route6(const struct rt_entry *rt)
{
  OLSR_PRINTF(2, "KERN: Deleting %s\n", olsr_rt_to_string(rt));
  return olsr_os_process_rt_entry(AF_INET6, rt, false);
}
#endif /* __linux__ */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
