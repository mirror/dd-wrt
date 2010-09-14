/*
 *	BIRD -- Protocols
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/protocol.h"
#include "lib/resource.h"
#include "lib/lists.h"
#include "lib/event.h"
#include "lib/string.h"
#include "conf/conf.h"
#include "nest/route.h"
#include "nest/iface.h"
#include "nest/cli.h"
#include "filter/filter.h"

pool *proto_pool;

static list protocol_list;
static list proto_list;

#define PD(pr, msg, args...) do { if (pr->debug & D_STATES) { log(L_TRACE "%s: " msg, pr->name , ## args); } } while(0)

list active_proto_list;
static list inactive_proto_list;
static list initial_proto_list;
static list flush_proto_list;

static event *proto_flush_event;

static char *p_states[] = { "DOWN", "START", "UP", "STOP" };
static char *c_states[] = { "HUNGRY", "FEEDING", "HAPPY", "FLUSHING" };

static void proto_flush_all(void *);
static void proto_rethink_goal(struct proto *p);
static char *proto_state_name(struct proto *p);

static void
proto_enqueue(list *l, struct proto *p)
{
  add_tail(l, &p->n);
  p->last_state_change = now;
}

static void
proto_relink(struct proto *p)
{
  list *l = NULL;

  if (p->debug & D_STATES)
    {
      char *name = proto_state_name(p);
      if (name != p->last_state_name_announced)
	{
	  p->last_state_name_announced = name;
	  PD(p, "State changed to %s", proto_state_name(p));
	}
    }
  else
    p->last_state_name_announced = NULL;
  rem_node(&p->n);
  switch (p->core_state)
    {
    case FS_HUNGRY:
      l = &inactive_proto_list;
      break;
    case FS_FEEDING:
    case FS_HAPPY:
      l = &active_proto_list;
      break;
    case FS_FLUSHING:
      l = &flush_proto_list;
      break;
    default:
      ASSERT(0);
    }
  proto_enqueue(l, p);
}

/**
 * proto_new - create a new protocol instance
 * @c: protocol configuration
 * @size: size of protocol data structure (each protocol instance is represented by
 * a structure starting with generic part [struct &proto] and continued
 * with data specific to the protocol)
 *
 * When a new configuration has been read in, the core code starts
 * initializing all the protocol instances configured by calling their
 * init() hooks with the corresponding instance configuration. The initialization
 * code of the protocol is expected to create a new instance according to the
 * configuration by calling this function and then modifying the default settings
 * to values wanted by the protocol.
 */
void *
proto_new(struct proto_config *c, unsigned size)
{
  struct protocol *pr = c->protocol;
  struct proto *p = mb_allocz(proto_pool, size);

  p->cf = c;
  p->debug = c->debug;
  p->mrtdump = c->mrtdump;
  p->name = c->name;
  p->preference = c->preference;
  p->disabled = c->disabled;
  p->proto = pr;
  p->table = c->table->table;
  p->in_filter = c->in_filter;
  p->out_filter = c->out_filter;
  p->hash_key = random_u32();
  c->proto = p;
  return p;
}

static void
proto_init_instance(struct proto *p)
{
  /* Here we cannot use p->cf->name since it won't survive reconfiguration */
  p->pool = rp_new(proto_pool, p->proto->name);
  p->attn = ev_new(p->pool);
  p->attn->data = p;
  rt_lock_table(p->table);
}

/**
 * proto_add_announce_hook - connect protocol to a routing table
 * @p: protocol instance
 * @t: routing table to connect to
 *
 * This function creates a connection between the protocol instance @p
 * and the routing table @t, making the protocol hear all changes in
 * the table.
 *
 * Unless you want to listen to multiple routing tables (as the Pipe
 * protocol does), you needn't to worry about this function since the
 * connection to the protocol's primary routing table is initialized
 * automatically by the core code.
 */
struct announce_hook *
proto_add_announce_hook(struct proto *p, struct rtable *t)
{
  struct announce_hook *h;

  if (!p->rt_notify)
    return NULL;
  DBG("Connecting protocol %s to table %s\n", p->name, t->name);
  PD(p, "Connected to table %s", t->name);
  h = mb_alloc(p->pool, sizeof(struct announce_hook));
  h->table = t;
  h->proto = p;
  h->next = p->ahooks;
  p->ahooks = h;
  add_tail(&t->hooks, &h->n);
  return h;
}

