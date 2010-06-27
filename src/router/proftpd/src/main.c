/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2010 The ProFTPD Project team
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
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/*
 * House initialization and main program loop
 * $Id: main.c,v 1.391 2010/02/04 17:14:47 castaglia Exp $
 */

#include "conf.h"

#include <signal.h>

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#ifdef HAVE_LIBUTIL_H
# include <libutil.h>
#endif /* HAVE_LIBUTIL_H */

#ifdef HAVE_REGEX_H
# include <regex.h>
#endif

#ifdef HAVE_REGEXP_H
# include <regexp.h>
#endif /* HAVE_REGEXP_H */

#ifdef HAVE_EXECINFO_H
# include <execinfo.h>
#endif

#ifdef HAVE_UNAME
# include <sys/utsname.h>
#endif

#ifdef HAVE_UCONTEXT_H
# include <ucontext.h>
#endif

#include "privs.h"

int (*cmd_auth_chk)(cmd_rec *);
void (*cmd_handler)(server_rec *, conn_t *);

#ifdef NEED_PERSISTENT_PASSWD
unsigned char persistent_passwd = TRUE;
#else
unsigned char persistent_passwd = FALSE;
#endif /* NEED_PERSISTENT_PASSWD */

/* From modules/module_glue.c */
extern module *static_modules[];

extern xaset_t *server_list;

unsigned long max_connects = 0UL;
unsigned int max_connect_interval = 1;

session_t session;

/* Is this process the master standalone daemon process? */
unsigned char is_master = TRUE;

pid_t mpid = 0;				/* Master pid */

uid_t daemon_uid;
gid_t daemon_gid;
array_header *daemon_gids;

static time_t shut = 0, deny = 0, disc = 0;
static char shutmsg[81] = {'\0'};

static unsigned char have_dead_child = FALSE;

/* The default command buffer size SHOULD be large enough to handle the
 * maximum path length, plus 4 bytes for the FTP command, plus 1 for the
 * whitespace separating command from path, and 2 for the terminating CRLF.
 */
#define PR_DEFAULT_CMD_BUFSZ	(PR_TUNABLE_PATH_MAX + 7)

/* From mod_auth_unix.c */
extern unsigned char persistent_passwd;

/* From response.c */
extern pr_response_t *resp_list, *resp_err_list;

static int nodaemon  = 0;
static int quiet     = 0;
static int shutdownp = 0;
static int syntax_check = 0;

/* Command handling */
static void cmd_loop(server_rec *, conn_t *);

/* Signal handling */
static RETSIGTYPE sig_disconnect(int);
static RETSIGTYPE sig_evnt(int);
static RETSIGTYPE sig_terminate(int);
#ifdef PR_DEVEL_STACK_TRACE
static void install_segv_handler(void);
#endif /* PR_DEVEL_STACK_TRACE */

volatile unsigned int recvd_signal_flags = 0;

/* Used to capture an "unknown" signal value that causes termination. */
static int term_signo = 0;

/* Signal processing functions */
static void handle_abort(void);
static void handle_chld(void);
static void handle_evnt(void);
#ifdef PR_DEVEL_STACK_TRACE
static void handle_segv(int, siginfo_t *, void *);
#endif /* PR_DEVEL_STACK_TRACE */
static void handle_xcpu(void);
static void handle_terminate(void);
static void handle_terminate_other(void);
static void finish_terminate(void);

static cmd_rec *make_ftp_cmd(pool *, char *, int);

static const char *config_filename = PR_CONFIG_FILE_PATH;

/* Add child semaphore fds into the rfd for selecting */
static int semaphore_fds(fd_set *rfd, int maxfd) {

  if (child_count()) {
    pr_child_t *ch;

    for (ch = child_get(NULL); ch; ch = child_get(ch)) {
      if (ch->ch_pipefd != -1) {
        FD_SET(ch->ch_pipefd, rfd);
        if (ch->ch_pipefd > maxfd)
          maxfd = ch->ch_pipefd;
      }
    }
  }

  return maxfd;
}

void set_auth_check(int (*chk)(cmd_rec*)) {
  cmd_auth_chk = chk;
}

void pr_cmd_set_handler(void (*handler)(server_rec *, conn_t *)) {
  if (handler == NULL) {
    cmd_handler = cmd_loop;

  } else {
    cmd_handler = handler;
  }
}

static void end_login_noexit(void) {

  /* Clear the scoreboard entry. */
  if (ServerType == SERVER_STANDALONE) {

    /* For standalone daemons, we only clear the scoreboard slot if we are
     * an exiting child process.
     */

    if (!is_master) {
      if (pr_scoreboard_entry_del(TRUE) < 0 &&
          errno != EINVAL &&
          errno != ENOENT) {
        pr_log_debug(DEBUG1, "error deleting scoreboard entry: %s",
          strerror(errno));
      }
    }

  } else if (ServerType == SERVER_INETD) {
    /* For inetd-spawned daemons, we always clear the scoreboard slot. */
    if (pr_scoreboard_entry_del(TRUE) < 0 &&
        errno != EINVAL &&
        errno != ENOENT) {
      pr_log_debug(DEBUG1, "error deleting scoreboard entry: %s",
        strerror(errno));
    }
  }

  /* If session.user is set, we have a valid login. */
  if (session.user &&
      session.wtmp_log) {
    const char *sess_ttyname;

    sess_ttyname = pr_session_get_ttyname(session.pool);
    log_wtmp(sess_ttyname, "", pr_netaddr_get_sess_remote_name(),
      pr_netaddr_get_sess_remote_addr());
  }

  /* These are necessary in order that cleanups associated with these pools
   * (and their subpools) are properly run.
   */
  if (session.d) {
    pr_inet_close(session.pool, session.d);
    session.d = NULL;
  }

  if (session.c) {
    pr_inet_close(session.pool, session.c);
    session.c = NULL;
  }

  /* Run all the exit handlers */
  pr_event_generate("core.exit", NULL);

  if (!is_master ||
      (ServerType == SERVER_INETD && !syntax_check)) {
    pr_log_pri(PR_LOG_INFO, "%s session closed.",
      pr_session_get_protocol(PR_SESS_PROTO_FL_LOGOUT));
  }

  log_closesyslog();
}

/* Finish any cleaning up, mark utmp as closed and exit without flushing
 * buffers
 */
void end_login(int exitcode) {
  end_login_noexit();

#ifdef PR_USE_DEVEL
  destroy_pool(session.pool);

  if (is_master) {
    main_server = NULL;
    free_pools();
    pr_proctitle_free();
  }
#endif /* PR_USE_DEVEL */

#ifdef PR_DEVEL_PROFILE
  /* Populating the gmon.out gprof file requires that the process exit
   * via exit(2) or by returning from main().  Using _exit(2) doesn't allow
   * the process the time to write its profile data out.
   */
  exit(exitcode);
#else
  _exit(exitcode);
#endif /* PR_DEVEL_PROFILE */
}

void session_exit(int pri, void *lv, int exitval, void *dummy) {
  char *msg = (char *) lv;

  pr_log_pri(pri, "%s", msg);

  if (ServerType == SERVER_STANDALONE &&
      is_master) {
    pr_log_pri(PR_LOG_NOTICE, "ProFTPD " PROFTPD_VERSION_TEXT
      " standalone mode SHUTDOWN");

    PRIVS_ROOT
    pr_delete_scoreboard();
    if (!nodaemon)
      pr_pidfile_remove();
    PRIVS_RELINQUISH
  }

  end_login(exitval);
}

static void shutdown_exit(void *d1, void *d2, void *d3, void *d4) {
  if (check_shutmsg(&shut, &deny, &disc, shutmsg, sizeof(shutmsg)) == 1) {
    char *user;
    time_t now;
    char *msg;
    const char *serveraddress;
    config_rec *c = NULL;
    unsigned char *authenticated = get_param_ptr(main_server->conf,
      "authenticated", FALSE);

    serveraddress = (session.c && session.c->local_addr) ?
      pr_netaddr_get_ipstr(session.c->local_addr) :
      main_server->ServerAddress;

    if ((c = find_config(main_server->conf, CONF_PARAM, "MasqueradeAddress",
        FALSE)) != NULL) {

      pr_netaddr_t *masq_addr = (pr_netaddr_t *) c->argv[0];
      serveraddress = pr_netaddr_get_ipstr(masq_addr);
    }

    time(&now);
    if (authenticated && *authenticated == TRUE) {
      user = pr_table_get(session.notes, "mod_auth.orig-user", NULL);

    } else {
      user = "NONE";
    }

    msg = sreplace(permanent_pool, shutmsg,
                   "%s", pstrdup(permanent_pool, pr_strtime(shut)),
                   "%r", pstrdup(permanent_pool, pr_strtime(deny)),
                   "%d", pstrdup(permanent_pool, pr_strtime(disc)),
		   "%C", (session.cwd[0] ? session.cwd : "(none)"),
		   "%L", serveraddress,
		   "%R", (session.c && session.c->remote_name ?
                         session.c->remote_name : "(unknown)"),
		   "%T", pstrdup(permanent_pool, pr_strtime(now)),
		   "%U", user,
		   "%V", main_server->ServerName,
                   NULL );

    pr_response_send_async(R_421, _("FTP server shutting down - %s"), msg);
    session_exit(PR_LOG_NOTICE, msg, 0, NULL);
  }

  signal(SIGUSR1, sig_disconnect);
}

static int get_command_class(const char *name) {
  int idx = -1;
  cmdtable *c = pr_stash_get_symbol(PR_SYM_CMD, name, NULL, &idx);

  while (c && c->cmd_type != CMD) {
    pr_signals_handle();
    c = pr_stash_get_symbol(PR_SYM_CMD, name, c, &idx);
  }

  /* By default, every command has a class of CL_ALL.  This insures that
   * any configured ExtendedLogs that default to "all" will log the command.
   */
  return (c ? c->class : CL_ALL);
}

static int _dispatch(cmd_rec *cmd, int cmd_type, int validate, char *match) {
  char *cmdargstr = NULL;
  cmdtable *c;
  modret_t *mr;
  int success = 0;
  int send_error = 0;
  static int match_index_cache = -1;
  static char *last_match = NULL;
  int *index_cache;

  send_error = (cmd_type == PRE_CMD || cmd_type == CMD ||
    cmd_type == POST_CMD_ERR);

  if (!match) {
    match = cmd->argv[0];
    index_cache = &cmd->stash_index;

  } else {
    if (last_match != match) {
      match_index_cache = -1;
      last_match = match;
    }

    index_cache = &match_index_cache;
  }

  c = pr_stash_get_symbol(PR_SYM_CMD, match, NULL, index_cache);

  while (c && !success) {
    pr_signals_handle();

    session.curr_cmd = cmd->argv[0];
    session.curr_phase = cmd_type;

    if (c->cmd_type == cmd_type) {
      if (c->group)
        cmd->group = pstrdup(cmd->pool, c->group);

      if (c->requires_auth &&
          cmd_auth_chk &&
          !cmd_auth_chk(cmd)) {
        pr_trace_msg("command", 8,
          "command '%s' failed 'requires_auth' check for mod_%s.c",
          cmd->argv[0], c->m->name);
        return -1;
      }

      if (cmd->tmp_pool == NULL) {
        cmd->tmp_pool = make_sub_pool(cmd->pool);
        pr_pool_tag(cmd->tmp_pool, "cmd_rec tmp pool");
      }

      cmdargstr = pr_cmd_get_displayable_str(cmd);

      if (cmd_type == CMD) {

        /* The client has successfully authenticated... */
        if (session.user) {
          char *args = strchr(cmdargstr, ' ');

          pr_scoreboard_entry_update(session.pid,
            PR_SCORE_CMD, "%s", cmd->argv[0], NULL, NULL);
          pr_scoreboard_entry_update(session.pid,
            PR_SCORE_CMD_ARG, "%s", args ? (args + 1) : "", NULL, NULL);

          pr_proctitle_set("%s - %s: %s", session.user, session.proc_prefix,
            cmdargstr);

        /* ...else the client has not yet authenticated */
        } else {
          pr_proctitle_set("%s:%d: %s", session.c->remote_addr ?
            pr_netaddr_get_ipstr(session.c->remote_addr) : "?",
            session.c->remote_port ? session.c->remote_port : 0, cmdargstr);
        }
      }

      pr_log_debug(DEBUG4, "dispatching %s command '%s' to mod_%s",
        (cmd_type == PRE_CMD ? "PRE_CMD" :
         cmd_type == CMD ? "CMD" :
         cmd_type == POST_CMD ? "POST_CMD" :
         cmd_type == POST_CMD_ERR ? "POST_CMD_ERR" :
         cmd_type == LOG_CMD ? "LOG_CMD" :
         cmd_type == LOG_CMD_ERR ? "LOG_CMD_ERR" :
         "(unknown)"),
        cmdargstr, c->m->name);

      pr_trace_msg("command", 7, "dispatching %s command '%s' to mod_%s.c",
        (cmd_type == PRE_CMD ? "PRE_CMD" :
         cmd_type == CMD ? "CMD" :
         cmd_type == POST_CMD ? "POST_CMD" :
         cmd_type == POST_CMD_ERR ? "POST_CMD_ERR" :
         cmd_type == LOG_CMD ? "LOG_CMD" :
         cmd_type == LOG_CMD_ERR ? "LOG_CMD_ERR" :
         "(unknown)"),
        cmdargstr, c->m->name);

      cmd->class |= c->class;

      /* KLUDGE: disable umask() for not G_WRITE operations.  Config/
       * Directory walking code will be completely redesigned in 1.3,
       * this is only necessary for perfomance reasons in 1.1/1.2
       */

      if (!c->group || strcmp(c->group, G_WRITE) != 0)
        kludge_disable_umask();
      mr = pr_module_call(c->m, c->handler, cmd);
      kludge_enable_umask();

      if (MODRET_ISHANDLED(mr)) {
        success = 1;

      } else if (MODRET_ISERROR(mr)) {
        if (cmd_type == POST_CMD ||
            cmd_type == LOG_CMD ||
            cmd_type == LOG_CMD_ERR) {
          if (MODRET_ERRMSG(mr))
            pr_log_pri(PR_LOG_NOTICE, "%s", MODRET_ERRMSG(mr));

        } else if (send_error) {
          if (MODRET_ERRNUM(mr) &&
              MODRET_ERRMSG(mr)) {
            pr_response_add_err(MODRET_ERRNUM(mr), "%s", MODRET_ERRMSG(mr));

          } else if (MODRET_ERRMSG(mr)) {
            pr_response_send_raw("%s", MODRET_ERRMSG(mr));
          }
        }

        success = -1;
      }

      if (session.user &&
          !(session.sf_flags & SF_XFER) &&
          cmd_type == CMD) {
        pr_session_set_idle();
      }

      destroy_pool(cmd->tmp_pool);
      cmd->tmp_pool = NULL;
    }

    if (!success) {
      c = pr_stash_get_symbol(PR_SYM_CMD, match, c, index_cache);
    }
  }

  if (!c &&
      !success &&
      validate) {
    char *method;

    /* Prettify the command method, if need be. */
    if (strchr(cmd->argv[0], '_') == NULL) {
      method = cmd->argv[0];

    } else {
      register unsigned int i;

      method = pstrdup(cmd->pool, cmd->argv[0]);
      for (i = 0; method[i]; i++) {
        if (method[i] == '_')
          method[i] = ' ';
      }
    }

    pr_response_add_err(R_500, _("%s not understood"), method);
    success = -1;
  }

  return success;
}

