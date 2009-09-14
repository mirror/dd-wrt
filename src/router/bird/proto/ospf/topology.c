/*
 *	BIRD -- OSPF Topological Database
 *
 *	(c) 1999       Martin Mares <mj@ucw.cz>
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "nest/bird.h"
#include "lib/string.h"

#include "ospf.h"

#define HASH_DEF_ORDER 6
#define HASH_HI_MARK *4
#define HASH_HI_STEP 2
#define HASH_HI_MAX 16
#define HASH_LO_MARK /5
#define HASH_LO_STEP 2
#define HASH_LO_MIN 8

int ptp_unnumbered_stub_lsa = 0;

static void *
lsab_alloc(struct proto_ospf *po, unsigned size)
{
  unsigned offset = po->lsab_used;
  po->lsab_used += size;
  if (po->lsab_used > po->lsab_size)
    {
      po->lsab_size = MAX(po->lsab_used, 2 * po->lsab_size);
      po->lsab = mb_realloc(po->proto.pool, po->lsab, po->lsab_size);
    }
  return ((byte *) po->lsab) + offset;
}

static inline void *
lsab_allocz(struct proto_ospf *po, unsigned size)
{
  void *r = lsab_alloc(po, size);
  bzero(r, size);
  return r;
}

static inline void *
lsab_flush(struct proto_ospf *po)
{
  void *r = mb_alloc(po->proto.pool, po->lsab_size);
  memcpy(r, po->lsab, po->lsab_used);
  po->lsab_used = 0;
  return r;
}

static int
configured_stubnet(struct ospf_area *oa, struct ifa *a)
{
  struct ospf_stubnet_config *sn;
  WALK_LIST(sn, oa->ac->stubnet_list)
    {
      if (sn->summary)
	{
	  if (ipa_in_net(a->prefix, sn->px.addr, sn->px.len) && (a->pxlen >= sn->px.len))
	    return 1;
	}
      else
	{
	  if (ipa_equal(a->prefix, sn->px.addr) && (a->pxlen == sn->px.len))
	    return 1;
	}
    }
  return 0;
}

static void *
originate_rt_lsa_body(struct ospf_area *oa, u16 * length)
{
  struct proto_ospf *po = oa->po;
  struct ospf_iface *ifa;
  int i = 0, j = 0, k = 0, bitv = 0;
  struct ospf_lsa_rt *rt;
  struct ospf_lsa_rt_link *ln;
  struct ospf_neighbor *neigh;

  DBG("%s: Originating RT_lsa body for area %R.\n", po->proto.name,
      oa->areaid);
    
  ASSERT(po->lsab_used == 0);
  rt = lsab_allocz(po, sizeof(struct ospf_lsa_rt));
  if (po->areano > 1)
    rt->veb.bit.b = 1;
  if ((po->ebit) && (!oa->stub))
    rt->veb.bit.e = 1;
  rt = NULL; /* buffer might be reallocated later */

  WALK_LIST(ifa, po->iface_list)
  {
    int master = 0;

    if ((ifa->type == OSPF_IT_VLINK) && (ifa->voa == oa) &&
	(!EMPTY_LIST(ifa->neigh_list)))
    {
      neigh = (struct ospf_neighbor *) HEAD(ifa->neigh_list);
      if ((neigh->state == NEIGHBOR_FULL) && (ifa->cost <= 0xffff))
	bitv = 1;
    }

    if ((ifa->oa != oa) || (ifa->state == OSPF_IS_DOWN))
      continue;

    /* BIRD does not support interface loops */
    ASSERT(ifa->state != OSPF_IS_LOOP);

    switch (ifa->type)
      {
      case OSPF_IT_PTP:	/* RFC2328 - 12.4.1.1 */
	neigh = (struct ospf_neighbor *) HEAD(ifa->neigh_list);
	if ((!EMPTY_LIST(ifa->neigh_list)) && (neigh->state == NEIGHBOR_FULL))
	{
	  ln = lsab_alloc(po, sizeof(struct ospf_lsa_rt_link));
	  ln->type = LSART_PTP;
	  ln->id = neigh->rid;
	  ln->data = (ifa->iface->addr->flags & IA_UNNUMBERED) ?
	    ifa->iface->index : ipa_to_u32(ifa->iface->addr->ip);
	  ln->metric = ifa->cost;
	  ln->notos = 0;
	  i++;
	  master = 1;
	}
	break;

      case OSPF_IT_BCAST: /* RFC2328 - 12.4.1.2 */
      case OSPF_IT_NBMA:
	if (ifa->state == OSPF_IS_WAITING)
	  break;

	j = 0, k = 0;
	WALK_LIST(neigh, ifa->neigh_list)
	  {
	    if ((neigh->rid == ifa->drid) && (neigh->state == NEIGHBOR_FULL))
	      k = 1;
	    if (neigh->state == NEIGHBOR_FULL)
	      j = 1;
	  }

	if (((ifa->state == OSPF_IS_DR) && (j == 1)) || (k == 1))
	  {
	    ln = lsab_alloc(po, sizeof(struct ospf_lsa_rt_link));
	    ln->type = LSART_NET;
	    ln->id = ipa_to_u32(ifa->drip);
	    ln->data = ipa_to_u32(ifa->iface->addr->ip);
	    ln->metric = ifa->cost;
	    ln->notos = 0;
	    i++;
	    master = 1;
	  }
	break;

      case OSPF_IT_VLINK: /* RFC2328 - 12.4.1.3 */
	neigh = (struct ospf_neighbor *) HEAD(ifa->neigh_list);
	if ((!EMPTY_LIST(ifa->neigh_list)) && (neigh->state == NEIGHBOR_FULL) && (ifa->cost <= 0xffff))
	{
	  ln = lsab_alloc(po, sizeof(struct ospf_lsa_rt_link));
	  ln->type = LSART_VLNK;
	  ln->id = neigh->rid;
	  ln->data = ipa_to_u32(ifa->iface->addr->ip);
	  ln->metric = ifa->cost;
	  ln->notos = 0;
	  i++;
	  master = 1;
        }
        break;

      default:
        log("Unknown interface type %s", ifa->iface->name);
        break;
      }

    /* Now we will originate stub areas for interfaces addresses */
    struct ifa *a;
    WALK_LIST(a, ifa->iface->addrs)
      {
	if (((a == ifa->iface->addr) && master) ||
	    (a->flags & IA_SECONDARY) ||
	    (a->flags & IA_UNNUMBERED) ||
	    configured_stubnet(oa, a))
	  continue;


	ln = lsab_alloc(po, sizeof(struct ospf_lsa_rt_link));
	ln->type = LSART_STUB;
	ln->id = ipa_to_u32(a->prefix);
	ln->data = ipa_to_u32(ipa_mkmask(a->pxlen));
	ln->metric = ifa->cost;
	ln->notos = 0;
	i++;
      }
  }

  struct ospf_stubnet_config *sn;
  WALK_LIST(sn, oa->ac->stubnet_list)
    if (!sn->hidden)
      {
	ln = lsab_alloc(po, sizeof(struct ospf_lsa_rt_link));
	ln->type = LSART_STUB;
	ln->id = ipa_to_u32(sn->px.addr);
	ln->data = ipa_to_u32(ipa_mkmask(sn->px.len));
	ln->metric = sn->cost;
	ln->notos = 0;
	i++;
      }

  rt = po->lsab;
  rt->links = i;
  rt->veb.bit.v = bitv;
  *length = po->lsab_used + sizeof(struct ospf_lsa_header);
  return lsab_flush(po);
}

