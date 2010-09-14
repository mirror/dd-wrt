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
  void (*format)(eattr *ea, byte *buf, int buflen);
};

static int
bgp_check_origin(struct bgp_proto *p UNUSED, byte *a, int len UNUSED)
{
  if (*a > 2)
    return 6;
  return 0;
}

static void
bgp_format_origin(eattr *a, byte *buf, int buflen UNUSED)
{
  static char *bgp_origin_names[] = { "IGP", "EGP", "Incomplete" };

  bsprintf(buf, bgp_origin_names[a->u.data]);
}

static int
path_segment_contains(byte *p, int bs, u32 asn)
{
  int i;
  int len = p[1];
  p += 2;

  for(i=0; i<len; i++)
    {
      u32 asn2 = (bs == 4) ? get_u32(p) : get_u16(p);
      if (asn2 == asn)
	return 1;
      p += bs;
    }

  return 0;
}

/* Validates path attribute, removes AS_CONFED_* segments, and also returns path length */
static int
validate_path(struct bgp_proto *p, int as_path, int bs, byte *idata, unsigned int *ilength)
{
  int res = 0;
  u8 *a, *dst;
  int len, plen, copy;

  dst = a = idata;
  len = *ilength;

  while (len)
    {
      if (len < 2)
	return -1;

      plen = 2 + bs * a[1];
      if (len < plen)
	return -1;

      switch (a[0])
	{
	case AS_PATH_SET:
	  copy = 1;
	  res++;
	  break;

	case AS_PATH_SEQUENCE:
	  copy = 1;
	  res += a[1];
	  break;

	case AS_PATH_CONFED_SEQUENCE:
	case AS_PATH_CONFED_SET:
	  if (as_path && path_segment_contains(a, bs, p->remote_as))
	    {
	      log(L_WARN "%s: AS_CONFED_* segment with peer ASN found, misconfigured confederation?", p->p.name);
	      return -1;
	    }

	  log(L_WARN "%s: %s_PATH attribute contains AS_CONFED_* segment, skipping segment",
	      p->p.name, as_path ? "AS" : "AS4");
	  copy = 0;
	  break;

	default:
	  return -1;
	}

      if (copy)
	{
	  if (dst != a)
	    memmove(dst, a, plen);
	  dst += plen;
	}

      len -= plen;
      a += plen;
    }

  *ilength = dst - idata;
  return res;
}

static inline int
validate_as_path(struct bgp_proto *p, byte *a, int *len)
{
  return validate_path(p, 1, p->as4_session ? 4 : 2, a, len);
}

