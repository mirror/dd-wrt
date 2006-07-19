/* dnsmasq is Copyright (c) 2000 - 2005 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include "dnsmasq.h"

static struct frec *frec_list = NULL;

static struct frec *get_new_frec(struct daemon *daemon, time_t now);
static struct frec *lookup_frec(unsigned short id);
static struct frec *lookup_frec_by_sender(unsigned short id,
					  union mysockaddr *addr,
					  unsigned int crc);
static unsigned short get_id(void);


/* Send a UDP packet with it's source address set as "source" 
   unless nowild is true, when we just send it with the kernel default */
static void send_from(int fd, int nowild, char *packet, size_t len, 
		      union mysockaddr *to, struct all_addr *source,
		      unsigned int iface)
{
  struct msghdr msg;
  struct iovec iov[1]; 
  union {
    struct cmsghdr align; /* this ensures alignment */
#if defined(HAVE_LINUX_NETWORK)
    char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
#elif defined(IP_SENDSRCADDR)
    char control[CMSG_SPACE(sizeof(struct in_addr))];
#endif
#ifdef HAVE_IPV6
    char control6[CMSG_SPACE(sizeof(struct in6_pktinfo))];
#endif
  } control_u;
  
  iov[0].iov_base = packet;
  iov[0].iov_len = len;

  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;
  msg.msg_name = to;
  msg.msg_namelen = sa_len(to);
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  
  if (!nowild)
    {
      struct cmsghdr *cmptr;
      msg.msg_control = &control_u;
      msg.msg_controllen = sizeof(control_u);
      cmptr = CMSG_FIRSTHDR(&msg);

      if (to->sa.sa_family == AF_INET)
	{
#if defined(HAVE_LINUX_NETWORK)
	  struct in_pktinfo *pkt = (struct in_pktinfo *)CMSG_DATA(cmptr);
	  pkt->ipi_ifindex = 0;
	  pkt->ipi_spec_dst = source->addr.addr4;
	  msg.msg_controllen = cmptr->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
	  cmptr->cmsg_level = SOL_IP;
	  cmptr->cmsg_type = IP_PKTINFO;
#elif defined(IP_SENDSRCADDR)
	  struct in_addr *a = (struct in_addr *)CMSG_DATA(cmptr);
	  *a = source->addr.addr4;
	  msg.msg_controllen = cmptr->cmsg_len = CMSG_LEN(sizeof(struct in_addr));
	  cmptr->cmsg_level = IPPROTO_IP;
	  cmptr->cmsg_type = IP_SENDSRCADDR;
#endif
	}
      else
#ifdef HAVE_IPV6
	{
	  struct in6_pktinfo *pkt = (struct in6_pktinfo *)CMSG_DATA(cmptr);
	  pkt->ipi6_ifindex = iface; /* Need iface for IPv6 to handle link-local addrs */
	  pkt->ipi6_addr = source->addr.addr6;
	  msg.msg_controllen = cmptr->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
	  cmptr->cmsg_type = IPV6_PKTINFO;
	  cmptr->cmsg_level = IPV6_LEVEL;
	}
#else
      iface = 0; /* eliminate warning */
#endif
    }
  
 retry:
  if (sendmsg(fd, &msg, 0) == -1)
    {
      /* certain Linux kernels seem to object to setting the source address in the IPv6 stack
	 by returning EINVAL from sendmsg. In that case, try again without setting the
	 source address, since it will nearly alway be correct anyway.  IPv6 stinks. */
      if (errno == EINVAL && msg.msg_controllen)
	{
	  msg.msg_controllen = 0;
	  goto retry;
	}
      if (retry_send())
	goto retry;
    }
}
          
static unsigned short search_servers(struct daemon *daemon, time_t now, struct all_addr **addrpp, 
				     unsigned short qtype, char *qdomain, int *type, char **domain)
			      
