/*
 * ProFTPD: mod_lang -- a module for handling the LANG command [RFC2640]
 *
 * Copyright (c) 2006-2009 The ProFTPD Project
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
 * $Id: mod_lang.c,v 1.23 2009/01/26 21:56:13 castaglia Exp $
 */

#include "conf.h"

#define MOD_LANG_VERSION		"mod_lang/0.9"

#if PROFTPD_VERSION_NUMBER < 0x0001030101
# error "ProFTPD 1.3.1rc1 or later required"
#endif

#if PR_USE_NLS

#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif

extern xaset_t *server_list;

module lang_module;

#define LANG_DEFAULT_LANG	"en_US"

static const char *lang_curr = LANG_DEFAULT_LANG;
static const char *lang_default = LANG_DEFAULT_LANG;
static int lang_engine = TRUE;
static pool *lang_pool = NULL;
static array_header *lang_list = NULL;

/* Support routines
 */

static void lang_feat_add(pool *p) {
  char *feat_str = "";

  if (lang_list &&
      lang_list->nelts > 0) {
    register unsigned int i;
    char **langs;
    size_t feat_strlen = 0;
 
    langs = lang_list->elts;
    for (i = 0; i < lang_list->nelts; i++) {
      char *lang_dup, *tmp;

      /* Convert all locales in the list to RFC1766 form, i.e. hyphens instead
       * of underscores.
       */
      lang_dup = pstrdup(p, langs[i]);
      tmp = strchr(lang_dup, '_');
      if (tmp) {
        *tmp = '-';
      }

      feat_str = pstrcat(p, feat_str, lang_dup, NULL);
      if (strcasecmp(lang_curr, lang_dup) == 0 ||
          strcasecmp(lang_curr, langs[i]) == 0) {
        /* This is the currently selected language; mark it with an asterisk,
         * as per RFC2640, Section 4.3.
         */
        feat_str = pstrcat(p, feat_str, "*", NULL);
      }

      feat_str = pstrcat(p, feat_str, ";", NULL);
    }
 
    feat_strlen = strlen(feat_str);

    /* Trim the trailing semicolon. */
    if (feat_str[feat_strlen-1] == ';') {
      feat_str[feat_strlen-1] = '\0';
    }

    feat_str = pstrcat(p, "LANG ", feat_str, NULL);
    pr_feat_add(feat_str);

  } else {
    feat_str = pstrcat(p, "LANG ", lang_curr, NULL);
    pr_feat_add(feat_str);
  }
}

static void lang_feat_remove(void) {
  const char *feat, *lang_feat = NULL;

  feat = pr_feat_get();
  while (feat) {
    pr_signals_handle();

    if (strncmp(feat, C_LANG, 4) == 0) {
      lang_feat = feat;
      break;
    }

    feat = pr_feat_get_next();
  }

  if (lang_feat)
    pr_feat_remove(lang_feat);
}

static int lang_set_lang(pool *p, const char *lang) {
  int category = LC_MESSAGES;
  char *curr_lang, *codeset;

  if (strlen(lang) == 0) {
    category = LC_ALL;
  }

  curr_lang = pstrdup(p, setlocale(LC_MESSAGES, NULL));

  if (setlocale(category, lang) == NULL) {
    if (errno == ENOENT) {
      /* The site may have an unknown/bad LANG environment variable set.
       * Report this, and fall back to using "C" as the locale.
       */
      pr_log_pri(PR_LOG_NOTICE,
        "unknown/unsupported language '%s' for %s, switching %s from '%s' "
        "to 'C'", lang, category == LC_MESSAGES ? "LC_MESSAGES" : "LC_ALL",
        curr_lang, category == LC_MESSAGES ? "LC_MESSAGES" : "LC_ALL");
      setlocale(category, "C");

    } else {
      pr_log_pri(PR_LOG_NOTICE, "unable to set %s to '%s': %s",
        category == LC_MESSAGES ? "LC_MESSAGES" : "LC_ALL", lang,
        strerror(errno));
      return -1;
    }

  } else {
    curr_lang = setlocale(LC_MESSAGES, NULL);
    pr_log_debug(DEBUG4, MOD_LANG_VERSION ": using %s messages",
      *lang ? lang : curr_lang);
  }

  /* Preserve the POSIX/portable handling of number formatting; local
   * formatting of decimal points, for example, can cause problems with
   * numbers in SQL queries.
   */
  if (setlocale(LC_NUMERIC, "C") == NULL) {
    pr_log_pri(PR_LOG_NOTICE, "unable to set LC_NUMERIC: %s",
      strerror(errno));
  }

  /* Change the encoding used in the Encode API.  Unfortunately this means
   * duplicating some of the logic from the Encode API here.
   */
#ifdef HAVE_NL_LANGINFO
  codeset = nl_langinfo(CODESET);
  if (codeset == NULL ||
      strlen(codeset) == 0) {
    codeset = "UTF-8";
 
  } else { 
 
    /* Workaround a stupid bug in many implementations where nl_langinfo()
     * returns "646" to mean "US-ASCII".  The problem is that iconv_open(3)
     * doesn't accept "646" as an acceptable encoding.
     */
    if (strcmp(codeset, "646") == 0) {
      codeset = "US-ASCII";
    }
  }
#else
  codeset = "UTF-8";
#endif /* !HAVE_NL_LANGINFO */

  if (category == LC_ALL) {
    if (pr_encode_enable_encoding(codeset) < 0)
      return -1;
  }

  return 0;
}

