/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Open Shortest Path First (OSPF)
 * 
 * The OSPF protocol is quite complicated and its complex implemenation is
 * split to many files. In |ospf.c|, you will find mainly the interface
 * for communication with the core (e.g., reconfiguration hooks, shutdown
 * and initialisation and so on). In |packet.c|, you will find various
 * functions for sending and receiving generic OSPF packets. There are
 * also routines for authentication and checksumming. File |iface.c| contains
 * the interface state machine and functions for allocation and deallocation of OSPF's
 * interface data structures. Source |neighbor.c| includes the neighbor state
 * machine and functions for election of Designated Router and Backup
 * Designated router. In |hello.c|, there are routines for sending
 * and receiving of hello packets as well as functions for maintaining
 * wait times and the inactivity timer. Files |lsreq.c|, |lsack.c|, |dbdes.c|
 * contain functions for sending and receiving of link-state requests,
 * link-state acknowledgements and database descriptions respectively.
 * In |lsupd.c|, there are functions for sending and receiving
 * of link-state updates and also the flooding algorithm. Source |topology.c| is
 * a place where routines for searching LSAs in the link-state database,
 * adding and deleting them reside, there also are functions for originating
 * of various types of LSAs (router LSA, net LSA, external LSA). File |rt.c|
 * contains routines for calculating the routing table. |lsalib.c| is a set
 * of various functions for working with the LSAs (endianity conversions,
 * calculation of checksum etc.).
 *
 * One instance of the protocol is able to hold LSA databases for
 * multiple OSPF areas, to exchange routing information between
 * multiple neighbors and to calculate the routing tables. The core
 * structure is &proto_ospf to which multiple &ospf_area and
 * &ospf_iface structures are connected. &ospf_area is also connected to
 * &top_hash_graph which is a dynamic hashing structure that
 * describes the link-state database. It allows fast search, addition
 * and deletion. Each LSA is kept in two pieces: header and body. Both of them are
 * kept in the endianity of the CPU.
 *
 * The heart beat of ospf is ospf_disp(). It is called at regular intervals
 * (&proto_ospf->tick). It is responsible for aging and flushing of LSAs in
 * the database, for routing table calculaction and it call area_disp() of every
 * ospf_area.
 * 
 * The function area_disp() is
 * responsible for late originating of router LSA and network LSA
 * and for cleanup after routing table calculation process in
 * the area.
 * To every &ospf_iface, we connect one or more
 * &ospf_neighbor's -- a structure containing many timers and queues
 * for building adjacency and for exchange of routing messages.
 *
 * BIRD's OSPF implementation respects RFC2328 in every detail, but
 * some of internal algorithms do differ. The RFC recommends making a snapshot
 * of the link-state database when a new adjacency is forming and sending
 * the database description packets based on the information in this 
 * snapshot. The database can be quite large in some networks, so
 * rather we walk through a &slist structure which allows us to
 * continue even if the actual LSA we were working with is deleted. New
 * LSAs are added at the tail of this &slist.
 *
 * We also don't keep a separate OSPF routing table, because the core
 * helps us by being able to recognize when a route is updated
 * to an identical one and it suppresses the update automatically.
 * Due to this, we can flush all the routes we've recalculated and
 * also those we've deleted to the core's routing table and the
 * core will take care of the rest. This simplifies the process
 * and conserves memory.
 */

#include <stdlib.h>
#include "ospf.h"


static void ospf_rt_notify(struct proto *p, net * n, rte * new, rte * old UNUSED, ea_list * attrs);
static void ospf_ifa_notify(struct proto *p, unsigned flags, struct ifa *a);
static int ospf_rte_better(struct rte *new, struct rte *old);
static int ospf_rte_same(struct rte *new, struct rte *old);
static void ospf_disp(timer *timer);

static void
ospf_area_initfib(struct fib_node *fn)
{
  struct area_net *an = (struct area_net *) fn;
  an->hidden = 0;
  an->active = -1;	/* Force to regenerate summary lsa */
	/* ac->oldactive will be rewritten by ospf_rt_spf() */
}

static void
add_area_nets(struct ospf_area *oa, struct ospf_area_config *ac)
{
    struct proto_ospf *po = oa->po;
    struct proto *p = &po->proto;
    struct area_net_config *anet;
    struct area_net *antmp;

    fib_init(&oa->net_fib, p->pool, sizeof(struct area_net), 16, ospf_area_initfib);

    WALK_LIST(anet, ac->net_list)
    {
      antmp = (struct area_net *) fib_get(&oa->net_fib, &anet->px.addr, anet->px.len);
      antmp->hidden = anet->hidden;
    }
}

