/*
 * ProFTPD: mod_ctrls_admin -- a module implementing admin control handlers
 *
 * Copyright (c) 2000-2010 TJ Saunders
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
 * This is mod_controls, contrib software for proftpd 1.2 and above.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 *
 * $Id: mod_ctrls_admin.c,v 1.40 2010/01/10 20:01:30 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"
#include "mod_ctrls.h"

#define MOD_CTRLS_ADMIN_VERSION		"mod_ctrls_admin/0.9.6"

/* Make sure the version of proftpd is as necessary. */
#if PROFTPD_VERSION_NUMBER < 0x0001030001
# error "ProFTPD 1.3.0rc1 or later required"
#endif

#ifndef PR_USE_CTRLS
# error "Controls support required (use --enable-ctrls)"
#endif

/* Values for the stop flags */
#define CTRL_STOP_DEFAULT     (1 << 0)
#define CTRL_STOP_CLEAN       (1 << 1)
#define CTRL_STOP_FULL        (1 << 2)
#define CTRL_STOP_GRACEFUL    (1 << 3)

/* For the 'shutdown' control action */
#define CTRLS_DEFAULT_SHUTDOWN_WAIT	5

/* From src/dirtree.c */
extern xaset_t *server_list;
extern int ServerUseReverseDNS;

module ctrls_admin_module;
static ctrls_acttab_t ctrls_admin_acttab[];

/* Pool for this module's use */
static pool *ctrls_admin_pool = NULL;

static unsigned int ctrls_admin_nrestarts = 0;
static time_t ctrls_admin_start = 0;

/* Support routines
 */

#if 0
/* Will be used when scheduled shutdowns are supported.. */
static unsigned char isnumeric(char *str) {
  while (str && isspace((int) *str))
    str++;

  if (!str || !*str)
    return FALSE;

  for (; str && *str; str++) {
    if (!isdigit((int) *str))
      return TRUE;
  }

  return 1;
}
#endif

static int respcmp(const void *a, const void *b) {
  return strcmp(*((char **) a), *((char **) b));
}

#ifdef PR_USE_DEVEL
static pr_ctrls_t *mem_ctrl = NULL;

static void mem_printf(const char *fmt, ...) {
  char buf[PR_TUNABLE_BUFFER_SIZE];
  va_list msg;

  memset(buf, '\0', sizeof(buf)); 

  va_start(msg, fmt);
  vsnprintf(buf, sizeof(buf), fmt, msg);
  va_end(msg);
 
  buf[sizeof(buf)-1] = '\0';
 
  pr_ctrls_add_response(mem_ctrl, "pool: %s", buf);
}
#endif /* !PR_USE_DEVEL */

/* Controls handlers
 */

static int ctrls_handle_debug(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {

  /* Check the debug ACL */
  if (!pr_ctrls_check_acl(ctrl, ctrls_admin_acttab, "debug")) {

    /* Access denied */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Sanity check */
  if (reqargc == 0 || reqargv == NULL) {
    pr_ctrls_add_response(ctrl, "debug: missing required parameters");
    return -1;
  }

  /* Handle 'debug level' requests */
  if (strcmp(reqargv[0], "level") == 0) {
    int level = 0;

    if (reqargc != 2) {
      pr_ctrls_add_response(ctrl, "debug: missing required parameters");
      return -1;
    }

    if ((level = atoi(reqargv[1])) < 0) {
      pr_ctrls_add_response(ctrl, "debug level must not be negative");
      return -1; 
    }
  
    pr_log_setdebuglevel(level);
    pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "debug: level set to %d", level);
    pr_ctrls_add_response(ctrl, "debug level set to %d", level);

#ifdef PR_USE_DEVEL
  /* Handle 'debug memory' requests */
  } else if (strcmp(reqargv[0], "mem") == 0 ||
             strcmp(reqargv[0], "memory") == 0) {

    if (reqargc != 1) {
      pr_ctrls_add_response(ctrl, "debug: too many parameters");
      return -1;
    }

    mem_ctrl = ctrl;
    pr_pool_debug_memory(mem_printf);
    mem_ctrl = NULL;

    pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "debug: dumped memory info");

#endif /* PR_USE_DEVEL */

  } else {
    pr_ctrls_add_response(ctrl, "unknown debug action: '%s'", reqargv[0]);
    return -1;
  }

  return 0;
}

