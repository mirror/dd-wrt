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
 * In OSPFv2 specification, it is implied that there is one IP prefix
 * for each physical network/interface (unless it is an ptp link). But
 * in modern systems, there might be more independent IP prefixes
 * associated with an interface.  To handle this situation, we have
 * one &ospf_iface for each active IP prefix (instead for each active
 * iface); This behaves like virtual interface for the purpose of OSPF.
 * If we receive packet, we associate it with a proper virtual interface
 * mainly according to its source address.
 *
 * OSPF keeps one socket per &ospf_iface. This allows us (compared to
 * one socket approach) to evade problems with a limit of multicast
 * groups per socket and with sending multicast packets to appropriate
 * interface in a portable way. The socket is associated with
 * underlying physical iface and should not receive packets received
 * on other ifaces (unfortunately, this is not true on
 * BSD). Generally, one packet can be received by more sockets (for
 * example, if there are more &ospf_iface on one physical iface),
 * therefore we explicitly filter received packets according to
 * src/dst IP address and received iface.
 *
 * Vlinks are implemented using particularly degenerate form of
 * &ospf_iface, which has several exceptions: it does not have its
 * iface or socket (it copies these from 'parent' &ospf_iface) and it
 * is present in iface list even when down (it is not freed in
 * ospf_iface_down()).
 *
 * The heart beat of ospf is ospf_disp(). It is called at regular intervals
 * (&proto_ospf->tick). It is responsible for aging and flushing of LSAs in
 * the database, for routing table calculaction and it call area_disp() of every
 * ospf_area.
 * 
 * The function area_disp() is
 * responsible for late originating of router LSA and network LSA
 * and for cleanup before routing table calculation process in
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


static int ospf_reload_routes(struct proto *p);
static void ospf_rt_notify(struct proto *p, struct rtable *table UNUSED, net * n, rte * new, rte * old UNUSED, ea_list * attrs);
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
    struct area_net_config *anc;
    struct area_net *an;

    fib_init(&oa->net_fib, po->proto.pool, sizeof(struct area_net), 0, ospf_area_initfib);
    fib_init(&oa->enet_fib, po->proto.pool, sizeof(struct area_net), 0, ospf_area_initfib);

    WALK_LIST(anc, ac->net_list)
    {
      an = (struct area_net *) fib_get(&oa->net_fib, &anc->px.addr, anc->px.len);
      an->hidden = anc->hidden;
    }

    WALK_LIST(anc, ac->enet_list)
    {
      an = (struct area_net *) fib_get(&oa->enet_fib, &anc->px.addr, anc->px.len);
      an->hidden = anc->hidden;
      an->tag = anc->tag;
    }
}

static void
ospf_area_add(struct proto_ospf *po, struct ospf_area_config *ac, int reconf)
{
  struct proto *p = &po->proto;
  struct ospf_area *oa;

  OSPF_TRACE(D_EVENTS, "Adding area %R", ac->areaid);

  oa = mb_allocz(p->pool, sizeof(struct ospf_area));
  add_tail(&po->area_list, NODE oa);
  po->areano++;

  oa->ac = ac;
  oa->areaid = ac->areaid;
  oa->rt = NULL;
  oa->po = po;
  fib_init(&oa->rtr, p->pool, sizeof(ort), 0, ospf_rt_initort);
  add_area_nets(oa, ac);

  if (oa->areaid == 0)
    po->backbone = oa;

#ifdef OSPFv2
  oa->options = ac->type;
#else /* OSPFv3 */
  oa->options = ac->type | OPT_V6 | (po->stub_router ? 0 : OPT_R);
#endif

  /*
   * Set E-bit for NSSA ABR routers. No need to explicitly call
   * schedule_rt_lsa() for other areas, will be done anyway.
   * We use cf->abr because po->areano is not yet complete.
   */
  if (oa_is_nssa(oa) && ((struct ospf_config *) (p->cf))->abr)
    po->ebit = 1;

  if (reconf)
    ospf_ifaces_reconfigure(oa, ac);
}

static void
ospf_area_remove(struct ospf_area *oa)
{
  struct proto *p = &oa->po->proto;
  OSPF_TRACE(D_EVENTS, "Removing area %R", oa->areaid);

  /* We suppose that interfaces are already removed */
  ospf_flush_area(oa->po, oa->areaid);
 
  fib_free(&oa->rtr);
  fib_free(&oa->net_fib);
  fib_free(&oa->enet_fib);

  if (oa->translator_timer)
    rfree(oa->translator_timer);

  oa->po->areano--;
  rem_node(NODE oa);
  mb_free(oa);
}


struct ospf_area *
ospf_find_area(struct proto_ospf *po, u32 aid)
{
  struct ospf_area *oa;
  WALK_LIST(oa, po->area_list)
    if (((struct ospf_area *) oa)->areaid == aid)
      return oa;
  return NULL;
}

static struct ospf_iface *
ospf_find_vlink(struct proto_ospf *po, u32 voa, u32 vid)
{
  struct ospf_iface *ifa;
  WALK_LIST(ifa, po->iface_list) 
    if ((ifa->type == OSPF_IT_VLINK) && (ifa->voa->areaid == voa) && (ifa->vid == vid))
      return ifa;
  return NULL;
}

