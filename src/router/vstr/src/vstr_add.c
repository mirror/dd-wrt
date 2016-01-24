#define VSTR_ADD_C
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
/* function to add data to a vstr */
#include "main.h"

static void vstr__cache_iovec_add_end(Vstr_base *base, Vstr_node *node,
                                      unsigned int len)
{
  char *tmp = NULL;
  unsigned int num = 0;

  tmp = vstr_export__node_ptr(node);
  ASSERT((node != base->beg) || !base->used);

  num = VSTR__CACHE(base)->vec->off + base->num - 1;
  
  ASSERT(num < VSTR__CACHE(base)->vec->sz);
  
  VSTR__CACHE(base)->vec->v[num].iov_len  = len;
  VSTR__CACHE(base)->vec->v[num].iov_base = tmp;
  VSTR__CACHE(base)->vec->t[num]          = node->type;
}

static void vstr__cache_iovec_add_beg(Vstr_base *base, Vstr_node *node,
                                      unsigned int len)
{
  char *tmp = NULL;
  unsigned int num = 0;

  tmp = vstr_export__node_ptr(node);

  ASSERT(VSTR__CACHE(base)->vec->off);
  num = --VSTR__CACHE(base)->vec->off;

  VSTR__CACHE(base)->vec->v[num].iov_len  = len;
  VSTR__CACHE(base)->vec->v[num].iov_base = tmp;
  VSTR__CACHE(base)->vec->t[num]          = node->type;
}

void vstr__cache_iovec_add_node_end(Vstr_base *base, unsigned int num,
                                    unsigned int len)
{ /* done by hand in vstr-inline.h -- for add_buf and add_rep_chr */
  if (!base->iovec_upto_date)
    return;

  num += VSTR__CACHE(base)->vec->off;
  VSTR__CACHE(base)->vec->v[num - 1].iov_len += len;
}

static void vstr__cache_iovec_maybe_add(Vstr_base *base, Vstr_node *node,
                                        int at_end, unsigned int len)
{
  if (!base->iovec_upto_date)
    return;
  
  if (at_end &&
      (base->num <= (VSTR__CACHE(base)->vec->sz - VSTR__CACHE(base)->vec->off)))
    vstr__cache_iovec_add_end(base, node, len);
  else if (!at_end && VSTR__CACHE(base)->vec->off)
    vstr__cache_iovec_add_beg(base, node, len);
  else
    base->iovec_upto_date = FALSE;
}

Vstr_node *vstr__add_setup_pos(Vstr_base *base, size_t *pos, unsigned int *num,
                               size_t *orig_scan_len)
{
  Vstr_node *scan = NULL;

  assert(base && pos && num && *pos);

  scan = vstr_base__pos(base, pos, num, TRUE);;

  if (orig_scan_len)
    *orig_scan_len = scan->len;

  if ((*pos != scan->len) && !(scan = vstr__base_split_node(base, scan, *pos)))
    return (NULL);
  assert(*pos == scan->len);

  return (scan);
}

