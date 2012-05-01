/*
 *	BIRD -- Routing Tables
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Routing tables
 *
 * Routing tables are probably the most important structures BIRD uses. They
 * hold all the information about known networks, the associated routes and
 * their attributes.
 *
 * There are multiple routing tables (a primary one together with any
 * number of secondary ones if requested by the configuration). Each table
 * is basically a FIB containing entries describing the individual
 * destination networks. For each network (represented by structure &net),
 * there is a one-way linked list of route entries (&rte), the first entry
 * on the list being the best one (i.e., the one we currently use
 * for routing), the order of the other ones is undetermined.
 *
 * The &rte contains information specific to the route (preference, protocol
 * metrics, time of last modification etc.) and a pointer to a &rta structure
 * (see the route attribute module for a precise explanation) holding the
 * remaining route attributes which are expected to be shared by multiple
 * routes in order to conserve memory.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/route.h"
#include "nest/protocol.h"
#include "nest/cli.h"
#include "nest/iface.h"
#include "lib/resource.h"
#include "lib/event.h"
#include "lib/string.h"
#include "conf/conf.h"
#include "filter/filter.h"
#include "lib/string.h"
#include "lib/alloca.h"

pool *rt_table_pool;

static slab *rte_slab;
static linpool *rte_update_pool;

static list routing_tables;

static void rt_format_via(rte *e, byte *via);
static void rt_free_hostcache(rtable *tab);
static void rt_notify_hostcache(rtable *tab, net *net);
static void rt_update_hostcache(rtable *tab);
static void rt_next_hop_update(rtable *tab);
static void rt_prune(rtable *tab);

static inline void rt_schedule_gc(rtable *tab);

/* Like fib_route(), but skips empty net entries */
static net *
net_route(rtable *tab, ip_addr a, int len)
{
  ip_addr a0;
  net *n;

  while (len >= 0)
    {
      a0 = ipa_and(a, ipa_mkmask(len));
      n = fib_find(&tab->fib, &a0, len);
      if (n && n->routes)
	return n;
      len--;
    }
  return NULL;
}

static void
rte_init(struct fib_node *N)
{
  net *n = (net *) N;

  N->flags = 0;
  n->routes = NULL;
}

/**
 * rte_find - find a route
 * @net: network node
 * @p: protocol
 *
 * The rte_find() function returns a route for destination @net
 * which belongs has been defined by protocol @p.
 */
rte *
rte_find(net *net, struct proto *p)
{
  rte *e = net->routes;

  while (e && e->attrs->proto != p)
    e = e->next;
  return e;
}

/**
 * rte_get_temp - get a temporary &rte
 * @a: attributes to assign to the new route (a &rta; in case it's
 * un-cached, rte_update() will create a cached copy automatically)
 *
 * Create a temporary &rte and bind it with the attributes @a.
 * Also set route preference to the default preference set for
 * the protocol.
 */
rte *
rte_get_temp(rta *a)
{
  rte *e = sl_alloc(rte_slab);

  e->attrs = a;
  e->flags = 0;
  e->pref = a->proto->preference;
  return e;
}

rte *
rte_do_cow(rte *r)
{
  rte *e = sl_alloc(rte_slab);

  memcpy(e, r, sizeof(rte));
  e->attrs = rta_clone(r->attrs);
  e->flags = 0;
  return e;
}

static int				/* Actually better or at least as good as */
rte_better(rte *new, rte *old)
{
  int (*better)(rte *, rte *);

  if (!old)
    return 1;
  if (new->pref > old->pref)
    return 1;
  if (new->pref < old->pref)
    return 0;
  if (new->attrs->proto->proto != old->attrs->proto->proto)
    {
      /*
       *  If the user has configured protocol preferences, so that two different protocols
       *  have the same preference, try to break the tie by comparing addresses. Not too
       *  useful, but keeps the ordering of routes unambiguous.
       */
      return new->attrs->proto->proto > old->attrs->proto->proto;
    }
  if (better = new->attrs->proto->rte_better)
    return better(new, old);
  return 0;
}

static void
rte_trace(struct proto *p, rte *e, int dir, char *msg)
{
  byte via[STD_ADDRESS_P_LENGTH+32];

  rt_format_via(e, via);
  log(L_TRACE "%s %c %s %I/%d %s", p->name, dir, msg, e->net->n.prefix, e->net->n.pxlen, via);
}

static inline void
rte_trace_in(unsigned int flag, struct proto *p, rte *e, char *msg)
{
  if (p->debug & flag)
    rte_trace(p, e, '>', msg);
}

static inline void
rte_trace_out(unsigned int flag, struct proto *p, rte *e, char *msg)
{
  if (p->debug & flag)
    rte_trace(p, e, '<', msg);
}

static inline void
do_rte_announce(struct announce_hook *a, int type UNUSED, net *net, rte *new, rte *old, ea_list *tmpa, int refeed)
{
  struct proto *p = a->proto;
  struct filter *filter = p->out_filter;
  struct proto_stats *stats = &p->stats;
  rte *new0 = new;
  rte *old0 = old;
  int ok;

#ifdef CONFIG_PIPE
  /* The secondary direction of the pipe */
  if (proto_is_pipe(p) && (p->table != a->table))
    {
      filter = p->in_filter;
      stats = pipe_get_peer_stats(p);
    }
#endif

  if (new)
    {
      stats->exp_updates_received++;

      char *drop_reason = NULL;
      if ((ok = p->import_control ? p->import_control(p, &new, &tmpa, rte_update_pool) : 0) < 0)
	{
	  stats->exp_updates_rejected++;
	  drop_reason = "rejected by protocol";
	}
      else if (ok)
	rte_trace_out(D_FILTERS, p, new, "forced accept by protocol");
      else if ((filter == FILTER_REJECT) ||
	       (filter && f_run(filter, &new, &tmpa, rte_update_pool, FF_FORCE_TMPATTR) > F_ACCEPT))
	{
	  stats->exp_updates_filtered++;
	  drop_reason = "filtered out";
	}
      if (drop_reason)
	{
	  rte_trace_out(D_FILTERS, p, new, drop_reason);
	  if (new != new0)
	    rte_free(new);
	  new = NULL;
	}
    }
  else
    stats->exp_withdraws_received++;

  /*
   * This is a tricky part - we don't know whether route 'old' was
   * exported to protocol 'p' or was filtered by the export filter.
   * We try tu run the export filter to know this to have a correct
   * value in 'old' argument of rte_update (and proper filter value)
   *
   * FIXME - this is broken because 'configure soft' may change
   * filters but keep routes. Refeed is expected to be called after
   * change of the filters and with old == new, therefore we do not
   * even try to run the filter on an old route, This may lead to 
   * 'spurious withdraws' but ensure that there are no 'missing
   * withdraws'.
   *
   * This is not completely safe as there is a window between
   * reconfiguration and the end of refeed - if a newly filtered
   * route disappears during this period, proper withdraw is not
   * sent (because old would be also filtered) and the route is
   * not refeeded (because it disappeared before that).
   */

  if (old && !refeed)
    {
      if (filter == FILTER_REJECT)
	old = NULL;
      else
	{
	  ea_list *tmpb = p->make_tmp_attrs ? p->make_tmp_attrs(old, rte_update_pool) : NULL;
	  ok = p->import_control ? p->import_control(p, &old, &tmpb, rte_update_pool) : 0;
	  if (ok < 0 || (!ok && filter && f_run(filter, &old, &tmpb, rte_update_pool, FF_FORCE_TMPATTR) > F_ACCEPT))
	    {
	      if (old != old0)
		rte_free(old);
	      old = NULL;
	    }
	}
    }

  /* FIXME - This is broken because of incorrect 'old' value (see above) */
  if (!new && !old)
    return;

  if (new)
    stats->exp_updates_accepted++;
  else
    stats->exp_withdraws_accepted++;

  /* Hack: We do not decrease exp_routes during refeed, we instead
     reset exp_routes at the start of refeed. */
  if (new)
    stats->exp_routes++;
  if (old && !refeed)
    stats->exp_routes--;

  if (p->debug & D_ROUTES)
    {
      if (new && old)
	rte_trace_out(D_ROUTES, p, new, "replaced");
      else if (new)
	rte_trace_out(D_ROUTES, p, new, "added");
      else if (old)
	rte_trace_out(D_ROUTES, p, old, "removed");
    }
  if (!new)
    p->rt_notify(p, a->table, net, NULL, old, NULL);
  else if (tmpa)
    {
      ea_list *t = tmpa;
      while (t->next)
	t = t->next;
      t->next = new->attrs->eattrs;
      p->rt_notify(p, a->table, net, new, old, tmpa);
      t->next = NULL;
    }
  else
    p->rt_notify(p, a->table, net, new, old, new->attrs->eattrs);
  if (new && new != new0)	/* Discard temporary rte's */
    rte_free(new);
  if (old && old != old0)
    rte_free(old);
}

