/*
 *	BIRD -- Neighbor Cache
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Neighbor cache
 *
 * Most routing protocols need to associate their internal state data with
 * neighboring routers, check whether an address given as the next hop
 * attribute of a route is really an address of a directly connected host
 * and which interface is it connected through. Also, they often need to
 * be notified when a neighbor ceases to exist or when their long awaited
 * neighbor becomes connected. The neighbor cache is there to solve all
 * these problems.
 *
 * The neighbor cache maintains a collection of neighbor entries. Each
 * entry represents one IP address corresponding to either our directly
 * connected neighbor or our own end of the link (when the scope of the
 * address is set to %SCOPE_HOST) together with per-neighbor data belonging to a
 * single protocol.
 *
 * Active entries represent known neighbors and are stored in a hash
 * table (to allow fast retrieval based on the IP address of the node) and
 * two linked lists: one global and one per-interface (allowing quick
 * processing of interface change events). Inactive entries exist only
 * when the protocol has explicitly requested it via the %NEF_STICKY
 * flag because it wishes to be notified when the node will again become
 * a neighbor. Such entries are enqueued in a special list which is walked
 * whenever an interface changes its state to up.
 *
 * When a neighbor event occurs (a neighbor gets disconnected or a sticky
 * inactive neighbor becomes connected), the protocol hook neigh_notify()
 * is called to advertise the change.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "lib/resource.h"

#define NEIGH_HASH_SIZE 256

static slab *neigh_slab;
static list sticky_neigh_list, neigh_hash_table[NEIGH_HASH_SIZE];

static inline unsigned int
neigh_hash(struct proto *p, ip_addr *a)
{
  return (p->hash_key ^ ipa_hash(*a)) & (NEIGH_HASH_SIZE-1);
}

static int
if_connected(ip_addr *a, struct iface *i, struct ifa **ap)
{
  struct ifa *b;

  if (!(i->flags & IF_UP))
  {
    *ap = NULL;
    return -1;
  }

  WALK_LIST(b, i->addrs)
    {
      *ap = b;

      if (ipa_equal(*a, b->ip))
	return SCOPE_HOST;
      if (b->flags & IA_PEER)
	{
	  if (ipa_equal(*a, b->opposite))
	    return b->scope;
	}
      else
	{
	  if (ipa_in_net(*a, b->prefix, b->pxlen))
	    {
#ifndef IPV6
	      if ((b->pxlen < (BITS_PER_IP_ADDRESS - 1)) &&
		  (ipa_equal(*a, b->prefix) ||	/* Network address */
		   ipa_equal(*a, b->brd)))	/* Broadcast */
	      {
		*ap = NULL;
		return -1;
	      }
#endif

	      return b->scope;
	    }
	}
      }

  *ap = NULL;
  return -1;
}

/**
 * neigh_find - find or create a neighbor entry.
 * @p: protocol which asks for the entry.
 * @a: pointer to IP address of the node to be searched for.
 * @flags: 0 or %NEF_STICKY if you want to create a sticky entry.
 *
 * Search the neighbor cache for a node with given IP address. If
 * it's found, a pointer to the neighbor entry is returned. If no
 * such entry exists and the node is directly connected on
 * one of our active interfaces, a new entry is created and returned
 * to the caller with protocol-dependent fields initialized to zero.
 * If the node is not connected directly or *@a is not a valid unicast
 * IP address, neigh_find() returns %NULL.
 */
neighbor *
neigh_find(struct proto *p, ip_addr *a, unsigned flags)
{
  return neigh_find2(p, a, NULL, flags);
}


