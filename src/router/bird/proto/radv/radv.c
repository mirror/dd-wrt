/*
 *	BIRD -- Router Advertisement
 *
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */


#include <stdlib.h>
#include "radv.h"

/**
 * DOC: Router Advertisements
 *
 * The RAdv protocol is implemented in two files: |radv.c| containing the
 * interface with BIRD core and the protocol logic and |packets.c| handling low
 * level protocol stuff (RX, TX and packet formats). The protocol does not
 * export any routes.
 *
 * The RAdv is structured in the usual way - for each handled interface there is
 * a structure &radv_iface that contains a state related to that interface
 * together with its resources (a socket, a timer). There is also a prepared RA
 * stored in a TX buffer of the socket associated with an iface. These iface
 * structures are created and removed according to iface events from BIRD core
 * handled by radv_if_notify() callback.
 *
 * The main logic of RAdv consists of two functions: radv_iface_notify(), which
 * processes asynchronous events (specified by RA_EV_* codes), and radv_timer(),
 * which triggers sending RAs and computes the next timeout.
 *
 * The RAdv protocol could receive routes (through radv_import_control() and
 * radv_rt_notify()), but only the configured trigger route is tracked (in
 * &active var).  When a radv protocol is reconfigured, the connected routing
 * table is examined (in radv_check_active()) to have proper &active value in
 * case of the specified trigger prefix was changed.
 *
 * Supported standards:
 * - RFC 4861 - main RA standard
 * - RFC 4191 - Default Router Preferences and More-Specific Routes
 * - RFC 6106 - DNS extensions (RDDNS, DNSSL)
 */

static void radv_prune_prefixes(struct radv_iface *ifa);
static void radv_prune_routes(struct radv_proto *p);

/* Invalidate cached RA packet */
static inline void radv_invalidate(struct radv_iface *ifa)
{ ifa->plen = 0; }

static void
radv_timer(timer *tm)
{
  struct radv_iface *ifa = tm->data;
  struct radv_proto *p = ifa->ra;

  RADV_TRACE(D_EVENTS, "Timer fired on %s", ifa->iface->name);

  if (ifa->valid_time <= now)
    radv_invalidate(ifa);

  if (ifa->prune_time <= now)
    radv_prune_prefixes(ifa);

  if (p->prune_time <= now)
    radv_prune_routes(p);

  radv_send_ra(ifa);

  /* Update timer */
  ifa->last = now;
  unsigned after = ifa->cf->min_ra_int;
  after += random() % (ifa->cf->max_ra_int - ifa->cf->min_ra_int + 1);

  if (ifa->initial)
    ifa->initial--;

  if (ifa->initial)
    after = MIN(after, MAX_INITIAL_RTR_ADVERT_INTERVAL);

  tm_start(ifa->timer, after);
}

static struct radv_prefix_config default_prefix = {
  .onlink = 1,
  .autonomous = 1,
  .valid_lifetime = DEFAULT_VALID_LIFETIME,
  .preferred_lifetime = DEFAULT_PREFERRED_LIFETIME
};

static struct radv_prefix_config dead_prefix = {
};

/* Find a corresponding config for the given prefix */
static struct radv_prefix_config *
radv_prefix_match(struct radv_iface *ifa, struct ifa *a)
{
  struct radv_proto *p = ifa->ra;
  struct radv_config *cf = (struct radv_config *) (p->p.cf);
  struct radv_prefix_config *pc;

  if (a->scope <= SCOPE_LINK)
    return NULL;

  WALK_LIST(pc, ifa->cf->pref_list)
    if ((a->pxlen >= pc->pxlen) && ipa_in_net(a->prefix, pc->prefix, pc->pxlen))
      return pc;

  WALK_LIST(pc, cf->pref_list)
    if ((a->pxlen >= pc->pxlen) && ipa_in_net(a->prefix, pc->prefix, pc->pxlen))
      return pc;

  return &default_prefix;
}

/*
 * Go through the list of prefixes, compare them with configs and decide if we
 * want them or not.
 */