{
  /* If the query ends in the domain in one of our servers, set
     domain to point to that name. We find the largest match to allow both
     domain.org and sub.domain.org to exist. */
  
  unsigned int namelen = strlen(qdomain);
  unsigned int matchlen = 0;
  struct server *serv;
  unsigned short flags = 0;
  
  for (serv = daemon->servers; serv; serv=serv->next)
    /* domain matches take priority over NODOTS matches */
    if ((serv->flags & SERV_FOR_NODOTS) && *type != SERV_HAS_DOMAIN && !strchr(qdomain, '.') && namelen != 0)
      {
	unsigned short sflag = serv->addr.sa.sa_family == AF_INET ? F_IPV4 : F_IPV6; 
	*type = SERV_FOR_NODOTS;
	if (serv->flags & SERV_NO_ADDR)
	  flags = F_NXDOMAIN;
	else if (serv->flags & SERV_LITERAL_ADDRESS) 
	  { 
	    if (sflag & qtype)
	      {
		flags = sflag;
		if (serv->addr.sa.sa_family == AF_INET) 
		  *addrpp = (struct all_addr *)&serv->addr.in.sin_addr;
#ifdef HAVE_IPV6
		else
		  *addrpp = (struct all_addr *)&serv->addr.in6.sin6_addr;
#endif 
	      }
	    else if (!flags)
	      flags = F_NOERR;
	  } 
      }
    else if (serv->flags & SERV_HAS_DOMAIN)
      {
	unsigned int domainlen = strlen(serv->domain);
	char *matchstart = qdomain + namelen - domainlen;
	if (namelen >= domainlen &&
	    hostname_isequal(matchstart, serv->domain) &&
	    domainlen >= matchlen &&
	    (domainlen == 0 || namelen == domainlen || *(serv->domain) == '.' || *(matchstart-1) == '.' ))
	  {
	    unsigned short sflag = serv->addr.sa.sa_family == AF_INET ? F_IPV4 : F_IPV6;
	    *type = SERV_HAS_DOMAIN;
	    *domain = serv->domain;
	    matchlen = domainlen;
	    if (serv->flags & SERV_NO_ADDR)
	      flags = F_NXDOMAIN;
	    else if (serv->flags & SERV_LITERAL_ADDRESS)
	      {
		if ((sflag | F_QUERY ) & qtype)
		  {
		    flags = qtype;
		    if (serv->addr.sa.sa_family == AF_INET) 
		      *addrpp = (struct all_addr *)&serv->addr.in.sin_addr;
#ifdef HAVE_IPV6
		    else
		      *addrpp = (struct all_addr *)&serv->addr.in6.sin6_addr;
#endif
		  }
		else if (!flags)
		  flags = F_NOERR;
	      }
	  } 
      }

  if (flags & ~(F_NOERR | F_NXDOMAIN)) /* flags set here means a literal found */
    {
      if (flags & F_QUERY)
	log_query(F_CONFIG | F_FORWARD | F_NEG, qdomain, NULL, 0, NULL, 0);
      else
	log_query(F_CONFIG | F_FORWARD | flags, qdomain, *addrpp, 0, NULL, 0);
    }
  else if (qtype && (daemon->options & OPT_NODOTS_LOCAL) && !strchr(qdomain, '.') && namelen != 0)
    flags = F_NXDOMAIN;
    
  if (flags == F_NXDOMAIN && check_for_local_domain(qdomain, now, daemon))
    flags = F_NOERR;

  if (flags == F_NXDOMAIN || flags == F_NOERR)
    log_query(F_CONFIG | F_FORWARD | F_NEG | qtype | (flags & F_NXDOMAIN), qdomain, NULL, 0, NULL, 0);

  return  flags;
}