static int ctrls_handle_dns(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  int bool;

  /* Check the dns ACL */
  if (!pr_ctrls_check_acl(ctrl, ctrls_admin_acttab, "dns")) {

    /* Access denied */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Sanity check */
  if (reqargc == 0 || reqargv == NULL) {
    pr_ctrls_add_response(ctrl, "dns: missing required parameters");
    return -1;
  }

  if (reqargc != 1 &&
      reqargc != 2) {
    pr_ctrls_add_response(ctrl, "dns: wrong number of parameters");
    return -1;
  }

  if (reqargc == 2 &&
      strcmp(reqargv[0], "cache") == 0) {
    if (strcmp(reqargv[1], "clear") != 0) {
      pr_ctrls_add_response(ctrl,
        "dns: error: expected 'clear' command: '%s'", reqargv[1]);
      return -1;
    }

    pr_netaddr_clear_cache();
    pr_ctrls_add_response(ctrl, "dns: netaddr cache cleared");
    
  } else {
    bool = pr_str_is_boolean(reqargv[0]);
    if (bool == -1) {
      pr_ctrls_add_response(ctrl,
        "dns: error: expected Boolean parameter: '%s'", reqargv[0]);
      return -1;
    }

    ServerUseReverseDNS = bool;

    pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "dns: UseReverseDNS set to '%s'",
      bool ? "on" : "off");
    pr_ctrls_add_response(ctrl, "dns: UseReverseDNS set to '%s'",
      bool ? "on" : "off");
  }

  return 0;
}

static int admin_addr_down(pr_ctrls_t *ctrl, pr_netaddr_t *addr,
    unsigned int port) {

  pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "down: disabling %s#%u",
    pr_netaddr_get_ipstr(addr), port);

  if (pr_ipbind_close(addr, port, FALSE) < 0) {
    if (errno == ENOENT)
      pr_ctrls_add_response(ctrl, "down: no such server: %s#%u",
        pr_netaddr_get_ipstr(addr), port);
    else
      pr_ctrls_add_response(ctrl, "down: %s#%u already disabled",
        pr_netaddr_get_ipstr(addr), port);

  } else
    pr_ctrls_add_response(ctrl, "down: %s#%u disabled",
      pr_netaddr_get_ipstr(addr), port);

  return 0;
}

