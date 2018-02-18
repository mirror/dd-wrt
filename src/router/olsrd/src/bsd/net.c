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

#if defined __FreeBSD_kernel__
#define _GNU_SOURCE 1
#endif /* defined __FreeBSD_kernel__ */

/* comment: we need this otherwise it does not include the proper files to find IPv6_PKTINFO */
#define __APPLE_USE_RFC_2292

#include "defs.h"
#include "net_os.h"
#include "ipcalc.h"
#include "parser.h"          /* dnc: needed for call to packet_parser() */
#include "olsr_protocol.h"
#include "olsr_cfg.h"
#include "olsr.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <syslog.h>
#include <netinet/in.h>
#include <net/if.h>

#ifdef _WRS_KERNEL
#include <vxWorks.h>
#include "wrn/coreip/netinet6/in6_var.h"
#include <sockLib.h>
#include <sys/socket.h>
#include "wrn/coreip/net/ifaddrs.h"
#include <selectLib.h>
#include <logLib.h>
#define syslog(a, b) fdprintf(a, b);
#else /* _WRS_KERNEL */
#include <sys/param.h>
#endif /* _WRS_KERNEL */

#ifdef __NetBSD__
#include <net/if_ether.h>
#include <netinet6/in6_var.h>   /* For struct in6_ifreq */
#include <net80211/ieee80211_ioctl.h>
#include <ifaddrs.h>
#endif /* __NetBSD__ */

#ifdef __OpenBSD__
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#include <netinet/icmp6.h>
#include <netinet6/in6_var.h>   /* For struct in6_ifreq */
#include <ifaddrs.h>
#include <sys/uio.h>
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>
#endif /* __OpenBSD__ */

#if defined __FreeBSD__ || defined __FreeBSD_kernel__
#include <net/if_var.h>
#include <net/ethernet.h>
#include <netinet/in_var.h>
#include <ifaddrs.h>
#ifndef FBSD_NO_80211
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>
#endif /* FBSD_NO_80211 */
#endif /* defined __FreeBSD__ || defined __FreeBSD_kernel__ */

#ifdef __APPLE__
#include <ifaddrs.h>
#include <net/if_var.h>
#include <net/ethernet.h>
#include <netinet/in_var.h>
#endif /* __APPLE__ */

#include <net/if_dl.h>
#ifdef SPOOF
#include <libnet.h>
#endif /* SPOOF */

#include <sys/sysctl.h>

static int ignore_redir;
static int send_redir;
static int gateway;

static int
set_sysctl_int(const char *name, int new)
{
  int old;
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__)
  size_t len = sizeof(old);
#else /* defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__) */
  unsigned int len = sizeof(old);
#endif /* defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__) */

#ifdef __OpenBSD__
  int mib[4];

  /* Set net.inet.ip.forwarding by default. */
  mib[0] = CTL_NET;
  mib[1] = PF_INET;
  mib[2] = IPPROTO_IP;
  mib[3] = IPCTL_FORWARDING;

  if (!strcmp(name, "net.inet6.ip6.forwarding")) {
    mib[1] = PF_INET6;
    mib[2] = IPPROTO_IPV6;
  } else if (!strcmp(name, "net.inet.icmp.rediraccept")) {
    mib[2] = IPPROTO_ICMP;
    mib[3] = ICMPCTL_REDIRACCEPT;
  } else if (!strcmp(name, "net.inet6.icmp6.rediraccept")) {
    mib[2] = IPPROTO_ICMPV6;
    mib[3] = ICMPV6CTL_REDIRACCEPT;
  } else if (!strcmp(name, "net.inet.ip.redirect")) {
    mib[3] = IPCTL_SENDREDIRECTS;
  } else if (!strcmp(name, "net.inet6.ip6.redirect")) {
    mib[1] = PF_INET6;
    mib[2] = IPPROTO_IPV6;
    mib[3] = IPCTL_SENDREDIRECTS;
  }

  if (sysctl(mib, 4, &old, &len, &new, sizeof(new)) < 0)
    return -1;
#else /* __OpenBSD__ */

  if (sysctlbyname((const char *)name, &old, &len, &new, sizeof(new)) < 0)
    return -1;
#endif /* __OpenBSD__ */

  return old;
}

