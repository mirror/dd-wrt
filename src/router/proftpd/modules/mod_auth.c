/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2012 The ProFTPD Project team
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* Authentication module for ProFTPD
 * $Id: mod_auth.c,v 1.295.2.1 2012/03/01 23:51:57 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"

extern pid_t mpid;

module auth_module;

#ifdef PR_USE_LASTLOG
static unsigned char lastlog = FALSE;
#endif /* PR_USE_LASTLOG */

static unsigned char mkhome = FALSE;
static unsigned char authenticated_without_pass = FALSE;
static int TimeoutLogin = PR_TUNABLE_TIMEOUTLOGIN;
static int logged_in = 0;
static int auth_tries = 0;
static char *auth_pass_resp_code = R_230;
static pr_fh_t *displaylogin_fh = NULL;
static unsigned int TimeoutSession = 0;

static int auth_scan_scoreboard(void);
static int auth_count_scoreboard(cmd_rec *, char *);

/* auth_cmd_chk_cb() is hooked into the main server's auth_hook function,
 * so that we can deny all commands until authentication is complete.
 */
static int auth_cmd_chk_cb(cmd_rec *cmd) {
  unsigned char *authenticated = get_param_ptr(cmd->server->conf,
    "authenticated", FALSE);

  if (!authenticated || *authenticated == FALSE) {
    pr_response_send(R_530, _("Please login with USER and PASS"));
    return FALSE;
  }

  return TRUE;
}

static int auth_login_timeout_cb(CALLBACK_FRAME) {
  pr_response_send_async(R_421,
    _("Login timeout (%d seconds): closing control connection"), TimeoutLogin);

  /* It's possible that any listeners of this event might terminate the
   * session process themselves (e.g. mod_ban).  So write out that the
   * TimeoutLogin has been exceeded to the log here, in addition to the
   * scheduled session exit message.
   */
  pr_log_pri(PR_LOG_NOTICE, "%s", "Login timeout exceeded, disconnected");
  pr_event_generate("core.timeout-login", NULL);

  pr_log_pri(PR_LOG_NOTICE, "%s", "Session timed out, disconnected");
  pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_TIMEOUT,
    "TimeoutLogin");

  /* Do not restart the timer (should never be reached). */
  return 0;
}

static int auth_session_timeout_cb(CALLBACK_FRAME) {
  pr_event_generate("core.timeout-session", NULL);
  pr_response_send_async(R_421,
    _("Session Timeout (%u seconds): closing control connection"),
    TimeoutSession);

  pr_log_pri(PR_LOG_NOTICE, "%s", "FTP session timed out, disconnected");
  pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_TIMEOUT,
    "TimeoutSession");

  /* no need to restart the timer -- session's over */
  return 0;
}

/* Event listeners
 */

static void auth_exit_ev(const void *event_data, void *user_data) {
  /* Close the scoreboard descriptor that we opened. */
  (void) pr_close_scoreboard();
}

static int auth_sess_init(void) {
  config_rec *c = NULL;
  unsigned char *tmp = NULL;
  int res = 0;

  /* Check for a server-specific TimeoutLogin */
  c = find_config(main_server->conf, CONF_PARAM, "TimeoutLogin", FALSE);
  if (c != NULL) {
    TimeoutLogin = *((int *) c->argv[0]);
  }

  /* Start the login timer */
  if (TimeoutLogin) {
    pr_timer_remove(PR_TIMER_LOGIN, &auth_module);
    pr_timer_add(TimeoutLogin, PR_TIMER_LOGIN, &auth_module,
      auth_login_timeout_cb, "TimeoutLogin");
  }

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

  pr_event_register(&auth_module, "core.exit", auth_exit_ev, NULL);

  /* Create an entry in the scoreboard for this session. */
  if (pr_scoreboard_entry_add() < 0)
    pr_log_pri(PR_LOG_NOTICE, "notice: unable to add scoreboard entry: %s",
      strerror(errno));

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_USER, "(none)",
    PR_SCORE_SERVER_PORT, main_server->ServerPort,
    PR_SCORE_SERVER_ADDR, session.c->local_addr, session.c->local_port,
    PR_SCORE_SERVER_LABEL, main_server->ServerName,
    PR_SCORE_CLIENT_ADDR, session.c->remote_addr,
    PR_SCORE_CLIENT_NAME, session.c->remote_name,
    PR_SCORE_CLASS, session.class ? session.class->cls_name : "",
    PR_SCORE_PROTOCOL, "ftp",
    PR_SCORE_BEGIN_SESSION, time(NULL),
    NULL);

  /* Should we create the home for a user, if they don't have one? */
  tmp = get_param_ptr(main_server->conf, "CreateHome", FALSE);
  if (tmp != NULL && *tmp == TRUE)
    mkhome = TRUE;
  else
    mkhome = FALSE;

#ifdef PR_USE_LASTLOG
  /* Use the lastlog file, if supported and requested. */
  tmp = get_param_ptr(main_server->conf, "UseLastlog", FALSE);
  if (tmp &&
      *tmp == TRUE)
    lastlog = TRUE;
  else
    lastlog = FALSE;
#endif /* PR_USE_LASTLOG */

  /* Scan the scoreboard now, in order to tally up certain values for
   * substituting in any of the Display* file variables.  This function
   * also performs the MaxConnectionsPerHost enforcement.
   */
  auth_scan_scoreboard();

  return 0;
}

static int auth_init(void) {

  /* Add the commands handled by this module to the HELP list. */ 
  pr_help_add(C_USER, _("<sp> username"), TRUE);
  pr_help_add(C_PASS, _("<sp> password"), TRUE);
  pr_help_add(C_ACCT, _("is not implemented"), FALSE);
  pr_help_add(C_REIN, _("is not implemented"), FALSE);

  /* By default, enable auth checking */
  set_auth_check(auth_cmd_chk_cb);

  return 0;
}

static int _do_auth(pool *p, xaset_t *conf, char *u, char *pw) {
  char *cpw = NULL;
  config_rec *c;

  if (conf) {
    c = find_config(conf, CONF_PARAM, "UserPassword", FALSE);

    while (c) {
      if (strcmp(c->argv[0], u) == 0) {
        cpw = (char *) c->argv[1];
        break;
      }

      c = find_config_next(c, c->next, CONF_PARAM, "UserPassword", FALSE);
    }
  }

  if (cpw) {
    if (pr_auth_getpwnam(p, u) == NULL) {
      if (errno == ENOENT)
        pr_log_pri(PR_LOG_NOTICE, "no such user '%s'", u);

      return PR_AUTH_NOPWD;
    }

    return pr_auth_check(p, cpw, u, pw);
  }

  return pr_auth_authenticate(p, u, pw);
}

MODRET auth_err_pass(cmd_rec *cmd) {

  /* Remove the stashed original USER name here in a LOG_CMD_ERR handler, so
   * that other modules, who may want to lookup the original USER parameter on
   * a failed login in an earlier command handler phase, have a chance to do
   * so.  This removal of the USER parameter on failure was happening directly
   * in the CMD handler previously, thus preventing POST_CMD_ERR handlers from
   * using USER.
   */
  pr_table_remove(session.notes, "mod_auth.orig-user", NULL);

  return PR_HANDLED(cmd);
}

MODRET auth_log_pass(cmd_rec *cmd) {
  size_t passwd_len;

  /* Only log, to the syslog, that the login has succeeded here, where we
   * know that the login has definitely succeeded.
   */
  pr_log_auth(PR_LOG_NOTICE, "%s %s: Login successful.",
    (session.anon_config != NULL) ? "ANON" : C_USER, session.user);

  if (cmd->arg != NULL) {
    /* And scrub the memory holding the password sent by the client, for
     * safety/security.
     */
    passwd_len = strlen(cmd->arg);
    pr_memscrub(cmd->arg, passwd_len);
  }

  return PR_DECLINED(cmd);
}

