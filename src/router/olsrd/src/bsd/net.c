/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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
 * $Id: net.c,v 1.26 2005/08/28 19:30:29 kattemat Exp $
 */

#include "defs.h"
#include "net_os.h"
#include "parser.h" /* dnc: needed for call to packet_parser() */
#include "net.h"

#include <net/if.h>

#ifdef __NetBSD__
#include <sys/param.h>
#include <net/if_ether.h>
#endif

#ifdef __OpenBSD__
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#include <netinet/icmp6.h>
#endif

#ifdef __FreeBSD__
#include <net/if_var.h>
#include <net/ethernet.h>
#ifndef FBSD_NO_80211
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>
#include <dev/wi/if_wavelan_ieee.h>
#include <dev/wi/if_wireg.h>
#endif
#endif

#ifdef SPOOF
#include <net/if_dl.h>
#include <libnet.h>
#endif /* SPOOF */

//#define	SIOCGIFGENERIC	_IOWR('i', 58, struct ifreq)	/* generic IF get op */
//#define SIOCGWAVELAN SIOCGIFGENERIC

#include <sys/sysctl.h>

static int ignore_redir;
static int send_redir;
static int gateway;

static int set_sysctl_int(char *name, int new)
{
  int old;
  unsigned int len = sizeof (old);

#ifdef __OpenBSD__
  int mib[4];

  /* Set net.inet.ip.forwarding by default. */
  mib[0] = CTL_NET;
  mib[1] = PF_INET;
  mib[2] = IPPROTO_IP;
  mib[3] = IPCTL_FORWARDING;

  if (!strcmp(name, "net.inet6.ip6.forwarding"))
  {
    mib[1] = PF_INET6;
    mib[2] = IPPROTO_IPV6;
  }
  else if (!strcmp(name, "net.inet.icmp.rediraccept"))
  {
    mib[2] = IPPROTO_ICMP;
    mib[3] = ICMPCTL_REDIRACCEPT;
  }
  else if (!strcmp(name, "net.inet6.icmp6.rediraccept"))
  {
    mib[2] = IPPROTO_ICMPV6;
    mib[3] = ICMPV6CTL_REDIRACCEPT;
  }
  else if (!strcmp(name, "net.inet.ip.redirect"))
  {
    mib[3] = IPCTL_SENDREDIRECTS;
  }
  else if (!strcmp(name, "net.inet6.ip6.redirect"))
  {
    mib[1] = PF_INET6;
    mib[2] = IPPROTO_IPV6;
    mib[3] = IPCTL_SENDREDIRECTS;
  }

  if (sysctl(mib, 4, &old, &len, &new, sizeof (new)) < 0)
    return -1;
#else

  if (sysctlbyname(name, &old, &len, &new, sizeof (new)) < 0)
    return -1;
#endif

  return old;
}

int enable_ip_forwarding(int version)
{
  char *name;

  if (olsr_cnf->ip_version == AF_INET)
    name = "net.inet.ip.forwarding";

  else
    name = "net.inet6.ip6.forwarding";

  gateway = set_sysctl_int(name, 1);

  if (gateway < 0)
    {
      fprintf(stderr, "Cannot enable IP forwarding. Please enable IP forwarding manually. Continuing in 3 seconds...\n");
      sleep(3);
    }

  return 1;
}

int
disable_redirects_global(int version)
{
  char *name;

  // do not accept ICMP redirects

#ifdef __OpenBSD__
  if (olsr_cnf->ip_version == AF_INET)
    name = "net.inet.icmp.rediraccept";

  else
    name = "net.inet6.icmp6.rediraccept";

  ignore_redir = set_sysctl_int(name, 0);
#else
  if (olsr_cnf->ip_version == AF_INET)
    name = "net.inet.icmp.drop_redirect";

  else
    name = "net.inet6.icmp6.drop_redirect";

  ignore_redir = set_sysctl_int(name, 1);
#endif

  if (ignore_redir < 0)
    {
      fprintf(stderr, "Cannot disable incoming ICMP redirect messages. Please disable them manually. Continuing in 3 seconds...\n");
      sleep(3);
    }

  // do not send ICMP redirects

  if (olsr_cnf->ip_version == AF_INET)
    name = "net.inet.ip.redirect";

  else
    name = "net.inet6.ip6.redirect";

  send_redir = set_sysctl_int(name, 0);

  if (send_redir < 0)
    {
      fprintf(stderr, "Cannot disable outgoing ICMP redirect messages. Please disable them manually. Continuing in 3 seconds...\n");
      sleep(3);
    }

  return 1;
}

int disable_redirects(char *if_name, int index, int version)
{
  // this function gets called for each interface olsrd uses; however,
  // FreeBSD can only globally control ICMP redirects, and not on a
  // per-interface basis; hence, only disable ICMP redirects in the "global"
  // function
  return 1;
}

