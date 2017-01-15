/*
 *	BIRD -- Linux Netlink Interface
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <errno.h>

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/route.h"
#include "nest/protocol.h"
#include "nest/iface.h"
#include "lib/timer.h"
#include "lib/unix.h"
#include "lib/krt.h"
#include "lib/socket.h"
#include "lib/string.h"
#include "lib/hash.h"
#include "conf/conf.h"

#include <asm/types.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>


#ifndef MSG_TRUNC			/* Hack: Several versions of glibc miss this one :( */
#define MSG_TRUNC 0x20
#endif

#ifndef IFA_FLAGS
#define IFA_FLAGS 8
#endif

#ifndef IFF_LOWER_UP
#define IFF_LOWER_UP 0x10000
#endif

#ifndef RTA_TABLE
#define RTA_TABLE  15
#endif


#ifdef IPV6
#define krt_ecmp6(X) 1
#else
#define krt_ecmp6(X) 0
#endif

/*
 * Structure nl_parse_state keeps state of received route processing. Ideally,
 * we could just independently parse received Netlink messages and immediately
 * propagate received routes to the rest of BIRD, but Linux kernel represents
 * and announces IPv6 ECMP routes not as one route with multiple next hops (like
 * RTA_MULTIPATH in IPv4 ECMP), but as a set of routes with the same prefix.
 *
 * Therefore, BIRD keeps currently processed route in nl_parse_state structure
 * and postpones its propagation until we expect it to be final; i.e., when
 * non-matching route is received or when the scan ends. When another matching
 * route is received, it is merged with the already processed route to form an
 * ECMP route. Note that merging is done only for IPv6 (merge == 1), but the
 * postponing is done in both cases (for simplicity). All IPv4 routes are just
 * considered non-matching.
 *
 * This is ignored for asynchronous notifications (every notification is handled
 * as a separate route). It is not an issue for our routes, as we ignore such
 * notifications anyways. But importing alien IPv6 ECMP routes does not work
 * properly.
 */

struct nl_parse_state
{
  struct linpool *pool;
  int scan;
  int merge;

  net *net;
  rta *attrs;
  struct krt_proto *proto;
  s8 new;
  s8 krt_src;
  u8 krt_type;
  u8 krt_proto;
  u32 krt_metric;
};

/*
 *	Synchronous Netlink interface
 */

struct nl_sock
{
  int fd;
  u32 seq;
  byte *rx_buffer;			/* Receive buffer */
  struct nlmsghdr *last_hdr;		/* Recently received packet */
  uint last_size;
};

#define NL_RX_SIZE 8192

#define NL_OP_DELETE	0
#define NL_OP_ADD	(NLM_F_CREATE|NLM_F_EXCL)
#define NL_OP_REPLACE	(NLM_F_CREATE|NLM_F_REPLACE)
#define NL_OP_APPEND	(NLM_F_CREATE|NLM_F_APPEND)

static linpool *nl_linpool;

static struct nl_sock nl_scan = {.fd = -1};	/* Netlink socket for synchronous scan */
static struct nl_sock nl_req  = {.fd = -1};	/* Netlink socket for requests */

static void
nl_open_sock(struct nl_sock *nl)
{
  if (nl->fd < 0)
    {
      nl->fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
      if (nl->fd < 0)
	die("Unable to open rtnetlink socket: %m");
      nl->seq = now;
      nl->rx_buffer = xmalloc(NL_RX_SIZE);
      nl->last_hdr = NULL;
      nl->last_size = 0;
    }
}

static void
nl_open(void)
{
  nl_open_sock(&nl_scan);
  nl_open_sock(&nl_req);
}

static void
nl_send(struct nl_sock *nl, struct nlmsghdr *nh)
{
  struct sockaddr_nl sa;

  memset(&sa, 0, sizeof(sa));
  sa.nl_family = AF_NETLINK;
  nh->nlmsg_pid = 0;
  nh->nlmsg_seq = ++(nl->seq);
  if (sendto(nl->fd, nh, nh->nlmsg_len, 0, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    die("rtnetlink sendto: %m");
  nl->last_hdr = NULL;
}

static void
nl_request_dump(int af, int cmd)
{
  struct {
    struct nlmsghdr nh;
    struct rtgenmsg g;
  } req = {
    .nh.nlmsg_type = cmd,
    .nh.nlmsg_len = sizeof(req),
    .nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP,
    .g.rtgen_family = af
  };
  nl_send(&nl_scan, &req.nh);
}

static struct nlmsghdr *
nl_get_reply(struct nl_sock *nl)
{
  for(;;)
    {
      if (!nl->last_hdr)
	{
	  struct iovec iov = { nl->rx_buffer, NL_RX_SIZE };
	  struct sockaddr_nl sa;
	  struct msghdr m = {
	    .msg_name = &sa,
	    .msg_namelen = sizeof(sa),
	    .msg_iov = &iov,
	    .msg_iovlen = 1,
	  };
	  int x = recvmsg(nl->fd, &m, 0);
	  if (x < 0)
	    die("nl_get_reply: %m");
	  if (sa.nl_pid)		/* It isn't from the kernel */
	    {
	      DBG("Non-kernel packet\n");
	      continue;
	    }
	  nl->last_size = x;
	  nl->last_hdr = (void *) nl->rx_buffer;
	  if (m.msg_flags & MSG_TRUNC)
	    bug("nl_get_reply: got truncated reply which should be impossible");
	}
      if (NLMSG_OK(nl->last_hdr, nl->last_size))
	{
	  struct nlmsghdr *h = nl->last_hdr;
	  nl->last_hdr = NLMSG_NEXT(h, nl->last_size);
	  if (h->nlmsg_seq != nl->seq)
	    {
	      log(L_WARN "nl_get_reply: Ignoring out of sequence netlink packet (%x != %x)",
		  h->nlmsg_seq, nl->seq);
	      continue;
	    }
	  return h;
	}
      if (nl->last_size)
	log(L_WARN "nl_get_reply: Found packet remnant of size %d", nl->last_size);
      nl->last_hdr = NULL;
    }
}

static struct tbf rl_netlink_err = TBF_DEFAULT_LOG_LIMITS;

static int
nl_error(struct nlmsghdr *h, int ignore_esrch)
{
  struct nlmsgerr *e;
  int ec;

  if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr)))
    {
      log(L_WARN "Netlink: Truncated error message received");
      return ENOBUFS;
    }
  e = (struct nlmsgerr *) NLMSG_DATA(h);
  ec = -e->error;
  if (ec && !(ignore_esrch && (ec == ESRCH)))
    log_rl(&rl_netlink_err, L_WARN "Netlink: %s", strerror(ec));
  return ec;
}

static struct nlmsghdr *
nl_get_scan(void)
{
  struct nlmsghdr *h = nl_get_reply(&nl_scan);

  if (h->nlmsg_type == NLMSG_DONE)
    return NULL;
  if (h->nlmsg_type == NLMSG_ERROR)
    {
      nl_error(h, 0);
      return NULL;
    }
  return h;
}