/* Supports comparison of RFC1766 language tags (case-insensitive, using
 * hyphens) from the client with the underscore-using locale names usually
 * used by iconv and setlocale().
 */
static int lang_supported(pool *p, const char *lang) {
  register unsigned int i;
  size_t lang_len;
  char *lang_dup, **langs;
  int ok = FALSE;

  if (!lang_list) {
    errno = EPERM;
    return -1;
  }

  lang_dup = pstrdup(p, lang);

  lang_len = strlen(lang_dup);
  if (lang_len > 4) {

    /* Transform something like "en-US" into "en_US". */
    if (lang_dup[2] == '-') {
      lang_dup[2] = '_';
    }
  }

  langs = lang_list->elts;

  for (i = 0; i < lang_list->nelts; i++) {
    if (strcasecmp(langs[i], lang_dup) == 0) {
      ok = TRUE;
      break;
    }
  }

  if (!ok) {
    errno = ENOENT;
    return -1;
  }

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
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (pr_fs_valid_path(cmd->argv[1]) < 0) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unable to use path '",
      cmd->argv[1], "' for locale files", NULL));
  }

  (void) add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: UseEncoding on|off|local-charset client-charset */
MODRET set_useencoding(cmd_rec *cmd) {
  config_rec *c;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (cmd->argc == 2) {
    int bool = -1;

    bool = get_boolean(cmd, 1);
    if (bool == -1) {
      CONF_ERROR(cmd, "expected Boolean parameter");
    }

    c = add_config_param(cmd->argv[0], 1, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(int));
    *((int *) c->argv[0]) = bool;

  } else if (cmd->argc == 3) {
    c = add_config_param_str(cmd->argv[0], 2, cmd->argv[1], cmd->argv[2]);

  } else {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

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

    if (lang_set_lang(cmd->tmp_pool, lang_default) < 0) {
      pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
        ": unable to use LangDefault '%s': %s", lang_default, strerror(errno));
      pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
        ": using LC_ALL environment variable value instead");

      if (lang_set_lang(cmd->tmp_pool, "") < 0) {
        pr_log_pri(PR_LOG_WARNING, MOD_LANG_VERSION
          ": unable to use LC_ALL value for locale: %s", strerror(errno));
        end_login(1);
      }
    }

    pr_response_add(R_200, _("Using default language %s"), lang_default);
    return PR_HANDLED(cmd);
  }

  if (lang_supported(cmd->tmp_pool, cmd->argv[1]) < 0) {
    pr_log_debug(DEBUG3, MOD_LANG_VERSION ": language '%s' unsupported: %s",
      cmd->argv[1], strerror(errno));
    pr_response_add_err(R_504, _("Language %s not supported"), cmd->argv[1]);
    return PR_ERROR(cmd);
  }

  pr_log_debug(DEBUG7, MOD_LANG_VERSION
    ": setting to client-requested language '%s'", cmd->argv[1]);

  if (lang_set_lang(cmd->tmp_pool, cmd->argv[1]) < 0) {
    pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
      ": unable to use client-requested language '%s': %s", cmd->argv[1],
      strerror(errno));
    pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
      ": using LangDefault '%s' instead", lang_default);

    if (lang_set_lang(cmd->tmp_pool, lang_default) < 0) {
      pr_log_pri(PR_LOG_WARNING, MOD_LANG_VERSION
        ": unable to use LangDefault '%s': %s", lang_default, strerror(errno));
      end_login(1);
    }
  }

  lang_curr = pstrdup(lang_pool, cmd->argv[1]);

  pr_log_debug(DEBUG5, MOD_LANG_VERSION
    ": now using client-requested language '%s'", lang_curr);

  /* If successful, remove the previous FEAT line for LANG, and update it
   * with a new one showing the currently selected language.
   */

  lang_feat_remove();
  lang_feat_add(cmd->tmp_pool);

  pr_response_add(R_200, _("Using language %s"), lang_curr);
  return PR_HANDLED(cmd);
}

