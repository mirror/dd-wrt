/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2004-2010 The ProFTPD Project team
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

/* Table API implementation
 * $Id: table.c,v 1.16 2010/02/14 00:36:18 castaglia Exp $
 */

#include "conf.h"

#define PR_TABLE_DEFAULT_NCHAINS	32
#define PR_TABLE_ENT_POOL_SIZE		64

struct table_rec {
  pool *pool;
  unsigned long flags;

  pr_table_entry_t **chains;
  unsigned int nchains;
  unsigned int nents;

  /* List of free structures. */
  pr_table_entry_t *free_ents;
  pr_table_key_t *free_keys;

  /* For iterating over all the keys in the entire table. */
  pr_table_entry_t *tab_iter_ent;

  /* For iterating through all of the possible multiple values for a single
   * key.  Only used if the PR_TABLE_FL_MULTI_VALUE flag is set.
   */
  pr_table_entry_t *val_iter_ent;

  /* Cache of last looked-up entry.  Usage of this field can be enabled
   * by using the PR_TABLE_FL_USE_CACHE flag.
   */
  pr_table_entry_t *cache_ent;

  /* Table callbacks. */
  int (*keycmp)(const void *, size_t, const void *, size_t);
  unsigned int (*keyhash)(const void *, size_t);
  void (*entinsert)(pr_table_entry_t **, pr_table_entry_t *);
  void (*entremove)(pr_table_entry_t **, pr_table_entry_t *);
};

static int handling_signal = FALSE;

/* Default table callbacks
 */

static int key_cmp(const void *key1, size_t keysz1, const void *key2,
    size_t keysz2) {
  return strcmp((const char *) key1, (const char *) key2);
}

/* Use Perl's hashing algorithm by default. */
static unsigned int key_hash(const void *key, size_t keysz) {
  unsigned int i = 0;
  size_t sz = !keysz ? strlen((const char *) key) : keysz;

  while (sz--) {
    const char *k = key;
    unsigned int c = *k;
    k++;

    if (!handling_signal) {
      /* Always handle signals in potentially long-running while loops. */
      pr_signals_handle();
    }

    i = (i * 33) + c;
  }

  return i; 
}

/* Default insertion is simply to add the given entry to the end of the
 * chain.
 */
static void entry_insert(pr_table_entry_t **h, pr_table_entry_t *e) {
  pr_table_entry_t *ei;

  for (ei = *h; ei && ei->next; ei = ei->next);

  /* Now, ei points to the last entry in the chain. */
  ei->next = e;
  e->prev = ei;
}

/* Default removal is simply to remove the entry from the chain. */
static void entry_remove(pr_table_entry_t **h, pr_table_entry_t *e) {

  if (e->next)
    e->next->prev = e->prev;

  if (e->prev)
    e->prev->next = e->next;

  if (e == *h &&
      e->next == NULL)
    /* This entry is the head, and is the only entry in this chain. */
    *h = NULL;

  else 
    *h = e->next;

  e->prev = e->next = NULL;
  return;
}

/* Table key management
 */

static pr_table_key_t *tab_key_alloc(pr_table_t *tab) {
  pr_table_key_t *k;

  /* Try to find a free key on the free list first... */
  if (tab->free_keys) {
    k = tab->free_keys;
    tab->free_keys = k->next;
    k->next = NULL;

    return k;
  }

  /* ...otherwise, allocate a new key. */
  k = pcalloc(tab->pool, sizeof(pr_table_key_t));

  return k;
}

static void tab_key_free(pr_table_t *tab, pr_table_key_t *k) {
  /* Clear everything from the given key. */
  memset(k, 0, sizeof(pr_table_key_t));

  /* Add this key to the table's free list. */
  if (tab->free_keys) {
    pr_table_key_t *i = tab->free_keys;

    /* Scan to the end of the list. */
    while (i->next != NULL) {
      if (!handling_signal) {
        pr_signals_handle();
      }

      i = i->next;
    }

    i->next = k;

  } else
    tab->free_keys = k;
}

