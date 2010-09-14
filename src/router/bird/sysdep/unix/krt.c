/*
 *	BIRD -- UNIX Kernel Synchronization
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Kernel synchronization
 *
 * This system dependent module implements the Kernel and Device protocol,
 * that is synchronization of interface lists and routing tables with the
 * OS kernel.
 *
 * The whole kernel synchronization is a bit messy and touches some internals
 * of the routing table engine, because routing table maintenance is a typical
 * example of the proverbial compatibility between different Unices and we want
 * to keep the overhead of our KRT business as low as possible and avoid maintaining
 * a local routing table copy.
 *
 * The kernel syncer can work in three different modes (according to system config header):
 * Either with a single routing table and single KRT protocol [traditional UNIX]
 * or with many routing tables and separate KRT protocols for all of them
 * or with many routing tables, but every scan including all tables, so we start
 * separate KRT protocols which cooperate with each other  [Linux 2.2].
 * In this case, we keep only a single scan timer.
 *
 * We use FIB node flags in the routing table to keep track of route
 * synchronization status. We also attach temporary &rte's to the routing table,
 * but it cannot do any harm to the rest of BIRD since table synchronization is
 * an atomic process.
 *
 * When starting up, we cheat by looking if there is another
 * KRT instance to be initialized later and performing table scan
 * only once for all the instances.
 */

/*
 *  If you are brave enough, continue now.  You cannot say you haven't been warned.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/route.h"
#include "nest/protocol.h"
#include "lib/timer.h"
#include "conf/conf.h"
#include "lib/string.h"

#include "unix.h"
#include "krt.h"

static int krt_uptodate(rte *k, rte *e);

/*
 *	Global resources
 */

pool *krt_pool;

void
krt_io_init(void)
{
  krt_pool = rp_new(&root_pool, "Kernel Syncer");
  krt_if_io_init();
}

/*
 *	Interfaces
 */

struct proto_config *cf_kif;

static struct kif_proto *kif_proto;
static timer *kif_scan_timer;
static bird_clock_t kif_last_shot;

static void
kif_preconfig(struct protocol *P UNUSED, struct config *c UNUSED)
{
  cf_kif = NULL;
}

static void
kif_scan(timer *t)
{
  struct kif_proto *p = t->data;

  KRT_TRACE(p, D_EVENTS, "Scanning interfaces");
  kif_last_shot = now;
  krt_if_scan(p);
}

static void
kif_force_scan(void)
{
  if (kif_proto && kif_last_shot + 2 < now)
    {
      kif_scan(kif_scan_timer);
      tm_start(kif_scan_timer, ((struct kif_config *) kif_proto->p.cf)->scan_time);
    }
}

static struct proto *
kif_init(struct proto_config *c)
{
  struct kif_proto *p = proto_new(c, sizeof(struct kif_proto));
  return &p->p;
}

static int
kif_start(struct proto *P)
{
  struct kif_proto *p = (struct kif_proto *) P;

  kif_proto = p;
  krt_if_start(p);

  /* Start periodic interface scanning */
  kif_scan_timer = tm_new(P->pool);
  kif_scan_timer->hook = kif_scan;
  kif_scan_timer->data = p;
  kif_scan_timer->recurrent = KIF_CF->scan_time;
  kif_scan(kif_scan_timer);
  tm_start(kif_scan_timer, KIF_CF->scan_time);

  return PS_UP;
}

static int
kif_shutdown(struct proto *P)
{
  struct kif_proto *p = (struct kif_proto *) P;

  tm_stop(kif_scan_timer);
  krt_if_shutdown(p);
  kif_proto = NULL;

  return PS_DOWN;
}


static inline int
prefer_scope(struct ifa *a, struct ifa *b)
{ return (a->scope > SCOPE_LINK) && (b->scope <= SCOPE_LINK); }

static inline int
prefer_addr(struct ifa *a, struct ifa *b)
{ return ipa_compare(a->ip, b->ip) < 0; }

static inline struct ifa *
find_preferred_ifa(struct iface *i, ip_addr prefix, ip_addr mask)
{
  struct ifa *a, *b = NULL;

  WALK_LIST(a, i->addrs)
    {
      if (!(a->flags & IA_SECONDARY) &&
	  ipa_equal(ipa_and(a->ip, mask), prefix) &&
	  (!b || prefer_scope(a, b) || prefer_addr(a, b)))
	b = a;
    }

  return b;
}