static inline int
validate_as4_path(struct bgp_proto *p, struct adata *path)
{
  return validate_path(p, 0, 4, path->data, &path->length);
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

static void
bgp_format_next_hop(eattr *a, byte *buf, int buflen UNUSED)
{
  ip_addr *ipp = (ip_addr *) a->u.ptr->data;
#ifdef IPV6
  /* in IPv6, we might have two addresses in NEXT HOP */
  if ((a->u.ptr->length == NEXT_HOP_LENGTH) && ipa_nonzero(ipp[1]))
    {
      bsprintf(buf, "%I %I", ipp[0], ipp[1]);
      return;
    }
#endif

  bsprintf(buf, "%I", ipp[0]);
}

static int
bgp_check_aggregator(struct bgp_proto *p, byte *a UNUSED, int len)
{
  int exp_len = p->as4_session ? 8 : 6;
  
  return (len == exp_len) ? 0 : 5;
}

static void
bgp_format_aggregator(eattr *a, byte *buf, int buflen UNUSED)
{
  struct adata *ad =  a->u.ptr;
  byte *data = ad->data;
  u32 as;

  as = get_u32(data);
  data += 4;

  bsprintf(buf, "%d.%d.%d.%d AS%d", data[0], data[1], data[2], data[3], as);
}

static int
bgp_check_cluster_list(struct bgp_proto *p UNUSED, byte *a UNUSED, int len)
{
  return ((len % 4) == 0) ? 0 : 5;
}

static void
bgp_format_cluster_list(eattr *a, byte *buf, int buflen UNUSED)
{
  int_set_format(a->u.ptr, 0, buf, buflen);
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
    NULL, NULL }, /* is checked by validate_as_path() as a special case */
  { "next_hop", 4, BAF_TRANSITIVE, EAF_TYPE_IP_ADDRESS, 1,			/* BA_NEXT_HOP */
    bgp_check_next_hop, bgp_format_next_hop },
  { "med", 4, BAF_OPTIONAL, EAF_TYPE_INT, 1,					/* BA_MULTI_EXIT_DISC */
    NULL, NULL },
  { "local_pref", 4, BAF_TRANSITIVE, EAF_TYPE_INT, 0,				/* BA_LOCAL_PREF */
    NULL, NULL },
  { "atomic_aggr", 0, BAF_TRANSITIVE, EAF_TYPE_OPAQUE, 1,			/* BA_ATOMIC_AGGR */
    NULL, NULL },
  { "aggregator", -1, BAF_OPTIONAL | BAF_TRANSITIVE, EAF_TYPE_OPAQUE, 1,	/* BA_AGGREGATOR */
    bgp_check_aggregator, bgp_format_aggregator },
  { "community", -1, BAF_OPTIONAL | BAF_TRANSITIVE, EAF_TYPE_INT_SET, 1,	/* BA_COMMUNITY */
    NULL, NULL },
  { "originator_id", 4, BAF_OPTIONAL, EAF_TYPE_ROUTER_ID, 0,			/* BA_ORIGINATOR_ID */
    NULL, NULL },
  { "cluster_list", -1, BAF_OPTIONAL, EAF_TYPE_INT_SET, 0,			/* BA_CLUSTER_LIST */
    bgp_check_cluster_list, bgp_format_cluster_list }, 
  { .name = NULL },								/* BA_DPA */
  { .name = NULL },									/* BA_ADVERTISER */
  { .name = NULL },									/* BA_RCID_PATH */
  { "mp_reach_nlri", -1, BAF_OPTIONAL, EAF_TYPE_OPAQUE, 1,			/* BA_MP_REACH_NLRI */
    bgp_check_reach_nlri, NULL },
  { "mp_unreach_nlri", -1, BAF_OPTIONAL, EAF_TYPE_OPAQUE, 1,			/* BA_MP_UNREACH_NLRI */
    bgp_check_unreach_nlri, NULL },
  {  .name = NULL },									/* BA_EXTENDED_COMM */
  { "as4_path", -1, BAF_OPTIONAL | BAF_TRANSITIVE, EAF_TYPE_OPAQUE, 1,		/* BA_AS4_PATH */
    NULL, NULL },
  { "as4_aggregator", -1, BAF_OPTIONAL | BAF_TRANSITIVE, EAF_TYPE_OPAQUE, 1,	/* BA_AS4_PATH */
    NULL, NULL }
};

/* BA_AS4_PATH is type EAF_TYPE_OPAQUE and not type EAF_TYPE_AS_PATH.
 * It does not matter as this attribute does not appear on routes in the routing table.
 */

#define ATTR_KNOWN(code) ((code) < ARRAY_SIZE(bgp_attr_table) && bgp_attr_table[code].name)

static inline struct adata *
bgp_alloc_adata(struct linpool *pool, unsigned len)
{
  struct adata *ad = lp_alloc(pool, sizeof(struct adata) + len);
  ad->length = len;
  return ad;
}

static void
bgp_set_attr(eattr *e, unsigned attr, uintptr_t val)
{
  ASSERT(ATTR_KNOWN(attr));
  e->id = EA_CODE(EAP_BGP, attr);
  e->type = bgp_attr_table[attr].type;
  e->flags = bgp_attr_table[attr].expected_flags;
  if (e->type & EAF_EMBEDDED)
    e->u.data = val;
  else
    e->u.ptr = (struct adata *) val;
}

static byte *
bgp_set_attr_wa(eattr *e, struct linpool *pool, unsigned attr, unsigned len)
{
  struct adata *ad = bgp_alloc_adata(pool, len);
  bgp_set_attr(e, attr, (uintptr_t) ad);
  return ad->data;
}

void
bgp_attach_attr(ea_list **to, struct linpool *pool, unsigned attr, uintptr_t val)
{
  ea_list *a = lp_alloc(pool, sizeof(ea_list) + sizeof(eattr));
  a->next = *to;
  *to = a;
  a->flags = EALF_SORTED;
  a->count = 1;
  bgp_set_attr(a->attrs, attr, val);
}

