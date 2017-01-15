/*
 *	BIRD -- OSPF
 *
 *	(c) 2000--2004 Ondrej Filip <feela@network.cz>
 *	(c) 2009--2014 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2009--2014 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"

static void add_cand(list * l, struct top_hash_entry *en,
		     struct top_hash_entry *par, u32 dist,
		     struct ospf_area *oa, int i);
static void rt_sync(struct ospf_proto *p);


static inline void reset_ri(ort *ort)
{
  bzero(&ort->n, sizeof(orta));
}

void
ospf_rt_initort(struct fib_node *fn)
{
  ort *ri = (ort *) fn;
  reset_ri(ri);
  ri->old_rta = NULL;
  ri->fn.flags = 0;
}

static inline int
nh_is_vlink(struct mpnh *nhs)
{
  return !nhs->iface;
}

static inline int
unresolved_vlink(ort *ort)
{
  return ort->n.nhs && nh_is_vlink(ort->n.nhs);
}

static inline struct mpnh *
new_nexthop(struct ospf_proto *p, ip_addr gw, struct iface *iface, byte weight)
{
  struct mpnh *nh = lp_alloc(p->nhpool, sizeof(struct mpnh));
  nh->gw = gw;
  nh->iface = iface;
  nh->next = NULL;
  nh->weight = weight;
  return nh;
}

/* Returns true if there are device nexthops in n */
static inline int
has_device_nexthops(const struct mpnh *n)
{
  for (; n; n = n->next)
    if (ipa_zero(n->gw))
      return 1;

  return 0;
}

/* Replace device nexthops with nexthops to gw */
static struct mpnh *
fix_device_nexthops(struct ospf_proto *p, const struct mpnh *n, ip_addr gw)
{
  struct mpnh *root1 = NULL;
  struct mpnh *root2 = NULL;
  struct mpnh **nn1 = &root1;
  struct mpnh **nn2 = &root2;

  if (!p->ecmp)
    return new_nexthop(p, gw, n->iface, n->weight);

  /* This is a bit tricky. We cannot just copy the list and update n->gw,
     because the list should stay sorted, so we create two lists, one with new
     gateways and one with old ones, and then merge them. */

  for (; n; n = n->next)
  {
    struct mpnh *nn = new_nexthop(p, ipa_zero(n->gw) ? gw : n->gw, n->iface, n->weight);

    if (ipa_zero(n->gw))
    {
      *nn1 = nn;
      nn1 = &(nn->next);
    }
    else
    {
      *nn2 = nn;
      nn2 = &(nn->next);
    }
  }

  return mpnh_merge(root1, root2, 1, 1, p->ecmp, p->nhpool);
}


/* Whether the ASBR or the forward address destination is preferred
   in AS external route selection according to 16.4.1. */
static inline int
epath_preferred(const orta *ep)
{
  return (ep->type == RTS_OSPF) && (ep->oa->areaid != 0);
}

/* Whether the ext route has ASBR/next_hop marked as preferred. */
static inline int
orta_pref(const orta *nf)
{
  return !!(nf->options & ORTA_PREF);
}

/* Classify orta entries according to RFC 3101 2.5 (6e) priorities:
   Type-7 LSA with P-bit, Type-5 LSA, Type-7 LSA without P-bit */
static int
orta_prio(const orta *nf)
{
  /* RFC 3103 2.5 (6e) priorities */
  u32 opts = nf->options & (ORTA_NSSA | ORTA_PROP);

  /* A Type-7 LSA with the P-bit set */
  if (opts == (ORTA_NSSA | ORTA_PROP))
    return 2;

  /* A Type-5 LSA */
  if (opts == 0)
    return 1;

  return 0;
}

/* Whether the route is better according to RFC 3101 2.5 (6e):
   Prioritize Type-7 LSA with P-bit, then Type-5 LSA, then higher router ID */
static int
orta_prefer_lsa(const orta *new, const orta *old)
{
  int pn = orta_prio(new);
  int po = orta_prio(old);

  return (pn > po) || ((pn == po) && (new->en->lsa.rt > old->en->lsa.rt));
}

/*
 * Compare an existing routing table entry with a new one. Applicable for
 * intra-area routes, inter-area routes and router entries. Returns integer
 * <, = or > than 0 if the new orta is less, equal or more preferred than
 * the old orta.
 */
static int
orta_compare(const struct ospf_proto *p, const orta *new, const orta *old)
{
  int r;

  if (old->type == RTS_DUMMY)
    return 1;

  /* Prefer intra-area to inter-area to externals */
  r = ((int) old->type) - ((int) new->type);
  if (r) return r;

  /* Prefer lowest type 1 metric */
  r = ((int) old->metric1) - ((int) new->metric1);
  if (r) return r;


  /* Rest is BIRD-specific */

  /* Area-wide routes should not mix next-hops from different areas.
     This generally should not happen unless there is some misconfiguration. */
  if (new->oa->areaid != old->oa->areaid)
    return (new->oa->areaid > old->oa->areaid) ? 1 : -1;

  /* Prefer routes for configured stubnets (!nhs) to regular routes to dummy
     vlink nexthops. We intentionally return -1 if both are stubnets or vlinks. */
  if (!old->nhs)
    return -1;
  if (!new->nhs)
    return 1;
  if (nh_is_vlink(new->nhs))
    return -1;
  if (nh_is_vlink(old->nhs))
    return 1;


  if (p->ecmp)
    return 0;

  /* Prefer routes with higher Router ID, just to be more deterministic */
  if (new->rid > old->rid)
    return 1;

  return -1;
}

/*
 * Compare ASBR routing table entry with a new one, used for precompute ASBRs
 * for AS external route selection (RFC 2328 16.4 (3)), Returns integer < or >
 * than 0 if the new ASBR is less or more preferred than the old ASBR.
 */
static int
orta_compare_asbr(const struct ospf_proto *p, const orta *new, const orta *old)
{
  int r;

  if (old->type == RTS_DUMMY)
    return 1;

  if (!p->rfc1583)
  {
    r = epath_preferred(new) - epath_preferred(old);
    if (r) return r;
  }

  r = ((int) old->metric1) - ((int) new->metric1);
  if (r) return r;

  /* Larger area ID is preferred */
  if (new->oa->areaid > old->oa->areaid)
    return 1;

  /* There is just one ASBR of that RID per area, so tie is not possible */
  return -1;
}

/*
 * Compare a routing table entry with a new one, for AS external routes
 * (RFC 2328 16.4) and NSSA routes (RFC 3103 2.5), Returns integer <, = or >
 * than 0 if the new orta is less, equal or more preferred than the old orta.
 */
static int
orta_compare_ext(const struct ospf_proto *p, const orta *new, const orta *old)
{
  int r;

  if (old->type == RTS_DUMMY)
    return 1;

  /* 16.4 (6a) - prefer routes with lower type */
  r = ((int) old->type) - ((int) new->type);
  if (r) return r;

  /* 16.4 (6b) - prefer routes with lower type 2 metric */
  if (new->type == RTS_OSPF_EXT2)
  {
    r = ((int) old->metric2) - ((int) new->metric2);
    if (r) return r;
  }

  /* 16.4 (6c) - if not RFC1583, prefer routes with preferred ASBR/next_hop */
  if (!p->rfc1583)
  {
    r = orta_pref(new) - orta_pref(old);
    if (r) return r;
  }

  /* 16.4 (6d) - prefer routes with lower type 1 metric */
  r = ((int) old->metric1) - ((int) new->metric1);
  if (r) return r;


  if (p->ecmp && p->merge_external)
    return 0;

  /*
   * RFC 3101 2.5 (6e) - prioritize Type-7 LSA with P-bit, then Type-5 LSA, then
   * LSA with higher router ID. Although this should apply just to functionally
   * equivalent LSAs (i.e. ones with the same non-zero forwarding address), we
   * use it also to disambiguate otherwise equally preferred nexthops.
   */
  if (orta_prefer_lsa(new, old))
    return 1;

  return -1;
}


