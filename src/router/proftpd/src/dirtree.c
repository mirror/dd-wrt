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

/* Read configuration file(s), and manage server/configuration structures.
 * $Id: dirtree.c,v 1.232 2010/02/23 18:01:03 castaglia Exp $
 */

#include "conf.h"

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_REGEX_H
# include <regex.h>
#endif

xaset_t *server_list = NULL;
server_rec *main_server = NULL;
int tcpBackLog = PR_TUNABLE_DEFAULT_BACKLOG;
int SocketBindTight = FALSE;
char ServerType = SERVER_STANDALONE;
int ServerMaxInstances = 0;
int ServerUseReverseDNS = TRUE;
char MultilineRFC2228 = 0;

/* Default TCP send/receive buffer sizes. */
static int tcp_rcvbufsz = 0;
static int tcp_sndbufsz = 0;
static int xfer_bufsz = 0;

/* From src/pool.c */
extern pool *global_config_pool;

/* Used by find_config_* */
static xaset_t *find_config_top = NULL;

static void config_dumpf(const char *, ...);
static void merge_down(xaset_t *, int);

static config_rec *_last_param_ptr = NULL;
static unsigned char _kludge_disable_umask = 0;

/* We have two different lists for Defines.  The 'perm' pool/list are
 * for "permanent" defines, i.e. those set on the command-line via the
 * -D/--define options.
 */
static pool *defines_pool = NULL;
static array_header *defines_list = NULL;

static pool *defines_perm_pool = NULL;
static array_header *defines_perm_list = NULL;

static pool *config_tab_pool = NULL;
static pr_table_t *config_tab = NULL;
static unsigned int config_id = 0;

static int allow_dyn_config(const char *path) {
  config_rec *c = NULL;
  unsigned int ctxt_precedence = 0;
  unsigned char allow = TRUE, found_config = FALSE;

  c = find_config(CURRENT_CONF, CONF_PARAM, "AllowOverride", FALSE);
  while (c) {
    pr_signals_handle();

    if (*((unsigned int *) c->argv[1]) > ctxt_precedence) {

      /* Set the context precedence. */
      ctxt_precedence = *((unsigned int *) c->argv[1]);

      allow = *((int *) c->argv[0]);

      found_config = TRUE;
    }

    c = find_config_next(c, c->next, CONF_PARAM, "AllowOverride", FALSE);
  }

  /* Print out some nice debugging information. */
  if (found_config) {
    pr_log_debug(DEBUG8, "AllowOverride for path '%s' %s .ftpaccess files",
      path, allow ? "allows" : "denies");
  }

  return allow;
}

/* Imported this function from modules/mod_ls.c -- it belongs more with the
 * dir_* functions here, rather than the ls_* functions there.
 */

/* Return true if dir is ".", "./", "../", or "..". */
int is_dotdir(const char *dir) {
  if (strcmp(dir, ".") == 0 || strcmp(dir, "./") == 0 ||
      strcmp(dir, "..") == 0 || strcmp(dir, "../") == 0)
    return TRUE;

  return FALSE;
}

/* Return true if str contains any of the glob(7) characters. */
int is_fnmatch(const char *str) {
  int have_bracket = 0;

  while (*str) {
    switch (*str) {
      case '?':
      case '*':
        return TRUE;

      case '\\':
        if (*str++ == '\0')
          return FALSE;
        break;

      case '[':
        have_bracket++;
        break;

      case ']':
        if (have_bracket)
          return TRUE;
        break;

      default:
        break;
    }

    str++;
  }

  return FALSE;
}

/* Lookup the best configuration set from which to retrieve configuration
 * values if the config_rec can appear in <Directory>.  This function
 * works around the issue caused by using the cached directory pointer
 * in session.dir_config.
 *
 * The issue with using session.dir_config is that it is assigned when
 * the client changes directories or doing other directory lookups, and so
 * dir_config may actually point to the configuration for a directory other
 * than the target directory for an uploaded, for example.  Unfortunately,
 * it is more expensive to lookup the configuration for the target directory
 * every time.  Perhaps some caching of looked up directory configurations
 * into a table, rather than a single pointer like session.dir_config,
 * might help.
 */
xaset_t *get_dir_ctxt(pool *p, char *dir_path) {
  config_rec *c = NULL;
  char *full_path = dir_path;

  if (session.chroot_path) {
    if (*dir_path != '/')
      full_path = pdircat(p, session.chroot_path, session.cwd, dir_path, NULL);

    else
      full_path = pdircat(p, session.chroot_path, dir_path, NULL);

  } else if (*dir_path != '/')
    full_path = pdircat(p, session.cwd, dir_path, NULL);

  c = dir_match_path(p, full_path);

  return c ? c->subset : session.anon_config ? session.anon_config->subset :
    main_server->conf;
}

/* Substitute any appearance of the %u variable in the given string with
 * the value.
 */
char *path_subst_uservar(pool *path_pool, char **path) {
  char *new_path = NULL, *substr = NULL, *substr_path = NULL;

  /* Sanity check. */
  if (path_pool == NULL ||
      path == NULL ||
      !*path) {
    errno = EINVAL;
    return NULL;
  }

  /* If no %u string present, do nothing. */
  if (strstr(*path, "%u") == NULL)
    return *path;

  /* First, deal with occurrences of "%u[index]" strings.  Note that
   * with this syntax, the '[' and ']' characters become invalid in paths,
   * but only if that '[' appears after a "%u" string -- certainly not
   * a common phenomenon (I hope).  This means that in the future, an escape
   * mechanism may be needed in this function.  Caveat emptor.
   */

  substr_path = *path;
  substr = substr_path ? strstr(substr_path, "%u[") : NULL;
  while (substr) {
    int i = 0;
    char *substr_end = NULL, *substr_dup = NULL, *endp = NULL;
    char ref_char[2] = {'\0', '\0'};

    pr_signals_handle();

    /* Now, find the closing ']'. If not found, it is a syntax error;
     * continue on without processing this occurrence.
     */
    substr_end = strchr(substr, ']');
    if (substr_end == NULL) {
      /* Just end here. */
      break;
    }

    /* Make a copy of the entire substring. */
    substr_dup = pstrdup(path_pool, substr);

    /* The substr_end variable (used as an index) should work here, too
     * (trying to obtain the entire substring).
     */
    substr_dup[substr_end - substr + 1] = '\0';

    /* Advance the substring pointer by three characters, so that it is
     * pointing at the character after the '['.
     */
    substr += 3;

    /* If the closing ']' is the next character after the opening '[', it
     * is a syntax error.
     */
    if (substr_end == substr) {

      /* Do not forget to advance the substring search path pointer. */
      substr_path = substr;

      continue;
    }

    /* Temporarily set the ']' to '\0', to make it easy for the string
     * scanning below.
     */
    *substr_end = '\0';

    /* Scan the index string into a number, watching for bad strings. */
    i = strtol(substr, &endp, 10);

    if (endp && *endp) {
      *substr_end = ']';
      substr_path = substr;
      continue;
    }

    /* Make sure that index is within bounds. */
    if (i < 0 || i > strlen(session.user) - 1) {

      /* Put the closing ']' back. */
      *substr_end = ']';

      /* Syntax error. Advance the substring search path pointer, and move
       * on.
       */
      substr_path = substr;

      continue;
    }

    ref_char[0] = session.user[i];

    /* Put the closing ']' back. */
    *substr_end = ']';

    /* Now, to substitute the whole "%u[index]" substring with the
     * referenced character/string.
     */
    substr_path = sreplace(path_pool, substr_path, substr_dup, ref_char, NULL);
    substr = substr_path ? strstr(substr_path, "%u[") : NULL;
  }

  /* Check for any bare "%u", and handle those if present. */
  if (substr_path &&
      strstr(substr_path, "%u") != NULL) {
    new_path = sreplace(path_pool, substr_path, "%u", session.user, NULL);

  } else {
    new_path = substr_path;
  }

  return new_path;
}

/* Check for configured HideFiles directives, and check the given path (full
 * _path_, not just filename) against those regexes if configured.
 *
 * Returns FALSE if the path should be shown/listed, TRUE if it should not
 * be visible.
 */
unsigned char dir_hide_file(const char *path) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  char *file_name = NULL, *dir_name = NULL;
  config_rec *c = NULL;
  regex_t *regexp = NULL;
  pool *tmp_pool;
  unsigned int ctxt_precedence = 0;
  unsigned char have_user_regex, have_group_regex, have_class_regex,
    have_all_regex, negated = FALSE;

  if (path == NULL) {
    return FALSE;
  }

  tmp_pool = make_sub_pool(session.pool);
  pr_pool_tag(tmp_pool, "dir_hide_file() tmp pool");

  have_user_regex = have_group_regex = have_class_regex = have_all_regex =
    FALSE;

  /* Separate the given path into directory and file components. */
  dir_name = pstrdup(tmp_pool, path);

  file_name = strrchr(dir_name, '/');
  if (file_name != NULL) {

    /* Make sure handle paths like "/path". */
    if (file_name != dir_name) {
      *file_name = '\0';

    } else {
      dir_name = "/";
    }

    file_name++;

  } else {
    file_name = dir_name;
  }

  /* Check for any configured HideFiles */
  c = find_config(get_dir_ctxt(tmp_pool, dir_name), CONF_PARAM, "HideFiles",
    FALSE);

  while (c) {
    if (c->argc >= 4) {

      /* check for a specified "user" classifier first... */
      if (strcmp(c->argv[3], "user") == 0) {
        if (pr_expr_eval_user_or((char **) &c->argv[4]) == TRUE) {

          if (*((unsigned int *) c->argv[2]) > ctxt_precedence) {
            ctxt_precedence = *((unsigned int *) c->argv[2]);

            regexp = *((regex_t **) c->argv[0]);
            negated = *((unsigned char *) c->argv[1]);

            have_group_regex = have_class_regex = have_all_regex = FALSE;
            have_user_regex = TRUE;
          }
        }

      /* ...then for a "group" classifier... */
      } else if (strcmp(c->argv[3], "group") == 0) {
        if (pr_expr_eval_group_and((char **) &c->argv[4]) == TRUE) {
          if (*((unsigned int *) c->argv[2]) > ctxt_precedence) {
            ctxt_precedence = *((unsigned int *) c->argv[2]);

            regexp = *((regex_t **) c->argv[0]);
            negated = *((unsigned char *) c->argv[1]);

            have_user_regex = have_class_regex = have_all_regex = FALSE;
            have_group_regex = TRUE;
          }
        }

      /* ...finally, for a "class" classifier.  NOTE: mod_time's
       * class_expression functionality should really be added into the
       * core code at some point.  When that happens, then this code will
       * need to be updated to process class-expressions.
       */
      } else if (strcmp(c->argv[3], "class") == 0) {
        if (pr_expr_eval_class_or((char **) &c->argv[4]) == TRUE) {
          if (*((unsigned int *) c->argv[2]) > ctxt_precedence) {
            ctxt_precedence = *((unsigned int *) c->argv[2]);

            regexp = *((regex_t **) c->argv[0]);
            negated = *((unsigned char *) c->argv[1]);

            have_user_regex = have_group_regex = have_all_regex = FALSE;
            have_class_regex = TRUE;
          }
        }
      }

    } else if (c->argc == 1) {

      /* This is the "none" HideFiles parameter. */
      destroy_pool(tmp_pool);
      return FALSE;

    } else {
      if (*((unsigned int *) c->argv[2]) > ctxt_precedence) {
        ctxt_precedence = *((unsigned int *) c->argv[2]);

        regexp = *((regex_t **) c->argv[0]);
        negated = *((unsigned char *) c->argv[1]);

        have_user_regex = have_group_regex = have_class_regex = FALSE;
        have_all_regex = TRUE;
      }
    }

    c = find_config_next(c, c->next, CONF_PARAM, "HideFiles", FALSE);
  }

  if (have_user_regex || have_group_regex ||
      have_class_regex || have_all_regex) {

    pr_log_debug(DEBUG4, "checking %sHideFiles pattern for current %s",
      negated ? "negated " : "",
      have_user_regex ? "user" : have_group_regex ? "group" :
      have_class_regex ? "class" : "session");

    if (regexp == NULL) {
      destroy_pool(tmp_pool);

      /* HideFiles none for this user/group/class */

      pr_log_debug(DEBUG9, "file '%s' did not match HideFiles pattern 'none'",
        file_name);
      return FALSE;
    }

    if (regexec(regexp, file_name, 0, NULL, 0) != 0) {
      destroy_pool(tmp_pool);

      pr_log_debug(DEBUG9, "file '%s' did not match %sHideFiles pattern",
        file_name, negated ? "negated " : "");

      /* The file failed to match the HideFiles regex, which means it should
       * be treated as a "visible" file.  If the regex was negated, though,
       * switch the result.
       */
      return (negated ? TRUE : FALSE);

    } else {
      destroy_pool(tmp_pool);

      pr_log_debug(DEBUG9, "file '%s' matched %sHideFiles pattern", file_name,
        negated ? "negated " : "");

      /* The file matched the HideFiles regex, which means it should be
       * considered a "hidden" file.  If the regex was negated, though,
       * switch the result.
       */
      return (negated ? FALSE : TRUE);
    }
  }

  destroy_pool(tmp_pool);
