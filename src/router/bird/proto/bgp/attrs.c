/*
 *	BIRD -- BGP Attributes
 *
 *	(c) 2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#undef LOCAL_DEBUG

#include <stdlib.h>

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "nest/route.h"
#include "nest/attrs.h"
#include "conf/conf.h"
#include "lib/resource.h"
#include "lib/string.h"
#include "lib/unaligned.h"

#include "bgp.h"

static byte bgp_mandatory_attrs[] = { BA_ORIGIN, BA_AS_PATH
#ifndef IPV6
,BA_NEXT_HOP
#endif
};

struct attr_desc {
  char *name;
  int expected_length;
  int expected_flags;
  int type;
  int allow_in_ebgp;
  int (*validate)(struct bgp_proto *p, byte *attr, int len);
  void (*format)(eattr *ea, byte *buf);
};

static int
bgp_check_origin(struct bgp_proto *p UNUSED, byte *a UNUSED, int len)
{
  if (len > 2)
    return 6;
  return 0;
}

static void
bgp_format_origin(eattr *a, byte *buf)
{
  static char *bgp_origin_names[] = { "IGP", "EGP", "Incomplete" };

  bsprintf(buf, bgp_origin_names[a->u.data]);
}

static int
bgp_check_path(struct bgp_proto *p UNUSED, byte *a, int len)
{
  while (len)
    {
      DBG("Path segment %02x %02x\n", a[0], a[1]);
      if (len < 2 ||
	  a[0] != AS_PATH_SET && a[0] != AS_PATH_SEQUENCE ||
	  2*a[1] + 2 > len)
	return 11;
      len -= 2*a[1] + 2;
      a += 2*a[1] + 2;
    }
  return 0;
}

static int
bgp_check_next_hop(struct bgp_proto *p UNUSED, byte *a, int len)
{
#ifdef IPV6
  return -1;
#else
  ip_addr addr;

  memcpy(&addr, a, len);
  ipa_ntoh(addr);
  if (ipa_classify(addr) & IADDR_HOST)
    return 0;
  else
    return 8;
#endif
}

static int
bgp_check_reach_nlri(struct bgp_proto *p UNUSED, byte *a UNUSED, int len UNUSED)
{
#ifdef IPV6
  p->mp_reach_start = a;
  p->mp_reach_len = len;
#endif
  return -1;
}

static int
bgp_check_unreach_nlri(struct bgp_proto *p UNUSED, byte *a UNUSED, int len UNUSED)
{
#ifdef IPV6
  p->mp_unreach_start = a;
  p->mp_unreach_len = len;
#endif
  return -1;
}

static struct attr_desc bgp_attr_table[] = {
  { NULL, -1, 0, 0, 0,								/* Undefined */
    NULL, NULL },
  { "origin", 1, BAF_TRANSITIVE, EAF_TYPE_INT, 1,				/* BA_ORIGIN */
    bgp_check_origin, bgp_format_origin },
  { "as_path", -1, BAF_TRANSITIVE, EAF_TYPE_AS_PATH, 1,				/* BA_AS_PATH */
    bgp_check_path, NULL },
  { "next_hop", 4, BAF_TRANSITIVE, EAF_TYPE_IP_ADDRESS, 1,			/* BA_NEXT_HOP */
    bgp_check_next_hop, NULL },
  { "med", 4, BAF_OPTIONAL, EAF_TYPE_INT, 0,					/* BA_MULTI_EXIT_DISC */
    NULL, NULL },
  { "local_pref", 4, BAF_TRANSITIVE, EAF_TYPE_INT, 0,				/* BA_LOCAL_PREF */
    NULL, NULL },
  { "atomic_aggr", 0, BAF_TRANSITIVE, EAF_TYPE_OPAQUE, 1,			/* BA_ATOMIC_AGGR */
    NULL, NULL },
  { "aggregator", 6, BAF_OPTIONAL | BAF_TRANSITIVE, EAF_TYPE_OPAQUE, 1,		/* BA_AGGREGATOR */
    NULL, NULL },
  { "community", -1, BAF_OPTIONAL | BAF_TRANSITIVE, EAF_TYPE_INT_SET, 1,	/* BA_COMMUNITY */
    NULL, NULL },
  { NULL, },									/* BA_ORIGINATOR_ID */
  { NULL, },									/* BA_CLUSTER_LIST */
  { NULL, },									/* BA_DPA */
  { NULL, },									/* BA_ADVERTISER */
  { NULL, },									/* BA_RCID_PATH */
  { "mp_reach_nlri", -1, BAF_OPTIONAL, EAF_TYPE_OPAQUE, 1,			/* BA_MP_REACH_NLRI */
    bgp_check_reach_nlri, NULL },
  { "mp_unreach_nlri", -1, BAF_OPTIONAL, EAF_TYPE_OPAQUE, 1,			/* BA_MP_UNREACH_NLRI */
    bgp_check_unreach_nlri, NULL },
};