void
net_os_set_global_ifoptions(void) {
  const char *name = olsr_cnf->ip_version == AF_INET ? "net.inet.ip.forwarding" : "net.inet6.ip6.forwarding";

  gateway = set_sysctl_int(name, 1);
  if (gateway < 0) {
    fprintf(stderr, "Cannot enable IP forwarding. Please enable IP forwarding manually." " Continuing in 3 seconds...\n");
    olsr_startup_sleep(3);
  }

  /* do not accept ICMP redirects */

#if defined(__OpenBSD__) || defined(__NetBSD__)
  if (olsr_cnf->ip_version == AF_INET)
    name = "net.inet.icmp.rediraccept";
  else
    name = "net.inet6.icmp6.rediraccept";

  ignore_redir = set_sysctl_int(name, 0);
#elif defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__
  if (olsr_cnf->ip_version == AF_INET) {
    name = "net.inet.icmp.drop_redirect";
    ignore_redir = set_sysctl_int(name, 1);
  } else {
    name = "net.inet6.icmp6.rediraccept";
    ignore_redir = set_sysctl_int(name, 0);
  }
#else /* defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ */
  if (olsr_cnf->ip_version == AF_INET)
    name = "net.inet.icmp.drop_redirect";
  else
    name = "net.inet6.icmp6.drop_redirect";

  ignore_redir = set_sysctl_int(name, 1);
#endif /* defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ */

  if (ignore_redir < 0) {
    fprintf(stderr,
            "Cannot disable incoming ICMP redirect messages. " "Please disable them manually. Continuing in 3 seconds...\n");
    olsr_startup_sleep(3);
  }

  /* do not send ICMP redirects */

  if (olsr_cnf->ip_version == AF_INET)
    name = "net.inet.ip.redirect";
  else
    name = "net.inet6.ip6.redirect";

  send_redir = set_sysctl_int(name, 0);
  if (send_redir < 0) {
    fprintf(stderr,
            "Cannot disable outgoing ICMP redirect messages. " "Please disable them manually. Continuing in 3 seconds...\n");
    olsr_startup_sleep(3);
  }
}

int net_os_set_ifoptions(const char *if_name __attribute__ ((unused)), struct interface_olsr *iface __attribute__ ((unused))) {
  return -1;
}

int
net_os_restore_ifoptions(void) {
  /* reset IP forwarding */
  const char *name = olsr_cnf->ip_version == AF_INET ? "net.inet.ip.forwarding" : "net.inet6.ip6.forwarding";

  set_sysctl_int(name, gateway);

  /* reset incoming ICMP redirects */

#ifdef __OpenBSD__
  name = olsr_cnf->ip_version == AF_INET ? "net.inet.icmp.rediraccept" : "net.inet6.icmp6.rediraccept";
#elif defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__
  name = olsr_cnf->ip_version == AF_INET ? "net.inet.icmp.drop_redirect" : "net.inet6.icmp6.rediraccept";
#else /* defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ */
  name = olsr_cnf->ip_version == AF_INET ? "net.inet.icmp.drop_redirect" : "net.inet6.icmp6.drop_redirect";
#endif /* defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ */
  set_sysctl_int(name, ignore_redir);

  /* reset outgoing ICMP redirects */
  name = olsr_cnf->ip_version == AF_INET ? "net.inet.ip.redirect" : "net.inet6.ip6.redirect";
  set_sysctl_int(name, send_redir);
  return 1;
}

/**
 *Creates a nonblocking broadcast socket.
 *@param pin sockaddr struct. Used for bind(2).
 *@return the FD of the socket or -1 on error.
 */
int
gethemusocket(struct sockaddr_in *pin)
{
  int sock, on = 1;

  OLSR_PRINTF(1, "       Connecting to switch daemon port 10150...");

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("hcsocket");
    syslog(LOG_ERR, "hcsocket: %m");
    return (-1);
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0) {
    perror("SO_REUSEADDR failed");
    close(sock);
    return (-1);
  }
  /* connect to PORT on HOST */
  if (connect(sock, (struct sockaddr *)pin, sizeof(*pin)) < 0) {
    printf("FAILED\n");
    fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno));
    printf("connection refused\n");
    close(sock);
    return (-1);
  }

  printf("OK\n");

  /* Keep TCP socket blocking */
  return (sock);
}