/* Returns the appropriate maximum buffer length to use for FTP commands
 * from the client, taking the CommandBufferSize directive into account.
 */
static long get_max_cmd_len(size_t buflen) {
  long res;
  int *bufsz = NULL;
  size_t default_cmd_bufsz;

  /* It's possible for the admin to select a PR_TUNABLE_BUFFER_SIZE which
   * is smaller than PR_DEFAULT_CMD_BUFSZ.  We need to handle such cases
   * properly.
   */
  default_cmd_bufsz = PR_DEFAULT_CMD_BUFSZ;
  if (default_cmd_bufsz > buflen) {
    default_cmd_bufsz = buflen;
  }
 
  bufsz = get_param_ptr(main_server->conf, "CommandBufferSize", FALSE);
  if (bufsz == NULL) {
    res = default_cmd_bufsz;

  } else if (*bufsz <= 0) {
    pr_log_pri(PR_LOG_WARNING, "invalid CommandBufferSize size (%d) given, "
      "using default buffer size (%lu) instead", *bufsz,
      (unsigned long) default_cmd_bufsz);
    res = default_cmd_bufsz;

  } else if (*bufsz + 1 > buflen) {
    pr_log_pri(PR_LOG_WARNING, "invalid CommandBufferSize size (%d) given, "
      "using default buffer size (%lu) instead", *bufsz,
      (unsigned long) default_cmd_bufsz);
    res = default_cmd_bufsz;

  } else {
    pr_log_debug(DEBUG1, "setting CommandBufferSize to %d", *bufsz);
    res = (long) *bufsz;
  }

  return res;
}

int pr_cmd_read(cmd_rec **res) {
  static long cmd_bufsz = -1;
  char buf[PR_DEFAULT_CMD_BUFSZ+1] = {'\0'};
  char *cp;
  size_t buflen;

  if (res == NULL) {
    errno = EINVAL;
    return -1;
  }

  while (TRUE) {
    pr_signals_handle();

    memset(buf, '\0', sizeof(buf));

    if (pr_netio_telnet_gets(buf, sizeof(buf)-1, session.c->instrm,
        session.c->outstrm) == NULL) {

      if (errno == E2BIG) {
        /* The client sent a too-long command which was ignored; give
         * them another chance?
         */
        continue;
      }

      if (session.c->instrm->strm_errno == 0) {
        pr_trace_msg("command", 6,
          "client sent EOF, closing control connection");
      }

      return -1;
    }

    break;
  }

  if (cmd_bufsz == -1)
    cmd_bufsz = get_max_cmd_len(sizeof(buf));

  /* This strlen(3) is guaranteed to terminate; the last byte of buf is
   * always NUL, since pr_netio_telnet_gets() is told that the buf size is
   * one byte less than it really is.
   *
   * If the strlen(3) says that the length is less than the cmd_bufsz, then
   * there is no need to truncate the buffer by inserting a NUL.
   */
  buflen = strlen(buf);
  if (buflen > (cmd_bufsz - 1)) {
    pr_log_debug(DEBUG0, "truncating incoming command length (%lu bytes) to "
      "CommandBufferSize %lu; use the CommandBufferSize directive to increase "
      "the allowed command length", (unsigned long) buflen,
      (unsigned long) cmd_bufsz);
    buf[cmd_bufsz - 1] = '\0';
  }

  if (buflen &&
      (buf[buflen-1] == '\n' || buf[buflen-1] == '\r')) {
    buf[buflen-1] = '\0';
    buflen--;

    if (buflen &&
        (buf[buflen-1] == '\n' || buf[buflen-1] =='\r'))
      buf[buflen-1] = '\0';
  }

  cp = buf;
  if (*cp == '\r')
    cp++;

  if (*cp) {
    int flags = 0;
    cmd_rec *cmd;

    /* If this is a SITE command, preserve embedded whitespace in the
     * command parameters, in order to handle file names that have multiple
     * spaces in the names.  Arguably this should be handled in the SITE
     * command handlers themselves, via cmd->arg.  This small hack
     * reduces the burden on SITE module developers, however.
     */
    if (strncasecmp(cp, C_SITE, 4) == 0)
      flags |= PR_STR_FL_PRESERVE_WHITESPACE;

    cmd = make_ftp_cmd(session.pool, cp, flags);

    if (cmd) {
      *res = cmd;
    } 
  }

  return 0;
}

int pr_cmd_dispatch_phase(cmd_rec *cmd, int phase, int flags) {
  char *cp = NULL;
  int success = 0;
  pool *resp_pool = NULL;

  cmd->server = main_server;

  if (flags & PR_CMD_DISPATCH_FL_CLEAR_RESPONSE) {
    resp_list = resp_err_list = NULL;
  }

  /* Get any previous pool that may be being used by the Response API.
   *
   * In most cases, this will be NULL.  However, if proftpd is in the
   * midst of a data transfer when a command comes in on the control
   * connection, then the pool in use will be that of the data transfer
   * instigating command.  We want to stash that pool, so that after this
   * command is dispatched, we can return the pool of the old command.
   * Otherwise, Bad Things (segfaults) happen.
   */
  resp_pool = pr_response_get_pool();

  /* Set the pool used by the Response API for this command. */
  pr_response_set_pool(cmd->pool);

  for (cp = cmd->argv[0]; *cp; cp++)
    *cp = toupper(*cp);

  if (!cmd->class)
    cmd->class = get_command_class(cmd->argv[0]);

  if (phase == 0) {
        
    /* First, dispatch to wildcard PRE_CMD handlers. */
    success = _dispatch(cmd, PRE_CMD, FALSE, C_ANY);

    if (!success)	/* run other pre_cmd */
      success = _dispatch(cmd, PRE_CMD, FALSE, NULL);

    if (success < 0) {

      /* Dispatch to POST_CMD_ERR handlers as well. */

      _dispatch(cmd, POST_CMD_ERR, FALSE, C_ANY);
      _dispatch(cmd, POST_CMD_ERR, FALSE, NULL);

      _dispatch(cmd, LOG_CMD_ERR, FALSE, C_ANY);
      _dispatch(cmd, LOG_CMD_ERR, FALSE, NULL);

      pr_response_flush(&resp_err_list);
      return success;
    }

    success = _dispatch(cmd, CMD, FALSE, C_ANY);

    if (!success)
      success = _dispatch(cmd, CMD, TRUE, NULL);

    if (success == 1) {
      success = _dispatch(cmd, POST_CMD, FALSE, C_ANY);
      if (!success)
        success = _dispatch(cmd, POST_CMD, FALSE, NULL);

      _dispatch(cmd, LOG_CMD, FALSE, C_ANY);
      _dispatch(cmd, LOG_CMD, FALSE, NULL);

      pr_response_flush(&resp_list);

    } else if (success < 0) {

      /* Allow for non-logging command handlers to be run if CMD fails. */

      success = _dispatch(cmd, POST_CMD_ERR, FALSE, C_ANY);
      if (!success)
        success = _dispatch(cmd, POST_CMD_ERR, FALSE, NULL);

      _dispatch(cmd, LOG_CMD_ERR, FALSE, C_ANY);
      _dispatch(cmd, LOG_CMD_ERR, FALSE, NULL);

      pr_response_flush(&resp_err_list);
    }

  } else {
    switch (phase) {
      case PRE_CMD:
      case POST_CMD:
      case POST_CMD_ERR:
        success = _dispatch(cmd, phase, FALSE, C_ANY);
        if (!success)
          success = _dispatch(cmd, phase, FALSE, NULL);
        break;

      case CMD:
        success = _dispatch(cmd, phase, FALSE, C_ANY);
        if (!success)
          success = _dispatch(cmd, phase, TRUE, NULL);
        break;

      case LOG_CMD:
      case LOG_CMD_ERR:
        (void) _dispatch(cmd, phase, FALSE, C_ANY);
        (void) _dispatch(cmd, phase, FALSE, NULL);
        break;

      default:
        errno = EINVAL;
        return -1;
    }

    if (flags & PR_CMD_DISPATCH_FL_SEND_RESPONSE) {
      if (success == 1) {
        pr_response_flush(&resp_list);

      } else if (success < 0) {
        pr_response_flush(&resp_err_list);
      }
    }
  }

  /* Restore any previous pool to the Response API. */
  pr_response_set_pool(resp_pool);

  return success;
}

int pr_cmd_dispatch(cmd_rec *cmd) {
  return pr_cmd_dispatch_phase(cmd, 0,
    PR_CMD_DISPATCH_FL_SEND_RESPONSE|PR_CMD_DISPATCH_FL_CLEAR_RESPONSE);
}

static cmd_rec *make_ftp_cmd(pool *p, char *buf, int flags) {
  char *cp = buf, *wrd;
  cmd_rec *cmd;
  pool *subpool;
  array_header *tarr;
  int str_flags = PR_STR_FL_PRESERVE_COMMENTS|flags;

  /* Be pedantic (and RFC-compliant) by not allowing leading whitespace
   * in an issued FTP command.  Will this cause troubles with many clients?
   */
  if (isspace((int) buf[0]))
    return NULL;

  /* Nothing there...bail out. */
  wrd = pr_str_get_word(&cp, str_flags);
  if (wrd == NULL)
    return NULL;

  subpool = make_sub_pool(p);
  cmd = (cmd_rec *) pcalloc(subpool, sizeof(cmd_rec));
  cmd->pool = subpool;
  cmd->tmp_pool = NULL;
  cmd->stash_index = -1;

  tarr = make_array(cmd->pool, 2, sizeof(char *));

  *((char **) push_array(tarr)) = pstrdup(cmd->pool, wrd);
  cmd->argc++;
  cmd->arg = pstrdup(cmd->pool, cp);

  while ((wrd = pr_str_get_word(&cp, str_flags)) != NULL) {
    *((char **) push_array(tarr)) = pstrdup(cmd->pool, wrd);
    cmd->argc++;
  }

  *((char **) push_array(tarr)) = NULL;
  cmd->argv = (char **) tarr->elts;

  /* This table will not contain that many entries, so a low number
   * of chains should suffice.
   */
  cmd->notes = pr_table_nalloc(cmd->pool, 0, 8);
  return cmd;
}

