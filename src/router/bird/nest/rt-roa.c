/*
 *	BIRD -- Route Origin Authorization
 *
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/route.h"
#include "nest/cli.h"
#include "lib/lists.h"
#include "lib/resource.h"
#include "lib/event.h"
#include "lib/string.h"
#include "conf/conf.h"


pool *roa_pool;
static slab *roa_slab;			/* Slab of struct roa_item */
static list roa_table_list;		/* List of struct roa_table */
struct roa_table *roa_table_default;	/* The first ROA table in the config */

static inline int
src_match(struct roa_item *it, byte src)
{ return !src || it->src == src; }

/**
 * roa_add_item - add a ROA entry
 * @t: ROA table
 * @prefix: prefix of the ROA entry
 * @pxlen: prefix length of the ROA entry
 * @maxlen: max length field of the ROA entry
 * @asn: AS number field of the ROA entry
 * @src: source of the ROA entry (ROA_SRC_*)
 *
 * The function adds a new ROA entry to the ROA table. If the same ROA
 * is already in the table, nothing is added. @src field is used to
 * distinguish different sources of ROAs.
 */
void
roa_add_item(struct roa_table *t, ip_addr prefix, byte pxlen, byte maxlen, u32 asn, byte src)
{
  struct roa_node *n = fib_get(&t->fib, &prefix, pxlen);

  // if ((n->items == NULL) && (n->n.x0 != ROA_INVALID))
  // t->cached_items--;

  struct roa_item *it;
  for (it = n->items; it; it = it->next)
    if ((it->maxlen == maxlen) && (it->asn == asn) && src_match(it, src))
      return;

  it = sl_alloc(roa_slab);
  it->asn = asn;
  it->maxlen = maxlen;
  it->src = src;
  it->next = n->items;
  n->items = it;
}

/**
 * roa_delete_item - delete a ROA entry
 * @t: ROA table
 * @prefix: prefix of the ROA entry
 * @pxlen: prefix length of the ROA entry
 * @maxlen: max length field of the ROA entry
 * @asn: AS number field of the ROA entry
 * @src: source of the ROA entry (ROA_SRC_*)
 *
 * The function removes a specified ROA entry from the ROA table and
 * frees it. If @src field is not ROA_SRC_ANY, only entries from
 * that source are considered.
 */
void
roa_delete_item(struct roa_table *t, ip_addr prefix, byte pxlen, byte maxlen, u32 asn, byte src)
{
  struct roa_node *n = fib_find(&t->fib, &prefix, pxlen);

  if (!n)
    return;

  struct roa_item *it, **itp;
  for (itp = &n->items; it = *itp; itp = &it->next)
    if ((it->maxlen == maxlen) && (it->asn == asn) && src_match(it, src))
      break;

  if (!it)
    return;

  *itp = it->next;
  sl_free(roa_slab, it);

  // if ((n->items == NULL) && (n->n.x0 != ROA_INVALID))
  // t->cached_items++;
}


/**
 * roa_flush - flush a ROA table
 * @t: ROA table
 * @src: source of ROA entries (ROA_SRC_*)
 *
 * The function removes and frees ROA entries from the ROA table. If
 * @src is ROA_SRC_ANY, all entries in the table are removed,
 * otherwise only all entries from that source are removed.
 */
void
roa_flush(struct roa_table *t, byte src)
{
  struct roa_item *it, **itp;
  struct roa_node *n;

  FIB_WALK(&t->fib, fn)
    {
      n = (struct roa_node *) fn;

      itp = &n->items;
      while (it = *itp)
	if (src_match(it, src))
	  {
	    *itp = it->next;
	    sl_free(roa_slab, it);
	  }
	else
	  itp = &it->next;
    }
  FIB_WALK_END;

  // TODO add cleanup of roa_nodes
}



