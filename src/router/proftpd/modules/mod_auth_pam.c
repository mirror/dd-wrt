/*
 * ProFTPD: mod_auth_pam -- Support for PAM-style authentication.
 * Copyright (c) 1998, 1999, 2000 Habeeb J. Dihu aka
 *   MacGyver <macgyver@tos.net>, All Rights Reserved.
 * Copyright 2000-2009 The ProFTPD Project
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
 * PAM module from ProFTPD
 *
 * This module should work equally well under all Linux distributions (which
 * have PAM support), as well as Solaris 2.5 and above.
 *
 * If you have any problems, questions, comments, or suggestions regarding
 * this module, please feel free to contact Habeeb J. Dihu aka MacGyver
 * <macgyver@tos.net>.
 *
 * -- DO NOT MODIFY THE TWO LINES BELOW --
 * $Libraries: -lpam$
 * $Id: mod_auth_pam.c,v 1.24 2009/03/05 05:24:06 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"

#ifdef HAVE_PAM

#define MOD_AUTH_PAM_VERSION		"mod_auth_pam/1.1"

#ifdef HAVE_SECURITY_PAM_APPL_H
# ifdef HPUX11
#  ifndef COMSEC
#    define COMSEC 1
#  endif
# endif /* HPUX11 */
# include <security/pam_appl.h>
#endif /* HAVE_SECURITY_PAM_APPL_H */

#ifdef HAVE_SECURITY_PAM_MODULES_H
# include <security/pam_modules.h>
#endif /* HAVE_SECURITY_PAM_MODULES_H */

/* Needed for the MAXLOGNAME restriction. */
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#ifdef HAVE_PAM_PAM_APPL_H
#include <pam/pam_appl.h>
#endif /* HAVE_PAM_PAM_APPL_H */

module auth_pam_module;
static authtable auth_pam_authtab[2];

static pam_handle_t *	pamh			= NULL;
static char *		pamconfig		= "ftp";
static char *		pam_user 		= NULL;
static char *		pam_pass 		= NULL;
static size_t		pam_user_len		= 0;
static size_t		pam_pass_len		= 0;
static int		pam_conv_error		= 0;

static unsigned long auth_pam_opts = 0UL;
#define AUTH_PAM_OPT_NO_TTY	0x0001

static const char *trace_channel = "pam";

/* On non-Solaris systems, the struct pam_message argument is declared
 * const, but on Solaris, it isn't.  To avoid compiler warnings about
 * incompatible pointer types, we need to use const or not as appropriate.
 */
#ifndef SOLARIS2
# define PR_PAM_CONST	const
#else
# define PR_PAM_CONST
#endif

static int pam_exchange(int num_msg, PR_PAM_CONST struct pam_message **msg,
    struct pam_response **resp, void *appdata_ptr) {
  register unsigned int i;
  struct pam_response *response = NULL;

  if (num_msg <= 0)
    return PAM_CONV_ERR;

  response = calloc(num_msg, sizeof(struct pam_response));

  if (response == NULL)
    return PAM_CONV_ERR;

  for (i = 0; i < num_msg; i++) {
    response[i].resp_retcode = 0; /* PAM_SUCCESS; */

    switch (msg[i]->msg_style) {
    case PAM_PROMPT_ECHO_ON:
      pr_trace_msg(trace_channel, 9, "received PAM_PROMPT_ECHO_ON message");

      /* PAM frees response and resp.  If you don't believe this, please read
       * the actual PAM RFCs as well as have a good look at libpam.
       */
      response[i].resp = pam_user ? strdup(pam_user) : NULL;
      break;

    case PAM_PROMPT_ECHO_OFF:
      pr_trace_msg(trace_channel, 9, "received PAM_PROMPT_ECHO_OFF message");

      /* PAM frees response and resp.  If you don't believe this, please read
       * the actual PAM RFCs as well as have a good look at libpam.
       */
      response[i].resp = pam_pass ? strdup(pam_pass) : NULL;
      break;

    case PAM_TEXT_INFO:
    case PAM_ERROR_MSG:
      pr_trace_msg(trace_channel, 9, "received %s response: %s",
        msg[i]->msg_style == PAM_TEXT_INFO ? "PAM_TEXT_INFO" : "PAM_ERROR_MSG",
        msg[i]->msg);

      /* Ignore it, but pam still wants a NULL response... */
      response[i].resp = NULL;
      break;

    default:
      /* Must be an error of some sort... */
      free(response[i].resp);
      free(response);

      pam_conv_error = 1;
      return PAM_CONV_ERR;
    }
  }

  *resp = response;
  return PAM_SUCCESS;
}

static struct pam_conv pam_conv = {
  &pam_exchange,
  NULL
};

