/*
 * ProFTPD: mod_ident -- a module for performing identd lookups [RFC1413]
 *
 * Copyright (c) 2008-2009 The ProFTPD Project
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
 * $Id: mod_ident.c,v 1.4 2009/02/12 20:13:41 castaglia Exp $
 */

#include "conf.h"

#define MOD_IDENT_VERSION		"mod_ident/1.0"

#if PROFTPD_VERSION_NUMBER < 0x0001030101
# error "ProFTPD 1.3.1rc1 or later required"
#endif

module ident_module;

static int ident_engine = FALSE;
static pr_netio_stream_t *ident_nstrm = NULL;
static int ident_timeout_triggered = FALSE;

static const char *trace_channel = "ident";

/* Support routines
 */

static int ident_timeout_cb(CALLBACK_FRAME) {
  ident_timeout_triggered = TRUE;

  if (ident_nstrm) {
    /* Abort the NetIO stream, which will cause netio_pool (and thus
     * netio_read) to also abort.  This is similar to the way data transfers
     * are aborted.
     */
    pr_netio_abort(ident_nstrm);
  }

  return 0;
}

static char *ident_lookup(pool *p, conn_t *conn) {
  conn_t *ident_conn = NULL, *ident_io = NULL;
  char buf[256], *ident = NULL;
  int timerno, res = 0;
  int ident_port = pr_inet_getservport(p, "ident", "tcp");

  ident_nstrm = NULL;
  ident_timeout_triggered = FALSE;
  
  if (ident_port == -1) {
    return NULL;
  }

  timerno = pr_timer_add(PR_TUNABLE_TIMEOUTIDENT, -1, &ident_module,
    ident_timeout_cb, "ident (RFC1413) lookup");
  if (timerno <= 0) {
    pr_trace_msg(trace_channel, 8, "error adding timer: %s", strerror(errno));
    return NULL;
  }

  ident_conn = pr_inet_create_conn(p, NULL, -1, conn->local_addr, INPORT_ANY,
    FALSE);
  if (ident_conn == NULL) {
    pr_trace_msg(trace_channel, 3, "error creating connection: %s",
      strerror(errno));
    return NULL;
  }

  pr_inet_set_nonblock(p, ident_conn);

  res = pr_inet_connect_nowait(p, ident_conn, conn->remote_addr,
    ident_port);
  if (res < 0) {
    int xerrno = errno;

    pr_timer_remove(timerno, &ident_module);
    pr_inet_close(p, ident_conn);

    pr_trace_msg(trace_channel, 5, "connection to %s, port %d failed: %s",
      pr_netaddr_get_ipstr(conn->remote_addr), ident_port, strerror(xerrno));

    errno = xerrno;
    return NULL;
  }

  if (res == 0) {
    /* Not yet connected */
    ident_nstrm = pr_netio_open(p, PR_NETIO_STRM_OTHR,
      ident_conn->listen_fd, PR_NETIO_IO_RD);
    if (ident_nstrm == NULL) {
      pr_trace_msg(trace_channel, 5, "error opening NetIO stream: %s",
        strerror(errno));
      return NULL;
    }

    pr_netio_set_poll_interval(ident_nstrm, 1);

    switch (pr_netio_poll(ident_nstrm)) {
      case 1: {
        /* Aborted, timed out */
        if (ident_timeout_triggered) {
          pr_netio_close(ident_nstrm);
          ident_nstrm = NULL;

          pr_inet_close(p, ident_conn);
          pr_timer_remove(timerno, &ident_module);

          pr_trace_msg(trace_channel, 5, "ident lookup timed out after %u secs",
            PR_TUNABLE_TIMEOUTIDENT);
          return NULL;
        }

        break;
      }

      case -1: {
        /* Error */
        int xerrno = errno;

        pr_netio_close(ident_nstrm);
        ident_nstrm = NULL;

        pr_inet_close(p, ident_conn);
        pr_timer_remove(timerno, &ident_module);

        pr_trace_msg(trace_channel, 6, "ident lookup failed: %s",
          strerror(xerrno));

        errno = xerrno;
        return NULL;
      }

      default: {
        /* Connected */
        ident_conn->mode = CM_OPEN;

        if (pr_inet_get_conn_info(ident_conn, ident_conn->listen_fd) < 0) {
          int xerrno = errno;

          pr_netio_close(ident_nstrm);
          ident_nstrm = NULL;

          pr_inet_close(p, ident_conn);
          pr_timer_remove(timerno, &ident_module);

          pr_trace_msg(trace_channel, 3,
            "error retrieving ident peer details: %s", strerror(xerrno));

          errno = xerrno;
          return NULL;
        }

        break;
      }
    }
  }

  ident_io = pr_inet_openrw(p, ident_conn, NULL, PR_NETIO_STRM_OTHR,
    -1, -1, -1, FALSE);
  if (ident_io == NULL) {
    int xerrno = errno;

    pr_netio_close(ident_nstrm);
    ident_nstrm = NULL;

    pr_inet_close(p, ident_conn);
    pr_timer_remove(timerno, &ident_module);

    pr_trace_msg(trace_channel, 3, "failed opening read/write connection: %s",
      strerror(xerrno));

    errno = xerrno;
    return NULL;
  }

  ident_nstrm = ident_io->instrm;

  pr_inet_set_nonblock(p, ident_io);
  pr_netio_set_poll_interval(ident_io->instrm, 1);
  pr_netio_set_poll_interval(ident_io->outstrm, 1);

  pr_netio_printf(ident_io->outstrm, "%d, %d\r\n", conn->remote_port,
    conn->local_port);

  pr_trace_msg(trace_channel, 4, "reading response from ident server at %s",
    pr_netaddr_get_ipstr(conn->remote_addr));

  /* If the timer fires while in netio_gets(), netio_gets() will simply return
   * either a partial string, or NULL.  This works because ident_timeout_cb()
   * aborts the stream from which we are reading.  netio_set_poll_interval()
   * is used to make sure significant delays do not occur on systems that
   * automatically restart system calls after the SIGALRM signal.
   */

  if (pr_netio_gets(buf, sizeof(buf), ident_io->instrm)) {
    char *tok = NULL, *tmp = NULL;

    pr_str_strip_end(buf, "\r\n");

    pr_trace_msg(trace_channel, 6, "received '%s' from ident server", buf);

    tmp = buf;
    tok = pr_str_get_token(&tmp, ":");
    if (tok &&
        (tok = pr_str_get_token(&tmp, ":"))) {
      while (*tok && isspace((int) *tok)) {
        pr_signals_handle();
        tok++;
      }

      pr_str_strip_end(tok, " \t");

      if (strcasecmp(tok, "ERROR") == 0) {
        if (tmp) {
          while (*tmp && isspace((int) *tmp)) {
            pr_signals_handle();
            tmp++;
          }

          pr_str_strip_end(tmp, " \t");

          if (strcasecmp(tmp, "HIDDEN-USER") == 0)
            ident = "HIDDEN-USER";
        }

      } else if (strcasecmp(tok, "USERID") == 0) {
        if (tmp &&
            (tok = pr_str_get_token(&tmp, ":"))) {
          if (tmp) {
            while (*tmp && isspace((int) *tmp)) {
              pr_signals_handle();
              tmp++;
            }

            pr_str_strip_end(tmp, " \t");
            ident = tmp;
          }
        }
      }
    }
  }

  pr_inet_close(p, ident_io);
  pr_inet_close(p, ident_conn);
  pr_timer_remove(timerno, &ident_module);

  return pstrdup(p, ident);
}

