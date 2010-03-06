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
static void ospf_ext_spf(struct proto_ospf *po);
static void rt_sync(struct proto_ospf *po);

/* In ospf_area->rtr we store paths to routers, but we use RID (and not IP address)
   as index, so we need to encapsulate RID to IP address */
#ifdef OSPFv2
#define ipa_from_rid(x) _MI(x)
#else /* OSPFv3 */
#define ipa_from_rid(x) _MI(0,0,0,x)
#endif


static void
fill_ri(orta * orta)
{
  orta->type = RTS_DUMMY;
  orta->options = 0;
  orta->oa = NULL;
  orta->metric1 = LSINFINITY;
  orta->metric2 = LSINFINITY;
  orta->nh = IPA_NONE;
  orta->ifa = NULL;
  orta->ar = NULL;
  orta->tag = 0;
  orta->rid = 0;
}

void
ospf_rt_initort(struct fib_node *fn)
{
  ort *ri = (ort *) fn;
  fill_ri(&ri->n);
  memcpy(&ri->o, &ri->n, sizeof(orta));
  ri->efn = NULL;
}

/* If new is better return 1 */

/* 
 * This is hard to understand:
 * If rfc1583 is set to 1, it work likes normal route_better()
 * But if it is set to 0, it prunes number of AS boundary
 * routes before it starts the router decision
 */
static int
ri_better(struct proto_ospf *po, orta * new, ort *nefn, orta * old, ort *oefn, int rfc1583)
{
  int newtype = new->type;
  int oldtype = old->type;

  if (old->type == RTS_DUMMY)
    return 1;

  if (old->metric1 == LSINFINITY)
    return 1;

  if (!rfc1583)
  {
    if ((new->type < RTS_OSPF_EXT1) && (new->oa->areaid == 0)) newtype = RTS_OSPF_IA;
    if ((old->type < RTS_OSPF_EXT1) && (old->oa->areaid == 0)) oldtype = RTS_OSPF_IA;
  }

  if (newtype < oldtype)
    return 1;

  if (newtype > oldtype)
    return 0;

  /* Same type */
  if (new->type == RTS_OSPF_EXT2)
  {
    if (new->metric2 < old->metric2) return 1;
    if (new->metric2 > old->metric2) return 0;
  }

  if (((new->type == RTS_OSPF_EXT2) || (new->type == RTS_OSPF_EXT1)) && (!po->rfc1583))
  {
    newtype = nefn->n.type;
    oldtype = oefn->n.type;

    if (nefn->n.oa->areaid == 0) newtype = RTS_OSPF_IA;
    if (oefn->n.oa->areaid == 0) oldtype = RTS_OSPF_IA;

    if (newtype < oldtype) return 1;
    if (newtype > oldtype) return 0;
  }

  if (new->metric1 < old->metric1)
    return 1;

  if (new->metric1 > old->metric1)
    return 0;

  if (new->oa->areaid > old->oa->areaid) return 1;	/* Larger AREAID is preffered */

  return 0;			/* Old is shorter or same */
}

static void
ri_install(struct proto_ospf *po, ip_addr prefix, int pxlen, int dest,
	   orta * new, ort * ipath)
{
  struct ospf_area *oa = new->oa;
  ort *old;

  if (dest == ORT_NET)
  {
    struct area_net *anet;
    old = (ort *) fib_get(&po->rtf, &prefix, pxlen);
    if (ri_better(po, new, ipath, &old->n, old->efn, 1))
    {
      memcpy(&old->n, new, sizeof(orta));
      old->efn = ipath;
      if ((new->type == RTS_OSPF) && (anet = (struct area_net *)fib_route(&oa->net_fib, prefix, pxlen)))
      {
        anet->active = 1;
        if (new->metric1 > anet->metric) anet->metric = new->metric1;
      }
    }
  }
  else
  {
    old = (ort *) fib_get(&oa->rtr, &prefix, pxlen);

    if (ri_better(po, new, ipath, &old->n, old->efn, 1))
    {
      memcpy(&old->n, new, sizeof(orta));
      old->efn = ipath;
    }
  }
}

