/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2006 The ProFTPD Project team
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

/*
 * Resource allocation code
 * $Id: pool.c,v 1.47 2007/01/11 04:09:37 castaglia Exp $
 */

#include "conf.h"

/* Manage free storage blocks */

union align {
  char *cp;
  void (*f)(void);
  long l;
  FILE *fp;
  double d;
};

#define CLICK_SZ (sizeof(union align))

union block_hdr {
  union align a;

  /* Padding */
#if defined(_LP64) || defined(__LP64__)
  char pad[32];
#endif

  /* Actual header */
  struct {
    char *endp;
    union block_hdr *next;
    char *first_avail;
  } h;
};

union block_hdr *block_freelist = NULL;

/* Statistics */
static unsigned int stat_malloc = 0;	/* incr when malloc required */
static unsigned int stat_freehit = 0;	/* incr when freelist used */

/* Lowest level memory allocation functions
 */

static void *null_alloc(size_t size) {
  void *ret = 0;

  if (size == 0)
    ret = malloc(size);
  if (ret == 0) {
    pr_log_pri(PR_LOG_ERR, "fatal: Memory exhausted");
    exit(1);
  }

  return ret;
}

static void *smalloc(size_t size) {
  void *ret;

  ret = malloc(size);
  if (ret == 0)
    ret = null_alloc(size);

  return ret;
}

/* Grab a completely new block from the system pool.  Relies on malloc()
 * to return truly aligned memory.
 */
static union block_hdr *malloc_block(int size) {
  union block_hdr *blok =
    (union block_hdr *) smalloc(size + sizeof(union block_hdr));

  blok->h.next = NULL;
  blok->h.first_avail = (char *) (blok + 1);
  blok->h.endp = size + blok->h.first_avail;

  return blok;
}

static void chk_on_blk_list(union block_hdr *blok, union block_hdr *free_blk) {
  /* Debug code */

  while (free_blk) {
    if (free_blk == blok) {
      pr_log_pri(PR_LOG_ERR, "Fatal: DEBUG: Attempt to free already free block "
       "in chk_on_blk_list()");
      exit(1);
    }

    free_blk = free_blk->h.next;
  }
}

/* Free a chain of blocks -- _must_ call with alarms blocked. */

static void free_blocks(union block_hdr *blok) {
  /* Puts new blocks at head of block list, point next pointer of
   * last block in chain to free blocks we already had.
   */

  union block_hdr *old_free_list = block_freelist;

  if (!blok)
    return;		/* Shouldn't be freeing an empty pool */

  block_freelist = blok;

  /* Adjust first_avail pointers */

  while (blok->h.next) {
    chk_on_blk_list(blok, old_free_list);
    blok->h.first_avail = (char *) (blok + 1);
    blok = blok->h.next;
  }

  chk_on_blk_list(blok, old_free_list);
  blok->h.first_avail = (char *) (blok + 1);
  blok->h.next = old_free_list;
}

/* Get a new block, from the free list if possible, otherwise malloc a new
 * one.  minsz is the requested size of the block to be allocated.
 * If exact is TRUE, then minsz is the exact size of the allocated block;
 * otherwise, the allocated size will be rounded up from minsz to the nearest
 * multiple of BLOCK_MINFREE.
 *
 * Important: BLOCK ALARMS BEFORE CALLING
 */

static union block_hdr *new_block(int minsz, int exact) {
  union block_hdr **lastptr = &block_freelist;
  union block_hdr *blok = block_freelist;

  if (!exact) {
    minsz = 1 + ((minsz - 1) / BLOCK_MINFREE);
    minsz *= BLOCK_MINFREE;
  }

  /* Check if we have anything of the requested size on our free list first...
   */
  while (blok) {
    if (minsz <= blok->h.endp - blok->h.first_avail) {
      *lastptr = blok->h.next;
      blok->h.next = NULL;

      stat_freehit++;
      return blok;

    } else {
      lastptr = &blok->h.next;
      blok = blok->h.next;
    }
  }

  /* Nope...damn.  Have to malloc() a new one. */
  stat_malloc++;
  return malloc_block(minsz);
}

struct cleanup;

static void run_cleanups(struct cleanup *);

/* Pool internal and management */

struct pool {
  union block_hdr *first;
  union block_hdr *last;
  struct cleanup *cleanups;
  struct pool *sub_pools;
  struct pool *sub_next;
  struct pool *sub_prev;
  struct pool *parent;
  char *free_first_avail;
  const char *tag;
};

pool *permanent_pool = NULL;
pool *global_config_pool = NULL;