static int
nl_exchange(struct nlmsghdr *pkt, int ignore_esrch)
{
  struct nlmsghdr *h;

  nl_send(&nl_req, pkt);
  for(;;)
    {
      h = nl_get_reply(&nl_req);
      if (h->nlmsg_type == NLMSG_ERROR)
	break;
      log(L_WARN "nl_exchange: Unexpected reply received");
    }
  return nl_error(h, ignore_esrch) ? -1 : 0;
}

/*
 *	Netlink attributes
 */

static int nl_attr_len;

static void *
nl_checkin(struct nlmsghdr *h, int lsize)
{
  nl_attr_len = h->nlmsg_len - NLMSG_LENGTH(lsize);
  if (nl_attr_len < 0)
    {
      log(L_ERR "nl_checkin: underrun by %d bytes", -nl_attr_len);
      return NULL;
    }
  return NLMSG_DATA(h);
}

struct nl_want_attrs {
  u8 defined:1;
  u8 checksize:1;
  u8 size;
};


#define BIRD_IFLA_MAX (IFLA_WIRELESS+1)

static struct nl_want_attrs ifla_attr_want[BIRD_IFLA_MAX] = {
  [IFLA_IFNAME]	  = { 1, 0, 0 },
  [IFLA_MTU]	  = { 1, 1, sizeof(u32) },
  [IFLA_WIRELESS] = { 1, 0, 0 },
};


#define BIRD_IFA_MAX  (IFA_FLAGS+1)

#ifndef IPV6
static struct nl_want_attrs ifa_attr_want4[BIRD_IFA_MAX] = {
  [IFA_ADDRESS]	  = { 1, 1, sizeof(ip4_addr) },
  [IFA_LOCAL]	  = { 1, 1, sizeof(ip4_addr) },
  [IFA_BROADCAST] = { 1, 1, sizeof(ip4_addr) },
  [IFA_FLAGS]	  = { 1, 1, sizeof(u32) },
};
#else
static struct nl_want_attrs ifa_attr_want6[BIRD_IFA_MAX] = {
  [IFA_ADDRESS]	  = { 1, 1, sizeof(ip6_addr) },
  [IFA_LOCAL]	  = { 1, 1, sizeof(ip6_addr) },
  [IFA_FLAGS]	  = { 1, 1, sizeof(u32) },
};
#endif


#define BIRD_RTA_MAX  (RTA_TABLE+1)

static struct nl_want_attrs mpnh_attr_want4[BIRD_RTA_MAX] = {
  [RTA_GATEWAY]	  = { 1, 1, sizeof(ip4_addr) },
};

#ifndef IPV6
static struct nl_want_attrs rtm_attr_want4[BIRD_RTA_MAX] = {
  [RTA_DST]	  = { 1, 1, sizeof(ip4_addr) },
  [RTA_OIF]	  = { 1, 1, sizeof(u32) },
  [RTA_GATEWAY]	  = { 1, 1, sizeof(ip4_addr) },
  [RTA_PRIORITY]  = { 1, 1, sizeof(u32) },
  [RTA_PREFSRC]	  = { 1, 1, sizeof(ip4_addr) },
  [RTA_METRICS]	  = { 1, 0, 0 },
  [RTA_MULTIPATH] = { 1, 0, 0 },
  [RTA_FLOW]	  = { 1, 1, sizeof(u32) },
  [RTA_TABLE]	  = { 1, 1, sizeof(u32) },
};
#else
static struct nl_want_attrs rtm_attr_want6[BIRD_RTA_MAX] = {
  [RTA_DST]	  = { 1, 1, sizeof(ip6_addr) },
  [RTA_IIF]	  = { 1, 1, sizeof(u32) },
  [RTA_OIF]	  = { 1, 1, sizeof(u32) },
  [RTA_GATEWAY]	  = { 1, 1, sizeof(ip6_addr) },
  [RTA_PRIORITY]  = { 1, 1, sizeof(u32) },
  [RTA_PREFSRC]	  = { 1, 1, sizeof(ip6_addr) },
  [RTA_METRICS]	  = { 1, 0, 0 },
  [RTA_FLOW]	  = { 1, 1, sizeof(u32) },
  [RTA_TABLE]	  = { 1, 1, sizeof(u32) },
};
#endif


static int
nl_parse_attrs(struct rtattr *a, struct nl_want_attrs *want, struct rtattr **k, int ksize)
{
  int max = ksize / sizeof(struct rtattr *);
  bzero(k, ksize);

  for ( ; RTA_OK(a, nl_attr_len); a = RTA_NEXT(a, nl_attr_len))
    {
      if ((a->rta_type >= max) || !want[a->rta_type].defined)
	continue;

      if (want[a->rta_type].checksize && (RTA_PAYLOAD(a) != want[a->rta_type].size))
	{
	  log(L_ERR "nl_parse_attrs: Malformed message received");
	  return 0;
	}

      k[a->rta_type] = a;
    }

  if (nl_attr_len)
    {
      log(L_ERR "nl_parse_attrs: remnant of size %d", nl_attr_len);
      return 0;
    }

  return 1;
}

static inline u32 rta_get_u32(struct rtattr *a)
{ return *(u32 *) RTA_DATA(a); }

static inline ip4_addr rta_get_ip4(struct rtattr *a)
{ return ip4_ntoh(*(ip4_addr *) RTA_DATA(a)); }

static inline ip6_addr rta_get_ip6(struct rtattr *a)
{ return ip6_ntoh(*(ip6_addr *) RTA_DATA(a)); }


struct rtattr *
nl_add_attr(struct nlmsghdr *h, uint bufsize, uint code, const void *data, uint dlen)
{
  uint pos = NLMSG_ALIGN(h->nlmsg_len);
  uint len = RTA_LENGTH(dlen);

  if (pos + len > bufsize)
    bug("nl_add_attr: packet buffer overflow");

  struct rtattr *a = (struct rtattr *)((char *)h + pos);
  a->rta_type = code;
  a->rta_len = len;
  h->nlmsg_len = pos + len;

  if (dlen > 0)
    memcpy(RTA_DATA(a), data, dlen);

  return a;
}

static inline void
nl_add_attr_u32(struct nlmsghdr *h, unsigned bufsize, int code, u32 data)
{
  nl_add_attr(h, bufsize, code, &data, 4);
}

static inline void
nl_add_attr_ipa(struct nlmsghdr *h, unsigned bufsize, int code, ip_addr ipa)
{
  ipa_hton(ipa);
  nl_add_attr(h, bufsize, code, &ipa, sizeof(ipa));
}

static inline struct rtattr *
nl_open_attr(struct nlmsghdr *h, uint bufsize, uint code)
{
  return nl_add_attr(h, bufsize, code, NULL, 0);
}