struct ifa *
kif_choose_primary(struct iface *i)
{
  struct kif_config *cf = (struct kif_config *) (kif_proto->p.cf);
  struct kif_primary_item *it;
  struct ifa *a;

  WALK_LIST(it, cf->primary)
    {
      if (!it->pattern || patmatch(it->pattern, i->name))
	if (a = find_preferred_ifa(i, it->prefix, ipa_mkmask(it->pxlen)))
	  return a;
    }

  return find_preferred_ifa(i, IPA_NONE, IPA_NONE);
}


static int
kif_reconfigure(struct proto *p, struct proto_config *new)
{
  struct kif_config *o = (struct kif_config *) p->cf;
  struct kif_config *n = (struct kif_config *) new;

  if (!kif_params_same(&o->iface, &n->iface))
    return 0;

  if (o->scan_time != n->scan_time)
    {
      tm_stop(kif_scan_timer);
      kif_scan_timer->recurrent = n->scan_time;
      kif_scan(kif_scan_timer);
      tm_start(kif_scan_timer, n->scan_time);
    }

  if (!EMPTY_LIST(o->primary) || !EMPTY_LIST(n->primary))
    {
      /* This is hack, we have to update a configuration
       * to the new value just now, because it is used
       * for recalculation of primary addresses.
       */
      p->cf = new;

      ifa_recalc_all_primary_addresses();
    }

  return 1;
}

struct protocol proto_unix_iface = {
  name:		"Device",
  template:	"device%d",
  preconfig:	kif_preconfig,
  init:		kif_init,
  start:	kif_start,
  shutdown:	kif_shutdown,
  reconfigure:	kif_reconfigure,
};

/*
 *	Tracing of routes
 */

static inline void
krt_trace_in(struct krt_proto *p, rte *e, char *msg)
{
  if (p->p.debug & D_PACKETS)
    log(L_TRACE "%s: %I/%d: %s", p->p.name, e->net->n.prefix, e->net->n.pxlen, msg);
}

static inline void
krt_trace_in_rl(struct rate_limit *rl, struct krt_proto *p, rte *e, char *msg)
{
  if (p->p.debug & D_PACKETS)
    log_rl(rl, L_TRACE "%s: %I/%d: %s", p->p.name, e->net->n.prefix, e->net->n.pxlen, msg);
}

/*
 *	Inherited Routes
 */

#ifdef KRT_ALLOW_LEARN

static struct rate_limit rl_alien_seen, rl_alien_updated, rl_alien_created, rl_alien_ignored;

static inline int
krt_same_key(rte *a, rte *b)
{
  return a->u.krt.proto == b->u.krt.proto &&
         a->u.krt.metric == b->u.krt.metric &&
         a->u.krt.type == b->u.krt.type;
}

static void
krt_learn_announce_update(struct krt_proto *p, rte *e)
{
  net *n = e->net;
  rta *aa = rta_clone(e->attrs);
  rte *ee = rte_get_temp(aa);
  net *nn = net_get(p->p.table, n->n.prefix, n->n.pxlen);
  ee->net = nn;
  ee->pflags = 0;
  ee->pref = p->p.preference;
  ee->u.krt = e->u.krt;
  rte_update(p->p.table, nn, &p->p, &p->p, ee);
}

static void
krt_learn_announce_delete(struct krt_proto *p, net *n)
{
  n = net_find(p->p.table, n->n.prefix, n->n.pxlen);
  if (n)
    rte_update(p->p.table, n, &p->p, &p->p, NULL);
}

