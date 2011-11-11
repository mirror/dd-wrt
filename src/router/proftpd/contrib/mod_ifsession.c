/*
 * ProFTPD: mod_ifsession -- a module supporting conditional
 *                            per-user/group/class configuration contexts.
 *
 * Copyright (c) 2002-2011 TJ Saunders
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
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * This is mod_ifsession, contrib software for proftpd 1.2 and above.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 *
 * $Id: mod_ifsession.c,v 1.37 2011/05/23 20:56:40 castaglia Exp $
 */

#include "conf.h"

#define MOD_IFSESSION_VERSION	"mod_ifsession/1.1"

/* Make sure the version of proftpd is as necessary. */
#if PROFTPD_VERSION_NUMBER < 0x0001030402
# error "ProFTPD 1.3.4rc2 or later required"
#endif

#define IFSESS_CLASS_NUMBER	100
#define IFSESS_CLASS_TEXT	"<IfClass>"
#define IFSESS_GROUP_NUMBER	101
#define	IFSESS_GROUP_TEXT	"<IfGroup>"
#define IFSESS_USER_NUMBER	102
#define	IFSESS_USER_TEXT	"<IfUser>"

module ifsession_module;

static int ifsess_ctx = -1;
static int ifsess_merged = FALSE;

static const char *trace_channel = "ifsession";

/* Support routines
 */

static void ifsess_remove_param(xaset_t *set, const char *name) {
  config_rec *c = NULL;

  pr_trace_msg(trace_channel, 9, "removing '%s' config", name);

  c = find_config(set, -1, name, TRUE);
  while (c != NULL) {
    xaset_t *fset;

    pr_signals_handle();

    fset = c->set;
    xaset_remove(fset, (xasetmember_t *) c);

    c = find_config(set, -1, name, TRUE);
  }
}

static void ifsess_dup_param(pool *dst_pool, xaset_t **dst, config_rec *c,
    config_rec *parent) {
  config_rec *dup_c = NULL;

  pr_trace_msg(trace_channel, 9, "adding '%s' config", c->name);

  if (!*dst)
    *dst = xaset_create(dst_pool, NULL);

  dup_c = add_config_set(dst, c->name);
  dup_c->config_type = c->config_type;
  dup_c->flags = c->flags;
  dup_c->parent = parent;
  dup_c->argc = c->argc;

  if (c->argc) {
    void **dst_argv = NULL, **src_argv = NULL;
    int dst_argc;

    dup_c->argv = pcalloc(dup_c->pool, (c->argc + 1) * sizeof(void *));

    src_argv = c->argv;
    dst_argv = dup_c->argv;
    dst_argc = dup_c->argc;

    while (dst_argc--)
      *dst_argv++ = *src_argv++;

    if (dst_argv)
      *dst_argv++ = NULL;
  }

  if (c->subset) {
    for (c = (config_rec *) c->subset->xas_list; c; c = c->next) {

      /* If this directive does not allow multiple instances, make sure
       * it is removed from the destination set first.  The "source"
       * directive then effectively replaces any directive there.
       */
      if (c->config_type == CONF_PARAM &&
          !(c->flags & CF_MERGEDOWN_MULTI) &&
          !(c->flags & CF_MULTI)) {
          pr_trace_msg(trace_channel, 15, "removing '%s' config because "
            "c->flags does not contain MULTI or MERGEDOWN_MULTI", c->name);
        ifsess_remove_param(dup_c->subset, c->name);
      }

      ifsess_dup_param(dst_pool, &dup_c->subset, c, dup_c);
    }
  }
}

