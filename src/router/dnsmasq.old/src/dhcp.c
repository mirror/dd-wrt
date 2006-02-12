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

void dhcp_init(struct daemon *daemon)
{
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  struct sockaddr_in saddr;
  int flags, oneopt = 1, zeroopt = 0;
  struct dhcp_config *configs, *cp;

  if (fd == -1)
    die (_("cannot create DHCP socket : %s"), NULL);
  
  if ((flags = fcntl(fd, F_GETFL, 0)) == -1 ||
      fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1 ||
#if defined(IP_PKTINFO)
      setsockopt(fd, SOL_IP, IP_PKTINFO, &oneopt, sizeof(oneopt)) == -1 ||
#elif defined(IP_RECVIF)
      setsockopt(fd, IPPROTO_IP, IP_RECVIF, &oneopt, sizeof(oneopt)) == -1 ||
#endif
      setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &oneopt, sizeof(oneopt)) == -1)  
    die(_("failed to set options on DHCP socket: %s"), NULL);
  
  /* When bind-interfaces is set, there might be more than one dnmsasq
     instance binding port 67. That's Ok if they serve different networks.
     Need to set REUSEADDR to make this posible. */
  if ((daemon->options & OPT_NOWILD) &&
      setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &oneopt, sizeof(oneopt)) == -1)
    die(_("failed to set SO_REUSEADDR on DHCP socket: %s"), NULL);
  
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(DHCP_SERVER_PORT);
  saddr.sin_addr.s_addr = INADDR_ANY;
#ifdef HAVE_SOCKADDR_SA_LEN
  saddr.sin_len = sizeof(struct sockaddr_in);
#endif

  if (bind(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in)))
    die(_("failed to bind DHCP server socket: %s"), NULL);

  daemon->dhcpfd = fd;

  if ((fd = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1 ||
      (flags = fcntl(fd, F_GETFL, 0)) == -1 ||
      fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1 ||
      setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &oneopt, sizeof(oneopt)) == -1 ||
      setsockopt(fd, SOL_SOCKET, SO_DONTROUTE, &zeroopt, sizeof(zeroopt)) == -1)
    die(_("cannot create ICMP raw socket: %s."), NULL);

  daemon->dhcp_icmp_fd = fd;

#ifdef HAVE_BPF
  { 
    int i = 0;
    while (1) 
      {
	char filename[50];
	sprintf(filename, "/dev/bpf%d", i++);
	if ((fd = open(filename, O_RDWR, 0)) != -1)
	  break;
	if (errno != EBUSY)
	  die(_("cannot create DHCP BPF socket: %s"), NULL);
      }	    
  }
#else
  /* since we don't ever use the packet socket for reception,
     and it receives copies of _all_ IP packets, then that data
     will build up in kernel buffers, wasting memory. Set the
     socket receive buffer size to one to avoid that. (zero is
     rejected as non-sensical by some BSD kernels) */
  if ((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETHERTYPE_IP))) == -1 ||
      setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &oneopt, sizeof(oneopt)) == -1)
    die(_("cannot create DHCP packet socket: %s. "
	  "Is CONFIG_PACKET enabled in your kernel?"), NULL);
#endif
  
  daemon->dhcp_raw_fd = fd;
  
  /* If the same IP appears in more than one host config, then DISCOVER
     for one of the hosts will get the address, but REQUEST will be NAKed,
     since the address is reserved by the other one -> protocol loop. */
  for (configs = daemon->dhcp_conf; configs; configs = configs->next)
    for (cp = configs->next; cp; cp = cp->next)
      if ((configs->flags & cp->flags & CONFIG_ADDR) &&	configs->addr.s_addr == cp->addr.s_addr)
	die(_("duplicate IP address %s in dhcp-config directive."), inet_ntoa(cp->addr));
  
  daemon->dhcp_packet = safe_malloc(sizeof(struct udp_dhcp_packet));
  /* These two each hold a DHCP option max size 255
     and get a terminating zero added */
  daemon->dhcp_buff = safe_malloc(256);
  daemon->dhcp_buff2 = safe_malloc(256); 
  daemon->ping_results = NULL;
}

