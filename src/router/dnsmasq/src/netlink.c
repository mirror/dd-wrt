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

#ifdef HAVE_RTNETLINK

#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

static struct iovec iov[1];

void netlink_init(struct daemon *daemon)
{
  struct sockaddr_nl addr;
  daemon->netlinkfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  
  if (daemon->netlinkfd < 0)
    return; /* no kernel support */

  addr.nl_family = AF_NETLINK;
  addr.nl_pad = 0;
  addr.nl_pid = getpid();
#ifdef HAVE_IPV6
  addr.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE;
#else
  addr.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE;
#endif
  
  if (bind(daemon->netlinkfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    die(_("cannot bind netlink socket: %s"), NULL);

  iov[0].iov_len = 200;
  iov[0].iov_base = safe_malloc(iov[0].iov_len);
}

static ssize_t netlink_recv(struct daemon *daemon)
{
  struct msghdr msg;
  ssize_t rc;

  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
    
 retry:
  while ((rc = recvmsg(daemon->netlinkfd, &msg, MSG_PEEK)) == -1 && retry_send());
  
  if (rc == -1)
    return -1;

  if (msg.msg_flags & MSG_TRUNC)
    {
      size_t newsz = iov[0].iov_len + 100;
      void *new = realloc(iov[0].iov_base, newsz);
      if (!new)
	return -1;
      iov[0].iov_len = newsz;
      iov[0].iov_base = new;
      goto retry;
    }
  
  while ((rc = recvmsg(daemon->netlinkfd, &msg, 0)) == -1 && retry_send());
  
  return rc;
}
  
int netlink_process(struct daemon *daemon, int index, struct in_addr relay, 
		    struct in_addr primary, struct dhcp_context **retp)
{
  struct sockaddr_nl addr;
  struct nlmsghdr *h;
  int found_primary = 0;
  ssize_t len;
  struct dhcp_context *ret = NULL;
  static unsigned int seq = 0;

  struct {
    struct nlmsghdr nlh;
    struct rtgenmsg g; 
  } req;

  if (daemon->netlinkfd == -1)
    return 0;

  addr.nl_family = AF_NETLINK;
  addr.nl_pad = 0;
  addr.nl_groups = 0;
  addr.nl_pid = 0; /* address to kernel */

  req.nlh.nlmsg_len = sizeof(req);
  req.nlh.nlmsg_type = RTM_GETADDR;
  req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
  req.nlh.nlmsg_pid = 0;
  req.nlh.nlmsg_seq = ++seq;
  req.g.rtgen_family = AF_INET; 

  /* Don't block in recvfrom if send fails */
  while((len = sendto(daemon->netlinkfd, (void *)&req, sizeof(req), 0, 
		      (struct sockaddr *)&addr, sizeof(addr))) == -1 && retry_send());

  if (len == -1)
    {
      /* if RTnetlink not configured in the kernel, don't keep trying. */
      if (errno == ECONNREFUSED)
	{
	  close(daemon->netlinkfd);
	  daemon->netlinkfd = -1;
	}
      return 0;
    }

 get_next:
  if ((len = netlink_recv(daemon)) == -1)
    return 0;
  
  for (h = (struct nlmsghdr *)iov[0].iov_base; NLMSG_OK(h, (unsigned int)len); h = NLMSG_NEXT(h, len))
    {
      if (h->nlmsg_seq != seq)
	goto get_next;
	
      if (h->nlmsg_type == NLMSG_DONE)
	break;

      if (h->nlmsg_type == NLMSG_ERROR)
	return 0;
      
      if (h->nlmsg_type == RTM_NEWADDR)
	{
	  struct ifaddrmsg *ifa = NLMSG_DATA(h);  
	  if (ifa->ifa_index == index && ifa->ifa_family == AF_INET)
	    {
	      struct rtattr *rta = IFA_RTA(ifa);
	      unsigned int len1 = h->nlmsg_len - NLMSG_LENGTH(sizeof(*ifa));
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
	      
	      if (addr.s_addr)
		{
		  ret = complete_context(daemon, addr, ret, netmask, broadcast, relay, primary);	
		  if (addr.s_addr == primary.s_addr)
		    found_primary = 1;
		}
	    }
	}
    }

  *retp = ret;
  return found_primary;
}


/* We arrange to receive netlink multicast messages whenever the network config changes.
   If this happens and we still have a DNS packet in the buffer, we re-send it.
   This helps on DoD links, where frequently the packet which triggers dialling is
   a DNS query, which then gets lost. By re-sending, we can avoid the lookup
   failing. */ 
void netlink_multicast(struct daemon *daemon)
{
  ssize_t len;
  struct nlmsghdr *h;
  
  if ((len = netlink_recv(daemon)) == -1)
    return;
  
  if (!daemon->srv_save)
    return;

  for (h = (struct nlmsghdr *)iov[0].iov_base; NLMSG_OK(h, (unsigned int)len); h = NLMSG_NEXT(h, len))
    {
      if (h->nlmsg_type == NLMSG_DONE || h->nlmsg_type == NLMSG_ERROR)
	break;
      
      if (h->nlmsg_type == RTM_NEWADDR || h->nlmsg_type == RTM_NEWROUTE) 
	{
	  while(sendto(daemon->srv_save->sfd->fd, daemon->packet, daemon->packet_len, 0,
		       &daemon->srv_save->addr.sa, sa_len(&daemon->srv_save->addr)) == -1 && retry_send()); 
	  break;
	}
    }
}

#endif

      