static void ifsess_dup_set(pool *dst_pool, xaset_t *dst, xaset_t *src) {
  config_rec *c, *next;

  for (c = (config_rec *) src->xas_list; c; c = next) {
    next = c->next;

    /* Skip the context lists. */
    if (c->config_type == IFSESS_CLASS_NUMBER ||
        c->config_type == IFSESS_GROUP_NUMBER ||
        c->config_type == IFSESS_USER_NUMBER)
      continue;

    /* If this directive does not allow multiple instances, make sure
     * it is removed from the destination set first.  The "source"
     * directive then effectively replaces any directive there.
     */
    if (c->config_type == CONF_PARAM &&
        !(c->flags & CF_MERGEDOWN_MULTI)) {
        pr_trace_msg(trace_channel, 15, "removing '%s' config because "
          "c->flags does not contain MERGEDOWN_MULTI", c->name);
      ifsess_remove_param(dst, c->name);
    }

    ifsess_dup_param(dst_pool, &dst, c, NULL);
  }
}

/* Configuration handlers
 */

MODRET start_ifctxt(cmd_rec *cmd) {
  config_rec *c = NULL;
  int config_type = 0, eval_type = 0;
  int argc = 0;
  char *name = NULL;
  char **argv = NULL;
  array_header *acl = NULL;

  if (cmd->argc-1 < 1)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = pr_parser_config_ctxt_open(cmd->argv[0]);

  /* "Inherit" the parent's context type. */
  c->config_type = (cmd->config && cmd->config->config_type != CONF_PARAM ?
    cmd->config->config_type : cmd->server->config_type ?
    cmd->server->config_type : CONF_ROOT);

  if (strcmp(cmd->argv[0], IFSESS_CLASS_TEXT) == 0) {
    name = "_IfClassList";
    ifsess_ctx = config_type = IFSESS_CLASS_NUMBER;
    eval_type = PR_EXPR_EVAL_OR;

  } else if (strcmp(cmd->argv[0], IFSESS_GROUP_TEXT) == 0) {
    name = "_IfGroupList";
    ifsess_ctx = config_type = IFSESS_GROUP_NUMBER;
    eval_type = PR_EXPR_EVAL_AND;

  } else if (strcmp(cmd->argv[0], IFSESS_USER_TEXT) == 0) {
    name = "_IfUserList";
    ifsess_ctx = config_type = IFSESS_USER_NUMBER;
    eval_type = PR_EXPR_EVAL_OR;
  }

  /* Is this a normal expression, an explicit AND, an explicit OR, or a
   * regular expression?
   */
  if (cmd->argc-1 > 1) {
    if (strncmp(cmd->argv[1], "AND", 4) == 0) {
      eval_type = PR_EXPR_EVAL_AND;
      argc = cmd->argc-2;
      argv = cmd->argv+1;

    } else if (strncmp(cmd->argv[1], "OR", 3) == 0) {
      eval_type = PR_EXPR_EVAL_OR;
      argc = cmd->argc-2;
      argv = cmd->argv+1;

    } else if (strncmp(cmd->argv[1], "regex", 6) == 0) {
#ifdef PR_USE_REGEX
      pr_regex_t *pre = NULL;
      int res = 0;

      if (cmd->argc != 3)
        CONF_ERROR(cmd, "wrong number of parameters");

      pre = pr_regexp_alloc(&ifsession_module);

      res = pr_regexp_compile(pre, cmd->argv[2], REG_EXTENDED|REG_NOSUB);
      if (res != 0) {
        char errstr[200] = {'\0'};

        pr_regexp_error(res, pre, errstr, sizeof(errstr));
        pr_regexp_free(NULL, pre);

        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": '", cmd->argv[2], "' failed "
          "regex compilation: ", errstr, NULL));
      }

      eval_type = PR_EXPR_EVAL_REGEX;

      c = add_config_param(name, 3, NULL, NULL, NULL);
      c->config_type = config_type;
      c->argv[0] = pstrdup(c->pool, cmd->arg);
      c->argv[1] = pcalloc(c->pool, sizeof(unsigned char));
      *((unsigned char *) c->argv[1]) = eval_type;
      c->argv[2] = (void *) pre;

      return PR_HANDLED(cmd);

#else
      CONF_ERROR(cmd, "The 'regex' parameter cannot be used on this system, "
        "as you do not have POSIX compliant regex support");
#endif /* regex support */

    } else {
      argc = cmd->argc-1;
      argv = cmd->argv;
    }

  } else {
    argc = cmd->argc-1;
    argv = cmd->argv;
  }

  acl = pr_expr_create(cmd->tmp_pool, &argc, argv);

  c = add_config_param(name, 0);

  c->config_type = config_type;
  c->argc = acl->nelts + 2;
  c->argv = pcalloc(c->pool, (c->argc + 2) * sizeof(char *));
  c->argv[0] = pstrdup(c->pool, cmd->arg);
  c->argv[1] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[1]) = eval_type;

  argv = (char **) c->argv + 2;

  if (acl) {
    while (acl->nelts--) {
      *argv++ = pstrdup(c->pool, *((char **) acl->elts));
      acl->elts = ((char **) acl->elts) + 1;
    }
  }

  *argv = NULL;
  return PR_HANDLED(cmd);
}