/* Each pool structure is allocated in the start of it's own first block,
 * so there is a need to know how many bytes that is (once properly
 * aligned).
 */

#define POOL_HDR_CLICKS (1 + ((sizeof(struct pool) - 1) / CLICK_SZ))
#define POOL_HDR_BYTES (POOL_HDR_CLICKS * CLICK_SZ)

#ifdef PR_USE_DEVEL

static unsigned long bytes_in_block_list(union block_hdr *blok) {
  unsigned long size = 0;

  while (blok) {
    size += blok->h.endp - (char *) (blok + 1);
    blok = blok->h.next;
  }

  return size;
}

/* Walk all pools, starting with top level permanent pool, displaying a
 * tree.
 */
static long walk_pools(pool *p, int level,
    void (*debugf)(const char *, ...)) {
  char _levelpad[80] = "";
  long total = 0;

  if (!p)
    return 0;

  if (level > 1) {
    memset(_levelpad, ' ', sizeof(_levelpad)-1);
    if ((level - 1) * 3 >= sizeof(_levelpad))
      _levelpad[sizeof(_levelpad)-1] = 0;
    else
      _levelpad[(level - 1) * 3] = '\0';
  }

  for (; p; p = p->sub_next) {
    total += bytes_in_block_list(p->first);
    if (level == 0)
      debugf("%s (%lu bytes)", p->tag ? p->tag : "[none]",
        bytes_in_block_list(p->first));

    else
      debugf("%s\\- %s (%lu bytes)", _levelpad,
        p->tag ? p->tag : "[none]", bytes_in_block_list(p->first));

    /* Recurse */
    if (p->sub_pools)
      total += walk_pools(p->sub_pools, level+1, debugf);
  }

  return total;
}

static void debug_pool_info(void (*debugf)(const char *, ...)) {
  if (block_freelist)
    debugf("Free block list: %lu bytes",
      bytes_in_block_list(block_freelist));
  else
    debugf("Free block list: EMPTY");

  debugf("%u count blocks allocated", stat_malloc);
  debugf("%u count blocks reused", stat_freehit);
}

void pr_pool_debug_memory(void (*debugf)(const char *, ...)) {
  debugf("Memory pool allocation:");
  debugf("Total %lu bytes allocated", walk_pools(permanent_pool, 0, debugf));
  debug_pool_info(debugf);
}
#endif /* PR_USE_DEVEL */

void pr_pool_tag(pool *p, const char *tag) {
  if (!p || !tag)
    return;

  p->tag = tag;
}

/* Release the entire free block list */
static void pool_release_free_block_list(void) {
  union block_hdr *blok,*next;

  pr_alarms_block();

  blok = block_freelist;
  if (blok) {
    for (next = blok->h.next; next; blok = next, next = blok->h.next)
      free(blok);
  }
  block_freelist = NULL;

  pr_alarms_unblock();
}

struct pool *make_sub_pool(struct pool *p) {
  union block_hdr *blok;
  pool *new_pool;

  pr_alarms_block();

  blok = new_block(0, FALSE);

  new_pool = (pool *) blok->h.first_avail;
  blok->h.first_avail += POOL_HDR_BYTES;

  memset(new_pool, 0, sizeof(struct pool));
  new_pool->free_first_avail = blok->h.first_avail;
  new_pool->first = new_pool->last = blok;

  if (p) {
    new_pool->parent = p;
    new_pool->sub_next = p->sub_pools;

    if (new_pool->sub_next)
      new_pool->sub_next->sub_prev = new_pool;

    p->sub_pools = new_pool;
  }

  pr_alarms_unblock();

  return new_pool;
}

struct pool *pr_pool_create_sz(struct pool *p, int sz) {
  union block_hdr *blok;
  pool *new_pool;

  pr_alarms_block();

  blok = new_block(sz + POOL_HDR_BYTES, TRUE);

  new_pool = (pool *) blok->h.first_avail;
  blok->h.first_avail += POOL_HDR_BYTES;

  memset(new_pool, 0, sizeof(struct pool));
  new_pool->free_first_avail = blok->h.first_avail;
  new_pool->first = new_pool->last = blok;

  if (p) {
    new_pool->parent = p;
    new_pool->sub_next = p->sub_pools;

    if (new_pool->sub_next)
      new_pool->sub_next->sub_prev = new_pool;

    p->sub_pools = new_pool;
  }

  pr_alarms_unblock();

  return new_pool;
}

/* Initialize the pool system by creating the base permanent_pool. */