/* Table entry management
 */

static pr_table_entry_t *tab_entry_alloc(pr_table_t *tab) {
  pr_table_entry_t *e;

  /* Try to find a free entry on the free list first... */
  if (tab->free_ents) {
    e = tab->free_ents;
    tab->free_ents = e->next;
    e->next = NULL;

    return e;
  }

  /* ...otherwise, allocate a new entry. */
  e = pcalloc(tab->pool, sizeof(pr_table_entry_t));

  return e;
}

static void tab_entry_free(pr_table_t *tab, pr_table_entry_t *e) {
  /* Clear everything from the given entry. */
  memset(e, 0, sizeof(pr_table_entry_t));

  /* Add this entry to the table's free list. */
  if (tab->free_ents) {
    pr_table_entry_t *i = tab->free_ents;

    /* Scan to the end of the list. */
    while (i->next != NULL) {
      if (!handling_signal) {
        pr_signals_handle();
      }

      i = i->next;
    }

    i->next = e;
 
  } else
    tab->free_ents = e;
}

static void tab_entry_insert(pr_table_t *tab, pr_table_entry_t *e) {
  pr_table_entry_t *h = tab->chains[e->idx];

  if (h &&
      h != e) {

    /* Only insert the entry if the head we found is different from the
     * the given entry.  There is an edge case when the entry being added
     * is the head of a new chain.
     */
    tab->entinsert(&h, e);
    tab->chains[e->idx] = h;
  }

  e->key->nents++;
  tab->nents++;
}

static pr_table_entry_t *tab_entry_next(pr_table_t *tab) {
  pr_table_entry_t *ent = NULL;

  if (tab->tab_iter_ent) {
    ent = tab->tab_iter_ent->next;

    if (!ent) {
      register unsigned int i;

      /* Reset ent to be NULL, so that if we don't find a populated chain,
       * we properly return NULL to the caller.
       */
      ent = NULL;

      /* Skip to the next populated chain. */
      for (i = tab->tab_iter_ent->idx + 1; i < tab->nchains; i++) {
        if (tab->chains[i]) {
          ent = tab->chains[i];
          break;
        }
      }
    }

  } else {
    register unsigned int i;

    /* Find the first non-empty chain. */
    for (i = 0; i < tab->nchains; i++) {
      if (tab->chains[i]) {
        ent = tab->chains[i];
        break;
      }
    }
  }

  tab->tab_iter_ent = ent;
  return ent;
}

static void tab_entry_remove(pr_table_t *tab, pr_table_entry_t *e) {
  pr_table_entry_t *h = tab->chains[e->idx];

  tab->entremove(&h, e);
  tab->chains[e->idx] = h;
  e->key->nents--;

  if (e->key->nents == 0) {
    tab_key_free(tab, e->key);
    e->key = NULL;
  }

  tab->nents--;
}

/* Public Table API
 */

