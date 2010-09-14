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

#include "static.h"

static void
static_install(struct proto *p, struct static_route *r, struct iface *ifa)
{
  net *n;
  rta a, *aa;
  rte *e;

  if (r->installed)
    return;

  DBG("Installing static route %I/%d, rtd=%d\n", r->net, r->masklen, r->dest);
  bzero(&a, sizeof(a));
  a.proto = p;
  a.source = (r->dest == RTD_DEVICE) ? RTS_STATIC_DEVICE : RTS_STATIC;
  a.scope = SCOPE_UNIVERSE;
  a.cast = RTC_UNICAST;
  a.dest = r->dest;
  a.gw = r->via;
  a.iface = ifa;
  aa = rta_lookup(&a);

  n = net_get(p->table, r->net, r->masklen);
  e = rte_get_temp(aa);
  e->net = n;
  e->pflags = 0;
  rte_update(p->table, n, p, p, e);
  r->installed = 1;
}

static void
static_remove(struct proto *p, struct static_route *r)
{
  net *n;

  if (!r->installed)
    return;

  DBG("Removing static route %I/%d\n", r->net, r->masklen);
  n = net_find(p->table, r->net, r->masklen);
  if (n)
    rte_update(p->table, n, p, p, NULL);
  r->installed = 0;
}

static void
static_add(struct proto *p, struct static_route *r)
{
  DBG("static_add(%I/%d,%d)\n", r->net, r->masklen, r->dest);
  switch (r->dest)
    {
    case RTD_ROUTER:
      {
	struct neighbor *n = neigh_find(p, &r->via, NEF_STICKY);
	if (n)
	  {
	    r->chain = n->data;
	    n->data = r;
	    r->neigh = n;
	    if (n->iface)
	      static_install(p, r, n->iface);
	  }
	else
	  log(L_ERR "Static route destination %I is invalid. Ignoring.", r->via);
	break;
      }
    case RTD_DEVICE:
      break;
    default:
      static_install(p, r, NULL);
    }
}

static int
static_start(struct proto *p)
{
  struct static_config *c = (void *) p->cf;
  struct static_route *r;

  DBG("Static: take off!\n");
  WALK_LIST(r, c->other_routes)
    static_add(p, r);
  return PS_UP;
}

static int
static_shutdown(struct proto *p)
{
  struct static_config *c = (void *) p->cf;
  struct static_route *r;

  /* Just reset the flag, the routes will be flushed by the nest */
  WALK_LIST(r, c->iface_routes)
    r->installed = 0;
  WALK_LIST(r, c->other_routes)
    r->installed = 0;

  return PS_DOWN;
}

static void
static_neigh_notify(struct neighbor *n)
{
  struct proto *p = n->proto;
  struct static_route *r;

  DBG("Static: neighbor notify for %I: iface %p\n", n->addr, n->iface);
  for(r=n->data; r; r=r->chain)
    if (n->iface)
      static_install(p, r, n->iface);
    else
      static_remove(p, r);
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
  c->c.preference = DEF_PREF_STATIC;
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
  return (x->dest == y->dest)
    && (x->dest != RTD_ROUTER || ipa_equal(x->via, y->via))
    && (x->dest != RTD_DEVICE || !strcmp(x->if_name, y->if_name));
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
      {
	t->installed = static_same_dest(r, t);
	return;
      }
  WALK_LIST(t, n->other_routes)
    if (static_same_net(r, t))
      {
	t->installed = static_same_dest(r, t);
	return;
      }
  static_remove(p, r);
}

static int
static_reconfigure(struct proto *p, struct proto_config *new)
{
  struct static_config *o = (void *) p->cf;
  struct static_config *n = (void *) new;
  struct static_route *r;

  /* Delete all obsolete routes and reset neighbor entries */
  WALK_LIST(r, o->iface_routes)
    static_match(p, r, n);
  WALK_LIST(r, o->other_routes)
    static_match(p, r, n);

  /* Now add all new routes, those not changed will be ignored by static_install() */
  WALK_LIST(r, n->iface_routes)
    {
      struct iface *ifa;
      if (ifa = if_find_by_name(r->if_name))
	static_install(p, r, ifa);
    }
  WALK_LIST(r, n->other_routes)
    static_add(p, r);

  return 1;
}

struct protocol proto_static = {
  name:		"Static",
  template:	"static%d",
  init:		static_init,
  dump:		static_dump,
  start:	static_start,
  shutdown:	static_shutdown,
  reconfigure:	static_reconfigure,
};

static void
static_show_rt(struct static_route *r)
{
  byte via[STD_ADDRESS_P_LENGTH + 16];

  switch (r->dest)
    {
    case RTD_ROUTER:	bsprintf(via, "via %I", r->via); break;
    case RTD_DEVICE:	bsprintf(via, "dev %s", r->if_name); break;
    case RTD_BLACKHOLE:	bsprintf(via, "blackhole"); break;
    case RTD_UNREACHABLE:	bsprintf(via, "unreachable"); break;
    case RTD_PROHIBIT:	bsprintf(via, "prohibited"); break;
    default:		bsprintf(via, "???");
    }
  cli_msg(-1009, "%I/%d %s%s", r->net, r->masklen, via, r->installed ? "" : " (dormant)");
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
