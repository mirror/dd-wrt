/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2009 The ProFTPD Project team
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

/* Authentication front-end for ProFTPD
 * $Id: auth.c,v 1.80 2009/12/06 17:19:17 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"

static pool *auth_pool = NULL;
static pr_table_t *auth_tab = NULL, *uid_tab = NULL, *gid_tab = NULL;
static xaset_t *auth_module_list = NULL;

struct auth_module_elt {
  struct auth_module_elt *prev, *next;
  const char *name;
};

static const char *trace_channel = "auth";

/* Caching of ID-to-name lookups, for both UIDs and GIDs, is enabled by
 * default.
 */
static unsigned int auth_caching = PR_AUTH_CACHE_FL_UID2NAME|PR_AUTH_CACHE_FL_GID2NAME|PR_AUTH_CACHE_FL_AUTH_MODULE;

/* Key comparison callback for the uidcache and gidcache. */
static int uid_keycmp_cb(const void *key1, size_t keysz1,
    const void *key2, size_t keysz2) {

  /* Return zero to indicate a match, non-zero otherwise. */
  return (*((uid_t *) key1) == *((uid_t *) key2) ? 0 : 1);
}

static int gid_keycmp_cb(const void *key1, size_t keysz1,
    const void *key2, size_t keysz2) {

  /* Return zero to indicate a match, non-zero otherwise. */
  return (*((gid_t *) key1) == *((gid_t *) key2) ? 0 : 1);
}

/* Key "hash" callback for the uidcache and gidcache. */
static unsigned int uid_hash_cb(const void *key, size_t keysz) {
  uid_t u;
  unsigned int res;

  memcpy(&u, key, keysz);
  res = (unsigned int) (u << 8);

  return res;
}

static unsigned int gid_hash_cb(const void *key, size_t keysz) {
  gid_t g;
  unsigned int res;

  memcpy(&g, key, keysz);
  res = (unsigned int) (g << 8);

  return res;
}

static void uidcache_create(void) {
  if ((auth_caching & PR_AUTH_CACHE_FL_UID2NAME) &&
      !uid_tab &&
      auth_pool) {
    int ok = TRUE;

    uid_tab = pr_table_alloc(auth_pool, 0);

    if (pr_table_ctl(uid_tab, PR_TABLE_CTL_SET_KEY_CMP, uid_keycmp_cb) < 0) {
      pr_trace_msg(trace_channel, 2,
        "error setting key comparison callback for uidcache: %s",
        strerror(errno));
      ok = FALSE;
    }

    if (pr_table_ctl(uid_tab, PR_TABLE_CTL_SET_KEY_HASH, uid_hash_cb) < 0) {
      pr_trace_msg(trace_channel, 2,
        "error setting key hash callback for uidcache: %s",
        strerror(errno));
      ok = FALSE;
    }

    if (!ok) {
      pr_trace_msg(trace_channel, 2, "%s",
        "destroying unusable uidcache table");
      pr_table_free(uid_tab);
      uid_tab = NULL;
    }
  }
}

static void uidcache_add(uid_t uid, const char *name) {
  uidcache_create();

  if (uid_tab) {
    int count;

    (void) pr_table_rewind(uid_tab);
    count = pr_table_kexists(uid_tab, (const void *) &uid, sizeof(uid_t));
    if (count <= 0) {
      uid_t *cache_uid;

      /* Allocate memory for a UID out of the ID cache pool, so that this
       * UID can be used as a key.
       */
      cache_uid = palloc(auth_pool, sizeof(uid_t));
      *cache_uid = uid;

      if (pr_table_kadd(uid_tab, (const void *) cache_uid, sizeof(uid_t),
          pstrdup(auth_pool, name), strlen(name) + 1) < 0 &&
          errno != EEXIST) {
        pr_trace_msg(trace_channel, 3,
          "error adding name '%s' for UID %lu to the uidcache: %s", name,
          (unsigned long) uid, strerror(errno));

      } else {
        pr_trace_msg(trace_channel, 5,
          "stashed name '%s' for UID %lu in the uidcache", name,
          (unsigned long) uid);
      }
    }
  }
}

static void gidcache_create(void) {
  if ((auth_caching & PR_AUTH_CACHE_FL_GID2NAME) &&
      !gid_tab &&
      auth_pool) {
    int ok = TRUE;

    gid_tab = pr_table_alloc(auth_pool, 0);

    if (pr_table_ctl(gid_tab, PR_TABLE_CTL_SET_KEY_CMP, gid_keycmp_cb) < 0) {
      pr_trace_msg(trace_channel, 2,
        "error setting key comparison callback for gidcache: %s",
        strerror(errno));
      ok = FALSE;
    }

    if (pr_table_ctl(gid_tab, PR_TABLE_CTL_SET_KEY_HASH, gid_hash_cb) < 0) {
      pr_trace_msg(trace_channel, 2,
        "error setting key hash callback for gidcache: %s",
        strerror(errno));
      ok = FALSE;
    }

    if (!ok) {
      pr_trace_msg(trace_channel, 2, "%s",
        "destroying unusable gidcache table");
      pr_table_free(gid_tab);
      gid_tab = NULL;
    }
  }
}

