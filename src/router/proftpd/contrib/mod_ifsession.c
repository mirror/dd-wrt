/*
 * ProFTPD: mod_ifsession -- a module supporting conditional
 *                            per-user/group/class configuration contexts.
 *
 * Copyright (c) 2002-2008 TJ Saunders
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
 * This is mod_ifsession, contrib software for proftpd 1.2 and above.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 *
 * $Id: mod_ifsession.c,v 1.24 2008/10/14 22:28:02 castaglia Exp $
 */

#include "conf.h"

#define MOD_IFSESSION_VERSION	"mod_ifsession/1.0"

/* Make sure the version of proftpd is as necessary. */
#if PROFTPD_VERSION_NUMBER < 0x0001021001
# error "ProFTPD 1.2.10rc1 or later required"
#endif

#define IFSESS_CLASS_NUMBER	100
#define IFSESS_CLASS_TEXT	"<IfClass>"
#define IFSESS_GROUP_NUMBER	101
#define	IFSESS_GROUP_TEXT	"<IfGroup>"
#define IFSESS_USER_NUMBER	102
#define	IFSESS_USER_TEXT	"<IfUser>"

static int ifsess_merged = FALSE;

/* Support routines
 */

static void ifsess_remove_param(xaset_t *set, const char *name) {
  config_rec *c = NULL;

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
          !(c->flags & CF_MERGEDOWN_MULTI))
        ifsess_remove_param(dup_c->subset, c->name);

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
        !(c->flags & CF_MERGEDOWN_MULTI))
      ifsess_remove_param(dst, c->name);

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
    config_type = IFSESS_CLASS_NUMBER;
    eval_type = PR_EXPR_EVAL_OR;

  } else if (strcmp(cmd->argv[0], IFSESS_GROUP_TEXT) == 0) {
    name = "_IfGroupList";
    config_type = IFSESS_GROUP_NUMBER;
    eval_type = PR_EXPR_EVAL_AND;

  } else if (strcmp(cmd->argv[0], IFSESS_USER_TEXT) == 0) {
    name = "_IfUserList";
    config_type = IFSESS_USER_NUMBER;
    eval_type = PR_EXPR_EVAL_OR;
  }

  /* Is this a normal expression, an explicit AND, an explicit OR, or a
   * regular expression?
   */
  if (cmd->argc-1 > 1) {
    if (strcmp(cmd->argv[1], "AND") == 0) {
      eval_type = PR_EXPR_EVAL_AND;
      argc = cmd->argc-2;
      argv = cmd->argv+1;

    } else if (strcmp(cmd->argv[1], "OR") == 0) {
      eval_type = PR_EXPR_EVAL_OR;
      argc = cmd->argc-2;
      argv = cmd->argv+1;

    } else if (strcmp(cmd->argv[1], "regex") == 0) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
      regex_t *preg = NULL;
      int res = 0;

      if (cmd->argc != 3)
        CONF_ERROR(cmd, "wrong number of parameters");

      preg = pr_regexp_alloc();

      res = regcomp(preg, cmd->argv[2], REG_EXTENDED|REG_NOSUB);
      if (res != 0) {
        char errstr[200] = {'\0'};

        regerror(res, preg, errstr, sizeof(errstr));
        pr_regexp_free(preg);

        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": '", cmd->argv[2], "' failed "
          "regex compilation: ", errstr, NULL));
      }

      c = add_config_param(name, 2, NULL, NULL);
      c->config_type = config_type;
      c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
      *((unsigned char *) c->argv[0]) = PR_EXPR_EVAL_REGEX;
      c->argv[1] = (void *) preg;

      return PR_HANDLED(cmd);

#else
      CONF_ERROR(cmd, "The 'regex' parameter cannot be used on this system, "
        "as you do not have POSIX compliant regex support");
#endif /* HAVE_REGEX_H && HAVE_REGCOMP */

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
  c->argc = acl->nelts + 1;
  c->argv = pcalloc(c->pool, (c->argc + 1) * sizeof(char *));
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = eval_type;

  argv = (char **) c->argv + 1;

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

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
      if (*((unsigned char *) list->argv[0]) == PR_EXPR_EVAL_REGEX) {
        regex_t *preg = (regex_t *) list->argv[1];

        if (session.group && regexec(preg, session.group, 0, NULL, 0) == 0)
          mergein = TRUE;

        else if (session.groups) {
          register int j = 0;

          for (j = session.groups->nelts-1; j >= 0; j--)
            if (regexec(preg, *(((char **) session.groups->elts) + j), 0,
                NULL, 0) == 0)
              mergein = TRUE;
        }
      } else