/**
 * rte_announce - announce a routing table change
 * @tab: table the route has been added to
 * @type: type of route announcement (RA_OPTIMAL or RA_ANY)
 * @net: network in question
 * @new: the new route to be announced
 * @old: the previous route for the same network
 * @tmpa: a list of temporary attributes belonging to the new route
 *
 * This function gets a routing table update and announces it
 * to all protocols that acccepts given type of route announcement
 * and are connected to the same table by their announcement hooks.
 *
 * Route announcement of type RA_OPTIMAL si generated when optimal
 * route (in routing table @tab) changes. In that case @old stores the
 * old optimal route.
 *
 * Route announcement of type RA_ANY si generated when any route (in
 * routing table @tab) changes In that case @old stores the old route
 * from the same protocol.
 *
 * For each appropriate protocol, we first call its import_control()
 * hook which performs basic checks on the route (each protocol has a
 * right to veto or force accept of the route before any filter is
 * asked) and adds default values of attributes specific to the new
 * protocol (metrics, tags etc.).  Then it consults the protocol's
 * export filter and if it accepts the route, the rt_notify() hook of
 * the protocol gets called.
 */
static void
rte_announce(rtable *tab, unsigned type, net *net, rte *new, rte *old, ea_list *tmpa)
{
  struct announce_hook *a;

  if (type == RA_OPTIMAL)
    {
      if (new)
	new->attrs->proto->stats.pref_routes++;
      if (old)
	old->attrs->proto->stats.pref_routes--;

      if (tab->hostcache)
	rt_notify_hostcache(tab, net);
    }

  WALK_LIST(a, tab->hooks)
    {
      ASSERT(a->proto->core_state == FS_HAPPY || a->proto->core_state == FS_FEEDING);
      if (a->proto->accept_ra_types == type)
	do_rte_announce(a, type, net, new, old, tmpa, 0);
    }
}


static inline int
rte_validate(rte *e)
{
  int c;
  net *n = e->net;

  if ((n->n.pxlen > BITS_PER_IP_ADDRESS) || !ip_is_prefix(n->n.prefix,n->n.pxlen))
    {
      log(L_WARN "Ignoring bogus prefix %I/%d received via %s",
	  n->n.prefix, n->n.pxlen, e->sender->name);
      return 0;
    }

  c = ipa_classify_net(n->n.prefix);
  if ((c < 0) || !(c & IADDR_HOST) || ((c & IADDR_SCOPE_MASK) <= SCOPE_LINK))
    {
      log(L_WARN "Ignoring bogus route %I/%d received via %s",
	  n->n.prefix, n->n.pxlen, e->sender->name);
      return 0;
    }

  return 1;
}

/**
 * rte_free - delete a &rte
 * @e: &rte to be deleted
 *
 * rte_free() deletes the given &rte from the routing table it's linked to.
 */
void
rte_free(rte *e)
{
  if (e->attrs->aflags & RTAF_CACHED)
    rta_free(e->attrs);
  sl_free(rte_slab, e);
}

static inline void
rte_free_quick(rte *e)
{
  rta_free(e->attrs);
  sl_free(rte_slab, e);
}

static int
rte_same(rte *x, rte *y)
{
  return
    x->attrs == y->attrs &&
    x->flags == y->flags &&
    x->pflags == y->pflags &&
    x->pref == y->pref &&
    (!x->attrs->proto->rte_same || x->attrs->proto->rte_same(x, y));
}