MODRET lang_utf8(cmd_rec *cmd) {
  register unsigned int i;
  int bool;
  const char *curr_encoding;
  char *method;

  method = pstrdup(cmd->tmp_pool, cmd->argv[0]);

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

  curr_encoding = pr_encode_get_encoding();

  if (pr_encode_is_utf8(curr_encoding) == TRUE) {
    if (bool) {
      /* Client requested that we use UTF8, and we already are.  Nothing
       * more needs to be done.
       */
      pr_response_add(R_200, _("UTF8 set to on"));

    } else {
      config_rec *c;

      /* Client requested that we NOT use UTF8, and we are.  Need to disable
       * encoding, then, unless the UseEncoding setting dictates that we
       * must.
       */

      c = find_config(main_server->conf, CONF_PARAM, "UseEncoding", FALSE);
      if (c) {
        /* We have explicit UseEncoding instructions; we cannot change
         * the encoding use as requested by the client.
         */
        pr_log_debug(DEBUG5, MOD_LANG_VERSION
          ": unable to accept 'OPTS UTF8 off' due to UseEncoding directive in "
          "config file");
        pr_response_add_err(R_451, _("Unable to accept %s"), method);
        return PR_ERROR(cmd);

      } else {
        pr_log_debug(DEBUG5, MOD_LANG_VERSION
          ": disabling use of UTF8 encoding as per client's request");

        /* No explicit UseEncoding instructions; we can turn off encoding. */
        pr_encode_disable_encoding();
        pr_fs_use_encoding(FALSE);

        pr_response_add(R_200, _("UTF8 set to off"));
      }
    }

  } else {

    if (bool) {
      config_rec *c;

      /* Client requested that we use UTF8, and we currently are not.
       * Enable UTF8 encoding, unless the UseEncoding setting dictates that
       * we cannot.
       */

      c = find_config(main_server->conf, CONF_PARAM, "UseEncoding", FALSE);
      if (c) {
        /* We have explicit UseEncoding instructions. */
        pr_log_debug(DEBUG5, MOD_LANG_VERSION
          ": unable to accept 'OPTS UTF8 on' due to UseEncoding directive in "
          "config file");
        pr_response_add_err(R_451, _("Unable to accept %s"), method);
        return PR_ERROR(cmd);

      } else {
        pr_log_debug(DEBUG5, MOD_LANG_VERSION
          ": enabling use of UTF8 encoding as per client's request");

        /* No explicit UseEncoding instructions; we can turn on encoding. */
        if (pr_encode_enable_encoding("UTF-8") < 0) {
          pr_log_debug(DEBUG3, MOD_LANG_VERSION
            ": error enabling UTF8 encoding: %s", strerror(errno));
          pr_response_add_err(R_451, _("Unable to accept %s"), method);
          return PR_ERROR(cmd);

        } else {
          pr_fs_use_encoding(TRUE);
          pr_response_add(R_200, _("UTF8 set to on"));
        }
      }

    } else {
      /* Client requested that we not use UTF8, and we are not.  Nothing more
       * needs to be done.
       */
      pr_response_add(R_200, _("UTF8 set to off"));
    }
  }

  return PR_HANDLED(cmd);
}

/* Event handlers
 */