byte *
bgp_attach_attr_wa(ea_list **to, struct linpool *pool, unsigned attr, unsigned len)
{
  struct adata *ad = bgp_alloc_adata(pool, len);
  bgp_attach_attr(to, pool, attr, (uintptr_t) ad);
  return ad->data;
}

static int
bgp_encode_attr_hdr(byte *dst, unsigned int flags, unsigned code, int len)
{
  int wlen;

  DBG("\tAttribute %02x (%d bytes, flags %02x)\n", code, len, flags);

  if (len < 256)
    {
      *dst++ = flags;
      *dst++ = code;
      *dst++ = len;
      wlen = 3;
    }
  else
    {
      *dst++ = flags | BAF_EXT_LEN;
      *dst++ = code;
      put_u16(dst, len);
      wlen = 4;
    }

  return wlen;
}

static void
aggregator_convert_to_old(struct adata *aggr, byte *dst, int *new_used)
{
  byte *src = aggr->data;
  *new_used = 0;

  u32 as = get_u32(src);
  if (as > 0xFFFF) 
    {
      as = AS_TRANS;
      *new_used = 1;
    }
  put_u16(dst, as);

  /* Copy IPv4 address */
  memcpy(dst + 2, src + 4, 4);
}

static void
aggregator_convert_to_new(struct adata *aggr, byte *dst)
{
  byte *src = aggr->data;

  u32 as   = get_u16(src);
  put_u32(dst, as);

  /* Copy IPv4 address */
  memcpy(dst + 4, src + 2, 4);
}

