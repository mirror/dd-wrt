/*
 *	BIRD -- Linux Netlink Interface
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <errno.h>

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/route.h"
#include "nest/protocol.h"
#include "nest/iface.h"
#include "lib/alloca.h"
#include "lib/timer.h"
#include "lib/unix.h"
#include "lib/krt.h"
#include "lib/socket.h"
#include "lib/string.h"
#include "conf/conf.h"

#include <asm/types.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#ifndef MSG_TRUNC			/* Hack: Several versions of glibc miss this one :( */
#define MSG_TRUNC 0x20
#endif

#ifndef IFF_LOWER_UP
#define IFF_LOWER_UP 0x10000
#endif

/*
 *	Synchronous Netlink interface
 */

struct nl_sock
{
  int fd;
  u32 seq;
  byte *rx_buffer;			/* Receive buffer */
  struct nlmsghdr *last_hdr;		/* Recently received packet */
  unsigned int last_size;
};

#define NL_RX_SIZE 8192

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
nl_request_dump(int cmd)
{
  struct {
    struct nlmsghdr nh;
    struct rtgenmsg g;
  } req;
  req.nh.nlmsg_type = cmd;
  req.nh.nlmsg_len = sizeof(req);
  req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  /* Is it important which PF_* is used for link-level interface scan?
     It seems that some information is available only when PF_INET is used. */
  req.g.rtgen_family = (cmd == RTM_GETLINK) ? PF_INET : BIRD_PF;
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
	  struct msghdr m = { (struct sockaddr *) &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
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

static struct rate_limit rl_netlink_err;

static int
nl_error(struct nlmsghdr *h)
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
  if (ec)
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
      nl_error(h);
      return NULL;
    }
  return h;
}

static int
nl_exchange(struct nlmsghdr *pkt)
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
  return nl_error(h) ? -1 : 0;
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

static int
nl_parse_attrs(struct rtattr *a, struct rtattr **k, int ksize)
{
  int max = ksize / sizeof(struct rtattr *);
  bzero(k, ksize);
  while (RTA_OK(a, nl_attr_len))
    {
      if (a->rta_type < max)
	k[a->rta_type] = a;
      a = RTA_NEXT(a, nl_attr_len);
    }
  if (nl_attr_len)
    {
      log(L_ERR "nl_parse_attrs: remnant of size %d", nl_attr_len);
      return 0;
    }
  else
    return 1;
}