#endif /* !HAVE_REGEX_H and !HAVE_REGCOMP */

  /* Return FALSE by default. */
  return FALSE;	
}

static void define_restart_ev(const void *event_data, void *user_data) {
  if (defines_pool) {
    destroy_pool(defines_pool);
    defines_pool = NULL;
    defines_list = NULL;
  }

  pr_event_unregister(NULL, "core.restart", define_restart_ev);
}

/* The 'survive_restarts' boolean indicates whether this Define is to be
 * permanent for the lifetime of the daemon (i.e. survives across restarts)
 * or whether it should be cleared when restarted.
 *
 * Right now, defines from the command-line will surive restarts, but
 * defines from the config (via the Define directive) will not.
 */
int pr_define_add(const char *definition, int survive_restarts) {

  if (definition == NULL ||
      (survive_restarts != FALSE && survive_restarts != TRUE)) {
    errno = EINVAL;
    return -1;
  }

  if (survive_restarts == FALSE) {
    if (defines_pool == NULL) {
      defines_pool = make_sub_pool(permanent_pool);
      pr_pool_tag(defines_pool, "Defines Pool");
      pr_event_register(NULL, "core.restart", define_restart_ev, NULL);
    }

    if (!defines_list) {
      defines_list = make_array(defines_pool, 0, sizeof(char *));

    }

    *((char **) push_array(defines_list)) = pstrdup(defines_pool, definition);
    return 0;
  }

  if (defines_perm_pool == NULL) {
    defines_perm_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(defines_perm_pool, "Permanent Defines Pool");
  }

  if (!defines_perm_list) {
    defines_perm_list = make_array(defines_perm_pool, 0, sizeof(char *)); 
  }

  *((char **) push_array(defines_perm_list)) =
    pstrdup(defines_perm_pool, definition);
  return 0;
}

unsigned char pr_define_exists(const char *definition) {
  if (definition == NULL) {
    errno = EINVAL;
    return FALSE;
  }

  if (defines_list) {
    char **defines = defines_list->elts;
    register unsigned int i = 0;

    for (i = 0; i < defines_list->nelts; i++) {
      if (defines[i] &&
          strcmp(defines[i], definition) == 0)
        return TRUE;
    }
  }

  if (defines_perm_list) {
    char **defines = defines_perm_list->elts;
    register unsigned int i = 0;

    for (i = 0; i < defines_perm_list->nelts; i++) {
      if (defines[i] &&
          strcmp(defines[i], definition) == 0)
        return TRUE;
    }
  }

  errno = ENOENT;
  return FALSE;
}

void kludge_disable_umask(void) {
  _kludge_disable_umask = TRUE;
}

void kludge_enable_umask(void) {
  _kludge_disable_umask = FALSE;
}

/* Adds a config_rec to the specified set */
config_rec *add_config_set(xaset_t **set, const char *name) {
  pool *conf_pool = NULL, *set_pool = NULL;
  config_rec *c, *parent = NULL;

  if (!*set) {

    /* Allocate a subpool from permanent_pool for the set. */
    set_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(set_pool, "config set pool");

    *set = xaset_create(set_pool, NULL);
    (*set)->pool = set_pool;

    /* Now, make a subpool for the config_rec to be allocated.  The default
     * pool size (PR_TUNABLE_NEW_POOL_SIZE, 512 by default) is a bit large
     * for config_rec pools; use a smaller size.
     */
    conf_pool = pr_pool_create_sz(set_pool, 128);

  } else {

    /* Find the parent set for the config_rec to be allocated. */
    if ((*set)->xas_list)
      parent = ((config_rec *) ((*set)->xas_list))->parent;

    /* Now, make a subpool for the config_rec to be allocated.  The default
     * pool size (PR_TUNABLE_NEW_POOL_SIZE, 512 by default) is a bit large
     * for config_rec pools; use a smaller size.  Allocate the subpool
     * from the parent's pool.
     */
    conf_pool = pr_pool_create_sz((*set)->pool, 128);
  }

  pr_pool_tag(conf_pool, "config_rec pool");

  c = (config_rec *) pcalloc(conf_pool, sizeof(config_rec));

  c->pool = conf_pool;
  c->set = *set;
  c->parent = parent;

  if (name) {
    c->name = pstrdup(conf_pool, name);
    c->config_id = pr_config_set_id(c->name);
  }

  xaset_insert_end(*set, (xasetmember_t *) c);
  return c;
}

/* Adds a config_rec to the given server.  If no server is specified, the
 * config_rec is added to the current "level".
 */
config_rec *add_config(server_rec *s, const char *name) {
  config_rec *parent = NULL, *c = NULL;
  pool *p = NULL;
  xaset_t **set = NULL;

  if (s == NULL) {
    s = pr_parser_server_ctxt_get();
  }

  c = pr_parser_config_ctxt_get();

  if (c) {
    parent = c;
    p = c->pool;
    set = &c->subset;

  } else {
    parent = NULL;

    if (s->conf == NULL ||
        s->conf->xas_list == NULL) {

      p = make_sub_pool(s->pool);
      pr_pool_tag(p, "add_config() subpool");

    } else {
      p = ((config_rec *) s->conf->xas_list)->pool;
    }

    set = &s->conf;
  }

  if (!*set) {
    *set = xaset_create(p, NULL);
  }

  c = add_config_set(set, name);
  c->parent = parent;

  return c;
}

/* Per-directory configuration */

static size_t _strmatch(register char *s1, register char *s2) {
  register size_t len = 0;

  while (*s1 && *s2 && *s1++ == *s2++)
    len++;

  return len;
}

static config_rec *recur_match_path(pool *p, xaset_t *s, char *path) {
  char *tmp_path = NULL;
  config_rec *c = NULL, *res = NULL;

  if (!s) {
    errno = EINVAL;
    return NULL;
  }

  for (c = (config_rec *) s->xas_list; c; c = c->next) {
    if (c->config_type == CONF_DIR) {
      size_t path_len;

      tmp_path = c->name;

      if (c->argv[1]) {
        if (*(char *)(c->argv[1]) == '~')
          c->argv[1] = dir_canonical_path(c->pool, (char *) c->argv[1]);

        tmp_path = pdircat(p, (char *) c->argv[1], tmp_path, NULL);
      }

      /* Exact path match */
      if (strcmp(tmp_path, path) == 0) {
        pr_trace_msg("directory", 8,
          "<Directory %s> is an exact path match for '%s'", c->name, path);
        return c;
      }

      /* Bug#3146 occurred because using strstr(3) works well for paths
       * which DO NOT contain the glob sequence, i.e. we used to do:
       *
       *  if (strstr(tmp_path, slash_star) == NULL) {
       *
       * But what if they do, just not at the end of the path?
       *
       * The fix is to explicitly check the last two characters of the path
       * for '/' and '*', rather than using strstr(3).  (Again, I wish there
       * was a strrstr(3) libc function.)
       */
      path_len = strlen(tmp_path);
      if (path_len >= 2 &&
          !(tmp_path[path_len-2] == '/' && tmp_path[path_len-1] == '*')) {

        /* Trim a trailing path separator, if present. */
        if (*tmp_path && *(tmp_path + path_len - 1) == '/' && path_len > 1) {
          *(tmp_path + path_len - 1) = '\0';
          path_len--;

          if (strcmp(tmp_path, path) == 0) {
            pr_trace_msg("directory", 8,
              "<Directory %s> is an exact path match for '%s'", c->name, path);
            return c;
          }
        }

        tmp_path = pdircat(p, tmp_path, "*", NULL);

      } else if (path_len == 1) {
        /* We still need to append the "*" if the path is just '/'. */
        tmp_path = pstrcat(p, tmp_path, "*", NULL);
      }

      pr_trace_msg("directory", 9,
        "checking if <Directory %s> is a glob match for %s", tmp_path, path);

      /* The flags argument here needs to include PR_FNM_PATHNAME in order
       * to prevent globs from matching the '/' character.
       */
      if (pr_fnmatch(tmp_path, path, 0) == 0) {
        pr_trace_msg("directory", 8,
          "<Directory %s> is a glob match for '%s'", tmp_path, path);

        if (c->subset) {
          /* If there's a subset config, check to see if there's a closer
           * match there.
           */
          res = recur_match_path(p, c->subset, path);
          if (res) {
            pr_trace_msg("directory", 8,
              "found closer matching <Directory %s> for '%s' in <Directory %s> "
              "sub-config", res->name, path, tmp_path);
            return res;
          }
        }

        pr_trace_msg("directory", 8, "found <Directory %s> for '%s'",
          c->name, path);
        return c;
      }
    }
  }

  errno = ENOENT;
  return NULL;
}

config_rec *dir_match_path(pool *p, char *path) {
  config_rec *res = NULL;
  char *tmp = NULL;
  size_t tmplen;

  if (!p || !path || !*path) {
    return NULL;
  }

  tmp = pstrdup(p, path);
  tmplen = strlen(tmp);

  if (*(tmp + tmplen - 1) == '*') {
    *(tmp + tmplen - 1) = '\0';
    tmplen = strlen(tmp);
  }

  if (*(tmp + tmplen - 1) == '/' && tmplen > 1)
    *(tmp + tmplen - 1) = '\0';

  if (session.anon_config) {
    res = recur_match_path(p, session.anon_config->subset, tmp);

    if (!res) {
      if (session.chroot_path &&
          !strncmp(session.chroot_path, tmp, strlen(session.chroot_path)))
        return NULL;
    }
  }

  if (!res) {
    res = recur_match_path(p, main_server->conf, tmp);
  }

  if (res) {
    pr_trace_msg("directory", 3, "matched <Directory %s> for path '%s'",
      res->name, tmp);

  } else {
    pr_trace_msg("directory", 3, "no matching <Directory> found for '%s': %s",
      tmp, strerror(errno));
  }

  return res;
}