static void gidcache_add(gid_t gid, const char *name) {
  gidcache_create();

  if (gid_tab) {
    int count;

    (void) pr_table_rewind(gid_tab);
    count = pr_table_kexists(gid_tab, (const void *) &gid, sizeof(gid_t));
    if (count <= 0) {
      gid_t *cache_gid;

      /* Allocate memory for a GID out of the ID cache pool, so that this
       * GID can be used as a key.
       */
      cache_gid = palloc(auth_pool, sizeof(gid_t));
      *cache_gid = gid;

      if (pr_table_kadd(gid_tab, (const void *) cache_gid, sizeof(gid_t),
          pstrdup(auth_pool, name), strlen(name) + 1) < 0 &&
          errno != EEXIST) {
        pr_trace_msg(trace_channel, 3,
          "error adding name '%s' for GID %lu to the gidcache: %s", name,
          (unsigned long) gid, strerror(errno));

      } else {
        pr_trace_msg(trace_channel, 5,
          "stashed name '%s' for GID %lu in the gidcache", name,
          (unsigned long) gid);
      }
    }
  }
}

/* The difference between this function, and pr_cmd_alloc(), is that this
 * allocates the cmd_rec directly from the given pool, whereas pr_cmd_alloc()
 * will allocate a subpool from the given pool, and allocate its cmd_rec
 * from the subpool.  This means that pr_cmd_alloc()'s cmd_rec's can be
 * subsequently destroyed easily; this function's cmd_rec's will be destroyed
 * when the given pool is destroyed.
 */
static cmd_rec *make_cmd(pool *cp, int argc, ...) {
  va_list args;
  cmd_rec *c;
  pool *sub_pool;

  c = pcalloc(cp, sizeof(cmd_rec));

  c->argc = argc;
  c->stash_index = -1;

  if (argc) {
    register unsigned int i;

    c->argv = pcalloc(cp, sizeof(void *) * (argc + 1));

    va_start(args, argc);

    for (i = 0; i < argc; i++) {
      c->argv[i] = (void *) va_arg(args, char *);
    }

    va_end(args);

    c->argv[argc] = NULL;
  }

  /* Make sure we provide pool and tmp_pool for the consumers. */
  sub_pool = make_sub_pool(cp);
  c->pool = c->tmp_pool = sub_pool;

  return c;
}

static modret_t *dispatch_auth(cmd_rec *cmd, char *match, module **m) {
  authtable *start_tab = NULL, *iter_tab = NULL;
  modret_t *mr = NULL;

  start_tab = pr_stash_get_symbol(PR_SYM_AUTH, match, NULL,
    &cmd->stash_index);
  iter_tab = start_tab;

  while (iter_tab) {
    pr_signals_handle();

    if (m && *m && *m != iter_tab->m) {
      goto next;
    }

    pr_trace_msg(trace_channel, 6,
      "dispatching auth request \"%s\" to module mod_%s",
      match, iter_tab->m->name);

    mr = pr_module_call(iter_tab->m, iter_tab->handler, cmd);

    if (iter_tab->auth_flags & PR_AUTH_FL_REQUIRED)
      break;

    if (MODRET_ISHANDLED(mr) ||
        MODRET_ISERROR(mr)) {

      /* Return a pointer, if requested, to the module which answered the
       * auth request.  This is used, for example, by auth_getpwnam() for
       * associating the answering auth module with the data looked up.
       */
      if (m)
        *m = iter_tab->m;

      break;
    }

  next:
    iter_tab = pr_stash_get_symbol(PR_SYM_AUTH, match, iter_tab,
      &cmd->stash_index);

    if (iter_tab == start_tab) {
      /* We have looped back to the start.  Break out now and do not loop
       * around again (and again, and again...)
       */
      mr = PR_DECLINED(cmd);
      break;
    }
  }

  return mr;
}

void pr_auth_setpwent(pool *p) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;

  cmd = make_cmd(p, 0);
  mr = dispatch_auth(cmd, "setpwent", NULL);

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return;
}

void pr_auth_endpwent(pool *p) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;

  cmd = make_cmd(p, 0);
  mr = dispatch_auth(cmd, "endpwent", NULL);

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  if (auth_tab) {
    pr_trace_msg(trace_channel, 5, "emptying authcache");
    (void) pr_table_empty(auth_tab);
    (void) pr_table_free(auth_tab);
    auth_tab = NULL;
  }

  return;
}

void pr_auth_setgrent(pool *p) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;

  cmd = make_cmd(p, 0);
  mr = dispatch_auth(cmd, "setgrent", NULL);

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return;
}

void pr_auth_endgrent(pool *p) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;

  cmd = make_cmd(p, 0);
  mr = dispatch_auth(cmd, "endgrent", NULL);

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return;
}

struct passwd *pr_auth_getpwent(pool *p) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  struct passwd *res = NULL;

  cmd = make_cmd(p, 0);
  mr = dispatch_auth(cmd, "getpwent", NULL);

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr))
    res = mr->data;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  /* Sanity check */
  if (res == NULL)
    return NULL;

  /* Make sure the UID and GID are not -1 */
  if (res->pw_uid == (uid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: UID of -1 not allowed");
    return NULL;
  }

  if (res->pw_gid == (gid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: GID of -1 not allowed");
    return NULL;
  }

  return res;
}

struct group *pr_auth_getgrent(pool *p) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  struct group *res = NULL;

  cmd = make_cmd(p, 0);
  mr = dispatch_auth(cmd, "getgrent", NULL);

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr))
    res = mr->data;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  /* Sanity check */
  if (res == NULL)
    return NULL;

  /* Make sure the GID is not -1 */
  if (res->gr_gid == (gid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: GID of -1 not allowed");
    return NULL;
  }

  return res;
}