static inline void
ort_replace(ort *o, const orta *new)
{
  memcpy(&o->n, new, sizeof(orta));
}

static void
ort_merge(struct ospf_proto *p, ort *o, const orta *new)
{
  orta *old = &o->n;

  if (old->nhs != new->nhs)
  {
    old->nhs = mpnh_merge(old->nhs, new->nhs, old->nhs_reuse, new->nhs_reuse,
			  p->ecmp, p->nhpool);
    old->nhs_reuse = 1;
  }

  if (old->rid < new->rid)
    old->rid = new->rid;
}

static void
ort_merge_ext(struct ospf_proto *p, ort *o, const orta *new)
{
  orta *old = &o->n;

  if (old->nhs != new->nhs)
  {
    old->nhs = mpnh_merge(old->nhs, new->nhs, old->nhs_reuse, new->nhs_reuse,
			  p->ecmp, p->nhpool);
    old->nhs_reuse = 1;
  }

  if (old->tag != new->tag)
    old->tag = 0;

  /*
   * Even with multipath, we store only one LSA in orta.en for the purpose of
   * NSSA/ext translation. Therefore, we apply procedures from RFC 3101 2.5 (6e)
   * to all chosen LSAs for given network, not just to functionally equivalent
   * ones (i.e. ones with the same non-zero forwarding address).
   */
  if (orta_prefer_lsa(new, old))
  {
    old->options = new->options;
    old->rid = new->rid;
    old->oa = new->oa;
    old->en = new->en;
  }
}



static inline void
ri_install_net(struct ospf_proto *p, ip_addr prefix, int pxlen, const orta *new)
{
  ort *old = (ort *) fib_get(&p->rtf, &prefix, pxlen);
  int cmp = orta_compare(p, new, &old->n);

  if (cmp > 0)
    ort_replace(old, new);
  else if (cmp == 0)
    ort_merge(p, old, new);
}

static inline void
ri_install_rt(struct ospf_area *oa, u32 rid, const orta *new)
{
  ip_addr addr = ipa_from_rid(rid);
  ort *old = (ort *) fib_get(&oa->rtr, &addr, MAX_PREFIX_LENGTH);
  int cmp = orta_compare(oa->po, new, &old->n);

  if (cmp > 0)
    ort_replace(old, new);
  else if (cmp == 0)
    ort_merge(oa->po, old, new);
}

static inline void
ri_install_asbr(struct ospf_proto *p, ip_addr *addr, const orta *new)
{
  ort *old = (ort *) fib_get(&p->backbone->rtr, addr, MAX_PREFIX_LENGTH);
  if (orta_compare_asbr(p, new, &old->n) > 0)
    ort_replace(old, new);
}

static inline void
ri_install_ext(struct ospf_proto *p, ip_addr prefix, int pxlen, const orta *new)
{
  ort *old = (ort *) fib_get(&p->rtf, &prefix, pxlen);
  int cmp = orta_compare_ext(p, new, &old->n);

  if (cmp > 0)
    ort_replace(old, new);
  else if (cmp == 0)
    ort_merge_ext(p, old, new);
}

static inline struct ospf_iface *
rt_pos_to_ifa(struct ospf_area *oa, int pos)
{
  struct ospf_iface *ifa;

  WALK_LIST(ifa, oa->po->iface_list)
    if (ifa->oa == oa && pos >= ifa->rt_pos_beg && pos < ifa->rt_pos_end)
      return ifa;

  return NULL;
}

static inline struct ospf_iface *
px_pos_to_ifa(struct ospf_area *oa, int pos)
{
  struct ospf_iface *ifa;

  WALK_LIST(ifa, oa->po->iface_list)
    if (ifa->oa == oa && pos >= ifa->px_pos_beg && pos < ifa->px_pos_end)
      return ifa;

  return NULL;
}


static void
add_network(struct ospf_area *oa, ip_addr px, int pxlen, int metric, struct top_hash_entry *en, int pos)
{
  struct ospf_proto *p = oa->po;

  orta nf = {
    .type = RTS_OSPF,
    .options = 0,
    .metric1 = metric,
    .metric2 = LSINFINITY,
    .tag = 0,
    .rid = en->lsa.rt,
    .oa = oa,
    .nhs = en->nhs
  };

  if (pxlen < 0 || pxlen > MAX_PREFIX_LENGTH)
  {
    log(L_WARN "%s: Invalid prefix in LSA (Type: %04x, Id: %R, Rt: %R)",
	p->p.name, en->lsa_type, en->lsa.id, en->lsa.rt);
    return;
  }

  if (en == oa->rt)
  {
    /*
     * Local stub networks do not have proper iface in en->nhi (because they all
     * have common top_hash_entry en). We have to find iface responsible for
     * that stub network. Configured stubnets do not have any iface. They will
     * be removed in rt_sync().
     */

    struct ospf_iface *ifa;
    ifa = ospf_is_v2(p) ? rt_pos_to_ifa(oa, pos) : px_pos_to_ifa(oa, pos);
    nf.nhs = ifa ? new_nexthop(p, IPA_NONE, ifa->iface, ifa->ecmp_weight) : NULL;
  }

  ri_install_net(p, px, pxlen, &nf);
}



static inline void
spfa_process_rt(struct ospf_proto *p, struct ospf_area *oa, struct top_hash_entry *act)
{
  struct ospf_lsa_rt *rt = act->lsa_body;
  struct ospf_lsa_rt_walk rtl;
  struct top_hash_entry *tmp;
  ip_addr prefix;
  int pxlen, i;

  if (rt->options & OPT_RT_V)
    oa->trcap = 1;

  /*
   * In OSPFv3, all routers are added to per-area routing
   * tables. But we use it just for ASBRs and ABRs. For the
   * purpose of the last step in SPF - prefix-LSA processing in
   * spfa_process_prefixes(), we use information stored in LSA db.
   */
  if (((rt->options & OPT_RT_E) || (rt->options & OPT_RT_B))
      && (act->lsa.rt != p->router_id))
  {
    orta nf = {
      .type = RTS_OSPF,
      .options = rt->options,
      .metric1 = act->dist,
      .metric2 = LSINFINITY,
      .tag = 0,
      .rid = act->lsa.rt,
      .oa = oa,
      .nhs = act->nhs
    };
    ri_install_rt(oa, act->lsa.rt, &nf);
  }

  /* Errata 2078 to RFC 5340 4.8.1 - skip links from non-routing nodes */
  if (ospf_is_v3(p) && (act != oa->rt) && !(rt->options & OPT_R))
    return;

  /* Now process Rt links */
  for (lsa_walk_rt_init(p, act, &rtl), i = 0; lsa_walk_rt(&rtl); i++)
  {
    tmp = NULL;

    switch (rtl.type)
    {
    case LSART_STUB:

      /* Should not happen, LSART_STUB is not defined in OSPFv3 */
      if (ospf_is_v3(p))
	break;

      /*
       * RFC 2328 in 16.1. (2a) says to handle stub networks in an
       * second phase after the SPF for an area is calculated. We get
       * the same result by handing them here because add_network()
       * will keep the best (not the first) found route.
       */
      prefix = ipa_from_u32(rtl.id & rtl.data);
      pxlen = u32_masklen(rtl.data);
      add_network(oa, prefix, pxlen, act->dist + rtl.metric, act, i);
      break;

    case LSART_NET:
      tmp = ospf_hash_find_net(p->gr, oa->areaid, rtl.id, rtl.nif);
      break;

    case LSART_VLNK:
    case LSART_PTP:
      tmp = ospf_hash_find_rt(p->gr, oa->areaid, rtl.id);
      break;
    }

    add_cand(&oa->cand, tmp, act, act->dist + rtl.metric, oa, i);
  }
}