static int
ospf_start(struct proto *p)
{
  struct proto_ospf *po = (struct proto_ospf *) p;
  struct ospf_config *c = (struct ospf_config *) (p->cf);
  struct ospf_area_config *ac;

  po->router_id = proto_get_router_id(p->cf);
  po->last_vlink_id = 0x80000000;
  po->rfc1583 = c->rfc1583;
  po->stub_router = c->stub_router;
  po->ebit = 0;
  po->ecmp = c->ecmp;
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
  po->nhpool = lp_new(p->pool, 12*sizeof(struct mpnh));
  init_list(&(po->iface_list));
  init_list(&(po->area_list));
  fib_init(&po->rtf, p->pool, sizeof(ort), 0, ospf_rt_initort);
  po->areano = 0;
  po->gr = ospf_top_new(p->pool);
  s_init_list(&(po->lsal));

  WALK_LIST(ac, c->area_list)
    ospf_area_add(po, ac, 0);

  /* Add all virtual links */
  struct ospf_iface_patt *ic;
  WALK_LIST(ic, c->vlink_list)
    ospf_iface_new(po->backbone, NULL, ic);

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

  /*
  OSPF_TRACE(D_EVENTS, "LSA graph dump start:");
  ospf_top_dump(po->gr, p);
  OSPF_TRACE(D_EVENTS, "LSA graph dump finished");
  */
  neigh_dump_all();
}

static struct proto *
ospf_init(struct proto_config *c)
{
  struct proto *p = proto_new(c, sizeof(struct proto_ospf));

  p->make_tmp_attrs = ospf_make_tmp_attrs;
  p->store_tmp_attrs = ospf_store_tmp_attrs;
  p->import_control = ospf_import_control;
  p->reload_routes = ospf_reload_routes;
  p->accept_ra_types = RA_OPTIMAL;
  p->rt_notify = ospf_rt_notify;
  p->if_notify = ospf_if_notify;
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
    new->u.ospf.tag == old->u.ospf.tag &&
    new->u.ospf.router_id == old->u.ospf.router_id;
}

static ea_list *
ospf_build_attrs(ea_list * next, struct linpool *pool, u32 m1, u32 m2,
		 u32 tag, u32 rid)
{
  struct ea_list *l =
    lp_alloc(pool, sizeof(struct ea_list) + 4 * sizeof(eattr));

  l->next = next;
  l->flags = EALF_SORTED;
  l->count = 4;
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
  l->attrs[3].id = EA_OSPF_ROUTER_ID;
  l->attrs[3].flags = 0;
  l->attrs[3].type = EAF_TYPE_ROUTER_ID | EAF_TEMP;
  l->attrs[3].u.data = rid;
  return l;
}

void
schedule_net_lsa(struct ospf_iface *ifa)
{
  struct proto *p = &ifa->oa->po->proto;

  OSPF_TRACE(D_EVENTS, "Scheduling network-LSA origination for iface %s", ifa->iface->name);
  ifa->orignet = 1;
}

#ifdef OSPFv3
void
schedule_link_lsa(struct ospf_iface *ifa)
{
  struct proto *p = &ifa->oa->po->proto;

  OSPF_TRACE(D_EVENTS, "Scheduling link-LSA origination for iface %s", ifa->iface->name);
  ifa->origlink = 1;
}
#endif

void
schedule_rt_lsa(struct ospf_area *oa)
{
  struct proto *p = &oa->po->proto;

  OSPF_TRACE(D_EVENTS, "Scheduling router-LSA origination for area %R", oa->areaid);
  oa->origrt = 1;
}

void
schedule_rtcalc(struct proto_ospf *po)
{
  struct proto *p = &po->proto;

  if (po->calcrt)
    return;

  OSPF_TRACE(D_EVENTS, "Scheduling routing table calculation");
  po->calcrt = 1;
}

static int
ospf_reload_routes(struct proto *p)
{
  struct proto_ospf *po = (struct proto_ospf *) p;

  if (po->calcrt != 2)
    OSPF_TRACE(D_EVENTS, "Scheduling routing table calculation with route reload");

  po->calcrt = 2;

  return 1;
}

/**
 * area_disp - invokes origination of
 * router LSA and routing table cleanup
 * @oa: ospf area
 *
 * It invokes aging and when @ospf_area->origrt is set to 1, start
 * function for origination of router, network LSAs.
 */
void
area_disp(struct ospf_area *oa)
{
  struct proto_ospf *po = oa->po;
  struct ospf_iface *ifa;

  /* Now try to originage rt_lsa */
  if (oa->origrt)
    update_rt_lsa(oa);

  /* Now try to originate network LSA's */
  WALK_LIST(ifa, po->iface_list)
  {
#ifdef OSPFv3
    /* Link LSA should be originated before Network LSA */
    if (ifa->origlink && (ifa->oa == oa))
      update_link_lsa(ifa);
#endif

    if (ifa->orignet && (ifa->oa == oa))
      update_net_lsa(ifa);
  }
}