struct passwd *pr_auth_getpwnam(pool *p, const char *name) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  struct passwd *res = NULL;
  module *m = NULL;

  cmd = make_cmd(p, 1, name);
  mr = dispatch_auth(cmd, "getpwnam", &m);

  if (MODRET_ISHANDLED(mr) &&
      MODRET_HASDATA(mr))
    res = mr->data;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  /* Sanity check */
  if (res == NULL) {
    errno = ENOENT;
    return NULL;
  }

  /* Make sure the UID and GID are not -1 */
  if (res->pw_uid == (uid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: UID of -1 not allowed");
    return NULL;
  }

  if (res->pw_gid == (gid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: GID of -1 not allowed");
    return NULL;
  }

  if ((auth_caching & PR_AUTH_CACHE_FL_AUTH_MODULE) &&
      !auth_tab &&
      auth_pool) {
    auth_tab = pr_table_alloc(auth_pool, 0);
  }

  if (m &&
      auth_tab) {
    int count = 0;
    void *value = NULL;

    value = palloc(auth_pool, sizeof(module *));
    *((module **) value) = m;

    count = pr_table_exists(auth_tab, name);
    if (count <= 0) {
      if (pr_table_add(auth_tab, pstrdup(auth_pool, name), value,
          sizeof(module *)) < 0) {
        pr_trace_msg(trace_channel, 3,
          "error adding module 'mod_%s.c' for user '%s' to the authcache: %s",
          m->name, name, strerror(errno));

      } else {
        pr_trace_msg(trace_channel, 5,
          "stashed module 'mod_%s.c' for user '%s' in the authcache",
          m->name, name);
      }

    } else {
      if (pr_table_set(auth_tab, pstrdup(auth_pool, name), value,
          sizeof(module *)) < 0) {
        pr_trace_msg(trace_channel, 3,
          "error setting module 'mod_%s.c' for user '%s' in the authcache: %s",
          m->name, name, strerror(errno));

      } else {
        pr_trace_msg(trace_channel, 5,
          "stashed module 'mod_%s.c' for user '%s' in the authcache",
          m->name, name);
      }
    }
  }

  uidcache_add(res->pw_uid, name);

  /* Get the (possibly rewritten) home directory. */
  res->pw_dir = pr_auth_get_home(p, res->pw_dir);

  pr_log_debug(DEBUG10, "retrieved UID %lu for user '%s'",
    (unsigned long) res->pw_uid, name);
  return res;
}

struct passwd *pr_auth_getpwuid(pool *p, uid_t uid) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  struct passwd *res = NULL;

  cmd = make_cmd(p, 1, (void *) &uid);
  mr = dispatch_auth(cmd, "getpwuid", NULL);

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr))
    res = mr->data;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  /* Sanity check */
  if (res == NULL) {
    errno = ENOENT;
    return NULL;
  }

  /* Make sure the UID and GID are not -1 */
  if (res->pw_uid == (uid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: UID of -1 not allowed");
    return NULL;
  }

  if (res->pw_gid == (gid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: GID of -1 not allowed");
    return NULL;
  }

  pr_log_debug(DEBUG10, "retrieved user '%s' for UID %lu",
    res->pw_name, (unsigned long) uid);
  return res;
}

struct group *pr_auth_getgrnam(pool *p, const char *name) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  struct group *res = NULL;

  cmd = make_cmd(p, 1, name);
  mr = dispatch_auth(cmd, "getgrnam", NULL);

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr))
    res = mr->data;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  /* Sanity check */
  if (res == NULL) {
    errno = ENOENT;
    return NULL;
  }

  /* Make sure the GID is not -1 */
  if (res->gr_gid == (gid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: GID of -1 not allowed");
    return NULL;
  }

  gidcache_add(res->gr_gid, name);

  pr_log_debug(DEBUG10, "retrieved GID %lu for group '%s'",
    (unsigned long) res->gr_gid, name);
  return res;
}

struct group *pr_auth_getgrgid(pool *p, gid_t gid) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  struct group *res = NULL;

  cmd = make_cmd(p, 1, (void *) &gid);
  mr = dispatch_auth(cmd, "getgrgid", NULL);

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr))
    res = mr->data;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  /* Sanity check */
  if (res == NULL) {
    errno = ENOENT;
    return NULL;
  }

  /* Make sure the GID is not -1 */
  if (res->gr_gid == (gid_t) -1) {
    pr_log_pri(PR_LOG_ERR, "error: GID of -1 not allowed");
    return NULL;
  }

  pr_log_debug(DEBUG10, "retrieved group '%s' for GID %lu",
    res->gr_name, (unsigned long) gid);
  return res;
}

int pr_auth_authenticate(pool *p, const char *name, const char *pw) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  module *m = NULL;
  int res = PR_AUTH_NOPWD;

  cmd = make_cmd(p, 2, name, pw);

  /* First, check for any of the modules in the "authenticating only" list
   * of modules.  This is usually only mod_auth_pam, but other modules
   * might also add themselves (e.g. mod_radius under certain conditions).
   */
  if (auth_module_list) {
    struct auth_module_elt *elt;

    for (elt = (struct auth_module_elt *) auth_module_list->xas_list; elt;
        elt = elt->next) {

      pr_trace_msg(trace_channel, 7, "checking with auth-only module '%s'",
        elt->name);

      m = pr_module_get(elt->name);
      if (m) {
        mr = dispatch_auth(cmd, "auth", &m);

        if (MODRET_ISHANDLED(mr)) {
          pr_trace_msg(trace_channel, 4,
            "module '%s' used for authenticating user '%s'", elt->name, name);

          res = MODRET_HASDATA(mr) ? PR_AUTH_RFC2228_OK : PR_AUTH_OK;

          if (cmd->tmp_pool) {
            destroy_pool(cmd->tmp_pool);
            cmd->tmp_pool = NULL;
          }

          return res;
        }

        if (MODRET_ISERROR(mr)) {
          res = MODRET_ERROR(mr);

          if (cmd->tmp_pool) {
            destroy_pool(cmd->tmp_pool);
            cmd->tmp_pool = NULL;
          }

          return res;
        }

        m = NULL;
      }
    }
  }

  if (auth_tab) {

    /* Fetch the specific module to be used for authenticating this user. */
    void *v = pr_table_get(auth_tab, name, NULL);
    if (v) {
      m = *((module **) v);

      pr_trace_msg(trace_channel, 4,
        "using module 'mod_%s.c' from authcache to authenticate user '%s'",
        m->name, name);
    }
  }

  mr = dispatch_auth(cmd, "auth", m ? &m : NULL);

  if (MODRET_ISHANDLED(mr))
    res = MODRET_HASDATA(mr) ? PR_AUTH_RFC2228_OK : PR_AUTH_OK;

  else if (MODRET_ISERROR(mr))
    res = MODRET_ERROR(mr);

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