static void send_session_banner(server_rec *server) {
  config_rec *c = NULL;
  char *display = NULL;
  const char *serveraddress = NULL;
  config_rec *masq = NULL;

  display = get_param_ptr(server->conf, "DisplayConnect", FALSE);
  if (display != NULL) {
    if (pr_display_file(display, NULL, R_220) < 0) {
      pr_log_debug(DEBUG6, "unable to display DisplayConnect file '%s': %s",
        display, strerror(errno));
    }
  }

  serveraddress = pr_netaddr_get_ipstr(session.c->local_addr);

  masq = find_config(server->conf, CONF_PARAM, "MasqueradeAddress", FALSE);
  if (masq != NULL) {
    pr_netaddr_t *masq_addr = (pr_netaddr_t *) masq->argv[0];
    serveraddress = pr_netaddr_get_ipstr(masq_addr);
  }

  c = find_config(server->conf, CONF_PARAM, "ServerIdent", FALSE);
  if (c == NULL ||
      *((unsigned char *) c->argv[0]) == FALSE) {
    unsigned char *defer_welcome = get_param_ptr(main_server->conf,
      "DeferWelcome", FALSE);

    if (c &&
        c->argc > 1) {
      char *server_ident = c->argv[1];

      if (strstr(server_ident, "%L") != NULL) {
        server_ident = sreplace(session.pool, server_ident, "%L",
          pr_netaddr_get_ipstr(session.c->local_addr), NULL);
      }

      if (strstr(server_ident, "%V") != NULL) {
        server_ident = sreplace(session.pool, server_ident, "%V",
          main_server->ServerFQDN, NULL);
      }

      if (strstr(server_ident, "%v") != NULL) {
        server_ident = sreplace(session.pool, server_ident, "%v",
          main_server->ServerName, NULL);
      }

      pr_response_send(R_220, "%s", server_ident);

    } else if (defer_welcome &&
               *defer_welcome == TRUE) {
      pr_response_send(R_220, "ProFTPD " PROFTPD_VERSION_TEXT
        " Server ready.");

    } else {
      pr_response_send(R_220, "ProFTPD " PROFTPD_VERSION_TEXT
        " Server (%s) [%s]", server->ServerName, serveraddress);
    }

  } else {
    pr_response_send(R_220, _("%s FTP server ready"), serveraddress);
  }
}

static void cmd_loop(server_rec *server, conn_t *c) {

  while (TRUE) {
    int res = 0; 
    cmd_rec *cmd = NULL;

    pr_signals_handle();

    res = pr_cmd_read(&cmd);
    if (res < 0) {
      if (PR_NETIO_ERRNO(session.c->instrm) == EINTR)
        /* Simple interrupted syscall */
        continue;

#ifndef PR_DEVEL_NO_DAEMON
      /* Otherwise, EOF */
      end_login(0);
#else
      return;
#endif /* PR_DEVEL_NO_DAEMON */
    }

    /* Data received, reset idle timer */
    if (pr_data_get_timeout(PR_DATA_TIMEOUT_IDLE) > 0) {
      pr_timer_reset(PR_TIMER_IDLE, ANY_MODULE);
    }

    if (cmd) {
      pr_cmd_dispatch(cmd);
      destroy_pool(cmd->pool);

    } else {
      pr_response_send(R_500, _("Invalid command: try being more creative"));
    }

    /* Release any working memory allocated in inet */
    pr_inet_clear();
  }
}

static void core_restart_cb(void *d1, void *d2, void *d3, void *d4) {
  if (is_master && mpid) {
    int maxfd;
    fd_set childfds;

    pr_log_pri(PR_LOG_NOTICE, "received SIGHUP -- master server reparsing "
      "configuration file");

    /* Make sure none of our children haven't completed start up */
    FD_ZERO(&childfds);
    maxfd = -1;

    maxfd = semaphore_fds(&childfds, maxfd);
    if (maxfd > -1) {
      pr_log_pri(PR_LOG_NOTICE, "waiting for child processes to complete "
        "initialization");

      while (maxfd != -1) {
	int i;
	
	i = select(maxfd + 1, &childfds, NULL, NULL, NULL);

        if (i > 0) {
          pr_child_t *ch;

          for (ch = child_get(NULL); ch; ch = child_get(ch)) {
            if (ch->ch_pipefd != -1 &&
               FD_ISSET(ch->ch_pipefd, &childfds)) {
              close(ch->ch_pipefd);
              ch->ch_pipefd = -1;
            }
          }
        }

	FD_ZERO(&childfds);
        maxfd = -1;
	maxfd = semaphore_fds(&childfds, maxfd);
      }
    }

    free_bindings();

    /* Run through the list of registered restart callbacks. */
    pr_event_generate("core.restart", NULL);

    init_log();
    init_netaddr();
    init_class();
    init_config();

#ifdef PR_USE_NLS
    encode_free();
#endif /* PR_USE_NLS */

    pr_netaddr_clear_cache();

    pr_parser_prepare(NULL, NULL);

    PRIVS_ROOT
    if (pr_parser_parse_file(NULL, config_filename, NULL, 0) == -1) {
      PRIVS_RELINQUISH
      pr_log_pri(PR_LOG_ERR,
        "Fatal: unable to read configuration file '%s': %s",
        config_filename, strerror(errno));
      end_login(1);
    }
    PRIVS_RELINQUISH

    if (pr_parser_cleanup() < 0) {
      pr_log_pri(PR_LOG_ERR, "Fatal: error processing configuration file '%s': "
       "unclosed configuration section", config_filename);
      end_login(1);
    }

#ifdef PR_USE_NLS
    encode_init();
#endif /* PR_USE_NLS */

    /* After configuration is complete, make sure that passwd, group
     * aren't held open (unnecessary fds for master daemon)
     */
    endpwent();
    endgrent();

    /* Set the (possibly new) resource limits. */
    set_daemon_rlimits();

    if (fixup_servers(server_list) < 0) {
      pr_log_pri(PR_LOG_ERR, "Fatal: error processing configuration file '%s'",
        config_filename);
      end_login(1);
    }

    pr_event_generate("core.postparse", NULL);

    /* Recreate the listen connection.  Can an inetd-spawned server accept
     * and process HUP?
     */
    init_bindings();

  } else {

    /* Child process -- cannot restart, log error */
    pr_log_pri(PR_LOG_ERR, "received SIGHUP, cannot restart child process");
  }
}

#ifndef PR_DEVEL_NO_FORK
static int dup_low_fd(int fd) {
  int i,need_close[3] = {-1, -1, -1};

  for (i = 0; i < 3; i++)
    if (fd == i) {
      fd = dup(fd);
      need_close[i] = 1;
    }

  for (i = 0; i < 3; i++)
    if (need_close[i] > -1)
      close(i);

  return fd;
}
#endif /* PR_DEVEL_NO_FORK */

static void set_server_privs(void) {
  uid_t server_uid, current_euid = geteuid();
  gid_t server_gid, current_egid = getegid();
  unsigned char switch_server_id = FALSE;

  uid_t *uid = get_param_ptr(main_server->conf, "UserID", FALSE);
  gid_t *gid =  get_param_ptr(main_server->conf, "GroupID", FALSE);

  if (uid) {
    server_uid = *uid;
    switch_server_id = TRUE;

  } else
    server_uid = current_euid;

  if (gid) {
    server_gid = *gid;
    switch_server_id = TRUE;

  } else
    server_gid = current_egid;

  if (switch_server_id) {
    PRIVS_ROOT

    /* Note: will it be necessary to double check this switch, as is done
     * in elsewhere in this file?
     */
    PRIVS_SETUP(server_uid, server_gid);
  }
}