neighbor *
neigh_find2(struct proto *p, ip_addr *a, struct iface *ifa, unsigned flags)
{
  neighbor *n;
  int class, scope = -1;
  unsigned int h = neigh_hash(p, a);
  struct iface *i;
  struct ifa *addr;

  WALK_LIST(n, neigh_hash_table[h])	/* Search the cache */
    if (n->proto == p && ipa_equal(*a, n->addr) && (!ifa || (ifa == n->iface)))
      return n;

  class = ipa_classify(*a);
  if (class < 0)			/* Invalid address */
    return NULL;
  if (((class & IADDR_SCOPE_MASK) == SCOPE_HOST) ||
      (((class & IADDR_SCOPE_MASK) == SCOPE_LINK) && (ifa == NULL)) ||
      !(class & IADDR_HOST))
    return NULL;			/* Bad scope or a somecast */

  if (ifa)
    {
      scope = if_connected(a, ifa, &addr);
      flags |= NEF_BIND;

      if ((scope < 0) && (flags & NEF_ONLINK))
	scope = class & IADDR_SCOPE_MASK;
    }
  else
    WALK_LIST(i, iface_list)
      if ((scope = if_connected(a, i, &addr)) >= 0)
	{
	  ifa = i;
	  break;
	}

  /* scope < 0 means i don't know neighbor */
  /* scope >= 0 implies ifa != NULL */

  if ((scope < 0) && !(flags & NEF_STICKY))
    return NULL;

  n = sl_alloc(neigh_slab);
  n->addr = *a;
  if (scope >= 0)
    {
      add_tail(&neigh_hash_table[h], &n->n);
      add_tail(&ifa->neighbors, &n->if_n);
    }
  else
    {
      add_tail(&sticky_neigh_list, &n->n);
      scope = -1;
    }
  n->iface = ifa;
  n->ifa = addr;
  n->proto = p;
  n->data = NULL;
  n->aux = 0;
  n->flags = flags;
  n->scope = scope;
  return n;
}

/**
 * neigh_dump - dump specified neighbor entry.
 * @n: the entry to dump
 *
 * This functions dumps the contents of a given neighbor entry
 * to debug output.
 */
void
neigh_dump(neighbor *n)
{
  debug("%p %I ", n, n->addr);
  if (n->iface)
    debug("%s ", n->iface->name);
  else
    debug("[] ");
  debug("%s %p %08x scope %s", n->proto->name, n->data, n->aux, ip_scope_text(n->scope));
  if (n->flags & NEF_STICKY)
    debug(" STICKY");
  debug("\n");
}

/**
 * neigh_dump_all - dump all neighbor entries.
 *
 * This function dumps the contents of the neighbor cache to
 * debug output.
 */
void
neigh_dump_all(void)
{
  neighbor *n;
  int i;

  debug("Known neighbors:\n");
  WALK_LIST(n, sticky_neigh_list)
    neigh_dump(n);
  for(i=0; i<NEIGH_HASH_SIZE; i++)
    WALK_LIST(n, neigh_hash_table[i])
      neigh_dump(n);
  debug("\n");
}

static void
neigh_up(neighbor *n, struct iface *i, int scope, struct ifa *a)
{
  n->iface = i;
  n->ifa = a;
  n->scope = scope;
  add_tail(&i->neighbors, &n->if_n);
  rem_node(&n->n);
  add_tail(&neigh_hash_table[neigh_hash(n->proto, &n->addr)], &n->n);
  DBG("Waking up sticky neighbor %I\n", n->addr);
  if (n->proto->neigh_notify && n->proto->core_state != FS_FLUSHING)
    n->proto->neigh_notify(n);
}

static void
neigh_down(neighbor *n)
{
  DBG("Flushing neighbor %I on %s\n", n->addr, n->iface->name);
  rem_node(&n->if_n);
  if (! (n->flags & NEF_BIND))
    n->iface = NULL;
  n->ifa = NULL;
  n->scope = -1;
  if (n->proto->neigh_notify && n->proto->core_state != FS_FLUSHING)
    n->proto->neigh_notify(n);
  rem_node(&n->n);
  if (n->flags & NEF_STICKY)
    {
      add_tail(&sticky_neigh_list, &n->n);

      /* Respawn neighbor if there is another matching prefix */
      struct iface *i;
      struct ifa *a;
      int scope;

      if (!n->iface)
	WALK_LIST(i, iface_list)
	  if ((scope = if_connected(&n->addr, i, &a)) >= 0)
	    {
	      neigh_up(n, i, scope, a);
	      return;
	    }
    }
  else
    sl_free(neigh_slab, n);
}


