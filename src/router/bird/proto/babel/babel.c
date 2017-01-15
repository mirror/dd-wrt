/*
 *	BIRD -- The Babel protocol
 *
 *	Copyright (c) 2015--2016 Toke Hoiland-Jorgensen
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 *
 *	This file contains the main routines for handling and sending TLVs, as
 *	well as timers and interaction with the nest.
 */

/**
 * DOC: The Babel protocol
 *
 * Babel (RFC6126) is a loop-avoiding distance-vector routing protocol that is
 * robust and efficient both in ordinary wired networks and in wireless mesh
 * networks.
 *
 * The Babel protocol keeps state for each neighbour in a &babel_neighbor
 * struct, tracking received Hello and I Heard You (IHU) messages. A
 * &babel_interface struct keeps hello and update times for each interface, and
 * a separate hello seqno is maintained for each interface.
 *
 * For each prefix, Babel keeps track of both the possible routes (with next hop
 * and router IDs), as well as the feasibility distance for each prefix and
 * router id. The prefix itself is tracked in a &babel_entry struct, while the
 * possible routes for the prefix are tracked as &babel_route entries and the
 * feasibility distance is maintained through &babel_source structures.
 *
 * The main route selection is done in babel_select_route(). This is called when
 * an entry is updated by receiving updates from the network or when modified by
 * internal timers. It performs feasibility checks on the available routes for
 * the prefix and selects the one with the lowest metric to be announced to the
 * core.
 */

#include <stdlib.h>
#include "babel.h"


#define OUR_ROUTE(r) (r->neigh == NULL)

/*
 * Is one number greater or equal than another mod 2^16? This is based on the
 * definition of serial number space in RFC 1982. Note that arguments are of
 * uint type to avoid integer promotion to signed integer.
 */
static inline int ge_mod64k(uint a, uint b)
{ return (u16)(a - b) < 0x8000; }

static void babel_dump_entry(struct babel_entry *e);
static void babel_dump_route(struct babel_route *r);
static void babel_select_route(struct babel_entry *e);
static void babel_send_route_request(struct babel_entry *e, struct babel_neighbor *n);
static void babel_send_wildcard_request(struct babel_iface *ifa);
static int  babel_cache_seqno_request(struct babel_proto *p, ip_addr prefix, u8 plen,
			       u64 router_id, u16 seqno);
static void babel_trigger_iface_update(struct babel_iface *ifa);
static void babel_trigger_update(struct babel_proto *p);
static void babel_send_seqno_request(struct babel_entry *e);
static inline void babel_kick_timer(struct babel_proto *p);
static inline void babel_iface_kick_timer(struct babel_iface *ifa);


/*
 *	Functions to maintain data structures
 */

static void
babel_init_entry(struct fib_node *n)
{
  struct babel_entry *e = (void *) n;
  e->proto = NULL;
  e->selected_in = NULL;
  e->selected_out = NULL;
  e->updated = now;
  init_list(&e->sources);
  init_list(&e->routes);
}

static inline struct babel_entry *
babel_find_entry(struct babel_proto *p, ip_addr prefix, u8 plen)
{
  return fib_find(&p->rtable, &prefix, plen);
}

static struct babel_entry *
babel_get_entry(struct babel_proto *p, ip_addr prefix, u8 plen)
{
  struct babel_entry *e = fib_get(&p->rtable, &prefix, plen);
  e->proto = p;
  return e;
}

static struct babel_source *
babel_find_source(struct babel_entry *e, u64 router_id)
{
  struct babel_source *s;

  WALK_LIST(s, e->sources)
    if (s->router_id == router_id)
      return s;

  return NULL;
}

static struct babel_source *
babel_get_source(struct babel_entry *e, u64 router_id)
{
  struct babel_proto *p = e->proto;
  struct babel_source *s = babel_find_source(e, router_id);

  if (s)
    return s;

  s = sl_alloc(p->source_slab);
  s->router_id = router_id;
  s->expires = now + BABEL_GARBAGE_INTERVAL;
  s->seqno = 0;
  s->metric = BABEL_INFINITY;
  add_tail(&e->sources, NODE s);

  return s;
}

static void
babel_expire_sources(struct babel_entry *e)
{
  struct babel_proto *p = e->proto;
  struct babel_source *n, *nx;

  WALK_LIST_DELSAFE(n, nx, e->sources)
  {
    if (n->expires && n->expires <= now)
    {
      rem_node(NODE n);
      sl_free(p->source_slab, n);
    }
  }
}

static struct babel_route *
babel_find_route(struct babel_entry *e, struct babel_neighbor *n)
{
  struct babel_route *r;

  WALK_LIST(r, e->routes)
    if (r->neigh == n)
      return r;

  return NULL;
}

static struct babel_route *
babel_get_route(struct babel_entry *e, struct babel_neighbor *nbr)
{
  struct babel_proto *p = e->proto;
  struct babel_route *r = babel_find_route(e, nbr);

  if (r)
    return r;

  r = sl_alloc(p->route_slab);
  memset(r, 0, sizeof(*r));
  r->e = e;
  add_tail(&e->routes, NODE r);

  if (nbr)
  {
    r->neigh = nbr;
    r->expires = now + BABEL_GARBAGE_INTERVAL;
    add_tail(&nbr->routes, NODE &r->neigh_route);
  }

  return r;
}

static void
babel_flush_route(struct babel_route *r)
{
  struct babel_proto *p = r->e->proto;

  DBG("Babel: Flush route %I/%d router_id %lR neigh %I\n",
      r->e->n.prefix, r->e->n.pxlen, r->router_id, r->neigh ? r->neigh->addr : IPA_NONE);

  rem_node(NODE r);

  if (r->neigh)
    rem_node(&r->neigh_route);

  if (r->e->selected_in == r)
    r->e->selected_in = NULL;

  if (r->e->selected_out == r)
    r->e->selected_out = NULL;

  sl_free(p->route_slab, r);
}

static void
babel_expire_route(struct babel_route *r)
{
  struct babel_proto *p = r->e->proto;
  struct babel_entry *e = r->e;

  TRACE(D_EVENTS, "Route expiry timer for %I/%d router-id %lR fired",
	e->n.prefix, e->n.pxlen, r->router_id);

  if (r->metric < BABEL_INFINITY)
  {
    r->metric = BABEL_INFINITY;
    r->expires = now + r->expiry_interval;
  }
  else
  {
    babel_flush_route(r);
  }
}

static void
babel_refresh_route(struct babel_route *r)
{
  if (!OUR_ROUTE(r) && (r == r->e->selected_in))
    babel_send_route_request(r->e, r->neigh);

  r->refresh_time = 0;
}

static void
babel_expire_routes(struct babel_proto *p)
{
  struct babel_entry *e;
  struct babel_route *r, *rx;
  struct fib_iterator fit;

  FIB_ITERATE_INIT(&fit, &p->rtable);

loop:
  FIB_ITERATE_START(&p->rtable, &fit, n)
  {
    e = (struct babel_entry *) n;
    int changed = 0;

    WALK_LIST_DELSAFE(r, rx, e->routes)
    {
      if (r->refresh_time && r->refresh_time <= now)
	babel_refresh_route(r);

      if (r->expires && r->expires <= now)
      {
	babel_expire_route(r);
	changed = 1;
      }
    }

    if (changed)
    {
      /*
       * We have to restart the iteration because there may be a cascade of
       * synchronous events babel_select_route() -> nest table change ->
       * babel_rt_notify() -> p->rtable change, invalidating hidden variables.
       */

      FIB_ITERATE_PUT(&fit, n);
      babel_select_route(e);
      goto loop;
    }

    babel_expire_sources(e);

    /* Remove empty entries */
    if (EMPTY_LIST(e->sources) && EMPTY_LIST(e->routes))
    {
      FIB_ITERATE_PUT(&fit, n);
      fib_delete(&p->rtable, e);
      goto loop;
    }
  }
  FIB_ITERATE_END(n);
}

static struct babel_neighbor *
babel_find_neighbor(struct babel_iface *ifa, ip_addr addr)
{
  struct babel_neighbor *nbr;

  WALK_LIST(nbr, ifa->neigh_list)
    if (ipa_equal(nbr->addr, addr))
      return nbr;

  return NULL;
}

