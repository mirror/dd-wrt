/*
 *	BIRD Internet Routing Daemon -- NetBSD Multicasting and Network Includes
 *
 *	(c) 2004       Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifdef IPV6

static inline void
set_inaddr(struct in6_addr * ia, ip_addr a)
{
	ipa_hton(a);
	memcpy(ia, &a, sizeof(a));
}

#else

#include <net/if.h>

static inline void
set_inaddr(struct in_addr * ia, ip_addr a)
{
	ipa_hton(a);
	memcpy(&ia->s_addr, &a, sizeof(a));
}

static inline char *
sysio_mcast_setup(sock * s)
{
	u8              zero = 0;
	u8		one = 1;

	if (ipa_nonzero(s->daddr)) {

		if (setsockopt(s->fd, IPPROTO_IP, IP_MULTICAST_LOOP, &zero, sizeof(zero)) < 0)
			return "IP_MULTICAST_LOOP";

		if (setsockopt(s->fd, IPPROTO_IP, IP_MULTICAST_TTL, &one, sizeof(one)) < 0)
			return "IP_MULTICAST_TTL";
	}
	return NULL;
}


static inline char *
sysio_mcast_join(sock * s)
{
	struct in_addr  m;
	struct ip_mreq  mreq;
	char   *err;

        set_inaddr(&m, s->iface->addr->ip );

	bzero(&mreq, sizeof(mreq));
	set_inaddr(&mreq.imr_interface, s->iface->addr->ip);
	set_inaddr(&mreq.imr_multiaddr, s->daddr);

	/* And this one sets interface for _receiving_ multicasts from */
	if (ipa_nonzero(s->daddr) &&
            setsockopt(s->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
		return "IP_ADD_MEMBERSHIP";


	/* This defines where should we send _outgoing_ multicasts */
	if (ipa_nonzero(s->daddr) && setsockopt(s->fd, IPPROTO_IP, IP_MULTICAST_IF, &m, sizeof(m)) < 0)
		return "IP_MULTICAST_IF";

	if (err = sysio_mcast_setup(s))
		return err;

	return NULL;
}

#endif


#include <netinet/tcp.h>
#ifndef TCP_KEYLEN_MAX
#define TCP_KEYLEN_MAX 80
#endif
#ifndef TCP_SIG_SPI
#define TCP_SIG_SPI 0x1000
#endif

/* 
 * FIXME: Passwords has to be set by setkey(8) command. This is the same
 * behaviour like Quagga. We need to add code for SA/SP entries
 * management.
 */

static int
sk_set_md5_auth_int(sock *s, sockaddr *sa, char *passwd)
{
  int enable = 0;
  if (passwd)
    {
      int len = strlen(passwd);

      enable = len ? TCP_SIG_SPI : 0;

      if (len > TCP_KEYLEN_MAX)
	{
	  log(L_ERR "MD5 password too long");
	  return -1;
	}
    }

  int rv = setsockopt(s->fd, IPPROTO_TCP, TCP_MD5SIG, &enable, sizeof(enable));

  if (rv < 0) 
    {
      if (errno == ENOPROTOOPT)
	log(L_ERR "Kernel does not support TCP MD5 signatures");
      else
	log(L_ERR "sk_set_md5_auth_int: setsockopt: %m");
    }

  return rv;
}