static inline void
nl_close_attr(struct nlmsghdr *h, struct rtattr *a)
{
  a->rta_len = (void *)h + NLMSG_ALIGN(h->nlmsg_len) - (void *)a;
}

static inline struct rtnexthop *
nl_open_nexthop(struct nlmsghdr *h, uint bufsize)
{
  uint pos = NLMSG_ALIGN(h->nlmsg_len);
  uint len = RTNH_LENGTH(0);

  if (pos + len > bufsize)
    bug("nl_open_nexthop: packet buffer overflow");

  h->nlmsg_len = pos + len;

  return (void *)h + pos;
}

static inline void
nl_close_nexthop(struct nlmsghdr *h, struct rtnexthop *nh)
{
  nh->rtnh_len = (void *)h + NLMSG_ALIGN(h->nlmsg_len) - (void *)nh;
}

static void
nl_add_multipath(struct nlmsghdr *h, unsigned bufsize, struct mpnh *nh)
{
  struct rtattr *a = nl_open_attr(h, bufsize, RTA_MULTIPATH);

  for (; nh; nh = nh->next)
  {
    struct rtnexthop *rtnh = nl_open_nexthop(h, bufsize);

    rtnh->rtnh_flags = 0;
    rtnh->rtnh_hops = nh->weight;
    rtnh->rtnh_ifindex = nh->iface->index;

    nl_add_attr_ipa(h, bufsize, RTA_GATEWAY, nh->gw);

    nl_close_nexthop(h, rtnh);
  }

  nl_close_attr(h, a);
}

static struct mpnh *
nl_parse_multipath(struct krt_proto *p, struct rtattr *ra)
{
  /* Temporary buffer for multicast nexthops */
  static struct mpnh *nh_buffer;
  static int nh_buf_size;	/* in number of structures */
  static int nh_buf_used;

  struct rtattr *a[BIRD_RTA_MAX];
  struct rtnexthop *nh = RTA_DATA(ra);
  struct mpnh *rv, *first, **last;
  unsigned len = RTA_PAYLOAD(ra);

  first = NULL;
  last = &first;
  nh_buf_used = 0;

  while (len)
    {
      /* Use RTNH_OK(nh,len) ?? */
      if ((len < sizeof(*nh)) || (len < nh->rtnh_len))
	return NULL;

      if (nh_buf_used == nh_buf_size)
      {
	nh_buf_size = nh_buf_size ? (nh_buf_size * 2) : 4;
	nh_buffer = xrealloc(nh_buffer, nh_buf_size * sizeof(struct mpnh));
      }
      *last = rv = nh_buffer + nh_buf_used++;
      rv->next = NULL;
      last = &(rv->next);

      rv->weight = nh->rtnh_hops;
      rv->iface = if_find_by_index(nh->rtnh_ifindex);
      if (!rv->iface)
	return NULL;

      /* Nonexistent RTNH_PAYLOAD ?? */
      nl_attr_len = nh->rtnh_len - RTNH_LENGTH(0);
      nl_parse_attrs(RTNH_DATA(nh), mpnh_attr_want4, a, sizeof(a));
      if (a[RTA_GATEWAY])
	{
	  memcpy(&rv->gw, RTA_DATA(a[RTA_GATEWAY]), sizeof(ip_addr));
	  ipa_ntoh(rv->gw);

	  neighbor *ng = neigh_find2(&p->p, &rv->gw, rv->iface,
				     (nh->rtnh_flags & RTNH_F_ONLINK) ? NEF_ONLINK : 0);
	  if (!ng || (ng->scope == SCOPE_HOST))
	    return NULL;
	}
      else
	return NULL;

      len -= NLMSG_ALIGN(nh->rtnh_len);
      nh = RTNH_NEXT(nh);
    }

  return first;
}

static void
nl_add_metrics(struct nlmsghdr *h, uint bufsize, u32 *metrics, int max)
{
  struct rtattr *a = nl_open_attr(h, bufsize, RTA_METRICS);
  int t;

  for (t = 1; t < max; t++)
    if (metrics[0] & (1 << t))
      nl_add_attr_u32(h, bufsize, t, metrics[t]);

  nl_close_attr(h, a);
}

static int
nl_parse_metrics(struct rtattr *hdr, u32 *metrics, int max)
{
  struct rtattr *a = RTA_DATA(hdr);
  int len = RTA_PAYLOAD(hdr);

  metrics[0] = 0;
  for (; RTA_OK(a, len); a = RTA_NEXT(a, len))
  {
    if (a->rta_type == RTA_UNSPEC)
      continue;

    if (a->rta_type >= max)
      continue;

    if (RTA_PAYLOAD(a) != 4)
      return -1;

    metrics[0] |= 1 << a->rta_type;
    metrics[a->rta_type] = rta_get_u32(a);
  }

  if (len > 0)
    return -1;

  return 0;
}


/*
 *	Scanning of interfaces
 */

static void
nl_parse_link(struct nlmsghdr *h, int scan)
{
  struct ifinfomsg *i;
  struct rtattr *a[BIRD_IFLA_MAX];
  int new = h->nlmsg_type == RTM_NEWLINK;
  struct iface f = {};
  struct iface *ifi;
  char *name;
  u32 mtu;
  uint fl;

  if (!(i = nl_checkin(h, sizeof(*i))) || !nl_parse_attrs(IFLA_RTA(i), ifla_attr_want, a, sizeof(a)))
    return;
  if (!a[IFLA_IFNAME] || (RTA_PAYLOAD(a[IFLA_IFNAME]) < 2) || !a[IFLA_MTU])
    {
      /*
       * IFLA_IFNAME and IFLA_MTU are required, in fact, but there may also come
       * a message with IFLA_WIRELESS set, where (e.g.) no IFLA_IFNAME exists.
       * We simply ignore all such messages with IFLA_WIRELESS without notice.
       */

      if (a[IFLA_WIRELESS])
	return;

      log(L_ERR "KIF: Malformed message received");
      return;
    }

  name = RTA_DATA(a[IFLA_IFNAME]);
  mtu = rta_get_u32(a[IFLA_MTU]);

  ifi = if_find_by_index(i->ifi_index);
  if (!new)
    {
      DBG("KIF: IF%d(%s) goes down\n", i->ifi_index, name);
      if (!ifi)
	return;

      if_delete(ifi);
    }
  else
    {
      DBG("KIF: IF%d(%s) goes up (mtu=%d,flg=%x)\n", i->ifi_index, name, mtu, i->ifi_flags);
      if (ifi && strncmp(ifi->name, name, sizeof(ifi->name)-1))
	if_delete(ifi);

      strncpy(f.name, name, sizeof(f.name)-1);
      f.index = i->ifi_index;
      f.mtu = mtu;

      fl = i->ifi_flags;
      if (fl & IFF_UP)
	f.flags |= IF_ADMIN_UP;
      if (fl & IFF_LOWER_UP)
	f.flags |= IF_LINK_UP;
      if (fl & IFF_LOOPBACK)		/* Loopback */
	f.flags |= IF_MULTIACCESS | IF_LOOPBACK | IF_IGNORE;
      else if (fl & IFF_POINTOPOINT)	/* PtP */
	f.flags |= IF_MULTICAST;
      else if (fl & IFF_BROADCAST)	/* Broadcast */
	f.flags |= IF_MULTIACCESS | IF_BROADCAST | IF_MULTICAST;
      else
	f.flags |= IF_MULTIACCESS;	/* NBMA */

      if (fl & IFF_MULTICAST)
	f.flags |= IF_MULTICAST;

      ifi = if_update(&f);

      if (!scan)
	if_end_partial_update(ifi);
    }
}