#define ATTR_KNOWN(code) ((code) < ARRAY_SIZE(bgp_attr_table) && bgp_attr_table[code].name)

static byte *
bgp_set_attr(eattr *e, struct linpool *pool, unsigned attr, unsigned val)
{
  ASSERT(ATTR_KNOWN(attr));
  e->id = EA_CODE(EAP_BGP, attr);
  e->type = bgp_attr_table[attr].type;
  e->flags = bgp_attr_table[attr].expected_flags;
  if (e->type & EAF_EMBEDDED)
    {
      e->u.data = val;
      return NULL;
    }
  else
    {
      e->u.ptr = lp_alloc(pool, sizeof(struct adata) + val);
      e->u.ptr->length = val;
      return e->u.ptr->data;
    }
}

byte *
bgp_attach_attr(ea_list **to, struct linpool *pool, unsigned attr, unsigned val)
{
  ea_list *a = lp_alloc(pool, sizeof(ea_list) + sizeof(eattr));
  a->next = *to;
  *to = a;
  a->flags = EALF_SORTED;
  a->count = 1;
  return bgp_set_attr(a->attrs, pool, attr, val);
}

/**
 * bgp_encode_attrs - encode BGP attributes
 * @w: buffer
 * @attrs: a list of extended attributes
 * @remains: remaining space in the buffer
 *
 * The bgp_encode_attrs() function takes a list of extended attributes
 * and converts it to its BGP representation (a part of an Update message).
 *
 * Result: Length of the attribute block generated.
 */
unsigned int
bgp_encode_attrs(byte *w, ea_list *attrs, int remains)
{
  unsigned int i, code, flags;
  byte *start = w;
  int len;

  for(i=0; i<attrs->count; i++)
    {
      eattr *a = &attrs->attrs[i];
      ASSERT(EA_PROTO(a->id) == EAP_BGP);
      code = EA_ID(a->id);
#ifdef IPV6
      /* When talking multiprotocol BGP, the NEXT_HOP attributes are used only temporarily. */
      if (code == BA_NEXT_HOP)
	continue;
#endif
      flags = a->flags & (BAF_OPTIONAL | BAF_TRANSITIVE | BAF_PARTIAL);
      if (ATTR_KNOWN(code))
	{
	  struct attr_desc *desc = &bgp_attr_table[code];
	  len = desc->expected_length;
	  if (len < 0)
	    {
	      ASSERT(!(a->type & EAF_EMBEDDED));
	      len = a->u.ptr->length;
	    }
	}
      else
	{
	  ASSERT((a->type & EAF_TYPE_MASK) == EAF_TYPE_OPAQUE);
	  len = a->u.ptr->length;
	}
      DBG("\tAttribute %02x (type %02x, %d bytes, flags %02x)\n", code, a->type, len, flags);
      if (remains < len + 4)
	{
	  log(L_ERR "BGP: attribute list too long, ignoring the remaining attributes");
	  break;
	}
      if (len < 256)
	{
	  *w++ = flags;
	  *w++ = code;
	  *w++ = len;
	  remains -= 3;
	}
      else
	{
	  *w++ = flags | BAF_EXT_LEN;
	  *w++ = code;
	  put_u16(w, len);
	  w += 2;
	  remains -= 4;
	}
      switch (a->type & EAF_TYPE_MASK)
	{
	case EAF_TYPE_INT:
	case EAF_TYPE_ROUTER_ID:
	  if (len == 4)
	    put_u32(w, a->u.data);
	  else
	    *w = a->u.data;
	  break;
	case EAF_TYPE_IP_ADDRESS:
	  {
	    ip_addr ip = *(ip_addr *)a->u.ptr->data;
	    ipa_hton(ip);
	    memcpy(w, &ip, len);
	    break;
	  }
	case EAF_TYPE_INT_SET:
	  {
	    u32 *z = (u32 *)a->u.ptr->data;
	    int i;
	    for(i=0; i<len; i+=4)
	      put_u32(w+i, *z++);
	    break;
	  }
	case EAF_TYPE_OPAQUE:
	case EAF_TYPE_AS_PATH:
	  memcpy(w, a->u.ptr->data, len);
	  break;
	default:
	  bug("bgp_encode_attrs: unknown attribute type %02x", a->type);
	}
      remains -= len;
      w += len;
    }
  return w - start;
}

