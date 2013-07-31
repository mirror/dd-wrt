/*
 *	BIRD Internet Routing Daemon -- NetBSD Multicasting and Network Includes
 *
 *	(c) 2004       Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifdef __NetBSD__

#ifndef IP_RECVTTL
#define IP_RECVTTL 23
#endif

#ifndef IP_MINTTL
#define IP_MINTTL 24
#endif

#endif

#ifdef __DragonFly__
#define TCP_MD5SIG	TCP_SIGNATURE_ENABLE
#endif

#ifdef IPV6

static inline void
set_inaddr(struct in6_addr * ia, ip_addr a)
{
  ipa_hton(a);
  memcpy(ia, &a, sizeof(a));
}

static inline void
get_inaddr(ip_addr *a, struct in6_addr *ia)
{
  memcpy(a, ia, sizeof(*a));
  ipa_ntoh(*a);
}

static inline char *
sysio_bind_to_iface(sock *s)
{
  /* Unfortunately not available */
  return NULL;
}


#else

#include <net/if.h>
#include <net/if_dl.h>

static inline void
set_inaddr(struct in_addr * ia, ip_addr a)
{
  ipa_hton(a);
  memcpy(&ia->s_addr, &a, sizeof(a));
}

static inline void
get_inaddr(ip_addr *a, struct in_addr *ia)
{
  memcpy(a, &ia->s_addr, sizeof(*a));
  ipa_ntoh(*a);
}


/* BSD Multicast handling for IPv4 */