/* add 2: one for setup_pos -> split_node ; one for rouding error in divide */
#define VSTR__ADD_BEG(max, int_type, o_p_s_l) do { \
 assert(vstr__check_spare_nodes(base->conf)); \
 assert(vstr__check_real_nodes(base)); \
 \
 if (pos && base->len) \
 { \
   scan = vstr__add_setup_pos(base, &pos, &num, o_p_s_l); \
   if (!scan) \
     return (FALSE); \
 } \
 \
 if (((VSTR_TYPE_NODE_ ## int_type) != VSTR_TYPE_NODE_BUF) || !scan || \
     (scan->type != VSTR_TYPE_NODE_BUF) || (pos != scan->len) || \
     (len > (base->conf->buf_sz - scan->len))) \
 { \
   unsigned int alloc_num = (len / max) + !!(len % max); \
   \
   if (!vstr_cntl_conf(base->conf, \
                       VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_ ## int_type, \
                       alloc_num, UINT_MAX)) \
     return (FALSE); \
 } \
 \
 if (pos && base->len) \
 { \
  pos_scan = scan; \
  pos_scan_next = scan->next \

#define VSTR__ADD_MID(max, node_type) \
  if (scan != base->end) \
    base->iovec_upto_date = FALSE; \
 } \
 else if (base->len) \
 { \
   pos_scan_next = base->beg; \
   assert(!pos); \
   vstr__base_zero_used(base); \
 } else if (!pos && !base->len) pos = 1; \
 \
 scan = (Vstr_node *)base->conf-> spare_ ## node_type ## _beg; \
 if (pos_scan) \
 { \
  assert(base->len); \
  pos_scan->next = scan; \
 } \
 else \
   base->beg = scan; \
 \
 num = 0; \
 base->len += len; \
 \
 while (len > 0) \
 { \
  size_t tmp = (max); \
  \
  if (tmp > len) \
    tmp = len; \
  \
  base-> node_ ## node_type ## _used = TRUE; \
  ++num; \
  ++base->num; \

#define VSTR__ADD_END(node_type, int_type, o_p_s_l) \
  scan->len = tmp; \
  \
  vstr__cache_iovec_maybe_add(base, scan, pos, tmp); \
  \
  len -= tmp; \
  \
  if (!len) \
    break; \
  \
  scan = scan->next; \
 } \
 \
 base->conf-> spare_ ## node_type ## _beg = \
   (Vstr_node_ ## node_type *)scan->next; \
 base->conf-> spare_ ## node_type ## _num -= num; \
 \
 assert(!scan || (scan->type == (VSTR_TYPE_NODE_ ## int_type))); \
 \
 if (!(scan->next = pos_scan_next)) \
   base->end = scan; \
 \
 vstr__cache_add(base, orig_pos, orig_len); \
 \
 ASSERT(vstr__check_spare_nodes(base->conf)); \
 ASSERT(vstr__check_real_nodes(base)); \
} while (FALSE)

int vstr_extern_inline_add_buf(Vstr_base *base, size_t pos,
                               const void *buffer, size_t len)
{
  unsigned int num = 0;
  size_t orig_pos = pos;
  size_t orig_len = len;
  Vstr_node *scan = NULL;
  Vstr_node *pos_scan = NULL;
  Vstr_node *pos_scan_next = NULL;
  size_t orig_pos_scan_len = 0;

  ASSERT_RET(!(!base || !buffer || !len || (pos > base->len)), FALSE);

  VSTR__ADD_BEG(base->conf->buf_sz, BUF, &orig_pos_scan_len);

  if ((scan->type == VSTR_TYPE_NODE_BUF) && (pos == scan->len) &&
      (scan->len < base->conf->buf_sz))
  {
    size_t tmp = (base->conf->buf_sz - scan->len);

    assert(base->node_buf_used);

    if (tmp > len)
      tmp = len;

    vstr_wrap_memcpy(((Vstr_node_buf *)scan)->buf + scan->len, buffer, tmp);
    scan->len += tmp;
    buffer = ((char *)buffer) + tmp;

    vstr__cache_iovec_add_node_end(base, num, tmp);

    base->len += tmp;
    len       -= tmp;

    if (!len)
    {
      vstr__cache_add(base, orig_pos, orig_len);

      assert(vstr__check_real_nodes(base));
      return (TRUE);
    }
  }

  VSTR__ADD_MID(base->conf->buf_sz, buf);

  vstr_wrap_memcpy(((Vstr_node_buf *)scan)->buf, buffer, tmp);
  buffer = ((char *)buffer) + tmp;

  VSTR__ADD_END(buf, BUF, orig_pos_scan_len);

  return (TRUE);
}

int vstr_add_ptr(Vstr_base *base, size_t pos,
                 const void *pass_ptr, size_t len)
{
  unsigned int num = 0;
  size_t orig_pos = pos;
  size_t orig_len = len;
  char *ptr = (char *)pass_ptr; /* store as a char *, but _don't_ alter it */
  Vstr_node *scan = NULL;
  Vstr_node *pos_scan = NULL;
  Vstr_node *pos_scan_next = NULL;

  ASSERT_RET(!(!base || (pos > base->len)), FALSE);

  if (!len)
    return (TRUE);

  VSTR__ADD_BEG(VSTR_MAX_NODE_ALL, PTR, NULL);

  if ((scan->type == VSTR_TYPE_NODE_PTR) &&
      ((((char *)((Vstr_node_ptr *)scan)->ptr) + scan->len) == ptr) &&
      (pos == scan->len) && (scan->len < VSTR_MAX_NODE_ALL))
  {
    size_t tmp = VSTR_MAX_NODE_ALL - scan->len;

    assert(base->node_ptr_used);

    if (tmp > len)
      tmp = len;

    scan->len += tmp;

    vstr__cache_iovec_add_node_end(base, num, tmp);

    base->len += tmp;
    len       -= tmp;

    if (!len)
    {
      vstr__cache_add(base, orig_pos, orig_len);

      assert(vstr__check_real_nodes(base));
      return (TRUE);
    }
  }

  VSTR__ADD_MID(VSTR_MAX_NODE_ALL, ptr);

  base->node_ptr_used = TRUE;
  ((Vstr_node_ptr *)scan)->ptr = ptr;
  ptr += tmp;

  VSTR__ADD_END(ptr, PTR, pos_scan->len);

  return (TRUE);
}

int vstr_add_non(Vstr_base *base, size_t pos, size_t len)
{
  unsigned int num = 0;
  size_t orig_pos = pos;
  size_t orig_len = len;
  Vstr_node *scan = NULL;
  Vstr_node *pos_scan = NULL;
  Vstr_node *pos_scan_next = NULL;
  size_t orig_pos_scan_len = 0;

  ASSERT_RET(!(!base || (pos > base->len)), FALSE);

  if (!len)
    return (TRUE);

  VSTR__ADD_BEG(VSTR_MAX_NODE_ALL, NON, &orig_pos_scan_len);

  if ((scan->type == VSTR_TYPE_NODE_NON) && (scan->len < VSTR_MAX_NODE_ALL))
  {
    size_t tmp = VSTR_MAX_NODE_ALL - scan->len;

    assert(base->node_non_used);

    if (tmp > len)
      tmp = len;

    scan->len += tmp;

    vstr__cache_iovec_add_node_end(base, num, tmp);

    base->len += tmp;
    len       -= tmp;

    if (!len)
    {
      vstr__cache_add(base, orig_pos, orig_len);

      assert(vstr__check_real_nodes(base));
      return (TRUE);
    }
  }

  VSTR__ADD_MID(VSTR_MAX_NODE_ALL, non);
  VSTR__ADD_END(non, NON, orig_pos_scan_len);

  return (TRUE);
}

int vstr_add_ref(Vstr_base *base, size_t pos,
                 Vstr_ref *ref, size_t off, size_t len)
{
  unsigned int num = 0;
  size_t orig_pos = pos;
  size_t orig_len = len;
  Vstr_node *scan = NULL;
  Vstr_node *pos_scan = NULL;
  Vstr_node *pos_scan_next = NULL;

  ASSERT_RET(!(!base || !ref || (pos > base->len)), FALSE);

  if (!len)
    return (TRUE);

  VSTR__ADD_BEG(VSTR_MAX_NODE_ALL, REF, NULL);

  if ((scan->type == VSTR_TYPE_NODE_REF) &&
      (((Vstr_node_ref *)scan)->ref == ref) &&
      ((((Vstr_node_ref *)scan)->off + scan->len) == off) &&
      (pos == scan->len) && (scan->len < VSTR_MAX_NODE_ALL))
  {
    size_t tmp = VSTR_MAX_NODE_ALL - scan->len;

    assert(base->node_ref_used);

    if (tmp > len)
      tmp = len;

    scan->len += tmp;

    vstr__cache_iovec_add_node_end(base, num, tmp);

    base->len += tmp;
    len       -= tmp;

    if (!len)
    {
      vstr__cache_add(base, orig_pos, orig_len);

      assert(vstr__check_real_nodes(base));
      return (TRUE);
    }
  }

  VSTR__ADD_MID(VSTR_MAX_NODE_ALL, ref);

  ((Vstr_node_ref *)scan)->ref = vstr_ref_add(ref);
  ((Vstr_node_ref *)scan)->off = off;
  off += len;

  VSTR__ADD_END(ref, REF, pos_scan->len);

  return (TRUE);
}

/* replace all buf nodes with ref nodes, we don't need to change the
 * vectors if they are there */
static int vstr__convert_buf_ref(Vstr_base *base, size_t pos, size_t len)
{
  Vstr_node **scan = NULL;
  unsigned int num = 0;

  scan = vstr__base_ptr_pos(base, &pos, &num);
  --pos;

  len += pos;

  if (*scan == base->beg)
  { /* if we are on the first node, the conversion will remove the
     * base->used before we get to the (*scan)->len compare */
    ASSERT(((*scan)->type == VSTR_TYPE_NODE_BUF) || !base->used);
    len -= base->used;
  }
  
  while (*scan)
  {
    if ((*scan)->type == VSTR_TYPE_NODE_BUF)
    {
      if (!vstr__chg_node_buf_ref(base, scan, num))
        return (FALSE);
      ASSERT((*scan != base->beg) || !base->used);
    }

    if (len <= (*scan)->len)
      break;
    len -= (*scan)->len;

    scan = &(*scan)->next;
    ++num;
  }
  assert(!len || (*scan && ((*scan)->len >= len)));

  return (TRUE);
}

static int vstr__add_all_ref(Vstr_base *base, size_t pos,
                             Vstr_base *from_base, size_t from_pos, size_t len)
{
  Vstr_ref *ref = NULL;
  size_t off = 0;

  if (!(ref = vstr_export_ref(from_base, from_pos, len, &off)))
  {
    base->conf->malloc_bad = TRUE;
    goto add_all_ref_fail;
  }

  if (!vstr_add_ref(base, pos, ref, off, len))
    goto add_ref_all_ref_fail;

  vstr_ref_del(ref);

  return (TRUE);

 add_ref_all_ref_fail:
  vstr_ref_del(ref);

 add_all_ref_fail:

  from_base->conf->malloc_bad = TRUE;
  return (FALSE);
}

static int vstr__add_vstr_node(Vstr_base *base, size_t pos,
                               Vstr_node *scan, size_t off, size_t len,
                               unsigned int add_type)
{
  switch (scan->type)
  {
    case VSTR_TYPE_NODE_BUF:
      /* all bufs should now be refs */
      assert(add_type != VSTR_TYPE_ADD_BUF_REF);

      if (add_type == VSTR_TYPE_ADD_BUF_PTR)
      {
        if (!vstr_add_ptr(base, pos,
                          ((Vstr_node_buf *)scan)->buf + off, len))
          return (FALSE);
        break;
      }

      if (!vstr_add_buf(base, pos, ((Vstr_node_buf *)scan)->buf + off, len))
        return (FALSE);
      break;

    case VSTR_TYPE_NODE_NON:
      if (!vstr_add_non(base, pos, len))
        return (FALSE);
      break;

    case VSTR_TYPE_NODE_PTR:
    {
      char *ptr = ((Vstr_node_ptr *)scan)->ptr;

      if (add_type == VSTR_TYPE_ADD_ALL_BUF)
      {
        if (!vstr_add_buf(base, pos, ptr + off, len))
          return (FALSE);
        break;
      }

      if (!vstr_add_ptr(base, pos, ptr + off, len))
        return (FALSE);
    }
    break;

    case VSTR_TYPE_NODE_REF:
      off += ((Vstr_node_ref *)scan)->off;
      if (add_type == VSTR_TYPE_ADD_ALL_BUF)
      {
        char *ptr = ((Vstr_node_ref *)scan)->ref->ptr;
        if (!vstr_add_buf(base, pos, ptr + off, len))
          return (FALSE);
        break;
      }

      if (!vstr_add_ref(base, pos, ((Vstr_node_ref *)scan)->ref, off, len))
        return (FALSE);

      ASSERT_NO_SWITCH_DEF();
  }

  return (TRUE);
}

# define DO_CP_LOOP_BEG(x) size_t tmp = scan->len; \
    tmp -= (x); \
    if (tmp > len) tmp = len

# define DO_CP_LOOP_END() do { \
    pos += tmp; \
    len -= tmp; \
    \
    scan = scan->next; \
} while (FALSE)

static size_t vstr__add_vstr_nodes(Vstr_base *base, size_t pos,
                                   Vstr_node *scan, size_t from_pos, size_t len,
                                   unsigned int add_type)
{
  if (len > 0)
  {
    size_t off = from_pos - 1;
    DO_CP_LOOP_BEG(off);

    if (!vstr__add_vstr_node(base, pos, scan, off, tmp, add_type))
      return (0);

    DO_CP_LOOP_END();
  }

  while (len > 0)
  {
    DO_CP_LOOP_BEG(0);

    if (!vstr__add_vstr_node(base, pos, scan, 0, tmp, add_type))
      return (0);

    DO_CP_LOOP_END();
  }

  return (pos);
}

# undef DO_CP_LOOP_BEG
# undef DO_CP_LOOP_END

/* it's so big it looks cluncky, so wrap in a define */
# define DO_VALID_CHK() do { \
    assert(vstr__check_spare_nodes(base->conf)); \
    assert(vstr__check_real_nodes(base)); \
    assert(vstr__check_spare_nodes(from_base->conf)); \
    assert(vstr__check_real_nodes((Vstr_base *)from_base)); \
} while (FALSE)

int vstr_add_vstr(Vstr_base *base, size_t pos,
                  const Vstr_base *from_base, size_t from_pos, size_t len,
                  unsigned int add_type)
{
  Vstr_base *nonconst_from_base = (Vstr_base *)from_base;
  size_t orig_pos = pos;
  size_t orig_from_pos = from_pos;
  size_t orig_len = len;
  size_t orig_base_len = 0;
  Vstr_node *scan = NULL;
  unsigned int dummy_num = 0;

  ASSERT_RET(!(!base || (pos > base->len) ||
               !from_base || ((from_pos > from_base->len) && len)), FALSE);

  orig_base_len = base->len;
  
  if (!len)
    return (TRUE);

  DO_VALID_CHK();

  /* quick short cut instead of using export_cstr_ref() also doesn't change
   * from_base in certain cases */
  if (add_type == VSTR_TYPE_ADD_ALL_REF)
  {
    if (!vstr__add_all_ref(base, pos, nonconst_from_base, from_pos, len))
      goto fail_beg;

    DO_VALID_CHK();

    return (TRUE);
  }

  /* make sure there are no buf nodes */
  if (add_type == VSTR_TYPE_ADD_BUF_REF)
  {
    if (!vstr__convert_buf_ref(nonconst_from_base, from_pos, len))
      goto fail_beg;

    DO_VALID_CHK();
  }

  scan = vstr_base__pos(from_base, &from_pos, &dummy_num, TRUE);
  
  /* do an initial split where it's comming from, if needed (Ie. not a buffer,
   * or it's at the start of the buffer), this is just so we don't call
   * memcpy() on overlapping data... however with split poisoning it'll do
   * bad things */
  if ((from_base == base) && (scan->type == VSTR_TYPE_NODE_BUF) &&
      (from_pos != (((scan == base->beg) ? base->used : 0U) + 1)))
  {
    if (!(scan = vstr__base_split_node(nonconst_from_base, scan, from_pos - 1)))
      goto fail_beg;
    
    ASSERT((from_pos - 1) == scan->len);
    ASSERT(scan->next);
    scan = scan->next;
    from_pos = 1;
  }
  
  if ((from_base == base) && (orig_from_pos <= orig_pos) &&
      ((orig_from_pos + orig_len - 1) > orig_pos))
  { /* ok the vstr has to be copied inside itself, Ie.
     *
     * aaXXXXaa
     *
     * Where a is the From vstr data, and X is the To vstr data,
     * so we have to copy the first part, skip the middle and copy the
     * second part */
    size_t before = vstr_sc_posdiff(orig_from_pos, orig_pos);
    //    size_t before = orig_pos - orig_from_pos;

    assert(before < len);

    if (!(pos = vstr__add_vstr_nodes(base, pos,
                                     scan, from_pos, before, add_type)))
      goto fail_end;

    len -= before;
    from_pos = orig_from_pos + (before * 2);
    scan = vstr_base__pos(from_base, &from_pos, &dummy_num, TRUE);
  }

  if (!vstr__add_vstr_nodes(base, pos, scan, from_pos, len, add_type))
    goto fail_end;

  vstr__cache_cstr_cpy(base, orig_pos, orig_len,
                       nonconst_from_base, orig_from_pos);

  DO_VALID_CHK();

  return (TRUE);

 fail_end:
  /* must work as orig_pos must now be at the begining of a node */
  vstr_del(base, orig_pos + 1, base->len - orig_base_len);
  assert(base->len == orig_base_len);

 fail_beg:
  from_base->conf->malloc_bad = TRUE;
  base->conf->malloc_bad = TRUE;
  DO_VALID_CHK();

  return (FALSE);
}
# undef DO_VALID_CHK

size_t vstr_add_iovec_buf_beg(Vstr_base *base, size_t pos,
                              unsigned int min, unsigned int max,
                              struct iovec **ret_iovs,
                              unsigned int *num)
{
  unsigned int sz = max;
  struct iovec *iovs = NULL;
  unsigned char *types = NULL;
  size_t bytes = 0;
  Vstr_node *scan = NULL;

  ASSERT(base && ret_iovs && num);
  ASSERT_RET(max && (max >= min), 0);

  ASSERT(vstr__check_spare_nodes(base->conf));
  ASSERT(vstr__check_real_nodes(base));

  if (pos != base->len)
    ++min;

  if (!vstr_cntl_conf(base->conf,
                      VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BUF, min, UINT_MAX))
    return (0);

  ASSERT(sz == max); /* max to write to at once */
  if (sz > base->conf->spare_buf_num)
    sz = base->conf->spare_buf_num;

  ASSERT(sz);

  if (!vstr__cache_iovec_alloc(base, base->num + sz))
    return (0);

  if (!vstr__cache_iovec_valid(base))
    assert(FALSE);

  iovs  = VSTR__CACHE(base)->vec->v + VSTR__CACHE(base)->vec->off;
  types = VSTR__CACHE(base)->vec->t + VSTR__CACHE(base)->vec->off;
  *num = 0;

  if (pos)
  {
    unsigned int scan_num = 0;

    ASSERT(base);
    ASSERT_RET((pos <= base->len), 0);
    
    if (!(scan = vstr__add_setup_pos(base, &pos, &scan_num, NULL)))
      return (0);

    if ((scan->type == VSTR_TYPE_NODE_BUF) && (base->conf->buf_sz > scan->len))
    {
      if (sz < max)
        ++sz;

      iovs  += scan_num - 1;
      types += scan_num - 1;

      iovs[0].iov_len  = (base->conf->buf_sz - pos);
      iovs[0].iov_base = (((Vstr_node_buf *)scan)->buf + pos);

      base->iovec_upto_date = FALSE;

      bytes = iovs[0].iov_len;
      *num = 1;
    }
    else
    {
      iovs  += scan_num;
      types += scan_num;

      if (scan != base->end)
        /* if we are adding into the middle of the Vstr then we don't keep
         * the vec valid for after the point where we are adding.
         * This is then updated again in the _end() func */
        base->iovec_upto_date = FALSE;
    }
  }
  else if (base->len)
    /* TODO: maybe work with offset to not damage iovec cache
     * non trivial though */
    base->iovec_upto_date = FALSE;

  scan = (Vstr_node *)base->conf->spare_buf_beg;
  assert(scan);

  while (*num < sz)
  {
    assert(scan->type == VSTR_TYPE_NODE_BUF);

    iovs[*num].iov_len  = base->conf->buf_sz;
    iovs[*num].iov_base = ((Vstr_node_buf *)scan)->buf;
    types[*num] = VSTR_TYPE_NODE_BUF;

    bytes += iovs[*num].iov_len;
    ++*num;

    scan = scan->next;
  }

  *ret_iovs = iovs;
  return (bytes);
}

void vstr_add_iovec_buf_end(Vstr_base *base, size_t pos, size_t bytes)
{
  size_t orig_pos = pos;
  size_t orig_bytes = bytes;
  struct iovec *iovs = NULL;
  unsigned char *types = NULL;
  unsigned int count = 0;
  Vstr_node *scan = NULL;
  Vstr_node **adder = NULL;

  base->node_buf_used |= !!bytes;

  iovs  = VSTR__CACHE(base)->vec->v + VSTR__CACHE(base)->vec->off;
  types = VSTR__CACHE(base)->vec->t + VSTR__CACHE(base)->vec->off;
  if (pos)
  {
    unsigned int scan_num = 0;

    scan = vstr_base__pos(base, &pos, &scan_num, TRUE);
    iovs  += scan_num - 1;
    types += scan_num - 1;

    assert(pos == scan->len);

    if ((scan->type == VSTR_TYPE_NODE_BUF) && (base->conf->buf_sz > scan->len))
    {
      size_t first_iov_len = 0;

      /* normally the first case is true ... but it's possible the user could
       * lowered the value for some reason -- As in vstr__sc_read_len_fd() */
      assert(((base->conf->buf_sz - scan->len) == iovs[0].iov_len) ||
             (((base->conf->buf_sz - scan->len) >  iovs[0].iov_len) &&
              (bytes <= iovs[0].iov_len)));
      assert((((Vstr_node_buf *)scan)->buf + scan->len) == iovs[0].iov_base);

      first_iov_len = iovs[0].iov_len;
      if (first_iov_len > bytes)
        first_iov_len = bytes;

      assert(!base->iovec_upto_date);
      if (scan == base->end)
      {
        base->end = NULL;
        base->iovec_upto_date = TRUE;
      }

      scan->len += first_iov_len;

      vstr__cache_iovec_reset_node(base, scan, scan_num);

      bytes -= first_iov_len;
    }
    else if (scan == base->end)
    {
      base->end = NULL;
      assert(base->iovec_upto_date);
    }

    ++iovs;
    ++types;

    adder = &scan->next;
  }
  else
    adder = &base->beg;

  base->len += orig_bytes;

  if (!bytes)
  {
    if (!base->end)
    {
      assert(!*adder);
      base->end = scan;
    }

    if (!base->iovec_upto_date && base->len)
    {
      count = 0;
      scan = *adder;
      while (scan)
      {
        iovs[count].iov_len = scan->len;

        if (scan == base->beg)
          iovs[count].iov_base = vstr_export__node_ptr(scan) + base->used;
        else
          iovs[count].iov_base = vstr_export__node_ptr(scan);

        types[count] = scan->type;

        ++count;
        scan = scan->next;
      }
    }

    if (orig_bytes)
      vstr__cache_add(base, orig_pos, orig_bytes);

    assert(vstr__check_spare_nodes(base->conf));
    assert(vstr__check_real_nodes(base));

    return;
  }

  scan = (Vstr_node *)base->conf->spare_buf_beg;
  count = 0;
  while (bytes > 0)
  {
    Vstr_node *scan_next = NULL;
    size_t tmp = iovs[count].iov_len;

    assert(scan);
    scan_next = scan->next;

    if (tmp > bytes)
      tmp = bytes;

    assert(((Vstr_node_buf *)scan)->buf == iovs[count].iov_base);
    assert((tmp == base->conf->buf_sz) || (tmp == bytes));

    scan->len = tmp;

    bytes -= tmp;

    if (!bytes)
    {
      if (!(scan->next = *adder))
      {
        assert(base->iovec_upto_date);
        base->end = scan;
      }

      iovs[count].iov_len = tmp;
    }

    scan = scan_next;

    ++count;
  }
  assert(base->conf->spare_buf_num >= count);

  base->num += count;
  base->conf->spare_buf_num -= count;

  assert(base->len);

  if (!base->iovec_upto_date)
  {
    Vstr_node *tmp = *adder;

    while (tmp)
    {
      iovs[count].iov_len  = tmp->len;
      iovs[count].iov_base = vstr_export__node_ptr(tmp);
      types[count] = tmp->type;

      ++count;
      tmp = tmp->next;
    }

    base->iovec_upto_date = TRUE;
  }
  else
    assert(!*adder);

  assert(base->end);

  *adder = (Vstr_node *)base->conf->spare_buf_beg;
  base->conf->spare_buf_beg = (Vstr_node_buf *)scan;

  if (orig_bytes)
    vstr__cache_add(base, orig_pos, orig_bytes);

  assert(vstr__check_spare_nodes(base->conf));
  assert(vstr__check_real_nodes(base));
}

int vstr_extern_inline_add_rep_chr(Vstr_base *base, size_t pos,
                                   char chr, size_t len)
{ /* almost embarassingly similar to add_buf */
  unsigned int num = 0;
  size_t orig_pos = pos;
  size_t orig_len = len;
  Vstr_node *scan = NULL;
  Vstr_node *pos_scan = NULL;
  Vstr_node *pos_scan_next = NULL;
  size_t orig_pos_scan_len = 0;

  ASSERT_RET(!(!base || !len || (pos > base->len)), FALSE);

  VSTR__ADD_BEG(base->conf->buf_sz, BUF, &orig_pos_scan_len);

  if ((scan->type == VSTR_TYPE_NODE_BUF) && (pos == scan->len) &&
      (scan->len < base->conf->buf_sz))
  {
    size_t tmp = (base->conf->buf_sz - scan->len);

    assert(base->node_buf_used);

    if (tmp > len)
      tmp = len;

    vstr_wrap_memset(((Vstr_node_buf *)scan)->buf + scan->len, chr, tmp);
    scan->len += tmp;

    vstr__cache_iovec_add_node_end(base, num, tmp);

    base->len += tmp;
    len       -= tmp;

    if (!len)
    {
      vstr__cache_add(base, orig_pos, orig_len);

      assert(vstr__check_real_nodes(base));
      return (TRUE);
    }
  }

  VSTR__ADD_MID(base->conf->buf_sz, buf);

  /* always do memset -- don't do above */
  vstr_wrap_memset(((Vstr_node_buf *)scan)->buf, chr, tmp);

  VSTR__ADD_END(buf, BUF, orig_pos_scan_len);

  return (TRUE);
}

#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(add_iovec_buf_beg);
VSTR__SYM_ALIAS(add_iovec_buf_end);
VSTR__SYM_ALIAS(add_non);
VSTR__SYM_ALIAS(add_ptr);
VSTR__SYM_ALIAS(add_ref);
VSTR__SYM_ALIAS(add_vstr);
VSTR__SYM_ALIAS(extern_inline_add_buf);
VSTR__SYM_ALIAS(extern_inline_add_rep_chr);