MODRET auth_post_pass(cmd_rec *cmd) {
  config_rec *c = NULL;
  char *grantmsg = NULL, *user;
  unsigned int ctxt_precedence = 0;
  unsigned char have_user_timeout, have_group_timeout, have_class_timeout,
    have_all_timeout, *privsdrop = NULL, *authenticated;
  struct stat st;

  /* Was there a precending USER command? Was the client successfully
   * authenticated?
   */
  authenticated = get_param_ptr(cmd->server->conf, "authenticated", FALSE);

  /* Clear the list of auth-only modules. */
  pr_auth_clear_auth_only_modules();

  if (authenticated != NULL &&
      *authenticated == TRUE) {

    /* At this point, we can look up the Protocols config if the client
     * has been authenticated, which may have been tweaked via mod_ifsession's
     * user/group/class-specific sections.
     */
    c = find_config(main_server->conf, CONF_PARAM, "Protocols", FALSE);
    if (c) {
      register unsigned int i;
      array_header *protocols;
      char **elts;
      const char *protocol;

      protocols = c->argv[0];
      elts = protocols->elts;

      protocol = pr_session_get_protocol(PR_SESS_PROTO_FL_LOGOUT);

      /* We only want to check for 'ftp' in the configured Protocols list
       * if a) a RFC2228 mechanism (e.g. SSL or GSS) is not in use, and
       *    b) an SSH protocol is not in use.
       */
      if (session.rfc2228_mech == NULL &&
          strncmp(protocol, "SSH2", 5) != 0) {
        int allow_ftp = FALSE;

        for (i = 0; i < protocols->nelts; i++) {
          char *proto;

          proto = elts[i];
          if (proto != NULL) {
            if (strncasecmp(proto, "ftp", 4) == 0) {
              allow_ftp = TRUE;
              break;
            }
          }
        }

        if (!allow_ftp) {
          pr_log_debug(DEBUG0, "%s", "ftp protocol denied by Protocols config");
          pr_response_send(R_530, "%s", _("Login incorrect."));
          pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_CONFIG_ACL,
            "Denied by Protocols setting");
        }
      }
    }
  }

  user = pr_table_get(session.notes, "mod_auth.orig-user", NULL);

  /* Count up various quantities in the scoreboard, checking them against
   * the Max* limits to see if the session should be barred from going
   * any further.
   */
  auth_count_scoreboard(cmd, session.user);

  /* Check for dynamic configuration.  This check needs to be after the
   * setting of any possible anon_config, as that context may be allowed
   * or denied .ftpaccess-parsing separately from the containing server.
   */
  if (pr_fsio_stat(session.cwd, &st) != -1)
    build_dyn_config(cmd->tmp_pool, session.cwd, &st, TRUE);

  have_user_timeout = have_group_timeout = have_class_timeout =
    have_all_timeout = FALSE;

  c = find_config(TOPLEVEL_CONF, CONF_PARAM, "TimeoutSession", FALSE);
  while (c != NULL) {
    pr_signals_handle();

    if (c->argc == 3) {
      if (strncmp(c->argv[1], "user", 5) == 0) {
        if (pr_expr_eval_user_or((char **) &c->argv[2]) == TRUE) {

          if (*((unsigned int *) c->argv[1]) > ctxt_precedence) {

            /* Set the context precedence. */
            ctxt_precedence = *((unsigned int *) c->argv[1]);

            TimeoutSession = *((unsigned int *) c->argv[0]);

            have_group_timeout = have_class_timeout = have_all_timeout = FALSE;
            have_user_timeout = TRUE;
          }
        }

      } else if (strncmp(c->argv[1], "group", 6) == 0) {
        if (pr_expr_eval_group_and((char **) &c->argv[2]) == TRUE) {

          if (*((unsigned int *) c->argv[1]) > ctxt_precedence) {

            /* Set the context precedence. */
            ctxt_precedence = *((unsigned int *) c->argv[1]);

            TimeoutSession = *((unsigned int *) c->argv[0]);

            have_user_timeout = have_class_timeout = have_all_timeout = FALSE;
            have_group_timeout = TRUE;
          }
        }

      } else if (strncmp(c->argv[1], "class", 6) == 0) {
        if (session.class &&
            strcmp(session.class->cls_name, c->argv[2]) == 0) {

          if (*((unsigned int *) c->argv[1]) > ctxt_precedence) {

            /* Set the context precedence. */
            ctxt_precedence = *((unsigned int *) c->argv[1]);

            TimeoutSession = *((unsigned int *) c->argv[0]);

            have_user_timeout = have_group_timeout = have_all_timeout = FALSE;
            have_class_timeout = TRUE;
          }
        }
      }

    } else {

      if (*((unsigned int *) c->argv[1]) > ctxt_precedence) {

        /* Set the context precedence. */
        ctxt_precedence = *((unsigned int *) c->argv[1]);

        TimeoutSession = *((unsigned int *) c->argv[0]);

        have_user_timeout = have_group_timeout = have_class_timeout = FALSE;
        have_all_timeout = TRUE;
      }
    }

    c = find_config_next(c, c->next, CONF_PARAM, "TimeoutSession", FALSE);
  }

  /* If configured, start a session timer.  The timer ID value for
   * session timers will not be #defined, as I think that is a bad approach.
   * A better mechanism would be to use the random timer ID generation, and
   * store the returned ID in order to later remove the timer.
   */

  if (have_user_timeout || have_group_timeout ||
      have_class_timeout || have_all_timeout) {
    pr_log_debug(DEBUG4, "setting TimeoutSession of %u seconds for current %s",
      TimeoutSession,
      have_user_timeout ? "user" : have_group_timeout ? "group" :
      have_class_timeout ? "class" : "all");
    pr_timer_add(TimeoutSession, PR_TIMER_SESSION, &auth_module,
      auth_session_timeout_cb, "TimeoutSession");
  }

  /* Handle a DisplayLogin file. */
  if (displaylogin_fh) {
    if (!(session.sf_flags & SF_ANON)) {
      if (pr_display_fh(displaylogin_fh, NULL, auth_pass_resp_code, 0) < 0) {
        pr_log_debug(DEBUG6, "unable to display DisplayLogin file '%s': %s",
          displaylogin_fh->fh_path, strerror(errno));
      }

      pr_fsio_close(displaylogin_fh);
      displaylogin_fh = NULL;

    } else {
      /* We're an <Anonymous> login, but there was a previous DisplayLogin
       * configured which was picked up earlier.  Close that filehandle,
       * and look for a new one.
       */
      char *displaylogin;

      pr_fsio_close(displaylogin_fh);
      displaylogin_fh = NULL;

      displaylogin = get_param_ptr(TOPLEVEL_CONF, "DisplayLogin", FALSE);
      if (displaylogin) {
        if (pr_display_file(displaylogin, NULL, auth_pass_resp_code, 0) < 0) {
          pr_log_debug(DEBUG6, "unable to display DisplayLogin file '%s': %s",
            displaylogin, strerror(errno));
        }
      }
    }

  } else {
    char *displaylogin = get_param_ptr(TOPLEVEL_CONF, "DisplayLogin", FALSE);
    if (displaylogin) {
      if (pr_display_file(displaylogin, NULL, auth_pass_resp_code, 0) < 0) {
        pr_log_debug(DEBUG6, "unable to display DisplayLogin file '%s': %s",
          displaylogin, strerror(errno));
      }
    }
  }

  grantmsg = get_param_ptr(TOPLEVEL_CONF, "AccessGrantMsg", FALSE);
  if (grantmsg == NULL) {
    /* Append the final greeting lines. */
    if (session.sf_flags & SF_ANON) {
      pr_response_add(auth_pass_resp_code, "%s",
        _("Anonymous access granted, restrictions apply"));

    } else {
      pr_response_add(auth_pass_resp_code, _("User %s logged in"), user);
    }

  } else {
     /* Handle any AccessGrantMsg directive. */
     grantmsg = sreplace(cmd->tmp_pool, grantmsg, "%u", user, NULL);
     pr_response_add(auth_pass_resp_code, "%s", grantmsg);
  }

  privsdrop = get_param_ptr(TOPLEVEL_CONF, "RootRevoke", FALSE);
  if (privsdrop != NULL &&
      *privsdrop == TRUE) {
    pr_signals_block();
    PRIVS_ROOT
    PRIVS_REVOKE
    pr_signals_unblock();

    /* Disable future attempts at UID/GID manipulation. */
    session.disable_id_switching = TRUE;

    /* If the server's listening port is less than 1025, block PORT
     * commands (effectively allowing only passive connections, which is
     * not necessarily a Bad Thing).  Only log this here -- the blocking
     * will need to occur in mod_core's cmd_port() function.
     */
    if (session.c->local_port < 1025)
      pr_log_debug(DEBUG0, "RootRevoke in effect, disabling active transfers");

    pr_log_debug(DEBUG0, "RootRevoke in effect, dropped root privs");
  }

  return PR_DECLINED(cmd);
}

/* Handle group based authentication, only checked if pw
 * based fails
 */

static config_rec *_auth_group(pool *p, char *user, char **group,
                               char **ournamep, char **anonnamep, char *pass)
{
  config_rec *c;
  char *ourname = NULL,*anonname = NULL;
  char **grmem;
  struct group *grp;

  ourname = (char*)get_param_ptr(main_server->conf,"UserName",FALSE);
  if (ournamep && ourname)
    *ournamep = ourname;

  c = find_config(main_server->conf, CONF_PARAM, "GroupPassword", TRUE);

  if (c) do {
    grp = pr_auth_getgrnam(p, c->argv[0]);

    if (!grp)
      continue;

    for (grmem = grp->gr_mem; *grmem; grmem++)
      if (strcmp(*grmem, user) == 0) {
        if (pr_auth_check(p, c->argv[1], user, pass) == 0)
          break;
      }

    if (*grmem) {
      if (group)
        *group = c->argv[0];

      if (c->parent)
        c = c->parent;

      if (c->config_type == CONF_ANON)
        anonname = (char*)get_param_ptr(c->subset,"UserName",FALSE);
      if (anonnamep)
        *anonnamep = anonname;
      if (anonnamep && !anonname && ourname)
        *anonnamep = ourname;

      break;
    }
  } while((c = find_config_next(c,c->next,CONF_PARAM,"GroupPassword",TRUE)) != NULL);

  return c;
}

/* Determine any applicable chdirs
 */

static char *get_default_chdir(pool *p, xaset_t *conf) {
  config_rec *c;
  char *dir = NULL;
  int ret;

  c = find_config(conf, CONF_PARAM, "DefaultChdir", FALSE);

  while (c) {
    /* Check the groups acl */
    if (c->argc < 2) {
      dir = c->argv[0];
      break;
    }

    ret = pr_expr_eval_group_and(((char **) c->argv)+1);

    if (ret) {
      dir = c->argv[0];
      break;
    }

    c = find_config_next(c, c->next, CONF_PARAM, "DefaultChdir", FALSE);
  }

  /* If the directory is relative, concatenate w/ session.cwd. */
  if (dir && *dir != '/' && *dir != '~')
    dir = pdircat(p, session.cwd, dir, NULL);

  /* Check for any expandable variables. */
  if (dir)
    dir = path_subst_uservar(p, &dir);

  return dir;
}

/* Determine if the user (non-anon) needs a default root dir other than /.
 */

static char *get_default_root(pool *p) {
  config_rec *c = NULL;
  char *dir = NULL;
  int ret;

  c = find_config(main_server->conf, CONF_PARAM, "DefaultRoot", FALSE);

  while (c) {
    pr_signals_handle();

    /* Check the groups acl */
    if (c->argc < 2) {
      dir = c->argv[0];
      break;
    }

    ret = pr_expr_eval_group_and(((char **) c->argv)+1);

    if (ret) {
      dir = c->argv[0];
      break;
    }

    c = find_config_next(c, c->next, CONF_PARAM, "DefaultRoot", FALSE);
  }

  if (dir) {
    char *new_dir;

    /* Check for any expandable variables. */
    new_dir = path_subst_uservar(p, &dir);
    if (new_dir != NULL) {
      dir = new_dir;
    }

    if (strncmp(dir, "/", 2) == 0) {
      dir = NULL;

    } else {
      char *realdir;
      int xerrno = 0;

      /* We need to be the final user here so that if the user has their home
       * directory with a mode the user proftpd is running (i.e. the User
       * directive) as can not traverse down, we can still have the default
       * root.
       */

      PRIVS_USER
      realdir = dir_realpath(p, dir);
      if (realdir == NULL)
        xerrno = errno;
      PRIVS_RELINQUISH

      if (realdir) {
        dir = realdir;

      } else {
        /* Try to provide a more informative message. */
        char interp_dir[PR_TUNABLE_PATH_MAX + 1];

        memset(interp_dir, '\0', sizeof(interp_dir));
        (void) pr_fs_interpolate(dir, interp_dir, sizeof(interp_dir)-1); 

        pr_log_pri(PR_LOG_NOTICE,
          "notice: unable to use '%s' [resolved to '%s']: %s", dir, interp_dir,
          strerror(xerrno));
      }
    }
  }

  return dir;
}

static struct passwd *passwd_dup(pool *p, struct passwd *pw) {
  struct passwd *npw;

  npw = pcalloc(p, sizeof(struct passwd));

  npw->pw_name = pstrdup(p, pw->pw_name);
  npw->pw_passwd = pstrdup(p, pw->pw_passwd);
  npw->pw_uid = pw->pw_uid;
  npw->pw_gid = pw->pw_gid;
  npw->pw_gecos = pstrdup(p, pw->pw_gecos);
  npw->pw_dir = pstrdup(p, pw->pw_dir);
  npw->pw_shell = pstrdup(p, pw->pw_shell);

  return npw;
}

static void ensure_open_passwd(pool *p) {
  /* Make sure pass/group is open.
   */
  pr_auth_setpwent(p);
  pr_auth_setgrent(p);

  /* On some unices the following is necessary to ensure the files
   * are open.  (BSDI 3.1)
   */
  pr_auth_getpwent(p);
  pr_auth_getgrent(p);
}

/* Next function (the biggie) handles all authentication, setting
 * up chroot() jail, etc.
 */
