/*
 *	BIRD Resource Manager -- A SLAB-like Memory Allocator
 *
 *	Heavily inspired by the original SLAB paper by Jeff Bonwick.
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Slabs
 *
 * Slabs are collections of memory blocks of a fixed size.
 * They support very fast allocation and freeing of such blocks, prevent memory
 * fragmentation and optimize L2 cache usage. Slabs have been invented by Jeff Bonwick
 * and published in USENIX proceedings as `The Slab Allocator: An Object-Caching Kernel
 * Memory Allocator'. Our implementation follows this article except that we don't use
 * constructors and destructors.
 *
 * When the |DEBUGGING| switch is turned on, we automatically fill all
 * newly allocated and freed blocks with a special pattern to make detection
 * of use of uninitialized or already freed memory easier.
 *
 * Example: Nodes of a FIB are allocated from a per-FIB Slab.
 */

#include <stdlib.h>
#include <stdint.h>

#include "nest/bird.h"
#include "lib/resource.h"
#include "lib/string.h"

#undef FAKE_SLAB	/* Turn on if you want to debug memory allocations */

#ifdef DEBUGGING
#define POISON		/* Poison all regions after they are freed */
#endif

static void slab_free(resource *r);
static void slab_dump(resource *r);
static resource *slab_lookup(resource *r, unsigned long addr);
static size_t slab_memsize(resource *r);

#ifdef FAKE_SLAB

/*
 *  Fake version used for debugging.
 */

struct slab {
  resource r;
  unsigned size;
  list objs;
};

static struct resclass sl_class = {
  "FakeSlab",
  sizeof(struct slab),
  slab_free,
  slab_dump,
  NULL,
  slab_memsize
};

struct sl_obj {
  node n;
  uintptr_t data_align[0];
  byte data[0];
};

slab *
sl_new(pool *p, unsigned size)
{
  slab *s = ralloc(p, &sl_class);
  s->size = size;
  init_list(&s->objs);
  return s;
}

void *
sl_alloc(slab *s)
{
  struct sl_obj *o = xmalloc(sizeof(struct sl_obj) + s->size);

  add_tail(&s->objs, &o->n);
  return o->data;
}

void
sl_free(slab *s, void *oo)
{
  struct sl_obj *o = SKIP_BACK(struct sl_obj, data, oo);

  rem_node(&o->n);
  xfree(o);
}

static void
slab_free(resource *r)
{
  slab *s = (slab *) r;
  struct sl_obj *o, *p;

  for(o = HEAD(s->objs); p = (struct sl_obj *) o->n.next; o = p)
    xfree(o);
}

static void
slab_dump(resource *r)
{
  slab *s = (slab *) r;
  int cnt = 0;
  struct sl_obj *o;

  WALK_LIST(o, s->objs)
    cnt++;
  debug("(%d objects per %d bytes)\n", cnt, s->size);
}

static size_t
slab_memsize(resource *r)
{
  slab *s = (slab *) r;
  int cnt = 0;
  struct sl_obj *o;

  WALK_LIST(o, s->objs)
    cnt++;

  return ALLOC_OVERHEAD + sizeof(struct slab) + cnt * (ALLOC_OVERHEAD + s->size);
}


#else

/*
 *  Real efficient version.
 */

#define SLAB_SIZE 4096
#define MAX_EMPTY_HEADS 1

struct slab {
  resource r;
  unsigned obj_size, head_size, objs_per_slab, num_empty_heads, data_size;
  list empty_heads, partial_heads, full_heads;
};

static struct resclass sl_class = {
  "Slab",
  sizeof(struct slab),
  slab_free,
  slab_dump,
  slab_lookup,
  slab_memsize
};

struct sl_head {
  node n;
  struct sl_obj *first_free;
  int num_full;
};

struct sl_obj {
  struct sl_head *slab;
  union {
    struct sl_obj *next;
    byte data[0];
  } u;
};

struct sl_alignment {			/* Magic structure for testing of alignment */
  byte data;
  int x[0];
};

/**
 * sl_new - create a new Slab
 * @p: resource pool
 * @size: block size
 *
 * This function creates a new Slab resource from which
 * objects of size @size can be allocated.
 */
slab *
sl_new(pool *p, unsigned size)
{
  slab *s = ralloc(p, &sl_class);
  unsigned int align = sizeof(struct sl_alignment);
  if (align < sizeof(int))
    align = sizeof(int);
  s->data_size = size;
  size += OFFSETOF(struct sl_obj, u.data);
  if (size < sizeof(struct sl_obj))
    size = sizeof(struct sl_obj);
  size = (size + align - 1) / align * align;
  s->obj_size = size;
  s->head_size = (sizeof(struct sl_head) + align - 1) / align * align;
  s->objs_per_slab = (SLAB_SIZE - s->head_size) / size;
  if (!s->objs_per_slab)
    bug("Slab: object too large");
  s->num_empty_heads = 0;
  init_list(&s->empty_heads);
  init_list(&s->partial_heads);
  init_list(&s->full_heads);
  return s;
}