static int ctrls_handle_down(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  register unsigned int i = 0;

  /* Handle scheduled downs of virtual servers in the future, and
   * cancellations of scheduled downs.
   */

  /* Check the 'down' ACL */
  if (!pr_ctrls_check_acl(ctrl, ctrls_admin_acttab, "down")) {

    /* Access denied */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Sanity check */
  if (reqargc < 1 || reqargv == NULL) {
    pr_ctrls_add_response(ctrl, "down: missing required parameters");
    return -1;
  }

  for (i = 0; i < reqargc; i++) {
    unsigned int server_port = 21;
    char *server_str = reqargv[i], *tmp = NULL;
    pr_netaddr_t *server_addr = NULL;
    array_header *addrs = NULL;

    /* Check for an argument of "all" */
    if (strcasecmp(server_str, "all") == 0) {
      pr_ipbind_close(NULL, 0, FALSE);
      pr_ctrls_add_response(ctrl, "down: all servers disabled");
      return 0;
    }

    tmp = strchr(server_str, '#');
    if (tmp != NULL) {
      server_port = atoi(tmp + 1);
      *tmp = '\0';
    }

    server_addr = pr_netaddr_get_addr(ctrl->ctrls_tmp_pool, server_str, &addrs);
    if (!server_addr) {
      pr_ctrls_add_response(ctrl, "down: no such server: %s#%u",
        server_str, server_port);
      continue;
    }

    admin_addr_down(ctrl, server_addr, server_port);

    if (addrs) {
      register unsigned int j;
      pr_netaddr_t **elts = addrs->elts;

      for (j = 0; j < addrs->nelts; j++)
        admin_addr_down(ctrl, elts[j], server_port);
    }
  }

  return 0;
}

static int ctrls_handle_get(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  int res = 0;

  /* Sanity check */
  if (reqargc == 0 || reqargv == NULL) {
    pr_ctrls_add_response(ctrl, "get: missing required parameters");
    return -1;
  }

  /* Handle 'get config' requests */
  if (strcmp(reqargv[0], "config") == 0) {
    if (reqargc >= 2) {
      register int i = 0;

      for (i = 1; i < reqargc; i++) {
        config_rec *c = NULL;

        /* NOTE: there are some directives that are not stored as config_recs,
         * but rather as static variables or as members of other structs.
         * Handle these exceptions as well?  These include ServerName,
         * ServerType, ServerAdmin, etc.  How to handle configs that should
         * be retrievable, but are Boolean values instead of strings.  Hmmm.
         */

        if ((c = find_config(main_server->conf, CONF_PARAM, reqargv[i],
            FALSE)) != NULL) {

#if 0
          /* Not yet supported */
          if (c->flags & CF_GCTRL)
            pr_ctrls_add_response(ctrl, "%s: %s", reqargv[i],
              (char *) c->argv[0]);
          else
#endif
            pr_ctrls_add_response(ctrl, "%s: not retrievable", reqargv[i]);

        } else
          pr_ctrls_add_response(ctrl, "%s: directive not found", reqargv[i]);
      }

    } else {
      pr_ctrls_add_response(ctrl, "%s: missing parameters", reqargv[0]);
      res = -1;
    }

  /* Handle 'get directives' requests */
  } else if (strcmp(reqargv[0], "directives") == 0) {

    if (reqargc == 1) {
      conftable *conftab;
      int stash_idx = -1;

      /* Create a list of all known configuration directives. */

      conftab = pr_stash_get_symbol(PR_SYM_CONF, NULL, NULL, &stash_idx);

      while (stash_idx != -1) {
        pr_signals_handle();

        if (conftab) {
          pr_ctrls_add_response(ctrl, "%s (mod_%s.c)", conftab->directive,
            conftab->m->name);

        } else
          stash_idx++;

        conftab = pr_stash_get_symbol(PR_SYM_CONF, NULL, conftab, &stash_idx);
      }

      /* Be nice, and sort the directives lexicographically */
      qsort(ctrl->ctrls_cb_resps->elts, ctrl->ctrls_cb_resps->nelts,
        sizeof(char *), respcmp);

    } else {
      pr_ctrls_add_response(ctrl, "%s: wrong number of parameters", reqargv[0]);
      res = -1;
    }

  } else {
    pr_ctrls_add_response(ctrl, "unknown get type requested: '%s'", reqargv[0]);
    res = -1;
  }

  return res;
}

static int ctrls_handle_kick(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  int res = 0;

  /* Check the kick ACL */
  if (!pr_ctrls_check_acl(ctrl, ctrls_admin_acttab, "kick")) {

    /* Access denied */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Sanity check */
  if (reqargc == 0 || reqargv == NULL) {
    pr_ctrls_add_response(ctrl, "missing required parameters");
    return -1;
  }

  /* Handle 'kick user' requests. */
  if (strcmp(reqargv[0], "user") == 0) {
    register unsigned int i = 0;
    pr_scoreboard_entry_t *score = NULL;

    if (reqargc == 1) {
      pr_ctrls_add_response(ctrl, "kick user: missing required user name(s)");
      return -1;
    }

    /* Iterate through the scoreboard, and send a SIGTERM to each
     * pid whose name matches the given user name(s).
     */
    for (i = 1; i < reqargc; i++) {
      unsigned char kicked_user = FALSE;

      if (pr_rewind_scoreboard() < 0) {
        pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "error rewinding scoreboard: %s",
          strerror(errno));
      }

      while ((score = pr_scoreboard_entry_read()) != NULL) {
        if (strcmp(reqargv[i], score->sce_user) == 0) {
          res = 0;

          PRIVS_ROOT
          res = pr_scoreboard_entry_kill(score, SIGTERM);
          PRIVS_RELINQUISH

          if (res == 0) {
            kicked_user = TRUE;

          } else {
            pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION,
              "error kicking user '%s': %s", reqargv[i], strerror(errno));
          }
        }
      }

      if (pr_restore_scoreboard() < 0) {
        pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "error restoring scoreboard: %s",
          strerror(errno));
      }

      if (kicked_user) {
        pr_ctrls_add_response(ctrl, "kicked user '%s'", reqargv[i]);
        pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "kicked user '%s'", reqargv[i]);
        pr_log_debug(DEBUG4, MOD_CTRLS_ADMIN_VERSION ": kicked user '%s'",
          reqargv[i]);

      } else
        pr_ctrls_add_response(ctrl, "user '%s' not connected", reqargv[i]);
    }

  /* Handle 'kick host' requests. */
  } else if (strcmp(reqargv[0], "host") == 0) {
    register unsigned int i = 0;
    pr_scoreboard_entry_t *score = NULL;

    if (reqargc == 1) {
      pr_ctrls_add_response(ctrl, "kick host: missing required host(s)");
      return -1;
    }

    /* Iterate through the scoreboard, and send a SIGTERM to each
     * pid whose address matches the given host name (resolve to
     * stringified IP address).
     */

    for (i = 1; i < reqargc; i++) {
      unsigned char kicked_host = FALSE;
      const char *addr;
      pr_netaddr_t *na;

      na = pr_netaddr_get_addr(ctrl->ctrls_tmp_pool, reqargv[1], NULL);
      if (!na) {
        pr_ctrls_add_response(ctrl, "kick host: error resolving '%s': %s",
          reqargv[1], strerror(errno));
        continue;
      }

      addr = pr_netaddr_get_ipstr(na);

      if (pr_rewind_scoreboard() < 0) {
        pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "error rewinding scoreboard: %s",
          strerror(errno));
      }

      while ((score = pr_scoreboard_entry_read()) != NULL) {
        if (strcmp(score->sce_client_addr, addr) == 0) {
          PRIVS_ROOT
          if (pr_scoreboard_entry_kill(score, SIGTERM) == 0)
            kicked_host = TRUE;
          PRIVS_RELINQUISH
        }
      }
      pr_restore_scoreboard();

      if (kicked_host) {
        pr_ctrls_add_response(ctrl, "kicked host '%s'", addr);
        pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "kicked host '%s'", addr);
        pr_log_debug(DEBUG4, MOD_CTRLS_ADMIN_VERSION ": kicked host '%s'",
          addr);

      } else
        pr_ctrls_add_response(ctrl, "host '%s' not connected", addr);
    }

  /* Handle 'kick class' requests. */
  } else if (strcmp(reqargv[0], "class") == 0) {
    register unsigned int i = 0;
    pr_scoreboard_entry_t *score = NULL;

    if (reqargc == 1) {
      pr_ctrls_add_response(ctrl, "kick class: missing required class name(s)");
      return -1;
    }

    /* Iterate through the scoreboard, and send a SIGTERM to each
     * pid whose name matches the given class name(s).
     */
    for (i = 1; i < reqargc; i++) {
      unsigned char kicked_class = FALSE;

      if (pr_rewind_scoreboard() < 0) {
        pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "error rewinding scoreboard: %s",
          strerror(errno));
      }

      while ((score = pr_scoreboard_entry_read()) != NULL) {
        if (strcmp(reqargv[i], score->sce_class) == 0) {
          res = 0;

          PRIVS_ROOT
          res = pr_scoreboard_entry_kill(score, SIGTERM);
          PRIVS_RELINQUISH

          if (res == 0) {
            kicked_class = TRUE;

          } else {
            pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION,
              "error kicking class '%s': %s", reqargv[i], strerror(errno));
          }
        }
      }

      if (pr_restore_scoreboard() < 0) {
        pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "error restoring scoreboard: %s",
          strerror(errno));
      }

      if (kicked_class) {
        pr_ctrls_add_response(ctrl, "kicked class '%s'", reqargv[i]);
        pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "kicked class '%s'", reqargv[i]);
        pr_log_debug(DEBUG4, MOD_CTRLS_ADMIN_VERSION ": kicked class '%s'",
          reqargv[i]);

      } else
        pr_ctrls_add_response(ctrl, "class '%s' not connected", reqargv[i]);
    }

  } else {
    pr_ctrls_add_response(ctrl, "unknown kick type requested: '%s'",
      reqargv[0]);
    res = -1;
  }

  return res;
}