static void
radv_prepare_prefixes(struct radv_iface *ifa)
{
  struct radv_proto *p = ifa->ra;
  struct radv_prefix *pfx, *next;

  /* First mark all the prefixes as unused */
  WALK_LIST(pfx, ifa->prefixes)
    pfx->mark = 0;

  /* Find all the prefixes we want to use and make sure they are in the list. */
  struct ifa *addr;
  WALK_LIST(addr, ifa->iface->addrs)
  {
    struct radv_prefix_config *pc = radv_prefix_match(ifa, addr);

    if (!pc || pc->skip)
      continue;

    /* Do we have it already? */
    struct radv_prefix *existing = NULL;
    WALK_LIST(pfx, ifa->prefixes)
      if ((pfx->len == addr->pxlen) && ipa_equal(pfx->prefix, addr->prefix))
      {
	existing = pfx;
	break;
      }

    if (!existing)
    {
      RADV_TRACE(D_EVENTS, "Adding new prefix %I/%d on %s",
		 addr->prefix, addr->pxlen, ifa->iface->name);

      existing = mb_allocz(ifa->pool, sizeof *existing);
      existing->prefix = addr->prefix;
      existing->len = addr->pxlen;
      add_tail(&ifa->prefixes, NODE existing);
    }

    /*
     * Update the information (it may have changed, or even bring a prefix back
     * to life).
     */
    existing->valid = 1;
    existing->changed = now;
    existing->mark = 1;
    existing->cf = pc;
  }

  WALK_LIST_DELSAFE(pfx, next, ifa->prefixes)
  {
    if (pfx->valid && !pfx->mark)
    {
      RADV_TRACE(D_EVENTS, "Invalidating prefix %I/%d on %s",
		 pfx->prefix, pfx->len, ifa->iface->name);

      pfx->valid = 0;
      pfx->changed = now;
      pfx->cf = &dead_prefix;
    }
  }
}

static void
radv_prune_prefixes(struct radv_iface *ifa)
{
  struct radv_proto *p = ifa->ra;
  bird_clock_t next = TIME_INFINITY;
  bird_clock_t expires = 0;

  struct radv_prefix *px, *pxn;
  WALK_LIST_DELSAFE(px, pxn, ifa->prefixes)
  {
    if (!px->valid)
    {
      expires = px->changed + ifa->cf->prefix_linger_time;

      if (expires <= now)
      {
	RADV_TRACE(D_EVENTS, "Removing prefix %I/%d on %s",
		   px->prefix, px->len, ifa->iface->name);

	rem_node(NODE px);
	mb_free(px);
      }
      else
	next = MIN(next, expires);
    }
  }

  ifa->prune_time = next;
}

static char* ev_name[] = { NULL, "Init", "Change", "RS" };

void
radv_iface_notify(struct radv_iface *ifa, int event)
{
  struct radv_proto *p = ifa->ra;

  if (!ifa->sk)
    return;

  RADV_TRACE(D_EVENTS, "Event %s on %s", ev_name[event], ifa->iface->name);

  switch (event)
  {
  case RA_EV_CHANGE:
    radv_invalidate(ifa);
  case RA_EV_INIT:
    ifa->initial = MAX_INITIAL_RTR_ADVERTISEMENTS;
    radv_prepare_prefixes(ifa);
    radv_prune_prefixes(ifa);
    break;

  case RA_EV_RS:
    break;
  }

  /* Update timer */
  unsigned delta = now - ifa->last;
  unsigned after = 0;

  if (delta < ifa->cf->min_delay)
    after = ifa->cf->min_delay - delta;

  tm_start(ifa->timer, after);
}

static void
radv_iface_notify_all(struct radv_proto *p, int event)
{
  struct radv_iface *ifa;

  WALK_LIST(ifa, p->iface_list)
    radv_iface_notify(ifa, event);
}

static struct radv_iface *
radv_iface_find(struct radv_proto *p, struct iface *what)
{
  struct radv_iface *ifa;

  WALK_LIST(ifa, p->iface_list)
    if (ifa->iface == what)
      return ifa;

  return NULL;
}

static void
radv_iface_add(struct object_lock *lock)
{
  struct radv_iface *ifa = lock->data;
  struct radv_proto *p = ifa->ra;

  if (! radv_sk_open(ifa))
  {
    log(L_ERR "%s: Socket open failed on interface %s", p->p.name, ifa->iface->name);
    return;
  }

  radv_iface_notify(ifa, RA_EV_INIT);
}