void dhcp_packet(struct daemon *daemon, time_t now)
{
  struct udp_dhcp_packet *rawpacket = daemon->dhcp_packet;
  struct dhcp_packet *mess = &rawpacket->data;
  struct dhcp_context *context;
  struct iname *tmp;
  struct ifreq ifr;
  struct msghdr msg;
  struct iovec iov[2];
  struct cmsghdr *cmptr;
  int sz, newlen, iface_index = 0;
  int unicast_dest = 0;
  struct in_addr iface_addr;
#ifdef HAVE_BPF
  unsigned char iface_hwaddr[ETHER_ADDR_LEN];
#endif

  union {
    struct cmsghdr align; /* this ensures alignment */
#ifdef IP_PKTINFO
    char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
#else
    char control[CMSG_SPACE(sizeof(struct sockaddr_dl))];
#endif
  } control_u;
  
  iov[0].iov_base = (char *)mess;
  iov[0].iov_len = sizeof(struct dhcp_packet);

  msg.msg_control = control_u.control;
  msg.msg_controllen = sizeof(control_u);
  msg.msg_flags = 0;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  
  sz = recvmsg(daemon->dhcpfd, &msg, 0);
  
  if (sz < (int)(sizeof(*mess) - sizeof(mess->options)))
    return;
  
#if defined (IP_PKTINFO)
  if (msg.msg_controllen < sizeof(struct cmsghdr))
    return;
  for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
    if (cmptr->cmsg_level == SOL_IP && cmptr->cmsg_type == IP_PKTINFO)
      {
	iface_index = ((struct in_pktinfo *)CMSG_DATA(cmptr))->ipi_ifindex;
	if (((struct in_pktinfo *)CMSG_DATA(cmptr))->ipi_addr.s_addr != INADDR_BROADCAST)
	  unicast_dest = 1;
      }

  if (!(ifr.ifr_ifindex = iface_index) || 
      ioctl(daemon->dhcpfd, SIOCGIFNAME, &ifr) == -1)
    return;
  
#elif defined(IP_RECVIF)
  if (msg.msg_controllen < sizeof(struct cmsghdr))
    return;
  for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
    if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_RECVIF)
      iface_index = ((struct sockaddr_dl *)CMSG_DATA(cmptr))->sdl_index;

  if (!iface_index || !if_indextoname(iface_index, ifr.ifr_name))
    return;

#else
  {
    struct iname *name;
    for (name = daemon->if_names; name->isloop; name = name->next);
    strcpy(ifr.ifr_name, name->name);
  }
#endif

#ifdef HAVE_BPF
  ifr.ifr_addr.sa_family = AF_LINK;
  if (ioctl(daemon->dhcpfd, SIOCGIFADDR, &ifr) < 0)
    return;
  memcpy(iface_hwaddr, LLADDR((struct sockaddr_dl *)&ifr.ifr_addr), ETHER_ADDR_LEN);
#endif
  
  ifr.ifr_addr.sa_family = AF_INET;
  if (ioctl(daemon->dhcpfd, SIOCGIFADDR, &ifr) < 0 )
    return;
  iface_addr = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr;

  /* enforce available interface configuration */
  for (tmp = daemon->if_except; tmp; tmp = tmp->next)
    if (tmp->name && (strcmp(tmp->name, ifr.ifr_name) == 0))
      return;
 
  for (tmp = daemon->dhcp_except; tmp; tmp = tmp->next)
    if (tmp->name && (strcmp(tmp->name, ifr.ifr_name) == 0))
      return;
  
  if (daemon->if_names || daemon->if_addrs)
    {
      for (tmp = daemon->if_names; tmp; tmp = tmp->next)
	if (tmp->name && (strcmp(tmp->name, ifr.ifr_name) == 0))
	  break;
      if (!tmp)
	for (tmp = daemon->if_addrs; tmp; tmp = tmp->next)
	  if (tmp->addr.sa.sa_family == AF_INET && 
	      tmp->addr.in.sin_addr.s_addr == iface_addr.s_addr)
	    break;
      if (!tmp)
	return; 
    }
  
  /* unlinked contexts are marked by context->current == context */
  for (context = daemon->dhcp; context; context = context->next)
    context->current = context;
  
