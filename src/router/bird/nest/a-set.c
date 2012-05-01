/*
 *	BIRD -- Set/Community-list Operations
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
#include "lib/string.h"

/**
 * int_set_format - format an &set for printing
 * @set: set attribute to be formatted
 * @way: style of format (0 for router ID list, 1 for community list)
 * @from: starting position in set
 * @buf: destination buffer
 * @size: size of buffer
 *
 * This function takes a set attribute and formats it. @way specifies
 * the style of format (router ID / community). @from argument can be
 * used to specify the first printed value for the purpose of printing
 * untruncated sets even with smaller buffers. If the output fits in
 * the buffer, 0 is returned, otherwise the position of the first not
 * printed item is returned. This value can be used as @from argument
 * in subsequent calls. If truncated output suffices, -1 can be
 * instead used as @from, in that case " ..." is eventually added at
 * the buffer to indicate truncation.
 */
int
int_set_format(struct adata *set, int way, int from, byte *buf, unsigned int size)
{
  u32 *z = (u32 *) set->data;
  byte *end = buf + size - 24;
  int from2 = MAX(from, 0);
  int to = set->length / 4;
  int i;

  for (i = from2; i < to; i++)
    {
      if (buf > end)
	{
	  if (from < 0)
	    strcpy(buf, " ...");
	  else
	    *buf = 0;
	  return i;
	}

      if (i > from2)
	*buf++ = ' ';

      if (way)
	buf += bsprintf(buf, "(%d,%d)", z[i] >> 16, z[i] & 0xffff);
      else
	buf += bsprintf(buf, "%R", z[i]);
    }
  *buf = 0;
  return 0;
}

int
ec_format(byte *buf, u64 ec)
{
  u32 type, key, val;
  char tbuf[16], *kind;

  type = ec >> 48;
  switch (type & 0xf0ff)
    {
    case EC_RT: kind = "rt"; break;
    case EC_RO: kind = "ro"; break;

    default:
      kind = tbuf;
      bsprintf(kind, "unknown 0x%x", type);
    }

  switch (ec >> 56)
    {
      /* RFC 4360 3.1.  Two-Octet AS Specific Extended Community */
    case 0x00:
    case 0x40:
      key = (ec >> 32) & 0xFFFF;
      val = ec;
      return bsprintf(buf, "(%s, %u, %u)", kind, key, val);

      /* RFC 4360 3.2.  IPv4 Address Specific Extended Community */
    case 0x01:
    case 0x41:
      key = ec >> 16;
      val = ec & 0xFFFF;
      return bsprintf(buf, "(%s, %R, %u)", kind, key, val);

      /* RFC 5668  4-Octet AS Specific BGP Extended Community */
    case 0x02:
    case 0x42:
      key = ec >> 16;
      val = ec & 0xFFFF;
      return bsprintf(buf, "(%s, %u, %u)", kind, key, val);

      /* Generic format for unknown kinds of extended communities */
    default:
      key = ec >> 32;
      val = ec;
      return bsprintf(buf, "(generic, 0x%x, 0x%x)", key, val);
    }

}

int
ec_set_format(struct adata *set, int from, byte *buf, unsigned int size)
{
  u32 *z = int_set_get_data(set);
  byte *end = buf + size - 24;
  int from2 = MAX(from, 0);
  int to = int_set_get_size(set);
  int i;

  for (i = from2; i < to; i += 2)
    {
      if (buf > end)
	{
	  if (from < 0)
	    strcpy(buf, " ...");
	  else
	    *buf = 0;
	  return i;
	}

      if (i > from2)
	*buf++ = ' ';

      buf += ec_format(buf, ec_get(z, i));
    }
  *buf = 0;
  return 0;
}

int
int_set_contains(struct adata *list, u32 val)
{
  if (!list)
    return 0;

  u32 *l = (u32 *) list->data;
  int len = int_set_get_size(list);
  int i;

  for (i = 0; i < len; i++)
    if (*l++ == val)
      return 1;

  return 0;
}

