/*
 *	BIRD Internet Routing Daemon -- Linux Multicasting and Network Includes
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifdef IPV6

#ifndef IPV6_UNICAST_HOPS
/* Needed on glibc 2.0 systems */
#include <linux/in6.h>
#define CONFIG_IPV6_GLIBC_20
#endif

static inline void
set_inaddr(struct in6_addr *ia, ip_addr a)
{
  ipa_hton(a);
  memcpy(ia, &a, sizeof(a));
}

#else

#include <net/if.h>

static inline void
set_inaddr(struct in_addr *ia, ip_addr a)
{
  ipa_hton(a);
  memcpy(&ia->s_addr, &a, sizeof(a));
}

/*
 *  Multicasting in Linux systems is a real mess. Not only different kernels
 *  have different interfaces, but also different libc's export it in different
 *  ways. Horrible.
 */

static inline char *sysio_mcast_setup(sock *s)
{
  int zero = 0;

  if (ipa_nonzero(s->daddr))
    {
      if (
#ifdef IP_DEFAULT_MULTICAST_TTL
	  s->ttl != IP_DEFAULT_MULTICAST_TTL &&
#endif
	  setsockopt(s->fd, SOL_IP, IP_MULTICAST_TTL, &s->ttl, sizeof(s->ttl)) < 0)
	return "IP_MULTICAST_TTL";
      if (
#ifdef IP_DEFAULT_MULTICAST_LOOP
	  IP_DEFAULT_MULTICAST_LOOP &&
#endif
	  setsockopt(s->fd, SOL_IP, IP_MULTICAST_LOOP, &zero, sizeof(zero)) < 0)
	return "IP_MULTICAST_LOOP";
    }
  return NULL;
}

#ifdef CONFIG_LINUX_MC_MREQN
/*
 *  2.1 and newer kernels use struct mreqn which passes ifindex, so no
 *  problems with unnumbered devices.
 */

#ifndef HAVE_STRUCT_IP_MREQN
/* Several versions of glibc don't define this structure, so we have to do it ourselves */
struct ip_mreqn
{
	struct in_addr	imr_multiaddr;		/* IP multicast address of group */
	struct in_addr	imr_address;		/* local IP address of interface */
	int		imr_ifindex;		/* Interface index */
};
#endif

static inline char *sysio_mcast_join(sock *s)
{
  struct ip_mreqn mreq;
  char *err;
  struct ifreq ifr;

  if (err = sysio_mcast_setup(s))
    return err;
  strcpy(ifr.ifr_name, s->iface->name);
  if (setsockopt(s->fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0)
    return "SO_BINDTODEVICE";
  mreq.imr_ifindex = s->iface->index;
  set_inaddr(&mreq.imr_address, s->iface->addr->ip);
  set_inaddr(&mreq.imr_multiaddr, s->daddr);
  /* This defines where should we send _outgoing_ multicasts */
  if (ipa_nonzero(s->daddr) && setsockopt(s->fd, SOL_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq)) < 0)
    return "IP_MULTICAST_IF";
  /* And this one sets interface for _receiving_ multicasts from */
  if (ipa_nonzero(s->saddr) && setsockopt(s->fd, SOL_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    return "IP_ADD_MEMBERSHIP";
  return NULL;
}
#endif

#if defined(CONFIG_LINUX_MC_MREQ) || defined(CONFIG_LINUX_MC_MREQ_BIND)
/*
 *  Older kernels support only struct mreq which matches interfaces by their
 *  addresses and thus fails on unnumbered devices. On newer 2.0 kernels
 *  we can use SO_BINDTODEVICE to circumvent this problem.
 */

static inline char *sysio_mcast_join(sock *s)
{
  struct in_addr mreq;
  struct ip_mreq mreq_add;
  char *err;

  if (err = sysio_mcast_setup(s))
    return err;
  set_inaddr(&mreq, s->iface->addr->ip);
#ifdef CONFIG_LINUX_MC_MREQ_BIND
  {
    struct ifreq ifr;
    strcpy(ifr.ifr_name, s->iface->name);
    if (setsockopt(s->fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0)
      return "SO_BINDTODEVICE";
    mreq_add.imr_interface.s_addr = INADDR_ANY;
  }
#else
  mreq_add.imr_interface = mreq;
#endif
  set_inaddr(&mreq_add.imr_multiaddr, s->daddr);
  /* This defines where should we send _outgoing_ multicasts */
  if (ipa_nonzero(s->daddr) && setsockopt(s->fd, SOL_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq)) < 0)
    return "IP_MULTICAST_IF";
  /* And this one sets interface for _receiving_ multicasts from */
  if (ipa_nonzero(s->saddr) && setsockopt(s->fd, SOL_IP, IP_ADD_MEMBERSHIP, &mreq_add, sizeof(mreq_add)) < 0)
    return "IP_ADD_MEMBERSHIP";
  return NULL;
}
#endif

#endif

#include <linux/socket.h>
#include <linux/tcp.h>

/* For the case that we have older kernel headers */
/* Copied from Linux kernel file include/linux/tcp.h */

#ifndef TCP_MD5SIG

#define TCP_MD5SIG  14
#define TCP_MD5SIG_MAXKEYLEN 80

struct tcp_md5sig {
  struct  sockaddr_storage tcpm_addr;             /* address associated */
  __u16   __tcpm_pad1;                            /* zero */
  __u16   tcpm_keylen;                            /* key length */
  __u32   __tcpm_pad2;                            /* zero */
  __u8    tcpm_key[TCP_MD5SIG_MAXKEYLEN];         /* key (binary) */
};

#endif

static int
sk_set_md5_auth_int(sock *s, sockaddr *sa, char *passwd)
{
  struct tcp_md5sig md5;

  memset(&md5, 0, sizeof(md5));
  memcpy(&md5.tcpm_addr, (struct sockaddr *) sa, sizeof(*sa));

  if (passwd)
    {
      int len = strlen(passwd);

      if (len > TCP_MD5SIG_MAXKEYLEN)
	{
	  log(L_ERR "MD5 password too long");
	  return -1;
	}

      md5.tcpm_keylen = len;
      memcpy(&md5.tcpm_key, passwd, len);
    }

  int rv = setsockopt(s->fd, IPPROTO_TCP, TCP_MD5SIG, &md5, sizeof(md5));

  if (rv < 0) 
    {
      if (errno == ENOPROTOOPT)
	log(L_ERR "Kernel does not support TCP MD5 signatures");
      else
	log(L_ERR "sk_set_md5_auth_int: setsockopt: %m");
    }

  return rv;
}