static void
proto_flush_hooks(struct proto *p)
{
  struct announce_hook *h;

  for(h=p->ahooks; h; h=h->next)
    rem_node(&h->n);
  p->ahooks = NULL;
}

/**
 * proto_config_new - create a new protocol configuration
 * @pr: protocol the configuration will belong to
 * @size: size of the structure including generic data
 *
 * Whenever the configuration file says that a new instance
 * of a routing protocol should be created, the parser calls
 * proto_config_new() to create a configuration entry for this
 * instance (a structure staring with the &proto_config header
 * containing all the generic items followed by protocol-specific
 * ones). Also, the configuration entry gets added to the list
 * of protocol instances kept in the configuration.
 */
void *
proto_config_new(struct protocol *pr, unsigned size)
{
  struct proto_config *c = cfg_allocz(size);

  add_tail(&new_config->protos, &c->n);
  c->global = new_config;
  c->protocol = pr;
  c->name = pr->name;
  c->out_filter = FILTER_REJECT;
  c->table = c->global->master_rtc;
  c->debug = new_config->proto_default_debug;
  c->mrtdump = new_config->proto_default_mrtdump;
  return c;
}

/**
 * protos_preconfig - pre-configuration processing
 * @c: new configuration
 *
 * This function calls the preconfig() hooks of all routing
 * protocols available to prepare them for reading of the new
 * configuration.
 */
void
protos_preconfig(struct config *c)
{
  struct protocol *p;

  init_list(&c->protos);
  DBG("Protocol preconfig:");
  WALK_LIST(p, protocol_list)
    {
      DBG(" %s", p->name);
      p->name_counter = 0;
      if (p->preconfig)
	p->preconfig(p, c);
    }
  DBG("\n");
}

/**
 * protos_postconfig - post-configuration processing
 * @c: new configuration
 *
 * This function calls the postconfig() hooks of all protocol
 * instances specified in configuration @c.
 */
void
protos_postconfig(struct config *c)
{
  struct proto_config *x;
  struct protocol *p;

  DBG("Protocol postconfig:");
  WALK_LIST(x, c->protos)
    {
      DBG(" %s", x->name);
      p = x->protocol;
      if (p->postconfig)
	p->postconfig(x);
    }
  DBG("\n");
}

static struct proto *
proto_init(struct proto_config *c)
{
  struct protocol *p = c->protocol;
  struct proto *q = p->init(c);

  q->proto_state = PS_DOWN;
  q->core_state = FS_HUNGRY;
  proto_enqueue(&initial_proto_list, q);
  add_tail(&proto_list, &q->glob_node);
  PD(q, "Initializing%s", q->disabled ? " [disabled]" : "");
  return q;
}

static int
proto_reconfigure(struct proto *p, struct proto_config *oc, struct proto_config *nc, int type)
{
  /* If the protocol is DOWN, we just restart it */
  if (p->proto_state == PS_DOWN)
    return 0;

  /* If there is a too big change in core attributes, ... */
  if ((nc->protocol != oc->protocol) ||
      (nc->disabled != oc->disabled) ||
      (nc->table->table != oc->table->table) ||
      (proto_get_router_id(nc) != proto_get_router_id(oc)))
    return 0;

  int import_changed = (type != RECONFIG_SOFT) && ! filter_same(nc->in_filter, oc->in_filter);
  int export_changed = (type != RECONFIG_SOFT) && ! filter_same(nc->out_filter, oc->out_filter);

  /* We treat a change in preferences by reimporting routes */
  if (nc->preference != oc->preference)
    import_changed = 1;

  /* If the protocol in not UP, it has no routes and we can ignore such changes */
  if (p->proto_state != PS_UP)
    import_changed = export_changed = 0;

  /* Without this hook we cannot reload routes and have to restart the protocol */
  if (import_changed && ! p->reload_routes)
    return 0;

  p->debug = nc->debug;
  p->mrtdump = nc->mrtdump;

  /* Execute protocol specific reconfigure hook */
  if (! (p->proto->reconfigure && p->proto->reconfigure(p, nc)))
    return 0;

  DBG("\t%s: same\n", oc->name);
  PD(p, "Reconfigured");
  p->cf = nc;
  p->name = nc->name;
  p->in_filter = nc->in_filter;
  p->out_filter = nc->out_filter;
  p->preference = nc->preference;

  if (import_changed || export_changed)
    log(L_INFO "Reloading protocol %s", p->name);

  if (import_changed && ! p->reload_routes(p))
    {
      /* Now, the protocol is reconfigured. But route reload failed
	 and we have to do regular protocol restart. */
      log(L_INFO "Restarting protocol %s", p->name);
      p->disabled = 1;
      proto_rethink_goal(p);
      p->disabled = 0;
      proto_rethink_goal(p);
      return 1;
    }

  if (export_changed)
    proto_request_feeding(p);

  return 1;
}

