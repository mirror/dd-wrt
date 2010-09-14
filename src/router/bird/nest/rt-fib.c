/*
 *	BIRD -- Forwarding Information Base -- Data Structures
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Forwarding Information Base
 *
 * FIB is a data structure designed for storage of routes indexed by their
 * network prefixes. It supports insertion, deletion, searching by prefix,
 * `routing' (in CIDR sense, that is searching for a longest prefix matching
 * a given IP address) and (which makes the structure very tricky to implement)
 * asynchronous reading, that is enumerating the contents of a FIB while other
 * modules add, modify or remove entries.
 *
 * Internally, each FIB is represented as a collection of nodes of type &fib_node
 * indexed using a sophisticated hashing mechanism.
 * We use two-stage hashing where we calculate a 16-bit primary hash key independent
 * on hash table size and then we just divide the primary keys modulo table size
 * to get a real hash key used for determining the bucket containing the node.
 * The lists of nodes in each bucket are sorted according to the primary hash
 * key, hence if we keep the total number of buckets to be a power of two,
 * re-hashing of the structure keeps the relative order of the nodes.
 *
 * To get the asynchronous reading consistent over node deletions, we need to
 * keep a list of readers for each node. When a node gets deleted, its readers
 * are automatically moved to the next node in the table.
 *
 * Basic FIB operations are performed by functions defined by this module,
 * enumerating of FIB contents is accomplished by using the FIB_WALK() macro
 * or FIB_ITERATE_START() if you want to do it asynchronously.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/route.h"
#include "lib/string.h"

#define HASH_DEF_ORDER 10
#define HASH_HI_MARK *4
#define HASH_HI_STEP 2
#define HASH_HI_MAX 16			/* Must be at most 16 */
#define HASH_LO_MARK /5
#define HASH_LO_STEP 2
#define HASH_LO_MIN 10

static void
fib_ht_alloc(struct fib *f)
{
  f->hash_size = 1 << f->hash_order;
  f->hash_shift = 16 - f->hash_order;
  if (f->hash_order > HASH_HI_MAX - HASH_HI_STEP)
    f->entries_max = ~0;
  else
    f->entries_max = f->hash_size HASH_HI_MARK;
  if (f->hash_order < HASH_LO_MIN + HASH_LO_STEP)
    f->entries_min = 0;
  else
    f->entries_min = f->hash_size HASH_LO_MARK;
  DBG("Allocating FIB hash of order %d: %d entries, %d low, %d high\n",
      f->hash_order, f->hash_size, f->entries_min, f->entries_max);
  f->hash_table = mb_alloc(f->fib_pool, f->hash_size * sizeof(struct fib_node *));
}

static inline void
fib_ht_free(struct fib_node **h)
{
  mb_free(h);
}

static inline unsigned
fib_hash(struct fib *f, ip_addr *a)
{
  return ipa_hash(*a) >> f->hash_shift;
}

static void
fib_dummy_init(struct fib_node *dummy UNUSED)
{
}

/**
 * fib_init - initialize a new FIB
 * @f: the FIB to be initialized (the structure itself being allocated by the caller)
 * @p: pool to allocate the nodes in
 * @node_size: node size to be used (each node consists of a standard header &fib_node
 * followed by user data)
 * @hash_order: initial hash order (a binary logarithm of hash table size), 0 to use default order
 * (recommended)
 * @init: pointer a function to be called to initialize a newly created node
 *
 * This function initializes a newly allocated FIB and prepares it for use.
 */
void
fib_init(struct fib *f, pool *p, unsigned node_size, unsigned hash_order, fib_init_func init)
{
  if (!hash_order)
    hash_order = HASH_DEF_ORDER;
  f->fib_pool = p;
  f->fib_slab = sl_new(p, node_size);
  f->hash_order = hash_order;
  fib_ht_alloc(f);
  bzero(f->hash_table, f->hash_size * sizeof(struct fib_node *));
  f->entries = 0;
  f->entries_min = 0;
  f->init = init ? : fib_dummy_init;
}