void
nl_add_attr(struct nlmsghdr *h, unsigned bufsize, unsigned code,
	    void *data, unsigned dlen)
{
  unsigned len = RTA_LENGTH(dlen);
  unsigned pos = NLMSG_ALIGN(h->nlmsg_len);
  struct rtattr *a;

  if (pos + len > bufsize)
    bug("nl_add_attr: packet buffer overflow");
  a = (struct rtattr *)((char *)h + pos);
  a->rta_type = code;
  a->rta_len = len;
  h->nlmsg_len = pos + len;
  memcpy(RTA_DATA(a), data, dlen);
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

#define RTNH_SIZE (sizeof(struct rtnexthop) + sizeof(struct rtattr) + sizeof(ip_addr))

static inline void
add_mpnexthop(char *buf, ip_addr ipa, unsigned iface, unsigned char weight)
{
  struct rtnexthop *nh = (void *) buf;
  struct rtattr *rt = (void *) (buf + sizeof(*nh));
  nh->rtnh_len = RTNH_SIZE;
  nh->rtnh_flags = 0;
  nh->rtnh_hops = weight;
  nh->rtnh_ifindex = iface;
  rt->rta_len = sizeof(*rt) + sizeof(ipa);
  rt->rta_type = RTA_GATEWAY;
  ipa_hton(ipa);
  memcpy(buf + sizeof(*nh) + sizeof(*rt), &ipa, sizeof(ipa));
}


static void
nl_add_multipath(struct nlmsghdr *h, unsigned bufsize, struct mpnh *nh)
{
  unsigned len = sizeof(struct rtattr);
  unsigned pos = NLMSG_ALIGN(h->nlmsg_len);
  char *buf = (char *)h + pos;
  struct rtattr *rt = (void *) buf;
  buf += len;
  
  for (; nh; nh = nh->next)
    {
      len += RTNH_SIZE;
      if (pos + len > bufsize)
	bug("nl_add_multipath: packet buffer overflow");

      add_mpnexthop(buf, nh->gw, nh->iface->index, nh->weight);
      buf += RTNH_SIZE;
    }

  rt->rta_type = RTA_MULTIPATH;
  rt->rta_len = len;
  h->nlmsg_len = pos + len;
}


static struct mpnh *
nl_parse_multipath(struct krt_proto *p, struct rtattr *ra)
{
  /* Temporary buffer for multicast nexthops */
  static struct mpnh *nh_buffer;
  static int nh_buf_size;	/* in number of structures */
  static int nh_buf_used;

  struct rtattr *a[RTA_CACHEINFO+1];
  struct rtnexthop *nh = RTA_DATA(ra);
  struct mpnh *rv, *first, **last;
  int len = RTA_PAYLOAD(ra);

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
      nl_parse_attrs(RTNH_DATA(nh), a, sizeof(a));
      if (a[RTA_GATEWAY])
	{
	  if (RTA_PAYLOAD(a[RTA_GATEWAY]) != sizeof(ip_addr))
	    return NULL;

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


/*
 *	Scanning of interfaces
 */

static void
nl_parse_link(struct nlmsghdr *h, int scan)
{
  struct ifinfomsg *i;
  struct rtattr *a[IFLA_WIRELESS+1];
  int new = h->nlmsg_type == RTM_NEWLINK;
  struct iface f = {};
  struct iface *ifi;
  char *name;
  u32 mtu;
  unsigned int fl;

  if (!(i = nl_checkin(h, sizeof(*i))) || !nl_parse_attrs(IFLA_RTA(i), a, sizeof(a)))
    return;
  if (!a[IFLA_IFNAME] || RTA_PAYLOAD(a[IFLA_IFNAME]) < 2 ||
      !a[IFLA_MTU] || RTA_PAYLOAD(a[IFLA_MTU]) != 4)
    {
      if (scan || !a[IFLA_WIRELESS])
        log(L_ERR "nl_parse_link: Malformed message received");
      return;
    }
  name = RTA_DATA(a[IFLA_IFNAME]);
  memcpy(&mtu, RTA_DATA(a[IFLA_MTU]), sizeof(u32));

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
      if_update(&f);
    }
}

static void
nl_parse_addr(struct nlmsghdr *h)
{
  struct ifaddrmsg *i;
  struct rtattr *a[IFA_ANYCAST+1];
  int new = h->nlmsg_type == RTM_NEWADDR;
  struct ifa ifa;
  struct iface *ifi;
  int scope;

  if (!(i = nl_checkin(h, sizeof(*i))) || !nl_parse_attrs(IFA_RTA(i), a, sizeof(a)))
    return;
  if (i->ifa_family != BIRD_AF)
    return;
  if (!a[IFA_ADDRESS] || RTA_PAYLOAD(a[IFA_ADDRESS]) != sizeof(ip_addr)
#ifdef IPV6
      || a[IFA_LOCAL] && RTA_PAYLOAD(a[IFA_LOCAL]) != sizeof(ip_addr)
#else
      || !a[IFA_LOCAL] || RTA_PAYLOAD(a[IFA_LOCAL]) != sizeof(ip_addr)
      || (a[IFA_BROADCAST] && RTA_PAYLOAD(a[IFA_BROADCAST]) != sizeof(ip_addr))
#endif
      )
    {
      log(L_ERR "nl_parse_addr: Malformed message received");
      return;
    }

  ifi = if_find_by_index(i->ifa_index);
  if (!ifi)
    {
      log(L_ERR "KIF: Received address message for unknown interface %d", i->ifa_index);
      return;
    }

  bzero(&ifa, sizeof(ifa));
  ifa.iface = ifi;
  if (i->ifa_flags & IFA_F_SECONDARY)
    ifa.flags |= IA_SECONDARY;

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
}

void
kif_do_scan(struct kif_proto *p UNUSED)
{
  struct nlmsghdr *h;

  if_start_update();

  nl_request_dump(RTM_GETLINK);
  while (h = nl_get_scan())
    if (h->nlmsg_type == RTM_NEWLINK || h->nlmsg_type == RTM_DELLINK)
      nl_parse_link(h, 1);
    else
      log(L_DEBUG "nl_scan_ifaces: Unknown packet received (type=%d)", h->nlmsg_type);

  nl_request_dump(RTM_GETADDR);
  while (h = nl_get_scan())
    if (h->nlmsg_type == RTM_NEWADDR || h->nlmsg_type == RTM_DELADDR)
      nl_parse_addr(h);
    else
      log(L_DEBUG "nl_scan_ifaces: Unknown packet received (type=%d)", h->nlmsg_type);

  if_end_update();
}

/*
 *	Routes
 */

static struct krt_proto *nl_table_map[NL_NUM_TABLES];

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
    rv += RTNH_SIZE;
  return rv;
}

static int
nl_send_route(struct krt_proto *p, rte *e, struct ea_list *eattrs, int new)
{
  eattr *ea;
  net *net = e->net;
  rta *a = e->attrs;
  struct {
    struct nlmsghdr h;
    struct rtmsg r;
    char buf[128 + nh_bufsize(a->nexthops)];
  } r;

  DBG("nl_send_route(%I/%d,new=%d)\n", net->n.prefix, net->n.pxlen, new);

  bzero(&r.h, sizeof(r.h));
  bzero(&r.r, sizeof(r.r));
  r.h.nlmsg_type = new ? RTM_NEWROUTE : RTM_DELROUTE;
  r.h.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  r.h.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | (new ? NLM_F_CREATE|NLM_F_EXCL : 0);

  r.r.rtm_family = BIRD_AF;
  r.r.rtm_dst_len = net->n.pxlen;
  r.r.rtm_tos = 0;
  r.r.rtm_table = KRT_CF->sys.table_id;
  r.r.rtm_protocol = RTPROT_BIRD;
  r.r.rtm_scope = RT_SCOPE_UNIVERSE;
  nl_add_attr_ipa(&r.h, sizeof(r), RTA_DST, net->n.prefix);

  u32 metric = 0;
  if (new && e->attrs->source == RTS_INHERIT)
    metric = e->u.krt.metric;
  if (ea = ea_find(eattrs, EA_KRT_METRIC))
    metric = ea->u.data;
  if (metric != 0)
    nl_add_attr_u32(&r.h, sizeof(r), RTA_PRIORITY, metric);

  if (ea = ea_find(eattrs, EA_KRT_PREFSRC))
    nl_add_attr_ipa(&r.h, sizeof(r), RTA_PREFSRC, *(ip_addr *)ea->u.ptr->data);

  if (ea = ea_find(eattrs, EA_KRT_REALM))
    nl_add_attr_u32(&r.h, sizeof(r), RTA_FLOW, ea->u.data);

  /* a->iface != NULL checked in krt_capable() for router and device routes */

  switch (a->dest)
    {
    case RTD_ROUTER:
      r.r.rtm_type = RTN_UNICAST;
      nl_add_attr_u32(&r.h, sizeof(r), RTA_OIF, a->iface->index);
      nl_add_attr_ipa(&r.h, sizeof(r), RTA_GATEWAY, a->gw);
      break;
    case RTD_DEVICE:
      r.r.rtm_type = RTN_UNICAST;
      nl_add_attr_u32(&r.h, sizeof(r), RTA_OIF, a->iface->index);
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
    default:
      bug("krt_capable inconsistent with nl_send_route");
    }

  return nl_exchange(&r.h);
}

void
krt_replace_rte(struct krt_proto *p, net *n, rte *new, rte *old, struct ea_list *eattrs)
{
  int err = 0;

  /*
   * NULL for eattr of the old route is a little hack, but we don't
   * get proper eattrs for old in rt_notify() anyway. NULL means no
   * extended route attributes and therefore matches if the kernel
   * route has any of them.
   */

  if (old)
    nl_send_route(p, old, NULL, 0);

  if (new)
    err = nl_send_route(p, new, eattrs, 1);

  if (err < 0)
    n->n.flags |= KRF_SYNC_ERROR;
  else
    n->n.flags &= ~KRF_SYNC_ERROR;
}


#define SKIP(ARG...) do { DBG("KRT: Ignoring route - " ARG); return; } while(0)

static void
nl_parse_route(struct nlmsghdr *h, int scan)
{
  struct krt_proto *p;
  struct rtmsg *i;
  struct rtattr *a[RTA_CACHEINFO+1];
  int new = h->nlmsg_type == RTM_NEWROUTE;

  ip_addr dst = IPA_NONE;
  u32 oif = ~0;
  int src;

  if (!(i = nl_checkin(h, sizeof(*i))) || !nl_parse_attrs(RTM_RTA(i), a, sizeof(a)))
    return;
  if (i->rtm_family != BIRD_AF)
    return;
  if ((a[RTA_DST] && RTA_PAYLOAD(a[RTA_DST]) != sizeof(ip_addr)) ||
#ifdef IPV6
      (a[RTA_IIF] && RTA_PAYLOAD(a[RTA_IIF]) != 4) ||
#endif
      (a[RTA_OIF] && RTA_PAYLOAD(a[RTA_OIF]) != 4) ||
      (a[RTA_GATEWAY] && RTA_PAYLOAD(a[RTA_GATEWAY]) != sizeof(ip_addr)) ||
      (a[RTA_PRIORITY] && RTA_PAYLOAD(a[RTA_PRIORITY]) != 4) ||
      (a[RTA_PREFSRC] && RTA_PAYLOAD(a[RTA_PREFSRC]) != sizeof(ip_addr)) ||
      (a[RTA_FLOW] && RTA_PAYLOAD(a[RTA_FLOW]) != 4))
    {
      log(L_ERR "KRT: Malformed message received");
      return;
    }

  if (a[RTA_DST])
    {
      memcpy(&dst, RTA_DATA(a[RTA_DST]), sizeof(dst));
      ipa_ntoh(dst);
    }

  if (a[RTA_OIF])
    memcpy(&oif, RTA_DATA(a[RTA_OIF]), sizeof(oif));

  p = nl_table_map[i->rtm_table];	/* Do we know this table? */
  DBG("KRT: Got %I/%d, type=%d, oif=%d, table=%d, prid=%d, proto=%s\n", dst, i->rtm_dst_len, i->rtm_type, oif, i->rtm_table, i->rtm_protocol, p ? p->p.name : "(none)");
  if (!p)
    SKIP("unknown table %d\n", i->rtm_table);


#ifdef IPV6
  if (a[RTA_IIF])
    SKIP("IIF set\n");
#else
  if (i->rtm_tos != 0)			/* We don't support TOS */
    SKIP("TOS %02x\n", i->rtm_tos);
#endif

  if (scan && !new)
    SKIP("RTM_DELROUTE in scan\n");

  int c = ipa_classify_net(dst);
  if ((c < 0) || !(c & IADDR_HOST) || ((c & IADDR_SCOPE_MASK) <= SCOPE_LINK))
    SKIP("strange class/scope\n");

  // ignore rtm_scope, it is not a real scope
  // if (i->rtm_scope != RT_SCOPE_UNIVERSE)
  //   SKIP("scope %u\n", i->rtm_scope);

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
      if (!scan)
	SKIP("echo\n");
      src = KRT_SRC_BIRD;
      break;

    case RTPROT_BOOT:
    default:
      src = KRT_SRC_ALIEN;
    }

  net *net = net_get(p->p.table, dst, i->rtm_dst_len);

  rta ra = {
    .proto = &p->p,
    .source = RTS_INHERIT,
    .scope = SCOPE_UNIVERSE,
    .cast = RTC_UNICAST
  };

  switch (i->rtm_type)
    {
    case RTN_UNICAST:

      if (a[RTA_MULTIPATH])
	{
	  ra.dest = RTD_MULTIPATH;
	  ra.nexthops = nl_parse_multipath(p, a[RTA_MULTIPATH]);
	  if (!ra.nexthops)
	    {
	      log(L_ERR "KRT: Received strange multipath route %I/%d",
		  net->n.prefix, net->n.pxlen);
	      return;
	    }
	    
	  break;
	}

      ra.iface = if_find_by_index(oif);
      if (!ra.iface)
	{
	  log(L_ERR "KRT: Received route %I/%d with unknown ifindex %u",
	      net->n.prefix, net->n.pxlen, oif);
	  return;
	}

      if (a[RTA_GATEWAY])
	{
	  neighbor *ng;
	  ra.dest = RTD_ROUTER;
	  memcpy(&ra.gw, RTA_DATA(a[RTA_GATEWAY]), sizeof(ra.gw));
	  ipa_ntoh(ra.gw);

#ifdef IPV6
	  /* Silently skip strange 6to4 routes */
	  if (ipa_in_net(ra.gw, IPA_NONE, 96))
	    return;
#endif

	  ng = neigh_find2(&p->p, &ra.gw, ra.iface,
			   (i->rtm_flags & RTNH_F_ONLINK) ? NEF_ONLINK : 0);
	  if (!ng || (ng->scope == SCOPE_HOST))
	    {
	      log(L_ERR "KRT: Received route %I/%d with strange next-hop %I",
		  net->n.prefix, net->n.pxlen, ra.gw);
	      return;
	    }
	}
      else
	{
	  ra.dest = RTD_DEVICE;

	  /*
	   * In Linux IPv6, 'native' device routes have proto
	   * RTPROT_BOOT and not RTPROT_KERNEL (which they have in
	   * IPv4 and which is expected). We cannot distinguish
	   * 'native' and user defined device routes, so we ignore all
	   * such device routes and for consistency, we have the same
	   * behavior in IPv4. Anyway, users should use RTPROT_STATIC
	   * for their 'alien' routes.
	   */

	  if (i->rtm_protocol == RTPROT_BOOT)
	    src = KRT_SRC_KERNEL;
	}

      break;
    case RTN_BLACKHOLE:
      ra.dest = RTD_BLACKHOLE;
      break;
    case RTN_UNREACHABLE:
      ra.dest = RTD_UNREACHABLE;
      break;
    case RTN_PROHIBIT:
      ra.dest = RTD_PROHIBIT;
      break;
    /* FIXME: What about RTN_THROW? */
    default:
      SKIP("type %d\n", i->rtm_type);
      return;
    }

  rte *e = rte_get_temp(&ra);
  e->net = net;
  e->u.krt.src = src;
  e->u.krt.proto = i->rtm_protocol;
  e->u.krt.type = i->rtm_type;

  if (a[RTA_PRIORITY])
    memcpy(&e->u.krt.metric, RTA_DATA(a[RTA_PRIORITY]), sizeof(e->u.krt.metric)); 
  else
    e->u.krt.metric = 0;

  if (a[RTA_PREFSRC])
    {
      ip_addr ps;
      memcpy(&ps, RTA_DATA(a[RTA_PREFSRC]), sizeof(ps));
      ipa_ntoh(ps);

      ea_list *ea = alloca(sizeof(ea_list) + sizeof(eattr));
      ea->next = ra.eattrs;
      ra.eattrs = ea;
      ea->flags = EALF_SORTED;
      ea->count = 1;
      ea->attrs[0].id = EA_KRT_PREFSRC;
      ea->attrs[0].flags = 0;
      ea->attrs[0].type = EAF_TYPE_IP_ADDRESS;
      ea->attrs[0].u.ptr = alloca(sizeof(struct adata) + sizeof(ps));
      ea->attrs[0].u.ptr->length = sizeof(ps);
      memcpy(ea->attrs[0].u.ptr->data, &ps, sizeof(ps));
    }

  if (a[RTA_FLOW])
    {
      ea_list *ea = alloca(sizeof(ea_list) + sizeof(eattr));
      ea->next = ra.eattrs;
      ra.eattrs = ea;
      ea->flags = EALF_SORTED;
      ea->count = 1;
      ea->attrs[0].id = EA_KRT_REALM;
      ea->attrs[0].flags = 0;
      ea->attrs[0].type = EAF_TYPE_INT;
      memcpy(&ea->attrs[0].u.data, RTA_DATA(a[RTA_FLOW]), 4);
    }

  if (scan)
    krt_got_route(p, e);
  else
    krt_got_route_async(p, e, new);
}