static int setup_env(pool *p, cmd_rec *cmd, char *user, char *pass) {
  struct passwd *pw;
  config_rec *c, *tmpc;
  char *origuser,*ourname,*anonname = NULL,*anongroup = NULL,*ugroup = NULL;
  char *defaulttransfermode, *defroot = NULL,*defchdir = NULL,*xferlog = NULL;
  const char *sess_ttyname;
  int aclp, i, res = 0, showsymlinks;
  unsigned char *wtmp_log = NULL, *anon_require_passwd = NULL;

  /********************* Authenticate the user here *********************/

  session.hide_password = TRUE;

  origuser = user;
  c = pr_auth_get_anon_config(p, &user, &ourname, &anonname);

  if (c)
    session.anon_config = c;

  if (!user) {
    pr_log_auth(PR_LOG_NOTICE, "USER %s: user is not a UserAlias from %s [%s] "
      "to %s:%i", origuser,session.c->remote_name,
      pr_netaddr_get_ipstr(session.c->remote_addr),
      pr_netaddr_get_ipstr(session.c->local_addr), session.c->local_port);
    goto auth_failure;
  }

  pw = pr_auth_getpwnam(p, user);
  if (pw == NULL) {
    pr_log_auth(PR_LOG_NOTICE,
      "USER %s: no such user found from %s [%s] to %s:%i",
      user, session.c->remote_name,
      pr_netaddr_get_ipstr(session.c->remote_addr),
      pr_netaddr_get_ipstr(session.c->local_addr), session.c->local_port);
    goto auth_failure;
  }

  /* Security: other functions perform pw lookups, thus we need to make
   * a local copy of the user just looked up.
   */
  pw = passwd_dup(p, pw);

  if (pw->pw_uid == PR_ROOT_UID) {
    unsigned char *root_allow = NULL;

    pr_event_generate("mod_auth.root-login", NULL);

    /* If RootLogin is set to true, we allow this... even though we
     * still log a warning. :)
     */
    if ((root_allow = get_param_ptr(c ? c->subset : main_server->conf,
        "RootLogin", FALSE)) == NULL || *root_allow != TRUE) {
      if (pass)
        pr_memscrub(pass, strlen(pass));
      pr_log_auth(PR_LOG_CRIT, "SECURITY VIOLATION: root login attempted.");
      return 0;
    }
  }

  session.user = pstrdup(p, pw->pw_name);
  session.group = pstrdup(p, pr_auth_gid2name(p, pw->pw_gid));

  /* Set the login_uid and login_uid */
  session.login_uid = pw->pw_uid;
  session.login_gid = pw->pw_gid;

  /* Check for any expandable variables in session.cwd. */
  pw->pw_dir = path_subst_uservar(p, &pw->pw_dir);

  /* Before we check for supplemental groups, check to see if the locally
   * resolved name of the user, returned via auth_getpwnam(), is different
   * from the USER argument sent by the client.  The name can change, since
   * auth modules can play all sorts of neat tricks on us.
   *
   * If the names differ, assume that any cached data in the session.gids
   * and session.groups lists are stale, and clear them out.
   */
  if (strcmp(pw->pw_name, user) != 0) {
    pr_log_debug(DEBUG10, "local user name '%s' differs from client-sent "
      "user name '%s', clearing cached group data", pw->pw_name, user);
    session.gids = NULL;
    session.groups = NULL;
  }

  if (!session.gids &&
      !session.groups) {
    /* Get the supplemental groups.  Note that we only look up the
     * supplemental group credentials if we have not cached the group
     * credentials before, in session.gids and session.groups.  
     *
     * Those credentials may have already been retrieved, as part of the
     * pr_auth_get_anon_config() call.
     */
     res = pr_auth_getgroups(p, pw->pw_name, &session.gids, &session.groups);
     if (res < 1) {
       pr_log_debug(DEBUG2, "no supplemental groups found for user '%s'",
         pw->pw_name);
     }
  }

  /* If c != NULL from this point on, we have an anonymous login */
  aclp = login_check_limits(main_server->conf, FALSE, TRUE, &i);

  if (c) {
    anongroup = get_param_ptr(c->subset, "GroupName", FALSE);
    if (!anongroup)
      anongroup = get_param_ptr(main_server->conf, "GroupName",FALSE);

    /* Check for configured AnonRejectPasswords regex here, and fail the login
     * if the given password matches the regex.
     */
#ifdef PR_USE_REGEX
    if ((tmpc = find_config(c->subset, CONF_PARAM, "AnonRejectPasswords",
        FALSE)) != NULL) {
      int re_res;
      pr_regex_t *pw_regex = (pr_regex_t *) tmpc->argv[0];

      if (pw_regex && pass &&
          ((re_res = pr_regexp_exec(pw_regex, pass, 0, NULL, 0, 0, 0)) == 0)) {
        char errstr[200] = {'\0'};

        pr_regexp_error(re_res, pw_regex, errstr, sizeof(errstr));
        pr_log_auth(PR_LOG_NOTICE, "ANON %s: AnonRejectPasswords denies login",
          origuser);
 
        pr_event_generate("mod_auth.anon-reject-passwords", session.c);
        goto auth_failure;
      }
    }
#endif

    if (!login_check_limits(c->subset, FALSE, TRUE, &i) || (!aclp && !i) ){
      pr_log_auth(PR_LOG_NOTICE, "ANON %s (Login failed): Limit access denies "
        "login.", origuser);
      goto auth_failure;
    }
  }

  if (c == NULL &&
      aclp == 0) {
    pr_log_auth(PR_LOG_NOTICE,
      "USER %s (Login failed): Limit access denies login", origuser);
    goto auth_failure;
  }

  if (c) {
    anon_require_passwd = get_param_ptr(c->subset, "AnonRequirePassword",
      FALSE);
  }

  if (!c ||
      (anon_require_passwd && *anon_require_passwd == TRUE)) {
    int auth_code;
    char *user_name = user;

    if (c &&
        origuser &&
        strcasecmp(user, origuser) != 0) {
      unsigned char *auth_using_alias = get_param_ptr(c->subset,
        "AuthUsingAlias", FALSE);

      /* If 'AuthUsingAlias' set and we're logging in under an alias,
       * then auth using that alias.
       */
      if (auth_using_alias &&
          *auth_using_alias == TRUE) {
        user_name = origuser;
        pr_log_auth(PR_LOG_NOTICE,
          "ANON AUTH: User %s, authenticating using alias %s", user,
          user_name);
      }
    }

    /* It is possible for the user to have already been authenticated during
     * the handling of the USER command, as by an RFC2228 mechanism.  If
     * that had happened, we won't need to call _do_auth() here.
     */
    if (!authenticated_without_pass) {
      auth_code = _do_auth(p, c ? c->subset : main_server->conf, user_name,
        pass);

    } else {
      auth_code = PR_AUTH_OK_NO_PASS;
    }

    pr_event_generate("mod_auth.authentication-code", &auth_code);

    if (auth_code < 0) {
      /* Normal authentication has failed, see if group authentication
       * passes
       */

      c = _auth_group(p, user, &anongroup, &ourname, &anonname, pass);
      if (c != NULL) {
        if (c->config_type != CONF_ANON) {
          c = NULL;
          ugroup = anongroup;
          anongroup = NULL;
        }

        auth_code = PR_AUTH_OK;
      }
    }

    if (pass)
      pr_memscrub(pass, strlen(pass));

    if (session.auth_mech)
      pr_log_debug(DEBUG2, "user '%s' authenticated by %s", user,
        session.auth_mech);

    switch (auth_code) {
      case PR_AUTH_OK_NO_PASS:
        auth_pass_resp_code = R_232;
        break;

      case PR_AUTH_OK:
        auth_pass_resp_code = R_230;
        break;

      case PR_AUTH_NOPWD:
        pr_log_auth(PR_LOG_NOTICE,
          "USER %s (Login failed): No such user found.", user);
        goto auth_failure;

      case PR_AUTH_BADPWD:
        pr_log_auth(PR_LOG_NOTICE,
          "USER %s (Login failed): Incorrect password.", origuser);
        goto auth_failure;

      case PR_AUTH_AGEPWD:
        pr_log_auth(PR_LOG_NOTICE, "USER %s (Login failed): Password expired.",
          user);
        goto auth_failure;

      case PR_AUTH_DISABLEDPWD:
        pr_log_auth(PR_LOG_NOTICE, "USER %s (Login failed): Account disabled.",
          user);
        goto auth_failure;

      default:
        break;
    };

    /* Catch the case where we forgot to handle a bad auth code above. */
    if (auth_code < 0)
      goto auth_failure;

    if (pw->pw_uid == PR_ROOT_UID)
      pr_log_auth(PR_LOG_WARNING, "ROOT FTP login successful.");

  } else if (c && (!anon_require_passwd || *anon_require_passwd == FALSE)) {
    session.hide_password = FALSE;
  }

  pr_auth_setgrent(p);

  res = pr_auth_is_valid_shell(c ? c->subset : main_server->conf,
    pw->pw_shell);
  if (res == FALSE) {
    pr_log_auth(PR_LOG_NOTICE, "USER %s (Login failed): Invalid shell: '%s'",
      user, pw->pw_shell);
    goto auth_failure;
  }

  res = pr_auth_banned_by_ftpusers(c ? c->subset : main_server->conf,
    pw->pw_name);
  if (res == TRUE) {
    pr_log_auth(PR_LOG_NOTICE, "USER %s (Login failed): User in "
      PR_FTPUSERS_PATH, user);
    goto auth_failure;
  }

  if (c) {
    struct group *grp = NULL;
    unsigned char *add_userdir = NULL;
    char *u;

    u = pr_table_get(session.notes, "mod_auth.orig-user", NULL);
    add_userdir = get_param_ptr(c->subset, "UserDirRoot", FALSE);

    /* If resolving an <Anonymous> user, make sure that user's groups
     * are set properly for the check of the home directory path (which
     * depend on those supplemental group memberships).  Additionally,
     * temporarily switch to the new user's uid.
     */

    pr_signals_block();

    PRIVS_ROOT
    res = set_groups(p, pw->pw_gid, session.gids);
    if (res < 0) {
      pr_log_pri(PR_LOG_ERR, "error: unable to set groups: %s",
        strerror(errno));
    }

#ifndef PR_DEVEL_COREDUMP
# ifdef __hpux
    setresuid(0, 0, 0);
    setresgid(0, 0, 0);
# else
    setuid(PR_ROOT_UID);
    setgid(PR_ROOT_GID);
# endif /* __hpux */
#endif /* PR_DEVEL_COREDUMP */

    PRIVS_SETUP(pw->pw_uid, pw->pw_gid)

    if ((add_userdir && *add_userdir == TRUE) &&
        strcmp(u, user) != 0) {
      char *userdir_path = pdircat(p, c->name, u, NULL);

      session.chroot_path = dir_realpath(p, userdir_path);
      if (session.chroot_path == NULL) {
        pr_log_debug(DEBUG8, "error resolving '%s': %s", userdir_path,
          strerror(errno));
      }

    } else {
      session.chroot_path = dir_realpath(p, c->name);
      if (session.chroot_path == NULL) {
        pr_log_debug(DEBUG8, "error resolving '%s': %s", c->name,
          strerror(errno));
      }
    }

    if (session.chroot_path &&
        pr_fsio_access(session.chroot_path, X_OK, session.uid,
          session.gid, session.gids) != 0) {
      session.chroot_path = NULL;

    } else {
      session.chroot_path = pstrdup(session.pool, session.chroot_path);
    }

    /* Return all privileges back to that of the daemon, for now. */
    PRIVS_ROOT
    res = set_groups(p, daemon_gid, daemon_gids);
    if (res < 0) {
      pr_log_pri(PR_LOG_ERR, "error: unable to set groups: %s",
        strerror(errno));
    }

#ifndef PR_DEVEL_COREDUMP
# ifdef __hpux
    setresuid(0, 0, 0);
    setresgid(0, 0, 0);
# else
    setuid(PR_ROOT_UID);
    setgid(PR_ROOT_GID);
# endif /* __hpux */
#endif /* PR_DEVEL_COREDUMP */

    PRIVS_SETUP(daemon_uid, daemon_gid)

    pr_signals_unblock();

    /* Sanity check, make sure we have daemon_uid and daemon_gid back */
#ifdef HAVE_GETEUID
    if (getegid() != daemon_gid ||
        geteuid() != daemon_uid) {

      PRIVS_RELINQUISH

      pr_log_pri(PR_LOG_ERR, "changing from %s back to daemon uid/gid: %s",
            session.user, strerror(errno));
      pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_BY_APPLICATION,
        NULL);
    }
#endif /* HAVE_GETEUID */

    if (anon_require_passwd &&
        *anon_require_passwd == TRUE) {
      session.anon_user = pstrdup(session.pool, origuser);

    } else {
      session.anon_user = pstrdup(session.pool, pass);
    }

    if (!session.chroot_path) {
      pr_log_pri(PR_LOG_ERR, "%s: Directory %s is not accessible.",
        session.user, c->name);
      pr_response_add_err(R_530, _("Unable to set anonymous privileges."));
      goto auth_failure;
    }

    sstrncpy(session.cwd, "/", sizeof(session.cwd));
    xferlog = get_param_ptr(c->subset, "TransferLog", FALSE);

    if (anongroup) {
      grp = pr_auth_getgrnam(p, anongroup);
      if (grp) {
        pw->pw_gid = grp->gr_gid;
        session.group = pstrdup(p, grp->gr_name);
      }
    }

  } else {
    struct group *grp;
    char *homedir;

    if (ugroup) {
      grp = pr_auth_getgrnam(p, ugroup);
      if (grp) {
        pw->pw_gid = grp->gr_gid;
        session.group = pstrdup(p, grp->gr_name);
      }
    }

    /* Attempt to resolve any possible symlinks. */
    PRIVS_USER
    homedir = dir_realpath(p, pw->pw_dir);
    PRIVS_RELINQUISH

    if (homedir)
      sstrncpy(session.cwd, homedir, sizeof(session.cwd));
    else
      sstrncpy(session.cwd, pw->pw_dir, sizeof(session.cwd));
  }

  /* Create the home directory, if need be. */

  if (!c && mkhome) {
    if (create_home(p, session.cwd, origuser, pw->pw_uid, pw->pw_gid) < 0) {

      /* NOTE: should this cause the login to fail? */
      goto auth_failure;
    }
  }

  /* Get default chdir (if any) */
  defchdir = get_default_chdir(p, (c ? c->subset : main_server->conf));

  if (defchdir)
    sstrncpy(session.cwd, defchdir, sizeof(session.cwd));

  /* Check limits again to make sure deny/allow directives still permit
   * access.
   */

  if (!login_check_limits((c ? c->subset : main_server->conf), FALSE, TRUE,
      &i)) {
    pr_log_auth(PR_LOG_NOTICE, "%s %s: Limit access denies login.",
      (c != NULL) ? "ANON" : C_USER, origuser);
    goto auth_failure;
  }

  /* Perform a directory fixup. */
  resolve_deferred_dirs(main_server);
  fixup_dirs(main_server, CF_DEFER);

  /* If running under an anonymous context, resolve all <Directory>
   * blocks inside it.
   */
  if (c && c->subset)
    resolve_anonymous_dirs(c->subset);

  /* Write the login to wtmp.  This must be done here because we won't
   * have access after we give up root.  This can result in falsified
   * wtmp entries if an error kicks the user out before we get
   * through with the login process.  Oh well.
   */

  sess_ttyname = pr_session_get_ttyname(p);

  /* Perform wtmp logging only if not turned off in <Anonymous>
   * or the current server
   */
  if (c)
    wtmp_log = get_param_ptr(c->subset, "WtmpLog", FALSE);

  if (wtmp_log == NULL)
    wtmp_log = get_param_ptr(main_server->conf, "WtmpLog", FALSE);

  /* As per Bug#3482, we need to disable WtmpLog for FreeBSD 9.0, as
   * an interim measure.
   *
   * The issue is that some platforms update multiple files for a single
   * pututxline(3) call; proftpd tries to update those files manually,
   * do to chroots (after which a pututxline(3) call will fail).  A proper
   * solution requires a separate process, running with the correct
   * privileges, which would handle wtmp logging. The proftpd session
   * processes would send messages to this logging daemon (via Unix domain
   * socket, or FIFO, or TCP socket).
   *
   * Also note that this hack to disable WtmpLog may need to be extended
   * to other platforms in the future.
   */