/* returns new last_server */	
static void forward_query(struct daemon *daemon, int udpfd, union mysockaddr *udpaddr,
			  struct all_addr *dst_addr, unsigned int dst_iface,
			  HEADER *header, size_t plen, time_t now, struct frec *forward)
{
  char *domain = NULL;
  int type = 0;
  struct all_addr *addrp = NULL;
  unsigned int crc = questions_crc(header, plen, daemon->namebuff);
  unsigned short flags = 0;
  unsigned short gotname = extract_request(header, plen, daemon->namebuff, NULL);
  struct server *start = NULL;
    
  /* may be no servers available. */
  if (!daemon->servers)
    forward = NULL;
  else if (forward || (forward = lookup_frec_by_sender(ntohs(header->id), udpaddr, crc)))
    {
      /* retry on existing query, send to all available servers  */
      domain = forward->sentto->domain;
      if (!(daemon->options & OPT_ORDER))
	{
	  forward->forwardall = 1;
	  daemon->last_server = NULL;
	}
      type = forward->sentto->flags & SERV_TYPE;
      if (!(start = forward->sentto->next))
	start = daemon->servers; /* at end of list, recycle */
      header->id = htons(forward->new_id);
    }
  else 
    {
      if (gotname)
	flags = search_servers(daemon, now, &addrp, gotname, daemon->namebuff, &type, &domain);
      
      if (!flags && !(forward = get_new_frec(daemon, now)))
	/* table full - server failure. */
	flags = F_NEG;
      
      if (forward)
	{
	  forward->source = *udpaddr;
	  forward->dest = *dst_addr;
	  forward->iface = dst_iface;
	  forward->new_id = get_id();
	  forward->fd = udpfd;
	  forward->orig_id = ntohs(header->id);
	  forward->crc = crc;
	  forward->forwardall = 0;
	  header->id = htons(forward->new_id);

	  /* In strict_order mode, or when using domain specific servers
	     always try servers in the order specified in resolv.conf,
	     otherwise, use the one last known to work. */
	  
	  if (type != 0  || (daemon->options & OPT_ORDER))
	    start = daemon->servers;
	  else if (!(start = daemon->last_server))
	    {
	      start = daemon->servers;
	      forward->forwardall = 1;
	    }
	}
    }

  /* check for send errors here (no route to host) 
     if we fail to send to all nameservers, send back an error
     packet straight away (helps modem users when offline)  */
  
  if (!flags && forward)
    {
      struct server *firstsentto = start;
      int forwarded = 0;

      while (1)
	{ 
	  /* only send to servers dealing with our domain.
	     domain may be NULL, in which case server->domain 
	     must be NULL also. */
	  
	  if (type == (start->flags & SERV_TYPE) &&
	      (type != SERV_HAS_DOMAIN || hostname_isequal(domain, start->domain)) &&
	      !(start->flags & SERV_LITERAL_ADDRESS))
	    {
	      if (sendto(start->sfd->fd, (char *)header, plen, 0,
			 &start->addr.sa,
			 sa_len(&start->addr)) == -1)
		{
		  if (retry_send())
		    continue;
		}
	      else
		{
		  /* Keep info in case we want to re-send this packet */
		  daemon->srv_save = start;
		  daemon->packet_len = plen;
		  
		  if (!gotname)
		    strcpy(daemon->namebuff, "query");
		  if (start->addr.sa.sa_family == AF_INET)
		    log_query(F_SERVER | F_IPV4 | F_FORWARD, daemon->namebuff, 
			      (struct all_addr *)&start->addr.in.sin_addr, 0,
			      NULL, 0); 
#ifdef HAVE_IPV6
		  else
		    log_query(F_SERVER | F_IPV6 | F_FORWARD, daemon->namebuff, 
			      (struct all_addr *)&start->addr.in6.sin6_addr, 0,
			      NULL, 0);
#endif 
		  forwarded = 1;
		  forward->sentto = start;
		  if (!forward->forwardall) 
		    break;
		  forward->forwardall++;
		}
	    } 
	  
	  if (!(start = start->next))
 	    start = daemon->servers;
	  
	  if (start == firstsentto)
	    break;
	}
      
      if (forwarded)
	  return;
      
      /* could not send on, prepare to return */ 
      header->id = htons(forward->orig_id);
      forward->new_id = 0; /* cancel */
    }	  
  
  /* could not send on, return empty answer or address if known for whole domain */
  if (udpfd != -1)
    {
      plen = setup_reply(header, plen, addrp, flags, daemon->local_ttl);
      send_from(udpfd, daemon->options & OPT_NOWILD, (char *)header, plen, udpaddr, dst_addr, dst_iface);
    }

  return;
}