static int
ospf_start(struct proto *p)
{
  struct proto_ospf *po = (struct proto_ospf *) p;
  struct ospf_config *c = (struct ospf_config *) (p->cf);
  struct ospf_area_config *ac;
  struct ospf_area *oa;

  po->rfc1583 = c->rfc1583;
  po->ebit = 0;

  po->tick = c->tick;
  po->disp_timer = tm_new(p->pool);
  po->disp_timer->data = po;
  po->disp_timer->randomize = 0;
  po->disp_timer->hook = ospf_disp;
  po->disp_timer->recurrent = po->tick;
  tm_start(po->disp_timer, 1);
  po->lsab_size = 256;
  po->lsab_used = 0;
  po->lsab = mb_alloc(p->pool, po->lsab_size);
  init_list(&(po->iface_list));
  init_list(&(po->area_list));
  fib_init(&po->rtf, p->pool, sizeof(ort), 16, ospf_rt_initort);
  po->areano = 0;
  po->gr = ospf_top_new(p->pool);
  po->cleanup = 1;
  s_init_list(&(po->lsal));
  if (EMPTY_LIST(c->area_list))
  {
    log(L_ERR "Cannot start, no OSPF areas configured!");
    return PS_DOWN;
  }

  WALK_LIST(ac, c->area_list)
  {
    oa = mb_allocz(p->pool, sizeof(struct ospf_area));
    add_tail(&po->area_list, NODE oa);
    po->areano++;
    oa->ac = ac;
    oa->stub = ac->stub;
    oa->areaid = ac->areaid;
    oa->rt = NULL;
    oa->po = po;
    add_area_nets(oa, ac);
    fib_init(&oa->rtr, p->pool, sizeof(ort), 16, ospf_rt_initort);

    if (oa->areaid == 0)
    {
      po->backbone = oa;
      if (oa->stub) log(L_ERR "Backbone cannot be stub. Ignoring!");
      oa->stub = 0;
    }

    oa->opt.byte = 0;
    if(!oa->stub) oa->opt.bit.e = 1;
  }

  /* Add all virtual links as interfaces */
  {
    struct ospf_iface_patt *ipatt;
    WALK_LIST(ac, c->area_list)
    {
      WALK_LIST(ipatt, ac->vlink_list)
      {
        if(!po->backbone)
	{
          oa = mb_allocz(p->pool, sizeof(struct ospf_area));
          add_tail(&po->area_list, NODE oa);
          po->areano++;
          oa->stub = 0;
          oa->areaid = 0;
          oa->rt = NULL;
          oa->po = po;
	  fib_init(&oa->net_fib, p->pool, sizeof(struct area_net), 16, ospf_area_initfib);
          fib_init(&oa->rtr, p->pool, sizeof(ort), 16, ospf_rt_initort);
          po->backbone = oa;
          oa->opt.byte = 0;
          oa->opt.bit.e = 1;
	}
        ospf_iface_new(po, NULL, ac, ipatt);
      }
    }
  }
  return PS_UP;
}

static void
ospf_dump(struct proto *p)
{
  struct ospf_iface *ifa;
  struct ospf_neighbor *n;
  struct proto_ospf *po = (struct proto_ospf *) p;

  OSPF_TRACE(D_EVENTS, "Area number: %d", po->areano);

  WALK_LIST(ifa, po->iface_list)
  {
    OSPF_TRACE(D_EVENTS, "Interface: %s", (ifa->iface ? ifa->iface->name : "(null)"));
    OSPF_TRACE(D_EVENTS, "state: %u", ifa->state);
    OSPF_TRACE(D_EVENTS, "DR:  %R", ifa->drid);
    OSPF_TRACE(D_EVENTS, "BDR: %R", ifa->bdrid);
    WALK_LIST(n, ifa->neigh_list)
    {
      OSPF_TRACE(D_EVENTS, "  neighbor %R in state %u", n->rid, n->state);
    }
  }

  OSPF_TRACE(D_EVENTS, "LSA graph dump start:");
  ospf_top_dump(po->gr, p);
  OSPF_TRACE(D_EVENTS, "LSA graph dump finished");
  neigh_dump_all();
}

static struct proto *
ospf_init(struct proto_config *c)
{
  struct proto *p = proto_new(c, sizeof(struct proto_ospf));

  p->import_control = ospf_import_control;
  p->make_tmp_attrs = ospf_make_tmp_attrs;
  p->store_tmp_attrs = ospf_store_tmp_attrs;
  p->accept_ra_types = RA_OPTIMAL;
  p->rt_notify = ospf_rt_notify;
  p->if_notify = ospf_iface_notify;
  p->ifa_notify = ospf_ifa_notify;
  p->rte_better = ospf_rte_better;
  p->rte_same = ospf_rte_same;

  return p;
}

/* If new is better return 1 */
static int
ospf_rte_better(struct rte *new, struct rte *old)
{
  if (new->u.ospf.metric1 == LSINFINITY)
    return 0;

  if(new->attrs->source < old->attrs->source) return 1;
  if(new->attrs->source > old->attrs->source) return 0;

  if(new->attrs->source == RTS_OSPF_EXT2)
  {
    if(new->u.ospf.metric2 < old->u.ospf.metric2) return 1;
    if(new->u.ospf.metric2 > old->u.ospf.metric2) return 0;
  }

  if (new->u.ospf.metric1 < old->u.ospf.metric1)
    return 1;

  return 0;			/* Old is shorter or same */
}

static int
ospf_rte_same(struct rte *new, struct rte *old)
{
  /* new->attrs == old->attrs always */
  return
    new->u.ospf.metric1 == old->u.ospf.metric1 &&
    new->u.ospf.metric2 == old->u.ospf.metric2 &&
    new->u.ospf.tag == old->u.ospf.tag;
}

static ea_list *
ospf_build_attrs(ea_list * next, struct linpool *pool, u32 m1, u32 m2,
		 u32 tag)
{
  struct ea_list *l =
    lp_alloc(pool, sizeof(struct ea_list) + 3 * sizeof(eattr));

  l->next = next;
  l->flags = EALF_SORTED;
  l->count = 3;
  l->attrs[0].id = EA_OSPF_METRIC1;
  l->attrs[0].flags = 0;
  l->attrs[0].type = EAF_TYPE_INT | EAF_TEMP;
  l->attrs[0].u.data = m1;
  l->attrs[1].id = EA_OSPF_METRIC2;
  l->attrs[1].flags = 0;
  l->attrs[1].type = EAF_TYPE_INT | EAF_TEMP;
  l->attrs[1].u.data = m2;
  l->attrs[2].id = EA_OSPF_TAG;
  l->attrs[2].flags = 0;
  l->attrs[2].type = EAF_TYPE_INT | EAF_TEMP;
  l->attrs[2].u.data = tag;
  return l;
}

