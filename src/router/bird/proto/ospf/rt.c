/*
 * BIRD -- OSPF
 * 
 * (c) 2000--2004 Ondrej Filip <feela@network.cz>
 * 
 * Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"

static void add_cand(list * l, struct top_hash_entry *en, 
		     struct top_hash_entry *par, u32 dist,
		     struct ospf_area *oa, int i);
static void rt_sync(struct proto_ospf *po);

/* In ospf_area->rtr we store paths to routers, but we use RID (and not IP address)
   as index, so we need to encapsulate RID to IP address */
#ifdef OSPFv2
#define ipa_from_rid(x) _MI(x)
#else /* OSPFv3 */
#define ipa_from_rid(x) _MI(0,0,0,x)
#endif


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
  ri->fn.x0 = ri->fn.x1 = 0;
}

static inline int
unresolved_vlink(struct mpnh *nhs)
{
  return nhs && !nhs->iface;
}

static inline struct mpnh *
new_nexthop(struct proto_ospf *po, ip_addr gw, struct iface *iface, unsigned char weight)
{
  struct mpnh *nh = lp_alloc(po->nhpool, sizeof(struct mpnh));
  nh->gw = gw;
  nh->iface = iface;
  nh->next = NULL;
  nh->weight = weight;
  return nh;
}

static inline struct mpnh *
copy_nexthop(struct proto_ospf *po, struct mpnh *src)
{
  struct mpnh *nh = lp_alloc(po->nhpool, sizeof(struct mpnh));
  nh->gw = src->gw;
  nh->iface = src->iface;
  nh->next = NULL;
  nh->weight = src->weight;
  return nh;
}


/* If new is better return 1 */
static int
ri_better(struct proto_ospf *po, orta *new, orta *old)
{
  if (old->type == RTS_DUMMY)
    return 1;

  if (new->type < old->type)
    return 1;

  if (new->type > old->type)
    return 0;

  if (new->metric1 < old->metric1)
    return 1;

  if (new->metric1 > old->metric1)
    return 0;

  return 0;
}


/* Whether the ASBR or the forward address destination is preferred
   in AS external route selection according to 16.4.1. */
static inline int
epath_preferred(orta *ep)
{
  return (ep->type == RTS_OSPF) && (ep->oa->areaid != 0);
}

/* 16.4. (3), return 1 if new is better */
static int
ri_better_asbr(struct proto_ospf *po, orta *new, orta *old)
{
  if (old->type == RTS_DUMMY)
    return 1;

  if (!po->rfc1583)
  {
    int new_pref = epath_preferred(new);
    int old_pref = epath_preferred(old);

    if (new_pref > old_pref)
      return 1;

    if (new_pref < old_pref)
      return 0;
  }

  if (new->metric1 < old->metric1)
    return 1;

  if (new->metric1 > old->metric1)
    return 0;

  /* Larger area ID is preferred */
  if (new->oa->areaid > old->oa->areaid)
    return 1;

  return 0;
}

static int
orta_prio(orta *nf)
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

/* 16.4. (6), return 1 if new is better */
static int
ri_better_ext(struct proto_ospf *po, orta *new, orta *old)
{
  if (old->type == RTS_DUMMY)
    return 1;

  /* 16.4. (6a) */
  if (new->type < old->type)
    return 1;

  if (new->type > old->type)
    return 0;

  /* 16.4. (6b), same type */
  if (new->type == RTS_OSPF_EXT2)
  {
    if (new->metric2 < old->metric2)
      return 1;

    if (new->metric2 > old->metric2)
      return 0;
  }

  /* 16.4. (6c) */
  if (!po->rfc1583)
  {
    u32 new_pref = new->options & ORTA_PREF;
    u32 old_pref = old->options & ORTA_PREF;

    if (new_pref > old_pref)
      return 1;

    if (new_pref < old_pref)
      return 0;
  }

  /* 16.4. (6d) */
  if (new->metric1 < old->metric1)
    return 1;

  if (new->metric1 > old->metric1)
    return 0;

  /* RFC 3103, 2.5. (6e) */
  int new_prio = orta_prio(new);
  int old_prio = orta_prio(old);

  if (new_prio > old_prio)
    return 1;

  if (old_prio > new_prio)
    return 0;

  /* make it more deterministic */
  if (new->rid > old->rid)
    return 1;

  return 0;
}

static inline void
ri_install_net(struct proto_ospf *po, ip_addr prefix, int pxlen, orta *new)
{
  ort *old = (ort *) fib_get(&po->rtf, &prefix, pxlen);
  if (ri_better(po, new, &old->n))
    memcpy(&old->n, new, sizeof(orta));
}

static inline void
ri_install_rt(struct ospf_area *oa, u32 rid, orta *new)
{
  ip_addr addr = ipa_from_rid(rid);
  ort *old = (ort *) fib_get(&oa->rtr, &addr, MAX_PREFIX_LENGTH);
  if (ri_better(oa->po, new, &old->n))
    memcpy(&old->n, new, sizeof(orta));
}

static inline void
ri_install_asbr(struct proto_ospf *po, ip_addr *addr, orta *new)
{
  ort *old = (ort *) fib_get(&po->backbone->rtr, addr, MAX_PREFIX_LENGTH);
  if (ri_better_asbr(po, new, &old->n))
    memcpy(&old->n, new, sizeof(orta));
}

static inline void
ri_install_ext(struct proto_ospf *po, ip_addr prefix, int pxlen, orta *new)
{
  ort *old = (ort *) fib_get(&po->rtf, &prefix, pxlen);
  if (ri_better_ext(po, new, &old->n))
    memcpy(&old->n, new, sizeof(orta));
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

#ifdef OSPFv3
static inline struct ospf_iface *
px_pos_to_ifa(struct ospf_area *oa, int pos)
{
  struct ospf_iface *ifa;
  WALK_LIST(ifa, oa->po->iface_list)
    if (ifa->oa == oa && pos >= ifa->px_pos_beg && pos < ifa->px_pos_end)
      return ifa;
  return NULL;
}
#endif


static void
add_network(struct ospf_area *oa, ip_addr px, int pxlen, int metric, struct top_hash_entry *en, int pos)
{
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
	oa->po->proto.name, en->lsa.type, en->lsa.id, en->lsa.rt);
    return;
  }

  if (en == oa->rt)
  {
    /* 
     * Local stub networks does not have proper iface in en->nhi
     * (because they all have common top_hash_entry en).
     * We have to find iface responsible for that stub network.
     * Configured stubnets does not have any iface. They will
     * be removed in rt_sync().
     */

    struct ospf_iface *ifa;
#ifdef OSPFv2
    ifa = rt_pos_to_ifa(oa, pos);
#else /* OSPFv3 */
    ifa = px_pos_to_ifa(oa, pos);
#endif

    nf.nhs = ifa ? new_nexthop(oa->po, IPA_NONE, ifa->iface, ifa->ecmp_weight) : NULL;
  }

  ri_install_net(oa->po, px, pxlen, &nf);
}