static void
rte_recalculate(rtable *table, net *net, struct proto *p, struct proto *src, rte *new, ea_list *tmpa)
{
  struct proto_stats *stats = &p->stats;
  rte *old_best = net->routes;
  rte *old = NULL;
  rte **k, *r, *s;

#ifdef CONFIG_PIPE
  if (proto_is_pipe(p) && (p->table == table))
    stats = pipe_get_peer_stats(p);
#endif

  k = &net->routes;			/* Find and remove original route from the same protocol */
  while (old = *k)
    {
      if (old->attrs->proto == src)
	{
	  /* If there is the same route in the routing table but from
	   * a different sender, then there are two paths from the
	   * source protocol to this routing table through transparent
	   * pipes, which is not allowed.
	   *
	   * We log that and ignore the route. If it is withdraw, we
	   * ignore it completely (there might be 'spurious withdraws',
	   * see FIXME in do_rte_announce())
	   */
	  if (old->sender != p)
	    {
	      if (new)
		{
		  log(L_ERR "Pipe collision detected when sending %I/%d to table %s",
		      net->n.prefix, net->n.pxlen, table->name);
		  rte_free_quick(new);
		}
	      return;
	    }

	  if (new && rte_same(old, new))
	    {
	      /* No changes, ignore the new route */
	      stats->imp_updates_ignored++;
	      rte_trace_in(D_ROUTES, p, new, "ignored");
	      rte_free_quick(new);
#ifdef CONFIG_RIP
	      /* lastmod is used internally by RIP as the last time
		 when the route was received. */
	      if (src->proto == &proto_rip)
		old->lastmod = now;
#endif
	      return;
	    }
	  *k = old->next;
	  break;
	}
      k = &old->next;
    }

  if (!old && !new)
    {
      stats->imp_withdraws_ignored++;
      return;
    }

  if (new)
    stats->imp_updates_accepted++;
  else
    stats->imp_withdraws_accepted++;

  if (new)
    stats->imp_routes++;
  if (old)
    stats->imp_routes--;

  rte_announce(table, RA_ANY, net, new, old, tmpa);

  if (src->rte_recalculate && src->rte_recalculate(table, net, new, old, old_best))
    goto do_recalculate;

  if (new && rte_better(new, old_best))
    {
      /* The first case - the new route is cleary optimal, we link it
	 at the first position and announce it */

      rte_trace_in(D_ROUTES, p, new, "added [best]");
      rte_announce(table, RA_OPTIMAL, net, new, old_best, tmpa);
      new->next = net->routes;
      net->routes = new;
    }
  else if (old == old_best)
    {
      /* The second case - the old best route disappeared, we add the
	 new route (if we have any) to the list (we don't care about
	 position) and then we elect the new optimal route and relink
	 that route at the first position and announce it. New optimal
	 route might be NULL if there is no more routes */

    do_recalculate:
      /* Add the new route to the list */
      if (new)
	{
	  rte_trace_in(D_ROUTES, p, new, "added");
	  new->next = net->routes;
	  net->routes = new;
	}

      /* Find new optimal route */
      r = NULL;
      for (s=net->routes; s; s=s->next)
	if (rte_better(s, r))
	  r = s;

      /* Announce optimal route */
      rte_announce(table, RA_OPTIMAL, net, r, old_best, tmpa);

      /* And relink it (if there is any) */
      if (r)
	{
	  k = &net->routes;
	  while (s = *k)
	    {
	      if (s == r)
		{
		  *k = r->next;
		  break;
		}
	      k = &s->next;
	    }
	  r->next = net->routes;
	  net->routes = r;
	}
      else if (table->gc_counter++ >= table->config->gc_max_ops &&
	       table->gc_time + table->config->gc_min_time <= now)
	rt_schedule_gc(table);
    }
  else if (new)
    {
      /* The third case - the new route is not better than the old
	 best route (therefore old_best != NULL) and the old best
	 route was not removed (therefore old_best == net->routes).
	 We just link the new route after the old best route. */

      ASSERT(net->routes != NULL);
      new->next = net->routes->next;
      net->routes->next = new;
      rte_trace_in(D_ROUTES, p, new, "added");
    }

  /* Log the route removal */
  if (!new && old && (p->debug & D_ROUTES))
    {
      if (old != old_best)
	rte_trace_in(D_ROUTES, p, old, "removed");
      else if (net->routes)
	rte_trace_in(D_ROUTES, p, old, "removed [replaced]");
      else
	rte_trace_in(D_ROUTES, p, old, "removed [sole]");
    }

  if (old)
    {
      if (p->rte_remove)
	p->rte_remove(net, old);
      rte_free_quick(old);
    }
  if (new)
    {
      new->lastmod = now;
      if (p->rte_insert)
	p->rte_insert(net, new);
    }
}

static int rte_update_nest_cnt;		/* Nesting counter to allow recursive updates */

static inline void
rte_update_lock(void)
{
  rte_update_nest_cnt++;
}

static inline void
rte_update_unlock(void)
{
  if (!--rte_update_nest_cnt)
    lp_flush(rte_update_pool);
}

/**
 * rte_update - enter a new update to a routing table
 * @table: table to be updated
 * @net: network node
 * @p: protocol submitting the update
 * @src: protocol originating the update
 * @new: a &rte representing the new route or %NULL for route removal.
 *
 * This function is called by the routing protocols whenever they discover
 * a new route or wish to update/remove an existing route. The right announcement
 * sequence is to build route attributes first (either un-cached with @aflags set
 * to zero or a cached one using rta_lookup(); in this case please note that
 * you need to increase the use count of the attributes yourself by calling
 * rta_clone()), call rte_get_temp() to obtain a temporary &rte, fill in all
 * the appropriate data and finally submit the new &rte by calling rte_update().
 *
 * @src specifies the protocol that originally created the route and the meaning
 * of protocol-dependent data of @new. If @new is not %NULL, @src have to be the
 * same value as @new->attrs->proto. @p specifies the protocol that called
 * rte_update(). In most cases it is the same protocol as @src. rte_update()
 * stores @p in @new->sender;
 *
 * When rte_update() gets any route, it automatically validates it (checks,
 * whether the network and next hop address are valid IP addresses and also
 * whether a normal routing protocol doesn't try to smuggle a host or link
 * scope route to the table), converts all protocol dependent attributes stored
 * in the &rte to temporary extended attributes, consults import filters of the
 * protocol to see if the route should be accepted and/or its attributes modified,
 * stores the temporary attributes back to the &rte.
 *
 * Now, having a "public" version of the route, we
 * automatically find any old route defined by the protocol @src
 * for network @n, replace it by the new one (or removing it if @new is %NULL),
 * recalculate the optimal route for this destination and finally broadcast
 * the change (if any) to all routing protocols by calling rte_announce().
 *
 * All memory used for attribute lists and other temporary allocations is taken
 * from a special linear pool @rte_update_pool and freed when rte_update()
 * finishes.
 */

void
rte_update(rtable *table, net *net, struct proto *p, struct proto *src, rte *new)
{
  ea_list *tmpa = NULL;
  struct proto_stats *stats = &p->stats;

#ifdef CONFIG_PIPE
  if (proto_is_pipe(p) && (p->table == table))
    stats = pipe_get_peer_stats(p);
#endif

  rte_update_lock();
  if (new)
    {
      new->sender = p;
      struct filter *filter = p->in_filter;

      /* Do not filter routes going through the pipe, 
	 they are filtered in the export filter only. */
#ifdef CONFIG_PIPE
      if (proto_is_pipe(p))
	filter = FILTER_ACCEPT;
#endif

      stats->imp_updates_received++;
      if (!rte_validate(new))
	{
	  rte_trace_in(D_FILTERS, p, new, "invalid");
	  stats->imp_updates_invalid++;
	  goto drop;
	}
      if (filter == FILTER_REJECT)
	{
	  stats->imp_updates_filtered++;
	  rte_trace_in(D_FILTERS, p, new, "filtered out");
	  goto drop;
	}
      if (src->make_tmp_attrs)
	tmpa = src->make_tmp_attrs(new, rte_update_pool);
      if (filter)
	{
	  ea_list *old_tmpa = tmpa;
	  int fr = f_run(filter, &new, &tmpa, rte_update_pool, 0);
	  if (fr > F_ACCEPT)
	    {
	      stats->imp_updates_filtered++;
	      rte_trace_in(D_FILTERS, p, new, "filtered out");
	      goto drop;
	    }
	  if (tmpa != old_tmpa && src->store_tmp_attrs)
	    src->store_tmp_attrs(new, tmpa);
	}
      if (!(new->attrs->aflags & RTAF_CACHED)) /* Need to copy attributes */
	new->attrs = rta_lookup(new->attrs);
      new->flags |= REF_COW;
    }
  else
    stats->imp_withdraws_received++;

  rte_recalculate(table, net, p, src, new, tmpa);
  rte_update_unlock();
  return;

drop:
  rte_free(new);
  rte_recalculate(table, net, p, src, NULL, NULL);
  rte_update_unlock();
}

