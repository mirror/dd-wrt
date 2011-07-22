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
  int to = set->length / 4;
  int i;

  for (i = MAX(from, 0); i < to; i++)
    {
      if (buf > end)
	{
	  if (from < 0)
	    strcpy(buf, " ...");
	  else
	    *buf = 0;
	  return i;
	}

      if (i > from)
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
int_set_contains(struct adata *list, u32 val)
{
  if (!list)
    return 0;

  u32 *l = (u32 *) list->data;
  unsigned int i;
  for (i=0; i<list->length/4; i++)
    if (*l++ == val)
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
  res = lp_alloc(pool, len + sizeof(struct adata) + 4);
  res->length = len + 4;
  * (u32 *) res->data = val;
  if (list)
    memcpy((char *) res->data + 4, list->data, list->length);
  return res;
}

struct adata *
int_set_del(struct linpool *pool, struct adata *list, u32 val)
{
  struct adata *res;
  u32 *l, *k;
  unsigned int i;

  if (!int_set_contains(list, val))
    return list;

  res = lp_alloc(pool, list->length + sizeof(struct adata) - 4);
  res->length = list->length-4;

  l = (u32 *) list->data;
  k = (u32 *) res->data;
  for (i=0; i<list->length/4; i++)
    if (l[i] != val)
      *k++ = l[i];

  return res;
}