/**
 * originate_rt_lsa - build new instance of router LSA
 * @oa: ospf_area which is LSA built to
 *
 * It builds router LSA walking through all OSPF interfaces in
 * specified OSPF area. This function is mostly called from
 * area_disp(). Builds new LSA, increases sequence number (if old
 * instance exists) and sets age of LSA to zero.
 */
void
originate_rt_lsa(struct ospf_area *oa)
{
  struct ospf_lsa_header lsa;
  struct proto_ospf *po = oa->po;
  struct proto *p = &po->proto;
  u32 rtid = po->proto.cf->global->router_id;
  struct top_hash_entry *en;
  void *body;

  if ((oa->rt) && ((oa->rt->inst_t + MINLSINTERVAL)) > now)
    return;
  /*
   * Tick is probably set to very low value. We cannot
   * originate new LSA before MINLSINTERVAL. We will
   * try to do it next tick.
   */

  OSPF_TRACE(D_EVENTS, "Originating RT_lsa for area %R.", oa->areaid);

  lsa.age = 0;
  lsa.id = rtid;
  lsa.type = LSA_T_RT;
  lsa.rt = rtid;
  lsa.options = oa->opt.byte;
  if (oa->rt == NULL)
  {
    lsa.sn = LSA_INITSEQNO;
  }
  else
  {
    lsa.sn = oa->rt->lsa.sn + 1;
  }
  body = originate_rt_lsa_body(oa, &lsa.length);
  lsasum_calculate(&lsa, body);
  en = lsa_install_new(&lsa, body, oa);
  oa->rt = en;
  ospf_lsupd_flood(NULL, NULL, &oa->rt->lsa, NULL, oa, 1);
  schedule_rtcalc(po);
  oa->origrt = 0;
}

