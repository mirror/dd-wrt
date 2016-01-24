#define VSTR_SUB_C
/*
 *  Copyright (C) 2001, 2002, 2003, 2004, 2005  James Antill
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
/* functions to substitute data in a vstr */
#include "main.h"

#define VSTR__SUB_BUF() do { \
  if (tmp > buf_len) \
    tmp = buf_len; \
  \
  vstr_wrap_memcpy((((Vstr_node_buf *)scan)->buf) + pos, buf, tmp); \
  buf_len -= tmp; \
  buf = ((char *)buf) + tmp; \
} while (FALSE)

static int vstr__sub_buf_fast(Vstr_base *base, size_t pos, size_t len,
                              const void *buf)
{
  Vstr_iter iter[1];
  size_t buf_len = len;

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (FALSE);

  do
  {
    const size_t tmp = iter->len;

    assert(iter->node->type == VSTR_TYPE_NODE_BUF);

    vstr_wrap_memcpy((char *)iter->ptr, buf, tmp);
    buf = ((char *)buf) + tmp;
  } while (vstr_iter_fwd_nxt(iter));

  vstr_cache_cb_sub(base, pos, buf_len);

  return (TRUE);
}

static int vstr__sub_buf_slow(Vstr_base *base, size_t pos, size_t len,
                              const void *buf, size_t buf_len)
{
  Vstr_iter iter[1];
  size_t orig_pos = pos;
  size_t orig_buf_len = buf_len;
  size_t sub_add_len = 0;
  size_t add_len = 0;
  size_t del_len = 0;
  size_t real_pos = 0;
  int ret = FALSE;
  
  assert(vstr__check_spare_nodes(base->conf));
  assert(vstr__check_real_nodes(base));

  if (len > buf_len)
  {
    del_len = len - buf_len;
    len = buf_len;
  }
  else
    add_len = buf_len - len;

  ret = vstr_iter_fwd_beg(base, pos, len, iter);
  ASSERT_RET(ret, FALSE);

  do
  {
    size_t tmp = iter->len;

    ASSERT(tmp <= buf_len); /* iter is capped to buffer length */

    if (iter->node->type != VSTR_TYPE_NODE_BUF)
      sub_add_len += tmp;

    buf_len -= tmp;
  } while (buf_len && vstr_iter_fwd_nxt(iter));

  buf_len = orig_buf_len;

  if ((sub_add_len == buf_len) || (sub_add_len == len))
  { /* no _BUF nodes, so we can't optimise it anyway ... */
    ret = vstr_add_buf(base, pos - 1, buf, buf_len);

    if (!ret)
      return (FALSE);

    ret = vstr_del(base, pos + buf_len, len + del_len);
    ASSERT(ret || !buf_len); /* if stuff added, then split happened ... so
                              * can't fail */

    return (ret);
  }

  add_len += sub_add_len;

  if (add_len)
  {
    /* allocate extra _BUF nodes needed, all other are ok -- no splits will
     * happen on non _BUF nodes */
    unsigned int num = (add_len / base->conf->buf_sz) + 2;
    if (!vstr_cntl_conf(base->conf, VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BUF,
                        num, UINT_MAX))
      return (FALSE);
  }
  
  if (sub_add_len)
  { /* loop again removing any non _BUF nodes */
    int nxt_iter = FALSE;

    ret = vstr_iter_fwd_beg(base, pos, len, iter);
    ASSERT_RET(ret, FALSE);

    do
    {
      size_t tmp_len        = iter->len;
      unsigned int tmp_type = iter->node->type;

      nxt_iter = vstr_iter_fwd_nxt(iter);

      if (tmp_type != VSTR_TYPE_NODE_BUF)
      {
        vstr_del(base, pos, tmp_len);
        len -= tmp_len;
      }

      pos += tmp_len;
    } while (buf_len && nxt_iter);
  }

  vstr__sub_buf_fast(base, orig_pos, len, buf);

  real_pos = orig_pos + len;
  buf_len -= len;
  buf = ((char *)buf) + len;

  if (del_len)
    vstr_del(base, real_pos, del_len);

  if (buf_len)
    vstr_add_buf(base, real_pos - 1, buf, buf_len);

  assert(vstr__check_spare_nodes(base->conf));
  assert(vstr__check_real_nodes(base));

  return (TRUE);
}