static void
nl_parse_addr(struct nlmsghdr *h, int scan)
{
  struct ifaddrmsg *i;
  struct rtattr *a[BIRD_IFA_MAX];
  int new = h->nlmsg_type == RTM_NEWADDR;
  struct ifa ifa;
  struct iface *ifi;
  int scope;
  u32 ifa_flags;

  if (!(i = nl_checkin(h, sizeof(*i))))
    return;

  switch (i->ifa_family)
    {
#ifndef IPV6
      case AF_INET:
	if (!nl_parse_attrs(IFA_RTA(i), ifa_attr_want4, a, sizeof(a)))
	  return;
	if (!a[IFA_LOCAL])
	  {
	    log(L_ERR "KIF: Malformed message received (missing IFA_LOCAL)");
	    return;
	  }
	break;
#else
      case AF_INET6:
	if (!nl_parse_attrs(IFA_RTA(i), ifa_attr_want6, a, sizeof(a)))
	  return;
	break;
#endif
      default:
	return;
    }

  if (!a[IFA_ADDRESS])
    {
      log(L_ERR "KIF: Malformed message received (missing IFA_ADDRESS)");
      return;
    }

  if (a[IFA_FLAGS])
    ifa_flags = rta_get_u32(a[IFA_FLAGS]);
  else
    ifa_flags = i->ifa_flags;

  ifi = if_find_by_index(i->ifa_index);
  if (!ifi)
    {
      log(L_ERR "KIF: Received address message for unknown interface %d", i->ifa_index);
      return;
    }

  bzero(&ifa, sizeof(ifa));
  ifa.iface = ifi;
  if (ifa_flags & IFA_F_SECONDARY)
    ifa.flags |= IA_SECONDARY;

#ifdef IPV6
  /* Ignore tentative addresses silently */
  if (ifa_flags & IFA_F_TENTATIVE)
    return;
#endif

  /* IFA_LOCAL can be unset for IPv6 interfaces */
  memcpy(&ifa.ip, RTA_DATA(a[IFA_LOCAL] ? : a[IFA_ADDRESS]), sizeof(ifa.ip));
  ipa_ntoh(ifa.ip);
  ifa.pxlen = i->ifa_prefixlen;
  if (i->ifa_prefixlen > BITS_PER_IP_ADDRESS)
    {
      log(L_ERR "KIF: Invalid prefix length for interface %s: %d", ifi->name, i->ifa_prefixlen);
      new = 0;
    }
  if (i->ifa_prefixlen == BITS_PER_IP_ADDRESS)
    {
      ip_addr addr;
      memcpy(&addr, RTA_DATA(a[IFA_ADDRESS]), sizeof(addr));
      ipa_ntoh(addr);
      ifa.prefix = ifa.brd = addr;

      /* It is either a host address or a peer address */
      if (ipa_equal(ifa.ip, addr))
	ifa.flags |= IA_HOST;
      else
	{
	  ifa.flags |= IA_PEER;
	  ifa.opposite = addr;
	}
    }
  else
    {
      ip_addr netmask = ipa_mkmask(ifa.pxlen);
      ifa.prefix = ipa_and(ifa.ip, netmask);
      ifa.brd = ipa_or(ifa.ip, ipa_not(netmask));
      if (i->ifa_prefixlen == BITS_PER_IP_ADDRESS - 1)
	ifa.opposite = ipa_opposite_m1(ifa.ip);

#ifndef IPV6
      if (i->ifa_prefixlen == BITS_PER_IP_ADDRESS - 2)
	ifa.opposite = ipa_opposite_m2(ifa.ip);

      if ((ifi->flags & IF_BROADCAST) && a[IFA_BROADCAST])
	{
	  ip_addr xbrd;
	  memcpy(&xbrd, RTA_DATA(a[IFA_BROADCAST]), sizeof(xbrd));
	  ipa_ntoh(xbrd);
	  if (ipa_equal(xbrd, ifa.prefix) || ipa_equal(xbrd, ifa.brd))
	    ifa.brd = xbrd;
	  else if (ifi->flags & IF_TMP_DOWN) /* Complain only during the first scan */
	    log(L_ERR "KIF: Invalid broadcast address %I for %s", xbrd, ifi->name);
	}
#endif
    }

  scope = ipa_classify(ifa.ip);
  if (scope < 0)
    {
      log(L_ERR "KIF: Invalid interface address %I for %s", ifa.ip, ifi->name);
      return;
    }
  ifa.scope = scope & IADDR_SCOPE_MASK;

  DBG("KIF: IF%d(%s): %s IPA %I, flg %x, net %I/%d, brd %I, opp %I\n",
      ifi->index, ifi->name,
      new ? "added" : "removed",
      ifa.ip, ifa.flags, ifa.prefix, ifa.pxlen, ifa.brd, ifa.opposite);

  if (new)
    ifa_update(&ifa);
  else
    ifa_delete(&ifa);

  if (!scan)
    if_end_partial_update(ifi);
}

void
kif_do_scan(struct kif_proto *p UNUSED)
{
  struct nlmsghdr *h;

  if_start_update();

  nl_request_dump(AF_UNSPEC, RTM_GETLINK);
  while (h = nl_get_scan())
    if (h->nlmsg_type == RTM_NEWLINK || h->nlmsg_type == RTM_DELLINK)
      nl_parse_link(h, 1);
    else
      log(L_DEBUG "nl_scan_ifaces: Unknown packet received (type=%d)", h->nlmsg_type);

  nl_request_dump(BIRD_AF, RTM_GETADDR);
  while (h = nl_get_scan())
    if (h->nlmsg_type == RTM_NEWADDR || h->nlmsg_type == RTM_DELADDR)
      nl_parse_addr(h, 1);
    else
      log(L_DEBUG "nl_scan_ifaces: Unknown packet received (type=%d)", h->nlmsg_type);

  if_end_update();
}

/*
 *	Routes
 */

static inline u32
krt_table_id(struct krt_proto *p)
{
  return KRT_CF->sys.table_id;
}

static HASH(struct krt_proto) nl_table_map;

#define RTH_FN(k)	u32_hash(k)
#define RTH_EQ(k1,k2)	k1 == k2
#define RTH_KEY(p)	krt_table_id(p)
#define RTH_NEXT(p)	p->sys.hash_next