static void
krt_learn_scan(struct krt_proto *p, rte *e)
{
  net *n0 = e->net;
  net *n = net_get(&p->krt_table, n0->n.prefix, n0->n.pxlen);
  rte *m, **mm;

  e->attrs->source = RTS_INHERIT;

  for(mm=&n->routes; m = *mm; mm=&m->next)
    if (krt_same_key(m, e))
      break;
  if (m)
    {
      if (krt_uptodate(m, e))
	{
	  krt_trace_in_rl(&rl_alien_seen, p, e, "[alien] seen");
	  rte_free(e);
	  m->u.krt.seen = 1;
	}
      else
	{
	  krt_trace_in_rl(&rl_alien_updated, p, e, "[alien] updated");
	  *mm = m->next;
	  rte_free(m);
	  m = NULL;
	}
    }
  else
    krt_trace_in_rl(&rl_alien_created, p, e, "[alien] created");
  if (!m)
    {
      e->attrs = rta_lookup(e->attrs);
      e->next = n->routes;
      n->routes = e;
      e->u.krt.seen = 1;
    }
}

static void
krt_learn_prune(struct krt_proto *p)
{
  struct fib *fib = &p->krt_table.fib;
  struct fib_iterator fit;

  KRT_TRACE(p, D_EVENTS, "Pruning inherited routes");

  FIB_ITERATE_INIT(&fit, fib);
again:
  FIB_ITERATE_START(fib, &fit, f)
    {
      net *n = (net *) f;
      rte *e, **ee, *best, **pbest, *old_best;

      old_best = n->routes;
      best = NULL;
      pbest = NULL;
      ee = &n->routes;
      while (e = *ee)
	{
	  if (!e->u.krt.seen)
	    {
	      *ee = e->next;
	      rte_free(e);
	      continue;
	    }
	  if (!best || best->u.krt.metric > e->u.krt.metric)
	    {
	      best = e;
	      pbest = ee;
	    }
	  e->u.krt.seen = 0;
	  ee = &e->next;
	}
      if (!n->routes)
	{
	  DBG("%I/%d: deleting\n", n->n.prefix, n->n.pxlen);
	  if (old_best)
	    {
	      krt_learn_announce_delete(p, n);
	      n->n.flags &= ~KRF_INSTALLED;
	    }
	  FIB_ITERATE_PUT(&fit, f);
	  fib_delete(fib, f);
	  goto again;
	}
      *pbest = best->next;
      best->next = n->routes;
      n->routes = best;
      if (best != old_best || !(n->n.flags & KRF_INSTALLED))
	{
	  DBG("%I/%d: announcing (metric=%d)\n", n->n.prefix, n->n.pxlen, best->u.krt.metric);
	  krt_learn_announce_update(p, best);
	  n->n.flags |= KRF_INSTALLED;
	}
      else
	DBG("%I/%d: uptodate (metric=%d)\n", n->n.prefix, n->n.pxlen, best->u.krt.metric);
    }
  FIB_ITERATE_END(f);
}

static void
krt_learn_async(struct krt_proto *p, rte *e, int new)
{
  net *n0 = e->net;
  net *n = net_get(&p->krt_table, n0->n.prefix, n0->n.pxlen);
  rte *g, **gg, *best, **bestp, *old_best;

  e->attrs->source = RTS_INHERIT;

  old_best = n->routes;
  for(gg=&n->routes; g = *gg; gg = &g->next)
    if (krt_same_key(g, e))
      break;
  if (new)
    {
      if (g)
	{
	  if (krt_uptodate(g, e))
	    {
	      krt_trace_in(p, e, "[alien async] same");
	      rte_free(e);
	      return;
	    }
	  krt_trace_in(p, e, "[alien async] updated");
	  *gg = g->next;
	  rte_free(g);
	}
      else
	krt_trace_in(p, e, "[alien async] created");
      e->attrs = rta_lookup(e->attrs);
      e->next = n->routes;
      n->routes = e;
    }
  else if (!g)
    {
      krt_trace_in(p, e, "[alien async] delete failed");
      rte_free(e);
      return;
    }
  else
    {
      krt_trace_in(p, e, "[alien async] removed");
      *gg = g->next;
      rte_free(e);
      rte_free(g);
    }
  best = n->routes;
  bestp = &n->routes;
  for(gg=&n->routes; g=*gg; gg=&g->next)
    if (best->u.krt.metric > g->u.krt.metric)
      {
	best = g;
	bestp = gg;
      }
  if (best)
    {
      *bestp = best->next;
      best->next = n->routes;
      n->routes = best;
    }
  if (best != old_best)
    {
      DBG("krt_learn_async: distributing change\n");
      if (best)
	{
	  krt_learn_announce_update(p, best);
	  n->n.flags |= KRF_INSTALLED;
	}
      else
	{
	  n->routes = NULL;
	  krt_learn_announce_delete(p, n);
	  n->n.flags &= ~KRF_INSTALLED;
	}
    }
}