static void *
originate_net_lsa_body(struct ospf_iface *ifa, u16 * length,
		       struct proto_ospf *po)
{
  u16 i = 1;
  struct ospf_neighbor *n;
  u32 *body;
  struct ospf_lsa_net *net;

  net = mb_alloc(po->proto.pool, sizeof(u32) * (ifa->fadj + 1) +
		 sizeof(struct ospf_lsa_net));
  net->netmask = ipa_mkmask(ifa->iface->addr->pxlen);

  body = (u32 *) (net + 1);
  i = 1;
  *body = po->proto.cf->global->router_id;
  WALK_LIST(n, ifa->neigh_list)
  {
    if (n->state == NEIGHBOR_FULL)
    {
      *(body + i) = n->rid;
      i++;
    }
  }
  *length = i * sizeof(u32) + sizeof(struct ospf_lsa_header) +
    sizeof(struct ospf_lsa_net);
  return net;
}

/**
 * originate_net_lsa - originates of deletes network LSA
 * @ifa: interface which is LSA originated for
 *
 * Interface counts number of adjacent neighbors. If this number is
 * lower than one or interface is not in state %OSPF_IS_DR it deletes
 * and premature ages instance of network LSA for specified interface.
 * In other case, new instance of network LSA is originated.
 */
void
originate_net_lsa(struct ospf_iface *ifa)
{
  struct proto_ospf *po = ifa->oa->po;
  struct ospf_lsa_header lsa;
  u32 rtid = po->proto.cf->global->router_id;
  struct proto *p = &po->proto;
  void *body;

  if (ifa->nlsa && ((ifa->nlsa->inst_t + MINLSINTERVAL) > now))
    return;
  /*
   * It's too early to originate new network LSA. We will
   * try to do it next tick
   */

  if ((ifa->state != OSPF_IS_DR) || (ifa->fadj == 0))
  {
    if (ifa->nlsa == NULL)
      return;

    OSPF_TRACE(D_EVENTS, "Deleting Net lsa for iface \"%s\".",
	       ifa->iface->name);
    ifa->nlsa->lsa.sn += 1;
    ifa->nlsa->lsa.age = LSA_MAXAGE;
    lsasum_calculate(&ifa->nlsa->lsa, ifa->nlsa->lsa_body);
    ospf_lsupd_flood(NULL, NULL, &ifa->nlsa->lsa, NULL, ifa->oa, 0);
    s_rem_node(SNODE ifa->nlsa);
    if (ifa->nlsa->lsa_body != NULL)
      mb_free(ifa->nlsa->lsa_body);
    ifa->nlsa->lsa_body = NULL;
    ospf_hash_delete(po->gr, ifa->nlsa);
    schedule_rtcalc(po);
    ifa->nlsa = NULL;
    return;
  }

  OSPF_TRACE(D_EVENTS, "Originating Net lsa for iface \"%s\".",
	     ifa->iface->name);

  lsa.age = 0;
  lsa.id = ipa_to_u32(ifa->iface->addr->ip);
  lsa.type = LSA_T_NET;
  lsa.rt = rtid;
  lsa.options = ifa->oa->opt.byte;
  if (ifa->nlsa == NULL)
  {
    lsa.sn = LSA_INITSEQNO;
  }
  else
  {
    lsa.sn = ifa->nlsa->lsa.sn + 1;
  }

  body = originate_net_lsa_body(ifa, &lsa.length, po);
  lsasum_calculate(&lsa, body);
  ifa->nlsa = lsa_install_new(&lsa, body, ifa->oa);
  ospf_lsupd_flood(NULL, NULL, &ifa->nlsa->lsa, NULL, ifa->oa, 1);
  ifa->orignet = 0;
}


