#define VSTR_DEL_C
/*
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004  James Antill
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
/* function to delete data from a vstr */
#include "main.h"


static void vstr__cache_iovec_reset(Vstr_base *base)
{
  ASSERT(!base->num);
  ASSERT(!base->len);

  if (!(base->cache_available && VSTR__CACHE(base) &&
        VSTR__CACHE(base)->vec && VSTR__CACHE(base)->vec->sz))
    return;

  base->iovec_upto_date = TRUE;

  if (VSTR__CACHE(base)->vec->sz > base->conf->iov_min_offset)
    VSTR__CACHE(base)->vec->off = base->conf->iov_min_offset;
  else
    VSTR__CACHE(base)->vec->off = 0;
}

static void vstr__cache_iovec_del_node_end(Vstr_base *base, unsigned int num,
                                           unsigned int len)
{ /* done by hand in vstr-inline.h -- for del */
  if (!base->iovec_upto_date)
    return;

  num += VSTR__CACHE(base)->vec->off;
  VSTR__CACHE(base)->vec->v[num - 1].iov_len -= len;
}

static void vstr__cache_iovec_del_node_beg(Vstr_base *base, Vstr_node *node,
                                           unsigned int num, unsigned int len)
{
  if (!base->iovec_upto_date)
    return;

  num += VSTR__CACHE(base)->vec->off;
  --num;

  if (node->type != VSTR_TYPE_NODE_NON)
  {
    char *tmp = VSTR__CACHE(base)->vec->v[num].iov_base;
    tmp += len;
    VSTR__CACHE(base)->vec->v[num].iov_base = tmp;
  }

  VSTR__CACHE(base)->vec->v[num].iov_len -= len;

  assert((node->len == VSTR__CACHE(base)->vec->v[num].iov_len) ||
         ((base->beg == node) &&
          (node->len == (VSTR__CACHE(base)->vec->v[num].iov_len + base->used))));
}

static void vstr__cache_iovec_del_beg(Vstr_base *base, unsigned int num)
{
  if (!base->iovec_upto_date)
    return;

  VSTR__CACHE(base)->vec->off += num;

  assert(VSTR__CACHE(base)->vec->sz > VSTR__CACHE(base)->vec->off);
  assert((VSTR__CACHE(base)->vec->sz - VSTR__CACHE(base)->vec->off) >= base->num);
}

static void vstr__del_beg_cleanup(Vstr_base *base, Vstr_node **scan,
                                  unsigned int num, int base_update)
{
  if (num)
  {
    Vstr_node *tmp = *scan;

    vstr__relink_nodes(base->conf, base->beg, scan, num);

    base->beg = tmp;

    if (base_update)
    {
      base->num -= num;

      ASSERT(base->beg); /* if == NULL: then
                          * should be del_all, not del_beg */

      vstr__cache_iovec_del_beg(base, num);
    }
  }
}

static void vstr__del_node(Vstr_node *node)
{ /* free resources of a node ... just ref atm. */
  if (node->type == VSTR_TYPE_NODE_REF)
    vstr_ref_del(((Vstr_node_ref *)node)->ref);
}

static void vstr__del_all(Vstr_base *base)
{
  size_t orig_len = base->len;
  unsigned int num = 0;
  unsigned int type;
  Vstr_node **scan = NULL;

  assert(vstr__check_spare_nodes(base->conf));
  assert(vstr__check_real_nodes(base));

  scan = &base->beg;
  type = (*scan)->type;

  base->len = 0;

  while (*scan)
  {
    if ((*scan)->type != type)
    {
      vstr__del_beg_cleanup(base, scan, num, FALSE);
      type = base->beg->type;
      scan = &base->beg;
      num = 0;
    }

    ++num;

    vstr__del_node(*scan);

    scan = &(*scan)->next;
  }
  vstr__del_beg_cleanup(base, scan, num, FALSE);

  base->used = 0;
  base->num = 0;
  ASSERT(!base->beg);
  base->end = NULL;

  base->node_buf_used = FALSE;
  base->node_non_used = FALSE;
  base->node_ptr_used = FALSE;
  base->node_ref_used = FALSE;

  vstr__cache_iovec_reset(base);

  vstr__cache_del(base, 1, orig_len);

  assert(vstr__check_spare_nodes(base->conf));
  assert(vstr__check_real_nodes(base));
}