static void
bgp_init_prefix(struct fib_node *N)
{
  struct bgp_prefix *p = (struct bgp_prefix *) N;
  p->bucket_node.next = NULL;
}

static int
bgp_compare_u32(const u32 *x, const u32 *y)
{
  return (*x < *y) ? -1 : (*x > *y) ? 1 : 0;
}

static void
bgp_normalize_set(u32 *dest, u32 *src, unsigned cnt)
{
  memcpy(dest, src, sizeof(u32) * cnt);
  qsort(dest, cnt, sizeof(u32), (int(*)(const void *, const void *)) bgp_compare_u32);
}

static void
bgp_rehash_buckets(struct bgp_proto *p)
{
  struct bgp_bucket **old = p->bucket_hash;
  struct bgp_bucket **new;
  unsigned oldn = p->hash_size;
  unsigned i, e, mask;
  struct bgp_bucket *b;

  p->hash_size = p->hash_limit;
  DBG("BGP: Rehashing bucket table from %d to %d\n", oldn, p->hash_size);
  p->hash_limit *= 4;
  if (p->hash_limit >= 65536)
    p->hash_limit = ~0;
  new = p->bucket_hash = mb_allocz(p->p.pool, p->hash_size * sizeof(struct bgp_bucket *));
  mask = p->hash_size - 1;
  for (i=0; i<oldn; i++)
    while (b = old[i])
      {
	old[i] = b->hash_next;
	e = b->hash & mask;
	b->hash_next = new[e];
	if (b->hash_next)
	  b->hash_next->hash_prev = b;
	b->hash_prev = NULL;
	new[e] = b;
      }
  mb_free(old);
}

static struct bgp_bucket *
bgp_new_bucket(struct bgp_proto *p, ea_list *new, unsigned hash)
{
  struct bgp_bucket *b;
  unsigned ea_size = sizeof(ea_list) + new->count * sizeof(eattr);
  unsigned ea_size_aligned = BIRD_ALIGN(ea_size, CPU_STRUCT_ALIGN);
  unsigned size = sizeof(struct bgp_bucket) + ea_size;
  unsigned i;
  byte *dest;
  unsigned index = hash & (p->hash_size - 1);

  /* Gather total size of non-inline attributes */
  for (i=0; i<new->count; i++)
    {
      eattr *a = &new->attrs[i];
      if (!(a->type & EAF_EMBEDDED))
	size += BIRD_ALIGN(sizeof(struct adata) + a->u.ptr->length, CPU_STRUCT_ALIGN);
    }

  /* Create the bucket and hash it */
  b = mb_alloc(p->p.pool, size);
  b->hash_next = p->bucket_hash[index];
  if (b->hash_next)
    b->hash_next->hash_prev = b;
  p->bucket_hash[index] = b;
  b->hash_prev = NULL;
  b->hash = hash;
  add_tail(&p->bucket_queue, &b->send_node);
  init_list(&b->prefixes);
  memcpy(b->eattrs, new, ea_size);
  dest = ((byte *)b->eattrs) + ea_size_aligned;

  /* Copy values of non-inline attributes */
  for (i=0; i<new->count; i++)
    {
      eattr *a = &b->eattrs->attrs[i];
      if (!(a->type & EAF_EMBEDDED))
	{
	  struct adata *oa = a->u.ptr;
	  struct adata *na = (struct adata *) dest;
	  memcpy(na, oa, sizeof(struct adata) + oa->length);
	  a->u.ptr = na;
	  dest += BIRD_ALIGN(sizeof(struct adata) + na->length, CPU_STRUCT_ALIGN);
	}
    }

  /* If needed, rehash */
  p->hash_count++;
  if (p->hash_count > p->hash_limit)
    bgp_rehash_buckets(p);

  return b;
}