#define RTH_REHASH		rth_rehash
#define RTH_PARAMS		/8, *2, 2, 2, 6, 20

HASH_DEFINE_REHASH_FN(RTH, struct krt_proto)

int
krt_capable(rte *e)
{
  rta *a = e->attrs;

  if (a->cast != RTC_UNICAST)
    return 0;

  switch (a->dest)
    {
    case RTD_ROUTER:
    case RTD_DEVICE:
      if (a->iface == NULL)
	return 0;
    case RTD_BLACKHOLE:
    case RTD_UNREACHABLE:
    case RTD_PROHIBIT:
    case RTD_MULTIPATH:
      break;
    default:
      return 0;
    }
  return 1;
}

static inline int
nh_bufsize(struct mpnh *nh)
{
  int rv = 0;
  for (; nh != NULL; nh = nh->next)
    rv += RTNH_LENGTH(RTA_LENGTH(sizeof(ip_addr)));
  return rv;
}

static int
nl_send_route(struct krt_proto *p, rte *e, struct ea_list *eattrs, int op, int dest, ip_addr gw, struct iface *iface)
{
  eattr *ea;
  net *net = e->net;
  rta *a = e->attrs;
  u32 priority = 0;

  struct {
    struct nlmsghdr h;
    struct rtmsg r;
    char buf[128 + KRT_METRICS_MAX*8 + nh_bufsize(a->nexthops)];
  } r;

  DBG("nl_send_route(%I/%d,op=%x)\n", net->n.prefix, net->n.pxlen, op);

  bzero(&r.h, sizeof(r.h));
  bzero(&r.r, sizeof(r.r));
  r.h.nlmsg_type = op ? RTM_NEWROUTE : RTM_DELROUTE;
  r.h.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  r.h.nlmsg_flags = op | NLM_F_REQUEST | NLM_F_ACK;

  r.r.rtm_family = BIRD_AF;
  r.r.rtm_dst_len = net->n.pxlen;
  r.r.rtm_protocol = RTPROT_BIRD;
  r.r.rtm_scope = RT_SCOPE_NOWHERE;
  nl_add_attr_ipa(&r.h, sizeof(r), RTA_DST, net->n.prefix);

  /*
   * Strange behavior for RTM_DELROUTE:
   * 1) rtm_family is ignored in IPv6, works for IPv4
   * 2) not setting RTA_PRIORITY is different from setting default value (on IPv6)
   * 3) not setting RTA_PRIORITY is equivalent to setting 0, which is wildcard
   */

  if (krt_table_id(p) < 256)
    r.r.rtm_table = krt_table_id(p);
  else
    nl_add_attr_u32(&r.h, sizeof(r), RTA_TABLE, krt_table_id(p));

  if (a->source == RTS_DUMMY)
    priority = e->u.krt.metric;
  else if (KRT_CF->sys.metric)
    priority = KRT_CF->sys.metric;
  else if ((op != NL_OP_DELETE) && (ea = ea_find(eattrs, EA_KRT_METRIC)))
    priority = ea->u.data;

  if (priority)
    nl_add_attr_u32(&r.h, sizeof(r), RTA_PRIORITY, priority);

  /* For route delete, we do not specify remaining route attributes */
  if (op == NL_OP_DELETE)
    goto dest;

  /* Default scope is LINK for device routes, UNIVERSE otherwise */
  if (ea = ea_find(eattrs, EA_KRT_SCOPE))
    r.r.rtm_scope = ea->u.data;
  else
    r.r.rtm_scope = (dest == RTD_DEVICE) ? RT_SCOPE_LINK : RT_SCOPE_UNIVERSE;

  if (ea = ea_find(eattrs, EA_KRT_PREFSRC))
    nl_add_attr_ipa(&r.h, sizeof(r), RTA_PREFSRC, *(ip_addr *)ea->u.ptr->data);

  if (ea = ea_find(eattrs, EA_KRT_REALM))
    nl_add_attr_u32(&r.h, sizeof(r), RTA_FLOW, ea->u.data);


  u32 metrics[KRT_METRICS_MAX];
  metrics[0] = 0;

  struct ea_walk_state ews = { .eattrs = eattrs };
  while (ea = ea_walk(&ews, EA_KRT_METRICS, KRT_METRICS_MAX))
  {
    int id = ea->id - EA_KRT_METRICS;
    metrics[0] |= 1 << id;
    metrics[id] = ea->u.data;
  }

  if (metrics[0])
    nl_add_metrics(&r.h, sizeof(r), metrics, KRT_METRICS_MAX);


dest:
  /* a->iface != NULL checked in krt_capable() for router and device routes */
  switch (dest)
    {
    case RTD_ROUTER:
      r.r.rtm_type = RTN_UNICAST;
      nl_add_attr_u32(&r.h, sizeof(r), RTA_OIF, iface->index);
      nl_add_attr_ipa(&r.h, sizeof(r), RTA_GATEWAY, gw);
      break;
    case RTD_DEVICE:
      r.r.rtm_type = RTN_UNICAST;
      nl_add_attr_u32(&r.h, sizeof(r), RTA_OIF, iface->index);
      break;
    case RTD_BLACKHOLE:
      r.r.rtm_type = RTN_BLACKHOLE;
      break;
    case RTD_UNREACHABLE:
      r.r.rtm_type = RTN_UNREACHABLE;
      break;
    case RTD_PROHIBIT:
      r.r.rtm_type = RTN_PROHIBIT;
      break;
    case RTD_MULTIPATH:
      r.r.rtm_type = RTN_UNICAST;
      nl_add_multipath(&r.h, sizeof(r), a->nexthops);
      break;
    case RTD_NONE:
      break;
    default:
      bug("krt_capable inconsistent with nl_send_route");
    }

  /* Ignore missing for DELETE */
  return nl_exchange(&r.h, (op == NL_OP_DELETE));
}

static inline int
nl_add_rte(struct krt_proto *p, rte *e, struct ea_list *eattrs)
{
  rta *a = e->attrs;
  int err = 0;

  if (krt_ecmp6(p) && (a->dest == RTD_MULTIPATH))
  {
    struct mpnh *nh = a->nexthops;

    err = nl_send_route(p, e, eattrs, NL_OP_ADD, RTD_ROUTER, nh->gw, nh->iface);
    if (err < 0)
      return err;

    for (nh = nh->next; nh; nh = nh->next)
      err += nl_send_route(p, e, eattrs, NL_OP_APPEND, RTD_ROUTER, nh->gw, nh->iface);

    return err;
  }

  return nl_send_route(p, e, eattrs, NL_OP_ADD, a->dest, a->gw, a->iface);
}

static inline int
nl_delete_rte(struct krt_proto *p, rte *e, struct ea_list *eattrs)
{
  int err = 0;

  /* For IPv6, we just repeatedly request DELETE until we get error */
  do
    err = nl_send_route(p, e, eattrs, NL_OP_DELETE, RTD_NONE, IPA_NONE, NULL);
  while (krt_ecmp6(p) && !err);

  return err;
}