int pr_table_kadd(pr_table_t *tab, const void *key_data, size_t key_datasz,
    void *value_data, size_t value_datasz) {
  unsigned int h, idx;
  pr_table_entry_t *e, *n;

  if (tab == NULL ||
      key_data == NULL ||
      (value_datasz > 0 && value_data == NULL)) {
    errno = EINVAL;
    return -1;
  }

  h = tab->keyhash(key_data, key_datasz);

  /* The index of the chain to use is the hash value modulo the number
   * of chains.
   */
  idx = h % tab->nchains;

  /* Allocate a new entry for the given values. */
  n = tab_entry_alloc(tab);
  n->value_data = value_data;
  n->value_datasz = value_datasz;
  n->idx = idx;

  /* Find the current chain entry at this index. */
  e = tab->chains[idx];

  if (e) {
    pr_table_entry_t *ei;

    /* There is a chain at this index.  Next step is to see if any entry
     * on this chain has the exact same key.  If so, increase the entry ref
     * count on that key, otherwise, create a new key.
     */

    for (ei = e; ei; ei = ei->next) {
      if (ei->key->hash != h)
        continue;

      /* Hash collision.  Now check if the key data that was hashed
       * is identical.  If so, we have multiple values for the same key.
       */

      if (tab->keycmp(ei->key->key_data, 0, key_data, 0) == 0) {

        /* Check if this table allows multivalues. */
        if (!(tab->flags & PR_TABLE_FL_MULTI_VALUE)) {
          errno = EEXIST;
          return -1;

        } else
          n->key = ei->key;
      }
    }

  } else {

    /* This new entry becomes the head of this chain. */
    tab->chains[idx] = n;
  }

  if (!n->key) {
    pr_table_key_t *k;

    /* Allocate a new key. */
    k = tab_key_alloc(tab);

    k->key_data = (void *) key_data;
    k->key_datasz = key_datasz;
    k->hash = h;
    k->nents = 0;

    n->key = k;
  }

  tab_entry_insert(tab, n);
  return 0;
}

int pr_table_kexists(pr_table_t *tab, const void *key_data, size_t key_datasz) {
  unsigned int h, idx;
  pr_table_entry_t *head, *ent;

  if (!tab || !key_data) {
    errno = EINVAL;
    return -1;
  }

  if (tab->nents == 0) {
    errno = ENOENT;
    return -1;
  }

  if (tab->flags & PR_TABLE_FL_USE_CACHE) {
    /* Has the caller already wanted to lookup this same key previously?
     * If so, reuse that lookup if we can.  In this case, "same key" means
     * the _exact same pointer_, not identical data.
     */
    if (tab->cache_ent &&
        tab->cache_ent->key->key_data == key_data)
      return tab->cache_ent->key->nents;
  }

  h = tab->keyhash(key_data, key_datasz);
  idx = h % tab->nchains;

  head = tab->chains[idx];

  if (!head) {
    tab->cache_ent = NULL;
    return 0;
  }

  for (ent = head; ent; ent = ent->next) {
    if (ent->key == NULL ||
        ent->key->hash != h)
      continue;

    /* Matching hashes.  Now to see if the keys themselves match. */
    if (tab->keycmp(ent->key->key_data, ent->key->key_datasz,
        key_data, key_datasz) == 0) {

      if (tab->flags & PR_TABLE_FL_USE_CACHE)
        tab->cache_ent = ent;

      return ent->key->nents;
    }
  }

  tab->cache_ent = NULL;

  errno = EINVAL;
  return 0;
}

void *pr_table_kget(pr_table_t *tab, const void *key_data, size_t key_datasz,
    size_t *value_datasz) {
  unsigned int h;
  pr_table_entry_t *head, *ent;

  if (!tab) {
    errno = EINVAL;
    return NULL;
  }

  /* Use a NULL key as a way of rewinding the per-key lookup. */
  if (!key_data) {
    tab->cache_ent = NULL;
    tab->val_iter_ent = NULL;

    errno = ENOENT;
    return NULL;
  }

  if (tab->nents == 0) {
    tab->cache_ent = NULL;
    tab->val_iter_ent = NULL;

    errno = ENOENT;
    return NULL;
  }

  h = tab->keyhash(key_data, key_datasz);

  /* Has the caller already looked up this same key previously?
   * If so, continue the lookup where we left off.  In this case,
   * "same key" means the _exact same pointer_, not identical data.
   */
  if (tab->val_iter_ent &&
      tab->val_iter_ent->key->key_data == key_data) {
    head = tab->val_iter_ent->next;

  } else if ((tab->flags & PR_TABLE_FL_USE_CACHE) &&
             tab->cache_ent &&
             tab->cache_ent->key->key_data == key_data) {

     /* If the cached lookup entry matches, we'll use it. */
     head = tab->cache_ent->next;

  } else {
    unsigned int idx = h % tab->nchains;
    head = tab->chains[idx];
  }

  if (!head) {
    tab->cache_ent = NULL;
    tab->val_iter_ent = NULL;

    errno = ENOENT;
    return NULL;
  }

  for (ent = head; ent; ent = ent->next) {
    if (ent->key == NULL ||
        ent->key->hash != h)
      continue;

    /* Matching hashes.  Now to see if the keys themselves match. */
    if (tab->keycmp(ent->key->key_data, ent->key->key_datasz,
        key_data, key_datasz) == 0) {

      if (tab->flags & PR_TABLE_FL_USE_CACHE) 
        tab->cache_ent = ent;

      if (tab->flags & PR_TABLE_FL_MULTI_VALUE)
        tab->val_iter_ent = ent;

      if (value_datasz)
        *value_datasz = ent->value_datasz;

      return ent->value_data;
    }
  }

  tab->cache_ent = NULL;
  tab->val_iter_ent = NULL;

  errno = ENOENT;
  return NULL;
}