/**
 * protos_commit - commit new protocol configuration
 * @new: new configuration
 * @old: old configuration or %NULL if it's boot time config
 * @force_reconfig: force restart of all protocols (used for example
 * when the router ID changes)
 * @type: type of reconfiguration (RECONFIG_SOFT or RECONFIG_HARD)
 *
 * Scan differences between @old and @new configuration and adjust all
 * protocol instances to conform to the new configuration.
 *
 * When a protocol exists in the new configuration, but it doesn't in the
 * original one, it's immediately started. When a collision with the other
 * running protocol would arise, the new protocol will be temporarily stopped
 * by the locking mechanism.
 *
 * When a protocol exists in the old configuration, but it doesn't in the
 * new one, it's shut down and deleted after the shutdown completes.
 *
 * When a protocol exists in both configurations, the core decides
 * whether it's possible to reconfigure it dynamically - it checks all
 * the core properties of the protocol (changes in filters are ignored
 * if type is RECONFIG_SOFT) and if they match, it asks the
 * reconfigure() hook of the protocol to see if the protocol is able
 * to switch to the new configuration.  If it isn't possible, the
 * protocol is shut down and a new instance is started with the new
 * configuration after the shutdown is completed.
 */
void
protos_commit(struct config *new, struct config *old, int force_reconfig, int type)
{
  struct proto_config *oc, *nc;
  struct proto *p, *n;

  DBG("protos_commit:\n");
  if (old)
    {
      WALK_LIST(oc, old->protos)
	{
	  struct proto *p = oc->proto;
	  struct symbol *sym = cf_find_symbol(oc->name);
	  if (sym && sym->class == SYM_PROTO && !new->shutdown)
	    {
	      /* Found match, let's check if we can smoothly switch to new configuration */
	      /* No need to check description */
	      nc = sym->def;
	      nc->proto = p;

	      /* We will try to reconfigure protocol p */
	      if (! force_reconfig && proto_reconfigure(p, oc, nc, type))
		continue;

	      /* Unsuccessful, we will restart it */
	      if (!p->disabled && !nc->disabled)
		log(L_INFO "Restarting protocol %s", p->name);
	      else if (p->disabled && !nc->disabled)
		log(L_INFO "Enabling protocol %s", p->name);
	      else if (!p->disabled && nc->disabled)
		log(L_INFO "Disabling protocol %s", p->name);

	      PD(p, "Restarting");
	      p->cf_new = nc;
	    }
	  else
	    {
	      if (!shutting_down)
		log(L_INFO "Removing protocol %s", p->name);
	      PD(p, "Unconfigured");
	      p->cf_new = NULL;
	    }
	  p->reconfiguring = 1;
	  config_add_obstacle(old);
	  proto_rethink_goal(p);
	}
    }

  WALK_LIST(nc, new->protos)
    if (!nc->proto)
      {
	if (old_config)		/* Not a first-time configuration */
	  log(L_INFO "Adding protocol %s", nc->name);
	proto_init(nc);
      }
  DBG("\tdone\n");

  DBG("Protocol start\n");
  WALK_LIST_DELSAFE(p, n, initial_proto_list)
    proto_rethink_goal(p);
}