static void auth_pam_exit_ev(const void *event_data, void *user_data) {
  int pam_error = 0;

  /* Sanity check.
   */
  if (pamh == NULL)
    return;

  /* We need privileges to be able to write to things like lastlog and
   * friends.
   */
  pr_signals_block();
  PRIVS_ROOT

  /* Give up our credentials, close our session, and finally close out this
   * instance of PAM authentication.
   */
#ifdef PAM_CRED_DELETE
  pam_error = pam_setcred(pamh, PAM_CRED_DELETE);
#else
  pam_error = pam_setcred(pamh, PAM_DELETE_CRED);
#endif /* !PAM_CRED_DELETE */
  if (pam_error != PAM_SUCCESS) {
    pr_trace_msg(trace_channel, 1,
      "error setting PDM_DELETE_CRED credential: %s",
      pam_strerror(pamh, pam_error));
  }

  pam_error = pam_close_session(pamh, PAM_SILENT);
  if (pam_error != PAM_SUCCESS) {
    pr_trace_msg(trace_channel, 1, "error closing PAM session: %s",
      pam_strerror(pamh, pam_error));
  }

#ifndef SOLARIS2
  pam_end(pamh, 0);
  pamh = NULL;
#endif

  if (pam_user != NULL) {
    memset(pam_user, '\0', pam_user_len);
    free(pam_user);
    pam_user = NULL;
    pam_user_len = 0;
  }

  PRIVS_RELINQUISH
  pr_signals_unblock();
}