static int
bgp_export_check(struct bgp_proto *p, ea_list *new)
{
  eattr *a;
  struct adata *d;

  /* Check if next hop is valid */
  a = ea_find(new, EA_CODE(EAP_BGP, BA_NEXT_HOP));
  if (!a || ipa_equal(p->next_hop, *(ip_addr *)a->u.ptr))
    {
      DBG("\tInvalid NEXT_HOP\n");
      return 0;
    }

  /* Check if we aren't forbidden to export the route by communities */
  a = ea_find(new, EA_CODE(EAP_BGP, BA_COMMUNITY));
  if (a)
    {
      d = a->u.ptr;
      if (int_set_contains(d, BGP_COMM_NO_ADVERTISE))
	{
	  DBG("\tNO_ADVERTISE\n");
	  return 0;
	}
      if (!p->is_internal &&
	  (int_set_contains(d, BGP_COMM_NO_EXPORT) ||
	   int_set_contains(d, BGP_COMM_NO_EXPORT_SUBCONFED)))
	{
	  DBG("\tNO_EXPORT\n");
	  return 0;
	}
    }

  return 1;
}

static struct bgp_bucket *
bgp_get_bucket(struct bgp_proto *p, ea_list *attrs, int originate)
{
  ea_list *new;
  unsigned i, cnt, hash, code;
  eattr *a, *d;
  u32 seen = 0;
  struct bgp_bucket *b;

  /* Merge the attribute list */
  new = alloca(ea_scan(attrs));
  ea_merge(attrs, new);
  ea_sort(new);

  /* Normalize attributes */
  d = new->attrs;
  cnt = new->count;
  new->count = 0;
  for(i=0; i<cnt; i++)
    {
      a = &new->attrs[i];
#ifdef LOCAL_DEBUG
      {
	byte buf[EA_FORMAT_BUF_SIZE];
	ea_format(a, buf);
	DBG("\t%s\n", buf);
      }
#endif
      if (EA_PROTO(a->id) != EAP_BGP)
	continue;
      code = EA_ID(a->id);
      if (ATTR_KNOWN(code))
	{
	  if (!bgp_attr_table[code].allow_in_ebgp && !p->is_internal)
	    continue;
	  /* The flags might have been zero if the attr was added by filters */
	  a->flags = (a->flags & BAF_PARTIAL) | bgp_attr_table[code].expected_flags;
	  if (code < 32)
	    seen |= 1 << code;
	}
      else
	{
	  /* Don't re-export unknown non-transitive attributes */
	  if (!(a->flags & BAF_TRANSITIVE))
	    continue;
	}
      *d = *a;
      if ((d->type & EAF_ORIGINATED) && !originate && (d->flags & BAF_TRANSITIVE) && (d->flags & BAF_OPTIONAL))
	d->flags |= BAF_PARTIAL;
      switch (d->type & EAF_TYPE_MASK)
	{
	case EAF_TYPE_INT_SET:
	  {
	    struct adata *z = alloca(sizeof(struct adata) + d->u.ptr->length);
	    z->length = d->u.ptr->length;
	    bgp_normalize_set((u32 *) z->data, (u32 *) d->u.ptr->data, z->length / 4);
	    d->u.ptr = z;
	    break;
	  }
	default: ;
	}
      d++;
      new->count++;
    }

  /* Hash */
  hash = ea_hash(new);
  for(b=p->bucket_hash[hash & (p->hash_size - 1)]; b; b=b->hash_next)
    if (b->hash == hash && ea_same(b->eattrs, new))
      {
	DBG("Found bucket.\n");
	return b;
      }

  /* Ensure that there are all mandatory attributes */
  for(i=0; i<ARRAY_SIZE(bgp_mandatory_attrs); i++)
    if (!(seen & (1 << bgp_mandatory_attrs[i])))
      {
	log(L_ERR "%s: Mandatory attribute %s missing", p->p.name, bgp_attr_table[bgp_mandatory_attrs[i]].name);
	return NULL;
      }

  if (!bgp_export_check(p, new))
    return NULL;

  /* Create new bucket */
  DBG("Creating bucket.\n");
  return bgp_new_bucket(p, new, hash);
}