static void vstr__del_beg(Vstr_base *base, size_t len)
{
  size_t orig_len = len;
  unsigned int num = 0;
  unsigned int type;
  Vstr_node **scan = NULL;

  assert(len && base->beg);

  assert(vstr__check_spare_nodes(base->conf));
  assert(vstr__check_real_nodes(base));

  scan = &base->beg;
  type = (*scan)->type;

  base->len -= len;

  if (base->used)
  {
    assert((*scan)->len > base->used);
    assert((*scan)->type == VSTR_TYPE_NODE_BUF);

    if (len < (size_t)((*scan)->len - base->used))
    {
      base->used += len;

      vstr__cache_iovec_del_node_beg(base, *scan, 1, len);

      vstr__cache_del(base, 1, orig_len);

      assert(vstr__check_real_nodes(base));
      return;
    }

    num = 1;

    len -= ((*scan)->len - base->used);
    /*  vstr_ref_del(((Vstr_node_ref *)(*scan))->ref); */
    type = VSTR_TYPE_NODE_BUF;

    scan = &(*scan)->next;

    base->used = 0;

    ASSERT(*scan || !len);
  }

  while (len > 0)
  {
    if ((*scan)->type != type)
    {
      vstr__del_beg_cleanup(base, scan, num, TRUE);
      type = base->beg->type;
      scan = &base->beg;
      num = 0;
    }

    if (len < (*scan)->len)
    {
      switch ((*scan)->type)
      {
        case VSTR_TYPE_NODE_BUF:
          base->used = len;
          break;
        case VSTR_TYPE_NODE_NON:
          (*scan)->len -= len;
          break;
        case VSTR_TYPE_NODE_PTR:
        {
          char *tmp = ((Vstr_node_ptr *)(*scan))->ptr;
          ((Vstr_node_ptr *)(*scan))->ptr = tmp + len;
          (*scan)->len -= len;
        }
        break;
        case VSTR_TYPE_NODE_REF:
          ((Vstr_node_ref *)(*scan))->off += len;
          (*scan)->len -= len;
          break;
        default:
          assert(FALSE);
      }

      break;
    }

    ++num;
    len -= (*scan)->len;

    vstr__del_node(*scan);

    scan = &(*scan)->next;
  }

  vstr__del_beg_cleanup(base, scan, num, TRUE);

  if (len)
    vstr__cache_iovec_del_node_beg(base, base->beg, 1, len);

  vstr__cache_del(base, 1, orig_len);

  assert(vstr__check_spare_nodes(base->conf));
  assert(vstr__check_real_nodes(base));
}

static Vstr_node *vstr__del_end_cleanup(Vstr_conf *conf,
                                        Vstr_node *beg, Vstr_node **end,
                                        unsigned int num)
{
  Vstr_node *ret = *end;

  vstr__relink_nodes(conf, beg, end, num);

  return (ret);
}