static void fork_server(int fd, conn_t *l, unsigned char nofork) {
  conn_t *conn = NULL;
  int i, rev;
  int semfds[2] = { -1, -1 };
  int xerrno = 0;

#ifndef PR_DEVEL_NO_FORK
  pid_t pid;
  sigset_t sig_set;

  if (!nofork) {

    /* A race condition exists on heavily loaded servers where the parent
     * catches SIGHUP and attempts to close/re-open the main listening
     * socket(s), however the children haven't finished closing them
     * (EADDRINUSE).  We use a semaphore pipe here to flag the parent once
     * the child has closed all former listening sockets.
     */

    if (pipe(semfds) == -1) {
      pr_log_pri(PR_LOG_ERR, "pipe(): %s", strerror(errno));
      close(fd);
      return;
    }

    /* Need to make sure the child (writer) end of the pipe isn't
     * < 2 (stdio/stdout/stderr) as this will cause problems later.
     */

    if (semfds[1] < 3)
      semfds[1] = dup_low_fd(semfds[1]);

    /* We block SIGCHLD to prevent a race condition if the child
     * dies before we can record it's pid.  Also block SIGTERM to
     * prevent sig_terminate() from examining the child list
     */

    sigemptyset(&sig_set);
    sigaddset(&sig_set, SIGTERM);
    sigaddset(&sig_set, SIGCHLD);
    sigaddset(&sig_set, SIGUSR1);
    sigaddset(&sig_set, SIGUSR2);

    sigprocmask(SIG_BLOCK, &sig_set, NULL);

    switch ((pid = fork())) {
    case 0: /* child */

      /* No longer the master process. */
      is_master = FALSE;
      sigprocmask(SIG_UNBLOCK, &sig_set, NULL);

      /* No longer need the read side of the semaphore pipe. */
      close(semfds[0]);
      break;

    case -1:
      sigprocmask(SIG_UNBLOCK, &sig_set, NULL);
      pr_log_pri(PR_LOG_ERR, "fork(): %s", strerror(errno));

      /* The parent doesn't need the socket open. */
      close(fd);
      close(semfds[0]);
      close(semfds[1]);

      return;

    default: /* parent */
      /* The parent doesn't need the socket open */
      close(fd);

      child_add(pid, semfds[0]);
      close(semfds[1]);

      /* Unblock the signals now as sig_child() will catch
       * an "immediate" death and remove the pid from the children list
       */
      sigprocmask(SIG_UNBLOCK, &sig_set, NULL);
      return;
    }
  }

  session.pid = getpid();

  /* No longer need any listening fds. */
  pr_ipbind_close_listeners();

  /* There would appear to be no useful purpose behind setting the process
   * group of the newly forked child.  In daemon/inetd mode, we should have no
   * controlling tty and either have the process group of the parent or of
   * inetd.  In non-daemon mode (-n), doing this may cause SIGTTOU to be
   * raised on output to the terminal (stderr logging).
   *
   * #ifdef HAVE_SETPGID
   *   setpgid(0,getpid());
   * #else
   * # ifdef SETPGRP_VOID
   *   setpgrp();
   * # else
   *   setpgrp(0,getpid());
   * # endif
   * #endif
   *
   */

  /* Reseed pseudo-randoms */
  srand((unsigned int) (time(NULL) * getpid()));

#endif /* PR_DEVEL_NO_FORK */

  /* Child is running here */
  signal(SIGUSR1, sig_disconnect);
  signal(SIGUSR2, sig_evnt);

  signal(SIGCHLD, SIG_DFL);
  signal(SIGHUP, SIG_IGN);

  /* From this point on, syslog stays open. We close it first so that the
   * logger will pick up our new PID.
   *
   * We have to delay calling log_opensyslog() until after pr_inet_openrw()
   * is called, otherwise the potential exists for the syslog FD to
   * be overwritten and the user to see logging information.
   *
   * This isn't that big of a deal because the logging functions will
   * just open it dynamically if they need to.
   */
  log_closesyslog();

  /* Specifically DO NOT perform reverse DNS at this point, to alleviate
   * the race condition mentioned above.  Instead we do it after closing
   * all former listening sockets.
   */
  conn = pr_inet_openrw(permanent_pool, l, NULL, PR_NETIO_STRM_CTRL, fd,
    STDIN_FILENO, STDOUT_FILENO, FALSE);

  /* Capture errno here, if necessary. */
  if (!conn)
    xerrno = errno;

  /* Now do the permanent syslog open
   */
  pr_signals_block();
  PRIVS_ROOT

  log_opensyslog(NULL);

  PRIVS_RELINQUISH
  pr_signals_unblock();

  if (!conn) {
    pr_log_pri(PR_LOG_ERR, "Fatal: unable to open incoming connection: %s",
      strerror(xerrno));
    exit(1);
  }

  pr_inet_set_proto_opts(permanent_pool, conn, 0, 1, 1, 0, 0);

  pr_event_generate("core.connect", conn);

  /* Find the server for this connection. */
  main_server = pr_ipbind_get_server(conn->local_addr, conn->local_port);

  /* The follow code was ostensibly used to conserve memory, to free all other
   * servers and associated configurations.  However, when large numbers of
   * servers are configured, this process adds significant time to the
   * establishment of a session.  More importantly, I do not think it is
   * really necessary; copy-on-write semantics mean that those portions of
   * memory won't actually be in this process' space until changed.  And if
   * those configurations will never be reached, the only time the associated
   * memory would change is now, when it is attempted to be freed.
   *
   * s = main_server;
   * while (s) {
   *   s_saved = s->next;
   *   if (s != serv) {
   *     if (s->listen && s->listen != l) {
   *       if (s->listen->listen_fd == conn->rfd ||
   *           s->listen->listen_fd == conn->wfd)
   *         s->listen->listen_fd = -1;
   *       else
   *         inet_close(s->pool,s->listen);
   *     }
   *
   *     if (s->listen) {
   *       if (s->listen->listen_fd == conn->rfd ||
   *          s->listen->listen_fd == conn->wfd)
   *            s->listen->listen_fd = -1;
   *     }
   *
   *     xaset_remove(server_list,(xasetmember_t*)s);
   *     destroy_pool(s->pool);
   *   }
   *   s = s_saved;
   * }
   */

  session.pool = make_sub_pool(permanent_pool);
  pr_pool_tag(session.pool, "Session Pool");

  session.c = conn;
  session.data_port = conn->remote_port - 1;
  session.sf_flags = 0;
  session.sp_flags = 0;
  session.proc_prefix = "(connecting)";

  /* Close the write side of the semaphore pipe to tell the parent
   * we are all grown up and have finished housekeeping (closing
   * former listen sockets).
   */
  close(semfds[1]);

  /* Now perform reverse DNS lookups. */
  if (ServerUseReverseDNS) {
    rev = pr_netaddr_set_reverse_dns(ServerUseReverseDNS);

    if (conn->remote_addr)
      conn->remote_name = pr_netaddr_get_dnsstr(conn->remote_addr);

    pr_netaddr_set_reverse_dns(rev);
  }

  pr_netaddr_set_sess_addrs();

  /* Check and see if we are shutdown */
  if (shutdownp) {
    time_t now;

    time(&now);
    if (!deny || deny <= now) {
      config_rec *c = NULL;
      char *reason = NULL;
      const char *serveraddress;

      serveraddress = (session.c && session.c->local_addr) ?
        pr_netaddr_get_ipstr(session.c->local_addr) :
        main_server->ServerAddress;

      if ((c = find_config(main_server->conf, CONF_PARAM, "MasqueradeAddress",
        FALSE)) != NULL) {

        pr_netaddr_t *masq_addr = (pr_netaddr_t *) c->argv[0];
        serveraddress = pr_netaddr_get_ipstr(masq_addr);
      }

      reason = sreplace(permanent_pool, shutmsg,
                   "%s", pstrdup(permanent_pool, pr_strtime(shut)),
                   "%r", pstrdup(permanent_pool, pr_strtime(deny)),
                   "%d", pstrdup(permanent_pool, pr_strtime(disc)),
		   "%C", (session.cwd[0] ? session.cwd : "(none)"),
		   "%L", serveraddress,
		   "%R", (session.c && session.c->remote_name ?
                         session.c->remote_name : "(unknown)"),
		   "%T", pstrdup(permanent_pool, pr_strtime(now)),
		   "%U", "NONE",
		   "%V", main_server->ServerName,
                   NULL );

      pr_log_auth(PR_LOG_NOTICE, "connection refused (%s) from %s [%s]",
               reason, session.c->remote_name,
               pr_netaddr_get_ipstr(session.c->remote_addr));

      pr_response_send(R_500,
        _("FTP server shut down (%s) -- please try again later"), reason);
      exit(0);
    }
  }

  /* If no server is configured to handle the addr the user is
   * connected to, drop them.
   */
  if (!main_server) {
    pr_log_debug(DEBUG2, "No server configuration found for IP address %s",
      pr_netaddr_get_ipstr(conn->local_addr));
    pr_log_debug(DEBUG2, "Use the DefaultServer directive to designate "
      "a default server configuration to handle requests like this");

    pr_response_send(R_500,
      _("Sorry, no server available to handle request on %s"),
      pr_netaddr_get_dnsstr(conn->local_addr));
    exit(0);
  }

  if (main_server->listen) {
    if (main_server->listen->listen_fd == conn->rfd ||
        main_server->listen->listen_fd == conn->wfd)
      main_server->listen->listen_fd = -1;

    destroy_pool(main_server->listen->pool);
    main_server->listen = NULL;
  }

  /* Set the ID/privs for the User/Group in this server */
  set_server_privs();

  /* Find the class for this session. */
  session.class = pr_class_match_addr(session.c->remote_addr);
  if (session.class != NULL) {
    pr_log_debug(DEBUG2, "session requested from client in '%s' class",
      session.class->cls_name);

  } else {
    pr_log_debug(DEBUG5, "session requested from client in unknown class");
  }

  /* Check config tree for <Limit LOGIN> directives.  Do not perform
   * this check until after the class of the session has been determined,
   * in order to properly handle any AllowClass/DenyClass directives
   * within the <Limit> section.
   */
  if (!login_check_limits(main_server->conf, TRUE, FALSE, &i)) {
    pr_log_pri(PR_LOG_NOTICE, "Connection from %s [%s] denied.",
      session.c->remote_name,
      pr_netaddr_get_ipstr(session.c->remote_addr));
    exit(0);
  }

  /* Create a table for modules to use. */
  session.notes = pr_table_alloc(session.pool, 0);
  if (session.notes == NULL) {
    pr_log_debug(DEBUG3, "error creating session.notes table: %s",
      strerror(errno));
  }

  /* Prepare the Timers API. */
  timers_init();

  /* Set the per-child resource limits. */
  set_session_rlimits();

  /* Inform all the modules that we are now a child */
  pr_log_debug(DEBUG7, "performing module session initializations");
  if (modules_session_init() < 0)
    end_login(1);

  pr_log_debug(DEBUG4, "connected - local  : %s:%d",
    pr_netaddr_get_ipstr(session.c->local_addr), session.c->local_port);
  pr_log_debug(DEBUG4, "connected - remote : %s:%d",
    pr_netaddr_get_ipstr(session.c->remote_addr), session.c->remote_port);

  pr_proctitle_set("connected: %s (%s:%d)",
    session.c->remote_name ? session.c->remote_name : "?",
    session.c->remote_addr ? pr_netaddr_get_ipstr(session.c->remote_addr) : "?",
    session.c->remote_port ? session.c->remote_port : 0);

  pr_log_pri(PR_LOG_INFO, "%s session opened.",
    pr_session_get_protocol(PR_SESS_PROTO_FL_LOGOUT));

  /* Make sure we can receive OOB data */
  pr_inet_set_async(session.pool, session.c);

  send_session_banner(main_server);

  cmd_handler(main_server, conn);

#ifdef PR_DEVEL_NO_DAEMON
  /* Cleanup */
  end_login_noexit();
  main_server = NULL;
  free_pools();
  pr_proctitle_free();
#endif /* PR_DEVEL_NO_DAEMON */
}

static void disc_children(void) {

  if (disc && disc <= time(NULL) && child_count()) {
    sigset_t sig_set;

    sigemptyset(&sig_set);
    sigaddset(&sig_set, SIGTERM);
    sigaddset(&sig_set, SIGCHLD);
    sigaddset(&sig_set, SIGUSR1);
    sigaddset(&sig_set, SIGUSR2);

    sigprocmask(SIG_BLOCK, &sig_set, NULL);

    PRIVS_ROOT
    child_signal(SIGUSR1);
    PRIVS_RELINQUISH

    sigprocmask(SIG_UNBLOCK, &sig_set, NULL);
  }
}

static void daemon_loop(void) {
  fd_set listenfds;
  conn_t *listen_conn;
  int fd, maxfd;
  int i, err_count = 0, xerrno = 0;
  unsigned long nconnects = 0UL;
  time_t last_error;
  struct timeval tv;
  static int running = 0;

  pr_proctitle_set("(accepting connections)");

  time(&last_error);

  while (TRUE) {
    run_schedule();

    FD_ZERO(&listenfds);
    maxfd = 0;
    maxfd = pr_ipbind_listen(&listenfds);

    /* Monitor children pipes */
    maxfd = semaphore_fds(&listenfds, maxfd);

    /* Check for ftp shutdown message file */
    switch (check_shutmsg(&shut, &deny, &disc, shutmsg, sizeof(shutmsg))) {
      case 1:
        if (!shutdownp)
          disc_children();
        shutdownp = 1;
        break;

      case 0:
        shutdownp = 0;
        deny = disc = (time_t) 0;
        break;
    }

    if (shutdownp) {
      tv.tv_sec = 5L;
      tv.tv_usec = 0L;

    } else {

      tv.tv_sec = PR_TUNABLE_SELECT_TIMEOUT;
      tv.tv_usec = 0L;
    }

    /* If running (a flag signaling whether proftpd is just starting up)
     * AND shutdownp (a flag signalling the present of /etc/shutmsg) are
     * true, then log an error stating this -- but don't stop the server.
     */
    if (shutdownp && !running) {

      /* Check the value of the deny time_t struct w/ the current time.
       * If the deny time has passed, log that all incoming connections
       * will be refused.  If not, note the date at which they will be
       * refused in the future.
       */
      time_t now = time(NULL);

      if (difftime(deny, now) < 0.0) {
        pr_log_pri(PR_LOG_ERR, PR_SHUTMSG_PATH
          " present: all incoming connections will be refused.");

      } else {
        pr_log_pri(PR_LOG_ERR, PR_SHUTMSG_PATH " present: incoming connections "
          "will be denied starting %s", CHOP(ctime(&deny)));
      }
    }

    running = 1;

    i = select(maxfd + 1, &listenfds, NULL, NULL, &tv);
    if (i < 0) {
      xerrno = errno;
    }

    if (i == -1 && xerrno == EINTR) {
      pr_signals_handle();
      continue;
    }

    if (have_dead_child) {
      sigset_t sig_set;

      sigemptyset(&sig_set);
      sigaddset(&sig_set, SIGCHLD);
      sigaddset(&sig_set, SIGTERM);
      pr_alarms_block();
      sigprocmask(SIG_BLOCK, &sig_set, NULL);

      have_dead_child = FALSE;
      child_update();

      sigprocmask(SIG_UNBLOCK, &sig_set, NULL);
      pr_alarms_unblock();
    }

    if (i == -1) {
      time_t this_error;

      time(&this_error);

      if ((this_error - last_error) <= 5 && err_count++ > 10) {
        pr_log_pri(PR_LOG_ERR, "Fatal: select() failing repeatedly, shutting "
          "down.");
        exit(1);

      } else if ((this_error - last_error) > 5) {
        last_error = this_error;
        err_count = 0;
      }

      pr_log_pri(PR_LOG_NOTICE, "select() failed in daemon_loop(): %s",
        strerror(xerrno));
    }

    if (i == 0)
      continue;

    /* Reset the connection counter.  Take into account this current
     * connection, which does not (yet) have an entry in the child list.
     */
    nconnects = 1UL;

    /* See if child semaphore pipes have signaled */
    if (child_count()) {
      pr_child_t *ch;
      time_t now = time(NULL);

      for (ch = child_get(NULL); ch; ch = child_get(ch)) {
	if (ch->ch_pipefd != -1 &&
            FD_ISSET(ch->ch_pipefd, &listenfds)) {
	  close(ch->ch_pipefd);
	  ch->ch_pipefd = -1;
	}

        /* While we're looking, tally up the number of children forked in
         * the past interval.
         */
        if (ch->ch_when >= (now - (unsigned long) max_connect_interval))
          nconnects++;
      }
    }

    pr_signals_handle();

    if (i < 0) {
      continue;
    }

    /* Accept the connection. */
    listen_conn = pr_ipbind_accept_conn(&listenfds, &fd);

    /* Fork off servers to handle each connection our job is to get back to
     * answering connections asap, so leave the work of determining which
     * server the connection is for to our child.
     */

    if (listen_conn) {

      /* Check for exceeded MaxInstances. */
      if (ServerMaxInstances && (child_count() >= ServerMaxInstances)) {
        pr_event_generate("core.max-instances", NULL);
        
        pr_log_pri(PR_LOG_WARNING,
          "MaxInstances (%d) reached, new connection denied",
          ServerMaxInstances);
        close(fd);

      /* Check for exceeded MaxConnectionRate. */
      } else if (max_connects && (nconnects > max_connects)) {
        pr_event_generate("core.max-connection-rate", NULL);

        pr_log_pri(PR_LOG_WARNING,
          "MaxConnectionRate (%lu/%u secs) reached, new connection denied",
          max_connects, max_connect_interval);
        close(fd);

      /* Fork off a child to handle the connection. */
      } else {
        fork_server(fd, listen_conn, FALSE);
      }
    }
#ifdef PR_DEVEL_NO_DAEMON
    /* Do not continue the while() loop here if not daemonizing. */
    break;
#endif /* PR_DEVEL_NO_DAEMON */
  }
}