int
getsocket(int bufspace, struct interface_olsr *ifp __attribute__ ((unused)))
{
  struct sockaddr_in sin;
  int on;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("socket");
    syslog(LOG_ERR, "socket: %m");
    return -1;
  }

  on = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&on, sizeof(on)) < 0) {
    perror("setsockopt");
    syslog(LOG_ERR, "setsockopt SO_BROADCAST: %m");
    close(sock);
    return -1;
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0) {
    perror("SO_REUSEADDR failed");
    close(sock);
    return -1;
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char *)&on, sizeof(on)) < 0) {
    perror("SO_REUSEPORT failed");
    close(sock);
    return -1;
  }

  if (setsockopt(sock, IPPROTO_IP, IP_RECVIF, (char *)&on, sizeof(on)) < 0) {
    perror("IP_RECVIF failed");
    close(sock);
    return -1;
  }

  if(bufspace > 0) {
    for (on = bufspace;; on -= 1024) {
      if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&on, sizeof(on)) == 0)
        break;
      if (on <= 8 * 1024) {
        perror("setsockopt");
        syslog(LOG_ERR, "setsockopt SO_RCVBUF: %m");
        break;
      }
    }
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(olsr_cnf->olsrport);

  if(bufspace <= 0) {
    sin.sin_addr.s_addr = ifp->int_addr.sin_addr.s_addr;
  }

  if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("bind");
    syslog(LOG_ERR, "bind: %m");
    close(sock);
    return -1;
  }

  on = fcntl(sock, F_GETFL);
  if (on == -1) {
    syslog(LOG_ERR, "fcntl (F_GETFL): %m\n");
  } else {
    if (fcntl(sock, F_SETFL, on | O_NONBLOCK) == -1) {
      syslog(LOG_ERR, "fcntl O_NONBLOCK: %m\n");
    }
  }
  return (sock);
}

int
getsocket6(int bufspace, struct interface_olsr *ifp __attribute__ ((unused)))
{
  struct sockaddr_in6 sin;
  int on;
  int sock = socket(AF_INET6, SOCK_DGRAM, 0);

  if (sock < 0) {
    perror("socket");
    syslog(LOG_ERR, "socket: %m");
    return -1;
  }

  if(bufspace > 0) {
    for (on = bufspace;; on -= 1024) {
      if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&on, sizeof(on)) == 0)
        break;
      if (on <= 8 * 1024) {
        perror("setsockopt");
        syslog(LOG_ERR, "setsockopt SO_RCVBUF: %m");
        break;
      }
    }
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0) {
    perror("SO_REUSEADDR failed");
    close(sock);
    return -1;
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char *)&on, sizeof(on)) < 0) {
    perror("SO_REUSEPORT failed");
    close(sock);
    return -1;
  }
#ifdef IPV6_RECVPKTINFO
  if (setsockopt(sock, IPPROTO_IPV6, IPV6_RECVPKTINFO, (char *)&on, sizeof(on)) < 0) {
    perror("IPV6_RECVPKTINFO failed");
    close(sock);
    return -1;
  }
#elif defined IPV6_PKTINFO
  if (setsockopt(sock, IPPROTO_IPV6, IPV6_PKTINFO, (char *)&on, sizeof(on)) < 0) {
    perror("IPV6_PKTINFO failed");
    close(sock);
    return -1;
  }
#endif /* defined IPV6_PKTINFO */

  memset(&sin, 0, sizeof(sin));
  sin.sin6_family = AF_INET6;
  sin.sin6_port = htons(olsr_cnf->olsrport);

  if(bufspace <= 0) {
    memcpy(&sin.sin6_addr, &ifp->int6_addr.sin6_addr, sizeof(struct in6_addr));
  }

  if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("bind");
    syslog(LOG_ERR, "bind: %m");
    close(sock);
    return -1;
  }

  on = fcntl(sock, F_GETFL);
  if (on == -1) {
    syslog(LOG_ERR, "fcntl (F_GETFL): %m\n");
  } else {
    if (fcntl(sock, F_SETFL, on | O_NONBLOCK) == -1) {
      syslog(LOG_ERR, "fcntl O_NONBLOCK: %m\n");
    }
  }
  return sock;
}