int deactivate_spoof(char *if_name, int index, int version)
{
  return 1;
}

int restore_settings(int version)
{
  char *name;

  // reset IP forwarding

  if (olsr_cnf->ip_version == AF_INET)
    name = "net.inet.ip.forwarding";

  else
    name = "net.inet6.ip6.forwarding";

  set_sysctl_int(name, gateway);

  // reset incoming ICMP redirects

#ifdef __OpenBSD__
  if (olsr_cnf->ip_version == AF_INET)
    name = "net.inet.icmp.rediraccept";
  else
    name = "net.inet6.icmp6.rediraccept";

#else
  if (olsr_cnf->ip_version == AF_INET)
    name = "net.inet.icmp.drop_redirect";

  else
    name = "net.inet6.icmp6.drop_redirect";
#endif

  set_sysctl_int(name, ignore_redir);

  // reset outgoing ICMP redirects

  if (olsr_cnf->ip_version == AF_INET)
    name = "net.inet.ip.redirect";

  else
    name = "net.inet6.ip6.redirect";

  set_sysctl_int(name, send_redir);

  return 1;
}


/**
 *Creates a nonblocking broadcast socket.
 *@param sa sockaddr struct. Used for bind(2).
 *@return the FD of the socket or -1 on error.
 */
int
gethemusocket(struct sockaddr_in *pin)
{
  int sock, on = 1;

  OLSR_PRINTF(1, "       Connecting to switch daemon port 10150...");


  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
      perror("hcsocket");
      syslog(LOG_ERR, "hcsocket: %m");
      return (-1);
    }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) 
    {
      perror("SO_REUSEADDR failed");
      return (-1);
    }
  /* connect to PORT on HOST */
  if (connect(sock,(struct sockaddr *) pin, sizeof(*pin)) < 0) 
    {
      printf("FAILED\n");
      fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno));
      printf("connection refused\n");
      return (-1);
    }

  printf("OK\n");

  /* Keep TCP socket blocking */  
  return (sock);
}


int
getsocket(struct sockaddr *sa, int bufspace, char *int_name)
{
  struct sockaddr_in *sin = (struct sockaddr_in *)sa;
  int sock, on = 1;

  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
      perror("socket");
      syslog(LOG_ERR, "socket: %m");
      return (-1);
    }

  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0)
    {
      perror("setsockopt");
      syslog(LOG_ERR, "setsockopt SO_BROADCAST: %m");
      close(sock);
      return (-1);
    }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) 
    {
      perror("SO_REUSEADDR failed");
      return (-1);
    }

#ifdef SPOOF
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0) 
    {
      perror("SO_REUSEPORT failed");
      return (-1);
    }

  if (setsockopt(sock, IPPROTO_IP, IP_RECVIF, &on, sizeof(on)) < 0) 
    {
      perror("IP_RECVIF failed");
      return (-1);
    }
#endif /* SPOOF */

  for (on = bufspace; ; on -= 1024) 
    {
      if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF,
		     &on, sizeof (on)) == 0)
	break;
      if (on <= 8*1024) 
	{
	  perror("setsockopt");
	  syslog(LOG_ERR, "setsockopt SO_RCVBUF: %m");
	  break;
	}
    }

  if (bind(sock, (struct sockaddr *)sin, sizeof (*sin)) < 0) 
    {
      perror("bind");
      syslog(LOG_ERR, "bind: %m");
      close(sock);
      return (-1);
    }

  if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
    syslog(LOG_ERR, "fcntl O_NONBLOCK: %m\n");

  return (sock);
}

int getsocket6(struct sockaddr_in6 *sin, int bufspace, char *int_name)
{
  int sock, on = 1;

  if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) 
    {
      perror("socket");
      syslog(LOG_ERR, "socket: %m");
      return (-1);
    }

  for (on = bufspace; ; on -= 1024) 
    {
      if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF,
		     &on, sizeof (on)) == 0)
	break;
      if (on <= 8*1024) 
	{
	  perror("setsockopt");
	  syslog(LOG_ERR, "setsockopt SO_RCVBUF: %m");
	  break;
	}
    }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) 
    {
      perror("SO_REUSEADDR failed");
      return (-1);
    }

  if (bind(sock, (struct sockaddr *)sin, sizeof (*sin)) < 0) 
    {
      perror("bind");
      syslog(LOG_ERR, "bind: %m");
      close(sock);
      return (-1);
    }

  if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
    syslog(LOG_ERR, "fcntl O_NONBLOCK: %m\n");

  return (sock);
}