/**
 * ospf_disp - invokes routing table calculation, aging and also area_disp()
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
    ospf_rt_spf(po);
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
  struct ospf_area *oa = ospf_main_area((struct proto_ospf *) p);
  rte *e = *new;

  if (p == e->attrs->proto)
    return -1;			/* Reject our own routes */

  if (oa_is_stub(oa))
    return -1;			/* Do not export routes to stub areas */

  eattr *ea = ea_find(e->attrs->eattrs, EA_GEN_IGP_METRIC);
  u32 m1 = (ea && (ea->u.data < LSINFINITY)) ? ea->u.data : LSINFINITY;

  *attrs = ospf_build_attrs(*attrs, pool, m1, 10000, 0, 0);
  return 0;			/* Leave decision to the filters */
}

struct ea_list *
ospf_make_tmp_attrs(struct rte *rt, struct linpool *pool)
{
  return ospf_build_attrs(NULL, pool, rt->u.ospf.metric1, rt->u.ospf.metric2,
			  rt->u.ospf.tag, rt->u.ospf.router_id);
}

void
ospf_store_tmp_attrs(struct rte *rt, struct ea_list *attrs)
{
  rt->u.ospf.metric1 = ea_get_int(attrs, EA_OSPF_METRIC1, LSINFINITY);
  rt->u.ospf.metric2 = ea_get_int(attrs, EA_OSPF_METRIC2, 10000);
  rt->u.ospf.tag = ea_get_int(attrs, EA_OSPF_TAG, 0);
  rt->u.ospf.router_id = ea_get_int(attrs, EA_OSPF_ROUTER_ID, 0);
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
  WALK_LIST(ifa, po->iface_list)
    ospf_iface_shutdown(ifa);

  /* Cleanup locked rta entries */
  FIB_WALK(&po->rtf, nftmp)
  {
    rta_free(((ort *) nftmp)->old_rta);
  }
  FIB_WALK_END;

  return PS_DOWN;
}

static void
ospf_rt_notify(struct proto *p, rtable *tbl UNUSED, net * n, rte * new, rte * old UNUSED, ea_list * attrs)
{
  struct proto_ospf *po = (struct proto_ospf *) p;
  struct ospf_area *oa = ospf_main_area(po);
  ort *nf = (ort *) fib_get(&po->rtf, &n->n.prefix, n->n.pxlen);
  struct fib_node *fn = &nf->fn;

  if (!new)
  {
    if (fn->x1 != EXT_EXPORT)
      return;

    flush_ext_lsa(oa, fn, oa_is_nssa(oa));

    /* Old external route might blocked some NSSA translation */
    if (po->areano > 1)
      schedule_rtcalc(po);

    return;
  }

  /* Get route attributes */
  u32 m1 = ea_get_int(attrs, EA_OSPF_METRIC1, LSINFINITY);
  u32 m2 = ea_get_int(attrs, EA_OSPF_METRIC2, 10000);
  u32 metric = (m1 != LSINFINITY) ? m1 : (m2 | LSA_EXT_EBIT);
  u32 tag = ea_get_int(attrs, EA_OSPF_TAG, 0);
  ip_addr gw = IPA_NONE;
  // FIXME check for gw should be per ifa, not per iface
  if ((new->attrs->dest == RTD_ROUTER) &&
      ipa_nonzero(new->attrs->gw) &&
      !ipa_has_link_scope(new->attrs->gw) &&
      (ospf_iface_find((struct proto_ospf *) p, new->attrs->iface) != NULL))
    gw = new->attrs->gw;

  originate_ext_lsa(oa, fn, EXT_EXPORT, metric, gw, tag, 1);
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
  if ((rte->attrs->source == RTS_OSPF_EXT1 || rte->attrs->source == RTS_OSPF_EXT2) && rte->u.ospf.tag)
  {
    buf += bsprintf(buf, " [%x]", rte->u.ospf.tag);
  }
  if (rte->u.ospf.router_id)
    buf += bsprintf(buf, " [%R]", rte->u.ospf.router_id);
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
    bsprintf(buf, "tag: 0x%08x", a->u.data);
    return GA_FULL;
  case EA_OSPF_ROUTER_ID:
    bsprintf(buf, "router_id");
    return GA_NAME;
  default:
    return GA_UNKNOWN;
  }
}