#if defined(HAVE_UTMPX_H) && \
    defined(__FreeBSD_version) && __FreeBSD_version >= 900007
  if (wtmp_log == NULL ||
      *wtmp_log == TRUE) {
    wtmp_log = pcalloc(p, sizeof(unsigned char));
    *wtmp_log = FALSE;

    pr_log_debug(DEBUG5,
      "WtpmLog automatically disabled; see Bug#3482 for details");
  }
#endif

  PRIVS_ROOT

  if (wtmp_log == NULL ||
      *wtmp_log == TRUE) {
    log_wtmp(sess_ttyname, session.user, session.c->remote_name,
      session.c->remote_addr);
    session.wtmp_log = TRUE;
  }

#ifdef PR_USE_LASTLOG
  if (lastlog) {
    log_lastlog(pw->pw_uid, session.user, sess_ttyname, session.c->remote_addr);
  }
#endif /* PR_USE_LASTLOG */

  /* Open any TransferLogs */
  if (!xferlog) {
    if (c)
      xferlog = get_param_ptr(c->subset, "TransferLog", FALSE);

    if (!xferlog)
      xferlog = get_param_ptr(main_server->conf, "TransferLog", FALSE);

    if (!xferlog)
      xferlog = PR_XFERLOG_PATH;
  }

  if (strcasecmp(xferlog, "NONE") == 0) {
    xferlog_open(NULL);

  } else {
    xferlog_open(xferlog);
  }

  res = set_groups(p, pw->pw_gid, session.gids);
  if (res < 0) {
    pr_log_pri(PR_LOG_ERR, "error: unable to set groups: %s",
      strerror(errno));
  }

  PRIVS_RELINQUISH

  /* Now check to see if the user has an applicable DefaultRoot */
  if (!c && (defroot = get_default_root(session.pool))) {

    ensure_open_passwd(p);

    if (pr_auth_chroot(defroot) == -1) {
      pr_log_pri(PR_LOG_ERR, "error: unable to set default root directory");
      pr_response_send(R_530, _("Login incorrect."));
      pr_session_end(0);
    }

    /* Re-calc the new cwd based on this root dir.  If not applicable
     * place the user in / (of defroot)
     */

    if (strncmp(session.cwd, defroot, strlen(defroot)) == 0) {
      char *newcwd = &session.cwd[strlen(defroot)];

      if (*newcwd == '/')
        newcwd++;
      session.cwd[0] = '/';
      sstrncpy(&session.cwd[1], newcwd, sizeof(session.cwd));
    }
  }

  if (c)
    ensure_open_passwd(p);

  if (c &&
      pr_auth_chroot(session.chroot_path) == -1) {
    pr_log_pri(PR_LOG_ERR, "error: unable to set anonymous privileges");
    pr_response_send(R_530, _("Login incorrect."));
    pr_session_end(0);
  }

  /* new in 1.1.x, I gave in and we don't give up root permanently..
   * sigh.
   */

  PRIVS_ROOT

#ifndef PR_DEVEL_COREDUMP
# ifdef __hpux
    setresuid(0, 0, 0);
    setresgid(0, 0, 0);
# else
    setuid(PR_ROOT_UID);
    setgid(PR_ROOT_GID);
# endif /* __hpux */
#endif /* PR_DEVEL_COREDUMP */

  PRIVS_SETUP(pw->pw_uid, pw->pw_gid)

#ifdef HAVE_GETEUID
  if (getegid() != pw->pw_gid ||
     geteuid() != pw->pw_uid) {

    PRIVS_RELINQUISH
    pr_log_pri(PR_LOG_ERR, "error: %s setregid() or setreuid(): %s",
      session.user, strerror(errno));
    pr_response_send(R_530, _("Login incorrect."));
    pr_session_end(0);
  }
#endif

  /* If the home directory is NULL or "", reject the login. */
  if (pw->pw_dir == NULL ||
      strncmp(pw->pw_dir, "", 1) == 0) {
    pr_log_pri(PR_LOG_ERR, "error: user %s home directory is NULL or \"\"",
      session.user);
    pr_response_send(R_530, _("Login incorrect."));
    pr_session_end(0);
  }

  {
    unsigned char *show_symlinks = get_param_ptr(
      c ? c->subset : main_server->conf, "ShowSymlinks", FALSE);

    if (!show_symlinks || *show_symlinks == TRUE)
      showsymlinks = TRUE;
    else
      showsymlinks = FALSE;
  }

  /* chdir to the proper directory, do this even if anonymous
   * to make sure we aren't outside our chrooted space.
   */

  /* Attempt to change to the correct directory -- use session.cwd first.
   * This will contain the DefaultChdir directory, if configured...
   */
  if (pr_fsio_chdir_canon(session.cwd, !showsymlinks) == -1) {

    /* if we've got DefaultRoot or anonymous login, ignore this error
     * and chdir to /
     */

    if (session.chroot_path != NULL || defroot) {

      pr_log_debug(DEBUG2, "unable to chdir to %s (%s), defaulting to chroot "
        "directory %s", session.cwd, strerror(errno),
        (session.chroot_path ? session.chroot_path : defroot));

      if (pr_fsio_chdir_canon("/", !showsymlinks) == -1) {
        pr_log_pri(PR_LOG_ERR, "%s chdir(\"/\"): %s", session.user,
          strerror(errno));
        pr_response_send(R_530, _("Login incorrect."));
        pr_session_end(0);
      }

    } else if (defchdir) {

      /* If we've got defchdir, failure is ok as well, simply switch to
       * user's homedir.
       */
      pr_log_debug(DEBUG2, "unable to chdir to %s (%s), defaulting to home "
        "directory %s", session.cwd, strerror(errno), pw->pw_dir);

      if (pr_fsio_chdir_canon(pw->pw_dir, !showsymlinks) == -1) {
        pr_log_pri(PR_LOG_ERR, "%s chdir(\"%s\"): %s", session.user,
          session.cwd, strerror(errno));
        pr_response_send(R_530, _("Login incorrect."));
        pr_session_end(0);
      }

    } else {

      /* Unable to switch to user's real home directory, which is not
       * allowed.
       */
      pr_log_pri(PR_LOG_ERR, "%s chdir(\"%s\"): %s", session.user, session.cwd,
        strerror(errno));
      pr_response_send(R_530, _("Login incorrect."));
      pr_session_end(0);
    }
  }

  sstrncpy(session.cwd, pr_fs_getcwd(), sizeof(session.cwd));
  sstrncpy(session.vwd, pr_fs_getvwd(), sizeof(session.vwd));

  /* Make sure directory config pointers are set correctly */
  dir_check_full(p, cmd, G_NONE, session.cwd, NULL);

  if (c) {
    if (!session.hide_password) {
      session.proc_prefix = pstrcat(session.pool, session.c->remote_name,
        ": anonymous/", pass, NULL);

    } else {
      session.proc_prefix = pstrcat(session.pool, session.c->remote_name,
        ": anonymous", NULL);
    }

    session.sf_flags = SF_ANON;

  } else {
    session.proc_prefix = pstrdup(session.pool, session.c->remote_name);
    session.sf_flags = 0;
  }

  /* While closing the pointer to the password database would avoid any
   * potential attempt to hijack this information, it is unfortunately needed
   * in a chroot()ed environment.  Otherwise, mappings from UIDs to names,
   * among other things, would fail.
   */
  /* pr_auth_endpwent(p); */

  /* Default transfer mode is ASCII */
  defaulttransfermode = (char *) get_param_ptr(main_server->conf,
    "DefaultTransferMode", FALSE);

  if (defaulttransfermode &&
      strcasecmp(defaulttransfermode, "binary") == 0) {
    session.sf_flags &= (SF_ALL^SF_ASCII);

  } else {
    session.sf_flags |= SF_ASCII;
  }

  /* Authentication complete, user logged in, now kill the login
   * timer.
   */

  /* Update the scoreboard entry */
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_USER, session.user,
    PR_SCORE_CWD, session.cwd,
    NULL);

  pr_session_set_idle();

  pr_timer_remove(PR_TIMER_LOGIN, &auth_module);

  /* These copies are made from the session.pool, instead of the more
   * volatile pool used originally, in order that the copied data maintain
   * its integrity for the lifetime of the session.
   */
  session.user = pstrdup(session.pool, session.user);

  if (session.group)
    session.group = pstrdup(session.pool, session.group);

  if (session.gids)
    session.gids = copy_array(session.pool, session.gids);

  /* session.groups is an array of strings, so we must copy the string data
   * as well as the pointers.
   */
  session.groups = copy_array_str(session.pool, session.groups);

  /* Resolve any deferred-resolution paths in the FS layer */
  pr_resolve_fs_map();

  return 1;