/* This function is to handle the dispatching of actions based on
 * signals received by the signal handlers, to avoid signal handler-based
 * race conditions.
 */

void pr_signals_handle(void) {
  table_handling_signal(TRUE);

  if (errno == EINTR &&
      PR_TUNABLE_EINTR_RETRY_INTERVAL > 0) {
    struct timeval tv;
    unsigned long interval_usecs = PR_TUNABLE_EINTR_RETRY_INTERVAL * 1000000;

    tv.tv_sec = (interval_usecs / 1000000);
    tv.tv_usec = (interval_usecs - (tv.tv_sec * 1000000));

    pr_trace_msg("signal", 8, "interrupted system call, "
      "delaying for %lu %s, %lu %s",
      (unsigned long) tv.tv_sec, tv.tv_sec != 1 ? "secs" : "sec",
      (unsigned long) tv.tv_usec, tv.tv_usec != 1 ? "microsecs" : "microsec");

    pr_signals_block();
    (void) select(0, NULL, NULL, NULL, &tv);
    pr_signals_unblock();
  }

  while (recvd_signal_flags) {

    if (recvd_signal_flags & RECEIVED_SIG_ALRM) {
      recvd_signal_flags &= ~RECEIVED_SIG_ALRM;
      pr_trace_msg("signal", 9, "handling SIGALRM (signal %d)", SIGALRM);
      handle_alarm();
    }

    if (recvd_signal_flags & RECEIVED_SIG_CHLD) {
      recvd_signal_flags &= ~RECEIVED_SIG_CHLD;
      pr_trace_msg("signal", 9, "handling SIGCHLD (signal %d)", SIGCHLD);
      handle_chld();
    }

    if (recvd_signal_flags & RECEIVED_SIG_EVENT) {
      recvd_signal_flags &= ~RECEIVED_SIG_EVENT;

      /* The "event" signal is SIGUSR2 in proftpd. */
      pr_trace_msg("signal", 9, "handling SIGUSR2 (signal %d)", SIGUSR2);
      handle_evnt();
    }

    if (recvd_signal_flags & RECEIVED_SIG_SEGV) {
      recvd_signal_flags &= ~RECEIVED_SIG_SEGV;
      pr_trace_msg("signal", 9, "handling SIGSEGV (signal %d)", SIGSEGV);
      handle_terminate_other();
    }

    if (recvd_signal_flags & RECEIVED_SIG_TERMINATE) {
      recvd_signal_flags &= ~RECEIVED_SIG_TERMINATE;
      pr_trace_msg("signal", 9, "handling signal %d", term_signo);
      handle_terminate();
    }

    if (recvd_signal_flags & RECEIVED_SIG_TERM_OTHER) {
      recvd_signal_flags &= ~RECEIVED_SIG_TERM_OTHER;
      pr_trace_msg("signal", 9, "handling signal %d", term_signo);
      handle_terminate_other();
    }

    if (recvd_signal_flags & RECEIVED_SIG_XCPU) {
      recvd_signal_flags &= ~RECEIVED_SIG_XCPU;
      pr_trace_msg("signal", 9, "handling SIGXCPU (signal %d)", SIGXCPU);
      handle_xcpu();
    }

    if (recvd_signal_flags & RECEIVED_SIG_ABORT) {
      recvd_signal_flags &= ~RECEIVED_SIG_ABORT;
      pr_trace_msg("signal", 9, "handling SIGABRT (signal %d)", SIGABRT);
      handle_abort();
    }

    if (recvd_signal_flags & RECEIVED_SIG_RESTART) {
      recvd_signal_flags &= ~RECEIVED_SIG_RESTART;
      pr_trace_msg("signal", 9, "handling SIGHUP (signal %d)", SIGHUP);

      /* NOTE: should this be done here, rather than using a schedule? */
      schedule(core_restart_cb, 0, NULL, NULL, NULL, NULL);
    }

    if (recvd_signal_flags & RECEIVED_SIG_EXIT) {
      recvd_signal_flags &= ~RECEIVED_SIG_EXIT;
      pr_trace_msg("signal", 9, "handling SIGUSR1 (signal %d)", SIGUSR1);
      session_exit(PR_LOG_NOTICE, "Parent process requested shutdown", 0, NULL);
    }

    if (recvd_signal_flags & RECEIVED_SIG_SHUTDOWN) {
      recvd_signal_flags &= ~RECEIVED_SIG_SHUTDOWN;
      pr_trace_msg("signal", 9, "handling SIGUSR1 (signal %d)", SIGUSR1);

      /* NOTE: should this be done here, rather than using a schedule? */
      schedule(shutdown_exit, 0, NULL, NULL, NULL, NULL);
    }
  }

  table_handling_signal(FALSE);
}

/* sig_restart occurs in the master daemon when manually "kill -HUP"
 * in order to re-read configuration files, and is sent to all
 * children by the master.
 */
static RETSIGTYPE sig_restart(int signo) {
  recvd_signal_flags |= RECEIVED_SIG_RESTART;
  signal(SIGHUP, sig_restart);
}

static RETSIGTYPE sig_evnt(int signo) {
  recvd_signal_flags |= RECEIVED_SIG_EVENT;
  signal(SIGUSR2, sig_evnt);
}

/* sig_disconnect is called in children when the parent daemon
 * detects that shutmsg has been created and ftp sessions should
 * be destroyed.  If a file transfer is underway, the process simply
 * dies, otherwise a function is scheduled to attempt to display
 * the shutdown reason.
 */
static RETSIGTYPE sig_disconnect(int signo) {

  /* If this is an anonymous session, or a transfer is in progress,
   * perform the exit a little later...
   */
  if ((session.sf_flags & SF_ANON) ||
      (session.sf_flags & SF_XFER))
    recvd_signal_flags |= RECEIVED_SIG_EXIT;
  else
    recvd_signal_flags |= RECEIVED_SIG_SHUTDOWN;

  signal(SIGUSR1, SIG_IGN);
}

static RETSIGTYPE sig_child(int signo) {
  recvd_signal_flags |= RECEIVED_SIG_CHLD;

  /* We make an exception here to the synchronous processing that is done
   * for other signals; SIGCHLD is handled asynchronously.  This is made
   * necessary by two things.
   *
   * First, we need to support non-POSIX systems.  Under POSIX, once a
   * signal handler has been configured for a given signal, that becomes
   * that signal's disposition, until explicitly changed later.  Non-POSIX
   * systems, on the other hand, will restore the default disposition of
   * a signal after a custom signal handler has been configured.  Thus,
   * to properly support non-POSIX systems, a call to signal(2) is necessary
   * as one of the last steps in our signal handlers.
   *
   * Second, SVR4 systems differ specifically in their semantics of signal(2)
   * and SIGCHLD.  These systems will check for any unhandled SIGCHLD
   * signals, waiting to be reaped via wait(2) or waitpid(2), whenever
   * the disposition of SIGCHLD is changed.  This means that if our process
   * handles SIGCHLD, but does not call wait(2) or waitpid(2), and then
   * calls signal(2), another SIGCHLD is generated; this loop repeats,
   * until the process runs out of stack space and terminates.
   *
   * Thus, in order to cover this interaction, we'll need to call handle_chld()
   * here, asynchronously.  handle_chld() does the work of reaping dead
   * child processes, and does not seem to call any non-reentrant functions,
   * so it should be safe.
   */

  handle_chld();
  signal(SIGCHLD, sig_child);
}

#ifdef PR_DEVEL_COREDUMP
static char *prepare_core(void) {
  static char dir[256] = {'\0'};

  snprintf(dir, sizeof(dir), "%s/proftpd-core-%lu", PR_CORE_DIR,
    (unsigned long) getpid());

  if (mkdir(dir, 0700) != -1)
    chdir(dir);

  else
    pr_log_pri(PR_LOG_ERR, "unable to create '%s': %s", dir, strerror(errno));
  return dir;
}
#endif /* PR_DEVEL_COREDUMP */

static RETSIGTYPE sig_abort(int signo) {
  recvd_signal_flags |= RECEIVED_SIG_ABORT;
  signal(SIGABRT, SIG_DFL);

#ifdef PR_DEVEL_COREDUMP
  pr_log_pri(PR_LOG_NOTICE, "ProFTPD received SIGABRT signal, generating core "
    "file in %s", prepare_core());
  end_login_noexit();
  abort();
#endif /* PR_DEVEL_COREDUMP */
}

static void handle_abort(void) {
  pr_log_pri(PR_LOG_NOTICE, "ProFTPD received SIGABRT signal, no core dump");
  finish_terminate();
}

#ifdef PR_DEVEL_STACK_TRACE
static void handle_segv(int signo, siginfo_t *info, void *ptr) {
  register unsigned i;
  ucontext_t *uc = (ucontext_t *) ptr;
  void *trace[PR_TUNABLE_CALLER_DEPTH];
  char **strings;
  size_t tracesz;

  /* Call the "normal" SIGSEGV handler. */
  table_handling_signal(TRUE);
  sig_terminate(signo);
  table_handling_signal(FALSE);

  pr_log_pri(PR_LOG_ERR, "-----BEGIN STACK TRACE-----");

  tracesz = backtrace(trace, PR_TUNABLE_CALLER_DEPTH);

  /* Overwrite sigaction with caller's address */
#if defined(REG_EIP)
  trace[1] = (void *) uc->uc_mcontext.gregs[REG_EIP];
#elif defined(REG_RIP)
  trace[1] = (void *) uc->uc_mcontext.gregs[REG_RIP];
#endif

  strings = backtrace_symbols(trace, tracesz);

  /* Skip first stack frame; it just points here. */
  for (i = 1; i < tracesz; ++i) {
    pr_log_pri(PR_LOG_ERR, "[%u] %s", i-1, strings[i]);
  }

  pr_log_pri(PR_LOG_ERR, "-----END STACK TRACE-----");

  exit(0);
}
#endif /* PR_DEVEL_STACK_TRACE */

static RETSIGTYPE sig_terminate(int signo) {

  /* Make sure the scoreboard slot is properly cleared.  Note that this is
   * possibly redundant, as it should already be handled properly in
   * end_login_noexit().
   */
  pr_scoreboard_entry_del(FALSE);

  /* Capture the signal number for later display purposes. */
  term_signo = signo;

  if (signo == SIGSEGV) {
    recvd_signal_flags |= RECEIVED_SIG_SEGV;

    /* This is probably not the safest thing to be doing, but since the
     * process is terminating anyway, why not?  It helps when knowing/logging
     * that a segfault happened...
     */
    pr_trace_msg("signal", 9, "handling SIGSEGV (signal %d)", signo);
    pr_log_pri(PR_LOG_NOTICE, "ProFTPD terminating (signal %d)", signo);

    pr_log_pri(PR_LOG_INFO, "%s session closed.",
      pr_session_get_protocol(PR_SESS_PROTO_FL_LOGOUT));

    /* Restore the default signal handler. */
#ifdef PR_DEVEL_STACK_TRACE
    install_segv_handler();
#else
    signal(SIGSEGV, SIG_DFL);
#endif /* PR_DEVEL_STACK_TRACE */

  } else if (signo == SIGTERM) {
    recvd_signal_flags |= RECEIVED_SIG_TERMINATE;

  } else if (signo == SIGXCPU) {
    recvd_signal_flags |= RECEIVED_SIG_XCPU;

  } else {
    recvd_signal_flags |= RECEIVED_SIG_TERM_OTHER;
  }
}

static void handle_chld(void) {
  sigset_t sig_set;
  pid_t pid;

  sigemptyset(&sig_set);
  sigaddset(&sig_set, SIGTERM);
  sigaddset(&sig_set, SIGCHLD);

  pr_alarms_block();

  /* Block SIGTERM in here, so we don't create havoc with the child list
   * while modifying it.
   */
  sigprocmask(SIG_BLOCK, &sig_set, NULL);

  while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
    if (child_remove(pid) == 0)
      have_dead_child = TRUE;
  }

  sigprocmask(SIG_UNBLOCK, &sig_set, NULL);
  pr_alarms_unblock();
}

static void handle_evnt(void) {
  pr_event_generate("core.signal.USR2", NULL);
}

static void handle_xcpu(void) {
  pr_log_pri(PR_LOG_NOTICE, "ProFTPD CPU limit exceeded (signal %d)", SIGXCPU);
  finish_terminate();
}

static void handle_terminate_other(void) {
  pr_log_pri(PR_LOG_ERR, "ProFTPD terminating (signal %d)", term_signo);
  finish_terminate();
}