int
join_mcast(struct interface *ifs, int sock)
{
  /* See linux/in6.h */

  struct ipv6_mreq mcastreq;

  COPY_IP(&mcastreq.ipv6mr_multiaddr, &ifs->int6_multaddr.sin6_addr);
  mcastreq.ipv6mr_interface = ifs->if_index;

#if 0
  OLSR_PRINTF(3, "Interface %s joining multicast %s...",	ifs->int_name, olsr_ip_to_string((union olsr_ip_addr *)&ifs->int6_multaddr.sin6_addr))
  /* Send multicast */
  if(setsockopt(sock, 
		IPPROTO_IPV6, 
		IPV6_ADD_MEMBERSHIP, 
		(char *)&mcastreq, 
		sizeof(struct ipv6_mreq)) 
     < 0)
    {
      perror("Join multicast");
      return -1;
    }
#else
#warning implement IPV6_ADD_MEMBERSHIP
#endif

  /* Old libc fix */
#ifdef IPV6_JOIN_GROUP
  /* Join reciever group */
  if(setsockopt(sock, 
		IPPROTO_IPV6, 
		IPV6_JOIN_GROUP, 
		(char *)&mcastreq, 
		sizeof(struct ipv6_mreq)) 
     < 0)
#else
  /* Join reciever group */
  if(setsockopt(sock, 
		IPPROTO_IPV6, 
		IPV6_ADD_MEMBERSHIP, 
		(char *)&mcastreq, 
		sizeof(struct ipv6_mreq)) 
     < 0)
#endif 
    {
      perror("Join multicast send");
      return -1;
    }

  
  if(setsockopt(sock, 
		IPPROTO_IPV6, 
		IPV6_MULTICAST_IF, 
		(char *)&mcastreq.ipv6mr_interface, 
		sizeof(mcastreq.ipv6mr_interface)) 
     < 0)
    {
      perror("Set multicast if");
      return -1;
    }


  OLSR_PRINTF(3, "OK\n")
  return 0;
}




int get_ipv6_address(char *ifname, struct sockaddr_in6 *saddr6, int scope_in)
{
  return 0;
}




/**
 * Wrapper for sendto(2)
 */

#ifdef SPOOF
static u_int16_t ip_id = 0;
#endif /* SPOOF */

ssize_t
olsr_sendto(int s, 
	    const void *buf, 
	    size_t len, 
	    int flags, 
	    const struct sockaddr *to, 
	    socklen_t tolen)
{
#ifdef SPOOF
  /* IPv4 for now! */

  libnet_t *context;
  char errbuf[LIBNET_ERRBUF_SIZE];
  libnet_ptag_t udp_tag, ip_tag, ether_tag;
  unsigned char enet_broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  int status;
  struct sockaddr_in *to_in = (struct sockaddr_in *) to;
  u_int32_t destip;
  struct interface *iface;

  udp_tag = ip_tag = ether_tag = 0;
  destip = to_in->sin_addr.s_addr;
  iface = if_ifwithsock (s);

  /* initialize libnet */
  context = libnet_init(LIBNET_LINK, iface->int_name, errbuf);
  if (context == NULL)
    {
      OLSR_PRINTF (1, "libnet init: %s\n", libnet_geterror (context))
      return (0);
    }

  /* initialize IP ID field if necessary */
  if (ip_id == 0)
    {
      ip_id = (u_int16_t) (arc4random () & 0xffff);
    }

  udp_tag = libnet_build_udp (698, 				/* src port */
			      698,				/* dest port */
			      LIBNET_UDP_H + len,		/* length */
			      0,				/* checksum */
			      buf,				/* payload */
			      len,				/* payload size */
			      context,				/* context */
			      udp_tag);				/* pblock */
  if (udp_tag == -1)
    {
      OLSR_PRINTF (1, "libnet UDP header: %s\n", libnet_geterror (context))
	return (0);
    }

  ip_tag = libnet_build_ipv4 (LIBNET_IPV4_H + LIBNET_UDP_H + len, /* len */
			      0,				/* TOS */
			      ip_id++,				/* IP id */
			      0,				/* IP frag */
			      1,				/* IP TTL */
			      IPPROTO_UDP,			/* protocol */
			      0,				/* checksum */
			      libnet_get_ipaddr4 (context),	/* src IP */
			      destip,				/* dest IP */
			      NULL,				/* payload */
			      0,				/* payload len */
			      context,				/* context */
			      ip_tag);				/* pblock */
  if (ip_tag == -1)
    {
      OLSR_PRINTF (1, "libnet IP header: %s\n", libnet_geterror (context))
      return (0);
    }

  ether_tag = libnet_build_ethernet (enet_broadcast,    	/* ethernet dest */
				     libnet_get_hwaddr (context), /* ethernet source */
				     ETHERTYPE_IP,		/* protocol type */
				     NULL,        		/* payload */
				     0,           		/* payload size */
				     context,     		/* libnet handle */
				     ether_tag);  		/* pblock tag */
  if (ether_tag == -1)
    {
      OLSR_PRINTF (1, "libnet ethernet header: %s\n", libnet_geterror (context))
      return (0);
    }
 
  status = libnet_write (context);
  if (status == -1)
    {
      OLSR_PRINTF (1, "libnet packet write: %s\n", libnet_geterror (context))
      return (0);
    }

  libnet_destroy (context);

  return (len);

#else
  return sendto(s, buf, len, flags, to, tolen);
#endif
}