/* Returns TRUE to allow, FALSE to deny. */
static int dir_check_op(pool *p, xaset_t *set, int op, const char *path,
    uid_t file_uid, gid_t file_gid, mode_t mode) {
  int res = TRUE;
  config_rec *c;
  unsigned char *hide_no_access = NULL;

  /* Default is to allow. */
  if (!set)
    return TRUE;

  switch (op) {
    case OP_HIDE:
      c = find_config(set, CONF_PARAM, "HideUser", FALSE);
      while (c) {
        unsigned char inverted;
        uid_t hide_uid;

        pr_signals_handle();

        hide_uid = *((uid_t *) c->argv[0]);
        inverted = *((unsigned char *) c->argv[1]);

        if (hide_uid == (uid_t) -1) {
          hide_uid = session.uid;
        }

        if (file_uid == hide_uid) {
          if (!inverted)
            res = FALSE;

          break;

        } else {
          if (inverted) {
            res = FALSE;
            break;
          }
        }

        c = find_config_next(c, c->next, CONF_PARAM, "HideUser", FALSE);
      }

      c = find_config(set, CONF_PARAM, "HideGroup", FALSE);
      while (c) {
        unsigned char inverted;
        gid_t hide_gid;

        pr_signals_handle();

        hide_gid = *((gid_t *) c->argv[0]);
        inverted = *((unsigned char *) c->argv[1]);

        if (hide_gid != (gid_t) -1) {
          if (file_gid == hide_gid) {
            if (!inverted)
              res = FALSE;

            break;

          } else {
            if (inverted) {
              res = FALSE;
              break;
            }
          }

        } else {
          register unsigned int i;
          gid_t *group_ids = session.gids->elts;

          /* First check to see if the file GID matches the session GID. */
          if (file_gid == session.gid) {
            if (!inverted)
              res = FALSE;

            break;
          }

          /* Next, scan the list of supplemental groups for this user. */
          for (i = 0; i < session.gids->nelts; i++) {
            if (file_gid == group_ids[i]) {
              if (!inverted)
                res = FALSE;

              break;
            }
          }

          if (inverted) {
            res = FALSE;
            break;
          }
        }

        c = find_config_next(c, c->next, CONF_PARAM, "HideGroup", FALSE);
      }

      hide_no_access = get_param_ptr(set, "HideNoAccess", FALSE);
      if (hide_no_access &&
          *hide_no_access == TRUE) {

        if (S_ISDIR(mode)) {
          /* Check to see if the mode of this directory allows the
           * current user to list its contents.
           */
          res = pr_fsio_access(path, X_OK, session.uid, session.gid,
            session.gids) == 0 ? TRUE : FALSE;

        } else {
          /* Check to see if the mode of this file allows the current
           * user to read it.
           */
          res = pr_fsio_access(path, R_OK, session.uid, session.gid,
            session.gids) == 0 ? TRUE : FALSE;
        }
      }
      break;

    case OP_COMMAND: {
      unsigned char *allow_all = get_param_ptr(set, "AllowAll", FALSE);
      unsigned char *deny_all = get_param_ptr(set, "DenyAll", FALSE);

      if (allow_all &&
          *allow_all == TRUE) {
        /* No-op */
        ;

      } else if (deny_all &&
                 *deny_all == TRUE) {
        res = FALSE;
        errno = EACCES;
      }
    }

    break;
  }

  return res;
}

static int check_user_access(xaset_t *set, const char *name) {
  int res = 0;
  config_rec *c = find_config(set, CONF_PARAM, name, FALSE);

  while (c) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
    if (*((unsigned char *) c->argv[0]) == PR_EXPR_EVAL_REGEX) {
      regex_t *preg = (regex_t *) c->argv[1];

      if (regexec(preg, session.user, 0, NULL, 0) == 0) {
        res = TRUE;
        break;
      }

    } else
#endif /* HAVE_REGEX_H and HAVE_REGCOMP */

    if (*((unsigned char *) c->argv[0]) == PR_EXPR_EVAL_OR) {
      res = pr_expr_eval_user_or((char **) &c->argv[1]);

      if (res == TRUE)
        break;

    } else if (*((unsigned char *) c->argv[0]) == PR_EXPR_EVAL_AND) {
      res = pr_expr_eval_user_and((char **) &c->argv[1]);

      if (res == TRUE)
        break;
    }

    c = find_config_next(c, c->next, CONF_PARAM, name, FALSE);
  }

  return res;
}

static int check_group_access(xaset_t *set, const char *name) {
  int res = 0;
  config_rec *c = find_config(set, CONF_PARAM, name, FALSE);

  while (c) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
    if (*((unsigned char *) c->argv[0]) == PR_EXPR_EVAL_REGEX) {
      regex_t *preg = (regex_t *) c->argv[1];

      if (session.group && regexec(preg, session.group, 0, NULL, 0) == 0) {
        res = TRUE;
        break;

      } else if (session.groups) {
        register int i = 0;

        for (i = session.groups->nelts-1; i >= 0; i--)
          if (regexec(preg, *(((char **) session.groups->elts) + i), 0,
              NULL, 0) == 0) {
            res = TRUE;
            break;
          }
      }

    } else
#endif /* HAVE_REGEX_H and HAVE_REGCOMP */

    if (*((unsigned char *) c->argv[0]) == PR_EXPR_EVAL_OR) {
      res = pr_expr_eval_group_or((char **) &c->argv[1]);

      if (res == TRUE)
        break;

    } else if (*((unsigned char *) c->argv[0]) == PR_EXPR_EVAL_AND) {
      res = pr_expr_eval_group_and((char **) &c->argv[1]);

      if (res == TRUE)
        break;
    }

    c = find_config_next(c, c->next, CONF_PARAM, name, FALSE);
  }

  return res;
}

static int check_class_access(xaset_t *set, const char *name) {
  int res = 0;
  config_rec *c = find_config(set, CONF_PARAM, name, FALSE);

  while (c) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
    if (*((unsigned char *) c->argv[0]) == PR_EXPR_EVAL_REGEX) {
      regex_t *preg = (regex_t *) c->argv[1];

      if (session.class &&
          regexec(preg, session.class->cls_name, 0, NULL, 0) == 0) {
        res = TRUE;
        break;
      }

    } else
#endif /* HAVE_REGEX_H and HAVE_REGCOMP */

    if (*((unsigned char *) c->argv[0]) == PR_EXPR_EVAL_OR) {
      res = pr_expr_eval_class_or((char **) &c->argv[1]);

      if (res == TRUE)
        break;

    } else if (*((unsigned char *) c->argv[0]) == PR_EXPR_EVAL_AND) {
      res = pr_expr_eval_class_and((char **) &c->argv[1]);

      if (res == TRUE)
        break;
    }

    c = find_config_next(c, c->next, CONF_PARAM, name, FALSE);
  }

  return res;
}

static int check_filter_access(xaset_t *set, const char *name, cmd_rec *cmd) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  int res = 0;
  config_rec *c;

  if (!cmd)
    return res;

  c = find_config(set, CONF_PARAM, name, FALSE);
  while (c) {
    regex_t *preg = (regex_t *) c->argv[0];

    if (regexec(preg, cmd->arg, 0, NULL, 0) == 0) {
      res = TRUE;
      break;
    }

    c = find_config_next(c, c->next, CONF_PARAM, name, FALSE);
  }

  return res;
#else
  return 0;
#endif /* HAVE_REGEX_H and HAVE_REGCOMP */
}

/* As of 1.2.0rc3, a '!' character in front of the IP address
 * negates the logic (i.e. doesn't match).
 *
 * Here are our rules for matching an IP/host list:
 *
 *   (negate-cond-1 && negate-cond-2 && ... negate-cond-n) &&
 *   (cond-1 || cond-2 || ... cond-n)
 *
 * This boils down to the following two rules:
 *
 *   1. ALL negative ('!') conditions must evaluate to logically TRUE.
 *   2. One (or more) normal conditions must evaluate to logically TRUE.
 */

/* Check an ACL for negated rules and make sure all of them evaluate to TRUE.
 * Default (if none exist) is TRUE.
 */
static int check_ip_negative(const config_rec *c) {
  int aclc;
  pr_netacl_t **aclv;

  for (aclc = c->argc, aclv = (pr_netacl_t **) c->argv; aclc; aclc--, aclv++) {
    if (pr_netacl_get_negated(*aclv) == FALSE)
      continue;

    switch (pr_netacl_match(*aclv, session.c->remote_addr)) {
      case 1:
        /* This actually means we DIDN'T match, and it's ok to short circuit
         * everything (negative).
         */
        return FALSE;

      case -1:
        /* -1 signifies a NONE match, which isn't valid for negative
         * conditions.
         */
        pr_log_pri(PR_LOG_ERR, "ooops, it looks like !NONE was used in an ACL "
          "somehow.");
        return FALSE;

      default:
        /* This means our match is actually true and we can continue */
        break;
    }
  }

  /* If we got this far either all conditions were TRUE or there were no
   * conditions.
   */

  return TRUE;
}

/* Check an ACL for positive conditions, short-circuiting if ANY of them are
 * TRUE.  Default return is FALSE.
 */
static int check_ip_positive(const config_rec *c) {
  int aclc;
  pr_netacl_t **aclv;

  for (aclc = c->argc, aclv = (pr_netacl_t **) c->argv; aclc; aclc--, aclv++) {
    if (pr_netacl_get_negated(*aclv) == TRUE)
      continue;

    switch (pr_netacl_match(*aclv, session.c->remote_addr)) {
      case 1:
        /* Found it! */
        return TRUE;

      case -1:
        /* Special value "NONE", meaning nothing can match, so we can
         * short-circuit on this as well.
         */
        return FALSE;

      default:
        /* No match, keep trying */
        break;
    }
  }

  /* default return value is FALSE */
  return FALSE;
}

static int check_ip_access(xaset_t *set, char *name) {
  int res = FALSE;

  config_rec *c = find_config(set, CONF_PARAM, name, FALSE);

  while (c) {
    /* If the negative check failed (default is success), short-circuit and
     * return FALSE
     */
    if (check_ip_negative(c) != TRUE) {
      return FALSE;
    }

    /* Otherwise, continue on with boolean or check */
    if (check_ip_positive(c) == TRUE) {
      res = TRUE;
    }

    /* Continue on, in case there are other acls that need to be checked
     * (multiple acls are logically OR'd)
     */
    c = find_config_next(c, c->next, CONF_PARAM, name, FALSE);
  }

  return res;
}

/* 1 if allowed, 0 otherwise */

static int check_limit_allow(config_rec *c, cmd_rec *cmd) {
  unsigned char *allow_all = NULL;

  /* If session.groups is null, this means no authentication
   * attempt has been made, so we simply check for the
   * very existance of an AllowGroup, and assume (for now) it's
   * allowed.  This works because later calls to check_limit_allow()
   * WILL have filled in the group members and we can truely check
   * group membership at that time.  Same goes for AllowUser.
   */

  if (!session.user) {
    if (find_config(c->subset, CONF_PARAM, "AllowUser", FALSE)) {
      return 1;
    }

  } else if (check_user_access(c->subset, "AllowUser")) {
    return 1;
  }

  if (!session.groups) {
    if (find_config(c->subset, CONF_PARAM, "AllowGroup", FALSE)) {
      return 1;
    }

  } else if (check_group_access(c->subset, "AllowGroup")) {
    return 1;
  }

  if (session.class &&
      check_class_access(c->subset, "AllowClass")) {
    return 1;
  }

  if (check_ip_access(c->subset, "Allow")) {
    return 1;
  }

  if (check_filter_access(c->subset, "AllowFilter", cmd)) {
    return 1;
  }

  allow_all = get_param_ptr(c->subset, "AllowAll", FALSE);
  if (allow_all &&
      *allow_all == TRUE) {
    return 1;
  }

  return 0;
}

