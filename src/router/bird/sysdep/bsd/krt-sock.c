/*
 *	BIRD -- Unix Routing Table Syncing
 *
 *	(c) 2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/route.h>
#include <net/if.h>
#include <net/if_dl.h>

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/route.h"
#include "nest/protocol.h"
#include "nest/iface.h"
#include "lib/timer.h"
#include "lib/unix.h"
#include "lib/krt.h"
#include "lib/string.h"
#include "lib/socket.h"

#ifdef IPV6
#define HOST_MASK 128
#else
#define HOST_MASK 32
#endif

int rt_sock = 0;

#define CHECK_FAMILY(sa) \
  ((((struct sockaddr *)sa)->sa_family) == BIRD_AF)

static struct iface *
krt_temp_iface_index(struct krt_proto *p, unsigned index)
{
  struct iface *i, *j;

  WALK_LIST(i, p->scan.temp_ifs)
    if (i->index == index)
      return i;
  i = mb_allocz(p->p.pool, sizeof(struct iface));
  if (j = if_find_by_index(index))
  {
    strcpy(i->name, j->name);
    i->addr = j->addr;
  }
  else
    strcpy(i->name, "?");
  i->index = index;
  add_tail(&p->scan.temp_ifs, &i->n);
  return i;
}


int
krt_capable(rte *e)
{
  rta *a = e->attrs;

#ifdef CONFIG_AUTO_ROUTES
  if (a->source == RTS_DEVICE)
    return 0;
#endif
  return
    a->cast == RTC_UNICAST &&
    (a->dest == RTD_ROUTER
     || a->dest == RTD_DEVICE
#ifdef RTF_REJECT
     || a->dest == RTD_UNREACHABLE
#endif
#ifdef RTF_BLACKHOLE
     || a->dest == RTD_BLACKHOLE	/* FIXME Prohibited? */
#endif
     );
}

#define ROUNDUP(a) \
        ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))

#define NEXTADDR(w, u) \
        if (msg.rtm.rtm_addrs & (w)) {\
          l = ROUNDUP(((struct sockaddr *)&(u))->sa_len);\
          memmove(body, &(u), l); body += l;}

static void
krt_sock_send(int cmd, rte *e)
{
  net *net = e->net;
  rta *a = e->attrs;
  static int msg_seq;
  struct iface *j, *i = a->iface;
  int l;
  struct ks_msg msg;
  char *body = (char *)msg.buf;
  sockaddr gate, mask, dst;

  DBG("krt-sock: send %I/%d via %I", net->n.prefix, net->n.pxlen, a->gw);

  fill_in_sockaddr(&dst, net->n.prefix, 0);
  fill_in_sockaddr(&mask, ipa_mkmask(net->n.pxlen), 0);
  fill_in_sockaddr(&gate, a->gw, 0);

  bzero(&msg,sizeof (struct rt_msghdr));
  msg.rtm.rtm_version = RTM_VERSION;
  msg.rtm.rtm_type = cmd;
  msg.rtm.rtm_seq = msg_seq++;
  msg.rtm.rtm_addrs = RTA_DST;
  msg.rtm.rtm_flags = RTF_UP;

  if (net->n.pxlen == HOST_MASK)
  {
    msg.rtm.rtm_flags |= RTF_HOST;
  }
  else
  {
    msg.rtm.rtm_addrs |= RTA_NETMASK;
  }

#ifdef RTF_REJECT
  if(a->dest == RTD_UNREACHABLE)
    msg.rtm.rtm_flags |= RTF_REJECT;
#endif
#ifdef RTF_BLACKHOLE
  if(a->dest == RTD_BLACKHOLE)
    msg.rtm.rtm_flags |= RTF_BLACKHOLE;
#endif

  /* This is really very nasty, but I'm not able
   * to add "(reject|blackhole)" route without
   * gateway set
   */
  if(!i)
  {
    i = HEAD(iface_list);

    WALK_LIST(j, iface_list)
    {
      if (j->flags & IF_LOOPBACK)
      {
        i = j;
        break;
      }
    }
  }

  switch (a->dest)
  {
    case RTD_ROUTER:
      msg.rtm.rtm_flags |= RTF_GATEWAY;
      msg.rtm.rtm_addrs |= RTA_GATEWAY;
      break;
#ifdef RTF_REJECT
    case RTD_UNREACHABLE:
#endif
#ifdef RTF_BLACKHOLE
    case RTD_BLACKHOLE:
#endif
    case RTD_DEVICE:
      if(i)
      {
        if (cmd == RTM_ADD && (i->flags & IF_MULTIACCESS) != IF_MULTIACCESS)	/* PTP */
          msg.rtm.rtm_flags |= RTF_CLONING;

        if(!i->addr) {
          log(L_ERR "KIF: interface \"%s\" has no IP addess", i->name);
          return;
        }

        fill_in_sockaddr(&gate, i->addr->ip, 0);
        msg.rtm.rtm_addrs |= RTA_GATEWAY;
      }
      break;
    default:
      bug("krt-sock: unknown flags, but not filtered");
  }

  msg.rtm.rtm_index = i->index;

  NEXTADDR(RTA_DST, dst);
  NEXTADDR(RTA_GATEWAY, gate);
  NEXTADDR(RTA_NETMASK, mask);

  l = body - (char *)&msg;
  msg.rtm.rtm_msglen = l;

  if ((l = write(rt_sock, (char *)&msg, l)) < 0) {
          log(L_ERR "KIF: error writting route to socket (%I/%d)", net->n.prefix, net->n.pxlen);
  }
}