static void
krt_learn_init(struct krt_proto *p)
{
  if (KRT_CF->learn)
    rt_setup(p->p.pool, &p->krt_table, "Inherited", NULL);
}

static void
krt_dump(struct proto *P)
{
  struct krt_proto *p = (struct krt_proto *) P;

  if (!KRT_CF->learn)
    return;
  bdebug("KRT: Table of inheritable routes\n");
  rt_dump(&p->krt_table);
}

static void
krt_dump_attrs(rte *e)
{
  bdebug(" [m=%d,p=%d,t=%d]", e->u.krt.metric, e->u.krt.proto, e->u.krt.type);
}

#endif

/*
 *	Routes
 */

#ifdef CONFIG_ALL_TABLES_AT_ONCE
static timer *krt_scan_timer;
static int krt_instance_count;
static list krt_instance_list;
#endif

static void
krt_flush_routes(struct krt_proto *p)
{
  struct rtable *t = p->p.table;

  KRT_TRACE(p, D_EVENTS, "Flushing kernel routes");
  FIB_WALK(&t->fib, f)
    {
      net *n = (net *) f;
      rte *e = n->routes;
      if (e)
	{
	  rta *a = e->attrs;
	  if ((n->n.flags & KRF_INSTALLED) &&
	      a->source != RTS_DEVICE && a->source != RTS_INHERIT)
	    {
	      krt_set_notify(p, e->net, NULL, e);
	      n->n.flags &= ~KRF_INSTALLED;
	    }
	}
    }
  FIB_WALK_END;
}

static int
krt_uptodate(rte *k, rte *e)
{
  rta *ka = k->attrs, *ea = e->attrs;

  if (ka->dest != ea->dest)
    return 0;
  switch (ka->dest)
    {
    case RTD_ROUTER:
      return ipa_equal(ka->gw, ea->gw);
    case RTD_DEVICE:
      return !strcmp(ka->iface->name, ea->iface->name);
    default:
      return 1;
    }
}

/*
 *  This gets called back when the low-level scanning code discovers a route.
 *  We expect that the route is a temporary rte and its attributes are uncached.
 */

void
krt_got_route(struct krt_proto *p, rte *e)
{
  rte *old;
  net *net = e->net;
  int verdict;

#ifdef KRT_ALLOW_LEARN
  switch (e->u.krt.src)
    {
    case KRT_SRC_KERNEL:
      verdict = KRF_IGNORE;
      goto sentenced;

    case KRT_SRC_REDIRECT:
      verdict = KRF_DELETE;
      goto sentenced;

    case  KRT_SRC_ALIEN:
      if (KRT_CF->learn)
	krt_learn_scan(p, e);
      else
	{
	  krt_trace_in_rl(&rl_alien_ignored, p, e, "[alien] ignored");
	  rte_free(e);
	}
      return;
    }
#endif
  /* The rest is for KRT_SRC_BIRD (or KRT_SRC_UNKNOWN) */

  if (net->n.flags & KRF_VERDICT_MASK)
    {
      /* Route to this destination was already seen. Strange, but it happens... */
      krt_trace_in(p, e, "already seen");
      rte_free(e);
      return;
    }

  if (net->n.flags & KRF_INSTALLED)
    {
      old = net->routes;
      ASSERT(old);
      if (krt_uptodate(e, old))
	verdict = KRF_SEEN;
      else
	verdict = KRF_UPDATE;
    }
  else
    verdict = KRF_DELETE;

 sentenced:
  krt_trace_in(p, e, ((char *[]) { "?", "seen", "will be updated", "will be removed", "ignored" }) [verdict]);
  net->n.flags = (net->n.flags & ~KRF_VERDICT_MASK) | verdict;
  if (verdict == KRF_UPDATE || verdict == KRF_DELETE)
    {
      /* Get a cached copy of attributes and link the route */
      rta *a = e->attrs;
      a->source = RTS_DUMMY;
      e->attrs = rta_lookup(a);
      e->next = net->routes;
      net->routes = e;
    }
  else
    rte_free(e);
}