MODRET pam_auth(cmd_rec *cmd) {
  int pam_error = 0, retval = PR_AUTH_ERROR, success = 0;
  config_rec *c = NULL;
  unsigned char *auth_pam = NULL, pam_authoritative = FALSE;
  char ttyentry[32];

  /* If we have been explicitly disabled, return now.  Otherwise,
   * the module is considered enabled.
   */
  auth_pam = get_param_ptr(main_server->conf, "AuthPAM", FALSE);
  if (auth_pam != NULL &&
      *auth_pam == FALSE) {
    return PR_DECLINED(cmd);
  }

  /* Figure out our default return style: whether or not PAM should allow
   * other auth modules a shot at this user or not is controlled by adding
   * '*' to a module name in the AuthOrder directive.  By default, auth
   * modules are not authoritative, and allow other auth modules a chance at
   * authenticating the user.  This is not the most secure configuration, but
   * it allows things like AuthUserFile to work "out of the box".
   */
  if (auth_pam_authtab[0].auth_flags & PR_AUTH_FL_REQUIRED) {
    pam_authoritative = TRUE;
  }

  /* Just in case...
   */
  if (cmd->argc != 2)
    return pam_authoritative ? PR_ERROR(cmd) : PR_DECLINED(cmd);

  /* Allocate our entries...we free these up at the end of the authentication.
   */
  if ((pam_user_len = strlen(cmd->argv[0]) + 1) > (PAM_MAX_MSG_SIZE + 1))
    pam_user_len = PAM_MAX_MSG_SIZE + 1;

#ifdef MAXLOGNAME
  /* Some platforms' PAM libraries do not handle login strings that
   * exceed this length.
   */
  if (pam_user_len > MAXLOGNAME) {
    pr_log_pri(PR_LOG_NOTICE,
      "PAM(%s): Name exceeds maximum login length (%u)", cmd->argv[0],
      MAXLOGNAME);
    pr_trace_msg(trace_channel, 1,
      "user name '%s' exceeds maximum login length %u, declining",
      cmd->argv[0], MAXLOGNAME);
    return PR_DECLINED(cmd);
  }
#endif
  pam_user = malloc(pam_user_len);
  if (pam_user == NULL)
    return pam_authoritative ? PR_ERROR(cmd) : PR_DECLINED(cmd);

  sstrncpy(pam_user, cmd->argv[0], pam_user_len);

  if ((pam_pass_len = strlen(cmd->argv[1]) + 1) > (PAM_MAX_MSG_SIZE + 1))
    pam_pass_len = PAM_MAX_MSG_SIZE + 1;
 
  pam_pass = malloc(pam_pass_len);
  if (pam_pass == NULL) {
    memset(pam_user, '\0', pam_user_len);
    free(pam_user);
    pam_user = NULL;
    pam_user_len = 0;
    pam_pass_len = 0;
    return pam_authoritative ? PR_ERROR(cmd) : PR_DECLINED(cmd);
  }

  sstrncpy(pam_pass, cmd->argv[1], pam_pass_len);

  /* Check for which PAM config file to use.  Since we have many different
   * potential servers, they may each require a separate type of PAM
   * authentication.
   */
  c = find_config(main_server->conf, CONF_PARAM, "AuthPAMConfig", FALSE);
  if (c != NULL) {
    pamconfig = c->argv[0];
    pr_trace_msg(trace_channel, 8, "using PAM service name '%s'", pamconfig);
  }

  /* Check for minor PAM configuration options such as use of PAM_TTY. */
  c = find_config(main_server->conf, CONF_PARAM, "AuthPAMOptions", FALSE);
  if (c != NULL) {
    auth_pam_opts = *((unsigned long *) c->argv[0]);
  }

#ifdef SOLARIS2
  /* For Solaris environments, the TTY environment will always be set,
   * in order to workaround a bug (Solaris Bug ID 4250887) where
   * pam_open_session() will crash unless both PAM_RHOST and PAM_TTY are
   * set, and the PAM_TTY setting is at least greater than the length of
   * the string "/dev/".
   */
  auth_pam_opts &= ~AUTH_PAM_OPT_NO_TTY;
#endif /* SOLARIS2 */

  /* Due to the different types of authentication used, such as shadow
   * passwords, etc. we need root privs for this operation.
   */
  pr_signals_block();
  PRIVS_ROOT

  /* The order of calls into PAM should be as follows, according to Sun's
   * documentation at http://www.sun.com/software/solaris/pam/:
   *
   * pam_start()
   * pam_authenticate()
   * pam_acct_mgmt()
   * pam_open_session()
   * pam_setcred()
   */
  pam_error = pam_start(pamconfig, pam_user, &pam_conv, &pamh);
  if (pam_error != PAM_SUCCESS)
    goto done;

  pam_set_item(pamh, PAM_RUSER, pam_user);

  /* Set our host environment for PAM modules that check host information.
   */
  if (session.c != NULL)
    pam_set_item(pamh, PAM_RHOST, session.c->remote_name);
  else
    pam_set_item(pamh, PAM_RHOST, "IHaveNoIdeaHowIGotHere");

  if (!(auth_pam_opts & AUTH_PAM_OPT_NO_TTY)) {
    memset(ttyentry, '\0', sizeof(ttyentry));
    snprintf(ttyentry, sizeof(ttyentry), "/dev/ftpd%02lu",
      (unsigned long) getpid());
    ttyentry[sizeof(ttyentry)-1] = '\0';

    pr_trace_msg(trace_channel, 9, "setting PAM_TTY to '%s'", ttyentry);
    pam_set_item(pamh, PAM_TTY, ttyentry);
  }

  /* Authenticate, and get any credentials as needed.
   */
  pam_error = pam_authenticate(pamh, PAM_SILENT);

  if (pam_error != PAM_SUCCESS) {
    switch (pam_error) {
      case PAM_USER_UNKNOWN:
        retval = PR_AUTH_NOPWD;
        break;

      default:
        retval = PR_AUTH_BADPWD;
        break;
    }

    pr_trace_msg(trace_channel, 1,
      "authentication error (%d) for user '%s': %s", pam_error, cmd->argv[0],
      pam_strerror(pamh, pam_error));
    goto done;
  }

  if (pam_conv_error != 0) {
    retval = PR_AUTH_BADPWD;
    goto done;
  }

  pam_error = pam_acct_mgmt(pamh, PAM_SILENT);

  if (pam_error != PAM_SUCCESS) {
    switch (pam_error) {
#ifdef PAM_AUTHTOKEN_REQD
      case PAM_AUTHTOKEN_REQD:
        pr_trace_msg(trace_channel, 8,
          "account mgmt error: PAM_AUTHTOKEN_REQD");
        retval = PR_AUTH_AGEPWD;
        break;
#endif /* PAM_AUTHTOKEN_REQD */

      case PAM_ACCT_EXPIRED:
        pr_trace_msg(trace_channel, 8, "account mgmt error: PAM_ACCT_EXPIRED");
        retval = PR_AUTH_DISABLEDPWD;
        break;

#ifdef PAM_ACCT_DISABLED
      case PAM_ACCT_DISABLED:
        pr_trace_msg(trace_channel, 8, "account mgmt error: PAM_ACCT_DISABLED");
        retval = PR_AUTH_DISABLEDPWD;
        break;
#endif /* PAM_ACCT_DISABLED */

      case PAM_USER_UNKNOWN:
        pr_trace_msg(trace_channel, 8, "account mgmt error: PAM_USER_UNKNOWN");
        retval = PR_AUTH_NOPWD;
        break;

      default:
        pr_trace_msg(trace_channel, 8, "account mgmt error: (unknown) [%d]",
          pam_error);
        retval = PR_AUTH_BADPWD;
        break;
    }

    pr_log_pri(PR_LOG_NOTICE, "PAM(%s): %s", cmd->argv[0],
      pam_strerror(pamh, pam_error));
    goto done;
  }

  /* Open the session. */
  pam_error = pam_open_session(pamh, PAM_SILENT);

  if (pam_error != PAM_SUCCESS) {
    switch (pam_error) {
      case PAM_SESSION_ERR:
      default:
        retval = PR_AUTH_DISABLEDPWD;
        break;
    }

    pr_log_pri(PR_LOG_NOTICE, "PAM(%s): %s", cmd->argv[0],
      pam_strerror(pamh, pam_error));
    goto done;
  }

  /* Finally, establish credentials. */
#ifdef PAM_CRED_ESTABLISH
  pam_error = pam_setcred(pamh, PAM_CRED_ESTABLISH);
#else
  pam_error = pam_setcred(pamh, PAM_ESTABLISH_CRED);
#endif /* !PAM_CRED_ESTABLISH */

  if (pam_error != PAM_SUCCESS) {
    switch (pam_error) {
      case PAM_CRED_EXPIRED:
        pr_trace_msg(trace_channel, 8, "credentials error: PAM_CRED_EXPIRED");
        retval = PR_AUTH_AGEPWD;
        break;

      case PAM_USER_UNKNOWN:
        pr_trace_msg(trace_channel, 8, "credentials error: PAM_USER_UNKNOWN");
        retval = PR_AUTH_NOPWD;
        break;

      default:
        pr_trace_msg(trace_channel, 8, "credentials error: (unknown) [%d]",
          pam_error);
        retval = PR_AUTH_BADPWD;
        break;
    }

    pr_log_pri(PR_LOG_NOTICE, "PAM(%s): %s", cmd->argv[0],
      pam_strerror(pamh, pam_error));
    goto done;
  }

  success++;

 done:

  /* And we're done.  Clean up and relinquish our root privs.  */

#if defined(SOLARIS2) || defined(HPUX10) || defined(HPUX11)
  if (success)
    pam_error = pam_close_session(pamh, 0);

  if (pamh)
    pam_end(pamh, pam_error);
  pamh = NULL;
#endif

  if (pam_pass != NULL) {
    pr_memscrub(pam_pass, pam_pass_len);
    free(pam_pass);
    pam_pass = NULL;
    pam_pass_len = 0;
  }

  PRIVS_RELINQUISH
  pr_signals_unblock();

  if (!success) {
    if (pam_user != NULL) {
      memset(pam_user, '\0', pam_user_len);
      free(pam_user);
      pam_user = NULL;
      pam_user_len = 0;
    }

    return pam_authoritative ? PR_ERROR_INT(cmd, retval) : PR_DECLINED(cmd);

  } else {
    session.auth_mech = "mod_auth_pam.c";
    pr_event_register(&auth_pam_module, "core.exit", auth_pam_exit_ev, NULL);
    return PR_HANDLED(cmd);
  }
}