static void
fib_rehash(struct fib *f, int step)
{
  unsigned old, new, oldn, newn, ni, nh;
  struct fib_node **n, *e, *x, **t, **m, **h;

  old = f->hash_order;
  oldn = f->hash_size;
  new = old + step;
  m = h = f->hash_table;
  DBG("Re-hashing FIB from order %d to %d\n", old, new);
  f->hash_order = new;
  fib_ht_alloc(f);
  t = n = f->hash_table;
  newn = f->hash_size;
  ni = 0;

  while (oldn--)
    {
      x = *h++;
      while (e = x)
	{
	  x = e->next;
	  nh = fib_hash(f, &e->prefix);
	  while (nh > ni)
	    {
	      *t = NULL;
	      ni++;
	      t = ++n;
	    }
	  *t = e;
	  t = &e->next;
	}
    }
  while (ni < newn)
    {
      *t = NULL;
      ni++;
      t = ++n;
    }
  fib_ht_free(m);
}

/**
 * fib_find - search for FIB node by prefix
 * @f: FIB to search in
 * @a: pointer to IP address of the prefix
 * @len: prefix length
 *
 * Search for a FIB node corresponding to the given prefix, return
 * a pointer to it or %NULL if no such node exists.
 */
void *
fib_find(struct fib *f, ip_addr *a, int len)
{
  struct fib_node *e = f->hash_table[fib_hash(f, a)];

  while (e && (e->pxlen != len || !ipa_equal(*a, e->prefix)))
    e = e->next;
  return e;
}

/*
int
fib_histogram(struct fib *f)
{
  log(L_WARN "Histogram dump start %d %d", f->hash_size, f->entries);

  int i, j;
  struct fib_node *e;

  for (i = 0; i < f->hash_size; i++)
    {
      j = 0;
      for (e = f->hash_table[i]; e != NULL; e = e->next)
	j++;
      if (j > 0)
        log(L_WARN "Histogram line %d: %d", i, j);
    }

  log(L_WARN "Histogram dump end");
}
*/

/**
 * fib_get - find or create a FIB node
 * @f: FIB to work with
 * @a: pointer to IP address of the prefix
 * @len: prefix length
 *
 * Search for a FIB node corresponding to the given prefix and
 * return a pointer to it. If no such node exists, create it.
 */
void *
fib_get(struct fib *f, ip_addr *a, int len)
{
  unsigned int h = ipa_hash(*a);
  struct fib_node **ee = f->hash_table + (h >> f->hash_shift);
  struct fib_node *g, *e = *ee;
  u32 uid = h << 16;

  while (e && (e->pxlen != len || !ipa_equal(*a, e->prefix)))
    e = e->next;
  if (e)
    return e;
#ifdef DEBUGGING
  if (len < 0 || len > BITS_PER_IP_ADDRESS || !ip_is_prefix(*a,len))
    bug("fib_get() called for invalid address");
#endif

  while ((g = *ee) && g->uid < uid)
    ee = &g->next;
  while ((g = *ee) && g->uid == uid)
    {
      ee = &g->next;
      uid++;
    }

  if ((uid >> 16) != h)
    log(L_ERR "FIB hash table chains are too long");

  // log (L_WARN "FIB_GET %I %x %x", *a, h, uid);

  e = sl_alloc(f->fib_slab);
  e->prefix = *a;
  e->pxlen = len;
  e->next = *ee;
  e->uid = uid;
  *ee = e;
  e->readers = NULL;
  f->init(e);
  if (f->entries++ > f->entries_max)
    fib_rehash(f, HASH_HI_STEP);

  return e;
}

/**
 * fib_route - CIDR routing lookup
 * @f: FIB to search in
 * @a: pointer to IP address of the prefix
 * @len: prefix length
 *
 * Search for a FIB node with longest prefix matching the given
 * network, that is a node which a CIDR router would use for routing
 * that network.
 */