#ifdef HAVE_RTNETLINK
  if (!netlink_process(daemon, iface_index, mess->giaddr, iface_addr, &context))
#endif
    {
      struct in_addr iface_netmask, iface_broadcast;
      
      if (ioctl(daemon->dhcpfd, SIOCGIFNETMASK, &ifr) < 0)
	return;
      iface_netmask = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr;
      
      if (ioctl(daemon->dhcpfd, SIOCGIFBRDADDR, &ifr) < 0)
	return;
      iface_broadcast = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr;
      
      context = complete_context(daemon, iface_addr, NULL, iface_netmask, 
				 iface_broadcast, mess->giaddr, iface_addr);
    }

  lease_prune(NULL, now); /* lose any expired leases */
  newlen = dhcp_reply(daemon, context, ifr.ifr_name, sz, now, unicast_dest);
fprintf(stderr,"update lease file\n");

  lease_update_file(0, now);
  lease_update_dns(daemon);
  
  if (newlen == 0)
    return;
  
  if (mess->giaddr.s_addr || mess->ciaddr.s_addr)
    {
      /* To send to BOOTP relay or configured client, use the IP packet */
      
      struct sockaddr_in dest;
      dest.sin_family = AF_INET;
#ifdef HAVE_SOCKADDR_SA_LEN
      dest.sin_len = sizeof(struct sockaddr_in);
#endif
     
      if (mess->giaddr.s_addr)
	{
	  dest.sin_port = htons(DHCP_SERVER_PORT);
	  dest.sin_addr = mess->giaddr; 
	}
      else
	{
	  dest.sin_port = htons(DHCP_CLIENT_PORT);
	  dest.sin_addr = mess->ciaddr;
	}
      
      while(sendto(daemon->dhcpfd, mess, newlen, 0, 
		   (struct sockaddr *)&dest, sizeof(dest)) == -1 &&
	    retry_send());
    }
  else
    {
      /* Hairy stuff, packet either has to go to the
	 net broadcast or the destination can't reply to ARP yet,
	 but we do know the physical address. 
	 Build the packet by steam, and send directly, bypassing
	 the kernel IP stack */
      
      u32 i, sum;
      unsigned char hwdest[ETHER_ADDR_LEN];
      
      if (ntohs(mess->flags) & 0x8000)
	{
	  memset(hwdest, 255,  ETHER_ADDR_LEN);
	  rawpacket->ip.ip_dst.s_addr = INADDR_BROADCAST;
	}
      else
	{
	  memcpy(hwdest, mess->chaddr, ETHER_ADDR_LEN); 
	  rawpacket->ip.ip_dst.s_addr = mess->yiaddr.s_addr;
	}
      
      rawpacket->ip.ip_p = IPPROTO_UDP;
      rawpacket->ip.ip_src.s_addr = iface_addr.s_addr;
      rawpacket->ip.ip_len = htons(sizeof(struct ip) + 
				   sizeof(struct udphdr) +
				   newlen) ;
      rawpacket->ip.ip_hl = sizeof(struct ip) / 4;
      rawpacket->ip.ip_v = IPVERSION;
      rawpacket->ip.ip_tos = 0;
      rawpacket->ip.ip_id = htons(0);
      rawpacket->ip.ip_off = htons(0x4000); /* don't fragment */
      rawpacket->ip.ip_ttl = IPDEFTTL;
      rawpacket->ip.ip_sum = 0;
      for (sum = 0, i = 0; i < sizeof(struct ip) / 2; i++)
	sum += ((u16 *)&rawpacket->ip)[i];
      while (sum>>16)
	sum = (sum & 0xffff) + (sum >> 16);  
      rawpacket->ip.ip_sum = (sum == 0xffff) ? sum : ~sum;
      
      rawpacket->udp.uh_sport = htons(DHCP_SERVER_PORT);
      rawpacket->udp.uh_dport = htons(DHCP_CLIENT_PORT);
      ((u8 *)&rawpacket->data)[newlen] = 0; /* for checksum, in case length is odd. */
      rawpacket->udp.uh_sum = 0;
      rawpacket->udp.uh_ulen = sum = htons(sizeof(struct udphdr) + newlen);
      sum += htons(IPPROTO_UDP);
      for (i = 0; i < 4; i++)
	sum += ((u16 *)&rawpacket->ip.ip_src)[i];
      for (i = 0; i < (sizeof(struct udphdr) + newlen + 1) / 2; i++)
	sum += ((u16 *)&rawpacket->udp)[i];
      while (sum>>16)
	sum = (sum & 0xffff) + (sum >> 16);
      rawpacket->udp.uh_sum = (sum == 0xffff) ? sum : ~sum;
      
      { 
#ifdef HAVE_BPF
	struct ether_header header;
	
	header.ether_type = htons(ETHERTYPE_IP);
	memcpy(header.ether_shost, iface_hwaddr, ETHER_ADDR_LEN);
	memcpy(header.ether_dhost, hwdest, ETHER_ADDR_LEN); 
	
	ioctl(daemon->dhcp_raw_fd, BIOCSETIF, &ifr);
	
	iov[0].iov_base = (char *)&header;
	iov[0].iov_len = sizeof(struct ether_header);
	iov[1].iov_base = (char *)rawpacket;
	iov[1].iov_len = ntohs(rawpacket->ip.ip_len);
	while (writev(daemon->dhcp_raw_fd, iov, 2) == -1 && retry_send());
#else
	struct sockaddr_ll dest;
	
	memset(&dest, 0, sizeof(dest));
	dest.sll_family = AF_PACKET;
	dest.sll_halen =  ETHER_ADDR_LEN;
	dest.sll_ifindex = iface_index;
	dest.sll_protocol = htons(ETHERTYPE_IP);
	memcpy(dest.sll_addr, hwdest, ETHER_ADDR_LEN); 
	while (sendto(daemon->dhcp_raw_fd, rawpacket, ntohs(rawpacket->ip.ip_len), 
		      0, (struct sockaddr *)&dest, sizeof(dest)) == -1 &&
	       retry_send());
#endif
      }
    }
}