void
schedule_net_lsa(struct ospf_iface *ifa)
{
  ifa->orignet = 1;
}

void
schedule_rt_lsa(struct ospf_area *oa)
{
  struct proto *p = &oa->po->proto;

  OSPF_TRACE(D_EVENTS, "Scheduling RT lsa origination for area %R.", oa->areaid);
  oa->origrt = 1;
}

void
schedule_rtcalc(struct proto_ospf *po)
{
  struct proto *p = &po->proto;

  if (po->calcrt)
    return;

  OSPF_TRACE(D_EVENTS, "Scheduling RT calculation.");
  po->calcrt = 1;
}

/**
 * area_disp - invokes origination of
 * router LSA and routing table cleanup
 * @oa: ospf area
 *
 * It invokes aging and when @ospf_area->origrt is set to 1, start
 * function for origination of router LSA and network LSAs.
 */
void
area_disp(struct ospf_area *oa)
{
  struct proto_ospf *po = oa->po;
  struct ospf_iface *ifa;

  /* Now try to originage rt_lsa */
  if (oa->origrt)
    originate_rt_lsa(oa);

  /* Now try to originate network LSA's */
  WALK_LIST(ifa, po->iface_list)
  {
    if (ifa->orignet && (ifa->oa == oa))
      originate_net_lsa(ifa);
  }
}

/**
 * ospf_disp - invokes routing table calctulation, aging and also area_disp()
 * @timer: timer usually called every @proto_ospf->tick second, @timer->data
 * point to @proto_ospf
 */
void
ospf_disp(timer * timer)
{
  struct proto_ospf *po = timer->data;
  struct ospf_area *oa;

  WALK_LIST(oa, po->area_list)
    area_disp(oa);

  /* Age LSA DB */
  ospf_age(po);

  /* Calculate routing table */
  if (po->calcrt)
    ospf_rt_spf (po);
}



/**
 * ospf_import_control - accept or reject new route from nest's routing table
 * @p: current instance of protocol
 * @new: the new route
 * @attrs: list of attributes
 * @pool: pool for allocation of attributes
 *
 * Its quite simple. It does not accept our own routes and leaves the decision on
 * import to the filters.
 */

int
ospf_import_control(struct proto *p, rte ** new, ea_list ** attrs,
		    struct linpool *pool)
{
  rte *e = *new;

  if (p == e->attrs->proto)
    return -1;			/* Reject our own routes */
  *attrs = ospf_build_attrs(*attrs, pool, LSINFINITY, 10000, 0);
  return 0;			/* Leave decision to the filters */
}

struct ea_list *
ospf_make_tmp_attrs(struct rte *rt, struct linpool *pool)
{
  return ospf_build_attrs(NULL, pool, rt->u.ospf.metric1, rt->u.ospf.metric2,
			  rt->u.ospf.tag);
}

void
ospf_store_tmp_attrs(struct rte *rt, struct ea_list *attrs)
{
  rt->u.ospf.metric1 = ea_get_int(attrs, EA_OSPF_METRIC1, LSINFINITY);
  rt->u.ospf.metric2 = ea_get_int(attrs, EA_OSPF_METRIC2, 10000);
  rt->u.ospf.tag = ea_get_int(attrs, EA_OSPF_TAG, 0);
}

/**
 * ospf_shutdown - Finish of OSPF instance
 * @p: current instance of protocol
 *
 * RFC does not define any action that should be taken before router
 * shutdown. To make my neighbors react as fast as possible, I send
 * them hello packet with empty neighbor list. They should start
 * their neighbor state machine with event %NEIGHBOR_1WAY.
 */

static int
ospf_shutdown(struct proto *p)
{
  struct proto_ospf *po = (struct proto_ospf *) p;
  struct ospf_iface *ifa;
  OSPF_TRACE(D_EVENTS, "Shutdown requested");

  /* And send to all my neighbors 1WAY */
  WALK_LIST(ifa, po->iface_list) ospf_iface_shutdown(ifa);

  return PS_DOWN;
}

static void
ospf_rt_notify(struct proto *p, net * n, rte * new, rte * old UNUSED,
	       ea_list * attrs)
{
  struct proto_ospf *po = (struct proto_ospf *) p;

/* Temporarily down write anything
  OSPF_TRACE(D_EVENTS, "Got route %I/%d %s", p->name, n->n.prefix,
    n->n.pxlen, new ? "up" : "down");
*/

  if (new)			/* Got some new route */
  {
    originate_ext_lsa(n, new, po, attrs);
  }
  else
  {
    u32 rtid = po->proto.cf->global->router_id;
    struct ospf_area *oa;
    struct top_hash_entry *en;
    u32 pr = ipa_to_u32(n->n.prefix);
    struct ospf_lsa_ext *ext;
    int i;
    int max = max_ext_lsa(n->n.pxlen);

    /* Flush old external LSA */
    for (i = 0; i < max; i++, pr++)
    {
      if (en = ospf_hash_find(po->gr, 0, pr, rtid, LSA_T_EXT))
      {
        ext = en->lsa_body;
	if (ipa_compare(ext->netmask, ipa_mkmask(n->n.pxlen)) == 0)
	{
          WALK_LIST(oa, po->area_list)
          {
	    ospf_lsupd_flush_nlsa(en, oa);
	  }
	}
        break;
      }
    }
  }
}

