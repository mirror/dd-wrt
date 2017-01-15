/*
 *	BIRD -- Routing Information Protocol (RIP)
 *
 *	(c) 1998--1999 Pavel Machek <pavel@ucw.cz>
 *	(c) 2004--2013 Ondrej Filip <feela@network.cz>
 *	(c) 2009--2015 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2009--2015 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Routing Information Protocol (RIP)
 *
 * The RIP protocol is implemented in two files: |rip.c| containing the protocol
 * logic, route management and the protocol glue with BIRD core, and |packets.c|
 * handling RIP packet processing, RX, TX and protocol sockets.
 *
 * Each instance of RIP is described by a structure &rip_proto, which contains
 * an internal RIP routing table, a list of protocol interfaces and the main
 * timer responsible for RIP routing table cleanup.
 *
 * RIP internal routing table contains incoming and outgoing routes. For each
 * network (represented by structure &rip_entry) there is one outgoing route
 * stored directly in &rip_entry and an one-way linked list of incoming routes
 * (structures &rip_rte). The list contains incoming routes from different RIP
 * neighbors, but only routes with the lowest metric are stored (i.e., all
 * stored incoming routes have the same metric).
 *
 * Note that RIP itself does not select outgoing route, that is done by the core
 * routing table. When a new incoming route is received, it is propagated to the
 * RIP table by rip_update_rte() and possibly stored in the list of incoming
 * routes. Then the change may be propagated to the core by rip_announce_rte().
 * The core selects the best route and propagate it to RIP by rip_rt_notify(),
 * which updates outgoing route part of &rip_entry and possibly triggers route
 * propagation by rip_trigger_update().
 *
 * RIP interfaces are represented by structures &rip_iface. A RIP interface
 * contains a per-interface socket, a list of associated neighbors, interface
 * configuration, and state information related to scheduled interface events
 * and running update sessions. RIP interfaces are added and removed based on
 * core interface notifications.
 *
 * There are two RIP interface events - regular updates and triggered updates.
 * Both are managed from the RIP interface timer (rip_iface_timer()). Regular
 * updates are called at fixed interval and propagate the whole routing table,
 * while triggered updates are scheduled by rip_trigger_update() due to some
 * routing table change and propagate only the routes modified since the time
 * they were scheduled. There are also unicast-destined requested updates, but
 * these are sent directly as a reaction to received RIP request message. The
 * update session is started by rip_send_table(). There may be at most one
 * active update session per interface, as the associated state (including the
 * fib iterator) is stored directly in &rip_iface structure.
 *
 * RIP neighbors are represented by structures &rip_neighbor. Compared to
 * neighbor handling in other routing protocols, RIP does not have explicit
 * neighbor discovery and adjacency maintenance, which makes the &rip_neighbor
 * related code a bit peculiar. RIP neighbors are interlinked with core neighbor
 * structures (&neighbor) and use core neighbor notifications to ensure that RIP
 * neighbors are timely removed. RIP neighbors are added based on received route
 * notifications and removed based on core neighbor and RIP interface events.
 *
 * RIP neighbors are linked by RIP routes and use counter to track the number of
 * associated routes, but when these RIP routes timeout, associated RIP neighbor
 * is still alive (with zero counter). When RIP neighbor is removed but still
 * has some associated routes, it is not freed, just changed to detached state
 * (core neighbors and RIP ifaces are unlinked), then during the main timer
 * cleanup phase the associated routes are removed and the &rip_neighbor
 * structure is finally freed.
 *
 * Supported standards:
 * - RFC 1058 - RIPv1
 * - RFC 2453 - RIPv2
 * - RFC 2080 - RIPng
 * - RFC 4822 - RIP cryptographic authentication
 */

#include <stdlib.h>
#include "rip.h"


static inline void rip_lock_neighbor(struct rip_neighbor *n);
static inline void rip_unlock_neighbor(struct rip_neighbor *n);
static inline int rip_iface_link_up(struct rip_iface *ifa);
static inline void rip_kick_timer(struct rip_proto *p);
static inline void rip_iface_kick_timer(struct rip_iface *ifa);
static void rip_iface_timer(timer *timer);
static void rip_trigger_update(struct rip_proto *p);


/*
 *	RIP routes
 */

static void
rip_init_entry(struct fib_node *fn)
{
  // struct rip_entry *en = (void) *fn;

  const uint offset = OFFSETOF(struct rip_entry, routes);
  memset((byte *)fn + offset, 0, sizeof(struct rip_entry) - offset);
}

static struct rip_rte *
rip_add_rte(struct rip_proto *p, struct rip_rte **rp, struct rip_rte *src)
{
  struct rip_rte *rt = sl_alloc(p->rte_slab);

  memcpy(rt, src, sizeof(struct rip_rte));
  rt->next = *rp;
  *rp = rt;

  rip_lock_neighbor(rt->from);

  return rt;
}