/* This is a complex routine: it gets called with each (address,netmask,broadcast) triple 
   of the interface on which a DHCP packet arrives (and any relay address) and does the 
   following things:
   1) Fills in any netmask and broadcast addresses which have not been explicitly configured.
   2) Fills in local (this host) and router (this host or relay) addresses.
   3) Links contexts which are valid for hosts directly connected to the arrival interface on ->current.
   Note that the current chain may be superceded later for configured hosts or those coming via gateways. */
struct dhcp_context *complete_context(struct daemon *daemon, struct in_addr local, struct dhcp_context *current,
				      struct in_addr netmask, struct in_addr broadcast, struct in_addr relay,
				      struct in_addr primary)
{
  struct dhcp_context *context;
  
  for (context = daemon->dhcp; context; context = context->next)
    {
      if (!(context->flags & CONTEXT_NETMASK) &&
	  (is_same_net(local, context->start, netmask) ||
	   is_same_net(local, context->end, netmask)))
      { 
	if (context->netmask.s_addr != netmask.s_addr &&
	    !(is_same_net(local, context->start, netmask) &&
	      is_same_net(local, context->end, netmask)))
	  {
	    strcpy(daemon->dhcp_buff, inet_ntoa(context->start));
	    strcpy(daemon->dhcp_buff2, inet_ntoa(context->end));
	    syslog(LOG_WARNING, _("DHCP range %s -- %s is not consistent with netmask %s"),
		   daemon->dhcp_buff, daemon->dhcp_buff2, inet_ntoa(netmask));
	  }	
 	context->netmask = netmask;
      }
      
      if (context->netmask.s_addr)
	{
	  if (is_same_net(local, context->start, context->netmask) &&
	      is_same_net(local, context->end, context->netmask))
	    {
	      /* link it onto the current chain if we've not seen it before */
	      if (context->current == context)
		{
		  context->router = local;
		  context->local = local;
		  context->current = current;
		  current = context;
		}
	      
	      if (!(context->flags & CONTEXT_BRDCAST))
		{
		  if (is_same_net(broadcast, context->start, context->netmask))
		    context->broadcast = broadcast;
		  else 
		    context->broadcast.s_addr  = context->start.s_addr | ~context->netmask.s_addr;
		}
	    }	
	  else if (relay.s_addr && is_same_net(relay, context->start, context->netmask))
	    {
	      context->router = relay;
	      context->local = primary;
	      /* fill in missing broadcast addresses for relayed ranges */
	      if (!(context->flags & CONTEXT_BRDCAST))
		context->broadcast.s_addr  = context->start.s_addr | ~context->netmask.s_addr;
	    }

	}
    }