static struct babel_neighbor *
babel_get_neighbor(struct babel_iface *ifa, ip_addr addr)
{
  struct babel_neighbor *nbr = babel_find_neighbor(ifa, addr);

  if (nbr)
    return nbr;

  nbr = mb_allocz(ifa->pool, sizeof(struct babel_neighbor));
  nbr->ifa = ifa;
  nbr->addr = addr;
  nbr->txcost = BABEL_INFINITY;
  init_list(&nbr->routes);
  add_tail(&ifa->neigh_list, NODE nbr);

  return nbr;
}

static void
babel_flush_neighbor(struct babel_neighbor *nbr)
{
  struct babel_proto *p = nbr->ifa->proto;
  node *n;

  TRACE(D_EVENTS, "Flushing neighbor %I", nbr->addr);

  WALK_LIST_FIRST(n, nbr->routes)
  {
    struct babel_route *r = SKIP_BACK(struct babel_route, neigh_route, n);
    struct babel_entry *e = r->e;
    int selected = (r == e->selected_in);

    babel_flush_route(r);

    if (selected)
      babel_select_route(e);
  }

  rem_node(NODE nbr);
  mb_free(nbr);
}

static void
babel_expire_ihu(struct babel_neighbor *nbr)
{
  nbr->txcost = BABEL_INFINITY;
}

static void
babel_expire_hello(struct babel_neighbor *nbr)
{
  nbr->hello_map <<= 1;

  if (nbr->hello_cnt < 16)
    nbr->hello_cnt++;

  if (!nbr->hello_map)
    babel_flush_neighbor(nbr);
}

static void
babel_expire_neighbors(struct babel_proto *p)
{
  struct babel_iface *ifa;
  struct babel_neighbor *nbr, *nbx;

  WALK_LIST(ifa, p->interfaces)
  {
    WALK_LIST_DELSAFE(nbr, nbx, ifa->neigh_list)
    {
      if (nbr->ihu_expiry && nbr->ihu_expiry <= now)
        babel_expire_ihu(nbr);

      if (nbr->hello_expiry && nbr->hello_expiry <= now)
        babel_expire_hello(nbr);
    }
  }
}


/*
 *	Best route selection
 */

/*
 * From the RFC (section 3.5.1):
 *
 * a route advertisement carrying the quintuple (prefix, plen, router-id, seqno,
 * metric) is feasible if one of the following conditions holds:
 *
 * - metric is infinite; or
 *
 * - no entry exists in the source table indexed by (id, prefix, plen); or
 *
 * - an entry (prefix, plen, router-id, seqno', metric') exists in the source
 *   table, and either
 *   - seqno' < seqno or
 *   - seqno = seqno' and metric < metric'.
 */
static inline int
babel_is_feasible(struct babel_source *s, u16 seqno, u16 metric)
{
  return !s ||
    (metric == BABEL_INFINITY) ||
    (seqno > s->seqno) ||
    ((seqno == s->seqno) && (metric < s->metric));
}

static u16
babel_compute_rxcost(struct babel_neighbor *n)
{
  struct babel_iface *ifa = n->ifa;
  u8 cnt, missed;
  u16 map=n->hello_map;

  if (!map) return BABEL_INFINITY;
  cnt = u32_popcount(map); // number of bits set
  missed = n->hello_cnt-cnt;

  if (ifa->cf->type == BABEL_IFACE_TYPE_WIRELESS)
  {
    /* ETX - Appendix 2.2 in the RFC.

       beta = prob. of successful transmission.
       rxcost = BABEL_RXCOST_WIRELESS/beta

       Since: beta = 1-missed/n->hello_cnt = cnt/n->hello_cnt
       Then: rxcost = BABEL_RXCOST_WIRELESS * n->hello_cnt / cnt
   */
    if (!cnt) return BABEL_INFINITY;
    return BABEL_RXCOST_WIRELESS * n->hello_cnt / cnt;
  }
  else
  {
    /* k-out-of-j selection - Appendix 2.1 in the RFC. */
    DBG("Babel: Missed %d hellos from %I\n", missed, n->addr);
    /* Link is bad if more than half the expected hellos were lost */
    return (missed > n->hello_cnt/2) ? BABEL_INFINITY : ifa->cf->rxcost;
  }
}


static u16
babel_compute_cost(struct babel_neighbor *n)
{
  struct babel_iface *ifa = n->ifa;
  u16 rxcost = babel_compute_rxcost(n);
  if (rxcost == BABEL_INFINITY) return rxcost;
  else if (ifa->cf->type == BABEL_IFACE_TYPE_WIRELESS)
  {
    /* ETX - Appendix 2.2 in the RFC */
    return (MAX(n->txcost, BABEL_RXCOST_WIRELESS) * rxcost)/BABEL_RXCOST_WIRELESS;
  }
  else
  {
    /* k-out-of-j selection - Appendix 2.1 in the RFC. */
    return n->txcost;
  }
}

/* Simple additive metric - Appendix 3.1 in the RFC */
static u16
babel_compute_metric(struct babel_neighbor *n, uint metric)
{
  metric += babel_compute_cost(n);
  return MIN(metric, BABEL_INFINITY);
}


/**
 * babel_announce_rte - announce selected route to the core
 * @p: Babel protocol instance
 * @e: Babel route entry to announce
 *
 * This function announces a Babel entry to the core if it has a selected
 * incoming path, and retracts it otherwise. If the selected entry has infinite
 * metric, the route is announced as unreachable.
 */
static void
babel_announce_rte(struct babel_proto *p, struct babel_entry *e)
{
  struct babel_route *r = e->selected_in;

  if (r)
  {
    net *n = net_get(p->p.table, e->n.prefix, e->n.pxlen);
    rta A = {
      .src = p->p.main_source,
      .source = RTS_BABEL,
      .scope = SCOPE_UNIVERSE,
      .cast = RTC_UNICAST,
      .dest = r->metric == BABEL_INFINITY ? RTD_UNREACHABLE : RTD_ROUTER,
      .flags = 0,
      .from = r->neigh->addr,
      .iface = r->neigh->ifa->iface,
    };

    if (r->metric < BABEL_INFINITY)
      A.gw = r->next_hop;

    rta *a = rta_lookup(&A);
    rte *rte = rte_get_temp(a);
    rte->u.babel.metric = r->metric;
    rte->u.babel.router_id = r->router_id;
    rte->net = n;
    rte->pflags = 0;

    rte_update(&p->p, n, rte);
  }
  else
  {
    /* Retraction */
    net *n = net_find(p->p.table, e->n.prefix, e->n.pxlen);
    rte_update(&p->p, n, NULL);
  }
}

/**
 * babel_select_route - select best route for given route entry
 * @e: Babel entry to select the best route for
 *
 * Select the best feasible route for a given prefix among the routes received
 * from peers, and propagate it to the nest. This just selects the feasible
 * route with the lowest metric.
 *
 * If no feasible route is available for a prefix that previously had a route
 * selected, a seqno request is sent to try to get a valid route. In the
 * meantime, the route is marked as infeasible in the nest (to blackhole packets
 * going to it, as per the RFC).
 *
 * If no feasible route is available, and no previous route is selected, the
 * route is removed from the nest entirely.
 */
static void
babel_select_route(struct babel_entry *e)
{
  struct babel_proto *p = e->proto;
  struct babel_route *r, *cur = e->selected_in;

  /* try to find the best feasible route */
  WALK_LIST(r, e->routes)
    if (!OUR_ROUTE(r) && /* prevent propagating our own routes back to core */
	(!cur || r->metric < cur->metric) &&
        babel_is_feasible(babel_find_source(e, r->router_id), r->seqno, r->advert_metric))
      cur = r;

  if (cur && !OUR_ROUTE(cur) &&
      ((!e->selected_in && cur->metric < BABEL_INFINITY) ||
       (e->selected_in && cur->metric < e->selected_in->metric)))
  {
    TRACE(D_EVENTS, "Picked new route for prefix %I/%d: router id %lR metric %d",
	  e->n.prefix, e->n.pxlen, cur->router_id, cur->metric);

    e->selected_in = cur;
    e->updated = now;
    babel_announce_rte(p, e);
  }
  else if (!cur || cur->metric == BABEL_INFINITY)
  {
    /* Couldn't find a feasible route. If we have a selected route, that means
       it just became infeasible; so set it's metric to infinite and install it
       (as unreachable), then send a seqno request.

       babel_build_rte() will set the unreachable flag if the metric is BABEL_INFINITY.*/
    if (e->selected_in)
    {
      TRACE(D_EVENTS, "Lost feasible route for prefix %I/%d",
	    e->n.prefix, e->n.pxlen);

      e->selected_in->metric = BABEL_INFINITY;
      e->updated = now;

      babel_send_seqno_request(e);
      babel_announce_rte(p, e);

      /* Section 3.6 of the RFC forbids an infeasible from being selected. This
	 is cleared after announcing the route to the core to make sure an
	 unreachable route is propagated first. */
      e->selected_in = NULL;
    }
    else
    {
      /* No route currently selected, and no new one selected; this means we
	 don't have a route to this destination anymore (and were probably
	 called from an expiry timer). Remove the route from the nest. */
      TRACE(D_EVENTS, "Flushing route for prefix %I/%d", e->n.prefix, e->n.pxlen);

      e->selected_in = NULL;
      e->updated = now;
      babel_announce_rte(p, e);
    }
  }
}