int pr_auth_authorize(pool *p, const char *name) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  module *m = NULL;
  int res = PR_AUTH_OK;

  cmd = make_cmd(p, 1, name);

  if (auth_tab) {

    /* Fetch the specific module to be used for authenticating this user. */
    void *v = pr_table_get(auth_tab, name, NULL);
    if (v) {
      m = *((module **) v);

      pr_trace_msg(trace_channel, 4,
        "using module 'mod_%s.c' from authcache to authorize user '%s'",
        m->name, name);
    }
  }

  mr = dispatch_auth(cmd, "authorize", m ? &m : NULL);

  /* Unlike the other auth calls, we assume here that unless the handlers
   * explicitly return ERROR, the user is authorized.  Thus HANDLED and
   * DECLINED are both treated as "yes, this user is authorized".  This
   * handles the case where the authenticating module (e.g. mod_sql)
   * does NOT provide an 'authorize' handler.
   */

  if (MODRET_ISERROR(mr)) {
    res = MODRET_ERROR(mr);
  }

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

int pr_auth_check(pool *p, const char *cpw, const char *name, const char *pw) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  module *m = NULL;
  int res = PR_AUTH_BADPWD;

  cmd = make_cmd(p, 3, cpw, name, pw);

  /* First, check for any of the modules in the "authenticating only" list
   * of modules.  This is usually only mod_auth_pam, but other modules
   * might also add themselves (e.g. mod_radius under certain conditions).
   */
  if (auth_module_list) {
    struct auth_module_elt *elt;

    for (elt = (struct auth_module_elt *) auth_module_list->xas_list; elt;
        elt = elt->next) {

      m = pr_module_get(elt->name);
      if (m) {
        mr = dispatch_auth(cmd, "check", &m);

        if (MODRET_ISHANDLED(mr)) {
          pr_trace_msg(trace_channel, 4,
            "module '%s' used for authenticating user '%s'", elt->name, name);

          res = MODRET_HASDATA(mr) ? PR_AUTH_RFC2228_OK : PR_AUTH_OK;

          if (cmd->tmp_pool) {
            destroy_pool(cmd->tmp_pool);
            cmd->tmp_pool = NULL;
          }

          return res;
        }

        if (MODRET_ISERROR(mr)) {
          res = MODRET_ERROR(mr);

          if (cmd->tmp_pool) {
            destroy_pool(cmd->tmp_pool);
            cmd->tmp_pool = NULL;
          }

          return res;
        }

        m = NULL;
      }
    }
  }

  if (auth_tab) {

    /* Fetch the specific module to be used for authenticating this user. */
    void *v = pr_table_get(auth_tab, name, NULL);
    if (v) {
      m = *((module **) v);

      pr_trace_msg(trace_channel, 4,
        "using module 'mod_%s.c' from authcache to authenticate user '%s'",
        m->name, name);
    }
  }

  mr = dispatch_auth(cmd, "check", m ? &m : NULL);

  if (MODRET_ISHANDLED(mr))
    res = MODRET_HASDATA(mr) ? PR_AUTH_RFC2228_OK : PR_AUTH_OK;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