static void
radv_iface_new(struct radv_proto *p, struct iface *iface, struct radv_iface_config *cf)
{
  struct radv_iface *ifa;

  RADV_TRACE(D_EVENTS, "Adding interface %s", iface->name);

  pool *pool = rp_new(p->p.pool, iface->name);
  ifa = mb_allocz(pool, sizeof(struct radv_iface));
  ifa->pool = pool;
  ifa->ra = p;
  ifa->cf = cf;
  ifa->iface = iface;
  ifa->addr = iface->llv6;
  init_list(&ifa->prefixes);
  ifa->prune_time = TIME_INFINITY;

  add_tail(&p->iface_list, NODE ifa);

  timer *tm = tm_new(pool);
  tm->hook = radv_timer;
  tm->data = ifa;
  tm->randomize = 0;
  tm->recurrent = 0;
  ifa->timer = tm;

  struct object_lock *lock = olock_new(pool);
  lock->type = OBJLOCK_IP;
  lock->port = ICMPV6_PROTO;
  lock->iface = iface;
  lock->data = ifa;
  lock->hook = radv_iface_add;
  ifa->lock = lock;

  olock_acquire(lock);
}

static void
radv_iface_remove(struct radv_iface *ifa)
{
  struct radv_proto *p = ifa->ra;
  RADV_TRACE(D_EVENTS, "Removing interface %s", ifa->iface->name);

  rem_node(NODE ifa);

  rfree(ifa->pool);
}

static void
radv_if_notify(struct proto *P, unsigned flags, struct iface *iface)
{
  struct radv_proto *p = (struct radv_proto *) P;
  struct radv_config *cf = (struct radv_config *) (P->cf);

  if (iface->flags & IF_IGNORE)
    return;

  if (flags & IF_CHANGE_UP)
  {
    struct radv_iface_config *ic = (struct radv_iface_config *)
      iface_patt_find(&cf->patt_list, iface, NULL);

    /* Ignore ifaces without link-local address */
    if (!iface->llv6)
      return;

    if (ic)
      radv_iface_new(p, iface, ic);

    return;
  }

  struct radv_iface *ifa = radv_iface_find(p, iface);
  if (!ifa)
    return;

  if (flags & IF_CHANGE_DOWN)
  {
    radv_iface_remove(ifa);
    return;
  }

  if ((flags & IF_CHANGE_LINK) && (iface->flags & IF_LINK_UP))
    radv_iface_notify(ifa, RA_EV_INIT);
}

static void
radv_ifa_notify(struct proto *P, unsigned flags UNUSED, struct ifa *a)
{
  struct radv_proto *p = (struct radv_proto *) P;

  if (a->flags & IA_SECONDARY)
    return;

  if (a->scope <= SCOPE_LINK)
    return;

  struct radv_iface *ifa = radv_iface_find(p, a->iface);

  if (ifa)
    radv_iface_notify(ifa, RA_EV_CHANGE);
}

static inline int radv_net_match_trigger(struct radv_config *cf, net *n)
{
  return cf->trigger_valid &&
    (n->n.pxlen == cf->trigger_pxlen) &&
    ipa_equal(n->n.prefix, cf->trigger_prefix);
}

int
radv_import_control(struct proto *P, rte **new, ea_list **attrs UNUSED, struct linpool *pool UNUSED)
{
  // struct radv_proto *p = (struct radv_proto *) P;
  struct radv_config *cf = (struct radv_config *) (P->cf);

  if (radv_net_match_trigger(cf, (*new)->net))
    return RIC_PROCESS;

  if (cf->propagate_routes)
    return RIC_PROCESS;
  else
    return RIC_DROP;
}