void
bgp_free_bucket(struct bgp_proto *p, struct bgp_bucket *buck)
{
  if (buck->hash_next)
    buck->hash_next->hash_prev = buck->hash_prev;
  if (buck->hash_prev)
    buck->hash_prev->hash_next = buck->hash_next;
  else
    p->bucket_hash[buck->hash & (p->hash_size-1)] = buck->hash_next;
  mb_free(buck);
}

void
bgp_rt_notify(struct proto *P, net *n, rte *new, rte *old UNUSED, ea_list *attrs)
{
  struct bgp_proto *p = (struct bgp_proto *) P;
  struct bgp_bucket *buck;
  struct bgp_prefix *px;

  DBG("BGP: Got route %I/%d %s\n", n->n.prefix, n->n.pxlen, new ? "up" : "down");

  if (new)
    {
      buck = bgp_get_bucket(p, attrs, new->attrs->source != RTS_BGP);
      if (!buck)			/* Inconsistent attribute list */
	return;
    }
  else
    {
      if (!(buck = p->withdraw_bucket))
	{
	  buck = p->withdraw_bucket = mb_alloc(P->pool, sizeof(struct bgp_bucket));
	  init_list(&buck->prefixes);
	}
    }
  px = fib_get(&p->prefix_fib, &n->n.prefix, n->n.pxlen);
  if (px->bucket_node.next)
    {
      DBG("\tRemoving old entry.\n");
      rem_node(&px->bucket_node);
    }
  add_tail(&buck->prefixes, &px->bucket_node);
  bgp_schedule_packet(p->conn, PKT_UPDATE);
}

static int
bgp_create_attrs(struct bgp_proto *p, rte *e, ea_list **attrs, struct linpool *pool)
{
  ea_list *ea = lp_alloc(pool, sizeof(ea_list) + 4*sizeof(eattr));
  rta *rta = e->attrs;
  byte *z;

  ea->next = *attrs;
  *attrs = ea;
  ea->flags = EALF_SORTED;
  ea->count = 4;

  bgp_set_attr(ea->attrs, pool, BA_ORIGIN,
       ((rta->source == RTS_OSPF_EXT1) || (rta->source == RTS_OSPF_EXT2)) ? ORIGIN_INCOMPLETE : ORIGIN_IGP);

  if (p->is_internal)
    bgp_set_attr(ea->attrs+1, pool, BA_AS_PATH, 0);
  else
    {
      z = bgp_set_attr(ea->attrs+1, pool, BA_AS_PATH, 4);
      z[0] = AS_PATH_SEQUENCE;
      z[1] = 1;				/* 1 AS */
      put_u16(z+2, p->local_as);
    }

  z = bgp_set_attr(ea->attrs+2, pool, BA_NEXT_HOP, sizeof(ip_addr));
  if (p->cf->next_hop_self ||
      !p->is_internal ||
      rta->dest != RTD_ROUTER)
    *(ip_addr *)z = p->local_addr;
  else
    *(ip_addr *)z = e->attrs->gw;

  bgp_set_attr(ea->attrs+3, pool, BA_LOCAL_PREF, 0);

  return 0;				/* Leave decision to the filters */
}

static ea_list *
bgp_path_prepend(struct linpool *pool, eattr *a, ea_list *old, int as)
{
  struct ea_list *e = lp_alloc(pool, sizeof(ea_list) + sizeof(eattr));
  struct adata *olda = a->u.ptr;

  e->next = old;
  e->flags = EALF_SORTED;
  e->count = 1;
  e->attrs[0].id = EA_CODE(EAP_BGP, BA_AS_PATH);
  e->attrs[0].flags = BAF_TRANSITIVE;
  e->attrs[0].type = EAF_TYPE_AS_PATH;
  e->attrs[0].u.ptr = as_path_prepend(pool, olda, as);
  return e;
}