#ifdef OSPFv3
static void
process_prefixes(struct ospf_area *oa)
{
  struct proto_ospf *po = oa->po;
  // struct proto *p = &po->proto;
  struct top_hash_entry *en, *src;
  struct ospf_lsa_prefix *px;
  ip_addr pxa;
  int pxlen;
  u8 pxopts;
  u16 metric;
  u32 *buf;
  int i;

  WALK_SLIST(en, po->lsal)
  {
    if (en->lsa.type != LSA_T_PREFIX)
      continue;

    if (en->domain != oa->areaid)
      continue;

    if (en->lsa.age == LSA_MAXAGE)
      continue;

    px = en->lsa_body;

    /* For router prefix-LSA, we would like to find the first router-LSA */
    if (px->ref_type == LSA_T_RT)
      src = ospf_hash_find_rt(po->gr, oa->areaid, px->ref_rt);
    else
      src = ospf_hash_find(po->gr, oa->areaid, px->ref_id, px->ref_rt, px->ref_type);

    if (!src)
      continue;

    /* Reachable in SPF */
    if (src->color != INSPF)
      continue;

    if ((src->lsa.type != LSA_T_RT) && (src->lsa.type != LSA_T_NET))
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
#endif


static void
ospf_rt_spfa_rtlinks(struct ospf_area *oa, struct top_hash_entry *act, struct top_hash_entry *en)
{
  // struct proto *p = &oa->po->proto;
  struct proto_ospf *po = oa->po;
  ip_addr prefix UNUSED;
  int pxlen UNUSED, i;

  struct ospf_lsa_rt *rt = en->lsa_body;
  struct ospf_lsa_rt_link *rr = (struct ospf_lsa_rt_link *) (rt + 1);

  for (i = 0; i < lsa_rt_count(&en->lsa); i++)
    {
      struct ospf_lsa_rt_link *rtl = rr + i;
      struct top_hash_entry *tmp = NULL;

      DBG("     Working on link: %R (type: %u)  ", rtl->id, rtl->type);
      switch (rtl->type)
	{
#ifdef OSPFv2
	case LSART_STUB:
	  /*
	   * RFC 2328 in 16.1. (2a) says to handle stub networks in an
	   * second phase after the SPF for an area is calculated. We get
	   * the same result by handing them here because add_network()
	   * will keep the best (not the first) found route.
	   */
	  prefix = ipa_from_u32(rtl->id & rtl->data);
	  pxlen = ipa_mklen(ipa_from_u32(rtl->data));
	  add_network(oa, prefix, pxlen, act->dist + rtl->metric, act, i);
	  break;
#endif

	case LSART_NET:
#ifdef OSPFv2
	  /* In OSPFv2, rtl->id is IP addres of DR, Router ID is not known */
	  tmp = ospf_hash_find_net(po->gr, oa->areaid, rtl->id);
#else /* OSPFv3 */
	  tmp = ospf_hash_find(po->gr, oa->areaid, rtl->nif, rtl->id, LSA_T_NET);
#endif
	  break;

	case LSART_VLNK:
	case LSART_PTP:
	  tmp = ospf_hash_find_rt(po->gr, oa->areaid, rtl->id);
	  break;

	default:
	  log("Unknown link type in router lsa. (rid = %R)", act->lsa.id);
	  break;
	}

      if (tmp)
	DBG("Going to add cand, Mydist: %u, Req: %u\n",
	    tmp->dist, act->dist + rtl->metric);
      add_cand(&oa->cand, tmp, act, act->dist + rtl->metric, oa, i);
    }
}

/* RFC 2328 16.1. calculating shortest paths for an area */
static void
ospf_rt_spfa(struct ospf_area *oa)
{
  struct proto *p = &oa->po->proto;
  struct proto_ospf *po = oa->po;
  struct ospf_lsa_rt *rt;
  struct ospf_lsa_net *ln;
  struct top_hash_entry *act, *tmp;
  ip_addr prefix UNUSED;
  int pxlen UNUSED;
  u32 i, *rts;
  node *n;

  if (oa->rt == NULL)
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
      oa->rt->lsa.rt, oa->rt->lsa.id, oa->rt->lsa.type);

  while (!EMPTY_LIST(oa->cand))
  {
    n = HEAD(oa->cand);
    act = SKIP_BACK(struct top_hash_entry, cn, n);
    rem_node(n);

    DBG("Working on LSA: rt: %R, id: %R, type: %u\n",
	act->lsa.rt, act->lsa.id, act->lsa.type);

    act->color = INSPF;
    switch (act->lsa.type)
    {
    case LSA_T_RT:
      rt = (struct ospf_lsa_rt *) act->lsa_body;
      if (rt->options & OPT_RT_V)
	oa->trcap = 1;

      /*
       * In OSPFv3, all routers are added to per-area routing
       * tables. But we use it just for ASBRs and ABRs. For the
       * purpose of the last step in SPF - prefix-LSA processing in
       * process_prefixes(), we use information stored in LSA db.
       */
      if (((rt->options & OPT_RT_E) || (rt->options & OPT_RT_B))
	  && (act->lsa.rt != po->router_id))
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

#ifdef OSPFv2
      ospf_rt_spfa_rtlinks(oa, act, act);
#else /* OSPFv3 */
      /* Errata 2078 to RFC 5340 4.8.1 - skip links from non-routing nodes */
      if ((act != oa->rt) && !(rt->options & OPT_R))
	break;

      for (tmp = ospf_hash_find_rt_first(po->gr, act->domain, act->lsa.rt);
	   tmp; tmp = ospf_hash_find_rt_next(tmp))
	ospf_rt_spfa_rtlinks(oa, act, tmp);
#endif

      break;
    case LSA_T_NET:
      ln = act->lsa_body;

#ifdef OSPFv2
      prefix = ipa_and(ipa_from_u32(act->lsa.id), ln->netmask);
      pxlen = ipa_mklen(ln->netmask);
      add_network(oa, prefix, pxlen, act->dist, act, -1);
#endif

      rts = (u32 *) (ln + 1);
      for (i = 0; i < lsa_net_count(&act->lsa); i++)
      {
	DBG("     Working on router %R ", rts[i]);
	tmp = ospf_hash_find_rt(po->gr, oa->areaid, rts[i]);
	if (tmp != NULL)
	  DBG("Found :-)\n");
	else
	  DBG("Not found!\n");
	add_cand(&oa->cand, tmp, act, act->dist, oa, -1);
      }
      break;
    }
  }

#ifdef OSPFv3
  process_prefixes(oa);
#endif
}

static int
link_back(struct ospf_area *oa, struct top_hash_entry *en, struct top_hash_entry *par)
{
  u32 i, *rts;
  struct ospf_lsa_net *ln;
  struct ospf_lsa_rt *rt;
  struct ospf_lsa_rt_link *rtl, *rr;
  struct top_hash_entry *tmp;
  struct proto_ospf *po = oa->po;

  if (!en || !par) return 0;

  /* We should check whether there is a link back from en to par,
     this is used in SPF calc (RFC 2328 16.1. (2b)). According to RFC 2328
     note 23, we don't have to find the same link that is used for par
     to en, any link is enough. This we do for ptp links. For net-rt
     links, we have to find the same link to compute proper lb/lb_id,
     which may be later used as the next hop. */

  /* In OSPFv2, en->lb is set here. In OSPFv3, en->lb is just cleared here,
     it is set in process_prefixes() to any global addres in the area */

  en->lb = IPA_NONE;
#ifdef OSPFv3
  en->lb_id = 0;
#endif
  switch (en->lsa.type)
  {
    case LSA_T_RT:
      rt = (struct ospf_lsa_rt *) en->lsa_body;
      rr = (struct ospf_lsa_rt_link *) (rt + 1);
      for (i = 0; i < lsa_rt_count(&en->lsa); i++)
      {
	rtl = (rr + i);
	switch (rtl->type)
	{
	case LSART_STUB:
	  break;
	case LSART_NET:
#ifdef OSPFv2
	  /* In OSPFv2, rtl->id is IP addres of DR, Router ID is not known */
	  tmp = ospf_hash_find_net(po->gr, oa->areaid, rtl->id);
#else /* OSPFv3 */
	  tmp = ospf_hash_find(po->gr, oa->areaid, rtl->nif, rtl->id, LSA_T_NET);
#endif
	  if (tmp == par)
	  {
#ifdef OSPFv2
	    en->lb = ipa_from_u32(rtl->data);
#else /* OSPFv3 */
	    en->lb_id = rtl->lif;
#endif
	    return 1;
	  }

	  break;
	case LSART_VLNK:
	case LSART_PTP:
	  /* Not necessary the same link, see RFC 2328 [23] */
	  tmp = ospf_hash_find_rt(po->gr, oa->areaid, rtl->id);
	  if (tmp == par)
            return 1;

	  break;
	default:
	  log(L_WARN "Unknown link type in router lsa. (rid = %R)", en->lsa.rt);
	  break;
	}
      }
      break;
    case LSA_T_NET:
      ln = en->lsa_body;
      rts = (u32 *) (ln + 1);
      for (i = 0; i < lsa_net_count(&en->lsa); i++)
      {
	tmp = ospf_hash_find_rt(po->gr, oa->areaid, rts[i]);
	if (tmp == par)
          return 1;
      }
      break;
    default:
      bug("Unknown lsa type %x.", en->lsa.type);
  }
  return 0;
}

  
/* RFC 2328 16.2. calculating inter-area routes */
static void
ospf_rt_sum(struct ospf_area *oa)
{
  struct proto_ospf *po = oa->po;
  struct proto *p = &po->proto;
  struct top_hash_entry *en;
  ip_addr ip = IPA_NONE;
  u32 dst_rid = 0;
  u32 metric, options;
  ort *abr;
  int pxlen = -1, type = -1;

  OSPF_TRACE(D_EVENTS, "Starting routing table calculation for inter-area (area %R)", oa->areaid);

  WALK_SLIST(en, po->lsal)
  {
    if ((en->lsa.type != LSA_T_SUM_RT) && (en->lsa.type != LSA_T_SUM_NET))
      continue;

    if (en->domain != oa->areaid)
      continue;

    /* 16.2. (1a) */
    if (en->lsa.age == LSA_MAXAGE)
      continue;

    /* 16.2. (2) */
    if (en->lsa.rt == po->router_id)
      continue;

    /* 16.2. (3) is handled later in ospf_rt_abr() by resetting such rt entry */

    if (en->lsa.type == LSA_T_SUM_NET)
    {
#ifdef OSPFv2
      struct ospf_lsa_sum *ls = en->lsa_body;
      ip = ipa_and(ipa_from_u32(en->lsa.id), ls->netmask);
      pxlen = ipa_mklen(ls->netmask);
#else /* OSPFv3 */
      u8 pxopts;
      u16 rest;
      struct ospf_lsa_sum_net *ls = en->lsa_body;
      lsa_get_ipv6_prefix(ls->prefix, &ip, &pxlen, &pxopts, &rest);

      if (pxopts & OPT_PX_NU)
	continue;
#endif

      if (pxlen < 0 || pxlen > MAX_PREFIX_LENGTH)
      {
	log(L_WARN "%s: Invalid prefix in LSA (Type: %04x, Id: %R, Rt: %R)",
	    p->name, en->lsa.type, en->lsa.id, en->lsa.rt);
	continue;
      }

      metric = ls->metric & METRIC_MASK;
      options = 0;
      type = ORT_NET;
    }
    else /* LSA_T_SUM_RT */
    {
#ifdef OSPFv2
      struct ospf_lsa_sum *ls = en->lsa_body;
      dst_rid = en->lsa.id;
      options = 0;
#else /* OSPFv3 */
      struct ospf_lsa_sum_rt *ls = en->lsa_body;
      dst_rid = ls->drid; 
      options = ls->options & OPTIONS_MASK;
#endif
      
      /* We don't want local router in ASBR routing table */
      if (dst_rid == po->router_id)
	continue;

      metric = ls->metric & METRIC_MASK;
      options |= ORTA_ASBR;
      type = ORT_ROUTER;
    }

    /* 16.2. (1b) */
    if (metric == LSINFINITY)
      continue;

    /* 16.2. (4) */
    ip_addr abrip = ipa_from_rid(en->lsa.rt);
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
      ri_install_net(po, ip, pxlen, &nf);
    else
      ri_install_rt(oa, dst_rid, &nf);
  }
}

/* RFC 2328 16.3. examining summary-LSAs in transit areas */
static void
ospf_rt_sum_tr(struct ospf_area *oa)
{
  struct proto *p = &oa->po->proto;
  struct proto_ospf *po = oa->po;
  struct ospf_area *bb = po->backbone;
  ip_addr abrip;
  struct top_hash_entry *en;
  u32 dst_rid, metric;
  ort *re = NULL, *abr;


  if (!bb) return;

  WALK_SLIST(en, po->lsal)
  {
    if ((en->lsa.type != LSA_T_SUM_RT) && (en->lsa.type != LSA_T_SUM_NET))
      continue;

    if (en->domain != oa->areaid)
      continue;

    /* 16.3 (1a) */
    if (en->lsa.age == LSA_MAXAGE)
      continue;

    /* 16.3 (2) */
    if (en->lsa.rt == po->router_id)
      continue;

    if (en->lsa.type == LSA_T_SUM_NET)
    {
      ip_addr ip;
      int pxlen;
#ifdef OSPFv2
      struct ospf_lsa_sum *ls = en->lsa_body;
      ip = ipa_and(ipa_from_u32(en->lsa.id), ls->netmask);
      pxlen = ipa_mklen(ls->netmask);
#else /* OSPFv3 */
      u8 pxopts;
      u16 rest;
      struct ospf_lsa_sum_net *ls = en->lsa_body;
      lsa_get_ipv6_prefix(ls->prefix, &ip, &pxlen, &pxopts, &rest);

      if (pxopts & OPT_PX_NU)
	continue;
#endif

      if (pxlen < 0 || pxlen > MAX_PREFIX_LENGTH)
      {
	log(L_WARN "%s: Invalid prefix in LSA (Type: %04x, Id: %R, Rt: %R)",
	    p->name, en->lsa.type, en->lsa.id, en->lsa.rt);
	continue;
      }

      metric = ls->metric & METRIC_MASK;
      re = fib_find(&po->rtf, &ip, pxlen);
    }
    else // en->lsa.type == LSA_T_SUM_RT
    {
#ifdef OSPFv2
      struct ospf_lsa_sum *ls = en->lsa_body;
      dst_rid = en->lsa.id;
#else /* OSPFv3 */
      struct ospf_lsa_sum_rt *ls = en->lsa_body;
      dst_rid = ls->drid; 
#endif

      metric = ls->metric & METRIC_MASK;
      ip_addr ip = ipa_from_rid(dst_rid);
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
	((metric == re->n.metric1) && unresolved_vlink(re->n.nhs)))
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

/* Decide about originating or flushing summary LSAs for condended area networks */
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
check_sum_net_lsa(struct proto_ospf *po, ort *nf)
{
  struct area_net *anet = NULL;
  struct ospf_area *anet_oa = NULL;

  /* RT entry marked as area network */
  if (nf->fn.x0)
  {
    /* It is a default route for stub areas, handled entirely in ospf_rt_abr() */
    if (nf->fn.pxlen == 0)
      return;

    /* Find that area network */
    WALK_LIST(anet_oa, po->area_list)
    {
      anet = (struct area_net *) fib_find(&anet_oa->net_fib, &nf->fn.prefix, nf->fn.pxlen);
      if (anet)
	break;
    }
  }

  struct ospf_area *oa;
  WALK_LIST(oa, po->area_list)
  {
    if (anet && decide_anet_lsa(oa, anet, anet_oa))
      originate_sum_net_lsa(oa, &nf->fn, anet->metric);
    else if (decide_sum_lsa(oa, nf, ORT_NET))
      originate_sum_net_lsa(oa, &nf->fn, nf->n.metric1);
    else
      flush_sum_lsa(oa, &nf->fn, ORT_NET);
  }
}

static inline void
check_sum_rt_lsa(struct proto_ospf *po, ort *nf)
{
  struct ospf_area *oa;
  WALK_LIST(oa, po->area_list)
  {
    if (decide_sum_lsa(oa, nf, ORT_ROUTER))
      originate_sum_rt_lsa(oa, &nf->fn, nf->n.metric1, nf->n.options);
    else
      flush_sum_lsa(oa, &nf->fn, ORT_ROUTER);
  }
}

static inline int
decide_nssa_lsa(ort *nf, u32 *rt_metric, ip_addr *rt_fwaddr, u32 *rt_tag)
{
  struct ospf_area *oa = nf->n.oa;
  struct top_hash_entry *en = nf->n.en;
  int propagate;

  if (!rt_is_nssa(nf) || !oa->translate)
    return 0;

  /* Condensed area network found */ 
  if (fib_route(&oa->enet_fib, nf->fn.prefix, nf->fn.pxlen))
    return 0;

  if (!en || (en->lsa.type != LSA_T_NSSA))
    return 0;

  /* We do not store needed data in struct orta, we have to parse the LSA */
  struct ospf_lsa_ext *le = en->lsa_body;

#ifdef OSPFv2
  *rt_fwaddr = le->fwaddr;
  *rt_tag = le->tag;
  propagate = en->lsa.options & OPT_P;
#else /* OSPFv3 */
  u32 *buf = le->rest;
  u8 pxlen = (*buf >> 24);
  u8 pxopts = (*buf >> 16);
  buf += IPV6_PREFIX_WORDS(pxlen);  /* Skip the IP prefix */

  if (pxopts & OPT_PX_NU)
    return 0;

  if (le->metric & LSA_EXT_FBIT)
    buf = lsa_get_ipv6_addr(buf, rt_fwaddr);
  else
    *rt_fwaddr = IPA_NONE;

  if (le->metric & LSA_EXT_TBIT)
    *rt_tag = *buf++;
  else
    *rt_tag = 0;

  propagate = pxopts & OPT_PX_P;
#endif

  if (!propagate || ipa_zero(*rt_fwaddr))
    return 0;

  *rt_metric = le->metric & (METRIC_MASK | LSA_EXT_EBIT);
  return 1;
}

/* RFC 3103 3.2 - translating Type-7 LSAs into Type-5 LSAs */
static inline void
check_nssa_lsa(struct proto_ospf *po, ort *nf)
{
  struct fib_node *fn = &nf->fn;
  struct area_net *anet = NULL;
  struct ospf_area *oa = NULL;
  u32 rt_metric, rt_tag;
  ip_addr rt_fwaddr;

  /* Do not translate LSA if there is already the external LSA from route export */
  if (fn->x1 == EXT_EXPORT)
    return;

  /* RT entry marked as area network */
  if (fn->x0)
  {
    /* Find that area network */
    WALK_LIST(oa, po->area_list)
    {
      anet = (struct area_net *) fib_find(&oa->enet_fib, &fn->prefix, fn->pxlen);
      if (anet)
	break;
    }
  }

  /* RFC 3103 3.2 (3) - originate the aggregated address range */
  if (anet && anet->active && !anet->hidden && oa->translate)
    originate_ext_lsa(po->backbone, fn, EXT_NSSA, anet->metric, IPA_NONE, anet->tag, 0);

  /* RFC 3103 3.2 (2) - originate the same network */
  else if (decide_nssa_lsa(nf, &rt_metric, &rt_fwaddr, &rt_tag))
    originate_ext_lsa(po->backbone, fn, EXT_NSSA, rt_metric, rt_fwaddr, rt_tag, 0);

  else if (fn->x1 == EXT_NSSA)
    flush_ext_lsa(po->backbone, fn, 0);
}

/* RFC 2328 16.7. p2 - find new/lost vlink endpoints */
static void
ospf_check_vlinks(struct proto_ospf *po)
{
  struct proto *p = &po->proto;

  struct ospf_iface *iface;
  WALK_LIST(iface, po->iface_list)
  {
    if (iface->type == OSPF_IT_VLINK)
    {
      struct top_hash_entry *tmp;
      tmp = ospf_hash_find_rt(po->gr, iface->voa->areaid, iface->vid);

      if (tmp && (tmp->color == INSPF) && ipa_nonzero(tmp->lb) && tmp->nhs)
      {
	struct ospf_iface *nhi = ospf_iface_find(po, tmp->nhs->iface);

        if ((iface->state != OSPF_IS_PTP)
	    || (iface->vifa != nhi)
	    || !ipa_equal(iface->vip, tmp->lb))
        {
          OSPF_TRACE(D_EVENTS, "Vlink peer %R found", tmp->lsa.id);
          ospf_iface_sm(iface, ISM_DOWN);
	  iface->vifa = nhi;
          iface->iface = nhi->iface;
	  iface->addr = nhi->addr;
	  iface->sk = nhi->sk;
	  iface->cost = tmp->dist;
          iface->vip = tmp->lb;
          ospf_iface_sm(iface, ISM_UP);
        }
	else if ((iface->state == OSPF_IS_PTP) && (iface->cost != tmp->dist))
	{
	  iface->cost = tmp->dist;
	  schedule_rt_lsa(po->backbone);
	}
      }
      else
      {
        if (iface->state > OSPF_IS_DOWN)
        {
          OSPF_TRACE(D_EVENTS, "Vlink peer %R lost", iface->vid);
	  ospf_iface_sm(iface, ISM_DOWN);
        }
      }
    }
  }
}


/* Miscellaneous route processing that needs to be done by ABRs */
static void
ospf_rt_abr1(struct proto_ospf *po)
{
  struct area_net *anet;
  ort *nf, *default_nf;

  FIB_WALK(&po->rtf, nftmp)
  {
    nf = (ort *) nftmp;


    /* RFC 2328 G.3 - incomplete resolution of virtual next hops */
    if (nf->n.type && unresolved_vlink(nf->n.nhs))
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
	  ort *nfi = (ort *) fib_get(&po->rtf, &anet->fn.prefix, anet->fn.pxlen);
	  nfi->fn.x0 = 1; /* mark and keep persistent, to have stable UID */

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
  default_nf = (ort *) fib_get(&po->rtf, &addr, 0);
  default_nf->fn.x0 = 1; /* keep persistent */

  struct ospf_area *oa;
  WALK_LIST(oa, po->area_list)
  {

    /* 12.4.3.1. - originate or flush default route for stub/NSSA areas */
    if (oa_is_stub(oa) || (oa_is_nssa(oa) && !oa->ac->summary))
      originate_sum_net_lsa(oa, &default_nf->fn, oa->ac->default_cost);
    else
      flush_sum_lsa(oa, &default_nf->fn, ORT_NET);

    /*
     * Originate type-7 default route for NSSA areas
     *
     * Because type-7 default LSAs are originated by ABRs, they do not
     * collide with other type-7 LSAs (as ABRs generate type-5 LSAs
     * for both external route export or external-NSSA translation),
     * so we use 0 for the src arg.
     */

    if (oa_is_nssa(oa) && oa->ac->default_nssa)
      originate_ext_lsa(oa, &default_nf->fn, 0, oa->ac->default_cost, IPA_NONE, 0, 0);
    else
      flush_ext_lsa(oa, &default_nf->fn, 1);


    /* RFC 2328 16.4. (3) - precompute preferred ASBR entries */
    if (oa_is_ext(oa))
    {
      FIB_WALK(&oa->rtr, nftmp)
      {
	nf = (ort *) nftmp;
	if (nf->n.options & ORTA_ASBR)
	  ri_install_asbr(po, &nf->fn.prefix, &nf->n);
      }
      FIB_WALK_END;
    }
  }


  /* Originate or flush ASBR summary LSAs */
  FIB_WALK(&po->backbone->rtr, nftmp)
  {
    check_sum_rt_lsa(po, (ort *) nftmp);
  }
  FIB_WALK_END;


  /* RFC 2328 16.7. p2 - find new/lost vlink endpoints */
  ospf_check_vlinks(po);
}


static void
translator_timer_hook(timer *timer)
{
  struct ospf_area *oa = timer->data;
  
  if (oa->translate != TRANS_WAIT)
    return;

  oa->translate = TRANS_OFF;
  schedule_rtcalc(oa->po);
}

static void
ospf_rt_abr2(struct proto_ospf *po)
{
  struct ospf_area *oa;
  struct top_hash_entry *en;
  ort *nf, *nf2;


  /* RFC 3103 3.1 - type-7 translator election */
  struct ospf_area *bb = po->backbone;
  WALK_LIST(oa, po->area_list)
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

	en = ospf_hash_find_rt(po->gr, oa->areaid, nf->n.rid);
	if (!en || (en->color != INSPF))
	  continue;

	struct ospf_lsa_rt *rt = en->lsa_body;
	/* There is better candidate - Nt-bit or higher Router ID */
	if ((rt->options & OPT_RT_NT) || (po->router_id < nf->n.rid))
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
	  oa->translator_timer = tm_new_set(po->proto.pool, translator_timer_hook, oa, 0, 0);

	/* Schedule the end of translation */
	tm_start(oa->translator_timer, oa->ac->transint);
	oa->translate = TRANS_WAIT;
      }
    }


  /* Compute condensed external networks */
  FIB_WALK(&po->rtf, nftmp)
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
	  nf2 = (ort *) fib_get(&po->rtf, &anet->fn.prefix, anet->fn.pxlen);
	  nf2->fn.x0 = 1;
	}

	u32 metric = (nf->n.type == RTS_OSPF_EXT1) ?
	  nf->n.metric1 : ((nf->n.metric2 + 1) | LSA_EXT_EBIT);

	if (anet->metric < metric)
	  anet->metric = metric;
      }
    }
  }
  FIB_WALK_END;


  FIB_WALK(&po->rtf, nftmp)
  {
    nf = (ort *) nftmp;

    check_sum_net_lsa(po, nf);
    check_nssa_lsa(po, nf);
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
ospf_ext_spf(struct proto_ospf *po)
{
  ort *nf1, *nf2;
  orta nfa;
  struct top_hash_entry *en;
  struct proto *p = &po->proto;
  struct ospf_lsa_ext *le;
  int pxlen, ebit, rt_fwaddr_valid, rt_propagate;
  ip_addr ip, rtid, rt_fwaddr;
  u32 br_metric, rt_metric, rt_tag;
  struct ospf_area *atmp;
  struct mpnh* nhs = NULL;

  OSPF_TRACE(D_EVENTS, "Starting routing table calculation for ext routes");

  WALK_SLIST(en, po->lsal)
  {
    /* 16.4. (1) */
    if ((en->lsa.type != LSA_T_EXT) && (en->lsa.type != LSA_T_NSSA))
      continue;

    if (en->lsa.age == LSA_MAXAGE)
      continue;

    /* 16.4. (2) */
    if (en->lsa.rt == po->router_id)
      continue;

    DBG("%s: Working on LSA. ID: %R, RT: %R, Type: %u\n",
	p->name, en->lsa.id, en->lsa.rt, en->lsa.type);

    le = en->lsa_body;

    rt_metric = le->metric & METRIC_MASK;
    ebit = le->metric & LSA_EXT_EBIT;

    if (rt_metric == LSINFINITY)
      continue;

#ifdef OSPFv2
    ip = ipa_and(ipa_from_u32(en->lsa.id), le->netmask);
    pxlen = ipa_mklen(le->netmask);
    rt_fwaddr = le->fwaddr;
    rt_fwaddr_valid = !ipa_equal(rt_fwaddr, IPA_NONE);
    rt_tag = le->tag;
    rt_propagate = en->lsa.options & OPT_P;
#else /* OSPFv3 */
    u8 pxopts;
    u16 rest;
    u32 *buf = le->rest;
    buf = lsa_get_ipv6_prefix(buf, &ip, &pxlen, &pxopts, &rest);

    if (pxopts & OPT_PX_NU)
      continue;

    rt_fwaddr_valid = le->metric & LSA_EXT_FBIT;
    if (rt_fwaddr_valid)
      buf = lsa_get_ipv6_addr(buf, &rt_fwaddr);
    else 
      rt_fwaddr = IPA_NONE;

    if (le->metric & LSA_EXT_TBIT)
      rt_tag = *buf++;
    else
      rt_tag = 0;

    rt_propagate = pxopts & OPT_PX_P;
#endif

    if (pxlen < 0 || pxlen > MAX_PREFIX_LENGTH)
    {
      log(L_WARN "%s: Invalid prefix in LSA (Type: %04x, Id: %R, Rt: %R)",
	  p->name, en->lsa.type, en->lsa.id, en->lsa.rt);
      continue;
    }


    /* 16.4. (3) */
    /* If there are more areas, we already precomputed preferred ASBR
       entries in ospf_rt_abr1() and stored them in the backbone
       table. For NSSA, we examine the area to which the LSA is assigned */
    if (en->lsa.type == LSA_T_EXT)
      atmp = ospf_main_area(po);
    else /* NSSA */
      atmp = ospf_find_area(po, en->domain);

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
    if ((en->lsa.type == LSA_T_NSSA) && ipa_zero(ip) && (pxlen == 0) &&
	(po->areano > 1) && !(rt_propagate && atmp->ac->summary))
      continue;

    if (!rt_fwaddr_valid)
    {
      nf2 = nf1;
      nhs = nf1->n.nhs;
      br_metric = nf1->n.metric1;
    }
    else
    {
      nf2 = ospf_fib_route(&po->rtf, rt_fwaddr, MAX_PREFIX_LENGTH);
      if (!nf2)
	continue;

      if (en->lsa.type == LSA_T_EXT)
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

      nhs = nf2->n.nhs;
      /* If gw is zero, it is a device route */
      if (ipa_zero(nhs->gw))
	nhs = new_nexthop(po, rt_fwaddr, nhs->iface, nhs->weight);
      br_metric = nf2->n.metric1;
    }

    if (ebit)
    {
      nfa.type = RTS_OSPF_EXT2;
      nfa.metric1 = br_metric;
      nfa.metric2 = rt_metric;
    }
    else
    {
      nfa.type = RTS_OSPF_EXT1;
      nfa.metric1 = br_metric + rt_metric;
      nfa.metric2 = LSINFINITY;
    }

    /* Mark the LSA as reachable */
    en->color = INSPF;

    /* Whether the route is preferred in route selection according to 16.4.1 */
    nfa.options = epath_preferred(&nf2->n) ? ORTA_PREF : 0;
    if (en->lsa.type == LSA_T_NSSA)
    {
      nfa.options |= ORTA_NSSA;
      if (rt_propagate)
	nfa.options |= ORTA_PROP;
    }

    nfa.tag = rt_tag;
    nfa.rid = en->lsa.rt;
    nfa.oa = atmp; /* undefined in RFC 2328 */
    nfa.voa = NULL;
    nfa.nhs = nhs;
    nfa.en = en; /* store LSA for later (NSSA processing) */

    ri_install_ext(po, ip, pxlen, &nfa);
  }
}

/* Cleanup of routing tables and data */
void
ospf_rt_reset(struct proto_ospf *po)
{
  struct ospf_area *oa;
  struct top_hash_entry *en;
  struct area_net *anet;
  ort *ri;

  /* Reset old routing table */
  FIB_WALK(&po->rtf, nftmp)
  {
    ri = (ort *) nftmp;
    ri->fn.x0 = 0;
    reset_ri(ri);
  }
  FIB_WALK_END;

  /* Reset SPF data in LSA db */
  WALK_SLIST(en, po->lsal)
  {
    en->color = OUTSPF;
    en->dist = LSINFINITY;
    en->nhs = NULL;
    en->lb = IPA_NONE;
  }

  WALK_LIST(oa, po->area_list)
  {
    /* Reset ASBR routing tables */
    FIB_WALK(&oa->rtr, nftmp)
    {
      ri = (ort *) nftmp;
      reset_ri(ri);
    }
    FIB_WALK_END;

    /* Reset condensed area networks */
    if (po->areano > 1)
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
 * @po: OSPF protocol
 *
 * Calculation of internal paths in an area is described in 16.1 of RFC 2328.
 * It's based on Dijkstra's shortest path tree algorithms.
 * This function is invoked from ospf_disp().
 */
void
ospf_rt_spf(struct proto_ospf *po)
{
  struct proto *p = &po->proto;
  struct ospf_area *oa;

  if (po->areano == 0)
    return;

  OSPF_TRACE(D_EVENTS, "Starting routing table calculation");

  /* 16. (1) */
  ospf_rt_reset(po);

  /* 16. (2) */
  WALK_LIST(oa, po->area_list)
    ospf_rt_spfa(oa);

  /* 16. (3) */
  ospf_rt_sum(ospf_main_area(po));

  /* 16. (4) */
  WALK_LIST(oa, po->area_list)
    if (oa->trcap && (oa->areaid != 0))
      ospf_rt_sum_tr(oa);

  if (po->areano > 1)
    ospf_rt_abr1(po);

  /* 16. (5) */
  ospf_ext_spf(po);

  if (po->areano > 1)
    ospf_rt_abr2(po);

  rt_sync(po);
  lp_flush(po->nhpool);
  
  po->calcrt = 0;
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
  // struct proto *p = &oa->po->proto;
  struct proto_ospf *po = oa->po;
  struct mpnh *pn = par->nhs;
  struct ospf_iface *ifa;
  u32 rid = en->lsa.rt;

  /* 16.1.1. The next hop calculation */
  DBG("     Next hop calculating for id: %R rt: %R type: %u\n",
      en->lsa.id, en->lsa.rt, en->lsa.type);

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
  if ((en->lsa.type == LSA_T_NET) && (par == oa->rt))
  {
    ifa = rt_pos_to_ifa(oa, pos);
    if (!ifa)
      return NULL;

    return new_nexthop(po, IPA_NONE, ifa->iface, ifa->ecmp_weight);
  }

  /* The second case - ptp or ptmp neighbor */
  if ((en->lsa.type == LSA_T_RT) && (par == oa->rt))
  {
    ifa = rt_pos_to_ifa(oa, pos);
    if (!ifa)
      return NULL;

    if (ifa->type == OSPF_IT_VLINK)
      return new_nexthop(po, IPA_NONE, NULL, 0);

    struct ospf_neighbor *m = find_neigh(ifa, rid);
    if (!m || (m->state != NEIGHBOR_FULL))
      return NULL;

    return new_nexthop(po, m->ip, ifa->iface, ifa->ecmp_weight);
  }

  /* The third case - bcast or nbma neighbor */
  if ((en->lsa.type == LSA_T_RT) && (par->lsa.type == LSA_T_NET))
  {
    /* par->nhi should be defined from parent's calc_next_hop() */
    if (!pn)
      goto bad;

#ifdef OSPFv2
    /*
     * In this case, next-hop is the same as link-back, which is
     * already computed in link_back().
     */
    if (ipa_zero(en->lb))
      goto bad;

    return new_nexthop(po, en->lb, pn->iface, pn->weight);

#else /* OSPFv3 */
    /*
     * Next-hop is taken from lladdr field of Link-LSA, en->lb_id
     * is computed in link_back().
     */
    struct top_hash_entry *lhe;
    lhe = ospf_hash_find(po->gr, pn->iface->index, en->lb_id, rid, LSA_T_LINK);

    if (!lhe)
      return NULL;

    struct ospf_lsa_link *llsa = lhe->lsa_body;
      
    if (ipa_zero(llsa->lladdr))
      return NULL;

    return new_nexthop(po, llsa->lladdr, pn->iface, pn->weight);
#endif
  }

 bad:
  /* Probably bug or some race condition, we log it */
  log(L_ERR "Unexpected case in next hop calculation");
  return NULL;
}

/* Compare nexthops during merge.
   We need to maintain nhs sorted to eliminate duplicities */
static int
cmp_nhs(struct mpnh *s1, struct mpnh *s2)
{
  int r;

  if (!s1)
    return 1;

  if (!s2)
    return -1;

  r = ((int) s2->weight) - ((int) s1->weight);
  if (r)
    return r;

  r = ipa_compare(s1->gw, s2->gw);
  if (r)
    return r;

  return ((int) s1->iface->index) - ((int) s2->iface->index);
}

static void
merge_nexthops(struct proto_ospf *po, struct top_hash_entry *en,
	       struct top_hash_entry *par, struct mpnh *new)
{
  if (en->nhs == new)
    return;

  int r1 = en->nhs_reuse;
  int r2 = (par->nhs != new);
  int count = po->ecmp;
  struct mpnh *s1 = en->nhs;
  struct mpnh *s2 = new;
  struct mpnh **n = &(en->nhs);

  /*
   * r1, r2 signalize whether we can reuse nexthops from s1, s2.
   * New nexthops (s2, new) can be reused if they are not inherited
   * from the parent (i.e. it is allocated in calc_next_hop()).
   * Current nexthops (s1, en->nhs) can be reused if they weren't
   * inherited in previous steps (that is stored in nhs_reuse,
   * i.e. created by merging or allocalted in calc_next_hop()).
   *
   * Generally, a node first inherits shared nexthops from its
   * parent and later possibly gets reusable copy during merging.
   */

  while ((s1 || s2) && count--)
  {
    int cmp = cmp_nhs(s1, s2);
    if (cmp < 0)
    {
      *n = r1 ? s1 : copy_nexthop(po, s1);
      s1 = s1->next;
    }
    else if (cmp > 0)
    {
      *n = r2 ? s2 : copy_nexthop(po, s2);
      s2 = s2->next;
    }
    else
    {
      *n = r1 ? s1 : (r2 ? s2 : copy_nexthop(po, s1));
      s1 = s1->next;
      s2 = s2->next;
    }
    n = &((*n)->next);
  }
  *n = NULL;

  en->nhs_reuse=1;
}

/* Add LSA into list of candidates in Dijkstra's algorithm */
static void
add_cand(list * l, struct top_hash_entry *en, struct top_hash_entry *par,
	 u32 dist, struct ospf_area *oa, int pos)
{
  struct proto_ospf *po = oa->po;
  node *prev, *n;
  int added = 0;
  struct top_hash_entry *act;

  /* 16.1. (2b) */
  if (en == NULL)
    return;
  if (en->lsa.age == LSA_MAXAGE)
    return;

#ifdef OSPFv3
  if (en->lsa.type == LSA_T_RT)
    {
      struct ospf_lsa_rt *rt = en->lsa_body;
      if (!(rt->options & OPT_V6))
	return;
    }
#endif

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
    log(L_WARN "Cannot find next hop for LSA (Type: %04x, Id: %R, Rt: %R)",
	en->lsa.type, en->lsa.id, en->lsa.rt);
    return;
  }

  if (dist == en->dist)
  {
    /*
     * For multipath, we should merge nexthops. We do not mix dummy
     * vlink nexthops, device nexthops and gateway nexthops. We merge
     * gateway nexthops only. We prefer device nexthops over gateway
     * nexthops and gateway nexthops over vlink nexthops. We either
     * keep old nexthops, merge old and new, or replace old with new.
     * 
     * We know that en->color == CANDIDATE and en->nhs is defined.
     */
    struct mpnh *onhs = en->nhs;

    /* Keep old ones */
    if (!po->ecmp || !nhs->iface || (onhs->iface && ipa_zero(onhs->gw)))
      return;

    /* Merge old and new */
    if (ipa_nonzero(nhs->gw) && ipa_nonzero(onhs->gw))
    {
      merge_nexthops(po, en, par, nhs);
      return;
    }

    /* Fallback to replace old ones */
  }

  DBG("     Adding candidate: rt: %R, id: %R, type: %u\n",
      en->lsa.rt, en->lsa.id, en->lsa.type);

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
	  ((act->dist == dist) && (act->lsa.type == LSA_T_RT)))
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
rt_sync(struct proto_ospf *po)
{
  struct proto *p = &po->proto;
  struct fib_iterator fit;
  struct fib *fib = &po->rtf;
  ort *nf;
  struct ospf_area *oa;

  /* This is used for forced reload of routes */
  int reload = (po->calcrt == 2);

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
	  neighbor *ng = neigh_find2(p, &nh->gw, nh->iface, 0);
	  if (!ng || (ng->scope == SCOPE_HOST))
	    { reset_ri(nf); break; }
	}
    }

    /* Remove configured stubnets */
    if (!nf->n.nhs)
      reset_ri(nf);

    if (nf->n.type) /* Add the route */
    {
      rta a0 = {
	.proto = p,
	.source = nf->n.type,
	.scope = SCOPE_UNIVERSE,
	.cast = RTC_UNICAST,
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
	net *ne = net_get(p->table, nf->fn.prefix, nf->fn.pxlen);
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
	e->pref = p->preference;

	DBG("Mod rte type %d - %I/%d via %I on iface %s, met %d\n",
	    a0.source, nf->fn.prefix, nf->fn.pxlen, a0.gw, a0.iface ? a0.iface->name : "(none)", nf->n.metric1);
	rte_update(p->table, ne, p, p, e);
      }
    }
    else if (nf->old_rta)
    {
      /* Remove the route */
      rta_free(nf->old_rta);
      nf->old_rta = NULL;

      net *ne = net_get(p->table, nf->fn.prefix, nf->fn.pxlen);
      rte_update(p->table, ne, p, p, NULL);
    }

    /* Remove unused rt entry. Entries with fn.x0 == 1 are persistent. */
    if (!nf->n.type && !nf->fn.x0 && !nf->fn.x1)
    {
      FIB_ITERATE_PUT(&fit, nftmp);
      fib_delete(fib, nftmp);
      goto again1;
    }
  }
  FIB_ITERATE_END(nftmp);


  WALK_LIST(oa, po->area_list)
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
}