static int
bgp_get_attr_len(eattr *a)
{
  int len;
  if (ATTR_KNOWN(EA_ID(a->id)))
    {
      int code = EA_ID(a->id);
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
  
  return len;
}

#define ADVANCE(w, r, l) do { r -= l; w += l; } while (0)

/**
 * bgp_encode_attrs - encode BGP attributes
 * @p: BGP instance
 * @w: buffer
 * @attrs: a list of extended attributes
 * @remains: remaining space in the buffer
 *
 * The bgp_encode_attrs() function takes a list of extended attributes
 * and converts it to its BGP representation (a part of an Update message).
 *
 * Result: Length of the attribute block generated or -1 if not enough space.
 */
unsigned int
bgp_encode_attrs(struct bgp_proto *p, byte *w, ea_list *attrs, int remains)
{
  unsigned int i, code, flags;
  byte *start = w;
  int len, rv;

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

      /* When AS4-aware BGP speaker is talking to non-AS4-aware BGP speaker,
       * we have to convert our 4B AS_PATH to 2B AS_PATH and send our AS_PATH 
       * as optional AS4_PATH attribute.
       */
      if ((code == BA_AS_PATH) && (! p->as4_session))
	{
	  len = a->u.ptr->length;

	  if (remains < (len + 4))
	    goto err_no_buffer;

	  /* Using temporary buffer because don't know a length of created attr
	   * and therefore a length of a header. Perhaps i should better always
	   * use BAF_EXT_LEN. */
	  
	  byte buf[len];
	  int new_used;
	  int nl = as_path_convert_to_old(a->u.ptr, buf, &new_used);

	  DBG("BGP: Encoding old AS_PATH\n");
	  rv = bgp_encode_attr_hdr(w, BAF_TRANSITIVE, BA_AS_PATH, nl);
	  ADVANCE(w, remains, rv);
	  memcpy(w, buf, nl);
	  ADVANCE(w, remains, nl);

	  if (! new_used)
	    continue;

	  if (remains < (len + 4))
	    goto err_no_buffer;

	  /* We should discard AS_CONFED_SEQUENCE or AS_CONFED_SET path segments 
	   * here but we don't support confederations and such paths we already
	   * discarded in bgp_check_as_path().
	   */

	  DBG("BGP: Encoding AS4_PATH\n");
	  rv = bgp_encode_attr_hdr(w, BAF_OPTIONAL | BAF_TRANSITIVE, BA_AS4_PATH, len);
	  ADVANCE(w, remains, rv);
	  memcpy(w, a->u.ptr->data, len);
	  ADVANCE(w, remains, len);

	  continue;
	}

      /* The same issue with AGGREGATOR attribute */
      if ((code == BA_AGGREGATOR) && (! p->as4_session))
	{
	  int new_used;

	  len = 6;
	  if (remains < (len + 3))
	    goto err_no_buffer;

	  rv = bgp_encode_attr_hdr(w, BAF_OPTIONAL | BAF_TRANSITIVE, BA_AGGREGATOR, len);
	  ADVANCE(w, remains, rv);
	  aggregator_convert_to_old(a->u.ptr, w, &new_used);
	  ADVANCE(w, remains, len);

	  if (! new_used)
	    continue;

	  len = 8;
	  if (remains < (len + 3))
	    goto err_no_buffer;

	  rv = bgp_encode_attr_hdr(w, BAF_OPTIONAL | BAF_TRANSITIVE, BA_AS4_AGGREGATOR, len);
	  ADVANCE(w, remains, rv);
	  memcpy(w, a->u.ptr->data, len);
	  ADVANCE(w, remains, len);

	  continue;
	}

      /* Standard path continues here ... */

      flags = a->flags & (BAF_OPTIONAL | BAF_TRANSITIVE | BAF_PARTIAL);
      len = bgp_get_attr_len(a);

      /* Skip empty int sets */ 
      if (((a->type & EAF_TYPE_MASK) == EAF_TYPE_INT_SET) && (len == 0))
	continue; 

      if (remains < len + 4)
	goto err_no_buffer;

      rv = bgp_encode_attr_hdr(w, flags, code, len);
      ADVANCE(w, remains, rv);

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
      ADVANCE(w, remains, len);
    }
  return w - start;

 err_no_buffer:
  return -1;
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

static struct bgp_bucket *
bgp_get_bucket(struct bgp_proto *p, net *n, ea_list *attrs, int originate)
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
	log(L_ERR "%s: Mandatory attribute %s missing in route %I/%d", p->p.name, bgp_attr_table[bgp_mandatory_attrs[i]].name, n->n.prefix, n->n.pxlen);
	return NULL;
      }

  /* Check if next hop is valid */
  a = ea_find(new, EA_CODE(EAP_BGP, BA_NEXT_HOP));
  if (!a || ipa_equal(p->next_hop, *(ip_addr *)a->u.ptr->data))
    {
      log(L_ERR "%s: Invalid NEXT_HOP attribute in route %I/%d", p->p.name, n->n.prefix, n->n.pxlen);
      return NULL;
    }

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
bgp_rt_notify(struct proto *P, rtable *tbl UNUSED, net *n, rte *new, rte *old UNUSED, ea_list *attrs)
{
  struct bgp_proto *p = (struct bgp_proto *) P;
  struct bgp_bucket *buck;
  struct bgp_prefix *px;

  DBG("BGP: Got route %I/%d %s\n", n->n.prefix, n->n.pxlen, new ? "up" : "down");

  if (new)
    {
      buck = bgp_get_bucket(p, n, attrs, new->attrs->source != RTS_BGP);
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

  bgp_set_attr(ea->attrs, BA_ORIGIN,
       ((rta->source == RTS_OSPF_EXT1) || (rta->source == RTS_OSPF_EXT2)) ? ORIGIN_INCOMPLETE : ORIGIN_IGP);

  if (p->is_internal)
    bgp_set_attr_wa(ea->attrs+1, pool, BA_AS_PATH, 0);
  else
    {
      z = bgp_set_attr_wa(ea->attrs+1, pool, BA_AS_PATH, 6);
      z[0] = AS_PATH_SEQUENCE;
      z[1] = 1;				/* 1 AS */
      put_u32(z+2, p->local_as);
    }

  z = bgp_set_attr_wa(ea->attrs+2, pool, BA_NEXT_HOP, NEXT_HOP_LENGTH);
  if (p->cf->next_hop_self ||
      rta->dest != RTD_ROUTER ||
      ipa_equal(e->attrs->gw, IPA_NONE) ||
      ipa_has_link_scope(rta->gw) ||
      (!p->is_internal && (rta->iface != p->neigh->iface)))
    set_next_hop(z, p->source_addr);
  else
    set_next_hop(z, rta->gw);

  bgp_set_attr(ea->attrs+3, BA_LOCAL_PREF, p->cf->default_local_pref);

  return 0;				/* Leave decision to the filters */
}


static inline int
bgp_as_path_loopy(struct bgp_proto *p, rta *a)
{
  eattr *e = ea_find(a->eattrs, EA_CODE(EAP_BGP, BA_AS_PATH));
  return (e && as_path_is_member(e->u.ptr, p->local_as));
}

static inline int
bgp_originator_id_loopy(struct bgp_proto *p, rta *a)
{
  eattr *e = ea_find(a->eattrs, EA_CODE(EAP_BGP, BA_ORIGINATOR_ID));
  return (e && (e->u.data == p->local_id));
}

static inline int
bgp_cluster_list_loopy(struct bgp_proto *p, rta *a)
{
  eattr *e = ea_find(a->eattrs, EA_CODE(EAP_BGP, BA_CLUSTER_LIST));
  return (e && p->rr_client && int_set_contains(e->u.ptr, p->rr_cluster_id));
}


static inline void
bgp_path_prepend(rte *e, ea_list **attrs, struct linpool *pool, u32 as)
{
  eattr *a = ea_find(e->attrs->eattrs, EA_CODE(EAP_BGP, BA_AS_PATH));
  bgp_attach_attr(attrs, pool, BA_AS_PATH, (uintptr_t) as_path_prepend(pool, a->u.ptr, as));
}

static inline void
bgp_cluster_list_prepend(rte *e, ea_list **attrs, struct linpool *pool, u32 cid)
{
  eattr *a = ea_find(e->attrs->eattrs, EA_CODE(EAP_BGP, BA_CLUSTER_LIST));
  bgp_attach_attr(attrs, pool, BA_CLUSTER_LIST, (uintptr_t) int_set_add(pool, a ? a->u.ptr : NULL, cid));
}

static int
bgp_update_attrs(struct bgp_proto *p, rte *e, ea_list **attrs, struct linpool *pool, int rr)
{
  eattr *a;

  if (!p->is_internal && !p->rs_client)
    {
      bgp_path_prepend(e, attrs, pool, p->local_as);

      /* The MULTI_EXIT_DISC attribute received from a neighboring AS MUST NOT be
       * propagated to other neighboring ASes.
       * Perhaps it would be better to undefine it.
       */
      a = ea_find(e->attrs->eattrs, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC));
      if (a)
	bgp_attach_attr(attrs, pool, BA_MULTI_EXIT_DISC, 0);
    }

  a = ea_find(e->attrs->eattrs, EA_CODE(EAP_BGP, BA_NEXT_HOP));
  if (a && !p->cf->next_hop_self && (p->is_internal || (!p->is_internal && e->attrs->iface == p->neigh->iface)))
    {
      /* Leave the original next hop attribute, will check later where does it point */
    }
  else
    {
      /* Need to create new one */
      byte *b = bgp_attach_attr_wa(attrs, pool, BA_NEXT_HOP, NEXT_HOP_LENGTH);
      set_next_hop(b, p->source_addr);
    }

  if (rr)
    {
      /* Handling route reflection, RFC 4456 */
      struct bgp_proto *src = (struct bgp_proto *) e->attrs->proto;

      a = ea_find(e->attrs->eattrs, EA_CODE(EAP_BGP, BA_ORIGINATOR_ID));
      if (!a)
	bgp_attach_attr(attrs, pool, BA_ORIGINATOR_ID, src->remote_id);

      /* We attach proper cluster ID according to whether the route is entering or leaving the cluster */
      bgp_cluster_list_prepend(e, attrs, pool, src->rr_client ? src->rr_cluster_id : p->rr_cluster_id);

      /* Two RR clients with different cluster ID, hmmm */
      if (src->rr_client && p->rr_client && (src->rr_cluster_id != p->rr_cluster_id))
	bgp_cluster_list_prepend(e, attrs, pool, p->rr_cluster_id);
    }

  return 0;				/* Leave decision to the filters */
}