static size_t process_reply(struct daemon *daemon, HEADER *header, time_t now, 
			    unsigned int query_crc, struct server *server, size_t n)
{
  unsigned char *pheader, *sizep;
  int munged = 0;
  size_t plen; 

  /* If upstream is advertising a larger UDP packet size
	 than we allow, trim it so that we don't get overlarge
	 requests for the client. */

  if ((pheader = find_pseudoheader(header, n, &plen, &sizep)))
    {
      unsigned short udpsz;
      unsigned char *psave = sizep;
      
      GETSHORT(udpsz, sizep);
      if (udpsz > daemon->edns_pktsz)
	PUTSHORT(daemon->edns_pktsz, psave);
    }

  if (header->opcode != QUERY || (header->rcode != NOERROR && header->rcode != NXDOMAIN))
    return n;
  
  /* Complain loudly if the upstream server is non-recursive. */
  if (!header->ra && header->rcode == NOERROR && ntohs(header->ancount) == 0 &&
      server && !(server->flags & SERV_WARNED_RECURSIVE))
    {
      prettyprint_addr(&server->addr, daemon->namebuff);
      syslog(LOG_WARNING, _("nameserver %s refused to do a recursive query"), daemon->namebuff);
      if (!(daemon->options & OPT_LOG))
	server->flags |= SERV_WARNED_RECURSIVE;
    }  
    
  if (daemon->bogus_addr && header->rcode != NXDOMAIN &&
      check_for_bogus_wildcard(header, n, daemon->namebuff, daemon->bogus_addr, now))
    {
      munged = 1;
      header->rcode = NXDOMAIN;
      header->aa = 0;
    }
  else 
    {
      if (header->rcode == NXDOMAIN && 
	  extract_request(header, n, daemon->namebuff, NULL) &&
	  check_for_local_domain(daemon->namebuff, now, daemon))
	{
	  /* if we forwarded a query for a locally known name (because it was for 
	     an unknown type) and the answer is NXDOMAIN, convert that to NODATA,
	     since we know that the domain exists, even if upstream doesn't */
	  munged = 1;
	  header->aa = 1;
	  header->rcode = NOERROR;
	}
  
      /* If the crc of the question section doesn't match the crc we sent, then
	 someone might be attempting to insert bogus values into the cache by 
	 sending replies containing questions and bogus answers. */
      if (query_crc == questions_crc(header, n, daemon->namebuff))
	extract_addresses(header, n, daemon->namebuff, now, daemon);
    }
  
  /* do this after extract_addresses. Ensure NODATA reply and remove
     nameserver info. */
  
  if (munged)
    {
      header->ancount = htons(0);
      header->nscount = htons(0);
      header->arcount = htons(0);
    }
  
  /* the bogus-nxdomain stuff, doctor and NXDOMAIN->NODATA munging can all elide
     sections of the packet. Find the new length here and put back pseudoheader
     if it was removed. */
  return resize_packet(header, n, pheader, plen);
}