static inline char *
sysio_setup_multicast(sock *s)
{
	struct in_addr m;
	u8 zero = 0;
	u8 ttl = s->ttl;

	if (setsockopt(s->fd, IPPROTO_IP, IP_MULTICAST_LOOP, &zero, sizeof(zero)) < 0)
		return "IP_MULTICAST_LOOP";

	if (setsockopt(s->fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
		return "IP_MULTICAST_TTL";

	/* This defines where should we send _outgoing_ multicasts */
        set_inaddr(&m, s->iface->addr->ip);
	if (setsockopt(s->fd, IPPROTO_IP, IP_MULTICAST_IF, &m, sizeof(m)) < 0)
		return "IP_MULTICAST_IF";

	return NULL;
}


static inline char *
sysio_join_group(sock *s, ip_addr maddr)
{
	struct ip_mreq  mreq;

	bzero(&mreq, sizeof(mreq));
	set_inaddr(&mreq.imr_interface, s->iface->addr->ip);
	set_inaddr(&mreq.imr_multiaddr, maddr);

	/* And this one sets interface for _receiving_ multicasts from */
	if (setsockopt(s->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
		return "IP_ADD_MEMBERSHIP";

	return NULL;
}

static inline char *
sysio_leave_group(sock *s, ip_addr maddr)
{
	struct ip_mreq mreq;

	bzero(&mreq, sizeof(mreq));
	set_inaddr(&mreq.imr_interface, s->iface->addr->ip);
	set_inaddr(&mreq.imr_multiaddr, maddr);

	/* And this one sets interface for _receiving_ multicasts from */
	if (setsockopt(s->fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
		return "IP_DROP_MEMBERSHIP";

	return NULL;
}


/* BSD RX/TX packet info handling for IPv4 */
/* it uses IP_RECVDSTADDR / IP_RECVIF socket options instead of IP_PKTINFO */

#define CMSG_RX_SPACE (CMSG_SPACE(sizeof(struct in_addr)) + \
		       CMSG_SPACE(sizeof(struct sockaddr_dl)) + \
		       CMSG_SPACE(sizeof(char)))
#define CMSG_TX_SPACE CMSG_SPACE(sizeof(struct in_addr))

static char *
sysio_register_cmsgs(sock *s)
{
  int ok = 1;
  if (s->flags & SKF_LADDR_RX)
  {
    if (setsockopt(s->fd, IPPROTO_IP, IP_RECVDSTADDR, &ok, sizeof(ok)) < 0)
      return "IP_RECVDSTADDR";

    if (setsockopt(s->fd, IPPROTO_IP, IP_RECVIF, &ok, sizeof(ok)) < 0)
      return "IP_RECVIF";
  }

  if ((s->flags & SKF_TTL_RX) &&
      (setsockopt(s->fd, IPPROTO_IP, IP_RECVTTL, &ok, sizeof(ok)) < 0))
    return "IP_RECVTTL";


  return NULL;
}

static void
sysio_process_rx_cmsgs(sock *s, struct msghdr *msg)
{
  struct cmsghdr *cm;
  struct in_addr *ra = NULL;
  struct sockaddr_dl *ri = NULL;
  unsigned char *ttl = NULL;

  for (cm = CMSG_FIRSTHDR(msg); cm != NULL; cm = CMSG_NXTHDR(msg, cm))
  {
    if (cm->cmsg_level == IPPROTO_IP && cm->cmsg_type == IP_RECVDSTADDR)
      ra = (struct in_addr *) CMSG_DATA(cm);

    if (cm->cmsg_level == IPPROTO_IP && cm->cmsg_type == IP_RECVIF)
      ri = (struct sockaddr_dl *) CMSG_DATA(cm);

    if (cm->cmsg_level == IPPROTO_IP && cm->cmsg_type == IP_RECVTTL)
      ttl = (unsigned char *) CMSG_DATA(cm);
  }

  if (s->flags & SKF_LADDR_RX)
  {
    s->laddr = IPA_NONE;
    s->lifindex = 0;

    if (ra)
      get_inaddr(&s->laddr, ra);
    if (ri)
      s->lifindex = ri->sdl_index;
  }

  if (s->flags & SKF_TTL_RX)
    s->ttl = ttl ? *ttl : -1;

  // log(L_WARN "RX %I %d", s->laddr, s->lifindex);
}

/* Unfortunately, IP_SENDSRCADDR does not work for raw IP sockets on BSD kernels */
/*
static void
sysio_prepare_tx_cmsgs(sock *s, struct msghdr *msg, void *cbuf, size_t cbuflen)
{
  struct cmsghdr *cm;
  struct in_addr *sa;

  if (!(s->flags & SKF_LADDR_TX))
    return;

  msg->msg_control = cbuf;
  msg->msg_controllen = cbuflen;

  if (s->iface)
    {
      struct in_addr m;
      set_inaddr(&m, s->saddr);
      setsockopt(s->fd, IPPROTO_IP, IP_MULTICAST_IF, &m, sizeof(m));
    }

  cm = CMSG_FIRSTHDR(msg);
  cm->cmsg_level = IPPROTO_IP;
  cm->cmsg_type = IP_SENDSRCADDR;
  cm->cmsg_len = CMSG_LEN(sizeof(*sa));

  sa = (struct in_addr *) CMSG_DATA(cm);
  set_inaddr(sa, s->saddr);

  msg->msg_controllen = cm->cmsg_len;
}
*/

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


#ifndef IPV6

static int
sk_set_min_ttl4(sock *s, int ttl)
{
  if (setsockopt(s->fd, IPPROTO_IP, IP_MINTTL, &ttl, sizeof(ttl)) < 0)
  {
    if (errno == ENOPROTOOPT)
      log(L_ERR "Kernel does not support IPv4 TTL security");
    else
      log(L_ERR "sk_set_min_ttl4: setsockopt: %m");

    return -1;
  }

  return 0;
}

#else /* IPv6 */

static int
sk_set_min_ttl6(sock *s, int ttl)
{
  log(L_ERR "IPv6 TTL security not supported");
  return -1;
}

#endif


int sk_priority_control = -1;

static int
sk_set_priority(sock *s, int prio UNUSED)
{
  log(L_WARN "Socket priority not supported");
  return -1;
}