static inline void
rip_remove_rte(struct rip_proto *p, struct rip_rte **rp)
{
  struct rip_rte *rt = *rp;

  rip_unlock_neighbor(rt->from);

  *rp = rt->next;
  sl_free(p->rte_slab, rt);
}

static inline int rip_same_rte(struct rip_rte *a, struct rip_rte *b)
{ return a->metric == b->metric && a->tag == b->tag && ipa_equal(a->next_hop, b->next_hop); }

static inline int rip_valid_rte(struct rip_rte *rt)
{ return rt->from->ifa != NULL; }

/**
 * rip_announce_rte - announce route from RIP routing table to the core
 * @p: RIP instance
 * @en: related network
 *
 * The function takes a list of incoming routes from @en, prepare appropriate
 * &rte for the core and propagate it by rte_update().
 */
static void
rip_announce_rte(struct rip_proto *p, struct rip_entry *en)
{
  struct rip_rte *rt = en->routes;

  /* Find first valid rte */
  while (rt && !rip_valid_rte(rt))
    rt = rt->next;

  if (rt)
  {
    /* Update */
    net *n = net_get(p->p.table, en->n.prefix, en->n.pxlen);

    rta a0 = {
      .src = p->p.main_source,
      .source = RTS_RIP,
      .scope = SCOPE_UNIVERSE,
      .cast = RTC_UNICAST
    };

    u8 rt_metric = rt->metric;
    u16 rt_tag = rt->tag;
    struct rip_rte *rt2 = rt->next;

    /* Find second valid rte */
    while (rt2 && !rip_valid_rte(rt2))
      rt2 = rt2->next;

    if (p->ecmp && rt2)
    {
      /* ECMP route */
      struct mpnh *nhs = NULL;
      int num = 0;

      for (rt = en->routes; rt && (num < p->ecmp); rt = rt->next)
      {
	if (!rip_valid_rte(rt))
	    continue;

	struct mpnh *nh = alloca(sizeof(struct mpnh));
	nh->gw = rt->next_hop;
	nh->iface = rt->from->nbr->iface;
	nh->weight = rt->from->ifa->cf->ecmp_weight;
	mpnh_insert(&nhs, nh);
	num++;

	if (rt->tag != rt_tag)
	  rt_tag = 0;
      }

      a0.dest = RTD_MULTIPATH;
      a0.nexthops = nhs;
    }
    else
    {
      /* Unipath route */
      a0.dest = RTD_ROUTER;
      a0.gw = rt->next_hop;
      a0.iface = rt->from->nbr->iface;
      a0.from = rt->from->nbr->addr;
    }

    rta *a = rta_lookup(&a0);
    rte *e = rte_get_temp(a);

    e->u.rip.from = a0.iface;
    e->u.rip.metric = rt_metric;
    e->u.rip.tag = rt_tag;

    e->net = n;
    e->pflags = 0;

    rte_update(&p->p, n, e);
  }
  else
  {
    /* Withdraw */
    net *n = net_find(p->p.table, en->n.prefix, en->n.pxlen);
    rte_update(&p->p, n, NULL);
  }
}

/**
 * rip_update_rte - enter a route update to RIP routing table
 * @p: RIP instance
 * @prefix: network prefix
 * @pxlen: network prefix length
 * @new: a &rip_rte representing the new route
 *
 * The function is called by the RIP packet processing code whenever it receives
 * a reachable route. The appropriate routing table entry is found and the list
 * of incoming routes is updated. Eventually, the change is also propagated to
 * the core by rip_announce_rte(). Note that for unreachable routes,
 * rip_withdraw_rte() should be called instead of rip_update_rte().
 */
void
rip_update_rte(struct rip_proto *p, ip_addr *prefix, int pxlen, struct rip_rte *new)
{
  struct rip_entry *en = fib_get(&p->rtable, prefix, pxlen);
  struct rip_rte *rt, **rp;
  int changed = 0;

  /* If the new route is better, remove all current routes */
  if (en->routes && new->metric < en->routes->metric)
    while (en->routes)
      rip_remove_rte(p, &en->routes);

  /* Find the old route (also set rp for later) */
  for (rp = &en->routes; rt = *rp; rp = &rt->next)
    if (rt->from == new->from)
    {
      if (rip_same_rte(rt, new))
      {
	rt->expires = new->expires;
	return;
      }

      /* Remove the old route */
      rip_remove_rte(p, rp);
      changed = 1;
      break;
    }

  /* If the new route is optimal, add it to the list */
  if (!en->routes || new->metric == en->routes->metric)
  {
    rt = rip_add_rte(p, rp, new);
    changed = 1;
  }

  /* Announce change if on relevant position (the first or any for ECMP) */
  if (changed && (rp == &en->routes || p->ecmp))
    rip_announce_rte(p, en);
}

/**
 * rip_withdraw_rte - enter a route withdraw to RIP routing table
 * @p: RIP instance
 * @prefix: network prefix
 * @pxlen: network prefix length
 * @from: a &rip_neighbor propagating the withdraw
 *
 * The function is called by the RIP packet processing code whenever it receives
 * an unreachable route. The incoming route for given network from nbr @from is
 * removed. Eventually, the change is also propagated by rip_announce_rte().
 */
