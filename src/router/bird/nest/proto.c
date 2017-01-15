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
static struct proto *initial_device_proto;

static event *proto_flush_event;
static timer *proto_shutdown_timer;
static timer *gr_wait_timer;

#define GRS_NONE	0
#define GRS_INIT	1
#define GRS_ACTIVE	2
#define GRS_DONE	3

static int graceful_restart_state;
static u32 graceful_restart_locks;

static char *p_states[] = { "DOWN", "START", "UP", "STOP" };
static char *c_states[] = { "HUNGRY", "???", "HAPPY", "FLUSHING" };

static void proto_flush_loop(void *);
static void proto_shutdown_loop(struct timer *);
static void proto_rethink_goal(struct proto *p);
static void proto_want_export_up(struct proto *p);
static void proto_fell_down(struct proto *p);
static char *proto_state_name(struct proto *p);

static void
proto_relink(struct proto *p)
{
  list *l = NULL;

  switch (p->core_state)
    {
    case FS_HUNGRY:
      l = &inactive_proto_list;
      break;
    case FS_HAPPY:
      l = &active_proto_list;
      break;
    case FS_FLUSHING:
      l = &flush_proto_list;
      break;
    default:
      ASSERT(0);
    }

  rem_node(&p->n);
  add_tail(l, &p->n);
}

static void
proto_log_state_change(struct proto *p)
{
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

  if (graceful_restart_state == GRS_INIT)
    p->gr_recovery = 1;

  if (! p->proto->multitable)
    rt_lock_table(p->table);
}

extern pool *rt_table_pool;
/**
 * proto_add_announce_hook - connect protocol to a routing table
 * @p: protocol instance
 * @t: routing table to connect to
 * @stats: per-table protocol statistics
 *
 * This function creates a connection between the protocol instance @p and the
 * routing table @t, making the protocol hear all changes in the table.
 *
 * The announce hook is linked in the protocol ahook list. Announce hooks are
 * allocated from the routing table resource pool and when protocol accepts
 * routes also in the table ahook list. The are linked to the table ahook list
 * and unlinked from it depending on export_state (in proto_want_export_up() and
 * proto_want_export_down()) and they are automatically freed after the protocol
 * is flushed (in proto_fell_down()).
 *
 * Unless you want to listen to multiple routing tables (as the Pipe protocol
 * does), you needn't to worry about this function since the connection to the
 * protocol's primary routing table is initialized automatically by the core
 * code.
 */
struct announce_hook *
proto_add_announce_hook(struct proto *p, struct rtable *t, struct proto_stats *stats)
{
  struct announce_hook *h;

  DBG("Connecting protocol %s to table %s\n", p->name, t->name);
  PD(p, "Connected to table %s", t->name);

  h = mb_allocz(rt_table_pool, sizeof(struct announce_hook));
  h->table = t;
  h->proto = p;
  h->stats = stats;

  h->next = p->ahooks;
  p->ahooks = h;

  if (p->rt_notify && (p->export_state != ES_DOWN))
    add_tail(&t->hooks, &h->n);
  return h;
}

/**
 * proto_find_announce_hook - find announce hooks
 * @p: protocol instance
 * @t: routing table
 *
 * Returns pointer to announce hook or NULL
 */
struct announce_hook *
proto_find_announce_hook(struct proto *p, struct rtable *t)
{
  struct announce_hook *a;

  for (a = p->ahooks; a; a = a->next)
    if (a->table == t)
      return a;

  return NULL;
}

static void
proto_link_ahooks(struct proto *p)
{
  struct announce_hook *h;

  if (p->rt_notify)
    for(h=p->ahooks; h; h=h->next)
      add_tail(&h->table->hooks, &h->n);
}

static void
proto_unlink_ahooks(struct proto *p)
{
  struct announce_hook *h;

  if (p->rt_notify)
    for(h=p->ahooks; h; h=h->next)
      rem_node(&h->n);
}

static void
proto_free_ahooks(struct proto *p)
{
  struct announce_hook *h, *hn;

  for(h = p->ahooks; h; h = hn)
  {
    hn = h->next;
    mb_free(h);
  }

  p->ahooks = NULL;
  p->main_ahook = NULL;
}


/**
 * proto_config_new - create a new protocol configuration
 * @pr: protocol the configuration will belong to
 * @class: SYM_PROTO or SYM_TEMPLATE
 *
 * Whenever the configuration file says that a new instance
 * of a routing protocol should be created, the parser calls
 * proto_config_new() to create a configuration entry for this
 * instance (a structure staring with the &proto_config header
 * containing all the generic items followed by protocol-specific
 * ones). Also, the configuration entry gets added to the list
 * of protocol instances kept in the configuration.
 *
 * The function is also used to create protocol templates (when class
 * SYM_TEMPLATE is specified), the only difference is that templates
 * are not added to the list of protocol instances and therefore not
 * initialized during protos_commit()).
 */
