 /* dnsmasq is Copyright (c) 2000-2006 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include "dnsmasq.h"

#ifdef HAVE_LINUX_NETWORK

#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

static struct iovec iov;

static void nl_err(struct nlmsghdr *h);
static void nl_routechange(struct daemon *daemon, struct nlmsghdr *h);

void netlink_init(struct daemon *daemon)
{
  struct sockaddr_nl addr;

  addr.nl_family = AF_NETLINK;
  addr.nl_pad = 0;
  addr.nl_pid = 0; /* autobind */
#ifdef HAVE_IPV6
  addr.nl_groups = RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE;
#else
  addr.nl_groups = RTMGRP_IPV4_ROUTE;
#endif
  
  /* May not be able to have permission to set multicast groups don't die in that case */
  if ((daemon->netlinkfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) != -1)
    {
      if (bind(daemon->netlinkfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
	  addr.nl_groups = 0;
	  if (errno != EPERM || bind(daemon->netlinkfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	    daemon->netlinkfd = -1;
	}
    }
  
  if (daemon->netlinkfd == -1)
    die(_("cannot create netlink socket: %s"), NULL);
  else
    {
      int flags = fcntl(daemon->netlinkfd, F_GETFD);
      if (flags != -1)
	fcntl(daemon->netlinkfd, F_SETFD, flags | FD_CLOEXEC); 
    }

  iov.iov_len = 200;
  iov.iov_base = safe_malloc(iov.iov_len);
}

static ssize_t netlink_recv(struct daemon *daemon)
{
  struct msghdr msg;
  ssize_t rc;

  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
    
  while (1)
    {
      msg.msg_flags = 0;
      while ((rc = recvmsg(daemon->netlinkfd, &msg, MSG_PEEK)) == -1 && errno == EINTR);
      
      /* 2.2.x doesn't suport MSG_PEEK at all, returning EOPNOTSUPP, so we just grab a 
	 big buffer and pray in that case. */
      if (rc == -1 && errno == EOPNOTSUPP)
	{
	  if (!expand_buf(&iov, 2000))
	    return -1;
	  break;
	}
      
      if (rc == -1 || !(msg.msg_flags & MSG_TRUNC))
        break;
            
      if (!expand_buf(&iov, iov.iov_len + 100))
	return -1;
    }

  /* finally, read it for real */
  while ((rc = recvmsg(daemon->netlinkfd, &msg, 0)) == -1 && errno == EINTR);
  
  return rc;
}
  
int iface_enumerate(struct daemon *daemon, void *parm, int (*ipv4_callback)(), int (*ipv6_callback)())
{
  struct sockaddr_nl addr;
  struct nlmsghdr *h;
  ssize_t len;
  static unsigned int seq = 0;
  int family = AF_INET;

  struct {
    struct nlmsghdr nlh;
    struct rtgenmsg g; 
  } req;

  addr.nl_family = AF_NETLINK;
  addr.nl_pad = 0;
  addr.nl_groups = 0;
  addr.nl_pid = 0; /* address to kernel */

 again:
  req.nlh.nlmsg_len = sizeof(req);
  req.nlh.nlmsg_type = RTM_GETADDR;
  req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST | NLM_F_ACK; 
  req.nlh.nlmsg_pid = 0;
  req.nlh.nlmsg_seq = ++seq;
  req.g.rtgen_family = family; 

  /* Don't block in recvfrom if send fails */
  while((len = sendto(daemon->netlinkfd, (void *)&req, sizeof(req), 0, 
		      (struct sockaddr *)&addr, sizeof(addr))) == -1 && retry_send());
  
  if (len == -1)
    return 0;
    
  while (1)
    {
      if ((len = netlink_recv(daemon)) == -1)
	return 0;
  
      for (h = (struct nlmsghdr *)iov.iov_base; NLMSG_OK(h, (size_t)len); h = NLMSG_NEXT(h, len))
 	if (h->nlmsg_type == NLMSG_ERROR)
	  nl_err(h);
	else if (h->nlmsg_seq != seq)
	  nl_routechange(daemon, h); /* May be multicast arriving async */
	else if (h->nlmsg_type == NLMSG_DONE)
	  {
#ifdef HAVE_IPV6
	    if (family == AF_INET && ipv6_callback)
	      {
		family = AF_INET6;
		goto again;
	      }
#endif
	    return 1;
	  }
	else if (h->nlmsg_type == RTM_NEWADDR)
	  {
	    struct ifaddrmsg *ifa = NLMSG_DATA(h);  
	    struct rtattr *rta = IFA_RTA(ifa);
	    unsigned int len1 = h->nlmsg_len - NLMSG_LENGTH(sizeof(*ifa));
	    
	    if (ifa->ifa_family == AF_INET)
	      {
		struct in_addr netmask, addr, broadcast;
		
		netmask.s_addr = htonl(0xffffffff << (32 - ifa->ifa_prefixlen));
		addr.s_addr = 0;
		broadcast.s_addr = 0;
		
		while (RTA_OK(rta, len1))
		  {
		    if (rta->rta_type == IFA_LOCAL)
		      addr = *((struct in_addr *)(rta+1));
		    else if (rta->rta_type == IFA_BROADCAST)
		      broadcast = *((struct in_addr *)(rta+1));
		    
		    rta = RTA_NEXT(rta, len1);
		  }
		
		if (addr.s_addr && ipv4_callback)
		  if (!((*ipv4_callback)(daemon, addr, ifa->ifa_index, netmask, broadcast, parm)))
		    return 0;
	      }
#ifdef HAVE_IPV6
	    else if (ifa->ifa_family == AF_INET6)
	      {
		struct in6_addr *addrp = NULL;
		while (RTA_OK(rta, len1))
		  {
		    if (rta->rta_type == IFA_ADDRESS)
		      addrp = ((struct in6_addr *)(rta+1)); 
		    
		    rta = RTA_NEXT(rta, len1);
		  }
		
		if (addrp && ipv6_callback)
		  if (!((*ipv6_callback)(daemon, addrp, ifa->ifa_index, ifa->ifa_index, parm)))
		    return 0;
	      }
#endif
	  }
    }
}

void netlink_multicast(struct daemon *daemon)
{
  ssize_t len;
  struct nlmsghdr *h;
  
  if ((len = netlink_recv(daemon)) != -1)
    {
      for (h = (struct nlmsghdr *)iov.iov_base; NLMSG_OK(h, (size_t)len); h = NLMSG_NEXT(h, len))
	if (h->nlmsg_type == NLMSG_ERROR)
	  nl_err(h);
	else
	  nl_routechange(daemon, h);
    }
}

static void nl_err(struct nlmsghdr *h)
{
  struct nlmsgerr *err = NLMSG_DATA(h);
  if (err->error != 0)
    syslog(LOG_ERR, _("netlink returns error: %s"), strerror(-(err->error)));
}

/* We arrange to receive netlink multicast messages whenever the network route is added.
   If this happens and we still have a DNS packet in the buffer, we re-send it.
   This helps on DoD links, where frequently the packet which triggers dialling is
   a DNS query, which then gets lost. By re-sending, we can avoid the lookup
   failing. */ 
static void nl_routechange(struct daemon *daemon, struct nlmsghdr *h)
{
  if (h->nlmsg_type == RTM_NEWROUTE && daemon->srv_save)
    {
      struct rtmsg *rtm = NLMSG_DATA(h);
      if (rtm->rtm_type == RTN_UNICAST &&
	  rtm->rtm_scope == RT_SCOPE_LINK) 
	while(sendto(daemon->srv_save->sfd->fd, daemon->packet, daemon->packet_len, 0,
		     &daemon->srv_save->addr.sa, sa_len(&daemon->srv_save->addr)) == -1 && retry_send()); 
    }
}
#endif

      