int pr_auth_requires_pass(pool *p, const char *name) {
  cmd_rec *cmd;
  modret_t *mr;
  int res = TRUE;

  cmd = make_cmd(p, 1, name);
  mr = dispatch_auth(cmd, "requires_pass", NULL);

  if (MODRET_ISHANDLED(mr))
    res = FALSE;

  else if (MODRET_ISERROR(mr))
    res = MODRET_ERROR(mr);

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

const char *pr_auth_uid2name(pool *p, uid_t uid) {
  static char namebuf[64];
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  char *res = NULL;
  int have_name = FALSE;

  memset(namebuf, '\0', sizeof(namebuf));

  uidcache_create();

  if (uid_tab) {
    void *v = NULL;

    v = pr_table_kget(uid_tab, (const void *) &uid, sizeof(uid_t), NULL);
    if (v) {
      sstrncpy(namebuf, v, sizeof(namebuf));

      pr_trace_msg(trace_channel, 8,
        "using name '%s' from uidcache for UID %lu", namebuf,
        (unsigned long) uid);
 
      res = namebuf;
      return res;

    } else {
      pr_trace_msg(trace_channel, 9,
        "no value found in uidcache for UID %lu: %s", (unsigned long) uid,
        strerror(errno));
    }
  }

  cmd = make_cmd(p, 1, (void *) &uid);
  mr = dispatch_auth(cmd, "uid2name", NULL);

  if (MODRET_ISHANDLED(mr) &&
      MODRET_HASDATA(mr)) {
    res = mr->data;
    sstrncpy(namebuf, res, sizeof(namebuf));
    res = namebuf;

    uidcache_add(uid, res);
    have_name = TRUE;
  }

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  if (!have_name) {
    snprintf(namebuf, sizeof(namebuf)-1, "%lu", (unsigned long) uid);
  }

  res = namebuf;
  return res;
}

const char *pr_auth_gid2name(pool *p, gid_t gid) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  static char namebuf[64];
  char *res = NULL;
  int have_name = FALSE;

  memset(namebuf, '\0', sizeof(namebuf));

  gidcache_create();

  if (gid_tab) {
    void *v = NULL;
 
    v = pr_table_kget(gid_tab, (const void *) &gid, sizeof(gid_t), NULL);
    if (v) {
      sstrncpy(namebuf, v, sizeof(namebuf));

      pr_trace_msg(trace_channel, 8,
        "using name '%s' from gidcache for GID %lu", namebuf,
        (unsigned long) gid);

      res = namebuf;
      return res;

    } else {
      pr_trace_msg(trace_channel, 9,
        "no value found in gidcache for GID %lu: %s", (unsigned long) gid,
        strerror(errno));
    }
  }

  cmd = make_cmd(p, 1, (void *) &gid);
  mr = dispatch_auth(cmd, "gid2name", NULL);

  if (MODRET_ISHANDLED(mr) &&
      MODRET_HASDATA(mr)) {
    res = mr->data;
    sstrncpy(namebuf, res, sizeof(namebuf));
    res = namebuf;

    gidcache_add(gid, res);
    have_name = TRUE;
  }

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  if (!have_name) {
    snprintf(namebuf, sizeof(namebuf)-1, "%lu", (unsigned long) gid);
  }

  res = namebuf;
  return res;
}

uid_t pr_auth_name2uid(pool *p, const char *name) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  uid_t res = (uid_t) -1;

  cmd = make_cmd(p, 1, name);
  mr = dispatch_auth(cmd, "name2uid", NULL);

  if (MODRET_ISHANDLED(mr))
    res = *((uid_t *) mr->data);
  else
    errno = EINVAL;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

gid_t pr_auth_name2gid(pool *p, const char *name) {
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  gid_t res = (gid_t) -1;

  cmd = make_cmd(p, 1, name);
  mr = dispatch_auth(cmd, "name2gid", NULL);

  if (MODRET_ISHANDLED(mr))
    res = *((gid_t *) mr->data);
  else
    errno = EINVAL;

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

int pr_auth_getgroups(pool *p, const char *name, array_header **group_ids,
    array_header **group_names) {

  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;
  int res = -1;

  /* Allocate memory for the array_headers of GIDs and group names. */
  if (group_ids)
    *group_ids = make_array(permanent_pool, 2, sizeof(gid_t));

  if (group_names)
    *group_names = make_array(permanent_pool, 2, sizeof(char *));

  cmd = make_cmd(p, 3, name, group_ids ? *group_ids : NULL,
    group_names ? *group_names : NULL);

  mr = dispatch_auth(cmd, "getgroups", NULL);

  if (MODRET_ISHANDLED(mr) && MODRET_HASDATA(mr)) {
    res = *((int *) mr->data);

    /* Note: the number of groups returned should, barring error,
     * always be at least 1, as per getgroups(2) behavior.  This one
     * ID is present because it is the primary group membership set in
     * struct passwd, from /etc/passwd.  This will need to be documented
     * for the benefit of auth_getgroup() implementors.
     */

    if (group_ids) {
      register unsigned int i;
      char *strgids = "";
      gid_t *gids = (*group_ids)->elts;

      for (i = 0; i < (*group_ids)->nelts; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf)-1, "%lu", (unsigned long) gids[i]);
        buf[sizeof(buf)-1] = '\0';

        strgids = pstrcat(p, strgids, i != 0 ? ", " : "", buf, NULL);
      }

      pr_log_debug(DEBUG10, "retrieved group %s: %s",
        (*group_ids)->nelts == 1 ? "ID" : "IDs", strgids);
    }

    if (group_names) {
      register unsigned int i;
      char *strgroups = ""; 
      char **groups = (*group_names)->elts;

      for (i = 0; i < (*group_names)->nelts; i++)
        strgroups = pstrcat(p, strgroups, i != 0 ? ", " : "", groups[i], NULL);
    
      pr_log_debug(DEBUG10, "retrieved group %s: %s",
        (*group_names)->nelts == 1 ? "name" : "names", strgroups);
    }
  }

  if (cmd->tmp_pool) {
    destroy_pool(cmd->tmp_pool);
    cmd->tmp_pool = NULL;
  }

  return res;
}