void init_pools(void) {
  if (!permanent_pool)
    permanent_pool = make_sub_pool(NULL);
  pr_pool_tag(permanent_pool, "permanent_pool");
}

void free_pools(void) {
  destroy_pool(permanent_pool);
  permanent_pool = NULL;
  pool_release_free_block_list();
}

static void clear_pool(struct pool *p) {

  /* Sanity check. */
  if (!p)
    return;

  pr_alarms_block();

  /* Run through any cleanups. */
  run_cleanups(p->cleanups);
  p->cleanups = NULL;

  /* Destroy subpools. */
  while (p->sub_pools)
    destroy_pool(p->sub_pools);
  p->sub_pools = NULL;

  free_blocks(p->first->h.next);
  p->first->h.next = NULL;

  p->last = p->first;
  p->first->h.first_avail = p->free_first_avail;

  pr_alarms_unblock();
}

void destroy_pool(pool *p) {
  if (p == NULL)
    return;

  pr_alarms_block();

  if (p->parent) {
    if (p->parent->sub_pools == p)
      p->parent->sub_pools = p->sub_next;

    if (p->sub_prev)
      p->sub_prev->sub_next = p->sub_next;

    if (p->sub_next)
      p->sub_next->sub_prev = p->sub_prev;
  }
  clear_pool(p);
  free_blocks(p->first);

  pr_alarms_unblock();
}

/* Allocation interface...
 */

static void *alloc_pool(struct pool *p, int reqsz, int exact) {

  /* Round up requested size to an even number of aligned units */
  int nclicks = 1 + ((reqsz - 1) / CLICK_SZ);
  int sz = nclicks * CLICK_SZ;

  /* For performance, see if space is available in the most recently
   * allocated block.
   */

  union block_hdr *blok = p->last;
  char *first_avail = blok->h.first_avail;
  char *new_first_avail;

  if (reqsz <= 0)
    return NULL;

  new_first_avail = first_avail + sz;

  if (new_first_avail <= blok->h.endp) {
    blok->h.first_avail = new_first_avail;
    return (void *) first_avail;
  }

  /* Need a new one that's big enough */
  pr_alarms_block();

  blok = new_block(sz, exact);
  p->last->h.next = blok;
  p->last = blok;

  first_avail = blok->h.first_avail;
  blok->h.first_avail += sz;

  pr_alarms_unblock();
  return (void *) first_avail;
}

void *palloc(struct pool *p, int sz) {
  return alloc_pool(p, sz, FALSE);
}

void *pallocsz(struct pool *p, int sz) {
  return alloc_pool(p, sz, TRUE);
}

void *pcalloc(struct pool *p, int sz) {
  void *res = palloc(p, sz);
  memset(res, '\0', sz);
  return res;
}

void *pcallocsz(struct pool *p, int sz) {
  void *res = pallocsz(p, sz);
  memset(res, '\0', sz);
  return res;
}

char *pstrdup(struct pool *p, const char *s) {
  char *res;
  size_t len;

  if (!p || !s) {
    errno = EINVAL;
    return NULL;
  }

  len = strlen(s) + 1;

  res = palloc(p, len);
  sstrncpy(res, s, len);
  return res;
}

char *pstrndup(struct pool *p, const char *s, int n) {
  char *res;

  if (!p || !s) {
    errno = EINVAL;
    return NULL;
  }

  res = palloc(p, n + 1);
  sstrncpy(res, s, n + 1);
  return res;
}

char *pdircat(pool *p, ...) {
  char *argp, *res;
  char last;

  int len = 0, count = 0;
  va_list dummy;

  va_start(dummy, p);

  last = 0;

  while ((res = va_arg(dummy, char *)) != NULL) {
    /* If the first argument is "", we have to account for a leading /
     * which must be added.
     */
    if (!count++ && !*res)
      len++;
    else if (last && last != '/' && *res != '/')
      len++;
    else if (last && last == '/' && *res == '/')
      len--;
    len += strlen(res);
    last = (*res ? res[strlen(res) - 1] : 0);
  }

  va_end(dummy);
  res = (char *) pcalloc(p, len + 1);

  va_start(dummy, p);

  last = 0;

  while ((argp = va_arg(dummy, char *)) != NULL) {
    if (last && last == '/' && *argp == '/')
      argp++;
    else if (last && last != '/' && *argp != '/')
      sstrcat(res, "/", len + 1);

    sstrcat(res, argp, len + 1);
    last = (*res ? res[strlen(res) - 1] : 0);
  }

  va_end(dummy);

  return res;
}

