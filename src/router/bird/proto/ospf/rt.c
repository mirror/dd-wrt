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
		     struct ospf_area *oa);
static int calc_next_hop(struct ospf_area *oa,
			 struct top_hash_entry *en,
			 struct top_hash_entry *par);
static void rt_sync(struct proto_ospf *po);

/* In ospf_area->rtr we store paths to routers, but we use RID (and not IP address)
   as index, so we need to encapsulate RID to IP address */
#ifdef OSPFv2
#define ipa_from_rid(x) _MI(x)
#else /* OSPFv3 */
#define ipa_from_rid(x) _MI(0,0,0,x)
#endif


static inline void reset_ri(orta * orta)
{
  bzero(orta, sizeof(orta));
}

void
ospf_rt_initort(struct fib_node *fn)
{
  ort *ri = (ort *) fn;
  reset_ri(&ri->n);
  reset_ri(&ri->o);
  ri->fn.x0 = 0;
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


#ifdef OSPFv2

static struct ospf_iface *
find_stub_src(struct ospf_area *oa, ip_addr px, int pxlen)
{
  struct ospf_iface *iff;

  WALK_LIST(iff, oa->po->iface_list)
    if ((iff->type != OSPF_IT_VLINK) &&
	(iff->oa == oa) &&
	ipa_equal(iff->addr->prefix, px) && 
	(iff->addr->pxlen == pxlen))
      return iff;

  return NULL;
}

#else /* OSPFv3 */

static struct ospf_iface *
find_stub_src(struct ospf_area *oa, ip_addr px, int pxlen)
{
  struct ospf_iface *iff;
  struct ifa *a;

  WALK_LIST(iff, oa->po->iface_list)
    if ((iff->type != OSPF_IT_VLINK) &&
	(iff->oa == oa))
      WALK_LIST(a, iff->iface->addrs)
	if (ipa_equal(a->prefix, px) && 
	    (a->pxlen == pxlen) &&
	    !(a->flags & IA_SECONDARY))
	  return iff;

  return NULL;
}

#endif

static void
add_network(struct ospf_area *oa, ip_addr px, int pxlen, int metric, struct top_hash_entry *en)
{
  orta nf = {
    .type = RTS_OSPF,
    .options = 0,
    .metric1 = metric,
    .metric2 = LSINFINITY,
    .tag = 0,
    .rid = en->lsa.rt,
    .oa = oa,
    .ifa = en->nhi,
    .nh = en->nh
  };

  if (en == oa->rt)
  {
    /* 
     * Local stub networks does not have proper iface in en->nhi
     * (because they all have common top_hash_entry en).
     * We have to find iface responsible for that stub network.
     * Configured stubnets does not have any iface. They will
     * be removed in rt_sync().
     */

    nf.ifa = find_stub_src(oa, px, pxlen);
    nf.nh = IPA_NONE;
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

	add_network(oa, pxa, pxlen, src->dist + metric, src);
      }
  }
}
#endif


static void
ospf_rt_spfa_rtlinks(struct ospf_area *oa, struct top_hash_entry *act, struct top_hash_entry *en)
{
  // struct proto *p = &oa->po->proto;
  struct proto_ospf *po = oa->po;
  u32 i;

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
	  add_network(oa, ipa_from_u32(rtl->id),
		      ipa_mklen(ipa_from_u32(rtl->data)),
		      act->dist + rtl->metric, act);
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
      add_cand(&oa->cand, tmp, act, act->dist + rtl->metric, oa);
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
	  .ifa = act->nhi,
	  .nh = act->nh
	};
	ri_install_rt(oa, act->lsa.rt, &nf);
      }

#ifdef OSPFv2
      ospf_rt_spfa_rtlinks(oa, act, act);
#else /* OSPFv3 */
      for (tmp = ospf_hash_find_rt_first(po->gr, act->domain, act->lsa.rt);
	   tmp; tmp = ospf_hash_find_rt_next(tmp))
	ospf_rt_spfa_rtlinks(oa, act, tmp);