void
rip_withdraw_rte(struct rip_proto *p, ip_addr *prefix, int pxlen, struct rip_neighbor *from)
{
  struct rip_entry *en = fib_find(&p->rtable, prefix, pxlen);
  struct rip_rte *rt, **rp;

  if (!en)
    return;

  /* Find the old route */
  for (rp = &en->routes; rt = *rp; rp = &rt->next)
    if (rt->from == from)
      break;

  if (!rt)
    return;

  /* Remove the old route */
  rip_remove_rte(p, rp);

  /* Announce change if on relevant position */
  if (rp == &en->routes || p->ecmp)
    rip_announce_rte(p, en);
}

/*
 * rip_rt_notify - core tells us about new route, so store
 * it into our data structures.
 */
static void
rip_rt_notify(struct proto *P, struct rtable *table UNUSED, struct network *net, struct rte *new,
	      struct rte *old UNUSED, struct ea_list *attrs)
{
  struct rip_proto *p = (struct rip_proto *) P;
  struct rip_entry *en;
  int old_metric;

  if (new)
  {
    /* Update */
    u32 rt_metric = ea_get_int(attrs, EA_RIP_METRIC, 1);
    u32 rt_tag = ea_get_int(attrs, EA_RIP_TAG, 0);

    if (rt_metric > p->infinity)
    {
      log(L_WARN "%s: Invalid rip_metric value %u for route %I/%d",
	  p->p.name, rt_metric, net->n.prefix, net->n.pxlen);
      rt_metric = p->infinity;
    }

    if (rt_tag > 0xffff)
    {
      log(L_WARN "%s: Invalid rip_tag value %u for route %I/%d",
	  p->p.name, rt_tag, net->n.prefix, net->n.pxlen);
      rt_metric = p->infinity;
      rt_tag = 0;
    }

    /*
     * Note that we accept exported routes with infinity metric (this could
     * happen if rip_metric is modified in filters). Such entry has infinity
     * metric but is RIP_ENTRY_VALID and therefore is not subject to garbage
     * collection.
     */

    en = fib_get(&p->rtable, &net->n.prefix, net->n.pxlen);

    old_metric = en->valid ? en->metric : -1;

    en->valid = RIP_ENTRY_VALID;
    en->metric = rt_metric;
    en->tag = rt_tag;
    en->from = (new->attrs->src->proto == P) ? new->u.rip.from : NULL;
    en->iface = new->attrs->iface;
    en->next_hop = new->attrs->gw;
  }
  else
  {
    /* Withdraw */
    en = fib_find(&p->rtable, &net->n.prefix, net->n.pxlen);

    if (!en || en->valid != RIP_ENTRY_VALID)
      return;

    old_metric = en->metric;

    en->valid = RIP_ENTRY_STALE;
    en->metric = p->infinity;
    en->tag = 0;
    en->from = NULL;
    en->iface = NULL;
    en->next_hop = IPA_NONE;
  }

  /* Activate triggered updates */
  if (en->metric != old_metric)
  {
    en->changed = now;
    rip_trigger_update(p);
  }
}


/*
 *	RIP neighbors
 */

struct rip_neighbor *
rip_get_neighbor(struct rip_proto *p, ip_addr *a, struct rip_iface *ifa)
{
  neighbor *nbr = neigh_find2(&p->p, a, ifa->iface, 0);

  if (!nbr || (nbr->scope == SCOPE_HOST) || !rip_iface_link_up(ifa))
    return NULL;

  if (nbr->data)
    return nbr->data;

  TRACE(D_EVENTS, "New neighbor %I on %s", *a, ifa->iface->name);

  struct rip_neighbor *n = mb_allocz(p->p.pool, sizeof(struct rip_neighbor));
  n->ifa = ifa;
  n->nbr = nbr;
  nbr->data = n;
  n->csn = nbr->aux;

  add_tail(&ifa->neigh_list, NODE n);

  return n;
}

static void
rip_remove_neighbor(struct rip_proto *p, struct rip_neighbor *n)
{
  neighbor *nbr = n->nbr;

  TRACE(D_EVENTS, "Removing neighbor %I on %s", nbr->addr, nbr->iface->name);

  rem_node(NODE n);
  n->ifa = NULL;
  n->nbr = NULL;
  nbr->data = NULL;
  nbr->aux = n->csn;

  rfree(n->bfd_req);
  n->bfd_req = NULL;
  n->last_seen = 0;

  if (!n->uc)
    mb_free(n);

  /* Related routes are removed in rip_timer() */
  rip_kick_timer(p);
}

static inline void
rip_lock_neighbor(struct rip_neighbor *n)
{
  n->uc++;
}

static inline void
rip_unlock_neighbor(struct rip_neighbor *n)
{
  n->uc--;

  if (!n->nbr && !n->uc)
    mb_free(n);
}