static void
krt_prune(struct krt_proto *p)
{
  struct rtable *t = p->p.table;

  KRT_TRACE(p, D_EVENTS, "Pruning table %s", t->name);
  FIB_WALK(&t->fib, f)
    {
      net *n = (net *) f;
      int verdict = f->flags & KRF_VERDICT_MASK;
      rte *new, *old;

      if (verdict != KRF_CREATE && verdict != KRF_SEEN && verdict != KRF_IGNORE)
	{
	  old = n->routes;
	  n->routes = old->next;
	}
      else
	old = NULL;
      new = n->routes;

      switch (verdict)
	{
	case KRF_CREATE:
	  if (new && (f->flags & KRF_INSTALLED))
	    {
	      krt_trace_in(p, new, "reinstalling");
	      krt_set_notify(p, n, new, NULL);
	    }
	  break;
	case KRF_SEEN:
	case KRF_IGNORE:
	  /* Nothing happens */
	  break;
	case KRF_UPDATE:
	  krt_trace_in(p, new, "updating");
	  krt_set_notify(p, n, new, old);
	  break;
	case KRF_DELETE:
	  krt_trace_in(p, old, "deleting");
	  krt_set_notify(p, n, NULL, old);
	  break;
	default:
	  bug("krt_prune: invalid route status");
	}
      if (old)
	rte_free(old);
      f->flags &= ~KRF_VERDICT_MASK;
    }
  FIB_WALK_END;

#ifdef KRT_ALLOW_LEARN
  if (KRT_CF->learn)
    krt_learn_prune(p);
#endif
  p->initialized = 1;
}

void
krt_got_route_async(struct krt_proto *p, rte *e, int new)
{
  net *net = e->net;

  switch (e->u.krt.src)
    {
    case KRT_SRC_BIRD:
      ASSERT(0);			/* Should be filtered by the back end */

    case KRT_SRC_REDIRECT:
      if (new)
	{
	  krt_trace_in(p, e, "[redirect] deleting");
	  krt_set_notify(p, net, NULL, e);
	}
      /* If !new, it is probably echo of our deletion */
      break;

#ifdef KRT_ALLOW_LEARN
    case KRT_SRC_ALIEN:
      if (KRT_CF->learn)
	{
	  krt_learn_async(p, e, new);
	  return;
	}
#endif
    }
  rte_free(e);
}

/*
 *	Periodic scanning
 */

static void
krt_scan(timer *t UNUSED)
{
  struct krt_proto *p;

  kif_force_scan();
#ifdef CONFIG_ALL_TABLES_AT_ONCE
  {
    void *q;
    /* We need some node to decide whether to print the debug messages or not */
    p = SKIP_BACK(struct krt_proto, instance_node, HEAD(krt_instance_list));
    if (p->instance_node.next)
      KRT_TRACE(p, D_EVENTS, "Scanning routing table");
    krt_scan_fire(NULL);
    WALK_LIST(q, krt_instance_list)
      {
	p = SKIP_BACK(struct krt_proto, instance_node, q);
	krt_prune(p);
      }
  }
#else
  p = t->data;
  KRT_TRACE(p, D_EVENTS, "Scanning routing table");
  krt_scan_fire(p);
  krt_prune(p);
#endif
}

/*
 *	Updates
 */
static int
krt_import_control(struct proto *P, rte **new, ea_list **attrs, struct linpool *pool)
{
  struct krt_proto *p = (struct krt_proto *) P;
  rte *e = *new;

  if (e->attrs->proto == P)
    return -1;

  if (!KRT_CF->devroutes && 
      (e->attrs->dest == RTD_DEVICE) && 
      (e->attrs->source != RTS_STATIC_DEVICE))
    return -1;

  if (!krt_capable(e))
    return -1;

  return 0;
}

static void
krt_notify(struct proto *P, struct rtable *table UNUSED, net *net,
	   rte *new, rte *old, struct ea_list *attrs UNUSED)
{
  struct krt_proto *p = (struct krt_proto *) P;

  if (shutting_down)
    return;
  if (!(net->n.flags & KRF_INSTALLED))
    old = NULL;
  if (new)
    net->n.flags |= KRF_INSTALLED;
  else
    net->n.flags &= ~KRF_INSTALLED;
  if (p->initialized)			/* Before first scan we don't touch the routes */
    krt_set_notify(p, net, new, old);
}

