/*
 *	BIRD Object Locks
 *
 *	(c) 1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Object locks
 *
 * The lock module provides a simple mechanism for avoiding conflicts between
 * various protocols which would like to use a single physical resource (for
 * example a network port). It would be easy to say that such collisions can
 * occur only when the user specifies an invalid configuration and therefore
 * he deserves to get what he has asked for, but unfortunately they can also
 * arise legitimately when the daemon is reconfigured and there exists (although
 * for a short time period only) an old protocol instance being shut down and a new one
 * willing to start up on the same interface.
 *
 * The solution is very simple: when any protocol wishes to use a network port
 * or some other non-shareable resource, it asks the core to lock it and it doesn't
 * use the resource until it's notified that it has acquired the lock.
 *
 * Object locks are represented by &object_lock structures which are in turn a kind of
 * resource. Lockable resources are uniquely determined by resource type
 * (%OBJLOCK_UDP for a UDP port etc.), IP address (usually a broadcast or
 * multicast address the port is bound to), port number and interface.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "lib/resource.h"
#include "nest/locks.h"
#include "nest/iface.h"

static list olock_list;
static event *olock_event;

static inline int
olock_same(struct object_lock *x, struct object_lock *y)
{
  return
    x->type == y->type &&
    x->iface == y->iface &&
    x->port == y->port &&
    ipa_equal(x->addr, y->addr);
}

static void
olock_free(resource *r)
{
  struct object_lock *q, *l = (struct object_lock *) r;
  node *n;

  DBG("olock: Freeing %p\n", l);
  switch (l->state)
    {
    case OLOCK_STATE_FREE:
      break;
    case OLOCK_STATE_LOCKED:
    case OLOCK_STATE_EVENT:
      rem_node(&l->n);
      n = HEAD(l->waiters);
      if (n->next)
	{
	  DBG("olock: -> %p becomes locked\n", n);
	  q = SKIP_BACK(struct object_lock, n, n);
	  rem_node(n);
	  add_tail_list(&l->waiters, &q->waiters);
	  q->state = OLOCK_STATE_EVENT;
	  add_head(&olock_list, n);
	  ev_schedule(olock_event);
	}
      break;
    case OLOCK_STATE_WAITING:
      rem_node(&l->n);
      break;
    default:
      ASSERT(0);
    }
}

static void
olock_dump(resource *r)
{
  struct object_lock *l = (struct object_lock *) r;
  static char *olock_states[] = { "free", "locked", "waiting", "event" };

  debug("(%d:%s:%I:%d) [%s]\n", l->type, (l->iface ? l->iface->name : "?"), l->addr, l->port, olock_states[l->state]);
  if (!EMPTY_LIST(l->waiters))
    debug(" [wanted]\n");
}

static struct resclass olock_class = {
  "ObjLock",
  sizeof(struct object_lock),
  olock_free,
  olock_dump,
  NULL
};

/**
 * olock_new - create an object lock
 * @p: resource pool to create the lock in.
 *
 * The olock_new() function creates a new resource of type &object_lock
 * and returns a pointer to it. After filling in the structure, the caller
 * should call olock_acquire() to do the real locking.
 */
struct object_lock *
olock_new(pool *p)
{
  struct object_lock *l = ralloc(p, &olock_class);

  l->state = OLOCK_STATE_FREE;
  init_list(&l->waiters);
  return l;
}

/**
 * olock_acquire - acquire a lock
 * @l: the lock to acquire
 *
 * This function attempts to acquire exclusive access to the non-shareable
 * resource described by the lock @l. It returns immediately, but as soon
 * as the resource becomes available, it calls the hook() function set up
 * by the caller.
 *
 * When you want to release the resource, just rfree() the lock.
 */
void
olock_acquire(struct object_lock *l)
{
  node *n;
  struct object_lock *q;

  WALK_LIST(n, olock_list)
    {
      q = SKIP_BACK(struct object_lock, n, n);
      if (olock_same(q, l))
	{
	  l->state = OLOCK_STATE_WAITING;
	  add_tail(&q->waiters, &l->n);
	  DBG("olock: %p waits\n", l);
	  return;
	}
    }
  DBG("olock: %p acquired immediately\n", l);
  l->state = OLOCK_STATE_EVENT;
  add_head(&olock_list, &l->n);
  ev_schedule(olock_event);
}

static void
olock_run_event(void *unused UNUSED)
{
  node *n;
  struct object_lock *q;

  DBG("olock: Processing events\n");
  for(;;)
    {
      n = HEAD(olock_list);
      if (!n->next)
	break;
      q = SKIP_BACK(struct object_lock, n, n);
      if (q->state != OLOCK_STATE_EVENT)
	break;
      DBG("olock: %p locked\n", q);
      q->state = OLOCK_STATE_LOCKED;
      rem_node(&q->n);
      add_tail(&olock_list, &q->n);
      q->hook(q);
    }
}

/**
 * olock_init - initialize the object lock mechanism
 *
 * This function is called during BIRD startup. It initializes
 * all the internal data structures of the lock module.
 */
void
olock_init(void)
{
  DBG("olock: init\n");
  init_list(&olock_list);
  olock_event = ev_new(&root_pool);
  olock_event->hook = olock_run_event;
}