/* Independent call to rte_announce(), used from next hop
   recalculation, outside of rte_update(). new must be non-NULL */
static inline void 
rte_announce_i(rtable *tab, unsigned type, net *n, rte *new, rte *old)
{
  struct proto *src;
  ea_list *tmpa;

  rte_update_lock();
  src = new->attrs->proto;
  tmpa = src->make_tmp_attrs ? src->make_tmp_attrs(new, rte_update_pool) : NULL;
  rte_announce(tab, type, n, new, old, tmpa);
  rte_update_unlock();
}

void
rte_discard(rtable *t, rte *old)	/* Non-filtered route deletion, used during garbage collection */
{
  rte_update_lock();
  rte_recalculate(t, old->net, old->sender, old->attrs->proto, NULL, NULL);
  rte_update_unlock();
}

/**
 * rte_dump - dump a route
 * @e: &rte to be dumped
 *
 * This functions dumps contents of a &rte to debug output.
 */
void
rte_dump(rte *e)
{
  net *n = e->net;
  if (n)
    debug("%-1I/%2d ", n->n.prefix, n->n.pxlen);
  else
    debug("??? ");
  debug("KF=%02x PF=%02x pref=%d lm=%d ", n->n.flags, e->pflags, e->pref, now-e->lastmod);
  rta_dump(e->attrs);
  if (e->attrs->proto->proto->dump_attrs)
    e->attrs->proto->proto->dump_attrs(e);
  debug("\n");
}

/**
 * rt_dump - dump a routing table
 * @t: routing table to be dumped
 *
 * This function dumps contents of a given routing table to debug output.
 */
void
rt_dump(rtable *t)
{
  rte *e;
  net *n;
  struct announce_hook *a;

  debug("Dump of routing table <%s>\n", t->name);
#ifdef DEBUGGING
  fib_check(&t->fib);
#endif
  FIB_WALK(&t->fib, fn)
    {
      n = (net *) fn;
      for(e=n->routes; e; e=e->next)
	rte_dump(e);
    }
  FIB_WALK_END;
  WALK_LIST(a, t->hooks)
    debug("\tAnnounces routes to protocol %s\n", a->proto->name);
  debug("\n");
}

/**
 * rt_dump_all - dump all routing tables
 *
 * This function dumps contents of all routing tables to debug output.
 */
void
rt_dump_all(void)
{
  rtable *t;

  WALK_LIST(t, routing_tables)
    rt_dump(t);
}

static inline void
rt_schedule_gc(rtable *tab)
{
  if (tab->gc_scheduled)
    return;

  tab->gc_scheduled = 1;
  ev_schedule(tab->rt_event);
}

static inline void
rt_schedule_hcu(rtable *tab)
{
  if (tab->hcu_scheduled)
    return;

  tab->hcu_scheduled = 1;
  ev_schedule(tab->rt_event);
}

static inline void
rt_schedule_nhu(rtable *tab)
{
  if (tab->nhu_state == 0)
    ev_schedule(tab->rt_event);

  /* state change 0->1, 2->3 */
  tab->nhu_state |= 1;
}

static void
rt_event(void *ptr)
{
  rtable *tab = ptr;

  if (tab->hcu_scheduled)
    rt_update_hostcache(tab);

  if (tab->nhu_state)
    rt_next_hop_update(tab);

  if (tab->gc_scheduled)
    rt_prune(tab);
}

void
rt_setup(pool *p, rtable *t, char *name, struct rtable_config *cf)
{
  bzero(t, sizeof(*t));
  fib_init(&t->fib, p, sizeof(net), 0, rte_init);
  t->name = name;
  t->config = cf;
  init_list(&t->hooks);
  if (cf)
    {
      t->rt_event = ev_new(p);
      t->rt_event->hook = rt_event;
      t->rt_event->data = t;
      t->gc_time = now;
    }
}

/**
 * rt_init - initialize routing tables
 *
 * This function is called during BIRD startup. It initializes the
 * routing table module.
 */
void
rt_init(void)
{
  rta_init();
  rt_table_pool = rp_new(&root_pool, "Routing tables");
  rte_update_pool = lp_new(rt_table_pool, 4080);
  rte_slab = sl_new(rt_table_pool, sizeof(rte));
  init_list(&routing_tables);
}

/**
 * rt_prune - prune a routing table
 * @tab: routing table to be pruned
 *
 * This function is called whenever a protocol shuts down. It scans
 * the routing table and removes all routes belonging to inactive
 * protocols and also stale network entries.
 */
static void
rt_prune(rtable *tab)
{
  struct fib_iterator fit;
  int rcnt = 0, rdel = 0, ncnt = 0, ndel = 0;

  DBG("Pruning route table %s\n", tab->name);
#ifdef DEBUGGING
  fib_check(&tab->fib);
#endif
  FIB_ITERATE_INIT(&fit, &tab->fib);
again:
  FIB_ITERATE_START(&tab->fib, &fit, f)
    {
      net *n = (net *) f;
      rte *e;
      ncnt++;
    rescan:
      for (e=n->routes; e; e=e->next, rcnt++)
	if (e->sender->core_state != FS_HAPPY &&
	    e->sender->core_state != FS_FEEDING)
	  {
	    rte_discard(tab, e);
	    rdel++;
	    goto rescan;
	  }
      if (!n->routes)		/* Orphaned FIB entry? */
	{
	  FIB_ITERATE_PUT(&fit, f);
	  fib_delete(&tab->fib, f);
	  ndel++;
	  goto again;
	}
    }
  FIB_ITERATE_END(f);
  DBG("Pruned %d of %d routes and %d of %d networks\n", rdel, rcnt, ndel, ncnt);
#ifdef DEBUGGING
  fib_check(&tab->fib);
#endif
  tab->gc_counter = 0;
  tab->gc_time = now;
  tab->gc_scheduled = 0;
}

/**
 * rt_prune_all - prune all routing tables
 *
 * This function calls rt_prune() for all known routing tables.
 */