char *pstrcat(pool *p, ...) {
  char *argp, *res;

  size_t len = 0;
  va_list dummy;

  va_start(dummy, p);

  while ((res = va_arg(dummy, char *)) != NULL)
    len += strlen(res);

  va_end(dummy);

  res = (char *) pcalloc(p, len + 1);

  va_start(dummy, p);

  while ((argp = va_arg(dummy, char *)) != NULL)
    sstrcat(res, argp, len + 1);

  va_end(dummy);

  return res;
}

/*
 * Array functions
 */

array_header *make_array(pool *p, int nelts, int elt_size) {
  array_header *res = (array_header *) palloc(p, sizeof(array_header));

  if (nelts < 1) nelts = 1;

  res->elts = pcalloc(p, nelts * elt_size);
  res->pool = p;
  res->elt_size = elt_size;
  res->nelts = 0;
  res->nalloc = nelts;

  return res;
}

void *push_array(array_header *arr) {
  if (arr->nelts == arr->nalloc) {
    char *new_data = pcalloc(arr->pool, arr->nalloc * arr->elt_size * 2);

    memcpy(new_data, arr->elts, arr->nalloc * arr->elt_size);
    arr->elts = new_data;
    arr->nalloc *= 2;
  }

  ++arr->nelts;
  return ((char *)arr->elts) + (arr->elt_size * (arr->nelts - 1));
}

void array_cat(array_header *dst, const array_header *src) {
  int elt_size = dst->elt_size;

  if (dst->nelts + src->nelts > dst->nalloc) {
    int new_size = dst->nalloc * 2;
    char *new_data;

    if (new_size == 0) ++new_size;

    while (dst->nelts + src->nelts > new_size)
      new_size *= 2;

    new_data = pcalloc(dst->pool, elt_size * new_size);
    memcpy(new_data, dst->elts, dst->nalloc * elt_size);

    dst->elts = new_data;
    dst->nalloc = new_size;
  }

  memcpy(((char *)dst->elts) + dst->nelts * elt_size, (char *)src->elts,
         elt_size * src->nelts);
  dst->nelts += src->nelts;
}

array_header *copy_array(pool *p, const array_header *arr) {
  array_header *res = make_array(p,arr->nalloc,arr->elt_size);

  memcpy(res->elts, arr->elts, arr->elt_size * arr->nelts);
  res->nelts = arr->nelts;
  return res;
}

/* copy an array that is assumed to consist solely of strings */
array_header *copy_array_str(pool *p, const array_header *arr) {
  array_header *res = copy_array(p,arr);
  int i;

  for (i = 0; i < arr->nelts; i++)
    ((char **)res->elts)[i] = pstrdup(p, ((char **)res->elts)[i]);

  return res;
}

array_header *copy_array_hdr(pool *p, const array_header *arr) {
  array_header *res = (array_header *)palloc(p,sizeof(array_header));

  res->elts = arr->elts;
  res->pool = p;
  res->elt_size = arr->elt_size;
  res->nelts = arr->nelts;
  res->nalloc = arr->nelts;		/* Force overflow on push */

  return res;
}

array_header *append_arrays(pool *p, const array_header *first,
    const array_header *second) {
  array_header *res = copy_array_hdr(p, first);

  array_cat(res, second);
  return res;
}

/*
 * Generic cleanups
 */

typedef struct cleanup {
  void *data;
  void (*plain_cleanup_cb)(void *);
  void (*child_cleanup_cb)(void *);
  struct cleanup *next;
} cleanup_t;

void register_cleanup(pool *p, void *data, void (*plain_cleanup_cb)(void*),
    void (*child_cleanup_cb)(void *)) {
  cleanup_t *c = pcalloc(p, sizeof(cleanup_t));
  c->data = data;
  c->plain_cleanup_cb = plain_cleanup_cb;
  c->child_cleanup_cb = child_cleanup_cb;

  /* Add this cleanup to the given pool's list of cleanups. */
  c->next = p->cleanups;
  p->cleanups = c;
}

void unregister_cleanup(pool *p, void *data, void (*cleanup_cb)(void *)) {
  cleanup_t *c = p->cleanups;
  cleanup_t **lastp = &p->cleanups;

  while (c) {
    if (c->data == data && c->plain_cleanup_cb == cleanup_cb) {

      /* Remove the given cleanup by pointing the previous next pointer to
       * the matching cleanup's next pointer.
       */
      *lastp = c->next;
      break;
    }

    lastp = &c->next;
    c = c->next;
  }
}

static void run_cleanups(cleanup_t *c) {
  while (c) {
    (*c->plain_cleanup_cb)(c->data);
    c = c->next;
  }
}