static inline void
spfa_process_net(struct ospf_proto *p, struct ospf_area *oa, struct top_hash_entry *act)
{
  struct ospf_lsa_net *ln = act->lsa_body;
  struct top_hash_entry *tmp;
  ip_addr prefix;
  int pxlen, i, cnt;

  if (ospf_is_v2(p))
  {
    prefix = ipa_from_u32(act->lsa.id & ln->optx);
    pxlen = u32_masklen(ln->optx);
    add_network(oa, prefix, pxlen, act->dist, act, -1);
  }

  cnt = lsa_net_count(&act->lsa);
  for (i = 0; i < cnt; i++)
  {
    tmp = ospf_hash_find_rt(p->gr, oa->areaid, ln->routers[i]);
    add_cand(&oa->cand, tmp, act, act->dist, oa, -1);
  }
}

static inline void
spfa_process_prefixes(struct ospf_proto *p, struct ospf_area *oa)
{
  struct top_hash_entry *en, *src;
  struct ospf_lsa_prefix *px;
  ip_addr pxa;
  int pxlen;
  u8 pxopts;
  u16 metric;
  u32 *buf;
  int i;

  WALK_SLIST(en, p->lsal)
  {
    if (en->lsa_type != LSA_T_PREFIX)
      continue;

    if (en->domain != oa->areaid)
      continue;

    if (en->lsa.age == LSA_MAXAGE)
      continue;

    px = en->lsa_body;

    /* For router prefix-LSA, we would like to find the first router-LSA */
    if (px->ref_type == LSA_T_RT)
      src = ospf_hash_find_rt(p->gr, oa->areaid, px->ref_rt);
    else
      src = ospf_hash_find(p->gr, oa->areaid, px->ref_id, px->ref_rt, px->ref_type);

    if (!src)
      continue;

    /* Reachable in SPF */
    if (src->color != INSPF)
      continue;

    if ((src->lsa_type != LSA_T_RT) && (src->lsa_type != LSA_T_NET))
      continue;

    buf = px->rest;
    for (i = 0; i < px->pxcount; i++)
      {
	buf = lsa_get_ipv6_prefix(buf, &pxa, &pxlen, &pxopts, &metric);

	if (pxopts & OPT_PX_NU)
	  continue;

	/* Store the first global address to use it later as a vlink endpoint */
	if ((pxopts & OPT_PX_LA) && ipa_zero(src->lb))
	  src->lb = pxa;

	add_network(oa, pxa, pxlen, src->dist + metric, src, i);
      }
  }
}

/* RFC 2328 16.1. calculating shortest paths for an area */
static void
ospf_rt_spfa(struct ospf_area *oa)
{
  struct ospf_proto *p = oa->po;
  struct top_hash_entry *act;
  node *n;

  if (oa->rt == NULL)
    return;
  if (oa->rt->lsa.age == LSA_MAXAGE)
    return;

  OSPF_TRACE(D_EVENTS, "Starting routing table calculation for area %R", oa->areaid);

  /* 16.1. (1) */
  init_list(&oa->cand);		/* Empty list of candidates */
  oa->trcap = 0;

  DBG("LSA db prepared, adding me into candidate list.\n");

  oa->rt->dist = 0;
  oa->rt->color = CANDIDATE;
  add_head(&oa->cand, &oa->rt->cn);
  DBG("RT LSA: rt: %R, id: %R, type: %u\n",
      oa->rt->lsa.rt, oa->rt->lsa.id, oa->rt->lsa_type);

  while (!EMPTY_LIST(oa->cand))
  {
    n = HEAD(oa->cand);
    act = SKIP_BACK(struct top_hash_entry, cn, n);
    rem_node(n);

    DBG("Working on LSA: rt: %R, id: %R, type: %u\n",
	act->lsa.rt, act->lsa.id, act->lsa_type);

    act->color = INSPF;
    switch (act->lsa_type)
    {
    case LSA_T_RT:
      spfa_process_rt(p, oa, act);
      break;

    case LSA_T_NET:
      spfa_process_net(p, oa, act);
      break;

    default:
      log(L_WARN "%s: Unknown LSA type in SPF: %d", p->p.name, act->lsa_type);
    }
  }

  if (ospf_is_v3(p))
    spfa_process_prefixes(p, oa);
}

static int
link_back(struct ospf_area *oa, struct top_hash_entry *en, struct top_hash_entry *par)
{
  struct ospf_proto *p = oa->po;
  struct ospf_lsa_rt_walk rtl;
  struct top_hash_entry *tmp;
  struct ospf_lsa_net *ln;
  u32 i, cnt;

  if (!en || !par) return 0;

  /* We should check whether there is a link back from en to par,
     this is used in SPF calc (RFC 2328 16.1. (2b)). According to RFC 2328
     note 23, we don't have to find the same link that is used for par
     to en, any link is enough. This we do for ptp links. For net-rt
     links, we have to find the same link to compute proper lb/lb_id,
     which may be later used as the next hop. */

  /* In OSPFv2, en->lb is set here. In OSPFv3, en->lb is just cleared here,
     it is set in process_prefixes() to any global address in the area */

  en->lb = IPA_NONE;
  en->lb_id = 0;

  switch (en->lsa_type)
  {
  case LSA_T_RT:
    lsa_walk_rt_init(p, en, &rtl);
    while (lsa_walk_rt(&rtl))
    {
      switch (rtl.type)
      {
      case LSART_STUB:
	break;

      case LSART_NET:
	tmp = ospf_hash_find_net(p->gr, oa->areaid, rtl.id, rtl.nif);
	if (tmp == par)
	{
	  if (ospf_is_v2(p))
	    en->lb = ipa_from_u32(rtl.data);
	  else
	    en->lb_id = rtl.lif;

	  return 1;
	}
	break;

      case LSART_VLNK:
      case LSART_PTP:
	/* Not necessary the same link, see RFC 2328 [23] */
	tmp = ospf_hash_find_rt(p->gr, oa->areaid, rtl.id);
	if (tmp == par)
	  return 1;
	break;
      }
    }
    break;

  case LSA_T_NET:
    ln = en->lsa_body;
    cnt = lsa_net_count(&en->lsa);
    for (i = 0; i < cnt; i++)
    {
      tmp = ospf_hash_find_rt(p->gr, oa->areaid, ln->routers[i]);
      if (tmp == par)
	return 1;
    }
    break;

  default:
    log(L_WARN "%s: Unknown LSA type in SPF: %d", p->p.name, en->lsa_type);
  }
  return 0;
}


/* RFC 2328 16.2. calculating inter-area routes */
static void
ospf_rt_sum(struct ospf_area *oa)
{
  struct ospf_proto *p = oa->po;
  struct top_hash_entry *en;
  ip_addr ip, abrip;
  u32 dst_rid, metric, options;
  ort *abr;
  int pxlen = -1, type = -1;
  u8 pxopts;


  OSPF_TRACE(D_EVENTS, "Starting routing table calculation for inter-area (area %R)", oa->areaid);

  WALK_SLIST(en, p->lsal)
  {
    if ((en->lsa_type != LSA_T_SUM_RT) && (en->lsa_type != LSA_T_SUM_NET))
      continue;

    if (en->domain != oa->areaid)
      continue;

    /* 16.2. (1a) */
    if (en->lsa.age == LSA_MAXAGE)
      continue;

    /* 16.2. (2) */
    if (en->lsa.rt == p->router_id)
      continue;

    /* 16.2. (3) is handled later in ospf_rt_abr() by resetting such rt entry */

    if (en->lsa_type == LSA_T_SUM_NET)
    {
      lsa_parse_sum_net(en, ospf_is_v2(p), &ip, &pxlen, &pxopts, &metric);

      if (pxopts & OPT_PX_NU)
	continue;

      if (pxlen < 0 || pxlen > MAX_PREFIX_LENGTH)
      {
	log(L_WARN "%s: Invalid prefix in LSA (Type: %04x, Id: %R, Rt: %R)",
	    p->p.name, en->lsa_type, en->lsa.id, en->lsa.rt);
	continue;
      }

      options = 0;
      type = ORT_NET;
    }
    else /* LSA_T_SUM_RT */
    {
      lsa_parse_sum_rt(en, ospf_is_v2(p), &dst_rid, &metric, &options);

      /* We don't want local router in ASBR routing table */
      if (dst_rid == p->router_id)
	continue;

      options |= ORTA_ASBR;
      type = ORT_ROUTER;
    }

    /* 16.2. (1b) */
    if (metric == LSINFINITY)
      continue;

    /* 16.2. (4) */
    abrip = ipa_from_rid(en->lsa.rt);
    abr = (ort *) fib_find(&oa->rtr, &abrip, MAX_PREFIX_LENGTH);
    if (!abr || !abr->n.type)
      continue;

    if (!(abr->n.options & ORTA_ABR))
      continue;

    /* This check is not mentioned in RFC 2328 */
    if (abr->n.type != RTS_OSPF)
      continue;

    /* 16.2. (5) */
    orta nf = {
      .type = RTS_OSPF_IA,
      .options = options,
      .metric1 = abr->n.metric1 + metric,
      .metric2 = LSINFINITY,
      .tag = 0,
      .rid = en->lsa.rt, /* ABR ID */
      .oa = oa,
      .nhs = abr->n.nhs
    };

    if (type == ORT_NET)
      ri_install_net(p, ip, pxlen, &nf);
    else
      ri_install_rt(oa, dst_rid, &nf);
  }
}