static void
rip_neigh_notify(struct neighbor *nbr)
{
  struct rip_proto *p = (struct rip_proto *) nbr->proto;
  struct rip_neighbor *n = nbr->data;

  if (!n)
    return;

  /*
   * We assume that rip_neigh_notify() is called before rip_if_notify() for
   * IF_CHANGE_DOWN and therefore n->ifa is still valid. We have no such
   * ordering assumption for IF_CHANGE_LINK, so we test link state of the
   * underlying iface instead of just rip_iface state.
   */
  if ((nbr->scope <= 0) || !rip_iface_link_up(n->ifa))
    rip_remove_neighbor(p, n);
}

static void
rip_bfd_notify(struct bfd_request *req)
{
  struct rip_neighbor *n = req->data;
  struct rip_proto *p = n->ifa->rip;

  if (req->down)
  {
    TRACE(D_EVENTS, "BFD session down for nbr %I on %s",
	  n->nbr->addr, n->ifa->iface->name);
    rip_remove_neighbor(p, n);
  }
}

void
rip_update_bfd(struct rip_proto *p, struct rip_neighbor *n)
{
  int use_bfd = n->ifa->cf->bfd && n->last_seen;

  if (use_bfd && !n->bfd_req)
  {
    /*
     * For RIPv2, use the same address as rip_open_socket(). For RIPng, neighbor
     * should contain an address from the same prefix, thus also link-local. It
     * may cause problems if two link-local addresses are assigned to one iface.
     */
    ip_addr saddr = rip_is_v2(p) ? n->ifa->sk->saddr : n->nbr->ifa->ip;
    n->bfd_req = bfd_request_session(p->p.pool, n->nbr->addr, saddr,
				     n->nbr->iface, rip_bfd_notify, n);
  }

  if (!use_bfd && n->bfd_req)
  {
    rfree(n->bfd_req);
    n->bfd_req = NULL;
  }
}


/*
 *	RIP interfaces
 */

static void
rip_iface_start(struct rip_iface *ifa)
{
  struct rip_proto *p = ifa->rip;

  TRACE(D_EVENTS, "Starting interface %s", ifa->iface->name);

  ifa->next_regular = now + (random() % ifa->cf->update_time) + 1;
  ifa->next_triggered = now;	/* Available immediately */
  ifa->want_triggered = 1;	/* All routes in triggered update */
  tm_start(ifa->timer, 1);	/* Or 100 ms */
  ifa->up = 1;

  if (!ifa->cf->passive)
    rip_send_request(ifa->rip, ifa);
}

static void
rip_iface_stop(struct rip_iface *ifa)
{
  struct rip_proto *p = ifa->rip;
  struct rip_neighbor *n;

  TRACE(D_EVENTS, "Stopping interface %s", ifa->iface->name);

  rip_reset_tx_session(p, ifa);

  WALK_LIST_FIRST(n, ifa->neigh_list)
    rip_remove_neighbor(p, n);

  tm_stop(ifa->timer);
  ifa->up = 0;
}

static inline int
rip_iface_link_up(struct rip_iface *ifa)
{
  return !ifa->cf->check_link || (ifa->iface->flags & IF_LINK_UP);
}

static void
rip_iface_update_state(struct rip_iface *ifa)
{
  int up = ifa->sk && rip_iface_link_up(ifa);

  if (up == ifa->up)
    return;

  if (up)
    rip_iface_start(ifa);
  else
    rip_iface_stop(ifa);
}

static void
rip_iface_update_buffers(struct rip_iface *ifa)
{
  if (!ifa->sk)
    return;

  uint rbsize = ifa->cf->rx_buffer ?: ifa->iface->mtu;
  uint tbsize = ifa->cf->tx_length ?: ifa->iface->mtu;
  rbsize = MAX(rbsize, tbsize);

  sk_set_rbsize(ifa->sk, rbsize);
  sk_set_tbsize(ifa->sk, tbsize);

  uint headers = (rip_is_v2(ifa->rip) ? IP4_HEADER_LENGTH : IP6_HEADER_LENGTH) + UDP_HEADER_LENGTH;
  ifa->tx_plen = tbsize - headers;

  if (ifa->cf->auth_type == RIP_AUTH_CRYPTO)
    ifa->tx_plen -= RIP_AUTH_TAIL_LENGTH + max_mac_length(ifa->cf->passwords);
}

static inline void
rip_iface_update_bfd(struct rip_iface *ifa)
{
  struct rip_proto *p = ifa->rip;
  struct rip_neighbor *n;

  WALK_LIST(n, ifa->neigh_list)
    rip_update_bfd(p, n);
}


static void
rip_iface_locked(struct object_lock *lock)
{
  struct rip_iface *ifa = lock->data;
  struct rip_proto *p = ifa->rip;

  if (!rip_open_socket(ifa))
  {
    log(L_ERR "%s: Cannot open socket for %s", p->p.name, ifa->iface->name);
    return;
  }

  rip_iface_update_buffers(ifa);
  rip_iface_update_state(ifa);
}