void *pr_table_kremove(pr_table_t *tab, const void *key_data,
    size_t key_datasz, size_t *value_datasz) {
  unsigned int h, idx;
  pr_table_entry_t *head, *ent;

  if (!tab || !key_data) {
    errno = EINVAL;
    return NULL;
  }

  if (tab->nents == 0) {
    errno = ENOENT;
    return NULL;
  }

  /* Has the caller already wanted to lookup this same key previously?
   * If so, reuse that lookup if we can.  In this case, "same key" means
   * the _exact same pointer_, not identical data.
   */
  if ((tab->flags & PR_TABLE_FL_USE_CACHE) &&
      tab->cache_ent &&
      tab->cache_ent->key->key_data == key_data) {
    void *value_data = tab->cache_ent->value_data;

    if (value_datasz)
      *value_datasz = tab->cache_ent->value_datasz;

    tab_entry_remove(tab, tab->cache_ent);
    tab_entry_free(tab, tab->cache_ent);
    tab->cache_ent = NULL;

    return value_data;
  }

  h = tab->keyhash(key_data, key_datasz);
  idx = h % tab->nchains;

  head = tab->chains[idx];

  if (!head) {
    tab->cache_ent = NULL;

    errno = ENOENT;
    return NULL;
  }

  for (ent = head; ent; ent = ent->next) {
    if (ent->key == NULL ||
        ent->key->hash != h)
      continue;

    /* Matching hashes.  Now to see if the keys themselves match. */
    if (tab->keycmp(ent->key->key_data, ent->key->key_datasz,
        key_data, key_datasz) == 0) {
      void *value_data = ent->value_data;

      if (value_datasz)
        *value_datasz = ent->value_datasz;

      tab_entry_remove(tab, ent);
      tab_entry_free(tab, ent);
      tab->cache_ent = NULL;

      return value_data;
    }
  }

  tab->cache_ent = NULL;

  errno = EINVAL;
  return NULL;
}