static void
proto_rethink_goal(struct proto *p)
{
  struct protocol *q;

  if (p->reconfiguring && p->core_state == FS_HUNGRY && p->proto_state == PS_DOWN)
    {
      struct proto_config *nc = p->cf_new;
      DBG("%s has shut down for reconfiguration\n", p->name);
      config_del_obstacle(p->cf->global);
      rem_node(&p->n);
      rem_node(&p->glob_node);
      mb_free(p);
      if (!nc)
	return;
      p = proto_init(nc);
    }

  /* Determine what state we want to reach */
  if (p->disabled || p->reconfiguring)
    {
      p->core_goal = FS_HUNGRY;
      if (p->core_state == FS_HUNGRY && p->proto_state == PS_DOWN)
	return;
    }
  else
    {
      p->core_goal = FS_HAPPY;
      if (p->core_state == FS_HAPPY && p->proto_state == PS_UP)
	return;
    }

  q = p->proto;
  if (p->core_goal == FS_HAPPY)		/* Going up */
    {
      if (p->core_state == FS_HUNGRY && p->proto_state == PS_DOWN)
	{
	  DBG("Kicking %s up\n", p->name);
	  PD(p, "Starting");
	  proto_init_instance(p);
	  proto_notify_state(p, (q->start ? q->start(p) : PS_UP));
	}
    }
  else 					/* Going down */
    {
      if (p->proto_state == PS_START || p->proto_state == PS_UP)
	{
	  DBG("Kicking %s down\n", p->name);
	  PD(p, "Shutting down");
	  proto_notify_state(p, (q->shutdown ? q->shutdown(p) : PS_DOWN));
	}
    }
}

/**
 * protos_dump_all - dump status of all protocols
 *
 * This function dumps status of all existing protocol instances to the
 * debug output. It involves printing of general status information
 * such as protocol states, its position on the protocol lists
 * and also calling of a dump() hook of the protocol to print
 * the internals.
 */
void
protos_dump_all(void)
{
  struct proto *p;

  bdebug("Protocols:\n");

  WALK_LIST(p, active_proto_list)
    {
      bdebug("  protocol %s state %s/%s\n", p->name,
	    p_states[p->proto_state], c_states[p->core_state]);
      if (p->in_filter)
	bdebug("\tInput filter: %s\n", filter_name(p->in_filter));
      if (p->out_filter != FILTER_REJECT)
	bdebug("\tOutput filter: %s\n", filter_name(p->out_filter));
      if (p->disabled)
	bdebug("\tDISABLED\n");
      else if (p->proto->dump)
	p->proto->dump(p);
    }
  WALK_LIST(p, inactive_proto_list)
    bdebug("  inactive %s: state %s/%s\n", p->name, p_states[p->proto_state], c_states[p->core_state]);
  WALK_LIST(p, initial_proto_list)
    bdebug("  initial %s\n", p->name);
  WALK_LIST(p, flush_proto_list)
    bdebug("  flushing %s\n", p->name);
}

/**
 * proto_build - make a single protocol available
 * @p: the protocol
 *
 * After the platform specific initialization code uses protos_build()
 * to add all the standard protocols, it should call proto_build() for
 * all platform specific protocols to inform the core that they exist.
 */
void
proto_build(struct protocol *p)
{
  add_tail(&protocol_list, &p->n);
  if (p->attr_class)
    {
      ASSERT(!attr_class_to_protocol[p->attr_class]);
      attr_class_to_protocol[p->attr_class] = p;
    }
}

/**
 * protos_build - build a protocol list
 *
 * This function is called during BIRD startup to insert
 * all standard protocols to the global protocol list. Insertion
 * of platform specific protocols (such as the kernel syncer)
 * is in the domain of competence of the platform dependent
 * startup code.
 */