static void handle_terminate(void) {

  /* Do not log if we are a child that has been terminated. */
  if (is_master) {

    /* Send a SIGTERM to all our children */
    if (child_count()) {
      PRIVS_ROOT
      child_signal(SIGTERM);
      PRIVS_RELINQUISH
    }

    pr_log_pri(PR_LOG_NOTICE, "ProFTPD killed (signal %d)", term_signo);
  }

  finish_terminate();
}

static void finish_terminate(void) {

  if (is_master &&
      mpid == getpid()) {
    PRIVS_ROOT

    /* Do not need the pidfile any longer. */
    if (ServerType == SERVER_STANDALONE &&
        !nodaemon)
      pr_pidfile_remove();

    /* Run any exit handlers registered in the master process here, so that
     * they may have the benefit of root privs.  More than likely these
     * exit handlers were registered by modules' module initialization
     * functions, which also occur under root priv conditions. (If an
     * exit handler is registered after the fork(), it won't be run here --
     * that registration occurs in a different process space.
     */
    pr_event_generate("core.exit", NULL);

    /* Remove the registered exit handlers now, so that the ensuing
     * end_login() call (outside the root privs condition) does not call
     * the exit handlers for the master process again.
     */
    pr_event_unregister(NULL, "core.exit", NULL);

    PRIVS_RELINQUISH

    if (ServerType == SERVER_STANDALONE) {
      pr_log_pri(PR_LOG_NOTICE, "ProFTPD " PROFTPD_VERSION_TEXT
        " standalone mode SHUTDOWN");

      /* Clean up the scoreboard */
      PRIVS_ROOT
      pr_delete_scoreboard();
      PRIVS_RELINQUISH
    }
  }

  end_login(1);
}

#ifdef PR_DEVEL_STACK_TRACE
static void install_segv_handler(void) {
  struct sigaction action;

  memset(&action, 0, sizeof(action));
  action.sa_sigaction = handle_segv;
  action.sa_flags = SA_SIGINFO;

  /* Ideally we would check the return value here. */
  sigaction(SIGSEGV, &action, NULL);
}
#endif /* PR_DEVEL_STACK_TRACE */

static void install_signal_handlers(void) {
  sigset_t sig_set;

  /* Should the master server (only applicable in standalone mode)
   * kill off children if we receive a signal that causes termination?
   * Hmmmm... maybe this needs to be rethought, but I've done it in
   * such a way as to only kill off our children if we receive a SIGTERM,
   * meaning that the admin wants us dead (and probably our kids too).
   */

  /* The sub-pool for the child list is created the first time we fork
   * off a child.  To conserve memory, the pool and list is destroyed
   * when our last child dies (to prevent the list from eating more and
   * more memory on long uptimes).
   */

  sigemptyset(&sig_set);

  sigaddset(&sig_set, SIGCHLD);
  sigaddset(&sig_set, SIGINT);
  sigaddset(&sig_set, SIGQUIT);
  sigaddset(&sig_set, SIGILL);
  sigaddset(&sig_set, SIGABRT);
  sigaddset(&sig_set, SIGFPE);
  sigaddset(&sig_set, SIGSEGV);
  sigaddset(&sig_set, SIGALRM);
  sigaddset(&sig_set, SIGTERM);
  sigaddset(&sig_set, SIGHUP);
  sigaddset(&sig_set, SIGUSR2);
#ifdef SIGSTKFLT
  sigaddset(&sig_set, SIGSTKFLT);
#endif /* SIGSTKFLT */
#ifdef SIGIO
  sigaddset(&sig_set, SIGIO);
#endif /* SIGIO */
#ifdef SIGBUS
  sigaddset(&sig_set, SIGBUS);
#endif /* SIGBUS */

  signal(SIGCHLD, sig_child);
  signal(SIGHUP, sig_restart);
  signal(SIGINT, sig_terminate);
  signal(SIGQUIT, sig_terminate);
  signal(SIGILL, sig_terminate);
  signal(SIGABRT, sig_abort);
  signal(SIGFPE, sig_terminate);
#ifdef PR_DEVEL_STACK_TRACE
  install_segv_handler();
#else
  signal(SIGSEGV, sig_terminate);
#endif /* PR_DEVEL_STACK_TRACE */
  signal(SIGTERM, sig_terminate);
  signal(SIGXCPU, sig_terminate);
  signal(SIGURG, SIG_IGN);
#ifdef SIGSTKFLT
  signal(SIGSTKFLT, sig_terminate);
#endif /* SIGSTKFLT */
#ifdef SIGIO
  signal(SIGIO, SIG_IGN);
#endif /* SIGIO */
#ifdef SIGBUS
  signal(SIGBUS, sig_terminate);
#endif /* SIGBUS */
  signal(SIGUSR2, sig_evnt);

  /* In case our parent left signals blocked (as happens under some
   * poor inetd implementations)
   */
  sigprocmask(SIG_UNBLOCK, &sig_set, NULL);
}

void set_daemon_rlimits(void) {
  config_rec *c = NULL;
  struct rlimit rlim;

  if (getrlimit(RLIMIT_CORE, &rlim) < 0)
    pr_log_pri(PR_LOG_ERR, "error: getrlimit(RLIMIT_CORE): %s",
      strerror(errno));

  else {
#ifdef PR_DEVEL_COREDUMP
    rlim.rlim_cur = rlim.rlim_max = RLIM_INFINITY;
#else
    rlim.rlim_cur = rlim.rlim_max = 0;
#endif /* PR_DEVEL_COREDUMP */

    PRIVS_ROOT
    if (setrlimit(RLIMIT_CORE, &rlim) < 0) {
      PRIVS_RELINQUISH
      pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_CORE): %s",
        strerror(errno));
      return;
    }
    PRIVS_RELINQUISH
  }

  /* Now check for the configurable resource limits */
  c = find_config(main_server->conf, CONF_PARAM, "RLimitCPU", FALSE);

#ifdef RLIMIT_CPU
  while (c) {
    /* Does this limit apply to the daemon? */
    if (c->argv[1] == NULL || !strcmp(c->argv[1], "daemon")) {
      struct rlimit *cpu_rlimit = (struct rlimit *) c->argv[0];

      PRIVS_ROOT
      if (setrlimit(RLIMIT_CPU, cpu_rlimit) < 0) {
        PRIVS_RELINQUISH
        pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_CPU): %s",
          strerror(errno));
        return;
      }
      PRIVS_RELINQUISH

      pr_log_debug(DEBUG2, "set RLimitCPU for daemon");
    }

    c = find_config_next(c, c->next, CONF_PARAM, "RLimitCPU", FALSE);
  }
#endif /* defined RLIMIT_CPU */

  c = find_config(main_server->conf, CONF_PARAM, "RLimitMemory", FALSE);

#if defined(RLIMIT_DATA) || defined(RLIMIT_AS) || defined(RLIMIT_VMEM)
  while (c) {
    /* Does this limit apply to the daemon? */
    if (c->argv[1] == NULL || !strcmp(c->argv[1], "daemon")) {
      struct rlimit *memory_rlimit = (struct rlimit *) c->argv[0];

      PRIVS_ROOT
#  if defined(RLIMIT_DATA)
      if (setrlimit(RLIMIT_DATA, memory_rlimit) < 0) {
        PRIVS_RELINQUISH
        pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_DATA): %s",
          strerror(errno));
        return;
      }
#  elif defined(RLIMIT_AS)
      if (setrlimit(RLIMIT_AS, memory_rlimit) < 0) {
        PRIVS_RELINQUISH
        pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_AS): %s",
          strerror(errno));
        return;
      }
#  elif defined(RLIMIT_VMEM)
      if (setrlimit(RLIMIT_VMEM, memory_rlimit) < 0) {
        PRIVS_RELINQUISH
        pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_VMEM): %s",
          strerror(errno));
        return;
      }
#  endif
      PRIVS_RELINQUISH

      pr_log_debug(DEBUG2, "set RLimitMemory for daemon");
    }

    c = find_config_next(c, c->next, CONF_PARAM, "RLimitMemory", FALSE);
  }
#endif /* no RLIMIT_DATA || RLIMIT_AS || RLIMIT_VMEM */

  c = find_config(main_server->conf, CONF_PARAM, "RLimitOpenFiles", FALSE);

#if defined(RLIMIT_NOFILE) || defined(RLIMIT_OFILE)
  while (c) {
    /* Does this limit apply to the daemon? */
    if (c->argv[1] == NULL || !strcmp(c->argv[1], "daemon")) {
      struct rlimit *nofile_rlimit = (struct rlimit *) c->argv[0];

      PRIVS_ROOT
#  if defined(RLIMIT_NOFILE)
      if (setrlimit(RLIMIT_NOFILE, nofile_rlimit) < 0) {
        PRIVS_RELINQUISH
        pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_NOFILE): %s",
          strerror(errno));
        return;
      }
#  elif defined(RLIMIT_OFILE)
      if (setrlimit(RLIMIT_OFILE, nofile_rlimit) < 0) {
        PRIVS_RELINQUISH
        pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_OFILE): %s",
          strerror(errno));
        return;
      }
#  endif
      PRIVS_RELINQUISH

      pr_log_debug(DEBUG2, "set RLimitOpenFiles for daemon");
    }

    c = find_config_next(c, c->next, CONF_PARAM, "RLimitOpenFiles", FALSE);
  }
#endif /* defined RLIMIT_NOFILE or defined RLIMIT_OFILE */
}

void set_session_rlimits(void) {
  config_rec *c = NULL;

  /* now check for the configurable rlimits */
  c = find_config(main_server->conf, CONF_PARAM, "RLimitCPU", FALSE);

#ifdef RLIMIT_CPU
  while (c) {
    /* Does this limit apply to the session? */
    if (c->argv[1] == NULL || !strcmp(c->argv[1], "session")) {
      struct rlimit *cpu_rlimit = (struct rlimit *) c->argv[0];

      PRIVS_ROOT
      if (setrlimit(RLIMIT_CPU, cpu_rlimit) < 0) {
        PRIVS_RELINQUISH
        pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_CPU): %s",
          strerror(errno));
        return;
      }
      PRIVS_RELINQUISH

      pr_log_debug(DEBUG2, "set RLimitCPU for session");
    }

    c = find_config_next(c, c->next, CONF_PARAM, "RLimitCPU", FALSE);
  }
#endif /* defined RLIMIT_CPU */

  c = find_config(main_server->conf, CONF_PARAM, "RLimitMemory", FALSE);

#if defined(RLIMIT_DATA) || defined(RLIMIT_AS) || defined(RLIMIT_VMEM)
  while (c) {
    /* Does this limit apply to the session? */
    if (c->argv[1] == NULL || !strcmp(c->argv[1], "session")) {
      struct rlimit *memory_rlimit = (struct rlimit *) c->argv[0];

      PRIVS_ROOT
#  if defined(RLIMIT_DATA)
      if (setrlimit(RLIMIT_DATA, memory_rlimit) < 0) {
        PRIVS_RELINQUISH
        pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_DATA): %s",
          strerror(errno));
        return;
      }
#  elif defined(RLIMIT_AS)
      if (setrlimit(RLIMIT_AS, memory_rlimit) < 0) {
        PRIVS_RELINQUISH
        pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_AS): %s",
          strerror(errno));
        return;
      }
#  elif defined(RLIMIT_VMEM)
      if (setrlimit(RLIMIT_VMEM, memory_rlimit) < 0) {
        PRIVS_RELINQUISH
        pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_VMEM): %s",
          strerror(errno));
        return;
      }
#  endif
      PRIVS_RELINQUISH

      pr_log_debug(DEBUG2, "set RLimitMemory for session");
    }

    c = find_config_next(c, c->next, CONF_PARAM, "RLimitMemory", FALSE);
  }
#endif /* no RLIMIT_DATA || RLIMIT_AS || RLIMIT_VMEM */

  c = find_config(main_server->conf, CONF_PARAM, "RLimitOpenFiles", FALSE);

#if defined(RLIMIT_NOFILE) || defined(RLIMIT_OFILE)
  while (c) {
    /* Does this limit apply to the session? */
    if (c->argv[1] == NULL || !strcmp(c->argv[1], "session")) {
      struct rlimit *nofile_rlimit = (struct rlimit *) c->argv[0];

      PRIVS_ROOT
#  if defined(RLIMIT_NOFILE)
      if (setrlimit(RLIMIT_NOFILE, nofile_rlimit) < 0) {
        PRIVS_RELINQUISH
        pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_NOFILE): %s",
          strerror(errno));
        return;
      }
#  elif defined(RLIMIT_OFILE)
      if (setrlimit(RLIMIT_OFILE, nofile_rlimit) < 0) {
        PRIVS_RELINQUISH
        pr_log_pri(PR_LOG_ERR, "error: setrlimit(RLIMIT_OFILE): %s",
          strerror(errno));
        return;
      }