static int check_limit_deny(config_rec *c, cmd_rec *cmd) {
  unsigned char *deny_all = get_param_ptr(c->subset, "DenyAll", FALSE);

  if (deny_all &&
      *deny_all == TRUE) {
    return 1;
  }

  if (session.user &&
      check_user_access(c->subset, "DenyUser")) {
    return 1;
  }

  if (session.groups &&
      check_group_access(c->subset, "DenyGroup")) {
    return 1;
  }

  if (session.class &&
      check_class_access(c->subset, "DenyClass")) {
    return 1;
  }

  if (check_ip_access(c->subset, "Deny")) {
    return 1;
  }

  if (check_filter_access(c->subset, "DenyFilter", cmd)) {
    return 1;
  }

  return 0;
}

/* check_limit returns 1 if allowed, 0 if implicitly allowed,
 * and -1 if implicitly denied and -2 if explicitly denied.
 */

static int check_limit(config_rec *c, cmd_rec *cmd) {
  int *tmp = get_param_ptr(c->subset, "Order", FALSE);
  int order = tmp ? *tmp : ORDER_ALLOWDENY;

  if (order == ORDER_DENYALLOW) {
    /* Check deny first */

    if (check_limit_deny(c, cmd)) {
      /* Explicit deny */
      errno = EPERM;
      return -2;
    }

    if (check_limit_allow(c, cmd)) {
      /* Explicit allow */
      return 1;
    }

    /* Implicit deny */
    errno = EPERM;
    return -1;
  }

  /* Check allow first */
  if (check_limit_allow(c, cmd)) {
    /* Explicit allow */
    return 1;
  }

  if (check_limit_deny(c, cmd)) {
    /* Explicit deny */
    errno = EPERM;
    return -2;
  }

  /* Implicit allow */
  return 0;
}

/* Note: if and == 1, the logic is short circuited so that the first
 * failure results in a FALSE return from the entire function, if and
 * == 0, an ORing operation is assumed and the function will return
 * TRUE if any <limit LOGIN> allows access.
 */

int login_check_limits(xaset_t *set, int recurse, int and, int *found) {
  int res = and;
  int rfound;
  config_rec *c;
  int argc;
  char **argv;

  *found = 0;

  if (!set || !set->xas_list)
    return TRUE;			/* default is to allow */

  /* First check top level */
  for (c = (config_rec *) set->xas_list; c; c = c->next) {
    if (c->config_type == CONF_LIMIT) {
      for (argc = c->argc, argv = (char **) c->argv; argc; argc--, argv++) {
        if (strcasecmp("LOGIN", *argv) == 0) {
          break;
        }
      }

      if (argc) {
        if (and) {
          switch (check_limit(c, NULL)) {
            case 1:
              res = (res && TRUE);
              (*found)++;
              break;

	    case -1:
            case -2:
              res = (res && FALSE);
              (*found)++;
              break;
          }

          if (!res)
            break;

        } else {
          switch (check_limit(c, NULL)) {
            case 1:
              res = TRUE;

	    case -1:
            case -2:
              (*found)++;
              break;
          }
        }
      }
    }
  }

  if (((res && and) || (!res && !and && *found)) && recurse) {
    for (c = (config_rec *) set->xas_list; c; c = c->next) {
      if (c->config_type == CONF_ANON &&
          c->subset &&
          c->subset->xas_list) {
       if (and) {
         res = (res && login_check_limits(c->subset, recurse, and, &rfound));
         (*found) += rfound;
         if (!res)
           break;

       } else {
         int rres;

         rres = login_check_limits(c->subset, recurse, and, &rfound);
         if (rfound)
           res = (res || rres);

         (*found) += rfound;
         if (res)
           break;
       }
     }
    }
  }

  if (!*found && !and)
    return TRUE;			/* Default is to allow */

  return res;
}

/* Check limit directives.
 */
static int check_limits(xaset_t *set, cmd_rec *cmd, const char *cmd_name,
    int hidden) {
  int res = 1, ignore_hidden = -1;
  config_rec *lc = NULL;

  errno = 0;

  if (!set)
    return res;

  for (lc = (config_rec *) set->xas_list; lc && (res == 1); lc = lc->next) {
    pr_signals_handle();

    if (lc->config_type == CONF_LIMIT) {
      register unsigned int i = 0;

      for (i = 0; i < lc->argc; i++) {
        if (strcasecmp(cmd_name, (char *) lc->argv[i]) == 0) {
          break;
        }
      }
	
      if (i == lc->argc)
        continue;

      /* Found a <Limit> directive associated with the current command.
       * ignore_hidden defaults to -1, if an explicit IgnoreHidden off is seen,
       * it is set to 0 and the check will not be done again up the chain.  If
       * an explicit "IgnoreHidden on" is seen, checking short-circuits and we
       * set ENOENT.
       */

      if (hidden && ignore_hidden == -1) {
        unsigned char *ignore = get_param_ptr(lc->subset, "IgnoreHidden",
          FALSE);

        if (ignore)
          ignore_hidden = *ignore;

        if (ignore_hidden == 1) {
          res = 0;
          errno = ENOENT;
          break;
        }
      }

      switch (check_limit(lc, cmd)) {
        case 1:
          res++;
          break;
	
        case -1:
        case -2:
          res = 0;
          break;
	
        default:
          continue;
      }
    }
  }

  if (!res && !errno)
    errno = EACCES;

  return res;
}

int dir_check_limits(cmd_rec *cmd, config_rec *c, const char *cmd_name,
    int hidden) {
  int res = 1;

  for (; c && (res == 1); c = c->parent) {
    res = check_limits(c->subset, cmd, cmd_name, hidden);
  }

  if (!c && (res == 1)) {
    /* vhost or main server has been reached without an explicit permit or deny,
     * so try the current server.
     */
    res = check_limits(main_server->conf, cmd, cmd_name, hidden);
  }

  return res;
}

void build_dyn_config(pool *p, const char *_path, struct stat *stp,
    unsigned char recurse) {
  char *fullpath = NULL, *path = NULL, *dynpath = NULL, *cp = NULL;
  struct stat st;
  config_rec *d = NULL;
  xaset_t **set = NULL;
  int isfile, removed = 0;

  /* Switch through each directory, from "deepest" up looking for
   * new or updated .ftpaccess files
   */

  if (!_path)
    return;

  /* Check to see whether .ftpaccess files are allowed to be parsed. */
  if (!allow_dyn_config(_path))
    return;

  path = pstrdup(p, _path);

  memcpy(&st, stp, sizeof(st));

  if (S_ISDIR(st.st_mode)) {
    dynpath = pdircat(p, path, "/.ftpaccess", NULL);

  } else {
    dynpath = NULL;
  }

  while (path) {
    pr_signals_handle();

    if (session.chroot_path) {
      fullpath = pdircat(p, session.chroot_path, path, NULL);

      if (strcmp(fullpath, "/") &&
          *(fullpath + strlen(fullpath) - 1) == '/') {
        *(fullpath + strlen(fullpath) - 1) = '\0';
      }

    } else {
      fullpath = path;
    }

    if (dynpath) {
      isfile = pr_fsio_stat(dynpath, &st);

    } else {
      isfile = -1;
    }

    d = dir_match_path(p, fullpath);

    if (!d &&
        isfile != -1 &&
        st.st_size > 0) {
      set = (session.anon_config ? &session.anon_config->subset :
        &main_server->conf);

      pr_trace_msg("ftpaccess", 6, "adding config for '%s'", fullpath);

      d = add_config_set(set, fullpath);
      d->config_type = CONF_DIR;
      d->argc = 1;
      d->argv = pcalloc(d->pool, 2 * sizeof (void *));

    } else if (d) {
      config_rec *newd, *dnext;

      if (isfile != -1 &&
          st.st_size > 0 &&
          strcmp(d->name, fullpath) != 0) {
        set = &d->subset;

        pr_trace_msg("ftpaccess", 6, "adding config for '%s'", fullpath);

        newd = add_config_set(set, fullpath);
        newd->config_type = CONF_DIR;
        newd->argc = 1;
        newd->argv = pcalloc(newd->pool, 2 * sizeof(void *));
	newd->parent = d;

        d = newd;

      } else if (strcmp(d->name, fullpath) == 0 &&
          (isfile == -1 ||
           st.st_mtime > (d->argv[0] ? *((time_t *) d->argv[0]) : 0))) {

        set = (d->parent ? &d->parent->subset : &main_server->conf);

	if (d->subset &&
            d->subset->xas_list) {

       	  /* Remove all old dynamic entries. */
          for (newd = (config_rec *) d->subset->xas_list; newd; newd = dnext) {
	    dnext = newd->next;

            if (newd->flags & CF_DYNAMIC) {
              xaset_remove(d->subset, (xasetmember_t *) newd);
              removed++;
            }
          }
	}

        if (d->subset &&
            !d->subset->xas_list) {
          destroy_pool(d->subset->pool);
          d->subset = NULL;
          d->argv[0] = NULL;

	  /* If the file has been removed and no entries exist in this
           * dynamic entry, remove it completely.
           */
          if (isfile == -1) {
            xaset_remove(*set, (xasetmember_t *) d);
          }
        }
      }
    }

    if (isfile != -1 &&
        d &&
        st.st_size > 0 &&
        st.st_mtime > (d->argv[0] ? *((time_t *) d->argv[0]) : 0)) {
      int res;

      /* File has been modified or not loaded yet */
      d->argv[0] = pcalloc(d->pool, sizeof(time_t));
      *((time_t *) d->argv[0]) = st.st_mtime;

      d->config_type = CONF_DYNDIR;

      pr_trace_msg("ftpaccess", 3, "parsing '%s'", dynpath);

      pr_parser_prepare(p, NULL);
      res = pr_parser_parse_file(p, dynpath, d, PR_PARSER_FL_DYNAMIC_CONFIG);
      pr_parser_cleanup();

      if (res == 0) {
        d->config_type = CONF_DIR;
        merge_down(*set, TRUE);

        pr_trace_msg("ftpaccess", 3, "fixing up directory configs");
        fixup_dirs(main_server, CF_SILENT);

      } else {
        pr_log_debug(DEBUG0, "error parsing '%s': %s", dynpath,
          strerror(errno));
      }
    }

    if (isfile == -1 &&
        removed &&
        d &&
        set) {
      pr_log_debug(DEBUG5, "dynamic configuration removed for %s", fullpath);
      merge_down(*set, FALSE);
    }

    if (!recurse)
      break;

    cp = strrchr(path, '/');
    if (cp) {
      if (strcmp(path, "/") != 0) {

        /* We need to handle the case where path might be "/path".  We
         * can't just set *cp to '\0', as that would result in the empty
         * string.  Thus check if cp is the same value as path, i.e.
         * that cp points to the start of the string.  If so, by definition
         * we know that we are dealing with the "/path" case.
         */
        if (cp != path) {
          *cp = '\0';

        } else {
          /* Set the recurse flag to 'false', so that we go one more pass
           * through the loop, but stop after that.
           */
          *(cp+1) = '\0';
          recurse = FALSE;        
        }

      } else {
        /* Set the recurse flag to 'false', so that we go one more pass
         * through the loop, but stop after that.
         */
        recurse = FALSE;        
      }

    } else {
      path = NULL;
    }

    if (path) {
      if (*path && *(path + strlen(path) - 1) == '*')
        *(path +strlen(path) - 1) = '\0';

      dynpath = pdircat(p, path, "/.ftpaccess", NULL);
    }
  }
}

/* dir_check_full() fully recurses the path passed
 * returns 1 if operation is allowed on current path,
 * or 0 if not.
 */

/* dir_check_full() and dir_check() both take a `hidden' argument which is a
 * pointer to an integer. This is provided so that they can tell the calling
 * function if an entry should be hidden or not.  This is used by mod_ls to
 * determine if a file should be displayed.  Note that in this context, hidden
 * means "hidden by configuration" (HideUser, etc), NOT "hidden because it's a
 * .dotfile".
 */