static void *
originate_ext_lsa_body(net * n, rte * e, struct proto_ospf *po,
		       struct ea_list *attrs)
{
  struct proto *p = &po->proto;
  struct ospf_lsa_ext *ext;
  struct ospf_lsa_ext_tos *et;
  u32 m1 = ea_get_int(attrs, EA_OSPF_METRIC1, LSINFINITY);
  u32 m2 = ea_get_int(attrs, EA_OSPF_METRIC2, 10000);
  u32 tag = ea_get_int(attrs, EA_OSPF_TAG, 0);
  int inas = 0;

  ext = mb_alloc(p->pool, sizeof(struct ospf_lsa_ext) +
		 sizeof(struct ospf_lsa_ext_tos));
  ext->netmask = ipa_mkmask(n->n.pxlen);

  et = (struct ospf_lsa_ext_tos *) (ext + 1);

  if (m1 != LSINFINITY)
  {
    et->etm.metric = m1;
    et->etm.etos.tos = 0;
    et->etm.etos.ebit = 0;
  }
  else
  {
    et->etm.metric = m2;
    et->etm.etos.tos = 0;
    et->etm.etos.ebit = 1;
  }
  et->tag = tag;
  if (!ipa_equal(e->attrs->gw, IPA_NONE))
  {
    if (ospf_iface_find((struct proto_ospf *) p, e->attrs->iface) != NULL)
      inas = 1;
  }

  if (!inas)
    et->fwaddr = IPA_NONE;
  else
    et->fwaddr = e->attrs->gw;
  return ext;
}

/**
 * max_ext_lsa - calculate the maximum amount of external networks
 * possible for the given prefix length.
 * @pxlen: network prefix length
 *
 * This is a fix for the previous static use of MAXNETS which did
 * only work correct if MAXNETS < possible IPs for given prefix.
 * This solution is kind of a hack as there can now only be one
 * route for /32 type entries but this is better than the crashes
 * I did experience whith close together /32 routes originating
 * on different hosts.
 */

int
max_ext_lsa(unsigned pxlen)
{
  int i;
  for (i = 1; pxlen < BITS_PER_IP_ADDRESS; pxlen++, i <<= 1)
    if (i >= MAXNETS)
      return MAXNETS;
  return i;
}

void
flush_sum_lsa(struct ospf_area *oa, struct fib_node *fn, int type)
{
  struct proto_ospf *po = oa->po;
  struct proto *p = &po->proto;
  struct top_hash_entry *en;
  u32 rtid = po->proto.cf->global->router_id;
  struct ospf_lsa_header lsa;
  int max, i;
  struct ospf_lsa_sum *sum = NULL;

  lsa.rt = rtid;
  lsa.type = LSA_T_SUM_NET;
  if (type == ORT_ROUTER)
    lsa.type = LSA_T_SUM_RT;

  max = max_ext_lsa(fn->pxlen);

  for (i = 0; i < max; i++)
  {
    lsa.id = ipa_to_u32(fn->prefix) + i;
    if ((en = ospf_hash_find_header(po->gr, oa->areaid, &lsa)) != NULL)
    {
      sum = en->lsa_body;
      if ((type == ORT_ROUTER) || (fn->pxlen == ipa_mklen(sum->netmask)))
      {
        en->lsa.age = LSA_MAXAGE;
        en->lsa.sn = LSA_MAXSEQNO;
        lsasum_calculate(&en->lsa, sum);
        OSPF_TRACE(D_EVENTS, "Flushing summary lsa. (id=%R, type=%d)",
		   en->lsa.id, en->lsa.type);
        ospf_lsupd_flood(NULL, NULL, &en->lsa, NULL, oa, 1);
        if (can_flush_lsa(po)) flush_lsa(en, po);
        break;
      }
    }
  }
}