auth_failure:
  if (pass)
    pr_memscrub(pass, strlen(pass));
  session.user = session.group = NULL;
  session.gids = session.groups = NULL;
  session.wtmp_log = FALSE;
  return 0;
}

/* This function counts the number of connected users. It only fills in the
 * Class-based counters and an estimate for the number of clients. The primary
 * purpose is to make it so that the %N/%y escapes work in a DisplayConnect
 * greeting.  A secondary purpose is to enforce any configured
 * MaxConnectionsPerHost limit.
 */
static int auth_scan_scoreboard(void) {
  char *key;
  void *v;
  config_rec *c = NULL;
  pr_scoreboard_entry_t *score = NULL;
  unsigned int cur = 0, ccur = 0, hcur = 0;
  char curr_server_addr[80] = {'\0'};
  const char *client_addr = pr_netaddr_get_ipstr(session.c->remote_addr);

  snprintf(curr_server_addr, sizeof(curr_server_addr), "%s:%d",
    pr_netaddr_get_ipstr(session.c->local_addr), main_server->ServerPort);
  curr_server_addr[sizeof(curr_server_addr)-1] = '\0';

  /* Determine how many users are currently connected */
  if (pr_rewind_scoreboard() < 0) {
    pr_log_pri(PR_LOG_NOTICE, "error rewinding scoreboard: %s",
      strerror(errno));
  }

  while ((score = pr_scoreboard_entry_read()) != NULL) {
    pr_signals_handle();

    /* Make sure it matches our current server */
    if (strcmp(score->sce_server_addr, curr_server_addr) == 0) {
      cur++;

      if (strcmp(score->sce_client_addr, client_addr) == 0)
        hcur++;

      /* Only count up authenticated clients, as per the documentation. */
      if (strncmp(score->sce_user, "(none)", 7) == 0)
        continue;

      /* Note: the class member of the scoreboard entry will never be
       * NULL.  At most, it may be the empty string.
       */
      if (session.class &&
          strcasecmp(score->sce_class, session.class->cls_name) == 0)
        ccur++;
    }
  }
  pr_restore_scoreboard();

  key = "client-count";
  (void) pr_table_remove(session.notes, key, NULL);
  v = palloc(session.pool, sizeof(unsigned int));
  *((unsigned int *) v) = cur;

  if (pr_table_add(session.notes, key, v, sizeof(unsigned int)) < 0) {
    pr_log_pri(PR_LOG_WARNING,
      "warning: error stashing '%s': %s", key, strerror(errno));
  }

  if (session.class) {
    key = "class-client-count";
    (void) pr_table_remove(session.notes, key, NULL);
    v = palloc(session.pool, sizeof(unsigned int));
    *((unsigned int *) v) = ccur;

    if (pr_table_add(session.notes, key, v, sizeof(unsigned int)) < 0) {
      pr_log_pri(PR_LOG_WARNING,
        "warning: error stashing '%s': %s", key, strerror(errno));
    }
  }

  /* Lookup any configured MaxConnectionsPerHost. */
  c = find_config(main_server->conf, CONF_PARAM, "MaxConnectionsPerHost",
    FALSE);

  if (c) {
    unsigned int *max = c->argv[0];

    if (*max &&
        hcur > *max) {

      char maxstr[20];
      char *msg = "Sorry, the maximum number of connections (%m) for your host "
        "are already connected.";

      pr_event_generate("mod_auth.max-connections-per-host", session.c);

      if (c->argc == 2)
        msg = c->argv[1];

      memset(maxstr, '\0', sizeof(maxstr));
      snprintf(maxstr, sizeof(maxstr), "%u", *max);
      maxstr[sizeof(maxstr)-1] = '\0';

      pr_response_send(R_530, "%s", sreplace(session.pool, msg,
        "%m", maxstr, NULL));

      pr_log_auth(PR_LOG_NOTICE,
        "Connection refused (MaxConnectionsPerHost %u)", *max);
      pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_CONFIG_ACL,
        "Denied by MaxConnectionsPerHost");
    }
  }

  return 0;
}

static int auth_count_scoreboard(cmd_rec *cmd, char *user) {
  char *key;
  void *v;
  pr_scoreboard_entry_t *score = NULL;
  long cur = 0, hcur = 0, ccur = 0, hostsperuser = 1, usersessions = 0;
  config_rec *c = NULL, *anon_config = NULL, *maxc = NULL;
  char *origuser;

  /* Determine how many users are currently connected. */
  origuser = user;
  anon_config = pr_auth_get_anon_config(cmd->tmp_pool, &user, NULL, NULL);

  /* Gather our statistics. */
  if (user) {
    char curr_server_addr[80] = {'\0'};

    snprintf(curr_server_addr, sizeof(curr_server_addr), "%s:%d",
      pr_netaddr_get_ipstr(session.c->local_addr), main_server->ServerPort);
    curr_server_addr[sizeof(curr_server_addr)-1] = '\0';

    if (pr_rewind_scoreboard() < 0) {
      pr_log_pri(PR_LOG_NOTICE, "error rewinding scoreboard: %s",
        strerror(errno));
    }

    while ((score = pr_scoreboard_entry_read()) != NULL) {
      unsigned char same_host = FALSE;

      pr_signals_handle();

      /* Make sure it matches our current server. */
      if (strcmp(score->sce_server_addr, curr_server_addr) == 0) {

        if ((c && c->config_type == CONF_ANON &&
            !strcmp(score->sce_user, user)) || !c) {

          /* This small hack makes sure that cur is incremented properly
           * when dealing with anonymous logins (the timing of anonymous
           * login updates to the scoreboard makes this...odd).
           */
          if (c && c->config_type == CONF_ANON && cur == 0)
              cur = 1;

          /* Only count authenticated clients, as per the documentation. */
          if (strncmp(score->sce_user, "(none)", 7) == 0)
            continue;

          cur++;

          /* Count up sessions on a per-host basis. */

          if (!strcmp(score->sce_client_addr,
              pr_netaddr_get_ipstr(session.c->remote_addr))) {
            same_host = TRUE;

            /* This small hack makes sure that hcur is incremented properly
             * when dealing with anonymous logins (the timing of anonymous
             * login updates to the scoreboard makes this...odd).
             */
            if (c && c->config_type == CONF_ANON && hcur == 0)
              hcur = 1;

            hcur++;
          }

          /* Take a per-user count of connections. */
          if (strcmp(score->sce_user, user) == 0) {
            usersessions++;

            /* Count up unique hosts. */
            if (!same_host)
              hostsperuser++;
          }
        }

        if (session.class &&
            strcasecmp(score->sce_class, session.class->cls_name) == 0) {
          ccur++;
        }
      }
    }
    pr_restore_scoreboard();
    PRIVS_RELINQUISH
  }

  key = "client-count";
  (void) pr_table_remove(session.notes, key, NULL);
  v = palloc(session.pool, sizeof(unsigned int));
  *((unsigned int *) v) = cur;

  if (pr_table_add(session.notes, key, v, sizeof(unsigned int)) < 0) {
    pr_log_pri(PR_LOG_WARNING,
      "warning: error stashing '%s': %s", key, strerror(errno));
  }

  if (session.class) {
    key = "class-client-count";
    (void) pr_table_remove(session.notes, key, NULL);
    v = palloc(session.pool, sizeof(unsigned int));
    *((unsigned int *) v) = ccur;

    if (pr_table_add(session.notes, key, v, sizeof(unsigned int)) < 0) {
      pr_log_pri(PR_LOG_WARNING,
        "warning: error stashing '%s': %s", key, strerror(errno));
    }
  }

  /* Try to determine what MaxClients/MaxHosts limits apply to this session
   * (if any) and count through the runtime file to see if this limit would
   * be exceeded.
   */

  maxc = find_config(cmd->server->conf, CONF_PARAM, "MaxClientsPerClass",
    FALSE);
  while (session.class && maxc) {
    char *maxstr = "Sorry, the maximum number of clients (%m) from your class "
      "are already connected.";
    unsigned int *max = maxc->argv[1];

    if (strcmp(maxc->argv[0], session.class->cls_name) != 0) {
      maxc = find_config_next(maxc, maxc->next, CONF_PARAM,
        "MaxClientsPerClass", FALSE);
      continue;
    }

    if (maxc->argc > 2)
      maxstr = maxc->argv[2];

    if (*max &&
        ccur > *max) {
      char maxn[20] = {'\0'};

      pr_event_generate("mod_auth.max-clients-per-class",
        session.class ? session.class->cls_name : NULL);

      snprintf(maxn, sizeof(maxn), "%u", *max);
      pr_response_send(R_530, "%s", sreplace(cmd->tmp_pool, maxstr, "%m", maxn,
        NULL));
      pr_log_auth(PR_LOG_NOTICE,
        "Connection refused (max clients %u per class %s).", *max,
        session.class->cls_name);
      pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_CONFIG_ACL,
        "Denied by MaxClientsPerClass");
    }

    break;
  }

  maxc = find_config(TOPLEVEL_CONF, CONF_PARAM, "MaxClientsPerHost", FALSE);
  if (maxc) {
    char *maxstr = "Sorry, the maximum number of clients (%m) from your host "
      "are already connected.";
    unsigned int *max = maxc->argv[0];

    if (maxc->argc > 1)
      maxstr = maxc->argv[1];

    if (*max && hcur > *max) {
      char maxn[20] = {'\0'};

      pr_event_generate("mod_auth.max-clients-per-host", session.c);

      snprintf(maxn, sizeof(maxn), "%u", *max);
      pr_response_send(R_530, "%s", sreplace(cmd->tmp_pool, maxstr, "%m", maxn,
        NULL));
      pr_log_auth(PR_LOG_NOTICE,
        "Connection refused (max clients per host %u).", *max);
      pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_CONFIG_ACL,
        "Denied by MaxClientsPerHost");
    }
  }

  /* Check for any configured MaxClientsPerUser. */
  maxc = find_config(TOPLEVEL_CONF, CONF_PARAM, "MaxClientsPerUser", FALSE);
  if (maxc) {
    char *maxstr = "Sorry, the maximum number of clients (%m) for this user "
      "are already connected.";
    unsigned int *max = maxc->argv[0];

    if (maxc->argc > 1)
      maxstr = maxc->argv[1];

    if (*max && usersessions > *max) {
      char maxn[20] = {'\0'};

      pr_event_generate("mod_auth.max-clients-per-user", user);

      snprintf(maxn, sizeof(maxn), "%u", *max);
      pr_response_send(R_530, "%s", sreplace(cmd->tmp_pool, maxstr, "%m", maxn,
        NULL));
      pr_log_auth(PR_LOG_NOTICE,
        "Connection refused (max clients per user %u).", *max);
      pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_CONFIG_ACL,
        "Denied by MaxClientsPerUser");
    }
  }

  maxc = find_config(TOPLEVEL_CONF, CONF_PARAM, "MaxClients", FALSE);
  if (maxc) {
    char *maxstr = "Sorry, the maximum number of allowed clients (%m) are "
      "already connected.";
    unsigned int *max = maxc->argv[0];

    if (maxc->argc > 1)
      maxstr = maxc->argv[1];

    if (*max && cur > *max) {
      char maxn[20] = {'\0'};

      pr_event_generate("mod_auth.max-clients", NULL);

      snprintf(maxn, sizeof(maxn), "%u", *max);
      pr_response_send(R_530, "%s", sreplace(cmd->tmp_pool, maxstr, "%m", maxn,
        NULL));
      pr_log_auth(PR_LOG_NOTICE, "Connection refused (max clients %u).", *max);
      pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_CONFIG_ACL,
        "Denied by MaxClients");
    }
  }

  maxc = find_config(TOPLEVEL_CONF, CONF_PARAM, "MaxHostsPerUser", FALSE);
  if (maxc) {
    char *maxstr = "Sorry, the maximum number of hosts (%m) for this user are "
      "already connected.";
    unsigned int *max = maxc->argv[0];

    if (maxc->argc > 1)
      maxstr = maxc->argv[1];

    if (*max && hostsperuser > *max) {
      char maxn[20] = {'\0'};

      pr_event_generate("mod_auth.max-hosts-per-user", user);

      snprintf(maxn, sizeof(maxn), "%u", *max);
      pr_response_send(R_530, "%s", sreplace(cmd->tmp_pool, maxstr, "%m", maxn,
        NULL));
      pr_log_auth(PR_LOG_NOTICE, "Connection refused (max hosts per host %u).",
        *max);
      pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_CONFIG_ACL,
        "Denied by MaxHostsPerUser");
    }
  }

  return 0;
}