  return current;
}
	  
struct dhcp_context *address_available(struct dhcp_context *context, struct in_addr taddr)
{
  /* Check is an address is OK for this network, check all
     possible ranges. */
  
  unsigned int start, end, addr = ntohl(taddr.s_addr);
  
  for (; context; context = context->current)
    {
      start = ntohl(context->start.s_addr);
      end = ntohl(context->end.s_addr);

      if (!(context->flags & CONTEXT_STATIC) &&
	  addr >= start &&
	  addr <= end)
	return context;
    }

  return NULL;
}

struct dhcp_context *narrow_context(struct dhcp_context *context, struct in_addr taddr)
{
  /* We start of with a set of possible contexts, all on the current subnet.
     These are chained on ->current.
     Here we have an address, and return the actual context correponding to that
     address. Note that none may fit, if the address came a dhcp-host and is outside
     any dhcp-range. In that case we return a static range is possible, or failing that,
     any context on the subnet. (If there's more than one, this is a dodgy configuration: 
     maybe there should be a warning.) */
  
  struct dhcp_context *tmp = address_available(context, taddr);

  if (tmp)
    return tmp;
  
  for (tmp = context; tmp; tmp = tmp->current)
    if (tmp->flags & CONTEXT_STATIC)
      return tmp;

  return context;
}

struct dhcp_config *config_find_by_address(struct dhcp_config *configs, struct in_addr addr)
{
  struct dhcp_config *config;
  
  for (config = configs; config; config = config->next)
    if ((config->flags & CONFIG_ADDR) && config->addr.s_addr == addr.s_addr)
      return config;

  return NULL;
}

/* Is every member of check matched by a member of pool? */
int match_netid(struct dhcp_netid *check, struct dhcp_netid *pool)
{
  struct dhcp_netid *tmp1;
  
  if (!check)
    return 0;

  for (; check; check = check->next)
    {
      if (check->net[0] != '#')
	{
	  for (tmp1 = pool; tmp1; tmp1 = tmp1->next)
	    if (strcmp(check->net, tmp1->net) == 0)
	      break;
	  if (!tmp1)
	    return 0;
	}
      else
	for (tmp1 = pool; tmp1; tmp1 = tmp1->next)
	  if (strcmp((check->net)+1, tmp1->net) == 0)
	    return 0;
    }
  return 1;
}