void
krt_replace_rte(struct krt_proto *p, net *n, rte *new, rte *old, struct ea_list *eattrs)
{
  int err = 0;

  /*
   * We could use NL_OP_REPLACE, but route replace on Linux has some problems:
   *
   * 1) Does not check for matching rtm_protocol
   * 2) Has broken semantics for IPv6 ECMP
   * 3) Crashes some kernel version when used for IPv6 ECMP
   *
   * So we use NL_OP_DELETE and then NL_OP_ADD. We also do not trust the old
   * route value, so we do not try to optimize IPv6 ECMP reconfigurations.
   */

  if (old)
    nl_delete_rte(p, old, eattrs);

  if (new)
    err = nl_add_rte(p, new, eattrs);

  if (err < 0)
    n->n.flags |= KRF_SYNC_ERROR;
  else
    n->n.flags &= ~KRF_SYNC_ERROR;
}


static inline struct mpnh *
nl_alloc_mpnh(struct nl_parse_state *s, ip_addr gw, struct iface *iface, byte weight)
{
  struct mpnh *nh = lp_alloc(s->pool, sizeof(struct mpnh));

  nh->gw = gw;
  nh->iface = iface;
  nh->next = NULL;
  nh->weight = weight;

  return nh;
}

static int
nl_mergable_route(struct nl_parse_state *s, net *net, struct krt_proto *p, uint priority, uint krt_type)
{
  /* Route merging must be active */
  if (!s->merge)
    return 0;

  /* Saved and new route must have same network, proto/table, and priority */
  if ((s->net != net) || (s->proto != p) || (s->krt_metric != priority))
    return 0;

  /* Both must be regular unicast routes */
  if ((s->krt_type != RTN_UNICAST) || (krt_type != RTN_UNICAST))
    return 0;

  return 1;
}

static void
nl_announce_route(struct nl_parse_state *s)
{
  rte *e = rte_get_temp(s->attrs);
  e->net = s->net;
  e->u.krt.src = s->krt_src;
  e->u.krt.proto = s->krt_proto;
  e->u.krt.seen = 0;
  e->u.krt.best = 0;
  e->u.krt.metric = s->krt_metric;

  if (s->scan)
    krt_got_route(s->proto, e);
  else
    krt_got_route_async(s->proto, e, s->new);

  s->net = NULL;
  s->attrs = NULL;
  s->proto = NULL;
  lp_flush(s->pool);
}

static inline void
nl_parse_begin(struct nl_parse_state *s, int scan, int merge)
{
  memset(s, 0, sizeof (struct nl_parse_state));
  s->pool = nl_linpool;
  s->scan = scan;
  s->merge = merge;
}

static inline void
nl_parse_end(struct nl_parse_state *s)
{
  if (s->net)
    nl_announce_route(s);
}


#define SKIP(ARG...) do { DBG("KRT: Ignoring route - " ARG); return; } while(0)

