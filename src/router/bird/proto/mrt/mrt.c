/*
 *	BIRD -- Multi-Threaded Routing Toolkit (MRT) Protocol
 *
 *	(c) 2017--2018 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2017--2018 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Multi-Threaded Routing Toolkit (MRT) protocol
 *
 * The MRT protocol is implemented in just one file: |mrt.c|. It contains of
 * several parts: Generic functions for preparing MRT messages in a buffer,
 * functions for MRT table dump (called from timer or CLI), functions for MRT
 * BGP4MP dump (called from BGP), and the usual protocol glue. For the MRT table
 * dump, the key structure is struct mrt_table_dump_state, which contains all
 * necessary data and created when the MRT dump cycle is started for the
 * duration of the MRT dump. The MBGP4MP dump is currently not bound to MRT
 * protocol instance and uses the config->mrtdump_file fd.
 *
 * The protocol is simple, just periodically scans routing table and export it
 * to a file. It does not use the regular update mechanism, but a direct access
 * in order to handle iteration through multiple routing tables. The table dump
 * needs to dump all peers first and then use indexes to address the peers, we
 * use a hash table (@peer_hash) to find peer index based on BGP protocol key
 * attributes.
 *
 * One thing worth documenting is the locking. During processing, the currently
 * processed table (@table field in the state structure) is locked and also the
 * explicitly named table is locked (@table_ptr field in the state structure) if
 * specified. Between dumps no table is locked. Also the current config is
 * locked (by config_add_obstacle()) during table dumps as some data (strings,
 * filters) are shared from the config and the running table dump may be
 * interrupted by reconfiguration.
 *
 * Supported standards:
 * - RFC 6396 - MRT format standard
 * - RFC 8050 - ADD_PATH extension
 */

#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "mrt.h"

#include "nest/cli.h"
#include "filter/filter.h"
#include "proto/bgp/bgp.h"
#include "sysdep/unix/unix.h"


#ifdef PATH_MAX
#define BIRD_PATH_MAX PATH_MAX
#else
#define BIRD_PATH_MAX 4096
#endif