static int
bgp_update_attrs(struct bgp_proto *p, rte *e, ea_list **attrs, struct linpool *pool)
{
  eattr *a;

  if (!p->is_internal && (a = ea_find(e->attrs->eattrs, EA_CODE(EAP_BGP, BA_AS_PATH))))
    *attrs = bgp_path_prepend(pool, a, *attrs, p->local_as);

  a = ea_find(e->attrs->eattrs, EA_CODE(EAP_BGP, BA_NEXT_HOP));
  if (a && (p->is_internal || (!p->is_internal && e->attrs->iface == p->neigh->iface)))
    {
      /* Leave the original next hop attribute, will check later where does it point */
    }
  else
    {
      /* Need to create new one */
      *(ip_addr *) bgp_attach_attr(attrs, pool, BA_NEXT_HOP, sizeof(ip_addr)) = p->local_addr;
    }

  return 0;				/* Leave decision to the filters */
}

int
bgp_import_control(struct proto *P, rte **new, ea_list **attrs, struct linpool *pool)
{
  rte *e = *new;
  struct bgp_proto *p = (struct bgp_proto *) P;
  struct bgp_proto *new_bgp = (e->attrs->proto->proto == &proto_bgp) ? (struct bgp_proto *) e->attrs->proto : NULL;

  if (p == new_bgp)			/* Poison reverse updates */
    return -1;
  if (new_bgp)
    {
      if (p->local_as == new_bgp->local_as && p->is_internal && new_bgp->is_internal)
	return -1;			/* Don't redistribute internal routes with IBGP */
      return bgp_update_attrs(p, e, attrs, pool);
    }
  else
    return bgp_create_attrs(p, e, attrs, pool);
}

int
bgp_rte_better(rte *new, rte *old)
{
  struct bgp_proto *new_bgp = (struct bgp_proto *) new->attrs->proto;
  struct bgp_proto *old_bgp = (struct bgp_proto *) old->attrs->proto;
  eattr *x, *y;
  u32 n, o;

  /* Start with local preferences */
  x = ea_find(new->attrs->eattrs, EA_CODE(EAP_BGP, BA_LOCAL_PREF));
  y = ea_find(old->attrs->eattrs, EA_CODE(EAP_BGP, BA_LOCAL_PREF));
  n = x ? x->u.data : new_bgp->cf->default_local_pref;
  o = y ? y->u.data : old_bgp->cf->default_local_pref;
  if (n > o)
    return 1;
  if (n < o)
    return 0;

  /* Use AS path lengths */
  if (new_bgp->cf->compare_path_lengths || old_bgp->cf->compare_path_lengths)
    {
      x = ea_find(new->attrs->eattrs, EA_CODE(EAP_BGP, BA_AS_PATH));
      y = ea_find(old->attrs->eattrs, EA_CODE(EAP_BGP, BA_AS_PATH));
      n = x ? as_path_getlen(x->u.ptr) : 100000;
      o = y ? as_path_getlen(y->u.ptr) : 100000;
      if (n < o)
	return 1;
      if (n > o)
	return 0;
    }

  /* Use origins */
  x = ea_find(new->attrs->eattrs, EA_CODE(EAP_BGP, BA_ORIGIN));
  y = ea_find(old->attrs->eattrs, EA_CODE(EAP_BGP, BA_ORIGIN));
  n = x ? x->u.data : ORIGIN_INCOMPLETE;
  o = y ? y->u.data : ORIGIN_INCOMPLETE;
  if (n < o)
    return 1;
  if (n > o)
    return 0;

  /* Compare MED's */
  x = ea_find(new->attrs->eattrs, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC));
  y = ea_find(old->attrs->eattrs, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC));
  n = x ? x->u.data : new_bgp->cf->default_med;
  o = y ? y->u.data : old_bgp->cf->default_med;
  if (n < o)
    return 1;
  if (n > o)
    return 0;

  /* A tie breaking procedure according to RFC 1771, section 9.1.2.1 */
  /* We don't have interior distances */
  /* We prefer external peers */
  if (new_bgp->is_internal > old_bgp->is_internal)
    return 0;
  if (new_bgp->is_internal < old_bgp->is_internal)
    return 1;
  /* Finally we compare BGP identifiers */
  return (new_bgp->remote_id < old_bgp->remote_id);
}