/* This is one messy function.  Yuck.  Yay legacy code. */
config_rec *pr_auth_get_anon_config(pool *p, char **login_name,
    char **user_name, char **anon_name) {
  config_rec *c = NULL, *topc = NULL, *anon_c = NULL;
  char *config_user_name, *config_anon_name = NULL;
  unsigned char is_alias = FALSE, *auth_alias_only = NULL;

  /* Precendence rules:
   *   1. Search for UserAlias directive.
   *   2. Search for Anonymous directive.
   *   3. Normal user login
   */

  config_user_name = get_param_ptr(main_server->conf, "UserName", FALSE);
  if (config_user_name &&
      user_name) {
    *user_name = config_user_name;
  }

  /* If the main_server->conf->set list is large (e.g. there are many
   * config_recs in the list, as can happen if MANY <Directory> sections are
   * configured), the login can timeout because this find_config() call takes
   * a long time.  The reason this issue strikes HERE first in the login
   * process is that this appears to the first find_config() call which has
   * a TRUE recurse flag.
   *
   * The find_config() call below is looking for a UserAlias directive
   * anywhere in the configuration, no matter how deeply buried in nested
   * config contexts it might be.
   */

  c = find_config(main_server->conf, CONF_PARAM, "UserAlias", TRUE);
  if (c) {
    do {
      pr_signals_handle();

      if (strcmp(c->argv[0], "*") == 0 ||
          strcmp(c->argv[0], *login_name) == 0) {
        is_alias = TRUE;
        break;
      }

    } while ((c = find_config_next(c, c->next, CONF_PARAM, "UserAlias",
      TRUE)) != NULL);
  }

  /* This is where things get messy, rapidly. */
  topc = c;

  while (c && c->parent &&
    (auth_alias_only = get_param_ptr(c->parent->set, "AuthAliasOnly", FALSE))) {

    /* while() loops should always handle signals. */
    pr_signals_handle();

    if (auth_alias_only) {
      /* If AuthAliasOnly is on, ignore this one and continue. */
      if (*auth_alias_only == TRUE) {
        c = find_config_next(c, c->next, CONF_PARAM, "UserAlias", TRUE);
        continue;
      }
    }

    /* At this point, we have found an "AuthAliasOnly off" config in
     * c->parent->set.  See if there's a UserAlias in the same config set.
     */

    is_alias = FALSE;

    find_config_set_top(topc);
    c = find_config_next(c, c->next, CONF_PARAM, "UserAlias", TRUE);

    if (c &&
        (strcmp(c->argv[0], "*") == 0 ||
         strcmp(c->argv[0], *login_name) == 0)) {
      is_alias = TRUE;
    }
  }

  if (c) {
    *login_name = c->argv[1];

    /* If the alias is applied inside an <Anonymous> context, we have found
     * our anon block.
     */
    if (c->parent &&
        c->parent->config_type == CONF_ANON) {
      c = c->parent;

    } else {
      c = NULL;
    }
  }

  /* Next, search for an anonymous entry. */

  if (c == NULL) {
    c = find_config(main_server->conf, CONF_ANON, NULL, FALSE);

  } else {
    find_config_set_top(c);
    anon_c = c;
  }

  if (c) {
    do {
      pr_signals_handle();

      config_anon_name = get_param_ptr(c->subset, "UserName", FALSE);

      if (!config_anon_name)
        config_anon_name = config_user_name;

      if (config_anon_name &&
          strcmp(config_anon_name, *login_name) == 0) {
         if (anon_name)
           *anon_name = config_anon_name;
         break;
      }
 
    } while ((c = find_config_next(c, c->next, CONF_ANON, NULL,
      FALSE)) != NULL);
  }

  if (!is_alias) {
    auth_alias_only = get_param_ptr(anon_c ? anon_c->subset : main_server->conf,
      "AuthAliasOnly", FALSE);

    if (auth_alias_only &&
        *auth_alias_only == TRUE) {
      if (anon_c &&
          anon_c->config_type == CONF_ANON) {
        anon_c = NULL;

      } else {
        *login_name = NULL;
      }

      auth_alias_only = get_param_ptr(main_server->conf, "AuthAliasOnly",
        FALSE);
      if (*login_name &&
          auth_alias_only &&
          *auth_alias_only == TRUE)
        *login_name = NULL;

      if ((!login_name || !anon_c) &&
          anon_name) {
        *anon_name = NULL;
      }
    }
  }

  return c;
}

int pr_auth_banned_by_ftpusers(xaset_t *ctx, const char *user) {
  int res = FALSE;
  unsigned char *use_ftp_users;

  use_ftp_users = get_param_ptr(ctx, "UseFtpUsers", FALSE);

  if (use_ftp_users == NULL ||
      *use_ftp_users == TRUE) {
    FILE *fh = NULL;
    char buf[256];

    PRIVS_ROOT
    fh = fopen(PR_FTPUSERS_PATH, "r");
    PRIVS_RELINQUISH

    if (fh == NULL)
      return res;

    memset(buf, '\0', sizeof(buf));

    while (fgets(buf, sizeof(buf)-1, fh)) {
      char *ptr;

      pr_signals_handle();

      buf[sizeof(buf)-1] = '\0';
      CHOP(buf);

      ptr = buf;
      while (isspace((int) *ptr) && *ptr) {
        ptr++;
      }

      if (!*ptr ||
          *ptr == '#') {
        continue;
      }

      if (strcmp(ptr, user) == 0 ) {
        res = TRUE;
        break;
      }

      memset(buf, '\0', sizeof(buf));
    }

    fclose(fh);
  }

  return res;
}

int pr_auth_is_valid_shell(xaset_t *ctx, const char *shell) {
  int res = TRUE;
  unsigned char *require_valid_shell;

  if (shell == NULL)
    return res;

  require_valid_shell = get_param_ptr(ctx, "RequireValidShell", FALSE);

  if (require_valid_shell == NULL ||
      *require_valid_shell == TRUE) {
    FILE *fh = NULL;
    char buf[256];

    fh = fopen(PR_VALID_SHELL_PATH, "r");
    if (fh == NULL)
      return res;

    res = FALSE;
    memset(buf, '\0', sizeof(buf));

    while (fgets(buf, sizeof(buf)-1, fh)) {
      pr_signals_handle();

      buf[sizeof(buf)-1] = '\0';
      CHOP(buf);

      if (strcmp(buf, shell) == 0) {
        res = TRUE;
        break;
      }

      memset(buf, '\0', sizeof(buf));
    }

    fclose(fh);
  }

  return res;
}