void
rt_prune_all(void)
{
  rtable *t;

  WALK_LIST(t, routing_tables)
    rt_prune(t);
}

void
rt_preconfig(struct config *c)
{
  struct symbol *s = cf_find_symbol("master");

  init_list(&c->tables);
  c->master_rtc = rt_new_table(s);
}


/* 
 * Some functions for handing internal next hop updates
 * triggered by rt_schedule_nhu().
 */

static inline int
rta_next_hop_outdated(rta *a)
{
  struct hostentry *he = a->hostentry;

  if (!he)
    return 0;

  if (!he->src)
    return a->dest != RTD_UNREACHABLE;

  return (a->iface != he->src->iface) || !ipa_equal(a->gw, he->gw) ||
    (a->dest != he->dest) || (a->igp_metric != he->igp_metric) ||
    !mpnh_same(a->nexthops, he->src->nexthops);
}

static inline void
rta_apply_hostentry(rta *a, struct hostentry *he)
{
  a->hostentry = he;
  a->iface = he->src ? he->src->iface : NULL;
  a->gw = he->gw;
  a->dest = he->dest;
  a->igp_metric = he->igp_metric;
  a->nexthops = he->src ? he->src->nexthops : NULL;
}

static inline rte *
rt_next_hop_update_rte(rtable *tab, rte *old)
{
  rta a;
  memcpy(&a, old->attrs, sizeof(rta));
  rta_apply_hostentry(&a, old->attrs->hostentry);
  a.aflags = 0;

  rte *e = sl_alloc(rte_slab);
  memcpy(e, old, sizeof(rte));
  e->attrs = rta_lookup(&a);

  return e;
}

static inline int
rt_next_hop_update_net(rtable *tab, net *n)
{
  rte **k, *e, *new, *old_best, **new_best;
  int count = 0;
  int free_old_best = 0;

  old_best = n->routes;
  if (!old_best)
    return 0;

  for (k = &n->routes; e = *k; k = &e->next)
    if (rta_next_hop_outdated(e->attrs))
      {
	new = rt_next_hop_update_rte(tab, e);
	*k = new;

	rte_announce_i(tab, RA_ANY, n, new, e);
	rte_trace_in(D_ROUTES, new->sender, new, "updated");

	/* Call a pre-comparison hook */
	/* Not really an efficient way to compute this */
	if (e->attrs->proto->rte_recalculate)
	  e->attrs->proto->rte_recalculate(tab, n, new, e, NULL);

	if (e != old_best)
	  rte_free_quick(e);
	else /* Freeing of the old best rte is postponed */
	  free_old_best = 1;

	e = new;
	count++;
      }

  if (!count)
    return 0;

  /* Find the new best route */
  new_best = NULL;
  for (k = &n->routes; e = *k; k = &e->next)
    {
      if (!new_best || rte_better(e, *new_best))
	new_best = k;
    }

  /* Relink the new best route to the first position */
  new = *new_best;
  if (new != n->routes)
    {
      *new_best = new->next;
      new->next = n->routes;
      n->routes = new;
    }

  /* Announce the new best route */
  if (new != old_best)
    {
      rte_announce_i(tab, RA_OPTIMAL, n, new, old_best);
      rte_trace_in(D_ROUTES, new->sender, new, "updated [best]");
    }

   if (free_old_best)
    rte_free_quick(old_best);

  return count;
}

static void
rt_next_hop_update(rtable *tab)
{
  struct fib_iterator *fit = &tab->nhu_fit;
  int max_feed = 32;

  if (tab->nhu_state == 0)
    return;

  if (tab->nhu_state == 1)
    {
      FIB_ITERATE_INIT(fit, &tab->fib);
      tab->nhu_state = 2;
    }

  FIB_ITERATE_START(&tab->fib, fit, fn)
    {
      if (max_feed <= 0)
	{
	  FIB_ITERATE_PUT(fit, fn);
	  ev_schedule(tab->rt_event);
	  return;
	}
      max_feed -= rt_next_hop_update_net(tab, (net *) fn);
    }
  FIB_ITERATE_END(fn);

  /* state change 2->0, 3->1 */
  tab->nhu_state &= 1;

  if (tab->nhu_state > 0)
    ev_schedule(tab->rt_event);
}


struct rtable_config *
rt_new_table(struct symbol *s)
{
  struct rtable_config *c = cfg_allocz(sizeof(struct rtable_config));

  cf_define_symbol(s, SYM_TABLE, c);
  c->name = s->name;
  add_tail(&new_config->tables, &c->n);
  c->gc_max_ops = 1000;
  c->gc_min_time = 5;
  return c;
}

/**
 * rt_lock_table - lock a routing table
 * @r: routing table to be locked
 *
 * Lock a routing table, because it's in use by a protocol,
 * preventing it from being freed when it gets undefined in a new
 * configuration.
 */
void
rt_lock_table(rtable *r)
{
  r->use_count++;
}

/**
 * rt_unlock_table - unlock a routing table
 * @r: routing table to be unlocked
 *
 * Unlock a routing table formerly locked by rt_lock_table(),
 * that is decrease its use count and delete it if it's scheduled
 * for deletion by configuration changes.
 */
void
rt_unlock_table(rtable *r)
{
  if (!--r->use_count && r->deleted)
    {
      struct config *conf = r->deleted;
      DBG("Deleting routing table %s\n", r->name);
      if (r->hostcache)
	rt_free_hostcache(r);
      rem_node(&r->n);
      fib_free(&r->fib);
      rfree(r->rt_event);
      mb_free(r);
      config_del_obstacle(conf);
    }
}

/**
 * rt_commit - commit new routing table configuration
 * @new: new configuration
 * @old: original configuration or %NULL if it's boot time config
 *
 * Scan differences between @old and @new configuration and modify
 * the routing tables according to these changes. If @new defines a
 * previously unknown table, create it, if it omits a table existing
 * in @old, schedule it for deletion (it gets deleted when all protocols
 * disconnect from it by calling rt_unlock_table()), if it exists
 * in both configurations, leave it unchanged.
 */
void
rt_commit(struct config *new, struct config *old)
{
  struct rtable_config *o, *r;

  DBG("rt_commit:\n");
  if (old)
    {
      WALK_LIST(o, old->tables)
	{
	  rtable *ot = o->table;
	  if (!ot->deleted)
	    {
	      struct symbol *sym = cf_find_symbol(o->name);
	      if (sym && sym->class == SYM_TABLE && !new->shutdown)
		{
		  DBG("\t%s: same\n", o->name);
		  r = sym->def;
		  r->table = ot;
		  ot->name = r->name;
		  ot->config = r;
		}
	      else
		{
		  DBG("\t%s: deleted\n", o->name);
		  ot->deleted = old;
		  config_add_obstacle(old);
		  rt_lock_table(ot);
		  rt_unlock_table(ot);
		}
	    }
	}
    }

  WALK_LIST(r, new->tables)
    if (!r->table)
      {
	rtable *t = mb_alloc(rt_table_pool, sizeof(struct rtable));
	DBG("\t%s: created\n", r->name);
	rt_setup(rt_table_pool, t, r->name, r);
	add_tail(&routing_tables, &t->n);
	r->table = t;
      }
  DBG("\tdone\n");
}