static struct rip_iface *
rip_find_iface(struct rip_proto *p, struct iface *what)
{
  struct rip_iface *ifa;

  WALK_LIST(ifa, p->iface_list)
    if (ifa->iface == what)
      return ifa;

  return NULL;
}

static void
rip_add_iface(struct rip_proto *p, struct iface *iface, struct rip_iface_config *ic)
{
  struct rip_iface *ifa;

  TRACE(D_EVENTS, "Adding interface %s", iface->name);

  ifa = mb_allocz(p->p.pool, sizeof(struct rip_iface));
  ifa->rip = p;
  ifa->iface = iface;
  ifa->cf = ic;

  if (ipa_nonzero(ic->address))
    ifa->addr = ic->address;
  else if (ic->mode == RIP_IM_MULTICAST)
    ifa->addr = rip_is_v2(p) ? IP4_RIP_ROUTERS : IP6_RIP_ROUTERS;
  else /* Broadcast */
    ifa->addr = iface->addr->brd;

  init_list(&ifa->neigh_list);

  add_tail(&p->iface_list, NODE ifa);

  ifa->timer = tm_new_set(p->p.pool, rip_iface_timer, ifa, 0, 0);

  struct object_lock *lock = olock_new(p->p.pool);
  lock->type = OBJLOCK_UDP;
  lock->port = ic->port;
  lock->iface = iface;
  lock->data = ifa;
  lock->hook = rip_iface_locked;
  ifa->lock = lock;

  olock_acquire(lock);
}

static void
rip_remove_iface(struct rip_proto *p, struct rip_iface *ifa)
{
  rip_iface_stop(ifa);

  TRACE(D_EVENTS, "Removing interface %s", ifa->iface->name);

  rem_node(NODE ifa);

  rfree(ifa->sk);
  rfree(ifa->lock);
  rfree(ifa->timer);

  mb_free(ifa);
}

static int
rip_reconfigure_iface(struct rip_proto *p, struct rip_iface *ifa, struct rip_iface_config *new)
{
  struct rip_iface_config *old = ifa->cf;

  /* Change of these options would require to reset the iface socket */
  if ((new->mode != old->mode) ||
      (new->port != old->port) ||
      (new->tx_tos != old->tx_tos) ||
      (new->tx_priority != old->tx_priority) ||
      (new->ttl_security != old->ttl_security))
    return 0;

  TRACE(D_EVENTS, "Reconfiguring interface %s", ifa->iface->name);

  ifa->cf = new;

  rip_iface_update_buffers(ifa);

  if (ifa->next_regular > (now + new->update_time))
    ifa->next_regular = now + (random() % new->update_time) + 1;

  if (new->check_link != old->check_link)
    rip_iface_update_state(ifa);

  if (new->bfd != old->bfd)
    rip_iface_update_bfd(ifa);

  if (ifa->up)
    rip_iface_kick_timer(ifa);

  return 1;
}

static void
rip_reconfigure_ifaces(struct rip_proto *p, struct rip_config *cf)
{
  struct iface *iface;

  WALK_LIST(iface, iface_list)
  {
    if (! (iface->flags & IF_UP))
      continue;

    struct rip_iface *ifa = rip_find_iface(p, iface);
    struct rip_iface_config *ic = (void *) iface_patt_find(&cf->patt_list, iface, NULL);

    if (ifa && ic)
    {
      if (rip_reconfigure_iface(p, ifa, ic))
	continue;

      /* Hard restart */
      log(L_INFO "%s: Restarting interface %s", p->p.name, ifa->iface->name);
      rip_remove_iface(p, ifa);
      rip_add_iface(p, iface, ic);
    }

    if (ifa && !ic)
      rip_remove_iface(p, ifa);

    if (!ifa && ic)
      rip_add_iface(p, iface, ic);
  }
}

static void
rip_if_notify(struct proto *P, unsigned flags, struct iface *iface)
{
  struct rip_proto *p = (void *) P;
  struct rip_config *cf = (void *) P->cf;

  if (iface->flags & IF_IGNORE)
    return;

  if (flags & IF_CHANGE_UP)
  {
    struct rip_iface_config *ic = (void *) iface_patt_find(&cf->patt_list, iface, NULL);

    if (ic)
      rip_add_iface(p, iface, ic);

    return;
  }

  struct rip_iface *ifa = rip_find_iface(p, iface);

  if (!ifa)
    return;

  if (flags & IF_CHANGE_DOWN)
  {
    rip_remove_iface(p, ifa);
    return;
  }

  if (flags & IF_CHANGE_MTU)
    rip_iface_update_buffers(ifa);

  if (flags & IF_CHANGE_LINK)
    rip_iface_update_state(ifa);
}


/*
 *	RIP timer events
 */