int
ec_set_contains(struct adata *list, u64 val)
{
  if (!list)
    return 0;

  u32 *l = int_set_get_data(list);
  int len = int_set_get_size(list);
  u32 eh = ec_hi(val);
  u32 el = ec_lo(val);
  int i;

  for (i=0; i < len; i += 2)
    if (l[i] == eh && l[i+1] == el)
      return 1;

  return 0;
}

struct adata *
int_set_add(struct linpool *pool, struct adata *list, u32 val)
{
  struct adata *res;
  int len;

  if (int_set_contains(list, val))
    return list;

  len = list ? list->length : 0;
  res = lp_alloc(pool, sizeof(struct adata) + len + 4);
  res->length = len + 4;
  * (u32 *) res->data = val;
  if (list)
    memcpy((char *) res->data + 4, list->data, list->length);
  return res;
}

struct adata *
ec_set_add(struct linpool *pool, struct adata *list, u64 val)
{
  if (ec_set_contains(list, val))
    return list;

  int olen = list ? list->length : 0;
  struct adata *res = lp_alloc(pool, sizeof(struct adata) + olen + 8);
  res->length = olen + 8;

  if (list)
    memcpy(res->data, list->data, list->length);

  u32 *l = (u32 *) (res->data + res->length - 8);
  l[0] = ec_hi(val);
  l[1] = ec_lo(val);

  return res;
}


struct adata *
int_set_del(struct linpool *pool, struct adata *list, u32 val)
{
  if (!int_set_contains(list, val))
    return list;

  struct adata *res;
  res = lp_alloc(pool, sizeof(struct adata) + list->length - 4);
  res->length = list->length - 4;

  u32 *l = int_set_get_data(list);
  u32 *k = int_set_get_data(res);
  int len = int_set_get_size(list);
  int i;

  for (i = 0; i < len; i++)
    if (l[i] != val)
      *k++ = l[i];

  return res;
}

struct adata *
ec_set_del(struct linpool *pool, struct adata *list, u64 val)
{
  if (!ec_set_contains(list, val))
    return list;

  struct adata *res;
  res = lp_alloc(pool, sizeof(struct adata) + list->length - 8);
  res->length = list->length - 8;

  u32 *l = int_set_get_data(list);
  u32 *k = int_set_get_data(res);
  int len = int_set_get_size(list);
  u32 eh = ec_hi(val);
  u32 el = ec_lo(val);
  int i;

  for (i=0; i < len; i += 2)
    if (! (l[i] == eh && l[i+1] == el))
      {
	*k++ = l[i];
	*k++ = l[i+1];
      }

  return res;
}


struct adata *
int_set_union(struct linpool *pool, struct adata *l1, struct adata *l2)
{
  if (!l1)
    return l2;
  if (!l2)
    return l1;

  struct adata *res;
  int len = int_set_get_size(l2);
  u32 *l = int_set_get_data(l2);
  u32 tmp[len];
  u32 *k = tmp;
  int i;

  for (i = 0; i < len; i++)
    if (!int_set_contains(l1, l[i]))
      *k++ = l[i];

  if (k == tmp)
    return l1;

  len = (k - tmp) * 4;
  res = lp_alloc(pool, sizeof(struct adata) + l1->length + len);
  res->length = l1->length + len;
  memcpy(res->data, l1->data, l1->length);
  memcpy(res->data + l1->length, tmp, len);
  return res;
}

struct adata *
ec_set_union(struct linpool *pool, struct adata *l1, struct adata *l2)
{
  if (!l1)
    return l2;
  if (!l2)
    return l1;

  struct adata *res;
  int len = int_set_get_size(l2);
  u32 *l = int_set_get_data(l2);
  u32 tmp[len];
  u32 *k = tmp;
  int i;

  for (i = 0; i < len; i += 2)
    if (!ec_set_contains(l1, ec_get(l, i)))
      {
	*k++ = l[i];
	*k++ = l[i+1];
      }

  if (k == tmp)
    return l1;

  len = (k - tmp) * 4;
  res = lp_alloc(pool, sizeof(struct adata) + l1->length + len);
  res->length = l1->length + len;
  memcpy(res->data, l1->data, l1->length);
  memcpy(res->data + l1->length, tmp, len);
  return res;
}