static int
bgp_community_filter(struct bgp_proto *p, rte *e)
{
  eattr *a;
  struct adata *d;

  /* Check if we aren't forbidden to export the route by communities */
  a = ea_find(e->attrs->eattrs, EA_CODE(EAP_BGP, BA_COMMUNITY));
  if (a)
    {
      d = a->u.ptr;
      if (int_set_contains(d, BGP_COMM_NO_ADVERTISE))
	{
	  DBG("\tNO_ADVERTISE\n");
	  return 1;
	}
      if (!p->is_internal &&
	  (int_set_contains(d, BGP_COMM_NO_EXPORT) ||
	   int_set_contains(d, BGP_COMM_NO_EXPORT_SUBCONFED)))
	{
	  DBG("\tNO_EXPORT\n");
	  return 1;
	}
    }

  return 0;
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
      /* We should check here for cluster list loop, because the receiving BGP instance
	 might have different cluster ID  */
      if (bgp_cluster_list_loopy(p, e->attrs))
	return -1;

      if (p->cf->interpret_communities && bgp_community_filter(p, e))
	return -1;

      if (p->local_as == new_bgp->local_as && p->is_internal && new_bgp->is_internal)
	{
	  /* Redistribution of internal routes with IBGP */
	  if (p->rr_client || new_bgp->rr_client)
	    /* Route reflection, RFC 4456 */
	    return bgp_update_attrs(p, e, attrs, pool, 1);
	  else
	    return -1;
	}
      else
	return bgp_update_attrs(p, e, attrs, pool, 0);
    }
  else
    return bgp_create_attrs(p, e, attrs, pool);
}

