#define VSTR_EXPORT_C
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
/* functions for exporting data out of the Vstr -- see also vstr_cstr.c */
#include "main.h"


size_t vstr_export_iovec_ptr_all(const Vstr_base *base,
                                 struct iovec **iovs, unsigned int *ret_num)
{
  ASSERT(base);

  if (!base->num)
    return (0);

  if (!vstr__cache_iovec_valid((Vstr_base *)base))
    return (0);

  if (iovs)
    *iovs = VSTR__CACHE(base)->vec->v + VSTR__CACHE(base)->vec->off;

  if (ret_num)
    *ret_num = base->num;

  return (base->len);
}

size_t vstr_export_iovec_cpy_buf(const Vstr_base *base,
                                 size_t pos, size_t len,
                                 struct iovec *iovs, unsigned int num_max,
                                 unsigned int *real_ret_num)
{
  Vstr_iter iter[1];
  size_t ret_len = 0;
  unsigned int dummy_ret_num = 0;
  unsigned int ret_num = 0;
  size_t used = 0;

  assert(iovs || !num_max);

  if (!num_max)
    return (0);

  if (!real_ret_num)
    real_ret_num = &dummy_ret_num;

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t tmp = iter->len;

    assert(tmp);
    assert(iovs[ret_num].iov_len);
    assert(iovs[ret_num].iov_len > used);

    if (tmp > (iovs[ret_num].iov_len - used))
      tmp = (iovs[ret_num].iov_len - used);

    if (iter->node->type != VSTR_TYPE_NODE_NON)
      vstr_wrap_memcpy(((char *)iovs[ret_num].iov_base) + used, iter->ptr, tmp);

    used += tmp;
    iter->ptr += tmp;
    iter->len -= tmp;
    ret_len += tmp;

    if (iovs[ret_num].iov_len == used)
    {
      used = 0;

      if (++ret_num >= num_max)
        break;
    }
  } while (iter->len || vstr_iter_fwd_nxt(iter));

  if (used)
  {
    iovs[ret_num].iov_len = used;
    ++ret_num;
  }

  *real_ret_num = ret_num;

  return (ret_len);
}

size_t vstr_export_iovec_cpy_ptr(const Vstr_base *base,
                                 size_t pos, size_t len,
                                 struct iovec *iovs, unsigned int num_max,
                                 unsigned int *real_ret_num)
{
  size_t orig_len = len;
  Vstr_iter iter[1];
  size_t ret_len = 0;
  unsigned int dummy_ret_num = 0;
  unsigned int ret_num = 0;

  assert(iovs || !num_max);

  if (!num_max)
    return (0);

  if (!real_ret_num)
    real_ret_num = &dummy_ret_num;

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    iovs[ret_num].iov_len  =         iter->len;
    iovs[ret_num].iov_base = (void *)iter->ptr;
    ret_len += iter->len;

  } while ((++ret_num < num_max) && vstr_iter_fwd_nxt(iter));
  assert((ret_len == orig_len) || (ret_num == num_max));

  *real_ret_num = ret_num;

  return (ret_len);
}

size_t vstr_export_buf(const Vstr_base *base, size_t pos, size_t len,
                       void *buf, size_t buf_len)
{
  Vstr_iter iter[1];

  ASSERT_RET(base && buf && pos &&
             (((pos <= base->len) &&
               (vstr_sc_poslast(pos, len) <= base->len)) || !len), 0);

  if (!buf_len)
    return (0);

  if (len > buf_len)
    len = buf_len;

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  ASSERT(len == vstr_iter_len(iter));
  
  do
  {
    if (iter->node->type != VSTR_TYPE_NODE_NON)
      vstr_wrap_memcpy(buf, iter->ptr, iter->len);

    buf = ((char *)buf) + iter->len;
  } while (vstr_iter_fwd_nxt(iter));

  return (len);
}

static Vstr_ref *vstr__export_buf_ref(const Vstr_base *base,
                                      size_t pos, size_t len)
{
  Vstr_ref *ref = NULL;

  ASSERT(len);

  if (!(ref = vstr_ref_make_malloc(len)))
  {
    base->conf->malloc_bad = TRUE;
    return (NULL);
  }

  assert(((Vstr__buf_ref *)ref)->buf == ref->ptr);

  vstr_export_buf(base, pos, len, ref->ptr, len);

  return (ref);
}

Vstr_ref *vstr_export_ref(const Vstr_base *base, size_t pos, size_t len,
                          size_t *ret_off)
{
  Vstr_node **scan = NULL;
  unsigned int num = 0;
  Vstr_ref *ref = NULL;
  size_t orig_pos = pos;

  assert(base && pos && len && ((pos + len - 1) <= base->len));
  assert(ret_off);

  if (base->cache_available)
  { /* use cstr cache if available */
    Vstr__cache_data_cstr *data = NULL;
    unsigned int off = base->conf->cache_pos_cb_cstr;

    if ((data = vstr_cache_get(base, off)) && data->ref && data->len)
    {
      if (pos >= data->pos)
      {
        size_t tmp = (pos - data->pos);
        if (data->len <= (len - tmp))
        {
          *ret_off = tmp;
          return (vstr_ref_add(data->ref));
        }
      }
    }
  }

  scan = vstr__base_ptr_pos(base, &pos, &num);
  --pos;

  if (((*scan)->len - pos) >= len)
  {
    if (0)
    { /* do nothing */; }
    else if ((*scan)->type == VSTR_TYPE_NODE_REF)
    {
      *ret_off = pos + ((Vstr_node_ref *)*scan)->off;
      return (vstr_ref_add(((Vstr_node_ref *)*scan)->ref));
    }
    else if ((*scan)->type == VSTR_TYPE_NODE_PTR)
    {
      void *ptr = ((char *)((Vstr_node_ptr *)*scan)->ptr) + pos;

      if (!base->conf->ref_grp_ptr)
      {
        Vstr_ref_grp_ptr *tmp = NULL;
        
        if (!(tmp = vstr__ref_grp_make(vstr_ref_cb_free_nothing, 0)))
        {
          base->conf->malloc_bad = TRUE;
          return (NULL);
        }
        
        base->conf->ref_grp_ptr = tmp;
      }

      if (!(ref = vstr__ref_grp_add(&base->conf->ref_grp_ptr, ptr)))
      {
        base->conf->malloc_bad = TRUE;
        return (NULL);
      }

      *ret_off = 0;

      return (ref);
    }
    else if ((*scan)->type == VSTR_TYPE_NODE_BUF)
    {
      if (!vstr__chg_node_buf_ref(base, scan, num))
        return (NULL);

      /* NOTE: pos can include ->used stuff, which is now gone */
      assert(pos >= ((Vstr_node_ref *)*scan)->off);
      *ret_off = pos;
      return (vstr_ref_add(((Vstr_node_ref *)*scan)->ref));
    }
  }

  *ret_off = 0;
  if (!(ref = vstr__export_buf_ref(base, orig_pos, len)))
    return (NULL);

  return (ref);
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(export_buf);
VSTR__SYM_ALIAS(export_iovec_cpy_buf);
VSTR__SYM_ALIAS(export_iovec_cpy_ptr);
VSTR__SYM_ALIAS(export_iovec_ptr_all);
VSTR__SYM_ALIAS(export_ref);