/**
 * rip_timer - RIP main timer hook
 * @t: timer
 *
 * The RIP main timer is responsible for routing table maintenance. Invalid or
 * expired routes (&rip_rte) are removed and garbage collection of stale routing
 * table entries (&rip_entry) is done. Changes are propagated to core tables,
 * route reload is also done here. Note that garbage collection uses a maximal
 * GC time, while interfaces maintain an illusion of per-interface GC times in
 * rip_send_response().
 *
 * Keeping incoming routes and the selected outgoing route are two independent
 * functions, therefore after garbage collection some entries now considered
 * invalid (RIP_ENTRY_DUMMY) still may have non-empty list of incoming routes,
 * while some valid entries (representing an outgoing route) may have that list
 * empty.
 *
 * The main timer is not scheduled periodically but it uses the time of the
 * current next event and the minimal interval of any possible event to compute
 * the time of the next run.
 */
static void
rip_timer(timer *t)
{
  struct rip_proto *p = t->data;
  struct rip_config *cf = (void *) (p->p.cf);
  struct rip_iface *ifa;
  struct rip_neighbor *n, *nn;
  struct fib_iterator fit;
  bird_clock_t next = now + MIN(cf->min_timeout_time, cf->max_garbage_time);
  bird_clock_t expires = 0;

  TRACE(D_EVENTS, "Main timer fired");

  FIB_ITERATE_INIT(&fit, &p->rtable);

  loop:
  FIB_ITERATE_START(&p->rtable, &fit, node)
  {
    struct rip_entry *en = (struct rip_entry *) node;
    struct rip_rte *rt, **rp;
    int changed = 0;

    /* Checking received routes for timeout and for dead neighbors */
    for (rp = &en->routes; rt = *rp; /* rp = &rt->next */)
    {
      if (!rip_valid_rte(rt) || (rt->expires <= now))
      {
	rip_remove_rte(p, rp);
	changed = 1;
	continue;
      }

      next = MIN(next, rt->expires);
      rp = &rt->next;
    }

    /* Propagating eventual change */
    if (changed || p->rt_reload)
    {
      /*
       * We have to restart the iteration because there may be a cascade of
       * synchronous events rip_announce_rte() -> nest table change ->
       * rip_rt_notify() -> p->rtable change, invalidating hidden variables.
       */

      FIB_ITERATE_PUT_NEXT(&fit, &p->rtable, node);
      rip_announce_rte(p, en);
      goto loop;
    }

    /* Checking stale entries for garbage collection timeout */
    if (en->valid == RIP_ENTRY_STALE)
    {
      expires = en->changed + cf->max_garbage_time;

      if (expires <= now)
      {
	// TRACE(D_EVENTS, "entry is too old: %I/%d", en->n.prefix, en->n.pxlen);
	en->valid = 0;
      }
      else
	next = MIN(next, expires);
    }

    /* Remove empty nodes */
    if (!en->valid && !en->routes)
    {
      FIB_ITERATE_PUT(&fit, node);
      fib_delete(&p->rtable, node);
      goto loop;
    }
  }
  FIB_ITERATE_END(node);

  p->rt_reload = 0;

  /* Handling neighbor expiration */
  WALK_LIST(ifa, p->iface_list)
    WALK_LIST_DELSAFE(n, nn, ifa->neigh_list)
      if (n->last_seen)
      {
	expires = n->last_seen + n->ifa->cf->timeout_time;

	if (expires <= now)
	  rip_remove_neighbor(p, n);
	else
	  next = MIN(next, expires);
      }

  tm_start(p->timer, MAX(next - now, 1));
}

static inline void
rip_kick_timer(struct rip_proto *p)
{
  if (p->timer->expires > (now + 1))
    tm_start(p->timer, 1);	/* Or 100 ms */
}

/**
 * rip_iface_timer - RIP interface timer hook
 * @t: timer
 *
 * RIP interface timers are responsible for scheduling both regular and
 * triggered updates. Fixed, delay-independent period is used for regular
 * updates, while minimal separating interval is enforced for triggered updates.
 * The function also ensures that a new update is not started when the old one
 * is still running.
 */
static void
rip_iface_timer(timer *t)
{
  struct rip_iface *ifa = t->data;
  struct rip_proto *p = ifa->rip;
  bird_clock_t period = ifa->cf->update_time;

  if (ifa->cf->passive)
    return;

  TRACE(D_EVENTS, "Interface timer fired for %s", ifa->iface->name);

  if (ifa->tx_active)
  {
    if (now < (ifa->next_regular + period))
      { tm_start(ifa->timer, 1); return; }

    /* We are too late, reset is done by rip_send_table() */
    log(L_WARN "%s: Too slow update on %s, resetting", p->p.name, ifa->iface->name);
  }

  if (now >= ifa->next_regular)
  {
    /* Send regular update, set timer for next period (or following one if necessay) */
    TRACE(D_EVENTS, "Sending regular updates for %s", ifa->iface->name);
    rip_send_table(p, ifa, ifa->addr, 0);
    ifa->next_regular += period * (1 + ((now - ifa->next_regular) / period));
    ifa->want_triggered = 0;
    p->triggered = 0;
  }
  else if (ifa->want_triggered && (now >= ifa->next_triggered))
  {
    /* Send triggered update, enforce interval between triggered updates */
    TRACE(D_EVENTS, "Sending triggered updates for %s", ifa->iface->name);
    rip_send_table(p, ifa, ifa->addr, ifa->want_triggered);
    ifa->next_triggered = now + MIN(5, period / 2 + 1);
    ifa->want_triggered = 0;
    p->triggered = 0;
  }

  tm_start(ifa->timer, ifa->want_triggered ? 1 : (ifa->next_regular - now));
}