/**
 * Wrapper for recvfrom(2)
 */

ssize_t  
olsr_recvfrom(int  s, 
	      void *buf, 
	      size_t len, 
	      int flags, 
	      struct sockaddr *from,
	      socklen_t *fromlen)
{
#if SPOOF
  struct msghdr mhdr;
  struct iovec iov;
  struct cmsghdr *cm;
  struct sockaddr_dl *sdl;
  struct sockaddr_in *sin = (struct sockaddr_in *) from; //XXX
  unsigned char chdr[4096];
  int count;
  struct interface *ifc;
  char iname[32];

  bzero(&mhdr, sizeof(mhdr));
  bzero(&iov, sizeof(iov));

  mhdr.msg_name = (caddr_t) from;
  mhdr.msg_namelen = *fromlen;
  mhdr.msg_iov = &iov;
  mhdr.msg_iovlen = 1;
  mhdr.msg_control = (caddr_t) chdr;
  mhdr.msg_controllen = sizeof (chdr);

  iov.iov_len = MAXMESSAGESIZE;
  iov.iov_base = buf;

  count = recvmsg (s, &mhdr, MSG_DONTWAIT);
  if (count <= 0)
    {
      return (count);
    }

  /* this needs to get communicated back to caller */
  *fromlen = mhdr.msg_namelen;

  cm = (struct cmsghdr *) chdr;
  sdl = (struct sockaddr_dl *) CMSG_DATA (cm);
  bzero (iname, sizeof (iname));
  memcpy (iname, sdl->sdl_data, sdl->sdl_nlen);

  ifc = if_ifwithsock (s);

  if (strcmp (ifc->int_name, iname) != 0)
    {
      return (0);
    }

  OLSR_PRINTF (2, "%d bytes from %s, socket associated %s really received on %s\n",
	       count,
	       inet_ntoa (sin->sin_addr),
	       ifc->int_name,
	       iname);

  return (count);

#else /* SPOOF */
  return recvfrom(s, 
		  buf, 
		  len, 
		  0, 
		  from, 
		  fromlen);
#endif /* SPOOF */
}

/**
 * Wrapper for select(2)
 */

int
olsr_select(int nfds,
	    fd_set *readfds,
	    fd_set *writefds,
	    fd_set *exceptfds,
	    struct timeval *timeout)
{
  return select(nfds,
		readfds,
		writefds,
		exceptfds,
		timeout);
}


int 
check_wireless_interface(char *ifname)
{
#if defined __FreeBSD__ &&  !defined FBSD_NO_80211
  struct wi_req	wreq;
  struct ifreq ifr;

  memset((char *)&wreq, 0, sizeof(wreq));
  memset((char *)&ifr, 0, sizeof(ifr));

  wreq.wi_len = WI_MAX_DATALEN;
  wreq.wi_type = WI_RID_IFACE_STATS;

  strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
  ifr.ifr_data = (caddr_t)&wreq;

  return (ioctl(ioctl_s, SIOCGWAVELAN, &ifr) >= 0) ? 1 : 0;
#else
  return 0;
#endif
}

#include <sys/sockio.h>

int
calculate_if_metric(char *ifname)
{
  if(check_wireless_interface(ifname))
    {
      /* Wireless */
      return 1;
    }
  else
    {
      /* Ethernet */
#if 0
      /* Andreas: Perhaps SIOCGIFMEDIA is the way to do this? */
      struct ifmediareq ifm;

      memset(&ifm, 0, sizeof(ifm));
      strlcpy(ifm.ifm_name, ifname, sizeof(ifm.ifm_name));

      if(ioctl(ioctl_s, SIOCGIFMEDIA, &ifm) < 0)
	{
	  OLSR_PRINTF(1, "Error SIOCGIFMEDIA(%s)\n", ifm.ifm_name)
	  return WEIGHT_ETHERNET_DEFAULT;
	}

      OLSR_PRINTF(1, "%s: STATUS 0x%08x\n", ifm.ifm_name, ifm.ifm_status)
#endif
      return WEIGHT_ETHERNET_DEFAULT;
    }
}