/* sets new last_server */
void reply_query(struct serverfd *sfd, struct daemon *daemon, time_t now)
{
  /* packet from peer server, extract data for cache, and send to
     original requester */
  struct frec *forward;
  HEADER *header;
  union mysockaddr serveraddr;
  socklen_t addrlen = sizeof(serveraddr);
  ssize_t n = recvfrom(sfd->fd, daemon->packet, daemon->edns_pktsz, 0, &serveraddr.sa, &addrlen);
  size_t nn;

  /* packet buffer overwritten */
  daemon->srv_save = NULL;

  /* Determine the address of the server replying  so that we can mark that as good */
  serveraddr.sa.sa_family = sfd->source_addr.sa.sa_family;
#ifdef HAVE_IPV6
  if (serveraddr.sa.sa_family == AF_INET6)
    serveraddr.in6.sin6_flowinfo = 0;
#endif
  
  header = (HEADER *)daemon->packet;
  forward = lookup_frec(ntohs(header->id));
  
  if (n >= (int)sizeof(HEADER) && header->qr && forward)
    {
       struct server *server = forward->sentto;
       
       if ((header->rcode == SERVFAIL || header->rcode == REFUSED) && forward->forwardall == 0)
	 /* for broken servers, attempt to send to another one. */
	 {
	   unsigned char *pheader;
	   size_t plen;
	   
	   /* recreate query from reply */
	   pheader = find_pseudoheader(header, (size_t)n, &plen, NULL);
	   header->ancount = htons(0);
	   header->nscount = htons(0);
	   header->arcount = htons(0);
	   if ((nn = resize_packet(header, (size_t)n, pheader, plen)))
	     {
	       forward->forwardall = 1;
	       header->qr = 0;
	       header->tc = 0;
	       forward_query(daemon, -1, NULL, NULL, 0, header, nn, now, forward);
	       return;
	     }
	 }   

       if ((forward->sentto->flags & SERV_TYPE) == 0)
	 {
	   if (header->rcode == SERVFAIL || header->rcode == REFUSED)
	     server = NULL;
	   else
	     {
	       struct server *last_server;
	       /* find good server by address if possible, otherwise assume the last one we sent to */ 
	       for (last_server = daemon->servers; last_server; last_server = last_server->next)
		 if (!(last_server->flags & (SERV_LITERAL_ADDRESS | SERV_HAS_DOMAIN | SERV_FOR_NODOTS | SERV_NO_ADDR)) &&
		     sockaddr_isequal(&last_server->addr, &serveraddr))
		   {
		     server = last_server;
		     break;
		   }
	     } 
	   daemon->last_server = server;
	 }
	         
      /* If the answer is an error, keep the forward record in place in case
	 we get a good reply from another server. Kill it when we've
	 had replies from all to avoid filling the forwarding table when
	 everything is broken */
      if (forward->forwardall == 0 || --forward->forwardall == 1 || 
	  (header->rcode != REFUSED && header->rcode != SERVFAIL))
	{
	  if ((nn = process_reply(daemon, header, now, forward->crc, server, (size_t)n)))
	    {
	      header->id = htons(forward->orig_id);
	      header->ra = 1; /* recursion if available */
	      send_from(forward->fd, daemon->options & OPT_NOWILD, daemon->packet, nn, 
			&forward->source, &forward->dest, forward->iface);
	    }
	  forward->new_id = 0; /* cancel */
	}
    }
}

void receive_query(struct listener *listen, struct daemon *daemon, time_t now)
{
  HEADER *header = (HEADER *)daemon->packet;
  union mysockaddr source_addr;
  unsigned short type;
  struct all_addr dst_addr;
  struct in_addr netmask, dst_addr_4;
  size_t m;
  ssize_t n;
  int if_index = 0;
  struct iovec iov[1];
  struct msghdr msg;
  struct cmsghdr *cmptr;
  union {
    struct cmsghdr align; /* this ensures alignment */
#ifdef HAVE_IPV6
    char control6[CMSG_SPACE(sizeof(struct in6_pktinfo))];
#endif
#if defined(HAVE_LINUX_NETWORK)
    char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
#elif defined(IP_RECVDSTADDR)
    char control[CMSG_SPACE(sizeof(struct in_addr)) +
		 CMSG_SPACE(sizeof(struct sockaddr_dl))];
#endif
  } control_u;
  
  /* packet buffer overwritten */
  daemon->srv_save = NULL;
  
  if (listen->family == AF_INET && (daemon->options & OPT_NOWILD))
    {
      dst_addr_4 = listen->iface->addr.in.sin_addr;
      netmask = listen->iface->netmask;
    }
  else
    {
      dst_addr_4.s_addr = 0;
      netmask.s_addr = 0;
    }

  iov[0].iov_base = daemon->packet;
  iov[0].iov_len = daemon->edns_pktsz;
    
  msg.msg_control = control_u.control;
  msg.msg_controllen = sizeof(control_u);
  msg.msg_flags = 0;
  msg.msg_name = &source_addr;
  msg.msg_namelen = sizeof(source_addr);
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  
  if ((n = recvmsg(listen->fd, &msg, 0)) == -1)
    return;
  
  if (n < (int)sizeof(HEADER) || 
      (msg.msg_flags & MSG_TRUNC) ||
      header->qr)
    return;
  
  source_addr.sa.sa_family = listen->family;
#ifdef HAVE_IPV6
  if (listen->family == AF_INET6)
    source_addr.in6.sin6_flowinfo = 0;
#endif
  
  if (!(daemon->options & OPT_NOWILD))
    {
      struct ifreq ifr;

      if (msg.msg_controllen < sizeof(struct cmsghdr))
	return;

#if defined(HAVE_LINUX_NETWORK)
      if (listen->family == AF_INET)
	for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
	  if (cmptr->cmsg_level == SOL_IP && cmptr->cmsg_type == IP_PKTINFO)
	    {
	      dst_addr_4 = dst_addr.addr.addr4 = ((struct in_pktinfo *)CMSG_DATA(cmptr))->ipi_spec_dst;
	      if_index = ((struct in_pktinfo *)CMSG_DATA(cmptr))->ipi_ifindex;
	    }
#elif defined(IP_RECVDSTADDR) && defined(IP_RECVIF)
      if (listen->family == AF_INET)
	{
	  for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
	    if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_RECVDSTADDR)
	      dst_addr_4 = dst_addr.addr.addr4 = *((struct in_addr *)CMSG_DATA(cmptr));
	    else if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_RECVIF)
	      if_index = ((struct sockaddr_dl *)CMSG_DATA(cmptr))->sdl_index;
	}