/* RFC 2328 16.3. examining summary-LSAs in transit areas */
static void
ospf_rt_sum_tr(struct ospf_area *oa)
{
  struct ospf_proto *p = oa->po;
  struct ospf_area *bb = p->backbone;
  struct top_hash_entry *en;
  ort *re, *abr;
  ip_addr ip, abrip;
  u32 dst_rid, metric, options;
  int pxlen;
  u8 pxopts;


  if (!bb)
    return;

  WALK_SLIST(en, p->lsal)
  {
    if ((en->lsa_type != LSA_T_SUM_RT) && (en->lsa_type != LSA_T_SUM_NET))
      continue;

    if (en->domain != oa->areaid)
      continue;

    /* 16.3 (1a) */
    if (en->lsa.age == LSA_MAXAGE)
      continue;

    /* 16.3 (2) */
    if (en->lsa.rt == p->router_id)
      continue;

    if (en->lsa_type == LSA_T_SUM_NET)
    {
      lsa_parse_sum_net(en, ospf_is_v2(p), &ip, &pxlen, &pxopts, &metric);

      if (pxopts & OPT_PX_NU)
	continue;

      if (pxlen < 0 || pxlen > MAX_PREFIX_LENGTH)
      {
	log(L_WARN "%s: Invalid prefix in LSA (Type: %04x, Id: %R, Rt: %R)",
	    p->p.name, en->lsa_type, en->lsa.id, en->lsa.rt);
	continue;
      }

      re = fib_find(&p->rtf, &ip, pxlen);
    }
    else // en->lsa_type == LSA_T_SUM_RT
    {
      lsa_parse_sum_rt(en, ospf_is_v2(p), &dst_rid, &metric, &options);

      ip = ipa_from_rid(dst_rid);
      re = fib_find(&bb->rtr, &ip, MAX_PREFIX_LENGTH);
    }

    /* 16.3 (1b) */
    if (metric == LSINFINITY)
      continue;

    /* 16.3 (3) */
    if (!re || !re->n.type)
      continue;

    if (re->n.oa->areaid != 0)
      continue;

    if ((re->n.type != RTS_OSPF) && (re->n.type != RTS_OSPF_IA))
      continue;

    /* 16.3. (4) */
    abrip = ipa_from_rid(en->lsa.rt);
    abr = fib_find(&oa->rtr, &abrip, MAX_PREFIX_LENGTH);
    if (!abr || !abr->n.type)
      continue;

    metric = abr->n.metric1 + metric; /* IAC */

    /* 16.3. (5) */
    if ((metric < re->n.metric1) ||
	((metric == re->n.metric1) && unresolved_vlink(re)))
    {
      /* We want to replace the next-hop even if the metric is equal
	 to replace a virtual next-hop through vlink with a real one.
	 Proper ECMP would merge nexthops here, but we do not do that.
	 We restrict nexthops to fit one area to simplify check
	 12.4.3 p4 in decide_sum_lsa() */

      re->n.metric1 = metric;
      re->n.voa = oa;
      re->n.nhs = abr->n.nhs;
    }
  }
}

/* Decide about originating or flushing summary LSAs for condensed area networks */
static int
decide_anet_lsa(struct ospf_area *oa, struct area_net *anet, struct ospf_area *anet_oa)
{
  /* 12.4.3.1. - for stub/NSSA areas, originating summary routes is configurable */
  if (!oa_is_ext(oa) && !oa->ac->summary)
    return 0;

  if (oa == anet_oa)
    return 0;

  /* Do not condense routing info when exporting from backbone to the transit area */
  if ((anet_oa == oa->po->backbone) && oa->trcap)
    return 0;

  return (anet->active && !anet->hidden);
}

/* Decide about originating or flushing summary LSAs (12.4.3) */
static int
decide_sum_lsa(struct ospf_area *oa, ort *nf, int dest)
{
  /* 12.4.3.1. - for stub/NSSA areas, originating summary routes is configurable */
  if (!oa_is_ext(oa) && !oa->ac->summary)
    return 0;

  /* Invalid field - no route */
  if (!nf->n.type)
    return 0;

  /* 12.4.3 p2 */
  if (nf->n.type > RTS_OSPF_IA)
    return 0;

  /* 12.4.3 p3 */
  if ((nf->n.oa->areaid == oa->areaid))
    return 0;

  /* 12.4.3 p4 */
  if (nf->n.voa && (nf->n.voa->areaid == oa->areaid))
    return 0;

  /* 12.4.3 p5 */
  if (nf->n.metric1 >= LSINFINITY)
    return 0;

  /* 12.4.3 p6 - AS boundary router */
  if (dest == ORT_ROUTER)
  {
    /* We call decide_sum_lsa() on preferred ASBR entries, no need for 16.4. (3) */
    /* 12.4.3 p1 */
    return oa_is_ext(oa) && (nf->n.options & ORTA_ASBR);
  }

  /* 12.4.3 p7 - inter-area route */
  if (nf->n.type == RTS_OSPF_IA)
  {
    /* Inter-area routes are not repropagated into the backbone */
    return (oa != oa->po->backbone);
  }

  /* 12.4.3 p8 - intra-area route */

  /* Do not condense routing info when exporting from backbone to the transit area */
  if ((nf->n.oa == oa->po->backbone) && oa->trcap)
    return 1;

  struct area_net *anet = (struct area_net *)
    fib_route(&nf->n.oa->net_fib, nf->fn.prefix, nf->fn.pxlen);

  /* Condensed area network found */
  if (anet)
    return 0;

  return 1;
}

/* RFC 2328 16.7. p1 - originate or flush summary LSAs */
static inline void
check_sum_net_lsa(struct ospf_proto *p, ort *nf)
{
  struct area_net *anet = NULL;
  struct ospf_area *anet_oa = NULL;

  if (nf->area_net)
  {
    /* It is a default route for stub areas, handled entirely in ospf_rt_abr() */
    if (nf->fn.pxlen == 0)
      return;

    /* Find that area network */
    WALK_LIST(anet_oa, p->area_list)
    {
      anet = (struct area_net *) fib_find(&anet_oa->net_fib, &nf->fn.prefix, nf->fn.pxlen);
      if (anet)
	break;
    }
  }

  struct ospf_area *oa;
  WALK_LIST(oa, p->area_list)
  {
    if (anet && decide_anet_lsa(oa, anet, anet_oa))
      ospf_originate_sum_net_lsa(p, oa, nf, anet->metric);
    else if (decide_sum_lsa(oa, nf, ORT_NET))
      ospf_originate_sum_net_lsa(p, oa, nf, nf->n.metric1);
  }
}