static void
ospf_ifa_notify(struct proto *p, unsigned flags, struct ifa *a)
{
  struct proto_ospf *po = (struct proto_ospf *) p;
  struct ospf_iface *ifa;
  
  if ((a->flags & IA_SECONDARY) || (a->flags & IA_UNNUMBERED))
    return;

  WALK_LIST(ifa, po->iface_list)
    {
      if (ifa->iface == a->iface)
	{
	  schedule_rt_lsa(ifa->oa);
	  return;
	}
    }
}

static void
ospf_get_status(struct proto *p, byte * buf)
{
  struct proto_ospf *po = (struct proto_ospf *) p;

  if (p->proto_state == PS_DOWN)
    buf[0] = 0;
  else
  {
    struct ospf_iface *ifa;
    struct ospf_neighbor *n;
    int adj = 0;

    WALK_LIST(ifa, po->iface_list)
      WALK_LIST(n, ifa->neigh_list) if (n->state == NEIGHBOR_FULL)
      adj = 1;

    if (adj == 0)
      strcpy(buf, "Alone");
    else
      strcpy(buf, "Running");
  }
}

static void
ospf_get_route_info(rte * rte, byte * buf, ea_list * attrs UNUSED)
{
  char *type = "<bug>";

  switch(rte->attrs->source)
  {
    case RTS_OSPF:
      type = "I";
      break;
    case RTS_OSPF_IA:
      type = "IA";
      break;
    case RTS_OSPF_EXT1:
      type = "E1";
      break;
    case RTS_OSPF_EXT2:
      type = "E2";
      break;
  }

  buf += bsprintf(buf, " %s", type);
  buf += bsprintf(buf, " (%d/%d", rte->pref, rte->u.ospf.metric1);
  if (rte->attrs->source == RTS_OSPF_EXT2)
    buf += bsprintf(buf, "/%d", rte->u.ospf.metric2);
  buf += bsprintf(buf, ")");
  if ((rte->attrs->source == RTS_OSPF_EXT2 || rte->attrs->source == RTS_OSPF_EXT1) && rte->u.ospf.tag)
  {
    buf += bsprintf(buf, " [%x]", rte->u.ospf.tag);
  }
}

static int
ospf_get_attr(eattr * a, byte * buf, int buflen UNUSED)
{
  switch (a->id)
  {
  case EA_OSPF_METRIC1:
    bsprintf(buf, "metric1");
    return GA_NAME;
  case EA_OSPF_METRIC2:
    bsprintf(buf, "metric2");
    return GA_NAME;
  case EA_OSPF_TAG:
    bsprintf(buf, "tag: %08x", a->u.data);
    return GA_FULL;
  default:
    return GA_UNKNOWN;
  }
}

static int
ospf_patt_compare(struct ospf_iface_patt *a, struct ospf_iface_patt *b)
{
  return (a->type == b->type);
}

/**
 * ospf_reconfigure - reconfiguration hook
 * @p: current instance of protocol (with old configuration)
 * @c: new configuration requested by user
 *
 * This hook tries to be a little bit intelligent. Instance of OSPF
 * will survive change of many constants like hello interval,
 * password change, addition or deletion of some neighbor on
 * nonbroadcast network, cost of interface, etc.
 */