void
protos_build(void)
{
  init_list(&protocol_list);
  init_list(&proto_list);
  init_list(&active_proto_list);
  init_list(&inactive_proto_list);
  init_list(&initial_proto_list);
  init_list(&flush_proto_list);
  proto_build(&proto_device);
#ifdef CONFIG_RIP
  proto_build(&proto_rip);
#endif
#ifdef CONFIG_STATIC
  proto_build(&proto_static);
#endif
#ifdef CONFIG_OSPF
  proto_build(&proto_ospf);
#endif
#ifdef CONFIG_PIPE
  proto_build(&proto_pipe);
#endif
#ifdef CONFIG_BGP
  proto_build(&proto_bgp);
#endif
  proto_pool = rp_new(&root_pool, "Protocols");
  proto_flush_event = ev_new(proto_pool);
  proto_flush_event->hook = proto_flush_all;
}

static void
proto_fell_down(struct proto *p)
{
  DBG("Protocol %s down\n", p->name);

  if (p->stats.imp_routes != 0)
    log(L_ERR "Protocol %s is down but still has %d routes", p->name, p->stats.imp_routes);

  bzero(&p->stats, sizeof(struct proto_stats));
  rt_unlock_table(p->table);

#ifdef CONFIG_PIPE
  if (proto_is_pipe(p))
    rt_unlock_table(pipe_get_peer_table(p));
#endif

  proto_rethink_goal(p);
}

static void
proto_feed_more(void *P)
{
  struct proto *p = P;

  if (p->core_state != FS_FEEDING)
    return;

  DBG("Feeding protocol %s continued\n", p->name);
  if (rt_feed_baby(p))
    {
      p->core_state = FS_HAPPY;
      proto_relink(p);
      DBG("Protocol %s up and running\n", p->name);
    }
  else
    {
      p->attn->hook = proto_feed_more;
      ev_schedule(p->attn);		/* Will continue later... */
    }
}

static void
proto_feed_initial(void *P)
{
  struct proto *p = P;

  if (p->core_state != FS_FEEDING)
    return;

  DBG("Feeding protocol %s\n", p->name);
  proto_add_announce_hook(p, p->table);
  if_feed_baby(p);
  proto_feed_more(P);
}

static void
proto_schedule_flush(struct proto *p)
{
  /* Need to abort feeding */
  if (p->core_state == FS_FEEDING)
    rt_feed_baby_abort(p);

  DBG("%s: Scheduling flush\n", p->name);
  p->core_state = FS_FLUSHING;
  proto_relink(p);
  proto_flush_hooks(p);
  ev_schedule(proto_flush_event);
}

static void
proto_schedule_feed(struct proto *p, int initial)
{
  DBG("%s: Scheduling meal\n", p->name);
  p->core_state = FS_FEEDING;
  p->refeeding = !initial;

  /* Hack: reset exp_routes during refeed, and do not decrease it later */
  if (!initial)
    p->stats.exp_routes = 0;

  proto_relink(p);
  p->attn->hook = initial ? proto_feed_initial : proto_feed_more;
  ev_schedule(p->attn);
}

/**
 * proto_request_feeding - request feeding routes to the protocol
 * @p: given protocol 
 *
 * Sometimes it is needed to send again all routes to the
 * protocol. This is called feeding and can be requested by this
 * function. This would cause protocol core state transition
 * to FS_FEEDING (during feeding) and when completed, it will
 * switch back to FS_HAPPY. This function can be called even
 * when feeding is already running, in that case it is restarted.
 */
void
proto_request_feeding(struct proto *p)
{
  ASSERT(p->proto_state == PS_UP);

  /* If we are already feeding, we want to restart it */
  if (p->core_state == FS_FEEDING)
    {
      /* Unless feeding is in initial state */
      if (p->attn->hook == proto_feed_initial)
	return;

      rt_feed_baby_abort(p);
    }

  proto_schedule_feed(p, 0);
}

/**
 * proto_notify_state - notify core about protocol state change
 * @p: protocol the state of which has changed
 * @ps: the new status
 *
 * Whenever a state of a protocol changes due to some event internal
 * to the protocol (i.e., not inside a start() or shutdown() hook),
 * it should immediately notify the core about the change by calling
 * proto_notify_state() which will write the new state to the &proto
 * structure and take all the actions necessary to adapt to the new
 * state. State change to PS_DOWN immediately frees resources of protocol
 * and might execute start callback of protocol; therefore,
 * it should be used at tail positions of protocol callbacks.
 */