int dir_check_full(pool *pp, cmd_rec *cmd, const char *group, const char *path,
    int *hidden) {
  char *fullpath, *owner;
  config_rec *c;
  struct stat st;
  pool *p;
  mode_t _umask = (mode_t) -1;
  int res = 1, isfile;
  int op_hidden = FALSE, regex_hidden = FALSE;

  if (!path) {
    errno = EINVAL;
    return -1;
  }

  p = make_sub_pool(pp);
  pr_pool_tag(p, "dir_check_full() subpool");

  fullpath = (char *) path;

  if (session.chroot_path)
    fullpath = pdircat(p, session.chroot_path, fullpath, NULL);

  pr_log_debug(DEBUG5, "in dir_check_full(): path = '%s', fullpath = '%s'.",
            path, fullpath);

  /* Check and build all appropriate dynamic configuration entries */
  pr_fs_clear_cache();
  isfile = pr_fsio_stat(path, &st);
  if (isfile == -1)
    memset(&st, '\0', sizeof(st));

  build_dyn_config(p, path, &st, TRUE);

  /* Check to see if this path is hidden by HideFiles. */
  regex_hidden = dir_hide_file(path);

  /* Cache a pointer to the set of configuration data for this directory in
   * session.dir_config.
   */
  session.dir_config = c = dir_match_path(p, fullpath);
  if (session.dir_config) {
    pr_trace_msg("directory", 2, "matched <Directory %s> for '%s'",
      session.dir_config->name, fullpath);
  }

  if (!c && session.anon_config)
    c = session.anon_config;

  if (!_kludge_disable_umask) {
    /* Check for a directory Umask. */
    if (S_ISDIR(st.st_mode) ||
        strcmp(cmd->argv[0], C_MKD) == 0 ||
        strcmp(cmd->argv[0], C_XMKD) == 0) {
      mode_t *dir_umask = get_param_ptr(CURRENT_CONF, "DirUmask", FALSE);
      _umask = dir_umask ? *dir_umask : (mode_t) -1;
    }

    /* It's either a file, or we had no directory Umask. */
    if (_umask == (mode_t) -1) {
      mode_t *file_umask = get_param_ptr(CURRENT_CONF, "Umask", FALSE);
      _umask = file_umask ? *file_umask : (mode_t) 0022;
    }
  }

  session.fsuid = (uid_t) -1;
  session.fsgid = (gid_t) -1;

  owner = get_param_ptr(CURRENT_CONF, "UserOwner", FALSE);
  if (owner != NULL) {
    /* Attempt chown() on all new files. */
    struct passwd *pw;

    pw = pr_auth_getpwnam(p, owner);
    if (pw != NULL)
      session.fsuid = pw->pw_uid;
  }

  owner = get_param_ptr(CURRENT_CONF, "GroupOwner", FALSE);
  if (owner != NULL) {
    /* Attempt chgrp() on all new files. */
    struct group *gr;

    gr = pr_auth_getgrnam(p, owner);
    if (gr != NULL)
      session.fsgid = gr->gr_gid;
  }

  if (isfile != -1) {
    /* Check to see if the current config "hides" the path or not. */
    op_hidden = !dir_check_op(p, CURRENT_CONF, OP_HIDE,
      session.chroot_path ? path : fullpath, st.st_uid, st.st_gid, st.st_mode);

    res = dir_check_op(p, CURRENT_CONF, OP_COMMAND,
      session.chroot_path ? path : fullpath, st.st_uid, st.st_gid, st.st_mode);
  }

  if (res) {
    /* Note that dir_check_limits() also handles IgnoreHidden.  If it is set,
     * these return 0 (no access), and also set errno to ENOENT so it looks
     * like the file doesn't exist.
     */
    res = dir_check_limits(cmd, c, cmd->argv[0], op_hidden || regex_hidden);

    /* If specifically allowed, res will be > 1 and we don't want to
     * check the command group limit.
     */
    if (res == 1 && group)
      res = dir_check_limits(cmd, c, group, op_hidden || regex_hidden);

    /* If still == 1, no explicit allow so check lowest priority "ALL" group.
     * Note that certain commands are deliberately excluded from the
     * ALL group (i.e. EPRT, EPSV, PASV, and PORT).
     */
    if (res == 1 &&
        strcmp(cmd->argv[0], C_EPRT) != 0 &&
        strcmp(cmd->argv[0], C_EPSV) != 0 &&
        strcmp(cmd->argv[0], C_PASV) != 0 &&
        strcmp(cmd->argv[0], C_PORT) != 0)
      res = dir_check_limits(cmd, c, "ALL", op_hidden || regex_hidden);
  }

  if (res &&
      _umask != (mode_t) -1)
    pr_log_debug(DEBUG5,
      "in dir_check_full(): setting umask to %04o (was %04o)",
        (unsigned int) _umask, (unsigned int) umask(_umask));

  destroy_pool(p);

  if (hidden)
    *hidden = op_hidden || regex_hidden;

  return res;
}

/* dir_check() checks the current dir configuration against the path,
 * if it matches (partially), a search is done only in the subconfig,
 * otherwise handed off to dir_check_full
 */

int dir_check(pool *pp, cmd_rec *cmd, const char *group, const char *path,
    int *hidden) {
  char *fullpath, *owner;
  config_rec *c;
  struct stat st;
  pool *p;
  mode_t _umask = (mode_t) -1;
  int res = 1, isfile;
  int op_hidden = FALSE, regex_hidden = FALSE;

  if (!path) {
    errno = EINVAL;
    return -1;
  }

  p = make_sub_pool(pp);
  pr_pool_tag(p, "dir_check() subpool");

  fullpath = (char *) path;

  if (session.chroot_path)
    fullpath = pdircat(p, session.chroot_path, fullpath, NULL);

  c = (session.dir_config ? session.dir_config :
        (session.anon_config ? session.anon_config : NULL));

  if (!c || strncmp(c->name, fullpath, strlen(c->name)) != 0) {
    destroy_pool(p);
    return dir_check_full(pp, cmd, group, path, hidden);
  }

  /* Check and build all appropriate dynamic configuration entries */
  pr_fs_clear_cache();
  isfile = pr_fsio_stat(path, &st);
  if (isfile == -1)
    memset(&st, 0, sizeof(st));

  build_dyn_config(p, path, &st, FALSE);

  /* Check to see if this path is hidden by HideFiles. */
  regex_hidden = dir_hide_file(path);

  /* Cache a pointer to the set of configuration data for this directory in
   * session.dir_config.
   */
  session.dir_config = c = dir_match_path(p, fullpath);
  if (session.dir_config) {
    pr_trace_msg("directory", 2, "matched <Directory %s> for '%s'",
      session.dir_config->name, fullpath);
  }

  if (!c && session.anon_config)
    c = session.anon_config;

  if (!_kludge_disable_umask) {
    /* Check for a directory Umask. */
    if (S_ISDIR(st.st_mode) ||
        strcmp(cmd->argv[0], C_MKD) == 0 ||
        strcmp(cmd->argv[0], C_XMKD) == 0) {
      mode_t *dir_umask = get_param_ptr(CURRENT_CONF, "DirUmask", FALSE);
      _umask = dir_umask ? *dir_umask : (mode_t) -1;
    }

    /* It's either a file, or we had no directory Umask. */
    if (_umask == (mode_t) -1) {
      mode_t *file_umask = get_param_ptr(CURRENT_CONF, "Umask", FALSE);
      _umask = file_umask ? *file_umask : (mode_t) 0022;
    }
  }

  session.fsuid = (uid_t) -1;
  session.fsgid = (gid_t) -1;

  owner = get_param_ptr(CURRENT_CONF, "UserOwner", FALSE);
  if (owner != NULL) {
    /* Attempt chown() on all new files. */
    struct passwd *pw;

    pw = pr_auth_getpwnam(p, owner);
    if (pw != NULL)
      session.fsuid = pw->pw_uid;
  }

  owner = get_param_ptr(CURRENT_CONF, "GroupOwner", FALSE);
  if (owner != NULL) {
    /* Attempt chgrp() on all new files. */
    struct group *gr;

    gr = pr_auth_getgrnam(p, owner);
    if (gr != NULL)
      session.fsgid = gr->gr_gid;
  }

  if (isfile != -1) {
    /* If not already marked as hidden by its name, check to see if the path
     * is to be hidden by nature of its mode
     */
    op_hidden = !dir_check_op(p, CURRENT_CONF, OP_HIDE,
      session.chroot_path ? path : fullpath, st.st_uid, st.st_gid, st.st_mode);

    res = dir_check_op(p, CURRENT_CONF, OP_COMMAND,
      session.chroot_path ? path : fullpath, st.st_uid, st.st_gid, st.st_mode);
  }

  if (res) {
    res = dir_check_limits(cmd, c, cmd->argv[0], op_hidden || regex_hidden);

    /* If specifically allowed, res will be > 1 and we don't want to
     * check the command group limit.
     */
    if (res == 1 && group)
      res = dir_check_limits(cmd, c, group, op_hidden || regex_hidden);

    /* If still == 1, no explicit allow so check lowest priority "ALL" group.
     * Note that certain commands are deliberately excluded from the
     * ALL group (i.e. EPRT, EPSV, PASV, and PORT).
     */
    if (res == 1 &&
        strcmp(cmd->argv[0], C_EPRT) != 0 &&
        strcmp(cmd->argv[0], C_EPSV) != 0 &&
        strcmp(cmd->argv[0], C_PASV) != 0 &&
        strcmp(cmd->argv[0], C_PORT) != 0)
      res = dir_check_limits(cmd, c, "ALL", op_hidden || regex_hidden);
  }

  if (res &&
      _umask != (mode_t) -1)
    pr_log_debug(DEBUG5, "in dir_check(): setting umask to %04o (was %04o)",
        (unsigned int) _umask, (unsigned int) umask(_umask));

  destroy_pool(p);

  if (hidden)
    *hidden = op_hidden || regex_hidden;

  return res;
}

/* dir_check_canon() canonocalizes as much of the path as possible (which may
 * not be all of it, as the target may not yet exist) then we hand off to
 * dir_check().
 */
int dir_check_canon(pool *pp, cmd_rec *cmd, const char *group,
    const char *path, int *hidden) {
  return dir_check(pp, cmd, group, dir_best_path(pp, path), hidden);
}

/* Move all the members (i.e. a "branch") of one config set to a different
 * parent.
 */
static void _reparent_all(config_rec *newparent, xaset_t *set) {
  config_rec *c,*cnext;

  if (!newparent->subset)
    newparent->subset = xaset_create(newparent->pool, NULL);

  for (c = (config_rec *) set->xas_list; c; c = cnext) {
    cnext = c->next;
    xaset_remove(set, (xasetmember_t *) c);
    xaset_insert(newparent->subset, (xasetmember_t *) c);
    c->set = newparent->subset;
    c->parent = newparent;
  }
}

/* Recursively find the most appropriate place to move a CONF_DIR
 * directive to.
 */