int vstr_extern_inline_del(Vstr_base *base, size_t pos, size_t len)
{
  unsigned int num = 0;
  size_t orig_pos = pos;
  size_t orig_len = len;
  Vstr_node *scan = NULL;
  Vstr_node **pscan = NULL;
  Vstr_node *beg = NULL;
  int type;
  Vstr_node *saved_beg = NULL;
  unsigned int saved_num = 0;
  unsigned int del_nodes = 0;

  ASSERT(len); /* inline version deals with !len */
  ASSERT_RET(base && pos &&
             ((pos <= base->len) &&
              (vstr_sc_poslast(pos, len) <= base->len)), FALSE);

  assert(pos);

  if (pos <= 1)
  {
    if (len >= base->len)
    {
      vstr__del_all(base);

      return (TRUE);
    }

    vstr__del_beg(base, len);
    return (TRUE);
  }

  ASSERT(vstr__check_spare_nodes(base->conf));
  ASSERT(vstr__check_real_nodes(base));

  --pos; /* pos == ammount of chars ok from begining */

  if ((pos + len) >= base->len)
    len = base->len - pos;

  scan = vstr_base__pos(base, &pos, &num, FALSE);

  base->len -= len;

  if (pos != scan->len)
  { /* need to del some data from this node ... */
    size_t tmp = scan->len - pos;

    if (len >= tmp)
    {
      vstr__cache_iovec_del_node_end(base, num, tmp);
      scan->len = pos;
      len -= tmp;
    }
    else
    { /* delete from the middle of a single node */
      switch (scan->type)
      {
        default:
          ASSERT(FALSE);
        case VSTR_TYPE_NODE_NON:
          break;

        case VSTR_TYPE_NODE_BUF:
          if (!base->conf->split_buf_del)
          {
            vstr_wrap_memmove(((Vstr_node_buf *)scan)->buf + pos,
                              ((Vstr_node_buf *)scan)->buf + pos + len,
                              scan->len - (pos + len));
            break;
          }
          /* else FALLTHROUGH */

        case VSTR_TYPE_NODE_PTR:
        case VSTR_TYPE_NODE_REF:
          scan = vstr__base_split_node(base, scan, pos + len);
          if (!scan)
          {
            base->len += len; /* make sure nothing changed */

            assert(vstr__check_spare_nodes(base->conf));
            assert(vstr__check_real_nodes(base));

            return (FALSE);
          }
          break;
      }
      scan->len -= len;
      vstr__cache_iovec_del_node_end(base, num, len);

      vstr__cache_del(base, orig_pos, orig_len);

      assert(vstr__check_spare_nodes(base->conf));
      assert(vstr__check_real_nodes(base));

      return (TRUE);
    }
  }

  saved_beg = scan;
  saved_num = num;
  del_nodes = 0;

  if (!len)
  {
    vstr__cache_del(base, orig_pos, orig_len);

    assert(vstr__check_spare_nodes(base->conf));
    assert(vstr__check_real_nodes(base));

    return (TRUE);
  }

  scan = scan->next;
  assert(scan);

  if (len < scan->len)
    goto fix_last_node;

  len -= scan->len;
  type = scan->type;
  beg = scan;
  vstr__del_node(scan);
  pscan = &scan->next;
  num = 1;
  while (*pscan && (len >= (*pscan)->len))
  {
    vstr__del_node(*pscan);

    len -= (*pscan)->len;

    if ((*pscan)->type != type)
    {
      del_nodes += num;

      scan = vstr__del_end_cleanup(base->conf, beg, pscan, num);
      type = scan->type;
      pscan = &scan->next;
      num = 1;
      beg = scan;
      continue;
    }

    ++num;
    pscan = &(*pscan)->next;
  }

  scan = vstr__del_end_cleanup(base->conf, beg, pscan, num);
  del_nodes += num;

  if (base->iovec_upto_date)
  {
    size_t rebeg_num = saved_num + del_nodes;
    Vstr__cache_data_iovec *vec = VSTR__CACHE(base)->vec;
    struct iovec  *vec_v = (vec->v + vec->off);
    unsigned char *vec_t = (vec->t + vec->off);
    
    vstr_wrap_memmove(vec_v + saved_num, vec_v + rebeg_num,
                      (base->num - rebeg_num) * sizeof(struct iovec));
    vstr_wrap_memmove(vec_t + saved_num, vec_t + rebeg_num,
                      (base->num - rebeg_num));
  }
  base->num -= del_nodes;

  saved_beg->next = scan;
  if (!scan)
    base->end = saved_beg;
  else if (len)
  {
   fix_last_node:
    assert(len < scan->len);

    switch (scan->type)
    {
      case VSTR_TYPE_NODE_BUF:
        /* if (!base->conf->split_buf_del) */
        /* no point in splitting */
        vstr_wrap_memmove(((Vstr_node_buf *)scan)->buf,
                          ((Vstr_node_buf *)scan)->buf + len,
                          scan->len - len);
        break;
      case VSTR_TYPE_NODE_NON:
        break;
      case VSTR_TYPE_NODE_PTR:
        ((Vstr_node_ptr *)scan)->ptr =
          ((char *)((Vstr_node_ptr *)scan)->ptr) + len;
        break;
      case VSTR_TYPE_NODE_REF:
        ((Vstr_node_ref *)scan)->off += len;
        break;
      default:
        assert(FALSE);
    }
    scan->len -= len;

    vstr__cache_iovec_reset_node(base, scan, saved_num + 1);
  }

  vstr__cache_del(base, orig_pos, orig_len);

  assert(vstr__check_spare_nodes(base->conf));
  assert(vstr__check_real_nodes(base));

  return (TRUE);
}

#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(extern_inline_del);