static void lang_postparse_ev(const void *event_data, void *user_data) {
  pool *tmp_pool;
  config_rec *c;
  const char *lang_path = PR_LOCALE_DIR;
  DIR *dirh;
  server_rec *s;
#ifdef HAVE_LIBINTL_H
  const char *locale_path = NULL;
#endif

  c = find_config(main_server->conf, CONF_PARAM, "LangEngine", FALSE);
  if (c) {
    int engine;

    engine = *((int *) c->argv[0]);
    if (!engine)
      return;
  }

  /* ANSI C says that every process starts off in the 'C' locale, regardless
   * of any environment variable settings (e.g. LANG).  Thus to honor the
   * LANG et al environment variables, we need to explicitly call
   * setlocale(3) with the empty string.
   */
  if (lang_set_lang(lang_pool, "") < 0) {
    pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
      ": error setting locale based on LANG and other environment "
      "variables: %s", strerror(errno));
  }

  /* Scan the LangPath for the .mo files to read in. */

  c = find_config(main_server->conf, CONF_PARAM, "LangPath", FALSE);
  if (c) {
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
    return;

  } else {
    pr_log_debug(DEBUG4, MOD_LANG_VERSION ": using locale files in '%s'",
      locale_path);

    /* By default, use UTF8 for the charset for gettext() messages. */
    if (bind_textdomain_codeset("proftpd", "UTF8") == NULL) {
      pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
        ": error setting client charset 'UTF8' for localised messages: %s",
        strerror(errno));
    }
  }

#else
  pr_log_debug(DEBUG2, MOD_LANG_VERSION
    ": unable to bind to text domain 'proftpd', lacking libintl support");
#endif /* !HAVE_LIBINTL_H */

  /* Scan locale_path using readdir(), to get the list of available
   * translations/langs.  Make sure to check for presence of 'proftpd.mo'
   * in the directories:
   *
   *  $lang/LC_MESSAGES/proftpd.mo
   *
   * In addition, make sure the directory name is a locale acceptable to
   * setlocale(3).
   */

  tmp_pool = make_sub_pool(lang_pool);

  dirh = opendir(locale_path);
  if (dirh != NULL) {
    register unsigned int i;
    struct dirent *dent;
    char *langs_str = "", **langs = NULL;

    if (!lang_list) {
      lang_list = make_array(lang_pool, 3, sizeof(char *));
    }

    while ((dent = readdir(dirh)) != NULL) {
      char *mo;
      struct stat st;

      pr_signals_handle();

      mo = pdircat(tmp_pool, locale_path, dent->d_name, "LC_MESSAGES",
        "proftpd.mo", NULL);

      if (stat(mo, &st) == 0) {
        char *curr_locale;

        /* Check that dent->d_name is a valid language name according to
         * setlocale(3) before adding it to the list.
         */
        curr_locale = pstrdup(tmp_pool, setlocale(LC_MESSAGES, NULL));

        if (setlocale(LC_MESSAGES, dent->d_name) != NULL) {
          *((char **) push_array(lang_list)) = pstrdup(lang_pool, dent->d_name);

        } else {
          if (errno == ENOENT) {
            pr_log_debug(DEBUG5, MOD_LANG_VERSION
              ": skipping possible language '%s': not supported by "
              "setlocale(3); see `locale -a'", dent->d_name);

          } else {
            pr_log_debug(DEBUG5, MOD_LANG_VERSION
              ": skipping possible language '%s': %s", dent->d_name,
              strerror(errno));
          }
        }

        /* Restore the current locale. */
        setlocale(LC_MESSAGES, curr_locale);
      }
    }

    closedir(dirh);

    langs = lang_list->elts;
    for (i = 0; i < lang_list->nelts; i++) {
      langs_str = pstrcat(tmp_pool, langs_str, *langs_str ? ", " : "",
        langs[i], NULL);
    }

    if (lang_list->nelts > 0) {
      pr_log_debug(DEBUG8, MOD_LANG_VERSION
      ": added the following supported languages: %s", langs_str);
    }

  } else {
    pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
      ": unable to scan the localised files in '%s': %s", locale_path,
      strerror(errno));
  }

  /* Iterate through the server list, checking each for a configured
   * LangDefault.  If configured, make sure that the specified language is
   * supported.
   */

  for (s = (server_rec *) server_list->xas_list; s; s = s->next) {
    c = find_config(s->conf, CONF_PARAM, "LangDefault", FALSE);
    if (c) {
      char *lang = c->argv[0];

      if (lang_supported(tmp_pool, lang) < 0) {
        pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
          ": LangDefault '%s', configured for server '%s', is not a supported "
          "language, removing", lang, s->ServerName);
        remove_config(s->conf, "LangDefault", FALSE);
      }
    }
  }

  if (tmp_pool)
    destroy_pool(tmp_pool);
}

static void lang_restart_ev(const void *event_data, void *user_data) {
  destroy_pool(lang_pool);
  lang_list = NULL;

  lang_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(lang_pool, MOD_LANG_VERSION);
}