static int
ospf_reconfigure(struct proto *p, struct proto_config *c)
{
  struct ospf_config *old = (struct ospf_config *) (p->cf);
  struct ospf_config *new = (struct ospf_config *) c;
  struct ospf_area_config *oldac, *newac;
  struct proto_ospf *po = (struct proto_ospf *) p;
  struct ospf_iface_patt *oldip, *newip;
  struct ospf_iface *ifa;
  struct nbma_node *nb1, *nb2, *nbnx;
  struct ospf_area *oa = NULL;
  int found, olddead, newdead;
  struct area_net_config *anc;
  struct area_net *an;

  po->rfc1583 = new->rfc1583;
  schedule_rtcalc(po);

  po->tick = new->tick;
  po->disp_timer->recurrent = po->tick;
  tm_start(po->disp_timer, 1);

  oldac = HEAD(old->area_list);
  newac = HEAD(new->area_list);

  /* I should get it in the same order */

  while (((NODE(oldac))->next != NULL) && ((NODE(newac))->next != NULL))
  {
    if (oldac->areaid != newac->areaid)
      return 0;

    WALK_LIST(oa, po->area_list)
      if (oa->areaid == newac->areaid)
        break;

    if (!oa)
      return 0;

    oa->ac = newac;
    oa->stub = newac->stub;
    if (newac->stub && (oa->areaid == 0)) oa->stub = 0;

    /* Check stubnet_list */
    struct ospf_stubnet_config *oldsn = HEAD(oldac->stubnet_list);
    struct ospf_stubnet_config *newsn = HEAD(newac->stubnet_list);

    while (((NODE(oldsn))->next != NULL) && ((NODE(newsn))->next != NULL))
      {
	if (!ipa_equal(oldsn->px.addr, newsn->px.addr) ||
	    (oldsn->px.len != newsn->px.len) ||
	    (oldsn->hidden != newsn->hidden) ||
	    (oldsn->summary != newsn->summary) ||
	    (oldsn->cost != newsn->cost))
	  break;

	oldsn = (struct ospf_stubnet_config *)(NODE(oldsn))->next;
	newsn = (struct ospf_stubnet_config *)(NODE(newsn))->next;
      }

    /* If there is no change, both pointers should be NULL */
    if (((NODE(oldsn))->next) != ((NODE(newsn))->next))
      schedule_rt_lsa(oa);

    /* Change net_list */
    FIB_WALK(&oa->net_fib, nf)	/* First check if some networks are deleted */
    {
      found = 0;
      WALK_LIST(anc, newac->net_list)
      {
        if (ipa_equal(anc->px.addr, nf->prefix) && (anc->px.len == nf->pxlen))
	{
	  found = 1;
	  break;
	}
	if (!found) flush_sum_lsa(oa, nf, ORT_NET);	/* And flush them */
      }
    }
    FIB_WALK_END;

    WALK_LIST(anc, newac->net_list)	/* Second add new networks */
    {
      an = fib_get(&oa->net_fib, &anc->px.addr, anc->px.len);
      an->hidden = anc->hidden;
    }

    if (!iface_patts_equal(&oldac->patt_list, &newac->patt_list,
			   (void *) ospf_patt_compare))
      return 0;

    WALK_LIST(ifa, po->iface_list)
    {
      if (oldip = (struct ospf_iface_patt *)
	  iface_patt_find(&oldac->patt_list, ifa->iface))
      {
	/* Now reconfigure interface */
	if (!(newip = (struct ospf_iface_patt *)
	      iface_patt_find(&newac->patt_list, ifa->iface)))
	  return 0;

	/* HELLO TIMER */
	if (oldip->helloint != newip->helloint)
	{
	  ifa->helloint = newip->helloint;
	  ifa->hello_timer->recurrent = ifa->helloint;
	  tm_start(ifa->hello_timer, ifa->helloint);
	  OSPF_TRACE(D_EVENTS,
		     "Changing hello interval on interface %s from %d to %d",
		     ifa->iface->name, oldip->helloint, newip->helloint);
	}

	/* POLL TIMER */
	if (oldip->pollint != newip->pollint)
	{
	  ifa->pollint = newip->helloint;
	  ifa->poll_timer->recurrent = ifa->pollint;
	  tm_start(ifa->poll_timer, ifa->pollint);
	  OSPF_TRACE(D_EVENTS,
		     "Changing poll interval on interface %s from %d to %d",
		     ifa->iface->name, oldip->pollint, newip->pollint);
	}

	/* COST */
	if (oldip->cost != newip->cost)
	{
	  ifa->cost = newip->cost;
	  OSPF_TRACE(D_EVENTS,
		     "Changing cost interface %s from %d to %d",
		     ifa->iface->name, oldip->cost, newip->cost);
	  schedule_rt_lsa(ifa->oa);
	}

	/* RX BUFF */
	if (oldip->rxbuf != newip->rxbuf)
	{
	  ifa->rxbuf = newip->rxbuf;
	  OSPF_TRACE(D_EVENTS,
		     "Changing rxbuf interface %s from %d to %d",
		     ifa->iface->name, oldip->rxbuf, newip->rxbuf);
	  ospf_iface_change_mtu(po, ifa);
	}

	/* strict nbma */
	if ((oldip->strictnbma == 0) && (newip->strictnbma != 0))
	{
	  ifa->strictnbma = newip->strictnbma;
	  OSPF_TRACE(D_EVENTS,
		     "Interface %s is now strict NBMA.", ifa->iface->name);
	}
	if ((oldip->strictnbma != 0) && (newip->strictnbma == 0))
	{
	  ifa->strictnbma = newip->strictnbma;
	  OSPF_TRACE(D_EVENTS,
		     "Interface %s is no longer strict NBMA.",
		     ifa->iface->name);
	}

	/* stub */
	if ((oldip->stub == 0) && (newip->stub != 0))
	{
	  ifa->stub = newip->stub;
	  OSPF_TRACE(D_EVENTS, "Interface %s is now stub.", ifa->iface->name);
	}
	if ((oldip->stub != 0) && (newip->stub == 0) &&
	    ((ifa->ioprob & OSPF_I_IP) == 0) &&
	    (((ifa->ioprob & OSPF_I_MC) == 0) || (ifa->type == OSPF_IT_NBMA)))
	{
	  ifa->stub = newip->stub;
	  OSPF_TRACE(D_EVENTS,
		     "Interface %s is no longer stub.", ifa->iface->name);
	}

	/* AUTHENTICATION */
	if (oldip->autype != newip->autype)
	{
	  ifa->autype = newip->autype;
	  OSPF_TRACE(D_EVENTS,
		     "Changing authentication type on interface %s",
		     ifa->iface->name);
	}
        /* Add *passwords */
	ifa->passwords = newip->passwords;

	/* priority */
	if (oldip->priority != newip->priority)
	{
	  ifa->priority = newip->priority;
	  OSPF_TRACE(D_EVENTS,
		     "Changing priority on interface %s from %d to %d",
		     ifa->iface->name, oldip->priority, newip->priority);
	}

	/* RXMT */
	if (oldip->rxmtint != newip->rxmtint)
	{
	  ifa->rxmtint = newip->rxmtint;
	  OSPF_TRACE(D_EVENTS,
		     "Changing retransmit interval on interface %s from %d to %d",
		     ifa->iface->name, oldip->rxmtint, newip->rxmtint);
	}

	/* WAIT */
	if (oldip->waitint != newip->waitint)
	{
	  ifa->waitint = newip->waitint;
	  if (ifa->wait_timer->expires != 0)
	    tm_start(ifa->wait_timer, ifa->waitint);
	  OSPF_TRACE(D_EVENTS,
		     "Changing wait interval on interface %s from %d to %d",
		     ifa->iface->name, oldip->waitint, newip->waitint);
	}

	/* INFTRANS */
	if (oldip->inftransdelay != newip->inftransdelay)
	{
	  ifa->inftransdelay = newip->inftransdelay;
	  OSPF_TRACE(D_EVENTS,
		     "Changing transmit delay on interface %s from %d to %d",
		     ifa->iface->name, oldip->inftransdelay,
		     newip->inftransdelay);
	}

	/* DEAD */
	olddead = (oldip->dead == 0) ? oldip->deadc * oldip->helloint : oldip->dead;
	newdead = (newip->dead == 0) ? newip->deadc * newip->helloint : newip->dead;
	if (olddead != newdead)
	{
	  ifa->dead = newdead;
	  OSPF_TRACE(D_EVENTS,
		     "Changing dead interval on interface %s from %d to %d",
		     ifa->iface->name, olddead, newdead);
	}

	/* NBMA LIST */
	/* First remove old */
	WALK_LIST_DELSAFE(nb1, nbnx, ifa->nbma_list)
	{
	  found = 0;
	  WALK_LIST(nb2, newip->nbma_list)
	    if (ipa_compare(nb1->ip, nb2->ip) == 0)
	  {
	    found = 1;
	    if (nb1->eligible != nb2->eligible)
	      OSPF_TRACE(D_EVENTS,
			 "Changing neighbor eligibility %I on interface %s",
			 nb1->ip, ifa->iface->name);
	    break;
	  }

	  if (!found)
	  {
	    OSPF_TRACE(D_EVENTS,
		       "Removing NBMA neighbor %I on interface %s",
		       nb1->ip, ifa->iface->name);
	    rem_node(NODE nb1);
	    mb_free(nb1);
	  }
	}
	/* And then add new */
	WALK_LIST(nb2, newip->nbma_list)
	{
	  found = 0;
	  WALK_LIST(nb1, ifa->nbma_list)
	    if (ipa_compare(nb1->ip, nb2->ip) == 0)
	  {
	    found = 1;
	    break;
	  }
	  if (!found)
	  {
	    nb1 = mb_alloc(p->pool, sizeof(struct nbma_node));
	    nb1->ip = nb2->ip;
	    nb1->eligible = nb2->eligible;
	    add_tail(&ifa->nbma_list, NODE nb1);
	    OSPF_TRACE(D_EVENTS,
		       "Adding NBMA neighbor %I on interface %s",
		       nb1->ip, ifa->iface->name);
	  }
	}
      }
    }

    oldac = (struct ospf_area_config *)(NODE(oldac))->next;
    newac = (struct ospf_area_config *)(NODE(newac))->next;
  }

  if (((NODE(oldac))->next) != ((NODE(newac))->next))
    return 0;			/* One is not null */

  return 1;			/* Everything OK :-) */
}