static inline void
rip_iface_kick_timer(struct rip_iface *ifa)
{
  if (ifa->timer->expires > (now + 1))
    tm_start(ifa->timer, 1);	/* Or 100 ms */
}

static void
rip_trigger_update(struct rip_proto *p)
{
  if (p->triggered)
    return;

  struct rip_iface *ifa;
  WALK_LIST(ifa, p->iface_list)
  {
    /* Interface not active */
    if (! ifa->up)
      continue;

    /* Already scheduled */
    if (ifa->want_triggered)
      continue;

    TRACE(D_EVENTS, "Scheduling triggered updates for %s", ifa->iface->name);
    ifa->want_triggered = now;
    rip_iface_kick_timer(ifa);
  }

  p->triggered = 1;
}


/*
 *	RIP protocol glue
 */

static struct ea_list *
rip_prepare_attrs(struct linpool *pool, ea_list *next, u8 metric, u16 tag)
{
  struct ea_list *l = lp_alloc(pool, sizeof(struct ea_list) + 2 * sizeof(eattr));

  l->next = next;
  l->flags = EALF_SORTED;
  l->count = 2;

  l->attrs[0].id = EA_RIP_METRIC;
  l->attrs[0].flags = 0;
  l->attrs[0].type = EAF_TYPE_INT | EAF_TEMP;
  l->attrs[0].u.data = metric;

  l->attrs[1].id = EA_RIP_TAG;
  l->attrs[1].flags = 0;
  l->attrs[1].type = EAF_TYPE_INT | EAF_TEMP;
  l->attrs[1].u.data = tag;

  return l;
}

static int
rip_import_control(struct proto *P UNUSED, struct rte **rt, struct ea_list **attrs, struct linpool *pool)
{
  /* Prepare attributes with initial values */
  if ((*rt)->attrs->source != RTS_RIP)
    *attrs = rip_prepare_attrs(pool, *attrs, 1, 0);

  return 0;
}

static int
rip_reload_routes(struct proto *P)
{
  struct rip_proto *p = (struct rip_proto *) P;

  if (p->rt_reload)
    return 1;

  TRACE(D_EVENTS, "Scheduling route reload");
  p->rt_reload = 1;
  rip_kick_timer(p);

  return 1;
}

static struct ea_list *
rip_make_tmp_attrs(struct rte *rt, struct linpool *pool)
{
  return rip_prepare_attrs(pool, NULL, rt->u.rip.metric, rt->u.rip.tag);
}

static void
rip_store_tmp_attrs(struct rte *rt, struct ea_list *attrs)
{
  rt->u.rip.metric = ea_get_int(attrs, EA_RIP_METRIC, 1);
  rt->u.rip.tag = ea_get_int(attrs, EA_RIP_TAG, 0);
}

static int
rip_rte_better(struct rte *new, struct rte *old)
{
  return new->u.rip.metric < old->u.rip.metric;
}

static int
rip_rte_same(struct rte *new, struct rte *old)
{
  return ((new->u.rip.metric == old->u.rip.metric) &&
	  (new->u.rip.tag == old->u.rip.tag) &&
	  (new->u.rip.from == old->u.rip.from));
}


static struct proto *
rip_init(struct proto_config *cfg)
{
  struct proto *P = proto_new(cfg, sizeof(struct rip_proto));

  P->accept_ra_types = RA_OPTIMAL;
  P->if_notify = rip_if_notify;
  P->rt_notify = rip_rt_notify;
  P->neigh_notify = rip_neigh_notify;
  P->import_control = rip_import_control;
  P->reload_routes = rip_reload_routes;
  P->make_tmp_attrs = rip_make_tmp_attrs;
  P->store_tmp_attrs = rip_store_tmp_attrs;
  P->rte_better = rip_rte_better;
  P->rte_same = rip_rte_same;

  return P;
}

static int
rip_start(struct proto *P)
{
  struct rip_proto *p = (void *) P;
  struct rip_config *cf = (void *) (P->cf);

  init_list(&p->iface_list);
  fib_init(&p->rtable, P->pool, sizeof(struct rip_entry), 0, rip_init_entry);
  p->rte_slab = sl_new(P->pool, sizeof(struct rip_rte));
  p->timer = tm_new_set(P->pool, rip_timer, p, 0, 0);

  p->ecmp = cf->ecmp;
  p->infinity = cf->infinity;
  p->triggered = 0;

  p->log_pkt_tbf = (struct tbf){ .rate = 1, .burst = 5 };
  p->log_rte_tbf = (struct tbf){ .rate = 4, .burst = 20 };

  tm_start(p->timer, MIN(cf->min_timeout_time, cf->max_garbage_time));

  return PS_UP;
}