static void
add_network(struct ospf_area *oa, ip_addr px, int pxlen, int metric, struct top_hash_entry *en)
{
  orta nf;
  nf.type = RTS_OSPF;
  nf.options = 0;
  nf.metric1 = metric;
  nf.metric2 = LSINFINITY;
  nf.tag = 0;
  nf.oa = oa;
  nf.ar = en;
  nf.nh = en->nh;
  nf.ifa = en->nhi;
  nf.rid = en->lsa.rt;

  /* FIXME check nf.ifa on stubs */
  ri_install(oa->po, px, pxlen, ORT_NET, &nf, NULL);
}

#ifdef OSPFv3
static void
process_prefixes(struct ospf_area *oa)
{
  struct proto_ospf *po = oa->po;
  struct proto *p = &po->proto;
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

    if (src->lsa.age == LSA_MAXAGE)
      continue;

    if ((src->lsa.type != LSA_T_RT) && (src->lsa.type != LSA_T_NET))
      continue;

    buf = px->rest;
    for (i = 0; i < px->pxcount; i++)
      {
	buf = lsa_get_ipv6_prefix(buf, &pxa, &pxlen, &pxopts, &metric);

	if (pxopts & OPT_PX_NU)
	  continue;

	add_network(oa, pxa, pxlen, src->dist + metric, src);
      }
  }
}
#endif


