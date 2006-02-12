/* dnsmasq is Copyright (c) 2000 - 2005 Simon Kelley

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

static int iface_allowed(struct daemon *daemon, struct irec *iface, 
			 char *name, int is_loopback, union mysockaddr *addr) 
{
  struct iname *tmp;
  
  /* If we are restricting the set of interfaces to use, make
     sure that loopback interfaces are in that set. */
  if (daemon->if_names && is_loopback)
    {
      struct iname *lo;
      for (lo = daemon->if_names; lo; lo = lo->next)
	if (lo->name && strcmp(lo->name, name) == 0)
	  {
	    lo->isloop = 1;
	    break;
	  }
      if (!lo)
	{
	  lo = safe_malloc(sizeof(struct iname));
	  lo->name = safe_malloc(strlen(name)+1);
	  strcpy(lo->name, name);
	  lo->isloop = lo->used = 1;
	  lo->next = daemon->if_names;
	  daemon->if_names = lo;
	}
    }
  
  /* check blacklist */
  if (daemon->if_except)
    for (tmp = daemon->if_except; tmp; tmp = tmp->next)
      if (tmp->name && strcmp(tmp->name, name) == 0)
	return 0;
	
  /* we may need to check the whitelist */
  if (daemon->if_names || daemon->if_addrs)
    { 
      int found = 0;

      for (tmp = daemon->if_names; tmp; tmp = tmp->next)
	if (tmp->name && (strcmp(tmp->name, name) == 0))
	  found = tmp->used = 1;
	  
      for (tmp = daemon->if_addrs; tmp; tmp = tmp->next)
	if (sockaddr_isequal(&tmp->addr, addr))
	  found = tmp->used = 1;
      
      if (!found) 
	return 0;
    }
  
  /* check whether the interface IP has been added already 
     it is possible to have multiple interfaces with the same address */
  for (; iface; iface = iface->next) 
    if (sockaddr_isequal(&iface->addr, addr))
      break;
  if (iface)
    return 0;
  
  return 1;
}

/* This does two different jobs: if chainp is non-NULL, it puts
   a list of all the interfaces allowed by config into *chainp.
   If chainp is NULL, it returns 1 if addr is  an address of an interface
   allowed by config and if that address is IPv4, it fills in the
   netmask of the interface. 
   
   If chainp is non-NULL, a zero return indicates a fatal error.

   If chainp is NULL, errors result in a match failure and zero return.
*/
int enumerate_interfaces(struct daemon *daemon, struct irec **chainp,
			 union mysockaddr *test_addrp, struct in_addr *netmaskp)
{
#if defined(HAVE_LINUX_IPV6_PROC) && defined(HAVE_IPV6)
  FILE *f;
#endif
  union mysockaddr addr;
  struct irec *iface = NULL;
  char *buf, *ptr;
  struct ifreq *ifr = NULL;
  struct ifconf ifc;
  int lastlen = 0;
  int len = 20 * sizeof(struct ifreq);
  int fd = socket(PF_INET, SOCK_DGRAM, 0);
  struct in_addr netmask;
  int ret = 0;
  
  netmask.s_addr = 0; /* eliminate warning */

  if (fd == -1)
    return 0;

#ifdef HAVE_IPV6
  if (test_addrp && test_addrp->sa.sa_family == AF_INET6)
    test_addrp->in6.sin6_flowinfo = htonl(0);
#endif
        
  while (1)
     {
       buf = safe_malloc(len);

       ifc.ifc_len = len;
       ifc.ifc_buf = buf;
       if (ioctl(fd, SIOCGIFCONF, &ifc) < 0)
	 {
	   if (errno != EINVAL || lastlen != 0)
	     goto exit;
	 }
       else
	 {
	   if (ifc.ifc_len == lastlen)
	     break; /* got a big enough buffer now */
	   lastlen = ifc.ifc_len;
	 }
       len += 10*sizeof(struct ifreq);
       free(buf);
     }
  
