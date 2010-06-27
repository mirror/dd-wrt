/*
 * ProFTPD: mod_dynmasq -- a module for dynamically updating MasqueradeAddress
 *                         configurations, as when DynDNS names are used
 *
 * Copyright (c) 2004-2010 TJ Saunders
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * This is mod_dynmasq, contrib software for proftpd 1.2.x and above.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 *
 * $Id: mod_dynmasq.c,v 1.7 2010/02/10 01:01:14 castaglia Exp $
 */

#include "conf.h"

#define MOD_DYNMASQ_VERSION		"mod_dynmasq/0.3"

/* Make sure the version of proftpd is as necessary. */
#if PROFTPD_VERSION_NUMBER < 0x0001030201
# error "ProFTPD 1.3.2rc1 or later required"
#endif

extern xaset_t *server_list;
module dynmasq_module;
static int dynmasq_timer_id = -1;
static int dynmasq_timer_interval = -1;

/* Configuration handlers
 */

/* usage: DynMasqRefresh <seconds> */
MODRET set_dynmasqrefresh(cmd_rec *cmd) {
  CHECK_CONF(cmd, CONF_ROOT);
  CHECK_ARGS(cmd, 1);

  dynmasq_timer_interval = atoi(cmd->argv[1]);
  if (dynmasq_timer_interval < 1)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      "must be greater than zero: '", cmd->argv[1], "'", NULL));

  return PR_HANDLED(cmd);
}

/* Timers
 */

static int dynmasq_update_cb(CALLBACK_FRAME) {
  server_rec *s;

  /* Clear the netaddr cache.  Sadly, this is required in order for any
   * updates to be discovered this way.
   */
  pr_netaddr_clear_cache();

  pr_log_debug(DEBUG2, MOD_DYNMASQ_VERSION
    ": resolving all MasqueradeAddress directives (could take a little while)");

  for (s = (server_rec *) server_list->xas_list; s; s = s->next) {
    config_rec *c = find_config(s->conf, CONF_PARAM, "MasqueradeAddress",
      FALSE);

    if (c) {
      pr_netaddr_t *na = pr_netaddr_get_addr(s->pool, c->argv[1], NULL);

      if (na) {
        /* Compare the obtained netaddr with the one already present.
         * Only update the "live" netaddr if they differ.
         */
        pr_log_debug(DEBUG2, MOD_DYNMASQ_VERSION
          ": resolved MasqueradeAddress '%s' to IP address %s",
          (const char *) c->argv[1], pr_netaddr_get_ipstr(na));

        if (pr_netaddr_cmp(c->argv[0], na) != 0) {
          pr_log_pri(PR_LOG_DEBUG, MOD_DYNMASQ_VERSION
            ": MasqueradeAddress '%s' updated for new address %s (was %s)",
            (const char *) c->argv[1], pr_netaddr_get_ipstr(na),
            pr_netaddr_get_ipstr(c->argv[0]));

          /* Overwrite the old netaddr pointer.  Note that this constitutes
           * a minor memory leak, as there currently isn't a way to free
           * the memory used by a netaddr object.  Hrm.
           */
          c->argv[0] = na;

        } else
          pr_log_debug(DEBUG2, MOD_DYNMASQ_VERSION
            ": MasqueradeAddress '%s' has not changed addresses",
            (const char *) c->argv[1]);
 
      } else
        pr_log_pri(PR_LOG_NOTICE, MOD_DYNMASQ_VERSION
          ": unable to resolve '%s', keep previous address",
          (const char *) c->argv[1]);
    }
  }

  return 1;
}

/* Event handlers
 */

#if defined(PR_SHARED_MODULE)
static void dynmasq_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp("mod_dynmasq.c", (const char *) event_data) == 0) {
    pr_timer_remove(dynmasq_timer_id, &dynmasq_module);
    dynmasq_timer_id = -1;

    pr_event_unregister(&dynmasq_module, NULL, NULL);
  }
}
#endif /* !PR_SHARED_MODULE */

static void dynmasq_postparse_ev(const void *event_data, void *user_data) {
  if (dynmasq_timer_interval != -1) {
    dynmasq_timer_id = pr_timer_add(dynmasq_timer_interval, -1,
      &dynmasq_module, dynmasq_update_cb, "dynmasq address update");
  }
}

static void dynmasq_restart_ev(const void *event_data, void *user_data) {
  if (dynmasq_timer_id != -1) {
    pr_timer_remove(dynmasq_timer_id, &dynmasq_module);
    dynmasq_timer_id = -1;
  }
}

/* Initialization functions
 */

static int dynmasq_init(void) {
#if defined(PR_SHARED_MODULE)
  pr_event_register(&dynmasq_module, "core.module-unload",
    dynmasq_mod_unload_ev, NULL);
#endif /* !PR_SHARED_MODULE */

  pr_event_register(&dynmasq_module, "core.postparse", dynmasq_postparse_ev,
    NULL);
  pr_event_register(&dynmasq_module, "core.restart", dynmasq_restart_ev,
    NULL);

  return 0;
}

static int dynmasq_sess_init(void) {

  /* Ensure that the timer only fires on the daemon process. */
  pr_timer_remove(dynmasq_timer_id, &dynmasq_module);
  dynmasq_timer_id = -1;

  pr_event_unregister(&dynmasq_module, "core.restart", NULL);

  return 0;
}

/* Module API tables
 */

static conftable dynmasq_conftab[] = {
  { "DynMasqRefresh",	set_dynmasqrefresh,	NULL },
  { NULL }
};

module dynmasq_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "dynmasq",

  /* Module configuration handler table */
  dynmasq_conftab,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  dynmasq_init,

  /* Session initialization function */
  dynmasq_sess_init,

  /* Module version */
  MOD_DYNMASQ_VERSION
};