static void
ospf_rt_spfa_rtlinks(struct ospf_area *oa, struct top_hash_entry *act, struct top_hash_entry *en)
{
  struct proto *p = &oa->po->proto;
  struct proto_ospf *po = oa->po;
  orta nf;
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
	   * This violates rfc2328! But it is mostly harmless.
	   */
	  DBG("\n");

	  nf.type = RTS_OSPF;
	  nf.options = 0;
	  nf.metric1 = act->dist + rtl->metric;
	  nf.metric2 = LSINFINITY;
	  nf.tag = 0;
	  nf.oa = oa;
	  nf.ar = act;
	  nf.nh = act->nh;
	  nf.ifa = act->nhi;
	  nf.rid = act->lsa.rt;

	  if (act == oa->rt)
	    {
	      struct ospf_iface *iff;

	      WALK_LIST(iff, po->iface_list)	/* Try to find corresponding interface */
		{
		  if (iff->iface && (iff->type != OSPF_IT_VLINK) &&
		      (rtl->id == (ipa_to_u32(ipa_mkmask(iff->iface->addr->pxlen))
				   & ipa_to_u32(iff->iface->addr->prefix))))	/* No VLINK and IP must match */
		    {
		      nf.ifa = iff;
		      break;
		    }
		}
	    }

	  if (!nf.ifa)
	    continue;

	  ri_install(po, ipa_from_u32(rtl->id),
		     ipa_mklen(ipa_from_u32(rtl->data)), ORT_NET, &nf, NULL);
	  break;
#endif

	case LSART_NET:
#ifdef OSPFv2
	  /* In OSPFv2, rtl->id is IP addres of DR, Router ID is not known */
	  tmp = ospf_hash_find_net(po->gr, oa->areaid, rtl->id);
#else /* OSPFv3 */
	  tmp = ospf_hash_find(po->gr, oa->areaid, rtl->nif, rtl->id, LSA_T_NET);
#endif
	  if (tmp == NULL)
	    DBG("Not found!\n");
	  else
	    DBG("Found. :-)\n");
	  break;

	case LSART_VLNK:
	case LSART_PTP:
	  tmp = ospf_hash_find_rt(po->gr, oa->areaid, rtl->id);
	  DBG("PTP found.\n");
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

/* 16.1 calculating shortest paths for an area */
static void
ospf_rt_spfa(struct ospf_area *oa)
{
  struct proto *p = &oa->po->proto;
  struct proto_ospf *po = oa->po;
  struct ospf_lsa_rt *rt;
  struct ospf_lsa_net *ln;
  struct ospf_iface *iface;
  struct top_hash_entry *act, *tmp;
  u32 i, *rts;
  orta nf;
  node *n;

  if (oa->rt == NULL)
    return;

  OSPF_TRACE(D_EVENTS, "Starting routing table calculation for area %R", oa->areaid);

  if (oa->rt->dist != LSINFINITY)
    bug("Aging was not processed.");

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

      /* In OSPFv2, just ASBRs and ABRs are needed to add to oa->rtr table */
      // ((rt->options & OPT_RT_V) || (rt->options & OPT_RT_E))

      nf.type = RTS_OSPF;
      nf.options = rt->options;
      nf.metric1 = act->dist;
      nf.metric2 = LSINFINITY;
      nf.tag = 0;
      nf.oa = oa;
      nf.ar = act;
      nf.nh = act->nh;
      nf.ifa = act->nhi;
      nf.rid = act->lsa.rt;
      ri_install(po, ipa_from_rid(act->lsa.rt), MAX_PREFIX_LENGTH, ORT_ROUTER, &nf, NULL);

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

  /* Find new/lost VLINK peers */
  WALK_LIST(iface, po->iface_list)
  {
    if ((iface->type == OSPF_IT_VLINK) && (iface->voa == oa))
    {
      if ((tmp = ospf_hash_find_rt(po->gr, oa->areaid, iface->vid)) &&
	  (!ipa_equal(tmp->lb, IPA_NONE)))
      {
        if ((iface->state != OSPF_IS_PTP) || (iface->iface != tmp->nhi->iface) || (!ipa_equal(iface->vip, tmp->lb)))
        {
          OSPF_TRACE(D_EVENTS, "Vlink peer %R found", tmp->lsa.id);
          ospf_iface_sm(iface, ISM_DOWN);
          iface->iface = tmp->nhi->iface;
          iface->vip = tmp->lb;
          ospf_iface_sm(iface, ISM_UP);
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

  // FIXME lb should be properly set for vlinks */
  en->lb = IPA_NONE;
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

/* 16.3 examining summary-LSAs in transit areas */
static void
ospf_rt_sum_tr(struct ospf_area *oa)
{
  struct proto *p = &oa->po->proto;
  struct proto_ospf *po = oa->po;
  struct ospf_area *bb = po->backbone;
  ip_addr ip, abrip;
  struct top_hash_entry *en;
  u32 dst_rid, metric, options;
  int pxlen = -1, type = -1;
  ort *re = NULL, *abr;
  orta nf;

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

    // if (en->dist == LSINFINITY)
    //   continue;

    /* 16.3 (2) */
    if (en->lsa.rt == po->router_id)
      continue;

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
      re = (ort *) fib_find(&po->rtf, &ip, pxlen);
    }
    else if (en->lsa.type == LSA_T_SUM_RT)
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

      ip = ipa_from_rid(dst_rid);
      pxlen = MAX_PREFIX_LENGTH;
      metric = ls->metric & METRIC_MASK;
      options |= ORTA_ASBR;
      type = ORT_ROUTER;
      re = (ort *) fib_find(&bb->rtr, &ip, pxlen);
    }

    /* 16.3 (1b) */ 
    if (metric == LSINFINITY) 
      continue; 

    /* 16.3 (3) */
    if (!re) continue;
    if (re->n.oa->areaid != 0) continue;
    if ((re->n.type != RTS_OSPF) && (re->n.type != RTS_OSPF_IA)) continue;

    /* 16.3. (4) */
    abrip = ipa_from_rid(en->lsa.rt);
    abr = fib_find(&oa->rtr, &abrip, MAX_PREFIX_LENGTH);
    if (!abr) continue;

    nf.type = re->n.type;
    nf.options = options;
    nf.metric1 = abr->n.metric1 + metric;
    nf.metric2 = LSINFINITY;
    nf.tag = 0;
    nf.oa = oa;
    nf.ar = abr->n.ar;
    nf.nh = abr->n.nh;
    nf.ifa = abr->n.ifa;
    nf.rid = en->lsa.rt; /* ABR ID */
    ri_install(po, ip, pxlen, type, &nf, NULL);
  }
}
  
/* 16.2 calculating inter-area routes */
static void
ospf_rt_sum(struct ospf_area *oa)
{
  struct proto_ospf *po = oa->po;
  struct proto *p = &po->proto;
  struct top_hash_entry *en;
  ip_addr ip, abrip;	/* abrIP is actually ID */
  u32 dst_rid, metric, options;
  struct area_net *anet;
  orta nf;
  ort *abr;
  int pxlen = -1, type = -1;

  OSPF_TRACE(D_EVENTS, "Starting routing table calculation for inter-area (area %R)", oa->areaid);

  WALK_SLIST(en, po->lsal)
  {
    if ((en->lsa.type != LSA_T_SUM_RT) && (en->lsa.type != LSA_T_SUM_NET))
      continue;

    if (en->domain != oa->areaid)
      continue;

    /* Page 169 (1) */
    if (en->lsa.age == LSA_MAXAGE)
      continue;

    /* Page 169 (2) */
    if (en->lsa.rt == po->router_id)
      continue;


    if (en->lsa.type == LSA_T_SUM_NET)
    {
      struct ospf_area *oaa;
      int skip = 0;

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

      /* Page 169 (3) */
      WALK_LIST(oaa, po->area_list)
      {
        if ((anet = fib_find(&oaa->net_fib, &ip, pxlen)) && anet->active)
	{
	  skip = 1;
	  break;
	}
      }
      if (skip) continue;
    }
    else
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

      ip = ipa_from_rid(dst_rid);
      pxlen = MAX_PREFIX_LENGTH;
      metric = ls->metric & METRIC_MASK;
      options |= ORTA_ASBR;
      type = ORT_ROUTER;
    }

    /* Page 169 (1) */
    if (metric == LSINFINITY)
      continue;

    /* Page 169 (4) */
    abrip = ipa_from_rid(en->lsa.rt);
    if (!(abr = (ort *) fib_find(&oa->rtr, &abrip, MAX_PREFIX_LENGTH))) continue;
    if (abr->n.metric1 == LSINFINITY) continue;
    if (!(abr->n.options & ORTA_ABR)) continue;

    nf.type = RTS_OSPF_IA;
    nf.options = options;
    nf.metric1 = abr->n.metric1 + metric;
    nf.metric2 = LSINFINITY;
    nf.tag = 0;
    nf.oa = oa;
    nf.ar = abr->n.ar;
    nf.nh = abr->n.nh;
    nf.ifa = abr->n.ifa;
    nf.rid = en->lsa.rt; /* ABR ID */
    ri_install(po, ip, pxlen, type, &nf, NULL);
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
  ort *ri;
  struct area_net *anet;

  if (po->areano == 0) return;

  po->cleanup = 1;

  OSPF_TRACE(D_EVENTS, "Starting routing table calculation");

  /* 16. (1) - Invalidate old routing table */
  FIB_WALK(&po->rtf, nftmp)
  {
    ri = (ort *) nftmp;
    memcpy(&ri->o, &ri->n, sizeof(orta));	/* Backup old data */
    fill_ri(&ri->n);
  }
  FIB_WALK_END;


  WALK_LIST(oa, po->area_list)
  {
    FIB_WALK(&oa->rtr, nftmp)
    {
      ri = (ort *) nftmp;
      memcpy(&ri->o, &ri->n, sizeof(orta));	/* Backup old data */
      fill_ri(&ri->n);
    }
    FIB_WALK_END;

    FIB_WALK(&oa->net_fib, nftmp)
    {
      anet = (struct area_net *) nftmp;
      anet->active = 0;
      anet->metric = 1;
    }
    FIB_WALK_END;

    /* 16. (2) */
    ospf_rt_spfa(oa);
  }

  /* 16. (3) */
  if ((po->areano == 1) || (!po->backbone))
  {
    ospf_rt_sum(HEAD(po->area_list));
  }
  else
  {
    ospf_rt_sum(po->backbone);
  }

  /* 16. (4) */
  WALK_LIST(oa, po->area_list)
  {
    if (oa->trcap && (oa->areaid != 0))
    {
      ospf_rt_sum_tr(oa);
      break;
    }
  }

  /* 16. (5) */
  ospf_ext_spf(po);

  rt_sync(po);

  po->calcrt = 0;
}

/**
 * ospf_ext_spf - calculate external paths
 * @po: protocol
 *
 * After routing table for any area is calculated, calculation of external
 * path is invoked. This process is described in 16.4 of RFC 2328.
 * Inter- and Intra-area paths are always prefered over externals.
 */
static void
ospf_ext_spf(struct proto_ospf *po)
{
  ort *nf1, *nf2, *nfh;
  orta nfa;
  struct top_hash_entry *en;
  struct proto *p = &po->proto;
  struct ospf_lsa_ext *le;
  int pxlen, ebit, rt_fwaddr_valid;
  ip_addr ip, nh, rtid, rt_fwaddr;
  struct ospf_iface *nhi = NULL;
  u32 br_metric, rt_metric, rt_tag;
  neighbor *nn;
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
    rtid = ipa_from_rid(en->lsa.rt);
    nf1 = NULL;
    WALK_LIST(atmp, po->area_list)
    {
      nfh = fib_find(&atmp->rtr, &rtid, MAX_PREFIX_LENGTH);
      if (nfh == NULL) continue;
      if (nf1 == NULL) nf1 = nfh;
      else if (ri_better(po, &nfh->n, NULL, &nf1->n, NULL, po->rfc1583)) nf1 = nfh;
    }

    if (!nf1)
      continue;			/* No AS boundary router found */

    if (nf1->n.metric1 == LSINFINITY)
      continue;			/* distance is INF */

    if (!(nf1->n.options & ORTA_ASBR))
      continue;			/* It is not ASBR */

    if (!rt_fwaddr_valid)
    {
      nh = nf1->n.nh;
      nhi = nf1->n.ifa;
      nfh = nf1;
      br_metric = nf1->n.metric1;
    }
    else
    {
      nf2 = fib_route(&po->rtf, rt_fwaddr, MAX_PREFIX_LENGTH);

      if (!nf2)
      {
	DBG("Cannot find network route (GW=%I)\n", rt_fwaddr);
	continue;
      }

      if ((nn = neigh_find(p, &rt_fwaddr, 0)) != NULL)
      {
	nh = rt_fwaddr;
	nhi = ospf_iface_find(po, nn->iface);
      }
      else
      {
	nh = nf2->n.nh;
	nhi = nf2->n.ifa;
      }

      br_metric = nf2->n.metric1;
      if (br_metric == LSINFINITY)
        continue;			/* distance is INF */
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

    nfa.options = 0;
    nfa.tag = rt_tag;
    nfa.oa = (po->backbone == NULL) ? HEAD(po->area_list) : po->backbone;
    nfa.ar = nf1->n.ar;
    nfa.nh = nh;
    nfa.ifa = nhi;
    nfa.rid = en->lsa.rt;
    ri_install(po, ip, pxlen, ORT_NET, &nfa, nfh);
  }

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

  /* 16.1. (2d) */
  if (dist >= en->dist)
    return;
  /*
   * FIXME The line above (=) is not a bug, but we don't support multiple
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
  struct ospf_neighbor *neigh;
  struct proto *p = &oa->po->proto;
  struct proto_ospf *po = oa->po;
  struct ospf_iface *ifa;

  /* 16.1.1. The next hop calculation */
  DBG("     Next hop called.\n");
  if (ipa_equal(par->nh, IPA_NONE))
  {
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

    /* 
     * Remaining cases - local neighbours.
     * There are two problems with this code:
     * 1) we use IP address from HELLO packet
     *    and not the one from LSA (router or link).
     *    This may break NBMA networks
     * 2) we use find_neigh_noifa() and does not
     *    take into account associated iface.
     *    This breaks neighbors connected by more links.
     */

    if ((en->lsa.type == LSA_T_RT) && 
	((par == oa->rt) || (par->lsa.type == LSA_T_NET)))
    {
      if ((neigh = find_neigh_noifa(po, en->lsa.rt)) != NULL)
	{
	  en->nh = neigh->ip;
	  en->nhi = neigh->ifa;
	  return 1;
	}
      return 0;
    }

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
  struct ospf_area *oa, *oaa;
  struct area_net *anet;
  int flush;

  /* This is used for forced reload of routes */
  int reload = (po->calcrt == 2);

  OSPF_TRACE(D_EVENTS, "Starting routing table synchronisation");

  DBG("Now syncing my rt table with nest's\n");
  FIB_ITERATE_INIT(&fit, fib);
again1:
  FIB_ITERATE_START(fib, &fit, nftmp)
  {
    nf = (ort *) nftmp;
    check_sum_lsa(po, nf, ORT_NET);
    if (reload || memcmp(&nf->n, &nf->o, sizeof(orta)))
    {				/* Some difference */
      net *ne;
      rta a0;
      rte *e;

      bzero(&a0, sizeof(a0));

      a0.proto = p;
      a0.source = nf->n.type;
      a0.scope = SCOPE_UNIVERSE;
      a0.cast = RTC_UNICAST;
      a0.dest = RTD_ROUTER;
      a0.flags = 0;
      a0.aflags = 0;
      a0.iface = NULL;
      if (nf->n.ifa) a0.iface = nf->n.ifa->iface;
      a0.gw = nf->n.nh;

      if (ipa_nonzero(nf->n.nh) && (!neigh_find2(p, &nf->n.nh, nf->n.ifa->iface, 0)))
      {
        int found = 0;
        struct ospf_iface *ifa;
        struct top_hash_entry *en;
        OSPF_TRACE(D_EVENTS, "Trying to find correct next hop %I/%d via %I", nf->fn.prefix, nf->fn.pxlen, nf->n.nh);
        WALK_LIST(ifa, po->iface_list)
        {
          if ((ifa->type == OSPF_IT_VLINK) && ipa_equal(ifa->vip, nf->n.nh))
          {
            if ((en = ospf_hash_find_rt(po->gr, ifa->voa->areaid, ifa->vid))
		&& (!ipa_equal(en->nh, IPA_NONE)))
            {
              a0.gw = en->nh;
              found = 1;
            }
            break;
          }
        }
        if (!found) nf->n.metric1 = LSINFINITY; /* Delete it */
      }

      if (ipa_equal(nf->n.nh, IPA_NONE)) a0.dest = RTD_DEVICE;

      if (!a0.iface)	/* Still no match? Can this really happen? */
        nf->n.metric1 = LSINFINITY;

      ne = net_get(p->table, nf->fn.prefix, nf->fn.pxlen);
      if (nf->n.metric1 < LSINFINITY)
      {
        e = rte_get_temp(&a0);
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
      else
      {
        rte_update(p->table, ne, p, p, NULL);
        FIB_ITERATE_PUT(&fit, nftmp);
        fib_delete(fib, nftmp);
        goto again1;
      }
    }
  }
  FIB_ITERATE_END(nftmp);

  WALK_LIST(oa, po->area_list)
  {
    FIB_ITERATE_INIT(&fit, &oa->rtr);
again2:
    FIB_ITERATE_START(&oa->rtr, &fit, nftmp)
    {
      nf = (ort *) nftmp;
      if (memcmp(&nf->n, &nf->o, sizeof(orta)))
      {				/* Some difference */
        check_sum_lsa(po, nf, ORT_ROUTER);
        if (nf->n.metric1 >= LSINFINITY)
        {
          FIB_ITERATE_PUT(&fit, nftmp);
          fib_delete(&oa->rtr, nftmp);
          goto again2;
        }
      }
    }
    FIB_ITERATE_END(nftmp);

    /* Check condensed summary LSAs */
    FIB_WALK(&oa->net_fib, nftmp)
    {
      flush = 1;
      anet = (struct area_net *) nftmp;
      if ((!anet->hidden) && anet->active)
        flush = 0;
          
      WALK_LIST(oaa, po->area_list)
      {
        int fl = flush;

        if (oaa == oa) continue;

	if ((oa == po->backbone) && oaa->trcap) fl = 1;

	if (oaa->stub) fl = 1;

        if (fl) flush_sum_lsa(oaa, &anet->fn, ORT_NET);
        else originate_sum_lsa(oaa, &anet->fn, ORT_NET, anet->metric, 0);
      }
    }
    FIB_WALK_END;

    /* Check default summary LSA for stub areas
     * just for router connected to backbone */
    if (po->backbone)
    {
      struct fib_node fnn;

      fnn.prefix = IPA_NONE;
      fnn.pxlen = 0;
      if (oa->stub) originate_sum_lsa(oa, &fnn, ORT_NET, oa->stub, 0);
      else flush_sum_lsa(oa, &fnn, ORT_NET);
    }
  }
}