static inline void
check_sum_rt_lsa(struct ospf_proto *p, ort *nf)
{
  struct ospf_area *oa;
  WALK_LIST(oa, p->area_list)
    if (decide_sum_lsa(oa, nf, ORT_ROUTER))
      ospf_originate_sum_rt_lsa(p, oa, nf, nf->n.metric1, nf->n.options);
}

static inline int
decide_nssa_lsa(struct ospf_proto *p UNUSED4 UNUSED6, ort *nf, struct ospf_lsa_ext_local *rt)
{
  struct ospf_area *oa = nf->n.oa;
  struct top_hash_entry *en = nf->n.en;

  if (!rt_is_nssa(nf) || !oa->translate)
    return 0;

  /* Condensed area network found */
  if (fib_route(&oa->enet_fib, nf->fn.prefix, nf->fn.pxlen))
    return 0;

  if (!en || (en->lsa_type != LSA_T_NSSA))
    return 0;

  /* We do not store needed data in struct orta, we have to parse the LSA */
  lsa_parse_ext(en, ospf_is_v2(p), rt);

  if (rt->pxopts & OPT_PX_NU)
    return 0;

  if (!rt->propagate || ipa_zero(rt->fwaddr))
    return 0;

  return 1;
}

/* RFC 3103 3.2 - translating Type-7 LSAs into Type-5 LSAs */
static inline void
check_nssa_lsa(struct ospf_proto *p, ort *nf)
{
  struct area_net *anet = NULL;
  struct ospf_area *oa = NULL;
  struct ospf_lsa_ext_local rt;

  /* Do not translate LSA if there is already the external LSA from route export */
  if (nf->external_rte)
    return;

  if (nf->area_net)
  {
    /* Find that area network */
    WALK_LIST(oa, p->area_list)
    {
      anet = (struct area_net *) fib_find(&oa->enet_fib, &nf->fn.prefix, nf->fn.pxlen);
      if (anet)
	break;
    }
  }

  /* RFC 3103 3.2 (3) - originate the aggregated address range */
  if (anet && anet->active && !anet->hidden && oa->translate)
    ospf_originate_ext_lsa(p, NULL, nf, LSA_M_RTCALC, anet->metric,
			   (anet->metric & LSA_EXT3_EBIT), IPA_NONE, anet->tag, 0);

  /* RFC 3103 3.2 (2) - originate the same network */
  else if (decide_nssa_lsa(p, nf, &rt))
    ospf_originate_ext_lsa(p, NULL, nf, LSA_M_RTCALC, rt.metric, rt.ebit, rt.fwaddr, rt.tag, 0);
}

/* RFC 2328 16.7. p2 - find new/lost vlink endpoints */
static void
ospf_check_vlinks(struct ospf_proto *p)
{
  struct ospf_iface *ifa;
  WALK_LIST(ifa, p->iface_list)
  {
    if (ifa->type == OSPF_IT_VLINK)
    {
      struct top_hash_entry *tmp;
      tmp = ospf_hash_find_rt(p->gr, ifa->voa->areaid, ifa->vid);

      if (tmp && (tmp->color == INSPF) && ipa_nonzero(tmp->lb) && tmp->nhs)
      {
	struct ospf_iface *nhi = ospf_iface_find(p, tmp->nhs->iface);

	if ((ifa->state != OSPF_IS_PTP)
	    || (ifa->vifa != nhi)
	    || !ipa_equal(ifa->vip, tmp->lb))
	{
	  OSPF_TRACE(D_EVENTS, "Vlink peer %R found", ifa->vid);
	  ospf_iface_sm(ifa, ISM_DOWN);
	  ifa->vifa = nhi;
	  ifa->addr = nhi->addr;
	  ifa->cost = tmp->dist;
	  ifa->vip = tmp->lb;
	  ospf_iface_sm(ifa, ISM_UP);
	}
	else if ((ifa->state == OSPF_IS_PTP) && (ifa->cost != tmp->dist))
	{
	  ifa->cost = tmp->dist;

	  /* RFC 2328 12.4 Event 8 - vlink state change */
	  ospf_notify_rt_lsa(ifa->oa);
	}
      }
      else
      {
	if (ifa->state > OSPF_IS_DOWN)
	{
	  OSPF_TRACE(D_EVENTS, "Vlink peer %R lost", ifa->vid);
	  ospf_iface_sm(ifa, ISM_DOWN);
	}
      }
    }
  }
}


/* Miscellaneous route processing that needs to be done by ABRs */
static void
ospf_rt_abr1(struct ospf_proto *p)
{
  struct area_net *anet;
  ort *nf, *default_nf;

  /* RFC 2328 G.3 - incomplete resolution of virtual next hops - routers */
  FIB_WALK(&p->backbone->rtr, nftmp)
  {
    nf = (ort *) nftmp;

    if (nf->n.type && unresolved_vlink(nf))
      reset_ri(nf);
  }
  FIB_WALK_END;


  FIB_WALK(&p->rtf, nftmp)
  {
    nf = (ort *) nftmp;


    /* RFC 2328 G.3 - incomplete resolution of virtual next hops - networks */
    if (nf->n.type && unresolved_vlink(nf))
      reset_ri(nf);


    /* Compute condensed area networks */
    if (nf->n.type == RTS_OSPF)
    {
      anet = (struct area_net *) fib_route(&nf->n.oa->net_fib, nf->fn.prefix, nf->fn.pxlen);
      if (anet)
      {
	if (!anet->active)
	{
	  anet->active = 1;

	  /* Get a RT entry and mark it to know that it is an area network */
	  ort *nfi = (ort *) fib_get(&p->rtf, &anet->fn.prefix, anet->fn.pxlen);
	  nfi->area_net = 1;

	  /* 16.2. (3) */
	  if (nfi->n.type == RTS_OSPF_IA)
	    reset_ri(nfi);
	}

	if (anet->metric < nf->n.metric1)
	  anet->metric = nf->n.metric1;
      }
    }
  }
  FIB_WALK_END;

  ip_addr addr = IPA_NONE;
  default_nf = (ort *) fib_get(&p->rtf, &addr, 0);
  default_nf->area_net = 1;

  struct ospf_area *oa;
  WALK_LIST(oa, p->area_list)
  {

    /* 12.4.3.1. - originate or flush default route for stub/NSSA areas */
    if (oa_is_stub(oa) || (oa_is_nssa(oa) && !oa->ac->summary))
      ospf_originate_sum_net_lsa(p, oa, default_nf, oa->ac->default_cost);

    /*
     * Originate type-7 default route for NSSA areas
     *
     * Because type-7 default LSAs are originated by ABRs, they do not
     * collide with other type-7 LSAs (as ABRs generate type-5 LSAs
     * for both external route export or external-NSSA translation),
     * so we use 0 for the src arg.
     */

    if (oa_is_nssa(oa) && oa->ac->default_nssa)
      ospf_originate_ext_lsa(p, oa, default_nf, LSA_M_RTCALC, oa->ac->default_cost,
			     (oa->ac->default_cost & LSA_EXT3_EBIT), IPA_NONE, 0, 0);

    /* RFC 2328 16.4. (3) - precompute preferred ASBR entries */
    if (oa_is_ext(oa))
    {
      FIB_WALK(&oa->rtr, nftmp)
      {
	nf = (ort *) nftmp;
	if (nf->n.options & ORTA_ASBR)
	  ri_install_asbr(p, &nf->fn.prefix, &nf->n);
      }
      FIB_WALK_END;
    }
  }


  /* Originate or flush ASBR summary LSAs */
  FIB_WALK(&p->backbone->rtr, nftmp)
  {
    check_sum_rt_lsa(p, (ort *) nftmp);
  }
  FIB_WALK_END;


  /* RFC 2328 16.7. p2 - find new/lost vlink endpoints */
  ospf_check_vlinks(p);
}