static int ctrls_handle_restart(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {

  /* Check the restart ACL */
  if (!pr_ctrls_check_acl(ctrl, ctrls_admin_acttab, "restart")) {

    /* Access denied */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Be pedantic */
  if (reqargc > 1) {
    pr_ctrls_add_response(ctrl, "bad number of arguments");
    return -1;
  }

  if (reqargc == 0) {
    PRIVS_ROOT
    raise(SIGHUP);
    PRIVS_RELINQUISH

    pr_ctrls_add_response(ctrl, "restarted server");

  } else if (reqargc == 1) {
    if (strcmp(reqargv[0], "count") == 0) {
      struct tm *tm;

      tm = pr_gmtime(ctrl->ctrls_tmp_pool, &ctrls_admin_start);
      pr_ctrls_add_response(ctrl,
        "server restarted %u %s since %04d-%02d-%02d %02d:%02d:%02d GMT",
        ctrls_admin_nrestarts, ctrls_admin_nrestarts != 1 ? "times" : "time",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min,
        tm->tm_sec);

    } else {
      pr_ctrls_add_response(ctrl, "unsupported parameter '%s'", reqargv[0]);
      return -1;
    }
  }

  return 0;
}

static int ctrls_handle_scoreboard(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {

  /* Check the scoreboard ACL. */
  if (!pr_ctrls_check_acl(ctrl, ctrls_admin_acttab, "scoreboard")) {

    /* Access denied. */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  if (reqargc != 1) {
    pr_ctrls_add_response(ctrl, "bad number of arguments");
    return -1;
  }

  if (strcmp(reqargv[0], "clean") == 0 ||
      strcmp(reqargv[0], "scrub") == 0) {

    pr_scoreboard_scrub();
    pr_ctrls_add_response(ctrl, "scrubbed scoreboard");
    return 0;
  }

  pr_ctrls_add_response(ctrl, "unknown scoreboard action '%s'", reqargv[0]);
  return -1;
}

static int ctrls_handle_shutdown(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  register unsigned int i = 0;
  int respargc = 0;
  char **respargv = NULL;

  /* Check the shutdown ACL */
  if (!pr_ctrls_check_acl(ctrl, ctrls_admin_acttab, "shutdown")) {

    /* Access denied */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Add a response */
  pr_ctrls_add_response(ctrl, "shutting down");

  if (reqargc >= 1 &&
      strcmp(reqargv[0], "graceful") == 0) {
    unsigned long nkids = 0;
    unsigned int waiting = CTRLS_DEFAULT_SHUTDOWN_WAIT;
    unsigned int timeout = 0;
    time_t now;

    if (reqargc == 2) {
      timeout = atoi(reqargv[1]);
      time(&now);

      pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION,
        "shutdown: waiting %u seconds before shutting down", timeout);

      /* If the timeout is less than the waiting period, reduce the
       * waiting period by half.
       */
      if (timeout < waiting)
        waiting /= 2;
    }

    /* Now, simply wait for all sessions to be done.  For bonus points,
     * the admin should be able to specify a timeout, after which any
     * sessions will be summarily terminated.  And, even better, have a
     * way to indicate to the sessions that the daemon wants to shut down,
     * and the session, if it is not involved in a data transfer, should
     * end itself.
     */

    nkids = child_count();
    while (nkids > 0) {
      if (timeout &&
          time(NULL) - now > timeout) {

        pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION,
          "shutdown: %u seconds elapsed, ending %lu remaining sessions",
          timeout, nkids);

        /* End all remaining sessions at this point. */
        PRIVS_ROOT
        child_signal(SIGTERM);
        PRIVS_RELINQUISH

        break;
      }

      pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION,
        "shutdown: waiting for %lu sessions to end", nkids);
      sleep(waiting);

      child_update();
      nkids = child_count();     

      /* Always check for sent signals in a while() loop. */
      pr_signals_handle();
    }
  }

  /* This is one of the rare cases where the control handler needs to
   * flush the responses out to the client manually, rather than waiting
   * for the normal controls cycle to handle it, as this handler is
   * not going to exit the function normally.
   */

  respargc = ctrl->ctrls_cb_resps->nelts;
  respargv = ctrl->ctrls_cb_resps->elts;

  /* Manually tweak the return value, for the benefit of the client */
  ctrl->ctrls_cb_retval = 0;

  if (pr_ctrls_flush_response(ctrl) < 0) {
    pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION,
      "shutdown: error flushing response: %s", strerror(errno));
  }

  /* For logging/accounting purposes */
  pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION,
    "shutdown: flushed to %s/%s client: return value: 0",
    ctrl->ctrls_cl->cl_user, ctrl->ctrls_cl->cl_group);

  for (i = 0; i < respargc; i++)
    pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION,
      "shutdown: flushed to %s/%s client: '%s'",
      ctrl->ctrls_cl->cl_user, ctrl->ctrls_cl->cl_group, respargv[i]);

  /* Shutdown by raising SIGTERM.  Easy. */
  raise(SIGTERM);

  return 0;
}