#  endif /* defined RLIMIT_OFILE */
      PRIVS_RELINQUISH

      pr_log_debug(DEBUG2, "set RLimitOpenFiles for session");
    }

    c = find_config_next(c, c->next, CONF_PARAM, "RLimitOpenFiles", FALSE);
  }
#endif /* defined RLIMIT_NOFILE or defined RLIMIT_OFILE */
}

static void daemonize(void) {
#ifndef HAVE_SETSID
  int ttyfd;
#endif

  /* Fork off and have parent exit.
   */
  switch (fork()) {
    case -1:
      perror("fork");
      exit(1);

    case 0:
      break;

    default: 
      exit(0);
  }

#ifdef HAVE_SETSID
  /* setsid() is the preferred way to disassociate from the
   * controlling terminal
   */
  setsid();
#else
  /* Open /dev/tty to access our controlling tty (if any) */
  if ((ttyfd = open("/dev/tty", O_RDWR)) != -1) {
    if (ioctl(ttyfd, TIOCNOTTY, NULL) == -1) {
      perror("ioctl");
      exit(1);
    }

    close(ttyfd);
  }
#endif /* HAVE_SETSID */

  /* Close the three big boys */
  close(fileno(stdin));
  close(fileno(stdout));
  close(fileno(stderr));

  /* Portable way to prevent re-acquiring a tty in the future */

#ifdef HAVE_SETPGID
  setpgid(0, getpid());
#else
# ifdef SETPGRP_VOID
  setpgrp();
# else
  setpgrp(0, getpid());
# endif
#endif

  /* Reset the cached "master PID" value to that of the daemon process;
   * there are places in the code which check this value to see if they
   * are the daemon process, e.g. at shutdown.
   */
  mpid = getpid();

  pr_fsio_chdir("/", 0);
}

static void inetd_main(void) {
  int res = 0;

  /* Make sure the scoreboard file exists. */
  PRIVS_ROOT
  if ((res = pr_open_scoreboard(O_RDWR)) < 0) {
    PRIVS_RELINQUISH

    switch (res) {
      case PR_SCORE_ERR_BAD_MAGIC:
        pr_log_pri(PR_LOG_ERR, "error opening scoreboard: bad/corrupted file");
        return;

      case PR_SCORE_ERR_OLDER_VERSION:
      case PR_SCORE_ERR_NEWER_VERSION:
        pr_log_pri(PR_LOG_ERR, "error opening scoreboard: wrong version, "
          "writing new scoreboard");

        /* Delete the scoreboard, then open it again (and assume that the
         * open succeeds).
         */
        PRIVS_ROOT
        pr_delete_scoreboard();
        pr_open_scoreboard(O_RDWR);
        break;

      default:
        pr_log_pri(PR_LOG_ERR, "error opening scoreboard: %s",
          strerror(errno));
        return;
    }
  }
  PRIVS_RELINQUISH
  pr_close_scoreboard();

  pr_event_generate("core.startup", NULL);

  init_bindings();

  /* Check our shutdown status */
  if (check_shutmsg(&shut, &deny, &disc, shutmsg, sizeof(shutmsg)) == 1)
    shutdownp = 1;

  /* Finally, call right into fork_server() to start servicing the
   * connection immediately.
   */
  fork_server(STDIN_FILENO, main_server->listen, TRUE);
}

static void standalone_main(void) {
  int res = 0;

  if (nodaemon) {
    log_stderr(quiet ? FALSE : TRUE);
    close(fileno(stdin));
    close(fileno(stdout));

  } else {
    log_stderr(FALSE);
    daemonize();
  }

  PRIVS_ROOT
  pr_delete_scoreboard();
  if ((res = pr_open_scoreboard(O_RDWR)) < 0) {
    PRIVS_RELINQUISH

    switch (res) {
      case PR_SCORE_ERR_BAD_MAGIC:
        pr_log_pri(PR_LOG_ERR,
          "error opening scoreboard: bad/corrupted file");
        return;

      case PR_SCORE_ERR_OLDER_VERSION:
        pr_log_pri(PR_LOG_ERR,
          "error opening scoreboard: bad version (too old)");
        return;

      case PR_SCORE_ERR_NEWER_VERSION:
        pr_log_pri(PR_LOG_ERR,
          "error opening scoreboard: bad version (too new)");
        return;

      default:
        pr_log_pri(PR_LOG_ERR, "error opening scoreboard: %s", strerror(errno));
        return;
    }
  }
  PRIVS_RELINQUISH
  pr_close_scoreboard();

  pr_event_generate("core.startup", NULL);

  init_bindings();

  pr_log_pri(PR_LOG_NOTICE, "ProFTPD %s (built %s) standalone mode STARTUP",
    PROFTPD_VERSION_TEXT " " PR_STATUS, BUILD_STAMP);

  pr_pidfile_write();
  daemon_loop();
}

extern char *optarg;
extern int optind, opterr, optopt;

#ifdef HAVE_GETOPT_LONG
static struct option opts[] = {
  { "nocollision",    0, NULL, 'N' },
  { "nodaemon",	      0, NULL, 'n' },
  { "quiet",	      0, NULL, 'q' },
  { "debug",	      1, NULL, 'd' },
  { "define",	      1, NULL, 'D' },
  { "config",	      1, NULL, 'c' },
  { "persistent",     1, NULL, 'p' },
  { "list",           0, NULL, 'l' },
  { "version",        0, NULL, 'v' },
  { "settings",       0, NULL, 'V' },
  { "version-status", 0, NULL, 1   },
  { "configtest",     0, NULL, 't' },
  { "help",	      0, NULL, 'h' },
  { "ipv4",           0, NULL, '4' },
  { "ipv6",           0, NULL, '6' },
  { NULL,	      0, NULL,  0  }
};
#endif /* HAVE_GETOPT_LONG */

static void show_settings(void) {
#ifdef HAVE_UNAME
  int res;
  struct utsname uts;
#endif /* !HAVE_UNAME */

  printf("Compile-time Settings:\n");
  printf("  Version: " PROFTPD_VERSION_TEXT " " PR_STATUS "\n");

#ifdef HAVE_UNAME
  /* We use uname(2) to get the 'machine', which will tell us whether
   * we're a 32- or 64-bit machine.
   */
  res = uname(&uts);
  if (res == 0) {
    printf("  Platform: " PR_PLATFORM " [%s %s %s]\n", uts.sysname,
      uts.release, uts.machine);

  } else {
    printf("  Platform: " PR_PLATFORM " [unavailable]\n");
  }
#else
  printf("  Platform: " PR_PLATFORM " [unknown]\n");
#endif /* !HAVE_UNAME */

  printf("  Built: " BUILD_STAMP "\n");
  printf("  Built With:\n    configure " PR_BUILD_OPTS "\n\n");

  printf("  CFLAGS: " PR_BUILD_CFLAGS "\n");
  printf("  LDFLAGS: " PR_BUILD_LDFLAGS "\n");
  printf("  LIBS: " PR_BUILD_LIBS "\n");

  printf("\n  Files:\n");
  printf("    Configuration File:\n");
  printf("      " PR_CONFIG_FILE_PATH "\n");
  printf("    Pid File:\n");
  printf("      " PR_PID_FILE_PATH "\n");
  printf("    Scoreboard File:\n");
  printf("      " PR_RUN_DIR "/proftpd.scoreboard\n");
#ifdef PR_USE_DSO
  printf("    Header Directory:\n");
  printf("      " PR_INCLUDE_DIR "/proftpd\n");
  printf("    Shared Module Directory:\n");
  printf("      " PR_LIBEXEC_DIR "\n");
#endif /* PR_USE_DSO */

  /* Feature settings */
  printf("\n  Features:\n");
#ifdef PR_USE_AUTO_SHADOW
  printf("    + Autoshadow support\n");
#else
  printf("    - Autoshadow support\n");
#endif /* PR_USE_AUTO_SHADOW */

#ifdef PR_USE_CTRLS
  printf("    + Controls support\n");
#else
  printf("    - Controls support\n");
#endif /* PR_USE_CTRLS */

#if defined(PR_USE_CURSES) && defined(HAVE_LIBCURSES)
  printf("    + curses support\n");
#else
  printf("    - curses support\n");
#endif /* PR_USE_CURSES && HAVE_LIBCURSES */

#ifdef PR_USE_DEVEL
  printf("    + Developer support\n");
#else
  printf("    - Developer support\n");
#endif /* PR_USE_DEVEL */

#ifdef PR_USE_DSO
  printf("    + DSO support\n");
#else
  printf("    - DSO support\n");
#endif /* PR_USE_DSO */

#ifdef PR_USE_IPV6
  printf("    + IPv6 support\n");
#else
  printf("    - IPv6 support\n");
#endif /* PR_USE_IPV6 */

#ifdef PR_USE_LARGEFILES
  printf("    + Largefile support\n");
#else
  printf("    - Largefile support\n");
#endif /* PR_USE_LARGEFILES */

#ifdef PR_USE_LASTLOG
  printf("    + Lastlog support\n");
#else
  printf("    - Lastlog support\n");
#endif /* PR_USE_LASTLOG */

#if defined(PR_USE_NCURSESW) && defined(HAVE_LIBNCURSESW)
  printf("    + ncursesw support\n");
#elif defined(PR_USE_NCURSES) && defined(HAVE_LIBNCURSES)
  printf("    + ncurses support\n");
#else
  printf("    - ncurses support\n");
#endif

#ifdef PR_USE_NLS
  printf("    + NLS support\n");
#else
  printf("    - NLS support\n");
#endif /* PR_USE_NLS */

#ifdef PR_USE_OPENSSL
  printf("    + OpenSSL support\n");
#else
  printf("    - OpenSSL support\n");
#endif /* PR_USE_OPENSSL */

#ifdef PR_USE_FACL
  printf("    + POSIX ACL support\n");
#else
  printf("    - POSIX ACL support\n");
#endif /* PR_USE_FACL */

#ifdef PR_USE_SHADOW
  printf("    + Shadow file support\n");
#else
  printf("    - Shadow file suppport\n");
#endif /* PR_USE_SHADOW */

#ifdef PR_USE_SENDFILE
  printf("    + Sendfile support\n");
#else
  printf("    - Sendfile support\n");
#endif /* PR_USE_SENDFILE */

#ifdef PR_USE_TRACE
  printf("    + Trace support\n");
#else
  printf("    - Trace support\n");
#endif /* PR_USE_TRACE */

  /* Tunable settings */
  printf("\n  Tunable Options:\n");
  printf("    PR_TUNABLE_BUFFER_SIZE = %u\n", PR_TUNABLE_BUFFER_SIZE);
  printf("    PR_TUNABLE_GLOBBING_MAX_MATCHES = %lu\n", PR_TUNABLE_GLOBBING_MAX_MATCHES);
  printf("    PR_TUNABLE_GLOBBING_MAX_RECURSION = %u\n", PR_TUNABLE_GLOBBING_MAX_RECURSION);
  printf("    PR_TUNABLE_HASH_TABLE_SIZE = %u\n", PR_TUNABLE_HASH_TABLE_SIZE);
  printf("    PR_TUNABLE_NEW_POOL_SIZE = %u\n", PR_TUNABLE_NEW_POOL_SIZE);
  printf("    PR_TUNABLE_SCOREBOARD_BUFFER_SIZE = %u\n",
    PR_TUNABLE_SCOREBOARD_BUFFER_SIZE);
  printf("    PR_TUNABLE_SCOREBOARD_SCRUB_TIMER = %u\n",
    PR_TUNABLE_SCOREBOARD_SCRUB_TIMER);
  printf("    PR_TUNABLE_SELECT_TIMEOUT = %u\n", PR_TUNABLE_SELECT_TIMEOUT);
  printf("    PR_TUNABLE_TIMEOUTIDENT = %u\n", PR_TUNABLE_TIMEOUTIDENT);
  printf("    PR_TUNABLE_TIMEOUTIDLE = %u\n", PR_TUNABLE_TIMEOUTIDLE);
  printf("    PR_TUNABLE_TIMEOUTLINGER = %u\n", PR_TUNABLE_TIMEOUTLINGER);
  printf("    PR_TUNABLE_TIMEOUTLOGIN = %u\n", PR_TUNABLE_TIMEOUTLOGIN);
  printf("    PR_TUNABLE_TIMEOUTNOXFER = %u\n", PR_TUNABLE_TIMEOUTNOXFER);
  printf("    PR_TUNABLE_TIMEOUTSTALLED = %u\n", PR_TUNABLE_TIMEOUTSTALLED);
  printf("    PR_TUNABLE_XFER_SCOREBOARD_UPDATES = %u\n\n",
    PR_TUNABLE_XFER_SCOREBOARD_UPDATES);
}