/* Configuration handlers
 */

/* usage: IdentLookups on|off */
MODRET set_identlookups(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

/* Initialization functions
 */

static int ident_sess_init(void) {
  pool *tmp_pool = NULL;
  config_rec *c;
  char *ident = NULL;

  c = find_config(main_server->conf, CONF_PARAM, "IdentLookups", FALSE);
  if (c)
    ident_engine = *((int *) c->argv[0]);

  if (!ident_engine) {
    pr_log_debug(DEBUG6, MOD_IDENT_VERSION ": ident lookup disabled");
    return 0;
  }

  tmp_pool = make_sub_pool(session.pool);

  /* Perform the RFC1413 lookup */
  pr_log_debug(DEBUG6, MOD_IDENT_VERSION ": performing ident lookup");

  ident = ident_lookup(tmp_pool, session.c);
  if (ident) {
    pr_log_debug(DEBUG6, MOD_IDENT_VERSION ": ident lookup returned '%s'",
      ident);

  } else {
    ident = "UNKNOWN";
    pr_log_debug(DEBUG6, MOD_IDENT_VERSION ": ident lookup failed, using '%s'",
      ident);
  }

  /* Stash the identity in session.notes, for later retrieval by the
   * TransferLog code.
   */
  if (pr_table_add_dup(session.notes, "mod_ident.rfc1413-ident",
      ident, 0) < 0) {
    pr_log_debug(DEBUG3, MOD_IDENT_VERSION
      ": error stashing 'mod_ident.rfc1413-ident' value '%s': %s", ident,
      strerror(errno));
  }

  destroy_pool(tmp_pool);
  return 0;
}

/* Module API tables
 */

static conftable ident_conftab[] = {
  { "IdentLookups",	set_identlookups,	NULL },
  { NULL }
};

module ident_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "ident",

  /* Module configuration handler table */
  ident_conftab,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  NULL,

  /* Session initialization function */
  ident_sess_init,

  /* Module version */
  MOD_IDENT_VERSION
};