static int admin_addr_status(pr_ctrls_t *ctrl, pr_netaddr_t *addr,
    unsigned int port) {
  pr_ipbind_t *ipbind = NULL;

  pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "status: checking %s#%u",
    pr_netaddr_get_ipstr(addr), port);

  /* Fetch the ipbind associated with this address/port. */
  ipbind = pr_ipbind_find(addr, port, FALSE);
  if (ipbind == NULL) {
    pr_ctrls_add_response(ctrl,
      "status: no server associated with %s#%u", pr_netaddr_get_ipstr(addr),
      port);
    return -1;
  }

  pr_ctrls_add_response(ctrl, "status: %s#%u %s", pr_netaddr_get_ipstr(addr),
    port, ipbind->ib_isactive ? "UP" : "DOWN");

  return 0;
}

static int ctrls_handle_status(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  register unsigned int i = 0;

  /* Check the status ACL. */
  if (!pr_ctrls_check_acl(ctrl, ctrls_admin_acttab, "status")) {

    /* Access denied. */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Sanity check */ 
  if (reqargc < 1 || reqargv == NULL) {
    pr_ctrls_add_response(ctrl, "status: missing required parameters");
    return -1;
  }

  for (i = 0; i < reqargc; i++) {
    unsigned int server_port = 21;
    char *server_str = reqargv[i], *tmp = NULL;
    pr_netaddr_t *server_addr = NULL;
    array_header *addrs = NULL;

    /* Check for an argument of "all" */
    if (strcasecmp(server_str, "all") == 0) {
      pr_ipbind_t *ipbind = NULL;

      pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "status: checking all servers");

      while ((ipbind = pr_ipbind_get(ipbind)) != NULL) {
        const char *ipbind_str = pr_netaddr_get_ipstr(ipbind->ib_addr); 

        pr_ctrls_add_response(ctrl, "status: %s#%u %s", ipbind_str,
          ipbind->ib_port, ipbind->ib_isactive ? "UP" : "DOWN");
      }

      return 0;
    }

    tmp = strchr(server_str, '#');
    if (tmp != NULL) {
      server_port = atoi(tmp + 1);
      *tmp = '\0';
    }

    server_addr = pr_netaddr_get_addr(ctrl->ctrls_tmp_pool, server_str, &addrs);

    if (!server_addr) {
      pr_ctrls_add_response(ctrl, "status: no such server: %s#%u",
        server_str, server_port);
      continue;
    }

    if (admin_addr_status(ctrl, server_addr, server_port) < 0)
      continue;

    if (addrs) {
      register unsigned int j;
      pr_netaddr_t **elts = addrs->elts;

      for (j = 0; j < addrs->nelts; j++)
        admin_addr_status(ctrl, elts[j], server_port);
    }
  }

  return 0;
}