int pr_table_kset(pr_table_t *tab, const void *key_data, size_t key_datasz,
    void *value_data, size_t value_datasz) {
  unsigned int h;
  pr_table_entry_t *head, *ent;

  /* XXX Should callers be allowed to set NULL values for keys? */

  if (tab == NULL ||
      key_data == NULL ||
      (value_datasz > 0 && value_data == NULL)) {
    errno = EINVAL;
    return -1;
  }

  if (tab->nents == 0) {
    errno = ENOENT;
    return -1;
  }

  h = tab->keyhash(key_data, key_datasz);

  /* Has the caller already looked up this same key previously?
   * If so, continue the lookup where we left off.  In this case,
   * "same key" means the _exact same pointer_, not identical data.
   */
  if (tab->val_iter_ent &&
      tab->val_iter_ent->key->key_data == key_data) {
    head = tab->val_iter_ent->next;

  } else if ((tab->flags & PR_TABLE_FL_USE_CACHE) &&
             tab->cache_ent &&
             tab->cache_ent->key->key_data == key_data) {

     /* If the cached lookup entry matches, we'll use it. */
     head = tab->cache_ent->next;

  } else {
    unsigned int idx = h % tab->nchains;

    head = tab->chains[idx];
  }

  if (!head) {
    tab->cache_ent = NULL;
    tab->val_iter_ent = NULL;

    errno = ENOENT;
    return -1;
  }

  for (ent = head; ent; ent = ent->next) {
    if (ent->key == NULL ||
        ent->key->hash != h)
      continue;

    /* Matching hashes.  Now to see if the keys themselves match. */
    if (tab->keycmp(ent->key->key_data, ent->key->key_datasz,
        key_data, key_datasz) == 0) {

      if (ent->value_data == value_data) {
        errno = EEXIST;
        return -1;
      }

      ent->value_data = value_data;
      ent->value_datasz = value_datasz;

      if (tab->flags & PR_TABLE_FL_USE_CACHE)
        tab->cache_ent = ent;

      if (tab->flags & PR_TABLE_FL_MULTI_VALUE)
        tab->val_iter_ent = ent;

      return 0;
    }
  }

  tab->cache_ent = NULL;
  tab->val_iter_ent = NULL;

  errno = EINVAL;
  return -1;
}

int pr_table_add(pr_table_t *tab, const char *key_data, void *value_data,
    size_t value_datasz) {

  if (tab == NULL ||
      key_data == NULL) {
    errno = EINVAL;
    return -1;
  }

  if (value_data &&
      value_datasz == 0)
    value_datasz = strlen((char *) value_data) + 1;

  return pr_table_kadd(tab, key_data, strlen(key_data) + 1, value_data,
    value_datasz);
}

int pr_table_add_dup(pr_table_t *tab, const char *key_data, void *value_data,
    size_t value_datasz) {
  void *dup_data;

  if (!tab || !key_data) {
    errno = EINVAL;
    return -1;
  }

  if (!value_data && value_datasz != 0) {
    errno = EINVAL;
    return -1;
  }

  if (value_data && value_datasz == 0)
    value_datasz = strlen((char *) value_data) + 1;

  dup_data = pcalloc(tab->pool, value_datasz);
  memcpy(dup_data, value_data, value_datasz);
 
  return pr_table_add(tab, key_data, dup_data, value_datasz);
}

pr_table_t *pr_table_nalloc(pool *p, int flags, unsigned int nchains) {
  pr_table_t *tab;
  pool *tab_pool;

  if (p == NULL ||
      nchains == 0) {
    errno = EINVAL;
    return NULL;
  }

  tab_pool = make_sub_pool(p);
  pr_pool_tag(tab_pool, "table pool");

  tab = pcalloc(tab_pool, sizeof(pr_table_t));
  tab->pool = tab_pool;
  tab->flags = flags;
  tab->nchains = nchains;
  tab->chains = pcalloc(tab_pool,
    sizeof(pr_table_entry_t *) * tab->nchains);

  tab->keycmp = key_cmp;
  tab->keyhash = key_hash;
  tab->entinsert = entry_insert;
  tab->entremove = entry_remove;

  return tab;
}

pr_table_t *pr_table_alloc(pool *p, int flags) {
  return pr_table_nalloc(p, flags, PR_TABLE_DEFAULT_NCHAINS);
}

int pr_table_count(pr_table_t *tab) {
  if (!tab) {
    errno = EINVAL;
    return -1;
  }

  return tab->nents;
}