void *
fib_route(struct fib *f, ip_addr a, int len)
{
  ip_addr a0;
  void *t;

  while (len >= 0)
    {
      a0 = ipa_and(a, ipa_mkmask(len));
      t = fib_find(f, &a0, len);
      if (t)
	return t;
      len--;
    }
  return NULL;
}

static inline void
fib_merge_readers(struct fib_iterator *i, struct fib_node *to)
{
  if (to)
    {
      struct fib_iterator *j = to->readers;
      if (!j)
	{
	  /* Fast path */
	  to->readers = i;
	  i->prev = (struct fib_iterator *) to;
	}
      else
	{
	  /* Really merging */
	  while (j->next)
	    j = j->next;
	  j->next = i;
	  i->prev = j;
	}
      while (i && i->node)
	{
	  i->node = NULL;
	  i = i->next;
	}
    }
  else					/* No more nodes */
    while (i)
      {
	i->prev = NULL;
	i = i->next;
      }
}

/**
 * fib_delete - delete a FIB node
 * @f: FIB to delete from
 * @E: entry to delete
 *
 * This function removes the given entry from the FIB,
 * taking care of all the asynchronous readers by shifting
 * them to the next node in the canonical reading order.
 */
void
fib_delete(struct fib *f, void *E)
{
  struct fib_node *e = E;
  unsigned int h = fib_hash(f, &e->prefix);
  struct fib_node **ee = f->hash_table + h;
  struct fib_iterator *it;

  while (*ee)
    {
      if (*ee == e)
	{
	  *ee = e->next;
	  if (it = e->readers)
	    {
	      struct fib_node *l = e->next;
	      while (!l)
		{
		  h++;
		  if (h >= f->hash_size)
		    break;
		  else
		    l = f->hash_table[h];
		}
	      fib_merge_readers(it, l);
	    }
	  sl_free(f->fib_slab, e);
	  if (f->entries-- < f->entries_min)
	    fib_rehash(f, -HASH_LO_STEP);
	  return;
	}
      ee = &((*ee)->next);
    }
  bug("fib_delete() called for invalid node");
}

/**
 * fib_free - delete a FIB
 * @f: FIB to be deleted
 *
 * This function deletes a FIB -- it frees all memory associated
 * with it and all its entries.
 */
void
fib_free(struct fib *f)
{
  fib_ht_free(f->hash_table);
  rfree(f->fib_slab);
}

void
fit_init(struct fib_iterator *i, struct fib *f)
{
  unsigned h;
  struct fib_node *n;

  i->efef = 0xff;
  for(h=0; h<f->hash_size; h++)
    if (n = f->hash_table[h])
      {
	i->prev = (struct fib_iterator *) n;
	if (i->next = n->readers)
	  i->next->prev = i;
	n->readers = i;
	i->node = n;
	return;
      }
  /* The fib is empty, nothing to do */
  i->prev = i->next = NULL;
  i->node = NULL;
}

struct fib_node *
fit_get(struct fib *f, struct fib_iterator *i)
{
  struct fib_node *n;
  struct fib_iterator *j, *k;

  if (!i->prev)
    {
      /* We are at the end */
      i->hash = ~0 - 1;
      return NULL;
    }
  if (!(n = i->node))
    {
      /* No node info available, we are a victim of merging. Try harder. */
      j = i;
      while (j->efef == 0xff)
	j = j->prev;
      n = (struct fib_node *) j;
    }
  j = i->prev;
  if (k = i->next)
    k->prev = j;
  j->next = k;
  i->hash = fib_hash(f, &n->prefix);
  return n;
}

void
fit_put(struct fib_iterator *i, struct fib_node *n)
{
  struct fib_iterator *j;

  i->node = n;
  if (j = n->readers)
    j->prev = i;
  i->next = j;
  n->readers = i;
  i->prev = (struct fib_iterator *) n;
}

#ifdef DEBUGGING