void
proto_notify_state(struct proto *p, unsigned ps)
{
  unsigned ops = p->proto_state;
  unsigned cs = p->core_state;

  DBG("%s reporting state transition %s/%s -> */%s\n", p->name, c_states[cs], p_states[ops], p_states[ps]);
  if (ops == ps)
    return;

  p->proto_state = ps;

  switch (ps)
    {
    case PS_DOWN:
      if ((cs == FS_FEEDING) || (cs == FS_HAPPY))
	proto_schedule_flush(p);

      neigh_prune(); // FIXME convert neighbors to resource?
      rfree(p->pool);
      p->pool = NULL;

      if (cs == FS_HUNGRY)		/* Shutdown finished */
	{
	  proto_fell_down(p);
	  return;			/* The protocol might have ceased to exist */
	}
      break;
    case PS_START:
      ASSERT(ops == PS_DOWN);
      ASSERT(cs == FS_HUNGRY);
      break;
    case PS_UP:
      ASSERT(ops == PS_DOWN || ops == PS_START);
      ASSERT(cs == FS_HUNGRY);
      proto_schedule_feed(p, 1);
      break;
    case PS_STOP:
      if ((cs == FS_FEEDING) || (cs == FS_HAPPY))
	proto_schedule_flush(p);
      break;
    default:
      bug("Invalid state transition for %s from %s/%s to */%s", p->name, c_states[cs], p_states[ops], p_states[ps]);
    }
}

extern struct protocol proto_unix_iface;

static void
proto_flush_all(void *unused UNUSED)
{
  struct proto *p;

  rt_prune_all();
  while ((p = HEAD(flush_proto_list))->n.next)
    {
      /* This will flush interfaces in the same manner
	 like rt_prune_all() flushes routes */
      if (p->proto == &proto_unix_iface)
	if_flush_ifaces(p);

      DBG("Flushing protocol %s\n", p->name);
      p->core_state = FS_HUNGRY;
      proto_relink(p);
      if (p->proto_state == PS_DOWN)
	proto_fell_down(p);
    }
}

/*
 *  CLI Commands
 */

static char *
proto_state_name(struct proto *p)
{
#define P(x,y) ((x << 4) | y)
  switch (P(p->proto_state, p->core_state))
    {
    case P(PS_DOWN, FS_HUNGRY):		return "down";
    case P(PS_START, FS_HUNGRY):	return "start";
    case P(PS_UP, FS_HUNGRY):
    case P(PS_UP, FS_FEEDING):		return "feed";
    case P(PS_STOP, FS_HUNGRY):		return "stop";
    case P(PS_UP, FS_HAPPY):		return "up";
    case P(PS_STOP, FS_FLUSHING):
    case P(PS_DOWN, FS_FLUSHING):	return "flush";
    default:      			return "???";
    }
#undef P
}

static void
proto_do_show_stats(struct proto *p)
{
  struct proto_stats *s = &p->stats;
  cli_msg(-1006, "  Routes:         %u imported, %u exported, %u preferred", 
	  s->imp_routes, s->exp_routes, s->pref_routes);
  cli_msg(-1006, "  Route change stats:     received   rejected   filtered    ignored   accepted");
  cli_msg(-1006, "    Import updates:     %10u %10u %10u %10u %10u",
	  s->imp_updates_received, s->imp_updates_invalid,
	  s->imp_updates_filtered, s->imp_updates_ignored,
	  s->imp_updates_accepted);
  cli_msg(-1006, "    Import withdraws:   %10u %10u        --- %10u %10u",
	  s->imp_withdraws_received, s->imp_withdraws_invalid,
	  s->imp_withdraws_ignored, s->imp_withdraws_accepted);
  cli_msg(-1006, "    Export updates:     %10u %10u %10u        --- %10u",
	  s->exp_updates_received, s->exp_updates_rejected,
	  s->exp_updates_filtered, s->exp_updates_accepted);
  cli_msg(-1006, "    Export withdraws:   %10u        ---        ---        --- %10u",
	  s->exp_withdraws_received, s->exp_withdraws_accepted);
}

