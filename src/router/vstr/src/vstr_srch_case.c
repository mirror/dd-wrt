#define VSTR_SRCH_CASE_C
/*
 *  Copyright (C) 2002, 2003, 2004  James Antill
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
/* functions for searching within a vstr, in a case independant manner */
#include "main.h"

size_t vstr_srch_case_chr_fwd(const Vstr_base *base, size_t pos, size_t len,
                              char srch)
{
  Vstr_iter iter[1];

  if (!VSTR__IS_ASCII_ALPHA(srch)) /* not searching for a case dependant char */
    return (vstr_srch_chr_fwd(base, pos, len, srch));

  if (VSTR__IS_ASCII_LOWER(srch))
    srch = VSTR__TO_ASCII_UPPER(srch);

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    if (iter->node->type != VSTR_TYPE_NODE_NON)
    {
      size_t count = 0;

      while (count < iter->len)
      {
        char scan_tmp = iter->ptr[count];
        if (VSTR__IS_ASCII_LOWER(scan_tmp))
          scan_tmp = VSTR__TO_ASCII_UPPER(scan_tmp);

        if (scan_tmp == srch)
          return (vstr_iter_pos(iter, pos, len) + count);

        ++count;
      }
    }
  } while (vstr_iter_fwd_nxt(iter));

  return (0);
}

size_t vstr_srch_case_chr_rev(const Vstr_base *base, size_t pos, size_t len,
                              char passed_srch)
{
  char srch[2];

  srch[0] = passed_srch; /* not searching for a case dependant char */
  if (!VSTR__IS_ASCII_ALPHA(srch[0]))
    return (vstr_srch_chr_rev(base, pos, len, srch[0]));

  if (VSTR__IS_ASCII_LOWER(srch[0]))
    srch[1] = VSTR__TO_ASCII_UPPER(srch[0]);
  else
    srch[1] = VSTR__TO_ASCII_LOWER(srch[0]);
  
  return (vstr_srch_chrs_rev(base, pos, len, srch, 2));
}

size_t vstr_srch_case_buf_fwd(const Vstr_base *base, size_t pos, size_t len,
                              const void *const str, const size_t str_len)
{
  Vstr_iter iter[1];
  char tmp = 0;

  if (!len || (str_len > len))
    return (0);

  if (!str_len)
    return (pos);

  if (!str) /* search for _NON lengths are no different */
    return (vstr_srch_buf_fwd(base, pos, len, str, str_len));

  if (str_len == 1)
    return (vstr_srch_case_chr_fwd(base, pos, len, *(const char *)str));

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  assert(len == vstr_iter_len(iter));

  tmp = *(const char *)str;
  if (VSTR__IS_ASCII_LOWER(tmp))
    tmp = VSTR__TO_ASCII_UPPER(tmp);

  do
  {
    assert(str);
    if (iter->node->type == VSTR_TYPE_NODE_NON)
      goto next_loop;

    /* find buf */
    while (iter->len && (vstr_iter_len(iter) >= str_len))
    {
      size_t beg_pos = 0;
      char scan_tmp = 0;

      scan_tmp = *iter->ptr;
      if (VSTR__IS_ASCII_LOWER(scan_tmp))
        scan_tmp = VSTR__TO_ASCII_UPPER(scan_tmp);
      if (scan_tmp != tmp)
        goto next_inc_loop;

      beg_pos = vstr_iter_pos(iter, pos, len);
      if (!vstr_cmp_case_buf(base, beg_pos, str_len,
                             (const char *)str, str_len))
        return (beg_pos);

     next_inc_loop:
      ++iter->ptr;
      --iter->len;
    }

   next_loop:
    continue;
  } while (vstr_iter_fwd_nxt(iter) && (vstr_iter_len(iter) >= str_len));

  return (0);
}

static size_t vstr__srch_case_buf_rev_slow(const Vstr_base *base,
                                           size_t pos, size_t len,
                                           const void *const str,
                                           const size_t str_len)
{
  size_t ret = 0;
  size_t scan_pos = pos;
  size_t scan_len = len;

  while ((scan_pos < (pos + len - 1)) &&
         (scan_len >= str_len))
  {
    size_t tmp = vstr_srch_case_buf_fwd(base, scan_pos, scan_len,
                                        str, str_len);
    if (!tmp)
      break;

    ret = tmp;

    scan_pos = ret + 1;
    scan_len = len - VSTR_SC_POSDIFF(pos, ret);
  }

  return (ret);
}

size_t vstr_srch_case_buf_rev(const Vstr_base *base, size_t pos, size_t len,
                              const void *const str, const size_t str_len)
{
  if (!len || (str_len > len))
    return (0);

  if (!str_len)
    return (pos + len - 1);

  if (str && (str_len == 1))
    return (vstr_srch_case_chr_rev(base, pos, len, *(const char *)str));

  return (vstr__srch_case_buf_rev_slow(base, pos, len, str, str_len));
}

size_t vstr_srch_case_vstr_fwd(const Vstr_base *base, size_t pos, size_t len,
                               const Vstr_base *ndl_base,
                               size_t ndl_pos, size_t ndl_len)
{
  Vstr_iter iter[1];
  size_t scan_pos = pos;
  size_t scan_len = len;

  if (ndl_len > len)
    return (0);

  if (!vstr_iter_fwd_beg(ndl_base, ndl_pos, ndl_len, iter))
    return (0);

  while ((scan_pos < (pos + len - 1)) &&
         (scan_len >= ndl_len))
  {
    if (!vstr_cmp_case(base, scan_pos, ndl_len, ndl_base, ndl_pos, ndl_len))
      return (scan_pos);

    --scan_len;
    ++scan_pos;

    if (iter->node->type != VSTR_TYPE_NODE_NON)
    {
      size_t tmp = 0;

      if (!(tmp = vstr_srch_case_buf_fwd(base, scan_pos, scan_len,
                                         iter->ptr, iter->len)))
        return (0);

      ASSERT(tmp >= scan_pos);
      scan_len -= tmp - scan_pos;
      scan_pos = tmp;
    }
  }

  return (0);
}

static size_t vstr__srch_case_vstr_rev_slow(const Vstr_base *base,
                                            size_t pos, size_t len,
                                            const Vstr_base *ndl_base,
                                            size_t ndl_pos, size_t ndl_len)
{
  size_t ret = 0;
  size_t scan_pos = pos;
  size_t scan_len = len;

  while ((scan_pos < (pos + len - 1)) &&
         (scan_len >= ndl_len))
  {
    size_t tmp = vstr_srch_case_vstr_fwd(base, scan_pos, scan_len,
                                         ndl_base, ndl_pos, ndl_len);
    if (!tmp)
      break;

    ret = tmp;

    scan_pos = ret + 1;
    scan_len = len - VSTR_SC_POSDIFF(pos, ret);
  }

  return (ret);
}

size_t vstr_srch_case_vstr_rev(const Vstr_base *base, size_t pos, size_t len,
                               const Vstr_base *ndl_base,
                               size_t ndl_pos, size_t ndl_len)
{
  return (vstr__srch_case_vstr_rev_slow(base, pos, len,
                                        ndl_base, ndl_pos, ndl_len));
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(srch_case_buf_fwd);
VSTR__SYM_ALIAS(srch_case_buf_rev);
VSTR__SYM_ALIAS(srch_case_chr_fwd);
VSTR__SYM_ALIAS(srch_case_chr_rev);
VSTR__SYM_ALIAS(srch_case_vstr_fwd);
VSTR__SYM_ALIAS(srch_case_vstr_rev);