int address_allocate(struct dhcp_context *context, struct daemon *daemon,
		     struct in_addr *addrp, unsigned char *hwaddr, 
		     struct dhcp_netid *netids, time_t now)   
{
  /* Find a free address: exclude anything in use and anything allocated to
     a particular hwaddr/clientid/hostname in our configuration.
     Try to return from contexts which mathc netis first. */

  struct in_addr start, addr ;
  struct dhcp_context *c;
  unsigned int i, j;
  
  for (c = context; c; c = c->current)
    if (c->flags & CONTEXT_STATIC)
      continue;
    else if (netids && !(c->flags & CONTEXT_FILTER))
      continue;
    else if (!netids && (c->flags & CONTEXT_FILTER))
      continue;
    else if (netids && (c->flags & CONTEXT_FILTER) && !match_netid(&c->netid, netids))
      continue;
    else
      {
	/* pick a seed based on hwaddr then iterate until we find a free address. */
	for (j = c->addr_epoch, i = 0; i < ETHER_ADDR_LEN; i++)
	  j += hwaddr[i] + (hwaddr[i] << 8) + (hwaddr[i] << 16);
	
	start.s_addr = addr.s_addr = 
	  htonl(ntohl(c->start.s_addr) + 
		(j % (1 + ntohl(c->end.s_addr) - ntohl(c->start.s_addr))));
	
	do {
	  if (!lease_find_by_addr(addr) && 
	      !config_find_by_address(daemon->dhcp_conf, addr))
	    {
	      struct ping_result *r, *victim = NULL;
	      int count;

	      /* check if we failed to ping addr sometime in the last
		 30s. If so, assume the same situation still exists.
		 This avoids problems when a stupid client bangs
		 on us repeatedly. As a final check, is we did more
		 than six ping checks in the last 30s, we are in 
		 high-load mode, so don't do any more. */
	      for (count = 0, r = daemon->ping_results; r; r = r->next)
		if (difftime(now, r->time) > 30.0)
		  victim = r; /* old record */
		else if (++count == 6 || r->addr.s_addr == addr.s_addr)
		  {
		    *addrp = addr;
		    return 1;
		  }
	      
	      if (icmp_ping(daemon, addr))
		/* address in use: perturb address selection so that we are
		   less likely to try this address again. */
		c->addr_epoch++;
	      else
		{
		  /* at this point victim may hold an expired record */
		  if (!victim)
		    {
		      if ((victim = malloc(sizeof(struct ping_result))))
			{
			  victim->next = daemon->ping_results;
			  daemon->ping_results = victim;
			}
		    }
		  
		  /* record that this address is OK for 30s 
		     without more ping checks */
		  if (victim)
		    {
		      victim->addr = addr;
		      victim->time = now;
		    }
		  *addrp = addr;
		  return 1;
		}
	    }

	  addr.s_addr = htonl(ntohl(addr.s_addr) + 1);
	  
	  if (addr.s_addr == htonl(ntohl(c->end.s_addr) + 1))
	    addr = c->start;
	  
	} while (addr.s_addr != start.s_addr);
      }

  if (netids)
    return address_allocate(context, daemon, addrp, hwaddr, NULL, now);

  return 0;
}

static int is_addr_in_context(struct dhcp_context *context, struct dhcp_config *config)
{
  if (!context) /* called via find_config() from lease_update_from_configs() */
    return 1; 
  if (!(config->flags & CONFIG_ADDR))
    return 1;
  for (; context; context = context->current)
    if (is_same_net(config->addr, context->start, context->netmask))
      return 1;
  
  return 0;
}


