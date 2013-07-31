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

#define OPT_PREFIX	3
#define OPT_MTU		5
#define OPT_RDNSS	25
#define OPT_DNSSL	31

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

struct radv_opt_rdnss
{
  u8 type;
  u8 length;
  u16 reserved;
  u32 lifetime;
  ip_addr servers[];
};

struct radv_opt_dnssl
{
  u8 type;
  u8 length;
  u16 reserved;
  u32 lifetime;
  char domain[];
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

static int
radv_prepare_rdnss(struct radv_iface *ifa, list *rdnss_list, char **buf, char *bufend)
{
  struct radv_rdnss_config *rcf = HEAD(*rdnss_list);

  while(NODE_VALID(rcf))
  {
    struct radv_rdnss_config *rcf_base = rcf;
    struct radv_opt_rdnss *op = (void *) *buf;
    int max_i = (bufend - *buf - sizeof(struct radv_opt_rdnss)) / sizeof(ip_addr);
    int i = 0;

    if (max_i < 1)
      goto too_much;

    op->type = OPT_RDNSS;
    op->reserved = 0;

    if (rcf->lifetime_mult)
      op->lifetime = htonl(rcf->lifetime_mult * ifa->cf->max_ra_int);
    else
      op->lifetime = htonl(rcf->lifetime);

    while(NODE_VALID(rcf) && 
	  (rcf->lifetime == rcf_base->lifetime) &&
	  (rcf->lifetime_mult == rcf_base->lifetime_mult))
      {
	if (i >= max_i)
	  goto too_much;

	op->servers[i] = rcf->server;
	ipa_hton(op->servers[i]);
	i++;

	rcf = NODE_NEXT(rcf);
      }
  
    op->length = 1+2*i;
    *buf += 8 * op->length;
  }

  return 0;

 too_much:
  log(L_WARN "%s: Too many RA options on interface %s",
      ifa->ra->p.name, ifa->iface->name);
  return -1;
}

int
radv_process_domain(struct radv_dnssl_config *cf)
{
  /* Format of domain in search list is <size> <label> <size> <label> ... 0 */

  char *dom = cf->domain;
  char *dom_end = dom; /* Just to  */
  u8 *dlen_save = &cf->dlen_first;
  int len;

  while (dom_end)
  {
    dom_end = strchr(dom, '.');
    len = dom_end ? (dom_end - dom) : strlen(dom);

    if (len < 1 || len > 63)
      return -1;

    *dlen_save = len;
    dlen_save = (u8 *) dom_end;

    dom += len + 1;
  }

  len = dom - cf->domain;
  if (len > 254)
    return -1;

  cf->dlen_all = len;

  return 0;
}

static int
radv_prepare_dnssl(struct radv_iface *ifa, list *dnssl_list, char **buf, char *bufend)
{
  struct radv_dnssl_config *dcf = HEAD(*dnssl_list);

  while(NODE_VALID(dcf))
  {
    struct radv_dnssl_config *dcf_base = dcf;
    struct radv_opt_dnssl *op = (void *) *buf;
    int bsize = bufend - *buf - sizeof(struct radv_opt_dnssl);
    int bpos = 0;

    if (bsize < 0)
      goto too_much;

    bsize = bsize & ~7; /* Round down to multiples of 8 */

    op->type = OPT_DNSSL;
    op->reserved = 0;

    if (dcf->lifetime_mult)
      op->lifetime = htonl(dcf->lifetime_mult * ifa->cf->max_ra_int);
    else
      op->lifetime = htonl(dcf->lifetime);

    while(NODE_VALID(dcf) && 
	  (dcf->lifetime == dcf_base->lifetime) &&
	  (dcf->lifetime_mult == dcf_base->lifetime_mult))
      {
	if (bpos + dcf->dlen_all + 1 > bsize)
	  goto too_much;

	op->domain[bpos++] = dcf->dlen_first;
	memcpy(op->domain + bpos, dcf->domain, dcf->dlen_all);
	bpos += dcf->dlen_all;

	dcf = NODE_NEXT(dcf);
      }

    int blen = (bpos + 7) / 8;
    bzero(op->domain + bpos, 8 * blen - bpos);
    op->length = 1 + blen;
    *buf += 8 * op->length;
  }

  return 0;

 too_much:
  log(L_WARN "%s: Too many RA options on interface %s",
      ifa->ra->p.name, ifa->iface->name);
  return -1;
}

static void
radv_prepare_ra(struct radv_iface *ifa)
{
  struct proto_radv *ra = ifa->ra;
  struct radv_config *cf = (struct radv_config *) (ra->p.cf);
  struct radv_iface_config *ic = ifa->cf;

  char *buf = ifa->sk->tbuf;
  char *bufstart = buf;
  char *bufend = buf + ifa->sk->tbsize;

  struct radv_ra_packet *pkt = (void *) buf;
  pkt->type = ICMPV6_RA;
  pkt->code = 0;
  pkt->checksum = 0;
  pkt->current_hop_limit = ic->current_hop_limit;
  pkt->flags = (ic->managed ? OPT_RA_MANAGED : 0) |
    (ic->other_config ? OPT_RA_OTHER_CFG : 0);
  pkt->router_lifetime = (ra->active || !ic->default_lifetime_sensitive) ?
    htons(ic->default_lifetime) : 0;
  pkt->reachable_time = htonl(ic->reachable_time);
  pkt->retrans_timer = htonl(ic->retrans_timer);
  buf += sizeof(*pkt);

  if (ic->link_mtu)
  {
    struct radv_opt_mtu *om = (void *) buf;
    om->type = OPT_MTU;
    om->length = 1;
    om->reserved = 0;
    om->mtu = htonl(ic->link_mtu);
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
      goto done;
    }

    struct radv_opt_prefix *op = (void *) buf;
    op->type = OPT_PREFIX;
    op->length = 4;
    op->pxlen = addr->pxlen;
    op->flags = (pc->onlink ? OPT_PX_ONLINK : 0) |
      (pc->autonomous ? OPT_PX_AUTONOMOUS : 0);
    op->valid_lifetime = (ra->active || !pc->valid_lifetime_sensitive) ?
      htonl(pc->valid_lifetime) : 0;
    op->preferred_lifetime = (ra->active || !pc->preferred_lifetime_sensitive) ?
      htonl(pc->preferred_lifetime) : 0;
    op->reserved = 0;
    op->prefix = addr->prefix;
    ipa_hton(op->prefix);
    buf += sizeof(*op);
  }

  if (! ic->rdnss_local)
    if (radv_prepare_rdnss(ifa, &cf->rdnss_list, &buf, bufend) < 0)
      goto done;

  if (radv_prepare_rdnss(ifa, &ic->rdnss_list, &buf, bufend) < 0)
    goto done;

  if (! ic->dnssl_local)
    if (radv_prepare_dnssl(ifa, &cf->dnssl_list, &buf, bufend) < 0)
      goto done;

  if (radv_prepare_dnssl(ifa, &ic->dnssl_list, &buf, bufend) < 0)
    goto done;

 done:
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
  log(L_ERR "%s: Socket error on %s: %M", ifa->ra->p.name, ifa->iface->name, err);
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

