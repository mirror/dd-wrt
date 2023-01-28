/*
 * ProFTPD: mod_ctrls -- a module implementing the ftpdctl local socket
 *                       server
 * Copyright (c) 2000-2020 The ProFTPD Project team
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
 * This is mod_ctrls, contrib software for proftpd 1.2 and above.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 */

#ifndef MOD_CTRLS_H
#define MOD_CTRLS_H

#include <signal.h>
#include "conf.h"

# if defined(PR_USE_CTRLS)

/* Controls access control objects */

typedef struct {
  unsigned char allow;
  unsigned int nuids;
  uid_t *uids;
} ctrls_user_acl_t;

typedef struct {
  unsigned char allow;
  unsigned int ngids;
  gid_t *gids;
} ctrls_group_acl_t;

typedef struct {
  pool *acl_pool;
  ctrls_user_acl_t acl_users;
  ctrls_group_acl_t acl_groups;
} ctrls_acl_t;

typedef struct {
  const char *act_action;
  const char *act_desc;
  ctrls_acl_t *act_acl;
  int (*act_cb)(pr_ctrls_t *, int, char **);
} ctrls_acttab_t;

int pr_ctrls_check_group_acl(gid_t, const ctrls_group_acl_t *);
int pr_ctrls_check_user_acl(uid_t, const ctrls_user_acl_t *);

/* Returns TRUE if use of ctrl named by action is allowed by the user or group
 * ACL, FALSE otherwise.  The ACLs associated with the given action are
 * looked up in the given table.  The default is to deny everyone, unless an
 * ACL has been configured to allow them.
 */
int pr_ctrls_check_acl(const pr_ctrls_t *ctrl,
  const ctrls_acttab_t *ctrls_acttab, const char *action);

/* Initialize an ctrls_acl_t object. */
int pr_ctrls_init_acl(ctrls_acl_t *acl);

/* Parse the comma-separated list of names in a Controls ACL into a
 * NULL-terminated array.
 */
char **pr_ctrls_parse_acl(pool *acl_pool, const char *acl_text);

int pr_ctrls_set_group_acl(pool *, ctrls_group_acl_t *, const char *, char *);
int pr_ctrls_set_user_acl(pool *, ctrls_user_acl_t *, const char *, char *);

/* Configures the ACLs for the actions implemented in a Controls module.
 *
 * On error, the bad_action argument, if provided, will be set to the
 * offending/unknown action.
 */
char *pr_ctrls_set_module_acls(ctrls_acttab_t *ctrls_acttab, pool *acl_pool,
  char **actions, const char *allow, const char *type, char *list);
int pr_ctrls_set_module_acls2(ctrls_acttab_t *ctrls_acttab, pool *acl_pool,
  char **actions, const char *allow, const char *type, char *list,
  const char **bad_action);

/* Unregisters a module's actions according to the configured list.
 *
 * On error, the bad_action argument, if provided, will be set to the
 * offending/unknown action.
 */
char *pr_ctrls_unregister_module_actions(ctrls_acttab_t *ctrls_acttab,
  char **actions, module *mod);
int pr_ctrls_unregister_module_actions2(ctrls_acttab_t *ctrls_acttab,
  char **actions, module *mod, const char **bad_action);

/* Set the file descriptor that the Controls API should use for logging. */
int pr_ctrls_set_logfd(int fd);

/* Logs into a mod_ctrls-specific log file. */
int pr_ctrls_log(const char *module_version, const char *fmt, ...)
#ifdef __GNUC__
       __attribute__ ((format (printf, 2, 3)));
#else
       ;
#endif

# endif /* PR_USE_CTRLS */
#endif /* MOD_CTRLS_H */