/**
 * fib_check - audit a FIB
 * @f: FIB to be checked
 *
 * This debugging function audits a FIB by checking its internal consistency.
 * Use when you suspect somebody of corrupting innocent data structures.
 */
void
fib_check(struct fib *f)
{
  unsigned int i, ec, lo, nulls;

  ec = 0;
  for(i=0; i<f->hash_size; i++)
    {
      struct fib_node *n;
      lo = 0;
      for(n=f->hash_table[i]; n; n=n->next)
	{
	  struct fib_iterator *j, *j0;
	  unsigned int h0 = ipa_hash(n->prefix);
	  if (h0 < lo)
	    bug("fib_check: discord in hash chains");
	  lo = h0;
	  if ((h0 >> f->hash_shift) != i)
	    bug("fib_check: mishashed %x->%x (order %d)", h0, i, f->hash_order);
	  j0 = (struct fib_iterator *) n;
	  nulls = 0;
	  for(j=n->readers; j; j=j->next)
	    {
	      if (j->prev != j0)
		bug("fib_check: iterator->prev mismatch");
	      j0 = j;
	      if (!j->node)
		nulls++;
	      else if (nulls)
		bug("fib_check: iterator nullified");
	      else if (j->node != n)
		bug("fib_check: iterator->node mismatch");
	    }
	  ec++;
	}
    }
  if (ec != f->entries)
    bug("fib_check: invalid entry count (%d != %d)", ec, f->entries);
}

#endif

#ifdef TEST

#include "lib/resource.h"

struct fib f;

void dump(char *m)
{
  unsigned int i;

  debug("%s ... order=%d, size=%d, entries=%d\n", m, f.hash_order, f.hash_size, f.hash_size);
  for(i=0; i<f.hash_size; i++)
    {
      struct fib_node *n;
      struct fib_iterator *j;
      for(n=f.hash_table[i]; n; n=n->next)
	{
	  debug("%04x %04x %p %I/%2d", i, ipa_hash(n->prefix), n, n->prefix, n->pxlen);
	  for(j=n->readers; j; j=j->next)
	    debug(" %p[%p]", j, j->node);
	  debug("\n");
	}
    }
  fib_check(&f);
  debug("-----\n");
}

void init(struct fib_node *n)
{
}

int main(void)
{
  struct fib_node *n;
  struct fib_iterator i, j;
  ip_addr a;
  int c;

  log_init_debug(NULL);
  resource_init();
  fib_init(&f, &root_pool, sizeof(struct fib_node), 4, init);
  dump("init");

  a = ipa_from_u32(0x01020304); n = fib_get(&f, &a, 32);
  a = ipa_from_u32(0x02030405); n = fib_get(&f, &a, 32);
  a = ipa_from_u32(0x03040506); n = fib_get(&f, &a, 32);
  a = ipa_from_u32(0x00000000); n = fib_get(&f, &a, 32);
  a = ipa_from_u32(0x00000c01); n = fib_get(&f, &a, 32);
  a = ipa_from_u32(0xffffffff); n = fib_get(&f, &a, 32);
  dump("fill");

  fit_init(&i, &f);
  dump("iter init");

  fib_rehash(&f, 1);
  dump("rehash up");

  fib_rehash(&f, -1);
  dump("rehash down");

next:
  c = 0;
  FIB_ITERATE_START(&f, &i, z)
    {
      if (c)
	{
	  FIB_ITERATE_PUT(&i, z);
	  dump("iter");
	  goto next;
	}
      c = 1;
      debug("got %p\n", z);
    }
  FIB_ITERATE_END;
  dump("iter end");

  fit_init(&i, &f);
  fit_init(&j, &f);
  dump("iter init 2");

  n = fit_get(&f, &i);
  dump("iter step 2");

  fit_put(&i, n->next);
  dump("iter step 3");

  a = ipa_from_u32(0xffffffff); n = fib_get(&f, &a, 32);
  fib_delete(&f, n);
  dump("iter step 3");

  return 0;
}

#endif