int vstr_sub_buf(Vstr_base *base, size_t pos, size_t len,
                 const void *buf, size_t buf_len)
{
  ASSERT_RET(base, FALSE);
  
  if (!len)
    return (vstr_add_buf(base, pos - 1, buf, buf_len));

  if (!buf_len)
    return (vstr_del(base, pos, len));
  
  if ((len == buf_len) &&
      !base->node_non_used &&
      !base->node_ptr_used &&
      !base->node_ref_used)
    return (vstr__sub_buf_fast(base, pos, len, buf));

  return (vstr__sub_buf_slow(base, pos, len, buf, buf_len));
}

int vstr_sub_non(Vstr_base *base, size_t pos, size_t len, size_t non_len)
{
  int ret = vstr_add_non(base, pos - 1, non_len);

  if (!ret)
    return (FALSE);

  ret = vstr_del(base, pos + non_len, len);
  ASSERT(ret || !non_len); /* if stuff added, then split happened ... so
                            * can't fail */

  return (ret);
}

int vstr_sub_ptr(Vstr_base *base, size_t pos, size_t len,
                 const void *ptr, size_t ptr_len)
{
  int ret = FALSE;
  Vstr_iter iter[1];

  if (!len || !ptr_len)
    goto simple_sub;

  if (!(ret = vstr_iter_fwd_beg(base, pos, len, iter)))
    return (FALSE);

  if ((ptr_len <= VSTR_MAX_NODE_ALL) && VSTR__ITER_EQ_ALL_NODE(base, iter))
  {
    Vstr_node_ptr *ptr_node = NULL;
    Vstr_node_ref *ref_node = NULL;

    if (iter->node->type == VSTR_TYPE_NODE_PTR)
      ptr_node = (Vstr_node_ptr *)iter->node;
    else
    {
      if (iter->node->type == VSTR_TYPE_NODE_REF)
        ref_node = (Vstr_node_ref *)iter->node;
      
      if (!vstr_cntl_conf(base->conf,
                          VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_PTR, 1, UINT_MAX))
        return (FALSE);
      --base->conf->spare_ptr_num;
      ptr_node = base->conf->spare_ptr_beg;
      base->conf->spare_ptr_beg = (Vstr_node_ptr *)ptr_node->s.next;
    }

    if (ptr_len < len)
    {
      size_t diff_len = len - ptr_len;

      len             -= diff_len;
      iter->node->len -= diff_len;
      base->len       -= diff_len;
      vstr__cache_del(base, pos, diff_len);
    }

    ptr_node->s.len = len;
    ptr_node->ptr = (void *)ptr;

    if (ref_node)
      vstr_ref_del(ref_node->ref);

    if (iter->node->type == VSTR_TYPE_NODE_PTR)
      vstr__cache_iovec_reset_node(base, &ptr_node->s, iter->num);
    else
      vstr__swap_node_X_X(base, pos, &ptr_node->s);

    vstr_cache_cb_sub(base, pos, ptr_len);

    if (ptr_len > len)
    {
      size_t diff_len = ptr_len - len;

      ptr_node->s.len += diff_len;
      base->len     += diff_len;

      vstr__cache_iovec_reset_node(base, &ptr_node->s, iter->num);
      vstr__cache_add(base, pos, diff_len);
    }

    assert(vstr__check_spare_nodes(base->conf));
    assert(vstr__check_real_nodes(base));
    return (TRUE);
  }

 simple_sub:
  ret = vstr_add_ptr(base, pos - 1, ptr, ptr_len);

  if (!ret)
    return (FALSE);

  ret = vstr_del(base, pos + ptr_len, len);
  ASSERT(ret || !ptr_len); /* if stuff added, then split happened ... so
                            * can't fail */

  return (ret);
}