static void
nl_parse_route(struct nl_parse_state *s, struct nlmsghdr *h)
{
  struct krt_proto *p;
  struct rtmsg *i;
  struct rtattr *a[BIRD_RTA_MAX];
  int new = h->nlmsg_type == RTM_NEWROUTE;

  ip_addr dst = IPA_NONE;
  u32 oif = ~0;
  u32 table;
  u32 priority = 0;
  u32 def_scope = RT_SCOPE_UNIVERSE;
  int src;

  if (!(i = nl_checkin(h, sizeof(*i))))
    return;

  switch (i->rtm_family)
    {
#ifndef IPV6
      case AF_INET:
	if (!nl_parse_attrs(RTM_RTA(i), rtm_attr_want4, a, sizeof(a)))
	  return;
	break;
#else
      case AF_INET6:
	if (!nl_parse_attrs(RTM_RTA(i), rtm_attr_want6, a, sizeof(a)))
	  return;
	break;
#endif
      default:
	return;
    }

  if (a[RTA_DST])
    {
      memcpy(&dst, RTA_DATA(a[RTA_DST]), sizeof(dst));
      ipa_ntoh(dst);
    }

  if (a[RTA_OIF])
    oif = rta_get_u32(a[RTA_OIF]);

  if (a[RTA_TABLE])
    table = rta_get_u32(a[RTA_TABLE]);
  else
    table = i->rtm_table;

  p = HASH_FIND(nl_table_map, RTH, table); /* Do we know this table? */
  DBG("KRT: Got %I/%d, type=%d, oif=%d, table=%d, prid=%d, proto=%s\n", dst, i->rtm_dst_len, i->rtm_type, oif, table, i->rtm_protocol, p ? p->p.name : "(none)");
  if (!p)
    SKIP("unknown table %d\n", table);

#ifdef IPV6
  if (a[RTA_IIF])
    SKIP("IIF set\n");
#else
  if (i->rtm_tos != 0)			/* We don't support TOS */
    SKIP("TOS %02x\n", i->rtm_tos);
#endif

  if (s->scan && !new)
    SKIP("RTM_DELROUTE in scan\n");

  if (a[RTA_PRIORITY])
    priority = rta_get_u32(a[RTA_PRIORITY]);

  int c = ipa_classify_net(dst);
  if ((c < 0) || !(c & IADDR_HOST) || ((c & IADDR_SCOPE_MASK) <= SCOPE_LINK))
    SKIP("strange class/scope\n");

  switch (i->rtm_protocol)
    {
    case RTPROT_UNSPEC:
      SKIP("proto unspec\n");

    case RTPROT_REDIRECT:
      src = KRT_SRC_REDIRECT;
      break;

    case RTPROT_KERNEL:
      src = KRT_SRC_KERNEL;
      return;

    case RTPROT_BIRD:
      if (!s->scan)
	SKIP("echo\n");
      src = KRT_SRC_BIRD;
      break;

    case RTPROT_BOOT:
    default:
      src = KRT_SRC_ALIEN;
    }

  net *net = net_get(p->p.table, dst, i->rtm_dst_len);

  if (s->net && !nl_mergable_route(s, net, p, priority, i->rtm_type))
    nl_announce_route(s);

  rta *ra = lp_allocz(s->pool, sizeof(rta));
  ra->src = p->p.main_source;
  ra->source = RTS_INHERIT;
  ra->scope = SCOPE_UNIVERSE;
  ra->cast = RTC_UNICAST;

  switch (i->rtm_type)
    {
    case RTN_UNICAST:

      if (a[RTA_MULTIPATH] && (i->rtm_family == AF_INET))
	{
	  ra->dest = RTD_MULTIPATH;
	  ra->nexthops = nl_parse_multipath(p, a[RTA_MULTIPATH]);
	  if (!ra->nexthops)
	    {
	      log(L_ERR "KRT: Received strange multipath route %I/%d",
		  net->n.prefix, net->n.pxlen);
	      return;
	    }

	  break;
	}

      ra->iface = if_find_by_index(oif);
      if (!ra->iface)
	{
	  log(L_ERR "KRT: Received route %I/%d with unknown ifindex %u",
	      net->n.prefix, net->n.pxlen, oif);
	  return;
	}

      if (a[RTA_GATEWAY])
	{
	  neighbor *ng;
	  ra->dest = RTD_ROUTER;
	  memcpy(&ra->gw, RTA_DATA(a[RTA_GATEWAY]), sizeof(ra->gw));
	  ipa_ntoh(ra->gw);

#ifdef IPV6
	  /* Silently skip strange 6to4 routes */
	  if (ipa_in_net(ra->gw, IPA_NONE, 96))
	    return;
#endif

	  ng = neigh_find2(&p->p, &ra->gw, ra->iface,
			   (i->rtm_flags & RTNH_F_ONLINK) ? NEF_ONLINK : 0);
	  if (!ng || (ng->scope == SCOPE_HOST))
	    {
	      log(L_ERR "KRT: Received route %I/%d with strange next-hop %I",
		  net->n.prefix, net->n.pxlen, ra->gw);
	      return;
	    }
	}
      else
	{
	  ra->dest = RTD_DEVICE;
	  def_scope = RT_SCOPE_LINK;
	}

      break;
    case RTN_BLACKHOLE:
      ra->dest = RTD_BLACKHOLE;
      break;
    case RTN_UNREACHABLE:
      ra->dest = RTD_UNREACHABLE;
      break;
    case RTN_PROHIBIT:
      ra->dest = RTD_PROHIBIT;
      break;
    /* FIXME: What about RTN_THROW? */
    default:
      SKIP("type %d\n", i->rtm_type);
      return;
    }

  if (i->rtm_scope != def_scope)
    {
      ea_list *ea = lp_alloc(s->pool, sizeof(ea_list) + sizeof(eattr));
      ea->next = ra->eattrs;
      ra->eattrs = ea;
      ea->flags = EALF_SORTED;
      ea->count = 1;
      ea->attrs[0].id = EA_KRT_SCOPE;
      ea->attrs[0].flags = 0;
      ea->attrs[0].type = EAF_TYPE_INT;
      ea->attrs[0].u.data = i->rtm_scope;
    }

  if (a[RTA_PREFSRC])
    {
      ip_addr ps;
      memcpy(&ps, RTA_DATA(a[RTA_PREFSRC]), sizeof(ps));
      ipa_ntoh(ps);

      ea_list *ea = lp_alloc(s->pool, sizeof(ea_list) + sizeof(eattr));
      ea->next = ra->eattrs;
      ra->eattrs = ea;
      ea->flags = EALF_SORTED;
      ea->count = 1;
      ea->attrs[0].id = EA_KRT_PREFSRC;
      ea->attrs[0].flags = 0;
      ea->attrs[0].type = EAF_TYPE_IP_ADDRESS;
      ea->attrs[0].u.ptr = lp_alloc(s->pool, sizeof(struct adata) + sizeof(ps));
      ea->attrs[0].u.ptr->length = sizeof(ps);
      memcpy(ea->attrs[0].u.ptr->data, &ps, sizeof(ps));
    }

  if (a[RTA_FLOW])
    {
      ea_list *ea = lp_alloc(s->pool, sizeof(ea_list) + sizeof(eattr));
      ea->next = ra->eattrs;
      ra->eattrs = ea;
      ea->flags = EALF_SORTED;
      ea->count = 1;
      ea->attrs[0].id = EA_KRT_REALM;
      ea->attrs[0].flags = 0;
      ea->attrs[0].type = EAF_TYPE_INT;
      ea->attrs[0].u.data = rta_get_u32(a[RTA_FLOW]);
    }

  if (a[RTA_METRICS])
    {
      u32 metrics[KRT_METRICS_MAX];
      ea_list *ea = lp_alloc(s->pool, sizeof(ea_list) + KRT_METRICS_MAX * sizeof(eattr));
      int t, n = 0;

      if (nl_parse_metrics(a[RTA_METRICS], metrics, ARRAY_SIZE(metrics)) < 0)
        {
	  log(L_ERR "KRT: Received route %I/%d with strange RTA_METRICS attribute",
	      net->n.prefix, net->n.pxlen);
	  return;
	}

      for (t = 1; t < KRT_METRICS_MAX; t++)
	if (metrics[0] & (1 << t))
	  {
	    ea->attrs[n].id = EA_CODE(EAP_KRT, KRT_METRICS_OFFSET + t);
	    ea->attrs[n].flags = 0;
	    ea->attrs[n].type = EAF_TYPE_INT; /* FIXME: Some are EAF_TYPE_BITFIELD */
	    ea->attrs[n].u.data = metrics[t];
	    n++;
	  }

      if (n > 0)
        {
	  ea->next = ra->eattrs;
	  ea->flags = EALF_SORTED;
	  ea->count = n;
	  ra->eattrs = ea;
	}
    }

  /*
   * Ideally, now we would send the received route to the rest of kernel code.
   * But IPv6 ECMP routes are sent as a sequence of routes, so we postpone it
   * and merge next hops until the end of the sequence.
   */

  if (!s->net)
  {
    /* Store the new route */
    s->net = net;
    s->attrs = ra;
    s->proto = p;
    s->new = new;
    s->krt_src = src;
    s->krt_type = i->rtm_type;
    s->krt_proto = i->rtm_protocol;
    s->krt_metric = priority;
  }
  else
  {
    /* Merge next hops with the stored route */
    rta *a = s->attrs;

    if (a->dest != RTD_MULTIPATH)
    {
      a->dest = RTD_MULTIPATH;
      a->nexthops = nl_alloc_mpnh(s, a->gw, a->iface, 0);
    }

    mpnh_insert(&a->nexthops, nl_alloc_mpnh(s, ra->gw, ra->iface, 0));
  }
}

void
krt_do_scan(struct krt_proto *p UNUSED)	/* CONFIG_ALL_TABLES_AT_ONCE => p is NULL */
{
  struct nlmsghdr *h;
  struct nl_parse_state s;

  nl_parse_begin(&s, 1, krt_ecmp6(p));

  nl_request_dump(BIRD_AF, RTM_GETROUTE);
  while (h = nl_get_scan())
    if (h->nlmsg_type == RTM_NEWROUTE || h->nlmsg_type == RTM_DELROUTE)
      nl_parse_route(&s, h);
    else
      log(L_DEBUG "nl_scan_fire: Unknown packet received (type=%d)", h->nlmsg_type);

  nl_parse_end(&s);
}

/*
 *	Asynchronous Netlink interface
 */

static sock *nl_async_sk;		/* BIRD socket for asynchronous notifications */
static byte *nl_async_rx_buffer;	/* Receive buffer */

