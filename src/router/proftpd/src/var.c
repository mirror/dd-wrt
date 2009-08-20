/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2004-2008 The ProFTPD Project team
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
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Variables API implementation
 * $Id: var.c,v 1.5 2008/02/18 02:13:41 castaglia Exp $
 */

#include "conf.h"

struct var {
  int v_type;
  const char *v_desc;
  void *v_val;
  void *v_data;
  size_t v_datasz;
};

static pool *var_pool = NULL;
static pr_table_t *var_tab = NULL;

typedef const char *(*var_vstr_cb)(void *, size_t);

/* Public API
 */

int pr_var_delete(const char *name) {
  if (!var_tab) {
    errno = EPERM;
    return -1;
  }

  if (!name) {
    errno = EINVAL;
    return -1;
  }

  return pr_table_remove(var_tab, name, NULL) ? 0 : -1;
}

int pr_var_exists(const char *name) {
  if (!var_tab) {
    errno = EPERM;
    return -1;
  }

  if (!name) {
    errno = EINVAL;
    return -1;
  }

  return pr_table_exists(var_tab, name) > 0 ? TRUE : FALSE;
}

const char *pr_var_get(const char *name) {
  struct var *v;

  if (!var_tab) {
    errno = EPERM;
    return NULL;
  }

  if (!name) {
    errno = EINVAL;
    return NULL;
  }

  v = pr_table_get(var_tab, name, NULL);
  if (!v)
    return NULL;

  switch (v->v_type) {
    case PR_VAR_TYPE_STR:
      return (const char *) v->v_val;
      break;

    case PR_VAR_TYPE_FUNC:
      return ((var_vstr_cb) v->v_val)(v->v_data, v->v_datasz);
      break;

    default:
      /* Pass through to the error case. */
      ;
  }

  errno = EINVAL;
  return NULL;
}

const char *pr_var_next(const char **desc) {
  const char *name;
  struct var *v;

  if (!var_tab) {
    errno = EPERM;
    return NULL;
  }

  name = pr_table_next(var_tab);
  if (!name)
    return NULL;

  v = pr_table_get(var_tab, name, NULL);
  if (v && desc)
    *desc = v->v_desc;

  return name;
}

void pr_var_rewind(void) {
  if (var_tab)
    pr_table_rewind(var_tab);
}

int pr_var_set(pool *p, const char *name, const char *desc, int type,
    void *val, void *data, size_t datasz) {
  struct var *v;

  if (var_tab == NULL) {
    errno = EPERM;
    return -1;
  }

  if (p == NULL ||
      name == NULL ||
      val == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* The length of the key must be greater than 3 characters (for "%{}"). */
  if (strlen(name) < 4) {
    errno = EINVAL;
    return -1;
  }

  /* Specifying data, but no length for that data, is an error. */
  if (data != NULL &&
      datasz == 0) {
    errno = EINVAL;
    return -1;
  }

  /* Specifying no data, but providing a non-zero length for that data, is an
   * error.
   */
  if (data == NULL &&
      datasz > 0) {
    errno = EINVAL;
    return -1;
  }

  /* Variable names MUST start with '%{', and end in '}'. */
  if (strncmp(name, "%{", 2) != 0 ||
      name[strlen(name)-1] != '}') {
    errno = EINVAL;
    return -1;
  }

  /* Remove any previously registered value for this name.  For names whose
   * values change rapidly (e.g. session.xfer.total_bytes), a callback
   * function should be used, rather than always setting the same name as an
   * update; using a callback avoids the memory consumption that setting does
   * (set always allocates a new struct var *).
   */
  (void) pr_var_delete(name);

  /* Note: if var_pool was used for allocating the struct var *, rather
   * than the given pool, then deleting an entry would not necessarily
   * lead to such memory consumption (assuming it would even be a problem).
   * However, if this was the case, then a churn counter would be needed,
   * and var_pool would need to be churned occasionally to limit memory
   * growth.
   */

  switch (type) {
    case PR_VAR_TYPE_STR:
      v = pcalloc(p, sizeof(struct var));

      if (desc)
        v->v_desc = (const char *) pstrdup(p, desc);
      v->v_type = PR_VAR_TYPE_STR; 
      v->v_val = pstrdup(p, (char *) val);
      v->v_datasz = strlen((char *) val);
      break;

    case PR_VAR_TYPE_FUNC:
      v = pcalloc(p, sizeof(struct var));

      if (desc)
        v->v_desc = (const char *) pstrdup(p, desc);
      v->v_type = PR_VAR_TYPE_FUNC; 
      v->v_val = (var_vstr_cb) val;

      if (data) {
        v->v_data = data;
        v->v_datasz = datasz;
      }

      break;

    default:
      errno = EINVAL;
      return -1;
  }

  return pr_table_add(var_tab, name, v, sizeof(struct var));
}

int var_init(void) {

  if (!var_pool) {
    var_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(var_pool, "Variables Pool");
  }

  if (!var_tab)
    var_tab = pr_table_alloc(var_pool, 0);

  return 0;
}

int var_free(void) {
  if (var_pool) {
    if (var_tab) {
      pr_table_empty(var_tab);
      pr_table_free(var_tab);
    }

    destroy_pool(var_pool);
    var_pool = NULL;
    var_tab = NULL;
  }

  return 0;
}
