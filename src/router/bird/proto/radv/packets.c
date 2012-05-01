/*
 *	BIRD -- RAdv Packet Processing
 *
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */


#include <stdlib.h>
#include "radv.h"

struct radv_ra_packet
{
  u8 type;
  u8 code;
  u16 checksum;
  u8 current_hop_limit;
  u8 flags;
  u16 router_lifetime;
  u32 reachable_time;
  u32 retrans_timer;
};

#define OPT_RA_MANAGED 0x80
#define OPT_RA_OTHER_CFG 0x40

#define OPT_PREFIX 3
#define OPT_MTU 5

struct radv_opt_prefix
{
  u8 type;
  u8 length;
  u8 pxlen;
  u8 flags;
  u32 valid_lifetime;
  u32 preferred_lifetime;
  u32 reserved;
  ip_addr prefix;
};

#define OPT_PX_ONLINK 0x80
#define OPT_PX_AUTONOMOUS 0x40

struct radv_opt_mtu
{
  u8 type;
  u8 length;
  u16 reserved;
  u32 mtu;
};

static struct radv_prefix_config default_prefix = {
  .onlink = 1,
  .autonomous = 1,
  .valid_lifetime = DEFAULT_VALID_LIFETIME,
  .preferred_lifetime = DEFAULT_PREFERRED_LIFETIME
};

static struct radv_prefix_config *
radv_prefix_match(struct radv_iface *ifa, struct ifa *a)
{
  struct proto *p = &ifa->ra->p;
  struct radv_config *cf = (struct radv_config *) (p->cf);
  struct radv_prefix_config *pc;

  if (a->scope <= SCOPE_LINK)
    return NULL;

  WALK_LIST(pc, ifa->cf->pref_list)
    if ((a->pxlen >= pc->pxlen) && ipa_in_net(a->prefix, pc->prefix, pc->pxlen))
      return pc;

  WALK_LIST(pc, cf->pref_list)
    if ((a->pxlen >= pc->pxlen) && ipa_in_net(a->prefix, pc->prefix, pc->pxlen))
      return pc;

  return &default_prefix;
}

static void
radv_prepare_ra(struct radv_iface *ifa)
{
  struct proto_radv *ra = ifa->ra;

  char *buf = ifa->sk->tbuf;
  char *bufstart = buf;
  char *bufend = buf + ifa->sk->tbsize;

  struct radv_ra_packet *pkt = (void *) buf;
  pkt->type = ICMPV6_RA;
  pkt->code = 0;
  pkt->checksum = 0;
  pkt->current_hop_limit = ifa->cf->current_hop_limit;
  pkt->flags = (ifa->cf->managed ? OPT_RA_MANAGED : 0) |
    (ifa->cf->other_config ? OPT_RA_OTHER_CFG : 0);
  pkt->router_lifetime = htons(ifa->cf->default_lifetime);
  pkt->reachable_time = htonl(ifa->cf->reachable_time);
  pkt->retrans_timer = htonl(ifa->cf->retrans_timer);
  buf += sizeof(*pkt);

  if (ifa->cf->link_mtu)
  {
    struct radv_opt_mtu *om = (void *) buf;
    om->type = OPT_MTU;
    om->length = 1;
    om->reserved = 0;
    om->mtu = htonl(ifa->cf->link_mtu);
    buf += sizeof (*om);
  }

  struct ifa *addr;
  WALK_LIST(addr, ifa->iface->addrs)
  {
    struct radv_prefix_config *pc;
    pc = radv_prefix_match(ifa, addr);

    if (!pc || pc->skip)
      continue;

    if (buf + sizeof(struct radv_opt_prefix) > bufend)
    {
      log(L_WARN "%s: Too many prefixes on interface %s", ra->p.name, ifa->iface->name);
      break;
    }

    struct radv_opt_prefix *op = (void *) buf;
    op->type = OPT_PREFIX;
    op->length = 4;
    op->pxlen = addr->pxlen;
    op->flags = (pc->onlink ? OPT_PX_ONLINK : 0) |
      (pc->autonomous ? OPT_PX_AUTONOMOUS : 0);
    op->valid_lifetime = htonl(pc->valid_lifetime);
    op->preferred_lifetime = htonl(pc->preferred_lifetime);
    op->reserved = 0;
    op->prefix = addr->prefix;
    ipa_hton(op->prefix);
    buf += sizeof(*op);
  }
  
  ifa->plen = buf - bufstart;
}