  for (ptr = buf; ptr < buf + ifc.ifc_len; )
    {
#ifdef HAVE_SOCKADDR_SA_LEN
      /* subsequent entries may not be aligned, so copy into
	 an aligned buffer to avoid nasty complaints about 
	 unaligned accesses. */
      int ifr_len = ((struct ifreq *)ptr)->ifr_addr.sa_len + IF_NAMESIZE;
      if (!(ifr = realloc(ifr, ifr_len)))
	goto exit;
      
      memcpy(ifr, ptr, ifr_len);
      ptr += ifr_len;
#else
      ifr = (struct ifreq *)ptr;
      ptr += sizeof(struct ifreq);
#endif
      
      /* copy address since getting flags overwrites */
      if (ifr->ifr_addr.sa_family == AF_INET)
	{
	  addr.in = *((struct sockaddr_in *) &ifr->ifr_addr);
	  addr.in.sin_port = htons(daemon->port);
	  if (ioctl(fd, SIOCGIFNETMASK, ifr) == -1)
	    goto exit;
	  netmask = ((struct sockaddr_in *) &ifr->ifr_addr)->sin_addr;
	}
#ifdef HAVE_IPV6
      else if (ifr->ifr_addr.sa_family == AF_INET6)
	{
#ifdef HAVE_BROKEN_SOCKADDR_IN6
	  addr.in6 = *((struct my_sockaddr_in6 *) &ifr->ifr_addr);
#else
	  addr.in6 = *((struct sockaddr_in6 *) &ifr->ifr_addr);
#endif
	  addr.in6.sin6_port = htons(daemon->port);
	  addr.in6.sin6_flowinfo = htonl(0);
	}
#endif
      else
	continue; /* unknown address family */
      
      if (ioctl(fd, SIOCGIFFLAGS, ifr) < 0)
	goto exit;

      if (iface_allowed(daemon, iface, ifr->ifr_name, ifr->ifr_flags & IFF_LOOPBACK, &addr))
	{
	  if (chainp)
	    {
	      struct irec *new = safe_malloc(sizeof(struct irec));
	      new->addr = addr;
	      new->netmask = netmask;
	      new->next = iface;
	      iface = new;
	    }
	  else if (sockaddr_isequal(&addr, test_addrp))
	    {
	      *netmaskp = netmask;
	      ret = 1;
	      goto exit;
	    }
	}
    }

#if defined(HAVE_LINUX_IPV6_PROC) && defined(HAVE_IPV6)
  /* IPv6 addresses don't seem to work with SIOCGIFCONF. Barf */
  /* This code snarfed from net-tools 1.60 and certainly linux specific, though
     it shouldn't break on other Unices, and their SIOGIFCONF might work. */
  if ((f = fopen(IP6INTERFACES, "r")))
    {
      unsigned int plen, scope, flags, if_idx;
      char devname[21], addrstring[33];
      
      while (fscanf(f, "%32s %x %x %x %x %20s\n",
		    addrstring, &if_idx, &plen, &scope, &flags, devname) != EOF) 
	{
	  int i;
	  struct ifreq sifr;
	  unsigned char *addr6p = (unsigned char *) &addr.in6.sin6_addr;
	  memset(&addr, 0, sizeof(addr));
	  addr.sa.sa_family = AF_INET6;
	  for (i=0; i<16; i++)
	    {
	      unsigned int byte;
	      sscanf(addrstring+i+i, "%02x", &byte);
	      addr6p[i] = byte;
	    }
	  addr.in6.sin6_port = htons(daemon->port);
	  addr.in6.sin6_flowinfo = htonl(0);
	  addr.in6.sin6_scope_id = htonl(scope);
	  
	  strncpy(sifr.ifr_name, devname, IF_NAMESIZE);
	  if (ioctl(fd, SIOCGIFFLAGS, &sifr) < 0)
	    goto exit;
	  
	  if (iface_allowed(daemon, iface, sifr.ifr_name, sifr.ifr_flags & IFF_LOOPBACK, &addr))
	    {
	      if (chainp)
		{
		  struct irec *new = safe_malloc(sizeof(struct irec));
		  new->addr = addr;
		  new->next = iface;
		  iface = new;
		}
	      else if (sockaddr_isequal(&addr, test_addrp))
		{
		  ret = 1;
		  goto exit;
		}
	    }
	}	    
      fclose(f);
    }
#endif /* LINUX */
  