static struct option_help {
  const char *long_opt, *short_opt, *desc;

} opts_help[] = {
  { "--help", "-h",
    "Display proftpd usage"},

  { "--nocollision", "-N",
    "Disable address/port collision checking" },

  { "--nodaemon", "-n",
    "Disable background daemon mode (and send all output to stderr)" },

  { "--quiet", "-q",
    "Don't send output to stderr when running with -n or --nodaemon" },

  { "--debug", "-d [level]",
    "Set debugging level (0-10, 10 = most debugging)" },

  { "--define", "-D [definition]",
    "Set arbitrary IfDefine definition" },

  { "--config", "-c [config-file]",
    "Specify alternate configuration file" },

  { "--persistent", "-p [0|1]",
    "Enable/disable default persistent passwd support" },

  { "--list", "-l",
    "List all compiled-in modules" },

  { "--serveraddr", "-S",
    "Specify IP address for server config" },

  { "--configtest", "-t",
    "Test the syntax of the specified config" },

  { "--settings", "-V",
    "Print compile-time settings and exit" },

  { "--version", "-v",
    "Print version number and exit" },

  { "--version-status", "-vv",
    "Print extended version information and exit" },

  { "--ipv4", "-4",
    "Support IPv4 connections only" },

  { "--ipv6", "-6",
    "Support IPv6 connections" },

  { NULL, NULL, NULL }
};

static void show_usage(int exit_code) {
  struct option_help *h;

  printf("usage: proftpd [options]\n");
  for (h = opts_help; h->long_opt; h++) {
#ifdef HAVE_GETOPT_LONG
    printf(" %s, %s\n ", h->short_opt, h->long_opt);
#else /* HAVE_GETOPT_LONG */
    printf(" %s\n", h->short_opt);
#endif /* HAVE_GETOPT_LONG */
    printf("    %s\n", h->desc);
  }

  exit(exit_code);
}

int main(int argc, char *argv[], char **envp) {
  int optc, show_version = 0;
  const char *cmdopts = "D:NVc:d:hlnp:qS:tv46";
  mode_t *main_umask = NULL;
  socklen_t peerlen;
  struct sockaddr peer;

#ifdef HAVE_SET_AUTH_PARAMETERS
  (void) set_auth_parameters(argc, argv);
#endif

#ifdef HAVE_TZSET
  /* Preserve timezone information in jailed environments.
   */
  tzset();
#endif

  memset(&session, 0, sizeof(session));

  pr_proctitle_init(argc, argv, envp);

  /* Seed rand */
  srand((unsigned int) (time(NULL) * getpid()));

  /* getpeername() fails if the fd isn't a socket */
  peerlen = sizeof(peer);
  memset(&peer, 0, peerlen);
  if (getpeername(fileno(stdin), &peer, &peerlen) != -1)
    log_stderr(FALSE);

  /* Open the syslog */
  log_opensyslog(NULL);

  /* Initialize the memory subsystem here */
  init_pools();

  /* Command line options supported:
   *
   * -D parameter       set run-time configuration parameter
   * --define parameter
   * -V
   * --settings         report compile-time settings
   * -c path            set the configuration path
   * --config path
   * -d n               set the debug level
   * --debug n
   * -q                 quiet mode; don't log to stderr when not daemonized
   * --quiet
   * -N                 disable address/port collision checks
   * --nocollision
   * -n                 standalone server does not daemonize, all logging
   * --nodaemon         redirected to stderr
   * -S                 specify the IP address for the 'server config',
   * --serveraddr       rather than using DNS on the hostname
   * -t                 syntax check of the configuration file
   * --configtest
   * -v                 report version number
   * --version
   * -4                 support IPv4 connections only
   * --ipv4
   * -6                 support IPv6 connections
   * --ipv6
   */

  opterr = 0;
  while ((optc =
#ifdef HAVE_GETOPT_LONG
	 getopt_long(argc, argv, cmdopts, opts, NULL)
#else /* HAVE_GETOPT_LONG */
	 getopt(argc, argv, cmdopts)
#endif /* HAVE_GETOPT_LONG */
	 ) != -1) {
    switch (optc) {

    case 'D':
      if (!optarg) {
        pr_log_pri(PR_LOG_ERR, "Fatal: -D requires definition argument");
        exit(1);
      }

      pr_define_add(optarg, TRUE);
      break;

    case 'V':
      show_settings();
      exit(0);
      break;

    case 'N':
      AddressCollisionCheck = FALSE;
      break;

    case 'n':
      nodaemon++;
#ifdef PR_USE_DEVEL
      pr_pool_debug_set_flags(PR_POOL_DEBUG_FL_OOM_DUMP_POOLS);
#endif
      break;

    case 'q':
      quiet++;
      break;

    case 'd':
      if (!optarg) {
        pr_log_pri(PR_LOG_ERR, "Fatal: -d requires debugging level argument.");
        exit(1);
      }
      pr_log_setdebuglevel(atoi(optarg));
      break;

    case 'c':
      if (!optarg) {
        pr_log_pri(PR_LOG_ERR,
          "Fatal: -c requires configuration path argument.");
        exit(1);
      }

      /* Note: we delay sanity-checking the given path until after the FSIO
       * layer has been initialized.
       */
      config_filename = strdup(optarg);
      break;

    case 'l':
      modules_list(PR_MODULES_LIST_FL_SHOW_STATIC);
      exit(0);
      break;

    case 'S':
      if (!optarg) {
        pr_log_pri(PR_LOG_ERR,
          "Fatal: -S requires IP address parameter.");
        exit(1);
      }

      if (pr_netaddr_set_localaddr_str(optarg) < 0) {
        pr_log_pri(PR_LOG_ERR,
          "Fatal: unable to use '%s' as server address: %s", optarg,
          strerror(errno));
        exit(1);
      }
      break;

    case 't':
      syntax_check = 1;
      printf("Checking syntax of configuration file\n");
      fflush(stdout);
      break;

    case 'p': {
      if (!optarg ||
          ((persistent_passwd = atoi(optarg)) != 1 && persistent_passwd != 0)) {
        pr_log_pri(PR_LOG_ERR, "Fatal: -p requires boolean (0|1) argument.");
        exit(1);
      }

      break;
    }

    case 'v':
      show_version++;
      break;

    case 1:
      show_version = 2;
      break;

    case 'h':
      show_usage(0);
      break;

    case 4:
      pr_netaddr_disable_ipv6();
      break;

    case 6:
      pr_netaddr_enable_ipv6();
      break;

    case '?':
      pr_log_pri(PR_LOG_ERR, "unknown option: %c", (char)optopt);
      show_usage(1);
      break;
    }
  }

  /* If we have any leftover parameters, it's an error. */
  if (argv[optind]) {
    pr_log_pri(PR_LOG_ERR, "unknown parameter: '%s'", argv[optind]);
    exit(1);
  }

  if (show_version &&
      show_version == 1) {
    printf("ProFTPD Version " PROFTPD_VERSION_TEXT "\n");
    exit(0);
  }

  mpid = getpid();

  /* Install signal handlers */
  install_signal_handlers();

  /* Initialize sub-systems */
  init_pools();
  init_regexp();
  init_log();
  init_inet();
  init_netio();
  init_netaddr();
  init_fs();
  init_class();
  free_bindings();
  init_config();
  init_stash();

#ifdef PR_USE_CTRLS
  init_ctrls();
#endif /* PR_USE_CTRLS */

  var_init();
  modules_init();

#ifdef PR_USE_NLS
# ifdef HAVE_LOCALE_H
  /* Initialize the locale based on environment variables. */
  if (setlocale(LC_ALL, "") == NULL) {
    const char *env_lang;

    env_lang = pr_env_get(permanent_pool, "LANG");
    pr_log_pri(PR_LOG_WARNING, "warning: unknown/unsupported LANG environment "
      "variable '%s', ignoring", env_lang);

    setlocale(LC_ALL, "C");

  } else {
    /* Make sure that LC_NUMERIC is always set to "C", so as not to interfere
     * with formatting of strings (like printing out floats in SQL query
     * strings).
     */
    setlocale(LC_NUMERIC, "C");
  }
# endif /* !HAVE_LOCALE_H */

 encode_init();
#endif /* PR_USE_NLS */

  /* Now, once the modules have had a chance to initialize themselves
   * but before the configuration stream is actually parsed, check
   * that the given configuration path is valid.
   */
  if (pr_fs_valid_path(config_filename) < 0) {
    pr_log_pri(PR_LOG_ERR, "Fatal: -c requires an absolute path");
    exit(1);
  }

  pr_parser_prepare(NULL, NULL);

  pr_event_generate("core.preparse", NULL);

  if (pr_parser_parse_file(NULL, config_filename, NULL, 0) == -1) {
    pr_log_pri(PR_LOG_ERR, "Fatal: unable to read configuration file '%s': %s",
      config_filename, strerror(errno));
    exit(1);
  }

  if (pr_parser_cleanup() < 0) {
    pr_log_pri(PR_LOG_ERR, "Fatal: error processing configuration file '%s': "
       "unclosed configuration section", config_filename);
    exit(1);
  }

  if (fixup_servers(server_list) < 0) {
    pr_log_pri(PR_LOG_ERR, "Fatal: error processing configuration file '%s'",
      config_filename);
    exit(1);
  }

  pr_event_generate("core.postparse", NULL);

  if (show_version &&
      show_version == 2) {

    printf("ProFTPD Version: %s", PROFTPD_VERSION_TEXT " " PR_STATUS "\n");
    printf("  Scoreboard Version: %08x\n", PR_SCOREBOARD_VERSION); 
    printf("  Built: %s\n\n", BUILD_STAMP);

    modules_list(PR_MODULES_LIST_FL_SHOW_VERSION);
    exit(0);
  }

  /* We're only doing a syntax check of the configuration file. */
  if (syntax_check) {
    printf("Syntax check complete.\n");
    end_login(0);
  }

  /* After configuration is complete, make sure that passwd, group
   * aren't held open (unnecessary fds for master daemon)
   */
  endpwent();
  endgrent();

  /* Security */
  {
    uid_t *uid = (uid_t *) get_param_ptr(main_server->conf, "UserID", FALSE);
    gid_t *gid = (gid_t *) get_param_ptr(main_server->conf, "GroupID", FALSE);

    if (uid)
      daemon_uid = *uid;
    else
      daemon_uid = PR_ROOT_UID;

    if (gid)
      daemon_gid = *gid;
    else
      daemon_gid = PR_ROOT_GID;
  }

  if (daemon_uid != PR_ROOT_UID) {
    /* Allocate space for daemon supplemental groups. */
    daemon_gids = make_array(permanent_pool, 2, sizeof(gid_t));

    if (pr_auth_getgroups(permanent_pool, (const char *) get_param_ptr(
        main_server->conf, "UserName", FALSE), &daemon_gids, NULL) < 0)
      pr_log_debug(DEBUG2, "unable to retrieve daemon supplemental groups");

    if (set_groups(permanent_pool, daemon_gid, daemon_gids) < 0)
      pr_log_pri(PR_LOG_ERR, "unable to set daemon groups: %s",
        strerror(errno));
  }

   if ((main_umask = (mode_t *) get_param_ptr(main_server->conf, "Umask",
       FALSE)) == NULL)
     umask((mode_t) 0022);
   else
     umask(*main_umask);

  /* Give up root and save our uid/gid for later use (if supported)
   * If we aren't currently root, PRIVS_SETUP will get rid of setuid
   * granted root and prevent further uid switching from being attempted.
   */

  PRIVS_SETUP(daemon_uid, daemon_gid)

#ifndef PR_DEVEL_COREDUMP
  /* Test to make sure that our uid/gid is correct.  Try to do this in
   * a portable fashion *gah!*
   */

  if (geteuid() != daemon_uid) {
    pr_log_pri(PR_LOG_ERR, "unable to set uid to %lu, current uid: %lu",
		    (unsigned long)daemon_uid,(unsigned long)geteuid());
    exit(1);
  }

  if (getegid() != daemon_gid) {
    pr_log_pri(PR_LOG_ERR, "unable to set gid to %lu, current gid: %lu",
		    (unsigned long)daemon_gid,(unsigned long)getegid());
    exit(1);
  }
#endif /* PR_DEVEL_COREDUMP */

#ifndef PR_DEVEL_NO_DAEMON
  set_daemon_rlimits();
#endif /* PR_DEVEL_NO_DAEMON */

  switch (ServerType) {
    case SERVER_STANDALONE:
      standalone_main();
      break;

    case SERVER_INETD:
      /* Reset the variable containing the pid of the master/daemon process;
       * it should only be non-zero in the case of standalone daemons.
       */
      mpid = 0;
      inetd_main();
      break;
  }

#ifdef PR_DEVEL_NO_DAEMON
  PRIVS_ROOT
  chdir(PR_RUN_DIR);
#endif /* PR_DEVEL_NO_DAEMON */

  return 0;
}