void
originate_sum_lsa(struct ospf_area *oa, struct fib_node *fn, int type, int metric)
{
  struct proto_ospf *po = oa->po;
  struct proto *p = &po->proto;
  struct top_hash_entry *en;
  u32 rtid = po->proto.cf->global->router_id;
  struct ospf_lsa_header lsa;
  int i, max, mlen = fn->pxlen, free = 0;
  u32 freeid = 0xFFFF;
  struct ospf_lsa_sum *sum = NULL;
  union ospf_lsa_sum_tm *tm;
  lsa.type = LSA_T_SUM_NET;

  if (type == ORT_ROUTER)
  {
    lsa.type = LSA_T_SUM_RT;
    mlen = 0;
  }

  lsa.age = 0;
  lsa.rt = rtid;
  lsa.sn = LSA_INITSEQNO;
  lsa.length = sizeof(struct ospf_lsa_sum) + sizeof(union ospf_lsa_sum_tm) +
    sizeof(struct ospf_lsa_header);
  lsa.options = oa->opt.byte;

  max = max_ext_lsa(fn->pxlen);
  for (i = 0; i < max; i++)
  {
    lsa.id = ipa_to_u32(fn->prefix) + i;
    if ((en = ospf_hash_find_header(po->gr, oa->areaid, &lsa)) == NULL)
    {
      if (!free)
      {
        freeid = lsa.id;
	free = 1;
      }
    }
    else
    {
      sum = en->lsa_body;
      if (mlen == ipa_mklen(sum->netmask))
      {
        tm = (union ospf_lsa_sum_tm *) (sum + 1);
        if (tm->metric == (unsigned) metric) return;	/* No reason for origination */
        lsa.sn = en->lsa.sn + 1;
        freeid = en->lsa.id;
	free = 1;
        break;
      }
    }
  }

  if(!free)
  {
    log("%s: got more routes for one /%d network then %d, ignoring", p->name,
        fn->pxlen, max);
    return;
  }
  lsa.id = freeid;
  
  OSPF_TRACE(D_EVENTS, "Originating summary (type %d) lsa for %I/%d (met %d).", lsa.type, fn->prefix,
             fn->pxlen, metric);

  sum = mb_alloc(p->pool, sizeof(struct ospf_lsa_sum) + sizeof(union ospf_lsa_sum_tm));
  sum->netmask = ipa_mkmask(mlen);
  tm = (union ospf_lsa_sum_tm *) (sum + 1);
  tm->metric = metric;
  tm->tos.tos = 0;

  lsasum_calculate(&lsa, sum);
  en = lsa_install_new(&lsa, sum, oa);
  ospf_lsupd_flood(NULL, NULL, &en->lsa, NULL, oa, 1);
}

void
check_sum_lsa(struct proto_ospf *po, ort *nf, int dest)
{
  struct ospf_area *oa;
  int flush, mlen;
  ip_addr ip;

  if (po->areano < 2) return;

  if ((nf->n.type > RTS_OSPF_IA) && (nf->o.type > RTS_OSPF_IA)) return;

#ifdef LOCAL_DEBUG
  DBG("Checking...dest = %d, %I/%d\n", dest, nf->fn.prefix, nf->fn.pxlen);
  if (nf->n.oa) DBG("New: met=%d, oa=%d\n", nf->n.metric1, nf->n.oa->areaid);
  if (nf->o.oa) DBG("Old: met=%d, oa=%d\n", nf->o.metric1, nf->o.oa->areaid);
#endif

  WALK_LIST(oa, po->area_list)
  {
    flush = 0;
    if ((nf->n.metric1 >= LSINFINITY) || (nf->n.type > RTS_OSPF_IA))
      flush = 1;
    if ((dest == ORT_ROUTER) && (!(nf->n.capa & ORTA_ASBR)))
      flush = 1;
    if ((!nf->n.oa) || (nf->n.oa->areaid == oa->areaid))
      flush = 1;

    if (nf->n.ifa) {
      if (nf->n.ifa->oa->areaid == oa->areaid)
        flush = 1;
      }
    else flush = 1;

    /* Don't send summary into stub areas
     * We send just default route (and later) */
    if (oa->stub)
      flush = 1;
    
    mlen = nf->fn.pxlen;
    ip = ipa_and(nf->fn.prefix, ipa_mkmask(mlen));

    if ((oa == po->backbone) && (nf->n.type == RTS_OSPF_IA)) flush = 1;	/* Only intra-area can go to the backbone */

    if ((!flush) && (dest == ORT_NET) && fib_route(&nf->n.oa->net_fib, ip, mlen))	/* The route fits into area networks */
    {
      flush = 1;
      if ((nf->n.oa == po->backbone) && (oa->trcap)) flush = 0;
    }

    if(flush)
      flush_sum_lsa(oa, &nf->fn, dest);
    else
      originate_sum_lsa(oa, &nf->fn, dest, nf->n.metric1);
  }
}