#endif
      
#ifdef HAVE_IPV6
      if (listen->family == AF_INET6)
	{
	  for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
	    if (cmptr->cmsg_level == IPV6_LEVEL && cmptr->cmsg_type == IPV6_PKTINFO)
	      {
		dst_addr.addr.addr6 = ((struct in6_pktinfo *)CMSG_DATA(cmptr))->ipi6_addr;
		if_index =((struct in6_pktinfo *)CMSG_DATA(cmptr))->ipi6_ifindex;
	      }
	}
#endif
      
      /* enforce available interface configuration */
      
      if (if_index == 0)
	return;
      
      if (daemon->if_except || daemon->if_names || (daemon->options & OPT_LOCALISE))
	{
#ifdef SIOCGIFNAME
	  ifr.ifr_ifindex = if_index;
	  if (ioctl(listen->fd, SIOCGIFNAME, &ifr) == -1)
	    return;
#else
	  if (!if_indextoname(if_index, ifr.ifr_name))
	    return;
#endif

	  if (listen->family == AF_INET &&
	      (daemon->options & OPT_LOCALISE) &&
	      ioctl(listen->fd, SIOCGIFNETMASK, &ifr) == -1)
	    return;

	  netmask = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr;
	}

      if (!iface_check(daemon, listen->family, &dst_addr, ifr.ifr_name))
	return;
    }
  
  if (extract_request(header, (size_t)n, daemon->namebuff, &type))
    {
      if (listen->family == AF_INET) 
	log_query(F_QUERY | F_IPV4 | F_FORWARD, daemon->namebuff, 
		  (struct all_addr *)&source_addr.in.sin_addr, type, NULL, 0);
#ifdef HAVE_IPV6
      else
	log_query(F_QUERY | F_IPV6 | F_FORWARD, daemon->namebuff, 
		  (struct all_addr *)&source_addr.in6.sin6_addr, type, NULL, 0);
#endif
    }

  m = answer_request (header, ((char *) header) + PACKETSZ, (size_t)n, daemon, 
		      dst_addr_4, netmask, now);
  if (m >= 1)
    send_from(listen->fd, daemon->options & OPT_NOWILD, (char *)header, m, &source_addr, &dst_addr, if_index);
  else
    forward_query(daemon, listen->fd, &source_addr, &dst_addr, if_index,
		  header, (size_t)n, now, NULL);
}

static int read_write(int fd, unsigned char *packet, int size, int rw)
{
  ssize_t n, done;
  
  for (done = 0; done < size; done += n)
    {
    retry:
      if (rw)
	n = read(fd, &packet[done], (size_t)(size - done));
      else
	n = write(fd, &packet[done], (size_t)(size - done));

      if (n == 0)
	return 0;
      else if (n == -1)
	{
	  if (errno == EINTR)
	    goto retry;
	  else
	    return 0;
	}
    }
  return 1;
}
  
/* The daemon forks before calling this: it should deal with one connection,
   blocking as neccessary, and then return. Note, need to be a bit careful
   about resources for debug mode, when the fork is suppressed: that's
   done by the caller. */