/*
byte
roa_check(struct roa_table *t, ip_addr prefix, byte pxlen, u32 asn)
{
  struct roa_node *n = fib_find(&t->fib, &prefix, pxlen);

  if (n && n->n.x0 == ROA_UNKNOWN)
    return ROA_UNKNOWN;

  if (n && n->n.x0 == ROA_VALID && asn == n->cached_asn)
    return ROA_VALID;

  byte rv = roa_match(t, n, prefix, pxlen, asn);

  if (rv != ROA_INVALID)
    {
      if (!n)
	{
	  if (t->cached_items >= t->cached_items_max)
	  n = fib_get(&t->fib, &prefix, pxlen);
	  t->cached_items++;
	}

      n->cached_asn = asn;
      n->n.x0 = rv;
    }

  return rv;
}
*/

/**
 * roa_check - check validity of route origination in a ROA table 
 * @t: ROA table
 * @prefix: network prefix to check
 * @pxlen: length of network prefix
 * @asn: AS number of network prefix
 *
 * Implements RFC 6483 route validation for the given network
 * prefix. The procedure is to find all candidate ROAs - ROAs whose
 * prefixes cover the give network prefix. If there is no candidate
 * ROA, return ROA_UNKNOWN. If there is a candidate ROA with matching
 * ASN and maxlen field greater than or equal to the given prefix
 * length, return ROA_VALID. Otherwise return ROA_INVALID. If caller
 * cannot determine origin AS, 0 could be used (in that case ROA_VALID
 * cannot happen).
 */
byte
roa_check(struct roa_table *t, ip_addr prefix, byte pxlen, u32 asn)
{
  struct roa_node *n;
  ip_addr px;
  byte anything = 0;

  int len;
  for (len = pxlen; len >= 0; len--)
    {
      px = ipa_and(prefix, ipa_mkmask(len));
      n = fib_find(&t->fib, &px, len);

      if (!n)
	continue;

      struct roa_item *it;
      for (it = n->items; it; it = it->next)
	{
	  anything = 1;
	  if ((it->maxlen >= pxlen) && (it->asn == asn) && asn)
	    return ROA_VALID;
	}
    }

  return anything ? ROA_INVALID : ROA_UNKNOWN;
}

static void
roa_node_init(struct fib_node *fn)
{
  struct roa_node *n = (struct roa_node *) fn;
  n->items = NULL;
}

static inline void
roa_populate(struct roa_table *t)
{
  struct roa_item_config *ric;
  for (ric = t->cf->roa_items; ric; ric = ric->next)
    roa_add_item(t, ric->prefix, ric->pxlen, ric->maxlen, ric->asn, ROA_SRC_CONFIG);
}

static void
roa_new_table(struct roa_table_config *cf)
{
  struct roa_table *t;

  t = mb_allocz(roa_pool, sizeof(struct roa_table));
  fib_init(&t->fib, roa_pool, sizeof(struct roa_node), 0, roa_node_init);
  t->name = cf->name;
  t->cf = cf;

  cf->table = t;
  add_tail(&roa_table_list, &t->n);

  roa_populate(t);
}

struct roa_table_config *
roa_new_table_config(struct symbol *s)
{
  struct roa_table_config *rtc = cfg_allocz(sizeof(struct roa_table_config));

  cf_define_symbol(s, SYM_ROA, rtc);
  rtc->name = s->name;
  add_tail(&new_config->roa_tables, &rtc->n);
  return rtc;
}

/**
 * roa_add_item_config - add a static ROA entry to a ROA table configuration
 *
 * Arguments are self-explanatory. The first is the ROA table config, rest
 * are specifying the ROA entry.
 */
void
roa_add_item_config(struct roa_table_config *rtc, ip_addr prefix, byte pxlen, byte maxlen, u32 asn)
{
  struct roa_item_config *ric = cfg_allocz(sizeof(struct roa_item_config));

  ric->prefix = prefix;
  ric->pxlen = pxlen;
  ric->maxlen = maxlen;
  ric->asn = asn;
  ric->next = rtc->roa_items;
  rtc->roa_items = ric;
}

/**
 * roa_init - initialize ROA tables
 *
 * This function is called during BIRD startup. It initializes
 * the ROA table module.
 */
void
roa_init(void)
{
  roa_pool = rp_new(&root_pool, "ROA tables");
  roa_slab = sl_new(roa_pool, sizeof(struct roa_item));
  init_list(&roa_table_list);
}

void
roa_preconfig(struct config *c)
{
  init_list(&c->roa_tables);
}