void
radv_send_ra(struct radv_iface *ifa, int shutdown)
{
  struct proto_radv *ra = ifa->ra;

  /* We store prepared RA in tbuf */
  if (!ifa->plen)
    radv_prepare_ra(ifa);

  if (shutdown)
  {
    /* Modify router lifetime to 0, it is not restored because
       we suppose that the iface will be removed */
    struct radv_ra_packet *pkt = (void *) ifa->sk->tbuf;
    pkt->router_lifetime = 0;
  }

  RADV_TRACE(D_PACKETS, "Sending RA via %s", ifa->iface->name);
  sk_send_to(ifa->sk, ifa->plen, AllNodes, 0);
}


static int
radv_rx_hook(sock *sk, int size)
{
  struct radv_iface *ifa = sk->data;
  struct proto_radv *ra = ifa->ra;

  /* We want just packets from sk->iface */
  if (sk->lifindex != sk->iface->index)
    return 1;

  if (ipa_equal(sk->faddr, ifa->addr->ip))
    return 1;

  if (size < 8)
    return 1;

  byte *buf = sk->rbuf;

  if (buf[1] != 0)
    return 1;

  /* Validation is a bit sloppy - Hop Limit is not checked and
     length of options is ignored for RS and left to later for RA */

  switch (buf[0])
  {
  case ICMPV6_RS:
    RADV_TRACE(D_PACKETS, "Received RS from %I via %s",
	       sk->faddr, ifa->iface->name);
    radv_iface_notify(ifa, RA_EV_RS);
    return 1;

  case ICMPV6_RA:
    RADV_TRACE(D_PACKETS, "Received RA from %I via %s",
	       sk->faddr, ifa->iface->name);
    /* FIXME - there should be some checking of received RAs, but we just ignore them */
    return 1;

  default:
    return 1;
  }
}

static void
radv_tx_hook(sock *sk)
{
  struct radv_iface *ifa = sk->data;
  log(L_WARN "%s: TX hook called", ifa->ra->p.name);
}

static void
radv_err_hook(sock *sk, int err)
{
  struct radv_iface *ifa = sk->data;
  log(L_ERR "%s: Socket error: %m", ifa->ra->p.name, err);
}

int
radv_sk_open(struct radv_iface *ifa)
{
  sock *sk = sk_new(ifa->ra->p.pool);
  sk->type = SK_IP;
  sk->dport = ICMPV6_PROTO;
  sk->saddr = IPA_NONE;

  sk->ttl = 255; /* Mandatory for Neighbor Discovery packets */
  sk->rx_hook = radv_rx_hook;
  sk->tx_hook = radv_tx_hook;
  sk->err_hook = radv_err_hook;
  sk->iface = ifa->iface;
  sk->rbsize = 1024; // bufsize(ifa);
  sk->tbsize = 1024; // bufsize(ifa);
  sk->data = ifa;
  sk->flags = SKF_LADDR_RX;

  if (sk_open(sk) != 0)
    goto err;

  sk->saddr = ifa->addr->ip;

  /* We want listen just to ICMPv6 messages of type RS and RA */
  if (sk_set_icmp_filter(sk, ICMPV6_RS, ICMPV6_RA) < 0)
    goto err;

  if (sk_setup_multicast(sk) < 0)
    goto err;

  if (sk_join_group(sk, AllRouters) < 0)
    goto err;

  ifa->sk = sk;
  return 1;

 err:
  rfree(sk);
  return 0;
}

