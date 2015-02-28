/*
 *	BIRD -- Static Route Generator
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Static
 *
 * The Static protocol is implemented in a straightforward way. It keeps
 * two lists of static routes: one containing interface routes and one
 * holding the remaining ones. Interface routes are inserted and removed according
 * to interface events received from the core via the if_notify() hook. Routes
 * pointing to a neighboring router use a sticky node in the neighbor cache
 * to be notified about gaining or losing the neighbor. Special
 * routes like black holes or rejects are inserted all the time.
 *
 * Multipath routes are tricky. Because these routes depends on
 * several neighbors we need to integrate that to the neighbor
 * notification handling, we use dummy static_route nodes, one for
 * each nexthop. Therefore, a multipath route consists of a master
 * static_route node (of dest RTD_MULTIPATH), which specifies prefix
 * and is used in most circumstances, and a list of dummy static_route
 * nodes (of dest RTD_NONE), which stores info about nexthops and are
 * connected to neighbor entries and neighbor notifications. Dummy
 * nodes are chained using mp_next, they aren't in other_routes list,
 * and abuse some fields (masklen, if_name) for other purposes.
 *
 * The only other thing worth mentioning is that when asked for reconfiguration,
 * Static not only compares the two configurations, but it also calculates
 * difference between the lists of static routes and it just inserts the
 * newly added routes and removes the obsolete ones.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "nest/route.h"
#include "nest/cli.h"
#include "conf/conf.h"
#include "lib/string.h"
#include "lib/alloca.h"

#include "static.h"

static inline rtable *
p_igp_table(struct proto *p)
{
  struct static_config *cf = (void *) p->cf;
  return cf->igp_table ? cf->igp_table->table : p->table;
}


static void
static_install(struct proto *p, struct static_route *r, struct iface *ifa)
{
  net *n;
  rta a, *aa;
  rte *e;

  if (r->installed > 0)
    return;

  DBG("Installing static route %I/%d, rtd=%d\n", r->net, r->masklen, r->dest);
  bzero(&a, sizeof(a));
  a.src = p->main_source;
  a.source = (r->dest == RTD_DEVICE) ? RTS_STATIC_DEVICE : RTS_STATIC;
  a.scope = SCOPE_UNIVERSE;
  a.cast = RTC_UNICAST;
  a.dest = r->dest;
  a.gw = r->via;
  a.iface = ifa;

  if (r->dest == RTD_MULTIPATH)
    {
      struct static_route *r2;
      struct mpnh *nhs = NULL;
      struct mpnh **nhp = &nhs;

      for (r2 = r->mp_next; r2; r2 = r2->mp_next)
	if (r2->installed)
	  {
	    struct mpnh *nh = alloca(sizeof(struct mpnh));
	    nh->gw = r2->via;
	    nh->iface = r2->neigh->iface;
	    nh->weight = r2->masklen; /* really */
	    nh->next = NULL;
	    *nhp = nh;
	    nhp = &(nh->next);
	  }

      /* There is at least one nexthop */
      if (!nhs->next)
	{
	  /* Fallback to unipath route for exactly one nexthop */
	  a.dest = RTD_ROUTER;
	  a.gw = nhs->gw;
	  a.iface = nhs->iface;
	}
      else
	a.nexthops = nhs;
    }

  if (r->dest == RTDX_RECURSIVE)
    rta_set_recursive_next_hop(p->table, &a, p_igp_table(p), &r->via, &r->via);

  aa = rta_lookup(&a);
  n = net_get(p->table, r->net, r->masklen);
  e = rte_get_temp(aa);
  e->net = n;
  e->pflags = 0;
  rte_update(p, n, e);
  r->installed = 1;
}

static void
static_remove(struct proto *p, struct static_route *r)
{
  net *n;

  if (!r->installed)
    return;

  DBG("Removing static route %I/%d via %I\n", r->net, r->masklen, r->via);
  n = net_find(p->table, r->net, r->masklen);
  rte_update(p, n, NULL);
  r->installed = 0;
}