unsigned char *tcp_request(struct daemon *daemon, int confd, time_t now,
			   struct in_addr local_addr, struct in_addr netmask)
{
  int size = 0;
  size_t m;
  unsigned short qtype, gotname;
  unsigned char c1, c2;
  /* Max TCP packet + slop */
  unsigned char *packet = malloc(65536 + MAXDNAME + RRFIXEDSZ);
  HEADER *header;
  struct server *last_server;
  
  while (1)
    {
      if (!packet ||
	  !read_write(confd, &c1, 1, 1) || !read_write(confd, &c2, 1, 1) ||
	  !(size = c1 << 8 | c2) ||
	  !read_write(confd, packet, size, 1))
       	return packet; 
  
      if (size < (int)sizeof(HEADER))
	continue;
      
      header = (HEADER *)packet;
      
      if ((gotname = extract_request(header, (unsigned int)size, daemon->namebuff, &qtype)))
	{
	  union mysockaddr peer_addr;
	  socklen_t peer_len = sizeof(union mysockaddr);
	  
	  if (getpeername(confd, (struct sockaddr *)&peer_addr, &peer_len) != -1)
	    {
	      if (peer_addr.sa.sa_family == AF_INET) 
		log_query(F_QUERY | F_IPV4 | F_FORWARD, daemon->namebuff, 
			  (struct all_addr *)&peer_addr.in.sin_addr, qtype, NULL, 0);
#ifdef HAVE_IPV6
	      else
		log_query(F_QUERY | F_IPV6 | F_FORWARD, daemon->namebuff, 
			  (struct all_addr *)&peer_addr.in6.sin6_addr, qtype, NULL, 0);
#endif
	    }
	}
      
      /* m > 0 if answered from cache */
      m = answer_request(header, ((char *) header) + 65536, (unsigned int)size, daemon, 
			 local_addr, netmask, now);
      
      if (m == 0)
	{
	  unsigned short flags = 0;
	  struct all_addr *addrp = NULL;
	  int type = 0;
	  char *domain = NULL;
	  
	  if (gotname)
	    flags = search_servers(daemon, now, &addrp, gotname, daemon->namebuff, &type, &domain);
	  
	  if (type != 0  || (daemon->options & OPT_ORDER) || !daemon->last_server)
	    last_server = daemon->servers;
	  else
	    last_server = daemon->last_server;
      
	  if (!flags && last_server)
	    {
	      struct server *firstsendto = NULL;
	      unsigned int crc = questions_crc(header, (unsigned int)size, daemon->namebuff);

	      /* Loop round available servers until we succeed in connecting to one.
	         Note that this code subtley ensures that consecutive queries on this connection
	         which can go to the same server, do so. */
	      while (1) 
		{
		  if (!firstsendto)
		    firstsendto = last_server;
		  else
		    {
		      if (!(last_server = last_server->next))
			last_server = daemon->servers;
		      
		      if (last_server == firstsendto)
			break;
		    }
	      
		  /* server for wrong domain */
		  if (type != (last_server->flags & SERV_TYPE) ||
		      (type == SERV_HAS_DOMAIN && !hostname_isequal(domain, last_server->domain)))
		    continue;
		  
		  if ((last_server->tcpfd == -1) &&
		      (last_server->tcpfd = socket(last_server->addr.sa.sa_family, SOCK_STREAM, 0)) != -1 &&
		      connect(last_server->tcpfd, &last_server->addr.sa, sa_len(&last_server->addr)) == -1)
		    {
		      close(last_server->tcpfd);
		      last_server->tcpfd = -1;
		    }
		  
		  if (last_server->tcpfd == -1)	
		    continue;
		  
		  c1 = size >> 8;
		  c2 = size;
		  
		  if (!read_write(last_server->tcpfd, &c1, 1, 0) ||
		      !read_write(last_server->tcpfd, &c2, 1, 0) ||
		      !read_write(last_server->tcpfd, packet, size, 0) ||
		      !read_write(last_server->tcpfd, &c1, 1, 1) ||
		      !read_write(last_server->tcpfd, &c2, 1, 1))
		    {
		      close(last_server->tcpfd);
		      last_server->tcpfd = -1;
		      continue;
		    } 
	      
		  m = (c1 << 8) | c2;
		  if (!read_write(last_server->tcpfd, packet, m, 1))
		    return packet;
		  
		  if (!gotname)
		    strcpy(daemon->namebuff, "query");
		  if (last_server->addr.sa.sa_family == AF_INET)
		    log_query(F_SERVER | F_IPV4 | F_FORWARD, daemon->namebuff, 
			      (struct all_addr *)&last_server->addr.in.sin_addr, 0, NULL, 0); 
#ifdef HAVE_IPV6
		  else
		    log_query(F_SERVER | F_IPV6 | F_FORWARD, daemon->namebuff, 
			      (struct all_addr *)&last_server->addr.in6.sin6_addr, 0, NULL, 0);
#endif 
		  
		  /* There's no point in updating the cache, since this process will exit and
		     lose the information after one query. We make this call for the alias and 
		     bogus-nxdomain side-effects. */
		  m = process_reply(daemon, header, now, crc, last_server, (unsigned int)m);
		  
		  break;
		}
	    }
	  
	  /* In case of local answer or no connections made. */
	  if (m == 0)
	    m = setup_reply(header, (unsigned int)size, addrp, flags, daemon->local_ttl);
	}
      
      c1 = m>>8;
      c2 = m;
      if (!read_write(confd, &c1, 1, 0) ||
	  !read_write(confd, &c2, 1, 0) || 
	  !read_write(confd, packet, m, 0))
	return packet;
    }
}