  if (chainp)
    {
      *chainp = iface;
      ret = 1;
    }
 
 exit:  
  if (buf)
    free(buf);
#ifdef HAVE_SOCKADDR_SA_LEN
  if (ifr)
    free(ifr);
#endif
  close(fd);

  return ret;
}

#if defined(HAVE_IPV6) && (defined(IP_PKTINFO) || (defined(IP_RECVDSTADDR) && defined(IP_RECVIF) && defined(IP_SENDSRCADDR)))
static int create_ipv6_listener(struct listener **link, int port)
{
  union mysockaddr addr;
  int tcpfd, fd, flags, save;
  struct listener *l;
  int opt = 1;

  addr.in6.sin6_family = AF_INET6;
  addr.in6.sin6_addr = in6addr_any;
  addr.in6.sin6_port = htons(port);
  addr.in6.sin6_flowinfo = htonl(0);
#ifdef HAVE_SOCKADDR_SA_LEN
  addr.in6.sin6_len = sizeof(struct sockaddr_in6);
#endif

  /* No error of the kernel doesn't support IPv6 */
  if ((fd = socket(AF_INET6, SOCK_DGRAM, 0)) == -1)
    return (errno == EPROTONOSUPPORT ||
	    errno == EAFNOSUPPORT ||
	    errno == EINVAL);
  
  if ((tcpfd = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
    {
      save = errno;
      close(fd);
      errno = save;
      return 0;
    }
  
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1 ||
      setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1 ||
      setsockopt(fd, IPV6_LEVEL, IPV6_V6ONLY, &opt, sizeof(opt)) == -1 ||
      setsockopt(tcpfd, IPV6_LEVEL, IPV6_V6ONLY, &opt, sizeof(opt)) == -1 ||
      (flags = fcntl(fd, F_GETFL, 0)) == -1 ||
      fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1 ||
      (flags = fcntl(tcpfd, F_GETFL, 0)) == -1 ||
      fcntl(tcpfd, F_SETFL, flags | O_NONBLOCK) == -1 ||
#ifdef IPV6_RECVPKTINFO
      setsockopt(fd, IPV6_LEVEL, IPV6_RECVPKTINFO, &opt, sizeof(opt)) == -1 ||
#else
      setsockopt(fd, IPV6_LEVEL, IPV6_PKTINFO, &opt, sizeof(opt)) == -1 ||
#endif
      bind(tcpfd, (struct sockaddr *)&addr, sa_len(&addr)) == -1 ||
      listen(tcpfd, 5) == -1 ||
      bind(fd, (struct sockaddr *)&addr, sa_len(&addr)) == -1) 
    {
      save = errno;
      close(fd);
      close(tcpfd);
      errno = save;
      return 0;
    }
  
  l = safe_malloc(sizeof(struct listener));
  l->fd = fd;
  l->tcpfd = tcpfd;
  l->family = AF_INET6;
  l->next = NULL;
  *link = l;
  
  return 1;
}
#endif

struct listener *create_wildcard_listeners(int port)
{
#if !(defined(IP_PKTINFO) || (defined(IP_RECVDSTADDR) && defined(IP_RECVIF) && defined(IP_SENDSRCADDR)))
  port = 0; /* eliminate warning */
  return NULL;
#else
  union mysockaddr addr;
  int opt = 1;
  struct listener *l, *l6 = NULL;
  int flags;
  int tcpfd, fd;

  addr.in.sin_family = AF_INET;
  addr.in.sin_addr.s_addr = INADDR_ANY;
  addr.in.sin_port = htons(port);
#ifdef HAVE_SOCKADDR_SA_LEN
  addr.in.sin_len = sizeof(struct sockaddr_in);
#endif

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    return NULL;
  
  if ((tcpfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      close (fd);
      return NULL;
    }
  
  if (setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1 ||
      bind(tcpfd, (struct sockaddr *)&addr, sa_len(&addr)) == -1 ||
      listen(tcpfd, 5) == -1 ||
      (flags = fcntl(tcpfd, F_GETFL, 0)) == -1 ||
      fcntl(tcpfd, F_SETFL, flags | O_NONBLOCK) == -1 ||
#ifdef HAVE_IPV6
      !create_ipv6_listener(&l6, port) ||
#endif
      setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1 ||
      (flags = fcntl(fd, F_GETFL, 0)) == -1 ||
      fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1 ||
#if defined(IP_PKTINFO) 
      setsockopt(fd, SOL_IP, IP_PKTINFO, &opt, sizeof(opt)) == -1 ||
#elif defined(IP_RECVDSTADDR) && defined(IP_RECVIF)
      setsockopt(fd, IPPROTO_IP, IP_RECVDSTADDR, &opt, sizeof(opt)) == -1 ||
      setsockopt(fd, IPPROTO_IP, IP_RECVIF, &opt, sizeof(opt)) == -1 ||
#endif 
      bind(fd, (struct sockaddr *)&addr, sa_len(&addr)) == -1)
    {
      close(fd);
      close(tcpfd);
      return NULL;
    }
  
  l = safe_malloc(sizeof(struct listener));
  l->family = AF_INET;
  l->fd = fd;
  l->tcpfd = tcpfd;
  l->next = l6;

  return l;

#endif
}

struct listener *create_bound_listeners(struct irec *interfaces, int port)
{

  struct listener *listeners = NULL;
  struct irec *iface;
  int flags = port, opt = 1;
  
  for (iface = interfaces ;iface; iface = iface->next)
    {
      struct listener *new = safe_malloc(sizeof(struct listener));
      new->family = iface->addr.sa.sa_family;
      new->iface = iface;
      new->next = listeners;
      if ((new->tcpfd = socket(iface->addr.sa.sa_family, SOCK_STREAM, 0)) == -1 ||
	  (new->fd = socket(iface->addr.sa.sa_family, SOCK_DGRAM, 0)) == -1 ||
	  setsockopt(new->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1 ||
	  setsockopt(new->tcpfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1 ||
	  /* See Stevens 16.6 */
	  (flags = fcntl(new->tcpfd, F_GETFL, 0)) == -1 ||
	  fcntl(new->tcpfd, F_SETFL, flags | O_NONBLOCK) == -1 ||
	  (flags = fcntl(new->fd, F_GETFL, 0)) == -1 ||
	  fcntl(new->fd, F_SETFL, flags | O_NONBLOCK) == -1)
	die(_("failed to create listening socket: %s"), NULL);
      
#ifdef HAVE_IPV6
      if (iface->addr.sa.sa_family == AF_INET6)
	{
	  if (setsockopt(new->fd, IPV6_LEVEL, IPV6_V6ONLY, &opt, sizeof(opt)) == -1 ||
	      setsockopt(new->tcpfd, IPV6_LEVEL, IPV6_V6ONLY, &opt, sizeof(opt)) == -1)
	    die(_("failed to set IPV6 options on listening socket: %s"), NULL);
	}
#endif
      
      if (bind(new->tcpfd, &iface->addr.sa, sa_len(&iface->addr)) == -1 ||
	  bind(new->fd, &iface->addr.sa, sa_len(&iface->addr)) == -1)
	{
#ifdef HAVE_IPV6
	  if (iface->addr.sa.sa_family == AF_INET6 && errno == ENODEV)
	    {
	      close(new->tcpfd);
	      close(new->fd);
	      free(new);
	    }
	  else
#endif
	    {
	      char addrbuff[ADDRSTRLEN];
	      prettyprint_addr(&iface->addr, addrbuff);
	      die(_("failed to bind listening socket for %s: %s"), addrbuff);
	    }
	}
      else
	 {
	   listeners = new;     
	   if (listen(new->tcpfd, 5) == -1)
	     die(_("failed to listen on socket: %s"), NULL);
	 }
    }
  
  return listeners;
}

struct serverfd *allocate_sfd(union mysockaddr *addr, struct serverfd **sfds)
{
  struct serverfd *sfd;
  int flags;

  /* may have a suitable one already */
  for (sfd = *sfds; sfd; sfd = sfd->next )
    if (sockaddr_isequal(&sfd->source_addr, addr))
      return sfd;
  
  /* need to make a new one. */
  errno = ENOMEM; /* in case malloc fails. */
  if (!(sfd = malloc(sizeof(struct serverfd))))
    return NULL;
  
  if ((sfd->fd = socket(addr->sa.sa_family, SOCK_DGRAM, 0)) == -1)
    {
      free(sfd);
      return NULL;
    }
  
  if (bind(sfd->fd, (struct sockaddr *)addr, sa_len(addr)) == -1 ||
      (flags = fcntl(sfd->fd, F_GETFL, 0)) == -1 ||
      fcntl(sfd->fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
      int errsave = errno; /* save error from bind. */
      close(sfd->fd);
      free(sfd);
      errno = errsave;
      return NULL;
    }
  
  sfd->source_addr = *addr;
  sfd->next = *sfds;
  *sfds = sfd;
  
  return sfd;
}

void check_servers(struct daemon *daemon)
{
  struct irec *iface;
  struct server *new, *tmp, *ret = NULL;
  int port = 0;

  /* forward table rules reference servers, so have to blow them away */
  forward_init(0);
  
  daemon->last_server = NULL;
  
  for (new = daemon->servers; new; new = tmp)
    {
      tmp = new->next;
      
      if (!(new->flags & (SERV_LITERAL_ADDRESS | SERV_NO_ADDR)))
	{
	  port = prettyprint_addr(&new->addr, daemon->namebuff);

	  for (iface = daemon->interfaces; iface; iface = iface->next)
	    if (sockaddr_isequal(&new->addr, &iface->addr))
	      break;
	  if (iface)
	    {
	      syslog(LOG_WARNING, _("ignoring nameserver %s - local interface"), daemon->namebuff);
	      free(new);
	      continue;
	    }
	  
	  /* Do we need a socket set? */
	  if (!new->sfd && !(new->sfd = allocate_sfd(&new->source_addr, &daemon->sfds)))
	    {
	      syslog(LOG_WARNING, 
		     _("ignoring nameserver %s - cannot make/bind socket: %m"), daemon->namebuff);
	      free(new);
	      continue;
	    }
	}
      
      /* reverse order - gets it right. */
      new->next = ret;
      ret = new;
      
      if (new->flags & (SERV_HAS_DOMAIN | SERV_FOR_NODOTS))
	{
	  char *s1, *s2;
	  if (new->flags & SERV_HAS_DOMAIN)
	    s1 = _("domain"), s2 = new->domain;
	  else
	    s1 = _("unqualified"), s2 = _("domains");
	  
	  if (new->flags & SERV_NO_ADDR)
	    syslog(LOG_INFO, _("using local addresses only for %s %s"), s1, s2);
	  else if (!(new->flags & SERV_LITERAL_ADDRESS))
	    syslog(LOG_INFO, _("using nameserver %s#%d for %s %s"), daemon->namebuff, port, s1, s2);
	}
      else
	syslog(LOG_INFO, _("using nameserver %s#%d"), daemon->namebuff, port); 
    }
  
  daemon->servers = ret;
}
  
void reload_servers(char *fname, struct daemon *daemon)
{
  FILE *f;
  char *line;
  struct server *old_servers = NULL;
  struct server *new_servers = NULL;
  struct server *serv = daemon->servers;

  /* move old servers to free list - we can reuse the memory 
     and not risk malloc if there are the same or fewer new servers. 
     Servers which were specced on the command line go to the new list. */
  while (serv)
    {
      struct server *tmp = serv->next;
      if (serv->flags & SERV_FROM_RESOLV)
	{
	  serv->next = old_servers;
	  old_servers = serv;
	}
      else
	{
	  serv->next = new_servers;
	  new_servers = serv;
	}
      serv = tmp;
    }

  /* buff happens to be NAXDNAME long... */
  f = fopen(fname, "r");
  if (!f)
    {
      syslog(LOG_ERR, _("failed to read %s: %m"), fname);
    }
  else
    {
      syslog(LOG_INFO, _("reading %s"), fname);
      while ((line = fgets(daemon->namebuff, MAXDNAME, f)))
	{
	  union  mysockaddr addr, source_addr;
	  char *token = strtok(line, " \t\n\r");
	  struct server *serv;
	  
	  if (!token || strcmp(token, "nameserver") != 0)
	    continue;
	  if (!(token = strtok(NULL, " \t\n\r")))
	    continue;
	  
#ifdef HAVE_IPV6
          if (inet_pton(AF_INET, token, &addr.in.sin_addr) > 0)
#else
          if ((addr.in.sin_addr.s_addr = inet_addr(token)) != (in_addr_t) -1)
#endif
	    {
#ifdef HAVE_SOCKADDR_SA_LEN
	      source_addr.in.sin_len = addr.in.sin_len = sizeof(struct sockaddr_in);
#endif
	      source_addr.in.sin_family = addr.in.sin_family = AF_INET;
	      addr.in.sin_port = htons(NAMESERVER_PORT);
	      source_addr.in.sin_addr.s_addr = INADDR_ANY;
	      source_addr.in.sin_port = htons(daemon->query_port);
	    }
#ifdef HAVE_IPV6
	  else if (inet_pton(AF_INET6, token, &addr.in6.sin6_addr) > 0)
	    {
#ifdef HAVE_SOCKADDR_SA_LEN
	      source_addr.in6.sin6_len = addr.in6.sin6_len = sizeof(struct sockaddr_in6);
#endif
	      source_addr.in6.sin6_family = addr.in6.sin6_family = AF_INET6;
	      addr.in6.sin6_port = htons(NAMESERVER_PORT);
	      source_addr.in6.sin6_flowinfo = addr.in6.sin6_flowinfo = htonl(0);
	      source_addr.in6.sin6_addr = in6addr_any;
	      source_addr.in6.sin6_port = htons(daemon->query_port);
	    }
#endif /* IPV6 */
	  else
	    continue;
	  
	  if (old_servers)
	    {
	      serv = old_servers;
	      old_servers = old_servers->next;
	    }
	  else if (!(serv = malloc(sizeof (struct server))))
	    continue;
	  
	  /* this list is reverse ordered: 
	     it gets reversed again in check_servers */
	  serv->next = new_servers;
	  new_servers = serv;
	  serv->addr = addr;
	  serv->source_addr = source_addr;
	  serv->domain = NULL;
	  serv->sfd = NULL;
	  serv->flags = SERV_FROM_RESOLV;
	}
  
      fclose(f);
    }

  /* Free any memory not used. */
  while(old_servers)
    {
      struct server *tmp = old_servers->next;
      free(old_servers);
      old_servers = tmp;
    }

  daemon->servers = new_servers;
}