void
krt_do_scan(struct krt_proto *p UNUSED)	/* CONFIG_ALL_TABLES_AT_ONCE => p is NULL */
{
  struct nlmsghdr *h;

  nl_request_dump(RTM_GETROUTE);
  while (h = nl_get_scan())
    if (h->nlmsg_type == RTM_NEWROUTE || h->nlmsg_type == RTM_DELROUTE)
      nl_parse_route(h, 1);
    else
      log(L_DEBUG "nl_scan_fire: Unknown packet received (type=%d)", h->nlmsg_type);
}

/*
 *	Asynchronous Netlink interface
 */

static sock *nl_async_sk;		/* BIRD socket for asynchronous notifications */
static byte *nl_async_rx_buffer;	/* Receive buffer */

static void
nl_async_msg(struct nlmsghdr *h)
{
  switch (h->nlmsg_type)
    {
    case RTM_NEWROUTE:
    case RTM_DELROUTE:
      DBG("KRT: Received async route notification (%d)\n", h->nlmsg_type);
      nl_parse_route(h, 0);
      break;
    case RTM_NEWLINK:
    case RTM_DELLINK:
      DBG("KRT: Received async link notification (%d)\n", h->nlmsg_type);
      nl_parse_link(h, 0);
      break;
    case RTM_NEWADDR:
    case RTM_DELADDR:
      DBG("KRT: Received async address notification (%d)\n", h->nlmsg_type);
      nl_parse_addr(h);
      break;
    default:
      DBG("KRT: Received unknown async notification (%d)\n", h->nlmsg_type);
    }
}