/* Configuration handlers
 */

MODRET set_authpam(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

MODRET set_authpamconfig(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: AuthPAMOptions opt1 opt2 ... */
MODRET set_authpamoptions(cmd_rec *cmd) {
  config_rec *c = NULL;
  register unsigned int i = 0;
  unsigned long opts = 0UL;

  if (cmd->argc-1 == 0)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = add_config_param(cmd->argv[0], 1, NULL);

  for (i = 1; i < cmd->argc; i++) {
    if (strcasecmp(cmd->argv[i], "NoTTY") == 0) {
      opts |= AUTH_PAM_OPT_NO_TTY;

    } else {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown AuthPAMOption: '",
        cmd->argv[i], "'", NULL));
    }
  }

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned long));
  *((unsigned long *) c->argv[0]) = opts;

  return PR_HANDLED(cmd);
}

/* Initialization routines
 */

static int auth_pam_sess_init(void) {
  if (pr_auth_add_auth_only_module("mod_auth_pam.c") < 0) {
    pr_log_pri(PR_LOG_NOTICE, MOD_AUTH_PAM_VERSION
      ": unable to add 'mod_auth_pam.c' as an auth-only module: %s",
      strerror(errno));
    return -1;
  }

  return 0;
}

static authtable auth_pam_authtab[] = {
  { 0, "auth", pam_auth },
  { 0, NULL, NULL }
};

static conftable auth_pam_conftab[] = {
  { "AuthPAM",			set_authpam,			NULL },
  { "AuthPAMConfig",		set_authpamconfig,		NULL },
  { "AuthPAMOptions",		set_authpamoptions,		NULL },
  { NULL, NULL, NULL }
};

module auth_pam_module = {
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "auth_pam",

  /* Module configuration handler table */
  auth_pam_conftab,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  auth_pam_authtab,

  /* Module initialization */
  NULL,

  /* Session initialization */
  auth_pam_sess_init,

  /* Module version */
  MOD_AUTH_PAM_VERSION
};

#endif /* HAVE_PAM */