static inline u32
bgp_get_neighbor(rte *r)
{
  eattr *e = ea_find(r->attrs->eattrs, EA_CODE(EAP_BGP, BA_AS_PATH));
  u32 as;

  if (e && as_path_get_first(e->u.ptr, &as))
    return as;
  else
    return ((struct bgp_proto *) r->attrs->proto)->remote_as;
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

  /* RFC 4271 9.1.2.2. a)  Use AS path lengths */
  if (new_bgp->cf->compare_path_lengths || old_bgp->cf->compare_path_lengths)
    {
      x = ea_find(new->attrs->eattrs, EA_CODE(EAP_BGP, BA_AS_PATH));
      y = ea_find(old->attrs->eattrs, EA_CODE(EAP_BGP, BA_AS_PATH));
      n = x ? as_path_getlen(x->u.ptr) : AS_PATH_MAXLEN;
      o = y ? as_path_getlen(y->u.ptr) : AS_PATH_MAXLEN;
      if (n < o)
	return 1;
      if (n > o)
	return 0;
    }

  /* RFC 4271 9.1.2.2. b) Use origins */
  x = ea_find(new->attrs->eattrs, EA_CODE(EAP_BGP, BA_ORIGIN));
  y = ea_find(old->attrs->eattrs, EA_CODE(EAP_BGP, BA_ORIGIN));
  n = x ? x->u.data : ORIGIN_INCOMPLETE;
  o = y ? y->u.data : ORIGIN_INCOMPLETE;
  if (n < o)
    return 1;
  if (n > o)
    return 0;

  /* RFC 4271 9.1.2.2. c) Compare MED's */
  if (bgp_get_neighbor(new) == bgp_get_neighbor(old))
    {
      x = ea_find(new->attrs->eattrs, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC));
      y = ea_find(old->attrs->eattrs, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC));
      n = x ? x->u.data : new_bgp->cf->default_med;
      o = y ? y->u.data : old_bgp->cf->default_med;
      if (n < o)
	return 1;
      if (n > o)
	return 0;
    }

  /* RFC 4271 9.1.2.2. d) Prefer external peers */
  if (new_bgp->is_internal > old_bgp->is_internal)
    return 0;
  if (new_bgp->is_internal < old_bgp->is_internal)
    return 1;

  /* Skipping RFC 4271 9.1.2.2. e) */
  /* We don't have interior distances */

  /* RFC 4271 9.1.2.2. f) Compare BGP identifiers */
  /* RFC 4456 9. a) Use ORIGINATOR_ID instead of local neighor ID */
  x = ea_find(new->attrs->eattrs, EA_CODE(EAP_BGP, BA_ORIGINATOR_ID));
  y = ea_find(old->attrs->eattrs, EA_CODE(EAP_BGP, BA_ORIGINATOR_ID));
  n = x ? x->u.data : new_bgp->remote_id;
  o = y ? y->u.data : old_bgp->remote_id;

  /* RFC 5004 - prefer older routes */
  /* (if both are external and from different peer) */
  if ((new_bgp->cf->prefer_older || old_bgp->cf->prefer_older) &&
      !new_bgp->is_internal && n != o)
    return 0;

  /* rest of RFC 4271 9.1.2.2. f) */
  if (n < o)
    return 1;
  if (n > o)
    return 0;

  /* RFC 4456 9. b) Compare cluster list lengths */
  x = ea_find(new->attrs->eattrs, EA_CODE(EAP_BGP, BA_CLUSTER_LIST));
  y = ea_find(old->attrs->eattrs, EA_CODE(EAP_BGP, BA_CLUSTER_LIST));
  n = x ? int_set_get_size(x->u.ptr) : 0;
  o = y ? int_set_get_size(y->u.ptr) : 0;
  if (n < o)
    return 1;
  if (n > o)
    return 0;

  /* RFC 4271 9.1.2.2. g) Compare peer IP adresses */
  return (ipa_compare(new_bgp->cf->remote_ip, old_bgp->cf->remote_ip) < 0);
}

