/*
 * ProFTPD: mod_lang -- a module for handling the LANG command [RFC2640]
 *
 * Copyright (c) 2006-2007 The ProFTPD Project
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
 * $Id: mod_lang.c,v 1.5 2007/01/19 23:01:25 castaglia Exp $
 */

#include "conf.h"

#define MOD_LANG_VERSION		"mod_lang/0.8"

#if PROFTPD_VERSION_NUMBER < 0x0001030101
# error "ProFTPD 1.3.1rc1 or later required"
#endif

#if PR_USE_NLS

module lang_module;

static const char *lang_default = "en";
static int lang_engine = TRUE;
static pool *lang_pool = NULL;
static pr_table_t *lang_tab = NULL;

/* Support routines
 */

static int lang_supported(const char *lang) {
  if (strcmp(lang, "en") != 0)
    return -1;

  return 0;
}

/* Configuration handlers
 */

/* usage: LangDefault lang */
MODRET set_langdefault(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  (void) add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: LangEngine on|off */
MODRET set_langengine(cmd_rec *cmd) {
  int bool;
  config_rec *c;

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

/* usage: LangPath path */
MODRET set_langpath(cmd_rec *cmd) {
  CHECK_CONF(cmd, CONF_ROOT);

  return PR_HANDLED(cmd);
}

/* Command handlers
 */

MODRET lang_lang(cmd_rec *cmd) {
  unsigned char *authenticated;

  if (!lang_engine)
    return PR_DECLINED(cmd);

  if (!dir_check(cmd->tmp_pool, cmd->argv[0], cmd->group, session.cwd, NULL)) {
    pr_log_debug(DEBUG4, MOD_LANG_VERSION ": LANG command denied by <Limit>");
    pr_response_add_err(R_500, _("Unable to handle command"));
    return PR_ERROR(cmd);
  }

  /* If the user has already authenticated (and thus possibly chrooted),
   * deny the command.  Once chrooted, we will not have access to the
   * message catalog files anymore.
   *
   * True, the user may not have been chrooted, but if we allow non-chrooted
   * users to issue LANG commands while chrooted users cannot, it can
   * constitute an information leak.  Best to avoid that altogether.
   */
   authenticated = get_param_ptr(cmd->server->conf, "authenticated", FALSE);
   if (authenticated &&
       *authenticated == TRUE) {
     pr_response_add_err(R_500, _("Unable to handle command"));
     return PR_ERROR(cmd);
   }

  if (cmd->argc > 2) {
    pr_response_add_err(R_501, _("Invalid number of arguments"));
    return PR_ERROR(cmd);
  }

  if (cmd->argc == 1) {
    pr_log_debug(DEBUG7, MOD_LANG_VERSION
      ": resetting to default language '%s'", lang_default);

    /* XXX Reset stuff here */

    pr_response_add(R_200, _("Using default language %s"), lang_default);
    return PR_HANDLED(cmd);
  }

  if (lang_supported(cmd->argv[1]) < 0) {
    pr_response_add_err(R_504, _("Language %s not supported"), cmd->argv[1]);
    return PR_ERROR(cmd);
  }

  /* If successful, remove the previous FEAT line for LANG, and update it
   * with a new one showing the currently selected language.
   */

  /* XXX As currently implemented, pr_feat_remove() allows for a memory
   * leak in the feat pool.  This means that a malicious client could
   * send LANG repeatedly, and cause proftpd memory usage to grow
   * (albeit very slowly).  Perhaps the LANG command should only be
   * accepted N number of times?
   */

  pr_response_add(R_200, _("Using language %s"), cmd->argv[1]);
  return PR_HANDLED(cmd);
}

MODRET lang_utf8(cmd_rec *cmd) {
  register unsigned int i;
  int bool, prev;
  char *method = pstrdup(cmd->tmp_pool, cmd->argv[0]);

  /* Convert underscores to spaces in the method name, for prettier
   * logging.
   */
  for (i = 0; method[i]; i++) {
    if (method[i] == '_')
      method[i] = ' ';
  }

  if (cmd->argc != 2) {
    pr_response_add_err(R_501, _("'%s' not understood"), method);
    return PR_ERROR(cmd);
  }

  bool = get_boolean(cmd, 1);
  if (bool < 0) {
    pr_response_add_err(R_501, _("'%s' not understood"), method);
    return PR_ERROR(cmd);
  }
 
  prev = pr_fs_use_utf8(bool);
  if (bool == TRUE &&
      prev == FALSE) {
    /* If we are being asked to enable UTF8, and the previous setting was to
     * NOT use UTF8, it usually means that the sysadmin set "UseUTF8 off" in
     * the proftpd.conf.  Thus we revert back to the non-UTF8 case, and
     * report an error back to the client. 
     */
    pr_fs_use_utf8(prev);
    pr_log_debug(DEBUG5, "unable to accept 'OPTS UTF8 on' due to UseUTF8 "
      "directive in config file");
    pr_response_add_err(R_451, _("Unable to accept %s"), method); 
    return PR_ERROR(cmd);
  }

  pr_response_add(R_200, _("UTF8 set to %s"), bool ? "on" : "off");
  return PR_HANDLED(cmd);
}

/* Event handlers
 */

static void lang_postparse_ev(const void *event_data, void *user_data) {
  config_rec *c;

  /* Scan the LangPath for the .mo files to read in. */
  const char *lang_path = PR_LOCALE_DIR;
#ifdef HAVE_LIBINTL_H
  const char *locale_path = NULL;
#endif

  c = find_config(main_server->conf, CONF_PARAM, "LangPath", FALSE);
  if (c) {

    /* XXX How to make the configured path exported to any interested
     * callers, e.g. modules that need to call bindtextdomain() for
     * their own catalogs?
     */

    lang_path = c->argv[0];
  }

#ifdef HAVE_LIBINTL_H
  pr_log_debug(DEBUG4, MOD_LANG_VERSION
    ": binding to text domain 'proftpd' using locale path '%s'", lang_path);
  locale_path = bindtextdomain("proftpd", lang_path); 
  if (locale_path == NULL) {
    pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
      ": unable to bind to text domain 'proftpd' using locale path '%s': %s",
      lang_path, strerror(errno));
  }
#else
  pr_log_debug(DEBUG2, MOD_LANG_VERSION
    ": unable to bind to text domain 'proftpd', lacking libintl support");
#endif /* !HAVE_LIBINTL_H */

  /* Iterate through the server_rec list, checking each for a configured
   * LangDefault.  If configured, make sure that the specified lang
   * is supported.
   */

  c = find_config(main_server->conf, CONF_PARAM, "LangDefault", FALSE);
  if (c) {

    /* If the selected default language is not in LangPath,
     * default to "en".
     */
  }
}

static void lang_restart_ev(const void *event_data, void *user_data) {
  destroy_pool(lang_pool);
  lang_tab = NULL;

  lang_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(lang_pool, MOD_LANG_VERSION);
}

/* Initialization functions
 */

static int lang_init(void) {
  if (setlocale(LC_ALL, "") == NULL) {
    pr_log_pri(PR_LOG_NOTICE, "unable to set LC_ALL: %s", strerror(errno));
    return -1;
  }

  /* Preserve the POSIX/portable handling of number formatting; local
   * formatting of decimal points, for example, can cause problems with
   * numbers in SQL queries.
   */
  if (setlocale(LC_NUMERIC, "C") == NULL) {
    pr_log_pri(PR_LOG_NOTICE, "unable to set LC_NUMERIC: %s",
      strerror(errno));
  }

  lang_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(lang_pool, MOD_LANG_VERSION);

  pr_event_register(&lang_module, "core.postparse", lang_postparse_ev, NULL);
  pr_event_register(&lang_module, "core.restart", lang_restart_ev, NULL);

  return 0;
}

static int lang_sess_init(void) {
  config_rec *c;

  c = find_config(main_server->conf, CONF_PARAM, "LangEngine", FALSE);
  if (c)
    lang_engine = *((int *) c->argv[0]);

  if (!lang_engine)
    return 0;

  pr_feat_add("UTF8");

  /* Configure a proper FEAT line, for our supported languages and our
   * default language.
   */
  pr_feat_add("LANG en");

  return 0;
}

/* Module API tables
 */

static conftable lang_conftab[] = {
  { "LangDefault",	set_langdefault,	NULL },
  { "LangEngine",	set_langengine,		NULL },
  { "LangPath",		set_langpath,		NULL },
  { NULL }
};

static cmdtable lang_cmdtab[] = {
  { CMD,	C_LANG,			G_NONE,	lang_lang,	FALSE,	FALSE },
  { CMD,	C_OPTS "_UTF8",		G_NONE,	lang_utf8,	FALSE,	FALSE },
  { 0, NULL }
};

module lang_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "lang",

  /* Module configuration handler table */
  lang_conftab,

  /* Module command handler table */
  lang_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  lang_init,

  /* Session initialization function */
  lang_sess_init,

  /* Module version */
  MOD_LANG_VERSION
};

#endif /* PR_USE_NLS */