MODRET end_ifctxt(cmd_rec *cmd) {
  pr_parser_config_ctxt_close(NULL);

  switch (ifsess_ctx) {
    case IFSESS_CLASS_NUMBER:
      if (strcasecmp("</IfClass>", cmd->argv[0]) == 0) {
        ifsess_ctx = -1;
      }
      break;

    case IFSESS_GROUP_NUMBER:
      if (strcasecmp("</IfGroup>", cmd->argv[0]) == 0) {
        ifsess_ctx = -1;
      }
      break;

    case IFSESS_USER_NUMBER:
      if (strcasecmp("</IfUser>", cmd->argv[0]) == 0) {
        ifsess_ctx = -1;
      }
      break;
  }

  return PR_HANDLED(cmd);
}

/* Command handlers
 */

MODRET ifsess_post_pass(cmd_rec *cmd) {
  register unsigned int i = 0;
  config_rec *c = NULL;
  int found = 0;
  pool *tmp_pool = make_sub_pool(session.pool);
  array_header *group_remove_list = make_array(tmp_pool, 1,
    sizeof(config_rec *));
  array_header *user_remove_list = make_array(tmp_pool, 1,
    sizeof(config_rec *));

  /* Unfortunately, I can't assign my own context types for these custom
   * contexts, otherwise the existing directives would not be allowed in
   * them.  Good to know for the future, though, when developing modules that
   * want to have their own complete contexts (e.g. mod_time-3.0).
   *
   * However, I _can_ add a directive config_rec to these contexts that has
   * its own custom config_type.  And by using -1 as the context type when
   * searching via find_config(), it will match any context as long as the
   * name also matches.  Note: using a type of -1 and a name of NULL will
   * result in a scan of the whole in-memory db.  Hmm...
   */

  c = find_config(main_server->conf, -1, IFSESS_GROUP_TEXT, FALSE);

  while (c) {
    config_rec *list = NULL;

    pr_signals_handle();

    list = find_config(c->subset, IFSESS_GROUP_NUMBER, NULL, FALSE);
    if (list != NULL) {
      unsigned char mergein = FALSE;

#ifdef PR_USE_REGEX
      if (*((unsigned char *) list->argv[1]) == PR_EXPR_EVAL_REGEX) {
        pr_regex_t *pre = list->argv[2];

        if (session.group != NULL) {
          pr_log_debug(DEBUG8, MOD_IFSESSION_VERSION
            ": evaluating regexp pattern '%s' against subject '%s'",
            pr_regexp_get_pattern(pre), session.group);

          if (pr_regexp_exec(pre, session.group, 0, NULL, 0, 0, 0) == 0) {
            mergein = TRUE;
          }
        }

        if (mergein == FALSE &&
            session.groups != NULL) {
          register int j = 0;

          for (j = session.groups->nelts-1; j >= 0; j--) {
            char *suppl_group;

            suppl_group = *(((char **) session.groups->elts) + j);

            pr_log_debug(DEBUG8, MOD_IFSESSION_VERSION
              ": evaluating regexp pattern '%s' against subject '%s'",
              pr_regexp_get_pattern(pre), suppl_group);

            if (pr_regexp_exec(pre, suppl_group, 0, NULL, 0, 0, 0) == 0) {
              mergein = TRUE;
              break;
            }
          }
        }

      } else
#endif /* regex support */
    
      if (*((unsigned char *) list->argv[1]) == PR_EXPR_EVAL_OR &&
          pr_expr_eval_group_or((char **) &list->argv[2]) == TRUE) {
        mergein = TRUE;

      } else if (*((unsigned char *) list->argv[1]) == PR_EXPR_EVAL_AND &&
          pr_expr_eval_group_and((char **) &list->argv[2]) == TRUE) {
        mergein = TRUE;
      }

      if (mergein) {
        pr_log_debug(DEBUG2, MOD_IFSESSION_VERSION
          ": merging <IfGroup %s> directives in", (char *) list->argv[0]);
        ifsess_dup_set(session.pool, main_server->conf, c->subset);

        /* Add this config_rec pointer to the list of pointers to be
         * removed later.
         */
        *((config_rec **) push_array(group_remove_list)) = c;

        resolve_deferred_dirs(main_server);

        /* We need to call fixup_dirs() twice: once for any added <Directory>
         * sections that use absolute paths, and again for any added <Directory>
         * sections that use deferred-resolution paths (e.g. "~").
         */
        fixup_dirs(main_server, 0);
        fixup_dirs(main_server, CF_DEFER);

        ifsess_merged = TRUE;

      } else {
        pr_log_debug(DEBUG9, MOD_IFSESSION_VERSION
          ": <IfGroup %s> not matched, skipping", (char *) list->argv[0]);
      }
    }

    /* Note: it would be more efficient, memory-wise, to destroy the
     * memory pool of the removed config_rec.  However, the dup'd data
     * from that config_rec may point to memory within the pool being
     * freed; and once freed, that memory becomes fair game, and thus may
     * (and probably will) be overwritten.  This means that, for now,
     * keep the removed config_rec's memory around, rather than calling
     * destroy_pool(c->pool) if removed_c is TRUE.
     */

    c = find_config_next(c, c->next, -1, IFSESS_GROUP_TEXT, FALSE);
  }

  /* Now, remove any <IfGroup> config_recs that have been merged in. */
  for (i = 0; i < group_remove_list->nelts; i++) {
    c = ((config_rec **) group_remove_list->elts)[i];
    xaset_remove(main_server->conf, (xasetmember_t *) c);
  }

  c = find_config(main_server->conf, -1, IFSESS_USER_TEXT, FALSE);

  while (c) {
    config_rec *list = NULL;

    pr_signals_handle();

    list = find_config(c->subset, IFSESS_USER_NUMBER, NULL, FALSE);
    if (list != NULL) {
      unsigned char mergein = FALSE;

#ifdef PR_USE_REGEX
      if (*((unsigned char *) list->argv[1]) == PR_EXPR_EVAL_REGEX) {
        pr_regex_t *pre = list->argv[2];

        pr_log_debug(DEBUG8, MOD_IFSESSION_VERSION
          ": evaluating regexp pattern '%s' against subject '%s'",
          pr_regexp_get_pattern(pre), session.user);

        if (pr_regexp_exec(pre, session.user, 0, NULL, 0, 0, 0) == 0) {
          mergein = TRUE;
        }

      } else
#endif /* regex support */

      if (*((unsigned char *) list->argv[1]) == PR_EXPR_EVAL_OR &&
          pr_expr_eval_user_or((char **) &list->argv[2]) == TRUE) {
        mergein = TRUE;

      } else if (*((unsigned char *) list->argv[1]) == PR_EXPR_EVAL_AND &&
          pr_expr_eval_user_and((char **) &list->argv[2]) == TRUE) {
        mergein = TRUE;
      }

      if (mergein) {
        pr_log_debug(DEBUG2, MOD_IFSESSION_VERSION
          ": merging <IfUser %s> directives in", (char *) list->argv[0]);
        ifsess_dup_set(session.pool, main_server->conf, c->subset);

        /* Add this config_rec pointer to the list of pointers to be
         * removed later.
         */
        *((config_rec **) push_array(user_remove_list)) = c;

        resolve_deferred_dirs(main_server);

        /* We need to call fixup_dirs() twice: once for any added <Directory>
         * sections that use absolute paths, and again for any added <Directory>
         * sections that use deferred-resolution paths (e.g. "~").
         */
        fixup_dirs(main_server, 0);
        fixup_dirs(main_server, CF_DEFER);

        ifsess_merged = TRUE;

      } else {
        pr_log_debug(DEBUG9, MOD_IFSESSION_VERSION
          ": <IfUser %s> not matched, skipping", (char *) list->argv[0]);
      }
    }

    c = find_config_next(c, c->next, -1, IFSESS_USER_TEXT, FALSE);
  }

  /* Now, remove any <IfUser> config_recs that have been merged in. */
  for (i = 0; i < user_remove_list->nelts; i++) {
    c = ((config_rec **) user_remove_list->elts)[i];
    xaset_remove(main_server->conf, (xasetmember_t *) c);
  }

  destroy_pool(tmp_pool);

  if (ifsess_merged) {
    /* Try to honor any <Limit LOGIN> sections that may have been merged in. */
    if (!login_check_limits(TOPLEVEL_CONF, FALSE, TRUE, &found)) {
      pr_log_debug(DEBUG3, MOD_IFSESSION_VERSION
        ": %s %s: Limit access denies login",
        session.anon_config ? "ANON" : C_USER, session.user);

      pr_log_auth(PR_LOG_NOTICE, "%s %s: Limit access denies login.",
        session.anon_config ? "ANON" : C_USER, session.user);
      pr_session_disconnect(&ifsession_module, PR_SESS_DISCONNECT_CONFIG_ACL,
        "Denied by <Limit LOGIN>");
    }
  }

  return PR_DECLINED(cmd);
}