static int
static_decide(struct static_config *cf, struct static_route *r)
{
  /* r->dest != RTD_MULTIPATH, but may be RTD_NONE (part of multipath route)
     the route also have to be valid (r->neigh != NULL) */

  if (r->neigh->scope < 0)
    return 0;

  if (cf->check_link && !(r->neigh->iface->flags & IF_LINK_UP))
    return 0;

  return 1;
}


static void
static_add(struct proto *p, struct static_config *cf, struct static_route *r)
{
  DBG("static_add(%I/%d,%d)\n", r->net, r->masklen, r->dest);
  switch (r->dest)
    {
    case RTD_ROUTER:
      {
	struct neighbor *n = neigh_find2(p, &r->via, r->via_if, NEF_STICKY);
	if (n)
	  {
	    r->chain = n->data;
	    n->data = r;
	    r->neigh = n;
	    if (static_decide(cf, r))
	      static_install(p, r, n->iface);
	    else
	      static_remove(p, r);
	  }
	else
	  {
	    log(L_ERR "Static route destination %I is invalid. Ignoring.", r->via);
	    static_remove(p, r);
	  }
	break;
      }

    case RTD_DEVICE:
      break;

    case RTD_MULTIPATH:
      {
	int count = 0;
	struct static_route *r2;

	for (r2 = r->mp_next; r2; r2 = r2->mp_next)
	  {
	    struct neighbor *n = neigh_find2(p, &r2->via, r2->via_if, NEF_STICKY);
	    if (n)
	      {
		r2->chain = n->data;
		n->data = r2;
		r2->neigh = n;
		r2->installed = static_decide(cf, r2);
		count += r2->installed;
	      }
	    else
	      {
		log(L_ERR "Static route destination %I is invalid. Ignoring.", r2->via);
		r2->installed = 0;
	      }
	  }

	if (count)
	  static_install(p, r, NULL);
	else
	  static_remove(p, r);
	break;
      }

    default:
      static_install(p, r, NULL);
    }
}

static int
static_start(struct proto *p)
{
  struct static_config *cf = (void *) p->cf;
  struct static_route *r;

  DBG("Static: take off!\n");

  if (cf->igp_table)
    rt_lock_table(cf->igp_table->table);

  /* We have to go UP before routes could be installed */
  proto_notify_state(p, PS_UP);

  WALK_LIST(r, cf->other_routes)
    static_add(p, cf, r);
  return PS_UP;
}

static int
static_shutdown(struct proto *p)
{
  struct static_config *cf = (void *) p->cf;
  struct static_route *r;

  /* Just reset the flag, the routes will be flushed by the nest */
  WALK_LIST(r, cf->iface_routes)
    r->installed = 0;
  WALK_LIST(r, cf->other_routes)
    r->installed = 0;

  return PS_DOWN;
}

static void
static_cleanup(struct proto *p)
{
  struct static_config *cf = (void *) p->cf;

  if (cf->igp_table)
    rt_unlock_table(cf->igp_table->table);
}


static void
static_neigh_notify(struct neighbor *n)
{
  struct proto *p = n->proto;
  struct static_route *r;

  DBG("Static: neighbor notify for %I: iface %p\n", n->addr, n->iface);
  for(r=n->data; r; r=r->chain)
    switch (r->dest)
      {
      case RTD_ROUTER:
	if (static_decide((struct static_config *) p->cf, r))
	  static_install(p, r, n->iface);
	else
	  static_remove(p, r);
	break;

      case RTD_NONE: /* a part of multipath route */
	{
	  int decision = static_decide((struct static_config *) p->cf, r);
	  if (decision == r->installed)
	    break; /* no change */
	  r->installed = decision;

	  struct static_route *r1, *r2;
	  int count = 0;
	  r1 = (void *) r->if_name; /* really */
	  for (r2 = r1->mp_next; r2; r2 = r2->mp_next)
	    count += r2->installed;

	  if (count)
	    {
	      /* Set of nexthops changed - force reinstall */
	      r1->installed = 0;
	      static_install(p, r1, NULL);
	    }
	  else
	    static_remove(p, r1);

	  break;
	}
      }
}