void
krt_set_notify(struct krt_proto *p UNUSED, net *net UNUSED, rte *new, rte *old)
{
  if (old)
    {
      DBG("krt_remove_route(%I/%d)\n", net->n.prefix, net->n.pxlen);
      krt_sock_send(RTM_DELETE, old);
    }
  if (new)
    {
      DBG("krt_add_route(%I/%d)\n", net->n.prefix, net->n.pxlen);
      krt_sock_send(RTM_ADD, new);
    }
}

static int
krt_set_hook(sock *sk, int size UNUSED)
{
  struct ks_msg msg;
  int l = read(sk->fd, (char *)&msg, sizeof(msg));

  if(l <= 0)
    log(L_ERR "krt-sock: read failed");
  else
  krt_read_msg((struct proto *)sk->data, &msg, 0);

  return 0;
}

void
krt_set_start(struct krt_proto *x, int first UNUSED)
{
  sock *sk_rt;
  static int ks_open_tried = 0;

  if (ks_open_tried)
    return;

  ks_open_tried = 1;

  DBG("KRT: Opening kernel socket\n");

  if( (rt_sock = socket(PF_ROUTE, SOCK_RAW, AF_UNSPEC)) < 0)
    die("Cannot open kernel socket for routes");

  sk_rt = sk_new(krt_pool);
  sk_rt->type = SK_MAGIC;
  sk_rt->rx_hook = krt_set_hook;
  sk_rt->fd = rt_sock;
  sk_rt->data = x;
  if (sk_open(sk_rt))
    bug("krt-sock: sk_open failed");
}

static void
krt_read_rt(struct ks_msg *msg, struct krt_proto *p, int scan)
{
  sockaddr gate, mask, dst;
  rta a;
  rte *e;
  net *net;
  ip_addr idst, igate, imask;
  void *body = (char *)msg->buf;
  int new = (msg->rtm.rtm_type == RTM_ADD);
  int src;
  int flags = msg->rtm.rtm_flags;
  int addrs = msg->rtm.rtm_addrs;
  int masklen = -1;

  if (!(flags & RTF_UP))
  {
    DBG("Down.\n");
    return;
  }

  if (flags & RTF_HOST)
    masklen = HOST_MASK;

  if(!CHECK_FAMILY(body)) return;

  if(msg->rtm.rtm_flags & RTF_LLINFO) return;	/* ARPs etc. */

#define GETADDR(p, F) \
  bzero(p, sizeof(*p));\
  if ((addrs & (F)) && ((struct sockaddr *)body)->sa_len) {\
    unsigned int l = ROUNDUP(((struct sockaddr *)body)->sa_len);\
    memcpy(p, body, (l > sizeof(*p) ? sizeof(*p) : l));\
    body += l;}

  GETADDR (&dst, RTA_DST);
  GETADDR (&gate, RTA_GATEWAY);
  GETADDR (&mask, RTA_NETMASK);

  idst = IPA_NONE;
  igate = IPA_NONE;
  imask = IPA_NONE;

  get_sockaddr(&dst, &idst, NULL, 0);
  if(CHECK_FAMILY(&gate)) get_sockaddr(&gate, &igate, NULL, 0);
  get_sockaddr(&mask, &imask, NULL, 0);

  if (masklen < 0) masklen = ipa_mklen(imask);

  if (flags & (RTF_DYNAMIC | RTF_MODIFIED))
  {
    log(L_WARN "krt: Ignoring redirect to %I/%d via %I", idst, masklen, igate);
    return;
  }

  if (masklen < 0)
  {
    log(L_WARN "krt: Got invalid route from kernel!");
    return;
  }

  net = net_get(p->p.table, idst, masklen);

  bzero(&a, sizeof(a));

  a.proto = &p->p;
  a.source = RTS_INHERIT;
  a.scope = SCOPE_UNIVERSE;
  a.cast = RTC_UNICAST;
  a.flags = a.aflags = 0;
  a.from = IPA_NONE;
  a.gw = IPA_NONE;
  a.iface = NULL;
  a.eattrs = NULL;

  a.dest = RTD_NONE;

  if (flags & RTF_GATEWAY)
  {
    neighbor *ng = neigh_find(&p->p, &igate, 0);
    if (ng && ng->scope)
      a.iface = ng->iface;
    else
      log(L_WARN "Kernel told us to use non-neighbor %I for %I/%d", igate, net->n.prefix, net->n.pxlen);

    a.dest = RTD_ROUTER;
    a.gw = igate;
  }
  else
  {
    a.dest = RTD_DEVICE;
    a.gw = IPA_NONE;
    a.iface = krt_temp_iface_index(p, msg->rtm.rtm_index);
  }

#ifdef RTF_REJECT
  if(flags & RTF_REJECT) {
    a.dest = RTD_UNREACHABLE;
    a.gw = IPA_NONE;
  }
#endif

#ifdef RTF_BLACKHOLE
  if(flags & RTF_BLACKHOLE) {
    a.dest = RTD_BLACKHOLE;
    a.gw = IPA_NONE;
  }
#endif

  if (a.dest == RTD_NONE)
  {
    log(L_WARN "Kernel reporting unknown route type to %I/%d", net->n.prefix, net->n.pxlen);
    return;
  }

  src = KRT_SRC_UNKNOWN;	/* FIXME */

  e = rte_get_temp(&a);
  e->net = net;
  e->u.krt.src = src;
  //e->u.krt.proto = i->rtm_protocol;
  //e->u.krt.type = i->rtm_type;
  e->u.krt.metric = 0;

  if (scan)
    krt_got_route(p, e);
  else
    krt_got_route_async(p, e, new);
}