/**
 * originate_ext_lsa - new route received from nest and filters
 * @n: network prefix and mask
 * @e: rte
 * @po: current instance of OSPF
 * @attrs: list of extended attributes
 *
 * If I receive a message that new route is installed, I try to originate an
 * external LSA. The LSA header of such LSA does not contain information about
 * prefix length, so if I have to originate multiple LSAs for route with
 * different prefixes I try to increment prefix id to find a "free" one.
 *
 * The function also sets flag ebit. If it's the first time, the new router lsa
 * origination is necessary.
 */
void
originate_ext_lsa(net * n, rte * e, struct proto_ospf *po,
		  struct ea_list *attrs)
{
  struct ospf_lsa_header lsa;
  u32 rtid = po->proto.cf->global->router_id;
  struct top_hash_entry *en = NULL;
  void *body = NULL;
  struct proto *p = &po->proto;
  struct ospf_area *oa;
  struct ospf_lsa_ext *ext1, *ext2;
  int i, max;

  OSPF_TRACE(D_EVENTS, "Originating Ext lsa for %I/%d.", n->n.prefix,
	     n->n.pxlen);

  lsa.age = 0;
  lsa.id = ipa_to_u32(n->n.prefix);
  lsa.type = LSA_T_EXT;
  lsa.rt = rtid;
  lsa.sn = LSA_INITSEQNO;
  lsa.options = 0;

  body = originate_ext_lsa_body(n, e, po, attrs);
  lsa.length = sizeof(struct ospf_lsa_ext) + sizeof(struct ospf_lsa_ext_tos) +
    sizeof(struct ospf_lsa_header);
  ext1 = body;
  max = max_ext_lsa(n->n.pxlen);

  for (i = 0; i < max; i++)
  {
    if ((en = ospf_hash_find_header(po->gr, 0 , &lsa)) != NULL)
    {
      ext2 = en->lsa_body;
      if (ipa_compare(ext1->netmask, ext2->netmask) != 0)
	lsa.id++;
      else
	break;
    }
    else
      break;
  }

  if (i == max)
  {
    log("%s: got more routes for one /%d network then %d, ignoring", p->name,
	n->n.pxlen, max);
    mb_free(body);
    return;
  }
  lsasum_calculate(&lsa, body);
  WALK_LIST(oa, po->area_list)
  {
    if (!oa->stub)
    {
      en = lsa_install_new(&lsa, body, oa);
      ospf_lsupd_flood(NULL, NULL, &en->lsa, NULL, oa, 1);
      body = originate_ext_lsa_body(n, e, po, attrs);
    }
  }
  mb_free(body);

  if (po->ebit == 0)
  {
    po->ebit = 1;
    WALK_LIST(oa, po->area_list)
    {
      schedule_rt_lsa(oa);
    }
  }
}


static void
ospf_top_ht_alloc(struct top_graph *f)
{
  f->hash_size = 1 << f->hash_order;
  f->hash_mask = f->hash_size - 1;
  if (f->hash_order > HASH_HI_MAX - HASH_HI_STEP)
    f->hash_entries_max = ~0;
  else
    f->hash_entries_max = f->hash_size HASH_HI_MARK;
  if (f->hash_order < HASH_LO_MIN + HASH_LO_STEP)
    f->hash_entries_min = 0;
  else
    f->hash_entries_min = f->hash_size HASH_LO_MARK;
  DBG("Allocating OSPF hash of order %d: %d hash_entries, %d low, %d high\n",
      f->hash_order, f->hash_size, f->hash_entries_min, f->hash_entries_max);
  f->hash_table =
    mb_alloc(f->pool, f->hash_size * sizeof(struct top_hash_entry *));
  bzero(f->hash_table, f->hash_size * sizeof(struct top_hash_entry *));
}

static inline void
ospf_top_ht_free(struct top_hash_entry **h)
{
  mb_free(h);
}