#endif

      break;
    case LSA_T_NET:
      ln = act->lsa_body;

#ifdef OSPFv2
      add_network(oa, ipa_and(ipa_from_u32(act->lsa.id), ln->netmask),
		  ipa_mklen(ln->netmask), act->dist, act);
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
	add_cand(&oa->cand, tmp, act, act->dist, oa);
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
      pxlen = ipa_mklen(ls->netmask);
      ip = ipa_and(ipa_from_u32(en->lsa.id), ls->netmask);
#else /* OSPFv3 */
      u8 pxopts;
      u16 rest;
      struct ospf_lsa_sum_net *ls = en->lsa_body;
      lsa_get_ipv6_prefix(ls->prefix, &ip, &pxlen, &pxopts, &rest);

      if (pxopts & OPT_PX_NU)
	continue;
#endif

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
      .ifa = abr->n.ifa,
      .nh = abr->n.nh
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
  // struct proto *p = &oa->po->proto;
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
      pxlen = ipa_mklen(ls->netmask);
      ip = ipa_and(ipa_from_u32(en->lsa.id), ls->netmask);
#else /* OSPFv3 */
      u8 pxopts;
      u16 rest;
      struct ospf_lsa_sum_net *ls = en->lsa_body;
      lsa_get_ipv6_prefix(ls->prefix, &ip, &pxlen, &pxopts, &rest);

      if (pxopts & OPT_PX_NU)
	continue;
#endif

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
    if (metric <= re->n.metric1)
    {
      /* We want to replace the next-hop even if the metric is equal
	 to replace a virtual next-hop through vlink with a real one */
      re->n.metric1 = metric;
      re->n.nh = abr->n.nh;
      re->n.ifa = abr->n.ifa;
    }
  }
}

