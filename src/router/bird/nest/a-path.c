/*
 *	BIRD -- Path Operations
 *
 *	(c) 2000 Martin Mares <mj@ucw.cz>
 *	(c) 2000 Pavel Machek <pavel@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "nest/bird.h"
#include "nest/route.h"
#include "nest/attrs.h"
#include "lib/resource.h"
#include "lib/unaligned.h"
#include "lib/string.h"
#include "filter/filter.h"

// static inline void put_as(byte *data, u32 as) { put_u32(data, as); }
// static inline u32 get_as(byte *data) { return get_u32(data); }

#define put_as put_u32
#define get_as get_u32
#define BS  4

struct adata *
as_path_prepend(struct linpool *pool, struct adata *olda, u32 as)
{
  struct adata *newa;

  if (olda->length && olda->data[0] == AS_PATH_SEQUENCE && olda->data[1] < 255)
    /* Starting with sequence => just prepend the AS number */
    {
      int nl = olda->length + BS;
      newa = lp_alloc(pool, sizeof(struct adata) + nl);
      newa->length = nl;
      newa->data[0] = AS_PATH_SEQUENCE;
      newa->data[1] = olda->data[1] + 1;
      memcpy(newa->data + BS + 2, olda->data + 2, olda->length - 2);
    }
  else /* Create new path segment */
    {
      int nl = olda->length + BS + 2;
      newa = lp_alloc(pool, sizeof(struct adata) + nl);
      newa->length = nl;
      newa->data[0] = AS_PATH_SEQUENCE;
      newa->data[1] = 1;
      memcpy(newa->data + BS + 2, olda->data, olda->length);
    }
  put_as(newa->data + 2, as);
  return newa;
}

int
as_path_convert_to_old(struct adata *path, byte *dst, int *new_used)
{
  byte *src = path->data;
  byte *src_end = src + path->length;
  byte *dst_start = dst;
  u32 as;
  int i, n;
  *new_used = 0;

  while (src < src_end)
    {
      n = src[1];
      *dst++ = *src++;
      *dst++ = *src++;

      for(i=0; i<n; i++)
	{
	  as = get_u32(src);
	  if (as > 0xFFFF) 
	    {
	      as = AS_TRANS;
	      *new_used = 1;
	    }
	  put_u16(dst, as);
	  src += 4;
	  dst += 2;
	}
    }

  return dst - dst_start;
}

int
as_path_convert_to_new(struct adata *path, byte *dst, int req_as)
{
  byte *src = path->data;
  byte *src_end = src + path->length;
  byte *dst_start = dst;
  u32 as;
  int i, t, n;


  while ((src < src_end) && (req_as > 0))
    {
      t = *src++;
      n = *src++;

      if (t == AS_PATH_SEQUENCE)
	{
	  if (n > req_as)
	    n = req_as;

	  req_as -= n;
	}
      else // t == AS_PATH_SET
	req_as--;

      *dst++ = t;
      *dst++ = n;

      for(i=0; i<n; i++)
	{
	  as = get_u16(src);
	  put_u32(dst, as);
	  src += 2;
	  dst += 4;
	}
    }

  return dst - dst_start;
}

void
as_path_format(struct adata *path, byte *buf, unsigned int size)
{
  byte *p = path->data;
  byte *e = p + path->length;
  byte *end = buf + size - 16;
  int sp = 1;
  int l, isset;

  while (p < e)
    {
      if (buf > end)
	{
	  strcpy(buf, " ...");
	  return;
	}
      isset = (*p++ == AS_PATH_SET);
      l = *p++;
      if (isset)
	{
	  if (!sp)
	    *buf++ = ' ';
	  *buf++ = '{';
	  sp = 0;
	}
      while (l-- && buf <= end)
	{
	  if (!sp)
	    *buf++ = ' ';
	  buf += bsprintf(buf, "%u", get_as(p));
	  p += BS;
	  sp = 0;
	}
      if (isset)
	{
	  *buf++ = ' ';
	  *buf++ = '}';
	  sp = 0;
	}
    }
  *buf = 0;
}

