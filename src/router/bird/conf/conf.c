/*
 *	BIRD Internet Routing Daemon -- Configuration File Handling
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Configuration manager
 *
 * Configuration of BIRD is complex, yet straightforward. There are three
 * modules taking care of the configuration: config manager (which takes care
 * of storage of the config information and controls switching between configs),
 * lexical analyzer and parser.
 *
 * The configuration manager stores each config as a &config structure
 * accompanied by a linear pool from which all information associated
 * with the config and pointed to by the &config structure is allocated.
 *
 * There can exist up to four different configurations at one time: an active
 * one (pointed to by @config), configuration we are just switching from
 * (@old_config), one queued for the next reconfiguration (@future_config;
 * if it's non-%NULL and the user wants to reconfigure once again, we just
 * free the previous queued config and replace it with the new one) and
 * finally a config being parsed (@new_config).
 *
 * Loading of new configuration is very simple: just call config_alloc()
 * to get a new &config structure, then use config_parse() to parse a
 * configuration file and fill all fields of the structure
 * and finally ask the config manager to switch to the new
 * config by calling config_commit().
 *
 * CLI commands are parsed in a very similar way -- there is also a stripped-down
 * &config structure associated with them and they are lex-ed and parsed by the
 * same functions, only a special fake token is prepended before the command
 * text to make the parser recognize only the rules corresponding to CLI commands.
 */

#include <setjmp.h>
#include <stdarg.h>

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/route.h"
#include "nest/protocol.h"
#include "nest/iface.h"
#include "lib/resource.h"
#include "lib/string.h"
#include "lib/event.h"
#include "lib/timer.h"
#include "conf/conf.h"
#include "filter/filter.h"

static jmp_buf conf_jmpbuf;

struct config *config, *new_config, *old_config, *future_config;
static event *config_event;
int shutting_down, future_type;
bird_clock_t boot_time;

/**
 * config_alloc - allocate a new configuration
 * @name: name of the config
 *
 * This function creates new &config structure, attaches a resource
 * pool and a linear memory pool to it and makes it available for
 * further use. Returns a pointer to the structure.
 */
struct config *
config_alloc(byte *name)
{
  pool *p = rp_new(&root_pool, "Config");
  linpool *l = lp_new(p, 4080);
  struct config *c = lp_allocz(l, sizeof(struct config));

  c->mrtdump_file = -1; /* Hack, this should be sysdep-specific */
  c->pool = p;
  cfg_mem = c->mem = l;
  c->file_name = cfg_strdup(name);
  c->load_time = now;
  c->tf_base.fmt1 = c->tf_log.fmt1 = "%d-%m-%Y %T";

  if (!boot_time)
    boot_time = now;
  return c;
}

/**
 * config_parse - parse a configuration
 * @c: configuration
 *
 * config_parse() reads input by calling a hook function pointed to
 * by @cf_read_hook and parses it according to the configuration
 * grammar. It also calls all the preconfig and postconfig hooks
 * before, resp. after parsing.
 *
 * Result: 1 if the config has been parsed successfully, 0 if any
 * error has occurred (such as anybody calling cf_error()) and
 * the @err_msg field has been set to the error message.
 */
int
config_parse(struct config *c)
{
  DBG("Parsing configuration file `%s'\n", c->file_name);
  new_config = c;
  cfg_mem = c->mem;
  if (setjmp(conf_jmpbuf))
    return 0;
  cf_lex_init(0);
  sysdep_preconfig(c);
  protos_preconfig(c);
  rt_preconfig(c);
  cf_parse();
  protos_postconfig(c);
  if (EMPTY_LIST(c->protos))
    cf_error("No protocol is specified in the config file");
#ifdef IPV6
  if (!c->router_id)
    cf_error("Router ID must be configured manually on IPv6 routers");
#endif
  return 1;
}

/**
 * cli_parse - parse a CLI command
 * @c: temporary config structure
 *
 * cli_parse() is similar to config_parse(), but instead of a configuration,
 * it parses a CLI command. See the CLI module for more information.
 */
int
cli_parse(struct config *c)
{
  new_config = c;
  c->sym_fallback = config->sym_hash;
  cfg_mem = c->mem;
  if (setjmp(conf_jmpbuf))
    return 0;
  cf_lex_init(1);
  cf_parse();
  return 1;
}

/**
 * config_free - free a configuration
 * @c: configuration to be freed
 *
 * This function takes a &config structure and frees all resources
 * associated with it.
 */
void
config_free(struct config *c)
{
  rfree(c->pool);
}

void
config_add_obstacle(struct config *c)
{
  DBG("+++ adding obstacle %d\n", c->obstacle_count);
  c->obstacle_count++;
}

void
config_del_obstacle(struct config *c)
{
  DBG("+++ deleting obstacle %d\n", c->obstacle_count);
  c->obstacle_count--;
  if (!c->obstacle_count)
    {
      ASSERT(config_event);
      ev_schedule(config_event);
    }
}

static int
global_commit(struct config *new, struct config *old)
{
  if (!old)
    return 0;

  if (!ipa_equal(old->listen_bgp_addr, new->listen_bgp_addr) ||
      (old->listen_bgp_port != new->listen_bgp_port) ||
      (old->listen_bgp_flags != new->listen_bgp_flags))
    log(L_WARN "Reconfiguration of BGP listening socket not implemented, please restart BIRD.");

  if (!new->router_id)
    new->router_id = old->router_id;
  if (new->router_id != old->router_id)
    return 1;
  return 0;
}