/*
 *	Functions to send replies
 */

static void
babel_send_ack(struct babel_iface *ifa, ip_addr dest, u16 nonce)
{
  struct babel_proto *p = ifa->proto;
  union babel_msg msg = {};

  TRACE(D_PACKETS, "Sending ACK to %I with nonce %d", dest, nonce);

  msg.type = BABEL_TLV_ACK;
  msg.ack.nonce = nonce;

  babel_send_unicast(&msg, ifa, dest);
}

static void
babel_build_ihu(union babel_msg *msg, struct babel_iface *ifa, struct babel_neighbor *n)
{
  struct babel_proto *p = ifa->proto;

  msg->type = BABEL_TLV_IHU;
  msg->ihu.addr = n->addr;
  msg->ihu.rxcost = babel_compute_rxcost(n);
  msg->ihu.interval = ifa->cf->ihu_interval;

  TRACE(D_PACKETS, "Sending IHU for %I with rxcost %d interval %d",
        msg->ihu.addr, msg->ihu.rxcost, msg->ihu.interval);
}

static void
babel_send_ihu(struct babel_iface *ifa, struct babel_neighbor *n)
{
  union babel_msg msg = {};
  babel_build_ihu(&msg, ifa, n);
  babel_send_unicast(&msg, ifa, n->addr);
}

static void
babel_send_ihus(struct babel_iface *ifa)
{
  struct babel_neighbor *n;
  WALK_LIST(n, ifa->neigh_list)
  {
    union babel_msg msg = {};
    babel_build_ihu(&msg, ifa, n);
    babel_enqueue(&msg, ifa);
  }
}

static void
babel_send_hello(struct babel_iface *ifa, u8 send_ihu)
{
  struct babel_proto *p = ifa->proto;
  union babel_msg msg = {};

  msg.type = BABEL_TLV_HELLO;
  msg.hello.seqno = ifa->hello_seqno++;
  msg.hello.interval = ifa->cf->hello_interval;

  TRACE(D_PACKETS, "Sending hello on %s with seqno %d interval %d",
	ifa->ifname, msg.hello.seqno, msg.hello.interval);

  babel_enqueue(&msg, ifa);

  if (send_ihu)
    babel_send_ihus(ifa);
}

static void
babel_send_route_request(struct babel_entry *e, struct babel_neighbor *n)
{
  struct babel_proto *p = e->proto;
  struct babel_iface *ifa = n->ifa;
  union babel_msg msg = {};

  TRACE(D_PACKETS, "Sending route request for %I/%d to %I",
        e->n.prefix, e->n.pxlen, n->addr);

  msg.type = BABEL_TLV_ROUTE_REQUEST;
  msg.route_request.prefix = e->n.prefix;
  msg.route_request.plen = e->n.pxlen;

  babel_send_unicast(&msg, ifa, n->addr);
}

static void
babel_send_wildcard_request(struct babel_iface *ifa)
{
  struct babel_proto *p = ifa->proto;
  union babel_msg msg = {};

  TRACE(D_PACKETS, "Sending wildcard route request on %s",
	ifa->ifname);

  msg.type = BABEL_TLV_ROUTE_REQUEST;
  msg.route_request.full = 1;

  babel_enqueue(&msg, ifa);
}

static void
babel_send_seqno_request(struct babel_entry *e)
{
  struct babel_proto *p = e->proto;
  struct babel_route *r = e->selected_in;
  struct babel_iface *ifa = NULL;
  struct babel_source *s = NULL;
  union babel_msg msg = {};

  s = babel_find_source(e, r->router_id);
  if (!s || !babel_cache_seqno_request(p, e->n.prefix, e->n.pxlen, r->router_id, s->seqno + 1))
    return;

  TRACE(D_PACKETS, "Sending seqno request for %I/%d router-id %lR seqno %d",
	e->n.prefix, e->n.pxlen, r->router_id, s->seqno + 1);

  msg.type = BABEL_TLV_SEQNO_REQUEST;
  msg.seqno_request.plen = e->n.pxlen;
  msg.seqno_request.seqno = s->seqno + 1;
  msg.seqno_request.hop_count = BABEL_INITIAL_HOP_COUNT;
  msg.seqno_request.router_id = r->router_id;
  msg.seqno_request.prefix = e->n.prefix;

  WALK_LIST(ifa, p->interfaces)
    babel_enqueue(&msg, ifa);
}

static void
babel_unicast_seqno_request(struct babel_route *r)
{
  struct babel_entry *e = r->e;
  struct babel_proto *p = e->proto;
  struct babel_iface *ifa = r->neigh->ifa;
  struct babel_source *s = NULL;
  union babel_msg msg = {};

  s = babel_find_source(e, r->router_id);
  if (!s || !babel_cache_seqno_request(p, e->n.prefix, e->n.pxlen, r->router_id, s->seqno + 1))
    return;

  TRACE(D_PACKETS, "Sending seqno request for %I/%d router-id %lR seqno %d",
	e->n.prefix, e->n.pxlen, r->router_id, s->seqno + 1);

  msg.type = BABEL_TLV_SEQNO_REQUEST;
  msg.seqno_request.plen = e->n.pxlen;
  msg.seqno_request.seqno = s->seqno + 1;
  msg.seqno_request.hop_count = BABEL_INITIAL_HOP_COUNT;
  msg.seqno_request.router_id = r->router_id;
  msg.seqno_request.prefix = e->n.prefix;

  babel_send_unicast(&msg, ifa, r->neigh->addr);
}

/**
 * babel_send_update - send route table updates
 * @ifa: Interface to transmit on
 * @changed: Only send entries changed since this time
 *
 * This function produces update TLVs for all entries changed since the time
 * indicated by the &changed parameter and queues them for transmission on the
 * selected interface. During the process, the feasibility distance for each
 * transmitted entry is updated.
 */
static void
babel_send_update(struct babel_iface *ifa, bird_clock_t changed)
{
  struct babel_proto *p = ifa->proto;

  FIB_WALK(&p->rtable, n)
  {
    struct babel_entry *e = (void *) n;
    struct babel_route *r = e->selected_out;

    if (!r)
      continue;

    /* Our own seqno might have changed, in which case we update the routes we
       originate. */
    if ((r->router_id == p->router_id) && (r->seqno < p->update_seqno))
    {
      r->seqno = p->update_seqno;
      e->updated = now;
    }

    /* Skip routes that weren't updated since 'changed' time */
    if (e->updated < changed)
      continue;

    TRACE(D_PACKETS, "Sending update for %I/%d router-id %lR seqno %d metric %d",
	  e->n.prefix, e->n.pxlen, r->router_id, r->seqno, r->metric);

    union babel_msg msg = {};
    msg.type = BABEL_TLV_UPDATE;
    msg.update.plen = e->n.pxlen;
    msg.update.interval = ifa->cf->update_interval;
    msg.update.seqno = r->seqno;
    msg.update.metric = r->metric;
    msg.update.prefix = e->n.prefix;
    msg.update.router_id = r->router_id;

    babel_enqueue(&msg, ifa);

    /* Update feasibility distance for redistributed routes */
    if (!OUR_ROUTE(r))
    {
      struct babel_source *s = babel_get_source(e, r->router_id);
      s->expires = now + BABEL_GARBAGE_INTERVAL;

      if ((msg.update.seqno > s->seqno) ||
	  ((msg.update.seqno == s->seqno) && (msg.update.metric < s->metric)))
      {
	s->seqno = msg.update.seqno;
	s->metric = msg.update.metric;
      }
    }
  }
  FIB_WALK_END;
}