static struct frec *get_new_frec(struct daemon *daemon, time_t now)
{
  struct frec *f = frec_list, *oldest = NULL;
  time_t oldtime = now;
  int count = 0;
  static time_t warntime = 0;

  while (f)
    {
      if (f->new_id == 0)
	{
	  f->time = now;
	  return f;
	}

      if (difftime(f->time, oldtime) <= 0)
	{
	  oldtime = f->time;
	  oldest = f;
	}

      count++;
      f = f->next;
    }
  
  /* can't find empty one, use oldest if there is one
     and it's older than timeout */
  if (oldest && difftime(now, oldtime)  > TIMEOUT)
    { 
      oldest->time = now;
      return oldest;
    }
  
  if (count > daemon->ftabsize)
    { /* limit logging rate so syslog isn't DOSed either */
      if (!warntime || difftime(now, warntime) > LOGRATE)
	{
	  warntime = now;
	  syslog(LOG_WARNING, _("forwarding table overflow: check for server loops."));
	}
      return NULL;
    }

  if ((f = (struct frec *)malloc(sizeof(struct frec))))
    {
      f->next = frec_list;
      f->time = now;
      f->new_id = 0;
      frec_list = f;
    }
  return f; /* OK if malloc fails and this is NULL */
}
 
static struct frec *lookup_frec(unsigned short id)
{
  struct frec *f;

  for(f = frec_list; f; f = f->next)
    if (f->new_id == id)
      return f;
      
  return NULL;
}

static struct frec *lookup_frec_by_sender(unsigned short id,
					  union mysockaddr *addr,
					  unsigned int crc)
{
  struct frec *f;
  
  for(f = frec_list; f; f = f->next)
    if (f->new_id &&
	f->orig_id == id && 
	f->crc == crc &&
	sockaddr_isequal(&f->source, addr))
      return f;
   
  return NULL;
}

/* A server record is going away, remove references to it */
void server_gone(struct daemon *daemon, struct server *server)
{
  struct frec *f;
  
  for (f = frec_list; f; f = f->next)
    if (f->new_id != 0 && f->sentto == server)
      f->new_id = 0;
  
  if (daemon->last_server == server)
    daemon->last_server = NULL;

  if (daemon->srv_save == server)
    daemon->srv_save = NULL;
}

/* return unique random ids between 1 and 65535 */
static unsigned short get_id(void)
{
  unsigned short ret = 0;

  while (ret == 0)
    {
      ret = rand16();
      
      /* scrap ids already in use */
      if ((ret != 0) && lookup_frec(ret))
	ret = 0;
    }

  return ret;
}