static void
krt_read_ifinfo(struct ks_msg *msg)
{
  struct if_msghdr *ifm = (struct if_msghdr *)&msg->rtm;
  void *body = (void *)(ifm + 1);
  struct sockaddr_dl *dl = NULL;
  unsigned int i;
  struct iface *iface = NULL, f;
  char *ifname = "(none)";
  int fl = ifm->ifm_flags;

  for(i = 1; i!=0; i <<= 1)
  {
    if((i & ifm->ifm_addrs) && (i == RTA_IFP))
    {
      if( i == RTA_IFP)
      {
        dl = (struct sockaddr_dl *)body;
        break;
      }
      body += ROUNDUP(((struct sockaddr *)&(body))->sa_len);\
    }
  }

  if(dl && (dl->sdl_family != AF_LINK))
  {
    log("Ignoring strange IFINFO");
    return;
  }

  if(dl) ifname = dl->sdl_data;

  iface = if_find_by_index(ifm->ifm_index);

  if(!iface)
  {
    /* New interface */
    if(!dl) return;	/* No interface name, ignoring */
    DBG("New interface \"%s\" found", ifname);
    bzero(&f, sizeof(f));
    f.index = ifm->ifm_index;
    strncpy(f.name, ifname, sizeof(f.name) -1);
  }
  else
  {
    memcpy(&f, iface, sizeof(struct iface));
  }

  f.mtu = ifm->ifm_data.ifi_mtu;
  f.flags = 0;

  if (fl & IFF_UP)
    f.flags |= IF_LINK_UP;
  if (fl & IFF_LOOPBACK)            /* Loopback */
    f.flags |= IF_MULTIACCESS | IF_LOOPBACK | IF_IGNORE;
  else if (fl & IFF_POINTOPOINT)    /* PtP */
    f.flags |= IF_MULTICAST;
  else if (fl & IFF_BROADCAST)      /* Broadcast */
    f.flags |= IF_MULTIACCESS | IF_BROADCAST | IF_MULTICAST;
  else
    f.flags |= IF_MULTIACCESS;      /* NBMA */

  if((!iface) || memcmp(&f, iface, sizeof(struct iface)))
    if_update(&f);	/* Just if something happens */
}