static config_rec *_find_best_dir(xaset_t *set, char *path, size_t *matchlen) {
  config_rec *c, *res = NULL, *rres;
  size_t len, pathlen, imatchlen, tmatchlen;

  *matchlen = 0;

  if (!set ||
      !set->xas_list)
    return NULL;

  pathlen = strlen(path);

  for (c = (config_rec *) set->xas_list; c; c = c->next) {
    if (c->config_type == CONF_DIR) {
      /* Note: this comparison of pointers, rather than of strings, is
       * intentional.  DO NOT CHANGE THIS TO A strcmp()!
       *
       * This function is only called by reorder_dirs(), and reorder_dirs()
       * always uses a c->name as the path parameter.  This means that
       * doing direct pointer/address comparisons is valid.  If ever this
       * assumption is broken, we will need to revert back to a more
       * costly (especially when there are many <Directory> config sections)
       * use of strcmp().
       */
      if (c->name == path)
        continue;

      len = strlen(c->name);
      while (len > 0 &&
             (*(c->name+len-1) == '*' || *(c->name+len-1) == '/'))
        len--;

      /*
       * Just a partial match on the pathname does not mean that the longer
       * path is the subdirectory of the other -- they might just be sharing
       * the last path component!
       * /var/www/.1
       * /var/www/.14
       *            ^ -- not /, not subdir
       * /var/www/.1
       * /var/www/.1/images
       *            ^ -- /, is subdir
       */
      if (pathlen > len &&
          path[len] != '/')
          continue;

      if (len < pathlen &&
          strncmp(c->name, path, len) == 0) {
        rres = _find_best_dir(c->subset ,path, &imatchlen);
        tmatchlen = _strmatch(path, c->name);
        if (!rres &&
            tmatchlen > *matchlen) {
          res = c;
          *matchlen = tmatchlen;

        } else if (imatchlen > *matchlen) {
          res = rres;
          *matchlen = imatchlen;
        }
      }
    }
  }

  return res;
}

/* Reorder all the CONF_DIR configuration sections, so that they are
 * in directory tree order
 */

static void _reorder_dirs(xaset_t *set, int flags) {
  config_rec *c = NULL, *cnext = NULL, *newparent = NULL;
  int defer = 0;
  size_t tmp;

  if (!set || !set->xas_list)
    return;

  if (!(flags & CF_DEFER))
    defer = 1;

  for (c = (config_rec *) set->xas_list; c; c = cnext) {
    cnext = c->next;

    if (c->config_type == CONF_DIR) {
      if (flags && !(c->flags & flags))
        continue;

      if (defer && (c->flags & CF_DEFER))
        continue;

      /* If <Directory *> is used inside <Anonymous>, move all
       * the directives from '*' into the higher level.
       */
      if (c->parent &&
          c->parent->config_type == CONF_ANON &&
          strcmp(c->name, "*") == 0) {

        if (c->subset)
          _reparent_all(c->parent, c->subset);

        xaset_remove(c->parent->subset, (xasetmember_t*) c);

      } else {
        newparent = _find_best_dir(set, c->name, &tmp);
        if (newparent) {
          if (!newparent->subset)
            newparent->subset = xaset_create(newparent->pool, NULL);

          xaset_remove(c->set, (xasetmember_t *) c);
          xaset_insert(newparent->subset, (xasetmember_t *) c);
          c->set = newparent->subset;
          c->parent = newparent;
        }
      }
    }
  }

  /* Top level is now sorted, now we recursively sort all the sublevels. */
  for (c = (config_rec *) set->xas_list; c; c = c->next)
    if (c->config_type == CONF_DIR || c->config_type == CONF_ANON)
      _reorder_dirs(c->subset, flags);
}

static void config_dumpf(const char *fmt, ...) {
  char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  va_list msg;

  va_start(msg, fmt);
  vsnprintf(buf, sizeof(buf), fmt, msg);
  va_end(msg);

  buf[sizeof(buf)-1] = '\0';

  pr_log_debug(DEBUG5, "%s", buf);
}

void pr_config_dump(void (*dumpf)(const char *, ...), xaset_t *s,
    char *indent) {
  config_rec *c = NULL;

  if (!s)
    return;

  if (!indent)
    indent = "";

  for (c = (config_rec *) s->xas_list; c; c = c->next) {

    /* Don't display directives whose name starts with an underscore. */
    if (c->name != NULL &&
        *(c->name) != '_') {
      dumpf("%s%s", indent, c->name);
    }

    if (c->subset) {
      pr_config_dump(dumpf, c->subset, pstrcat(c->pool, indent, " ", NULL));
    }
  }
}

#ifdef PR_USE_DEVEL
void pr_dirs_dump(void (*dumpf)(const char *, ...), xaset_t *s, char *indent) {
  config_rec *c;

  if (!s)
    return;

  if (!indent)
    indent = " ";

  for (c = (config_rec *) s->xas_list; c; c = c->next) {
    if (c->config_type != CONF_DIR)
      continue;

    dumpf("%s<Directory %s>", indent, c->name);

    if (c->subset)
      pr_dirs_dump(dumpf, c->subset, pstrcat(c->pool, indent, " ", NULL));
  }

  return;
}
#endif /* PR_USE_DEVEL */

static void merge_down(xaset_t *s, int dynamic) {
  config_rec *c, *dst, *newconf;
  int argc;
  void **argv, **sargv;

  if (!s ||
      !s->xas_list)
    return;

  for (c = (config_rec *) s->xas_list; c; c = c->next) {
    if ((c->flags & CF_MERGEDOWN) ||
        (c->flags & CF_MERGEDOWN_MULTI))
      for (dst = (config_rec *) s->xas_list; dst; dst = dst->next) {
        if (dst->config_type == CONF_ANON ||
           dst->config_type == CONF_DIR) {

          /* If an option of the same name/type is found in the
           * next level down, it overrides, so we don't merge.
           */
          if ((c->flags & CF_MERGEDOWN) &&
              find_config(dst->subset, c->config_type, c->name, FALSE))
            continue;

          if (dynamic) {
            /* If we are doing a dynamic merge (i.e. .ftpaccess files) then
             * we do not need to re-merge the static configs that are already
             * there.  Otherwise we are creating copies needlessly of any
             * config_rec marked with the CF_MERGEDOWN_MULTI flag, which
             * adds to the memory usage/processing time.
             *
             * If neither the src or the dst config have the CF_DYNAMIC
             * flag, it's a static config, and we can skip this merge and move
             * on.  Otherwise, we can merge it.
             */
            if (!(c->flags & CF_DYNAMIC) && !(dst->flags & CF_DYNAMIC)) {
              continue;
            }
          }

          if (!dst->subset)
            dst->subset = xaset_create(dst->pool, NULL);

          newconf = add_config_set(&dst->subset, c->name);
          newconf->config_type = c->config_type;
          newconf->flags = c->flags | (dynamic ? CF_DYNAMIC : 0);
          newconf->argc = c->argc;
          newconf->argv = pcalloc(newconf->pool, (c->argc+1) * sizeof(void *));
          argv = newconf->argv;
          sargv = c->argv;
          argc = newconf->argc;

          while (argc--) {
            *argv++ = *sargv++;
          }

          *argv++ = NULL;
        }
      }
  }

  /* Top level merged, recursively merge lower levels */
  for (c = (config_rec *) s->xas_list; c; c = c->next) {
    if (c->subset &&
        (c->config_type == CONF_ANON ||
         c->config_type == CONF_DIR)) {
      merge_down(c->subset, dynamic);
    }
  }
}

/* Iterate through <Directory> blocks inside of anonymous and
 * resolve each one.
 */
void resolve_anonymous_dirs(xaset_t *clist) {
  config_rec *c;
  char *realdir;

  if (!clist)
    return;

  for (c = (config_rec *) clist->xas_list; c; c = c->next) {
    if (c->config_type == CONF_DIR) {
      if (c->argv[1]) {
        realdir = dir_best_path(c->pool, c->argv[1]);
        if (realdir) {
          c->argv[1] = realdir;

        } else {
          realdir = dir_canonical_path(c->pool, c->argv[1]);
          if (realdir)
            c->argv[1] = realdir;
        }
      }

      if (c->subset)
        resolve_anonymous_dirs(c->subset);
    }
  }
}

/* Iterate through directory configuration items and resolve ~ references. */
void resolve_deferred_dirs(server_rec *s) {
  config_rec *c;

  if (!s ||
      !s->conf)
    return;

  for (c = (config_rec *) s->conf->xas_list; c; c = c->next) {
    if (c->config_type == CONF_DIR &&
        (c->flags & CF_DEFER)) {
      char *realdir;

      /* Check for any expandable variables. */
      c->name = path_subst_uservar(c->pool, &c->name);

      realdir = dir_best_path(c->pool, c->name);
      if (realdir) {
        c->name = realdir;

      } else {
        realdir = dir_canonical_path(c->pool, c->name);
        if (realdir)
          c->name = realdir;
      }
    }
  }
}

static void copy_recur(xaset_t **set, pool *p, config_rec *c,
    config_rec *new_parent) {
  config_rec *newconf;
  int argc;
  void **argv, **sargv;

  if (!*set)
    *set = xaset_create(p, NULL);

  newconf = add_config_set(set, c->name);
  newconf->config_type = c->config_type;
  newconf->flags = c->flags;
  newconf->parent = new_parent;
  newconf->argc = c->argc;

  if (c->argc) {
    newconf->argv = pcalloc(newconf->pool, (c->argc+1) * sizeof(void *));
    argv = newconf->argv;
    sargv = c->argv;
    argc = newconf->argc;

    while (argc--)
      *argv++ = *sargv++;

    if (argv)
      *argv++ = NULL;
  }

  if (c->subset)
    for (c = (config_rec *) c->subset->xas_list; c; c = c->next)
      copy_recur(&newconf->subset, p, c, newconf);
}

static void copy_global_to_all(xaset_t *set) {
  server_rec *s;
  config_rec *c;

  if (!set || !set->xas_list)
    return;

  for (c = (config_rec *) set->xas_list; c; c = c->next)
    for (s = (server_rec *) server_list->xas_list; s; s = s->next)
      copy_recur(&s->conf, s->pool, c, NULL);
}

static void fixup_globals(xaset_t *list) {
  server_rec *s = NULL, *smain = NULL;
  config_rec *c = NULL, *cnext = NULL;

  smain = (server_rec *) list->xas_list;
  for (s = smain; s; s = s->next) {
    /* Loop through each top level directive looking for a CONF_GLOBAL
     * context.
     */
    if (!s->conf ||
        !s->conf->xas_list)
      continue;

    for (c = (config_rec *) s->conf->xas_list; c; c = cnext) {
      cnext = c->next;

      if (c->config_type == CONF_GLOBAL &&
          strcmp(c->name, "<Global>") == 0) {
        /* Copy the contents of the block to all other servers
         * (including this one), then pull the block "out of play".
         */
        if (c->subset &&
            c->subset->xas_list)
          copy_global_to_all(c->subset);

        xaset_remove(s->conf, (xasetmember_t *) c);

        if (!s->conf->xas_list) {
          destroy_pool(s->conf->pool);
          s->conf = NULL;
        }
      }
    }
  }
}

void fixup_dirs(server_rec *s, int flags) {
  if (!s)
    return;

  if (!s->conf) {
    if (!(flags & CF_SILENT)) {
      pr_log_debug(DEBUG5, "%s", "");
      pr_log_debug(DEBUG5, "Config for %s:", s->ServerName);
    }

    return;
  }
 
  _reorder_dirs(s->conf, flags);

  /* Merge mergeable configuration items down. */
  merge_down(s->conf, FALSE);

  if (!(flags & CF_SILENT)) {
    pr_log_debug(DEBUG5, "%s", "");
    pr_log_debug(DEBUG5, "Config for %s:", s->ServerName);
    pr_config_dump(config_dumpf, s->conf, NULL);
  }

  return;
}