static int
nl_async_hook(sock *sk, int size UNUSED)
{
  struct iovec iov = { nl_async_rx_buffer, NL_RX_SIZE };
  struct sockaddr_nl sa;
  struct msghdr m = { (struct sockaddr *) &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
  struct nlmsghdr *h;
  int x;
  unsigned int len;

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
nl_open_async(void)
{
  sock *sk;
  struct sockaddr_nl sa;
  int fd;
  static int nl_open_tried = 0;

  if (nl_open_tried)
    return;
  nl_open_tried = 1;

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
      return;
    }

  sk = nl_async_sk = sk_new(krt_pool);
  sk->type = SK_MAGIC;
  sk->rx_hook = nl_async_hook;
  sk->fd = fd;
  if (sk_open(sk))
    bug("Netlink: sk_open failed");

  if (!nl_async_rx_buffer)
    nl_async_rx_buffer = xmalloc(NL_RX_SIZE);
}

/*
 *	Interface to the UNIX krt module
 */

static u8 nl_cf_table[(NL_NUM_TABLES+7) / 8];

void
krt_sys_start(struct krt_proto *p)
{
  nl_table_map[KRT_CF->sys.table_id] = p;

  nl_open();
  nl_open_async();
}

void
krt_sys_shutdown(struct krt_proto *p UNUSED)
{
}

int
krt_sys_reconfigure(struct krt_proto *p UNUSED, struct krt_config *n, struct krt_config *o)
{
  return n->sys.table_id == o->sys.table_id;
}


void
krt_sys_preconfig(struct config *c UNUSED)
{
  bzero(&nl_cf_table, sizeof(nl_cf_table));
}

void
krt_sys_postconfig(struct krt_config *x)
{
  int id = x->sys.table_id;

  if (nl_cf_table[id/8] & (1 << (id%8)))
    cf_error("Multiple kernel syncers defined for table #%d", id);
  nl_cf_table[id/8] |= (1 << (id%8));
}

void
krt_sys_init_config(struct krt_config *cf)
{
  cf->sys.table_id = RT_TABLE_MAIN;
}

void
krt_sys_copy_config(struct krt_config *d, struct krt_config *s)
{
  d->sys.table_id = s->sys.table_id;
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