void *
proto_config_new(struct protocol *pr, int class)
{
  struct proto_config *c = cfg_allocz(pr->config_size);

  if (class == SYM_PROTO)
    add_tail(&new_config->protos, &c->n);
  c->global = new_config;
  c->protocol = pr;
  c->name = pr->name;
  c->preference = pr->preference;
  c->class = class;
  c->out_filter = FILTER_REJECT;
  c->table = c->global->master_rtc;
  c->debug = new_config->proto_default_debug;
  c->mrtdump = new_config->proto_default_mrtdump;
  return c;
}

/**
 * proto_copy_config - copy a protocol configuration
 * @dest: destination protocol configuration
 * @src: source protocol configuration
 *
 * Whenever a new instance of a routing protocol is created from the
 * template, proto_copy_config() is called to copy a content of
 * the source protocol configuration to the new protocol configuration.
 * Name, class and a node in protos list of @dest are kept intact.
 * copy_config() protocol hook is used to copy protocol-specific data.
 */
void
proto_copy_config(struct proto_config *dest, struct proto_config *src)
{
  node old_node;
  int old_class;
  char *old_name;

  if (dest->protocol != src->protocol)
    cf_error("Can't copy configuration from a different protocol type");

  if (dest->protocol->copy_config == NULL)
    cf_error("Inheriting configuration for %s is not supported", src->protocol->name);

  DBG("Copying configuration from %s to %s\n", src->name, dest->name);

  /* 
   * Copy struct proto_config here. Keep original node, class and name.
   * protocol-specific config copy is handled by protocol copy_config() hook
   */

  old_node = dest->n;
  old_class = dest->class;
  old_name = dest->name;

  memcpy(dest, src, sizeof(struct proto_config));

  dest->n = old_node;
  dest->class = old_class;
  dest->name = old_name;

  dest->protocol->copy_config(dest, src);
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
 * instances specified in configuration @c. The hooks are not
 * called for protocol templates.
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

extern struct protocol proto_unix_iface;

static struct proto *
proto_init(struct proto_config *c)
{
  struct protocol *p = c->protocol;
  struct proto *q = p->init(c);

  q->proto_state = PS_DOWN;
  q->core_state = FS_HUNGRY;
  q->export_state = ES_DOWN;
  q->last_state_change = now;

  add_tail(&initial_proto_list, &q->n);

  if (p == &proto_unix_iface)
    initial_device_proto = q;

  add_tail(&proto_list, &q->glob_node);
  PD(q, "Initializing%s", q->disabled ? " [disabled]" : "");
  return q;
}

int proto_reconfig_type;  /* Hack to propagate type info to pipe reconfigure hook */

static int
proto_reconfigure(struct proto *p, struct proto_config *oc, struct proto_config *nc, int type)
{
  /* If the protocol is DOWN, we just restart it */
  if (p->proto_state == PS_DOWN)
    return 0;

  /* If there is a too big change in core attributes, ... */
  if ((nc->protocol != oc->protocol) ||
      (nc->disabled != p->disabled) ||
      (nc->table->table != oc->table->table))
    return 0;

  p->debug = nc->debug;
  p->mrtdump = nc->mrtdump;
  proto_reconfig_type = type;

  /* Execute protocol specific reconfigure hook */
  if (! (p->proto->reconfigure && p->proto->reconfigure(p, nc)))
    return 0;

  DBG("\t%s: same\n", oc->name);
  PD(p, "Reconfigured");
  p->cf = nc;
  p->name = nc->name;
  p->preference = nc->preference;


  /* Multitable protocols handle rest in their reconfigure hooks */
  if (p->proto->multitable)
    return 1;

  /* Update filters and limits in the main announce hook
     Note that this also resets limit state */
  if (p->main_ahook)
    {  
      struct announce_hook *ah = p->main_ahook;
      ah->in_filter = nc->in_filter;
      ah->out_filter = nc->out_filter;
      ah->rx_limit = nc->rx_limit;
      ah->in_limit = nc->in_limit;
      ah->out_limit = nc->out_limit;
      ah->in_keep_filtered = nc->in_keep_filtered;
      proto_verify_limits(ah);
    }

  /* Update routes when filters changed. If the protocol in not UP,
     it has no routes and we can ignore such changes */
  if ((p->proto_state != PS_UP) || (type == RECONFIG_SOFT))
    return 1;

  int import_changed = ! filter_same(nc->in_filter, oc->in_filter);
  int export_changed = ! filter_same(nc->out_filter, oc->out_filter);

  /* We treat a change in preferences by reimporting routes */
  if (nc->preference != oc->preference)
    import_changed = 1;

  if (import_changed || export_changed)
    log(L_INFO "Reloading protocol %s", p->name);

  /* If import filter changed, call reload hook */
  if (import_changed && ! (p->reload_routes && p->reload_routes(p)))
    {
      /* Now, the protocol is reconfigured. But route reload failed
	 and we have to do regular protocol restart. */
      log(L_INFO "Restarting protocol %s", p->name);
      p->disabled = 1;
      p->down_code = PDC_CF_RESTART;
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
  struct symbol *sym;

  DBG("protos_commit:\n");
  if (old)
    {
      WALK_LIST(oc, old->protos)
	{
	  p = oc->proto;
	  sym = cf_find_symbol(new, oc->name);
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

	      p->down_code = nc->disabled ? PDC_CF_DISABLE : PDC_CF_RESTART;
	      p->cf_new = nc;
	    }
	  else if (!new->shutdown)
	    {
	      log(L_INFO "Removing protocol %s", p->name);
	      p->down_code = PDC_CF_REMOVE;
	      p->cf_new = NULL;
	    }
	  else /* global shutdown */
	    {
	      p->down_code = PDC_CMD_SHUTDOWN;
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
	if (old)		/* Not a first-time configuration */
	  log(L_INFO "Adding protocol %s", nc->name);
	proto_init(nc);
      }
  DBG("\tdone\n");

  DBG("Protocol start\n");

  /* Start device protocol first */
  if (initial_device_proto)
  {
    proto_rethink_goal(initial_device_proto);
    initial_device_proto = NULL;
  }

  /* Determine router ID for the first time - it has to be here and not in
     global_commit() because it is postponed after start of device protocol */
  if (!config->router_id)
    {
      config->router_id = if_choose_router_id(config->router_id_from, 0);
      if (!config->router_id)
	die("Cannot determine router ID, please configure it manually");
    }

  /* Start all other protocols */
  WALK_LIST_DELSAFE(p, n, initial_proto_list)
    proto_rethink_goal(p);
}

static void
proto_rethink_goal(struct proto *p)
{
  struct protocol *q;
  byte goal;

  if (p->reconfiguring && p->core_state == FS_HUNGRY && p->proto_state == PS_DOWN)
    {
      struct proto_config *nc = p->cf_new;
      DBG("%s has shut down for reconfiguration\n", p->name);
      p->cf->proto = NULL;
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
    goal = PS_DOWN;
  else
    goal = PS_UP;

  q = p->proto;
  if (goal == PS_UP) 			/* Going up */
    {
      if (p->proto_state == PS_DOWN && p->core_state == FS_HUNGRY)
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
 * DOC: Graceful restart recovery
 *
 * Graceful restart of a router is a process when the routing plane (e.g. BIRD)
 * restarts but both the forwarding plane (e.g kernel routing table) and routing
 * neighbors keep proper routes, and therefore uninterrupted packet forwarding
 * is maintained.
 *
 * BIRD implements graceful restart recovery by deferring export of routes to
 * protocols until routing tables are refilled with the expected content. After
 * start, protocols generate routes as usual, but routes are not propagated to
 * them, until protocols report that they generated all routes. After that,
 * graceful restart recovery is finished and the export (and the initial feed)
 * to protocols is enabled.
 *
 * When graceful restart recovery need is detected during initialization, then
 * enabled protocols are marked with @gr_recovery flag before start. Such
 * protocols then decide how to proceed with graceful restart, participation is
 * voluntary. Protocols could lock the recovery by proto_graceful_restart_lock()
 * (stored in @gr_lock flag), which means that they want to postpone the end of
 * the recovery until they converge and then unlock it. They also could set
 * @gr_wait before advancing to %PS_UP, which means that the core should defer
 * route export to that protocol until the end of the recovery. This should be
 * done by protocols that expect their neigbors to keep the proper routes
 * (kernel table, BGP sessions with BGP graceful restart capability).
 *
 * The graceful restart recovery is finished when either all graceful restart
 * locks are unlocked or when graceful restart wait timer fires.
 *
 */

static void graceful_restart_done(struct timer *t);

/**
 * graceful_restart_recovery - request initial graceful restart recovery
 *
 * Called by the platform initialization code if the need for recovery
 * after graceful restart is detected during boot. Have to be called
 * before protos_commit().
 */
void
graceful_restart_recovery(void)
{
  graceful_restart_state = GRS_INIT;
}

/**
 * graceful_restart_init - initialize graceful restart
 *
 * When graceful restart recovery was requested, the function starts an active
 * phase of the recovery and initializes graceful restart wait timer. The
 * function have to be called after protos_commit().
 */
void
graceful_restart_init(void)
{
  if (!graceful_restart_state)
    return;

  log(L_INFO "Graceful restart started");

  if (!graceful_restart_locks)
    {
      graceful_restart_done(NULL);
      return;
    }

  graceful_restart_state = GRS_ACTIVE;
  gr_wait_timer = tm_new(proto_pool);
  gr_wait_timer->hook = graceful_restart_done;
  tm_start(gr_wait_timer, config->gr_wait);
}

/**
 * graceful_restart_done - finalize graceful restart
 * @t: unused
 *
 * When there are no locks on graceful restart, the functions finalizes the
 * graceful restart recovery. Protocols postponing route export until the end of
 * the recovery are awakened and the export to them is enabled. All other
 * related state is cleared. The function is also called when the graceful
 * restart wait timer fires (but there are still some locks).
 */
static void
graceful_restart_done(struct timer *t UNUSED)
{
  struct proto *p;
  node *n;

  log(L_INFO "Graceful restart done");
  graceful_restart_state = GRS_DONE;

  WALK_LIST2(p, n, proto_list, glob_node)
    {
      if (!p->gr_recovery)
	continue;

      /* Resume postponed export of routes */
      if ((p->proto_state == PS_UP) && p->gr_wait)
      {
	proto_want_export_up(p);
	proto_log_state_change(p);
      }

      /* Cleanup */
      p->gr_recovery = 0;
      p->gr_wait = 0;
      p->gr_lock = 0;
    }

  graceful_restart_locks = 0;
}

void
graceful_restart_show_status(void)
{
  if (graceful_restart_state != GRS_ACTIVE)
    return;

  cli_msg(-24, "Graceful restart recovery in progress");
  cli_msg(-24, "  Waiting for %d protocols to recover", graceful_restart_locks);
  cli_msg(-24, "  Wait timer is %d/%d", tm_remains(gr_wait_timer), config->gr_wait);
}

/**
 * proto_graceful_restart_lock - lock graceful restart by protocol
 * @p: protocol instance
 *
 * This function allows a protocol to postpone the end of graceful restart
 * recovery until it converges. The lock is removed when the protocol calls
 * proto_graceful_restart_unlock() or when the protocol is stopped.
 *
 * The function have to be called during the initial phase of graceful restart
 * recovery and only for protocols that are part of graceful restart (i.e. their
 * @gr_recovery is set), which means it should be called from protocol start
 * hooks.
 */
void
proto_graceful_restart_lock(struct proto *p)
{
  ASSERT(graceful_restart_state == GRS_INIT);
  ASSERT(p->gr_recovery);

  if (p->gr_lock)
    return;

  p->gr_lock = 1;
  graceful_restart_locks++;
}

/**
 * proto_graceful_restart_unlock - unlock graceful restart by protocol
 * @p: protocol instance
 *
 * This function unlocks a lock from proto_graceful_restart_lock(). It is also
 * automatically called when the lock holding protocol went down.
 */
void
proto_graceful_restart_unlock(struct proto *p)
{
  if (!p->gr_lock)
    return;

  p->gr_lock = 0;
  graceful_restart_locks--;

  if ((graceful_restart_state == GRS_ACTIVE) && !graceful_restart_locks)
    tm_start(gr_wait_timer, 0);
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
  struct announce_hook *a;

  debug("Protocols:\n");

  WALK_LIST(p, active_proto_list)
    {
      debug("  protocol %s state %s/%s\n", p->name,
	    p_states[p->proto_state], c_states[p->core_state]);
      for (a = p->ahooks; a; a = a->next)
	{
	  debug("\tTABLE %s\n", a->table->name);
	  if (a->in_filter)
	    debug("\tInput filter: %s\n", filter_name(a->in_filter));
	  if (a->out_filter != FILTER_REJECT)
	    debug("\tOutput filter: %s\n", filter_name(a->out_filter));
	}
      if (p->disabled)
	debug("\tDISABLED\n");
      else if (p->proto->dump)
	p->proto->dump(p);
    }
  WALK_LIST(p, inactive_proto_list)
    debug("  inactive %s: state %s/%s\n", p->name, p_states[p->proto_state], c_states[p->core_state]);
  WALK_LIST(p, initial_proto_list)
    debug("  initial %s\n", p->name);
  WALK_LIST(p, flush_proto_list)
    debug("  flushing %s\n", p->name);
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

/* FIXME: convert this call to some protocol hook */
extern void bfd_init_all(void);

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
#ifdef CONFIG_RADV
  proto_build(&proto_radv);
#endif
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
#ifdef CONFIG_BFD
  proto_build(&proto_bfd);
  bfd_init_all();
#endif
#ifdef CONFIG_BABEL
  proto_build(&proto_babel);
#endif

  proto_pool = rp_new(&root_pool, "Protocols");
  proto_flush_event = ev_new(proto_pool);
  proto_flush_event->hook = proto_flush_loop;
  proto_shutdown_timer = tm_new(proto_pool);
  proto_shutdown_timer->hook = proto_shutdown_loop;
}

static void
proto_feed_more(void *P)
{
  struct proto *p = P;

  if (p->export_state != ES_FEEDING)
    return;

  DBG("Feeding protocol %s continued\n", p->name);
  if (rt_feed_baby(p))
    {
      DBG("Feeding protocol %s finished\n", p->name);
      p->export_state = ES_READY;
      proto_log_state_change(p);

      if (p->feed_end)
	p->feed_end(p);
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

  if (p->export_state != ES_FEEDING)
    return;

  DBG("Feeding protocol %s\n", p->name);

  if_feed_baby(p);
  proto_feed_more(P);
}

static void
proto_schedule_feed(struct proto *p, int initial)
{
  DBG("%s: Scheduling meal\n", p->name);

  p->export_state = ES_FEEDING;
  p->refeeding = !initial;

  p->attn->hook = initial ? proto_feed_initial : proto_feed_more;
  ev_schedule(p->attn);

  if (p->feed_begin)
    p->feed_begin(p, initial);
}

/*
 * Flushing loop is responsible for flushing routes and protocols
 * after they went down. It runs in proto_flush_event. At the start of
 * one round, protocols waiting to flush are marked in
 * proto_schedule_flush_loop(). At the end of the round (when routing
 * table flush is complete), marked protocols are flushed and a next
 * round may start.
 */

static int flush_loop_state;	/* 1 -> running */

static void
proto_schedule_flush_loop(void)
{
  struct proto *p;
  struct announce_hook *h;

  if (flush_loop_state)
    return;
  flush_loop_state = 1;

  WALK_LIST(p, flush_proto_list)
  {
    p->flushing = 1;
    for (h=p->ahooks; h; h=h->next)
      rt_mark_for_prune(h->table);
  }

  ev_schedule(proto_flush_event);
}

static void
proto_flush_loop(void *unused UNUSED)
{
  struct proto *p;

  if (! rt_prune_loop())
    {
      /* Rtable pruning is not finished */
      ev_schedule(proto_flush_event);
      return;
    }

  rt_prune_sources();

 again:
  WALK_LIST(p, flush_proto_list)
    if (p->flushing)
      {
	/* This will flush interfaces in the same manner
	   like rt_prune_all() flushes routes */
	if (p->proto == &proto_unix_iface)
	  if_flush_ifaces(p);

	DBG("Flushing protocol %s\n", p->name);
	p->flushing = 0;
	p->core_state = FS_HUNGRY;
	proto_relink(p);
	proto_log_state_change(p);
	if (p->proto_state == PS_DOWN)
	  proto_fell_down(p);
	goto again;
      }

  /* This round finished, perhaps there will be another one */
  flush_loop_state = 0;
  if (!EMPTY_LIST(flush_proto_list))
    proto_schedule_flush_loop();
}


/* Temporary hack to propagate restart to BGP */
int proto_restart;

static void
proto_shutdown_loop(struct timer *t UNUSED)
{
  struct proto *p, *p_next;

  WALK_LIST_DELSAFE(p, p_next, active_proto_list)
    if (p->down_sched)
      {
	proto_restart = (p->down_sched == PDS_RESTART);

	p->disabled = 1;
	proto_rethink_goal(p);
	if (proto_restart)
	  {
	    p->disabled = 0;
	    proto_rethink_goal(p);
	  }
      }
}

static inline void
proto_schedule_down(struct proto *p, byte restart, byte code)
{
  /* Does not work for other states (even PS_START) */
  ASSERT(p->proto_state == PS_UP);

  /* Scheduled restart may change to shutdown, but not otherwise */
  if (p->down_sched == PDS_DISABLE)
    return;

  p->down_sched = restart ? PDS_RESTART : PDS_DISABLE;
  p->down_code = code;
  tm_start_max(proto_shutdown_timer, restart ? 2 : 0);
}


/**
 * proto_request_feeding - request feeding routes to the protocol
 * @p: given protocol 
 *
 * Sometimes it is needed to send again all routes to the
 * protocol. This is called feeding and can be requested by this
 * function. This would cause protocol export state transition
 * to ES_FEEDING (during feeding) and when completed, it will
 * switch back to ES_READY. This function can be called even
 * when feeding is already running, in that case it is restarted.
 */
void
proto_request_feeding(struct proto *p)
{
  ASSERT(p->proto_state == PS_UP);

  /* Do nothing if we are still waiting for feeding */
  if (p->export_state == ES_DOWN)
    return;

  /* If we are already feeding, we want to restart it */
  if (p->export_state == ES_FEEDING)
    {
      /* Unless feeding is in initial state */
      if (p->attn->hook == proto_feed_initial)
	return;

      rt_feed_baby_abort(p);
    }

  /* FIXME: This should be changed for better support of multitable protos */
  struct announce_hook *ah;
  for (ah = p->ahooks; ah; ah = ah->next)
    proto_reset_limit(ah->out_limit);

  /* Hack: reset exp_routes during refeed, and do not decrease it later */
  p->stats.exp_routes = 0;

  proto_schedule_feed(p, 0);
  proto_log_state_change(p);
}

static const char *
proto_limit_name(struct proto_limit *l)
{
  const char *actions[] = {
    [PLA_WARN] = "warn",
    [PLA_BLOCK] = "block",
    [PLA_RESTART] = "restart",
    [PLA_DISABLE] = "disable",
  };

  return actions[l->action];
}

/**
 * proto_notify_limit: notify about limit hit and take appropriate action
 * @ah: announce hook
 * @l: limit being hit
 * @dir: limit direction (PLD_*)
 * @rt_count: the number of routes 
 *
 * The function is called by the route processing core when limit @l
 * is breached. It activates the limit and tooks appropriate action
 * according to @l->action.
 */
void
proto_notify_limit(struct announce_hook *ah, struct proto_limit *l, int dir, u32 rt_count)
{
  const char *dir_name[PLD_MAX] = { "receive", "import" , "export" };
  const byte dir_down[PLD_MAX] = { PDC_RX_LIMIT_HIT, PDC_IN_LIMIT_HIT, PDC_OUT_LIMIT_HIT };
  struct proto *p = ah->proto;

  if (l->state == PLS_BLOCKED)
    return;

  /* For warning action, we want the log message every time we hit the limit */
  if (!l->state || ((l->action == PLA_WARN) && (rt_count == l->limit)))
    log(L_WARN "Protocol %s hits route %s limit (%d), action: %s",
	p->name, dir_name[dir], l->limit, proto_limit_name(l));

  switch (l->action)
    {
    case PLA_WARN:
      l->state = PLS_ACTIVE;
      break;

    case PLA_BLOCK:
      l->state = PLS_BLOCKED;
      break;

    case PLA_RESTART:
    case PLA_DISABLE:
      l->state = PLS_BLOCKED;
      if (p->proto_state == PS_UP)
	proto_schedule_down(p, l->action == PLA_RESTART, dir_down[dir]);
      break;
    }
}

void
proto_verify_limits(struct announce_hook *ah)
{
  struct proto_limit *l;
  struct proto_stats *stats = ah->stats;
  u32 all_routes = stats->imp_routes + stats->filt_routes;

  l = ah->rx_limit;
  if (l && (all_routes > l->limit))
    proto_notify_limit(ah, l, PLD_RX, all_routes);

  l = ah->in_limit;
  if (l && (stats->imp_routes > l->limit))
    proto_notify_limit(ah, l, PLD_IN, stats->imp_routes);

  l = ah->out_limit;
  if (l && (stats->exp_routes > l->limit))
    proto_notify_limit(ah, l, PLD_OUT, stats->exp_routes);
}


static void
proto_want_core_up(struct proto *p)
{
  ASSERT(p->core_state == FS_HUNGRY);

  if (!p->proto->multitable)
    {
      p->main_source = rt_get_source(p, 0);
      rt_lock_source(p->main_source);

      /* Connect protocol to routing table */
      p->main_ahook = proto_add_announce_hook(p, p->table, &p->stats);
      p->main_ahook->in_filter = p->cf->in_filter;
      p->main_ahook->out_filter = p->cf->out_filter;
      p->main_ahook->rx_limit = p->cf->rx_limit;
      p->main_ahook->in_limit = p->cf->in_limit;
      p->main_ahook->out_limit = p->cf->out_limit;
      p->main_ahook->in_keep_filtered = p->cf->in_keep_filtered;

      proto_reset_limit(p->main_ahook->rx_limit);
      proto_reset_limit(p->main_ahook->in_limit);
      proto_reset_limit(p->main_ahook->out_limit);
    }

  p->core_state = FS_HAPPY;
  proto_relink(p);
}

static void
proto_want_export_up(struct proto *p)
{
  ASSERT(p->core_state == FS_HAPPY);
  ASSERT(p->export_state == ES_DOWN);

  proto_link_ahooks(p);
  proto_schedule_feed(p, 1); /* Sets ES_FEEDING */
}

static void
proto_want_export_down(struct proto *p)
{
  ASSERT(p->export_state != ES_DOWN);

  /* Need to abort feeding */
  if (p->export_state == ES_FEEDING)
    rt_feed_baby_abort(p);

  p->export_state = ES_DOWN;
  p->stats.exp_routes = 0;
  proto_unlink_ahooks(p);
}

static void
proto_want_core_down(struct proto *p)
{
  ASSERT(p->core_state == FS_HAPPY);
  ASSERT(p->export_state == ES_DOWN);

  p->core_state = FS_FLUSHING;
  proto_relink(p);
  proto_schedule_flush_loop();

  if (!p->proto->multitable)
    {
      rt_unlock_source(p->main_source);
      p->main_source = NULL;
    }
}

static void
proto_falling_down(struct proto *p)
{
  p->gr_recovery = 0;
  p->gr_wait = 0;
  if (p->gr_lock)
    proto_graceful_restart_unlock(p);
}

static void
proto_fell_down(struct proto *p)
{
  DBG("Protocol %s down\n", p->name);

  u32 all_routes = p->stats.imp_routes + p->stats.filt_routes;
  if (all_routes != 0)
    log(L_ERR "Protocol %s is down but still has %d routes", p->name, all_routes);

  bzero(&p->stats, sizeof(struct proto_stats));
  proto_free_ahooks(p);

  if (! p->proto->multitable)
    rt_unlock_table(p->table);

  if (p->proto->cleanup)
    p->proto->cleanup(p);

  proto_rethink_goal(p);
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
  unsigned es = p->export_state;

  DBG("%s reporting state transition %s/%s -> */%s\n", p->name, c_states[cs], p_states[ops], p_states[ps]);
  if (ops == ps)
    return;

  p->proto_state = ps;
  p->last_state_change = now;

  switch (ps)
    {
    case PS_START:
      ASSERT(ops == PS_DOWN || ops == PS_UP);
      ASSERT(cs == FS_HUNGRY || cs == FS_HAPPY);

      if (es != ES_DOWN)
	proto_want_export_down(p);
      break;

    case PS_UP:
      ASSERT(ops == PS_DOWN || ops == PS_START);
      ASSERT(cs == FS_HUNGRY || cs == FS_HAPPY);
      ASSERT(es == ES_DOWN);

      if (cs == FS_HUNGRY)
	proto_want_core_up(p);
      if (!p->gr_wait)
	proto_want_export_up(p);
      break;

    case PS_STOP:
      ASSERT(ops == PS_START || ops == PS_UP);

      p->down_sched = 0;

      if (es != ES_DOWN)
	proto_want_export_down(p);
      if (cs == FS_HAPPY)
	proto_want_core_down(p);
      proto_falling_down(p);
      break;

    case PS_DOWN:
      p->down_code = 0;
      p->down_sched = 0;

      if (es != ES_DOWN)
	proto_want_export_down(p);
      if (cs == FS_HAPPY)
	proto_want_core_down(p);
      if (ops != PS_STOP)
	proto_falling_down(p);

      neigh_prune(); // FIXME convert neighbors to resource?
      rfree(p->pool);
      p->pool = NULL;

      if (cs == FS_HUNGRY)		/* Shutdown finished */
	{
	  proto_log_state_change(p);
	  proto_fell_down(p);
	  return;			/* The protocol might have ceased to exist */
	}
      break;

    default:
      bug("%s: Invalid state %d", p->name, ps);
    }

  proto_log_state_change(p);
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
    case P(PS_START, FS_HUNGRY):
    case P(PS_START, FS_HAPPY):		return "start";
    case P(PS_UP, FS_HAPPY):
      switch (p->export_state)
	{
	case ES_DOWN:			return "wait";
	case ES_FEEDING:		return "feed";
	case ES_READY:			return "up";
	default:      			return "???";
	}
    case P(PS_STOP, FS_HUNGRY):
    case P(PS_STOP, FS_FLUSHING):	return "stop";
    case P(PS_DOWN, FS_FLUSHING):	return "flush";
    default:      			return "???";
    }
#undef P
}

static void
proto_show_stats(struct proto_stats *s, int in_keep_filtered)
{
  if (in_keep_filtered)
    cli_msg(-1006, "  Routes:         %u imported, %u filtered, %u exported, %u preferred", 
	    s->imp_routes, s->filt_routes, s->exp_routes, s->pref_routes);
  else
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

void
proto_show_limit(struct proto_limit *l, const char *dsc)
{
  if (!l)
    return;

  cli_msg(-1006, "  %-16s%d%s", dsc, l->limit, l->state ? " [HIT]" : "");
  cli_msg(-1006, "    Action:       %s", proto_limit_name(l));
}

void
proto_show_basic_info(struct proto *p)
{
  // cli_msg(-1006, "  Table:          %s", p->table->name);
  cli_msg(-1006, "  Preference:     %d", p->preference);
  cli_msg(-1006, "  Input filter:   %s", filter_name(p->cf->in_filter));
  cli_msg(-1006, "  Output filter:  %s", filter_name(p->cf->out_filter));

  if (graceful_restart_state == GRS_ACTIVE)
    cli_msg(-1006, "  GR recovery:   %s%s",
	    p->gr_lock ? " pending" : "",
	    p->gr_wait ? " waiting" : "");

  proto_show_limit(p->cf->rx_limit, "Receive limit:");
  proto_show_limit(p->cf->in_limit, "Import limit:");
  proto_show_limit(p->cf->out_limit, "Export limit:");

  if (p->proto_state != PS_DOWN)
    proto_show_stats(&p->stats, p->cf->in_keep_filtered);
}

void
proto_cmd_show(struct proto *p, uint verbose, int cnt)
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

      if (p->proto->show_proto_info)
	p->proto->show_proto_info(p);
      else
	proto_show_basic_info(p);

      cli_msg(-1006, "");
    }
}

void
proto_cmd_disable(struct proto *p, uint arg UNUSED, int cnt UNUSED)
{
  if (p->disabled)
    {
      cli_msg(-8, "%s: already disabled", p->name);
      return;
    }

  log(L_INFO "Disabling protocol %s", p->name);
  p->disabled = 1;
  p->down_code = PDC_CMD_DISABLE;
  proto_rethink_goal(p);
  cli_msg(-9, "%s: disabled", p->name);
}

void
proto_cmd_enable(struct proto *p, uint arg UNUSED, int cnt UNUSED)
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
proto_cmd_restart(struct proto *p, uint arg UNUSED, int cnt UNUSED)
{
  if (p->disabled)
    {
      cli_msg(-8, "%s: already disabled", p->name);
      return;
    }

  log(L_INFO "Restarting protocol %s", p->name);
  p->disabled = 1;
  p->down_code = PDC_CMD_RESTART;
  proto_rethink_goal(p);
  p->disabled = 0;
  proto_rethink_goal(p);
  cli_msg(-12, "%s: restarted", p->name);
}

void
proto_cmd_reload(struct proto *p, uint dir, int cnt UNUSED)
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
    {
      if (! (p->reload_routes && p->reload_routes(p)))
	{
	  cli_msg(-8006, "%s: reload failed", p->name);
	  return;
	}

      /*
       * Should be done before reload_routes() hook?
       * Perhaps, but these hooks work asynchronously.
       */
      if (!p->proto->multitable)
	{
	  proto_reset_limit(p->main_ahook->rx_limit);
	  proto_reset_limit(p->main_ahook->in_limit);
	}
    }

  /* re-exporting routes */
  if (dir != CMD_RELOAD_IN)
    proto_request_feeding(p);

  cli_msg(-15, "%s: reloading", p->name);
}

void
proto_cmd_debug(struct proto *p, uint mask, int cnt UNUSED)
{
  p->debug = mask;
}

void
proto_cmd_mrtdump(struct proto *p, uint mask, int cnt UNUSED)
{
  p->mrtdump = mask;
}

static void
proto_apply_cmd_symbol(struct symbol *s, void (* cmd)(struct proto *, uint, int), uint arg)
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
proto_apply_cmd_patt(char *patt, void (* cmd)(struct proto *, uint, int), uint arg)
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
proto_apply_cmd(struct proto_spec ps, void (* cmd)(struct proto *, uint, int),
		int restricted, uint arg)
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