#endif /* HAVE_REGEX_H && HAVE_REGCOMP */
    
      if (*((unsigned char *) list->argv[0]) == PR_EXPR_EVAL_OR &&
          pr_expr_eval_group_or((char **) &list->argv[1]) == TRUE)
        mergein = TRUE;

      else if (*((unsigned char *) list->argv[0]) == PR_EXPR_EVAL_AND &&
          pr_expr_eval_group_and((char **) &list->argv[1]) == TRUE)
        mergein = TRUE;
 
      if (pr_expr_eval_group_and((char **) &list->argv[0]) == TRUE)
        mergein = TRUE;

      if (mergein) {
        pr_log_debug(DEBUG2, MOD_IFSESSION_VERSION
          ": merging <IfGroup> directives in");
        ifsess_dup_set(session.pool, main_server->conf, c->subset);

        /* Add this config_rec pointer to the list of pointers to be
         * removed later.
         */
        *((config_rec **) push_array(group_remove_list)) = c;

        resolve_deferred_dirs(main_server);
        fixup_dirs(main_server, CF_DEFER);

        ifsess_merged = TRUE;

      } else
        pr_log_debug(DEBUG9, MOD_IFSESSION_VERSION
          ": <IfGroup> not matched, skipping");
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

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
      if (*((unsigned char *) list->argv[0]) == PR_EXPR_EVAL_REGEX) {
        regex_t *preg = (regex_t *) list->argv[1];

        if (regexec(preg, session.user, 0, NULL, 0) == 0)
          mergein = TRUE;

      } else
#endif /* HAVE_REGEX_H && HAVE_REGCOMP */

      if (*((unsigned char *) list->argv[0]) == PR_EXPR_EVAL_OR &&
          pr_expr_eval_user_or((char **) &list->argv[1]) == TRUE)
        mergein = TRUE;

      else if (*((unsigned char *) list->argv[0]) == PR_EXPR_EVAL_AND &&
          pr_expr_eval_user_and((char **) &list->argv[1]) == TRUE)
        mergein = TRUE;

      if (mergein) {
        pr_log_debug(DEBUG2, MOD_IFSESSION_VERSION
          ": merging <IfUser> directives in");
        ifsess_dup_set(session.pool, main_server->conf, c->subset);

        /* Add this config_rec pointer to the list of pointers to be
         * removed later.
         */
        *((config_rec **) push_array(user_remove_list)) = c;

        resolve_deferred_dirs(main_server);
        fixup_dirs(main_server, CF_DEFER);

        ifsess_merged = TRUE;

      } else
        pr_log_debug(DEBUG9, MOD_IFSESSION_VERSION
          ": <IfUser> not matched, skipping");
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
      pr_log_auth(PR_LOG_NOTICE, "%s %s: Limit access denies login.",
        session.anon_config ? "ANON" : C_USER, session.user);
      end_login(0);
    }
  }

  return PR_DECLINED(cmd);
}

/* Initialization routines
 */

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

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
      if (*((unsigned char *) list->argv[0]) == PR_EXPR_EVAL_REGEX) {
        regex_t *preg = (regex_t *) list->argv[1];

        if (session.class && regexec(preg, session.class->cls_name, 0, NULL,
            0) == 0)
          mergein = TRUE;

      } else
#endif /* HAVE_REGEX_H && HAVE_REGCOMP */

      if (*((unsigned char *) list->argv[0]) == PR_EXPR_EVAL_OR &&
          pr_expr_eval_class_or((char **) &list->argv[1]) == TRUE)
        mergein = TRUE;

      else if (*((unsigned char *) list->argv[0]) == PR_EXPR_EVAL_AND &&
          pr_expr_eval_class_and((char **) &list->argv[1]) == TRUE)
        mergein = TRUE;

      if (mergein) {
        pr_log_debug(DEBUG2, MOD_IFSESSION_VERSION
          ": merging <IfClass> directives in");
        ifsess_dup_set(session.pool, main_server->conf, c->subset);

        /* Add this config_rec pointer to the list of pointers to be
         * removed later.
         */
        *((config_rec **) push_array(class_remove_list)) = c;

        fixup_dirs(main_server, CF_DEFER);

        ifsess_merged = TRUE;

      } else
        pr_log_debug(DEBUG9, MOD_IFSESSION_VERSION
          ": <IfClass> not matched, skipping");
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
  NULL,

  /* Session initialization function */
  ifsess_sess_init,

  /* Module version */
  MOD_IFSESSION_VERSION
};