int vstr_sub_ref(Vstr_base *base, size_t pos, size_t len,
                 Vstr_ref *ref, size_t off, size_t ref_len)
{
  int ret = FALSE;
  Vstr_iter iter[1];

  if (!len || !ref_len)
    goto simple_sub;

  if (!(ret = vstr_iter_fwd_beg(base, pos, len, iter)))
    return (FALSE);

  if ((ref_len <= VSTR_MAX_NODE_ALL) && VSTR__ITER_EQ_ALL_NODE(base, iter))
  {
    Vstr_node_ref *ref_node = NULL;

    if (iter->node->type == VSTR_TYPE_NODE_REF)
      ref_node = (Vstr_node_ref *)iter->node;
    else
    {
      if (!vstr_cntl_conf(base->conf,
                          VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_REF, 1, UINT_MAX))
        return (FALSE);
      --base->conf->spare_ref_num;
      ref_node = base->conf->spare_ref_beg;
      base->conf->spare_ref_beg = (Vstr_node_ref *)ref_node->s.next;
      ref_node->ref = NULL;
    }

    if (ref_len < len)
    {
      size_t diff_len = len - ref_len;

      len             -= diff_len;
      iter->node->len -= diff_len;
      base->len       -= diff_len;
      vstr__cache_del(base, pos, diff_len);
    }

    vstr_ref_add(ref);
    vstr_ref_del(ref_node->ref);

    ref_node->s.len = len;
    ref_node->ref = ref;
    ref_node->off = off;

    if (iter->node->type == VSTR_TYPE_NODE_REF)
      vstr__cache_iovec_reset_node(base, &ref_node->s, iter->num);
    else
      vstr__swap_node_X_X(base, pos, &ref_node->s);

    vstr_cache_cb_sub(base, pos, ref_len);

    if (ref_len > len)
    {
      size_t diff_len = ref_len - len;

      ref_node->s.len += diff_len;
      base->len     += diff_len;

      vstr__cache_iovec_reset_node(base, &ref_node->s, iter->num);
      vstr__cache_add(base, pos, diff_len);
    }

    assert(vstr__check_spare_nodes(base->conf));
    assert(vstr__check_real_nodes(base));
    return (TRUE);
  }

 simple_sub:
  ret = vstr_add_ref(base, pos - 1, ref, off, ref_len);

  if (!ret)
    return (FALSE);

  ret = vstr_del(base, pos + ref_len, len);
  ASSERT(ret || !ref_len); /* if stuff added, then split happened ... so
                            * can't fail */

  return (ret);
}

int vstr_sub_vstr(Vstr_base *base, size_t pos, size_t len,
                  const Vstr_base *from_base,
                  size_t from_pos, size_t from_len, unsigned int type)
{ /* TODO: this is inefficient compared to vstr_sub_buf() because of atomic
   * guarantees - could be made to work with _ALL_BUF */
  int ret = TRUE;

  assert(pos && from_pos);

  ret = vstr_add_vstr(base, pos - 1, from_base, from_pos, from_len, type);

  if (!ret)
    return (FALSE);

  ret = vstr_del(base, pos + from_len, len);
  ASSERT(ret || !from_len); /* if stuff added, then split happened ... so
                             * can't fail */

  return (ret);
}

int vstr_sub_rep_chr(Vstr_base *base, size_t pos, size_t len,
                     char chr, size_t rep_len)
{
  int ret = TRUE;
  
  ASSERT_RET(ret, FALSE);
  
  if (!len && !rep_len)
    return (TRUE);

  if (rep_len == 1)
    return (vstr_sub_buf(base, pos, len, &chr, 1));
  
  /* TODO: this is a simple opt. */
  if ((len == rep_len) &&
      !base->node_non_used &&
      !base->node_ptr_used &&
      !base->node_ref_used)
  {
    Vstr_iter iter[1];

    if (!(ret = vstr_iter_fwd_beg(base, pos, len, iter)))
      return (FALSE);

    do
    {
      const size_t tmp = iter->len;

      assert(iter->node->type == VSTR_TYPE_NODE_BUF);

      vstr_wrap_memset((char *)iter->ptr, chr, tmp);
    } while (vstr_iter_fwd_nxt(iter));

    vstr_cache_cb_sub(base, pos, rep_len);

    return (TRUE);
  }
  /* need something like vstr_sub_buf() so mostly _BUF strings can
   * be quick here */
     
  ret = vstr_add_rep_chr(base, pos - 1, chr, rep_len);

  if (!ret)
    return (FALSE);

  ret = vstr_del(base, pos + rep_len, len);
  ASSERT(ret || !rep_len); /* if stuff added, then split happened ... so
                            * can't fail */

  return (ret);
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(sub_buf);
VSTR__SYM_ALIAS(sub_non);
VSTR__SYM_ALIAS(sub_ptr);
VSTR__SYM_ALIAS(sub_ref);
VSTR__SYM_ALIAS(sub_rep_chr);
VSTR__SYM_ALIAS(sub_vstr);