/* Decide about originating or flushing summary LSAs for condended area networks */
static int
decide_anet_lsa(struct ospf_area *oa, struct area_net *anet, struct ospf_area *anet_oa)
{
  if (oa->stub)
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
  /* 12.4.3.1. - do not send summary into stub areas, we send just default route */
  if (oa->stub)
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
  if (nf->n.ifa && (nf->n.ifa->oa->areaid == oa->areaid))
    return 0;

  /* 12.4.3 p5 */
  if (nf->n.metric1 >= LSINFINITY)
    return 0;

  /* 12.4.3 p6 - AS boundary router */
  if (dest == ORT_ROUTER)
  {
    /* We call decide_sum_lsa() on preferred ASBR entries, no need for 16.4. (3) */
    /* 12.4.3 p1 */
    return (nf->n.options & ORTA_ASBR);
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

      if (tmp && (tmp->color == INSPF) && ipa_nonzero(tmp->lb))
      {
        if ((iface->state != OSPF_IS_PTP)
	    || (iface->vifa != tmp->nhi)
	    || !ipa_equal(iface->vip, tmp->lb))
        {
          OSPF_TRACE(D_EVENTS, "Vlink peer %R found", tmp->lsa.id);
          ospf_iface_sm(iface, ISM_DOWN);
	  iface->vifa = tmp->nhi;
          iface->iface = tmp->nhi->iface;
	  iface->addr = tmp->nhi->addr;
	  iface->sk = tmp->nhi->sk;
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
ospf_rt_abr(struct proto_ospf *po)
{
  struct area_net *anet;
  ort *nf, *default_nf;

  FIB_WALK(&po->rtf, nftmp)
  {
    nf = (ort *) nftmp;


    /* RFC 2328 G.3 - incomplete resolution of virtual next hops */
    if (nf->n.type && nf->n.ifa && (nf->n.ifa->type == OSPF_IT_VLINK))
      reset_ri(&nf->n);


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
	    reset_ri(&nfi->n);
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

    /* 12.4.3.1. - originate or flush default summary LSA for stub areas */
    if (oa->stub)
      originate_sum_net_lsa(oa, &default_nf->fn, oa->stub);
    else
      flush_sum_lsa(oa, &default_nf->fn, ORT_NET);


    /* RFC 2328 16.4. (3) - precompute preferred ASBR entries */
    FIB_WALK(&oa->rtr, nftmp)
    {
      nf = (ort *) nftmp;
      if (nf->n.options & ORTA_ASBR)
	ri_install_asbr(po, &nf->fn.prefix, &nf->n);
    }
    FIB_WALK_END;
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
  int pxlen, ebit, rt_fwaddr_valid;
  ip_addr ip, nh, rtid, rt_fwaddr;
  struct ospf_iface *nhi = NULL;
  u32 br_metric, rt_metric, rt_tag;
  struct ospf_area *atmp;

  OSPF_TRACE(D_EVENTS, "Starting routing table calculation for ext routes");

  WALK_SLIST(en, po->lsal)
  {
    /* 16.4. (1) */
    if (en->lsa.type != LSA_T_EXT)
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
#endif

    if (pxlen < 0)
    {
      log(L_WARN "%s: Invalid mask in LSA (Type: %04x, Id: %R, Rt: %R)",
	  p->name, en->lsa.type, en->lsa.id, en->lsa.rt);
      continue;
    }
    nhi = NULL;
    nh = IPA_NONE;

    /* 16.4. (3) */
    /* If there are more areas, we already precomputed preferred ASBR entries
       in ospf_asbr_spf() and stored them in the backbone table */
    atmp = (po->areano > 1) ? po->backbone : HEAD(po->area_list);
    rtid = ipa_from_rid(en->lsa.rt);
    nf1 = fib_find(&atmp->rtr, &rtid, MAX_PREFIX_LENGTH);

    if (!nf1 || !nf1->n.type)
      continue;			/* No AS boundary router found */

    if (!(nf1->n.options & ORTA_ASBR))
      continue;			/* It is not ASBR */

    if (!rt_fwaddr_valid)
    {
      nf2 = nf1;
      nh = nf1->n.nh;
      nhi = nf1->n.ifa;
      br_metric = nf1->n.metric1;
    }
    else
    {
      nf2 = ospf_fib_route(&po->rtf, rt_fwaddr, MAX_PREFIX_LENGTH);
      if (!nf2)
	continue;

      if ((nf2->n.type != RTS_OSPF) && (nf2->n.type != RTS_OSPF_IA))
	continue;

      /* Next-hop is a part of a configured stubnet */
      if (!nf2->n.ifa)
	continue;

      /* If nh is zero, it is a device route */
      nh = ipa_nonzero(nf2->n.nh) ? nf2->n.nh : rt_fwaddr;
      nhi = nf2->n.ifa;
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

    nfa.tag = rt_tag;
    nfa.rid = en->lsa.rt;
    nfa.oa = nf1->n.oa; /* undefined in RFC 2328 */
    nfa.ifa = nhi;
    nfa.nh = nh;

    ri_install_ext(po, ip, pxlen, &nfa);
  }
}

/* Cleanup of routing tables and data Cleanup  */
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
    memcpy(&ri->o, &ri->n, sizeof(orta));	/* Backup old data */
    ri->fn.x0 = 0;
    reset_ri(&ri->n);
  }
  FIB_WALK_END;

  /* Reset SPF data in LSA db */
  WALK_SLIST(en, po->lsal)
  {
    en->color = OUTSPF;
    en->dist = LSINFINITY;
    en->nhi = NULL;
    en->nh = IPA_NONE;
    en->lb = IPA_NONE;
  }

  WALK_LIST(oa, po->area_list)
  {
    /* Reset ASBR routing tables */
    FIB_WALK(&oa->rtr, nftmp)
    {
      ri = (ort *) nftmp;
      memcpy(&ri->o, &ri->n, sizeof(orta));	/* Backup old data */
      reset_ri(&ri->n);
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
  if (po->areano == 1)
    ospf_rt_sum(HEAD(po->area_list));
  else
    ospf_rt_sum(po->backbone);

  /* 16. (4) */
  WALK_LIST(oa, po->area_list)
    if (oa->trcap && (oa->areaid != 0))
      ospf_rt_sum_tr(oa);

  if (po->areano > 1)
    ospf_rt_abr(po);

  /* 16. (5) */
  ospf_ext_spf(po);

  rt_sync(po);

  po->calcrt = 0;
}

/* Add LSA into list of candidates in Dijkstra's algorithm */
static void
add_cand(list * l, struct top_hash_entry *en, struct top_hash_entry *par,
	 u32 dist, struct ospf_area *oa)
{
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
      if (!(rt->options & OPT_V6) || !(rt->options & OPT_R))
	return;
    }
#endif

  /* 16.1. (2c) */
  if (en->color == INSPF)
    return;

  /* 16.1. (2d), also checks that dist < LSINFINITY */
  if (dist >= en->dist)
    return;
  /*
   * The line above (=) is not a bug, but we don't support multiple
   * next hops. I'll start as soon as nest will
   */

  /* We should check whether there is a reverse link from en to par, */
  if (!link_back(oa, en, par))
    return;

  if (!calc_next_hop(oa, en, par))
  {
    log(L_WARN "Cannot find next hop for LSA (Type: %04x, Id: %R, Rt: %R)",
	en->lsa.type, en->lsa.id, en->lsa.rt);
    return;
  }

  DBG("     Adding candidate: rt: %R, id: %R, type: %u\n",
      en->lsa.rt, en->lsa.id, en->lsa.type);

  if (en->color == CANDIDATE)
  {				/* We found a shorter path */
    rem_node(&en->cn);
  }
  en->dist = dist;
  en->color = CANDIDATE;

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
	  ((act->dist == dist) && (act->lsa.type == LSA_T_NET)))
      /* FIXME - shouldn't be here LSA_T_RT ??? */
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
match_dr(struct ospf_iface *ifa, struct top_hash_entry *en)
{
#ifdef OSPFv2
  return (ifa->drid == en->lsa.rt) && (ipa_to_u32(ifa->drip) == en->lsa.id);
#else /* OSPFv3 */
  return (ifa->drid == en->lsa.rt) && (ifa->dr_iface_id == en->lsa.id);
#endif
}

static int
calc_next_hop(struct ospf_area *oa, struct top_hash_entry *en,
	      struct top_hash_entry *par)
{
  // struct proto *p = &oa->po->proto;
  struct ospf_neighbor *neigh, *m;
  struct proto_ospf *po = oa->po;
  struct ospf_iface *ifa;

  /* 16.1.1. The next hop calculation */
  DBG("     Next hop called.\n");
  if (ipa_zero(par->nh))
  {
    u32 rid = en->lsa.rt;
    DBG("     Next hop calculating for id: %R rt: %R type: %u\n",
	en->lsa.id, en->lsa.rt, en->lsa.type);

    /* 
     * There are three cases:
     * 1) en is a local network (and par is root)
     * 2) en is a ptp or ptmp neighbor (and par is root)
     * 3) en is a bcast or nbma neighbor (and par is local network)
     */

    /* The first case - local network */
    if ((en->lsa.type == LSA_T_NET) && (par == oa->rt))
    {
      WALK_LIST(ifa, po->iface_list)
	if (match_dr(ifa, en))
	  {
	    en->nh = IPA_NONE;
	    en->nhi = ifa;
	    return 1;
	  }
      return 0;
    }

    /* The second case - ptp or ptmp neighbor */
    if ((en->lsa.type == LSA_T_RT) && (par == oa->rt))
    {
      /*
       * We don't know which iface was used to reach this neighbor
       * (there might be more parallel ifaces) so we will find
       * the best PTP iface with given fully adjacent neighbor.
       */
      neigh = NULL;
      WALK_LIST(ifa, po->iface_list)
	if ((ifa->type == OSPF_IT_PTP) || (ifa->type == OSPF_IT_VLINK))
	{
	  m = find_neigh(ifa, rid);
	  if (m && (m->state == NEIGHBOR_FULL))
	  {
	    if (!neigh || (m->ifa->cost < neigh->ifa->cost))
	      neigh = m;
	  }
	}

      if (!neigh)
	return 0;

      en->nh = neigh->ip;
      en->nhi = neigh->ifa;
      return 1;
    }

    /* The third case - bcast or nbma neighbor */
    if ((en->lsa.type == LSA_T_RT) && (par->lsa.type == LSA_T_NET))
    {
      /* par->nhi should be defined from parent's calc_next_hop() */
      if (!par->nhi)
	goto bad;

#ifdef OSPFv2
      /*
       * In this case, next-hop is the same as link-back, which is
       * already computed in link_back().
       */
      if (ipa_zero(en->lb))
	goto bad;

      en->nh = en->lb;
      en->nhi = par->nhi;
      return 1;

#else /* OSPFv3 */
      /*
       * Next-hop is taken from lladdr field of Link-LSA, en->lb_id
       * is computed in link_back().
       */
      struct top_hash_entry *lhe;
      lhe = ospf_hash_find(po->gr, par->nhi->iface->index, en->lb_id, rid, LSA_T_LINK);

      if (!lhe)
	return 0;

      struct ospf_lsa_link *llsa = lhe->lsa_body;
      
      if (ipa_zero(llsa->lladdr))
	return 0;

      en->nh = llsa->lladdr;
      en->nhi = par->nhi;
      return 1;
#endif
    }

  bad:
    /* Probably bug or some race condition, we log it */
    log(L_ERR "Unexpected case in next hop calculation");
    return 0;
  }

  en->nh = par->nh;
  en->nhi = par->nhi;
  return 1;
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

    /* Sanity check of next-hop address */
    if (nf->n.type && ipa_nonzero(nf->n.nh))
    {
      neighbor *ng = neigh_find2(p, &nf->n.nh, nf->n.ifa->iface, 0);
      if (!ng || (ng->scope == SCOPE_HOST))
	reset_ri(&nf->n);
    }

    if (po->areano > 1)
      check_sum_net_lsa(po, nf);

    /* Remove configured stubnets */
    if (!nf->n.ifa)
      reset_ri(&nf->n);

    if (reload || memcmp(&nf->n, &nf->o, sizeof(orta)))
    {
      net *ne = net_get(p->table, nf->fn.prefix, nf->fn.pxlen);

      if (nf->n.type) /* Add the route */
      {
	rta a0 = {
	  .proto = p,
	  .source = nf->n.type,
	  .scope = SCOPE_UNIVERSE,
	  .cast = RTC_UNICAST,
	  .iface = nf->n.ifa->iface
	};

	if (ipa_nonzero(nf->n.nh))
	{
	  a0.dest = RTD_ROUTER;
	  a0.gw = nf->n.nh;
	}
	else
	  a0.dest = RTD_DEVICE;

	rte *e = rte_get_temp(&a0);
	e->u.ospf.metric1 = nf->n.metric1;
	e->u.ospf.metric2 = nf->n.metric2;
	e->u.ospf.tag = nf->n.tag;
	e->u.ospf.router_id = nf->n.rid;
	e->pflags = 0;
	e->net = ne;
	e->pref = p->preference;
	DBG("Mod rte type %d - %I/%d via %I on iface %s, met %d\n",
	    a0.source, nf->fn.prefix, nf->fn.pxlen, a0.gw, a0.iface ? a0.iface->name : "(none)", nf->n.metric1);
	rte_update(p->table, ne, p, p, e);
      }
      else /* Remove the route */
	rte_update(p->table, ne, p, p, NULL);
    }

    /* Remove unused rt entry. Entries with fn.x0 == 1 are persistent. */
    if (!nf->n.type && !nf->fn.x0)
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