#ifdef CONFIG_PIPE
static void
proto_do_show_pipe_stats(struct proto *p)
{
  struct proto_stats *s1 = &p->stats;
  struct proto_stats *s2 = pipe_get_peer_stats(p);

  /*
   * Pipe stats (as anything related to pipes) are a bit tricky. There
   * are two sets of stats - s1 for routes going from the primary
   * routing table to the secondary routing table ('exported' from the
   * user point of view) and s2 for routes going in the other
   * direction ('imported' from the user point of view).
   *
   * Each route going through a pipe is, technically, first exported
   * to the pipe and then imported from that pipe and such operations
   * are counted in one set of stats according to the direction of the
   * route propagation. Filtering is done just in the first part
   * (export). Therefore, we compose stats for one directon for one
   * user direction from both import and export stats, skipping
   * immediate and irrelevant steps (exp_updates_accepted,
   * imp_updates_received, imp_updates_filtered, ...)
   */

  cli_msg(-1006, "  Routes:         %u imported, %u exported", 
	  s2->imp_routes, s1->imp_routes);
  cli_msg(-1006, "  Route change stats:     received   rejected   filtered    ignored   accepted");
  cli_msg(-1006, "    Import updates:     %10u %10u %10u %10u %10u",
	  s2->exp_updates_received, s2->exp_updates_rejected + s2->imp_updates_invalid,
	  s2->exp_updates_filtered, s2->imp_updates_ignored, s2->imp_updates_accepted);
  cli_msg(-1006, "    Import withdraws:   %10u %10u        --- %10u %10u",
	  s2->exp_withdraws_received, s2->imp_withdraws_invalid,
	  s2->imp_withdraws_ignored, s2->imp_withdraws_accepted);
  cli_msg(-1006, "    Export updates:     %10u %10u %10u %10u %10u",
	  s1->exp_updates_received, s1->exp_updates_rejected + s1->imp_updates_invalid,
	  s1->exp_updates_filtered, s1->imp_updates_ignored, s1->imp_updates_accepted);
  cli_msg(-1006, "    Export withdraws:   %10u %10u        --- %10u %10u",
	  s1->exp_withdraws_received, s1->imp_withdraws_invalid,
	  s1->imp_withdraws_ignored, s1->imp_withdraws_accepted);
}
#endif

void
proto_cmd_show(struct proto *p, unsigned int verbose, int cnt)
{
  byte buf[256], tbuf[TM_DATETIME_BUFFER_SIZE];

  /* First protocol - show header */
  if (!cnt)
    cli_msg(-2002, "name     proto    table    state  since       info");

  buf[0] = 0;
  if (p->proto->get_status)
    p->proto->get_status(p, buf);
  tm_format_datetime(tbuf, &config->tf_proto, p->last_state_change);
  cli_msg(-1002, "%-8s %-8s %-8s %-5s  %-10s  %s",
	  p->name,
	  p->proto->name,
	  p->table->name,
	  proto_state_name(p),
	  tbuf,
	  buf);
  if (verbose)
    {
      if (p->cf->dsc)
	cli_msg(-1006, "  Description:    %s", p->cf->dsc);
      if (p->cf->router_id)
	cli_msg(-1006, "  Router ID:      %R", p->cf->router_id);
      cli_msg(-1006, "  Preference:     %d", p->preference);
      cli_msg(-1006, "  Input filter:   %s", filter_name(p->in_filter));
      cli_msg(-1006, "  Output filter:  %s", filter_name(p->out_filter));

      if (p->proto_state != PS_DOWN)
	{
#ifdef CONFIG_PIPE
	  if (proto_is_pipe(p))
	    proto_do_show_pipe_stats(p);
	  else
#endif
	    proto_do_show_stats(p);
	}

      if (p->proto->show_proto_info)
	p->proto->show_proto_info(p);

      cli_msg(-1006, "");
    }
}

void
proto_cmd_disable(struct proto *p, unsigned int arg UNUSED, int cnt UNUSED)
{
  if (p->disabled)
    {
      cli_msg(-8, "%s: already disabled", p->name);
      return;
    }

  log(L_INFO "Disabling protocol %s", p->name);
  p->disabled = 1;
  proto_rethink_goal(p);
  cli_msg(-9, "%s: disabled", p->name);
}