static struct adata *
bgp_aggregator_convert_to_new(struct adata *old, struct linpool *pool)
{
  struct adata *newa = lp_alloc(pool, sizeof(struct adata) + 8);
  newa->length = 8;
  aggregator_convert_to_new(old, newa->data);
  return newa;
}


/* Take last req_as ASNs from path old2 (in 2B format), convert to 4B format
 * and append path old4 (in 4B format).
 */
static struct adata *
bgp_merge_as_paths(struct adata *old2, struct adata *old4, int req_as, struct linpool *pool)
{
  byte buf[old2->length * 2];

  int ol = as_path_convert_to_new(old2, buf, req_as);
  int nl = ol + (old4 ? old4->length : 0);

  struct adata *newa = lp_alloc(pool, sizeof(struct adata) + nl);
  newa->length = nl;
  memcpy(newa->data, buf, ol);
  if (old4) memcpy(newa->data + ol, old4->data, old4->length);

  return newa;
}

static int
as4_aggregator_valid(struct adata *aggr)
{
  if (aggr->length != 8)
    return 0;

  u32 *a = (u32 *) aggr->data;

  if ((a[0] == 0) || (a[1] == 0))
    return 0;

  return 1;
}


/* Reconstruct 4B AS_PATH and AGGREGATOR according to RFC 4893 4.2.3 */
static void
bgp_reconstruct_4b_atts(struct bgp_proto *p, rta *a, struct linpool *pool)
{
  eattr *p2 =ea_find(a->eattrs, EA_CODE(EAP_BGP, BA_AS_PATH));
  eattr *p4 =ea_find(a->eattrs, EA_CODE(EAP_BGP, BA_AS4_PATH));
  eattr *a2 =ea_find(a->eattrs, EA_CODE(EAP_BGP, BA_AGGREGATOR));
  eattr *a4 =ea_find(a->eattrs, EA_CODE(EAP_BGP, BA_AS4_AGGREGATOR));
  int a4_removed = 0;

  if (a4 && !as4_aggregator_valid(a4->u.ptr))
    {
      log(L_WARN "%s: AS4_AGGREGATOR attribute is invalid, skipping attribute", p->p.name);
      a4 = NULL;
      a4_removed = 1;
    }

  if (a2)
    {
      u32 a2_as = get_u16(a2->u.ptr->data);

      if (a4)
	{
	  if (a2_as != AS_TRANS)
	    {
	      /* Routes were aggregated by old router and therefore AS4_PATH
	       * and AS4_AGGREGATOR is invalid
	       *
	       * Convert AS_PATH and AGGREGATOR to 4B format and finish.
	       */

	      a2->u.ptr = bgp_aggregator_convert_to_new(a2->u.ptr, pool);
	      p2->u.ptr = bgp_merge_as_paths(p2->u.ptr, NULL, AS_PATH_MAXLEN, pool);

	      return;
	    }
	  else
	    {
	      /* Common case, use AS4_AGGREGATOR attribute */
	      a2->u.ptr = a4->u.ptr;
	    }
	}
      else
	{
	  /* Common case, use old AGGREGATOR attribute */
	  a2->u.ptr = bgp_aggregator_convert_to_new(a2->u.ptr, pool);

	  if ((a2_as == AS_TRANS) && !a4_removed)
	    log(L_WARN "%s: AGGREGATOR attribute contain AS_TRANS, but AS4_AGGREGATOR is missing", p->p.name);
	}
    }
  else
    if (a4)
      log(L_WARN "%s: AS4_AGGREGATOR attribute received, but AGGREGATOR attribute is missing", p->p.name);

  int p2_len = as_path_getlen_int(p2->u.ptr, 2);
  int p4_len = p4 ? validate_as4_path(p, p4->u.ptr) : -1;

  if (p4 && (p4_len < 0))
    log(L_WARN "%s: AS4_PATH attribute is malformed, skipping attribute", p->p.name);

  if ((p4_len <= 0) || (p2_len < p4_len))
    p2->u.ptr = bgp_merge_as_paths(p2->u.ptr, NULL, AS_PATH_MAXLEN, pool);
  else
    p2->u.ptr = bgp_merge_as_paths(p2->u.ptr, p4->u.ptr, p2_len - p4_len, pool);
}