static void
babel_trigger_iface_update(struct babel_iface *ifa)
{
  struct babel_proto *p = ifa->proto;

  /* Interface not active or already scheduled */
  if (!ifa->up || ifa->want_triggered)
    return;

  TRACE(D_EVENTS, "Scheduling triggered updates for %s seqno %d",
	ifa->iface->name, p->update_seqno);

  ifa->want_triggered = now;
  babel_iface_kick_timer(ifa);
}

/* Sends and update on all interfaces. */
static void
babel_trigger_update(struct babel_proto *p)
{
  if (p->triggered)
    return;

  struct babel_iface *ifa;
  WALK_LIST(ifa, p->interfaces)
    babel_trigger_iface_update(ifa);

  p->triggered = 1;
}

/* A retraction is an update with an infinite metric */
static void
babel_send_retraction(struct babel_iface *ifa, ip_addr prefix, int plen)
{
  struct babel_proto *p = ifa->proto;
  union babel_msg msg = {};

  TRACE(D_PACKETS, "Sending retraction for %I/%d seqno %d",
	prefix, plen, p->update_seqno);

  msg.type = BABEL_TLV_UPDATE;
  msg.update.plen = plen;
  msg.update.interval = ifa->cf->update_interval;
  msg.update.seqno = p->update_seqno;
  msg.update.metric = BABEL_INFINITY;
  msg.update.prefix = prefix;

  babel_enqueue(&msg, ifa);
}

static void
babel_send_wildcard_retraction(struct babel_iface *ifa)
{
  struct babel_proto *p = ifa->proto;
  union babel_msg msg = {};

  TRACE(D_PACKETS, "Sending wildcard retraction on %s", ifa->ifname);

  msg.type = BABEL_TLV_UPDATE;
  msg.update.wildcard = 1;
  msg.update.interval = ifa->cf->update_interval;
  msg.update.seqno = p->update_seqno;
  msg.update.metric = BABEL_INFINITY;

  babel_enqueue(&msg, ifa);
}


/*
 *	TLV handler helpers
 */

/* Update hello history according to Appendix A1 of the RFC */
static void
babel_update_hello_history(struct babel_neighbor *n, u16 seqno, u16 interval)
{
  /*
   * Compute the difference between expected and received seqno (modulo 2^16).
   * If the expected and received seqnos are within 16 of each other, the modular
   * difference is going to be less than 16 for one of the directions. Otherwise,
   * the values differ too much, so just reset the state.
   */

  u16 delta = ((uint) seqno - (uint) n->next_hello_seqno);

  if (delta == 0)
  {
    /* Do nothing */
  }
  else if (delta <= 16)
  {
    /* Sending node decreased interval; fast-forward */
    n->hello_map <<= delta;
    n->hello_cnt = MIN(n->hello_cnt + delta, 16);
  }
  else if (delta >= 0xfff0)
  {
    u8 diff = (0xffff - delta);
    /* Sending node increased interval; undo history */
    n->hello_map >>= diff;
    n->hello_cnt = (diff < n->hello_cnt) ? n->hello_cnt - diff : 0;
  }
  else
  {
    /* Note state reset - flush entries */
    n->hello_map = n->hello_cnt = 0;
  }

  /* Current entry */
  n->hello_map = (n->hello_map << 1) | 1;
  n->next_hello_seqno = seqno+1;
  if (n->hello_cnt < 16) n->hello_cnt++;
  n->hello_expiry = now + BABEL_HELLO_EXPIRY_FACTOR(interval);
}

static void
babel_expire_seqno_requests(struct babel_proto *p)
{
  struct babel_seqno_request *n, *nx;
  WALK_LIST_DELSAFE(n, nx, p->seqno_cache)
  {
    if ((n->updated + BABEL_SEQNO_REQUEST_EXPIRY) <= now)
    {
      rem_node(NODE n);
      sl_free(p->seqno_slab, n);
    }
  }
}

/*
 * Checks the seqno request cache for a matching request and returns failure if
 * found. Otherwise, a new entry is stored in the cache.
 */
static int
babel_cache_seqno_request(struct babel_proto *p, ip_addr prefix, u8 plen,
                          u64 router_id, u16 seqno)
{
  struct babel_seqno_request *r;

  WALK_LIST(r, p->seqno_cache)
  {
    if (ipa_equal(r->prefix, prefix) && (r->plen == plen) &&
	(r->router_id == router_id) && (r->seqno == seqno))
      return 0;
  }

  /* no entries found */
  r = sl_alloc(p->seqno_slab);
  r->prefix = prefix;
  r->plen = plen;
  r->router_id = router_id;
  r->seqno = seqno;
  r->updated = now;
  add_tail(&p->seqno_cache, NODE r);

  return 1;
}

static void
babel_forward_seqno_request(struct babel_entry *e,
                            struct babel_msg_seqno_request *in,
                            ip_addr sender)
{
  struct babel_proto *p = e->proto;
  struct babel_route *r;

  TRACE(D_PACKETS, "Forwarding seqno request for %I/%d router-id %lR seqno %d",
	e->n.prefix, e->n.pxlen, in->router_id, in->seqno);

  WALK_LIST(r, e->routes)
  {
    if ((r->router_id == in->router_id) &&
	!OUR_ROUTE(r) &&
	!ipa_equal(r->neigh->addr, sender))
    {
      if (!babel_cache_seqno_request(p, e->n.prefix, e->n.pxlen, in->router_id, in->seqno))
	return;

      union babel_msg msg = {};
      msg.type = BABEL_TLV_SEQNO_REQUEST;
      msg.seqno_request.plen = in->plen;
      msg.seqno_request.seqno = in->seqno;
      msg.seqno_request.hop_count = in->hop_count-1;
      msg.seqno_request.router_id = in->router_id;
      msg.seqno_request.prefix = e->n.prefix;

      babel_send_unicast(&msg, r->neigh->ifa, r->neigh->addr);
      return;
    }
  }
}


/*
 *	TLV handlers
 */

void
babel_handle_ack_req(union babel_msg *m, struct babel_iface *ifa)
{
  struct babel_proto *p = ifa->proto;
  struct babel_msg_ack_req *msg = &m->ack_req;

  TRACE(D_PACKETS, "Handling ACK request nonce %d interval %d",
	msg->nonce, msg->interval);

  babel_send_ack(ifa, msg->sender, msg->nonce);
}

void
babel_handle_hello(union babel_msg *m, struct babel_iface *ifa)
{
  struct babel_proto *p = ifa->proto;
  struct babel_msg_hello *msg = &m->hello;

  TRACE(D_PACKETS, "Handling hello seqno %d interval %d",
	msg->seqno, msg->interval);

  struct babel_neighbor *n = babel_get_neighbor(ifa, msg->sender);
  babel_update_hello_history(n, msg->seqno, msg->interval);
  if (ifa->cf->type == BABEL_IFACE_TYPE_WIRELESS)
    babel_send_ihu(ifa, n);
}

void
babel_handle_ihu(union babel_msg *m, struct babel_iface *ifa)
{
  struct babel_proto *p = ifa->proto;
  struct babel_msg_ihu *msg = &m->ihu;

  /* Ignore IHUs that are not about us */
  if ((msg->ae != BABEL_AE_WILDCARD) && !ipa_equal(msg->addr, ifa->addr))
    return;

  TRACE(D_PACKETS, "Handling IHU rxcost %d interval %d",
	msg->rxcost, msg->interval);

  struct babel_neighbor *n = babel_get_neighbor(ifa, msg->sender);
  n->txcost = msg->rxcost;
  n->ihu_expiry = now + BABEL_IHU_EXPIRY_FACTOR(msg->interval);
}

/**
 * babel_handle_update - handle incoming route updates
 * @m: Incoming update TLV
 * @ifa: Interface the update was received on
 *
 * This function is called as a handler for update TLVs and handles the updating
 * and maintenance of route entries in Babel's internal routing cache. The
 * handling follows the actions described in the Babel RFC, and at the end of
 * each update handling, babel_select_route() is called on the affected entry to
 * optionally update the selected routes and propagate them to the core.
 */