static void
nl_async_msg(struct nlmsghdr *h)
{
  struct nl_parse_state s;

  switch (h->nlmsg_type)
    {
    case RTM_NEWROUTE:
    case RTM_DELROUTE:
      DBG("KRT: Received async route notification (%d)\n", h->nlmsg_type);
      nl_parse_begin(&s, 0, 0);
      nl_parse_route(&s, h);
      nl_parse_end(&s);
      break;
    case RTM_NEWLINK:
    case RTM_DELLINK:
      DBG("KRT: Received async link notification (%d)\n", h->nlmsg_type);
      if (kif_proto)
	nl_parse_link(h, 0);
      break;
    case RTM_NEWADDR:
    case RTM_DELADDR:
      DBG("KRT: Received async address notification (%d)\n", h->nlmsg_type);
      if (kif_proto)
	nl_parse_addr(h, 0);
      break;
    default:
      DBG("KRT: Received unknown async notification (%d)\n", h->nlmsg_type);
    }
}

static int
nl_async_hook(sock *sk, uint size UNUSED)
{
  struct iovec iov = { nl_async_rx_buffer, NL_RX_SIZE };
  struct sockaddr_nl sa;
  struct msghdr m = {
    .msg_name = &sa,
    .msg_namelen = sizeof(sa),
    .msg_iov = &iov,
    .msg_iovlen = 1,
  };
  struct nlmsghdr *h;
  int x;
  uint len;

  x = recvmsg(sk->fd, &m, 0);
  if (x < 0)
    {
      if (errno == ENOBUFS)
	{
	  /*
	   *  Netlink reports some packets have been thrown away.
	   *  One day we might react to it by asking for route table
	   *  scan in near future.
	   */
	  log(L_WARN "Kernel dropped some netlink messages, will resync on next scan.");
	  return 1;	/* More data are likely to be ready */
	}
      else if (errno != EWOULDBLOCK)
	log(L_ERR "Netlink recvmsg: %m");
      return 0;
    }
  if (sa.nl_pid)		/* It isn't from the kernel */
    {
      DBG("Non-kernel packet\n");
      return 1;
    }
  h = (void *) nl_async_rx_buffer;
  len = x;
  if (m.msg_flags & MSG_TRUNC)
    {
      log(L_WARN "Netlink got truncated asynchronous message");
      return 1;
    }
  while (NLMSG_OK(h, len))
    {
      nl_async_msg(h);
      h = NLMSG_NEXT(h, len);
    }
  if (len)
    log(L_WARN "nl_async_hook: Found packet remnant of size %d", len);
  return 1;
}

static void
nl_async_err_hook(sock *sk, int e UNUSED)
{
  nl_async_hook(sk, 0);
}

static void
nl_open_async(void)
{
  sock *sk;
  struct sockaddr_nl sa;
  int fd;

  if (nl_async_sk)
    return;

  DBG("KRT: Opening async netlink socket\n");

  fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (fd < 0)
    {
      log(L_ERR "Unable to open asynchronous rtnetlink socket: %m");
      return;
    }

  bzero(&sa, sizeof(sa));
  sa.nl_family = AF_NETLINK;
#ifdef IPV6
  sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV6_IFADDR | RTMGRP_IPV6_ROUTE;
#else
  sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE;
#endif
  if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0)
    {
      log(L_ERR "Unable to bind asynchronous rtnetlink socket: %m");
      close(fd);
      return;
    }

  nl_async_rx_buffer = xmalloc(NL_RX_SIZE);

  sk = nl_async_sk = sk_new(krt_pool);
  sk->type = SK_MAGIC;
  sk->rx_hook = nl_async_hook;
  sk->err_hook = nl_async_err_hook;
  sk->fd = fd;
  if (sk_open(sk) < 0)
    bug("Netlink: sk_open failed");
}


/*
 *	Interface to the UNIX krt module
 */

void
krt_sys_io_init(void)
{
  nl_linpool = lp_new(krt_pool, 4080);
  HASH_INIT(nl_table_map, krt_pool, 6);
}

int
krt_sys_start(struct krt_proto *p)
{
  struct krt_proto *old = HASH_FIND(nl_table_map, RTH, krt_table_id(p));

  if (old)
    {
      log(L_ERR "%s: Kernel table %u already registered by %s",
	  p->p.name, krt_table_id(p), old->p.name);
      return 0;
    }

  HASH_INSERT2(nl_table_map, RTH, krt_pool, p);

  nl_open();
  nl_open_async();

  return 1;
}

void
krt_sys_shutdown(struct krt_proto *p)
{
  HASH_REMOVE2(nl_table_map, RTH, krt_pool, p);
}

int
krt_sys_reconfigure(struct krt_proto *p UNUSED, struct krt_config *n, struct krt_config *o)
{
  return (n->sys.table_id == o->sys.table_id) && (n->sys.metric == o->sys.metric);
}

void
krt_sys_init_config(struct krt_config *cf)
{
  cf->sys.table_id = RT_TABLE_MAIN;
  cf->sys.metric = 0;
}

void
krt_sys_copy_config(struct krt_config *d, struct krt_config *s)
{
  d->sys.table_id = s->sys.table_id;
  d->sys.metric = s->sys.metric;
}

static const char *krt_metrics_names[KRT_METRICS_MAX] = {
  NULL, "lock", "mtu", "window", "rtt", "rttvar", "sstresh", "cwnd", "advmss",
  "reordering", "hoplimit", "initcwnd", "features", "rto_min", "initrwnd", "quickack"
};

static const char *krt_features_names[KRT_FEATURES_MAX] = {
  "ecn", NULL, NULL, "allfrag"
};

int
krt_sys_get_attr(eattr *a, byte *buf, int buflen UNUSED)
{
  switch (a->id)
  {
  case EA_KRT_PREFSRC:
    bsprintf(buf, "prefsrc");
    return GA_NAME;

  case EA_KRT_REALM:
    bsprintf(buf, "realm");
    return GA_NAME;

  case EA_KRT_SCOPE:
    bsprintf(buf, "scope");
    return GA_NAME;

  case EA_KRT_LOCK:
    buf += bsprintf(buf, "lock:");
    ea_format_bitfield(a, buf, buflen, krt_metrics_names, 2, KRT_METRICS_MAX);
    return GA_FULL;

  case EA_KRT_FEATURES:
    buf += bsprintf(buf, "features:");
    ea_format_bitfield(a, buf, buflen, krt_features_names, 0, KRT_FEATURES_MAX);
    return GA_FULL;

  default:;
    int id = (int)EA_ID(a->id) - KRT_METRICS_OFFSET;
    if (id > 0 && id < KRT_METRICS_MAX)
    {
      bsprintf(buf, "%s", krt_metrics_names[id]);
      return GA_NAME;
    }

    return GA_UNKNOWN;
  }
}



void
kif_sys_start(struct kif_proto *p UNUSED)
{
  nl_open();
  nl_open_async();
}

void
kif_sys_shutdown(struct kif_proto *p UNUSED)
{
}