/**
 * roa_commit - commit new ROA table configuration
 * @new: new configuration
 * @old: original configuration or %NULL if it's boot time config
 *
 * Scan differences between @old and @new configuration and modify the
 * ROA tables according to these changes. If @new defines a previously
 * unknown table, create it, if it omits a table existing in @old,
 * delete it (there are no references, only indirect through struct
 * roa_table_config). If it exists in both configurations, update the
 * configured ROA entries.
 */
void
roa_commit(struct config *new, struct config *old)
{
  struct roa_table_config *cf;
  struct roa_table *t;

  if (old)
    WALK_LIST(t, roa_table_list)
      {
	struct symbol *sym = cf_find_symbol(t->name);
	if (sym && sym->class == SYM_ROA)
	  {
	    /* Found old table in new config */
	    cf = sym->def;
	    cf->table = t;
	    t->name = cf->name;
	    t->cf = cf;

	    /* Reconfigure it */
	    roa_flush(t, ROA_SRC_CONFIG);
	    roa_populate(t);
	  }
	else
	  {
	    t->cf->table = NULL;

	    /* Free it now */
	    roa_flush(t, ROA_SRC_ANY);
	    rem_node(&t->n);
	    fib_free(&t->fib);
	    mb_free(t);
	  }
      }

  /* Add new tables */
  WALK_LIST(cf, new->roa_tables)
    if (! cf->table)
      roa_new_table(cf);

  roa_table_default = EMPTY_LIST(new->roa_tables) ? NULL :
    ((struct roa_table_config *) HEAD(new->roa_tables))->table;
}



static void
roa_show_node(struct cli *c, struct roa_node *rn, int len, u32 asn)
{
  struct roa_item *ri;

  for (ri = rn->items; ri; ri = ri->next)
    if ((ri->maxlen >= len) && (!asn || (ri->asn == asn)))
      cli_printf(c, -1019, "%I/%d max %d as %u", rn->n.prefix, rn->n.pxlen, ri->maxlen, ri->asn);
}

static void
roa_show_cont(struct cli *c)
{
  struct roa_show_data *d = c->rover;
  struct fib *fib = &d->table->fib;
  struct fib_iterator *it = &d->fit;
  struct roa_node *rn;
  unsigned max = 32;

  FIB_ITERATE_START(fib, it, f)
    {
      rn = (struct roa_node *) f;

      if (!max--)
	{
	  FIB_ITERATE_PUT(it, f);
	  return;
	}

      if ((d->mode == ROA_SHOW_ALL) ||
	  net_in_net(rn->n.prefix, rn->n.pxlen, d->prefix, d->pxlen))
	roa_show_node(c, rn, 0, d->asn);
    }
  FIB_ITERATE_END(f);

  cli_printf(c, 0, "");
  c->cont = c->cleanup = NULL;
}

static void
roa_show_cleanup(struct cli *c)
{
  struct roa_show_data *d = c->rover;

  /* Unlink the iterator */
  fit_get(&d->table->fib, &d->fit);
}

void
roa_show(struct roa_show_data *d)
{
  struct roa_node *rn;
  ip_addr px;
  int len;

  switch (d->mode)
    {
    case ROA_SHOW_ALL:
    case ROA_SHOW_IN:
      FIB_ITERATE_INIT(&d->fit, &d->table->fib);
      this_cli->cont = roa_show_cont;
      this_cli->cleanup = roa_show_cleanup;
      this_cli->rover = d;
      break;

    case ROA_SHOW_PX:
      rn = fib_find(&d->table->fib, &d->prefix, d->pxlen);
      if (rn)
	{
	  roa_show_node(this_cli, rn, 0, d->asn);
	  cli_msg(0, "");
	}
      else
	cli_msg(-8001, "Network not in table");
      break;

    case ROA_SHOW_FOR:
      for (len = d->pxlen; len >= 0; len--)
	{
	  px = ipa_and(d->prefix, ipa_mkmask(len));
	  rn = fib_find(&d->table->fib, &px, len);

	  if (!rn)
	    continue;

	  roa_show_node(this_cli, rn, 0, d->asn);
	}
      cli_msg(0, "");
      break;
    }
}