void
babel_handle_update(union babel_msg *m, struct babel_iface *ifa)
{
  struct babel_proto *p = ifa->proto;
  struct babel_msg_update *msg = &m->update;

  struct babel_neighbor *nbr;
  struct babel_entry *e;
  struct babel_source *s;
  struct babel_route *r;
  node *n;
  int feasible;

  TRACE(D_PACKETS, "Handling update for %I/%d with seqno %d metric %d",
	msg->prefix, msg->plen, msg->seqno, msg->metric);

  nbr = babel_find_neighbor(ifa, msg->sender);
  if (!nbr)
  {
    DBG("Babel: Haven't heard from neighbor %I; ignoring update.\n", msg->sender);
    return;
  }

  if (msg->router_id == p->router_id)
  {
    DBG("Babel: Ignoring update for our own router ID.\n");
    return;
  }

  /*
   * RFC section 3.5.4:
   *
   * When a Babel node receives an update (id, prefix, seqno, metric) from a
   * neighbour neigh with a link cost value equal to cost, it checks whether it
   * already has a routing table entry indexed by (neigh, id, prefix).
   *
   * If no such entry exists:
   *
   * o if the update is unfeasible, it is ignored;
   *
   * o if the metric is infinite (the update is a retraction), the update is
   *   ignored;
   *
   * o otherwise, a new route table entry is created, indexed by (neigh, id,
   *   prefix), with seqno equal to seqno and an advertised metric equal to the
   *   metric carried by the update.
   *
   * If such an entry exists:
   *
   * o if the entry is currently installed and the update is unfeasible, then
   *   the behaviour depends on whether the router-ids of the two entries match.
   *   If the router-ids are different, the update is treated as though it were
   *   a retraction (i.e., as though the metric were FFFF hexadecimal). If the
   *   router-ids are equal, the update is ignored;
   *
   * o otherwise (i.e., if either the update is feasible or the entry is not
   *   currently installed), then the entry's sequence number, advertised
   *   metric, metric, and router-id are updated and, unless the advertised
   *   metric is infinite, the route's expiry timer is reset to a small multiple
   *   of the Interval value included in the update.
   */

  /* Retraction */
  if (msg->metric == BABEL_INFINITY)
  {
    if (msg->wildcard)
    {
      /*
       * Special case: This is a retraction of all prefixes announced by this
       * neighbour (see second-to-last paragraph of section 4.4.9 in the RFC).
       */
      WALK_LIST(n, nbr->routes)
      {
	r = SKIP_BACK(struct babel_route, neigh_route, n);
	r->metric = BABEL_INFINITY;
	babel_select_route(r->e);
      }
    }
    else
    {
      e = babel_find_entry(p, msg->prefix, msg->plen);

      if (!e)
	return;

      /* The route entry indexed by neighbour */
      r = babel_find_route(e, nbr);

      if (!r)
	return;

      r->metric = BABEL_INFINITY;
      babel_select_route(e);
    }

    /* Done with retractions */
    return;
  }

  e = babel_get_entry(p, msg->prefix, msg->plen);
  r = babel_find_route(e, nbr); /* the route entry indexed by neighbour */
  s = babel_find_source(e, msg->router_id); /* for feasibility */
  feasible = babel_is_feasible(s, msg->seqno, msg->metric);

  if (!r)
  {
    if (!feasible)
      return;

    r = babel_get_route(e, nbr);
    r->advert_metric = msg->metric;
    r->router_id = msg->router_id;
    r->metric = babel_compute_metric(nbr, msg->metric);
    r->next_hop = msg->next_hop;
    r->seqno = msg->seqno;
  }
  else if (r == r->e->selected_in && !feasible)
  {
    /*
     * Route is installed and update is infeasible - we may lose the route,
     * so send a unicast seqno request (section 3.8.2.2 second paragraph).
     */
    babel_unicast_seqno_request(r);

    if (msg->router_id == r->router_id)
      return;

    /* Treat as retraction */
    r->metric = BABEL_INFINITY;
  }
  else
  {
    /* Last paragraph above - update the entry */
    r->advert_metric = msg->metric;
    r->metric = babel_compute_metric(nbr, msg->metric);
    r->next_hop = msg->next_hop;

    r->router_id = msg->router_id;
    r->seqno = msg->seqno;

    r->expiry_interval = BABEL_ROUTE_EXPIRY_FACTOR(msg->interval);
    r->expires = now + r->expiry_interval;
    if (r->expiry_interval > BABEL_ROUTE_REFRESH_INTERVAL)
      r->refresh_time = now + r->expiry_interval - BABEL_ROUTE_REFRESH_INTERVAL;

    /* If the route is not feasible at this point, it means it is from another
       neighbour than the one currently selected; so send a unicast seqno
       request to try to get a better route (section 3.8.2.2 last paragraph). */
    if (!feasible)
      babel_unicast_seqno_request(r);
  }

  babel_select_route(e);
}

void
babel_handle_route_request(union babel_msg *m, struct babel_iface *ifa)
{
  struct babel_proto *p = ifa->proto;
  struct babel_msg_route_request *msg = &m->route_request;

  /* RFC 6126 3.8.1.1 */

  /* Wildcard request - full update on the interface */
  if (msg->full)
  {
    TRACE(D_PACKETS, "Handling wildcard route request");
    ifa->want_triggered = 1;
    return;
  }

  TRACE(D_PACKETS, "Handling route request for %I/%d", msg->prefix, msg->plen);

  /* Non-wildcard request - see if we have an entry for the route.
     If not, send a retraction, otherwise send an update. */
  struct babel_entry *e = babel_find_entry(p, msg->prefix, msg->plen);
  if (!e)
  {
    babel_send_retraction(ifa, msg->prefix, msg->plen);
  }
  else
  {
    babel_trigger_iface_update(ifa);
    e->updated = now;
  }
}


void
babel_handle_seqno_request(union babel_msg *m, struct babel_iface *ifa)
{
  struct babel_proto *p = ifa->proto;
  struct babel_msg_seqno_request *msg = &m->seqno_request;

  /* RFC 6126 3.8.1.2 */

  TRACE(D_PACKETS, "Handling seqno request for %I/%d router-id %lR seqno %d hop count %d",
	msg->prefix, msg->plen, msg->router_id, msg->seqno, msg->hop_count);

  /* Ignore if we have no such entry or entry has infinite metric */
  struct babel_entry *e = babel_find_entry(p, msg->prefix, msg->plen);
  if (!e || !e->selected_out || (e->selected_out->metric == BABEL_INFINITY))
    return;

  /* Trigger update on incoming interface if we have a selected route with
     different router id or seqno no smaller than requested */
  struct babel_route *r = e->selected_out;
  if ((r->router_id != msg->router_id) || ge_mod64k(r->seqno, msg->seqno))
  {
    babel_trigger_iface_update(ifa);
    e->updated = now;
    return;
  }

  /* Seqno is larger; check if we own the router id */
  if (msg->router_id == p->router_id)
  {
    /* Ours; update seqno and trigger global update */
    p->update_seqno++;
    babel_trigger_update(p);
  }
  else
  {
    /* Not ours; forward if TTL allows it */
    if (msg->hop_count > 1)
      babel_forward_seqno_request(e, msg, msg->sender);
  }
}


/*
 *	Babel interfaces
 */

/**
 * babel_iface_timer - Babel interface timer handler
 * @t: Timer
 *
 * This function is called by the per-interface timer and triggers sending of
 * periodic Hello's and both triggered and periodic updates. Periodic Hello's
 * and updates are simply handled by setting the next_{hello,regular} variables
 * on the interface, and triggering an update (and resetting the variable)
 * whenever 'now' exceeds that value.
 *
 * For triggered updates, babel_trigger_iface_update() will set the
 * want_triggered field on the interface to a timestamp value. If this is set
 * (and the next_triggered time has passed; this is a rate limiting mechanism),
 * babel_send_update() will be called with this timestamp as the second
 * parameter. This causes updates to be send consisting of only the routes that
 * have changed since the time saved in want_triggered.
 *
 * Mostly when an update is triggered, the route being modified will be set to
 * the value of 'now' at the time of the trigger; the >= comparison for
 * selecting which routes to send in the update will make sure this is included.
 */