MODRET auth_pre_user(cmd_rec *cmd) {

  if (logged_in)
    return PR_DECLINED(cmd);

  /* Close the passwd and group databases, because libc won't let us see new
   * entries to these files without this (only in PersistentPasswd mode).
   */
  pr_auth_endpwent(cmd->tmp_pool);
  pr_auth_endgrent(cmd->tmp_pool);

  /* Check for a user name that exceeds PR_TUNABLE_LOGIN_MAX. */
  if (strlen(cmd->arg) > PR_TUNABLE_LOGIN_MAX) {
    pr_log_pri(PR_LOG_NOTICE, "USER %s (Login failed): "
      "maximum login length exceeded", cmd->arg);
    pr_response_add_err(R_501, _("Login incorrect."));
    return PR_ERROR(cmd);
  }

  return PR_DECLINED(cmd);
}

MODRET auth_user(cmd_rec *cmd) {
  int nopass = FALSE;
  config_rec *c;
  char *denymsg = NULL, *user, *origuser;
  int failnopwprompt = 0, aclp, i;
  unsigned char *anon_require_passwd = NULL, *login_passwd_prompt = NULL;

  if (logged_in)
    return PR_ERROR_MSG(cmd, R_503, _("You are already logged in"));

  if (cmd->argc < 2)
    return PR_ERROR_MSG(cmd, R_500, _("USER: command requires a parameter"));

  user = cmd->arg;

  (void) pr_table_remove(session.notes, "mod_auth.orig-user", NULL);
  (void) pr_table_remove(session.notes, "mod_auth.anon-passwd", NULL);

  if (pr_table_add_dup(session.notes, "mod_auth.orig-user", user, 0) < 0) {
    pr_log_debug(DEBUG3, "error stashing 'mod_auth.orig-user' in "
      "session.notes: %s", strerror(errno));
  }

  origuser = user;
  c = pr_auth_get_anon_config(cmd->tmp_pool, &user, NULL, NULL);

  /* Check for AccessDenyMsg */
  if ((denymsg = get_param_ptr((c ? c->subset : cmd->server->conf),
      "AccessDenyMsg", FALSE)) != NULL) {

    if (strstr(denymsg, "%u") != NULL) {
      denymsg = sreplace(cmd->tmp_pool, denymsg, "%u", user, NULL);
    }
  }

  login_passwd_prompt = get_param_ptr(
    (c && c->config_type == CONF_ANON) ? c->subset : main_server->conf,
    "LoginPasswordPrompt", FALSE);

  if (login_passwd_prompt &&
      *login_passwd_prompt == FALSE) {
    failnopwprompt = TRUE;

  } else {
    failnopwprompt = FALSE;
  }

  if (failnopwprompt) {
    if (!user) {
      (void) pr_table_remove(session.notes, "mod_auth.orig-user", NULL);
      (void) pr_table_remove(session.notes, "mod_auth.anon-passwd", NULL);

      pr_log_pri(PR_LOG_NOTICE, "USER %s (Login failed): Not a UserAlias.",
        origuser);

      if (denymsg) {
        pr_response_send(R_530, "%s", denymsg);

      } else {
        pr_response_send(R_530, _("Login incorrect."));
      }

      pr_session_end(0);
    }

    aclp = login_check_limits(main_server->conf, FALSE, TRUE, &i);

    if (c && c->config_type != CONF_ANON) {
      c = (config_rec *) pcalloc(session.pool, sizeof(config_rec));
      c->config_type = CONF_ANON;
      c->name = "";	/* don't really need this yet */
      c->subset = main_server->conf;
    }

    if (c) {
      if (!login_check_limits(c->subset, FALSE, TRUE, &i) ||
          (!aclp && !i) ) {
        (void) pr_table_remove(session.notes, "mod_auth.orig-user", NULL);
        (void) pr_table_remove(session.notes, "mod_auth.anon-passwd", NULL);

        pr_log_auth(PR_LOG_NOTICE, "ANON %s: Limit access denies login.",
          origuser);

        if (denymsg) {
          pr_response_send(R_530, "%s", denymsg);

        } else {
          pr_response_send(R_530, _("Login incorrect."));
        }

        pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_CONFIG_ACL,
          "Denied by <Limit LOGIN>");
      }
    }

    if (c == NULL &&
        aclp == 0) {
      (void) pr_table_remove(session.notes, "mod_auth.orig-user", NULL);
      (void) pr_table_remove(session.notes, "mod_auth.anon-passwd", NULL);

      pr_log_auth(PR_LOG_NOTICE,
        "USER %s: Limit access denies login.", origuser);

      if (denymsg) {
        pr_response_send(R_530, "%s", denymsg);

      } else {
        pr_response_send(R_530, "%s", _("Login incorrect."));
      }

      pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_CONFIG_ACL,
        "Denied by <Limit LOGIN>");
    }
  }

  if (c)
    anon_require_passwd = get_param_ptr(c->subset, "AnonRequirePassword",
      FALSE);

  if (c && user && (!anon_require_passwd || *anon_require_passwd == FALSE))
    nopass = TRUE;

  session.gids = NULL;
  session.groups = NULL;
  session.user = NULL;
  session.group = NULL;

  if (nopass) {
    pr_response_add(R_331, _("Anonymous login ok, send your complete email "
      "address as your password"));

  /* Check to see if a password from the client is required.  In the
   * vast majority of cases, a password will be required.
   */
  } else if (pr_auth_requires_pass(cmd->tmp_pool, user) == FALSE) {

    /* Act as if we received a PASS command from the client. */
    cmd_rec *fakecmd = pr_cmd_alloc(cmd->pool, 2, NULL);

    /* We use pstrdup() here, rather than assigning C_PASS directly, since
     * code elsewhere will attempt to modify this buffer, and C_PASS is
     * a string literal.
     */
    fakecmd->argv[0] = pstrdup(fakecmd->pool, C_PASS);
    fakecmd->argv[1] = NULL;
    fakecmd->arg = NULL;

    c = add_config_param_set(&cmd->server->conf, "authenticated", 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
    *((unsigned char *) c->argv[0]) = TRUE;

    authenticated_without_pass = TRUE;
    pr_log_auth(PR_LOG_NOTICE, "USER %s: Authenticated without password", user);

    pr_cmd_dispatch(fakecmd);

  } else {
    pr_response_add(R_331, _("Password required for %s"), cmd->argv[1]);
  }

  return PR_HANDLED(cmd);
}

/* Close the passwd and group databases, similar to auth_pre_user(). */
MODRET auth_pre_pass(cmd_rec *cmd) {
  char *displaylogin;

  pr_auth_endpwent(cmd->tmp_pool);
  pr_auth_endgrent(cmd->tmp_pool);

  /* Look for a DisplayLogin file which has an absolute path.  If we find one,
   * open a filehandle, such that that file can be displayed even if the
   * session is chrooted.  DisplayLogin files with relative paths will be
   * handled after chroot, preserving the old behavior.
   */

  displaylogin = get_param_ptr(TOPLEVEL_CONF, "DisplayLogin", FALSE);
  if (displaylogin &&
      *displaylogin == '/') {

    displaylogin_fh = pr_fsio_open(displaylogin, O_RDONLY);
    if (displaylogin_fh == NULL)
      pr_log_debug(DEBUG6, "unable to open DisplayLogin file '%s': %s",
        displaylogin, strerror(errno));
  }

  return PR_DECLINED(cmd);
}

MODRET auth_pass(cmd_rec *cmd) {
  char *user = NULL;
  int res = 0;

  if (logged_in)
    return PR_ERROR_MSG(cmd, R_503, _("You are already logged in"));

  user = pr_table_get(session.notes, "mod_auth.orig-user", NULL);
  if (!user) {
    (void) pr_table_remove(session.notes, "mod_auth.orig-user", NULL);
    (void) pr_table_remove(session.notes, "mod_auth.anon-passwd", NULL);

    return PR_ERROR_MSG(cmd, R_503, _("Login with USER first"));
  }

  /* Clear any potentially cached directory config */
  session.anon_config = NULL;
  session.dir_config = NULL;

  res = setup_env(cmd->tmp_pool, cmd, user, cmd->arg);
  if (res == 1) {
    config_rec *c = NULL;

    c = add_config_param_set(&cmd->server->conf, "authenticated", 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
    *((unsigned char *) c->argv[0]) = TRUE;

    set_auth_check(NULL);

    (void) pr_table_remove(session.notes, "mod_auth.anon-passwd", NULL);

    if (session.sf_flags & SF_ANON) {
      if (pr_table_add_dup(session.notes, "mod_auth.anon-passwd",
          pr_fs_decode_path(cmd->server->pool, cmd->arg), 0) < 0) {
        pr_log_debug(DEBUG3,
          "error stashing anonymous password in session.notes: %s",
          strerror(errno));
      }
    }

    logged_in = 1;
    return PR_HANDLED(cmd);
  }

  (void) pr_table_remove(session.notes, "mod_auth.anon-passwd", NULL);

  if (res == 0) {
    unsigned int max_logins, *max = NULL;
    char *denymsg = NULL;

    /* check for AccessDenyMsg */
    if ((denymsg = get_param_ptr((session.anon_config ?
        session.anon_config->subset : cmd->server->conf),
        "AccessDenyMsg", FALSE)) != NULL) {

      if (strstr(denymsg, "%u") != NULL) {
        denymsg = sreplace(cmd->tmp_pool, denymsg, "%u", user, NULL);
      }
    }

    max = get_param_ptr(main_server->conf, "MaxLoginAttempts", FALSE);
    if (max != NULL) {
      max_logins = *max;

    } else {
      max_logins = 3;
    }

    if (max_logins > 0 &&
        ++auth_tries >= max_logins) {
      if (denymsg) {
        pr_response_send(R_530, "%s", denymsg);

      } else {
        pr_response_send(R_530, "%s", _("Login incorrect."));
      }

      pr_log_auth(PR_LOG_NOTICE,
        "Maximum login attempts (%u) exceeded, connection refused", max_logins);

      /* Generate an event about this limit being exceeded. */
      pr_event_generate("mod_auth.max-login-attempts", session.c);

      pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_CONFIG_ACL,
        "Denied by MaxLoginAttempts");
    }

    return PR_ERROR_MSG(cmd, R_530, denymsg ? denymsg : _("Login incorrect."));
  }

  return PR_HANDLED(cmd);
}