void
ospf_sh_neigh(struct proto *p, char *iff)
{
  struct ospf_iface *ifa = NULL, *f;
  struct ospf_neighbor *n;
  struct proto_ospf *po = (struct proto_ospf *) p;

  if (p->proto_state != PS_UP)
  {
    cli_msg(-1013, "%s: is not up", p->name);
    cli_msg(0, "");
    return;
  }

  if (iff != NULL)
  {
    WALK_LIST(f, po->iface_list)
    {
      if (strcmp(iff, f->iface->name) == 0)
      {
	ifa = f;
	break;
      }
    }
    if (ifa == NULL)
    {
      cli_msg(0, "");
      return;
    }
    cli_msg(-1013, "%s:", p->name);
    cli_msg(-1013, "%-12s\t%3s\t%-15s\t%-5s\t%-12s\t%-10s", "Router ID",
	    "Pri", "     State", "DTime", "Router IP", "Interface");
    WALK_LIST(n, ifa->neigh_list) ospf_sh_neigh_info(n);
    cli_msg(0, "");
    return;
  }

  cli_msg(-1013, "%s:", p->name);
  cli_msg(-1013, "%-12s\t%3s\t%-15s\t%-5s\t%-12s\t%-10s", "Router ID", "Pri",
	  "     State", "DTime", "Router IP", "Interface");
  WALK_LIST(ifa, po->iface_list)
    WALK_LIST(n, ifa->neigh_list) ospf_sh_neigh_info(n);
  cli_msg(0, "");
}