#define mrt_log(s, msg, args...)				\
  ({								\
    if (s->cli)							\
      cli_printf(s->cli, -8009, msg, ## args);			\
    if (s->proto)						\
      log(L_ERR "%s: " msg, s->proto->p.name, ## args);		\
  })


/*
 *	MRT buffer code
 */

static void
mrt_buffer_init(buffer *b, pool *pool, size_t n)
{
  b->start = mb_alloc(pool, n);
  b->pos = b->start;
  b->end = b->start + n;
}

static void
mrt_buffer_grow(buffer *b, size_t n)
{
  size_t used = b->pos - b->start;
  size_t size = b->end - b->start;
  size_t req = used + n;

  while (size < req)
    size = size * 3 / 2;

  b->start = mb_realloc(b->start, size);
  b->pos = b->start + used;
  b->end = b->start + size;
}

static inline void
mrt_buffer_need(buffer *b, size_t n)
{
  if (b->pos + n > b->end)
    mrt_buffer_grow(b, n);
}

static inline uint
mrt_buffer_pos(buffer *b)
{
  return b->pos - b->start;
}

static inline void
mrt_buffer_flush(buffer *b)
{
  b->pos = b->start;
}

#define MRT_DEFINE_TYPE(S, T)						\
  static inline void mrt_put_##S##_(buffer *b, T x)			\
  {									\
    put_##S(b->pos, x);							\
    b->pos += sizeof(T);						\
  }									\
									\
  static inline void mrt_put_##S(buffer *b, T x)			\
  {									\
    mrt_buffer_need(b, sizeof(T));					\
    put_##S(b->pos, x);							\
    b->pos += sizeof(T);						\
  }

MRT_DEFINE_TYPE(u8, u8)
MRT_DEFINE_TYPE(u16, u16)
MRT_DEFINE_TYPE(u32, u32)
MRT_DEFINE_TYPE(u64, u64)
MRT_DEFINE_TYPE(ip4, ip4_addr)
MRT_DEFINE_TYPE(ip6, ip6_addr)

static inline void
mrt_put_ipa(buffer *b, ip_addr x)
{
  if (ipa_is_ip4(x))
    mrt_put_ip4(b, ipa_to_ip4(x));
  else
    mrt_put_ip6(b, ipa_to_ip6(x));
}

static inline void
mrt_put_data(buffer *b, const void *src, size_t n)
{
  if (!n)
    return;

  mrt_buffer_need(b, n);
  memcpy(b->pos, src, n);
  b->pos += n;
}

static void
mrt_init_message(buffer *b, u16 type, u16 subtype)
{
  /* Reset buffer */
  mrt_buffer_flush(b);
  mrt_buffer_need(b, MRT_HDR_LENGTH);

  /* Prepare header */
  mrt_put_u32_(b, now_real);
  mrt_put_u16_(b, type);
  mrt_put_u16_(b, subtype);

  /* Message length, will be fixed later */
  mrt_put_u32_(b, 0);
}

static void
mrt_dump_message(buffer *b, int fd)
{
  uint len = mrt_buffer_pos(b);

  /* Fix message length */
  ASSERT(len >= MRT_HDR_LENGTH);
  put_u32(b->start + 8, len - MRT_HDR_LENGTH);

  if (fd < 0)
    return;

  if (write(fd, b->start, len) < 0)
    log(L_ERR "Write to MRT file failed: %m"); /* TODO: name of file */
}

static int
bstrsub(char *dst, size_t n, const char *src, const char *key, const char *val)
{
  const char *last, *next;
  char *pos = dst;
  size_t step, klen = strlen(key), vlen = strlen(val);

  for (last = src; next = strstr(last, key); last = next + klen)
  {
    step = next - last;
    if (n <= step + vlen)
      return 0;

    memcpy(pos, last, step);
    ADVANCE(pos, n, step);

    memcpy(pos, val, vlen);
    ADVANCE(pos, n, vlen);
  }

  step = strlen(last);
  if (n <= step)
    return 0;

  memcpy(pos, last, step);
  ADVANCE(pos, n, step);

  pos[0] = 0;
  return 1;
}

static inline rtable *
mrt_next_table_(rtable *tab, rtable *tab_ptr, const char *pattern)
{
  /* Handle explicit table, return it in the first pass */
  if (tab_ptr)
    return !tab ? tab_ptr : NULL;

  /* Walk routing_tables list, starting after tab (if non-NULL) */
  for (tab = !tab ? HEAD(routing_tables) : NODE_NEXT(tab);
       NODE_VALID(tab);
       tab = NODE_NEXT(tab))
    if (patmatch(pattern, tab->name))
      return tab;

  return NULL;
}

static rtable *
mrt_next_table(struct mrt_table_dump_state *s)
{
  rtable *tab = mrt_next_table_(s->table, s->table_ptr, s->table_expr);

  if (s->table)
    rt_unlock_table(s->table);

  s->table = tab;

  if (s->table)
    rt_lock_table(s->table);

  return s->table;
}

static int
mrt_open_file(struct mrt_table_dump_state *s)
{
  char fmt1[BIRD_PATH_MAX];
  char name[BIRD_PATH_MAX];

  if (!bstrsub(fmt1, sizeof(fmt1), s->filename, "%N", s->table->name) ||
      !tm_format_real_time(name, sizeof(name), fmt1, now_real))
  {
    mrt_log(s, "Invalid filename '%s'", s->filename);
    return 0;
  }

  s->file = rf_open(s->pool, name, "a");
  if (!s->file)
  {
    mrt_log(s, "Unable to open MRT file '%s': %m", name);
    return 0;
  }

  s->fd = rf_fileno(s->file);
  s->time_offset = now_real - now;

  return 1;
}

static void
mrt_close_file(struct mrt_table_dump_state *s)
{
  rfree(s->file);
  s->file = NULL;
  s->fd = -1;
}


/*
 *	MRT Table Dump: Peer Index Table
 */

#define PEER_KEY(n)		n->peer_id, n->peer_as, n->peer_ip
#define PEER_NEXT(n)		n->next
#define PEER_EQ(id1,as1,ip1,id2,as2,ip2) \
  id1 == id2 && as1 == as2 && ipa_equal(ip1, ip2)
#define PEER_FN(id,as,ip)	ipa_hash(ip)

static void
mrt_peer_table_header(struct mrt_table_dump_state *s, u32 router_id, const char *name)
{
  buffer *b = &s->buf;

  /* Collector BGP ID */
  mrt_put_u32(b, router_id);

  /* View Name */
  uint name_length = name ? strlen(name) : 0;
  name_length = MIN(name_length, 65535);
  mrt_put_u16(b, name_length);
  mrt_put_data(b, name, name_length);

  /* Peer Count, will be fixed later */
  s->peer_count = 0;
  s->peer_count_offset = mrt_buffer_pos(b);
  mrt_put_u16(b, 0);

  HASH_INIT(s->peer_hash, s->pool, 10);
}

static void
mrt_peer_table_entry(struct mrt_table_dump_state *s, u32 peer_id, u32 peer_as, ip_addr peer_ip)
{
  buffer *b = &s->buf;

  uint type = MRT_PEER_TYPE_32BIT_ASN;
  if (ipa_is_ip6(peer_ip))
    type |= MRT_PEER_TYPE_IPV6;

  /* Dump peer to buffer */
  mrt_put_u8(b, type);
  mrt_put_u32(b, peer_id);
  mrt_put_ipa(b, peer_ip);
  mrt_put_u32(b, peer_as);

  /* Add peer to hash table */
  struct mrt_peer_entry *n = lp_allocz(s->peer_lp, sizeof(struct mrt_peer_entry));
  n->peer_id = peer_id;
  n->peer_as = peer_as;
  n->peer_ip = peer_ip;
  n->index = s->peer_count++;

  HASH_INSERT(s->peer_hash, PEER, n);
}

static void
mrt_peer_table_dump(struct mrt_table_dump_state *s)
{
  mrt_init_message(&s->buf, MRT_TABLE_DUMP_V2, MRT_PEER_INDEX_TABLE);
  mrt_peer_table_header(s, config->router_id, s->table->name);

  /* 0 is fake peer for non-BGP routes */
  mrt_peer_table_entry(s, 0, 0, IPA_NONE);

#ifdef CONFIG_BGP
  struct proto *P;
  WALK_LIST(P, active_proto_list)
    if (P->proto == &proto_bgp)
    {
      struct bgp_proto *p = (void *) P;
      mrt_peer_table_entry(s, p->remote_id, p->remote_as, p->cf->remote_ip);
    }
#endif

  /* Fix Peer Count */
  put_u16(s->buf.start + s->peer_count_offset, s->peer_count);

  mrt_dump_message(&s->buf, s->fd);
}

static void
mrt_peer_table_flush(struct mrt_table_dump_state *s)
{
  lp_flush(s->peer_lp);
  HASH_FREE(s->peer_hash);
}


/*
 *	MRT Table Dump: RIB Table
 */

static void
mrt_rib_table_header(struct mrt_table_dump_state *s, net *n)
{
  buffer *b = &s->buf;

  /* Sequence Number */
  mrt_put_u32(b, s->seqnum);

  /* Network Prefix */
  ip_addr a = n->n.prefix;
  ipa_hton(a);

  mrt_put_u8(b, n->n.pxlen);
  mrt_put_data(b, &a, BYTES(n->n.pxlen));

  /* Entry Count, will be fixed later */
  s->entry_count = 0;
  s->entry_count_offset = mrt_buffer_pos(b);
  mrt_put_u16(b, 0);
}

static void
mrt_rib_table_entry(struct mrt_table_dump_state *s, rte *r, struct ea_list *tmpa)
{
  buffer *b = &s->buf;
  uint peer = 0;

#ifdef CONFIG_BGP
  /* Find peer index */
  if (r->attrs->src->proto->proto == &proto_bgp)
  {
    struct bgp_proto *p = (void *) r->attrs->src->proto;
    struct mrt_peer_entry *n =
      HASH_FIND(s->peer_hash, PEER, p->remote_id, p->remote_as, p->cf->remote_ip);

    peer = n ? n->index : 0;
  }
#endif

  /* Peer Index and Originated Time */
  mrt_put_u16(b, peer);
  mrt_put_u32(b, r->lastmod + s->time_offset);

  /* Path Identifier */
  if (s->add_path)
    mrt_put_u32(b, r->attrs->src->private_id);

  /* Route Attributes */
  mrt_put_u16(b, 0);

#ifdef CONFIG_BGP
  if (r->attrs->eattrs || tmpa)
  {
    struct ea_list *eattrs = r->attrs->eattrs;

    if (!rta_is_cached(r->attrs) || tmpa)
    {
      /* Attributes must be merged and sorted for bgp_encode_attrs() */
      tmpa = ea_append(tmpa, eattrs);
      eattrs = alloca(ea_scan(tmpa));
      ea_merge(tmpa, eattrs);
      ea_sort(eattrs);
    }

    mrt_buffer_need(b, MRT_ATTR_BUFFER_SIZE);
    int alen = bgp_encode_attrs(NULL, b->pos, eattrs, MRT_ATTR_BUFFER_SIZE);

    if (alen < 0)
    {
      mrt_log(s, "Attribute list too long for %I/%d",
	      r->net->n.prefix, r->net->n.pxlen);
      alen = 0;
    }

    put_u16(b->pos - 2, alen);
    b->pos += alen;
  }
#endif

  s->entry_count++;
}

static void
mrt_rib_table_dump(struct mrt_table_dump_state *s, net *n, int add_path)
{
  rte *rt, *rt0;
  int subtype;

  s->add_path = add_path;

#ifndef IPV6
  subtype = !add_path ? MRT_RIB_IPV4_UNICAST : MRT_RIB_IPV4_UNICAST_ADDPATH;
#else
  subtype = !add_path ? MRT_RIB_IPV6_UNICAST : MRT_RIB_IPV6_UNICAST_ADDPATH;
#endif

  mrt_init_message(&s->buf, MRT_TABLE_DUMP_V2, subtype);
  mrt_rib_table_header(s, n);

  for (rt0 = n->routes; rt = rt0; rt0 = rt0->next)
  {
    if (rte_is_filtered(rt))
      continue;

    /* Skip routes that should be reported in the other phase */
    if (!s->always_add_path && (!rt->attrs->src->private_id != !s->add_path))
    {
      s->want_add_path = 1;
      continue;
    }

    struct ea_list *tmp_attrs = rte_make_tmp_attrs(rt, s->linpool);

    if (f_run(s->filter, &rt, &tmp_attrs, s->linpool, 0) <= F_ACCEPT)
      mrt_rib_table_entry(s, rt, tmp_attrs);

    if (rt != rt0)
      rte_free(rt);

    lp_flush(s->linpool);
  }

  /* Fix Entry Count */
  put_u16(s->buf.start + s->entry_count_offset, s->entry_count);

  /* Update max counter */
  s->max -= 1 + s->entry_count;

  /* Skip empty entries */
  if (!s->entry_count)
    return;

  s->seqnum++;
  mrt_dump_message(&s->buf, s->fd);
}


/*
 *	MRT Table Dump: main logic
 */

static struct mrt_table_dump_state *
mrt_table_dump_init(pool *pp)
{
  pool *pool = rp_new(pp, "MRT Table Dump");
  struct mrt_table_dump_state *s = mb_allocz(pool, sizeof(struct mrt_table_dump_state));

  s->pool = pool;
  s->linpool = lp_new(pool, 4080);
  s->peer_lp = lp_new(pool, 4080);
  mrt_buffer_init(&s->buf, pool, 2 * MRT_ATTR_BUFFER_SIZE);

  /* We lock the current config as we may reference it indirectly by filter */
  s->config = config;
  config_add_obstacle(s->config);

  s->fd = -1;

  return s;
}

static void
mrt_table_dump_free(struct mrt_table_dump_state *s)
{
  if (s->table_open)
    FIB_ITERATE_UNLINK(&s->fit, &s->table->fib);

  if (s->table)
    rt_unlock_table(s->table);

  if (s->table_ptr)
    rt_unlock_table(s->table_ptr);

  config_del_obstacle(s->config);

  rfree(s->pool);
}


static int
mrt_table_dump_step(struct mrt_table_dump_state *s)
{
  s->max = 2048;

  if (s->table_open)
    goto step;

  while (mrt_next_table(s))
  {
    if (!mrt_open_file(s))
      continue;

    mrt_peer_table_dump(s);

    FIB_ITERATE_INIT(&s->fit, &s->table->fib);
    s->table_open = 1;

  step:
    FIB_ITERATE_START(&s->table->fib, &s->fit, fn)
    {
      if (s->max < 0)
      {
	FIB_ITERATE_PUT(&s->fit, fn);
	return 0;
      }

      /* With Always ADD_PATH option, we jump directly to second phase */
      s->want_add_path = s->always_add_path;

      if (s->want_add_path == 0)
	mrt_rib_table_dump(s, (net *) fn, 0);

      if (s->want_add_path == 1)
	mrt_rib_table_dump(s, (net *) fn, 1);
    }
    FIB_ITERATE_END(fn);
    s->table_open = 0;

    mrt_close_file(s);
    mrt_peer_table_flush(s);
  }

  return 1;
}

static void
mrt_timer(timer *t)
{
  struct mrt_proto *p = t->data;
  struct mrt_config *cf = (void *) (p->p.cf);

  if (p->table_dump)
  {
    log(L_WARN "%s: Earlier RIB table dump still not finished, skipping next one", p->p.name);
    return;
  }

  TRACE(D_EVENTS, "RIB table dump started");

  struct mrt_table_dump_state *s = mrt_table_dump_init(p->p.pool);

  s->proto = p;
  s->table_expr = cf->table_expr;
  s->table_ptr = cf->table_cf ? cf->table_cf->table : NULL;
  s->filter = cf->filter;
  s->filename = cf->filename;
  s->always_add_path = cf->always_add_path;

  if (s->table_ptr)
    rt_lock_table(s->table_ptr);

  p->table_dump = s;
  ev_schedule(p->event);
}

static void
mrt_event(void *P)
{
  struct mrt_proto *p = P;

  if (!p->table_dump)
    return;

  if (!mrt_table_dump_step(p->table_dump))
  {
    ev_schedule(p->event);
    return;
  }

  mrt_table_dump_free(p->table_dump);
  p->table_dump = NULL;

  TRACE(D_EVENTS, "RIB table dump done");

  if (p->p.proto_state == PS_STOP)
    proto_notify_state(&p->p, PS_DOWN);
}


/*
 *	MRT Table Dump: CLI command
 */

static void
mrt_dump_cont(struct cli *c)
{
  if (!mrt_table_dump_step(c->rover))
    return;

  cli_printf(c, 0, "");
  mrt_table_dump_free(c->rover);
  c->cont = c->cleanup = c->rover = NULL;
}

static void
mrt_dump_cleanup(struct cli *c)
{
  mrt_table_dump_free(c->rover);
  c->rover = NULL;
}

void
mrt_dump_cmd(struct mrt_dump_data *d)
{
  if (cli_access_restricted())
    return;

  if (!d->table_expr && !d->table_ptr)
    cf_error("Table not specified");

  if (!d->filename)
    cf_error("File not specified");

  struct mrt_table_dump_state *s = mrt_table_dump_init(this_cli->pool);

  s->cli = this_cli;
  s->table_expr = d->table_expr;
  s->table_ptr = d->table_ptr;
  s->filter = d->filter;
  s->filename = d->filename;

  if (s->table_ptr)
    rt_lock_table(s->table_ptr);

  this_cli->cont = mrt_dump_cont;
  this_cli->cleanup = mrt_dump_cleanup;
  this_cli->rover = s;
}


/*
 *	MRT BGP4MP dump
 */

static buffer *
mrt_bgp_buffer(void)
{
  /* Static buffer for BGP4MP dump, TODO: change to use MRT protocol */
  static buffer b;

  if (!b.start)
    mrt_buffer_init(&b, &root_pool, 1024);

  return &b;
}

static void
mrt_bgp_header(buffer *b, struct mrt_bgp_data *d)
{
  if (d->as4)
  {
    mrt_put_u32(b, d->peer_as);
    mrt_put_u32(b, d->local_as);
  }
  else
  {
    mrt_put_u16(b, (d->peer_as <= 0xFFFF) ? d->peer_as : AS_TRANS);
    mrt_put_u16(b, (d->local_as <= 0xFFFF) ? d->local_as : AS_TRANS);
  }

  mrt_put_u16(b, (d->index <= 0xFFFF) ? d->index : 0);
  mrt_put_u16(b, d->af);

  if (d->af == BGP_AF_IPV4)
  {
    mrt_put_ip4(b, ipa_to_ip4(d->peer_ip));
    mrt_put_ip4(b, ipa_to_ip4(d->local_ip));
  }
  else
  {
    mrt_put_ip6(b, ipa_to_ip6(d->peer_ip));
    mrt_put_ip6(b, ipa_to_ip6(d->local_ip));
  }
}

void
mrt_dump_bgp_message(struct mrt_bgp_data *d)
{
  const u16 subtypes[] = {
    MRT_BGP4MP_MESSAGE,			MRT_BGP4MP_MESSAGE_AS4,
    MRT_BGP4MP_MESSAGE_LOCAL,		MRT_BGP4MP_MESSAGE_AS4_LOCAL,
    MRT_BGP4MP_MESSAGE_ADDPATH,		MRT_BGP4MP_MESSAGE_AS4_ADDPATH,
    MRT_BGP4MP_MESSAGE_LOCAL_ADDPATH,	MRT_BGP4MP_MESSAGE_AS4_LOCAL_ADDPATH,
  };

  buffer *b = mrt_bgp_buffer();
  mrt_init_message(b, MRT_BGP4MP, subtypes[d->as4 + 4*d->add_path]);
  mrt_bgp_header(b, d);
  mrt_put_data(b, d->message, d->msg_len);
  mrt_dump_message(b, config->mrtdump_file);
}

void
mrt_dump_bgp_state_change(struct mrt_bgp_data *d)
{
  /* Convert state from our BS_* values to values used in MRTDump */
  const u16 states[BS_MAX] = {1, 2, 3, 4, 5, 6, 1};

  if (states[d->old_state] == states[d->new_state])
    return;

  /* Always use AS4 mode for STATE_CHANGE */
  d->as4 = 1;

  buffer *b = mrt_bgp_buffer();
  mrt_init_message(b, MRT_BGP4MP, MRT_BGP4MP_STATE_CHANGE_AS4);
  mrt_bgp_header(b, d);
  mrt_put_u16(b, states[d->old_state]);
  mrt_put_u16(b, states[d->new_state]);
  mrt_dump_message(b, config->mrtdump_file);
}


/*
 *	MRT protocol glue
 */

void
mrt_check_config(struct proto_config *C)
{
  struct mrt_config *cf = (void *) C;

  /* c.table must be always defined, but it is relevant only if table_expr is not set */
  if (!cf->table_expr)
    cf->table_cf = cf->c.table;

  if (!cf->table_expr && !cf->table_cf)
    cf_error("Table not specified");

  if (!cf->filename)
    cf_error("File not specified");

  if (!cf->period)
    cf_error("Period not specified");
}

static struct proto *
mrt_init(struct proto_config *C)
{
  struct proto *P = proto_new(C, sizeof(struct mrt_proto));

  return P;
}

static int
mrt_start(struct proto *P)
{
  struct mrt_proto *p = (void *) P;
  struct mrt_config *cf = (void *) (P->cf);

  p->timer = tm_new_set(P->pool, mrt_timer, p, 0, cf->period);
  p->event = ev_new_set(P->pool, mrt_event, p);

  tm_start(p->timer, cf->period);

  return PS_UP;
}

static int
mrt_shutdown(struct proto *P)
{
  struct mrt_proto *p = (void *) P;

  return p->table_dump ? PS_STOP : PS_DOWN;
}

static int
mrt_reconfigure(struct proto *P, struct proto_config *CF)
{
  struct mrt_proto *p = (void *) P;
  struct mrt_config *old = (void *) (P->cf);
  struct mrt_config *new = (void *) CF;

  if (new->period != old->period)
  {
    TRACE(D_EVENTS, "Changing period from %d to %d s", old->period, new->period);

    bird_clock_t new_time = p->timer->expires - old->period + new->period;
    tm_start(p->timer, (new_time > now) ? (new_time - now) : 0);
    p->timer->recurrent = new->period;
  }

  return 1;
}

static void
mrt_copy_config(struct proto_config *dest, struct proto_config *src)
{
  /* Just a shallow copy, not many items here */
  proto_copy_rest(dest, src, sizeof(struct mrt_config));
}


struct protocol proto_mrt = {
  .name =		"MRT",
  .template =		"mrt%d",
  .config_size =	sizeof(struct mrt_config),
  .init =		mrt_init,
  .start =		mrt_start,
  .shutdown =		mrt_shutdown,
  .reconfigure =	mrt_reconfigure,
  .copy_config =	mrt_copy_config,
};