static inline void
do_feed_baby(struct proto *p, int type, struct announce_hook *h, net *n, rte *e)
{
  struct proto *q = e->attrs->proto;
  ea_list *tmpa;

  rte_update_lock();
  tmpa = q->make_tmp_attrs ? q->make_tmp_attrs(e, rte_update_pool) : NULL;
  do_rte_announce(h, type, n, e, p->refeeding ? e : NULL, tmpa, p->refeeding);
  rte_update_unlock();
}

/**
 * rt_feed_baby - advertise routes to a new protocol
 * @p: protocol to be fed
 *
 * This function performs one pass of advertisement of routes to a newly
 * initialized protocol. It's called by the protocol code as long as it
 * has something to do. (We avoid transferring all the routes in single
 * pass in order not to monopolize CPU time.)
 */
int
rt_feed_baby(struct proto *p)
{
  struct announce_hook *h;
  struct fib_iterator *fit;
  int max_feed = 256;

  if (!p->feed_ahook)			/* Need to initialize first */
    {
      if (!p->ahooks)
	return 1;
      DBG("Announcing routes to new protocol %s\n", p->name);
      p->feed_ahook = p->ahooks;
      fit = p->feed_iterator = mb_alloc(p->pool, sizeof(struct fib_iterator));
      goto next_hook;
    }
  fit = p->feed_iterator;

again:
  h = p->feed_ahook;
  FIB_ITERATE_START(&h->table->fib, fit, fn)
    {
      net *n = (net *) fn;
      rte *e = n->routes;
      if (max_feed <= 0)
	{
	  FIB_ITERATE_PUT(fit, fn);
	  return 0;
	}

      if (p->accept_ra_types == RA_OPTIMAL)
	if (e)
	  {
	    if (p->core_state != FS_FEEDING)
	      return 1;  /* In the meantime, the protocol fell down. */
	    do_feed_baby(p, RA_OPTIMAL, h, n, e);
	    max_feed--;
	  }

      if (p->accept_ra_types == RA_ANY)
	for(e = n->routes; e != NULL; e = e->next)
	  {
	    if (p->core_state != FS_FEEDING)
	      return 1;  /* In the meantime, the protocol fell down. */
	    do_feed_baby(p, RA_ANY, h, n, e);
	    max_feed--;
	  }
    }
  FIB_ITERATE_END(fn);
  p->feed_ahook = h->next;
  if (!p->feed_ahook)
    {
      mb_free(p->feed_iterator);
      p->feed_iterator = NULL;
      return 1;
    }

next_hook:
  h = p->feed_ahook;
  FIB_ITERATE_INIT(fit, &h->table->fib);
  goto again;
}

/**
 * rt_feed_baby_abort - abort protocol feeding
 * @p: protocol
 *
 * This function is called by the protocol code when the protocol
 * stops or ceases to exist before the last iteration of rt_feed_baby()
 * has finished.
 */
void
rt_feed_baby_abort(struct proto *p)
{
  if (p->feed_ahook)
    {
      /* Unlink the iterator and exit */
      fit_get(&p->feed_ahook->table->fib, p->feed_iterator);
      p->feed_ahook = NULL;
    }
}


static inline unsigned
ptr_hash(void *ptr)
{
  uintptr_t p = (uintptr_t) ptr;
  return p ^ (p << 8) ^ (p >> 16);
}

static inline unsigned
hc_hash(ip_addr a, rtable *dep)
{
  return (ipa_hash(a) ^ ptr_hash(dep)) & 0xffff;
}

static inline void
hc_insert(struct hostcache *hc, struct hostentry *he)
{
  unsigned int k = he->hash_key >> hc->hash_shift;
  he->next = hc->hash_table[k];
  hc->hash_table[k] = he;
}

static inline void
hc_remove(struct hostcache *hc, struct hostentry *he)
{
  struct hostentry **hep;
  unsigned int k = he->hash_key >> hc->hash_shift;

  for (hep = &hc->hash_table[k]; *hep != he; hep = &(*hep)->next);
  *hep = he->next;
}

#define HC_DEF_ORDER 10
#define HC_HI_MARK *4
#define HC_HI_STEP 2
#define HC_HI_ORDER 16			/* Must be at most 16 */
#define HC_LO_MARK /5
#define HC_LO_STEP 2
#define HC_LO_ORDER 10

static void
hc_alloc_table(struct hostcache *hc, unsigned order)
{
  unsigned hsize = 1 << order;
  hc->hash_order = order;
  hc->hash_shift = 16 - order;
  hc->hash_max = (order >= HC_HI_ORDER) ? ~0 : (hsize HC_HI_MARK);
  hc->hash_min = (order <= HC_LO_ORDER) ?  0 : (hsize HC_LO_MARK);

  hc->hash_table = mb_allocz(rt_table_pool, hsize * sizeof(struct hostentry *));
}

static void
hc_resize(struct hostcache *hc, unsigned new_order)
{
  unsigned old_size = 1 << hc->hash_order;
  struct hostentry **old_table = hc->hash_table;
  struct hostentry *he, *hen;
  int i;

  hc_alloc_table(hc, new_order);
  for (i = 0; i < old_size; i++)
    for (he = old_table[i]; he != NULL; he=hen)
      {
	hen = he->next;
	hc_insert(hc, he);
      }
  mb_free(old_table);
}

static struct hostentry *
hc_new_hostentry(struct hostcache *hc, ip_addr a, ip_addr ll, rtable *dep, unsigned k)
{
  struct hostentry *he = sl_alloc(hc->slab);

  he->addr = a;
  he->link = ll;
  he->tab = dep;
  he->hash_key = k;
  he->uc = 0;
  he->src = NULL;

  add_tail(&hc->hostentries, &he->ln);
  hc_insert(hc, he);

  hc->hash_items++;
  if (hc->hash_items > hc->hash_max)
    hc_resize(hc, hc->hash_order + HC_HI_STEP);

  return he;
}

static void
hc_delete_hostentry(struct hostcache *hc, struct hostentry *he)
{
  rta_free(he->src);

  rem_node(&he->ln);
  hc_remove(hc, he);
  sl_free(hc->slab, he);

  hc->hash_items--;
  if (hc->hash_items < hc->hash_min)
    hc_resize(hc, hc->hash_order - HC_LO_STEP);
}