int
join_mcast(struct interface_olsr *ifs, int sock)
{
  /* See netinet6/in6.h */
  struct ipaddr_str addrstr;
  struct ipv6_mreq mcastreq;
#ifdef IPV6_USE_MIN_MTU
  int on;
#endif /* IPV6_USE_MIN_MTU */

  mcastreq.ipv6mr_multiaddr = ifs->int6_multaddr.sin6_addr;
  mcastreq.ipv6mr_interface = ifs->if_index;

  if (ifs->olsr_socket == sock) {

    OLSR_PRINTF(3, "Interface %s joining multicast %s...", ifs->int_name,
                olsr_ip_to_string(&addrstr, (union olsr_ip_addr *)&ifs->int6_multaddr.sin6_addr));

    /* rfc 3493 */
#ifdef IPV6_JOIN_GROUP
    /* Join receiver group */
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *)&mcastreq, sizeof(struct ipv6_mreq)) < 0)
#else /* IPV6_JOIN_GROUP */ /* rfc 2133, obsoleted */
    /* Join receiver group */
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)&mcastreq, sizeof(struct ipv6_mreq)) < 0)
#endif /* IPV6_JOIN_GROUP */
    {
      perror("Join multicast send");
      return -1;
    }
  }

  if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char *)&mcastreq.ipv6mr_interface, sizeof(mcastreq.ipv6mr_interface)) < 0) {
    perror("Set multicast if");
    return -1;
  }

#ifdef IPV6_USE_MIN_MTU
  /*
   * This allow multicast packets to use the full interface MTU and not
   * be limited to 1280 bytes.
   */
  on = 0;
  if (setsockopt(sock, IPPROTO_IPV6, IPV6_USE_MIN_MTU, (char *)&on, sizeof(on)) < 0) {
    perror("IPV6_USE_MIN_MTU failed");
    close(sock);
    return -1;
  }
#endif /* IPV6_USE_MIN_MTU */

  OLSR_PRINTF(3, "OK\n");
  return 0;
}

int
get_ipv6_address(char *ifname, struct sockaddr_in6 *saddr6, struct olsr_ip_prefix *prefix)
{
  struct ifaddrs *ifap, *ifa;
  const struct sockaddr_in6 *sin6 = NULL;
  const union olsr_ip_addr *tmp_ip;
  struct in6_ifreq ifr6;
  int found = 0;
  int s6;
  u_int32_t flags6;

  if (getifaddrs(&ifap) != 0) {
    OLSR_PRINTF(3, "get_ipv6_address: getifaddrs() failed.\n");
    return 0;
  }

  for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
    if ((ifa->ifa_addr->sa_family == AF_INET6) && (strcmp(ifa->ifa_name, ifname) == 0)) {
      sin6 = (const struct sockaddr_in6 *)(ifa->ifa_addr);
      if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr))
        continue;
      strscpy(ifr6.ifr_name, ifname, sizeof(ifr6.ifr_name));
      if ((s6 = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        OLSR_PRINTF(3, "socket(AF_INET6,SOCK_DGRAM)");
        break;
      }
      ifr6.ifr_addr = *sin6;
      if (ioctl(s6, SIOCGIFAFLAG_IN6, &ifr6) < 0) {
        OLSR_PRINTF(3, "ioctl(SIOCGIFAFLAG_IN6)");
        close(s6);
        break;
      }
      close(s6);
      flags6 = ifr6.ifr_ifru.ifru_flags6;
      if ((flags6 & IN6_IFF_ANYCAST) != 0)
        continue;

      tmp_ip = (const union olsr_ip_addr *) &sin6->sin6_addr;
      if ((prefix == NULL && !IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr))
          || (prefix != NULL && ip_in_net(tmp_ip, prefix))) {
        memcpy(&saddr6->sin6_addr, &sin6->sin6_addr, sizeof(struct in6_addr));
        found = 1;
        break;
      }
    }
  }
  freeifaddrs(ifap);
  if (found)
    return 1;

  return 0;
}

/**
 * Wrapper for sendto(2)
 */

#ifdef SPOOF
static u_int16_t ip_id = 0;
#endif /* SPOOF */