config_rec *find_config_next(config_rec *prev, config_rec *c, int type,
    const char *name, int recurse) {
  config_rec *top = c;
  unsigned int cid = 0;

  /* We do two searches (if recursing) so that we find the "deepest"
   * level first.
   */

  if (!c &&
      !prev)
    return NULL;

  if (!prev)
    prev = top;

  if (name)
    cid = pr_config_get_id(name);

  if (recurse) {
    do {
      config_rec *res = NULL;

      pr_signals_handle();

      for (c = top; c; c = c->next) {
        if (c->subset &&
            c->subset->xas_list) {
          config_rec *subc = NULL;

          for (subc = (config_rec *) c->subset->xas_list;
               subc;
               subc = subc->next) {
            res = find_config_next(NULL, subc, type, name, recurse + 1);
            if (res)
              return res;
          }
        }
      }

      /* If deep recursion yielded no match try the current subset.
       *
       * NOTE: the string comparison here is specifically case sensitive.
       * The config_rec names are supplied by the modules and intentionally
       * case sensitive (they shouldn't be verbatim from the config file)
       * Do NOT change this to strcasecmp(), no matter how tempted you are
       * to do so, it will break stuff. ;)
       */
      for (c = top; c; c = c->next) {
        if (type == -1 ||
            type == c->config_type) {

          if (!name)
            return c;

          if (cid != 0 &&
              cid == c->config_id) {
            return c;
          }

          if (strcmp(name, c->name) == 0)
            return c;
        }
      }

      /* Restart the search at the previous level if required */
      if (prev->parent &&
          recurse == 1 &&
          prev->parent->next &&
          prev->parent->set != find_config_top) {
        prev = top = prev->parent->next;
        c = top;
        continue;
      }

      break;
    } while (TRUE);

  } else {
    for (c = top; c; c = c->next) {
      if (type == -1 ||
          type == c->config_type) {

        if (!name)
          return c;

        if (cid != 0 &&
            cid == c->config_id) {
          return c;
        }

        if (strcmp(name, c->name) == 0)
          return c;
      }
    }
  }

  return NULL;
}

void find_config_set_top(config_rec *c) {
  if (c &&
      c->parent) {
    find_config_top = c->parent->set;

  } else {
    find_config_top = NULL;
  }
}


config_rec *find_config(xaset_t *set, int type, const char *name, int recurse) {
  if (!set ||
      !set->xas_list)
    return NULL;

  find_config_set_top((config_rec *) set->xas_list);

  return find_config_next(NULL, (config_rec *) set->xas_list, type, name,
    recurse);
}

void *get_param_ptr(xaset_t *set, const char *name, int recurse) {
  config_rec *c;

  if (!set) {
    _last_param_ptr = NULL;
    return NULL;
  }

  c = find_config(set, CONF_PARAM, name, recurse);

  if (c &&
      c->argc) {
    _last_param_ptr = c;
    return c->argv[0];
  }

  _last_param_ptr = NULL;
  return NULL;
}

void *get_param_ptr_next(const char *name,int recurse) {
  config_rec *c;

  if (!_last_param_ptr ||
      !_last_param_ptr->next) {
    _last_param_ptr = NULL;
    return NULL;
  }

  c = find_config_next(_last_param_ptr, _last_param_ptr->next, CONF_PARAM,
    name, recurse);

  if (c &&
      c->argv) {
    _last_param_ptr = c;
    return c->argv[0];
  }

  _last_param_ptr = NULL;
  return NULL;
}

int remove_config(xaset_t *set, const char *name, int recurse) {
  server_rec *s = pr_parser_server_ctxt_get();
  config_rec *c;
  int found = 0;
  xaset_t *fset;

  if (!s)
    s = main_server;

  while ((c = find_config(set, -1, name, recurse)) != NULL) {
    found++;

    fset = c->set;
    xaset_remove(fset, (xasetmember_t *) c);

    /* If the set is empty, and has no more contained members in the xas_list,
     * destroy the set.
     */
    if (!fset->xas_list) {

      /* First, set any pointers to the container of the set to NULL. */
      if (c->parent && c->parent->subset == fset)
        c->parent->subset = NULL;

      else if (s->conf == fset)
        s->conf = NULL;

      /* Next, destroy the set's pool, which destroys the set as well. */
      destroy_pool(fset->pool);

    } else {

      /* If the set was not empty, destroy only the requested config_rec. */
      destroy_pool(c->pool);
    }
  }

  return found;
}

config_rec *add_config_param_set(xaset_t **set, const char *name,
    int num, ...) {
  config_rec *c = add_config_set(set, name);
  void **argv;
  va_list ap;

  if (c) {
    c->config_type = CONF_PARAM;
    c->argc = num;
    c->argv = pcalloc(c->pool, (num+1) * sizeof(void *));

    argv = c->argv;
    va_start(ap,num);

    while (num-- > 0)
      *argv++ = va_arg(ap, void *);

    va_end(ap);
  }

  return c;
}

config_rec *add_config_param_str(const char *name, int num, ...) {
  config_rec *c = add_config(NULL, name);
  char *arg = NULL;
  void **argv = NULL;
  va_list ap;

  if (c) {
    c->config_type = CONF_PARAM;
    c->argc = num;
    c->argv = pcalloc(c->pool, (num+1) * sizeof(char *));

    argv = c->argv;
    va_start(ap, num);

    while (num-- > 0) {
      arg = va_arg(ap, char *);
      if (arg)
        *argv++ = pstrdup(c->pool, arg);
      else
        *argv++ = NULL;
    }

    va_end(ap);
  }

  return c;
}

config_rec *pr_conf_add_server_config_param_str(server_rec *s, const char *name,
    int num, ...) {
  config_rec *c = add_config(s, name);
  char *arg = NULL;
  void **argv = NULL;
  va_list ap;

  if (c) {
    c->config_type = CONF_PARAM;
    c->argc = num;
    c->argv = pcalloc(c->pool, (num+1) * sizeof(char *));

    argv = c->argv;
    va_start(ap, num);

    while (num-- > 0) {
      arg = va_arg(ap, char *);
      if (arg)
        *argv++ = pstrdup(c->pool, arg);
      else
        *argv++ = NULL;
    }

    va_end(ap);
  }

  return c;
}

config_rec *add_config_param(const char *name, int num, ...) {
  config_rec *c;
  void **argv;
  va_list ap;

  if (name == NULL) {
    errno = EINVAL;
    return NULL;
  }

  c = add_config(NULL, name);
  if (c) {
    c->config_type = CONF_PARAM;
    c->argc = num;
    c->argv = pcalloc(c->pool, (num+1) * sizeof(void*));

    argv = c->argv;
    va_start(ap, num);

    while (num-- > 0)
      *argv++ = va_arg(ap, void *);

    va_end(ap);
  }

  return c;
}

static int config_cmp(const void *a, const void *b) {
  return strcmp(*((char **) a), *((char **) b));
}

int parse_config_path(pool *p, const char *path) {
  struct stat st;
  int have_glob;
  
  if (!path) {
    errno = EINVAL;
    return -1;
  }

  have_glob = is_fnmatch(path); 

  if (!have_glob && pr_fsio_lstat(path, &st) < 0)
    return -1;

  if (have_glob ||
      (!S_ISLNK(st.st_mode) && S_ISDIR(st.st_mode))) {
    void *dirh;
    struct dirent *dent;
    array_header *file_list;
    char *dup_path = pstrdup(p, path);
    char *tmp = strrchr(dup_path, '/');

    if (have_glob && tmp) {
      *tmp++ = '\0';

      if (is_fnmatch(dup_path)) {
        pr_log_pri(PR_LOG_ERR, "error: wildcard patterns not allowed in "
          "configuration directory name '%s'", dup_path);
        errno = EINVAL;
        return -1;
      }

      /* Check the directory component. */
      pr_fsio_lstat(dup_path, &st);

      if (S_ISLNK(st.st_mode) || !S_ISDIR(st.st_mode)) {
        pr_log_pri(PR_LOG_ERR, "error: cannot read configuration path '%s'",
          dup_path);
        errno = EINVAL;
        return -1;
      }

      if (!is_fnmatch(tmp)) {
        pr_log_pri(PR_LOG_ERR, "error: wildcard pattern required for file '%s'",
          tmp);
        errno = EINVAL;
        return -1;
      }
    }

    pr_log_pri(PR_LOG_INFO, "processing configuration directory '%s'",
      dup_path);

    dirh = pr_fsio_opendir(dup_path);
    if (!dirh) {
      pr_log_pri(PR_LOG_ERR,
        "error: unable to open configuration directory '%s': %s", dup_path,
        strerror(errno));
      errno = EINVAL;
      return -1;
    }

    file_list = make_array(p, 0, sizeof(char *));

    while ((dent = pr_fsio_readdir(dirh)) != NULL) {
      if (strcmp(dent->d_name, ".") != 0 &&
          strcmp(dent->d_name, "..") != 0 &&
          (!have_glob ||
           pr_fnmatch(tmp, dent->d_name, PR_FNM_PERIOD) == 0))
        *((char **) push_array(file_list)) = pdircat(p, dup_path,
          dent->d_name, NULL);
    }

    pr_fsio_closedir(dirh);

    if (file_list->nelts) {
      register unsigned int i;

      qsort((void *) file_list->elts, file_list->nelts, sizeof(char *),
        config_cmp);

      for (i = 0; i < file_list->nelts; i++) {
        char *file = ((char **) file_list->elts)[i];
        pr_parser_parse_file(p, file, NULL, 0);
      }
    }

    return 0;
  }

  return pr_parser_parse_file(p, path, NULL, 0);
}

/* Go through each server configuration and complain if important information
 * is missing (post reading configuration files).  Otherwise, fill in defaults
 * where applicable.
 */
int fixup_servers(xaset_t *list) {
  config_rec *c = NULL;
  server_rec *s = NULL, *next_s = NULL;

  fixup_globals(list);

  s = (server_rec *) list->xas_list;
  if (s && !s->ServerName)
    s->ServerName = pstrdup(s->pool, "ProFTPD");

  for (; s; s = next_s) {
    unsigned char *default_server = NULL;

    next_s = s->next;
    if (!s->ServerAddress) {
      array_header *addrs = NULL;

      s->ServerAddress = pr_netaddr_get_localaddr_str(s->pool);

      s->addr = pr_netaddr_get_addr(s->pool, s->ServerAddress, &addrs);
     
      if (addrs) {
        register unsigned int i;
        pr_netaddr_t **elts = addrs->elts;

        /* For every additional address, implicitly add a bind record. */
        for (i = 0; i < addrs->nelts; i++) {
          const char *ipstr = pr_netaddr_get_ipstr(elts[i]);

#ifdef PR_USE_IPV6
          if (pr_netaddr_use_ipv6()) {
            char *ipbuf = pcalloc(s->pool, INET6_ADDRSTRLEN + 1);
            if (pr_netaddr_get_family(elts[i]) == AF_INET) {

              /* Create the bind record using the IPv4-mapped IPv6 version of
               * this address.
               */
              snprintf(ipbuf, INET6_ADDRSTRLEN, "::ffff:%s", ipstr);
              ipstr = pstrdup(s->pool, ipbuf);
            }
          }
#endif /* PR_USE_IPV6 */

          if (ipstr)
            pr_conf_add_server_config_param_str(s, "_bind", 1, ipstr);
        }
      }
 
    } else 
      s->addr = pr_netaddr_get_addr(s->pool, s->ServerAddress, NULL);

    if (s->addr == NULL) {
      pr_log_pri(PR_LOG_WARNING,
        "warning: unable to determine IP address of '%s'", s->ServerAddress);

      if (s == main_server) {
        main_server = NULL;
      }

      xaset_remove(list, (xasetmember_t *) s);
      destroy_pool(s->pool);
      continue;
    }

    s->ServerFQDN = pr_netaddr_get_dnsstr(s->addr);

    if (!s->ServerFQDN)
      s->ServerFQDN = s->ServerAddress;

    if (!s->ServerAdmin)
      s->ServerAdmin = pstrcat(s->pool, "root@", s->ServerFQDN, NULL);

    if (!s->ServerName) {
      server_rec *m = (server_rec *) list->xas_list;
      s->ServerName = pstrdup(s->pool, m->ServerName);
    }

    if (!s->tcp_rcvbuf_len)
      s->tcp_rcvbuf_len = tcp_rcvbufsz;

    if (!s->tcp_sndbuf_len)
      s->tcp_sndbuf_len = tcp_sndbufsz;

    c = find_config(s->conf, CONF_PARAM, "MasqueradeAddress", FALSE);
    if (c != NULL) {
      pr_log_pri(PR_LOG_INFO, "%s:%d masquerading as %s",
        pr_netaddr_get_ipstr(s->addr), s->ServerPort,
        pr_netaddr_get_ipstr((pr_netaddr_t *) c->argv[0]));
    }

    /* Honor the DefaultServer directive only if SocketBindTight is not
     * in effect.
     */
    default_server = get_param_ptr(s->conf, "DefaultServer", FALSE);

    if (default_server &&
        *default_server == TRUE) {

      if (!SocketBindTight) {
        pr_netaddr_set_sockaddr_any(s->addr);

      } else {
        pr_log_pri(PR_LOG_NOTICE,
          "SocketBindTight in effect, ignoring DefaultServer");
      }
    }

    fixup_dirs(s, 0);
  }

  /* Make sure there actually are server_recs remaining in the list
   * before continuing.  Badly configured/resolved vhosts are rejected, and
   * it's possible to have all vhosts (even the default) rejected.
   */
  if (list->xas_list == NULL) {
    pr_log_pri(PR_LOG_NOTICE, "error: no valid servers configured");
    return -1;
  }

  pr_inet_clear();
  return 0;
}