static void
babel_iface_timer(timer *t)
{
  struct babel_iface *ifa = t->data;
  struct babel_proto *p = ifa->proto;
  bird_clock_t hello_period = ifa->cf->hello_interval;
  bird_clock_t update_period = ifa->cf->update_interval;

  if (now >= ifa->next_hello)
  {
    babel_send_hello(ifa, (ifa->cf->type == BABEL_IFACE_TYPE_WIRELESS ||
                           ifa->hello_seqno % BABEL_IHU_INTERVAL_FACTOR == 0));
    ifa->next_hello +=  hello_period * (1 + (now - ifa->next_hello) / hello_period);
  }

  if (now >= ifa->next_regular)
  {
    TRACE(D_EVENTS, "Sending regular updates on %s", ifa->ifname);
    babel_send_update(ifa, 0);
    ifa->next_regular += update_period * (1 + (now - ifa->next_regular) / update_period);
    ifa->want_triggered = 0;
    p->triggered = 0;
  }
  else if (ifa->want_triggered && (now >= ifa->next_triggered))
  {
    TRACE(D_EVENTS, "Sending triggered updates on %s", ifa->ifname);
    babel_send_update(ifa, ifa->want_triggered);
    ifa->next_triggered = now + MIN(5, update_period / 2 + 1);
    ifa->want_triggered = 0;
    p->triggered = 0;
  }

  bird_clock_t next_event = MIN(ifa->next_hello, ifa->next_regular);
  tm_start(ifa->timer, ifa->want_triggered ? 1 : (next_event - now));
}

static inline void
babel_iface_kick_timer(struct babel_iface *ifa)
{
  if (ifa->timer->expires > (now + 1))
    tm_start(ifa->timer, 1);
}

static void
babel_iface_start(struct babel_iface *ifa)
{
  struct babel_proto *p = ifa->proto;

  TRACE(D_EVENTS, "Starting interface %s", ifa->ifname);

  ifa->next_hello = now + (random() % ifa->cf->hello_interval) + 1;
  ifa->next_regular = now + (random() % ifa->cf->update_interval) + 1;
  ifa->next_triggered = now + MIN(5, ifa->cf->update_interval / 2 + 1);
  ifa->want_triggered = 0;	/* We send an immediate update (below) */
  tm_start(ifa->timer, 1);
  ifa->up = 1;

  babel_send_hello(ifa, 0);
  babel_send_wildcard_retraction(ifa);
  babel_send_wildcard_request(ifa);
  babel_send_update(ifa, 0);	/* Full update */
}

static void
babel_iface_stop(struct babel_iface *ifa)
{
  struct babel_proto *p = ifa->proto;
  struct babel_neighbor *nbr;
  struct babel_route *r;
  node *n;

  TRACE(D_EVENTS, "Stopping interface %s", ifa->ifname);

  /*
   * Rather than just flushing the neighbours, we set the metric of their routes
   * to infinity. This allows us to keep the neighbour hello state for when the
   * interface comes back up. The routes will also be kept until they expire.
   */
  WALK_LIST(nbr, ifa->neigh_list)
  {
    WALK_LIST(n, nbr->routes)
    {
      r = SKIP_BACK(struct babel_route, neigh_route, n);
      r->metric = BABEL_INFINITY;
      r->expires = now + r->expiry_interval;
      babel_select_route(r->e);
    }
  }

  tm_stop(ifa->timer);
  ifa->up = 0;
}

static inline int
babel_iface_link_up(struct babel_iface *ifa)
{
  return !ifa->cf->check_link || (ifa->iface->flags & IF_LINK_UP);
}

static void
babel_iface_update_state(struct babel_iface *ifa)
{
  int up = ifa->sk && babel_iface_link_up(ifa);

  if (up == ifa->up)
    return;

  if (up)
    babel_iface_start(ifa);
  else
    babel_iface_stop(ifa);
}

static void
babel_iface_update_buffers(struct babel_iface *ifa)
{
  if (!ifa->sk)
    return;

  uint mtu = MAX(BABEL_MIN_MTU, ifa->iface->mtu);
  uint rbsize = ifa->cf->rx_buffer ?: mtu;
  uint tbsize = ifa->cf->tx_length ?: mtu;
  rbsize = MAX(rbsize, tbsize);

  sk_set_rbsize(ifa->sk, rbsize);
  sk_set_tbsize(ifa->sk, tbsize);

  ifa->tx_length = tbsize - BABEL_OVERHEAD;
}

static struct babel_iface*
babel_find_iface(struct babel_proto *p, struct iface *what)
{
  struct babel_iface *ifa;

  WALK_LIST (ifa, p->interfaces)
    if (ifa->iface == what)
      return ifa;

  return NULL;
}

static void
babel_iface_locked(struct object_lock *lock)
{
  struct babel_iface *ifa = lock->data;
  struct babel_proto *p = ifa->proto;

  if (!babel_open_socket(ifa))
  {
    log(L_ERR "%s: Cannot open socket for %s", p->p.name, ifa->iface->name);
    return;
  }

  babel_iface_update_buffers(ifa);
  babel_iface_update_state(ifa);
}

static void
babel_add_iface(struct babel_proto *p, struct iface *new, struct babel_iface_config *ic)
{
  struct babel_iface *ifa;

  TRACE(D_EVENTS, "Adding interface %s", new->name);

  pool *pool = rp_new(p->p.pool, new->name);

  ifa = mb_allocz(pool, sizeof(struct babel_iface));
  ifa->proto = p;
  ifa->iface = new;
  ifa->cf = ic;
  ifa->pool = pool;
  ifa->ifname = new->name;

  add_tail(&p->interfaces, NODE ifa);

  struct ifa *addr;
  WALK_LIST(addr, new->addrs)
    if (ipa_is_link_local(addr->ip))
      ifa->addr = addr->ip;

  if (ipa_zero(ifa->addr))
    log(L_WARN "%s: Cannot find link-local addr on %s", p->p.name, new->name);

  init_list(&ifa->neigh_list);
  ifa->hello_seqno = 1;

  ifa->timer = tm_new_set(ifa->pool, babel_iface_timer, ifa, 0, 0);

  init_list(&ifa->msg_queue);
  ifa->send_event = ev_new(ifa->pool);
  ifa->send_event->hook = babel_send_queue;
  ifa->send_event->data = ifa;

  struct object_lock *lock = olock_new(ifa->pool);
  lock->type = OBJLOCK_UDP;
  lock->addr = IP6_BABEL_ROUTERS;
  lock->port = ifa->cf->port;
  lock->iface = ifa->iface;
  lock->hook = babel_iface_locked;
  lock->data = ifa;

  olock_acquire(lock);
}

static void
babel_remove_iface(struct babel_proto *p, struct babel_iface *ifa)
{
  TRACE(D_EVENTS, "Removing interface %s", ifa->iface->name);

  struct babel_neighbor *n;
  WALK_LIST_FIRST(n, ifa->neigh_list)
    babel_flush_neighbor(n);

  rem_node(NODE ifa);

  rfree(ifa->pool); /* contains ifa itself, locks, socket, etc */
}

static void
babel_if_notify(struct proto *P, unsigned flags, struct iface *iface)
{
  struct babel_proto *p = (void *) P;
  struct babel_config *cf = (void *) P->cf;

  if (iface->flags & IF_IGNORE)
    return;

  if (flags & IF_CHANGE_UP)
  {
    struct babel_iface_config *ic = (void *) iface_patt_find(&cf->iface_list, iface, iface->addr);

    /* we only speak multicast */
    if (!(iface->flags & IF_MULTICAST))
      return;

    if (ic)
      babel_add_iface(p, iface, ic);

    return;
  }

  struct babel_iface *ifa = babel_find_iface(p, iface);

  if (!ifa)
    return;

  if (flags & IF_CHANGE_DOWN)
  {
    babel_remove_iface(p, ifa);
    return;
  }

  if (flags & IF_CHANGE_MTU)
    babel_iface_update_buffers(ifa);

  if (flags & IF_CHANGE_LINK)
    babel_iface_update_state(ifa);
}