static int ctrls_handle_trace(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
#ifdef PR_USE_TRACE

  /* Check the trace ACL. */
  if (!pr_ctrls_check_acl(ctrl, ctrls_admin_acttab, "trace")) {

    /* Access denied. */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Sanity check */
  if (reqargc < 1 || reqargv == NULL) {
    pr_ctrls_add_response(ctrl, "trace: missing required parameters");
    return -1;
  }

  if (strcmp(reqargv[0], "info") != 0) {
    register unsigned int i;

    for (i = 0; i < reqargc; i++) {
      char *channel, *tmp;
      int level;

      tmp = strchr(reqargv[i], ':');
      if (!tmp) {
        pr_ctrls_add_response(ctrl, "trace: badly formatted parameter: '%s'",
          reqargv[i]);
        return -1;
      }

      channel = reqargv[i];
      *tmp = '\0';
      level = atoi(++tmp);

      if (pr_trace_set_level(channel, level) < 0) {
        pr_ctrls_add_response(ctrl,
          "trace: error setting channel '%s' to level %d: %s", channel, level,
          strerror(errno));
        return -1;

      } else {
        pr_ctrls_add_response(ctrl, "trace: set channel '%s' to level %d",
          channel, level);
      }
    }
 
  } else {
    pr_table_t *trace_tab = pr_trace_get_table();

    if (trace_tab) {
      void *key = NULL, *value = NULL;

      pr_ctrls_add_response(ctrl, "%-10s %-6s", "Channel", "Level");
      pr_ctrls_add_response(ctrl, "---------- ------");

      pr_table_rewind(trace_tab);
      key = pr_table_next(trace_tab);
      while (key) {
        pr_signals_handle();

        value = pr_table_get(trace_tab, (const char *) key, NULL);
        if (value) {
          pr_ctrls_add_response(ctrl, "%10s %-6d", (const char *) key,
            *((int *) value));
        }

        key = pr_table_next(trace_tab);
      }

    } else {
      pr_ctrls_add_response(ctrl, "trace: no info available");
    }
  }
 
  return 0;
#else
  pr_ctrls_add_response(ctrl, "trace: requires trace support (--enable-trace");
  return -1;
#endif /* PR_USE_TRACE */
}

static int admin_addr_up(pr_ctrls_t *ctrl, pr_netaddr_t *addr,
    unsigned int port) {
  pr_ipbind_t *ipbind = NULL;
  int res = 0;

  /* Fetch the ipbind associated with this address/port. */
  ipbind = pr_ipbind_find(addr, port, FALSE);
  if (ipbind == NULL) {
    pr_ctrls_add_response(ctrl,
      "up: no server associated with %s#%u", pr_netaddr_get_ipstr(addr),
      port);
    return -1;
  }

  /* If this ipbind is already active, abort now. */
  if (ipbind->ib_isactive) {
    pr_ctrls_add_response(ctrl, "up: %s#%u already enabled",
      pr_netaddr_get_ipstr(addr), port);
    return 0;
  }

  /* Determine whether this server_rec needs a listening connection
   * created.  A ServerType of SERVER_STANDALONE combined with a
   * SocketBindTight means each server_rec will have its own listen
   * connection; any other combination means that all the server_recs
   * share the same listen connection.
   */
  if (ipbind->ib_server->ServerPort && !ipbind->ib_server->listen) {
    ipbind->ib_server->listen =
      pr_inet_create_conn(ipbind->ib_server->pool, server_list, -1,
      (SocketBindTight ? ipbind->ib_server->addr : NULL),
      ipbind->ib_server->ServerPort, FALSE);
  }

  pr_ctrls_log(MOD_CTRLS_ADMIN_VERSION, "up: attempting to enable %s#%u",
    pr_netaddr_get_ipstr(addr), port);

  PR_OPEN_IPBIND(ipbind->ib_server->addr, ipbind->ib_server->ServerPort,
    ipbind->ib_server->listen, FALSE, FALSE, TRUE);

  if (res < 0)
    pr_ctrls_add_response(ctrl, "up: no server listening on %s#%u",
      pr_netaddr_get_ipstr(addr), port);
  else
    pr_ctrls_add_response(ctrl, "up: %s#%u enabled",
      pr_netaddr_get_ipstr(addr), port);

  PR_ADD_IPBINDS(ipbind->ib_server);

  return 0;
}

static int ctrls_handle_up(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  register unsigned int i = 0;

  /* Handle scheduled ups of virtual servers in the future, and
   * cancellations of scheduled ups.
   */

  /* Check the 'up' ACL */
  if (!pr_ctrls_check_acl(ctrl, ctrls_admin_acttab, "up")) {

    /* Access denied */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Sanity check */
  if (reqargc < 1 || reqargv == NULL) {
    pr_ctrls_add_response(ctrl, "up: missing required parameters");
    return -1;
  }

  for (i = 0; i < reqargc; i++) {
    unsigned int server_port = 21;
    char *server_str = reqargv[i], *tmp = NULL;
    pr_netaddr_t *server_addr = NULL;
    array_header *addrs = NULL;

    tmp = strchr(server_str, '#');
    if (tmp != NULL) {
      server_port = atoi(tmp + 1);
      *tmp = '\0';
    }

    server_addr = pr_netaddr_get_addr(ctrl->ctrls_tmp_pool, server_str, &addrs);
    if (!server_addr) {
      pr_ctrls_add_response(ctrl, "up: unable to resolve address for '%s'",
        server_str);
      return -1;
    }

    if (admin_addr_up(ctrl, server_addr, server_port) < 0)
      return -1;

    if (addrs) {
      register unsigned int j;
      pr_netaddr_t **elts = addrs->elts;

      for (j = 0; j < addrs->nelts; j++)
        if (admin_addr_up(ctrl, elts[j], server_port) < 0)
          return -1;
    }
  }

  return 0;
}

/* Configuration handlers
 */

/* usage: AdminControlsACLs actions|all allow|deny user|group list */
MODRET set_adminctrlsacls(cmd_rec *cmd) {
  char *bad_action = NULL, **actions = NULL;

  CHECK_ARGS(cmd, 4);
  CHECK_CONF(cmd, CONF_ROOT);

  /* We can cheat here, and use the ctrls_parse_acl() routine to
   * separate the given string...
   */
  actions = ctrls_parse_acl(cmd->tmp_pool, cmd->argv[1]);

  /* Check the second parameter to make sure it is "allow" or "deny" */
  if (strcmp(cmd->argv[2], "allow") != 0 &&
      strcmp(cmd->argv[2], "deny") != 0)
    CONF_ERROR(cmd, "second parameter must be 'allow' or 'deny'");

  /* Check the third parameter to make sure it is "user" or "group" */
  if (strcmp(cmd->argv[3], "user") != 0 &&
      strcmp(cmd->argv[3], "group") != 0)
    CONF_ERROR(cmd, "third parameter must be 'user' or 'group'");

  bad_action = pr_ctrls_set_module_acls(ctrls_admin_acttab, ctrls_admin_pool,
    actions, cmd->argv[2], cmd->argv[3], cmd->argv[4]);
  if (bad_action != NULL)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown action: '",
      bad_action, "'", NULL));

  return PR_HANDLED(cmd);
}