static void
radv_rt_notify(struct proto *P, rtable *tbl UNUSED, net *n, rte *new, rte *old UNUSED, ea_list *attrs)
{
  struct radv_proto *p = (struct radv_proto *) P;
  struct radv_config *cf = (struct radv_config *) (P->cf);
  struct radv_route *rt;
  eattr *ea;

  if (radv_net_match_trigger(cf, n))
  {
    u8 old_active = p->active;
    p->active = !!new;

    if (p->active == old_active)
      return;

    if (p->active)
      RADV_TRACE(D_EVENTS, "Triggered");
    else
      RADV_TRACE(D_EVENTS, "Suppressed");

    radv_iface_notify_all(p, RA_EV_CHANGE);
    return;
  }

  if (!cf->propagate_routes)
    return;

  /*
   * Some other route we want to send (or stop sending). Update the cache,
   * with marking a removed one as dead or creating a new one as needed.
   *
   * And yes, we exclude the trigger route on purpose.
   */

  if (new)
  {
    /* Update */

    ea = ea_find(attrs, EA_RA_PREFERENCE);
    uint preference = ea ? ea->u.data : RA_PREF_MEDIUM;
    uint preference_set = !!ea;

    ea = ea_find(attrs, EA_RA_LIFETIME);
    uint lifetime = ea ? ea->u.data : 0;
    uint lifetime_set = !!ea;

    if ((preference != RA_PREF_LOW) &&
	(preference != RA_PREF_MEDIUM) &&
	(preference != RA_PREF_HIGH))
    {
      log(L_WARN "%s: Invalid ra_preference value %u on route %I/%d",
	  p->p.name, preference, n->n.prefix, n->n.pxlen);
      preference = RA_PREF_MEDIUM;
      preference_set = 1;
      lifetime = 0;
      lifetime_set = 1;
    }

    rt = fib_get(&p->routes, &n->n.prefix, n->n.pxlen);

    /* Ignore update if nothing changed */
    if (rt->valid &&
	(rt->preference == preference) &&
	(rt->preference_set == preference_set) &&
	(rt->lifetime == lifetime) &&
	(rt->lifetime_set == lifetime_set))
      return;

    if (p->routes.entries == 18)
      log(L_WARN "%s: More than 17 routes exported to RAdv", p->p.name);

    rt->valid = 1;
    rt->changed = now;
    rt->preference = preference;
    rt->preference_set = preference_set;
    rt->lifetime = lifetime;
    rt->lifetime_set = lifetime_set;
  }
  else
  {
    /* Withdraw */
    rt = fib_find(&p->routes, &n->n.prefix, n->n.pxlen);

    if (!rt || !rt->valid)
      return;

    /* Invalidate the route */
    rt->valid = 0;
    rt->changed = now;

    /* Invalidated route will be pruned eventually */
    bird_clock_t expires = rt->changed + cf->max_linger_time;
    p->prune_time = MIN(p->prune_time, expires);
  }

  radv_iface_notify_all(p, RA_EV_CHANGE);
}

/*
 * Cleans up all the dead routes that expired and schedules itself to be run
 * again if there are more routes waiting for expiration.
 */
static void
radv_prune_routes(struct radv_proto *p)
{
  struct radv_config *cf = (struct radv_config *) (p->p.cf);
  bird_clock_t next = TIME_INFINITY;
  bird_clock_t expires = 0;

  /* Should not happen */
  if (!p->fib_up)
    return;

  struct fib_iterator fit;
  FIB_ITERATE_INIT(&fit, &p->routes);

again:
  FIB_ITERATE_START(&p->routes, &fit, node)
  {
    struct radv_route *rt = (void *) node;

    if (!rt->valid)
    {
      expires = rt->changed + cf->max_linger_time;

      /* Delete expired nodes */
      if (expires <= now)
      {
	FIB_ITERATE_PUT(&fit, node);
	fib_delete(&p->routes, node);
	goto again;
      }
      else
	next = MIN(next, expires);
    }
  }
  FIB_ITERATE_END(node);

  p->prune_time = next;
}

static int
radv_check_active(struct radv_proto *p)
{
  struct radv_config *cf = (struct radv_config *) (p->p.cf);

  if (! cf->trigger_valid)
    return 1;

  return rt_examine(p->p.table, cf->trigger_prefix, cf->trigger_pxlen,
		    &(p->p), p->p.cf->out_filter);
}

static struct proto *
radv_init(struct proto_config *c)
{
  struct proto *P = proto_new(c, sizeof(struct radv_proto));

  P->accept_ra_types = RA_OPTIMAL;
  P->import_control = radv_import_control;
  P->rt_notify = radv_rt_notify;
  P->if_notify = radv_if_notify;
  P->ifa_notify = radv_ifa_notify;

  return P;
}

static void
radv_set_fib(struct radv_proto *p, int up)
{
  if (up == p->fib_up)
    return;

  if (up)
    fib_init(&p->routes, p->p.pool, sizeof(struct radv_route), 4, NULL);
  else
    fib_free(&p->routes);

  p->fib_up = up;
  p->prune_time = TIME_INFINITY;
}