static int
bgp_path_loopy(struct bgp_proto *p, eattr *a)
{
  byte *path = a->u.ptr->data;
  int len = a->u.ptr->length;
  int i, n;

  while (len > 0)
    {
      n = path[1];
      len -= 2 + 2*n;
      path += 2;
      for(i=0; i<n; i++)
	{
	  if (get_u16(path) == p->local_as)
	    return 1;
	  path += 2;
	}
    }
  return 0;
}

/**
 * bgp_decode_attrs - check and decode BGP attributes
 * @conn: connection
 * @attr: start of attribute block
 * @len: length of attribute block
 * @pool: linear pool to make all the allocations in
 * @mandatory: 1 iff presence of mandatory attributes has to be checked
 *
 * This function takes a BGP attribute block (a part of an Update message), checks
 * its consistency and converts it to a list of BIRD route attributes represented
 * by a &rta.
 */
struct rta *
bgp_decode_attrs(struct bgp_conn *conn, byte *attr, unsigned int len, struct linpool *pool, int mandatory)
{
  struct bgp_proto *bgp = conn->bgp;
  rta *a = lp_alloc(pool, sizeof(struct rta));
  unsigned int flags, code, l, i, type;
  int errcode;
  byte *z, *attr_start;
  byte seen[256/8];
  eattr *e;
  ea_list *ea;
  struct adata *ad;

  a->proto = &bgp->p;
  a->source = RTS_BGP;
  a->scope = SCOPE_UNIVERSE;
  a->cast = RTC_UNICAST;
  a->dest = RTD_ROUTER;
  a->flags = 0;
  a->aflags = 0;
  a->from = bgp->cf->remote_ip;
  a->eattrs = NULL;

  /* Parse the attributes */
  bzero(seen, sizeof(seen));
  DBG("BGP: Parsing attributes\n");
  while (len)
    {
      if (len < 2)
	goto malformed;
      attr_start = attr;
      flags = *attr++;
      code = *attr++;
      len -= 2;
      if (flags & BAF_EXT_LEN)
	{
	  if (len < 2)
	    goto malformed;
	  l = get_u16(attr);
	  attr += 2;
	  len -= 2;
	}
      else
	{
	  if (len < 1)
	    goto malformed;
	  l = *attr++;
	  len--;
	}
      if (l > len)
	goto malformed;
      len -= l;
      z = attr;
      attr += l;
      DBG("Attr %02x %02x %d\n", code, flags, l);
      if (seen[code/8] & (1 << (code%8)))
	goto malformed;
      if (ATTR_KNOWN(code))
	{
	  struct attr_desc *desc = &bgp_attr_table[code];
	  if (desc->expected_length >= 0 && desc->expected_length != (int) l)
	    { errcode = 5; goto err; }
	  if ((desc->expected_flags ^ flags) & (BAF_OPTIONAL | BAF_TRANSITIVE))
	    { errcode = 4; goto err; }
	  if (!desc->allow_in_ebgp && !bgp->is_internal)
	    continue;
	  if (desc->validate)
	    {
	      errcode = desc->validate(bgp, z, l);
	      if (errcode > 0)
		goto err;
	      if (errcode < 0)
		continue;
	    }
	  type = desc->type;
	}
      else				/* Unknown attribute */
	{
	  if (!(flags & BAF_OPTIONAL))
	    { errcode = 2; goto err; }
	  type = EAF_TYPE_OPAQUE;
	}
      seen[code/8] |= (1 << (code%8));
      ea = lp_alloc(pool, sizeof(ea_list) + sizeof(eattr));
      ea->next = a->eattrs;
      a->eattrs = ea;
      ea->flags = 0;
      ea->count = 1;
      ea->attrs[0].id = EA_CODE(EAP_BGP, code);
      ea->attrs[0].flags = flags;
      ea->attrs[0].type = type;
      if (type & EAF_EMBEDDED)
	ad = NULL;
      else
	{
	  ad = lp_alloc(pool, sizeof(struct adata) + l);
	  ea->attrs[0].u.ptr = ad;
	  ad->length = l;
	  memcpy(ad->data, z, l);
	}
      switch (type)
	{
	case EAF_TYPE_ROUTER_ID:
	case EAF_TYPE_INT:
	  if (l == 1)
	    ea->attrs[0].u.data = *z;
	  else
	    ea->attrs[0].u.data = get_u32(z);
	  break;
	case EAF_TYPE_IP_ADDRESS:
	  ipa_ntoh(*(ip_addr *)ad->data);
	  break;
	case EAF_TYPE_INT_SET:
	  {
	    u32 *z = (u32 *) ad->data;
	    for(i=0; i<ad->length/4; i++)
	      z[i] = ntohl(z[i]);
	    break;
	  }
	}
    }

#ifdef IPV6
  if (seen[BA_MP_REACH_NLRI / 8] & (1 << (BA_MP_REACH_NLRI % 8)))
    mandatory = 1;
#endif

  /* Check if all mandatory attributes are present */
  if (mandatory)
    {
      for(i=0; i < ARRAY_SIZE(bgp_mandatory_attrs); i++)
	{
	  code = bgp_mandatory_attrs[i];
	  if (!(seen[code/8] & (1 << (code%8))))
	    {
	      bgp_error(conn, 3, 3, &bgp_mandatory_attrs[i], 1);
	      return NULL;
	    }
	}
    }

  /* If the AS path attribute contains our AS, reject the routes */
  e = ea_find(a->eattrs, EA_CODE(EAP_BGP, BA_AS_PATH));
  if (e && bgp_path_loopy(bgp, e))
    {
      DBG("BGP: Path loop!\n");
      return NULL;
    }

  /* If there's no local preference, define one */
  if (!(seen[0] && (1 << BA_LOCAL_PREF)))
    bgp_attach_attr(&a->eattrs, pool, BA_LOCAL_PREF, 0);
  return a;

malformed:
  bgp_error(conn, 3, 1, NULL, 0);
  return NULL;

err:
  bgp_error(conn, 3, errcode, attr_start, z+l-attr_start);
  return NULL;
}

