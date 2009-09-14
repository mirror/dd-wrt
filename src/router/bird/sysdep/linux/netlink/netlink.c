/*
 *	BIRD -- Linux Netlink Interface
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <fcntl.h>
#include <net/if.h>
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
#include "conf/conf.h"

#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#ifndef MSG_TRUNC			/* Hack: Several versions of glibc miss this one :( */
#define MSG_TRUNC 0x20
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
  req.g.rtgen_family = BIRD_PF;
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
  return nl_error(h);
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

static void
nl_add_attr_u32(struct nlmsghdr *h, unsigned maxsize, int code, u32 data)
{
  unsigned len = RTA_LENGTH(4);
  struct rtattr *a;

  if (NLMSG_ALIGN(h->nlmsg_len) + len > maxsize)
    bug("nl_add_attr32: packet buffer overflow");
  a = (struct rtattr *)((char *)h + NLMSG_ALIGN(h->nlmsg_len));
  a->rta_type = code;
  a->rta_len = len;
  memcpy(RTA_DATA(a), &data, 4);
  h->nlmsg_len = NLMSG_ALIGN(h->nlmsg_len) + len;
}

static void
nl_add_attr_ipa(struct nlmsghdr *h, unsigned maxsize, int code, ip_addr ipa)
{
  unsigned len = RTA_LENGTH(sizeof(ipa));
  struct rtattr *a;

  if (NLMSG_ALIGN(h->nlmsg_len) + len > maxsize)
    bug("nl_add_attr_ipa: packet buffer overflow");
  a = (struct rtattr *)((char *)h + NLMSG_ALIGN(h->nlmsg_len));
  a->rta_type = code;
  a->rta_len = len;
  ipa_hton(ipa);
  memcpy(RTA_DATA(a), &ipa, sizeof(ipa));
  h->nlmsg_len = NLMSG_ALIGN(h->nlmsg_len) + len;
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
  struct iface f;
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
      if (ifi && !scan)
	{
	  memcpy(&f, ifi, sizeof(struct iface));
	  f.flags |= IF_ADMIN_DOWN;
	  if_update(&f);
	}
    }
  else
    {
      DBG("KIF: IF%d(%s) goes up (mtu=%d,flg=%x)\n", i->ifi_index, name, mtu, i->ifi_flags);
      if (ifi)
	memcpy(&f, ifi, sizeof(f));
      else
	{
	  bzero(&f, sizeof(f));
	  f.index = i->ifi_index;
	}
      strncpy(f.name, RTA_DATA(a[IFLA_IFNAME]), sizeof(f.name)-1);
      f.mtu = mtu;
      f.flags = 0;
      fl = i->ifi_flags;
      if (fl & IFF_UP)
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
  if (i->ifa_prefixlen > BITS_PER_IP_ADDRESS ||
      i->ifa_prefixlen == BITS_PER_IP_ADDRESS - 1)
    {
      log(L_ERR "KIF: Invalid prefix length for interface %s: %d", ifi->name, i->ifa_prefixlen);
      new = 0;
    }
  if (i->ifa_prefixlen == BITS_PER_IP_ADDRESS)
    {
      ifa.flags |= IA_UNNUMBERED;
      memcpy(&ifa.opposite, RTA_DATA(a[IFA_ADDRESS]), sizeof(ifa.opposite));
      ipa_ntoh(ifa.opposite);
      ifa.prefix = ifa.brd = ifa.opposite;
    }
  else
    {
      ip_addr netmask = ipa_mkmask(ifa.pxlen);
      ip_addr xbrd;
      ifa.prefix = ipa_and(ifa.ip, netmask);
      ifa.brd = ipa_or(ifa.ip, ipa_not(netmask));
#ifndef IPV6
      if (i->ifa_prefixlen == BITS_PER_IP_ADDRESS - 2)
	ifa.opposite = ipa_opposite(ifa.ip, i->ifa_prefixlen);
      if ((ifi->flags & IF_BROADCAST) && a[IFA_BROADCAST])
	{
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
krt_if_scan(struct kif_proto *p UNUSED)
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

  if (a->cast != RTC_UNICAST
#if 0
      && a->cast != RTC_ANYCAST
#endif
      )
    return 0;
  if (a->source == RTS_DEVICE)	/* Kernel takes care of device routes itself */
    return 0;
  switch (a->dest)
    {
    case RTD_ROUTER:
    case RTD_DEVICE:
    case RTD_BLACKHOLE:
    case RTD_UNREACHABLE:
    case RTD_PROHIBIT:
      break;
    default:
      return 0;
    }
  return 1;
}

static void
nl_send_route(struct krt_proto *p, rte *e, int new)
{
  net *net = e->net;
  rta *a = e->attrs;
  struct {
    struct nlmsghdr h;
    struct rtmsg r;
    char buf[128];
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
  r.r.rtm_table = KRT_CF->scan.table_id;
  r.r.rtm_protocol = RTPROT_BIRD;
  r.r.rtm_scope = RT_SCOPE_UNIVERSE;
  nl_add_attr_ipa(&r.h, sizeof(r), RTA_DST, net->n.prefix);
  switch (a->dest)
    {
    case RTD_ROUTER:
      r.r.rtm_type = RTN_UNICAST;
      nl_add_attr_ipa(&r.h, sizeof(r), RTA_GATEWAY, a->gw);
      break;
    case RTD_DEVICE:
      if (!a->iface)
	return;
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
    default:
      bug("krt_capable inconsistent with nl_send_route");
    }

  nl_exchange(&r.h);
}

void
krt_set_notify(struct krt_proto *p, net *n UNUSED, rte *new, rte *old)
{
  if (old)
    nl_send_route(p, old, 0);

  if (new)
    nl_send_route(p, new, 1);
}

static struct iface *
krt_temp_iface(struct krt_proto *p, unsigned index)
{
  struct iface *i, *j;

  WALK_LIST(i, p->scan.temp_ifs)
    if (i->index == index)
      return i;
  i = mb_allocz(p->p.pool, sizeof(struct iface));
  if (j = if_find_by_index(index))
    strcpy(i->name, j->name);
  else
    strcpy(i->name, "?");
  i->index = index;
  add_tail(&p->scan.temp_ifs, &i->n);
  return i;
}

static void
nl_parse_route(struct nlmsghdr *h, int scan)
{
  struct krt_proto *p;
  struct rtmsg *i;
  struct rtattr *a[RTA_CACHEINFO+1];
  int new = h->nlmsg_type == RTM_NEWROUTE;
  ip_addr dst;
  rta ra;
  rte *e;
  net *net;
  u32 oif;
  int src;

  if (!(i = nl_checkin(h, sizeof(*i))) || !nl_parse_attrs(RTM_RTA(i), a, sizeof(a)))
    return;
  if (i->rtm_family != BIRD_AF)
    return;
  if ((a[RTA_DST] && RTA_PAYLOAD(a[RTA_DST]) != sizeof(ip_addr)) ||
      (a[RTA_OIF] && RTA_PAYLOAD(a[RTA_OIF]) != 4) ||
      (a[RTA_PRIORITY] && RTA_PAYLOAD(a[RTA_PRIORITY]) != 4) ||
#ifdef IPV6
      (a[RTA_IIF] && RTA_PAYLOAD(a[RTA_IIF]) != 4) ||
#endif
      (a[RTA_GATEWAY] && RTA_PAYLOAD(a[RTA_GATEWAY]) != sizeof(ip_addr)))
    {
      log(L_ERR "nl_parse_route: Malformed message received");
      return;
    }

  p = nl_table_map[i->rtm_table];	/* Do we know this table? */
  if (!p)
    return;

#ifdef IPV6
  if (a[RTA_IIF])
    {
      DBG("KRT: Ignoring route with IIF set\n");
      return;
    }
#else
  if (i->rtm_tos != 0)			/* We don't support TOS */
    {
      DBG("KRT: Ignoring route with TOS %02x\n", i->rtm_tos);
      return;
    }
#endif

  if (scan && !new)
    {
      DBG("KRT: Ignoring route deletion\n");
      return;
    }

  if (a[RTA_DST])
    {
      memcpy(&dst, RTA_DATA(a[RTA_DST]), sizeof(dst));
      ipa_ntoh(dst);
    }
  else
    dst = IPA_NONE;
  if (a[RTA_OIF])
    memcpy(&oif, RTA_DATA(a[RTA_OIF]), sizeof(oif));
  else
    oif = ~0;

  DBG("Got %I/%d, type=%d, oif=%d, table=%d, prid=%d, proto=%s\n", dst, i->rtm_dst_len, i->rtm_type, oif, i->rtm_table, i->rtm_protocol, p->p.name);

  switch (i->rtm_protocol)
    {
    case RTPROT_REDIRECT:
      src = KRT_SRC_REDIRECT;
      break;
    case RTPROT_KERNEL:
      DBG("Route originated in kernel, ignoring\n");
      return;
    case RTPROT_BIRD:
#ifdef IPV6
    case RTPROT_BOOT:
      /* Current Linux kernels don't remember rtm_protocol for IPv6 routes and supply RTPROT_BOOT instead */
#endif
      if (!scan)
	{
	  DBG("Echo of our own route, ignoring\n");
	  return;
	}
      src = KRT_SRC_BIRD;
      break;
    default:
      src = KRT_SRC_ALIEN;
    }

  net = net_get(p->p.table, dst, i->rtm_dst_len);
  ra.proto = &p->p;
  ra.source = RTS_INHERIT;
  ra.scope = SCOPE_UNIVERSE;
  ra.cast = RTC_UNICAST;
  ra.flags = ra.aflags = 0;
  ra.from = IPA_NONE;
  ra.gw = IPA_NONE;
  ra.iface = NULL;
  ra.eattrs = NULL;

  switch (i->rtm_type)
    {
    case RTN_UNICAST:
      if (oif == ~0U)
	{
	  log(L_ERR "KRT: Mysterious route with no OIF (%I/%d)", net->n.prefix, net->n.pxlen);
	  return;
	}
      if (a[RTA_GATEWAY])
	{
	  neighbor *ng;
	  ra.dest = RTD_ROUTER;
	  memcpy(&ra.gw, RTA_DATA(a[RTA_GATEWAY]), sizeof(ra.gw));
	  ipa_ntoh(ra.gw);
	  ng = neigh_find(&p->p, &ra.gw, 0);
	  if (ng && ng->scope)
	    ra.iface = ng->iface;
	  else
	    /* FIXME: Remove this warning? Handle it somehow... */
	    log(L_WARN "Kernel told us to use non-neighbor %I for %I/%d", ra.gw, net->n.prefix, net->n.pxlen);
	}
      else
	{
	  ra.dest = RTD_DEVICE;
	  ra.iface = krt_temp_iface(p, oif);
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
      DBG("KRT: Ignoring route with type=%d\n", i->rtm_type);
      return;
    }

  if (i->rtm_scope != RT_SCOPE_UNIVERSE)
    {
      DBG("KRT: Ignoring route with scope=%d\n", i->rtm_scope);
      return;
    }

  e = rte_get_temp(&ra);
  e->net = net;
  e->u.krt.src = src;
  e->u.krt.proto = i->rtm_protocol;
  e->u.krt.type = i->rtm_type;
  if (a[RTA_PRIORITY])
    memcpy(&e->u.krt.metric, RTA_DATA(a[RTA_PRIORITY]), sizeof(e->u.krt.metric));
  else
    e->u.krt.metric = 0;
  if (scan)
    krt_got_route(p, e);
  else
    krt_got_route_async(p, e, new);
}

void
krt_scan_fire(struct krt_proto *p UNUSED)	/* CONFIG_ALL_TABLES_AT_ONCE => p is NULL */
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
krt_scan_preconfig(struct config *c UNUSED)
{
  bzero(&nl_cf_table, sizeof(nl_cf_table));
}

void
krt_scan_postconfig(struct krt_config *x)
{
  int id = x->scan.table_id;

  if (nl_cf_table[id/8] & (1 << (id%8)))
    cf_error("Multiple kernel syncers defined for table #%d", id);
  nl_cf_table[id/8] |= (1 << (id%8));
}

void
krt_scan_construct(struct krt_config *x)
{
#ifndef IPV6
  x->scan.table_id = RT_TABLE_MAIN;
#else
  x->scan.table_id = 254;
#endif
}

void
krt_scan_start(struct krt_proto *p, int first)
{
  init_list(&p->scan.temp_ifs);
  nl_table_map[KRT_CF->scan.table_id] = p;
  if (first)
    {
      nl_open();
      nl_open_async();
    }
}

void
krt_scan_shutdown(struct krt_proto *p UNUSED, int last UNUSED)
{
}

void
krt_if_start(struct kif_proto *p UNUSED)
{
  nl_open();
  nl_open_async();
}