/* Event handlers
 */

#ifdef PR_SHARED_MODULE
static void ifsess_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp("mod_ifsession.c", (const char *) event_data) == 0) {
    pr_event_unregister(&ifsession_module, NULL, NULL);
  }
}
#endif /* PR_SHARED_MODULE */

static void ifsess_postparse_ev(const void *event_data, void *user_data) {
  /* Make sure that all mod_ifsession sections have been properly closed. */

  if (ifsess_ctx == -1) {
    /* All sections properly closed; nothing to do. */
    return;
  }

  switch (ifsess_ctx) {
    case IFSESS_CLASS_NUMBER:
      pr_log_pri(PR_LOG_ERR,
        "error: unclosed <IfClass> context in config file");
      break;

    case IFSESS_GROUP_NUMBER:
      pr_log_pri(PR_LOG_ERR,
        "error: unclosed <IfGroup> context in config file");
      break;

    case IFSESS_USER_NUMBER:
      pr_log_pri(PR_LOG_ERR,
        "error: unclosed <IfUser> context in config file");
      break;
  }

  pr_session_disconnect(&ifsession_module, PR_SESS_DISCONNECT_BAD_CONFIG, NULL);
  return;
}

/* Initialization routines
 */

static int ifsess_init(void) {
#ifdef PR_SHARED_MODULE
  pr_event_register(&ifsession_module, "core.module-unload",
    ifsess_mod_unload_ev, NULL);
#endif /* PR_SHARED_MODULE */

  pr_event_register(&ifsession_module, "core.postparse",
    ifsess_postparse_ev, NULL);

  return 0;
}