static int
rip_reconfigure(struct proto *P, struct proto_config *c)
{
  struct rip_proto *p = (void *) P;
  struct rip_config *new = (void *) c;
  // struct rip_config *old = (void *) (P->cf);

  if (new->infinity != p->infinity)
    return 0;

  TRACE(D_EVENTS, "Reconfiguring");

  p->p.cf = c;
  p->ecmp = new->ecmp;
  rip_reconfigure_ifaces(p, new);

  p->rt_reload = 1;
  rip_kick_timer(p);

  return 1;
}

static void
rip_get_route_info(rte *rte, byte *buf, ea_list *attrs UNUSED)
{
  buf += bsprintf(buf, " (%d/%d)", rte->pref, rte->u.rip.metric);

  if (rte->u.rip.tag)
    bsprintf(buf, " [%04x]", rte->u.rip.tag);
}

static int
rip_get_attr(eattr *a, byte *buf, int buflen UNUSED)
{
  switch (a->id)
  {
  case EA_RIP_METRIC:
    bsprintf(buf, "metric: %d", a->u.data);
    return GA_FULL;

  case EA_RIP_TAG:
    bsprintf(buf, "tag: %04x", a->u.data);
    return GA_FULL;

  default:
    return GA_UNKNOWN;
  }
}

void
rip_show_interfaces(struct proto *P, char *iff)
{
  struct rip_proto *p = (void *) P;
  struct rip_iface *ifa = NULL;
  struct rip_neighbor *n = NULL;

  if (p->p.proto_state != PS_UP)
  {
    cli_msg(-1021, "%s: is not up", p->p.name);
    cli_msg(0, "");
    return;
  }

  cli_msg(-1021, "%s:", p->p.name);
  cli_msg(-1021, "%-10s %-6s %6s %6s %6s",
	  "Interface", "State", "Metric", "Nbrs", "Timer");

  WALK_LIST(ifa, p->iface_list)
  {
    if (iff && !patmatch(iff, ifa->iface->name))
      continue;

    int nbrs = 0;
    WALK_LIST(n, ifa->neigh_list)
      if (n->last_seen)
	nbrs++;

    int timer = MAX(ifa->next_regular - now, 0);
    cli_msg(-1021, "%-10s %-6s %6u %6u %6u",
	    ifa->iface->name, (ifa->up ? "Up" : "Down"), ifa->cf->metric, nbrs, timer);
  }

  cli_msg(0, "");
}

void
rip_show_neighbors(struct proto *P, char *iff)
{
  struct rip_proto *p = (void *) P;
  struct rip_iface *ifa = NULL;
  struct rip_neighbor *n = NULL;

  if (p->p.proto_state != PS_UP)
  {
    cli_msg(-1022, "%s: is not up", p->p.name);
    cli_msg(0, "");
    return;
  }

  cli_msg(-1022, "%s:", p->p.name);
  cli_msg(-1022, "%-25s %-10s %6s %6s %6s",
	  "IP address", "Interface", "Metric", "Routes", "Seen");

  WALK_LIST(ifa, p->iface_list)
  {
    if (iff && !patmatch(iff, ifa->iface->name))
      continue;

    WALK_LIST(n, ifa->neigh_list)
    {
      if (!n->last_seen)
	continue;

      int timer = now - n->last_seen;
      cli_msg(-1022, "%-25I %-10s %6u %6u %6u",
	      n->nbr->addr, ifa->iface->name, ifa->cf->metric, n->uc, timer);
    }
  }

  cli_msg(0, "");
}

static void
rip_dump(struct proto *P)
{
  struct rip_proto *p = (struct rip_proto *) P;
  struct rip_iface *ifa;
  int i;

  i = 0;
  FIB_WALK(&p->rtable, e)
  {
    struct rip_entry *en = (struct rip_entry *) e;
    debug("RIP: entry #%d: %I/%d via %I dev %s valid %d metric %d age %d s\n",
	  i++, en->n.prefix, en->n.pxlen, en->next_hop, en->iface->name,
	  en->valid, en->metric, now - en->changed);
  }
  FIB_WALK_END;

  i = 0;
  WALK_LIST(ifa, p->iface_list)
  {
    debug("RIP: interface #%d: %s, %I, up = %d, busy = %d\n",
	  i++, ifa->iface->name, ifa->sk ? ifa->sk->daddr : IPA_NONE,
	  ifa->up, ifa->tx_active);
  }
}


struct protocol proto_rip = {
  .name =		"RIP",
  .template =		"rip%d",
  .attr_class =		EAP_RIP,
  .preference =		DEF_PREF_RIP,
  .config_size =	sizeof(struct rip_config),
  .init =		rip_init,
  .dump =		rip_dump,
  .start =		rip_start,
  .reconfigure =	rip_reconfigure,
  .get_route_info =	rip_get_route_info,
  .get_attr =		rip_get_attr
};