ssize_t
olsr_sendto(int s, const void *buf, size_t len, int flags __attribute__ ((unused)), const struct sockaddr *to, socklen_t tolen)
{
#ifdef SPOOF
  /* IPv4 for now! */

  libnet_t *context;
  char errbuf[LIBNET_ERRBUF_SIZE];
  libnet_ptag_t udp_tag, ip_tag, ether_tag;
  unsigned char enet_broadcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  int status;
  struct sockaddr_in *to_in = (struct sockaddr_in *)to;
  u_int32_t destip;
  struct interface_olsr *iface;

  udp_tag = ip_tag = ether_tag = 0;
  destip = to_in->sin_addr.s_addr;
  iface = if_ifwithsock(s);

  /* initialize libnet */
  context = libnet_init(LIBNET_LINK, iface->int_name, errbuf);
  if (context == NULL) {
    OLSR_PRINTF(1, "libnet init: %s\n", libnet_geterror(context));
    return (0);
  }

  /* initialize IP ID field if necessary */
  if (ip_id == 0) {
    ip_id = (u_int16_t) (olsr_random() & 0xffff);
  }

  udp_tag = libnet_build_udp(olsr_cnf->olsrport,        /* src port */
                             olsr_cnf->olsrport,        /* dest port */
                             LIBNET_UDP_H + len,        /* length */
                             0,         /* checksum */
                             buf,       /* payload */
                             len,       /* payload size */
                             context,   /* context */
                             udp_tag);  /* pblock */
  if (udp_tag == -1) {
    OLSR_PRINTF(1, "libnet UDP header: %s\n", libnet_geterror(context));
    return (0);
  }

  ip_tag = libnet_build_ipv4(LIBNET_IPV4_H + LIBNET_UDP_H + len,        /* len */
                             0, /* TOS */
                             ip_id++,   /* IP id */
                             0, /* IP frag */
                             1, /* IP TTL */
                             IPPROTO_UDP,       /* protocol */
                             0, /* checksum */
                             libnet_get_ipaddr4(context),       /* src IP */
                             destip,    /* dest IP */
                             NULL,      /* payload */
                             0, /* payload len */
                             context,   /* context */
                             ip_tag);   /* pblock */
  if (ip_tag == -1) {
    OLSR_PRINTF(1, "libnet IP header: %s\n", libnet_geterror(context));
    return (0);
  }

  ether_tag = libnet_build_ethernet(enet_broadcast,     /* ethernet dest */
                                    libnet_get_hwaddr(context), /* ethernet source */
                                    ETHERTYPE_IP,       /* protocol type */
                                    NULL,       /* payload */
                                    0,  /* payload size */
                                    context,    /* libnet handle */
                                    ether_tag); /* pblock tag */
  if (ether_tag == -1) {
    OLSR_PRINTF(1, "libnet ethernet header: %s\n", libnet_geterror(context));
    return (0);
  }

  status = libnet_write(context);
  if (status == -1) {
    OLSR_PRINTF(1, "libnet packet write: %s\n", libnet_geterror(context));
    return (0);
  }

  libnet_destroy(context);

  return (len);

#else /* SPOOF */
  return sendto(s, buf, len, flags, (const struct sockaddr *)to, tolen);
#endif /* SPOOF */
}

/**
 * Wrapper for recvfrom(2)
 */