/*
 *	Protocol glue
 */

struct proto_config *cf_krt;

static void
krt_preconfig(struct protocol *P UNUSED, struct config *c)
{
  cf_krt = NULL;
  krt_scan_preconfig(c);
}

static void
krt_postconfig(struct proto_config *C)
{
  struct krt_config *c = (struct krt_config *) C;

#ifdef CONFIG_ALL_TABLES_AT_ONCE
  struct krt_config *first = (struct krt_config *) cf_krt;
  if (first->scan_time != c->scan_time)
    cf_error("All kernel syncers must use the same table scan interval");
#endif

  if (C->table->krt_attached)
    cf_error("Kernel syncer (%s) already attached to table %s", C->table->krt_attached->name, C->table->name);
  C->table->krt_attached = C;
  krt_scan_postconfig(c);
}

static timer *
krt_start_timer(struct krt_proto *p)
{
  timer *t;

  t = tm_new(p->krt_pool);
  t->hook = krt_scan;
  t->data = p;
  t->recurrent = KRT_CF->scan_time;
  tm_start(t, 0);
  return t;
}

static int
krt_start(struct proto *P)
{
  struct krt_proto *p = (struct krt_proto *) P;
  int first = 1;

#ifdef CONFIG_ALL_TABLES_AT_ONCE
  if (!krt_instance_count++)
    init_list(&krt_instance_list);
  else
    first = 0;
  p->krt_pool = krt_pool;
  add_tail(&krt_instance_list, &p->instance_node);
#else
  p->krt_pool = P->pool;
#endif

#ifdef KRT_ALLOW_LEARN
  krt_learn_init(p);
#endif

  krt_scan_start(p, first);
  krt_set_start(p, first);

  /* Start periodic routing table scanning */
#ifdef CONFIG_ALL_TABLES_AT_ONCE
  if (first)
    krt_scan_timer = krt_start_timer(p);
  else
    tm_start(krt_scan_timer, 0);
  p->scan_timer = krt_scan_timer;
#else
  p->scan_timer = krt_start_timer(p);
#endif

  return PS_UP;
}

static int
krt_shutdown(struct proto *P)
{
  struct krt_proto *p = (struct krt_proto *) P;
  int last = 1;

#ifdef CONFIG_ALL_TABLES_AT_ONCE
  rem_node(&p->instance_node);
  if (--krt_instance_count)
    last = 0;
  else
#endif
    tm_stop(p->scan_timer);

  /* FIXME we should flush routes even when persist during reconfiguration */
  if (p->initialized && !KRT_CF->persist)
    krt_flush_routes(p);

  krt_set_shutdown(p, last);
  krt_scan_shutdown(p, last);

#ifdef CONFIG_ALL_TABLES_AT_ONCE
  if (last)
    rfree(krt_scan_timer);
#endif

  return PS_DOWN;
}

static struct proto *
krt_init(struct proto_config *c)
{
  struct krt_proto *p = proto_new(c, sizeof(struct krt_proto));

  p->p.accept_ra_types = RA_OPTIMAL;
  p->p.import_control = krt_import_control;
  p->p.rt_notify = krt_notify;
  return &p->p;
}

static int
krt_reconfigure(struct proto *p, struct proto_config *new)
{
  struct krt_config *o = (struct krt_config *) p->cf;
  struct krt_config *n = (struct krt_config *) new;

  return o->scan_time == n->scan_time
    && o->learn == n->learn		/* persist needn't be the same */
    && o->devroutes == n->devroutes
    && krt_set_params_same(&o->set, &n->set)
    && krt_scan_params_same(&o->scan, &n->scan)
    ;
}

struct protocol proto_unix_kernel = {
  name:		"Kernel",
  template:	"kernel%d",
  preconfig:	krt_preconfig,
  postconfig:	krt_postconfig,
  init:		krt_init,
  start:	krt_start,
  shutdown:	krt_shutdown,
  reconfigure:	krt_reconfigure,
#ifdef KRT_ALLOW_LEARN
  dump:		krt_dump,
  dump_attrs:	krt_dump_attrs,
#endif
};
