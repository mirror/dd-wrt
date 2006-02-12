/* dnsmasq is Copyright (c) 2000-2005 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

/* Author's email: simon@thekelleys.org.uk */

#include "dnsmasq.h"

#ifdef HAVE_RTNETLINK

#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

int netlink_init(void)
{
  struct sockaddr_nl addr;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  
  if (sock < 0)
    return -1; /* no kernel support */

  addr.nl_family = AF_NETLINK;
  addr.nl_pad = 0;
  addr.nl_pid = getpid();
  addr.nl_groups = 0;
  
  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    die(_("cannot bind netlink socket: %s"), NULL);
  
  return sock;
}


/* We borrow the DNS packet buffer here. (The DHCP one already has a packet in it)
   Since it's used only within this routine, that's fine, just remember 
   that calling icmp_echo() will trash it */
int netlink_process(struct daemon *daemon, int index, struct in_addr relay, 
		    struct in_addr primary, struct dhcp_context **retp)
{
  struct sockaddr_nl addr;
  struct nlmsghdr *h;
  int len, found_primary = 0;
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
  while((len = recvfrom(daemon->netlinkfd, daemon->packet, daemon->packet_buff_sz, 
			MSG_WAITALL, NULL, 0)) == -1 && retry_send());
 
  if (len == -1)
    return 0;
  
  h = (struct nlmsghdr *)daemon->packet;

  while (NLMSG_OK(h, (unsigned int)len))
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

      h = NLMSG_NEXT(h, len);
    }

  *retp = ret;

  return found_primary;
}

#endif

      