struct dhcp_config *find_config(struct dhcp_config *configs,
				struct dhcp_context *context,
				unsigned char *clid, int clid_len,
				unsigned char *hwaddr, char *hostname)
{
  struct dhcp_config *config; 
  
  if (clid)
    for (config = configs; config; config = config->next)
      if (config->flags & CONFIG_CLID)
	{
	  if (config->clid_len == clid_len && 
	      memcmp(config->clid, clid, clid_len) == 0 &&
	      is_addr_in_context(context, config))
	    return config;
	  
	  /* dhcpcd prefixes ASCII client IDs by zero which is wrong, but we try and
	     cope with that here */
	  if (*clid == 0 && config->clid_len == clid_len-1  &&
	      memcmp(config->clid, clid+1, clid_len-1) == 0 &&
	      is_addr_in_context(context, config))
	    return config;
	}
  

  if (hwaddr)
    for (config = configs; config; config = config->next)
      if ((config->flags & CONFIG_HWADDR) &&
	  config->wildcard_mask == 0 &&
	  memcmp(config->hwaddr, hwaddr, ETHER_ADDR_LEN) == 0 &&
	  is_addr_in_context(context, config))
	return config;
  
  
  if (hostname && context)
    for (config = configs; config; config = config->next)
      if ((config->flags & CONFIG_NAME) && 
	  hostname_isequal(config->hostname, hostname) &&
	  is_addr_in_context(context, config))
	return config;
  
  if (hwaddr)
    for (config = configs; config; config = config->next)
      if ((config->flags & CONFIG_HWADDR) &&
	  config->wildcard_mask != 0 &&
	  is_addr_in_context(context, config))
	{
	  int i;
	  unsigned int mask = config->wildcard_mask;
	  for (i = ETHER_ADDR_LEN - 1; i >= 0; i--, mask = mask >> 1)
	    if (mask & 1)
	      config->hwaddr[i] = hwaddr[i];
	  if (memcmp(config->hwaddr, hwaddr, ETHER_ADDR_LEN) == 0)
	    return config;
	}
  
  return NULL;
}

void dhcp_read_ethers(struct daemon *daemon)
{
  FILE *f = fopen(ETHERSFILE, "r");
  unsigned int flags;
  char *buff = daemon->namebuff;
  char *ip, *cp;
  struct in_addr addr;
  unsigned char hwaddr[ETHER_ADDR_LEN];
  struct dhcp_config *config, *configs = daemon->dhcp_conf;
  int count = 0, lineno = 0;

  addr.s_addr = 0; /* eliminate warning */
  
  if (!f)
    {
      syslog(LOG_ERR, _("failed to read %s:%m"), ETHERSFILE);
      return;
    }

  while (fgets(buff, MAXDNAME, f))
    {
      lineno++;
      
      while (strlen(buff) > 0 && isspace(buff[strlen(buff)-1]))
	buff[strlen(buff)-1] = 0;
      
      if ((*buff == '#') || (*buff == '+'))
	continue;
      
      for (ip = buff; *ip && !isspace(*ip); ip++);
      for(; *ip && isspace(*ip); ip++)
	*ip = 0;
      if (!*ip || parse_hex(buff, hwaddr, 6, NULL) != 6)
	{
	  syslog(LOG_ERR, _("bad line at %s line %d"), ETHERSFILE, lineno); 
	  continue;
	}
      
      /* check for name or dotted-quad */
      for (cp = ip; *cp; cp++)
	if (!(*cp == '.' || (*cp >='0' && *cp <= '9')))
	  break;
      
      if (!*cp)
	{
	  if ((addr.s_addr = inet_addr(ip)) == (in_addr_t)-1)
	    {
	      syslog(LOG_ERR, _("bad address at %s line %d"), ETHERSFILE, lineno); 
	      continue;
	    }

	  flags = CONFIG_ADDR;
	  
	  for (config = configs; config; config = config->next)
	    if ((config->flags & CONFIG_ADDR) && config->addr.s_addr == addr.s_addr)
	      break;
	}
      else 
	{
	  if (!canonicalise(ip))
	    {
	      syslog(LOG_ERR, _("bad name at %s line %d"), ETHERSFILE, lineno); 
	      continue;
	    }

	  flags = CONFIG_NAME;

	  for (config = configs; config; config = config->next)
	    if ((config->flags & CONFIG_NAME) && hostname_isequal(config->hostname, ip))
	      break;
	}
      
      if (!config)
	{ 
	  for (config = configs; config; config = config->next)
	    if ((config->flags & CONFIG_HWADDR) && 
		config->wildcard_mask == 0 &&
		memcmp(config->hwaddr, hwaddr, ETHER_ADDR_LEN) == 0)
	      break;
	  
	  if (!config)
	    {
	      if (!(config = malloc(sizeof(struct dhcp_config))))
		continue;
	      config->flags = 0;
	      config->wildcard_mask = 0;
	      config->next = configs;
	      configs = config;
	    }
	  
	  config->flags |= flags;
	  
	  if (flags & CONFIG_NAME)
	    {
	      if ((config->hostname = malloc(strlen(ip)+1)))
		strcpy(config->hostname, ip);
	      else
		config->flags &= ~CONFIG_NAME;
	    }
	  
	  if (flags & CONFIG_ADDR)
	    config->addr = addr;
	}
      
      config->flags |= CONFIG_HWADDR | CONFIG_NOCLID;
      memcpy(config->hwaddr, hwaddr, ETHER_ADDR_LEN);

      count++;
    }
  
  fclose(f);

  syslog(LOG_INFO, _("read %s - %d addresses"), ETHERSFILE, count);
  
  daemon->dhcp_conf =  configs;
}