static void
static_dump_rt(struct static_route *r)
{
  debug("%-1I/%2d: ", r->net, r->masklen);
  switch (r->dest)
    {
    case RTD_ROUTER:
      debug("via %I\n", r->via);
      break;
    case RTD_DEVICE:
      debug("dev %s\n", r->if_name);
      break;
    default:
      debug("rtd %d\n", r->dest);
      break;
    }
}

static void
static_dump(struct proto *p)
{
  struct static_config *c = (void *) p->cf;
  struct static_route *r;

  debug("Independent static routes:\n");
  WALK_LIST(r, c->other_routes)
    static_dump_rt(r);
  debug("Device static routes:\n");
  WALK_LIST(r, c->iface_routes)
    static_dump_rt(r);
}

static void
static_if_notify(struct proto *p, unsigned flags, struct iface *i)
{
  struct static_route *r;
  struct static_config *c = (void *) p->cf;

  if (flags & IF_CHANGE_UP)
    {
      WALK_LIST(r, c->iface_routes)
	if (!strcmp(r->if_name, i->name))
	  static_install(p, r, i);
    }
  else if (flags & IF_CHANGE_DOWN)
    {
      WALK_LIST(r, c->iface_routes)
	if (!strcmp(r->if_name, i->name))
	  static_remove(p, r);
    }
}

void
static_init_config(struct static_config *c)
{
  init_list(&c->iface_routes);
  init_list(&c->other_routes);
}

static struct proto *
static_init(struct proto_config *c)
{
  struct proto *p = proto_new(c, sizeof(struct proto));

  p->neigh_notify = static_neigh_notify;
  p->if_notify = static_if_notify;

  return p;
}

static inline int
static_same_net(struct static_route *x, struct static_route *y)
{
  return ipa_equal(x->net, y->net) && (x->masklen == y->masklen);
}

static inline int
static_same_dest(struct static_route *x, struct static_route *y)
{
  if (x->dest != y->dest)
    return 0;

  switch (x->dest)
    {
    case RTD_ROUTER:
      return ipa_equal(x->via, y->via) && (x->via_if == y->via_if);

    case RTD_DEVICE:
      return !strcmp(x->if_name, y->if_name);

    case RTD_MULTIPATH:
      for (x = x->mp_next, y = y->mp_next;
	   x && y;
	   x = x->mp_next, y = y->mp_next)
	if (!ipa_equal(x->via, y->via) || (x->via_if != y->via_if))
	  return 0;
      return !x && !y;

    case RTDX_RECURSIVE:
      return ipa_equal(x->via, y->via);

    default:
      return 1;
    }
}

static void
static_match(struct proto *p, struct static_route *r, struct static_config *n)
{
  struct static_route *t;

  /*
   * For given old route *r we find whether a route to the same
   * network is also in the new route list. In that case, we keep the
   * route and possibly update the route later if destination changed.
   * Otherwise, we remove the route.
   */

  if (r->neigh)
    r->neigh->data = NULL;

  WALK_LIST(t, n->iface_routes)
    if (static_same_net(r, t))
      goto found;

  WALK_LIST(t, n->other_routes)
    if (static_same_net(r, t))
      goto found;

  static_remove(p, r);
  return;

 found:
  /* If destination is different, force reinstall */
  if ((r->installed > 0) && !static_same_dest(r, t))
    t->installed = -1;
  else
    t->installed = r->installed;
}

static inline rtable *
cf_igp_table(struct static_config *cf)
{
  return cf->igp_table ? cf->igp_table->table : NULL;
}

