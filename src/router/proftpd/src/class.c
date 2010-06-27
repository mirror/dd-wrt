/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2003-2008 The ProFTPD Project team
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
 * As a special exemption, the ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Class routines
 * $Id: class.c,v 1.9 2009/11/05 02:19:03 castaglia Exp $
 */

#include "conf.h"

static const char *trace_channel = "class";

/* Store the defined Classes in a linked list.  If many Classes are defined,
 * this may need to be redefined to be a collision-chained hash.
 */
static pr_class_t *class_list = NULL;
static pr_class_t *curr_cls = NULL;

pr_class_t *pr_class_get(pr_class_t *prev) {
  if (prev)
    return prev->cls_next;

  return class_list;
}

pr_class_t *pr_class_match_addr(pr_netaddr_t *addr) {
  pr_class_t *cls;
  pool *tmp_pool;

  if (!addr) {
    errno = EINVAL;
    return NULL;
  }

  tmp_pool = make_sub_pool(permanent_pool);

  for (cls = class_list; cls; cls = cls->cls_next) {
    array_header *acl_list = cls->cls_acls;
    pr_netacl_t **acls = acl_list->elts;
    register int i;
    int next_class = FALSE;

    /* For each ACL rule in this class, compare the rule against the given
     * address.  The address matches the given class depending on the
     * Satisfy setting: if "any", the class matches if any rule matches;
     * if "all", the class matches only if _all_ rules match.
     */
    for (i = 0; i < acl_list->nelts; i++) {
      int res;

      if (next_class)
        break;

      if (acls[i] == NULL)
        continue;

      switch (cls->cls_satisfy) {
        case PR_CLASS_SATISFY_ANY:
          pr_trace_msg(trace_channel, 6,
            "checking addr '%s' (%s) against class '%s' rule: %s "
            "(requires any ACL matching)", pr_netaddr_get_ipstr(addr),
            pr_netaddr_get_dnsstr(addr), cls->cls_name,
            pr_netacl_get_str(tmp_pool, acls[i]));

          res = pr_netacl_match(acls[i], addr);
          if (res == 1) {
            destroy_pool(tmp_pool);
            return cls;
          }
          break;

        case PR_CLASS_SATISFY_ALL:
          pr_trace_msg(trace_channel, 6,
            "checking addr '%s' (%s) against class '%s' ACL: %s "
            "(requires all ACLs matching)", pr_netaddr_get_ipstr(addr),
            pr_netaddr_get_dnsstr(addr), cls->cls_name,
            pr_netacl_get_str(tmp_pool, acls[i]));

          res = pr_netacl_match(acls[i], addr);

          if (res <= 0)
            next_class = TRUE;
          break;
      }
    }

    /* If this is a "Satisfy all" class, and all rules have matched
     * (positively or negatively), then it matches the address.
     */
    if (next_class == FALSE &&
        cls->cls_satisfy == PR_CLASS_SATISFY_ALL &&
        i == acl_list->nelts) {
      destroy_pool(tmp_pool);
      return cls;
    }
  }

  destroy_pool(tmp_pool);

  errno = ENOENT;
  return NULL;
}

pr_class_t *pr_class_find(const char *name) {
  pr_class_t *cls;

  if (!name) {
    errno = EINVAL;
    return NULL;
  }

  for (cls = class_list; cls; cls = cls->cls_next)
    if (strcmp(cls->cls_name, name) == 0)
      return cls;

  errno = ENOENT;
  return NULL;
}

int pr_class_add_acl(pr_netacl_t *acl) {

  if (!acl) {
    errno = EINVAL;
    return -1;
  }

  if (!curr_cls) {
    errno = EPERM;
    return -1;
  }

  /* Add this ACL rule to the current Class. */
  if (!curr_cls->cls_acls)
    curr_cls->cls_acls = make_array(curr_cls->cls_pool, 1,
      sizeof(pr_netacl_t *));

  *((pr_netacl_t **) push_array(curr_cls->cls_acls)) =
    pr_netacl_dup(curr_cls->cls_pool, acl);

  return 0;
}

int pr_class_set_satisfy(int satisfy) {

  if (!curr_cls) {
    errno = EPERM;
    return -1;
  }

  if (satisfy != PR_CLASS_SATISFY_ANY &&
      satisfy != PR_CLASS_SATISFY_ALL) {
    errno = EINVAL;
    return -1;
  }

  /* Set the Satisfy flag on the current Class. */
  curr_cls->cls_satisfy = satisfy;

  return 0;
}

int pr_class_open(pool *p, const char *name) {
  pr_class_t *cls;
  pool *cls_pool;

  if (!p || !name) {
    errno = EINVAL;
    return -1;
  }

  /* Allocate a sub pool from the given pool, from which a new Class will
   * be allocated.
   */
  cls_pool = make_sub_pool(p);
  pr_pool_tag(cls_pool, "<Class> Pool");

  cls = pcalloc(cls_pool, sizeof(pr_class_t));
  cls->cls_pool = cls_pool;
  cls->cls_name = pstrdup(cls->cls_pool, name);
  cls->cls_satisfy = PR_CLASS_SATISFY_ANY;
 
  /* Change the configuration context type. */
  main_server->config_type = CONF_CLASS;

  curr_cls = cls;
  return 0;
}

int pr_class_close(void) {

  /* If there is no current Class, there is nothing to do. */
  if (!curr_cls)
    return 0;

  /* If there are no client rules in this class, simply remove it.  No need
   * to waste space.
   */
  if (!curr_cls->cls_acls) {
    destroy_pool(curr_cls->cls_pool);
    curr_cls = NULL;

    /* Restore the configuration context type. */
    main_server->config_type = CONF_ROOT;

    errno = EINVAL;
    return -1;
  }

  /* Make sure the list of clients is NULL-terminated. */
  push_array(curr_cls->cls_acls);

  /* Now add the current Class to the end of the list. */
  if (class_list) {
    pr_class_t *ci = class_list;
    while (ci && ci->cls_next)
      ci = ci->cls_next;

    ci->cls_next = curr_cls;

  } else
    class_list = curr_cls;

  curr_cls = NULL;

  /* Restore the configuration context type. */
  main_server->config_type = CONF_ROOT;

  return 0;
}

void init_class(void) {
  class_list = NULL;
  return;
}