void
ospf_sh(struct proto *p)
{
  struct ospf_area *oa;
  struct proto_ospf *po = (struct proto_ospf *) p;
  struct ospf_iface *ifa;
  struct ospf_neighbor *n;
  int ifano, nno, adjno, firstfib;
  struct area_net *anet;

  if (p->proto_state != PS_UP)
  {
    cli_msg(-1014, "%s: is not up", p->name);
    cli_msg(0, "");
    return;
  }

  cli_msg(-1014, "%s:", p->name);
  cli_msg(-1014, "RFC1583 compatibility: %s", (po->rfc1583 ? "enable" : "disabled"));
  cli_msg(-1014, "RT scheduler tick: %d", po->tick);
  cli_msg(-1014, "Number of areas: %u", po->areano);
  cli_msg(-1014, "Number of LSAs in DB:\t%u", po->gr->hash_entries);

  WALK_LIST(oa, po->area_list)
  {
    cli_msg(-1014, "\tArea: %R (%u) %s", oa->areaid, oa->areaid,
	    oa->areaid == 0 ? "[BACKBONE]" : "");
    ifano = 0;
    nno = 0;
    adjno = 0;
    WALK_LIST(ifa, po->iface_list)
    {
      if (oa == ifa->oa)
      {
	ifano++;
        WALK_LIST(n, ifa->neigh_list)
        {
	  nno++;
	  if (n->state == NEIGHBOR_FULL)
	    adjno++;
        }
      }
    }
    cli_msg(-1014, "\t\tStub:\t%s", oa->stub ? "Yes" : "No");
    cli_msg(-1014, "\t\tTransit:\t%s", oa->trcap ? "Yes" : "No");
    cli_msg(-1014, "\t\tNumber of interfaces:\t%u", ifano);
    cli_msg(-1014, "\t\tNumber of neighbors:\t%u", nno);
    cli_msg(-1014, "\t\tNumber of adjacent neighbors:\t%u", adjno);

    firstfib = 1;
    FIB_WALK(&oa->net_fib, nftmp)
    {
      anet = (struct area_net *) nftmp;
      if(firstfib)
      {
        cli_msg(-1014, "\t\tArea networks:");
        firstfib = 0;
      }
      cli_msg(-1014, "\t\t\t%1I/%u\t%s\t%s", anet->fn.prefix, anet->fn.pxlen,
		anet->hidden ? "Hidden" : "Advertise", anet->active ? "Active" : "");
    }
    FIB_WALK_END;
  }
  cli_msg(0, "");
}

void
ospf_sh_iface(struct proto *p, char *iff)
{
  struct proto_ospf *po = (struct proto_ospf *) p;
  struct ospf_iface *ifa = NULL, *f;

  if (p->proto_state != PS_UP)
  {
    cli_msg(-1015, "%s: is not up", p->name);
    cli_msg(0, "");
    return;
  }

  if (iff != NULL)
  {
    WALK_LIST(f, po->iface_list)
    {
      if (strcmp(iff, f->iface->name) == 0)
      {
	ifa = f;
	break;
      }
    }

    if (ifa == NULL)
    {
      cli_msg(0, "");
      return;
    }
    cli_msg(-1015, "%s:", p->name);
    ospf_iface_info(ifa);
    cli_msg(0, "");
    return;
  }
  cli_msg(-1015, "%s:", p->name);
  WALK_LIST(ifa, po->iface_list) ospf_iface_info(ifa);
  cli_msg(0, "");
}

/* First we want to separate network-LSAs and other LSAs (because network-LSAs
 * will be presented as network nodes and other LSAs together as router nodes)
 * Network-LSAs are sorted according to network prefix, other LSAs are sorted
 * according to originating router id (to get all LSA needed to represent one
 * router node together). Then, according to LSA type, ID and age.
 */
static int
he_compare(const void *p1, const void *p2)
{
  struct top_hash_entry * he1 = * (struct top_hash_entry **) p1;
  struct top_hash_entry * he2 = * (struct top_hash_entry **) p2;
  struct ospf_lsa_header *lsa1 = &(he1->lsa);
  struct ospf_lsa_header *lsa2 = &(he2->lsa);
  int nt1 = (lsa1->type == LSA_T_NET);
  int nt2 = (lsa2->type == LSA_T_NET);

  if (he1->oa->areaid != he2->oa->areaid)
    return he1->oa->areaid - he2->oa->areaid;

  if (nt1 != nt2)
    return nt1 - nt2;

  if (nt1)
  {
    // we are cheating for now
    if (lsa1->id != lsa2->id)
      return lsa1->id - lsa2->id;
    
    return lsa1->age - lsa2->age;
  }
  else 
    {
      if (lsa1->rt != lsa2->rt)
	return lsa1->rt - lsa2->rt;
 
      if (lsa1->type != lsa2->type)
	return lsa1->type - lsa2->type;
  
      if (lsa1->id != lsa2->id)
	return lsa1->id - lsa2->id;
  
      return lsa1->age - lsa2->age;
    }
}

static inline void
show_lsa_router(struct top_hash_entry *he)
{
  struct ospf_lsa_header *lsa = &(he->lsa);
  struct ospf_lsa_rt *rt = he->lsa_body;
  struct ospf_lsa_rt_link *rr = (struct ospf_lsa_rt_link *) (rt + 1);
  u32 i;

  for (i = 0; i < rt->links; i++)
    if (rr[i].type == LSART_PTP)
      cli_msg(-1016, "\t\trouter %R metric %u ", rr[i].id, rr[i].metric);

  for (i = 0; i < rt->links; i++)
    if (rr[i].type == LSART_NET)
    {
      struct proto_ospf *po = he->oa->po;
      struct top_hash_entry *net_he = ospf_hash_find(po->gr, he->oa->areaid, rr[i].id, rr[i].id, LSA_T_NET);
      if (net_he)
      {
	struct ospf_lsa_header *net_lsa = &(net_he->lsa);
	struct ospf_lsa_net *net_ln = net_he->lsa_body;
	cli_msg(-1016, "\t\tnetwork %I/%d metric %u ", ipa_and(ipa_from_u32(net_lsa->id), net_ln->netmask), ipa_mklen(net_ln->netmask), rr[i].metric);
      }
      else
	cli_msg(-1016, "\t\tnetwork ??? metric %u ", rr[i].metric);
    }

  for (i = 0; i < rt->links; i++)
    if (rr[i].type == LSART_STUB)
      cli_msg(-1016, "\t\tstubnet %I/%d metric %u ", ipa_from_u32(rr[i].id), ipa_mklen(ipa_from_u32(rr[i].data)), rr[i].metric);

  for (i = 0; i < rt->links; i++)
    if (rr[i].type == LSART_VLNK)
      cli_msg(-1016, "\t\tvlink %I metric %u ", ipa_from_u32(rr[i].id), rr[i].metric);
}