static void
rt_init_hostcache(rtable *tab)
{
  struct hostcache *hc = mb_allocz(rt_table_pool, sizeof(struct hostcache));
  init_list(&hc->hostentries);

  hc->hash_items = 0;
  hc_alloc_table(hc, HC_DEF_ORDER);
  hc->slab = sl_new(rt_table_pool, sizeof(struct hostentry));

  hc->lp = lp_new(rt_table_pool, 1008);
  hc->trie = f_new_trie(hc->lp);

  tab->hostcache = hc;
}

static void
rt_free_hostcache(rtable *tab)
{
  struct hostcache *hc = tab->hostcache;

  node *n;
  WALK_LIST(n, hc->hostentries)
    {
      struct hostentry *he = SKIP_BACK(struct hostentry, ln, n);
      rta_free(he->src);

      if (he->uc)
	log(L_ERR "Hostcache is not empty in table %s", tab->name);
    }

  rfree(hc->slab);
  rfree(hc->lp);
  mb_free(hc->hash_table);
  mb_free(hc);
}

static void
rt_notify_hostcache(rtable *tab, net *net)
{
  struct hostcache *hc = tab->hostcache;

  if (tab->hcu_scheduled)
    return;

  if (trie_match_prefix(hc->trie, net->n.prefix, net->n.pxlen))
    rt_schedule_hcu(tab);
}

static int
if_local_addr(ip_addr a, struct iface *i)
{
  struct ifa *b;

  WALK_LIST(b, i->addrs)
    if (ipa_equal(a, b->ip))
      return 1;

  return 0;
}

static u32 
rt_get_igp_metric(rte *rt)
{
  eattr *ea = ea_find(rt->attrs->eattrs, EA_GEN_IGP_METRIC);

  if (ea)
    return ea->u.data;

  rta *a = rt->attrs;

#ifdef CONFIG_OSPF
  if ((a->source == RTS_OSPF) ||
      (a->source == RTS_OSPF_IA) ||
      (a->source == RTS_OSPF_EXT1))
    return rt->u.ospf.metric1;
#endif

#ifdef CONFIG_RIP
  if (a->source == RTS_RIP)
    return rt->u.rip.metric;
#endif

  /* Device routes */
  if ((a->dest != RTD_ROUTER) && (a->dest != RTD_MULTIPATH))
    return 0;

  return IGP_METRIC_UNKNOWN;
}

static int
rt_update_hostentry(rtable *tab, struct hostentry *he)
{
  rta *old_src = he->src;
  int pxlen = 0;

  /* Reset the hostentry */ 
  he->src = NULL;
  he->gw = IPA_NONE;
  he->dest = RTD_UNREACHABLE;
  he->igp_metric = 0;

  net *n = net_route(tab, he->addr, MAX_PREFIX_LENGTH);
  if (n)
    {
      rta *a = n->routes->attrs;
      pxlen = n->n.pxlen;

      if (a->hostentry)
	{
	  /* Recursive route should not depend on another recursive route */
	  log(L_WARN "Next hop address %I resolvable through recursive route for %I/%d",
	      he->addr, n->n.prefix, pxlen);
	  goto done;
	}

      if (a->dest == RTD_DEVICE)
	{
	  if (if_local_addr(he->addr, a->iface))
	    {
	      /* The host address is a local address, this is not valid */
	      log(L_WARN "Next hop address %I is a local address of iface %s",
		  he->addr, a->iface->name);
	      goto done;
      	    }

	  /* The host is directly reachable, use link as a gateway */
	  he->gw = he->link;
	  he->dest = RTD_ROUTER;
	}
      else
	{
	  /* The host is reachable through some route entry */
	  he->gw = a->gw;
	  he->dest = a->dest;
	}

      he->src = rta_clone(a);
      he->igp_metric = rt_get_igp_metric(n->routes);
    }

 done:
  /* Add a prefix range to the trie */
  trie_add_prefix(tab->hostcache->trie, he->addr, MAX_PREFIX_LENGTH, pxlen, MAX_PREFIX_LENGTH);

  rta_free(old_src);
  return old_src != he->src;
}

static void
rt_update_hostcache(rtable *tab)
{
  struct hostcache *hc = tab->hostcache;
  struct hostentry *he;
  node *n, *x;

  /* Reset the trie */
  lp_flush(hc->lp);
  hc->trie = f_new_trie(hc->lp);

  WALK_LIST_DELSAFE(n, x, hc->hostentries)
    {
      he = SKIP_BACK(struct hostentry, ln, n);
      if (!he->uc)
	{
	  hc_delete_hostentry(hc, he);
	  continue;
	}

      if (rt_update_hostentry(tab, he))
	rt_schedule_nhu(he->tab);
    }

  tab->hcu_scheduled = 0;
}

static struct hostentry *
rt_find_hostentry(rtable *tab, ip_addr a, ip_addr ll, rtable *dep)
{
  struct hostentry *he;

  if (!tab->hostcache)
    rt_init_hostcache(tab);

  unsigned int k = hc_hash(a, dep);
  struct hostcache *hc = tab->hostcache;
  for (he = hc->hash_table[k >> hc->hash_shift]; he != NULL; he = he->next)
    if (ipa_equal(he->addr, a) && (he->tab == dep))
      return he;

  he = hc_new_hostentry(hc, a, ll, dep, k);
  rt_update_hostentry(tab, he);
  return he;
}

void
rta_set_recursive_next_hop(rtable *dep, rta *a, rtable *tab, ip_addr *gw, ip_addr *ll)
{
  rta_apply_hostentry(a, rt_find_hostentry(tab, *gw, *ll, dep));
}

/*
 *  CLI commands
 */

static void
rt_format_via(rte *e, byte *via)
{
  rta *a = e->attrs;

  switch (a->dest)
    {
    case RTD_ROUTER:	bsprintf(via, "via %I on %s", a->gw, a->iface->name); break;
    case RTD_DEVICE:	bsprintf(via, "dev %s", a->iface->name); break;
    case RTD_BLACKHOLE:	bsprintf(via, "blackhole"); break;
    case RTD_UNREACHABLE:	bsprintf(via, "unreachable"); break;
    case RTD_PROHIBIT:	bsprintf(via, "prohibited"); break;
    case RTD_MULTIPATH:	bsprintf(via, "multipath"); break;
    default:		bsprintf(via, "???");
    }
}