/* usage: AdminControlsEngine on|off|actions */
MODRET set_adminctrlsengine(cmd_rec *cmd) {
  int bool = -1;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if ((bool = get_boolean(cmd, 1)) != -1) {
    /* If bool is TRUE, there's no need to do anything.  If FALSE,
     * then unregister all the controls of this module.
     */
    if (!bool) {
      register unsigned int i = 0;

      for (i = 0; ctrls_admin_acttab[i].act_action; i++) {
        pr_ctrls_unregister(&ctrls_admin_module,
          ctrls_admin_acttab[i].act_action);
        destroy_pool(ctrls_admin_acttab[i].act_acl->acl_pool);
      }
    }

  } else {
    char *bad_action = NULL;

    /* Parse the given string of actions into a char **.  Then iterate
     * through the acttab, checking to see if a given control is _not_ in
     * the list.  If not in the list, unregister that control.
     */

    /* We can cheat here, and use the ctrls_parse_acl() routine to
     * separate the given string...
     */
    char **actions = ctrls_parse_acl(cmd->tmp_pool, cmd->argv[1]);

    bad_action = pr_ctrls_unregister_module_actions(ctrls_admin_acttab, actions,
      &ctrls_admin_module);
    if (bad_action != NULL)
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown action: '",
          bad_action, "'", NULL));
  }

  return PR_HANDLED(cmd);
}

/* Event handlers
 */

#if defined(PR_SHARED_MODULE)
static void ctrls_admin_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp("mod_ctrls_admin.c", (const char *) event_data) == 0) {
    register unsigned int i;

    pr_event_unregister(&ctrls_admin_module, NULL, NULL);

    for (i = 0; ctrls_admin_acttab[i].act_action; i++) {
      pr_ctrls_unregister(&ctrls_admin_module,
        ctrls_admin_acttab[i].act_action);
    }

    if (ctrls_admin_pool) {
      destroy_pool(ctrls_admin_pool);
      ctrls_admin_pool = NULL;
    }
  }
}
#endif /* PR_SHARED_MODULE */