int
bgp_get_attr(eattr *a, byte *buf)
{
  unsigned int i = EA_ID(a->id);
  struct attr_desc *d;

  if (ATTR_KNOWN(i))
    {
      d = &bgp_attr_table[i];
      buf += bsprintf(buf, "%s", d->name);
      if (d->format)
	{
	  *buf++ = ':';
	  *buf++ = ' ';
	  d->format(a, buf);
	  return GA_FULL;
	}
      return GA_NAME;
    }
  bsprintf(buf, "%02x%s", i, (a->flags & BAF_TRANSITIVE) ? " [t]" : "");
  return GA_NAME;
}

void
bgp_attr_init(struct bgp_proto *p)
{
  p->hash_size = 256;
  p->hash_limit = p->hash_size * 4;
  p->bucket_hash = mb_allocz(p->p.pool, p->hash_size * sizeof(struct bgp_bucket *));
  init_list(&p->bucket_queue);
  p->withdraw_bucket = NULL;
  fib_init(&p->prefix_fib, p->p.pool, sizeof(struct bgp_prefix), 0, bgp_init_prefix);
}

void
bgp_get_route_info(rte *e, byte *buf, ea_list *attrs)
{
  eattr *p = ea_find(attrs, EA_CODE(EAP_BGP, BA_AS_PATH));
  eattr *o = ea_find(attrs, EA_CODE(EAP_BGP, BA_ORIGIN));
  int origas;

  buf += bsprintf(buf, " (%d) [", e->pref);
  if (p && (origas = as_path_get_first(p->u.ptr)) >= 0)
    buf += bsprintf(buf, "AS%d", origas);
  if (o)
    buf += bsprintf(buf, "%c", "ie?"[o->u.data]);
  strcpy(buf, "]");
}