static inline void
show_lsa_network(struct top_hash_entry *he)
{
  struct ospf_lsa_header *lsa = &(he->lsa);
  struct ospf_lsa_net *ln = he->lsa_body;
  u32 *rts = (u32 *) (ln + 1);
  u32 max = (lsa->length - sizeof(struct ospf_lsa_header) - sizeof(struct ospf_lsa_net)) / sizeof(u32);
  u32 i;

  cli_msg(-1016, "");
  cli_msg(-1016, "\tnetwork %I/%d", ipa_and(ipa_from_u32(lsa->id), ln->netmask), ipa_mklen(ln->netmask));
  cli_msg(-1016, "\t\tdr %R", lsa->rt);

  for (i = 0; i < max; i++)
    cli_msg(-1016, "\t\trouter %R", rts[i]);
}

static inline void
show_lsa_sum_net(struct top_hash_entry *he)
{
  struct ospf_lsa_header *lsa = &(he->lsa);
  struct ospf_lsa_sum *sm = he->lsa_body;

  cli_msg(-1016, "\t\txnetwork %I/%d", ipa_and(ipa_from_u32(lsa->id), sm->netmask), ipa_mklen(sm->netmask));
}

static inline void
show_lsa_sum_rt(struct top_hash_entry *he)
{
  cli_msg(-1016, "\t\txrouter %R", he->lsa.id);
}


static inline void
show_lsa_external(struct top_hash_entry *he)
{
  struct ospf_lsa_header *lsa = &(he->lsa);
  struct ospf_lsa_ext *ext = he->lsa_body;
  struct ospf_lsa_ext_tos *et = (struct ospf_lsa_ext_tos *) (ext + 1);

  char str_via[STD_ADDRESS_P_LENGTH + 8] = "";
  char str_tag[16] = "";
  
  if (ipa_nonzero(et->fwaddr))
    bsprintf(str_via, " via %I", et->fwaddr);

  if (et->tag)
    bsprintf(str_tag, " tag %08x", et->tag);

  cli_msg(-1016, "\t\texternal %I/%d metric%s %u%s%s",
	  ipa_and(ipa_from_u32(lsa->id), ext->netmask),
	  ipa_mklen(ext->netmask), et->etm.etos.ebit ? "2" : "",
	  et->etm.metric & METRIC_MASK, str_via, str_tag);
}


void
ospf_sh_state(struct proto *p, int verbose)
{
  struct proto_ospf *po = (struct proto_ospf *) p;
  struct top_graph *f = po->gr;
  unsigned int i, j;
  u32 last_rt = 0xFFFFFFFF;
  u32 last_area = 0xFFFFFFFF;

  if (p->proto_state != PS_UP)
  {
    cli_msg(-1016, "%s: is not up", p->name);
    cli_msg(0, "");
    return;
  }

  struct top_hash_entry *hea[f->hash_entries];
  struct top_hash_entry *he;

  j = 0;
  for (i = 0; i < f->hash_size; i++)
    for (he = f->hash_table[i]; he != NULL; he = he->next)
      hea[j++] = he;

  if (j == f->hash_size)
    die("Fatal mismatch");

  qsort(hea, j, sizeof(struct top_hash_entry *), he_compare);

  for (i = 0; i < j; i++)
  {
    if ((verbose == 0) && (hea[i]->lsa.type > LSA_T_NET))
      continue;

    if (last_area != hea[i]->oa->areaid)
    {
      cli_msg(-1016, "");
      cli_msg(-1016, "area %R", hea[i]->oa->areaid);
      last_area = hea[i]->oa->areaid;
      last_rt = 0xFFFFFFFF;
    }

    if ((hea[i]->lsa.rt != last_rt) && (hea[i]->lsa.type != LSA_T_NET))
    {
      cli_msg(-1016, "");
      cli_msg(-1016, (hea[i]->lsa.type != LSA_T_EXT) ? "\trouter %R" : "\txrouter %R", hea[i]->lsa.rt);
      last_rt = hea[i]->lsa.rt;
    }

    switch (hea[i]->lsa.type)
    {
      case LSA_T_RT:
	show_lsa_router(hea[i]);
	break;

      case LSA_T_NET:
	show_lsa_network(hea[i]);
	break;

      case LSA_T_SUM_NET:
	show_lsa_sum_net(hea[i]);
	break;

      case LSA_T_SUM_RT:
	show_lsa_sum_rt(hea[i]);
	break;
      case LSA_T_EXT:
	show_lsa_external(hea[i]);
	break;
    }
  }

  cli_msg(0, "");
}

struct protocol proto_ospf = {
  name:"OSPF",
  template:"ospf%d",
  attr_class:EAP_OSPF,
  init:ospf_init,
  dump:ospf_dump,
  start:ospf_start,
  shutdown:ospf_shutdown,
  get_route_info:ospf_get_route_info,
  get_attr:ospf_get_attr,
  get_status:ospf_get_status,
  reconfigure:ospf_reconfigure
};
