#define VSTR_MOV_C
/*
 *  Copyright (C) 2001, 2002, 2003  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
 */
/* functions to move data from one vstr to another */
#include "main.h"

static int vstr__mov_slow(Vstr_base *base, size_t pos,
                          Vstr_base *from_base, size_t from_pos, size_t len)
{
  int ret = 0;

  assert(base != from_base);

  ret = vstr_add_vstr(base, pos,
                      from_base, from_pos, len, VSTR_TYPE_ADD_DEF);
  if (!ret)
    return (FALSE);

  ret = vstr_del(from_base, from_pos, len);
  if (!ret)
  {
    ret = vstr_del(base, pos + 1, len);
    assert(ret); /* this must work as a split can't happen */
    return (FALSE);
  }

  return (ret);
}

static int vstr__mov_single_node(Vstr_base *base, size_t pos,
                                 size_t from_pos, size_t len)
{
  Vstr_node *scan = NULL;
  char tbuf[VSTR__STACK_BUF_SZ];
  unsigned int num = 0;

  if (len > sizeof(tbuf))
    return (FALSE);

  /* XX>XXXFFX = cp T, F1, FL; mv P1+FL, P1, F1-P1; cp P1, T, FL */
  /* 123456789 */
  /* XXFFXX>XX = cp T, F1, FL; mv F1, F1+FL, P1-F1; cp P1, T, FL */

  scan = vstr_base__pos(base, &pos, &num, TRUE);
  if ((scan->type == VSTR_TYPE_NODE_BUF) &&
      (vstr_base__pos(base, &from_pos, &num, TRUE) == scan) &&
      (scan->len > len) &&
      ((scan->len - len) >= from_pos))
  {
    char *ptr = vstr_export__node_ptr(scan);

    vstr_wrap_memcpy(tbuf, ptr + from_pos - 1, len);
    if (pos > from_pos)
      vstr_wrap_memmove(ptr + from_pos + len - 1,
                        ptr + from_pos - 1, (pos + 1) - from_pos); /* POSDIFF */
    else
      vstr_wrap_memmove(ptr + pos + len,
                        ptr + pos, from_pos - (pos + 1));
    vstr_wrap_memcpy(ptr + pos, tbuf, len);

    return (TRUE);
  }

  return (FALSE);
}

/* *ret == start of data */
static Vstr_node **vstr__mov_setup_beg(Vstr_base *base, size_t pos,
                                       unsigned int *num, Vstr_node **prev)
{
  Vstr_node *scan = NULL;

  assert(num && pos && prev);
  --pos;
  if (!pos)
  {
    *num = 1;
    vstr__base_zero_used(base);
    *prev = NULL;
    return (&base->beg);
  }

  scan = vstr_base__pos(base, &pos, num, TRUE);

  if ((pos != scan->len) && !(scan = vstr__base_split_node(base, scan, pos)))
    return (NULL);

  ++*num;

  *prev = scan;
  return (&scan->next);
}

/* *ret == after end of data */
static Vstr_node **vstr__mov_setup_end(Vstr_base *base, size_t pos,
                                       unsigned int *num)
{
  Vstr_node *scan = NULL;
  unsigned int dummy_num;

  if (!num)
    num = &dummy_num;

  if (!pos)
  {
    *num = 0;
    vstr__base_zero_used(base);
    return (&base->beg);
  }

  scan = vstr_base__pos(base, &pos, num, TRUE);

  if ((pos != scan->len) && !(scan = vstr__base_split_node(base, scan, pos)))
    return (NULL);

  return (&scan->next);
}

static void vstr__mov_iovec_kill(Vstr_base *base)
{
  if (!base->cache_available)
    return;

  assert(VSTR__CACHE(base));

  base->iovec_upto_date = FALSE;
}

int vstr_mov(Vstr_base *base, size_t pos,
             Vstr_base *from_base, size_t from_pos, size_t len)
{
  Vstr_node *from_prev = NULL;
  Vstr_node **beg = NULL;
  Vstr_node **end = NULL;
  Vstr_node **con = NULL;
  Vstr_node *tmp = NULL;
  unsigned int beg_num = 0;
  unsigned int end_num = 0;
  unsigned int num = 0;
  unsigned int from_node_buf_used = FALSE;
  unsigned int from_node_non_used = FALSE;
  unsigned int from_node_ptr_used = FALSE;
  unsigned int from_node_ref_used = FALSE;
  unsigned int count = 0;

  if (!len)
    return (TRUE);

  assert(!(!base || (pos > base->len) ||
           !from_base || (from_pos > from_base->len)));

  if ((base->conf->buf_sz != from_base->conf->buf_sz) &&
      from_base->node_buf_used)
    return (vstr__mov_slow(base, pos, from_base, from_pos, len));

  if (base == from_base)
  {
    if ((pos >= (from_pos - 1)) && (pos < (from_pos + len)))
      return (TRUE); /* move a string anywhere into itself -- nop */

    if (vstr__mov_single_node(base, pos, from_pos, len))
      return (TRUE);
  }

  assert(vstr__check_real_nodes(base));
  assert((from_base == base) || vstr__check_real_nodes(from_base));

  /* have to aquire the pointers in the right order,
   * depending on which is first */
  if (pos > from_pos)
    goto move_up;

  while (count < 2)
  {
    if (!(con = vstr__mov_setup_end(base, pos, NULL)))
      return (FALSE);
    ++count;

    if (count >= 2)
      break;
    
   move_up:
    if (!(beg = vstr__mov_setup_beg(from_base, from_pos, &beg_num, &from_prev)))
      return (FALSE);

    if (!(end = vstr__mov_setup_end(from_base, from_pos + len - 1, &end_num)))
      return (FALSE);
    ++count;
  }
  assert(count == 2);

  from_node_buf_used = from_base->node_buf_used;
  from_node_non_used = from_base->node_non_used;
  from_node_ptr_used = from_base->node_ptr_used;
  from_node_ref_used = from_base->node_ref_used;

  /* NOTE: the numbers might be off by one if con is before beg,
   * but that doesn't matter as we just need the difference */
  num = end_num - beg_num + 1;

  tmp = *beg;
  if (!(*beg = *end))
    from_base->end = from_prev;

  ASSERT(!!from_base->beg == !!from_base->end);

  from_base->len -= len;
  from_base->num -= num;
  vstr__cache_del(from_base, from_pos, len);
  vstr__mov_iovec_kill(from_base);

  if (!from_base->len)
  {
    from_base->node_buf_used = FALSE;
    from_base->node_non_used = FALSE;
    from_base->node_ptr_used = FALSE;
    from_base->node_ref_used = FALSE;
  }

  beg = &tmp;

  *end = *con;
  *con = *beg;

  if (!*end)
    base->end = VSTR__CONV_PTR_NEXT_PREV(end);

  base->len += len;
  base->num += num;

  base->node_buf_used |= from_node_buf_used;
  base->node_non_used |= from_node_non_used;
  base->node_ptr_used |= from_node_ptr_used;
  base->node_ref_used |= from_node_ref_used;

  vstr__cache_add(base, pos, len);
  vstr__mov_iovec_kill(base); /* This is easy to rm, if they append */

  assert(vstr__check_real_nodes(base));
  assert((from_base == base) || vstr__check_real_nodes(from_base));

  return (TRUE);
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(mov);