static void
translator_timer_hook(timer *timer)
{
  struct ospf_area *oa = timer->data;

  if (oa->translate != TRANS_WAIT)
    return;

  oa->translate = TRANS_OFF;
  ospf_schedule_rtcalc(oa->po);
}

static void
ospf_rt_abr2(struct ospf_proto *p)
{
  struct ospf_area *oa;
  struct top_hash_entry *en;
  ort *nf, *nf2;


  /* RFC 3103 3.1 - type-7 translator election */
  struct ospf_area *bb = p->backbone;
  WALK_LIST(oa, p->area_list)
    if (oa_is_nssa(oa))
    {
      int translate = 1;

      if (oa->ac->translator)
	goto decided;

      FIB_WALK(&oa->rtr, nftmp)
      {
	nf = (ort *) nftmp;
	if (!nf->n.type || !(nf->n.options & ORTA_ABR))
	  continue;

	nf2 = fib_find(&bb->rtr, &nf->fn.prefix, MAX_PREFIX_LENGTH);
	if (!nf2 || !nf2->n.type || !(nf2->n.options & ORTA_ABR))
	  continue;

	en = ospf_hash_find_rt(p->gr, oa->areaid, nf->n.rid);
	if (!en || (en->color != INSPF))
	  continue;

	struct ospf_lsa_rt *rt = en->lsa_body;
	/* There is better candidate - Nt-bit or higher Router ID */
	if ((rt->options & OPT_RT_NT) || (p->router_id < nf->n.rid))
	{
	  translate = 0;
	  goto decided;
	}
      }
      FIB_WALK_END;

    decided:
      if (translate && (oa->translate != TRANS_ON))
      {
	if (oa->translate == TRANS_WAIT)
	  tm_stop(oa->translator_timer);

	oa->translate = TRANS_ON;
      }

      if (!translate && (oa->translate == TRANS_ON))
      {
	if (oa->translator_timer == NULL)
	  oa->translator_timer = tm_new_set(p->p.pool, translator_timer_hook, oa, 0, 0);

	/* Schedule the end of translation */
	tm_start(oa->translator_timer, oa->ac->transint);
	oa->translate = TRANS_WAIT;
      }
    }


  /* Compute condensed external networks */
  FIB_WALK(&p->rtf, nftmp)
  {
    nf = (ort *) nftmp;
    if (rt_is_nssa(nf) && (nf->n.options & ORTA_PROP))
    {
      struct area_net *anet = (struct area_net *)
	fib_route(&nf->n.oa->enet_fib, nf->fn.prefix, nf->fn.pxlen);

      if (anet)
      {
	if (!anet->active)
	{
	  anet->active = 1;

	  /* Get a RT entry and mark it to know that it is an area network */
	  nf2 = (ort *) fib_get(&p->rtf, &anet->fn.prefix, anet->fn.pxlen);
	  nf2->area_net = 1;
	}

	u32 metric = (nf->n.type == RTS_OSPF_EXT1) ?
	  nf->n.metric1 : ((nf->n.metric2 + 1) | LSA_EXT3_EBIT);

	if (anet->metric < metric)
	  anet->metric = metric;
      }
    }
  }
  FIB_WALK_END;


  FIB_WALK(&p->rtf, nftmp)
  {
    nf = (ort *) nftmp;

    check_sum_net_lsa(p, nf);
    check_nssa_lsa(p, nf);
  }
  FIB_WALK_END;
}


/* Like fib_route(), but ignores dummy rt entries */
static void *
ospf_fib_route(struct fib *f, ip_addr a, int len)
{
  ip_addr a0;
  ort *nf;

  while (len >= 0)
  {
    a0 = ipa_and(a, ipa_mkmask(len));
    nf = fib_find(f, &a0, len);
    if (nf && nf->n.type)
      return nf;
    len--;
  }
  return NULL;
}

/* RFC 2328 16.4. calculating external routes */
static void
ospf_ext_spf(struct ospf_proto *p)
{
  struct top_hash_entry *en;
  struct ospf_lsa_ext_local rt;
  ort *nf1, *nf2;
  ip_addr rtid;
  u32 br_metric;
  struct ospf_area *atmp;

  OSPF_TRACE(D_EVENTS, "Starting routing table calculation for ext routes");

  WALK_SLIST(en, p->lsal)
  {
    orta nfa = {};

    /* 16.4. (1) */
    if ((en->lsa_type != LSA_T_EXT) && (en->lsa_type != LSA_T_NSSA))
      continue;

    if (en->lsa.age == LSA_MAXAGE)
      continue;

    /* 16.4. (2) */
    if (en->lsa.rt == p->router_id)
      continue;

    DBG("%s: Working on LSA. ID: %R, RT: %R, Type: %u\n",
	p->p.name, en->lsa.id, en->lsa.rt, en->lsa_type);

    lsa_parse_ext(en, ospf_is_v2(p), &rt);

    if (rt.metric == LSINFINITY)
      continue;

    if (rt.pxopts & OPT_PX_NU)
      continue;

    if (rt.pxlen < 0 || rt.pxlen > MAX_PREFIX_LENGTH)
    {
      log(L_WARN "%s: Invalid prefix in LSA (Type: %04x, Id: %R, Rt: %R)",
	  p->p.name, en->lsa_type, en->lsa.id, en->lsa.rt);
      continue;
    }


    /* 16.4. (3) */
    /* If there are more areas, we already precomputed preferred ASBR
       entries in ospf_rt_abr1() and stored them in the backbone
       table. For NSSA, we examine the area to which the LSA is assigned */
    if (en->lsa_type == LSA_T_EXT)
      atmp = ospf_main_area(p);
    else /* NSSA */
      atmp = ospf_find_area(p, en->domain);

    if (!atmp)
      continue;			/* Should not happen */

    rtid = ipa_from_rid(en->lsa.rt);
    nf1 = fib_find(&atmp->rtr, &rtid, MAX_PREFIX_LENGTH);

    if (!nf1 || !nf1->n.type)
      continue;			/* No AS boundary router found */

    if (!(nf1->n.options & ORTA_ASBR))
      continue;			/* It is not ASBR */

    /* 16.4. (3) NSSA - special rule for default routes */
    /* ABR should use default only if P-bit is set and summaries are active */
    if ((en->lsa_type == LSA_T_NSSA) && ipa_zero(rt.ip) && (rt.pxlen == 0) &&
	(p->areano > 1) && !(rt.propagate && atmp->ac->summary))
      continue;

    if (!rt.fbit)
    {
      nf2 = nf1;
      nfa.nhs = nf1->n.nhs;
      br_metric = nf1->n.metric1;
    }
    else
    {
      nf2 = ospf_fib_route(&p->rtf, rt.fwaddr, MAX_PREFIX_LENGTH);
      if (!nf2)
	continue;

      if (en->lsa_type == LSA_T_EXT)
      {
	/* For ext routes, we accept intra-area or inter-area routes */
	if ((nf2->n.type != RTS_OSPF) && (nf2->n.type != RTS_OSPF_IA))
	  continue;
      }
      else /* NSSA */
      {
	/* For NSSA routes, we accept just intra-area in the same area */
	if ((nf2->n.type != RTS_OSPF) || (nf2->n.oa != atmp))
	  continue;
      }

      /* Next-hop is a part of a configured stubnet */
      if (!nf2->n.nhs)
	continue;

      nfa.nhs = nf2->n.nhs;
      br_metric = nf2->n.metric1;

      /* Replace device nexthops with nexthops to forwarding address from LSA */
      if (has_device_nexthops(nfa.nhs))
      {
	nfa.nhs = fix_device_nexthops(p, nfa.nhs, rt.fwaddr);
	nfa.nhs_reuse = 1;
      }
    }

    if (rt.ebit)
    {
      nfa.type = RTS_OSPF_EXT2;
      nfa.metric1 = br_metric;
      nfa.metric2 = rt.metric;
    }
    else
    {
      nfa.type = RTS_OSPF_EXT1;
      nfa.metric1 = br_metric + rt.metric;
      nfa.metric2 = LSINFINITY;
    }

    /* Mark the LSA as reachable */
    en->color = INSPF;

    /* Whether the route is preferred in route selection according to 16.4.1 */
    nfa.options = epath_preferred(&nf2->n) ? ORTA_PREF : 0;
    if (en->lsa_type == LSA_T_NSSA)
    {
      nfa.options |= ORTA_NSSA;
      if (rt.propagate)
	nfa.options |= ORTA_PROP;
    }

    nfa.tag = rt.tag;
    nfa.rid = en->lsa.rt;
    nfa.oa = atmp; /* undefined in RFC 2328 */
    nfa.en = en; /* store LSA for later (NSSA processing) */

    ri_install_ext(p, rt.ip, rt.pxlen, &nfa);
  }
}

