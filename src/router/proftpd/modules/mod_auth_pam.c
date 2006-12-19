/*
 * ProFTPD: mod_auth_pam -- Support for PAM-style authentication.
 * Copyright (c) 1998, 1999, 2000 Habeeb J. Dihu aka
 *   MacGyver <macgyver@tos.net>, All Rights Reserved.
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
 * $Id: mod_auth_pam.c,v 1.1 2006/04/24 11:39:28 honor Exp $
 */

#include "conf.h"
#include "privs.h"

#ifdef HAVE_PAM

#ifdef HAVE_SECURITY_PAM_APPL_H
# ifdef HPUX11
#  ifndef COMSEC
#    define COMSEC 1
#  endif
# endif /* HPUX11 */
# include <security/pam_appl.h>
#endif /* HAVE_SECURITY_PAM_APPL_H */

/* Needed for the MAXLOGNAME restriction. */
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#ifdef HAVE_PAM_PAM_APPL_H
#include <pam/pam_appl.h>
#endif /* HAVE_PAM_PAM_APPL_H */

module auth_pam_module;

static pam_handle_t *	pamh			= NULL;
static char *		pamconfig		= "ftp";
static char *		pam_user 		= NULL;
static char *		pam_pass 		= NULL;
static size_t		pam_user_len		= 0;
static size_t		pam_pass_len		= 0;
static int		pam_conv_error		= 0;

static int pam_exchange(num_msg, msg, resp, appdata_ptr)
     int num_msg;
     struct pam_message **msg;
     struct pam_response **resp;
     void *appdata_ptr;
{
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
      /* PAM frees response and resp.  If you don't believe this, please read
       * the actual PAM RFCs as well as have a good look at libpam.
       */
      response[i].resp = pam_user ? strdup(pam_user) : NULL;
      break;

    case PAM_PROMPT_ECHO_OFF:
      /* PAM frees response and resp.  If you don't believe this, please read
       * the actual PAM RFCs as well as have a good look at libpam.
       */
      response[i].resp = pam_pass ? strdup(pam_pass) : NULL;
      break;

    case PAM_TEXT_INFO:
    case PAM_ERROR_MSG:
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
  if (pam_error != PAM_SUCCESS)
    pr_log_pri(PR_LOG_NOTICE, "PAM(setcred): %s",
      pam_strerror(pamh, pam_error));

  pam_error = pam_close_session(pamh, PAM_SILENT);
  if (pam_error != PAM_SUCCESS)
    pr_log_pri(PR_LOG_NOTICE, "PAM(close_session): %s",
      pam_strerror(pamh, pam_error));

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
  unsigned char *auth_pam = NULL, *auth_pam_authoritative = NULL,
    pam_authoritative = FALSE;

#ifdef SOLARIS2
  char ttyentry[32];
#endif /* SOLARIS2 */

  /* If we have been explicitly disabled, return now.  Otherwise,
   * the module is considered enabled.
   */
  if ((auth_pam = get_param_ptr(main_server->conf, "AuthPAM",
      FALSE)) != NULL && *auth_pam == FALSE)
    return DECLINED(cmd);

  /* Figure out our default return style: whether or not PAM should allow
   * other auth modules a shot at this user or not is controlled by the
   * AuthPAMAuthoritative directive..  It defaults to "no", meaning that PAM
   * is not authoritative, and allows other auth modules a chance at
   * authenticating the user.  This is not the most secure configuration, but
   * it allows things like AuthUserFile to work "out of the box".
   */
  if ((auth_pam_authoritative = get_param_ptr(main_server->conf,
      "AuthPAMAuthoritative", FALSE)) != NULL &&
      *auth_pam_authoritative == TRUE)
    pam_authoritative = TRUE;

  /* Just in case...
   */
  if (cmd->argc != 2)
    return pam_authoritative ? ERROR(cmd) : DECLINED(cmd);

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
    return DECLINED(cmd);
  }