static int
babel_reconfigure_iface(struct babel_proto *p, struct babel_iface *ifa, struct babel_iface_config *new)
{
  struct babel_iface_config *old = ifa->cf;

  /* Change of these options would require to reset the iface socket */
  if ((new->port != old->port) ||
      (new->tx_tos != old->tx_tos) ||
      (new->tx_priority != old->tx_priority))
    return 0;

  TRACE(D_EVENTS, "Reconfiguring interface %s", ifa->iface->name);

  ifa->cf = new;

  if (ifa->next_hello > (now + new->hello_interval))
    ifa->next_hello = now + (random() % new->hello_interval) + 1;

  if (ifa->next_regular > (now + new->update_interval))
    ifa->next_regular = now + (random() % new->update_interval) + 1;

  if ((new->tx_length != old->tx_length) || (new->rx_buffer != old->rx_buffer))
    babel_iface_update_buffers(ifa);

  if (new->check_link != old->check_link)
    babel_iface_update_state(ifa);

  if (ifa->up)
    babel_iface_kick_timer(ifa);

  return 1;
}

static void
babel_reconfigure_ifaces(struct babel_proto *p, struct babel_config *cf)
{
  struct iface *iface;

  WALK_LIST(iface, iface_list)
  {
    if (! (iface->flags & IF_UP))
      continue;

    struct babel_iface *ifa = babel_find_iface(p, iface);
    struct babel_iface_config *ic = (void *) iface_patt_find(&cf->iface_list, iface, NULL);

    if (ifa && ic)
    {
      if (babel_reconfigure_iface(p, ifa, ic))
	continue;

      /* Hard restart */
      log(L_INFO "%s: Restarting interface %s", p->p.name, ifa->iface->name);
      babel_remove_iface(p, ifa);
      babel_add_iface(p, iface, ic);
    }

    if (ifa && !ic)
      babel_remove_iface(p, ifa);

    if (!ifa && ic)
      babel_add_iface(p, iface, ic);
  }
}


/*
 *	Debugging and info output functions
 */

static void
babel_dump_source(struct babel_source *s)
{
  debug("Source router_id %lR seqno %d metric %d expires %d\n",
	s->router_id, s->seqno, s->metric, s->expires ? s->expires-now : 0);
}

static void
babel_dump_route(struct babel_route *r)
{
  debug("Route neigh %I if %s seqno %d metric %d/%d router_id %lR expires %d\n",
	r->neigh ? r->neigh->addr : IPA_NONE,
        r->neigh ? r->neigh->ifa->ifname : "(none)",
        r->seqno, r->advert_metric, r->metric,
	r->router_id, r->expires ? r->expires-now : 0);
}

static void
babel_dump_entry(struct babel_entry *e)
{
  struct babel_source *s;
  struct babel_route *r;

  debug("Babel: Entry %I/%d:\n", e->n.prefix, e->n.pxlen);

  WALK_LIST(s,e->sources)
  { debug(" "); babel_dump_source(s); }

  WALK_LIST(r,e->routes)
  {
    debug(" ");
    if (r == e->selected_out) debug("*");
    if (r == e->selected_in) debug("+");
    babel_dump_route(r);
  }
}

static void
babel_dump_neighbor(struct babel_neighbor *n)
{
  debug("Neighbor %I txcost %d hello_map %x next seqno %d expires %d/%d\n",
	n->addr, n->txcost, n->hello_map, n->next_hello_seqno,
        n->hello_expiry ? n->hello_expiry - now : 0,
        n->ihu_expiry ? n->ihu_expiry - now : 0);
}

static void
babel_dump_iface(struct babel_iface *ifa)
{
  struct babel_neighbor *n;

  debug("Babel: Interface %s addr %I rxcost %d type %d hello seqno %d intervals %d %d\n",
	ifa->ifname, ifa->addr, ifa->cf->rxcost, ifa->cf->type, ifa->hello_seqno,
	ifa->cf->hello_interval, ifa->cf->update_interval);

  WALK_LIST(n, ifa->neigh_list)
  { debug(" "); babel_dump_neighbor(n); }
}

static void
babel_dump(struct proto *P)
{
  struct babel_proto *p = (struct babel_proto *) P;
  struct babel_iface *ifa;

  debug("Babel: router id %lR update seqno %d\n", p->router_id, p->update_seqno);

  WALK_LIST(ifa, p->interfaces)
    babel_dump_iface(ifa);

  FIB_WALK(&p->rtable, n)
  {
    babel_dump_entry((struct babel_entry *) n);
  }
  FIB_WALK_END;
}

static void
babel_get_route_info(rte *rte, byte *buf, ea_list *attrs UNUSED)
{
  buf += bsprintf(buf, " (%d/%d) [%lR]", rte->pref, rte->u.babel.metric, rte->u.babel.router_id);
}

static int
babel_get_attr(eattr *a, byte *buf, int buflen UNUSED)
{
  switch (a->id)
  {
  case EA_BABEL_METRIC:
    bsprintf(buf, "metric: %d", a->u.data);
    return GA_FULL;

  case EA_BABEL_ROUTER_ID:
  {
    u64 rid = 0;
    memcpy(&rid, a->u.ptr->data, sizeof(u64));
    bsprintf(buf, "router_id: %lR", rid);
    return GA_FULL;
  }

  default:
    return GA_UNKNOWN;
  }
}

void
babel_show_interfaces(struct proto *P, char *iff)
{
  struct babel_proto *p = (void *) P;
  struct babel_iface *ifa = NULL;
  struct babel_neighbor *nbr = NULL;

  if (p->p.proto_state != PS_UP)
  {
    cli_msg(-1023, "%s: is not up", p->p.name);
    cli_msg(0, "");
    return;
  }

  cli_msg(-1023, "%s:", p->p.name);
  cli_msg(-1023, "%-10s %-6s %7s %6s %6s",
	  "Interface", "State", "RX cost", "Nbrs", "Timer");

  WALK_LIST(ifa, p->interfaces)
  {
    if (iff && !patmatch(iff, ifa->iface->name))
      continue;

    int nbrs = 0;
    WALK_LIST(nbr, ifa->neigh_list)
	nbrs++;

    int timer = MIN(ifa->next_regular, ifa->next_hello) - now;
    cli_msg(-1023, "%-10s %-6s %7u %6u %6u",
	    ifa->iface->name, (ifa->up ? "Up" : "Down"), ifa->cf->rxcost, nbrs, MAX(timer, 0));
  }

  cli_msg(0, "");
}

void
babel_show_neighbors(struct proto *P, char *iff)
{
  struct babel_proto *p = (void *) P;
  struct babel_iface *ifa = NULL;
  struct babel_neighbor *n = NULL;
  struct babel_route *r = NULL;

  if (p->p.proto_state != PS_UP)
  {
    cli_msg(-1024, "%s: is not up", p->p.name);
    cli_msg(0, "");
    return;
  }

  cli_msg(-1024, "%s:", p->p.name);
  cli_msg(-1024, "%-25s %-10s %6s %6s %10s",
	  "IP address", "Interface", "Metric", "Routes", "Next hello");

  WALK_LIST(ifa, p->interfaces)
  {
    if (iff && !patmatch(iff, ifa->iface->name))
      continue;

    WALK_LIST(n, ifa->neigh_list)
    {
      int rts = 0;
      WALK_LIST(r, n->routes)
        rts++;

      int timer = n->hello_expiry - now;
      cli_msg(-1024, "%-25I %-10s %6u %6u %10u",
	      n->addr, ifa->iface->name, n->txcost, rts, MAX(timer, 0));
    }
  }

  cli_msg(0, "");
}

void
babel_show_entries(struct proto *P)
{
  struct babel_proto *p = (void *) P;
  struct babel_entry *e = NULL;
  struct babel_source *s = NULL;
  struct babel_route *r = NULL;

  char ipbuf[STD_ADDRESS_P_LENGTH+5];
  char ridbuf[ROUTER_ID_64_LENGTH+1];

  if (p->p.proto_state != PS_UP)
  {
    cli_msg(-1025, "%s: is not up", p->p.name);
    cli_msg(0, "");
    return;
  }

  cli_msg(-1025, "%s:", p->p.name);
  cli_msg(-1025, "%-29s %-23s %6s %5s %7s %7s",
	  "Prefix", "Router ID", "Metric", "Seqno", "Expires", "Sources");

  FIB_WALK(&p->rtable, n)
  {
    e = (struct babel_entry *) n;
    r = e->selected_in ? e->selected_in : e->selected_out;

    int srcs = 0;
    WALK_LIST(s, e->sources)
      srcs++;

    bsprintf(ipbuf, "%I/%u", e->n.prefix, e->n.pxlen);

    if (r)
    {
      if (r->router_id == p->router_id)
        bsprintf(ridbuf, "%s", "<self>");
      else
        bsprintf(ridbuf, "%lR", r->router_id);

      int time = r->expires ? r->expires - now : 0;
      cli_msg(-1025, "%-29s %-23s %6u %5u %7u %7u",
	      ipbuf, ridbuf, r->metric, r->seqno, MAX(time, 0), srcs);
    }
    else
    {
      cli_msg(-1025, "%-29s %-44s %7u", ipbuf, "<pending>", srcs);
    }
  }
  FIB_WALK_END;

  cli_msg(0, "");
}