static int
config_do_commit(struct config *c, int type)
{
  int force_restart, nobs;

  DBG("do_commit\n");
  old_config = config;
  config = new_config = c;
  if (old_config)
    old_config->obstacle_count++;

  DBG("sysdep_commit\n");
  force_restart = sysdep_commit(c, old_config);
  DBG("global_commit\n");
  force_restart |= global_commit(c, old_config);
  DBG("rt_commit\n");
  rt_commit(c, old_config);
  DBG("protos_commit\n");
  protos_commit(c, old_config, force_restart, type);
  new_config = NULL;			/* Just to be sure nobody uses that now */
  if (old_config)
    nobs = --old_config->obstacle_count;
  else
    nobs = 0;
  DBG("do_commit finished with %d obstacles remaining\n", nobs);
  return !nobs;
}

static void
config_done(void *unused UNUSED)
{
  struct config *c;

  DBG("config_done\n");
  for(;;)
    {
      if (config->shutdown)
	sysdep_shutdown_done();
      log(L_INFO "Reconfigured");
      if (old_config)
	{
	  config_free(old_config);
	  old_config = NULL;
	}
      if (!future_config)
	break;
      c = future_config;
      future_config = NULL;
      log(L_INFO "Reconfiguring to queued configuration");
      if (!config_do_commit(c, future_type))
	break;
    }
}

/**
 * config_commit - commit a configuration
 * @c: new configuration
 * @type: type of reconfiguration (RECONFIG_SOFT or RECONFIG_HARD)
 *
 * When a configuration is parsed and prepared for use, the
 * config_commit() function starts the process of reconfiguration.
 * It checks whether there is already a reconfiguration in progress
 * in which case it just queues the new config for later processing.
 * Else it notifies all modules about the new configuration by calling
 * their commit() functions which can either accept it immediately
 * or call config_add_obstacle() to report that they need some time
 * to complete the reconfiguration. After all such obstacles are removed
 * using config_del_obstacle(), the old configuration is freed and
 * everything runs according to the new one.
 *
 * Result: %CONF_DONE if the configuration has been accepted immediately,
 * %CONF_PROGRESS if it will take some time to switch to it, %CONF_QUEUED
 * if it's been queued due to another reconfiguration being in progress now
 * or %CONF_SHUTDOWN if BIRD is in shutdown mode and no new configurations
 * are accepted.
 */
int
config_commit(struct config *c, int type)
{
  if (!config)				/* First-time configuration */
    {
      config_do_commit(c, RECONFIG_HARD);
      return CONF_DONE;
    }
  if (old_config)			/* Reconfiguration already in progress */
    {
      if (shutting_down == 2)
	{
	  log(L_INFO "New configuration discarded due to shutdown");
	  config_free(c);
	  return CONF_SHUTDOWN;
	}
      if (future_config)
	{
	  log(L_INFO "Queueing new configuration, ignoring the one already queued");
	  config_free(future_config);
	}
      else
	log(L_INFO "Queued new configuration");
      future_config = c;
      future_type = type;
      return CONF_QUEUED;
    }

  if (!shutting_down)
    log(L_INFO "Reconfiguring");

  if (config_do_commit(c, type))
    {
      config_done(NULL);
      return CONF_DONE;
    }
  if (!config_event)
    {
      config_event = ev_new(&root_pool);
      config_event->hook = config_done;
    }
  return CONF_PROGRESS;
}

/**
 * order_shutdown - order BIRD shutdown
 *
 * This function initiates shutdown of BIRD. It's accomplished by asking
 * for switching to an empty configuration.
 */
void
order_shutdown(void)
{
  struct config *c;

  if (shutting_down)
    return;
  log(L_INFO "Shutting down");
  c = lp_alloc(config->mem, sizeof(struct config));
  memcpy(c, config, sizeof(struct config));
  init_list(&c->protos);
  init_list(&c->tables);
  c->shutdown = 1;
  shutting_down = 1;
  config_commit(c, RECONFIG_HARD);
  shutting_down = 2;
}

/**
 * cf_error - report a configuration error
 * @msg: printf-like format string
 *
 * cf_error() can be called during execution of config_parse(), that is
 * from the parser, a preconfig hook or a postconfig hook, to report an
 * error in the configuration.
 */
void
cf_error(char *msg, ...)
{
  char buf[1024];
  va_list args;

  va_start(args, msg);
  if (bvsnprintf(buf, sizeof(buf), msg, args) < 0)
    strcpy(buf, "<bug: error message too long>");
  new_config->err_msg = cfg_strdup(buf);
  new_config->err_lino = conf_lino;
  longjmp(conf_jmpbuf, 1);
}

/**
 * cfg_strdup - copy a string to config memory
 * @c: string to copy
 *
 * cfg_strdup() creates a new copy of the string in the memory
 * pool associated with the configuration being currently parsed.
 * It's often used when a string literal occurs in the configuration
 * and we want to preserve it for further use.
 */
char *
cfg_strdup(char *c)
{
  int l = strlen(c) + 1;
  char *z = cfg_allocu(l);
  memcpy(z, c, l);
  return z;
}