void dhcp_update_configs(struct dhcp_config *configs)
{
  /* Some people like to keep all static IP addresses in /etc/hosts.
     This goes through /etc/hosts and sets static addresses for any DHCP config
     records which don't have an address and whose name matches. 
     We take care to maintain the invariant that any IP address can appear
     in at most one dhcp-host. */
  
  struct dhcp_config *config;
  struct crec *crec;
  
  for (config = configs; config; config = config->next)
    if (!(config->flags & CONFIG_ADDR) &&
	(config->flags & CONFIG_NAME) && 
	(crec = cache_find_by_name(NULL, config->hostname, 0, F_IPV4)) &&
	(crec->flags & F_HOSTS))
      {
	if (config_find_by_address(configs, crec->addr.addr.addr.addr4))
	  syslog(LOG_WARNING, _("duplicate IP address %s (%s) in dhcp-config directive"), 
		 inet_ntoa(crec->addr.addr.addr.addr4), config->hostname);
	else
	  {
	    config->addr = crec->addr.addr.addr.addr4;
	    config->flags |= CONFIG_ADDR;
	  }
      }
}

/* If we've not found a hostname any other way, try and see if there's one in /etc/hosts
   for this address. If it has a domain part, that must match the set domain and
   it gets stripped. */
char *host_from_dns(struct daemon *daemon, struct in_addr addr)
{
  struct crec *lookup = cache_find_by_addr(NULL, (struct all_addr *)&addr, 0, F_IPV4);
  char *hostname = NULL;
  
  if (lookup && (lookup->flags & F_HOSTS))
    {
      hostname = daemon->dhcp_buff;
      hostname[256] = 0;
      strncpy(hostname, cache_get_name(lookup), 256);
      hostname = strip_hostname(daemon, hostname);
    }

  return hostname;
}

char *strip_hostname(struct daemon *daemon, char *hostname)
{
  char *dot = strchr(hostname, '.');
  if (dot)
    {
      if (!daemon->domain_suffix || !hostname_isequal(dot+1, daemon->domain_suffix))
	{
	  syslog(LOG_WARNING, _("Ignoring DHCP host name %s because it has an illegal domain part"), hostname);
	  hostname = NULL;
	}
      else
	{
	  *dot = 0; /* truncate */
	  if (strlen(hostname) == 0)
	    hostname = NULL; /* nothing left */
	}
    }
  return hostname;
}