void
proto_cmd_enable(struct proto *p, unsigned int arg UNUSED, int cnt UNUSED)
{
  if (!p->disabled)
    {
      cli_msg(-10, "%s: already enabled", p->name);
      return;
    }

  log(L_INFO "Enabling protocol %s", p->name);
  p->disabled = 0;
  proto_rethink_goal(p);
  cli_msg(-11, "%s: enabled", p->name);
}

void
proto_cmd_restart(struct proto *p, unsigned int arg UNUSED, int cnt UNUSED)
{
  if (p->disabled)
    {
      cli_msg(-8, "%s: already disabled", p->name);
      return;
    }

  log(L_INFO "Restarting protocol %s", p->name);
  p->disabled = 1;
  proto_rethink_goal(p);
  p->disabled = 0;
  proto_rethink_goal(p);
  cli_msg(-12, "%s: restarted", p->name);
}

void
proto_cmd_reload(struct proto *p, unsigned int dir, int cnt UNUSED)
{
  if (p->disabled)
    {
      cli_msg(-8, "%s: already disabled", p->name);
      return;
    }

  /* If the protocol in not UP, it has no routes */
  if (p->proto_state != PS_UP)
    return;

  log(L_INFO "Reloading protocol %s", p->name);

  /* re-importing routes */
  if (dir != CMD_RELOAD_OUT)
    if (! (p->reload_routes && p->reload_routes(p)))
      {
	cli_msg(-8006, "%s: reload failed", p->name);
	return;
      }
		 
  /* re-exporting routes */
  if (dir != CMD_RELOAD_IN)
    proto_request_feeding(p);

  cli_msg(-15, "%s: reloading", p->name);
}

void
proto_cmd_debug(struct proto *p, unsigned int mask, int cnt UNUSED)
{
  p->debug = mask;
}

void
proto_cmd_mrtdump(struct proto *p, unsigned int mask, int cnt UNUSED)
{
  p->mrtdump = mask;
}

static void
proto_apply_cmd_symbol(struct symbol *s, void (* cmd)(struct proto *, unsigned int, int), unsigned int arg)
{
  if (s->class != SYM_PROTO)
    {
      cli_msg(9002, "%s is not a protocol", s->name);
      return;
    }

  cmd(((struct proto_config *)s->def)->proto, arg, 0);
  cli_msg(0, "");
}

static void
proto_apply_cmd_patt(char *patt, void (* cmd)(struct proto *, unsigned int, int), unsigned int arg)
{
  int cnt = 0;

  node *nn;
  WALK_LIST(nn, proto_list)
    {
      struct proto *p = SKIP_BACK(struct proto, glob_node, nn);

      if (!patt || patmatch(patt, p->name))
	cmd(p, arg, cnt++);
    }

  if (!cnt)
    cli_msg(8003, "No protocols match");
  else
    cli_msg(0, "");
}

void
proto_apply_cmd(struct proto_spec ps, void (* cmd)(struct proto *, unsigned int, int),
		int restricted, unsigned int arg)
{
  if (restricted && cli_access_restricted())
    return;

  if (ps.patt)
    proto_apply_cmd_patt(ps.ptr, cmd, arg);
  else
    proto_apply_cmd_symbol(ps.ptr, cmd, arg);
}

struct proto *
proto_get_named(struct symbol *sym, struct protocol *pr)
{
  struct proto *p, *q;

  if (sym)
    {
      if (sym->class != SYM_PROTO)
	cf_error("%s: Not a protocol", sym->name);
      p = ((struct proto_config *)sym->def)->proto;
      if (!p || p->proto != pr)
	cf_error("%s: Not a %s protocol", sym->name, pr->name);
    }
  else
    {
      p = NULL;
      WALK_LIST(q, active_proto_list)
	if (q->proto == pr)
	  {
	    if (p)
	      cf_error("There are multiple %s protocols running", pr->name);
	    p = q;
	  }
      if (!p)
	cf_error("There is no %s protocol running", pr->name);
    }
  return p;
}