static int
radv_start(struct proto *P)
{
  struct radv_proto *p = (struct radv_proto *) P;
  struct radv_config *cf = (struct radv_config *) (P->cf);

  init_list(&(p->iface_list));
  p->valid = 1;
  p->active = !cf->trigger_valid;

  p->fib_up = 0;
  radv_set_fib(p, cf->propagate_routes);
  p->prune_time = TIME_INFINITY;

  return PS_UP;
}

static inline void
radv_iface_shutdown(struct radv_iface *ifa)
{
  if (ifa->sk)
  {
    radv_invalidate(ifa);
    radv_send_ra(ifa);
  }
}

static int
radv_shutdown(struct proto *P)
{
  struct radv_proto *p = (struct radv_proto *) P;

  p->valid = 0;

  struct radv_iface *ifa;
  WALK_LIST(ifa, p->iface_list)
    radv_iface_shutdown(ifa);

  return PS_DOWN;
}

static int
radv_reconfigure(struct proto *P, struct proto_config *c)
{
  struct radv_proto *p = (struct radv_proto *) P;
  struct radv_config *old = (struct radv_config *) (P->cf);
  struct radv_config *new = (struct radv_config *) c;

  P->cf = c; /* radv_check_active() requires proper P->cf */
  p->active = radv_check_active(p);

  /* Allocate or free FIB */
  radv_set_fib(p, new->propagate_routes);

  /* We started to accept routes so we need to refeed them */
  if (!old->propagate_routes && new->propagate_routes)
    proto_request_feeding(&p->p);

  struct iface *iface;
  WALK_LIST(iface, iface_list)
  {
    struct radv_iface *ifa = radv_iface_find(p, iface);
    struct radv_iface_config *ic = (struct radv_iface_config *)
      iface_patt_find(&new->patt_list, iface, NULL);

    if (ifa && ic)
    {
      ifa->cf = ic;

      /* We cheat here - always notify the change even if there isn't
	 any. That would leads just to a few unnecessary RAs. */
      radv_iface_notify(ifa, RA_EV_CHANGE);
    }

    if (ifa && !ic)
    {
      radv_iface_shutdown(ifa);
      radv_iface_remove(ifa);
    }

    if (!ifa && ic)
      radv_iface_new(p, iface, ic);
  }

  return 1;
}

static void
radv_copy_config(struct proto_config *dest, struct proto_config *src)
{
  struct radv_config *d = (struct radv_config *) dest;
  struct radv_config *s = (struct radv_config *) src;

  /* We clean up patt_list, ifaces are non-sharable */
  init_list(&d->patt_list);

  /* We copy pref_list, shallow copy suffices */
  cfg_copy_list(&d->pref_list, &s->pref_list, sizeof(struct radv_prefix_config));
}

static void
radv_get_status(struct proto *P, byte *buf)
{
  struct radv_proto *p = (struct radv_proto *) P;

  if (!p->active)
    strcpy(buf, "Suppressed");
}

static const char *
radv_pref_str(u32 pref)
{
  switch (pref)
  {
    case RA_PREF_LOW:
      return "low";
    case RA_PREF_MEDIUM:
      return "medium";
    case RA_PREF_HIGH:
      return "high";
    default:
      return "??";
  }
}

/* The buffer has some minimal size */
static int
radv_get_attr(eattr *a, byte *buf, int buflen UNUSED)
{
  switch (a->id)
  {
  case EA_RA_PREFERENCE:
    bsprintf(buf, "preference: %s", radv_pref_str(a->u.data));
    return GA_FULL;
  case EA_RA_LIFETIME:
    bsprintf(buf, "lifetime");
    return GA_NAME;
  default:
    return GA_UNKNOWN;
  }
}

struct protocol proto_radv = {
  .name =		"RAdv",
  .template =		"radv%d",
  .attr_class =		EAP_RADV,
  .config_size =	sizeof(struct radv_config),
  .init =		radv_init,
  .start =		radv_start,
  .shutdown =		radv_shutdown,
  .reconfigure =	radv_reconfigure,
  .copy_config =	radv_copy_config,
  .get_status =		radv_get_status,
  .get_attr =		radv_get_attr
};