static struct sl_head *
sl_new_head(slab *s)
{
  struct sl_head *h = xmalloc(SLAB_SIZE);
  struct sl_obj *o = (struct sl_obj *)((byte *)h+s->head_size);
  struct sl_obj *no;
  unsigned int n = s->objs_per_slab;

  h->first_free = o;
  h->num_full = 0;
  while (n--)
    {
      o->slab = h;
      no = (struct sl_obj *)((char *) o+s->obj_size);
      o->u.next = n ? no : NULL;
      o = no;
    }
  return h;
}

/**
 * sl_alloc - allocate an object from Slab
 * @s: slab
 *
 * sl_alloc() allocates space for a single object from the
 * Slab and returns a pointer to the object.
 */
void *
sl_alloc(slab *s)
{
  struct sl_head *h;
  struct sl_obj *o;

redo:
  h = HEAD(s->partial_heads);
  if (!h->n.next)
    goto no_partial;
okay:
  o = h->first_free;
  if (!o)
    goto full_partial;
  h->first_free = o->u.next;
  h->num_full++;
#ifdef POISON
  memset(o->u.data, 0xcd, s->data_size);
#endif
  return o->u.data;

full_partial:
  rem_node(&h->n);
  add_tail(&s->full_heads, &h->n);
  goto redo;

no_partial:
  h = HEAD(s->empty_heads);
  if (h->n.next)
    {
      rem_node(&h->n);
      add_head(&s->partial_heads, &h->n);
      s->num_empty_heads--;
      goto okay;
    }
  h = sl_new_head(s);
  add_head(&s->partial_heads, &h->n);
  goto okay;
}

/**
 * sl_free - return a free object back to a Slab
 * @s: slab
 * @oo: object returned by sl_alloc()
 *
 * This function frees memory associated with the object @oo
 * and returns it back to the Slab @s.
 */
void
sl_free(slab *s, void *oo)
{
  struct sl_obj *o = SKIP_BACK(struct sl_obj, u.data, oo);
  struct sl_head *h = o->slab;

#ifdef POISON
  memset(oo, 0xdb, s->data_size);
#endif
  o->u.next = h->first_free;
  h->first_free = o;
  if (!--h->num_full)
    {
      rem_node(&h->n);
      if (s->num_empty_heads >= MAX_EMPTY_HEADS)
	xfree(h);
      else
	{
	  add_head(&s->empty_heads, &h->n);
	  s->num_empty_heads++;
	}
    }
  else if (!o->u.next)
    {
      rem_node(&h->n);
      add_head(&s->partial_heads, &h->n);
    }
}

static void
slab_free(resource *r)
{
  slab *s = (slab *) r;
  struct sl_head *h, *g;

  WALK_LIST_DELSAFE(h, g, s->empty_heads)
    xfree(h);
  WALK_LIST_DELSAFE(h, g, s->partial_heads)
    xfree(h);
  WALK_LIST_DELSAFE(h, g, s->full_heads)
    xfree(h);
}

static void
slab_dump(resource *r)
{
  slab *s = (slab *) r;
  int ec=0, pc=0, fc=0;
  struct sl_head *h;

  WALK_LIST(h, s->empty_heads)
    ec++;
  WALK_LIST(h, s->partial_heads)
    pc++;
  WALK_LIST(h, s->full_heads)
    fc++;
  debug("(%de+%dp+%df blocks per %d objs per %d bytes)\n", ec, pc, fc, s->objs_per_slab, s->obj_size);
}

static size_t
slab_memsize(resource *r)
{
  slab *s = (slab *) r;
  int heads = 0;
  struct sl_head *h;

  WALK_LIST(h, s->empty_heads)
    heads++;
  WALK_LIST(h, s->partial_heads)
    heads++;
  WALK_LIST(h, s->full_heads)
    heads++;

  return ALLOC_OVERHEAD + sizeof(struct slab) + heads * (ALLOC_OVERHEAD + SLAB_SIZE);
}

static resource *
slab_lookup(resource *r, unsigned long a)
{
  slab *s = (slab *) r;
  struct sl_head *h;

  WALK_LIST(h, s->partial_heads)
    if ((unsigned long) h < a && (unsigned long) h + SLAB_SIZE < a)
      return r;
  WALK_LIST(h, s->full_heads)
    if ((unsigned long) h < a && (unsigned long) h + SLAB_SIZE < a)
      return r;
  return NULL;
}

#endif