/* Cleanup of routing tables and data */
void
ospf_rt_reset(struct ospf_proto *p)
{
  struct ospf_area *oa;
  struct top_hash_entry *en;
  struct area_net *anet;
  ort *ri;

  /* Reset old routing table */
  FIB_WALK(&p->rtf, nftmp)
  {
    ri = (ort *) nftmp;
    ri->area_net = 0;
    ri->keep = 0;
    reset_ri(ri);
  }
  FIB_WALK_END;

  /* Reset SPF data in LSA db */
  WALK_SLIST(en, p->lsal)
  {
    en->color = OUTSPF;
    en->dist = LSINFINITY;
    en->nhs = NULL;
    en->lb = IPA_NONE;

    if (en->mode == LSA_M_RTCALC)
      en->mode = LSA_M_STALE;
  }

  WALK_LIST(oa, p->area_list)
  {
    /* Reset ASBR routing tables */
    FIB_WALK(&oa->rtr, nftmp)
    {
      ri = (ort *) nftmp;
      reset_ri(ri);
    }
    FIB_WALK_END;

    /* Reset condensed area networks */
    if (p->areano > 1)
    {
      FIB_WALK(&oa->net_fib, nftmp)
      {
	anet = (struct area_net *) nftmp;
	anet->active = 0;
	anet->metric = 0;
      }
      FIB_WALK_END;

      FIB_WALK(&oa->enet_fib, nftmp)
      {
	anet = (struct area_net *) nftmp;
	anet->active = 0;
	anet->metric = 0;
      }
      FIB_WALK_END;
    }
  }
}

/**
 * ospf_rt_spf - calculate internal routes
 * @p: OSPF protocol instance
 *
 * Calculation of internal paths in an area is described in 16.1 of RFC 2328.
 * It's based on Dijkstra's shortest path tree algorithms.
 * This function is invoked from ospf_disp().
 */
void
ospf_rt_spf(struct ospf_proto *p)
{
  struct ospf_area *oa;

  if (p->areano == 0)
    return;

  OSPF_TRACE(D_EVENTS, "Starting routing table calculation");

  /* 16. (1) */
  ospf_rt_reset(p);

  /* 16. (2) */
  WALK_LIST(oa, p->area_list)
    ospf_rt_spfa(oa);

  /* 16. (3) */
  ospf_rt_sum(ospf_main_area(p));

  /* 16. (4) */
  WALK_LIST(oa, p->area_list)
    if (oa->trcap && (oa->areaid != 0))
      ospf_rt_sum_tr(oa);

  if (p->areano > 1)
    ospf_rt_abr1(p);

  /* 16. (5) */
  ospf_ext_spf(p);

  if (p->areano > 1)
    ospf_rt_abr2(p);

  rt_sync(p);
  lp_flush(p->nhpool);

  p->calcrt = 0;
}


static inline int
inherit_nexthops(struct mpnh *pn)
{
  /* Proper nexthops (with defined GW) or dummy vlink nexthops (without iface) */
  return pn && (ipa_nonzero(pn->gw) || !pn->iface);
}

static struct mpnh *
calc_next_hop(struct ospf_area *oa, struct top_hash_entry *en,
	      struct top_hash_entry *par, int pos)
{
  struct ospf_proto *p = oa->po;
  struct mpnh *pn = par->nhs;
  struct ospf_iface *ifa;
  u32 rid = en->lsa.rt;

  /* 16.1.1. The next hop calculation */
  DBG("     Next hop calculating for id: %R rt: %R type: %u\n",
      en->lsa.id, en->lsa.rt, en->lsa_type);

  /* Usually, we inherit parent nexthops */
  if (inherit_nexthops(pn))
    return pn;

  /*
   * There are three cases:
   * 1) en is a local network (and par is root)
   * 2) en is a ptp or ptmp neighbor (and par is root)
   * 3) en is a bcast or nbma neighbor (and par is local network)
   */

  /* The first case - local network */
  if ((en->lsa_type == LSA_T_NET) && (par == oa->rt))
  {
    ifa = rt_pos_to_ifa(oa, pos);
    if (!ifa)
      return NULL;

    return new_nexthop(p, IPA_NONE, ifa->iface, ifa->ecmp_weight);
  }

  /* The second case - ptp or ptmp neighbor */
  if ((en->lsa_type == LSA_T_RT) && (par == oa->rt))
  {
    ifa = rt_pos_to_ifa(oa, pos);
    if (!ifa)
      return NULL;

    if (ifa->type == OSPF_IT_VLINK)
      return new_nexthop(p, IPA_NONE, NULL, 0);

    struct ospf_neighbor *m = find_neigh(ifa, rid);
    if (!m || (m->state != NEIGHBOR_FULL))
      return NULL;

    return new_nexthop(p, m->ip, ifa->iface, ifa->ecmp_weight);
  }

  /* The third case - bcast or nbma neighbor */
  if ((en->lsa_type == LSA_T_RT) && (par->lsa_type == LSA_T_NET))
  {
    /* par->nhi should be defined from parent's calc_next_hop() */
    if (!pn)
      goto bad;

    if (ospf_is_v2(p))
    {
      /*
       * In this case, next-hop is the same as link-back, which is
       * already computed in link_back().
       */
      if (ipa_zero(en->lb))
	goto bad;

      return new_nexthop(p, en->lb, pn->iface, pn->weight);
    }
    else /* OSPFv3 */
    {
      /*
       * Next-hop is taken from lladdr field of Link-LSA, en->lb_id
       * is computed in link_back().
       */
      struct top_hash_entry *lhe;
      lhe = ospf_hash_find(p->gr, pn->iface->index, en->lb_id, rid, LSA_T_LINK);

      if (!lhe)
	return NULL;

      struct ospf_lsa_link *llsa = lhe->lsa_body;

      if (ip6_zero(llsa->lladdr))
	return NULL;

      return new_nexthop(p, ipa_from_ip6(llsa->lladdr), pn->iface, pn->weight);
    }
  }

 bad:
  /* Probably bug or some race condition, we log it */
  log(L_ERR "%s: Unexpected case in next hop calculation", p->p.name);
  return NULL;
}