static void
ospf_area_reconfigure(struct ospf_area *oa, struct ospf_area_config *nac)
{
  oa->ac = nac;

  // FIXME better area type reconfiguration
#ifdef OSPFv2
  oa->options = nac->type;
#else /* OSPFv3 */
  oa->options = nac->type | OPT_V6 | (oa->po->stub_router ? 0 : OPT_R);
#endif
  if (oa_is_nssa(oa) && (oa->po->areano > 1))
    oa->po->ebit = 1;

  ospf_ifaces_reconfigure(oa, nac);

  /* Handle net_list */
  fib_free(&oa->net_fib);
  fib_free(&oa->enet_fib);
  add_area_nets(oa, nac);

  /* No need to handle stubnet_list */

  oa->marked = 0;
  schedule_rt_lsa(oa);
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
  struct proto_ospf *po = (struct proto_ospf *) p;
  struct ospf_config *old = (struct ospf_config *) (p->cf);
  struct ospf_config *new = (struct ospf_config *) c;
  struct ospf_area_config *nac;
  struct ospf_area *oa, *oax;
  struct ospf_iface *ifa, *ifx;
  struct ospf_iface_patt *ip;

  if (proto_get_router_id(c) != po->router_id)
    return 0;

  if (po->rfc1583 != new->rfc1583)
    return 0;

  if (old->abr != new->abr)
    return 0;

  po->stub_router = new->stub_router;
  po->ecmp = new->ecmp;
  po->tick = new->tick;
  po->disp_timer->recurrent = po->tick;
  tm_start(po->disp_timer, 1);

  /* Mark all areas and ifaces */
  WALK_LIST(oa, po->area_list)
    oa->marked = 1;

  WALK_LIST(ifa, po->iface_list)
    ifa->marked = 1;

  /* Add and update areas */
  WALK_LIST(nac, new->area_list)
  {
    oa = ospf_find_area(po, nac->areaid);
    if (oa)
      ospf_area_reconfigure(oa, nac);
    else
      ospf_area_add(po, nac, 1);
  }

  /* Add and update vlinks */
  WALK_LIST(ip, new->vlink_list)
  {
    ifa = ospf_find_vlink(po, ip->voa, ip->vid);
    if (ifa)
      ospf_iface_reconfigure(ifa, ip);
    else
      ospf_iface_new(po->backbone, NULL, ip);
  }

  /* Delete remaining ifaces and areas */
  WALK_LIST_DELSAFE(ifa, ifx, po->iface_list)
    if (ifa->marked)
    {
      ospf_iface_shutdown(ifa);
      ospf_iface_remove(ifa);
    }

  WALK_LIST_DELSAFE(oa, oax, po->area_list)
    if (oa->marked)
      ospf_area_remove(oa);

  schedule_rtcalc(po);
  
  return 1;
}