static void
rt_show_rte(struct cli *c, byte *ia, rte *e, struct rt_show_data *d, ea_list *tmpa)
{
  byte via[STD_ADDRESS_P_LENGTH+32], from[STD_ADDRESS_P_LENGTH+8];
  byte tm[TM_DATETIME_BUFFER_SIZE], info[256];
  rta *a = e->attrs;
  int primary = (e->net->routes == e);
  int sync_error = (e->net->n.flags & KRF_SYNC_ERROR);
  struct mpnh *nh;

  rt_format_via(e, via);
  tm_format_datetime(tm, &config->tf_route, e->lastmod);
  if (ipa_nonzero(a->from) && !ipa_equal(a->from, a->gw))
    bsprintf(from, " from %I", a->from);
  else
    from[0] = 0;
  if (a->proto->proto->get_route_info || d->verbose)
    {
      /* Need to normalize the extended attributes */
      ea_list *t = tmpa;
      t = ea_append(t, a->eattrs);
      tmpa = alloca(ea_scan(t));
      ea_merge(t, tmpa);
      ea_sort(tmpa);
    }
  if (a->proto->proto->get_route_info)
    a->proto->proto->get_route_info(e, info, tmpa);
  else
    bsprintf(info, " (%d)", e->pref);
  cli_printf(c, -1007, "%-18s %s [%s %s%s]%s%s", ia, via, a->proto->name,
	     tm, from, primary ? (sync_error ? " !" : " *") : "", info);
  for (nh = a->nexthops; nh; nh = nh->next)
    cli_printf(c, -1007, "\tvia %I on %s weight %d", nh->gw, nh->iface->name, nh->weight + 1);
  if (d->verbose)
    rta_show(c, a, tmpa);
}

static void
rt_show_net(struct cli *c, net *n, struct rt_show_data *d)
{
  rte *e, *ee;
  byte ia[STD_ADDRESS_P_LENGTH+8];
  int ok;

  bsprintf(ia, "%I/%d", n->n.prefix, n->n.pxlen);
  if (n->routes)
    d->net_counter++;
  for(e=n->routes; e; e=e->next)
    {
      struct ea_list *tmpa, *old_tmpa;
      struct proto *p0 = e->attrs->proto;
      struct proto *p1 = d->export_protocol;
      struct proto *p2 = d->show_protocol;
      d->rt_counter++;
      ee = e;
      rte_update_lock();		/* We use the update buffer for filtering */
      old_tmpa = tmpa = p0->make_tmp_attrs ? p0->make_tmp_attrs(e, rte_update_pool) : NULL;
      ok = (d->filter == FILTER_ACCEPT || f_run(d->filter, &e, &tmpa, rte_update_pool, FF_FORCE_TMPATTR) <= F_ACCEPT);
      if (p2 && p2 != p0) ok = 0;
      if (ok && d->export_mode)
	{
	  int ic;
	  if ((ic = p1->import_control ? p1->import_control(p1, &e, &tmpa, rte_update_pool) : 0) < 0)
	    ok = 0;
	  else if (!ic && d->export_mode > 1)
	    {
	      /* FIXME - this shows what should be exported according
		 to current filters, but not what was really exported.
		 'configure soft' command may change the export filter
		 and do not update routes */

	      if ((p1->out_filter == FILTER_REJECT) ||
		  (p1->out_filter && f_run(p1->out_filter, &e, &tmpa, rte_update_pool, FF_FORCE_TMPATTR) > F_ACCEPT))
		ok = 0;
	    }
	}
      if (ok)
	{
	  d->show_counter++;
	  if (d->stats < 2)
	    rt_show_rte(c, ia, e, d, tmpa);
	  ia[0] = 0;
	}
      if (e != ee)
      {
	rte_free(e);
	e = ee;
      }
      rte_update_unlock();
      if (d->primary_only)
	break;
    }
}

static void
rt_show_cont(struct cli *c)
{
  struct rt_show_data *d = c->rover;
#ifdef DEBUGGING
  unsigned max = 4;
#else
  unsigned max = 64;
#endif
  struct fib *fib = &d->table->fib;
  struct fib_iterator *it = &d->fit;

  FIB_ITERATE_START(fib, it, f)
    {
      net *n = (net *) f;
      if (d->running_on_config && d->running_on_config != config)
	{
	  cli_printf(c, 8004, "Stopped due to reconfiguration");
	  goto done;
	}
      if (d->export_protocol &&
	  d->export_protocol->core_state != FS_HAPPY &&
	  d->export_protocol->core_state != FS_FEEDING)
	{
	  cli_printf(c, 8005, "Protocol is down");
	  goto done;
	}
      if (!max--)
	{
	  FIB_ITERATE_PUT(it, f);
	  return;
	}
      rt_show_net(c, n, d);
    }
  FIB_ITERATE_END(f);
  if (d->stats)
    cli_printf(c, 14, "%d of %d routes for %d networks", d->show_counter, d->rt_counter, d->net_counter);
  else
    cli_printf(c, 0, "");
done:
  c->cont = c->cleanup = NULL;
}

static void
rt_show_cleanup(struct cli *c)
{
  struct rt_show_data *d = c->rover;

  /* Unlink the iterator */
  fit_get(&d->table->fib, &d->fit);
}

void
rt_show(struct rt_show_data *d)
{
  net *n;

  if (d->pxlen == 256)
    {
      FIB_ITERATE_INIT(&d->fit, &d->table->fib);
      this_cli->cont = rt_show_cont;
      this_cli->cleanup = rt_show_cleanup;
      this_cli->rover = d;
    }
  else
    {
      if (d->show_for)
	n = net_route(d->table, d->prefix, d->pxlen);
      else
	n = net_find(d->table, d->prefix, d->pxlen);
      if (n)
	{
	  rt_show_net(this_cli, n, d);
	  cli_msg(0, "");
	}
      else
	cli_msg(8001, "Network not in table");
    }
}

/*
 *  Documentation for functions declared inline in route.h
 */
#if 0

/**
 * net_find - find a network entry
 * @tab: a routing table
 * @addr: address of the network
 * @len: length of the network prefix
 *
 * net_find() looks up the given network in routing table @tab and
 * returns a pointer to its &net entry or %NULL if no such network
 * exists.
 */
static inline net *net_find(rtable *tab, ip_addr addr, unsigned len)
{ DUMMY; }

/**
 * net_get - obtain a network entry
 * @tab: a routing table
 * @addr: address of the network
 * @len: length of the network prefix
 *
 * net_get() looks up the given network in routing table @tab and
 * returns a pointer to its &net entry. If no such entry exists, it's
 * created.
 */
static inline net *net_get(rtable *tab, ip_addr addr, unsigned len)
{ DUMMY; }

/**
 * rte_cow - copy a route for writing
 * @r: a route entry to be copied
 *
 * rte_cow() takes a &rte and prepares it for modification. The exact action
 * taken depends on the flags of the &rte -- if it's a temporary entry, it's
 * just returned unchanged, else a new temporary entry with the same contents
 * is created.
 *
 * The primary use of this function is inside the filter machinery -- when
 * a filter wants to modify &rte contents (to change the preference or to
 * attach another set of attributes), it must ensure that the &rte is not
 * shared with anyone else (and especially that it isn't stored in any routing
 * table).
 *
 * Result: a pointer to the new writable &rte.
 */
static inline rte * rte_cow(rte *r)
{ DUMMY; }

#endif