int pr_auth_chroot(const char *path) {
  int res;

#if defined(HAVE_SETENV) && defined(__GLIBC__) && defined(__GLIBC_MINOR__) && \
  __GLIBC__ == 2 && __GLIBC_MINOR__ >= 3
  char *tz;

  tz = pr_env_get(session.pool, "TZ"); 
  if (tz == NULL) {
    if (pr_env_set(session.pool, "TZ", pstrdup(permanent_pool,
        tzname[0])) < 0) { 
      pr_log_debug(DEBUG0, "error setting TZ environment variable to " 
        "'%s': %s", tzname[0], strerror(errno));

    } else {
      pr_log_debug(DEBUG10, "set TZ environment variable to '%s'", tzname[0]);
    }
  }
#endif

  pr_log_pri(PR_LOG_INFO, "Preparing to chroot to directory '%s'", path);

  PRIVS_ROOT
  res = pr_fsio_chroot(path);
  PRIVS_RELINQUISH

  if (res < 0) {
    pr_log_pri(PR_LOG_ERR, "chroot to '%s' failed for user '%s': %s", path,
      session.user, strerror(errno));
    return -1;
  }

  pr_log_debug(DEBUG1, "Environment successfully chroot()ed");
  return 0;
}

int set_groups(pool *p, gid_t primary_gid, array_header *suppl_gids) {
  int res = 0;
  pool *tmp_pool = NULL;

#ifdef HAVE_SETGROUPS
  register unsigned int i = 0;
  gid_t *gids = NULL, *proc_gids = NULL;
  size_t ngids = 0, nproc_gids = 0;
  char *strgids = "";

  /* sanity check */
  if (!p || !suppl_gids)
    return 0;

  tmp_pool = make_sub_pool(p);
  pr_pool_tag(tmp_pool, "set_groups() tmp pool");

  /* Check for a NULL supplemental group ID list. */
  if (suppl_gids) {
    ngids = suppl_gids->nelts;
    gids = suppl_gids->elts;

    if (ngids && gids) {
      proc_gids = pcalloc(tmp_pool, sizeof(gid_t) * (ngids));

      /* Note: the list of supplemental GIDs may contain duplicates.  Sort
       * through the list and keep only the unique IDs - this should help avoid
       * running into the NGROUPS limit when possible.  This algorithm may slow
       * things down some; optimize it if/when possible.
       */
      proc_gids[nproc_gids++] = gids[0];
    }
  }

  for (i = 1; i < ngids; i++) {
    register unsigned int j = 0;
    unsigned char skip_gid = FALSE;

    /* This duplicate ID search only needs to be done after the first GID
     * in the given list is examined, as the first GID cannot be a duplicate.
     */
    for (j = 0; j < nproc_gids; j++) {
      if (proc_gids[j] == gids[i]) {
        skip_gid = TRUE;
        break;
      }
    }

    if (!skip_gid)
      proc_gids[nproc_gids++] = gids[i];
  }

  for (i = 0; i < nproc_gids; i++) {
    char buf[64];
    snprintf(buf, sizeof(buf)-1, "%lu", (unsigned long) proc_gids[i]);
    buf[sizeof(buf)-1] = '\0';

    strgids = pstrcat(p, strgids, i != 0 ? ", " : "", buf, NULL);
  }

  pr_log_debug(DEBUG10, "setting group %s: %s", nproc_gids == 1 ? "ID" : "IDs",
    strgids);

  /* Set the supplemental groups. */
  res = setgroups(nproc_gids, proc_gids);
  if (res < 0) {
    destroy_pool(tmp_pool);
    return res;
  }
#endif /* !HAVE_SETGROUPS */

#ifndef PR_DEVEL_COREDUMP
  /* Set the primary GID of the process.
   */
  res = setgid(primary_gid);
  if (res < 0) {
    if (tmp_pool)
      destroy_pool(tmp_pool);
    return res;
  }
#endif /* PR_DEVEL_COREDUMP */

  if (tmp_pool)
    destroy_pool(tmp_pool);

  return res;
}

int pr_auth_cache_set(int bool, unsigned int flags) {
  if (bool != 0 &&
      bool != 1) {
    errno = EINVAL;
    return -1;
  }

  if (bool == 0) {
    if (flags & PR_AUTH_CACHE_FL_UID2NAME) {
      auth_caching &= ~PR_AUTH_CACHE_FL_UID2NAME;
      pr_trace_msg(trace_channel, 7, "UID-to-name caching (uidcache) disabled");
    }

    if (flags & PR_AUTH_CACHE_FL_GID2NAME) {
      auth_caching &= ~PR_AUTH_CACHE_FL_GID2NAME;
      pr_trace_msg(trace_channel, 7, "GID-to-name caching (gidcache) disabled");
    }

    if (flags & PR_AUTH_CACHE_FL_AUTH_MODULE) {
      auth_caching &= ~PR_AUTH_CACHE_FL_AUTH_MODULE;
      pr_trace_msg(trace_channel, 7,
        "auth module caching (authcache) disabled");
    }
  }

  if (bool == 1) {
    if (flags & PR_AUTH_CACHE_FL_UID2NAME) {
      auth_caching |= PR_AUTH_CACHE_FL_UID2NAME;
      pr_trace_msg(trace_channel, 7, "UID-to-name caching (uidcache) enabled");
    }

    if (flags & PR_AUTH_CACHE_FL_GID2NAME) {
      auth_caching |= PR_AUTH_CACHE_FL_GID2NAME;
      pr_trace_msg(trace_channel, 7, "GID-to-name caching (gidcache) enabled");
    }

    if (flags & PR_AUTH_CACHE_FL_AUTH_MODULE) {
      auth_caching &= ~PR_AUTH_CACHE_FL_AUTH_MODULE;
      pr_trace_msg(trace_channel, 7, "auth module caching (authcache) enabled");
    }
  }

  return 0;
}