static void set_tcp_bufsz(void) {
  int sockfd;
  socklen_t optlen = 0;
  struct protoent *p = NULL;

#ifdef HAVE_SETPROTOENT
  setprotoent(FALSE);
#endif

  p = getprotobyname("tcp");
  if (!p) {
#ifndef PR_TUNABLE_RCVBUFSZ
    tcp_rcvbufsz = PR_TUNABLE_DEFAULT_RCVBUFSZ;
#else
    tcp_rcvbufsz = PR_TUNABLE_RCVBUFSZ;
#endif /* PR_TUNABLE_RCVBUFSZ */

#ifndef PR_TUNABLE_SNDBUFSZ
    tcp_sndbufsz = PR_TUNABLE_DEFAULT_SNDBUFSZ;
#else
    tcp_sndbufsz = PR_TUNABLE_SNDBUFSZ;
#endif /* PR_TUNABLE_SNDBUFSZ */

    pr_log_debug(DEBUG3, "getprotobyname error for 'tcp': %s", strerror(errno));
    pr_log_debug(DEBUG4, "using default TCP receive/send buffer sizes");

#ifndef PR_TUNABLE_XFER_BUFFER_SIZE
  /* Choose the smaller of the two TCP buffer sizes as the overall transfer
   * size (for use by the data transfer layer).
   */
   xfer_bufsz = tcp_sndbufsz < tcp_rcvbufsz ? tcp_sndbufsz : tcp_rcvbufsz;
#else
  xfer_bufsz = PR_TUNABLE_XFER_BUFFER_SIZE;
#endif /* PR_TUNABLE_XFER_BUFFER_SIZE */

    return;
  }

  sockfd = socket(AF_INET, SOCK_STREAM, p->p_proto);
  if (sockfd < 0) {
    tcp_rcvbufsz = PR_TUNABLE_DEFAULT_RCVBUFSZ;
    tcp_sndbufsz = PR_TUNABLE_DEFAULT_SNDBUFSZ;

    pr_log_debug(DEBUG3, "socket error: %s", strerror(errno));
    pr_log_debug(DEBUG4, "using default TCP receive/send buffer sizes");
  }

#ifndef PR_TUNABLE_RCVBUFSZ
  /* Determine the optimal size of the TCP receive buffer. */
  optlen = sizeof(tcp_rcvbufsz);
  if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void *) &tcp_rcvbufsz,
      &optlen) < 0) {
    tcp_rcvbufsz = PR_TUNABLE_DEFAULT_RCVBUFSZ;

    pr_log_debug(DEBUG3, "getsockopt error for SO_RCVBUF: %s", strerror(errno));
    pr_log_debug(DEBUG4, "using default TCP receive buffer size of %d bytes",
      tcp_rcvbufsz);

  } else {
    pr_log_debug(DEBUG5, "using TCP receive buffer size of %d bytes",
      tcp_rcvbufsz);
  }
#else
  optlen = -1;
  tcp_sndbufsz = PR_TUNABLE_RCVBUFSZ;
  pr_log_debug(DEBUG5, "using preset TCP receive buffer size of %d bytes",
    tcp_rcvbufsz);
#endif /* PR_TUNABLE_RCVBUFSZ */

#ifndef PR_TUNABLE_SNDBUFSZ
  /* Determine the optimal size of the TCP send buffer. */
  optlen = sizeof(tcp_sndbufsz);
  if (getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void *) &tcp_sndbufsz,
      &optlen) < 0) {
    tcp_sndbufsz = PR_TUNABLE_DEFAULT_SNDBUFSZ;
    
    pr_log_debug(DEBUG3, "getsockopt error for SO_SNDBUF: %s", strerror(errno));
    pr_log_debug(DEBUG4, "using default TCP send buffer size of %d bytes",
      tcp_sndbufsz);
  
  } else {
    pr_log_debug(DEBUG5, "using TCP send buffer size of %d bytes",
      tcp_sndbufsz);
  }
#else
  optlen = -1;
  tcp_sndbufsz = PR_TUNABLE_SNDBUFSZ;
  pr_log_debug(DEBUG5, "using preset TCP send buffer size of %d bytes",
    tcp_sndbufsz);
#endif /* PR_TUNABLE_SNDBUFSZ */

  /* Choose the smaller of the two TCP buffer sizes as the overall transfer
   * size (for use by the data transfer layer).
   */
   xfer_bufsz = tcp_sndbufsz < tcp_rcvbufsz ? tcp_sndbufsz : tcp_rcvbufsz;

  (void) close(sockfd);
}

void init_config(void) {
  pool *conf_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(conf_pool, "Config Pool");

  /* Make sure global_config_pool is destroyed */
  if (global_config_pool) {
    destroy_pool(global_config_pool);
    global_config_pool = NULL;
  }

  if (config_tab) {
    /* Clear the existing config ID table.  This needs to happen when proftpd
     * is restarting.
     */
    if (pr_table_empty(config_tab) < 0) {
      pr_log_debug(DEBUG0, "error emptying config ID table: %s",
        strerror(errno));
    }

    if (pr_table_free(config_tab) < 0) {
      pr_log_debug(DEBUG0, "error destroying config ID table: %s",
        strerror(errno));
    }

    config_tab = pr_table_alloc(config_tab_pool, 0);

    /* Reset the ID counter as well.  Otherwise, an exceedingly long-lived
     * proftpd, restarted many times, has the possibility of overflowing
     * the counter data type.
     */
    config_id = 0;

  } else {

    config_tab_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(config_tab_pool, "Config ID Table Pool");
    config_tab = pr_table_alloc(config_tab_pool, 0);
  }

  if (server_list) {
    server_rec *s, *s_next;

    /* Free the old configuration completely */
    for (s = (server_rec *) server_list->xas_list; s; s = s_next) {
      s_next = s->next;
      destroy_pool(s->pool);
    }

    destroy_pool(server_list->pool);
    server_list = NULL;
  }

  /* Note: xaset_create() assigns the given pool to the 'pool' member
   * of the created list, i.e. server_list->pool == conf_pool.  Hence
   * why we create yet another subpool, reusing the conf_pool pointer.
   * The pool creation below is not redundant.
   */
  server_list = xaset_create(conf_pool, NULL);

  conf_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(conf_pool, "main_server pool");

  main_server = (server_rec *) pcalloc(conf_pool, sizeof(server_rec));
  xaset_insert(server_list, (xasetmember_t *) main_server);

  main_server->pool = conf_pool;
  main_server->set = server_list;
  main_server->sid = 1;

  /* Default server port */
  main_server->ServerPort = pr_inet_getservport(main_server->pool,
    "ftp", "tcp");

  set_tcp_bufsz();
  return;
}

/* These functions are used by modules to help parse configuration.
 */

unsigned char check_context(cmd_rec *cmd, int allowed) {
  int ctxt = (cmd->config && cmd->config->config_type != CONF_PARAM ?
     cmd->config->config_type : cmd->server->config_type ?
     cmd->server->config_type : CONF_ROOT);

  if (ctxt & allowed)
    return TRUE;

  /* default */
  return FALSE;
}

char *get_context_name(cmd_rec *cmd) {
  static char cbuf[20];

  if (!cmd->config || cmd->config->config_type == CONF_PARAM) {
    if (cmd->server->config_type == CONF_VIRTUAL)
      return "<VirtualHost>";
    else
      return "server config";
  }

  switch (cmd->config->config_type) {
    case CONF_DIR:
      return "<Directory>";

    case CONF_ANON:
      return "<Anonymous>";

    case CONF_CLASS:
      return "<Class>";

    case CONF_LIMIT:
      return "<Limit>";

    case CONF_DYNDIR:
      return ".ftpaccess";

    case CONF_GLOBAL:
      return "<Global>";

    case CONF_USERDATA:
      return "user data";

    default:
      /* XXX should dispatch to modules here, to allow them to create and
       * handle their own arbitrary configuration contexts.
       */
      memset(cbuf, '\0', sizeof(cbuf));
      snprintf(cbuf, sizeof(cbuf), "%d", cmd->config->config_type);
      return cbuf;
  }
}

int get_boolean(cmd_rec *cmd, int av) {
  char *cp = cmd->argv[av];

  return pr_str_is_boolean(cp);
}

char *get_full_cmd(cmd_rec *cmd) {
  pool *p = cmd->tmp_pool;
  char *res = "";

  if (cmd->arg && *cmd->arg)
    res = pstrcat(p, cmd->argv[0], " ", pr_fs_decode_path(p, cmd->arg), NULL);

  else if (cmd->argc > 1) {
    register unsigned int i = 0;
    res = cmd->argv[0];

    for (i = 1; i < cmd->argc; i++)
      res = pstrcat(p, res, pr_fs_decode_path(p, cmd->argv[i]), " ", NULL);

    while (res[strlen(res)-1] == ' ' && *res)
      res[strlen(res)-1] = '\0';

  } else
    res = pstrdup(p, cmd->argv[0]);

  return res;
}

unsigned int pr_config_get_id(const char *name) {
  void *ptr = NULL;
  unsigned int id = 0;

  if (!name) {
    errno = EINVAL;
    return 0;
  }

  if (!config_tab) {
    errno = EPERM;
    return 0;
  }

  ptr = pr_table_get(config_tab, name, NULL);
  if (ptr == NULL) {
    errno = ENOENT;
    return 0;
  }

  id = *((unsigned int *) ptr);
  return id;
}

unsigned int pr_config_set_id(const char *name) {
  unsigned int *ptr = NULL;
  unsigned int id;

  if (!name) {
    errno = EINVAL;
    return 0;
  }

  if (!config_tab) {
    errno = EPERM;
    return 0;
  }

  ptr = pr_table_pcalloc(config_tab, sizeof(unsigned int));
  *ptr = ++config_id;

  if (pr_table_add(config_tab, name, ptr, sizeof(unsigned int *)) < 0) {
    if (errno == EEXIST) {
      id = pr_config_get_id(name);

    } else {
      pr_log_debug(DEBUG0, "error adding '%s' to config ID table: %s",
        name, strerror(errno));
      return 0;
    }

  } else {
    id = *ptr;
  }

  return id;
}

int pr_config_get_xfer_bufsz(void) {
  return xfer_bufsz;
}