static int ifsess_sess_init(void) {
  register unsigned int i = 0;
  config_rec *c = NULL;
  pool *tmp_pool = make_sub_pool(session.pool);
  array_header *class_remove_list = make_array(tmp_pool, 1,
    sizeof(config_rec *));

  c = find_config(main_server->conf, -1, IFSESS_CLASS_TEXT, FALSE);

  while (c) {
    config_rec *list = NULL;

    pr_signals_handle();

    list = find_config(c->subset, IFSESS_CLASS_NUMBER, NULL, FALSE);
    if (list != NULL) {
      unsigned char mergein = FALSE;

#ifdef PR_USE_REGEX
      if (*((unsigned char *) list->argv[1]) == PR_EXPR_EVAL_REGEX) {
        pr_regex_t *pre = list->argv[2];

        if (session.class) {
          pr_log_debug(DEBUG8, MOD_IFSESSION_VERSION
            ": evaluating regexp pattern '%s' against subject '%s'",
            pr_regexp_get_pattern(pre), session.class->cls_name);

          if (pr_regexp_exec(pre, session.class->cls_name, 0, NULL, 0, 0,
              0) == 0) {
            mergein = TRUE;
          }
        }

      } else
#endif /* regex support */

      if (*((unsigned char *) list->argv[1]) == PR_EXPR_EVAL_OR &&
          pr_expr_eval_class_or((char **) &list->argv[2]) == TRUE) {
        mergein = TRUE;

      } else if (*((unsigned char *) list->argv[1]) == PR_EXPR_EVAL_AND &&
          pr_expr_eval_class_and((char **) &list->argv[2]) == TRUE) {
        mergein = TRUE;
      }

      if (mergein) {
        pr_log_debug(DEBUG2, MOD_IFSESSION_VERSION
          ": merging <IfClass %s> directives in", (char *) list->argv[0]);
        ifsess_dup_set(session.pool, main_server->conf, c->subset);

        /* Add this config_rec pointer to the list of pointers to be
         * removed later.
         */
        *((config_rec **) push_array(class_remove_list)) = c;

        /* We need to call fixup_dirs() twice: once for any added <Directory>
         * sections that use absolute paths, and again for any added <Directory>
         * sections that use deferred-resolution paths (e.g. "~").
         */
        fixup_dirs(main_server, 0);
        fixup_dirs(main_server, CF_DEFER);

        ifsess_merged = TRUE;

      } else {
        pr_log_debug(DEBUG9, MOD_IFSESSION_VERSION
          ": <IfClass %s> not matched, skipping", (char *) list->argv[0]);
      }
    }

    c = find_config_next(c, c->next, -1, IFSESS_CLASS_TEXT, FALSE);
  }

  /* Now, remove any <IfClass> config_recs that have been merged in. */
  for (i = 0; i < class_remove_list->nelts; i++) {
    c = ((config_rec **) class_remove_list->elts)[i];
    xaset_remove(main_server->conf, (xasetmember_t *) c);
  }

  destroy_pool(tmp_pool);
  return 0;
}

/* Module API tables
 */

static conftable ifsess_conftab[] = {
  { IFSESS_CLASS_TEXT,	start_ifctxt,	NULL },
  { "</IfClass>",	end_ifctxt,	NULL },
  { IFSESS_GROUP_TEXT,	start_ifctxt,	NULL },
  { "</IfGroup>",	end_ifctxt,	NULL },
  { IFSESS_USER_TEXT,	start_ifctxt,	NULL },
  { "</IfUser>",	end_ifctxt,	NULL },
  { NULL }
};

static cmdtable ifsess_cmdtab[] = {
  { POST_CMD, C_PASS, G_NONE, ifsess_post_pass, FALSE, FALSE },
  { 0, NULL }
};

module ifsession_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "ifsession",

  /* Module configuration handler table */
  ifsess_conftab,

  /* Module command handler table */
  ifsess_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  ifsess_init,

  /* Session initialization function */
  ifsess_sess_init,

  /* Module version */
  MOD_IFSESSION_VERSION
};