static int
static_reconfigure(struct proto *p, struct proto_config *new)
{
  struct static_config *o = (void *) p->cf;
  struct static_config *n = (void *) new;
  struct static_route *r;

  if (cf_igp_table(o) != cf_igp_table(n))
    return 0;

  /* Delete all obsolete routes and reset neighbor entries */
  WALK_LIST(r, o->iface_routes)
    static_match(p, r, n);
  WALK_LIST(r, o->other_routes)
    static_match(p, r, n);

  /* Now add all new routes, those not changed will be ignored by static_install() */
  WALK_LIST(r, n->iface_routes)
    {
      struct iface *ifa;
      if ((ifa = if_find_by_name(r->if_name)) && (ifa->flags & IF_UP))
	static_install(p, r, ifa);
    }
  WALK_LIST(r, n->other_routes)
    static_add(p, n, r);

  return 1;
}

static void
static_copy_routes(list *dlst, list *slst)
{
  struct static_route *dr, *sr;

  init_list(dlst);
  WALK_LIST(sr, *slst)
    {
      /* copy one route */
      dr = cfg_alloc(sizeof(struct static_route));
      memcpy(dr, sr, sizeof(struct static_route));

      /* This fn is supposed to be called on fresh src routes, which have 'live'
	 fields (like .chain, .neigh or .installed) zero, so no need to zero them */

      /* We need to copy multipath chain, because there are backptrs in 'if_name' */
      if (dr->dest == RTD_MULTIPATH)
	{
	  struct static_route *md, *ms, **mp_last;

	  mp_last = &(dr->mp_next);
	  for (ms = sr->mp_next; ms; ms = ms->mp_next)
	    {
	      md = cfg_alloc(sizeof(struct static_route));
	      memcpy(md, ms, sizeof(struct static_route));
	      md->if_name = (void *) dr; /* really */

	      *mp_last = md;
	      mp_last = &(md->mp_next);
	    }
	  *mp_last = NULL;
	}

      add_tail(dlst, (node *) dr);
    }
}

static void
static_copy_config(struct proto_config *dest, struct proto_config *src)
{
  struct static_config *d = (struct static_config *) dest;
  struct static_config *s = (struct static_config *) src;

  /* Shallow copy of everything */
  proto_copy_rest(dest, src, sizeof(struct static_config));

  /* Copy route lists */
  static_copy_routes(&d->iface_routes, &s->iface_routes);
  static_copy_routes(&d->other_routes, &s->other_routes);
}


struct protocol proto_static = {
  name:		"Static",
  template:	"static%d",
  preference:	DEF_PREF_STATIC,
  init:		static_init,
  dump:		static_dump,
  start:	static_start,
  shutdown:	static_shutdown,
  cleanup:	static_cleanup,
  reconfigure:	static_reconfigure,
  copy_config:	static_copy_config
};

static void
static_show_rt(struct static_route *r)
{
  byte via[STD_ADDRESS_P_LENGTH + 16];

  switch (r->dest)
    {
    case RTD_ROUTER:	bsprintf(via, "via %I%J", r->via, r->via_if); break;
    case RTD_DEVICE:	bsprintf(via, "dev %s", r->if_name); break;
    case RTD_BLACKHOLE:	bsprintf(via, "blackhole"); break;
    case RTD_UNREACHABLE: bsprintf(via, "unreachable"); break;
    case RTD_PROHIBIT:	bsprintf(via, "prohibited"); break;
    case RTD_MULTIPATH:	bsprintf(via, "multipath"); break;
    case RTDX_RECURSIVE: bsprintf(via, "recursive %I", r->via); break;
    default:		bsprintf(via, "???");
    }
  cli_msg(-1009, "%I/%d %s%s", r->net, r->masklen, via, r->installed ? "" : " (dormant)");

  struct static_route *r2;
  if (r->dest == RTD_MULTIPATH)
    for (r2 = r->mp_next; r2; r2 = r2->mp_next)
      cli_msg(-1009, "\tvia %I%J weight %d%s", r2->via, r2->via_if, r2->masklen + 1, /* really */
	      r2->installed ? "" : " (dormant)");
}

void
static_show(struct proto *P)
{
  struct static_config *c = (void *) P->cf;
  struct static_route *r;

  WALK_LIST(r, c->other_routes)
    static_show_rt(r);
  WALK_LIST(r, c->iface_routes)
    static_show_rt(r);
  cli_msg(0, "");
}