int pr_table_do(pr_table_t *tab, int (*cb)(const void *key_data,
    size_t key_datasz, void *value_data, size_t value_datasz, void *user_data),
    void *user_data, int flags) {
  register unsigned int i;

  if (!tab || !cb) {
    errno = EINVAL;
    return -1;
  }

  if (tab->nents == 0)
    return 0;

  for (i = 0; i < tab->nchains; i++) {
    pr_table_entry_t *ent = tab->chains[i];

    while (ent) {
      int res;

      if (!handling_signal) { 
        pr_signals_handle();
      }

      res = cb(ent->key->key_data, ent->key->key_datasz, ent->value_data,
        ent->value_datasz, user_data);
      if (res < 0 &&
          !(flags & PR_TABLE_DO_FL_ALL)) {
        errno = EPERM;
        return -1;
      }

      ent = ent->next;
    }
  }

  return 0;
}

int pr_table_empty(pr_table_t *tab) {
  register unsigned int i;

  if (!tab) {
    errno = EINVAL;
    return -1;
  }

  if (tab->nents == 0)
    return 0;

  for (i = 0; i < tab->nchains; i++) {
    pr_table_entry_t *e = tab->chains[i];

    while (e) {
      if (!handling_signal) {
        pr_signals_handle();
      }

      tab_entry_remove(tab, e);
      tab_entry_free(tab, e);

      e = tab->chains[i];
    }

    tab->chains[i] = NULL;
  }

  return 0;
}

int pr_table_exists(pr_table_t *tab, const char *key_data) {
  if (!tab || !key_data) {
    errno = EINVAL;
    return -1;
  }

  return pr_table_kexists(tab, key_data, strlen(key_data) + 1);
}

int pr_table_free(pr_table_t *tab) {

  if (!tab) {
    errno = EINVAL;
    return -1;
  }

  if (tab->nents != 0) {
    errno = EPERM;
    return -1;
  }

  destroy_pool(tab->pool);
  return 0;
}

void *pr_table_get(pr_table_t *tab, const char *key_data,
    size_t *value_datasz) {
  size_t key_datasz = 0;

  if (!tab) {
    errno = EINVAL;
    return NULL;
  }

  if (key_data)
    key_datasz = strlen(key_data) + 1;  

  return pr_table_kget(tab, key_data, key_datasz, value_datasz);
}

void *pr_table_next(pr_table_t *tab) {
  pr_table_entry_t *ent, *prev;

  if (!tab) {
    errno = EINVAL;
    return NULL;
  }

  prev = tab->tab_iter_ent;

  ent = tab_entry_next(tab);
  while (ent) {
    if (!handling_signal) {
      pr_signals_handle();
    }

    if (prev &&
        ent->key == prev->key) {
      ent = tab_entry_next(tab);
      continue;
    }

    break;
  }

  if (!ent) {
    errno = EPERM;
    return NULL;
  }

  return ent->key->key_data;
}

void *pr_table_remove(pr_table_t *tab, const char *key_data,
    size_t *value_datasz) {

  if (!tab || !key_data) {
    errno = EINVAL;
    return NULL;
  }

  return pr_table_kremove(tab, key_data, strlen(key_data) + 1, value_datasz);
}

int pr_table_rewind(pr_table_t *tab) {
  if (!tab) {
    errno = EINVAL;
    return -1;
  }

  tab->tab_iter_ent = NULL;
  return 0;
}

int pr_table_set(pr_table_t *tab, const char *key_data, void *value_data,
    size_t value_datasz) {

  if (!tab || !key_data) {
    errno = EINVAL;
    return -1;
  }

  if (value_data &&
      value_datasz == 0)
    value_datasz = strlen((char *) value_data) + 1;

  return pr_table_kset(tab, key_data, strlen(key_data) + 1, value_data,
    value_datasz);
}