static inline u32
ospf_top_hash_u32(u32 a)
{
  /* Shamelessly stolen from IP address hashing in ipv4.h */
  a ^= a >> 16;
  a ^= a << 10;
  return a;
}

static inline unsigned
ospf_top_hash(struct top_graph *f, u32 areaid, u32 lsaid, u32 rtrid, u32 type)
{
#if 1				/* Dirty patch to make rt table calculation work. */
  return (ospf_top_hash_u32(lsaid) +
	  ospf_top_hash_u32((type ==
			     LSA_T_NET) ? lsaid : rtrid) + type +
          (type == LSA_T_EXT ? 0 : areaid)) & f->hash_mask;
#else
  return (ospf_top_hash_u32(lsaid) + ospf_top_hash_u32(rtrid) +
	  type + areaid) & f->hash_mask;
#endif
}

/**
 * ospf_top_new - allocated new topology database
 * @p: current instance of OSPF
 *
 * This dynamically hashed structure is often used for keeping LSAs. Mainly
 * its used in @ospf_area structure.
 */
struct top_graph *
ospf_top_new(pool *pool)
{
  struct top_graph *f;

  f = mb_allocz(pool, sizeof(struct top_graph));
  f->pool = pool;
  f->hash_slab = sl_new(f->pool, sizeof(struct top_hash_entry));
  f->hash_order = HASH_DEF_ORDER;
  ospf_top_ht_alloc(f);
  f->hash_entries = 0;
  f->hash_entries_min = 0;
  return f;
}

void
ospf_top_free(struct top_graph *f)
{
  rfree(f->hash_slab);
  ospf_top_ht_free(f->hash_table);
  mb_free(f);
}

static void
ospf_top_rehash(struct top_graph *f, int step)
{
  unsigned int oldn, oldh;
  struct top_hash_entry **n, **oldt, **newt, *e, *x;

  oldn = f->hash_size;
  oldt = f->hash_table;
  DBG("Re-hashing topology hash from order %d to %d\n", f->hash_order,
      f->hash_order + step);
  f->hash_order += step;
  ospf_top_ht_alloc(f);
  newt = f->hash_table;

  for (oldh = 0; oldh < oldn; oldh++)
  {
    e = oldt[oldh];
    while (e)
    {
      x = e->next;
      n = newt + ospf_top_hash(f, e->oa->areaid, e->lsa.id, e->lsa.rt, e->lsa.type);
      e->next = *n;
      *n = e;
      e = x;
    }
  }
  ospf_top_ht_free(oldt);
}

struct top_hash_entry *
ospf_hash_find_header(struct top_graph *f, u32 areaid, struct ospf_lsa_header *h)
{
  return ospf_hash_find(f, areaid, h->id, h->rt, h->type);
}

struct top_hash_entry *
ospf_hash_get_header(struct top_graph *f, struct ospf_area *oa, struct ospf_lsa_header *h)
{
  return ospf_hash_get(f, oa, h->id, h->rt, h->type);
}

struct top_hash_entry *
ospf_hash_find(struct top_graph *f, u32 areaid, u32 lsa, u32 rtr, u32 type)
{
  struct top_hash_entry *e;

  e = f->hash_table[ospf_top_hash(f, areaid, lsa, rtr, type)];

  /* Dirty patch to make rt table calculation work. */
  if (type == LSA_T_NET)
  {
    while (e && (e->lsa.id != lsa || e->lsa.type != LSA_T_NET || e->oa->areaid != areaid))
      e = e->next;
  }
  else if (type == LSA_T_EXT)
  {
    while (e && (e->lsa.id != lsa || e->lsa.type != type || e->lsa.rt != rtr))
      e = e->next;
  }
  else
  {
    while (e && (e->lsa.id != lsa || e->lsa.type != type || e->lsa.rt != rtr || e->oa->areaid != areaid))
      e = e->next;
  }

  return e;
}