/**
 * neigh_if_up: notify neighbor cache about interface up event
 * @i: interface in question
 *
 * Tell the neighbor cache that a new interface became up.
 *
 * The neighbor cache wakes up all inactive sticky neighbors with
 * addresses belonging to prefixes of the interface @i.
 */
void
neigh_if_up(struct iface *i)
{
  struct ifa *a;
  neighbor *n, *next;
  int scope;

  WALK_LIST_DELSAFE(n, next, sticky_neigh_list)
    if ((!n->iface || n->iface == i) &&
	((scope = if_connected(&n->addr, i, &a)) >= 0))
      neigh_up(n, i, scope, a);
}

/**
 * neigh_if_down - notify neighbor cache about interface down event
 * @i: the interface in question
 *
 * Notify the neighbor cache that an interface has ceased to exist.
 *
 * It causes all entries belonging to neighbors connected to this interface
 * to be flushed.
 */
void
neigh_if_down(struct iface *i)
{
  node *x, *y;

  WALK_LIST_DELSAFE(x, y, i->neighbors)
    neigh_down(SKIP_BACK(neighbor, if_n, x));
}

/**
 * neigh_if_link - notify neighbor cache about interface link change
 * @i: the interface in question
 *
 * Notify the neighbor cache that an interface changed link state.
 * All owners of neighbor entries connected to this interface are
 * notified.
 */
void
neigh_if_link(struct iface *i)
{
  node *x, *y;

  WALK_LIST_DELSAFE(x, y, i->neighbors)
    {
      neighbor *n = SKIP_BACK(neighbor, if_n, x);
      if (n->proto->neigh_notify && n->proto->core_state != FS_FLUSHING)
	n->proto->neigh_notify(n);
    }
}

/**
 * neigh_ifa_update: notify neighbor cache about interface address add or remove event
 * @ifa: interface address in question
 *
 * Tell the neighbor cache that an address was added or removed.
 *
 * The neighbor cache wakes up all inactive sticky neighbors with
 * addresses belonging to prefixes of the interface belonging to @ifa
 * and causes all unreachable neighbors to be flushed.
 */
void
neigh_ifa_update(struct ifa *a)
{
  struct iface *i = a->iface;
  node *x, *y;
 
  /* Remove all neighbors whose scope has changed */
  WALK_LIST_DELSAFE(x, y, i->neighbors)
    {
      struct ifa *aa;
      neighbor *n = SKIP_BACK(neighbor, if_n, x);
      if (if_connected(&n->addr, i, &aa) != n->scope)
	neigh_down(n);
    }

  /* Wake up all sticky neighbors that are reachable now */
  neigh_if_up(i);
}

static inline void
neigh_prune_one(neighbor *n)
{
  if (n->proto->proto_state != PS_DOWN)
    return;
  rem_node(&n->n);
  if (n->scope >= 0)
    rem_node(&n->if_n);
  sl_free(neigh_slab, n);
}

/**
 * neigh_prune - prune neighbor cache
 *
 * neigh_prune() examines all neighbor entries cached and removes those
 * corresponding to inactive protocols. It's called whenever a protocol
 * is shut down to get rid of all its heritage.
 */
void
neigh_prune(void)
{
  neighbor *n;
  node *m;
  int i;

  DBG("Pruning neighbors\n");
  for(i=0; i<NEIGH_HASH_SIZE; i++)
    WALK_LIST_DELSAFE(n, m, neigh_hash_table[i])
      neigh_prune_one(n);
  WALK_LIST_DELSAFE(n, m, sticky_neigh_list)
    neigh_prune_one(n);
}

/**
 * neigh_init - initialize the neighbor cache.
 * @if_pool: resource pool to be used for neighbor entries.
 *
 * This function is called during BIRD startup to initialize
 * the neighbor cache module.
 */
void
neigh_init(pool *if_pool)
{
  int i;

  neigh_slab = sl_new(if_pool, sizeof(neighbor));
  init_list(&sticky_neigh_list);
  for(i=0; i<NEIGH_HASH_SIZE; i++)
    init_list(&neigh_hash_table[i]);
}