int pr_table_ctl(pr_table_t *tab, int cmd, void *arg) {

  if (!tab) {
    errno = EINVAL;
    return -1;
  }

  /* Set control operations can only be performed on an empty table. */
  if (tab->nents != 0) {
    errno = EPERM;
    return -1;
  }

  switch (cmd) {
    case PR_TABLE_CTL_SET_KEY_HASH:
      tab->keyhash = arg ?
        (unsigned int (*)(const void *, size_t)) arg :
        (unsigned int (*)(const void *, size_t)) key_hash;
      return 0;

    case PR_TABLE_CTL_SET_FLAGS:
      if (!arg) {
        errno = EINVAL;
        return -1;
      }

      tab->flags = *((unsigned long *) arg);
      return 0;

    case PR_TABLE_CTL_SET_KEY_CMP:
      tab->keycmp = arg ?
        (int (*)(const void *, size_t, const void *, size_t)) arg :
        (int (*)(const void *, size_t, const void *, size_t)) key_cmp;
      return 0;

    case PR_TABLE_CTL_SET_ENT_INSERT:
      tab->entinsert = arg ?
        (void (*)(pr_table_entry_t **, pr_table_entry_t *)) arg :
        (void (*)(pr_table_entry_t **, pr_table_entry_t *)) entry_insert;
      return 0;

    case PR_TABLE_CTL_SET_ENT_REMOVE:
      tab->entremove = arg ?
        (void (*)(pr_table_entry_t **, pr_table_entry_t *)) arg :
        (void (*)(pr_table_entry_t **, pr_table_entry_t *)) entry_remove;
      return 0;

    case PR_TABLE_CTL_SET_NCHAINS: {
      unsigned int new_nchains;

      if (arg == NULL) {
        errno = EINVAL;
        return -1;
      }

      new_nchains = *((unsigned int *) arg);
      if (new_nchains == 0) {
        errno = EINVAL;
        return -1;
      }

      tab->nchains = new_nchains;
      
      /* Note: by not freeing the memory of the previously allocated
       * chains, this constitutes a minor leak of the table's memory pool.
       */
      tab->chains = pcalloc(tab->pool,
        sizeof(pr_table_entry_t *) * tab->nchains);
      return 0;
    }

    default:
      errno = EINVAL;
  }

  return -1;
}

void *pr_table_pcalloc(pr_table_t *tab, size_t sz) {
  if (!tab ||
      sz == 0) {
    errno = EINVAL;
    return NULL;
  }

  return pcalloc(tab->pool, sz);
}

void pr_table_dump(void (*dumpf)(const char *fmt, ...), pr_table_t *tab) {
  register unsigned int i;

  if (tab == NULL ||
      dumpf == NULL)
    return;

  if (tab->flags == 0)
    dumpf("%s", "[table flags]: None");

  else {
    if ((tab->flags & PR_TABLE_FL_MULTI_VALUE) &&
        (tab->flags & PR_TABLE_FL_USE_CACHE)) {
      dumpf("%s", "[table flags]: MultiValue, UseCache");

    } else {
      if (tab->flags & PR_TABLE_FL_MULTI_VALUE)
        dumpf("%s", "[table flags]: MultiValue");

      if (tab->flags & PR_TABLE_FL_USE_CACHE)
        dumpf("%s", "[table flags]: UseCache");
    }
  }

  if (tab->nents == 0) {
    dumpf("[empty table]");
    return;

  } else
    dumpf("[table count]: %u", tab->nents);

  for (i = 0; i < tab->nchains; i++) {
    register unsigned int j = 0;
    pr_table_entry_t *ent = tab->chains[i];

    while (ent) {
      if (!handling_signal) {
        pr_signals_handle();
      }

      dumpf("[chain %u#%u] '%s' => '%s' (%u)", i, j++, ent->key->key_data,
        ent->value_data, ent->value_datasz);
      ent = ent->next;
    }
  }

  return;
}

int table_handling_signal(int bool) {
  if (bool == TRUE ||
      bool == FALSE) {
    handling_signal = bool;
    return 0;
  }

  errno = EINVAL;
  return -1;
}