struct top_hash_entry *
ospf_hash_get(struct top_graph *f, struct ospf_area *oa, u32 lsa, u32 rtr, u32 type)
{
  struct top_hash_entry **ee;
  struct top_hash_entry *e;
  u32 nareaid = (type == LSA_T_EXT ? 0 : oa->areaid);

  ee = f->hash_table + ospf_top_hash(f, nareaid, lsa, rtr, type);
  e = *ee;

  if (type == LSA_T_EXT)
  {
    while (e && (e->lsa.id != lsa || e->lsa.rt != rtr || e->lsa.type != type))
      e = e->next;
  }
  else
  {
    while (e && (e->lsa.id != lsa || e->lsa.rt != rtr || e->lsa.type != type || e->oa->areaid != nareaid))
      e = e->next;
  }

  if (e)
    return e;

  e = sl_alloc(f->hash_slab);
  e->color = OUTSPF;
  e->dist = LSINFINITY;
  e->nhi = NULL;
  e->nh = IPA_NONE;
  e->lb = IPA_NONE;
  e->lsa.id = lsa;
  e->lsa.rt = rtr;
  e->lsa.type = type;
  e->lsa_body = NULL;
  e->nhi = NULL;
  e->oa = oa;
  e->next = *ee;
  *ee = e;
  if (f->hash_entries++ > f->hash_entries_max)
    ospf_top_rehash(f, HASH_HI_STEP);
  return e;
}

void
ospf_hash_delete(struct top_graph *f, struct top_hash_entry *e)
{
  struct top_hash_entry **ee = f->hash_table + 
    ospf_top_hash(f, e->oa->areaid, e->lsa.id, e->lsa.rt, e->lsa.type);

  while (*ee)
  {
    if (*ee == e)
    {
      *ee = e->next;
      sl_free(f->hash_slab, e);
      if (f->hash_entries-- < f->hash_entries_min)
	ospf_top_rehash(f, -HASH_LO_STEP);
      return;
    }
    ee = &((*ee)->next);
  }
  bug("ospf_hash_delete() called for invalid node");
}

static void
ospf_dump_lsa(struct top_hash_entry *he, struct proto *p)
{
  struct ospf_lsa_rt *rt = NULL;
  struct ospf_lsa_rt_link *rr = NULL;
  struct ospf_lsa_net *ln = NULL;
  u32 *rts = NULL;
  u32 i, max;

  OSPF_TRACE(D_EVENTS, "- %1x %-1R %-1R %4u 0x%08x 0x%04x %-1R",
	     he->lsa.type, he->lsa.id, he->lsa.rt, he->lsa.age, he->lsa.sn,
	     he->lsa.checksum, he->oa ? he->oa->areaid : 0 );

  switch (he->lsa.type)
    {
    case LSA_T_RT:
      rt = he->lsa_body;
      rr = (struct ospf_lsa_rt_link *) (rt + 1);

      for (i = 0; i < rt->links; i++)
        OSPF_TRACE(D_EVENTS, "  - %1x %-1R %-1R %5u",
		   rr[i].type, rr[i].id, rr[i].data, rr[i].metric);
      break;

    case LSA_T_NET:
      ln = he->lsa_body;
      rts = (u32 *) (ln + 1);
      max = (he->lsa.length - sizeof(struct ospf_lsa_header) -
		sizeof(struct ospf_lsa_net)) / sizeof(u32);

      for (i = 0; i < max; i++)
        OSPF_TRACE(D_EVENTS, "  - %-1R", rts[i]);
      break;

    default:
      break;
    }
}

void
ospf_top_dump(struct top_graph *f, struct proto *p)
{
  unsigned int i;
  OSPF_TRACE(D_EVENTS, "Hash entries: %d", f->hash_entries);

  for (i = 0; i < f->hash_size; i++)
  {
    struct top_hash_entry *e;
    for (e = f->hash_table[i]; e != NULL; e = e->next)
      ospf_dump_lsa(e, p);
  }
}

/* This is very inefficient, please don't call it often */

/* I should also test for every LSA if it's in some link state
 * retransmission list for every neighbor. I will not test it.
 * It could happen that I'll receive some strange ls ack's.
 */

int
can_flush_lsa(struct proto_ospf *po)
{
  struct ospf_iface *ifa;
  struct ospf_neighbor *n;

  WALK_LIST(ifa, po->iface_list)
  {
    WALK_LIST(n, ifa->neigh_list)
    {
      if ((n->state == NEIGHBOR_EXCHANGE) || (n->state == NEIGHBOR_LOADING))
        return 0;

      break;
    }
  }

  return 1;
}