/*
 *	Babel protocol glue
 */

/**
 * babel_timer - global timer hook
 * @t: Timer
 *
 * This function is called by the global protocol instance timer and handles
 * expiration of routes and neighbours as well as pruning of the seqno request
 * cache.
 */
static void
babel_timer(timer *t)
{
  struct babel_proto *p = t->data;

  babel_expire_routes(p);
  babel_expire_seqno_requests(p);
  babel_expire_neighbors(p);
}

static inline void
babel_kick_timer(struct babel_proto *p)
{
  if (p->timer->expires > (now + 1))
    tm_start(p->timer, 1);
}


static struct ea_list *
babel_prepare_attrs(struct linpool *pool, ea_list *next, uint metric, u64 router_id)
{
  struct ea_list *l = lp_alloc(pool, sizeof(struct ea_list) + 2*sizeof(eattr));
  struct adata *rid = lp_alloc(pool, sizeof(struct adata) + sizeof(u64));
  rid->length = sizeof(u64);
  memcpy(&rid->data, &router_id, sizeof(u64));

  l->next = next;
  l->flags = EALF_SORTED;
  l->count = 2;

  l->attrs[0].id = EA_BABEL_METRIC;
  l->attrs[0].flags = 0;
  l->attrs[0].type = EAF_TYPE_INT | EAF_TEMP;
  l->attrs[0].u.data = metric;

  l->attrs[1].id = EA_BABEL_ROUTER_ID;
  l->attrs[1].flags = 0;
  l->attrs[1].type = EAF_TYPE_OPAQUE | EAF_TEMP;
  l->attrs[1].u.ptr = rid;

  return l;
}


static int
babel_import_control(struct proto *P, struct rte **rt, struct ea_list **attrs, struct linpool *pool)
{
  struct babel_proto *p = (void *) P;

  /* Prepare attributes with initial values */
  if ((*rt)->attrs->source != RTS_BABEL)
    *attrs = babel_prepare_attrs(pool, NULL, 0, p->router_id);

  return 0;
}

static struct ea_list *
babel_make_tmp_attrs(struct rte *rt, struct linpool *pool)
{
  return babel_prepare_attrs(pool, NULL, rt->u.babel.metric, rt->u.babel.router_id);
}

static void
babel_store_tmp_attrs(struct rte *rt, struct ea_list *attrs)
{
  rt->u.babel.metric = ea_get_int(attrs, EA_BABEL_METRIC, 0);
}

/*
 * babel_rt_notify - core tells us about new route (possibly our own),
 * so store it into our data structures.
 */
static void
babel_rt_notify(struct proto *P, struct rtable *table UNUSED, struct network *net,
		struct rte *new, struct rte *old UNUSED, struct ea_list *attrs UNUSED)
{
  struct babel_proto *p = (void *) P;
  struct babel_entry *e;
  struct babel_route *r;

  if (new)
  {
    /* Update */
    e = babel_get_entry(p, net->n.prefix, net->n.pxlen);

    if (new->attrs->src->proto != P)
    {
      r = babel_get_route(e, NULL);
      r->seqno = p->update_seqno;
      r->router_id = p->router_id;
      r->metric = 0;	/* FIXME: should be selectable */
    }
    else
      r = e->selected_in;

    if (r != e->selected_out)
    {
      e->selected_out = r;
      e->updated = now;
      babel_trigger_update(p);
    }
  }
  else
  {
    /* Withdraw */
    e = babel_find_entry(p, net->n.prefix, net->n.pxlen);
    if (!e || !e->selected_out)
      return;

    if (OUR_ROUTE(e->selected_out))
    {
      /*
       * We originate this route, so set its metric to infinity and set an
       * expiry time. This causes a retraction to be sent, and later the route
       * to be flushed once the hold time has passed.
       */
      e->selected_out->metric = BABEL_INFINITY;
      e->selected_out->expires = now + BABEL_HOLD_TIME;
      e->updated = now;
      babel_trigger_update(p);
    }
    else
    {
      /*
       * This is a route originating from someone else that was lost; presumably
       * because an export filter was updated to filter it. This means we can't
       * set the metric to infinity (it would be overridden on subsequent
       * updates from the peer originating the route), so just clear the
       * exported route.
       *
       * This causes peers to expire the route after a while (like if we just
       * shut down), but it's the best we can do in these circumstances; and
       * since export filters presumably aren't updated that often this is
       * acceptable.
       */
      e->selected_out = NULL;
    }
  }
}

static int
babel_rte_better(struct rte *new, struct rte *old)
{
  return new->u.babel.metric < old->u.babel.metric;
}

static int
babel_rte_same(struct rte *new, struct rte *old)
{
  return ((new->u.babel.router_id == old->u.babel.router_id) &&
          (new->u.babel.metric == old->u.babel.metric));
}


static struct proto *
babel_init(struct proto_config *cfg)
{
  struct proto *P = proto_new(cfg, sizeof(struct babel_proto));

  P->accept_ra_types = RA_OPTIMAL;
  P->if_notify = babel_if_notify;
  P->rt_notify = babel_rt_notify;
  P->import_control = babel_import_control;
  P->make_tmp_attrs = babel_make_tmp_attrs;
  P->store_tmp_attrs = babel_store_tmp_attrs;
  P->rte_better = babel_rte_better;
  P->rte_same = babel_rte_same;

  return P;
}

static int
babel_start(struct proto *P)
{
  struct babel_proto *p = (void *) P;
  struct babel_config *cf = (void *) P->cf;

  fib_init(&p->rtable, P->pool, sizeof(struct babel_entry), 0, babel_init_entry);
  init_list(&p->interfaces);
  p->timer = tm_new_set(P->pool, babel_timer, p, 0, 1);
  tm_start(p->timer, 2);
  p->update_seqno = 1;
  p->router_id = proto_get_router_id(&cf->c);

  p->route_slab = sl_new(P->pool, sizeof(struct babel_route));
  p->source_slab = sl_new(P->pool, sizeof(struct babel_source));
  p->msg_slab = sl_new(P->pool, sizeof(struct babel_msg_node));
  p->seqno_slab = sl_new(P->pool, sizeof(struct babel_seqno_request));
  init_list(&p->seqno_cache);

  p->log_pkt_tbf = (struct tbf){ .rate = 1, .burst = 5 };

  return PS_UP;
}

static inline void
babel_iface_shutdown(struct babel_iface *ifa)
{
  if (ifa->sk)
  {
    babel_send_wildcard_retraction(ifa);
    babel_send_queue(ifa);
  }
}

static int
babel_shutdown(struct proto *P)
{
  struct babel_proto *p = (void *) P;
  struct babel_iface *ifa;

  TRACE(D_EVENTS, "Shutdown requested");

  WALK_LIST(ifa, p->interfaces)
    babel_iface_shutdown(ifa);

  return PS_DOWN;
}

static int
babel_reconfigure(struct proto *P, struct proto_config *c)
{
  struct babel_proto *p = (void *) P;
  struct babel_config *new = (void *) c;

  TRACE(D_EVENTS, "Reconfiguring");

  p->p.cf = c;
  babel_reconfigure_ifaces(p, new);

  babel_trigger_update(p);
  babel_kick_timer(p);

  return 1;
}


struct protocol proto_babel = {
  .name =		"Babel",
  .template =		"babel%d",
  .attr_class =		EAP_BABEL,
  .preference =		DEF_PREF_BABEL,
  .config_size =	sizeof(struct babel_config),
  .init =		babel_init,
  .dump =		babel_dump,
  .start =		babel_start,
  .shutdown =		babel_shutdown,
  .reconfigure =	babel_reconfigure,
  .get_route_info =	babel_get_route_info,
  .get_attr =		babel_get_attr
};