MODRET auth_acct(cmd_rec *cmd) {
  pr_response_add(R_502, _("ACCT command not implemented"));
  return PR_HANDLED(cmd);
}

MODRET auth_rein(cmd_rec *cmd) {
  pr_response_add(R_502, _("REIN command not implemented"));
  return PR_HANDLED(cmd);
}

/* Configuration handlers
 */

MODRET set_accessdenymsg(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  c = add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_accessgrantmsg(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  c = add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_anonrequirepassword(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ANON);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

MODRET set_anonrejectpasswords(cmd_rec *cmd) {
#ifdef PR_USE_REGEX
  pr_regex_t *pre = NULL;
  int res;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ANON);

  pre = pr_regexp_alloc(&auth_module);

  res = pr_regexp_compile(pre, cmd->argv[1], REG_EXTENDED|REG_NOSUB);
  if (res != 0) {
    char errstr[200] = {'\0'};

    pr_regexp_error(res, pre, errstr, 200);
    pr_regexp_free(NULL, pre);

    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "Unable to compile regex '",
      cmd->argv[1], "': ", errstr, NULL));
  }

  (void) add_config_param(cmd->argv[0], 1, (void *) pre);
  return PR_HANDLED(cmd);

#else
  CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "The ", cmd->argv[0], " directive "
    "cannot be used on this system, as you do not have POSIX compliant "
    "regex support", NULL));
#endif
}

MODRET set_authaliasonly(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  c->flags |= CF_MERGEDOWN;
  return PR_HANDLED(cmd);
}

MODRET set_authusingalias(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ANON);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

MODRET set_createhome(cmd_rec *cmd) {
  int bool = -1, start = 2;
  mode_t mode = (mode_t) 0700, dirmode = (mode_t) 0711;
  char *skel_path = NULL;
  config_rec *c = NULL;
  uid_t cuid = 0;
  gid_t cgid = 0, hgid = -1;

  if (cmd->argc-1 < 1)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  /* No need to process the rest if bool is FALSE. */
  if (bool == FALSE) {
    c = add_config_param(cmd->argv[0], 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
    *((unsigned char *) c->argv[0]) = bool;

    return PR_HANDLED(cmd);
  }

  /* Check the mode parameter, if present */
  if (cmd->argc-1 >= 2 &&
      strcasecmp(cmd->argv[2], "dirmode") != 0 &&
      strcasecmp(cmd->argv[2], "skel") != 0) {
    char *tmp = NULL;

    mode = strtol(cmd->argv[2], &tmp, 8);

    if (tmp && *tmp)
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": bad mode parameter: '",
        cmd->argv[2], "'", NULL));

    start = 3;
  }

  if (cmd->argc-1 > 2) {
    register unsigned int i;

    /* Cycle through the rest of the parameters */
    for (i = start; i < cmd->argc;) {
      if (strcasecmp(cmd->argv[i], "skel") == 0) {
        struct stat st;

        /* Check that the skel directory, if configured, meets the
         * requirements.
         */

        skel_path = cmd->argv[++i];

        if (*skel_path != '/')
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "skel path '",
            skel_path, "' is not a full path", NULL));

        if (pr_fsio_stat(skel_path, &st) < 0)
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unable to stat '",
            skel_path, "': ", strerror(errno), NULL));

        if (!S_ISDIR(st.st_mode))
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", skel_path,
            "' is not a directory", NULL));

        /* Must not be world-writable. */
        if (st.st_mode & S_IWOTH)
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", skel_path,
            "' is world-writable", NULL));

        /* Move the index past the skel parameter */
        i++;

      } else if (strcasecmp(cmd->argv[i], "dirmode") == 0) {
        char *tmp = NULL;

        dirmode = strtol(cmd->argv[++i], &tmp, 8);
 
        if (tmp && *tmp)
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "bad mode parameter: '",
            cmd->argv[i], "'", NULL));

        /* Move the index past the dirmode parameter */
        i++;

      } else if (strcasecmp(cmd->argv[i], "uid") == 0) {

        /* Check for a "~" parameter. */
        if (strncmp(cmd->argv[i+1], "~", 2) != 0) {
          char *tmp = NULL;
          uid_t uid;

          uid = strtol(cmd->argv[++i], &tmp, 10);

          if (tmp && *tmp) {
            CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "bad UID parameter: '",
              cmd->argv[i], "'", NULL));
          }

          cuid = uid;

        } else {
          cuid = (uid_t) -1;       
          i++;
        }

        /* Move the index past the uid parameter */
        i++;

      } else if (strcasecmp(cmd->argv[i], "gid") == 0) {

        /* Check for a "~" parameter. */
        if (strncmp(cmd->argv[i+1], "~", 2) != 0) {
          char *tmp = NULL;
          gid_t gid;

          gid = strtol(cmd->argv[++i], &tmp, 10);

          if (tmp && *tmp) {
            CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "bad GID parameter: '",
              cmd->argv[i], "'", NULL));
          }

          cgid = gid;

        } else {
          cgid = (gid_t) -1;
          i++;
        }

        /* Move the index past the gid parameter */
        i++;

      } else if (strcasecmp(cmd->argv[i], "homegid") == 0) {
        char *tmp = NULL;
        gid_t gid;

        gid = strtol(cmd->argv[++i], &tmp, 10);

        if (tmp && *tmp) {
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "bad GID parameter: '",
            cmd->argv[i], "'", NULL));
        }

        hgid = gid;

        /* Move the index past the homegid parameter */
        i++;

      } else {
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unknown parameter: '",
          cmd->argv[i], "'", NULL));
      }
    }
  }

  c = add_config_param(cmd->argv[0], 7, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL);

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;
  c->argv[1] = pcalloc(c->pool, sizeof(mode_t));
  *((mode_t *) c->argv[1]) = mode;
  c->argv[2] = pcalloc(c->pool, sizeof(mode_t));
  *((mode_t *) c->argv[2]) = dirmode;

  if (skel_path)
    c->argv[3] = pstrdup(c->pool, skel_path);

  c->argv[4] = pcalloc(c->pool, sizeof(uid_t));
  *((uid_t *) c->argv[4]) = cuid;
  c->argv[5] = pcalloc(c->pool, sizeof(gid_t));
  *((gid_t *) c->argv[5]) = cgid;
  c->argv[6] = pcalloc(c->pool, sizeof(gid_t));
  *((gid_t *) c->argv[6]) = hgid;
 
  return PR_HANDLED(cmd);
}

MODRET add_defaultroot(cmd_rec *cmd) {
  config_rec *c;
  char *dir,**argv;
  int argc;
  array_header *acl = NULL;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (cmd->argc < 2)
    CONF_ERROR(cmd,"syntax: DefaultRoot <directory> [<group-expression>]");

  argv = cmd->argv;
  argc = cmd->argc - 2;

  dir = *++argv;

  /* dir must be / or ~. */
  if (*dir != '/' &&
      *dir != '~')
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "(", dir, ") absolute pathname "
      "required", NULL));

  if (strchr(dir, '*'))
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "(", dir, ") wildcards not allowed "
      "in pathname", NULL));

  if (*(dir + strlen(dir) - 1) != '/')
    dir = pstrcat(cmd->tmp_pool, dir, "/", NULL);

  acl = pr_expr_create(cmd->tmp_pool, &argc, argv);

  c = add_config_param(cmd->argv[0], 0);

  c->argc = argc + 1;
  c->argv = pcalloc(c->pool, (argc + 2) * sizeof(char *));
  argv = (char **) c->argv;
  *argv++ = pstrdup(c->pool, dir);

  if (argc && acl)
    while(argc--) {
      *argv++ = pstrdup(c->pool, *((char **) acl->elts));
      acl->elts = ((char **) acl->elts) + 1;
    }

  *argv = NULL;
  return PR_HANDLED(cmd);
}

MODRET add_defaultchdir(cmd_rec *cmd) {
  config_rec *c;
  char *dir,**argv;
  int argc;
  array_header *acl = NULL;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  if (cmd->argc < 2)
    CONF_ERROR(cmd, "syntax: DefaultChdir <directory> [<group-expression>]");

  argv = cmd->argv;
  argc = cmd->argc - 2;

  dir = *++argv;

  if (strchr(dir, '*'))
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "(", dir, ") wildcards not allowed "
      "in pathname", NULL));

  if (*(dir + strlen(dir) - 1) != '/')
    dir = pstrcat(cmd->tmp_pool, dir, "/", NULL);

  acl = pr_expr_create(cmd->tmp_pool, &argc, argv);

  c = add_config_param(cmd->argv[0], 0);

  c->argc = argc + 1;
  c->argv = pcalloc(c->pool, (argc + 2) * sizeof(char *));
  argv = (char **) c->argv;
  *argv++ = pstrdup(c->pool, dir);

  if (argc && acl)
    while(argc--) {
      *argv++ = pstrdup(c->pool, *((char **) acl->elts));
      acl->elts = ((char **) acl->elts) + 1;
    }

  *argv = NULL;

  c->flags |= CF_MERGEDOWN;
  return PR_HANDLED(cmd);
}

MODRET set_displaylogin(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  c = add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_grouppassword(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  c = add_config_param_str(cmd->argv[0], 2, cmd->argv[1], cmd->argv[2]);
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_loginpasswordprompt(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

/* usage: MaxClientsPerClass class max|"none" ["message"] */
MODRET set_maxclientsclass(cmd_rec *cmd) {
  int max;
  config_rec *c;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (strcasecmp(cmd->argv[2], "none") == 0)
    max = 0;

  else {
    char *endp = NULL;

    max = (int) strtol(cmd->argv[2], &endp, 10);

    if ((endp && *endp) || max < 1)
      CONF_ERROR(cmd, "max must be 'none' or a number greater than 0");
  }

  if (cmd->argc == 4) {
    c = add_config_param(cmd->argv[0], 3, NULL, NULL, NULL);
    c->argv[0] = pstrdup(c->pool, cmd->argv[1]);
    c->argv[1] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[1]) = max;
    c->argv[2] = pstrdup(c->pool, cmd->argv[3]);

  } else {
    c = add_config_param(cmd->argv[0], 2, NULL, NULL);
    c->argv[0] = pstrdup(c->pool, cmd->argv[1]);
    c->argv[1] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[1]) = max;
  }

  return PR_HANDLED(cmd);
}

/* usage: MaxClients max|"none" ["message"] */
MODRET set_maxclients(cmd_rec *cmd) {
  int max;
  config_rec *c = NULL;

  if (cmd->argc < 2 || cmd->argc > 3)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  if (!strcasecmp(cmd->argv[1], "none"))
    max = 0;

  else {
    char *endp = NULL;

    max = (int) strtol(cmd->argv[1], &endp, 10);

    if ((endp && *endp) || max < 1)
      CONF_ERROR(cmd, "parameter must be 'none' or a number greater than 0");
  }

  if (cmd->argc == 3) {
    c = add_config_param(cmd->argv[0], 2, NULL, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[0]) = max;
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);

  } else {
    c = add_config_param(cmd->argv[0], 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[0]) = max;
  }

  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

/* usage: MaxClientsPerHost max|"none" ["message"] */
MODRET set_maxhostclients(cmd_rec *cmd) {
  int max;
  config_rec *c = NULL;

  if (cmd->argc < 2 || cmd->argc > 3)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  if (!strcasecmp(cmd->argv[1], "none"))
    max = 0;

  else {
    char *endp = NULL;

    max = (int) strtol(cmd->argv[1], &endp, 10);

    if ((endp && *endp) || max < 1)
      CONF_ERROR(cmd, "parameter must be 'none' or a number greater than 0");
  }

  if (cmd->argc == 3) {
    c = add_config_param(cmd->argv[0], 2, NULL, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[0]) = max;
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);

  } else {
    c = add_config_param(cmd->argv[0], 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[0]) = max;
  }

  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}