/* Initialization functions
 */

static int lang_init(void) {
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

  /* ANSI C says that every process starts off in the 'C' locale, regardless
   * of any environment variable settings (e.g. LANG).  Thus to honor the
   * LANG et al environment variables, we need to explicitly call
   * setlocale(3) appropriately.
   */
  if (lang_set_lang(lang_pool, "") < 0) {
    pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
      ": error setting locale based on LANG and other environment "
      "variables: %s", strerror(errno));
  }

  c = find_config(main_server->conf, CONF_PARAM, "LangDefault", FALSE);
  if (c) {
    char *lang;

    lang = c->argv[0];

    if (lang_set_lang(lang_pool, lang) < 0) {
      pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
        ": unable to use LangDefault '%s': %s", lang, strerror(errno));
      pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
        ": using LC_ALL environment variable value instead");

      if (lang_set_lang(lang_pool, "") < 0) {
        pr_log_pri(PR_LOG_WARNING, MOD_LANG_VERSION
          ": unable to use LC_ALL value for locale: %s", strerror(errno));

        errno = EPERM;
        return -1;
      }
    }

    lang_curr = lang_default = lang;

  } else {
    /* No explicit default language configured; rely on the environment
     * variables.
     */
    if (lang_set_lang(lang_pool, "") < 0) {
      pr_log_pri(PR_LOG_WARNING, MOD_LANG_VERSION
        ": unable to use LC_ALL value for locale: %s", strerror(errno));

      errno = EPERM;
      return -1;
    }

    lang_curr = pstrdup(lang_pool, setlocale(LC_MESSAGES, NULL));
    if (strcasecmp(lang_curr, "C") == 0) {
      lang_curr = LANG_DEFAULT_LANG;
    }

    lang_default = lang_curr;

    /* If a list of languages is empty (perhaps because the message catalogs
     * could not be found for some reason), we should still have an entry for
     * the current language.
     */
    if (!lang_list) {
      lang_list = make_array(lang_pool, 1, sizeof(char *));
    }

    if (lang_list->nelts == 0) {
      *((char **) push_array(lang_list)) = pstrdup(lang_pool, lang_curr);
    } 
  }

  c = find_config(main_server->conf, CONF_PARAM, "UseEncoding", FALSE);
  if (c) {
    if (c->argc == 1) {
      int bool;

      bool = *((int *) c->argv[0]);
      if (bool) {
        pr_feat_add("UTF8");
        pr_fs_use_encoding(TRUE);

      } else {
        pr_encode_disable_encoding();
        pr_fs_use_encoding(FALSE);
      }

    } else {
      char *local_charset, *client_charset;

      local_charset = c->argv[0];
      client_charset = c->argv[1];

      if (pr_encode_set_charset_encoding(local_charset, client_charset) < 0) {
        pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
          ": error setting local charset '%s', client charset '%s': %s",
          local_charset, client_charset, strerror(errno));
        pr_fs_use_encoding(FALSE);

      } else {
        pr_log_debug(DEBUG3, MOD_LANG_VERSION ": using local charset '%s', "
          "client charset '%s' for path encoding", local_charset,
          client_charset);
        pr_fs_use_encoding(TRUE);

        /* Make sure that gettext() uses the specified charset as well. */
        if (bind_textdomain_codeset("proftpd", client_charset) == NULL) {
          pr_log_pri(PR_LOG_NOTICE, MOD_LANG_VERSION
            ": error setting client charset '%s' for localised messages: %s",
            client_charset, strerror(errno));
        }

        /* If the client charset specified happens to be UTF8, we need to
         * make sure it shows up in FEAT.
         */
        if (strcasecmp(client_charset, "UTF8") == 0 ||
            strcasecmp(client_charset, "UTF-8") == 0) {
          pr_feat_add("UTF8");
        }
      }
    }

  } else {
    /* Default is to use UTF8. */
    pr_feat_add("UTF8");
    pr_fs_use_encoding(TRUE);
  }

  /* Configure a proper FEAT line, for our supported languages and our
   * default language.
   */
  lang_feat_add(main_server->pool);

  return 0;
}

/* Module API tables
 */

static conftable lang_conftab[] = {
  { "LangDefault",	set_langdefault,	NULL },
  { "LangEngine",	set_langengine,		NULL },
  { "LangPath",		set_langpath,		NULL },
  { "UseEncoding",	set_useencoding,	NULL },
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