static void
bgp_remove_as4_attrs(struct bgp_proto *p, rta *a)
{
  unsigned id1 = EA_CODE(EAP_BGP, BA_AS4_PATH);
  unsigned id2 = EA_CODE(EAP_BGP, BA_AS4_AGGREGATOR);
  ea_list **el = &(a->eattrs);

  /* We know that ea_lists constructed in bgp_decode attrs have one attribute per ea_list struct */
  while (*el != NULL)
    {
      unsigned fid = (*el)->attrs[0].id;

      if ((fid == id1) || (fid == id2))
	{
	  *el = (*el)->next;
	  if (p->as4_session)
	    log(L_WARN "BGP: Unexpected AS4_* attributes received");
	}
      else
	el = &((*el)->next);
    }
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
	  else if (code == BA_AS_PATH)
	    {
	      /* Special case as it might also trim the attribute */
	      if (validate_as_path(bgp, z, &l) < 0)
		{ errcode = 11; goto err; }
	    }
	  type = desc->type;
	}
      else				/* Unknown attribute */
	{
	  if (!(flags & BAF_OPTIONAL))
	    { errcode = 2; goto err; }
	  type = EAF_TYPE_OPAQUE;
	}
      
      // Only OPTIONAL and TRANSITIVE attributes may have non-zero PARTIAL flag
      // if (!((flags & BAF_OPTIONAL) && (flags & BAF_TRANSITIVE)) && (flags & BAF_PARTIAL))
      //   { errcode = 4; goto err; }

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
  /* If we received MP_REACH_NLRI we should check mandatory attributes */
  if (bgp->mp_reach_len != 0)
    mandatory = 1;
#endif

  /* If there is no (reachability) NLRI, we should exit now */
  if (! mandatory)
    return a;

  /* Check if all mandatory attributes are present */
  for(i=0; i < ARRAY_SIZE(bgp_mandatory_attrs); i++)
    {
      code = bgp_mandatory_attrs[i];
      if (!(seen[code/8] & (1 << (code%8))))
	{
	  bgp_error(conn, 3, 3, &bgp_mandatory_attrs[i], 1);
	  return NULL;
	}
    }

  /* When receiving attributes from non-AS4-aware BGP speaker,
   * we have to reconstruct 4B AS_PATH and AGGREGATOR attributes
   */
  if (! bgp->as4_session)
    bgp_reconstruct_4b_atts(bgp, a, pool);

  bgp_remove_as4_attrs(bgp, a);

  /* If the AS path attribute contains our AS, reject the routes */
  if (bgp_as_path_loopy(bgp, a))
    goto loop;

  /* Two checks for IBGP loops caused by route reflection, RFC 4456 */ 
  if (bgp_originator_id_loopy(bgp, a) ||
      bgp_cluster_list_loopy(bgp, a))
    goto loop;

  /* If there's no local preference, define one */
  if (!(seen[0] & (1 << BA_LOCAL_PREF)))
    bgp_attach_attr(&a->eattrs, pool, BA_LOCAL_PREF, bgp->cf->default_local_pref);

  return a;

loop:
  DBG("BGP: Path loop!\n");
  return NULL;

malformed:
  bgp_error(conn, 3, 1, NULL, 0);
  return NULL;

err:
  bgp_error(conn, 3, errcode, attr_start, z+l-attr_start);
  return NULL;
}

int
bgp_get_attr(eattr *a, byte *buf, int buflen)
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
	  d->format(a, buf, buflen);
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
  u32 origas;

  buf += bsprintf(buf, " (%d) [", e->pref);
  if (p && as_path_get_last(p->u.ptr, &origas))
    buf += bsprintf(buf, "AS%u", origas);
  if (o)
    buf += bsprintf(buf, "%c", "ie?"[o->u.data]);
  strcpy(buf, "]");
}