/* usage: MaxClientsPerUser max|"none" ["message"] */
MODRET set_maxuserclients(cmd_rec *cmd) {
  int max;
  config_rec *c = NULL;

  if (cmd->argc < 2 || cmd->argc > 3)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  if (!strcasecmp(cmd->argv[1], "none"))
    max = 0;

  else {
    char *endp = NULL;

    max = (int) strtol(cmd->argv[1], &endp, 10);

    if ((endp && *endp) || max < 1)
      CONF_ERROR(cmd, "parameter must be 'none' or a number greater than 0");
  }

  if (cmd->argc == 3) {
    c = add_config_param(cmd->argv[0], 2, NULL, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[0]) = max;
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);

  } else {
    c = add_config_param(cmd->argv[0], 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[0]) = max;
  }

  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

/* usage: MaxConnectionsPerHost max|"none" ["message"] */
MODRET set_maxconnectsperhost(cmd_rec *cmd) {
  int max;
  config_rec *c;

  if (cmd->argc < 2 || cmd->argc > 3)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (strcasecmp(cmd->argv[1], "none") == 0)
    max = 0;

  else {
    char *tmp = NULL;

    max = (int) strtol(cmd->argv[1], &tmp, 10);

    if ((tmp && *tmp) || max < 1)
      CONF_ERROR(cmd, "parameter must be 'none' or a number greater than 0");
  }

  if (cmd->argc == 3) {
    c = add_config_param(cmd->argv[0], 2, NULL, NULL);
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);

  } else
    c = add_config_param(cmd->argv[0], 1, NULL);

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = max;

  return PR_HANDLED(cmd);
}

/* usage: MaxHostsPerUser max|"none" ["message"] */
MODRET set_maxhostsperuser(cmd_rec *cmd) {
  int max;
  config_rec *c = NULL;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  if (cmd->argc < 2 || cmd->argc > 3)
    CONF_ERROR(cmd, "wrong number of parameters");

  if (!strcasecmp(cmd->argv[1], "none"))
    max = 0;

  else {
    char *endp = NULL;

    max = (int) strtol(cmd->argv[1], &endp, 10);

    if ((endp && *endp) || max < 1)
      CONF_ERROR(cmd, "parameter must be 'none' or a number greater than 0");
  }

  if (cmd->argc == 3) {
    c = add_config_param(cmd->argv[0], 2, NULL, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[0]) = max;
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);

  } else {
    c = add_config_param(cmd->argv[0], 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[0]) = max;
  }

  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_maxloginattempts(cmd_rec *cmd) {
  int max;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (strcasecmp(cmd->argv[1], "none") == 0) {
    max = 0;

  } else {
    char *endp = NULL;
    max = (int) strtol(cmd->argv[1], &endp, 10);

    if ((endp && *endp) || max < 1)
      CONF_ERROR(cmd, "parameter must be 'none' or a number greater than 0");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = max;

  return PR_HANDLED(cmd);
}

MODRET set_requirevalidshell(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

/* usage: RewriteHome on|off */
MODRET set_rewritehome(cmd_rec *cmd) {
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

MODRET set_rootlogin(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd,1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = (unsigned char) bool;
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_rootrevoke(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = (unsigned char) bool;

  c->flags |= CF_MERGEDOWN;
  return PR_HANDLED(cmd);
}

MODRET set_timeoutlogin(cmd_rec *cmd) {
  int timeout = -1;
  char *endp = NULL;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  timeout = (int) strtol(cmd->argv[1], &endp, 10);

  if ((endp && *endp) || timeout < 0 || timeout > 65535)
    CONF_ERROR(cmd, "timeout values must be between 0 and 65535");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = timeout;

  return PR_HANDLED(cmd);
}

MODRET set_timeoutsession(cmd_rec *cmd) {
  int seconds = 0, precedence = 0;
  config_rec *c = NULL;

  int ctxt = (cmd->config && cmd->config->config_type != CONF_PARAM ?
     cmd->config->config_type : cmd->server->config_type ?
     cmd->server->config_type : CONF_ROOT);

  /* this directive must have either 1 or 3 arguments */
  if (cmd->argc-1 != 1 && cmd->argc-1 != 3)
    CONF_ERROR(cmd, "missing arguments");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  /* Set the precedence for this config_rec based on its configuration
   * context.
   */
  if (ctxt & CONF_GLOBAL) {
    precedence = 1;

  /* These will never appear simultaneously */
  } else if ((ctxt & CONF_ROOT) ||
             (ctxt & CONF_VIRTUAL)) {
    precedence = 2;

  } else if (ctxt & CONF_ANON) {
    precedence = 3;
  }

  seconds = atoi(cmd->argv[1]);
  if (seconds < 0) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      "seconds must be greater than or equal to 0", NULL));

  } else if (seconds == 0) {
    /* do nothing */
    return PR_HANDLED(cmd);
  }

  if (cmd->argc-1 == 3) {
    if (strncmp(cmd->argv[2], "user", 5) == 0 ||
        strncmp(cmd->argv[2], "group", 6) == 0 ||
        strncmp(cmd->argv[2], "class", 6) == 0) {

       /* no op */

     } else {
       CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, cmd->argv[0],
         ": unknown classifier used: '", cmd->argv[2], "'", NULL));
    }
  }

  if (cmd->argc-1 == 1) {
    c = add_config_param(cmd->argv[0], 2, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[0]) = seconds;
    c->argv[1] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[1]) = precedence;

  } else if (cmd->argc-1 == 3) {
    array_header *acl = NULL;
    int argc = cmd->argc - 3;
    char **argv = cmd->argv + 2;

    acl = pr_expr_create(cmd->tmp_pool, &argc, argv);

    c = add_config_param(cmd->argv[0], 0);
    c->argc = argc + 2;

    /* add 3 to argc for the argv of the config_rec: one for the
     * seconds value, one for the precedence, one for the classifier,
     * and one for the terminating NULL
     */
    c->argv = pcalloc(c->pool, ((argc + 4) * sizeof(char *)));

    /* capture the config_rec's argv pointer for doing the by-hand
     * population
     */
    argv = (char **) c->argv;

    /* Copy in the seconds. */
    *argv = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) *argv++) = seconds;

    /* Copy in the precedence. */
    *argv = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) *argv++) = precedence;

    /* Copy in the classifier. */
    *argv++ = pstrdup(c->pool, cmd->argv[2]);

    /* now, copy in the expression arguments */
    if (argc && acl) {
      while (argc--) {
        *argv++ = pstrdup(c->pool, *((char **) acl->elts));
        acl->elts = ((char **) acl->elts) + 1;
      }
    }

    /* don't forget the terminating NULL */
    *argv = NULL;

  } else {
    /* Should never reach here. */
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  c->flags |= CF_MERGEDOWN_MULTI;
  return PR_HANDLED(cmd);
}

MODRET set_useftpusers(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

/* usage: UseLastlog on|off */
MODRET set_uselastlog(cmd_rec *cmd) {
#ifdef PR_USE_LASTLOG
  int bool;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
#else
  CONF_ERROR(cmd, "requires lastlog support (--with-lastlog)");
#endif /* PR_USE_LASTLOG */
}

/* usage: UserAlias alias real-user */
MODRET set_useralias(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  /* Make sure that the given names differ. */
  if (strcmp(cmd->argv[1], cmd->argv[2]) == 0)
    CONF_ERROR(cmd, "alias and real user names must differ");

  c = add_config_param_str(cmd->argv[0], 2, cmd->argv[1], cmd->argv[2]);

  /* Note: only merge this directive down if it is not appearing in an
   * <Anonymous> context.
   */
  if (!check_context(cmd, CONF_ANON))
    c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_userdirroot(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ANON);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

MODRET set_userpassword(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  c = add_config_param_str(cmd->argv[0], 2, cmd->argv[1], cmd->argv[2]);
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

/* Module API tables
 */

static conftable auth_conftab[] = {
  { "AccessDenyMsg",		set_accessdenymsg,		NULL },
  { "AccessGrantMsg",		set_accessgrantmsg,		NULL },
  { "AnonRequirePassword",	set_anonrequirepassword,	NULL },
  { "AnonRejectPasswords",	set_anonrejectpasswords,	NULL },
  { "AuthAliasOnly",		set_authaliasonly,		NULL },
  { "AuthUsingAlias",		set_authusingalias,		NULL },
  { "CreateHome",		set_createhome,			NULL },
  { "DefaultChdir",		add_defaultchdir,		NULL },
  { "DefaultRoot",		add_defaultroot,		NULL },
  { "DisplayLogin",		set_displaylogin,		NULL },
  { "GroupPassword",		set_grouppassword,		NULL },
  { "LoginPasswordPrompt",	set_loginpasswordprompt,	NULL },
  { "MaxClients",		set_maxclients,			NULL },
  { "MaxClientsPerClass",	set_maxclientsclass,		NULL },
  { "MaxClientsPerHost",	set_maxhostclients,		NULL },
  { "MaxClientsPerUser",	set_maxuserclients,		NULL },
  { "MaxConnectionsPerHost",	set_maxconnectsperhost,		NULL },
  { "MaxHostsPerUser",		set_maxhostsperuser,		NULL },
  { "MaxLoginAttempts",		set_maxloginattempts,		NULL },
  { "RequireValidShell",	set_requirevalidshell,		NULL },
  { "RewriteHome",		set_rewritehome,		NULL },
  { "RootLogin",		set_rootlogin,			NULL },
  { "RootRevoke",		set_rootrevoke,			NULL },
  { "TimeoutLogin",		set_timeoutlogin,		NULL },
  { "TimeoutSession",		set_timeoutsession,		NULL },
  { "UseFtpUsers",		set_useftpusers,		NULL },
  { "UseLastlog",		set_uselastlog,			NULL },
  { "UserAlias",		set_useralias,			NULL },
  { "UserDirRoot",		set_userdirroot,		NULL },
  { "UserPassword",		set_userpassword,		NULL },
  { NULL,			NULL,				NULL }
};

static cmdtable auth_cmdtab[] = {
  { PRE_CMD,	C_USER,	G_NONE,	auth_pre_user,	FALSE,	FALSE,	CL_AUTH },
  { CMD,	C_USER,	G_NONE,	auth_user,	FALSE,	FALSE,	CL_AUTH },
  { PRE_CMD,	C_PASS,	G_NONE,	auth_pre_pass,	FALSE,	FALSE,	CL_AUTH },
  { CMD,	C_PASS,	G_NONE,	auth_pass,	FALSE,	FALSE,	CL_AUTH },
  { POST_CMD,	C_PASS,	G_NONE,	auth_post_pass,	FALSE,	FALSE,	CL_AUTH },
  { LOG_CMD,	C_PASS,	G_NONE,	auth_log_pass,  FALSE,  FALSE },
  { LOG_CMD_ERR,C_PASS,	G_NONE,	auth_err_pass,  FALSE,  FALSE },
  { CMD,	C_ACCT,	G_NONE,	auth_acct,	FALSE,	FALSE,	CL_AUTH },
  { CMD,	C_REIN,	G_NONE,	auth_rein,	FALSE,	FALSE,	CL_AUTH },
  { 0, NULL }
};

/* Module interface */

module auth_module = {
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "auth",

  /* Module configuration directive table */
  auth_conftab,	

  /* Module command handler table */
  auth_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  auth_init,

  /* Session initialization function */
  auth_sess_init
};