int
as_path_getlen(struct adata *path)
{
  return as_path_getlen_int(path, BS);
}

int
as_path_getlen_int(struct adata *path, int bs)
{
  int res = 0;
  u8 *p = path->data;
  u8 *q = p+path->length;
  int len;

  while (p<q)
    {
      switch (*p++)
	{
	case AS_PATH_SET:      len = *p++; res++;      p += bs * len; break;
	case AS_PATH_SEQUENCE: len = *p++; res += len; p += bs * len; break;
	default: bug("as_path_getlen: Invalid path segment");
	}
    }
  return res;
}

int
as_path_get_last(struct adata *path, u32 *orig_as)
{
  int found = 0;
  u32 res = 0;
  u8 *p = path->data;
  u8 *q = p+path->length;
  int len;

  while (p<q)
    {
      switch (*p++)
	{
	case AS_PATH_SET:
	  if (len = *p++)
	    {
	      found = 0;
	      p += BS * len;
	    }
	  break;
	case AS_PATH_SEQUENCE:
	  if (len = *p++)
	    {
	      found = 1;
	      res = get_as(p + BS * (len - 1));
	      p += BS * len;
	    }
	  break;
	default: bug("as_path_get_first: Invalid path segment");
	}
    }

  if (found)
    *orig_as = res;
  return found;
}

int
as_path_get_first(struct adata *path, u32 *last_as)
{
  u8 *p = path->data;

  if ((path->length == 0) || (p[0] != AS_PATH_SEQUENCE) || (p[1] == 0))
    return 0;
  else
    {
      *last_as = get_as(p+2);
      return 1;
    }
}

int
as_path_contains(struct adata *path, u32 as, int min)
{
  u8 *p = path->data;
  u8 *q = p+path->length;
  int num = 0;
  int i, n;

  while (p<q)
    {
      n = p[1];
      p += 2;
      for(i=0; i<n; i++)
	{
	  if (get_as(p) == as)
	    if (++num == min)
	      return 1;
	  p += BS;
	}
    }
  return 0;
}

int
as_path_match_set(struct adata *path, struct f_tree *set)
{
  u8 *p = path->data;
  u8 *q = p+path->length;
  int i, n;

  while (p<q)
    {
      n = p[1];
      p += 2;
      for (i=0; i<n; i++)
	{
	  struct f_val v = {T_INT, .val.i = get_as(p)};
	  if (find_tree(set, v))
	    return 1;
	  p += BS;
	}
    }

  return 0;
}

struct adata *
as_path_filter(struct linpool *pool, struct adata *path, struct f_tree *set, u32 key, int pos)
{
  if (!path)
    return NULL;

  int len = path->length;
  u8 *p = path->data;
  u8 *q = path->data + len;
  u8 *d, *d2;
  int i, bt, sn, dn;
  u8 buf[len];

  d = buf;
  while (p<q)
    {
      /* Read block header (type and length) */
      bt = p[0];
      sn = p[1];
      dn = 0;
      p += 2;
      d2 = d + 2;

      for (i = 0; i < sn; i++)
	{
	  u32 as = get_as(p);
	  int match;

	  if (set)
	    match = !!find_tree(set, (struct f_val){T_INT, .val.i = as});
	  else
	    match = (as == key);

	  if (match == pos)
	    {
	      put_as(d2, as);
	      d2 += BS;
	      dn++;
	    }

	  p += BS;
	}

      if (dn > 0)
	{
	  /* Nonempty block, set block header and advance */
	  d[0] = bt;
	  d[1] = dn;
	  d = d2;
	}
  }

  int nl = d - buf;
  if (nl == path->length)
    return path;

  struct adata *res = lp_alloc(pool, sizeof(struct adata) + nl);
  res->length = nl;
  memcpy(res->data, buf, nl);

  return res;
}


struct pm_pos
{
  u8 set;
  u8 mark;
  union
  {
    char *sp;
    u32 asn;
  } val;
};