/* Add LSA into list of candidates in Dijkstra's algorithm */
static void
add_cand(list * l, struct top_hash_entry *en, struct top_hash_entry *par,
	 u32 dist, struct ospf_area *oa, int pos)
{
  struct ospf_proto *p = oa->po;
  node *prev, *n;
  int added = 0;
  struct top_hash_entry *act;

  /* 16.1. (2b) */
  if (en == NULL)
    return;
  if (en->lsa.age == LSA_MAXAGE)
    return;

  if (ospf_is_v3(p) && (en->lsa_type == LSA_T_RT))
  {
    /* In OSPFv3, check V6 flag */
    struct ospf_lsa_rt *rt = en->lsa_body;
    if (!(rt->options & OPT_V6))
      return;
  }

  /* 16.1. (2c) */
  if (en->color == INSPF)
    return;

  /* 16.1. (2d), also checks that dist < LSINFINITY */
  if (dist > en->dist)
    return;

  /* We should check whether there is a reverse link from en to par, */
  if (!link_back(oa, en, par))
    return;

  struct mpnh *nhs = calc_next_hop(oa, en, par, pos);
  if (!nhs)
  {
    log(L_WARN "%s: Cannot find next hop for LSA (Type: %04x, Id: %R, Rt: %R)",
	p->p.name, en->lsa_type, en->lsa.id, en->lsa.rt);
    return;
  }

  /* If en->dist > 0, we know that en->color == CANDIDATE and en->nhs is defined. */
  if ((dist == en->dist) && !nh_is_vlink(en->nhs))
  {
    /*
     * For multipath, we should merge nexthops. We merge regular nexthops only.
     * Dummy vlink nexthops are less preferred and handled as a special case.
     *
     * During merging, new nexthops (nhs) can be reused if they are not
     * inherited from the parent (i.e. they are allocated in calc_next_hop()).
     * Current nexthops (en->nhs) can be reused if they weren't inherited in
     * previous steps (that is stored in nhs_reuse, i.e. created by merging or
     * allocated in calc_next_hop()).
     *
     * Generally, a node first inherits shared nexthops from its parent and
     * later possibly gets reusable (private) copy during merging. This is more
     * or less same for both top_hash_entry nodes and orta nodes.
     *
     * Note that when a child inherits a private nexthop from its parent, it
     * should make the nexthop shared for both parent and child, while we only
     * update nhs_reuse for the child node. This makes nhs_reuse field for the
     * parent technically incorrect, but it is not a problem as parent's nhs
     * will not be modified (and nhs_reuse examined) afterwards.
     */

    /* Keep old ones */
    if (!p->ecmp || nh_is_vlink(nhs) || (nhs == en->nhs))
      return;

    /* Merge old and new */
    int new_reuse = (par->nhs != nhs);
    en->nhs = mpnh_merge(en->nhs, nhs, en->nhs_reuse, new_reuse, p->ecmp, p->nhpool);
    en->nhs_reuse = 1;
    return;
  }

  DBG("     Adding candidate: rt: %R, id: %R, type: %u\n",
      en->lsa.rt, en->lsa.id, en->lsa_type);

  if (en->color == CANDIDATE)
  {				/* We found a shorter path */
    rem_node(&en->cn);
  }
  en->nhs = nhs;
  en->dist = dist;
  en->color = CANDIDATE;
  en->nhs_reuse = (par->nhs != nhs);

  prev = NULL;

  if (EMPTY_LIST(*l))
  {
    add_head(l, &en->cn);
  }
  else
  {
    WALK_LIST(n, *l)
    {
      act = SKIP_BACK(struct top_hash_entry, cn, n);
      if ((act->dist > dist) ||
	  ((act->dist == dist) && (act->lsa_type == LSA_T_RT)))
      {
	if (prev == NULL)
	  add_head(l, &en->cn);
	else
	  insert_node(&en->cn, prev);
	added = 1;
	break;
      }
      prev = n;
    }

    if (!added)
    {
      add_tail(l, &en->cn);
    }
  }
}

static inline int
ort_changed(ort *nf, rta *nr)
{
  rta *or = nf->old_rta;
  return !or ||
    (nf->n.metric1 != nf->old_metric1) || (nf->n.metric2 != nf->old_metric2) ||
    (nf->n.tag != nf->old_tag) || (nf->n.rid != nf->old_rid) ||
    (nr->source != or->source) || (nr->dest != or->dest) ||
    (nr->iface != or->iface) || !ipa_equal(nr->gw, or->gw) ||
    !mpnh_same(nr->nexthops, or->nexthops);
}

static void
rt_sync(struct ospf_proto *p)
{
  struct top_hash_entry *en;
  struct fib_iterator fit;
  struct fib *fib = &p->rtf;
  ort *nf;
  struct ospf_area *oa;

  /* This is used for forced reload of routes */
  int reload = (p->calcrt == 2);

  OSPF_TRACE(D_EVENTS, "Starting routing table synchronisation");

  DBG("Now syncing my rt table with nest's\n");
  FIB_ITERATE_INIT(&fit, fib);
again1:
  FIB_ITERATE_START(fib, &fit, nftmp)
  {
    nf = (ort *) nftmp;

    /* Sanity check of next-hop addresses, failure should not happen */
    if (nf->n.type)
    {
      struct mpnh *nh;
      for (nh = nf->n.nhs; nh; nh = nh->next)
	if (ipa_nonzero(nh->gw))
	{
	  neighbor *ng = neigh_find2(&p->p, &nh->gw, nh->iface, 0);
	  if (!ng || (ng->scope == SCOPE_HOST))
	    { reset_ri(nf); break; }
	}
    }

    /* Remove configured stubnets but keep the entries */
    if (nf->n.type && !nf->n.nhs)
    {
      reset_ri(nf);
      nf->keep = 1;
    }

    if (nf->n.type) /* Add the route */
    {
      rta a0 = {
	.src = p->p.main_source,
	.source = nf->n.type,
	.scope = SCOPE_UNIVERSE,
	.cast = RTC_UNICAST
      };

      if (nf->n.nhs->next)
      {
	a0.dest = RTD_MULTIPATH;
	a0.nexthops = nf->n.nhs;
      }
      else if (ipa_nonzero(nf->n.nhs->gw))
      {
	a0.dest = RTD_ROUTER;
	a0.iface = nf->n.nhs->iface;
	a0.gw = nf->n.nhs->gw;
      }
      else
      {
	a0.dest = RTD_DEVICE;
	a0.iface = nf->n.nhs->iface;
      }

      if (reload || ort_changed(nf, &a0))
      {
	net *ne = net_get(p->p.table, nf->fn.prefix, nf->fn.pxlen);
	rta *a = rta_lookup(&a0);
	rte *e = rte_get_temp(a);

	rta_free(nf->old_rta);
	nf->old_rta = rta_clone(a);
	e->u.ospf.metric1 = nf->old_metric1 = nf->n.metric1;
	e->u.ospf.metric2 = nf->old_metric2 = nf->n.metric2;
	e->u.ospf.tag = nf->old_tag = nf->n.tag;
	e->u.ospf.router_id = nf->old_rid = nf->n.rid;
	e->pflags = 0;
	e->net = ne;
	e->pref = p->p.preference;

	DBG("Mod rte type %d - %I/%d via %I on iface %s, met %d\n",
	    a0.source, nf->fn.prefix, nf->fn.pxlen, a0.gw, a0.iface ? a0.iface->name : "(none)", nf->n.metric1);
	rte_update(&p->p, ne, e);
      }
    }
    else if (nf->old_rta)
    {
      /* Remove the route */
      rta_free(nf->old_rta);
      nf->old_rta = NULL;

      net *ne = net_get(p->p.table, nf->fn.prefix, nf->fn.pxlen);
      rte_update(&p->p, ne, NULL);
    }

    /* Remove unused rt entry, some special entries are persistent */
    if (!nf->n.type && !nf->external_rte && !nf->area_net && !nf->keep)
    {
      FIB_ITERATE_PUT(&fit, nftmp);
      fib_delete(fib, nftmp);
      goto again1;
    }
  }
  FIB_ITERATE_END(nftmp);


  WALK_LIST(oa, p->area_list)
  {
    /* Cleanup ASBR hash tables */
    FIB_ITERATE_INIT(&fit, &oa->rtr);
again2:
    FIB_ITERATE_START(&oa->rtr, &fit, nftmp)
    {
      nf = (ort *) nftmp;

      if (!nf->n.type)
      {
	FIB_ITERATE_PUT(&fit, nftmp);
	fib_delete(&oa->rtr, nftmp);
	goto again2;
      }
    }
    FIB_ITERATE_END(nftmp);
  }

  /* Cleanup stale LSAs */
  WALK_SLIST(en, p->lsal)
    if (en->mode == LSA_M_STALE)
      ospf_flush_lsa(p, en);
}