int pr_auth_add_auth_only_module(const char *name) {
  struct auth_module_elt *elt = NULL;

  if (!name) {
    errno = EINVAL;
    return -1;
  }

  if (!auth_pool) {
    auth_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(auth_pool, "Auth API");
  }

  if (!(auth_caching & PR_AUTH_CACHE_FL_AUTH_MODULE)) {
    /* We won't be using the auth-only module cache, so there's no need to
     * accept this.
     */
    pr_trace_msg(trace_channel, 9, "not adding '%s' to the auth-only list: "
      "caching of auth-only modules disabled", name);
    return 0;
  }

  if (auth_module_list == NULL) {
    auth_module_list = xaset_create(auth_pool, NULL);
  }

  /* Prevent duplicates; they could lead to a memory leak. */
  for (elt = (struct auth_module_elt *) auth_module_list->xas_list; elt;
      elt = elt->next) {
    if (strcmp(elt->name, name) == 0) {
      errno = EEXIST;
      return -1;
    }
  }

  elt = pcalloc(auth_pool, sizeof(struct auth_module_elt));
  elt->name = pstrdup(auth_pool, name);

  if (xaset_insert_end(auth_module_list, (xasetmember_t *) elt) < 0) {
    pr_trace_msg(trace_channel, 1, "error adding '%s' to auth-only "
      "module set: %s", name, strerror(errno));
    return -1;
  }

  pr_trace_msg(trace_channel, 5, "added '%s' to auth-only module list", name);
  return 0;
}

int pr_auth_clear_auth_only_modules(void) {
  if (auth_module_list == NULL) {
    errno = EINVAL;
    return -1;
  }

  auth_module_list = NULL;
  pr_trace_msg(trace_channel, 5, "cleared auth-only module list");
  return 0;
}

int pr_auth_remove_auth_only_module(const char *name) {
  struct auth_module_elt *elt = NULL;

  if (!name) {
    errno = EINVAL;
    return -1;
  }

  if (!(auth_caching & PR_AUTH_CACHE_FL_AUTH_MODULE)) {
    /* We won't be using the auth-only module cache, so there's no need to
     * accept this.
     */
    pr_trace_msg(trace_channel, 9, "not removing '%s' to the auth-only list: "
      "caching of auth-only modules disabled", name);
    return 0;
  }

  if (auth_module_list == NULL) {
    pr_trace_msg(trace_channel, 9, "not removing '%s' from list: "
      "empty auth-only module list", name);
    errno = ENOENT;
    return -1;
  }

  for (elt = (struct auth_module_elt *) auth_module_list->xas_list; elt;
      elt = elt->next) {
    if (strcmp(elt->name, name) == 0) {
      if (xaset_remove(auth_module_list, (xasetmember_t *) elt) < 0) {
        pr_trace_msg(trace_channel, 1, "error removing '%s' from auth-only "
          "module set: %s", name, strerror(errno));
        return -1;
      }

      pr_trace_msg(trace_channel, 5, "removed '%s' from auth-only module list",
        name);
      return 0;
    }
  }

  errno = ENOENT;
  return -1;
}

char *pr_auth_get_home(pool *p, char *pw_dir) {
  config_rec *c;
  char *home_dir;

  home_dir = pw_dir;

  c = find_config(main_server->conf, CONF_PARAM, "RewriteHome", FALSE);
  if (c == NULL)
    return home_dir;

  if (*((int *) c->argv[0]) == FALSE)
    return home_dir;

  /* Rather than using a cmd_rec dispatched to mod_rewrite's PRE_CMD handler,
   * we use an approach with looser coupling to mod_rewrite: stash the
   * home directory in the session.notes table, and generate an event.
   * The mod_rewrite module will listen for this event, rewrite the stashed
   * home directory as necessary, and be done.
   *
   * Thus after the event has been generated, we retrieve (and remove) the
   * (possibly rewritten) home directory from the session.notes table.
   * This approach means that other modules which wish to get involved
   * in the rewriting of the home directory can also do so.
   */

  (void) pr_table_remove(session.notes, "mod_auth.home-dir", NULL);
  if (pr_table_add(session.notes, "mod_auth.home-dir",
      pstrdup(p, pw_dir), 0) < 0) {
    pr_trace_msg(trace_channel, 3,
      "error stashing home dir in session.notes: %s", strerror(errno));
    return home_dir;
  }

  pr_event_generate("mod_auth.rewrite-home", NULL);

  home_dir = pr_table_get(session.notes, "mod_auth.home-dir", NULL);
  if (home_dir == NULL) {
    pr_trace_msg(trace_channel, 3,
      "error getting home dir from session.notes: %s", strerror(errno));
    return pw_dir;
  }

  (void) pr_table_remove(session.notes, "mod_auth.home-dir", NULL);

  pr_log_debug(DEBUG9, "returning rewritten home directory '%s' for original "
    "home directory '%s'", home_dir, pw_dir);
  pr_trace_msg(trace_channel, 9, "returning rewritten home directory '%s' "
    "for original home directory '%s'", home_dir, pw_dir);

  return home_dir;
}

/* Internal use only.  To be called in the session process. */
int init_auth(void) {
  if (!auth_pool) {
    auth_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(auth_pool, "Auth API");
  }

  return 0;
}