static int
parse_path(struct adata *path, struct pm_pos *pos)
{
  u8 *p = path->data;
  u8 *q = p + path->length;
  struct pm_pos *opos = pos;
  int i, len;


  while (p < q)
    switch (*p++)
      {
      case AS_PATH_SET:
	pos->set = 1;
	pos->mark = 0;
	pos->val.sp = p;
	len = *p;
	p += 1 + BS * len;
	pos++;
	break;
      
      case AS_PATH_SEQUENCE:
	len = *p++;
	for (i = 0; i < len; i++)
	  {
	    pos->set = 0;
	    pos->mark = 0;
	    pos->val.asn = get_as(p);
	    p += BS;
	    pos++;
	  }
	break;

      default:
	bug("as_path_match: Invalid path component");
      }
  
  return pos - opos;
}


static int
pm_match(struct pm_pos *pos, u32 asn)
{
  if (! pos->set)
    return pos->val.asn == asn;

  u8 *p = pos->val.sp;
  int len = *p++;
  int i;

  for (i = 0; i < len; i++)
    if (get_as(p + i * BS) == asn)
      return 1;

  return 0;
}

static void
pm_mark(struct pm_pos *pos, int i, int plen, int *nl, int *nh)
{
  int j;

  if (pos[i].set)
    pos[i].mark = 1;
  
  for (j = i + 1; (j < plen) && pos[j].set && (! pos[j].mark); j++)
    pos[j].mark = 1;
  pos[j].mark = 1;

  /* We are going downwards, therefore every mark is
     new low and just the first mark is new high */

  *nl = i + (pos[i].set ? 0 : 1);

  if (*nh < 0)
    *nh = j;
}

/* AS path matching is nontrivial. Because AS path can
 * contain sets, it is not a plain wildcard matching. A set 
 * in an AS path is interpreted as it might represent any
 * sequence of AS numbers from that set (possibly with
 * repetitions). So it is also a kind of a pattern,
 * more complicated than a path mask.
 *
 * The algorithm for AS path matching is a variant
 * of nondeterministic finite state machine, where
 * positions in AS path are states, and items in
 * path mask are input for that finite state machine.
 * During execution of the algorithm we maintain a set
 * of marked states - a state is marked if it can be
 * reached by any walk through NFSM with regard to
 * currently processed part of input. When we process
 * next part of mask, we advance each marked state.
 * We start with marked first position, when we
 * run out of marked positions, we reject. When
 * we process the whole mask, we accept iff final position
 * (auxiliary position after last real position in AS path)
 * is marked.
 */

int
as_path_match(struct adata *path, struct f_path_mask *mask)
{
  struct pm_pos pos[2048 + 1];
  int plen = parse_path(path, pos);
  int l, h, i, nh, nl;
  u32 val = 0;

  /* l and h are bound of interval of positions where
     are marked states */

  pos[plen].set = 0;
  pos[plen].mark = 0;

  l = h = 0;
  pos[0].mark = 1;
  
  while (mask)
    {
      /* We remove this mark to not step after pos[plen] */
      pos[plen].mark = 0;

      switch (mask->kind)
	{
	case PM_ASTERISK:
	  for (i = l; i <= plen; i++)
	    pos[i].mark = 1;
	  h = plen;
	  break;

	case PM_ASN:
	  val = mask->val;
	  goto step;
	case PM_ASN_EXPR:
	  val = f_eval_asn((struct f_inst *) mask->val);
	  goto step;
	case PM_QUESTION:
	step:
	  nh = nl = -1;
	  for (i = h; i >= l; i--)
	    if (pos[i].mark)
	      {
		pos[i].mark = 0;
		if ((mask->kind == PM_QUESTION) || pm_match(pos + i, val))
		  pm_mark(pos, i, plen, &nl, &nh);
	      }

	  if (nh < 0)
	    return 0;

	  h = nh;
	  l = nl;
	  break;
	}

      mask = mask->next;
    }

  return pos[plen].mark;
}