void
ospf_sh_neigh(struct proto *p, char *iff)
{
  struct ospf_iface *ifa = NULL;
  struct ospf_neighbor *n;
  struct proto_ospf *po = (struct proto_ospf *) p;

  if (p->proto_state != PS_UP)
  {
    cli_msg(-1013, "%s: is not up", p->name);
    cli_msg(0, "");
    return;
  }

  cli_msg(-1013, "%s:", p->name);
  cli_msg(-1013, "%-12s\t%3s\t%-15s\t%-5s\t%-10s %-12s", "Router ID", "Pri",
	  "     State", "DTime", "Interface", "Router IP");
  WALK_LIST(ifa, po->iface_list)
    if ((iff == NULL) || patmatch(iff, ifa->iface->name))
      WALK_LIST(n, ifa->neigh_list)
	ospf_sh_neigh_info(n);
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
  cli_msg(-1014, "Stub router: %s", (po->stub_router ? "Yes" : "No"));
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

    cli_msg(-1014, "\t\tStub:\t%s", oa_is_stub(oa) ? "Yes" : "No");
    cli_msg(-1014, "\t\tNSSA:\t%s", oa_is_nssa(oa) ? "Yes" : "No");
    cli_msg(-1014, "\t\tTransit:\t%s", oa->trcap ? "Yes" : "No");

    if (oa_is_nssa(oa))
      cli_msg(-1014, "\t\tNSSA translation:\t%s%s", oa->translate ? "Yes" : "No",
	      oa->translate == TRANS_WAIT ? " (run down)" : "");
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

    firstfib = 1;
    FIB_WALK(&oa->enet_fib, nftmp)
    {
      anet = (struct area_net *) nftmp;
      if(firstfib)
      {
        cli_msg(-1014, "\t\tArea external networks:");
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
  struct ospf_iface *ifa = NULL;

  if (p->proto_state != PS_UP)
  {
    cli_msg(-1015, "%s: is not up", p->name);
    cli_msg(0, "");
    return;
  }

  cli_msg(-1015, "%s:", p->name);
  WALK_LIST(ifa, po->iface_list)
    if ((iff == NULL) || patmatch(iff, ifa->iface->name))
      ospf_iface_info(ifa);
  cli_msg(0, "");
}

/* lsa_compare_for_state() - Compare function for 'show ospf state'
 *
 * First we want to separate network-LSAs and other LSAs (because network-LSAs
 * will be presented as network nodes and other LSAs together as router nodes)
 * Network-LSAs are sorted according to network prefix, other LSAs are sorted
 * according to originating router id (to get all LSA needed to represent one
 * router node together). Then, according to LSA type, ID and age.
 *
 * For OSPFv3, we have to handle also Prefix-LSAs. We would like to put each
 * immediately after the referenced LSA. We will make faked LSA based on ref_
 * values
 */

#ifdef OSPFv3

static struct ospf_lsa_header *
fake_lsa_from_prefix_lsa(struct ospf_lsa_header *dst, struct ospf_lsa_header *src,
			 struct ospf_lsa_prefix *px)
{
  dst->age = src->age;
  dst->type = px->ref_type;
  dst->id = px->ref_id;
  dst->rt = px->ref_rt;
  dst->sn = src->sn;

  return dst;
}

#endif

static int
lsa_compare_for_state(const void *p1, const void *p2)
{
  struct top_hash_entry * he1 = * (struct top_hash_entry **) p1;
  struct top_hash_entry * he2 = * (struct top_hash_entry **) p2;
  struct ospf_lsa_header *lsa1 = &(he1->lsa);
  struct ospf_lsa_header *lsa2 = &(he2->lsa);

  if (he1->domain < he2->domain)
    return -1;
  if (he1->domain > he2->domain)
    return 1;

#ifdef OSPFv3
  struct ospf_lsa_header lsatmp1, lsatmp2;

  int px1 = (lsa1->type == LSA_T_PREFIX);
  int px2 = (lsa2->type == LSA_T_PREFIX);

  if (px1)
    lsa1 = fake_lsa_from_prefix_lsa(&lsatmp1, lsa1, he1->lsa_body);

  if (px2)
    lsa2 = fake_lsa_from_prefix_lsa(&lsatmp2, lsa2, he2->lsa_body);
#endif

  int nt1 = (lsa1->type == LSA_T_NET);
  int nt2 = (lsa2->type == LSA_T_NET);

  if (nt1 != nt2)
    return nt1 - nt2;

  if (nt1)
  {
#ifdef OSPFv3
    /* In OSPFv3, neworks are named base on ID of DR */
    if (lsa1->rt < lsa2->rt)
      return -1;
    if (lsa1->rt > lsa2->rt)
      return 1;
#endif

    /* For OSPFv2, this is IP of the network,
       for OSPFv3, this is interface ID */
    if (lsa1->id < lsa2->id)
      return -1;
    if (lsa1->id > lsa2->id)
      return 1;

#ifdef OSPFv3
    if (px1 != px2)
      return px1 - px2;
#endif

    return lsa1->sn - lsa2->sn;
  }
  else 
  {
    if (lsa1->rt < lsa2->rt)
      return -1;
    if (lsa1->rt > lsa2->rt)
      return 1;

    if (lsa1->type < lsa2->type)
      return -1;
    if (lsa1->type > lsa2->type)
      return 1;

    if (lsa1->id < lsa2->id)
      return -1;
    if (lsa1->id > lsa2->id)
      return 1;

#ifdef OSPFv3
    if (px1 != px2)
      return px1 - px2;
#endif
  
    return lsa1->sn - lsa2->sn;
  }
}

static int
ext_compare_for_state(const void *p1, const void *p2)
{
  struct top_hash_entry * he1 = * (struct top_hash_entry **) p1;
  struct top_hash_entry * he2 = * (struct top_hash_entry **) p2;
  struct ospf_lsa_header *lsa1 = &(he1->lsa);
  struct ospf_lsa_header *lsa2 = &(he2->lsa);

  if (lsa1->rt < lsa2->rt)
    return -1;
  if (lsa1->rt > lsa2->rt)
    return 1;

  if (lsa1->id < lsa2->id)
    return -1;
  if (lsa1->id > lsa2->id)
    return 1;

  return lsa1->sn - lsa2->sn;
}

static inline void
show_lsa_distance(struct top_hash_entry *he)
{
  if (he->color == INSPF)
    cli_msg(-1016, "\t\tdistance %u", he->dist);
  else
    cli_msg(-1016, "\t\tunreachable");
}

static inline void
show_lsa_router(struct proto_ospf *po, struct top_hash_entry *he, int first, int verbose)
{
  struct ospf_lsa_header *lsa = &(he->lsa);
  struct ospf_lsa_rt *rt = he->lsa_body;
  struct ospf_lsa_rt_link *rr = (struct ospf_lsa_rt_link *) (rt + 1);
  int max = lsa_rt_count(lsa);
  int i;

  if (first)
  {
    cli_msg(-1016, "");
    cli_msg(-1016, "\trouter %R", he->lsa.rt);
    show_lsa_distance(he);
  }


  for (i = 0; i < max; i++)
    if (rr[i].type == LSART_VLNK)
      cli_msg(-1016, "\t\tvlink %R metric %u", rr[i].id, rr[i].metric);

  for (i = 0; i < max; i++)
    if (rr[i].type == LSART_PTP)
      cli_msg(-1016, "\t\trouter %R metric %u", rr[i].id, rr[i].metric);

  for (i = 0; i < max; i++)
    if (rr[i].type == LSART_NET)
    {
#ifdef OSPFv2
      struct top_hash_entry *net_he = ospf_hash_find_net(po->gr, he->domain, rr[i].id);

      if (net_he)
      {
	struct ospf_lsa_header *net_lsa = &(net_he->lsa);
	struct ospf_lsa_net *net_ln = net_he->lsa_body;

	cli_msg(-1016, "\t\tnetwork %I/%d metric %u", 
		ipa_and(ipa_from_u32(net_lsa->id), net_ln->netmask),
		ipa_mklen(net_ln->netmask), rr[i].metric);
      }
      else
	cli_msg(-1016, "\t\tnetwork [%R] metric %u", rr[i].id, rr[i].metric);

#else /* OSPFv3 */
      cli_msg(-1016, "\t\tnetwork [%R-%u] metric %u", rr[i].id, rr[i].nif, rr[i].metric);
#endif
    }

#ifdef OSPFv2
  if (!verbose)
    return;

  for (i = 0; i < max; i++)
    if (rr[i].type == LSART_STUB)
      cli_msg(-1016, "\t\tstubnet %I/%d metric %u", ipa_from_u32(rr[i].id),
	      ipa_mklen(ipa_from_u32(rr[i].data)), rr[i].metric);
#endif
}

static inline void
show_lsa_network(struct top_hash_entry *he)
{
  struct ospf_lsa_header *lsa = &(he->lsa);
  struct ospf_lsa_net *ln = he->lsa_body;
  u32 i;

#ifdef OSPFv2
  cli_msg(-1016, "");
  cli_msg(-1016, "\tnetwork %I/%d", ipa_and(ipa_from_u32(lsa->id), ln->netmask), ipa_mklen(ln->netmask));
  cli_msg(-1016, "\t\tdr %R", lsa->rt);
#else /* OSPFv3 */
  cli_msg(-1016, "");
  cli_msg(-1016, "\tnetwork [%R-%u]", lsa->rt, lsa->id);
#endif

  show_lsa_distance(he);

  for (i = 0; i < lsa_net_count(lsa); i++)
    cli_msg(-1016, "\t\trouter %R", ln->routers[i]);
}

static inline void
show_lsa_sum_net(struct top_hash_entry *he)
{
  ip_addr ip;
  int pxlen;

#ifdef OSPFv2
  struct ospf_lsa_sum *ls = he->lsa_body;
  pxlen = ipa_mklen(ls->netmask);
  ip = ipa_and(ipa_from_u32(he->lsa.id), ls->netmask);
#else /* OSPFv3 */
  u8 pxopts;
  u16 rest;
  struct ospf_lsa_sum_net *ls = he->lsa_body;
  lsa_get_ipv6_prefix(ls->prefix, &ip, &pxlen, &pxopts, &rest);
#endif

  cli_msg(-1016, "\t\txnetwork %I/%d metric %u", ip, pxlen, ls->metric);
}

static inline void
show_lsa_sum_rt(struct top_hash_entry *he)
{
  u32 dst_rid;

#ifdef OSPFv2
  struct ospf_lsa_sum *ls = he->lsa_body;
  dst_rid = he->lsa.id;
  // options = 0;
#else /* OSPFv3 */
  struct ospf_lsa_sum_rt *ls = he->lsa_body;
  dst_rid = ls->drid; 
  // options = ls->options & OPTIONS_MASK;
#endif

  cli_msg(-1016, "\t\txrouter %R metric %u", dst_rid, ls->metric);
}


static inline void
show_lsa_external(struct top_hash_entry *he)
{
  struct ospf_lsa_ext *ext = he->lsa_body;
  char str_via[STD_ADDRESS_P_LENGTH + 8] = "";
  char str_tag[16] = "";
  ip_addr ip, rt_fwaddr;
  int pxlen, ebit, rt_fwaddr_valid;
  u32 rt_tag, rt_metric;

  if (he->lsa.type == LSA_T_EXT)
    he->domain = 0; /* Unmark the LSA */

  rt_metric = ext->metric & METRIC_MASK;
  ebit = ext->metric & LSA_EXT_EBIT;
#ifdef OSPFv2
  ip = ipa_and(ipa_from_u32(he->lsa.id), ext->netmask);
  pxlen = ipa_mklen(ext->netmask);
  rt_fwaddr = ext->fwaddr;
  rt_fwaddr_valid = !ipa_equal(rt_fwaddr, IPA_NONE);
  rt_tag = ext->tag;
#else /* OSPFv3 */
  u8 pxopts;
  u16 rest;
  u32 *buf = ext->rest;
  buf = lsa_get_ipv6_prefix(buf, &ip, &pxlen, &pxopts, &rest);

  rt_fwaddr_valid = ext->metric & LSA_EXT_FBIT;
  if (rt_fwaddr_valid)
    buf = lsa_get_ipv6_addr(buf, &rt_fwaddr);
  else 
    rt_fwaddr = IPA_NONE;

  if (ext->metric & LSA_EXT_TBIT)
    rt_tag = *buf++;
  else
    rt_tag = 0;
#endif
  
  if (rt_fwaddr_valid)
    bsprintf(str_via, " via %I", rt_fwaddr);

  if (rt_tag)
    bsprintf(str_tag, " tag %08x", rt_tag);

  cli_msg(-1016, "\t\t%s %I/%d metric%s %u%s%s",
	  (he->lsa.type == LSA_T_NSSA) ? "nssa-ext" : "external",
	  ip, pxlen, ebit ? "2" : "", rt_metric, str_via, str_tag);
}

#ifdef OSPFv3
static inline void
show_lsa_prefix(struct top_hash_entry *he, struct ospf_lsa_header *cnode)
{
  struct ospf_lsa_prefix *px = he->lsa_body;
  ip_addr pxa;
  int pxlen;
  u8 pxopts;
  u16 metric;
  u32 *buf;
  int i;

  /* We check whether given prefix-LSA is related to the current node */
  if ((px->ref_type != cnode->type) || (px->ref_rt != cnode->rt))
    return;

  if ((px->ref_type == LSA_T_RT) && (px->ref_id != 0))
    return;

  if ((px->ref_type == LSA_T_NET) && (px->ref_id != cnode->id))
    return;

  buf = px->rest;
  for (i = 0; i < px->pxcount; i++)
    {
      buf = lsa_get_ipv6_prefix(buf, &pxa, &pxlen, &pxopts, &metric);

      if (px->ref_type == LSA_T_RT)
	cli_msg(-1016, "\t\tstubnet %I/%d metric %u", pxa, pxlen, metric);
      else
	cli_msg(-1016, "\t\taddress %I/%d", pxa, pxlen);
    }
}
#endif

void
ospf_sh_state(struct proto *p, int verbose, int reachable)
{
  struct proto_ospf *po = (struct proto_ospf *) p;
  struct ospf_lsa_header *cnode = NULL;
  int num = po->gr->hash_entries;
  unsigned int i, ix, j1, j2, jx;
  u32 last_area = 0xFFFFFFFF;

  if (p->proto_state != PS_UP)
  {
    cli_msg(-1016, "%s: is not up", p->name);
    cli_msg(0, "");
    return;
  }

  /* We store interesting area-scoped LSAs in array hea and 
     global-scoped (LSA_T_EXT) LSAs in array hex */

  struct top_hash_entry *hea[num];
  struct top_hash_entry *hex[verbose ? num : 0];
  struct top_hash_entry *he;

  j1 = j2 = jx = 0;
  WALK_SLIST(he, po->lsal)
  {
    int accept;

    switch (he->lsa.type)
      {
      case LSA_T_RT:
      case LSA_T_NET:
	accept = 1;
	break;

      case LSA_T_SUM_NET:
      case LSA_T_SUM_RT:
      case LSA_T_NSSA:
#ifdef OSPFv3
      case LSA_T_PREFIX:
#endif
	accept = verbose;
	break;

      case LSA_T_EXT:
	if (verbose)
	{
	  he->domain = 1; /* Abuse domain field to mark the LSA */
	  hex[jx++] = he;
	}
      default:
	accept = 0;
      }

    if (accept)
      hea[j1++] = he;
    else
      j2++;
  }

  if ((j1 + j2) != num)
    die("Fatal mismatch");

  qsort(hea, j1, sizeof(struct top_hash_entry *), lsa_compare_for_state);
  qsort(hex, jx, sizeof(struct top_hash_entry *), ext_compare_for_state);

  /*
   * This code is a bit tricky, we have a primary LSAs (router and
   * network) that are presented as a node, and secondary LSAs that
   * are presented as a part of a primary node. cnode represents an
   * currently opened node (whose header was presented). The LSAs are
   * sorted to get secondary LSAs just after related primary LSA (if
   * available). We present secondary LSAs only when related primary
   * LSA is opened.
   *
   * AS-external LSAs are stored separately as they might be presented
   * several times (for each area when related ASBR is opened). When
   * the node is closed, related external routes are presented. We
   * also have to take into account that in OSPFv3, there might be
   * more router-LSAs and only the first should be considered as a
   * primary. This is handled by not closing old router-LSA when next
   * one is processed (which is not opened because there is already
   * one opened).
   */

  ix = 0;
  for (i = 0; i < j1; i++)
  {
    he = hea[i];

    /* If there is no opened node, we open the LSA (if appropriate) or skip to the next one */
    if (!cnode)
    {
      if (((he->lsa.type == LSA_T_RT) || (he->lsa.type == LSA_T_NET))
	  && ((he->color == INSPF) || !reachable))
      {
	cnode = &(he->lsa);

	if (he->domain != last_area)
	{
	  cli_msg(-1016, "");
	  cli_msg(-1016, "area %R", he->domain);
	  last_area = he->domain;
	  ix = 0;
	}
      }
      else
	continue;
    }

    ASSERT(cnode && (he->domain == last_area) && (he->lsa.rt == cnode->rt));

    switch (he->lsa.type)
    {
      case LSA_T_RT:
	show_lsa_router(po, he, he->lsa.id == cnode->id, verbose);
	break;

      case LSA_T_NET:
	show_lsa_network(he);
	break;

      case LSA_T_SUM_NET:
	if (cnode->type == LSA_T_RT)
	  show_lsa_sum_net(he);
	break;

      case LSA_T_SUM_RT:
	if (cnode->type == LSA_T_RT)
	  show_lsa_sum_rt(he);
	break;

#ifdef OSPFv3
      case LSA_T_PREFIX:
	show_lsa_prefix(he, cnode);
	break;
#endif

      case LSA_T_EXT:
      case LSA_T_NSSA:
	show_lsa_external(he);
	break;
    }

    /* In these cases, we close the current node */
    if ((i+1 == j1)
	|| (hea[i+1]->domain != last_area)
	|| (hea[i+1]->lsa.rt != cnode->rt)
	|| (hea[i+1]->lsa.type == LSA_T_NET))
    {
      while ((ix < jx) && (hex[ix]->lsa.rt < cnode->rt))
	ix++;

      while ((ix < jx) && (hex[ix]->lsa.rt == cnode->rt))
	show_lsa_external(hex[ix++]);

      cnode = NULL;
    }
  }

  int hdr = 0;
  u32 last_rt = 0xFFFFFFFF;
  for (ix = 0; ix < jx; ix++)
  {
    he = hex[ix];

    /* If it is still marked, we show it now. */
    if (he->domain)
    {
      he->domain = 0;

      if ((he->color != INSPF) && reachable)
	continue;

      if (!hdr)
      {
	cli_msg(-1016, "");
	cli_msg(-1016, "other ASBRs");
	hdr = 1;
      }

      if (he->lsa.rt != last_rt)
      {
	cli_msg(-1016, "");
	cli_msg(-1016, "\trouter %R", he->lsa.rt);
	last_rt = he->lsa.rt;
      }

      show_lsa_external(he);
    }
  }

  cli_msg(0, "");
}


static int
lsa_compare_for_lsadb(const void *p1, const void *p2)
{
  struct top_hash_entry * he1 = * (struct top_hash_entry **) p1;
  struct top_hash_entry * he2 = * (struct top_hash_entry **) p2;
  struct ospf_lsa_header *lsa1 = &(he1->lsa);
  struct ospf_lsa_header *lsa2 = &(he2->lsa);
  int sc1 = LSA_SCOPE(lsa1);
  int sc2 = LSA_SCOPE(lsa2);

  if (sc1 != sc2)
    return sc2 - sc1;

  if (he1->domain != he2->domain)
    return he1->domain - he2->domain;

  if (lsa1->rt != lsa2->rt)
    return lsa1->rt - lsa2->rt;
  
  if (lsa1->id != lsa2->id)
    return lsa1->id - lsa2->id;

  if (lsa1->type != lsa2->type)
    return lsa1->type - lsa2->type;

  return lsa1->sn - lsa2->sn;
}

void
ospf_sh_lsadb(struct lsadb_show_data *ld)
{
  struct proto *p = proto_get_named(ld->name, &proto_ospf);
  struct proto_ospf *po = (struct proto_ospf *) p;
  int num = po->gr->hash_entries;
  unsigned int i, j;
  int last_dscope = -1;
  u32 last_domain = 0;

  if (p->proto_state != PS_UP)
  {
    cli_msg(-1017, "%s: is not up", p->name);
    cli_msg(0, "");
    return;
  }

  if (ld->router == SH_ROUTER_SELF)
    ld->router = po->router_id;

  struct top_hash_entry *hea[num];
  struct top_hash_entry *he;

  j = 0;
  WALK_SLIST(he, po->lsal)
    hea[j++] = he;

  if (j != num)
    die("Fatal mismatch");

  qsort(hea, j, sizeof(struct top_hash_entry *), lsa_compare_for_lsadb);

  for (i = 0; i < j; i++)
  {
    struct ospf_lsa_header *lsa = &(hea[i]->lsa);
    int dscope = LSA_SCOPE(lsa);

    if (ld->scope && (dscope != (ld->scope & 0xf000)))
      continue;

    if ((ld->scope == LSA_SCOPE_AREA) && (hea[i]->domain != ld->area))
      continue;

    /* Ignore high nibble */
    if (ld->type && ((lsa->type & 0x0fff) != (ld->type & 0x0fff)))
      continue;

    if (ld->lsid && (lsa->id != ld->lsid))
      continue;

    if (ld->router && (lsa->rt != ld->router))
      continue;
    
    if ((dscope != last_dscope) || (hea[i]->domain != last_domain))
    {
      cli_msg(-1017, "");
      switch (dscope)
      {
	case LSA_SCOPE_AS:
	  cli_msg(-1017, "Global");
	  break;
	case LSA_SCOPE_AREA:
	  cli_msg(-1017, "Area %R", hea[i]->domain);
	  break;
#ifdef OSPFv3
	case LSA_SCOPE_LINK:
	  {
	    struct iface *ifa = if_find_by_index(hea[i]->domain);
	    cli_msg(-1017, "Link %s", (ifa != NULL) ? ifa->name : "?");
	  }
	  break;
#endif
      }
      cli_msg(-1017, "");
      cli_msg(-1017," Type   LS ID           Router           Age  Sequence  Checksum");

      last_dscope = dscope;
      last_domain = hea[i]->domain;
    }


    cli_msg(-1017," %04x  %-15R %-15R %5u  %08x    %04x",
	    lsa->type, lsa->id, lsa->rt, lsa->age, lsa->sn, lsa->checksum);
  }
  cli_msg(0, "");
}


struct protocol proto_ospf = {
  name:			"OSPF",
  template:		"ospf%d",
  attr_class:		EAP_OSPF,
  preference:		DEF_PREF_OSPF,
  init:			ospf_init,
  dump:			ospf_dump,
  start:		ospf_start,
  shutdown:		ospf_shutdown,
  reconfigure:		ospf_reconfigure,
  get_status:		ospf_get_status,
  get_attr:		ospf_get_attr,
  get_route_info:	ospf_get_route_info
  // show_proto_info:	ospf_sh
};