static void ctrls_admin_restart_ev(const void *event_data, void *user_data) {
  register unsigned int i;

  if (ctrls_admin_pool)
    destroy_pool(ctrls_admin_pool);

  /* Allocate the pool for this module's use */
  ctrls_admin_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(ctrls_admin_pool, MOD_CTRLS_ADMIN_VERSION);

  /* Register the control handlers */
  for (i = 0; ctrls_admin_acttab[i].act_action; i++) {

    /* Allocate and initialize the ACL for this control. */
    ctrls_admin_acttab[i].act_acl = pcalloc(ctrls_admin_pool,
      sizeof(ctrls_acl_t));
    pr_ctrls_init_acl(ctrls_admin_acttab[i].act_acl);
  }

  ctrls_admin_nrestarts++;
  return;
}

static void ctrls_admin_startup_ev(const void *event_data, void *user_data) {
  int res;

  /* Make sure the process has an fd to the scoreboard. */
  PRIVS_ROOT
  res = pr_open_scoreboard(O_RDWR);
  PRIVS_RELINQUISH

  if (res < 0) {
    switch (res) {
      case PR_SCORE_ERR_BAD_MAGIC:
        pr_log_debug(DEBUG0, "error opening scoreboard: bad/corrupted file");
        break;

      case PR_SCORE_ERR_OLDER_VERSION:
        pr_log_debug(DEBUG0, "error opening scoreboard: bad version (too old)");
        break;

      case PR_SCORE_ERR_NEWER_VERSION:
        pr_log_debug(DEBUG0, "error opening scoreboard: bad version (too new)");
        break;

      default:
        pr_log_debug(DEBUG0, "error opening scoreboard: %s", strerror(errno));
        break;
    }
  }

  return;
}

/* Initialization routines
 */

static int ctrls_admin_init(void) {
  register unsigned int i = 0;

  /* Allocate the pool for this module's use */
  ctrls_admin_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(ctrls_admin_pool, MOD_CTRLS_ADMIN_VERSION);

  /* Register the control handlers */
  for (i = 0; ctrls_admin_acttab[i].act_action; i++) {

    /* Allocate and initialize the ACL for this control. */
    ctrls_admin_acttab[i].act_acl = pcalloc(ctrls_admin_pool,
      sizeof(ctrls_acl_t));
    pr_ctrls_init_acl(ctrls_admin_acttab[i].act_acl);

    if (pr_ctrls_register(&ctrls_admin_module,
        ctrls_admin_acttab[i].act_action, ctrls_admin_acttab[i].act_desc,
        ctrls_admin_acttab[i].act_cb) < 0)
     pr_log_pri(PR_LOG_INFO, MOD_CTRLS_ADMIN_VERSION
        ": error registering '%s' control: %s",
        ctrls_admin_acttab[i].act_action, strerror(errno));
  }

#if defined(PR_SHARED_MODULE)
  pr_event_register(&ctrls_admin_module, "core.module-unload",
    ctrls_admin_mod_unload_ev, NULL);
#endif /* PR_SHARED_MODULE */
  pr_event_register(&ctrls_admin_module, "core.restart",
    ctrls_admin_restart_ev, NULL);
  pr_event_register(&ctrls_admin_module, "core.startup",
    ctrls_admin_startup_ev, NULL);

  time(&ctrls_admin_start);
  return 0;
}

static ctrls_acttab_t ctrls_admin_acttab[] = {
  { "debug",    "set debugging level",		NULL,
    ctrls_handle_debug },
  { "dns",	"set UseReverseDNS configuration",	NULL,
    ctrls_handle_dns },
  { "down",     "disable an individual virtual server", NULL,
    ctrls_handle_down },
  { "get",      "list configuration data",	NULL,
    ctrls_handle_get },
  { "kick",	"disconnect a class, host, or user",	NULL,
    ctrls_handle_kick },
  { "restart",  "restart the daemon (similar to using HUP)",	NULL,
    ctrls_handle_restart },
  { "scoreboard", "clean the ScoreboardFile", NULL,
    ctrls_handle_scoreboard },
  { "shutdown", "shutdown the daemon",	NULL,
    ctrls_handle_shutdown },
  { "status",	"display status of servers",		NULL,
    ctrls_handle_status },
  { "trace",	"set trace levels",		NULL,
    ctrls_handle_trace },
  { "up",       "enable a downed virtual server",       NULL,
    ctrls_handle_up },
  { NULL, NULL,	NULL, NULL }
};

/* Module API tables
 */

static conftable ctrls_admin_conftab[] = {
  { "AdminControlsACLs",    	set_adminctrlsacls, 		NULL },
  { "AdminControlsEngine",	set_adminctrlsengine,		NULL },
  { NULL }
};

module ctrls_admin_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "ctrls_admin",

  /* Module configuration handler table */
  ctrls_admin_conftab,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  ctrls_admin_init,

  /* Session initialization function */
  NULL,

  /* Module version */
  MOD_CTRLS_ADMIN_VERSION
};