ssize_t
olsr_recvfrom(int s, void *buf, size_t len, int flags __attribute__ ((unused)), struct sockaddr *from, socklen_t * fromlen)
{
  struct msghdr mhdr;
  struct iovec iov;
  union {
    struct cmsghdr cmsg;
    unsigned char chdr[4096];
  } cmu;
  struct cmsghdr *cm;
  struct sockaddr_dl *sdl;
  struct sockaddr_in *sin = (struct sockaddr_in *)from;
  struct sockaddr_in6 *sin6;
  struct in6_addr *iaddr6;
  struct in6_pktinfo *pkti;
  struct interface_olsr *ifc;
  char addrstr[INET6_ADDRSTRLEN];
  char iname[IFNAMSIZ];
  int count;

  memset(&mhdr, 0, sizeof(mhdr));
  memset(&iov, 0, sizeof(iov));

  mhdr.msg_name = (caddr_t) from;
  mhdr.msg_namelen = *fromlen;
  mhdr.msg_iov = &iov;
  mhdr.msg_iovlen = 1;
  mhdr.msg_control = (caddr_t) & cmu;
  mhdr.msg_controllen = sizeof(cmu);

  iov.iov_len = len;
  iov.iov_base = buf;

  count = recvmsg(s, &mhdr, MSG_DONTWAIT);
  if (count <= 0) {
    return (count);
  }

  /* this needs to get communicated back to caller */
  *fromlen = mhdr.msg_namelen;
  if (olsr_cnf->ip_version == AF_INET6) {
    for (cm = (struct cmsghdr *)CMSG_FIRSTHDR(&mhdr); cm; cm = (struct cmsghdr *)CMSG_NXTHDR(&mhdr, cm)) {
      if (cm->cmsg_level == IPPROTO_IPV6 && cm->cmsg_type == IPV6_PKTINFO) {
        pkti = (struct in6_pktinfo *)CMSG_DATA(cm);
        iaddr6 = &pkti->ipi6_addr;
        if_indextoname(pkti->ipi6_ifindex, iname);
      }
    }
  } else {
    cm = &cmu.cmsg;
    sdl = (struct sockaddr_dl *)CMSG_DATA(cm);
    memset(iname, 0, sizeof(iname));
    memcpy(iname, sdl->sdl_data, sdl->sdl_nlen);
  }

  ifc = if_ifwithsock(s);

  sin6 = (struct sockaddr_in6 *)from;
  OLSR_PRINTF(4, "%d bytes from %s, socket associated %s really received on %s\n", count,
              inet_ntop(olsr_cnf->ip_version, olsr_cnf->ip_version == AF_INET6 ? (char *)&sin6->sin6_addr : (char *)&sin->sin_addr,
                        addrstr, sizeof(addrstr)), ifc->int_name, iname);

#ifndef __NetBSD__
  if (strcmp(ifc->int_name, iname) != 0) {
    return (0);
  }
#endif /* __NetBSD__ */

  return (count);
}

/**
 * Wrapper for select(2)
 */

int
olsr_select(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval *timeout)
{
  return select(nfds, readfds, writefds, exceptfds, timeout);
}

int
check_wireless_interface(char *ifname)
{
#if (defined __FreeBSD__ || defined __FreeBSD_kernel__ ) &&  !defined FBSD_NO_80211

/* From FreeBSD ifconfig/ifieee80211.c ieee80211_status() */
  struct ieee80211req ireq;
  u_int8_t data[32];

  memset(&ireq, 0, sizeof(ireq));
  strscpy(ireq.i_name, ifname, sizeof(ireq.i_name));
  ireq.i_data = &data;
  ireq.i_type = IEEE80211_IOC_SSID;
  ireq.i_val = -1;
  return (ioctl(olsr_cnf->ioctl_s, SIOCG80211, &ireq) >= 0) ? 1 : 0;
#elif defined __OpenBSD__
  struct ieee80211_nodereq nr;
  bzero(&nr, sizeof(nr));
  strscpy(nr.nr_ifname, ifname, sizeof(nr.nr_ifname));
  return (ioctl(olsr_cnf->ioctl_s, SIOCG80211FLAGS, &nr) >= 0) ? 1 : 0;
#elif defined __NetBSD__
  struct ifreq ireq;
  struct ieee80211_nwid data;
  int ret;

  memset(&ireq, 0, sizeof(ireq));
  strscpy(ireq.ifr_name, ifname, sizeof(ireq.ifr_name));
  ireq.ifr_data = &data;
  ret = ioctl(olsr_cnf->ioctl_s, SIOCG80211NWID, &ireq);
  if(ret == 0)
	  return 1;
  return 0;
#else /* defined __NetBSD__ */
  ifname = NULL;                /* squelsh compiler warning */
  return 0;
#endif /* defined __NetBSD__ */
}

#include <sys/sockio.h>

int
calculate_if_metric(char *ifname)
{
  if (check_wireless_interface(ifname)) {
    /* Wireless */
    return 1;
  } else {
    /* Ethernet */
    return WEIGHT_ETHERNET_DEFAULT;
  }
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