static void
krt_read_addr(struct ks_msg *msg)
{
  struct ifa_msghdr *ifam = (struct ifa_msghdr *)&msg->rtm;
  void *body = (void *)(ifam + 1);
  sockaddr addr, mask, brd;
  struct iface *iface = NULL;
  struct ifa ifa;
  struct sockaddr null;
  ip_addr iaddr, imask, ibrd;
  int addrs = ifam->ifam_addrs;
  int scope, masklen = -1;
  int new = (ifam->ifam_type == RTM_NEWADDR);

  if(!(iface = if_find_by_index(ifam->ifam_index)))
  {
    log(L_ERR "KIF: Received address message for unknown interface %d", ifam->ifam_index);
    return;
  }

  GETADDR (&null, RTA_DST);
  GETADDR (&null, RTA_GATEWAY);
  GETADDR (&mask, RTA_NETMASK);
  GETADDR (&null, RTA_GENMASK);
  GETADDR (&null, RTA_IFP);
  GETADDR (&addr, RTA_IFA);
  GETADDR (&null, RTA_AUTHOR);
  GETADDR (&brd, RTA_BRD);

  if(!CHECK_FAMILY(&addr)) return; /* Some other family address */

  get_sockaddr(&addr, &iaddr, NULL, 0);
  get_sockaddr(&mask, &imask, NULL, 0);
  get_sockaddr(&brd, &ibrd, NULL, 0);

  if ((masklen = ipa_mklen(imask)) < 0)
  {
    log("Invalid masklen");
    return;
  }

  bzero(&ifa, sizeof(ifa));

  ifa.iface = iface;

  memcpy(&ifa.ip, &iaddr, sizeof(ip_addr));
  ifa.pxlen = masklen;
  memcpy(&ifa.brd, &ibrd, sizeof(ip_addr));

  scope = ipa_classify(ifa.ip);

  ifa.prefix = ipa_and(ifa.ip, ipa_mkmask(masklen));

  if (scope < 0)
  {
    log(L_ERR "KIF: Invalid interface address %I for %s", ifa.ip, iface->name);
    return;
  }
  ifa.scope = scope & IADDR_SCOPE_MASK;

  if (new)
    ifa_update(&ifa);
  else
    ifa_delete(&ifa);
}


void
krt_read_msg(struct proto *p, struct ks_msg *msg, int scan)
{
  switch (msg->rtm.rtm_type)
  {
    case RTM_GET:
      if(!scan) return;
    case RTM_ADD:
    case RTM_DELETE:
      krt_read_rt(msg, (struct krt_proto *)p, scan);
      break;
    case RTM_IFINFO:
      krt_read_ifinfo(msg);
      break;
    case RTM_NEWADDR:
    case RTM_DELADDR:
      krt_read_addr(msg);
      break;
#ifdef RTM_IFANNOUNCE
    case RTM_IFANNOUNCE:	/* FIXME: We should handle it */
      break;
#endif /* RTM_IFANNOUNCE */
    default:
        log(L_ERR "Unprocessed RTM_type: %d", msg->rtm.rtm_type);
      break;
  }
}

void
krt_scan_construct(struct krt_config *c UNUSED)
{
}

void
krt_scan_preconfig(struct config *c UNUSED)
{
}

void
krt_scan_postconfig(struct krt_config *c UNUSED)
{
}

void
krt_scan_start(struct krt_proto *x, int first UNUSED)
{
  init_list(&x->scan.temp_ifs);
}

void
krt_scan_shutdown(struct krt_proto *x UNUSED, int last UNUSED)
{
}

static void
krt_sysctl_scan(struct proto *p, pool *pool, byte **buf, size_t *bl, int cmd)
{
  byte *next;
  int mib[6], on;
  size_t obl, needed;
  struct ks_msg *m;

  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;
  mib[3] = BIRD_PF;
  mib[4] = cmd;
  mib[5] = 0;

  if( sysctl(mib, 6 , NULL , &needed, NULL, 0) < 0)
  {
    die("RT scan...");
  }

  obl = *bl;

  while(needed > *bl) *bl *= 2;
  while(needed < (*bl/2)) *bl /= 2;

  if( (obl!=*bl) || !*buf)
  {
    if(*buf) mb_free(*buf);
    if( (*buf = mb_alloc(pool, *bl)) == NULL ) die("RT scan buf alloc");
  }

  on = needed;

  if( sysctl(mib, 6 , *buf, &needed, NULL, 0) < 0)
  {
    if(on != needed) return; 	/* The buffer size changed since last sysctl */
    die("RT scan 2");
  }

  for (next = *buf; next < (*buf + needed); next += m->rtm.rtm_msglen)
  {
    m = (struct ks_msg *)next;
    krt_read_msg(p, m, 1);
  }
}

void
krt_scan_fire(struct krt_proto *p)
{
  static byte *buf = NULL;
  static size_t bl = 32768;
  krt_sysctl_scan((struct proto *)p , p->krt_pool, &buf, &bl, NET_RT_DUMP);
}

void
krt_if_scan(struct kif_proto *p)
{
  static byte *buf = NULL;
  static size_t bl = 4096;
  struct proto *P = (struct proto *)p;
  if_start_update();
  krt_sysctl_scan(P, P->pool, &buf, &bl, NET_RT_IFLIST);
  if_end_update();
}


void
krt_set_construct(struct krt_config *c UNUSED)
{
}

void
krt_set_shutdown(struct krt_proto *x UNUSED, int last UNUSED)
{
} 

void
krt_if_io_init(void)
{
}

void
krt_if_construct(struct kif_config *c UNUSED)
{
}

void
krt_if_start(struct kif_proto *p UNUSED)
{
}

void
krt_if_shutdown(struct kif_proto *p UNUSED)
{
}