#endif
  if ((pam_user = malloc(pam_user_len)) == NULL)
    return pam_authoritative ? ERROR(cmd) : DECLINED(cmd);

  sstrncpy(pam_user, cmd->argv[0], pam_user_len);

  if ((pam_pass_len = strlen(cmd->argv[1]) + 1) > (PAM_MAX_MSG_SIZE + 1))
    pam_pass_len = PAM_MAX_MSG_SIZE + 1;
 
  if ((pam_pass = malloc(pam_pass_len)) == NULL) {
    memset(pam_user, '\0', pam_user_len);
    free(pam_user);
    pam_user = NULL;
    pam_user_len = 0;
    pam_pass_len = 0;
    return pam_authoritative ? ERROR(cmd) : DECLINED(cmd);
  }

  sstrncpy(pam_pass, cmd->argv[1], pam_pass_len);

  /* Check for which PAM config file to use.  Since we have many different
   * potential servers, they may each require a separate type of PAM
   * authentication.
   */
  if ((c = find_config(main_server->conf, CONF_PARAM, "AuthPAMConfig",
      FALSE)) != NULL)
    pamconfig = c->argv[0];

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

  /* Set our host environment for PAM modules that check host information.
   */
  if (session.c != NULL)
    pam_set_item(pamh, PAM_RHOST, session.c->remote_name);
  else
    pam_set_item(pamh, PAM_RHOST, "IHaveNoIdeaHowIGotHere");

#ifdef SOLARIS2
  /* Set our TTY environment.  This is apparently required for Solaris
   * environments, since unless PAM_RHOST and PAM_TTY are defined, and
   * the string given to PAM_TTY must be of the form (or at least greater
   * than the length of) "/dev/", pam_open_session() will crash and burn
   * a horrible death that took many hours to debug...YUCK.
   *
   * This bug is Sun bugid 4250887, and should be fixed in an update for
   * Solaris.  -- MacGyver
   */
  snprintf(ttyentry, sizeof(ttyentry), "/dev/ftp%02lu",
    (unsigned long) getpid());
  pam_set_item(pamh, PAM_TTY, ttyentry);
#endif /* SOLARIS2 */

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

    pr_log_pri(PR_LOG_NOTICE, "PAM(%s): %s.", cmd->argv[0],
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
        retval = PR_AUTH_AGEPWD;
        break;
#endif /* PAM_AUTHTOKEN_REQD */

      case PAM_ACCT_EXPIRED:
#ifdef PAM_ACCT_DISABLED
      case PAM_ACCT_DISABLED:
#endif /* PAM_ACCT_DISABLED */
        retval = PR_AUTH_DISABLEDPWD;
        break;

      case PAM_USER_UNKNOWN:
        retval = PR_AUTH_NOPWD;
        break;

      default:
        retval = PR_AUTH_BADPWD;
        break;
    }

    pr_log_pri(PR_LOG_NOTICE, "PAM(%s): %s.", cmd->argv[0],
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

    pr_log_pri(PR_LOG_NOTICE, "PAM(%s): %s.", cmd->argv[0],
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
        retval = PR_AUTH_AGEPWD;
        break;

      case PAM_USER_UNKNOWN:
        retval = PR_AUTH_NOPWD;
        break;

      default:
        retval = PR_AUTH_BADPWD;
        break;
    }

    pr_log_pri(PR_LOG_NOTICE, "PAM(%s): %s.", cmd->argv[0],
      pam_strerror(pamh, pam_error));
    goto done;
  }

  success++;

 done:

  /* And we're done.  Clean up and relinquish our root privs.
   */

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

    return pam_authoritative ? ERROR_INT(cmd, retval) : DECLINED(cmd);

  } else {
    session.auth_mech = "mod_auth_pam.c";
    pr_event_register(&auth_pam_module, "core.exit", auth_pam_exit_ev, NULL);
    return HANDLED(cmd);
  }
}

/* Configuration handlers
 */

MODRET set_authpam(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((bool = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return HANDLED(cmd);
}

MODRET set_authpamconfig(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);

  return HANDLED(cmd);
}

static authtable auth_pam_authtab[] = {
  { 0, "auth", pam_auth },
  { 0, NULL, NULL}
};

static conftable auth_pam_conftab[] = {
  { "AuthPAM",			set_authpam,			NULL },
  { "AuthPAMConfig",		set_authpamconfig,		NULL },
  { NULL, NULL, NULL}
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
  NULL
};

#endif /* HAVE_PAM */
